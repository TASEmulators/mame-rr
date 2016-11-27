/***************************************************************************

    Mostek MK3801 Serial Timer Interrupt Controller (Z80-STI) emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

/*

    TODO:

    - timers (other than delay mode)
    - serial I/O
    - reset behavior

*/

#include "emu.h"
#include "z80sti.h"
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

// registers
enum
{
	Z80STI_REGISTER_IR = 0,
	Z80STI_REGISTER_GPIP,
	Z80STI_REGISTER_IPRB,
	Z80STI_REGISTER_IPRA,
	Z80STI_REGISTER_ISRB,
	Z80STI_REGISTER_ISRA,
	Z80STI_REGISTER_IMRB,
	Z80STI_REGISTER_IMRA,
	Z80STI_REGISTER_PVR,
	Z80STI_REGISTER_TABC,
	Z80STI_REGISTER_TBDR,
	Z80STI_REGISTER_TADR,
	Z80STI_REGISTER_UCR,
	Z80STI_REGISTER_RSR,
	Z80STI_REGISTER_TSR,
	Z80STI_REGISTER_UDR
};

// variable registers
enum
{
	Z80STI_REGISTER_IR_SCR = 0,
	Z80STI_REGISTER_IR_TDDR,
	Z80STI_REGISTER_IR_TCDR,
	Z80STI_REGISTER_IR_AER,
	Z80STI_REGISTER_IR_IERB,
	Z80STI_REGISTER_IR_IERA,
	Z80STI_REGISTER_IR_DDR,
	Z80STI_REGISTER_IR_TCDC
};

// timers
enum
{
	TIMER_A = 0,
	TIMER_B,
	TIMER_C,
	TIMER_D,
	TIMER_COUNT
};

// interrupt levels
enum
{
	Z80STI_IR_P0 = 0,
	Z80STI_IR_P1,
	Z80STI_IR_P2,
	Z80STI_IR_P3,
	Z80STI_IR_TD,
	Z80STI_IR_TC,
	Z80STI_IR_P4,
	Z80STI_IR_P5,
	Z80STI_IR_TB,
	Z80STI_IR_XE,
	Z80STI_IR_XB,
	Z80STI_IR_RE,
	Z80STI_IR_RB,
	Z80STI_IR_TA,
	Z80STI_IR_P6,
	Z80STI_IR_P7
};

// timer C/D control register
const int Z80STI_TCDC_TARS	= 0x80;
const int Z80STI_TCDC_TBRS	= 0x08;

// interrupt vector register
const int Z80STI_PVR_ISE	= 0x08;
const int Z80STI_PVR_VR4	= 0x10;

// general purpose I/O interrupt levels
static const int INT_LEVEL_GPIP[] =
{
	Z80STI_IR_P0, Z80STI_IR_P1, Z80STI_IR_P2, Z80STI_IR_P3,	Z80STI_IR_P4, Z80STI_IR_P5, Z80STI_IR_P6, Z80STI_IR_P7
};

// timer interrupt levels
static const int INT_LEVEL_TIMER[] =
{
	Z80STI_IR_TA, Z80STI_IR_TB, Z80STI_IR_TC, Z80STI_IR_TD
};

// interrupt vectors
static const UINT8 INT_VECTOR[] =
{
	0x00, 0x02, 0x04, 0x06, 0x08, 0x0a, 0x0c, 0x0e,
	0x10, 0x12, 0x14, 0x16, 0x18, 0x1a, 0x1c, 0x1e
};

// timer prescaler divisors
static const int PRESCALER[] = { 0, 4, 10, 16, 50, 64, 100, 200 };



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  z80sti_device_config - constructor
//-------------------------------------------------

z80sti_device_config::z80sti_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
	: device_config(mconfig, static_alloc_device_config, "Mostek MK3801", tag, owner, clock),
	  device_config_z80daisy_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  static_alloc_device_config - allocate a new
//  configuration object
//-------------------------------------------------

device_config *z80sti_device_config::static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
{
	return global_alloc(z80sti_device_config(mconfig, tag, owner, clock));
}


//-------------------------------------------------
//  alloc_device - allocate a new device object
//-------------------------------------------------

device_t *z80sti_device_config::alloc_device(running_machine &machine) const
{
	return auto_alloc(&machine, z80sti_device(machine, *this));
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void z80sti_device_config::device_config_complete()
{
	// inherit a copy of the static data
	const z80sti_interface *intf = reinterpret_cast<const z80sti_interface *>(static_config());
	if (intf != NULL)
		*static_cast<z80sti_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		m_rx_clock = m_tx_clock = 0;
		memset(&m_out_int_func, 0, sizeof(m_out_int_func));
		memset(&m_in_gpio_func, 0, sizeof(m_in_gpio_func));
		memset(&m_out_gpio_func, 0, sizeof(m_out_gpio_func));
		memset(&m_in_si_func, 0, sizeof(m_in_si_func));
		memset(&m_out_so_func, 0, sizeof(m_out_so_func));
		memset(&m_out_tao_func, 0, sizeof(m_out_tao_func));
		memset(&m_out_tbo_func, 0, sizeof(m_out_tbo_func));
		memset(&m_out_tco_func, 0, sizeof(m_out_tco_func));
		memset(&m_out_tdo_func, 0, sizeof(m_out_tdo_func));
	}
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  z80sti_device - constructor
//-------------------------------------------------

z80sti_device::z80sti_device(running_machine &_machine, const z80sti_device_config &config)
	: device_t(_machine, config),
	  device_z80daisy_interface(_machine, config, *this),
	  m_config(config)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void z80sti_device::device_start()
{
	// resolve callbacks
	devcb_resolve_read8(&m_in_gpio_func, &m_config.m_in_gpio_func, this);
	devcb_resolve_write8(&m_out_gpio_func, &m_config.m_out_gpio_func, this);
	devcb_resolve_read_line(&m_in_si_func, &m_config.m_in_si_func, this);
	devcb_resolve_write_line(&m_out_so_func, &m_config.m_out_so_func, this);
	devcb_resolve_write_line(&m_out_timer_func[TIMER_A], &m_config.m_out_tao_func, this);
	devcb_resolve_write_line(&m_out_timer_func[TIMER_B], &m_config.m_out_tbo_func, this);
	devcb_resolve_write_line(&m_out_timer_func[TIMER_C], &m_config.m_out_tco_func, this);
	devcb_resolve_write_line(&m_out_timer_func[TIMER_D], &m_config.m_out_tdo_func, this);
	devcb_resolve_write_line(&m_out_int_func, &m_config.m_out_int_func, this);

	// create the counter timers
	m_timer[TIMER_A] = timer_alloc(&m_machine, static_timer_count, (void *)this);
	m_timer[TIMER_B] = timer_alloc(&m_machine, static_timer_count, (void *)this);
	m_timer[TIMER_C] = timer_alloc(&m_machine, static_timer_count, (void *)this);
	m_timer[TIMER_D] = timer_alloc(&m_machine, static_timer_count, (void *)this);

	// create serial receive clock timer
	if (m_config.m_rx_clock > 0)
	{
		m_rx_timer = timer_alloc(&m_machine, static_rx_tick, (void *)this);
		timer_adjust_periodic(m_rx_timer, attotime_zero, 0, ATTOTIME_IN_HZ(m_config.m_rx_clock));
	}

	// create serial transmit clock timer
	if (m_config.m_tx_clock > 0)
	{
		m_tx_timer = timer_alloc(&m_machine, static_tx_tick, (void *)this);
		timer_adjust_periodic(m_tx_timer, attotime_zero, 0, ATTOTIME_IN_HZ(m_config.m_tx_clock));
	}

	// register for state saving
	state_save_register_device_item(this, 0, m_gpip);
	state_save_register_device_item(this, 0, m_aer);
	state_save_register_device_item(this, 0, m_ddr);
	state_save_register_device_item(this, 0, m_ier);
	state_save_register_device_item(this, 0, m_ipr);
	state_save_register_device_item(this, 0, m_isr);
	state_save_register_device_item(this, 0, m_imr);
	state_save_register_device_item(this, 0, m_pvr);
	state_save_register_device_item_array(this, 0, m_int_state);
	state_save_register_device_item(this, 0, m_tabc);
	state_save_register_device_item(this, 0, m_tcdc);
	state_save_register_device_item_array(this, 0, m_tdr);
	state_save_register_device_item_array(this, 0, m_tmc);
	state_save_register_device_item_array(this, 0, m_to);
	state_save_register_device_item(this, 0, m_scr);
	state_save_register_device_item(this, 0, m_ucr);
	state_save_register_device_item(this, 0, m_rsr);
	state_save_register_device_item(this, 0, m_tsr);
	state_save_register_device_item(this, 0, m_udr);

}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void z80sti_device::device_reset()
{
}



//**************************************************************************
//  DAISY CHAIN INTERFACE
//**************************************************************************

//-------------------------------------------------
//  z80daisy_irq_state - get interrupt status
//-------------------------------------------------

int z80sti_device::z80daisy_irq_state()
{
	int state = 0, i;

	// loop over all interrupt sources
	for (i = 15; i >= 0; i--)
	{
		// if we're servicing a request, don't indicate more interrupts
		if (m_int_state[i] & Z80_DAISY_IEO)
		{
			state |= Z80_DAISY_IEO;
			break;
		}

		if (BIT(m_imr, i))
		{
			state |= m_int_state[i];
		}
	}

	LOG(("Z80STI '%s' Interrupt State: %u\n", tag(), state));

	return state;
}


//-------------------------------------------------
//  z80daisy_irq_ack - interrupt acknowledge
//-------------------------------------------------

int z80sti_device::z80daisy_irq_ack()
{
	int i;

	// loop over all interrupt sources
	for (i = 15; i >= 0; i--)
	{
		// find the first channel with an interrupt requested
		if (m_int_state[i] & Z80_DAISY_INT)
		{
			UINT8 vector = (m_pvr & 0xe0) | INT_VECTOR[i];

			// clear interrupt, switch to the IEO state, and update the IRQs
			m_int_state[i] = Z80_DAISY_IEO;

			// clear interrupt pending register bit
			m_ipr &= ~(1 << i);

			// set interrupt in-service register bit
			m_isr |= (1 << i);

			check_interrupts();

			LOG(("Z80STI '%s' Interrupt Acknowledge Vector: %02x\n", tag(), vector));

			return vector;
		}
	}

	logerror("z80sti_irq_ack: failed to find an interrupt to ack!\n");

	return 0;
}


//-------------------------------------------------
//  z80daisy_irq_reti - return from interrupt
//-------------------------------------------------

void z80sti_device::z80daisy_irq_reti()
{
	int i;

	LOG(("Z80STI '%s' Return from Interrupt\n", tag()));

	// loop over all interrupt sources
	for (i = 15; i >= 0; i--)
	{
		// find the first channel with an IEO pending
		if (m_int_state[i] & Z80_DAISY_IEO)
		{
			// clear the IEO state and update the IRQs
			m_int_state[i] &= ~Z80_DAISY_IEO;

			// clear interrupt in-service register bit
			m_isr &= ~(1 << i);

			check_interrupts();
			return;
		}
	}

	logerror("z80sti_irq_reti: failed to find an interrupt to clear IEO on!\n");
}



//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  check_interrupts - set the interrupt request
//  line state
//-------------------------------------------------

void z80sti_device::check_interrupts()
{
	if (m_ipr & m_imr)
	{
		devcb_call_write_line(&m_out_int_func, ASSERT_LINE);
	}
	else
	{
		devcb_call_write_line(&m_out_int_func, CLEAR_LINE);
	}
}


//-------------------------------------------------
//  take_interrupt - mark an interrupt pending
//-------------------------------------------------

void z80sti_device::take_interrupt(int level)
{
	// set interrupt pending register bit
	m_ipr |= 1 << level;

	// trigger interrupt
	m_int_state[level] |= Z80_DAISY_INT;

	check_interrupts();
}


//-------------------------------------------------
//  serial_receive - receive serial bit
//-------------------------------------------------

void z80sti_device::serial_receive()
{
}


//-------------------------------------------------
//  serial_transmit - transmit serial bit
//-------------------------------------------------

void z80sti_device::serial_transmit()
{
}


//-------------------------------------------------
//  read - register read
//-------------------------------------------------

UINT8 z80sti_device::read(offs_t offset)
{
	switch (offset & 0x0f)
	{
	case Z80STI_REGISTER_IR:
		switch (m_pvr & 0x07)
		{
		case Z80STI_REGISTER_IR_SCR:	return m_scr;
		case Z80STI_REGISTER_IR_TDDR:	return m_tmc[TIMER_D];
		case Z80STI_REGISTER_IR_TCDR:	return m_tmc[TIMER_C];
		case Z80STI_REGISTER_IR_AER:	return m_aer;
		case Z80STI_REGISTER_IR_IERB:	return m_ier & 0xff;
		case Z80STI_REGISTER_IR_IERA:	return m_ier >> 8;
		case Z80STI_REGISTER_IR_DDR:	return m_ddr;
		case Z80STI_REGISTER_IR_TCDC:	return m_tcdc;
		}
		break;

	case Z80STI_REGISTER_GPIP:	m_gpip = (devcb_call_read8(&m_in_gpio_func, 0) & ~m_ddr) | (m_gpip & m_ddr); return m_gpip;
	case Z80STI_REGISTER_IPRB:	return m_ipr & 0xff;
	case Z80STI_REGISTER_IPRA:	return m_ipr >> 8;
	case Z80STI_REGISTER_ISRB:	return m_isr & 0xff;
	case Z80STI_REGISTER_ISRA:	return m_isr >> 8;
	case Z80STI_REGISTER_IMRB:	return m_imr & 0xff;
	case Z80STI_REGISTER_IMRA:	return m_imr >> 8;
	case Z80STI_REGISTER_PVR:	return m_pvr;
	case Z80STI_REGISTER_TABC:	return m_tabc;
	case Z80STI_REGISTER_TBDR:	return m_tmc[TIMER_B];
	case Z80STI_REGISTER_TADR:	return m_tmc[TIMER_A];
	case Z80STI_REGISTER_UCR:	return m_ucr;
	case Z80STI_REGISTER_RSR:	return m_rsr;
	case Z80STI_REGISTER_TSR:	return m_tsr;
	case Z80STI_REGISTER_UDR:	return m_udr;
	}

	return 0;
}


//-------------------------------------------------
//  write - register write
//-------------------------------------------------

void z80sti_device::write(offs_t offset, UINT8 data)
{
	switch (offset & 0x0f)
	{
	case Z80STI_REGISTER_IR:
		switch (m_pvr & 0x07)
		{
		case Z80STI_REGISTER_IR_SCR:
			LOG(("Z80STI '%s' Sync Character Register: %x\n", tag(), data));
			m_scr = data;
			break;

		case Z80STI_REGISTER_IR_TDDR:
			LOG(("Z80STI '%s' Timer D Data Register: %x\n", tag(), data));
			m_tdr[TIMER_D] = data;
			break;

		case Z80STI_REGISTER_IR_TCDR:
			LOG(("Z80STI '%s' Timer C Data Register: %x\n", tag(), data));
			m_tdr[TIMER_C] = data;
			break;

		case Z80STI_REGISTER_IR_AER:
			LOG(("Z80STI '%s' Active Edge Register: %x\n", tag(), data));
			m_aer = data;
			break;

		case Z80STI_REGISTER_IR_IERB:
			LOG(("Z80STI '%s' Interrupt Enable Register B: %x\n", tag(), data));
			m_ier = (m_ier & 0xff00) | data;
			check_interrupts();
			break;

		case Z80STI_REGISTER_IR_IERA:
			LOG(("Z80STI '%s' Interrupt Enable Register A: %x\n", tag(), data));
			m_ier = (data << 8) | (m_ier & 0xff);
			check_interrupts();
			break;

		case Z80STI_REGISTER_IR_DDR:
			LOG(("Z80STI '%s' Data Direction Register: %x\n", tag(), data));
			m_ddr = data;
			break;

		case Z80STI_REGISTER_IR_TCDC:
			{
			int tcc = PRESCALER[(data >> 4) & 0x07];
			int tdc = PRESCALER[data & 0x07];

			m_tcdc = data;

			LOG(("Z80STI '%s' Timer C Prescaler: %u\n", tag(), tcc));
			LOG(("Z80STI '%s' Timer D Prescaler: %u\n", tag(), tdc));

			if (tcc)
				timer_adjust_periodic(m_timer[TIMER_C], ATTOTIME_IN_HZ(clock() / tcc), TIMER_C, ATTOTIME_IN_HZ(clock() / tcc));
			else
				timer_enable(m_timer[TIMER_C], 0);

			if (tdc)
				timer_adjust_periodic(m_timer[TIMER_D], ATTOTIME_IN_HZ(clock() / tdc), TIMER_D, ATTOTIME_IN_HZ(clock() / tdc));
			else
				timer_enable(m_timer[TIMER_D], 0);

			if (BIT(data, 7))
			{
				LOG(("Z80STI '%s' Timer A Reset\n", tag()));
				m_to[TIMER_A] = 0;

				devcb_call_write_line(&m_out_timer_func[TIMER_A], m_to[TIMER_A]);
			}

			if (BIT(data, 3))
			{
				LOG(("Z80STI '%s' Timer B Reset\n", tag()));
				m_to[TIMER_B] = 0;

				devcb_call_write_line(&m_out_timer_func[TIMER_B], m_to[TIMER_B]);
			}
			}
			break;
		}
		break;

	case Z80STI_REGISTER_GPIP:
		LOG(("Z80STI '%s' General Purpose I/O Register: %x\n", tag(), data));
		m_gpip = data & m_ddr;
		devcb_call_write8(&m_out_gpio_func, 0, m_gpip);
		break;

	case Z80STI_REGISTER_IPRB:
		{
		int i;
		LOG(("Z80STI '%s' Interrupt Pending Register B: %x\n", tag(), data));
		m_ipr &= (m_ipr & 0xff00) | data;

		for (i = 0; i < 16; i++)
		{
			if (!BIT(m_ipr, i) && (m_int_state[i] == Z80_DAISY_INT)) m_int_state[i] = 0;
		}

		check_interrupts();
		}
		break;

	case Z80STI_REGISTER_IPRA:
		{
		int i;
		LOG(("Z80STI '%s' Interrupt Pending Register A: %x\n", tag(), data));
		m_ipr &= (data << 8) | (m_ipr & 0xff);

		for (i = 0; i < 16; i++)
		{
			if (!BIT(m_ipr, i) && (m_int_state[i] == Z80_DAISY_INT)) m_int_state[i] = 0;
		}

		check_interrupts();
		}
		break;

	case Z80STI_REGISTER_ISRB:
		LOG(("Z80STI '%s' Interrupt In-Service Register B: %x\n", tag(), data));
		m_isr &= (m_isr & 0xff00) | data;
		break;

	case Z80STI_REGISTER_ISRA:
		LOG(("Z80STI '%s' Interrupt In-Service Register A: %x\n", tag(), data));
		m_isr &= (data << 8) | (m_isr & 0xff);
		break;

	case Z80STI_REGISTER_IMRB:
		LOG(("Z80STI '%s' Interrupt Mask Register B: %x\n", tag(), data));
		m_imr = (m_imr & 0xff00) | data;
		m_isr &= m_imr;
		check_interrupts();
		break;

	case Z80STI_REGISTER_IMRA:
		LOG(("Z80STI '%s' Interrupt Mask Register A: %x\n", tag(), data));
		m_imr = (data << 8) | (m_imr & 0xff);
		m_isr &= m_imr;
		check_interrupts();
		break;

	case Z80STI_REGISTER_PVR:
		LOG(("Z80STI '%s' Interrupt Vector: %02x\n", tag(), data & 0xe0));
		LOG(("Z80STI '%s' IR Address: %01x\n", tag(), data & 0x07));
		m_pvr = data;
		break;

	case Z80STI_REGISTER_TABC:
		{
		int tac = PRESCALER[(data >> 4) & 0x07];
		int tbc = PRESCALER[data & 0x07];

		m_tabc = data;

		LOG(("Z80STI '%s' Timer A Prescaler: %u\n", tag(), tac));
		LOG(("Z80STI '%s' Timer B Prescaler: %u\n", tag(), tbc));

		if (tac)
			timer_adjust_periodic(m_timer[TIMER_A], ATTOTIME_IN_HZ(clock() / tac), TIMER_A, ATTOTIME_IN_HZ(clock() / tac));
		else
			timer_enable(m_timer[TIMER_A], 0);

		if (tbc)
			timer_adjust_periodic(m_timer[TIMER_B], ATTOTIME_IN_HZ(clock() / tbc), TIMER_B, ATTOTIME_IN_HZ(clock() / tbc));
		else
			timer_enable(m_timer[TIMER_B], 0);
		}
		break;

	case Z80STI_REGISTER_TBDR:
		LOG(("Z80STI '%s' Timer B Data Register: %x\n", tag(), data));
		m_tdr[TIMER_B] = data;
		break;

	case Z80STI_REGISTER_TADR:
		LOG(("Z80STI '%s' Timer A Data Register: %x\n", tag(), data));
		m_tdr[TIMER_A] = data;
		break;
#if 0
    case Z80STI_REGISTER_UCR:
        m_ucr = data;
        break;

    case Z80STI_REGISTER_RSR:
        m_rsr = data;
        break;

    case Z80STI_REGISTER_TSR:
        m_tsr = data;
        break;

    case Z80STI_REGISTER_UDR:
        m_udr = data;
        break;
#endif
	default:
		LOG(("Z80STI '%s' Unsupported Register %x\n", tag(), offset & 0x0f));
	}
}


//-------------------------------------------------
//  timer_count - timer count down
//-------------------------------------------------

void z80sti_device::timer_count(int index)
{
	if (m_tmc[index] == 0x01)
	{
		//LOG(("Z80STI '%s' Timer %c Expired\n", tag(), 'A' + index));

		// toggle timer output signal
		m_to[index] = !m_to[index];

		devcb_call_write_line(&m_out_timer_func[index], m_to[index]);

		if (m_ier & (1 << INT_LEVEL_TIMER[index]))
		{
			LOG(("Z80STI '%s' Interrupt Pending for Timer %c\n", tag(), 'A' + index));

			// signal timer elapsed interrupt
			take_interrupt(INT_LEVEL_TIMER[index]);
		}

		// load timer main counter
		m_tmc[index] = m_tdr[index];
	}
	else
	{
		// count down
		m_tmc[index]--;
	}
}


//-------------------------------------------------
//  gpip_input - GPIP input line write
//-------------------------------------------------

void z80sti_device::gpip_input(int bit, int state)
{
	int aer = BIT(m_aer, bit);
	int old_state = BIT(m_gpip, bit);

	if ((old_state ^ aer) && !(state ^ aer))
	{
		LOG(("Z80STI '%s' Edge Transition Detected on Bit: %u\n", tag(), bit));

		if (m_ier & (1 << INT_LEVEL_GPIP[bit]))
		{
			LOG(("Z80STI '%s' Interrupt Pending for P%u\n", tag(), bit));

			take_interrupt(INT_LEVEL_GPIP[bit]);
		}
	}

	m_gpip = (m_gpip & ~(1 << bit)) | (state << bit);
}



//**************************************************************************
//  GLOBAL STUBS
//**************************************************************************

READ8_DEVICE_HANDLER( z80sti_r ) { return downcast<z80sti_device *>(device)->read(offset); }
WRITE8_DEVICE_HANDLER( z80sti_w ) { downcast<z80sti_device *>(device)->write(offset, data); }

WRITE_LINE_DEVICE_HANDLER( z80sti_rc_w ) { if (state) downcast<z80sti_device *>(device)->serial_receive(); }
WRITE_LINE_DEVICE_HANDLER( z80sti_tc_w ) { if (state) downcast<z80sti_device *>(device)->serial_transmit(); }

WRITE_LINE_DEVICE_HANDLER( z80sti_i0_w ) { downcast<z80sti_device *>(device)->gpip_input(0, state); }
WRITE_LINE_DEVICE_HANDLER( z80sti_i1_w ) { downcast<z80sti_device *>(device)->gpip_input(1, state); }
WRITE_LINE_DEVICE_HANDLER( z80sti_i2_w ) { downcast<z80sti_device *>(device)->gpip_input(2, state); }
WRITE_LINE_DEVICE_HANDLER( z80sti_i3_w ) { downcast<z80sti_device *>(device)->gpip_input(3, state); }
WRITE_LINE_DEVICE_HANDLER( z80sti_i4_w ) { downcast<z80sti_device *>(device)->gpip_input(4, state); }
WRITE_LINE_DEVICE_HANDLER( z80sti_i5_w ) { downcast<z80sti_device *>(device)->gpip_input(5, state); }
WRITE_LINE_DEVICE_HANDLER( z80sti_i6_w ) { downcast<z80sti_device *>(device)->gpip_input(6, state); }
WRITE_LINE_DEVICE_HANDLER( z80sti_i7_w ) { downcast<z80sti_device *>(device)->gpip_input(7, state); }

const device_type Z80STI = z80sti_device_config::static_alloc_device_config;
