#include "emu.h"
#include "video/konicdev.h"
#include "includes/gbusters.h"


/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

void gbusters_tile_callback( running_machine *machine, int layer, int bank, int *code, int *color, int *flags, int *priority )
{
	gbusters_state *state = (gbusters_state *)machine->driver_data;
	/* (color & 0x02) is flip y handled internally by the 052109 */
	*code |= ((*color & 0x0d) << 8) | ((*color & 0x10) << 5) | (bank << 12);
	*color = state->layer_colorbase[layer] + ((*color & 0xe0) >> 5);
}

/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

void gbusters_sprite_callback( running_machine *machine, int *code, int *color, int *priority, int *shadow )
{
	gbusters_state *state = (gbusters_state *)machine->driver_data;
	*priority = (*color & 0x30) >> 4;
	*color = state->sprite_colorbase + (*color & 0x0f);
}


/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

VIDEO_START( gbusters )
{
	gbusters_state *state = (gbusters_state *)machine->driver_data;
	state->layer_colorbase[0] = 48;
	state->layer_colorbase[1] = 0;
	state->layer_colorbase[2] = 16;
	state->sprite_colorbase = 32;
}


VIDEO_UPDATE( gbusters )
{
	gbusters_state *state = (gbusters_state *)screen->machine->driver_data;

	k052109_tilemap_update(state->k052109);

	/* sprite priority 3 = disable */
	if (state->priority)
	{
//      k051960_sprites_draw(state->k051960, bitmap, cliprect, 1, 1);  /* are these used? */
		k052109_tilemap_draw(state->k052109, bitmap, cliprect, 2, TILEMAP_DRAW_OPAQUE, 0);
		k051960_sprites_draw(state->k051960, bitmap, cliprect, 2, 2);
		k052109_tilemap_draw(state->k052109, bitmap, cliprect, 1, 0, 0);
		k051960_sprites_draw(state->k051960, bitmap, cliprect, 0, 0);
		k052109_tilemap_draw(state->k052109, bitmap, cliprect, 0, 0, 0);
	}
	else
	{
//      k051960_sprites_draw(state->k051960, bitmap, cliprect, 1, 1);  /* are these used? */
		k052109_tilemap_draw(state->k052109, bitmap, cliprect, 1, TILEMAP_DRAW_OPAQUE, 0);
		k051960_sprites_draw(state->k051960, bitmap, cliprect, 2, 2);
		k052109_tilemap_draw(state->k052109, bitmap, cliprect, 2, 0, 0);
		k051960_sprites_draw(state->k051960, bitmap, cliprect, 0, 0);
		k052109_tilemap_draw(state->k052109, bitmap, cliprect, 0, 0, 0);
	}
	return 0;
}
