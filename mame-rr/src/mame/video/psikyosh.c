/*
Psikyo PS6406B (PS3v1/PS5/PS5v2):
See src/drivers/psikyosh.c for more info

Hardware is extremely flexible (and luckily underused, although we now have a relatively complete implementation :). Many effects are subtle (e.g. fades, scanline effects).

Banks:
There are 32 data banks of 0x800 bytes starting at 0x3000000 (ps3) / 0x4000000 (ps5/ps5v2)

Can be one of:
* a set of adjacent banks for sprites (we always use the first 7 banks currently, looks to be configurable in vidregs
* a sprite draw list (max 1024 sprites, always in bank 07?)
* pre (i.e. screen clear) + post (i.e. drawn with a non-zero priority) line-fill/blend layer (usually bank 08)
* tilemap tiles (or 2 adjacent banks for large tilemaps) (usually banks 0c-1f)
* xscroll/yscroll, pri/xzoom/alpha/bank per-layer (of which there are four) -> which points to another tilebank per-layer (usually bank 0a/0b)
* xscroll/yscroll, pri/xzoom/alpha/bank per-scanline (typically 224) -> which points to another tilebank per-line (usually bank 0c/0d)

Most games use bank 0x0a and 0x0b for the registers for double-buffering, which then refer to other banks for the tiles. If they use line-effects, they tend to populate the per-line registers into one of the other banks e.g. daraku or S1945II test+level 7+level8 boss and S1945III levels 7+8, soldivid final boss.
*/

/*
BG Scroll/Priority/Zoom/Alpha/Tilebank registers:
Either at 0x30053f0/4/8 (For 0a), 0x3005bf0/4/8 (For 0b) etc. (i.e. in the middle of a bank) Or per-line in a bank
   0x?vvv?xxx - v = vertical scroll - x = x scroll
Either at 0x30057f0/4/8 (For 0a), 0x3005ff0/4/8 (For 0b) etc. (i.e. at the end of the bank) Or per-line in a bank
   0xppzzaabb - p = priority, z = zoom/expand(00 is none), a = alpha value/effect, b = tilebank

Video Registers: at 0x305ffe0 for ps3 or 0x405ffe0 for ps5/ps5v2:
0x00 -- ffffffff alpha values for sprites, 8-bits per value (0-0x3f, 0x80 indicates per-pen alpha). sbomberb = 0000 3830 2820 1810
0x04 -- ffffffff above continued.
0x08 -- ffff0000 priority values for sprites, 4-bits per value
        0000ff00 unknown. always 20. number of addressable banks? boards are populated with 20.
        000000f0 unknown. s1945ii/s1945iii/gunbird2/gnbarich/tgm2 sets to c. soldivid/daraku is 0. another bank select?
        0000000f is priority for per-line post-blending
0x0c -- 3f3f3f3f unknown. A table of 4 6-bit values. usually 0f102038. tgm2 is 0a172838.
        c0c00000 unknown. unused?
        0000c000 is flipscreen (currently ignored). presumably flipy<<1|flipx.
        000000c0 is screen size select. 0 is 224 lines, c is 240 (see tgm2, not confirmed).
0x10 -- ffff0000 is always 00aa
        0000f000 number of banks for sprites (not confirmed). mjgtaste/tgm2/sbomberb/s1945ii is 3, gunbird2/s1945iii is 2, soldivid/daraku is b.
        00000fff Controls gfx data bank available to be read by SH-2 for verification.
0x14 -- ffffffff always 83ff000e
0x18 -- ffffffff bank for tilemaps. As follows for the different tilemaps: 11223344. Bit 0x80 indicates use of line effects and the bank should be used to look up the tile-bank per line.
0x1c -- ff000000 controls bank for 'pre'/'post' values
        00ff0000 unknown, always 0?
        0000ffff enable bits for 4 tilemaps. 8 is enable. 4 indicates 8bpp tiles. 1 is size select for tilemap
*/

/*
TODO:
* Correct sprite-sprite priority? Currently this is strictly in the order of the sprites in the sprite list. However, there's an additional priority parameter which looks to split the sprites into 4 discrete sets with decreasing priority. In addition to the sprite-tilemap mixing the only way I can think to emulate this is how the hardware would work. Iterate over the sprite list 4 times rendering the sprites to a bitmap, and then mix each pixel against the tilemaps and other elements with comparable priority. This will be pretty slow though. Justification: The unknown priority bits are used to seperate score/enemy bullets from ships/enemies from incidental effects. daraku appears to have a black, screen-filling srite which it uses for a flash immediately efore the screen fade/white flash when doing special moves. Currently obscured behind the other sprites.
* Perform tests on real hardware to document limits and remaining registers
** Fix background line zoom to be pixel-correct. There must be an internal LUT.
** Confirm existence of 4th tilemap layer on real hw by configuring it. No games ever get as far as enabling it.
** Confirm sprite-sprite priority behaviours (two overlapping sprites in sprite list order, with differing priorities etc.)
** Figure out why the sprite zoom is not 100% when we even have a lookup table. See TGM2 MT report. Possibly we should offset calcs by half a pixel (i.e. start in the middle of the first source pixel rather than corner).
** Figure out screen size registers and xflip/yflip
* Hookup configurable sprite banks (not needed? reports of tgm2 dropping sprites when busy on real hw)
* Hookup screen size select
* Flip screen, located but not implemented. wait until tilemaps.
* The stuff might be converted to use the tilemaps once all the features is worked out ...
The only viable way to do this is to have one tilemap per bank (0x0a-0x20), and every pair of adjacent banks for large tilemaps. This is rather than having one per background layer, due to the line effects. Would also need to support all of the logic relating to alpha table blending, row and column scroll/zoom etc.
*/

#include "emu.h"
#include "profiler.h"
#include "drawgfxm.h"
#include "includes/psikyosh.h"
#include "ui.h"

//#define DEBUG_KEYS
//#define DEBUG_MESSAGE

static UINT8 alphatable[256];	// this might be moved to psikyosh_state, if we ever add a *machine parameter to drawgfxm.h macros


/*-------------------------------------------------
    palette.h like macros
-------------------------------------------------*/

//#define MAKE_ARGB_RGB(a, rgb) MAKE_ARGB(a, RGB_RED(rgb), RGB_GREEN(rgb), RGB_BLUE(rgb))
#define MAKE_ARGB_RGB(a, rgb)	((((rgb_t)(a) & 0xff) << 24) | ((rgb) & 0xffffff))

/*-------------------------------------------------
    drawgfxm.h like macros
-------------------------------------------------*/

// combine in 'alpha' when copying to store in ARGB
#define PIXEL_OP_REMAP_TRANS0_ALPHASTORE32(DEST, PRIORITY, SOURCE)									\
do																									\
{																									\
	UINT32 srcdata = (SOURCE);																		\
	if (srcdata != 0)																				\
		(DEST) = MAKE_ARGB_RGB(alpha,paldata[srcdata]);												\
}																									\
while (0)																							\

// combine in 'alphatable' value to store in ARGB
#define PIXEL_OP_REMAP_TRANS0_ALPHATABLESTORE32(DEST, PRIORITY, SOURCE)								\
do																									\
{																									\
	UINT32 srcdata = (SOURCE);																		\
	if (srcdata != 0)																				\
		(DEST) = MAKE_ARGB_RGB(alphatable[srcdata], paldata[srcdata]);								\
}																									\
while (0)																							\

// take ARGB pixel with stored alpha and blend in to RGB32 bitmap
#define PIXEL_OP_COPY_TRANSPEN_ARGBRENDER32(DEST, PRIORITY, SOURCE)								\
do																									\
{																									\
	UINT32 srcdata = (SOURCE);																		\
	if (srcdata != transpen)																		\
		(DEST) = alpha_blend_r32((DEST), srcdata, RGB_ALPHA(srcdata));								\
}																									\
while (0)																							\

// take RGB pixel with seperate alpha and blend in to RGB32 bitmap
#define PIXEL_OP_COPY_TRANSPEN_ALPHARENDER32(DEST, PRIORITY, SOURCE)								\
do																									\
{																									\
	UINT32 srcdata = (SOURCE);																		\
	if (srcdata != transpen)																		\
		(DEST) = alpha_blend_r32((DEST), srcdata, alpha);								\
}																									\
while (0)																							\

// take ARGB pixel with stored alpha and copy in to RGB32 bitmap, scipping BG_TRANSPEN
#define PIXEL_OP_COPY_TRANSPEN_RENDER32(DEST, PRIORITY, SOURCE)								\
do																									\
{																									\
	UINT32 srcdata = (SOURCE);																		\
	if (srcdata != transpen)																		\
		(DEST) = srcdata;																			\
}																									\
while (0)																							\

// drawgfxm.h macro to render alpha into 32-bit buffer
#define PIXEL_OP_REMAP_TRANS0_ALPHATABLE32(DEST, PRIORITY, SOURCE)									\
do																									\
{																									\
	UINT32 srcdata = (SOURCE);																		\
	if (srcdata != 0)																				\
		(DEST) = alpha_blend_r32((DEST), paldata[srcdata], alphatable[srcdata]);					\
}																									\
while (0)																							\

/*-------------------------------------------------
    draw_scanline32_alpha - take an RGB-encoded UINT32
    scanline and alpha-blend it into the destination bitmap
-------------------------------------------------*/
static void draw_scanline32_alpha(bitmap_t *bitmap, INT32 destx, INT32 desty, INT32 length, const UINT32 *srcptr, int alpha)
{
	bitmap_t *priority = NULL;	/* dummy, no priority in this case */
	UINT32 transpen = BG_TRANSPEN;

	assert(bitmap != NULL);
	assert(bitmap->bpp == 32);

	DRAWSCANLINE_CORE(UINT32, PIXEL_OP_COPY_TRANSPEN_ALPHARENDER32, NO_PRIORITY);
}

/*-------------------------------------------------
    draw_scanline32_argb - take an ARGB-encoded UINT32
    scanline and alpha-blend it into the destination bitmap
-------------------------------------------------*/
static void draw_scanline32_argb(bitmap_t *bitmap, INT32 destx, INT32 desty, INT32 length, const UINT32 *srcptr)
{
	bitmap_t *priority = NULL;	/* dummy, no priority in this case */
	UINT32 transpen = BG_TRANSPEN;

	assert(bitmap != NULL);
	assert(bitmap->bpp == 32);
	assert(bitmap->format == BITMAP_FORMAT_ARGB32);

	DRAWSCANLINE_CORE(UINT32, PIXEL_OP_COPY_TRANSPEN_ARGBRENDER32, NO_PRIORITY);
}

/*-------------------------------------------------
    draw_scanline32_tranpens - take an RGB-encoded UINT32
    scanline and copy it into the destination bitmap, testing for the special ARGB transpen
-------------------------------------------------*/
static void draw_scanline32_transpen(bitmap_t *bitmap, INT32 destx, INT32 desty, INT32 length, const UINT32 *srcptr)
{
	bitmap_t *priority = NULL;	/* dummy, no priority in this case */
	UINT32 transpen = BG_TRANSPEN;

	assert(bitmap != NULL);
	assert(bitmap->bpp == 32);
	assert(bitmap->format == BITMAP_FORMAT_ARGB32);

	DRAWSCANLINE_CORE(UINT32, PIXEL_OP_COPY_TRANSPEN_RENDER32, NO_PRIORITY);
}

/*-------------------------------------------------
    drawgfx_alphastore - render a gfx element with
    a single transparent pen, storing the alpha value
    in alpha field of ARGB32, negative alpha implies alphatable
-------------------------------------------------*/
static void drawgfx_alphastore(bitmap_t *dest, const rectangle *cliprect, const gfx_element *gfx,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		int fixedalpha)
{
	bitmap_t *priority = NULL;	/* dummy, no priority in this case */
	const pen_t *paldata;

	assert(dest != NULL);
	assert(dest->bpp == 32);
	assert(dest->format == BITMAP_FORMAT_ARGB32);
	assert(gfx != NULL);
	assert(alphatable != NULL);

	/* if we have a fixed alpha, call the standard drawgfx_transpen */
	if (fixedalpha == 0xff)
	{
		drawgfx_transpen(dest, cliprect, gfx, code, color, flipx, flipy, destx, desty, 0);
		return;
	}

	/* get final code and color, and grab lookup tables */
	code %= gfx->total_elements;
	color %= gfx->total_colors;
	paldata = &gfx->machine->pens[gfx->color_base + gfx->color_granularity * color];

	/* early out if completely transparent */
	if (gfx->pen_usage != NULL && (gfx->pen_usage[code] & ~(1 << 0)) == 0)
		return;

	if (fixedalpha >= 0)
	{
		UINT8 alpha = fixedalpha;
		DRAWGFX_CORE(UINT32, PIXEL_OP_REMAP_TRANS0_ALPHASTORE32, NO_PRIORITY);
	}
	else
	{
		DRAWGFX_CORE(UINT32, PIXEL_OP_REMAP_TRANS0_ALPHATABLESTORE32, NO_PRIORITY);
	}
}

/*-------------------------------------------------
    drawgfx_alphatable - render a sprite with either
    a fixed alpha value, or if alpha==-1 then uses
    the per-pen alphatable[] array
 -------------------------------------------------*/
static void drawgfx_alphatable(bitmap_t *dest, const rectangle *cliprect, const gfx_element *gfx,
		UINT32 code, UINT32 color, int flipx, int flipy, INT32 destx, INT32 desty,
		int fixedalpha)
{
	bitmap_t *priority = NULL;	/* dummy, no priority in this case */

	const pen_t *paldata;

	/* if we have a fixed alpha, call the standard drawgfx_alpha */
	if (fixedalpha >= 0)
	{
		drawgfx_alpha(dest, cliprect, gfx, code, color, flipx, flipy, destx, desty, 0, fixedalpha);
		return;
	}

	assert(dest != NULL);
	assert(dest->bpp == 32);
	assert(gfx != NULL);
	assert(alphatable != NULL);

	/* get final code and color, and grab lookup tables */
	code %= gfx->total_elements;
	color %= gfx->total_colors;
	paldata = &gfx->machine->pens[gfx->color_base + gfx->color_granularity * color];

	/* early out if completely transparent */
	if (gfx->pen_usage != NULL && (gfx->pen_usage[code] & ~(1 << 0)) == 0)
		return;

	DRAWGFX_CORE(UINT32, PIXEL_OP_REMAP_TRANS0_ALPHATABLE32, NO_PRIORITY);
}

/* Psikyo PS6406B */
/* --- BACKGROUNDS --- */

/* 'Normal' layers, no line/columnscroll. No per-line effects.
Zooming isn't supported just because it's not used and it would be slow */
static void draw_bglayer( running_machine *machine, int layer, bitmap_t *bitmap, const rectangle *cliprect, UINT8 req_pri )
{
	psikyosh_state *state = (psikyosh_state *)machine->driver_data;
	gfx_element *gfx;
	int offs = 0, sx, sy;
	int scrollx, scrolly, regbank, tilebank, alpha, alphamap, zoom, pri, size, width;

	assert(!BG_LINE(layer));

	gfx = BG_DEPTH_8BPP(layer) ? machine->gfx[1] : machine->gfx[0];
	size = BG_LARGE(layer) ? 32 : 16;
	width = 16 * size;

	regbank = BG_TYPE(layer);

	scrollx  = (state->bgram[(regbank * 0x800) / 4 + 0x3f0 / 4 + (layer * 0x04) / 4 - 0x4000 / 4] & 0x000001ff) >> 0;
	scrolly  = (state->bgram[(regbank * 0x800) / 4 + 0x3f0 / 4 + (layer * 0x04) / 4 - 0x4000 / 4] & 0x03ff0000) >> 16;

	tilebank = (state->bgram[(regbank * 0x800) / 4 + 0x7f0 / 4 + (layer * 0x04) / 4 - 0x4000 / 4] & 0x000000ff) >> 0;
	alpha    = (state->bgram[(regbank * 0x800) / 4 + 0x7f0 / 4 + (layer * 0x04) / 4 - 0x4000 / 4] & 0x00003f00) >> 8;
	alphamap = (state->bgram[(regbank * 0x800) / 4 + 0x7f0 / 4 + (layer * 0x04) / 4 - 0x4000 / 4] & 0x00008000) >> 15;
	zoom     = (state->bgram[(regbank * 0x800) / 4 + 0x7f0 / 4 + (layer * 0x04) / 4 - 0x4000 / 4] & 0x00ff0000) >> 16;
	pri      = (state->bgram[(regbank * 0x800) / 4 + 0x7f0 / 4 + (layer * 0x04) / 4 - 0x4000 / 4] & 0xff000000) >> 24;

	if(pri != req_pri) return;

	if (alphamap) /* alpha values are per-pen */
		alpha = -1;
	else
		alpha = pal6bit(0x3f - alpha);	/* 0x3f-0x00 maps to 0x00-0xff */

	if(zoom) {
		popmessage("draw_bglayer() zoom not implemented\nContact MAMEDEV");
	}

	if ((tilebank >= 0x0a) && (tilebank <= 0x1f)) /* 20 banks of 0x800 bytes. filter garbage. */
	{
		for (sy = 0; sy < size; sy++)
		{
			for (sx = 0; sx < 32; sx++)
			{
				int tileno, colour;

				tileno = (state->bgram[(tilebank * 0x800) / 4 + offs - 0x4000 / 4] & 0x0007ffff); /* seems to take into account spriteram, hence -0x4000 */
				colour = (state->bgram[(tilebank * 0x800) / 4 + offs - 0x4000 / 4] & 0xff000000) >> 24;

				drawgfx_alphatable(bitmap, cliprect, gfx, tileno, colour, 0, 0, (16 * sx + scrollx) & 0x1ff, ((16 * sy + scrolly) & (width - 1)), alpha); /* normal */

				if (scrollx)
					drawgfx_alphatable(bitmap, cliprect, gfx, tileno, colour, 0, 0, ((16 * sx + scrollx) & 0x1ff) - 0x200, ((16 * sy + scrolly) & (width - 1)), alpha); /* wrap x */
				if (scrolly)
					drawgfx_alphatable(bitmap, cliprect, gfx, tileno, colour, 0, 0, (16 * sx + scrollx) & 0x1ff, ((16 * sy + scrolly) & (width - 1)) - width, alpha); /* wrap y */
				if (scrollx && scrolly)
					drawgfx_alphatable(bitmap, cliprect, gfx, tileno, colour, 0, 0, ((16 * sx + scrollx) & 0x1ff) - 0x200, ((16 * sy + scrolly) & (width - 1)) - width, alpha); /* wrap xy */

				offs++;
			}
		}
	}
}


/* populate state->bg_bitmap for the given bank if it's not already */
static void cache_bitmap(int scanline, psikyosh_state *state, gfx_element *gfx, int size, int tilebank, int alpha, int *last_bank)
{
	// test if the tile row is the cached one or not
	int sy = scanline / 16;

	assert(sy > 0 && sy < 32);

	if(tilebank != last_bank[sy])
	{
		rectangle cliprect;

		cliprect.min_x = 0;
		cliprect.max_x = state->bg_bitmap->width - 1;
		cliprect.min_y = sy * 16;
		cliprect.max_y = cliprect.min_y + 16 - 1;

		bitmap_fill(state->bg_bitmap, &cliprect, BG_TRANSPEN);
		int width = size * 16;

		int offs = size * sy;
		int sx;

		for (sx = 0; sx < 32; sx++)
		{
			int tileno, colour;

			tileno = (state->bgram[(tilebank * 0x800) / 4 + offs - 0x4000 / 4] & 0x0007ffff); /* seems to take into account spriteram, hence -0x4000 */
			colour = (state->bgram[(tilebank * 0x800) / 4 + offs - 0x4000 / 4] & 0xff000000) >> 24;
			int need_alpha = alpha < 0 ? -1 : 0xff; // store per-pen alpha in bitmap, otherwise don't since we'll need it per-line

			if(tileno) { // valid tile, but blank in all games?
				drawgfx_alphastore(state->bg_bitmap, NULL, gfx, tileno, colour, 0, 0, (16 * sx) & 0x1ff, ((16 * sy) & (width - 1)), need_alpha);
			}

			offs++;
		}
		last_bank[sy] = tilebank;
	}
}


/* Row Scroll/Zoom and/or Column Zoom, has per-column Alpha/Bank/Priority
Bitmap is first rendered to an ARGB image, taking into account the per-pen alpha (if used).
From there we extract data as we compose the image, one scanline at a time, blending the ARGB pixels
into the RGB32 bitmap (with either the alpha information from the ARGB, or per-line alpha */
static void draw_bglayerscroll( running_machine *machine, int layer, bitmap_t *bitmap, const rectangle *cliprect, UINT8 req_pri )
{
	psikyosh_state *state = (psikyosh_state *)machine->driver_data;
	assert(BG_LINE(layer));

	gfx_element *gfx = BG_DEPTH_8BPP(layer) ? machine->gfx[1] : machine->gfx[0];
	int size = BG_LARGE(layer) ? 32 : 16;
	int width = size * 16;

	int linebank = BG_TYPE(layer);

	/* cache rendered bitmap */
	int last_bank[32]; // corresponds to bank of bitmap in state->bg_bitmap. bg_bitmap is split into 16/32-rows of one-tile high each
	for(int ii = 0; ii < 32; ii++) last_bank[ii] = -1;

	int scr_width = (cliprect->max_x-cliprect->min_x + 1);
	int scr_height = (cliprect->max_y-cliprect->min_y + 1);
	UINT32 *scroll_reg = &state->bgram[(linebank * 0x800) / 4 - 0x4000 / 4];
	UINT32 *pzab_reg   = &state->bgram[(linebank * 0x800) / 4 - 0x4000 / 4 + 0x400 / 4]; // pri, zoom, alpha, bank

// now, for each scanline, check priority,
// extract the relevant scanline from the bitmap, after applying per-scanline vscroll,
// stretch it and scroll it into another buffer
// write it with alpha
	for(int scanline = 0; scanline < scr_height; scanline++)
	{
		int pri = (*pzab_reg & 0xff000000) >> 24;

		if(pri == req_pri)
		{
			int scrollx  = (*scroll_reg & 0x000001ff) >> 0;
			int scrolly  = (*scroll_reg & 0x03ff0000) >> 16;

			int zoom     = (*pzab_reg & 0x00ff0000) >> 16;
			int alphamap = (*pzab_reg & 0x00008000) >> 15;
			int alpha    = (*pzab_reg & 0x00003f00) >> 8;
			int tilebank = (*pzab_reg & 0x000000ff) >> 0;

			if(alphamap) /* alpha values are per-pen */
				alpha = -1;
			else
				alpha = pal6bit(0x3f - alpha);

			if ((tilebank >= 0x0a) && (tilebank <= 0x1f)) /* 20 banks of 0x800 bytes. filter garbage. */
			{
				int tilemap_scanline = (scanline - scrolly + 0x400) % 0x200;

				// render reelvant tiles to temp bitmap, assume bank changes infrequently/never. render alpha as per-pen
				cache_bitmap(tilemap_scanline, state, gfx, size, tilebank, alpha, last_bank);

				/* zoomy and 'wibbly' effects - extract an entire row from tilemap */
				profiler_mark_start(PROFILER_USER2);
				UINT32 tilemap_line[32 * 16];
				UINT32 scr_line[64 * 8];
				extract_scanline32(state->bg_bitmap, 0, tilemap_scanline, width, tilemap_line);
				profiler_mark_end();

				/* slow bit, needs optimising. apply scrollx and zoomx by assembling scanline from row */
				profiler_mark_start(PROFILER_USER3);
				if(zoom) {
					int step = state->bg_zoom[zoom];
					int jj = 0x400 << 10; // ensure +ve for mod
					for(int ii = 0; ii < scr_width; ii++) {
						scr_line[ii] = tilemap_line[((jj>>10) - scrollx) % width];
						jj += step;
					}
				}
				else {
					for(int ii = 0; ii < scr_width; ii++) {
						scr_line[ii] = tilemap_line[(ii - scrollx + 0x400) % width];
					}
				}
				profiler_mark_end();

				/* blend line into output */
				profiler_mark_start(PROFILER_USER4);
				if(alpha == 0xff) {
					draw_scanline32_transpen(bitmap, 0, scanline, scr_width, scr_line);
				}
				else if (alpha > 0) {
					draw_scanline32_alpha(bitmap, 0, scanline, scr_width, scr_line, alpha);
				}
				else if (alpha < 0) {
					draw_scanline32_argb(bitmap, 0, scanline, scr_width, scr_line);
				}
				profiler_mark_end();
			}
		}

		scroll_reg++;
		pzab_reg++;
	}
}

/* 3 BG layers, with priority */
static void draw_background( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, UINT8 req_pri )
{
	psikyosh_state *state = (psikyosh_state *)machine->driver_data;
	int i;

#ifdef DEBUG_KEYS
	const int lay_keys[8] = {KEYCODE_Q, KEYCODE_W, KEYCODE_E, KEYCODE_R};
	bool lay_debug = false;
	for (i = 0; i <= 3; i++)
	{
		if(input_code_pressed(machine, lay_keys[i])) {
			lay_debug = true;
		}
	}
#endif

	/* 1st-4th layers */
	for (i = 0; i <= 3; i++)
	{
#ifdef DEBUG_KEYS
		if(lay_debug && !input_code_pressed(machine, lay_keys[i]))
			continue;
#endif

		if (!BG_LAYER_ENABLE(i))
			continue;

		if(BG_LINE(i)) {
			/* per-line alpha, scroll, zoom etc. check the priority for the first scanline */
			draw_bglayerscroll(machine, i, bitmap, cliprect, req_pri);
		}
		else {
			/* not per-line alpha, scroll, zoom etc. */
			draw_bglayer(machine, i, bitmap, cliprect, req_pri);
		}
	}
}

/* --- SPRITES --- */

/* 32-bit ONLY */
/* zoomx/y are pixel slopes in 6.10 fixed point, not scale. 0x400 is 1:1. drawgfx zoom algorithm doesn't produce identical results to hardware. */
/* high/wide are number of tiles wide/high up to max size of zoom_bitmap in either direction */
/* code is index of first tile and incremented across rows then down columns (adjusting for flip obviously) */
/* sx and sy is top-left of entire sprite regardless of flip */
/* Note that Level 5-4 of sbomberb boss is perfect! (Alpha blended zoomed) as well as S1945II logo */
/* pixel is only plotted if z is >= priority_buffer[y][x] */
static void psikyosh_drawgfxzoom( running_machine *machine,
		bitmap_t *dest_bmp,const rectangle *clip,const gfx_element *gfx,
		UINT32 code,UINT32 color,int flipx,int flipy,int offsx,int offsy,
		int alpha, int zoomx, int zoomy, int wide, int high, UINT32 z)
{
	psikyosh_state *state = (psikyosh_state *)machine->driver_data;
	rectangle myclip; /* Clip to screen boundaries */
	int code_offset = 0;
	int xtile, ytile, xpixel, ypixel;

	if (!zoomx || !zoomy)
		return;

	profiler_mark_start(PROFILER_DRAWGFX);

	assert(dest_bmp->bpp == 32);

	/* KW 991012 -- Added code to force clip to bitmap boundary */
	if (clip)
	{
		myclip.min_x = clip->min_x;
		myclip.max_x = clip->max_x;
		myclip.min_y = clip->min_y;
		myclip.max_y = clip->max_y;

		if (myclip.min_x < 0) myclip.min_x = 0;
		if (myclip.max_x >= dest_bmp->width) myclip.max_x = dest_bmp->width - 1;
		if (myclip.min_y < 0) myclip.min_y = 0;
		if (myclip.max_y >= dest_bmp->height) myclip.max_y = dest_bmp->height-1;

		clip = &myclip;
	}

	/* Temporary fallback for non-zoomed, needs z-buffer. Note that this is probably a lot slower than drawgfx.c, especially if there was seperate code for flipped cases */
	if (zoomx == 0x400 && zoomy == 0x400)
	{
		int xstart, ystart, xend, yend, xinc, yinc;

		if (flipx)	{ xstart = wide - 1; xend = -1;   xinc = -1; }
		else		{ xstart = 0;        xend = wide; xinc = +1; }

		if (flipy)	{ ystart = high - 1; yend = -1;   yinc = -1; }
		else		{ ystart = 0;        yend = high; yinc = +1; }

		/* Start drawing */
		if (gfx)
		{
			for (ytile = ystart; ytile != yend; ytile += yinc)
			{
				for (xtile = xstart; xtile != xend; xtile += xinc)
				{
					const pen_t *pal = &gfx->machine->pens[gfx->color_base + gfx->color_granularity * (color % gfx->total_colors)];
					const UINT8 *code_base = gfx_element_get_data(gfx, (code + code_offset++) % gfx->total_elements);

					int x_index_base, y_index, sx, sy, ex, ey;

					if (flipx)	{ x_index_base = gfx->width - 1; }
					else		{ x_index_base = 0; }

					if (flipy)	{ y_index = gfx->height-1; }
					else		{ y_index = 0; }

					/* start coordinates */
					sx = offsx + xtile * gfx->width;
					sy = offsy + ytile * gfx->height;

					/* end coordinates */
					ex = sx + gfx->width;
					ey = sy + gfx->height;

					if (clip)
					{
						if (sx < clip->min_x)
						{ /* clip left */
							int pixels = clip->min_x - sx;
							sx += pixels;
							x_index_base += xinc * pixels;
						}
						if (sy < clip->min_y)
						{ /* clip top */
							int pixels = clip->min_y - sy;
							sy += pixels;
							y_index += yinc * pixels;
						}
						/* NS 980211 - fixed incorrect clipping */
						if (ex > clip->max_x + 1)
						{ /* clip right */
							int pixels = ex - clip->max_x - 1;
							ex -= pixels;
						}
						if (ey > clip->max_y + 1)
						{ /* clip bottom */
							int pixels = ey - clip->max_y - 1;
							ey -= pixels;
						}
					}

					if (ex > sx)
					{ /* skip if inner loop doesn't draw anything */
						int y;

						/* case 1: no alpha */
						if (alpha == 0xff)
						{
							if (z > 0)
							{
								const UINT8 *source = code_base + (y_index) * gfx->line_modulo + x_index_base;
								UINT32 *dest = (UINT32 *)dest_bmp->base + sy * dest_bmp->rowpixels + sx;
								UINT16 *pri = (UINT16 *)state->z_bitmap->base + sy * state->z_bitmap->rowpixels + sx;
								int src_modulo = yinc * gfx->line_modulo - xinc * (ex - sx);
								int dst_modulo = dest_bmp->rowpixels - (ex - sx);

								for (y = sy; y < ey; y++)
								{
									int x;
									for (x = sx; x < ex; x++)
									{
										if (z >= *pri)
										{
											int c = *source;
											if (c != 0)
											{
												*dest = pal[c];
												*pri = z;
											}
										}
										dest++;
										pri++;
										source += xinc;
									}
									dest += dst_modulo;
									pri += dst_modulo;
									source += src_modulo;
								}
							}
							else
							{
								const UINT8 *source = code_base + y_index * gfx->line_modulo + x_index_base;
								UINT32 *dest = (UINT32 *)dest_bmp->base + sy * dest_bmp->rowpixels + sx;
								int src_modulo = yinc * gfx->line_modulo - xinc * (ex - sx);
								int dst_modulo = dest_bmp->rowpixels - (ex - sx);

								for (y = sy; y < ey; y++)
								{
									int x;
									for (x = sx; x < ex; x++)
									{
										int c = *source;
										if (c != 0)
											*dest = pal[c];

										dest++;
										source += xinc;
									}
									dest += dst_modulo;
									source += src_modulo;
								}
							}
						}

						/* case 6: alpha-blended */
						else if (alpha >= 0)
						{
							if (z > 0)
							{
								const UINT8 *source = code_base + y_index * gfx->line_modulo + x_index_base;
								UINT32 *dest = (UINT32 *)dest_bmp->base + sy * dest_bmp->rowpixels + sx;
								UINT16 *pri = (UINT16 *)state->z_bitmap->base + sy * state->z_bitmap->rowpixels + sx;
								int src_modulo = yinc * gfx->line_modulo - xinc * (ex - sx);
								int dst_modulo = dest_bmp->rowpixels - (ex - sx);

								for (y = sy; y < ey; y++)
								{
									int x;
									for (x = sx; x < ex; x++)
									{
										if (z >= *pri)
										{
											int c = *source;
											if (c != 0)
											{
												*dest = alpha_blend_r32(*dest, pal[c], alpha);
												*pri = z;
											}
										}
										dest++;
										pri++;
										source += xinc;
									}
									dest += dst_modulo;
									pri += dst_modulo;
									source += src_modulo;
								}
							}
							else
							{
								const UINT8 *source = code_base + y_index * gfx->line_modulo + x_index_base;
								UINT32 *dest = (UINT32 *)dest_bmp->base + sy * dest_bmp->rowpixels + sx;
								int src_modulo = yinc * gfx->line_modulo - xinc * (ex - sx);
								int dst_modulo = dest_bmp->rowpixels - (ex - sx);

								for (y = sy; y < ey; y++)
								{
									int x;
									for (x = sx; x < ex; x++)
									{
										int c = *source;
										if (c != 0)
											*dest = alpha_blend_r32(*dest, pal[c], alpha);

										dest++;
										source += xinc;
									}
									dest += dst_modulo;
									source += src_modulo;
								}
							}
						}

						/* pjp 31/5/02 */
						/* case 7: TRANSPARENCY_ALPHARANGE */
						else
						{
							if (z > 0)
							{
								const UINT8 *source = code_base + y_index * gfx->line_modulo + x_index_base;
								UINT32 *dest = (UINT32 *)dest_bmp->base + sy * dest_bmp->rowpixels + sx;
								UINT16 *pri = (UINT16 *)state->z_bitmap->base + sy * state->z_bitmap->rowpixels + sx;
								int src_modulo = yinc * gfx->line_modulo - xinc * (ex - sx);
								int dst_modulo = dest_bmp->rowpixels - (ex - sx);

								for (y = sy; y < ey; y++)
								{
									int x;
									for (x = sx; x < ex; x++)
									{
										if (z >= *pri)
										{
											int c = *source;
											if (c != 0)
											{
												if (alphatable[c] == 0xff)
													*dest = pal[c];
												else
													*dest = alpha_blend_r32(*dest, pal[c], alphatable[c]);

												*pri = z;
											}
										}
										dest++;
										pri++;
										source += xinc;
									}
									dest += dst_modulo;
									pri += dst_modulo;
									source += src_modulo;
								}
							}
							else
							{
								const UINT8 *source = code_base + y_index * gfx->line_modulo + x_index_base;
								UINT32 *dest = (UINT32 *)dest_bmp->base + sy * dest_bmp->rowpixels + sx;
								int src_modulo = yinc * gfx->line_modulo - xinc * (ex - sx);
								int dst_modulo = dest_bmp->rowpixels - (ex - sx);

								for (y = sy; y < ey; y++)
								{
									int x;
									for (x = sx; x < ex; x++)
									{
										int c = *source;
										if (c != 0)
										{
											if (alphatable[c] == 0xff)
												*dest = pal[c];
											else
												*dest = alpha_blend_r32(*dest, pal[c], alphatable[c]);
										}
										dest++;
										source += xinc;
									}
									dest += dst_modulo;
									source += src_modulo;
								}
							}
						}

					}
				}
			}
		}
	}
	else /* Zoomed */
	{
		/* Make a copy of complete sprite at top-left of zoom_bitmap */
		/* Because I'm too slow to get it to work on the fly */
		for (ytile = 0; ytile < high; ytile++)
		{
			for (xtile = 0; xtile < wide; xtile++)
			{
				const UINT8 *code_base = gfx_element_get_data(gfx, (code + code_offset++) % gfx->total_elements);
				for (ypixel = 0; ypixel < gfx->height; ypixel++)
				{
					const UINT8 *source = code_base + ypixel * gfx->line_modulo;
					UINT8 *dest = BITMAP_ADDR8(state->zoom_bitmap, ypixel + ytile*gfx->height, 0);

					for (xpixel = 0; xpixel < gfx->width; xpixel++)
					{
						dest[xpixel + xtile*gfx->width] = source[xpixel];
					}
				}
			}
		}

		/* Start drawing */
		if (gfx)
		{
			const pen_t *pal = &gfx->machine->pens[gfx->color_base + gfx->color_granularity * (color % gfx->total_colors)];

			int sprite_screen_height = ((high * gfx->height * (0x400 * 0x400)) / zoomy + 0x200) >> 10; /* Round up to nearest pixel */
			int sprite_screen_width = ((wide * gfx->width * (0x400 * 0x400)) / zoomx + 0x200) >> 10;

			if (sprite_screen_width && sprite_screen_height)
			{
				/* start coordinates */
				int sx = offsx;
				int sy = offsy;

				/* end coordinates */
				int ex = sx + sprite_screen_width;
				int ey = sy + sprite_screen_height;

				int x_index_base;
				int y_index;

				int dx, dy;

				if (flipx)	{ x_index_base = (sprite_screen_width - 1) * zoomx; dx = -zoomx; }
				else		{ x_index_base = 0; dx = zoomx; }

				if (flipy)	{ y_index = (sprite_screen_height - 1) * zoomy; dy = -zoomy; }
				else		{ y_index = 0; dy = zoomy; }

				if (clip)
				{
					if (sx < clip->min_x)
					{ /* clip left */
						int pixels = clip->min_x - sx;
						sx += pixels;
						x_index_base += pixels * dx;
					}
					if (sy < clip->min_y)
					{ /* clip top */
						int pixels = clip->min_y - sy;
						sy += pixels;
						y_index += pixels * dy;
					}
					/* NS 980211 - fixed incorrect clipping */
					if (ex > clip->max_x + 1)
					{ /* clip right */
						int pixels = ex-clip->max_x - 1;
						ex -= pixels;
					}
					if (ey > clip->max_y + 1)
					{ /* clip bottom */
						int pixels = ey-clip->max_y - 1;
						ey -= pixels;
					}
				}

				if (ex > sx)
				{ /* skip if inner loop doesn't draw anything */
					int y;

					/* case 1: no alpha */
					/* Note: adjusted to >>10 and draws from zoom_bitmap not gfx */
					if (alpha == 0xff)
					{
						if (z > 0)
						{
							for (y = sy; y < ey; y++)
							{
								UINT8 *source = BITMAP_ADDR8(state->zoom_bitmap, y_index >> 10, 0);
								UINT32 *dest = BITMAP_ADDR32(dest_bmp, y, 0);
								UINT16 *pri = BITMAP_ADDR16(state->z_bitmap, y, 0);

								int x, x_index = x_index_base;
								for (x = sx; x < ex; x++)
								{
									if (z >= pri[x])
									{
										int c = source[x_index >> 10];
										if (c != 0)
										{
											dest[x] = pal[c];
											pri[x] = z;
										}
									}
									x_index += dx;
								}

								y_index += dy;
							}
						}
						else
						{
							for (y = sy; y < ey; y++)
							{
								UINT8 *source = BITMAP_ADDR8(state->zoom_bitmap, y_index >> 10, 0);
								UINT32 *dest = BITMAP_ADDR32(dest_bmp, y, 0);

								int x, x_index = x_index_base;
								for (x = sx; x < ex; x++)
								{
									int c = source[x_index >> 10];
									if (c != 0)
										dest[x] = pal[c];
									x_index += dx;
								}

								y_index += dy;
							}
						}
					}

					/* case 6: alpha-blended */
					else if (alpha >= 0)
					{
						if (z > 0)
						{
							for (y = sy; y < ey; y++)
							{
								UINT8 *source = BITMAP_ADDR8(state->zoom_bitmap, y_index >> 10, 0);
								UINT32 *dest = BITMAP_ADDR32(dest_bmp, y, 0);
								UINT16 *pri = BITMAP_ADDR16(state->z_bitmap, y, 0);

								int x, x_index = x_index_base;
								for (x = sx; x < ex; x++)
								{
									if (z >= pri[x])
									{
										int c = source[x_index >> 10];
										if (c != 0)
										{
											dest[x] = alpha_blend_r32(dest[x], pal[c], alpha);
											pri[x] = z;
										}
									}
									x_index += dx;
								}

								y_index += dy;
							}
						}
						else
						{
							for (y = sy; y < ey; y++)
							{
								UINT8 *source = BITMAP_ADDR8(state->zoom_bitmap, y_index >> 10, 0);
								UINT32 *dest = BITMAP_ADDR32(dest_bmp, y, 0);

								int x, x_index = x_index_base;
								for (x = sx; x < ex; x++)
								{
									int c = source[x_index >> 10];
									if (c != 0) dest[x] = alpha_blend_r32(dest[x], pal[c], alpha);
									x_index += dx;
								}

								y_index += dy;
							}
						}
					}

					/* case 7: TRANSPARENCY_ALPHARANGE */
					else
					{
						if (z > 0)
						{
							for (y = sy; y < ey; y++)
							{
								UINT8 *source = BITMAP_ADDR8(state->zoom_bitmap, y_index >> 10, 0);
								UINT32 *dest = BITMAP_ADDR32(dest_bmp, y, 0);
								UINT16 *pri = BITMAP_ADDR16(state->z_bitmap, y, 0);

								int x, x_index = x_index_base;
								for (x = sx; x < ex; x++)
								{
									if (z >= pri[x])
									{
										int c = source[x_index >> 10];
										if (c != 0)
										{
											if (alphatable[c] == 0xff)
												dest[x] = pal[c];
											else
												dest[x] = alpha_blend_r32(dest[x], pal[c], alphatable[c]);

											pri[x] = z;
										}
									}
									x_index += dx;
								}

								y_index += dy;
							}
						}
						else
						{
							for (y = sy; y < ey; y++)
							{
								UINT8 *source = BITMAP_ADDR8(state->zoom_bitmap, y_index >> 10, 0);
								UINT32 *dest = BITMAP_ADDR32(dest_bmp, y, 0);

								int x, x_index = x_index_base;
								for (x = sx; x < ex; x++)
								{
									int c = source[x_index >> 10];
									if (c != 0)
									{
										if (alphatable[c] == 0xff)
											dest[x] = pal[c];
										else
											dest[x] = alpha_blend_r32(dest[x], pal[c], alphatable[c]);
									}
									x_index += dx;
								}

								y_index += dy;
							}
						}
					}
				}
			}
		}
	}
	profiler_mark_end();
}


static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, UINT8 req_pri)
{
	/*- Sprite Format 0x0000 - 0x37ff -**

    0 ---- --yy yyyy yyyy | ---- --xx xxxx xxxx  1  F-?? hhhh ZZZZ ZZZZ | f-PP wwww zzzz zzzz
    2 pppp pppp -aaa -nnn | nnnn nnnn nnnn nnnn  3  ---- ---- ---- ---- | ---- ---- ---- ----

    y = ypos
    x = xpos

    h = height
    w = width

    F = flip (y)
    f = flip (x)

    Z = zoom (y)
    z = zoom (x)

    n = tile number

    p = palette

    a = alpha blending, selects which of the 8 alpha values in vid_regs[0-1] to use

    P = priority
    Points to a 4-bit entry in vid_regs[2] which provides a priority comparable with the bg layer's priorities.
    However, sprite-sprite priority needs to be preserved.
    daraku and soldivid only use the lsb

    ? = unknown
    Could be a sprite-sprite priority, tests seem to back this up

    **- End Sprite Format -*/
	const int spr_keys[8] = {KEYCODE_Y, KEYCODE_U, KEYCODE_I, KEYCODE_O};
	bool spr_debug = false;
#ifdef DEBUG_KEYS
	for (int i = 0; i <= 3; i++)
	{
		if(input_code_pressed(machine, spr_keys[i])) {
			spr_debug = true;
		}
	}
#endif


	psikyosh_state *state = (psikyosh_state *)machine->driver_data;
	const gfx_element *gfx;
	UINT32 *src = machine->generic.buffered_spriteram.u32; /* Use buffered spriteram */
	UINT16 *list = (UINT16 *)src + 0x3800 / 2;
	UINT16 listlen = 0x800/2;
	UINT16 *zoom_table = (UINT16 *)state->zoomram;
	UINT8  *alpha_table = (UINT8 *)&(state->vidregs[0]);

	UINT16 listcntr = 0;
	while (listcntr < listlen)
	{
		UINT32 listdat, sprnum, xpos, ypos, high, wide, flpx, flpy, zoomx, zoomy, tnum, colr, dpth;
		UINT8 bg_pri, spr_pri, alphamap;
		int alpha;

		listdat = list[BYTE_XOR_BE(listcntr)];
		sprnum = (listdat & 0x03ff) * 4;

		bg_pri  = (src[sprnum + 1] & 0x00003000) >> 12;
		bg_pri = SPRITE_PRI(bg_pri);

		// sprite vs backgrounds pri
		if (bg_pri == req_pri)
		{
			ypos = (src[sprnum + 0] & 0x03ff0000) >> 16;
			xpos = (src[sprnum + 0] & 0x000003ff) >> 00;

			if (ypos & 0x200) ypos -= 0x400;
			if (xpos & 0x200) xpos -= 0x400;

			high  = ((src[sprnum + 1] & 0x0f000000) >> 24) + 1;
			wide  = ((src[sprnum + 1] & 0x00000f00) >> 8) + 1;

			flpy  = (src[sprnum + 1] & 0x80000000) >> 31;
			spr_pri = (src[sprnum + 1] & 0x30000000) >> 28;
			flpx  = (src[sprnum + 1] & 0x00008000) >> 15;

			zoomy = (src[sprnum + 1] & 0x00ff0000) >> 16;
			zoomx = (src[sprnum + 1] & 0x000000ff) >> 00;

			tnum  = (src[sprnum + 2] & 0x0007ffff) >> 00;
			dpth  = (src[sprnum + 2] & 0x00800000) >> 23;
			colr  = (src[sprnum + 2] & 0xff000000) >> 24;

			alpha = (src[sprnum + 2] & 0x00700000) >> 20;

			alphamap = (alpha_table[BYTE4_XOR_BE(alpha)] & 0x80)? 1:0;
			alpha = alpha_table[BYTE4_XOR_BE(alpha)] & 0x3f;

			gfx = dpth ? machine->gfx[1] : machine->gfx[0];

			if (alphamap) /* alpha values are per-pen */
				alpha = -1;
			else
				alpha = pal6bit(0x3f - alpha); /* 0x3f-0x00 maps to 0x00-0xff */

			if(!spr_debug || input_code_pressed(machine, spr_keys[spr_pri]))
			{
				/* start drawing */
				if (zoom_table[BYTE_XOR_BE(zoomy)] && zoom_table[BYTE_XOR_BE(zoomx)]) /* Avoid division-by-zero when table contains 0 (Uninitialised/Bug) */
				{
					psikyosh_drawgfxzoom(machine, bitmap, cliprect, gfx, tnum, colr, flpx, flpy, xpos, ypos, alpha,
										(UINT32)zoom_table[BYTE_XOR_BE(zoomx)], (UINT32)zoom_table[BYTE_XOR_BE(zoomy)], wide, high, listcntr);
				}
				/* end drawing */
			}

		}
		listcntr++;
		if (listdat & 0x4000) break;
	}
}


static void psikyosh_prelineblend( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	/* There are 224 values for pre-lineblending. Using one for every row currently */
	/* I suspect that it should be blended against black by the amount specified as
       gnbarich sets the 0x000000ff to 0x7f in test mode whilst the others use 0x80.
       tgm2 sets it to 0x00 on warning screen. Likely has no effect. */
	psikyosh_state *state = (psikyosh_state *)machine->driver_data;
	UINT32 *dstline;
	int bank = (state->vidregs[7] & 0xff000000) >> 24; /* bank is always 8 (0x4000) except for daraku/soldivid */
	UINT32 *linefill = &state->bgram[(bank * 0x800) / 4 - 0x4000 / 4]; /* Per row */
	int x, y;

	assert(bitmap->bpp == 32);

	profiler_mark_start(PROFILER_USER8);
	for (y = cliprect->min_y; y <= cliprect->max_y; y += 1) {

		dstline = BITMAP_ADDR32(bitmap, y, 0);

		/* linefill[y] & 0xff does what? */
		for (x = cliprect->min_x; x <= cliprect->max_x; x += 1)
			dstline[x] = linefill[y] >> 8;
	}
	profiler_mark_end();
}


static void psikyosh_postlineblend( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, UINT8 req_pri )
{
	/* There are 224 values for post-lineblending. Using one for every row currently */
	psikyosh_state *state = (psikyosh_state *)machine->driver_data;
	UINT32 *dstline;
	int bank = (state->vidregs[7] & 0xff000000) >> 24; /* bank is always 8 (i.e. 0x4000) except for daraku/soldivid */
	UINT32 *lineblend = &state->bgram[(bank * 0x800) / 4 - 0x4000 / 4 + 0x400 / 4]; /* Per row */
	int x, y;

	assert(bitmap->bpp == 32);

	if ((state->vidregs[2] & 0xf) != req_pri) {
		return;
	}

	profiler_mark_start(PROFILER_USER8);
	for (y = cliprect->min_y; y <= cliprect->max_y; y += 1) {

		dstline = BITMAP_ADDR32(bitmap, y, 0);

		if (lineblend[y] & 0x80) /* solid */
		{
			for (x = cliprect->min_x; x <= cliprect->max_x; x += 1)
				dstline[x] = lineblend[y] >> 8;
		}
		else if (lineblend[y] & 0x7f) /* blended */
		{
			for (x = cliprect->min_x; x <= cliprect->max_x; x += 1)
				dstline[x] = alpha_blend_r32(dstline[x], lineblend[y] >> 8, 2 * (lineblend[y] & 0x7f));
		}
	}
	profiler_mark_end();
}


VIDEO_START( psikyosh )
{
	psikyosh_state *state = (psikyosh_state *)machine->driver_data;
	int width = machine->primary_screen->width();
	int height = machine->primary_screen->height();

	state->z_bitmap = auto_bitmap_alloc(machine, width, height, BITMAP_FORMAT_INDEXED16); /* z-buffer */
	state->zoom_bitmap = auto_bitmap_alloc(machine, 16*16, 16*16, BITMAP_FORMAT_INDEXED8); /* temp buffer for assembling sprites */
	state->bg_bitmap = auto_bitmap_alloc(machine, 32*16, 32*16, BITMAP_FORMAT_RGB32); /* temp buffer for assembling tilemaps */
	state->bg_zoom = auto_alloc_array(machine, UINT16, 256);

	machine->gfx[1]->color_granularity = 16; /* 256 colour sprites with palette selectable on 16 colour boundaries */

	/* Pens 0xc0-0xff have a gradient of alpha values associated with them */
	int i;
	for (i = 0; i < 0xc0; i++) {
		alphatable[i] = 0xff;
	}
	for (i = 0; i < 0x40; i++)
	{
		int alpha = pal6bit(0x3f - i);
		alphatable[i + 0xc0] = alpha;
	}

	/* precompute the background zoom table. verified against hardware.
       unsure of the precision, we use .10 fixed point like the sprites */
	for(i = 0; i < 0x100; i++) {
		state->bg_zoom[i] = (64 * 0x400) / (i + 64);
	}

	state_save_register_global_bitmap(machine, state->z_bitmap);
	state_save_register_global_bitmap(machine, state->zoom_bitmap);
	state_save_register_global_bitmap(machine, state->bg_bitmap);
	state_save_register_global_pointer(machine, state->bg_zoom, 256);
}


VIDEO_UPDATE( psikyosh ) /* Note the z-buffer on each sprite to get correct priority */
{
	int i;
	psikyosh_state *state = (psikyosh_state *)screen->machine->driver_data;

	// show only the priority associated with a given keypress(s) and/or hide sprites/tilemaps
	int pri_debug = false;
	int sprites = true;
	int backgrounds = true;
	const int pri_keys[8] = {KEYCODE_Z, KEYCODE_X, KEYCODE_C, KEYCODE_V, KEYCODE_B, KEYCODE_N, KEYCODE_M, KEYCODE_K};
#ifdef DEBUG_KEYS
	for (i = 0; i <= 7; i++)
	{
		if(input_code_pressed(screen->machine, pri_keys[i])) {
			pri_debug = true;
		}
	}
	if(input_code_pressed(screen->machine, KEYCODE_G)) {
		sprites = false;
	}
	if(input_code_pressed(screen->machine, KEYCODE_H)) {
		backgrounds = false;
	}
#endif

#ifdef DEBUG_MESSAGE
popmessage   ("%08x %08x %08x %08x\n%08x %08x %08x %08x",
    state->vidregs[0], state->vidregs[1],
    state->vidregs[2], state->vidregs[3],
    state->vidregs[4], state->vidregs[5],
    state->vidregs[6], state->vidregs[7]);
#endif

	bitmap_fill(state->z_bitmap, cliprect, 0); /* z-buffer */

	psikyosh_prelineblend(screen->machine, bitmap, cliprect); // fills screen
	for (i = 0; i <= 7; i++)
	{
		if(!pri_debug || input_code_pressed(screen->machine, pri_keys[i]))
		{
			if(sprites) {
				draw_sprites(screen->machine, bitmap, cliprect, i); // When same priority bg's have higher pri
			}
			if(backgrounds) {
				draw_background(screen->machine, bitmap, cliprect, i);
			}
			psikyosh_postlineblend(screen->machine, bitmap, cliprect, i); // assume this has highest priority at same priority level
		}
	}
	return 0;
}

VIDEO_EOF( psikyosh )
{
	const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);
	buffer_spriteram32_w(space, 0, 0, 0xffffffff);
}

