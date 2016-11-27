/***************************************************************************

    RP5H01


    2009-06 Converted to be a device

***************************************************************************/

#include "emu.h"
#include "machine/rp5h01.h"


/***************************************************************************
    PARAMETERS
***************************************************************************/

/* these also work as the address masks */
enum {
	COUNTER_MODE_6_BITS = 0x3f,
	COUNTER_MODE_7_BITS = 0x7f
};


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _rp5h01_state rp5h01_state;
struct _rp5h01_state
{
	int counter;
	int counter_mode;	/* test pin */
	int enabled;		/* chip enable */
	int old_reset;		/* reset pin state (level-triggered) */
	int old_clock;		/* clock pin state (level-triggered) */
	UINT8 *data;
};

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE rp5h01_state *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert((device->type() == RP5H01));
	return (rp5h01_state *)downcast<legacy_device_base *>(device)->token();
}

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    rp5h01_enable_w
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( rp5h01_enable_w )
{
	rp5h01_state *rp5h01 = get_safe_token(device);

	/* process the /CE signal and enable/disable the IC */
	rp5h01->enabled = (data == 0) ? 1 : 0;
}

/*-------------------------------------------------
    rp5h01_reset_w
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( rp5h01_reset_w )
{
	rp5h01_state *rp5h01 = get_safe_token(device);
	int newstate = (data == 0) ? 0 : 1;

	/* if it's not enabled, ignore */
	if (!rp5h01->enabled)
		return;

	/* now look for a 0->1 transition */
	if (rp5h01->old_reset == 0 && newstate == 1)
	{
		/* reset the counter */
		rp5h01->counter = 0;
	}

	/* update the pin */
	rp5h01->old_reset = newstate;
}

/*-------------------------------------------------
    rp5h01_clock_w
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( rp5h01_clock_w )
{
	rp5h01_state *rp5h01 = get_safe_token(device);
	int newstate = (data == 0) ? 0 : 1;

	/* if it's not enabled, ignore */
	if (!rp5h01->enabled)
		return;

	/* now look for a 1->0 transition */
	if (rp5h01->old_clock == 1 && newstate == 0)
	{
		/* increment the counter, and mask it with the mode */
		rp5h01->counter++;
	}

	/* update the pin */
	rp5h01->old_clock = newstate;
}

/*-------------------------------------------------
    rp5h01_test_w
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( rp5h01_test_w )
{
	rp5h01_state *rp5h01 = get_safe_token(device);

	/* if it's not enabled, ignore */
	if (!rp5h01->enabled)
		return;

	/* process the test signal and change the counter mode */
	rp5h01->counter_mode = (data == 0) ? COUNTER_MODE_6_BITS : COUNTER_MODE_7_BITS;
}

/*-------------------------------------------------
    rp5h01_counter_r
-------------------------------------------------*/

READ8_DEVICE_HANDLER( rp5h01_counter_r )
{
	rp5h01_state *rp5h01 = get_safe_token(device);

	/* if it's not enabled, ignore */
	if (!rp5h01->enabled)
		return 0; /* ? (should be high impedance) */

	/* return A5 */
	return (rp5h01->counter >> 5) & 1;
}

/*-------------------------------------------------
    rp5h01_data_r
-------------------------------------------------*/

READ8_DEVICE_HANDLER( rp5h01_data_r )
{
	rp5h01_state *rp5h01 = get_safe_token(device);
	int byte, bit;

	/* if it's not enabled, ignore */
	if (!rp5h01->enabled)
		return 0; /* ? (should be high impedance) */

	/* get the byte offset and bit offset */
	byte = (rp5h01->counter & rp5h01->counter_mode) >> 3;
	bit = 7 - (rp5h01->counter & 7);

	/* return the data */
	return (rp5h01->data[byte] >> bit) & 1;
}

/*-------------------------------------------------
    DEVICE_START( rp5h01 )
-------------------------------------------------*/

static DEVICE_START( rp5h01 )
{
	rp5h01_state *rp5h01 = get_safe_token(device);

	assert(device->baseconfig().static_config() == NULL);

	rp5h01->data = *device->region();

	/* register for state saving */
	state_save_register_device_item(device, 0, rp5h01->counter);
	state_save_register_device_item(device, 0, rp5h01->counter_mode);
	state_save_register_device_item(device, 0, rp5h01->enabled);
	state_save_register_device_item(device, 0, rp5h01->old_reset);
	state_save_register_device_item(device, 0, rp5h01->old_clock);
}

/*-------------------------------------------------
    DEVICE_RESET( rp5h01 )
-------------------------------------------------*/

static DEVICE_RESET( rp5h01 )
{
	rp5h01_state *rp5h01 = get_safe_token(device);

	rp5h01->counter = 0;
	rp5h01->counter_mode = COUNTER_MODE_6_BITS;
	rp5h01->enabled = 0;
	rp5h01->old_reset = -1;
	rp5h01->old_clock = -1;
}

/*-------------------------------------------------
    device definition
-------------------------------------------------*/

static const char DEVTEMPLATE_SOURCE[] = __FILE__;

#define DEVTEMPLATE_ID(p,s)		p##rp5h01##s
#define DEVTEMPLATE_FEATURES	DT_HAS_START | DT_HAS_RESET
#define DEVTEMPLATE_NAME		"RP5H01"
#define DEVTEMPLATE_FAMILY		"RP5H01"
#include "devtempl.h"


DEFINE_LEGACY_DEVICE(RP5H01, rp5h01);
