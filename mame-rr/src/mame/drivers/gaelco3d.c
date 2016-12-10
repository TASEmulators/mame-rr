/*************************************************************************

    Driver for Gaelco 3D games

    driver by Aaron Giles

    Games supported:
        * Speed Up
        * Surf Planet
        * Radikal Bikers

    Known bugs:
        * EEPROM interface not right

***************************************************************************

PCB Layout -- Radikal Bikers
----------

Top board

REF. 980311
|----------------------------------------------------------------|
|  RAB.6  RAB.14                          PAL            RAB.48* |
|     RAB.12  RAB.19                                             |
|                         RAB.23*   ADSP-2115  16MHz     RAB.45* |
|                                                                |
|                                                                |
|                             RAB.24  RAB.32           TDA1543   |
|                                                         TL074  |
| RAB.8*    RAB.15*           RAB.25  RAB.33                     |
| RAB.9*    RAB.16*                                              |
|                             RAB.26  RAB.34                     |
|                                                                |
| RAB.10*   RAB.17*           RAB.27  RAB.35           TDA1543   |
| RAB.11*   RAB.18*                                       TL074  |
|                                                                |
|                                                                |
|                                                                |
|----------------------------------------------------------------|
Notes:
      Contents of RAB.24 to RAB.27 = RAB.32 to RAB.35
      * - These ROMs are surface mounted


Bottom board

REF. 980512
|----------------------------------------------------------------|
|       ALTERA                             TMS320C31      50MHz  |
|50MHz  EPM7064   68EC020  CY7C199         (QFP132)              |
|                          CY7C199                               |
|                          CY7C199         KM4216C256            |
|93C66                     CY7C199                               |
|                          CY7C199        |--------|             |
|                          CY7C199        |3D-3G   |             |
|75LBC176                                 |(QFP206)|             |
|75LBC176                                 |        |             |
|                   |------------|        |--------|             |
|                   |CHIP EXPRESS|                               |
|                   |RASTER      |         45MHz                 |
|                   |M1178-01    |                               |
|                   |M032541-4   |                               |
|     CY7C199       |------------|                               |
|     CY7C199                                                    |
|     CY7C199                                                    |
|     CY7C199                                                    |
|                  KM4216C256   KM4216C256                       |
|  |------------|                                                |
|  |CHIP EXPRESS|  KM4216C256   KM4216C256    PAL                |
|  |CHK1        |                                                |
|  |M1105-01    |               KM4216C256                       |
|  |M048494-22  |                                                |
|  |------------|                                                |
|----------------------------------------------------------------|


PCB Layout -- Surf Planet
----------

Top board

REF. 971223
|----------------------------------------------------------------|
|  PLS.5  PLS.11                          PAL            PLS.37* |
|     PLS.8  PLS.13                                              |
|                         PLS.18*   ADSP-2115  16MHz     PLS.40* |
|                                                                |
|                                                                |
|                             PLS.19  PLS.27           TDA1543   |
|                                                         TL074  |
|                             PLS.20  PLS.28                     |
|                                                                |
|                             PLS.21  PLS.29                     |
|                                                                |
| PLS.7*  PLS.12*             PLS.22  PLS.30           TDA1543   |
|    PLS.9*   PLS.15*                                     TL074  |
|                                                                |
| TLC549                                                         |
| LM358                                                          |
|----------------------------------------------------------------|
Notes:
      Contents of PLS.19 to PLS.22 = PLS.27 to PLS.30
      * - These ROMs are surface mounted


Bottom board

REF. 970429
|----------------------------------------------------------------|
|       PAL                 60MHz          TMS320C31             |
|          68HC000         CY7C199         (QFP132)              |
|                          CY7C199                               |
|                          CY7C199         KM4216C256            |
|93C66                     CY7C199                               |
|                          CY7C199        |--------|             |
|                          CY7C199        |3D-3G   |             |
|75LBC176                                 |(QFP206)|             |
|75LBC176                                 |        |             |
|                   |------------|        |--------|             |
|                   |CHIP EXPRESS|                               |
|                   |RASTER      |         45MHz                 |
|                   |M027851-1   |                               |
|                   |9706 CI USA |                               |
|     CY7C199       |------------|                               |
|     CY7C199                                                    |
|     CY7C199                                                    |
|     CY7C199                                                    |
|                  KM4216C256   KM4216C256                       |
|  |------------|                                                |
|  |CHIP EXPRESS|  KM4216C256   KM4216C256    PAL                |
|  |SU3DCOL     |                                                |
|  |M026402-3   |               KM4216C256                       |
|  |9647 CI USA |                                                |
|  |------------|                                                |
|----------------------------------------------------------------|


6 * KM4216C256G = 6 * 256k x 16
10 * CY7C199 =   10 * 32k x 8


**************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "includes/gaelco3d.h"
#include "cpu/tms32031/tms32031.h"
#include "cpu/adsp2100/adsp2100.h"
#include "machine/eeprom.h"
#include "sound/dmadac.h"

#define	LOG				0

#define SOUND_CHANNELS	4


static UINT32 *adsp_ram_base;
static UINT16 *m68k_ram_base;
static UINT16 *tms_comm_base;
static UINT16 sound_data;
static UINT8 sound_status;
static offs_t tms_offset_xor;
static UINT8 analog_ports[4];
static UINT8 framenum;

static timer_device *adsp_autobuffer_timer;
static UINT16 *adsp_control_regs;
static UINT16 *adsp_fastram_base;
static UINT8 adsp_ireg;
static offs_t adsp_ireg_base, adsp_incs, adsp_size;
static dmadac_sound_device *dmadac[SOUND_CHANNELS];

static void adsp_tx_callback(cpu_device &device, int port, INT32 data);


/*************************************
 *
 *  Machine init
 *
 *************************************/

static MACHINE_START( gaelco3d )
{
	/* Save state support */
	state_save_register_global(machine, sound_data);
	state_save_register_global(machine, sound_status);
	state_save_register_global_array(machine, analog_ports);
	state_save_register_global(machine, framenum);
	state_save_register_global(machine, adsp_ireg);
	state_save_register_global(machine, adsp_ireg_base);
	state_save_register_global(machine, adsp_incs);
	state_save_register_global(machine, adsp_size);
}


static MACHINE_RESET( common )
{
	UINT16 *src;
	int i;

	framenum = 0;

	/* boot the ADSP chip */
	src = (UINT16 *)memory_region(machine, "user1");
	for (i = 0; i < (src[3] & 0xff) * 8; i++)
	{
		UINT32 opcode = ((src[i*4+0] & 0xff) << 16) | ((src[i*4+1] & 0xff) << 8) | (src[i*4+2] & 0xff);
		adsp_ram_base[i] = opcode;
	}

	/* allocate a timer for feeding the autobuffer */
	adsp_autobuffer_timer = machine->device<timer_device>("adsp_timer");

	memory_configure_bank(machine, "bank1", 0, 256, memory_region(machine, "user1"), 0x4000);
	memory_set_bank(machine, "bank1", 0);

	/* keep the TMS32031 halted until the code is ready to go */
	cputag_set_input_line(machine, "tms", INPUT_LINE_RESET, ASSERT_LINE);

	for (i = 0; i < SOUND_CHANNELS; i++)
	{
		char buffer[10];
		sprintf(buffer, "dac%d", i + 1);
		dmadac[i] = machine->device<dmadac_sound_device>(buffer);
	}
}


static MACHINE_RESET( gaelco3d )
{
	MACHINE_RESET_CALL( common );
	tms_offset_xor = 0;
}


static MACHINE_RESET( gaelco3d2 )
{
	MACHINE_RESET_CALL( common );
	tms_offset_xor = BYTE_XOR_BE(0);
}



/*************************************
 *
 *  IRQ handling
 *
 *************************************/

static INTERRUPT_GEN( vblank_gen )
{
	gaelco3d_render(*device->machine->primary_screen);
	cpu_set_input_line(device, 2, ASSERT_LINE);
}


static WRITE16_HANDLER( irq_ack_w )
{
	cputag_set_input_line(space->machine, "maincpu", 2, CLEAR_LINE);
}



/*************************************
 *
 *  EEPROM (93C66B)
 *
 *************************************/

static READ16_DEVICE_HANDLER( eeprom_data_r )
{
	UINT16 result = 0xffff;
	if (eeprom_read_bit(device))
		result ^= 0x0004;
	if (LOG)
		logerror("eeprom_data_r(%02X)\n", result);
	return result;
}


static WRITE16_DEVICE_HANDLER( eeprom_data_w )
{
	if (ACCESSING_BITS_0_7)
		eeprom_write_bit(device, data & 0x01);
}


static WRITE16_DEVICE_HANDLER( eeprom_clock_w )
{
	if (ACCESSING_BITS_0_7)
		eeprom_set_clock_line(device, (data & 0x01) ? ASSERT_LINE : CLEAR_LINE);
}


static WRITE16_DEVICE_HANDLER( eeprom_cs_w )
{
	if (ACCESSING_BITS_0_7)
		eeprom_set_cs_line(device, (data & 0x01) ? CLEAR_LINE : ASSERT_LINE);
}



/*************************************
 *
 *  Sound CPU comms
 *
 *************************************/

static TIMER_CALLBACK( delayed_sound_w )
{
	if (LOG)
		logerror("delayed_sound_w(%02X)\n", param);
	sound_data = param;
	cputag_set_input_line(machine, "adsp", ADSP2115_IRQ2, ASSERT_LINE);
}


static WRITE16_HANDLER( sound_data_w )
{
	if (LOG)
		logerror("%06X:sound_data_w(%02X) = %08X & %08X\n", cpu_get_pc(space->cpu), offset, data, mem_mask);
	if (ACCESSING_BITS_0_7)
		timer_call_after_resynch(space->machine, NULL, data & 0xff, delayed_sound_w);
}


static READ16_HANDLER( sound_data_r )
{
	if (LOG)
		logerror("sound_data_r(%02X)\n", sound_data);
	cputag_set_input_line(space->machine, "adsp", ADSP2115_IRQ2, CLEAR_LINE);
	return sound_data;
}


static READ16_HANDLER( sound_status_r )
{
	if (LOG)
		logerror("%06X:sound_status_r(%02X) = %02X\n", cpu_get_pc(space->cpu), offset, sound_status);
	if (ACCESSING_BITS_0_7)
		return sound_status;
	return 0xffff;
}


static WRITE16_HANDLER( sound_status_w )
{
	if (LOG)
		logerror("sound_status_w(%02X)\n", sound_data);
	sound_status = data;
}



/*************************************
 *
 *  Input ports
 *
 *************************************/

static CUSTOM_INPUT( analog_bit_r )
{
	int which = (FPTR)param;
	return (analog_ports[which] >> 7) & 0x01;
}


static WRITE16_HANDLER( analog_port_clock_w )
{
	/* a zero/one combo is written here to clock the next analog port bit */
	if (ACCESSING_BITS_0_7)
	{
		if (!(data & 0xff))
		{
			analog_ports[0] <<= 1;
			analog_ports[1] <<= 1;
			analog_ports[2] <<= 1;
			analog_ports[3] <<= 1;
		}
	}
	else
	{
		if (LOG)
			logerror("%06X:analog_port_clock_w(%02X) = %08X & %08X\n", cpu_get_pc(space->cpu), offset, data, mem_mask);
	}
}


static WRITE16_HANDLER( analog_port_latch_w )
{
	/* a zero is written here to read the analog ports, and a one is written when finished */
	if (ACCESSING_BITS_0_7)
	{
		if (!(data & 0xff))
		{
			analog_ports[0] = input_port_read_safe(space->machine, "ANALOG0", 0);
			analog_ports[1] = input_port_read_safe(space->machine, "ANALOG1", 0);
			analog_ports[2] = input_port_read_safe(space->machine, "ANALOG2", 0);
			analog_ports[3] = input_port_read_safe(space->machine, "ANALOG3", 0);
		}
	}
	else
	{
		if (LOG)
			logerror("%06X:analog_port_latch_w(%02X) = %08X & %08X\n", cpu_get_pc(space->cpu), offset, data, mem_mask);
	}

}



/*************************************
 *
 *  TMS32031 interface
 *
 *************************************/

static READ32_HANDLER( tms_m68k_ram_r )
{
//  logerror("%06X:tms_m68k_ram_r(%04X) = %08X\n", cpu_get_pc(space->cpu), offset, !(offset & 1) ? ((INT32)m68k_ram_base[offset/2] >> 16) : (int)(INT16)m68k_ram_base[offset/2]);
	return (INT32)(INT16)m68k_ram_base[offset ^ tms_offset_xor];
}


static WRITE32_HANDLER( tms_m68k_ram_w )
{
	m68k_ram_base[offset ^ tms_offset_xor] = data;
}


static void iack_w(running_device *device, UINT8 state, offs_t addr)
{
	if (LOG)
		logerror("iack_w(%d) - %06X\n", state, addr);
	cpu_set_input_line(device, 0, CLEAR_LINE);
}



/*************************************
 *
 *  TMS32031 control
 *
 *************************************/

static WRITE16_HANDLER( tms_reset_w )
{
	/* this is set to 0 while data is uploaded, then set to $ffff after it is done */
	/* it does not ever appear to be touched after that */
	if (LOG)
		logerror("%06X:tms_reset_w(%02X) = %08X & %08X\n", cpu_get_pc(space->cpu), offset, data, mem_mask);
		cputag_set_input_line(space->machine, "tms", INPUT_LINE_RESET, (data == 0xffff) ? CLEAR_LINE : ASSERT_LINE);
}


static WRITE16_HANDLER( tms_irq_w )
{
	/* this is written twice, 0,1, in quick succession */
	/* done after uploading, and after modifying the comm area */
	if (LOG)
		logerror("%06X:tms_irq_w(%02X) = %08X & %08X\n", cpu_get_pc(space->cpu), offset, data, mem_mask);
	if (ACCESSING_BITS_0_7)
		cputag_set_input_line(space->machine, "tms", 0, (data & 0x01) ? CLEAR_LINE : ASSERT_LINE);
}


static WRITE16_HANDLER( tms_control3_w )
{
	if (LOG)
		logerror("%06X:tms_control3_w(%02X) = %08X & %08X\n", cpu_get_pc(space->cpu), offset, data, mem_mask);
}


static WRITE16_HANDLER( tms_comm_w )
{
	COMBINE_DATA(&tms_comm_base[offset ^ tms_offset_xor]);
	if (LOG)
		logerror("%06X:tms_comm_w(%02X) = %08X & %08X\n", cpu_get_pc(space->cpu), offset*2, data, mem_mask);
}



/*************************************
 *
 *  ADSP control registers
 *
 *************************************/

/* These are the some of the control register, we dont use them all */
enum
{
	S1_AUTOBUF_REG = 15,
	S1_RFSDIV_REG,
	S1_SCLKDIV_REG,
	S1_CONTROL_REG,
	S0_AUTOBUF_REG,
	S0_RFSDIV_REG,
	S0_SCLKDIV_REG,
	S0_CONTROL_REG,
	S0_MCTXLO_REG,
	S0_MCTXHI_REG,
	S0_MCRXLO_REG,
	S0_MCRXHI_REG,
	TIMER_SCALE_REG,
	TIMER_COUNT_REG,
	TIMER_PERIOD_REG,
	WAITSTATES_REG,
	SYSCONTROL_REG
};

/*
ADSP control 3FFF W = 0008  (SYSCONTROL_REG)
ADSP control 3FFE W = 1249  (WAITSTATES_REG)
ADSP control 3FEF W = 0D82  (S1_AUTOBUF_REG)
ADSP control 3FF1 W = 0005  (S1_SCLKDIV_REG)
ADSP control 3FF2 W = 4A0F  (S1_CONTROL_REG)
ADSP control 3FFF W = 0C08  (SYSCONTROL_REG)
*/

static WRITE16_HANDLER( adsp_control_w )
{
	if (LOG)
		logerror("ADSP control %04X W = %04X\n", 0x3fe0 + offset, data);

	adsp_control_regs[offset] = data;
	switch (offset)
	{
		case SYSCONTROL_REG:
			/* see if SPORT1 got disabled */
			if ((data & 0x0800) == 0)
			{
				dmadac_enable(&dmadac[0], SOUND_CHANNELS, 0);
				adsp_autobuffer_timer->reset();
			}
			break;

		case S1_AUTOBUF_REG:
			/* autobuffer off: nuke the timer, and disable the DAC */
			if ((data & 0x0002) == 0)
			{
				dmadac_enable(&dmadac[0], SOUND_CHANNELS, 0);
				adsp_autobuffer_timer->reset();
			}
			break;

		case S1_CONTROL_REG:
			if (((data >> 4) & 3) == 2)
				logerror("Oh no!, the data is compresed with u-law encoding\n");
			if (((data >> 4) & 3) == 3)
				logerror("Oh no!, the data is compresed with A-law encoding\n");
			break;
	}
}


static WRITE16_HANDLER( adsp_rombank_w )
{
	if (LOG)
		logerror("adsp_rombank_w(%d) = %04X\n", offset, data);
	memory_set_bank(space->machine, "bank1", (offset & 1) * 0x80 + (data & 0x7f));
}



/*************************************
 *
 *  ADSP sound generation
 *
 *************************************/

static TIMER_DEVICE_CALLBACK( adsp_autobuffer_irq )
{
	cpu_device *adsp = timer.machine->device<cpu_device>("adsp");

	/* get the index register */
	int reg = adsp->state(ADSP2100_I0 + adsp_ireg);

	/* copy the current data into the buffer */
// logerror("ADSP buffer: I%d=%04X incs=%04X size=%04X\n", adsp_ireg, reg, adsp_incs, adsp_size);
	if (adsp_incs)
		dmadac_transfer(&dmadac[0], SOUND_CHANNELS, adsp_incs, SOUND_CHANNELS * adsp_incs, adsp_size / (SOUND_CHANNELS * adsp_incs), (INT16 *)&adsp_fastram_base[reg - 0x3800]);

	/* increment it */
	reg += adsp_size;

	/* check for wrapping */
	if (reg >= adsp_ireg_base + adsp_size)
	{
		/* reset the base pointer */
		reg = adsp_ireg_base;

		/* generate the (internal, thats why the pulse) irq */
		generic_pulse_irq_line(adsp, ADSP2105_IRQ1);
	}

	/* store it */
	adsp->set_state(ADSP2100_I0 + adsp_ireg, reg);
}


static void adsp_tx_callback(cpu_device &device, int port, INT32 data)
{
	/* check if it's for SPORT1 */
	if (port != 1)
		return;

	/* check if SPORT1 is enabled */
	if (adsp_control_regs[SYSCONTROL_REG] & 0x0800) /* bit 11 */
	{
		/* we only support autobuffer here (wich is what this thing uses), bail if not enabled */
		if (adsp_control_regs[S1_AUTOBUF_REG] & 0x0002) /* bit 1 */
		{
			/* get the autobuffer registers */
			int		mreg, lreg;
			UINT16	source;
			attotime sample_period;

			adsp_ireg = (adsp_control_regs[S1_AUTOBUF_REG] >> 9) & 7;
			mreg = (adsp_control_regs[S1_AUTOBUF_REG] >> 7) & 3;
			mreg |= adsp_ireg & 0x04; /* msb comes from ireg */
			lreg = adsp_ireg;

			/* now get the register contents in a more legible format */
			/* we depend on register indexes to be continuous (wich is the case in our core) */
			source = device.state(ADSP2100_I0 + adsp_ireg);
			adsp_incs = device.state(ADSP2100_M0 + mreg);
			adsp_size = device.state(ADSP2100_L0 + lreg);

			/* get the base value, since we need to keep it around for wrapping */
			source -= adsp_incs;

			/* make it go back one so we dont lose the first sample */
			device.set_state(ADSP2100_I0 + adsp_ireg, source);

			/* save it as it is now */
			adsp_ireg_base = source;

			/* calculate how long until we generate an interrupt */

			/* period per each bit sent */
			sample_period = attotime_mul(ATTOTIME_IN_HZ(device.clock()), 2 * (adsp_control_regs[S1_SCLKDIV_REG] + 1));

			/* now put it down to samples, so we know what the channel frequency has to be */
			sample_period = attotime_mul(sample_period, 16 * SOUND_CHANNELS);

			dmadac_set_frequency(&dmadac[0], SOUND_CHANNELS, ATTOSECONDS_TO_HZ(sample_period.attoseconds));
			dmadac_enable(&dmadac[0], SOUND_CHANNELS, 1);

			/* fire off a timer wich will hit every half-buffer */
			sample_period = attotime_div(attotime_mul(sample_period, adsp_size), SOUND_CHANNELS * adsp_incs);

			adsp_autobuffer_timer->adjust(sample_period, 0, sample_period);

			return;
		}
		else
			logerror( "ADSP SPORT1: trying to transmit and autobuffer not enabled!\n" );
	}

	/* if we get there, something went wrong. Disable playing */
	dmadac_enable(&dmadac[0], SOUND_CHANNELS, 0);

	/* remove timer */
	adsp_autobuffer_timer->reset();
}



/*************************************
 *
 *  Unknown accesses
 *
 *************************************/

static WRITE32_HANDLER( unknown_107_w )
{
	/* arbitrary data written */
	if (ACCESSING_BITS_0_7)
		logerror("%06X:unknown_107_w = %02X\n", cpu_get_pc(space->cpu), data & 0xff);
	else
		logerror("%06X:unknown_107_w(%02X) = %08X & %08X\n", cpu_get_pc(space->cpu), offset, data, mem_mask);
}

static WRITE32_HANDLER( unknown_127_w )
{
	/* arbitrary data written */
	if (ACCESSING_BITS_0_7)
		logerror("%06X:unknown_127_w = %02X\n", cpu_get_pc(space->cpu), data & 0xff);
	else
		logerror("%06X:unknown_127_w(%02X) = %08X & %08X\n", cpu_get_pc(space->cpu), offset, data, mem_mask);
}

static WRITE32_HANDLER( unknown_137_w )
{
	/* only written $00 or $ff */
	if (ACCESSING_BITS_0_7)
		logerror("%06X:unknown_137_w = %02X\n", cpu_get_pc(space->cpu), data & 0xff);
	else
		logerror("%06X:unknown_137_w(%02X) = %08X & %08X\n", cpu_get_pc(space->cpu), offset, data, mem_mask);
}

static WRITE32_HANDLER( unknown_13a_w )
{
	/* only written $0000 or $0001 */
	if (ACCESSING_BITS_0_15)
		logerror("%06X:unknown_13a_w = %04X\n", cpu_get_pc(space->cpu), data & 0xffff);
	else
		logerror("%06X:unknown_13a_w(%02X) = %08X & %08X\n", cpu_get_pc(space->cpu), offset, data, mem_mask);
}


static WRITE16_HANDLER( led_0_w )
{
	/* this has $00 written during an IRQ 6, then reset to $ff afterwards */
	/* LED??? */
	if (ACCESSING_BITS_0_7)
		set_led_status(space->machine, 0, data != 0);
}


static WRITE16_HANDLER( led_1_w )
{
	/* LED??? -- only written $00 or $ff */
	if (ACCESSING_BITS_0_7)
		set_led_status(space->machine, 1, data != 0);
}



/*************************************
 *
 *  Memory maps
 *
 *************************************/

static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 16 )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x1fffff) AM_ROM
	AM_RANGE(0x400000, 0x40ffff) AM_RAM_WRITE(gaelco3d_paletteram_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x51000c, 0x51000d) AM_READ_PORT("IN0")
	AM_RANGE(0x51001c, 0x51001d) AM_READ_PORT("IN1")
	AM_RANGE(0x51002c, 0x51002d) AM_READ_PORT("IN2")
	AM_RANGE(0x51003c, 0x51003d) AM_READ_PORT("IN3")
	AM_RANGE(0x510040, 0x510041) AM_WRITE(sound_data_w)
	AM_RANGE(0x510042, 0x510043) AM_READ(sound_status_r)
	AM_RANGE(0x510100, 0x510101) AM_DEVREAD("eeprom", eeprom_data_r)
	AM_RANGE(0x510100, 0x510101) AM_WRITE(irq_ack_w)
	AM_RANGE(0x51010a, 0x51010b) AM_WRITENOP		// very noisy when starting a new game
	AM_RANGE(0x510110, 0x510113) AM_DEVWRITE("eeprom", eeprom_data_w)
	AM_RANGE(0x510116, 0x510117) AM_WRITE(tms_control3_w)
	AM_RANGE(0x510118, 0x51011b) AM_DEVWRITE("eeprom", eeprom_clock_w)
	AM_RANGE(0x510120, 0x510123) AM_DEVWRITE("eeprom", eeprom_cs_w)
	AM_RANGE(0x51012a, 0x51012b) AM_WRITE(tms_reset_w)
	AM_RANGE(0x510132, 0x510133) AM_WRITE(tms_irq_w)
	AM_RANGE(0x510146, 0x510147) AM_WRITE(led_0_w)
	AM_RANGE(0x510156, 0x510157) AM_WRITE(analog_port_clock_w)
	AM_RANGE(0x510166, 0x510167) AM_WRITE(analog_port_latch_w)
	AM_RANGE(0x510176, 0x510177) AM_WRITE(led_1_w)
	AM_RANGE(0xfe7f80, 0xfe7fff) AM_WRITE(tms_comm_w) AM_BASE(&tms_comm_base)
	AM_RANGE(0xfe0000, 0xfeffff) AM_RAM AM_BASE(&m68k_ram_base)
ADDRESS_MAP_END


static ADDRESS_MAP_START( main020_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM
	AM_RANGE(0x400000, 0x40ffff) AM_RAM_WRITE(gaelco3d_paletteram_020_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x51000c, 0x51000f) AM_READ_PORT("IN0")
	AM_RANGE(0x51001c, 0x51001f) AM_READ_PORT("IN1")
	AM_RANGE(0x51002c, 0x51002f) AM_READ_PORT("IN2")
	AM_RANGE(0x51003c, 0x51003f) AM_READ_PORT("IN3")
	AM_RANGE(0x510040, 0x510043) AM_READ16(sound_status_r, 0x0000ffff)
	AM_RANGE(0x510040, 0x510043) AM_WRITE16(sound_data_w, 0xffff0000)
	AM_RANGE(0x510100, 0x510103) AM_DEVREAD16("eeprom", eeprom_data_r, 0xffff0000)
	AM_RANGE(0x510100, 0x510103) AM_WRITE16(irq_ack_w, 0xffff0000)
	AM_RANGE(0x510104, 0x510107) AM_WRITE(unknown_107_w)
	AM_RANGE(0x510110, 0x510113) AM_DEVWRITE16("eeprom", eeprom_data_w, 0x0000ffff)
	AM_RANGE(0x510114, 0x510117) AM_WRITE16(tms_control3_w, 0x0000ffff)
	AM_RANGE(0x510118, 0x51011b) AM_DEVWRITE16("eeprom", eeprom_clock_w, 0x0000ffff)
	AM_RANGE(0x510120, 0x510123) AM_DEVWRITE16("eeprom", eeprom_cs_w, 0x0000ffff)
	AM_RANGE(0x510124, 0x510127) AM_WRITE(unknown_127_w)
	AM_RANGE(0x510128, 0x51012b) AM_WRITE16(tms_reset_w, 0x0000ffff)
	AM_RANGE(0x510130, 0x510133) AM_WRITE16(tms_irq_w, 0x0000ffff)
	AM_RANGE(0x510134, 0x510137) AM_WRITE(unknown_137_w)
	AM_RANGE(0x510138, 0x51013b) AM_WRITE(unknown_13a_w)
	AM_RANGE(0x510144, 0x510147) AM_WRITE16(led_0_w, 0x0000ffff)
	AM_RANGE(0x510154, 0x510157) AM_WRITE16(analog_port_clock_w, 0x0000ffff)
	AM_RANGE(0x510164, 0x510167) AM_WRITE16(analog_port_latch_w, 0x0000ffff)
	AM_RANGE(0x510174, 0x510177) AM_WRITE16(led_1_w, 0x0000ffff)
	AM_RANGE(0xfe7f80, 0xfe7fff) AM_WRITE16(tms_comm_w, 0xffffffff) AM_BASE((UINT32 **)&tms_comm_base)
	AM_RANGE(0xfe0000, 0xfeffff) AM_RAM AM_BASE((UINT32 **)&m68k_ram_base)
ADDRESS_MAP_END

static ADDRESS_MAP_START( tms_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x000000, 0x007fff) AM_READWRITE(tms_m68k_ram_r, tms_m68k_ram_w)
	AM_RANGE(0x400000, 0x5fffff) AM_ROM AM_REGION("user2", 0)
	AM_RANGE(0xc00000, 0xc00007) AM_WRITE(gaelco3d_render_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( adsp_program_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x0000, 0x03ff) AM_RAM AM_BASE(&adsp_ram_base)		/* 1k words internal RAM */
	AM_RANGE(0x37ff, 0x37ff) AM_READNOP							/* speedup hammers this for no apparent reason */
ADDRESS_MAP_END

static ADDRESS_MAP_START( adsp_data_map, ADDRESS_SPACE_DATA, 16 )
	AM_RANGE(0x0000, 0x0001) AM_WRITE(adsp_rombank_w)
	AM_RANGE(0x0000, 0x1fff) AM_ROMBANK("bank1")
	AM_RANGE(0x2000, 0x2000) AM_READWRITE(sound_data_r, sound_status_w)
	AM_RANGE(0x3800, 0x39ff) AM_RAM AM_BASE(&adsp_fastram_base)	/* 512 words internal RAM */
	AM_RANGE(0x3fe0, 0x3fff) AM_WRITE(adsp_control_w) AM_BASE(&adsp_control_regs)
ADDRESS_MAP_END



/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( speedup )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON5 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_LSHIFT)	// view
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_LALT)		// brake
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_SPACE)	PORT_TOGGLE // gear (low=1 high=2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )		// start
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )		// verified
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)	// checked after reading analog from port 1
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)	// checked after reading analog from port 2
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)	// checked after reading analog from port 3
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_CUSTOM(analog_bit_r, (void *)0)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_CUSTOM(analog_bit_r, (void *)1)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_CUSTOM(analog_bit_r, (void *)2)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_CUSTOM(analog_bit_r, (void *)3)

	PORT_START("IN3")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )		// verified
	PORT_SERVICE_NO_TOGGLE( 0x0200, IP_ACTIVE_LOW )		// verified
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("ANALOG0")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(25) PORT_KEYDELTA(25)

	PORT_START("ANALOG1")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(20)
INPUT_PORTS_END


static INPUT_PORTS_START( surfplnt )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_VOLUME_UP )	// low two bits read, compared against 3
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )	// low four bits read, compared against f
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )		// checked
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )		// start
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )		// coin
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0800, IP_ACTIVE_LOW )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_CUSTOM(analog_bit_r, (void *)0)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_CUSTOM(analog_bit_r, (void *)1)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_CUSTOM(analog_bit_r, (void *)2)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_CUSTOM(analog_bit_r, (void *)3)

	PORT_START("IN3")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("ANALOG0")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(25) PORT_KEYDELTA(25)
INPUT_PORTS_END


static INPUT_PORTS_START( radikalb )
	PORT_START("IN0")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )	// handle up
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_BUTTON3 )		// view
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_BUTTON2 )		// brake
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_BUTTON1 )		// accel
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_START1 )		// start
	PORT_BIT( 0xff000000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x00ff0000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_COIN1 )		// coin
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x08000000, IP_ACTIVE_LOW )
	PORT_BIT( 0x10000000, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_CUSTOM(analog_bit_r, (void *)0)
	PORT_BIT( 0x20000000, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_CUSTOM(analog_bit_r, (void *)1)
	PORT_BIT( 0x40000000, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_CUSTOM(analog_bit_r, (void *)2)
	PORT_BIT( 0x80000000, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_CUSTOM(analog_bit_r, (void *)3)

	PORT_START("IN3")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("ANALOG0")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(25) PORT_KEYDELTA(25)
INPUT_PORTS_END



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static const adsp21xx_config adsp_config =
{
	NULL,					/* callback for serial receive */
	adsp_tx_callback,		/* callback for serial transmit */
	NULL					/* callback for timer fired */
};

static const tms32031_config tms_config =
{
	0x1000,
	0,
	0,
	iack_w
};


static MACHINE_DRIVER_START( gaelco3d )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 15000000)
	MDRV_CPU_PROGRAM_MAP(main_map)
	MDRV_CPU_VBLANK_INT("screen", vblank_gen)

	MDRV_CPU_ADD("tms", TMS32031, 60000000)
	MDRV_CPU_CONFIG(tms_config)
	MDRV_CPU_PROGRAM_MAP(tms_map)

	MDRV_CPU_ADD("adsp", ADSP2115, 16000000)
	MDRV_CPU_CONFIG(adsp_config)
	MDRV_CPU_PROGRAM_MAP(adsp_program_map)
	MDRV_CPU_DATA_MAP(adsp_data_map)

	MDRV_MACHINE_START(gaelco3d)
	MDRV_MACHINE_RESET(gaelco3d)

	MDRV_EEPROM_93C66B_ADD("eeprom")

	MDRV_QUANTUM_TIME(HZ(6000))

	MDRV_TIMER_ADD("adsp_timer", adsp_autobuffer_irq)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(576, 432)
	MDRV_SCREEN_VISIBLE_AREA(0, 575, 0, 431)

	MDRV_PALETTE_LENGTH(32768)

	MDRV_PALETTE_INIT(RRRRR_GGGGG_BBBBB)
	MDRV_VIDEO_START(gaelco3d)
	MDRV_VIDEO_UPDATE(gaelco3d)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("dac1", DMADAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)	/* speedup: front mono */

	MDRV_SOUND_ADD("dac2", DMADAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)	/* speedup: left rear */

	MDRV_SOUND_ADD("dac3", DMADAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)	/* speedup: right rear */

	MDRV_SOUND_ADD("dac4", DMADAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)	/* speedup: seat speaker */
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( gaelco3d2 )
	MDRV_IMPORT_FROM(gaelco3d)

	/* basic machine hardware */
	MDRV_CPU_REPLACE("maincpu", M68EC020, 25000000)
	MDRV_CPU_PROGRAM_MAP(main020_map)
	MDRV_CPU_VBLANK_INT("screen", vblank_gen)

	MDRV_CPU_MODIFY("tms")
	MDRV_CPU_CLOCK(50000000)

	MDRV_MACHINE_RESET(gaelco3d2)
MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( speedup )
	ROM_REGION( 0x200000, "maincpu", 0 )	/* 68000 code */
	ROM_LOAD16_BYTE( "sup10.bin", 0x000000, 0x80000, CRC(07e70bae) SHA1(17013d859ec075e12518b094040a056d850b3271) )
	ROM_LOAD16_BYTE( "sup15.bin", 0x000001, 0x80000, CRC(7947c28d) SHA1(46efb56d0f7fe2e92d0d04dcd2f130aef3be436d) )

	ROM_REGION16_LE( 0x400000, "user1", 0 )	/* ADSP-2115 code & data */
	ROM_LOAD( "sup25.bin", 0x0000000, 0x400000, CRC(284c7cd1) SHA1(58fbe73195aac9808a347c543423593e17ad3a10) )

	ROM_REGION32_LE( 0x800000, "user2", 0 )
	ROM_LOAD32_WORD( "sup32.bin", 0x000000, 0x200000, CRC(aed151de) SHA1(a139d4451d3758aa70621a25289d64c98c26d5c0) )
	ROM_LOAD32_WORD( "sup33.bin", 0x000002, 0x200000, CRC(9be6ab7d) SHA1(8bb07f2a096d1f8989a5a409f87b35b7d771de88) )

	ROM_REGION( 0x1000000, "gfx1", 0 )
	ROM_LOAD( "sup12.bin", 0x0000000, 0x400000, CRC(311f3247) SHA1(95014ea177011521a01df85fb511e5e6673dbdcb) )
	ROM_LOAD( "sup14.bin", 0x0400000, 0x400000, CRC(3ad3c089) SHA1(1bd577679ed436251995a100aece2c26c0214fd8) )
	ROM_LOAD( "sup11.bin", 0x0800000, 0x400000, CRC(b993e65a) SHA1(b95bd4c1eac7fba1d2429250446b58f741350bb3) )
	ROM_LOAD( "sup13.bin", 0x0c00000, 0x400000, CRC(ad00023c) SHA1(9d7cce280fff38d7e0dac21e7a1774809d9758bd) )

	ROM_REGION( 0x0080000, "gfx2", 0 )
	ROM_LOAD( "ic35.bin", 0x0000000, 0x020000, CRC(34737d1d) SHA1(e9109a88e211aa49851e72a6fa3417f1cad1cb8b) )
	ROM_LOAD( "ic34.bin", 0x0020000, 0x020000, CRC(e89e829b) SHA1(50c99bd9667d78a61252eaad5281a2e7f57be85a) )
	/* these 2 are copies of the previous 2 */
//  ROM_LOAD( "ic43.bin", 0x0000000, 0x020000, CRC(34737d1d) SHA1(e9109a88e211aa49851e72a6fa3417f1cad1cb8b) )
//  ROM_LOAD( "ic42.bin", 0x0020000, 0x020000, CRC(e89e829b) SHA1(50c99bd9667d78a61252eaad5281a2e7f57be85a) )
ROM_END


ROM_START( surfplnt )
	ROM_REGION( 0x200000, "maincpu", 0 )	/* 68000 code */
	ROM_LOAD16_BYTE( "surfplnt.u5",  0x000000, 0x80000, CRC(c96e0a18) SHA1(b313d02d1d1bff8717b3d798e6ae681baefc1061) )
	ROM_LOAD16_BYTE( "surfplnt.u11", 0x000001, 0x80000, CRC(99211d2d) SHA1(dee5b157489ce9c6988c8eec92fa91fff60d521c) )
	ROM_LOAD16_BYTE( "surfplnt.u8",  0x100000, 0x80000, CRC(aef9e1d0) SHA1(15258e62fbf61e21e7d77aa7a81fdbf842fd4560) )
	ROM_LOAD16_BYTE( "surfplnt.u13", 0x100001, 0x80000, CRC(d9754369) SHA1(0d82569cb925402a9f4634e52f15435112ec4878) )

	ROM_REGION16_LE( 0x400000, "user1", 0 )	/* ADSP-2115 code & data */
	ROM_LOAD( "pls.18", 0x0000000, 0x400000, CRC(a1b64695) SHA1(7487cd51305e30a5b55aada0bae9161fcb3fcd19) )

	ROM_REGION32_LE( 0x800000, "user2", 0 )
	ROM_LOAD32_WORD( "pls.40", 0x000000, 0x400000, CRC(26877ad3) SHA1(2e0c15b0e060e0b3d5b5cdaf1e22b9ec8e1abc9a) )
	ROM_LOAD32_WORD( "pls.37", 0x000002, 0x400000, CRC(75893062) SHA1(81f10243336a309f8cc8532ee9a130ecc35bbcd6) )

	ROM_REGION( 0x1000000, "gfx1", 0 )
	ROM_LOAD( "pls.7",  0x0000000, 0x400000, CRC(04bd1605) SHA1(4871758e57af5132c30137cd6c46f1a3a567b640) )
	ROM_LOAD( "pls.9",  0x0400000, 0x400000, CRC(f4400160) SHA1(206557cd4c73b6b3a04bd35b48de736c7546c5e1) )
	ROM_LOAD( "pls.12", 0x0800000, 0x400000, CRC(edc2e826) SHA1(48d428f928a9805a62bbeaecffcac21aaa76ce77) )
	ROM_LOAD( "pls.15", 0x0c00000, 0x400000, CRC(b0f6b8da) SHA1(7404ec7455adf145919a28907443994f6a5706a1) )

	ROM_REGION( 0x0080000, "gfx2", 0 )
	ROM_LOAD( "surfplnt.u19", 0x0000000, 0x020000, CRC(691bd7a7) SHA1(2ff404b3974a64097372ed15fb5fbbe52c503265) )
	ROM_LOAD( "surfplnt.u20", 0x0020000, 0x020000, CRC(fb293318) SHA1(d255fe3db1b91ec7cc744b0158e70503bca5ceab) )
	ROM_LOAD( "surfplnt.u21", 0x0040000, 0x020000, CRC(b80611fb) SHA1(70d6767ddfb04e94cf2796e3f7090f89fd36fe8c) )
	ROM_LOAD( "surfplnt.u22", 0x0060000, 0x020000, CRC(ccf88f7e) SHA1(c6a3bb9d6cf14a93a36ed20a47b7c068ccd630aa) )
	/* these 4 are copies of the previous 4 */
//  ROM_LOAD( "surfplnt.u27", 0x0000000, 0x020000, CRC(691bd7a7) SHA1(2ff404b3974a64097372ed15fb5fbbe52c503265) )
//  ROM_LOAD( "surfplnt.u28", 0x0020000, 0x020000, CRC(fb293318) SHA1(d255fe3db1b91ec7cc744b0158e70503bca5ceab) )
//  ROM_LOAD( "surfplnt.u29", 0x0040000, 0x020000, CRC(b80611fb) SHA1(70d6767ddfb04e94cf2796e3f7090f89fd36fe8c) )
//  ROM_LOAD( "surfplnt.u30", 0x0060000, 0x020000, CRC(ccf88f7e) SHA1(c6a3bb9d6cf14a93a36ed20a47b7c068ccd630aa) )
ROM_END


ROM_START( surfplnt40 )
	ROM_REGION( 0x200000, "maincpu", 0 )	/* 68000 code */
	ROM_LOAD16_BYTE( "surfpl40.u5",  0x000000, 0x80000, CRC(572e0343) SHA1(badb08a5a495611b5fd2d821d4299348b2c9f308) )
	ROM_LOAD16_BYTE( "surfpl40.u11", 0x000001, 0x80000, CRC(6056edaa) SHA1(9bc2df54d1367b9d58272a8f506e523e74110361) )
	ROM_LOAD16_BYTE( "surfplnt.u8",  0x100000, 0x80000, CRC(aef9e1d0) SHA1(15258e62fbf61e21e7d77aa7a81fdbf842fd4560) )
	ROM_LOAD16_BYTE( "surfplnt.u13", 0x100001, 0x80000, CRC(d9754369) SHA1(0d82569cb925402a9f4634e52f15435112ec4878) )

	ROM_REGION16_LE( 0x400000, "user1", 0 )	/* ADSP-2115 code & data */
	ROM_LOAD( "pls.18", 0x0000000, 0x400000, CRC(a1b64695) SHA1(7487cd51305e30a5b55aada0bae9161fcb3fcd19) )

	ROM_REGION32_LE( 0x800000, "user2", 0 )
	ROM_LOAD32_WORD( "pls.40", 0x000000, 0x400000, CRC(26877ad3) SHA1(2e0c15b0e060e0b3d5b5cdaf1e22b9ec8e1abc9a) )
	ROM_LOAD32_WORD( "pls.37", 0x000002, 0x400000, CRC(75893062) SHA1(81f10243336a309f8cc8532ee9a130ecc35bbcd6) )

	ROM_REGION( 0x1000000, "gfx1", 0 )
	ROM_LOAD( "pls.7",  0x0000000, 0x400000, CRC(04bd1605) SHA1(4871758e57af5132c30137cd6c46f1a3a567b640) )
	ROM_LOAD( "pls.9",  0x0400000, 0x400000, CRC(f4400160) SHA1(206557cd4c73b6b3a04bd35b48de736c7546c5e1) )
	ROM_LOAD( "pls.12", 0x0800000, 0x400000, CRC(edc2e826) SHA1(48d428f928a9805a62bbeaecffcac21aaa76ce77) )
	ROM_LOAD( "pls.15", 0x0c00000, 0x400000, CRC(b0f6b8da) SHA1(7404ec7455adf145919a28907443994f6a5706a1) )

	ROM_REGION( 0x0080000, "gfx2", 0 )
	ROM_LOAD( "surfplnt.u19", 0x0000000, 0x020000, CRC(691bd7a7) SHA1(2ff404b3974a64097372ed15fb5fbbe52c503265) )
	ROM_LOAD( "surfplnt.u20", 0x0020000, 0x020000, CRC(fb293318) SHA1(d255fe3db1b91ec7cc744b0158e70503bca5ceab) )
	ROM_LOAD( "surfplnt.u21", 0x0040000, 0x020000, CRC(b80611fb) SHA1(70d6767ddfb04e94cf2796e3f7090f89fd36fe8c) )
	ROM_LOAD( "surfplnt.u22", 0x0060000, 0x020000, CRC(ccf88f7e) SHA1(c6a3bb9d6cf14a93a36ed20a47b7c068ccd630aa) )
	/* these 4 are copies of the previous 4 */
//  ROM_LOAD( "surfplnt.u27", 0x0000000, 0x020000, CRC(691bd7a7) SHA1(2ff404b3974a64097372ed15fb5fbbe52c503265) )
//  ROM_LOAD( "surfplnt.u28", 0x0020000, 0x020000, CRC(fb293318) SHA1(d255fe3db1b91ec7cc744b0158e70503bca5ceab) )
//  ROM_LOAD( "surfplnt.u29", 0x0040000, 0x020000, CRC(b80611fb) SHA1(70d6767ddfb04e94cf2796e3f7090f89fd36fe8c) )
//  ROM_LOAD( "surfplnt.u30", 0x0060000, 0x020000, CRC(ccf88f7e) SHA1(c6a3bb9d6cf14a93a36ed20a47b7c068ccd630aa) )
ROM_END


ROM_START( radikalb )
	ROM_REGION( 0x200000, "maincpu", 0 )	/* 68020 code */
	ROM_LOAD32_BYTE( "rab.6",  0x000000, 0x80000, CRC(ccac98c5) SHA1(43a30caf9880f48aba79676f9e746fdc6258139d) )
	ROM_LOAD32_BYTE( "rab.12", 0x000001, 0x80000, CRC(26199506) SHA1(1b7b44895aa296eab8061ae85cbb5b0d30119dc7) )
	ROM_LOAD32_BYTE( "rab.14", 0x000002, 0x80000, CRC(4a0ac8cb) SHA1(4883e5eddb833dcd39376be435aa8e8e2ec47ab5) )
	ROM_LOAD32_BYTE( "rab.19", 0x000003, 0x80000, CRC(2631bd61) SHA1(57331ad49e7284b82073f696049de109b7683b03) )

	ROM_REGION16_LE( 0x400000, "user1", 0 )	/* ADSP-2115 code & data */
	ROM_LOAD( "rab.23", 0x0000000, 0x400000, CRC(dcf52520) SHA1(ab54421c182436660d2a56a334c1aa335424644a) )

	ROM_REGION32_LE( 0x800000, "user2", 0 )
	ROM_LOAD32_WORD( "rab.48", 0x000000, 0x400000, CRC(9c56a06a) SHA1(54f12d8b55fa14446c47e31684c92074c4157fe1) )
	ROM_LOAD32_WORD( "rab.45", 0x000002, 0x400000, CRC(7e698584) SHA1(a9423835a126396902c499e9f7df3b68c2ab28a8) )

	ROM_REGION( 0x2000000, "gfx1", 0 )
	ROM_LOAD( "rab.8",  0x0000000, 0x400000, CRC(4fbd4737) SHA1(594438d3edbe00682290986cc631615d7bef67f3) )
	ROM_LOAD( "rab.10", 0x0800000, 0x400000, CRC(870b0ce4) SHA1(75910dca87d2eb3a6b4a28f6e9c63a6b6700de84) )
	ROM_LOAD( "rab.15", 0x1000000, 0x400000, CRC(edb9d409) SHA1(1f8df507e990eee197f2779b45bd8f143d1bd439) )
	ROM_LOAD( "rab.17", 0x1800000, 0x400000, CRC(e120236b) SHA1(37d7996530eda3df0c19bca1cbf26e5ba58b0977) )

	ROM_LOAD( "rab.9",  0x0400000, 0x400000, CRC(9e3e038d) SHA1(4a5f0b3c54c508d7f310f270dbd11bffb2bcfa89) )
	ROM_LOAD( "rab.11", 0x0c00000, 0x400000, CRC(75672271) SHA1(ebbdf089b4a4d5ead7d62555bb5c9a921aaa1c22) )
	ROM_LOAD( "rab.16", 0x1400000, 0x400000, CRC(9d595e46) SHA1(b985332974e1fb0b9d20d521da0d7deceea93a8a) )
	ROM_LOAD( "rab.18", 0x1c00000, 0x400000, CRC(3084bc49) SHA1(9da43482293eeb08ceae67455b2fcd97b6ef5109) )

	ROM_REGION( 0x0080000, "gfx2", 0 )
	ROM_LOAD( "rab.24", 0x0000000, 0x020000, CRC(2984bc1d) SHA1(1f62bdaa86feeff96640e325f8241b9c5f383a44) )
	ROM_LOAD( "rab.25", 0x0020000, 0x020000, CRC(777758e3) SHA1(bd334b1ba46189ac8509eee3a4ab295c121400fd) )
	ROM_LOAD( "rab.26", 0x0040000, 0x020000, CRC(bd9c1b54) SHA1(c9ef679cf7eca9ed315ea62a7ada452bc85f7a6a) )
	ROM_LOAD( "rab.27", 0x0060000, 0x020000, CRC(bbcf6977) SHA1(0282c8ba79c35ed1240711d5812bfb590d151738) )
	/* these 4 are copies of the previous 4 */
//  ROM_LOAD( "rab.32", 0x0000000, 0x020000, CRC(2984bc1d) SHA1(1f62bdaa86feeff96640e325f8241b9c5f383a44) )
//  ROM_LOAD( "rab.33", 0x0020000, 0x020000, CRC(777758e3) SHA1(bd334b1ba46189ac8509eee3a4ab295c121400fd) )
//  ROM_LOAD( "rab.34", 0x0040000, 0x020000, CRC(bd9c1b54) SHA1(c9ef679cf7eca9ed315ea62a7ada452bc85f7a6a) )
//  ROM_LOAD( "rab.35", 0x0060000, 0x020000, CRC(bbcf6977) SHA1(0282c8ba79c35ed1240711d5812bfb590d151738) )
ROM_END



/*************************************
 *
 *  Driver init
 *
 *************************************/

static DRIVER_INIT( gaelco3d )
{
	UINT8 *src, *dst;
	int x, y;

	/* allocate memory */
	gaelco3d_texture_size = memory_region_length(machine, "gfx1");
	gaelco3d_texmask_size = memory_region_length(machine, "gfx2") * 8;
	gaelco3d_texture = auto_alloc_array(machine, UINT8, gaelco3d_texture_size);
	gaelco3d_texmask = auto_alloc_array(machine, UINT8, gaelco3d_texmask_size);

	/* first expand the pixel data */
	src = memory_region(machine, "gfx1");
	dst = gaelco3d_texture;
	for (y = 0; y < gaelco3d_texture_size/4096; y += 2)
		for (x = 0; x < 4096; x += 2)
		{
			dst[(y + 0) * 4096 + (x + 1)] = src[0*gaelco3d_texture_size/4 + (y/2) * 2048 + (x/2)];
			dst[(y + 1) * 4096 + (x + 1)] = src[1*gaelco3d_texture_size/4 + (y/2) * 2048 + (x/2)];
			dst[(y + 0) * 4096 + (x + 0)] = src[2*gaelco3d_texture_size/4 + (y/2) * 2048 + (x/2)];
			dst[(y + 1) * 4096 + (x + 0)] = src[3*gaelco3d_texture_size/4 + (y/2) * 2048 + (x/2)];
		}

	/* then expand the mask data */
	src = memory_region(machine, "gfx2");
	dst = gaelco3d_texmask;
	for (y = 0; y < gaelco3d_texmask_size/4096; y++)
		for (x = 0; x < 4096; x++)
			dst[y * 4096 + x] = (src[(x / 1024) * (gaelco3d_texmask_size/8/4) + (y * 1024 + x % 1024) / 8] >> (x % 8)) & 1;
}



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1996, speedup,    0,        gaelco3d,  speedup,  gaelco3d, ROT0, "Gaelco", "Speed Up (Version 1.20)", GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE )
GAME( 1997, surfplnt,   0,        gaelco3d,  surfplnt, gaelco3d, ROT0, "Gaelco", "Surf Planet (Version 4.1)", GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE)
GAME( 1997, surfplnt40, surfplnt, gaelco3d,  surfplnt, gaelco3d, ROT0, "Gaelco", "Surf Planet (Version 4.0)", GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE)
GAME( 1998, radikalb,   0,        gaelco3d2, radikalb, gaelco3d, ROT0, "Gaelco", "Radikal Bikers (Version 2.02)", GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE)
