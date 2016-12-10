/***************************************************************************

    crsshair.h

    Crosshair handling.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __CRSSHAIR_H__
#define __CRSSHAIR_H__


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define CROSSHAIR_SCREEN_NONE	((device_t *) 0)
#define CROSSHAIR_SCREEN_ALL	((device_t *) ~0)

/* user settings for visibility mode */
#define CROSSHAIR_VISIBILITY_OFF				0
#define CROSSHAIR_VISIBILITY_ON					1
#define CROSSHAIR_VISIBILITY_AUTO				2
#define CROSSHAIR_VISIBILITY_DEFAULT			CROSSHAIR_VISIBILITY_AUTO

/* range allowed for auto visibility */
#define CROSSHAIR_VISIBILITY_AUTOTIME_MIN			0
#define CROSSHAIR_VISIBILITY_AUTOTIME_MAX			50
#define CROSSHAIR_VISIBILITY_AUTOTIME_DEFAULT		15

/* maximum crosshair pic filename size */
#define CROSSHAIR_PIC_NAME_LENGTH				12



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* user-controllable settings for a player */
typedef struct _crosshair_user_settings crosshair_user_settings;
struct _crosshair_user_settings
{
	UINT8			used;		/* is used */
	UINT8			mode;		/* visibility mode */
	UINT8			auto_time;	/* time in seconds to blank crosshair if no movement */
	char			name[CROSSHAIR_PIC_NAME_LENGTH + 1];		/* bitmap name */
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* initializes the crosshair system */
void crosshair_init(running_machine *machine);

/* draws crosshair(s) in a given screen, if neccessary */
void crosshair_render(screen_device &screen);

/* sets the screen(s) for a given player's crosshair */
void crosshair_set_screen(running_machine *machine, int player, device_t *screen);

/* return TRUE if any crosshairs are used */
int crosshair_get_usage(running_machine *machine);

/* return the current crosshair settings for the given player */
void crosshair_get_user_settings(running_machine *machine, UINT8 player, crosshair_user_settings *settings);

/* modify the current crosshair settings for the given player */
void crosshair_set_user_settings(running_machine *machine, UINT8 player, crosshair_user_settings *settings);


#endif	/* __CRSSHAIR_H__ */
