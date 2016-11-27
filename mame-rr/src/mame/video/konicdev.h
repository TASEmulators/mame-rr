/*************************************************************************

    konicdev.h

    Implementation of various Konami custom video ICs

**************************************************************************/

#pragma once
#ifndef __KONICDEV_H__
#define __KONICDEV_H__

#include "devlegcy.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef void (*k007342_callback)(running_machine *machine, int tmap, int bank, int *code, int *color, int *flags);
typedef void (*k007420_callback)(running_machine *machine, int *code, int *color);
typedef void (*k052109_callback)(running_machine *machine, int layer, int bank, int *code, int *color, int *flags, int *priority);
typedef void (*k051960_callback)(running_machine *machine, int *code, int *color, int *priority, int *shadow);
typedef void (*k05324x_callback)(running_machine *machine, int *code, int *color, int *priority);
typedef void (*k051316_callback)(running_machine *machine, int *code, int *color, int *flags);
typedef void (*k056832_callback)(running_machine *machine, int layer, int *code, int *color, int *flags);


typedef struct _k007342_interface k007342_interface;
struct _k007342_interface
{
	int                gfxnum;
	k007342_callback   callback;
};

typedef struct _k007420_interface k007420_interface;
struct _k007420_interface
{
	int                banklimit;
	k007420_callback   callback;
};

typedef struct _k052109_interface k052109_interface;
struct _k052109_interface
{
	const char         *gfx_memory_region;
	int                gfx_num;
	int                plane_order;
	int                deinterleave;
	k052109_callback   callback;
};

typedef struct _k051960_interface k051960_interface;
struct _k051960_interface
{
	const char         *gfx_memory_region;
	int                gfx_num;
	int                plane_order;
	int                deinterleave;
	k051960_callback   callback;
};

typedef struct _k05324x_interface k05324x_interface;
struct _k05324x_interface
{
	const char         *gfx_memory_region;
	int                gfx_num;
	int                plane_order;
	int                dx, dy;
	int                deinterleave;
	k05324x_callback   callback;
};

typedef struct _k053247_interface k053247_interface;
struct _k053247_interface
{
	const char         *screen;
	const char         *gfx_memory_region;
	int                gfx_num;
	int                plane_order;
	int                dx, dy;
	int                deinterleave;
	k05324x_callback   callback;
};

typedef struct _k051316_interface k051316_interface;
struct _k051316_interface
{
	const char         *gfx_memory_region;
	int                gfx_num;
	int                bpp, pen_is_mask, transparent_pen;
	int                wrap, xoffs, yoffs;
	k051316_callback   callback;
};

typedef struct _k053936_interface k053936_interface;
struct _k053936_interface
{
	int                wrap, xoff, yoff;
};

typedef struct _k056832_interface k056832_interface;
struct _k056832_interface
{
	const char         *gfx_memory_region;
	int                gfx_num;
	int                bpp;
	int                big;
	int                djmain_hack;
	int                deinterleave;
	k056832_callback   callback;

	const char         *k055555;	// tbyahhoo uses the k056832 together with a k055555
};

typedef struct _k054338_interface k054338_interface;
struct _k054338_interface
{
	const char         *screen;
	int                alpha_inv;
	const char         *k055555;
};

typedef struct _k053250_interface k053250_interface;
struct _k053250_interface
{
	const char         *screen;
	const char         *gfx_memory_region;
	int                xoff, yoff;
};

typedef struct _k001006_interface k001006_interface;
struct _k001006_interface
{
	const char     *gfx_region;
};

typedef struct _k001005_interface k001005_interface;
struct _k001005_interface
{
	const char     *screen;
	const char     *cpu;
	const char     *dsp;
	const char     *k001006_1;
	const char     *k001006_2;

	const char     *gfx_memory_region;
	int            gfx_index;
};

typedef struct _k001604_interface k001604_interface;
struct _k001604_interface
{
	int            gfx_index_1;
	int            gfx_index_2;
	int            layer_size;
	int            roz_size;

	int            is_slrasslt;
};

typedef struct _k037122_interface k037122_interface;
struct _k037122_interface
{
	const char     *screen;
	int            gfx_index;
};

DECLARE_LEGACY_DEVICE(K007121, k007121);
DECLARE_LEGACY_DEVICE(K007342, k007342);
DECLARE_LEGACY_DEVICE(K007420, k007420);
DECLARE_LEGACY_DEVICE(K052109, k052109);
DECLARE_LEGACY_DEVICE(K051960, k051960);
DECLARE_LEGACY_DEVICE(K053244, k05324x);
#define K053245 K053244
DECLARE_LEGACY_DEVICE(K053246, k053247);
#define K053247 K053246
DECLARE_LEGACY_DEVICE(K055673, k055673);
DECLARE_LEGACY_DEVICE(K051316, k051316);
DECLARE_LEGACY_DEVICE(K053936, k053936);
DECLARE_LEGACY_DEVICE(K053251, k053251);
DECLARE_LEGACY_DEVICE(K054000, k054000);
DECLARE_LEGACY_DEVICE(K051733, k051733);
DECLARE_LEGACY_DEVICE(K056832, k056832);
DECLARE_LEGACY_DEVICE(K055555, k055555);
DECLARE_LEGACY_DEVICE(K054338, k054338);
DECLARE_LEGACY_DEVICE(K053250, k053250);
DECLARE_LEGACY_DEVICE(K053252, k053252);
DECLARE_LEGACY_DEVICE(K001006, k001006);
DECLARE_LEGACY_DEVICE(K001005, k001005);
DECLARE_LEGACY_DEVICE(K001604, k001604);
DECLARE_LEGACY_DEVICE(K037122, k037122);


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MDRV_K007121_ADD(_tag) \
	MDRV_DEVICE_ADD(_tag, K007121, 0)

#define MDRV_K007342_ADD(_tag, _interface) \
	MDRV_DEVICE_ADD(_tag, K007342, 0) \
	MDRV_DEVICE_CONFIG(_interface)

#define MDRV_K007420_ADD(_tag, _interface) \
	MDRV_DEVICE_ADD(_tag, K007420, 0) \
	MDRV_DEVICE_CONFIG(_interface)

#define MDRV_K052109_ADD(_tag, _interface) \
	MDRV_DEVICE_ADD(_tag, K052109, 0) \
	MDRV_DEVICE_CONFIG(_interface)

#define MDRV_K051960_ADD(_tag, _interface) \
	MDRV_DEVICE_ADD(_tag, K051960, 0) \
	MDRV_DEVICE_CONFIG(_interface)

#define MDRV_K053244_ADD(_tag, _interface) \
	MDRV_DEVICE_ADD(_tag, K053244, 0) \
	MDRV_DEVICE_CONFIG(_interface)

#define MDRV_K053245_ADD(_tag, _interface) \
	MDRV_DEVICE_ADD(_tag, K053245, 0) \
	MDRV_DEVICE_CONFIG(_interface)

#define MDRV_K053246_ADD(_tag, _interface) \
	MDRV_DEVICE_ADD(_tag, K053246, 0) \
	MDRV_DEVICE_CONFIG(_interface)

#define MDRV_K053247_ADD(_tag, _interface) \
	MDRV_DEVICE_ADD(_tag, K053247, 0) \
	MDRV_DEVICE_CONFIG(_interface)

#define MDRV_K055673_ADD(_tag, _interface) \
	MDRV_DEVICE_ADD(_tag, K055673, 0) \
	MDRV_DEVICE_CONFIG(_interface)

#define MDRV_K051316_ADD(_tag, _interface) \
	MDRV_DEVICE_ADD(_tag, K051316, 0) \
	MDRV_DEVICE_CONFIG(_interface)

#define MDRV_K053936_ADD(_tag, _interface) \
	MDRV_DEVICE_ADD(_tag, K053936, 0) \
	MDRV_DEVICE_CONFIG(_interface)

#define MDRV_K053251_ADD(_tag) \
	MDRV_DEVICE_ADD(_tag, K053251, 0)

#define MDRV_K054000_ADD(_tag) \
	MDRV_DEVICE_ADD(_tag, K054000, 0)

#define MDRV_K051733_ADD(_tag) \
	MDRV_DEVICE_ADD(_tag, K051733, 0)

#define MDRV_K056832_ADD(_tag, _interface) \
	MDRV_DEVICE_ADD(_tag, K056832, 0) \
	MDRV_DEVICE_CONFIG(_interface)

#define MDRV_K055555_ADD(_tag) \
	MDRV_DEVICE_ADD(_tag, K055555, 0)

#define MDRV_K054338_ADD(_tag, _interface) \
	MDRV_DEVICE_ADD(_tag, K054338, 0) \
	MDRV_DEVICE_CONFIG(_interface)

#define MDRV_K053250_ADD(_tag, _interface) \
	MDRV_DEVICE_ADD(_tag, K053250, 0) \
	MDRV_DEVICE_CONFIG(_interface)

#define MDRV_K053252_ADD(_tag) \
	MDRV_DEVICE_ADD(_tag, K053252, 0)


#define MDRV_K001006_ADD(_tag, _interface) \
	MDRV_DEVICE_ADD(_tag, K001006, 0) \
	MDRV_DEVICE_CONFIG(_interface)


#define MDRV_K001005_ADD(_tag, _interface) \
	MDRV_DEVICE_ADD(_tag, K001005, 0) \
	MDRV_DEVICE_CONFIG(_interface)


#define MDRV_K001604_ADD(_tag, _interface) \
	MDRV_DEVICE_ADD(_tag, K001604, 0) \
	MDRV_DEVICE_CONFIG(_interface)


#define MDRV_K037122_ADD(_tag, _interface) \
	MDRV_DEVICE_ADD(_tag, K037122, 0) \
	MDRV_DEVICE_CONFIG(_interface)


/***************************************************************************
    HELPERS FOR DRIVERS
***************************************************************************/

enum
{
	KONAMI_ROM_DEINTERLEAVE_NONE = 0,
	KONAMI_ROM_DEINTERLEAVE_2,
	KONAMI_ROM_DEINTERLEAVE_2_HALF,
	KONAMI_ROM_DEINTERLEAVE_4,
	KONAMI_ROM_SHUFFLE8
};

/* helper function to join two 16-bit ROMs and form a 32-bit data stream */
void konamid_rom_deinterleave_2(running_machine *machine, const char *mem_region);
void konamid_rom_deinterleave_2_half(running_machine *machine, const char *mem_region);
/* helper function to join four 16-bit ROMs and form a 64-bit data stream */
void konamid_rom_deinterleave_4(running_machine *machine, const char *mem_region);

/* helper function to sort three tile layers by priority order */
void konami_sortlayers3(int *layer, int *pri);
/* helper function to sort four tile layers by priority order */
void konami_sortlayers4(int *layer, int *pri);
/* helper function to sort five tile layers by priority order */
void konami_sortlayers5(int *layer, int *pri);

/***************************************************************************
    DEVICE I/O FUNCTIONS
***************************************************************************/

/**  Konami 007121  **/
READ8_DEVICE_HANDLER( k007121_ctrlram_r );
WRITE8_DEVICE_HANDLER( k007121_ctrl_w );

/* shall we move source in the interface? */
/* also notice that now we directly pass *gfx[chip] instead of **gfx !! */
void k007121_sprites_draw( running_device *device, bitmap_t *bitmap, const rectangle *cliprect, gfx_element *gfx, colortable_t *ctable,
						  const UINT8 *source, int base_color, int global_x_offset, int bank_base, UINT32 pri_mask );


/**  Konami 007342  **/
READ8_DEVICE_HANDLER( k007342_r );
WRITE8_DEVICE_HANDLER( k007342_w );
READ8_DEVICE_HANDLER( k007342_scroll_r );
WRITE8_DEVICE_HANDLER( k007342_scroll_w );
WRITE8_DEVICE_HANDLER( k007342_vreg_w );

void k007342_tilemap_update(running_device *device);
void k007342_tilemap_draw(running_device *device, bitmap_t *bitmap, const rectangle *cliprect, int num, int flags, UINT32 priority);
int k007342_is_int_enabled(running_device *device);


/**  Konami 007420  **/
#define K007420_SPRITERAM_SIZE 0x200

READ8_DEVICE_HANDLER( k007420_r );
WRITE8_DEVICE_HANDLER( k007420_w );
void k007420_sprites_draw(running_device *device, bitmap_t *bitmap, const rectangle *cliprect, gfx_element *gfx);


/**  Konami 052109  **/
/*
You don't have to decode the graphics: the vh_start() routines will do that
for you, using the plane order passed.
Of course the ROM data must be in the correct order. This is a way to ensure
that the ROM test will pass.
The konami_rom_deinterleave() function above will do the reorganization for
you in most cases (but see tmnt.c for additional bit rotations or byte
permutations which may be required).
*/
#define NORMAL_PLANE_ORDER 0x0123
#define REVERSE_PLANE_ORDER 0x3210
#define GRADIUS3_PLANE_ORDER 0x1111
#define TASMAN_PLANE_ORDER 0x1616

/*
The callback is passed:
- layer number (0 = FIX, 1 = A, 2 = B)
- bank (range 0-3, output of the pins CAB1 and CAB2)
- code (range 00-FF, output of the pins VC3-VC10)
  NOTE: code is in the range 0000-FFFF for X-Men, which uses extra RAM
- color (range 00-FF, output of the pins COL0-COL7)
The callback must put:
- in code the resulting tile number
- in color the resulting color index
- if necessary, put flags and/or priority for the TileMap code in the tile_info
  structure (e.g. TILE_FLIPX). Note that TILE_FLIPY is handled internally by the
  chip so it must not be set by the callback.
*/

READ8_DEVICE_HANDLER( k052109_r );
WRITE8_DEVICE_HANDLER( k052109_w );
READ16_DEVICE_HANDLER( k052109_word_r );
WRITE16_DEVICE_HANDLER( k052109_word_w );
READ16_DEVICE_HANDLER( k052109_lsb_r );
WRITE16_DEVICE_HANDLER( k052109_lsb_w );

void k052109_set_rmrd_line(running_device *device, int state);
int k052109_get_rmrd_line(running_device *device);
void k052109_tilemap_update(running_device *device);
int k052109_is_irq_enabled(running_device *device);
void k052109_set_layer_offsets(running_device *device, int layer, int dx, int dy);
void k052109_tilemap_mark_dirty(running_device *device, int tmap_num);
void k052109_tilemap_draw(running_device *device, bitmap_t *bitmap, const rectangle *cliprect, int tmap_num, UINT32 flags, UINT8 priority);


/**  Konami 051960 / 051937  **/
/*
The callback is passed:
- code (range 00-1FFF, output of the pins CA5-CA17)
- color (range 00-FF, output of the pins OC0-OC7). Note that most of the
  time COL7 seems to be "shadow", but not always (e.g. Aliens).
The callback must put:
- in code the resulting sprite number
- in color the resulting color index
- if necessary, in priority the priority of the sprite wrt tilemaps
- if necessary, alter shadow to indicate whether the sprite has shadows enabled.
  shadow is preloaded with color & 0x80 so it doesn't need to be changed unless
  the game has special treatment (Aliens)
*/

READ8_DEVICE_HANDLER( k051960_r );
WRITE8_DEVICE_HANDLER( k051960_w );
READ16_DEVICE_HANDLER( k051960_word_r );
WRITE16_DEVICE_HANDLER( k051960_word_w );

READ8_DEVICE_HANDLER( k051937_r );
WRITE8_DEVICE_HANDLER( k051937_w );
READ16_DEVICE_HANDLER( k051937_word_r );
WRITE16_DEVICE_HANDLER( k051937_word_w );

void k051960_sprites_draw(running_device *device, bitmap_t *bitmap, const rectangle *cliprect, int min_priority, int max_priority);
int k051960_is_irq_enabled(running_device *device);
int k051960_is_nmi_enabled(running_device *device);
void k051960_set_sprite_offsets(running_device *device, int dx, int dy);

#if 0 // to be moved in the specific drivers!
/* special handling for the chips sharing address space */
READ8_HANDLER( k052109_051960_r );
WRITE8_HANDLER( k052109_051960_w );
#endif


/**  Konami 053244 / 053245  **/
READ16_DEVICE_HANDLER( k053245_word_r );
WRITE16_DEVICE_HANDLER( k053245_word_w );
READ8_DEVICE_HANDLER( k053245_r );
WRITE8_DEVICE_HANDLER( k053245_w );
READ8_DEVICE_HANDLER( k053244_r );
WRITE8_DEVICE_HANDLER( k053244_w );
READ16_DEVICE_HANDLER( k053244_lsb_r );
WRITE16_DEVICE_HANDLER( k053244_lsb_w );
READ16_DEVICE_HANDLER( k053244_word_r );
WRITE16_DEVICE_HANDLER( k053244_word_w );
void k053244_bankselect(running_device *device, int bank);	/* used by TMNT2, Asterix and Premier Soccer for ROM testing */
void k053245_sprites_draw(running_device *device, bitmap_t *bitmap, const rectangle *cliprect);
void k053245_sprites_draw_lethal(running_device *device, bitmap_t *bitmap, const rectangle *cliprect); /* for lethal enforcers */
void k053245_clear_buffer(running_device *device);
void k053245_set_sprite_offs(running_device *device, int offsx, int offsy);



/**  Konami 053246 / 053247 / 055673  **/
#define K055673_LAYOUT_GX  0
#define K055673_LAYOUT_RNG 1
#define K055673_LAYOUT_LE2 2
#define K055673_LAYOUT_GX6 3

READ16_DEVICE_HANDLER( k055673_rom_word_r );
READ16_DEVICE_HANDLER( k055673_GX6bpp_rom_word_r );

/*
Callback procedures for non-standard shadows:

1) translate shadow code to the correct 2-bit form (0=off, 1-3=style)
2) shift shadow code left by K053247_SHDSHIFT and add the K053247_CUSTOMSHADOW flag
3) combine the result with sprite color
*/
#define K053247_CUSTOMSHADOW	0x20000000
#define K053247_SHDSHIFT		20

READ8_DEVICE_HANDLER( k053247_r );
WRITE8_DEVICE_HANDLER( k053247_w );
READ16_DEVICE_HANDLER( k053247_word_r );
WRITE16_DEVICE_HANDLER( k053247_word_w );
READ32_DEVICE_HANDLER( k053247_long_r );
WRITE32_DEVICE_HANDLER( k053247_long_w );
WRITE16_DEVICE_HANDLER( k053247_reg_word_w ); // "OBJSET2" registers
WRITE32_DEVICE_HANDLER( k053247_reg_long_w );

void k053247_sprites_draw(running_device *device, bitmap_t *bitmap,const rectangle *cliprect);
int k053247_read_register(running_device *device, int regnum);
void k053247_set_sprite_offs(running_device *device, int offsx, int offsy);
void k053247_wraparound_enable(running_device *device, int status);
void k05324x_set_z_rejection(running_device *device, int zcode); // common to k053244/5
void k053247_set_z_rejection(running_device *device, int zcode); // common to k053246/7
void k053247_get_ram(running_device *device, UINT16 **ram);
int k053247_get_dx(running_device *device);
int k053247_get_dy(running_device *device);

READ8_DEVICE_HANDLER( k053246_r );
WRITE8_DEVICE_HANDLER( k053246_w );
READ16_DEVICE_HANDLER( k053246_word_r );
WRITE16_DEVICE_HANDLER( k053246_word_w );
READ32_DEVICE_HANDLER( k053246_long_r );
WRITE32_DEVICE_HANDLER( k053246_long_w );

void k053246_set_objcha_line(running_device *device, int state);
int k053246_is_irq_enabled(running_device *device);
int k053246_read_register(running_device *device, int regnum);

/**  Konami 051316  **/
/*
The callback is passed:
- code (range 00-FF, contents of the first tilemap RAM byte)
- color (range 00-FF, contents of the first tilemap RAM byte). Note that bit 6
  seems to be hardcoded as flip X.
The callback must put:
- in code the resulting tile number
- in color the resulting color index
- if necessary, put flags for the TileMap code in the tile_info
  structure (e.g. TILE_FLIPX)
*/

READ8_DEVICE_HANDLER( k051316_r );
WRITE8_DEVICE_HANDLER( k051316_w );
READ8_DEVICE_HANDLER( k051316_rom_r );
WRITE8_DEVICE_HANDLER( k051316_ctrl_w );
void k051316_zoom_draw(running_device *device, bitmap_t *bitmap,const rectangle *cliprect,int flags,UINT32 priority);
void k051316_wraparound_enable(running_device *device, int status);


/**  Konami 053936  **/
WRITE16_DEVICE_HANDLER( k053936_ctrl_w );
READ16_DEVICE_HANDLER( k053936_ctrl_r );	// FIXME: this is probably unused... to be checked!
WRITE16_DEVICE_HANDLER( k053936_linectrl_w );
READ16_DEVICE_HANDLER( k053936_linectrl_r );
void k053936_zoom_draw(running_device *device, bitmap_t *bitmap, const rectangle *cliprect, tilemap_t *tmap, int flags, UINT32 priority, int glfgreat_hack);
void k053936_wraparound_enable(running_device *device, int status);	// shall we merge this into the configuration intf?
void k053936_set_offset(running_device *device, int xoffs, int yoffs);	// shall we merge this into the configuration intf?


/**  Konami 053251 **/
/*
  Note: k053251_w() automatically does a tilemap_mark_all_tiles_dirty(ALL_TILEMAPS)
  when some palette index changes. If ALL_TILEMAPS is too expensive, use
  k053251_set_tilemaps() to indicate which tilemap is associated with each index.
 */
WRITE8_DEVICE_HANDLER( k053251_w );
WRITE16_DEVICE_HANDLER( k053251_lsb_w );
WRITE16_DEVICE_HANDLER( k053251_msb_w );
int k053251_get_priority(running_device *device, int ci);
int k053251_get_palette_index(running_device *device, int ci);
int k053251_get_tmap_dirty(running_device *device, int tmap_num);
void k053251_set_tmap_dirty(running_device *device, int tmap_num, int data);

enum
{
	K053251_CI0 = 0,
	K053251_CI1,
	K053251_CI2,
	K053251_CI3,
	K053251_CI4
};

/**  Konami 054000 **/
WRITE8_DEVICE_HANDLER( k054000_w );
READ8_DEVICE_HANDLER( k054000_r );
WRITE16_DEVICE_HANDLER( k054000_lsb_w );
READ16_DEVICE_HANDLER( k054000_lsb_r );


/**  Konami 051733 **/
WRITE8_DEVICE_HANDLER( k051733_w );
READ8_DEVICE_HANDLER( k051733_r );


/**  Konami 056832 **/
void k056832_SetExtLinescroll(running_device *device);	/* Lethal Enforcers */

#define K056832_DRAW_FLAG_FORCE_XYSCROLL		0x00800000

READ16_DEVICE_HANDLER( k056832_ram_word_r );
WRITE16_DEVICE_HANDLER( k056832_ram_word_w );
READ16_DEVICE_HANDLER( k056832_ram_half_word_r );
WRITE16_DEVICE_HANDLER( k056832_ram_half_word_w );
READ16_DEVICE_HANDLER( k056832_5bpp_rom_word_r );
READ32_DEVICE_HANDLER( k056832_5bpp_rom_long_r );
READ32_DEVICE_HANDLER( k056832_6bpp_rom_long_r );
READ16_DEVICE_HANDLER( k056832_rom_word_r );
READ16_DEVICE_HANDLER( k056832_mw_rom_word_r );
READ16_DEVICE_HANDLER( k056832_bishi_rom_word_r );
READ16_DEVICE_HANDLER( k056832_old_rom_word_r );
READ16_DEVICE_HANDLER( k056832_rom_word_8000_r );
WRITE16_DEVICE_HANDLER( k056832_word_w ); // "VRAM" registers
WRITE16_DEVICE_HANDLER( k056832_b_word_w );
READ8_DEVICE_HANDLER( k056832_ram_code_lo_r );
READ8_DEVICE_HANDLER( k056832_ram_code_hi_r );
READ8_DEVICE_HANDLER( k056832_ram_attr_lo_r );
READ8_DEVICE_HANDLER( k056832_ram_attr_hi_r );
WRITE8_DEVICE_HANDLER( k056832_ram_code_lo_w );
WRITE8_DEVICE_HANDLER( k056832_ram_code_hi_w );
WRITE8_DEVICE_HANDLER( k056832_ram_attr_lo_w );
WRITE8_DEVICE_HANDLER( k056832_ram_attr_hi_w );
WRITE8_DEVICE_HANDLER( k056832_w );
WRITE8_DEVICE_HANDLER( k056832_b_w );
void k056832_mark_plane_dirty(running_device *device, int num);
void k056832_mark_all_tmaps_dirty(running_device *device);
void k056832_tilemap_draw(running_device *device, bitmap_t *bitmap, const rectangle *cliprect, int num, UINT32 flags, UINT32 priority);
void k056832_tilemap_draw_dj(running_device *device, bitmap_t *bitmap, const rectangle *cliprect, int layer, UINT32 flags, UINT32 priority);
void k056832_set_layer_association(running_device *device, int status);
int  k056832_get_layer_association(running_device *device);
void k056832_set_layer_offs(running_device *device, int layer, int offsx, int offsy);
void k056832_set_lsram_page(running_device *device, int logical_page, int physical_page, int physical_offset);
void k056832_linemap_enable(running_device *device, int enable);
int  k056832_is_irq_enabled(running_device *device, int irqline);
void k056832_read_avac(running_device *device, int *mode, int *data);
int  k056832_read_register(running_device *device, int regnum);
int k056832_get_current_rambank(running_device *device);
int k056832_get_lookup(running_device *device, int bits);	/* Asterix */
void k056832_set_tile_bank(running_device *device, int bank);	/* Asterix */

READ32_DEVICE_HANDLER( k056832_ram_long_r );
READ32_DEVICE_HANDLER( k056832_rom_long_r );
WRITE32_DEVICE_HANDLER( k056832_ram_long_w );
READ32_DEVICE_HANDLER( k056832_unpaged_ram_long_r );
WRITE32_DEVICE_HANDLER( k056832_unpaged_ram_long_w );
WRITE32_DEVICE_HANDLER( k056832_long_w );
WRITE32_DEVICE_HANDLER( k056832_b_long_w );

/* bit depths for the 56832 */
#define K056832_BPP_4	0
#define K056832_BPP_5	1
#define K056832_BPP_6	2
#define K056832_BPP_8	3
#define K056832_BPP_4dj	4
#define K056832_BPP_8LE	5
#define K056832_BPP_8TASMAN	6

/**  Konami 055555  **/
void k055555_write_reg(running_device *device, UINT8 regnum, UINT8 regdat);
WRITE16_DEVICE_HANDLER( k055555_word_w );
WRITE32_DEVICE_HANDLER( k055555_long_w );
int k055555_read_register(running_device *device, int regnum);
int k055555_get_palette_index(running_device *device, int idx);


/* K055555 registers */
/* priority inputs */
#define K55_PALBASE_BG        0	// background palette
#define K55_CONTROL           1	// control register
#define K55_COLSEL_0          2	// layer A, B color depth
#define K55_COLSEL_1          3	// layer C, D color depth
#define K55_COLSEL_2          4	// object, S1 color depth
#define K55_COLSEL_3          5	// S2, S3 color depth

#define K55_PRIINP_0          7	// layer A pri 0
#define K55_PRIINP_1          8	// layer A pri 1
#define K55_PRIINP_2          9	// layer A "COLPRI"
#define K55_PRIINP_3          10	// layer B pri 0
#define K55_PRIINP_4          11	// layer B pri 1
#define K55_PRIINP_5          12	// layer B "COLPRI"
#define K55_PRIINP_6          13	// layer C pri
#define K55_PRIINP_7          14	// layer D pri
#define K55_PRIINP_8          15	// OBJ pri
#define K55_PRIINP_9          16	// sub 1 (GP:PSAC) pri
#define K55_PRIINP_10         17	// sub 2 (GX:PSAC) pri
#define K55_PRIINP_11         18	// sub 3 pri

#define K55_OINPRI_ON         19	// object priority bits selector

#define K55_PALBASE_A         23	// layer A palette
#define K55_PALBASE_B         24	// layer B palette
#define K55_PALBASE_C         25	// layer C palette
#define K55_PALBASE_D         26	// layer D palette
#define K55_PALBASE_OBJ       27	// OBJ palette
#define K55_PALBASE_SUB1      28	// SUB1 palette
#define K55_PALBASE_SUB2      29	// SUB2 palette
#define K55_PALBASE_SUB3      30	// SUB3 palette

#define K55_BLEND_ENABLES     33	// blend enables for tilemaps
#define K55_VINMIX_ON         34	// additional blend enables for tilemaps
#define K55_OSBLEND_ENABLES   35	// obj/sub blend enables
#define K55_OSBLEND_ON        36	// not sure, related to obj/sub blend

#define K55_SHAD1_PRI         37	// shadow/highlight 1 priority
#define K55_SHAD2_PRI         38	// shadow/highlight 2 priority
#define K55_SHAD3_PRI         39	// shadow/highlight 3 priority
#define K55_SHD_ON            40	// shadow/highlight
#define K55_SHD_PRI_SEL       41	// shadow/highlight

#define K55_VBRI              42	// VRAM layer brightness enable
#define K55_OSBRI             43	// obj/sub brightness enable, part 1
#define K55_OSBRI_ON          44	// obj/sub brightness enable, part 2
#define K55_INPUT_ENABLES     45	// input enables

/* bit masks for the control register */
#define K55_CTL_GRADDIR       0x01	// 0=vertical, 1=horizontal
#define K55_CTL_GRADENABLE    0x02	// 0=BG is base color only, 1=gradient
#define K55_CTL_FLIPPRI       0x04	// 0=standard Konami priority, 1=reverse
#define K55_CTL_SDSEL         0x08	// 0=normal shadow timing, 1=(not used by GX)

/* bit masks for the input enables */
#define K55_INP_VRAM_A        0x01
#define K55_INP_VRAM_B        0x02
#define K55_INP_VRAM_C        0x04
#define K55_INP_VRAM_D        0x08
#define K55_INP_OBJ           0x10
#define K55_INP_SUB1          0x20
#define K55_INP_SUB2          0x40
#define K55_INP_SUB3          0x80


/**  Konami 054338  **/
/* mixer/alpha blender */

WRITE16_DEVICE_HANDLER( k054338_word_w ); // "CLCT" registers
WRITE32_DEVICE_HANDLER( k054338_long_w );
int k054338_register_r(running_device *device, int reg);
void k054338_update_all_shadows(running_device *device, int rushingheroes_hack);			// called at the beginning of VIDEO_UPDATE()
void k054338_fill_solid_bg(running_device *device, bitmap_t *bitmap);				// solid backcolor fill
void k054338_fill_backcolor(running_device *device, bitmap_t *bitmap, int mode);	// unified fill, 0=solid, 1=gradient (by using a k055555)
int  k054338_set_alpha_level(running_device *device, int pblend);							// blend style 0-2
void k054338_invert_alpha(running_device *device, int invert);								// 0=0x00(invis)-0x1f(solid), 1=0x1f(invis)-0x00(solod)
//void K054338_export_config(running_device *device, int **shdRGB);

#define K338_REG_BGC_R		0
#define K338_REG_BGC_GB		1
#define K338_REG_SHAD1R		2
#define K338_REG_BRI3		11
#define K338_REG_PBLEND		13
#define K338_REG_CONTROL	15

#define K338_CTL_KILL		0x01	/* 0 = no video output, 1 = enable */
#define K338_CTL_MIXPRI		0x02
#define K338_CTL_SHDPRI		0x04
#define K338_CTL_BRTPRI		0x08
#define K338_CTL_WAILSL		0x10
#define K338_CTL_CLIPSL		0x20


/**  Konami 053250  **/
WRITE16_DEVICE_HANDLER( k053250_w );
READ16_DEVICE_HANDLER( k053250_r );
WRITE16_DEVICE_HANDLER( k053250_ram_w );
READ16_DEVICE_HANDLER( k053250_ram_r );
READ16_DEVICE_HANDLER( k053250_rom_r );

// K053250_draw() control flags
#define K053250_WRAP500		0x01
#define K053250_OVERDRIVE	0x02

void k053250_draw(running_device *device, bitmap_t *bitmap, const rectangle *cliprect, int colorbase, int flags, int pri);
void k053250_dma(running_device *device, int limiter);


/**  Konami 053252  **/
/* CRT and interrupt control unit */
READ16_DEVICE_HANDLER( k053252_word_r );	// CCU registers
WRITE16_DEVICE_HANDLER( k053252_word_w );
WRITE32_DEVICE_HANDLER( k053252_long_w );


/**  Konami 001006  **/
UINT32 k001006_get_palette(running_device *device, int index);

READ32_DEVICE_HANDLER( k001006_r );
WRITE32_DEVICE_HANDLER( k001006_w );


/**  Konami 001005  **/
void k001005_draw(running_device *device, bitmap_t *bitmap, const rectangle *cliprect);
void k001005_swap_buffers(running_device *device);
void k001005_preprocess_texture_data(UINT8 *rom, int length, int gticlub);

READ32_DEVICE_HANDLER( k001005_r );
WRITE32_DEVICE_HANDLER( k001005_w );


/**  Konami 001604  **/
void k001604_draw_back_layer( running_device *device, bitmap_t *bitmap, const rectangle *cliprect );
void k001604_draw_front_layer( running_device *device, bitmap_t *bitmap, const rectangle *cliprect );
WRITE32_DEVICE_HANDLER( k001604_tile_w );
READ32_DEVICE_HANDLER( k001604_tile_r );
WRITE32_DEVICE_HANDLER( k001604_char_w );
READ32_DEVICE_HANDLER( k001604_char_r );
WRITE32_DEVICE_HANDLER( k001604_reg_w );
READ32_DEVICE_HANDLER( k001604_reg_r );


/**  Konami 037122  **/
void k037122_tile_draw( running_device *device, bitmap_t *bitmap, const rectangle *cliprect );
READ32_DEVICE_HANDLER( k037122_sram_r );
WRITE32_DEVICE_HANDLER( k037122_sram_w );
READ32_DEVICE_HANDLER( k037122_char_r );
WRITE32_DEVICE_HANDLER( k037122_char_w );
READ32_DEVICE_HANDLER( k037122_reg_r );
WRITE32_DEVICE_HANDLER( k037122_reg_w );


// debug handlers
READ16_DEVICE_HANDLER( k056832_word_r );        // VACSET
READ16_DEVICE_HANDLER( k056832_b_word_r );      // VSCCS  (board dependent)
READ16_DEVICE_HANDLER( k053246_reg_word_r );    // OBJSET1
READ16_DEVICE_HANDLER( k053247_reg_word_r );    // OBJSET2
READ16_DEVICE_HANDLER( k053251_lsb_r );         // PCU1
READ16_DEVICE_HANDLER( k053251_msb_r );         // PCU1
READ16_DEVICE_HANDLER( k055555_word_r );        // PCU2
READ16_DEVICE_HANDLER( k054338_word_r );        // CLTC

READ32_DEVICE_HANDLER( k056832_long_r );        // VACSET
READ32_DEVICE_HANDLER( k053247_reg_long_r );    // OBJSET2
READ32_DEVICE_HANDLER( k055555_long_r );        // PCU2

READ16_DEVICE_HANDLER( k053244_reg_word_r );    // OBJSET0

#endif
