#include "emu.h"
#include "includes/gberet.h"


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Green Beret has a 32 bytes palette PROM and two 256 bytes color lookup table
  PROMs (one for sprites, one for characters).
  The palette PROM is connected to the RGB output, this way:

  bit 7 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

***************************************************************************/

PALETTE_INIT( gberet )
{
	int i;

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 0x20);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x20; i++)
	{
		int bit0, bit1, bit2;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* blue component */
		bit0 = 0;
		bit1 = (color_prom[i] >> 6) & 0x01;
		bit2 = (color_prom[i] >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		colortable_palette_set_color(machine->colortable, i, MAKE_RGB(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x20;

	for (i = 0; i < 0x100; i++)
	{
		UINT8 ctabentry = (color_prom[i] & 0x0f) | 0x10;
		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}

	for (i = 0x100; i < 0x200; i++)
	{
		UINT8 ctabentry;

		if (color_prom[i] & 0x0f)
			ctabentry = color_prom[i] & 0x0f;
		else
			ctabentry = 0;

		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}
}

WRITE8_HANDLER( gberet_videoram_w )
{
	gberet_state *state = (gberet_state *)space->machine->driver_data;
	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

WRITE8_HANDLER( gberet_colorram_w )
{
	gberet_state *state = (gberet_state *)space->machine->driver_data;
	state->colorram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

WRITE8_HANDLER( gberet_scroll_w )
{
	gberet_state *state = (gberet_state *)space->machine->driver_data;
	int scroll;

	state->scrollram[offset] = data;

	scroll = state->scrollram[offset & 0x1f] | (state->scrollram[offset | 0x20] << 8);
	tilemap_set_scrollx(state->bg_tilemap, offset & 0x1f, scroll);
}

WRITE8_HANDLER( gberet_sprite_bank_w )
{
	gberet_state *state = (gberet_state *)space->machine->driver_data;
	state->spritebank = data;
}

static TILE_GET_INFO( get_bg_tile_info )
{
	gberet_state *state = (gberet_state *)machine->driver_data;
	int attr = state->colorram[tile_index];
	int code = state->videoram[tile_index] + ((attr & 0x40) << 2);
	int color = attr & 0x0f;
	int flags = TILE_FLIPYX((attr & 0x30) >> 4);

	tileinfo->group = color;
	tileinfo->category = (attr & 0x80) >> 7;

	SET_TILE_INFO(0, code, color, flags);
}

VIDEO_START( gberet )
{
	gberet_state *state = (gberet_state *)machine->driver_data;

	state->bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 64, 32);
	colortable_configure_tilemap_groups(machine->colortable, state->bg_tilemap, machine->gfx[0], 0x10);
	tilemap_set_scroll_rows(state->bg_tilemap, 32);
}

static void gberet_draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	gberet_state *state = (gberet_state *)machine->driver_data;
	int offs;
	UINT8 *sr;

	if (state->spritebank & 0x08)
		sr = state->spriteram2;
	else
		sr = state->spriteram;

	for (offs = 0; offs < 0xc0; offs += 4)
	{
		if (sr[offs + 3])
		{
			int attr = sr[offs + 1];
			int code = sr[offs + 0] + ((attr & 0x40) << 2);
			int color = attr & 0x0f;
			int sx = sr[offs + 2] - 2 * (attr & 0x80);
			int sy = sr[offs + 3];
			int flipx = attr & 0x10;
			int flipy = attr & 0x20;

			if (flip_screen_get(machine))
			{
				sx = 240 - sx;
				sy = 240 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			drawgfx_transmask(bitmap, cliprect, machine->gfx[1], code, color, flipx, flipy, sx, sy,
				colortable_get_transpen_mask(machine->colortable, machine->gfx[1], color, 0));
		}
	}
}

VIDEO_UPDATE( gberet )
{
	gberet_state *state = (gberet_state *)screen->machine->driver_data;

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, TILEMAP_DRAW_OPAQUE | TILEMAP_DRAW_ALL_CATEGORIES, 0);
	gberet_draw_sprites(screen->machine, bitmap, cliprect);
	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	return 0;
}

/* Green Beret (bootleg) */

WRITE8_HANDLER( gberetb_scroll_w )
{
	gberet_state *state = (gberet_state *)space->machine->driver_data;
	int scroll = data;

	if (offset)
		scroll |= 0x100;

	for (offset = 6; offset < 29; offset++)
		tilemap_set_scrollx(state->bg_tilemap, offset, scroll + 64 - 8);
}

static void gberetb_draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	gberet_state *state = (gberet_state *)machine->driver_data;
	UINT8 *spriteram = state->spriteram;
	int offs;

	for (offs = state->spriteram_size - 4; offs >= 0; offs -= 4)
	{
		if (spriteram[offs + 1])
		{
			int attr = spriteram[offs + 3];
			int code = spriteram[offs] + ((attr & 0x40) << 2);
			int color = attr & 0x0f;
			int sx = spriteram[offs + 2] - 2 * (attr & 0x80);
			int sy = 240 - spriteram[offs + 1];
			int flipx = attr & 0x10;
			int flipy = attr & 0x20;

			if (flip_screen_get(machine))
			{
				sx = 240 - sx;
				sy = 240 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			drawgfx_transmask(bitmap, cliprect, machine->gfx[1], code, color, flipx, flipy, sx, sy,
				colortable_get_transpen_mask(machine->colortable, machine->gfx[1], color, 0));
		}
	}
}

VIDEO_UPDATE( gberetb )
{
	gberet_state *state = (gberet_state *)screen->machine->driver_data;
	tilemap_draw(bitmap, cliprect, state->bg_tilemap, TILEMAP_DRAW_OPAQUE | TILEMAP_DRAW_ALL_CATEGORIES, 0);
	gberetb_draw_sprites(screen->machine, bitmap, cliprect);
	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	return 0;
}
