/***************************************************************************

  pcecommn.h

  Headers for the common stuff for NEC PC Engine/TurboGrafx16.

***************************************************************************/

#ifndef PCECOMMON_H
#define PCECOMMON_H

#define	PCE_MAIN_CLOCK		21477270

extern unsigned char *pce_user_ram; /* scratch RAM at F8 */
WRITE8_HANDLER ( pce_joystick_w );
 READ8_HANDLER ( pce_joystick_r );

#define TG_16_JOY_SIG		0x00
#define PCE_JOY_SIG			0x40
#define NO_CD_SIG			0x80
#define CD_SIG				0x00
/* these might be used to indicate something, but they always seem to return 1 */
#define CONST_SIG			0x30

struct pce_struct
{
	UINT8 io_port_options; /*driver-specific options for the PCE*/
};
extern struct pce_struct pce;
DRIVER_INIT( pce );
MACHINE_RESET( pce );

void pce_set_joystick_readinputport_callback( UINT8 (*joy_read)(running_machine *));
#endif
