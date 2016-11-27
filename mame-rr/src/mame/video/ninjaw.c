#include "emu.h"
#include "video/taitoic.h"
#include "includes/ninjaw.h"

/**********************************************************/

VIDEO_START( ninjaw )
{
	ninjaw_state *state = (ninjaw_state *)machine->driver_data;

	/* Ensure palette from correct TC0110PCR used for each screen */
	tc0100scn_set_colbanks(state->tc0100scn_1, 0x0, 0x100, 0x200);
}

/************************************************************
            SPRITE DRAW ROUTINE
************************************************************/

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int primask, int x_offs, int y_offs )
{
	ninjaw_state *state = (ninjaw_state *)machine->driver_data;
	UINT16 *spriteram = state->spriteram;
	int offs, data, tilenum, color, flipx, flipy;
	int x, y, priority, curx, cury;
	int code;

#ifdef MAME_DEBUG
	int unknown = 0;
#endif

	for (offs = (state->spriteram_size / 2) - 4; offs >= 0; offs -= 4)
	{
		data = spriteram[offs + 2];
		tilenum = data & 0x7fff;

		if (!tilenum)
			continue;

		data = spriteram[offs + 0];
		x = (data - 32) & 0x3ff;	/* aligns sprites on rock outcrops and sewer hole */

		data = spriteram[offs + 1];
		y = (data - 0) & 0x1ff;

		/*
            The purpose of the bit at data&0x8 (below) is unknown, but it is set
            on Darius explosions, some enemy missiles and at least 1 boss.
            It is most likely another priority bit but as there are no obvious
            visual problems it will need checked against the original pcb.

            There is a report this bit is set when the player intersects
            the tank sprite in Ninja Warriors however I was unable to repro
            this or find any use of this bit in that game.

            Bit&0x8000 is set on some sprites in later levels of Darius
            but is again unknown, and there is no obvious visual problem.
        */
		data = spriteram[offs + 3];
		flipx    = (data & 0x1);
		flipy    = (data & 0x2) >> 1;
		priority = (data & 0x4) >> 2; // 1 = low
		/* data&0x8 - unknown */
		if (priority != primask)
			continue;
		color    = (data & 0x7f00) >> 8;
		/* data&0x8000 - unknown */

#ifdef MAME_DEBUG
		if (data & 0x80f0)   unknown |= (data &0x80f0);
#endif

		x -= x_offs;
		y += y_offs;

		/* sprite wrap: coords become negative at high values */
		if (x > 0x3c0) x -= 0x400;
		if (y > 0x180) y -= 0x200;

		curx = x;
		cury = y;
		code = tilenum;

		drawgfx_transpen(bitmap,cliprect,machine->gfx[0],
				code, color,
				flipx, flipy,
				curx, cury, 0);
	}

#ifdef MAME_DEBUG
	if (unknown)
		popmessage("unknown sprite bits: %04x",unknown);
#endif
}


/**************************************************************
                SCREEN REFRESH
**************************************************************/

VIDEO_UPDATE( ninjaw )
{
	ninjaw_state *state = (ninjaw_state *)screen->machine->driver_data;
	int xoffs = 0;
	UINT8 layer[3], nodraw;
	running_device *tc0100scn = NULL;

	if (screen == state->lscreen)
	{
		xoffs = 36 * 8 * 0;
		tc0100scn = state->tc0100scn_1;
	}
	else if (screen == state->mscreen)
	{
		xoffs = 36 * 8 * 1;
		tc0100scn = state->tc0100scn_2;
	}
	else if (screen == state->rscreen)
	{
		xoffs = 36 * 8 * 2;
		tc0100scn = state->tc0100scn_3;
	}

	tc0100scn_tilemap_update(tc0100scn);

	layer[0] = tc0100scn_bottomlayer(tc0100scn);
	layer[1] = layer[0] ^ 1;
	layer[2] = 2;

	/* chip 0 does tilemaps on the left, chip 1 center, chip 2 the right */
	// draw bottom layer
	nodraw  = tc0100scn_tilemap_draw(tc0100scn, bitmap, cliprect, layer[0], TILEMAP_DRAW_OPAQUE, 0);	/* left */

	/* Ensure screen blanked even when bottom layers not drawn due to disable bit */
	if (nodraw)
		bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));

	/* Sprites can be under/over the layer below text layer */
	draw_sprites(screen->machine, bitmap, cliprect, 1, xoffs, 8); // draw sprites with priority 1 which are under the mid layer

	// draw middle layer
	tc0100scn_tilemap_draw(tc0100scn, bitmap, cliprect, layer[1], 0, 0);

	draw_sprites(screen->machine,bitmap,cliprect,0,xoffs,8); // draw sprites with priority 0 which are over the mid layer

	// draw top(text) layer
	tc0100scn_tilemap_draw(tc0100scn, bitmap, cliprect, layer[2], 0, 0);
	return 0;
}
