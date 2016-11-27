/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"

UINT8 *popeye_videoram;
UINT8 *popeye_colorram;
UINT8 *popeye_background_pos;
UINT8 *popeye_palettebank;
static UINT8 *popeye_bitmapram;
static const size_t popeye_bitmapram_size = 0x2000;

static bitmap_t *tmpbitmap2;
static UINT8 invertmask;
static UINT8 bitmap_type;
enum { TYPE_SKYSKIPR, TYPE_POPEYE };

static tilemap_t *fg_tilemap;

static UINT8 lastflip;

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Popeye has four color PROMS:
  - 32x8 char palette
  - 32x8 background palette
  - two 256x4 sprite palette

  The char and sprite PROMs are connected to the RGB output this way:

  bit 7 -- 220 ohm resistor  -- BLUE (inverted)
        -- 470 ohm resistor  -- BLUE (inverted)
        -- 220 ohm resistor  -- GREEN (inverted)
        -- 470 ohm resistor  -- GREEN (inverted)
        -- 1  kohm resistor  -- GREEN (inverted)
        -- 220 ohm resistor  -- RED (inverted)
        -- 470 ohm resistor  -- RED (inverted)
  bit 0 -- 1  kohm resistor  -- RED (inverted)

  The background PROM is connected to the RGB output this way:

  bit 7 -- 470 ohm resistor  -- BLUE (inverted)
        -- 680 ohm resistor  -- BLUE (inverted)  (1300 ohm in Sky Skipper)
        -- 470 ohm resistor  -- GREEN (inverted)
        -- 680 ohm resistor  -- GREEN (inverted)
        -- 1.2kohm resistor  -- GREEN (inverted)
        -- 470 ohm resistor  -- RED (inverted)
        -- 680 ohm resistor  -- RED (inverted)
  bit 0 -- 1.2kohm resistor  -- RED (inverted)

  The bootleg is the same, but the outputs are not inverted.

***************************************************************************/
static void convert_color_prom(running_machine *machine,const UINT8 *color_prom)
{
	int i;


	/* palette entries 0-15 are directly used by the background and changed at runtime */
	color_prom += 32;

	/* characters */
	for (i = 0;i < 16;i++)
	{
		int prom_offs = i | ((i & 8) << 1);	/* address bits 3 and 4 are tied together */
		int bit0,bit1,bit2,r,g,b;

		/* red component */
		bit0 = ((color_prom[prom_offs] ^ invertmask) >> 0) & 0x01;
		bit1 = ((color_prom[prom_offs] ^ invertmask) >> 1) & 0x01;
		bit2 = ((color_prom[prom_offs] ^ invertmask) >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = ((color_prom[prom_offs] ^ invertmask) >> 3) & 0x01;
		bit1 = ((color_prom[prom_offs] ^ invertmask) >> 4) & 0x01;
		bit2 = ((color_prom[prom_offs] ^ invertmask) >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = 0;
		bit1 = ((color_prom[prom_offs] ^ invertmask) >> 6) & 0x01;
		bit2 = ((color_prom[prom_offs] ^ invertmask) >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine,16 + (2 * i) + 1,MAKE_RGB(r,g,b));
	}

	color_prom += 32;

	/* sprites */
	for (i = 0;i < 256;i++)
	{
		int bit0,bit1,bit2,r,g,b;


		/* red component */
		bit0 = ((color_prom[0] ^ invertmask) >> 0) & 0x01;
		bit1 = ((color_prom[0] ^ invertmask) >> 1) & 0x01;
		bit2 = ((color_prom[0] ^ invertmask) >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = ((color_prom[0] ^ invertmask) >> 3) & 0x01;
		bit1 = ((color_prom[256] ^ invertmask) >> 0) & 0x01;
		bit2 = ((color_prom[256] ^ invertmask) >> 1) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = 0;
		bit1 = ((color_prom[256] ^ invertmask) >> 2) & 0x01;
		bit2 = ((color_prom[256] ^ invertmask) >> 3) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine,48+i,MAKE_RGB(r,g,b));

		color_prom++;
	}
}

PALETTE_INIT( popeye )
{
	invertmask = 0xff;

	convert_color_prom(machine,color_prom);
}

PALETTE_INIT( popeyebl )
{
	invertmask = 0x00;

	convert_color_prom(machine,color_prom);
}

static void set_background_palette(running_machine *machine,int bank)
{
	int i;
	UINT8 *color_prom = memory_region(machine, "proms") + 16 * bank;

	for (i = 0;i < 16;i++)
	{
		int bit0,bit1,bit2;
		int r,g,b;

		/* red component */
		bit0 = ((*color_prom ^ invertmask) >> 0) & 0x01;
		bit1 = ((*color_prom ^ invertmask) >> 1) & 0x01;
		bit2 = ((*color_prom ^ invertmask) >> 2) & 0x01;
		r = 0x1c * bit0 + 0x31 * bit1 + 0x47 * bit2;
		/* green component */
		bit0 = ((*color_prom ^ invertmask) >> 3) & 0x01;
		bit1 = ((*color_prom ^ invertmask) >> 4) & 0x01;
		bit2 = ((*color_prom ^ invertmask) >> 5) & 0x01;
		g = 0x1c * bit0 + 0x31 * bit1 + 0x47 * bit2;
		/* blue component */
		bit0 = 0;
		bit1 = ((*color_prom ^ invertmask) >> 6) & 0x01;
		bit2 = ((*color_prom ^ invertmask) >> 7) & 0x01;
		if (bitmap_type == TYPE_SKYSKIPR)
		{
			/* Sky Skipper has different weights */
			bit0 = bit1;
			bit1 = 0;
		}
		b = 0x1c * bit0 + 0x31 * bit1 + 0x47 * bit2;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));

		color_prom++;
	}
}

WRITE8_HANDLER( popeye_videoram_w )
{
	popeye_videoram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap, offset);
}

WRITE8_HANDLER( popeye_colorram_w )
{
	popeye_colorram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap, offset);
}

WRITE8_HANDLER( popeye_bitmap_w )
{
	int sx,sy,x,y,colour;

	popeye_bitmapram[offset] = data;

	if (bitmap_type == TYPE_SKYSKIPR)
	{
		sx = 8 * (offset % 128);
		sy = 8 * (offset / 128);

		if (flip_screen_get(space->machine))
			sy = 512-8 - sy;

		colour = data & 0x0f;
		for (y = 0; y < 8; y++)
		{
			for (x = 0; x < 8; x++)
			{
				*BITMAP_ADDR16(tmpbitmap2, sy+y, sx+x) = colour;
			}
		}
	}
	else
	{
		sx = 8 * (offset % 64);
		sy = 4 * (offset / 64);

		if (flip_screen_get(space->machine))
			sy = 512-4 - sy;

		colour = data & 0x0f;
		for (y = 0; y < 4; y++)
		{
			for (x = 0; x < 8; x++)
			{
				*BITMAP_ADDR16(tmpbitmap2, sy+y, sx+x) = colour;
			}
		}
	}
}

WRITE8_HANDLER( skyskipr_bitmap_w )
{
	offset = ((offset & 0xfc0) << 1) | (offset & 0x03f);
	if (data & 0x80)
		offset |= 0x40;

	popeye_bitmap_w(space,offset,data);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	int code = popeye_videoram[tile_index];
	int color = popeye_colorram[tile_index] & 0x0f;

	SET_TILE_INFO(0, code, color, 0);
}

VIDEO_START( skyskipr )
{
	popeye_bitmapram = auto_alloc_array(machine, UINT8, popeye_bitmapram_size);
	tmpbitmap2 = auto_bitmap_alloc(machine,1024,1024,machine->primary_screen->format());	/* actually 1024x512 but not rolling over vertically? */

	bitmap_type = TYPE_SKYSKIPR;

	fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 16, 16, 32, 32);
	tilemap_set_transparent_pen(fg_tilemap, 0);

    lastflip = 0;

    state_save_register_global(machine, lastflip);
    state_save_register_global_bitmap(machine, tmpbitmap2);
    state_save_register_global_pointer(machine, popeye_bitmapram, popeye_bitmapram_size);
}

VIDEO_START( popeye )
{
	popeye_bitmapram = auto_alloc_array(machine, UINT8, popeye_bitmapram_size);
	tmpbitmap2 = auto_bitmap_alloc(machine,512,512,machine->primary_screen->format());

	bitmap_type = TYPE_POPEYE;

	fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 16, 16, 32, 32);
	tilemap_set_transparent_pen(fg_tilemap, 0);

    lastflip = 0;

    state_save_register_global(machine, lastflip);
    state_save_register_global_bitmap(machine, tmpbitmap2);
    state_save_register_global_pointer(machine, popeye_bitmapram, popeye_bitmapram_size);
}

static void draw_background(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	int offs;
	const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);

	if (lastflip != flip_screen_get(machine))
	{
		for (offs = 0;offs < popeye_bitmapram_size;offs++)
			popeye_bitmap_w(space,offs,popeye_bitmapram[offs]);

		lastflip = flip_screen_get(machine);
	}

	set_background_palette(machine, (*popeye_palettebank & 0x08) >> 3);

	if (popeye_background_pos[1] == 0)	/* no background */
		bitmap_fill(bitmap,cliprect,0);
	else
	{
		/* copy the background graphics */
		int scrollx = 200 - popeye_background_pos[0] - 256*(popeye_background_pos[2]&1); /* ??? */
		int scrolly = 2 * (256 - popeye_background_pos[1]);

		if (bitmap_type == TYPE_SKYSKIPR)
			scrollx = 2*scrollx - 512;

		if (flip_screen_get(machine))
		{
			if (bitmap_type == TYPE_POPEYE)
				scrollx = -scrollx;
			scrolly = -scrolly;
		}

		copyscrollbitmap(bitmap,tmpbitmap2,1,&scrollx,1,&scrolly,cliprect);
	}
}

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	UINT8 *spriteram = machine->generic.spriteram.u8;
	int offs;

	for (offs = 0;offs < machine->generic.spriteram_size;offs += 4)
	{
		int code,color,flipx,flipy,sx,sy;

		/*
         * offs+3:
         * bit 7 ?
         * bit 6 ?
         * bit 5 ?
         * bit 4 MSB of sprite code
         * bit 3 vertical flip
         * bit 2 sprite bank
         * bit 1 \ color (with bit 2 as well)
         * bit 0 /
         */

		code = (spriteram[offs + 2] & 0x7f) + ((spriteram[offs + 3] & 0x10) << 3)
							+ ((spriteram[offs + 3] & 0x04) << 6);
		color = (spriteram[offs + 3] & 0x07) + 8*(*popeye_palettebank & 0x07);
		if (bitmap_type == TYPE_SKYSKIPR)
		{
			/* Two of the PROM address pins are tied together and one is not connected... */
			color = (color & 0x0f) | ((color & 0x08) << 1);
		}

		flipx = spriteram[offs + 2] & 0x80;
		flipy = spriteram[offs + 3] & 0x08;

		sx = 2*(spriteram[offs])-8;
		sy = 2*(256-spriteram[offs + 1]);

		if (flip_screen_get(machine))
		{
			flipx = !flipx;
			flipy = !flipy;
			sx = 496 - sx;
			sy = 496 - sy;
		}

		if (spriteram[offs] != 0)
			drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
					code ^ 0x1ff,
					color,
					flipx,flipy,
					sx,sy,0);
	}
}

VIDEO_UPDATE( popeye )
{
	draw_background(screen->machine, bitmap, cliprect);
	draw_sprites(screen->machine, bitmap, cliprect);
	tilemap_draw(bitmap, cliprect, fg_tilemap, 0, 0);
	return 0;
}
