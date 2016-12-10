/*********************************************************************

    debugcon.h

    Debugger console engine.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#ifndef __DEBUGCON_H__
#define __DEBUGCON_H__

#include "textbuf.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define MAX_COMMAND_LENGTH					512
#define MAX_COMMAND_PARAMS					16

/* flags for command parsing */
#define CMDFLAG_NONE						(0x0000)
#define CMDFLAG_KEEP_QUOTES					(0x0001)

/* values for the error code in a command error */
#define CMDERR_NONE							(0)
#define CMDERR_UNKNOWN_COMMAND				(1)
#define CMDERR_AMBIGUOUS_COMMAND			(2)
#define CMDERR_UNBALANCED_PARENS			(3)
#define CMDERR_UNBALANCED_QUOTES			(4)
#define CMDERR_NOT_ENOUGH_PARAMS			(5)
#define CMDERR_TOO_MANY_PARAMS				(6)
#define CMDERR_EXPRESSION_ERROR				(7)

/* parameter separator macros */
#define CMDPARAM_SEPARATOR					"\0"
#define CMDPARAM_TERMINATOR					"\0\0"



/***************************************************************************
    MACROS
***************************************************************************/

/* command error assembly/disassembly macros */
#define CMDERR_ERROR_CLASS(x)				((x) >> 16)
#define CMDERR_ERROR_OFFSET(x)				((x) & 0xffff)
#define MAKE_CMDERR(a,b)					(((a) << 16) | ((b) & 0xffff))

/* macros to assemble specific error conditions */
#define MAKE_CMDERR_UNKNOWN_COMMAND(x)		MAKE_CMDERR(CMDERR_UNKNOWN_COMMAND, (x))
#define MAKE_CMDERR_AMBIGUOUS_COMMAND(x)	MAKE_CMDERR(CMDERR_AMBIGUOUS_COMMAND, (x))
#define MAKE_CMDERR_UNBALANCED_PARENS(x)	MAKE_CMDERR(CMDERR_UNBALANCED_PARENS, (x))
#define MAKE_CMDERR_UNBALANCED_QUOTES(x)	MAKE_CMDERR(CMDERR_UNBALANCED_QUOTES, (x))
#define MAKE_CMDERR_NOT_ENOUGH_PARAMS(x)	MAKE_CMDERR(CMDERR_NOT_ENOUGH_PARAMS, (x))
#define MAKE_CMDERR_TOO_MANY_PARAMS(x)		MAKE_CMDERR(CMDERR_TOO_MANY_PARAMS, (x))
#define MAKE_CMDERR_EXPRESSION_ERROR(x)		MAKE_CMDERR(CMDERR_EXPRESSION_ERROR, (x))


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* CMDERR is an error code for command evaluation */
typedef UINT32 CMDERR;



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* initialization */
void				debug_console_init(running_machine *machine);

/* command handling */
CMDERR				debug_console_execute_command(running_machine *machine, const char *command, int echo);
CMDERR				debug_console_validate_command(running_machine *machine, const char *command);
void				debug_console_register_command(running_machine *machine, const char *command, UINT32 flags, int ref, int minparams, int maxparams, void (*handler)(running_machine *machine, int ref, int params, const char **param));
const char *		debug_cmderr_to_string(CMDERR error);

/* console management */
void CLIB_DECL		debug_console_printf(running_machine *machine, const char *format, ...) ATTR_PRINTF(2,3);
void CLIB_DECL		debug_console_vprintf(running_machine *machine, const char *format, va_list args);
void CLIB_DECL		debug_console_printf_wrap(running_machine *machine, int wrapcol, const char *format, ...) ATTR_PRINTF(3,4);
text_buffer *		debug_console_get_textbuf(void);

/* errorlog management */
void				debug_errorlog_write_line(running_machine &machine, const char *line);
text_buffer *		debug_errorlog_get_textbuf(void);

#endif
