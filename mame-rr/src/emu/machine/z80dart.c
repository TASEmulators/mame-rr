/***************************************************************************

    Z80 DART Dual Asynchronous Receiver/Transmitter emulation

    Copyright (c) 2008, The MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

/*

    TODO:

    - break detection
    - wr0 reset tx interrupt pending
    - wait/ready
    - 1.5 stop bits

*/

#include "emu.h"
#include "z80dart.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"



//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define VERBOSE 0

#define LOG(x) do { if (VERBOSE) logerror x; } while (0)



//**************************************************************************
//  CONSTANTS
//**************************************************************************

enum
{
	STATE_START = 0,
	STATE_DATA,
	STATE_PARITY,
	STATE_STOP,
	STATE_STOP2
};

enum
{
	INT_TRANSMIT = 0,
	INT_EXTERNAL,
	INT_RECEIVE,
	INT_SPECIAL
};

const int Z80DART_RR0_RX_CHAR_AVAILABLE		= 0x01;
const int Z80DART_RR0_INTERRUPT_PENDING		= 0x02;
const int Z80DART_RR0_TX_BUFFER_EMPTY		= 0x04;
const int Z80DART_RR0_DCD					= 0x08;
const int Z80DART_RR0_RI					= 0x10;
const int Z80DART_RR0_CTS					= 0x20;
const int Z80DART_RR0_BREAK					= 0x80; // not supported

const int Z80DART_RR1_ALL_SENT				= 0x01;
const int Z80DART_RR1_PARITY_ERROR			= 0x10;
const int Z80DART_RR1_RX_OVERRUN_ERROR		= 0x20;
const int Z80DART_RR1_FRAMING_ERROR			= 0x40;

const int Z80DART_WR0_REGISTER_MASK			= 0x07;
const int Z80DART_WR0_COMMAND_MASK			= 0x38;
const int Z80DART_WR0_NULL_CODE				= 0x00;
const int Z80DART_WR0_RESET_EXT_STATUS		= 0x10;
const int Z80DART_WR0_CHANNEL_RESET			= 0x18;
const int Z80DART_WR0_ENABLE_INT_NEXT_RX	= 0x20;
const int Z80DART_WR0_RESET_TX_INT			= 0x28; // not supported
const int Z80DART_WR0_ERROR_RESET			= 0x30;
const int Z80DART_WR0_RETURN_FROM_INT		= 0x38; // not supported

const int Z80DART_WR1_EXT_INT_ENABLE		= 0x01;
const int Z80DART_WR1_TX_INT_ENABLE			= 0x02;
const int Z80DART_WR1_STATUS_VECTOR			= 0x04;
const int Z80DART_WR1_RX_INT_ENABLE_MASK	= 0x18;
const int Z80DART_WR1_RX_INT_DISABLE		= 0x00;
const int Z80DART_WR1_RX_INT_FIRST			= 0x08;
const int Z80DART_WR1_RX_INT_ALL_PARITY		= 0x10; // not supported
const int Z80DART_WR1_RX_INT_ALL			= 0x18;
const int Z80DART_WR1_WRDY_ON_RX_TX			= 0x20; // not supported
const int Z80DART_WR1_WRDY_FUNCTION			= 0x40; // not supported
const int Z80DART_WR1_WRDY_ENABLE			= 0x80; // not supported

const int Z80DART_WR3_RX_ENABLE				= 0x01;
const int Z80DART_WR3_AUTO_ENABLES			= 0x20;
const int Z80DART_WR3_RX_WORD_LENGTH_MASK	= 0xc0;
const int Z80DART_WR3_RX_WORD_LENGTH_5		= 0x00;
const int Z80DART_WR3_RX_WORD_LENGTH_7		= 0x40;
const int Z80DART_WR3_RX_WORD_LENGTH_6		= 0x80;
const int Z80DART_WR3_RX_WORD_LENGTH_8		= 0xc0;

const int Z80DART_WR4_PARITY_ENABLE			= 0x01; // not supported
const int Z80DART_WR4_PARITY_EVEN			= 0x02; // not supported
const int Z80DART_WR4_STOP_BITS_MASK		= 0x0c;
const int Z80DART_WR4_STOP_BITS_1			= 0x04;
const int Z80DART_WR4_STOP_BITS_1_5			= 0x08; // not supported
const int Z80DART_WR4_STOP_BITS_2			= 0x0c;
const int Z80DART_WR4_CLOCK_MODE_MASK		= 0xc0;
const int Z80DART_WR4_CLOCK_MODE_X1			= 0x00;
const int Z80DART_WR4_CLOCK_MODE_X16		= 0x40;
const int Z80DART_WR4_CLOCK_MODE_X32		= 0x80;
const int Z80DART_WR4_CLOCK_MODE_X64		= 0xc0;

const int Z80DART_WR5_RTS					= 0x02;
const int Z80DART_WR5_TX_ENABLE				= 0x08;
const int Z80DART_WR5_SEND_BREAK			= 0x10;
const int Z80DART_WR5_TX_WORD_LENGTH_MASK	= 0xc0;
const int Z80DART_WR5_TX_WORD_LENGTH_5		= 0x00;
const int Z80DART_WR5_TX_WORD_LENGTH_7		= 0x40;
const int Z80DART_WR5_TX_WORD_LENGTH_6		= 0x80;
const int Z80DART_WR5_TX_WORD_LENGTH_8		= 0xc0;
const int Z80DART_WR5_DTR					= 0x80;



//**************************************************************************
//  MACROS
//**************************************************************************

#define RXD \
	devcb_call_read_line(&m_in_rxd_func)

#define TXD(_state) \
	devcb_call_write_line(&m_out_txd_func, _state)

#define RTS(_state) \
	devcb_call_write_line(&m_out_rts_func, _state)

#define DTR(_state) \
	devcb_call_write_line(&m_out_dtr_func, _state)



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  z80dart_device_config - constructor
//-------------------------------------------------

z80dart_device_config::z80dart_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
	: device_config(mconfig, static_alloc_device_config, "Zilog Z80 DART", tag, owner, clock),
	  device_config_z80daisy_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  static_alloc_device_config - allocate a new
//  configuration object
//-------------------------------------------------

device_config *z80dart_device_config::static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
{
	return global_alloc(z80dart_device_config(mconfig, tag, owner, clock));
}


//-------------------------------------------------
//  alloc_device - allocate a new device object
//-------------------------------------------------

device_t *z80dart_device_config::alloc_device(running_machine &machine) const
{
	return auto_alloc(&machine, z80dart_device(machine, *this));
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void z80dart_device_config::device_config_complete()
{
	// inherit a copy of the static data
	const z80dart_interface *intf = reinterpret_cast<const z80dart_interface *>(static_config());
	if (intf != NULL)
		*static_cast<z80dart_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		m_rx_clock_a = m_tx_clock_a = m_rx_tx_clock_b = 0;
		memset(&m_in_rxda_func, 0, sizeof(m_in_rxda_func));
		memset(&m_out_txda_func, 0, sizeof(m_out_txda_func));
		memset(&m_out_dtra_func, 0, sizeof(m_out_dtra_func));
		memset(&m_out_rtsa_func, 0, sizeof(m_out_rtsa_func));
		memset(&m_out_wrdya_func, 0, sizeof(m_out_wrdya_func));
		memset(&m_in_rxdb_func, 0, sizeof(m_in_rxdb_func));
		memset(&m_out_txdb_func, 0, sizeof(m_out_txdb_func));
		memset(&m_out_dtrb_func, 0, sizeof(m_out_dtrb_func));
		memset(&m_out_rtsb_func, 0, sizeof(m_out_rtsb_func));
		memset(&m_out_wrdyb_func, 0, sizeof(m_out_wrdyb_func));
		memset(&m_out_int_func, 0, sizeof(m_out_int_func));
	}
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  z80dart_device - constructor
//-------------------------------------------------

z80dart_device::z80dart_device(running_machine &_machine, const z80dart_device_config &config)
	: device_t(_machine, config),
	  device_z80daisy_interface(_machine, config, *this),
	  m_config(config)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void z80dart_device::device_start()
{
	// resolve callbacks
	devcb_resolve_write_line(&m_out_int_func, &m_config.m_out_int_func, this);

	m_channel[Z80DART_CH_A].start(this, Z80DART_CH_A, m_config.m_in_rxda_func, m_config.m_out_txda_func, m_config.m_out_dtra_func, m_config.m_out_rtsa_func, m_config.m_out_wrdya_func);
	m_channel[Z80DART_CH_B].start(this, Z80DART_CH_B, m_config.m_in_rxdb_func, m_config.m_out_txdb_func, m_config.m_out_dtrb_func, m_config.m_out_rtsb_func, m_config.m_out_wrdyb_func);

	if (m_config.m_rx_clock_a != 0)
	{
		// allocate channel A receive timer
		m_rxca_timer = timer_alloc(&m_machine, dart_channel::static_rxca_tick, (void *)&m_channel[Z80DART_CH_A]);
		timer_adjust_periodic(m_rxca_timer, attotime_zero, 0, ATTOTIME_IN_HZ(m_config.m_rx_clock_a));
	}

	if (m_config.m_tx_clock_a != 0)
	{
		// allocate channel A transmit timer
		m_txca_timer = timer_alloc(&m_machine, dart_channel::static_txca_tick, (void *)&m_channel[Z80DART_CH_A]);
		timer_adjust_periodic(m_txca_timer, attotime_zero, 0, ATTOTIME_IN_HZ(m_config.m_tx_clock_a));
	}

	if (m_config.m_rx_tx_clock_b != 0)
	{
		// allocate channel B receive/transmit timer
		m_rxtxcb_timer = timer_alloc(&m_machine, dart_channel::static_rxtxcb_tick, (void *)&m_channel[Z80DART_CH_B]);
		timer_adjust_periodic(m_rxtxcb_timer, attotime_zero, 0, ATTOTIME_IN_HZ(m_config.m_rx_tx_clock_b));
	}

	state_save_register_device_item_array(this, 0, m_int_state);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void z80dart_device::device_reset()
{
	LOG(("Z80DART \"%s\" Reset\n", tag()));

	for (int channel = Z80DART_CH_A; channel <= Z80DART_CH_B; channel++)
	{
		m_channel[channel].reset();
	}

	check_interrupts();
}



//**************************************************************************
//  DAISY CHAIN INTERFACE
//**************************************************************************

//-------------------------------------------------
//  z80daisy_irq_state - get interrupt status
//-------------------------------------------------

int z80dart_device::z80daisy_irq_state()
{
	int state = 0;
	int i;

	LOG(("Z80DART \"%s\" : Interrupt State B:%d%d%d%d A:%d%d%d%d\n", tag(),
				m_int_state[0], m_int_state[1], m_int_state[2], m_int_state[3],
				m_int_state[4], m_int_state[5], m_int_state[6], m_int_state[7]));

	// loop over all interrupt sources
	for (i = 0; i < 8; i++)
	{
		// if we're servicing a request, don't indicate more interrupts
		if (m_int_state[i] & Z80_DAISY_IEO)
		{
			state |= Z80_DAISY_IEO;
			break;
		}
		state |= m_int_state[i];
	}

	LOG(("Z80DART \"%s\" : Interrupt State %u\n", tag(), state));

	return state;
}


//-------------------------------------------------
//  z80daisy_irq_ack - interrupt acknowledge
//-------------------------------------------------

int z80dart_device::z80daisy_irq_ack()
{
	int i;

	LOG(("Z80DART \"%s\" Interrupt Acknowledge\n", tag()));

	// loop over all interrupt sources
	for (i = 0; i < 8; i++)
	{
		// find the first channel with an interrupt requested
		if (m_int_state[i] & Z80_DAISY_INT)
		{
			// clear interrupt, switch to the IEO state, and update the IRQs
			m_int_state[i] = Z80_DAISY_IEO;
			m_channel[Z80DART_CH_A].m_rr[0] &= ~Z80DART_RR0_INTERRUPT_PENDING;
			check_interrupts();

			LOG(("Z80DART \"%s\" : Interrupt Acknowledge Vector %02x\n", tag(), m_channel[Z80DART_CH_B].m_rr[2]));

			return m_channel[Z80DART_CH_B].m_rr[2];
		}
	}

	logerror("z80dart_irq_ack: failed to find an interrupt to ack!\n");

	return m_channel[Z80DART_CH_B].m_rr[2];
}


//-------------------------------------------------
//  z80daisy_irq_reti - return from interrupt
//-------------------------------------------------

void z80dart_device::z80daisy_irq_reti()
{
	int i;

	LOG(("Z80DART \"%s\" Return from Interrupt\n", tag()));

	// loop over all interrupt sources
	for (i = 0; i < 8; i++)
	{
		// find the first channel with an IEO pending
		if (m_int_state[i] & Z80_DAISY_IEO)
		{
			// clear the IEO state and update the IRQs
			m_int_state[i] &= ~Z80_DAISY_IEO;
			check_interrupts();
			return;
		}
	}

	logerror("z80dart_irq_reti: failed to find an interrupt to clear IEO on!\n");
}



//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  check_interrupts - control interrupt line
//-------------------------------------------------

void z80dart_device::check_interrupts()
{
	int state = (z80daisy_irq_state() & Z80_DAISY_INT) ? ASSERT_LINE : CLEAR_LINE;
	devcb_call_write_line(&m_out_int_func, state);
}


//-------------------------------------------------
//  take_interrupt - trigger interrupt
//-------------------------------------------------

void z80dart_device::take_interrupt(int priority)
{
	m_int_state[priority] |= Z80_DAISY_INT;
	m_channel[Z80DART_CH_A].m_rr[0] |= Z80DART_RR0_INTERRUPT_PENDING;

	// check for interrupt
	check_interrupts();
}



//**************************************************************************
//  DART CHANNEL
//**************************************************************************

//-------------------------------------------------
//  dart_channel - constructor
//-------------------------------------------------

z80dart_device::dart_channel::dart_channel()
	: m_rx_shift(0),
	  m_rx_error(0),
	  m_rx_fifo(0),
	  m_rx_clock(0),
	  m_rx_state(0),
	  m_rx_bits(0),
	  m_rx_first(0),
	  m_rx_parity(0),
	  m_rx_break(0),
	  m_rx_rr0_latch(0),
	  m_ri(0),
	  m_cts(0),
	  m_dcd(0),
	  m_tx_data(0),
	  m_tx_shift(0),
	  m_tx_clock(0),
	  m_tx_state(0),
	  m_tx_bits(0),
	  m_tx_parity(0),
	  m_dtr(0),
	  m_rts(0)
{
	memset(&m_in_rxd_func, 0, sizeof(m_in_rxd_func));
	memset(&m_out_txd_func, 0, sizeof(m_out_txd_func));
	memset(&m_out_dtr_func, 0, sizeof(m_out_dtr_func));
	memset(&m_out_rts_func, 0, sizeof(m_out_rts_func));
	memset(&m_out_wrdy_func, 0, sizeof(m_out_wrdy_func));
	memset(&m_rr, 0, sizeof(m_rr));
	memset(&m_wr, 0, sizeof(m_wr));
	memset(&m_rx_data_fifo, 0, sizeof(m_rx_data_fifo));
	memset(&m_rx_error_fifo, 0, sizeof(m_rx_error_fifo));
}


//-------------------------------------------------
//  start - channel startup
//-------------------------------------------------

void z80dart_device::dart_channel::start(z80dart_device *device, int index, const devcb_read_line &in_rxd, const devcb_write_line &out_txd, const devcb_write_line &out_dtr, const devcb_write_line &out_rts, const devcb_write_line &out_wrdy)
{
	m_index = index;
	m_device = device;

	devcb_resolve_read_line(&m_in_rxd_func, &in_rxd, m_device);
	devcb_resolve_write_line(&m_out_txd_func, &out_txd, m_device);
	devcb_resolve_write_line(&m_out_dtr_func, &out_dtr, m_device);
	devcb_resolve_write_line(&m_out_rts_func, &out_rts, m_device);
	devcb_resolve_write_line(&m_out_wrdy_func, &out_wrdy, m_device);

	state_save_register_device_item_array(m_device, m_index, m_rr);
	state_save_register_device_item_array(m_device, m_index, m_wr);
	state_save_register_device_item_array(m_device, m_index, m_rx_data_fifo);
	state_save_register_device_item_array(m_device, m_index, m_rx_error_fifo);
	state_save_register_device_item(m_device, m_index, m_rx_shift);
	state_save_register_device_item(m_device, m_index, m_rx_error);
	state_save_register_device_item(m_device, m_index, m_rx_fifo);
	state_save_register_device_item(m_device, m_index, m_rx_clock);
	state_save_register_device_item(m_device, m_index, m_rx_state);
	state_save_register_device_item(m_device, m_index, m_rx_bits);
	state_save_register_device_item(m_device, m_index, m_rx_first);
	state_save_register_device_item(m_device, m_index, m_rx_parity);
	state_save_register_device_item(m_device, m_index, m_rx_break);
	state_save_register_device_item(m_device, m_index, m_rx_rr0_latch);
	state_save_register_device_item(m_device, m_index, m_ri);
	state_save_register_device_item(m_device, m_index, m_cts);
	state_save_register_device_item(m_device, m_index, m_dcd);
	state_save_register_device_item(m_device, m_index, m_tx_data);
	state_save_register_device_item(m_device, m_index, m_tx_shift);
	state_save_register_device_item(m_device, m_index, m_tx_clock);
	state_save_register_device_item(m_device, m_index, m_tx_state);
	state_save_register_device_item(m_device, m_index, m_tx_bits);
	state_save_register_device_item(m_device, m_index, m_tx_parity);
	state_save_register_device_item(m_device, m_index, m_dtr);
	state_save_register_device_item(m_device, m_index, m_rts);
}


//-------------------------------------------------
//  take_interrupt - trigger interrupt
//-------------------------------------------------

void z80dart_device::dart_channel::take_interrupt(int level)
{
	UINT8 vector = m_wr[2];
	int priority = (m_index << 2) | level;

	LOG(("Z80DART \"%s\" Channel %c : Interrupt Request %u\n", m_device->tag(), 'A' + m_index, level));

	if ((m_index == Z80DART_CH_B) && (m_wr[1] & Z80DART_WR1_STATUS_VECTOR))
	{
		// status affects vector
		vector = (m_wr[2] & 0xf1) | (!m_index << 3) | (level << 1);
	}

	// update vector register
	m_rr[2] = vector;

	// trigger interrupt
	m_device->take_interrupt(priority);
}


//-------------------------------------------------
//  get_clock_mode - get clock divisor
//-------------------------------------------------

int z80dart_device::dart_channel::get_clock_mode()
{
	int clocks = 1;

	switch (m_wr[4] & Z80DART_WR4_CLOCK_MODE_MASK)
	{
	case Z80DART_WR4_CLOCK_MODE_X1:		clocks = 1;		break;
	case Z80DART_WR4_CLOCK_MODE_X16:	clocks = 16;	break;
	case Z80DART_WR4_CLOCK_MODE_X32:	clocks = 32;	break;
	case Z80DART_WR4_CLOCK_MODE_X64:	clocks = 64;	break;
	}

	return clocks;
}


//-------------------------------------------------
//  get_stop_bits - get number of stop bits
//-------------------------------------------------

float z80dart_device::dart_channel::get_stop_bits()
{
	float bits = 1;

	switch (m_wr[4] & Z80DART_WR4_STOP_BITS_MASK)
	{
	case Z80DART_WR4_STOP_BITS_1:		bits = 1;		break;
	case Z80DART_WR4_STOP_BITS_1_5:		bits = 1.5;		break;
	case Z80DART_WR4_STOP_BITS_2:		bits = 2;		break;
	}

	return bits;
}


//-------------------------------------------------
//  get_rx_word_length - get receive word length
//-------------------------------------------------

int z80dart_device::dart_channel::get_rx_word_length()
{
	int bits = 5;

	switch (m_wr[3] & Z80DART_WR3_RX_WORD_LENGTH_MASK)
	{
	case Z80DART_WR3_RX_WORD_LENGTH_5:	bits = 5;		break;
	case Z80DART_WR3_RX_WORD_LENGTH_6:	bits = 6;		break;
	case Z80DART_WR3_RX_WORD_LENGTH_7:	bits = 7;		break;
	case Z80DART_WR3_RX_WORD_LENGTH_8:	bits = 8;		break;
	}

	return bits;
}


//-------------------------------------------------
//  get_tx_word_length - get transmit word length
//-------------------------------------------------

int z80dart_device::dart_channel::get_tx_word_length()
{
	int bits = 5;

	switch (m_wr[5] & Z80DART_WR5_TX_WORD_LENGTH_MASK)
	{
	case Z80DART_WR5_TX_WORD_LENGTH_5:	bits = 5;	break;
	case Z80DART_WR5_TX_WORD_LENGTH_6:	bits = 6;	break;
	case Z80DART_WR5_TX_WORD_LENGTH_7:	bits = 7;	break;
	case Z80DART_WR5_TX_WORD_LENGTH_8:	bits = 8;	break;
	}

	return bits;
}


//-------------------------------------------------
//  reset - reset channel status
//-------------------------------------------------

void z80dart_device::dart_channel::reset()
{
	// disable receiver
	m_wr[3] &= ~Z80DART_WR3_RX_ENABLE;
	m_rx_state = STATE_START;

	// disable transmitter
	m_wr[5] &= ~Z80DART_WR5_TX_ENABLE;
	m_tx_state = STATE_START;

	// reset external lines
	RTS(1);
	DTR(1);

	if (m_index == Z80DART_CH_A)
	{
		// reset interrupt logic
		int i;

		for (i = 0; i < 8; i++)
		{
			m_device->m_int_state[i] = 0;
		}

		m_device->check_interrupts();
	}
}


//-------------------------------------------------
//  detect_start_bit - detect start bit
//-------------------------------------------------

int z80dart_device::dart_channel::detect_start_bit()
{
	if (!(m_wr[3] & Z80DART_WR3_RX_ENABLE)) return 0;

	return !RXD;
}


//-------------------------------------------------
//  shift_data_in - shift in serial data
//-------------------------------------------------

void z80dart_device::dart_channel::shift_data_in()
{
	if (m_rx_bits < 8)
	{
		int rxd = RXD;

		m_rx_shift >>= 1;
		m_rx_shift = (rxd << 7) | (m_rx_shift & 0x7f);
		m_rx_parity ^= rxd;
		m_rx_bits++;
	}
}


//-------------------------------------------------
//  character_completed - check if complete
//  data word has been transferred
//-------------------------------------------------

bool z80dart_device::dart_channel::character_completed()
{
	return m_rx_bits == get_rx_word_length();
}


//-------------------------------------------------
//  detect_parity_error - detect parity error
//-------------------------------------------------

void z80dart_device::dart_channel::detect_parity_error()
{
	int parity = (m_wr[1] & Z80DART_WR4_PARITY_EVEN) ? 1 : 0;

	if (RXD != (m_rx_parity ^ parity))
	{
		// parity error detected
		m_rx_error |= Z80DART_RR1_PARITY_ERROR;

		switch (m_wr[1] & Z80DART_WR1_RX_INT_ENABLE_MASK)
		{
		case Z80DART_WR1_RX_INT_FIRST:
			if (!m_rx_first)
			{
				take_interrupt(INT_SPECIAL);
			}
			break;

		case Z80DART_WR1_RX_INT_ALL_PARITY:
			take_interrupt(INT_SPECIAL);
			break;

		case Z80DART_WR1_RX_INT_ALL:
			take_interrupt(INT_RECEIVE);
			break;
		}
	}
}


//-------------------------------------------------
//  detect_framing_error - detect framing error
//-------------------------------------------------

void z80dart_device::dart_channel::detect_framing_error()
{
	if (!RXD)
	{
		// framing error detected
		m_rx_error |= Z80DART_RR1_FRAMING_ERROR;

		switch (m_wr[1] & Z80DART_WR1_RX_INT_ENABLE_MASK)
		{
		case Z80DART_WR1_RX_INT_FIRST:
			if (!m_rx_first)
			{
				take_interrupt(INT_SPECIAL);
			}
			break;

		case Z80DART_WR1_RX_INT_ALL_PARITY:
		case Z80DART_WR1_RX_INT_ALL:
			take_interrupt(INT_SPECIAL);
			break;
		}
	}
}


//-------------------------------------------------
//  receive - receive serial data
//-------------------------------------------------

void z80dart_device::dart_channel::receive()
{
	float stop_bits = get_stop_bits();

	switch (m_rx_state)
	{
	case STATE_START:
		// check for start bit
		if (detect_start_bit())
		{
			// start bit detected
			m_rx_shift = 0;
			m_rx_error = 0;
			m_rx_bits = 0;
			m_rx_parity = 0;

			// next bit is a data bit
			m_rx_state = STATE_DATA;
		}
		break;

	case STATE_DATA:
		// shift bit into shift register
		shift_data_in();

		if (character_completed())
		{
			// all data bits received
			if (m_wr[4] & Z80DART_WR4_PARITY_ENABLE)
			{
				// next bit is the parity bit
				m_rx_state = STATE_PARITY;
			}
			else
			{
				// next bit is a STOP bit
				if (stop_bits == 1)
					m_rx_state = STATE_STOP2;
				else
					m_rx_state = STATE_STOP;
			}
		}
		break;

	case STATE_PARITY:
		// shift bit into shift register
		shift_data_in();

		// check for parity error
		detect_parity_error();

		// next bit is a STOP bit
		if (stop_bits == 1)
			m_rx_state = STATE_STOP2;
		else
			m_rx_state = STATE_STOP;
		break;

	case STATE_STOP:
		// shift bit into shift register
		shift_data_in();

		// check for framing error
		detect_framing_error();

		// next bit is the second STOP bit
		m_rx_state = STATE_STOP2;
		break;

	case STATE_STOP2:
		// shift bit into shift register
		shift_data_in();

		// check for framing error
		detect_framing_error();

		// store data into FIFO
		receive_data(m_rx_shift);

		// next bit is the START bit
		m_rx_state = STATE_START;
		break;
	}
}


//-------------------------------------------------
//  transmit - transmit serial data
//-------------------------------------------------

void z80dart_device::dart_channel::transmit()
{
	int word_length = get_tx_word_length();
	float stop_bits = get_stop_bits();

	switch (m_tx_state)
	{
	case STATE_START:
		if ((m_wr[5] & Z80DART_WR5_TX_ENABLE) && !(m_rr[0] & Z80DART_RR0_TX_BUFFER_EMPTY))
		{
			// transmit start bit
			TXD(0);

			m_tx_bits = 0;
			m_tx_shift = m_tx_data;

			// empty transmit buffer
			m_rr[0] |= Z80DART_RR0_TX_BUFFER_EMPTY;

			if (m_wr[1] & Z80DART_WR1_TX_INT_ENABLE)
				take_interrupt(INT_TRANSMIT);

			m_tx_state = STATE_DATA;
		}
		else if (m_wr[5] & Z80DART_WR5_SEND_BREAK)
		{
			// transmit break
			TXD(0);
		}
		else
		{
			// transmit marking line
			TXD(1);
		}

		break;

	case STATE_DATA:
		// transmit data bit
		TXD(BIT(m_tx_shift, 0));

		// shift data
		m_tx_shift >>= 1;
		m_tx_bits++;

		if (m_rx_bits == word_length)
		{
			if (m_wr[4] & Z80DART_WR4_PARITY_ENABLE)
				m_tx_state = STATE_PARITY;
			else
			{
				if (stop_bits == 1)
					m_tx_state = STATE_STOP2;
				else
					m_tx_state = STATE_STOP;
			}
		}
		break;

	case STATE_PARITY:
		// TODO: calculate parity
		if (stop_bits == 1)
			m_tx_state = STATE_STOP2;
		else
			m_tx_state = STATE_STOP;
		break;

	case STATE_STOP:
		// transmit stop bit
		TXD(1);

		m_tx_state = STATE_STOP2;
		break;

	case STATE_STOP2:
		// transmit stop bit
		TXD(1);

		// if transmit buffer is empty
		if (m_rr[0] & Z80DART_RR0_TX_BUFFER_EMPTY)
		{
			// then all characters have been sent
			m_rr[1] |= Z80DART_RR1_ALL_SENT;

			// when the RTS bit is reset, the _RTS output goes high after the transmitter empties
			if (!m_rts)
				RTS(1);
		}

		m_tx_state = STATE_START;
		break;
	}
}


//-------------------------------------------------
//  control_read - read control register
//-------------------------------------------------

UINT8 z80dart_device::dart_channel::control_read()
{
	UINT8 data = 0;

	int reg = m_wr[0] & Z80DART_WR0_REGISTER_MASK;

	switch (reg)
	{
	case 0:
	case 1:
		data = m_rr[reg];
		break;

	case 2:
		// channel B only
		if (m_index == Z80DART_CH_B)
			data = m_rr[reg];
		break;
	}

	LOG(("Z80DART \"%s\" Channel %c : Control Register Read '%02x'\n", m_device->tag(), 'A' + m_index, data));

	return data;
}


//-------------------------------------------------
//  control_write - write control register
//-------------------------------------------------

void z80dart_device::dart_channel::control_write(UINT8 data)
{
	int reg = m_wr[0] & Z80DART_WR0_REGISTER_MASK;

	LOG(("Z80DART \"%s\" Channel %c : Control Register Write '%02x'\n", m_device->tag(), 'A' + m_index, data));

	// write data to selected register
	m_wr[reg] = data;

	if (reg != 0)
	{
		// mask out register index
		m_wr[0] &= ~Z80DART_WR0_REGISTER_MASK;
	}

	switch (reg)
	{
	case 0:
		switch (data & Z80DART_WR0_COMMAND_MASK)
		{
		case Z80DART_WR0_RESET_EXT_STATUS:
			// reset external/status interrupt
			m_rr[0] &= ~(Z80DART_RR0_DCD | Z80DART_RR0_RI | Z80DART_RR0_CTS | Z80DART_RR0_BREAK);

			if (!m_dcd) m_rr[0] |= Z80DART_RR0_DCD;
			if (m_ri) m_rr[0] |= Z80DART_RR0_RI;
			if (m_cts) m_rr[0] |= Z80DART_RR0_CTS;

			m_rx_rr0_latch = 0;

			LOG(("Z80DART \"%s\" Channel %c : Reset External/Status Interrupt\n", m_device->tag(), 'A' + m_index));
			break;

		case Z80DART_WR0_CHANNEL_RESET:
			// channel reset
			LOG(("Z80DART \"%s\" Channel %c : Channel Reset\n", m_device->tag(), 'A' + m_index));
			reset();
			break;

		case Z80DART_WR0_ENABLE_INT_NEXT_RX:
			// enable interrupt on next receive character
			LOG(("Z80DART \"%s\" Channel %c : Enable Interrupt on Next Received Character\n", m_device->tag(), 'A' + m_index));
			m_rx_first = 1;
			break;

		case Z80DART_WR0_RESET_TX_INT:
			// reset transmitter interrupt pending
			LOG(("Z80DART \"%s\" Channel %c : Reset Transmitter Interrupt Pending\n", m_device->tag(), 'A' + m_index));
			break;

		case Z80DART_WR0_ERROR_RESET:
			// error reset
			LOG(("Z80DART \"%s\" Channel %c : Error Reset\n", m_device->tag(), 'A' + m_index));
			m_rr[1] &= ~(Z80DART_RR1_FRAMING_ERROR | Z80DART_RR1_RX_OVERRUN_ERROR | Z80DART_RR1_PARITY_ERROR);
			break;

		case Z80DART_WR0_RETURN_FROM_INT:
			// return from interrupt
			LOG(("Z80DART \"%s\" Channel %c : Return from Interrupt\n", m_device->tag(), 'A' + m_index));
			m_device->z80daisy_irq_reti();
			break;
		}
		break;

	case 1:
		LOG(("Z80DART \"%s\" Channel %c : External Interrupt Enable %u\n", m_device->tag(), 'A' + m_index, (data & Z80DART_WR1_EXT_INT_ENABLE) ? 1 : 0));
		LOG(("Z80DART \"%s\" Channel %c : Transmit Interrupt Enable %u\n", m_device->tag(), 'A' + m_index, (data & Z80DART_WR1_TX_INT_ENABLE) ? 1 : 0));
		LOG(("Z80DART \"%s\" Channel %c : Status Affects Vector %u\n", m_device->tag(), 'A' + m_index, (data & Z80DART_WR1_STATUS_VECTOR) ? 1 : 0));
		LOG(("Z80DART \"%s\" Channel %c : Wait/Ready Enable %u\n", m_device->tag(), 'A' + m_index, (data & Z80DART_WR1_WRDY_ENABLE) ? 1 : 0));

		switch (data & Z80DART_WR1_RX_INT_ENABLE_MASK)
		{
		case Z80DART_WR1_RX_INT_DISABLE:
			LOG(("Z80DART \"%s\" Channel %c : Receiver Interrupt Disabled\n", m_device->tag(), 'A' + m_index));
			break;

		case Z80DART_WR1_RX_INT_FIRST:
			LOG(("Z80DART \"%s\" Channel %c : Receiver Interrupt on First Character\n", m_device->tag(), 'A' + m_index));
			break;

		case Z80DART_WR1_RX_INT_ALL_PARITY:
			LOG(("Z80DART \"%s\" Channel %c : Receiver Interrupt on All Characters, Parity Affects Vector\n", m_device->tag(), 'A' + m_index));
			break;

		case Z80DART_WR1_RX_INT_ALL:
			LOG(("Z80DART \"%s\" Channel %c : Receiver Interrupt on All Characters\n", m_device->tag(), 'A' + m_index));
			break;
		}

		m_device->check_interrupts();
		break;

	case 2:
		// interrupt vector
		m_device->check_interrupts();
		LOG(("Z80DART \"%s\" Channel %c : Interrupt Vector %02x\n", m_device->tag(), 'A' + m_index, data));
		break;

	case 3:
		LOG(("Z80DART \"%s\" Channel %c : Receiver Enable %u\n", m_device->tag(), 'A' + m_index, (data & Z80DART_WR3_RX_ENABLE) ? 1 : 0));
		LOG(("Z80DART \"%s\" Channel %c : Auto Enables %u\n", m_device->tag(), 'A' + m_index, (data & Z80DART_WR3_AUTO_ENABLES) ? 1 : 0));
		LOG(("Z80DART \"%s\" Channel %c : Receiver Bits/Character %u\n", m_device->tag(), 'A' + m_index, get_rx_word_length()));
		break;

	case 4:
		LOG(("Z80DART \"%s\" Channel %c : Parity Enable %u\n", m_device->tag(), 'A' + m_index, (data & Z80DART_WR4_PARITY_ENABLE) ? 1 : 0));
		LOG(("Z80DART \"%s\" Channel %c : Parity %s\n", m_device->tag(), 'A' + m_index, (data & Z80DART_WR4_PARITY_EVEN) ? "Even" : "Odd"));
		LOG(("Z80DART \"%s\" Channel %c : Stop Bits %f\n", m_device->tag(), 'A' + m_index, get_stop_bits()));
		LOG(("Z80DART \"%s\" Channel %c : Clock Mode %uX\n", m_device->tag(), 'A' + m_index, get_clock_mode()));
		break;

	case 5:
		LOG(("Z80DART \"%s\" Channel %c : Transmitter Enable %u\n", m_device->tag(), 'A' + m_index, (data & Z80DART_WR5_TX_ENABLE) ? 1 : 0));
		LOG(("Z80DART \"%s\" Channel %c : Transmitter Bits/Character %u\n", m_device->tag(), 'A' + m_index, get_tx_word_length()));
		LOG(("Z80DART \"%s\" Channel %c : Send Break %u\n", m_device->tag(), 'A' + m_index, (data & Z80DART_WR5_SEND_BREAK) ? 1 : 0));
		LOG(("Z80DART \"%s\" Channel %c : Request to Send %u\n", m_device->tag(), 'A' + m_index, (data & Z80DART_WR5_RTS) ? 1 : 0));
		LOG(("Z80DART \"%s\" Channel %c : Data Terminal Ready %u\n", m_device->tag(), 'A' + m_index, (data & Z80DART_WR5_DTR) ? 1 : 0));

		if (data & Z80DART_WR5_RTS)
		{
			// when the RTS bit is set, the _RTS output goes low
			RTS(0);

			m_rts = 1;
		}
		else
		{
			// when the RTS bit is reset, the _RTS output goes high after the transmitter empties
			m_rts = 0;
		}

		// data terminal ready output follows the state programmed into the DTR bit*/
		m_dtr = (data & Z80DART_WR5_DTR) ? 0 : 1;
		DTR(m_dtr);
		break;
	}
}


//-------------------------------------------------
//  data_read - read data register
//-------------------------------------------------

UINT8 z80dart_device::dart_channel::data_read()
{
	UINT8 data = 0;

	if (m_rx_fifo >= 0)
	{
		// load data from the FIFO
		data = m_rx_data_fifo[m_rx_fifo];

		// load error status from the FIFO, retain overrun and parity errors
		m_rr[1] = (m_rr[1] & (Z80DART_RR1_RX_OVERRUN_ERROR | Z80DART_RR1_PARITY_ERROR)) | m_rx_error_fifo[m_rx_fifo];

		// decrease FIFO pointer
		m_rx_fifo--;

		if (m_rx_fifo < 0)
		{
			// no more characters available in the FIFO
			m_rr[0] &= ~ Z80DART_RR0_RX_CHAR_AVAILABLE;
		}
	}

	LOG(("Z80DART \"%s\" Channel %c : Data Register Read '%02x'\n", m_device->tag(), 'A' + m_index, data));

	return data;
}


//-------------------------------------------------
//  data_write - write data register
//-------------------------------------------------

void z80dart_device::dart_channel::data_write(UINT8 data)
{
	m_tx_data = data;

	m_rr[0] &= ~Z80DART_RR0_TX_BUFFER_EMPTY;
	m_rr[1] &= ~Z80DART_RR1_ALL_SENT;

	LOG(("Z80DART \"%s\" Channel %c : Data Register Write '%02x'\n", m_device->tag(), 'A' + m_index, data));
}


//-------------------------------------------------
//  receive_data - receive data word
//-------------------------------------------------

void z80dart_device::dart_channel::receive_data(UINT8 data)
{
	LOG(("Z80DART \"%s\" Channel %c : Receive Data Byte '%02x'\n", m_device->tag(), 'A' + m_index, data));

	if (m_rx_fifo == 2)
	{
		// receive overrun error detected
		m_rx_error |= Z80DART_RR1_RX_OVERRUN_ERROR;

		switch (m_wr[1] & Z80DART_WR1_RX_INT_ENABLE_MASK)
		{
		case Z80DART_WR1_RX_INT_FIRST:
			if (!m_rx_first)
			{
				take_interrupt(INT_SPECIAL);
			}
			break;

		case Z80DART_WR1_RX_INT_ALL_PARITY:
		case Z80DART_WR1_RX_INT_ALL:
			take_interrupt(INT_SPECIAL);
			break;
		}
	}
	else
	{
		m_rx_fifo++;
	}

	// store received character and error status into FIFO
	m_rx_data_fifo[m_rx_fifo] = data;
	m_rx_error_fifo[m_rx_fifo] = m_rx_error;

	m_rr[0] |= Z80DART_RR0_RX_CHAR_AVAILABLE;

	// receive interrupt
	switch (m_wr[1] & Z80DART_WR1_RX_INT_ENABLE_MASK)
	{
	case Z80DART_WR1_RX_INT_FIRST:
		if (m_rx_first)
		{
			take_interrupt(INT_RECEIVE);

			m_rx_first = 0;
		}
		break;

	case Z80DART_WR1_RX_INT_ALL_PARITY:
	case Z80DART_WR1_RX_INT_ALL:
		take_interrupt(INT_RECEIVE);
		break;
	}
}


//-------------------------------------------------
//  cts_w - clear to send handler
//-------------------------------------------------

void z80dart_device::dart_channel::cts_w(int state)
{
	LOG(("Z80DART \"%s\" Channel %c : CTS %u\n", m_device->tag(), 'A' + m_index, state));

	if (m_cts != state)
	{
		// enable transmitter if in auto enables mode
		if (!state)
			if (m_wr[3] & Z80DART_WR3_AUTO_ENABLES)
				m_wr[5] |= Z80DART_WR5_TX_ENABLE;

		// set clear to send
		m_cts = state;

		if (!m_rx_rr0_latch)
		{
			if (!m_cts)
				m_rr[0] |= Z80DART_RR0_CTS;
			else
				m_rr[0] &= ~Z80DART_RR0_CTS;

			// trigger interrupt
			if (m_wr[1] & Z80DART_WR1_EXT_INT_ENABLE)
			{
				// trigger interrupt
				take_interrupt(INT_EXTERNAL);

				// latch read register 0
				m_rx_rr0_latch = 1;
			}
		}
	}
}


//-------------------------------------------------
//  dcd_w - data carrier detected handler
//-------------------------------------------------

void z80dart_device::dart_channel::dcd_w(int state)
{
	LOG(("Z80DART \"%s\" Channel %c : DCD %u\n", m_device->tag(), 'A' + m_index, state));

	if (m_dcd != state)
	{
		// enable receiver if in auto enables mode
		if (!state)
			if (m_wr[3] & Z80DART_WR3_AUTO_ENABLES)
				m_wr[3] |= Z80DART_WR3_RX_ENABLE;

		// set data carrier detect
		m_dcd = state;

		if (!m_rx_rr0_latch)
		{
			if (m_dcd)
				m_rr[0] |= Z80DART_RR0_DCD;
			else
				m_rr[0] &= ~Z80DART_RR0_DCD;

			if (m_wr[1] & Z80DART_WR1_EXT_INT_ENABLE)
			{
				// trigger interrupt
				take_interrupt(INT_EXTERNAL);

				// latch read register 0
				m_rx_rr0_latch = 1;
			}
		}
	}
}


//-------------------------------------------------
//  ri_w - ring indicator handler
//-------------------------------------------------

void z80dart_device::dart_channel::ri_w(int state)
{
	LOG(("Z80DART \"%s\" Channel %c : RI %u\n", m_device->tag(), 'A' + m_index, state));

	if (m_ri != state)
	{
		// set ring indicator state
		m_ri = state;

		if (!m_rx_rr0_latch)
		{
			if (m_ri)
				m_rr[0] |= Z80DART_RR0_RI;
			else
				m_rr[0] &= ~Z80DART_RR0_RI;

			if (m_wr[1] & Z80DART_WR1_EXT_INT_ENABLE)
			{
				// trigger interrupt
				take_interrupt(INT_EXTERNAL);

				// latch read register 0
				m_rx_rr0_latch = 1;
			}
		}
	}
}


//-------------------------------------------------
//  rx_w - receive clock
//-------------------------------------------------

void z80dart_device::dart_channel::rx_w(int state)
{
	int clocks = get_clock_mode();

	if (!state) return;

	LOG(("Z80DART \"%s\" Channel %c : Receiver Clock Pulse\n", m_device->tag(), m_index + 'A'));

	m_rx_clock++;
	if (m_rx_clock == clocks)
	{
		m_rx_clock = 0;
		receive();
	}
}


//-------------------------------------------------
//  tx_w - transmit clock
//-------------------------------------------------

void z80dart_device::dart_channel::tx_w(int state)
{
	int clocks = get_clock_mode();

	if (!state) return;

	LOG(("Z80DART \"%s\" Channel %c : Transmitter Clock Pulse\n", m_device->tag(), m_index + 'A'));

	m_tx_clock++;
	if (m_tx_clock == clocks)
	{
		m_tx_clock = 0;
		transmit();
	}
}



//**************************************************************************
//  GLOBAL STUBS
//**************************************************************************

READ8_DEVICE_HANDLER( z80dart_c_r ) { return downcast<z80dart_device *>(device)->control_read(offset & 1); }
READ8_DEVICE_HANDLER( z80dart_d_r ) { return downcast<z80dart_device *>(device)->data_read(offset & 1); }

WRITE8_DEVICE_HANDLER( z80dart_c_w ) { downcast<z80dart_device *>(device)->control_write(offset & 1, data); }
WRITE8_DEVICE_HANDLER( z80dart_d_w ) { downcast<z80dart_device *>(device)->data_write(offset & 1, data); }

WRITE_LINE_DEVICE_HANDLER( z80dart_ctsa_w ) { downcast<z80dart_device *>(device)->cts_w(Z80DART_CH_A, state); }
WRITE_LINE_DEVICE_HANDLER( z80dart_ctsb_w ) { downcast<z80dart_device *>(device)->cts_w(Z80DART_CH_B, state); }
WRITE_LINE_DEVICE_HANDLER( z80dart_dcda_w ) { downcast<z80dart_device *>(device)->dcd_w(Z80DART_CH_A, state); }
WRITE_LINE_DEVICE_HANDLER( z80dart_dcdb_w ) { downcast<z80dart_device *>(device)->dcd_w(Z80DART_CH_B, state); }
WRITE_LINE_DEVICE_HANDLER( z80dart_ria_w ) { downcast<z80dart_device *>(device)->ri_w(Z80DART_CH_A, state); }
WRITE_LINE_DEVICE_HANDLER( z80dart_rib_w ) { downcast<z80dart_device *>(device)->ri_w(Z80DART_CH_B, state); }

WRITE_LINE_DEVICE_HANDLER( z80dart_rxca_w ) { downcast<z80dart_device *>(device)->rx_w(Z80DART_CH_A, state); }
WRITE_LINE_DEVICE_HANDLER( z80dart_txca_w ) { downcast<z80dart_device *>(device)->tx_w(Z80DART_CH_A, state); }
WRITE_LINE_DEVICE_HANDLER( z80dart_rxtxcb_w ) { downcast<z80dart_device *>(device)->rx_w(Z80DART_CH_B, state); downcast<z80dart_device *>(device)->tx_w(Z80DART_CH_B, state); }

READ8_DEVICE_HANDLER( z80dart_cd_ba_r )
{
	return (offset & 2) ? z80dart_c_r(device, offset & 1) : z80dart_d_r(device, offset & 1);
}

WRITE8_DEVICE_HANDLER( z80dart_cd_ba_w )
{
	if (offset & 2)
		z80dart_c_w(device, offset & 1, data);
	else
		z80dart_d_w(device, offset & 1, data);
}

READ8_DEVICE_HANDLER( z80dart_ba_cd_r )
{
	int channel = BIT(offset, 1);

	return (offset & 1) ? z80dart_c_r(device, channel) : z80dart_d_r(device, channel);
}

WRITE8_DEVICE_HANDLER( z80dart_ba_cd_w )
{
	int channel = BIT(offset, 1);

	if (offset & 1)
		z80dart_c_w(device, channel, data);
	else
		z80dart_d_w(device, channel, data);
}

const device_type Z80DART = z80dart_device_config::static_alloc_device_config;
