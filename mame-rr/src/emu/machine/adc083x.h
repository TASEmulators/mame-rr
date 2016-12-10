/***************************************************************************

    National Semiconductor ADC0831 / ADC0832 / ADC0834 / ADC0838

    8-Bit serial I/O A/D Converters with Muliplexer Options

***************************************************************************/

#ifndef __ADC083X_H__
#define __ADC083X_H__

#include "devlegcy.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define ADC083X_CH0		0
#define ADC083X_CH1		1
#define ADC083X_CH2		2
#define ADC083X_CH3		3
#define ADC083X_CH4		4
#define ADC083X_CH5		5
#define ADC083X_CH6		6
#define ADC083X_CH7		7
#define ADC083X_COM		8
#define ADC083X_AGND	9
#define ADC083X_VREF	10

/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

DECLARE_LEGACY_DEVICE(ADC0831, adc0831);
DECLARE_LEGACY_DEVICE(ADC0832, adc0832);
DECLARE_LEGACY_DEVICE(ADC0834, adc0834);
DECLARE_LEGACY_DEVICE(ADC0838, adc0838);

#define MDRV_ADC0831_ADD(_tag, _config) \
	MDRV_DEVICE_ADD(_tag, ADC0831, 0) \
	MDRV_DEVICE_CONFIG(_config)

#define MDRV_ADC0832_ADD(_tag, _config) \
	MDRV_DEVICE_ADD(_tag, ADC0832, 0) \
	MDRV_DEVICE_CONFIG(_config)

#define MDRV_ADC0834_ADD(_tag, _config) \
	MDRV_DEVICE_ADD(_tag, ADC0834, 0) \
	MDRV_DEVICE_CONFIG(_config)

#define MDRV_ADC0838_ADD(_tag, _config) \
	MDRV_DEVICE_ADD(_tag, ADC0838, 0) \
	MDRV_DEVICE_CONFIG(_config)


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef double (*adc083x_input_convert_func)(running_device *device, UINT8 input);

typedef struct _adc083x_interface adc083x_interface;
struct _adc083x_interface
{
	adc083x_input_convert_func input_callback_r;
};


/***************************************************************************
    PROTOTYPES
***************************************************************************/

extern WRITE_LINE_DEVICE_HANDLER( adc083x_cs_write );
extern WRITE_LINE_DEVICE_HANDLER( adc083x_clk_write );
extern WRITE_LINE_DEVICE_HANDLER( adc083x_di_write );
extern WRITE_LINE_DEVICE_HANDLER( adc083x_se_write );
extern READ_LINE_DEVICE_HANDLER( adc083x_sars_read );
extern READ_LINE_DEVICE_HANDLER( adc083x_do_read );

#endif	/* __ADC083X_H__ */
