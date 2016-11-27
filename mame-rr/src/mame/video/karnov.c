/*******************************************************************************

    Karnov - Bryan McPhail, mish@tendril.co.uk

*******************************************************************************/

#include "emu.h"
#include "includes/karnov.h"

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Karnov has two 1024x8 palette PROM.
  I don't know the exact values of the resistors between the RAM and the
  RGB output. I assumed these values (the same as Commando)

  bit 7 -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 2.2kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
        -- 1  kohm resistor  -- RED
  bit 0 -- 2.2kohm resistor  -- RED

  bit 7 -- unused
        -- unused
        -- unused
        -- unused
        -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 1  kohm resistor  -- BLUE
  bit 0 -- 2.2kohm resistor  -- BLUE

***************************************************************************/

PALETTE_INIT( karnov )
{
	int i;

	for (i = 0; i < machine->total_colors(); i++)
	{
		int bit0, bit1, bit2, bit3, r, g, b;

		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		bit3 = (color_prom[0] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[0] >> 4) & 0x01;
		bit1 = (color_prom[0] >> 5) & 0x01;
		bit2 = (color_prom[0] >> 6) & 0x01;
		bit3 = (color_prom[0] >> 7) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[machine->total_colors()] >> 0) & 0x01;
		bit1 = (color_prom[machine->total_colors()] >> 1) & 0x01;
		bit2 = (color_prom[machine->total_colors()] >> 2) & 0x01;
		bit3 = (color_prom[machine->total_colors()] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
		color_prom++;
	}
}

void karnov_flipscreen_w( running_machine *machine, int data )
{
	karnov_state *state = (karnov_state *)machine->driver_data;
	state->flipscreen = data;
	tilemap_set_flip_all(machine, state->flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
}

static void draw_background( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	karnov_state *state = (karnov_state *)machine->driver_data;
	int my, mx, offs, color, tile, fx, fy;
	int scrollx = state->scroll[0];
	int scrolly = state->scroll[1];

	if (state->flipscreen)
		fx = fy = 1;
	else
		fx = fy = 0;

	mx = -1;
	my = 0;

	for (offs = 0; offs < 0x400; offs ++)
	{
		mx++;
		if (mx == 32)
		{
			mx=0;
			my++;
		}

		tile = state->pf_data[offs];
		color = tile >> 12;
		tile = tile & 0x7ff;
		if (state->flipscreen)
			drawgfx_opaque(state->bitmap_f, 0, machine->gfx[1],tile,
				color, fx, fy, 496-16*mx,496-16*my);
		else
			drawgfx_opaque(state->bitmap_f, 0, machine->gfx[1],tile,
				color, fx, fy, 16*mx,16*my);
	}

	if (!state->flipscreen)
	{
		scrolly = -scrolly;
		scrollx = -scrollx;
	}
	else
	{
		scrolly = scrolly + 256;
		scrollx = scrollx + 256;
	}

	copyscrollbitmap(bitmap, state->bitmap_f, 1, &scrollx, 1, &scrolly, cliprect);
}

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	karnov_state *state = (karnov_state *)machine->driver_data;
	UINT16 *buffered_spriteram16 = machine->generic.buffered_spriteram.u16;
	int offs;

	for (offs = 0; offs <0x800; offs += 4)
	{
		int x, y, sprite, sprite2, colour, fx, fy, extra;

		y = buffered_spriteram16[offs];
		if (!(y & 0x8000))
			continue;

		y = y & 0x1ff;
		sprite = buffered_spriteram16[offs + 3];
		colour = sprite >> 12;
		sprite = sprite & 0xfff;
		x = buffered_spriteram16[offs + 2] & 0x1ff;

		fx = buffered_spriteram16[offs + 1];
		extra = (fx & 0x10) ? 1 : 0;
		fy = fx & 0x2;
		fx = fx & 0x4;

		if (extra)
			y = y + 16;

		/* Convert the co-ords..*/
		x = (x + 16) % 0x200;
		y = (y + 16) % 0x200;
		x = 256 - x;
		y = 256 - y;
		if (state->flipscreen)
		{
			y = 240 - y;
			x = 240 - x;
			if (fx) fx = 0; else fx = 1;
			if (fy) fy = 0; else fy = 1;
			if (extra) y = y - 16;
		}

		/* Y Flip determines order of multi-sprite */
		if (extra && fy)
		{
			sprite2 = sprite;
			sprite++;
		}
		else
			sprite2 = sprite + 1;

		drawgfx_transpen(bitmap,cliprect,machine->gfx[2],
				sprite,
				colour,fx,fy,x,y,0);

    	/* 1 more sprite drawn underneath */
    	if (extra)
    		drawgfx_transpen(bitmap,cliprect,machine->gfx[2],
				sprite2,
				colour,fx,fy,x,y+16,0);
	}
}

/******************************************************************************/

VIDEO_UPDATE( karnov )
{
	karnov_state *state = (karnov_state *)screen->machine->driver_data;
	draw_background(screen->machine, bitmap, cliprect);
	draw_sprites(screen->machine, bitmap, cliprect);
	tilemap_draw(bitmap, cliprect, state->fix_tilemap, 0, 0);
	return 0;
}

/******************************************************************************/

static TILE_GET_INFO( get_fix_tile_info )
{
	karnov_state *state = (karnov_state *)machine->driver_data;
	int tile = state->videoram[tile_index];
	SET_TILE_INFO(
			0,
			tile&0xfff,
			tile>>14,
			0);
}

WRITE16_HANDLER( karnov_videoram_w )
{
	karnov_state *state = (karnov_state *)space->machine->driver_data;
	COMBINE_DATA(&state->videoram[offset]);
	tilemap_mark_tile_dirty(state->fix_tilemap, offset);
}

WRITE16_HANDLER( karnov_playfield_swap_w )
{
	karnov_state *state = (karnov_state *)space->machine->driver_data;
	offset = ((offset & 0x1f) << 5) | ((offset & 0x3e0) >> 5);
	COMBINE_DATA(&state->pf_data[offset]);
}

/******************************************************************************/

VIDEO_START( karnov )
{
	karnov_state *state = (karnov_state *)machine->driver_data;

	/* Allocate bitmap & tilemap */
	state->bitmap_f = auto_bitmap_alloc(machine, 512, 512, machine->primary_screen->format());
	state->fix_tilemap = tilemap_create(machine, get_fix_tile_info, tilemap_scan_rows, 8, 8, 32, 32);

	state_save_register_global_bitmap(machine, state->bitmap_f);

	tilemap_set_transparent_pen(state->fix_tilemap, 0);
}

VIDEO_START( wndrplnt )
{
	karnov_state *state = (karnov_state *)machine->driver_data;

	/* Allocate bitmap & tilemap */
	state->bitmap_f = auto_bitmap_alloc(machine, 512, 512, machine->primary_screen->format());
	state->fix_tilemap = tilemap_create(machine, get_fix_tile_info, tilemap_scan_cols, 8, 8, 32, 32);

	state_save_register_global_bitmap(machine, state->bitmap_f);

	tilemap_set_transparent_pen(state->fix_tilemap, 0);
}

/******************************************************************************/
