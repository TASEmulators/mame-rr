/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/galaga.h"

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Xevious has three 256x4 palette PROMs (one per gun) and four 512x4 lookup
  table PROMs (two for sprites, two for background tiles; foreground
  characters map directly to a palette color without using a PROM).
  The palette PROMs are connected to the RGB output this way:

  bit 3 -- 220 ohm resistor  -- RED/GREEN/BLUE
        -- 470 ohm resistor  -- RED/GREEN/BLUE
        -- 1  kohm resistor  -- RED/GREEN/BLUE
  bit 0 -- 2.2kohm resistor  -- RED/GREEN/BLUE

***************************************************************************/
PALETTE_INIT( xevious )
{
	int i;
	#define TOTAL_COLORS(gfxn) (machine->gfx[gfxn]->total_colors * machine->gfx[gfxn]->color_granularity)

	machine->colortable = colortable_alloc(machine, 128+1);

	for (i = 0;i < 128;i++)
	{
		int bit0,bit1,bit2,bit3,r,g,b;

		/* red component */
		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		bit3 = (color_prom[0] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		/* green component */
		bit0 = (color_prom[256] >> 0) & 0x01;
		bit1 = (color_prom[256] >> 1) & 0x01;
		bit2 = (color_prom[256] >> 2) & 0x01;
		bit3 = (color_prom[256] >> 3) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		/* blue component */
		bit0 = (color_prom[2*256] >> 0) & 0x01;
		bit1 = (color_prom[2*256] >> 1) & 0x01;
		bit2 = (color_prom[2*256] >> 2) & 0x01;
		bit3 = (color_prom[2*256] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		colortable_palette_set_color(machine->colortable,i,MAKE_RGB(r,g,b));
		color_prom++;
	}

	/* color 0x80 is used by sprites to mark transparency */
	colortable_palette_set_color(machine->colortable,0x80,MAKE_RGB(0,0,0));

	color_prom += 128;  /* the bottom part of the PROM is unused */
	color_prom += 2*256;
	/* color_prom now points to the beginning of the lookup table */

	/* background tiles */
	for (i = 0;i < TOTAL_COLORS(1);i++)
	{
		colortable_entry_set_value(machine->colortable, machine->gfx[1]->color_base + i,
				(color_prom[0] & 0x0f) | ((color_prom[TOTAL_COLORS(1)] & 0x0f) << 4));

		color_prom++;
	}
	color_prom += TOTAL_COLORS(1);

	/* sprites */
	for (i = 0;i < TOTAL_COLORS(2);i++)
	{
		int c = (color_prom[0] & 0x0f) | ((color_prom[TOTAL_COLORS(2)] & 0x0f) << 4);

		colortable_entry_set_value(machine->colortable, machine->gfx[2]->color_base + i,
				(c & 0x80) ? (c & 0x7f) : 0x80);

		color_prom++;
	}
	color_prom += TOTAL_COLORS(2);

	/* foreground characters */
	for (i = 0;i < TOTAL_COLORS(0);i++)
	{
		colortable_entry_set_value(machine->colortable, machine->gfx[0]->color_base + i,
				(i % 2 != 0) ? (i / 2) : 0x80);
	}
}



PALETTE_INIT( battles )
{
	int i;

	machine->colortable = colortable_alloc(machine, 128+1);

	for (i = 0;i < 128;i++)
	{
		int bit0,bit1,bit2,bit3,r,g,b;

		/* red component */
		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		bit3 = (color_prom[0] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		/* green component */
		bit0 = (color_prom[256] >> 0) & 0x01;
		bit1 = (color_prom[256] >> 1) & 0x01;
		bit2 = (color_prom[256] >> 2) & 0x01;
		bit3 = (color_prom[256] >> 3) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		/* blue component */
		bit0 = (color_prom[2*256] >> 0) & 0x01;
		bit1 = (color_prom[2*256] >> 1) & 0x01;
		bit2 = (color_prom[2*256] >> 2) & 0x01;
		bit3 = (color_prom[2*256] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		colortable_palette_set_color(machine->colortable,i,MAKE_RGB(r,g,b));
		color_prom++;
	}

	/* color 0x80 is used by sprites to mark transparency */
	colortable_palette_set_color(machine->colortable,0x80,MAKE_RGB(0,0,0));

	color_prom += 128;  /* the bottom part of the PROM is unused */
	color_prom += 2*256;
	/* color_prom now points to the beginning of the lookup table */

	/* background tiles */
	for (i = 0;i < TOTAL_COLORS(1);i++)
	{
		colortable_entry_set_value(machine->colortable, machine->gfx[1]->color_base + i,
				(color_prom[0] & 0x0f) | ((color_prom[0x400] & 0x0f) << 4));

		color_prom++;
	}
	color_prom += 0x600;

	/* sprites */
	for (i = 0;i < TOTAL_COLORS(2);i++)
	{
		int c = (color_prom[0] & 0x0f) | ((color_prom[0x400] & 0x0f) << 4);

		colortable_entry_set_value(machine->colortable, machine->gfx[2]->color_base + i,
				(c & 0x80) ? (c & 0x7f) : 0x80);

		color_prom++;
	}

	/* foreground characters */
	for (i = 0;i < TOTAL_COLORS(0);i++)
	{
		colortable_entry_set_value(machine->colortable, machine->gfx[0]->color_base + i,
				(i % 2 != 0) ? (i / 2) : 0x80);
	}
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_fg_tile_info )
{
	_galaga_state *state = (_galaga_state *) machine->driver_data;

	UINT8 attr = state->xevious_fg_colorram[tile_index];

	/* the hardware has two character sets, one normal and one x-flipped. When
       screen is flipped, character y flip is done by the hardware inverting the
       timing signals, while x flip is done by selecting the 2nd character set.
       We reproduce this here, but since the tilemap system automatically flips
       characters when screen is flipped, we have to flip them back. */
	UINT8 color = ((attr & 0x03) << 4) | ((attr & 0x3c) >> 2);
	SET_TILE_INFO(
			0,
			state->xevious_fg_videoram[tile_index] | (flip_screen_get(machine) ? 0x100 : 0),
			color,
			TILE_FLIPYX((attr & 0xc0) >> 6) ^ (flip_screen_get(machine) ? TILE_FLIPX : 0));
}

static TILE_GET_INFO( get_bg_tile_info )
{
	_galaga_state *state = (_galaga_state *) machine->driver_data;

	UINT8 code = state->xevious_bg_videoram[tile_index];
	UINT8 attr = state->xevious_bg_colorram[tile_index];
	UINT8 color = ((attr & 0x3c) >> 2) | ((code & 0x80) >> 3) | ((attr & 0x03) << 5);
	SET_TILE_INFO(
			1,
			code + ((attr & 0x01) << 8),
			color,
			TILE_FLIPYX((attr & 0xc0) >> 6));
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( xevious )
{
	_galaga_state *state = (_galaga_state *) machine->driver_data;

	state->bg_tilemap = tilemap_create(machine, get_bg_tile_info,tilemap_scan_rows,     8,8,64,32);
	state->fg_tilemap = tilemap_create(machine, get_fg_tile_info,tilemap_scan_rows,8,8,64,32);

	tilemap_set_scrolldx(state->bg_tilemap,-20,288+27);
	tilemap_set_scrolldy(state->bg_tilemap,-16,-16);
	tilemap_set_scrolldx(state->fg_tilemap,-32,288+32);
	tilemap_set_scrolldy(state->fg_tilemap,-18,-10);
	tilemap_set_transparent_pen(state->fg_tilemap,0);
	state->xevious_bs[0] = 0;
	state->xevious_bs[1] = 0;

	state_save_register_global_array(machine, state->xevious_bs);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( xevious_fg_videoram_w )
{
	_galaga_state *state = (_galaga_state *) space->machine->driver_data;

	state->xevious_fg_videoram[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap,offset);
}

WRITE8_HANDLER( xevious_fg_colorram_w )
{
	_galaga_state *state = (_galaga_state *) space->machine->driver_data;

	state->xevious_fg_colorram[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap,offset);
}

WRITE8_HANDLER( xevious_bg_videoram_w )
{
	_galaga_state *state = (_galaga_state *) space->machine->driver_data;

	state->xevious_bg_videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap,offset);
}

WRITE8_HANDLER( xevious_bg_colorram_w )
{
	_galaga_state *state = (_galaga_state *) space->machine->driver_data;

	state->xevious_bg_colorram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap,offset);
}

WRITE8_HANDLER( xevious_vh_latch_w )
{
	_galaga_state *state = (_galaga_state *) space->machine->driver_data;

	int reg;
	int scroll = data + ((offset&0x01)<<8);   /* A0 -> D8 */

	reg = (offset&0xf0)>>4;

	switch (reg)
	{
	case 0:
		tilemap_set_scrollx(state->bg_tilemap,0,scroll);
		break;
	case 1:
		tilemap_set_scrollx(state->fg_tilemap,0,scroll);
		break;
	case 2:
		tilemap_set_scrolly(state->bg_tilemap,0,scroll);
		break;
	case 3:
		tilemap_set_scrolly(state->fg_tilemap,0,scroll);
		break;
	case 7:
		flip_screen_set(space->machine, scroll & 1);
		break;
   default:
		   logerror("CRTC WRITE REG: %x  Data: %03x\n",reg, scroll);
		   break;
	}
}


/* emulation for schematic 9B */
WRITE8_HANDLER( xevious_bs_w )
{
	_galaga_state *state = (_galaga_state *) space->machine->driver_data;

	state->xevious_bs[offset & 1] = data;
}

READ8_HANDLER( xevious_bb_r )
{
	_galaga_state *state = (_galaga_state *) space->machine->driver_data;

	UINT8 *rom2a = memory_region(space->machine, "gfx4");
	UINT8 *rom2b = rom2a+0x1000;
	UINT8 *rom2c = rom2a+0x3000;
	int adr_2b,adr_2c;
	int dat1,dat2;

	/* get BS to 12 bit data from 2A,2B */
	adr_2b = ((state->xevious_bs[1] & 0x7e) << 6) | ((state->xevious_bs[0] & 0xfe) >> 1);

	if (adr_2b & 1)
	{
		/* high bits select */
		dat1 = ((rom2a[adr_2b >> 1] & 0xf0) << 4) | rom2b[adr_2b];
	}
	else
	{
	    /* low bits select */
	    dat1 = ((rom2a[adr_2b >> 1] & 0x0f) << 8) | rom2b[adr_2b];
	}

	adr_2c = ((dat1 & 0x1ff) << 2) | ((state->xevious_bs[1] & 1) << 1) | (state->xevious_bs[0] & 1);
	if (dat1 & 0x400) adr_2c ^= 1;
	if (dat1 & 0x200) adr_2c ^= 2;

	if (offset & 1)
	{
		/* return BB1 */
		dat2 = rom2c[adr_2c | 0x800];
	}
	else
	{
		/* return BB0 */
		dat2 = rom2c[adr_2c];
		/* swap bit 6 & 7 */
		dat2 = BITSWAP8(dat2, 6,7,5,4,3,2,1,0);
		/* flip x & y */
		if (dat1 & 0x400) dat2 ^= 0x40;
		if (dat1 & 0x200) dat2 ^= 0x80;
	}
	return dat2;
}


/*
background pattern data

colorram mapping
b000-bfff background attribute
          bit 0-1 COL:palette set select
          bit 2-5 AN :color select
          bit 6   AFF:Y flip
          bit 7   PFF:X flip
c000-cfff background pattern name
          bit 0-7 PP0-7

seet 8A
                                        2     +-------+
COL0,1 -------------------------------------->|backg. |
                                        1     |color  |
PP7------------------------------------------>|replace|
                                        4     | ROM   |  6
AN0-3 --------------------------------------->|  4H   |-----> color code 6 bit
        1  +-----------+      +--------+      |  4F   |
COL0  ---->|B8   ROM 3C| 16   |custom  |  2   |       |
        8  |           |----->|shifter |----->|       |
PP0-7 ---->|B0-7 ROM 3D|      |16->2*8 |      |       |
           +-----------+      +--------+      +-------+

font rom controller
       1  +-----+     +--------+
ANF   --->| ROM |  8  |shift   |  1
       8  | 3B  |---->|reg     |-----> font data
PP0-7 --->|     |     |8->1*8  |
          +-----+     +--------+

font color ( not use color map )
        2  |
COL0-1 --->|  color code 6 bit
        4  |
AN0-3  --->|

sprite

ROM 3M,3L color replace table for sprite

*/




/***************************************************************************

  Display refresh

***************************************************************************/

static void draw_sprites(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect)
{
	_galaga_state *state = (_galaga_state *) machine->driver_data;

	UINT8 *spriteram = state->xevious_sr3 + 0x780;
	UINT8 *spriteram_2 = state->xevious_sr1 + 0x780;
	UINT8 *spriteram_3 = state->xevious_sr2 + 0x780;
	int offs,sx,sy;

	for (offs = 0;offs < 0x80;offs += 2)
	{
		if ((spriteram[offs + 1] & 0x40) == 0)  /* I'm not sure about this one */
		{
			int bank,code,color,flipx,flipy;
			UINT32 transmask;

			if (spriteram_3[offs] & 0x80)
			{
				bank = 2;
				code = (spriteram[offs] & 0x3f) + 0x100;
			}
			else
			{
				bank = 2;
				code = spriteram[offs];
			}

			color = spriteram[offs + 1] & 0x7f;
			flipx = spriteram_3[offs] & 4;
			flipy = spriteram_3[offs] & 8;

			sx = spriteram_2[offs + 1] - 40 + 0x100*(spriteram_3[offs + 1] & 1);
			sy = 28*8-spriteram_2[offs]-1;

			if (flip_screen_get(machine))
			{
				flipx = !flipx;
				flipy = !flipy;
				sy += 48;
			}

			transmask = colortable_get_transpen_mask(machine->colortable, machine->gfx[bank], color, 0x80);

			if (spriteram_3[offs] & 2)  /* double height (?) */
			{
				if (spriteram_3[offs] & 1)  /* double width, double height */
				{
					code &= ~3;
					drawgfx_transmask(bitmap,cliprect,machine->gfx[bank],
							code+3,color,flipx,flipy,
							flipx ? sx : sx+16,flipy ? sy-16 : sy,transmask);
					drawgfx_transmask(bitmap,cliprect,machine->gfx[bank],
							code+1,color,flipx,flipy,
							flipx ? sx : sx+16,flipy ? sy : sy-16,transmask);
				}
				code &= ~2;
				drawgfx_transmask(bitmap,cliprect,machine->gfx[bank],
						code+2,color,flipx,flipy,
						flipx ? sx+16 : sx,flipy ? sy-16 : sy,transmask);
				drawgfx_transmask(bitmap,cliprect,machine->gfx[bank],
						code,color,flipx,flipy,
						flipx ? sx+16 : sx,flipy ? sy : sy-16,transmask);
			}
			else if (spriteram_3[offs] & 1) /* double width */
			{
				code &= ~1;
				drawgfx_transmask(bitmap,cliprect,machine->gfx[bank],
						code,color,flipx,flipy,
						flipx ? sx+16 : sx,flipy ? sy-16 : sy,transmask);
				drawgfx_transmask(bitmap,cliprect,machine->gfx[bank],
						code+1,color,flipx,flipy,
						flipx ? sx : sx+16,flipy ? sy-16 : sy,transmask);
			}
			else	/* normal */
			{
				drawgfx_transmask(bitmap,cliprect,machine->gfx[bank],
						code,color,flipx,flipy,sx,sy,transmask);
			}
		}
	}
}


VIDEO_UPDATE( xevious )
{
	_galaga_state *state = (_galaga_state *) screen->machine->driver_data;

	tilemap_draw(bitmap,cliprect,state->bg_tilemap,0,0);
	draw_sprites(screen->machine, bitmap,cliprect);
	tilemap_draw(bitmap,cliprect,state->fg_tilemap,0,0);
	return 0;
}
