/***************************************************************************

An Hitachi HD637A01X0 MCU programmed to act as a sample player.
Used by some Namco System 86 games.

The MCU has internal ROM which hasn't been dumped, so here we simulate its
simple functions.

The chip can address ROM space up to 8 block of 0x10000 bytes. At the beginning
of each block there's a table listing the start offset of each sample.
Samples are 8 bit unsigned, 0xff marks the end of the sample. 0x00 is used for
silence compression: '00 nn' must be replaced by nn+1 times '80'.

***************************************************************************/

#include "emu.h"
#include "streams.h"
#include "n63701x.h"


typedef struct
{
	int select;
	int playing;
	int base_addr;
	int position;
	int volume;
	int silence_counter;
} voice;


typedef struct _namco_63701x namco_63701x;
struct _namco_63701x
{
	voice voices[2];
	sound_stream * stream;		/* channel assigned by the mixer */
	UINT8 *rom;		/* pointer to sample ROM */
};


/* volume control has three resistors: 22000, 10000 and 3300 Ohm.
   22000 is always enabled, the other two can be turned off.
   Since 0x00 and 0xff samples have special meaning, the available range is
   0x01 to 0xfe, therefore 258 * (0x01 - 0x80) = 0x8002 just keeps us
   inside 16 bits without overflowing.
 */
static const int vol_table[4] = { 26, 84, 200, 258 };


INLINE namco_63701x *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->type() == SOUND_NAMCO_63701X);
	return (namco_63701x *)downcast<legacy_device_base *>(device)->token();
}


static STREAM_UPDATE( namco_63701x_update )
{
	namco_63701x *chip = (namco_63701x *)param;
	int ch;

	for (ch = 0;ch < 2;ch++)
	{
		stream_sample_t *buf = outputs[ch];
		voice *v = &chip->voices[ch];

		if (v->playing)
		{
			UINT8 *base = chip->rom + v->base_addr;
			int pos = v->position;
			int vol = vol_table[v->volume];
			int p;

			for (p = 0;p < samples;p++)
			{
				if (v->silence_counter)
				{
					v->silence_counter--;
					*(buf++) = 0;
				}
				else
				{
					int data = base[(pos++) & 0xffff];

					if (data == 0xff)	/* end of sample */
					{
						v->playing = 0;
						break;
					}
					else if (data == 0x00)	/* silence compression */
					{
						data = base[(pos++) & 0xffff];
						v->silence_counter = data;
						*(buf++) = 0;
					}
					else
					{
						*(buf++) = vol * (data - 0x80);
					}
				}
			}

			v->position = pos;
		}
		else
			memset(buf, 0, samples * sizeof(*buf));
	}
}


static DEVICE_START( namco_63701x )
{
	namco_63701x *chip = get_safe_token(device);

	chip->rom = *device->region();

	chip->stream = stream_create(device, 0, 2, device->clock()/1000, chip, namco_63701x_update);
}



WRITE8_DEVICE_HANDLER( namco_63701x_w )
{
	namco_63701x *chip = get_safe_token(device);
	int ch = offset / 2;

	if (offset & 1)
		chip->voices[ch].select = data;
	else
	{
		/*
          should we stop the playing sample if voice_select[ch] == 0 ?
          originally we were, but this makes us lose a sample in genpeitd,
          after the continue counter reaches 0. Either we shouldn't stop
          the sample, or genpeitd is returning to the title screen too soon.
         */
		if (chip->voices[ch].select & 0x1f)
		{
			int rom_offs;

			/* update the streams */
			stream_update(chip->stream);

			chip->voices[ch].playing = 1;
			chip->voices[ch].base_addr = 0x10000 * ((chip->voices[ch].select & 0xe0) >> 5);
			rom_offs = chip->voices[ch].base_addr + 2 * ((chip->voices[ch].select & 0x1f) - 1);
			chip->voices[ch].position = (chip->rom[rom_offs] << 8) + chip->rom[rom_offs+1];
			/* bits 6-7 = volume */
			chip->voices[ch].volume = data >> 6;
			/* bits 0-5 = counter to indicate new sample start? we don't use them */

			chip->voices[ch].silence_counter = 0;
		}
	}
}






/**************************************************************************
 * Generic get_info
 **************************************************************************/

DEVICE_GET_INFO( namco_63701x )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(namco_63701x);						break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( namco_63701x );	break;
		case DEVINFO_FCT_STOP:							/* Nothing */										break;
		case DEVINFO_FCT_RESET:							/* Nothing */										break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Namco 63701X");					break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Namco custom");					break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");								break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);							break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}


DEFINE_LEGACY_SOUND_DEVICE(NAMCO_63701X, namco_63701x);
