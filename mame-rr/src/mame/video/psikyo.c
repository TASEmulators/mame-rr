/***************************************************************************

                            -= Psikyo Games =-

                driver by   Luca Elia (l.elia@tin.it)


Note:   if MAME_DEBUG is defined, pressing Z with:

        Q           shows layer 0
        W           shows layer 1
        A           shows the sprites

        Keys can be used together!


                            [ 2 Scrolling Layers ]

        - Dynamic Size
        - Line Scroll

        Layer Sizes:             512 x 2048 ( $20 x $80 tiles)
                                1024 x 1048 ( $40 x $40 tiles)
                                2048 x  512 ( $80 x $20 tiles)
                                4096 x  256 ($100 x $10 tiles)

        Tengai uses all four above

        Tiles:                  16x16x4
        Color Codes:            8


                    [ ~ $300 Multi-Tile Sprites With Zoom ]


        Each sprite is made of 16x16 tiles, up to 8x8 tiles.

        There are $300 sprites, followed by a list of the indexes
        of the sprites to actually display ($400 max). The list is
        terminated by the special index value FFFF.

        The tile code specified for a sprite is actually fed to a
        ROM holding a look-up table with the real tile code to display.

        Sprites can be shrinked up to ~50% following a linear curve of
        sizes.


        Since the tilemaps can change size its safest to allocate all
        the possible sizes at startup, as opposed to during the emulation

        By doing it this way theres no chance of a memory allocation
        failing during gameplay and crashing MAME

**************************************************************************/

#include "emu.h"
#include "includes/psikyo.h"


/***************************************************************************

                        Callbacks for the TileMap code

                              [ Tiles Format ]

Offset:

0000.w          fed- ---- ---- ----     Color
                ---c ba98 7654 3210     Code

***************************************************************************/

static TILE_GET_INFO( get_tile_info_0 )
{
	psikyo_state *state = (psikyo_state *)machine->driver_data;
	UINT16 code = ((UINT16 *)state->vram_0)[BYTE_XOR_BE(tile_index)];
	SET_TILE_INFO(
			1,
			(code & 0x1fff) + 0x2000 * state->tilemap_0_bank,
			(code >> 13) & 7,
			0);
}

static TILE_GET_INFO( get_tile_info_1 )
{
	psikyo_state *state = (psikyo_state *)machine->driver_data;
	UINT16 code = ((UINT16 *)state->vram_1)[BYTE_XOR_BE(tile_index)];
	SET_TILE_INFO(
			1,
			(code & 0x1fff) + 0x2000 * state->tilemap_1_bank,
			((code >> 13) & 7) + 0x40, // So we only have to decode the gfx once.
			0);
}


WRITE32_HANDLER( psikyo_vram_0_w )
{
	psikyo_state *state = (psikyo_state *)space->machine->driver_data;

	COMBINE_DATA(&state->vram_0[offset]);
	if (ACCESSING_BITS_16_31)
	{
		tilemap_mark_tile_dirty(state->tilemap_0_size0, offset * 2);
		tilemap_mark_tile_dirty(state->tilemap_0_size1, offset * 2);
		tilemap_mark_tile_dirty(state->tilemap_0_size2, offset * 2);
		tilemap_mark_tile_dirty(state->tilemap_0_size3, offset * 2);
	}

	if (ACCESSING_BITS_0_15)
	{
		tilemap_mark_tile_dirty(state->tilemap_0_size0, offset * 2 + 1);
		tilemap_mark_tile_dirty(state->tilemap_0_size1, offset * 2 + 1);
		tilemap_mark_tile_dirty(state->tilemap_0_size2, offset * 2 + 1);
		tilemap_mark_tile_dirty(state->tilemap_0_size3, offset * 2 + 1);
	}
}

WRITE32_HANDLER( psikyo_vram_1_w )
{
	psikyo_state *state = (psikyo_state *)space->machine->driver_data;

	COMBINE_DATA(&state->vram_1[offset]);
	if (ACCESSING_BITS_16_31)
	{
		tilemap_mark_tile_dirty(state->tilemap_1_size0, offset * 2);
		tilemap_mark_tile_dirty(state->tilemap_1_size1, offset * 2);
		tilemap_mark_tile_dirty(state->tilemap_1_size2, offset * 2);
		tilemap_mark_tile_dirty(state->tilemap_1_size3, offset * 2);
	}

	if (ACCESSING_BITS_0_15)
	{
		tilemap_mark_tile_dirty(state->tilemap_1_size0, offset * 2 + 1);
		tilemap_mark_tile_dirty(state->tilemap_1_size1, offset * 2 + 1);
		tilemap_mark_tile_dirty(state->tilemap_1_size2, offset * 2 + 1);
		tilemap_mark_tile_dirty(state->tilemap_1_size3, offset * 2 + 1);
	}
}

void psikyo_switch_banks( running_machine *machine, int tmap, int bank )
{
	psikyo_state *state = (psikyo_state *)machine->driver_data;

	if ((tmap == 0) && (bank != state->tilemap_0_bank))
	{
		state->tilemap_0_bank = bank;
		tilemap_mark_all_tiles_dirty(state->tilemap_0_size0);
		tilemap_mark_all_tiles_dirty(state->tilemap_0_size1);
		tilemap_mark_all_tiles_dirty(state->tilemap_0_size2);
		tilemap_mark_all_tiles_dirty(state->tilemap_0_size3);
	}
	else if ((tmap == 1) && (bank != state->tilemap_1_bank))
	{
		state->tilemap_1_bank = bank;
		tilemap_mark_all_tiles_dirty(state->tilemap_1_size0);
		tilemap_mark_all_tiles_dirty(state->tilemap_1_size1);
		tilemap_mark_all_tiles_dirty(state->tilemap_1_size2);
		tilemap_mark_all_tiles_dirty(state->tilemap_1_size3);
	}
}


VIDEO_START( psikyo )
{
	psikyo_state *state = (psikyo_state *)machine->driver_data;

	/* The Hardware is Capable of Changing the Dimensions of the Tilemaps, its safer to create
       the various sized tilemaps now as opposed to later */

	state->tilemap_0_size0 = tilemap_create(machine, get_tile_info_0, tilemap_scan_rows, 16, 16, 0x20, 0x80);
	state->tilemap_0_size1 = tilemap_create(machine, get_tile_info_0, tilemap_scan_rows, 16, 16, 0x40, 0x40);
	state->tilemap_0_size2 = tilemap_create(machine, get_tile_info_0, tilemap_scan_rows, 16, 16, 0x80, 0x20);
	state->tilemap_0_size3 = tilemap_create(machine, get_tile_info_0, tilemap_scan_rows, 16, 16, 0x100, 0x10);

	state->tilemap_1_size0 = tilemap_create(machine, get_tile_info_1, tilemap_scan_rows, 16, 16, 0x20, 0x80);
	state->tilemap_1_size1 = tilemap_create(machine, get_tile_info_1, tilemap_scan_rows, 16, 16, 0x40, 0x40);
	state->tilemap_1_size2 = tilemap_create(machine, get_tile_info_1, tilemap_scan_rows, 16, 16, 0x80, 0x20);
	state->tilemap_1_size3 = tilemap_create(machine, get_tile_info_1, tilemap_scan_rows, 16, 16, 0x100, 0x10);

	state->spritebuf1 = auto_alloc_array(machine, UINT32, 0x2000 / 4);
	state->spritebuf2 = auto_alloc_array(machine, UINT32, 0x2000 / 4);

	tilemap_set_scroll_rows(state->tilemap_0_size0, 0x80 * 16);	// line scrolling
	tilemap_set_scroll_cols(state->tilemap_0_size0, 1);

	tilemap_set_scroll_rows(state->tilemap_0_size1, 0x40 * 16);	// line scrolling
	tilemap_set_scroll_cols(state->tilemap_0_size1, 1);

	tilemap_set_scroll_rows(state->tilemap_0_size2, 0x20 * 16);	// line scrolling
	tilemap_set_scroll_cols(state->tilemap_0_size2, 1);

	tilemap_set_scroll_rows(state->tilemap_0_size3, 0x10 * 16);	// line scrolling
	tilemap_set_scroll_cols(state->tilemap_0_size3, 1);

	tilemap_set_scroll_rows(state->tilemap_1_size0, 0x80 * 16);	// line scrolling
	tilemap_set_scroll_cols(state->tilemap_1_size0, 1);

	tilemap_set_scroll_rows(state->tilemap_1_size1, 0x40 * 16);	// line scrolling
	tilemap_set_scroll_cols(state->tilemap_1_size1, 1);

	tilemap_set_scroll_rows(state->tilemap_1_size2, 0x20 * 16);	// line scrolling
	tilemap_set_scroll_cols(state->tilemap_1_size2, 1);

	tilemap_set_scroll_rows(state->tilemap_1_size3, 0x10 * 16);	// line scrolling
	tilemap_set_scroll_cols(state->tilemap_1_size3, 1);

	state_save_register_global_pointer(machine, state->spritebuf1, 0x2000 / 4);
	state_save_register_global_pointer(machine, state->spritebuf2, 0x2000 / 4);
}

VIDEO_START( sngkace )
{
	VIDEO_START_CALL( psikyo );

	psikyo_switch_banks(machine, 0, 0); // sngkace / samuraia don't use banking
	psikyo_switch_banks(machine, 1, 1); // They share "gfx2" to save memory on other boards
}



/***************************************************************************

                                Sprites Drawing

Offset:         Value:

0000/2.w        Y/X + Y/X Size

                    fedc ---- ---- ----     Zoom Y/X ???
                    ---- ba9- ---- ----     Tiles along Y/X
                    ---- ---8 7654 3210     Position


0004.w          Color + Flags

                    f--- ---- ---- ----     Flip Y
                    -e-- ---- ---- ----     Flip X
                    --d- ---- ---- ----     ? USED
                    ---c ba98 ---- ----     Color
                    ---- ---- 76-- ----     Priority
                    ---- ---- --54 321-     -
                    ---- ---- ---- ---0     Code High Bit


0006.w                                      Code Low Bits

                (Code goes into a LUT in ROM where
                 the real tile code is.)


Note:   Not all sprites are displayed: in the top part of spriteram
        (e.g. 401800-401fff) there's the list of sprites indexes to
        actually display, terminated by FFFF.

        The last entry (e.g. 401ffe) is special and holds some flags:

            fedc ba98 7654 ----
            ---- ---- ---- 32--     Transparent Pen select? 10 for 0xf, 01 for 0x0.
            ---- ---- ---- --1-
            ---- ---- ---- ---0     Sprites Disable


***************************************************************************/

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int trans_pen )
{
	psikyo_state *state = (psikyo_state *)machine->driver_data;

	/* tile layers 0 & 1 have priorities 1 & 2 */
	static const int pri[] = { 0, 0xfc, 0xff, 0xff };
	int offs;
	UINT16 *spritelist = (UINT16 *)(state->spritebuf2 + 0x1800 / 4);
	UINT8 *TILES = memory_region(machine, "spritelut");	// Sprites LUT
	int TILES_LEN = memory_region_length(machine, "spritelut");

	int width = machine->primary_screen->width();
	int height = machine->primary_screen->height();

	/* Exit if sprites are disabled */
	if (spritelist[BYTE_XOR_BE((0x800 - 2) / 2)] & 1)	return;

	/* Look for "end of sprites" marker in the sprites list */
	for (offs = 0/2 ; offs < (0x800 - 2)/2 ; offs += 2/2)	// skip last "sprite"
	{
		UINT16 sprite = spritelist[BYTE_XOR_BE(offs)];
		if (sprite == 0xffff)
			break;
	}

	offs -= 2/2;

	//  fprintf(stderr, "\n");
	for ( ; offs >= 0/2 ; offs -= 2/2)
	{
		UINT32 *source;
		int sprite;

		int x, y, attr, code, flipx, flipy, nx, ny, zoomx, zoomy;
		int dx, dy, xstart, ystart, xend, yend, xinc, yinc;

		/* Get next entry in the list */
		sprite = spritelist[BYTE_XOR_BE(offs)];

		sprite %= 0x300;
		source = &state->spritebuf2[sprite * 8 / 4];

		/* Draw this sprite */

		y	=	source[0 / 4] >> 16;
		x	=	source[0 / 4] & 0xffff;
		attr	=	source[4 / 4] >> 16;
		code	=	source[4 / 4] & 0x1ffff;

		flipx	=	attr & 0x4000;
		flipy	=	attr & 0x8000;

		zoomx	=	((x & 0xf000) >> 12);
		zoomy	=	((y & 0xf000) >> 12);
		nx	=	((x & 0x0e00) >> 9) + 1;
		ny	=	((y & 0x0e00) >> 9) + 1;
		x	=	((x & 0x01ff));
		y	=	((y & 0x00ff)) - (y & 0x100);

		/* 180-1ff are negative coordinates. Note that $80 pixels is
           the maximum extent of a sprite, which can therefore be moved
           out of screen without problems */
		if (x >= 0x180)
			x -= 0x200;

		x += (nx * zoomx + 2) / 4;
		y += (ny * zoomy + 2) / 4;

		zoomx = 32 - zoomx;
		zoomy = 32 - zoomy;

		if (flip_screen_get(machine))
		{
			x = width  - x - (nx * zoomx) / 2;
			y = height - y - (ny * zoomy) / 2;
			flipx = !flipx;
			flipy = !flipy;
		}

		if (flipx)	{ xstart = nx - 1;  xend = -1;  xinc = -1; }
		else		{ xstart = 0;       xend = nx;  xinc = +1; }

		if (flipy)	{ ystart = ny - 1;  yend = -1;   yinc = -1; }
		else		{ ystart = 0;       yend = ny;   yinc = +1; }

		for (dy = ystart; dy != yend; dy += yinc)
		{
			for (dx = xstart; dx != xend; dx += xinc)
			{
				int addr = (code * 2) & (TILES_LEN - 1);

				if (zoomx == 32 && zoomy == 32)
					pdrawgfx_transpen(bitmap,cliprect,machine->gfx[0],
							TILES[addr+1] * 256 + TILES[addr],
							attr >> 8,
							flipx, flipy,
							x + dx * 16, y + dy * 16,
							machine->priority_bitmap,
							pri[(attr & 0xc0) >> 6],trans_pen);
				else
					pdrawgfxzoom_transpen(bitmap,cliprect,machine->gfx[0],
								TILES[addr+1] * 256 + TILES[addr],
								attr >> 8,
								flipx, flipy,
								x + (dx * zoomx) / 2, y + (dy * zoomy) / 2,
								zoomx << 11,zoomy << 11,
								machine->priority_bitmap,pri[(attr & 0xc0) >> 6],trans_pen);

				code++;
			}
		}
	}
}


// for now this is the same as the above function
// until I work out why it makes a partial copy of the sprite list, and how best to apply it
// sprite placement of the explosion graphic seems incorrect compared to the original sets? (no / different zoom support?)
// it might be a problem with the actual bootleg
static void draw_sprites_bootleg( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int trans_pen )
{
	psikyo_state *state = (psikyo_state *)machine->driver_data;

	/* tile layers 0 & 1 have priorities 1 & 2 */
	static const int pri[] = { 0, 0xfc, 0xff, 0xff };
	int offs;

//  UINT16 *spritelist  =   (UINT16 *)(machine->generic.spriteram.u32 + 0x1800/4);
	UINT16 *spritelist = (UINT16 *)(state->spritebuf2 + 0x1800 / 4);

	UINT8 *TILES = memory_region(machine, "spritelut");	// Sprites LUT
	int TILES_LEN = memory_region_length(machine, "spritelut");

	int width = machine->primary_screen->width();
	int height = machine->primary_screen->height();

	/* Exit if sprites are disabled */
	if (spritelist[BYTE_XOR_BE((0x800 - 2) / 2)] & 1)
		return;

	/* Look for "end of sprites" marker in the sprites list */
	for (offs = 0/2 ; offs < (0x800 - 2)/2 ; offs += 2/2)	// skip last "sprite"
	{
		UINT16 sprite = spritelist[BYTE_XOR_BE(offs)];
		if (sprite == 0xffff)
			break;
	}

	offs -= 2/2;

	//  fprintf(stderr, "\n");
	for ( ; offs >= 0/2 ; offs -= 2/2)
	{
		UINT32 *source;
		int sprite;

		int x, y, attr, code, flipx, flipy, nx, ny, zoomx, zoomy;
		int dx, dy, xstart, ystart, xend, yend, xinc, yinc;

		/* Get next entry in the list */
		sprite = spritelist[BYTE_XOR_BE(offs)];

		sprite %= 0x300;
//      source = &state->bootleg_spritebuffer[sprite * 8 / 4];
		source = &state->spritebuf2[sprite * 8 / 4];

		/* Draw this sprite */

		y	=	source[0] >> 16;
		x	=	source[0] & 0xffff;
		attr	=	source[1] >> 16;
		code	=	source[1] & 0x1ffff;

		flipx	=	attr & 0x4000;
		flipy	=	attr & 0x8000;

		zoomx	=	((x & 0xf000) >> 12);
		zoomy	=	((y & 0xf000) >> 12);
		nx	=	((x & 0x0e00) >> 9) + 1;
		ny	=	((y & 0x0e00) >> 9) + 1;
		x	=	((x & 0x01ff));
		y	=	((y & 0x00ff)) - (y & 0x100);

		/* 180-1ff are negative coordinates. Note that $80 pixels is
           the maximum extent of a sprite, which can therefore be moved
           out of screen without problems */
		if (x >= 0x180)
			x -= 0x200;

		x += (nx * zoomx + 2) / 4;
		y += (ny * zoomy + 2) / 4;

		zoomx = 32 - zoomx;
		zoomy = 32 - zoomy;


		if (flip_screen_get(machine))
		{
			x = width  - x - (nx * zoomx) / 2;
			y = height - y - (ny * zoomy) / 2;
			flipx = !flipx;
			flipy = !flipy;
		}

		if (flipx)	{ xstart = nx - 1;  xend = -1;  xinc = -1; }
		else		{ xstart = 0;       xend = nx;  xinc = +1; }

		if (flipy)	{ ystart = ny - 1;  yend = -1;   yinc = -1; }
		else		{ ystart = 0;       yend = ny;   yinc = +1; }

		for (dy = ystart; dy != yend; dy += yinc)
		{
			for (dx = xstart; dx != xend; dx += xinc)
			{
				int addr = (code * 2) & (TILES_LEN-1);

				if (zoomx == 32 && zoomy == 32)
					pdrawgfx_transpen(bitmap,cliprect,machine->gfx[0],
							TILES[addr+1] * 256 + TILES[addr],
							attr >> 8,
							flipx, flipy,
							x + dx * 16, y + dy * 16,
							machine->priority_bitmap,
							pri[(attr & 0xc0) >> 6],trans_pen);
				else
					pdrawgfxzoom_transpen(bitmap,cliprect,machine->gfx[0],
								TILES[addr+1] * 256 + TILES[addr],
								attr >> 8,
								flipx, flipy,
								x + (dx * zoomx) / 2, y + (dy * zoomy) / 2,
								zoomx << 11,zoomy << 11,
								machine->priority_bitmap,pri[(attr & 0xc0) >> 6],trans_pen);

				code++;
			}
		}
	}
}





/***************************************************************************

                                Screen Drawing

***************************************************************************/

static int tilemap_width( int size )
{
	if (size == 0)
		return 0x80 * 16;
	else if(size == 1)
		return 0x40 * 16;
	else if(size == 2)
		return 0x20 * 16;
	else
		return 0x10 * 16;
}

VIDEO_UPDATE( psikyo )
{
	psikyo_state *state = (psikyo_state *)screen->machine->driver_data;
	int i, layers_ctrl = -1;

	UINT32 tm0size, tm1size;

	UINT32 layer0_scrollx, layer0_scrolly;
	UINT32 layer1_scrollx, layer1_scrolly;
	UINT32 layer0_ctrl = state->vregs[0x412 / 4];
	UINT32 layer1_ctrl = state->vregs[0x416 / 4];
	UINT32 spr_ctrl = state->spritebuf2[0x1ffe / 4];

	tilemap_t *tmptilemap0, *tmptilemap1;

	flip_screen_set(screen->machine, ~input_port_read(screen->machine, "DSW") & 0x00010000);		// hardwired to a DSW bit

	/* Layers enable (not quite right) */

	/* bit  0   : layer enable
            1   : opaque tiles (used in Gunbird attract mode)
            2   : ?
            3   : transparent colour (0 or 15)
            4- 5: ?
            6- 7: tilemap size
            8   : per-line rowscroll
            9   : per-tile rowscroll
           10   : tilebank (btlkroad/gunbird/s1945jn only)
           11-15: ? */

/*
    gunbird:    L:00d0-04d0 S:0008 (00e1 04e1 0009 or 00e2 04e2 000a, for a blink, on scene transitions)
    sngkace:    L:00d0-00d0 S:0008 (00d1 00d1 0009, for a blink, on scene transitions)
    s1945:      L:00d0-04d0 S:0008
    btlkrodj:   L:0120-0510 S:0008 (0121 0511 0009, for a blink, on scene transitions)
    tengai: L:0178-0508 S:0004 <-- Transpen is 0 as opposed to 15.

    tengai:
        L:01f8-05c8, 1 needs size 0, 2 needs size 0 Title
        L:00f8-05c8, 1 needs size 0, 2 needs size 0 No RowScroll on layer 0
        L:01b8-05c8, 1 needs size 3, 2 needs size 0
        L:0178-0508, 1 needs size ?, 2 needs size 1 Psikyo logo
        L:0178-0508, 1 needs size 2, 2 needs size 1 Intro
        L:0178-0548, 1 needs size 2, 2 needs size ? Test
        L:0178-0588,                 2 needs size 3 More Intro
*/

	/* For gfx banking for s1945jn/gunbird/btlkroad */
	if (state->ka302c_banking)
	{
		psikyo_switch_banks(screen->machine, 0, (layer0_ctrl & 0x400) >> 10);
		psikyo_switch_banks(screen->machine, 1, (layer1_ctrl & 0x400) >> 10);
	}

	switch ((layer0_ctrl & 0x00c0) >> 6)
	{
	case 0:	tm0size = 1;	break;
	case 1:	tm0size = 2;	break;
	case 2:	tm0size = 3;	break;
	default:	tm0size = 0;	break;
	}

	switch ((layer1_ctrl & 0x00c0) >> 6)
	{
	case 0:	tm1size = 1;	break;
	case 1:	tm1size = 2;	break;
	case 2:	tm1size = 3;	break;
	default:	tm1size = 0;	break;
	}

	if (tm0size == 0)
		tmptilemap0 = state->tilemap_0_size0;
	else if (tm0size == 1)
		tmptilemap0 = state->tilemap_0_size1;
	else if (tm0size == 2)
		tmptilemap0 = state->tilemap_0_size2;
	else
		tmptilemap0 = state->tilemap_0_size3;

	if (tm1size == 0)
		tmptilemap1 = state->tilemap_1_size0;
	else if (tm1size == 1)
		tmptilemap1 = state->tilemap_1_size1;
	else if (tm1size == 2)
		tmptilemap1 = state->tilemap_1_size2;
	else
		tmptilemap1 = state->tilemap_1_size3;

	tilemap_set_enable(tmptilemap0, ~layer0_ctrl & 1);
	tilemap_set_enable(tmptilemap1, ~layer1_ctrl & 1);

	/* Layers scrolling */

	layer0_scrolly = state->vregs[0x402 / 4];
	layer0_scrollx = state->vregs[0x406 / 4];
	layer1_scrolly = state->vregs[0x40a / 4];
	layer1_scrollx = state->vregs[0x40e / 4];

	tilemap_set_scrolly(tmptilemap0, 0, layer0_scrolly);

	tilemap_set_scrolly(tmptilemap1, 0, layer1_scrolly);

	for (i = 0; i < 256; i++)	/* 256 screen lines */
	{
		int x0 = 0, x1 = 0;

		/* layer 0 */
		if (layer0_ctrl & 0x0300)
		{
			if (layer0_ctrl & 0x0200)
				/* per-tile rowscroll */
				x0 = ((UINT16 *)state->vregs)[BYTE_XOR_BE(0x000/2 + i/16)];
			else
				/* per-line rowscroll */
				x0 = ((UINT16 *)state->vregs)[BYTE_XOR_BE(0x000/2 + i)];
		}

		tilemap_set_scrollx(
			tmptilemap0,
			(i + layer0_scrolly) % tilemap_width(tm0size),
			layer0_scrollx + x0 );


		/* layer 1 */
		if (layer1_ctrl & 0x0300)
		{
			if (layer1_ctrl & 0x0200)
				/* per-tile rowscroll */
				x1 = ((UINT16 *)state->vregs)[BYTE_XOR_BE(0x200/2 + i/16)];
			else
				/* per-line rowscroll */
				x1 = ((UINT16 *)state->vregs)[BYTE_XOR_BE(0x200/2 + i)];
		}

		tilemap_set_scrollx(
			tmptilemap1,
			(i + layer1_scrolly) % tilemap_width(tm1size),
			layer1_scrollx + x1 );
	}

	tilemap_set_transparent_pen(state->tilemap_0_size0, (layer0_ctrl & 8 ? 0 : 15));
	tilemap_set_transparent_pen(state->tilemap_0_size1, (layer0_ctrl & 8 ? 0 : 15));
	tilemap_set_transparent_pen(state->tilemap_0_size2, (layer0_ctrl & 8 ? 0 : 15));
	tilemap_set_transparent_pen(state->tilemap_0_size3, (layer0_ctrl & 8 ? 0 : 15));

	tilemap_set_transparent_pen(state->tilemap_1_size0, (layer1_ctrl & 8 ? 0 : 15));
	tilemap_set_transparent_pen(state->tilemap_1_size1, (layer1_ctrl & 8 ? 0 : 15));
	tilemap_set_transparent_pen(state->tilemap_1_size2, (layer1_ctrl & 8 ? 0 : 15));
	tilemap_set_transparent_pen(state->tilemap_1_size3, (layer1_ctrl & 8 ? 0 : 15));

	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));

	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

	if (layers_ctrl & 1)
		tilemap_draw(bitmap, cliprect, tmptilemap0, layer0_ctrl & 2 ? TILEMAP_DRAW_OPAQUE : 0, 1);

	if (layers_ctrl & 2)
		tilemap_draw(bitmap, cliprect, tmptilemap1, layer1_ctrl & 2 ? TILEMAP_DRAW_OPAQUE : 0, 2);

	if (layers_ctrl & 4)
		draw_sprites(screen->machine, bitmap, cliprect, (spr_ctrl & 4 ? 0 : 15));

	return 0;
}

/* todo: work out why the sprites flicker,
  if it misses a frame due to slowdown it wipes both the list
  and the extra buffer the bootleg has

  layer offsets should also differ?

*/

VIDEO_UPDATE( psikyo_bootleg )
{
	psikyo_state *state = (psikyo_state *)screen->machine->driver_data;
	int i, layers_ctrl = -1;

	UINT32 tm0size, tm1size;

	UINT32 layer0_scrollx, layer0_scrolly;
	UINT32 layer1_scrollx, layer1_scrolly;
	UINT32 layer0_ctrl = state->vregs[0x412 / 4];
	UINT32 layer1_ctrl = state->vregs[0x416 / 4];
	UINT32 spr_ctrl = state->spritebuf2[0x1ffe / 4];

	tilemap_t *tmptilemap0, *tmptilemap1;

	flip_screen_set(screen->machine, ~input_port_read(screen->machine, "DSW") & 0x00010000);		// hardwired to a DSW bit

	/* Layers enable (not quite right) */

	/* bit  0   : layer enable
            1   : opaque tiles (used in Gunbird attract mode)
            2   : ?
            3   : transparent colour (0 or 15)
            4- 5: ?
            6- 7: tilemap size
            8   : per-line rowscroll
            9   : per-tile rowscroll
           10   : tilebank (btlkroad/gunbird/s1945jn only)
           11-15: ? */

/*
    gunbird:    L:00d0-04d0 S:0008 (00e1 04e1 0009 or 00e2 04e2 000a, for a blink, on scene transitions)
    sngkace:    L:00d0-00d0 S:0008 (00d1 00d1 0009, for a blink, on scene transitions)
    s1945:      L:00d0-04d0 S:0008
    btlkrodj:   L:0120-0510 S:0008 (0121 0511 0009, for a blink, on scene transitions)
    tengai: L:0178-0508 S:0004 <-- Transpen is 0 as opposed to 15.

    tengai:
        L:01f8-05c8, 1 needs size 0, 2 needs size 0 Title
        L:00f8-05c8, 1 needs size 0, 2 needs size 0 No RowScroll on layer 0
        L:01b8-05c8, 1 needs size 3, 2 needs size 0
        L:0178-0508, 1 needs size ?, 2 needs size 1 Psikyo logo
        L:0178-0508, 1 needs size 2, 2 needs size 1 Intro
        L:0178-0548, 1 needs size 2, 2 needs size ? Test
        L:0178-0588,                 2 needs size 3 More Intro
*/

	/* For gfx banking for s1945jn/gunbird/btlkroad */
	if (state->ka302c_banking)
	{
		psikyo_switch_banks(screen->machine, 0, (layer0_ctrl & 0x400) >> 10);
		psikyo_switch_banks(screen->machine, 1, (layer1_ctrl & 0x400) >> 10);
	}

	switch ((layer0_ctrl & 0x00c0) >> 6)
	{
	case 0:	tm0size = 1;	break;
	case 1:	tm0size = 2;	break;
	case 2:	tm0size = 3;	break;
	default:	tm0size = 0;	break;
	}

	switch ((layer1_ctrl & 0x00c0) >> 6)
	{
	case 0:	tm1size = 1;	break;
	case 1:	tm1size = 2;	break;
	case 2:	tm1size = 3;	break;
	default:	tm1size = 0;	break;
	}

	if (tm0size == 0)
		tmptilemap0 = state->tilemap_0_size0;
	else if (tm0size == 1)
		tmptilemap0 = state->tilemap_0_size1;
	else if (tm0size == 2)
		tmptilemap0 = state->tilemap_0_size2;
	else
		tmptilemap0 = state->tilemap_0_size3;

	if (tm1size == 0)
		tmptilemap1 = state->tilemap_1_size0;
	else if (tm1size == 1)
		tmptilemap1 = state->tilemap_1_size1;
	else if (tm1size == 2)
		tmptilemap1 = state->tilemap_1_size2;
	else
		tmptilemap1 = state->tilemap_1_size3;

	tilemap_set_enable(tmptilemap0, ~layer0_ctrl & 1);
	tilemap_set_enable(tmptilemap1, ~layer1_ctrl & 1);

	/* Layers scrolling */

	layer0_scrolly = state->vregs[0x402 / 4];
	layer0_scrollx = state->vregs[0x406 / 4];
	layer1_scrolly = state->vregs[0x40a / 4];
	layer1_scrollx = state->vregs[0x40e / 4];

	tilemap_set_scrolly(tmptilemap0, 0, layer0_scrolly);

	tilemap_set_scrolly(tmptilemap1, 0, layer1_scrolly);

	for (i = 0; i < 256; i++)	/* 256 screen lines */
	{
		int x0 = 0, x1 = 0;

		/* layer 0 */
		if (layer0_ctrl & 0x0300)
		{
			if (layer0_ctrl & 0x0200)
				/* per-tile rowscroll */
				x0 = ((UINT16 *)state->vregs)[BYTE_XOR_BE(0x000/2 + i/16)];
			else
				/* per-line rowscroll */
				x0 = ((UINT16 *)state->vregs)[BYTE_XOR_BE(0x000/2 + i)];
		}

		tilemap_set_scrollx(
			tmptilemap0,
			(i + layer0_scrolly) % tilemap_width(tm0size),
			layer0_scrollx + x0 );


		/* layer 1 */
		if (layer1_ctrl & 0x0300)
		{
			if (layer1_ctrl & 0x0200)
				/* per-tile rowscroll */
				x1 = ((UINT16 *)state->vregs)[BYTE_XOR_BE(0x200/2 + i/16)];
			else
				/* per-line rowscroll */
				x1 = ((UINT16 *)state->vregs)[BYTE_XOR_BE(0x200/2 + i)];
		}

		tilemap_set_scrollx(
			tmptilemap1,
			(i + layer1_scrolly) % tilemap_width(tm1size),
			layer1_scrollx + x1 );
	}

	tilemap_set_transparent_pen(state->tilemap_0_size0, (layer0_ctrl & 8 ? 0 : 15));
	tilemap_set_transparent_pen(state->tilemap_0_size1, (layer0_ctrl & 8 ? 0 : 15));
	tilemap_set_transparent_pen(state->tilemap_0_size2, (layer0_ctrl & 8 ? 0 : 15));
	tilemap_set_transparent_pen(state->tilemap_0_size3, (layer0_ctrl & 8 ? 0 : 15));

	tilemap_set_transparent_pen(state->tilemap_1_size0, (layer1_ctrl & 8 ? 0 : 15));
	tilemap_set_transparent_pen(state->tilemap_1_size1, (layer1_ctrl & 8 ? 0 : 15));
	tilemap_set_transparent_pen(state->tilemap_1_size2, (layer1_ctrl & 8 ? 0 : 15));
	tilemap_set_transparent_pen(state->tilemap_1_size3, (layer1_ctrl & 8 ? 0 : 15));

	bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine));

	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

	if (layers_ctrl & 1)
		tilemap_draw(bitmap, cliprect, tmptilemap0, layer0_ctrl & 2 ? TILEMAP_DRAW_OPAQUE : 0, 1);

	if (layers_ctrl & 2)
		tilemap_draw(bitmap, cliprect, tmptilemap1, layer1_ctrl & 2 ? TILEMAP_DRAW_OPAQUE : 0, 2);

	if (layers_ctrl & 4)
		draw_sprites_bootleg(screen->machine, bitmap, cliprect, (spr_ctrl & 4 ? 0 : 15));

	return 0;
}


VIDEO_EOF( psikyo )
{
	psikyo_state *state = (psikyo_state *)machine->driver_data;
	memcpy(state->spritebuf2, state->spritebuf1, 0x2000);
	memcpy(state->spritebuf1, machine->generic.spriteram.u32, 0x2000);
}
