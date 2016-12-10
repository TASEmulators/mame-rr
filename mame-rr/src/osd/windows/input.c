//============================================================
//
//  input.c - Win32 implementation of MAME input routines
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

// For testing purposes: force DirectInput
#define FORCE_DIRECTINPUT	0

// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winioctl.h>
#include <tchar.h>

// undef WINNT for dinput.h to prevent duplicate definition
#undef WINNT
#include <dinput.h>
#undef interface

// standard C headers
#include <conio.h>
#include <ctype.h>
#include <stddef.h>

// MAME headers
#include "osdepend.h"
#include "emu.h"
#include "ui.h"

// MAMEOS headers
#include "winmain.h"
#include "window.h"
#include "input.h"
#include "debugwin.h"
#include "video.h"
#include "strconv.h"
#include "config.h"

#ifdef MESS
#include "uimess.h"
#endif



//============================================================
//  PARAMETERS
//============================================================

enum
{
	POVDIR_LEFT = 0,
	POVDIR_RIGHT,
	POVDIR_UP,
	POVDIR_DOWN
};

#define MAX_KEYS			256

#define MAME_KEY			0
#define DI_KEY				1
#define VIRTUAL_KEY			2
#define ASCII_KEY			3



//============================================================
//  MACROS
//============================================================

#define STRUCTSIZE(x)		((dinput_version == 0x0300) ? sizeof(x##_DX3) : sizeof(x))

#ifdef UNICODE
#define UNICODE_SUFFIX		"W"
#else
#define UNICODE_SUFFIX		"A"
#endif



//============================================================
//  TYPEDEFS
//============================================================

// state information for a keyboard; DirectInput state must be first element
typedef struct _keyboard_state keyboard_state;
struct _keyboard_state
{
	UINT8					state[MAX_KEYS];
	INT8					oldkey[MAX_KEYS];
	INT8					currkey[MAX_KEYS];
};


// state information for a mouse; DirectInput state must be first element
typedef struct _mouse_state mouse_state;
struct _mouse_state
{
	DIMOUSESTATE2			state;
	LONG					raw_x, raw_y, raw_z;
};


// state information for a joystick; DirectInput state must be first element
typedef struct _joystick_state joystick_state;
struct _joystick_state
{
	DIJOYSTATE				state;
	LONG					rangemin[8];
	LONG					rangemax[8];
};


// DirectInput-specific information about a device
typedef struct _dinput_device_info dinput_device_info;
struct _dinput_device_info
{
	LPDIRECTINPUTDEVICE		device;
	LPDIRECTINPUTDEVICE2	device2;
	DIDEVCAPS				caps;
	LPCDIDATAFORMAT			format;
};


// RawInput-specific information about a device
typedef struct _rawinput_device_info rawinput_device_info;
struct _rawinput_device_info
{
	HANDLE					device;
};


// generic device information
typedef struct _device_info device_info;
struct _device_info
{
	// device information
	device_info **			head;
	device_info *			next;
	const char *			name;
	void					(*poll)(device_info *info);

	// MAME information
	running_machine *		machine;
	input_device *			device;

	// device state
	union
	{
		keyboard_state		keyboard;
		mouse_state			mouse;
		joystick_state		joystick;
	};

	// DirectInput/RawInput-specific state
	dinput_device_info		dinput;
	rawinput_device_info	rawinput;
};


// RawInput APIs
typedef /*WINUSERAPI*/ INT (WINAPI *get_rawinput_device_list_ptr)(OUT PRAWINPUTDEVICELIST pRawInputDeviceList, IN OUT PINT puiNumDevices, IN UINT cbSize);
typedef /*WINUSERAPI*/ INT (WINAPI *get_rawinput_data_ptr)(IN HRAWINPUT hRawInput, IN UINT uiCommand, OUT LPVOID pData, IN OUT PINT pcbSize, IN UINT cbSizeHeader);
typedef /*WINUSERAPI*/ INT (WINAPI *get_rawinput_device_info_ptr)(IN HANDLE hDevice, IN UINT uiCommand, OUT LPVOID pData, IN OUT PINT pcbSize);
typedef /*WINUSERAPI*/ BOOL (WINAPI *register_rawinput_devices_ptr)(IN PCRAWINPUTDEVICE pRawInputDevices, IN UINT uiNumDevices, IN UINT cbSize);



//============================================================
//  LOCAL VARIABLES
//============================================================

// global states
static UINT8				input_enabled;
static osd_lock *			input_lock;
static UINT8				input_paused;
static DWORD				last_poll;

// DirectInput variables
static LPDIRECTINPUT		dinput;
static int					dinput_version;

// RawInput variables
static get_rawinput_device_list_ptr		get_rawinput_device_list;
static get_rawinput_data_ptr			get_rawinput_data;
static get_rawinput_device_info_ptr 	get_rawinput_device_info;
static register_rawinput_devices_ptr	register_rawinput_devices;

// keyboard states
static UINT8				keyboard_win32_reported_key_down;
static device_info *		keyboard_list;

// mouse states
static UINT8				mouse_enabled;
static device_info *		mouse_list;

// lightgun states
static UINT8				lightgun_shared_axis_mode;
static UINT8				lightgun_enabled;
static device_info *		lightgun_list;

// joystick states
static device_info *		joystick_list;

// default axis names
static const TCHAR *const default_axis_name[] =
{
	TEXT("X"), TEXT("Y"), TEXT("Z"), TEXT("RX"),
	TEXT("RY"), TEXT("RZ"), TEXT("SL1"), TEXT("SL2")
};



//============================================================
//  PROTOTYPES
//============================================================

static void wininput_pause(running_machine &machine);
static void wininput_resume(running_machine &machine);
static void wininput_exit(running_machine &machine);

// device list management
static void device_list_poll_devices(device_info *devlist_head);
static void device_list_reset_devices(device_info *devlist_head);

// generic device management
static device_info *generic_device_alloc(running_machine *machine, device_info **devlist_head_ptr, const TCHAR *name);
static void generic_device_free(device_info *devinfo);
static int generic_device_index(device_info *devlist_head, device_info *devinfo);
static void generic_device_reset(device_info *devinfo);
static INT32 generic_button_get_state(void *device_internal, void *item_internal);
static INT32 generic_axis_get_state(void *device_internal, void *item_internal);

// Win32-specific input code
static void win32_init(running_machine *machine);
static void win32_exit(running_machine &machine);
static void win32_keyboard_poll(device_info *devinfo);
static void win32_lightgun_poll(device_info *devinfo);

// DirectInput-specific code
static void dinput_init(running_machine *machine);
static void dinput_exit(running_machine &machine);
static HRESULT dinput_set_dword_property(LPDIRECTINPUTDEVICE device, REFGUID property_guid, DWORD object, DWORD how, DWORD value);
static device_info *dinput_device_create(running_machine *machine, device_info **devlist_head_ptr, LPCDIDEVICEINSTANCE instance, LPCDIDATAFORMAT format1, LPCDIDATAFORMAT format2, DWORD cooperative_level);
static void dinput_device_release(device_info *devinfo);
static char *dinput_device_item_name(device_info *devinfo, int offset, const TCHAR *defstring, const TCHAR *suffix);
static HRESULT dinput_device_poll(device_info *devinfo);
static BOOL CALLBACK dinput_keyboard_enum(LPCDIDEVICEINSTANCE instance, LPVOID ref);
static void dinput_keyboard_poll(device_info *devinfo);
static BOOL CALLBACK dinput_mouse_enum(LPCDIDEVICEINSTANCE instance, LPVOID ref);
static void dinput_mouse_poll(device_info *devinfo);
static BOOL CALLBACK dinput_joystick_enum(LPCDIDEVICEINSTANCE instance, LPVOID ref);
static void dinput_joystick_poll(device_info *devinfo);
static INT32 dinput_joystick_pov_get_state(void *device_internal, void *item_internal);

// RawInput-specific code
static void rawinput_init(running_machine *machine);
static void rawinput_exit(running_machine &machine);
static device_info *rawinput_device_create(running_machine *machine, device_info **devlist_head_ptr, PRAWINPUTDEVICELIST device);
static void rawinput_device_release(device_info *info);
static TCHAR *rawinput_device_improve_name(TCHAR *name);
static void rawinput_keyboard_enum(running_machine *machine, PRAWINPUTDEVICELIST device);
static void rawinput_keyboard_update(HANDLE device, RAWKEYBOARD *data);
static void rawinput_mouse_enum(running_machine *machine, PRAWINPUTDEVICELIST device);
static void rawinput_mouse_update(HANDLE device, RAWMOUSE *data);
static void rawinput_mouse_poll(device_info *devinfo);

// misc utilities
static TCHAR *reg_query_string(HKEY key, const TCHAR *path);
static const TCHAR *default_button_name(int which);
static const TCHAR *default_pov_name(int which);



//============================================================
//  KEYBOARD/JOYSTICK LIST
//============================================================

// master keyboard translation table
static const int win_key_trans_table[][4] =
{
	// MAME key             dinput key          virtual key     ascii
	{ ITEM_ID_ESC,			DIK_ESCAPE,			VK_ESCAPE,		27 },
	{ ITEM_ID_1,			DIK_1,				'1',			'1' },
	{ ITEM_ID_2,			DIK_2,				'2',			'2' },
	{ ITEM_ID_3,			DIK_3,				'3',			'3' },
	{ ITEM_ID_4,			DIK_4,				'4',			'4' },
	{ ITEM_ID_5,			DIK_5,				'5',			'5' },
	{ ITEM_ID_6,			DIK_6,				'6',			'6' },
	{ ITEM_ID_7,			DIK_7,				'7',			'7' },
	{ ITEM_ID_8,			DIK_8,				'8',			'8' },
	{ ITEM_ID_9,			DIK_9,				'9',			'9' },
	{ ITEM_ID_0,			DIK_0,				'0',			'0' },
	{ ITEM_ID_MINUS,		DIK_MINUS,			VK_OEM_MINUS,	'-' },
	{ ITEM_ID_EQUALS,		DIK_EQUALS,			VK_OEM_PLUS,	'=' },
	{ ITEM_ID_BACKSPACE,	DIK_BACK,			VK_BACK,		8 },
	{ ITEM_ID_TAB,			DIK_TAB,			VK_TAB, 		9 },
	{ ITEM_ID_Q,			DIK_Q,				'Q',			'Q' },
	{ ITEM_ID_W,			DIK_W,				'W',			'W' },
	{ ITEM_ID_E,			DIK_E,				'E',			'E' },
	{ ITEM_ID_R,			DIK_R,				'R',			'R' },
	{ ITEM_ID_T,			DIK_T,				'T',			'T' },
	{ ITEM_ID_Y,			DIK_Y,				'Y',			'Y' },
	{ ITEM_ID_U,			DIK_U,				'U',			'U' },
	{ ITEM_ID_I,			DIK_I,				'I',			'I' },
	{ ITEM_ID_O,			DIK_O,				'O',			'O' },
	{ ITEM_ID_P,			DIK_P,				'P',			'P' },
	{ ITEM_ID_OPENBRACE,	DIK_LBRACKET,		VK_OEM_4,		'[' },
	{ ITEM_ID_CLOSEBRACE,	DIK_RBRACKET,		VK_OEM_6,		']' },
	{ ITEM_ID_ENTER,		DIK_RETURN, 		VK_RETURN,		13 },
	{ ITEM_ID_LCONTROL, 	DIK_LCONTROL,		VK_LCONTROL,	0 },
	{ ITEM_ID_A,			DIK_A,				'A',			'A' },
	{ ITEM_ID_S,			DIK_S,				'S',			'S' },
	{ ITEM_ID_D,			DIK_D,				'D',			'D' },
	{ ITEM_ID_F,			DIK_F,				'F',			'F' },
	{ ITEM_ID_G,			DIK_G,				'G',			'G' },
	{ ITEM_ID_H,			DIK_H,				'H',			'H' },
	{ ITEM_ID_J,			DIK_J,				'J',			'J' },
	{ ITEM_ID_K,			DIK_K,				'K',			'K' },
	{ ITEM_ID_L,			DIK_L,				'L',			'L' },
	{ ITEM_ID_COLON,		DIK_SEMICOLON,		VK_OEM_1,		';' },
	{ ITEM_ID_QUOTE,		DIK_APOSTROPHE,		VK_OEM_7,		'\'' },
	{ ITEM_ID_TILDE,		DIK_GRAVE,			VK_OEM_3,		'`' },
	{ ITEM_ID_LSHIFT,		DIK_LSHIFT, 		VK_LSHIFT,		0 },
	{ ITEM_ID_BACKSLASH,	DIK_BACKSLASH,		VK_OEM_5,		'\\' },
	{ ITEM_ID_Z,			DIK_Z,				'Z',			'Z' },
	{ ITEM_ID_X,			DIK_X,				'X',			'X' },
	{ ITEM_ID_C,			DIK_C,				'C',			'C' },
	{ ITEM_ID_V,			DIK_V,				'V',			'V' },
	{ ITEM_ID_B,			DIK_B,				'B',			'B' },
	{ ITEM_ID_N,			DIK_N,				'N',			'N' },
	{ ITEM_ID_M,			DIK_M,				'M',			'M' },
	{ ITEM_ID_COMMA,		DIK_COMMA,			VK_OEM_COMMA,	',' },
	{ ITEM_ID_STOP, 		DIK_PERIOD, 		VK_OEM_PERIOD,	'.' },
	{ ITEM_ID_SLASH,		DIK_SLASH,			VK_OEM_2,		'/' },
	{ ITEM_ID_RSHIFT,		DIK_RSHIFT, 		VK_RSHIFT,		0 },
	{ ITEM_ID_ASTERISK, 	DIK_MULTIPLY,		VK_MULTIPLY,	'*' },
	{ ITEM_ID_LALT, 		DIK_LMENU,			VK_LMENU,		0 },
	{ ITEM_ID_SPACE,		DIK_SPACE,			VK_SPACE,		' ' },
	{ ITEM_ID_CAPSLOCK, 	DIK_CAPITAL,		VK_CAPITAL, 	0 },
	{ ITEM_ID_F1,			DIK_F1,				VK_F1,			0 },
	{ ITEM_ID_F2,			DIK_F2,				VK_F2,			0 },
	{ ITEM_ID_F3,			DIK_F3,				VK_F3,			0 },
	{ ITEM_ID_F4,			DIK_F4,				VK_F4,			0 },
	{ ITEM_ID_F5,			DIK_F5,				VK_F5,			0 },
	{ ITEM_ID_F6,			DIK_F6,				VK_F6,			0 },
	{ ITEM_ID_F7,			DIK_F7,				VK_F7,			0 },
	{ ITEM_ID_F8,			DIK_F8,				VK_F8,			0 },
	{ ITEM_ID_F9,			DIK_F9,				VK_F9,			0 },
	{ ITEM_ID_F10,			DIK_F10,			VK_F10, 		0 },
	{ ITEM_ID_NUMLOCK,		DIK_NUMLOCK,		VK_NUMLOCK, 	0 },
	{ ITEM_ID_SCRLOCK,		DIK_SCROLL,			VK_SCROLL,		0 },
	{ ITEM_ID_7_PAD,		DIK_NUMPAD7,		VK_NUMPAD7, 	0 },
	{ ITEM_ID_8_PAD,		DIK_NUMPAD8,		VK_NUMPAD8, 	0 },
	{ ITEM_ID_9_PAD,		DIK_NUMPAD9,		VK_NUMPAD9, 	0 },
	{ ITEM_ID_MINUS_PAD,	DIK_SUBTRACT,		VK_SUBTRACT,	0 },
	{ ITEM_ID_4_PAD,		DIK_NUMPAD4,		VK_NUMPAD4, 	0 },
	{ ITEM_ID_5_PAD,		DIK_NUMPAD5,		VK_NUMPAD5, 	0 },
	{ ITEM_ID_6_PAD,		DIK_NUMPAD6,		VK_NUMPAD6, 	0 },
	{ ITEM_ID_PLUS_PAD, 	DIK_ADD,			VK_ADD, 		0 },
	{ ITEM_ID_1_PAD,		DIK_NUMPAD1,		VK_NUMPAD1, 	0 },
	{ ITEM_ID_2_PAD,		DIK_NUMPAD2,		VK_NUMPAD2, 	0 },
	{ ITEM_ID_3_PAD,		DIK_NUMPAD3,		VK_NUMPAD3, 	0 },
	{ ITEM_ID_0_PAD,		DIK_NUMPAD0,		VK_NUMPAD0, 	0 },
	{ ITEM_ID_DEL_PAD,		DIK_DECIMAL,		VK_DECIMAL, 	0 },
	{ ITEM_ID_F11,			DIK_F11,			VK_F11, 		0 },
	{ ITEM_ID_F12,			DIK_F12,			VK_F12, 		0 },
	{ ITEM_ID_F13,			DIK_F13,			VK_F13, 		0 },
	{ ITEM_ID_F14,			DIK_F14,			VK_F14, 		0 },
	{ ITEM_ID_F15,			DIK_F15,			VK_F15, 		0 },
	{ ITEM_ID_ENTER_PAD,	DIK_NUMPADENTER,	VK_RETURN,		0 },
	{ ITEM_ID_RCONTROL, 	DIK_RCONTROL,		VK_RCONTROL,	0 },
	{ ITEM_ID_SLASH_PAD,	DIK_DIVIDE,			VK_DIVIDE,		0 },
	{ ITEM_ID_PRTSCR,		DIK_SYSRQ,			0,				0 },
	{ ITEM_ID_RALT, 		DIK_RMENU,			VK_RMENU,		0 },
	{ ITEM_ID_HOME, 		DIK_HOME,			VK_HOME,		0 },
	{ ITEM_ID_UP,			DIK_UP,				VK_UP,			0 },
	{ ITEM_ID_PGUP, 		DIK_PRIOR,			VK_PRIOR,		0 },
	{ ITEM_ID_LEFT, 		DIK_LEFT,			VK_LEFT,		0 },
	{ ITEM_ID_RIGHT,		DIK_RIGHT,			VK_RIGHT,		0 },
	{ ITEM_ID_END,			DIK_END,			VK_END, 		0 },
	{ ITEM_ID_DOWN, 		DIK_DOWN,			VK_DOWN,		0 },
	{ ITEM_ID_PGDN, 		DIK_NEXT,			VK_NEXT,		0 },
	{ ITEM_ID_INSERT,		DIK_INSERT,			VK_INSERT,		0 },
	{ ITEM_ID_DEL,			DIK_DELETE,			VK_DELETE,		0 },
	{ ITEM_ID_LWIN, 		DIK_LWIN,			VK_LWIN,		0 },
	{ ITEM_ID_RWIN, 		DIK_RWIN,			VK_RWIN,		0 },
	{ ITEM_ID_MENU, 		DIK_APPS,			VK_APPS,		0 },
	{ ITEM_ID_PAUSE,		DIK_PAUSE,			VK_PAUSE,		0 },
	{ ITEM_ID_CANCEL,		0,					VK_CANCEL,		0 },

	// New keys introduced in Windows 2000. These have no MAME codes to
	// preserve compatibility with old config files that may refer to them
	// as e.g. FORWARD instead of e.g. KEYCODE_WEBFORWARD. They need table
	// entries anyway because otherwise they aren't recognized when
	// GetAsyncKeyState polling is used (as happens currently when MAME is
	// paused). Some codes are missing because the mapping to vkey codes
	// isn't clear, and MapVirtualKey is no help.

	{ ITEM_ID_OTHER_SWITCH,	DIK_MUTE,			VK_VOLUME_MUTE,			0 },
	{ ITEM_ID_OTHER_SWITCH,	DIK_VOLUMEDOWN,		VK_VOLUME_DOWN,			0 },
	{ ITEM_ID_OTHER_SWITCH,	DIK_VOLUMEUP,		VK_VOLUME_UP,			0 },
	{ ITEM_ID_OTHER_SWITCH,	DIK_WEBHOME,		VK_BROWSER_HOME,		0 },
	{ ITEM_ID_OTHER_SWITCH,	DIK_WEBSEARCH,		VK_BROWSER_SEARCH,		0 },
	{ ITEM_ID_OTHER_SWITCH,	DIK_WEBFAVORITES,	VK_BROWSER_FAVORITES,	0 },
	{ ITEM_ID_OTHER_SWITCH,	DIK_WEBREFRESH,		VK_BROWSER_REFRESH,		0 },
	{ ITEM_ID_OTHER_SWITCH,	DIK_WEBSTOP,		VK_BROWSER_STOP,		0 },
	{ ITEM_ID_OTHER_SWITCH,	DIK_WEBFORWARD,		VK_BROWSER_FORWARD,		0 },
	{ ITEM_ID_OTHER_SWITCH,	DIK_WEBBACK,		VK_BROWSER_BACK,		0 },
	{ ITEM_ID_OTHER_SWITCH,	DIK_MAIL,			VK_LAUNCH_MAIL,			0 },
	{ ITEM_ID_OTHER_SWITCH,	DIK_MEDIASELECT,	VK_LAUNCH_MEDIA_SELECT,	0 },
};


//============================================================
//  INLINE FUNCTIONS
//============================================================

INLINE void poll_if_necessary(running_machine *machine)
{
	// make sure we poll at least once every 1/4 second
	if (GetTickCount() > last_poll + 1000 / 4)
		wininput_poll(machine);
}


INLINE input_item_id keyboard_map_scancode_to_itemid(int scancode)
{
	int tablenum;

	// scan the table for a match
	for (tablenum = 0; tablenum < ARRAY_LENGTH(win_key_trans_table); tablenum++)
		if (win_key_trans_table[tablenum][DI_KEY] == scancode)
			return (input_item_id)win_key_trans_table[tablenum][MAME_KEY];

	// default to an "other" switch
	return ITEM_ID_OTHER_SWITCH;
}


INLINE INT32 normalize_absolute_axis(INT32 raw, INT32 rawmin, INT32 rawmax)
{
	INT32 center = (rawmax + rawmin) / 2;

	// make sure we have valid data
	if (rawmin >= rawmax)
		return raw;

	// above center
	if (raw >= center)
	{
		INT32 result = (INT64)(raw - center) * (INT64)INPUT_ABSOLUTE_MAX / (INT64)(rawmax - center);
		return MIN(result, INPUT_ABSOLUTE_MAX);
	}

	// below center
	else
	{
		INT32 result = -((INT64)(center - raw) * (INT64)-INPUT_ABSOLUTE_MIN / (INT64)(center - rawmin));
		return MAX(result, INPUT_ABSOLUTE_MIN);
	}
}



//============================================================
//  wininput_init
//============================================================

void wininput_init(running_machine *machine)
{
	// we need pause and exit callbacks
	machine->add_notifier(MACHINE_NOTIFY_PAUSE, wininput_pause);
	machine->add_notifier(MACHINE_NOTIFY_RESUME, wininput_resume);
	machine->add_notifier(MACHINE_NOTIFY_EXIT, wininput_exit);

	// allocate a lock for input synchronizations, since messages sometimes come from another thread
	input_lock = osd_lock_alloc();
	assert_always(input_lock != NULL, "Failed to allocate input_lock");

	// decode the options
	lightgun_shared_axis_mode = options_get_bool(machine->options(), WINOPTION_DUAL_LIGHTGUN);

	// initialize RawInput and DirectInput (RawInput first so we can fall back)
	rawinput_init(machine);
	dinput_init(machine);
	win32_init(machine);

	// poll once to get the initial states
	input_enabled = TRUE;
	wininput_poll(machine);
}


//============================================================
//  wininput_pause
//============================================================

static void wininput_pause(running_machine &machine)
{
	// keep track of the paused state
	input_paused = true;
}

static void wininput_resume(running_machine &machine)
{
	// keep track of the paused state
	input_paused = false;
}


//============================================================
//  wininput_exit
//============================================================

static void wininput_exit(running_machine &machine)
{
	// acquire the lock and turn off input (this ensures everyone is done)
	osd_lock_acquire(input_lock);
	input_enabled = FALSE;
	osd_lock_release(input_lock);

	// free the lock
	osd_lock_free(input_lock);
}


//============================================================
//  wininput_poll
//============================================================

void wininput_poll(running_machine *machine)
{
	int hasfocus = winwindow_has_focus() && input_enabled;

	// ignore if not enabled
	if (input_enabled)
	{
		// remember when this happened
		last_poll = GetTickCount();

		// periodically process events, in case they're not coming through
		// this also will make sure the mouse state is up-to-date
		winwindow_process_events_periodic(machine);

		// track if mouse/lightgun is enabled, for mouse hiding purposes
		mouse_enabled = input_device_class_enabled(machine, DEVICE_CLASS_MOUSE);
		lightgun_enabled = input_device_class_enabled(machine, DEVICE_CLASS_LIGHTGUN);
	}

	// poll all of the devices
	if (hasfocus)
	{
		device_list_poll_devices(keyboard_list);
		device_list_poll_devices(mouse_list);
		device_list_poll_devices(lightgun_list);
		device_list_poll_devices(joystick_list);
	}
	else
	{
		device_list_reset_devices(keyboard_list);
		device_list_reset_devices(mouse_list);
		device_list_reset_devices(lightgun_list);
		device_list_reset_devices(joystick_list);
	}
}


//============================================================
//  wininput_should_hide_mouse
//============================================================

int wininput_should_hide_mouse(void)
{
	// if we are paused or disabled, no
	if (input_paused || !input_enabled)
		return FALSE;

	// if neither mice nor lightguns enabled in the core, then no
	if (!mouse_enabled && !lightgun_enabled)
		return FALSE;

	// if the window has a menu, no
	if (win_window_list != NULL && win_has_menu(win_window_list))
		return FALSE;

	// otherwise, yes
	return TRUE;
}


//============================================================
//  wininput_handle_mouse_button
//============================================================

BOOL wininput_handle_mouse_button(int button, int down, int x, int y)
{
	device_info *devinfo;

	// ignore if not enabled
	if (!input_enabled)
		return FALSE;

	// only need this for shared axis hack
	if (!lightgun_shared_axis_mode || button >= 4)
		return FALSE;

	// choose a device based on the button
	devinfo = lightgun_list;
	if (button >= 2 && devinfo != NULL)
	{
		button -= 2;
		devinfo = devinfo->next;
	}

	// take the lock
	osd_lock_acquire(input_lock);

	// set the button state
	devinfo->mouse.state.rgbButtons[button] = down ? 0x80 : 0x00;
	if (down)
	{
		RECT client_rect;
		POINT mousepos;

		// get the position relative to the window
		GetClientRect(win_window_list->hwnd, &client_rect);
		mousepos.x = x;
		mousepos.y = y;
		ScreenToClient(win_window_list->hwnd, &mousepos);

		// convert to absolute coordinates
		devinfo->mouse.state.lX = normalize_absolute_axis(mousepos.x, client_rect.left, client_rect.right);
		devinfo->mouse.state.lY = normalize_absolute_axis(mousepos.y, client_rect.top, client_rect.bottom);
	}

	// release the lock
	osd_lock_release(input_lock);
	return TRUE;
}


//============================================================
//  wininput_handle_raw
//============================================================

BOOL wininput_handle_raw(HANDLE device)
{
	BYTE small_buffer[4096];
	LPBYTE data = small_buffer;
	BOOL result = FALSE;
	int size;

	// ignore if not enabled
	if (!input_enabled)
		return result;

	// determine the size of databuffer we need
	if ((*get_rawinput_data)((HRAWINPUT)device, RID_INPUT, NULL, &size, sizeof(RAWINPUTHEADER)) != 0)
		return result;

	// if necessary, allocate a temporary buffer and fetch the data
	if (size > sizeof(small_buffer))
	{
		data = global_alloc_array(BYTE, size);
		if (data == NULL)
			return result;
	}

	// fetch the data and process the appropriate message types
	result = (*get_rawinput_data)((HRAWINPUT)device, RID_INPUT, data, &size, sizeof(RAWINPUTHEADER));
	if (result != 0)
	{
		RAWINPUT *input = (RAWINPUT *)data;

		// handle keyboard input
		if (input->header.dwType == RIM_TYPEKEYBOARD)
		{
			osd_lock_acquire(input_lock);
			rawinput_keyboard_update(input->header.hDevice, &input->data.keyboard);
			osd_lock_release(input_lock);
			result = TRUE;
		}

		// handle mouse input
		else if (input->header.dwType == RIM_TYPEMOUSE)
		{
			osd_lock_acquire(input_lock);
			rawinput_mouse_update(input->header.hDevice, &input->data.mouse);
			osd_lock_release(input_lock);
			result = TRUE;
		}
	}

	// free the temporary buffer and return the result
	if (data != small_buffer)
		global_free(data);
	return result;
}


//============================================================
//  wininput_vkey_for_mame_code
//============================================================

int wininput_vkey_for_mame_code(input_code code)
{
	// only works for keyboard switches
	if (INPUT_CODE_DEVCLASS(code) == DEVICE_CLASS_KEYBOARD && INPUT_CODE_ITEMCLASS(code) == ITEM_CLASS_SWITCH)
	{
		input_item_id id = INPUT_CODE_ITEMID(code);
		int tablenum;

		// scan the table for a match
		for (tablenum = 0; tablenum < ARRAY_LENGTH(win_key_trans_table); tablenum++)
			if (win_key_trans_table[tablenum][MAME_KEY] == id)
				return win_key_trans_table[tablenum][VIRTUAL_KEY];
	}
	return 0;
}


//============================================================
//  osd_customize_mapping_list
//============================================================

void osd_customize_input_type_list(input_type_desc *typelist)
{
	input_type_desc *typedesc;

	// loop over the defaults
	for (typedesc = typelist; typedesc != NULL; typedesc = typedesc->next)
		switch (typedesc->type)
		{
			// disable the config menu if the ALT key is down
			// (allows ALT-TAB to switch between windows apps)
			case IPT_UI_CONFIGURE:
				input_seq_set_5(&typedesc->seq[SEQ_TYPE_STANDARD], KEYCODE_TAB, SEQCODE_NOT, KEYCODE_LALT, SEQCODE_NOT, KEYCODE_RALT);
				break;

			// alt-enter for fullscreen
			case IPT_OSD_1:
				typedesc->token = "TOGGLE_FULLSCREEN";
				typedesc->name = "Toggle Fullscreen";
				input_seq_set_2(&typedesc->seq[SEQ_TYPE_STANDARD], KEYCODE_LALT, KEYCODE_ENTER);
				break;

			case IPT_OSD_2:
				if (ui_use_newui())
				{
					typedesc->token = "TOGGLE_MENUBAR";
					typedesc->name = "Toggle Menu Bar";
					input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD], KEYCODE_ESC);
				}
				break;

			// movies
			case IPT_OSD_3:
				typedesc->token = "MOVIE_RECORD";
				typedesc->name = "Start Movie Recording";
				input_seq_set_2(&typedesc->seq[SEQ_TYPE_STANDARD], KEYCODE_N, KEYCODE_LCONTROL);
				break;
			case IPT_OSD_4:
				typedesc->token = "MOVIE_PLAYBACK";
				typedesc->name = "Start Movie Playback";
				input_seq_set_2(&typedesc->seq[SEQ_TYPE_STANDARD], KEYCODE_R, KEYCODE_LCONTROL);
				break;
			// RAM tools
			case IPT_OSD_5:
				typedesc->token = "RAM_SEARCH";
				typedesc->name = "RAM Search";
				input_seq_set_2(&typedesc->seq[SEQ_TYPE_STANDARD], KEYCODE_F, KEYCODE_LCONTROL);
				break;
			case IPT_OSD_6:
				typedesc->token = "RAM_WATCH";
				typedesc->name = "RAM Watch";
				input_seq_set_2(&typedesc->seq[SEQ_TYPE_STANDARD], KEYCODE_W, KEYCODE_LCONTROL);
				break;

#ifdef MESS
			case IPT_UI_THROTTLE:
				input_seq_set_0(&typedesc->seq[SEQ_TYPE_STANDARD]);
				break;
#endif
		}
}


//============================================================
//  device_list_poll_devices
//============================================================

static void device_list_poll_devices(device_info *devlist_head)
{
	device_info *curdev;

	for (curdev = devlist_head; curdev != NULL; curdev = curdev->next)
		if (curdev->poll != NULL)
			(*curdev->poll)(curdev);
}


//============================================================
//  device_list_reset_devices
//============================================================

static void device_list_reset_devices(device_info *devlist_head)
{
	device_info *curdev;

	for (curdev = devlist_head; curdev != NULL; curdev = curdev->next)
		generic_device_reset(curdev);
}


//============================================================
//  generic_device_alloc
//============================================================

static device_info *generic_device_alloc(running_machine *machine, device_info **devlist_head_ptr, const TCHAR *name)
{
	device_info **curdev_ptr;
	device_info *devinfo;

	// allocate memory for the device object
	devinfo = global_alloc_clear(device_info);
	devinfo->head = devlist_head_ptr;
	devinfo->machine = machine;

	// allocate a UTF8 copy of the name
	devinfo->name = utf8_from_tstring(name);
	if (devinfo->name == NULL)
		goto error;

	// append us to the list
	for (curdev_ptr = devinfo->head; *curdev_ptr != NULL; curdev_ptr = &(*curdev_ptr)->next) ;
	*curdev_ptr = devinfo;

	return devinfo;

error:
	global_free(devinfo);
	return NULL;
}


//============================================================
//  generic_device_free
//============================================================

static void generic_device_free(device_info *devinfo)
{
	device_info **curdev_ptr;

	// remove us from the list
	for (curdev_ptr = devinfo->head; *curdev_ptr != devinfo && *curdev_ptr != NULL; curdev_ptr = &(*curdev_ptr)->next) ;
	if (*curdev_ptr == devinfo)
		*curdev_ptr = devinfo->next;

	// free the copy of the name if present
	if (devinfo->name != NULL)
		osd_free((void *)devinfo->name);
	devinfo->name = NULL;

	// and now free the info
	global_free(devinfo);
}


//============================================================
//  generic_device_index
//============================================================

static int generic_device_index(device_info *devlist_head, device_info *devinfo)
{
	int index = 0;
	while (devlist_head != NULL)
	{
		if (devlist_head == devinfo)
			return index;
		index++;
		devlist_head = devlist_head->next;
	}
	return -1;
}


//============================================================
//  generic_device_reset
//============================================================

static void generic_device_reset(device_info *devinfo)
{
	// keyboard case
	if (devinfo->head == &keyboard_list)
		memset(devinfo->keyboard.state, 0, sizeof(devinfo->keyboard.state));

	// mouse/lightgun case
	else if (devinfo->head == &mouse_list || devinfo->head == &lightgun_list)
		memset(&devinfo->mouse.state, 0, sizeof(devinfo->mouse.state));

	// joystick case
	else if (devinfo->head == &joystick_list)
	{
		int povnum;

		memset(&devinfo->joystick.state, 0, sizeof(devinfo->joystick.state));
		for (povnum = 0; povnum < ARRAY_LENGTH(devinfo->joystick.state.rgdwPOV); povnum++)
			devinfo->joystick.state.rgdwPOV[povnum] = 0xffff;
	}
}


//============================================================
//  generic_button_get_state
//============================================================

static INT32 generic_button_get_state(void *device_internal, void *item_internal)
{
	device_info *devinfo = (device_info *)device_internal;
	BYTE *itemdata = (BYTE *)item_internal;

	// return the current state
	poll_if_necessary(devinfo->machine);
	return *itemdata >> 7;
}


//============================================================
//  generic_axis_get_state
//============================================================

static INT32 generic_axis_get_state(void *device_internal, void *item_internal)
{
	device_info *devinfo = (device_info *)device_internal;
	LONG *axisdata = (LONG *)item_internal;

	// return the current state
	poll_if_necessary(devinfo->machine);
	return *axisdata;
}


//============================================================
//  win32_init
//============================================================

static void win32_init(running_machine *machine)
{
	int gunnum;

	// we don't need any initialization unless we are using shared axis mode for lightguns
	if (!lightgun_shared_axis_mode)
		return;

	// we need an exit callback
	machine->add_notifier(MACHINE_NOTIFY_EXIT, win32_exit);

	// allocate two lightgun devices
	for (gunnum = 0; gunnum < 2; gunnum++)
	{
		static const TCHAR *const gun_names[] = { TEXT("Shared Axis Gun 1"), TEXT("Shared Axis Gun 2") };
		device_info *devinfo;
		int axisnum, butnum;

		// allocate a device
		devinfo = generic_device_alloc(machine, &lightgun_list, gun_names[gunnum]);
		if (devinfo == NULL)
			break;

		// add the device
		devinfo->device = input_device_add(machine, DEVICE_CLASS_LIGHTGUN, devinfo->name, devinfo);

		// populate the axes
		for (axisnum = 0; axisnum < 2; axisnum++)
		{
			char *name = utf8_from_tstring(default_axis_name[axisnum]);
			input_device_item_add(devinfo->device, name, &devinfo->mouse.state.lX + axisnum, (input_item_id)(ITEM_ID_XAXIS + axisnum), generic_axis_get_state);
			osd_free(name);
		}

		// populate the buttons
		for (butnum = 0; butnum < 2; butnum++)
		{
			char *name = utf8_from_tstring(default_button_name(butnum));
			input_device_item_add(devinfo->device, name, &devinfo->mouse.state.rgbButtons[butnum], (input_item_id)(ITEM_ID_BUTTON1 + butnum), generic_button_get_state);
			osd_free(name);
		}
	}
}


//============================================================
//  win32_exit
//============================================================

static void win32_exit(running_machine &machine)
{
	// skip if we're in shared axis mode
	if (!lightgun_shared_axis_mode)
		return;

	// delete the lightgun devices
	while (lightgun_list != NULL)
		generic_device_free(lightgun_list);
}


//============================================================
//  win32_keyboard_poll
//============================================================

static void win32_keyboard_poll(device_info *devinfo)
{
	int keynum;

	// clear the flag that says we detected a key down via win32
	keyboard_win32_reported_key_down = FALSE;

	// reset the keyboard state and then repopulate
	memset(devinfo->keyboard.state, 0, sizeof(devinfo->keyboard.state));

	// iterate over keys
	for (keynum = 0; keynum < ARRAY_LENGTH(win_key_trans_table); keynum++)
	{
		int vk = win_key_trans_table[keynum][VIRTUAL_KEY];
		if (vk != 0 && (GetAsyncKeyState(vk) & 0x8000) != 0)
		{
			int dik = win_key_trans_table[keynum][DI_KEY];

			// conver the VK code to a scancode (DIK code)
			if (dik != 0)
				devinfo->keyboard.state[dik] = 0x80;

			// set this flag so that we continue to use win32 until all keys are up
			keyboard_win32_reported_key_down = TRUE;
		}
	}
}


//============================================================
//  win32_lightgun_poll
//============================================================

static void win32_lightgun_poll(device_info *devinfo)
{
	INT32 xpos = 0, ypos = 0;
	POINT mousepos;

	// if we are using the shared axis hack, the data is updated via Windows messages only
	if (lightgun_shared_axis_mode)
		return;

	// get the cursor position and transform into final results
	GetCursorPos(&mousepos);
	if (win_window_list != NULL)
	{
		RECT client_rect;

		// get the position relative to the window
		GetClientRect(win_window_list->hwnd, &client_rect);
		ScreenToClient(win_window_list->hwnd, &mousepos);

		// convert to absolute coordinates
		xpos = normalize_absolute_axis(mousepos.x, client_rect.left, client_rect.right);
		ypos = normalize_absolute_axis(mousepos.y, client_rect.top, client_rect.bottom);
	}

	// update the X/Y positions
	devinfo->mouse.state.lX = xpos;
	devinfo->mouse.state.lY = ypos;
}


//============================================================
//  dinput_init
//============================================================

static void dinput_init(running_machine *machine)
{
	HRESULT result;
#if DIRECTINPUT_VERSION >= 0x800
	int didevtype_keyboard = DI8DEVCLASS_KEYBOARD;
	int didevtype_mouse = DI8DEVCLASS_POINTER;
	int didevtype_joystick = DI8DEVCLASS_GAMECTRL;

	dinput_version = DIRECTINPUT_VERSION;
	result = DirectInput8Create(GetModuleHandle(NULL), dinput_version, IID_IDirectInput8, (void **)&dinput, NULL);
	if (result != DI_OK)
	{
		dinput_version = 0;
		return;
	}
#else
	int didevtype_keyboard = DIDEVTYPE_KEYBOARD;
	int didevtype_mouse = DIDEVTYPE_MOUSE;
	int didevtype_joystick = DIDEVTYPE_JOYSTICK;

	// first attempt to initialize DirectInput at the current version
	dinput_version = DIRECTINPUT_VERSION;
	result = DirectInputCreate(GetModuleHandle(NULL), dinput_version, &dinput, NULL);
	if (result != DI_OK)
	{
		// if that fails, try version 5
		dinput_version = 0x0500;
		result = DirectInputCreate(GetModuleHandle(NULL), dinput_version, &dinput, NULL);
		if (result != DI_OK)
		{
			// if that fails, try version 3
			dinput_version = 0x0300;
			result = DirectInputCreate(GetModuleHandle(NULL), dinput_version, &dinput, NULL);
			if (result != DI_OK)
			{
				dinput_version = 0;
				return;
			}
		}
	}
#endif

	mame_printf_verbose("DirectInput: Using DirectInput %d\n", dinput_version >> 8);

	// we need an exit callback
	machine->add_notifier(MACHINE_NOTIFY_EXIT, dinput_exit);

	// initialize keyboard devices, but only if we don't have any yet
	if (keyboard_list == NULL)
	{
		// enumerate the ones we have
		result = IDirectInput_EnumDevices(dinput, didevtype_keyboard, dinput_keyboard_enum, machine, DIEDFL_ATTACHEDONLY);
		if (result != DI_OK)
			fatalerror("DirectInput: Unable to enumerate keyboards (result=%08X)\n", (UINT32)result);
	}

	// initialize mouse & lightgun devices, but only if we don't have any yet
	if (mouse_list == NULL)
	{
		// enumerate the ones we have
		result = IDirectInput_EnumDevices(dinput, didevtype_mouse, dinput_mouse_enum, machine, DIEDFL_ATTACHEDONLY);
		if (result != DI_OK)
			fatalerror("DirectInput: Unable to enumerate mice (result=%08X)\n", (UINT32)result);
	}

	// initialize joystick devices
	result = IDirectInput_EnumDevices(dinput, didevtype_joystick, dinput_joystick_enum, machine, DIEDFL_ATTACHEDONLY);
	if (result != DI_OK)
		fatalerror("DirectInput: Unable to enumerate joysticks (result=%08X)\n", (UINT32)result);
}


//============================================================
//  dinput_exit
//============================================================

static void dinput_exit(running_machine &machine)
{
	// release all our devices
	while (joystick_list != NULL && joystick_list->dinput.device != NULL)
		dinput_device_release(joystick_list);
	while (lightgun_list != NULL)
		generic_device_free(lightgun_list);
	while (mouse_list != NULL && mouse_list->dinput.device != NULL)
		dinput_device_release(mouse_list);
	while (keyboard_list != NULL && keyboard_list->dinput.device != NULL)
		dinput_device_release(keyboard_list);

	// release DirectInput
	if (dinput != NULL)
		IDirectInput_Release(dinput);
	dinput = NULL;
}


//============================================================
//  dinput_set_dword_property
//============================================================

static HRESULT dinput_set_dword_property(LPDIRECTINPUTDEVICE device, REFGUID property_guid, DWORD object, DWORD how, DWORD value)
{
	DIPROPDWORD dipdw;

	dipdw.diph.dwSize       = sizeof(dipdw);
	dipdw.diph.dwHeaderSize = sizeof(dipdw.diph);
	dipdw.diph.dwObj        = object;
	dipdw.diph.dwHow        = how;
	dipdw.dwData            = value;

	return IDirectInputDevice_SetProperty(device, property_guid, &dipdw.diph);
}


//============================================================
//  dinput_device_create
//============================================================

static device_info *dinput_device_create(running_machine *machine, device_info **devlist_head_ptr, LPCDIDEVICEINSTANCE instance, LPCDIDATAFORMAT format1, LPCDIDATAFORMAT format2, DWORD cooperative_level)
{
	device_info *devinfo;
	HRESULT result;

	// allocate memory for the device object
	devinfo = generic_device_alloc(machine, devlist_head_ptr, instance->tszInstanceName);

	// attempt to create a device
	result = IDirectInput_CreateDevice(dinput, WRAP_REFIID(instance->guidInstance), &devinfo->dinput.device, NULL);
	if (result != DI_OK)
		goto error;

	// try to get a version 2 device for it
	result = IDirectInputDevice_QueryInterface(devinfo->dinput.device, WRAP_REFIID(IID_IDirectInputDevice2), (void **)&devinfo->dinput.device2);
	if (result != DI_OK)
		devinfo->dinput.device2 = NULL;

	// get the caps
	devinfo->dinput.caps.dwSize = STRUCTSIZE(DIDEVCAPS);
	result = IDirectInputDevice_GetCapabilities(devinfo->dinput.device, &devinfo->dinput.caps);
	if (result != DI_OK)
		goto error;

	// attempt to set the data format
	devinfo->dinput.format = format1;
	result = IDirectInputDevice_SetDataFormat(devinfo->dinput.device, devinfo->dinput.format);
	if (result != DI_OK)
	{
		// use the secondary format if available
		if (format2 != NULL)
		{
			devinfo->dinput.format = format2;
			result = IDirectInputDevice_SetDataFormat(devinfo->dinput.device, devinfo->dinput.format);
		}
		if (result != DI_OK)
			goto error;
	}

	// set the cooperative level
	result = IDirectInputDevice_SetCooperativeLevel(devinfo->dinput.device, win_window_list->hwnd, cooperative_level);
	if (result != DI_OK)
		goto error;
	return devinfo;

error:
	dinput_device_release(devinfo);
	return NULL;
}


//============================================================
//  dinput_device_release
//============================================================

static void dinput_device_release(device_info *devinfo)
{
	// release the version 2 device if present
	if (devinfo->dinput.device2 != NULL)
		IDirectInputDevice_Release(devinfo->dinput.device2);
	devinfo->dinput.device2 = NULL;

	// release the regular device if present
	if (devinfo->dinput.device != NULL)
		IDirectInputDevice_Release(devinfo->dinput.device);
	devinfo->dinput.device = NULL;

	// free the item list
	generic_device_free(devinfo);
}


//============================================================
//  dinput_device_item_name
//============================================================

static char *dinput_device_item_name(device_info *devinfo, int offset, const TCHAR *defstring, const TCHAR *suffix)
{
	DIDEVICEOBJECTINSTANCE instance = { 0 };
	const TCHAR *namestring = instance.tszName;
	TCHAR *combined;
	HRESULT result;
	char *utf8;

	// query the key name
	instance.dwSize = STRUCTSIZE(DIDEVICEOBJECTINSTANCE);
	result = IDirectInputDevice_GetObjectInfo(devinfo->dinput.device, &instance, offset, DIPH_BYOFFSET);

	// if we got an error and have no default string, just return NULL
	if (result != DI_OK)
	{
		if (defstring == NULL)
			return NULL;
		namestring = defstring;
	}

	// if no suffix, return as-is
	if (suffix == NULL)
		return utf8_from_tstring(namestring);

	// otherwise, allocate space to add the suffix
	combined = global_alloc_array(TCHAR, _tcslen(namestring) + 1 + _tcslen(suffix) + 1);
	_tcscpy(combined, namestring);
	_tcscat(combined, TEXT(" "));
	_tcscat(combined, suffix);

	// convert to UTF8, free the temporary string, and return
	utf8 = utf8_from_tstring(combined);
	global_free(combined);
	return utf8;
}


//============================================================
//  dinput_device_poll
//============================================================

static HRESULT dinput_device_poll(device_info *devinfo)
{
	HRESULT result;

	// first poll the device, then get the state
	if (devinfo->dinput.device2 != NULL)
		IDirectInputDevice2_Poll(devinfo->dinput.device2);
	result = IDirectInputDevice_GetDeviceState(devinfo->dinput.device, devinfo->dinput.format->dwDataSize, &devinfo->joystick.state);

	// handle lost inputs here
	if (result == DIERR_INPUTLOST || result == DIERR_NOTACQUIRED)
	{
		result = IDirectInputDevice_Acquire(devinfo->dinput.device);
		if (result == DI_OK)
			result = IDirectInputDevice_GetDeviceState(devinfo->dinput.device, devinfo->dinput.format->dwDataSize, &devinfo->joystick.state);
	}

	return result;
}


//============================================================
//  dinput_keyboard_enum
//============================================================

static BOOL CALLBACK dinput_keyboard_enum(LPCDIDEVICEINSTANCE instance, LPVOID ref)
{
	running_machine *machine = (running_machine *)ref;
	device_info *devinfo;
	int keynum;

	// allocate and link in a new device
	devinfo = dinput_device_create(machine, &keyboard_list, instance, &c_dfDIKeyboard, NULL, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if (devinfo == NULL)
		goto exit;

	// add the device
	devinfo->device = input_device_add(machine, DEVICE_CLASS_KEYBOARD, devinfo->name, devinfo);
	devinfo->poll = dinput_keyboard_poll;

	// populate it
	for (keynum = 0; keynum < MAX_KEYS; keynum++)
	{
		input_item_id itemid = keyboard_map_scancode_to_itemid(keynum);
		TCHAR defname[20];
		char *name;

		// generate/fetch the name
		_sntprintf(defname, ARRAY_LENGTH(defname), TEXT("Scan%03d"), keynum);
		name = dinput_device_item_name(devinfo, keynum, defname, NULL);

		// add the item to the device
		input_device_item_add(devinfo->device, name, &devinfo->keyboard.state[keynum], itemid, generic_button_get_state);
		osd_free(name);
	}

exit:
	return DIENUM_CONTINUE;
}


//============================================================
//  dinput_keyboard_poll
//============================================================

static void dinput_keyboard_poll(device_info *devinfo)
{
	HRESULT result = dinput_device_poll(devinfo);

	// for the first device, if we errored, or if we previously reported win32 keys,
	// then ignore the dinput state and poll using win32
	if (devinfo == keyboard_list && (result != DI_OK || keyboard_win32_reported_key_down))
		win32_keyboard_poll(devinfo);
}


//============================================================
//  dinput_mouse_enum
//============================================================

static BOOL CALLBACK dinput_mouse_enum(LPCDIDEVICEINSTANCE instance, LPVOID ref)
{
	device_info *devinfo, *guninfo = NULL;
	running_machine *machine = (running_machine *)ref;
	int axisnum, butnum;
	HRESULT result;

	// allocate and link in a new device
	devinfo = dinput_device_create(machine, &mouse_list, instance, &c_dfDIMouse2, &c_dfDIMouse, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if (devinfo == NULL)
		goto exit;

	// allocate a second device for the gun (unless we are using the shared axis mode)
	// we only support a single gun in dinput mode, so only add one
	if (!lightgun_shared_axis_mode && devinfo == mouse_list)
	{
		guninfo = generic_device_alloc(machine, &lightgun_list, instance->tszInstanceName);
		if (guninfo == NULL)
			goto exit;
	}

	// set relative mode on the mouse device
	result = dinput_set_dword_property(devinfo->dinput.device, DIPROP_AXISMODE, 0, DIPH_DEVICE, DIPROPAXISMODE_REL);
	if (result != DI_OK)
	{
		mame_printf_error("DirectInput: Unable to set relative mode for mouse %d (%s)\n", generic_device_index(mouse_list, devinfo), devinfo->name);
		goto error;
	}

	// add the device
	devinfo->device = input_device_add(machine, DEVICE_CLASS_MOUSE, devinfo->name, devinfo);
	devinfo->poll = dinput_mouse_poll;
	if (guninfo != NULL)
	{
		guninfo->device = input_device_add(machine, DEVICE_CLASS_LIGHTGUN, guninfo->name, guninfo);
		guninfo->poll = win32_lightgun_poll;
	}

	// cap the number of axes and buttons based on the format
	devinfo->dinput.caps.dwAxes = MIN(devinfo->dinput.caps.dwAxes, 3);
	devinfo->dinput.caps.dwButtons = MIN(devinfo->dinput.caps.dwButtons, (devinfo->dinput.format == &c_dfDIMouse) ? 4 : 8);

	// populate the axes
	for (axisnum = 0; axisnum < devinfo->dinput.caps.dwAxes; axisnum++)
	{
		char *name = dinput_device_item_name(devinfo, offsetof(DIMOUSESTATE, lX) + axisnum * sizeof(LONG), default_axis_name[axisnum], NULL);

		// add to the mouse device and optionally to the gun device as well
		input_device_item_add(devinfo->device, name, &devinfo->mouse.state.lX + axisnum, (input_item_id)(ITEM_ID_XAXIS + axisnum), generic_axis_get_state);
		if (guninfo != NULL && axisnum < 2)
			input_device_item_add(guninfo->device, name, &guninfo->mouse.state.lX + axisnum, (input_item_id)(ITEM_ID_XAXIS + axisnum), generic_axis_get_state);

		osd_free(name);
	}

	// populate the buttons
	for (butnum = 0; butnum < devinfo->dinput.caps.dwButtons; butnum++)
	{
		FPTR offset = (FPTR)(&((DIMOUSESTATE *)NULL)->rgbButtons[butnum]);
		char *name = dinput_device_item_name(devinfo, offset, default_button_name(butnum), NULL);

		// add to the mouse device and optionally to the gun device as well
		// note that the gun device points to the mouse buttons rather than its own
		input_device_item_add(devinfo->device, name, &devinfo->mouse.state.rgbButtons[butnum], (input_item_id)(ITEM_ID_BUTTON1 + butnum), generic_button_get_state);
		if (guninfo != NULL)
			input_device_item_add(guninfo->device, name, &devinfo->mouse.state.rgbButtons[butnum], (input_item_id)(ITEM_ID_BUTTON1 + butnum), generic_button_get_state);

		osd_free(name);
	}

exit:
	return DIENUM_CONTINUE;

error:
	if (guninfo != NULL)
		generic_device_free(guninfo);
	if (devinfo != NULL)
		dinput_device_release(devinfo);
	goto exit;
}


//============================================================
//  dinput_mouse_poll
//============================================================

static void dinput_mouse_poll(device_info *devinfo)
{
	// poll
	dinput_device_poll(devinfo);

	// scale the axis data
	devinfo->mouse.state.lX *= INPUT_RELATIVE_PER_PIXEL;
	devinfo->mouse.state.lY *= INPUT_RELATIVE_PER_PIXEL;
	devinfo->mouse.state.lZ *= INPUT_RELATIVE_PER_PIXEL;
}


//============================================================
//  dinput_joystick_enum
//============================================================

static BOOL CALLBACK dinput_joystick_enum(LPCDIDEVICEINSTANCE instance, LPVOID ref)
{
	DWORD cooperative_level = (HAS_WINDOW_MENU ? DISCL_BACKGROUND : DISCL_FOREGROUND) | DISCL_EXCLUSIVE;
	int axisnum, axiscount, povnum, butnum;
	running_machine *machine = (running_machine *)ref;
	device_info *devinfo;
	HRESULT result;

	// allocate and link in a new device
	devinfo = dinput_device_create(machine, &joystick_list, instance, &c_dfDIJoystick, NULL, cooperative_level);
	if (devinfo == NULL)
		goto exit;

	// set absolute mode
	result = dinput_set_dword_property(devinfo->dinput.device, DIPROP_AXISMODE, 0, DIPH_DEVICE, DIPROPAXISMODE_ABS);
	if (result != DI_OK)
		mame_printf_warning("DirectInput: Unable to set absolute mode for joystick %d (%s)\n", generic_device_index(joystick_list, devinfo), devinfo->name);

	// turn off deadzone; we do our own calculations
	result = dinput_set_dword_property(devinfo->dinput.device, DIPROP_DEADZONE, 0, DIPH_DEVICE, 0);
	if (result != DI_OK)
		mame_printf_warning("DirectInput: Unable to reset deadzone for joystick %d (%s)\n", generic_device_index(joystick_list, devinfo), devinfo->name);

	// turn off saturation; we do our own calculations
	result = dinput_set_dword_property(devinfo->dinput.device, DIPROP_SATURATION, 0, DIPH_DEVICE, 10000);
	if (result != DI_OK)
		mame_printf_warning("DirectInput: Unable to reset saturation for joystick %d (%s)\n", generic_device_index(joystick_list, devinfo), devinfo->name);

	// cap the number of axes, POVs, and buttons based on the format
	devinfo->dinput.caps.dwAxes = MIN(devinfo->dinput.caps.dwAxes, 8);
	devinfo->dinput.caps.dwPOVs = MIN(devinfo->dinput.caps.dwPOVs, 4);
	devinfo->dinput.caps.dwButtons = MIN(devinfo->dinput.caps.dwButtons, 128);

	// add the device
	devinfo->device = input_device_add(machine, DEVICE_CLASS_JOYSTICK, devinfo->name, devinfo);
	devinfo->poll = dinput_joystick_poll;

	// populate the axes
	for (axisnum = axiscount = 0; axiscount < devinfo->dinput.caps.dwAxes && axisnum < 8; axisnum++)
	{
		DIPROPRANGE dipr;
		char *name;

		// fetch the range of this axis
		dipr.diph.dwSize = sizeof(dipr);
		dipr.diph.dwHeaderSize = sizeof(dipr.diph);
		dipr.diph.dwObj = offsetof(DIJOYSTATE2, lX) + axisnum * sizeof(LONG);
		dipr.diph.dwHow = DIPH_BYOFFSET;
		result = IDirectInputDevice_GetProperty(devinfo->dinput.device, DIPROP_RANGE, &dipr.diph);
		if (result != DI_OK)
			continue;
		devinfo->joystick.rangemin[axisnum] = dipr.lMin;
		devinfo->joystick.rangemax[axisnum] = dipr.lMax;

		// populate the item description as well
		name = dinput_device_item_name(devinfo, offsetof(DIJOYSTATE2, lX) + axisnum * sizeof(LONG), default_axis_name[axisnum], NULL);
		input_device_item_add(devinfo->device, name, &devinfo->joystick.state.lX + axisnum, (input_item_id)(ITEM_ID_XAXIS + axisnum), generic_axis_get_state);
		osd_free(name);

		axiscount++;
	}

	// populate the POVs
	for (povnum = 0; povnum < devinfo->dinput.caps.dwPOVs; povnum++)
	{
		char *name;

		// left
		name = dinput_device_item_name(devinfo, offsetof(DIJOYSTATE2, rgdwPOV) + povnum * sizeof(DWORD), default_pov_name(povnum), TEXT("L"));
		input_device_item_add(devinfo->device, name, (void *)(FPTR)(povnum * 4 + POVDIR_LEFT), ITEM_ID_OTHER_SWITCH, dinput_joystick_pov_get_state);
		osd_free(name);

		// right
		name = dinput_device_item_name(devinfo, offsetof(DIJOYSTATE2, rgdwPOV) + povnum * sizeof(DWORD), default_pov_name(povnum), TEXT("R"));
		input_device_item_add(devinfo->device, name, (void *)(FPTR)(povnum * 4 + POVDIR_RIGHT), ITEM_ID_OTHER_SWITCH, dinput_joystick_pov_get_state);
		osd_free(name);

		// up
		name = dinput_device_item_name(devinfo, offsetof(DIJOYSTATE2, rgdwPOV) + povnum * sizeof(DWORD), default_pov_name(povnum), TEXT("U"));
		input_device_item_add(devinfo->device, name, (void *)(FPTR)(povnum * 4 + POVDIR_UP), ITEM_ID_OTHER_SWITCH, dinput_joystick_pov_get_state);
		osd_free(name);

		// down
		name = dinput_device_item_name(devinfo, offsetof(DIJOYSTATE2, rgdwPOV) + povnum * sizeof(DWORD), default_pov_name(povnum), TEXT("D"));
		input_device_item_add(devinfo->device, name, (void *)(FPTR)(povnum * 4 + POVDIR_DOWN), ITEM_ID_OTHER_SWITCH, dinput_joystick_pov_get_state);
		osd_free(name);
	}

	// populate the buttons
	for (butnum = 0; butnum < devinfo->dinput.caps.dwButtons; butnum++)
	{
		FPTR offset = (FPTR)(&((DIJOYSTATE2 *)NULL)->rgbButtons[butnum]);
		char *name = dinput_device_item_name(devinfo, offset, default_button_name(butnum), NULL);
		input_device_item_add(devinfo->device, name, &devinfo->joystick.state.rgbButtons[butnum], (butnum < 16) ? (input_item_id)(ITEM_ID_BUTTON1 + butnum) : ITEM_ID_OTHER_SWITCH, generic_button_get_state);
		osd_free(name);
	}

exit:
	return DIENUM_CONTINUE;
}


//============================================================
//  dinput_joystick_poll
//============================================================

static void dinput_joystick_poll(device_info *devinfo)
{
	int axisnum;

	// poll the device first
	dinput_device_poll(devinfo);

	// normalize axis values
	for (axisnum = 0; axisnum < 8; axisnum++)
	{
		LONG *axis = (&devinfo->joystick.state.lX) + axisnum;
		*axis = normalize_absolute_axis(*axis, devinfo->joystick.rangemin[axisnum], devinfo->joystick.rangemax[axisnum]);
	}
}


//============================================================
//  dinput_joystick_pov_get_state
//============================================================

static INT32 dinput_joystick_pov_get_state(void *device_internal, void *item_internal)
{
	device_info *devinfo = (device_info *)device_internal;
	int povnum = (FPTR)item_internal / 4;
	int povdir = (FPTR)item_internal % 4;
	INT32 result = 0;
	DWORD pov;

	// get the current state
	poll_if_necessary(devinfo->machine);
	pov = devinfo->joystick.state.rgdwPOV[povnum];

	// if invalid, return 0
	if ((pov & 0xffff) == 0xffff)
		return result;

	// return the current state
	switch (povdir)
	{
		case POVDIR_LEFT:	result = (pov >= 22500 && pov <= 31500);	break;
		case POVDIR_RIGHT:	result = (pov >= 4500 && pov <= 13500);		break;
		case POVDIR_UP:		result = (pov >= 31500 || pov <= 4500);		break;
		case POVDIR_DOWN:	result = (pov >= 13500 && pov <= 22500);	break;
	}
	return result;
}


//============================================================
//  rawinput_init
//============================================================

static void rawinput_init(running_machine *machine)
{
	RAWINPUTDEVICELIST *devlist = NULL;
	int device_count, devnum, regcount;
	RAWINPUTDEVICE reglist[2];
	HMODULE user32;

	// we need pause and exit callbacks
	machine->add_notifier(MACHINE_NOTIFY_EXIT, rawinput_exit);

	// look in user32 for the raw input APIs
	user32 = LoadLibrary(TEXT("user32.dll"));
	if (user32 == NULL)
		goto error;

	// look up the entry points
	register_rawinput_devices = (register_rawinput_devices_ptr)GetProcAddress(user32, "RegisterRawInputDevices");
	get_rawinput_device_list = (get_rawinput_device_list_ptr)GetProcAddress(user32, "GetRawInputDeviceList");
	get_rawinput_device_info = (get_rawinput_device_info_ptr)GetProcAddress(user32, "GetRawInputDeviceInfo" UNICODE_SUFFIX);
	get_rawinput_data = (get_rawinput_data_ptr)GetProcAddress(user32, "GetRawInputData");
	if (register_rawinput_devices == NULL || get_rawinput_device_list == NULL || get_rawinput_device_info == NULL || get_rawinput_data == NULL)
		goto error;
	mame_printf_verbose("RawInput: APIs detected\n");

	// get the number of devices, allocate a device list, and fetch it
	if ((*get_rawinput_device_list)(NULL, &device_count, sizeof(*devlist)) != 0)
		goto error;
	if (device_count == 0)
		goto error;
	devlist = global_alloc_array(RAWINPUTDEVICELIST, device_count);
	if ((*get_rawinput_device_list)(devlist, &device_count, sizeof(*devlist)) == -1)
		goto error;

	// iterate backwards through devices; new devices are added at the head
	for (devnum = device_count - 1; devnum >= 0; devnum--)
	{
		RAWINPUTDEVICELIST *device = &devlist[devnum];

		// handle keyboards
		if (device->dwType == RIM_TYPEKEYBOARD && !FORCE_DIRECTINPUT)
			rawinput_keyboard_enum(machine, device);

		// handle mice
		else if (device->dwType == RIM_TYPEMOUSE && !FORCE_DIRECTINPUT)
			rawinput_mouse_enum(machine, device);
	}

	// finally, register to recieve raw input WM_INPUT messages
	regcount = 0;
	if (keyboard_list != NULL)
	{
		reglist[regcount].usUsagePage = 0x01;
		reglist[regcount].usUsage = 0x06;
		reglist[regcount].dwFlags = RIDEV_INPUTSINK;
		reglist[regcount].hwndTarget = win_window_list->hwnd;
		regcount++;
	}
	if (mouse_list != NULL)
	{
		reglist[regcount].usUsagePage = 0x01;
		reglist[regcount].usUsage = 0x02;
		reglist[regcount].dwFlags = 0;
		reglist[regcount].hwndTarget = win_window_list->hwnd;
		regcount++;
	}

	// if the registration fails, we need to back off
	if (regcount > 0)
		if (!(*register_rawinput_devices)(reglist, regcount, sizeof(reglist[0])))
			goto error;

	global_free(devlist);
	return;

error:
	if (devlist != NULL)
		global_free(devlist);
}


//============================================================
//  rawinput_exit
//============================================================

static void rawinput_exit(running_machine &machine)
{
	// release all our devices
	while (lightgun_list != NULL && lightgun_list->rawinput.device != NULL)
		rawinput_device_release(lightgun_list);
	while (mouse_list != NULL && mouse_list->rawinput.device != NULL)
		rawinput_device_release(mouse_list);
	while (keyboard_list != NULL && keyboard_list->rawinput.device != NULL)
		rawinput_device_release(keyboard_list);
}


//============================================================
//  rawinput_device_create
//============================================================

static device_info *rawinput_device_create(running_machine *machine, device_info **devlist_head_ptr, PRAWINPUTDEVICELIST device)
{
	device_info *devinfo = NULL;
	TCHAR *tname = NULL;
	INT name_length;

	// determine the length of the device name, allocate it, and fetch it
	if ((*get_rawinput_device_info)(device->hDevice, RIDI_DEVICENAME, NULL, &name_length) != 0)
		goto error;
	tname = global_alloc_array(TCHAR, name_length);
	if ((*get_rawinput_device_info)(device->hDevice, RIDI_DEVICENAME, tname, &name_length) == -1)
		goto error;

	// if this is an RDP name, skip it
	if (_tcsstr(tname, TEXT("Root#RDP_")) != NULL)
		goto error;

	// improve the name and then allocate a device
	tname = rawinput_device_improve_name(tname);
	devinfo = generic_device_alloc(machine, devlist_head_ptr, tname);
	global_free(tname);

	// copy the handle
	devinfo->rawinput.device = device->hDevice;
	return devinfo;

error:
	if (tname != NULL)
		global_free(tname);
	if (devinfo != NULL)
		rawinput_device_release(devinfo);
	return NULL;
}


//============================================================
//  rawinput_device_release
//============================================================

static void rawinput_device_release(device_info *devinfo)
{
	// free the item list
	generic_device_free(devinfo);
}


//============================================================
//  rawinput_device_improve_name
//============================================================

static TCHAR *rawinput_device_improve_name(TCHAR *name)
{
	static const TCHAR usbbasepath[] = TEXT("SYSTEM\\CurrentControlSet\\Enum\\USB");
	static const TCHAR basepath[] = TEXT("SYSTEM\\CurrentControlSet\\Enum\\");
	TCHAR *regstring = NULL;
	TCHAR *parentid = NULL;
	TCHAR *regpath = NULL;
	const TCHAR *chsrc;
	HKEY regkey = NULL;
	int usbindex;
	TCHAR *chdst;
	LONG result;

	// The RAW name received is formatted as:
	//   \??\type-id#hardware-id#instance-id#{DeviceClasses-id}
	// XP starts with "\??\"
	// Vista64 starts with "\\?\"

	// ensure the name is something we can handle
	if (_tcsncmp(name, TEXT("\\\\?\\"), 4) != 0 && _tcsncmp(name, TEXT("\\??\\"), 4) != 0)
		return name;

	// allocate a temporary string and concatenate the base path plus the name
	regpath = global_alloc_array(TCHAR, _tcslen(basepath) + 1 + _tcslen(name));
	_tcscpy(regpath, basepath);
	chdst = regpath + _tcslen(regpath);

	// convert all # to \ in the name
	for (chsrc = name + 4; *chsrc != 0; chsrc++)
		*chdst++ = (*chsrc == '#') ? '\\' : *chsrc;
	*chdst = 0;

	// remove the final chunk
	chdst = _tcsrchr(regpath, '\\');
	if (chdst == NULL)
		goto exit;
	*chdst = 0;

	// now try to open the registry key
	result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, regpath, 0, KEY_READ, &regkey);
	if (result != ERROR_SUCCESS)
		goto exit;

	// fetch the device description; if it exists, we are finished
	regstring = reg_query_string(regkey, TEXT("DeviceDesc"));
	if (regstring != NULL)
		goto convert;

	// close this key
	RegCloseKey(regkey);
	regkey = NULL;

	// if the key name does not contain "HID", it's not going to be in the USB tree; give up
	if (_tcsstr(regpath, TEXT("HID")) == NULL)
		goto exit;

	// extract the expected parent ID from the regpath
	parentid = _tcsrchr(regpath, '\\');
	if (parentid == NULL)
		goto exit;
	parentid++;

	// open the USB key
	result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, usbbasepath, 0, KEY_READ, &regkey);
	if (result != ERROR_SUCCESS)
		goto exit;

	// enumerate the USB key
	for (usbindex = 0; result == ERROR_SUCCESS && regstring == NULL; usbindex++)
	{
		TCHAR keyname[MAX_PATH];
		DWORD namelen;

		// get the next enumerated subkey and scan it
		namelen = ARRAY_LENGTH(keyname) - 1;
		result = RegEnumKeyEx(regkey, usbindex, keyname, &namelen, NULL, NULL, NULL, NULL);
		if (result == ERROR_SUCCESS)
		{
			LONG subresult;
			int subindex;
			HKEY subkey;

			// open the subkey
			subresult = RegOpenKeyEx(regkey, keyname, 0, KEY_READ, &subkey);
			if (subresult != ERROR_SUCCESS)
				continue;

			// enumerate the subkey
			for (subindex = 0; subresult == ERROR_SUCCESS && regstring == NULL; subindex++)
			{
				// get the next enumerated subkey and scan it
				namelen = ARRAY_LENGTH(keyname) - 1;
				subresult = RegEnumKeyEx(subkey, subindex, keyname, &namelen, NULL, NULL, NULL, NULL);
				if (subresult == ERROR_SUCCESS)
				{
					TCHAR *endparentid;
					LONG endresult;
					HKEY endkey;

					// open this final key
					endresult = RegOpenKeyEx(subkey, keyname, 0, KEY_READ, &endkey);
					if (endresult != ERROR_SUCCESS)
						continue;

					// do we have a match?
					endparentid = reg_query_string(endkey, TEXT("ParentIdPrefix"));
					if (endparentid != NULL && _tcsncmp(parentid, endparentid, _tcslen(endparentid)) == 0)
						regstring = reg_query_string(endkey, TEXT("DeviceDesc"));

					// free memory and close the key
					if (endparentid != NULL)
						global_free(endparentid);
					RegCloseKey(endkey);
				}
			}

			// close the subkey
			RegCloseKey(subkey);
		}
	}

	// if we didn't find anything, go to the exit
	if (regstring == NULL)
		goto exit;

convert:
	// replace the name with the nicer one
	global_free(name);

	// remove anything prior to the final semicolon
	chsrc = _tcsrchr(regstring, ';');
	if (chsrc != NULL)
		chsrc++;
	else
		chsrc = regstring;
	name = global_alloc_array(TCHAR, _tcslen(chsrc) + 1);
	_tcscpy(name, chsrc);

exit:
	if (regstring != NULL)
		global_free(regstring);
	if (regpath != NULL)
		global_free(regpath);
	if (regkey != NULL)
		RegCloseKey(regkey);

	return name;
}


//============================================================
//  rawinput_keyboard_enum
//============================================================

static void rawinput_keyboard_enum(running_machine *machine, PRAWINPUTDEVICELIST device)
{
	device_info *devinfo;
	int keynum;

	// allocate and link in a new device
	devinfo = rawinput_device_create(machine, &keyboard_list, device);
	if (devinfo == NULL)
		return;

	// add the device
	devinfo->device = input_device_add(machine, DEVICE_CLASS_KEYBOARD, devinfo->name, devinfo);

	// populate it
	for (keynum = 0; keynum < MAX_KEYS; keynum++)
	{
		input_item_id itemid = keyboard_map_scancode_to_itemid(keynum);
		TCHAR keyname[100];
		char *name;

		// generate the name
		if (GetKeyNameText(((keynum & 0x7f) << 16) | ((keynum & 0x80) << 17), keyname, ARRAY_LENGTH(keyname)) == 0)
			_sntprintf(keyname, ARRAY_LENGTH(keyname), TEXT("Scan%03d"), keynum);
		name = utf8_from_tstring(keyname);

		// add the item to the device
		input_device_item_add(devinfo->device, name, &devinfo->keyboard.state[keynum], itemid, generic_button_get_state);
		osd_free(name);
	}
}


//============================================================
//  rawinput_keyboard_update
//============================================================

static void rawinput_keyboard_update(HANDLE device, RAWKEYBOARD *data)
{
	device_info *devinfo;

	// find the keyboard in the list and process
	for (devinfo = keyboard_list; devinfo != NULL; devinfo = devinfo->next)
		if (devinfo->rawinput.device == device)
		{
			// determine the full DIK-compatible scancode
			UINT8 scancode = (data->MakeCode & 0x7f) | ((data->Flags & RI_KEY_E0) ? 0x80 : 0x00);

			// scancode 0xaa is a special shift code we need to ignore
			if (scancode == 0xaa)
				break;

			// set or clear the key
			if (!(data->Flags & RI_KEY_BREAK))
				devinfo->keyboard.state[scancode] = 0x80;
			else
				devinfo->keyboard.state[scancode] = 0x00;
			break;
		}
}


//============================================================
//  rawinput_mouse_enum
//============================================================

static void rawinput_mouse_enum(running_machine *machine, PRAWINPUTDEVICELIST device)
{
	device_info *devinfo, *guninfo = NULL;
	int axisnum, butnum;

	// allocate and link in a new mouse device
	devinfo = rawinput_device_create(machine, &mouse_list, device);
	if (devinfo == NULL)
		return;
	devinfo->poll = rawinput_mouse_poll;

	// allocate a second device for the gun (unless we are using the shared axis mode)
	if (!lightgun_shared_axis_mode)
	{
		guninfo = rawinput_device_create(machine, &lightgun_list, device);
		assert(guninfo != NULL);
	}

	// add the device
	devinfo->device = input_device_add(machine, DEVICE_CLASS_MOUSE, devinfo->name, devinfo);
	if (guninfo != NULL)
	{
		guninfo->device = input_device_add(machine, DEVICE_CLASS_LIGHTGUN, guninfo->name, guninfo);
		guninfo->poll = NULL;
	}

	// populate the axes
	for (axisnum = 0; axisnum < 3; axisnum++)
	{
		char *name = utf8_from_tstring(default_axis_name[axisnum]);

		// add to the mouse device and optionally to the gun device as well
		input_device_item_add(devinfo->device, name, &devinfo->mouse.state.lX + axisnum, (input_item_id)(ITEM_ID_XAXIS + axisnum), generic_axis_get_state);
		if (guninfo != NULL && axisnum < 2)
			input_device_item_add(guninfo->device, name, &guninfo->mouse.state.lX + axisnum, (input_item_id)(ITEM_ID_XAXIS + axisnum), generic_axis_get_state);

		osd_free(name);
	}

	// populate the buttons
	for (butnum = 0; butnum < 5; butnum++)
	{
		char *name = utf8_from_tstring(default_button_name(butnum));

		// add to the mouse device and optionally to the gun device as well
		input_device_item_add(devinfo->device, name, &devinfo->mouse.state.rgbButtons[butnum], (input_item_id)(ITEM_ID_BUTTON1 + butnum), generic_button_get_state);
		if (guninfo != NULL)
			input_device_item_add(guninfo->device, name, &guninfo->mouse.state.rgbButtons[butnum], (input_item_id)(ITEM_ID_BUTTON1 + butnum), generic_button_get_state);

		osd_free(name);
	}
}


//============================================================
//  rawinput_mouse_update
//============================================================

static void rawinput_mouse_update(HANDLE device, RAWMOUSE *data)
{
	device_info *devlist = (data->usFlags & MOUSE_MOVE_ABSOLUTE) ? lightgun_list : mouse_list;
	device_info *devinfo;

	// find the mouse in the list and process
	for (devinfo = devlist; devinfo != NULL; devinfo = devinfo->next)
		if (devinfo->rawinput.device == device)
		{
			// if we got relative data, update it as a mouse
			if (!(data->usFlags & MOUSE_MOVE_ABSOLUTE))
			{
				devinfo->mouse.raw_x += data->lLastX * INPUT_RELATIVE_PER_PIXEL;
				devinfo->mouse.raw_y += data->lLastY * INPUT_RELATIVE_PER_PIXEL;

				// update zaxis
				if (data->usButtonFlags & RI_MOUSE_WHEEL)
					devinfo->mouse.raw_z += (INT16)data->usButtonData * INPUT_RELATIVE_PER_PIXEL;
			}

			// otherwise, update it as a lightgun
			else
			{
				devinfo->mouse.state.lX = normalize_absolute_axis(data->lLastX, 0, 0xffff);
				devinfo->mouse.state.lY = normalize_absolute_axis(data->lLastY, 0, 0xffff);
			}

			// update the button states; always update the corresponding mouse buttons
			if (data->usButtonFlags & RI_MOUSE_BUTTON_1_DOWN) devinfo->mouse.state.rgbButtons[0] = 0x80;
			if (data->usButtonFlags & RI_MOUSE_BUTTON_1_UP)   devinfo->mouse.state.rgbButtons[0] = 0x00;
			if (data->usButtonFlags & RI_MOUSE_BUTTON_2_DOWN) devinfo->mouse.state.rgbButtons[1] = 0x80;
			if (data->usButtonFlags & RI_MOUSE_BUTTON_2_UP)   devinfo->mouse.state.rgbButtons[1] = 0x00;
			if (data->usButtonFlags & RI_MOUSE_BUTTON_3_DOWN) devinfo->mouse.state.rgbButtons[2] = 0x80;
			if (data->usButtonFlags & RI_MOUSE_BUTTON_3_UP)   devinfo->mouse.state.rgbButtons[2] = 0x00;
			if (data->usButtonFlags & RI_MOUSE_BUTTON_4_DOWN) devinfo->mouse.state.rgbButtons[3] = 0x80;
			if (data->usButtonFlags & RI_MOUSE_BUTTON_4_UP)   devinfo->mouse.state.rgbButtons[3] = 0x00;
			if (data->usButtonFlags & RI_MOUSE_BUTTON_5_DOWN) devinfo->mouse.state.rgbButtons[4] = 0x80;
			if (data->usButtonFlags & RI_MOUSE_BUTTON_5_UP)   devinfo->mouse.state.rgbButtons[4] = 0x00;
			break;
		}
}


//============================================================
//  rawinput_mouse_poll
//============================================================

static void rawinput_mouse_poll(device_info *devinfo)
{
	poll_if_necessary(devinfo->machine);

	// copy the accumulated raw state to the actual state
	osd_lock_acquire(input_lock);
	devinfo->mouse.state.lX = devinfo->mouse.raw_x;
	devinfo->mouse.state.lY = devinfo->mouse.raw_y;
	devinfo->mouse.state.lZ = devinfo->mouse.raw_z;
	devinfo->mouse.raw_x = 0;
	devinfo->mouse.raw_y = 0;
	devinfo->mouse.raw_z = 0;
	osd_lock_release(input_lock);
}


//============================================================
//  reg_query_string
//============================================================

static TCHAR *reg_query_string(HKEY key, const TCHAR *path)
{
	TCHAR *buffer;
	DWORD datalen;
	LONG result;

	// first query to get the length
	result = RegQueryValueEx(key, path, NULL, NULL, NULL, &datalen);
	if (result != ERROR_SUCCESS)
		return NULL;

	// allocate a buffer
	buffer = global_alloc_array(TCHAR, datalen + sizeof(*buffer));
	buffer[datalen / sizeof(*buffer)] = 0;

	// now get the actual data
	result = RegQueryValueEx(key, path, NULL, NULL, (LPBYTE)buffer, &datalen);
	if (result == ERROR_SUCCESS)
		return buffer;

	// otherwise return a NULL buffer
	global_free(buffer);
	return NULL;
}


//============================================================
//  default_button_name
//============================================================

static const TCHAR *default_button_name(int which)
{
	static TCHAR buffer[20];
	_sntprintf(buffer, ARRAY_LENGTH(buffer), TEXT("B%d"), which);
	return buffer;
}


//============================================================
//  default_pov_name
//============================================================

static const TCHAR *default_pov_name(int which)
{
	static TCHAR buffer[20];
	_sntprintf(buffer, ARRAY_LENGTH(buffer), TEXT("POV%d"), which);
	return buffer;
}
