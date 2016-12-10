/***************************************************************************

    B-Wings

***************************************************************************/

#define BW_DEBUG 0
#define MAX_SOUNDS 16

class bwing_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, bwing_state(machine)); }

	bwing_state(running_machine &machine) { }

	/* memory pointers */
	UINT8 *    videoram;
	UINT8 *    spriteram;
	UINT8 *    paletteram;
	UINT8 *    bwp1_sharedram1;
	UINT8 *    bwp2_sharedram1;
	UINT8 *    bwp3_rombase;
	size_t     bwp3_romsize;

	/* video-related */
	tilemap_t *charmap, *fgmap, *bgmap;
	UINT8 *srbase[4], *fgdata, *bgdata;
	int *srxlat;
	unsigned sreg[8], palatch, srbank, mapmask, mapflip;

	/* sound-related */
	UINT8 sound_fifo[MAX_SOUNDS];
	int bwp3_nmimask, bwp3_u8F_d, ffcount, ffhead, fftail;

	/* misc */
	UINT8 *bwp123_membase[3];
	int coin;

	/* device */
	running_device *maincpu;
	running_device *subcpu;
	running_device *audiocpu;
};


/*----------- defined in video/bwing.c -----------*/

extern const gfx_layout bwing_tilelayout;

WRITE8_HANDLER( bwing_paletteram_w );
WRITE8_HANDLER( bwing_videoram_w );
WRITE8_HANDLER( bwing_spriteram_w );
WRITE8_HANDLER( bwing_scrollreg_w );
WRITE8_HANDLER( bwing_scrollram_w );
READ8_HANDLER( bwing_scrollram_r );

VIDEO_START( bwing );
VIDEO_UPDATE( bwing );
