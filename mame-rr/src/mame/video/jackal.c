/***************************************************************************

  video.c

  Written by Kenneth Lin (kenneth_lin@ai.vancouver.bc.ca)

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/jackal.h"


PALETTE_INIT( jackal )
{
	int i;

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 0x200);

	for (i = 0; i < 0x100; i++)
	{
		UINT16 ctabentry = i | 0x100;
		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}

	for (i = 0x100; i < 0x200; i++)
	{
		UINT16 ctabentry = color_prom[i - 0x100] & 0x0f;
		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}

	for (i = 0x200; i < 0x300; i++)
	{
		UINT16 ctabentry = (color_prom[i - 0x100] & 0x0f) | 0x10;
		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}
}


static void set_pens( running_machine *machine )
{
	jackal_state *state = (jackal_state *)machine->driver_data;
	int i;

	for (i = 0; i < 0x400; i += 2)
	{
		UINT16 data = state->paletteram[i] | (state->paletteram[i | 1] << 8);

		rgb_t color = MAKE_RGB(pal5bit(data >> 0), pal5bit(data >> 5), pal5bit(data >> 10));

		colortable_palette_set_color(machine->colortable, i >> 1, color);
	}
}


void jackal_mark_tile_dirty( running_machine *machine, int offset )
{
	jackal_state *state = (jackal_state *)machine->driver_data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	UINT8 *RAM = memory_region(machine, "master");

	int attr = RAM[0x2000 + tile_index];
	int code = RAM[0x2400 + tile_index] + ((attr & 0xc0) << 2) + ((attr & 0x30) << 6);
	int color = 0;//attr & 0x0f;
	int flags = ((attr & 0x10) ? TILE_FLIPX : 0) | ((attr & 0x20) ? TILE_FLIPY : 0);

	SET_TILE_INFO(0, code, color, flags);
}

VIDEO_START( jackal )
{
	jackal_state *state = (jackal_state *)machine->driver_data;
	state->bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
}

static void draw_background( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	jackal_state *state = (jackal_state *)machine->driver_data;
	UINT8 *RAM = memory_region(machine, "master");
	int i;

	state->scrollram = &RAM[0x0020];

	tilemap_set_scroll_rows(state->bg_tilemap, 1);
	tilemap_set_scroll_cols(state->bg_tilemap, 1);

	tilemap_set_scrolly(state->bg_tilemap, 0, state->videoctrl[0]);
	tilemap_set_scrollx(state->bg_tilemap, 0, state->videoctrl[1]);

	if (state->videoctrl[2] & 0x02)
	{
		if (state->videoctrl[2] & 0x08)
		{
			tilemap_set_scroll_rows(state->bg_tilemap, 32);

			for (i = 0; i < 32; i++)
				tilemap_set_scrollx(state->bg_tilemap, i, state->scrollram[i]);
		}

		if (state->videoctrl[2] & 0x04)
		{
			tilemap_set_scroll_cols(state->bg_tilemap, 32);

			for (i = 0; i < 32; i++)
				tilemap_set_scrolly(state->bg_tilemap, i, state->scrollram[i]);
		}
	}

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
}

#define DRAW_SPRITE(bank, code, sx, sy) drawgfx_transpen(bitmap, cliprect, machine->gfx[bank], code, color, flipx, flipy, sx, sy, 0);

static void draw_sprites_region( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, const UINT8 *sram, int length, int bank )
{
	int offs;

	for (offs = 0; offs < length; offs += 5)
	{
		int sn1 = sram[offs];
		int sn2 = sram[offs + 1];
		int sy  = sram[offs + 2];
		int sx  = sram[offs + 3];
		int attr = sram[offs + 4];
		int flipx = attr & 0x20;
		int flipy = attr & 0x40;
		int color = ((sn2 & 0xf0) >> 4);

		if (attr & 0x01)
			sx = sx - 256;
		if (sy > 0xf0)
			sy = sy - 256;

		if (flip_screen_get(machine))
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		if (attr & 0xC)    // half-size sprite
		{
			int spritenum = sn1 * 4 + ((sn2 & (8 + 4)) >> 2) + ((sn2 & (2 + 1)) << 10);
			int mod = -8;

			if (flip_screen_get(machine))
			{
				sx += 8;
				sy -= 8;
				mod = 8;
			}

			if ((attr & 0x0C) == 0x0C)
			{
				if (flip_screen_get(machine)) sy += 16;
				DRAW_SPRITE(bank + 1, spritenum, sx, sy)
			}

			if ((attr & 0x0C) == 0x08)
			{
				sy += 8;
				DRAW_SPRITE(bank + 1, spritenum,     sx, sy)
				DRAW_SPRITE(bank + 1, spritenum - 2, sx, sy + mod)
			}

			if ((attr & 0x0C) == 0x04)
			{
				DRAW_SPRITE(bank + 1, spritenum,     sx,       sy)
				DRAW_SPRITE(bank + 1, spritenum + 1, sx + mod, sy)
			}
		}
		else
		{
			int spritenum = sn1 + ((sn2 & 0x03) << 8);

			if (attr & 0x10)
			{
				if (flip_screen_get(machine))
				{
					sx -= 16;
					sy -= 16;
				}

				DRAW_SPRITE(bank, spritenum,     flipx ? sx+16 : sx, flipy ? sy+16 : sy)
				DRAW_SPRITE(bank, spritenum + 1, flipx ? sx : sx+16, flipy ? sy+16 : sy)
				DRAW_SPRITE(bank, spritenum + 2, flipx ? sx+16 : sx, flipy ? sy : sy+16)
				DRAW_SPRITE(bank, spritenum + 3, flipx ? sx : sx+16, flipy ? sy : sy+16)
			}
			else
			{
				DRAW_SPRITE(bank, spritenum, sx, sy)
			}
		}
	}
}

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	jackal_state *state = (jackal_state *)machine->driver_data;
	UINT8 *RAM = memory_region(machine, "master");
	UINT8 *sr, *ss;

	if (state->videoctrl[0x03] & 0x08)
	{
		sr = &RAM[0x03800];	// Sprite 2
		ss = &RAM[0x13800];	// Additional Sprite 2
	}
	else
	{
		sr = &RAM[0x03000];	// Sprite 1
		ss = &RAM[0x13000];	// Additional Sprite 1
	}

	draw_sprites_region(machine, bitmap, cliprect, ss, 0x0f5, 3);
	draw_sprites_region(machine, bitmap, cliprect, sr, 0x500, 1);
}

VIDEO_UPDATE( jackal )
{
	set_pens(screen->machine);
	draw_background(screen->machine, bitmap, cliprect);
	draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}
