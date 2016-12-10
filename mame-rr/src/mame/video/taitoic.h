/*************************************************************************

    taitoic.h

    Implementation of various Taito custom video & input ICs

**************************************************************************/

#include "devlegcy.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _pc080sn_interface pc080sn_interface;
struct _pc080sn_interface
{
	int                gfxnum;

	int                x_offset, y_offset;
	int                y_invert;
	int                dblwidth;
};


typedef struct _pc090oj_interface pc090oj_interface;
struct _pc090oj_interface
{
	int                gfxnum;

	int                x_offset, y_offset;
	int                use_buffer;
};


typedef struct _tc0080vco_interface tc0080vco_interface;
struct _tc0080vco_interface
{
	int                gfxnum;
	int                txnum;

	int                bg_xoffs, bg_yoffs;
	int                bg_flip_yoffs;

	int                has_fg0;	/* for debug */
};

typedef struct _tc0100scn_interface tc0100scn_interface;
struct _tc0100scn_interface
{
	const char         *screen;

	int                gfxnum;
	int                txnum;

	int                x_offset, y_offset;
	int                flip_xoffs, flip_yoffs;
	int                flip_text_xoffs, flip_text_yoffs;

	int                multiscrn_xoffs;
	int                multiscrn_hack;
};


typedef struct _tc0280grd_interface tc0280grd_interface;
struct _tc0280grd_interface
{
	int                gfxnum;
};


typedef struct _tc0480scp_interface tc0480scp_interface;
struct _tc0480scp_interface
{
	int                gfxnum;
	int                txnum;

	int                pixels;

	int                x_offset, y_offset;
	int                text_xoffs, text_yoffs;
	int                flip_xoffs, flip_yoffs;

	int                col_base;
};


typedef struct _tc0150rod_interface tc0150rod_interface;
struct _tc0150rod_interface
{
	const char      *gfx_region;	/* gfx region for the road */
};


typedef struct _tc0110pcr_interface tc0110pcr_interface;
struct _tc0110pcr_interface
{
	int               pal_offs;
};

typedef struct _tc0180vcu_interface tc0180vcu_interface;
struct _tc0180vcu_interface
{
	int            bg_color_base;
	int            fg_color_base;
	int            tx_color_base;
};

DECLARE_LEGACY_DEVICE(PC080SN, pc080sn);
DECLARE_LEGACY_DEVICE(PC090OJ, pc090oj);
DECLARE_LEGACY_DEVICE(TC0080VCO, tc0080vco);
DECLARE_LEGACY_DEVICE(TC0100SCN, tc0100scn);
DECLARE_LEGACY_DEVICE(TC0280GRD, tc0280grd);
#define TC0430GRW TC0280GRD
DECLARE_LEGACY_DEVICE(TC0360PRI, tc0360pri);
DECLARE_LEGACY_DEVICE(TC0480SCP, tc0480scp);
DECLARE_LEGACY_DEVICE(TC0150ROD, tc0150rod);
DECLARE_LEGACY_DEVICE(TC0110PCR, tc0110pcr);
DECLARE_LEGACY_DEVICE(TC0180VCU, tc0180vcu);

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MDRV_PC080SN_ADD(_tag, _interface) \
	MDRV_DEVICE_ADD(_tag, PC080SN, 0) \
	MDRV_DEVICE_CONFIG(_interface)

#define MDRV_PC090OJ_ADD(_tag, _interface) \
	MDRV_DEVICE_ADD(_tag, PC090OJ, 0) \
	MDRV_DEVICE_CONFIG(_interface)

#define MDRV_TC0080VCO_ADD(_tag, _interface) \
	MDRV_DEVICE_ADD(_tag, TC0080VCO, 0) \
	MDRV_DEVICE_CONFIG(_interface)

#define MDRV_TC0100SCN_ADD(_tag, _interface) \
	MDRV_DEVICE_ADD(_tag, TC0100SCN, 0) \
	MDRV_DEVICE_CONFIG(_interface)

#define MDRV_TC0280GRD_ADD(_tag, _interface) \
	MDRV_DEVICE_ADD(_tag, TC0280GRD, 0) \
	MDRV_DEVICE_CONFIG(_interface)

#define MDRV_TC0430GRW_ADD(_tag, _interface) \
	MDRV_DEVICE_ADD(_tag, TC0430GRW, 0) \
	MDRV_DEVICE_CONFIG(_interface)

#define MDRV_TC0360PRI_ADD(_tag) \
	MDRV_DEVICE_ADD(_tag, TC0360PRI, 0)

#define MDRV_TC0150ROD_ADD(_tag, _interface) \
	MDRV_DEVICE_ADD(_tag, TC0150ROD, 0) \
	MDRV_DEVICE_CONFIG(_interface)

#define MDRV_TC0480SCP_ADD(_tag, _interface) \
	MDRV_DEVICE_ADD(_tag, TC0480SCP, 0) \
	MDRV_DEVICE_CONFIG(_interface)

#define MDRV_TC0110PCR_ADD(_tag, _interface) \
	MDRV_DEVICE_ADD(_tag, TC0110PCR, 0) \
	MDRV_DEVICE_CONFIG(_interface)

#define MDRV_TC0180VCU_ADD(_tag, _interface) \
	MDRV_DEVICE_ADD(_tag, TC0180VCU, 0) \
	MDRV_DEVICE_CONFIG(_interface)


/***************************************************************************
    DEVICE I/O FUNCTIONS
***************************************************************************/

/**  PC080SN  **/
READ16_DEVICE_HANDLER( pc080sn_word_r );
WRITE16_DEVICE_HANDLER( pc080sn_word_w );
WRITE16_DEVICE_HANDLER( pc080sn_xscroll_word_w );
WRITE16_DEVICE_HANDLER( pc080sn_yscroll_word_w );
WRITE16_DEVICE_HANDLER( pc080sn_ctrl_word_w );

void pc080sn_set_scroll(running_device *device, int tilemap_num, int scrollx, int scrolly);
void pc080sn_set_trans_pen(running_device *device, int tilemap_num, int pen);
void pc080sn_tilemap_update(running_device *device);
void pc080sn_tilemap_draw(running_device *device, bitmap_t *bitmap, const rectangle *cliprect, int layer, int flags, UINT32 priority);
void pc080sn_tilemap_draw_offset(running_device *device, bitmap_t *bitmap, const rectangle *cliprect, int layer, int flags, UINT32 priority, int xoffs, int yoffs);

/* For Topspeed */
void pc080sn_tilemap_draw_special(running_device *device, bitmap_t *bitmap, const rectangle *cliprect, int layer, int flags, UINT32 priority, UINT16 *ram);


/**  PC090OJ  **/
READ16_DEVICE_HANDLER( pc090oj_word_r );
WRITE16_DEVICE_HANDLER( pc090oj_word_w );

void pc090oj_set_sprite_ctrl(running_device *device, UINT16 sprctrl);
void pc090oj_eof_callback(running_device *device);
void pc090oj_draw_sprites(running_device *device, bitmap_t *bitmap, const rectangle *cliprect, int pri_type);


/** TC0080VCO **/
READ16_DEVICE_HANDLER( tc0080vco_word_r );
WRITE16_DEVICE_HANDLER( tc0080vco_word_w );

void tc0080vco_tilemap_update(running_device *device);
void tc0080vco_tilemap_draw(running_device *device, bitmap_t *bitmap, const rectangle *cliprect, int layer, int flags, UINT32 priority);

READ16_DEVICE_HANDLER( tc0080vco_cram_0_r );
READ16_DEVICE_HANDLER( tc0080vco_cram_1_r );
READ16_DEVICE_HANDLER( tc0080vco_sprram_r );
READ16_DEVICE_HANDLER( tc0080vco_scrram_r );
READ_LINE_DEVICE_HANDLER( tc0080vco_flipscreen_r );


/** TC0100SCN **/
#define TC0100SCN_SINGLE_VDU    1024

/* Function to set separate color banks for the three tilemapped layers.
   To change from the default (0,0,0) use after calling TC0100SCN_vh_start */
void tc0100scn_set_colbanks(running_device *device, int bg0, int bg1, int tx);

/* Function to set separate color banks for each TC0100SCN.
   To change from the default (0,0,0) use after calling TC0100SCN_vh_start */
void tc0100scn_set_colbank(running_device *device, int colbank);

/* Function to set bg tilemask < 0xffff */
void tc0100scn_set_bg_tilemask(running_device *device, int mask);

/* Function to for Mjnquest to select gfx bank */
WRITE16_DEVICE_HANDLER( tc0100scn_gfxbank_w );

READ16_DEVICE_HANDLER( tc0100scn_word_r );
WRITE16_DEVICE_HANDLER( tc0100scn_word_w );
READ16_DEVICE_HANDLER( tc0100scn_ctrl_word_r );
WRITE16_DEVICE_HANDLER( tc0100scn_ctrl_word_w );

/* Functions for use with 68020 (Under Fire) */
READ32_DEVICE_HANDLER( tc0100scn_long_r );
WRITE32_DEVICE_HANDLER( tc0100scn_long_w );
READ32_DEVICE_HANDLER( tc0100scn_ctrl_long_r );
WRITE32_DEVICE_HANDLER( tc0100scn_ctrl_long_w );

void tc0100scn_tilemap_update(running_device *device);
int tc0100scn_tilemap_draw(running_device *device, bitmap_t *bitmap, const rectangle *cliprect, int layer, int flags, UINT32 priority);

/* returns 0 or 1 depending on the lowest priority tilemap set in the internal
   register. Use this function to draw tilemaps in the correct order. */
int tc0100scn_bottomlayer(running_device *device);


/** TC0280GRD & TC0430GRW **/
READ16_DEVICE_HANDLER( tc0280grd_word_r );
WRITE16_DEVICE_HANDLER( tc0280grd_word_w );
WRITE16_DEVICE_HANDLER( tc0280grd_ctrl_word_w );
void tc0280grd_tilemap_update(running_device *device, int base_color);
void tc0280grd_zoom_draw(running_device *device, bitmap_t *bitmap, const rectangle *cliprect, int xoffset, int yoffset, UINT32 priority);

READ16_DEVICE_HANDLER( tc0430grw_word_r );
WRITE16_DEVICE_HANDLER( tc0430grw_word_w );
WRITE16_DEVICE_HANDLER( tc0430grw_ctrl_word_w );
void tc0430grw_tilemap_update(running_device *device, int base_color);
void tc0430grw_zoom_draw(running_device *device, bitmap_t *bitmap, const rectangle *cliprect, int xoffset, int yoffset, UINT32 priority);


/** TC0360PRI **/
WRITE8_DEVICE_HANDLER( tc0360pri_w );
READ8_DEVICE_HANDLER( tc0360pri_r );


/** TC0480SCP **/
/* When writing a driver, pass zero for the text and flip offsets initially:
   then tweak them once you have the 4 bg layer positions correct. Col_base
   may be needed when tilemaps use a palette area from sprites. */

READ16_DEVICE_HANDLER( tc0480scp_word_r );
WRITE16_DEVICE_HANDLER( tc0480scp_word_w );
READ16_DEVICE_HANDLER( tc0480scp_ctrl_word_r );
WRITE16_DEVICE_HANDLER( tc0480scp_ctrl_word_w );

/* Functions for use with 68020 (Super-Z system) */
READ32_DEVICE_HANDLER( tc0480scp_long_r );
WRITE32_DEVICE_HANDLER( tc0480scp_long_w );
READ32_DEVICE_HANDLER( tc0480scp_ctrl_long_r );
WRITE32_DEVICE_HANDLER( tc0480scp_ctrl_long_w );

void tc0480scp_tilemap_update(running_device *device);
void tc0480scp_tilemap_draw(running_device *device, bitmap_t *bitmap, const rectangle *cliprect, int layer, int flags, UINT32 priority);

/* Returns the priority order of the bg tilemaps set in the internal
   register. The order in which the four layers should be drawn is
   returned in the lowest four nibbles  (msn = bottom layer; lsn = top) */
int tc0480scp_get_bg_priority(running_device *device);

/* Undrfire needs to read this for a sprite/tile priority hack */
READ8_DEVICE_HANDLER( tc0480scp_pri_reg_r );


/** TC0150ROD **/
READ16_DEVICE_HANDLER( tc0150rod_word_r );
WRITE16_DEVICE_HANDLER( tc0150rod_word_w );
void tc0150rod_draw(running_device *device, bitmap_t *bitmap, const rectangle *cliprect, int y_offs, int palette_offs, int type, int road_trans, UINT32 low_priority, UINT32 high_priority);


/** TC0110PCR **/
READ16_DEVICE_HANDLER( tc0110pcr_word_r );
WRITE16_DEVICE_HANDLER( tc0110pcr_word_w );	/* color index goes up in step of 2 */
WRITE16_DEVICE_HANDLER( tc0110pcr_step1_word_w );	/* color index goes up in step of 1 */
WRITE16_DEVICE_HANDLER( tc0110pcr_step1_rbswap_word_w );	/* swaps red and blue components */
WRITE16_DEVICE_HANDLER( tc0110pcr_step1_4bpg_word_w );	/* only 4 bits per color gun */


/** TC0180VCU **/
READ8_DEVICE_HANDLER( tc0180vcu_get_fb_page );
WRITE8_DEVICE_HANDLER( tc0180vcu_set_fb_page );
READ8_DEVICE_HANDLER( tc0180vcu_get_videoctrl );
READ16_DEVICE_HANDLER( tc0180vcu_ctrl_r );
WRITE16_DEVICE_HANDLER( tc0180vcu_ctrl_w );
READ16_DEVICE_HANDLER( tc0180vcu_scroll_r );
WRITE16_DEVICE_HANDLER( tc0180vcu_scroll_w );
READ16_DEVICE_HANDLER( tc0180vcu_word_r );
WRITE16_DEVICE_HANDLER( tc0180vcu_word_w );
void tc0180vcu_tilemap_draw(running_device *device, bitmap_t *bitmap, const rectangle *cliprect, int tmap_num, int plane);
