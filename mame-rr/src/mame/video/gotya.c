#include "emu.h"
#include "video/resnet.h"
#include "includes/gotya.h"


/***************************************************************************

  Convert the color PROMs into a more useable format.

***************************************************************************/

PALETTE_INIT( gotya )
{
	static const int resistances_rg[3] = { 1000, 470, 220 };
	static const int resistances_b [2] = { 470, 220 };
	double rweights[3], gweights[3], bweights[2];
	int i;

	/* compute the color output resistor weights */
	compute_resistor_weights(0,	255, -1.0,
			3, &resistances_rg[0], rweights, 0, 0,
			3, &resistances_rg[0], gweights, 0, 0,
			2, &resistances_b[0],  bweights, 0, 0);

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 32);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x20; i++)
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

	for (i = 0; i < 0x40; i++)
	{
		UINT8 ctabentry = color_prom[i] & 0x07;
		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}
}

WRITE8_HANDLER( gotya_videoram_w )
{
	gotya_state *state = (gotya_state *)space->machine->driver_data;
	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

WRITE8_HANDLER( gotya_colorram_w )
{
	gotya_state *state = (gotya_state *)space->machine->driver_data;
	state->colorram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

WRITE8_HANDLER( gotya_video_control_w )
{
	gotya_state *state = (gotya_state *)space->machine->driver_data;

	/* bit 0 - scroll bit 8
       bit 1 - flip screen
       bit 2 - sound disable ??? */

	state->scroll_bit_8 = data & 0x01;

	if (flip_screen_get(space->machine) != (data & 0x02))
	{
		flip_screen_set(space->machine, data & 0x02);
		tilemap_mark_all_tiles_dirty_all(space->machine);
	}
}

static TILE_GET_INFO( get_bg_tile_info )
{
	gotya_state *state = (gotya_state *)machine->driver_data;
	int code = state->videoram[tile_index];
	int color = state->colorram[tile_index] & 0x0f;

	SET_TILE_INFO(0, code, color, 0);
}

static TILEMAP_MAPPER( tilemap_scan_rows_thehand )
{
	/* logical (col,row) -> memory offset */
	row = 31 - row;
	col = 63 - col;
	return ((row) * (num_cols >> 1)) + (col & 31) + ((col >> 5) * 0x400);
}

VIDEO_START( gotya )
{
	gotya_state *state = (gotya_state *)machine->driver_data;
	state->bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows_thehand, 8, 8, 64, 32);
}

static void draw_status_row( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int sx, int col )
{
	gotya_state *state = (gotya_state *)machine->driver_data;
	int row;

	if (flip_screen_get(machine))
	{
		sx = 35 - sx;
	}

	for (row = 29; row >= 0; row--)
	{
		int sy;

		if (flip_screen_get(machine))
			sy = row;
		else
			sy = 31 - row;

		drawgfx_opaque(bitmap,cliprect,
			machine->gfx[0],
			state->videoram2[row * 32 + col],
			state->videoram2[row * 32 + col + 0x10] & 0x0f,
			flip_screen_x_get(machine), flip_screen_y_get(machine),
			8 * sx, 8 * sy);
	}
}

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	gotya_state *state = (gotya_state *)machine->driver_data;
	UINT8 *spriteram = state->spriteram;
	int offs;

	for (offs = 2; offs < 0x0e; offs += 2)
	{
		int code = spriteram[offs + 0x01] >> 2;
		int color = spriteram[offs + 0x11] & 0x0f;
		int sx = 256 - spriteram[offs + 0x10] + (spriteram[offs + 0x01] & 0x01) * 256;
		int sy = spriteram[offs + 0x00];

		if (flip_screen_get(machine))
			sy = 240 - sy;

		drawgfx_transpen(bitmap,cliprect,
			machine->gfx[1],
			code, color,
			flip_screen_x_get(machine), flip_screen_y_get(machine),
			sx, sy, 0);
	}
}

static void draw_status( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	draw_status_row(machine, bitmap, cliprect, 0,  1);
	draw_status_row(machine, bitmap, cliprect, 1,  0);
	draw_status_row(machine, bitmap, cliprect, 2,  2);	/* these two are blank, but I dont' know if the data comes */
	draw_status_row(machine, bitmap, cliprect, 33, 13);	/* from RAM or 'hardcoded' into the hardware. Likely the latter */
	draw_status_row(machine, bitmap, cliprect, 35, 14);
	draw_status_row(machine, bitmap, cliprect, 34, 15);
}

VIDEO_UPDATE( gotya )
{
	gotya_state *state = (gotya_state *)screen->machine->driver_data;
	tilemap_set_scrollx(state->bg_tilemap, 0, -(*state->scroll + (state->scroll_bit_8 * 256)) - 2 * 8);
	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	draw_sprites(screen->machine, bitmap, cliprect);
	draw_status(screen->machine, bitmap, cliprect);
	return 0;
}
