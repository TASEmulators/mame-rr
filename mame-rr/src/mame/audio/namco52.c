/***************************************************************************

    Namco 52XX

    This instance of the Fujitsu MB8843 MCU is programmed to act as a
    sample player. It is used by just two games: Bosconian and Pole
    Position.

    A0-A15 = address to read from sample ROMs
    D0-D7 = data from sample ROMs
    CMD0-CMD3 = command from CPU (sample to play, 0 = none)
    OUT0-OUT3 = sound output

                  +------+
                EX|1   42|Vcc
                 X|2   41|K3 (CMD3)
            /RESET|3   40|K2 (CMD2)
              /IRQ|4   39|K1 (CMD1)
         (n.c.) SO|5   38|K0 (CMD0)
            [1] SI|6   37|R15 (A7)
     (n.c.) /SC/TO|7   36|R14 (A6)
           [2] /TC|8   35|R13 (A5)
         (OUT0) P0|9   34|R12 (A4)
         (OUT1) P1|10  33|R11 (A3)
         (OUT2) P2|11  32|R10 (A2)
         (OUT3) P3|12  31|R9 (A1)
           (A8) O0|13  30|R8 (A0)
           (A9) O1|14  29|R7 (D7)
          (A10) O2|15  28|R6 (D6)
          (A11) O3|16  27|R5 (D5)
          (A12) O4|17  26|R4 (D4)
          (A13) O5|18  25|R3 (D3)
          (A14) O6|19  24|R2 (D2)
          (A15) O7|20  23|R1 (D1)
               GND|21  22|R0 (D0)
                  +------+

    [1] in polepos, +5V; in bosco, GND
        this value controls the ROM addressing mode:
           if 0 (GND), A12-A15 are direct active-low chip enables
           if 1 (Vcc), A12-A15 are address lines

    [2] in polepos, GND; in bosco, output from a 555 timer
        this value is an external timer, which is used for some samples

***************************************************************************/

#include "emu.h"
#include "namco52.h"
#include "cpu/mb88xx/mb88xx.h"

typedef struct _namco_52xx_state namco_52xx_state;
struct _namco_52xx_state
{
	running_device *cpu;
	running_device *discrete;
	int basenode;
	devcb_resolved_read8 romread;
	devcb_resolved_read8 si;
	UINT8 latched_cmd;
	UINT32 address;
};

INLINE namco_52xx_state *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->type() == NAMCO_52XX);

	return (namco_52xx_state *)downcast<legacy_device_base *>(device)->token();
}



static TIMER_CALLBACK( namco_52xx_latch_callback )
{
	namco_52xx_state *state = get_safe_token((running_device *)ptr);
	state->latched_cmd = param;
}

static READ8_HANDLER( namco_52xx_K_r )
{
	namco_52xx_state *state = get_safe_token(space->cpu->owner());
	return state->latched_cmd & 0x0f;
}

static READ8_HANDLER( namco_52xx_SI_r )
{
	namco_52xx_state *state = get_safe_token(space->cpu->owner());
	return devcb_call_read8(&state->si, 0) ? 1 : 0;
}

static READ8_HANDLER( namco_52xx_R0_r )
{
	namco_52xx_state *state = get_safe_token(space->cpu->owner());
	return devcb_call_read8(&state->romread, state->address) & 0x0f;
}

static READ8_HANDLER( namco_52xx_R1_r )
{
	namco_52xx_state *state = get_safe_token(space->cpu->owner());
	return devcb_call_read8(&state->romread, state->address) >> 4;
}


static WRITE8_HANDLER( namco_52xx_P_w )
{
	namco_52xx_state *state = get_safe_token(space->cpu->owner());
	discrete_sound_w(state->discrete, NAMCO_52XX_P_DATA(state->basenode), data & 0x0f);
}

static WRITE8_HANDLER( namco_52xx_R2_w )
{
	namco_52xx_state *state = get_safe_token(space->cpu->owner());
	state->address = (state->address & 0xfff0) | ((data & 0xf) << 0);
}

static WRITE8_HANDLER( namco_52xx_R3_w )
{
	namco_52xx_state *state = get_safe_token(space->cpu->owner());
	state->address = (state->address & 0xff0f) | ((data & 0xf) << 4);
}

static WRITE8_HANDLER( namco_52xx_O_w )
{
	namco_52xx_state *state = get_safe_token(space->cpu->owner());
	if (data & 0x10)
		state->address = (state->address & 0x0fff) | ((data & 0xf) << 12);
	else
		state->address = (state->address & 0xf0ff) | ((data & 0xf) << 8);
}




static TIMER_CALLBACK( namco_52xx_irq_clear )
{
	namco_52xx_state *state = get_safe_token((running_device *)ptr);
	cpu_set_input_line(state->cpu, 0, CLEAR_LINE);
}

WRITE8_DEVICE_HANDLER( namco_52xx_write )
{
	namco_52xx_state *state = get_safe_token(device);

	timer_call_after_resynch(device->machine, (void *)device, data, namco_52xx_latch_callback);

	cpu_set_input_line(state->cpu, 0, ASSERT_LINE);

	// The execution time of one instruction is ~4us, so we must make sure to
	// give the cpu time to poll the /IRQ input before we clear it.
	// The input clock to the 06XX interface chip is 64H, that is
	// 18432000/6/64 = 48kHz, so it makes sense for the irq line to be
	// asserted for one clock cycle ~= 21us.

	/* the 52xx uses TSTI to check for an interrupt; it also may be handling
       a timer interrupt, so we need to ensure the IRQ line is held long enough */
	timer_set(device->machine, ATTOTIME_IN_USEC(5*21), (void *)device, 0, namco_52xx_irq_clear);
}


static TIMER_CALLBACK( external_clock_pulse )
{
	namco_52xx_state *state = get_safe_token((running_device *)ptr);
	mb88_external_clock_w(state->cpu, 1);
	mb88_external_clock_w(state->cpu, 0);
}


/***************************************************************************
    DEVICE INTERFACE
***************************************************************************/

static ADDRESS_MAP_START( namco_52xx_map_io, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(MB88_PORTK,  MB88_PORTK)  AM_READ(namco_52xx_K_r)
	AM_RANGE(MB88_PORTO,  MB88_PORTO)  AM_WRITE(namco_52xx_O_w)
	AM_RANGE(MB88_PORTP,  MB88_PORTP)  AM_WRITE(namco_52xx_P_w)
	AM_RANGE(MB88_PORTSI, MB88_PORTSI) AM_READ(namco_52xx_SI_r)
	AM_RANGE(MB88_PORTR0, MB88_PORTR0) AM_READ(namco_52xx_R0_r)
	AM_RANGE(MB88_PORTR1, MB88_PORTR1) AM_READ(namco_52xx_R1_r)
	AM_RANGE(MB88_PORTR2, MB88_PORTR2) AM_WRITE(namco_52xx_R2_w)
	AM_RANGE(MB88_PORTR3, MB88_PORTR3) AM_WRITE(namco_52xx_R3_w)
ADDRESS_MAP_END


static MACHINE_DRIVER_START( namco_52xx )
	MDRV_CPU_ADD("mcu", MB8843, DERIVED_CLOCK(1,1))		/* parent clock, internally divided by 6 */
	MDRV_CPU_IO_MAP(namco_52xx_map_io)
MACHINE_DRIVER_END


ROM_START( namco_52xx )
	ROM_REGION( 0x400, "mcu", ROMREGION_LOADBYNAME )
	ROM_LOAD( "52xx.bin",     0x0000, 0x0400, CRC(3257d11e) SHA1(4883b2fdbc99eb7b9906357fcc53915842c2c186) )
ROM_END


/*-------------------------------------------------
    device start callback
-------------------------------------------------*/

static DEVICE_START( namco_52xx )
{
	namco_52xx_interface *intf = (namco_52xx_interface *)device->baseconfig().static_config();
	namco_52xx_state *state = get_safe_token(device);
	astring tempstring;

	/* find our CPU */
	state->cpu = device->subdevice("mcu");
	assert(state->cpu != NULL);

	/* find the attached discrete sound device */
	assert(intf->discrete != NULL);
	state->discrete = device->machine->device(intf->discrete);
	assert(state->discrete != NULL);
	state->basenode = intf->firstnode;

	/* resolve our read/write callbacks */
	devcb_resolve_read8(&state->romread, &intf->romread, device);
	devcb_resolve_read8(&state->si, &intf->si, device);

	/* start the external clock */
	if (intf->extclock != 0)
		timer_pulse(device->machine, attotime_make(0, intf->extclock), (void *)device, 0, external_clock_pulse);
}


/*-------------------------------------------------
    device definition
-------------------------------------------------*/

static const char DEVTEMPLATE_SOURCE[] = __FILE__;

#define DEVTEMPLATE_ID(p,s)		p##namco_52xx##s
#define DEVTEMPLATE_FEATURES	DT_HAS_START | DT_HAS_ROM_REGION | DT_HAS_MACHINE_CONFIG
#define DEVTEMPLATE_NAME		"Namco 52xx"
#define DEVTEMPLATE_FAMILY		"Namco I/O"
#include "devtempl.h"


DEFINE_LEGACY_DEVICE(NAMCO_52XX, namco_52xx);
