#include "emu.h"
#include "includes/playmark.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( bigtwin_get_tx_tile_info )
{
	playmark_state *state = (playmark_state *)machine->driver_data;
	UINT16 code = state->videoram1[2 * tile_index];
	UINT16 color = state->videoram1[2 * tile_index + 1];
	SET_TILE_INFO(
			2,
			code,
			color,
			0);
}

static TILE_GET_INFO( bigtwin_get_fg_tile_info )
{
	playmark_state *state = (playmark_state *)machine->driver_data;
	UINT16 code = state->videoram2[2 * tile_index];
	UINT16 color = state->videoram2[2 * tile_index + 1];
	SET_TILE_INFO(
			1,
			code,
			color,
			0);
}

static TILE_GET_INFO( wbeachvl_get_tx_tile_info )
{
	playmark_state *state = (playmark_state *)machine->driver_data;
	UINT16 code = state->videoram1[2 * tile_index];
	UINT16 color = state->videoram1[2 * tile_index + 1];

	SET_TILE_INFO(
			2,
			code,
			color / 4,
			0);
}

static TILE_GET_INFO( wbeachvl_get_fg_tile_info )
{
	playmark_state *state = (playmark_state *)machine->driver_data;
	UINT16 code = state->videoram2[2 * tile_index];
	UINT16 color = state->videoram2[2 * tile_index + 1];

	SET_TILE_INFO(
			1,
			code & 0x7fff,
			color / 4 + 8,
			(code & 0x8000) ? TILE_FLIPX : 0);
}

static TILE_GET_INFO( wbeachvl_get_bg_tile_info )
{
	playmark_state *state = (playmark_state *)machine->driver_data;
	UINT16 code = state->videoram3[2 * tile_index];
	UINT16 color = state->videoram3[2 * tile_index + 1];

	SET_TILE_INFO(
			1,
			code & 0x7fff,
			color / 4,
			(code & 0x8000) ? TILE_FLIPX : 0);
}

static TILE_GET_INFO( hrdtimes_get_tx_tile_info )
{
	playmark_state *state = (playmark_state *)machine->driver_data;
	int code = state->videoram1[tile_index] & 0x03ff;
	int colr = state->videoram1[tile_index] & 0xe000;

	SET_TILE_INFO(2,code + state->txt_tile_offset, colr >> 13, 0);
}

static TILE_GET_INFO( hrdtimes_get_fg_tile_info )
{
	playmark_state *state = (playmark_state *)machine->driver_data;
	int code = state->videoram2[tile_index] & 0x1fff;
	int colr = state->videoram2[tile_index] & 0xe000;

	SET_TILE_INFO(1,code + 0x2000,(colr >> 13) + 8,0);
}

static TILE_GET_INFO( hrdtimes_get_bg_tile_info )
{
	playmark_state *state = (playmark_state *)machine->driver_data;
	int code = state->videoram3[tile_index] & 0x1fff;
	int colr = state->videoram3[tile_index] & 0xe000;

	SET_TILE_INFO(1, code, colr >> 13, 0);
}

/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( bigtwin )
{
	playmark_state *state = (playmark_state *)machine->driver_data;

	state->tx_tilemap = tilemap_create(machine, bigtwin_get_tx_tile_info, tilemap_scan_rows, 8, 8, 64, 32);
	state->fg_tilemap = tilemap_create(machine, bigtwin_get_fg_tile_info, tilemap_scan_rows, 16, 16, 32, 32);

	tilemap_set_transparent_pen(state->tx_tilemap, 0);

	state->xoffset = 0;
	state->yoffset = 0;
	state->txt_tile_offset = 0;

	state->pri_masks[0] = 0;
	state->pri_masks[1] = 0;
	state->pri_masks[2] = 0;
}


VIDEO_START( wbeachvl )
{
	playmark_state *state = (playmark_state *)machine->driver_data;

	state->tx_tilemap = tilemap_create(machine, wbeachvl_get_tx_tile_info, tilemap_scan_rows, 8, 8, 64, 32);
	state->fg_tilemap = tilemap_create(machine, wbeachvl_get_fg_tile_info, tilemap_scan_rows, 16, 16, 64, 32);
	state->bg_tilemap = tilemap_create(machine, wbeachvl_get_bg_tile_info, tilemap_scan_rows, 16, 16, 64, 32);

	tilemap_set_transparent_pen(state->tx_tilemap, 0);
	tilemap_set_transparent_pen(state->fg_tilemap, 0);

	state->xoffset = 0;
	state->yoffset = 0;
	state->txt_tile_offset = 0;

	state->pri_masks[0] = 0xfff0;
	state->pri_masks[1] = 0xfffc;
	state->pri_masks[2] = 0;
}

VIDEO_START( excelsr )
{
	playmark_state *state = (playmark_state *)machine->driver_data;

	state->tx_tilemap = tilemap_create(machine, bigtwin_get_tx_tile_info, tilemap_scan_rows, 16, 16, 32, 32);
	state->fg_tilemap = tilemap_create(machine, bigtwin_get_fg_tile_info, tilemap_scan_rows, 16, 16, 32, 32);

	tilemap_set_transparent_pen(state->tx_tilemap, 0);

	state->xoffset = 0;
	state->yoffset = 0;
	state->txt_tile_offset = 0;

	state->pri_masks[0] = 0;
	state->pri_masks[1] = 0xfffc;
	state->pri_masks[2] = 0xfff0;
}

VIDEO_START( hotmind )
{
	playmark_state *state = (playmark_state *)machine->driver_data;

	state->tx_tilemap = tilemap_create(machine, hrdtimes_get_tx_tile_info, tilemap_scan_rows, 8, 8, 64, 64);
	state->fg_tilemap = tilemap_create(machine, hrdtimes_get_fg_tile_info, tilemap_scan_rows, 16, 16, 32, 32);
	state->bg_tilemap = tilemap_create(machine, hrdtimes_get_bg_tile_info, tilemap_scan_rows, 16, 16, 32, 32);

	tilemap_set_transparent_pen(state->tx_tilemap, 0);
	tilemap_set_transparent_pen(state->fg_tilemap, 0);

	tilemap_set_scrolldx(state->tx_tilemap, -14, -14);
	tilemap_set_scrolldx(state->fg_tilemap, -14, -14);
	tilemap_set_scrolldx(state->bg_tilemap, -14, -14);

	state->xoffset = -9;
	state->yoffset = -8;
	state->txt_tile_offset = 0x9000;

	state->pri_masks[0] = 0xfff0;
	state->pri_masks[1] = 0xfffc;
	state->pri_masks[2] = 0;
}

VIDEO_START( hrdtimes )
{
	playmark_state *state = (playmark_state *)machine->driver_data;

	state->tx_tilemap = tilemap_create(machine, hrdtimes_get_tx_tile_info,tilemap_scan_rows, 8, 8, 64, 64);
	state->fg_tilemap = tilemap_create(machine, hrdtimes_get_fg_tile_info,tilemap_scan_rows, 16, 16, 32, 32);
	state->bg_tilemap = tilemap_create(machine, hrdtimes_get_bg_tile_info,tilemap_scan_rows, 16, 16, 32, 32);

	tilemap_set_transparent_pen(state->tx_tilemap, 0);
	tilemap_set_transparent_pen(state->fg_tilemap, 0);

	tilemap_set_scrolldx(state->tx_tilemap, -14, -14);
	tilemap_set_scrolldx(state->fg_tilemap, -10, -10);
	tilemap_set_scrolldx(state->bg_tilemap, -12, -12);

	state->xoffset = -8;
	state->yoffset = -8;
	state->txt_tile_offset = 0xfc00;

	state->pri_masks[0] = 0xfff0;
	state->pri_masks[1] = 0xfffc;
	state->pri_masks[2] = 0;
}

/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE16_HANDLER( wbeachvl_txvideoram_w )
{
	playmark_state *state = (playmark_state *)space->machine->driver_data;

	COMBINE_DATA(&state->videoram1[offset]);
	tilemap_mark_tile_dirty(state->tx_tilemap, offset / 2);
}

WRITE16_HANDLER( wbeachvl_fgvideoram_w )
{
	playmark_state *state = (playmark_state *)space->machine->driver_data;

	COMBINE_DATA(&state->videoram2[offset]);
	tilemap_mark_tile_dirty(state->fg_tilemap, offset / 2);
}

WRITE16_HANDLER( wbeachvl_bgvideoram_w )
{
	playmark_state *state = (playmark_state *)space->machine->driver_data;

	COMBINE_DATA(&state->videoram3[offset]);
	tilemap_mark_tile_dirty(state->bg_tilemap, offset / 2);
}

WRITE16_HANDLER( hrdtimes_txvideoram_w )
{
	playmark_state *state = (playmark_state *)space->machine->driver_data;

	COMBINE_DATA(&state->videoram1[offset]);
	tilemap_mark_tile_dirty(state->tx_tilemap, offset);
}

WRITE16_HANDLER( hrdtimes_fgvideoram_w )
{
	playmark_state *state = (playmark_state *)space->machine->driver_data;

	COMBINE_DATA(&state->videoram2[offset]);
	tilemap_mark_tile_dirty(state->fg_tilemap, offset);
}

WRITE16_HANDLER( hrdtimes_bgvideoram_w )
{
	playmark_state *state = (playmark_state *)space->machine->driver_data;

	COMBINE_DATA(&state->videoram3[offset]);
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}


WRITE16_HANDLER( bigtwin_paletteram_w )
{
	int r, g, b, val;

	COMBINE_DATA(&space->machine->generic.paletteram.u16[offset]);

	val = space->machine->generic.paletteram.u16[offset];
	r = (val >> 11) & 0x1e;
	g = (val >>  7) & 0x1e;
	b = (val >>  3) & 0x1e;

	r |= ((val & 0x08) >> 3);
	g |= ((val & 0x04) >> 2);
	b |= ((val & 0x02) >> 1);

	palette_set_color_rgb(space->machine, offset, pal5bit(r), pal5bit(g), pal5bit(b));
}

WRITE16_HANDLER( bigtwin_scroll_w )
{
	playmark_state *state = (playmark_state *)space->machine->driver_data;

	data = COMBINE_DATA(&state->scroll[offset]);

	switch (offset)
	{
		case 0: 	tilemap_set_scrollx(state->tx_tilemap, 0, data + 2); break;
		case 1: 	tilemap_set_scrolly(state->tx_tilemap, 0, data);   break;
		case 2: 	state->bgscrollx = -(data + 4);                    break;
		case 3: 	state->bgscrolly = (-data) & 0x1ff;
				state->bg_enable = data & 0x0200;
				state->bg_full_size = data & 0x0400;
				break;
		case 4: 	tilemap_set_scrollx(state->fg_tilemap, 0, data + 6); break;
		case 5: 	tilemap_set_scrolly(state->fg_tilemap, 0, data);   break;
	}
}

WRITE16_HANDLER( wbeachvl_scroll_w )
{
	playmark_state *state = (playmark_state *)space->machine->driver_data;

	data = COMBINE_DATA(&state->scroll[offset]);

	switch (offset)
	{
		case 0: 	tilemap_set_scrollx(state->tx_tilemap, 0, data + 2); break;
		case 1: 	tilemap_set_scrolly(state->tx_tilemap, 0, data);   break;
		case 2: 	state->fgscrollx = data + 4;break;
		case 3: 	tilemap_set_scrolly(state->fg_tilemap, 0, data & 0x3ff);
				state->fg_rowscroll_enable = data & 0x0800;
				break;
		case 4: 	tilemap_set_scrollx(state->bg_tilemap, 0, data + 6); break;
		case 5: 	tilemap_set_scrolly(state->bg_tilemap, 0, data);   break;
	}
}

WRITE16_HANDLER( excelsr_scroll_w )
{
	playmark_state *state = (playmark_state *)space->machine->driver_data;

	data = COMBINE_DATA(&state->scroll[offset]);

	switch (offset)
	{
		case 0:	tilemap_set_scrollx(state->tx_tilemap, 0, data + 2); break;
		case 1: 	tilemap_set_scrolly(state->tx_tilemap, 0, data);   break;
		case 2: 	state->bgscrollx = -data;	break;
		case 3: 	state->bgscrolly = (-data + 2)& 0x1ff;
				state->bg_enable = data & 0x0200;
				state->bg_full_size = data & 0x0400;
				break;
		case 4:	tilemap_set_scrollx(state->fg_tilemap, 0, data + 6); break;
		case 5:	tilemap_set_scrolly(state->fg_tilemap, 0, data);   break;
	}
}

WRITE16_HANDLER( hrdtimes_scroll_w )
{
	playmark_state *state = (playmark_state *)space->machine->driver_data;

	data = COMBINE_DATA(&state->scroll[offset]);

	switch (offset)
	{
		case 0: tilemap_set_scrollx(state->tx_tilemap, 0, data); break;
		case 1: tilemap_set_scrolly(state->tx_tilemap, 0, data); break;
		case 2: tilemap_set_scrollx(state->fg_tilemap, 0, data); break;
		case 3: tilemap_set_scrolly(state->fg_tilemap, 0, data); break;
		case 4: tilemap_set_scrollx(state->bg_tilemap, 0, data); break;
		case 5: tilemap_set_scrolly(state->bg_tilemap, 0, data); break;
	}
}

/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int codeshift )
{
	playmark_state *state = (playmark_state *)machine->driver_data;
	int offs, start_offset = state->spriteram_size / 2 - 4;
	int height = machine->gfx[0]->height;
	int colordiv = machine->gfx[0]->color_granularity / 16;
	UINT16 *spriteram = state->spriteram;

	// find the "end of list" to draw the sprites in reverse order
	for (offs = 4; offs < state->spriteram_size / 2; offs += 4)
	{
		if (spriteram[offs + 3 - 4] == 0x2000) /* end of list marker */
		{
			start_offset = offs - 4;
			break;
		}
	}

	for (offs = start_offset; offs >= 4; offs -= 4)
	{
		int sx, sy, code, color, flipx, pri;

		sy = spriteram[offs + 3 - 4];	/* -4? what the... ??? */

		flipx = sy & 0x4000;
		sx = (spriteram[offs + 1] & 0x01ff) - 16 - 7;
		sy = (256 - 8 - height - sy) & 0xff;
		code = spriteram[offs + 2] >> codeshift;
		color = ((spriteram[offs + 1] & 0x3e00) >> 9) / colordiv;
		pri = (spriteram[offs + 1] & 0x8000) >> 15;

		if(!pri && (color & 0x0c) == 0x0c)
			pri = 2;

		pdrawgfx_transpen(bitmap,cliprect,machine->gfx[0],
				 code,
				 color,
				 flipx,0,
				 sx + state->xoffset,sy + state->yoffset,
				 machine->priority_bitmap,state->pri_masks[pri],0);
	}
}

static void draw_bitmap( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	playmark_state *state = (playmark_state *)machine->driver_data;
	int x, y, count;
	int color;
	UINT8 *pri;

	count = 0;
	for (y = 0; y < 512; y++)
	{
		for (x = 0; x < 512; x++)
		{
			color = state->bgvideoram[count] & 0xff;

			if (color)
			{
				if (state->bg_full_size)
				{
					*BITMAP_ADDR16(bitmap, (y + state->bgscrolly) & 0x1ff, (x + state->bgscrollx) & 0x1ff) = 0x100 + color;

					pri = BITMAP_ADDR8(machine->priority_bitmap, (y + state->bgscrolly) & 0x1ff, 0);
					pri[(x + state->bgscrollx) & 0x1ff] |= 2;
				}
				else
				{
					/* 50% size */
					if(!(x % 2) && !(y % 2))
					{
						*BITMAP_ADDR16(bitmap, (y / 2 + state->bgscrolly) & 0x1ff, (x / 2 + state->bgscrollx) & 0x1ff) = 0x100 + color;

						pri = BITMAP_ADDR8(machine->priority_bitmap, (y / 2 + state->bgscrolly) & 0x1ff, 0);
						pri[(x / 2 + state->bgscrollx) & 0x1ff] |= 2;
					}
				}
			}

			count++;
		}
	}
}

VIDEO_UPDATE( bigtwin )
{
	playmark_state *state = (playmark_state *)screen->machine->driver_data;

	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

	tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0);
	if (state->bg_enable)
		draw_bitmap(screen->machine, bitmap, cliprect);
	draw_sprites(screen->machine, bitmap, cliprect, 4);
	tilemap_draw(bitmap, cliprect, state->tx_tilemap, 0, 0);
	return 0;
}

VIDEO_UPDATE( excelsr )
{
	playmark_state *state = (playmark_state *)screen->machine->driver_data;

	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

	tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 1);
	if (state->bg_enable)
		draw_bitmap(screen->machine, bitmap, cliprect);
	tilemap_draw(bitmap, cliprect, state->tx_tilemap, 0, 4);
	draw_sprites(screen->machine, bitmap, cliprect, 2);
	return 0;
}

VIDEO_UPDATE( wbeachvl )
{
	playmark_state *state = (playmark_state *)screen->machine->driver_data;

	if (state->fg_rowscroll_enable)
	{
		int i;

		tilemap_set_scroll_rows(state->fg_tilemap, 512);
		for (i = 0; i < 256; i++)
			tilemap_set_scrollx(state->fg_tilemap, i + 1, state->rowscroll[8 * i]);
	}
	else
	{
		tilemap_set_scroll_rows(state->fg_tilemap, 1);
		tilemap_set_scrollx(state->fg_tilemap, 0, state->fgscrollx);
	}

	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 1);
	tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 2);
	draw_sprites(screen->machine, bitmap, cliprect, 0);
	tilemap_draw(bitmap, cliprect, state->tx_tilemap, 0, 0);
	return 0;
}

VIDEO_UPDATE( hrdtimes )
{
	playmark_state *state = (playmark_state *)screen->machine->driver_data;

	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

	// video enabled
	if (state->scroll[6] & 1)
	{
		tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 1);
		tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 2);
		draw_sprites(screen->machine, bitmap, cliprect, 2);
		tilemap_draw(bitmap, cliprect, state->tx_tilemap, 0, 0);
	}
	else
		bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));
	return 0;
}
