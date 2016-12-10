//============================================================
//
//  video.c - SDL video handling
//
//  Copyright (c) 1996-2010, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#include <SDL/SDL.h>
// on win32 this includes windows.h by itself and breaks us!
#ifndef SDLMAME_WIN32
#include <SDL/SDL_syswm.h>
#endif

#ifdef SDLMAME_X11
#include <X11/extensions/Xinerama.h>
#endif

#ifdef SDLMAME_MACOSX
#undef Status
#include <Carbon/Carbon.h>
#endif

#ifdef SDLMAME_WIN32
// for multimonitor
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x501
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "strconv.h"
#endif

#ifdef SDLMAME_OS2
#define INCL_WIN
#include <os2.h>
#endif

// MAME headers
#include "emu.h"
#include "rendutil.h"
#include "ui.h"
#include "emuopts.h"
#include "uiinput.h"


// MAMEOS headers
#include "video.h"
#include "window.h"
#include "input.h"
#include "debugwin.h"

#include "osdsdl.h"
#include "sdlos.h"

//============================================================
//  CONSTANTS
//============================================================


//============================================================
//  GLOBAL VARIABLES
//============================================================

sdl_video_config video_config;

#ifndef NO_OPENGL
#ifdef USE_DISPATCH_GL
osd_gl_dispatch *gl_dispatch;
#endif
#endif

//============================================================
//  LOCAL VARIABLES
//============================================================

static sdl_monitor_info *primary_monitor;
static sdl_monitor_info *sdl_monitor_list;

static bitmap_t *effect_bitmap;

//============================================================
//  PROTOTYPES
//============================================================

static void video_exit(running_machine &machine);
static void init_monitors(void);
static sdl_monitor_info *pick_monitor(int index);

static void check_osd_inputs(running_machine *machine);

static void extract_video_config(running_machine *machine);
static void extract_window_config(running_machine *machine, int index, sdl_window_config *conf);
static void load_effect_overlay(running_machine *machine, const char *filename);
static float get_aspect(const char *name, int report_error);
static void get_resolution(const char *name, sdl_window_config *config, int report_error);


//============================================================
//  sdlvideo_init
//============================================================

int sdlvideo_init(running_machine *machine)
{
	int index, tc;

	// extract data from the options
	extract_video_config(machine);

	// ensure we get called on the way out
	machine->add_notifier(MACHINE_NOTIFY_EXIT, video_exit);

	// set up monitors first
	init_monitors();

	// we need the beam width in a float, contrary to what the core does.
	video_config.beamwidth = options_get_float(machine->options(), OPTION_BEAM);

	// initialize the window system so we can make windows
	if (sdlwindow_init(machine))
		goto error;

	tc = machine->total_colors();

	// create the windows
	for (index = 0; index < video_config.numscreens; index++)
	{
		sdl_window_config conf;
		memset(&conf, 0, sizeof(conf));
		extract_window_config(machine, index, &conf);
		conf.totalColors = tc;
		if (sdlwindow_video_window_create(machine, index, pick_monitor(index), &conf))
			goto error;
	}


	return 0;

error:
	return 1;
}


//============================================================
//  video_exit
//============================================================

static void video_exit(running_machine &machine)
{
	// free the overlay effect
	global_free(effect_bitmap);
	effect_bitmap = NULL;

	// free all of our monitor information
	while (sdl_monitor_list != NULL)
	{
		sdl_monitor_info *temp = sdl_monitor_list;
		sdl_monitor_list = temp->next;
		global_free(temp);
	}

}


//============================================================
//  sdlvideo_monitor_refresh
//============================================================

void sdlvideo_monitor_refresh(sdl_monitor_info *monitor)
{
	#if (SDL_VERSION_ATLEAST(1,3,0))
	SDL_DisplayMode dmode;

	SDL_SelectVideoDisplay(monitor->handle);
	SDL_GetDesktopDisplayMode(&dmode);
	monitor->monitor_width = dmode.w;
	monitor->monitor_height = dmode.h;
	monitor->center_width = dmode.w;
	monitor->center_height = dmode.h;
	#else
	#if defined(SDLMAME_WIN32)	// Win32 version
	MONITORINFOEX info;
	info.cbSize = sizeof(info);
    GetMonitorInfo((HMONITOR)monitor->handle, (LPMONITORINFO)&info);
	monitor->center_width = monitor->monitor_width = info.rcMonitor.right - info.rcMonitor.left;
	monitor->center_height = monitor->monitor_height = info.rcMonitor.bottom - info.rcMonitor.top;
	char *temp = utf8_from_wstring(info.szDevice);
	strcpy(monitor->monitor_device, temp);
	osd_free(temp);
	#elif defined(SDLMAME_MACOSX)	// Mac OS X Core Imaging version
	CGDirectDisplayID primary;
	CGRect dbounds;

	// get the main display
	primary = CGMainDisplayID();
	dbounds = CGDisplayBounds(primary);

	monitor->center_width = monitor->monitor_width = dbounds.size.width - dbounds.origin.x;
	monitor->center_height = monitor->monitor_height = dbounds.size.height - dbounds.origin.y;
	strcpy(monitor->monitor_device, "Mac OS X display");
    #elif defined(SDLMAME_X11) || defined(SDLMAME_NO_X11)       // X11 version
	{
		#if defined(SDLMAME_X11)
		// X11 version
		int screen;
		SDL_SysWMinfo info;
		SDL_VERSION(&info.version);

		if ( SDL_GetWMInfo(&info) && (info.subsystem == SDL_SYSWM_X11) )
		{
			screen = DefaultScreen(info.info.x11.display);
			SDL_VideoDriverName(monitor->monitor_device, sizeof(monitor->monitor_device)-1);
			monitor->monitor_width = DisplayWidth(info.info.x11.display, screen);
			monitor->monitor_height = DisplayHeight(info.info.x11.display, screen);

			if ((XineramaIsActive(info.info.x11.display)) && video_config.restrictonemonitor)
			{
				XineramaScreenInfo *xineinfo;
				int numscreens;

    				xineinfo = XineramaQueryScreens(info.info.x11.display, &numscreens);

				monitor->center_width = xineinfo[0].width;
				monitor->center_height = xineinfo[0].height;

				XFree(xineinfo);
			}
			else
			{
				monitor->center_width = monitor->monitor_width;
				monitor->center_height = monitor->monitor_height;
			}
		}
		else
		#endif // defined(SDLMAME_X11)
		{
			static int first_call=0;
			static int cw, ch;

			SDL_VideoDriverName(monitor->monitor_device, sizeof(monitor->monitor_device)-1);
			if (first_call==0)
			{
				char *dimstr = osd_getenv(SDLENV_DESKTOPDIM);
				const SDL_VideoInfo *sdl_vi;

				sdl_vi = SDL_GetVideoInfo();
				#if (SDL_VERSION_ATLEAST(1,2,10))
				cw = sdl_vi->current_w;
				ch = sdl_vi->current_h;
				#endif
				first_call=1;
				if ((cw==0) || (ch==0))
				{
					if (dimstr != NULL)
					{
						sscanf(dimstr, "%dx%d", &cw, &ch);
					}
					if ((cw==0) || (ch==0))
					{
						mame_printf_warning("WARNING: SDL_GetVideoInfo() for driver <%s> is broken.\n", monitor->monitor_device);
						mame_printf_warning("         You should set SDLMAME_DESKTOPDIM to your desktop size.\n");
						mame_printf_warning("            e.g. export SDLMAME_DESKTOPDIM=800x600\n");
						mame_printf_warning("         Assuming 1024x768 now!\n");
						cw=1024;
						ch=768;
					}
				}
			}
			monitor->monitor_width = cw;
			monitor->monitor_height = ch;
			monitor->center_width = cw;
			monitor->center_height = ch;
		}
	}
	#elif defined(SDLMAME_OS2)		// OS2 version
	monitor->center_width = monitor->monitor_width = WinQuerySysValue( HWND_DESKTOP, SV_CXSCREEN );
	monitor->center_height = monitor->monitor_height = WinQuerySysValue( HWND_DESKTOP, SV_CYSCREEN );
	strcpy(monitor->monitor_device, "OS/2 display");
	#else
	#error Unknown SDLMAME_xx OS type!
	#endif

	{
		static int info_shown=0;
		if (!info_shown)
		{
			mame_printf_verbose("SDL Device Driver     : %s\n", monitor->monitor_device);
			mame_printf_verbose("SDL Monitor Dimensions: %d x %d\n", monitor->monitor_width, monitor->monitor_height);
			info_shown = 1;
		}
	}
	#endif //  (SDL_VERSION_ATLEAST(1,3,0))
}



//============================================================
//  sdlvideo_monitor_get_aspect
//============================================================

float sdlvideo_monitor_get_aspect(sdl_monitor_info *monitor)
{
	// refresh the monitor information and compute the aspect
	if (video_config.keepaspect)
	{
		sdlvideo_monitor_refresh(monitor);
		return monitor->aspect / ((float)monitor->monitor_width / (float)monitor->monitor_height);
	}
	return 0.0f;
}



//============================================================
//  sdlvideo_monitor_from_handle
//============================================================

sdl_monitor_info *sdlvideo_monitor_from_handle(UINT32 hmonitor)
{
	sdl_monitor_info *monitor;

	// find the matching monitor
	for (monitor = sdl_monitor_list; monitor != NULL; monitor = monitor->next)
		if (monitor->handle == hmonitor)
			return monitor;
	return NULL;
}


//============================================================
//  osd_update
//============================================================

void osd_update(running_machine *machine, int skip_redraw)
{
	sdl_window_info *window;

	// if we're not skipping this redraw, update all windows
	if (!skip_redraw)
	{
//      profiler_mark(PROFILER_BLIT);
		for (window = sdl_window_list; window != NULL; window = window->next)
			sdlwindow_video_window_update(machine, window);
//      profiler_mark(PROFILER_END);
	}

	// poll the joystick values here
	sdlinput_poll(machine);
	check_osd_inputs(machine);

	if ((machine->debug_flags & DEBUG_FLAG_OSD_ENABLED) != 0)
		debugwin_update_during_game(machine);
}


//============================================================
//  add_primary_monitor
//============================================================

#if !defined(SDLMAME_WIN32) && !(SDL_VERSION_ATLEAST(1,3,0))
static void add_primary_monitor(void *data)
{
	sdl_monitor_info ***tailptr = (sdl_monitor_info ***)data;
	sdl_monitor_info *monitor;

	// allocate a new monitor info
	monitor = global_alloc_clear(sdl_monitor_info);

	// copy in the data
	monitor->handle = 1;

	sdlvideo_monitor_refresh(monitor);

	// guess the aspect ratio assuming square pixels
	monitor->aspect = (float)(monitor->monitor_width) / (float)(monitor->monitor_height);

	// save the primary monitor handle
	primary_monitor = monitor;

	// hook us into the list
	**tailptr = monitor;
	*tailptr = &monitor->next;
}
#endif


//============================================================
//  monitor_enum_callback
//============================================================

#if defined(SDLMAME_WIN32) && !(SDL_VERSION_ATLEAST(1,3,0))
static BOOL CALLBACK monitor_enum_callback(HMONITOR handle, HDC dc, LPRECT rect, LPARAM data)
{
	sdl_monitor_info ***tailptr = (sdl_monitor_info ***)data;
	sdl_monitor_info *monitor;
	MONITORINFOEX info;
	BOOL result;

	// get the monitor info
	info.cbSize = sizeof(info);
	result = GetMonitorInfo(handle, (LPMONITORINFO)&info);
	assert(result);

	// allocate a new monitor info
	monitor = global_alloc_clear(sdl_monitor_info);

	// copy in the data

#ifdef PTR64
    monitor->handle = (UINT64)handle;
#else
    monitor->handle = (UINT32)handle;
#endif
	monitor->monitor_width = info.rcMonitor.right - info.rcMonitor.left;
	monitor->monitor_height = info.rcMonitor.bottom - info.rcMonitor.top;
	char *temp = utf8_from_wstring(info.szDevice);
	strcpy(monitor->monitor_device, temp);
	osd_free(temp);

	// guess the aspect ratio assuming square pixels
	monitor->aspect = (float)(info.rcMonitor.right - info.rcMonitor.left) / (float)(info.rcMonitor.bottom - info.rcMonitor.top);

	// save the primary monitor handle
	if (info.dwFlags & MONITORINFOF_PRIMARY)
		primary_monitor = monitor;

	// hook us into the list
	**tailptr = monitor;
	*tailptr = &monitor->next;

	// enumerate all the available monitors so to list their names in verbose mode
	return TRUE;
}
#endif


//============================================================
//  init_monitors
//============================================================

static void init_monitors(void)
{
	sdl_monitor_info **tailptr;

	// make a list of monitors
	sdl_monitor_list = NULL;
	tailptr = &sdl_monitor_list;

	#if (SDL_VERSION_ATLEAST(1,3,0))
	{
		int i, temp;

		mame_printf_verbose("Enter init_monitors\n");
		temp = SDL_GetCurrentVideoDisplay();

		for (i = 0; i < SDL_GetNumVideoDisplays(); i++)
		{
			sdl_monitor_info *monitor;
			SDL_DisplayMode dmode;

			// allocate a new monitor info
			monitor = global_alloc_clear(sdl_monitor_info);

			snprintf(monitor->monitor_device, sizeof(monitor->monitor_device)-1, "%s%d", SDLOPTION_SCREEN(""),i);

			SDL_SelectVideoDisplay(i);
			SDL_GetDesktopDisplayMode(&dmode);
			monitor->monitor_width = dmode.w;
			monitor->monitor_height = dmode.h;
			monitor->center_width = dmode.w;
			monitor->center_height = dmode.h;
	        monitor->handle = i;
	        // guess the aspect ratio assuming square pixels
	        monitor->aspect = (float)(dmode.w) / (float)(dmode.h);
	        mame_printf_verbose("Adding monitor %s (%d x %d)\n", monitor->monitor_device, dmode.w, dmode.h);

	        // save the primary monitor handle
	        if (i == 0)
	        	primary_monitor = monitor;

			// hook us into the list
			*tailptr = monitor;
			tailptr = &monitor->next;
		}
		SDL_SelectVideoDisplay(temp);
	}
	mame_printf_verbose("Leave init_monitors\n");
	#elif defined(SDLMAME_WIN32)
	EnumDisplayMonitors(NULL, NULL, monitor_enum_callback, (LPARAM)&tailptr);
	#else
	add_primary_monitor((void *)&tailptr);
	#endif
}


//============================================================
//  pick_monitor
//============================================================

#if (SDL_VERSION_ATLEAST(1,3,0)) || defined(SDLMAME_WIN32)
static sdl_monitor_info *pick_monitor(int index)
{
	sdl_monitor_info *monitor;
	const char *scrname;
	int moncount = 0;
	char option[20];
	float aspect;

	// get the screen option
	sprintf(option, SDLOPTION_SCREEN("%d"), index);
	scrname = options_get_string(mame_options(), option);

	// get the aspect ratio
	sprintf(option, SDLOPTION_ASPECT("%d"), index);
	aspect = get_aspect(option, TRUE);

	// look for a match in the name first
	if (scrname != NULL)
		for (monitor = sdl_monitor_list; monitor != NULL; monitor = monitor->next)
		{
			moncount++;
			if (strcmp(scrname, monitor->monitor_device) == 0)
				goto finishit;
		}

	// didn't find it; alternate monitors until we hit the jackpot
	index %= moncount;
    for (monitor = sdl_monitor_list; monitor != NULL; monitor = monitor->next)
		if (index-- == 0)
			goto finishit;

	// return the primary just in case all else fails
	monitor = primary_monitor;

finishit:
	if (aspect != 0)
	{
		monitor->aspect = aspect;
	}
	return monitor;
}
#else
static sdl_monitor_info *pick_monitor(int index)
{
	sdl_monitor_info *monitor;
	char option[20];
	float aspect;

	// get the aspect ratio
	sprintf(option, SDLOPTION_ASPECT("%d"), index);
	aspect = get_aspect(option, TRUE);

	// return the primary just in case all else fails
	monitor = primary_monitor;

	if (aspect != 0)
	{
		monitor->aspect = aspect;
	}
	return monitor;
}
#endif


//============================================================
//  check_osd_inputs
//============================================================

static void check_osd_inputs(running_machine *machine)
{
	sdl_window_info *window = sdlinput_get_focus_window(machine);

	// check for toggling fullscreen mode
	if (ui_input_pressed(machine, IPT_OSD_1))
		sdlwindow_toggle_full_screen(machine, window);

	if (ui_input_pressed(machine, IPT_OSD_2))
	{
		//FIXME: on a per window basis
		video_config.fullstretch = !video_config.fullstretch;
		ui_popup_time(1, "Uneven stretch %s", video_config.fullstretch? "enabled":"disabled");
	}

	if (ui_input_pressed(machine, IPT_OSD_4))
	{
		//FIXME: on a per window basis
		video_config.keepaspect = !video_config.keepaspect;
		ui_popup_time(1, "Keepaspect %s", video_config.keepaspect? "enabled":"disabled");
	}

	if (USE_OPENGL || SDL_VERSION_ATLEAST(1,3,0))
	{
		//FIXME: on a per window basis
		if (ui_input_pressed(machine, IPT_OSD_5))
		{
			video_config.filter = !video_config.filter;
			ui_popup_time(1, "Filter %s", video_config.filter? "enabled":"disabled");
		}
	}

	if (ui_input_pressed(machine, IPT_OSD_6))
		sdlwindow_modify_prescale(machine, window, -1);

	if (ui_input_pressed(machine, IPT_OSD_7))
		sdlwindow_modify_prescale(machine, window, 1);
}

//============================================================
//  extract_window_config
//============================================================

static void extract_window_config(running_machine *machine, int index, sdl_window_config *conf)
{
	char buf[20];

	sprintf(buf,SDLOPTION_RESOLUTION("%d"), index);
	// per-window options: extract the data
	get_resolution(buf, conf, TRUE);
}

//============================================================
//  extract_video_config
//============================================================

static void extract_video_config(running_machine *machine)
{
	const char *stemp;

	video_config.perftest    = options_get_bool(machine->options(), SDLOPTION_SDLVIDEOFPS);

	// global options: extract the data
	video_config.windowed      = options_get_bool(machine->options(), SDLOPTION_WINDOW);
	video_config.keepaspect    = options_get_bool(machine->options(), SDLOPTION_KEEPASPECT);
	video_config.numscreens    = options_get_int(machine->options(), SDLOPTION_NUMSCREENS);
	video_config.fullstretch   = options_get_bool(machine->options(), SDLOPTION_UNEVENSTRETCH);
	#ifdef SDLMAME_X11
	video_config.restrictonemonitor = !options_get_bool(machine->options(), SDLOPTION_USEALLHEADS);
	#endif


	if (machine->debug_flags & DEBUG_FLAG_OSD_ENABLED)
		video_config.windowed = TRUE;

	stemp = options_get_string(machine->options(), SDLOPTION_EFFECT);
	if (stemp != NULL && strcmp(stemp, "none") != 0)
		load_effect_overlay(machine, stemp);

	// default to working video please
	video_config.novideo = 0;

	// d3d options: extract the data
	stemp = options_get_string(machine->options(), SDLOPTION_VIDEO);
	if (strcmp(stemp, SDLOPTVAL_SOFT) == 0)
		video_config.mode = VIDEO_MODE_SOFT;
	else if (strcmp(stemp, SDLOPTVAL_NONE) == 0)
	{
		video_config.mode = VIDEO_MODE_SOFT;
		video_config.novideo = 1;

		if (options_get_int(machine->options(), OPTION_SECONDS_TO_RUN) == 0)
			mame_printf_warning("Warning: -video none doesn't make much sense without -seconds_to_run\n");
	}
	else if (USE_OPENGL && (strcmp(stemp, SDLOPTVAL_OPENGL) == 0))
		video_config.mode = VIDEO_MODE_OPENGL;
	else if (USE_OPENGL && (strcmp(stemp, SDLOPTVAL_OPENGL16) == 0))
	{
		video_config.mode = VIDEO_MODE_OPENGL;
		video_config.prefer16bpp_tex = 1;
	}
	else if (SDL_VERSION_ATLEAST(1,3,0) && (strcmp(stemp, SDLOPTVAL_SDL13) == 0))
	{
		video_config.mode = VIDEO_MODE_SDL13;
		video_config.prefer16bpp_tex = 1;
	}
	else
	{
		mame_printf_warning("Invalid video value %s; reverting to software\n", stemp);
		video_config.mode = VIDEO_MODE_SOFT;
	}

	video_config.switchres     = options_get_bool(machine->options(), SDLOPTION_SWITCHRES);
	video_config.centerh       = options_get_bool(machine->options(), SDLOPTION_CENTERH);
	video_config.centerv       = options_get_bool(machine->options(), SDLOPTION_CENTERV);
	video_config.waitvsync     = options_get_bool(machine->options(), SDLOPTION_WAITVSYNC);

	if (USE_OPENGL || SDL_VERSION_ATLEAST(1,3,0))
	{
		video_config.filter        = options_get_bool(machine->options(), SDLOPTION_FILTER);
	}

	if (USE_OPENGL)
	{
		video_config.prescale      = options_get_int(machine->options(), SDLOPTION_PRESCALE);
		if (video_config.prescale < 1 || video_config.prescale > 3)
		{
			mame_printf_warning("Invalid prescale option, reverting to '1'\n");
			video_config.prescale = 1;
		}
		// default to working video please
		video_config.prefer16bpp_tex = 0;
		video_config.forcepow2texture = options_get_bool(machine->options(), SDLOPTION_GL_FORCEPOW2TEXTURE)==1;
		video_config.allowtexturerect = options_get_bool(machine->options(), SDLOPTION_GL_NOTEXTURERECT)==0;
		video_config.vbo         = options_get_bool(machine->options(), SDLOPTION_GL_VBO);
		video_config.pbo         = options_get_bool(machine->options(), SDLOPTION_GL_PBO);
		video_config.glsl        = options_get_bool(machine->options(), SDLOPTION_GL_GLSL);
		if ( video_config.glsl )
		{
			int i;
			static char buffer[20]; // gl_glsl_filter[0..9]?

			video_config.glsl_filter = options_get_int (machine->options(), SDLOPTION_GLSL_FILTER);

			video_config.glsl_shader_mamebm_num=0;

			for(i=0; i<GLSL_SHADER_MAX; i++)
			{
				snprintf(buffer, 18, SDLOPTION_SHADER_MAME("%d"), i); buffer[17]=0;

				stemp = options_get_string(machine->options(), buffer);
				if (stemp && strcmp(stemp, SDLOPTVAL_NONE) != 0 && strlen(stemp)>0)
				{
					video_config.glsl_shader_mamebm[i] = (char *) malloc(strlen(stemp)+1);
					strcpy(video_config.glsl_shader_mamebm[i], stemp);
					video_config.glsl_shader_mamebm_num++;
				} else {
					video_config.glsl_shader_mamebm[i] = NULL;
				}
			}

			video_config.glsl_shader_scrn_num=0;

			for(i=0; i<GLSL_SHADER_MAX; i++)
			{
				snprintf(buffer, 20, SDLOPTION_SHADER_SCREEN("%d"), i); buffer[19]=0;

				stemp = options_get_string(machine->options(), buffer);
				if (stemp && strcmp(stemp, SDLOPTVAL_NONE) != 0 && strlen(stemp)>0)
				{
					video_config.glsl_shader_scrn[i] = (char *) malloc(strlen(stemp)+1);
					strcpy(video_config.glsl_shader_scrn[i], stemp);
					video_config.glsl_shader_scrn_num++;
				} else {
					video_config.glsl_shader_scrn[i] = NULL;
				}
			}

			video_config.glsl_vid_attributes = options_get_int (machine->options(), SDLOPTION_GL_GLSL_VID_ATTR);
			{
				// Disable feature: glsl_vid_attributes, as long we have the gamma calculation
				// disabled within the direct shaders .. -> too slow.
				// IMHO the gamma setting should be done global anyways, and for the whole system,
				// not just MAME ..
				float gamma = options_get_float(machine->options(), OPTION_GAMMA);
				if (gamma != 1.0 && video_config.glsl_vid_attributes && video_config.glsl)
				{
					video_config.glsl_vid_attributes = FALSE;
					mame_printf_warning("OpenGL: GLSL - disable handling of brightness and contrast, gamma is set to %f\n", gamma);
				}
			}
		} else {
			int i;
			video_config.glsl_filter = 0;
			video_config.glsl_shader_mamebm_num=0;
			for(i=0; i<GLSL_SHADER_MAX; i++)
			{
				video_config.glsl_shader_mamebm[i] = NULL;
			}
			video_config.glsl_shader_scrn_num=0;
			for(i=0; i<GLSL_SHADER_MAX; i++)
			{
				video_config.glsl_shader_scrn[i] = NULL;
			}
			video_config.glsl_vid_attributes = 0;
		}

	}
	// misc options: sanity check values

	// global options: sanity check values
#if (!SDL_VERSION_ATLEAST(1,3,0))
	if (video_config.numscreens < 1 || video_config.numscreens > 1) //MAX_VIDEO_WINDOWS)
	{
		mame_printf_warning("Invalid numscreens value %d; reverting to 1\n", video_config.numscreens);
		video_config.numscreens = 1;
	}
#else
	if (video_config.numscreens < 1 || video_config.numscreens > MAX_VIDEO_WINDOWS)
	{
		mame_printf_warning("Invalid numscreens value %d; reverting to 1\n", video_config.numscreens);
		video_config.numscreens = 1;
	}
#endif
	// yuv settings ...
	stemp = options_get_string(machine->options(), SDLOPTION_SCALEMODE);
	video_config.scale_mode = drawsdl_scale_mode(stemp);
	if (video_config.scale_mode < 0)
	{
		mame_printf_warning("Invalid yuvmode value %s; reverting to none\n", stemp);
		video_config.scale_mode = VIDEO_SCALE_MODE_NONE;
	}
	if ( (video_config.mode != VIDEO_MODE_SOFT) && (video_config.scale_mode != VIDEO_SCALE_MODE_NONE) )
	{
		mame_printf_warning("scalemode is only for -video soft, overriding\n");
		video_config.scale_mode = VIDEO_SCALE_MODE_NONE;
	}
}


//============================================================
//  load_effect_overlay
//============================================================

static void load_effect_overlay(running_machine *machine, const char *filename)
{
	char *tempstr = global_alloc_array(char, strlen(filename) + 5);
	char *dest;

	// append a .PNG extension
	strcpy(tempstr, filename);
	dest = strrchr(tempstr, '.');
	if (dest == NULL)
		dest = &tempstr[strlen(tempstr)];
	strcpy(dest, ".png");

	// load the file
	effect_bitmap = render_load_png(OPTION_ARTPATH, NULL, tempstr, NULL, NULL);
	if (effect_bitmap == NULL)
	{
		mame_printf_error("Unable to load PNG file '%s'\n", tempstr);
		global_free(tempstr);
		return;
	}

	// set the overlay on all screens
	for (screen_device *screen = screen_first(*machine); screen != NULL; screen = screen_next(screen))
		render_container_set_overlay(render_container_get_screen(screen), effect_bitmap);

	global_free(tempstr);
}


//============================================================
//  get_aspect
//============================================================

static float get_aspect(const char *name, int report_error)
{
	const char *defdata = options_get_string(mame_options(), SDLOPTION_ASPECT(""));
	const char *data = options_get_string(mame_options(), name);
	int num = 0, den = 1;

	if (strcmp(data, SDLOPTVAL_AUTO) == 0)
	{
		if (strcmp(defdata,SDLOPTVAL_AUTO) == 0)
			return 0;
		data = defdata;
	}
	if (sscanf(data, "%d:%d", &num, &den) != 2 && report_error)
		mame_printf_error("Illegal aspect ratio value for %s = %s\n", name, data);
	return (float)num / (float)den;
}


//============================================================
//  get_resolution
//============================================================

static void get_resolution(const char *name, sdl_window_config *config, int report_error)
{
	const char *defdata = options_get_string(mame_options(), SDLOPTION_RESOLUTION(""));
	const char *data = options_get_string(mame_options(), name);

	config->width = config->height = config->depth = config->refresh = 0;
	if (strcmp(data, SDLOPTVAL_AUTO) == 0)
	{
		if (strcmp(defdata, SDLOPTVAL_AUTO) == 0)
		{
			return;
		}

		data = defdata;
	}

	if (sscanf(data, "%dx%dx%d@%d", &config->width, &config->height, &config->depth, &config->refresh) < 2 && report_error)
		mame_printf_error("Illegal resolution value for %s = %s\n", name, data);
}

