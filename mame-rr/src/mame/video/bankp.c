/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/bankp.h"


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Bank Panic has a 32x8 palette PROM (I'm not sure whether the second 16
  bytes are used - they contain the same colors as the first 16 with only
  one different) and two 256x4 lookup table PROMs (one for charset #1, one
  for charset #2 - only the first 128 nibbles seem to be used).

  I don't know for sure how the palette PROM is connected to the RGB output,
  but it's probably the usual:

  bit 7 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

***************************************************************************/

PALETTE_INIT( bankp )
{
	int i;

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 32);

	for (i = 0; i < 32; i++)
	{
		int bit0, bit1, bit2, r, g, b;

		/* red component */
		bit0 = (*color_prom >> 0) & 0x01;
		bit1 = (*color_prom >> 1) & 0x01;
		bit2 = (*color_prom >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (*color_prom >> 3) & 0x01;
		bit1 = (*color_prom >> 4) & 0x01;
		bit2 = (*color_prom >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = 0;
		bit1 = (*color_prom >> 6) & 0x01;
		bit2 = (*color_prom >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		colortable_palette_set_color(machine->colortable, i, MAKE_RGB(r,g,b));

		color_prom++;
	}

	/* color_prom now points to the beginning of the lookup table */

	/* charset #1 lookup table */
	for (i = 0; i < machine->gfx[0]->total_colors * machine->gfx[0]->color_granularity; i++)
		colortable_entry_set_value(machine->colortable, machine->gfx[0]->color_base + i, *color_prom++ & 0x0f);

	color_prom += 128;	/* skip the bottom half of the PROM - seems to be not used */

	/* charset #2 lookup table */
	for (i = 0; i < machine->gfx[1]->total_colors * machine->gfx[1]->color_granularity; i++)
		colortable_entry_set_value(machine->colortable, machine->gfx[1]->color_base + i, *color_prom++ & 0x0f);

	/* the bottom half of the PROM seems to be not used */
}

WRITE8_HANDLER( bankp_scroll_w )
{
	bankp_state *state = (bankp_state *)space->machine->driver_data;

	state->scroll_x = data;
}

WRITE8_HANDLER( bankp_videoram_w )
{
	bankp_state *state = (bankp_state *)space->machine->driver_data;

	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap, offset);
}

WRITE8_HANDLER( bankp_colorram_w )
{
	bankp_state *state = (bankp_state *)space->machine->driver_data;

	state->colorram[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap, offset);
}

WRITE8_HANDLER( bankp_videoram2_w )
{
	bankp_state *state = (bankp_state *)space->machine->driver_data;

	state->videoram2[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

WRITE8_HANDLER( bankp_colorram2_w )
{
	bankp_state *state = (bankp_state *)space->machine->driver_data;

	state->colorram2[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

WRITE8_HANDLER( bankp_out_w )
{
	bankp_state *state = (bankp_state *)space->machine->driver_data;
	/* bits 0-1 are playfield priority */
	/* TODO: understand how this works */
	state->priority = data & 0x03;

	/* bits 2-3 unknown (2 is used) */

	/* bit 4 controls NMI */
	interrupt_enable_w(space, 0, (data & 0x10) >> 4);

	/* bit 5 controls screen flip */
	flip_screen_set(space->machine, data & 0x20);

	/* bits 6-7 unknown */
}

static TILE_GET_INFO( get_bg_tile_info )
{
	bankp_state *state = (bankp_state *)machine->driver_data;
	int code = state->videoram2[tile_index] + 256 * (state->colorram2[tile_index] & 0x07);
	int color = state->colorram2[tile_index] >> 4;
	int flags = (state->colorram2[tile_index] & 0x08) ? TILE_FLIPX : 0;

	SET_TILE_INFO(1, code, color, flags);
	tileinfo->group = color;
}

static TILE_GET_INFO( get_fg_tile_info )
{
	bankp_state *state = (bankp_state *)machine->driver_data;
	int code = state->videoram[tile_index] + 256 * ((state->colorram[tile_index] & 3) >> 0);
	int color = state->colorram[tile_index] >> 3;
	int flags = (state->colorram[tile_index] & 0x04) ? TILE_FLIPX : 0;

	SET_TILE_INFO(0, code, color, flags);
	tileinfo->group = color;
}

VIDEO_START( bankp )
{
	bankp_state *state = (bankp_state *)machine->driver_data;

	state->bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	state->fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);

	colortable_configure_tilemap_groups(machine->colortable, state->bg_tilemap, machine->gfx[1], 0);
	colortable_configure_tilemap_groups(machine->colortable, state->fg_tilemap, machine->gfx[0], 0);

	state_save_register_global(machine, state->scroll_x);
	state_save_register_global(machine, state->priority);
}

VIDEO_UPDATE( bankp )
{
	bankp_state *state = (bankp_state *)screen->machine->driver_data;

	if (flip_screen_get(screen->machine))
	{
		tilemap_set_scrollx(state->fg_tilemap, 0, -state->scroll_x);
		tilemap_set_scrollx(state->bg_tilemap, 0, 0);
	}
	else
	{
		tilemap_set_scrollx(state->fg_tilemap, 0, state->scroll_x);
		tilemap_set_scrollx(state->bg_tilemap, 0, 0);
	}


	// only one bit matters?
	switch (state->priority)
	{
	case 0: // combat hawk uses this
		tilemap_draw(bitmap, cliprect, state->bg_tilemap, TILEMAP_DRAW_OPAQUE, 0);
		tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0);
		break;
	case 1:
		tilemap_draw(bitmap, cliprect, state->bg_tilemap, TILEMAP_DRAW_OPAQUE, 0);
		tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0);
		break;
	case 2:
		tilemap_draw(bitmap, cliprect, state->fg_tilemap, TILEMAP_DRAW_OPAQUE, 0);
		tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
		break;
	case 3:
		tilemap_draw(bitmap, cliprect, state->fg_tilemap, TILEMAP_DRAW_OPAQUE, 0); // just a guess
		tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
		break;
	}
	return 0;
}
