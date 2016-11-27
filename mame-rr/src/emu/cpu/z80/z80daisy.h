/***************************************************************************

    z80daisy.h

    Z80/180 daisy chaining support functions.

***************************************************************************/

#pragma once

#ifndef __Z80DAISY_H__
#define __Z80DAISY_H__



//**************************************************************************
//  CONSTANTS
//**************************************************************************

// these constants are returned from the irq_state function
const UINT8 Z80_DAISY_INT = 0x01;		// interrupt request mask
const UINT8 Z80_DAISY_IEO =	0x02;		// interrupt disable mask (IEO)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> z80_daisy_config

struct z80_daisy_config
{
	const char *	devname;					// name of the device
};



// ======================> device_config_z80daisy_interface

// device_config_z80daisy_interface represents configuration information for a z80daisy device
class device_config_z80daisy_interface : public device_config_interface
{
public:
	// construction/destruction
	device_config_z80daisy_interface(const machine_config &mconfig, device_config &devconfig);
	virtual ~device_config_z80daisy_interface();
};



// ======================> device_z80daisy_interface

class device_z80daisy_interface : public device_interface
{
public:
	// construction/destruction
	device_z80daisy_interface(running_machine &machine, const device_config &config, device_t &device);
	virtual ~device_z80daisy_interface();

	// required operation overrides
	virtual int z80daisy_irq_state() = 0;
	virtual int z80daisy_irq_ack() = 0;
	virtual void z80daisy_irq_reti() = 0;

protected:
	const device_config_z80daisy_interface &m_z80daisy_config;
};



// ======================> z80_daisy_chain

class z80_daisy_chain
{
public:
	z80_daisy_chain();
	void init(device_t *cpudevice, const z80_daisy_config *daisy);

	bool present() const { return (m_daisy_list != NULL); }

	void reset();
	int update_irq_state();
	int call_ack_device();
	void call_reti_device();

protected:
	class daisy_entry
	{
	public:
		daisy_entry(device_t *device);

		daisy_entry *				m_next;			// next device
		device_t *					m_device;		// associated device
		device_z80daisy_interface *	m_interface;	// associated device's daisy interface
	};

	daisy_entry *			m_daisy_list;	// head of the daisy chain
};


#endif
