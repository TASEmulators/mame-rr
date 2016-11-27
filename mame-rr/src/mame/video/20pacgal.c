/***************************************************************************

    Ms.Pac-Man/Galaga - 20 Year Reunion hardware

    driver by Nicola Salmoria

***************************************************************************/

#include "emu.h"
#include "includes/20pacgal.h"


#define SCREEN_HEIGHT	(224)
#define SCREEN_WIDTH	(288)
#define NUM_PENS		(0x1000)
#define NUM_STAR_PENS	(64)


/*************************************
 *
 *  Palette handling
 *
 *************************************/

static void get_pens(running_machine *machine, const _20pacgal_state *state, pen_t *pens)
{
	offs_t offs;
	UINT8 *color_prom = memory_region(machine, "proms") + (NUM_PENS * state->game_selected);

	for (offs = 0; offs < NUM_PENS ;offs++)
	{
		int bit0,bit1,bit2,r,g,b;

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

		pens[offs] = MAKE_RGB(r, g, b);

		color_prom++;
	}
	/* Star field */

	/* palette for the stars */
	for (offs = 0;offs < 64;offs++)
	{
		int bits,r,g,b;
		static const int map[4] = { 0x00, 0x47, 0x97 ,0xde };

		bits = (offs >> 0) & 0x03;
		r = map[bits];
		bits = (offs >> 2) & 0x03;
		g = map[bits];
		bits = (offs >> 4) & 0x03;
		b = map[bits];

		pens[NUM_PENS + offs] = MAKE_RGB(r, g, b);
	}
}


static void do_pen_lookup(running_machine *machine, const _20pacgal_state *state, bitmap_t *bitmap, const rectangle *cliprect)
{
	int y, x;
	pen_t pens[NUM_PENS + NUM_STAR_PENS];

	get_pens(machine, state, pens);

	for (y = cliprect->min_y; y <= cliprect->max_y; y++)
		for(x = cliprect->min_x; x <= cliprect->max_x; x++)
			*BITMAP_ADDR32(bitmap, y, x) = pens[*BITMAP_ADDR32(bitmap, y, x)];
}



/*************************************
 *
 *  Sprite drawing
 *
 *************************************/

static void draw_sprite(const _20pacgal_state *state, bitmap_t *bitmap, int y, int x,
						UINT8 code, UINT8 color, int flip_y, int flip_x)
{
	int sy;

	offs_t pen_base = (color & 0x3f) << 2;

	if (flip_y)
		y = y + 0x0f;

	if (flip_x)
		x = x + 0x0f;

	/* for each row in the sprite */
	for (sy = 0; sy < 0x10; sy++)
	{
		int x_sav = x;

		if ((y >= 0) && (y < SCREEN_HEIGHT))
		{
			int sx;
			UINT32 data;

			offs_t gfx_offs = ((code & 0x7f) << 6) | (sy << 2);

			/* address mangling */
			gfx_offs = (gfx_offs & 0x1f83) | ((gfx_offs & 0x003c) << 1) | ((gfx_offs & 0x0040) >> 4);

			data = (state->sprite_gfx_ram[gfx_offs + 0] << 24) |
				   (state->sprite_gfx_ram[gfx_offs + 1] << 16) |
				   (state->sprite_gfx_ram[gfx_offs + 2] << 8) |
				   (state->sprite_gfx_ram[gfx_offs + 3] << 0);

			/* for each pixel in the row */
			for (sx = 0; sx < 0x10; sx++)
			{
				if ((x >= 0) && (x < SCREEN_WIDTH))
				{
					offs_t pen = (data & 0xc0000000) >> 30;
					UINT8 col;

					col = state->sprite_color_lookup[pen_base | pen] & 0x0f;

					/* pen bits A0-A3 */
					if (col)
						*BITMAP_ADDR32(bitmap, y, x) = (*BITMAP_ADDR32(bitmap, y, x) & 0xff0) | col;
				}

				/* next pixel */
				if (flip_x)
					x = x - 1;
				else
					x = x + 1;

				data = data << 2;
			}
		}

		/* next row */
		if (flip_y)
			y = y - 1;
		else
			y = y + 1;

		x = x_sav;
	}
}


static void draw_sprites(const _20pacgal_state *state, bitmap_t *bitmap)
{
	int offs;

	for (offs = 0x80 - 2; offs >= 0; offs -= 2)
	{
		static const int code_offs[2][2] =
		{
			{ 0, 1 },
			{ 2, 3 }
		};
		int x, y;

		UINT8 code = state->sprite_ram[offs + 0x000];
		UINT8 color = state->sprite_ram[offs + 0x001];

		int sx = state->sprite_ram[offs + 0x081] - 41 + 0x100*(state->sprite_ram[offs + 0x101] & 3);
		int sy = 256 - state->sprite_ram[offs + 0x080] + 1;

		int flip_x = (state->sprite_ram[offs + 0x100] & 0x01) >> 0;
		int flip_y = (state->sprite_ram[offs + 0x100] & 0x02) >> 1;
		int size_x = (state->sprite_ram[offs + 0x100] & 0x04) >> 2;
		int size_y = (state->sprite_ram[offs + 0x100] & 0x08) >> 3;

		sy = sy - (16 * size_y);
		sy = (sy & 0xff) - 32;	/* fix wraparound */

		/* only Galaga appears to be effected by the global flip state */
		if (state->game_selected && (state->flip[0] & 0x01))
		{
			flip_x = !flip_x;
			flip_y = !flip_y;
		}

		for (y = 0; y <= size_y; y++)
			for (x = 0; x <= size_x; x++)
				draw_sprite(state, bitmap,
							sy + (16 * y), sx + (16 * x),
							code + code_offs[y ^ (size_y * flip_y)][x ^ (size_x * flip_x)],
							color,
							flip_y, flip_x);
	}
}



/*************************************
 *
 *  Character map drawing
 *
 *************************************/

static void draw_chars(const _20pacgal_state *state, bitmap_t *bitmap)
{
	offs_t offs;

	int flip = state->flip[0] & 0x01;

	/* for each byte in the video RAM */
	for (offs = 0; offs < 0x400; offs++)
	{
		int sy;
		int y, x;

		UINT8 *gfx = &state->char_gfx_ram[state->video_ram[0x0000 | offs] << 4];
		UINT32 color_base = (state->video_ram[0x0400 | offs] & 0x3f) << 2;

		/* map the offset to (x, y) character coordinates */
		if ((offs & 0x03c0) == 0)
		{
			y = (offs & 0x1f) - 2;
			x = (offs >> 5) + 34;
		}
		else if ((offs & 0x03c0) == 0x3c0)
		{
			y = (offs & 0x1f) - 2;
			x = (offs >> 5) - 30;
		}
		else
		{
			y = (offs >> 5) - 2;
			x = (offs & 0x1f) + 2;
		}

		if ((y < 0) || (y > 27)) continue;

		/* conver to pixel coordinates */
		y = y << 3;
		x = x << 3;

		if (flip)
		{
			y = SCREEN_HEIGHT - 1 - y;
			x = SCREEN_WIDTH - 1 - x;
		}

		/* for each row in the character */
		for (sy = 0; sy < 8; sy++)
		{
			int sx;
			int x_sav = x;

			UINT16 data = (gfx[8] << 8) | gfx[0];

			/* for each pixel in the row */
			for (sx = 0; sx < 8; sx++)
			{
				UINT32 col = ((data & 0x8000) >> 14) | ((data & 0x0800) >> 11);

				/* pen bits A4-A11 */
				if ( col != 0 )
					*BITMAP_ADDR32(bitmap, y, x) = (color_base | col) << 4;

				/* next pixel */
				if (flip)
					x = x - 1;
				else
					x = x + 1;

				if (sx == 0x03)
					data = data << 5;
				else
					data = data << 1;
			}

			/* next row */
			if (flip)
				y = y - 1;
			else
				y = y + 1;

			x = x_sav;

			gfx = gfx + 1;
		}
	}
}

/*************************************
 *
 *  Draw stars
 *
 *************************************/

/* starfield lfsr code is at d616 and at d648
 *
 * Code at d616:
 *
 * bit 6 (0x40) from 4be4 (port 8A) is rotated into top bit of 586D/586C
 * Based on bit 4 (0x10) and the carry out , bit 6 of 4be4 is set to:
 *
 * Carry out   bit 4    Result
 *    1          1         1
 *    1          0         0
 *    0          1         0
 *    0          0         1
 *
 * Code at d648 is opposite direction.
 *
 * The two videos show, that the starfield is covering the whole screen.
 * Galaga is different, the 256H signal is delivered with 12 pixels
 * delay ( NAND of 1H,2H,4H) to the starfield generator 05xx.
 *
 *    http://www.youtube.com/watch?v=c1zIitLkpGs
 *
 *    http://www.youtube.com/watch?v=bS2Zcin6OwM&feature=PlayList&p=649FD471A1803ABB&playnext_from=PL&playnext=1&index=8
 *
 * Galaga (for comparison)
 *
 *    http://www.youtube.com/watch?v=-R4CCB2g5bE
 *
 * The typical lfsr star count pattern is
 *
 * 132 stars
 * 125 stars
 * 129 stars
 * 68 stars
 * 132 stars
 * 125 stars
 * 129 stars
 * 68 stars
 *
 * This is close to Galaga's 63/126
 *
 */

static void draw_stars(_20pacgal_state *state, bitmap_t *bitmap, const rectangle *cliprect )
{
	if ( (state->stars_ctrl[0] >> 5) & 1 )
	{
		int clock;
		UINT16 lfsr =   state->stars_seed[0] + state->stars_seed[1]*256;
		UINT8 feedback = (state->stars_ctrl[0] >> 6) & 1;
		UINT16 star_seta = (state->stars_ctrl[0] >> 3) & 0x01;
		UINT16 star_setb = (state->stars_ctrl[0] >> 3) & 0x02;
		int cnt = 0;

		/* This is a guess based on galaga star sets */
		star_seta = 0x3fc0 | (star_seta << 14);
		star_setb = 0x3fc0 | (star_setb << 14);


		for (clock=0; clock < 288*224; clock++)
		{
			int x, y;
			int carryout;

			x = clock % 288;
			y = clock / 288;

			/* code at d616 translates into:
             * carryout = lfsr & 1;
             * lfsr = lfsr>>1;
             * lfsr = (feedback << 15) | lfsr;
             * feedback = (((lfsr>>4) & 1) ^ (carryout & 1)) ^ 1;
             *
             * and needs a Hack:
             *  x = 288 - x;
             *
             */

			/* code at d648 */
			carryout = ((lfsr >> 4) ^ feedback ^ 1) & 1;
			feedback = (lfsr >> 15) & 1;
			lfsr = (lfsr << 1) | carryout;

			if (((lfsr & 0xffc0) == star_seta) || ((lfsr & 0xffc0) == star_setb))
			{
				if (y >= cliprect->min_y && y <= cliprect->max_y)
					*BITMAP_ADDR32(bitmap, y, x) = NUM_PENS + (lfsr & 0x3f);
				cnt++;
			}
		}
	}
}


/*************************************
 *
 *  Video update
 *
 *************************************/

static VIDEO_UPDATE( 20pacgal )
{
	_20pacgal_state *state = (_20pacgal_state *)screen->machine->driver_data;

	bitmap_fill(bitmap,cliprect,0);
	draw_stars(state, bitmap,cliprect);
	draw_chars(state, bitmap);
	draw_sprites(state, bitmap);
	do_pen_lookup(screen->machine, state, bitmap, cliprect);

	return 0;
}



/*************************************
 *
 *  Machine driver
 *
 *************************************/

MACHINE_DRIVER_START( 20pacgal_video )

	MDRV_VIDEO_UPDATE(20pacgal)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(SCREEN_WIDTH, SCREEN_HEIGHT)
	MDRV_SCREEN_VISIBLE_AREA(0, SCREEN_WIDTH - 1, 0, SCREEN_HEIGHT - 1)
MACHINE_DRIVER_END
