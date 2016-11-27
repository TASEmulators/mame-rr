/***************************************************************************

  sn76496.c
  by Nicola Salmoria

  Routines to emulate the:
  Texas Instruments SN76489, SN76489A, SN76494/SN76496
  ( Also known as, or at least compatible with, the TMS9919 and SN94624.)
  and the Sega 'PSG' used on the Master System, Game Gear, and Megadrive/Genesis
  This chip is known as the Programmable Sound Generator, or PSG, and is a 4
  channel sound generator, with three squarewave channels and a noise/arbitrary
  duty cycle channel.

  Noise emulation for all verified chips should be accurate:

  ** SN76489 uses a 15-bit shift register with taps on bits D and E, output on E,
  XOR function.
  It uses a 15-bit ring buffer for periodic noise/arbitrary duty cycle.
  Its output is inverted.
  ** SN94624 is the same as SN76489 but lacks the /8 divider on its clock input.
  ** SN76489A uses a 15-bit shift register with taps on bits D and E, output on F,
  XOR function.
  It uses a 15-bit ring buffer for periodic noise/arbitrary duty cycle.
  Its output is not inverted.
  ** SN76494 is the same as SN76489A but lacks the /8 divider on its clock input.
  ** SN76496 is identical in operation to the SN76489A, but the audio input is
  documented.
  All the SN7xxxx chips have an audio input line which is mixed with the 4 channels
  of output. (It is undocumented and may not function properly on the sn76489, 76489a
  and 76494; the sn76489a input is mentioned in datasheets for the tms5200)
  ** Sega Master System III/MD/Genesis PSG uses a 16-bit shift register with taps
  on bits C and F, output on F
  It uses a 16-bit ring buffer for periodic noise/arbitrary duty cycle.
  (whether it uses an XOR or XNOR needs to be verified, assumed XOR)
  (whether output is inverted or not needs to be verified, assumed to be inverted)
  ** Sega Game Gear PSG is identical to the SMS3/MD/Genesis one except it has an
  extra register for mapping which channels go to which speaker.
  The register, connected to a z80 port, means:
  for bits 7  6  5  4  3  2  1  0
           L3 L2 L1 L0 R3 R2 R1 R0
  Noise is an XOR function, and audio output is negated before being output.
  ** NCR7496 (as used on the Tandy 1000) is similar to the SN76489 but with a
  different noise LFSR patttern: taps on bits A and E, output on E
  It uses a 15-bit ring buffer for periodic noise/arbitrary duty cycle.
  (all this chip's info needs to be verified)

  28/03/2005 : Sebastien Chevalier
  Update th SN76496Write func, according to SN76489 doc found on SMSPower.
   - On write with 0x80 set to 0, when LastRegister is other then TONE,
   the function is similar than update with 0x80 set to 1

  23/04/2007 : Lord Nightmare
  Major update, implement all three different noise generation algorithms and a
  set_variant call to discern among them.

  28/04/2009 : Lord Nightmare
  Add READY line readback; cleaned up struct a bit. Cleaned up comments.
  Add more TODOs. Fixed some unsaved savestate related stuff.

  04/11/2009 : Lord Nightmare
  Changed the way that the invert works (it now selects between XOR and XNOR
  for the taps), and added R->OldNoise to simulate the extra 0 that is always
  output before the noise LFSR contents are after an LFSR reset.
  This fixes SN76489/A to match chips. Added SN94624.

  14/11/2009 : Lord Nightmare
  Removed STEP mess, vastly simplifying the code. Made output bipolar rather
  than always above the 0 line, but disabled that code due to pending issues.

  16/11/2009 : Lord Nightmare
  Fix screeching in regulus: When summing together four equal channels, the
  size of the max amplitude per channel should be 1/4 of the max range, not
  1/3. Added NCR7496.

  18/11/2009 : Lord Nightmare
  Modify Init functions to support negating the audio output. The gamegear
  psg does this. Change gamegear and sega psgs to use XOR rather than XNOR
  based on testing. Got rid of R->OldNoise and fixed taps accordingly.
  Added stereo support for game gear.

  15/01/2010 : Lord Nightmare
  Fix an issue with SN76489 and SN76489A having the wrong periodic noise periods.
  Note that properly emulating the noise cycle bit timing accurately may require
  extensive rewriting.

  24/01/2010: Lord Nightmare
  Implement periodic noise as forcing one of the XNOR or XOR taps to 1 or 0 respectively.
  Thanks to PlgDavid for providing samples which helped immensely here.
  Added true clock divider emulation, so sn94624 and sn76494 run 8x faster than
  the others, as in real life.

  15/02/2010: Lord Nightmare & Michael Zapf (additional testing by PlgDavid)
  Fix noise period when set to mirror channel 3 and channel 3 period is set to 0 (tested on hardware for noise, wave needs tests) - MZ
  Fix phase of noise on sn94624 and sn76489; all chips use a standard XOR, the only inversion is the output itself - LN, Plgdavid
  Thanks to PlgDavid and Michael Zapf for providing samples which helped immensely here.

  TODO: * Implement the TMS9919 - any difference to sn94624?
        * Implement the T6W28; has registers in a weird order, needs writes
          to be 'sanitized' first. Also is stereo, similar to game gear.
        * Test the NCR7496; Smspower says the whitenoise taps are A and E,
          but this needs verification on real hardware.
        * Factor out common code so that the SAA1099 can share some code.
***************************************************************************/

#include "emu.h"
#include "streams.h"
#include "sn76496.h"


#define MAX_OUTPUT 0x7fff
#define NOISEMODE (R->Register[6]&4)?1:0


typedef struct _sn76496_state sn76496_state;
struct _sn76496_state
{
	sound_stream * Channel;
	INT32 VolTable[16];	/* volume table (for 4-bit to db conversion)*/
	INT32 Register[8];	/* registers */
	INT32 LastRegister;	/* last register written */
	INT32 Volume[4];	/* db volume of voice 0-2 and noise */
	UINT32 RNG;			/* noise generator LFSR*/
	INT32 ClockDivider;	/* clock divider */
	INT32 CurrentClock;
	INT32 FeedbackMask;	/* mask for feedback */
	INT32 WhitenoiseTap1;	/* mask for white noise tap 1 (higher one, usually bit 14) */
	INT32 WhitenoiseTap2;	/* mask for white noise tap 2 (lower one, usually bit 13)*/
	INT32 Negate;		/* output negate flag */
	INT32 Stereo;		/* whether we're dealing with stereo or not */
	INT32 StereoMask;	/* the stereo output mask */
	INT32 Period[4];	/* Length of 1/2 of waveform */
	INT32 Count[4];		/* Position within the waveform */
	INT32 Output[4];	/* 1-bit output of each channel, pre-volume */
	INT32 CyclestoREADY;/* number of cycles until the READY line goes active */
};


INLINE sn76496_state *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->type() == SOUND_SN76496 ||
		   device->type() == SOUND_SN76489 ||
		   device->type() == SOUND_SN76489A ||
		   device->type() == SOUND_SN76494 ||
		   device->type() == SOUND_SN94624 ||
		   device->type() == SOUND_NCR7496 ||
		   device->type() == SOUND_GAMEGEAR ||
		   device->type() == SOUND_SMSIII);
	return (sn76496_state *)downcast<legacy_device_base *>(device)->token();
}

READ_LINE_DEVICE_HANDLER( sn76496_ready_r )
{
	sn76496_state *R = get_safe_token(device);
	stream_update(R->Channel);
	return (R->CyclestoREADY? 0 : 1);
}

WRITE8_DEVICE_HANDLER( sn76496_stereo_w )
{
	sn76496_state *R = get_safe_token(device);
	stream_update(R->Channel);
	if (R->Stereo) R->StereoMask = data;
	else fatalerror("Call to stereo write with mono chip!\n");
}

WRITE8_DEVICE_HANDLER( sn76496_w )
{
	sn76496_state *R = get_safe_token(device);
	int n, r, c;


	/* update the output buffer before changing the registers */
	stream_update(R->Channel);

	/* set number of cycles until READY is active; this is always one
           'sample', i.e. it equals the clock divider exactly; until the
           clock divider is fully supported, we delay until one sample has
           played. The fact that this below is '2' and not '1' is because
           of a ?race condition? in the mess crvision driver, where after
           any sample is played at all, no matter what, the cycles_to_ready
           ends up never being not ready, unless this value is greater than
           1. Once the full clock divider stuff is written, this should no
           longer be an issue. */
	R->CyclestoREADY = 2;

	if (data & 0x80)
	{
		r = (data & 0x70) >> 4;
		R->LastRegister = r;
		R->Register[r] = (R->Register[r] & 0x3f0) | (data & 0x0f);
	}
	else
    {
		r = R->LastRegister;
	}
	c = r/2;
	switch (r)
	{
		case 0:	/* tone 0 : frequency */
		case 2:	/* tone 1 : frequency */
		case 4:	/* tone 2 : frequency */
		    if ((data & 0x80) == 0) R->Register[r] = (R->Register[r] & 0x0f) | ((data & 0x3f) << 4);
			if (R->Register[r] != 0) R->Period[c] = R->Register[r];
			else R->Period[c] = 0x400;
			if (r == 4)
			{
				/* update noise shift frequency */
				if ((R->Register[6] & 0x03) == 0x03)
					R->Period[3] = 2 * R->Period[2];
			}
			break;
		case 1:	/* tone 0 : volume */
		case 3:	/* tone 1 : volume */
		case 5:	/* tone 2 : volume */
		case 7:	/* noise  : volume */
			R->Volume[c] = R->VolTable[data & 0x0f];
			if ((data & 0x80) == 0) R->Register[r] = (R->Register[r] & 0x3f0) | (data & 0x0f);
			break;
		case 6:	/* noise  : frequency, mode */
			{
				if ((data & 0x80) == 0) logerror("sn76489: write to reg 6 with bit 7 clear; data was %03x, new write is %02x! report this to LN!\n", R->Register[6], data);
				if ((data & 0x80) == 0) R->Register[r] = (R->Register[r] & 0x3f0) | (data & 0x0f);
				n = R->Register[6];
				/* N/512,N/1024,N/2048,Tone #3 output */
				R->Period[3] = ((n&3) == 3) ? 2 * R->Period[2] : (1 << (5+(n&3)));
				R->RNG = R->FeedbackMask;
			}
			break;
	}
}

static STREAM_UPDATE( SN76496Update )
{
	int i;
	sn76496_state *R = (sn76496_state *)param;
	stream_sample_t *lbuffer = outputs[0];
	stream_sample_t *rbuffer = (R->Stereo)?outputs[1]:NULL;
	INT16 out = 0;
	INT16 out2 = 0;

	while (samples > 0)
	{
		// clock chip once
		if (R->CurrentClock > 0) // not ready for new divided clock
		{
			R->CurrentClock--;
		}
		else // ready for new divided clock, make a new sample
		{
			R->CurrentClock = R->ClockDivider-1;
			/* decrement Cycles to READY by one */
			if (R->CyclestoREADY >0) R->CyclestoREADY--;

			// handle channels 0,1,2
			for (i = 0;i < 3;i++)
			{
				R->Count[i]--;
				if (R->Count[i] <= 0)
				{
					R->Output[i] ^= 1;
					R->Count[i] = R->Period[i];
				}
			}

			// handle channel 3
			R->Count[3]--;
			if (R->Count[3] <= 0)
			{
			// if noisemode is 1, both taps are enabled
			// if noisemode is 0, the lower tap, whitenoisetap2, is held at 0
				if (((R->RNG & R->WhitenoiseTap1)?1:0) ^ ((((R->RNG & R->WhitenoiseTap2)?1:0))*(NOISEMODE)))
				{
					R->RNG >>= 1;
					R->RNG |= R->FeedbackMask;
				}
				else
				{
					R->RNG >>= 1;
				}
				R->Output[3] = R->RNG & 1;

				R->Count[3] = R->Period[3];
			}
		}


		if (R->Stereo)
		{
			out = (((R->StereoMask&0x10)&&R->Output[0])?R->Volume[0]:0)
				+ (((R->StereoMask&0x20)&&R->Output[1])?R->Volume[1]:0)
				+ (((R->StereoMask&0x40)&&R->Output[2])?R->Volume[2]:0)
				+ (((R->StereoMask&0x80)&&R->Output[3])?R->Volume[3]:0);

			out2 = (((R->StereoMask&0x1)&&R->Output[0])?R->Volume[0]:0)
				+ (((R->StereoMask&0x2)&&R->Output[1])?R->Volume[1]:0)
				+ (((R->StereoMask&0x4)&&R->Output[2])?R->Volume[2]:0)
				+ (((R->StereoMask&0x8)&&R->Output[3])?R->Volume[3]:0);
		}
		else
		{
			out = (R->Output[0]?R->Volume[0]:0)
				+(R->Output[1]?R->Volume[1]:0)
				+(R->Output[2]?R->Volume[2]:0)
				+(R->Output[3]?R->Volume[3]:0);
		}

		if(R->Negate) { out = -out; out2 = -out2; }

		*(lbuffer++) = out;
		if (R->Stereo) *(rbuffer++) = out2;
		samples--;
	}
}



static void SN76496_set_gain(sn76496_state *R,int gain)
{
	int i;
	double out;


	gain &= 0xff;

	/* increase max output basing on gain (0.2 dB per step) */
	out = MAX_OUTPUT / 4; // four channels, each gets 1/4 of the total range
	while (gain-- > 0)
		out *= 1.023292992;	/* = (10 ^ (0.2/20)) */

	/* build volume table (2dB per step) */
	for (i = 0;i < 15;i++)
	{
		/* limit volume to avoid clipping */
		if (out > MAX_OUTPUT / 4) R->VolTable[i] = MAX_OUTPUT / 4;
		else R->VolTable[i] = out;

		out /= 1.258925412;	/* = 10 ^ (2/20) = 2dB */
	}
	R->VolTable[15] = 0;
}



static int SN76496_init(running_device *device, sn76496_state *R, int stereo)
{
	int sample_rate = device->clock()/2;
	int i;

	R->Channel = stream_create(device,0,(stereo?2:1),sample_rate,R,SN76496Update);

	for (i = 0;i < 4;i++) R->Volume[i] = 0;

	R->LastRegister = 0;
	for (i = 0;i < 8;i+=2)
	{
		R->Register[i] = 0;
		R->Register[i + 1] = 0x0f;	/* volume = 0 */
	}

	for (i = 0;i < 4;i++)
	{
		R->Output[i] = R->Period[i] = R->Count[i] = 0;
	}

	/* Default is SN76489A */
	R->ClockDivider = 8;
	R->FeedbackMask = 0x10000;     /* mask for feedback */
	R->WhitenoiseTap1 = 0x04;   /* mask for white noise tap 1*/
	R->WhitenoiseTap2 = 0x08;   /* mask for white noise tap 2*/
	R->Negate = 0; /* channels are not negated */
	R->Stereo = stereo; /* depends on init */
	R->CyclestoREADY = 1; /* assume ready is not active immediately on init. is this correct?*/
	R->StereoMask = 0xFF; /* all channels enabled */

	R->RNG = R->FeedbackMask;
	R->Output[3] = R->RNG & 1;

	return 0;
}


static void generic_start(running_device *device, int feedbackmask, int noisetap1, int noisetap2, int negate, int stereo, int clockdivider)
{
	sn76496_state *chip = get_safe_token(device);

	if (SN76496_init(device,chip,stereo) != 0)
		fatalerror("Error creating SN76496 chip");
	SN76496_set_gain(chip, 0);

	chip->FeedbackMask = feedbackmask;
	chip->WhitenoiseTap1 = noisetap1;
	chip->WhitenoiseTap2 = noisetap2;
	chip->Negate = negate;
	chip->Stereo = stereo;
	chip->ClockDivider = clockdivider;
	chip->CurrentClock = clockdivider-1;

	state_save_register_device_item_array(device, 0, chip->VolTable);
	state_save_register_device_item_array(device, 0, chip->Register);
	state_save_register_device_item(device, 0, chip->LastRegister);
	state_save_register_device_item_array(device, 0, chip->Volume);
	state_save_register_device_item(device, 0, chip->RNG);
	state_save_register_device_item(device, 0, chip->ClockDivider);
	state_save_register_device_item(device, 0, chip->CurrentClock);
	state_save_register_device_item(device, 0, chip->FeedbackMask);
	state_save_register_device_item(device, 0, chip->WhitenoiseTap1);
	state_save_register_device_item(device, 0, chip->WhitenoiseTap2);
	state_save_register_device_item(device, 0, chip->Negate);
	state_save_register_device_item(device, 0, chip->Stereo);
	state_save_register_device_item(device, 0, chip->StereoMask);
	state_save_register_device_item_array(device, 0, chip->Period);
	state_save_register_device_item_array(device, 0, chip->Count);
	state_save_register_device_item_array(device, 0, chip->Output);
	state_save_register_device_item(device, 0, chip->CyclestoREADY);
}

// function parameters: device, feedback destination tap, feedback source taps,
// normal(false)/invert(true), mono(false)/stereo(true), clock divider factor

static DEVICE_START( sn76489 )
{
	generic_start(device, 0x4000, 0x01, 0x02, TRUE, FALSE, 8); // SN76489 not verified yet. todo: verify;
}

static DEVICE_START( sn76489a )
{
	generic_start(device, 0x10000, 0x04, 0x08, FALSE, FALSE, 8); // SN76489A: whitenoise verified, phase verified, periodic verified (by plgdavid)
}

static DEVICE_START( sn76494 )
{
	generic_start(device, 0x10000, 0x04, 0x08, FALSE, FALSE, 1); // SN76494 not verified, (according to datasheet: same as sn76489a but without the /8 divider)
}

static DEVICE_START( sn76496 )
{
	generic_start(device, 0x10000, 0x04, 0x08, FALSE, FALSE, 8); // SN76496: Whitenoise verified, phase verified, periodic verified (by Michael Zapf)
}

static DEVICE_START( sn94624 )
{
	generic_start(device, 0x4000, 0x01, 0x02, TRUE, FALSE, 1); // SN94624 whitenoise verified, phase verified, period verified; verified by PlgDavid
}

static DEVICE_START( ncr7496 )
{
	generic_start(device, 0x8000, 0x02, 0x20, FALSE, FALSE, 8); // NCR7496 not verified; info from smspower wiki
}

static DEVICE_START( gamegear )
{
	generic_start(device, 0x8000, 0x01, 0x08, TRUE, TRUE, 8); // Verified by Justin Kerk
}

static DEVICE_START( smsiii )
{
	generic_start(device, 0x8000, 0x01, 0x08, TRUE, FALSE, 8); // todo: verify; from smspower wiki, assumed to have same invert as gamegear
}


/**************************************************************************
 * Generic get_info
 **************************************************************************/

DEVICE_GET_INFO( sn76496 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(sn76496_state);				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( sn76496 );		break;
		case DEVINFO_FCT_STOP:							/* Nothing */									break;
		case DEVINFO_FCT_RESET:							/* Nothing */									break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "SN76496");						break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "TI PSG");						break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.1");							break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}

DEVICE_GET_INFO( sn76489 )
{
	switch (state)
	{
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( sn76489 );		break;
		case DEVINFO_STR_NAME:							strcpy(info->s, "SN76489");						break;
		default:										DEVICE_GET_INFO_CALL(sn76496);						break;
	}
}

DEVICE_GET_INFO( sn76489a )
{
	switch (state)
	{
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( sn76489a );		break;
		case DEVINFO_STR_NAME:							strcpy(info->s, "SN76489A");					break;
		default:										DEVICE_GET_INFO_CALL(sn76496);						break;
	}
}

DEVICE_GET_INFO( sn76494 )
{
	switch (state)
	{
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( sn76494 );		break;
		case DEVINFO_STR_NAME:							strcpy(info->s, "SN76494");						break;
		default:										DEVICE_GET_INFO_CALL(sn76496);						break;
	}
}

DEVICE_GET_INFO( sn94624 )
{
	switch (state)
	{
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( sn94624 );		break;
		case DEVINFO_STR_NAME:							strcpy(info->s, "SN94624");						break;
		default:										DEVICE_GET_INFO_CALL(sn76496);						break;
	}
}

DEVICE_GET_INFO( ncr7496 )
{
	switch (state)
	{
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( ncr7496 );		break;
		case DEVINFO_STR_NAME:							strcpy(info->s, "NCR7496");						break;
		default:										DEVICE_GET_INFO_CALL(sn76496);						break;
	}
}

DEVICE_GET_INFO( gamegear )
{
	switch (state)
	{
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( gamegear );		break;
		case DEVINFO_STR_NAME:							strcpy(info->s, "Game Gear PSG");				break;
		default:										DEVICE_GET_INFO_CALL(sn76496);						break;
	}
}

DEVICE_GET_INFO( smsiii )
{
	switch (state)
	{
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( smsiii );			break;
		case DEVINFO_STR_NAME:							strcpy(info->s, "SMSIII PSG");					break;
		default:										DEVICE_GET_INFO_CALL(sn76496);						break;
	}
}


DEFINE_LEGACY_SOUND_DEVICE(SN76496, sn76496);
DEFINE_LEGACY_SOUND_DEVICE(SN76489, sn76489);
DEFINE_LEGACY_SOUND_DEVICE(SN76489A, sn76489a);
DEFINE_LEGACY_SOUND_DEVICE(SN76494, sn76494);
DEFINE_LEGACY_SOUND_DEVICE(SN94624, sn94624);
DEFINE_LEGACY_SOUND_DEVICE(NCR7496, ncr7496);
DEFINE_LEGACY_SOUND_DEVICE(GAMEGEAR, gamegear);
DEFINE_LEGACY_SOUND_DEVICE(SMSIII, smsiii);
