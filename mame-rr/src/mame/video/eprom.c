/***************************************************************************

    Atari Escape hardware

****************************************************************************/

#include "emu.h"
#include "includes/eprom.h"
#include "includes/thunderj.h"


/*************************************
 *
 *  Palette
 *
 *************************************/

static void update_palette(running_machine *machine)
{
	eprom_state *state = (eprom_state *)machine->driver_data;
	int color;

	for (color = 0; color < 0x800; ++color)
	{
		int i, r, g, b;
		UINT16 const data = machine->generic.paletteram.u16[color];

		/* FIXME this is only a very crude approximation of the palette output.
         * The circuit involves a dozen transistors and probably has an output
         * which is quite different from this.
         * This is, however, good enough to match the video and description
         * of MAMETesters bug #02677.
         */
		i = (((data >> 12) & 15) + 1) * (4 - state->screen_intensity);
		if (i < 0)
			i = 0;

		r = ((data >> 8) & 15) * i / 4;
		g = ((data >> 4) & 15) * i / 4;
		b = ((data >> 0) & 15) * i / 4;

		palette_set_color_rgb(machine, color, r, g, b);
	}
}



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

static TILE_GET_INFO( get_alpha_tile_info )
{
	eprom_state *state = (eprom_state *)machine->driver_data;
	UINT16 data = state->atarigen.alpha[tile_index];
	int code = data & 0x3ff;
	int color = ((data >> 10) & 0x0f) | ((data >> 9) & 0x20);
	int opaque = data & 0x8000;
	SET_TILE_INFO(1, code, color, opaque ? TILE_FORCE_LAYER0 : 0);
}


static TILE_GET_INFO( get_playfield_tile_info )
{
	eprom_state *state = (eprom_state *)machine->driver_data;
	UINT16 data1 = state->atarigen.playfield[tile_index];
	UINT16 data2 = state->atarigen.playfield_upper[tile_index] >> 8;
	int code = data1 & 0x7fff;
	int color = 0x10 + (data2 & 0x0f);
	SET_TILE_INFO(0, code, color, (data1 >> 15) & 1);
}


static TILE_GET_INFO( guts_get_playfield_tile_info )
{
	eprom_state *state = (eprom_state *)machine->driver_data;
	UINT16 data1 = state->atarigen.playfield[tile_index];
	UINT16 data2 = state->atarigen.playfield_upper[tile_index] >> 8;
	int code = data1 & 0x7fff;
	int color = 0x10 + (data2 & 0x0f);
	SET_TILE_INFO(2, code, color, (data1 >> 15) & 1);
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START( eprom )
{
	static const atarimo_desc modesc =
	{
		0,					/* index to which gfx system */
		1,					/* number of motion object banks */
		1,					/* are the entries linked? */
		0,					/* are the entries split? */
		1,					/* render in reverse order? */
		0,					/* render in swapped X/Y order? */
		0,					/* does the neighbor bit affect the next object? */
		8,					/* pixels per SLIP entry (0 for no-slip) */
		0,					/* pixel offset for SLIPs */
		0,					/* maximum number of links to visit/scanline (0=all) */

		0x100,				/* base palette entry */
		0x100,				/* maximum number of colors */
		0,					/* transparent pen index */

		{{ 0x03ff,0,0,0 }},	/* mask for the link */
		{{ 0 }},			/* mask for the graphics bank */
		{{ 0,0x7fff,0,0 }},	/* mask for the code index */
		{{ 0 }},			/* mask for the upper code index */
		{{ 0,0,0x000f,0 }},	/* mask for the color */
		{{ 0,0,0xff80,0 }},	/* mask for the X position */
		{{ 0,0,0,0xff80 }},	/* mask for the Y position */
		{{ 0,0,0,0x0070 }},	/* mask for the width, in tiles*/
		{{ 0,0,0,0x0007 }},	/* mask for the height, in tiles */
		{{ 0,0,0,0x0008 }},	/* mask for the horizontal flip */
		{{ 0 }},			/* mask for the vertical flip */
		{{ 0,0,0x0070,0 }},	/* mask for the priority */
		{{ 0 }},			/* mask for the neighbor */
		{{ 0 }},			/* mask for absolute coordinates */

		{{ 0 }},			/* mask for the special value */
		0,					/* resulting value to indicate "special" */
		0					/* callback routine for special entries */
	};
	eprom_state *state = (eprom_state *)machine->driver_data;

	/* initialize the playfield */
	state->atarigen.playfield_tilemap = tilemap_create(machine, get_playfield_tile_info, tilemap_scan_cols,  8,8, 64,64);

	/* initialize the motion objects */
	atarimo_init(machine, 0, &modesc);

	/* initialize the alphanumerics */
	state->atarigen.alpha_tilemap = tilemap_create(machine, get_alpha_tile_info, tilemap_scan_rows,  8,8, 64,32);
	tilemap_set_transparent_pen(state->atarigen.alpha_tilemap, 0);

	/* save states */
	state_save_register_global(machine, state->screen_intensity);
	state_save_register_global(machine, state->video_disable);
}


VIDEO_START( guts )
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

		{{ 0x03ff,0,0,0 }},	/* mask for the link */
		{{ 0 }},			/* mask for the graphics bank */
		{{ 0,0x7fff,0,0 }},	/* mask for the code index */
		{{ 0 }},			/* mask for the upper code index */
		{{ 0,0,0x000f,0 }},	/* mask for the color */
		{{ 0,0,0xff80,0 }},	/* mask for the X position */
		{{ 0,0,0,0xff80 }},	/* mask for the Y position */
		{{ 0,0,0,0x0070 }},	/* mask for the width, in tiles*/
		{{ 0,0,0,0x000f }},	/* mask for the height, in tiles */
		{{ 0,0x8000,0,0 }},	/* mask for the horizontal flip */
		{{ 0 }},			/* mask for the vertical flip */
		{{ 0,0,0x0070,0 }},	/* mask for the priority */
		{{ 0 }},			/* mask for the neighbor */
		{{ 0 }},			/* mask for absolute coordinates */

		{{ 0 }},			/* mask for the special value */
		0,					/* resulting value to indicate "special" */
		0					/* callback routine for special entries */
	};
	eprom_state *state = (eprom_state *)machine->driver_data;

	/* initialize the playfield */
	state->atarigen.playfield_tilemap = tilemap_create(machine, guts_get_playfield_tile_info, tilemap_scan_cols,  8,8, 64,64);

	/* initialize the motion objects */
	atarimo_init(machine, 0, &modesc);

	/* initialize the alphanumerics */
	state->atarigen.alpha_tilemap = tilemap_create(machine, get_alpha_tile_info, tilemap_scan_rows,  8,8, 64,32);
	tilemap_set_transparent_pen(state->atarigen.alpha_tilemap, 0);

	/* save states */
	state_save_register_global(machine, state->screen_intensity);
	state_save_register_global(machine, state->video_disable);
}



/*************************************
 *
 *  Periodic scanline updater
 *
 *************************************/

void eprom_scanline_update(screen_device &screen, int scanline)
{
	eprom_state *state = (eprom_state *)screen.machine->driver_data;

	/* update the playfield */
	if (scanline == 0)
	{
		int xscroll = (state->atarigen.alpha[0x780] >> 7) & 0x1ff;
		int yscroll = (state->atarigen.alpha[0x781] >> 7) & 0x1ff;
		tilemap_set_scrollx(state->atarigen.playfield_tilemap, 0, xscroll);
		tilemap_set_scrolly(state->atarigen.playfield_tilemap, 0, yscroll);
		atarimo_set_xscroll(0, xscroll);
		atarimo_set_yscroll(0, yscroll);
	}
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

VIDEO_UPDATE( eprom )
{
	eprom_state *state = (eprom_state *)screen->machine->driver_data;
	atarimo_rect_list rectlist;
	bitmap_t *mobitmap;
	int x, y, r;

	if (state->video_disable)
	{
		bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine));
		return 0;
	}

	update_palette(screen->machine);

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
					/* verified from the GALs on the real PCB; equations follow
                     *
                     *      --- FORCEMC0 forces 3 bits of the MO color to 0 under some conditions
                     *      FORCEMC0=!PFX3*PFX4*PFX5*!MPR0
                     *          +!PFX3*PFX5*!MPR1
                     *          +!PFX3*PFX4*!MPR0*!MPR1
                     *
                     *      --- SHADE selects an alternate color bank for the playfield
                     *      !SHADE=!MPX0
                     *          +MPX1
                     *          +MPX2
                     *          +MPX3
                     *          +!MPX4*!MPX5*!MPX6*!MPX7
                     *          +FORCEMC0
                     *
                     *      --- PF/M is 1 if playfield has priority, or 0 if MOs have priority
                     *      !PF/M=MPR0*MPR1
                     *          +PFX3
                     *          +!PFX4*MPR1
                     *          +!PFX5*MPR1
                     *          +!PFX5*MPR0
                     *          +!PFX4*!PFX5*!MPR0*!MPR1
                     *
                     *      --- M7 is passed as the upper MO bit to the GPC ASIC
                     *      M7=MPX0*!MPX1*!MPX2*!MPX3
                     *
                     *      --- CL10-9 are outputs from the GPC, specifying which layer to render
                     *      CL10 = 1 if pf
                     *      CL9 = 1 if mo
                     *
                     *      --- CRA10 is the 0x200 bit of the color RAM index; it comes directly from the GPC
                     *      CRA10 = CL10
                     *
                     *      --- CRA9 is the 0x100 bit of the color RAM index; is comes directly from the GPC
                     *          or if the SHADE flag is set, it affects the playfield color bank
                     *      CRA9 = SHADE*CL10
                     *          +CL9
                     *
                     *      --- CRA8-1 are the low 8 bits of the color RAM index; set as expected
                     */
					int mopriority = (mo[x] >> ATARIMO_PRIORITY_SHIFT) & 7;
					int pfpriority = (pf[x] >> 4) & 3;
					int forcemc0 = 0, shade = 1, pfm = 1, m7 = 0;

					/* upper bit of MO priority signals special rendering and doesn't draw anything */
					if (mopriority & 4)
						continue;

					/* compute the FORCEMC signal */
					if (!(pf[x] & 8))
					{
						if (((pfpriority == 3) && !(mopriority & 1)) ||
							((pfpriority & 2) && !(mopriority & 2)) ||
							((pfpriority & 1) && (mopriority == 0)))
							forcemc0 = 1;
					}

					/* compute the SHADE signal */
					if (((mo[x] & 0x0f) != 1) ||
						((mo[x] & 0xf0) == 0) ||
						forcemc0)
						shade = 0;

					/* compute the PF/M signal */
					if ((mopriority == 3) ||
						(pf[x] & 8) ||
						(!(pfpriority & 1) && (mopriority & 2)) ||
						(!(pfpriority & 2) && (mopriority & 2)) ||
						(!(pfpriority & 2) && (mopriority & 1)) ||
						((pfpriority == 0) && (mopriority == 0)))
						pfm = 0;

					/* compute the M7 signal */
					if ((mo[x] & 0x0f) == 1)
						m7 = 1;

					/* PF/M and M7 go in the GPC ASIC and select playfield or MO layers */
					if (!pfm && !m7)
					{
						if (!forcemc0)
							pf[x] = mo[x] & ATARIMO_DATA_MASK;
						else
							pf[x] = mo[x] & ATARIMO_DATA_MASK & ~0x70;
					}
					else
					{
						if (shade)
							pf[x] |= 0x100;
						if (m7)
							pf[x] |= 0x080;
					}

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
					}

					/* erase behind ourselves */
					mo[x] = 0;
				}
		}
	return 0;
}


VIDEO_UPDATE( guts )
{
	eprom_state *state = (eprom_state *)screen->machine->driver_data;
	atarimo_rect_list rectlist;
	bitmap_t *mobitmap;
	int x, y, r;

	if (state->video_disable)
	{
		bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine));
		return 0;
	}

	update_palette(screen->machine);

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
					int mopriority = (mo[x] >> ATARIMO_PRIORITY_SHIFT) & 7;
					int pfpriority = (pf[x] >> 5) & 3;

					/* upper bit of MO priority signals special rendering and doesn't draw anything */
					if (mopriority & 4)
						continue;

					/* check the priority */
					if (!(pf[x] & 8) || mopriority >= pfpriority)
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
					}

					/* erase behind ourselves */
					mo[x] = 0;
				}
		}

	return 0;
}
