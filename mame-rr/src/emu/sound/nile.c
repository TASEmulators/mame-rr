/************************************
      Seta custom Nile ST-0026 chip
      sound emulation by Tomasz Slanina
      based on ST-0016 emulation

8 voices, 16 words of config data for each:

   00
   01 - sptr  ?? (always 0)
   02 - sptr  LO
   03 - sptr  HI
   04
   05 - flags? 00000000 0000?L0?   - bit 0 loops, other bits appear to be not used by the chip
   06 - freq
   07 - lsptr LO
   08
   09 - lsptr HI
   0a - leptr LO
   0b - leptr HI
   0c - eptr  LO
   0d - eptr  HI
   0e - vol R
   0f - vol L

************************************/

#include "emu.h"
#include "streams.h"
#include "nile.h"

#define NILE_VOICES 8

enum
{
	NILE_REG_UNK0=0,
	NILE_REG_SPTR_TOP,
	NILE_REG_SPTR_LO,
	NILE_REG_SPTR_HI,
	NILE_REG_UNK_4,
	NILE_REG_FLAGS,
	NILE_REG_FREQ,
	NILE_REG_LSPTR_LO,
	MILE_REG_UNK_8,
	NILE_REG_LSPTR_HI,
	NILE_REG_LEPTR_LO,
	NILE_REG_LEPTR_HI,
	NILE_REG_EPTR_LO,
	NILE_REG_EPTR_HI,
	NILE_REG_VOL_R,
	NILE_REG_VOL_L
};



UINT16 *nile_sound_regs;

typedef struct _nile_state nile_state;
struct _nile_state
{
	sound_stream * stream;
	UINT8 *sound_ram;
	int vpos[NILE_VOICES], frac[NILE_VOICES], lponce[NILE_VOICES];
	UINT16 ctrl;
};

INLINE nile_state *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->type() == SOUND_NILE);
	return (nile_state *)downcast<legacy_device_base *>(device)->token();
}


WRITE16_DEVICE_HANDLER( nile_sndctrl_w )
{
	nile_state *info = get_safe_token(device);
	UINT16 ctrl=info->ctrl;

	stream_update(info->stream);

	COMBINE_DATA(&info->ctrl);

//  printf("CTRL: %04x -> %04x (PC=%x)\n", ctrl, info->ctrl, cpu_get_pc(space->cpu));

	ctrl^=info->ctrl;
}

READ16_DEVICE_HANDLER( nile_sndctrl_r )
{
	nile_state *info = get_safe_token(device);

	stream_update(info->stream);

	return info->ctrl;
}

READ16_DEVICE_HANDLER( nile_snd_r )
{
	nile_state *info = get_safe_token(device);
	int reg=offset&0xf;

	stream_update(info->stream);

	if(reg==2 || reg==3)
	{
		int slot=offset/16;
		int sptr = ((nile_sound_regs[slot*16+3]<<16)|nile_sound_regs[slot*16+2])+info->vpos[slot];

		if(reg==2)
		{
			return sptr&0xffff;
		}
		else
		{
			return sptr>>16;
		}
	}
	return nile_sound_regs[offset];
}

WRITE16_DEVICE_HANDLER( nile_snd_w )
{
	nile_state *info = get_safe_token(device);
	int v, r;

	stream_update(info->stream);

	COMBINE_DATA(&nile_sound_regs[offset]);

	v = offset / 16;
	r = offset % 16;

	if ((r == 2) || (r == 3))
	{
		info->vpos[v] = info->frac[v] = info->lponce[v] = 0;
	}

//  printf("v%02d: %04x to reg %02d (PC=%x)\n", v, nile_sound_regs[offset], r, cpu_get_pc(space->cpu));
}

static STREAM_UPDATE( nile_update )
{
	nile_state *info = (nile_state *)param;
	UINT8 *sound_ram = info->sound_ram;
	int v, i, snum;
	UINT16 *slot;
	INT32 mix[48000*2];
	INT32 *mixp;
	INT16 sample;
	int sptr, eptr, freq, lsptr, leptr;

	lsptr=leptr=0;

	memset(mix, 0, sizeof(mix[0])*samples*2);

	for (v = 0; v < NILE_VOICES; v++)
	{
		slot = &nile_sound_regs[v * 16];

		if (info->ctrl&(1<<v))
		{
			mixp = &mix[0];

			sptr = slot[NILE_REG_SPTR_HI]<<16 | slot[NILE_REG_SPTR_LO];
			eptr = slot[NILE_REG_EPTR_HI]<<16 | slot[NILE_REG_EPTR_LO];

			freq=slot[NILE_REG_FREQ]*14;
			lsptr = slot[NILE_REG_LSPTR_HI]<<16 | slot[NILE_REG_LSPTR_LO];
			leptr = slot[NILE_REG_LEPTR_HI]<<16 | slot[NILE_REG_LEPTR_LO];

			for (snum = 0; snum < samples; snum++)
			{
				sample = sound_ram[sptr + info->vpos[v]]<<8;

				*mixp++ += (sample * (INT32)slot[NILE_REG_VOL_R]) >> 16;
				*mixp++ += (sample * (INT32)slot[NILE_REG_VOL_L]) >> 16;

				info->frac[v] += freq;
				info->vpos[v] += info->frac[v]>>16;
				info->frac[v] &= 0xffff;

				// stop if we're at the end
				if (info->lponce[v])
				{
					// we've looped once, check loop end rather than sample end
					if ((info->vpos[v] + sptr) >= leptr)
					{
						info->vpos[v] = (lsptr - sptr);
					}
				}
				else
				{
					// not looped yet, check sample end
					if ((info->vpos[v] + sptr) >= eptr)
					{
						// code at 11d8c:
						// if bit 2 (0x4) is set, check if loop start = loop end.
						// if they are equal, clear bit 0 and don't set the loop start/end
						// registers in the NiLe.  if they aren't, set bit 0 and set
						// the loop start/end registers in the NiLe.
						if ((slot[NILE_REG_FLAGS] & 0x5) == 0x5)
						{
							info->vpos[v] = (lsptr - sptr);
							info->lponce[v] = 1;
						}
						else
						{
							info->ctrl &= ~(1<<v);
							info->vpos[v] = (eptr - sptr);
							info->frac[v] = 0;
						}

					}
				}
			}
		}
	}
	mixp = &mix[0];
	for (i = 0; i < samples; i++)
	{
		outputs[0][i] = (*mixp++)>>4;
		outputs[1][i] = (*mixp++)>>4;
	}
}

static DEVICE_START( nile )
{
	nile_state *info = get_safe_token(device);

	info->sound_ram = *device->region();

	info->stream = stream_create(device, 0, 2, 44100, info, nile_update);
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

DEVICE_GET_INFO( nile )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(nile_state);				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( nile );	break;
		case DEVINFO_FCT_STOP:							/* Nothing */								break;
		case DEVINFO_FCT_RESET:							/* Nothing */								break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "NiLe");					break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Seta custom");				break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");						break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);					break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}


DEFINE_LEGACY_SOUND_DEVICE(NILE, nile);
