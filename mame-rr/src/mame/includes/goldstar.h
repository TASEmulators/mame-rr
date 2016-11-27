class goldstar_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, goldstar_state(machine)); }

	goldstar_state(running_machine &machine) { }

	int dataoffset;

	UINT8 *nvram;
	size_t nvram_size;

	UINT8 *atrram;
	UINT8 *fg_atrram;
	UINT8 *fg_vidram;

	UINT8 *reel1_scroll;
	UINT8 *reel2_scroll;
	UINT8 *reel3_scroll;

	UINT8 *reel1_ram;
	UINT8 *reel2_ram;
	UINT8 *reel3_ram;

	/* reelx_attrram for unkch sets */
	UINT8 *reel1_attrram;
	UINT8 *reel2_attrram;
	UINT8 *reel3_attrram;
	UINT8 unkch_vidreg;

	tilemap_t *reel1_tilemap;
	tilemap_t *reel2_tilemap;
	tilemap_t *reel3_tilemap;

	int bgcolor;
	tilemap_t *fg_tilemap;
	UINT8 cmaster_girl_num;
	UINT8 cmaster_girl_pal;
	UINT8 cm_enable_reg;
	UINT8 cm_girl_scroll;
	UINT8 lucky8_nmi_enable;
	int tile_bank;

};


/*----------- defined in video/goldstar.c -----------*/

WRITE8_HANDLER( goldstar_reel1_ram_w );
WRITE8_HANDLER( goldstar_reel2_ram_w );
WRITE8_HANDLER( goldstar_reel3_ram_w );

WRITE8_HANDLER( unkch_reel1_attrram_w );
WRITE8_HANDLER( unkch_reel2_attrram_w );
WRITE8_HANDLER( unkch_reel3_attrram_w );

WRITE8_HANDLER( goldstar_fg_vidram_w );
WRITE8_HANDLER( goldstar_fg_atrram_w );
WRITE8_HANDLER( cm_girl_scroll_w );

WRITE8_HANDLER( goldstar_fa00_w );
WRITE8_HANDLER( cm_background_col_w );
WRITE8_HANDLER( cm_outport0_w );
VIDEO_START( goldstar );
VIDEO_START( cherrym );
VIDEO_START( unkch );
VIDEO_START( magical );
VIDEO_UPDATE( goldstar );
VIDEO_UPDATE( cmast91 );
VIDEO_UPDATE( amcoe1a );
VIDEO_UPDATE( unkch );
VIDEO_UPDATE( magical );
