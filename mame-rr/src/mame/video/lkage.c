/***************************************************************************

    video/lkage.c


    lkage_scroll[0x00]: text layer horizontal scroll
    lkage_scroll[0x01]: text layer vertical scroll
    lkage_scroll[0x02]: foreground layer horizontal scroll
    lkage_scroll[0x03]: foreground layer vertical scroll
    lkage_scroll[0x04]: background layer horizontal scroll
    lkage_scroll[0x05]: background layer vertical scroll

    lkage_vreg[0]: 0x00,0x04
        0x04: fg tile bank select
        0x08: ?

    lkage_vreg[1]: 0x7d
        0xf0: background/foreground palette select
        0x08: bg tile bank select
        0x07: priority config?

    lkage_vreg[2]: 0xf3
        0x03: flip screen x/y
        0xf0: normally 1111, but 1001 and 0001 inbetween stages (while the
        backgrounds are are being redrawn). These bits are probably used to enable
        individual layers, but we have no way of knowing the mapping.

    lkage_vreg:
        04 7d f3 : title screen 101
        0c 7d f3 : high score   101
        04 06 f3 : attract#1    110
        04 1e f3 : attract#2    110
        04 1e f3 : attract#3    110
        00 4e f3 : attract#4    110


***************************************************************************/

#include "emu.h"
#include "includes/lkage.h"


WRITE8_HANDLER( lkage_videoram_w )
{
	lkage_state *state = (lkage_state *)space->machine->driver_data;

	state->videoram[offset] = data;

	switch (offset / 0x400)
	{
	case 0:
		tilemap_mark_tile_dirty(state->tx_tilemap, offset & 0x3ff);
		break;

	case 1:
		tilemap_mark_tile_dirty(state->fg_tilemap, offset & 0x3ff);
		break;

	case 2:
		tilemap_mark_tile_dirty(state->bg_tilemap, offset & 0x3ff);
		break;

	default:
		break;
	}
}

static TILE_GET_INFO( get_bg_tile_info )
{
	lkage_state *state = (lkage_state *)machine->driver_data;
	int code = state->videoram[tile_index + 0x800] + 256 * (state->bg_tile_bank ? 5 : 1);
	SET_TILE_INFO( 0/*gfx*/, code, 0/*color*/, 0/*flags*/ );
}

static TILE_GET_INFO( get_fg_tile_info )
{
	lkage_state *state = (lkage_state *)machine->driver_data;
	int code = state->videoram[tile_index + 0x400] + 256 * (state->fg_tile_bank ? 1 : 0);
	SET_TILE_INFO( 0/*gfx*/, code, 0/*color*/, 0/*flags*/);
}

static TILE_GET_INFO( get_tx_tile_info )
{
	lkage_state *state = (lkage_state *)machine->driver_data;
	int code = state->videoram[tile_index];
	SET_TILE_INFO( 0/*gfx*/, code, 0/*color*/, 0/*flags*/);
}

VIDEO_START( lkage )
{
	lkage_state *state = (lkage_state *)machine->driver_data;

	state->bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	state->fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	state->tx_tilemap = tilemap_create(machine, get_tx_tile_info, tilemap_scan_rows, 8, 8, 32, 32);

	tilemap_set_transparent_pen(state->fg_tilemap, 0);
	tilemap_set_transparent_pen(state->tx_tilemap, 0);

	tilemap_set_scrolldx(state->bg_tilemap, -5, -5 + 24);
	tilemap_set_scrolldx(state->fg_tilemap, -3, -3 + 24);
	tilemap_set_scrolldx(state->tx_tilemap, -1, -1 + 24);
}


static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	lkage_state *state = (lkage_state *)machine->driver_data;
	const UINT8 *source = state->spriteram;
	const UINT8 *finish = source + 0x60;

	while (source < finish)
	{
		int attributes = source[2];
		/* 0x01: horizontal flip
         * 0x02: vertical flip
         * 0x04: bank select
         * 0x08: sprite size
         * 0x70: color
         * 0x80: priority
         */
		int priority_mask = 0;
		int color = (attributes >> 4) & 7;
		int flipx = attributes & 0x01;
		int flipy = attributes & 0x02;
		int height = (attributes & 0x08) ? 2 : 1;
		int sx = source[0] - 15;
		int sy = 256 - 16 * height - source[1];
		int sprite_number = source[3] + ((attributes & 0x04) << 6);
		int y;

		if (attributes & 0x80)
		{
			priority_mask = (0xf0 | 0xcc);
		}
		else
		{
			priority_mask = 0xf0;
		}

		if (flip_screen_x_get(machine))
		{
			sx = 239 - sx - 24;
			flipx = !flipx;
		}
		if (flip_screen_y_get(machine))
		{
			sy = 254 - 16 * height - sy;
			flipy = !flipy;
		}
		if (height == 2 && !flipy)
		{
			sprite_number ^= 1;
		}

		for (y = 0; y < height; y++)
		{
			pdrawgfx_transpen(
				bitmap,
				cliprect,
				machine->gfx[1],
				sprite_number ^ y,
				color,
				flipx,flipy,
				sx&0xff,
				sy + 16*y,
				machine->priority_bitmap,
				priority_mask,0 );
		}
		source += 4;
	}
}

VIDEO_UPDATE( lkage )
{
	lkage_state *state = (lkage_state *)screen->machine->driver_data;
	int bank;

	flip_screen_x_set(screen->machine, ~state->vreg[2] & 0x01);
	flip_screen_y_set(screen->machine, ~state->vreg[2] & 0x02);

	bank = state->vreg[1] & 0x08;
	if (state->bg_tile_bank != bank)
	{
		state->bg_tile_bank = bank;
		tilemap_mark_all_tiles_dirty(state->bg_tilemap);
	}

	bank = state->vreg[0]&0x04;
	if (state->fg_tile_bank != bank)
	{
		state->fg_tile_bank = bank;
		tilemap_mark_all_tiles_dirty(state->fg_tilemap);
	}

	tilemap_set_palette_offset(state->bg_tilemap, 0x300 + (state->vreg[1] & 0xf0));
	tilemap_set_palette_offset(state->fg_tilemap, 0x200 + (state->vreg[1] & 0xf0));
	tilemap_set_palette_offset(state->tx_tilemap, 0x110);

	tilemap_set_scrollx(state->tx_tilemap, 0, state->scroll[0]);
	tilemap_set_scrolly(state->tx_tilemap, 0, state->scroll[1]);

	tilemap_set_scrollx(state->fg_tilemap, 0, state->scroll[2]);
	tilemap_set_scrolly(state->fg_tilemap, 0, state->scroll[3]);

	tilemap_set_scrollx(state->bg_tilemap, 0, state->scroll[4]);
	tilemap_set_scrolly(state->bg_tilemap, 0, state->scroll[5]);

	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);
	if ((state->vreg[2] & 0xf0) == 0xf0)
	{
		tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 1);
		tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, (state->vreg[1] & 2) ? 2 : 4);
		tilemap_draw(bitmap, cliprect, state->tx_tilemap, 0, 4);
	}
	else
	{
		tilemap_draw(bitmap, cliprect, state->tx_tilemap, TILEMAP_DRAW_OPAQUE, 0);
	}
	draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}
