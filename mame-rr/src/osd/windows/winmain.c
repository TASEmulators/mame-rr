//============================================================
//
//  winmain.c - Win32 main program
//
//============================================================
//
//  Copyright Aaron Giles
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or
//  without modification, are permitted provided that the
//  following conditions are met:
//
//    * Redistributions of source code must retain the above
//      copyright notice, this list of conditions and the
//      following disclaimer.
//    * Redistributions in binary form must reproduce the
//      above copyright notice, this list of conditions and
//      the following disclaimer in the documentation and/or
//      other materials provided with the distribution.
//    * Neither the name 'MAME' nor the names of its
//      contributors may be used to endorse or promote
//      products derived from this software without specific
//      prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND
//  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
//  FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
//  EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
//  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//  DAMAGE (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
//  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
//  IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//============================================================

// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <mmsystem.h>
#include <tchar.h>

// standard C headers
#include <ctype.h>
#include <stdarg.h>
#include <psapi.h>
#include <dbghelp.h>

// MAME headers
#include "emu.h"
#include "clifront.h"
#include "emuopts.h"

// MAMEOS headers
#include "winmain.h"
#include "window.h"
#include "video.h"
#include "sound.h"
#include "input.h"
#include "output.h"
#include "config.h"
#include "osdepend.h"
#include "strconv.h"
#include "winutf8.h"
#include "winutil.h"
#include "debugger.h"

#define DEBUG_SLOW_LOCKS	0


//**************************************************************************
//  MACROS
//**************************************************************************

#ifdef UNICODE
#define UNICODE_POSTFIX "W"
#else
#define UNICODE_POSTFIX "A"
#endif



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

template<typename func_ptr>
class dynamic_bind
{
public:
	// constructor which looks up the function
	dynamic_bind(const TCHAR *dll, const char *symbol)
		: m_function(NULL)
	{
		HMODULE module = LoadLibrary(dll);
		if (module != NULL)
			m_function = reinterpret_cast<func_ptr>(GetProcAddress(module, symbol));
	}

	// bool to test if the function is NULL or not
	operator bool() const { return (m_function != NULL); }

	// dereference to get the underlying pointer
	func_ptr operator *() const { return m_function; }

private:
	func_ptr	m_function;
};


class stack_walker
{
public:
	stack_walker();

	FPTR ip() const { return m_stackframe.AddrPC.Offset; }
	FPTR sp() const { return m_stackframe.AddrStack.Offset; }
	FPTR frame() const { return m_stackframe.AddrFrame.Offset; }

	void reset(CONTEXT &context, HANDLE thread);
	bool unwind();

private:
	HANDLE			m_process;
	HANDLE			m_thread;
	STACKFRAME64	m_stackframe;
	CONTEXT			m_context;
	bool			m_first;

	dynamic_bind<BOOL (WINAPI *)(DWORD, HANDLE, HANDLE, LPSTACKFRAME64, PVOID, PREAD_PROCESS_MEMORY_ROUTINE64, PFUNCTION_TABLE_ACCESS_ROUTINE64, PGET_MODULE_BASE_ROUTINE64, PTRANSLATE_ADDRESS_ROUTINE64)>
			m_stack_walk_64;
	dynamic_bind<BOOL (WINAPI *)(HANDLE, LPCTSTR, BOOL)> m_sym_initialize;
	dynamic_bind<PVOID (WINAPI *)(HANDLE, DWORD64)> m_sym_function_table_access_64;
	dynamic_bind<DWORD64 (WINAPI *)(HANDLE, DWORD64)> m_sym_get_module_base_64;

	static bool		s_initialized;
};


class symbol_manager
{
public:
	// construction/destruction
	symbol_manager(const char *argv0);
	~symbol_manager();

	// getters
	FPTR last_base() const { return m_last_base; }

	// core symbol lookup
	const char *symbol_for_address(FPTR address);
	const char *symbol_for_address(PVOID address) { return symbol_for_address(reinterpret_cast<FPTR>(address)); }

	// force symbols to be cached
	void cache_symbols() { scan_file_for_address(0, true); }

private:
	// internal helpers
	bool query_system_for_address(FPTR address);
	void scan_file_for_address(FPTR address, bool create_cache);
	bool parse_sym_line(const char *line, FPTR &address, astring &symbol);
	bool parse_map_line(const char *line, FPTR &address, astring &symbol);
	void scan_cache_for_address(FPTR address);
	void format_symbol(const char *name, UINT32 displacement, const char *filename = NULL, int linenumber = 0);

	static FPTR get_text_section_base();

	struct cache_entry
	{
		cache_entry(FPTR address, const char *symbol) : m_next(NULL), m_address(address), m_name(symbol) { }
		cache_entry *	m_next;
		FPTR			m_address;
		astring			m_name;
	};
	cache_entry *	m_cache;

	astring			m_mapfile;
	astring			m_symfile;
	astring			m_buffer;
	HANDLE			m_process;
	FPTR			m_last_base;
	FPTR			m_text_base;

	dynamic_bind<BOOL (WINAPI *)(HANDLE, DWORD64, PDWORD64, PSYMBOL_INFO)> m_sym_from_addr;
	dynamic_bind<BOOL (WINAPI *)(HANDLE, DWORD64, PDWORD, PIMAGEHLP_LINE64)> m_sym_get_line_from_addr_64;
};


class sampling_profiler
{
public:
	sampling_profiler(UINT32 max_seconds, UINT8 stack_depth);
	~sampling_profiler();

	void start();
	void stop();

//  void reset();
	void print_results(symbol_manager &symbols);

private:
	static DWORD WINAPI thread_entry(LPVOID lpParameter);
	void thread_run();

	static int CLIB_DECL compare_address(const void *item1, const void *item2);
	static int CLIB_DECL compare_frequency(const void *item1, const void *item2);

	HANDLE			m_target_thread;

	HANDLE			m_thread;
	DWORD			m_thread_id;
	volatile bool	m_thread_exit;

	UINT8			m_stack_depth;
	UINT8			m_entry_stride;
	UINT32			m_max_seconds;
	FPTR *			m_buffer;
	FPTR *			m_buffer_ptr;
	FPTR *			m_buffer_end;
};




//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// this line prevents globbing on the command line
int _CRT_glob = 0;



//**************************************************************************
//  LOCAL VARIABLES
//**************************************************************************

//static dynamic_bind<HANDLE (WINAPI *)(LPCTSTR, LPDWORD)> av_set_mm_thread_characteristics(TEXT("avrt.dll"), "AvSetMmThreadCharacteristics" UNICODE_POSTFIX);
//static dynamic_bind<HANDLE (WINAPI *)(LPCTSTR, LPCTSTR, LPDWORD)> av_set_mm_max_thread_characteristics(TEXT("avrt.dll"), "AvSetMmMaxThreadCharacteristics" UNICODE_POSTFIX);
//static dynamic_bind<BOOL (WINAPI *)(HANDLE)> av_revert_mm_thread_characteristics(TEXT("avrt.dll"), "AvRevertMmThreadCharacteristics");


static LPTOP_LEVEL_EXCEPTION_FILTER pass_thru_filter;

static HANDLE watchdog_reset_event;
static HANDLE watchdog_exit_event;
static HANDLE watchdog_thread;


#ifndef MESS
static const TCHAR helpfile[] = TEXT("docs\\windows.txt");
#else
static const TCHAR helpfile[] = TEXT("mess.chm");
#endif

//static HANDLE mm_task = NULL;
//static DWORD task_index = 0;
static int timeresult;
//static MMRESULT result;
static TIMECAPS caps;

static sampling_profiler *profiler = NULL;
static symbol_manager *symbols = NULL;

bool stack_walker::s_initialized = false;


//**************************************************************************
//  FUNCTION PROTOTYPES
//**************************************************************************

static void osd_exit(running_machine &machine);

static int is_double_click_start(int argc);
static DWORD WINAPI watchdog_thread_entry(LPVOID lpParameter);
static LONG WINAPI exception_filter(struct _EXCEPTION_POINTERS *info);
static void winui_output_error(void *param, const char *format, va_list argptr);



//**************************************************************************
//  OPTIONS
//**************************************************************************

// struct definitions
const options_entry mame_win_options[] =
{
	// debugging options
	{ NULL,                       NULL,       OPTION_HEADER,     "WINDOWS DEBUGGING OPTIONS" },
	{ "oslog",                    "0",        OPTION_BOOLEAN,    "output error.log data to the system debugger" },
	{ "watchdog;wdog",            "0",        0,                 "force the program to terminate if no updates within specified number of seconds" },
	{ "debugger_font;dfont",      "Lucida Console", 0,           "specifies the font to use for debugging; defaults to Lucida Console" },
	{ "debugger_font_size;dfontsize", "9",    0,                 "specifies the font size to use for debugging; defaults to 9 pt" },

	// performance options
	{ NULL,                       NULL,       OPTION_HEADER,     "WINDOWS PERFORMANCE OPTIONS" },
	{ "priority(-15-1)",          "0",        0,                 "thread priority for the main game thread; range from -15 to 1" },
	{ "multithreading;mt",        "0",        OPTION_BOOLEAN,    "enable multithreading; this enables rendering and blitting on a separate thread" },
	{ "numprocessors;np",         "auto",     0,				 "number of processors; this overrides the number the system reports" },
	{ "profile",                  "0",        0,                 "enable profiling, specifying the stack depth to track" },

	// video options
	{ NULL,                       NULL,       OPTION_HEADER,     "WINDOWS VIDEO OPTIONS" },
	{ "video",                    "d3d",      0,                 "video output method: none, gdi, ddraw, or d3d" },
	{ "numscreens(1-4)",          "1",        0,                 "number of screens to create; usually, you want just one" },
	{ "window;w",                 "1",        OPTION_BOOLEAN,    "enable window mode; otherwise, full screen mode is assumed" },
	{ "maximize;max",             "0",        OPTION_BOOLEAN,    "default to maximized windows; otherwise, windows will be minimized" },
	{ "keepaspect;ka",            "1",        OPTION_BOOLEAN,    "constrain to the proper aspect ratio" },
	{ "prescale",                 "1",        0,                 "scale screen rendering by this amount in software" },
	{ "effect",                   "none",     0,                 "name of a PNG file to use for visual effects, or 'none'" },
	{ "waitvsync",                "0",        OPTION_BOOLEAN,    "enable waiting for the start of VBLANK before flipping screens; reduces tearing effects" },
	{ "syncrefresh",              "0",        OPTION_BOOLEAN,    "enable using the start of VBLANK for throttling instead of the game time" },

	// DirectDraw-specific options
	{ NULL,                       NULL,       OPTION_HEADER,     "DIRECTDRAW-SPECIFIC OPTIONS" },
	{ "hwstretch;hws",            "1",        OPTION_BOOLEAN,    "enable hardware stretching" },

	// Direct3D-specific options
	{ NULL,                       NULL,       OPTION_HEADER,     "DIRECT3D-SPECIFIC OPTIONS" },
	{ "d3dversion(8-9)",          "9",        0,                 "specify the preferred Direct3D version (8 or 9)" },
	{ "filter;d3dfilter;flt",     "1",        OPTION_BOOLEAN,    "enable bilinear filtering on screen output" },

	// per-window options
	{ NULL,                       NULL,       OPTION_HEADER,     "PER-WINDOW VIDEO OPTIONS" },
	{ "screen",                   "auto",     0,                 "explicit name of all screens; 'auto' here will try to make a best guess" },
	{ "aspect;screen_aspect",     "auto",     0,                 "aspect ratio for all screens; 'auto' here will try to make a best guess" },
	{ "resolution;r",             "auto",     0,                 "preferred resolution for all screens; format is <width>x<height>[@<refreshrate>] or 'auto'" },
	{ "view",                     "auto",     0,                 "preferred view for all screens" },

	{ "screen0",                  "auto",     0,                 "explicit name of the first screen; 'auto' here will try to make a best guess" },
	{ "aspect0",                  "auto",     0,                 "aspect ratio of the first screen; 'auto' here will try to make a best guess" },
	{ "resolution0;r0",           "auto",     0,                 "preferred resolution of the first screen; format is <width>x<height>[@<refreshrate>] or 'auto'" },
	{ "view0",                    "auto",     0,                 "preferred view for the first screen" },

	{ "screen1",                  "auto",     0,                 "explicit name of the second screen; 'auto' here will try to make a best guess" },
	{ "aspect1",                  "auto",     0,                 "aspect ratio of the second screen; 'auto' here will try to make a best guess" },
	{ "resolution1;r1",           "auto",     0,                 "preferred resolution of the second screen; format is <width>x<height>[@<refreshrate>] or 'auto'" },
	{ "view1",                    "auto",     0,                 "preferred view for the second screen" },

	{ "screen2",                  "auto",     0,                 "explicit name of the third screen; 'auto' here will try to make a best guess" },
	{ "aspect2",                  "auto",     0,                 "aspect ratio of the third screen; 'auto' here will try to make a best guess" },
	{ "resolution2;r2",           "auto",     0,                 "preferred resolution of the third screen; format is <width>x<height>[@<refreshrate>] or 'auto'" },
	{ "view2",                    "auto",     0,                 "preferred view for the third screen" },

	{ "screen3",                  "auto",     0,                 "explicit name of the fourth screen; 'auto' here will try to make a best guess" },
	{ "aspect3",                  "auto",     0,                 "aspect ratio of the fourth screen; 'auto' here will try to make a best guess" },
	{ "resolution3;r3",           "auto",     0,                 "preferred resolution of the fourth screen; format is <width>x<height>[@<refreshrate>] or 'auto'" },
	{ "view3",                    "auto",     0,                 "preferred view for the fourth screen" },

	// full screen options
	{ NULL,                       NULL,       OPTION_HEADER,     "FULL SCREEN OPTIONS" },
	{ "triplebuffer;tb",          "0",        OPTION_BOOLEAN,    "enable triple buffering" },
	{ "switchres",                "0",        OPTION_BOOLEAN,    "enable resolution switching" },
	{ "full_screen_brightness;fsb(0.1-2.0)","1.0",     0,        "brightness value in full screen mode" },
	{ "full_screen_contrast;fsc(0.1-2.0)", "1.0",      0,        "contrast value in full screen mode" },
	{ "full_screen_gamma;fsg(0.1-3.0)",    "1.0",      0,        "gamma value in full screen mode" },

	// sound options
	{ NULL,                       NULL,       OPTION_HEADER,     "WINDOWS SOUND OPTIONS" },
	{ "audio_latency(1-5)",       "2",        0,                 "set audio latency (increase to reduce glitches)" },

	// input options
	{ NULL,                       NULL,       OPTION_HEADER,     "INPUT DEVICE OPTIONS" },
	{ "dual_lightgun;dual",       "0",        OPTION_BOOLEAN,    "enable dual lightgun input" },

	{ NULL }
};



//**************************************************************************
//  MAIN ENTRY POINT
//**************************************************************************


//============================================================
//  utf8_main
//============================================================

int main(int argc, char *argv[])
{
	// initialize common controls
	InitCommonControls();

	// allocate symbols
	symbol_manager local_symbols(argv[0]);
	symbols = &local_symbols;

	// set up exception handling
	pass_thru_filter = SetUnhandledExceptionFilter(exception_filter);
	SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);

	// if we're a GUI app, out errors to message boxes
	if (win_is_gui_application() || is_double_click_start(argc))
	{
		// if we are a GUI app, output errors to message boxes
		mame_set_output_channel(OUTPUT_CHANNEL_ERROR, winui_output_error, NULL, NULL, NULL);

		// make sure any console window that opened on our behalf is nuked
		FreeConsole();
	}

	// parse config and cmdline options
	DWORD result = cli_execute(argc, argv, mame_win_options);

	// free symbols
	symbols = NULL;
	return result;
}


//============================================================
//  winui_output_error
//============================================================

static void winui_output_error(void *param, const char *format, va_list argptr)
{
	char buffer[1024];

	// if we are in fullscreen mode, go to windowed mode
	if ((video_config.windowed == 0) && (win_window_list != NULL))
		winwindow_toggle_full_screen();

	vsnprintf(buffer, ARRAY_LENGTH(buffer), format, argptr);
	win_message_box_utf8(win_window_list ? win_window_list->hwnd : NULL, buffer, APPNAME, MB_OK);
}



//============================================================
//  output_oslog
//============================================================

static void output_oslog(running_machine &machine, const char *buffer)
{
	win_output_debug_string_utf8(buffer);
}


//============================================================
//  osd_init
//============================================================

void osd_init(running_machine *machine)
{
	const char *stemp;

	// determine if we are profiling, and adjust options appropriately
	int profile = options_get_int(machine->options(), WINOPTION_PROFILE);
	if (profile > 0)
	{
		options_set_bool(machine->options(), OPTION_THROTTLE, false, OPTION_PRIORITY_MAXIMUM);
		options_set_bool(machine->options(), WINOPTION_MULTITHREADING, false, OPTION_PRIORITY_MAXIMUM);
		options_set_bool(machine->options(), WINOPTION_NUMPROCESSORS, 1, OPTION_PRIORITY_MAXIMUM);
	}

	// thread priority
	if (!(machine->debug_flags & DEBUG_FLAG_OSD_ENABLED))
		SetThreadPriority(GetCurrentThread(), options_get_int(machine->options(), WINOPTION_PRIORITY));

	// ensure we get called on the way out
	machine->add_notifier(MACHINE_NOTIFY_EXIT, osd_exit);

	// get number of processors
	stemp = options_get_string(machine->options(), WINOPTION_NUMPROCESSORS);

	osd_num_processors = 0;

	if (strcmp(stemp, "auto") != 0)
	{
		osd_num_processors = atoi(stemp);
		if (osd_num_processors < 1)
		{
			mame_printf_warning("Warning: numprocessors < 1 doesn't make much sense. Assuming auto ...\n");
			osd_num_processors = 0;
		}
	}

	// initialize the subsystems
	winvideo_init(machine);
	winsound_init(machine);
	wininput_init(machine);
	winoutput_init(machine);

	// notify listeners of screen configuration
	astring tempstring;
	for (win_window_info *info = win_window_list; info != NULL; info = info->next)
	{
		tempstring.printf("Orientation(%s)", utf8_from_tstring(info->monitor->info.szDevice));
		output_set_value(tempstring, info->targetorient);
	}

	// hook up the debugger log
	if (options_get_bool(machine->options(), WINOPTION_OSLOG))
		machine->add_logerror_callback(output_oslog);

	// crank up the multimedia timer resolution to its max
	// this gives the system much finer timeslices
	timeresult = timeGetDevCaps(&caps, sizeof(caps));
	if (timeresult == TIMERR_NOERROR)
		timeBeginPeriod(caps.wPeriodMin);

	// set our multimedia tasks if we can
//      if (av_set_mm_thread_characteristics != NULL)
//          mm_task = (*av_set_mm_thread_characteristics)(TEXT("Playback"), &task_index);

	// if a watchdog thread is requested, create one
	int watchdog = options_get_int(machine->options(), WINOPTION_WATCHDOG);
	if (watchdog != 0)
	{
		watchdog_reset_event = CreateEvent(NULL, FALSE, FALSE, NULL);
		assert_always(watchdog_reset_event != NULL, "Failed to create watchdog reset event");
		watchdog_exit_event = CreateEvent(NULL, TRUE, FALSE, NULL);
		assert_always(watchdog_exit_event != NULL, "Failed to create watchdog exit event");
		watchdog_thread = CreateThread(NULL, 0, watchdog_thread_entry, (LPVOID)(FPTR)watchdog, 0, NULL);
		assert_always(watchdog_thread != NULL, "Failed to create watchdog thread");
	}

	// create and start the profiler
	if (profile > 0)
	{
		profiler = global_alloc(sampling_profiler(1000, profile - 1));
		profiler->start();
	}
}


//============================================================
//  osd_exit
//============================================================

static void osd_exit(running_machine &machine)
{
	// take down the watchdog thread if it exists
	if (watchdog_thread != NULL)
	{
		SetEvent(watchdog_exit_event);
		WaitForSingleObject(watchdog_thread, INFINITE);
		CloseHandle(watchdog_reset_event);
		CloseHandle(watchdog_exit_event);
		CloseHandle(watchdog_thread);
		watchdog_reset_event = NULL;
		watchdog_exit_event = NULL;
		watchdog_thread = NULL;
	}

	// stop the profiler
	if (profiler != NULL)
	{
		profiler->stop();
		profiler->print_results(*symbols);
		global_free(profiler);
	}

	// turn off our multimedia tasks
//      if (av_revert_mm_thread_characteristics)
//          (*av_revert_mm_thread_characteristics)(mm_task);

	// restore the timer resolution
	if (timeresult == TIMERR_NOERROR)
		timeEndPeriod(caps.wPeriodMin);

	// one last pass at events
	winwindow_process_events(&machine, 0);
}


//============================================================
//  check_for_double_click_start
//============================================================

static int is_double_click_start(int argc)
{
	STARTUPINFO startup_info = { sizeof(STARTUPINFO) };

	// determine our startup information
	GetStartupInfo(&startup_info);

	// try to determine if MAME was simply double-clicked
	return (argc <= 1 && startup_info.dwFlags && !(startup_info.dwFlags & STARTF_USESTDHANDLES));
}


//============================================================
//  watchdog_thread_entry
//============================================================

static DWORD WINAPI watchdog_thread_entry(LPVOID lpParameter)
{
	DWORD timeout = (int)(FPTR)lpParameter * 1000;

	while (TRUE)
	{
		HANDLE handle_list[2];
		DWORD wait_result;

		// wait for either a reset or an exit, or a timeout
		handle_list[0] = watchdog_reset_event;
		handle_list[1] = watchdog_exit_event;
		wait_result = WaitForMultipleObjects(2, handle_list, FALSE, timeout);

		// on a reset, just loop around and re-wait
		if (wait_result == WAIT_OBJECT_0 + 0)
			continue;

		// on an exit, break out
		if (wait_result == WAIT_OBJECT_0 + 1)
			break;

		// on a timeout, kill the process
		if (wait_result == WAIT_TIMEOUT)
		{
			fprintf(stderr, "Terminating due to watchdog timeout\n");
			TerminateProcess(GetCurrentProcess(), -1);
		}
	}
	return EXCEPTION_CONTINUE_SEARCH;
}


//============================================================
//  winmain_watchdog_ping
//============================================================

void winmain_watchdog_ping(void)
{
	// if we have a watchdog, reset it
	if (watchdog_reset_event != NULL)
		SetEvent(watchdog_reset_event);
}


//============================================================
//  exception_filter
//============================================================

static LONG WINAPI exception_filter(struct _EXCEPTION_POINTERS *info)
{
	static const struct
	{
		DWORD code;
		const char *string;
	} exception_table[] =
	{
		{ EXCEPTION_ACCESS_VIOLATION,		"ACCESS VIOLATION" },
		{ EXCEPTION_DATATYPE_MISALIGNMENT,	"DATATYPE MISALIGNMENT" },
		{ EXCEPTION_BREAKPOINT, 			"BREAKPOINT" },
		{ EXCEPTION_SINGLE_STEP,			"SINGLE STEP" },
		{ EXCEPTION_ARRAY_BOUNDS_EXCEEDED,	"ARRAY BOUNDS EXCEEDED" },
		{ EXCEPTION_FLT_DENORMAL_OPERAND,	"FLOAT DENORMAL OPERAND" },
		{ EXCEPTION_FLT_DIVIDE_BY_ZERO,		"FLOAT DIVIDE BY ZERO" },
		{ EXCEPTION_FLT_INEXACT_RESULT,		"FLOAT INEXACT RESULT" },
		{ EXCEPTION_FLT_INVALID_OPERATION,	"FLOAT INVALID OPERATION" },
		{ EXCEPTION_FLT_OVERFLOW,			"FLOAT OVERFLOW" },
		{ EXCEPTION_FLT_STACK_CHECK,		"FLOAT STACK CHECK" },
		{ EXCEPTION_FLT_UNDERFLOW,			"FLOAT UNDERFLOW" },
		{ EXCEPTION_INT_DIVIDE_BY_ZERO,		"INTEGER DIVIDE BY ZERO" },
		{ EXCEPTION_INT_OVERFLOW,			"INTEGER OVERFLOW" },
		{ EXCEPTION_PRIV_INSTRUCTION,		"PRIVILEGED INSTRUCTION" },
		{ EXCEPTION_IN_PAGE_ERROR,			"IN PAGE ERROR" },
		{ EXCEPTION_ILLEGAL_INSTRUCTION,	"ILLEGAL INSTRUCTION" },
		{ EXCEPTION_NONCONTINUABLE_EXCEPTION,"NONCONTINUABLE EXCEPTION" },
		{ EXCEPTION_STACK_OVERFLOW, 		"STACK OVERFLOW" },
		{ EXCEPTION_INVALID_DISPOSITION,	"INVALID DISPOSITION" },
		{ EXCEPTION_GUARD_PAGE, 			"GUARD PAGE VIOLATION" },
		{ EXCEPTION_INVALID_HANDLE, 		"INVALID HANDLE" },
		{ 0,								"UNKNOWN EXCEPTION" }
	};
	static int already_hit = 0;
	int i;

	// if we're hitting this recursively, just exit
	if (already_hit)
		return EXCEPTION_CONTINUE_SEARCH;
	already_hit = 1;

	// flush any debugging traces that were live
	debugger_flush_all_traces_on_abnormal_exit();

	// find our man
	for (i = 0; exception_table[i].code != 0; i++)
		if (info->ExceptionRecord->ExceptionCode == exception_table[i].code)
			break;

	// print the exception type and address
	fprintf(stderr, "\n-----------------------------------------------------\n");
	fprintf(stderr, "Exception at EIP=%p%s: %s\n", info->ExceptionRecord->ExceptionAddress,
			symbols->symbol_for_address((FPTR)info->ExceptionRecord->ExceptionAddress), exception_table[i].string);

	// for access violations, print more info
	if (info->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
		fprintf(stderr, "While attempting to %s memory at %p\n",
				info->ExceptionRecord->ExceptionInformation[0] ? "write" : "read",
				(void *)info->ExceptionRecord->ExceptionInformation[1]);

	// print the state of the CPU
	fprintf(stderr, "-----------------------------------------------------\n");
#ifdef PTR64
	fprintf(stderr, "RAX=%p RBX=%p RCX=%p RDX=%p\n",
			(void *)info->ContextRecord->Rax,
			(void *)info->ContextRecord->Rbx,
			(void *)info->ContextRecord->Rcx,
			(void *)info->ContextRecord->Rdx);
	fprintf(stderr, "RSI=%p RDI=%p RBP=%p RSP=%p\n",
			(void *)info->ContextRecord->Rsi,
			(void *)info->ContextRecord->Rdi,
			(void *)info->ContextRecord->Rbp,
			(void *)info->ContextRecord->Rsp);
	fprintf(stderr, " R8=%p  R9=%p R10=%p R11=%p\n",
			(void *)info->ContextRecord->R8,
			(void *)info->ContextRecord->R9,
			(void *)info->ContextRecord->R10,
			(void *)info->ContextRecord->R11);
	fprintf(stderr, "R12=%p R13=%p R14=%p R15=%p\n",
			(void *)info->ContextRecord->R12,
			(void *)info->ContextRecord->R13,
			(void *)info->ContextRecord->R14,
			(void *)info->ContextRecord->R15);
#else
	fprintf(stderr, "EAX=%p EBX=%p ECX=%p EDX=%p\n",
			(void *)info->ContextRecord->Eax,
			(void *)info->ContextRecord->Ebx,
			(void *)info->ContextRecord->Ecx,
			(void *)info->ContextRecord->Edx);
	fprintf(stderr, "ESI=%p EDI=%p EBP=%p ESP=%p\n",
			(void *)info->ContextRecord->Esi,
			(void *)info->ContextRecord->Edi,
			(void *)info->ContextRecord->Ebp,
			(void *)info->ContextRecord->Esp);
#endif

	stack_walker walker;
	walker.reset(*info->ContextRecord, GetCurrentThread());

	// reprint the actual exception address
	fprintf(stderr, "-----------------------------------------------------\n");
	fprintf(stderr, "Stack crawl:\n");

	// walk the stack
	while (walker.unwind())
		fprintf(stderr, "  %p: %p%s\n", (void *)walker.frame(), (void *)walker.ip(), symbols->symbol_for_address(walker.ip()));

	// exit
	return EXCEPTION_CONTINUE_SEARCH;
}



//**************************************************************************
//  STACK WALKER
//**************************************************************************

//-------------------------------------------------
//  stack_walker - constructor
//-------------------------------------------------

stack_walker::stack_walker()
	: m_process(GetCurrentProcess()),
	  m_thread(GetCurrentThread()),
	  m_first(true),
	  m_stack_walk_64(TEXT("dbghelp.dll"), "StackWalk64"),
	  m_sym_initialize(TEXT("dbghelp.dll"), "SymInitialize"),
	  m_sym_function_table_access_64(TEXT("dbghelp.dll"), "SymFunctionTableAccess64"),
	  m_sym_get_module_base_64(TEXT("dbghelp.dll"), "SymGetModuleBase64")
{
	// zap the structs
	memset(&m_stackframe, 0, sizeof(m_stackframe));
	memset(&m_context, 0, sizeof(m_context));

	// initialize the symbols
	if (!s_initialized && m_sym_initialize && m_stack_walk_64 && m_sym_function_table_access_64 && m_sym_get_module_base_64)
	{
		(*m_sym_initialize)(m_process, NULL, TRUE);
		s_initialized = true;
	}
}


//-------------------------------------------------
//  reset - set up a new context
//-------------------------------------------------

void stack_walker::reset(CONTEXT &initial, HANDLE thread)
{
	// set up the initial state
	m_context = initial;
	m_thread = thread;
	m_first = true;

	// initialize the stackframe
	memset(&m_stackframe, 0, sizeof(m_stackframe));
	m_stackframe.AddrPC.Mode = AddrModeFlat;
	m_stackframe.AddrFrame.Mode = AddrModeFlat;
	m_stackframe.AddrStack.Mode = AddrModeFlat;

	// pull architecture-specific fields from the context
#ifdef PTR64
	m_stackframe.AddrPC.Offset = m_context.Rip;
	m_stackframe.AddrFrame.Offset = m_context.Rsp;
	m_stackframe.AddrStack.Offset = m_context.Rsp;
#else
	m_stackframe.AddrPC.Offset = m_context.Eip;
	m_stackframe.AddrFrame.Offset = m_context.Ebp;
	m_stackframe.AddrStack.Offset = m_context.Esp;
#endif
}


//-------------------------------------------------
//  unwind - unwind a single level
//-------------------------------------------------

bool stack_walker::unwind()
{
	// if we were able to initialize, then we have everything we need
	if (s_initialized)
	{
#ifdef PTR64
		return (*m_stack_walk_64)(IMAGE_FILE_MACHINE_AMD64, m_process, m_thread, &m_stackframe, &m_context, NULL, *m_sym_function_table_access_64, *m_sym_get_module_base_64, NULL);
#else
		return (*m_stack_walk_64)(IMAGE_FILE_MACHINE_I386, m_process, m_thread, &m_stackframe, &m_context, NULL, *m_sym_function_table_access_64, *m_sym_get_module_base_64, NULL);
#endif
	}

	// otherwise, fake the first unwind, which will just return info from the context
	else
	{
		bool result = m_first;
		m_first = false;
		return result;
	}
}



//**************************************************************************
//  SYMBOL MANAGER
//**************************************************************************

//-------------------------------------------------
//  symbol_manager - constructor
//-------------------------------------------------

symbol_manager::symbol_manager(const char *argv0)
	: m_cache(NULL),
	  m_mapfile(argv0),
	  m_symfile(argv0),
	  m_process(GetCurrentProcess()),
	  m_last_base(0),
	  m_text_base(0),
	  m_sym_from_addr(TEXT("dbghelp.dll"), "SymFromAddr"),
	  m_sym_get_line_from_addr_64(TEXT("dbghelp.dll"), "SymGetLineFromAddr64")
{
#ifdef __GNUC__
	// compute the name of the mapfile
	int extoffs = m_mapfile.rchr(0, '.');
	if (extoffs != -1)
		m_mapfile.substr(0, extoffs);
	m_mapfile.cat(".map");

	// and the name of the symfile
	extoffs = m_symfile.rchr(0, '.');
	if (extoffs != -1)
		m_symfile.substr(0, extoffs);
	m_symfile.cat(".sym");

	// figure out the base of the .text section
	m_text_base = get_text_section_base();
#endif

	// expand the buffer to be decently large up front
	m_buffer.printf("%500s", "");
}


//-------------------------------------------------
//  ~symbol_manager - destructor
//-------------------------------------------------

symbol_manager::~symbol_manager()
{
	// clean up the cache
	while (m_cache != NULL)
	{
		cache_entry *entry = m_cache;
		m_cache = entry->m_next;
		global_free(entry);
	}
}


//-------------------------------------------------
//  symbol_for_address - return a symbol by looking
//  it up either in the cache or by scanning the
//  file
//-------------------------------------------------

const char *symbol_manager::symbol_for_address(FPTR address)
{
	// default the buffer
	m_buffer.cpy(" (not found)");
	m_last_base = 0;

	// first try to do it using system APIs
	if (!query_system_for_address(address))
	{
		// if that fails, scan the cache if we have one
		if (m_cache != NULL)
			scan_cache_for_address(address);

		// or else try to open a sym/map file and find it there
		else
			scan_file_for_address(address, false);
	}
	return m_buffer;
}


//-------------------------------------------------
//  query_system_for_address - ask the system to
//  look up our address
//-------------------------------------------------

bool symbol_manager::query_system_for_address(FPTR address)
{
	// need at least the sym_from_addr API
	if (m_sym_from_addr == NULL)
		return false;

	BYTE info_buffer[sizeof(SYMBOL_INFO) + 256] = { 0 };
	SYMBOL_INFO &info = *reinterpret_cast<SYMBOL_INFO *>(&info_buffer[0]);
	DWORD64 displacement;

	// even through the struct says TCHAR, we actually get back an ANSI string here
	info.SizeOfStruct = sizeof(info);
	info.MaxNameLen = sizeof(info_buffer) - sizeof(info);
	if ((*m_sym_from_addr)(m_process, address, &displacement, &info))
	{
		// try to get source info as well; again we are returned an ANSI string
		IMAGEHLP_LINE64 lineinfo = { sizeof(lineinfo) };
		DWORD linedisp;
		if (m_sym_get_line_from_addr_64 != NULL && (*m_sym_get_line_from_addr_64)(m_process, address, &linedisp, &lineinfo))
			format_symbol(info.Name, displacement, lineinfo.FileName, lineinfo.LineNumber);
		else
			format_symbol(info.Name, displacement);

		// set the last base
		m_last_base = address - displacement;
		return true;
	}
	return false;
}


//-------------------------------------------------
//  scan_file_for_address - walk either the map
//  or symbol files and find the best match for
//  the given address, optionally creating a cache
//  along the way
//-------------------------------------------------

void symbol_manager::scan_file_for_address(FPTR address, bool create_cache)
{
	bool is_symfile = false;
	FILE *srcfile = NULL;

#ifdef __GNUC__
	// see if we have a symbol file (gcc only)
	srcfile = fopen(m_symfile, "r");
	is_symfile = (srcfile != NULL);
#endif

	// if not, see if we have a map file
	if (srcfile == NULL)
		srcfile = fopen(m_mapfile, "r");

	// if not, fail
	if (srcfile == NULL)
		return;

	// reset the best info
	astring best_symbol;
	FPTR best_addr = 0;

	// parse the file, looking for valid entries
	cache_entry **tailptr = &m_cache;
	astring symbol;
	char line[1024];
	while (fgets(line, sizeof(line) - 1, srcfile))
	{
		// parse the line looking for an interesting symbol
		FPTR addr;
		bool valid = is_symfile ? parse_sym_line(line, addr, symbol) : parse_map_line(line, addr, symbol);

		// if we got one, see if this is the best
		if (valid)
		{
			// if this is the best one so far, remember it
			if (addr <= address && addr > best_addr)
			{
				best_addr = addr;
				best_symbol = symbol;
			}

			// also create a cache entry if we can
			if (create_cache)
			{
				*tailptr = global_alloc(cache_entry(addr, symbol));
				tailptr = &(*tailptr)->m_next;
			}
		}
	}

	// close the file
	fclose(srcfile);

	// format the symbol and remember the last base
	format_symbol(best_symbol, address - best_addr);
	m_last_base = best_addr;
}


//-------------------------------------------------
//  scan_cache_for_address - walk the cache to
//  find the best match for the given address
//-------------------------------------------------

void symbol_manager::scan_cache_for_address(FPTR address)
{
	// reset the best info
	astring best_symbol;
	FPTR best_addr = 0;

	// walk the cache, looking for valid entries
	for (cache_entry *entry = m_cache; entry != NULL; entry = entry->m_next)

		// if this is the best one so far, remember it
		if (entry->m_address <= address && entry->m_address > best_addr)
		{
			best_addr = entry->m_address;
			best_symbol = entry->m_name;
		}

	// format the symbol and remember the last base
	format_symbol(best_symbol, address - best_addr);
	m_last_base = best_addr;
}


//-------------------------------------------------
//  parse_sym_line - parse a line from a sym file
//  which is just the output of objdump
//-------------------------------------------------

bool symbol_manager::parse_sym_line(const char *line, FPTR &address, astring &symbol)
{
#ifdef __GNUC__
/*
    32-bit gcc symbol line:
[271778](sec  1)(fl 0x00)(ty  20)(scl   3) (nx 0) 0x007df675 line_to_symbol(char const*, unsigned int&, bool)

    64-bit gcc symbol line:
[271775](sec  1)(fl 0x00)(ty  20)(scl   3) (nx 0) 0x00000000008dd1e9 line_to_symbol(char const*, unsigned long long&, bool)
*/

	// first look for a (ty) entry
	const char *type = strstr(line, "(ty  20)");
	if (type == NULL)
		return false;

	// scan forward in the line to find the address
	bool in_parens = false;
	for (const char *chptr = type; *chptr != 0; chptr++)
	{
		// track open/close parentheses
		if (*chptr == '(')
			in_parens = true;
		else if (*chptr == ')')
			in_parens = false;

		// otherwise, look for an 0x address
		else if (!in_parens && *chptr == '0' && chptr[1] == 'x')
		{
			// make sure we can get an address
			void *temp;
			if (sscanf(chptr, "0x%p", &temp) != 1)
				return false;
			address = m_text_base + reinterpret_cast<FPTR>(temp);

			// skip forward until we're past the space
			while (*chptr != 0 && !isspace(*chptr))
				chptr++;

			// extract the symbol name
			symbol.cpy(chptr).trimspace();
			return (symbol.len() > 0);
		}
	}
#endif
	return false;
}


//-------------------------------------------------
//  parse_map_line - parse a line from a linker-
//  generated map file
//-------------------------------------------------

bool symbol_manager::parse_map_line(const char *line, FPTR &address, astring &symbol)
{
#ifdef __GNUC__
/*
    32-bit gcc map line:
                0x0089cb00                nbmj9195_palette_r(_address_space const*, unsigned int)

    64-bit gcc map line:
                0x0000000000961afc                nbmj9195_palette_r(_address_space const*, unsigned int)
*/

	// find a matching start
	if (strncmp(line, "                0x", 18) == 0)
	{
		// make sure we can get an address
		void *temp;
		if (sscanf(&line[16], "0x%p", &temp) != 1)
			return false;
		address = reinterpret_cast<FPTR>(temp);

		// skip forward until we're past the space
		const char *chptr = &line[16];
		while (*chptr != 0 && !isspace(*chptr))
			chptr++;

		// extract the symbol name
		symbol.cpy(chptr).trimspace();
		return (symbol.len() > 0);
	}
#endif
	return false;
}


//-------------------------------------------------
//  format_symbol - common symbol formatting
//-------------------------------------------------

void symbol_manager::format_symbol(const char *name, UINT32 displacement, const char *filename, int linenumber)
{
	// start with the address and offset
	m_buffer.printf(" (%s", name);
	if (displacement != 0)
		m_buffer.catprintf("+0x%04x", (UINT32)displacement);

	// append file/line if present
	if (filename != NULL)
		m_buffer.catprintf(", %s:%d", filename, linenumber);

	// close up the string
	m_buffer.cat(")");
}


//-------------------------------------------------
//  get_text_section_base - figure out the base
//  of the .text section
//-------------------------------------------------

FPTR symbol_manager::get_text_section_base()
{
	dynamic_bind<PIMAGE_SECTION_HEADER (WINAPI *)(PIMAGE_NT_HEADERS, PVOID, ULONG)> image_rva_to_section(TEXT("dbghelp.dll"), "ImageRvaToSection");
	dynamic_bind<PIMAGE_NT_HEADERS (WINAPI *)(PVOID)> image_nt_header(TEXT("dbghelp.dll"), "ImageNtHeader");

	// start with the image base
	PVOID base = reinterpret_cast<PVOID>(GetModuleHandle(NULL));
	assert(base != NULL);

	// make sure we have the functions we need
	if (image_nt_header != NULL && image_rva_to_section != NULL)
	{
		// get the NT header
		PIMAGE_NT_HEADERS headers = (*image_nt_header)(base);
		assert(headers != NULL);

		// look ourself up (assuming we are in the .text section)
		PIMAGE_SECTION_HEADER section = (*image_rva_to_section)(headers, base, reinterpret_cast<FPTR>(get_text_section_base) - reinterpret_cast<FPTR>(base));
		if (section != NULL)
			return reinterpret_cast<FPTR>(base) + section->VirtualAddress;
	}

	// fallback to returning the image base (wrong)
	return reinterpret_cast<FPTR>(base);
}



//**************************************************************************
//  SAMPLING PROFILER
//**************************************************************************

//-------------------------------------------------
//  sampling_profiler - constructor
//-------------------------------------------------

sampling_profiler::sampling_profiler(UINT32 max_seconds, UINT8 stack_depth = 0)
	: m_thread(NULL),
	  m_thread_id(0),
	  m_thread_exit(false),
	  m_stack_depth(stack_depth),
	  m_entry_stride(stack_depth + 2),
	  m_max_seconds(max_seconds),
	  m_buffer(global_alloc(FPTR[max_seconds * 1000 * m_entry_stride])),
	  m_buffer_ptr(m_buffer),
	  m_buffer_end(m_buffer + max_seconds * 1000 * m_entry_stride)
{
}


//-------------------------------------------------
//  sampling_profiler - destructor
//-------------------------------------------------

sampling_profiler::~sampling_profiler()
{
	global_free(m_buffer);
}


//-------------------------------------------------
//  start - begin gathering profiling information
//-------------------------------------------------

void sampling_profiler::start()
{
	// do the dance to get a handle to ourself
	BOOL result = DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &m_target_thread,
			THREAD_GET_CONTEXT | THREAD_SUSPEND_RESUME | THREAD_QUERY_INFORMATION, FALSE, 0);
	assert_always(result, "Failed to get thread handle for main thread");

	// reset the exit flag
	m_thread_exit = false;

	// start the thread
	m_thread = CreateThread(NULL, 0, thread_entry, (LPVOID)this, 0, &m_thread_id);
	assert_always(m_thread != NULL, "Failed to create profiler thread\n");

	// max out the priority
	SetThreadPriority(m_thread, THREAD_PRIORITY_TIME_CRITICAL);
}


//-------------------------------------------------
//  stop - stop gathering profiling information
//-------------------------------------------------

void sampling_profiler::stop()
{
	// set the flag and wait a couple of seconds (max)
	m_thread_exit = true;
	WaitForSingleObject(m_thread, 2000);

	// regardless, close the handle
	CloseHandle(m_thread);
}


//-------------------------------------------------
//  compare_address - compare two entries by their
//  bucket address
//-------------------------------------------------

int CLIB_DECL sampling_profiler::compare_address(const void *item1, const void *item2)
{
	const FPTR *entry1 = reinterpret_cast<const FPTR *>(item1);
	const FPTR *entry2 = reinterpret_cast<const FPTR *>(item2);
	int mincount = MIN(entry1[0], entry2[0]);

	// sort in order of: bucket, caller, caller's caller, etc.
	for (int index = 1; index <= mincount; index++)
		if (entry1[index] != entry2[index])
			return entry1[index] - entry2[index];

	// if we match to the end, sort by the depth of the stack
	return entry1[0] - entry2[0];
}


//-------------------------------------------------
//  compare_frequency - compare two entries by
//  their frequency of occurrence
//-------------------------------------------------

int CLIB_DECL sampling_profiler::compare_frequency(const void *item1, const void *item2)
{
	const FPTR *entry1 = reinterpret_cast<const FPTR *>(item1);
	const FPTR *entry2 = reinterpret_cast<const FPTR *>(item2);

	// sort by frequency, then by address
	if (entry1[0] != entry2[0])
		return entry2[0] - entry1[0];
	return entry1[1] - entry2[1];
}


//-------------------------------------------------
//  print_results - output the results
//-------------------------------------------------

void sampling_profiler::print_results(symbol_manager &symbols)
{
	// cache the symbols
	symbols.cache_symbols();

	// step 1: find the base of each entry
	for (FPTR *current = m_buffer; current < m_buffer_ptr; current += m_entry_stride)
	{
		assert(current[0] >= 1 && current[0] < m_entry_stride);

		// convert the sampled PC to its function base as a bucket
		symbols.symbol_for_address(current[1]);
		current[1] = symbols.last_base();
	}

	// step 2: sort the results
	qsort(m_buffer, (m_buffer_ptr - m_buffer) / m_entry_stride, m_entry_stride * sizeof(FPTR), compare_address);

	// step 3: count and collapse unique entries
	UINT32 total_count = 0;
	for (FPTR *current = m_buffer; current < m_buffer_ptr; )
	{
		int count = 1;
		FPTR *scan;
		for (scan = current + m_entry_stride; scan < m_buffer_ptr; scan += m_entry_stride)
		{
			if (compare_address(current, scan) != 0)
				break;
			scan[0] = 0;
			count++;
		}
		current[0] = count;
		total_count += count;
		current = scan;
	}

	// step 4: sort the results again, this time by frequency
	qsort(m_buffer, (m_buffer_ptr - m_buffer) / m_entry_stride, m_entry_stride * sizeof(FPTR), compare_frequency);

	// step 5: print the results
	UINT32 num_printed = 0;
	for (FPTR *current = m_buffer; current < m_buffer_ptr && num_printed < 30; current += m_entry_stride)
	{
		// once we hit 0 frequency, we're done
		if (current[0] == 0)
			break;

		// output the result
		printf("%4.1f%% - %6d : %p%s\n", (double)current[0] * 100.0 / (double)total_count, (UINT32)current[0], reinterpret_cast<void *>(current[1]), symbols.symbol_for_address(current[1]));
		for (int index = 2; index < m_entry_stride; index++)
		{
			if (current[index] == 0)
				break;
			printf("                 %p%s\n", reinterpret_cast<void *>(current[index]), symbols.symbol_for_address(current[index]));
		}
		printf("\n");
		num_printed++;
	}
}


//-------------------------------------------------
//  thread_entry - thread entry stub
//-------------------------------------------------

DWORD WINAPI sampling_profiler::thread_entry(LPVOID lpParameter)
{
	reinterpret_cast<sampling_profiler *>(lpParameter)->thread_run();
	return 0;
}


//-------------------------------------------------
//  thread_run - sampling thread
//-------------------------------------------------

void sampling_profiler::thread_run()
{
	CONTEXT context;
	memset(&context, 0, sizeof(context));

	// loop until done
	stack_walker walker;
	while (!m_thread_exit && m_buffer_ptr < m_buffer_end)
	{
		// pause the main thread and get its context
		SuspendThread(m_target_thread);
		context.ContextFlags = CONTEXT_FULL;
		GetThreadContext(m_target_thread, &context);

		// first entry is a count
		FPTR *count = m_buffer_ptr++;
		*count = 0;

		// iterate over the frames until we run out or hit an error
		walker.reset(context, m_target_thread);
		int frame;
		for (frame = 0; frame <= m_stack_depth && walker.unwind(); frame++)
		{
			*m_buffer_ptr++ = walker.ip();
			*count += 1;
		}

		// fill in any missing parts with NULLs
		for (; frame <= m_stack_depth; frame++)
			*m_buffer_ptr++ = 0;

		// resume the thread
		ResumeThread(m_target_thread);

		// sleep for 1ms
		Sleep(1);
	}
}
