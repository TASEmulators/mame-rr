/*************************************************************************

    Asterix

*************************************************************************/

class asterix_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, asterix_state(machine)); }

	asterix_state(running_machine &machine) { }

	/* memory pointers */
//  UINT16 *    paletteram;    // currently this uses generic palette handling

	/* video-related */
	int         sprite_colorbase;
	int         layer_colorbase[4], layerpri[3];
	UINT16      spritebank;
	int         tilebanks[4];
	int         spritebanks[4];

	/* misc */
	UINT8       cur_control2;
	UINT16      prot[2];

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *k053260;
	running_device *k056832;
	running_device *k053244;
	running_device *k053251;
};



/*----------- defined in video/asterix.c -----------*/

VIDEO_UPDATE( asterix );
WRITE16_HANDLER( asterix_spritebank_w );

extern void asterix_tile_callback(running_machine *machine, int layer, int *code, int *color, int *flags);
extern void asterix_sprite_callback(running_machine *machine, int *code, int *color, int *priority_mask);
