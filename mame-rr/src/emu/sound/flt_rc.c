#include "emu.h"
#include "streams.h"
#include "flt_rc.h"

typedef struct _filter_rc_state filter_rc_state;
struct _filter_rc_state
{
	running_device *device;
	sound_stream *	stream;
	int				k;
	int				memory;
	int				type;
};

INLINE filter_rc_state *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->type() == SOUND_FILTER_RC);
	return (filter_rc_state *)downcast<legacy_device_base *>(device)->token();
}

const flt_rc_config flt_rc_ac_default = {FLT_RC_AC, 10000, 0, 0, CAP_U(1)};


static STREAM_UPDATE( filter_rc_update )
{
	stream_sample_t *src = inputs[0];
	stream_sample_t *dst = outputs[0];
	filter_rc_state *info = (filter_rc_state *)param;
	int memory = info->memory;

	switch (info->type)
	{
		case FLT_RC_LOWPASS:
			while (samples--)
			{
				memory += ((*src++ - memory) * info->k) / 0x10000;
				*dst++ = memory;
			}
			break;
		case FLT_RC_HIGHPASS:
		case FLT_RC_AC:
			while (samples--)
			{
				*dst++ = *src - memory;
				memory += ((*src++ - memory) * info->k) / 0x10000;
			}
			break;
	}
	info->memory = memory;
}

static void set_RC_info(filter_rc_state *info, int type, double R1, double R2, double R3, double C)
{
	double Req;

	info->type = type;

	switch (info->type)
	{
		case FLT_RC_LOWPASS:
			if (C == 0.0)
			{
				/* filter disabled */
				info->k = 0x10000;
				return;
			}
			Req = (R1 * (R2 + R3)) / (R1 + R2 + R3);
			break;
		case FLT_RC_HIGHPASS:
		case FLT_RC_AC:
			if (C == 0.0)
			{
				/* filter disabled */
				info->k = 0x0;
				info->memory = 0x0;
				return;
			}
			Req = R1;
			break;
		default:
			fatalerror("filter_rc_setRC: Wrong filter type %d\n", info->type);
	}

	/* Cut Frequency = 1/(2*Pi*Req*C) */
	/* k = (1-(EXP(-TIMEDELTA/RC)))    */
	info->k = 0x10000 - 0x10000 * (exp(-1 / (Req * C) / info->device->machine->sample_rate));
}


static DEVICE_START( filter_rc )
{
	filter_rc_state *info = get_safe_token(device);
	const flt_rc_config *conf = (const flt_rc_config *)device->baseconfig().static_config();

	info->device = device;
	info->stream = stream_create(device, 1, 1, device->machine->sample_rate, info, filter_rc_update);
	if (conf)
		set_RC_info(info, conf->type, conf->R1, conf->R2, conf->R3, conf->C);
	else
		set_RC_info(info, FLT_RC_LOWPASS, 1, 1, 1, 0);
}


void filter_rc_set_RC(running_device *device, int type, double R1, double R2, double R3, double C)
{
	filter_rc_state *info = get_safe_token(device);

	stream_update(info->stream);

	set_RC_info(info, type, R1, R2, R3, C);

}

/**************************************************************************
 * Generic get_info
 **************************************************************************/

DEVICE_GET_INFO( filter_rc )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(filter_rc_state);				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( filter_rc );	break;
		case DEVINFO_FCT_STOP:							/* Nothing */									break;
		case DEVINFO_FCT_RESET:							/* Nothing */									break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "RC Filter");					break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Filters");						break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}


DEFINE_LEGACY_SOUND_DEVICE(FILTER_RC, filter_rc);
