/***************************************************************************

    Irem M52 hardware

***************************************************************************/

#include "emu.h"
#include "includes/iremz80.h"
#include "video/resnet.h"

#define BGHEIGHT 64


/*************************************
 *
 *  Palette configuration
 *
 *************************************/

PALETTE_INIT( m52 )
{
	const UINT8 *char_pal = color_prom + 0x000;
	const UINT8 *back_pal = color_prom + 0x200;
	const UINT8 *sprite_pal = color_prom + 0x220;
	const UINT8 *sprite_table = color_prom + 0x240;
	static const int resistances_3[3] = { 1000, 470, 220 };
	static const int resistances_2[2]  = { 470, 220 };
	double weights_r[3], weights_g[3], weights_b[3], scale;
	int i;

	machine->colortable = colortable_alloc(machine, 512 + 32 + 32);

	/* compute palette information for characters/backgrounds */
	scale = compute_resistor_weights(0,	255, -1.0,
			3, resistances_3, weights_r, 0, 0,
			3, resistances_3, weights_g, 0, 0,
			2, resistances_2, weights_b, 0, 0);

	/* character palette */
	for (i = 0; i < 512; i++)
	{
		UINT8 promval = char_pal[i];
		int r = combine_3_weights(weights_r, BIT(promval,0), BIT(promval,1), BIT(promval,2));
		int g = combine_3_weights(weights_g, BIT(promval,3), BIT(promval,4), BIT(promval,5));
		int b = combine_2_weights(weights_b, BIT(promval,6), BIT(promval,7));

		colortable_palette_set_color(machine->colortable, i, MAKE_RGB(r,g,b));
	}

	/* background palette */
	for (i = 0; i < 32; i++)
	{
		UINT8 promval = back_pal[i];
		int r = combine_3_weights(weights_r, BIT(promval,0), BIT(promval,1), BIT(promval,2));
		int g = combine_3_weights(weights_g, BIT(promval,3), BIT(promval,4), BIT(promval,5));
		int b = combine_2_weights(weights_b, BIT(promval,6), BIT(promval,7));

		colortable_palette_set_color(machine->colortable, 512+i, MAKE_RGB(r,g,b));
	}

	/* compute palette information for sprites */
	compute_resistor_weights(0,	255, scale,
			2, resistances_2, weights_r, 470, 0,
			3, resistances_3, weights_g, 470, 0,
			3, resistances_3, weights_b, 470, 0);

	/* sprite palette */
	for (i = 0; i < 32; i++)
	{
		UINT8 promval = sprite_pal[i];
		int r = combine_2_weights(weights_r, BIT(promval,6), BIT(promval,7));
		int g = combine_3_weights(weights_g, BIT(promval,3), BIT(promval,4), BIT(promval,5));
		int b = combine_3_weights(weights_b, BIT(promval,0), BIT(promval,1), BIT(promval,2));

		colortable_palette_set_color(machine->colortable, 512 + 32 + i, MAKE_RGB(r,g,b));
	}

	/* character lookup table */
	for (i = 0; i < 512; i++)
		colortable_entry_set_value(machine->colortable, i, i);

	/* sprite lookup table */
	for (i = 0; i < 16 * 4; i++)
	{
		UINT8 promval = sprite_table[(i & 3) | ((i & ~3) << 1)];
		colortable_entry_set_value(machine->colortable, 512 + i, 512 + 32 + promval);
	}

	/* background */
	/* the palette is a 32x8 PROM with many colors repeated. The address of */
	/* the colors to pick is as follows: */
	/* xbb00: mountains */
	/* 0xxbb: hills */
	/* 1xxbb: city */
	colortable_entry_set_value(machine->colortable, 512+16*4+0*4+0, 512);
	colortable_entry_set_value(machine->colortable, 512+16*4+0*4+1, 512+4);
	colortable_entry_set_value(machine->colortable, 512+16*4+0*4+2, 512+8);
	colortable_entry_set_value(machine->colortable, 512+16*4+0*4+3, 512+12);
	colortable_entry_set_value(machine->colortable, 512+16*4+1*4+0, 512);
	colortable_entry_set_value(machine->colortable, 512+16*4+1*4+1, 512+1);
	colortable_entry_set_value(machine->colortable, 512+16*4+1*4+2, 512+2);
	colortable_entry_set_value(machine->colortable, 512+16*4+1*4+3, 512+3);
	colortable_entry_set_value(machine->colortable, 512+16*4+2*4+0, 512);
	colortable_entry_set_value(machine->colortable, 512+16*4+2*4+1, 512+16+1);
	colortable_entry_set_value(machine->colortable, 512+16*4+2*4+2, 512+16+2);
	colortable_entry_set_value(machine->colortable, 512+16*4+2*4+3, 512+16+3);
}



/*************************************
 *
 *  Tilemap info callback
 *
 *************************************/

static TILE_GET_INFO( get_tile_info )
{
	irem_z80_state *state = (irem_z80_state *)machine->driver_data;
	UINT8 video = state->videoram[tile_index];
	UINT8 color = state->colorram[tile_index];

	int flag = 0;
	int code = 0;

	code = video;

	if (color & 0x80)
	{
		code |= 0x100;
	}

	if (tile_index / 32 <= 6)
	{
		flag |= TILE_FORCE_LAYER0; /* lines 0 to 6 are opaqe? */
	}

	SET_TILE_INFO(0, code, color & 0x3f, flag);
}



/*************************************
 *
 *  Video startup
 *
 *************************************/

VIDEO_START( m52 )
{
	irem_z80_state *state = (irem_z80_state *)machine->driver_data;

	state->bg_tilemap = tilemap_create(machine, get_tile_info, tilemap_scan_rows,  8, 8, 32, 32);

	tilemap_set_transparent_pen(state->bg_tilemap, 0);
	tilemap_set_scrolldx(state->bg_tilemap, 128 - 1, -1);
	tilemap_set_scrolldy(state->bg_tilemap, 16, 16);
	tilemap_set_scroll_rows(state->bg_tilemap, 4); /* only lines 192-256 scroll */

	state_save_register_global(machine, state->bg1xpos);
	state_save_register_global(machine, state->bg1ypos);
	state_save_register_global(machine, state->bg2xpos);
	state_save_register_global(machine, state->bg2ypos);
	state_save_register_global(machine, state->bgcontrol);
}



/*************************************
 *
 *  Scroll registers
 *
 *************************************/

WRITE8_HANDLER( m52_scroll_w )
{
	irem_z80_state *state = (irem_z80_state *)space->machine->driver_data;
/*
    According to the schematics there is only one video register that holds the X scroll value
    with a NAND gate on the V64 and V128 lines to control when it's read, and when
    255 (via 8 pull up resistors) is used.

    So we set the first 3 quarters to 255 and the last to the scroll value
*/
	tilemap_set_scrollx(state->bg_tilemap, 0, 255);
	tilemap_set_scrollx(state->bg_tilemap, 1, 255);
	tilemap_set_scrollx(state->bg_tilemap, 2, 255);
	tilemap_set_scrollx(state->bg_tilemap, 3, -data);
}



/*************************************
 *
 *  Video RAM write handlers
 *
 *************************************/

WRITE8_HANDLER( m52_videoram_w )
{
	irem_z80_state *state = (irem_z80_state *)space->machine->driver_data;

	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}


WRITE8_HANDLER( m52_colorram_w )
{
	irem_z80_state *state = (irem_z80_state *)space->machine->driver_data;

	state->colorram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}



/*************************************
 *
 *  Custom protection
 *
 *************************************/

/* This looks like some kind of protection implemented by a custom chip on the
   scroll board. It mangles the value written to the port m52_bg1xpos_w, as
   follows: result = popcount(value & 0x7f) ^ (value >> 7) */
READ8_HANDLER( m52_protection_r )
{
	irem_z80_state *state = (irem_z80_state *)space->machine->driver_data;
	int popcount = 0;
	int temp;

	for (temp = state->bg1xpos & 0x7f; temp != 0; temp >>= 1)
		popcount += temp & 1;
	return popcount ^ (state->bg1xpos >> 7);
}



/*************************************
 *
 *  Background control write handlers
 *
 *************************************/

WRITE8_HANDLER( m52_bg1ypos_w )
{
	irem_z80_state *state = (irem_z80_state *)space->machine->driver_data;
	state->bg1ypos = data;
}

WRITE8_HANDLER( m52_bg1xpos_w )
{
	irem_z80_state *state = (irem_z80_state *)space->machine->driver_data;
	state->bg1xpos = data;
}

WRITE8_HANDLER( m52_bg2xpos_w )
{
	irem_z80_state *state = (irem_z80_state *)space->machine->driver_data;
	state->bg2xpos = data;
}

WRITE8_HANDLER( m52_bg2ypos_w )
{
	irem_z80_state *state = (irem_z80_state *)space->machine->driver_data;
	state->bg2ypos = data;
}

WRITE8_HANDLER( m52_bgcontrol_w )
{
	irem_z80_state *state = (irem_z80_state *)space->machine->driver_data;
	state->bgcontrol = data;
}



/*************************************
 *
 *  Outputs
 *
 *************************************/

WRITE8_HANDLER( m52_flipscreen_w )
{
	/* screen flip is handled both by software and hardware */
	flip_screen_set(space->machine, (data & 0x01) ^ (~input_port_read(space->machine, "DSW2") & 0x01));

	coin_counter_w(space->machine, 0, data & 0x02);
	coin_counter_w(space->machine, 1, data & 0x20);
}

WRITE8_HANDLER( alpha1v_flipscreen_w )
{
	flip_screen_set(space->machine, data & 0x01);
}



/*************************************
 *
 *  Background rendering
 *
 *************************************/

static void draw_background(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int xpos, int ypos, int image)
{
	rectangle rect;
	const rectangle &visarea = machine->primary_screen->visible_area();

	if (flip_screen_get(machine))
	{
		xpos = 255 - xpos;
		ypos = 255 - ypos - BGHEIGHT;
	}

	xpos += 128;

	/* this may not be correct */
	ypos = ypos + (22 - 8);

	drawgfx_transpen(bitmap, cliprect,
		machine->gfx[image],
		0, 0,
		flip_screen_get(machine),
		flip_screen_get(machine),
		xpos,
		ypos, 0);

	drawgfx_transpen(bitmap, cliprect,
		machine->gfx[image],
		0, 0,
		flip_screen_get(machine),
		flip_screen_get(machine),
		xpos - 256,
		ypos, 0);

	rect.min_x = visarea.min_x;
	rect.max_x = visarea.max_x;

	if (flip_screen_get(machine))
	{
		rect.min_y = ypos - BGHEIGHT;
		rect.max_y = ypos - 1;
	}
	else
	{
		rect.min_y = ypos + BGHEIGHT;
		rect.max_y = ypos + 2 * BGHEIGHT - 1;
	}

	bitmap_fill(bitmap, &rect, machine->gfx[image]->color_base + 3);
}



/*************************************
 *
 *  Video render
 *
 *************************************/

VIDEO_UPDATE( m52 )
{
	irem_z80_state *state = (irem_z80_state *)screen->machine->driver_data;
	int offs;

	bitmap_fill(bitmap, cliprect, 0);

	if (!(state->bgcontrol & 0x20))
	{
		if (!(state->bgcontrol & 0x10))
			draw_background(screen->machine, bitmap, cliprect, state->bg2xpos, state->bg2ypos, 2); /* distant mountains */

		if (!(state->bgcontrol & 0x02))
			draw_background(screen->machine, bitmap, cliprect, state->bg1xpos, state->bg1ypos, 3); /* hills */

		if (!(state->bgcontrol & 0x04))
			draw_background(screen->machine, bitmap, cliprect, state->bg1xpos, state->bg1ypos, 4); /* cityscape */
	}

	tilemap_set_flip(state->bg_tilemap, flip_screen_get(screen->machine) ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);

	/* draw the sprites */
	for (offs = 0xfc; offs >= 0; offs -= 4)
	{
		int sy = 257 - state->spriteram[offs];
		int color = state->spriteram[offs + 1] & 0x3f;
		int flipx = state->spriteram[offs + 1] & 0x40;
		int flipy = state->spriteram[offs + 1] & 0x80;
		int code = state->spriteram[offs + 2];
		int sx = state->spriteram[offs + 3];
		rectangle clip;

		/* sprites from offsets $00-$7F are processed in the upper half of the frame */
		/* sprites from offsets $80-$FF are processed in the lower half of the frame */
		clip = *cliprect;
		if (!(offs & 0x80))
			clip.min_y = 0, clip.max_y = 127;
		else
			clip.min_y = 128, clip.max_y = 255;

		/* adjust for flipping */
		if (flip_screen_get(screen->machine))
		{
			int temp = clip.min_y;
			clip.min_y = 255 - clip.max_y;
			clip.max_y = 255 - temp;
			flipx = !flipx;
			flipy = !flipy;
			sx = 240 - sx;
			sy = 257 + 11 - sy;
		}

		sx += 128;

		/* in theory anyways; in practice, some of the molecule-looking guys get clipped */
#ifdef SPLIT_SPRITES
		sect_rect(&clip, cliprect);
#else
		clip = *cliprect;
#endif

		drawgfx_transmask(bitmap, &clip, screen->machine->gfx[1],
			code, color, flipx, flipy, sx, sy,
			colortable_get_transpen_mask(screen->machine->colortable, screen->machine->gfx[1], color, 512 + 32));
	}
	return 0;
}
