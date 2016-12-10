/*****************************************************************************
 *
 * audio/snes_spc.h
 *
 ****************************************************************************/

#ifndef __SNES_SPC_H__
#define __SNES_SPC_H__

#include "devcb.h"

#define SNES_SPCRAM_SIZE      0x10000


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

DECLARE_LEGACY_SOUND_DEVICE(SNES, snes_sound);
#define SNES_SPC SOUND_SNES


/***************************************************************************
    I/O PROTOTYPES
***************************************************************************/

READ8_DEVICE_HANDLER( spc_io_r );
WRITE8_DEVICE_HANDLER( spc_io_w );
READ8_DEVICE_HANDLER( spc_ram_r );
WRITE8_DEVICE_HANDLER( spc_ram_w );
READ8_DEVICE_HANDLER( spc_ipl_r );
READ8_DEVICE_HANDLER( spc_port_out );
WRITE8_DEVICE_HANDLER( spc_port_in );

UINT8 *spc_get_ram(running_device *device);

#endif /* __SNES_SPC_H__ */
