/******************************************************************************
* FILE
*   Yamaha 3812 emulator interface - MAME VERSION
*
* CREATED BY
*   Ernesto Corvi
*
* UPDATE LOG
*   JB  28-04-2002  Fixed simultaneous usage of all three different chip types.
*                       Used real sample rate when resample filter is active.
*       AAT 12-28-2001  Protected Y8950 from accessing unmapped port and keyboard handlers.
*   CHS 1999-01-09  Fixes new ym3812 emulation interface.
*   CHS 1998-10-23  Mame streaming sound chip update
*   EC  1998        Created Interface
*
* NOTES
*
******************************************************************************/
#include "emu.h"
#include "streams.h"
#include "3526intf.h"
#include "fm.h"
#include "sound/fmopl.h"


typedef struct _ym3526_state ym3526_state;
struct _ym3526_state
{
	sound_stream *	stream;
	emu_timer *		timer[2];
	void *			chip;
	const ym3526_interface *intf;
	running_device *device;
};


INLINE ym3526_state *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->type() == SOUND_YM3526);
	return (ym3526_state *)downcast<legacy_device_base *>(device)->token();
}


/* IRQ Handler */
static void IRQHandler(void *param,int irq)
{
	ym3526_state *info = (ym3526_state *)param;
	if (info->intf->handler) (info->intf->handler)(info->device, irq ? ASSERT_LINE : CLEAR_LINE);
}
/* Timer overflow callback from timer.c */
static TIMER_CALLBACK( timer_callback_0 )
{
	ym3526_state *info = (ym3526_state *)ptr;
	ym3526_timer_over(info->chip,0);
}
static TIMER_CALLBACK( timer_callback_1 )
{
	ym3526_state *info = (ym3526_state *)ptr;
	ym3526_timer_over(info->chip,1);
}
/* TimerHandler from fm.c */
static void TimerHandler(void *param,int c,attotime period)
{
	ym3526_state *info = (ym3526_state *)param;
	if( attotime_compare(period, attotime_zero) == 0 )
	{	/* Reset FM Timer */
		timer_enable(info->timer[c], 0);
	}
	else
	{	/* Start FM Timer */
		timer_adjust_oneshot(info->timer[c], period, 0);
	}
}


static STREAM_UPDATE( ym3526_stream_update )
{
	ym3526_state *info = (ym3526_state *)param;
	ym3526_update_one(info->chip, outputs[0], samples);
}

static void _stream_update(void *param, int interval)
{
	ym3526_state *info = (ym3526_state *)param;
	stream_update(info->stream);
}


static DEVICE_START( ym3526 )
{
	static const ym3526_interface dummy = { 0 };
	ym3526_state *info = get_safe_token(device);
	int rate = device->clock()/72;

	info->intf = device->baseconfig().static_config() ? (const ym3526_interface *)device->baseconfig().static_config() : &dummy;
	info->device = device;

	/* stream system initialize */
	info->chip = ym3526_init(device,device->clock(),rate);
	assert_always(info->chip != NULL, "Error creating YM3526 chip");

	info->stream = stream_create(device,0,1,rate,info,ym3526_stream_update);
	/* YM3526 setup */
	ym3526_set_timer_handler (info->chip, TimerHandler, info);
	ym3526_set_irq_handler   (info->chip, IRQHandler, info);
	ym3526_set_update_handler(info->chip, _stream_update, info);

	info->timer[0] = timer_alloc(device->machine, timer_callback_0, info);
	info->timer[1] = timer_alloc(device->machine, timer_callback_1, info);
}

static DEVICE_STOP( ym3526 )
{
	ym3526_state *info = get_safe_token(device);
	ym3526_shutdown(info->chip);
}

static DEVICE_RESET( ym3526 )
{
	ym3526_state *info = get_safe_token(device);
	ym3526_reset_chip(info->chip);
}


READ8_DEVICE_HANDLER( ym3526_r )
{
	ym3526_state *info = get_safe_token(device);
	return ym3526_read(info->chip, offset & 1);
}

WRITE8_DEVICE_HANDLER( ym3526_w )
{
	ym3526_state *info = get_safe_token(device);
	ym3526_write(info->chip, offset & 1, data);
}

READ8_DEVICE_HANDLER( ym3526_status_port_r ) { return ym3526_r(device, 0); }
READ8_DEVICE_HANDLER( ym3526_read_port_r ) { return ym3526_r(device, 1); }
WRITE8_DEVICE_HANDLER( ym3526_control_port_w ) { ym3526_w(device, 0, data); }
WRITE8_DEVICE_HANDLER( ym3526_write_port_w ) { ym3526_w(device, 1, data); }


/**************************************************************************
 * Generic get_info
 **************************************************************************/

DEVICE_GET_INFO( ym3526 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(ym3526_state);				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( ym3526 );				break;
		case DEVINFO_FCT_STOP:							info->stop = DEVICE_STOP_NAME( ym3526 );				break;
		case DEVINFO_FCT_RESET:							info->reset = DEVICE_RESET_NAME( ym3526 );				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "YM3526");							break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Yamaha FM");						break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");								break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);							break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}


DEFINE_LEGACY_SOUND_DEVICE(YM3526, ym3526);
