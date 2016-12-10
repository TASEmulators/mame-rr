/*
 *  Depth Charge audio routines
 */

#include "emu.h"
#include "sound/samples.h"
#include "includes/vicdual.h"

/* output port 0x01 definitions - sound effect drive outputs */
#define OUT_PORT_1_LONGEXPL     0x01
#define OUT_PORT_1_SHRTEXPL     0x02
#define OUT_PORT_1_SPRAY        0x04
#define OUT_PORT_1_SONAR        0x08


#define PLAY(samp,id,loop)      sample_start( samp, id, id, loop )
#define STOP(samp,id)           sample_stop( samp, id )


/* sample file names */
static const char *const depthch_sample_names[] =
{
	"*depthch",
	"longex.wav",
	"shortex.wav",
	"spray.wav",
	"sonar.wav",
	0
};


static const samples_interface depthch_samples_interface =
{
	4,
	depthch_sample_names
};


MACHINE_DRIVER_START( depthch_audio )
	MDRV_SOUND_ADD("samples", SAMPLES, 0)
	MDRV_SOUND_CONFIG(depthch_samples_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_DRIVER_END


/* sample IDs - must match sample file name table above */
enum
{
	SND_LONGEXPL = 0,
	SND_SHRTEXPL,
	SND_SPRAY,
	SND_SONAR,
};


WRITE8_HANDLER( depthch_audio_w )
{
	static int port1State = 0;
	running_device *samples = space->machine->device("samples");
	int bitsChanged;
	int bitsGoneHigh;
	int bitsGoneLow;


	bitsChanged  = port1State ^ data;
	bitsGoneHigh = bitsChanged & data;
	bitsGoneLow  = bitsChanged & ~data;

	port1State = data;

	if ( bitsGoneHigh & OUT_PORT_1_LONGEXPL )
	{
		PLAY( samples, SND_LONGEXPL, 0 );
	}

	if ( bitsGoneHigh & OUT_PORT_1_SHRTEXPL )
	{
		PLAY( samples, SND_SHRTEXPL, 0 );
	}

	if ( bitsGoneHigh & OUT_PORT_1_SPRAY )
	{
		PLAY( samples, SND_SPRAY, 0 );
	}

	if ( bitsGoneHigh & OUT_PORT_1_SONAR )
	{
		PLAY( samples, SND_SONAR, 1 );
	}
	if ( bitsGoneLow & OUT_PORT_1_SONAR )
	{
		STOP( samples, SND_SONAR );
	}
}
