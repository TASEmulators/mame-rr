#pragma once

#ifndef __UPD7759_H__
#define __UPD7759_H__

#include "devlegcy.h"

/* There are two modes for the uPD7759, selected through the !MD pin.
   This is the mode select input.  High is stand alone, low is slave.
   We're making the assumption that nobody switches modes through
   software. */

#define UPD7759_STANDARD_CLOCK		640000

typedef struct _upd7759_interface upd7759_interface;
struct _upd7759_interface
{
	void (*drqcallback)(running_device *device, int param);	/* drq callback (per chip, slave mode only) */
};

void upd7759_set_bank_base(running_device *device, offs_t base);

void upd7759_reset_w(running_device *device, UINT8 data);
void upd7759_start_w(running_device *device, UINT8 data);
int upd7759_busy_r(running_device *device);
WRITE8_DEVICE_HANDLER( upd7759_port_w );

DECLARE_LEGACY_SOUND_DEVICE(UPD7759, upd7759);

#endif /* __UPD7759_H__ */
