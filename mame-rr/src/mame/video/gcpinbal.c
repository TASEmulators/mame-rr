#include "emu.h"
#include "includes/gcpinbal.h"


/*******************************************************************/


static TILE_GET_INFO( get_bg0_tile_info )
{
	gcpinbal_state *state = (gcpinbal_state *)machine->driver_data;
	UINT16 tilenum = state->tilemapram[0 + tile_index * 2];
	UINT16 attr    = state->tilemapram[1 + tile_index * 2];

	SET_TILE_INFO(
			1,
			(tilenum & 0xfff) + state->bg0_gfxset,
			(attr & 0x1f),
			TILE_FLIPYX( (attr & 0x300) >> 8));
}

static TILE_GET_INFO( get_bg1_tile_info )
{
	gcpinbal_state *state = (gcpinbal_state *)machine->driver_data;
	UINT16 tilenum = state->tilemapram[0x800 + tile_index * 2];
	UINT16 attr    = state->tilemapram[0x801 + tile_index * 2];

	SET_TILE_INFO(
			1,
			(tilenum & 0xfff) + 0x2000 + state->bg1_gfxset,
			(attr & 0x1f) + 0x30,
			TILE_FLIPYX( (attr & 0x300) >> 8));
}

static TILE_GET_INFO( get_fg_tile_info )
{
	gcpinbal_state *state = (gcpinbal_state *)machine->driver_data;
	UINT16 tilenum = state->tilemapram[0x1000 + tile_index];

	SET_TILE_INFO(
			2,
			(tilenum & 0xfff),
			(tilenum >> 12) | 0x70,
			0);
}

static void gcpinbal_core_vh_start( running_machine *machine )
{
	gcpinbal_state *state = (gcpinbal_state *)machine->driver_data;
	int xoffs = 0;
	int yoffs = 0;

	state->tilemap[0] = tilemap_create(machine, get_bg0_tile_info,tilemap_scan_rows,16,16,32,32);
	state->tilemap[1] = tilemap_create(machine, get_bg1_tile_info,tilemap_scan_rows,16,16,32,32);
	state->tilemap[2] = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows,8,8,64,64);

	tilemap_set_transparent_pen(state->tilemap[0], 0);
	tilemap_set_transparent_pen(state->tilemap[1], 0);
	tilemap_set_transparent_pen(state->tilemap[2], 0);

	/* flipscreen n/a */
	tilemap_set_scrolldx(state->tilemap[0], -xoffs, 0);
	tilemap_set_scrolldx(state->tilemap[1], -xoffs, 0);
	tilemap_set_scrolldx(state->tilemap[2], -xoffs, 0);
	tilemap_set_scrolldy(state->tilemap[0], -yoffs, 0);
	tilemap_set_scrolldy(state->tilemap[1], -yoffs, 0);
	tilemap_set_scrolldy(state->tilemap[2], -yoffs, 0);
}

VIDEO_START( gcpinbal )
{
	gcpinbal_core_vh_start(machine);
}


/******************************************************************
                   TILEMAP READ AND WRITE HANDLERS
*******************************************************************/

READ16_HANDLER( gcpinbal_tilemaps_word_r )
{
	gcpinbal_state *state = (gcpinbal_state *)space->machine->driver_data;
	return state->tilemapram[offset];
}

WRITE16_HANDLER( gcpinbal_tilemaps_word_w )
{
	gcpinbal_state *state = (gcpinbal_state *)space->machine->driver_data;
	COMBINE_DATA(&state->tilemapram[offset]);

	if (offset < 0x800)	/* BG0 */
		tilemap_mark_tile_dirty(state->tilemap[0], offset / 2);
	else if ((offset < 0x1000))	/* BG1 */
		tilemap_mark_tile_dirty(state->tilemap[1], (offset % 0x800) / 2);
	else if ((offset < 0x1800))	/* FG */
		tilemap_mark_tile_dirty(state->tilemap[2], (offset % 0x800));
}


#ifdef UNUSED_FUNCTION

READ16_HANDLER( gcpinbal_ctrl_word_r )
{
    // ***** NOT HOOKED UP *****

    return gcpinbal_piv_ctrlram[offset];
}


WRITE16_HANDLER( gcpinbal_ctrl_word_w )
{
    // ***** NOT HOOKED UP *****

    COMBINE_DATA(&gcpinbal_piv_ctrlram[offset]);
    data = gcpinbal_piv_ctrlram[offset];

    switch (offset)
    {
        case 0x00:
            gcpinbal_scrollx[0] = -data;
            break;

        case 0x01:
            gcpinbal_scrollx[1] = -data;
            break;

        case 0x02:
            gcpinbal_scrollx[2] = -data;
            break;

        case 0x03:
            gcpinbal_scrolly[0] = data;
            break;

        case 0x04:
            gcpinbal_scrolly[1] = data;
            break;

        case 0x05:
            gcpinbal_scrolly[2] = data;
            break;

        case 0x06:
            gcpinbal_ctrl_reg = data;
            break;
    }
}

#endif


/****************************************************************
                     SPRITE DRAW ROUTINE

    Word |     Bit(s)      | Use
    -----+-----------------+-----------------
      0  |........ xxxxxxxx| X lo
      1  |........ xxxxxxxx| X hi
      2  |........ xxxxxxxx| Y lo
      3  |........ xxxxxxxx| Y hi
      4  |........ x.......| Disable
      4  |........ ...x....| Flip Y
      4  |........ ....x...| 1 = Y chain, 0 = X chain
      4  |........ .....xxx| Chain size
      5  |........ ??xxxxxx| Tile (low)
      6  |........ xxxxxxxx| Tile (high)
      7  |........ ....xxxx| Color Bank

    Modified table from Raine

****************************************************************/

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int y_offs )
{
	gcpinbal_state *state = (gcpinbal_state *)machine->driver_data;
	UINT16 *spriteram = state->spriteram;
	int offs, chain_pos;
	int x, y, curx, cury;
	int priority = 0;
	UINT8 col, flipx, flipy, chain;
	UINT16 code;

	/* According to Raine, word in ioc_ram determines sprite/tile priority... */
	priority = (state->ioc_ram[0x68 / 2] & 0x8800) ? 0 : 1;

	for (offs = state->spriteram_size / 2 - 8; offs >= 0; offs -= 8)
	{
		code = ((spriteram[offs + 5]) & 0xff) + (((spriteram[offs + 6]) & 0xff) << 8);
		code &= 0x3fff;

		if (!(spriteram[offs + 4] &0x80))	/* active sprite ? */
		{
			x = ((spriteram[offs + 0]) & 0xff) + (((spriteram[offs + 1]) & 0xff) << 8);
			y = ((spriteram[offs + 2]) & 0xff) + (((spriteram[offs + 3]) & 0xff) << 8);

			/* Treat coords as signed */
			if (x & 0x8000)  x -= 0x10000;
			if (y & 0x8000)  y -= 0x10000;

			col  = ((spriteram[offs + 7]) & 0x0f) | 0x60;
			chain = (spriteram[offs + 4]) & 0x07;
			flipy = (spriteram[offs + 4]) & 0x10;
			flipx = 0;

			curx = x;
			cury = y;

			if (((spriteram[offs + 4]) & 0x08) && flipy)
				cury += (chain * 16);

			for (chain_pos = chain; chain_pos >= 0; chain_pos--)
			{
				pdrawgfx_transpen(bitmap, cliprect,machine->gfx[0],
						code,
						col,
						flipx, flipy,
						curx,cury,
						machine->priority_bitmap,
						priority ? 0xfc : 0xf0,0);

				code++;

				if ((spriteram[offs + 4]) & 0x08)	/* Y chain */
				{
					if (flipy)	cury -= 16;
					else cury += 16;
				}
				else	/* X chain */
				{
					curx += 16;
				}
			}
		}
	}
#if 0
	if (rotate)
	{
		char buf[80];
		sprintf(buf, "sprite rotate offs %04x ?", rotate);
		popmessage(buf);
	}
#endif
}




/**************************************************************
                        SCREEN REFRESH
**************************************************************/

VIDEO_UPDATE( gcpinbal )
{
	gcpinbal_state *state = (gcpinbal_state *)screen->machine->driver_data;
	int i;
	UINT16 tile_sets = 0;
	UINT8 layer[3];

#ifdef MAME_DEBUG
	if (input_code_pressed_once(screen->machine, KEYCODE_V))
	{
		state->dislayer[0] ^= 1;
		popmessage("bg0: %01x", state->dislayer[0]);
	}

	if (input_code_pressed_once(screen->machine, KEYCODE_B))
	{
		state->dislayer[1] ^= 1;
		popmessage("bg1: %01x", state->dislayer[1]);
	}

	if (input_code_pressed_once(screen->machine, KEYCODE_N))
	{
		state->dislayer[2] ^= 1;
		popmessage("fg: %01x", state->dislayer[2]);
	}
#endif

	state->scrollx[0] =  state->ioc_ram[0x14 / 2];
	state->scrolly[0] =  state->ioc_ram[0x16 / 2];
	state->scrollx[1] =  state->ioc_ram[0x18 / 2];
	state->scrolly[1] =  state->ioc_ram[0x1a / 2];
	state->scrollx[2] =  state->ioc_ram[0x1c / 2];
	state->scrolly[2] =  state->ioc_ram[0x1e / 2];

	tile_sets = state->ioc_ram[0x88 / 2];
	state->bg0_gfxset = (tile_sets & 0x400) ? 0x1000 : 0;
	state->bg1_gfxset = (tile_sets & 0x800) ? 0x1000 : 0;

	for (i = 0; i < 3; i++)
	{
		tilemap_set_scrollx(state->tilemap[i], 0, state->scrollx[i]);
		tilemap_set_scrolly(state->tilemap[i], 0, state->scrolly[i]);
	}

	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);
	bitmap_fill(bitmap, cliprect, 0);

	layer[0] = 0;
	layer[1] = 1;
	layer[2] = 2;


#ifdef MAME_DEBUG
	if (state->dislayer[layer[0]] == 0)
#endif
	tilemap_draw(bitmap, cliprect, state->tilemap[layer[0]], TILEMAP_DRAW_OPAQUE, 1);

#ifdef MAME_DEBUG
	if (state->dislayer[layer[1]] == 0)
#endif
	tilemap_draw(bitmap, cliprect, state->tilemap[layer[1]], 0, 2);

#ifdef MAME_DEBUG
	if (state->dislayer[layer[2]] == 0)
#endif
	tilemap_draw(bitmap, cliprect, state->tilemap[layer[2]], 0, 4);


	draw_sprites(screen->machine, bitmap, cliprect, 16);

#if 0
	{
//      char buf[80];
		sprintf(buf,"bg0_gfx: %04x bg1_gfx: %04x ", state->bg0_gfxset, state->bg1_gfxset);
		popmessage(buf);
	}
#endif

	return 0;
}
