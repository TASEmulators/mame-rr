/***************************************************************************

    Taito Qix hardware

    driver by John Butler, Ed Mueller, Aaron Giles

***************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "cpu/m6805/m6805.h"
#include "cpu/m6809/m6809.h"
#include "sound/sn76496.h"
#include "includes/qix.h"



/*************************************
 *
 *  Static function prototypes
 *
 *************************************/

static READ8_DEVICE_HANDLER( qixmcu_coin_r );
static WRITE8_DEVICE_HANDLER( qixmcu_coinctrl_w );
static WRITE8_DEVICE_HANDLER( qixmcu_coin_w );

static WRITE8_DEVICE_HANDLER( qix_coinctl_w );

static WRITE8_DEVICE_HANDLER( slither_76489_0_w );
static WRITE8_DEVICE_HANDLER( slither_76489_1_w );

static READ8_DEVICE_HANDLER( slither_trak_lr_r );
static READ8_DEVICE_HANDLER( slither_trak_ud_r );



/***************************************************************************

    Qix has 6 PIAs on board:

    From the ROM I/O schematic:

    PIA 0 = U11: (mapped to $9400 on the data CPU)
        port A = external input (input_port_0)
        port B = external input (input_port_1) (coin)

    PIA 1 = U20: (mapped to $9800/$9900 on the data CPU)
        port A = external input (input_port_2)
        port B = external input (input_port_3)

    PIA 2 = U30: (mapped to $9c00 on the data CPU)
        port A = external input (input_port_4)
        port B = external output (coin control)


    From the data/sound processor schematic:

    PIA 3 = U20: (mapped to $9000 on the data CPU)
        port A = data CPU to sound CPU communication
        port B = stereo volume control, 2 4-bit values
        CA1 = interrupt signal from sound CPU
        CA2 = interrupt signal to sound CPU
        CB1 = VS input signal (vertical sync)
        CB2 = INV output signal (cocktail flip)
        IRQA = /DINT1 signal
        IRQB = /DINT1 signal

    PIA 4 = U8: (mapped to $4000 on the sound CPU)
        port A = sound CPU to data CPU communication
        port B = DAC value (port B)
        CA1 = interrupt signal from data CPU
        CA2 = interrupt signal to data CPU
        IRQA = /SINT1 signal
        IRQB = /SINT1 signal

    PIA 5 = U7: (never actually used, mapped to $2000 on the sound CPU)
        port A = unused
        port B = sound CPU to TMS5220 communication
        CA1 = interrupt signal from TMS5220
        CA2 = write signal to TMS5220
        CB1 = ready signal from TMS5220
        CB2 = read signal to TMS5220
        IRQA = /SINT2 signal
        IRQB = /SINT2 signal

***************************************************************************/

const pia6821_interface qix_pia_0_intf =
{
	DEVCB_INPUT_PORT("P1"),		/* port A in */
	DEVCB_INPUT_PORT("COIN"),	/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_NULL,		/* port A out */
	DEVCB_NULL,		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_NULL,		/* port CB2 out */
	DEVCB_NULL,		/* IRQA */
	DEVCB_NULL		/* IRQB */
};

const pia6821_interface qix_pia_1_intf =
{
	DEVCB_INPUT_PORT("SPARE"),	/* port A in */
	DEVCB_INPUT_PORT("IN0"),	/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_NULL,		/* port A out */
	DEVCB_NULL,		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_NULL,		/* port CB2 out */
	DEVCB_NULL,		/* IRQA */
	DEVCB_NULL		/* IRQB */
};

const pia6821_interface qix_pia_2_intf =
{
	DEVCB_INPUT_PORT("P2"),				/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_NULL,		/* port A out */
	DEVCB_HANDLER(qix_coinctl_w),		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_NULL,		/* port CB2 out */
	DEVCB_NULL,		/* IRQA */
	DEVCB_NULL		/* IRQB */
};



/***************************************************************************

    Games with an MCU need to handle coins differently, and provide
    communication with the MCU

***************************************************************************/

const pia6821_interface qixmcu_pia_0_intf =
{
	DEVCB_INPUT_PORT("P1"),			/* port A in */
	DEVCB_HANDLER(qixmcu_coin_r),	/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_NULL,		/* port A out */
	DEVCB_HANDLER(qixmcu_coin_w),	/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_NULL,		/* port CB2 out */
	DEVCB_NULL,		/* IRQA */
	DEVCB_NULL		/* IRQB */
};

const pia6821_interface qixmcu_pia_2_intf =
{
	DEVCB_INPUT_PORT("P2"),				/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_NULL,		/* port A out */
	DEVCB_HANDLER(qixmcu_coinctrl_w),	/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_NULL,		/* port CB2 out */
	DEVCB_NULL,		/* IRQA */
	DEVCB_NULL		/* IRQB */
};



/***************************************************************************

    Slither uses 2 SN76489's for sound instead of the 6802+DAC; these
    are accessed via the PIAs.

***************************************************************************/

const pia6821_interface slither_pia_1_intf =
{
	DEVCB_HANDLER(slither_trak_lr_r),	/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_NULL,		/* port A out */
	DEVCB_HANDLER(slither_76489_0_w),		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_NULL,		/* port CB2 out */
	DEVCB_NULL,		/* IRQA */
	DEVCB_NULL		/* IRQB */
};

const pia6821_interface slither_pia_2_intf =
{
	DEVCB_HANDLER(slither_trak_ud_r),	/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_NULL,		/* port A out */
	DEVCB_HANDLER(slither_76489_1_w),		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_NULL,		/* port CB2 out */
	DEVCB_NULL,		/* IRQA */
	DEVCB_NULL		/* IRQB */
};



/*************************************
 *
 *  Machine initialization
 *
 *************************************/

MACHINE_RESET( qix )
{
	qix_state *state = (qix_state *)machine->driver_data;

	/* reset the coin counter register */
	state->coinctrl = 0x00;
}


MACHINE_START( qixmcu )
{
	qix_state *state = (qix_state *)machine->driver_data;

	/* set up save states */
	state_save_register_global_array(machine, state->_68705_port_in);
	state_save_register_global(machine, state->coinctrl);
}



/*************************************
 *
 *  VSYNC change callback
 *
 *************************************/

WRITE_LINE_DEVICE_HANDLER( qix_vsync_changed )
{
	running_device *pia = device->machine->device("sndpia0");
	pia6821_cb1_w(pia, state);
}



/*************************************
 *
 *  Zoo Keeper bankswitching
 *
 *************************************/

WRITE8_HANDLER( zookeep_bankswitch_w )
{
	memory_set_bank(space->machine, "bank1", (data >> 2) & 1);
	/* not necessary, but technically correct */
	qix_palettebank_w(space, offset, data);
}



/*************************************
 *
 *  Data CPU FIRQ generation/ack
 *
 *************************************/

WRITE8_HANDLER( qix_data_firq_w )
{
	cputag_set_input_line(space->machine, "maincpu", M6809_FIRQ_LINE, ASSERT_LINE);
}


WRITE8_HANDLER( qix_data_firq_ack_w )
{
	cputag_set_input_line(space->machine, "maincpu", M6809_FIRQ_LINE, CLEAR_LINE);
}


READ8_HANDLER( qix_data_firq_r )
{
	cputag_set_input_line(space->machine, "maincpu", M6809_FIRQ_LINE, ASSERT_LINE);
	return 0xff;
}


READ8_HANDLER( qix_data_firq_ack_r )
{
	cputag_set_input_line(space->machine, "maincpu", M6809_FIRQ_LINE, CLEAR_LINE);
	return 0xff;
}



/*************************************
 *
 *  Video CPU FIRQ generation/ack
 *
 *************************************/

WRITE8_HANDLER( qix_video_firq_w )
{
	cputag_set_input_line(space->machine, "videocpu", M6809_FIRQ_LINE, ASSERT_LINE);
}


WRITE8_HANDLER( qix_video_firq_ack_w )
{
	cputag_set_input_line(space->machine, "videocpu", M6809_FIRQ_LINE, CLEAR_LINE);
}


READ8_HANDLER( qix_video_firq_r )
{
	cputag_set_input_line(space->machine, "videocpu", M6809_FIRQ_LINE, ASSERT_LINE);
	return 0xff;
}


READ8_HANDLER( qix_video_firq_ack_r )
{
	cputag_set_input_line(space->machine, "videocpu", M6809_FIRQ_LINE, CLEAR_LINE);
	return 0xff;
}



/*************************************
 *
 *  68705 Communication
 *
 *************************************/

READ8_DEVICE_HANDLER( qixmcu_coin_r )
{
	qix_state *state = (qix_state *)device->machine->driver_data;

	logerror("6809:qixmcu_coin_r = %02X\n", state->_68705_port_out[0]);
	return state->_68705_port_out[0];
}


static WRITE8_DEVICE_HANDLER( qixmcu_coin_w )
{
	qix_state *state = (qix_state *)device->machine->driver_data;

	logerror("6809:qixmcu_coin_w = %02X\n", data);
	/* this is a callback called by pia6821_w(), so I don't need to synchronize */
	/* the CPUs - they have already been synchronized by qix_pia_w() */
	state->_68705_port_in[0] = data;
}


static WRITE8_DEVICE_HANDLER( qixmcu_coinctrl_w )
{
	qix_state *state = (qix_state *)device->machine->driver_data;

	/* if (!(data & 0x04)) */
	if (data & 0x04)
	{
		cputag_set_input_line(device->machine, "mcu", M68705_IRQ_LINE, ASSERT_LINE);
		/* temporarily boost the interleave to sync things up */
		/* note: I'm using 50 because 30 is not enough for space dungeon at game over */
		cpuexec_boost_interleave(device->machine, attotime_zero, ATTOTIME_IN_USEC(50));
	}
	else
		cputag_set_input_line(device->machine, "mcu", M68705_IRQ_LINE, CLEAR_LINE);

	/* this is a callback called by pia6821_w(), so I don't need to synchronize */
	/* the CPUs - they have already been synchronized by qix_pia_w() */
	state->coinctrl = data;
	logerror("6809:qixmcu_coinctrl_w = %02X\n", data);
}



/*************************************
 *
 *  68705 Port Inputs
 *
 *************************************/

READ8_HANDLER( qix_68705_portA_r )
{
	qix_state *state = (qix_state *)space->machine->driver_data;

	UINT8 ddr = state->_68705_ddr[0];
	UINT8 out = state->_68705_port_out[0];
	UINT8 in = state->_68705_port_in[0];
	logerror("68705:portA_r = %02X (%02X)\n", (out & ddr) | (in & ~ddr), in);
	return (out & ddr) | (in & ~ddr);
}


READ8_HANDLER( qix_68705_portB_r )
{
	qix_state *state = (qix_state *)space->machine->driver_data;

	UINT8 ddr = state->_68705_ddr[1];
	UINT8 out = state->_68705_port_out[1];
	UINT8 in = (input_port_read(space->machine, "COIN") & 0x0f) | ((input_port_read(space->machine, "COIN") & 0x80) >> 3);
	return (out & ddr) | (in & ~ddr);
}


READ8_HANDLER( qix_68705_portC_r )
{
	qix_state *state = (qix_state *)space->machine->driver_data;

	UINT8 ddr = state->_68705_ddr[2];
	UINT8 out = state->_68705_port_out[2];
	UINT8 in = (state->coinctrl & 0x08) | ((input_port_read(space->machine, "COIN") & 0x70) >> 4);
	return (out & ddr) | (in & ~ddr);
}



/*************************************
 *
 *  68705 Port Outputs
 *
 *************************************/

WRITE8_HANDLER( qix_68705_portA_w )
{
	qix_state *state = (qix_state *)space->machine->driver_data;

	logerror("68705:portA_w = %02X\n", data);
	state->_68705_port_out[0] = data;
}


WRITE8_HANDLER( qix_68705_portB_w )
{
	qix_state *state = (qix_state *)space->machine->driver_data;

	state->_68705_port_out[1] = data;
	coin_lockout_w(space->machine, 0, (~data >> 6) & 1);
	coin_counter_w(space->machine, 0, (data >> 7) & 1);
}


WRITE8_HANDLER( qix_68705_portC_w )
{
	qix_state *state = (qix_state *)space->machine->driver_data;

	state->_68705_port_out[2] = data;
}



/*************************************
 *
 *  Data CPU PIA 0 synchronization
 *
 *************************************/

static TIMER_CALLBACK( pia_w_callback )
{
	running_device *device = (running_device *)ptr;
	pia6821_w(device, param >> 8, param & 0xff);
}


WRITE8_DEVICE_HANDLER( qix_pia_w )
{
	/* make all the CPUs synchronize, and only AFTER that write the command to the PIA */
	/* otherwise the 68705 will miss commands */
	timer_call_after_resynch(device->machine, (void *)device, data | (offset << 8), pia_w_callback);
}



/*************************************
 *
 *  Coin I/O for games without coin CPU
 *
 *************************************/

static WRITE8_DEVICE_HANDLER( qix_coinctl_w )
{
	coin_lockout_w(device->machine, 0, (~data >> 2) & 1);
	coin_counter_w(device->machine, 0, (data >> 1) & 1);
}



/*************************************
 *
 *  Slither SN76489 I/O
 *
 *************************************/

static WRITE8_DEVICE_HANDLER( slither_76489_0_w )
{
	/* write to the sound chip */
	sn76496_w(device->machine->device("sn1"), 0, data);

	/* clock the ready line going back into CB1 */
	pia6821_cb1_w(device, 0);
	pia6821_cb1_w(device, 1);
}


static WRITE8_DEVICE_HANDLER( slither_76489_1_w )
{
	/* write to the sound chip */
	sn76496_w(device->machine->device("sn2"), 0, data);

	/* clock the ready line going back into CB1 */
	pia6821_cb1_w(device, 0);
	pia6821_cb1_w(device, 1);
}



/*************************************
 *
 *  Slither trackball I/O
 *
 *************************************/

static READ8_DEVICE_HANDLER( slither_trak_lr_r )
{
	qix_state *state = (qix_state *)device->machine->driver_data;

	return input_port_read(device->machine, state->flip ? "AN3" : "AN1");
}


static READ8_DEVICE_HANDLER( slither_trak_ud_r )
{
	qix_state *state = (qix_state *)device->machine->driver_data;

	return input_port_read(device->machine, state->flip ? "AN2" : "AN0");
}
