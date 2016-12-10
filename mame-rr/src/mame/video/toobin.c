/***************************************************************************

    Atari Toobin' hardware

****************************************************************************/

#include "emu.h"
#include "machine/atarigen.h"
#include "includes/toobin.h"



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

static TILE_GET_INFO( get_alpha_tile_info )
{
	toobin_state *state = (toobin_state *)machine->driver_data;
	UINT16 data = state->atarigen.alpha[tile_index];
	int code = data & 0x3ff;
	int color = (data >> 12) & 0x0f;
	SET_TILE_INFO(2, code, color, (data >> 10) & 1);
}


static TILE_GET_INFO( get_playfield_tile_info )
{
	toobin_state *state = (toobin_state *)machine->driver_data;
	UINT16 data1 = state->atarigen.playfield[tile_index * 2];
	UINT16 data2 = state->atarigen.playfield[tile_index * 2 + 1];
	int code = data2 & 0x3fff;
	int color = data1 & 0x0f;
	SET_TILE_INFO(0, code, color, TILE_FLIPYX(data2 >> 14));
	tileinfo->category = (data1 >> 4) & 3;
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START( toobin )
{
	static const atarimo_desc modesc =
	{
		1,					/* index to which gfx system */
		1,					/* number of motion object banks */
		1,					/* are the entries linked? */
		0,					/* are the entries split? */
		0,					/* render in reverse order? */
		1,					/* render in swapped X/Y order? */
		0,					/* does the neighbor bit affect the next object? */
		1024,				/* pixels per SLIP entry (0 for no-slip) */
		0,					/* pixel offset for SLIPs */
		0,					/* maximum number of links to visit/scanline (0=all) */

		0x100,				/* base palette entry */
		0x100,				/* maximum number of colors */
		0,					/* transparent pen index */

		{{ 0,0,0x00ff,0 }},	/* mask for the link */
		{{ 0 }},			/* mask for the graphics bank */
		{{ 0,0x3fff,0,0 }},	/* mask for the code index */
		{{ 0 }},			/* mask for the upper code index */
		{{ 0,0,0,0x000f }},	/* mask for the color */
		{{ 0,0,0,0xffc0 }},	/* mask for the X position */
		{{ 0x7fc0,0,0,0 }},	/* mask for the Y position */
		{{ 0x0007,0,0,0 }},	/* mask for the width, in tiles*/
		{{ 0x0038,0,0,0 }},	/* mask for the height, in tiles */
		{{ 0,0x4000,0,0 }},	/* mask for the horizontal flip */
		{{ 0,0x8000,0,0 }},	/* mask for the vertical flip */
		{{ 0 }},			/* mask for the priority */
		{{ 0 }},			/* mask for the neighbor */
		{{ 0x8000,0,0,0 }},	/* mask for absolute coordinates */

		{{ 0 }},			/* mask for the special value */
		0,					/* resulting value to indicate "special" */
		0					/* callback routine for special entries */
	};
	toobin_state *state = (toobin_state *)machine->driver_data;

	/* initialize the playfield */
	state->atarigen.playfield_tilemap = tilemap_create(machine, get_playfield_tile_info, tilemap_scan_rows,  8,8, 128,64);

	/* initialize the motion objects */
	atarimo_init(machine, 0, &modesc);

	/* initialize the alphanumerics */
	state->atarigen.alpha_tilemap = tilemap_create(machine, get_alpha_tile_info, tilemap_scan_rows,  8,8, 64,48);
	tilemap_set_transparent_pen(state->atarigen.alpha_tilemap, 0);

	/* allocate a playfield bitmap for rendering */
	state->pfbitmap = auto_bitmap_alloc(machine, machine->primary_screen->width(), machine->primary_screen->height(), BITMAP_FORMAT_INDEXED16);

	state_save_register_global(machine, state->brightness);
}



/*************************************
 *
 *  Palette RAM write handler
 *
 *************************************/

WRITE16_HANDLER( toobin_paletteram_w )
{
	toobin_state *state = (toobin_state *)space->machine->driver_data;
	int newword;

	COMBINE_DATA(&space->machine->generic.paletteram.u16[offset]);
	newword = space->machine->generic.paletteram.u16[offset];

	{
		int red =   (((newword >> 10) & 31) * 224) >> 5;
		int green = (((newword >>  5) & 31) * 224) >> 5;
		int blue =  (((newword      ) & 31) * 224) >> 5;

		if (red) red += 38;
		if (green) green += 38;
		if (blue) blue += 38;

		palette_set_color(space->machine, offset & 0x3ff, MAKE_RGB(red, green, blue));
		if (!(newword & 0x8000))
			palette_set_pen_contrast(space->machine, offset & 0x3ff, state->brightness);
		else
			palette_set_pen_contrast(space->machine, offset & 0x3ff, 1.0);
	}
}


WRITE16_HANDLER( toobin_intensity_w )
{
	toobin_state *state = (toobin_state *)space->machine->driver_data;
	int i;

	if (ACCESSING_BITS_0_7)
	{
		state->brightness = (double)(~data & 0x1f) / 31.0;

		for (i = 0; i < 0x400; i++)
			if (!(space->machine->generic.paletteram.u16[i] & 0x8000))
				palette_set_pen_contrast(space->machine, i, state->brightness);
	}
}



/*************************************
 *
 *  X/Y scroll handlers
 *
 *************************************/

WRITE16_HANDLER( toobin_xscroll_w )
{
	toobin_state *state = (toobin_state *)space->machine->driver_data;
	UINT16 oldscroll = *state->atarigen.xscroll;
	UINT16 newscroll = oldscroll;
	COMBINE_DATA(&newscroll);

	/* if anything has changed, force a partial update */
	if (newscroll != oldscroll)
		space->machine->primary_screen->update_partial(space->machine->primary_screen->vpos());

	/* update the playfield scrolling - hscroll is clocked on the following scanline */
	tilemap_set_scrollx(state->atarigen.playfield_tilemap, 0, newscroll >> 6);
	atarimo_set_xscroll(0, newscroll >> 6);

	/* update the data */
	*state->atarigen.xscroll = newscroll;
}


WRITE16_HANDLER( toobin_yscroll_w )
{
	toobin_state *state = (toobin_state *)space->machine->driver_data;
	UINT16 oldscroll = *state->atarigen.yscroll;
	UINT16 newscroll = oldscroll;
	COMBINE_DATA(&newscroll);

	/* if anything has changed, force a partial update */
	if (newscroll != oldscroll)
		space->machine->primary_screen->update_partial(space->machine->primary_screen->vpos());

	/* if bit 4 is zero, the scroll value is clocked in right away */
	tilemap_set_scrolly(state->atarigen.playfield_tilemap, 0, newscroll >> 6);
	atarimo_set_yscroll(0, (newscroll >> 6) & 0x1ff);

	/* update the data */
	*state->atarigen.yscroll = newscroll;
}



/*************************************
 *
 *  X/Y scroll handlers
 *
 *************************************/

WRITE16_HANDLER( toobin_slip_w )
{
	int oldslip = atarimo_0_slipram[offset];
	int newslip = oldslip;
	COMBINE_DATA(&newslip);

	/* if the SLIP is changing, force a partial update first */
	if (oldslip != newslip)
		space->machine->primary_screen->update_partial(space->machine->primary_screen->vpos());

	/* update the data */
	atarimo_0_slipram_w(space, offset, data, mem_mask);
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

VIDEO_UPDATE( toobin )
{
	toobin_state *state = (toobin_state *)screen->machine->driver_data;
	bitmap_t *priority_bitmap = screen->machine->priority_bitmap;
	const rgb_t *palette = palette_entry_list_adjusted(screen->machine->palette);
	atarimo_rect_list rectlist;
	bitmap_t *mobitmap;
	int x, y;

	/* draw the playfield */
	bitmap_fill(priority_bitmap, cliprect, 0);
	tilemap_draw(state->pfbitmap, cliprect, state->atarigen.playfield_tilemap, 0, 0);
	tilemap_draw(state->pfbitmap, cliprect, state->atarigen.playfield_tilemap, 1, 1);
	tilemap_draw(state->pfbitmap, cliprect, state->atarigen.playfield_tilemap, 2, 2);
	tilemap_draw(state->pfbitmap, cliprect, state->atarigen.playfield_tilemap, 3, 3);

	/* draw and merge the MO */
	mobitmap = atarimo_render(0, cliprect, &rectlist);
	for (y = cliprect->min_y; y <= cliprect->max_y; y++)
	{
		UINT32 *dest = BITMAP_ADDR32(bitmap, y, 0);
		UINT16 *mo = BITMAP_ADDR16(mobitmap, y, 0);
		UINT16 *pf = BITMAP_ADDR16(state->pfbitmap, y, 0);
		UINT8 *pri = BITMAP_ADDR8(priority_bitmap, y, 0);
		for (x = cliprect->min_x; x <= cliprect->max_x; x++)
		{
			UINT16 pix = pf[x];
			if (mo[x])
			{
				/* not verified: logic is all controlled in a PAL

                   factors: LBPRI1-0, LBPIX3, ANPIX1-0, PFPIX3, PFPRI1-0,
                            (~LBPIX3 & ~LBPIX2 & ~LBPIX1 & ~LBPIX0)
               */

				/* only draw if not high priority PF */
				if (!pri[x] || !(pix & 8))
					pix = mo[x];

				/* erase behind ourselves */
				mo[x] = 0;
			}
			dest[x] = palette[pix];
		}
	}

	/* add the alpha on top */
	tilemap_draw(bitmap, cliprect, state->atarigen.alpha_tilemap, 0, 0);
	return 0;
}
