/***************************************************************************

    video.c

    Functions to emulate the video hardware of the machine.

    This file is also used by scregg.c

***************************************************************************/

#include "emu.h"
#include "includes/btime.h"

/***************************************************************************

    Burger Time doesn't have a color PROM. It uses RAM to dynamically
    create the palette.
    The palette RAM is connected to the RGB output this way:

    bit 7 -- 15 kohm resistor  -- BLUE (inverted)
          -- 33 kohm resistor  -- BLUE (inverted)
          -- 15 kohm resistor  -- GREEN (inverted)
          -- 33 kohm resistor  -- GREEN (inverted)
          -- 47 kohm resistor  -- GREEN (inverted)
          -- 15 kohm resistor  -- RED (inverted)
          -- 33 kohm resistor  -- RED (inverted)
    bit 0 -- 47 kohm resistor  -- RED (inverted)

***************************************************************************/

PALETTE_INIT( btime )
{
	int i;


	/* Burger Time doesn't have a color PROM, but Hamburge has. */
	/* This function is also used by Eggs. */
	if (color_prom == 0) return;

	for (i = 0; i < machine->total_colors(); i++)
	{
		int bit0, bit1, bit2, r, g, b;

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

		palette_set_color(machine, i, MAKE_RGB(r,g,b));
	}
}

/***************************************************************************

    Convert the color PROMs into a more useable format.

    The PROM is connected to the RGB output this way:

    bit 7 -- 47 kohm resistor  -- RED
          -- 33 kohm resistor  -- RED
          -- 15 kohm resistor  -- RED
          -- 47 kohm resistor  -- GREEN
          -- 33 kohm resistor  -- GREEN
          -- 15 kohm resistor  -- GREEN
          -- 33 kohm resistor  -- BLUE
    bit 0 -- 15 kohm resistor  -- BLUE

***************************************************************************/

PALETTE_INIT( lnc )
{
	int i;

	for (i = 0; i < machine->total_colors(); i++)
	{
		int bit0, bit1, bit2, r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 7) & 0x01;
		bit1 = (color_prom[i] >> 6) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (color_prom[i] >> 4) & 0x01;
		bit1 = (color_prom[i] >> 3) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = 0;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 0) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine, i, MAKE_RGB(r,g,b));
	}
}

/***************************************************************************

Start the video hardware emulation.

***************************************************************************/

VIDEO_START( btime )
{
	btime_state *state = (btime_state *)machine->driver_data;

	if (machine->gfx[0]->srcdata == NULL)
		gfx_element_set_source(machine->gfx[0], state->deco_charram);
	if (machine->gfx[1]->srcdata == NULL)
		gfx_element_set_source(machine->gfx[1], state->deco_charram);
}


VIDEO_START( bnj )
{
	btime_state *state = (btime_state *)machine->driver_data;
	/* the background area is twice as wide as the screen */
	int width = 256;
	int height = 256;
	bitmap_format format = machine->primary_screen->format();
	state->background_bitmap = auto_bitmap_alloc(machine, 2 * width, height, format);

	state_save_register_global_bitmap(machine, state->background_bitmap);

	VIDEO_START_CALL(btime);
}


WRITE8_HANDLER( btime_paletteram_w )
{
	/* RGB output is inverted */
	paletteram_BBGGGRRR_w(space, offset, ~data);
}

WRITE8_HANDLER( lnc_videoram_w )
{
	btime_state *state = (btime_state *)space->machine->driver_data;
	state->videoram[offset] = data;
	state->colorram[offset] = *state->lnc_charbank;
}

READ8_HANDLER( btime_mirrorvideoram_r )
{
	btime_state *state = (btime_state *)space->machine->driver_data;
	int x, y;

	/* swap x and y coordinates */
	x = offset / 32;
	y = offset % 32;
	offset = 32 * y + x;

	return state->videoram[offset];
}

READ8_HANDLER( btime_mirrorcolorram_r )
{
	btime_state *state = (btime_state *)space->machine->driver_data;
	int x, y;

	/* swap x and y coordinates */
	x = offset / 32;
	y = offset % 32;
	offset = 32 * y + x;

	return state->colorram[offset];
}

WRITE8_HANDLER( btime_mirrorvideoram_w )
{
	btime_state *state = (btime_state *)space->machine->driver_data;
	int x, y;

	/* swap x and y coordinates */
	x = offset / 32;
	y = offset % 32;
	offset = 32 * y + x;

	state->videoram[offset] = data;
}

WRITE8_HANDLER( lnc_mirrorvideoram_w )
{
	int x, y;

	/* swap x and y coordinates */
	x = offset / 32;
	y = offset % 32;
	offset = 32 * y + x;

	lnc_videoram_w(space, offset, data);
}

WRITE8_HANDLER( btime_mirrorcolorram_w )
{
	btime_state *state = (btime_state *)space->machine->driver_data;
	int x, y;

	/* swap x and y coordinates */
	x = offset / 32;
	y = offset % 32;
	offset = 32 * y + x;

	state->colorram[offset] = data;
}

WRITE8_HANDLER( deco_charram_w )
{
	btime_state *state = (btime_state *)space->machine->driver_data;
	if (state->deco_charram[offset] == data)
		return;

	state->deco_charram[offset] = data;

	offset &= 0x1fff;

	/* dirty sprite */
	gfx_element_mark_dirty(space->machine->gfx[1], offset >> 5);

	/* diry char */
	gfx_element_mark_dirty(space->machine->gfx[0], offset >> 3);
}

WRITE8_HANDLER( bnj_background_w )
{
	btime_state *state = (btime_state *)space->machine->driver_data;
	state->bnj_backgroundram[offset] = data;
}

WRITE8_HANDLER( bnj_scroll1_w )
{
	btime_state *state = (btime_state *)space->machine->driver_data;
	state->bnj_scroll1 = data;
}

WRITE8_HANDLER( bnj_scroll2_w )
{
	btime_state *state = (btime_state *)space->machine->driver_data;
	state->bnj_scroll2 = data;
}

WRITE8_HANDLER( btime_video_control_w )
{
	// Btime video control
	//
	// Bit 0   = Flip screen
	// Bit 1-7 = Unknown

	flip_screen_set(space->machine, data & 0x01);
}

WRITE8_HANDLER( bnj_video_control_w )
{
	/* Bnj/Lnc works a little differently than the btime/eggs (apparently). */
	/* According to the information at: */
	/* http://www.davesclassics.com/arcade/Switch_Settings/BumpNJump.sw */
	/* SW8 is used for cocktail video selection (as opposed to controls), */
	/* but bit 7 of the input port is used for vblank input. */
	/* My guess is that this switch open circuits some connection to */
	/* the monitor hardware. */
	/* For now we just check 0x40 in DSW1, and ignore the write if we */
	/* are in upright controls mode. */

	if (input_port_read(space->machine, "DSW1") & 0x40) /* cocktail mode */
		btime_video_control_w(space, offset, data);
}

WRITE8_HANDLER( zoar_video_control_w )
{
	btime_state *state = (btime_state *)space->machine->driver_data;
	// Zoar video control
	//
	// Bit 0-2 = Unknown (always 0). Marked as MCOL on schematics
	// Bit 3-4 = Palette
	// Bit 7   = Flip Screen

	state->btime_palette = (data & 0x30) >> 3;

	if (input_port_read(space->machine, "DSW1") & 0x40) /* cocktail mode */
		flip_screen_set(space->machine, data & 0x80);
}

WRITE8_HANDLER( disco_video_control_w )
{
	btime_state *state = (btime_state *)space->machine->driver_data;
	state->btime_palette = (data >> 2) & 0x03;

	if (!(input_port_read(space->machine, "DSW1") & 0x40)) /* cocktail mode */
		flip_screen_set(space->machine, data & 0x01);
}


static void draw_chars( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, UINT8 transparency, UINT8 color, int priority )
{
	btime_state *state = (btime_state *)machine->driver_data;
	offs_t offs;

	for (offs = 0; offs < state->videoram_size; offs++)
	{
		UINT8 x = 31 - (offs / 32);
		UINT8 y = offs % 32;

		UINT16 code = state->videoram[offs] + 256 * (state->colorram[offs] & 3);

		/* check priority */
		if ((priority != -1) && (priority != ((code >> 7) & 0x01)))
			continue;

		if (flip_screen_get(machine))
		{
			x = 31 - x;
			y = 33 - y;
		}

		drawgfx_transpen(bitmap,cliprect,machine->gfx[0],
				code,
				color,
				flip_screen_get(machine),flip_screen_get(machine),
				8*x,8*y,
				transparency ? 0 : -1);
	}
}

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, UINT8 color,
						 UINT8 sprite_y_adjust, UINT8 sprite_y_adjust_flip_screen,
						 UINT8 *sprite_ram, offs_t interleave )
{
	int i;
	offs_t offs;

	/* draw the sprites */
	for (i = 0, offs = 0; i < 8; i++, offs += 4 * interleave)
	{
		int x, y;
		UINT8 flipx, flipy;

		if (!(sprite_ram[offs + 0] & 0x01)) continue;

		x = 240 - sprite_ram[offs + 3 * interleave];
		y = 240 - sprite_ram[offs + 2 * interleave];

		flipx = sprite_ram[offs + 0] & 0x04;
		flipy = sprite_ram[offs + 0] & 0x02;

		if (flip_screen_get(machine))
		{
			x = 240 - x;
			y = 256 - y + sprite_y_adjust_flip_screen;

			flipx = !flipx;
			flipy = !flipy;
		}

		y = y - sprite_y_adjust;

		drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
				sprite_ram[offs + interleave],
				color,
				flipx,flipy,
				x, y,0);

		y = y + (flip_screen_get(machine) ? -256 : 256);

		// Wrap around
		drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
				sprite_ram[offs + interleave],
				color,
				flipx,flipy,
				x,y,0);
	}
}


static void draw_background( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, UINT8* tmap, UINT8 color )
{
	btime_state *state = (btime_state *)machine->driver_data;
	int i;
	const UINT8 *gfx = memory_region(machine, "bg_map");
	int scroll = -(state->bnj_scroll2 | ((state->bnj_scroll1 & 0x03) << 8));

	// One extra iteration for wrap around
	for (i = 0; i < 5; i++, scroll += 256)
	{
		offs_t offs;
		offs_t tileoffset = tmap[i & 3] * 0x100;

		// Skip if this tile is completely off the screen
		if (scroll > 256)
			break;
		if (scroll < -256)
			continue;

		for (offs = 0; offs < 0x100; offs++)
		{
			int x = 240 - (16 * (offs / 16) + scroll) - 1;
			int y = 16 * (offs % 16);

			if (flip_screen_get(machine))
			{
				x = 240 - x;
				y = 256 - y;
			}

			drawgfx_opaque(bitmap, cliprect,machine->gfx[2],
					gfx[tileoffset + offs],
					color,
					flip_screen_get(machine),flip_screen_get(machine),
					x,y);
		}
	}
}


VIDEO_UPDATE( btime )
{
	btime_state *state = (btime_state *)screen->machine->driver_data;
	if (state->bnj_scroll1 & 0x10)
	{
		int i, start;

		// Generate tile map
		if (flip_screen_get(screen->machine))
			start = 0;
		else
			start = 1;

		for (i = 0; i < 4; i++)
		{
			state->btime_tilemap[i] = start | (state->bnj_scroll1 & 0x04);
			start = (start + 1) & 0x03;
		}

		draw_background(screen->machine, bitmap, cliprect, state->btime_tilemap, 0);
		draw_chars(screen->machine, bitmap, cliprect, TRUE, 0, -1);
	}
	else
		draw_chars(screen->machine, bitmap, cliprect, FALSE, 0, -1);

	draw_sprites(screen->machine, bitmap, cliprect, 0, 1, 0, state->videoram, 0x20);

	return 0;
}


VIDEO_UPDATE( eggs )
{
	btime_state *state = (btime_state *)screen->machine->driver_data;
	draw_chars(screen->machine, bitmap, cliprect, FALSE, 0, -1);
	draw_sprites(screen->machine, bitmap, cliprect, 0, 0, 0, state->videoram, 0x20);

	return 0;
}


VIDEO_UPDATE( lnc )
{
	btime_state *state = (btime_state *)screen->machine->driver_data;
	draw_chars(screen->machine, bitmap, cliprect, FALSE, 0, -1);
	draw_sprites(screen->machine, bitmap, cliprect, 0, 1, 2, state->videoram, 0x20);

	return 0;
}


VIDEO_UPDATE( zoar )
{
	btime_state *state = (btime_state *)screen->machine->driver_data;
	if (state->bnj_scroll1 & 0x04)
	{
		draw_background(screen->machine, bitmap, cliprect, state->zoar_scrollram, state->btime_palette);
		draw_chars(screen->machine, bitmap, cliprect, TRUE, state->btime_palette + 1, -1);
	}
	else
		draw_chars(screen->machine, bitmap, cliprect, FALSE, state->btime_palette + 1, -1);

	/* The order is important for correct priorities */
	draw_sprites(screen->machine, bitmap, cliprect, state->btime_palette + 1, 1, 2, state->videoram + 0x1f, 0x20);
	draw_sprites(screen->machine, bitmap, cliprect, state->btime_palette + 1, 1, 2, state->videoram, 0x20);

	return 0;
}


VIDEO_UPDATE( bnj )
{
	btime_state *state = (btime_state *)screen->machine->driver_data;
	if (state->bnj_scroll1)
	{
		int scroll, offs;

		for (offs = state->bnj_backgroundram_size - 1; offs >=0; offs--)
		{
			int sx, sy;

			sx = 16 * ((offs < 0x100) ? ((offs % 0x80) / 8) : ((offs % 0x80) / 8) + 16);
			sy = 16 * (((offs % 0x100) < 0x80) ? offs % 8 : (offs % 8) + 8);
			sx = 496 - sx;

			if (flip_screen_get(screen->machine))
			{
				sx = 496 - sx;
				sy = 256 - sy;
			}

			drawgfx_opaque(state->background_bitmap, 0, screen->machine->gfx[2],
					(state->bnj_backgroundram[offs] >> 4) + ((offs & 0x80) >> 3) + 32,
					0,
					flip_screen_get(screen->machine), flip_screen_get(screen->machine),
					sx, sy);
		}

		/* copy the background bitmap to the screen */
		scroll = (state->bnj_scroll1 & 0x02) * 128 + 511 - state->bnj_scroll2;
		if (!flip_screen_get(screen->machine))
			scroll = 767 - scroll;
		copyscrollbitmap(bitmap, state->background_bitmap, 1, &scroll, 0, 0, cliprect);

		/* copy the low priority characters followed by the sprites
           then the high priority characters */
		draw_chars(screen->machine, bitmap, cliprect, TRUE, 0, 1);
		draw_sprites(screen->machine, bitmap, cliprect, 0, 0, 0, state->videoram, 0x20);
		draw_chars(screen->machine, bitmap, cliprect, TRUE, 0, 0);
	}
	else
	{
		draw_chars(screen->machine, bitmap, cliprect, FALSE, 0, -1);
		draw_sprites(screen->machine, bitmap, cliprect, 0, 0, 0, state->videoram, 0x20);
	}

	return 0;
}


VIDEO_UPDATE( cookrace )
{
	btime_state *state = (btime_state *)screen->machine->driver_data;
	int offs;

	for (offs = state->bnj_backgroundram_size - 1; offs >=0; offs--)
	{
		int sx, sy;

		sx = 31 - (offs / 32);
		sy = offs % 32;

		if (flip_screen_get(screen->machine))
		{
			sx = 31 - sx;
			sy = 33 - sy;
		}

		drawgfx_opaque(bitmap, cliprect, screen->machine->gfx[2],
				state->bnj_backgroundram[offs],
				0,
				flip_screen_get(screen->machine), flip_screen_get(screen->machine),
				8*sx,8*sy);
	}

	draw_chars(screen->machine, bitmap, cliprect, TRUE, 0, -1);
	draw_sprites(screen->machine, bitmap, cliprect, 0, 1, 0, state->videoram, 0x20);

	return 0;
}


VIDEO_UPDATE( disco )
{
	btime_state *state = (btime_state *)screen->machine->driver_data;
	draw_chars(screen->machine, bitmap, cliprect, FALSE, state->btime_palette, -1);
	draw_sprites(screen->machine, bitmap, cliprect, state->btime_palette, 0, 0, state->spriteram, 1);

	return 0;
}
