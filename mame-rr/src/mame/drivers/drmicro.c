/*****************************************************************************

Dr. Micro (c) 1983 Sanritsu

        driver by Uki

Quite similar to Appoooh

*****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/msm5205.h"
#include "sound/sn76496.h"
#include "includes/drmicro.h"

#define MCLK 18432000


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

static INTERRUPT_GEN( drmicro_interrupt )
{
	drmicro_state *state = (drmicro_state *)device->machine->driver_data;

	if (state->nmi_enable)
		 cpu_set_input_line(device, INPUT_LINE_NMI, PULSE_LINE);
}

static WRITE8_HANDLER( nmi_enable_w )
{
	drmicro_state *state = (drmicro_state *)space->machine->driver_data;

	state->nmi_enable = data & 1;
	state->flipscreen = (data & 2) ? 1 : 0;
	flip_screen_set(space->machine, data & 2);

	// bit2,3 unknown
}


static void pcm_w(running_device *device)
{
	drmicro_state *state = (drmicro_state *)device->machine->driver_data;
	UINT8 *PCM = memory_region(device->machine, "adpcm");

	int data = PCM[state->pcm_adr / 2];

	if (data != 0x70) // ??
	{
		if (~state->pcm_adr & 1)
			data >>= 4;

		msm5205_data_w(device, data & 0x0f);
		msm5205_reset_w(device, 0);

		state->pcm_adr = (state->pcm_adr + 1) & 0x7fff;
	}
	else
		msm5205_reset_w(device, 1);
}

static WRITE8_HANDLER( pcm_set_w )
{
	drmicro_state *state = (drmicro_state *)space->machine->driver_data;
	state->pcm_adr = ((data & 0x3f) << 9);
	pcm_w(state->msm);
}

/*************************************
 *
 *  Address maps
 *
 *************************************/

static ADDRESS_MAP_START( drmicro_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xefff) AM_RAM_WRITE(drmicro_videoram_w)
	AM_RANGE(0xf000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("P1") AM_DEVWRITE("sn1", sn76496_w)
	AM_RANGE(0x01, 0x01) AM_READ_PORT("P2") AM_DEVWRITE("sn2", sn76496_w)
	AM_RANGE(0x02, 0x02) AM_DEVWRITE("sn3", sn76496_w)
	AM_RANGE(0x03, 0x03) AM_READ_PORT("DSW1") AM_WRITE(pcm_set_w)
	AM_RANGE(0x04, 0x04) AM_READ_PORT("DSW2") AM_WRITE(nmi_enable_w)
	AM_RANGE(0x05, 0x05) AM_NOP // unused? / watchdog?
ADDRESS_MAP_END

/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( drmicro )
	PORT_START("P1")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY

	PORT_START("P2")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x08, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "30000 100000" )
	PORT_DIPSETTING(    0x08, "50000 150000" )
	PORT_DIPSETTING(    0x10, "70000 200000" )
	PORT_DIPSETTING(    0x18, "100000 300000" )
	PORT_SERVICE( 0x20, IP_ACTIVE_HIGH )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_BIT( 0xf8, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // 4-8
INPUT_PORTS_END

/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout spritelayout4 =
{
	16,16,
	0x100,
	2,
	{0,0x2000*8},
	{STEP8(7,-1),STEP8(71,-1)},
	{STEP8(0,8),STEP8(128,8)},
	8*8*4
};

static const gfx_layout spritelayout8 =
{
	16,16,
	0x100,
	3,
	{0x2000*16,0x2000*8,0},
	{STEP8(7,-1),STEP8(71,-1)},
	{STEP8(0,8),STEP8(128,8)},
	8*8*4
};

static const gfx_layout charlayout4 =
{
	8,8,
	0x400,
	2,
	{0,0x2000*8},
	{STEP8(7,-1)},
	{STEP8(0,8)},
	8*8*1
};

static const gfx_layout charlayout8 =
{
	8,8,
	0x400,
	3,
	{0x2000*16,0x2000*8,0},
	{STEP8(7,-1)},
	{STEP8(0,8)},
	8*8*1
};

static GFXDECODE_START( drmicro )
	GFXDECODE_ENTRY( "gfx1", 0x0000, charlayout4,     0, 64 ) /* tiles */
	GFXDECODE_ENTRY( "gfx2", 0x0000, charlayout8,   256, 32 ) /* tiles */
	GFXDECODE_ENTRY( "gfx1", 0x0000, spritelayout4,   0, 64 ) /* sprites */
	GFXDECODE_ENTRY( "gfx2", 0x0000, spritelayout8, 256, 32 ) /* sprites */
GFXDECODE_END

static const msm5205_interface msm5205_config =
{
	pcm_w,			/* IRQ handler */
	MSM5205_S64_4B	/* 6 KHz */
};


/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_START( drmicro )
{
	drmicro_state *state = (drmicro_state *)machine->driver_data;

	state->msm = machine->device("msm");

	state_save_register_global(machine, state->nmi_enable);
	state_save_register_global(machine, state->pcm_adr);
	state_save_register_global(machine, state->flipscreen);
}

static MACHINE_RESET( drmicro )
{
	drmicro_state *state = (drmicro_state *)machine->driver_data;

	state->nmi_enable = 0;
	state->pcm_adr = 0;
	state->flipscreen = 0;
}


static MACHINE_DRIVER_START( drmicro )

	/* driver data */
	MDRV_DRIVER_DATA(drmicro_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80,MCLK/6)	/* 3.072MHz? */
	MDRV_CPU_PROGRAM_MAP(drmicro_map)
	MDRV_CPU_IO_MAP(io_map)
	MDRV_CPU_VBLANK_INT("screen", drmicro_interrupt)

	MDRV_QUANTUM_TIME(HZ(60))

	MDRV_MACHINE_START(drmicro)
	MDRV_MACHINE_RESET(drmicro)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(drmicro)
	MDRV_PALETTE_LENGTH(512)

	MDRV_PALETTE_INIT(drmicro)
	MDRV_VIDEO_START(drmicro)
	MDRV_VIDEO_UPDATE(drmicro)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("sn1", SN76496, MCLK/4)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD("sn2", SN76496, MCLK/4)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD("sn3", SN76496, MCLK/4)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD("msm", MSM5205, 384000)
	MDRV_SOUND_CONFIG(msm5205_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.75)
MACHINE_DRIVER_END

/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( drmicro )
	ROM_REGION( 0x10000, "maincpu", 0 ) // CPU
	ROM_LOAD( "dm-00.13b", 0x0000,  0x2000, CRC(270f2145) SHA1(1557428387e2c0f711c676a13a763c8d48aa497b) )
	ROM_LOAD( "dm-01.14b", 0x2000,  0x2000, CRC(bba30c80) SHA1(a084429fad58fa6348936084652235d5f55e3b89) )
	ROM_LOAD( "dm-02.15b", 0x4000,  0x2000, CRC(d9e4ca6b) SHA1(9fb6d1d6b45628891deae389cf1d142332b110ba) )
	ROM_LOAD( "dm-03.13d", 0x6000,  0x2000, CRC(b7bcb45b) SHA1(61035afc642bac2e1c56c36c188bed4e1949523f) )
	ROM_LOAD( "dm-04.14d", 0x8000,  0x2000, CRC(071db054) SHA1(75929b7692bebf2246fa84581b6d1eedb02c9aba) )
	ROM_LOAD( "dm-05.15d", 0xa000,  0x2000, CRC(f41b8d8a) SHA1(802830f3f0362ec3df257f31dc22390e8ae4207c) )

	ROM_REGION( 0x04000, "gfx1", 0 ) // gfx 1
	ROM_LOAD( "dm-23.5l",  0x0000,  0x2000, CRC(279a76b8) SHA1(635650621bdce5873bb5faf64f8352149314e784) )
	ROM_LOAD( "dm-24.5n",  0x2000,  0x2000, CRC(ee8ed1ec) SHA1(7afc05c73186af9fe3d3f3ce13412c8ee560b146) )

	ROM_REGION( 0x06000, "gfx2", 0 ) // gfx 2
	ROM_LOAD( "dm-20.4a",  0x0000,  0x2000, CRC(6f5dbf22) SHA1(41ef084336e2ebb1016b28505dcb43483e37a0de) )
	ROM_LOAD( "dm-21.4c",  0x2000,  0x2000, CRC(8b17ff47) SHA1(5bcc14489ea1d4f1fe8e51c24a72a8e787ab8159) )
	ROM_LOAD( "dm-22.4d",  0x4000,  0x2000, CRC(84daf771) SHA1(d187debcca59ceab6cd696be246370120ee575c6) )

	ROM_REGION( 0x04000, "adpcm", 0 ) // samples
	ROM_LOAD( "dm-40.12m",  0x0000,  0x2000, CRC(3d080af9) SHA1(f9527fae69fe3ca0762024ac4a44b1f02fbee66a) )
	ROM_LOAD( "dm-41.13m",  0x2000,  0x2000, CRC(ddd7bda2) SHA1(bbe9276cb47fa3e82081d592522640e04b4a9223) )

	ROM_REGION( 0x00220, "proms", 0 ) // PROMs
	ROM_LOAD( "dm-62.9h", 0x0000,  0x0020, CRC(e3e36eaf) SHA1(5954400190e587a20cad60f5829f4bddc85ea526) )
	ROM_LOAD( "dm-61.4m", 0x0020,  0x0100, CRC(0dd8e365) SHA1(cbd43a2d4af053860932af32ca5e13bef728e38a) )
	ROM_LOAD( "dm-60.6e", 0x0120,  0x0100, CRC(540a3953) SHA1(bc65388a1019dadf8c71705e234763f5c735e282) )
ROM_END


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1983, drmicro, 0, drmicro, drmicro, 0, ROT270, "Sanritsu", "Dr. Micro", GAME_SUPPORTS_SAVE )

