/******************************************************************************

Himeshikibu (C) 1989 Hi-Soft

Video hardware
    driver by Uki

******************************************************************************/

#include "emu.h"
#include "includes/himesiki.h"

static TILE_GET_INFO( get_bg_tile_info )
{
	himesiki_state *state = (himesiki_state *)machine->driver_data;
	int code = state->bg_ram[tile_index * 2] + state->bg_ram[tile_index * 2 + 1] * 0x100 ;
	int col = code >> 12;

	code &= 0xfff;

	SET_TILE_INFO(0, code, col, 0);
}

VIDEO_START( himesiki )
{
	himesiki_state *state = (himesiki_state *)machine->driver_data;
	state->bg_tilemap = tilemap_create( machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 64, 32);
}

WRITE8_HANDLER( himesiki_bg_ram_w )
{
	himesiki_state *state = (himesiki_state *)space->machine->driver_data;
	state->bg_ram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset / 2);
}

WRITE8_HANDLER( himesiki_scrollx_w )
{
	himesiki_state *state = (himesiki_state *)space->machine->driver_data;
	state->scrollx[offset] = data;
}

WRITE8_HANDLER( himesiki_flip_w )
{
	himesiki_state *state = (himesiki_state *)space->machine->driver_data;
	state->flipscreen = data & 0xc0;
	flip_screen_set(space->machine, state->flipscreen);

	if (data & 0x3f)
		logerror("p08_w %02x\n",data);
}

static void himesiki_draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	himesiki_state *state = (himesiki_state *)machine->driver_data;
	UINT8 *spriteram = state->spriteram;
	int offs;

	for (offs = 0x100; offs < 0x160; offs += 4)
	{
		int attr = spriteram[offs + 1];
		int code = spriteram[offs + 0] | (attr & 3) << 8;
		int x = spriteram[offs + 3] | (attr & 8) << 5;
		int y = spriteram[offs + 2];

		int col = (attr & 0xf0) >> 4;
		int fx = attr & 4;
		int fy = 0;

		if (x > 0x1e0)
			x -= 0x200;

		if (state->flipscreen)
		{
			y = (y + 33) & 0xff;
			x = 224 - x;
			fx ^= 4;
			fy = 1;
		}
		else
		{
			y = 257 - y;
			if (y > 0xc0)
				y -= 0x100;
		}

		drawgfx_transpen(bitmap, cliprect, machine->gfx[1], code, col, fx, fy, x, y, 15);
	}

	for (offs = 0; offs < 0x100; offs += 4)
	{
		int attr = spriteram[offs + 1];
		int code = spriteram[offs + 0] | (attr & 7) << 8;
		int x = spriteram[offs + 3] | (attr & 8) << 5;
		int y = spriteram[offs + 2];

		int col = (attr & 0xf0) >> 4;
		int f = 0;

		if (x > 0x1e0)
			x -= 0x200;

		if (state->flipscreen)
		{
			y += 49;
			x = 240 - x;
			f = 1;
		}
		else
			y = 257 - y;

		y &= 0xff;
		if (y > 0xf0)
			y -= 0x100;

		drawgfx_transpen(bitmap, cliprect, machine->gfx[2], code, col, f, f, x, y, 15);
	}
}

VIDEO_UPDATE( himesiki )
{
	himesiki_state *state = (himesiki_state *)screen->machine->driver_data;
	int x = -(state->scrollx[0] << 8 | state->scrollx[1]) & 0x1ff;
	tilemap_set_scrolldx(state->bg_tilemap, x, x);

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, TILEMAP_DRAW_OPAQUE, 0);
	himesiki_draw_sprites(screen->machine, bitmap, cliprect);

	return 0;
}
