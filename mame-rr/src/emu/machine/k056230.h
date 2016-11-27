/***************************************************************************

    Konami 056230

***************************************************************************/

#ifndef __K056230_H__
#define __K056230_H__

#include "devlegcy.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _k056230_interface k056230_interface;
struct _k056230_interface
{
	const char         *cpu;
	int                is_thunderh;
};

DECLARE_LEGACY_DEVICE(K056230, k056230);


/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

#define MDRV_K056230_ADD(_tag, _config) \
	MDRV_DEVICE_ADD(_tag, K056230, 0) \
	MDRV_DEVICE_CONFIG(_config)


/***************************************************************************
    DEVICE I/O FUNCTIONS
***************************************************************************/

extern READ32_DEVICE_HANDLER( lanc_ram_r );
extern WRITE32_DEVICE_HANDLER( lanc_ram_w );
extern READ8_DEVICE_HANDLER( k056230_r );
extern WRITE8_DEVICE_HANDLER( k056230_w );


#endif	/* __K056230_H__ */
