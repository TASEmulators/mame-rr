/*
 *  Invinco sound routines
 */

#include "emu.h"
#include "sound/samples.h"
#include "includes/vicdual.h"

/* output port 0x02 definitions - sound effect drive outputs */
#define OUT_PORT_2_SAUCER		0x04
#define OUT_PORT_2_MOVE1		0x08
#define OUT_PORT_2_MOVE2		0x10
#define OUT_PORT_2_FIRE			0x20
#define OUT_PORT_2_INVHIT		0x40
#define OUT_PORT_2_SHIPHIT		0x80


#define PLAY(samp,id,loop)      sample_start( samp, id, id, loop )
#define STOP(samp,id)           sample_stop( samp, id )


/* sample file names */
static const char *const invinco_sample_names[] =
{
	"*invinco",
	"saucer.wav",
	"move1.wav",
	"move2.wav",
	"fire.wav",
	"invhit.wav",
	"shiphit.wav",
	"move3.wav",	/* currently not used */
	"move4.wav",	/* currently not used */
	0
};


static const samples_interface invinco_samples_interface =
{
	8,
	invinco_sample_names
};


MACHINE_DRIVER_START( invinco_audio )
	MDRV_SOUND_ADD("samples", SAMPLES, 0)
	MDRV_SOUND_CONFIG(invinco_samples_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_DRIVER_END


/* sample IDs - must match sample file name table above */
enum
{
	SND_SAUCER = 0,
	SND_MOVE1,
	SND_MOVE2,
	SND_FIRE,
	SND_INVHIT,
	SND_SHIPHIT,
	SND_MOVE3,
	SND_MOVE4
};


WRITE8_HANDLER( invinco_audio_w )
{
	static int port2State = 0;
	running_device *samples = space->machine->device("samples");
	int bitsChanged;
	//int bitsGoneHigh;
	int bitsGoneLow;


	bitsChanged  = port2State ^ data;
	//bitsGoneHigh = bitsChanged & data;
	bitsGoneLow  = bitsChanged & ~data;

	port2State = data;

	if ( bitsGoneLow & OUT_PORT_2_SAUCER )
	{
		PLAY( samples, SND_SAUCER, 0 );
	}

	if ( bitsGoneLow & OUT_PORT_2_MOVE1 )
	{
		PLAY( samples, SND_MOVE1, 0 );
	}

	if ( bitsGoneLow & OUT_PORT_2_MOVE2 )
	{
		PLAY( samples, SND_MOVE2, 0 );
	}

	if ( bitsGoneLow & OUT_PORT_2_FIRE )
	{
		PLAY( samples, SND_FIRE, 0 );
	}

	if ( bitsGoneLow & OUT_PORT_2_INVHIT )
	{
		PLAY( samples, SND_INVHIT, 0 );
	}

	if ( bitsGoneLow & OUT_PORT_2_SHIPHIT )
	{
		PLAY( samples, SND_SHIPHIT, 0 );
	}

#if 0
	logerror("Went LO: %02X  %04X\n", bitsGoneLow, cpu_get_pc(space->cpu));
#endif
}
