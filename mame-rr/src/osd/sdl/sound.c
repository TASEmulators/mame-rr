//============================================================
//
//  sound.c - SDL implementation of MAME sound routines
//
//  Copyright (c) 1996-2010, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

// standard sdl header
#include <SDL/SDL.h>

// MAME headers
#include "emu.h"
#include "emuopts.h"

#include "osdepend.h"
#include "osdsdl.h"

//============================================================
//  DEBUGGING
//============================================================

#define LOG_SOUND		0

//============================================================
//  PARAMETERS
//============================================================

// number of samples per SDL callback
#define SDL_XFER_SAMPLES	(512)

static int sdl_xfer_samples = SDL_XFER_SAMPLES;
static int stream_in_initialized = 0;
static int stream_loop = 0;

// maximum audio latency
#define MAX_AUDIO_LATENCY		10

//============================================================
//  LOCAL VARIABLES
//============================================================

static int				attenuation = 0;

static int				initialized_audio = 0;
static int				buf_locked;

static INT8				*stream_buffer;
static volatile INT32	stream_playpos;

static UINT32			stream_buffer_size;
static UINT32			stream_buffer_in;

// buffer over/underflow counts
static int				buffer_underflows;
static int				buffer_overflows;

// debugging
static FILE *sound_log;


// sound enable
static int snd_enabled;

//============================================================
//  PROTOTYPES
//============================================================

static int			sdl_init(running_machine *machine);
static void			sdl_kill(running_machine *machine);
static int			sdl_create_buffers(void);
static void			sdl_destroy_buffers(void);
static void			sdl_cleanup_audio(running_machine &machine);
static void			sdl_callback(void *userdata, Uint8 *stream, int len);



//============================================================
//  osd_start_audio_stream
//============================================================
void sdlaudio_init(running_machine *machine)
{
	if (LOG_SOUND)
		sound_log = fopen(SDLMAME_SOUND_LOG, "w");

	// skip if sound disabled
	if (machine->sample_rate != 0)
	{
		// attempt to initialize SDL
		if (sdl_init(machine))
			return;

		machine->add_notifier(MACHINE_NOTIFY_EXIT, sdl_cleanup_audio);
		// set the startup volume
		osd_set_mastervolume(attenuation);
	}
	return;
}



//============================================================
//  osd_stop_audio_stream
//============================================================

static void sdl_cleanup_audio(running_machine &machine)
{
	// if nothing to do, don't do it
	if (machine.sample_rate == 0)
		return;

	// kill the buffers and dsound
	sdl_kill(&machine);
	sdl_destroy_buffers();

	// print out over/underflow stats
	if (buffer_overflows || buffer_underflows)
		mame_printf_verbose("Sound buffer: overflows=%d underflows=%d\n", buffer_overflows, buffer_underflows);

	if (LOG_SOUND)
	{
		fprintf(sound_log, "Sound buffer: overflows=%d underflows=%d\n", buffer_overflows, buffer_underflows);
		fclose(sound_log);
	}
}

//============================================================
//  lock_buffer
//============================================================
static int lock_buffer(long offset, long size, void **buffer1, long *length1, void **buffer2, long *length2)
{
	volatile long pstart, pend, lstart, lend;

	if (!buf_locked)
	{
		if (video_get_throttle())
		{
			pstart = stream_playpos;
			pend = (pstart + sdl_xfer_samples);
			lstart = offset;
			lend = lstart+size;
			while (((pstart >= lstart) && (pstart <= lend)) ||
			       ((pend >= lstart) && (pend <= lend)))
			{
				pstart = stream_playpos;
				pend = pstart + sdl_xfer_samples;
			}
		}

		SDL_LockAudio();
		buf_locked++;
	}

	// init lengths
	*length1 = *length2 = 0;

	if ((offset + size) > stream_buffer_size)
	{
		// 2-piece case
		*length1 = stream_buffer_size - offset;
		*buffer1 = &stream_buffer[offset];
		*length2 = size - *length1;
		*buffer2 = stream_buffer;
	}
	else
	{
		// normal 1-piece case
		*length1 = size;
		*buffer1 = &stream_buffer[offset];
	}

	if (LOG_SOUND)
		fprintf(sound_log, "locking %ld bytes at offset %ld (total %d, playpos %d): len1 %ld len2 %ld\n",
	        size, offset, stream_buffer_size, stream_playpos, *length1, *length2);

	return 0;
}

//============================================================
//  unlock_buffer
//============================================================
static void unlock_buffer(void)
{
	buf_locked--;
	if (!buf_locked)
		SDL_UnlockAudio();

	if (LOG_SOUND)
		fprintf(sound_log, "unlocking\n");

}

//============================================================
//  Apply attenuation
//============================================================

static void att_memcpy(void *dest, INT16 *data, int bytes_to_copy)
{
	int level= (int) (pow(10.0, (float) attenuation / 20.0) * 128.0);
	INT16 *d = (INT16 *) dest;
	int count = bytes_to_copy/2;
	while (count>0)
	{
		*d++ = (*data++ * level) >> 7; /* / 128 */
		count--;
	}
}

//============================================================
//  copy_sample_data
//============================================================

static void copy_sample_data(INT16 *data, int bytes_to_copy)
{
	void *buffer1, *buffer2 = (void *)NULL;
	long length1, length2;
	int cur_bytes;

	// attempt to lock the stream buffer
	if (lock_buffer(stream_buffer_in, bytes_to_copy, &buffer1, &length1, &buffer2, &length2) < 0)
	{
		buffer_underflows++;
		return;
	}

	// adjust the input pointer
	stream_buffer_in += bytes_to_copy;
	if (stream_buffer_in >= stream_buffer_size)
	{
		stream_buffer_in -= stream_buffer_size;
		stream_loop = 1;

		if (LOG_SOUND)
			fprintf(sound_log, "stream_loop set to 1 (stream_buffer_in looped)\n");
	}

	// copy the first chunk
	cur_bytes = (bytes_to_copy > length1) ? length1 : bytes_to_copy;
	att_memcpy(buffer1, data, cur_bytes);

	// adjust for the number of bytes
	bytes_to_copy -= cur_bytes;
	data = (INT16 *)((UINT8 *)data + cur_bytes);

	// copy the second chunk
	if (bytes_to_copy != 0)
	{
		cur_bytes = (bytes_to_copy > length2) ? length2 : bytes_to_copy;
		att_memcpy(buffer2, data, cur_bytes);
	}

	// unlock
	unlock_buffer();
}


//============================================================
//  osd_update_audio_stream
//============================================================

void osd_update_audio_stream(running_machine *machine, INT16 *buffer, int samples_this_frame)
{
	// if nothing to do, don't do it
	if (machine->sample_rate != 0 && stream_buffer)
	{
		int bytes_this_frame = samples_this_frame * sizeof(INT16) * 2;
		int play_position, write_position, stream_in;
		int orig_write; // used in LOG

		play_position = stream_playpos;

		write_position = stream_playpos + ((machine->sample_rate / 50) * sizeof(INT16) * 2);
		orig_write = write_position;

		if (!stream_in_initialized)
		{
			stream_in = stream_buffer_in = (write_position + stream_buffer_size) / 2;

			if (LOG_SOUND)
			{
				fprintf(sound_log, "stream_in = %d\n", (int)stream_in);
				fprintf(sound_log, "stream_playpos = %d\n", (int)stream_playpos);
				fprintf(sound_log, "write_position = %d\n", (int)write_position);
			}
			// start playing
			SDL_PauseAudio(0);

			stream_in_initialized = 1;
		}
		else
		{
			// normalize the stream in position
			stream_in = stream_buffer_in;
			if (stream_in < write_position && stream_loop == 1)
				stream_in += stream_buffer_size;

			// now we should have, in order:
			//    <------pp---wp---si--------------->

			// if we're between play and write positions, then bump forward, but only in full chunks
			while (stream_in < write_position)
			{
				if (LOG_SOUND)
					fprintf(sound_log, "Underflow: PP=%d  WP=%d(%d)  SI=%d(%d)  BTF=%d\n", (int)play_position, (int)write_position, (int)orig_write, (int)stream_in, (int)stream_buffer_in, (int)bytes_this_frame);

				buffer_underflows++;
				stream_in += samples_this_frame;
			}

			// if we're going to overlap the play position, just skip this chunk
			if (stream_in + bytes_this_frame > play_position + stream_buffer_size)
			{
				if (LOG_SOUND)
					fprintf(sound_log, "Overflow: PP=%d  WP=%d(%d)  SI=%d(%d)  BTF=%d\n", (int)play_position, (int)write_position, (int)orig_write, (int)stream_in, (int)stream_buffer_in, (int)bytes_this_frame);

				buffer_overflows++;
				return;
			}
		}

		if (stream_in >= stream_buffer_size)
		{
			stream_in -= stream_buffer_size;
			stream_loop = 1;

			if (LOG_SOUND)
				fprintf(sound_log, "stream_loop set to 1 (stream_in looped)\n");

		}

		// now we know where to copy; let's do it
		stream_buffer_in = stream_in;
		copy_sample_data(buffer, bytes_this_frame);
	}
}



//============================================================
//  osd_set_mastervolume
//============================================================

void osd_set_mastervolume(int _attenuation)
{
	// clamp the attenuation to 0-32 range
	if (_attenuation > 0)
		_attenuation = 0;
	if (_attenuation < -32)
		_attenuation = -32;

	attenuation = _attenuation;
}

//============================================================
//  sdl_callback
//============================================================
static void sdl_callback(void *userdata, Uint8 *stream, int len)
{
	int len1, len2, sb_in;

	sb_in = stream_buffer_in;
	if (stream_loop)
		sb_in += stream_buffer_size;

	if (sb_in < (stream_playpos+len))
	{
		if (LOG_SOUND)
			fprintf(sound_log, "Underflow at sdl_callback: SPP=%d SBI=%d(%d) Len=%d\n", (int)stream_playpos, (int)sb_in, (int)stream_buffer_in, (int)len);

		return;
	}
	else if ((stream_playpos+len) > stream_buffer_size)
	{
		len1 = stream_buffer_size - stream_playpos;
		len2 = len - len1;
	}
	else
	{
		len1 = len;
		len2 = 0;
	}

	if (snd_enabled)
	{
		memcpy(stream, stream_buffer + stream_playpos, len1);
		memset(stream_buffer + stream_playpos, 0, len1); // no longer needed
		if (len2)
		{
			memcpy(stream+len1, stream_buffer, len2);
			memset(stream_buffer, 0, len2); // no longer needed
		}

	}
	else
	{
		memset(stream, 0, len);
	}

	// move the play cursor
	stream_playpos += len1 + len2;
	if (stream_playpos >= stream_buffer_size)
	{
		stream_playpos -= stream_buffer_size;
		stream_loop = 0;

		if (LOG_SOUND)
			fprintf(sound_log, "stream_loop set to 0 (stream_playpos looped)\n");
	}

	if (LOG_SOUND)
		fprintf(sound_log, "callback: xfer len1 %d len2 %d, playpos %d\n",
				len1, len2, stream_playpos);
}


//============================================================
//  sdl_init
//============================================================
static int sdl_init(running_machine *machine)
{
	int			n_channels = 2;
	int			audio_latency;
	SDL_AudioSpec	aspec, obtained;
	char audio_driver[16] = "";

	if (initialized_audio)
	{
		sdl_cleanup_audio(*machine);
	}

	mame_printf_verbose("Audio: Start initialization\n");
#if (SDL_VERSION_ATLEAST(1,3,0))
	strncpy(audio_driver, SDL_GetCurrentAudioDriver(), sizeof(audio_driver));
#else
	SDL_AudioDriverName(audio_driver, sizeof(audio_driver));
#endif
	mame_printf_verbose("Audio: Driver is %s\n", audio_driver);

	initialized_audio = 0;

	sdl_xfer_samples = SDL_XFER_SAMPLES;
	stream_in_initialized = 0;
	stream_loop = 0;

	// set up the audio specs
	aspec.freq = machine->sample_rate;
	aspec.format = AUDIO_S16SYS;	// keep endian independent
	aspec.channels = n_channels;
	aspec.samples = sdl_xfer_samples;
	aspec.callback = sdl_callback;
	aspec.userdata = 0;

	if (SDL_OpenAudio(&aspec, &obtained) < 0)
		goto cant_start_audio;

	initialized_audio = 1;
	snd_enabled = 1;

	mame_printf_verbose("Audio: frequency: %d, channels: %d, samples: %d\n",
	                    obtained.freq, obtained.channels, obtained.samples);

	sdl_xfer_samples = obtained.samples;

	audio_latency = options_get_int(machine->options(), SDLOPTION_AUDIO_LATENCY);

	// pin audio latency
	if (audio_latency > MAX_AUDIO_LATENCY)
	{
		audio_latency = MAX_AUDIO_LATENCY;
	}
	else if (audio_latency < 1)
	{
		audio_latency = 1;
	}

	// compute the buffer sizes
	stream_buffer_size = machine->sample_rate * 2 * sizeof(INT16) * audio_latency / MAX_AUDIO_LATENCY;
	stream_buffer_size = (stream_buffer_size / 1024) * 1024;
	if (stream_buffer_size < 1024)
		stream_buffer_size = 1024;

	// create the buffers
	if (sdl_create_buffers())
		goto cant_create_buffers;

	mame_printf_verbose("Audio: End initialization\n");
	return 0;

	// error handling
cant_create_buffers:
cant_start_audio:
	mame_printf_verbose("Audio: Initialization failed. SDL error: %s\n", SDL_GetError());

	return 0;
}



//============================================================
//  sdl_kill
//============================================================

static void sdl_kill(running_machine *machine)
{
	if (initialized_audio)
	{
		mame_printf_verbose("sdl_kill: closing audio\n");

		SDL_CloseAudio();
	}
}



//============================================================
//  dsound_create_buffers
//============================================================

static int sdl_create_buffers(void)
{
	mame_printf_verbose("sdl_create_buffers: creating stream buffer of %u bytes\n", stream_buffer_size);

	stream_buffer = global_alloc_array_clear(INT8, stream_buffer_size);
	stream_playpos = 0;
	buf_locked = 0;
	return 0;
}



//============================================================
//  sdl_destroy_buffers
//============================================================

static void sdl_destroy_buffers(void)
{
	// release the buffer
	if (stream_buffer)
		global_free(stream_buffer);
	stream_buffer = NULL;
}

