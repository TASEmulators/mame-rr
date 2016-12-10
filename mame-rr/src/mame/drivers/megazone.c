/**************************************************************************

Based on drivers from Juno First emulator by Chris Hardy (chris@junofirst.freeserve.co.uk)

To enter service mode, keep 1&2 pressed on reset

***************************************************************************/

#include "emu.h"
#include "machine/konami1.h"
#include "cpu/m6809/m6809.h"
#include "cpu/mcs48/mcs48.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "sound/flt_rc.h"
#include "includes/konamipt.h"
#include "includes/megazone.h"


static READ8_DEVICE_HANDLER( megazone_port_a_r )
{
	megazone_state *state = (megazone_state *)device->machine->driver_data;
	int clock, timer;


	/* main xtal 14.318MHz, divided by 8 to get the AY-3-8910 clock, further */
	/* divided by 1024 to get this timer */
	/* The base clock for the CPU and 8910 is NOT the same, so we have to */
	/* compensate. */
	/* (divide by (1024/2), and not 1024, because the CPU cycle counter is */
	/* incremented every other state change of the clock) */

	clock = state->audiocpu->total_cycles() * 7159/12288;	/* = (14318/8)/(18432/6) */
	timer = (clock / (1024/2)) & 0x0f;

	/* low three bits come from the 8039 */
	return (timer << 4) | state->i8039_status;
}

static WRITE8_DEVICE_HANDLER( megazone_port_b_w )
{
	static const char *const fltname[] = { "filter.0.0", "filter.0.1", "filter.0.2" };
	int i;

	for (i = 0; i < 3; i++)
	{
		int C = 0;
		if (data & 1)
			C +=  10000;	/*  10000pF = 0.01uF */
		if (data & 2)
			C += 220000;	/* 220000pF = 0.22uF */

		data >>= 2;
		filter_rc_set_RC(device->machine->device(fltname[i]),FLT_RC_LOWPASS,1000,2200,200,CAP_P(C));
	}
}

static WRITE8_HANDLER( megazone_i8039_irq_w )
{
	megazone_state *state = (megazone_state *)space->machine->driver_data;
	cpu_set_input_line(state->daccpu, 0, ASSERT_LINE);
}

static WRITE8_HANDLER( i8039_irqen_and_status_w )
{
	megazone_state *state = (megazone_state *)space->machine->driver_data;

	if ((data & 0x80) == 0)
		cpu_set_input_line(state->daccpu, 0, CLEAR_LINE);
	state->i8039_status = (data & 0x70) >> 4;
}

static WRITE8_HANDLER( megazone_coin_counter_w )
{
	coin_counter_w(space->machine, 1 - offset, data);		/* 1-offset, because coin counters are in reversed order */
}



static ADDRESS_MAP_START( megazone_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0001) AM_WRITE(megazone_coin_counter_w) /* coin counter 2, coin counter 1 */
	AM_RANGE(0x0005, 0x0005) AM_WRITE(megazone_flipscreen_w)
	AM_RANGE(0x0007, 0x0007) AM_WRITE(interrupt_enable_w)
	AM_RANGE(0x0800, 0x0800) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x1000, 0x1000) AM_WRITEONLY AM_BASE_MEMBER(megazone_state, scrolly)
	AM_RANGE(0x1800, 0x1800) AM_WRITEONLY AM_BASE_MEMBER(megazone_state, scrollx)
	AM_RANGE(0x2000, 0x23ff) AM_RAM AM_BASE_SIZE_MEMBER(megazone_state, videoram, videoram_size)
	AM_RANGE(0x2400, 0x27ff) AM_RAM AM_BASE_SIZE_MEMBER(megazone_state, videoram2, videoram2_size)
	AM_RANGE(0x2800, 0x2bff) AM_RAM AM_BASE_MEMBER(megazone_state, colorram)
	AM_RANGE(0x2c00, 0x2fff) AM_RAM AM_BASE_MEMBER(megazone_state, colorram2)
	AM_RANGE(0x3000, 0x33ff) AM_RAM AM_BASE_SIZE_MEMBER(megazone_state, spriteram, spriteram_size)
	AM_RANGE(0x3800, 0x3fff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x4000, 0xffff) AM_ROM		/* 4000->5FFF is a debug rom */
ADDRESS_MAP_END

static ADDRESS_MAP_START( megazone_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x2000) AM_WRITE(megazone_i8039_irq_w)	/* START line. Interrupts 8039 */
	AM_RANGE(0x4000, 0x4000) AM_WRITE(soundlatch_w)			/* CODE  line. Command Interrupts 8039 */
	AM_RANGE(0x6000, 0x6000) AM_READ_PORT("IN0")			/* IO Coin */
	AM_RANGE(0x6001, 0x6001) AM_READ_PORT("IN1")			/* P1 IO */
	AM_RANGE(0x6002, 0x6002) AM_READ_PORT("IN2")			/* P2 IO */
	AM_RANGE(0x8000, 0x8000) AM_READ_PORT("DSW1")
	AM_RANGE(0x8001, 0x8001) AM_READ_PORT("DSW2")
	AM_RANGE(0xa000, 0xa000) AM_WRITENOP					/* INTMAIN - Interrupts main CPU (unused) */
	AM_RANGE(0xc000, 0xc000) AM_WRITENOP					/* INT (Actually is NMI) enable/disable (unused)*/
	AM_RANGE(0xc001, 0xc001) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0xe000, 0xe7ff) AM_RAM AM_SHARE("share1")
ADDRESS_MAP_END

static ADDRESS_MAP_START( megazone_sound_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_DEVWRITE("aysnd", ay8910_address_w)
	AM_RANGE(0x00, 0x02) AM_DEVREAD("aysnd", ay8910_r)
	AM_RANGE(0x02, 0x02) AM_DEVWRITE("aysnd", ay8910_data_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( megazone_i8039_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( megazone_i8039_io_map, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x00, 0xff) AM_READ(soundlatch_r)
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_DEVWRITE("dac", dac_w)
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_WRITE(i8039_irqen_and_status_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( megazone )
	/* 0x6000 -> 0xe320 (CPU1) = 0x3b20 (CPU0) */
	PORT_START("IN0")
	KONAMI8_SYSTEM_UNK

	/* 0x6001 -> 0xe31e (CPU1) = 0x3b1e (CPU0) */
	PORT_START("IN1")
	KONAMI8_MONO_B1_UNK

	/* 0x6002 -> 0xe31e (CPU1) = 0x3b1e (CPU0) or 0xe31f (CPU1) = 0x3b1f (CPU0) in "test mode" */
	PORT_START("IN2")
	KONAMI8_COCKTAIL_B1_UNK

	/* 0x8000 -> 0xe020 (CPU1) = 0x3820 (CPU0) */
	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x18, "20k 70k 70k+" )
	PORT_DIPSETTING(    0x10, "20k 80k 80k+" )
	PORT_DIPSETTING(    0x08, "30k 90k 90k+" )
	PORT_DIPSETTING(    0x00, "30k 100k 100k+" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	/* 0x8001 -> 0xe021 (CPU1) = 0x3821 (CPU0) */
	PORT_START("DSW2")
	KONAMI_COINAGE(DEF_STR( Free_Play ), "No Coin B")
	/* "No Coin B" = coins produce sound, but no effect on coin counter */
INPUT_PORTS_END

static INPUT_PORTS_START( megazona )
	PORT_INCLUDE( megazone )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x00, "7" )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	512,    /* 512 characters */
	4,      /* 4 bits per pixel */
	{ 0, 1, 2, 3 }, /* the four bitplanes are packed in one nibble */
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8    /* every char takes 8 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	16,16,	/* 16*16 sprites */
	256,	/* 256 sprites */
	4,	/* 4 bits per pixel */
	{ 0x4000*8+4, 0x4000*8+0, 4, 0 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 24*8+0, 24*8+1, 24*8+2, 24*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 ,
		32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8 },
	64*8	/* every sprite takes 64 consecutive bytes */
};

static GFXDECODE_START( megazone )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout,     0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, charlayout,   16*16, 16 )
GFXDECODE_END



static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_HANDLER(megazone_port_a_r),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_HANDLER(megazone_port_b_w)
};



static MACHINE_START( megazone )
{
	megazone_state *state = (megazone_state *)machine->driver_data;

	state->maincpu = machine->device<cpu_device>("maincpu");
	state->audiocpu = machine->device<cpu_device>("audiocpu");
	state->daccpu = machine->device<cpu_device>("daccpu");

	state_save_register_global(machine, state->flipscreen);
	state_save_register_global(machine, state->i8039_status);
}

static MACHINE_RESET( megazone )
{
	megazone_state *state = (megazone_state *)machine->driver_data;

	state->flipscreen = 0;
	state->i8039_status = 0;
}

static MACHINE_DRIVER_START( megazone )

	/* driver data */
	MDRV_DRIVER_DATA(megazone_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M6809, 18432000/9)        /* 2 MHz */
	MDRV_CPU_PROGRAM_MAP(megazone_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_CPU_ADD("audiocpu", Z80,18432000/6)     /* Z80 Clock is derived from the H1 signal */
	MDRV_CPU_PROGRAM_MAP(megazone_sound_map)
	MDRV_CPU_IO_MAP(megazone_sound_io_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_CPU_ADD("daccpu", I8039,14318000/2)	/* 1/2 14MHz crystal */
	MDRV_CPU_PROGRAM_MAP(megazone_i8039_map)
	MDRV_CPU_IO_MAP(megazone_i8039_io_map)

	MDRV_QUANTUM_TIME(HZ(900))
	MDRV_MACHINE_START(megazone)
	MDRV_MACHINE_RESET(megazone)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(36*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 36*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(megazone)
	MDRV_PALETTE_LENGTH(16*16+16*16)

	MDRV_PALETTE_INIT(megazone)
	MDRV_VIDEO_START(megazone)
	MDRV_VIDEO_UPDATE(megazone)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("aysnd", AY8910, 14318000/8)
	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(0, "filter.0.0", 0.30)
	MDRV_SOUND_ROUTE(1, "filter.0.1", 0.30)
	MDRV_SOUND_ROUTE(2, "filter.0.2", 0.30)

	MDRV_SOUND_ADD("dac", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD("filter.0.0", FILTER_RC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
	MDRV_SOUND_ADD("filter.0.1", FILTER_RC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
	MDRV_SOUND_ADD("filter.0.2", FILTER_RC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( megazone )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "319i07.bin",    0x6000, 0x2000, CRC(94b22ea8) SHA1(dc3ed2a0d1a12df51e46561324d78b7d655be313) )
	ROM_LOAD( "319i06.bin",    0x8000, 0x2000, CRC(0468b619) SHA1(a6755728fab37674749f9b77cb53f6f228102f2f) )
	ROM_LOAD( "319i05.bin",    0xa000, 0x2000, CRC(ac59000c) SHA1(c7568589f6b0e1706e996fdfed9c16755541951e) )
	ROM_LOAD( "319i04.bin",    0xc000, 0x2000, CRC(1e968603) SHA1(fd818f678a3dc8d48a30f9f7670bfcb42a3009a2) )
	ROM_LOAD( "319i03.bin",    0xe000, 0x2000, CRC(0888b803) SHA1(37249bfb14c6c3ce40ad68be457ab1f66fd7ea70) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "319e02.bin",   0x0000, 0x2000, CRC(d5d45edb) SHA1(3808d1b58fe152f8f5b49bf0aa40c53e9c9dd4bd) )

	ROM_REGION( 0x1000, "daccpu", 0 )     /* 4k for the 8039 DAC CPU */
	ROM_LOAD( "319e01.bin",   0x0000, 0x1000, CRC(ed5725a0) SHA1(64f54621487291fbfe827fb4cecca299fd0db781) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "319e11.bin",    0x0000, 0x2000, CRC(965a7ff6) SHA1(210aae91a3838e5f7c78747d9b7419d266538ffc) )
	ROM_LOAD( "319e09.bin",    0x2000, 0x2000, CRC(5eaa7f3e) SHA1(4c038e80d575988407252897a1f1bc6b76af597c) )
	ROM_LOAD( "319e10.bin",    0x4000, 0x2000, CRC(7bb1aeee) SHA1(be2dd46cd0121cedad6dab90a22643798a3176ab) )
	ROM_LOAD( "319e08.bin",    0x6000, 0x2000, CRC(6add71b1) SHA1(fc8c0ecd3b7f03d63b6c3143143986883345fa38) )

	ROM_REGION( 0x04000, "gfx2", 0 )
	ROM_LOAD( "319e12.bin",    0x0000, 0x2000, CRC(e0fb7835) SHA1(44ccaaf92bdb83323f45e08dbe118697720e9105) )
	ROM_LOAD( "319e13.bin",    0x2000, 0x2000, CRC(3d8f3743) SHA1(1f6fbf804dacfa44cd11b4cf41d0bedb7f2ff6b6) )

	ROM_REGION( 0x0260, "proms", 0 )
	ROM_LOAD( "319b18.a16",  0x0000, 0x020, CRC(23cb02af) SHA1(aba459826a75ec07bc6d97580e934f58aa22f4f4) ) /* palette */
	ROM_LOAD( "319b16.c6",   0x0020, 0x100, CRC(5748e933) SHA1(c1478c31533a11421cd4579ccfdbb430e193d17b) ) /* sprite lookup table */
	ROM_LOAD( "319b17.a11",  0x0120, 0x100, CRC(1fbfce73) SHA1(1c58eb91982d5f10511d54a83070e859ac57d2f1) ) /* character lookup table */
	ROM_LOAD( "319b14.e7",   0x0220, 0x020, CRC(55044268) SHA1(29cf4158314ed897daf045a7f07be865dd977663) ) /* timing (not used) */
	ROM_LOAD( "319b15.e8",   0x0240, 0x020, CRC(31fd7ab9) SHA1(04d6e51b4930619c8ee12fb0d7b5f981e4d6d8d3) ) /* timing (not used) */
ROM_END

ROM_START( megazonei )
	ROM_REGION( 2*0x10000, "maincpu", 0 )
	ROM_LOAD( "ic59_cpu.bin",  0x6000, 0x2000, CRC(f41922a0) SHA1(9f54509da18721a76593921c6e52085e62e6ea6b) )
	ROM_LOAD( "ic58_cpu.bin",  0x8000, 0x2000, CRC(7fd7277b) SHA1(e773247e0c9419cae49e04962ea362a2976c2db2) )
	ROM_LOAD( "ic57_cpu.bin",  0xa000, 0x2000, CRC(a4b33b51) SHA1(12bb4da0319a7fe355e5ea4945759c8709aed5fe) )
	ROM_LOAD( "ic56_cpu.bin",  0xc000, 0x2000, CRC(2aabcfbf) SHA1(f0054af98bd68158eab3328f8cf2a04b35e812c7) )
	ROM_LOAD( "ic55_cpu.bin",  0xe000, 0x2000, CRC(b33a3c37) SHA1(2f1fdf1b9f18fcc9bd97cc9adeecc4ce77dd30c9) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "319e02.bin",   0x0000, 0x2000, CRC(d5d45edb) SHA1(3808d1b58fe152f8f5b49bf0aa40c53e9c9dd4bd) )

	ROM_REGION( 0x1000, "daccpu", 0 )     /* 4k for the 8039 DAC CPU */
	ROM_LOAD( "319e01.bin",   0x0000, 0x1000, CRC(ed5725a0) SHA1(64f54621487291fbfe827fb4cecca299fd0db781) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "319e11.bin",    0x0000, 0x2000, CRC(965a7ff6) SHA1(210aae91a3838e5f7c78747d9b7419d266538ffc) )
	ROM_LOAD( "319e09.bin",    0x2000, 0x2000, CRC(5eaa7f3e) SHA1(4c038e80d575988407252897a1f1bc6b76af597c) )
	ROM_LOAD( "319e10.bin",    0x4000, 0x2000, CRC(7bb1aeee) SHA1(be2dd46cd0121cedad6dab90a22643798a3176ab) )
	ROM_LOAD( "319e08.bin",    0x6000, 0x2000, CRC(6add71b1) SHA1(fc8c0ecd3b7f03d63b6c3143143986883345fa38) )

	ROM_REGION( 0x04000, "gfx2", 0 )
	ROM_LOAD( "ic40_vid.bin",  0x0000, 0x2000, CRC(07b8b24b) SHA1(faadcb20ee8b26b9ab0692df6a81e5423514863e) )
	ROM_LOAD( "319e13.bin",    0x2000, 0x2000, CRC(3d8f3743) SHA1(1f6fbf804dacfa44cd11b4cf41d0bedb7f2ff6b6) )

	ROM_REGION( 0x0260, "proms", 0 )
	ROM_LOAD( "319b18.a16",  0x0000, 0x020, CRC(23cb02af) SHA1(aba459826a75ec07bc6d97580e934f58aa22f4f4) ) /* palette */
	ROM_LOAD( "319b16.c6",   0x0020, 0x100, CRC(5748e933) SHA1(c1478c31533a11421cd4579ccfdbb430e193d17b) ) /* sprite lookup table */
	ROM_LOAD( "319b17.a11",  0x0120, 0x100, CRC(1fbfce73) SHA1(1c58eb91982d5f10511d54a83070e859ac57d2f1) ) /* character lookup table */
	ROM_LOAD( "319b14.e7",   0x0220, 0x020, CRC(55044268) SHA1(29cf4158314ed897daf045a7f07be865dd977663) ) /* timing (not used) */
	ROM_LOAD( "319b15.e8",   0x0240, 0x020, CRC(31fd7ab9) SHA1(04d6e51b4930619c8ee12fb0d7b5f981e4d6d8d3) ) /* timing (not used) */
ROM_END

/* Info provided with these alt sets

        MEGA ZONE   CHIP PLACEMENT

USES 69A09EP, Z80 CPU'S & AY-3-8910 SOUND CHIP W/8039 CPU

THERE ARE AT LEAST THREE VERSIONS OF MEGA ZONE, ALL THE ROMS ARE THE
SAME EXCEPT POSITION 6,7,8,9,11H IN SETS 1,2
ALL ROMS ARE 2764 EXCEPT H01 (E01) IS A 2732

CHIP #     POSITION                         VERS 3
-----------------------------------------------------
VER-1                VER-2
-----------------------------------------------------
319-E08    2D        E08      REAR BOARD    8  SAME
319-E09    2E        E09       "            9  SAME
319-E10    3D        E10       "           10  SAME
319-E11    3E        E11       "           11  SAME
319-G12    8C        G12       "           12
319-G13    10C       G13       "           13  SAME
319-E02    6D        E02      CONN BOARD    2  SAME
319-H03    6H        J03       "            3
319-H04    7H        J04       "            4
319-H05    8H        J05       "            5
319-H06    9H        J06       "            6
319-H07    11H       J07       "            7
319-H01    3A        E01       "            1  SAME
Z80        7E                                        IC#
AY-3-8910  8B                               PROM     98     TBP18S030 (82S123)
AO72       12F   KONAMI                     PROM     48       "
K824-501   8D    KONAMI                     PROM     42       "
8039       4B                               PROM     63     TBP24S10  (823126)
                                            PROM     33       "
                                            PAL16L8  63
                                            PAL16L8A 67

VERSION 3 IS ON THE SAME SIZE CONNECTOR BOARD, BUT THE BOTTOM
BOARD IS ABOUT 1 1/4" LONGER AND WIDER

THE CHIPS THAT HAVE THE DESIGNATION SCRATCHED OFF ON THE ORIGINAL
BOARDS ARE      NAME          CHIP TYPE
            ---------------------------
CONN BOARD      IC3             TMP8039P-6
 "              IC6             AY3-8910
 "              IC26            Z-80
 "              IC39            MC68A09EP (CUSTOM ON ORIGINAL)
 "              IC27            N/U       (CUSTOM ON ORIGINAL)
REAR BOARD      1C026           N/U       (CUSTOM ON ORIGINAL)

*/

ROM_START( megazonea )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "7.12g",  0x6000, 0x2000, CRC(d42d67bf) SHA1(adac80d183ad26a9b1ec25a2da7ebbb33b441b63) )
	ROM_LOAD( "6.10g",  0x8000, 0x2000, CRC(692398eb) SHA1(518001d738c2fb9417e52edfe9a7b74a074af3b0) )
	ROM_LOAD( "5.9g",   0xa000, 0x2000, CRC(620ffec3) SHA1(e047beb29e0cda72126e8dcdd0b7504a202efba2) )
	ROM_LOAD( "4.8g",   0xc000, 0x2000, CRC(28650971) SHA1(25e405fb9f648b7118e3c7c7b3ba59a7b7c29c42) )
	ROM_LOAD( "3.6g",   0xe000, 0x2000, CRC(f264018f) SHA1(6ca0f7e26311799b0650a6c58567405bc2a7f922) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "319-h02",   0x0000, 0x2000, CRC(d5d45edb) SHA1(3808d1b58fe152f8f5b49bf0aa40c53e9c9dd4bd) )

	ROM_REGION( 0x1000, "daccpu", 0 )     /* 4k for the 8039 DAC CPU */
	ROM_LOAD( "319-h01",   0x0000, 0x1000, CRC(ed5725a0) SHA1(64f54621487291fbfe827fb4cecca299fd0db781) ) // same as e01?

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "319e11.bin",    0x0000, 0x2000, CRC(965a7ff6) SHA1(210aae91a3838e5f7c78747d9b7419d266538ffc) )
	ROM_LOAD( "319e09.bin",    0x2000, 0x2000, CRC(5eaa7f3e) SHA1(4c038e80d575988407252897a1f1bc6b76af597c) )
	ROM_LOAD( "319e10.bin",    0x4000, 0x2000, CRC(7bb1aeee) SHA1(be2dd46cd0121cedad6dab90a22643798a3176ab) )
	ROM_LOAD( "319e08.bin",    0x6000, 0x2000, CRC(6add71b1) SHA1(fc8c0ecd3b7f03d63b6c3143143986883345fa38) )

	ROM_REGION( 0x04000, "gfx2", 0 )
	ROM_LOAD( "319e12.bin",    0x0000, 0x2000, CRC(e0fb7835) SHA1(44ccaaf92bdb83323f45e08dbe118697720e9105) ) // 12.9c
	ROM_LOAD( "319-g13",  0x2000, 0x2000, CRC(3d8f3743) SHA1(1f6fbf804dacfa44cd11b4cf41d0bedb7f2ff6b6) ) // same as e13?

	ROM_REGION( 0x0260, "proms", 0 )
	ROM_LOAD( "319b18.a16",  0x0000, 0x020, CRC(23cb02af) SHA1(aba459826a75ec07bc6d97580e934f58aa22f4f4) ) /* palette */
	ROM_LOAD( "319b16.c6",   0x0020, 0x100, CRC(5748e933) SHA1(c1478c31533a11421cd4579ccfdbb430e193d17b) ) /* sprite lookup table */
	ROM_LOAD( "319b17.a11",  0x0120, 0x100, CRC(1fbfce73) SHA1(1c58eb91982d5f10511d54a83070e859ac57d2f1) ) /* character lookup table */
	ROM_LOAD( "319b14.e7",   0x0220, 0x020, CRC(55044268) SHA1(29cf4158314ed897daf045a7f07be865dd977663) ) /* timing (not used) */
	ROM_LOAD( "319b15.e8",   0x0240, 0x020, CRC(31fd7ab9) SHA1(04d6e51b4930619c8ee12fb0d7b5f981e4d6d8d3) ) /* timing (not used) */
ROM_END

ROM_START( megazoneb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "319-j07",  0x6000, 0x2000,  CRC(5161a523) SHA1(90b456c30bccaaca96c75c2f421af3a2875b0b6b) )
	ROM_LOAD( "319-j06",  0x8000, 0x2000,  CRC(7344c3de) SHA1(d3867738d4828afa50c8b43116d68cc6074d6cb5) )
	ROM_LOAD( "319-j05",  0xa000, 0x2000,  CRC(affa492b) SHA1(ee6789f293902716d65d08a89ae12dd96c75c885) )
	ROM_LOAD( "319-j04",  0xc000, 0x2000,  CRC(03544ab3) SHA1(efa034cc6976b47915601cf215758df23e308878) )
	ROM_LOAD( "319-j03",  0xe000, 0x2000,  CRC(0d95cc0a) SHA1(9aadadf09a4826da451ee35c89ee0254ec552d80) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "319-h02",   0x0000, 0x2000, CRC(d5d45edb) SHA1(3808d1b58fe152f8f5b49bf0aa40c53e9c9dd4bd) )

	ROM_REGION( 0x1000, "daccpu", 0 )     /* 4k for the 8039 DAC CPU */
	ROM_LOAD( "319-h01",   0x0000, 0x1000, CRC(ed5725a0) SHA1(64f54621487291fbfe827fb4cecca299fd0db781) ) // same as e01?

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "319e11.bin",    0x0000, 0x2000, CRC(965a7ff6) SHA1(210aae91a3838e5f7c78747d9b7419d266538ffc) )
	ROM_LOAD( "319e09.bin",    0x2000, 0x2000, CRC(5eaa7f3e) SHA1(4c038e80d575988407252897a1f1bc6b76af597c) )
	ROM_LOAD( "319e10.bin",    0x4000, 0x2000, CRC(7bb1aeee) SHA1(be2dd46cd0121cedad6dab90a22643798a3176ab) )
	ROM_LOAD( "319e08.bin",    0x6000, 0x2000, CRC(6add71b1) SHA1(fc8c0ecd3b7f03d63b6c3143143986883345fa38) )

	ROM_REGION( 0x04000, "gfx2", 0 )
	ROM_LOAD( "319e12.bin",    0x0000, 0x2000, CRC(e0fb7835) SHA1(44ccaaf92bdb83323f45e08dbe118697720e9105) ) // 12.9c
	ROM_LOAD( "319-g13",  0x2000, 0x2000, CRC(3d8f3743) SHA1(1f6fbf804dacfa44cd11b4cf41d0bedb7f2ff6b6) ) // same as e13?

	ROM_REGION( 0x0260, "proms", 0 )
	ROM_LOAD( "319b18.a16",  0x0000, 0x020, CRC(23cb02af) SHA1(aba459826a75ec07bc6d97580e934f58aa22f4f4) ) /* palette */
	ROM_LOAD( "319b16.c6",   0x0020, 0x100, CRC(5748e933) SHA1(c1478c31533a11421cd4579ccfdbb430e193d17b) ) /* sprite lookup table */
	ROM_LOAD( "319b17.a11",  0x0120, 0x100, CRC(1fbfce73) SHA1(1c58eb91982d5f10511d54a83070e859ac57d2f1) ) /* character lookup table */
	ROM_LOAD( "319b14.e7",   0x0220, 0x020, CRC(55044268) SHA1(29cf4158314ed897daf045a7f07be865dd977663) ) /* timing (not used) */
	ROM_LOAD( "319b15.e8",   0x0240, 0x020, CRC(31fd7ab9) SHA1(04d6e51b4930619c8ee12fb0d7b5f981e4d6d8d3) ) /* timing (not used) */
ROM_END

ROM_START( megazonec )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "319-h07",  0x6000, 0x2000,  CRC(8ca47f64) SHA1(1a20db5ac504b9b004116cfa6992d63a86a04cc5) )
	ROM_LOAD( "319-h06",  0x8000, 0x2000,  CRC(ed35b12e) SHA1(69e88c4801c838a24aba0a867af205a7169ad089) )
	ROM_LOAD( "319-h05",  0xa000, 0x2000,  CRC(c3655ccd) SHA1(b86b58a12c6ced9a7e0a6d0cdb3881a28220a650) )
	ROM_LOAD( "319-h04",  0xc000, 0x2000,  CRC(9e221177) SHA1(0c6fffd657d66090362108578aa78eb36bdcce6b) )
	ROM_LOAD( "319-h03",  0xe000, 0x2000,  CRC(9048955b) SHA1(d8a7b46d984832f566298d3b417b3a34c9fea6c7) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "319-h02",   0x0000, 0x2000, CRC(d5d45edb) SHA1(3808d1b58fe152f8f5b49bf0aa40c53e9c9dd4bd) )

	ROM_REGION( 0x1000, "daccpu", 0 )     /* 4k for the 8039 DAC CPU */
	ROM_LOAD( "319-h01",   0x0000, 0x1000, CRC(ed5725a0) SHA1(64f54621487291fbfe827fb4cecca299fd0db781) ) // same as e01?

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "319e11.bin",    0x0000, 0x2000, CRC(965a7ff6) SHA1(210aae91a3838e5f7c78747d9b7419d266538ffc) )
	ROM_LOAD( "319e09.bin",    0x2000, 0x2000, CRC(5eaa7f3e) SHA1(4c038e80d575988407252897a1f1bc6b76af597c) )
	ROM_LOAD( "319e10.bin",    0x4000, 0x2000, CRC(7bb1aeee) SHA1(be2dd46cd0121cedad6dab90a22643798a3176ab) )
	ROM_LOAD( "319e08.bin",    0x6000, 0x2000, CRC(6add71b1) SHA1(fc8c0ecd3b7f03d63b6c3143143986883345fa38) )

	ROM_REGION( 0x04000, "gfx2", 0 )
	ROM_LOAD( "319e12.bin",    0x0000, 0x2000, CRC(e0fb7835) SHA1(44ccaaf92bdb83323f45e08dbe118697720e9105) ) // 12.9c
	ROM_LOAD( "319-g13",  0x2000, 0x2000, CRC(3d8f3743) SHA1(1f6fbf804dacfa44cd11b4cf41d0bedb7f2ff6b6) ) // same as e13?

	ROM_REGION( 0x0260, "proms", 0 )
	ROM_LOAD( "319b18.a16",  0x0000, 0x020, CRC(23cb02af) SHA1(aba459826a75ec07bc6d97580e934f58aa22f4f4) ) /* palette */
	ROM_LOAD( "319b16.c6",   0x0020, 0x100, CRC(5748e933) SHA1(c1478c31533a11421cd4579ccfdbb430e193d17b) ) /* sprite lookup table */
	ROM_LOAD( "319b17.a11",  0x0120, 0x100, CRC(1fbfce73) SHA1(1c58eb91982d5f10511d54a83070e859ac57d2f1) ) /* character lookup table */
	ROM_LOAD( "319b14.e7",   0x0220, 0x020, CRC(55044268) SHA1(29cf4158314ed897daf045a7f07be865dd977663) ) /* timing (not used) */
	ROM_LOAD( "prom.48",     0x0240, 0x020, CRC(796dea94) SHA1(bab3c2a5466e1c07ec27cccf7b1a21e9de4ed982) ) /* timing (not used) */
ROM_END


static DRIVER_INIT( megazone )
{
	konami1_decode(machine, "maincpu");
}

/* these just display a Konami copyright, no logo */
GAME( 1983, megazone, 0,         megazone, megazone, megazone, ROT90, "Konami",                       "Mega Zone (Konami set 1)", GAME_SUPPORTS_SAVE )
GAME( 1983, megazonea, megazone, megazone, megazona, megazone, ROT90, "Konami",                       "Mega Zone (Konami set 2)", GAME_SUPPORTS_SAVE )

/* these display Konami and Kosuka copyright, no logo */
GAME( 1983, megazoneb, megazone, megazone, megazone, megazone, ROT90, "Konami / Kosuka",              "Mega Zone (Kosuka set 1)", GAME_SUPPORTS_SAVE )
GAME( 1983, megazonec, megazone, megazone, megazone, megazone, ROT90, "Konami / Kosuka",              "Mega Zone (Kosuka set 2)", GAME_SUPPORTS_SAVE )

/* this displays Konami and Kosuka copyright with a Konami / Interlogic logo */
GAME( 1983, megazonei, megazone, megazone, megazone, megazone, ROT90, "Konami / Interlogic / Kosuka", "Mega Zone (Interlogic / Kosuka)", GAME_SUPPORTS_SAVE )
