/***************************************************************************

                          -= Fuuki 32 Bit Games (FG-3) =-

                driver by Paul Priest and David Haywood
                based on fuukifg2 by Luca Elia


    [ 4 Scrolling Layers ]

                            [ Layer 0 ]     [ Layer 1 ]     [ Layers 2&3 (double-buffered) ]

    Tile Size:              16 x 16 x 8     16 x 16 x 8     8 x 8 x 4
    Layer Size (tiles):     64 x 32         64 x 32         64 x 32

    [ 1024? Zooming Sprites ]

    Sprites are made of 16 x 16 x 4 tiles. Size can vary from 1 to 16
    tiles both horizontally and vertically.
    There is zooming (from full size to half size) and 4 levels of
    priority (wrt layers)

    Per-line raster effects used on many stages
    Sprites buffered by two frames
    Tilebank buffered by 3 frames? Only 2 in attract
    Sprite pens needs to be buffered by 3 frames? Or lazy programming? Probably 2

***************************************************************************/

#include "emu.h"
#include "includes/fuukifg3.h"


/***************************************************************************


                                    Tilemaps

    Offset:     Bits:                   Value:

        0.w                             Code

        2.w     fedc ba98 ---- ----
                ---- ---- 7--- ----     Flip Y
                ---- ---- -6-- ----     Flip X
                ---- ---- --54 3210     Color


***************************************************************************/

#define LAYER_8BPP( _N_ ) \
\
static TILE_GET_INFO( get_tile_info_##_N_ ) \
{ \
	fuuki32_state *state = (fuuki32_state *)machine->driver_data; \
	UINT16 code = (state->vram_##_N_[tile_index] & 0xffff0000) >> 16; \
	UINT16 attr = (state->vram_##_N_[tile_index] & 0x0000ffff); \
	SET_TILE_INFO(1 + _N_, code, (attr & 0x3f) >> 4, TILE_FLIPYX((attr >> 6) & 3)); \
} \
\
WRITE32_HANDLER( fuuki32_vram_##_N_##_w ) \
{ \
	fuuki32_state *state = (fuuki32_state *)space->machine->driver_data; \
	COMBINE_DATA(&state->vram_##_N_[offset]); \
	tilemap_mark_tile_dirty(state->tilemap_##_N_,offset); \
}

#define LAYER_4BPP( _N_ ) \
\
static TILE_GET_INFO( get_tile_info_##_N_ ) \
{ \
	fuuki32_state *state = (fuuki32_state *)machine->driver_data; \
	UINT16 code = (state->vram_##_N_[tile_index] & 0xffff0000) >> 16; \
	UINT16 attr = (state->vram_##_N_[tile_index] & 0x0000ffff); \
	SET_TILE_INFO(1 + _N_, code, attr & 0x3f, TILE_FLIPYX((attr >> 6) & 3)); \
} \
\
WRITE32_HANDLER( fuuki32_vram_##_N_##_w ) \
{ \
	fuuki32_state *state = (fuuki32_state *)space->machine->driver_data; \
	COMBINE_DATA(&state->vram_##_N_[offset]); \
	tilemap_mark_tile_dirty(state->tilemap_##_N_, offset); \
}


LAYER_8BPP( 0 )
LAYER_8BPP( 1 )
LAYER_4BPP( 2 )
LAYER_4BPP( 3 )


/***************************************************************************


                            Video Hardware Init


***************************************************************************/

VIDEO_START( fuuki32 )
{
	fuuki32_state *state = (fuuki32_state *)machine->driver_data;
	state->buf_spriteram = auto_alloc_array(machine, UINT32, state->spriteram_size / 4);
	state->buf_spriteram2 = auto_alloc_array(machine, UINT32, state->spriteram_size / 4);

	state_save_register_global_pointer(machine, state->buf_spriteram, state->spriteram_size / 4);
	state_save_register_global_pointer(machine, state->buf_spriteram2, state->spriteram_size / 4);

	state->tilemap_0 = tilemap_create(machine, get_tile_info_0, tilemap_scan_rows, 16, 16, 64, 32);
	state->tilemap_1 = tilemap_create(machine, get_tile_info_1, tilemap_scan_rows, 16, 16, 64, 32);
	state->tilemap_2 = tilemap_create(machine, get_tile_info_2, tilemap_scan_rows, 8, 8, 64, 32);
	state->tilemap_3 = tilemap_create(machine, get_tile_info_3, tilemap_scan_rows, 8, 8, 64, 32);

	tilemap_set_transparent_pen(state->tilemap_0, 0xff);	// 8 bits
	tilemap_set_transparent_pen(state->tilemap_1, 0xff);	// 8 bits
	tilemap_set_transparent_pen(state->tilemap_2, 0x0f);	// 4 bits
	tilemap_set_transparent_pen(state->tilemap_3, 0x0f);	// 4 bits

	//machine->gfx[1]->color_granularity = 16; /* 256 colour tiles with palette selectable on 16 colour boundaries */
	//machine->gfx[2]->color_granularity = 16;
}


/***************************************************************************


                                Sprites Drawing

    Offset:     Bits:                   Value:

        0.w     fedc ---- ---- ----     Number Of Tiles Along X - 1
                ---- b--- ---- ----     Flip X
                ---- -a-- ---- ----     1 = Don't Draw This Sprite
                ---- --98 7654 3210     X (Signed)

        2.w     fedc ---- ---- ----     Number Of Tiles Along Y - 1
                ---- b--- ---- ----     Flip Y
                ---- -a-- ---- ----
                ---- --98 7654 3210     Y (Signed)

        4.w     fedc ---- ---- ----     Zoom X ($0 = Full Size, $F = Half Size)
                ---- ba98 ---- ----     Zoom Y ""
                ---- ---- 76-- ----     Priority
                ---- ---- --54 3210     Color

        6.w     fe-- ---- ---- ----     Tile Bank
                --dc ba98 7654 3210     Tile Code


***************************************************************************/

static void draw_sprites( screen_device &screen, bitmap_t *bitmap, const rectangle *cliprect )
{
	fuuki32_state *state = (fuuki32_state *)screen.machine->driver_data;
	int offs;
	const gfx_element *gfx = screen.machine->gfx[0];
	bitmap_t *priority_bitmap = screen.machine->priority_bitmap;
	const rectangle &visarea = screen.visible_area();
	int max_x =	visarea.max_x + 1;
	int max_y =	visarea.max_y + 1;

	UINT32 *src = state->buf_spriteram2; /* Use spriteram buffered by 2 frames, need palette buffered by one frame? */

	/* Draw them backwards, for pdrawgfx */
	for (offs = (state->spriteram_size - 8) / 4; offs >= 0; offs -= 8/4)
	{
		int x, y, xstart, ystart, xend, yend, xinc, yinc;
		int xnum, ynum, xzoom, yzoom, flipx, flipy;
		int pri_mask;

		int sx = (src[offs + 0]& 0xffff0000) >> 16;
		int sy = (src[offs + 0]& 0x0000ffff);
		int attr = (src[offs + 1]& 0xffff0000) >> 16;
		int code = (src[offs + 1]& 0x0000ffff);

		int bank = (code & 0xc000) >> 14;
		int bank_lookedup;

		bank_lookedup = ((state->spr_buffered_tilebank[1] & 0xffff0000) >> (16 + bank * 4)) & 0xf;
		code &= 0x3fff;
		code += bank_lookedup * 0x4000;

		if (sx & 0x400)
			continue;

		flipx = sx & 0x0800;
		flipy = sy & 0x0800;

		xnum = ((sx >> 12) & 0xf) + 1;
		ynum = ((sy >> 12) & 0xf) + 1;

		xzoom = 16 * 8 - (8 * ((attr >> 12) & 0xf)) / 2;
		yzoom = 16 * 8 - (8 * ((attr >>  8) & 0xf)) / 2;

		switch( (attr >> 6) & 3 )
		{
			case 3:	pri_mask = 0xf0 | 0xcc | 0xaa;	break;	// behind all layers
			case 2:	pri_mask = 0xf0 | 0xcc;			break;	// behind fg + middle layer
			case 1:	pri_mask = 0xf0;				break;	// behind fg layer
			case 0:
			default:	pri_mask = 0;						// above all
		}

		sx = (sx & 0x1ff) - (sx & 0x200);
		sy = (sy & 0x1ff) - (sy & 0x200);

		if (flip_screen_get(screen.machine))
		{
			flipx = !flipx;		sx = max_x - sx - xnum * 16;
			flipy = !flipy;		sy = max_y - sy - ynum * 16;
		}

		if (flipx)	{ xstart = xnum-1;  xend = -1;    xinc = -1; }
		else		{ xstart = 0;       xend = xnum;  xinc = +1; }

		if (flipy)	{ ystart = ynum-1;  yend = -1;    yinc = -1; }
		else		{ ystart = 0;       yend = ynum;  yinc = +1; }

#if 0
		if(!( (input_code_pressed(screen.machine, KEYCODE_V) && (((attr >> 6)&3) == 0))
		   || (input_code_pressed(screen.machine, KEYCODE_B) && (((attr >> 6)&3) == 1))
		   || (input_code_pressed(screen.machine, KEYCODE_N) && (((attr >> 6)&3) == 2))
		   || (input_code_pressed(screen.machine, KEYCODE_M) && (((attr >> 6)&3) == 3))
		   ))
#endif

		for (y = ystart; y != yend; y += yinc)
		{
			for (x = xstart; x != xend; x += xinc)
			{
				if (xzoom == (16*8) && yzoom == (16*8))
					pdrawgfx_transpen(bitmap,cliprect,gfx,
									code++,
									attr & 0x3f,
									flipx, flipy,
									sx + x * 16, sy + y * 16,
									priority_bitmap,
									pri_mask,15	);
				else
					pdrawgfxzoom_transpen(bitmap,cliprect,gfx,
									code++,
									attr & 0x3f,
									flipx, flipy,
									sx + (x * xzoom) / 8, sy + (y * yzoom) / 8,
									(0x10000/0x10/8) * (xzoom + 8),(0x10000/0x10/8) * (yzoom + 8),	priority_bitmap,// nearest greater integer value to avoid holes
									pri_mask,15	);
			}
		}

#ifdef MAME_DEBUG
#if 0
if (input_code_pressed(screen.machine, KEYCODE_X))
{	/* Display some info on each sprite */
	char buf[40];
	sprintf(buf, "%Xx%X %X",xnum,ynum,(attr>>6)&3);
	ui_draw_text(buf, sx, sy);
}
#endif
#endif
	}
}


/***************************************************************************


                                Screen Drawing

    Video Registers (fuuki32_vregs):

        00.w        Layer 0 Scroll Y
        02.w        Layer 0 Scroll X
        04.w        Layer 1 Scroll Y
        06.w        Layer 1 Scroll X
        08.w        Layer 2 Scroll Y
        0a.w        Layer 2 Scroll X
        0c.w        Layers Y Offset
        0e.w        Layers X Offset

        10-1a.w     ? 0
        1c.w        Trigger a level 5 irq on this raster line
        1e.w        ? $3390/$3393 (Flip Screen Off/On), $0040 is buffer for tilemap 2 or 3

    Priority Register (fuuki32_priority):

        fedc ba98 7654 3---
        ---- ---- ---- -210     Layer Order


    Unknown Registers ($de0000.l):

        00.w        ? $0200/$0201   (Flip Screen Off/On)
        02.w        ? $f300/$0330

***************************************************************************/

/* Wrapper to handle bg and bg2 ttogether */
static void fuuki32_draw_layer( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int i, int flag, int pri )
{
	fuuki32_state *state = (fuuki32_state *)machine->driver_data;
	int buffer = ((state->vregs[0x1e / 4] & 0x0000ffff) & 0x40);

	switch( i )
	{
		case 2:	if (buffer)	tilemap_draw(bitmap, cliprect, state->tilemap_3, flag, pri);
				else		tilemap_draw(bitmap, cliprect, state->tilemap_2, flag, pri);
				return;
		case 1:	tilemap_draw(bitmap, cliprect, state->tilemap_1, flag, pri);
				return;
		case 0:	tilemap_draw(bitmap, cliprect, state->tilemap_0, flag, pri);
				return;
	}
}

VIDEO_UPDATE( fuuki32 )
{
	fuuki32_state *state = (fuuki32_state *)screen->machine->driver_data;
	UINT16 layer0_scrollx, layer0_scrolly;
	UINT16 layer1_scrollx, layer1_scrolly;
	UINT16 layer2_scrollx, layer2_scrolly;
	UINT16 scrollx_offs,   scrolly_offs;

	/*
    It's not independent bits causing layers to switch, that wouldn't make sense with 3 bits.
    */

	static const int pri_table[6][3] = {
		{ 0, 1, 2 }, // Special moves 0>1, 0>2 (0,1,2 or 0,2,1)
		{ 0, 2, 1 }, // Two Levels - 0>1 (0,1,2 or 0,2,1 or 2,0,1)
		{ 1, 0, 2 }, // Most Levels - 2>1 1>0 2>0 (1,0,2)
		{ 1, 2, 0 }, // Not used?
		{ 2, 0, 1 }, // Title etc. - 0>1 (0,1,2 or 0,2,1 or 2,0,1)
		{ 2, 1, 0 }}; // Char Select, prison stage 1>0 (leaves 1,2,0 or 2,1,0)

	int tm_front  = pri_table[(state->priority[0] >> 16) & 0x0f][0];
	int tm_middle = pri_table[(state->priority[0] >> 16) & 0x0f][1];
	int tm_back   = pri_table[(state->priority[0] >> 16) & 0x0f][2];

	flip_screen_set(screen->machine, (state->vregs[0x1e / 4] & 0x0000ffff) & 1);

	/* Layers scrolling */

	scrolly_offs = ((state->vregs[0xc / 4] & 0xffff0000) >> 16) - (flip_screen_get(screen->machine) ? 0x103 : 0x1f3);
	scrollx_offs =  (state->vregs[0xc / 4] & 0x0000ffff) - (flip_screen_get(screen->machine) ? 0x2c7 : 0x3f6);

	layer0_scrolly = ((state->vregs[0x0 / 4] & 0xffff0000) >> 16) + scrolly_offs;
	layer0_scrollx = ((state->vregs[0x0 / 4] & 0x0000ffff)) + scrollx_offs;
	layer1_scrolly = ((state->vregs[0x4 / 4] & 0xffff0000) >> 16) + scrolly_offs;
	layer1_scrollx = ((state->vregs[0x4 / 4] & 0x0000ffff)) + scrollx_offs;

	layer2_scrolly = ((state->vregs[0x8 / 4] & 0xffff0000) >> 16);
	layer2_scrollx = ((state->vregs[0x8 / 4] & 0x0000ffff));

	tilemap_set_scrollx(state->tilemap_0, 0, layer0_scrollx);
	tilemap_set_scrolly(state->tilemap_0, 0, layer0_scrolly);
	tilemap_set_scrollx(state->tilemap_1, 0, layer1_scrollx);
	tilemap_set_scrolly(state->tilemap_1, 0, layer1_scrolly);

	tilemap_set_scrollx(state->tilemap_2, 0, layer2_scrollx);
	tilemap_set_scrolly(state->tilemap_2, 0, layer2_scrolly);
	tilemap_set_scrollx(state->tilemap_3, 0, layer2_scrollx);
	tilemap_set_scrolly(state->tilemap_3, 0, layer2_scrolly);

	/* The bg colour is the last pen i.e. 0x1fff */
	bitmap_fill(bitmap, cliprect, (0x800 * 4) - 1);
	bitmap_fill(screen->machine->priority_bitmap,cliprect,0);

	fuuki32_draw_layer(screen->machine, bitmap, cliprect, tm_back,   0, 1);
	fuuki32_draw_layer(screen->machine, bitmap, cliprect, tm_middle, 0, 2);
	fuuki32_draw_layer(screen->machine, bitmap, cliprect, tm_front,  0, 4);

	draw_sprites(*screen, bitmap, cliprect);
	return 0;
}

VIDEO_EOF( fuuki32 )
{
	fuuki32_state *state = (fuuki32_state *)machine->driver_data;

	/* Buffer sprites and tilebank by 2 frames */
	state->spr_buffered_tilebank[1] = state->spr_buffered_tilebank[0];
	state->spr_buffered_tilebank[0] = state->tilebank[0];
	memcpy(state->buf_spriteram2, state->buf_spriteram, state->spriteram_size);
	memcpy(state->buf_spriteram, state->spriteram, state->spriteram_size);
}
