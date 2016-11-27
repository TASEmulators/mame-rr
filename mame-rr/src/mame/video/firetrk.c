/***************************************************************************

Atari Fire Truck + Super Bug + Monte Carlo video emulation

***************************************************************************/

#include "emu.h"
#include "includes/firetrk.h"


UINT8 *firetrk_alpha_num_ram;
UINT8 *firetrk_playfield_ram;
UINT8 *firetrk_scroll_x;
UINT8 *firetrk_scroll_y;
UINT8 *firetrk_car_rot;
UINT8 *firetrk_drone_rot;
UINT8 *firetrk_drone_x;
UINT8 *firetrk_drone_y;
UINT8 *firetrk_blink;
UINT8  firetrk_flash;

UINT8 firetrk_skid[2];
UINT8 firetrk_crash[2];

static bitmap_t *helper1;
static bitmap_t *helper2;

static UINT32 color1_mask;
static UINT32 color2_mask;


static const rectangle playfield_window = { 0x02a, 0x115, 0x000, 0x0ff };

static tilemap_t *tilemap1; /* for screen display */
static tilemap_t *tilemap2; /* for collision detection */



PALETTE_INIT( firetrk )
{
	int i;

	static const UINT8 colortable_source[] =
	{
		0, 0, 1, 0,
		2, 0, 3, 0,
		3, 3, 2, 3,
		1, 3, 0, 3,
		0, 0, 1, 0,
		2, 0, 0, 3,
		3, 0, 0, 3
	};
	static const rgb_t palette_source[] =
	{
		RGB_BLACK,
		MAKE_RGB(0x5b, 0x5b, 0x5b),
		MAKE_RGB(0xa4, 0xa4, 0xa4),
		RGB_WHITE
	};

	color1_mask = color2_mask = 0;

	for (i = 0; i < ARRAY_LENGTH(colortable_source); i++)
	{
		UINT8 color = colortable_source[i];

		if (color == 1)
			color1_mask |= 1 << i;
		else if (color == 2)
			color2_mask |= 1 << i;

		palette_set_color(machine, i, palette_source[color]);
	}
}


static void prom_to_palette(running_machine *machine, int number, UINT8 val)
{
	palette_set_color(machine, number, MAKE_RGB(pal1bit(val >> 2), pal1bit(val >> 1), pal1bit(val >> 0)));
}


PALETTE_INIT( montecar )
{
	int i;

	static const UINT8 colortable_source[] =
	{
		0x00, 0x00, 0x00, 0x01,
		0x00, 0x02, 0x00, 0x03,
		0x03, 0x03, 0x03, 0x02,
		0x03, 0x01, 0x03, 0x00,
		0x00, 0x00, 0x02, 0x00,
		0x02, 0x01, 0x02, 0x02,
		0x00, 0x10, 0x20, 0x30,
		0x00, 0x04, 0x08, 0x0c,
		0x00, 0x44, 0x48, 0x4c,
		0x00, 0x84, 0x88, 0x8c,
		0x00, 0xc4, 0xc8, 0xcc
	};

	/*
     * The color PROM is addressed as follows:
     *
     *   A0 => PLAYFIELD 1
     *   A1 => PLAYFIELD 2
     *   A2 => DRONE 1
     *   A3 => DRONE 2
     *   A4 => CAR 1
     *   A5 => CAR 2
     *   A6 => DRONE COLOR 1
     *   A7 => DRONE COLOR 2
     *   A8 => PLAYFIELD WINDOW
     *
     * This driver hard-codes some behavior which actually depends
     * on the PROM, like priorities, clipping and transparency.
     *
     */

	color1_mask = color2_mask = 0;

	for (i = 0; i < ARRAY_LENGTH(colortable_source); i++)
	{
		UINT8 color = colortable_source[i];

		if (color == 1)
			color1_mask |= 1 << i;
		else if (color == 2)
			color2_mask |= 1 << i;

		prom_to_palette(machine, i, color_prom[0x100 + colortable_source[i]]);
	}

	palette_set_color(machine, ARRAY_LENGTH(colortable_source) + 0, RGB_BLACK);
	palette_set_color(machine, ARRAY_LENGTH(colortable_source) + 1, RGB_WHITE);
}


static TILE_GET_INFO( firetrk_get_tile_info1 )
{
	int code = firetrk_playfield_ram[tile_index] & 0x3f;
	int color = (firetrk_playfield_ram[tile_index] >> 6) & 0x03;

	if (*firetrk_blink && (code >= 0x04) && (code <= 0x0b))
		color = 0;

	if (firetrk_flash)
		color = color | 0x04;

	SET_TILE_INFO(1, code, color, 0);
}


static TILE_GET_INFO( superbug_get_tile_info1 )
{
	int code = firetrk_playfield_ram[tile_index] & 0x3f;
	int color = (firetrk_playfield_ram[tile_index] >> 6) & 0x03;

	if (*firetrk_blink && (code >= 0x08) && (code <= 0x0f))
		color = 0;

	if (firetrk_flash)
		color = color | 0x04;

	SET_TILE_INFO(1, code, color, 0);
}


static TILE_GET_INFO( montecar_get_tile_info1 )
{
	int code = firetrk_playfield_ram[tile_index] & 0x3f;
	int color = (firetrk_playfield_ram[tile_index] >> 6) & 0x03;

	if (firetrk_flash)
		color = color | 0x04;

	SET_TILE_INFO(1, code, color, 0);
}


static TILE_GET_INFO( firetrk_get_tile_info2 )
{
	UINT8 code = firetrk_playfield_ram[tile_index] & 0x3f;
	int color = 0;

	/* palette 1 for crash and palette 2 for skid */
	if (((code & 0x30) != 0x00) || ((code & 0x0c) == 0x00))
		color = 1;   /* palette 0, 1 */

	if ((code & 0x3c) == 0x0c)
		color = 2;   /* palette 0, 2 */

	SET_TILE_INFO(2, code, color, 0);
}


static TILE_GET_INFO( superbug_get_tile_info2 )
{
	UINT8 code = firetrk_playfield_ram[tile_index] & 0x3f;
	int color = 0;

	/* palette 1 for crash and palette 2 for skid */
	if ((code & 0x30) != 0x00)
		color = 1;   /* palette 0, 1 */

	if ((code & 0x38) == 0x00)
		color = 2;   /* palette 0, 2 */

	SET_TILE_INFO(2, code, color, 0);
}


static TILE_GET_INFO( montecar_get_tile_info2 )
{
	UINT8 code = firetrk_playfield_ram[tile_index];
	int color = 0;

	/* palette 1 for crash and palette 2 for skid */
	if (((code & 0xc0) == 0x40) || ((code & 0xc0) == 0x80))
		color = 2;   /* palette 2, 1 */

	if ((code & 0xc0) == 0xc0)
		color = 1;   /* palette 2, 0 */

	if ((code & 0xc0) == 0x00)
		color = 3;   /* palette 2, 2 */

	if ((code & 0x30) == 0x30)
		color = 0;   /* palette 0, 0 */

	SET_TILE_INFO(2, code & 0x3f, color, 0);
}


VIDEO_START( firetrk )
{
	helper1 = machine->primary_screen->alloc_compatible_bitmap();
	helper2 = machine->primary_screen->alloc_compatible_bitmap();

	tilemap1 = tilemap_create(machine, firetrk_get_tile_info1, tilemap_scan_rows, 16, 16, 16, 16);
	tilemap2 = tilemap_create(machine, firetrk_get_tile_info2, tilemap_scan_rows, 16, 16, 16, 16);
}


VIDEO_START( superbug )
{
	helper1 = machine->primary_screen->alloc_compatible_bitmap();
	helper2 = machine->primary_screen->alloc_compatible_bitmap();

	tilemap1 = tilemap_create(machine, superbug_get_tile_info1, tilemap_scan_rows, 16, 16, 16, 16);
	tilemap2 = tilemap_create(machine, superbug_get_tile_info2, tilemap_scan_rows, 16, 16, 16, 16);
}


VIDEO_START( montecar )
{
	helper1 = machine->primary_screen->alloc_compatible_bitmap();
	helper2 = machine->primary_screen->alloc_compatible_bitmap();

	tilemap1 = tilemap_create(machine, montecar_get_tile_info1, tilemap_scan_rows, 16, 16, 16, 16);
	tilemap2 = tilemap_create(machine, montecar_get_tile_info2, tilemap_scan_rows, 16, 16, 16, 16);
}


static void firetrk_draw_car(bitmap_t *bitmap, const rectangle *cliprect, gfx_element **gfx, int which, int flash)
{
	int gfx_bank, code, color, flip_x, flip_y, x, y;

	if (which)
	{
		gfx_bank = 5;
		code = *firetrk_drone_rot & 0x07;
		color = flash ? 1 : 0;
		flip_x = *firetrk_drone_rot & 0x08;
		flip_y = *firetrk_drone_rot & 0x10;
		x = (flip_x ? *firetrk_drone_x - 63 : 192 - *firetrk_drone_x) + 36;
		y =  flip_y ? *firetrk_drone_y - 63 : 192 - *firetrk_drone_y;
	}
	else
	{
		gfx_bank = (*firetrk_car_rot & 0x10) ? 4 : 3;
		code = *firetrk_car_rot & 0x03;
		color = flash ? 1 : 0;
		flip_x = *firetrk_car_rot & 0x04;
		flip_y = *firetrk_car_rot & 0x08;
		x = 144;
		y = 104;
	}

	drawgfx_transpen(bitmap, cliprect, gfx[gfx_bank], code, color, flip_x, flip_y, x, y, 0);
}


static void superbug_draw_car(bitmap_t *bitmap, const rectangle *cliprect, gfx_element **gfx, int flash)
{
	int gfx_bank = (*firetrk_car_rot & 0x10) ? 4 : 3;
	int code = ~*firetrk_car_rot & 0x03;
	int color = flash ? 1 : 0;
	int flip_x = *firetrk_car_rot & 0x04;
	int flip_y = *firetrk_car_rot & 0x08;

	drawgfx_transpen(bitmap, cliprect, gfx[gfx_bank], code, color, flip_x, flip_y, 144, 104, 0);
}


static void montecar_draw_car(bitmap_t *bitmap, const rectangle *cliprect, gfx_element **gfx, int which, int is_collision_detection)
{
	int gfx_bank, code, color, flip_x, flip_y, x, y;

	if (which)
	{
		gfx_bank = 4;
		code = *firetrk_drone_rot & 0x07;
		color = is_collision_detection ? 0 : (((*firetrk_car_rot & 0x80) >> 6) | ((*firetrk_drone_rot & 0x80) >> 7));
		flip_x = *firetrk_drone_rot & 0x10;
		flip_y = *firetrk_drone_rot & 0x08;
		x = (flip_x ? *firetrk_drone_x - 31 : 224 - *firetrk_drone_x) + 34;
		y =  flip_y ? *firetrk_drone_y - 31 : 224 - *firetrk_drone_y;
	}
	else
	{
		gfx_bank = 3;
		code = *firetrk_car_rot & 0x07;
		color = 0;
		flip_x = *firetrk_car_rot & 0x10;
		flip_y = *firetrk_car_rot & 0x08;
		x = 144;
		y = 104;
	}

	drawgfx_transpen(bitmap, cliprect, gfx[gfx_bank], code, color, flip_x, flip_y, x, y, 0);
}


static void draw_text(bitmap_t *bitmap, const rectangle *cliprect, gfx_element **gfx, UINT8 *alpha_ram,
					  int x, int count, int height)
{
	int i;

	for (i = 0; i < count; i++)
		drawgfx_opaque(bitmap, cliprect, gfx[0], alpha_ram[i], 0, 0, 0, x, i * height);
}


static void check_collision(int which)
{
	int y, x;

	for (y = playfield_window.min_y; y <= playfield_window.max_y; y++)
		for (x = playfield_window.min_x; x <= playfield_window.max_x; x++)
		{
			pen_t a = *BITMAP_ADDR16(helper1, y, x);
			pen_t b = *BITMAP_ADDR16(helper2, y, x);

			if (b != 0xff && (color1_mask >> a) & 1)
				firetrk_crash[which] = 1;

			if (b != 0xff && (color2_mask >> a) & 1)
				firetrk_skid[which] = 1;
		}
}


VIDEO_UPDATE( firetrk )
{
	tilemap_mark_all_tiles_dirty_all(screen->machine);
	tilemap_set_scrollx(tilemap1, 0, *firetrk_scroll_x - 37);
	tilemap_set_scrollx(tilemap2, 0, *firetrk_scroll_x - 37);
	tilemap_set_scrolly(tilemap1, 0, *firetrk_scroll_y);
	tilemap_set_scrolly(tilemap2, 0, *firetrk_scroll_y);

	bitmap_fill(bitmap, cliprect, 0);
	tilemap_draw(bitmap, &playfield_window, tilemap1, 0, 0);
	firetrk_draw_car(bitmap, &playfield_window, screen->machine->gfx, 0, firetrk_flash);
	firetrk_draw_car(bitmap, &playfield_window, screen->machine->gfx, 1, firetrk_flash);
	draw_text(bitmap, cliprect, screen->machine->gfx, firetrk_alpha_num_ram + 0x00, 296, 0x10, 0x10);
	draw_text(bitmap, cliprect, screen->machine->gfx, firetrk_alpha_num_ram + 0x10,   8, 0x10, 0x10);

	if (cliprect->max_y == screen->visible_area().max_y)
	{
		tilemap_draw(helper1, &playfield_window, tilemap2, 0, 0);

		bitmap_fill(helper2, &playfield_window, 0xff);
		firetrk_draw_car(helper2, &playfield_window, screen->machine->gfx, 0, FALSE);
		check_collision(0);

		bitmap_fill(helper2, &playfield_window, 0xff);
		firetrk_draw_car(helper2, &playfield_window, screen->machine->gfx, 1, FALSE);
		check_collision(1);

		*firetrk_blink = FALSE;
	}

	return 0;
}


VIDEO_UPDATE( superbug )
{
	tilemap_mark_all_tiles_dirty_all(screen->machine);
	tilemap_set_scrollx(tilemap1, 0, *firetrk_scroll_x - 37);
	tilemap_set_scrollx(tilemap2, 0, *firetrk_scroll_x - 37);
	tilemap_set_scrolly(tilemap1, 0, *firetrk_scroll_y);
	tilemap_set_scrolly(tilemap2, 0, *firetrk_scroll_y);

	bitmap_fill(bitmap, cliprect, 0);
	tilemap_draw(bitmap, &playfield_window, tilemap1, 0, 0);
	superbug_draw_car(bitmap, &playfield_window, screen->machine->gfx, firetrk_flash);
	draw_text(bitmap, cliprect, screen->machine->gfx, firetrk_alpha_num_ram + 0x00, 296, 0x10, 0x10);
	draw_text(bitmap, cliprect, screen->machine->gfx, firetrk_alpha_num_ram + 0x10,   8, 0x10, 0x10);

	if (cliprect->max_y == screen->visible_area().max_y)
	{
		tilemap_draw(helper1, &playfield_window, tilemap2, 0, 0);

		bitmap_fill(helper2, &playfield_window, 0xff);
		superbug_draw_car(helper2, &playfield_window, screen->machine->gfx, FALSE);
		check_collision(0);

		*firetrk_blink = FALSE;
	}

	return 0;
}


VIDEO_UPDATE( montecar )
{
	tilemap_mark_all_tiles_dirty_all(screen->machine);
	tilemap_set_scrollx(tilemap1, 0, *firetrk_scroll_x - 37);
	tilemap_set_scrollx(tilemap2, 0, *firetrk_scroll_x - 37);
	tilemap_set_scrolly(tilemap1, 0, *firetrk_scroll_y);
	tilemap_set_scrolly(tilemap2, 0, *firetrk_scroll_y);

	bitmap_fill(bitmap, cliprect, 0x2c);
	tilemap_draw(bitmap, &playfield_window, tilemap1, 0, 0);
	montecar_draw_car(bitmap, &playfield_window, screen->machine->gfx, 0, FALSE);
	montecar_draw_car(bitmap, &playfield_window, screen->machine->gfx, 1, FALSE);
	draw_text(bitmap, cliprect, screen->machine->gfx, firetrk_alpha_num_ram + 0x00, 24, 0x20, 0x08);
	draw_text(bitmap, cliprect, screen->machine->gfx, firetrk_alpha_num_ram + 0x20, 16, 0x20, 0x08);

	if (cliprect->max_y == screen->visible_area().max_y)
	{
		tilemap_draw(helper1, &playfield_window, tilemap2, 0, 0);

		bitmap_fill(helper2, &playfield_window, 0xff);
		montecar_draw_car(helper2, &playfield_window, screen->machine->gfx, 0, TRUE);
		check_collision(0);

		bitmap_fill(helper2, &playfield_window, 0xff);
		montecar_draw_car(helper2, &playfield_window, screen->machine->gfx, 1, TRUE);
		check_collision(1);
	}

	return 0;
}
