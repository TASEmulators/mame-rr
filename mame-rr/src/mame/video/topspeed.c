#include "emu.h"
#include "video/taitoic.h"
#include "includes/topspeed.h"


/****************************************************************************

                                     SPRITES

    Layout 8 bytes per sprite
    -------------------------

    +0x00   xxxxxxx. ........   Zoom Y
            .......x xxxxxxxx   Y

    +0x02   x....... ........   Flip Y
            ........ .xxxxxxx   Zoom X

    +0x04   x....... ........   Priority
            .x...... ........   Flip X
            ..x..... ........   Unknown
            .......x xxxxxxxx   X

    +0x06   xxxxxxxx ........   Color
            ........ xxxxxxxx   Tile number

********************************************************************************/

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	topspeed_state *state = (topspeed_state *)machine->driver_data;
	UINT16 *spriteram = state->spriteram;
	int offs, map_offset, x, y, curx, cury, sprite_chunk;
	UINT16 *spritemap = state->spritemap;
	UINT16 data, tilenum, code, color;
	UINT8 flipx, flipy, priority, bad_chunks;
	UINT8 j, k, px, py, zx, zy, zoomx, zoomy;
	static const int primasks[2] = { 0xff00, 0xfffc };	/* Sprites are over bottom layer or under top layer */

	/* Most of spriteram is not used by the 68000: rest is scratch space for the h/w perhaps ? */
	for (offs = 0; offs < (0x2c0 / 2); offs += 4)
	{
		data = spriteram[offs + 2];

		tilenum = spriteram[offs + 3] & 0xff;
		color = (spriteram[offs + 3] & 0xff00) >> 8;
		flipx = (data & 0x4000) >> 14;
		flipy = (spriteram[offs + 1] & 0x8000) >> 15;
		x = data & 0x1ff;
		y = spriteram[offs] & 0x1ff;
		zoomx = (spriteram[offs + 1]& 0x7f);
		zoomy = (spriteram[offs] & 0xfe00) >> 9;
		priority = (data & 0x8000) >> 15;
//      unknown = (data & 0x2000) >> 13;

		if (y == 0x180)
			continue;	/* dead sprite */

		map_offset = tilenum << 7;

		zoomx += 1;
		zoomy += 1;

		y += 3 + (128-zoomy);

		/* treat coords as signed */
		if (x > 0x140) x -= 0x200;
		if (y > 0x140) y -= 0x200;

		bad_chunks = 0;

		for (sprite_chunk = 0; sprite_chunk < 128; sprite_chunk++)
		{
			k = sprite_chunk % 8;   /* 8 sprite chunks per row */
			j = sprite_chunk / 8;   /* 16 rows */

			/* pick tiles back to front for x and y flips */
			px = (flipx) ?  (7 - k) : (k);
			py = (flipy) ? (15 - j) : (j);

			code = spritemap[map_offset + (py << 3) + px];

			if (code & 0x8000)
			{
				bad_chunks += 1;
				continue;
			}

			curx = x + ((k * zoomx) / 8);
			cury = y + ((j * zoomy) / 16);

			zx = x + (((k + 1) * zoomx) / 8) - curx;
			zy = y + (((j + 1) * zoomy) / 16) - cury;

			pdrawgfxzoom_transpen(bitmap,cliprect,machine->gfx[0],
					code,
					color,
					flipx,flipy,
					curx,cury,
					zx<<12,zy<<13,
					machine->priority_bitmap,primasks[priority],0);
		}

		if (bad_chunks)
			logerror("Sprite number %04x had %02x invalid chunks\n",tilenum,bad_chunks);
	}
}


/***************************************************************************/

VIDEO_UPDATE( topspeed )
{
	topspeed_state *state = (topspeed_state *)screen->machine->driver_data;
	UINT8 layer[4];

#ifdef MAME_DEBUG
	static UINT8 dislayer[5];
#endif

#ifdef MAME_DEBUG
	if (input_code_pressed_once (screen->machine, KEYCODE_V))
	{
		dislayer[0] ^= 1;
		popmessage("bg: %01x", dislayer[0]);
	}

	if (input_code_pressed_once (screen->machine, KEYCODE_B))
	{
		dislayer[1] ^= 1;
		popmessage("fg: %01x", dislayer[1]);
	}

	if (input_code_pressed_once (screen->machine, KEYCODE_N))
	{
		dislayer[2] ^= 1;
		popmessage("bg2: %01x", dislayer[2]);
	}

	if (input_code_pressed_once (screen->machine, KEYCODE_M))
	{
		dislayer[3] ^= 1;
		popmessage("fg2: %01x", dislayer[3]);
	}

	if (input_code_pressed_once (screen->machine, KEYCODE_C))
	{
		dislayer[4] ^= 1;
		popmessage("sprites: %01x", dislayer[4]);
	}
#endif

	pc080sn_tilemap_update(state->pc080sn_1);
	pc080sn_tilemap_update(state->pc080sn_2);

	/* Tilemap layer priority seems hardwired (the order is odd, too) */
	layer[0] = 1;
	layer[1] = 0;
	layer[2] = 1;
	layer[3] = 0;

	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);
	bitmap_fill(bitmap, cliprect, 0);

#ifdef MAME_DEBUG
	if (dislayer[3] == 0)
#endif
	pc080sn_tilemap_draw(state->pc080sn_2, bitmap, cliprect, layer[0], TILEMAP_DRAW_OPAQUE, 1);

#ifdef MAME_DEBUG
	if (dislayer[2] == 0)
#endif
	pc080sn_tilemap_draw_special(state->pc080sn_2, bitmap, cliprect, layer[1], 0, 2, state->raster_ctrl);

#ifdef MAME_DEBUG
	if (dislayer[1] == 0)
#endif
	pc080sn_tilemap_draw_special(state->pc080sn_1, bitmap, cliprect, layer[2], 0, 4, state->raster_ctrl + 0x100);

#ifdef MAME_DEBUG
	if (dislayer[0] == 0)
#endif
	pc080sn_tilemap_draw(state->pc080sn_1, bitmap, cliprect, layer[3], 0, 8);

#ifdef MAME_DEBUG
	if (dislayer[4] == 0)
#endif

	draw_sprites(screen->machine, bitmap,cliprect);
	return 0;
}
