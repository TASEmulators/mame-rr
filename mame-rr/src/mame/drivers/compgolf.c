/*******************************************************************************************

    Competition Golf Final Round (c) 1986 / 1985 Data East

    Driver by Angelo Salese, Bryan McPhail and Pierpaolo Prazzoli
    Thanks to David Haywood for the bg roms expansion

    Nb:  The black border around the player sprite in attract mode happens on the real pcb
    as well.

*******************************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "sound/2203intf.h"
#include "includes/compgolf.h"


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

static WRITE8_DEVICE_HANDLER( compgolf_scrollx_lo_w )
{
	compgolf_state *state = (compgolf_state *)device->machine->driver_data;
	state->scrollx_lo = data;
}

static WRITE8_DEVICE_HANDLER( compgolf_scrolly_lo_w )
{
	compgolf_state *state = (compgolf_state *)device->machine->driver_data;
	state->scrolly_lo = data;
}

static WRITE8_HANDLER( compgolf_ctrl_w )
{
	compgolf_state *state = (compgolf_state *)space->machine->driver_data;

	/* bit 4 and 6 are always set */

	int new_bank = (data & 4) >> 2;

	if (state->bank != new_bank)
	{
		state->bank = new_bank;
		memory_set_bank(space->machine, "bank1", state->bank);
	}

	state->scrollx_hi = (data & 1) << 8;
	state->scrolly_hi = (data & 2) << 7;
}


/*************************************
 *
 *  Address maps
 *
 *************************************/

static ADDRESS_MAP_START( compgolf_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_RAM
	AM_RANGE(0x1000, 0x17ff) AM_RAM_WRITE(compgolf_video_w) AM_BASE_MEMBER(compgolf_state, videoram)
	AM_RANGE(0x1800, 0x1fff) AM_RAM_WRITE(compgolf_back_w) AM_BASE_MEMBER(compgolf_state, bg_ram)
	AM_RANGE(0x2000, 0x2060) AM_RAM AM_BASE_MEMBER(compgolf_state, spriteram)
	AM_RANGE(0x2061, 0x2061) AM_WRITENOP
	AM_RANGE(0x3000, 0x3000) AM_READ_PORT("P1")
	AM_RANGE(0x3001, 0x3001) AM_READ_PORT("P2") AM_WRITE(compgolf_ctrl_w)
	AM_RANGE(0x3002, 0x3002) AM_READ_PORT("DSW1")
	AM_RANGE(0x3003, 0x3003) AM_READ_PORT("DSW2")
	AM_RANGE(0x3800, 0x3801) AM_DEVREADWRITE("ymsnd", ym2203_r, ym2203_w)
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank1")
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( compgolf )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_VBLANK )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03,   0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c,   0x0c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x04, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x10,   0x10, "Wind Force" )
	PORT_DIPSETTING(      0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x00, "Strong" )
	PORT_DIPNAME( 0x20,   0x20, "Grain of Turf" )
	PORT_DIPSETTING(      0x20, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x00, "Strong" )
	PORT_DIPNAME( 0x40,   0x40, "Range of Ball" )
	PORT_DIPSETTING(      0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x00, "Less" )
	PORT_DIPNAME( 0x80,   0x80, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01,   0x01, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02,   0x02, "Freeze" ) // this is more likely a switch...
	PORT_DIPSETTING(      0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04,   0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(      0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x08,   0x08, DEF_STR( Unused ) ) /* Manual states dips 4-8 are "Unused" */
	PORT_DIPSETTING(      0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10,   0x10, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )
INPUT_PORTS_END


/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ STEP8(8*8*2,1), STEP8(8*8*0,1) },
	{ STEP8(8*8*0,8), STEP8(8*8*1,8) },
	16*16
};

static const gfx_layout tilelayoutbg =
{
	16,16,
	RGN_FRAC(1,2),
	3,
	{ RGN_FRAC(1,2)+0, 0, 4 },
	{ 0, 1, 2, 3, 16*8+0, 16*8+1, 16*8+2, 16*8+3,
		2*16*8+0, 2*16*8+1, 2*16*8+2, 2*16*8+3, 3*16*8+0, 3*16*8+1, 3*16*8+2, 3*16*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
	  8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*16
};

static const gfx_layout tilelayout8 =
{
	8,8,
	RGN_FRAC(1,2),
	3,
	{ RGN_FRAC(1,2)+4, 0, 4 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8
};

static GFXDECODE_START( compgolf )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout, 0, 0x10 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayoutbg, 0, 0x20 )
	GFXDECODE_ENTRY( "gfx3", 0, tilelayout8,  0, 0x10 )
GFXDECODE_END

/*************************************
 *
 *  Sound interface
 *
 *************************************/

static void sound_irq(running_device *device, int linestate)
{
	cputag_set_input_line(device->machine, "maincpu", 0, linestate);
}

static const ym2203_interface ym2203_config =
{
	{
			AY8910_LEGACY_OUTPUT,
			AY8910_DEFAULT_LOADS,
			DEVCB_NULL,
			DEVCB_NULL,
			DEVCB_HANDLER(compgolf_scrollx_lo_w),
			DEVCB_HANDLER(compgolf_scrolly_lo_w),
	},
	sound_irq
};


/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_START( compgolf )
{
	compgolf_state *state = (compgolf_state *)machine->driver_data;

	state_save_register_global(machine, state->bank);
	state_save_register_global(machine, state->scrollx_lo);
	state_save_register_global(machine, state->scrollx_hi);
	state_save_register_global(machine, state->scrolly_lo);
	state_save_register_global(machine, state->scrolly_hi);
}

static MACHINE_RESET( compgolf )
{
	compgolf_state *state = (compgolf_state *)machine->driver_data;

	state->bank = -1;
	state->scrollx_lo = 0;
	state->scrollx_hi = 0;
	state->scrolly_lo = 0;
	state->scrolly_hi = 0;
}

static MACHINE_DRIVER_START( compgolf )

	/* driver data */
	MDRV_DRIVER_DATA(compgolf_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M6809, 2000000)
	MDRV_CPU_PROGRAM_MAP(compgolf_map)
	MDRV_CPU_VBLANK_INT("screen", nmi_line_pulse)

	MDRV_MACHINE_START(compgolf)
	MDRV_MACHINE_RESET(compgolf)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(1*8, 32*8-1, 1*8, 31*8-1)

	MDRV_PALETTE_LENGTH(0x100)
	MDRV_PALETTE_INIT(compgolf)
	MDRV_GFXDECODE(compgolf)

	MDRV_VIDEO_START(compgolf)
	MDRV_VIDEO_UPDATE(compgolf)

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ymsnd", YM2203, 1500000)
	MDRV_SOUND_CONFIG(ym2203_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( compgolf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cv05-3.bin",   0x08000, 0x8000, CRC(af9805bf) SHA1(bdde482906bb267e76317067785ac0ab7816df63) )

	ROM_REGION( 0x8000, "user1", 0 ) // background data
	ROM_LOAD( "cv06.bin",     0x00000, 0x8000, CRC(8f76979d) SHA1(432f6a1402fd3276669f5f45f03fd12380900178) )

	ROM_REGION( 0x18000, "gfx1", 0 ) // Sprites
	ROM_LOAD( "cv00.bin",     0x00000, 0x8000, CRC(aa3d3b99) SHA1(eb968e40bcc7e7dd1acc0bbe885fd3f7d70d4bb5) )
	ROM_LOAD( "cv01.bin",     0x08000, 0x8000, CRC(f68c2ff6) SHA1(dda9159fb59d3855025b98c272722b031617c89a) )
	ROM_LOAD( "cv02.bin",     0x10000, 0x8000, CRC(979cdb5a) SHA1(25c1f3e6ddf50168c7e1a967bfa2753bea6106ec) )

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "cv03.bin",     0x00000, 0x8000, CRC(cc7ed6d8) SHA1(4ffcfa3f720414e1b7e929bdf29359ebcd8717c3) )
	/* we expand rom cv04.bin to 0x8000 - 0xffff */

	ROM_REGION( 0x8000,  "gfx3", 0 )
	ROM_LOAD( "cv07.bin",     0x00000, 0x8000, CRC(ed5441ba) SHA1(69d50695e8b92544f9857c6f3de0efb399899a2c) )

	ROM_REGION( 0x4000, "gfx4", 0 )
	ROM_LOAD( "cv04.bin",     0x00000, 0x4000, CRC(df693a04) SHA1(45bef98c7e66881f8c62affecc1ab90dd2707240) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cv08-1.bpr",   0x00000, 0x0100, CRC(b7c43db9) SHA1(418b11e4c8a9bce6873b0624ac53a5011c5807d0) )
ROM_END

ROM_START( compgolfo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cv05.bin",     0x08000, 0x8000, CRC(3cef62c9) SHA1(c4827b45faf7aa4c80ddd3c57f1ed6ba76b5c49b) )

	ROM_REGION( 0x8000, "user1", 0 ) // background data
	ROM_LOAD( "cv06.bin",     0x00000, 0x8000, CRC(8f76979d) SHA1(432f6a1402fd3276669f5f45f03fd12380900178) )

	ROM_REGION( 0x18000, "gfx1", 0 ) // Sprites
	ROM_LOAD( "cv00.bin",     0x00000, 0x8000, CRC(aa3d3b99) SHA1(eb968e40bcc7e7dd1acc0bbe885fd3f7d70d4bb5) )
	ROM_LOAD( "cv01.bin",     0x08000, 0x8000, CRC(f68c2ff6) SHA1(dda9159fb59d3855025b98c272722b031617c89a) )
	ROM_LOAD( "cv02.bin",     0x10000, 0x8000, CRC(979cdb5a) SHA1(25c1f3e6ddf50168c7e1a967bfa2753bea6106ec) )

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "cv03.bin",     0x00000, 0x8000, CRC(cc7ed6d8) SHA1(4ffcfa3f720414e1b7e929bdf29359ebcd8717c3) )
	/* we expand rom cv04.bin to 0x8000 - 0xffff */

	ROM_REGION( 0x8000,  "gfx3", 0 )
	ROM_LOAD( "cv07.bin",     0x00000, 0x8000, CRC(ed5441ba) SHA1(69d50695e8b92544f9857c6f3de0efb399899a2c) )

	ROM_REGION( 0x4000, "gfx4", 0 )
	ROM_LOAD( "cv04.bin",     0x00000, 0x4000, CRC(df693a04) SHA1(45bef98c7e66881f8c62affecc1ab90dd2707240) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "cv08-1.bpr",   0x00000, 0x0100, CRC(b7c43db9) SHA1(418b11e4c8a9bce6873b0624ac53a5011c5807d0) )
ROM_END


/*************************************
 *
 *  Driver initialization
 *
 *************************************/

static void compgolf_expand_bg(running_machine *machine)
{
	UINT8 *GFXDST = memory_region(machine, "gfx2");
	UINT8 *GFXSRC = memory_region(machine, "gfx4");

	int x;

	for (x = 0; x < 0x4000; x++)
	{
		GFXDST[0x8000 + x]  = (GFXSRC[x] & 0x0f) << 4;
		GFXDST[0xc000 + x]  = (GFXSRC[x] & 0xf0);
	}
}

static DRIVER_INIT( compgolf )
{
	memory_configure_bank(machine, "bank1", 0, 2, memory_region(machine, "user1"), 0x4000);
	compgolf_expand_bg(machine);
}


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1986, compgolf, 0,        compgolf, compgolf, compgolf, ROT0, "Data East", "Competition Golf Final Round (revision 3)", GAME_SUPPORTS_SAVE )
GAME( 1985, compgolfo,compgolf, compgolf, compgolf, compgolf, ROT0, "Data East", "Competition Golf Final Round (old version)", GAME_SUPPORTS_SAVE )
