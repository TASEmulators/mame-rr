/******************************************************************************

    Video Hardware for Video System Mahjong series and Pipe Dream.

    Driver by Takahiro Nogi <nogi@kt.rim.or.jp> 2001/02/04 -
    and Bryan McPhail, Nicola Salmoria, Aaron Giles

******************************************************************************/

#include "emu.h"
#include "includes/fromance.h"


static TIMER_CALLBACK( crtc_interrupt_gen );

/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

INLINE void get_fromance_tile_info( running_machine *machine, tile_data *tileinfo, int tile_index, int layer )
{
	fromance_state *state = (fromance_state *)machine->driver_data;
	int tile = ((state->local_videoram[layer][0x0000 + tile_index] & 0x80) << 9) |
				(state->local_videoram[layer][0x1000 + tile_index] << 8) |
				state->local_videoram[layer][0x2000 + tile_index];
	int color = state->local_videoram[layer][tile_index] & 0x7f;

	SET_TILE_INFO(layer, tile, color, 0);
}

static TILE_GET_INFO( get_fromance_bg_tile_info ) { get_fromance_tile_info(machine, tileinfo, tile_index, 0); }
static TILE_GET_INFO( get_fromance_fg_tile_info ) { get_fromance_tile_info(machine, tileinfo, tile_index, 1); }


INLINE void get_nekkyoku_tile_info( running_machine *machine, tile_data *tileinfo, int tile_index, int layer )
{
	fromance_state *state = (fromance_state *)machine->driver_data;
	int tile = (state->local_videoram[layer][0x0000 + tile_index] << 8) |
				state->local_videoram[layer][0x1000 + tile_index];
	int color = state->local_videoram[layer][tile_index + 0x2000] & 0x3f;

	SET_TILE_INFO(layer, tile, color, 0);
}

static TILE_GET_INFO( get_nekkyoku_bg_tile_info ) { get_nekkyoku_tile_info(machine, tileinfo, tile_index, 0); }
static TILE_GET_INFO( get_nekkyoku_fg_tile_info ) { get_nekkyoku_tile_info(machine, tileinfo, tile_index, 1); }



/*************************************
 *
 *  Video system start
 *
 *************************************/

static void init_common( running_machine *machine )
{
	fromance_state *state = (fromance_state *)machine->driver_data;

	/* allocate local videoram */
	state->local_videoram[0] = auto_alloc_array(machine, UINT8, 0x1000 * 3);
	state->local_videoram[1] = auto_alloc_array(machine, UINT8, 0x1000 * 3);

	/* allocate local palette RAM */
	state->local_paletteram = auto_alloc_array(machine, UINT8, 0x800 * 2);

	/* configure tilemaps */
	tilemap_set_transparent_pen(state->fg_tilemap, 15);

	/* reset the timer */
	state->crtc_timer = timer_alloc(machine, crtc_interrupt_gen, NULL);

	/* state save */
	state_save_register_global(machine, state->selected_videoram);
	state_save_register_global_pointer(machine, state->local_videoram[0], 0x1000 * 3);
	state_save_register_global_pointer(machine, state->local_videoram[1], 0x1000 * 3);
	state_save_register_global(machine, state->selected_paletteram);
	state_save_register_global_array(machine, state->scrollx);
	state_save_register_global_array(machine, state->scrolly);
	state_save_register_global(machine, state->gfxreg);
	state_save_register_global(machine, state->flipscreen);
	state_save_register_global(machine, state->flipscreen_old);
	state_save_register_global(machine, state->scrollx_ofs);
	state_save_register_global(machine, state->scrolly_ofs);
	state_save_register_global(machine, state->crtc_register);
	state_save_register_global_array(machine, state->crtc_data);
	state_save_register_global_pointer(machine, state->local_paletteram, 0x800 * 2);
}

VIDEO_START( fromance )
{
	fromance_state *state = (fromance_state *)machine->driver_data;

	/* allocate tilemaps */
	state->bg_tilemap = tilemap_create(machine, get_fromance_bg_tile_info, tilemap_scan_rows, 8, 4, 64, 64);
	state->fg_tilemap = tilemap_create(machine, get_fromance_fg_tile_info, tilemap_scan_rows, 8, 4, 64, 64);

	init_common(machine);
}

VIDEO_START( nekkyoku )
{
	fromance_state *state = (fromance_state *)machine->driver_data;

	/* allocate tilemaps */
	state->bg_tilemap = tilemap_create(machine, get_nekkyoku_bg_tile_info, tilemap_scan_rows, 8, 4, 64, 64);
	state->fg_tilemap = tilemap_create(machine, get_nekkyoku_fg_tile_info, tilemap_scan_rows, 8, 4, 64, 64);

	init_common(machine);
}

VIDEO_START( pipedrm )
{
	fromance_state *state = (fromance_state *)machine->driver_data;

	VIDEO_START_CALL(fromance);
	state->scrolly_ofs = 0x00;
}

VIDEO_START( hatris )
{
	fromance_state *state = (fromance_state *)machine->driver_data;

	VIDEO_START_CALL(fromance);
	state->scrollx_ofs = 0xB9;
	state->scrolly_ofs = 0x00;
}

/*************************************
 *
 *  Graphics control register
 *
 *************************************/

WRITE8_HANDLER( fromance_gfxreg_w )
{
	fromance_state *state = (fromance_state *)space->machine->driver_data;

	state->gfxreg = data;
	state->flipscreen = (data & 0x01);
	state->selected_videoram = (~data >> 1) & 1;
	state->selected_paletteram = (data >> 6) & 1;

	if (state->flipscreen != state->flipscreen_old)
	{
		state->flipscreen_old = state->flipscreen;
		tilemap_set_flip_all(space->machine, state->flipscreen ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0);
	}
}



/*************************************
 *
 *  Banked palette RAM
 *
 *************************************/

READ8_HANDLER( fromance_paletteram_r )
{
	fromance_state *state = (fromance_state *)space->machine->driver_data;

	/* adjust for banking and read */
	offset |= state->selected_paletteram << 11;
	return state->local_paletteram[offset];
}


WRITE8_HANDLER( fromance_paletteram_w )
{
	fromance_state *state = (fromance_state *)space->machine->driver_data;
	int palword;

	/* adjust for banking and modify */
	offset |= state->selected_paletteram << 11;
	state->local_paletteram[offset] = data;

	/* compute R,G,B */
	palword = (state->local_paletteram[offset | 1] << 8) | state->local_paletteram[offset & ~1];
	palette_set_color_rgb(space->machine, offset / 2, pal5bit(palword >> 10), pal5bit(palword >> 5), pal5bit(palword >> 0));
}



/*************************************
 *
 *  Video RAM read/write
 *
 *************************************/

READ8_HANDLER( fromance_videoram_r )
{
	fromance_state *state = (fromance_state *)space->machine->driver_data;
	return state->local_videoram[state->selected_videoram][offset];
}


WRITE8_HANDLER( fromance_videoram_w )
{
	fromance_state *state = (fromance_state *)space->machine->driver_data;
	state->local_videoram[state->selected_videoram][offset] = data;
	tilemap_mark_tile_dirty(state->selected_videoram ? state->fg_tilemap : state->bg_tilemap, offset & 0x0fff);
}



/*************************************
 *
 *  Scroll registers
 *
 *************************************/

WRITE8_HANDLER( fromance_scroll_w )
{
	fromance_state *state = (fromance_state *)space->machine->driver_data;
	if (state->flipscreen)
	{
		switch (offset)
		{
			case 0:
				state->scrollx[1] = (data + (((state->gfxreg & 0x08) >> 3) * 0x100) - state->scrollx_ofs);
				break;
			case 1:
				state->scrolly[1] = (data + (((state->gfxreg & 0x04) >> 2) * 0x100) - state->scrolly_ofs); // - 0x10
				break;
			case 2:
				state->scrollx[0] = (data + (((state->gfxreg & 0x20) >> 5) * 0x100) - state->scrollx_ofs);
				break;
			case 3:
				state->scrolly[0] = (data + (((state->gfxreg & 0x10) >> 4) * 0x100) - state->scrolly_ofs);
				break;
		}
	}
	else
	{
		switch (offset)
		{
			case 0:
				state->scrollx[1] = (data + (((state->gfxreg & 0x08) >> 3) * 0x100) - 0x1f7);
				break;
			case 1:
				state->scrolly[1] = (data + (((state->gfxreg & 0x04) >> 2) * 0x100) - 0xf9);
				break;
			case 2:
				state->scrollx[0] = (data + (((state->gfxreg & 0x20) >> 5) * 0x100) - 0x1f7);
				break;
			case 3:
				state->scrolly[0] = (data + (((state->gfxreg & 0x10) >> 4) * 0x100) - 0xf9);
				break;
		}
	}
}



/*************************************
 *
 *  Fake video controller
 *
 *************************************/

static TIMER_CALLBACK( crtc_interrupt_gen )
{
	fromance_state *state = (fromance_state *)machine->driver_data;
	cpu_set_input_line(state->subcpu, 0, HOLD_LINE);
	if (param != 0)
		timer_adjust_periodic(state->crtc_timer, attotime_div(machine->primary_screen->frame_period(), param), 0, attotime_div(machine->primary_screen->frame_period(), param));
}


WRITE8_HANDLER( fromance_crtc_data_w )
{
	fromance_state *state = (fromance_state *)space->machine->driver_data;
	state->crtc_data[state->crtc_register] = data;

	switch (state->crtc_register)
	{
		/* only register we know about.... */
		case 0x0b:
			timer_adjust_oneshot(state->crtc_timer, space->machine->primary_screen->time_until_vblank_start(), (data > 0x80) ? 2 : 1);
			break;

		default:
			logerror("CRTC register %02X = %02X\n", state->crtc_register, data & 0xff);
			break;
	}
}


WRITE8_HANDLER( fromance_crtc_register_w )
{
	fromance_state *state = (fromance_state *)space->machine->driver_data;
	state->crtc_register = data;
}



/*************************************
 *
 *  Sprite routines (Pipe Dream)
 *
 *************************************/

static void draw_sprites( screen_device &screen, bitmap_t *bitmap, const rectangle *cliprect, int draw_priority )
{
	fromance_state *state = (fromance_state *)screen.machine->driver_data;
	static const UINT8 zoomtable[16] = { 0,7,14,20,25,30,34,38,42,46,49,52,54,57,59,61 };
	const rectangle &visarea = screen.visible_area();
	UINT8 *spriteram = state->spriteram;
	int offs;

	/* draw the sprites */
	for (offs = 0; offs < state->spriteram_size; offs += 8)
	{
		int data2 = spriteram[offs + 4] | (spriteram[offs + 5] << 8);
		int priority = (data2 >> 4) & 1;

		/* turns out the sprites are the same as in aerofgt.c */
		if ((data2 & 0x80) && priority == draw_priority)
		{
			int data0 = spriteram[offs + 0] | (spriteram[offs + 1] << 8);
			int data1 = spriteram[offs + 2] | (spriteram[offs + 3] << 8);
			int data3 = spriteram[offs + 6] | (spriteram[offs + 7] << 8);
			int code = data3 & 0xfff;
			int color = data2 & 0x0f;
			int y = (data0 & 0x1ff) - 6;
			int x = (data1 & 0x1ff) - 13;
			int yzoom = (data0 >> 12) & 15;
			int xzoom = (data1 >> 12) & 15;
			int zoomed = (xzoom | yzoom);
			int ytiles = ((data2 >> 12) & 7) + 1;
			int xtiles = ((data2 >> 8) & 7) + 1;
			int yflip = (data2 >> 15) & 1;
			int xflip = (data2 >> 11) & 1;
			int xt, yt;

			/* compute the zoom factor -- stolen from aerofgt.c */
			xzoom = 16 - zoomtable[xzoom] / 8;
			yzoom = 16 - zoomtable[yzoom] / 8;

			/* wrap around */
			if (x > visarea.max_x)
				x -= 0x200;
			if (y > visarea.max_y)
				y -= 0x200;

			/* flip ? */
			if (state->flipscreen)
			{
				y = visarea.max_y - y - 16 * ytiles - 4;
				x = visarea.max_x - x - 16 * xtiles - 24;
				xflip=!xflip;
				yflip=!yflip;
			}

			/* normal case */
			if (!xflip && !yflip)
			{
				for (yt = 0; yt < ytiles; yt++)
					for (xt = 0; xt < xtiles; xt++, code++)
						if (!zoomed)
							drawgfx_transpen(bitmap, cliprect, screen.machine->gfx[2], code, color, 0, 0,
									x + xt * 16, y + yt * 16, 15);
						else
							drawgfxzoom_transpen(bitmap, cliprect, screen.machine->gfx[2], code, color, 0, 0,
									x + xt * xzoom, y + yt * yzoom,
									0x1000 * xzoom, 0x1000 * yzoom, 15);
			}

			/* xflipped case */
			else if (xflip && !yflip)
			{
				for (yt = 0; yt < ytiles; yt++)
					for (xt = 0; xt < xtiles; xt++, code++)
						if (!zoomed)
							drawgfx_transpen(bitmap, cliprect, screen.machine->gfx[2], code, color, 1, 0,
									x + (xtiles - 1 - xt) * 16, y + yt * 16, 15);
						else
							drawgfxzoom_transpen(bitmap, cliprect, screen.machine->gfx[2], code, color, 1, 0,
									x + (xtiles - 1 - xt) * xzoom, y + yt * yzoom,
									0x1000 * xzoom, 0x1000 * yzoom, 15);
			}

			/* yflipped case */
			else if (!xflip && yflip)
			{
				for (yt = 0; yt < ytiles; yt++)
					for (xt = 0; xt < xtiles; xt++, code++)
						if (!zoomed)
							drawgfx_transpen(bitmap, cliprect, screen.machine->gfx[2], code, color, 0, 1,
									x + xt * 16, y + (ytiles - 1 - yt) * 16, 15);
						else
							drawgfxzoom_transpen(bitmap, cliprect, screen.machine->gfx[2], code, color, 0, 1,
									x + xt * xzoom, y + (ytiles - 1 - yt) * yzoom,
									0x1000 * xzoom, 0x1000 * yzoom, 15);
			}

			/* x & yflipped case */
			else
			{
				for (yt = 0; yt < ytiles; yt++)
					for (xt = 0; xt < xtiles; xt++, code++)
						if (!zoomed)
							drawgfx_transpen(bitmap, cliprect, screen.machine->gfx[2], code, color, 1, 1,
									x + (xtiles - 1 - xt) * 16, y + (ytiles - 1 - yt) * 16, 15);
						else
							drawgfxzoom_transpen(bitmap, cliprect, screen.machine->gfx[2], code, color, 1, 1,
									x + (xtiles - 1 - xt) * xzoom, y + (ytiles - 1 - yt) * yzoom,
									0x1000 * xzoom, 0x1000 * yzoom, 15);
			}
		}
	}
}



/*************************************
 *
 *  Main screen refresh
 *
 *************************************/

VIDEO_UPDATE( fromance )
{
	fromance_state *state = (fromance_state *)screen->machine->driver_data;

	tilemap_set_scrollx(state->bg_tilemap, 0, state->scrollx[0]);
	tilemap_set_scrolly(state->bg_tilemap, 0, state->scrolly[0]);
	tilemap_set_scrollx(state->fg_tilemap, 0, state->scrollx[1]);
	tilemap_set_scrolly(state->fg_tilemap, 0, state->scrolly[1]);

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0);
	return 0;
}


VIDEO_UPDATE( pipedrm )
{
	fromance_state *state = (fromance_state *)screen->machine->driver_data;

	/* there seems to be no logical mapping for the X scroll register -- maybe it's gone */
	tilemap_set_scrolly(state->bg_tilemap, 0, state->scrolly[1]);
	tilemap_set_scrolly(state->fg_tilemap, 0, state->scrolly[0]);

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0);

	draw_sprites(*screen, bitmap, cliprect, 0);
	draw_sprites(*screen, bitmap, cliprect, 1);
	return 0;
}
