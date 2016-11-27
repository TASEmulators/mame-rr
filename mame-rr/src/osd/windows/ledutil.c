//============================================================
//
//  ledutil.c - Win32 example code that tracks changing
//  outputs and updates the keyboard LEDs in response
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
//
//  This is sample code. To use it as a starting point, you
//  should do the following:
//
//  1. Change the CLIENT_ID define to something unique.
//
//  2. Change the WINDOW_CLASS and WINDOW_NAME defines to
//  something unique.
//
//  3. Delete all the code from the >8 snip 8< comment and
//  downward.
//
//  4. Implement the following functions:
//
//      output_startup - called at app init time
//      output_shutdown - called before the app exits
//      output_mame_start - called when MAME starts
//      output_mame_stop - called when MAME exits
//      output_set_state - called whenever state changes
//
//============================================================

// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winioctl.h>

// standard C headers
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// MAME output header file
typedef void running_machine;
#include "osdcomm.h"
#include "output.h"



//============================================================
//  DEBUGGING
//============================================================

// note you need to compile as a console app to have any of
// these printfs show up
#define DEBUG_VERSION		0

#if DEBUG_VERSION
#define DEBUG_PRINTF(x)		printf x
#else
#define DEBUG_PRINTF(x)
#endif


//============================================================
//  CONSTANTS
//============================================================

// unique client ID
#define CLIENT_ID							(('M' << 24) | ('L' << 16) | ('E' << 8) | ('D' << 0))

// LED methods
#define LED_METHOD_PS2						0
#define LED_METHOD_USB						1
#define LED_METHOD_WIN9X					2

// window parameters
#define WINDOW_CLASS						TEXT("LEDSample")
#define WINDOW_NAME							TEXT("LEDSample")

// window styles
#define WINDOW_STYLE						WS_OVERLAPPEDWINDOW
#define WINDOW_STYLE_EX						0

// Define the keyboard indicators.
// (Definitions borrowed from ntddkbd.h)

#define IOCTL_KEYBOARD_SET_INDICATORS		CTL_CODE(FILE_DEVICE_KEYBOARD, 0x0002, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_KEYBOARD_QUERY_TYPEMATIC		CTL_CODE(FILE_DEVICE_KEYBOARD, 0x0008, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_KEYBOARD_QUERY_INDICATORS		CTL_CODE(FILE_DEVICE_KEYBOARD, 0x0010, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define KEYBOARD_SCROLL_LOCK_ON 			1
#define KEYBOARD_NUM_LOCK_ON				2
#define KEYBOARD_CAPS_LOCK_ON				4



//============================================================
//  TYPE DEFINITIONS
//============================================================

typedef struct _KEYBOARD_INDICATOR_PARAMETERS
{
    USHORT UnitId;             // Unit identifier.
    USHORT LedFlags;           // LED indicator state.
} KEYBOARD_INDICATOR_PARAMETERS, *PKEYBOARD_INDICATOR_PARAMETERS;


typedef struct _id_map_entry id_map_entry;
struct _id_map_entry
{
	id_map_entry *			next;
	const char *			name;
	WPARAM					id;
};



//============================================================
//  GLOBAL VARIABLES
//============================================================

static int					ledmethod;
static int					original_state;
static int					current_state;
static int					pause_state;
static HANDLE				hKbdDev;

static HWND					mame_target;
static HWND					listener_hwnd;

static id_map_entry *		idmaplist;

// message IDs
static UINT					om_mame_start;
static UINT					om_mame_stop;
static UINT					om_mame_update_state;
static UINT					om_mame_register_client;
static UINT					om_mame_unregister_client;
static UINT					om_mame_get_id_string;



//============================================================
//  FUNCTION PROTOTYPES
//============================================================

static int create_window_class(void);
static LRESULT CALLBACK listener_window_proc(HWND wnd, UINT message, WPARAM wparam, LPARAM lparam);
static LRESULT handle_mame_start(WPARAM wparam, LPARAM lparam);
static LRESULT handle_mame_stop(WPARAM wparam, LPARAM lparam);
static LRESULT handle_copydata(WPARAM wparam, LPARAM lparam);
static void reset_id_to_outname_cache(void);
static const char *map_id_to_outname(WPARAM id);
static LRESULT handle_update_state(WPARAM wparam, LPARAM lparam);

// these functions provide the meat
static void output_startup(const char *commandline);
static void output_mame_start(void);
static void output_set_state(const char *name, INT32 state);
static void output_mame_stop(void);
static void output_shutdown(void);

static int led_get_state(void);
static void led_set_state(int state);


//============================================================
//  main
//============================================================

int main(int argc, char *argv[])
{
	const char *arg = (argc > 1) ? argv[1] : "";
	int exitcode = 1;
	HWND otherwnd;
	MSG message;
	int result;

	// see if there is another instance of us running
	otherwnd = FindWindow(WINDOW_CLASS, WINDOW_NAME);

	// if the argument is "-kill", post a close message
	if (strcmp(arg, "-kill") == 0)
	{
		if (otherwnd != NULL)
			PostMessage(otherwnd, WM_QUIT, 0, 0);
		return (otherwnd != NULL) ? 1 : 0;
	}

	// if we had another instance, defer to it
	if (otherwnd != NULL)
		return 0;

	// call the startup code
	output_startup(arg);

	// create our window class
	result = create_window_class();
	if (result != 0)
		goto error;

	// create a window
	listener_hwnd = CreateWindowEx(
						WINDOW_STYLE_EX,
						WINDOW_CLASS,
						WINDOW_NAME,
						WINDOW_STYLE,
						0, 0,
						1, 1,
						NULL,
						NULL,
						GetModuleHandle(NULL),
						NULL);
	if (listener_hwnd == NULL)
		goto error;

	// allocate message ids
	om_mame_start = RegisterWindowMessage(OM_MAME_START);
	if (om_mame_start == 0)
		goto error;
	om_mame_stop = RegisterWindowMessage(OM_MAME_STOP);
	if (om_mame_stop == 0)
		goto error;
	om_mame_update_state = RegisterWindowMessage(OM_MAME_UPDATE_STATE);
	if (om_mame_update_state == 0)
		goto error;

	om_mame_register_client = RegisterWindowMessage(OM_MAME_REGISTER_CLIENT);
	if (om_mame_register_client == 0)
		goto error;
	om_mame_unregister_client = RegisterWindowMessage(OM_MAME_UNREGISTER_CLIENT);
	if (om_mame_unregister_client == 0)
		goto error;
	om_mame_get_id_string = RegisterWindowMessage(OM_MAME_GET_ID_STRING);
	if (om_mame_get_id_string == 0)
		goto error;

	// see if MAME is already running
	otherwnd = FindWindow(OUTPUT_WINDOW_CLASS, OUTPUT_WINDOW_NAME);
	if (otherwnd != NULL)
		handle_mame_start((WPARAM)otherwnd, 0);

	// process messages
	while (GetMessage(&message, NULL, 0, 0))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);
	}

	// reset on the way out if still live
	if (mame_target != NULL)
		handle_mame_stop((WPARAM)mame_target, 0);
	exitcode = 0;

error:
	// call the shutdown code
	output_shutdown();

	return exitcode;
}


//============================================================
//  create_window_class
//============================================================

static int create_window_class(void)
{
	static int classes_created = FALSE;

	/* only do this once */
	if (!classes_created)
	{
		WNDCLASS wc = { 0 };

		// initialize the description of the window class
		wc.lpszClassName	= WINDOW_CLASS;
		wc.hInstance		= GetModuleHandle(NULL);
		wc.lpfnWndProc		= listener_window_proc;

		// register the class; fail if we can't
		if (!RegisterClass(&wc))
			return 1;
		classes_created = TRUE;
	}

	return 0;
}


//============================================================
//  window_proc
//============================================================

static LRESULT CALLBACK listener_window_proc(HWND wnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	// OM_MAME_START: register ourselves with the new MAME (first instance only)
	if (message == om_mame_start)
		return handle_mame_start(wparam, lparam);

	// OM_MAME_STOP: no need to unregister, just note that we've stopped caring and reset the LEDs
	else if (message == om_mame_stop)
		return handle_mame_stop(wparam, lparam);

	// OM_MAME_UPDATE_STATE: update the state of this item if we care
	else if (message == om_mame_update_state)
		return handle_update_state(wparam, lparam);

	// WM_COPYDATA: extract the string and create an ID map entry
	else if (message == WM_COPYDATA)
		return handle_copydata(wparam, lparam);

	// everything else is default
	else
		return DefWindowProc(wnd, message, wparam, lparam);
}


//============================================================
//  handle_mame_start
//============================================================

static LRESULT handle_mame_start(WPARAM wparam, LPARAM lparam)
{
	DEBUG_PRINTF(("mame_start (%08X)\n", (UINT32)wparam));

	// make this the targeted version of MAME
	mame_target = (HWND)wparam;

	// initialize the LED states
	output_mame_start();
	reset_id_to_outname_cache();

	// register ourselves as a client
	PostMessage(mame_target, om_mame_register_client, (WPARAM)listener_hwnd, CLIENT_ID);

	// get the game name
	map_id_to_outname(0);
	return 0;
}


//============================================================
//  handle_mame_stop
//============================================================

static LRESULT handle_mame_stop(WPARAM wparam, LPARAM lparam)
{
	DEBUG_PRINTF(("mame_stop (%08X)\n", (UINT32)wparam));

	// ignore if this is not the instance we care about
	if (mame_target != (HWND)wparam)
		return 1;

	// clear our target out
	mame_target = NULL;
	reset_id_to_outname_cache();

	// reset the LED states
	output_mame_stop();
	return 0;
}


//============================================================
//  handle_copydata
//============================================================

static LRESULT handle_copydata(WPARAM wparam, LPARAM lparam)
{
	COPYDATASTRUCT *copydata = (COPYDATASTRUCT *)lparam;
	copydata_id_string *data = (copydata_id_string *)copydata->lpData;
	id_map_entry *entry;
	char *string;

	DEBUG_PRINTF(("copydata (%08X)\n", (UINT32)wparam));

	// ignore requests we don't care about
	if (mame_target != (HWND)wparam)
		return 1;

	// allocate memory
	entry = (id_map_entry *)malloc(sizeof(*entry));
	if (entry == NULL)
		return 0;

	string = (char *)malloc(strlen(data->string) + 1);
	if (string == NULL)
	{
		free(entry);
		return 0;
	}

	// if all allocations worked, make a new entry
	entry->next = idmaplist;
	entry->name = string;
	entry->id = data->id;

	// copy the string and hook us into the list
	strcpy(string, data->string);
	idmaplist = entry;

	DEBUG_PRINTF(("  id %d = '%s'\n", entry->id, entry->name));

	return 0;
}


//============================================================
//  reset_id_to_outname_cache
//============================================================

static void reset_id_to_outname_cache(void)
{
	// free our ID list
	while (idmaplist != NULL)
	{
		id_map_entry *temp = idmaplist;
		idmaplist = temp->next;
		free((void*)temp->name);
		free(temp);
	}
}


//============================================================
//  map_id_to_outname
//============================================================

static const char *map_id_to_outname(WPARAM id)
{
	id_map_entry *entry;

	// see if we have an entry in our map
	for (entry = idmaplist; entry != NULL; entry = entry->next)
		if (entry->id == id)
			return entry->name;

	// no entry yet; we have to ask
	SendMessage(mame_target, om_mame_get_id_string, (WPARAM)listener_hwnd, id);

	// now see if we have the entry in our map
	for (entry = idmaplist; entry != NULL; entry = entry->next)
		if (entry->id == id)
			return entry->name;

	// if not, use an empty string
	return "";
}


//============================================================
//  handle_update_state
//============================================================

static LRESULT handle_update_state(WPARAM wparam, LPARAM lparam)
{
	DEBUG_PRINTF(("update_state: id=%d state=%d\n", (UINT32)wparam, (UINT32)lparam));
	output_set_state(map_id_to_outname(wparam), lparam);
	return 0;
}


//
// END BOILERPLATE CODE
//
// ------------------------>8 snip 8<-------------------------
//
// BEGIN LED-SPECIFIC CODE
//


//============================================================
//  output_startup
//============================================================

static void output_startup(const char *commandline)
{
	OSVERSIONINFO osinfo = { sizeof(OSVERSIONINFO) };

	// default to PS/2, override if USB is specified as a parameter
	ledmethod = LED_METHOD_PS2;
	if (commandline != NULL && strcmp(commandline, "-usb") == 0)
		ledmethod = LED_METHOD_USB;

	// force Win9x method if we're on Win 9x
	GetVersionEx(&osinfo);
	if (osinfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
		ledmethod = LED_METHOD_WIN9X;

	// output the method
	switch (ledmethod)
	{
		case LED_METHOD_PS2:
			DEBUG_PRINTF(("Using PS/2 method\n"));
			break;

		case LED_METHOD_USB:
			DEBUG_PRINTF(("Using USB method\n"));
			break;

		case LED_METHOD_WIN9X:
			DEBUG_PRINTF(("Using Win9x method\n"));
			break;
	}
}


//============================================================
//  output_shutdown
//============================================================

static void output_shutdown(void)
{
	// nothing to do here
}


//============================================================
//  output_mame_start
//============================================================

static void output_mame_start(void)
{
	HRESULT error_number;

	// initialize the system based on the method
	switch (ledmethod)
	{
		case LED_METHOD_PS2:
			if (!DefineDosDevice(DDD_RAW_TARGET_PATH, TEXT("Kbd"), TEXT("\\Device\\KeyboardClass0")))
			{
				error_number = GetLastError();
				fprintf(stderr, "Unable to open the keyboard device. (error %d)\n", (UINT32)error_number);
				return;
			}

			hKbdDev = CreateFile(TEXT("\\\\.\\Kbd"), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
			if (hKbdDev == INVALID_HANDLE_VALUE)
			{
				error_number = GetLastError();
				fprintf(stderr, "Unable to open the keyboard device. (error %d)\n", (UINT32)error_number);
				return;
			}
			break;
	}

	// remember the initial LED states
	original_state = current_state = led_get_state();
}


//============================================================
//  output_mame_stop
//============================================================

static void output_mame_stop(void)
{
	int error_number = 0;

	// restore the initial LED states
	led_set_state(original_state);

	switch (ledmethod)
	{
		case LED_METHOD_PS2:
			if (!DefineDosDevice(DDD_REMOVE_DEFINITION, TEXT("Kbd"), NULL))
			{
				error_number = GetLastError();
				fprintf(stderr, "Unable to close the keyboard device. (error %d)\n", error_number);
				return;
			}
			if (!CloseHandle(hKbdDev))
			{
				error_number = GetLastError();
				fprintf(stderr, "Unable to close the keyboard device. (error %d)\n", error_number);
				return;
			}
			break;
	}
}


//============================================================
//  output_set_state
//============================================================

static void output_set_state(const char *outname, INT32 state)
{
	// look for pause state
	if (strcmp(outname, "pause") == 0)
	{
		if (state)
		{
			pause_state = led_get_state();
			led_set_state(original_state);
		}
		else
		{
			original_state = led_get_state();
			led_set_state(pause_state);
		}
	}
	// look for LED0/LED1/LED2 states and update accordingly
	else if (strcmp(outname, "led0") == 0)
		led_set_state((current_state & ~1) | (state & 1));
	else if (strcmp(outname, "led1") == 0)
		led_set_state((current_state & ~2) | ((state & 1) << 1));
	else if (strcmp(outname, "led2") == 0)
		led_set_state((current_state & ~4) | ((state & 1) << 2));
}


//============================================================
//  led_get_state
//============================================================

static int led_get_state(void)
{
	int result = 0;

	switch (ledmethod)
	{
		case LED_METHOD_WIN9X:
		case LED_METHOD_USB:
		{
			BYTE key_states[256];

			// get the current state
			GetKeyboardState(&key_states[0]);

			// set the numlock bit
			result |= (key_states[VK_NUMLOCK] & 1);
			result |= (key_states[VK_CAPITAL] & 1) << 1;
			result |= (key_states[VK_SCROLL] & 1) << 2;
			break;
		}

		case LED_METHOD_PS2:
		{
			KEYBOARD_INDICATOR_PARAMETERS OutputBuffer;	  // Output buffer for DeviceIoControl
			ULONG				DataLength = sizeof(KEYBOARD_INDICATOR_PARAMETERS);
			ULONG				ReturnedLength; // Number of bytes returned in output buffer

			// Address first keyboard
			OutputBuffer.UnitId = 0;

			DeviceIoControl(hKbdDev, IOCTL_KEYBOARD_QUERY_INDICATORS,
							NULL, 0,
							&OutputBuffer, DataLength,
							&ReturnedLength, NULL);

			// Demangle lights to match 95/98
			if (OutputBuffer.LedFlags & KEYBOARD_NUM_LOCK_ON) result |= 0x1;
			if (OutputBuffer.LedFlags & KEYBOARD_CAPS_LOCK_ON) result |= 0x2;
			if (OutputBuffer.LedFlags & KEYBOARD_SCROLL_LOCK_ON) result |= 0x4;
			break;
		}
	}

	return result;
}


//============================================================
//  led_set_state
//============================================================

static void led_set_state(int state)
{
	current_state = state;

	switch (ledmethod)
	{
		case LED_METHOD_WIN9X:
		{
			// thanks to Lee Taylor for the original version of this code
			BYTE key_states[256];

			// get the current state
			GetKeyboardState(&key_states[0]);

			// mask states and set new states
			key_states[VK_NUMLOCK] = (key_states[VK_NUMLOCK] & ~1) | ((state >> 0) & 1);
			key_states[VK_CAPITAL] = (key_states[VK_CAPITAL] & ~1) | ((state >> 1) & 1);
			key_states[VK_SCROLL] = (key_states[VK_SCROLL] & ~1) | ((state >> 2) & 1);

			SetKeyboardState(&key_states[0]);
			break;
		}

		case LED_METHOD_USB:
		{
			static const BYTE vk[3] = { VK_NUMLOCK, VK_CAPITAL, VK_SCROLL };
			BYTE keyState[256];
			int k;

			GetKeyboardState((LPBYTE)&keyState);
			for (k = 0; k < 3; k++)
			{
				if ((((state >> k) & 1) && !(keyState[vk[k]] & 1)) ||
					(!((state >> k) & 1) && (keyState[vk[k]] & 1)))
				{
					// Simulate a key press
					keybd_event(vk[k], 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);

					// Simulate a key release
					keybd_event(vk[k], 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
				}
			}

			keyState[VK_NUMLOCK] = (keyState[VK_NUMLOCK] & ~1) | ((state >> 0) & 1);
			keyState[VK_CAPITAL] = (keyState[VK_CAPITAL] & ~1) | ((state >> 1) & 1);
			keyState[VK_SCROLL] = (keyState[VK_SCROLL] & ~1) | ((state >> 2) & 1);
			SetKeyboardState(&keyState[0]);
			break;
		}

		case LED_METHOD_PS2:
		{
			KEYBOARD_INDICATOR_PARAMETERS InputBuffer;	  // Input buffer for DeviceIoControl
			ULONG				DataLength = sizeof(KEYBOARD_INDICATOR_PARAMETERS);
			ULONG				ReturnedLength; // Number of bytes returned in output buffer
			UINT				LedFlags=0;

			// Demangle lights to match 95/98
			if (state & 0x1) LedFlags |= KEYBOARD_NUM_LOCK_ON;
			if (state & 0x2) LedFlags |= KEYBOARD_CAPS_LOCK_ON;
			if (state & 0x4) LedFlags |= KEYBOARD_SCROLL_LOCK_ON;

			// Address first keyboard
			InputBuffer.UnitId = 0;
			InputBuffer.LedFlags = LedFlags;
			DeviceIoControl(hKbdDev, IOCTL_KEYBOARD_SET_INDICATORS,
							&InputBuffer, DataLength,
							NULL, 0,
							&ReturnedLength, NULL);
			break;
		}
	}
}
