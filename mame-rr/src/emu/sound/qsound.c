/***************************************************************************

  Capcom System QSound(tm)
  ========================

  Driver by Paul Leaman (paul@vortexcomputing.demon.co.uk)
        and Miguel Angel Horna (mahorna@teleline.es)

  A 16 channel stereo sample player.

  QSpace position is simulated by panning the sound in the stereo space.

  Register
  0  xxbb   xx = unknown bb = start high address
  1  ssss   ssss = sample start address
  2  pitch
  3  unknown (always 0x8000)
  4  loop offset from end address
  5  end
  6  master channel volume
  7  not used
  8  Balance (left=0x0110  centre=0x0120 right=0x0130)
  9  unknown (most fixed samples use 0 for this register)

  Many thanks to CAB (the author of Amuse), without whom this probably would
  never have been finished.

  If anybody has some information about this hardware, please send it to me
  to mahorna@teleline.es or 432937@cepsz.unizar.es.
  http://teleline.terra.es/personal/mahorna

***************************************************************************/

#include "emu.h"
#include "streams.h"
#include "qsound.h"

/*
Debug defines
*/
#define LOG_WAVE	0
#define VERBOSE  0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

/* 8 bit source ROM samples */
typedef INT8 QSOUND_SRC_SAMPLE;


#define QSOUND_CLOCKDIV 166			 /* Clock divider */
#define QSOUND_CHANNELS 16
typedef stream_sample_t QSOUND_SAMPLE;

struct QSOUND_CHANNEL
{
	INT32 bank;	   /* bank (x16)    */
	INT32 address;	/* start address */
	INT32 pitch;	  /* pitch */
	INT32 reg3;	   /* unknown (always 0x8000) */
	INT32 loop;	   /* loop address */
	INT32 end;		/* end address */
	INT32 vol;		/* master volume */
	INT32 pan;		/* Pan value */
	INT32 reg9;	   /* unknown */

	/* Work variables */
	INT32 key;		/* Key on / key off */

	INT32 lvol;	   /* left volume */
	INT32 rvol;	   /* right volume */
	INT32 lastdt;	 /* last sample value */
	INT32 offset;	 /* current offset counter */
};

typedef struct _qsound_state qsound_state;
struct _qsound_state
{
	/* Private variables */
	sound_stream * stream;				/* Audio stream */
	struct QSOUND_CHANNEL channel[QSOUND_CHANNELS];
	int data;				  /* register latch data */
	QSOUND_SRC_SAMPLE *sample_rom;	/* Q sound sample ROM */
	UINT32 sample_rom_length;

	int pan_table[33];		 /* Pan volume table */
	float frq_ratio;		   /* Frequency ratio */

	FILE *fpRawDataL;
	FILE *fpRawDataR;
};

INLINE qsound_state *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->type() == SOUND_QSOUND);
	return (qsound_state *)downcast<legacy_device_base *>(device)->token();
}


/* Function prototypes */
static STREAM_UPDATE( qsound_update );
static void qsound_set_command(qsound_state *chip, int data, int value);

static DEVICE_START( qsound )
{
	qsound_state *chip = get_safe_token(device);
	int i;

	chip->sample_rom = (QSOUND_SRC_SAMPLE *)*device->region();
	chip->sample_rom_length = device->region()->bytes();

	memset(chip->channel, 0, sizeof(chip->channel));

	chip->frq_ratio = 16.0;

	/* Create pan table */
	for (i=0; i<33; i++)
	{
		chip->pan_table[i]=(int)((256/sqrt(32.0)) * sqrt((double)i));
	}

	LOG(("Pan table\n"));
	for (i=0; i<33; i++)
		LOG(("%02x ", chip->pan_table[i]));

	{
		/* Allocate stream */
		chip->stream = stream_create(
			device, 0, 2,
			device->clock() / QSOUND_CLOCKDIV,
			chip,
			qsound_update );
	}

	if (LOG_WAVE)
	{
		chip->fpRawDataR=fopen("qsoundr.raw", "w+b");
		chip->fpRawDataL=fopen("qsoundl.raw", "w+b");
	}

	/* state save */
	for (i=0; i<QSOUND_CHANNELS; i++)
	{
		state_save_register_device_item(device, i, chip->channel[i].bank);
		state_save_register_device_item(device, i, chip->channel[i].address);
		state_save_register_device_item(device, i, chip->channel[i].pitch);
		state_save_register_device_item(device, i, chip->channel[i].loop);
		state_save_register_device_item(device, i, chip->channel[i].end);
		state_save_register_device_item(device, i, chip->channel[i].vol);
		state_save_register_device_item(device, i, chip->channel[i].pan);
		state_save_register_device_item(device, i, chip->channel[i].key);
		state_save_register_device_item(device, i, chip->channel[i].lvol);
		state_save_register_device_item(device, i, chip->channel[i].rvol);
		state_save_register_device_item(device, i, chip->channel[i].lastdt);
		state_save_register_device_item(device, i, chip->channel[i].offset);
	}
}

static DEVICE_STOP( qsound )
{
	qsound_state *chip = get_safe_token(device);
	if (chip->fpRawDataR)
	{
		fclose(chip->fpRawDataR);
	}
	chip->fpRawDataR = NULL;
	if (chip->fpRawDataL)
	{
		fclose(chip->fpRawDataL);
	}
	chip->fpRawDataL = NULL;
}

WRITE8_DEVICE_HANDLER( qsound_w )
{
	qsound_state *chip = get_safe_token(device);
	switch (offset)
	{
		case 0:
			chip->data=(chip->data&0xff)|(data<<8);
			break;

		case 1:
			chip->data=(chip->data&0xff00)|data;
			break;

		case 2:
			qsound_set_command(chip, data, chip->data);
			break;

		default:
			logerror("%s: unexpected qsound write to offset %d == %02X\n", cpuexec_describe_context(device->machine), offset, data);
			break;
	}
}

READ8_DEVICE_HANDLER( qsound_r )
{
	/* Port ready bit (0x80 if ready) */
	return 0x80;
}

static void qsound_set_command(qsound_state *chip, int data, int value)
{
	int ch=0,reg=0;
	if (data < 0x80)
	{
		ch=data>>3;
		reg=data & 0x07;
	}
	else
	{
		if (data < 0x90)
		{
			ch=data-0x80;
			reg=8;
		}
		else
		{
			if (data >= 0xba && data < 0xca)
			{
				ch=data-0xba;
				reg=9;
			}
			else
			{
				/* Unknown registers */
				ch=99;
				reg=99;
			}
		}
	}

	switch (reg)
	{
		case 0: /* Bank */
			ch=(ch+1)&0x0f;	/* strange ... */
			chip->channel[ch].bank=(value&0x7f)<<16;
#ifdef MAME_DEBUG
			if (!(value & 0x8000))
				popmessage("Register3=%04x",value);
#endif

			break;
		case 1: /* start */
			chip->channel[ch].address=value;
			break;
		case 2: /* pitch */
			chip->channel[ch].pitch=value * 16;
			if (!value)
			{
				/* Key off */
				chip->channel[ch].key=0;
			}
			break;
		case 3: /* unknown */
			chip->channel[ch].reg3=value;
#ifdef MAME_DEBUG
			if (value != 0x8000)
				popmessage("Register3=%04x",value);
#endif
			break;
		case 4: /* loop offset */
			chip->channel[ch].loop=value;
			break;
		case 5: /* end */
			chip->channel[ch].end=value;
			break;
		case 6: /* master volume */
			if (value==0)
			{
				/* Key off */
				chip->channel[ch].key=0;
			}
			else if (chip->channel[ch].key==0)
			{
				/* Key on */
				chip->channel[ch].key=1;
				chip->channel[ch].offset=0;
				chip->channel[ch].lastdt=0;
			}
			chip->channel[ch].vol=value;
			break;

		case 7:  /* unused */
#ifdef MAME_DEBUG
				popmessage("UNUSED QSOUND REG 7=%04x",value);
#endif

			break;
		case 8:
			{
			   int pandata=(value-0x10)&0x3f;
			   if (pandata > 32)
			   {
					pandata=32;
			   }
			   chip->channel[ch].rvol=chip->pan_table[pandata];
			   chip->channel[ch].lvol=chip->pan_table[32-pandata];
			   chip->channel[ch].pan = value;
			}
			break;
		 case 9:
			chip->channel[ch].reg9=value;
/*
#ifdef MAME_DEBUG
            popmessage("QSOUND REG 9=%04x",value);
#endif
*/
			break;
	}
	LOG(("QSOUND WRITE %02x CH%02d-R%02d =%04x\n", data, ch, reg, value));
}


static STREAM_UPDATE( qsound_update )
{
	qsound_state *chip = (qsound_state *)param;
	int i,j;
	int rvol, lvol, count;
	struct QSOUND_CHANNEL *pC=&chip->channel[0];
	stream_sample_t  *datap[2];

	datap[0] = outputs[0];
	datap[1] = outputs[1];
	memset( datap[0], 0x00, samples * sizeof(*datap[0]) );
	memset( datap[1], 0x00, samples * sizeof(*datap[1]) );

	for (i=0; i<QSOUND_CHANNELS; i++)
	{
		if (pC->key)
		{
			QSOUND_SAMPLE *pOutL=datap[0];
			QSOUND_SAMPLE *pOutR=datap[1];
			rvol=(pC->rvol*pC->vol)>>8;
			lvol=(pC->lvol*pC->vol)>>8;

			for (j=samples-1; j>=0; j--)
			{
				count=(pC->offset)>>16;
				pC->offset &= 0xffff;
				if (count)
				{
					pC->address += count;
					if (pC->address >= pC->end)
					{
						if (!pC->loop)
						{
							/* Reached the end of a non-looped sample */
							pC->key=0;
							break;
						}
						/* Reached the end, restart the loop */
						pC->address = (pC->end - pC->loop) & 0xffff;
					}
					pC->lastdt=chip->sample_rom[(pC->bank+pC->address)%(chip->sample_rom_length)];
				}

				(*pOutL) += ((pC->lastdt * lvol) >> 6);
				(*pOutR) += ((pC->lastdt * rvol) >> 6);
				pOutL++;
				pOutR++;
				pC->offset += pC->pitch;
			}
		}
		pC++;
	}

	if (chip->fpRawDataL)
		fwrite(datap[0], samples*sizeof(QSOUND_SAMPLE), 1, chip->fpRawDataL);
	if (chip->fpRawDataR)
		fwrite(datap[1], samples*sizeof(QSOUND_SAMPLE), 1, chip->fpRawDataR);
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

DEVICE_GET_INFO( qsound )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(qsound_state);			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( qsound );			break;
		case DEVINFO_FCT_STOP:							info->stop = DEVICE_STOP_NAME( qsound );			break;
		case DEVINFO_FCT_RESET:							/* Nothing */									break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Q-Sound");						break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Capcom custom");				break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}

/**************** end of file ****************/

DEFINE_LEGACY_SOUND_DEVICE(QSOUND, qsound);
