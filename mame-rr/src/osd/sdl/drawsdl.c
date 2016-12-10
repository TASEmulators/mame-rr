//============================================================
//
//  drawsdl.c - SDL software and OpenGL implementation
//
//  Copyright (c) 1996-2010, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

// standard C headers
#include <math.h>
#include <stdio.h>

// MAME headers
#include "emu.h"
#include "ui.h"

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

//============================================================
//  TYPES
//============================================================

typedef struct _sdl_scale_mode sdl_scale_mode;

/* sdl_info is the information about SDL for the current screen */
typedef struct _sdl_info sdl_info;
struct _sdl_info
{
	INT32				blittimer;
	UINT32				extra_flags;

#if (SDL_VERSION_ATLEAST(1,3,0))
	SDL_TextureID		texture_id;
#else
	// SDL surface
	SDL_Surface 		*sdlsurf;
	SDL_Overlay 		*yuvsurf;
#endif

	// YUV overlay
	UINT32				*yuv_lookup;
	UINT16				*yuv_bitmap;

	// if we leave scaling to SDL and the underlying driver, this
	// is the render_target_width/height to use

	int					hw_scale_width;
	int					hw_scale_height;
	int					last_hofs;
	int					last_vofs;
	int					old_blitwidth;
	int					old_blitheight;

	// shortcut to scale mode info

	const sdl_scale_mode		*scale_mode;
};

struct _sdl_scale_mode
{
	const char *name;
	int		is_scale;			/* Scale mode?           */
	int		is_yuv;				/* Yuv mode?             */
	int		mult_w;				/* Width multiplier      */
	int		mult_h;				/* Height multiplier     */
#if (!SDL_VERSION_ATLEAST(1,3,0))
	int		extra_flags;		/* Texture/surface flags */
#else
	int		sdl_scale_mode;		/* sdl 1.3 scale mode    */
#endif
	int		pixel_format;		/* Pixel/Overlay format  */
	void    (*yuv_blit)(UINT16 *bitmap, sdl_info *sdl, UINT8 *ptr, int pitch);
};

//============================================================
//  INLINES
//============================================================

//============================================================
//  PROTOTYPES
//============================================================

// core functions
static void drawsdl_exit(void);
static void drawsdl_attach(sdl_draw_info *info, sdl_window_info *window);
static int drawsdl_window_create(sdl_window_info *window, int width, int height);
static void drawsdl_window_resize(sdl_window_info *window, int width, int height);
static void drawsdl_window_destroy(sdl_window_info *window);
static const render_primitive_list *drawsdl_window_get_primitives(sdl_window_info *window);
static int drawsdl_window_draw(sdl_window_info *window, UINT32 dc, int update);
static void drawsdl_destroy_all_textures(sdl_window_info *window);
static void drawsdl_window_clear(sdl_window_info *window);
static int drawsdl_xy_to_render_target(sdl_window_info *window, int x, int y, int *xt, int *yt);

#if (SDL_VERSION_ATLEAST(1,3,0))
static void setup_texture(sdl_window_info *window, int tempwidth, int tempheight);
#endif

// soft rendering
static void drawsdl_rgb888_draw_primitives(const render_primitive *primlist, void *dstdata, UINT32 width, UINT32 height, UINT32 pitch);
static void drawsdl_bgr888_draw_primitives(const render_primitive *primlist, void *dstdata, UINT32 width, UINT32 height, UINT32 pitch);
static void drawsdl_bgra888_draw_primitives(const render_primitive *primlist, void *dstdata, UINT32 width, UINT32 height, UINT32 pitch);
static void drawsdl_rgb565_draw_primitives(const render_primitive *primlist, void *dstdata, UINT32 width, UINT32 height, UINT32 pitch);
static void drawsdl_rgb555_draw_primitives(const render_primitive *primlist, void *dstdata, UINT32 width, UINT32 height, UINT32 pitch);

// YUV overlays

static void drawsdl_yuv_init(sdl_info *sdl);
static void yuv_RGB_to_YV12(UINT16 *bitmap, sdl_info *sdl, UINT8 *ptr, int pitch);
static void yuv_RGB_to_YV12X2(UINT16 *bitmap, sdl_info *sdl, UINT8 *ptr, int pitch);
static void yuv_RGB_to_YUY2(UINT16 *bitmap, sdl_info *sdl, UINT8 *ptr, int pitch);
static void yuv_RGB_to_YUY2X2(UINT16 *bitmap, sdl_info *sdl, UINT8 *ptr, int pitch);

// Static declarations

#if (!SDL_VERSION_ATLEAST(1,3,0))
static int shown_video_info = 0;

static const sdl_scale_mode scale_modes[] =
{
		{ "none",    0, 0, 0, 0, SDL_DOUBLEBUF, 0, 0 },
		{ "async",   0, 0, 0, 0, SDL_DOUBLEBUF | SDL_ASYNCBLIT, 0, 0 },
		{ "yv12",    1, 1, 1, 1, 0,              SDL_YV12_OVERLAY, yuv_RGB_to_YV12 },
		{ "yv12x2",  1, 1, 2, 2, 0,              SDL_YV12_OVERLAY, yuv_RGB_to_YV12X2 },
		{ "yuy2",    1, 1, 1, 1, 0,              SDL_YUY2_OVERLAY, yuv_RGB_to_YUY2 },
		{ "yuy2x2",  1, 1, 2, 1, 0,              SDL_YUY2_OVERLAY, yuv_RGB_to_YUY2X2 },
		{ NULL }
};
#else
static const sdl_scale_mode scale_modes[] =
{
		{ "none",    0, 0, 0, 0, SDL_TEXTURESCALEMODE_NONE, 0, 0 },
		{ "hwblit",  1, 0, 1, 1, SDL_TEXTURESCALEMODE_FAST, 0, 0 },
		{ "hwbest",  1, 0, 1, 1, SDL_TEXTURESCALEMODE_BEST, 0, 0 },
		{ "yv12",    1, 1, 1, 1, SDL_TEXTURESCALEMODE_NONE, SDL_PIXELFORMAT_YV12, yuv_RGB_to_YV12 },
		{ "yv12x2",  1, 1, 2, 2, SDL_TEXTURESCALEMODE_NONE, SDL_PIXELFORMAT_YV12, yuv_RGB_to_YV12X2 },
		{ "yuy2",    1, 1, 1, 1, SDL_TEXTURESCALEMODE_NONE, SDL_PIXELFORMAT_YUY2, yuv_RGB_to_YUY2 },
		{ "yuy2x2",  1, 1, 2, 1, SDL_TEXTURESCALEMODE_NONE, SDL_PIXELFORMAT_YUY2, yuv_RGB_to_YUY2X2 },
		{ NULL }
};
#endif

//============================================================
//  drawsdl_scale_mode
//============================================================

const char *drawsdl_scale_mode_str(int index)
{
	const sdl_scale_mode *sm = scale_modes;

	while (index>0)
	{
		if (sm->name == NULL)
			return NULL;
		index--;
		sm++;
	}
	return sm->name;
};

int drawsdl_scale_mode(const char *s)
{
	const sdl_scale_mode *sm = scale_modes;
	int index;

	index = 0;
	while (sm->name != NULL)
	{
		if (strcmp(sm->name, s) == 0)
			return index;
		index++;
		sm++;
	}
	return -1;
}

//============================================================
//  drawsdl_init
//============================================================

int drawsdl_init(sdl_draw_info *callbacks)
{
	// fill in the callbacks
	callbacks->exit = drawsdl_exit;
	callbacks->attach = drawsdl_attach;

	if (SDL_VERSION_ATLEAST(1,3,0))
		mame_printf_verbose("Using SDL multi-window soft driver (SDL 1.3+)\n");
	else
		mame_printf_verbose("Using SDL single-window soft driver (SDL 1.2)\n");

	return 0;
}

//============================================================
//  drawsdl_exit
//============================================================

static void drawsdl_exit(void)
{
}

//============================================================
//  drawsdl_attach
//============================================================

static void drawsdl_attach(sdl_draw_info *info, sdl_window_info *window)
{
	// fill in the callbacks
	window->create = drawsdl_window_create;
	window->resize = drawsdl_window_resize;
	window->get_primitives = drawsdl_window_get_primitives;
	window->draw = drawsdl_window_draw;
	window->destroy = drawsdl_window_destroy;
	window->destroy_all_textures = drawsdl_destroy_all_textures;
	window->clear = drawsdl_window_clear;
	window->xy_to_render_target = drawsdl_xy_to_render_target;
}

//============================================================
//  drawsdl_destroy_all_textures
//============================================================

static void drawsdl_destroy_all_textures(sdl_window_info *window)
{
	/* nothing to be done in soft mode */
}

//============================================================
//  setup_texture for window
//============================================================

#if (SDL_VERSION_ATLEAST(1,3,0))
static void setup_texture(sdl_window_info *window, int tempwidth, int tempheight)
{
	sdl_info *sdl = (sdl_info *) window->dxdata;
	const sdl_scale_mode *sdl_sm = sdl->scale_mode;
	SDL_DisplayMode mode;
	UINT32 fmt;

	// Determine preferred pixelformat and set up yuv if necessary
	SDL_GetCurrentDisplayMode(&mode);

	if (sdl->yuv_bitmap)
	{
		global_free(sdl->yuv_bitmap);
		sdl->yuv_bitmap = NULL;
	}

	if (sdl_sm->is_scale)
	{
		render_target_get_minimum_size(window->target, &sdl->hw_scale_width, &sdl->hw_scale_height);
		if (video_config.prescale)
		{
			sdl->hw_scale_width *= video_config.prescale;
			sdl->hw_scale_height *= video_config.prescale;
		}
	}

	if (sdl_sm->is_yuv)
		sdl->yuv_bitmap = global_alloc_array(UINT16, sdl->hw_scale_width * sdl->hw_scale_height);

	fmt = (sdl_sm->pixel_format ? sdl_sm->pixel_format : mode.format);

	if (sdl_sm->is_scale)
	{
		int w = sdl->hw_scale_width * sdl_sm->mult_w;
		int h = sdl->hw_scale_height * sdl_sm->mult_h;

		sdl->texture_id = SDL_CreateTexture(fmt, SDL_TEXTUREACCESS_STREAMING, w, h);

	}
	else
	{
		sdl->texture_id = SDL_CreateTexture(fmt, SDL_TEXTUREACCESS_STREAMING,
				tempwidth, tempheight);
	}

	SDL_SetTextureScaleMode(sdl->texture_id, sdl_sm->sdl_scale_mode);

}
#endif

//============================================================
//  yuv_overlay_init
//============================================================

#if (!SDL_VERSION_ATLEAST(1,3,0))
static void yuv_overlay_init(sdl_window_info *window)
{
	sdl_info *sdl = (sdl_info *) window->dxdata;
	const sdl_scale_mode *sdl_sm = sdl->scale_mode;
	int minimum_width, minimum_height;

	render_target_get_minimum_size(window->target, &minimum_width, &minimum_height);

	if (video_config.prescale)
	{
		minimum_width *= video_config.prescale;
		minimum_height *= video_config.prescale;
	}

	if (sdl->yuvsurf != NULL)
	{
		SDL_FreeYUVOverlay(sdl->yuvsurf);
		sdl->yuvsurf = NULL;
	}

	if (sdl->yuv_bitmap != NULL)
	{
		global_free(sdl->yuv_bitmap);
	}

	mame_printf_verbose("SDL: Creating %d x %d YUV-Overlay ...\n", minimum_width, minimum_height);

	sdl->yuv_bitmap = global_alloc_array(UINT16, minimum_width*minimum_height);

	sdl->yuvsurf = SDL_CreateYUVOverlay(minimum_width * sdl_sm->mult_w, minimum_height * sdl_sm->mult_h,
			sdl_sm->pixel_format, sdl->sdlsurf);

	if ( sdl->yuvsurf == NULL ) {
		mame_printf_error("SDL: Couldn't create SDL_yuv_overlay: %s\n", SDL_GetError());
		//return 1;
	}

	sdl->hw_scale_width = minimum_width;
	sdl->hw_scale_height = minimum_height;

	if (!shown_video_info)
	{
		mame_printf_verbose("YUV Mode         : %s\n", sdl_sm->name);
		mame_printf_verbose("YUV Overlay Size : %d x %d\n", minimum_width, minimum_height);
		mame_printf_verbose("YUV Acceleration : %s\n", sdl->yuvsurf->hw_overlay ? "Hardware" : "Software");
		shown_video_info = 1;
	}
}
#endif

//============================================================
//  drawsdl_show_info
//============================================================

#if (SDL_VERSION_ATLEAST(1,3,0))
static void	drawsdl_show_info(sdl_window_info *window, struct SDL_RendererInfo *render_info)
{
#define RF_ENTRY(x) {x, #x }
	static struct {
		int flag;
		const char *name;
	} rflist[] =
		{
			RF_ENTRY(SDL_RENDERER_SINGLEBUFFER),
			RF_ENTRY(SDL_RENDERER_PRESENTCOPY),
			RF_ENTRY(SDL_RENDERER_PRESENTFLIP2),
			RF_ENTRY(SDL_RENDERER_PRESENTFLIP3),
			RF_ENTRY(SDL_RENDERER_PRESENTDISCARD),
			RF_ENTRY(SDL_RENDERER_PRESENTVSYNC),
			RF_ENTRY(SDL_RENDERER_ACCELERATED),
			{-1, NULL}
		};
	int i;

	mame_printf_verbose("window: using renderer %s\n", render_info->name ? render_info->name : "<unknown>");
	for (i = 0; rflist[i].name != NULL; i++)
		if (render_info->flags & rflist[i].flag)
			mame_printf_verbose("renderer: flag %s\n", rflist[i].name);
}
#endif

//============================================================
//  drawsdl_window_create
//============================================================

static int drawsdl_window_create(sdl_window_info *window, int width, int height)
{
	sdl_info *sdl;

	// allocate memory for our structures
	sdl = (sdl_info *) osd_malloc(sizeof(sdl_info));
	memset(sdl, 0, sizeof(sdl_info));

	window->dxdata = sdl;

	sdl->scale_mode = &scale_modes[window->scale_mode];

#if (SDL_VERSION_ATLEAST(1,3,0))
	sdl->extra_flags = (window->fullscreen ?
			SDL_WINDOW_BORDERLESS | SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS
			| SDL_WINDOW_INPUT_GRABBED : SDL_WINDOW_RESIZABLE);

	SDL_SelectVideoDisplay(window->monitor->handle);

	if (window->fullscreen && video_config.switchres)
	{
		SDL_DisplayMode mode;
		SDL_GetCurrentDisplayMode(&mode);
		mode.w = width;
		mode.h = height;
		if (window->refresh)
			mode.refresh_rate = window->refresh;
		SDL_SetWindowDisplayMode(window->sdl_window, &mode);	// Try to set mode
	}
	else
		SDL_SetWindowDisplayMode(window->sdl_window, NULL);	// Use desktop

	window->sdl_window = SDL_CreateWindow(window->title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			width, height, sdl->extra_flags);
	SDL_ShowWindow(window->sdl_window);

	SDL_SetWindowFullscreen(window->sdl_window, window->fullscreen);
	SDL_GetWindowSize(window->sdl_window, &window->width, &window->height);
	SDL_RaiseWindow(window->sdl_window);

	/* FIXME: Bug in SDL 1.3 */
	if (window->fullscreen)
		SDL_SetWindowGrab(window->sdl_window, 1);

	// create a texture

	if (video_config.waitvsync)
		SDL_CreateRenderer(window->sdl_window, -1, SDL_RENDERER_PRESENTFLIP2 | SDL_RENDERER_PRESENTDISCARD | SDL_RENDERER_PRESENTVSYNC);
	else
		SDL_CreateRenderer(window->sdl_window, -1, SDL_RENDERER_PRESENTFLIP2 | SDL_RENDERER_PRESENTDISCARD);

    SDL_SelectRenderer(window->sdl_window);

    {
        struct SDL_RendererInfo render_info;
    	SDL_GetRendererInfo(&render_info);

    	drawsdl_show_info(window, &render_info);

    	// Check scale mode

    	if (sdl->scale_mode->pixel_format)
    	{
    		int i;
    		int found = 0;

    		for (i=0; i < render_info.num_texture_formats; i++)
    			if (sdl->scale_mode->pixel_format == render_info.texture_formats[i])
    				found = 1;

    		if (!found)
    		{
    			mame_printf_verbose("window: Scale mode %s not supported! Using default.\n", sdl->scale_mode->name);
    			ui_popup_time(3, "Scale mode %s not supported! Using default.", sdl->scale_mode->name);
    			sdl->scale_mode = &scale_modes[0];
    		}
    	}
    }

	setup_texture(window, width, height);
#else
	sdl->extra_flags = (window->fullscreen ?  SDL_FULLSCREEN : SDL_RESIZABLE);

	sdl->extra_flags |= sdl->scale_mode->extra_flags;

	sdl->sdlsurf = SDL_SetVideoMode(width, height,
				   0, SDL_SWSURFACE | SDL_ANYFORMAT | sdl->extra_flags);

	if (!sdl->sdlsurf)
		return 1;

	window->width = sdl->sdlsurf->w;
	window->height = sdl->sdlsurf->h;

	if (sdl->scale_mode->is_yuv)
		yuv_overlay_init(window);

	// set the window title
	SDL_WM_SetCaption(window->title, "SDLMAME");
#endif
	sdl->yuv_lookup = NULL;
	sdl->blittimer = 0;

	drawsdl_yuv_init(sdl);
	return 0;
}

//============================================================
//  drawsdl_window_resize
//============================================================

static void drawsdl_window_resize(sdl_window_info *window, int width, int height)
{

#if (SDL_VERSION_ATLEAST(1,3,0))
	SDL_SetWindowSize(window->sdl_window, width, height);
	SDL_GetWindowSize(window->sdl_window, &window->width, &window->height);

#else
	sdl_info *sdl = (sdl_info *) window->dxdata;

	if (sdl->yuvsurf != NULL)
	{
		SDL_FreeYUVOverlay(sdl->yuvsurf);
		sdl->yuvsurf = NULL;
	}
	SDL_FreeSurface(sdl->sdlsurf);
	//printf("SetVideoMode %d %d\n", wp->resize_new_width, wp->resize_new_height);

	sdl->sdlsurf = SDL_SetVideoMode(width, height, 0,
			SDL_SWSURFACE | SDL_ANYFORMAT | sdl->extra_flags);
	window->width = sdl->sdlsurf->w;
	window->height = sdl->sdlsurf->h;

	if (sdl->scale_mode->is_yuv)
	{
		yuv_overlay_init(window);
	}

#endif
}


//============================================================
//  drawsdl_window_destroy
//============================================================

static void drawsdl_window_destroy(sdl_window_info *window)
{
	sdl_info *sdl = (sdl_info *) window->dxdata;

	// skip if nothing
	if (sdl == NULL)
		return;

#if (SDL_VERSION_ATLEAST(1,3,0))
	SDL_SelectRenderer(window->sdl_window);
	SDL_DestroyTexture(sdl->texture_id);
	//SDL_DestroyRenderer(window->sdl_window);
	SDL_DestroyWindow(window->sdl_window);
#else
	if (sdl->yuvsurf != NULL)
	{
		SDL_FreeYUVOverlay(sdl->yuvsurf);
		sdl->yuvsurf = NULL;
	}

	if (sdl->sdlsurf)
	{
		SDL_FreeSurface(sdl->sdlsurf);
		sdl->sdlsurf = NULL;
	}
#endif
	// free the memory in the window

	if (sdl->yuv_lookup != NULL)
	{
		global_free(sdl->yuv_lookup);
		sdl->yuv_lookup = NULL;
	}
	if (sdl->yuv_bitmap != NULL)
	{
		global_free(sdl->yuv_bitmap);
		sdl->yuv_bitmap = NULL;
	}
	osd_free(sdl);
	window->dxdata = NULL;
}

//============================================================
//  drawsdl_window_clear
//============================================================

static void drawsdl_window_clear(sdl_window_info *window)
{
	sdl_info *sdl = (sdl_info *) window->dxdata;

	sdl->blittimer = 3;
}

//============================================================
//  drawsdl_xy_to_render_target
//============================================================

static int drawsdl_xy_to_render_target(sdl_window_info *window, int x, int y, int *xt, int *yt)
{
	sdl_info *sdl = (sdl_info *) window->dxdata;

	*xt = x - sdl->last_hofs;
	*yt = y - sdl->last_vofs;
	if (*xt<0 || *xt >= window->blitwidth)
		return 0;
	if (*yt<0 || *xt >= window->blitheight)
		return 0;
	if (!sdl->scale_mode->is_scale)
	{
		return 1;
	}
	/* Rescale */
	*xt = (*xt * sdl->hw_scale_width) / window->blitwidth;
	*yt = (*yt * sdl->hw_scale_height) / window->blitheight;
	return 1;
}

//============================================================
//  drawsdl_window_get_primitives
//============================================================

static const render_primitive_list *drawsdl_window_get_primitives(sdl_window_info *window)
{
	sdl_info *sdl = (sdl_info *) window->dxdata;

	if ((!window->fullscreen) || (video_config.switchres))
	{
		sdlwindow_blit_surface_size(window, window->width, window->height);
	}
	else
	{
		sdlwindow_blit_surface_size(window, window->monitor->center_width, window->monitor->center_height);
	}
	if (!sdl->scale_mode->is_scale)
		render_target_set_bounds(window->target, window->blitwidth, window->blitheight, sdlvideo_monitor_get_aspect(window->monitor));
	else
		render_target_set_bounds(window->target, sdl->hw_scale_width, sdl->hw_scale_height, 0);
	return render_target_get_primitives(window->target);
}

//============================================================
//  drawsdl_window_draw
//============================================================

static int drawsdl_window_draw(sdl_window_info *window, UINT32 dc, int update)
{
	sdl_info *sdl = (sdl_info *) window->dxdata;
	UINT8 *surfptr;
	INT32 pitch;
	int bpp;
	Uint32 rmask, gmask, bmask, amask;
	INT32 vofs, hofs, blitwidth, blitheight, ch, cw;

	if (video_config.novideo)
	{
		return 0;
	}

	// if we haven't been created, just punt
	if (sdl == NULL)
		return 1;

	// lock it if we need it
#if (!SDL_VERSION_ATLEAST(1,3,0))

	pitch = sdl->sdlsurf->pitch;
	bpp = sdl->sdlsurf->format->BytesPerPixel;
	rmask = sdl->sdlsurf->format->Rmask;
	gmask = sdl->sdlsurf->format->Gmask;
	bmask = sdl->sdlsurf->format->Bmask;
	amask = sdl->sdlsurf->format->Amask;

	if (window->blitwidth != sdl->old_blitwidth || window->blitheight != sdl->old_blitheight)
	{
		if (sdl->scale_mode->is_yuv)
			yuv_overlay_init(window);
		sdl->old_blitwidth = window->blitwidth;
		sdl->old_blitheight = window->blitheight;
		sdl->blittimer = 3;
	}

	if (SDL_MUSTLOCK(sdl->sdlsurf)) SDL_LockSurface(sdl->sdlsurf);
	// Clear if necessary

	if (sdl->blittimer > 0)
	{
		memset(sdl->sdlsurf->pixels, 0, window->height * sdl->sdlsurf->pitch);
		sdl->blittimer--;
	}


	if (sdl->scale_mode->is_yuv)
	{
		SDL_LockYUVOverlay(sdl->yuvsurf);
		surfptr = sdl->yuvsurf->pixels[0]; // (UINT8 *) sdl->yuv_bitmap;
		pitch = sdl->yuvsurf->pitches[0]; // (UINT8 *) sdl->yuv_bitmap;
	}
	else
		surfptr = (UINT8 *)sdl->sdlsurf->pixels;
#else
	SDL_SelectRenderer(window->sdl_window);

	if (window->blitwidth != sdl->old_blitwidth || window->blitheight != sdl->old_blitheight)
	{
		SDL_DestroyTexture(sdl->texture_id);
		setup_texture(window, window->blitwidth, window->blitheight);
		sdl->old_blitwidth = window->blitwidth;
		sdl->old_blitheight = window->blitheight;
		sdl->blittimer = 3;
	}

	{
        Uint32 format;
        int access, w, h;

        SDL_QueryTexture(sdl->texture_id, &format, &access, &w, &h);
		SDL_PixelFormatEnumToMasks(format, &bpp, &rmask, &gmask, &bmask, &amask);
		bpp = bpp / 8; /* convert to bytes per pixels */
	}

	// Clear if necessary
	if (sdl->blittimer > 0)
	{
		/* SDL Underlays need alpha = 0 ! */
		SDL_SetRenderDrawColor(0,0,0,0);
		SDL_RenderFillRect(NULL);
		//SDL_RenderFill(0,0,0,0 /*255*/,NULL);
		sdl->blittimer--;
	}

	SDL_LockTexture(sdl->texture_id, NULL, 1, (void **) &surfptr, &pitch);

#endif
	// get ready to center the image
	vofs = hofs = 0;
	blitwidth = window->blitwidth;
	blitheight = window->blitheight;

	// figure out what coordinate system to use for centering - in window mode it's always the
	// SDL surface size.  in fullscreen the surface covers all monitors, so center according to
	// the first one only
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

	// do not crash if the window's smaller than the blit area
	if (blitheight > ch)
	{
			blitheight = ch;
	}
	else if (video_config.centerv)
	{
		vofs = (ch - window->blitheight) / 2;
	}

	if (blitwidth > cw)
	{
			blitwidth = cw;
	}
	else if (video_config.centerh)
	{
		hofs = (cw - window->blitwidth) / 2;
	}

	sdl->last_hofs = hofs;
	sdl->last_vofs = vofs;

	osd_lock_acquire(window->primlist->lock);

	// render to it
	if (!sdl->scale_mode->is_yuv)
	{
		int mamewidth, mameheight;

		if (!sdl->scale_mode->is_scale)
		{
			mamewidth = blitwidth;
			mameheight = blitheight;
#if !SDL_VERSION_ATLEAST(1,3,0)
			surfptr += ((vofs * pitch) + (hofs * bpp));
#endif
		}
		else
		{
			mamewidth = sdl->hw_scale_width;
			mameheight = sdl->hw_scale_height;
		}
		switch (rmask)
		{
			case 0x0000ff00:
				drawsdl_bgra888_draw_primitives(window->primlist->head, surfptr, mamewidth, mameheight, pitch / 4);
				break;

			case 0x00ff0000:
				drawsdl_rgb888_draw_primitives(window->primlist->head, surfptr, mamewidth, mameheight, pitch / 4);
				break;

			case 0x000000ff:
				drawsdl_bgr888_draw_primitives(window->primlist->head, surfptr, mamewidth, mameheight, pitch / 4);
				break;

			case 0xf800:
				drawsdl_rgb565_draw_primitives(window->primlist->head, surfptr, mamewidth, mameheight, pitch / 2);
				break;

			case 0x7c00:
				drawsdl_rgb555_draw_primitives(window->primlist->head, surfptr, mamewidth, mameheight, pitch / 2);
				break;

			default:
				mame_printf_error("SDL: ERROR! Unknown video mode: R=%08X G=%08X B=%08X\n", rmask, gmask, bmask);
				break;
		}
	}
	else
	{
		assert (sdl->yuv_bitmap != NULL);
		assert (surfptr != NULL);
		drawsdl_rgb555_draw_primitives(window->primlist->head, sdl->yuv_bitmap, sdl->hw_scale_width, sdl->hw_scale_height, sdl->hw_scale_width);
		sdl->scale_mode->yuv_blit((UINT16 *)sdl->yuv_bitmap, sdl, surfptr, pitch);
	}

	osd_lock_release(window->primlist->lock);

	// unlock and flip
#if (!SDL_VERSION_ATLEAST(1,3,0))
	if (SDL_MUSTLOCK(sdl->sdlsurf)) SDL_UnlockSurface(sdl->sdlsurf);
	if (!sdl->scale_mode->is_yuv)
	{
		SDL_Flip(sdl->sdlsurf);
	}
	else
	{
		SDL_Rect r;

		SDL_UnlockYUVOverlay(sdl->yuvsurf);
		r.x=hofs;
		r.y=vofs;
		r.w=blitwidth;
		r.h=blitheight;
		SDL_DisplayYUVOverlay(sdl->yuvsurf, &r);
	}
#else
	SDL_UnlockTexture(sdl->texture_id);
	{
		SDL_Rect r;

		r.x=hofs;
		r.y=vofs;
		r.w=blitwidth;
		r.h=blitheight;
		//printf("blitwidth %d %d - %d %d\n", blitwidth, blitheight, window->width, window->height);
		//SDL_UpdateTexture(sdl->sdltex, NULL, sdl->sdlsurf->pixels, pitch);
		SDL_RenderCopy(sdl->texture_id, NULL, &r);
		SDL_RenderPresent();
	}
#endif
	return 0;
}

//============================================================
//  SOFTWARE RENDERING
//============================================================

#define FUNC_PREFIX(x)		drawsdl_rgb888_##x
#define PIXEL_TYPE			UINT32
#define SRCSHIFT_R			0
#define SRCSHIFT_G			0
#define SRCSHIFT_B			0
#define DSTSHIFT_R			16
#define DSTSHIFT_G			8
#define DSTSHIFT_B			0

#include "rendersw.c"

#define FUNC_PREFIX(x)		drawsdl_bgr888_##x
#define PIXEL_TYPE			UINT32
#define SRCSHIFT_R			0
#define SRCSHIFT_G			0
#define SRCSHIFT_B			0
#define DSTSHIFT_R			0
#define DSTSHIFT_G			8
#define DSTSHIFT_B			16

#include "rendersw.c"

#define FUNC_PREFIX(x)		drawsdl_bgra888_##x
#define PIXEL_TYPE			UINT32
#define SRCSHIFT_R			0
#define SRCSHIFT_G			0
#define SRCSHIFT_B			0
#define DSTSHIFT_R			8
#define DSTSHIFT_G			16
#define DSTSHIFT_B			24

#include "rendersw.c"

#define FUNC_PREFIX(x)		drawsdl_rgb565_##x
#define PIXEL_TYPE			UINT16
#define SRCSHIFT_R			3
#define SRCSHIFT_G			2
#define SRCSHIFT_B			3
#define DSTSHIFT_R			11
#define DSTSHIFT_G			5
#define DSTSHIFT_B			0

#include "rendersw.c"

#define FUNC_PREFIX(x)		drawsdl_rgb555_##x
#define PIXEL_TYPE			UINT16
#define SRCSHIFT_R			3
#define SRCSHIFT_G			3
#define SRCSHIFT_B			3
#define DSTSHIFT_R			10
#define DSTSHIFT_G			5
#define DSTSHIFT_B			0

#include "rendersw.c"

//============================================================
// YUV Blitting
//============================================================

#define CU_CLAMP(v, a, b)	((v < a)? a: ((v > b)? b: v))
#define RGB2YUV_F(r,g,b,y,u,v) \
	 (y) = (0.299*(r) + 0.587*(g) + 0.114*(b) ); \
	 (u) = (-0.169*(r) - 0.331*(g) + 0.5*(b) + 128); \
	 (v) = (0.5*(r) - 0.419*(g) - 0.081*(b) + 128); \
	(y) = CU_CLAMP(y,0,255); \
	(u) = CU_CLAMP(u,0,255); \
	(v) = CU_CLAMP(v,0,255)

#define RGB2YUV(r,g,b,y,u,v) \
	 (y) = ((  8453*(r) + 16594*(g) +  3223*(b) +  524288) >> 15); \
	 (u) = (( -4878*(r) -  9578*(g) + 14456*(b) + 4210688) >> 15); \
	 (v) = (( 14456*(r) - 12105*(g) -  2351*(b) + 4210688) >> 15)

#ifdef LSB_FIRST
#define Y1MASK  0x000000FF
#define  UMASK  0x0000FF00
#define Y2MASK  0x00FF0000
#define  VMASK  0xFF000000
#define Y1SHIFT  0
#define  USHIFT  8
#define Y2SHIFT 16
#define  VSHIFT 24
#else
#define Y1MASK  0xFF000000
#define  UMASK  0x00FF0000
#define Y2MASK  0x0000FF00
#define  VMASK  0x000000FF
#define Y1SHIFT 24
#define  USHIFT 16
#define Y2SHIFT  8
#define  VSHIFT  0
#endif

#define YMASK  (Y1MASK|Y2MASK)
#define UVMASK (UMASK|VMASK)

static void yuv_lookup_set(sdl_info *sdl, unsigned int pen, unsigned char red,
			unsigned char green, unsigned char blue)
{
	UINT32 y,u,v;

	RGB2YUV(red,green,blue,y,u,v);

	/* Storing this data in YUYV order simplifies using the data for
       YUY2, both with and without smoothing... */
	sdl->yuv_lookup[pen]=(y<<Y1SHIFT)|(u<<USHIFT)|(y<<Y2SHIFT)|(v<<VSHIFT);
}

static void drawsdl_yuv_init(sdl_info *sdl)
{
	unsigned char r,g,b;
	if (sdl->yuv_lookup == NULL)
		sdl->yuv_lookup = global_alloc_array(UINT32, 65536);
	for (r = 0; r < 32; r++)
		for (g = 0; g < 32; g++)
			for (b = 0; b < 32; b++)
			{
				int idx = (r << 10) | (g << 5) | b;
				yuv_lookup_set(sdl, idx,
					(r << 3) | (r >> 2),
					(g << 3) | (g >> 2),
					(b << 3) | (b >> 2));
			}
}

static void yuv_RGB_to_YV12(UINT16 *bitmap, sdl_info *sdl, UINT8 *ptr, int pitch)
{
	int x, y;
	UINT8 *dest_y;
	UINT8 *dest_u;
	UINT8 *dest_v;
	UINT16 *src;
	UINT16 *src2;
	UINT32 *lookup = sdl->yuv_lookup;
	UINT8 *pixels[3];
	int u1,v1,y1,u2,v2,y2,u3,v3,y3,u4,v4,y4;	  /* 12 */

	pixels[0] = ptr;
	pixels[1] = ptr + pitch * sdl->hw_scale_height;
	pixels[2] = pixels[1] + pitch * sdl->hw_scale_height / 4;

	for(y=0;y<sdl->hw_scale_height;y+=2)
	{
		src=bitmap + (y * sdl->hw_scale_width) ;
		src2=src + sdl->hw_scale_width;

		dest_y = pixels[0] + y * pitch;
		dest_v = pixels[1] + (y>>1) * pitch / 2;
		dest_u = pixels[2] + (y>>1) * pitch / 2;

		for(x=0;x<sdl->hw_scale_width;x+=2)
		{
			v1 = lookup[src[x]];
			y1 = (v1>>Y1SHIFT) & 0xff;
			u1 = (v1>>USHIFT)  & 0xff;
			v1 = (v1>>VSHIFT)  & 0xff;

			v2 = lookup[src[x+1]];
			y2 = (v2>>Y1SHIFT) & 0xff;
			u2 = (v2>>USHIFT)  & 0xff;
			v2 = (v2>>VSHIFT)  & 0xff;

			v3 = lookup[src2[x]];
			y3 = (v3>>Y1SHIFT) & 0xff;
			u3 = (v3>>USHIFT)  & 0xff;
			v3 = (v3>>VSHIFT)  & 0xff;

			v4 = lookup[src2[x+1]];
			y4 = (v4>>Y1SHIFT) & 0xff;
			u4 = (v4>>USHIFT)  & 0xff;
			v4 = (v4>>VSHIFT)  & 0xff;

			dest_y[x] = y1;
			dest_y[x+pitch] = y3;
			dest_y[x+1] = y2;
			dest_y[x+pitch+1] = y4;

			dest_u[x>>1] = (u1+u2+u3+u4)/4;
			dest_v[x>>1] = (v1+v2+v3+v4)/4;

		}
	}
}

static void yuv_RGB_to_YV12X2(UINT16 *bitmap, sdl_info *sdl, UINT8 *ptr, int pitch)
{
	/* this one is used when scale==2 */
	unsigned int x,y;
	UINT16 *dest_y;
	UINT8 *dest_u;
	UINT8 *dest_v;
	UINT16 *src;
	int u1,v1,y1;
	UINT8 *pixels[3];

	pixels[0] = ptr;
	pixels[1] = ptr + pitch * sdl->hw_scale_height * 2;
	pixels[2] = pixels[1] + pitch * sdl->hw_scale_height / 2;

	for(y=0;y<sdl->hw_scale_height;y++)
	{
		src = bitmap + (y * sdl->hw_scale_width) ;

		dest_y = (UINT16 *)(pixels[0] + 2 * y * pitch);
		dest_v = pixels[1] + y * pitch / 2;
		dest_u = pixels[2] + y * pitch / 2;
		for(x=0;x<sdl->hw_scale_width;x++)
		{
			v1 = sdl->yuv_lookup[src[x]];
			y1 = (v1 >> Y1SHIFT) & 0xff;
			u1 = (v1 >> USHIFT)  & 0xff;
			v1 = (v1 >> VSHIFT)  & 0xff;

			dest_y[x + pitch/2] = y1 << 8 | y1;
			dest_y[x] = y1 << 8 | y1;
			dest_u[x] = u1;
			dest_v[x] = v1;
		}
	}
}

static void yuv_RGB_to_YUY2(UINT16 *bitmap, sdl_info *sdl, UINT8 *ptr, int pitch)
{
	/* this one is used when scale==2 */
	unsigned int y;
	UINT32 *dest;
	UINT16 *src;
	UINT16 *end;
	UINT32 p1,p2,uv;
	UINT32 *lookup = sdl->yuv_lookup;
	int yuv_pitch = pitch/4;

	for(y=0;y<sdl->hw_scale_height;y++)
	{
		src=bitmap + (y * sdl->hw_scale_width) ;
		end=src+sdl->hw_scale_width;

		dest = (UINT32 *) ptr;
		dest += y * yuv_pitch;
		for(; src<end; src+=2)
		{
			p1  = lookup[src[0]];
			p2  = lookup[src[1]];
			uv = (p1&UVMASK)>>1;
			uv += (p2&UVMASK)>>1;
			*dest++ = (p1&Y1MASK)|(p2&Y2MASK)|(uv&UVMASK);
		}
	}
}

static void yuv_RGB_to_YUY2X2(UINT16 *bitmap, sdl_info *sdl, UINT8 *ptr, int pitch)
{
	/* this one is used when scale==2 */
	unsigned int y;
	UINT32 *dest;
	UINT16 *src;
	UINT16 *end;
	UINT32 *lookup = sdl->yuv_lookup;
	int yuv_pitch = pitch / 4;

	for(y=0;y<sdl->hw_scale_height;y++)
	{
		src=bitmap + (y * sdl->hw_scale_width) ;
		end=src+sdl->hw_scale_width;

		dest = (UINT32 *) ptr;
		dest += (y * yuv_pitch);
		for(; src<end; src++)
		{
			dest[0] = lookup[src[0]];
			dest++;
		}
	}
}
