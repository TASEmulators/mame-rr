/***************************************************************************

  Tumblepop (World)     (c) 1991 Data East Corporation
  Tumblepop (Japan)     (c) 1991 Data East Corporation

Stephh's notes (based on the games M68000 code and some tests) :

1) 'tumblep*' and 'jumpkids'

  - I don't understand the interest of the "Remove Monsters" Dip Switch :
    as I haven't found a way to "end" a level, I guess that it was used to
    test the backgrounds and the "platforms".

  - The "Edit Levels" Dip Switch allows you to add/delete monsters and
    change their position.

    Notes (for 'tumblep', 'tumblepj', 'tumblep2') :
      * "worlds" and levels are 0-based (00-09 & 00-09) :

          World      Name
            0      America
            1      Brazil
            2      Asia
            3      Soviet
            4      Europe
            5      Egypt
            6      Australia
            7      Antartica
            8      Stratosphere
            9      Space

      * As levels x-9 and 9-x are only constitued of a "big boss", you can't
        edit them !
      * All data is stored within the range 0x02b8c8-0x02d2c9, but it should be
        extended to 0x02ebeb (and perhaps 0x02ffff). TO BE CONFIRMED !
      * Once your levels are ready, turn the Dip Switch OFF and reset the game.
      * Of course, there is no possibility to save the levels when you exit
        MAME, nor the way to reload the default ones 8(

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/h6280/h6280.h"
#include "includes/decocrpt.h"
#include "sound/2151intf.h"
#include "sound/3812intf.h"
#include "sound/okim6295.h"
#include "includes/tumblep.h"
#include "video/deco16ic.h"

#define TUMBLEP_HACK	0

/******************************************************************************/

#ifdef UNUSED_FUNCTION
static WRITE16_DEVICE_HANDLER( tumblep_oki_w )
{
	okim6295_w(0, data & 0xff);
    /* STUFF IN OTHER BYTE TOO..*/
}

static READ16_HANDLER( tumblep_prot_r )
{
	return ~0;
}
#endif

static WRITE16_HANDLER( tumblep_sound_w )
{
	tumblep_state *state = (tumblep_state *)space->machine->driver_data;
	soundlatch_w(space, 0, data & 0xff);
	cpu_set_input_line(state->audiocpu, 0, HOLD_LINE);
}

#ifdef UNUSED_FUNCTION
static WRITE16_HANDLER( jumppop_sound_w )
{
	tumblep_state *state = (tumblep_state *)space->machine->driver_data;
	soundlatch_w(space, 0, data & 0xff);
	cputag_set_input_line(state->audiocpu, 0, ASSERT_LINE );
}
#endif

/******************************************************************************/

static READ16_HANDLER( tumblepop_controls_r )
{
	switch (offset << 1)
	{
		case 0:
			return input_port_read(space->machine, "PLAYERS");
		case 2:
			return input_port_read(space->machine, "DSW");
		case 8:
			return input_port_read(space->machine, "SYSTEM");
		case 10: /* ? */
		case 12:
        	return 0;
	}

	return ~0;
}

/******************************************************************************/

static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 16 )
#if TUMBLEP_HACK
	AM_RANGE(0x000000, 0x07ffff) AM_WRITEONLY	// To write levels modifications
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
#else
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
#endif
	AM_RANGE(0x100000, 0x100001) AM_WRITE(tumblep_sound_w)
	AM_RANGE(0x120000, 0x123fff) AM_RAM
	AM_RANGE(0x140000, 0x1407ff) AM_RAM_WRITE(paletteram16_xxxxBBBBGGGGRRRR_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x180000, 0x18000f) AM_READ(tumblepop_controls_r)
	AM_RANGE(0x18000c, 0x18000d) AM_WRITENOP
	AM_RANGE(0x1a0000, 0x1a07ff) AM_RAM AM_BASE_SIZE_MEMBER(tumblep_state, spriteram, spriteram_size)
	AM_RANGE(0x300000, 0x30000f) AM_DEVWRITE("deco_custom", deco16ic_pf12_control_w)
	AM_RANGE(0x320000, 0x320fff) AM_DEVREADWRITE("deco_custom", deco16ic_pf1_data_r, deco16ic_pf1_data_w)
	AM_RANGE(0x322000, 0x322fff) AM_DEVREADWRITE("deco_custom", deco16ic_pf2_data_r, deco16ic_pf2_data_w)
	AM_RANGE(0x340000, 0x3407ff) AM_WRITEONLY AM_BASE_MEMBER(tumblep_state, pf1_rowscroll) // unused
	AM_RANGE(0x342000, 0x3427ff) AM_WRITEONLY AM_BASE_MEMBER(tumblep_state, pf2_rowscroll) // unused
ADDRESS_MAP_END

/******************************************************************************/

/* Physical memory map (21 bits) */
static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x000000, 0x00ffff) AM_ROM
	AM_RANGE(0x100000, 0x100001) AM_NOP	/* YM2203 - this board doesn't have one */
	AM_RANGE(0x110000, 0x110001) AM_DEVREADWRITE("ymsnd", ym2151_r, ym2151_w)
	AM_RANGE(0x120000, 0x120001) AM_DEVREADWRITE("oki", okim6295_r, okim6295_w)
	AM_RANGE(0x130000, 0x130001) AM_NOP	/* This board only has 1 oki chip */
	AM_RANGE(0x140000, 0x140001) AM_READ(soundlatch_r)
	AM_RANGE(0x1f0000, 0x1f1fff) AM_RAMBANK("bank8")
	AM_RANGE(0x1fec00, 0x1fec01) AM_WRITE(h6280_timer_w)
	AM_RANGE(0x1ff400, 0x1ff403) AM_WRITE(h6280_irq_status_w)
ADDRESS_MAP_END

/******************************************************************************/

static INPUT_PORTS_START( tumblep )
	PORT_START("PLAYERS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* button 3 - unused */
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* button 3 - unused */
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_VBLANK )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x00e0, 0x00e0, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x0080, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00e0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0060, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00a0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00c0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0040, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x001c, 0x001c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x001c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x000c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0014, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0018, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0008, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0001, 0x0001, "2 Coins to Start, 1 to Continue" )
	PORT_DIPSETTING(    0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x8000, "1" )
	PORT_DIPSETTING(    0x0000, "2" )
	PORT_DIPSETTING(    0xc000, "3" )
	PORT_DIPSETTING(    0x4000, "4" )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x1000, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x3000, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x2000, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Hardest ) )
#if TUMBLEP_HACK
	PORT_DIPNAME( 0x0800, 0x0800, "Remove Monsters" )
#else
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unused ) )		// See notes
#endif
	PORT_DIPSETTING(    0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
#if TUMBLEP_HACK
	PORT_DIPNAME( 0x0400, 0x0400, "Edit Levels" )
#else
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unused ) )		// See notes
#endif
	PORT_DIPSETTING(    0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(    0x0200, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
INPUT_PORTS_END

/******************************************************************************/

static const gfx_layout tile_8x8_layout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8,RGN_FRAC(1,2)+0,RGN_FRAC(0,2)+8,RGN_FRAC(0,2)+0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	8*16
};

static const gfx_layout tile_16x16_layout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8,RGN_FRAC(1,2)+0,RGN_FRAC(0,2)+8,RGN_FRAC(0,2)+0 },
	{ 256,257,258,259,260,261,262,263,0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,8*16,9*16,10*16,11*16,12*16,13*16,14*16,15*16 },
	32*16
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 24,8,16,0 },
	{ 512,513,514,515,516,517,518,519, 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
	  8*32, 9*32,10*32,11*32,12*32,13*32,14*32,15*32},
	32*32
};

static GFXDECODE_START( tumblep )
	GFXDECODE_ENTRY( "gfx1", 0, tile_8x8_layout,     0x100, 32 )	/* Tiles (8x8) */
	GFXDECODE_ENTRY( "gfx1", 0, tile_16x16_layout,   0x100, 32 )	/* Tiles (16x16) */
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout,        0, 16 )	/* Sprites (16x16) */
GFXDECODE_END

/***************************************************************************/

static void sound_irq(running_device *device, int state)
{
	tumblep_state *driver_state = (tumblep_state *)device->machine->driver_data;
	cpu_set_input_line(driver_state->audiocpu, 1, state); /* IRQ 2 */
}

static const ym2151_interface ym2151_config =
{
	sound_irq
};

static const deco16ic_interface tumblep_deco16ic_intf =
{
	"screen",
	1, 0, 1,
	0x0f, 0x0f, 0x0f, 0x0f,	/* trans masks (default values) */
	0, 16, 0, 16, /* color base (default values) */
	0x0f, 0x0f, 0x0f, 0x0f,	/* color masks (default values) */
	NULL, NULL, NULL, NULL
};

static MACHINE_START( tumblep )
{
	tumblep_state *state = (tumblep_state *)machine->driver_data;

	state->maincpu = machine->device("maincpu");
	state->audiocpu = machine->device("audiocpu");
	state->deco16ic = machine->device("deco_custom");
}

static MACHINE_DRIVER_START( tumblep )

	/* driver data */
	MDRV_DRIVER_DATA(tumblep_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 14000000)
	MDRV_CPU_PROGRAM_MAP(main_map)
	MDRV_CPU_VBLANK_INT("screen", irq6_line_hold)

	MDRV_CPU_ADD("audiocpu", H6280, 32220000/8)	/* Custom chip 45; Audio section crystal is 32.220 MHz */
	MDRV_CPU_PROGRAM_MAP(sound_map)

	MDRV_MACHINE_START(tumblep)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(58)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(529))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(40*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 40*8-2, 1*8, 31*8-1) // hmm

	MDRV_GFXDECODE(tumblep)
	MDRV_PALETTE_LENGTH(1024)

	MDRV_VIDEO_UPDATE(tumblep)

	MDRV_DECO16IC_ADD("deco_custom", tumblep_deco16ic_intf)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2151, 32220000/9)
	MDRV_SOUND_CONFIG(ym2151_config)
	MDRV_SOUND_ROUTE(0, "lspeaker", 0.45)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.45)

	MDRV_OKIM6295_ADD("oki", 1023924, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)
MACHINE_DRIVER_END

/******************************************************************************/

ROM_START( tumblep )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE("hl00-1.f12", 0x00000, 0x40000, CRC(fd697c1b) SHA1(1a3dee4c7383f2bc2d73037e80f8f5d8297e7433) )
	ROM_LOAD16_BYTE("hl01-1.f13", 0x00001, 0x40000, CRC(d5a62a3f) SHA1(7249563993fa8e1f19ddae51306d4a576b5cb206) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Sound cpu */
	ROM_LOAD( "hl02-.f16",    0x00000, 0x10000, CRC(a5cab888) SHA1(622f6adb01e31b8f3adbaed2b9900b54c5922c57) )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "map-02.rom",   0x00000, 0x80000, CRC(dfceaa26) SHA1(83e391ff39efda71e5fa368ac68ba7d6134bac21) )	// encrypted

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "map-01.rom",   0x00000, 0x80000, CRC(e81ffa09) SHA1(01ada9557ead91eb76cf00db118d6c432104a398) )
	ROM_LOAD16_BYTE( "map-00.rom",   0x00001, 0x80000, CRC(8c879cfe) SHA1(a53ef7811f14a8b105749b1cf29fe8a3a33bab5e) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Oki samples */
	ROM_LOAD( "hl03-.j15",    0x00000, 0x20000, CRC(01b81da0) SHA1(914802f3206dc59a720af9d57eb2285bc8ba822b) )
ROM_END

ROM_START( tumblepj )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE("hk00-1.f12", 0x00000, 0x40000, CRC(2d3e4d3d) SHA1(0acc8b93bd49395904dff11c582bdbaccdbd3eef) )
	ROM_LOAD16_BYTE("hk01-1.f13", 0x00001, 0x40000, CRC(56912a00) SHA1(0545f6bff2a0aa2f36adda0f9d73b165387abc3a) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Sound cpu */
	ROM_LOAD( "hl02-.f16",    0x00000, 0x10000, CRC(a5cab888) SHA1(622f6adb01e31b8f3adbaed2b9900b54c5922c57) )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "map-02.rom",   0x00000, 0x80000, CRC(dfceaa26) SHA1(83e391ff39efda71e5fa368ac68ba7d6134bac21) )	// encrypted

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "map-01.rom",   0x00000, 0x80000, CRC(e81ffa09) SHA1(01ada9557ead91eb76cf00db118d6c432104a398) )
	ROM_LOAD16_BYTE( "map-00.rom",   0x00001, 0x80000, CRC(8c879cfe) SHA1(a53ef7811f14a8b105749b1cf29fe8a3a33bab5e) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Oki samples */
	ROM_LOAD( "hl03-.j15",    0x00000, 0x20000, CRC(01b81da0) SHA1(914802f3206dc59a720af9d57eb2285bc8ba822b) )
ROM_END

/******************************************************************************/

#if TUMBLEP_HACK
void tumblep_patch_code(UINT16 offset)
{
	/* A hack which enables all Dip Switches effects */
	UINT16 *RAM = (UINT16 *)memory_region(machine, "maincpu");
	RAM[(offset + 0)/2] = 0x0240;
	RAM[(offset + 2)/2] = 0xffff;	// andi.w  #$f3ff, D0
}
#endif

static DRIVER_INIT( tumblep )
{
	deco56_decrypt_gfx(machine, "gfx1");

	#if TUMBLEP_HACK
	tumblep_patch_code(0x000132);
	#endif
}

/******************************************************************************/

GAME( 1991, tumblep,  0,       tumblep,   tumblep,  tumblep,  ROT0, "Data East Corporation", "Tumble Pop (World)", GAME_SUPPORTS_SAVE )
GAME( 1991, tumblepj, tumblep, tumblep,   tumblep,  tumblep,  ROT0, "Data East Corporation", "Tumble Pop (Japan)", GAME_SUPPORTS_SAVE )
/* for bootlegs and games on similar hardware see tumbleb.c */
