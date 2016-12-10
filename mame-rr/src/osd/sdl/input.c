//============================================================
//
//  input.c - SDL implementation of MAME input routines
//
//  Copyright (c) 1996-2010, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//  SixAxis info: left analog is axes 0 & 1, right analog is axes 2 & 3,
//                analog L2 is axis 12 and analog L3 is axis 13
//
//============================================================

// standard sdl header
#include <SDL/SDL.h>
#include <ctype.h>
#include <stddef.h>

// MAME headers
#include "emu.h"
#include "ui.h"
#include "uiinput.h"
#include "emuopts.h"


// MAMEOS headers
#include "input.h"
#include "osdsdl.h"
#include "window.h"

// winnt.h defines this
#ifdef DELETE
#undef DELETE
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
#define MAX_AXES			32
#define MAX_BUTTONS			32
#define MAX_HATS			8
#define MAX_POV				4
#define MAX_DEVMAP_ENTRIES	16

//============================================================
//  MACROS
//============================================================

// introduced in 1.3

#ifndef SDLK_INDEX
#define SDLK_INDEX(x)		(x)
#endif


//============================================================
//  TYPEDEFS
//============================================================

// state information for a keyboard
typedef struct _keyboard_state keyboard_state;
struct _keyboard_state
{
	INT32	state[0x3ff];                               	// must be INT32!
	INT8	oldkey[MAX_KEYS];
	INT8	currkey[MAX_KEYS];
};


// state information for a mouse
typedef struct _mouse_state mouse_state;
struct _mouse_state
{
	INT32 lX, lY;
	INT32 buttons[MAX_BUTTONS];
};


// state information for a joystick; DirectInput state must be first element
typedef struct _joystick_state joystick_state;
struct _joystick_state
{
	SDL_Joystick *device;
	INT32 axes[MAX_AXES];
	INT32 buttons[MAX_BUTTONS];
	INT32 hatsU[MAX_HATS], hatsD[MAX_HATS], hatsL[MAX_HATS], hatsR[MAX_HATS];
};

// generic device information
typedef struct _device_info device_info;
struct _device_info
{
	// device information
	device_info **			head;
	device_info *			next;
	char	*			name;

	// MAME information
	input_device *			device;

	// device state
	union
	{
		keyboard_state		keyboard;
		mouse_state		mouse;
		joystick_state		joystick;
	};
};


//============================================================
//  LOCAL VARIABLES
//============================================================

// global states
static osd_lock *			input_lock;
static UINT8				input_paused;

static sdl_window_info *	focus_window = NULL;

// input buffer - only for SDLMAME_EVENTS_IN_WORKER_THREAD
#define MAX_BUF_EVENTS		(500)		/* 100 not enough for SDL 1.3 */
static SDL_Event			event_buf[MAX_BUF_EVENTS];
static int					event_buf_count;

// keyboard states
static device_info *		keyboard_list;

// mouse states
static UINT8				app_has_mouse_focus;
static UINT8				mouse_enabled;
static device_info *		mouse_list;

// lightgun states
static UINT8				lightgun_enabled;
static device_info *		lightgun_list;

// joystick states
static device_info *		joystick_list;

// joystick mapper

typedef struct _device_map_t device_map_t;
struct _device_map_t
{
	struct {
		char	*name;
		int		physical;
	} map[MAX_DEVMAP_ENTRIES];
	int 	logical[MAX_DEVMAP_ENTRIES];
	int     initialized;
};

static device_map_t joy_map;
static device_map_t mouse_map;
static device_map_t keyboard_map;

static int sixaxis_mode;


//============================================================
//  PROTOTYPES
//============================================================

static void sdlinput_pause(running_machine &machine);
static void sdlinput_resume(running_machine &machine);
static void sdlinput_exit(running_machine &machine);

// deivce list management
static void device_list_reset_devices(device_info *devlist_head);
static void device_list_free_devices(device_info **devlist_head);

// generic device management
static device_info *generic_device_alloc(device_info **devlist_head_ptr, const char *name);
static void generic_device_free(device_info *devinfo);
static int generic_device_index(device_info *devlist_head, device_info *devinfo);
static void generic_device_reset(device_info *devinfo);
static INT32 generic_button_get_state(void *device_internal, void *item_internal);
static INT32 generic_axis_get_state(void *device_internal, void *item_internal);
static device_info *generic_device_find_index(device_info *devlist_head, int index);


//============================================================
//  KEYBOARD/JOYSTICK LIST
//============================================================

// master keyboard translation table
typedef struct _kt_table kt_table;
struct _kt_table {
	INT32			mame_key;
	INT32			sdl_key;
	//const char *  vkey;
	//const char *  ascii;
	const char	*	mame_key_name;
	char		*	ui_name;
};

#if (SDL_VERSION_ATLEAST(1,3,0))

#define OSD_SDL_INDEX(x) (x)
#define OSD_SDL_INDEX_KEYSYM(keysym) ((keysym)->scancode)

#define GET_WINDOW(ev) window_from_id((ev)->windowID)
//#define GET_WINDOW(ev) ((ev)->windowID)
// FIXME: sdl does not properly report the window for certain OS.
#define GET_FOCUS_WINDOW(ev) focus_window
//#define GET_FOCUS_WINDOW(ev) window_from_id((ev)->windowID)


#define KTT_ENTRY0(MAME, SDL, VK, AS, UI) { ITEM_ID_ ## MAME, SDL_SCANCODE_ ## SDL, "ITEM_ID_" #MAME, (char *) UI }
#define KTT_ENTRY1(MAME, SDL) KTT_ENTRY0(MAME, SDL, MAME, MAME, #MAME)
// only for reference ...
#define KTT_ENTRY2(MAME, SDL) KTT_ENTRY0(MAME, SDL, 0, 0, #MAME)


static kt_table sdl_key_trans_table[] =
{
	// MAME key         SDL key         vkey    ascii
	KTT_ENTRY0(  ESC,			ESCAPE,			0x1b,	0x1b,		"ESC"  ),
	KTT_ENTRY1(  1,				1 ),
	KTT_ENTRY1(  2,				2 ),
	KTT_ENTRY1(  3,				3 ),
	KTT_ENTRY1(  4,				4 ),
	KTT_ENTRY1(  5,				5 ),
	KTT_ENTRY1(  6,				6 ),
	KTT_ENTRY1(  7,				7 ),
	KTT_ENTRY1(  8,				8 ),
	KTT_ENTRY1(  9,				9 ),
	KTT_ENTRY1(  0,				0 ),
	KTT_ENTRY0(  MINUS,			MINUS,			0xbd,	'-',	"MINUS" ),
	KTT_ENTRY0(  EQUALS,		EQUALS,			0xbb,	'=',	"EQUALS" ),
	KTT_ENTRY0(  BACKSPACE,		BACKSPACE,		0x08,	0x08,	"BACKSPACE" ),
	KTT_ENTRY0(  TAB,			TAB,			0x09,	0x09,	"TAB" ),
	KTT_ENTRY1(  Q,				Q ),
	KTT_ENTRY1(  W,				W ),
	KTT_ENTRY1(  E,				E ),
	KTT_ENTRY1(  R,				R ),
	KTT_ENTRY1(  T,				T ),
	KTT_ENTRY1(  Y,				Y ),
	KTT_ENTRY1(  U,				U ),
	KTT_ENTRY1(  I,				I ),
	KTT_ENTRY1(  O,				O ),
	KTT_ENTRY1(  P,				P ),
	KTT_ENTRY0(  OPENBRACE,	LEFTBRACKET,		0xdb,	'[',	"OPENBRACE" ),
	KTT_ENTRY0(  CLOSEBRACE,RIGHTBRACKET,		0xdd,	']',	"CLOSEBRACE" ),
	KTT_ENTRY0(  ENTER,		RETURN, 			0x0d,	0x0d,	"RETURN" ),
	KTT_ENTRY2(  LCONTROL,	LCTRL ),
	KTT_ENTRY1(  A,				A ),
	KTT_ENTRY1(  S, 			S ),
	KTT_ENTRY1(  D, 			D ),
	KTT_ENTRY1(  F, 			F ),
	KTT_ENTRY1(  G, 			G ),
	KTT_ENTRY1(  H, 			H ),
	KTT_ENTRY1(  J, 			J ),
	KTT_ENTRY1(  K, 			K ),
	KTT_ENTRY1(  L, 			L ),
	KTT_ENTRY0(  COLON, 		SEMICOLON,		0xba,	';',	"COLON" ),
	KTT_ENTRY0(  QUOTE, 		APOSTROPHE,			0xde,	'\'',	"QUOTE" ),
	KTT_ENTRY2(  LSHIFT,		LSHIFT ),
	KTT_ENTRY0(  BACKSLASH,		BACKSLASH,		0xdc,	'\\',	"BACKSLASH" ),
	KTT_ENTRY1(  Z, 			Z ),
	KTT_ENTRY1(  X, 			X ),
	KTT_ENTRY1(  C, 			C ),
	KTT_ENTRY1(  V, 			V ),
	KTT_ENTRY1(  B, 			B ),
	KTT_ENTRY1(  N, 			N ),
	KTT_ENTRY1(  M, 			M ),
	KTT_ENTRY0(  COMMA, 		COMMA,	    	0xbc,	',',	"COMMA" ),
	KTT_ENTRY0(  STOP,			PERIOD, 		0xbe,	'.',	"STOP"  ),
	KTT_ENTRY0(  SLASH, 		SLASH,	    	0xbf,	'/',	"SLASH" ),
	KTT_ENTRY2(  RSHIFT,		RSHIFT ),
	KTT_ENTRY0(  ASTERISK,		KP_MULTIPLY,    '*',	'*',	"ASTERIX" ),
	KTT_ENTRY2(  LALT,			LALT ),
	KTT_ENTRY0(  SPACE, 		SPACE,			' ',	' ',	"SPACE" ),
	KTT_ENTRY2(  CAPSLOCK,		CAPSLOCK ),
	KTT_ENTRY2(  F1,			F1 ),
	KTT_ENTRY2(  F2,			F2 ),
	KTT_ENTRY2(  F3,			F3 ),
	KTT_ENTRY2(  F4,			F4 ),
	KTT_ENTRY2(  F5,			F5 ),
	KTT_ENTRY2(  F6,			F6 ),
	KTT_ENTRY2(  F7,			F7 ),
	KTT_ENTRY2(  F8,			F8 ),
	KTT_ENTRY2(  F9,			F9 ),
	KTT_ENTRY2(  F10,			F10 ),
	KTT_ENTRY2(  NUMLOCK,		NUMLOCKCLEAR ),
	KTT_ENTRY2(  SCRLOCK,		SCROLLLOCK ),
	KTT_ENTRY2(  7_PAD, 		KP_7 ),
	KTT_ENTRY2(  8_PAD, 		KP_8 ),
	KTT_ENTRY2(  9_PAD, 		KP_9 ),
	KTT_ENTRY2(  MINUS_PAD,		KP_MINUS ),
	KTT_ENTRY2(  4_PAD, 		KP_4 ),
	KTT_ENTRY2(  5_PAD, 		KP_5 ),
	KTT_ENTRY2(  6_PAD, 		KP_6 ),
	KTT_ENTRY2(  PLUS_PAD,		KP_PLUS ),
	KTT_ENTRY2(  1_PAD, 		KP_1 ),
	KTT_ENTRY2(  2_PAD, 		KP_2 ),
	KTT_ENTRY2(  3_PAD, 		KP_3 ),
	KTT_ENTRY2(  0_PAD, 		KP_0 ),
	KTT_ENTRY2(  DEL_PAD,		KP_PERIOD ),
	KTT_ENTRY2(  F11,			F11 ),
	KTT_ENTRY2(  F12,			F12 ),
	KTT_ENTRY2(  F13,			F13 ),
	KTT_ENTRY2(  F14,			F14 ),
	KTT_ENTRY2(  F15,			F15 ),
	KTT_ENTRY2(  ENTER_PAD,		KP_ENTER  ),
	KTT_ENTRY2(  RCONTROL,		RCTRL ),
	KTT_ENTRY2(  SLASH_PAD,		KP_DIVIDE ),
	KTT_ENTRY2(  PRTSCR,		PRINTSCREEN ),
	KTT_ENTRY2(  RALT,			RALT ),
	KTT_ENTRY2(  HOME,			HOME ),
	KTT_ENTRY2(  UP,			UP ),
	KTT_ENTRY2(  PGUP,			PAGEUP ),
	KTT_ENTRY2(  LEFT,			LEFT ),
	KTT_ENTRY2(  RIGHT, 		RIGHT ),
	KTT_ENTRY2(  END,			END ),
	KTT_ENTRY2(  DOWN,			DOWN ),
	KTT_ENTRY2(  PGDN,			PAGEDOWN ),
	KTT_ENTRY2(  INSERT,		INSERT ),
	{ ITEM_ID_DEL, SDL_SCANCODE_DELETE,  "ITEM_ID_DEL", (char *)"DELETE" },
	KTT_ENTRY2(  LWIN,			LGUI ),
	KTT_ENTRY2(  RWIN,			RGUI ),
	KTT_ENTRY2(  MENU,			MENU ),
	KTT_ENTRY0(  TILDE, 		GRAVE,  	0xc0,	'`',	"TILDE" ),
	KTT_ENTRY0(  BACKSLASH2,	NONUSBACKSLASH,     0xdc,   '\\', "BACKSLASH2" ),
	{ -1 }
};
#else

#define OSD_SDL_INDEX(x) (SDLK_INDEX(x)-SDLK_FIRST)
#define OSD_SDL_INDEX_KEYSYM(keysym) (OSD_SDL_INDEX((keysym)->sym))
#define GET_WINDOW(ev) sdl_window_list
#define GET_FOCUS_WINDOW(ev) sdl_window_list

#define KTT_ENTRY0(MAME, SDL, VK, AS, UI) { ITEM_ID_ ## MAME, SDLK_ ## SDL, "ITEM_ID_" #MAME, (char *) UI }
#define KTT_ENTRY1(MAME, SDL) KTT_ENTRY0(MAME, SDL, MAME, MAME, #MAME)
// only for reference ...
#define KTT_ENTRY2(MAME, SDL) KTT_ENTRY0(MAME, SDL, 0, 0, #MAME)


static kt_table sdl_key_trans_table[] =
{
	// MAME key         SDL key         vkey    ascii
	KTT_ENTRY0(  ESC,			ESCAPE,			0x1b,	0x1b,		"ESC"  ),
	KTT_ENTRY1(  1,				1 ),
	KTT_ENTRY1(  2,				2 ),
	KTT_ENTRY1(  3,				3 ),
	KTT_ENTRY1(  4,				4 ),
	KTT_ENTRY1(  5,				5 ),
	KTT_ENTRY1(  6,				6 ),
	KTT_ENTRY1(  7,				7 ),
	KTT_ENTRY1(  8,				8 ),
	KTT_ENTRY1(  9,				9 ),
	KTT_ENTRY1(  0,				0 ),
	KTT_ENTRY0(  MINUS,			MINUS,			0xbd,	'-',	"MINUS" ),
	KTT_ENTRY0(  EQUALS,		EQUALS,			0xbb,	'=',	"EQUALS" ),
	KTT_ENTRY0(  BACKSPACE,		BACKSPACE,		0x08,	0x08,	"BACKSPACE" ),
	KTT_ENTRY0(  TAB,			TAB,			0x09,	0x09,	"TAB" ),
	KTT_ENTRY1(  Q,				q ),
	KTT_ENTRY1(  W,				w ),
	KTT_ENTRY1(  E,				e ),
	KTT_ENTRY1(  R,				r ),
	KTT_ENTRY1(  T,				t ),
	KTT_ENTRY1(  Y,				y ),
	KTT_ENTRY1(  U,				u ),
	KTT_ENTRY1(  I,				i ),
	KTT_ENTRY1(  O,				o ),
	KTT_ENTRY1(  P,				p ),
	KTT_ENTRY0(  OPENBRACE,	LEFTBRACKET,		0xdb,	'[',	"OPENBRACE" ),
	KTT_ENTRY0(  CLOSEBRACE,RIGHTBRACKET,		0xdd,	']',	"CLOSEBRACE" ),
	KTT_ENTRY0(  ENTER,		RETURN, 			0x0d,	0x0d,	"RETURN" ),
	KTT_ENTRY2(  LCONTROL,	LCTRL ),
	KTT_ENTRY1(  A,				a ),
	KTT_ENTRY1(  S, 			s ),
	KTT_ENTRY1(  D, 			d ),
	KTT_ENTRY1(  F, 			f ),
	KTT_ENTRY1(  G, 			g ),
	KTT_ENTRY1(  H, 			h ),
	KTT_ENTRY1(  J, 			j ),
	KTT_ENTRY1(  K, 			k ),
	KTT_ENTRY1(  L, 			l ),
	KTT_ENTRY0(  COLON, 		SEMICOLON,		0xba,	';',	"COLON" ),
	KTT_ENTRY0(  QUOTE, 		QUOTE,			0xde,	'\'',	"QUOTE" ),
	KTT_ENTRY2(  LSHIFT,		LSHIFT ),
	KTT_ENTRY0(  BACKSLASH,		BACKSLASH,		0xdc,	'\\',	"BACKSLASH" ),
	KTT_ENTRY1(  Z, 			z ),
	KTT_ENTRY1(  X, 			x ),
	KTT_ENTRY1(  C, 			c ),
	KTT_ENTRY1(  V, 			v ),
	KTT_ENTRY1(  B, 			b ),
	KTT_ENTRY1(  N, 			n ),
	KTT_ENTRY1(  M, 			m ),
	KTT_ENTRY0(  COMMA, 		COMMA,	    	0xbc,	',',	"COMMA" ),
	KTT_ENTRY0(  STOP,			PERIOD, 		0xbe,	'.',	"STOP"  ),
	KTT_ENTRY0(  SLASH, 		SLASH,	    	0xbf,	'/',	"SLASH" ),
	KTT_ENTRY2(  RSHIFT,		RSHIFT ),
	KTT_ENTRY0(  ASTERISK,		KP_MULTIPLY,    '*',	'*',	"ASTERIX" ),
	KTT_ENTRY2(  LALT,			LALT ),
	KTT_ENTRY0(  SPACE, 		SPACE,			' ',	' ',	"SPACE" ),
	KTT_ENTRY2(  CAPSLOCK,		CAPSLOCK ),
	KTT_ENTRY2(  F1,			F1 ),
	KTT_ENTRY2(  F2,			F2 ),
	KTT_ENTRY2(  F3,			F3 ),
	KTT_ENTRY2(  F4,			F4 ),
	KTT_ENTRY2(  F5,			F5 ),
	KTT_ENTRY2(  F6,			F6 ),
	KTT_ENTRY2(  F7,			F7 ),
	KTT_ENTRY2(  F8,			F8 ),
	KTT_ENTRY2(  F9,			F9 ),
	KTT_ENTRY2(  F10,			F10 ),
	KTT_ENTRY2(  NUMLOCK,		NUMLOCK ),
	KTT_ENTRY2(  SCRLOCK,		SCROLLOCK ),
	KTT_ENTRY2(  7_PAD, 		KP7 ),
	KTT_ENTRY2(  8_PAD, 		KP8 ),
	KTT_ENTRY2(  9_PAD, 		KP9 ),
	KTT_ENTRY2(  MINUS_PAD,		KP_MINUS ),
	KTT_ENTRY2(  4_PAD, 		KP4 ),
	KTT_ENTRY2(  5_PAD, 		KP5 ),
	KTT_ENTRY2(  6_PAD, 		KP6 ),
	KTT_ENTRY2(  PLUS_PAD,		KP_PLUS ),
	KTT_ENTRY2(  1_PAD, 		KP1 ),
	KTT_ENTRY2(  2_PAD, 		KP2 ),
	KTT_ENTRY2(  3_PAD, 		KP3 ),
	KTT_ENTRY2(  0_PAD, 		KP0 ),
	KTT_ENTRY2(  DEL_PAD,		KP_PERIOD ),
	KTT_ENTRY2(  F11,			F11 ),
	KTT_ENTRY2(  F12,			F12 ),
	KTT_ENTRY2(  F13,			F13 ),
	KTT_ENTRY2(  F14,			F14 ),
	KTT_ENTRY2(  F15,			F15 ),
	KTT_ENTRY2(  ENTER_PAD,		KP_ENTER  ),
	KTT_ENTRY2(  RCONTROL,		RCTRL ),
	KTT_ENTRY2(  SLASH_PAD,		KP_DIVIDE ),
	KTT_ENTRY2(  PRTSCR,		PRINT ),
	KTT_ENTRY2(  RALT,			RALT ),
	KTT_ENTRY2(  HOME,			HOME ),
	KTT_ENTRY2(  UP,			UP ),
	KTT_ENTRY2(  PGUP,			PAGEUP ),
	KTT_ENTRY2(  LEFT,			LEFT ),
	KTT_ENTRY2(  RIGHT, 		RIGHT ),
	KTT_ENTRY2(  END,			END ),
	KTT_ENTRY2(  DOWN,			DOWN ),
	KTT_ENTRY2(  PGDN,			PAGEDOWN ),
	KTT_ENTRY2(  INSERT,		INSERT ),
	{ ITEM_ID_DEL, SDLK_DELETE,  "ITEM_ID_DEL", (char *)"DELETE" },
	KTT_ENTRY2(  LWIN,			LSUPER ),
	KTT_ENTRY2(  RWIN,			RSUPER ),
	KTT_ENTRY2(  MENU,			MENU ),
	KTT_ENTRY0(  TILDE, 		BACKQUOTE,  	0xc0,	'`',	"TILDE" ),
	KTT_ENTRY0(  BACKSLASH2,	HASH,     0xdc,   '\\', "BACKSLASH2" ),
	{ -1 }
};
#endif

typedef struct _key_lookup_table key_lookup_table;
struct _key_lookup_table
{
	int code;
	const char *name;
};

#if (SDL_VERSION_ATLEAST(1,3,0))
#define KE(x) { SDL_SCANCODE_ ## x, "SDL_SCANCODE_" #x },
#define KE8(A, B, C, D, E, F, G, H) KE(A) KE(B) KE(C) KE(D) KE(E) KE(F) KE(G) KE(H)
#define KE7(A, B, C, D, E, F, G) KE(A) KE(B) KE(C) KE(D) KE(E) KE(F) KE(G)
#define KE5(A, B, C, D, E) KE(A) KE(B) KE(C) KE(D) KE(E)
#define KE3(A, B, C) KE(A) KE(B) KE(C)

static key_lookup_table sdl_lookup_table[] =
{
	KE7(UNKNOWN,	BACKSPACE,	TAB,		CLEAR,		RETURN,		PAUSE,		ESCAPE		)
	KE(SPACE)
	KE5(COMMA,		MINUS,		PERIOD,		SLASH,		0			)
	KE8(1,			2,			3,				4,			5,			6,			7,			8			)
	KE3(9,			SEMICOLON,		EQUALS)
	KE5(LEFTBRACKET,BACKSLASH,	RIGHTBRACKET,	A,			B			)
	KE8(C,			D,			E,				F,			G,			H,			I,			J			)
	KE8(K,			L,			M,				N,			O,			P,			Q,			R			)
	KE8(S,			T,			U,				V,			W,			X,			Y,			Z			)
	KE8(DELETE,		KP_0,		KP_1,			KP_2,		KP_3,		KP_4,		KP_5,		KP_6		)
	KE8(KP_7,		KP_8,		KP_9,			KP_PERIOD,	KP_DIVIDE,	KP_MULTIPLY,KP_MINUS,	KP_PLUS		)
	KE8(KP_ENTER,	KP_EQUALS,	UP,				DOWN,		RIGHT,		LEFT,		INSERT,		HOME		)
	KE8(END,		PAGEUP,		PAGEDOWN,		F1,			F2,			F3,			F4,			F5			)
	KE8(F6,			F7,			F8,				F9,			F10,		F11,		F12,		F13			)
	KE8(F14,		F15,		NUMLOCKCLEAR,	CAPSLOCK,	SCROLLLOCK,	RSHIFT,		LSHIFT,		RCTRL		)
	KE5(LCTRL,		RALT,		LALT,			LGUI,		RGUI)
	KE(PRINTSCREEN)
	KE(MENU)
	KE(UNDO)
	{-1, ""}
};
#else
#define KE(x) { SDLK_ ## x, "SDLK_" #x },
#define KE8(A, B, C, D, E, F, G, H) KE(A) KE(B) KE(C) KE(D) KE(E) KE(F) KE(G) KE(H)

static key_lookup_table sdl_lookup_table[] =
{
	KE8(UNKNOWN,	FIRST,		BACKSPACE,		TAB,		CLEAR,		RETURN,		PAUSE,		ESCAPE		)
	KE8(SPACE,		EXCLAIM,	QUOTEDBL,		HASH,		DOLLAR,		AMPERSAND,	QUOTE,		LEFTPAREN	)
	KE8(RIGHTPAREN,	ASTERISK,	PLUS,			COMMA,		MINUS,		PERIOD,		SLASH,		0			)
	KE8(1,			2,			3,				4,			5,			6,			7,			8			)
	KE8(9,			COLON,		SEMICOLON,		LESS,		EQUALS,		GREATER,	QUESTION,	AT			)
	KE8(LEFTBRACKET,BACKSLASH,	RIGHTBRACKET,	CARET,		UNDERSCORE,	BACKQUOTE,	a,			b			)
	KE8(c,			d,			e,				f,			g,			h,			i,			j			)
	KE8(k,			l,			m,				n,			o,			p,			q,			r			)
	KE8(s,			t,			u,				v,			w,			x,			y,			z			)
	KE8(DELETE,		WORLD_0,	WORLD_1,		WORLD_2,	WORLD_3,	WORLD_4,	WORLD_5,	WORLD_6		)
	KE8(WORLD_7,	WORLD_8,	WORLD_9,		WORLD_10,	WORLD_11,	WORLD_12,	WORLD_13,	WORLD_14	)
	KE8(WORLD_15,	WORLD_16,	WORLD_17,		WORLD_18,	WORLD_19,	WORLD_20,	WORLD_21,	WORLD_22	)
	KE8(WORLD_23,	WORLD_24,	WORLD_25,		WORLD_26,	WORLD_27,	WORLD_28,	WORLD_29,	WORLD_30	)
	KE8(WORLD_31,	WORLD_32,	WORLD_33,		WORLD_34,	WORLD_35,	WORLD_36,	WORLD_37,	WORLD_38	)
	KE8(WORLD_39,	WORLD_40,	WORLD_41,		WORLD_42,	WORLD_43,	WORLD_44,	WORLD_45,	WORLD_46	)
	KE8(WORLD_47,	WORLD_48,	WORLD_49,		WORLD_50,	WORLD_51,	WORLD_52,	WORLD_53,	WORLD_54	)
	KE8(WORLD_55,	WORLD_56,	WORLD_57,		WORLD_58,	WORLD_59,	WORLD_60,	WORLD_61,	WORLD_62	)
	KE8(WORLD_63,	WORLD_64,	WORLD_65,		WORLD_66,	WORLD_67,	WORLD_68,	WORLD_69,	WORLD_70	)
	KE8(WORLD_71,	WORLD_72,	WORLD_73,		WORLD_74,	WORLD_75,	WORLD_76,	WORLD_77,	WORLD_78	)
	KE8(WORLD_79,	WORLD_80,	WORLD_81,		WORLD_82,	WORLD_83,	WORLD_84,	WORLD_85,	WORLD_86	)
	KE8(WORLD_87,	WORLD_88,	WORLD_89,		WORLD_90,	WORLD_91,	WORLD_92,	WORLD_93,	WORLD_94	)
	KE8(WORLD_95,	KP0,		KP1,			KP2,		KP3,		KP4,		KP5,		KP6			)
	KE8(KP7,		KP8,		KP9,			KP_PERIOD,	KP_DIVIDE,	KP_MULTIPLY,KP_MINUS,	KP_PLUS		)
	KE8(KP_ENTER,	KP_EQUALS,	UP,				DOWN,		RIGHT,		LEFT,		INSERT,		HOME		)
	KE8(END,		PAGEUP,		PAGEDOWN,		F1,			F2,			F3,			F4,			F5			)
	KE8(F6,			F7,			F8,				F9,			F10,		F11,		F12,		F13			)
	KE8(F14,		F15,		NUMLOCK,		CAPSLOCK,	SCROLLOCK,	RSHIFT,		LSHIFT,		RCTRL		)
	KE8(LCTRL,		RALT,		LALT,			RMETA,		LMETA,		LSUPER,		RSUPER,		MODE		)
	KE8(COMPOSE,	HELP,		PRINT,			SYSREQ,		BREAK,		MENU,		POWER,		EURO		)
	KE(UNDO)
	KE(LAST)
	{-1, ""}
};
#endif


//============================================================
//  INLINE FUNCTIONS
//============================================================

static int devmap_leastfree(device_map_t *devmap)
{
	int i;
	for (i=0;i<MAX_DEVMAP_ENTRIES;i++)
	{
		if (*devmap->map[i].name == 0)
			return i;
	}
	return -1;
}

static char *remove_spaces(running_machine *machine, const char *s)
{
	char *r, *p;
	static const char *def_name[] = { "Unknown" };

	while (*s && *s == ' ')
		s++;

	if (strlen(s) == 0)
	{
		r = auto_alloc_array(machine, char, strlen((char *)def_name) + 1);
		strcpy(r, (char *)def_name);
		return r;
	}

	r = auto_alloc_array(machine, char, strlen(s) + 1);
	p = r;
	while (*s)
	{
		if (*s != ' ')
			*p++ = *s++;
		else
		{
			while (*s && *s == ' ')
				s++;
			if (*s)
				*p++ = ' ';
		}
	}
	*p = 0;
	return r;
}

static void devmap_register(device_map_t *devmap, int physical_idx, char *name)
{
	int found = 0;
	int stick, i;

	for (i=0;i<MAX_DEVMAP_ENTRIES;i++)
	{
		if (strcmp(name,devmap->map[i].name) == 0 && devmap->map[i].physical < 0)
		{
			devmap->map[i].physical = physical_idx;
			found = 1;
			devmap->logical[physical_idx] = i;
		}
	}
	if (found == 0)
	{
		stick = devmap_leastfree(devmap);
		devmap->map[stick].physical = physical_idx;
		devmap->map[stick].name = name;
		devmap->logical[physical_idx] = stick;
	}

}

//============================================================
//  init_joymap
//============================================================

static void devmap_init(running_machine *machine, device_map_t *devmap, const char *opt, int max_devices, const char *label)
{
	int dev;
	char defname[20];

	assert(max_devices <= MAX_DEVMAP_ENTRIES);

	for (dev = 0; dev < MAX_DEVMAP_ENTRIES; dev++)
	{
		devmap->map[dev].name = (char *)"";
		devmap->map[dev].physical = -1;
		devmap->logical[dev] = -1;
	}
	devmap->initialized = 0;

	for (dev = 0; dev < max_devices; dev++)
	{
		const char *dev_name;
		sprintf(defname, "%s%d", opt, dev + 1);

		dev_name = options_get_string(machine->options(), defname);
		if (dev_name && *dev_name && strcmp(dev_name,SDLOPTVAL_AUTO))
		{
			devmap->map[dev].name = remove_spaces(machine, dev_name);
			mame_printf_verbose("%s: Logical id %d: %s\n", label, dev + 1, joy_map.map[dev].name);
			devmap->initialized = 1;
		}
	}
}

static device_info *devmap_class_register(running_machine *machine, device_map_t *devmap,
		int index, device_info **devlist, input_device_class devclass)
{
	device_info *devinfo = NULL;
	char tempname[20];

	if (*devmap->map[index].name == 0)
	{
		/* only map place holders if there were mappings specified is enabled */
		if (devmap->initialized)
		{
			sprintf(tempname, "NC%d", index);
			devinfo = generic_device_alloc(devlist, tempname);
			devinfo->device = input_device_add(machine, devclass, devinfo->name, devinfo);
		}
		return NULL;
	}
	else
	{
		devinfo = generic_device_alloc(devlist, devmap->map[index].name);
		devinfo->device = input_device_add(machine, devclass, devinfo->name, devinfo);
	}
	return devinfo;
}


//============================================================
//  sdlinput_register_joysticks
//============================================================

static void sdlinput_register_joysticks(running_machine *machine)
{
	device_info *devinfo;
	int physical_stick, axis, button, hat, stick;
	char tempname[512];
	SDL_Joystick *joy;

	devmap_init(machine, &joy_map, SDLOPTION_JOYINDEX, 8, "Joystick mapping");

	mame_printf_verbose("Joystick: Start initialization\n");
	for (physical_stick = 0; physical_stick < SDL_NumJoysticks(); physical_stick++)
	{
		char *joy_name = remove_spaces(machine, SDL_JoystickName(physical_stick));

		devmap_register(&joy_map, physical_stick, joy_name);
	}

	for (stick = 0; stick < MAX_DEVMAP_ENTRIES; stick++)
	{
		devinfo = devmap_class_register(machine, &joy_map, stick, &joystick_list, DEVICE_CLASS_JOYSTICK);

		if (devinfo == NULL)
			continue;

		physical_stick = joy_map.map[stick].physical;

		joy = SDL_JoystickOpen(physical_stick);

		devinfo->joystick.device = joy;

		mame_printf_verbose("Joystick: %s\n", devinfo->name);
		mame_printf_verbose("Joystick:   ...  %d axes, %d buttons %d hats\n", SDL_JoystickNumAxes(joy), SDL_JoystickNumButtons(joy), SDL_JoystickNumHats(joy));
		mame_printf_verbose("Joystick:   ...  Physical id %d mapped to logical id %d\n", physical_stick, stick);

		// loop over all axes
		for (axis = 0; axis < SDL_JoystickNumAxes(joy); axis++)
		{
			input_item_id itemid;

			if (axis < INPUT_MAX_AXIS)
				itemid = (input_item_id) (ITEM_ID_XAXIS + axis);
			else if (axis < INPUT_MAX_AXIS + INPUT_MAX_ADD_ABSOLUTE)
				itemid = (input_item_id) (ITEM_ID_ADD_ABSOLUTE1 - INPUT_MAX_AXIS + axis);
			else
				itemid = ITEM_ID_OTHER_AXIS_ABSOLUTE;

			sprintf(tempname, "A%d %s", axis, devinfo->name);
			input_device_item_add(devinfo->device, tempname, &devinfo->joystick.axes[axis], itemid, generic_axis_get_state);
		}

		// loop over all buttons
		for (button = 0; button < SDL_JoystickNumButtons(joy); button++)
		{
			input_item_id itemid;

			devinfo->joystick.buttons[button] = 0;

			if (button < INPUT_MAX_BUTTONS)
				itemid = (input_item_id) (ITEM_ID_BUTTON1 + button);
			else if (button < INPUT_MAX_BUTTONS + INPUT_MAX_ADD_SWITCH)
				itemid = (input_item_id) (ITEM_ID_ADD_SWITCH1 - INPUT_MAX_BUTTONS + button);
			else
				itemid = ITEM_ID_OTHER_SWITCH;

			sprintf(tempname, "button %d", button);
			input_device_item_add(devinfo->device, tempname, &devinfo->joystick.buttons[button], itemid, generic_button_get_state);
		}

		// loop over all hats
		for (hat = 0; hat < SDL_JoystickNumHats(joy); hat++)
		{
			input_item_id itemid;

			sprintf(tempname, "hat %d Up", hat);
			itemid = (input_item_id) ((hat < INPUT_MAX_HATS) ? ITEM_ID_HAT1UP + 4 * hat : ITEM_ID_OTHER_SWITCH);
			input_device_item_add(devinfo->device, tempname, &devinfo->joystick.hatsU[hat], itemid, generic_button_get_state);
			sprintf(tempname, "hat %d Down", hat);
			itemid = (input_item_id) ((hat < INPUT_MAX_HATS) ? ITEM_ID_HAT1DOWN + 4 * hat : ITEM_ID_OTHER_SWITCH);
			input_device_item_add(devinfo->device, tempname, &devinfo->joystick.hatsD[hat], itemid, generic_button_get_state);
			sprintf(tempname, "hat %d Left", hat);
			itemid = (input_item_id) ((hat < INPUT_MAX_HATS) ? ITEM_ID_HAT1LEFT + 4 * hat : ITEM_ID_OTHER_SWITCH);
			input_device_item_add(devinfo->device, tempname, &devinfo->joystick.hatsL[hat], itemid, generic_button_get_state);
			sprintf(tempname, "hat %d Right", hat);
			itemid = (input_item_id) ((hat < INPUT_MAX_HATS) ? ITEM_ID_HAT1RIGHT + 4 * hat : ITEM_ID_OTHER_SWITCH);
	    	input_device_item_add(devinfo->device, tempname, &devinfo->joystick.hatsR[hat], itemid, generic_button_get_state);
		}
	}
	mame_printf_verbose("Joystick: End initialization\n");
}

//============================================================
//  sdlinput_deregister_joysticks
//============================================================

static void sdlinput_deregister_joysticks(running_machine *machine)
{
	device_info *curdev;

	mame_printf_verbose("Joystick: Start deinitialization\n");

	for (curdev = joystick_list; curdev != NULL; curdev = curdev->next)
	{
		SDL_JoystickClose(curdev->joystick.device);
	}

	mame_printf_verbose("Joystick: End deinitialization\n");
}

//============================================================
//  sdlinput_register_mice
//============================================================

#if (SDL_VERSION_ATLEAST(1,3,0))
static void sdlinput_register_mice(running_machine *machine)
{
	int index, physical_mouse;

	mouse_enabled = options_get_bool(machine->options(), OPTION_MOUSE);

	devmap_init(machine, &mouse_map, SDLOPTION_MOUSEINDEX, 8, "Mouse mapping");

	for (physical_mouse = 0; physical_mouse < SDL_GetNumMice(); physical_mouse++)
	{
		char *mouse_name = remove_spaces(machine, SDL_GetMouseName(physical_mouse));

		devmap_register(&mouse_map, physical_mouse, mouse_name);
	}

	mame_printf_verbose("Mouse: Start initialization\n");
	for (index = 0; index < MAX_DEVMAP_ENTRIES; index++)
	{
		device_info *devinfo;
		char defname[90];
		int button;

		devinfo = devmap_class_register(machine, &mouse_map, index, &mouse_list, DEVICE_CLASS_MOUSE);

		if (devinfo == NULL)
			continue;

		// add the axes
		sprintf(defname, "X %s", devinfo->name);
		input_device_item_add(devinfo->device, defname, &devinfo->mouse.lX, ITEM_ID_XAXIS, generic_axis_get_state);
		sprintf(defname, "Y %s", devinfo->name);
		input_device_item_add(devinfo->device, defname, &devinfo->mouse.lY, ITEM_ID_YAXIS, generic_axis_get_state);

		for (button = 0; button < 4; button++)
		{
			input_item_id itemid;

			sprintf(defname, "B%d", button + 1);
			itemid = (input_item_id) (ITEM_ID_BUTTON1+button);

			input_device_item_add(devinfo->device, defname, &devinfo->mouse.buttons[button], itemid, generic_button_get_state);
		}

		if (0 && mouse_enabled)
			SDL_SetRelativeMouseMode(index, SDL_TRUE);
		mame_printf_verbose("Mouse: Registered %s\n", devinfo->name);
	}
	mame_printf_verbose("Mouse: End initialization\n");
}
#else
static void sdlinput_register_mice(running_machine *machine)
{
	device_info *devinfo;
	char defname[20];
	int button;

	mame_printf_verbose("Mouse: Start initialization\n");

	mouse_map.logical[0] = 0;

	// SDL 1.2 has only 1 mouse - 1.3+ will also change that, so revisit this then
	devinfo = generic_device_alloc(&mouse_list, "System mouse");
	devinfo->device = input_device_add(machine, DEVICE_CLASS_MOUSE, devinfo->name, devinfo);

	mouse_enabled = options_get_bool(machine->options(), OPTION_MOUSE);

	// add the axes
	input_device_item_add(devinfo->device, "X", &devinfo->mouse.lX, ITEM_ID_XAXIS, generic_axis_get_state);
	input_device_item_add(devinfo->device, "Y", &devinfo->mouse.lY, ITEM_ID_YAXIS, generic_axis_get_state);

	for (button = 0; button < 4; button++)
	{
		input_item_id itemid = (input_item_id) (ITEM_ID_BUTTON1+button);
		sprintf(defname, "B%d", button + 1);

		input_device_item_add(devinfo->device, defname, &devinfo->mouse.buttons[button], itemid, generic_button_get_state);
	}

	mame_printf_verbose("Mouse: Registered %s\n", devinfo->name);
	mame_printf_verbose("Mouse: End initialization\n");
}
#endif


//============================================================
//  lookup_sdl_code
//============================================================

static int lookup_sdl_code(const char *scode)
{
	int i=0;

	while (sdl_lookup_table[i].code>=0)
	{
		if (!strcmp(scode, sdl_lookup_table[i].name))
			return sdl_lookup_table[i].code;
		i++;
	}
	return -1;
}


//============================================================
//  lookup_mame_code
//============================================================

static int lookup_mame_index(const char *scode)
{
	int index, i;

	index=-1;
	i=0;
	while (sdl_key_trans_table[i].mame_key >= 0)
	{
		if (!strcmp(scode, sdl_key_trans_table[i].mame_key_name))
		{
			index=i;
			break;
		}
		i++;
	}
	return index;
}

static int lookup_mame_code(const char *scode)
{
	int index;
	index = lookup_mame_index(scode);
	if (index >= 0)
		return sdl_key_trans_table[index].mame_key;
	else
		return -1;
}


//============================================================
//  sdlinput_read_keymap
//============================================================

static kt_table * sdlinput_read_keymap(running_machine *machine)
{
	char *keymap_filename;
	kt_table *key_trans_table;
	FILE *keymap_file;
	int line = 1;
	int index,i, sk, vk, ak;
	char buf[256];
	char mks[21];
	char sks[21];
	char kns[21];

	if (!options_get_bool(machine->options(), SDLOPTION_KEYMAP))
		return sdl_key_trans_table;

	keymap_filename = (char *)options_get_string(machine->options(), SDLOPTION_KEYMAP_FILE);
	mame_printf_verbose("Keymap: Start reading keymap_file %s\n", keymap_filename);

	keymap_file = fopen(keymap_filename, "r");
	if (keymap_file == NULL)
	{
		mame_printf_warning( "Keymap: Unable to open keymap %s, using default\n", keymap_filename);
		return sdl_key_trans_table;
	}

	key_trans_table = auto_alloc_array(machine, kt_table, ARRAY_LENGTH(sdl_key_trans_table));
	memcpy((void *) key_trans_table, sdl_key_trans_table, sizeof(sdl_key_trans_table));

	while (!feof(keymap_file))
	{
		fgets(buf, 255, keymap_file);
		if (*buf && buf[0] && buf[0] != '#')
		{
			buf[255]=0;
			i=strlen(buf);
			if (i && buf[i-1] == '\n')
				buf[i-1] = 0;
			mks[0]=0;
			sks[0]=0;
			memset(kns, 0, ARRAY_LENGTH(kns));
			sscanf(buf, "%20s %20s %x %x %20c\n",
					mks, sks, &vk, &ak, kns);

			index=lookup_mame_index(mks);
			sk = lookup_sdl_code(sks);

			if ( sk >= 0 && index >=0)
			{
				key_trans_table[index].sdl_key = sk;
				// vk and ak are not really needed
				//key_trans_table[index][VIRTUAL_KEY] = vk;
				//key_trans_table[index][ASCII_KEY] = ak;
				key_trans_table[index].ui_name = auto_alloc_array(machine, char, strlen(kns)+1);
				strcpy(key_trans_table[index].ui_name, kns);
				mame_printf_verbose("Keymap: Mapped <%s> to <%s> with ui-text <%s>\n", sks, mks, kns);
			}
			else
				mame_printf_warning("Keymap: Error on line %d - %s key not found: %s\n", line, (sk<0) ? "sdl" : "mame", buf);
		}
		line++;
	}
	fclose(keymap_file);
	mame_printf_verbose("Keymap: Processed %d lines\n", line);

	return key_trans_table;
}


//============================================================
//  sdlinput_register_keyboards
//============================================================

#if (SDL_VERSION_ATLEAST(1,3,0))
static void sdlinput_register_keyboards(running_machine *machine)
{
	int physical_keyboard;
	int index;
	kt_table *key_trans_table;

	key_trans_table = sdlinput_read_keymap(machine);

	devmap_init(machine, &keyboard_map, SDLOPTION_KEYBINDEX, 8, "Keyboard mapping");

	for (physical_keyboard = 0; physical_keyboard < SDL_GetNumKeyboards(); physical_keyboard++)
	{
		char defname[90];
		snprintf(defname, sizeof(defname)-1, "Keyboard #%d", physical_keyboard + 1);

		devmap_register(&keyboard_map, physical_keyboard, defname);
	}

	mame_printf_verbose("Keyboard: Start initialization\n");
	for (index = 0; index < MAX_DEVMAP_ENTRIES; index++)
	{
		device_info *devinfo;
		char defname[90];
		int keynum;

		devinfo = devmap_class_register(machine, &keyboard_map, index, &keyboard_list, DEVICE_CLASS_KEYBOARD);

		if (devinfo == NULL)
			continue;

		// populate it
		for (keynum = 0; sdl_key_trans_table[keynum].mame_key >= 0; keynum++)
		{
			input_item_id itemid;

			itemid = (input_item_id) key_trans_table[keynum].mame_key;

			// generate the default / modified name
			snprintf(defname, sizeof(defname)-1, "%s", key_trans_table[keynum].ui_name);

			// add the item to the device
			input_device_item_add(devinfo->device, defname, &devinfo->keyboard.state[OSD_SDL_INDEX(key_trans_table[keynum].sdl_key)], itemid, generic_button_get_state);
		}

		mame_printf_verbose("Keyboard: Registered %s\n", devinfo->name);
	}
	mame_printf_verbose("Keyboard: End initialization\n");
}
#else
static void sdlinput_register_keyboards(running_machine *machine)
{
	device_info *devinfo;
	char defname[20];
	int keynum;
	kt_table *key_trans_table;

	key_trans_table = sdlinput_read_keymap(machine);

	keyboard_map.logical[0] = 0;

	mame_printf_verbose("Keyboard: Start initialization\n");

	// SDL 1.2 only has 1 keyboard (1.3+ will have multiple, this must be revisited then)
	// add it now
	devinfo = generic_device_alloc(&keyboard_list, "System keyboard");
	devinfo->device = input_device_add(machine, DEVICE_CLASS_KEYBOARD, devinfo->name, devinfo);

	// populate it
	for (keynum = 0; sdl_key_trans_table[keynum].mame_key >= 0; keynum++)
	{
		input_item_id itemid;

		itemid = (input_item_id) key_trans_table[keynum].mame_key;

		// generate the default / modified name
		snprintf(defname, sizeof(defname)-1, "%s", key_trans_table[keynum].ui_name);

		// add the item to the device
		//printf("Test %d\n", OSD_SDL_INDEX(key_trans_table[keynum].sdl_key) );
		input_device_item_add(devinfo->device, defname, &devinfo->keyboard.state[OSD_SDL_INDEX(key_trans_table[keynum].sdl_key)], itemid, generic_button_get_state);
	}

	mame_printf_verbose("Keyboard: Registered %s\n", devinfo->name);
	mame_printf_verbose("Keyboard: End initialization\n");
}
#endif

//============================================================
//  sdlinput_init
//============================================================

void sdlinput_init(running_machine *machine)
{
	keyboard_list = NULL;
	joystick_list = NULL;
	mouse_list = NULL;
	lightgun_list = NULL;

	app_has_mouse_focus = 1;

	// we need pause and exit callbacks
	machine->add_notifier(MACHINE_NOTIFY_PAUSE, sdlinput_pause);
	machine->add_notifier(MACHINE_NOTIFY_RESUME, sdlinput_resume);
	machine->add_notifier(MACHINE_NOTIFY_EXIT, sdlinput_exit);

	// allocate a lock for input synchronizations
	input_lock = osd_lock_alloc();
	assert_always(input_lock != NULL, "Failed to allocate input_lock");

	// register the keyboards
	sdlinput_register_keyboards(machine);

	// register the mice
	sdlinput_register_mice(machine);

	if (machine->debug_flags & DEBUG_FLAG_OSD_ENABLED)
	{
		mame_printf_warning("Debug Build: Disabling input grab for -debug\n");
		mouse_enabled = 0;
	}

	// get Sixaxis special mode info
	sixaxis_mode = options_get_bool(machine->options(), SDLOPTION_SIXAXIS);

	// register the joysticks
	sdlinput_register_joysticks(machine);

	// now reset all devices
	device_list_reset_devices(keyboard_list);
	device_list_reset_devices(mouse_list);
	device_list_reset_devices(joystick_list);

}


//============================================================
//  sdlinput_pause
//============================================================

static void sdlinput_pause(running_machine &machine)
{
	// keep track of the paused state
	input_paused = true;
}

static void sdlinput_resume(running_machine &machine)
{
	// keep track of the paused state
	input_paused = false;
}


//============================================================
//  sdlinput_exit
//============================================================

static void sdlinput_exit(running_machine &machine)
{
	// free the lock
	osd_lock_free(input_lock);

	// deregister

	sdlinput_deregister_joysticks(&machine);

	// free all devices
	device_list_free_devices(&keyboard_list);
	device_list_free_devices(&mouse_list);
	device_list_free_devices(&joystick_list);
}


//============================================================
//  sdlinput_get_focus_window
//============================================================

sdl_window_info *sdlinput_get_focus_window(running_machine *machine)
{
	if (focus_window)  // only be set on SDL >= 1.3
		return focus_window;
	else
		return sdl_window_list;
}


//============================================================
//  sdlinput_poll
//============================================================

#if (SDL_VERSION_ATLEAST(1,3,0))
INLINE sdl_window_info * window_from_id(Uint32 windowID)
{
	sdl_window_info *w;
	SDL_Window *window = SDL_GetWindowFromID(windowID);

	for (w = sdl_window_list; w != NULL; w = w->next)
	{
		//printf("w->window_id: %d\n", w->window_id);
		if (w->sdl_window == window)
		{
			return w;
		}
	}
	return NULL;
}

INLINE void resize_all_windows(void)
{
	sdl_window_info *w;
	osd_ticks_t now = osd_ticks();

	if (SDL13_COMBINE_RESIZE)
	{
		for (w = sdl_window_list; w != NULL; w = w->next)
		{
			if (w->resize_width && w->resize_height && ((now - w->last_resize) > osd_ticks_per_second() / 10))
			{
				sdlwindow_resize(w, w->resize_width, w->resize_height);
				w->resize_width = 0;
				w->resize_height = 0;
			}
		}
	}
}

#endif

void sdlinput_process_events_buf(running_machine *machine)
{
	SDL_Event event;

	if (SDLMAME_EVENTS_IN_WORKER_THREAD)
	{
		osd_lock_acquire(input_lock);
	#if (SDL_VERSION_ATLEAST(1,3,0))
		/* Make sure we get all pending events */
		SDL_PumpEvents();
	#endif
		while(SDL_PollEvent(&event))
		{
			if (event_buf_count < MAX_BUF_EVENTS)
				event_buf[event_buf_count++] = event;
			else
				mame_printf_warning("Event Buffer Overflow!\n");
		}
		osd_lock_release(input_lock);
	}
}


void sdlinput_poll(running_machine *machine)
{
	device_info *devinfo;
	SDL_Event event;
	int index;

	// only for SDLMAME_EVENTS_IN_WORKER_THREAD
	SDL_Event			loc_event_buf[MAX_BUF_EVENTS];
	int					loc_event_buf_count;
	int bufp;

	for (index=0; ;index++)
	{
		devinfo = generic_device_find_index( mouse_list, index);
		if (devinfo == NULL)
			break;
		devinfo->mouse.lX = 0;
		devinfo->mouse.lY = 0;
	}

	if (SDLMAME_EVENTS_IN_WORKER_THREAD)
	{
		osd_lock_acquire(input_lock);
		memcpy(loc_event_buf, event_buf, sizeof(event_buf));
		loc_event_buf_count = event_buf_count;
		event_buf_count = 0;
		osd_lock_release(input_lock);
		bufp = 0;
	}

	while (TRUE)
	{
		if (SDLMAME_EVENTS_IN_WORKER_THREAD)
		{
			if (bufp >= loc_event_buf_count)
				break;
			event = loc_event_buf[bufp++];
		}
		else
		{
			if (!SDL_PollEvent(&event))
				break;
		}

		if (event.type == SDL_KEYUP &&
		    event.key.keysym.sym == SDLK_CAPSLOCK)
		{
			/* more caps-lock hack */
			event.type = SDL_KEYDOWN;
		}
		switch(event.type) {
		case SDL_KEYDOWN:
			devinfo = generic_device_find_index( keyboard_list, keyboard_map.logical[event.key.which]);
			//printf("Key down %d %d %s\n", event.key.which, event.key.keysym.scancode, devinfo->name);
			devinfo->keyboard.state[OSD_SDL_INDEX_KEYSYM(&event.key.keysym)] = 0x80;
#if (!SDL_VERSION_ATLEAST(1,3,0))
			ui_input_push_char_event(machine, sdl_window_list->target, (unicode_char) event.key.keysym.unicode);
#endif
			break;
		case SDL_KEYUP:
			devinfo = generic_device_find_index( keyboard_list, keyboard_map.logical[event.key.which]);
			devinfo->keyboard.state[OSD_SDL_INDEX_KEYSYM(&event.key.keysym)] = 0x00;
			break;
		case SDL_JOYAXISMOTION:
			devinfo = generic_device_find_index(joystick_list, joy_map.logical[event.jaxis.which]);
			if (devinfo)
			{
				if (sixaxis_mode)
				{
					int axis = event.jaxis.axis;

					if (axis <= 3)
					{
						devinfo->joystick.axes[event.jaxis.axis] = (event.jaxis.value * 2);
					}
					else
					{
						int magic = (event.jaxis.value / 2) + 16384;

						devinfo->joystick.axes[event.jaxis.axis] = magic;
					}
				}
				else
				{
					devinfo->joystick.axes[event.jaxis.axis] = (event.jaxis.value * 2);
				}
			}
			break;
		case SDL_JOYHATMOTION:
			devinfo = generic_device_find_index(joystick_list, joy_map.logical[event.jhat.which]);
			if (devinfo)
			{
				if (event.jhat.value & SDL_HAT_UP)
				{
					devinfo->joystick.hatsU[event.jhat.hat] = 0x80;
				}
				else
				{
					devinfo->joystick.hatsU[event.jhat.hat] = 0;
				}
				if (event.jhat.value & SDL_HAT_DOWN)
				{
					devinfo->joystick.hatsD[event.jhat.hat] = 0x80;
				}
				else
				{
					devinfo->joystick.hatsD[event.jhat.hat] = 0;
				}
				if (event.jhat.value & SDL_HAT_LEFT)
				{
					devinfo->joystick.hatsL[event.jhat.hat] = 0x80;
				}
				else
				{
					devinfo->joystick.hatsL[event.jhat.hat] = 0;
				}
				if (event.jhat.value & SDL_HAT_RIGHT)
				{
					devinfo->joystick.hatsR[event.jhat.hat] = 0x80;
				}
				else
				{
					devinfo->joystick.hatsR[event.jhat.hat] = 0;
				}
			}
			break;
		case SDL_JOYBUTTONDOWN:
		case SDL_JOYBUTTONUP:
			devinfo = generic_device_find_index(joystick_list, joy_map.logical[event.jbutton.which]);
			if (devinfo)
			{
				devinfo->joystick.buttons[event.jbutton.button] = (event.jbutton.state == SDL_PRESSED) ? 0x80 : 0;
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
			devinfo = generic_device_find_index(mouse_list, mouse_map.logical[event.button.which]);
			devinfo->mouse.buttons[event.button.button-1] = 0x80;
			//printf("But down %d %d %d %d %s\n", event.button.which, event.button.button, event.button.x, event.button.y, devinfo->name);
			if (event.button.button == 1)
			{
				// FIXME Move static declaration
				static osd_ticks_t last_click = 0;
				static int last_x = 0;
				static int last_y = 0;
				int cx, cy;
				osd_ticks_t click = osd_ticks() * 1000 / osd_ticks_per_second();
				sdl_window_info *window = GET_FOCUS_WINDOW(&event.button);
				if (window != NULL && window->xy_to_render_target(window, event.button.x,event.button.y, &cx, &cy) )
				{
					ui_input_push_mouse_down_event(machine, window->target, cx, cy);
					// FIXME Parameter ?
					if ((click-last_click < 250)
							&& (cx >= last_x - 4 && cx <= last_x  + 4)
							&& (cy >= last_y - 4 && cy <= last_y  + 4) )
					{
						last_click = 0;
						ui_input_push_mouse_double_click_event(machine, window->target, cx, cy);
					}
					else
					{
						last_click = click;
						last_x = cx;
						last_y = cy;
					}
				}
			}
			break;
		case SDL_MOUSEBUTTONUP:
			devinfo = generic_device_find_index(mouse_list, mouse_map.logical[event.button.which]);
			devinfo->mouse.buttons[event.button.button-1] = 0;
			//printf("But up %d %d %d %d\n", event.button.which, event.button.button, event.button.x, event.button.y);

			if (event.button.button == 1)
			{
				int cx, cy;
				sdl_window_info *window = GET_FOCUS_WINDOW(&event.button);

				if (window != NULL && window->xy_to_render_target(window, event.button.x,event.button.y, &cx, &cy) )
				{
					ui_input_push_mouse_up_event(machine, window->target, cx, cy);
				}
			}
			break;
		case SDL_MOUSEMOTION:
			devinfo = generic_device_find_index(mouse_list, mouse_map.logical[event.motion.which]);
#if (SDL_VERSION_ATLEAST(1,3,0))
			// FIXME: may apply to 1.2 as well ...
			//printf("Motion %d %d %d %s\n", event.motion.which, event.motion.x, event.motion.y, devinfo->name);
			devinfo->mouse.lX += event.motion.xrel * INPUT_RELATIVE_PER_PIXEL;
			devinfo->mouse.lY += event.motion.yrel * INPUT_RELATIVE_PER_PIXEL;
#else
			devinfo->mouse.lX = event.motion.xrel * INPUT_RELATIVE_PER_PIXEL;
			devinfo->mouse.lY = event.motion.yrel * INPUT_RELATIVE_PER_PIXEL;
#endif
			{
				int cx=-1, cy=-1;
				sdl_window_info *window = GET_FOCUS_WINDOW(&event.motion);

				if (window != NULL && window->xy_to_render_target(window, event.motion.x, event.motion.y, &cx, &cy) )
					ui_input_push_mouse_move_event(machine, window->target, cx, cy);
			}
			break;
#if (!SDL_VERSION_ATLEAST(1,3,0))
		case SDL_APPMOUSEFOCUS:
			app_has_mouse_focus = event.active.gain;
			if (!event.active.gain)
			{
				sdl_window_info *window = GET_FOCUS_WINDOW(&event.motion);
				ui_input_push_mouse_leave_event(machine, window->target);
			}
			break;
		case SDL_QUIT:
			machine->schedule_exit();
			break;
		case SDL_VIDEORESIZE:
			sdlwindow_resize(sdl_window_list, event.resize.w, event.resize.h);
			break;
#else
		case SDL_TEXTINPUT:
			if (*event.text.text)
			{
				sdl_window_info *window = GET_FOCUS_WINDOW(&event.text);
				unicode_char result;
				if (window != NULL )
				{
					osd_uchar_from_osdchar(&result, event.text.text, 1);
					ui_input_push_char_event(machine, window->target, result);
				}
			}
			break;
		case SDL_WINDOWEVENT:
		{
			sdl_window_info *window = GET_WINDOW(&event.window);

			if (window == NULL)
				break;

			switch (event.window.event)
			{
			case SDL_WINDOWEVENT_CLOSE:
				machine->schedule_exit();
				break;
			case  SDL_WINDOWEVENT_LEAVE:
				ui_input_push_mouse_leave_event(machine, window->target);
				app_has_mouse_focus = 0;
				break;
			case SDL_WINDOWEVENT_MOVED:
				sdlwindow_clear(window);
				focus_window = window;
				break;
			case SDL_WINDOWEVENT_RESIZED:
				if (SDL13_COMBINE_RESIZE)
				{
					window->resize_width = event.window.data1;
					window->resize_height = event.window.data2;
					window->last_resize = osd_ticks();
				}
				else
				{
					if (event.window.data1 != window->width || event.window.data2 != window->height)
						sdlwindow_resize(window, event.window.data1, event.window.data2);
				}
				focus_window = window;
				break;
			case SDL_WINDOWEVENT_ENTER:
				app_has_mouse_focus = 1;
				/* fall through */
			case SDL_WINDOWEVENT_FOCUS_GAINED:
			case SDL_WINDOWEVENT_EXPOSED:
			case SDL_WINDOWEVENT_MAXIMIZED:
			case SDL_WINDOWEVENT_RESTORED:
				focus_window = window;
				break;
			}
			break;
		}
#endif
		}
	}
#if (SDL_VERSION_ATLEAST(1,3,0))
	resize_all_windows();
#endif
}


//============================================================
//  sdlinput_release_keys
//============================================================


void  sdlinput_release_keys(running_machine *machine)
{
	// FIXME: SDL >= 1.3 will nuke the window event buffer when
	// a window is closed. This will leave keys in a pressed
	// state when a window is destroyed and recreated.
#if (SDL_VERSION_ATLEAST(1,3,0))
	device_info *devinfo;
	int index;

	for (index = 0; ;index++)
	{
		devinfo = generic_device_find_index( keyboard_list, index);
		if (devinfo == NULL)
			break;
		memset(&devinfo->keyboard.state, 0, sizeof(devinfo->keyboard.state));
	}
#endif
}


//============================================================
//  sdlinput_should_hide_mouse
//============================================================

int sdlinput_should_hide_mouse(running_machine *machine)
{
	// if we are paused, no
	if (input_paused)
		return FALSE;

	// if neither mice nor lightguns enabled in the core, then no
	if (!mouse_enabled && !lightgun_enabled)
		return FALSE;

	if (!app_has_mouse_focus)
		return FALSE;

	// otherwise, yes
	return TRUE;
}


//============================================================
//  osd_customize_inputport_list
//============================================================

void osd_customize_input_type_list(input_type_desc *typelist)
{
	int mameid_code ,ui_code;
	input_type_desc *typedesc;
	const char*  uimode;

	// loop over the defaults
	for (typedesc = typelist; typedesc != NULL; typedesc = typedesc->next)
	{
		switch (typedesc->type)
		{
			// configurable UI mode switch
			case IPT_UI_TOGGLE_UI:
				uimode = (const char *)options_get_string(mame_options(), SDLOPTION_UIMODEKEY);
				if(!strcmp(uimode,"auto")) {
					#if defined(__APPLE__) && defined(__MACH__)
					mameid_code = lookup_mame_code("ITEM_ID_INSERT");
					#else
					mameid_code = lookup_mame_code("ITEM_ID_SCRLOCK");
					#endif
				} else {
					mameid_code = lookup_mame_code(uimode);
				}
				ui_code = INPUT_CODE(DEVICE_CLASS_KEYBOARD, 0, ITEM_CLASS_SWITCH, ITEM_MODIFIER_NONE, mameid_code);
				input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD], ui_code);
				break;
			// alt-enter for fullscreen
			case IPT_OSD_1:
				typedesc->token = "TOGGLE_FULLSCREEN";
				typedesc->name = "Toggle Fullscreen";
				input_seq_set_2(&typedesc->seq[SEQ_TYPE_STANDARD], KEYCODE_ENTER, KEYCODE_LALT);
				break;

			// disable UI_SELECT when LALT is down, this stops selecting
			// things in the menu when toggling fullscreen with LALT+ENTER
/*          case IPT_UI_SELECT:
                input_seq_set_3(&typedesc->seq[SEQ_TYPE_STANDARD], KEYCODE_ENTER, SEQCODE_NOT, KEYCODE_LALT);
                break;*/

			// page down for fastforward (must be OSD_3 as per src/emu/ui.c)
			case IPT_UI_FAST_FORWARD:
				typedesc->token = "FAST_FORWARD";
				typedesc->name = "Fast Forward";
				input_seq_set_1(&typedesc->seq[SEQ_TYPE_STANDARD], KEYCODE_PGDN);
				break;

			// OSD hotkeys use LCTRL and start at F3, they start at
			// F3 because F1-F2 are hardcoded into many drivers to
			// various dipswitches, and pressing them together with
			// LCTRL will still press/toggle these dipswitches.

			// LCTRL-F3 to toggle fullstretch
			case IPT_OSD_2:
				typedesc->token = "TOGGLE_FULLSTRETCH";
				typedesc->name = "Toggle Uneven stretch";
				input_seq_set_2(&typedesc->seq[SEQ_TYPE_STANDARD], KEYCODE_F3, KEYCODE_LCONTROL);
				break;
			// add a Not lcrtl condition to the reset key
			case IPT_UI_SOFT_RESET:
				input_seq_set_5(&typedesc->seq[SEQ_TYPE_STANDARD], KEYCODE_F3, SEQCODE_NOT, KEYCODE_LCONTROL, SEQCODE_NOT, KEYCODE_LSHIFT);
				break;

			// LCTRL-F4 to toggle keep aspect
			case IPT_OSD_4:
				typedesc->token = "TOGGLE_KEEP_ASPECT";
				typedesc->name = "Toggle Keepaspect";
				input_seq_set_2(&typedesc->seq[SEQ_TYPE_STANDARD], KEYCODE_F4, KEYCODE_LCONTROL);
				break;
			// add a Not lcrtl condition to the show gfx key
			case IPT_UI_SHOW_GFX:
				input_seq_set_3(&typedesc->seq[SEQ_TYPE_STANDARD], KEYCODE_F4, SEQCODE_NOT, KEYCODE_LCONTROL);
				break;

			// LCTRL-F5 to toggle OpenGL filtering
			case IPT_OSD_5:
				typedesc->token = "TOGGLE_FILTER";
				typedesc->name = "Toggle Filter";
				input_seq_set_2(&typedesc->seq[SEQ_TYPE_STANDARD], KEYCODE_F5, KEYCODE_LCONTROL);
				break;
			// add a Not lcrtl condition to the toggle debug key
			case IPT_UI_TOGGLE_DEBUG:
				input_seq_set_3(&typedesc->seq[SEQ_TYPE_STANDARD], KEYCODE_F5, SEQCODE_NOT, KEYCODE_LCONTROL);
				break;

			// LCTRL-F6 to decrease OpenGL prescaling
			case IPT_OSD_6:
				typedesc->token = "DECREASE_PRESCALE";
				typedesc->name = "Decrease Prescaling";
				input_seq_set_2(&typedesc->seq[SEQ_TYPE_STANDARD], KEYCODE_F6, KEYCODE_LCONTROL);
				break;
			// add a Not lcrtl condition to the toggle cheat key
			case IPT_UI_TOGGLE_CHEAT:
				input_seq_set_3(&typedesc->seq[SEQ_TYPE_STANDARD], KEYCODE_F6, SEQCODE_NOT, KEYCODE_LCONTROL);
				break;

			// LCTRL-F7 to increase OpenGL prescaling
			case IPT_OSD_7:
				typedesc->token = "INCREASE_PRESCALE";
				typedesc->name = "Increase Prescaling";
				input_seq_set_2(&typedesc->seq[SEQ_TYPE_STANDARD], KEYCODE_F7, KEYCODE_LCONTROL);
				break;
			// add a Not lcrtl condition to the load state key
			case IPT_UI_LOAD_STATE:
				input_seq_set_5(&typedesc->seq[SEQ_TYPE_STANDARD], KEYCODE_F7, SEQCODE_NOT, KEYCODE_LCONTROL, SEQCODE_NOT, KEYCODE_LSHIFT);
				break;

			// add a Not lcrtl condition to the throttle key
			case IPT_UI_THROTTLE:
				input_seq_set_3(&typedesc->seq[SEQ_TYPE_STANDARD], KEYCODE_F10, SEQCODE_NOT, KEYCODE_LCONTROL);
				break;

			// disable the config menu if the ALT key is down
			// (allows ALT-TAB to switch between apps)
			case IPT_UI_CONFIGURE:
				input_seq_set_5(&typedesc->seq[SEQ_TYPE_STANDARD], KEYCODE_TAB, SEQCODE_NOT, KEYCODE_LALT, SEQCODE_NOT, KEYCODE_RALT);
				break;
		}
	}
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
//  device_list_free_devices
//============================================================

static void device_list_free_devices(device_info **devlist_head)
{
	device_info *curdev, *next;

	for (curdev = *devlist_head; curdev != NULL; )
	{
		next = curdev->next;
		generic_device_free(curdev);
		curdev = next;
	}
	*devlist_head = NULL;
}


//============================================================
//  generic_device_alloc
//============================================================

static device_info *generic_device_alloc(device_info **devlist_head_ptr, const char *name)
{
	device_info **curdev_ptr;
	device_info *devinfo;

	// allocate memory for the device object
	devinfo = global_alloc_clear(device_info);
	devinfo->head = devlist_head_ptr;

	// allocate a UTF8 copy of the name
	devinfo->name = (char *) global_alloc_array(char, strlen(name)+1);
	if (devinfo->name == NULL)
		goto error;
	strcpy(devinfo->name, (char *)name);

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
	{
		global_free((void *)devinfo->name);
	}
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


static device_info *generic_device_find_index(device_info *devlist_head, int index)
{
	device_info *orig_head = devlist_head;

	while (devlist_head != NULL)
	{
		if (generic_device_index(orig_head, devlist_head) == index)
		{
			return devlist_head;
		}

		devlist_head = devlist_head->next;
	}
	return NULL;
}


//============================================================
//  generic_device_reset
//============================================================

static void generic_device_reset(device_info *devinfo)
{
	// keyboard case
	if (devinfo->head == &keyboard_list)
		memset(&devinfo->keyboard, 0, sizeof(devinfo->keyboard));

	// mouse/lightgun case
	else if (devinfo->head == &mouse_list || devinfo->head == &lightgun_list)
		memset(&devinfo->mouse, 0, sizeof(devinfo->mouse));

	// joystick case
	else if (devinfo->head == &joystick_list)
	{
		memset(&devinfo->joystick, 0, sizeof(devinfo->joystick));
	}
}


//============================================================
//  generic_button_get_state
//============================================================

static INT32 generic_button_get_state(void *device_internal, void *item_internal)
{
	INT32 *itemdata = (INT32 *) item_internal;

	// return the current state
	return *itemdata >> 7;
}


//============================================================
//  generic_axis_get_state
//============================================================

static INT32 generic_axis_get_state(void *device_internal, void *item_internal)
{
	INT32 *axisdata = (INT32 *) item_internal;

	// return the current state
	return *axisdata;
}
