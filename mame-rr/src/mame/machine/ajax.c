/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "cpu/z80/z80.h"
#include "cpu/konami/konami.h"
#include "video/konicdev.h"
#include "includes/ajax.h"

/*  ajax_bankswitch_w:
    Handled by the LS273 Octal +ve edge trigger D-type Flip-flop with Reset at H11:

    Bit Description
    --- -----------
    7   MRB3    Selects ROM N11/N12
    6   CCOUNT2 Coin Counter 2  (*)
    5   CCOUNT1 Coin Counter 1  (*)
    4   SRESET  Slave CPU Reset?
    3   PRI0    Layer Priority Selector
    2   MRB2    \
    1   MRB1     |  ROM Bank Select
    0   MRB0    /

    (*) The Coin Counters are handled by the Konami Custom 051550
*/

static WRITE8_HANDLER( ajax_bankswitch_w )
{
	ajax_state *state = (ajax_state *)space->machine->driver_data;
	int bank = 0;

	/* rom select */
	if (!(data & 0x80))
		bank += 4;

	/* coin counters */
	coin_counter_w(space->machine, 0, data & 0x20);
	coin_counter_w(space->machine, 1, data & 0x40);

	/* priority */
	state->priority = data & 0x08;

	/* bank # (ROMS N11 and N12) */
	bank += (data & 0x07);
	memory_set_bank(space->machine, "bank2", bank);
}

/*  ajax_lamps_w:
    Handled by the LS273 Octal +ve edge trigger D-type Flip-flop with Reset at B9:

    Bit Description
    --- -----------
    7   LAMP7 & LAMP8 - Game over lamps (*)
    6   LAMP3 & LAMP4 - Game over lamps (*)
    5   LAMP1 - Start lamp (*)
    4   Control panel quaking (**)
    3   Joystick vibration (**)
    2   LAMP5 & LAMP6 - Power up lamps (*)
    1   LAMP2 - Super weapon lamp (*)
    0   unused

    (*) The Lamps are handled by the M54585P
    (**)Vibration/Quaking handled by these chips:
        Chip        Location    Description
        ----        --------    -----------
        PS2401-4    B21         ???
        UPA1452H    B22         ???
        LS74        H2          Dual +ve edge trigger D-type Flip-flop with SET and RESET
        LS393       C20         Dual -ve edge trigger 4-bit Binary Ripple Counter with Resets
*/

static WRITE8_HANDLER( ajax_lamps_w )
{
	set_led_status(space->machine, 1, data & 0x02);	/* super weapon lamp */
	set_led_status(space->machine, 2, data & 0x04);	/* power up lamps */
	set_led_status(space->machine, 5, data & 0x04);	/* power up lamps */
	set_led_status(space->machine, 0, data & 0x20);	/* start lamp */
	set_led_status(space->machine, 3, data & 0x40);	/* game over lamps */
	set_led_status(space->machine, 6, data & 0x40);	/* game over lamps */
	set_led_status(space->machine, 4, data & 0x80);	/* game over lamps */
	set_led_status(space->machine, 7, data & 0x80);	/* game over lamps */
}

/*  ajax_ls138_f10:
    The LS138 1-of-8 Decoder/Demultiplexer at F10 selects what to do:

    Address R/W Description
    ------- --- -----------
    0x0000  (r) ??? I think this read is because a CPU core bug
            (w) 0x0000  NSFIRQ  Trigger FIRQ on the M6809
                0x0020  AFR     Watchdog reset (handled by the 051550)
    0x0040  (w) SOUND           Cause interrupt on the Z80
    0x0080  (w) SOUNDDATA       Sound code number
    0x00c0  (w) MBL1            Enables the LS273 at H11 (Banking + Coin counters)
    0x0100  (r) MBL2            Enables 2P Inputs reading
    0x0140  (w) MBL3            Enables the LS273 at B9 (Lamps + Vibration)
    0x0180  (r) MIO1            Enables 1P Inputs + DIPSW #1 & #2 reading
    0x01c0  (r) MIO2            Enables DIPSW #3 reading
*/

READ8_HANDLER( ajax_ls138_f10_r )
{
	int data = 0, index;
	static const char *const portnames[] = { "SYSTEM", "P1", "DSW1", "DSW2" };

	switch ((offset & 0x01c0) >> 6)
	{
		case 0x00:	/* ??? */
			data = mame_rand(space->machine);
			break;
		case 0x04:	/* 2P inputs */
			data = input_port_read(space->machine, "P2");
			break;
		case 0x06:	/* 1P inputs + DIPSW #1 & #2 */
			index = offset & 0x01;
			data = input_port_read(space->machine, (offset & 0x02) ? portnames[2 + index] : portnames[index]);
			break;
		case 0x07:	/* DIPSW #3 */
			data = input_port_read(space->machine, "DSW3");
			break;

		default:
			logerror("%04x: (ls138_f10) read from an unknown address %02x\n",cpu_get_pc(space->cpu), offset);
	}

	return data;
}

WRITE8_HANDLER( ajax_ls138_f10_w )
{
	ajax_state *state = (ajax_state *)space->machine->driver_data;

	switch ((offset & 0x01c0) >> 6)
	{
		case 0x00:	/* NSFIRQ + AFR */
			if (offset)
				watchdog_reset_w(space, 0, data);
			else{
				if (state->firq_enable)	/* Cause interrupt on slave CPU */
					cpu_set_input_line(state->subcpu, M6809_FIRQ_LINE, HOLD_LINE);
			}
			break;
		case 0x01:	/* Cause interrupt on audio CPU */
			cpu_set_input_line(state->audiocpu, 0, HOLD_LINE);
			break;
		case 0x02:	/* Sound command number */
			soundlatch_w(space, offset, data);
			break;
		case 0x03:	/* Bankswitch + coin counters + priority*/
			ajax_bankswitch_w(space, 0, data);
			break;
		case 0x05:	/* Lamps + Joystick vibration + Control panel quaking */
			ajax_lamps_w(space, 0, data);
			break;

		default:
			logerror("%04x: (ls138_f10) write %02x to an unknown address %02x\n", cpu_get_pc(space->cpu), data, offset);
	}
}

/*  ajax_bankswitch_w_2:
    Handled by the LS273 Octal +ve edge trigger D-type Flip-flop with Reset at K14:

    Bit Description
    --- -----------
    7   unused
    6   RMRD    Enable char ROM reading through the video RAM
    5   RVO     enables 051316 wraparound
    4   FIRQST  FIRQ control
    3   SRB3    \
    2   SRB2     |
    1   SRB1     |  ROM Bank Select
    0   SRB0    /
*/

WRITE8_HANDLER( ajax_bankswitch_2_w )
{
	ajax_state *state = (ajax_state *)space->machine->driver_data;

	/* enable char ROM reading through the video RAM */
	k052109_set_rmrd_line(state->k052109, (data & 0x40) ? ASSERT_LINE : CLEAR_LINE);

	/* bit 5 enables 051316 wraparound */
	k051316_wraparound_enable(state->k051316, data & 0x20);

	/* FIRQ control */
	state->firq_enable = data & 0x10;

	/* bank # (ROMS G16 and I16) */
	memory_set_bank(space->machine, "bank1", data & 0x0f);
}

MACHINE_START( ajax )
{
	ajax_state *state = (ajax_state *)machine->driver_data;
	UINT8 *MAIN = memory_region(machine, "maincpu");
	UINT8 *SUB  = memory_region(machine, "sub");

	memory_configure_bank(machine, "bank1", 0,  9,  &SUB[0x10000], 0x2000);
	memory_configure_bank(machine, "bank2", 0, 12, &MAIN[0x10000], 0x2000);

	memory_set_bank(machine, "bank1", 0);
	memory_set_bank(machine, "bank2", 0);

	state->maincpu = machine->device("maincpu");
	state->audiocpu = machine->device("audiocpu");
	state->subcpu = machine->device("sub");
	state->k007232_1 = machine->device("k007232_1");
	state->k007232_2 = machine->device("k007232_2");
	state->k052109 = machine->device("k052109");
	state->k051960 = machine->device("k051960");
	state->k051316 = machine->device("k051316");

	state_save_register_global(machine, state->priority);
	state_save_register_global(machine, state->firq_enable);
}

MACHINE_RESET( ajax )
{
	ajax_state *state = (ajax_state *)machine->driver_data;

	state->priority = 0;
	state->firq_enable = 0;
}

INTERRUPT_GEN( ajax_interrupt )
{
	ajax_state *state = (ajax_state *)device->machine->driver_data;

	if (k051960_is_irq_enabled(state->k051960))
		cpu_set_input_line(device, KONAMI_IRQ_LINE, HOLD_LINE);
}
