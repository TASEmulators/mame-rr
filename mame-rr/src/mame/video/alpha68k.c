/***************************************************************************

   Alpha 68k video emulation - Bryan McPhail, mish@tendril.co.uk

****************************************************************************/

#include "emu.h"
#include "includes/alpha68k.h"


void alpha68k_flipscreen_w( running_machine *machine, int flip )
{
	alpha68k_state *state = (alpha68k_state *)machine->driver_data;
	state->flipscreen = flip;
}

void alpha68k_V_video_bank_w( running_machine *machine, int bank )
{
	alpha68k_state *state = (alpha68k_state *)machine->driver_data;
	state->bank_base = bank & 0xf;
}

WRITE16_HANDLER( alpha68k_paletteram_w )
{
	alpha68k_state *state = (alpha68k_state *)space->machine->driver_data;
	int newword;
	int r, g, b;

	COMBINE_DATA(state->paletteram + offset);
	newword = state->paletteram[offset];

	r = ((newword >> 7) & 0x1e) | ((newword >> 14) & 0x01);
	g = ((newword >> 3) & 0x1e) | ((newword >> 13) & 0x01);
	b = ((newword << 1) & 0x1e) | ((newword >> 12) & 0x01);

	palette_set_color_rgb(space->machine, offset, pal5bit(r), pal5bit(g), pal5bit(b));
}

/******************************************************************************/

static TILE_GET_INFO( get_tile_info )
{
	alpha68k_state *state = (alpha68k_state *)machine->driver_data;
	int tile = state->videoram[2 * tile_index] & 0xff;
	int color = state->videoram[2 * tile_index + 1] & 0x0f;

	tile = tile | (state->bank_base << 8);

	SET_TILE_INFO(0, tile, color, 0);
}

WRITE16_HANDLER( alpha68k_videoram_w )
{
	alpha68k_state *state = (alpha68k_state *)space->machine->driver_data;
	/* Doh. */
	if(ACCESSING_BITS_0_7)
		if(ACCESSING_BITS_8_15)
			state->videoram[offset] = data;
		else
			state->videoram[offset] = data & 0xff;
	else
		state->videoram[offset] = (data >> 8) & 0xff;

	tilemap_mark_tile_dirty(state->fix_tilemap, offset / 2);
}

VIDEO_START( alpha68k )
{
	alpha68k_state *state = (alpha68k_state *)machine->driver_data;

	state->fix_tilemap = tilemap_create(machine, get_tile_info, tilemap_scan_cols, 8, 8, 32, 32);
	tilemap_set_transparent_pen(state->fix_tilemap, 0);
}

/******************************************************************************/

//AT
static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int j, int s, int e )
{
	alpha68k_state *state = (alpha68k_state *)machine->driver_data;
	UINT16 *spriteram = state->spriteram;
	int offs, mx, my, color, tile, fx, fy, i;

	for (offs = s; offs < e; offs += 0x40)
	{
		my = spriteram[offs + 3 + (j << 1)];
		mx = spriteram[offs + 2 + (j << 1)] << 1 | my >> 15;
		my = -my & 0x1ff;
		mx = ((mx + 0x100) & 0x1ff) - 0x100;
		if (j == 0 && s == 0x7c0)
			my++;
//ZT
		if (state->flipscreen)
		{
			mx = 240 - mx;
			my = 240 - my;
		}

		for (i = 0; i < 0x40; i += 2)
		{
			tile = spriteram[offs + 1 + i + (0x800 * j) + 0x800];
			color = spriteram[offs + i + (0x800 * j) + 0x800] & 0x7f;

			fy = tile & 0x8000;
			fx = tile & 0x4000;
			tile &= 0x3fff;

			if (state->flipscreen)
			{
				if (fx) fx = 0; else fx = 1;
				if (fy) fy = 0; else fy = 1;
			}

			if (color)
				drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
					tile,
					color,
					fx,fy,
					mx,my,0);

			if (state->flipscreen)
				my = (my - 16) & 0x1ff;
			else
				my = (my + 16) & 0x1ff;
		}
	}
}

/******************************************************************************/

VIDEO_UPDATE( alpha68k_II )
{
	alpha68k_state *state = (alpha68k_state *)screen->machine->driver_data;

	if (state->last_bank != state->bank_base)
		tilemap_mark_all_tiles_dirty_all(screen->machine);

	state->last_bank = state->bank_base;
	tilemap_set_flip_all(screen->machine, state->flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	bitmap_fill(bitmap, cliprect, 2047);
//AT
	draw_sprites(screen->machine, bitmap, cliprect, 0, 0x07c0, 0x0800);
	draw_sprites(screen->machine, bitmap, cliprect, 1, 0x0000, 0x0800);
	draw_sprites(screen->machine, bitmap, cliprect, 2, 0x0000, 0x0800);
	draw_sprites(screen->machine, bitmap, cliprect, 0, 0x0000, 0x07c0);
//ZT
	tilemap_draw(bitmap, cliprect, state->fix_tilemap, 0, 0);
	return 0;
}

/******************************************************************************/

/*
    Video banking:

    Write to these locations in this order for correct bank:

    20 28 30 for Bank 0
    60 28 30 for Bank 1
    20 68 30 etc
    60 68 30
    20 28 70
    60 28 70
    20 68 70
    60 68 70 for Bank 7

    Actual data values written don't matter!

*/

WRITE16_HANDLER( alpha68k_II_video_bank_w )
{
	alpha68k_state *state = (alpha68k_state *)space->machine->driver_data;
	switch (offset)
	{
		case 0x10: /* Reset */
			state->bank_base = state->buffer_28 = state->buffer_60 = state->buffer_68 = 0;
			return;
		case 0x14:
			if (state->buffer_60) state->bank_base=1; else state->bank_base=0;
			state->buffer_28 = 1;
			return;
		case 0x18:
			if (state->buffer_68) {if (state->buffer_60) state->bank_base = 3; else state->bank_base = 2; }
			if (state->buffer_28) {if (state->buffer_60) state->bank_base = 1; else state->bank_base = 0; }
			return;
		case 0x30:
			state->buffer_28 = state->buffer_68 = 0; state->bank_base = 1;
			state->buffer_60 = 1;
			return;
		case 0x34:
			if (state->buffer_60) state->bank_base = 3; else state->bank_base = 2;
			state->buffer_68 = 1;
			return;
		case 0x38:
			if (state->buffer_68) {if (state->buffer_60) state->bank_base = 7; else state->bank_base = 6; }
			if (state->buffer_28) {if (state->buffer_60) state->bank_base = 5; else state->bank_base = 4; }
			return;
		case 0x08: /* Graphics flags?  Not related to fix chars anyway */
		case 0x0c:
		case 0x28:
		case 0x2c:
			return;
	}

	logerror("%04x \n",offset);
}

/******************************************************************************/

WRITE16_HANDLER( alpha68k_V_video_control_w )
{
	switch (offset)
	{
		case 0x08: /* Graphics flags?  Not related to fix chars anyway */
		case 0x0c:
		case 0x28:
		case 0x2c:
			return;
	}
}

static void draw_sprites_V( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int j, int s, int e, int fx_mask, int fy_mask, int sprite_mask )
{
	alpha68k_state *state = (alpha68k_state *)machine->driver_data;
	UINT16 *spriteram = state->spriteram;
	int offs, mx, my, color, tile, fx, fy, i;

	for (offs = s; offs < e; offs += 0x40)
	{
//AT
		my = spriteram[offs + 3 + (j << 1)];
		mx = spriteram[offs + 2 + (j << 1)] << 1 | my >> 15;
		my = -my & 0x1ff;
		mx = ((mx + 0x100) & 0x1ff) - 0x100;
		if (j == 0 && s == 0x7c0)
			my++;
//ZT
		if (state->flipscreen)
		{
			mx = 240 - mx;
			my = 240 - my;
		}

		for (i = 0; i < 0x40; i += 2)
		{
			tile = spriteram[offs + 1 + i + (0x800 * j) + 0x800];
			color = spriteram[offs + i + (0x800 * j) + 0x800] & 0xff;

			fx = tile & fx_mask;
			fy = tile & fy_mask;
			tile = tile & sprite_mask;
			if (tile > 0x4fff)
				continue;

			if (state->flipscreen)
			{
				if (fx) fx = 0; else fx = 1;
				if (fy) fy = 0; else fy = 1;
			}

			if (color)
				drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
					tile,
					color,
					fx,fy,
					mx,my,0);

			if (state->flipscreen)
				my = (my - 16) & 0x1ff;
			else
				my = (my + 16) & 0x1ff;
		}
	}
}

VIDEO_UPDATE( alpha68k_V )
{
	alpha68k_state *state = (alpha68k_state *)screen->machine->driver_data;
	UINT16 *spriteram = state->spriteram;

	if (state->last_bank != state->bank_base)
		tilemap_mark_all_tiles_dirty_all(screen->machine);

	state->last_bank = state->bank_base;
	tilemap_set_flip_all(screen->machine, state->flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	bitmap_fill(bitmap, cliprect, 4095);

	/* This appears to be correct priority */
	if (state->microcontroller_id == 0x8814) /* Sky Adventure */
	{
		draw_sprites_V(screen->machine, bitmap, cliprect, 0, 0x07c0, 0x0800, 0, 0x8000, 0x7fff);
		draw_sprites_V(screen->machine, bitmap, cliprect, 1, 0x0000, 0x0800, 0, 0x8000, 0x7fff);
		//AT: *KLUDGE* fixes priest priority in level 1(could be a game bug)
		if (spriteram[0x1bde] == 0x24 && (spriteram[0x1bdf] >> 8) == 0x3b)
		{
			draw_sprites_V(screen->machine, bitmap, cliprect, 2, 0x03c0, 0x0800, 0, 0x8000, 0x7fff);
			draw_sprites_V(screen->machine, bitmap, cliprect, 2, 0x0000, 0x03c0, 0, 0x8000, 0x7fff);
		}
		else
			draw_sprites_V(screen->machine, bitmap, cliprect, 2, 0x0000, 0x0800, 0, 0x8000, 0x7fff);

		draw_sprites_V(screen->machine, bitmap, cliprect, 0, 0x0000, 0x07c0, 0, 0x8000, 0x7fff);
	}
	else	/* gangwars */
	{
		draw_sprites_V(screen->machine, bitmap, cliprect, 0, 0x07c0, 0x0800, 0x8000, 0, 0x7fff);
		draw_sprites_V(screen->machine, bitmap, cliprect, 1, 0x0000, 0x0800, 0x8000, 0, 0x7fff);
		draw_sprites_V(screen->machine, bitmap, cliprect, 2, 0x0000, 0x0800, 0x8000, 0, 0x7fff);
		draw_sprites_V(screen->machine, bitmap, cliprect, 0, 0x0000, 0x07c0, 0x8000, 0, 0x7fff);
	}

	tilemap_draw(bitmap, cliprect, state->fix_tilemap, 0, 0);
	return 0;
}

VIDEO_UPDATE( alpha68k_V_sb )
{
	alpha68k_state *state = (alpha68k_state *)screen->machine->driver_data;

	if (state->last_bank != state->bank_base)
		tilemap_mark_all_tiles_dirty_all(screen->machine);

	state->last_bank = state->bank_base;
	tilemap_set_flip_all(screen->machine, state->flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	bitmap_fill(bitmap, cliprect, 4095);

	/* This appears to be correct priority */
	draw_sprites_V(screen->machine, bitmap, cliprect, 0, 0x07c0, 0x0800, 0x4000, 0x8000, 0x3fff);
	draw_sprites_V(screen->machine, bitmap, cliprect, 1, 0x0000, 0x0800, 0x4000, 0x8000, 0x3fff);
	draw_sprites_V(screen->machine, bitmap, cliprect, 2, 0x0000, 0x0800, 0x4000, 0x8000, 0x3fff);
	draw_sprites_V(screen->machine, bitmap, cliprect, 0, 0x0000, 0x07c0, 0x4000, 0x8000, 0x3fff);

	tilemap_draw(bitmap, cliprect, state->fix_tilemap, 0, 0);
	return 0;
}

/******************************************************************************/
//AT
static void draw_sprites_I( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int c, int d, int yshift )
{
	alpha68k_state *state = (alpha68k_state *)machine->driver_data;
	UINT16 *spriteram = state->spriteram;
	int data, offs, mx, my, tile, color, fy, i;
	UINT8 *color_prom = memory_region(machine, "user1");
	gfx_element *gfx = machine->gfx[0];

	for (offs = 0; offs < 0x400; offs += 0x20)
	{
		mx = spriteram[offs + c];
		my = (yshift - (mx >> 8)) & 0xff;
		mx &= 0xff;

		for (i = 0; i < 0x20; i++)
		{
			data = spriteram[offs + d + i];
			tile = data & 0x3fff;
			fy = data & 0x4000;
			color = color_prom[tile << 1 | data >> 15];

			drawgfx_transpen(bitmap, cliprect, gfx, tile, color, 0, fy, mx, my, 0);

			my = (my + 8) & 0xff;
		}
	}
}

VIDEO_UPDATE( alpha68k_I )
{
	alpha68k_state *state = (alpha68k_state *)screen->machine->driver_data;
	int yshift = (state->microcontroller_id == 0x890a) ? 1 : 0; // The Next Space is 1 pixel off

	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));

	/* This appears to be correct priority */
	draw_sprites_I(screen->machine, bitmap, cliprect, 2, 0x0800, yshift);
	draw_sprites_I(screen->machine, bitmap, cliprect, 3, 0x0c00, yshift);
	draw_sprites_I(screen->machine, bitmap, cliprect, 1, 0x0400, yshift);
	return 0;
}
//ZT
/******************************************************************************/

PALETTE_INIT( kyros )
{
	int i;

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 0x100);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x100; i++)
	{
		int r = pal4bit(color_prom[i + 0x000]);
		int g = pal4bit(color_prom[i + 0x100]);
		int b = pal4bit(color_prom[i + 0x200]);

		colortable_palette_set_color(machine->colortable, i, MAKE_RGB(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x300;

	for (i = 0; i < 0x100; i++)
	{
		UINT8 ctabentry = ((color_prom[i] & 0x0f) << 4) | (color_prom[i + 0x100] & 0x0f);
		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}
}

PALETTE_INIT( paddlem )
{
	int i;

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 0x100);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x100; i++)
	{
		int r = pal4bit(color_prom[i + 0x000]);
		int g = pal4bit(color_prom[i + 0x100]);
		int b = pal4bit(color_prom[i + 0x200]);

		colortable_palette_set_color(machine->colortable, i, MAKE_RGB(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x300;

	for (i = 0; i < 0x400; i++)
	{
		UINT8 ctabentry = ((color_prom[i + 0x400] & 0x0f) << 4) | (color_prom[i] & 0x0f);
		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}
}

static void kyros_video_banking(int *bank, int data)
{
	*bank = (data >> 13 & 4) | (data >> 10 & 3);
}

static void jongbou_video_banking(int *bank, int data)
{
	*bank = (data >> 11 & 4) | (data >> 10 & 3);
}

static void kyros_draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int c, int d )
{
	alpha68k_state *state = (alpha68k_state *)machine->driver_data;
	UINT16 *spriteram = state->spriteram;
	int offs, mx, my, color, tile, i, bank, fy, fx;
	int data;
	UINT8 *color_prom = memory_region(machine, "user1");

//AT
	for (offs = 0; offs < 0x400; offs += 0x20)
	{
		mx = spriteram[offs + c];
		my = -(mx >> 8) & 0xff;
		mx &= 0xff;

		if (state->flipscreen)
			my = 249 - my;

		for (i = 0; i < 0x20; i++)
		{
			data = spriteram[offs + d + i];
			if (data!=0x20)
			{
				color = color_prom[(data >> 1 & 0x1000) | (data & 0xffc) | (data >> 14 & 3)];
				if (color != 0xff)
				{
					fy = data & 0x1000;
					fx = 0;

					if(state->flipscreen)
					{
						if (fy) fy = 0; else fy = 1;
						fx = 1;
					}

					tile = (data >> 3 & 0x400) | (data & 0x3ff);
					if (state->game_id == ALPHA68K_KYROS)
						kyros_video_banking(&bank, data);
					else
						jongbou_video_banking(&bank, data);

					drawgfx_transpen(bitmap, cliprect, machine->gfx[bank], tile, color, fx, fy, mx, my, 0);
				}
			}
//ZT
			if (state->flipscreen)
				my = (my - 8) & 0xff;
			else
				my = (my + 8) & 0xff;
		}
	}
}

VIDEO_UPDATE( kyros )
{
	alpha68k_state *state = (alpha68k_state *)screen->machine->driver_data;
	colortable_entry_set_value(screen->machine->colortable, 0x100, *state->videoram & 0xff);
	bitmap_fill(bitmap, cliprect, 0x100); //AT

	kyros_draw_sprites(screen->machine, bitmap, cliprect, 2, 0x0800);
	kyros_draw_sprites(screen->machine, bitmap, cliprect, 3, 0x0c00);
	kyros_draw_sprites(screen->machine, bitmap, cliprect, 1, 0x0400);
	return 0;
}

/******************************************************************************/

static void sstingry_draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int c, int d )
{
//AT
	alpha68k_state *state = (alpha68k_state *)machine->driver_data;
	UINT16 *spriteram = state->spriteram;
	int data, offs, mx, my, color, tile, i, bank, fy, fx;

	for (offs = 0; offs < 0x400; offs += 0x20)
	{
		mx = spriteram[offs + c];
		my = -(mx >> 8) & 0xff;
		mx &= 0xff;
		if (mx > 0xf8)
			mx -= 0x100;

		if (state->flipscreen)
			my = 249 - my;

		for (i = 0; i < 0x20; i++)
		{
			data = spriteram[offs + d + i];
			if (data != 0x40)
			{
				fy = data & 0x1000;
				fx = 0;

				if(state->flipscreen)
				{
					if (fy) fy = 0; else fy = 1;
					fx = 1;
				}

				color = (data >> 7 & 0x18) | (data >> 13 & 7);
				tile = data & 0x3ff;
				bank = data >> 10 & 3;
				drawgfx_transpen(bitmap, cliprect, machine->gfx[bank], tile, color, fx, fy, mx, my, 0);
			}
//ZT
			if(state->flipscreen)
				my = (my - 8) & 0xff;
			else
				my = (my + 8) & 0xff;
		}
	}
}

VIDEO_UPDATE( sstingry )
{
	alpha68k_state *state = (alpha68k_state *)screen->machine->driver_data;
	colortable_entry_set_value(screen->machine->colortable, 0x100, *state->videoram & 0xff);
	bitmap_fill(bitmap, cliprect, 0x100); //AT

	sstingry_draw_sprites(screen->machine, bitmap, cliprect, 2, 0x0800);
	sstingry_draw_sprites(screen->machine, bitmap, cliprect, 3, 0x0c00);
	sstingry_draw_sprites(screen->machine, bitmap, cliprect, 1, 0x0400);
	return 0;
}
