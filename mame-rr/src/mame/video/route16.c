/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "sound/sn76477.h"


UINT8 *route16_videoram1;
UINT8 *route16_videoram2;
size_t route16_videoram_size;
//int route16_hardware;

static UINT8 flipscreen;
static UINT8 palette_1;
static UINT8 palette_2;



/*************************************
 *
 *  Memory handlers
 *
 *************************************/

WRITE8_HANDLER( route16_out0_w )
{
	palette_1 = data & 0x1f;

	coin_counter_w(space->machine, 0, (data >> 5) & 0x01);
}


WRITE8_HANDLER( route16_out1_w )
{
	palette_2 = data & 0x1f;

	flipscreen = (data >> 5) & 0x01;
}



/*************************************
 *
 *  Video update
 *
 *************************************/

static pen_t route16_make_pen(UINT8 color)
{
	return MAKE_RGB(pal1bit((color >> 0) & 0x01),
					pal1bit((color >> 1) & 0x01),
					pal1bit((color >> 2) & 0x01));

}


static pen_t ttmajng_make_pen(UINT8 color)
{
	return MAKE_RGB(pal1bit((color >> 2) & 0x01),
					pal1bit((color >> 1) & 0x01),
					pal1bit((color >> 0) & 0x01));

}


/*
 *  Game observation shows that Route 16 can blank each
 *  bitmap by setting bit 1 of the palette register.
 *  Since the schematics are missing the relevant pages, I
 *  cannot confirm how this works, but I am 99% sure the bit 1
 *  would be connected to A7 of the color PROM.  Since the
 *  color PROMs contain 0 in the upper half, this would produce
 *  a black output.
 */

VIDEO_UPDATE( route16 )
{
	offs_t offs;

	UINT8 *color_prom1 = &memory_region(screen->machine, "proms")[0x000];
	UINT8 *color_prom2 = &memory_region(screen->machine, "proms")[0x100];

	for (offs = 0; offs < route16_videoram_size; offs++)
	{
		int i;

		UINT8 y = offs >> 6;
		UINT8 x = offs << 2;

		UINT8 data1 = route16_videoram1[offs];
		UINT8 data2 = route16_videoram2[offs];

		for (i = 0; i < 4; i++)
		{
			UINT8 color1 = color_prom1[((palette_1 << 6) & 0x80) |
									    (palette_1 << 2) |
										((data1 >> 3) & 0x02) |
										((data1 >> 0) & 0x01)];

			/* bit 7 of the 2nd color is the OR of the 1st color bits 0 and 1 - this is a guess */
			UINT8 color2 = color_prom2[((palette_2 << 6) & 0x80) | (((color1 << 6) & 0x80) | ((color1 << 7) & 0x80)) |
										(palette_2 << 2) |
										((data2 >> 3) & 0x02) |
										((data2 >> 0) & 0x01)];

			/* the final color is the OR of the two colors (verified) */
			UINT8 final_color = color1 | color2;

			pen_t pen = route16_make_pen(final_color);

			if (flipscreen)
				*BITMAP_ADDR32(bitmap, 255 - y, 255 - x) = pen;
			else
				*BITMAP_ADDR32(bitmap, y, x) = pen;

			x = x + 1;
			data1 = data1 >> 1;
			data2 = data2 >> 1;
		}
	}

	return 0;
}


/*
 *  The Stratovox video connections have been verified from the schematics
 */

static int video_update_stratvox_ttmahjng(running_machine *machine, bitmap_t *bitmap,
										  const rectangle *cliprect,
										  pen_t (*make_pen)(UINT8))
{
	offs_t offs;

	UINT8 *color_prom1 = &memory_region(machine, "proms")[0x000];
	UINT8 *color_prom2 = &memory_region(machine, "proms")[0x100];

	for (offs = 0; offs < route16_videoram_size; offs++)
	{
		int i;

		UINT8 y = offs >> 6;
		UINT8 x = offs << 2;

		UINT8 data1 = route16_videoram1[offs];
		UINT8 data2 = route16_videoram2[offs];

		for (i = 0; i < 4; i++)
		{
			UINT8 color1 = color_prom1[(palette_1 << 2) |
									   ((data1 >> 3) & 0x02) |
									   ((data1 >> 0) & 0x01)];

			/* bit 7 of the 2nd color is the OR of the 1st color bits 0 and 1 (verified) */
			UINT8 color2 = color_prom2[(((data1 << 3) & 0x80) | ((data1 << 7) & 0x80)) |
									   (palette_2 << 2) |
									   ((data2 >> 3) & 0x02) |
									   ((data2 >> 0) & 0x01)];

			/* the final color is the OR of the two colors */
			UINT8 final_color = color1 | color2;

			pen_t pen = make_pen(final_color);

			if (flipscreen)
				*BITMAP_ADDR32(bitmap, 255 - y, 255 - x) = pen;
			else
				*BITMAP_ADDR32(bitmap, y, x) = pen;

			x = x + 1;
			data1 = data1 >> 1;
			data2 = data2 >> 1;
		}
	}

	return 0;
}


VIDEO_UPDATE( stratvox )
{
	return video_update_stratvox_ttmahjng(screen->machine, bitmap, cliprect, route16_make_pen);
}


VIDEO_UPDATE( ttmahjng )
{
	return video_update_stratvox_ttmahjng(screen->machine, bitmap, cliprect, ttmajng_make_pen);
}
