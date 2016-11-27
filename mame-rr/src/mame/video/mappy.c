#include "emu.h"
#include "video/resnet.h"
#include "includes/mappy.h"


/***************************************************************************

  Convert the color PROMs.

  All games except Phozon have one 32x8 palette PROM and two 256x4 color
  lookup table PROMs (one for characters, one for sprites), except todruaga
  which has a larger 1024x4 PROM for sprites.
  The palette PROM is connected to the RGB output this way:

  bit 7 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

  The way how the lookup tables are mapped to palette colors, and priority
  handling, are controlled by a PAL (SPV-5 in Super Pacman, MPI-4 in Mappy),
  so the two hardwares work differently.
  Super Pacman has a special "super priority" for sprite colors, allowing
  one pen to be over high priority tiles (used by Pac & Pal for ghost eyes),
  which isn't present in Mappy.

***************************************************************************/

PALETTE_INIT( superpac )
{
	static const int resistances[3] = { 1000, 470, 220 };
	double rweights[3], gweights[3], bweights[2];
	int i;

	/* compute the color output resistor weights */
	compute_resistor_weights(0,	255, -1.0,
			3, &resistances[0], rweights, 0, 0,
			3, &resistances[0], gweights, 0, 0,
			2, &resistances[1], bweights, 0, 0);

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 32);

	/* create a lookup table for the palette */
	for (i = 0; i < 32; i++)
	{
		int bit0, bit1, bit2;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = combine_3_weights(rweights, bit0, bit1, bit2);

		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = combine_3_weights(gweights, bit0, bit1, bit2);

		/* blue component */
		bit0 = (color_prom[i] >> 6) & 0x01;
		bit1 = (color_prom[i] >> 7) & 0x01;
		b = combine_2_weights(bweights, bit0, bit1);

		colortable_palette_set_color(machine->colortable, i, MAKE_RGB(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 32;

	/* characters map to the upper 16 palette entries */
	for (i = 0; i < 64*4; i++)
	{
		UINT8 ctabentry = color_prom[i] & 0x0f;
		colortable_entry_set_value(machine->colortable, i, (ctabentry ^ 15) + 0x10);
	}

	/* sprites map to the lower 16 palette entries */
	for (i = 64*4; i < 128*4; i++)
	{
		UINT8 ctabentry = color_prom[i] & 0x0f;
		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}
}

PALETTE_INIT( mappy )
{
	static const int resistances[3] = { 1000, 470, 220 };
	double rweights[3], gweights[3], bweights[2];
	int i;

	/* compute the color output resistor weights */
	compute_resistor_weights(0,	255, -1.0,
			3, &resistances[0], rweights, 0, 0,
			3, &resistances[0], gweights, 0, 0,
			2, &resistances[1], bweights, 0, 0);

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 32);

	/* create a lookup table for the palette */
	for (i = 0; i < 32; i++)
	{
		int bit0, bit1, bit2;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = combine_3_weights(rweights, bit0, bit1, bit2);

		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = combine_3_weights(gweights, bit0, bit1, bit2);

		/* blue component */
		bit0 = (color_prom[i] >> 6) & 0x01;
		bit1 = (color_prom[i] >> 7) & 0x01;
		b = combine_2_weights(bweights, bit0, bit1);

		colortable_palette_set_color(machine->colortable, i, MAKE_RGB(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 32;

	/* characters map to the upper 16 palette entries */
	for (i = 0*4; i < 64*4; i++)
	{
		UINT8 ctabentry = color_prom[i] & 0x0f;
		colortable_entry_set_value(machine->colortable, i, ctabentry + 0x10);
	}

	/* sprites map to the lower 16 palette entries */
	for (i = 64*4; i < machine->total_colors(); i++)
	{
		UINT8 ctabentry = color_prom[i] & 0x0f;
		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}
}


/***************************************************************************

  In Phozon, the palette PROMs are connected to the RGB output this way:

  bit 3 -- 220 ohm resistor  -- RED/GREEN/BLUE
        -- 470 ohm resistor  -- RED/GREEN/BLUE
        -- 1  kohm resistor  -- RED/GREEN/BLUE
  bit 0 -- 2.2kohm resistor  -- RED/GREEN/BLUE

***************************************************************************/

PALETTE_INIT( phozon )
{
	static const int resistances[4] = { 2200, 1000, 470, 220 };
	double rweights[4], gweights[4], bweights[4];
	int i;

	/* compute the color output resistor weights */
	compute_resistor_weights(0,	255, -1.0,
			4, &resistances[0], rweights, 0, 0,
			4, &resistances[0], gweights, 0, 0,
			4, &resistances[0], bweights, 0, 0);

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 32);

	/* create a lookup table for the palette */
	for (i = 0; i < 32; i++)
	{
		int bit0, bit1, bit2, bit3;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		bit3 = (color_prom[i] >> 3) & 0x01;
		r = combine_4_weights(rweights, bit0, bit1, bit2, bit3);

		/* green component */
		bit0 = (color_prom[i + 0x100] >> 0) & 0x01;
		bit1 = (color_prom[i + 0x100] >> 1) & 0x01;
		bit2 = (color_prom[i + 0x100] >> 2) & 0x01;
		bit3 = (color_prom[i + 0x100] >> 3) & 0x01;
		g = combine_4_weights(gweights, bit0, bit1, bit2, bit3);

		/* blue component */
		bit0 = (color_prom[i + 0x200] >> 0) & 0x01;
		bit1 = (color_prom[i + 0x200] >> 1) & 0x01;
		bit2 = (color_prom[i + 0x200] >> 2) & 0x01;
		bit3 = (color_prom[i + 0x200] >> 3) & 0x01;
		b = combine_4_weights(bweights, bit0, bit1, bit2, bit3);

		colortable_palette_set_color(machine->colortable, i, MAKE_RGB(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x300;

	/* characters map to the lower 16 palette entries */
	for (i = 0; i < 64*4; i++)
	{
		UINT8 ctabentry = color_prom[i] & 0x0f;
		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}

	/* sprites map to the upper 16 palette entries */
	for (i = 64*4; i < 128*4; i++)
	{
		UINT8 ctabentry = color_prom[i] & 0x0f;
		colortable_entry_set_value(machine->colortable, i, ctabentry + 0x10);
	}
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

/* convert from 32x32 to 36x28 */
static TILEMAP_MAPPER( superpac_tilemap_scan )
{
	int offs;

	row += 2;
	col -= 2;
	if (col & 0x20)
		offs = row + ((col & 0x1f) << 5);
	else
		offs = col + (row << 5);

	return offs;
}

/* tilemap is a composition of a 32x60 scrolling portion and two 2x28 fixed portions on the sides */
static TILEMAP_MAPPER( mappy_tilemap_scan )
{
	int offs;

	col -= 2;
	if (col & 0x20)
	{
		/* in the following code, note the +2 followed by & 0x0f. This causes unintuitive
           mapping from logical to hardware coordinates, which is true to the hardware.
           Not doing it that way would cause missing tiles in motos and todruaga */
		if (row & 0x20)
			offs = 0x7ff;	// outside visible area
		else
			offs = ((row + 2) & 0x0f) + (row & 0x10) + ((col & 3) << 5) + 0x780;
	}
	else
		offs = col + (row << 5);

	return offs;
}

static TILE_GET_INFO( superpac_get_tile_info )
{
	mappy_state *state = (mappy_state *)machine->driver_data;
	UINT8 attr = state->videoram[tile_index + 0x400];

	tileinfo->category = (attr & 0x40) >> 6;
	tileinfo->group = attr & 0x3f;
	SET_TILE_INFO(
			0,
			state->videoram[tile_index],
			attr & 0x3f,
			0);
}

static TILE_GET_INFO( phozon_get_tile_info )
{
	mappy_state *state = (mappy_state *)machine->driver_data;
	UINT8 attr = state->videoram[tile_index + 0x400];

	tileinfo->category = (attr & 0x40) >> 6;
	tileinfo->group = attr & 0x3f;
	SET_TILE_INFO(
			0,
			state->videoram[tile_index] + ((attr & 0x80) << 1),
			attr & 0x3f,
			0);
}

static TILE_GET_INFO( mappy_get_tile_info )
{
	mappy_state *state = (mappy_state *)machine->driver_data;
	UINT8 attr = state->videoram[tile_index + 0x800];

	tileinfo->category = (attr & 0x40) >> 6;
	tileinfo->group = attr & 0x3f;
	SET_TILE_INFO(
			0,
			state->videoram[tile_index],
			attr & 0x3f,
			0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( superpac )
{
	mappy_state *state = (mappy_state *)machine->driver_data;

	state->bg_tilemap = tilemap_create(machine, superpac_get_tile_info,superpac_tilemap_scan,8,8,36,28);
	state->sprite_bitmap = machine->primary_screen->alloc_compatible_bitmap();

	colortable_configure_tilemap_groups(machine->colortable, state->bg_tilemap, machine->gfx[0], 31);
}

VIDEO_START( phozon )
{
	mappy_state *state = (mappy_state *)machine->driver_data;

	state->bg_tilemap = tilemap_create(machine, phozon_get_tile_info,superpac_tilemap_scan,8,8,36,28);

	colortable_configure_tilemap_groups(machine->colortable, state->bg_tilemap, machine->gfx[0], 15);

	state_save_register_global(machine, state->scroll);
}

VIDEO_START( mappy )
{
	mappy_state *state = (mappy_state *)machine->driver_data;

	state->bg_tilemap = tilemap_create(machine, mappy_get_tile_info,mappy_tilemap_scan,8,8,36,60);

	colortable_configure_tilemap_groups(machine->colortable, state->bg_tilemap, machine->gfx[0], 31);
	tilemap_set_scroll_cols(state->bg_tilemap, 36);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( superpac_videoram_w )
{
	mappy_state *state = (mappy_state *)space->machine->driver_data;

	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap,offset & 0x3ff);
}

WRITE8_HANDLER( mappy_videoram_w )
{
	mappy_state *state = (mappy_state *)space->machine->driver_data;

	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap,offset & 0x7ff);
}

WRITE8_HANDLER( superpac_flipscreen_w )
{
	flip_screen_set(space->machine, data & 1);
}

READ8_HANDLER( superpac_flipscreen_r )
{
	flip_screen_set(space->machine, 1);
	return 0xff;
}

WRITE8_HANDLER( mappy_scroll_w )
{
	mappy_state *state = (mappy_state *)space->machine->driver_data;

	state->scroll = offset >> 3;
}



/***************************************************************************

  Display refresh

***************************************************************************/

/* also used by toypop.c */
void mappy_draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, UINT8 *spriteram_base, int xoffs, int yoffs, int transcolor)
{
	UINT8 *spriteram = spriteram_base + 0x780;
	UINT8 *spriteram_2 = spriteram + 0x800;
	UINT8 *spriteram_3 = spriteram_2 + 0x800;
	int offs;

	for (offs = 0;offs < 0x80;offs += 2)
	{
		/* is it on? */
		if ((spriteram_3[offs+1] & 2) == 0)
		{
			static const UINT8 gfx_offs[2][2] =
			{
				{ 0, 1 },
				{ 2, 3 }
			};
			int sprite = spriteram[offs];
			int color = spriteram[offs+1];
			int sx = spriteram_2[offs+1] + 0x100 * (spriteram_3[offs+1] & 1) - 40 + xoffs;
			int sy = 256 - spriteram_2[offs] + yoffs + 1;	// sprites are buffered and delayed by one scanline
			int flipx = (spriteram_3[offs] & 0x01);
			int flipy = (spriteram_3[offs] & 0x02) >> 1;
			int sizex = (spriteram_3[offs] & 0x04) >> 2;
			int sizey = (spriteram_3[offs] & 0x08) >> 3;
			int x,y;

			sprite &= ~sizex;
			sprite &= ~(sizey << 1);

			sy -= 16 * sizey;
			sy = (sy & 0xff) - 32;	// fix wraparound

			if (flip_screen_get(machine))
			{
				flipx ^= 1;
				flipy ^= 1;
				sy += 40;
			}

			for (y = 0;y <= sizey;y++)
			{
				for (x = 0;x <= sizex;x++)
				{
					drawgfx_transmask(bitmap,cliprect,machine->gfx[1],
						sprite + gfx_offs[y ^ (sizey * flipy)][x ^ (sizex * flipx)],
						color,
						flipx,flipy,
						sx + 16*x,sy + 16*y,
						colortable_get_transpen_mask(machine->colortable, machine->gfx[1], color, transcolor));
				}
			}
		}
	}
}


/*
sprite format:

spriteram
0   xxxxxxxx  tile number
1   --xxxxxx  color

spriteram_2
0   xxxxxxxx  Y position
1   xxxxxxxx  X position

spriteram_3
0   xx------  tile number LSB
0   --xx----  Y size (16, 8, 32, 4?)
0   ----xx--  X size (16, 8, 32, 4?)
0   ------x-  Y flip
0   -------x  X flip
1   ------x-  disable
1   -------x  X position MSB
*/

static void phozon_draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, UINT8 *spriteram_base )
{
	UINT8 *spriteram = spriteram_base + 0x780;
	UINT8 *spriteram_2 = spriteram + 0x800;
	UINT8 *spriteram_3 = spriteram_2 + 0x800;
	int offs;

	for (offs = 0;offs < 0x80;offs += 2)
	{
		/* is it on? */
		if ((spriteram_3[offs+1] & 2) == 0)
		{
			static const UINT8 size[4] = { 1, 0, 3, 0 };	/* 16, 8, 32 pixels; fourth combination unused? */
			static const UINT8 gfx_offs[4][4] =
			{
				{ 0, 1, 4, 5 },
				{ 2, 3, 6, 7 },
				{ 8, 9,12,13 },
				{10,11,14,15 }
			};
			int sprite = (spriteram[offs] << 2) | ((spriteram_3[offs] & 0xc0) >> 6);
			int color = spriteram[offs+1] & 0x3f;
			int sx = spriteram_2[offs+1] + 0x100 * (spriteram_3[offs+1] & 1) - 69;
			int sy = 256 - spriteram_2[offs];
			int flipx = (spriteram_3[offs] & 0x01);
			int flipy = (spriteram_3[offs] & 0x02) >> 1;
			int sizex = size[(spriteram_3[offs] & 0x0c) >> 2];
			int sizey = size[(spriteram_3[offs] & 0x30) >> 4];
			int x,y;

			sy -= 8 * sizey;
			sy = (sy & 0xff) - 32;	// fix wraparound

			if (flip_screen_get(machine))
			{
				flipx ^= 1;
				flipy ^= 1;
				sy += 40;
			}

			for (y = 0;y <= sizey;y++)
			{
				for (x = 0;x <= sizex;x++)
				{
					drawgfx_transmask(bitmap,cliprect,machine->gfx[1],
						sprite + gfx_offs[y ^ (sizey * flipy)][x ^ (sizex * flipx)],
						color,
						flipx,flipy,
						sx + 8*x,sy + 8*y,
						colortable_get_transpen_mask(machine->colortable, machine->gfx[1], color, 31));
				}
			}
		}
	}
}


VIDEO_UPDATE( superpac )
{
	mappy_state *state = (mappy_state *)screen->machine->driver_data;
	bitmap_t *sprite_bitmap = state->sprite_bitmap;
	int x,y;

	tilemap_set_scrolldx(state->bg_tilemap, 0, 96);
	tilemap_set_scrolldy(state->bg_tilemap, 0, 0);

	tilemap_draw(bitmap,cliprect,state->bg_tilemap,TILEMAP_DRAW_OPAQUE | TILEMAP_DRAW_ALL_CATEGORIES,0);

	bitmap_fill(sprite_bitmap,cliprect,15);
	mappy_draw_sprites(screen->machine,sprite_bitmap,cliprect,state->spriteram,0,0,15);
	copybitmap_trans(bitmap,sprite_bitmap,0,0,0,0,cliprect,15);

	/* Redraw the high priority characters */
	tilemap_draw(bitmap,cliprect,state->bg_tilemap,1,0);

	/* sprite color 0 still has priority over that (ghost eyes in Pac 'n Pal) */
	for (y = 0;y < sprite_bitmap->height;y++)
	{
		for (x = 0;x < sprite_bitmap->width;x++)
		{
			if (*BITMAP_ADDR16(sprite_bitmap, y, x) == 0)
				*BITMAP_ADDR16(bitmap, y, x) = 0;
		}
	}
	return 0;
}

VIDEO_UPDATE( phozon )
{
	mappy_state *state = (mappy_state *)screen->machine->driver_data;

	/* flip screen control is embedded in RAM */
	flip_screen_set(screen->machine, state->spriteram[0x1f7f-0x800] & 1);

	tilemap_set_scrolldx(state->bg_tilemap, 0, 96);
	tilemap_set_scrolldy(state->bg_tilemap, 0, 0);

	tilemap_draw(bitmap,cliprect,state->bg_tilemap,TILEMAP_DRAW_OPAQUE | TILEMAP_DRAW_ALL_CATEGORIES,0);

	phozon_draw_sprites(screen->machine,bitmap,cliprect,state->spriteram);

	/* Redraw the high priority characters */
	tilemap_draw(bitmap,cliprect,state->bg_tilemap,1,0);
	return 0;
}

VIDEO_UPDATE( mappy )
{
	mappy_state *state = (mappy_state *)screen->machine->driver_data;
	int offs;

	tilemap_set_scrolldx(state->bg_tilemap, 0, 96);
	tilemap_set_scrolldy(state->bg_tilemap, 0, 0);

	for (offs = 2;offs < 34;offs++)
		tilemap_set_scrolly(state->bg_tilemap,offs,state->scroll);

	tilemap_draw(bitmap,cliprect,state->bg_tilemap,TILEMAP_DRAW_OPAQUE | TILEMAP_DRAW_ALL_CATEGORIES,0);

	mappy_draw_sprites(screen->machine,bitmap,cliprect,state->spriteram,0,0,15);

	/* Redraw the high priority characters */
	tilemap_draw(bitmap,cliprect,state->bg_tilemap,1,0);
	return 0;
}
