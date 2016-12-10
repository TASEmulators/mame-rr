/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/commando.h"


WRITE8_HANDLER( commando_videoram_w )
{
	commando_state *state = (commando_state *)space->machine->driver_data;

	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

WRITE8_HANDLER( commando_colorram_w )
{
	commando_state *state = (commando_state *)space->machine->driver_data;

	state->colorram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

WRITE8_HANDLER( commando_videoram2_w )
{
	commando_state *state = (commando_state *)space->machine->driver_data;

	state->videoram2[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap, offset);
}

WRITE8_HANDLER( commando_colorram2_w )
{
	commando_state *state = (commando_state *)space->machine->driver_data;

	state->colorram2[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap, offset);
}

WRITE8_HANDLER( commando_scrollx_w )
{
	commando_state *state = (commando_state *)space->machine->driver_data;

	state->scroll_x[offset] = data;
	tilemap_set_scrollx(state->bg_tilemap, 0, state->scroll_x[0] | (state->scroll_x[1] << 8));
}

WRITE8_HANDLER( commando_scrolly_w )
{
	commando_state *state = (commando_state *)space->machine->driver_data;

	state->scroll_y[offset] = data;
	tilemap_set_scrolly(state->bg_tilemap, 0, state->scroll_y[0] | (state->scroll_y[1] << 8));
}

WRITE8_HANDLER( commando_c804_w )
{
	commando_state *state = (commando_state *)space->machine->driver_data;

	// bits 0 and 1 are coin counters
	coin_counter_w(space->machine, 0, data & 0x01);
	coin_counter_w(space->machine, 1, data & 0x02);

	// bit 4 resets the sound CPU
	cpu_set_input_line(state->audiocpu, INPUT_LINE_RESET, (data & 0x10) ? ASSERT_LINE : CLEAR_LINE);

	// bit 7 flips screen
	flip_screen_set(space->machine, data & 0x80);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	commando_state *state = (commando_state *)machine->driver_data;
	int attr = state->colorram[tile_index];
	int code = state->videoram[tile_index] + ((attr & 0xc0) << 2);
	int color = attr & 0x0f;
	int flags = TILE_FLIPYX((attr & 0x30) >> 4);

	SET_TILE_INFO(1, code, color, flags);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	commando_state *state = (commando_state *)machine->driver_data;
	int attr = state->colorram2[tile_index];
	int code = state->videoram2[tile_index] + ((attr & 0xc0) << 2);
	int color = attr & 0x0f;
	int flags = TILE_FLIPYX((attr & 0x30) >> 4);

	SET_TILE_INFO(0, code, color, flags);
}

VIDEO_START( commando )
{
	commando_state *state = (commando_state *)machine->driver_data;

	state->bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_cols, 16, 16, 32, 32);
	state->fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);

	tilemap_set_transparent_pen(state->fg_tilemap, 3);
}

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	UINT8 *buffered_spriteram = machine->generic.buffered_spriteram.u8;
	int offs;

	for (offs = machine->generic.spriteram_size - 4; offs >= 0; offs -= 4)
	{
		// bit 1 of attr is not used
		int attr = buffered_spriteram[offs + 1];
		int bank = (attr & 0xc0) >> 6;
		int code = buffered_spriteram[offs] + 256 * bank;
		int color = (attr & 0x30) >> 4;
		int flipx = attr & 0x04;
		int flipy = attr & 0x08;
		int sx = buffered_spriteram[offs + 3] - ((attr & 0x01) << 8);
		int sy = buffered_spriteram[offs + 2];

		if (flip_screen_get(machine))
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		if (bank < 3)
			drawgfx_transpen(bitmap, cliprect, machine->gfx[2], code, color, flipx, flipy, sx, sy, 15);
	}
}

VIDEO_UPDATE( commando )
{
	commando_state *state = (commando_state *)screen->machine->driver_data;

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	draw_sprites(screen->machine, bitmap, cliprect);
	tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0);
	return 0;
}

VIDEO_EOF( commando )
{
	const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);

	buffer_spriteram_w(space, 0, 0);
}
