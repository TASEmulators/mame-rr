/***************************************************************************

  video/jack.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/jack.h"


WRITE8_HANDLER( jack_videoram_w )
{
	jack_state *state = (jack_state *)space->machine->driver_data;
	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

WRITE8_HANDLER( jack_colorram_w )
{
	jack_state *state = (jack_state *)space->machine->driver_data;
	state->colorram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

WRITE8_HANDLER( jack_paletteram_w )
{
	/* RGB output is inverted */
	paletteram_BBGGGRRR_w(space, offset, ~data);
}

READ8_HANDLER( jack_flipscreen_r )
{
	flip_screen_set(space->machine, offset);
	return 0;
}

WRITE8_HANDLER( jack_flipscreen_w )
{
	flip_screen_set(space->machine, offset);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	jack_state *state = (jack_state *)machine->driver_data;
	int code = state->videoram[tile_index] + ((state->colorram[tile_index] & 0x18) << 5);
	int color = state->colorram[tile_index] & 0x07;

	// striv: state->colorram[tile_index] & 0x80 ???

	SET_TILE_INFO(0, code, color, 0);
}

static UINT32 tilemap_scan_cols_flipy( UINT32 col, UINT32 row, UINT32 num_cols, UINT32 num_rows )
{
	/* logical (col,row) -> memory offset */
	return (col * num_rows) + (num_rows - 1 - row);
}

VIDEO_START( jack )
{
	jack_state *state = (jack_state *)machine->driver_data;
	state->bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_cols_flipy, 8, 8, 32, 32);
}

static void jack_draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	jack_state *state = (jack_state *)machine->driver_data;
	UINT8 *spriteram = state->spriteram;
	int offs;

	for (offs = state->spriteram_size - 4; offs >= 0; offs -= 4)
	{
		int sx, sy, num, color, flipx, flipy;

		sx    = spriteram[offs + 1];
		sy    = spriteram[offs];
		num   = spriteram[offs + 2] + ((spriteram[offs + 3] & 0x08) << 5);
		color = spriteram[offs + 3] & 0x07;
		flipx = (spriteram[offs + 3] & 0x80);
		flipy = (spriteram[offs + 3] & 0x40);

		if (flip_screen_get(machine))
		{
			sx = 248 - sx;
			sy = 248 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap,cliprect,machine->gfx[0],
				num,
				color,
				flipx,flipy,
				sx,sy,0);
	}
}

VIDEO_UPDATE( jack )
{
	jack_state *state = (jack_state *)screen->machine->driver_data;
	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	jack_draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}

/*
   Joinem has a bit different video hardware with proms based palette,
   3bpp gfx and different banking / colors bits
*/

PALETTE_INIT( joinem )
{
	int i;

	for (i = 0; i < machine->total_colors(); i++)
	{
		int bit0, bit1, bit2, r, g, b;
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = 0;
		bit1 = (color_prom[i] >> 6) & 0x01;
		bit2 = (color_prom[i] >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine, i, MAKE_RGB(r,g,b));
	}
}

static TILE_GET_INFO( joinem_get_bg_tile_info )
{
	jack_state *state = (jack_state *)machine->driver_data;
	int code = state->videoram[tile_index] + ((state->colorram[tile_index] & 0x03) << 8);
	int color = (state->colorram[tile_index] & 0x38) >> 3;

	SET_TILE_INFO(0, code, color, 0);
}

VIDEO_START( joinem )
{
	jack_state *state = (jack_state *)machine->driver_data;
	state->bg_tilemap = tilemap_create(machine, joinem_get_bg_tile_info, tilemap_scan_cols_flipy, 8, 8, 32, 32);
}

static void joinem_draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	jack_state *state = (jack_state *)machine->driver_data;
	UINT8 *spriteram = state->spriteram;
	int offs;

	for (offs = state->spriteram_size - 4; offs >= 0; offs -= 4)
	{
		int sx, sy, num, color, flipx, flipy;

		sx    = spriteram[offs + 1];
		sy    = spriteram[offs];
		num   = spriteram[offs + 2] + ((spriteram[offs + 3] & 0x01) << 8);
		color = (spriteram[offs + 3] & 0x38) >> 3;
		flipx = (spriteram[offs + 3] & 0x80);
		flipy = (spriteram[offs + 3] & 0x40);

		if (flip_screen_get(machine))
		{
			sx = 248 - sx;
			sy = 248 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap,cliprect,machine->gfx[0],
				num,
				color,
				flipx,flipy,
				sx,sy,0);
	}
}

VIDEO_UPDATE( joinem )
{
	jack_state *state = (jack_state *)screen->machine->driver_data;
	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	joinem_draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}
