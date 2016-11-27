/*

Note: this hardware is a copy of Psikyo's 68020 based hardware,
      the Strikers 1945 bootleg has the same unknown rom!

Dream World
SemiCom, 2000

PCB Layout
----------

|-------------------------------------------------|
|    M6295  ROM5    62256   ACTEL           ROM10 |
|VOL M6295  ROM6    62256   A40MX04               |
|    PAL  PAL       32MHz                         |
| 62256  62256              PAL                   |
| ROM1 ROM3       68EC020   PAL    PAL            |
| ROM2 ROM4                 PAL    PAL            |
|J 62256 62256              PAL                   |
|A                          PAL    27MHz          |
|M                                 PAL            |
|M                         ACTEL    M5M44260      |
|A             6116        A40MX04  M5M44260      |
|              6116                               |
|                          PAL                    |
|              6264        PAL                    |
|              6264                               |
| DSW1                      ROM11                 |
|        8752        ROM7   ROM9                  |
| DSW2               ROM8                         |
|-------------------------------------------------|
Notes:
      68020 @ 16.0MHz [32/2]
      M6295 (both) @ 1.0MHz [32/32]. pin 7 LOW
      8752 @ 16.0MHz [32/2]
      HSync @ 15.2kHz
      VSync @ 58Hz


Stephh's notes (based on the game M68EC020 code and some tests) :

  - Don't trust the "test mode" as it displays Dip Switches infos
    that are in fact unused by the game ! Leftover from another game ?

    PORT_START("DSW")
    PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
    PORT_DIPSETTING(      0x0002, "1" )
    PORT_DIPSETTING(      0x0003, "2" )
    PORT_DIPSETTING(      0x0001, "3" )
    PORT_DIPSETTING(      0x0000, "4" )
    PORT_DIPUNUSED_DIPLOC( 0x0004, IP_ACTIVE_LOW, "SW2:3" )
    PORT_DIPUNUSED_DIPLOC( 0x0008, IP_ACTIVE_LOW, "SW2:4" )
    PORT_DIPUNUSED_DIPLOC( 0x0010, IP_ACTIVE_LOW, "SW2:5" )
    PORT_DIPNAME( 0x0060, 0x0060, "Ticket Payout" )         PORT_DIPLOCATION("SW2:6,7")
    PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
    PORT_DIPSETTING(      0x0020, "Little" )
    PORT_DIPSETTING(      0x0060, DEF_STR( Normal ) )
    PORT_DIPSETTING(      0x0040, "Much" )
    PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Free_Play ) )    PORT_DIPLOCATION("SW2:8")
    PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:1")
    PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0e00, 0x0e00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:2,3,4")
    PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
    PORT_DIPSETTING(      0x0200, DEF_STR( 4C_1C ) )
    PORT_DIPSETTING(      0x0400, DEF_STR( 3C_1C ) )
    PORT_DIPSETTING(      0x0600, DEF_STR( 2C_1C ) )
    PORT_DIPSETTING(      0x0e00, DEF_STR( 1C_1C ) )
    PORT_DIPSETTING(      0x0a00, DEF_STR( 2C_3C ) )
    PORT_DIPSETTING(      0x0c00, DEF_STR( 1C_2C ) )
    PORT_DIPSETTING(      0x0800, DEF_STR( 1C_3C ) )
    PORT_DIPNAME( 0x7000, 0x7000, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:5,6,7")
    PORT_DIPSETTING(      0x2000, "Level 1" )
    PORT_DIPSETTING(      0x1000, "Level 2" )
    PORT_DIPSETTING(      0x0000, "Level 3" )
    PORT_DIPSETTING(      0x7000, "Level 4" )
    PORT_DIPSETTING(      0x6000, "Level 5" )
    PORT_DIPSETTING(      0x5000, "Level 6" )
    PORT_DIPSETTING(      0x4000, "Level 7" )
    PORT_DIPSETTING(      0x3000, "Level 8" )
    PORT_SERVICE_DIPLOC( 0x8000, IP_ACTIVE_LOW, "SW1:8" )

*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"

#define MASTER_CLOCK 32000000

class dreamwld_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, dreamwld_state(machine)); }

	dreamwld_state(running_machine &machine) { }

	/* memory pointers */
	UINT32 *  bg_videoram;
	UINT32 *  bg2_videoram;
	UINT32 *  bg_scroll;
	UINT32 *  paletteram;
	UINT32 *  spriteram;

	/* video-related */
	tilemap_t  *bg_tilemap,*bg2_tilemap;
	int      tilebank[2], tilebankold[2];

	/* misc */
	int      protindex;
};



static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	dreamwld_state *state = (dreamwld_state *)machine->driver_data;
	const gfx_element *gfx = machine->gfx[0];
	UINT32 *source = state->spriteram;
	UINT32 *finish = state->spriteram + 0x1000 / 4;
	UINT16 *redirect = (UINT16 *)memory_region(machine, "gfx3");

	while (source < finish)
	{
		int xpos, ypos, tileno;
		int xsize, ysize, xinc;
		int xct, yct;
		int xflip;
		int colour;

		xpos  = (source[0] & 0x000001ff) >> 0;
		ypos  = (source[0] & 0x01ff0000) >> 16;
		xsize = (source[0] & 0x00000e00) >> 9;
		ysize = (source[0] & 0x0e000000) >> 25;

		tileno = (source[1] & 0x0000ffff) >>0;
		colour = (source[1] & 0x3f000000) >>24;
		xflip  = (source[1] & 0x40000000);

		xinc = 16;

		if (xflip)
		{
			xinc = -16;
			xpos += 16 * xsize;
		}

		ysize++; xsize++; // size 0 = 1 tile

		xpos -=16;


		for (yct = 0; yct < ysize; yct++)
		{
			for (xct = 0; xct < xsize; xct++)
			{
				drawgfx_transpen(bitmap, cliprect, gfx, redirect[tileno], colour, xflip, 0, xpos + xct * xinc, ypos + yct * 16, 0);
				drawgfx_transpen(bitmap, cliprect, gfx, redirect[tileno], colour, xflip, 0, (xpos + xct * xinc) - 0x200, ypos + yct * 16, 0);
				drawgfx_transpen(bitmap, cliprect, gfx, redirect[tileno], colour, xflip, 0, (xpos + xct * xinc) - 0x200, (ypos + yct * 16) - 0x200, 0);
				drawgfx_transpen(bitmap, cliprect, gfx, redirect[tileno], colour, xflip, 0, xpos + xct * xinc, (ypos + yct * 16) - 0x200 , 0);

				tileno++;
			}
		}

		source += 2;
	}
}


static WRITE32_HANDLER( dreamwld_bg_videoram_w )
{
	dreamwld_state *state = (dreamwld_state *)space->machine->driver_data;
	COMBINE_DATA(&state->bg_videoram[offset]);
	tilemap_mark_tile_dirty(state->bg_tilemap, offset * 2);
	tilemap_mark_tile_dirty(state->bg_tilemap, offset * 2 + 1);
}

static TILE_GET_INFO( get_dreamwld_bg_tile_info )
{
	dreamwld_state *state = (dreamwld_state *)machine->driver_data;
	int tileno, colour;
	tileno = (tile_index & 1) ? (state->bg_videoram[tile_index >> 1] & 0xffff) : ((state->bg_videoram[tile_index >> 1] >> 16) & 0xffff);
	colour = tileno >> 13;
	tileno &= 0x1fff;
	SET_TILE_INFO(1, tileno + state->tilebank[0] * 0x2000, 0x80 + colour, 0);
}


static WRITE32_HANDLER( dreamwld_bg2_videoram_w )
{
	dreamwld_state *state = (dreamwld_state *)space->machine->driver_data;
	COMBINE_DATA(&state->bg2_videoram[offset]);
	tilemap_mark_tile_dirty(state->bg2_tilemap, offset * 2);
	tilemap_mark_tile_dirty(state->bg2_tilemap, offset * 2 + 1);
}

static TILE_GET_INFO( get_dreamwld_bg2_tile_info )
{
	dreamwld_state *state = (dreamwld_state *)machine->driver_data;
	UINT16 tileno, colour;
	tileno = (tile_index & 1) ? (state->bg2_videoram[tile_index >> 1] & 0xffff) : ((state->bg2_videoram[tile_index >> 1] >> 16) & 0xffff);
	colour = tileno >> 13;
	tileno &= 0x1fff;
	SET_TILE_INFO(1, tileno + state->tilebank[1] * 0x2000, 0xc0 + colour, 0);
}

static VIDEO_START( dreamwld )
{
	dreamwld_state *state = (dreamwld_state *)machine->driver_data;

	state->bg_tilemap = tilemap_create(machine, get_dreamwld_bg_tile_info,tilemap_scan_rows, 16, 16, 64,32);
	state->bg2_tilemap = tilemap_create(machine, get_dreamwld_bg2_tile_info,tilemap_scan_rows, 16, 16, 64,32);
	tilemap_set_transparent_pen(state->bg2_tilemap,0);
}

static VIDEO_UPDATE( dreamwld )
{
	dreamwld_state *state = (dreamwld_state *)screen->machine->driver_data;

	tilemap_set_scrolly(state->bg_tilemap, 0, state->bg_scroll[(0x400 / 4)] + 32);
	tilemap_set_scrolly(state->bg2_tilemap, 0, state->bg_scroll[(0x400 / 4) + 2] + 32);
	tilemap_set_scrollx(state->bg_tilemap, 0, state->bg_scroll[(0x400 / 4) + 1] + 3);
	tilemap_set_scrollx(state->bg2_tilemap, 0, state->bg_scroll[(0x400 / 4) + 3] + 5);

	state->tilebank[0] = (state->bg_scroll[(0x400 / 4) + 4] >> 6) & 1;
	state->tilebank[1] = (state->bg_scroll[(0x400 / 4) + 5] >> 6) & 1;

	if (state->tilebank[0] != state->tilebankold[0])
	{
		state->tilebankold[0] = state->tilebank[0];
		tilemap_mark_all_tiles_dirty(state->bg_tilemap);
	}

	if (state->tilebank[1] != state->tilebankold[1])
	{
		state->tilebankold[1] = state->tilebank[1];
		tilemap_mark_all_tiles_dirty(state->bg2_tilemap);
	}

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, state->bg2_tilemap, 0, 0);

	draw_sprites(screen->machine, bitmap, cliprect);

	return 0;
}


static READ32_HANDLER( dreamwld_protdata_r )
{
	dreamwld_state *state = (dreamwld_state *)space->machine->driver_data;

	UINT8 *protdata = memory_region(space->machine, "user1");
	size_t protsize = memory_region_length(space->machine, "user1");
	UINT8 dat = protdata[(state->protindex++) % protsize];
	return dat << 24;
}


static WRITE32_HANDLER( dreamwld_palette_w )
{
	dreamwld_state *state = (dreamwld_state *)space->machine->driver_data;
	UINT16 dat;
	int color;

	COMBINE_DATA(&state->paletteram[offset]);
	color = offset * 2;

	dat = state->paletteram[offset] & 0x7fff;
	palette_set_color_rgb(space->machine, color + 1, pal5bit(dat >> 10), pal5bit(dat >> 5), pal5bit(dat >> 0));

	dat = (state->paletteram[offset] >> 16) & 0x7fff;
	palette_set_color_rgb(space->machine, color, pal5bit(dat >> 10), pal5bit(dat >> 5), pal5bit(dat >> 0));
}

static void dreamwld_oki_setbank( running_machine *machine, UINT8 chip, UINT8 bank )
{
	/* 0x30000-0x3ffff is banked.
        banks are at 0x30000,0x40000,0x50000 and 0x60000 in rom */
	UINT8 *sound = memory_region(machine, chip ? "oki1" : "oki2");
	logerror("OKI%d: set bank %02x\n", chip, bank);
	memcpy(sound + 0x30000, sound + 0xb0000 + 0x10000 * bank, 0x10000);
}


static WRITE32_HANDLER( dreamwld_6295_0_bank_w )
{
	if (ACCESSING_BITS_0_7)
		dreamwld_oki_setbank(space->machine, 0, data & 0x3);
	else
		logerror("OKI0: unk bank write %x mem_mask %8x\n", data, mem_mask);
}

static WRITE32_HANDLER( dreamwld_6295_1_bank_w )
{
	if (ACCESSING_BITS_0_7)
		dreamwld_oki_setbank(space->machine, 1, data & 0x3);
	else
		logerror("OKI1: unk bank write %x mem_mask %8x\n", data, mem_mask);
}

static ADDRESS_MAP_START( dreamwld_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM  AM_WRITENOP

	AM_RANGE(0x400000, 0x401fff) AM_RAM AM_BASE_MEMBER(dreamwld_state, spriteram)
	AM_RANGE(0x600000, 0x601fff) AM_RAM_WRITE(dreamwld_palette_w) AM_BASE_MEMBER(dreamwld_state, paletteram)  // real palette?
	AM_RANGE(0x800000, 0x801fff) AM_RAM_WRITE(dreamwld_bg_videoram_w ) AM_BASE_MEMBER(dreamwld_state, bg_videoram)
	AM_RANGE(0x802000, 0x803fff) AM_RAM_WRITE(dreamwld_bg2_videoram_w ) AM_BASE_MEMBER(dreamwld_state, bg2_videoram)
	AM_RANGE(0x804000, 0x805fff) AM_RAM AM_BASE_MEMBER(dreamwld_state, bg_scroll)  // scroll regs etc.

	AM_RANGE(0xc00000, 0xc00003) AM_READ_PORT("INPUTS")
	AM_RANGE(0xc00004, 0xc00007) AM_READ_PORT("c00004")

	AM_RANGE(0xc0000c, 0xc0000f) AM_WRITE(dreamwld_6295_0_bank_w) // sfx
	AM_RANGE(0xc00018, 0xc0001b) AM_DEVREADWRITE8("oki1", okim6295_r, okim6295_w, 0xff000000) // sfx

	AM_RANGE(0xc0002c, 0xc0002f) AM_WRITE(dreamwld_6295_1_bank_w) // sfx
	AM_RANGE(0xc00028, 0xc0002b) AM_DEVREADWRITE8("oki2", okim6295_r, okim6295_w, 0xff000000) // sfx

	AM_RANGE(0xc00030, 0xc00033) AM_READ(dreamwld_protdata_r) // it reads protection data (irq code) from here and puts it at ffd000

	AM_RANGE(0xfe0000, 0xffffff) AM_RAM // work ram
ADDRESS_MAP_END


static INPUT_PORTS_START( dreamwld )
	PORT_START("INPUTS")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0000fffc, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) /* "Book" (when you get one of them) */
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) /* "Jump" */
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) /* "Dig" */
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(2)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(2)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(2)
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) /* "Book" (when you get one of them) */
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) /* "Jump" */
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) /* "Dig" */
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1)
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1)
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(1)

	PORT_START("c00004")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(custom_port_read, "DSW")
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(custom_port_read, "DSW")

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0001, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPUNUSED_DIPLOC( 0x0004, IP_ACTIVE_LOW, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x0008, IP_ACTIVE_LOW, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x0010, IP_ACTIVE_LOW, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x0020, IP_ACTIVE_LOW, "SW2:6" ) /* see notes - "Ticket Payout" */
	PORT_DIPUNUSED_DIPLOC( 0x0040, IP_ACTIVE_LOW, "SW2:7" ) /* see notes - "Ticket Payout" */
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Free_Play ) )    PORT_DIPLOCATION("SW2:8")     /* gives in fact 99 credits */
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0100, IP_ACTIVE_LOW, "SW1:1" ) /* see notes - "Demo Sounds" */
	PORT_DIPNAME( 0x0e00, 0x0e00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:2,3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0e00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0a00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_3C ) )
	PORT_DIPUNUSED_DIPLOC( 0x1000, IP_ACTIVE_LOW, "SW1:5" ) /* see notes - "Difficulty" */
	PORT_DIPUNUSED_DIPLOC( 0x2000, IP_ACTIVE_LOW, "SW1:6" ) /* see notes - "Difficulty" */
	PORT_DIPUNUSED_DIPLOC( 0x4000, IP_ACTIVE_LOW, "SW1:7" ) /* see notes - "Difficulty" */
	PORT_SERVICE_DIPLOC( 0x8000, IP_ACTIVE_LOW, "SW1:8" )
INPUT_PORTS_END


static const gfx_layout tiles16x16_layout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,8*64,9*64,10*64,11*64,12*64,13*64,14*64,15*64 },
	16*64
};


static GFXDECODE_START( dreamwld )
	GFXDECODE_ENTRY( "gfx1", 0, tiles16x16_layout, 0, 0x100 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles16x16_layout, 0, 0x100 )
GFXDECODE_END


static MACHINE_START( dreamwld )
{
	dreamwld_state *state = (dreamwld_state *)machine->driver_data;

	state_save_register_global(machine, state->protindex);
	state_save_register_global_array(machine, state->tilebank);
	state_save_register_global_array(machine, state->tilebankold);
}

static MACHINE_RESET( dreamwld )
{
	dreamwld_state *state = (dreamwld_state *)machine->driver_data;

	state->tilebankold[0] = state->tilebankold[1] = -1;
	state->tilebank[0] = state->tilebank[1] = 0;
	state->protindex = 0;
}

static MACHINE_DRIVER_START( dreamwld )

	/* driver data */
	MDRV_DRIVER_DATA(dreamwld_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68EC020, MASTER_CLOCK/2)
	MDRV_CPU_PROGRAM_MAP(dreamwld_map)
	MDRV_CPU_VBLANK_INT("screen", irq4_line_hold) // 4, 5, or 6, all point to the same place

	MDRV_MACHINE_START(dreamwld)
	MDRV_MACHINE_RESET(dreamwld)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(58)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(512,256)
	MDRV_SCREEN_VISIBLE_AREA(0, 304-1, 0, 224-1)

	MDRV_PALETTE_LENGTH(0x1000)
	MDRV_GFXDECODE(dreamwld)

	MDRV_VIDEO_START(dreamwld)
	MDRV_VIDEO_UPDATE(dreamwld)

	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_OKIM6295_ADD("oki1", MASTER_CLOCK/32, OKIM6295_PIN7_LOW)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)

	MDRV_OKIM6295_ADD("oki2", MASTER_CLOCK/32, OKIM6295_PIN7_LOW)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)
MACHINE_DRIVER_END



ROM_START( dreamwld )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD32_BYTE( "1.bin", 0x000002, 0x040000, CRC(35c94ee5) SHA1(3440a65a807622b619c97bc2a88fd7d875c26f66) )
	ROM_LOAD32_BYTE( "2.bin", 0x000003, 0x040000, CRC(5409e7fc) SHA1(2f94a6a8e4c94b36b43f0b94d58525f594339a9d) )
	ROM_LOAD32_BYTE( "3.bin", 0x000000, 0x040000, CRC(e8f7ae78) SHA1(cfd393cec6dec967c82e1131547b7e7fdc5d814f) )
	ROM_LOAD32_BYTE( "4.bin", 0x000001, 0x040000, CRC(3ef5d51b) SHA1(82a00b4ff7155f6d5553870dfd510fed9469d9b5) )

	ROM_REGION( 0x10000, "cpu1", 0 ) /* 87C52 MCU Code */
	ROM_LOAD( "87c52.mcu", 0x00000, 0x10000 , NO_DUMP ) /* can't be dumped. */

	ROM_REGION( 0x6c9, "user1", 0 ) /* Protection data  */
	/* The MCU supplies this data.
      The 68k reads it through a port, taking the size and destination write address from the level 1
      and level 2 irq positions in the 68k vector table (there is code to check that they haven't been
      modofied!)  It then decodes the data using the rom checksum previously calculated and puts it in
      ram.  The interrupt vectors point at the code placed in RAM. */
	ROM_LOAD( "protdata.bin", 0x000, 0x6c9 ,  CRC(f284b2fd) SHA1(9e8096c8aa8a288683f002311b38787b120748d1) ) /* extracted */

	ROM_REGION( 0x100000, "oki1", 0 ) /* OKI Samples - 1st chip*/
	ROM_LOAD( "5.bin", 0x000000, 0x80000, CRC(9689570a) SHA1(4414233da8f46214ca7e9022df70953922a63aa4) )
	ROM_RELOAD(0x80000,0x80000) // fot the banks

	ROM_REGION( 0x100000, "oki2", 0 ) /* OKI Samples - 2nd chip*/
	ROM_LOAD( "6.bin", 0x000000, 0x80000, CRC(c8b91f30) SHA1(706004ca56d0a74bc7a3dfd73a21cdc09eb90f05) )
	ROM_RELOAD(0x80000,0x80000) // fot the banks

	ROM_REGION( 0x400000, "gfx1", 0 ) /* Sprite Tiles - decoded */
	ROM_LOAD( "9.bin", 0x000000, 0x200000, CRC(fa84e3af) SHA1(5978737d348fd382f4ec004d29870656c864d137) )

	ROM_REGION( 0x200000, "gfx2", 0 ) /* BG Tiles - decoded */
	ROM_LOAD( "10.bin",0x000000, 0x200000, CRC(3553e4f5) SHA1(c335494f4a12a01a88e7cd578cae922954303cfd) )

	ROM_REGION( 0x040000, "gfx3", 0 ) /* Sprite Code Lookup ... */
	ROM_LOAD16_BYTE( "8.bin", 0x000000, 0x020000, CRC(8d570df6) SHA1(e53e4b099c64eca11d027e0083caa101fcd99959) )
	ROM_LOAD16_BYTE( "7.bin", 0x000001, 0x020000, CRC(a68bf35f) SHA1(f48540a5415a7d9723ca6e7e03cab039751dce17) )

	ROM_REGION( 0x10000, "gfx4", 0 ) /* ???? - not decoded seems to be in blocks of 0x41 bytes.. */
	ROM_LOAD( "11.bin", 0x000000, 0x10000, CRC(0da8db45) SHA1(7d5bd71c5b0b28ff74c732edd7c662f46f2ab25b) )
ROM_END


GAME( 2000, dreamwld, 0, dreamwld, dreamwld, 0, ROT0,  "SemiCom", "Dream World", GAME_SUPPORTS_SAVE )
