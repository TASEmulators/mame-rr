//============================================================
//
//  sdlprefix.h - prefix file, included by ALL files
//
//  Copyright (c) 1996-2006, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

//============================================================
// System specific defines
//============================================================

/* Only problems ... */
#ifdef SDLMAME_WIN32
#define _SDL_main_h
#endif


#ifdef __APPLE__
#define SDLMAME_DARWIN 1
#endif /* __APPLE__ */

#ifdef SDLMAME_UNIX

#if defined(__sun__) && defined(__svr4__)
#define SDLMAME_SOLARIS 1
#undef _XOPEN_SOURCE
#undef _XOPEN_VERSION
#undef _XOPEN_SOURCE_EXTENDED
#undef _XPG6
#undef _XPG5
#undef _XPG4_2

#define _XOPEN_SOURCE
#define _XOPEN_VERSION 4

#elif defined(__irix__) || defined(__sgi)
#define SDLMAME_IRIX 1
/* Large file support on IRIX needs _SGI_SOURCE */
#undef _POSIX_SOURCE

#elif defined(__linux__)
#define SDLMAME_LINUX 1

#elif defined(__FreeBSD__) || defined(__DragonFly__) || defined(__OpenBSD__)
#define SDLMAME_BSD 1
#endif

// fix for Ubuntu 8.10
#ifdef _FORTIFY_SOURCE
#undef _FORTIFY_SOURCE
#endif

#endif /* SDLMAME_UNIX */

#ifdef _MSC_VER
void *__cdecl _alloca(size_t);
#define alloca _alloca
#endif

#ifdef __GNUC__
#define alloca	__builtin_alloca
#endif

//============================================================
// misc.
//============================================================

#define PATH_SEPARATOR		"/"

