/***************************************************************************

    Intel 8259A

    Programmable Interrupt Controller

                            _____   _____
                   _CS   1 |*    \_/     | 28  VCC
                   _WR   2 |             | 27  A0
                   _RD   3 |             | 26  _INTA
                    D7   4 |             | 25  IR7
                    D6   5 |             | 24  IR6
                    D5   6 |             | 23  IR5
                    D4   7 |    8259A    | 22  IR4
                    D3   8 |             | 21  IR3
                    D2   9 |             | 20  IR2
                    D1  10 |             | 19  IR1
                    D0  11 |             | 18  IR0
                  CAS0  12 |             | 17  INT
                  CAS1  13 |             | 16  _SP/_EN
                   GND  14 |_____________| 15  CAS2

***************************************************************************/

#ifndef __PIC8259_H__
#define __PIC8259_H__

#include "devlegcy.h"
#include "devcb.h"

DECLARE_LEGACY_DEVICE(PIC8259, pic8259);

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct pic8259_interface
{
	/* Called when int line changes */
	devcb_write_line out_int_func;
};


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MDRV_PIC8259_ADD(_tag, _intrf) \
	MDRV_DEVICE_ADD(_tag, PIC8259, 0) \
	MDRV_DEVICE_CONFIG(_intrf)


/* device interface */
READ8_DEVICE_HANDLER( pic8259_r );
WRITE8_DEVICE_HANDLER( pic8259_w );
int pic8259_acknowledge(running_device *device);

/* interrupt requests */
WRITE_LINE_DEVICE_HANDLER( pic8259_ir0_w );
WRITE_LINE_DEVICE_HANDLER( pic8259_ir1_w );
WRITE_LINE_DEVICE_HANDLER( pic8259_ir2_w );
WRITE_LINE_DEVICE_HANDLER( pic8259_ir3_w );
WRITE_LINE_DEVICE_HANDLER( pic8259_ir4_w );
WRITE_LINE_DEVICE_HANDLER( pic8259_ir5_w );
WRITE_LINE_DEVICE_HANDLER( pic8259_ir6_w );
WRITE_LINE_DEVICE_HANDLER( pic8259_ir7_w );


#endif /* __PIC8259_H__ */
