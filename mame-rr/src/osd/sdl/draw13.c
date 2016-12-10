//============================================================
//
//  draw13.c - SDL 1.3 drawing implementation
//
//  Copyright (c) 1996-2010, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//  Note: D3D9 goes to a lot of trouble to fiddle with MODULATE
//        mode on textures.  That is the default in OpenGL so we
//        don't have to touch it.
//
//============================================================

// standard C headers
#include <math.h>
#include <stdio.h>

// MAME headers
#include "emu.h"
#include "options.h"

// standard SDL headers
#include <SDL/SDL.h>

// OSD headers
#include "osdsdl.h"
#include "window.h"

//============================================================
//  DEBUGGING
//============================================================

//============================================================
//  CONSTANTS
//============================================================

#define STAT_PIXEL_THRESHOLD (150*150)

enum
{
	TEXTURE_TYPE_NONE,
	TEXTURE_TYPE_PLAIN,
	TEXTURE_TYPE_SURFACE
};


//============================================================
//  MACROS
//============================================================

#define IS_OPAQUE(a)		(a >= 1.0f)
#define IS_TRANSPARENT(a)	(a <  0.0001f)

#define MAX4(a, b, c, d) MAX(a, MAX(b, MAX(c, d)))
#define MIN4(a, b, c, d) MIN(a, MIN(b, MIN(c, d)))


//============================================================
//  TYPES
//============================================================


typedef struct _quad_setup_data quad_setup_data;
struct _quad_setup_data
{
	INT32			dudx, dvdx, dudy, dvdy;
	INT32			startu, startv;
	INT32			rotwidth, rotheight;
};

typedef struct _texture_info texture_info;

typedef void (*texture_copy_func)(texture_info *texture, const render_texinfo *texsource);

typedef struct _copy_info copy_info;
struct _copy_info {
	int 				src_fmt;
	Uint32				dst_fmt;
	int					dst_bpp;
	int					rotate;
	texture_copy_func	func;
	Uint32				bm_mask;
	const char			*srcname;
	const char			*dstname;
	/* Statistics */
	UINT64				pixel_count;
	INT64				time;
	int					samples;
	int					perf;
	/* list */
	copy_info			*next;
};

/* texture_info holds information about a texture */
struct _texture_info
{
	texture_info *		next;				// next texture in the list

	HashT				hash;				// hash value for the texture (must be >= pointer size)
	UINT32				flags;				// rendering flags
	render_texinfo		texinfo;			// copy of the texture info

	int					rawwidth, rawheight;// raw width/height of the texture

	int					format;				// texture format
	void				*pixels;			// pixels for the texture
	int					pitch;
	int					pixels_own;			// do we own / allocated it ?

	SDL_TextureID		texture_id;

	copy_info			*copyinfo;
	Uint32				sdl_access;
	Uint32				sdl_blendmode;
	quad_setup_data		setup;
	int					is_rotated;

	osd_ticks_t			last_access;
};

/* sdl_info is the information about SDL for the current screen */
typedef struct _sdl_info sdl_info;
struct _sdl_info
{
	INT32			blittimer;
	UINT32			extra_flags;

	texture_info *	texlist;				// list of active textures
	INT32			texture_max_width;  	// texture maximum width
	INT32			texture_max_height; 	// texture maximum height

	float			last_hofs;
	float			last_vofs;

	// Stats
	INT64			last_blit_time;
	INT64			last_blit_pixels;
};

//============================================================
//  PROTOTYPES
//============================================================

// core functions

static void draw13_exit(void);
static void draw13_attach(sdl_draw_info *info, sdl_window_info *window);
static int draw13_window_create(sdl_window_info *window, int width, int height);
static void draw13_window_resize(sdl_window_info *window, int width, int height);
static void draw13_window_destroy(sdl_window_info *window);
static int draw13_window_draw(sdl_window_info *window, UINT32 dc, int update);
static const render_primitive_list *draw13_window_get_primitives(sdl_window_info *window);
static void draw13_destroy_all_textures(sdl_window_info *window);
static void draw13_window_clear(sdl_window_info *window);
static int draw13_xy_to_render_target(sdl_window_info *window, int x, int y, int *xt, int *yt);
static void draw13_destroy_texture(sdl_info *sdl, texture_info *texture);

//============================================================
//  Textures
//============================================================

static void texture_set_data(sdl_info *sdl, texture_info *texture, const render_texinfo *texsource, UINT32 flags);
static texture_info *texture_create(sdl_window_info *window, const render_texinfo *texsource, quad_setup_data *setup, UINT32 flags);
static texture_info *texture_find(sdl_info *sdl, const render_primitive *prim, quad_setup_data *setup);
static texture_info * texture_update(sdl_window_info *window, const render_primitive *prim);


//============================================================
//  TEXCOPY FUNCS
//============================================================

#include "blit13.h"

//============================================================
//  STATIC VARIABLES
//============================================================

#define SDL_TEXFORMAT_LAST SDL_TEXFORMAT_PALETTE16A
#define BM_ALL (-1)
//( SDL_BLENDMODE_MASK | SDL_BLENDMODE_BLEND | SDL_BLENDMODE_ADD | SDL_BLENDMODE_MOD)

#define texcopy_NULL NULL
#define ENTRY(a,b,c,d,f) { SDL_TEXFORMAT_ ## a, SDL_PIXELFORMAT_ ## b, c, d, texcopy_ ## f, BM_ALL, #a, #b, 0, 0, 0, 0}
#define ENTRY_BM(a,b,c,d,f,bm) { SDL_TEXFORMAT_ ## a, SDL_PIXELFORMAT_ ## b, c, d, texcopy_ ## f, bm, #a, #b, 0, 0, 0, 0}
#define ENTRY_LR(a,b,c,d,f) { SDL_TEXFORMAT_ ## a, SDL_PIXELFORMAT_ ## b, c, d, texcopy_ ## f, BM_ALL, #a, #b, 0, 0, 0, -1}

static copy_info blit_info_default[] =
{
	/* no rotation */
	ENTRY(ARGB32,			ARGB8888,	4, 0, NULL),
	ENTRY_LR(ARGB32,		RGB888, 	4, 0, argb32_rgb32),
	/* Entry for primarily for directfb */
	ENTRY_BM(ARGB32,		RGB888, 	4, 0, argb32_rgb32, SDL_BLENDMODE_ADD),
	ENTRY_BM(ARGB32,		RGB888, 	4, 0, argb32_rgb32, SDL_BLENDMODE_MOD),
	ENTRY_BM(ARGB32,		RGB888, 	4, 0, argb32_rgb32, SDL_BLENDMODE_NONE),

	ENTRY(RGB32,			ARGB8888,	4, 0, rgb32_argb32),
	ENTRY(RGB32,			RGB888, 	4, 0, NULL),

	ENTRY(RGB32_PALETTED,	ARGB8888,	4, 0, rgb32pal_argb32),
	ENTRY(RGB32_PALETTED,	RGB888, 	4, 0, rgb32pal_argb32),

	ENTRY(YUY16,			UYVY,		2, 0, NULL /* yuv16_uyvy*/),
	ENTRY(YUY16,			YUY2,		2, 0, yuv16_yuy2),
	ENTRY(YUY16,			YVYU,		2, 0, yuv16_yvyu),
	ENTRY(YUY16,			ARGB8888,	4, 0, yuv16_argb32),
	ENTRY(YUY16,			RGB888,		4, 0, yuv16pal_argb32),

	ENTRY(YUY16_PALETTED,	UYVY,		2, 0, yuv16pal_uyvy),
	ENTRY(YUY16_PALETTED,	YUY2,		2, 0, yuv16pal_yuy2),
	ENTRY(YUY16_PALETTED,	YVYU,		2, 0, yuv16pal_yvyu),
	ENTRY(YUY16_PALETTED,	ARGB8888,	4, 0, yuv16pal_argb32),
	ENTRY(YUY16_PALETTED,	RGB888, 	4, 0, yuv16pal_argb32),

	ENTRY(PALETTE16,		ARGB8888,	4, 0, pal16_argb32),
	ENTRY(PALETTE16,		RGB888, 	4, 0, pal16_argb32),

	ENTRY(RGB15,			RGB555, 	2, 0, NULL /* rgb15_argb1555 */),
	ENTRY(RGB15,			ARGB1555,	2, 0, rgb15_argb1555),
	ENTRY(RGB15,			ARGB8888,	4, 0, rgb15_argb32),
	ENTRY(RGB15,			RGB888, 	4, 0, rgb15_argb32),

	ENTRY(RGB15_PALETTED,	ARGB8888,	4, 0, rgb15pal_argb32),
	ENTRY(RGB15_PALETTED,	RGB888, 	4, 0, rgb15pal_argb32),

	ENTRY(PALETTE16A,		ARGB8888,	4, 0, pal16a_argb32),
	ENTRY(PALETTE16A,		RGB888, 	4, 0, pal16a_rgb32),

	/* rotation */
	ENTRY(ARGB32,			ARGB8888,	4, 1, rot_argb32_argb32),
	ENTRY_LR(ARGB32,		RGB888, 	4, 1, rot_argb32_rgb32),
	/* Entry for primarily for directfb */
	ENTRY_BM(ARGB32,		RGB888, 	4, 1, rot_argb32_rgb32, SDL_BLENDMODE_ADD),
	ENTRY_BM(ARGB32,		RGB888, 	4, 1, rot_argb32_rgb32, SDL_BLENDMODE_MOD),
	ENTRY_BM(ARGB32,		RGB888, 	4, 1, rot_argb32_rgb32, SDL_BLENDMODE_NONE),

	ENTRY(RGB32,			ARGB8888,	4, 1, rot_rgb32_argb32),
	ENTRY(RGB32,			RGB888, 	4, 1, rot_argb32_argb32),

	ENTRY(RGB32_PALETTED,	ARGB8888,	4, 1, rot_rgb32pal_argb32),
	ENTRY(RGB32_PALETTED,	RGB888, 	4, 1, rot_rgb32pal_argb32),

	ENTRY(YUY16,			ARGB8888,	4, 1, rot_yuv16_argb32),
	ENTRY(YUY16,			RGB888,		4, 1, rot_yuv16_argb32),

	ENTRY(YUY16_PALETTED,	ARGB8888,	4, 1, rot_yuv16pal_argb32),
	ENTRY(YUY16_PALETTED,	RGB888,		4, 1, rot_yuv16pal_argb32),

	ENTRY(PALETTE16,		ARGB8888,	4, 1, rot_pal16_argb32),
	ENTRY(PALETTE16,		RGB888, 	4, 1, rot_pal16_argb32),

	ENTRY(RGB15,			RGB555, 	2, 1, rot_rgb15_argb1555),
	ENTRY(RGB15,			ARGB1555,	2, 1, rot_rgb15_argb1555),
	ENTRY(RGB15,			ARGB8888,	4, 1, rot_rgb15_argb32),
	ENTRY(RGB15,			RGB888, 	4, 1, rot_rgb15_argb32),

	ENTRY(RGB15_PALETTED,	ARGB8888,	4, 1, rot_rgb15pal_argb32),
	ENTRY(RGB15_PALETTED,	RGB888, 	4, 1, rot_rgb15pal_argb32),

	ENTRY(PALETTE16A,		ARGB8888,	4, 1, rot_pal16a_argb32),
	ENTRY(PALETTE16A,		RGB888, 	4, 1, rot_pal16a_rgb32),

{ -1 },
};

static copy_info blit_info_16bpp[] =
{
	/* no rotation */
	ENTRY(PALETTE16,		RGB555, 	2, 0, pal16_argb1555),
	ENTRY(PALETTE16,		ARGB1555,	2, 0, pal16_argb1555),

	ENTRY(RGB15_PALETTED,	RGB555, 	2, 0, rgb15pal_argb1555),
	ENTRY(RGB15_PALETTED,	ARGB1555,	2, 0, rgb15pal_argb1555),

	/* rotation */
	ENTRY(PALETTE16,		RGB555, 	2, 1, rot_pal16_argb1555),
	ENTRY(PALETTE16,		ARGB1555,	2, 1, rot_pal16_argb1555),

	ENTRY(RGB15_PALETTED,	RGB555, 	2, 1, rot_rgb15pal_argb1555),
	ENTRY(RGB15_PALETTED,	ARGB1555,	2, 1, rot_rgb15pal_argb1555),

{ -1 },
};

static copy_info *blit_info[SDL_TEXFORMAT_LAST+1];

static struct
{
	Uint32	format;
	int		status;
} fmt_support[30] = { { 0, 0 } };


//============================================================
//  INLINES
//============================================================


INLINE float round_nearest(float f)
{
	return floor(f + 0.5f);
}

INLINE HashT texture_compute_hash(const render_texinfo *texture, UINT32 flags)
{
	return (HashT)texture->base ^ (flags & (PRIMFLAG_BLENDMODE_MASK | PRIMFLAG_TEXFORMAT_MASK));
}

INLINE Uint32 map_blendmode(int blendmode)
{
	switch (blendmode)
	{
		case BLENDMODE_NONE:
			return SDL_BLENDMODE_NONE;
		case BLENDMODE_ALPHA:
			return SDL_BLENDMODE_BLEND;
		case BLENDMODE_RGB_MULTIPLY:
			return SDL_BLENDMODE_MOD;
		case BLENDMODE_ADD:
			return SDL_BLENDMODE_ADD;
		default:
			mame_printf_warning("Unknown Blendmode %d", blendmode);
	}
	return SDL_BLENDMODE_NONE;
}

INLINE void set_coloralphamode(SDL_TextureID texture_id, const render_color *color)
{
	UINT32 sr = (UINT32)(255.0f * color->r);
	UINT32 sg = (UINT32)(255.0f * color->g);
	UINT32 sb = (UINT32)(255.0f * color->b);
	UINT32 sa = (UINT32)(255.0f * color->a);


	if (color->r >= 1.0f && color->g >= 1.0f && color->b >= 1.0f && IS_OPAQUE(color->a))
	{
		SDL_SetTextureColorMod(texture_id, 0xFF, 0xFF, 0xFF);
		SDL_SetTextureAlphaMod(texture_id, 0xFF);
	}
	/* coloring-only case */
	else if (IS_OPAQUE(color->a))
	{
		SDL_SetTextureColorMod(texture_id, sr, sg, sb);
		SDL_SetTextureAlphaMod(texture_id, 0xFF);
	}
	/* alpha and/or coloring case */
	else if (!IS_TRANSPARENT(color->a))
	{
		SDL_SetTextureColorMod(texture_id, sr, sg, sb);
		SDL_SetTextureAlphaMod(texture_id, sa);
	}
	else
	{
		SDL_SetTextureColorMod(texture_id, 0xFF, 0xFF, 0xFF);
		SDL_SetTextureAlphaMod(texture_id, 0xFF);
	}
}

INLINE void render_quad(sdl_info *sdl, texture_info *texture, render_primitive *prim, int x, int y)
{
	SDL_TextureID texture_id;
	SDL_Rect target_rect;

	target_rect.x = x;
	target_rect.y = y;
	target_rect.w = round_nearest(prim->bounds.x1 - prim->bounds.x0);
	target_rect.h = round_nearest(prim->bounds.y1 - prim->bounds.y0);

	if (texture)
	{
		texture_id = texture->texture_id;

		texture->copyinfo->time -= osd_ticks();
		if ((PRIMFLAG_GET_SCREENTEX(prim->flags)) && video_config.filter)
		{
			SDL_SetTextureScaleMode(texture->texture_id,  SDL_TEXTURESCALEMODE_BEST);
		}
		else
		{
			SDL_SetTextureScaleMode(texture->texture_id,  SDL_TEXTURESCALEMODE_FAST);
		}
		SDL_SetTextureBlendMode(texture_id, texture->sdl_blendmode);
		set_coloralphamode(texture_id, &prim->color);
		SDL_RenderCopy(texture_id, NULL, &target_rect);
		texture->copyinfo->time += osd_ticks();

		texture->copyinfo->pixel_count += MAX(STAT_PIXEL_THRESHOLD , (texture->rawwidth * texture->rawheight));
		if (sdl->last_blit_pixels)
		{
			texture->copyinfo->time += (sdl->last_blit_time * (INT64) (texture->rawwidth * texture->rawheight)) / (INT64) sdl->last_blit_pixels;
		}
		texture->copyinfo->samples++;
		texture->copyinfo->perf = ( texture->copyinfo->pixel_count * (osd_ticks_per_second()/1000)) / texture->copyinfo->time;
	}
	else
	{
		UINT32 sr = (UINT32)(255.0f * prim->color.r);
		UINT32 sg = (UINT32)(255.0f * prim->color.g);
		UINT32 sb = (UINT32)(255.0f * prim->color.b);
		UINT32 sa = (UINT32)(255.0f * prim->color.a);

		SDL_SetRenderDrawBlendMode(map_blendmode(PRIMFLAG_GET_BLENDMODE(prim->flags)));
		SDL_SetRenderDrawColor(sr, sg, sb, sa);
		SDL_RenderFillRect(&target_rect);
	}
}

#if 0
static int RendererSupportsFormat(Uint32 format, Uint32 access, const char *sformat)
{
    struct SDL_RendererInfo render_info;
	int i;

    SDL_GetRendererInfo(&render_info);

	for (i=0; i < render_info.num_texture_formats; i++)
	{
		if (format == render_info.texture_formats[i])
			return 1;
	}
	mame_printf_verbose("Pixelformat <%s> not supported\n", sformat);
	return 0;
}
#else
static int RendererSupportsFormat(Uint32 format, Uint32 access, const char *sformat)
{
	int i;
	SDL_TextureID texid;
	for (i=0; fmt_support[i].format != 0; i++)
	{
		if (format == fmt_support[i].format)
		{
			return fmt_support[i].status;
		}
	}
	/* not tested yet */
	fmt_support[i].format = format;
	fmt_support[i + 1].format = 0;
	texid = SDL_CreateTexture(format, access, 16, 16);
	if (texid)
	{
		fmt_support[i].status = 1;
		SDL_DestroyTexture(texid);
		return 1;
	}
	mame_printf_verbose("Pixelformat <%s> error %s \n", sformat, SDL_GetError());
	mame_printf_verbose("Pixelformat <%s> not supported\n", sformat);
	fmt_support[i].status = 0;
	return 0;
}
#endif

//============================================================
//  draw13_init
//============================================================

static void add_list(copy_info **head, copy_info *element, Uint32 bm)
{
	copy_info *newci = (copy_info *) osd_malloc(sizeof(copy_info));
	*newci = *element;

	newci->bm_mask = bm;
	newci->next = *head;
	*head = newci;
}

static void expand_copy_info(copy_info *list)
{
	copy_info	*bi;

	for (bi = list; bi->src_fmt != -1; bi++)
	{
		if (bi->bm_mask == BM_ALL)
		{
			add_list(&blit_info[bi->src_fmt], bi, SDL_BLENDMODE_NONE);
			add_list(&blit_info[bi->src_fmt], bi, SDL_BLENDMODE_ADD);
			add_list(&blit_info[bi->src_fmt], bi, SDL_BLENDMODE_MOD);
			add_list(&blit_info[bi->src_fmt], bi, SDL_BLENDMODE_BLEND);
		}
		else
			add_list(&blit_info[bi->src_fmt], bi, bi->bm_mask);
	}
}

int draw13_init(sdl_draw_info *callbacks)
{
	const char *stemp;

	// fill in the callbacks
	callbacks->exit = draw13_exit;
	callbacks->attach = draw13_attach;

	mame_printf_verbose("Using SDL native texturing driver (SDL 1.3+)\n");

	expand_copy_info(blit_info_default);
	//FIXME: -opengl16 should be -opengl -prefer16bpp
	//if (video_config.prefer16bpp_tex)
	expand_copy_info(blit_info_16bpp);

	// Load the GL library now - else MT will fail

	stemp = options_get_string(mame_options(), SDLOPTION_GL_LIB);
	if (stemp != NULL && strcmp(stemp, SDLOPTVAL_AUTO) == 0)
		stemp = NULL;

	// No fatalerror here since not all video drivers support GL !
	if (SDL_GL_LoadLibrary(stemp) != 0) // Load library (default for e==NULL
		mame_printf_verbose("Warning: Unable to load opengl library: %s\n", stemp ? stemp : "<default>");
	else
		mame_printf_verbose("Loaded opengl shared library: %s\n", stemp ? stemp : "<default>");

	return 0;
}


//============================================================
//  draw13_exit
//============================================================

static void draw13_exit(void)
{
	int i;
	copy_info *bi, *freeme;
	for (i = 0; i <= SDL_TEXFORMAT_LAST; i++)
		for (bi = blit_info[i]; bi != NULL; )
		{
			if (bi->pixel_count)
				mame_printf_verbose("%s -> %s %s blendmode 0x%02x, %d samples: %d KPixel/sec\n", bi->srcname, bi->dstname,
						bi->rotate ? "rot" : "norot", bi->bm_mask, bi->samples,
						(int) bi->perf);
			freeme = bi;
			bi = bi->next;
			osd_free(freeme);
		}
}

//============================================================
//  draw13_attach
//============================================================

static void draw13_attach(sdl_draw_info *info, sdl_window_info *window)
{
	// fill in the callbacks
	window->create = draw13_window_create;
	window->resize = draw13_window_resize;
	window->get_primitives = draw13_window_get_primitives;
	window->draw = draw13_window_draw;
	window->destroy = draw13_window_destroy;
	window->destroy_all_textures = draw13_destroy_all_textures;
	window->clear = draw13_window_clear;
	window->xy_to_render_target = draw13_xy_to_render_target;
}

//============================================================
//  draw13_window_create
//============================================================

static int draw13_window_create(sdl_window_info *window, int width, int height)
{
	// allocate memory for our structures
	sdl_info *sdl = (sdl_info *) osd_malloc(sizeof(*sdl));
	int result;

	mame_printf_verbose("Enter draw13_window_create\n");

	memset(sdl, 0, sizeof(*sdl));

	window->dxdata = sdl;

	sdl->extra_flags = (window->fullscreen ?
			SDL_WINDOW_BORDERLESS | SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_FULLSCREEN: SDL_WINDOW_RESIZABLE);

	// create the SDL window
	SDL_SelectVideoDisplay(window->monitor->handle);

	if (window->fullscreen && video_config.switchres)
	{
		SDL_DisplayMode mode;
		SDL_GetCurrentDisplayMode(&mode);
		mode.w = width;
		mode.h = height;
		if (window->refresh)
			mode.refresh_rate = window->refresh;
		if (window->depth)
		{
			switch (window->depth)
			{
			case 15:
				mode.format = SDL_PIXELFORMAT_RGB555;
				break;
			case 16:
				mode.format = SDL_PIXELFORMAT_RGB565;
				break;
			case 24:
				mode.format = SDL_PIXELFORMAT_RGB24;
				break;
			case 32:
				mode.format = SDL_PIXELFORMAT_RGB888;
				break;
			default:
				mame_printf_warning("Ignoring depth %d\n", window->depth);
			}
		}
		SDL_SetWindowDisplayMode(window->sdl_window, &mode);	// Try to set mode
	}
	else
		SDL_SetWindowDisplayMode(window->sdl_window, NULL);	// Use desktop

	window->sdl_window = SDL_CreateWindow(window->title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			width, height, sdl->extra_flags);

	SDL_ShowWindow(window->sdl_window);
	//SDL_SetWindowFullscreen(window->window_id, window->fullscreen);
	SDL_RaiseWindow(window->sdl_window);
	SDL_GetWindowSize(window->sdl_window, &window->width, &window->height);

	// create renderer

	if (video_config.waitvsync)
		result = SDL_CreateRenderer(window->sdl_window, -1, SDL_RENDERER_PRESENTFLIP2 | SDL_RENDERER_PRESENTDISCARD | SDL_RENDERER_PRESENTVSYNC);
	else
		result = SDL_CreateRenderer(window->sdl_window, -1, SDL_RENDERER_PRESENTFLIP2 | SDL_RENDERER_PRESENTDISCARD);

	if (result)
	{
		fatalerror("Error on creating renderer: %s \n", SDL_GetError());
	}

    SDL_SelectRenderer(window->sdl_window);

	sdl->blittimer = 3;

	// in case any textures try to come up before these are validated,
	// OpenGL guarantees all implementations can handle something this size.
	sdl->texture_max_width = 64;
	sdl->texture_max_height = 64;

	mame_printf_verbose("Leave draw13_window_create\n");
	return 0;
}

//============================================================
//  draw13_window_resize
//============================================================

static void draw13_window_resize(sdl_window_info *window, int width, int height)
{
	sdl_info *sdl = (sdl_info *) window->dxdata;

	SDL_SetWindowSize(window->sdl_window, width, height);
	SDL_GetWindowSize(window->sdl_window, &window->width, &window->height);
	sdl->blittimer = 3;

}

//============================================================
//  drawsdl_xy_to_render_target
//============================================================

static int draw13_xy_to_render_target(sdl_window_info *window, int x, int y, int *xt, int *yt)
{
	sdl_info *sdl = (sdl_info *) window->dxdata;

	*xt = x - sdl->last_hofs;
	*yt = y - sdl->last_vofs;
	if (*xt<0 || *xt >= window->blitwidth)
		return 0;
	if (*yt<0 || *yt >= window->blitheight)
		return 0;
	return 1;
}

//============================================================
//  draw13_window_get_primitives
//============================================================

static const render_primitive_list *draw13_window_get_primitives(sdl_window_info *window)
{
	if ((!window->fullscreen) || (video_config.switchres))
	{
		sdlwindow_blit_surface_size(window, window->width, window->height);
	}
	else
	{
		sdlwindow_blit_surface_size(window, window->monitor->center_width, window->monitor->center_height);
	}
	render_target_set_bounds(window->target, window->blitwidth, window->blitheight, sdlvideo_monitor_get_aspect(window->monitor));
	return render_target_get_primitives(window->target);
}

//============================================================
//  draw13_window_draw
//============================================================

static int draw13_window_draw(sdl_window_info *window, UINT32 dc, int update)
{
	sdl_info *sdl = (sdl_info *) window->dxdata;
	render_primitive *prim;
	texture_info *texture=NULL;
	float vofs, hofs;
	int blit_pixels = 0;

	if (video_config.novideo)
	{
		return 0;
	}

	SDL_SelectRenderer(window->sdl_window);

	if (sdl->blittimer > 0)
	{
		/* SDL Underlays need alpha = 0 ! */
		SDL_SetRenderDrawBlendMode(SDL_BLENDMODE_NONE);
		SDL_SetRenderDrawColor(0,0,0,0 /*255*/);
		SDL_RenderFillRect(NULL);
		sdl->blittimer--;
	}

	// compute centering parameters
	vofs = hofs = 0.0f;

	if (video_config.centerv || video_config.centerh)
	{
		int ch, cw;

		if ((window->fullscreen) && (!video_config.switchres))
		{
			ch = window->monitor->center_height;
			cw = window->monitor->center_width;
		}
		else
		{
			ch = window->height;
			cw = window->width;
		}

		if (video_config.centerv)
		{
			vofs = (ch - window->blitheight) / 2.0f;
		}
		if (video_config.centerh)
		{
			hofs = (cw - window->blitwidth) / 2.0f;
		}
	}

	sdl->last_hofs = hofs;
	sdl->last_vofs = vofs;

	osd_lock_acquire(window->primlist->lock);

	// now draw
	for (prim = window->primlist->head; prim != NULL; prim = prim->next)
	{
		Uint8 sr, sg, sb, sa;

		switch (prim->type)
		{
			case RENDER_PRIMITIVE_LINE:
				sr = (int)(255.0f * prim->color.r);
				sg = (int)(255.0f * prim->color.g);
				sb = (int)(255.0f * prim->color.b);
				sa = (int)(255.0f * prim->color.a);

				SDL_SetRenderDrawBlendMode(map_blendmode(PRIMFLAG_GET_BLENDMODE(prim->flags)));
				SDL_SetRenderDrawColor(sr, sg, sb, sa);
				SDL_RenderDrawLine(prim->bounds.x0 + hofs, prim->bounds.y0 + vofs,
						prim->bounds.x1 + hofs, prim->bounds.y1 + vofs);
				break;
			case RENDER_PRIMITIVE_QUAD:
				texture = texture_update(window, prim);
				if (texture)
					blit_pixels += (texture->rawheight * texture->rawwidth);
				render_quad(sdl, texture, prim,
						round_nearest(hofs + prim->bounds.x0),
						round_nearest(vofs + prim->bounds.y0));
				break;
		}
	}

	osd_lock_release(window->primlist->lock);

	sdl->last_blit_pixels = blit_pixels;
	sdl->last_blit_time = -osd_ticks();
	SDL_RenderPresent();
	sdl->last_blit_time += osd_ticks();

	return 0;
}


//============================================================
//  draw13_window_clear
//============================================================

static void draw13_window_clear(sdl_window_info *window)
{
	sdl_info *sdl = (sdl_info *) window->dxdata;

	sdl->blittimer = 2;
}


//============================================================
//  draw13_window_destroy
//============================================================

static void draw13_window_destroy(sdl_window_info *window)
{
	sdl_info *sdl = (sdl_info *) window->dxdata;

	// skip if nothing
	if (sdl == NULL)
		return;

	// free the memory in the window

	draw13_destroy_all_textures(window);

	SDL_DestroyWindow(window->sdl_window);

	osd_free(sdl);
	window->dxdata = NULL;
}

//============================================================
//  texture handling
//============================================================

//============================================================
//  texture_compute_size and type
//============================================================

static copy_info *texture_compute_size_type(const render_texinfo *texsource, texture_info *texture, UINT32 flags)
{
	copy_info *bi;
	copy_info *result = NULL;
	int maxperf = 0;
	//int bm = PRIMFLAG_GET_BLENDMODE(flags);

	for (bi = blit_info[texture->format]; bi != NULL; bi = bi->next)
	{
		if ((texture->is_rotated == bi->rotate)
				&& (texture->sdl_blendmode == bi->bm_mask))
		{
			if (RendererSupportsFormat(bi->dst_fmt, texture->sdl_access, bi->dstname))
			{
				if (bi->perf == 0)
					return bi;
				else if (bi->perf > (maxperf * 102) / 100)
				{
					result = bi;
					maxperf = bi->perf;
				}
			}
		}
	}
	if (result)
		return result;
	/* try last resort handlers */
	for (bi = blit_info[texture->format]; bi != NULL; bi = bi->next)
	{
		if ((texture->is_rotated == bi->rotate)
			&& (texture->sdl_blendmode == bi->bm_mask))
			if (RendererSupportsFormat(bi->dst_fmt, texture->sdl_access, bi->dstname))
				return bi;
	}
	//FIXME: crash implement a -do nothing handler */
	return NULL;
}

//============================================================
//  texture_create
//============================================================

static texture_info *texture_create(sdl_window_info *window, const render_texinfo *texsource, quad_setup_data *setup, UINT32 flags)
{
	sdl_info *sdl = (sdl_info *) window->dxdata;
	texture_info *texture;

	// allocate a new texture
	texture = (texture_info *) osd_malloc(sizeof(*texture));
	memset(texture, 0, sizeof(*texture));

	// fill in the core data
	texture->hash = texture_compute_hash(texsource, flags);
	texture->flags = flags;
	texture->texinfo = *texsource;
	texture->texinfo.seqid = -1; // force set data
	texture->is_rotated = FALSE;
	texture->setup = *setup;
	texture->sdl_blendmode = map_blendmode(PRIMFLAG_GET_BLENDMODE(flags));

	switch (PRIMFLAG_GET_TEXFORMAT(flags))
	{
		case TEXFORMAT_ARGB32:
			texture->format = SDL_TEXFORMAT_ARGB32;
			break;
		case TEXFORMAT_RGB32:
            texture->format = texsource->palette ? SDL_TEXFORMAT_RGB32_PALETTED : SDL_TEXFORMAT_RGB32;
			break;
		case TEXFORMAT_PALETTE16:
			texture->format = SDL_TEXFORMAT_PALETTE16;
			break;
		case TEXFORMAT_RGB15:
			texture->format = texsource->palette ? SDL_TEXFORMAT_RGB15_PALETTED : SDL_TEXFORMAT_RGB15;
			break;
		case TEXFORMAT_PALETTEA16:
			texture->format = SDL_TEXFORMAT_PALETTE16A;
			break;
		case TEXFORMAT_YUY16:
			texture->format = texsource->palette ? SDL_TEXFORMAT_YUY16_PALETTED : SDL_TEXFORMAT_YUY16;
			break;

		default:
			mame_printf_error("Unknown textureformat %d\n", PRIMFLAG_GET_TEXFORMAT(flags));
	}

	texture->rawwidth = texsource->width;
	texture->rawheight = texsource->height;
	if (setup->rotwidth != texture->rawwidth || setup->rotheight != texture->rawheight
			|| setup->dudx < 0 )
		texture->is_rotated = TRUE;
	else
		texture->is_rotated = FALSE;

	//texture->sdl_access = SDL_TEXTUREACCESS_STATIC;
	texture->sdl_access = SDL_TEXTUREACCESS_STREAMING;

	// Watch out for 0x0 textures ...
	if (!texture->setup.rotwidth || !texture->setup.rotheight)
		mame_printf_warning("Trying to create texture with zero dim\n");

	// compute the size
	texture->copyinfo = texture_compute_size_type(texsource, texture, flags);

	texture->texture_id = SDL_CreateTexture(texture->copyinfo->dst_fmt, texture->sdl_access,
			texture->setup.rotwidth, texture->setup.rotheight);

	if (!texture->texture_id)
		mame_printf_error("Error creating texture: %d x %d, pixelformat %s error: %s\n", texture->setup.rotwidth, texture->setup.rotheight,
				texture->copyinfo->dstname, SDL_GetError());

	if ( (texture->copyinfo->func != NULL) && (texture->sdl_access == SDL_TEXTUREACCESS_STATIC))
	{
		texture->pixels = osd_malloc(texture->setup.rotwidth * texture->setup.rotheight * texture->copyinfo->dst_bpp);
	 texture->pixels_own=TRUE;
 }
	/* add us to the texture list */
	texture->next = sdl->texlist;
	sdl->texlist = texture;

	texture->last_access = osd_ticks();

	return texture;
}

//============================================================
//  texture_set_data
//============================================================

static void texture_set_data(sdl_info *sdl, texture_info *texture, const render_texinfo *texsource, UINT32 flags)
{
	texture->copyinfo->time -= osd_ticks();
	if (texture->sdl_access == SDL_TEXTUREACCESS_STATIC)
	{
		if ( texture->copyinfo->func )
	    {
			texture->pitch = texture->setup.rotwidth * texture->copyinfo->dst_bpp;
			texture->copyinfo->func(texture, texsource);
	    }
		else
		{
			texture->pixels = texsource->base;
			texture->pitch = texture->texinfo.rowpixels * texture->copyinfo->dst_bpp;
		}
		SDL_UpdateTexture(texture->texture_id, NULL, texture->pixels, texture->pitch);
	}
	else
	{
		SDL_LockTexture(texture->texture_id, NULL, 1, (void **) &texture->pixels, &texture->pitch);
		if ( texture->copyinfo->func )
			texture->copyinfo->func(texture, texsource);
		else
		{
			UINT8 *src = (UINT8 *) texsource->base;
			UINT8 *dst = (UINT8 *) texture->pixels;
			int spitch = texsource->rowpixels * texture->copyinfo->dst_bpp;
			int num = texsource->width * texture->copyinfo->dst_bpp;
			int h = texsource->height;
			while (h--) {
				memcpy(dst, src, num);
				src += spitch;
				dst += texture->pitch;
			}
		}
		SDL_UnlockTexture(texture->texture_id);
	}
	texture->copyinfo->time += osd_ticks();
}

//============================================================
//  compute rotation setup
//============================================================

static void compute_setup(sdl_info *sdl, const render_primitive *prim, quad_setup_data *setup, int flags)
{
	const render_quad_texuv *texcoords = &prim->texcoords;
	int texwidth = prim->texture.width;
	int texheight = prim->texture.height;
	float fdudx, fdvdx, fdudy, fdvdy;
	float width, height;
	float fscale;
	/* determine U/V deltas */
	if ((PRIMFLAG_GET_SCREENTEX(flags)))
		fscale = (float) video_config.prescale;
	else
		fscale = 1.0f;

	fdudx = (texcoords->tr.u - texcoords->tl.u) / fscale; // a a11
	fdvdx = (texcoords->tr.v - texcoords->tl.v) / fscale; // c a21
	fdudy = (texcoords->bl.u - texcoords->tl.u) / fscale; // b a12
	fdvdy = (texcoords->bl.v - texcoords->tl.v) / fscale; // d a22

	/* compute start and delta U,V coordinates now */

	setup->dudx = round_nearest(65536.0f * fdudx);
	setup->dvdx = round_nearest(65536.0f * fdvdx);
	setup->dudy = round_nearest(65536.0f * fdudy);
	setup->dvdy = round_nearest(65536.0f * fdvdy);
	setup->startu = round_nearest(65536.0f * (float) texwidth * texcoords->tl.u);
	setup->startv = round_nearest(65536.0f * (float) texheight * texcoords->tl.v);

	/* clamp to integers */

	width = fabs((fdudx * (float) (texwidth) + fdvdx * (float) (texheight)) * fscale * fscale);
	height = fabs((fdudy * (float)(texwidth) + fdvdy * (float) (texheight)) * fscale * fscale);

	setup->rotwidth = width;
	setup->rotheight = height;

	setup->startu += (setup->dudx + setup->dudy) / 2;
	setup->startv += (setup->dvdx + setup->dvdy) / 2;

}

//============================================================
//  texture_find
//============================================================

static texture_info *texture_find(sdl_info *sdl, const render_primitive *prim, quad_setup_data *setup)
{
	HashT texhash = texture_compute_hash(&prim->texture, prim->flags);
	texture_info *texture;
	osd_ticks_t now = osd_ticks();

	// find a match
	for (texture = sdl->texlist; texture != NULL; )
		if (texture->hash == texhash &&
			texture->texinfo.base == prim->texture.base &&
			texture->texinfo.width == prim->texture.width &&
			texture->texinfo.height == prim->texture.height &&
			texture->texinfo.rowpixels == prim->texture.rowpixels &&
			texture->setup.dudx == setup->dudx &&
			texture->setup.dvdx == setup->dvdx &&
			texture->setup.dudy == setup->dudy &&
			texture->setup.dvdy == setup->dvdy &&
			((texture->flags ^ prim->flags) & (PRIMFLAG_BLENDMODE_MASK | PRIMFLAG_TEXFORMAT_MASK)) == 0)
		{
			/* would we choose another blitter ? */
			if ((texture->copyinfo->samples & 0x1f) == 0x1f)
			{
				if (texture->copyinfo != texture_compute_size_type(&texture->texinfo, texture, prim->flags))
					return NULL;
#if 0
				else
				{
					/* reset stats */
					texture->copyinfo->samples = 0;
					texture->copyinfo->time = 0;
					texture->copyinfo->pixel_count = 0;
				}
#endif
			}
			texture->last_access = now;
			return texture;
		}
		else
		{
			/* free resources not needed any longer? */
			texture_info *expire = texture;
			texture = texture->next;
			if (now - expire->last_access > osd_ticks_per_second())
				draw13_destroy_texture(sdl, expire);
		}

	// nothing found
	return NULL;
}

//============================================================
//  texture_update
//============================================================

static texture_info * texture_update(sdl_window_info *window, const render_primitive *prim)
{
	sdl_info *sdl = (sdl_info *) window->dxdata;
	quad_setup_data setup;
	texture_info *texture;

	compute_setup(sdl, prim, &setup, prim->flags);

	texture = texture_find(sdl, prim, &setup);

	// if we didn't find one, create a new texture
	if (texture == NULL && prim->texture.base != NULL)
    {
        texture = texture_create(window, &prim->texture, &setup, prim->flags);
    }

    if (texture != NULL)
	{
		if (prim->texture.base != NULL && texture->texinfo.seqid != prim->texture.seqid)
		{
			texture->texinfo.seqid = prim->texture.seqid;
			// if we found it, but with a different seqid, copy the data
			texture_set_data(sdl, texture, &prim->texture, prim->flags);
		}

	}
    return texture;
}

static void draw13_destroy_texture(sdl_info *sdl, texture_info *texture)
{
	texture_info *p;

	SDL_DestroyTexture(texture->texture_id);
	if ( texture->pixels_own )
	{
		osd_free(texture->pixels);
		texture->pixels=NULL;
		texture->pixels_own=FALSE;
	}

	for (p=sdl->texlist; p != NULL; p = p->next)
		if (p->next == texture)
			break;
	if (p == NULL)
	    sdl->texlist = NULL;
	else
		p->next = texture->next;
	osd_free(texture);
}

static void draw13_destroy_all_textures(sdl_window_info *window)
{
	sdl_info *sdl = (sdl_info *) window->dxdata;
	texture_info *next_texture=NULL, *texture = NULL;
	int lock=FALSE;

	if (sdl == NULL)
		return;

	if(window->primlist && window->primlist->lock)
	{
		lock=TRUE;
		osd_lock_acquire(window->primlist->lock);
	}

	texture = sdl->texlist;

	while (texture)
	{
		next_texture = texture->next;
		draw13_destroy_texture(sdl, texture);
		texture = next_texture;
	}

	if (lock)
		osd_lock_release(window->primlist->lock);
}
