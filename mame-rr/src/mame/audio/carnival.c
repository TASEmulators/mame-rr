/*****************************************************************************/
/*                                                                           */
/*                    (C) Copyright       Peter J.C.Clare                    */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*                                                                           */
/*      Module name:    carnival.c                                           */
/*                                                                           */
/*      Creation date:  15/03/98                Revision date:  09/01/99     */
/*                                                                           */
/*      Produced by:    Peter J.C.Clare                                      */
/*                                                                           */
/*                                                                           */
/*      Abstract:                                                            */
/*                                                                           */
/*              MAME sound & music driver for Sega/Gremlin Carnival.         */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*      Acknowledgements:                                                    */
/*                                                                           */
/*      Mike Coates, for the original Carnival MAME sound driver.            */
/*      Virtu-Al, for the sound samples & hardware information.              */
/*      The MAME Team, for the emulator framework.                           */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*      Revision history                                                     */
/*      ================                                                     */
/*                                                                           */
/*         Date     Vsn.  Initials   Description                             */
/*         ~~~~     ~~~~  ~~~~~~~~   ~~~~~~~~~~~                             */
/*      03/20/2007  1.1      ZV      Moved structures from driver to make    */
/*                                   file more self contained.               */
/*                                                                           */
/*****************************************************************************/

#include "emu.h"
#include "cpu/mcs48/mcs48.h"
#include "sound/ay8910.h"
#include "sound/samples.h"
#include "includes/vicdual.h"


#define	PSG_CLOCK		(3579545 / 3)	/* Hz */


/* output port 0x01 definitions - sound effect drive outputs */
#define OUT_PORT_1_RIFLE_SHOT   0x01
#define OUT_PORT_1_CLANG        0x02
#define OUT_PORT_1_DUCK_1       0x04
#define OUT_PORT_1_DUCK_2       0x08
#define OUT_PORT_1_DUCK_3       0x10
#define OUT_PORT_1_PIPE_HIT     0x20
#define OUT_PORT_1_BONUS_1      0x40
#define OUT_PORT_1_BONUS_2      0x80


/* output port 0x02 definitions - sound effect drive outputs */
#define OUT_PORT_2_BEAR         0x04
#define OUT_PORT_2_MUSIC_T1     0x08
#define OUT_PORT_2_MUSIC_RESET  0x10
#define OUT_PORT_2_RANKING      0x20


/* music CPU port definitions */
#define MUSIC_PORT2_PSG_BDIR    0x40    /* bit 6 on P2 */
#define MUSIC_PORT2_PSG_BC1     0x80    /* bit 7 on P2 */


#define PSG_BC_INACTIVE         0
#define PSG_BC_READ             MUSIC_PORT2_PSG_BC1
#define PSG_BC_WRITE            MUSIC_PORT2_PSG_BDIR
#define PSG_BC_LATCH_ADDRESS    ( MUSIC_PORT2_PSG_BDIR | MUSIC_PORT2_PSG_BC1 )


#define PLAY(samp,id,loop)      sample_start( samp, id, id, loop )
#define STOP(samp,id)           sample_stop( samp, id )


/* sample file names */
static const char *const carnival_sample_names[] =
{
	"*carnival",
	"bear.wav",
	"bonus1.wav",
	"bonus2.wav",
	"clang.wav",
	"duck1.wav",
	"duck2.wav",
	"duck3.wav",
	"pipehit.wav",
	"ranking.wav",
	"rifle.wav",
	0
};


static const samples_interface carnival_samples_interface =
{
	10,
	carnival_sample_names
};


/* sample IDs - must match sample file name table above */
enum
{
	SND_BEAR = 0,
	SND_BONUS_1,
	SND_BONUS_2,
	SND_CLANG,
	SND_DUCK_1,
	SND_DUCK_2,
	SND_DUCK_3,
	SND_PIPE_HIT,
	SND_RANKING,
	SND_RIFLE_SHOT
};


static int port2State = 0;
static int psgData = 0;


WRITE8_HANDLER( carnival_audio_1_w )
{
	static int port1State = 0;
	running_device *samples = space->machine->device("samples");
	int bitsChanged;
	int bitsGoneHigh;
	int bitsGoneLow;


	/* U64 74LS374 8 bit latch */

	/* bit 0: connector pin 36 - rifle shot */
	/* bit 1: connector pin 35 - clang */
	/* bit 2: connector pin 33 - duck #1 */
	/* bit 3: connector pin 34 - duck #2 */
	/* bit 4: connector pin 32 - duck #3 */
	/* bit 5: connector pin 31 - pipe hit */
	/* bit 6: connector pin 30 - bonus #1 */
	/* bit 7: connector pin 29 - bonus #2 */

	bitsChanged  = port1State ^ data;
	bitsGoneHigh = bitsChanged & data;
	bitsGoneLow  = bitsChanged & ~data;

	port1State = data;

	if ( bitsGoneLow & OUT_PORT_1_RIFLE_SHOT )
	{
		PLAY( samples, SND_RIFLE_SHOT, 0 );
	}

	if ( bitsGoneLow & OUT_PORT_1_CLANG )
	{
		PLAY( samples, SND_CLANG, 0 );
	}

	if ( bitsGoneLow & OUT_PORT_1_DUCK_1 )
	{
		PLAY( samples, SND_DUCK_1, 1 );
	}
	if ( bitsGoneHigh & OUT_PORT_1_DUCK_1 )
	{
		STOP( samples, SND_DUCK_1 );
	}

	if ( bitsGoneLow & OUT_PORT_1_DUCK_2 )
	{
		PLAY( samples, SND_DUCK_2, 1 );
	}
	if ( bitsGoneHigh & OUT_PORT_1_DUCK_2 )
	{
		STOP( samples, SND_DUCK_2 );
	}

	if ( bitsGoneLow & OUT_PORT_1_DUCK_3 )
	{
		PLAY( samples, SND_DUCK_3, 1 );
	}
	if ( bitsGoneHigh & OUT_PORT_1_DUCK_3 )
	{
		STOP( samples, SND_DUCK_3 );
	}

	if ( bitsGoneLow & OUT_PORT_1_PIPE_HIT )
	{
		PLAY( samples, SND_PIPE_HIT, 0 );
	}

	if ( bitsGoneLow & OUT_PORT_1_BONUS_1 )
	{
		PLAY( samples, SND_BONUS_1, 0 );
	}

	if ( bitsGoneLow & OUT_PORT_1_BONUS_2 )
	{
		PLAY( samples, SND_BONUS_2, 0 );
	}
}


WRITE8_HANDLER( carnival_audio_2_w )
{
	running_device *samples = space->machine->device("samples");
	int bitsChanged;
	int bitsGoneHigh;
	int bitsGoneLow;

	/* U63 74LS374 8 bit latch */

	/* bit 0: connector pin 48 */
	/* bit 1: connector pin 47 */
	/* bit 2: connector pin 45 - bear */
	/* bit 3: connector pin 46 - Music !T1 input */
	/* bit 4: connector pin 44 - Music reset */
	/* bit 5: connector pin 43 - ranking */
	/* bit 6: connector pin 42 */
	/* bit 7: connector pin 41 */

	bitsChanged  = port2State ^ data;
	bitsGoneHigh = bitsChanged & data;
	bitsGoneLow  = bitsChanged & ~data;

	port2State = data;

	if ( bitsGoneLow & OUT_PORT_2_BEAR )
		PLAY( samples, SND_BEAR, 0 );

	if ( bitsGoneLow & OUT_PORT_2_RANKING )
		PLAY( samples, SND_RANKING, 0 );

	if ( bitsGoneHigh & OUT_PORT_2_MUSIC_RESET )
		/* reset output is no longer asserted active low */
		cputag_set_input_line(space->machine, "audiocpu", INPUT_LINE_RESET, PULSE_LINE );
}


static READ8_HANDLER( carnival_music_port_t1_r )
{
	/* note: 8039 T1 signal is inverted on music board */
	return ( port2State & OUT_PORT_2_MUSIC_T1 ) ? 0 : 1;
}


static WRITE8_HANDLER( carnival_music_port_1_w )
{
	psgData = data;
}


static WRITE8_DEVICE_HANDLER( carnival_music_port_2_w )
{
	static int psgSelect = 0;
	int newSelect;

	newSelect = data & ( MUSIC_PORT2_PSG_BDIR | MUSIC_PORT2_PSG_BC1 );
	if ( psgSelect != newSelect )
	{
		psgSelect = newSelect;

		switch ( psgSelect )
		{
		case PSG_BC_INACTIVE:
			/* do nowt */
			break;

		case PSG_BC_READ:
			/* not very sensible for a write */
			break;

		case PSG_BC_WRITE:
			ay8910_data_w( device, 0, psgData );
			break;

		case PSG_BC_LATCH_ADDRESS:
			ay8910_address_w( device, 0, psgData );
			break;
		}
	}
}


static ADDRESS_MAP_START( carnival_audio_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( carnival_audio_io_map, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(MCS48_PORT_T1, MCS48_PORT_T1) AM_READ(carnival_music_port_t1_r)
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_WRITE(carnival_music_port_1_w)
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_DEVWRITE("psg", carnival_music_port_2_w)
ADDRESS_MAP_END


MACHINE_DRIVER_START( carnival_audio )
	MDRV_CPU_ADD("audiocpu", I8039, 3579545)
	MDRV_CPU_PROGRAM_MAP(carnival_audio_map)
	MDRV_CPU_IO_MAP(carnival_audio_io_map)

	MDRV_QUANTUM_TIME(HZ(600))

	MDRV_SOUND_ADD("psg", AY8910, PSG_CLOCK)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)

	MDRV_SOUND_ADD("samples", SAMPLES, 0)
	MDRV_SOUND_CONFIG(carnival_samples_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_DRIVER_END
