/***************************************************************************

    Namco 06XX

    This chip is used as an interface to up to 4 other custom chips.
    It signals IRQs to the custom MCUs when writes happen, and generates
    NMIs to the controlling CPU to drive reads based on a clock.

    SD0-SD7 are data I/O lines connecting to the controlling CPU
    SEL selects either control (1) or data (0), usually connected to
        an address line of the controlling CPU
    /NMI is an NMI signal line for the controlling CPU

    ID0-ID7 are data I/O lines connecting to the other custom chips
    /IO1-/IO4 are IRQ signal lines for each custom chip

                   +------+
                [1]|1   28|Vcc
                ID7|2   27|SD7
                ID6|3   26|SD6
                ID5|4   25|SD5
                ID4|5   24|SD4
                ID3|6   23|SD3
                ID2|7   22|SD2
                ID1|8   21|SD1
                ID0|9   20|SD0
               /IO1|10  19|/NMI
               /IO2|11  18|/CS
               /IO3|12  17|CLOCK
               /IO4|13  16|R/W
                GND|14  15|SEL
                   +------+

    [1] on polepos, galaga, xevious, and bosco: connected to K3 of the 51xx
        on bosco and xevious, connected to R8 of the 50xx


    06XX interface:
    ---------------
    Galaga                  51XX  ----  ----  54XX
    Bosconian (CPU board)   51XX  ----  50XX  54XX
    Bosconian (Video board) 50XX  52XX  ----  ----
    Xevious                 51XX  ----  50XX  54XX
    Dig Dug                 51XX  53XX  ----  ----
    Pole Position / PP II   51XX  53XX  52XX  54XX


    Galaga writes:
        control = 10(000), data = FF at startup
        control = 71(011), read 3, control = 10
        control = A1(101), write 4, control = 10
        control = A8(101), write 12, control = 10

    Xevious writes:
        control = 10 at startup
        control = A1(101), write 6, control = 10
        control = 71(011), read 3, control = 10
        control = 64(011), write 1, control = 10
        control = 74(011), read 4, control = 10
        control = 68(011), write 7, control = 10

    Dig Dug writes:
        control = 10(000), data = 10 at startup
        control = A1(101), write 3, control = 10
        control = 71(011), read 3, control = 10
        control = D2(110), read 2, control = 10

    Bosco writes:
        control = 10(000), data = FF at startup
        control = C8(110), write 17, control = 10
        control = 61(011), write 1, control = 10
        control = 71(011), read 3, control = 10
        control = 94(100), read 4, control = 10
        control = 64(011), write 1, control = 10
        control = 84(100), write 5, control = 10


        control = 34(001), write 1, control = 10

***************************************************************************/

#include "emu.h"
#include "machine/namco06.h"
#include "machine/namco50.h"
#include "machine/namco51.h"
#include "machine/namco53.h"
#include "audio/namco52.h"
#include "audio/namco54.h"


#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)



typedef struct _namco_06xx_state namco_06xx_state;
struct _namco_06xx_state
{
	UINT8 control;
	emu_timer *nmi_timer;
	cpu_device *nmicpu;
	device_t *device[4];
	read8_device_func read[4];
	void (*readreq[4])(running_device *device);
	write8_device_func write[4];
};

INLINE namco_06xx_state *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->type() == NAMCO_06XX);

	return (namco_06xx_state *)downcast<legacy_device_base *>(device)->token();
}



static TIMER_CALLBACK( nmi_generate )
{
	namco_06xx_state *state = get_safe_token((running_device *)ptr);

	if (!state->nmicpu->suspended(SUSPEND_REASON_HALT | SUSPEND_REASON_RESET | SUSPEND_REASON_DISABLE))
	{
		LOG(("NMI cpu '%s'\n",state->nmicpu->tag()));

		cpu_set_input_line(state->nmicpu, INPUT_LINE_NMI, PULSE_LINE);
	}
	else
		LOG(("NMI not generated because cpu '%s' is suspended\n",state->nmicpu->tag()));
}


READ8_DEVICE_HANDLER( namco_06xx_data_r )
{
	namco_06xx_state *state = get_safe_token(device);
	UINT8 result = 0xff;
	int devnum;

	LOG(("%s: 06XX '%s' read offset %d\n",cpuexec_describe_context(device->machine),device->tag(),offset));

	if (!(state->control & 0x10))
	{
		logerror("%s: 06XX '%s' read in write mode %02x\n",cpuexec_describe_context(device->machine),device->tag(),state->control);
		return 0;
	}

	for (devnum = 0; devnum < 4; devnum++)
		if ((state->control & (1 << devnum)) && state->read[devnum] != NULL)
			result &= (*state->read[devnum])(state->device[devnum], 0);

	return result;
}


WRITE8_DEVICE_HANDLER( namco_06xx_data_w )
{
	namco_06xx_state *state = get_safe_token(device);
	int devnum;

	LOG(("%s: 06XX '%s' write offset %d = %02x\n",cpuexec_describe_context(device->machine),device->tag(),offset,data));

	if (state->control & 0x10)
	{
		logerror("%s: 06XX '%s' write in read mode %02x\n",cpuexec_describe_context(device->machine),device->tag(),state->control);
		return;
	}

	for (devnum = 0; devnum < 4; devnum++)
		if ((state->control & (1 << devnum)) && state->write[devnum] != NULL)
			(*state->write[devnum])(state->device[devnum], 0, data);
}


READ8_DEVICE_HANDLER( namco_06xx_ctrl_r )
{
	namco_06xx_state *state = get_safe_token(device);
	LOG(("%s: 06XX '%s' ctrl_r\n",cpuexec_describe_context(device->machine),device->tag()));
	return state->control;
}

WRITE8_DEVICE_HANDLER( namco_06xx_ctrl_w )
{
	namco_06xx_state *state = get_safe_token(device);
	int devnum;

	LOG(("%s: 06XX '%s' control %02x\n",cpuexec_describe_context(device->machine),device->tag(),data));

	state->control = data;

	if ((state->control & 0x0f) == 0)
	{
		LOG(("disabling nmi generate timer\n"));
		timer_adjust_oneshot(state->nmi_timer, attotime_never, 0);
	}
	else
	{
		LOG(("setting nmi generate timer to 200us\n"));

		// this timing is critical. Due to a bug, Bosconian will stop responding to
		// inputs if a transfer terminates at the wrong time.
		// On the other hand, the time cannot be too short otherwise the 54XX will
		// not have enough time to process the incoming controls.
		timer_adjust_periodic(state->nmi_timer, ATTOTIME_IN_USEC(200), 0, ATTOTIME_IN_USEC(200));

		if (state->control & 0x10)
			for (devnum = 0; devnum < 4; devnum++)
				if ((state->control & (1 << devnum)) && state->readreq[devnum] != NULL)
					(*state->readreq[devnum])(state->device[devnum]);
	}
}


/***************************************************************************
    DEVICE INTERFACE
***************************************************************************/

/*-------------------------------------------------
    device start callback
-------------------------------------------------*/

static DEVICE_START( namco_06xx )
{
	const namco_06xx_config *config = (const namco_06xx_config *)downcast<const legacy_device_config_base &>(device->baseconfig()).inline_config();
	namco_06xx_state *state = get_safe_token(device);
	int devnum;

	assert(config != NULL);

	/* resolve our CPU */
	state->nmicpu = device->machine->device<cpu_device>(config->nmicpu);
	assert(state->nmicpu != NULL);

	/* resolve our devices */
	state->device[0] = (config->chip0 != NULL) ? device->machine->device(config->chip0) : NULL;
	assert(state->device[0] != NULL || config->chip0 == NULL);
	state->device[1] = (config->chip1 != NULL) ? device->machine->device(config->chip1) : NULL;
	assert(state->device[1] != NULL || config->chip1 == NULL);
	state->device[2] = (config->chip2 != NULL) ? device->machine->device(config->chip2) : NULL;
	assert(state->device[2] != NULL || config->chip2 == NULL);
	state->device[3] = (config->chip3 != NULL) ? device->machine->device(config->chip3) : NULL;
	assert(state->device[3] != NULL || config->chip3 == NULL);

	/* loop over devices and set their read/write handlers */
	for (devnum = 0; devnum < 4; devnum++)
		if (state->device[devnum] != NULL)
		{
			device_type type = state->device[devnum]->type();

			if (type == NAMCO_50XX)
			{
				state->read[devnum] = namco_50xx_read;
				state->readreq[devnum] = namco_50xx_read_request;
				state->write[devnum] = namco_50xx_write;
			}
			else if (type == NAMCO_51XX)
			{
				state->read[devnum] = namco_51xx_read;
				state->write[devnum] = namco_51xx_write;
			}
			else if (type == NAMCO_52XX)
				state->write[devnum] = namco_52xx_write;
			else if (type == NAMCO_53XX)
			{
				state->read[devnum] = namco_53xx_read;
				state->readreq[devnum] = namco_53xx_read_request;
			}
			else if (type == NAMCO_54XX)
				state->write[devnum] = namco_54xx_write;
			else
				fatalerror("Unknown device type %s connected to Namco 06xx", state->device[devnum]->name());
		}

	/* allocate a timer */
	state->nmi_timer = timer_alloc(device->machine, nmi_generate, (void *)device);

	state_save_register_device_item(device, 0, state->control);
}


/*-------------------------------------------------
    device reset callback
-------------------------------------------------*/

static DEVICE_RESET( namco_06xx )
{
	namco_06xx_state *state = get_safe_token(device);
	state->control = 0;
}


/*-------------------------------------------------
    device definition
-------------------------------------------------*/

static const char DEVTEMPLATE_SOURCE[] = __FILE__;

#define DEVTEMPLATE_ID(p,s)		p##namco_06xx##s
#define DEVTEMPLATE_FEATURES	DT_HAS_START | DT_HAS_RESET | DT_HAS_INLINE_CONFIG
#define DEVTEMPLATE_NAME		"Namco 06xx"
#define DEVTEMPLATE_FAMILY		"Namco I/O"
#include "devtempl.h"


DEFINE_LEGACY_DEVICE(NAMCO_06XX, namco_06xx);
