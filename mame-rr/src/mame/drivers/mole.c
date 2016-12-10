/*****************************************************************************

   MOLE ATTACK by Yachiyo Electronics Co.,LTD. 1982

   Known Clones:
   "Holey Moley", from tai (Thomas Automatics, Inc.)

   emulated by Jason Nelson, Phil Stroffolino

   Known Issues:
   - some dips not mapped
   - protection isn't fully understood, but game seems to be ok.
   - cpu speeds unverified

   Buttons are laid out as follows:
   7   8   9
   4   5   6
   1   2   3

   Working RAM notes:
   0x2e0                    number of credits
   0x2F1                    coin up trigger
   0x2F2                    round counter
   0x2F3                    flag value
   0x2FD                    hammer aim for attract mode
   0x2E1-E2                 high score
   0x2ED-EE                 score
   0x301-309                presence and height of mole in each hole, from bottom left
   0x30a
   0x32E-336                if a hammer is above a mole. (Not the same as collision)
   0x337                    dip switch related
   0x338                    dip switch related
   0x340                    hammer control: manual=0; auto=1
   0x34C                    round point 10s.
   0x34D                    which bonus round pattern to use for moles.
   0x349                    button pressed (0..8 / 0xff)
   0x350                    number of players
   0x351                    irq-related
   0x361
   0x362
   0x363
   0x364
   0x366                    mirrors tile bank
   0x36B                    controls which player is playing. (1 == player 2);
   0x3DC                    affects mole popmessage
   0x3E5                    round point/passing point control?
   0x3E7                    round point/passing point control?

******************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "sound/ay8910.h"


class mole_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, mole_state(machine)); }

	mole_state(running_machine &machine) { }

	/* memory pointers */
	UINT16 *     tileram;

	/* video-related */
	tilemap_t    *bg_tilemap;
	int          tile_bank;
};


/*************************************
 *
 *  Video emulation
 *
 *************************************/

static PALETTE_INIT( mole )
{
	int i;

	for (i = 0; i < 8; i++)
		palette_set_color_rgb(machine, i, pal1bit(i >> 0), pal1bit(i >> 2), pal1bit(i >> 1));
}

static TILE_GET_INFO( get_bg_tile_info )
{
	mole_state *state = (mole_state *)machine->driver_data;
	UINT16 code = state->tileram[tile_index];

	SET_TILE_INFO((code & 0x200) ? 1 : 0, code & 0x1ff, 0, 0);
}

static VIDEO_START( mole )
{
	mole_state *state = (mole_state *)machine->driver_data;
	state->tileram = auto_alloc_array_clear(machine, UINT16, 0x400);
	state->bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 40, 25);

	state_save_register_global_pointer(machine, state->tileram, 0x400);
}

static WRITE8_HANDLER( mole_videoram_w )
{
	mole_state *state = (mole_state *)space->machine->driver_data;

	state->tileram[offset] = data | (state->tile_bank << 8);
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

static WRITE8_HANDLER( mole_tilebank_w )
{
	mole_state *state = (mole_state *)space->machine->driver_data;

	state->tile_bank = data;
	tilemap_mark_all_tiles_dirty(state->bg_tilemap);
}

static WRITE8_HANDLER( mole_flipscreen_w )
{
	flip_screen_set(space->machine, data & 0x01);
}

static VIDEO_UPDATE( mole )
{
	mole_state *state = (mole_state *)screen->machine->driver_data;

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	return 0;
}



/*************************************
 *
 *  Memory handlers
 *
 *************************************/

static READ8_HANDLER( mole_protection_r )
{

    /*  Following are all known examples of Mole Attack
    **  code reading from the protection circuitry:
    **
    **  5b0b:
    **  ram[0x0361] = (ram[0x885+ram[0x8a5])&ram[0x886]
    **  ram[0x0363] = ram[0x886]
    **
    **  53c9:
    **  ram[0xe0] = ram[0x800]+ram[0x802]+ram[0x804]
    **  ram[0xea] = ram[0x828]
    **
    **  ram[0xe2] = (ram[0x806]&ram[0x826])|ram[0x820]
    **  ram[0xe3] = ram[0x826]
    **
    **  ram[0x361] = (ram[0x8cd]&ram[0x8ad])|ram[0x8ce]
    **  ram[0x362] = ram[0x8ae] = 0x32
    **
    **  ram[0x363] = ram[0x809]+ram[0x829]+ram[0x828]
    **  ram[0x364] = ram[0x808]
    */

	switch (offset)
	{
	case 0x08: return 0xb0; /* random mole placement */
	case 0x26:
		if (cpu_get_pc(space->cpu) == 0x53d7)
		{
			return 0x06; /* bonus round */
		}
		else
		{ // pc == 0x515b, 0x5162
			return 0xc6; /* game start */
		}
	case 0x86: return 0x91; /* game over */
	case 0xae: return 0x32; /* coinage */
	}

    /*  The above are critical protection reads.
    **  It isn't clear what effect (if any) the
    **  remaining reads have; for now we simply
    **  return 0x00
    */

	return 0x00;
}


/*************************************
 *
 *  Address maps
 *
 *************************************/

static ADDRESS_MAP_START( mole_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x03ff) AM_RAM
	AM_RANGE(0x0800, 0x08ff) AM_READ(mole_protection_r)
	AM_RANGE(0x0800, 0x0800) AM_WRITENOP // ???
	AM_RANGE(0x0820, 0x0820) AM_WRITENOP // ???
	AM_RANGE(0x5000, 0x7fff) AM_MIRROR(0x8000) AM_ROM
	AM_RANGE(0x8000, 0x83ff) AM_RAM_WRITE(mole_videoram_w) AM_BASE_GENERIC(videoram)
	AM_RANGE(0x8400, 0x8400) AM_WRITE(mole_tilebank_w)
	AM_RANGE(0x8c00, 0x8c01) AM_DEVWRITE("aysnd", ay8910_data_address_w)
	AM_RANGE(0x8c40, 0x8c40) AM_WRITENOP // ???
	AM_RANGE(0x8c80, 0x8c80) AM_WRITENOP // ???
	AM_RANGE(0x8c81, 0x8c81) AM_WRITENOP // ???
	AM_RANGE(0x8d00, 0x8d00) AM_READ_PORT("DSW") AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x8d40, 0x8d40) AM_READ_PORT("IN0")
	AM_RANGE(0x8d80, 0x8d80) AM_READ_PORT("IN1")
	AM_RANGE(0x8dc0, 0x8dc0) AM_READ_PORT("IN2") AM_WRITE(mole_flipscreen_w)
ADDRESS_MAP_END


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( mole )
	PORT_START("DSW")	/* 0x8d00 */
	PORT_DIPNAME( 0x01, 0x00, "Round Points" )
	PORT_DIPSETTING(	0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(	0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(	0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	0x00, "A" )
	PORT_DIPSETTING(	0x04, "B" )
	PORT_DIPSETTING(	0x08, "C" )
	PORT_DIPSETTING(	0x0c, "D" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	0x00, "A" )
	PORT_DIPSETTING(	0x10, "B" )
	PORT_DIPSETTING(	0x20, "C" )
	PORT_DIPSETTING(	0x30, "D" )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN0")	/* 0x8d40 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 Pad 1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 Pad 2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 Pad 3") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 Pad 4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 Pad 5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 Pad 6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 Pad 7") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 Pad 8") PORT_CODE(KEYCODE_8_PAD)

	PORT_START("IN1")	/* 0x8d80 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 Pad 9") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Pad 1") PORT_CODE(KEYCODE_Q) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Pad 2") PORT_CODE(KEYCODE_W) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Pad 3") PORT_CODE(KEYCODE_E) PORT_COCKTAIL
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING( 0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING( 0x10, DEF_STR( Cocktail ) )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("IN2")	/* 0x8dc0 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Pad 8") PORT_CODE(KEYCODE_X) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Pad 7") PORT_CODE(KEYCODE_Z) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Pad 4") PORT_CODE(KEYCODE_A) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Pad 9") PORT_CODE(KEYCODE_C) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Pad 6") PORT_CODE(KEYCODE_D) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Pad 5") PORT_CODE(KEYCODE_S) PORT_COCKTAIL
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout tile_layout =
{
	8,8,	/* character size */
	512,	/* number of characters */
	3,		/* number of bitplanes */
	{ 0x0000*8, 0x1000*8, 0x2000*8 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


static GFXDECODE_START( mole )
	GFXDECODE_ENTRY( "gfx1", 0x0000, tile_layout, 0x00, 1 )
	GFXDECODE_ENTRY( "gfx1", 0x3000, tile_layout, 0x00, 1 )
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_START( mole )
{
	mole_state *state = (mole_state *)machine->driver_data;

	state_save_register_global(machine, state->tile_bank);
}

static MACHINE_RESET( mole )
{
	mole_state *state = (mole_state *)machine->driver_data;

	state->tile_bank = 0;
}

static MACHINE_DRIVER_START( mole )

	/* driver data */
	MDRV_DRIVER_DATA(mole_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M6502, 4000000) // ???
	MDRV_CPU_PROGRAM_MAP(mole_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_MACHINE_START(mole)
	MDRV_MACHINE_RESET(mole)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(40*8, 25*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0*8, 25*8-1)

	MDRV_GFXDECODE(mole)
	MDRV_PALETTE_LENGTH(8)

	MDRV_PALETTE_INIT(mole)
	MDRV_VIDEO_START(mole)
	MDRV_VIDEO_UPDATE(mole)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("aysnd", AY8910, 2000000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( mole ) // ALL ROMS ARE 2732
	ROM_REGION( 0x10000, "maincpu", 0 )	// 64k for 6502 code
	ROM_LOAD( "m3a.5h",	0x5000, 0x1000, CRC(5fbbdfef) SHA1(8129e90a05b3ca50f47f7610eec51c16c8609590) )
	ROM_LOAD( "m2a.7h",	0x6000, 0x1000, CRC(f2a90642) SHA1(da6887725d70924fc4b9cca83172276976f5020c) )
	ROM_LOAD( "m1a.8h",	0x7000, 0x1000, CRC(cff0119a) SHA1(48fc81b8c68e977680e7b8baf1193f0e7e0cd013) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "mea.4a",	0x0000, 0x1000, CRC(49d89116) SHA1(aa4cde07e10624072e50ba5bd209acf93092cf78) )
	ROM_LOAD( "mca.6a",	0x1000, 0x1000, CRC(04e90300) SHA1(c908a3a651e50428eedc2974160cdbf2ed946abc) )
	ROM_LOAD( "maa.9a",	0x2000, 0x1000, CRC(6ce9442b) SHA1(c08bf0911f1dfd4a3f9452efcbb3fd3688c4bf8c) )
	ROM_LOAD( "mfa.3a",	0x3000, 0x1000, CRC(0d0c7d13) SHA1(8a6d371571391f2b54ffa65b77e4e83fd607d2c9) )
	ROM_LOAD( "mda.5a",	0x4000, 0x1000, CRC(41ae1842) SHA1(afc169c3245b0946ef81e65d0b755d498ee71667) )
	ROM_LOAD( "mba.8a",	0x5000, 0x1000, CRC(50c43fc9) SHA1(af478f3d89cd6c87f32dcdda7fabce25738c340b) )
ROM_END


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1982, mole, 0, mole, mole, 0, ROT0, "Yachiyo Electronics, Ltd.", "Mole Attack", GAME_SUPPORTS_SAVE )
