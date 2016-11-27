#include "emu.h"
#include "video/konicdev.h"
#include "includes/f1gp.h"


#define TOTAL_CHARS 0x800


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( f1gp_get_roz_tile_info )
{
	f1gp_state *state = (f1gp_state *)machine->driver_data;
	int code = state->rozvideoram[tile_index];

	SET_TILE_INFO(3, code & 0x7ff, code >> 12, 0);
}

static TILE_GET_INFO( f1gp2_get_roz_tile_info )
{
	f1gp_state *state = (f1gp_state *)machine->driver_data;
	int code = state->rozvideoram[tile_index];

	SET_TILE_INFO(2, (code & 0x7ff) + (state->roz_bank << 11), code >> 12, 0);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	f1gp_state *state = (f1gp_state *)machine->driver_data;
	int code = state->fgvideoram[tile_index];

	SET_TILE_INFO(0, code & 0x7fff, 0, (code & 0x8000) ? TILE_FLIPY : 0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( f1gp )
{
	f1gp_state *state = (f1gp_state *)machine->driver_data;

	state->roz_tilemap = tilemap_create(machine, f1gp_get_roz_tile_info, tilemap_scan_rows, 16, 16, 64, 64);
	state->fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 8, 8, 64, 32);

	tilemap_set_transparent_pen(state->fg_tilemap, 0xff);

	state->zoomdata = (UINT16 *)memory_region(machine, "gfx4");
	gfx_element_set_source(machine->gfx[3], (UINT8 *)state->zoomdata);

//  state_save_register_global_pointer(machine, state->zoomdata, memory_region_length(machine, "gfx4"));
}


VIDEO_START( f1gpb )
{
	f1gp_state *state = (f1gp_state *)machine->driver_data;

	state->roz_tilemap = tilemap_create(machine, f1gp_get_roz_tile_info, tilemap_scan_rows, 16, 16, 64, 64);
	state->fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 8, 8, 64, 32);

	tilemap_set_transparent_pen(state->fg_tilemap, 0xff);

	state->zoomdata = (UINT16 *)memory_region(machine, "gfx4");
	gfx_element_set_source(machine->gfx[3], (UINT8 *)state->zoomdata);

//  state_save_register_global_pointer(machine, state->zoomdata, memory_region_length(machine, "gfx4"));
}

VIDEO_START( f1gp2 )
{
	f1gp_state *state = (f1gp_state *)machine->driver_data;

	state->roz_tilemap = tilemap_create(machine, f1gp2_get_roz_tile_info, tilemap_scan_rows, 16, 16, 64, 64);
	state->fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 8, 8, 64, 32);

	tilemap_set_transparent_pen(state->fg_tilemap, 0xff);
	tilemap_set_transparent_pen(state->roz_tilemap, 0x0f);

	tilemap_set_scrolldx(state->fg_tilemap, -80, 0);
	tilemap_set_scrolldy(state->fg_tilemap, -26, 0);
}


/***************************************************************************

  Memory handlers

***************************************************************************/

READ16_HANDLER( f1gp_zoomdata_r )
{
	f1gp_state *state = (f1gp_state *)space->machine->driver_data;
	return state->zoomdata[offset];
}

WRITE16_HANDLER( f1gp_zoomdata_w )
{
	f1gp_state *state = (f1gp_state *)space->machine->driver_data;
	COMBINE_DATA(&state->zoomdata[offset]);
	gfx_element_mark_dirty(space->machine->gfx[3], offset / 64);
}

READ16_HANDLER( f1gp_rozvideoram_r )
{
	f1gp_state *state = (f1gp_state *)space->machine->driver_data;
	return state->rozvideoram[offset];
}

WRITE16_HANDLER( f1gp_rozvideoram_w )
{
	f1gp_state *state = (f1gp_state *)space->machine->driver_data;
	COMBINE_DATA(&state->rozvideoram[offset]);
	tilemap_mark_tile_dirty(state->roz_tilemap, offset);
}

WRITE16_HANDLER( f1gp_fgvideoram_w )
{
	f1gp_state *state = (f1gp_state *)space->machine->driver_data;
	COMBINE_DATA(&state->fgvideoram[offset]);
	tilemap_mark_tile_dirty(state->fg_tilemap, offset);
}

WRITE16_HANDLER( f1gp_fgscroll_w )
{
	f1gp_state *state = (f1gp_state *)space->machine->driver_data;
	COMBINE_DATA(&state->scroll[offset]);

	tilemap_set_scrollx(state->fg_tilemap, 0, state->scroll[0]);
	tilemap_set_scrolly(state->fg_tilemap, 0, state->scroll[1]);
}

WRITE16_HANDLER( f1gp_gfxctrl_w )
{
	f1gp_state *state = (f1gp_state *)space->machine->driver_data;
	if (ACCESSING_BITS_0_7)
	{
		state->flipscreen = data & 0x20;
		state->gfxctrl = data & 0xdf;
	}
}

WRITE16_HANDLER( f1gp2_gfxctrl_w )
{
	f1gp_state *state = (f1gp_state *)space->machine->driver_data;
	if (ACCESSING_BITS_0_7)
	{
		state->flipscreen = data & 0x20;

		/* bit 0/1 = fg/sprite/roz priority */
		/* bit 2 = blank screen */

		state->gfxctrl = data & 0xdf;
	}

	if (ACCESSING_BITS_8_15)
	{
		if (state->roz_bank != (data >> 8))
		{
			state->roz_bank = (data >> 8);
			tilemap_mark_all_tiles_dirty(state->roz_tilemap);
		}
	}
}


/***************************************************************************

  Display refresh

***************************************************************************/

static void f1gp_draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int chip, int primask )
{
	f1gp_state *state = (f1gp_state *)machine->driver_data;
	int attr_start, first;
	UINT16 *spram = chip ? state->spr2vram : state->spr1vram;

	first = 4 * spram[0x1fe];

	for (attr_start = 0x0200 - 8; attr_start >= first; attr_start -= 4)
	{
		int map_start;
		int ox, oy, x, y, xsize, ysize, zoomx, zoomy, flipx, flipy, color/*, pri*/;
		/* table hand made by looking at the ship explosion in attract mode */
		/* it's almost a logarithmic scale but not exactly */
		static const int zoomtable[16] = { 0,7,14,20,25,30,34,38,42,46,49,52,54,57,59,61 };

		if (!(spram[attr_start + 2] & 0x0080)) continue;

		ox = spram[attr_start + 1] & 0x01ff;
		xsize = (spram[attr_start + 2] & 0x0700) >> 8;
		zoomx = (spram[attr_start + 1] & 0xf000) >> 12;
		oy = spram[attr_start + 0] & 0x01ff;
		ysize = (spram[attr_start + 2] & 0x7000) >> 12;
		zoomy = (spram[attr_start + 0] & 0xf000) >> 12;
		flipx = spram[attr_start + 2] & 0x0800;
		flipy = spram[attr_start + 2] & 0x8000;
		color = (spram[attr_start + 2] & 0x000f);// + 16 * spritepalettebank;
		//pri = spram[attr_start + 2] & 0x0010;
		map_start = spram[attr_start + 3];

		zoomx = 16 - zoomtable[zoomx] / 8;
		zoomy = 16 - zoomtable[zoomy] / 8;

		for (y = 0; y <= ysize; y++)
		{
			int sx, sy;

			if (flipy) sy = ((oy + zoomy * (ysize - y) + 16) & 0x1ff) - 16;
			else sy = ((oy + zoomy * y + 16) & 0x1ff) - 16;

			for (x = 0; x <= xsize; x++)
			{
				int code;

				if (flipx) sx = ((ox + zoomx * (xsize - x) + 16) & 0x1ff) - 16;
				else sx = ((ox + zoomx * x + 16) & 0x1ff) - 16;

				if (chip == 0)
					code = state->spr1cgram[map_start % (state->spr1cgram_size / 2)];
				else
					code = state->spr2cgram[map_start % (state->spr2cgram_size / 2)];

				pdrawgfxzoom_transpen(bitmap,cliprect,machine->gfx[1 + chip],
						code,
						color,
						flipx,flipy,
						sx,sy,
						0x1000 * zoomx,0x1000 * zoomy,
						machine->priority_bitmap,
//                      pri ? 0 : 0x2);
						primask,15);
				map_start++;
			}

			if (xsize == 2) map_start += 1;
			if (xsize == 4) map_start += 3;
			if (xsize == 5) map_start += 2;
			if (xsize == 6) map_start += 1;
		}
	}
}


VIDEO_UPDATE( f1gp )
{
	f1gp_state *state = (f1gp_state *)screen->machine->driver_data;

	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

	k053936_zoom_draw(state->k053936, bitmap, cliprect, state->roz_tilemap, 0, 0, 1);

	tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 1);

	/* quick kludge for "continue" screen priority */
	if (state->gfxctrl == 0x00)
	{
		f1gp_draw_sprites(screen->machine, bitmap, cliprect, 0, 0x02);
		f1gp_draw_sprites(screen->machine, bitmap, cliprect, 1, 0x02);
	}
	else
	{
		f1gp_draw_sprites(screen->machine, bitmap, cliprect, 0, 0x00);
		f1gp_draw_sprites(screen->machine, bitmap, cliprect, 1, 0x02);
	}
	return 0;
}


static void f1gpb_draw_sprites( running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect )
{
	f1gp_state *state = (f1gp_state *)machine->driver_data;
	UINT16 *spriteram = state->spriteram;
	int attr_start, start_offset = state->spriteram_size / 2 - 4;

	// find the "end of list" to draw the sprites in reverse order
	for (attr_start = 4; attr_start < state->spriteram_size / 2; attr_start += 4)
	{
		if (spriteram[attr_start + 3 - 4] == 0xffff) /* end of list marker */
		{
			start_offset = attr_start - 4;
			break;
		}
	}

	for (attr_start = start_offset;attr_start >= 4;attr_start -= 4)
	{
		int code, gfx;
		int x, y, flipx, flipy, color, pri;

		x = (spriteram[attr_start + 2] & 0x03ff) - 48;
		y = (256 - (spriteram[attr_start + 3 - 4] & 0x03ff)) - 15;
		flipx = spriteram[attr_start + 1] & 0x0800;
		flipy = spriteram[attr_start + 1] & 0x8000;
		color = spriteram[attr_start + 1] & 0x000f;
		code = spriteram[attr_start + 0] & 0x3fff;
		pri = 0; //?

		if((spriteram[attr_start + 1] & 0x00f0) && (spriteram[attr_start + 1] & 0x00f0) != 0xc0)
		{
			printf("attr %X\n",spriteram[attr_start + 1] & 0x00f0);
			code = mame_rand(machine);
		}

/*
        if (spriteram[attr_start + 1] & ~0x88cf)
            printf("1 = %X\n", spriteram[attr_start + 1] & ~0x88cf);
*/
		if(code >= 0x2000)
		{
			gfx = 1;
			code -= 0x2000;
		}
		else
		{
			gfx = 0;
		}

		pdrawgfx_transpen(bitmap,cliprect,machine->gfx[1 + gfx],
			code,
			color,
			flipx,flipy,
			x,y,
			machine->priority_bitmap,
			pri ? 0 : 0x2,15);

		// wrap around x
		pdrawgfx_transpen(bitmap,cliprect,machine->gfx[1 + gfx],
			code,
			color,
			flipx,flipy,
			x - 512,y,
			machine->priority_bitmap,
			pri ? 0 : 0x2,15);
	}
}

VIDEO_UPDATE( f1gpb )
{
	f1gp_state *state = (f1gp_state *)screen->machine->driver_data;
	UINT32 startx, starty;
	int incxx, incxy, incyx, incyy;

	incxy = (INT16)state->rozregs[1];
	incyx = -incxy;
	incxx = incyy = (INT16)state->rozregs[3];
	startx = state->rozregs[0] + 328;
	starty = state->rozregs[2];

	tilemap_set_scrolly(state->fg_tilemap, 0, state->fgregs[0] + 8);

	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

	tilemap_draw_roz(bitmap, cliprect, state->roz_tilemap,
		startx << 13, starty << 13,
		incxx << 5, incxy << 5, incyx << 5, incyy << 5,
		1, 0, 0);

	tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 1);

	f1gpb_draw_sprites(screen->machine, bitmap, cliprect);

	return 0;
}


static void f1gp2_draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	f1gp_state *state = (f1gp_state *)machine->driver_data;
	int offs;

	offs = 0;
	while (offs < 0x0400 && (state->spritelist[offs] & 0x4000) == 0)
	{
		int attr_start;
		int map_start;
		int ox, oy, x, y, xsize, ysize, zoomx, zoomy, flipx, flipy, color;

		attr_start = 4 * (state->spritelist[offs++] & 0x01ff);

		ox = state->spritelist[attr_start + 1] & 0x01ff;
		xsize = (state->spritelist[attr_start + 1] & 0x0e00) >> 9;
		zoomx = (state->spritelist[attr_start + 1] & 0xf000) >> 12;
		oy = state->spritelist[attr_start + 0] & 0x01ff;
		ysize = (state->spritelist[attr_start + 0] & 0x0e00) >> 9;
		zoomy = (state->spritelist[attr_start + 0] & 0xf000) >> 12;
		flipx = state->spritelist[attr_start + 2] & 0x4000;
		flipy = state->spritelist[attr_start + 2] & 0x8000;
		color = (state->spritelist[attr_start + 2] & 0x1f00) >> 8;
		map_start = state->spritelist[attr_start + 3] & 0x7fff;

// aerofgt has the following adjustment, but doing it here would break the title screen
//      ox += (xsize*zoomx+2)/4;
//      oy += (ysize*zoomy+2)/4;

		zoomx = 32 - zoomx;
		zoomy = 32 - zoomy;

		if (state->spritelist[attr_start + 2] & 0x20ff)
			color = mame_rand(machine);

		for (y = 0; y <= ysize; y++)
		{
			int sx,sy;

			if (flipy) sy = ((oy + zoomy * (ysize - y)/2 + 16) & 0x1ff) - 16;
			else sy = ((oy + zoomy * y / 2 + 16) & 0x1ff) - 16;

			for (x = 0; x <= xsize; x++)
			{
				int code;

				if (flipx) sx = ((ox + zoomx * (xsize - x) / 2 + 16) & 0x1ff) - 16;
				else sx = ((ox + zoomx * x / 2 + 16) & 0x1ff) - 16;

				code = state->sprcgram[map_start & 0x3fff];
				map_start++;

				if (state->flipscreen)
					drawgfxzoom_transpen(bitmap,cliprect,machine->gfx[1],
							code,
							color,
							!flipx,!flipy,
							304-sx,208-sy,
							zoomx << 11,zoomy << 11,15);
				else
					drawgfxzoom_transpen(bitmap,cliprect,machine->gfx[1],
							code,
							color,
							flipx,flipy,
							sx,sy,
							zoomx << 11,zoomy << 11,15);
			}
		}
	}
}


VIDEO_UPDATE( f1gp2 )
{
	f1gp_state *state = (f1gp_state *)screen->machine->driver_data;

	if (state->gfxctrl & 4)	/* blank screen */
		bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));
	else
	{
		switch (state->gfxctrl & 3)
		{
			case 0:
				k053936_zoom_draw(state->k053936, bitmap, cliprect, state->roz_tilemap, TILEMAP_DRAW_OPAQUE, 0, 1);
				f1gp2_draw_sprites(screen->machine, bitmap, cliprect);
				tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0);
				break;
			case 1:
				k053936_zoom_draw(state->k053936, bitmap, cliprect, state->roz_tilemap, TILEMAP_DRAW_OPAQUE, 0, 1);
				tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0);
				f1gp2_draw_sprites(screen->machine, bitmap, cliprect);
				break;
			case 2:
				tilemap_draw(bitmap, cliprect, state->fg_tilemap, TILEMAP_DRAW_OPAQUE, 0);
				k053936_zoom_draw(state->k053936, bitmap, cliprect, state->roz_tilemap, 0, 0, 1);
				f1gp2_draw_sprites(screen->machine, bitmap, cliprect);
				break;
#ifdef MAME_DEBUG
			case 3:
				popmessage("unsupported priority 3\n");
#endif
		}
	}
	return 0;
}
