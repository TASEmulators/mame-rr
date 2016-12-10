/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"

UINT8 *seicross_row_scroll;

UINT8 *seicross_videoram;
UINT8 *seicross_colorram;
static tilemap_t *bg_tilemap;

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Seicross has two 32x8 palette PROMs, connected to the RGB output this way:

  bit 7 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

***************************************************************************/
PALETTE_INIT( seicross )
{
	int i;


	for (i = 0;i < machine->total_colors();i++)
	{
		int bit0,bit1,bit2,r,g,b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = (color_prom[i] >> 6) & 0x01;
		bit1 = (color_prom[i] >> 7) & 0x01;
		b = 0x4f * bit0 + 0xa8 * bit1;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}
}

WRITE8_HANDLER( seicross_videoram_w )
{
	seicross_videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

WRITE8_HANDLER( seicross_colorram_w )
{
	/* bit 5 of the address is not used for color memory. There is just */
	/* 512k of memory; every two consecutive rows share the same memory */
	/* region. */
	offset &= 0xffdf;

	seicross_colorram[offset] = data;
	seicross_colorram[offset + 0x20] = data;

	tilemap_mark_tile_dirty(bg_tilemap, offset);
	tilemap_mark_tile_dirty(bg_tilemap, offset + 0x20);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int code = seicross_videoram[tile_index] + ((seicross_colorram[tile_index] & 0x10) << 4);
	int color = seicross_colorram[tile_index] & 0x0f;
	int flags = ((seicross_colorram[tile_index] & 0x40) ? TILE_FLIPX : 0) | ((seicross_colorram[tile_index] & 0x80) ? TILE_FLIPY : 0);

	SET_TILE_INFO(0, code, color, flags);
}

VIDEO_START( seicross )
{
	bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows,
		 8, 8, 32, 32);

	tilemap_set_scroll_cols(bg_tilemap, 32);
}

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	UINT8 *spriteram = machine->generic.spriteram.u8;
	UINT8 *spriteram_2 = machine->generic.spriteram2.u8;
	int offs;

	for (offs = machine->generic.spriteram_size - 4;offs >= 0;offs -= 4)
	{
		int x = spriteram[offs + 3];
		drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
				(spriteram[offs] & 0x3f) + ((spriteram[offs + 1] & 0x10) << 2) + 128,
				spriteram[offs + 1] & 0x0f,
				spriteram[offs] & 0x40,spriteram[offs] & 0x80,
				x,240-spriteram[offs + 2],0);
		if(x>0xf0)
			drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
					(spriteram[offs] & 0x3f) + ((spriteram[offs + 1] & 0x10) << 2) + 128,
					spriteram[offs + 1] & 0x0f,
					spriteram[offs] & 0x40,spriteram[offs] & 0x80,
					x-256,240-spriteram[offs + 2],0);
	}

	for (offs = machine->generic.spriteram2_size - 4;offs >= 0;offs -= 4)
	{
		int x = spriteram_2[offs + 3];
		drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
				(spriteram_2[offs] & 0x3f) + ((spriteram_2[offs + 1] & 0x10) << 2),
				spriteram_2[offs + 1] & 0x0f,
				spriteram_2[offs] & 0x40,spriteram_2[offs] & 0x80,
				x,240-spriteram_2[offs + 2],0);
		if(x>0xf0)
			drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
					(spriteram_2[offs] & 0x3f) + ((spriteram_2[offs + 1] & 0x10) << 2),
					spriteram_2[offs + 1] & 0x0f,
					spriteram_2[offs] & 0x40,spriteram_2[offs] & 0x80,
					x-256,240-spriteram_2[offs + 2],0);
	}
}

VIDEO_UPDATE( seicross )
{
	int col;

	for (col = 0; col < 32; col++)
		tilemap_set_scrolly(bg_tilemap, col, seicross_row_scroll[col]);

	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}
