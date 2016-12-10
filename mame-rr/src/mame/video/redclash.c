/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/ladybug.h"

/***************************************************************************

  Convert the color PROMs into a more useable format.

  I'm using the same palette conversion as Lady Bug, but the Zero Hour
  schematics show a different resistor network.

***************************************************************************/

PALETTE_INIT( redclash )
{
	int i;

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 0x40);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x20; i++)
	{
		int bit0, bit1;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 5) & 0x01;
		r = 0x47 * bit0 + 0x97 * bit1;

		/* green component */
		bit0 = (color_prom[i] >> 2) & 0x01;
		bit1 = (color_prom[i] >> 6) & 0x01;
		g = 0x47 * bit0 + 0x97 * bit1;

		/* blue component */
		bit0 = (color_prom[i] >> 4) & 0x01;
		bit1 = (color_prom[i] >> 7) & 0x01;
		b = 0x47 * bit0 + 0x97 * bit1;

		colortable_palette_set_color(machine->colortable, i, MAKE_RGB(r, g, b));
	}

	/* star colors */
	for (i = 0x20; i < 0x40; i++)
	{
		int bit0, bit1;
		int r, g, b;

		/* red component */
		bit0 = ((i - 0x20) >> 3) & 0x01;
		bit1 = ((i - 0x20) >> 4) & 0x01;
		b = 0x47 * bit0 + 0x97 * bit1;

		/* green component */
		bit0 = ((i - 0x20) >> 1) & 0x01;
		bit1 = ((i - 0x20) >> 2) & 0x01;
		g = 0x47 * bit0 + 0x97 * bit1;

		/* blue component */
		bit0 = ((i - 0x20) >> 0) & 0x01;
		r = 0x47 * bit0;

		colortable_palette_set_color(machine->colortable, i, MAKE_RGB(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x20;

	/* characters */
	for (i = 0; i < 0x20; i++)
	{
		UINT8 ctabentry = ((i << 3) & 0x18) | ((i >> 2) & 0x07);
		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}

	/* sprites */
	for (i = 0x20; i < 0x40; i++)
	{
		UINT8 ctabentry = color_prom[(i - 0x20) >> 1];

		ctabentry = BITSWAP8((color_prom[i - 0x20] >> 0) & 0x0f, 7,6,5,4,0,1,2,3);
		colortable_entry_set_value(machine->colortable, i + 0x00, ctabentry);

		ctabentry = BITSWAP8((color_prom[i - 0x20] >> 4) & 0x0f, 7,6,5,4,0,1,2,3);
		colortable_entry_set_value(machine->colortable, i + 0x20, ctabentry);
	}

	/* stars */
	for (i = 0x60; i < 0x80; i++)
		colortable_entry_set_value(machine->colortable, i, (i - 0x60) + 0x20);
}

WRITE8_HANDLER( redclash_videoram_w )
{
	ladybug_state *state = (ladybug_state *)space->machine->driver_data;

	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap, offset);
}

WRITE8_HANDLER( redclash_gfxbank_w )
{
	ladybug_state *state = (ladybug_state *)space->machine->driver_data;

	if (state->gfxbank != (data & 0x01))
	{
		state->gfxbank = data & 0x01;
		tilemap_mark_all_tiles_dirty_all(space->machine);
	}
}

WRITE8_HANDLER( redclash_flipscreen_w )
{
	flip_screen_set(space->machine, data & 0x01);
}

void redclash_set_stars_enable( running_machine *machine, UINT8 on ); //temp
void redclash_set_stars_speed( running_machine *machine, UINT8 speed );  //temp

/*
star_speed:
0 = unused
1 = unused
2 = forward fast
3 = forward medium
4 = forward slow
5 = backwards slow
6 = backwards medium
7 = backwards fast
*/
WRITE8_HANDLER( redclash_star0_w )
{
	ladybug_state *state = (ladybug_state *)space->machine->driver_data;

	state->star_speed = (state->star_speed & ~1) | ((data & 1) << 0);
	redclash_set_stars_speed(space->machine, state->star_speed);
}

WRITE8_HANDLER( redclash_star1_w )
{
	ladybug_state *state = (ladybug_state *)space->machine->driver_data;

	state->star_speed = (state->star_speed & ~2) | ((data & 1) << 1);
	redclash_set_stars_speed(space->machine, state->star_speed);
}

WRITE8_HANDLER( redclash_star2_w )
{
	ladybug_state *state = (ladybug_state *)space->machine->driver_data;

	state->star_speed = (state->star_speed & ~4) | ((data & 1) << 2);
	redclash_set_stars_speed(space->machine, state->star_speed);
}

WRITE8_HANDLER( redclash_star_reset_w )
{
	redclash_set_stars_enable(space->machine, 1);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	ladybug_state *state = (ladybug_state *)machine->driver_data;
	int code = state->videoram[tile_index];
	int color = (state->videoram[tile_index] & 0x70) >> 4; // ??

	SET_TILE_INFO(0, code, color, 0);
}

VIDEO_START( redclash )
{
	ladybug_state *state = (ladybug_state *)machine->driver_data;

	state->fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	tilemap_set_transparent_pen(state->fg_tilemap, 0);
}

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	ladybug_state *state = (ladybug_state *)machine->driver_data;
	UINT8 *spriteram = state->spriteram;
	int i, offs;

	for (offs = state->spriteram_size - 0x20; offs >= 0; offs -= 0x20)
	{
		i = 0;
		while (i < 0x20 && spriteram[offs + i] != 0)
			i += 4;

		while (i > 0)
		{
			i -= 4;

			if (spriteram[offs + i] & 0x80)
			{
				int color = spriteram[offs + i + 2] & 0x0f;
				int sx = spriteram[offs + i + 3];
				int sy = offs / 4 + (spriteram[offs + i] & 0x07);


				switch ((spriteram[offs + i] & 0x18) >> 3)
				{
					case 3:	/* 24x24 */
					{
						int code = ((spriteram[offs + i + 1] & 0xf0) >> 4) + ((state->gfxbank & 1) << 4);

						drawgfx_transpen(bitmap,cliprect,machine->gfx[3],
								code,
								color,
								0,0,
								sx,sy - 16,0);
						/* wraparound */
						drawgfx_transpen(bitmap,cliprect,machine->gfx[3],
								code,
								color,
								0,0,
								sx - 256,sy - 16,0);
						break;
					}

					case 2:	/* 16x16 */
						if (spriteram[offs + i] & 0x20)	/* zero hour spaceships */
						{
							int code = ((spriteram[offs + i + 1] & 0xf8) >> 3) + ((state->gfxbank & 1) << 5);
							int bank = (spriteram[offs + i + 1] & 0x02) >> 1;

							drawgfx_transpen(bitmap,cliprect,machine->gfx[4+bank],
									code,
									color,
									0,0,
									sx,sy - 16,0);
						}
						else
						{
							int code = ((spriteram[offs + i + 1] & 0xf0) >> 4) + ((state->gfxbank & 1) << 4);

							drawgfx_transpen(bitmap,cliprect,machine->gfx[2],
									code,
									color,
									0,0,
									sx,sy - 16,0);
						}
						break;

					case 1:	/* 8x8 */
						drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
								spriteram[offs + i + 1],// + 4 * (spriteram[offs + i + 2] & 0x10),
								color,
								0,0,
								sx,sy - 16,0);
						break;

					case 0:
						popmessage("unknown sprite size 0");
						break;
				}
			}
		}
	}
}

static void draw_bullets( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	ladybug_state *state = (ladybug_state *)machine->driver_data;
	int offs;

	for (offs = 0; offs < 0x20; offs++)
	{
//      sx = state->videoramoffs];
		int sx = 8 * offs + (state->videoram[offs] & 0x07);	/* ?? */
		int sy = 0xff - state->videoram[offs + 0x20];

		if (flip_screen_get(machine))
		{
			sx = 240 - sx;
		}

		if (sx >= cliprect->min_x && sx <= cliprect->max_x &&
			sy >= cliprect->min_y && sy <= cliprect->max_y)
			*BITMAP_ADDR16(bitmap, sy, sx) = 0x19;
	}
}

/*
 * These functions emulate the star generator board
 * All this comes from the schematics for Zero Hour
 *
 * It has a 17-bit LFSR which has a period of 2^17-1 clocks
 * (This is one pixel shy of "two screens" worth.)
 * So, there are two starfields drawn on alternate frames
 * These will scroll at a rate controlled by the speed register
 *
 * I'm basically doing the same thing by drawing each
 *  starfield on alternate frames, and then offseting them
 */

/* This line can reset the LFSR to zero and disables the star generator */
void redclash_set_stars_enable( running_machine *machine, UINT8 on )
{	ladybug_state *state = (ladybug_state *)machine->driver_data;

	if ((state->stars_enable == 0) && (on == 1))
	{
		state->stars_offset = 0;
	}

	state->stars_enable = on;
}

/* This sets up which starfield to draw and the offset, */
/* To be called from VIDEO_EOF() */

void redclash_update_stars_state( running_machine *machine )
{
	ladybug_state *state = (ladybug_state *)machine->driver_data;
	if (state->stars_enable == 0)
		return;

	state->stars_count++;
	state->stars_count %= 2;

	if (state->stars_count == 0)
	{
		state->stars_offset += ((state->stars_speed * 2) - 0x09);
		state->stars_offset %= 256 * 256;
		state->stars_state = 0;
	}
	else
		state->stars_state = 0x1fc71;
}

/* Set the speed register (3 bits) */

/*
 * 0 left/down fastest (-9/2 pix per frame)
 * 1 left/down faster  (-7/2 pix per frame)
 * 2 left/down fast    (-5/2 pix per frame)
 * 3 left/down medium  (-3/2 pix per frame)
 * 4 left/down slow    (-1/2 pix per frame)
 * 5 right/up slow     (+1/2 pix per frame)
 * 6 right/up medium   (+3/2 pix per frame)
 * 7 right/up fast     (+5/2 pix per frame)
 */

void redclash_set_stars_speed( running_machine *machine, UINT8 speed )
{
	ladybug_state *state = (ladybug_state *)machine->driver_data;
	state->stars_speed = speed;
}

/* Draw the stars */

/* Space Raider doesn't use the Va bit, and it is also set up to */
/* window the stars to a certain x range */

void redclash_draw_stars( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, UINT8 palette_offset, UINT8 sraider, UINT8 firstx, UINT8 lastx )
{
	ladybug_state *redclash = (ladybug_state *)machine->driver_data;
	int i;
	UINT8 tempbit, feedback, star_color, xloc, yloc;
	UINT32 state;
	UINT8 hcond, vcond;

	if (redclash->stars_enable == 0)
		return;

	state = redclash->stars_state;

	for(i = 0; i < 256 * 256; i++)
	{
		xloc = (redclash->stars_offset + i) % 256;
		yloc = ((redclash->stars_offset + i) /256 ) % 256;

		if ((state & 0x10000) == 0)
			tempbit = 1;
		else
			tempbit = 0;

		if ((state & 0x00020) != 0)
			feedback = tempbit ^ 1;
		else
			feedback = tempbit ^ 0;

		hcond = ((xloc + 8) & 0x10) >> 4;

		// sraider doesn't have Va hooked up
		if (sraider)
			vcond = 1;
		else
			vcond = yloc & 0x01;

		if (xloc >= cliprect->min_x && xloc <= cliprect->max_x && yloc >= cliprect->min_y && yloc <= cliprect->max_y)
		{
			if ((hcond ^ vcond) == 0)
			{
				/* enable condition */
				if (((state & 0x000ff) == 0x000ff) && (feedback == 0))
				{
					/* used by space raider */
					if ((xloc >= firstx) && (xloc <= lastx))
					{
						star_color = (state >> 9) & 0x1f;
						*BITMAP_ADDR16(bitmap, yloc, xloc) = palette_offset + star_color;
					}
				}
			}
		}

		/* update LFSR state */
		state = ((state << 1) & 0x1fffe) | feedback;
	}
}

VIDEO_EOF( redclash )
{
	redclash_update_stars_state(machine);
}

VIDEO_UPDATE( redclash )
{
	ladybug_state *state = (ladybug_state *)screen->machine->driver_data;

	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));
	redclash_draw_stars(screen->machine, bitmap, cliprect, 0x60, 0, 0x00, 0xff);
	draw_sprites(screen->machine, bitmap, cliprect);
	draw_bullets(screen->machine, bitmap, cliprect);
	tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0);
	return 0;
}
