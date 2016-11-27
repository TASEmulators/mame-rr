/***************************************************************************

    Wiping sound driver (quick hack of the Namco sound driver)

    used by wiping.c and clshroad.c

***************************************************************************/

#include "emu.h"
#include "streams.h"
#include "includes/wiping.h"


/* 8 voices max */
#define MAX_VOICES 8


static const int samplerate = 48000;
static const int defgain = 48;


/* this structure defines the parameters for a channel */
typedef struct
{
	int frequency;
	int counter;
	int volume;
	const UINT8 *wave;
	int oneshot;
	int oneshotplaying;
} sound_channel;


/* globals available to everyone */
UINT8 *wiping_soundregs;

/* data about the sound system */
static sound_channel channel_list[MAX_VOICES];
static sound_channel *last_channel;

/* global sound parameters */
static const UINT8 *sound_prom,*sound_rom;
static int num_voices;
static int sound_enable;
static sound_stream *stream;

/* mixer tables and internal buffers */
static INT16 *mixer_table;
static INT16 *mixer_lookup;
static short *mixer_buffer;
static short *mixer_buffer_2;



/* build a table to divide by the number of voices; gain is specified as gain*16 */
static void make_mixer_table(running_machine *machine, int voices, int gain)
{
	int count = voices * 128;
	int i;

	/* allocate memory */
	mixer_table = auto_alloc_array(machine, INT16, 256 * voices);

	/* find the middle of the table */
	mixer_lookup = mixer_table + (128 * voices);

	/* fill in the table - 16 bit case */
	for (i = 0; i < count; i++)
	{
		int val = i * gain * 16 / voices;
		if (val > 32767) val = 32767;
		mixer_lookup[ i] = val;
		mixer_lookup[-i] = -val;
	}
}


/* generate sound to the mix buffer in mono */
static STREAM_UPDATE( wiping_update_mono )
{
	stream_sample_t *buffer = outputs[0];
	sound_channel *voice;
	short *mix;
	int i;

	/* if no sound, we're done */
	if (sound_enable == 0)
	{
		memset(buffer, 0, samples * sizeof(*buffer));
		return;
	}

	/* zap the contents of the mixer buffer */
	memset(mixer_buffer, 0, samples * sizeof(short));

	/* loop over each voice and add its contribution */
	for (voice = channel_list; voice < last_channel; voice++)
	{
		int f = 16*voice->frequency;
		int v = voice->volume;

		/* only update if we have non-zero volume and frequency */
		if (v && f)
		{
			const UINT8 *w = voice->wave;
			int c = voice->counter;

			mix = mixer_buffer;

			/* add our contribution */
			for (i = 0; i < samples; i++)
			{
				int offs;

				c += f;

				if (voice->oneshot)
				{
					if (voice->oneshotplaying)
					{
						offs = (c >> 15);
						if (w[offs>>1] == 0xff)
						{
							voice->oneshotplaying = 0;
						}

						else
						{
							/* use full byte, first the high 4 bits, then the low 4 bits */
							if (offs & 1)
								*mix++ += ((w[offs>>1] & 0x0f) - 8) * v;
							else
								*mix++ += (((w[offs>>1]>>4) & 0x0f) - 8) * v;
						}
					}
				}
				else
				{
					offs = (c >> 15) & 0x1f;

					/* use full byte, first the high 4 bits, then the low 4 bits */
					if (offs & 1)
						*mix++ += ((w[offs>>1] & 0x0f) - 8) * v;
					else
						*mix++ += (((w[offs>>1]>>4) & 0x0f) - 8) * v;
				}
			}

			/* update the counter for this voice */
			voice->counter = c;
		}
	}

	/* mix it down */
	mix = mixer_buffer;
	for (i = 0; i < samples; i++)
		*buffer++ = mixer_lookup[*mix++];
}



static DEVICE_START( wiping_sound )
{
	running_machine *machine = device->machine;
	sound_channel *voice;

	/* get stream channels */
	stream = stream_create(device, 0, 1, samplerate, NULL, wiping_update_mono);

	/* allocate a pair of buffers to mix into - 1 second's worth should be more than enough */
	mixer_buffer = auto_alloc_array(machine, short, 2 * samplerate);
	mixer_buffer_2 = mixer_buffer + samplerate;

	/* build the mixer table */
	make_mixer_table(device->machine, 8, defgain);

	/* extract globals from the interface */
	num_voices = 8;
	last_channel = channel_list + num_voices;

	sound_rom = memory_region(machine, "samples");
	sound_prom = memory_region(machine, "soundproms");

	/* start with sound enabled, many games don't have a sound enable register */
	sound_enable = 1;

	/* reset all the voices */
	for (voice = channel_list; voice < last_channel; voice++)
	{
		voice->frequency = 0;
		voice->volume = 0;
		voice->wave = &sound_prom[0];
		voice->counter = 0;
	}
}


DEVICE_GET_INFO( wiping_sound )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(wiping_sound);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Wiping Custom");				break;
		case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);						break;
	}
}



/********************************************************************************/

WRITE8_HANDLER( wiping_sound_w )
{
	sound_channel *voice;
	int base;

	/* update the streams */
	stream_update(stream);

	/* set the register */
	wiping_soundregs[offset] = data;

	/* recompute all the voice parameters */
	if (offset <= 0x3f)
	{
		for (base = 0, voice = channel_list; voice < last_channel; voice++, base += 8)
		{
			voice->frequency = wiping_soundregs[0x02 + base] & 0x0f;
			voice->frequency = voice->frequency * 16 + ((wiping_soundregs[0x01 + base]) & 0x0f);
			voice->frequency = voice->frequency * 16 + ((wiping_soundregs[0x00 + base]) & 0x0f);

			voice->volume = wiping_soundregs[0x07 + base] & 0x0f;
			if (wiping_soundregs[0x5 + base] & 0x0f)
			{
				voice->wave = &sound_rom[128 * (16 * (wiping_soundregs[0x5 + base] & 0x0f)
						+ (wiping_soundregs[0x2005 + base] & 0x0f))];
				voice->oneshot = 1;
			}
			else
			{
				voice->wave = &sound_rom[16 * (wiping_soundregs[0x3 + base] & 0x0f)];
				voice->oneshot = 0;
				voice->oneshotplaying = 0;
			}
		}
	}
	else if (offset >= 0x2000)
	{
		voice = &channel_list[(offset & 0x3f)/8];
		if (voice->oneshot)
		{
			voice->counter = 0;
			voice->oneshotplaying = 1;
		}
	}
}


DEFINE_LEGACY_SOUND_DEVICE(WIPING, wiping_sound);
