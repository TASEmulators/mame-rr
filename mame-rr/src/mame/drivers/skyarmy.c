/*
 2010.04.05. stephh

    - Fixed Dip Switches and Inputs (after verification of the Z80 code)
    - Updated memory map to partially handle screen flipping

 05/01/2003  MooglyGuy/Ryan Holtz
    - Corrected second AY (shouldn't have been there)
    - Added first AY's status read
    - Added coinage DIP
    - What the hell are those unmapped port writes!? Not AY...

 2003.01.01. Tomasz Slanina

  changes :
    - nmi generation ( incorrect freq probably)
    - music/sfx (partially)
    - more sprite tiles (twice than before)
    - fixed sprites flips
    - scrolling (2nd game level)
    - better colors (weird 'hack' .. but works in most cases ( comparing with screens from emustatus ))
    - dips - lives
    - visible area .. a bit smaller (at least bg 'generation' is not visible for scrolling levels )
    - cpu clock .. now 4 mhz
*/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"

class skyarmy_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, skyarmy_state(machine)); }

	skyarmy_state(running_machine &machine) { }

	UINT8 *spriteram;
	UINT8 *videoram;
	UINT8 *colorram;
	UINT8 *scrollram;
	tilemap_t* tilemap;
	int nmi;
};

static WRITE8_HANDLER( skyarmy_flip_screen_x_w )
{
	flip_screen_x_set(space->machine, data & 0x01);
}

static WRITE8_HANDLER( skyarmy_flip_screen_y_w )
{
	flip_screen_y_set(space->machine, data & 0x01);
}

static TILE_GET_INFO( get_skyarmy_tile_info )
{
	skyarmy_state *state = (skyarmy_state *)machine->driver_data;
	int code = state->videoram[tile_index];
	int attr = BITSWAP8(state->colorram[tile_index], 7, 6, 5, 4, 3, 0, 1, 2) & 7;

	SET_TILE_INFO( 0, code, attr, 0);
}

static WRITE8_HANDLER( skyarmy_videoram_w )
{
	skyarmy_state *state = (skyarmy_state *)space->machine->driver_data;

	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->tilemap,offset);
}

static WRITE8_HANDLER( skyarmy_colorram_w )
{
	skyarmy_state *state = (skyarmy_state *)space->machine->driver_data;

	state->colorram[offset] = data;
	tilemap_mark_tile_dirty(state->tilemap,offset);
}

static PALETTE_INIT( skyarmy )
{
	int i;

	for (i = 0;i < 32;i++)
	{
		int bit0,bit1,bit2,r,g,b;

		bit0 = (*color_prom >> 0) & 0x01;
		bit1 = (*color_prom >> 1) & 0x01;
		bit2 = (*color_prom >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit0 = (*color_prom >> 3) & 0x01;
		bit1 = (*color_prom >> 4) & 0x01;
		bit2 = (*color_prom >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit0=0;
		bit1 = (*color_prom >> 6) & 0x01;
		bit2 = (*color_prom >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
		color_prom++;
	}
}

static VIDEO_START( skyarmy )
{
	skyarmy_state *state = (skyarmy_state *)machine->driver_data;

	state->tilemap = tilemap_create(machine, get_skyarmy_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	tilemap_set_scroll_cols(state->tilemap,32);
}


static VIDEO_UPDATE( skyarmy )
{
	skyarmy_state *state = (skyarmy_state *)screen->machine->driver_data;
	UINT8 *spriteram = state->spriteram;
	int sx, sy, flipx, flipy, offs,pal;
	int i;

	for(i=0;i<0x20;i++)
		tilemap_set_scrolly( state->tilemap,i,state->scrollram[i]);

	tilemap_draw(bitmap,cliprect,state->tilemap,0,0);

	for (offs = 0 ; offs < 0x40; offs+=4)
	{
		pal = BITSWAP8(spriteram[offs+2], 7, 6, 5, 4, 3, 0, 1, 2) & 7;

		sx = spriteram[offs+3];
		sy = 240-(spriteram[offs]+1);
		flipy = (spriteram[offs+1]&0x80)>>7;
		flipx = (spriteram[offs+1]&0x40)>>6;

		drawgfx_transpen(bitmap,cliprect,screen->machine->gfx[1],
			spriteram[offs+1]&0x3f,
			pal,
			flipx,flipy,
			sx,sy,0);
	}

	return 0;
}

static INTERRUPT_GEN( skyarmy_nmi_source )
{
	skyarmy_state *state = (skyarmy_state *)device->machine->driver_data;

	if(state->nmi) cpu_set_input_line(device,INPUT_LINE_NMI, PULSE_LINE);
}


static WRITE8_HANDLER( nmi_enable_w )
{
	skyarmy_state *state = (skyarmy_state *)space->machine->driver_data;

	state->nmi=data & 1;
}


static ADDRESS_MAP_START( skyarmy_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x8800, 0x8fff) AM_RAM_WRITE(skyarmy_videoram_w) AM_BASE_MEMBER(skyarmy_state,videoram) /* Video RAM */
	AM_RANGE(0x9000, 0x93ff) AM_RAM_WRITE(skyarmy_colorram_w) AM_BASE_MEMBER(skyarmy_state,colorram) /* Color RAM */
	AM_RANGE(0x9800, 0x983f) AM_RAM AM_BASE_MEMBER(skyarmy_state,spriteram) /* Sprites */
	AM_RANGE(0x9840, 0x985f) AM_RAM AM_BASE_MEMBER(skyarmy_state,scrollram)  /* Scroll RAM */
	AM_RANGE(0xa000, 0xa000) AM_READ_PORT("DSW")
	AM_RANGE(0xa001, 0xa001) AM_READ_PORT("P1")
	AM_RANGE(0xa002, 0xa002) AM_READ_PORT("P2")
	AM_RANGE(0xa003, 0xa003) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xa004, 0xa004) AM_WRITE(nmi_enable_w) // ???
	AM_RANGE(0xa005, 0xa005) AM_WRITE(skyarmy_flip_screen_x_w)
	AM_RANGE(0xa006, 0xa006) AM_WRITE(skyarmy_flip_screen_y_w)
	AM_RANGE(0xa007, 0xa007) AM_WRITENOP
ADDRESS_MAP_END

static ADDRESS_MAP_START( skyarmy_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x04, 0x05) AM_DEVWRITE("aysnd", ay8910_address_data_w)
	AM_RANGE(0x06, 0x06) AM_DEVREAD("aysnd", ay8910_r)
ADDRESS_MAP_END


/* verified from Z80 code */
static INPUT_PORTS_START( skyarmy )
	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x03, DEF_STR ( Infinite ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x04, "20000" )
	PORT_DIPSETTING(    0x08, "30000" )
	PORT_DIPSETTING(    0x0c, "40000" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	256,
	2,
	{ 0, 256*8*8 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout spritelayout =
{
	16,16,
	32*2,
	2,
	{ 0, 256*8*8 },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
	  8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
	 16*8,17*8,18*8,19*8,20*8,21*8,22*8,23*8 },
	32*8
};

static GFXDECODE_START( skyarmy )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0, 8 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 0, 8 )
GFXDECODE_END

static MACHINE_DRIVER_START( skyarmy )

	MDRV_DRIVER_DATA( skyarmy_state )

	MDRV_CPU_ADD("maincpu", Z80,4000000)
	MDRV_CPU_PROGRAM_MAP(skyarmy_map)
	MDRV_CPU_IO_MAP(skyarmy_io_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)
	MDRV_CPU_PERIODIC_INT(skyarmy_nmi_source,650)	/* Hz */

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8,32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8,32*8-1,1*8,31*8-1)

	MDRV_GFXDECODE(skyarmy)
	MDRV_PALETTE_LENGTH(32)

	MDRV_PALETTE_INIT(skyarmy)
	MDRV_VIDEO_START(skyarmy)
	MDRV_VIDEO_UPDATE(skyarmy)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("aysnd", AY8910, 2500000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)
MACHINE_DRIVER_END


ROM_START( skyarmy )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a1h.bin", 0x0000, 0x2000, CRC(e3fb9d70) SHA1(b8e3a6d7d6ef30c1397f9b741132c5257c16be2d) )
	ROM_LOAD( "a2h.bin", 0x2000, 0x2000, CRC(0417653e) SHA1(4f6ad7335b5b7e85b4e16cce3c127488c02401b2) )
	ROM_LOAD( "a3h.bin", 0x4000, 0x2000, CRC(95485e56) SHA1(c4cbcd31ba68769d2d0d0875e2a92982265339ae) )
	ROM_LOAD( "j4.bin",  0x6000, 0x2000, CRC(843783df) SHA1(256d8375a8af7de080d456dbc6290a22473d011b) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "13b.bin", 0x0000, 0x0800, CRC(3b0e0f7c) SHA1(2bbba10121d3e745146f50c14dc6df97de40fb96) )
	ROM_LOAD( "15b.bin", 0x0800, 0x0800, CRC(5ccfd782) SHA1(408406ae068e5578b8a742abed1c37dcd3720fe5) )

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "8b.bin",  0x0000, 0x0800, CRC(6ac6bd98) SHA1(e653d80ec1b0f8e07821ea781942dae3de7d238d) )
	ROM_LOAD( "10b.bin", 0x0800, 0x0800, CRC(cada7682) SHA1(83ce8336274cb8006a445ac17a179d9ffd4d6809) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "a6.bin",  0x0000, 0x0020, CRC(c721220b) SHA1(61b3320fb616c0600d56840cb6438616c7e0c6eb) )
ROM_END

GAME( 1982, skyarmy, 0, skyarmy, skyarmy, 0, ROT90, "Shoei", "Sky Army", GAME_NO_COCKTAIL )
