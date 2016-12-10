/***************************************************************************

    config.h

    Wrappers for handling MAME configuration files

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "xmlfile.h"



/*************************************
 *
 *  Constants
 *
 *************************************/

#define CONFIG_VERSION			10

enum
{
	CONFIG_TYPE_INIT = 0,					/* opportunity to initialize things first */
	CONFIG_TYPE_CONTROLLER,					/* loading from controller file */
	CONFIG_TYPE_DEFAULT,					/* loading from default.cfg */
	CONFIG_TYPE_GAME,					/* loading from game.cfg */
	CONFIG_TYPE_FINAL					/* opportunity to finish initialization */
};



/*************************************
 *
 *  Type definitions
 *
 *************************************/

typedef void (*config_callback_func)(running_machine *machine, int config_type, xml_data_node *parentnode);



/*************************************
 *
 *  Function prototypes
 *
 *************************************/

void config_init(running_machine *machine);
void config_register(running_machine *machine, const char *nodename, config_callback_func load, config_callback_func save);
int config_load_settings(running_machine *machine);
void config_save_settings(running_machine *machine);

#endif	/* __CONFIG_H__ */
