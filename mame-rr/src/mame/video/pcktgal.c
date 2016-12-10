#include "emu.h"

static tilemap_t *bg_tilemap;

PALETTE_INIT( pcktgal )
{
	int i;

	for (i = 0;i < machine->total_colors();i++)
	{
		int bit0,bit1,bit2,bit3,r,g,b;

		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		bit3 = (color_prom[i] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[i] >> 4) & 0x01;
		bit1 = (color_prom[i] >> 5) & 0x01;
		bit2 = (color_prom[i] >> 6) & 0x01;
		bit3 = (color_prom[i] >> 7) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[i + machine->total_colors()] >> 0) & 0x01;
		bit1 = (color_prom[i + machine->total_colors()] >> 1) & 0x01;
		bit2 = (color_prom[i + machine->total_colors()] >> 2) & 0x01;
		bit3 = (color_prom[i + machine->total_colors()] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}
}

WRITE8_HANDLER( pcktgal_videoram_w )
{
	space->machine->generic.videoram.u8[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset / 2);
}

WRITE8_HANDLER( pcktgal_flipscreen_w )
{
	if (flip_screen_get(space->machine) != (data & 0x80))
	{
		flip_screen_set(space->machine, data & 0x80);
		tilemap_mark_all_tiles_dirty_all(space->machine);
	}
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int code = machine->generic.videoram.u8[tile_index*2+1] + ((machine->generic.videoram.u8[tile_index*2] & 0x0f) << 8);
	int color = machine->generic.videoram.u8[tile_index*2] >> 4;

	SET_TILE_INFO(0, code, color, 0);
}

VIDEO_START( pcktgal )
{
	bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows,
		 8, 8, 32, 32);
}

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	UINT8 *spriteram = machine->generic.spriteram.u8;
	int offs;

	for (offs = 0;offs < machine->generic.spriteram_size;offs += 4)
	{
		if (spriteram[offs] != 0xf8)
		{
			int sx,sy,flipx,flipy;


			sx = 240 - spriteram[offs+2];
			sy = 240 - spriteram[offs];

			flipx = spriteram[offs+1] & 0x04;
			flipy = spriteram[offs+1] & 0x02;
			if (flip_screen_get(machine)) {
				sx=240-sx;
				sy=240-sy;
				if (flipx) flipx=0; else flipx=1;
				if (flipy) flipy=0; else flipy=1;
			}

			drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
					spriteram[offs+3] + ((spriteram[offs+1] & 1) << 8),
					(spriteram[offs+1] & 0x70) >> 4,
					flipx,flipy,
					sx,sy,0);
		}
	}
}

VIDEO_UPDATE( pcktgal )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}
