/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/espial.h"

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Espial has two 256x4 palette PROMs.

  I don't know for sure how the palette PROMs are connected to the RGB
  output, but it's probably the usual:

  bit 3 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 220 ohm resistor  -- GREEN
  bit 0 -- 470 ohm resistor  -- GREEN
  bit 3 -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

***************************************************************************/

PALETTE_INIT( espial )
{
	int i;

	for (i = 0; i < machine->total_colors(); i++)
	{
		int bit0, bit1, bit2, r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i + machine->total_colors()] >> 0) & 0x01;
		bit2 = (color_prom[i + machine->total_colors()] >> 1) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = 0;
		bit1 = (color_prom[i + machine->total_colors()] >> 2) & 0x01;
		bit2 = (color_prom[i + machine->total_colors()] >> 3) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine, i, MAKE_RGB(r,g,b));
	}
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_tile_info )
{
	espial_state *state = (espial_state *)machine->driver_data;
	UINT8 code = state->videoram[tile_index];
	UINT8 col = state->colorram[tile_index];
	UINT8 attr = state->attributeram[tile_index];
	SET_TILE_INFO(0,
				  code | ((attr & 0x03) << 8),
				  col & 0x3f,
				  TILE_FLIPYX(attr >> 2));
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START( espial )
{
	espial_state *state = (espial_state *)machine->driver_data;

	state->bg_tilemap = tilemap_create(machine, get_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	tilemap_set_scroll_cols(state->bg_tilemap, 32);

	state_save_register_global(machine, state->flipscreen);
}

VIDEO_START( netwars )
{
	espial_state *state = (espial_state *)machine->driver_data;

	/* Net Wars has a tile map that's twice as big as Espial's */
	state->bg_tilemap = tilemap_create(machine, get_tile_info, tilemap_scan_rows, 8, 8, 32, 64);

	tilemap_set_scroll_cols(state->bg_tilemap, 32);
	tilemap_set_scrolldy(state->bg_tilemap, 0, 0x100);

	state_save_register_global(machine, state->flipscreen);
}


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

WRITE8_HANDLER( espial_videoram_w )
{
	espial_state *state = (espial_state *)space->machine->driver_data;

	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}


WRITE8_HANDLER( espial_colorram_w )
{
	espial_state *state = (espial_state *)space->machine->driver_data;

	state->colorram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}


WRITE8_HANDLER( espial_attributeram_w )
{
	espial_state *state = (espial_state *)space->machine->driver_data;

	state->attributeram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}


WRITE8_HANDLER( espial_scrollram_w )
{
	espial_state *state = (espial_state *)space->machine->driver_data;

	state->scrollram[offset] = data;
	tilemap_set_scrolly(state->bg_tilemap, offset, data);
}


WRITE8_HANDLER( espial_flipscreen_w )
{
	espial_state *state = (espial_state *)space->machine->driver_data;

	state->flipscreen = data;
	tilemap_set_flip(state->bg_tilemap, state->flipscreen ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);
}


/*************************************
 *
 *  Video update
 *
 *************************************/

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	espial_state *state = (espial_state *)machine->driver_data;
	int offs;

	/* Note that it is important to draw them exactly in this */
	/* order, to have the correct priorities. */
	for (offs = 0; offs < 16; offs++)
	{
		int sx, sy, code, color, flipx, flipy;


		sx = state->spriteram_1[offs + 16];
		sy = state->spriteram_2[offs];
		code = state->spriteram_1[offs] >> 1;
		color = state->spriteram_2[offs + 16];
		flipx = state->spriteram_3[offs] & 0x04;
		flipy = state->spriteram_3[offs] & 0x08;

		if (state->flipscreen)
		{
			flipx = !flipx;
			flipy = !flipy;
		}
		else
		{
			sy = 240 - sy;
		}

		if (state->spriteram_1[offs] & 1)	/* double height */
		{
			if (state->flipscreen)
			{
				drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
						code,color,
						flipx,flipy,
						sx,sy + 16,0);
				drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
						code + 1,
						color,
						flipx,flipy,
						sx,sy,0);
			}
			else
			{
				drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
						code,color,
						flipx,flipy,
						sx,sy - 16,0);
				drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
						code + 1,color,
						flipx,flipy,
						sx,sy,0);
			}
		}
		else
		{
			drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
					code,color,
					flipx,flipy,
					sx,sy,0);
		}
	}
}


VIDEO_UPDATE( espial )
{
	espial_state *state = (espial_state *)screen->machine->driver_data;

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}
