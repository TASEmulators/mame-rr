/***************************************************************************

    Namco 53XX

    This instance of the Fujitsu MB8843 MCU is programmed to act as an I/O
    device. It is used by just two games: Dig Dug and Pole Position.

    MOD0-MOD2 = input mode
    CS0-CS3 = chip select lines used to select 1 of 4 input sources
    OUT0-OUT7 = 8-bit final output data
    P0.0-P0.3 = input port 0 data
    P1.0-P1.3 = input port 1 data
    P2.0-P2.3 = input port 2 data
    P3.0-P3.3 = input port 3 data

                   +------+
                 EX|1   42|Vcc
                  X|2   41|K3 (MOD2)
             /RESET|3   40|K2 (MOD1)
               /IRQ|4   39|K1 (MOD0)
                 SO|5   38|K0
                 SI|6   37|R15 (P3.3)
            /SC /TO|7   36|R14 (P3.2)
                /TC|8   35|R13 (P3.1)
           (CS0) P0|9   34|R12 (P3.0)
           (CS1) P1|10  33|R11 (P2.3)
           (CS2) P2|11  32|R10 (P2.2)
           (CS3) P3|12  31|R9 (P2.1)
          (OUT0) O0|13  30|R8 (P2.0)
          (OUT1) O1|14  29|R7 (P1.3)
          (OUT2) O2|15  28|R6 (P1.2)
          (OUT3) O3|16  27|R5 (P1.1)
          (OUT4) O4|17  26|R4 (P1.0)
          (OUT5) O5|18  25|R3 (P0.3)
          (OUT6) O6|19  24|R2 (P0.2)
          (OUT7) O7|20  23|R1 (P0.1)
                GND|21  22|R0 (P0.0)
                   +------+

    MOD selects one of 8 modes in which the input data is interpreted.

    Pole Position is hard-wired to use mode 0, which reads 4 steering
    inputs and 4 DIP switches (only 1 of each is used). The steering
    inputs are clocked on P0 and direction on P1, 1 bit per analog input.
    The DIP switches are connected to P2 and P3.

    Dig Dug can control which mode to use via the MOD bit latches. It sets
    these values to mode 7 when running.

    Unknowns:
        SO is connected to IOSEL on Pole Position

***************************************************************************/

#include "emu.h"
#include "namco53.h"
#include "cpu/mb88xx/mb88xx.h"


#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)


typedef struct _namco_53xx_state namco_53xx_state;
struct _namco_53xx_state
{
	running_device *	cpu;
	UINT8					portO;
	devcb_resolved_read8	k;
	devcb_resolved_read8	in[4];
	devcb_resolved_write8	p;
};

INLINE namco_53xx_state *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->type() == NAMCO_53XX);

	return (namco_53xx_state *)downcast<legacy_device_base *>(device)->token();
}



static READ8_HANDLER( namco_53xx_K_r )
{
	namco_53xx_state *state = get_safe_token(space->cpu->owner());
	return devcb_call_read8(&state->k, 0);
}

static READ8_HANDLER( namco_53xx_Rx_r )
{
	namco_53xx_state *state = get_safe_token(space->cpu->owner());
	return devcb_call_read8(&state->in[offset], 0);
}

static WRITE8_HANDLER( namco_53xx_O_w )
{
	namco_53xx_state *state = get_safe_token(space->cpu->owner());
	UINT8 out = (data & 0x0f);
	if (data & 0x10)
		state->portO = (state->portO & 0x0f) | (out << 4);
	else
		state->portO = (state->portO & 0xf0) | (out);
}

static WRITE8_HANDLER( namco_53xx_P_w )
{
	namco_53xx_state *state = get_safe_token(space->cpu->owner());
	devcb_call_write8(&state->p, 0, data);
}


static TIMER_CALLBACK( namco_53xx_irq_clear )
{
	namco_53xx_state *state = get_safe_token((running_device *)ptr);
	cpu_set_input_line(state->cpu, 0, CLEAR_LINE);
}

void namco_53xx_read_request(running_device *device)
{
	namco_53xx_state *state = get_safe_token(device);
	cpu_set_input_line(state->cpu, 0, ASSERT_LINE);

	// The execution time of one instruction is ~4us, so we must make sure to
	// give the cpu time to poll the /IRQ input before we clear it.
	// The input clock to the 06XX interface chip is 64H, that is
	// 18432000/6/64 = 48kHz, so it makes sense for the irq line to be
	// asserted for one clock cycle ~= 21us.
	timer_set(device->machine, ATTOTIME_IN_USEC(21), (void *)device, 0, namco_53xx_irq_clear);
}

READ8_DEVICE_HANDLER( namco_53xx_read )
{
	namco_53xx_state *state = get_safe_token(device);
	UINT8 res = state->portO;

	namco_53xx_read_request(device);

	return res;
}


/***************************************************************************
    DEVICE INTERFACE
***************************************************************************/

static ADDRESS_MAP_START( namco_53xx_map_io, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(MB88_PORTK,  MB88_PORTK)  AM_READ(namco_53xx_K_r)
	AM_RANGE(MB88_PORTO,  MB88_PORTO)  AM_WRITE(namco_53xx_O_w)
	AM_RANGE(MB88_PORTP,  MB88_PORTP)  AM_WRITE(namco_53xx_P_w)
	AM_RANGE(MB88_PORTR0, MB88_PORTR3) AM_READ(namco_53xx_Rx_r)
ADDRESS_MAP_END


static MACHINE_DRIVER_START( namco_53xx )
	MDRV_CPU_ADD("mcu", MB8843, DERIVED_CLOCK(1,1))		/* parent clock, internally divided by 6 */
	MDRV_CPU_IO_MAP(namco_53xx_map_io)
MACHINE_DRIVER_END


ROM_START( namco_53xx )
	ROM_REGION( 0x400, "mcu", ROMREGION_LOADBYNAME )
	ROM_LOAD( "53xx.bin",     0x0000, 0x0400, CRC(b326fecb) SHA1(758d8583d658e4f1df93184009d86c3eb8713899) )
ROM_END


/*-------------------------------------------------
    device start callback
-------------------------------------------------*/

static DEVICE_START( namco_53xx )
{
	const namco_53xx_interface *config = (const namco_53xx_interface *)device->baseconfig().static_config();
	namco_53xx_state *state = get_safe_token(device);
	astring tempstring;

	assert(config != NULL);

	/* find our CPU */
	state->cpu = device->subdevice("mcu");
	assert(state->cpu != NULL);

	/* resolve our read/write callbacks */
	devcb_resolve_read8(&state->k, &config->k, device);
	devcb_resolve_read8(&state->in[0], &config->in[0], device);
	devcb_resolve_read8(&state->in[1], &config->in[1], device);
	devcb_resolve_read8(&state->in[2], &config->in[2], device);
	devcb_resolve_read8(&state->in[3], &config->in[3], device);
	devcb_resolve_write8(&state->p, &config->p, device);

	state_save_register_device_item(device, 0, state->portO);
}


/*-------------------------------------------------
    device definition
-------------------------------------------------*/

static const char DEVTEMPLATE_SOURCE[] = __FILE__;

#define DEVTEMPLATE_ID(p,s)		p##namco_53xx##s
#define DEVTEMPLATE_FEATURES	DT_HAS_START | DT_HAS_ROM_REGION | DT_HAS_MACHINE_CONFIG
#define DEVTEMPLATE_NAME		"Namco 53xx"
#define DEVTEMPLATE_FAMILY		"Namco I/O"
#include "devtempl.h"


DEFINE_LEGACY_DEVICE(NAMCO_53XX, namco_53xx);
