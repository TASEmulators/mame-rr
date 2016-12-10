/***************************************************************************

    distate.h

    Device state interfaces.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __DISTATE_H__
#define __DISTATE_H__


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// standard state indexes
enum
{
	STATE_GENPC = -1,				// generic program counter (live)
	STATE_GENPCBASE = -2,			// generic program counter (base of current instruction)
	STATE_GENSP = -3,				// generic stack pointer
	STATE_GENFLAGS = -4				// generic flags
};



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> device_state_entry

// class describing a single item of exposed device state
class device_state_entry
{
	friend class device_state_interface;

private:
	// construction/destruction
	device_state_entry(int index, const char *symbol, void *dataptr, UINT8 size);

public:
	// post-construction modifiers
	device_state_entry &mask(UINT64 _mask) { m_datamask = _mask; format_from_mask(); return *this; }
	device_state_entry &signed_mask(UINT64 _mask) { m_datamask = _mask; m_flags |= DSF_IMPORT_SEXT; format_from_mask(); return *this; }
	device_state_entry &formatstr(const char *_format);
	device_state_entry &callimport() { m_flags |= DSF_IMPORT; return *this; }
	device_state_entry &callexport() { m_flags |= DSF_EXPORT; return *this; }
	device_state_entry &noshow() { m_flags |= DSF_NOSHOW; return *this; }

	// iteration helpers
	const device_state_entry *next() const { return m_next; }

	// query information
	int index() const { return m_index; }
	void *dataptr() const { return m_dataptr.v; }
	const char *symbol() const { return m_symbol; }
	bool visible() const { return ((m_flags & DSF_NOSHOW) == 0); }

protected:
	// device state flags
	static const UINT8 DSF_NOSHOW =			0x01;	// don't display this entry in the registers view
	static const UINT8 DSF_IMPORT =			0x02;	// call the import function after writing new data
	static const UINT8 DSF_IMPORT_SEXT =	0x04;	// sign-extend the data when writing new data
	static const UINT8 DSF_EXPORT =			0x08;	// call the export function prior to fetching the data
	static const UINT8 DSF_CUSTOM_STRING =	0x10;	// set if the format has a custom string

	// helpers
	bool needs_custom_string() const { return ((m_flags & DSF_CUSTOM_STRING) != 0); }
	void format_from_mask();

	// return the current value -- only for our friends who handle export
	bool needs_export() const { return ((m_flags & DSF_EXPORT) != 0); }
	UINT64 value() const;
	astring &format(astring &dest, const char *string, bool maxout = false) const;

	// set the current value -- only for our friends who handle import
	bool needs_import() const { return ((m_flags & DSF_IMPORT) != 0); }
	void set_value(UINT64 value) const;
	void set_value(const char *string) const;

	// statics
	static const UINT64 k_decimal_divisor[20];		// divisors for outputting decimal values

	// public state description
	device_state_entry *	m_next;					// link to next item
	UINT32					m_index;				// index by which this item is referred
	generic_ptr				m_dataptr;				// pointer to where the data lives
	UINT64					m_datamask;				// mask that applies to the data
	UINT8					m_datasize;				// size of the data
	UINT8					m_flags;				// flags for this data
	astring					m_symbol;				// symbol for display; all lower-case version for expressions
	astring					m_format;				// supported formats
	bool					m_default_format;		// true if we are still using default format
	UINT64					m_sizemask;				// mask derived from the data size
};



// ======================> device_config_state_interface

// class representing interface-specific configuration state
class device_config_state_interface : public device_config_interface
{
public:
	// construction/destruction
	device_config_state_interface(const machine_config &mconfig, device_config &device);
	virtual ~device_config_state_interface();
};



// ======================> device_state_interface

// class representing interface-specific live state
class device_state_interface : public device_interface
{
public:
	// construction/destruction
	device_state_interface(running_machine &machine, const device_config &config, device_t &device);
	virtual ~device_state_interface();

	// configuration access
	const device_config_state_interface &state_config() const { return m_state_config; }
	const device_state_entry *state_first() const { return m_state_list; }

	// state getters
	UINT64 state(int index);
	offs_t pc() { return state(STATE_GENPC); }
	offs_t pcbase() { return state(STATE_GENPCBASE); }
	offs_t sp() { return state(STATE_GENSP); }
	UINT64 flags() { return state(STATE_GENFLAGS); }
	astring &state_string(int index, astring &dest);
	int state_string_max_length(int index);

	// state setters
	void set_state(int index, UINT64 value);
	void set_state(int index, const char *string);

public:	// protected eventually

	// add a new state item
	template<class T> device_state_entry &state_add(int index, const char *symbol, T &data)
	{
		return state_add(index, symbol, &data, sizeof(data));
	}
	device_state_entry &state_add(int index, const char *symbol, void *data, UINT8 size);

protected:
	// derived class overrides
	virtual void state_import(const device_state_entry &entry);
	virtual void state_export(const device_state_entry &entry);
	virtual void state_string_import(const device_state_entry &entry, astring &string);
	virtual void state_string_export(const device_state_entry &entry, astring &string);

	// internal operation overrides
	virtual void interface_post_start();

	// find the entry for a given index
	const device_state_entry *state_find_entry(int index);

	// constants
	static const int k_fast_state_min = -4;							// range for fast state
	static const int k_fast_state_max = 256;						// lookups

	// state
	running_machine &						m_machine;				// reference to owning machine
	const device_config_state_interface &	m_state_config;			// reference to configuration data
	device_state_entry *					m_state_list;			// head of state list
	device_state_entry *					m_fast_state[k_fast_state_max  + 1 - k_fast_state_min];
																	// fast access to common entries
};



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  device_state - return a pointer to the device
//  state interface for this device
//-------------------------------------------------

inline device_state_interface *device_state(device_t *device)
{
	device_state_interface *intf;
	if (!device->interface(intf))
		throw emu_fatalerror("Device '%s' does not have state interface", device->tag());
	return intf;
}


#endif	/* __DISTATE_H__ */
