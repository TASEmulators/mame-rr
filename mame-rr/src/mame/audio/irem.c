/***************************************************************************

    Irem M52/M62 sound hardware

***************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "sound/ay8910.h"
#include "sound/msm5205.h"
#include "sound/discrete.h"
#include "includes/iremz80.h"


/*************************************
 *
 *  Sound board initialization
 *
 *************************************/

static SOUND_START( irem_audio )
{
	irem_z80_state *state = (irem_z80_state *)machine->driver_data;

	state->adpcm1 = machine->device("msm1");
	state->adpcm2 = machine->device("msm2");
	state->ay1 = machine->device("ay1");
	state->ay2 = machine->device("ay2");

	state_save_register_global(machine, state->port1);
	state_save_register_global(machine, state->port2);
}



/*************************************
 *
 *  External writes to the sound
 *  command register
 *
 *************************************/

WRITE8_HANDLER( irem_sound_cmd_w )
{
	if ((data & 0x80) == 0)
		soundlatch_w(space, 0, data & 0x7f);
	else
		cputag_set_input_line(space->machine, "iremsound", 0, ASSERT_LINE);
}



/*************************************
 *
 *  6803 output ports
 *
 *************************************/

static WRITE8_HANDLER( m6803_port1_w )
{
	irem_z80_state *state = (irem_z80_state *)space->machine->driver_data;

	state->port1 = data;
}


static WRITE8_HANDLER( m6803_port2_w )
{
	irem_z80_state *state = (irem_z80_state *)space->machine->driver_data;

	/* write latch */
	if ((state->port2 & 0x01) && !(data & 0x01))
	{
		/* control or data port? */
		if (state->port2 & 0x04)
		{
			/* PSG 0 or 1? */
			if (state->port2 & 0x08)
				ay8910_address_w(state->ay1, 0, state->port1);
			if (state->port2 & 0x10)
				ay8910_address_w(state->ay2, 0, state->port1);
		}
		else
		{
			/* PSG 0 or 1? */
			if (state->port2 & 0x08)
				ay8910_data_w(state->ay1, 0, state->port1);
			if (state->port2 & 0x10)
				ay8910_data_w(state->ay2, 0, state->port1);
		}
	}
	state->port2 = data;
}



/*************************************
 *
 *  6803 input ports ports
 *
 *************************************/

static READ8_HANDLER( m6803_port1_r )
{
	irem_z80_state *state = (irem_z80_state *)space->machine->driver_data;

	/* PSG 0 or 1? */
	if (state->port2 & 0x08)
		return ay8910_r(state->ay1, 0);
	if (state->port2 & 0x10)
		return ay8910_r(state->ay2, 0);
	return 0xff;
}


static READ8_HANDLER( m6803_port2_r )
{
	return 0;
}



/*************************************
 *
 *  AY-8910 output ports
 *
 *************************************/

static WRITE8_DEVICE_HANDLER( ay8910_0_portb_w )
{
	irem_z80_state *state = (irem_z80_state *)device->machine->driver_data;

	/* bits 2-4 select MSM5205 clock & 3b/4b playback mode */
	msm5205_playmode_w(state->adpcm1, (data >> 2) & 7);
	if (state->adpcm2 != NULL)
		msm5205_playmode_w(state->adpcm2, ((data >> 2) & 4) | 3);	/* always in slave mode */

	/* bits 0 and 1 reset the two chips */
	msm5205_reset_w(state->adpcm1, data & 1);
	if (state->adpcm2 != NULL)
		msm5205_reset_w(state->adpcm2, data & 2);
}


static WRITE8_DEVICE_HANDLER( ay8910_1_porta_w )
{
#ifdef MAME_DEBUG
	if (data & 0x0f) popmessage("analog sound %x",data&0x0f);
#endif
}



/*************************************
 *
 *  Memory-mapped accesses
 *
 *************************************/

static WRITE8_HANDLER( sound_irq_ack_w )
{
	cputag_set_input_line(space->machine, "iremsound", 0, CLEAR_LINE);
}


static WRITE8_HANDLER( m52_adpcm_w )
{
	irem_z80_state *state = (irem_z80_state *)space->machine->driver_data;

	if (offset & 1)
	{
		msm5205_data_w(state->adpcm1, data);
	}
	if (offset & 2)
	{
		if (state->adpcm2 != NULL)
			msm5205_data_w(state->adpcm2, data);
	}
}


static WRITE8_HANDLER( m62_adpcm_w )
{
	irem_z80_state *state = (irem_z80_state *)space->machine->driver_data;

	running_device *adpcm = (offset & 1) ? state->adpcm2 : state->adpcm1;
	if (adpcm != NULL)
		msm5205_data_w(adpcm, data);
}



/*************************************
 *
 *  MSM5205 data ready signals
 *
 *************************************/

static void adpcm_int(running_device *device)
{
	irem_z80_state *state = (irem_z80_state *)device->machine->driver_data;

	cputag_set_input_line(device->machine, "iremsound", INPUT_LINE_NMI, PULSE_LINE);

	/* the first MSM5205 clocks the second */
	if (state->adpcm2 != NULL)
	{
		msm5205_vclk_w(state->adpcm2, 1);
		msm5205_vclk_w(state->adpcm2, 0);
	}
}



/*************************************
 *
 *  Sound interfaces
 *
 *************************************/

/* All 6 (3*2) AY-3-8910 outputs are tied together
 * and put with 470 Ohm to gnd.
 * The following is a approximation, since
 * it does not take cross-chip mixing effects into account.
 */

static const ay8910_interface irem_ay8910_interface_1 =
{
	AY8910_SINGLE_OUTPUT | AY8910_DISCRETE_OUTPUT,
	{470, 0, 0},
	DEVCB_MEMORY_HANDLER("iremsound", PROGRAM, soundlatch_r),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_HANDLER(ay8910_0_portb_w)
};

static const ay8910_interface irem_ay8910_interface_2 =
{
	AY8910_SINGLE_OUTPUT | AY8910_DISCRETE_OUTPUT,
	{470, 0, 0},
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_HANDLER(ay8910_1_porta_w),
	DEVCB_NULL
};

static const msm5205_interface irem_msm5205_interface_1 =
{
	adpcm_int,			/* interrupt function */
	MSM5205_S96_4B		/* default to 4KHz, but can be changed at run time */
};

static const msm5205_interface irem_msm5205_interface_2 =
{
	0,				/* interrupt function */
	MSM5205_SEX_4B		/* default to 4KHz, but can be changed at run time, slave */
};

/*
 * http://newsgroups.derkeiler.com/Archive/Rec/rec.games.video.arcade.collecting/2006-06/msg03108.html
 *
 * mentions, that moon patrol does work on moon ranger hardware.
 * There is no MSM5250, but a 74LS00 producing white noise for explosions
 */

/* Certain values are different from schematics
 * I did my test to verify against pcb pictures of "tropical angel"
 */

#define M52_R9		560
#define M52_R10		330
#define M52_R12		RES_K(10)
#define M52_R13		RES_K(10)
#define M52_R14		RES_K(10)
#define M52_R15		RES_K(2.2)	/* schematics RES_K(22) , althought 10-Yard states 2.2 */
#define M52_R19		RES_K(10)
#define M52_R22		RES_K(47)
#define M52_R23		RES_K(2.2)
#define M52_R25		RES_K(10)
#define M52_VR1		RES_K(50)

#define M52_C28		CAP_U(1)
#define M52_C30		CAP_U(0.022)
#define M52_C32		CAP_U(0.022)
#define M52_C35		CAP_U(47)
#define M52_C37		CAP_U(0.1)
#define M52_C38		CAP_U(0.0068)

/*
 * C35 is disabled, the mixer would just deliver
 * no signals if it is enabled.
 * TODO: Check discrete mixer
 *
 */

static const discrete_mixer_desc m52_sound_c_stage1 =
	{DISC_MIXER_IS_RESISTOR,
		{M52_R19, M52_R22, M52_R23 },
		{      0,       0,		 0 },	/* variable resistors   */
		{M52_C37,		0,		 0 },	/* node capacitors      */
		       0,		0,				/* rI, rF               */
		M52_C35*0,						/* cF                   */
		0,								/* cAmp                 */
		0, 1};

static const discrete_op_amp_filt_info m52_sound_c_sallen_key =
	{ M52_R13, M52_R14, 0, 0, 0,
	  M52_C32, M52_C38, 0
	};

static const discrete_mixer_desc m52_sound_c_mix1 =
	{DISC_MIXER_IS_RESISTOR,
		{M52_R25, M52_R15 },
		{      0,       0 },	/* variable resistors   */
		{      0,       0 },    /* node capacitors      */
		       0, M52_VR1,		/* rI, rF               */
		0,						/* cF                   */
		CAP_U(1),				/* cAmp                 */
		0, 1};

static DISCRETE_SOUND_START( m52_sound_c )

	/* Chip AY8910/1 */
	DISCRETE_INPUTX_STREAM(NODE_01, 0, 1.0, 0)
	/* Chip AY8910/2 */
	DISCRETE_INPUTX_STREAM(NODE_02, 1, 1.0, 0)
	/* Chip MSM5250 */
	DISCRETE_INPUTX_STREAM(NODE_03, 2, 1.0, 0)

	/* Just mix the two AY8910s */
	DISCRETE_ADDER2(NODE_09, 1, NODE_01, NODE_02)
	DISCRETE_DIVIDE(NODE_10, 1, NODE_09, 2.0)

	/* Mix in 5 V to MSM5250 signal */
	DISCRETE_MIXER3(NODE_20, 1, NODE_03, 32767.0, 0, &m52_sound_c_stage1)

	/* Sallen - Key Filter */
	/* TODO: R12, C30: This looks like a band pass */
	DISCRETE_RCFILTER(NODE_25, NODE_20, M52_R12, M52_C30)
	DISCRETE_SALLEN_KEY_FILTER(NODE_30, 1, NODE_25, DISC_SALLEN_KEY_LOW_PASS, &m52_sound_c_sallen_key)

	/* Mix signals */
	DISCRETE_MIXER2(NODE_40, 1, NODE_10, NODE_25, &m52_sound_c_mix1)
	DISCRETE_CRFILTER(NODE_45, NODE_40, M52_R10+M52_R9, M52_C28)

	DISCRETE_OUTPUT(NODE_40, 18.0)

DISCRETE_SOUND_END

/*************************************
 *
 *  Address maps
 *
 *************************************/

/* complete address map verified from Moon Patrol/10 Yard Fight schematics */
/* large map uses 8k ROMs, small map uses 4k ROMs; this is selected via a jumper */
static ADDRESS_MAP_START( m52_small_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x0fff) AM_WRITE(m52_adpcm_w)
	AM_RANGE(0x1000, 0x1fff) AM_WRITE(sound_irq_ack_w)
	AM_RANGE(0x2000, 0x7fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( m52_large_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_WRITE(m52_adpcm_w)
	AM_RANGE(0x2000, 0x3fff) AM_WRITE(sound_irq_ack_w)
	AM_RANGE(0x4000, 0xffff) AM_ROM
ADDRESS_MAP_END


/* complete address map verified from Kid Niki schematics */
static ADDRESS_MAP_START( m62_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0800, 0x0800) AM_MIRROR(0xf7fc) AM_WRITE(sound_irq_ack_w)
	AM_RANGE(0x0801, 0x0802) AM_MIRROR(0xf7fc) AM_WRITE(m62_adpcm_w)
	AM_RANGE(0x4000, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( irem_sound_portmap, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(M6803_PORT1, M6803_PORT1) AM_READWRITE(m6803_port1_r, m6803_port1_w)
	AM_RANGE(M6803_PORT2, M6803_PORT2) AM_READWRITE(m6803_port2_r, m6803_port2_w)
ADDRESS_MAP_END



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static MACHINE_DRIVER_START( irem_audio_base )

	MDRV_SOUND_START(irem_audio)

	/* basic machine hardware */
	MDRV_CPU_ADD("iremsound", M6803, XTAL_3_579545MHz) /* verified on pcb */
	MDRV_CPU_IO_MAP(irem_sound_portmap)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ay1", AY8910, XTAL_3_579545MHz/4) /* verified on pcb */
	MDRV_SOUND_CONFIG(irem_ay8910_interface_1)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MDRV_SOUND_ADD("ay2", AY8910, XTAL_3_579545MHz/4) /* verified on pcb */
	MDRV_SOUND_CONFIG(irem_ay8910_interface_2)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MDRV_SOUND_ADD("msm1", MSM5205, XTAL_384kHz) /* verified on pcb */
	MDRV_SOUND_CONFIG(irem_msm5205_interface_1)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MDRV_SOUND_ADD("msm2", MSM5205, XTAL_384kHz) /* verified on pcb */
	MDRV_SOUND_CONFIG(irem_msm5205_interface_2)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)
MACHINE_DRIVER_END

MACHINE_DRIVER_START( m52_sound_c_audio )

	MDRV_SOUND_START(irem_audio)

	/* basic machine hardware */
	MDRV_CPU_ADD("iremsound", M6803, XTAL_3_579545MHz) /* verified on pcb */
	MDRV_CPU_IO_MAP(irem_sound_portmap)
	MDRV_CPU_PROGRAM_MAP(m52_small_sound_map)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ay1", AY8910, XTAL_3_579545MHz/4) /* verified on pcb */
	MDRV_SOUND_CONFIG(irem_ay8910_interface_1)
	MDRV_SOUND_ROUTE_EX(0, "filtermix", 1.0, 0)

	MDRV_SOUND_ADD("ay2", AY8910, XTAL_3_579545MHz/4) /* verified on pcb */
	MDRV_SOUND_CONFIG(irem_ay8910_interface_2)
	MDRV_SOUND_ROUTE_EX(0, "filtermix", 1.0, 1)

	MDRV_SOUND_ADD("msm1", MSM5205, XTAL_384kHz) /* verified on pcb */
	MDRV_SOUND_CONFIG(irem_msm5205_interface_1)
	MDRV_SOUND_ROUTE_EX(0, "filtermix", 1.0, 2)

	MDRV_SOUND_ADD("filtermix", DISCRETE, 0)
	MDRV_SOUND_CONFIG_DISCRETE(m52_sound_c)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

MACHINE_DRIVER_END

MACHINE_DRIVER_START( m52_large_audio )	/* 10 yard fight */
	MDRV_IMPORT_FROM(irem_audio_base)

	/* basic machine hardware */
	MDRV_CPU_MODIFY("iremsound")
	MDRV_CPU_PROGRAM_MAP(m52_large_sound_map)
MACHINE_DRIVER_END


MACHINE_DRIVER_START( m62_audio )
	MDRV_IMPORT_FROM(irem_audio_base)

	/* basic machine hardware */
	MDRV_CPU_MODIFY("iremsound")
	MDRV_CPU_PROGRAM_MAP(m62_sound_map)
MACHINE_DRIVER_END
