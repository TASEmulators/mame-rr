/**************************************************************************

                            Ginga NinkyouDen
                            (C) 1987 Jaleco

                    driver by Luca Elia (l.elia@tin.it)


Note:   if MAME_DEBUG is defined, pressing Z with:

        Q       shows background
        W       shows foreground
        E       shows frontmost (text) layer
        A       shows sprites

        Keys can be used togheter!


[Screen]
    Visible Size:       256H x 240V
    Dynamic Colors:     256 x 4
    Color Space:        16R x 16G x 16B

[Scrolling layers]
    Format (all layers):    Offset:     0x400    0x000
                            Bit:        fedc---- --------   Color
                                        ----ba98 76543210   Code

    [Background]
        Size:               8192 x 512  (static: stored in ROM)
        Scrolling:          X,Y         (registers: $60006.w, $60004.w)
        Tiles Size:         16 x 16
        Tiles Number:       $400
        Colors:             $300-$3ff

    [Foreground]
        Size:               4096 x 512
        Scrolling:          X,Y         (registers: $60002.w, $60000.w)
        Tiles Size:         16 x 16
        Tiles Number:       $400
        Colors:             $200-$2ff

    [Frontmost]
        Size:               256 x 256
        Scrolling:          -
        Tiles Size:         8 x 8
        Tiles Number:       $200
        Colors:             $000-$0ff


[Sprites]
    On Screen:          256
    In ROM:             $a00
    Colors:             $100-$1ff
    Format:             See Below


**************************************************************************/

#include "emu.h"
#include "includes/ginganin.h"


/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/


/* Background - Resides in ROM */

#define BG_GFX (0)
#define BG_NX  (16*32)
#define BG_NY  (16*2)

static TILE_GET_INFO( get_bg_tile_info )
{
	UINT8 *gfx = memory_region(machine, "gfx5");
	int code = gfx[2 * tile_index + 0] * 256 + gfx[2 * tile_index + 1];
	SET_TILE_INFO(
			BG_GFX,
			code,
			code >> 12,
			0);
}


/* Foreground - Resides in RAM */

#define FG_GFX (1)
#define FG_NX  (16*16)
#define FG_NY  (16*2)

static TILE_GET_INFO( get_fg_tile_info )
{
	ginganin_state *state = (ginganin_state *)machine->driver_data;
	UINT16 code = state->fgram[tile_index];
	SET_TILE_INFO(
			FG_GFX,
			code,
			code >> 12,
			0);
}

WRITE16_HANDLER( ginganin_fgram16_w )
{
	ginganin_state *state = (ginganin_state *)space->machine->driver_data;
	COMBINE_DATA(&state->fgram[offset]);
	tilemap_mark_tile_dirty(state->fg_tilemap, offset);
}


/* Frontmost (text) Layer - Resides in RAM */

#define TXT_GFX (2)
#define TXT_NX	(32)
#define TXT_NY	(32)

static TILE_GET_INFO( get_txt_tile_info )
{
	ginganin_state *state = (ginganin_state *)machine->driver_data;
	UINT16 code = state->txtram[tile_index];
	SET_TILE_INFO(
			TXT_GFX,
			code,
			code >> 12,
			0);
}

WRITE16_HANDLER( ginganin_txtram16_w )
{
	ginganin_state *state = (ginganin_state *)space->machine->driver_data;
	COMBINE_DATA(&state->txtram[offset]);
	tilemap_mark_tile_dirty(state->tx_tilemap, offset);
}


VIDEO_START( ginganin )
{
	ginganin_state *state = (ginganin_state *)machine->driver_data;
	state->bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_cols, 16, 16, BG_NX, BG_NY);
	state->fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_cols, 16, 16, FG_NX, FG_NY);
	state->tx_tilemap = tilemap_create(machine, get_txt_tile_info, tilemap_scan_rows, 8, 8, TXT_NX, TXT_NY);

	tilemap_set_transparent_pen(state->fg_tilemap, 15);
	tilemap_set_transparent_pen(state->tx_tilemap, 15);
}


WRITE16_HANDLER( ginganin_vregs16_w )
{
	ginganin_state *state = (ginganin_state *)space->machine->driver_data;
	COMBINE_DATA(&state->vregs[offset]);
	data = state->vregs[offset];

	switch (offset)
	{
	case 0:
		tilemap_set_scrolly(state->fg_tilemap, 0, data);
		break;
	case 1:
		tilemap_set_scrollx(state->fg_tilemap, 0, data);
		break;
	case 2:
		tilemap_set_scrolly(state->bg_tilemap, 0, data);
		break;
	case 3:
		tilemap_set_scrollx(state->bg_tilemap, 0, data);
		break;
	case 4:
		state->layers_ctrl = data;
		break;
/*  case 5:
 *      break;
 */
	case 6:
		state->flipscreen = !(data & 1);
		tilemap_set_flip_all(space->machine, state->flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
		break;
	case 7:
		soundlatch_w(space, 0, data);
		cpu_set_input_line(state->audiocpu, INPUT_LINE_NMI, PULSE_LINE);
		break;
	default:
		logerror("CPU #0 PC %06X : Warning, videoreg %04X <- %04X\n", cpu_get_pc(space->cpu), offset, data);
	}
}



/* --------------------------[ Sprites Format ]----------------------------

Offset:         Values:         Format:

0000.w          y position      fedc ba9- ---- ----     unused
                                ---- ---8 ---- ----     subtract 256
                                ---- ---- 7654 3210     position

0002.w          x position      See above

0004.w          code            f--- ---- ---- ----     y flip
                                -e-- ---- ---- ----     x flip
                                --dc ---- ---- ----     unused?
                                ---- ba98 7654 3210     code

0006.w          colour          fedc ---- ---- ----     colour code
                                ---- ba98 7654 3210     unused?

------------------------------------------------------------------------ */

static void draw_sprites( running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect )
{
	ginganin_state *state = (ginganin_state *)machine->driver_data;
	UINT16 *spriteram = state->spriteram;
	int offs;

	for (offs = 0; offs < (state->spriteram_size >> 1); offs += 4)
	{
		int y = spriteram[offs + 0];
		int x = spriteram[offs + 1];
		int code = spriteram[offs + 2];
		int attr = spriteram[offs + 3];
		int flipx = code & 0x4000;
		int flipy = code & 0x8000;

		x = (x & 0xff) - (x & 0x100);
		y = (y & 0xff) - (y & 0x100);

		if (state->flipscreen)
		{
			x = 240 - x;
			y = 240 - y;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap,cliprect,machine->gfx[3],
				code & 0x3fff,
				attr >> 12,
				flipx, flipy,
				x,y,15);

	}
}


VIDEO_UPDATE( ginganin )
{
	ginganin_state *state = (ginganin_state *)screen->machine->driver_data;
	int layers_ctrl1 = state->layers_ctrl;

#ifdef MAME_DEBUG
if (input_code_pressed(screen->machine, KEYCODE_Z))
{
	int msk = 0;

	if (input_code_pressed(screen->machine, KEYCODE_Q)) { msk |= 0xfff1;}
	if (input_code_pressed(screen->machine, KEYCODE_W)) { msk |= 0xfff2;}
	if (input_code_pressed(screen->machine, KEYCODE_E)) { msk |= 0xfff4;}
	if (input_code_pressed(screen->machine, KEYCODE_A))	{ msk |= 0xfff8;}
	if (msk != 0) layers_ctrl1 &= msk;

#define SETSCROLL \
	tilemap_set_scrollx(state->bg_tilemap, 0, state->posx); \
	tilemap_set_scrolly(state->bg_tilemap, 0, state->posy); \
	tilemap_set_scrollx(state->fg_tilemap, 0, state->posx); \
	tilemap_set_scrolly(state->fg_tilemap, 0, state->posy); \
	popmessage("B>%04X:%04X F>%04X:%04X",state->posx%(BG_NX*16),state->posy%(BG_NY*16),state->posx%(FG_NX*16),state->posy%(FG_NY*16));

	if (input_code_pressed(screen->machine, KEYCODE_L))	{ state->posx +=8; SETSCROLL }
	if (input_code_pressed(screen->machine, KEYCODE_J))	{ state->posx -=8; SETSCROLL }
	if (input_code_pressed(screen->machine, KEYCODE_K))	{ state->posy +=8; SETSCROLL }
	if (input_code_pressed(screen->machine, KEYCODE_I))	{ state->posy -=8; SETSCROLL }
	if (input_code_pressed(screen->machine, KEYCODE_H))	{ state->posx = state->posy = 0;	SETSCROLL }

}
#endif


	if (layers_ctrl1 & 1)
		tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	else
		bitmap_fill(bitmap, cliprect, 0);

	if (layers_ctrl1 & 2)
		tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0);
	if (layers_ctrl1 & 8)
		draw_sprites(screen->machine, bitmap, cliprect);
	if (layers_ctrl1 & 4)
		tilemap_draw(bitmap, cliprect, state->tx_tilemap, 0, 0);

	return 0;
}

