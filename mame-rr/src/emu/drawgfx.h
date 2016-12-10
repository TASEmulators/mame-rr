/*********************************************************************

    drawgfx.h

    Generic graphic functions.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __DRAWGFX_H__
#define __DRAWGFX_H__



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define MAX_GFX_PLANES			8
#define MAX_GFX_SIZE			32
#define MAX_ABS_GFX_SIZE		1024

#define EXTENDED_XOFFS			{ 0 }
#define EXTENDED_YOFFS			{ 0 }

#define GFX_ELEMENT_PACKED		1	/* two 4bpp pixels are packed in one byte of gfxdata */
#define GFX_ELEMENT_DONT_FREE	2	/* gfxdata was not malloc()ed, so don't free it on exit */

#define GFX_RAW 				0x12345678
/* When planeoffset[0] is set to GFX_RAW, the gfx data is left as-is, with no conversion.
   No buffer is allocated for the decoded data, and gfxdata is set to point to the source
   data.
   xoffset[0] is an optional displacement (*8) from the beginning of the source data, while
   yoffset[0] is the line modulo (*8) and charincrement the char modulo (*8). They are *8
   for consistency with the usual behaviour, but the bottom 3 bits are not used.
   GFX_ELEMENT_PACKED is automatically set if planes is <= 4.

   This special mode can be used to save memory in games that require several different
   handlings of the same ROM data (e.g. metro.c can use both 4bpp and 8bpp tiles, and both
   8x8 and 16x16; cps.c has 8x8, 16x16 and 32x32 tiles all fetched from the same ROMs).
*/

enum
{
	DRAWMODE_NONE,
	DRAWMODE_SOURCE,
	DRAWMODE_SHADOW
};



/***************************************************************************
    MACROS
***************************************************************************/

/* these macros describe gfx_layouts in terms of fractions of a region */
/* they can be used for total, planeoffset, xoffset, yoffset */
#define RGN_FRAC(num,den)		(0x80000000 | (((num) & 0x0f) << 27) | (((den) & 0x0f) << 23))
#define IS_FRAC(offset)			((offset) & 0x80000000)
#define FRAC_NUM(offset)		(((offset) >> 27) & 0x0f)
#define FRAC_DEN(offset)		(((offset) >> 23) & 0x0f)
#define FRAC_OFFSET(offset)		((offset) & 0x007fffff)

/* these macros are useful in gfx_layouts */
#define STEP2(START,STEP)		(START),(START)+(STEP)
#define STEP4(START,STEP)		STEP2(START,STEP),STEP2((START)+2*(STEP),STEP)
#define STEP8(START,STEP)		STEP4(START,STEP),STEP4((START)+4*(STEP),STEP)
#define STEP16(START,STEP)		STEP8(START,STEP),STEP8((START)+8*(STEP),STEP)
#define STEP32(START,STEP)		STEP16(START,STEP),STEP16((START)+16*(STEP),STEP)
#define STEP64(START,STEP)		STEP32(START,STEP),STEP32((START)+32*(STEP),STEP)
#define STEP128(START,STEP)		STEP64(START,STEP),STEP64((START)+64*(STEP),STEP)
#define STEP256(START,STEP)		STEP128(START,STEP),STEP128((START)+128*(STEP),STEP)
#define STEP512(START,STEP)		STEP256(START,STEP),STEP256((START)+256*(STEP),STEP)
#define STEP1024(START,STEP)	STEP512(START,STEP),STEP512((START)+512*(STEP),STEP)
#define STEP2048(START,STEP)	STEP1024(START,STEP),STEP1024((START)+1024*(STEP),STEP)


/* these macros are used for declaring gfx_decode_entry_entry info arrays. */
#define GFXDECODE_NAME( name ) gfxdecodeinfo_##name
#define GFXDECODE_EXTERN( name ) extern const gfx_decode_entry GFXDECODE_NAME(name)[]
#define GFXDECODE_START( name ) const gfx_decode_entry GFXDECODE_NAME(name)[] = {
#define GFXDECODE_ENTRY(region,offset,layout,start,colors) { region, offset, &layout, start, colors, 0, 0 },
#define GFXDECODE_SCALE(region,offset,layout,start,colors,xscale,yscale) { region, offset, &layout, start, colors, xscale, yscale },
#define GFXDECODE_END { 0 } };

/* these macros are used for declaring gfx_layout structures. */
#define GFXLAYOUT_RAW( name, planes, width, height, linemod, charmod ) \
const gfx_layout name = { width, height, RGN_FRAC(1,1), planes, { GFX_RAW }, { 0 }, { linemod }, charmod };



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _gfx_layout gfx_layout;
struct _gfx_layout
{
	UINT16			width;				/* pixel width of each element */
	UINT16			height;				/* pixel height of each element */
	UINT32			total;				/* total number of elements, or RGN_FRAC() */
	UINT16			planes;				/* number of bitplanes */
	UINT32			planeoffset[MAX_GFX_PLANES]; /* bit offset of each bitplane */
	UINT32			xoffset[MAX_GFX_SIZE]; /* bit offset of each horizontal pixel */
	UINT32			yoffset[MAX_GFX_SIZE]; /* bit offset of each vertical pixel */
	UINT32			charincrement;		/* distance between two consecutive elements (in bits) */
	const UINT32 *	extxoffs;			/* extended X offset array for really big layouts */
	const UINT32 *	extyoffs;			/* extended Y offset array for really big layouts */
};


class gfx_element
{
public:
	UINT16			width;				/* current pixel width of each element (changeble with source clipping) */
	UINT16			height;				/* current pixel height of each element (changeble with source clipping) */
	UINT16			startx;				/* current source clip X offset */
	UINT16			starty;				/* current source clip Y offset */

	UINT16			origwidth;			/* starting pixel width of each element */
	UINT16			origheight;			/* staring pixel height of each element */
	UINT8			flags;				/* one of the GFX_ELEMENT_* flags above */
	UINT32			total_elements;		/* total number of decoded elements */

	UINT32			color_base;			/* base color for rendering */
	UINT16			color_depth;		/* number of colors each pixel can represent */
	UINT16			color_granularity;	/* number of colors for each color code */
	UINT32			total_colors;		/* number of color codes */

	UINT32 *		pen_usage;			/* bitmask of pens that are used (pens 0-31 only) */

	UINT8 *			gfxdata;			/* pixel data, 8bpp or 4bpp (if GFX_ELEMENT_PACKED) */
	UINT32			line_modulo;		/* bytes between each row of data */
	UINT32			char_modulo;		/* bytes between each element */
	const UINT8 *	srcdata;			/* pointer to the source data for decoding */
	UINT8 *			dirty;				/* dirty array for detecting tiles that need decoding */
	UINT32			dirtyseq;			/* sequence number; incremented each time a tile is dirtied */

	running_machine *machine;			/* pointer to the owning machine */
	gfx_layout		layout;				/* copy of the original layout */
};


struct gfx_decode_entry
{
	const char *	memory_region;		/* memory region where the data resides */
	UINT32			start;				/* offset of beginning of data to decode */
	const gfx_layout *gfxlayout;		/* pointer to gfx_layout describing the layout; NULL marks the end of the array */
	UINT16			color_codes_start;	/* offset in the color lookup table where color codes start */
	UINT16			total_color_codes;	/* total number of color codes */
	UINT8			xscale;				/* optional horizontal scaling factor; 0 means 1x */
	UINT8			yscale;				/* optional vertical scaling factor; 0 means 1x */
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/


/* ----- graphics elements ----- */

/* allocate memory for the graphics elements referenced by a machine */
void gfx_init(running_machine *machine);

/* allocate a gfx_element structure based on a given layout */
gfx_element *gfx_element_alloc(running_machine *machine, const gfx_layout *gl, const UINT8 *srcdata, UINT32 total_colors, UINT32 color_base);

/* update a single code in a gfx_element */
void gfx_element_decode(const gfx_element *gfx, UINT32 code);

/* free a gfx_element */
void gfx_element_free(gfx_element *gfx);

/* create a temporary one-off gfx_element */
void gfx_element_build_temporary(gfx_element *gfx, running_machine *machine, UINT8 *base, UINT32 width, UINT32 height, UINT32 rowbytes, UINT32 color_base, UINT32 color_granularity, UINT32 flags);



/* ----- core graphics drawing ----- */

/* specific drawgfx implementations for each transparency type */
void drawgfx_opaque(bitmap_t *dest, const rectangle *cliprect, const gfx_element *gfx, UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty);
void drawgfx_transpen(bitmap_t *dest, const rectangle *cliprect, const gfx_element *gfx, UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty, UINT32 transpen);
void drawgfx_transpen_raw(bitmap_t *dest, const rectangle *cliprect, const gfx_element *gfx, UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty, UINT32 transpen);
void drawgfx_transmask(bitmap_t *dest, const rectangle *cliprect, const gfx_element *gfx, UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty, UINT32 transmask);
void drawgfx_transtable(bitmap_t *dest, const rectangle *cliprect, const gfx_element *gfx, UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty, const UINT8 *pentable, const pen_t *shadowtable);
void drawgfx_alpha(bitmap_t *dest, const rectangle *cliprect, const gfx_element *gfx, UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty, UINT32 transpen, UINT8 alpha);



/* ----- zoomed graphics drawing ----- */

/* specific drawgfxzoom implementations for each transparency type */
void drawgfxzoom_opaque(bitmap_t *dest, const rectangle *cliprect, const gfx_element *gfx, UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty, UINT32 scalex, UINT32 scaley);
void drawgfxzoom_transpen(bitmap_t *dest, const rectangle *cliprect, const gfx_element *gfx, UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty, UINT32 scalex, UINT32 scaley, UINT32 transpen);
void drawgfxzoom_transpen_raw(bitmap_t *dest, const rectangle *cliprect, const gfx_element *gfx, UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty, UINT32 scalex, UINT32 scaley, UINT32 transpen);
void drawgfxzoom_transmask(bitmap_t *dest, const rectangle *cliprect, const gfx_element *gfx, UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty, UINT32 scalex, UINT32 scaley, UINT32 transmask);
void drawgfxzoom_transtable(bitmap_t *dest, const rectangle *cliprect, const gfx_element *gfx, UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty, UINT32 scalex, UINT32 scaley, const UINT8 *pentable, const pen_t *shadowtable);
void drawgfxzoom_alpha(bitmap_t *dest, const rectangle *cliprect, const gfx_element *gfx, UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty, UINT32 scalex, UINT32 scaley, UINT32 transpen, UINT8 alpha);



/* ----- priority masked graphics drawing ----- */

/* specific pdrawgfx implementations for each transparency type */
void pdrawgfx_opaque(bitmap_t *dest, const rectangle *cliprect, const gfx_element *gfx, UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty, bitmap_t *priority, UINT32 pmask);
void pdrawgfx_transpen(bitmap_t *dest, const rectangle *cliprect, const gfx_element *gfx, UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty, bitmap_t *priority, UINT32 pmask, UINT32 transpen);
void pdrawgfx_transpen_raw(bitmap_t *dest, const rectangle *cliprect, const gfx_element *gfx, UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty, bitmap_t *priority, UINT32 pmask, UINT32 transpen);
void pdrawgfx_transmask(bitmap_t *dest, const rectangle *cliprect, const gfx_element *gfx, UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty, bitmap_t *priority, UINT32 pmask, UINT32 transmask);
void pdrawgfx_transtable(bitmap_t *dest, const rectangle *cliprect, const gfx_element *gfx, UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty, bitmap_t *priority, UINT32 pmask, const UINT8 *pentable, const pen_t *shadowtable);
void pdrawgfx_alpha(bitmap_t *dest, const rectangle *cliprect, const gfx_element *gfx, UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty, bitmap_t *priority, UINT32 pmask, UINT32 transpen, UINT8 alpha);



/* ----- priority masked zoomed graphics drawing ----- */

/* specific pdrawgfxzoom implementations for each transparency type */
void pdrawgfxzoom_opaque(bitmap_t *dest, const rectangle *cliprect, const gfx_element *gfx, UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty, UINT32 scalex, UINT32 scaley, bitmap_t *priority, UINT32 pmask);
void pdrawgfxzoom_transpen(bitmap_t *dest, const rectangle *cliprect, const gfx_element *gfx, UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty, UINT32 scalex, UINT32 scaley, bitmap_t *priority, UINT32 pmask, UINT32 transpen);
void pdrawgfxzoom_transpen_raw(bitmap_t *dest, const rectangle *cliprect, const gfx_element *gfx, UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty, UINT32 scalex, UINT32 scaley, bitmap_t *priority, UINT32 pmask, UINT32 transpen);
void pdrawgfxzoom_transmask(bitmap_t *dest, const rectangle *cliprect, const gfx_element *gfx, UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty, UINT32 scalex, UINT32 scaley, bitmap_t *priority, UINT32 pmask, UINT32 transmask);
void pdrawgfxzoom_transtable(bitmap_t *dest, const rectangle *cliprect, const gfx_element *gfx, UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty, UINT32 scalex, UINT32 scaley, bitmap_t *priority, UINT32 pmask, const UINT8 *pentable, const pen_t *shadowtable);
void pdrawgfxzoom_alpha(bitmap_t *dest, const rectangle *cliprect, const gfx_element *gfx, UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty, UINT32 scalex, UINT32 scaley, bitmap_t *priority, UINT32 pmask, UINT32 transpen, UINT8 alpha);



/* ----- scanline copying ----- */

/* copy pixels from an 8bpp buffer to a single scanline of a bitmap */
void draw_scanline8(bitmap_t *bitmap, INT32 destx, INT32 desty, INT32 length, const UINT8 *srcptr, const pen_t *paldata);

/* copy pixels from a 16bpp buffer to a single scanline of a bitmap */
void draw_scanline16(bitmap_t *bitmap, INT32 destx, INT32 desty, INT32 length, const UINT16 *srcptr, const pen_t *paldata);

/* copy pixels from a 32bpp buffer to a single scanline of a bitmap */
void draw_scanline32(bitmap_t *bitmap, INT32 destx, INT32 desty, INT32 length, const UINT32 *srcptr, const pen_t *paldata);



/* ----- scanline extraction ----- */

/* copy pixels from a single scanline of a bitmap to an 8bpp buffer */
void extract_scanline8(bitmap_t *bitmap, INT32 srcx, INT32 srcy, INT32 length, UINT8 *destptr);

/* copy pixels from a single scanline of a bitmap to a 16bpp buffer */
void extract_scanline16(bitmap_t *bitmap, INT32 srcx, INT32 srcy, INT32 length, UINT16 *destptr);

/* copy pixels from a single scanline of a bitmap to a 32bpp buffer */
void extract_scanline32(bitmap_t *bitmap, INT32 srcx, INT32 srcy, INT32 length, UINT32 *destptr);



/* ----- bitmap copying ----- */

/* copy from one bitmap to another, copying all unclipped pixels */
void copybitmap(bitmap_t *dest, bitmap_t *src, int flipx, int flipy, INT32 destx, INT32 desty, const rectangle *cliprect);

/* copy from one bitmap to another, copying all unclipped pixels except those that match transpen */
void copybitmap_trans(bitmap_t *dest, bitmap_t *src, int flipx, int flipy, INT32 destx, INT32 desty, const rectangle *cliprect, UINT32 transpen);

/*
  Copy a bitmap onto another with scroll and wraparound.
  These functions support multiple independently scrolling rows/columns.
  "rows" is the number of indepentently scrolling rows. "rowscroll" is an
  array of integers telling how much to scroll each row. Same thing for
  "numcols" and "colscroll".
  If the bitmap cannot scroll in one direction, set numrows or columns to 0.
  If the bitmap scrolls as a whole, set numrows and/or numcols to 1.
  Bidirectional scrolling is, of course, supported only if the bitmap
  scrolls as a whole in at least one direction.
*/

/* copy from one bitmap to another, copying all unclipped pixels, and applying scrolling to one or more rows/colums */
void copyscrollbitmap(bitmap_t *dest, bitmap_t *src, UINT32 numrows, const INT32 *rowscroll, UINT32 numcols, const INT32 *colscroll, const rectangle *cliprect);

/* copy from one bitmap to another, copying all unclipped pixels except those that match transpen, and applying scrolling to one or more rows/colums */
void copyscrollbitmap_trans(bitmap_t *dest, bitmap_t *src, UINT32 numrows, const INT32 *rowscroll, UINT32 numcols, const INT32 *colscroll, const rectangle *cliprect, UINT32 transpen);

/*
    Copy a bitmap applying rotation, zooming, and arbitrary distortion.
    This function works in a way that mimics some real hardware like the Konami
    051316, so it requires little or no further processing on the caller side.

    Two 16.16 fixed point counters are used to keep track of the position on
    the source bitmap. startx and starty are the initial values of those counters,
    indicating the source pixel that will be drawn at coordinates (0,0) in the
    destination bitmap. The destination bitmap is scanned left to right, top to
    bottom; every time the cursor moves one pixel to the right, incxx is added
    to startx and incxy is added to starty. Every time the cursor moves to the
    next line, incyx is added to startx and incyy is added to startyy.

    What this means is that if incxy and incyx are both 0, the bitmap will be
    copied with only zoom and no rotation. If e.g. incxx and incyy are both 0x8000,
    the source bitmap will be doubled.

    Rotation is performed this way:
    incxx = 0x10000 * cos(theta)
    incxy = 0x10000 * -sin(theta)
    incyx = 0x10000 * sin(theta)
    incyy = 0x10000 * cos(theta)
    this will perform a rotation around (0,0), you'll have to adjust startx and
    starty to move the center of rotation elsewhere.

    Optionally the bitmap can be tiled across the screen instead of doing a single
    copy. This is obtained by setting the wraparound parameter to true.
*/

/* copy from one bitmap to another, with zoom and rotation, copying all unclipped pixels */
void copyrozbitmap(bitmap_t *dest, const rectangle *cliprect, bitmap_t *src, INT32 startx, INT32 starty, INT32 incxx, INT32 incxy, INT32 incyx, INT32 incyy, int wraparound);

/* copy from one bitmap to another, with zoom and rotation, copying all unclipped pixels whose values do not match transpen */
void copyrozbitmap_trans(bitmap_t *dest, const rectangle *cliprect, bitmap_t *src, INT32 startx, INT32 starty, INT32 incxx, INT32 incxy, INT32 incyx, INT32 incyy, int wraparound, UINT32 transparent_color);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    gfx_element_set_source - set a pointer to
    the source of a gfx_element
-------------------------------------------------*/

INLINE void gfx_element_set_source(gfx_element *gfx, const UINT8 *source)
{
	gfx->srcdata = source;
	memset(gfx->dirty, 1, gfx->total_elements);
}


/*-------------------------------------------------
    gfx_element_get_data - return a pointer to
    the base of the given code within a
    gfx_element, decoding it if it is dirty
-------------------------------------------------*/

INLINE const UINT8 *gfx_element_get_data(const gfx_element *gfx, UINT32 code)
{
	assert(code < gfx->total_elements);
	if (gfx->dirty[code])
		gfx_element_decode(gfx, code);
	return gfx->gfxdata + code * gfx->char_modulo + gfx->starty * gfx->line_modulo + gfx->startx;
}


/*-------------------------------------------------
    gfx_element_mark_dirty - mark a code of a
    gfx_element dirty
-------------------------------------------------*/

INLINE void gfx_element_mark_dirty(gfx_element *gfx, UINT32 code)
{
	if (code < gfx->total_elements)
	{
		gfx->dirty[code] = 1;
		gfx->dirtyseq++;
	}
}


/*-------------------------------------------------
    gfx_element_set_source_clip - set a source
    clipping area to apply to subsequent renders
-------------------------------------------------*/

INLINE void gfx_element_set_source_clip(gfx_element *gfx, UINT32 xoffs, UINT32 width, UINT32 yoffs, UINT32 height)
{
	assert(xoffs < gfx->origwidth);
	assert(yoffs < gfx->origheight);
	assert(xoffs + width <= gfx->origwidth);
	assert(yoffs + height <= gfx->origheight);

	gfx->width = width;
	gfx->height = height;
	gfx->startx = xoffs;
	gfx->starty = yoffs;
}


/*-------------------------------------------------
    alpha_blend_r16 - alpha blend two 16-bit
    5-5-5 RGB pixels
-------------------------------------------------*/

INLINE UINT32 alpha_blend_r16(UINT32 d, UINT32 s, UINT8 level)
{
	int alphad = 256 - level;
	return ((((s & 0x001f) * level + (d & 0x001f) * alphad) >> 8)) |
		   ((((s & 0x03e0) * level + (d & 0x03e0) * alphad) >> 8) & 0x03e0) |
		   ((((s & 0x7c00) * level + (d & 0x7c00) * alphad) >> 8) & 0x7c00);
}


/*-------------------------------------------------
    alpha_blend_r32 - alpha blend two 32-bit
    8-8-8 RGB pixels
-------------------------------------------------*/

INLINE UINT32 alpha_blend_r32(UINT32 d, UINT32 s, UINT8 level)
{
	int alphad = 256 - level;
	return ((((s & 0x0000ff) * level + (d & 0x0000ff) * alphad) >> 8)) |
		   ((((s & 0x00ff00) * level + (d & 0x00ff00) * alphad) >> 8) & 0x00ff00) |
		   ((((s & 0xff0000) * level + (d & 0xff0000) * alphad) >> 8) & 0xff0000);
}


#endif	/* __DRAWGFX_H__ */
