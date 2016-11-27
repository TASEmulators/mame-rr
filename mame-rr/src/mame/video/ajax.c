/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "video/konicdev.h"
#include "includes/ajax.h"


/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

void ajax_tile_callback( running_machine *machine, int layer, int bank, int *code, int *color, int *flags, int *priority )
{
	ajax_state *state = (ajax_state *)machine->driver_data;
	*code |= ((*color & 0x0f) << 8) | (bank << 12);
	*color = state->layer_colorbase[layer] + ((*color & 0xf0) >> 4);
}


/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

void ajax_sprite_callback( running_machine *machine, int *code, int *color, int *priority, int *shadow )
{
	/* priority bits:
       4 over zoom (0 = have priority)
       5 over B    (0 = have priority)
       6 over A    (1 = have priority)
       never over F
    */
	ajax_state *state = (ajax_state *)machine->driver_data;
	*priority = 0xff00;							/* F = 8 */
	if ( *color & 0x10) *priority |= 0xf0f0;	/* Z = 4 */
	if (~*color & 0x40) *priority |= 0xcccc;	/* A = 2 */
	if ( *color & 0x20) *priority |= 0xaaaa;	/* B = 1 */
	*color = state->sprite_colorbase + (*color & 0x0f);
}


/***************************************************************************

  Callbacks for the K051316

***************************************************************************/

void ajax_zoom_callback( running_machine *machine, int *code, int *color, int *flags )
{
	ajax_state *state = (ajax_state *)machine->driver_data;
	*code |= ((*color & 0x07) << 8);
	*color = state->zoom_colorbase + ((*color & 0x08) >> 3);
}


/***************************************************************************

    Start the video hardware emulation.

***************************************************************************/

VIDEO_START( ajax )
{
	ajax_state *state = (ajax_state *)machine->driver_data;

	state->layer_colorbase[0] = 64;
	state->layer_colorbase[1] = 0;
	state->layer_colorbase[2] = 32;
	state->sprite_colorbase = 16;
	state->zoom_colorbase = 6;	/* == 48 since it's 7-bit graphics */
}



/***************************************************************************

    Display Refresh

***************************************************************************/

VIDEO_UPDATE( ajax )
{
	ajax_state *state = (ajax_state *)screen->machine->driver_data;

	k052109_tilemap_update(state->k052109);

	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

	bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine));
	k052109_tilemap_draw(state->k052109, bitmap, cliprect, 2, 0, 1);
	if (state->priority)
	{
		/* basic layer order is B, zoom, A, F */
		k051316_zoom_draw(state->k051316, bitmap, cliprect, 0, 4);
		k052109_tilemap_draw(state->k052109, bitmap, cliprect, 1, 0, 2);
	}
	else
	{
		/* basic layer order is B, A, zoom, F */
		k052109_tilemap_draw(state->k052109, bitmap, cliprect, 1, 0, 2);
		k051316_zoom_draw(state->k051316, bitmap, cliprect, 0, 4);
	}
	k052109_tilemap_draw(state->k052109, bitmap, cliprect, 0, 0, 8);

	k051960_sprites_draw(state->k051960, bitmap, cliprect, -1, -1);
	return 0;
}
