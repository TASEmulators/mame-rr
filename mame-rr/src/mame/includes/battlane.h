/***************************************************************************

    Battle Lane Vol. 5

***************************************************************************/

class battlane_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, battlane_state(machine)); }

	battlane_state(running_machine &machine) { }

	/* memory pointers */
	UINT8 *     tileram;
	UINT8 *     spriteram;

	/* video-related */
	tilemap_t     *bg_tilemap;
	bitmap_t    *screen_bitmap;
	int         video_ctrl;
	int         cpu_control;	/* CPU interrupt control register */

	/* devices */
	running_device *maincpu;
	running_device *subcpu;
};


/*----------- defined in video/battlane.c -----------*/

WRITE8_HANDLER( battlane_palette_w );
WRITE8_HANDLER( battlane_scrollx_w );
WRITE8_HANDLER( battlane_scrolly_w );
WRITE8_HANDLER( battlane_tileram_w );
WRITE8_HANDLER( battlane_spriteram_w );
WRITE8_HANDLER( battlane_bitmap_w );
WRITE8_HANDLER( battlane_video_ctrl_w );

VIDEO_START( battlane );
VIDEO_UPDATE( battlane );
