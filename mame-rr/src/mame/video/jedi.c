/***************************************************************************

    Atari Return of the Jedi hardware

    driver by Dan Boris

    Return of the Jedi has a peculiar playfield/motion object
    priority system. That is, there is no priority system ;-)
    The color of the pixel which appears on screen depends on
    all three of the foreground, background and motion objects.
    The 1024 colors palette is appropriately set up by the program
    to "emulate" a priority system, but it can also be used to display
    completely different colors (see the palette test in service mode)

***************************************************************************/

#include "emu.h"
#include "includes/jedi.h"


#define NUM_PENS	(0x1000)



/*************************************
 *
 *  Start
 *
 *************************************/

static VIDEO_START( jedi )
{
	jedi_state *state = (jedi_state *)machine->driver_data;

	/* register for saving */
	state_save_register_global(machine, state->vscroll);
	state_save_register_global(machine, state->hscroll);
}



/*************************************
 *
 *  Palette RAM
 *
 *************************************
 *
 *  Color RAM format
 *  Color RAM is 1024x12
 *
 *  RAM address: A0..A3 = Playfield color code
 *      A4..A7 = Motion object color code
 *      A8..A9 = Alphanumeric color code
 *
 *  RAM data:
 *      0..2 = Blue
 *      3..5 = Green
 *      6..8 = Blue
 *      9..11 = Intensity
 *
 *  Output resistor values:
 *      bit 0 = 22K
 *      bit 1 = 10K
 *      bit 2 = 4.7K
 *
 *************************************/

static void get_pens(jedi_state *state, pen_t *pens)
{
	offs_t offs;

	for (offs = 0; offs < NUM_PENS; offs++)
	{
		int r, g, b, bits, intensity;

		UINT16 color = state->paletteram[offs] | (state->paletteram[offs | 0x400] << 8);

		intensity = (color >> 9) & 7;
		bits = (color >> 6) & 7;
		r = 5 * bits * intensity;
		bits = (color >> 3) & 7;
		g = 5 * bits * intensity;
		bits = (color >> 0) & 7;
		b = 5 * bits * intensity;

		pens[offs] = MAKE_RGB(r, g, b);
	}
}


static void do_pen_lookup(jedi_state *state, bitmap_t *bitmap, const rectangle *cliprect)
{
	int y, x;
	pen_t pens[NUM_PENS];

	get_pens(state, pens);

	for (y = cliprect->min_y; y <= cliprect->max_y; y++)
		for(x = cliprect->min_x; x <= cliprect->max_x; x++)
			*BITMAP_ADDR32(bitmap, y, x) = pens[*BITMAP_ADDR32(bitmap, y, x)];
}



/*************************************
 *
 *  Scroll offsets
 *
 *************************************/

WRITE8_HANDLER( jedi_vscroll_w )
{
	jedi_state *state = (jedi_state *)space->machine->driver_data;

	state->vscroll = data | (offset << 8);
}


WRITE8_HANDLER( jedi_hscroll_w )
{
	jedi_state *state = (jedi_state *)space->machine->driver_data;

	state->hscroll = data | (offset << 8);
}



/*************************************
 *
 *  Background/text layer drawing
 *  with smoothing
 *
 *************************************/

static void draw_background_and_text(running_machine *machine, jedi_state *state, bitmap_t *bitmap, const rectangle *cliprect)
{
	int y;
	int background_line_buffer[0x200];	/* RAM chip at 2A */

	UINT8 *tx_gfx = memory_region(machine, "gfx1");
	UINT8 *bg_gfx = memory_region(machine, "gfx2");
	UINT8 *prom1 = &memory_region(machine, "proms")[0x0000 | ((*state->smoothing_table & 0x03) << 8)];
	UINT8 *prom2 = &memory_region(machine, "proms")[0x0800 | ((*state->smoothing_table & 0x03) << 8)];
	int vscroll = state->vscroll;
	int hscroll = state->hscroll;
	int tx_bank = *state->foreground_bank;
	UINT8 *tx_ram = state->foregroundram;
	UINT8 *bg_ram = state->backgroundram;

	memset(background_line_buffer, 0, 0x200 * sizeof(int));

	for (y = cliprect->min_y; y <= cliprect->max_y; y++)
	{
		int x;
		int bg_last_col = 0;

		for (x = cliprect->min_x; x <= cliprect->max_x; x += 2)
		{
			int tx_col1, tx_col2, bg_col;
			int bg_tempcol;
			offs_t tx_gfx_offs, bg_gfx_offs;
			int tx_data, bg_data1, bg_data2;

			int sy = y + vscroll;
			int sx = x + hscroll;

			/* determine offsets into video memory */
			offs_t tx_offs = ((y & 0xf8) << 3) | (x >> 3);
			offs_t bg_offs = ((sy & 0x1f0) << 1) | ((sx & 0x1f0) >> 4);

			/* get the character codes */
			int tx_code = ((tx_bank & 0x80) << 1) | tx_ram[tx_offs];
			int bg_bank = bg_ram[0x0400 | bg_offs];
			int bg_code = bg_ram[0x0000 | bg_offs] |
						  ((bg_bank & 0x01) << 8) |
						  ((bg_bank & 0x08) << 6) |
						  ((bg_bank & 0x02) << 9);

			/* background flip X */
			if (bg_bank & 0x04)
				sx = sx ^ 0x0f;

			/* calculate the address of the gfx data */
			tx_gfx_offs = (tx_code << 4) | ((y & 0x07) << 1) | ((( x & 0x04) >> 2));
			bg_gfx_offs = (bg_code << 4) | (sy & 0x0e)       | (((sx & 0x08) >> 3));

			/* get the gfx data */
			tx_data  = tx_gfx[         tx_gfx_offs];
			bg_data1 = bg_gfx[0x0000 | bg_gfx_offs];
			bg_data2 = bg_gfx[0x8000 | bg_gfx_offs];

			/* the text layer pixel determines pen address bits A8 and A9 */
			if (x & 0x02)
			{
				tx_col1 = ((tx_data  & 0x0c) << 6);
				tx_col2 = ((tx_data  & 0x03) << 8);
			}
			else
			{
				tx_col1 = ((tx_data  & 0xc0) << 2);
				tx_col2 = ((tx_data  & 0x30) << 4);
			}

			/* the background pixel determines pen address bits A0-A3 */
			switch (sx & 0x06)
			{
			case 0x00: bg_col = ((bg_data1 & 0x80) >> 4) | ((bg_data1 & 0x08) >> 1) | ((bg_data2 & 0x80) >> 6) | ((bg_data2 & 0x08) >> 3); break;
			case 0x02: bg_col = ((bg_data1 & 0x40) >> 3) | ((bg_data1 & 0x04) >> 0) | ((bg_data2 & 0x40) >> 5) | ((bg_data2 & 0x04) >> 2); break;
			case 0x04: bg_col = ((bg_data1 & 0x20) >> 2) | ((bg_data1 & 0x02) << 1) | ((bg_data2 & 0x20) >> 4) | ((bg_data2 & 0x02) >> 1); break;
			default:   bg_col = ((bg_data1 & 0x10) >> 1) | ((bg_data1 & 0x01) << 2) | ((bg_data2 & 0x10) >> 3) | ((bg_data2 & 0x01) >> 0); break;
			}

			/* the first pixel is smoothed via a lookup using the current and last pixel value -
               the next pixel just uses the current value directly. After we done with a pixel
               save it for later in the line buffer RAM */
			bg_tempcol = prom1[(bg_last_col << 4) | bg_col];
			*BITMAP_ADDR32(bitmap, y, x + 0) = tx_col1 | prom2[(background_line_buffer[x + 0] << 4) | bg_tempcol];
			*BITMAP_ADDR32(bitmap, y, x + 1) = tx_col2 | prom2[(background_line_buffer[x + 1] << 4) | bg_col];
			background_line_buffer[x + 0] = bg_tempcol;
			background_line_buffer[x + 1] = bg_col;

			bg_last_col = bg_col;
		}
	}
}



/*************************************
 *
 *  Sprite drawing
 *
 *************************************/

static void draw_sprites(running_machine *machine, jedi_state *state, bitmap_t *bitmap, const rectangle *cliprect)
{
	offs_t offs;
	UINT8 *spriteram = state->spriteram;
	UINT8 *gfx3 = memory_region(machine, "gfx3");

	for (offs = 0x00; offs < 0x30; offs++)
	{
		int sy;
		int y_size;
		UINT8 *gfx;

		/* coordinates adjustments made to match screenshot */
		UINT8 y = 240 - spriteram[offs + 0x80] + 1;
		int flip_x = spriteram[offs + 0x40] & 0x10;
		int flip_y = spriteram[offs + 0x40] & 0x20;
		int tall = spriteram[offs + 0x40] & 0x08;

		/* shuffle the bank bits in */
		UINT16 code = spriteram[offs] |
					  ((spriteram[offs + 0x40] & 0x04) << 8) |
					  ((spriteram[offs + 0x40] & 0x40) << 3) |
					  ((spriteram[offs + 0x40] & 0x02) << 7);

		/* adjust for double-height */
		if (tall)
		{
			code &= ~1;
			y_size = 0x20;
			y = y - 0x10;
		}
		else
			y_size = 0x10;

		gfx = &gfx3[code << 5];

		if (flip_y)
			y = y + y_size - 1;

		for (sy = 0; sy < y_size; sy++)
		{
			int i;
			UINT16 x = spriteram[offs + 0x100] + ((spriteram[offs + 0x40] & 0x01) << 8) - 2;

			if ((y < cliprect->min_y) || (y > cliprect->max_y))
				continue;

			if (flip_x)
				x = x + 7;

			for (i = 0; i < 2; i++)
			{
				int sx;
				UINT8 data1 = *(0x00000 + gfx);
				UINT8 data2 = *(0x10000 + gfx);

				for (sx = 0; sx < 4; sx++)
				{
					/* the sprite pixel determines pen address bits A4-A7 */
					UINT32 col = ((data1 & 0x80) >> 0) | ((data1 & 0x08) << 3) | ((data2 & 0x80) >> 2) | ((data2 & 0x08) << 1);

					x = x & 0x1ff;

					if (col)
						*BITMAP_ADDR32(bitmap, y, x) = (*BITMAP_ADDR32(bitmap, y, x) & 0x30f) | col;

					/* next pixel */
					if (flip_x)
						x = x - 1;
					else
						x = x + 1;

					data1 = data1 << 1;
					data2 = data2 << 1;
				}

				gfx = gfx + 1;
			}

			if (flip_y)
				y = y - 1;
			else
				y = y + 1;
		}
	}
}



/*************************************
 *
 *  Core video refresh
 *
 *************************************/

static VIDEO_UPDATE( jedi )
{
	jedi_state *state = (jedi_state *)screen->machine->driver_data;

	/* if no video, clear it all to black */
	if (*state->video_off & 0x01)
		bitmap_fill(bitmap, cliprect, RGB_BLACK);
	else
	{
		/* draw the background/text layers, followed by the sprites
           - it needs to be done in this order*/
		draw_background_and_text(screen->machine, state, bitmap, cliprect);
		draw_sprites(screen->machine, state, bitmap, cliprect);
		do_pen_lookup(state, bitmap, cliprect);
	}

	return 0;
}



/*************************************
 *
 *  Machine driver
 *
 *************************************/

MACHINE_DRIVER_START( jedi_video )


	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_SIZE(64*8, 262) /* verify vert size */
	MDRV_SCREEN_VISIBLE_AREA(0*8, 37*8-1, 0*8, 30*8-1)

	MDRV_VIDEO_START(jedi)
	MDRV_VIDEO_UPDATE(jedi)
MACHINE_DRIVER_END
