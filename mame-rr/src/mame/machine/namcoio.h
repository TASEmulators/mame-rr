#ifndef __NAMCOIO_H__
#define __NAMCOIO_H__

#include "devlegcy.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _namcoio_interface namcoio_interface;
struct _namcoio_interface
{
	devcb_read8 in[4];
	devcb_write8 out[2];

	running_device *device;
};

DECLARE_LEGACY_DEVICE(NAMCO56XX, namcoio);
#define NAMCO58XX NAMCO56XX
#define NAMCO59XX NAMCO56XX

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MDRV_NAMCO56XX_ADD(_tag, _interface) \
	MDRV_DEVICE_ADD(_tag, NAMCO56XX, 0) \
	MDRV_DEVICE_CONFIG(_interface)

#define MDRV_NAMCO58XX_ADD(_tag, _interface) \
	MDRV_DEVICE_ADD(_tag, NAMCO58XX, 0) \
	MDRV_DEVICE_CONFIG(_interface)

#define MDRV_NAMCO59XX_ADD(_tag, _interface) \
	MDRV_DEVICE_ADD(_tag, NAMCO59XX, 0) \
	MDRV_DEVICE_CONFIG(_interface)


/***************************************************************************
    DEVICE I/O FUNCTIONS
***************************************************************************/

READ8_DEVICE_HANDLER( namcoio_r );
WRITE8_DEVICE_HANDLER( namcoio_w );

WRITE_LINE_DEVICE_HANDLER( namcoio_set_reset_line );
READ_LINE_DEVICE_HANDLER( namcoio_read_reset_line );


/* these must be used in the single drivers, inside a timer */
void namco_customio_56xx_run(running_device *device);
void namco_customio_58xx_run(running_device *device);
void namco_customio_59xx_run(running_device *device);


#endif	/* __NAMCOIO_H__ */
