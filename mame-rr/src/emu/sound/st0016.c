/************************************
      Seta custom ST-0016 chip
      sound emulation by R. Belmont, Tomasz Slanina, and David Haywood
************************************/

#include "emu.h"
#include "streams.h"
#include "st0016.h"

#define VERBOSE (0)
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

typedef struct _st0016_state st0016_state;
struct _st0016_state
{
	sound_stream * stream;
	UINT8 **sound_ram;
	int vpos[8], frac[8], lponce[8];
	UINT8 regs[0x100];
};

INLINE st0016_state *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->type() == SOUND_ST0016);
	return (st0016_state *)downcast<legacy_device_base *>(device)->token();
}


READ8_DEVICE_HANDLER( st0016_snd_r )
{
	st0016_state *info = get_safe_token(device);
	return info->regs[offset];
}

WRITE8_DEVICE_HANDLER( st0016_snd_w )
{
	st0016_state *info = get_safe_token(device);
	int voice = offset/32;
	int reg = offset & 0x1f;
	int oldreg = info->regs[offset];
	int vbase = offset & ~0x1f;

	info->regs[offset] = data;

	if ((voice < 8) && (data != oldreg))
	{
		if ((reg == 0x16) && (data != 0))
		{
			info->vpos[voice] = info->frac[voice] = info->lponce[voice] = 0;

			LOG(("Key on V%02d: st %06x-%06x lp %06x-%06x frq %x flg %x\n", voice,
				info->regs[vbase+2]<<16 | info->regs[vbase+1]<<8 | info->regs[vbase+2],
				info->regs[vbase+0xe]<<16 | info->regs[vbase+0xd]<<8 | info->regs[vbase+0xc],
				info->regs[vbase+6]<<16 | info->regs[vbase+5]<<8 | info->regs[vbase+4],
				info->regs[vbase+0xa]<<16 | info->regs[vbase+0x9]<<8 | info->regs[vbase+0x8],
				info->regs[vbase+0x11]<<8 | info->regs[vbase+0x10],
				info->regs[vbase+0x16]));
		}
	}
}

static STREAM_UPDATE( st0016_update )
{
	st0016_state *info = (st0016_state *)param;
	UINT8 *sound_ram = *info->sound_ram;
	int v, i, snum;
	unsigned char *slot;
	INT32 mix[48000*2];
	INT32 *mixp;
	INT16 sample;
	int sptr, eptr, freq, lsptr, leptr;

	memset(mix, 0, sizeof(mix[0])*samples*2);

	for (v = 0; v < 8; v++)
	{
		slot = (unsigned char *)&info->regs[v * 32];

		if (slot[0x16] & 0x06)
		{
			mixp = &mix[0];

			sptr = slot[0x02]<<16 | slot[0x01]<<8 | slot[0x00];
			eptr = slot[0x0e]<<16 | slot[0x0d]<<8 | slot[0x0c];
			freq = slot[0x11]<<8 | slot[0x10];
			lsptr = slot[0x06]<<16 | slot[0x05]<<8 | slot[0x04];
			leptr = slot[0x0a]<<16 | slot[0x09]<<8 | slot[0x08];

			for (snum = 0; snum < samples; snum++)
			{
				sample = sound_ram[(sptr + info->vpos[v])&0x1fffff]<<8;

				*mixp++ += (sample * (char)slot[0x14]) >> 8;
				*mixp++ += (sample * (char)slot[0x15]) >> 8;

				info->frac[v] += freq;
				info->vpos[v] += (info->frac[v]>>16);
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
						if (slot[0x16] & 0x01)	// loop?
						{
							info->vpos[v] = (lsptr - sptr);
							info->lponce[v] = 1;
						}
						else
						{
							slot[0x16] = 0;
							info->vpos[v] = info->frac[v] = 0;
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

static DEVICE_START( st0016 )
{
	const st0016_interface *intf = (const st0016_interface *)device->baseconfig().static_config();
	st0016_state *info = get_safe_token(device);

	info->sound_ram = intf->p_soundram;

	info->stream = stream_create(device, 0, 2, 44100, info, st0016_update);
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

DEVICE_GET_INFO( st0016 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(st0016_state); 			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( st0016 );			break;
		case DEVINFO_FCT_STOP:							/* Nothing */									break;
		case DEVINFO_FCT_RESET:							/* Nothing */									break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "ST0016");						break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Seta custom");					break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}


DEFINE_LEGACY_SOUND_DEVICE(ST0016, st0016);
