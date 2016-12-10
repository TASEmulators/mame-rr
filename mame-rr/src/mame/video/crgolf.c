/***************************************************************************

    Kitco Crowns Golf hardware

***************************************************************************/

#include "emu.h"
#include "includes/crgolf.h"


#define NUM_PENS		(0x20)
#define VIDEORAM_SIZE	(0x2000 * 3)


/*************************************
 *
 *  Video RAM access
 *
 *************************************/

WRITE8_HANDLER( crgolf_videoram_w )
{
	crgolf_state *state = (crgolf_state *)space->machine->driver_data;

	if (*state->screen_select & 1)
		state->videoram_b[offset] = data;
	else
		state->videoram_a[offset] = data;
}


READ8_HANDLER( crgolf_videoram_r )
{
	crgolf_state *state = (crgolf_state *)space->machine->driver_data;
	UINT8 ret;

	if (*state->screen_select & 1)
		ret = state->videoram_b[offset];
	else
		ret = state->videoram_a[offset];

	return ret;
}



/*************************************
 *
 *  Palette handling
 *
 *************************************/

static void get_pens( running_machine *machine, pen_t *pens )
{
	offs_t offs;
	const UINT8 *prom = memory_region(machine, "proms");

	for (offs = 0; offs < NUM_PENS; offs++)
	{
		int bit0, bit1, bit2, r, g, b;

		UINT8 data = prom[offs];

		/* red component */
		bit0 = (data >> 0) & 0x01;
		bit1 = (data >> 1) & 0x01;
		bit2 = (data >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* green component */
		bit0 = (data >> 3) & 0x01;
		bit1 = (data >> 4) & 0x01;
		bit2 = (data >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* blue component */
		bit0 = (data >> 6) & 0x01;
		bit1 = (data >> 7) & 0x01;
		b = 0x4f * bit0 + 0xa8 * bit1;

		pens[offs] = MAKE_RGB(r, g, b);
	}
}



/*************************************
 *
 *  Video startup
 *
 *************************************/

static VIDEO_START( crgolf )
{
	crgolf_state *state = (crgolf_state *)machine->driver_data;

	/* allocate memory for the two bitmaps */
	state->videoram_a = auto_alloc_array(machine, UINT8, VIDEORAM_SIZE);
	state->videoram_b = auto_alloc_array(machine, UINT8, VIDEORAM_SIZE);

	/* register for save states */
	state_save_register_global_pointer(machine, state->videoram_a, VIDEORAM_SIZE);
	state_save_register_global_pointer(machine, state->videoram_b, VIDEORAM_SIZE);
}



/*************************************
 *
 *  Video update
 *
 *************************************/

static VIDEO_UPDATE( crgolf )
{
	crgolf_state *state = (crgolf_state *)screen->machine->driver_data;
	int flip = *state->screen_flip & 1;

	offs_t offs;
	pen_t pens[NUM_PENS];

	get_pens(screen->machine, pens);

	/* for each byte in the video RAM */
	for (offs = 0; offs < VIDEORAM_SIZE / 3; offs++)
	{
		int i;

		UINT8 y = (offs & 0x1fe0) >> 5;
		UINT8 x = (offs & 0x001f) << 3;

		UINT8 data_a0 = state->videoram_a[0x2000 | offs];
		UINT8 data_a1 = state->videoram_a[0x0000 | offs];
		UINT8 data_a2 = state->videoram_a[0x4000 | offs];
		UINT8 data_b0 = state->videoram_b[0x2000 | offs];
		UINT8 data_b1 = state->videoram_b[0x0000 | offs];
		UINT8 data_b2 = state->videoram_b[0x4000 | offs];

		if (flip)
		{
			y = ~y;
			x = ~x;
		}

		/* for each pixel in the byte */
		for (i = 0; i < 8; i++)
		{
			offs_t color;
			UINT8 data_b = 0;
			UINT8 data_a = 0;

			if (~*state->screena_enable & 1)
				data_a = ((data_a0 & 0x80) >> 7) | ((data_a1 & 0x80) >> 6) | ((data_a2 & 0x80) >> 5);

			if (~*state->screenb_enable & 1)
				data_b = ((data_b0 & 0x80) >> 7) | ((data_b1 & 0x80) >> 6) | ((data_b2 & 0x80) >> 5);

			/* screen A has priority over B */
			if (data_a)
				color = data_a;
			else
				color = data_b | 0x08;

			/* add HI bit if enabled */
			if (*state->color_select)
				color = color | 0x10;

			*BITMAP_ADDR32(bitmap, y, x) = pens[color];

			/* next pixel */
			data_a0 = data_a0 << 1;
			data_a1 = data_a1 << 1;
			data_a2 = data_a2 << 1;
			data_b0 = data_b0 << 1;
			data_b1 = data_b1 << 1;
			data_b2 = data_b2 << 1;

			if (flip)
				x = x - 1;
			else
				x = x + 1;
		}
	}

	return 0;
}


/*************************************
 *
 *  Machine driver
 *
 *************************************/

MACHINE_DRIVER_START( crgolf_video )

	MDRV_VIDEO_START(crgolf)
	MDRV_VIDEO_UPDATE(crgolf)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 255, 8, 247)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
MACHINE_DRIVER_END
