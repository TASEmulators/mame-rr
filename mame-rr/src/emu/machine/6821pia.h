/**********************************************************************

    Motorola 6821 PIA interface and emulation

    Notes:
        * pia6821_get_port_b_z_mask() gives the caller the bitmask
          that show which bits are high-impendance when
          reading port B, thus neither 0 or 1.
          pia6821_get_output_cb2_z() returns the same
          information for the CB2 pin
        * pia6821_set_port_a_z_mask allows the input callback to
          indicate which port A bits are disconnected.
          For these bit, the read operation will return the
          output buffer's contents
        * the 'alt' interface functions are used when the A0 and A1
          address bits are swapped
        * all 'int' data or return values are Boolean

**********************************************************************/

#ifndef __6821PIA_H__
#define __6821PIA_H__

#include "devlegcy.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/*------------- PIA interface structure -----------------*/

typedef struct _pia6821_interface pia6821_interface;
struct _pia6821_interface
{
	devcb_read8 in_a_func;
	devcb_read8 in_b_func;
	devcb_read_line in_ca1_func;
	devcb_read_line in_cb1_func;
	devcb_read_line in_ca2_func;
	devcb_read_line in_cb2_func;
	devcb_write8 out_a_func;
	devcb_write8 out_b_func;
	devcb_write_line out_ca2_func;
	devcb_write_line out_cb2_func;
	devcb_write_line irq_a_func;
	devcb_write_line irq_b_func;
};

DECLARE_LEGACY_DEVICE(PIA6821, pia6821);
DECLARE_LEGACY_DEVICE(PIA6822, pia6822);



/***************************************************************************
    PROTOTYPES
***************************************************************************/

READ8_DEVICE_HANDLER( pia6821_r );
WRITE8_DEVICE_HANDLER( pia6821_w );

READ8_DEVICE_HANDLER( pia6821_alt_r );
WRITE8_DEVICE_HANDLER( pia6821_alt_w );

UINT8 pia6821_get_port_b_z_mask(running_device *device);  /* see first note */
void pia6821_set_port_a_z_mask(running_device *device, UINT8 data);  /* see second note */

READ8_DEVICE_HANDLER( pia6821_porta_r );
WRITE8_DEVICE_HANDLER( pia6821_porta_w );
void pia6821_set_input_a(running_device *device, UINT8 data, UINT8 z_mask);
UINT8 pia6821_get_output_a(running_device *device);

READ_LINE_DEVICE_HANDLER( pia6821_ca1_r );
WRITE_LINE_DEVICE_HANDLER( pia6821_ca1_w );

READ_LINE_DEVICE_HANDLER( pia6821_ca2_r );
WRITE_LINE_DEVICE_HANDLER( pia6821_ca2_w );
int pia6821_get_output_ca2(running_device *device);
int pia6821_get_output_ca2_z(running_device *device);

READ8_DEVICE_HANDLER( pia6821_portb_r );
WRITE8_DEVICE_HANDLER( pia6821_portb_w );
UINT8 pia6821_get_output_b(running_device *device);

READ_LINE_DEVICE_HANDLER( pia6821_cb1_r );
WRITE_LINE_DEVICE_HANDLER( pia6821_cb1_w );

READ_LINE_DEVICE_HANDLER( pia6821_cb2_r );
WRITE_LINE_DEVICE_HANDLER( pia6821_cb2_w );
int pia6821_get_output_cb2(running_device *device);
int pia6821_get_output_cb2_z(running_device *device);

int pia6821_get_irq_a(running_device *device);
int pia6821_get_irq_b(running_device *device);


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MDRV_PIA6821_ADD(_tag, _intrf) \
	MDRV_DEVICE_ADD(_tag, PIA6821, 0) \
	MDRV_DEVICE_CONFIG(_intrf)

#define MDRV_PIA6821_MODIFY(_tag, _intrf) \
	MDRV_DEVICE_MODIFY(_tag) \
	MDRV_DEVICE_CONFIG(_intrf)

#define MDRV_PIA6822_ADD(_tag, _intrf) \
	MDRV_DEVICE_ADD(_tag, PIA6822, 0) \
	MDRV_DEVICE_CONFIG(_intrf)

#define MDRV_PIA6822_MODIFY(_tag, _intrf) \
	MDRV_DEVICE_MODIFY(_tag) \
	MDRV_DEVICE_CONFIG(_intrf)

#endif /* __6821NEW_H__ */
