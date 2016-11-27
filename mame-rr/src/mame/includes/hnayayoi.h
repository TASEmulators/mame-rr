/*************************************************************************

    Hana Yayoi & other Dynax games (using 1st version of their blitter)

*************************************************************************/

class hnayayoi_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, hnayayoi_state(machine)); }

	hnayayoi_state(running_machine &machine) { }

	/* video-related */
	UINT8      *pixmap[8];
	int        palbank;
	int        total_pixmaps;
	UINT8      blit_layer;
	UINT16     blit_dest;
	UINT32     blit_src;

	/* misc */
	int        keyb;
};


/*----------- defined in video/hnayayoi.c -----------*/

VIDEO_START( hnayayoi );
VIDEO_START( untoucha );
VIDEO_UPDATE( hnayayoi );

WRITE8_HANDLER( dynax_blitter_rev1_param_w );
WRITE8_HANDLER( dynax_blitter_rev1_start_w );
WRITE8_HANDLER( dynax_blitter_rev1_clear_w );
WRITE8_HANDLER( hnayayoi_palbank_w );
