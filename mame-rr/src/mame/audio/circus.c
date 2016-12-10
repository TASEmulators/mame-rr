#include "emu.h"
#include "sound/samples.h"
#include "includes/circus.h"

static const char *const circus_sample_names[] =
{
	"*circus",
	"pop.wav",
	"miss.wav",
	"bounce.wav",
	0
};

const samples_interface circus_samples_interface =
{
	3,	/* 3 channels */
	circus_sample_names
};

static const char *const crash_sample_names[] =
{
	"*crash",
	"crash.wav",
	0
};

const samples_interface crash_samples_interface =
{
	1,	/* 1 channel */
	crash_sample_names
};

static const char *const ripcord_sample_names[] =
{
	"*ripcord",
	"splash.wav",
	"scream.wav",
	"chute.wav",
	"whistle.wav",
	0
};

const samples_interface ripcord_samples_interface =
{
	4,	/* 4 channels */
	ripcord_sample_names
};

static const char *const robotbwl_sample_names[] =
{
	"*robotbwl",
	"hit.wav",
	"roll.wav",
	"balldrop.wav",
	"demerit.wav",
	"reward.wav",
	0
};

const samples_interface robotbwl_samples_interface =
{
	5,	/* 5 channels */
	robotbwl_sample_names
};

/* Nodes - Inputs */
#define CIRCUS_MUSIC_BIT	NODE_01
/* Nodes - Sounds */
#define CIRCUS_MUSIC_SND	NODE_10

DISCRETE_SOUND_START(circus)
	/************************************************/
	/* Input register mapping for circus            */
	/************************************************/
	DISCRETE_INPUTX_NOT(CIRCUS_MUSIC_BIT,    20000,  0,      1)

	/************************************************/
	/* Music is just a 1 bit DAC                    */
	/************************************************/
	DISCRETE_CRFILTER(CIRCUS_MUSIC_SND, CIRCUS_MUSIC_BIT, RES_K(50), CAP_U(.1))	/* 50K is just an average value */

	DISCRETE_OUTPUT(CIRCUS_MUSIC_SND, 1)
DISCRETE_SOUND_END

static const discrete_mixer_desc crash_mixer =
{
	DISC_MIXER_IS_RESISTOR,
	{RES_K(22), RES_K(5)},
	{0},
	{CAP_U(.1), CAP_U(.1)},
	0, RES_K(100), 0, CAP_U(.1), 0, 10000
};

static const discrete_555_desc crash_beeper_555m =
{
	DISC_555_OUT_SQW | DISC_555_OUT_DC | DISC_555_TRIGGER_IS_LOGIC,
	5,		// B+ voltage of 555
	DEFAULT_555_VALUES
};

static const discrete_555_desc crash_beeper_555a =
{
	DISC_555_OUT_SQW | DISC_555_OUT_DC,
	5,		// B+ voltage of 555
	DEFAULT_555_VALUES
};

/* Nodes - Inputs */
#define CRASH_MUSIC_BIT		NODE_01
#define CRASH_BEEPER_EN		NODE_02
/* Nodes - Adjusters */
#define CRASH_R63			NODE_10
#define CRASH_R39			NODE_11
/* Nodes - Sounds */
#define CRASH_MUSIC_SND		NODE_20
#define CRASH_BEEPER_SND	NODE_21

DISCRETE_SOUND_START(crash)
	/************************************************/
	/* Input register mapping for crash             */
	/************************************************/
	DISCRETE_INPUT_LOGIC(CRASH_MUSIC_BIT)
	DISCRETE_INPUT_PULSE(CRASH_BEEPER_EN, 1)

	DISCRETE_ADJUSTMENT(CRASH_R63, 0, 5.0*RES_K(100)/(RES_K(47+100))-0.5, DISC_LINADJ, "R63")
	DISCRETE_ADJUSTMENT(CRASH_R39, 0, 1, DISC_LINADJ, "R39")

	/************************************************/
	/* Music is just a 1 bit DAC                    */
	/************************************************/
	DISCRETE_MULTADD(CRASH_MUSIC_SND, CRASH_MUSIC_BIT, CRASH_R63, 0.5)

	/************************************************/
	/* Beeper - oneshot gates tone                  */
	/************************************************/
	DISCRETE_555_MSTABLE(NODE_30, 1, CRASH_BEEPER_EN, RES_K(22), CAP_U(.47), &crash_beeper_555m)
	DISCRETE_555_ASTABLE(NODE_31, NODE_30, RES_K(4.7), RES_K(4.7), CAP_U(.1), &crash_beeper_555a)
	DISCRETE_MULTIPLY(CRASH_BEEPER_SND, NODE_31, CRASH_R39)

	/************************************************/
	/* Final mix with gain                          */
	/************************************************/
	DISCRETE_MIXER2(NODE_90, 1, CRASH_MUSIC_SND, CRASH_BEEPER_SND, &crash_mixer)

	DISCRETE_OUTPUT(NODE_90, 1)
DISCRETE_SOUND_END

/* Nodes - Inputs */
#define ROBOTBWL_MUSIC_BIT		NODE_01
/* Nodes - Sounds */
#define ROBOTBWL_MUSIC_SND		NODE_10

DISCRETE_SOUND_START(robotbwl)
	/************************************************/
	/* Input register mapping for robotbwl          */
	/************************************************/
	DISCRETE_INPUTX_LOGIC(ROBOTBWL_MUSIC_BIT,    30000,  0,      0)

	/************************************************/
	/* Music is just a 1 bit DAC                    */
	/************************************************/
	DISCRETE_RCFILTER(NODE_20, ROBOTBWL_MUSIC_BIT, RES_K(10), CAP_U(.47))
	DISCRETE_CRFILTER(ROBOTBWL_MUSIC_SND, NODE_20, RES_K(10) + RES_K(22), CAP_U(.1))

	DISCRETE_OUTPUT(ROBOTBWL_MUSIC_SND, 1)
DISCRETE_SOUND_END


/* This register controls the clown image currently displayed */
/* and also is used to enable the amplifier and trigger the   */
/* discrete circuitry that produces sound effects and music   */

WRITE8_HANDLER( circus_clown_z_w )
{
	circus_state *state = (circus_state *)space->machine->driver_data;

	state->clown_z = (data & 0x0f);
	*(memory_region(space->machine, "maincpu") + 0x8000) = data; logerror("Z:%02x\n",data); //DEBUG

	/* Bits 4-6 enable/disable trigger different events */
	switch (state->game_id)
	{
		case 1:	/* circus */
		case 4:	/* ripcord */
			switch ((data & 0x70) >> 4)
			{
				case 0 : /* All Off */
					discrete_sound_w(state->discrete, CIRCUS_MUSIC_BIT, 0);
					break;

				case 1 : /* Music */
					discrete_sound_w(state->discrete, CIRCUS_MUSIC_BIT, 1);
					break;

				case 2 : /* Circus = Pop; Rip Cord = Splash */
					sample_start(state->samples, 0, 0, 0);
					break;

				case 3 : /* Normal Video */
					break;

				case 4 : /* Circus = Miss; Rip Cord = Scream */
					sample_start(state->samples, 1, 1, 0);
					break;

				case 5 : /* Invert Video */
					break;

				case 6 : /* Circus = Bounce; Rip Cord = Chute Open */
					sample_start(state->samples, 2, 2, 0);
					break;

				case 7 : /* Circus = not used; Rip Cord = Whistle */
					if GAME_IS_RIPCORD
						sample_start(state->samples, 3, 3, 0);
					break;
			}
			break;

		case 2:	/* robotbwl */
			discrete_sound_w(state->discrete, ROBOTBWL_MUSIC_BIT, data & 0x08);	/* Footsteps */

			if (data & 0x40)	/* Hit */
				sample_start(state->samples, 0, 0, 0);

			if (data & 0x20)	/* Roll */
				sample_start(state->samples, 1, 1, 0);

			if (data & 0x10)	/* Ball Drop */
				sample_start(state->samples, 2, 2, 0);

			if (data & 0x02)	/* Demerit */
				sample_start(state->samples, 3, 3, 0);

			if (data & 0x01)	/* Reward */
				sample_start(state->samples, 4, 4, 0);

			// if (data & 0x04) /* Invert */
			break;

		case 3:	/* crash */
			/* Only the crash can be done with a sample */
			switch ((data & 0x70) >> 4)
			{
				case 0 : /* All Off */
					discrete_sound_w(state->discrete, CRASH_MUSIC_BIT, 0);
					break;

				case 1 : /* Music */
					discrete_sound_w(state->discrete, CRASH_MUSIC_BIT, 1);
					break;

				case 2 : /* Crash */
					sample_start(state->samples, 0, 0, 0);
					break;

				case 3 : /* Normal Video and Beep */
					discrete_sound_w(state->discrete, CRASH_BEEPER_EN, 0);
					break;

				case 4 : /* Skid */
					break;

				case 5 : /* Invert Video and Beep */
					discrete_sound_w(state->discrete, CRASH_BEEPER_EN, 0);
					break;

				case 6 : /* Hi Motor */
					break;

				case 7 : /* Low Motor */
					break;
			}
			break;
	}

	/* Bit 7 enables amplifier (0 = on) */
	sound_global_enable(space->machine, ~data & 0x80);
}
