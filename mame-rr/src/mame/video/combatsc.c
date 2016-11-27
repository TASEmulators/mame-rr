/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "video/konicdev.h"
#include "includes/combatsc.h"

PALETTE_INIT( combatsc )
{
	int pal;

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 0x80);

	for (pal = 0; pal < 8; pal++)
	{
		int i, clut;

		switch (pal)
		{
			default:
			case 0: /* other sprites */
			case 2: /* other sprites(alt) */
			clut = 1;	/* 0 is wrong for Firing Range III targets */
			break;

			case 4: /* player sprites */
			case 6: /* player sprites(alt) */
			clut = 2;
			break;

			case 1: /* background */
			case 3: /* background(alt) */
			clut = 1;
			break;

			case 5: /* foreground tiles */
			case 7: /* foreground tiles(alt) */
			clut = 3;
			break;
		}

		for (i = 0; i < 0x100; i++)
		{
			UINT8 ctabentry;

			if (((pal & 0x01) == 0) && (color_prom[(clut << 8) | i] == 0))
				ctabentry = 0;
			else
				ctabentry = (pal << 4) | (color_prom[(clut << 8) | i] & 0x0f);

			colortable_entry_set_value(machine->colortable, (pal << 8) | i, ctabentry);
		}
	}
}


PALETTE_INIT( combatscb )
{
	int pal;

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 0x80);

	for (pal = 0; pal < 8; pal++)
	{
		int i;

		for (i = 0; i < 0x100; i++)
		{
			UINT8 ctabentry;

			if ((pal & 1) == 0)
				/* sprites */
				ctabentry = (pal << 4) | (~color_prom[i] & 0x0f);
			else
				/* chars - no lookup? */
				ctabentry = (pal << 4) | (i & 0x0f);	/* no lookup? */

			colortable_entry_set_value(machine->colortable, (pal << 8) | i, ctabentry);
		}
	}
}


static void set_pens( running_machine *machine )
{
	combatsc_state *state = (combatsc_state *)machine->driver_data;
	int i;

	for (i = 0x00; i < 0x100; i += 2)
	{
		UINT16 data = state->paletteram[i] | (state->paletteram[i | 1] << 8);

		rgb_t color = MAKE_RGB(pal5bit(data >> 0), pal5bit(data >> 5), pal5bit(data >> 10));

		colortable_palette_set_color(machine->colortable, i >> 1, color);
	}
}



/***************************************************************************

    Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_tile_info0 )
{
	combatsc_state *state = (combatsc_state *)machine->driver_data;
	UINT8 ctrl_6 = k007121_ctrlram_r(state->k007121_1, 6);
	UINT8 attributes = state->page[0][tile_index];
	int bank = 4 * ((state->vreg & 0x0f) - 1);
	int number, color;

	if (bank < 0)
		bank = 0;

	if ((attributes & 0xb0) == 0)
		bank = 0;	/* text bank */

	if (attributes & 0x80)
		bank += 1;

	if (attributes & 0x10)
		bank += 2;

	if (attributes & 0x20)
		bank += 4;

	color = ((ctrl_6 & 0x10) * 2 + 16) + (attributes & 0x0f);

	number = state->page[0][tile_index + 0x400] + 256 * bank;

	SET_TILE_INFO(
			0,
			number,
			color,
			0);
	tileinfo->category = (attributes & 0x40) >> 6;
}

static TILE_GET_INFO( get_tile_info1 )
{
	combatsc_state *state = (combatsc_state *)machine->driver_data;
	UINT8 ctrl_6 = k007121_ctrlram_r(state->k007121_2, 6);
	UINT8 attributes = state->page[1][tile_index];
	int bank = 4 * ((state->vreg >> 4) - 1);
	int number, color;

	if (bank < 0)
		bank = 0;

	if ((attributes & 0xb0) == 0)
		bank = 0;	/* text bank */

	if (attributes & 0x80)
		bank += 1;

	if (attributes & 0x10)
		bank += 2;

	if (attributes & 0x20)
		bank += 4;

	color = ((ctrl_6 & 0x10) * 2 + 16 + 4 * 16) + (attributes & 0x0f);

	number = state->page[1][tile_index + 0x400] + 256 * bank;

	SET_TILE_INFO(
			1,
			number,
			color,
			0);
	tileinfo->category = (attributes & 0x40) >> 6;
}

static TILE_GET_INFO( get_text_info )
{
	combatsc_state *state = (combatsc_state *)machine->driver_data;
	UINT8 attributes = state->page[0][tile_index + 0x800];
	int number = state->page[0][tile_index + 0xc00];
	int color = 16 + (attributes & 0x0f);

	SET_TILE_INFO(
			0,
			number,
			color,
			0);
}


static TILE_GET_INFO( get_tile_info0_bootleg )
{
	combatsc_state *state = (combatsc_state *)machine->driver_data;
	UINT8 attributes = state->page[0][tile_index];
	int bank = 4 * ((state->vreg & 0x0f) - 1);
	int number, pal, color;

	if (bank < 0)
		bank = 0;

	if ((attributes & 0xb0) == 0)
		bank = 0;	/* text bank */

	if (attributes & 0x80)
		bank += 1;

	if (attributes & 0x10)
		bank += 2;

	if (attributes & 0x20)
		bank += 4;

	pal = (bank == 0 || bank >= 0x1c || (attributes & 0x40)) ? 1 : 3;
	color = pal*16;// + (attributes & 0x0f);
	number = state->page[0][tile_index + 0x400] + 256 * bank;

	SET_TILE_INFO(
			0,
			number,
			color,
			0);
}

static TILE_GET_INFO( get_tile_info1_bootleg )
{
	combatsc_state *state = (combatsc_state *)machine->driver_data;
	UINT8 attributes = state->page[1][tile_index];
	int bank = 4*((state->vreg >> 4) - 1);
	int number, pal, color;

	if (bank < 0)
		bank = 0;

	if ((attributes & 0xb0) == 0)
		bank = 0;	/* text bank */

	if (attributes & 0x80)
		bank += 1;

	if (attributes & 0x10)
		bank += 2;

	if (attributes & 0x20)
		bank += 4;

	pal = (bank == 0 || bank >= 0x1c || (attributes & 0x40)) ? 5 : 7;
	color = pal * 16;// + (attributes & 0x0f);
	number = state->page[1][tile_index + 0x400] + 256 * bank;

	SET_TILE_INFO(
			1,
			number,
			color,
			0);
}

static TILE_GET_INFO( get_text_info_bootleg )
{
	combatsc_state *state = (combatsc_state *)machine->driver_data;
//  UINT8 attributes = state->page[0][tile_index + 0x800];
	int number = state->page[0][tile_index + 0xc00];
	int color = 16;// + (attributes & 0x0f);

	SET_TILE_INFO(
			1,
			number,
			color,
			0);
}

/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

VIDEO_START( combatsc )
{
	combatsc_state *state = (combatsc_state *)machine->driver_data;

	state->bg_tilemap[0] = tilemap_create(machine, get_tile_info0, tilemap_scan_rows, 8, 8, 32, 32);
	state->bg_tilemap[1] = tilemap_create(machine, get_tile_info1, tilemap_scan_rows, 8, 8, 32, 32);
	state->textlayer =  tilemap_create(machine, get_text_info, tilemap_scan_rows, 8, 8, 32, 32);

	state->spriteram[0] = auto_alloc_array_clear(machine, UINT8, 0x800);
	state->spriteram[1] = auto_alloc_array_clear(machine, UINT8, 0x800);

	tilemap_set_transparent_pen(state->bg_tilemap[0], 0);
	tilemap_set_transparent_pen(state->bg_tilemap[1], 0);
	tilemap_set_transparent_pen(state->textlayer, 0);

	tilemap_set_scroll_rows(state->textlayer, 32);

	state_save_register_global_pointer(machine, state->spriteram[0], 0x800);
	state_save_register_global_pointer(machine, state->spriteram[1], 0x800);
}

VIDEO_START( combatscb )
{
	combatsc_state *state = (combatsc_state *)machine->driver_data;

	state->bg_tilemap[0] = tilemap_create(machine, get_tile_info0_bootleg, tilemap_scan_rows, 8, 8, 32, 32);
	state->bg_tilemap[1] = tilemap_create(machine, get_tile_info1_bootleg, tilemap_scan_rows, 8, 8, 32, 32);
	state->textlayer = tilemap_create(machine, get_text_info_bootleg, tilemap_scan_rows, 8, 8, 32, 32);

	state->spriteram[0] = auto_alloc_array_clear(machine, UINT8, 0x800);
	state->spriteram[1] = auto_alloc_array_clear(machine, UINT8, 0x800);

	tilemap_set_transparent_pen(state->bg_tilemap[0], 0);
	tilemap_set_transparent_pen(state->bg_tilemap[1], 0);
	tilemap_set_transparent_pen(state->textlayer, 0);

	tilemap_set_scroll_rows(state->bg_tilemap[0], 32);
	tilemap_set_scroll_rows(state->bg_tilemap[1], 32);

	state_save_register_global_pointer(machine, state->spriteram[0], 0x800);
	state_save_register_global_pointer(machine, state->spriteram[1], 0x800);
}

/***************************************************************************

    Memory handlers

***************************************************************************/

READ8_HANDLER( combatsc_video_r )
{
	combatsc_state *state = (combatsc_state *)space->machine->driver_data;
	return state->videoram[offset];
}

WRITE8_HANDLER( combatsc_video_w )
{
	combatsc_state *state = (combatsc_state *)space->machine->driver_data;
	state->videoram[offset] = data;

	if (offset < 0x800)
	{
		if (state->video_circuit)
			tilemap_mark_tile_dirty(state->bg_tilemap[1], offset & 0x3ff);
		else
			tilemap_mark_tile_dirty(state->bg_tilemap[0], offset & 0x3ff);
	}
	else if (offset < 0x1000 && state->video_circuit == 0)
	{
		tilemap_mark_tile_dirty(state->textlayer, offset & 0x3ff);
	}
}

WRITE8_HANDLER( combatsc_pf_control_w )
{
	combatsc_state *state = (combatsc_state *)space->machine->driver_data;
	running_device *k007121 = state->video_circuit ? state->k007121_2 : state->k007121_1;
	k007121_ctrl_w(k007121, offset, data);

	if (offset == 7)
		tilemap_set_flip(state->bg_tilemap[state->video_circuit],(data & 0x08) ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	if (offset == 3)
	{
		if (data & 0x08)
			memcpy(state->spriteram[state->video_circuit], state->page[state->video_circuit] + 0x1000, 0x800);
		else
			memcpy(state->spriteram[state->video_circuit], state->page[state->video_circuit] + 0x1800, 0x800);
	}
}

READ8_HANDLER( combatsc_scrollram_r )
{
	combatsc_state *state = (combatsc_state *)space->machine->driver_data;
	return state->scrollram[offset];
}

WRITE8_HANDLER( combatsc_scrollram_w )
{
	combatsc_state *state = (combatsc_state *)space->machine->driver_data;
	state->scrollram[offset] = data;
}



/***************************************************************************

    Display Refresh

***************************************************************************/

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, const UINT8 *source, int circuit, UINT32 pri_mask )
{
	combatsc_state *state = (combatsc_state *)machine->driver_data;
	running_device *k007121 = circuit ? state->k007121_2 : state->k007121_1;
	int base_color = (circuit * 4) * 16 + (k007121_ctrlram_r(k007121, 6) & 0x10) * 2;

	k007121_sprites_draw(k007121, bitmap, cliprect, machine->gfx[circuit], machine->colortable, source, base_color, 0, 0, pri_mask);
}


VIDEO_UPDATE( combatsc )
{
	combatsc_state *state = (combatsc_state *)screen->machine->driver_data;
	int i;

	set_pens(screen->machine);

	if (k007121_ctrlram_r(state->k007121_1, 1) & 0x02)
	{
		tilemap_set_scroll_rows(state->bg_tilemap[0], 32);
		for (i = 0; i < 32; i++)
			tilemap_set_scrollx(state->bg_tilemap[0], i, state->scrollram0[i]);
	}
	else
	{
		tilemap_set_scroll_rows(state->bg_tilemap[0], 1);
		tilemap_set_scrollx(state->bg_tilemap[0], 0, k007121_ctrlram_r(state->k007121_1, 0) | ((k007121_ctrlram_r(state->k007121_1, 1) & 0x01) << 8));
	}

	if (k007121_ctrlram_r(state->k007121_2, 1) & 0x02)
	{
		tilemap_set_scroll_rows(state->bg_tilemap[1], 32);
		for (i = 0; i < 32; i++)
			tilemap_set_scrollx(state->bg_tilemap[1], i, state->scrollram1[i]);
	}
	else
	{
		tilemap_set_scroll_rows(state->bg_tilemap[1], 1);
		tilemap_set_scrollx(state->bg_tilemap[1], 0, k007121_ctrlram_r(state->k007121_2, 0) | ((k007121_ctrlram_r(state->k007121_2, 1) & 0x01) << 8));
	}

	tilemap_set_scrolly(state->bg_tilemap[0], 0, k007121_ctrlram_r(state->k007121_1, 2));
	tilemap_set_scrolly(state->bg_tilemap[1], 0, k007121_ctrlram_r(state->k007121_2, 2));

	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

	if (state->priority == 0)
	{
		tilemap_draw(bitmap, cliprect, state->bg_tilemap[1], TILEMAP_DRAW_OPAQUE | 0, 4);
		tilemap_draw(bitmap, cliprect, state->bg_tilemap[1], TILEMAP_DRAW_OPAQUE | 1, 8);
		tilemap_draw(bitmap, cliprect, state->bg_tilemap[0], 0, 1);
		tilemap_draw(bitmap, cliprect, state->bg_tilemap[0], 1, 2);

		/* we use the priority buffer so sprites are drawn front to back */
		draw_sprites(screen->machine, bitmap, cliprect, state->spriteram[1], 1, 0x0f00);
		draw_sprites(screen->machine, bitmap, cliprect, state->spriteram[0], 0, 0x4444);
	}
	else
	{
		tilemap_draw(bitmap, cliprect, state->bg_tilemap[0], TILEMAP_DRAW_OPAQUE | 0, 1);
		tilemap_draw(bitmap, cliprect, state->bg_tilemap[0], TILEMAP_DRAW_OPAQUE | 1, 2);
		tilemap_draw(bitmap, cliprect, state->bg_tilemap[1], 1, 4);
		tilemap_draw(bitmap, cliprect, state->bg_tilemap[1], 0, 8);

		/* we use the priority buffer so sprites are drawn front to back */
		draw_sprites(screen->machine, bitmap, cliprect, state->spriteram[1], 1, 0x0f00);
		draw_sprites(screen->machine, bitmap, cliprect, state->spriteram[0], 0, 0x4444);
	}

	if (k007121_ctrlram_r(state->k007121_1, 1) & 0x08)
	{
		for (i = 0; i < 32; i++)
		{
			tilemap_set_scrollx(state->textlayer, i, state->scrollram0[0x20 + i] ? 0 : TILE_LINE_DISABLED);
			tilemap_draw(bitmap, cliprect, state->textlayer, 0, 0);
		}
	}

	/* chop the extreme columns if necessary */
	if (k007121_ctrlram_r(state->k007121_1, 3) & 0x40)
	{
		rectangle clip;

		clip = *cliprect;
		clip.max_x = clip.min_x + 7;
		bitmap_fill(bitmap, &clip, 0);

		clip = *cliprect;
		clip.min_x = clip.max_x - 7;
		bitmap_fill(bitmap, &clip, 0);
	}
	return 0;
}








/***************************************************************************

    bootleg Combat School sprites. Each sprite has 5 bytes:

byte #0:    sprite number
byte #1:    y position
byte #2:    x position
byte #3:
    bit 0:      x position (bit 0)
    bits 1..3:  ???
    bit 4:      flip x
    bit 5:      unused?
    bit 6:      sprite bank # (bit 2)
    bit 7:      ???
byte #4:
    bits 0,1:   sprite bank # (bits 0 & 1)
    bits 2,3:   unused?
    bits 4..7:  sprite color

***************************************************************************/

static void bootleg_draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, const UINT8 *source, int circuit )
{
	const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);
	const gfx_element *gfx = machine->gfx[circuit + 2];

	int limit = circuit ? (memory_read_byte(space, 0xc2) * 256 + memory_read_byte(space, 0xc3)) : (memory_read_byte(space, 0xc0) * 256 + memory_read_byte(space, 0xc1));
	const UINT8 *finish;

	source += 0x1000;
	finish = source;
	source += 0x400;
	limit = (0x3400 - limit) / 8;
	if (limit >= 0)
		finish = source - limit * 8;
	source -= 8;

	while (source > finish)
	{
		UINT8 attributes = source[3]; /* PBxF ?xxX */
		{
			int number = source[0];
			int x = source[2] - 71 + (attributes & 0x01)*256;
			int y = 242 - source[1];
			UINT8 color = source[4]; /* CCCC xxBB */

			int bank = (color & 0x03) | ((attributes & 0x40) >> 4);

			number = ((number & 0x02) << 1) | ((number & 0x04) >> 1) | (number & (~6));
			number += 256 * bank;

			color = (circuit * 4) * 16 + (color >> 4);

			/*  hacks to select alternate palettes */
//          if(state->vreg == 0x40 && (attributes & 0x40)) color += 1*16;
//          if(state->vreg == 0x23 && (attributes & 0x02)) color += 1*16;
//          if(state->vreg == 0x66 ) color += 2*16;

			drawgfx_transpen( bitmap, cliprect, gfx,
				number, color,
				attributes & 0x10,0, /* flip */
				x,y, 15 );
		}
		source -= 8;
	}
}

VIDEO_UPDATE( combatscb )
{
	combatsc_state *state = (combatsc_state *)screen->machine->driver_data;
	int i;

	set_pens(screen->machine);

	for (i = 0; i < 32; i++)
	{
		tilemap_set_scrollx(state->bg_tilemap[0], i, state->io_ram[0x040 + i] + 5);
		tilemap_set_scrollx(state->bg_tilemap[1], i, state->io_ram[0x060 + i] + 3);
	}
	tilemap_set_scrolly(state->bg_tilemap[0], 0, state->io_ram[0x000]);
	tilemap_set_scrolly(state->bg_tilemap[1], 0, state->io_ram[0x020]);

	if (state->priority == 0)
	{
		tilemap_draw(bitmap, cliprect, state->bg_tilemap[1], TILEMAP_DRAW_OPAQUE,0);
		bootleg_draw_sprites(screen->machine, bitmap,cliprect, state->page[1], 1);

		tilemap_draw(bitmap, cliprect, state->bg_tilemap[0], 0 ,0);
		bootleg_draw_sprites(screen->machine, bitmap, cliprect, state->page[0], 0);
	}
	else
	{
		tilemap_draw(bitmap, cliprect, state->bg_tilemap[0], TILEMAP_DRAW_OPAQUE, 0);
		bootleg_draw_sprites(screen->machine, bitmap,cliprect, state->page[0], 0);

		tilemap_draw(bitmap,cliprect, state->bg_tilemap[1], 0, 0);
		bootleg_draw_sprites(screen->machine, bitmap,cliprect, state->page[1], 1);
	}

	tilemap_draw(bitmap, cliprect, state->textlayer, 0, 0);
	return 0;
}
