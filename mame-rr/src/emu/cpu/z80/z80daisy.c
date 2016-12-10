/***************************************************************************

    z80daisy.c

    Z80/180 daisy chaining support functions.

***************************************************************************/

#include "emu.h"
#include "z80daisy.h"


//**************************************************************************
//  DEVICE CONFIG Z80 DAISY INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_config_z80daisy_interface - constructor
//-------------------------------------------------

device_config_z80daisy_interface::device_config_z80daisy_interface(const machine_config &mconfig, device_config &devconfig)
	: device_config_interface(mconfig, devconfig)
{
}


//-------------------------------------------------
//  ~device_config_z80daisy_interface - destructor
//-------------------------------------------------

device_config_z80daisy_interface::~device_config_z80daisy_interface()
{
}



//**************************************************************************
//  DEVICE Z80 DAISY INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_z80daisy_interface - constructor
//-------------------------------------------------

device_z80daisy_interface::device_z80daisy_interface(running_machine &machine, const device_config &config, device_t &device)
	: device_interface(machine, config, device),
	  m_z80daisy_config(dynamic_cast<const device_config_z80daisy_interface &>(config))
{
}


//-------------------------------------------------
//  ~device_z80daisy_interface - destructor
//-------------------------------------------------

device_z80daisy_interface::~device_z80daisy_interface()
{
}



//**************************************************************************
//  Z80 DAISY CHAIN
//**************************************************************************

//-------------------------------------------------
//  z80_daisy_chain - constructor
//-------------------------------------------------

z80_daisy_chain::z80_daisy_chain()
	: m_daisy_list(NULL)
{
}


//-------------------------------------------------
//  init - allocate the daisy chain based on the
//  provided configuration
//-------------------------------------------------

void z80_daisy_chain::init(device_t *cpudevice, const z80_daisy_config *daisy)
{
	// create a linked list of devices
	daisy_entry **tailptr = &m_daisy_list;
	for ( ; daisy->devname != NULL; daisy++)
	{
		// find the device
		device_t *target = cpudevice->siblingdevice(daisy->devname);
		if (target == NULL)
			fatalerror("Unable to locate device '%s'", daisy->devname);

		// make sure it has an interface
		device_z80daisy_interface *intf;
		if (!target->interface(intf))
			fatalerror("Device '%s' does not implement the z80daisy interface!", daisy->devname);

		// append to the end
		*tailptr = auto_alloc(cpudevice->machine, daisy_entry(target));
		tailptr = &(*tailptr)->m_next;
	}
}


//-------------------------------------------------
//  reset - send a reset signal to all chained
//  devices
//-------------------------------------------------

void z80_daisy_chain::reset()
{
	// loop over all devices and call their reset function
	for (daisy_entry *daisy = m_daisy_list; daisy != NULL; daisy = daisy->m_next)
		daisy->m_device->reset();
}


//-------------------------------------------------
//  update_irq_state - update the IRQ state and
//  return assert/clear based on the state
//-------------------------------------------------

int z80_daisy_chain::update_irq_state()
{
	// loop over all devices; dev[0] is highest priority
	for (daisy_entry *daisy = m_daisy_list; daisy != NULL; daisy = daisy->m_next)
	{
		// if this device is asserting the INT line, that's the one we want
		int state = daisy->m_interface->z80daisy_irq_state();
		if (state & Z80_DAISY_INT)
			return ASSERT_LINE;

		// if this device is asserting the IEO line, it blocks everyone else
		if (state & Z80_DAISY_IEO)
			return CLEAR_LINE;
	}
	return CLEAR_LINE;
}


//-------------------------------------------------
//  call_ack_device - acknowledge an interrupt
//  from a chained device and return the vector
//-------------------------------------------------

int z80_daisy_chain::call_ack_device()
{
	// loop over all devices; dev[0] is the highest priority
	for (daisy_entry *daisy = m_daisy_list; daisy != NULL; daisy = daisy->m_next)
	{
		// if this device is asserting the INT line, that's the one we want
		int state = daisy->m_interface->z80daisy_irq_state();
		if (state & Z80_DAISY_INT)
			return daisy->m_interface->z80daisy_irq_ack();
	}
	logerror("z80daisy_call_ack_device: failed to find an device to ack!\n");
	return 0;
}


//-------------------------------------------------
//  call_reti_device - signal a RETI operator to
//  the chain
//-------------------------------------------------

void z80_daisy_chain::call_reti_device()
{
	// loop over all devices; dev[0] is the highest priority
	for (daisy_entry *daisy = m_daisy_list; daisy != NULL; daisy = daisy->m_next)
	{
		// if this device is asserting the IEO line, that's the one we want
		int state = daisy->m_interface->z80daisy_irq_state();
		if (state & Z80_DAISY_IEO)
		{
			daisy->m_interface->z80daisy_irq_reti();
			return;
		}
	}
	logerror("z80daisy_call_reti_device: failed to find an device to reti!\n");
}


//-------------------------------------------------
//  daisy_entry - constructor
//-------------------------------------------------

z80_daisy_chain::daisy_entry::daisy_entry(device_t *device)
	: m_next(NULL),
	  m_device(device),
	  m_interface(NULL)
{
	device->interface(m_interface);
}
