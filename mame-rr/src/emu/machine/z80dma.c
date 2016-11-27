/**********************************************************************

    Z80 DMA interface and emulation

    For datasheet http://www.zilog.com/docs/z80/ps0179.pdf

    2008/01     couriersud

        - architecture copied from 8257 DMA
        - significant changes to implementation
        - This is only a minimum implementation to support dkong3 and mario drivers
        - Only memory to memory is tested!

    TODO:
        - implement missing features
        - implement more asserts
        - implement a INPUT_LINE_BUSREQ for Z80. As a workaround,
          HALT is used. This implies burst mode.

**********************************************************************/

#include "emu.h"
#include "memconv.h"
#include "z80dma.h"
#include "cpu/z80/z80daisy.h"



//**************************************************************************
//  CONSTANTS
//**************************************************************************

enum
{
	INT_RDY = 0,
	INT_MATCH,
	INT_END_OF_BLOCK,
	INT_MATCH_END_OF_BLOCK
};

const int COMMAND_RESET							= 0xc3;
const int COMMAND_RESET_PORT_A_TIMING			= 0xc7;
const int COMMAND_RESET_PORT_B_TIMING			= 0xcb;
const int COMMAND_LOAD							= 0xcf;
const int COMMAND_CONTINUE						= 0xd3;
const int COMMAND_DISABLE_INTERRUPTS			= 0xaf;
const int COMMAND_ENABLE_INTERRUPTS				= 0xab;
const int COMMAND_RESET_AND_DISABLE_INTERRUPTS	= 0xa3;
const int COMMAND_ENABLE_AFTER_RETI				= 0xb7;
const int COMMAND_READ_STATUS_BYTE				= 0xbf;
const int COMMAND_REINITIALIZE_STATUS_BYTE		= 0x8b;
const int COMMAND_INITIATE_READ_SEQUENCE		= 0xa7;
const int COMMAND_FORCE_READY					= 0xb3;
const int COMMAND_ENABLE_DMA					= 0x87;
const int COMMAND_DISABLE_DMA					= 0x83;
const int COMMAND_READ_MASK_FOLLOWS				= 0xbb;

const int TM_TRANSFER			= 0x01;
const int TM_SEARCH				= 0x02;
const int TM_SEARCH_TRANSFER	= 0x03;



//**************************************************************************
//  MACROS
//**************************************************************************

#define LOG 0

#define REGNUM(_m, _s)			(((_m)<<3) + (_s))
#define GET_REGNUM(_r)			(&(_r) - &(WR0))
#define REG(_m, _s) 			m_regs[REGNUM(_m,_s)]
#define WR0						REG(0, 0)
#define WR1						REG(1, 0)
#define WR2						REG(2, 0)
#define WR3						REG(3, 0)
#define WR4						REG(4, 0)
#define WR5						REG(5, 0)
#define WR6						REG(6, 0)

#define PORTA_ADDRESS_L			REG(0,1)
#define PORTA_ADDRESS_H			REG(0,2)

#define BLOCKLEN_L				REG(0,3)
#define BLOCKLEN_H				REG(0,4)

#define PORTA_TIMING			REG(1,1)
#define PORTB_TIMING			REG(2,1)

#define MASK_BYTE				REG(3,1)
#define MATCH_BYTE				REG(3,2)

#define PORTB_ADDRESS_L			REG(4,1)
#define PORTB_ADDRESS_H			REG(4,2)
#define INTERRUPT_CTRL			REG(4,3)
#define INTERRUPT_VECTOR		REG(4,4)
#define PULSE_CTRL				REG(4,5)

#define READ_MASK				REG(6,1)

#define PORTA_ADDRESS			((PORTA_ADDRESS_H<<8) | PORTA_ADDRESS_L)
#define PORTB_ADDRESS			((PORTB_ADDRESS_H<<8) | PORTB_ADDRESS_L)
#define BLOCKLEN				((BLOCKLEN_H<<8) | BLOCKLEN_L)

#define PORTA_STEP				(((WR1 >> 4) & 0x03)*2-1)
#define PORTB_STEP				(((WR2 >> 4) & 0x03)*2-1)
#define PORTA_INC				(WR1 & 0x10)
#define PORTB_INC				(WR2 & 0x10)
#define PORTA_FIXED				(((WR1 >> 4) & 0x02) == 0x02)
#define PORTB_FIXED				(((WR2 >> 4) & 0x02) == 0x02)
#define PORTA_MEMORY			(((WR1 >> 3) & 0x01) == 0x00)
#define PORTB_MEMORY			(((WR2 >> 3) & 0x01) == 0x00)

#define PORTA_CYCLE_LEN			(4-(PORTA_TIMING & 0x03))
#define PORTB_CYCLE_LEN			(4-(PORTB_TIMING & 0x03))

#define PORTA_IS_SOURCE			((WR0 >> 2) & 0x01)
#define PORTB_IS_SOURCE			(!PORTA_IS_SOURCE)
#define TRANSFER_MODE			(WR0 & 0x03)

#define MATCH_F_SET				(m_status &= ~0x10)
#define MATCH_F_CLEAR			(m_status |= 0x10)
#define EOB_F_SET				(m_status &= ~0x20)
#define EOB_F_CLEAR				(m_status |= 0x20)

#define READY_ACTIVE_HIGH		((WR5>>3) & 0x01)

#define INTERRUPT_ENABLE		(WR3 & 0x20)
#define INT_ON_MATCH			(INTERRUPT_CTRL & 0x01)
#define INT_ON_END_OF_BLOCK		(INTERRUPT_CTRL & 0x02)
#define INT_ON_READY			(INTERRUPT_CTRL & 0x40)
#define STATUS_AFFECTS_VECTOR	(INTERRUPT_CTRL & 0x20)



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  z80dma_device_config - constructor
//-------------------------------------------------

z80dma_device_config::z80dma_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
	: device_config(mconfig, static_alloc_device_config, "Z8410", tag, owner, clock),
	  device_config_z80daisy_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  static_alloc_device_config - allocate a new
//  configuration object
//-------------------------------------------------

device_config *z80dma_device_config::static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
{
	return global_alloc(z80dma_device_config(mconfig, tag, owner, clock));
}


//-------------------------------------------------
//  alloc_device - allocate a new device object
//-------------------------------------------------

device_t *z80dma_device_config::alloc_device(running_machine &machine) const
{
	return auto_alloc(&machine, z80dma_device(machine, *this));
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void z80dma_device_config::device_config_complete()
{
	// inherit a copy of the static data
	const z80dma_interface *intf = reinterpret_cast<const z80dma_interface *>(static_config());
	if (intf != NULL)
		*static_cast<z80dma_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		memset(&m_out_busreq_func, 0, sizeof(m_out_busreq_func));
		memset(&m_out_int_func, 0, sizeof(m_out_int_func));
		memset(&m_out_bao_func, 0, sizeof(m_out_bao_func));
		memset(&m_in_mreq_func, 0, sizeof(m_in_mreq_func));
		memset(&m_out_mreq_func, 0, sizeof(m_out_mreq_func));
		memset(&m_in_iorq_func, 0, sizeof(m_in_iorq_func));
		memset(&m_out_iorq_func, 0, sizeof(m_out_iorq_func));
	}
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  z80dma_device - constructor
//-------------------------------------------------

z80dma_device::z80dma_device(running_machine &_machine, const z80dma_device_config &_config)
	: device_t(_machine, _config),
	  device_z80daisy_interface(_machine, _config, *this),
	  m_config(_config)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void z80dma_device::device_start()
{
	// resolve callbacks
	devcb_resolve_write_line(&m_out_busreq_func, &m_config.m_out_busreq_func, this);
	devcb_resolve_write_line(&m_out_int_func, &m_config.m_out_int_func, this);
	devcb_resolve_write_line(&m_out_bao_func, &m_config.m_out_bao_func, this);
	devcb_resolve_read8(&m_in_mreq_func, &m_config.m_in_mreq_func, this);
	devcb_resolve_write8(&m_out_mreq_func, &m_config.m_out_mreq_func, this);
	devcb_resolve_read8(&m_in_iorq_func, &m_config.m_in_iorq_func, this);
	devcb_resolve_write8(&m_out_iorq_func, &m_config.m_out_iorq_func, this);

	// allocate timer
	m_timer = timer_alloc(&m_machine, static_timerproc, (void *)this);

	// register for state saving
	state_save_register_device_item_array(this, 0, m_regs);
	state_save_register_device_item_array(this, 0, m_regs_follow);
	state_save_register_device_item(this, 0, m_num_follow);
	state_save_register_device_item(this, 0, m_cur_follow);
	state_save_register_device_item(this, 0, m_status);
	state_save_register_device_item(this, 0, m_dma_enabled);
	state_save_register_device_item(this, 0, m_vector);
	state_save_register_device_item(this, 0, m_ip);
	state_save_register_device_item(this, 0, m_ius);
	state_save_register_device_item(this, 0, m_addressA);
	state_save_register_device_item(this, 0, m_addressB);
	state_save_register_device_item(this, 0, m_count);
	state_save_register_device_item(this, 0, m_rdy);
	state_save_register_device_item(this, 0, m_force_ready);
	state_save_register_device_item(this, 0, m_is_read);
	state_save_register_device_item(this, 0, m_cur_cycle);
	state_save_register_device_item(this, 0, m_latch);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void z80dma_device::device_reset()
{
	m_status = 0;
	m_rdy = 0;
	m_force_ready = 0;
	m_num_follow = 0;
	m_dma_enabled = 0;
	m_read_num_follow = m_read_cur_follow = 0;
	m_reset_pointer = 0;

	// disable interrupts
	WR3 &= ~0x20;
	m_ip = 0;
	m_ius = 0;
	m_vector = 0;

	update_status();
}



//**************************************************************************
//  DAISY CHAIN INTERFACE
//**************************************************************************

//-------------------------------------------------
//  z80daisy_irq_state - return the overall IRQ
//  state for this device
//-------------------------------------------------

int z80dma_device::z80daisy_irq_state()
{
	int state = 0;

	if (m_ip)
	{
		// interrupt pending
		state = Z80_DAISY_INT;
	}
	else if (m_ius)
	{
		// interrupt under service
		state = Z80_DAISY_IEO;
	}

	if (LOG) logerror("Z80DMA '%s' Interrupt State: %u\n", tag(), state);

	return state;
}


//-------------------------------------------------
//  z80daisy_irq_ack - acknowledge an IRQ and
//  return the appropriate vector
//-------------------------------------------------

int z80dma_device::z80daisy_irq_ack()
{
	if (m_ip)
	{
	    if (LOG) logerror("Z80DMA '%s' Interrupt Acknowledge\n", tag());

		// clear interrupt pending flag
		m_ip = 0;
	    interrupt_check();

		// set interrupt under service flag
		m_ius = 1;

		// disable DMA
		m_dma_enabled = 0;

		return m_vector;
	}

	logerror("z80dma_irq_ack: failed to find an interrupt to ack!\n");

	return 0;
}


//-------------------------------------------------
//  z80daisy_irq_reti - clear the interrupt
//  pending state to allow other interrupts through
//-------------------------------------------------

void z80dma_device::z80daisy_irq_reti()
{
	if (m_ius)
	{
	    if (LOG) logerror("Z80DMA '%s' Return from Interrupt\n", tag());

		// clear interrupt under service flag
		m_ius = 0;
	    interrupt_check();

		return;
	}

	logerror("z80dma_irq_reti: failed to find an interrupt to clear IEO on!\n");
}



//**************************************************************************
//  INTERNAL STATE MANAGEMENT
//**************************************************************************

//-------------------------------------------------
//  interrupt_check - update IRQ line state
//-------------------------------------------------

int z80dma_device::is_ready()
{
	return (m_force_ready) || (m_rdy == READY_ACTIVE_HIGH);
}


//-------------------------------------------------
//  interrupt_check - update IRQ line state
//-------------------------------------------------

void z80dma_device::interrupt_check()
{
	devcb_call_write_line(&m_out_int_func, m_ip ? ASSERT_LINE : CLEAR_LINE);
}


//-------------------------------------------------
//  trigger_interrupt - trigger DMA interrupt
//-------------------------------------------------

void z80dma_device::trigger_interrupt(int level)
{
	if (!m_ius && INTERRUPT_ENABLE)
	{
		// set interrupt pending flag
		m_ip = 1;

		// set interrupt vector
		if (STATUS_AFFECTS_VECTOR)
		{
			m_vector = (INTERRUPT_VECTOR & 0xf9) | (level << 1);
		}
		else
		{
			m_vector = INTERRUPT_VECTOR;
		}

		m_status &= ~0x08;

		if (LOG) logerror("Z80DMA '%s' Interrupt Pending\n", tag());

		interrupt_check();
	}
}


//-------------------------------------------------
//  do_read - perform DMA read
//-------------------------------------------------

void z80dma_device::do_read()
{
	UINT8 mode;

	mode = TRANSFER_MODE;
	switch(mode) {
		case TM_TRANSFER:
		case TM_SEARCH:
			if (PORTA_IS_SOURCE)
			{
				if (PORTA_MEMORY)
					m_latch = devcb_call_read8(&m_in_mreq_func, m_addressA);
				else
					m_latch = devcb_call_read8(&m_in_iorq_func, m_addressA);

				if (LOG) logerror("A src: %04x \n",m_addressA);
				m_addressA += PORTA_FIXED ? 0 : PORTA_INC ? PORTA_STEP : -PORTA_STEP;
			}
			else
			{
				if (PORTB_MEMORY)
					m_latch = devcb_call_read8(&m_in_mreq_func, m_addressB);
				else
					m_latch = devcb_call_read8(&m_in_iorq_func, m_addressB);

				if (LOG) logerror("B src: %04x \n",m_addressB);
				m_addressB += PORTB_FIXED ? 0 : PORTB_INC ? PORTB_STEP : -PORTB_STEP;
			}
			break;
		case TM_SEARCH_TRANSFER:
			fatalerror("z80dma_do_operation: unhandled search & transfer mode !\n");
			break;
		default:
			fatalerror("z80dma_do_operation: invalid mode %d!\n", mode);
			break;
	}
}


//-------------------------------------------------
//  do_write - perform DMA write
//-------------------------------------------------

int z80dma_device::do_write()
{
	int done;
	UINT8 mode;

	mode = TRANSFER_MODE;
	if (m_count == 0x0000)
	{
		//FIXME: Any signal here
	}
	switch(mode) {
		case TM_TRANSFER:
			if (PORTA_IS_SOURCE)
			{
				if (PORTB_MEMORY)
					devcb_call_write8(&m_out_mreq_func, m_addressB, m_latch);
				else
					devcb_call_write8(&m_out_iorq_func, m_addressB, m_latch);

				if (LOG) logerror("B dst: %04x \n",m_addressB);
				m_addressB += PORTB_FIXED ? 0 : PORTB_INC ? PORTB_STEP : -PORTB_STEP;
			}
			else
			{
				if (PORTA_MEMORY)
					devcb_call_write8(&m_out_mreq_func, m_addressA, m_latch);
				else
					devcb_call_write8(&m_out_iorq_func, m_addressA, m_latch);

				if (LOG) logerror("A dst: %04x \n",m_addressA);
				m_addressA += PORTA_FIXED ? 0 : PORTA_INC ? PORTA_STEP : -PORTA_STEP;
			}
			m_count--;
			done = (m_count == 0xFFFF);
			break;

		case TM_SEARCH:
			{
				UINT8 load_byte,match_byte;
				load_byte = m_latch | MASK_BYTE;
				match_byte = MATCH_BYTE | MASK_BYTE;
				//if (LOG) logerror("%02x %02x\n",load_byte,match_byte));
				if (load_byte == match_byte)
				{
					if (INT_ON_MATCH)
					{
						trigger_interrupt(INT_MATCH);
					}
				}

				m_count--;
				done = (m_count == 0xFFFF); //correct?
			}
			break;

		case TM_SEARCH_TRANSFER:
			fatalerror("z80dma_do_operation: unhandled search & transfer mode !\n");
			break;

		default:
			fatalerror("z80dma_do_operation: invalid mode %d!\n", mode);
			break;
	}
	if (done)
	{
		//FIXME: interrupt ?
	}
	return done;
}


//-------------------------------------------------
//  timerproc
//-------------------------------------------------

void z80dma_device::timerproc()
{
	int done;

	if (--m_cur_cycle)
	{
		return;
	}

	if (m_is_read)
	{
		do_read();
		done = 0;
		m_is_read = false;
		m_cur_cycle = (PORTA_IS_SOURCE ? PORTA_CYCLE_LEN : PORTB_CYCLE_LEN);
	}
	else
	{
		done = do_write();
		m_is_read = true;
		m_cur_cycle = (PORTB_IS_SOURCE ? PORTA_CYCLE_LEN : PORTB_CYCLE_LEN);
	}

	if (done)
	{
		m_dma_enabled = 0; //FIXME: Correct?
        m_status = 0x19;

		m_status |= !is_ready() << 1; // ready line status

		if(TRANSFER_MODE == TM_TRANSFER)     m_status |= 0x10;   // no match found

		update_status();
		if (LOG) logerror("Z80DMA '%s' End of Block\n", tag());

		if (INT_ON_END_OF_BLOCK)
        {
			trigger_interrupt(INT_END_OF_BLOCK);
        }
	}
}


//-------------------------------------------------
//  update_status - update DMA status
//-------------------------------------------------

void z80dma_device::update_status()
{
	UINT16 pending_transfer;
	attotime next;

	// no transfer is active right now; is there a transfer pending right now?
	pending_transfer = is_ready() & m_dma_enabled;

	if (pending_transfer)
	{
		m_is_read = true;
		m_cur_cycle = (PORTA_IS_SOURCE ? PORTA_CYCLE_LEN : PORTB_CYCLE_LEN);
		next = ATTOTIME_IN_HZ(clock());
		timer_adjust_periodic(m_timer,
			attotime_zero,
			0,
			// 1 byte transferred in 4 clock cycles
			next);
	}
	else
	{
		if (m_is_read)
		{
			// no transfers active right now
			timer_reset(m_timer, attotime_never);
		}
	}

	// set the busreq line
	devcb_call_write_line(&m_out_busreq_func, pending_transfer ? ASSERT_LINE : CLEAR_LINE);
}



//**************************************************************************
//  READ/WRITE INTERFACES
//**************************************************************************

//-------------------------------------------------
//  read - register read
//-------------------------------------------------

UINT8 z80dma_device::read()
{
	UINT8 res;

	res = m_read_regs_follow[m_read_cur_follow];
	m_read_cur_follow++;

	if(m_read_cur_follow >= m_read_num_follow)
		m_read_cur_follow = 0;

	return res;
}


//-------------------------------------------------
//  write - register write
//-------------------------------------------------

void z80dma_device::write(UINT8 data)
{
	if (m_num_follow == 0)
	{
		if ((data & 0x87) == 0) // WR2
		{
			WR2 = data;
			if (data & 0x40)
				m_regs_follow[m_num_follow++] = GET_REGNUM(PORTB_TIMING);
		}
		else if ((data & 0x87) == 0x04) // WR1
		{
			WR1 = data;
			if (data & 0x40)
				m_regs_follow[m_num_follow++] = GET_REGNUM(PORTA_TIMING);
		}
		else if ((data & 0x80) == 0) // WR0
		{
			WR0 = data;
			if (data & 0x08)
				m_regs_follow[m_num_follow++] = GET_REGNUM(PORTA_ADDRESS_L);
			if (data & 0x10)
				m_regs_follow[m_num_follow++] = GET_REGNUM(PORTA_ADDRESS_H);
			if (data & 0x20)
				m_regs_follow[m_num_follow++] = GET_REGNUM(BLOCKLEN_L);
			if (data & 0x40)
				m_regs_follow[m_num_follow++] = GET_REGNUM(BLOCKLEN_H);
		}
		else if ((data & 0x83) == 0x80) // WR3
		{
			WR3 = data;
			if (data & 0x08)
				m_regs_follow[m_num_follow++] = GET_REGNUM(MASK_BYTE);
			if (data & 0x10)
				m_regs_follow[m_num_follow++] = GET_REGNUM(MATCH_BYTE);
		}
		else if ((data & 0x83) == 0x81) // WR4
		{
			WR4 = data;
			if (data & 0x04)
				m_regs_follow[m_num_follow++] = GET_REGNUM(PORTB_ADDRESS_L);
			if (data & 0x08)
				m_regs_follow[m_num_follow++] = GET_REGNUM(PORTB_ADDRESS_H);
			if (data & 0x10)
				m_regs_follow[m_num_follow++] = GET_REGNUM(INTERRUPT_CTRL);
		}
		else if ((data & 0xC7) == 0x82) // WR5
		{
			WR5 = data;
		}
		else if ((data & 0x83) == 0x83) // WR6
		{
			m_dma_enabled = 0;

			WR6 = data;


			switch (data)
			{
				case COMMAND_ENABLE_AFTER_RETI:
					fatalerror("Unimplemented WR6 command %02x", data);
					break;
				case COMMAND_READ_STATUS_BYTE:
					if (LOG) logerror("CMD Read status Byte\n");
        			READ_MASK = 0;
        			break;
				case COMMAND_RESET_AND_DISABLE_INTERRUPTS:
					WR3 &= ~0x20;
					m_ip = 0;
					m_ius = 0;
					m_force_ready = 0;
					m_status |= 0x08;
					break;
				case COMMAND_INITIATE_READ_SEQUENCE:
					if (LOG) logerror("Initiate Read Sequence\n");
					m_read_cur_follow = m_read_num_follow = 0;
					if(READ_MASK & 0x01) { m_read_regs_follow[m_read_num_follow++] = m_status; }
					if(READ_MASK & 0x02) { m_read_regs_follow[m_read_num_follow++] = BLOCKLEN_L; } //byte counter (low)
					if(READ_MASK & 0x04) { m_read_regs_follow[m_read_num_follow++] = BLOCKLEN_H; } //byte counter (high)
					if(READ_MASK & 0x08) { m_read_regs_follow[m_read_num_follow++] = PORTA_ADDRESS_L; } //port A address (low)
					if(READ_MASK & 0x10) { m_read_regs_follow[m_read_num_follow++] = PORTA_ADDRESS_H; } //port A address (high)
					if(READ_MASK & 0x20) { m_read_regs_follow[m_read_num_follow++] = PORTB_ADDRESS_L; } //port B address (low)
					if(READ_MASK & 0x40) { m_read_regs_follow[m_read_num_follow++] = PORTA_ADDRESS_H; } //port B address (high)
					break;
				case COMMAND_RESET:
					if (LOG) logerror("Reset\n");
					m_dma_enabled = 0;
					m_force_ready = 0;
					// Needs six reset commands to reset the DMA
					{
						UINT8 WRi;

						for(WRi=0;WRi<7;WRi++)
							REG(WRi,m_reset_pointer) = 0;

						m_reset_pointer++;
						if(m_reset_pointer >= 6) { m_reset_pointer = 0; }
					}
					m_status = 0x38;
					break;
				case COMMAND_LOAD:
					m_force_ready = 0;
					m_addressA = PORTA_ADDRESS;
					m_addressB = PORTB_ADDRESS;
					m_count = BLOCKLEN;
					m_status |= 0x30;
					if (LOG) logerror("Load A: %x B: %x N: %x\n", m_addressA, m_addressB, m_count);
					break;
				case COMMAND_DISABLE_DMA:
					if (LOG) logerror("Disable DMA\n");
					m_dma_enabled = 0;
					break;
				case COMMAND_ENABLE_DMA:
					if (LOG) logerror("Enable DMA\n");
					m_dma_enabled = 1;
					break;
				case COMMAND_READ_MASK_FOLLOWS:
					if (LOG) logerror("Set Read Mask\n");
					m_regs_follow[m_num_follow++] = GET_REGNUM(READ_MASK);
					break;
				case COMMAND_CONTINUE:
					if (LOG) logerror("Continue\n");
					m_count = BLOCKLEN;
					m_dma_enabled = 1;
					//"match not found" & "end of block" status flags zeroed here
					m_status |= 0x30;
					break;
				case COMMAND_RESET_PORT_A_TIMING:
					if (LOG) logerror("Reset Port A Timing\n");
					PORTA_TIMING = 0;
					break;
				case COMMAND_RESET_PORT_B_TIMING:
					if (LOG) logerror("Reset Port B Timing\n");
					PORTB_TIMING = 0;
					break;
				case COMMAND_FORCE_READY:
					if (LOG) logerror("Force ready\n");
					m_force_ready = 1;
					update_status();
					break;
				case COMMAND_ENABLE_INTERRUPTS:
					if (LOG) logerror("Enable IRQ\n");
					WR3 |= 0x20;
					break;
				case COMMAND_DISABLE_INTERRUPTS:
					if (LOG) logerror("Disable IRQ\n");
					WR3 &= ~0x20;
					break;
				case COMMAND_REINITIALIZE_STATUS_BYTE:
					if (LOG) logerror("Reinitialize status byte\n");
					m_status |= 0x30;
					m_ip = 0;
					break;
				case 0xFB:
					if (LOG) logerror("Z80DMA undocumented command triggered 0x%02X!\n",data);
					break;
				default:
					fatalerror("Unknown WR6 command %02x", data);
			}
		}
		else
			fatalerror("Unknown base register %02x", data);
		m_cur_follow = 0;
	}
	else
	{
		int nreg = m_regs_follow[m_cur_follow];
		m_regs[nreg] = data;
		m_cur_follow++;
		if (m_cur_follow>=m_num_follow)
			m_num_follow = 0;
		if (nreg == REGNUM(4,3))
		{
			m_num_follow=0;
			if (data & 0x08)
				m_regs_follow[m_num_follow++] = GET_REGNUM(PULSE_CTRL);
			if (data & 0x10)
				m_regs_follow[m_num_follow++] = GET_REGNUM(INTERRUPT_VECTOR);
			m_cur_follow = 0;
		}
	}
}


//-------------------------------------------------
//  rdy_write_callback - deferred RDY signal write
//-------------------------------------------------

void z80dma_device::rdy_write_callback(int state)
{
	// normalize state
	m_rdy = state;
	m_status = (m_status & 0xFD) | (!is_ready() << 1);

	update_status();

	if (is_ready() && INT_ON_READY)
    {
		trigger_interrupt(INT_RDY);
    }
}


//-------------------------------------------------
//  rdy_w - ready input
//-------------------------------------------------

void z80dma_device::rdy_w(int state)
{
	if (LOG) logerror("RDY: %d Active High: %d\n", state, READY_ACTIVE_HIGH);
	timer_call_after_resynch(&m_machine, (void *)this, state, static_rdy_write_callback);
}


//-------------------------------------------------
//  wait_w - wait input
//-------------------------------------------------

void z80dma_device::wait_w(int state)
{
}


//-------------------------------------------------
//  bai_w - bus acknowledge input
//-------------------------------------------------

void z80dma_device::bai_w(int state)
{
	if (m_ius)
	{
	    if (LOG) logerror("Z80DMA '%s' Return from Interrupt\n", tag());

		/* clear interrupt under service flag */
		m_ius = 0;
	    interrupt_check();

		return;
	}

	logerror("z80dma_irq_reti: failed to find an interrupt to clear IEO on!\n");
}



//**************************************************************************
//  GLOBAL STUBS
//**************************************************************************

READ8_DEVICE_HANDLER( z80dma_r ) { return downcast<z80dma_device *>(device)->read(); }
WRITE8_DEVICE_HANDLER( z80dma_w ) { downcast<z80dma_device *>(device)->write(data); }

WRITE_LINE_DEVICE_HANDLER( z80dma_rdy_w ) { downcast<z80dma_device *>(device)->rdy_w(state); }
WRITE_LINE_DEVICE_HANDLER( z80dma_wait_w ) { downcast<z80dma_device *>(device)->wait_w(state); }
WRITE_LINE_DEVICE_HANDLER( z80dma_bai_w ) { downcast<z80dma_device *>(device)->bai_w(state); }

const device_type Z80DMA = z80dma_device_config::static_alloc_device_config;
