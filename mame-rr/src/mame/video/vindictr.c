/***************************************************************************

    Atari Vindicators hardware

****************************************************************************/

#include "emu.h"
#include "includes/vindictr.h"
#include "includes/thunderj.h"



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

static TILE_GET_INFO( get_alpha_tile_info )
{
	vindictr_state *state = (vindictr_state *)machine->driver_data;
	UINT16 data = state->atarigen.alpha[tile_index];
	int code = data & 0x3ff;
	int color = ((data >> 10) & 0x0f) | ((data >> 9) & 0x20);
	int opaque = data & 0x8000;
	SET_TILE_INFO(1, code, color, opaque ? TILE_FORCE_LAYER0 : 0);
}


static TILE_GET_INFO( get_playfield_tile_info )
{
	vindictr_state *state = (vindictr_state *)machine->driver_data;
	UINT16 data = state->atarigen.playfield[tile_index];
	int code = (state->playfield_tile_bank * 0x1000) + (data & 0xfff);
	int color = 0x10 + 2 * ((data >> 12) & 7);
	SET_TILE_INFO(0, code, color, (data >> 15) & 1);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START( vindictr )
{
	static const atarimo_desc modesc =
	{
		0,					/* index to which gfx system */
		1,					/* number of motion object banks */
		1,					/* are the entries linked? */
		0,					/* are the entries split? */
		0,					/* render in reverse order? */
		0,					/* render in swapped X/Y order? */
		0,					/* does the neighbor bit affect the next object? */
		8,					/* pixels per SLIP entry (0 for no-slip) */
		0,					/* pixel offset for SLIPs */
		0,					/* maximum number of links to visit/scanline (0=all) */

		0x100,				/* base palette entry */
		0x100,				/* maximum number of colors */
		0,					/* transparent pen index */

		{{ 0,0,0,0x03ff }},	/* mask for the link */
		{{ 0 }},			/* mask for the graphics bank */
		{{ 0x7fff,0,0,0 }},	/* mask for the code index */
		{{ 0 }},			/* mask for the upper code index */
		{{ 0,0x000f,0,0 }},	/* mask for the color */
		{{ 0,0xff80,0,0 }},	/* mask for the X position */
		{{ 0,0,0xff80,0 }},	/* mask for the Y position */
		{{ 0,0,0x0038,0 }},	/* mask for the width, in tiles*/
		{{ 0,0,0x0007,0 }},	/* mask for the height, in tiles */
		{{ 0,0,0x0040,0 }},	/* mask for the horizontal flip */
		{{ 0 }},			/* mask for the vertical flip */
		{{ 0,0x0070,0,0 }},	/* mask for the priority */
		{{ 0 }},			/* mask for the neighbor */
		{{ 0 }},			/* mask for absolute coordinates */

		{{ 0 }},			/* mask for the special value */
		0,					/* resulting value to indicate "special" */
		NULL				/* callback routine for special entries */
	};
	vindictr_state *state = (vindictr_state *)machine->driver_data;

	/* initialize the playfield */
	state->atarigen.playfield_tilemap = tilemap_create(machine, get_playfield_tile_info, tilemap_scan_cols,  8,8, 64,64);

	/* initialize the motion objects */
	atarimo_init(machine, 0, &modesc);

	/* initialize the alphanumerics */
	state->atarigen.alpha_tilemap = tilemap_create(machine, get_alpha_tile_info, tilemap_scan_rows,  8,8, 64,32);
	tilemap_set_transparent_pen(state->atarigen.alpha_tilemap, 0);

	/* save states */
	state_save_register_global(machine, state->playfield_tile_bank);
	state_save_register_global(machine, state->playfield_xscroll);
	state_save_register_global(machine, state->playfield_yscroll);
}



/*************************************
 *
 *  Palette RAM control
 *
 *************************************/

WRITE16_HANDLER( vindictr_paletteram_w )
{
	static const int ztable[16] =
		{ 0x0, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0x10, 0x11 };
	int c;

	/* first blend the data */
	COMBINE_DATA(&space->machine->generic.paletteram.u16[offset]);
	data = space->machine->generic.paletteram.u16[offset];

	/* now generate colors at all 16 intensities */
	for (c = 0; c < 8; c++)
	{
		int i = ztable[((data >> 12) + (c * 2)) & 15];
		int r = ((data >> 8) & 15) * i;
		int g = ((data >> 4) & 15) * i;
		int b = ((data >> 0) & 15) * i;

		palette_set_color(space->machine,offset + c*2048,MAKE_RGB(r,g,b));
	}
}



/*************************************
 *
 *  Periodic scanline updater
 *
 *************************************/

void vindictr_scanline_update(screen_device &screen, int scanline)
{
	vindictr_state *state = (vindictr_state *)screen.machine->driver_data;
	UINT16 *base = &state->atarigen.alpha[((scanline - 8) / 8) * 64 + 42];
	int x;

	/* keep in range */
	if (base < state->atarigen.alpha)
		base += 0x7c0;
	else if (base >= &state->atarigen.alpha[0x7c0])
		return;

	/* update the current parameters */
	for (x = 42; x < 64; x++)
	{
		UINT16 data = *base++;

		switch ((data >> 9) & 7)
		{
			case 2:		/* /PFB */
				if (state->playfield_tile_bank != (data & 7))
				{
					screen.update_partial(scanline - 1);
					state->playfield_tile_bank = data & 7;
					tilemap_mark_all_tiles_dirty(state->atarigen.playfield_tilemap);
				}
				break;

			case 3:		/* /PFHSLD */
				if (state->playfield_xscroll != (data & 0x1ff))
				{
					screen.update_partial(scanline - 1);
					tilemap_set_scrollx(state->atarigen.playfield_tilemap, 0, data);
					state->playfield_xscroll = data & 0x1ff;
				}
				break;

			case 4:		/* /MOHS */
				if (atarimo_get_xscroll(0) != (data & 0x1ff))
				{
					screen.update_partial(scanline - 1);
					atarimo_set_xscroll(0, data & 0x1ff);
				}
				break;

			case 5:		/* /PFSPC */
				break;

			case 6:		/* /VIRQ */
				atarigen_scanline_int_gen(screen.machine->device("maincpu"));
				break;

			case 7:		/* /PFVS */
			{
				/* a new vscroll latches the offset into a counter; we must adjust for this */
				int offset = scanline;
				const rectangle &visible_area = screen.visible_area();
				if (offset > visible_area.max_y)
					offset -= visible_area.max_y + 1;

				if (state->playfield_yscroll != ((data - offset) & 0x1ff))
				{
					screen.update_partial(scanline - 1);
					tilemap_set_scrolly(state->atarigen.playfield_tilemap, 0, data - offset);
					atarimo_set_yscroll(0, (data - offset) & 0x1ff);
				}
				break;
			}
		}
	}
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

VIDEO_UPDATE( vindictr )
{
	vindictr_state *state = (vindictr_state *)screen->machine->driver_data;
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
					/* partially verified via schematics (there are a lot of PALs involved!):

                        SHADE = PAL(MPR1-0, LB7-0, PFX6-5, PFX3-2, PF/M)

                        if (SHADE)
                            CRA |= 0x100

                        MOG3-1 = ~MAT3-1 if MAT6==1 and MSD3==1
                    */
					int mopriority = mo[x] >> ATARIMO_PRIORITY_SHIFT;

					/* upper bit of MO priority signals special rendering and doesn't draw anything */
					if (mopriority & 4)
						continue;

					/* MO pen 1 doesn't draw, but it sets the SHADE flag and bumps the palette offset */
					if ((mo[x] & 0x0f) == 1)
					{
						if ((mo[x] & 0xf0) != 0)
							pf[x] |= 0x100;
					}
					else
						pf[x] = mo[x] & ATARIMO_DATA_MASK;

					/* don't erase yet -- we need to make another pass later */
				}
		}

	/* add the alpha on top */
	tilemap_draw(bitmap, cliprect, state->atarigen.alpha_tilemap, 0, 0);

	/* now go back and process the upper bit of MO priority */
	rectlist.rect -= rectlist.numrects;
	for (r = 0; r < rectlist.numrects; r++, rectlist.rect++)
		for (y = rectlist.rect->min_y; y <= rectlist.rect->max_y; y++)
		{
			UINT16 *mo = (UINT16 *)mobitmap->base + mobitmap->rowpixels * y;
			UINT16 *pf = (UINT16 *)bitmap->base + bitmap->rowpixels * y;
			for (x = rectlist.rect->min_x; x <= rectlist.rect->max_x; x++)
				if (mo[x])
				{
					int mopriority = mo[x] >> ATARIMO_PRIORITY_SHIFT;

					/* upper bit of MO priority might mean palette kludges */
					if (mopriority & 4)
					{
						/* if bit 2 is set, start setting high palette bits */
						if (mo[x] & 2)
							thunderj_mark_high_palette(bitmap, pf, mo, x, y);

						/* if the upper bit of pen data is set, we adjust the final intensity */
						if (mo[x] & 8)
							pf[x] |= (~mo[x] & 0xe0) << 6;
					}

					/* erase behind ourselves */
					mo[x] = 0;
				}
		}
	return 0;
}
