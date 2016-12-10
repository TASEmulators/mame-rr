/***************************************************************************

    Mustache Boy
    (c)1987 March Electronics

***************************************************************************/


#include "emu.h"

static tilemap_t *bg_tilemap;
static int control_byte;

PALETTE_INIT(mustache)
{
  int i;

  for (i = 0;i < 256;i++)
  {
	int bit0,bit1,bit2,bit3,r,g,b;

	/* red component */
	bit0 = (color_prom[i] >> 0) & 0x01;
	bit1 = (color_prom[i] >> 1) & 0x01;
	bit2 = (color_prom[i] >> 2) & 0x01;
	bit3 = (color_prom[i] >> 3) & 0x01;
	r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

	/* green component */
	bit0 = (color_prom[i + 256] >> 0) & 0x01;
	bit1 = (color_prom[i + 256] >> 1) & 0x01;
	bit2 = (color_prom[i + 256] >> 2) & 0x01;
	bit3 = (color_prom[i + 256] >> 3) & 0x01;
	g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

	/* blue component */
	bit0 = (color_prom[i + 512] >> 0) & 0x01;
	bit1 = (color_prom[i + 512] >> 1) & 0x01;
	bit2 = (color_prom[i + 512] >> 2) & 0x01;
	bit3 = (color_prom[i + 512] >> 3) & 0x01;
	b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

	palette_set_color(machine,i,MAKE_RGB(r,g,b));
  }
}

WRITE8_HANDLER( mustache_videoram_w )
{
	space->machine->generic.videoram.u8[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset / 2);
}

WRITE8_HANDLER (mustache_video_control_w)
{
	if (flip_screen_get(space->machine) != (data & 0x01))
	{
		flip_screen_set(space->machine, data & 0x01);
		tilemap_mark_all_tiles_dirty_all(space->machine);
	}

	/* tile bank */

	if ((control_byte ^ data) & 0x08)
	{
		control_byte = data;
		tilemap_mark_all_tiles_dirty_all(space->machine);
	}
}

WRITE8_HANDLER( mustache_scroll_w )
{
	tilemap_set_scrollx(bg_tilemap, 0, 0x100 - data);
	tilemap_set_scrollx(bg_tilemap, 1, 0x100 - data);
	tilemap_set_scrollx(bg_tilemap, 2, 0x100 - data);
	tilemap_set_scrollx(bg_tilemap, 3, 0x100);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int attr = machine->generic.videoram.u8[2 * tile_index + 1];
	int code = machine->generic.videoram.u8[2 * tile_index] + ((attr & 0x60) << 3) + ((control_byte & 0x08) << 7);
	int color = attr & 0x0f;

	SET_TILE_INFO(0, code, color, ((attr & 0x10) ? TILE_FLIPX : 0) | ((attr & 0x80) ? TILE_FLIPY : 0)   );


}

VIDEO_START( mustache )
{
	bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows_flip_x,
		 8, 8, 64, 32);

	tilemap_set_scroll_rows(bg_tilemap, 4);
}

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	rectangle clip = *cliprect;
	const gfx_element *gfx = machine->gfx[1];
	const rectangle &visarea = machine->primary_screen->visible_area();
	UINT8 *spriteram = machine->generic.spriteram.u8;
	int offs;

	for (offs = 0;offs < machine->generic.spriteram_size;offs += 4)
	{
		int sy = 240-spriteram[offs];
		int sx = 240-spriteram[offs+3];
		int code = spriteram[offs+2];
		int attr = spriteram[offs+1];
		int color = (attr & 0xe0)>>5;

		if (sy == 240) continue;

		code+=(attr&0x0c)<<6;

		if ((control_byte & 0xa))
			clip.max_y = visarea.max_y;
		else
			if (flip_screen_get(machine))
				clip.min_y = visarea.min_y + 56;
			else
				clip.max_y = visarea.max_y - 56;

		if (flip_screen_get(machine))
		{
			sx = 240 - sx;
			sy = 240 - sy;
		}

		drawgfx_transpen(bitmap,&clip,gfx,
				code,
				color,
				flip_screen_get(machine),flip_screen_get(machine),
				sx,sy,0);
	}
}

VIDEO_UPDATE( mustache )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}
