#include "emu.h"
#include "includes/galaga.h"


/***************************************************************************

  Convert the color PROMs.

  digdug has one 32x8 palette PROM and two 256x4 color lookup table PROMs
  (one for characters, one for sprites).
  The palette PROM is connected to the RGB output this way:

  bit 7 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

***************************************************************************/

PALETTE_INIT( digdug )
{
	int i;

	machine->colortable = colortable_alloc(machine, 32);

	for (i = 0;i < 32;i++)
	{
		int bit0,bit1,bit2,r,g,b;

		bit0 = (*color_prom >> 0) & 0x01;
		bit1 = (*color_prom >> 1) & 0x01;
		bit2 = (*color_prom >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (*color_prom >> 3) & 0x01;
		bit1 = (*color_prom >> 4) & 0x01;
		bit2 = (*color_prom >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = 0;
		bit1 = (*color_prom >> 6) & 0x01;
		bit2 = (*color_prom >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		colortable_palette_set_color(machine->colortable,i,MAKE_RGB(r,g,b));
		color_prom++;
	}

	/* characters - direct mapping */
	for (i = 0; i < 16; i++)
	{
		colortable_entry_set_value(machine->colortable, i*2+0, 0);
		colortable_entry_set_value(machine->colortable, i*2+1, i);
	}

	/* sprites */
	for (i = 0;i < 0x100;i++)
		colortable_entry_set_value(machine->colortable, 16*2+i, (*color_prom++ & 0x0f) + 0x10);

	/* bg_select */
	for (i = 0;i < 0x100;i++)
		colortable_entry_set_value(machine->colortable, 16*2+256+i, *color_prom++ & 0x0f);
}



/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

/* convert from 32x32 to 36x28 */
static TILEMAP_MAPPER( tilemap_scan )
{
	int offs;

	row += 2;
	col -= 2;
	if (col & 0x20)
		offs = row + ((col & 0x1f) << 5);
	else
		offs = col + (row << 5);

	return offs;
}


static TILE_GET_INFO( bg_get_tile_info )
{
	UINT8 *rom = memory_region(machine, "gfx4");
	_galaga_state *state = (_galaga_state *) machine->driver_data;

	int code = rom[tile_index | (state->bg_select << 10)];
	/* when the background is "disabled", it is actually still drawn, but using
       a color code that makes all pixels black. There are pullups setting the
       code to 0xf, but also solder pads that optionally connect the lines with
       tilemap RAM, therefore allowing to pick some bits of the color code from
       the top 4 bits of alpha code. This feature is not used by Dig Dug. */
	int color = state->bg_disable ? 0xf : (code >> 4);
	SET_TILE_INFO(
			2,
			code,
			color | state->bg_color_bank,
			0);
}

static TILE_GET_INFO( tx_get_tile_info )
{
	_galaga_state *state = (_galaga_state *) machine->driver_data;
	UINT8 code = state->videoram[tile_index];
	int color;

	/* the hardware has two ways to pick the color, either straight from the
       bottom 4 bits of the character code, or from the top 4 bits through a
       formula. The former method isnot used by Dig Dug and seems kind of
       useless (I don't know what use they were thinking of when they added
       it), anyway here it is reproduced faithfully. */
	if (state->tx_color_mode)
		color = code & 0x0f;
	else
		color = ((code >> 4) & 0x0e) | ((code >> 3) & 2);

	/* the hardware has two character sets, one normal and one x-flipped. When
       screen is flipped, character y flip is done by the hardware inverting the
       timing signals, while x flip is done by selecting the 2nd character set.
       We reproduce this here, but since the tilemap system automatically flips
       characters when screen is flipped, we have to flip them back. */
	SET_TILE_INFO(
			0,
			(code & 0x7f) | (flip_screen_get(machine) ? 0x80 : 0),
			color,
			flip_screen_get(machine) ? TILE_FLIPX : 0);
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( digdug )
{
	_galaga_state *state = (_galaga_state *) machine->driver_data;

	state->bg_tilemap = tilemap_create(machine, bg_get_tile_info,tilemap_scan,     8,8,36,28);
	state->fg_tilemap = tilemap_create(machine, tx_get_tile_info,tilemap_scan,8,8,36,28);

	tilemap_set_transparent_pen(state->fg_tilemap, 0);

	state_save_register_global(machine, state->bg_select);
	state_save_register_global(machine, state->tx_color_mode);
	state_save_register_global(machine, state->bg_disable);
	state_save_register_global(machine, state->bg_color_bank);
}



/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( digdug_videoram_w )
{
	_galaga_state *state = (_galaga_state *) space->machine->driver_data;

	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap,offset & 0x3ff);
}

WRITE8_HANDLER( digdug_PORT_w )
{
	_galaga_state *state = (_galaga_state *) space->machine->driver_data;

	switch (offset)
	{
		case 0:	/* select background picture */
		case 1:
			{
				int shift = offset;
				int mask = 1 << shift;

				if ((state->bg_select & mask) != ((data & 1) << shift))
				{
					state->bg_select = (state->bg_select & ~mask) | ((data & 1) << shift);
					tilemap_mark_all_tiles_dirty(state->bg_tilemap);
				}
			}
			break;

		case 2:	/* select alpha layer color mode (see tx_get_tile_info) */
			if (state->tx_color_mode != (data & 1))
			{
				state->tx_color_mode = data & 1;
				tilemap_mark_all_tiles_dirty(state->fg_tilemap);
			}
			break;

		case 3:	/* "disable" background (see bg_get_tile_info) */
			if (state->bg_disable != (data & 1))
			{
				state->bg_disable = data & 1;
				tilemap_mark_all_tiles_dirty(state->bg_tilemap);
			}
			break;

		case 4:	/* background color bank */
		case 5:
			{
				int shift = offset;
				int mask = 1 << shift;

				if ((state->bg_color_bank & mask) != ((data & 1) << shift))
				{
					state->bg_color_bank = (state->bg_color_bank & ~mask) | ((data & 1) << shift);
					tilemap_mark_all_tiles_dirty(state->bg_tilemap);
				}
			}
			break;

		case 6:	/* n.c. */
			break;

		case 7:	/* FLIP */
			flip_screen_set(space->machine, data & 1);
			break;
	}
}



/***************************************************************************

  Display refresh

***************************************************************************/

static const rectangle spritevisiblearea =
{
	2*8, 34*8-1,
	0*8, 28*8-1
};

static void draw_sprites(running_machine* machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	_galaga_state *state = (_galaga_state *) machine->driver_data;
	UINT8 *spriteram = state->digdug_objram + 0x380;
	UINT8 *spriteram_2 = state->digdug_posram + 0x380;
	UINT8 *spriteram_3 = state->digdug_flpram + 0x380;
	int offs;

	for (offs = 0;offs < 0x80;offs += 2)
	{
		static const int gfx_offs[2][2] =
		{
			{ 0, 1 },
			{ 2, 3 }
		};
		int sprite = spriteram[offs];
		int color = spriteram[offs+1] & 0x3f;
		int sx = spriteram_2[offs+1] - 40+1;
		int sy = 256 - spriteram_2[offs] + 1;	// sprites are buffered and delayed by one scanline
		int flipx = (spriteram_3[offs] & 0x01);
		int flipy = (spriteram_3[offs] & 0x02) >> 1;
		int size  = (sprite & 0x80) >> 7;
		int x,y;

		if (size)
			sprite = (sprite & 0xc0) | ((sprite & ~0xc0) << 2);

		sy -= 16 * size;
		sy = (sy & 0xff) - 32;	// fix wraparound

		if (flip_screen_get(machine))
		{
			flipx ^= 1;
			flipy ^= 1;
			sy += 48;
		}

		for (y = 0;y <= size;y++)
		{
			for (x = 0;x <= size;x++)
			{
				UINT32 transmask = colortable_get_transpen_mask(machine->colortable, machine->gfx[1], color, 0x1f);
				drawgfx_transmask(bitmap,&spritevisiblearea,machine->gfx[1],
					sprite + gfx_offs[y ^ (size * flipy)][x ^ (size * flipx)],
					color,
					flipx,flipy,
					((sx + 16*x) & 0xff), sy + 16*y,transmask);
				/* wraparound */
				drawgfx_transmask(bitmap,&spritevisiblearea,machine->gfx[1],
					sprite + gfx_offs[y ^ (size * flipy)][x ^ (size * flipx)],
					color,
					flipx,flipy,
					((sx + 16*x) & 0xff) + 0x100, sy + 16*y,transmask);
			}
		}
	}
}


VIDEO_UPDATE( digdug )
{
	_galaga_state *state = (_galaga_state *) screen->machine->driver_data;

	tilemap_draw(bitmap,cliprect,state->bg_tilemap,0,0);
	tilemap_draw(bitmap,cliprect,state->fg_tilemap,0,0);
	draw_sprites(screen->machine,bitmap,cliprect);
	return 0;
}
