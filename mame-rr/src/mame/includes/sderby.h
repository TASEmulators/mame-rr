class sderby_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, sderby_state(machine)); }

	sderby_state(running_machine &machine) { }

	UINT16 *spriteram;
	size_t spriteram_size;

	UINT16 *videoram;
	UINT16 *md_videoram;
	UINT16 *fg_videoram;

	tilemap_t *tilemap;
	tilemap_t *md_tilemap;
	tilemap_t *fg_tilemap;

	UINT16 scroll[6];
};


/*----------- defined in video/sderby.c -----------*/

WRITE16_HANDLER( sderby_videoram_w );
WRITE16_HANDLER( sderby_md_videoram_w );
WRITE16_HANDLER( sderby_fg_videoram_w );
VIDEO_START( sderby );
VIDEO_UPDATE( sderby );
VIDEO_UPDATE( pmroulet );
WRITE16_HANDLER( sderby_scroll_w );
