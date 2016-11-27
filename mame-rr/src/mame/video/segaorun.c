/***************************************************************************

    Sega Outrun hardware

***************************************************************************/

#include "emu.h"
#include "video/segaic16.h"
#include "includes/segas16.h"



/*************************************
 *
 *  Video startup
 *
 *************************************/

VIDEO_START( shangon )
{
	/* compute palette info */
	segaic16_palette_init(0x1000);

	/* initialize the tile/text layers */
	segaic16_tilemap_init(machine, 0, SEGAIC16_TILEMAP_16B_ALT, 0x000, 0, 2);

	/* initialize the road */
	segaic16_road_init(machine, 0, SEGAIC16_ROAD_OUTRUN, 0x7f6, 0x7c0, 0x7c0, 0);
}


VIDEO_START( outrun )
{
	/* compute palette info */
	segaic16_palette_init(0x1000);

	/* initialize the tile/text layers */
	segaic16_tilemap_init(machine, 0, SEGAIC16_TILEMAP_16B, 0x000, 0, 2);

	/* initialize the road */
	segaic16_road_init(machine, 0, SEGAIC16_ROAD_OUTRUN, 0x400, 0x420, 0x780, 0);
}



/*************************************
 *
 *  Video update
 *
 *************************************/

VIDEO_UPDATE( shangon )
{
	/* reset priorities */
	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

	/* draw the low priority road layer */
	segaic16_road_draw(0, bitmap, cliprect, SEGAIC16_ROAD_BACKGROUND);

	/* draw background */
	segaic16_tilemap_draw(screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_BACKGROUND, 0, 0x01);
	segaic16_tilemap_draw(screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_BACKGROUND, 1, 0x02);

	/* draw foreground */
	segaic16_tilemap_draw(screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_FOREGROUND, 0, 0x02);
	segaic16_tilemap_draw(screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_FOREGROUND, 1, 0x04);

	/* draw the high priority road */
	segaic16_road_draw(0, bitmap, cliprect, SEGAIC16_ROAD_FOREGROUND);

	/* text layer */
	/* note that we inflate the priority of the text layer to prevent sprites */
	/* from drawing over the high scores */
	segaic16_tilemap_draw(screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_TEXT, 0, 0x08);
	segaic16_tilemap_draw(screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_TEXT, 1, 0x08);

	/* draw the sprites */
	segaic16_sprites_draw(screen, bitmap, cliprect, 0);
	return 0;
}


VIDEO_UPDATE( outrun )
{
	/* if no drawing is happening, fill with black and get out */
	if (!segaic16_display_enable)
	{
		bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));
		return 0;
	}

	/* reset priorities */
	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

	/* draw the low priority road layer */
	segaic16_road_draw(0, bitmap, cliprect, SEGAIC16_ROAD_BACKGROUND);

	/* draw background */
	segaic16_tilemap_draw(screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_BACKGROUND, 0, 0x01);
	segaic16_tilemap_draw(screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_BACKGROUND, 1, 0x02);

	/* draw foreground */
	segaic16_tilemap_draw(screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_FOREGROUND, 0, 0x02);
	segaic16_tilemap_draw(screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_FOREGROUND, 1, 0x04);

	/* draw the high priority road */
	segaic16_road_draw(0, bitmap, cliprect, SEGAIC16_ROAD_FOREGROUND);

	/* text layer */
	segaic16_tilemap_draw(screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_TEXT, 0, 0x04);
	segaic16_tilemap_draw(screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_TEXT, 1, 0x08);

	/* draw the sprites */
	segaic16_sprites_draw(screen, bitmap, cliprect, 0);
	return 0;
}
