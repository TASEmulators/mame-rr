#include "emu.h"
#include "includes/sf.h"

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_bg_tile_info )
{
	UINT8 *base = memory_region(machine, "gfx5") + 2 * tile_index;
	int attr = base[0x10000];
	int color = base[0];
	int code = (base[0x10000 + 1] << 8) | base[1];
	SET_TILE_INFO(
			0,
			code,
			color,
			TILE_FLIPYX(attr & 3));
}

static TILE_GET_INFO( get_fg_tile_info )
{
	UINT8 *base = memory_region(machine, "gfx5") + 0x20000 + 2 * tile_index;
	int attr = base[0x10000];
	int color = base[0];
	int code = (base[0x10000 + 1] << 8) | base[1];
	SET_TILE_INFO(
			1,
			code,
			color,
			TILE_FLIPYX(attr & 3));
}

static TILE_GET_INFO( get_tx_tile_info )
{
	sf_state *state = (sf_state *)machine->driver_data;
	int code = state->videoram[tile_index];
	SET_TILE_INFO(
			3,
			code & 0x3ff,
			code>>12,
			TILE_FLIPYX((code & 0xc00)>>10));
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( sf )
{
	sf_state *state = (sf_state *)machine->driver_data;

	state->bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_cols, 16, 16, 2048, 16);
	state->fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_cols, 16, 16, 2048, 16);
	state->tx_tilemap = tilemap_create(machine, get_tx_tile_info, tilemap_scan_rows, 8, 8, 64, 32);

	tilemap_set_transparent_pen(state->fg_tilemap, 15);
	tilemap_set_transparent_pen(state->tx_tilemap, 3);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE16_HANDLER( sf_videoram_w )
{
	sf_state *state = (sf_state *)space->machine->driver_data;
	COMBINE_DATA(&state->videoram[offset]);
	tilemap_mark_tile_dirty(state->tx_tilemap, offset);
}

WRITE16_HANDLER( sf_bg_scroll_w )
{
	sf_state *state = (sf_state *)space->machine->driver_data;
	COMBINE_DATA(&state->bgscroll);
	tilemap_set_scrollx(state->bg_tilemap, 0, state->bgscroll);
}

WRITE16_HANDLER( sf_fg_scroll_w )
{
	sf_state *state = (sf_state *)space->machine->driver_data;
	COMBINE_DATA(&state->fgscroll);
	tilemap_set_scrollx(state->fg_tilemap, 0, state->fgscroll);
}

WRITE16_HANDLER( sf_gfxctrl_w )
{
	/* b0 = reset, or maybe "set anyway" */
	/* b1 = pulsed when control6.b6==0 until it's 1 */
	/* b2 = active when dip 8 (flip) on */
	/* b3 = active character plane */
	/* b4 = unused */
	/* b5 = active background plane */
	/* b6 = active middle plane */
	/* b7 = active sprites */

	sf_state *state = (sf_state *)space->machine->driver_data;
	if (ACCESSING_BITS_0_7)
	{
		state->sf_active = data & 0xff;
		flip_screen_set(space->machine, data & 0x04);
		tilemap_set_enable(state->tx_tilemap, data & 0x08);
		tilemap_set_enable(state->bg_tilemap, data & 0x20);
		tilemap_set_enable(state->fg_tilemap, data & 0x40);
	}
}



/***************************************************************************

  Display refresh

***************************************************************************/

INLINE int sf_invert( int nb )
{
	static const int delta[4] = {0x00, 0x18, 0x18, 0x00};
	return nb ^ delta[(nb >> 3) & 3];
}

static void draw_sprites( running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect )
{
	sf_state *state = (sf_state *)machine->driver_data;
	int offs;

	for (offs = 0x1000 - 0x20; offs >= 0; offs -= 0x20)
	{
		int c = state->objectram[offs];
		int attr = state->objectram[offs + 1];
		int sy = state->objectram[offs + 2];
		int sx = state->objectram[offs + 3];
		int color = attr & 0x000f;
		int flipx = attr & 0x0100;
		int flipy = attr & 0x0200;

		if (attr & 0x400)	/* large sprite */
		{
			int c1, c2, c3, c4, t;

			if (flip_screen_get(machine))
			{
				sx = 480 - sx;
				sy = 224 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			c1 = c;
			c2 = c + 1;
			c3 = c + 16;
			c4 = c + 17;

			if (flipx)
			{
				t = c1; c1 = c2; c2 = t;
				t = c3; c3 = c4; c4 = t;
			}
			if (flipy)
			{
				t = c1; c1 = c3; c3 = t;
				t = c2; c2 = c4; c4 = t;
			}

			drawgfx_transpen(bitmap,
					cliprect, machine->gfx[2],
					sf_invert(c1),
					color,
					flipx,flipy,
					sx,sy, 15);
			drawgfx_transpen(bitmap,
					cliprect, machine->gfx[2],
					sf_invert(c2),
					color,
					flipx,flipy,
					sx+16,sy, 15);
			drawgfx_transpen(bitmap,
					cliprect, machine->gfx[2],
					sf_invert(c3),
					color,
					flipx,flipy,
					sx,sy+16, 15);
			drawgfx_transpen(bitmap,
					cliprect, machine->gfx[2],
					sf_invert(c4),
					color,
					flipx,flipy,
					sx+16,sy+16, 15);
		}
		else
		{
			if (flip_screen_get(machine))
			{
				sx = 496 - sx;
				sy = 240 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			drawgfx_transpen(bitmap,
					cliprect, machine->gfx[2],
					sf_invert(c),
					color,
					flipx,flipy,
					sx,sy, 15);
		}
	}
}


VIDEO_UPDATE( sf )
{
	sf_state *state = (sf_state *)screen->machine->driver_data;

	if (state->sf_active & 0x20)
		tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	else
		bitmap_fill(bitmap, cliprect, 0);

	tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0);

	if (state->sf_active & 0x80)
		draw_sprites(screen->machine, bitmap, cliprect);

	tilemap_draw(bitmap, cliprect, state->tx_tilemap, 0, 0);
	return 0;
}
