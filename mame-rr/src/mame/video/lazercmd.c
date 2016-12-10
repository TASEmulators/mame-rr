/*************************************************************/
/*                                                           */
/* Lazer Command video handler                               */
/*                                                           */
/*************************************************************/

#include "emu.h"
#include "includes/lazercmd.h"

/* scale a markers vertical position */
/* the following table shows how the markers */
/* vertical position worked in hardware  */
/*  marker_y  lines    marker_y  lines   */
/*     0      0 + 1       8      10 + 11 */
/*     1      2 + 3       9      12 + 13 */
/*     2      4 + 5      10      14 + 15 */
/*     3      6 + 7      11      16 + 17 */
/*     4      8 + 9      12      18 + 19 */
static int vert_scale(int data)
{
	return ((data & 0x07) << 1) + ((data & 0xf8) >> 3) * VERT_CHR;
}

/* plot a bitmap marker */
/* hardware has 2 marker sizes 2x2 and 4x2 selected by jumper */
/* meadows lanes normaly use 2x2 pixels and lazer command uses either */
static void plot_pattern( running_machine *machine, bitmap_t *bitmap, int x, int y )
{
	int xbit, ybit, size;

	size = 2;
	if (input_port_read(machine, "DSW") & 0x40)
	{
		size = 4;
	}

	for (ybit = 0; ybit < 2; ybit++)
	{
	    if (y + ybit < 0 || y + ybit >= VERT_RES * VERT_CHR)
		    return;

	    for (xbit = 0; xbit < size; xbit++)
		{
			if (x + xbit < 0 || x + xbit >= HORZ_RES * HORZ_CHR)
				continue;

			*BITMAP_ADDR16(bitmap, y + ybit, x + xbit) = 4;
		}
	}
}


VIDEO_UPDATE( lazercmd )
{
	lazercmd_state *state = (lazercmd_state *)screen->machine->driver_data;
	int i, x, y;

	int video_inverted = input_port_read(screen->machine, "DSW") & 0x20;

	/* The first row of characters are invisible */
	for (i = 0; i < (VERT_RES - 1) * HORZ_RES; i++)
	{
		int sx, sy;

		sx = i % HORZ_RES;
		sy = i / HORZ_RES;

		sx *= HORZ_CHR;
		sy *= VERT_CHR;

		drawgfx_opaque(bitmap, cliprect,screen->machine->gfx[0],
				state->videoram[i], video_inverted ? 1 : 0,
				0,0,
				sx,sy);
	}

	x = state->marker_x - 1;             /* normal video lags marker by 1 pixel */
	y = vert_scale(state->marker_y) - VERT_CHR; /* first line used as scratch pad */
	plot_pattern(screen->machine, bitmap, x, y);

	return 0;
}
