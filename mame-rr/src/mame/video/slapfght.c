/***************************************************************************

  video.c

  Functions to emulate the video hardware of early Toaplan hardware.

***************************************************************************/

#include "emu.h"
#include "includes/slapfght.h"

UINT8 *slapfight_videoram;
UINT8 *slapfight_colorram;
UINT8 *slapfight_fixvideoram;
UINT8 *slapfight_fixcolorram;
UINT8 *slapfight_scrollx_lo,*slapfight_scrollx_hi,*slapfight_scrolly;
static int flipscreen, slapfight_palette_bank = 0;

static tilemap_t *pf1_tilemap,*fix_tilemap;




/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_pf_tile_info )	/* For Performan only */
{
	int tile,color;

	tile=slapfight_videoram[tile_index] + ((slapfight_colorram[tile_index] & 0x03) << 8);
	color=(slapfight_colorram[tile_index] >> 3) & 0x0f;
	SET_TILE_INFO(
			0,
			tile,
			color,
			0);
}

static TILE_GET_INFO( get_pf1_tile_info )
{
	int tile,color;

	tile=slapfight_videoram[tile_index] + ((slapfight_colorram[tile_index] & 0x0f) << 8);
	color=(slapfight_colorram[tile_index] & 0xf0) >> 4;

	SET_TILE_INFO(
			1,
			tile,
			color,
			0);
}

static TILE_GET_INFO( get_fix_tile_info )
{
	int tile,color;

	tile=slapfight_fixvideoram[tile_index] + ((slapfight_fixcolorram[tile_index] & 0x03) << 8);
	color=(slapfight_fixcolorram[tile_index] & 0xfc) >> 2;

	SET_TILE_INFO(
			0,
			tile,
			color,
			0);
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

VIDEO_START( perfrman )
{
	pf1_tilemap = tilemap_create(machine, get_pf_tile_info,tilemap_scan_rows,8,8,64,32);

	tilemap_set_transparent_pen(pf1_tilemap,0);
}

VIDEO_START( slapfight )
{
	pf1_tilemap = tilemap_create(machine, get_pf1_tile_info,tilemap_scan_rows,8,8,64,32);
	fix_tilemap = tilemap_create(machine, get_fix_tile_info,tilemap_scan_rows,8,8,64,32);

	tilemap_set_transparent_pen(fix_tilemap,0);
}


/***************************************************************************

  Memory handlers

***************************************************************************/

WRITE8_HANDLER( slapfight_videoram_w )
{
	slapfight_videoram[offset]=data;
	tilemap_mark_tile_dirty(pf1_tilemap,offset);
}

WRITE8_HANDLER( slapfight_colorram_w )
{
	slapfight_colorram[offset]=data;
	tilemap_mark_tile_dirty(pf1_tilemap,offset);
}

WRITE8_HANDLER( slapfight_fixram_w )
{
	slapfight_fixvideoram[offset]=data;
	tilemap_mark_tile_dirty(fix_tilemap,offset);
}

WRITE8_HANDLER( slapfight_fixcol_w )
{
	slapfight_fixcolorram[offset]=data;
	tilemap_mark_tile_dirty(fix_tilemap,offset);
}

WRITE8_HANDLER( slapfight_flipscreen_w )
{
	logerror("Writing %02x to flipscreen\n",offset);
	if (offset==0) flipscreen=1; /* Port 0x2 is flipscreen */
	else flipscreen=0; /* Port 0x3 is normal */
}

WRITE8_HANDLER( slapfight_palette_bank_w )
{
	slapfight_palette_bank = offset;
}

static void slapfght_log_vram(running_machine *machine)
{
#ifdef MAME_DEBUG
	if ( input_code_pressed_once(machine, KEYCODE_B) )
	{
		int i;
		for (i=0; i<0x800; i++)
		{
			logerror("Offset:%03x   TileRAM:%02x   AttribRAM:%02x   SpriteRAM:%02x\n",i, slapfight_videoram[i],slapfight_colorram[i],machine->generic.spriteram.u8[i]);
		}
	}
#endif
}

/***************************************************************************

  Render the Sprites

***************************************************************************/
static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int priority_to_display )
{
	UINT8 *buffered_spriteram = machine->generic.buffered_spriteram.u8;
	int offs;

	for (offs = 0;offs < machine->generic.spriteram_size;offs += 4)
	{
		int sx, sy;

		if ((buffered_spriteram[offs+2] & 0x80) == priority_to_display)
		{
			if (flipscreen)
			{
				sx = 265 - buffered_spriteram[offs+1];
				sy = 239 - buffered_spriteram[offs+3];
				sy &= 0xff;
			}
			else
			{
				sx = buffered_spriteram[offs+1] + 3;
				sy = buffered_spriteram[offs+3] - 1;
			}
			drawgfx_transpen(bitmap,cliprect,machine->gfx[1],
				buffered_spriteram[offs],
				((buffered_spriteram[offs+2] >> 1) & 3)	|
					((buffered_spriteram[offs+2] << 2) & 4) | (slapfight_palette_bank << 3),
				flipscreen, flipscreen,
				sx, sy,0);
		}
	}
}


VIDEO_UPDATE( perfrman )
{
	tilemap_set_flip( pf1_tilemap, flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
	tilemap_set_scrolly( pf1_tilemap ,0 , 0 );
	if (flipscreen) {
		tilemap_set_scrollx( pf1_tilemap ,0 , 264 );
	}
	else {
		tilemap_set_scrollx( pf1_tilemap ,0 , -16 );
	}

	tilemap_draw(bitmap,cliprect,pf1_tilemap,TILEMAP_DRAW_OPAQUE,0);
	draw_sprites(screen->machine, bitmap,cliprect,0);
	tilemap_draw(bitmap,cliprect,pf1_tilemap,0,0);
	draw_sprites(screen->machine, bitmap,cliprect,0x80);

	slapfght_log_vram(screen->machine);
	return 0;
}


VIDEO_UPDATE( slapfight )
{
	UINT8 *buffered_spriteram = screen->machine->generic.buffered_spriteram.u8;
	int offs;

	tilemap_set_flip_all(screen->machine,flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
	if (flipscreen) {
		tilemap_set_scrollx( fix_tilemap,0,296);
		tilemap_set_scrollx( pf1_tilemap,0,(*slapfight_scrollx_lo + 256 * *slapfight_scrollx_hi)+296 );
		tilemap_set_scrolly( pf1_tilemap,0, (*slapfight_scrolly)+15 );
		tilemap_set_scrolly( fix_tilemap,0, -1 ); /* Glitch in Tiger Heli otherwise */
	}
	else {
		tilemap_set_scrollx( fix_tilemap,0,0);
		tilemap_set_scrollx( pf1_tilemap,0,(*slapfight_scrollx_lo + 256 * *slapfight_scrollx_hi) );
		tilemap_set_scrolly( pf1_tilemap,0, (*slapfight_scrolly)-1 );
		tilemap_set_scrolly( fix_tilemap,0, -1 ); /* Glitch in Tiger Heli otherwise */
	}

	tilemap_draw(bitmap,cliprect,pf1_tilemap,0,0);

	/* Draw the sprites */
	for (offs = 0;offs < screen->machine->generic.spriteram_size;offs += 4)
	{
		if (flipscreen)
			drawgfx_transpen(bitmap,cliprect,screen->machine->gfx[2],
				buffered_spriteram[offs] + ((buffered_spriteram[offs+2] & 0xc0) << 2),
				(buffered_spriteram[offs+2] & 0x1e) >> 1,
				1,1,
				288-(buffered_spriteram[offs+1] + ((buffered_spriteram[offs+2] & 0x01) << 8)) +18,240-buffered_spriteram[offs+3],0);
		else
			drawgfx_transpen(bitmap,cliprect,screen->machine->gfx[2],
				buffered_spriteram[offs] + ((buffered_spriteram[offs+2] & 0xc0) << 2),
				(buffered_spriteram[offs+2] & 0x1e) >> 1,
				0,0,
				(buffered_spriteram[offs+1] + ((buffered_spriteram[offs+2] & 0x01) << 8)) - 13,buffered_spriteram[offs+3],0);
	}

	tilemap_draw(bitmap,cliprect,fix_tilemap,0,0);

	slapfght_log_vram(screen->machine);
	return 0;
}
