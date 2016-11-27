/***************************************************************************

Space Force Memory Map

driver by Zsolt Vasvari


0000-3fff   R   ROM
4000-43ff   R/W RAM
7000-7002   R   input ports 0-2
7000          W sound command
7001          W sound CPU IRQ trigger on bit 3 falling edge
7002          W unknown
7008          W unknown
7009          W unknown
700a          W unknown
700b          W flip screen
700c          W unknown
700d          W unknown
700e          W main CPU interrupt enable (it uses RST7.5)
700f          W unknown
8000-83ff   R/W bit 0-7 of character code
9000-93ff   R/W attributes RAM
                bit 0   - bit 8 of character code
                bit 1-3 - unused
                bit 4-6 - color
                bit 7   - unused
a000-a3ff   R/W X/Y scroll position of each character (can be scrolled up
                to 7 pixels in each direction)


***************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "cpu/mcs48/mcs48.h"
#include "sound/sn76496.h"


extern UINT8 *spcforce_videoram;
extern UINT8 *spcforce_colorram;
extern UINT8 *spcforce_scrollram;

WRITE8_HANDLER( spcforce_flip_screen_w );
VIDEO_UPDATE( spcforce );


static int spcforce_SN76496_latch;
static int spcforce_SN76496_select;

static WRITE8_HANDLER( spcforce_SN76496_latch_w )
{
	spcforce_SN76496_latch = data;
}

static READ8_HANDLER( spcforce_SN76496_select_r )
{
	if (~spcforce_SN76496_select & 0x40) return sn76496_ready_r(space->machine->device("sn1"));
	if (~spcforce_SN76496_select & 0x20) return sn76496_ready_r(space->machine->device("sn2"));
	if (~spcforce_SN76496_select & 0x10) return sn76496_ready_r(space->machine->device("sn3"));

	return 0;
}

static WRITE8_HANDLER( spcforce_SN76496_select_w )
{
    spcforce_SN76496_select = data;

	if (~data & 0x40)  sn76496_w(space->machine->device("sn1"), 0, spcforce_SN76496_latch);
	if (~data & 0x20)  sn76496_w(space->machine->device("sn2"), 0, spcforce_SN76496_latch);
	if (~data & 0x10)  sn76496_w(space->machine->device("sn3"), 0, spcforce_SN76496_latch);
}

static READ8_HANDLER( spcforce_t0_r )
{
	/* SN76496 status according to Al - not supported by MAME?? */
	return mame_rand(space->machine) & 1;
}


static WRITE8_HANDLER( spcforce_soundtrigger_w )
{
	cputag_set_input_line(space->machine, "audiocpu", 0, (~data & 0x08) ? ASSERT_LINE : CLEAR_LINE);
}


static ADDRESS_MAP_START( spcforce_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x43ff) AM_RAM
	AM_RANGE(0x7000, 0x7000) AM_READ_PORT("DSW") AM_WRITE(soundlatch_w)
	AM_RANGE(0x7001, 0x7001) AM_READ_PORT("P1") AM_WRITE(spcforce_soundtrigger_w)
	AM_RANGE(0x7002, 0x7002) AM_READ_PORT("P2")
	AM_RANGE(0x700b, 0x700b) AM_WRITE(spcforce_flip_screen_w)
	AM_RANGE(0x700e, 0x700e) AM_WRITE(interrupt_enable_w)
	AM_RANGE(0x700f, 0x700f) AM_WRITENOP
	AM_RANGE(0x8000, 0x83ff) AM_RAM AM_BASE(&spcforce_videoram)
	AM_RANGE(0x9000, 0x93ff) AM_RAM AM_BASE(&spcforce_colorram)
	AM_RANGE(0xa000, 0xa3ff) AM_RAM AM_BASE(&spcforce_scrollram)
ADDRESS_MAP_END

static ADDRESS_MAP_START( spcforce_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( spcforce_sound_io_map, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(MCS48_PORT_BUS, MCS48_PORT_BUS) AM_READ(soundlatch_r)
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_WRITE(spcforce_SN76496_latch_w)
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_READWRITE(spcforce_SN76496_select_r, spcforce_SN76496_select_w)
	AM_RANGE(MCS48_PORT_T0, MCS48_PORT_T0) AM_READ(spcforce_t0_r)
ADDRESS_MAP_END


static INPUT_PORTS_START( spcforce )
	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x18, 0x08, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x18, "5" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )	/* probably unused */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_START("P1")
	PORT_BIT ( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT ( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT ( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT ( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT ( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT ( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY

	PORT_START("P2")
	PORT_BIT ( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT ( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT ( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT ( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT ( 0x40, IP_ACTIVE_LOW, IPT_VBLANK )
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL
INPUT_PORTS_END

/* same as spcforce, but no cocktail mode */
static INPUT_PORTS_START( spcforc2 )
	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x18, 0x08, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x18, "5" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )	/* probably unused */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )  /* probably unused */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("P1")
	PORT_BIT ( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT ( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT ( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT ( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT ( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT ( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY

	PORT_START("P2")
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT ( 0x40, IP_ACTIVE_LOW, IPT_VBLANK )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,    /* 8*8 chars */
	512,    /* 512 characters */
	3,      /* 3 bits per pixel */
	{ 2*512*8*8, 512*8*8, 0 },  /* The bitplanes are seperate */
	{ 0, 1, 2, 3, 4, 5, 6, 7},
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8},
	8*8     /* every char takes 8 consecutive bytes */
};


static GFXDECODE_START( spcforce )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 8 )
GFXDECODE_END


/* 1-bit RGB palette */
static const int colortable_source[] =
{
	0, 1, 2, 3, 4, 5, 6, 7,
	0, 0, 1, 2, 3, 4, 5, 6,	 /* not sure about these, but they are only used */
	0, 7, 0, 1, 2, 3, 4, 5,  /* to change the text color. During the game,   */
	0, 6, 7, 0, 1, 2, 3, 4,  /* only color 0 is used, which is correct.      */
	0, 5, 6, 7, 0, 1, 2, 3,
	0, 4, 5, 6, 7, 0, 1, 2,
	0, 3, 4, 5, 6, 7, 0, 1,
	0, 2, 3, 4, 5, 6, 7, 0
};

static PALETTE_INIT( spcforce )
{
	int i;

	for (i = 0; i < sizeof(colortable_source) / sizeof(colortable_source[0]); i++)
	{
		int data = colortable_source[i];
		rgb_t color = MAKE_RGB(pal1bit(data >> 0), pal1bit(data >> 1), pal1bit(data >> 2));

		palette_set_color(machine, i, color);
	}
}


static MACHINE_DRIVER_START( spcforce )

	/* basic machine hardware */
	/* FIXME: The 8085A had a max clock of 6MHz, internally divided by 2! */
	MDRV_CPU_ADD("maincpu", I8085A, 8000000 * 2)        /* 4.00 MHz??? */
	MDRV_CPU_PROGRAM_MAP(spcforce_map)
	MDRV_CPU_VBLANK_INT("screen", irq3_line_pulse)

	MDRV_CPU_ADD("audiocpu", I8035, 6144000)		/* divisor ??? */
	MDRV_CPU_PROGRAM_MAP(spcforce_sound_map)
	MDRV_CPU_IO_MAP(spcforce_sound_io_map)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 28*8-1)

	MDRV_GFXDECODE(spcforce)
	MDRV_PALETTE_LENGTH(sizeof(colortable_source) / sizeof(colortable_source[0]))

	MDRV_PALETTE_INIT(spcforce)
	MDRV_VIDEO_UPDATE(spcforce)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("sn1", SN76496, 2000000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MDRV_SOUND_ADD("sn2", SN76496, 2000000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MDRV_SOUND_ADD("sn3", SN76496, 2000000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


/***************************************************************************

  Game driver(s)

***************************************************************************/
ROM_START( spcforce )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "m1v4f.1a",	  0x0000, 0x0800, CRC(7da0d1ed) SHA1(2ee145f590da557be057f181b4861014627872e7) )
	ROM_LOAD( "m2v4f.1c",	  0x0800, 0x0800, CRC(25605bff) SHA1(afda2884a00fdbc000191dd548fd8e34df3e2f49) )
	ROM_LOAD( "m3v5f.2a",	  0x1000, 0x0800, CRC(6f879366) SHA1(ef624619dbaad1f2adf4fab82e04bac117dbfac6) )
	ROM_LOAD( "m4v5f.2c",	  0x1800, 0x0800, CRC(7fbfabfa) SHA1(0d6bbdcc80e251aa0ebd12e66549afaf6d8ccb0e) )
							/*0x2000 empty */
	ROM_LOAD( "m6v4f.3c",	  0x2800, 0x0800, CRC(12128e9e) SHA1(b2a113b419e11ca094f56ae93870df11690b119a) )
	ROM_LOAD( "m7v4f.4a",	  0x3000, 0x0800, CRC(978ad452) SHA1(fa84dcc6587403dd939da719a747d8c7332ed038) )
	ROM_LOAD( "m8v4f.4c",	  0x3800, 0x0800, CRC(f805c3cd) SHA1(78eb13b99aae895742b34ed56bee9313d3643de1) )

	ROM_REGION( 0x1000, "audiocpu", 0 )		/* sound MCU */
	ROM_LOAD( "vm5.k10",      0x0000, 0x0800, CRC(8820913c) SHA1(90002cafdf5f32f916e5457e013ebe53405d5ca8) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_LOAD( "rm1v2.6s",     0x0000, 0x0800, CRC(8e3490d7) SHA1(a5e47f953bb833c2bb769b266fff60f7a20c69a6) )
	ROM_LOAD( "rm2v1.7s",     0x0800, 0x0800, CRC(fbbfa05a) SHA1(c737b216f47e14c069cb84b5dbcc5a79fcc13648) )
	ROM_LOAD( "gm1v2.6p",     0x1000, 0x0800, CRC(4f574920) SHA1(05930a8ea5c6e05d01d1b4faabb3305aab44125c) )
	ROM_LOAD( "gm2v1.7p",     0x1800, 0x0800, CRC(0cd89ce2) SHA1(adb101400eb00119930494e99629948248d99d2f) )
	ROM_LOAD( "bm1v2.6m",     0x2000, 0x0800, CRC(130869ce) SHA1(588d6c9403d5fd966266b4f0333ee47b36c8b1d8) )
	ROM_LOAD( "bm2v1.7m",     0x2800, 0x0800, CRC(472f0a9b) SHA1(a8a9e2aa62374cd3bd938b5cb5fb20face3114c3) )
ROM_END

ROM_START( spcforc2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "spacefor.1a",  0x0000, 0x0800, CRC(ef6fdccb) SHA1(2fff28437597958b39a821f93ac30f32c24f50aa) )
	ROM_LOAD( "spacefor.1c",  0x0800, 0x0800, CRC(44bd1cdd) SHA1(6dd5ae7a64079c61b63667f06e0d34dec48eac7c) )
	ROM_LOAD( "spacefor.2a",  0x1000, 0x0800, CRC(fcbc7df7) SHA1(b6e89dbfc80d5d9dcf889f618a8278c182773a14) )
	ROM_LOAD( "vm4",	      0x1800, 0x0800, CRC(c5b073b9) SHA1(93b77c77488aa954c35880439be6c7629448a3ea) )
							/*0x2000 empty */
	ROM_LOAD( "spacefor.3c",  0x2800, 0x0800, CRC(9fd52301) SHA1(1ea5d5b888dd2f7ac6aab227c78b86c2f2f320da) )
	ROM_LOAD( "spacefor.4a",  0x3000, 0x0800, CRC(89aefc0a) SHA1(0b56efa613bce972af4bbf145853bfc0cda60ef9) )
	ROM_LOAD( "m8v4f.4c",	  0x3800, 0x0800, CRC(f805c3cd) SHA1(78eb13b99aae895742b34ed56bee9313d3643de1) )

	ROM_REGION( 0x1000, "audiocpu", 0 )		/* sound MCU */
	ROM_LOAD( "vm5.k10",      0x0000, 0x0800, CRC(8820913c) SHA1(90002cafdf5f32f916e5457e013ebe53405d5ca8) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_LOAD( "spacefor.6s",  0x0000, 0x0800, CRC(848ae522) SHA1(deb28ba09556d04d9f6c906a163372f842b00c63) )
	ROM_LOAD( "rm2v1.7s",     0x0800, 0x0800, CRC(fbbfa05a) SHA1(c737b216f47e14c069cb84b5dbcc5a79fcc13648) )
	ROM_LOAD( "spacefor.6p",  0x1000, 0x0800, CRC(95446911) SHA1(843025d1c557156f73c2e9a1278c02738b69fb5d) )
	ROM_LOAD( "gm2v1.7p",     0x1800, 0x0800, CRC(0cd89ce2) SHA1(adb101400eb00119930494e99629948248d99d2f) )
	ROM_LOAD( "bm1v2.6m",     0x2000, 0x0800, CRC(130869ce) SHA1(588d6c9403d5fd966266b4f0333ee47b36c8b1d8) )
	ROM_LOAD( "bm2v1.7m",     0x2800, 0x0800, CRC(472f0a9b) SHA1(a8a9e2aa62374cd3bd938b5cb5fb20face3114c3) )
ROM_END

ROM_START( meteor )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "vm1",	      0x0000, 0x0800, CRC(894fe9b1) SHA1(617e05523392e2ba2608ca13aa24d6601289fe87) )
	ROM_LOAD( "vm2",	      0x0800, 0x0800, CRC(28685a68) SHA1(f911a3ccb8d63cf82a6dc8f069f3f498e9081656) )
	ROM_LOAD( "vm3",	      0x1000, 0x0800, CRC(c88fb12a) SHA1(1eeb26caf7a1421ec2d570f71b8c4675ad7ea172) )
	ROM_LOAD( "vm4",	      0x1800, 0x0800, CRC(c5b073b9) SHA1(93b77c77488aa954c35880439be6c7629448a3ea) )
							/*0x2000 empty */
	ROM_LOAD( "vm6",	      0x2800, 0x0800, CRC(9969ec43) SHA1(3ce067c34b84e9559f195e7ef9939a78070693b1) )
	ROM_LOAD( "vm7",	      0x3000, 0x0800, CRC(39f43ac2) SHA1(b45275759f4003a22a32dc04227a98908bd140a9) )
	ROM_LOAD( "vm8",	      0x3800, 0x0800, CRC(a0508de3) SHA1(75666a4e46b6c433f1c1f8e76c30fd087354097b) )

	ROM_REGION( 0x1000, "audiocpu", 0 )		/* sound MCU */
	ROM_LOAD( "vm5",	      0x0000, 0x0800, CRC(b14ccd57) SHA1(0349ec5d0ca7f98ffdd96d7bf01cf096fe547f7a) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_LOAD( "rm1v",         0x0000, 0x0800, CRC(d621fe96) SHA1(29b75333ea8103095a4d452636eea4a1055845e5) )
	ROM_LOAD( "rm2v",         0x0800, 0x0800, CRC(b3981251) SHA1(b6743d121a6b3ad8e8beebe1faff2678b89e7d16) )
	ROM_LOAD( "gm1v",         0x1000, 0x0800, CRC(d44617e8) SHA1(1cec7984cc5e3472c25c23f02179380c4a5b4076) )
	ROM_LOAD( "gm2v",         0x1800, 0x0800, CRC(0997d945) SHA1(16eba77b14c62b2a0ebea47a28d4d5d21d7a2234) )
	ROM_LOAD( "bm1v",         0x2000, 0x0800, CRC(cc97c890) SHA1(e852bfe9d4b2d31801a840c1bacdd4386a93a22f) )
	ROM_LOAD( "bm2v",         0x2800, 0x0800, CRC(2858cf5c) SHA1(1313b4e4adda074499153e4a42bc2c6b41b0ec7e) )
ROM_END


GAME( 1980, spcforce, 0,        spcforce, spcforce, 0, ROT270, "Venture Line", "Space Force (set 1)", GAME_IMPERFECT_COLORS )
GAME( 19??, spcforc2, spcforce, spcforce, spcforc2, 0, ROT270, "bootleg? (Elcon)", "Space Force (set 2)", GAME_IMPERFECT_COLORS )
GAME( 1981, meteor,   spcforce, spcforce, spcforc2, 0, ROT270, "Venture Line", "Meteoroids", GAME_IMPERFECT_COLORS )
