/******************************************************************************

    Gomoku Narabe Renju
    (c)1981 Nihon Bussan Co.,Ltd.

    Driver by Takahiro Nogi <nogi@kt.rim.or.jp> 1999/11/06 -
    Updated to compile again by David Haywood 19th Oct 2002

******************************************************************************/

#include "emu.h"
#include "includes/gomoku.h"


static int gomoku_flipscreen;
static int gomoku_bg_dispsw;
static tilemap_t *fg_tilemap;
static bitmap_t *gomoku_bg_bitmap;

UINT8 *gomoku_videoram;
UINT8 *gomoku_colorram;
UINT8 *gomoku_bgram;


/******************************************************************************

    palette RAM

******************************************************************************/

PALETTE_INIT( gomoku )
{
	int i;
	int bit0, bit1, bit2, r, g, b;

	for (i = 0; i < machine->total_colors(); i++)
	{
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

		palette_set_color(machine,i, MAKE_RGB(r, g, b));
		color_prom++;
	}
}


/******************************************************************************

    Tilemap callbacks

******************************************************************************/

static TILE_GET_INFO( get_fg_tile_info )
{
	int code = (gomoku_videoram[tile_index]);
	int attr = (gomoku_colorram[tile_index]);
	int color = (attr& 0x0f);
	int flipyx = (attr & 0xc0) >> 6;

	SET_TILE_INFO(
			0,
			code,
			color,
			TILE_FLIPYX(flipyx));
}

WRITE8_HANDLER( gomoku_videoram_w )
{
	gomoku_videoram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap,offset);
}

WRITE8_HANDLER( gomoku_colorram_w )
{
	gomoku_colorram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap,offset);
}

WRITE8_HANDLER( gomoku_bgram_w )
{
	gomoku_bgram[offset] = data;
}

WRITE8_HANDLER( gomoku_flipscreen_w )
{
	gomoku_flipscreen = (data & 0x02) ? 0 : 1;
}

WRITE8_HANDLER( gomoku_bg_dispsw_w )
{
	gomoku_bg_dispsw = (data & 0x02) ? 0 : 1;
}


/******************************************************************************

    Start the video hardware emulation

******************************************************************************/

VIDEO_START( gomoku )
{
	UINT8 *GOMOKU_BG_X = memory_region( machine, "user1" );
	UINT8 *GOMOKU_BG_Y = memory_region( machine, "user2" );
	UINT8 *GOMOKU_BG_D = memory_region( machine, "user3" );
	int x, y;
	int bgdata;
	int color;

	gomoku_bg_bitmap = machine->primary_screen->alloc_compatible_bitmap();

	fg_tilemap = tilemap_create(machine, get_fg_tile_info,tilemap_scan_rows,8,8,32, 32);

	tilemap_set_transparent_pen(fg_tilemap,0);

	/* make background bitmap */
	bitmap_fill(gomoku_bg_bitmap, 0, 0x20);

	// board
	for (y = 0; y < 256; y++)
	{
		for (x = 0; x < 256; x++)
		{
			bgdata = GOMOKU_BG_D[ GOMOKU_BG_X[x] + (GOMOKU_BG_Y[y] << 4) ];

			color = 0x20;						// outside frame (black)

			if (bgdata & 0x01) color = 0x21;	// board (brown)
			if (bgdata & 0x02) color = 0x20;	// frame line (while)

			*BITMAP_ADDR16(gomoku_bg_bitmap, (255 - y - 1) & 0xff, (255 - x + 7) & 0xff) = color;
		}
	}
}


/******************************************************************************

    Display refresh

******************************************************************************/

VIDEO_UPDATE( gomoku )
{
	UINT8 *GOMOKU_BG_X = memory_region( screen->machine, "user1" );
	UINT8 *GOMOKU_BG_Y = memory_region( screen->machine, "user2" );
	UINT8 *GOMOKU_BG_D = memory_region( screen->machine, "user3" );
	int x, y;
	int bgram;
	int bgoffs;
	int bgdata;
	int color;

	/* draw background layer */
	if (gomoku_bg_dispsw)
	{
		/* copy bg bitmap */
		copybitmap(bitmap, gomoku_bg_bitmap, 0, 0, 0, 0, cliprect);

		// stone
		for (y = 0; y < 256; y++)
		{
			for (x = 0; x < 256; x++)
			{
				bgoffs = ((((255 - x - 2) / 14) | (((255 - y - 10) / 14) << 4)) & 0xff);

				bgdata = GOMOKU_BG_D[ GOMOKU_BG_X[x] + (GOMOKU_BG_Y[y] << 4) ];
				bgram = gomoku_bgram[bgoffs];

				if (bgdata & 0x04)
				{
					if (bgram & 0x01)
					{
						color = 0x2f;	// stone (black)
					}
					else if (bgram & 0x02)
					{
						color = 0x22;	// stone (white)
					}
					else continue;
				}
				else continue;

				*BITMAP_ADDR16(bitmap, (255 - y - 1) & 0xff, (255 - x + 7) & 0xff) = color;
			}
		}

		// cursor
		for (y = 0; y < 256; y++)
		{
			for (x = 0; x < 256; x++)
			{
				bgoffs = ((((255 - x - 2) / 14) | (((255 - y - 10) / 14) << 4)) & 0xff);

				bgdata = GOMOKU_BG_D[ GOMOKU_BG_X[x] + (GOMOKU_BG_Y[y] << 4) ];
				bgram = gomoku_bgram[bgoffs];

				if (bgdata & 0x08)
				{
					if (bgram & 0x04)
					{
							color = 0x2f;	// cursor (black)
					}
					else if (bgram & 0x08)
					{
						color = 0x22;		// cursor (white)
					}
					else continue;
				}
				else continue;

				*BITMAP_ADDR16(bitmap, (255 - y - 1) & 0xff, (255 - x + 7) & 0xff) = color;
			}
		}
	}
	else
	{
		bitmap_fill(bitmap, 0, 0x20);
	}

	tilemap_draw(bitmap, cliprect, fg_tilemap, 0, 0);
	return 0;
}
