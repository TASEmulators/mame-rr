/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "video/konicdev.h"
#include "includes/mainevt.h"


/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

void mainevt_tile_callback( running_machine *machine, int layer, int bank, int *code, int *color, int *flags, int *priority )
{
	mainevt_state *state = (mainevt_state *)machine->driver_data;

	*flags = (*color & 0x02) ? TILE_FLIPX : 0;

	/* priority relative to HALF priority sprites */
	*priority = (layer == 2) ? (*color & 0x20) >> 5 : 0;
	*code |= ((*color & 0x01) << 8) | ((*color & 0x1c) << 7);
	*color = state->layer_colorbase[layer] + ((*color & 0xc0) >> 6);
}

void dv_tile_callback( running_machine *machine, int layer, int bank, int *code, int *color, int *flags, int *priority )
{
	mainevt_state *state = (mainevt_state *)machine->driver_data;

	/* (color & 0x02) is flip y handled internally by the 052109 */
	*code |= ((*color & 0x01) << 8) | ((*color & 0x3c) << 7);
	*color = state->layer_colorbase[layer] + ((*color & 0xc0) >> 6);
}


/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

void mainevt_sprite_callback(running_machine *machine, int *code,int *color,int *priority_mask,int *shadow)
{
	mainevt_state *state = (mainevt_state *)machine->driver_data;

	/* bit 5 = priority over layer B (has precedence) */
	/* bit 6 = HALF priority over layer B (used for crowd when you get out of the ring) */
	if (*color & 0x20)
		*priority_mask = 0xff00;
	else if (*color & 0x40)
		*priority_mask = 0xff00 | 0xf0f0;
	else
		*priority_mask = 0xff00 | 0xf0f0 | 0xcccc;
	/* bit 7 is shadow, not used */

	*color = state->sprite_colorbase + (*color & 0x03);
}

void dv_sprite_callback(running_machine *machine, int *code,int *color,int *priority,int *shadow)
{
	mainevt_state *state = (mainevt_state *)machine->driver_data;

	/* TODO: the priority/shadow handling (bits 5-7) seems to be quite complex (see PROM) */
	*color = state->sprite_colorbase + (*color & 0x07);
}


/*****************************************************************************/

VIDEO_START( mainevt )
{
	mainevt_state *state = (mainevt_state *)machine->driver_data;
	state->layer_colorbase[0] = 0;
	state->layer_colorbase[1] = 8;
	state->layer_colorbase[2] = 4;
	state->sprite_colorbase = 12;
}

VIDEO_START( dv )
{
	mainevt_state *state = (mainevt_state *)machine->driver_data;
	state->layer_colorbase[0] = 0;
	state->layer_colorbase[1] = 0;
	state->layer_colorbase[2] = 4;
	state->sprite_colorbase = 8;
}

/*****************************************************************************/

VIDEO_UPDATE( mainevt )
{
	mainevt_state *state = (mainevt_state *)screen->machine->driver_data;

	k052109_tilemap_update(state->k052109);

	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);
	k052109_tilemap_draw(state->k052109, bitmap, cliprect, 1, TILEMAP_DRAW_OPAQUE, 1);
	k052109_tilemap_draw(state->k052109, bitmap, cliprect, 2, 1, 2);	/* low priority part of layer */
	k052109_tilemap_draw(state->k052109, bitmap, cliprect, 2, 0, 4);	/* high priority part of layer */
	k052109_tilemap_draw(state->k052109, bitmap, cliprect, 0, 0, 8);

	k051960_sprites_draw(state->k051960, bitmap, cliprect, -1, -1);
	return 0;
}

VIDEO_UPDATE( dv )
{
	mainevt_state *state = (mainevt_state *)screen->machine->driver_data;

	k052109_tilemap_update(state->k052109);

	k052109_tilemap_draw(state->k052109, bitmap, cliprect, 1, TILEMAP_DRAW_OPAQUE, 0);
	k052109_tilemap_draw(state->k052109, bitmap, cliprect, 2, 0, 0);
	k051960_sprites_draw(state->k051960, bitmap, cliprect, 0, 0);
	k052109_tilemap_draw(state->k052109, bitmap, cliprect, 0, 0, 0);
	return 0;
}
