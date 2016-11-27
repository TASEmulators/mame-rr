/***************************************************************************

    Roc'n Rope (c) 1983 Konami

    Based on drivers from Juno First emulator by Chris Hardy (chrish@kcbbs.gen.nz)

***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "machine/konami1.h"
#include "includes/timeplt.h"
#include "includes/konamipt.h"

#define MASTER_CLOCK          XTAL_18_432MHz


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

/* Roc'n'Rope has the IRQ vectors in RAM. The rom contains $FFFF at this address! */
static WRITE8_HANDLER( rocnrope_interrupt_vector_w )
{
	UINT8 *RAM = memory_region(space->machine, "maincpu");

	RAM[0xfff2 + offset] = data;
}


/*************************************
 *
 *  Address maps
 *
 *************************************/

static ADDRESS_MAP_START( rocnrope_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x3080, 0x3080) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x3081, 0x3081) AM_READ_PORT("P1")
	AM_RANGE(0x3082, 0x3082) AM_READ_PORT("P2")
	AM_RANGE(0x3083, 0x3083) AM_READ_PORT("DSW1")
	AM_RANGE(0x3000, 0x3000) AM_READ_PORT("DSW2")
	AM_RANGE(0x3100, 0x3100) AM_READ_PORT("DSW3")
	AM_RANGE(0x4000, 0x402f) AM_RAM AM_BASE_MEMBER(timeplt_state, spriteram2)
	AM_RANGE(0x4400, 0x442f) AM_RAM AM_BASE_SIZE_MEMBER(timeplt_state, spriteram, spriteram_size)
	AM_RANGE(0x4000, 0x47ff) AM_RAM
	AM_RANGE(0x4800, 0x4bff) AM_RAM_WRITE(rocnrope_colorram_w) AM_BASE_MEMBER(timeplt_state, colorram)
	AM_RANGE(0x4c00, 0x4fff) AM_RAM_WRITE(rocnrope_videoram_w) AM_BASE_MEMBER(timeplt_state, videoram)
	AM_RANGE(0x5000, 0x5fff) AM_RAM
	AM_RANGE(0x8000, 0x8000) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x8080, 0x8080) AM_WRITE(rocnrope_flipscreen_w)
	AM_RANGE(0x8081, 0x8081) AM_WRITE(timeplt_sh_irqtrigger_w)  /* cause interrupt on audio CPU */
	AM_RANGE(0x8082, 0x8082) AM_WRITENOP	/* interrupt acknowledge??? */
	AM_RANGE(0x8083, 0x8083) AM_WRITENOP	/* Coin counter 1 */
	AM_RANGE(0x8084, 0x8084) AM_WRITENOP	/* Coin counter 2 */
	AM_RANGE(0x8087, 0x8087) AM_WRITE(interrupt_enable_w)
	AM_RANGE(0x8100, 0x8100) AM_WRITE(soundlatch_w)
	AM_RANGE(0x8182, 0x818d) AM_WRITE(rocnrope_interrupt_vector_w)
	AM_RANGE(0x6000, 0xffff) AM_ROM
ADDRESS_MAP_END


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( rocnrope )
	PORT_START("SYSTEM")
	KONAMI8_SYSTEM_UNK

	PORT_START("P1")
	KONAMI8_MONO_4WAY_B12_UNK

	PORT_START("P2")
	KONAMI8_COCKTAIL_4WAY_B12_UNK

	PORT_START("DSW1")
	KONAMI_COINAGE(DEF_STR( Free_Play ), "No Coin B")
	/* "No Coin B" = coins produce sound, but no effect on coin counter */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "255 (Cheat)")
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x78, 0x78, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x78, "Easy 1" )
	PORT_DIPSETTING(    0x70, "Easy 2" )
	PORT_DIPSETTING(    0x68, "Easy 3" )
	PORT_DIPSETTING(    0x60, "Easy 4" )
	PORT_DIPSETTING(    0x58, "Normal 1" )
	PORT_DIPSETTING(    0x50, "Normal 2" )
	PORT_DIPSETTING(    0x48, "Normal 3" )
	PORT_DIPSETTING(    0x40, "Normal 4" )
	PORT_DIPSETTING(    0x38, "Normal 5" )
	PORT_DIPSETTING(    0x30, "Normal 6" )
	PORT_DIPSETTING(    0x28, "Normal 7" )
	PORT_DIPSETTING(    0x20, "Normal 8" )
	PORT_DIPSETTING(    0x18, "Difficult 1" )
	PORT_DIPSETTING(    0x10, "Difficult 2" )
	PORT_DIPSETTING(    0x08, "Difficult 3" )
	PORT_DIPSETTING(    0x00, "Difficult 4" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x07, 0x07, "First Bonus" )
	PORT_DIPSETTING(    0x07, "20000" )
	PORT_DIPSETTING(    0x05, "30000" )
	PORT_DIPSETTING(    0x04, "40000" )
	PORT_DIPSETTING(    0x03, "50000" )
	PORT_DIPSETTING(    0x02, "60000" )
	PORT_DIPSETTING(    0x01, "70000" )
	PORT_DIPSETTING(    0x00, "80000" )
	/* 0x06 gives 20000 */
	PORT_DIPNAME( 0x38, 0x38, "Repeated Bonus" )
	PORT_DIPSETTING(    0x38, "40000" )
	PORT_DIPSETTING(    0x18, "50000" )
	PORT_DIPSETTING(    0x10, "60000" )
	PORT_DIPSETTING(    0x08, "70000" )
	PORT_DIPSETTING(    0x00, "80000" )
	/* 0x20, 0x28 and 0x30 all gives 40000 */
	PORT_DIPNAME( 0x40, 0x00, "Grant Repeated Bonus" )
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x00, "Unknown DSW 8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,	/* 8*8 sprites */
	512,	/* 512 characters */
	4,	/* 4 bits per pixel */
	{ 0x2000*8+4, 0x2000*8+0, 4, 0 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8	/* every sprite takes 64 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	16,16,	/* 16*16 sprites */
	256,	/* 256 sprites */
	4,	/* 4 bits per pixel */
	{ 256*64*8+4, 256*64*8+0, 4, 0 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 24*8+0, 24*8+1, 24*8+2, 24*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8 },
	64*8	/* every sprite takes 64 consecutive bytes */
};

static GFXDECODE_START( rocnrope )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout,     0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, charlayout,   16*16, 16 )
GFXDECODE_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_DRIVER_START( rocnrope )

	/* driver data */
	MDRV_DRIVER_DATA(timeplt_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M6809, MASTER_CLOCK / 3 / 4)        /* Verified in schematics */
	MDRV_CPU_PROGRAM_MAP(rocnrope_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(rocnrope)
	MDRV_PALETTE_LENGTH(16*16+16*16)

	MDRV_PALETTE_INIT(rocnrope)
	MDRV_VIDEO_START(rocnrope)
	MDRV_VIDEO_UPDATE(rocnrope)

	/* sound hardware */
	MDRV_IMPORT_FROM(timeplt_sound)
MACHINE_DRIVER_END

/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

/*
    Roc'n Rope

    CPU/Video Board: KT-A207-1C
    Sound Board:     KT-2207-2A
*/

ROM_START( rocnrope )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rr1.1h",       0x6000, 0x2000, CRC(83093134) SHA1(c9509cfb9f9043cd6c226cc84dbc2e2b744488f6) )
	ROM_LOAD( "rr2.2h",       0x8000, 0x2000, CRC(75af8697) SHA1(70bb4b838cdafedf3d94425fad84f77815898d83) )
	ROM_LOAD( "rr3.3h",       0xa000, 0x2000, CRC(b21372b1) SHA1(c08ab3caaa646f4752f890d8339bce6b723864bb) )
	ROM_LOAD( "rr4.4h",       0xc000, 0x2000, CRC(7acb2a05) SHA1(93762d1890f40abc98372a2aa9fe0f63252b6389) )
	ROM_LOAD( "rnr_h5.vid",   0xe000, 0x2000, CRC(150a6264) SHA1(930ccf8dcf4971d0a15f406d9114be5ecfaa1727) )

	ROM_REGION( 0x10000, "tpsound", 0 )
	ROM_LOAD( "rnr_7a.snd",   0x0000, 0x1000, CRC(75d2c4e2) SHA1(b701019b4e7b06b268be660ce7958b5367318c27) )
	ROM_LOAD( "rnr_8a.snd",   0x1000, 0x1000, CRC(ca4325ae) SHA1(34ac035c0c2ed6bcafde1491d976bb9e9d2a2a7d) )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "rnr_a11.vid",  0x0000, 0x2000, CRC(afdaba5e) SHA1(27c090cb1c3767c997daeedbe1ba24786f9e78f1) )
	ROM_LOAD( "rnr_a12.vid",  0x2000, 0x2000, CRC(054cafeb) SHA1(4c3cd850b347217af3dd5c9bb84bcff7b30689bd) )
	ROM_LOAD( "rnr_a9.vid",   0x4000, 0x2000, CRC(9d2166b2) SHA1(42d2b05360e58b1b2b3ad06c98eb46d9da2b1c21) )
	ROM_LOAD( "rnr_a10.vid",  0x6000, 0x2000, CRC(aff6e22f) SHA1(476d67821519feddc9f9c8537b46e6eede790035) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "rnr_h12.vid",  0x0000, 0x2000, CRC(e2114539) SHA1(0ea19ae4d7c2da14f23c81abb8e2c931785b2715) )
	ROM_LOAD( "rnr_h11.vid",  0x2000, 0x2000, CRC(169a8f3f) SHA1(182c7c9b9849ebb57b3ff7c0b629f2f8e2efa9ba) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "a17_prom.bin", 0x0000, 0x0020, CRC(22ad2c3e) SHA1(1c2198b286c75aa9e78d000432795b1ce86ad6b9) )
	ROM_LOAD( "b16_prom.bin", 0x0020, 0x0100, CRC(750a9677) SHA1(7a5b4aed5f87180850657b8852bb3f3138d58b5b) )
	ROM_LOAD( "rocnrope.pr3", 0x0120, 0x0100, CRC(b5c75a27) SHA1(923d6ccf015fd7458494416cc05426cc922a9238) )

    ROM_REGION( 0x0001, "pal_cpuvidbd", 0 ) /* PAL located on the cpu/video board */
    ROM_LOAD( "h100.6g",      0x0000, 0x0001, NO_DUMP ) /* 20 Pin chip.  Appears to be a PAL.  Schematics obsfucated. */
ROM_END

ROM_START( rocnropek )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rnr_h1.vid",   0x6000, 0x2000, CRC(0fddc1f6) SHA1(a9c6c033799883dc45eaa448387d4f0728b9e47e) )
	ROM_LOAD( "rnr_h2.vid",   0x8000, 0x2000, CRC(ce9db49a) SHA1(7a6ffb65cb65aa90e953706ee67c6ae91322ebf6) )
	ROM_LOAD( "rnr_h3.vid",   0xa000, 0x2000, CRC(6d278459) SHA1(a1417f158f288b0b0cbffc58f9e22b6c7cdf0fc7) )
	ROM_LOAD( "rnr_h4.vid",   0xc000, 0x2000, CRC(9b2e5f2a) SHA1(e91d7a9141dbe0fc5eacc2c5a672935993a3316f) )
	ROM_LOAD( "rnr_h5.vid",   0xe000, 0x2000, CRC(150a6264) SHA1(930ccf8dcf4971d0a15f406d9114be5ecfaa1727) )

	ROM_REGION( 0x10000, "tpsound", 0 )
	ROM_LOAD( "rnr_7a.snd",   0x0000, 0x1000, CRC(75d2c4e2) SHA1(b701019b4e7b06b268be660ce7958b5367318c27) )
	ROM_LOAD( "rnr_8a.snd",   0x1000, 0x1000, CRC(ca4325ae) SHA1(34ac035c0c2ed6bcafde1491d976bb9e9d2a2a7d) )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "rnr_a11.vid",  0x0000, 0x2000, CRC(afdaba5e) SHA1(27c090cb1c3767c997daeedbe1ba24786f9e78f1) )
	ROM_LOAD( "rnr_a12.vid",  0x2000, 0x2000, CRC(054cafeb) SHA1(4c3cd850b347217af3dd5c9bb84bcff7b30689bd) )
	ROM_LOAD( "rnr_a9.vid",   0x4000, 0x2000, CRC(9d2166b2) SHA1(42d2b05360e58b1b2b3ad06c98eb46d9da2b1c21) )
	ROM_LOAD( "rnr_a10.vid",  0x6000, 0x2000, CRC(aff6e22f) SHA1(476d67821519feddc9f9c8537b46e6eede790035) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "rnr_h12.vid",  0x0000, 0x2000, CRC(e2114539) SHA1(0ea19ae4d7c2da14f23c81abb8e2c931785b2715) )
	ROM_LOAD( "rnr_h11.vid",  0x2000, 0x2000, CRC(169a8f3f) SHA1(182c7c9b9849ebb57b3ff7c0b629f2f8e2efa9ba) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "a17_prom.bin", 0x0000, 0x0020, CRC(22ad2c3e) SHA1(1c2198b286c75aa9e78d000432795b1ce86ad6b9) )
	ROM_LOAD( "b16_prom.bin", 0x0020, 0x0100, CRC(750a9677) SHA1(7a5b4aed5f87180850657b8852bb3f3138d58b5b) )
	ROM_LOAD( "rocnrope.pr3", 0x0120, 0x0100, CRC(b5c75a27) SHA1(923d6ccf015fd7458494416cc05426cc922a9238) )

    ROM_REGION( 0x0001, "pal_cpuvidbd", 0 ) /* PAL located on the cpu/video board */
    ROM_LOAD( "h100.6g",      0x0000, 0x0001, NO_DUMP ) /* 20 Pin chip.  Appears to be a PAL.  Schematics obsfucated. */
ROM_END



/*************************************
 *
 *  Driver initialization
 *
 *************************************/

static DRIVER_INIT( rocnrope )
{
	UINT8 *decrypted = konami1_decode(machine, "maincpu");

	decrypted[0x703d] = 0x98;	/* fix one instruction */
}

static DRIVER_INIT( rocnropk )
{
	konami1_decode(machine, "maincpu");
}


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1983, rocnrope, 0,        rocnrope, rocnrope, rocnrope, ROT270, "Konami", "Roc'n Rope", GAME_SUPPORTS_SAVE )
GAME( 1983, rocnropek,rocnrope, rocnrope, rocnrope, rocnropk, ROT270, "Konami / Kosuka", "Roc'n Rope (Kosuka)", GAME_SUPPORTS_SAVE )
