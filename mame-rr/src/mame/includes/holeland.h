/*************************************************************************

    Hole Land

*************************************************************************/

class holeland_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, holeland_state(machine)); }

	holeland_state(running_machine &machine) { }

	/* memory pointers */
	UINT8 *    videoram;
	UINT8 *    colorram;
	UINT8 *    spriteram;
	size_t     videoram_size;
	size_t     spriteram_size;

	/* video-related */
	tilemap_t    *bg_tilemap;
	int        palette_offset;
	int        po[2];
};


/*----------- defined in video/holeland.c -----------*/

VIDEO_START( holeland );
VIDEO_START( crzrally );
VIDEO_UPDATE( holeland );
VIDEO_UPDATE( crzrally );

WRITE8_HANDLER( holeland_videoram_w );
WRITE8_HANDLER( holeland_colorram_w );
WRITE8_HANDLER( holeland_flipscreen_w );
WRITE8_HANDLER( holeland_pal_offs_w );
WRITE8_HANDLER( holeland_scroll_w );
