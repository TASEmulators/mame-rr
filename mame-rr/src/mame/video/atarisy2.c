/***************************************************************************

    Atari System 2 hardware

****************************************************************************/

#include "emu.h"
#include "includes/slapstic.h"
#include "includes/atarisy2.h"



/*************************************
 *
 *  Prototypes
 *
 *************************************/

static TIMER_CALLBACK( reset_yscroll_callback );



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

static TILE_GET_INFO( get_alpha_tile_info )
{
	atarisy2_state *state = (atarisy2_state *)machine->driver_data;
	UINT16 data = state->atarigen.alpha[tile_index];
	int code = data & 0x3ff;
	int color = (data >> 13) & 0x07;
	SET_TILE_INFO(2, code, color, 0);
}


static TILE_GET_INFO( get_playfield_tile_info )
{
	atarisy2_state *state = (atarisy2_state *)machine->driver_data;
	UINT16 data = state->atarigen.playfield[tile_index];
	int code = state->playfield_tile_bank[(data >> 10) & 1] + (data & 0x3ff);
	int color = (data >> 11) & 7;
	SET_TILE_INFO(0, code, color, 0);
	tileinfo->category = (~data >> 14) & 3;
}



/*************************************
 *
 *  Video system start
 *
 *************************************/

VIDEO_START( atarisy2 )
{
	static const atarimo_desc modesc =
	{
		1,					/* index to which gfx system */
		1,					/* number of motion object banks */
		1,					/* are the entries linked? */
		0,					/* are the entries split? */
		0,					/* render in reverse order? */
		0,					/* render in swapped X/Y order? */
		0,					/* does the neighbor bit affect the next object? */
		0,					/* pixels per SLIP entry (0 for no-slip) */
		0,					/* pixel offset for SLIPs */
		0,					/* maximum number of links to visit/scanline (0=all) */

		0x00,				/* base palette entry */
		0x40,				/* maximum number of colors */
		15,					/* transparent pen index */

		{{ 0,0,0,0x07f8 }},	/* mask for the link */
		{{ 0 }},			/* mask for the graphics bank */
		{{ 0,0x07ff,0,0 }},	/* mask for the code index */
		{{ 0x0007,0,0,0 }},	/* mask for the upper code index */
		{{ 0,0,0,0x3000 }},	/* mask for the color */
		{{ 0,0,0xffc0,0 }},	/* mask for the X position */
		{{ 0x7fc0,0,0,0 }},	/* mask for the Y position */
		{{ 0 }},			/* mask for the width, in tiles*/
		{{ 0,0x3800,0,0 }},	/* mask for the height, in tiles */
		{{ 0,0x4000,0,0 }},	/* mask for the horizontal flip */
		{{ 0 }},			/* mask for the vertical flip */
		{{ 0,0,0,0xc000 }},	/* mask for the priority */
		{{ 0,0x8000,0,0 }},	/* mask for the neighbor */
		{{ 0 }},			/* mask for absolute coordinates */

		{{ 0 }},			/* mask for the special value */
		0,					/* resulting value to indicate "special" */
		0					/* callback routine for special entries */
	};
	atarisy2_state *state = (atarisy2_state *)machine->driver_data;

	/* initialize banked memory */
	state->atarigen.alpha = &state->vram[0x0000];
	atarimo_0_spriteram = &state->vram[0x0c00];
	state->atarigen.playfield = &state->vram[0x2000];

	/* initialize the playfield */
	state->atarigen.playfield_tilemap = tilemap_create(machine, get_playfield_tile_info, tilemap_scan_rows,  8,8, 128,64);

	/* initialize the motion objects */
	atarimo_init(machine, 0, &modesc);

	/* initialize the alphanumerics */
	state->atarigen.alpha_tilemap = tilemap_create(machine, get_alpha_tile_info, tilemap_scan_rows,  8,8, 64,48);
	tilemap_set_transparent_pen(state->atarigen.alpha_tilemap, 0);

	/* reset the statics */
	state->yscroll_reset_timer = timer_alloc(machine, reset_yscroll_callback, NULL);
	state->videobank = 0;

	/* save states */
	state_save_register_global_array(machine, state->playfield_tile_bank);
	state_save_register_global(machine, state->videobank);
	state_save_register_global_array(machine, state->vram);
}



/*************************************
 *
 *  Scroll/playfield bank write
 *
 *************************************/

WRITE16_HANDLER( atarisy2_xscroll_w )
{
	atarisy2_state *state = (atarisy2_state *)space->machine->driver_data;
	UINT16 oldscroll = *state->atarigen.xscroll;
	UINT16 newscroll = oldscroll;
	COMBINE_DATA(&newscroll);

	/* if anything has changed, force a partial update */
	if (newscroll != oldscroll)
		space->machine->primary_screen->update_partial(space->machine->primary_screen->vpos());

	/* update the playfield scrolling - hscroll is clocked on the following scanline */
	tilemap_set_scrollx(state->atarigen.playfield_tilemap, 0, newscroll >> 6);

	/* update the playfield banking */
	if (state->playfield_tile_bank[0] != (newscroll & 0x0f) * 0x400)
	{
		state->playfield_tile_bank[0] = (newscroll & 0x0f) * 0x400;
		tilemap_mark_all_tiles_dirty(state->atarigen.playfield_tilemap);
	}

	/* update the data */
	*state->atarigen.xscroll = newscroll;
}


static TIMER_CALLBACK( reset_yscroll_callback )
{
	atarisy2_state *state = (atarisy2_state *)machine->driver_data;
	tilemap_set_scrolly(state->atarigen.playfield_tilemap, 0, param);
}


WRITE16_HANDLER( atarisy2_yscroll_w )
{
	atarisy2_state *state = (atarisy2_state *)space->machine->driver_data;
	UINT16 oldscroll = *state->atarigen.yscroll;
	UINT16 newscroll = oldscroll;
	COMBINE_DATA(&newscroll);

	/* if anything has changed, force a partial update */
	if (newscroll != oldscroll)
		space->machine->primary_screen->update_partial(space->machine->primary_screen->vpos());

	/* if bit 4 is zero, the scroll value is clocked in right away */
	if (!(newscroll & 0x10))
		tilemap_set_scrolly(state->atarigen.playfield_tilemap, 0, (newscroll >> 6) - space->machine->primary_screen->vpos());
	else
		timer_adjust_oneshot(state->yscroll_reset_timer, space->machine->primary_screen->time_until_pos(0), newscroll >> 6);

	/* update the playfield banking */
	if (state->playfield_tile_bank[1] != (newscroll & 0x0f) * 0x400)
	{
		state->playfield_tile_bank[1] = (newscroll & 0x0f) * 0x400;
		tilemap_mark_all_tiles_dirty(state->atarigen.playfield_tilemap);
	}

	/* update the data */
	*state->atarigen.yscroll = newscroll;
}



/*************************************
 *
 *  Palette RAM write handler
 *
 *************************************/

WRITE16_HANDLER( atarisy2_paletteram_w )
{
	static const int intensity_table[16] =
	{
		#define ZB 115
		#define Z3 78
		#define Z2 37
		#define Z1 17
		#define Z0 9
		0, ZB+Z0, ZB+Z1, ZB+Z1+Z0, ZB+Z2, ZB+Z2+Z0, ZB+Z2+Z1, ZB+Z2+Z1+Z0,
		ZB+Z3, ZB+Z3+Z0, ZB+Z3+Z1, ZB+Z3+Z1+Z0,ZB+ Z3+Z2, ZB+Z3+Z2+Z0, ZB+Z3+Z2+Z1, ZB+Z3+Z2+Z1+Z0
	};
	static const int color_table[16] =
		{ 0x0, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xe, 0xf, 0xf };

	int newword, inten, red, green, blue;

	COMBINE_DATA(&space->machine->generic.paletteram.u16[offset]);
	newword = space->machine->generic.paletteram.u16[offset];

	inten = intensity_table[newword & 15];
	red = (color_table[(newword >> 12) & 15] * inten) >> 4;
	green = (color_table[(newword >> 8) & 15] * inten) >> 4;
	blue = (color_table[(newword >> 4) & 15] * inten) >> 4;
	palette_set_color(space->machine, offset, MAKE_RGB(red, green, blue));
}



/*************************************
 *
 *  Video RAM bank read/write handlers
 *
 *************************************/

READ16_HANDLER( atarisy2_slapstic_r )
{
	atarisy2_state *state = (atarisy2_state *)space->machine->driver_data;
	int result = state->slapstic_base[offset];
	slapstic_tweak(space, offset);

	/* an extra tweak for the next opcode fetch */
	state->videobank = slapstic_tweak(space, 0x1234) * 0x1000;
	return result;
}


WRITE16_HANDLER( atarisy2_slapstic_w )
{
	atarisy2_state *state = (atarisy2_state *)space->machine->driver_data;

	slapstic_tweak(space, offset);

	/* an extra tweak for the next opcode fetch */
	state->videobank = slapstic_tweak(space, 0x1234) * 0x1000;
}



/*************************************
 *
 *  Video RAM read/write handlers
 *
 *************************************/

READ16_HANDLER( atarisy2_videoram_r )
{
	atarisy2_state *state = (atarisy2_state *)space->machine->driver_data;
	return state->vram[offset | state->videobank];
}


WRITE16_HANDLER( atarisy2_videoram_w )
{
	atarisy2_state *state = (atarisy2_state *)space->machine->driver_data;
	int offs = offset | state->videobank;

	/* alpharam? */
	if (offs < 0x0c00)
	{
		COMBINE_DATA(&state->atarigen.alpha[offs]);
		tilemap_mark_tile_dirty(state->atarigen.alpha_tilemap, offs);
	}

	/* spriteram? */
	else if (offs < 0x1000)
	{
		/* force an update if the link of object 0 is about to change */
		if (offs == 0x0c03)
			space->machine->primary_screen->update_partial(space->machine->primary_screen->vpos());
		atarimo_0_spriteram_w(space, offs - 0x0c00, data, mem_mask);
	}

	/* playfieldram? */
	else if (offs >= 0x2000)
	{
		offs -= 0x2000;
		COMBINE_DATA(&state->atarigen.playfield[offs]);
		tilemap_mark_tile_dirty(state->atarigen.playfield_tilemap, offs);
	}

	/* generic case */
	else
	{
		COMBINE_DATA(&state->vram[offs]);
	}
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

VIDEO_UPDATE( atarisy2 )
{
	atarisy2_state *state = (atarisy2_state *)screen->machine->driver_data;
	bitmap_t *priority_bitmap = screen->machine->priority_bitmap;
	atarimo_rect_list rectlist;
	bitmap_t *mobitmap;
	int x, y, r;

	/* draw the playfield */
	bitmap_fill(priority_bitmap, cliprect, 0);
	tilemap_draw(bitmap, cliprect, state->atarigen.playfield_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, state->atarigen.playfield_tilemap, 1, 1);
	tilemap_draw(bitmap, cliprect, state->atarigen.playfield_tilemap, 2, 2);
	tilemap_draw(bitmap, cliprect, state->atarigen.playfield_tilemap, 3, 3);

	/* draw and merge the MO */
	mobitmap = atarimo_render(0, cliprect, &rectlist);
	for (r = 0; r < rectlist.numrects; r++, rectlist.rect++)
		for (y = rectlist.rect->min_y; y <= rectlist.rect->max_y; y++)
		{
			UINT16 *mo = (UINT16 *)mobitmap->base + mobitmap->rowpixels * y;
			UINT16 *pf = (UINT16 *)bitmap->base + bitmap->rowpixels * y;
			UINT8 *pri = (UINT8 *)priority_bitmap->base + priority_bitmap->rowpixels * y;
			for (x = rectlist.rect->min_x; x <= rectlist.rect->max_x; x++)
				if (mo[x] != 0x0f)
				{
					int mopriority = mo[x] >> ATARIMO_PRIORITY_SHIFT;

					/* high priority PF? */
					if ((mopriority + pri[x]) & 2)
					{
						/* only gets priority if PF pen is less than 8 */
						if (!(pf[x] & 0x08))
							pf[x] = mo[x] & ATARIMO_DATA_MASK;
					}

					/* low priority */
					else
						pf[x] = mo[x] & ATARIMO_DATA_MASK;

					/* erase behind ourselves */
					mo[x] = 0x0f;
				}
		}

	/* add the alpha on top */
	tilemap_draw(bitmap, cliprect, state->atarigen.alpha_tilemap, 0, 0);
	return 0;
}
