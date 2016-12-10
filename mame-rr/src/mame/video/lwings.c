/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/lwings.h"

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILEMAP_MAPPER( get_bg2_memory_offset )
{
	return (row * 0x800) | (col * 2);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	lwings_state *state = (lwings_state *)machine->driver_data;
	int code = state->fgvideoram[tile_index];
	int color = state->fgvideoram[tile_index + 0x400];
	SET_TILE_INFO(
			0,
			code + ((color & 0xc0) << 2),
			color & 0x0f,
			TILE_FLIPYX((color & 0x30) >> 4));
}

static TILE_GET_INFO( lwings_get_bg1_tile_info )
{
	lwings_state *state = (lwings_state *)machine->driver_data;
	int code = state->bg1videoram[tile_index];
	int color = state->bg1videoram[tile_index + 0x400];
	SET_TILE_INFO(
			1,
			code + ((color & 0xe0) << 3),
			color & 0x07,
			TILE_FLIPYX((color & 0x18) >> 3));
}

static TILE_GET_INFO( trojan_get_bg1_tile_info )
{
	lwings_state *state = (lwings_state *)machine->driver_data;
	int code = state->bg1videoram[tile_index];
	int color = state->bg1videoram[tile_index + 0x400];
	code += (color & 0xe0)<<3;
	SET_TILE_INFO(
			1,
			code,
			state->bg2_avenger_hw ? ((color & 7) ^ 6) : (color & 7),
			((color & 0x10) ? TILE_FLIPX : 0));

	tileinfo->group = (color & 0x08) >> 3;
}

static TILE_GET_INFO( get_bg2_tile_info )
{
	lwings_state *state = (lwings_state *)machine->driver_data;
	int code, color;
	UINT8 *rom = memory_region(machine, "gfx5");
	int mask = memory_region_length(machine, "gfx5") - 1;

	tile_index = (tile_index + state->bg2_image * 0x20) & mask;
	code = rom[tile_index];
	color = rom[tile_index + 1];
	SET_TILE_INFO(
			3,
			code + ((color & 0x80) << 1),
			color & 0x07,
			TILE_FLIPYX((color & 0x30) >> 4));
}

/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( lwings )
{
	lwings_state *state = (lwings_state *)machine->driver_data;

	state->fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	state->bg1_tilemap = tilemap_create(machine, lwings_get_bg1_tile_info, tilemap_scan_cols, 16, 16, 32, 32);

	tilemap_set_transparent_pen(state->fg_tilemap, 3);
}

VIDEO_START( trojan )
{
	lwings_state *state = (lwings_state *)machine->driver_data;

	state->fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	state->bg1_tilemap = tilemap_create(machine, trojan_get_bg1_tile_info,tilemap_scan_cols, 16, 16, 32, 32);
	state->bg2_tilemap = tilemap_create(machine, get_bg2_tile_info, get_bg2_memory_offset, 16, 16, 32, 16);

	tilemap_set_transparent_pen(state->fg_tilemap, 3);
	tilemap_set_transmask(state->bg1_tilemap, 0, 0xffff, 0x0001); /* split type 0 is totally transparent in front half */
	tilemap_set_transmask(state->bg1_tilemap, 1, 0xf07f, 0x0f81); /* split type 1 has pens 7-11 opaque in front half */

	state->bg2_avenger_hw = 0;
}

VIDEO_START( avengers )
{
	lwings_state *state = (lwings_state *)machine->driver_data;

	VIDEO_START_CALL(trojan);
	state->bg2_avenger_hw = 1;
}

/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( lwings_fgvideoram_w )
{
	lwings_state *state = (lwings_state *)space->machine->driver_data;
	state->fgvideoram[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap, offset & 0x3ff);
}

WRITE8_HANDLER( lwings_bg1videoram_w )
{
	lwings_state *state = (lwings_state *)space->machine->driver_data;
	state->bg1videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg1_tilemap, offset & 0x3ff);
}


WRITE8_HANDLER( lwings_bg1_scrollx_w )
{
	lwings_state *state = (lwings_state *)space->machine->driver_data;
	state->scroll_x[offset] = data;
	tilemap_set_scrollx(state->bg1_tilemap, 0, state->scroll_x[0] | (state->scroll_x[1] << 8));
}

WRITE8_HANDLER( lwings_bg1_scrolly_w )
{
	lwings_state *state = (lwings_state *)space->machine->driver_data;
	state->scroll_y[offset] = data;
	tilemap_set_scrolly(state->bg1_tilemap, 0, state->scroll_y[0] | (state->scroll_y[1] << 8));
}

WRITE8_HANDLER( trojan_bg2_scrollx_w )
{
	lwings_state *state = (lwings_state *)space->machine->driver_data;
	tilemap_set_scrollx(state->bg2_tilemap, 0, data);
}

WRITE8_HANDLER( trojan_bg2_image_w )
{
	lwings_state *state = (lwings_state *)space->machine->driver_data;

	if (state->bg2_image != data)
	{
		state->bg2_image = data;
		tilemap_mark_all_tiles_dirty(state->bg2_tilemap);
	}
}


/***************************************************************************

  Display refresh

***************************************************************************/

INLINE int is_sprite_on( UINT8 *buffered_spriteram, int offs )
{
	int sx, sy;

	sx = buffered_spriteram[offs + 3] - 0x100 * (buffered_spriteram[offs + 1] & 0x01);
	sy = buffered_spriteram[offs + 2];

	return sx || sy;
}

static void lwings_draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	UINT8 *buffered_spriteram = machine->generic.buffered_spriteram.u8;
	int offs;

	for (offs = machine->generic.spriteram_size - 4; offs >= 0; offs -= 4)
	{
		if (is_sprite_on(buffered_spriteram, offs))
		{
			int code, color, sx, sy, flipx, flipy;

			sx = buffered_spriteram[offs + 3] - 0x100 * (buffered_spriteram[offs + 1] & 0x01);
			sy = buffered_spriteram[offs + 2];
			if (sy > 0xf8)
				sy -= 0x100;
			code = buffered_spriteram[offs] | (buffered_spriteram[offs + 1] & 0xc0) << 2;
			color = (buffered_spriteram[offs + 1] & 0x38) >> 3;
			flipx = buffered_spriteram[offs + 1] & 0x02;
			flipy = buffered_spriteram[offs + 1] & 0x04;

			if (flip_screen_get(machine))
			{
				sx = 240 - sx;
				sy = 240 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			drawgfx_transpen(bitmap,cliprect,machine->gfx[2],
					code,color,
					flipx,flipy,
					sx,sy,15);
		}
	}
}

static void trojan_draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	lwings_state *state = (lwings_state *)machine->driver_data;
	UINT8 *buffered_spriteram = machine->generic.buffered_spriteram.u8;
	int offs;

	for (offs = machine->generic.spriteram_size - 4; offs >= 0; offs -= 4)
	{
		if (is_sprite_on(buffered_spriteram, offs))
		{
			int code, color, sx, sy, flipx, flipy;

			sx = buffered_spriteram[offs + 3] - 0x100 * (buffered_spriteram[offs + 1] & 0x01);
			sy = buffered_spriteram[offs + 2];
			if (sy > 0xf8)
				sy -= 0x100;
			code = buffered_spriteram[offs] |
				   ((buffered_spriteram[offs + 1] & 0x20) << 4) |
				   ((buffered_spriteram[offs + 1] & 0x40) << 2) |
				   ((buffered_spriteram[offs + 1] & 0x80) << 3);
			color = (buffered_spriteram[offs + 1] & 0x0e) >> 1;

			if (state->bg2_avenger_hw)
			{
				flipx = 0;										/* Avengers */
				flipy = ~buffered_spriteram[offs + 1] & 0x10;
			}
			else
			{
				flipx = buffered_spriteram[offs + 1] & 0x10;	/* Trojan */
				flipy = 1;
			}

			if (flip_screen_get(machine))
			{
				sx = 240 - sx;
				sy = 240 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			drawgfx_transpen(bitmap,cliprect,machine->gfx[2],
					code,color,
					flipx,flipy,
					sx,sy,15);
		}
	}
}

VIDEO_UPDATE( lwings )
{
	lwings_state *state = (lwings_state *)screen->machine->driver_data;

	tilemap_draw(bitmap, cliprect, state->bg1_tilemap, 0, 0);
	lwings_draw_sprites(screen->machine, bitmap, cliprect);
	tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0);
	return 0;
}

VIDEO_UPDATE( trojan )
{
	lwings_state *state = (lwings_state *)screen->machine->driver_data;

	tilemap_draw(bitmap, cliprect, state->bg2_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, state->bg1_tilemap, TILEMAP_DRAW_LAYER1, 0);
	trojan_draw_sprites(screen->machine, bitmap, cliprect);
	tilemap_draw(bitmap, cliprect, state->bg1_tilemap, TILEMAP_DRAW_LAYER0, 0);
	tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0);
	return 0;
}

VIDEO_EOF( lwings )
{
	const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);

	buffer_spriteram_w(space, 0, 0);
}
