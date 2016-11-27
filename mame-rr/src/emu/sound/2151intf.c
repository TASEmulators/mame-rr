/***************************************************************************

  2151intf.c

  Support interface YM2151(OPM)

***************************************************************************/

#include "emu.h"
#include "streams.h"
#include "fm.h"
#include "2151intf.h"
#include "ym2151.h"


typedef struct _ym2151_state ym2151_state;
struct _ym2151_state
{
	sound_stream *			stream;
	emu_timer *				timer[2];
	void *					chip;
	UINT8					lastreg;
	const ym2151_interface *intf;
};


INLINE ym2151_state *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->type() == SOUND_YM2151);
	return (ym2151_state *)downcast<legacy_device_base *>(device)->token();
}


static STREAM_UPDATE( ym2151_update )
{
	ym2151_state *info = (ym2151_state *)param;
	ym2151_update_one(info->chip, outputs, samples);
}


static STATE_POSTLOAD( ym2151intf_postload )
{
	ym2151_state *info = (ym2151_state *)param;
	ym2151_postload(machine, info->chip);
}


static DEVICE_START( ym2151 )
{
	static const ym2151_interface dummy = { 0 };
	ym2151_state *info = get_safe_token(device);
	int rate;

	info->intf = device->baseconfig().static_config() ? (const ym2151_interface *)device->baseconfig().static_config() : &dummy;

	rate = device->clock()/64;

	/* stream setup */
	info->stream = stream_create(device,0,2,rate,info,ym2151_update);

	info->chip = ym2151_init(device,device->clock(),rate);
	assert_always(info->chip != NULL, "Error creating YM2151 chip");

	state_save_register_postload(device->machine, ym2151intf_postload, info);

	ym2151_set_irq_handler(info->chip,info->intf->irqhandler);
	ym2151_set_port_write_handler(info->chip,info->intf->portwritehandler);
}


static DEVICE_STOP( ym2151 )
{
	ym2151_state *info = get_safe_token(device);
	ym2151_shutdown(info->chip);
}

static DEVICE_RESET( ym2151 )
{
	ym2151_state *info = get_safe_token(device);
	ym2151_reset_chip(info->chip);
}


READ8_DEVICE_HANDLER( ym2151_r )
{
	ym2151_state *token = get_safe_token(device);

	if (offset & 1)
	{
		stream_update(token->stream);
		return ym2151_read_status(token->chip);
	}
	else
		return 0xff;	/* confirmed on a real YM2151 */
}

WRITE8_DEVICE_HANDLER( ym2151_w )
{
	ym2151_state *token = get_safe_token(device);

	if (offset & 1)
	{
		stream_update(token->stream);
		ym2151_write_reg(token->chip, token->lastreg, data);
	}
	else
		token->lastreg = data;
}


READ8_DEVICE_HANDLER( ym2151_status_port_r ) { return ym2151_r(device, 1); }

WRITE8_DEVICE_HANDLER( ym2151_register_port_w ) { ym2151_w(device, 0, data); }
WRITE8_DEVICE_HANDLER( ym2151_data_port_w ) { ym2151_w(device, 1, data); }



/**************************************************************************
 * Generic get_info
 **************************************************************************/

DEVICE_GET_INFO( ym2151 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(ym2151_state);					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( ym2151 );		break;
		case DEVINFO_FCT_STOP:							info->stop = DEVICE_STOP_NAME( ym2151 );		break;
		case DEVINFO_FCT_RESET:							info->reset = DEVICE_RESET_NAME( ym2151 );		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "YM2151");						break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Yamaha FM");					break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}


DEFINE_LEGACY_SOUND_DEVICE(YM2151, ym2151);
