#include "emu.h"
#include "streams.h"
#include "2203intf.h"
#include "fm.h"


typedef struct _ym2203_state ym2203_state;
struct _ym2203_state
{
	sound_stream *	stream;
	emu_timer *		timer[2];
	void *			chip;
	void *			psg;
	const ym2203_interface *intf;
	running_device *device;
};


INLINE ym2203_state *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->type() == SOUND_YM2203);
	return (ym2203_state *)downcast<legacy_device_base *>(device)->token();
}


static void psg_set_clock(void *param, int clock)
{
	ym2203_state *info = (ym2203_state *)param;
	ay8910_set_clock_ym(info->psg, clock);
}

static void psg_write(void *param, int address, int data)
{
	ym2203_state *info = (ym2203_state *)param;
	ay8910_write_ym(info->psg, address, data);
}

static int psg_read(void *param)
{
	ym2203_state *info = (ym2203_state *)param;
	return ay8910_read_ym(info->psg);
}

static void psg_reset(void *param)
{
	ym2203_state *info = (ym2203_state *)param;
	ay8910_reset_ym(info->psg);
}

static const ssg_callbacks psgintf =
{
	psg_set_clock,
	psg_write,
	psg_read,
	psg_reset
};

/* IRQ Handler */
static void IRQHandler(void *param,int irq)
{
	ym2203_state *info = (ym2203_state *)param;
	if (info->intf->handler != NULL)
		(*info->intf->handler)(info->device, irq);
}

/* Timer overflow callback from timer.c */
static TIMER_CALLBACK( timer_callback_2203_0 )
{
	ym2203_state *info = (ym2203_state *)ptr;
	ym2203_timer_over(info->chip,0);
}

static TIMER_CALLBACK( timer_callback_2203_1 )
{
	ym2203_state *info = (ym2203_state *)ptr;
	ym2203_timer_over(info->chip,1);
}

/* update request from fm.c */
void ym2203_update_request(void *param)
{
	ym2203_state *info = (ym2203_state *)param;
	stream_update(info->stream);
}


static void timer_handler(void *param,int c,int count,int clock)
{
	ym2203_state *info = (ym2203_state *)param;
	if( count == 0 )
	{	/* Reset FM Timer */
		timer_enable(info->timer[c], 0);
	}
	else
	{	/* Start FM Timer */
		attotime period = attotime_mul(ATTOTIME_IN_HZ(clock), count);
		if (!timer_enable(info->timer[c], 1))
			timer_adjust_oneshot(info->timer[c], period, 0);
	}
}

static STREAM_UPDATE( ym2203_stream_update )
{
	ym2203_state *info = (ym2203_state *)param;
	ym2203_update_one(info->chip, outputs[0], samples);
}


static STATE_POSTLOAD( ym2203_intf_postload )
{
	ym2203_state *info = (ym2203_state *)param;
	ym2203_postload(info->chip);
}


static DEVICE_START( ym2203 )
{
	static const ym2203_interface generic_2203 =
	{
		{
			AY8910_LEGACY_OUTPUT,
			AY8910_DEFAULT_LOADS,
			DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL
		},
		NULL
	};
	const ym2203_interface *intf = device->baseconfig().static_config() ? (const ym2203_interface *)device->baseconfig().static_config() : &generic_2203;
	ym2203_state *info = get_safe_token(device);
	int rate = device->clock()/72; /* ??? */

	info->intf = intf;
	info->device = device;
	info->psg = ay8910_start_ym(NULL, SOUND_YM2203, device, device->clock(), &intf->ay8910_intf);
	assert_always(info->psg != NULL, "Error creating YM2203/AY8910 chip");

	/* Timer Handler set */
	info->timer[0] = timer_alloc(device->machine, timer_callback_2203_0, info);
	info->timer[1] = timer_alloc(device->machine, timer_callback_2203_1, info);

	/* stream system initialize */
	info->stream = stream_create(device,0,1,rate,info,ym2203_stream_update);

	/* Initialize FM emurator */
	info->chip = ym2203_init(info,device,device->clock(),rate,timer_handler,IRQHandler,&psgintf);
	assert_always(info->chip != NULL, "Error creating YM2203 chip");

	state_save_register_postload(device->machine, ym2203_intf_postload, info);
}

static DEVICE_STOP( ym2203 )
{
	ym2203_state *info = get_safe_token(device);
	ym2203_shutdown(info->chip);
	ay8910_stop_ym(info->psg);
}

static DEVICE_RESET( ym2203 )
{
	ym2203_state *info = get_safe_token(device);
	ym2203_reset_chip(info->chip);
}



READ8_DEVICE_HANDLER( ym2203_r )
{
	ym2203_state *info = get_safe_token(device);
	return ym2203_read(info->chip, offset & 1);
}

WRITE8_DEVICE_HANDLER( ym2203_w )
{
	ym2203_state *info = get_safe_token(device);
	ym2203_write(info->chip, offset & 1, data);
}


READ8_DEVICE_HANDLER( ym2203_status_port_r ) { return ym2203_r(device, 0); }
READ8_DEVICE_HANDLER( ym2203_read_port_r ) { return ym2203_r(device, 1); }
WRITE8_DEVICE_HANDLER( ym2203_control_port_w ) { ym2203_w(device, 0, data); }
WRITE8_DEVICE_HANDLER( ym2203_write_port_w ) { ym2203_w(device, 1, data); }


/**************************************************************************
 * Generic get_info
 **************************************************************************/

DEVICE_GET_INFO( ym2203 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(ym2203_state);					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( ym2203 );		break;
		case DEVINFO_FCT_STOP:							info->stop = DEVICE_STOP_NAME( ym2203 );		break;
		case DEVINFO_FCT_RESET:							info->reset = DEVICE_RESET_NAME( ym2203 );		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "YM2203");						break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Yamaha FM");					break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}


DEFINE_LEGACY_SOUND_DEVICE(YM2203, ym2203);
