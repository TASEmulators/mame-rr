/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "includes/trackfld.h"

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Super Basketball has three 256x4 palette PROMs (one per gun) and two 256x4
  lookup table PROMs (one for characters, one for sprites).
  I don't know for sure how the palette PROMs are connected to the RGB
  output, but it's probably the usual:

  bit 3 -- 220 ohm resistor  -- RED/GREEN/BLUE
        -- 470 ohm resistor  -- RED/GREEN/BLUE
        -- 1  kohm resistor  -- RED/GREEN/BLUE
  bit 0 -- 2.2kohm resistor  -- RED/GREEN/BLUE

***************************************************************************/

PALETTE_INIT( sbasketb )
{
	static const int resistances[4] = { 2000, 1000, 470, 220 };
	double rweights[4], gweights[4], bweights[4];
	int i;

	/* compute the color output resistor weights */
	compute_resistor_weights(0,	255, -1.0,
			4, resistances, rweights, 1000, 0,
			4, resistances, gweights, 1000, 0,
			4, resistances, bweights, 1000, 0);

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 0x100);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x100; i++)
	{
		int bit0, bit1, bit2, bit3;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i + 0x000] >> 0) & 0x01;
		bit1 = (color_prom[i + 0x000] >> 1) & 0x01;
		bit2 = (color_prom[i + 0x000] >> 2) & 0x01;
		bit3 = (color_prom[i + 0x000] >> 3) & 0x01;
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

	/* color_prom now points to the beginning of the lookup table,*/
	color_prom += 0x300;

	/* characters use colors 0xf0-0xff */
	for (i = 0; i < 0x100; i++)
	{
		UINT8 ctabentry = (color_prom[i] & 0x0f) | 0xf0;
		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}

	/* sprites use colors 0-256 (?) in 16 banks */
	for (i = 0; i < 0x100; i++)
	{
		int j;

		for (j = 0; j < 0x10; j++)
		{
			UINT8 ctabentry = (j << 4) | (color_prom[i + 0x100] & 0x0f);
			colortable_entry_set_value(machine->colortable, 0x100 + ((j << 8) | i), ctabentry);
		}
	}
}

WRITE8_HANDLER( sbasketb_videoram_w )
{
	trackfld_state *state = (trackfld_state *)space->machine->driver_data;
	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

WRITE8_HANDLER( sbasketb_colorram_w )
{
	trackfld_state *state = (trackfld_state *)space->machine->driver_data;
	state->colorram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

WRITE8_HANDLER( sbasketb_flipscreen_w )
{
	if (flip_screen_get(space->machine) != data)
	{
		flip_screen_set(space->machine, data);
		tilemap_mark_all_tiles_dirty_all(space->machine);
	}
}

static TILE_GET_INFO( get_bg_tile_info )
{
	trackfld_state *state = (trackfld_state *)machine->driver_data;
	int code = state->videoram[tile_index] + ((state->colorram[tile_index] & 0x20) << 3);
	int color = state->colorram[tile_index] & 0x0f;
	int flags = ((state->colorram[tile_index] & 0x40) ? TILE_FLIPX : 0) | ((state->colorram[tile_index] & 0x80) ? TILE_FLIPY : 0);

	SET_TILE_INFO(0, code, color, flags);
}

VIDEO_START( sbasketb )
{
	trackfld_state *state = (trackfld_state *)machine->driver_data;

	state->bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	tilemap_set_scroll_cols(state->bg_tilemap, 32);
}

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	trackfld_state *state = (trackfld_state *)machine->driver_data;
	UINT8 *spriteram = state->spriteram;
	int offs = (*state->spriteram_select & 0x01) * 0x100;
	int i;

	for (i = 0; i < 64; i++, offs += 4)
	{
		int sx = spriteram[offs + 2];
		int sy = spriteram[offs + 3];

		if (sx || sy)
		{
			int code  =  spriteram[offs + 0] | ((spriteram[offs + 1] & 0x20) << 3);
			int color = (spriteram[offs + 1] & 0x0f) + 16 * *state->palettebank;
			int flipx =  spriteram[offs + 1] & 0x40;
			int flipy =  spriteram[offs + 1] & 0x80;

			if (flip_screen_get(machine))
			{
				sx = 240 - sx;
				sy = 240 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			drawgfx_transpen(bitmap,cliprect,
				machine->gfx[1],
				code, color,
				flipx, flipy,
				sx, sy, 0);
		}
	}
}

VIDEO_UPDATE( sbasketb )
{
	trackfld_state *state = (trackfld_state *)screen->machine->driver_data;
	int col;

	for (col = 6; col < 32; col++)
		tilemap_set_scrolly(state->bg_tilemap, col, *state->scroll);

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}
