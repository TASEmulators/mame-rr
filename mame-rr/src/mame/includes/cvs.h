/***************************************************************************

    Century CVS System

    (and Quasar)

****************************************************************************/


#define CVS_S2636_Y_OFFSET     (3)
#define CVS_S2636_X_OFFSET     (-26)
#define CVS_MAX_STARS          250

struct cvs_star
{
	int x, y, code;
};

class cvs_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, cvs_state(machine)); }

	cvs_state(running_machine &machine) { }

	/* memory pointers */
	UINT8 *    video_ram;
	UINT8 *    bullet_ram;
	UINT8 *    fo_state;
	UINT8 *    cvs_4_bit_dac_data;
	UINT8 *    tms5110_ctl_data;
	UINT8 *    dac3_state;
	UINT8 *    color_ram;
	UINT8 *    palette_ram;
	UINT8 *    character_ram;
	UINT8 *    effectram;	// quasar

	/* video-related */
	struct cvs_star stars[CVS_MAX_STARS];
	bitmap_t   *collision_background;
	bitmap_t   *background_bitmap;
	bitmap_t   *scrolled_collision_background;
	int        collision_register;
	int        total_stars;
	int        stars_on;
	UINT8      scroll_reg;
	UINT8      effectcontrol;	// quasar
	int        stars_scroll;

	/* misc */
	emu_timer  *cvs_393hz_timer;
	UINT8      cvs_393hz_clock;

	UINT8      character_banking_mode;
	UINT16     character_ram_page_start;
	UINT16     speech_rom_bit_address;

	UINT8      page, io_page;	// quasar

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *speech;
	running_device *dac3;
	running_device *tms;
	running_device *s2636_0;
	running_device *s2636_1;
	running_device *s2636_2;
};

/*----------- defined in drivers/cvs.c -----------*/

MACHINE_START( cvs );
MACHINE_RESET( cvs );

READ8_HANDLER( cvs_video_or_color_ram_r );
WRITE8_HANDLER( cvs_video_or_color_ram_w );

READ8_HANDLER( cvs_bullet_ram_or_palette_r );
WRITE8_HANDLER( cvs_bullet_ram_or_palette_w );

READ8_HANDLER( cvs_s2636_0_or_character_ram_r );
WRITE8_HANDLER( cvs_s2636_0_or_character_ram_w );
READ8_HANDLER( cvs_s2636_1_or_character_ram_r );
WRITE8_HANDLER( cvs_s2636_1_or_character_ram_w );
READ8_HANDLER( cvs_s2636_2_or_character_ram_r );
WRITE8_HANDLER( cvs_s2636_2_or_character_ram_w );

/*----------- defined in video/cvs.c -----------*/

WRITE8_HANDLER( cvs_scroll_w );
WRITE8_HANDLER( cvs_video_fx_w );

READ8_HANDLER( cvs_collision_r );
READ8_HANDLER( cvs_collision_clear );

void cvs_scroll_stars(running_machine *machine);

PALETTE_INIT( cvs );
VIDEO_UPDATE( cvs );
VIDEO_START( cvs );

/*----------- defined in video/quasar.c -----------*/

PALETTE_INIT( quasar );
VIDEO_UPDATE( quasar );
VIDEO_START( quasar );
