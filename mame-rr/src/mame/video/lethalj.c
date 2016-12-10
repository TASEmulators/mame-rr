/***************************************************************************

    The Game Room Lethal Justice hardware

***************************************************************************/

#include "emu.h"
#include "cpu/tms34010/tms34010.h"
#include "includes/lethalj.h"


#define BLITTER_SOURCE_WIDTH		1024
#define BLITTER_DEST_WIDTH			512
#define BLITTER_DEST_HEIGHT			512


static UINT16 blitter_data[8];

static UINT16 *screenram;
static UINT8 vispage;
static UINT16 *blitter_base;
static int blitter_rows;

static UINT16 gunx, guny;
static UINT8 blank_palette;




/*************************************
 *
 *  Compute X/Y coordinates
 *
 *************************************/

INLINE void get_crosshair_xy(running_machine *machine, int player, int *x, int *y)
{
	static const char *const gunnames[] = { "LIGHT0_X", "LIGHT0_Y", "LIGHT1_X", "LIGHT1_Y" };
	const rectangle &visarea = machine->primary_screen->visible_area();
	int width = visarea.max_x + 1 - visarea.min_x;
	int height = visarea.max_y + 1 - visarea.min_y;

	*x = ((input_port_read_safe(machine, gunnames[player * 2], 0x00) & 0xff) * width) / 255;
	*y = ((input_port_read_safe(machine, gunnames[1 + player * 2], 0x00) & 0xff) * height) / 255;
}



/*************************************
 *
 *  Gun input handling
 *
 *************************************/

READ16_HANDLER( lethalj_gun_r )
{
	UINT16 result = 0;
	int beamx, beamy;

	switch (offset)
	{
		case 4:
		case 5:
			/* latch the crosshair position */
			get_crosshair_xy(space->machine, offset - 4, &beamx, &beamy);
			gunx = beamx;
			guny = beamy;
			blank_palette = 1;
			break;

		case 6:
			result = gunx / 2;
			break;

		case 7:
			result = guny + 4;
			break;
	}
/*  logerror("%08X:lethalj_gun_r(%d) = %04X\n", cpu_get_pc(space->cpu), offset, result); */
	return result;
}



/*************************************
 *
 *  video startup
 *
 *************************************/

VIDEO_START( lethalj )
{
	/* allocate video RAM for screen */
	screenram = auto_alloc_array(machine, UINT16, BLITTER_DEST_WIDTH * BLITTER_DEST_HEIGHT);

	/* predetermine blitter info */
	blitter_base = (UINT16 *)memory_region(machine, "gfx1");
	blitter_rows = memory_region_length(machine, "gfx1") / (2*BLITTER_SOURCE_WIDTH);
}



/*************************************
 *
 *  Memory maps
 *
 *************************************/

static TIMER_CALLBACK( gen_ext1_int )
{
	cputag_set_input_line(machine, "maincpu", 0, ASSERT_LINE);
}



static void do_blit(void)
{
	int dsty = (INT16)blitter_data[1];
	int srcx = (UINT16)blitter_data[2];
	int srcy = (UINT16)(blitter_data[3] + 1);
	int width = (UINT16)blitter_data[5];
	int dstx = (INT16)blitter_data[6];
	int height = (UINT16)blitter_data[7];
	int y;
/*
    logerror("blitter data = %04X %04X %04X %04X %04X %04X %04X %04X\n",
            blitter_data[0], blitter_data[1], blitter_data[2], blitter_data[3],
            blitter_data[4], blitter_data[5], blitter_data[6], blitter_data[7]);
*/
	/* loop over Y coordinates */
	for (y = 0; y <= height; y++, srcy++, dsty++)
	{
		/* clip in Y */
		if (dsty >= 0 && dsty < BLITTER_DEST_HEIGHT/2)
		{
			UINT16 *source = blitter_base + (srcy % blitter_rows) * BLITTER_SOURCE_WIDTH;
			UINT16 *dest = screenram + (dsty + (vispage ^ 1) * 256) * BLITTER_DEST_WIDTH;
			int sx = srcx;
			int dx = dstx;
			int x;

			/* loop over X coordinates */
			for (x = 0; x <= width; x++, sx++, dx++)
				if (dx >= 0 && dx < BLITTER_DEST_WIDTH)
				{
					int pix = source[sx % BLITTER_SOURCE_WIDTH];
					if (pix)
						dest[dx] = pix;
				}
		}
	}
}


WRITE16_HANDLER( lethalj_blitter_w )
{
	/* combine the data */
	COMBINE_DATA(&blitter_data[offset]);

	/* blit on a write to offset 7, and signal an IRQ */
	if (offset == 7)
	{
		if (blitter_data[6] == 2 && blitter_data[7] == 2)
			vispage ^= 1;
		else
			do_blit();

		timer_set(space->machine, attotime_mul(ATTOTIME_IN_HZ(XTAL_32MHz), (blitter_data[5] + 1) * (blitter_data[7] + 1)), NULL, 0, gen_ext1_int);
	}

	/* clear the IRQ on offset 0 */
	else if (offset == 0)
		cputag_set_input_line(space->machine, "maincpu", 0, CLEAR_LINE);
}



/*************************************
 *
 *  video update
 *
 *************************************/

void lethalj_scanline_update(screen_device &screen, bitmap_t *bitmap, int scanline, const tms34010_display_params *params)
{
	UINT16 *src = &screenram[(vispage << 17) | ((params->rowaddr << 9) & 0x3fe00)];
	UINT16 *dest = BITMAP_ADDR16(bitmap, scanline, 0);
	int coladdr = params->coladdr << 1;
	int x;

	/* blank palette: fill with white */
	if (blank_palette)
	{
		for (x = params->heblnk; x < params->hsblnk; x++)
			dest[x] = 0x7fff;
		if (scanline == screen.visible_area().max_y)
			blank_palette = 0;
		return;
	}

	/* copy the non-blanked portions of this scanline */
	for (x = params->heblnk; x < params->hsblnk; x++)
		dest[x] = src[coladdr++ & 0x1ff] & 0x7fff;
}
