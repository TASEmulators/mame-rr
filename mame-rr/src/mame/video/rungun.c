/*************************************************************************

   Run and Gun
   (c) 1993 Konami

   Video hardware emulation.

   Driver by R. Belmont

*************************************************************************/

#include "emu.h"
#include "video/konicdev.h"
#include "includes/rungun.h"

/* TTL text plane stuff */
static TILE_GET_INFO( ttl_get_tile_info )
{
	rungun_state *state = (rungun_state *)machine->driver_data;
	UINT8 *lvram = (UINT8 *)state->ttl_vram;
	int attr, code;

	attr = (lvram[BYTE_XOR_LE(tile_index<<2)] & 0xf0) >> 4;
	code = ((lvram[BYTE_XOR_LE(tile_index<<2)] & 0x0f) << 8) | (lvram[BYTE_XOR_LE((tile_index<<2)+2)]);

	SET_TILE_INFO(state->ttl_gfx_index, code, attr, 0);
}

void rng_sprite_callback( running_machine *machine, int *code, int *color, int *priority_mask )
{
	rungun_state *state = (rungun_state *)machine->driver_data;
	*color = state->sprite_colorbase | (*color & 0x001f);
}

READ16_HANDLER( rng_ttl_ram_r )
{
	rungun_state *state = (rungun_state *)space->machine->driver_data;
	return state->ttl_vram[offset];
}

WRITE16_HANDLER( rng_ttl_ram_w )
{
	rungun_state *state = (rungun_state *)space->machine->driver_data;
	COMBINE_DATA(&state->ttl_vram[offset]);
}

/* 53936 (PSAC2) rotation/zoom plane */
WRITE16_HANDLER(rng_936_videoram_w)
{
	rungun_state *state = (rungun_state *)space->machine->driver_data;
	COMBINE_DATA(&state->_936_videoram[offset]);
	tilemap_mark_tile_dirty(state->_936_tilemap, offset / 2);
}

static TILE_GET_INFO( get_rng_936_tile_info )
{
	rungun_state *state = (rungun_state *)machine->driver_data;
	int tileno, colour, flipx;

	tileno = state->_936_videoram[tile_index * 2 + 1] & 0x3fff;
	flipx = (state->_936_videoram[tile_index * 2 + 1] & 0xc000) >> 14;
	colour = 0x10 + (state->_936_videoram[tile_index * 2] & 0x000f);

	SET_TILE_INFO(0, tileno, colour, TILE_FLIPYX(flipx));
}


VIDEO_START( rng )
{
	static const gfx_layout charlayout =
	{
		8, 8,	// 8x8
		4096,	// # of tiles
		4,		// 4bpp
		{ 0, 1, 2, 3 },	// plane offsets
		{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },	// X offsets
		{ 0*8*4, 1*8*4, 2*8*4, 3*8*4, 4*8*4, 5*8*4, 6*8*4, 7*8*4 },	// Y offsets
		8*8*4
	};

	rungun_state *state = (rungun_state *)machine->driver_data;
	int gfx_index;

	state->_936_tilemap = tilemap_create(machine, get_rng_936_tile_info, tilemap_scan_rows, 16, 16, 128, 128);
	tilemap_set_transparent_pen(state->_936_tilemap, 0);

	/* find first empty slot to decode gfx */
	for (gfx_index = 0; gfx_index < MAX_GFX_ELEMENTS; gfx_index++)
		if (machine->gfx[gfx_index] == 0)
			break;

	assert(gfx_index != MAX_GFX_ELEMENTS);

	// decode the ttl layer's gfx
	machine->gfx[gfx_index] = gfx_element_alloc(machine, &charlayout, memory_region(machine, "gfx3"), machine->total_colors() / 16, 0);
	state->ttl_gfx_index = gfx_index;

	// create the tilemap
	state->ttl_tilemap = tilemap_create(machine, ttl_get_tile_info, tilemap_scan_rows, 8, 8, 64, 32);

	tilemap_set_transparent_pen(state->ttl_tilemap, 0);

	state->sprite_colorbase = 0x20;
}

VIDEO_UPDATE(rng)
{
	rungun_state *state = (rungun_state *)screen->machine->driver_data;

	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));
	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

	k053936_zoom_draw(state->k053936, bitmap, cliprect, state->_936_tilemap, 0, 0, 1);

	k053247_sprites_draw(state->k055673, bitmap, cliprect);

	tilemap_mark_all_tiles_dirty(state->ttl_tilemap);
	tilemap_draw(bitmap, cliprect, state->ttl_tilemap, 0, 0);
	return 0;
}
