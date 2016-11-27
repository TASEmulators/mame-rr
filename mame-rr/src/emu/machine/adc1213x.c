/***************************************************************************

    National Semiconductor ADC12130 / ADC12132 / ADC12138

    Self-calibrating 12-bit Plus Sign Serial I/O A/D Converters with MUX
        and Sample/Hold

    TODO:
        - Only ADC12138 currently supported

    2009-06 Converted to be a device

***************************************************************************/

#include "emu.h"
#include "adc1213x.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _adc12138_state adc12138_state;
struct _adc12138_state
{
	adc1213x_input_convert_func input_callback_r;

	int cycle;
	int data_out;
	int data_in;
	int conv_mode;
	int auto_cal;
	int auto_zero;
	int acq_time;
	int data_out_sign;
	int mode;
	int input_shift_reg;
	int output_shift_reg;
	int end_conv;
};


#define ADC1213X_CONV_MODE_12_MSB_FIRST			0
#define ADC1213X_CONV_MODE_16_MSB_FIRST			1
#define ADC1213X_CONV_MODE_12_LSB_FIRST			2
#define ADC1213X_CONV_MODE_16_LSB_FIRST			3

#define ADC1213X_ACQUISITION_TIME_6_CCLK		0
#define ADC1213X_ACQUISITION_TIME_10_CCLK		1
#define ADC1213X_ACQUISITION_TIME_18_CCLK		2
#define ADC1213X_ACQUISITION_TIME_34_CCLK		3

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE adc12138_state *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert((device->type() == ADC12130) || (device->type() == ADC12132) || (device->type() == ADC12138));
	return (adc12138_state *)downcast<legacy_device_base *>(device)->token();
}

INLINE const adc12138_interface *get_interface(running_device *device)
{
	assert(device != NULL);
	assert((device->type() == ADC12130) || (device->type() == ADC12132) || (device->type() == ADC12138));
	return (const adc12138_interface *) device->baseconfig().static_config();
}


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    adc1213x_di_w
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( adc1213x_di_w )
{
	adc12138_state *adc1213x = get_safe_token(device);
	adc1213x->data_in = data & 1;
}

/*-------------------------------------------------
    adc1213x_convert
-------------------------------------------------*/

static void adc1213x_convert(running_device *device, int channel, int bits16, int lsbfirst)
{
	adc12138_state *adc1213x = get_safe_token(device);
	int i;
	int bits;
	int input_value;
	double input = 0;

	if (bits16)
		fatalerror("ADC1213X: 16-bit mode not supported\n");

	if (lsbfirst)
		fatalerror("ADC1213X: LSB first not supported\n");

	switch (channel)
	{
		case 0x8:		// H L L L - CH0 (single-ended)
		{
			input = adc1213x->input_callback_r(device, 0);
			break;
		}
		case 0xc:		// H H L L - CH1 (single-ended)
		{
			input = adc1213x->input_callback_r(device, 1);
			break;
		}
		case 0x9:		// H L L H - CH2 (single-ended)
		{
			input = adc1213x->input_callback_r(device, 2);
			break;
		}
		case 0xd:		// H H L H - CH3 (single-ended)
		{
			input = adc1213x->input_callback_r(device, 3);
			break;
		}
		case 0xa:		// H L H L - CH4 (single-ended)
		{
			input = adc1213x->input_callback_r(device, 4);
			break;
		}
		case 0xe:		// H H H L - CH5 (single-ended)
		{
			input = adc1213x->input_callback_r(device, 5);
			break;
		}
		case 0xb:		// H L H H - CH6 (single-ended)
		{
			input = adc1213x->input_callback_r(device, 6);
			break;
		}
		case 0xf:		// H H H H - CH7 (single-ended)
		{
			input = adc1213x->input_callback_r(device, 7);
			break;
		}
		default:
		{
			fatalerror("ADC1213X: unsupported channel %02X\n", channel);
		}
	}

	input_value = (int)(input * 2047.0);

	bits = 12;

	// sign-extend if needed
	if (adc1213x->data_out_sign)
	{
		input_value = input_value | ((input_value & 0x800) << 1);
		bits++;
	}

	adc1213x->output_shift_reg = 0;

	for (i=0; i < bits; i++)
	{
		if (input_value & (1 << ((bits-1) - i)))
		{
			adc1213x->output_shift_reg |= (1 << i);
		}
	}

	adc1213x->data_out = adc1213x->output_shift_reg & 1;
	adc1213x->output_shift_reg >>= 1;
}

/*-------------------------------------------------
    adc1213x_cs_w
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( adc1213x_cs_w )
{
	adc12138_state *adc1213x = get_safe_token(device);

	if (data)
	{
		//printf("ADC: CS\n");

		if (adc1213x->cycle >= 7)
		{
			int mode = adc1213x->input_shift_reg >> (adc1213x->cycle - 8);

			switch (mode & 0xf)
			{
				case 0x0:		// X X X X L L L L - 12 or 13 Bit MSB First conversion
				{
					adc1213x_convert(device, (mode >> 4) & 0xf, 0, 0);
					break;
				}
				case 0x1:		// X X X X L L L H - 16 or 17 Bit MSB First conversion
				{
					adc1213x_convert(device, (mode >> 4) & 0xf, 1, 0);
					break;
				}
				case 0x4:		// X X X X L H L L - 12 or 13 Bit LSB First conversion
				{
					adc1213x_convert(device, (mode >> 4) & 0xf, 0, 1);
					break;
				}
				case 0x5:		// X X X X L H L H - 16 or 17 Bit LSB First conversion
				{
					adc1213x_convert(device, (mode >> 4) & 0xf, 1, 1);
					break;
				}

				default:
				{
					switch (mode)
					{
						case 0x08:		// L L L L H L L L - Auto cal
						{
							adc1213x->auto_cal = 1;
							break;
						}

						case 0x0e:		// L L L L H H H L - Acquisition time 6 CCLK cycles
						{
							adc1213x->acq_time = ADC1213X_ACQUISITION_TIME_6_CCLK;
							break;
						}

						case 0x8d:		// H L L L H H L H - Data out with sign
						{
							adc1213x->data_out_sign = 1;
							break;
						}

						default:
						{
							fatalerror("ADC1213X: unknown config mode %02X\n", mode);
						}
					}
					break;
				}
			}
		}

		adc1213x->cycle = 0;
		adc1213x->input_shift_reg = 0;

		adc1213x->end_conv = 0;
	}
}

/*-------------------------------------------------
    adc1213x_sclk_w
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( adc1213x_sclk_w )
{
	adc12138_state *adc1213x = get_safe_token(device);

	if (data)
	{
		//printf("ADC: cycle %d, DI = %d\n", adc1213x->cycle, adc1213x->data_in);

		adc1213x->input_shift_reg <<= 1;
		adc1213x->input_shift_reg |= adc1213x->data_in;

		adc1213x->data_out = adc1213x->output_shift_reg & 1;
		adc1213x->output_shift_reg >>= 1;

		adc1213x->cycle++;
	}
}

/*-------------------------------------------------
    adc1213x_conv_w
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( adc1213x_conv_w )
{
	adc12138_state *adc1213x = get_safe_token(device);
	adc1213x->end_conv = 1;
}

/*-------------------------------------------------
    adc1213x_do_r
-------------------------------------------------*/

READ8_DEVICE_HANDLER( adc1213x_do_r )
{
	adc12138_state *adc1213x = get_safe_token(device);

	//printf("ADC: DO\n");
	return adc1213x->data_out;
}

/*-------------------------------------------------
    adc1213x_eoc_r
-------------------------------------------------*/

READ8_DEVICE_HANDLER( adc1213x_eoc_r )
{
	adc12138_state *adc1213x = get_safe_token(device);
	return adc1213x->end_conv;
}

/*-------------------------------------------------
    DEVICE_START( adc1213x )
-------------------------------------------------*/

static DEVICE_START( adc12138 )
{
	adc12138_state *adc1213x = get_safe_token(device);
	const adc12138_interface *intf = get_interface(device);

	/* resolve callbacks */
	adc1213x->input_callback_r = intf->input_callback_r;

	/* register for state saving */
	state_save_register_device_item(device, 0, adc1213x->cycle);
	state_save_register_device_item(device, 0, adc1213x->data_out);
	state_save_register_device_item(device, 0, adc1213x->data_in);
	state_save_register_device_item(device, 0, adc1213x->conv_mode);
	state_save_register_device_item(device, 0, adc1213x->auto_cal);
	state_save_register_device_item(device, 0, adc1213x->auto_zero);
	state_save_register_device_item(device, 0, adc1213x->acq_time);
	state_save_register_device_item(device, 0, adc1213x->data_out_sign);
	state_save_register_device_item(device, 0, adc1213x->mode);
	state_save_register_device_item(device, 0, adc1213x->input_shift_reg);
	state_save_register_device_item(device, 0, adc1213x->output_shift_reg);
	state_save_register_device_item(device, 0, adc1213x->end_conv);
}


/*-------------------------------------------------
    DEVICE_RESET( adc1213x )
-------------------------------------------------*/

static DEVICE_RESET( adc12138 )
{
	adc12138_state *adc1213x = get_safe_token(device);

	adc1213x->conv_mode = ADC1213X_CONV_MODE_12_MSB_FIRST;
	adc1213x->data_out_sign = 1;
	adc1213x->auto_cal = 0;
	adc1213x->auto_zero = 0;
	adc1213x->acq_time = ADC1213X_ACQUISITION_TIME_10_CCLK;
}

/*-------------------------------------------------
    device definition
-------------------------------------------------*/

static const char DEVTEMPLATE_SOURCE[] = __FILE__;

#define DEVTEMPLATE_ID(p,s)		p##adc12138##s
#define DEVTEMPLATE_FEATURES	DT_HAS_START | DT_HAS_RESET
#define DEVTEMPLATE_NAME		"A/D Converter 12138"
#define DEVTEMPLATE_FAMILY		"National Semiconductor A/D Converters 1213x"
#include "devtempl.h"

#define DEVTEMPLATE_DERIVED_ID(p,s)		p##adc12130##s
#define DEVTEMPLATE_DERIVED_FEATURES	0
#define DEVTEMPLATE_DERIVED_NAME		"A/D Converter 12130"
#include "devtempl.h"

#define DEVTEMPLATE_DERIVED_ID(p,s)		p##adc12132##s
#define DEVTEMPLATE_DERIVED_FEATURES	0
#define DEVTEMPLATE_DERIVED_NAME		"A/D Converter 12132"
#include "devtempl.h"


DEFINE_LEGACY_DEVICE(ADC12130, adc12130);
DEFINE_LEGACY_DEVICE(ADC12132, adc12132);
DEFINE_LEGACY_DEVICE(ADC12138, adc12138);
