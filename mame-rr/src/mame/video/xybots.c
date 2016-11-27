/***************************************************************************

    Atari Xybots hardware

****************************************************************************/

#include "emu.h"
#include "machine/atarigen.h"
#include "includes/xybots.h"



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

static TILE_GET_INFO( get_alpha_tile_info )
{
	xybots_state *state = (xybots_state *)machine->driver_data;
	UINT16 data = state->atarigen.alpha[tile_index];
	int code = data & 0x3ff;
	int color = (data >> 12) & 7;
	int opaque = data & 0x8000;
	SET_TILE_INFO(2, code, color, opaque ? TILE_FORCE_LAYER0 : 0);
}


static TILE_GET_INFO( get_playfield_tile_info )
{
	xybots_state *state = (xybots_state *)machine->driver_data;
	UINT16 data = state->atarigen.playfield[tile_index];
	int code = data & 0x1fff;
	int color = (data >> 11) & 0x0f;
	SET_TILE_INFO(0, code, color, (data >> 15) & 1);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START( xybots )
{
	static const atarimo_desc modesc =
	{
		1,					/* index to which gfx system */
		1,					/* number of motion object banks */
		0,					/* are the entries linked? */
		0,					/* are the entries split? */
		0,					/* render in reverse order? */
		0,					/* render in swapped X/Y order? */
		0,					/* does the neighbor bit affect the next object? */
		0,					/* pixels per SLIP entry (0 for no-slip) */
		0,					/* pixel offset for SLIPs */
		0,					/* maximum number of links to visit/scanline (0=all) */

		0x100,				/* base palette entry */
		0x300,				/* maximum number of colors */
		0,					/* transparent pen index */

		{{ 0x3f }},			/* mask for the link (dummy) */
		{{ 0 }},			/* mask for the graphics bank */
		{{ 0x3fff,0,0,0 }},	/* mask for the code index */
		{{ 0 }},			/* mask for the upper code index */
		{{ 0,0,0,0x000f }},	/* mask for the color */
		{{ 0,0,0,0xff80 }},	/* mask for the X position */
		{{ 0,0,0xff80,0 }},	/* mask for the Y position */
		{{ 0 }},			/* mask for the width, in tiles*/
		{{ 0,0,0x0007,0 }},	/* mask for the height, in tiles */
		{{ 0x8000,0,0,0 }},	/* mask for the horizontal flip */
		{{ 0 }},			/* mask for the vertical flip */
		{{ 0,0x000f,0,0 }},	/* mask for the priority */
		{{ 0 }},			/* mask for the neighbor */
		{{ 0 }},			/* mask for absolute coordinates */

		{{ 0 }},			/* mask for the special value */
		0,					/* resulting value to indicate "special" */
		NULL				/* callback routine for special entries */
	};
	xybots_state *state = (xybots_state *)machine->driver_data;

	/* initialize the playfield */
	state->atarigen.playfield_tilemap = tilemap_create(machine, get_playfield_tile_info, tilemap_scan_rows,  8,8, 64,32);

	/* initialize the motion objects */
	atarimo_init(machine, 0, &modesc);

	/* initialize the alphanumerics */
	state->atarigen.alpha_tilemap = tilemap_create(machine, get_alpha_tile_info, tilemap_scan_rows,  8,8, 64,32);
	tilemap_set_transparent_pen(state->atarigen.alpha_tilemap, 0);
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

VIDEO_UPDATE( xybots )
{
	xybots_state *state = (xybots_state *)screen->machine->driver_data;
	atarimo_rect_list rectlist;
	bitmap_t *mobitmap;
	int x, y, r;

	/* draw the playfield */
	tilemap_draw(bitmap, cliprect, state->atarigen.playfield_tilemap, 0, 0);

	/* draw and merge the MO */
	mobitmap = atarimo_render(0, cliprect, &rectlist);
	for (r = 0; r < rectlist.numrects; r++, rectlist.rect++)
		for (y = rectlist.rect->min_y; y <= rectlist.rect->max_y; y++)
		{
			UINT16 *mo = (UINT16 *)mobitmap->base + mobitmap->rowpixels * y;
			UINT16 *pf = (UINT16 *)bitmap->base + bitmap->rowpixels * y;
			for (x = rectlist.rect->min_x; x <= rectlist.rect->max_x; x++)
				if (mo[x])
				{
					/* verified via schematics:

                        PRIEN = ~(~MOPIX3 & ~MOPIX2 & ~MOPIX1) = (MOPIX3-0 > 1)

                        if (PRIEN)
                            PF/MO = (~MOPRI3-0 > PFCOL3-0)
                        else
                            PF/MO = (~MOPRI3-0 >= PFCOL3-0)

                        if (PF/MO | ~(PRIEN & MOCOL3))
                            GPC(P3-0) = PFPIX3-0
                        else
                            GPC(P3-0) = ~MOCOL3-0
                    */
					int mopriority = (mo[x] >> ATARIMO_PRIORITY_SHIFT) ^ 15;
					int pfcolor = (pf[x] >> 4) & 0x0f;
					int prien = ((mo[x] & 0x0f) > 1);

					if (prien)
					{
						if (mopriority <= pfcolor)
						{
							/* this first case doesn't make sense from the schematics, but it has */
							/* the correct effect */
							if (mo[x] & 0x80)
								pf[x] = (mo[x] ^ 0x2f0) & ATARIMO_DATA_MASK;
							else
								pf[x] = mo[x] & ATARIMO_DATA_MASK;
						}
					}
					else
					{
						if (mopriority < pfcolor)
							pf[x] = mo[x] & ATARIMO_DATA_MASK;
					}

					/* erase behind ourselves */
					mo[x] = 0;
				}
		}

	/* add the alpha on top */
	tilemap_draw(bitmap, cliprect, state->atarigen.alpha_tilemap, 0, 0);
	return 0;
}
