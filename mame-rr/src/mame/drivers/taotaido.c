/***************************************************************************

  Tao Taido             (c) 1993 Video System


    driver by David Haywood - Dip Switches and Inputs by stephh
    based on other Video System drivers

Stephh's notes (based on the games M68000 code and some tests) :

0) all games

  - Don't trust the test mode ! It shows inputs for 4 players as well as
    3 buttons for each player, while the game is a 2 players game with only
    2 buttons (punch and kick) for each player.
    IMO, it's a leftover from a previous game.
    If you want to test all inputs, turn TAOTAIDO_SHOW_ALL_INPUTS to 1.

  - The "Buy-In" features allows a player to recover some energy and reset timer
    by pressing his "Start" button (provided he has at least one credit).

  - 'taotaido' allows to play a 2 players game with a single credit while
    this isn't possible with 'taotaida'.
    Now, telling which version is the newest one is another story ;)

  - 'taotaido' seems to show you how to do the special moves, 'taotaida' doesn't
    and they don't seem to work in the same way (unless this is a bug)

  - Coin buttons act differently depending on the "Coin Slot" Dip Switch :

      * "Coin Slot" Dip Switch set to "Same" :

          . COIN1 : adds coin(s)/credit(s) depending on "Coinage" Dip Switch
          . COIN2 : adds 1 credit
          . SERVICE1 : adds coin(s)/credit(s) depending on "Coinage" Dip Switch

      * "Coin Slot" Dip Switch set to "Individual" :

          . COIN1 : adds coin(s)/credit(s) for player 1 depending on "Coinage" Dip Switch
          . COIN2 : adds coin(s)/credit(s) for player 2 depending on "Coinage" Dip Switch
          . SERVICE1 : adds 1 credit for player 1

***************************************************************************/

/* Tao Taido
(c)1993 Video System

CPU:    68000-16
Sound:  Z80-B
        YM2610
OSC:    14.31818MHz
        20.0000MHz
        32.0000MHz
Chips:  VS9108
        VS920B
        VS9209 x2

****************************************************************************

zooming might be wrong

***************************************************************************/



#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "sound/2610intf.h"
#include "includes/taotaido.h"

#define TAOTAIDO_SHOW_ALL_INPUTS	0

static int pending_command;

static READ16_HANDLER( pending_command_r )
{
	/* Only bit 0 is tested */
	return pending_command;
}

static WRITE16_HANDLER( sound_command_w )
{
	if (ACCESSING_BITS_0_7)
	{
		pending_command = 1;
		soundlatch_w(space, offset, data & 0xff);
		cputag_set_input_line(space->machine, "audiocpu", INPUT_LINE_NMI, PULSE_LINE);
	}
}
static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x800000, 0x803fff) AM_RAM_WRITE(taotaido_bgvideoram_w) AM_BASE(&taotaido_bgram)	// bg ram?
	AM_RANGE(0xa00000, 0xa01fff) AM_RAM AM_BASE(&taotaido_spriteram)		// sprite ram
	AM_RANGE(0xc00000, 0xc0ffff) AM_RAM AM_BASE(&taotaido_spriteram2)		// sprite tile lookup ram
	AM_RANGE(0xfe0000, 0xfeffff) AM_RAM										// main ram
	AM_RANGE(0xffc000, 0xffcfff) AM_RAM_WRITE(paletteram16_xRRRRRGGGGGBBBBB_word_w) AM_BASE_GENERIC(paletteram)	// palette ram
	AM_RANGE(0xffe000, 0xffe3ff) AM_RAM AM_BASE(&taotaido_scrollram)		// rowscroll / rowselect / scroll ram
	AM_RANGE(0xffff80, 0xffff81) AM_READ_PORT("P1")
	AM_RANGE(0xffff82, 0xffff83) AM_READ_PORT("P2")
	AM_RANGE(0xffff84, 0xffff85) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xffff86, 0xffff87) AM_READ_PORT("DSW1")
	AM_RANGE(0xffff88, 0xffff89) AM_READ_PORT("DSW2")
	AM_RANGE(0xffff8a, 0xffff8b) AM_READ_PORT("DSW3")
	AM_RANGE(0xffff8c, 0xffff8d) AM_READONLY						// unknown
	AM_RANGE(0xffff8e, 0xffff8f) AM_READ_PORT("JP")
	AM_RANGE(0xffffa0, 0xffffa1) AM_READ_PORT("P3")						// used only by taotaida
	AM_RANGE(0xffffa2, 0xffffa3) AM_READ_PORT("P4")						// used only by taotaida
	AM_RANGE(0xffff00, 0xffff0f) AM_WRITE(taotaido_tileregs_w)
	AM_RANGE(0xffff10, 0xffff11) AM_WRITENOP						// unknown
	AM_RANGE(0xffff20, 0xffff21) AM_WRITENOP						// unknown - flip screen related
	AM_RANGE(0xffff40, 0xffff47) AM_WRITE(taotaido_sprite_character_bank_select_w)
	AM_RANGE(0xffffc0, 0xffffc1) AM_WRITE(sound_command_w)				// seems right
	AM_RANGE(0xffffe0, 0xffffe1) AM_READ(pending_command_r)	// guess - seems to be needed for all the sounds to work
ADDRESS_MAP_END

/* sound cpu - same as aerofgt */


static WRITE8_HANDLER( pending_command_clear_w )
{
	pending_command = 0;
}

static WRITE8_HANDLER( taotaido_sh_bankswitch_w )
{
	UINT8 *rom = memory_region(space->machine, "audiocpu") + 0x10000;

	memory_set_bankptr(space->machine, "bank1",rom + (data & 0x03) * 0x8000);
}

static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x77ff) AM_ROM
	AM_RANGE(0x7800, 0x7fff) AM_RAM
	AM_RANGE(0x8000, 0xffff) AM_ROMBANK("bank1")
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_port_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("ymsnd", ym2610_r, ym2610_w)
	AM_RANGE(0x04, 0x04) AM_WRITE(taotaido_sh_bankswitch_w)
	AM_RANGE(0x08, 0x08) AM_WRITE(pending_command_clear_w)
	AM_RANGE(0x0c, 0x0c) AM_READ(soundlatch_r)
ADDRESS_MAP_END


static INPUT_PORTS_START( taotaido )
	PORT_START("P1")	/* 0xffff81.b */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)	// "Punch"
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)	// "Kick"
#if TAOTAIDO_SHOW_ALL_INPUTS
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
#else
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
#endif
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")	/* 0xffff83.b */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)	// "Punch"
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)	// "Kick"
#if TAOTAIDO_SHOW_ALL_INPUTS
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
#else
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
#endif
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	/* These inputs are only to fit the test mode - leftover from another game ? */
	PORT_START("P3")	/* 0xffffa1.b */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )
#if TAOTAIDO_SHOW_ALL_INPUTS
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
#endif

	PORT_START("P4")	/* 0xffffa3.b */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )
#if TAOTAIDO_SHOW_ALL_INPUTS
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
#endif

	PORT_START("SYSTEM")	/* 0xffff85.b */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )	// see notes
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
#if TAOTAIDO_SHOW_ALL_INPUTS
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F1)
#else
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )	// "Test" in "test mode"
#endif
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_TILT )		// not working ?
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )	// see notes - SERVICE in "test mode"
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )	// VBLANK ? The game freezes when ON

	PORT_START("DSW1")	/* 0xffff87.b -> !0xfe2f6c.w or !0xfe30d0 */
	PORT_DIPNAME( 0x01, 0x01, "Coin Slot" )
	PORT_DIPSETTING(    0x01, "Same" )
	PORT_DIPSETTING(    0x00, "Individual" )
	PORT_DIPNAME( 0x0e, 0x0e, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x80, 0x80, "2 Coins to Start, 1 to Continue" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")	/* 0xffff89.b -> !0xfe73c2.w or !0xfe751c.w */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )	// check code at 0x0963e2 or 0x845e2
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED( 0x02, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_SERVICE( 0x20, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )

	PORT_START("DSW3")	/* 0xffff8b.b -> !0xfe2f94.w or !0xfe30f8.w */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )	// doesn't seem to be demo sounds
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Buy In" )			// see notes
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPUNUSED( 0x04, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x08, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )

	PORT_START("JP")	/* Jumpers (0xffff8f.b) */
	PORT_DIPNAME( 0x0f, 0x08, "Country" )
	PORT_DIPSETTING(    0x00, "US" )				// also (c) Mc O'River Inc
	PORT_DIPSETTING(    0x01, DEF_STR( Japan ) )
	PORT_DIPSETTING(    0x02, "Hong-Kong/Taiwan" )
//  PORT_DIPSETTING(    0x03, DEF_STR( Japan ) )
	PORT_DIPSETTING(    0x04, "Korea" )
//  PORT_DIPSETTING(    0x05, DEF_STR( Japan ) )
//  PORT_DIPSETTING(    0x06, DEF_STR( Japan ) )
//  PORT_DIPSETTING(    0x07, DEF_STR( Japan ) )
	PORT_DIPSETTING(    0x08, DEF_STR( World ) )
	/* 0x09 to 0x0f : DEF_STR( Japan ) */
INPUT_PORTS_END

static INPUT_PORTS_START( taotaida )
	PORT_INCLUDE(taotaido)

	PORT_MODIFY("P1")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)

	PORT_MODIFY("P2")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_MODIFY("P3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("P4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static const gfx_layout taotaido_layout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 1*4, 0*4, 3*4, 2*4, 5*4, 4*4, 7*4, 6*4,
			9*4, 8*4, 11*4, 10*4, 13*4, 12*4, 15*4, 14*4 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
			8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	128*8
};

static GFXDECODE_START( taotaido )
	GFXDECODE_ENTRY( "gfx1", 0, taotaido_layout,  0x000, 256  ) /* sprites */
	GFXDECODE_ENTRY( "gfx2", 0, taotaido_layout,  0x300, 256  ) /* bg tiles */
GFXDECODE_END

static void irqhandler(running_device *device, int irq)
{
	cputag_set_input_line(device->machine, "audiocpu", 0, irq ? ASSERT_LINE : CLEAR_LINE);
}

static const ym2610_interface ym2610_config =
{
	irqhandler
};

static MACHINE_DRIVER_START( taotaido )
	MDRV_CPU_ADD("maincpu", M68000, 32000000/2)
	MDRV_CPU_PROGRAM_MAP(main_map)
	MDRV_CPU_VBLANK_INT("screen", irq1_line_hold)

	MDRV_CPU_ADD("audiocpu", Z80,20000000/4) // ??
	MDRV_CPU_PROGRAM_MAP(sound_map)
	MDRV_CPU_IO_MAP(sound_port_map)
								/* IRQs are triggered by the YM2610 */

	MDRV_GFXDECODE(taotaido)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(40*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0*8, 28*8-1)

	MDRV_PALETTE_LENGTH(0x800)

	MDRV_VIDEO_START(taotaido)
	MDRV_VIDEO_UPDATE(taotaido)
	MDRV_VIDEO_EOF( taotaido )

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2610, 8000000)
	MDRV_SOUND_CONFIG(ym2610_config)
	MDRV_SOUND_ROUTE(0, "lspeaker",  0.25)
	MDRV_SOUND_ROUTE(0, "rspeaker", 0.25)
	MDRV_SOUND_ROUTE(1, "lspeaker",  1.0)
	MDRV_SOUND_ROUTE(2, "rspeaker", 1.0)
MACHINE_DRIVER_END


ROM_START( taotaido )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "1-u90.bin", 0x00000, 0x80000, CRC(a3ee30da) SHA1(920a83ce9192bf785bffdc041e280f1a420de4c9) )
	ROM_LOAD16_WORD_SWAP( "2-u91.bin", 0x80000, 0x80000, CRC(30b7e4fb) SHA1(15e1f6d252c736fdee33b691a0a1a45f0307bffb) )

	ROM_REGION( 0x30000, "audiocpu", 0 ) /* z80 Code */
	ROM_LOAD( "3-u113.bin", 0x000000, 0x20000, CRC(a167c4e4) SHA1(d32184e7040935cd440d4d82c66491b710ec87a8) )
	ROM_RELOAD ( 0x10000, 0x20000 )

	ROM_REGION( 0x100000, "ymsnd.deltat", 0 ) /* sound samples */
	ROM_LOAD( "u104.bin",     0x000000, 0x100000, CRC(e89387a9) SHA1(1deeee056af367d1a5aa0722dd3d6c68a82d0489) )

	ROM_REGION( 0x200000, "ymsnd", 0 ) /* sound samples */
	ROM_LOAD( "u127.bin",     0x00000, 0x200000, CRC(0cf0cb23) SHA1(a87e7159db2fa0d50446cbf45ec9fbf585b8f396) )

	ROM_REGION( 0x600000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD( "u86.bin", 0x000000, 0x200000, CRC(908e251e) SHA1(5a135787f3263bfb195f8fd1e814c580d840531f) )
	ROM_LOAD( "u87.bin", 0x200000, 0x200000, CRC(c4290ba6) SHA1(4132ffad4668f1dd3f708f009e18435e7dd60120) )
	ROM_LOAD( "u88.bin", 0x400000, 0x200000, CRC(407d9aeb) SHA1(d532c7b80f6c192dba86542fb6eb3ef24fbbbdb9) )

	ROM_REGION( 0x200000, "gfx2", 0 ) /* BG Tiles */
	ROM_LOAD( "u15.bin", 0x000000, 0x200000, CRC(e95823e9) SHA1(362583944ad4fdde4f9e29928cf34376c7ad931f) )
ROM_END

ROM_START( taotaidoa )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "tt0-u90.bin", 0x00000, 0x80000, CRC(69d4cca7) SHA1(f1aba74fef8fe4271d19763f428fc0e2674d08b3) )
	ROM_LOAD16_WORD_SWAP( "tt1-u91.bin", 0x80000, 0x80000, CRC(41025469) SHA1(fa3a424ca3ecb513f418e436e4191ff76f6a0de1) )

	ROM_REGION( 0x30000, "audiocpu", 0 ) /* z80 Code */
	ROM_LOAD( "3-u113.bin", 0x000000, 0x20000, CRC(a167c4e4) SHA1(d32184e7040935cd440d4d82c66491b710ec87a8) )
	ROM_RELOAD ( 0x10000, 0x20000 )

	ROM_REGION( 0x100000, "ymsnd.deltat", 0 ) /* sound samples */
	ROM_LOAD( "u104.bin",     0x000000, 0x100000, CRC(e89387a9) SHA1(1deeee056af367d1a5aa0722dd3d6c68a82d0489) )

	ROM_REGION( 0x200000, "ymsnd", 0 ) /* sound samples */
	ROM_LOAD( "u127.bin",     0x00000, 0x200000, CRC(0cf0cb23) SHA1(a87e7159db2fa0d50446cbf45ec9fbf585b8f396) )

	ROM_REGION( 0x600000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD( "u86.bin", 0x000000, 0x200000, CRC(908e251e) SHA1(5a135787f3263bfb195f8fd1e814c580d840531f) )
	ROM_LOAD( "u87.bin", 0x200000, 0x200000, CRC(c4290ba6) SHA1(4132ffad4668f1dd3f708f009e18435e7dd60120) )
	ROM_LOAD( "u88.bin", 0x400000, 0x200000, CRC(407d9aeb) SHA1(d532c7b80f6c192dba86542fb6eb3ef24fbbbdb9) )

	ROM_REGION( 0x200000, "gfx2", 0 ) /* BG Tiles */
	ROM_LOAD( "u15.bin", 0x000000, 0x200000, CRC(e95823e9) SHA1(362583944ad4fdde4f9e29928cf34376c7ad931f) )
ROM_END

GAME( 1993, taotaido, 0,        taotaido, taotaido, 0, ROT0, "Video System Co.", "Tao Taido (set 1)", GAME_NO_COCKTAIL )
GAME( 1993, taotaidoa,taotaido, taotaido, taotaida, 0, ROT0, "Video System Co.", "Tao Taido (set 2)", GAME_NO_COCKTAIL )
