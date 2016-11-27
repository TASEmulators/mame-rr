/***************************************************************************

    NEC uPD4701

    Incremental Encoder Control

    2009-06 Converted to be a device

***************************************************************************/

#include "emu.h"
#include "upd4701.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _upd4701_state upd4701_state;
struct _upd4701_state
{
	int cs;
	int xy;
	int ul;
	int resetx;
	int resety;
	int latchx;
	int latchy;
	int startx;
	int starty;
	int x;
	int y;
	int switches;
	int latchswitches;
	int cf;
};

/* x,y increments can be 12bit (see MASK_COUNTER), hence we need a couple of
16bit handlers in the following  */

#define MASK_SWITCHES ( 7 )
#define MASK_COUNTER ( 0xfff )


/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE upd4701_state *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert((device->type() == UPD4701));
	return (upd4701_state *)downcast<legacy_device_base *>(device)->token();
}


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    upd4701_ul_w
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( upd4701_ul_w )
{
	upd4701_state *upd4701 = get_safe_token(device);
	upd4701->ul = data;
}

/*-------------------------------------------------
    upd4701_xy_w
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( upd4701_xy_w )
{
	upd4701_state *upd4701 = get_safe_token(device);
	upd4701->xy = data;
}

/*-------------------------------------------------
    upd4701_cs_w
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( upd4701_cs_w )
{
	upd4701_state *upd4701 = get_safe_token(device);

	if (data != upd4701->cs)
	{
		upd4701->cs = data;

		if (!upd4701->cs)
		{
			upd4701->latchx = (upd4701->x - upd4701->startx) & MASK_COUNTER;
			upd4701->latchy = (upd4701->y - upd4701->starty) & MASK_COUNTER;

			upd4701->latchswitches = (~upd4701->switches) & MASK_SWITCHES;
			if (upd4701->latchswitches != 0)
			{
				upd4701->latchswitches |= 8;
			}

			upd4701->cf = 1;
		}
	}
}

/*-------------------------------------------------
    upd4701_resetx_w
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( upd4701_resetx_w )
{
	upd4701_state *upd4701 = get_safe_token(device);

	if (upd4701->resetx != data)
	{
		upd4701->resetx = data;

		if (upd4701->resetx)
		{
			upd4701->startx = upd4701->x;
		}
	}
}

/*-------------------------------------------------
    upd4701_resety_w
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( upd4701_resety_w )
{
	upd4701_state *upd4701 = get_safe_token(device);

	if (upd4701->resety != data)
	{
		upd4701->resety = data;

		if (upd4701->resety)
		{
			upd4701->starty = upd4701->y;
		}
	}
}

/*-------------------------------------------------
    upd4701_x_add
-------------------------------------------------*/

WRITE16_DEVICE_HANDLER( upd4701_x_add )
{
	upd4701_state *upd4701 = get_safe_token(device);

	if (!upd4701->resetx && data != 0)
	{
		upd4701->x += data;

		if (upd4701->cs)
		{
			upd4701->cf = 0;
		}
	}
}

/*-------------------------------------------------
    upd4701_y_add
-------------------------------------------------*/

WRITE16_DEVICE_HANDLER( upd4701_y_add )
{
	upd4701_state *upd4701 = get_safe_token(device);

	if (!upd4701->resety && data != 0)
	{
		upd4701->y += data;

		if (upd4701->cs)
		{
			upd4701->cf = 0;
		}
	}
}

/*-------------------------------------------------
    upd4701_switches_set
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( upd4701_switches_set )
{
	upd4701_state *upd4701 = get_safe_token(device);
	upd4701->switches = data;
}

/*-------------------------------------------------
    upd4701_d_r
-------------------------------------------------*/

READ16_DEVICE_HANDLER( upd4701_d_r )
{
	upd4701_state *upd4701 = get_safe_token(device);
	int data;

	if (upd4701->cs)
	{
		return 0xff;
	}

	if (upd4701->xy)
	{
		data = upd4701->latchy;
	}
	else
	{
		data = upd4701->latchx;
	}

	data |= upd4701->latchswitches << 12;

	if (upd4701->ul)
	{
		return data >> 8;
	}
	else
	{
		return data & 0xff;
	}
}

/*-------------------------------------------------
    upd4701_sf_r
-------------------------------------------------*/

READ8_DEVICE_HANDLER( upd4701_sf_r )
{
	upd4701_state *upd4701 = get_safe_token(device);

	if ((upd4701->switches & MASK_SWITCHES) != MASK_SWITCHES)
	{
		return 0;
	}

	return 1;
}

/*-------------------------------------------------
    upd4701_cf_r
-------------------------------------------------*/

READ8_DEVICE_HANDLER( upd4701_cf_r )
{
	upd4701_state *upd4701 = get_safe_token(device);
	return upd4701->cf;
}

/*-------------------------------------------------
    DEVICE_START( upd4701 )
-------------------------------------------------*/

static DEVICE_START( upd4701 )
{
	upd4701_state *upd4701 = get_safe_token(device);

	/* register for state saving */
	state_save_register_device_item(device, 0, upd4701->cs);
	state_save_register_device_item(device, 0, upd4701->xy);
	state_save_register_device_item(device, 0, upd4701->ul);
	state_save_register_device_item(device, 0, upd4701->resetx);
	state_save_register_device_item(device, 0, upd4701->resety);
	state_save_register_device_item(device, 0, upd4701->latchx);
	state_save_register_device_item(device, 0, upd4701->latchy);
	state_save_register_device_item(device, 0, upd4701->startx);
	state_save_register_device_item(device, 0, upd4701->starty);
	state_save_register_device_item(device, 0, upd4701->x);
	state_save_register_device_item(device, 0, upd4701->y);
	state_save_register_device_item(device, 0, upd4701->switches);
	state_save_register_device_item(device, 0, upd4701->latchswitches);
	state_save_register_device_item(device, 0, upd4701->cf);
}

/*-------------------------------------------------
    DEVICE_RESET( upd4701 )
-------------------------------------------------*/

static DEVICE_RESET( upd4701 )
{
	upd4701_state *upd4701 = get_safe_token(device);

	upd4701->cs = 1;
	upd4701->xy = 0;
	upd4701->ul = 0;
	upd4701->resetx = 0;
	upd4701->resety = 0;
	upd4701->latchx = 0;
	upd4701->latchy = 0;
	upd4701->startx = 0;
	upd4701->starty = 0;
	upd4701->x = 0;
	upd4701->y = 0;
	upd4701->switches = 0;
	upd4701->latchswitches = 0;
	upd4701->cf = 1;
}

/*-------------------------------------------------
    device definition
-------------------------------------------------*/

static const char DEVTEMPLATE_SOURCE[] = __FILE__;

#define DEVTEMPLATE_ID(p,s)		p##upd4701##s
#define DEVTEMPLATE_FEATURES	DT_HAS_START | DT_HAS_RESET
#define DEVTEMPLATE_NAME		"NEC uPD4701 Encoder"
#define DEVTEMPLATE_FAMILY		"NEC uPD4701 Encoder"
#include "devtempl.h"


DEFINE_LEGACY_DEVICE(UPD4701, upd4701);
