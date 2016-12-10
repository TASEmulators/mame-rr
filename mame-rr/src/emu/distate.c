/***************************************************************************

    distate.c

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

#include "emu.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const UINT64 device_state_entry::k_decimal_divisor[] =
{
	1,
	10,
	100,
	1000,
	10000,
	100000,
	1000000,
	10000000,
	100000000,
	1000000000,
	U64(10000000000),
	U64(100000000000),
	U64(1000000000000),
	U64(10000000000000),
	U64(100000000000000),
	U64(1000000000000000),
	U64(10000000000000000),
	U64(100000000000000000),
	U64(1000000000000000000),
	U64(10000000000000000000)
};



//**************************************************************************
//  DEVICE STATE ENTRY
//**************************************************************************

//-------------------------------------------------
//  device_state_entry - constructor
//-------------------------------------------------

device_state_entry::device_state_entry(int index, const char *symbol, void *dataptr, UINT8 size)
	: m_next(NULL),
	  m_index(index),
	  m_datamask(0),
	  m_datasize(size),
	  m_flags(0),
	  m_symbol(symbol),
	  m_default_format(true),
	  m_sizemask(0)
{
	// set the data pointer
	m_dataptr.v = dataptr;

	// convert the size to a mask
	assert(size == 1 || size == 2 || size == 4 || size == 8);
	if (size == 1)
		m_sizemask = 0xff;
	else if (size == 2)
		m_sizemask = 0xffff;
	else if (size == 4)
		m_sizemask = 0xffffffff;
	else
		m_sizemask = ~U64(0);

	// default the data mask to the same
	m_datamask = m_sizemask;
	format_from_mask();

	// override well-known symbols
	if (index == STATE_GENPC)
		m_symbol.cpy("CURPC");
	else if (index == STATE_GENPCBASE)
		m_symbol.cpy("CURPCBASE");
	else if (index == STATE_GENSP)
		m_symbol.cpy("CURSP");
	else if (index == STATE_GENFLAGS)
		m_symbol.cpy("CURFLAGS");
}


//-------------------------------------------------
//  formatstr - specify a format string
//-------------------------------------------------

device_state_entry &device_state_entry::formatstr(const char *_format)
{
	m_format.cpy(_format);
	m_default_format = false;

	// set the DSF_CUSTOM_STRING flag by formatting with a NULL string
	m_flags &= ~DSF_CUSTOM_STRING;
	astring dummy;
	format(dummy, NULL);

	return *this;
}


//-------------------------------------------------
//  value - return the current value as a UINT64
//-------------------------------------------------

UINT64 device_state_entry::value() const
{
	// pick up the value
	UINT64 result = ~(UINT64)0;
	switch (m_datasize)
	{
		default:
		case 1:	result = *m_dataptr.u8;		break;
		case 2:	result = *m_dataptr.u16;	break;
		case 4:	result = *m_dataptr.u32;	break;
		case 8:	result = *m_dataptr.u64;	break;
	}
	return result & m_datamask;
}


//-------------------------------------------------
//  format - return the value of the given
//  pieces of indexed state as a string
//-------------------------------------------------

astring &device_state_entry::format(astring &dest, const char *string, bool maxout) const
{
	UINT64 result = value();

	// parse the format
	bool leadzero = false;
	bool percent = false;
	bool explicitsign = false;
	bool hitnonzero = false;
	bool reset = true;
	int width = 0;
	for (const char *fptr = m_format; *fptr != 0; fptr++)
	{
		// reset any accumulated state
		if (reset)
		{
			leadzero = maxout;
			percent = explicitsign = reset = false;
			width = 0;
		}

		// if we're not within a format, then anything other than a % outputs directly
		if (!percent && *fptr != '%')
		{
			dest.cat(fptr, 1);
			continue;
		}

		// handle each character in turn
		switch (*fptr)
		{
			// % starts a format; %% outputs a single %
			case '%':
				if (!percent)
					percent = true;
				else
				{
					dest.cat(fptr, 1);
					percent = false;
				}
				break;

			// 0 means insert leading 0s, unless it follows another width digit
			case '0':
				if (width == 0)
					leadzero = true;
				else
					width *= 10;
				break;

			// 1-9 accumulate into the width
			case '1':	case '2':	case '3':	case '4':	case '5':
			case '6':	case '7':	case '8':	case '9':
				width = width * 10 + (*fptr - '0');
				break;

			// + means explicit sign
			case '+':
				explicitsign = true;
				break;

			// X outputs as hexadecimal
			case 'X':
				if (width == 0)
					throw emu_fatalerror("Width required for %%X formats\n");
				hitnonzero = false;
				while (leadzero && width > 16)
				{
					dest.cat(" ");
					width--;
				}
				for (int digitnum = 15; digitnum >= 0; digitnum--)
				{
					int digit = (result >> (4 * digitnum)) & 0x0f;
					if (digit != 0)
					{
						static const char hexchars[] = "0123456789ABCDEF";
						dest.cat(&hexchars[digit], 1);
						hitnonzero = true;
					}
					else if (hitnonzero || (leadzero && digitnum < width) || digitnum == 0)
						dest.cat("0");
				}
				reset = true;
				break;

			// d outputs as signed decimal
			case 'd':
				if (width == 0)
					throw emu_fatalerror("Width required for %%d formats\n");
				if ((result & m_datamask) > (m_datamask >> 1))
				{
					result = -result & m_datamask;
					dest.cat("-");
					width--;
				}
				else if (explicitsign)
				{
					dest.cat("+");
					width--;
				}
				// fall through to unsigned case

			// u outputs as unsigned decimal
			case 'u':
				if (width == 0)
					throw emu_fatalerror("Width required for %%u formats\n");
				hitnonzero = false;
				while (leadzero && width > ARRAY_LENGTH(k_decimal_divisor))
				{
					dest.cat(" ");
					width--;
				}
				for (int digitnum = ARRAY_LENGTH(k_decimal_divisor) - 1; digitnum >= 0; digitnum--)
				{
					int digit = (result >= k_decimal_divisor[digitnum]) ? (result / k_decimal_divisor[digitnum]) % 10 : 0;
					if (digit != 0)
					{
						static const char decchars[] = "0123456789";
						dest.cat(&decchars[digit], 1);
						hitnonzero = true;
					}
					else if (hitnonzero || (leadzero && digitnum < width) || digitnum == 0)
						dest.cat("0");
				}
				reset = true;
				break;

			// s outputs a custom string
			case 's':
				if (width == 0)
					throw emu_fatalerror("Width required for %%s formats\n");
				if (string == NULL)
				{
					const_cast<device_state_entry *>(this)->m_flags |= DSF_CUSTOM_STRING;
					return dest;
				}
				if (strlen(string) <= width)
				{
					dest.cat(string);
					width -= strlen(string);
					while (width-- != 0)
						dest.cat(" ");
				}
				else
					dest.cat(string, width);
				reset = true;
				break;

			// other formats unknown
			default:
				throw emu_fatalerror("Unknown format character '%c'\n", *fptr);
				break;
		}
	}
	return dest;
}


//-------------------------------------------------
//  set_value - set the value from a UINT64
//-------------------------------------------------

void device_state_entry::set_value(UINT64 value) const
{
	// apply the mask
	value &= m_datamask;

	// sign-extend if necessary
	if ((m_flags & DSF_IMPORT_SEXT) != 0 && value > (m_datamask >> 1))
		value |= ~m_datamask;

	// store the value
	switch (m_datasize)
	{
		default:
		case 1:	*m_dataptr.u8 = value;		break;
		case 2:	*m_dataptr.u16 = value;		break;
		case 4:	*m_dataptr.u32 = value;		break;
		case 8:	*m_dataptr.u64 = value;		break;
	}
}


//-------------------------------------------------
//  set_value - set the value from a string
//-------------------------------------------------

void device_state_entry::set_value(const char *string) const
{
	// not implemented
}


//-------------------------------------------------
//  format_from_mask - make a format based on
//  the data mask
//-------------------------------------------------

void device_state_entry::format_from_mask()
{
	// skip if we have a user-provided format
	if (!m_default_format)
		return;

	// make up a format based on the mask
	int width = 0;
	for (UINT64 tempmask = m_datamask; tempmask != 0; tempmask >>= 4)
		width++;
	m_format.printf("%%0%dX", width);
}



//**************************************************************************
//  DEVICE CONFIG STATE INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_config_state_interface - constructor
//-------------------------------------------------

device_config_state_interface::device_config_state_interface(const machine_config &mconfig, device_config &devconfig)
	: device_config_interface(mconfig, devconfig)
{
}


//-------------------------------------------------
//  ~device_config_state_interface - destructor
//-------------------------------------------------

device_config_state_interface::~device_config_state_interface()
{
}



//**************************************************************************
//  DEVICE STATE INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_state_interface - constructor
//-------------------------------------------------

device_state_interface::device_state_interface(running_machine &machine, const device_config &config, device_t &device)
	: device_interface(machine, config, device),
	  m_machine(machine),
	  m_state_config(dynamic_cast<const device_config_state_interface &>(config)),
	  m_state_list(NULL)
{
	memset(m_fast_state, 0, sizeof(m_fast_state));
}


//-------------------------------------------------
//  ~device_state_interface - destructor
//-------------------------------------------------

device_state_interface::~device_state_interface()
{
}


//-------------------------------------------------
//  state - return the value of the given piece
//  of indexed state as a UINT64
//-------------------------------------------------

UINT64 device_state_interface::state(int index)
{
	// NULL or out-of-range entry returns 0
	const device_state_entry *entry = state_find_entry(index);
	if (entry == NULL)
		return 0;

	// call the exporter before we do anything
	if (entry->needs_export())
		state_export(*entry);

	// pick up the value
	return entry->value();
}


//-------------------------------------------------
//  state_string - return the value of the given
//  pieces of indexed state as a string
//-------------------------------------------------

astring &device_state_interface::state_string(int index, astring &dest)
{
	// NULL or out-of-range entry returns bogus string
	const device_state_entry *entry = state_find_entry(index);
	if (entry == NULL)
		return dest.cpy("???");

	// get the custom string if needed
	astring custom;
	if (entry->needs_custom_string())
		state_string_export(*entry, custom);

	// ask the entry to format itself
	return entry->format(dest, custom);
}


//-------------------------------------------------
//  state_string_max_length - return the maximum
//  length of the given state string
//-------------------------------------------------

int device_state_interface::state_string_max_length(int index)
{
	// NULL or out-of-range entry returns bogus string
	const device_state_entry *entry = state_find_entry(index);
	if (entry == NULL)
		return 3;

	// ask the entry to format itself maximally
	astring tempstring;
	return entry->format(tempstring, "", true).len();
}


//-------------------------------------------------
//  set_state - set the value of the given piece
//  of indexed state from a UINT64
//-------------------------------------------------

void device_state_interface::set_state(int index, UINT64 value)
{
	// NULL or out-of-range entry is a no-op
	const device_state_entry *entry = state_find_entry(index);
	if (entry == NULL)
		return;

	// set the value
	entry->set_value(value);

	// call the importer to finish up
	if (entry->needs_import())
		state_import(*entry);
}


//-------------------------------------------------
//  set_state - set the value of the given piece
//  of indexed state from a string
//-------------------------------------------------

void device_state_interface::set_state(int index, const char *string)
{
	// NULL or out-of-range entry is a no-op
	const device_state_entry *entry = state_find_entry(index);
	if (entry == NULL)
		return;

	// set the value
	entry->set_value(string);

	// call the importer to finish up
	if (entry->needs_import())
		state_import(*entry);
}


//-------------------------------------------------
//  state_import - called after new state is
//  written to perform any post-processing
//-------------------------------------------------

void device_state_interface::state_import(const device_state_entry &entry)
{
}


//-------------------------------------------------
//  state_export - called prior to new state
//  reading the state
//-------------------------------------------------

void device_state_interface::state_export(const device_state_entry &entry)
{
}


//-------------------------------------------------
//  state_string_import - called after new state is
//  written to perform any post-processing
//-------------------------------------------------

void device_state_interface::state_string_import(const device_state_entry &entry, astring &string)
{
}


//-------------------------------------------------
//  state_string_export - called after new state is
//  written to perform any post-processing
//-------------------------------------------------

void device_state_interface::state_string_export(const device_state_entry &entry, astring &string)
{
}


//-------------------------------------------------
//  state_add - return the value of the given
//  pieces of indexed state as a UINT64
//-------------------------------------------------

device_state_entry &device_state_interface::state_add(int index, const char *symbol, void *data, UINT8 size)
{
	// assert validity of incoming parameters
	assert(size == 1 || size == 2 || size == 4 || size == 8);
	assert(symbol != NULL);

	// allocate new entry
	device_state_entry *entry = auto_alloc(&m_machine, device_state_entry(index, symbol, data, size));

	// append to the end of the list
	device_state_entry **tailptr;
	for (tailptr = &m_state_list; *tailptr != NULL; tailptr = &(*tailptr)->m_next) ;
	*tailptr = entry;

	// set the fast entry if applicable
	if (index >= k_fast_state_min && index <= k_fast_state_max)
		m_fast_state[index - k_fast_state_min] = entry;

	return *entry;
}


//-------------------------------------------------
//  state_find_entry - return a pointer to the
//  state entry for the given index
//-------------------------------------------------

const device_state_entry *device_state_interface::state_find_entry(int index)
{
	// use fast lookup if possible
	if (index >= k_fast_state_min && index <= k_fast_state_max)
		return m_fast_state[index - k_fast_state_min];

	// otherwise, scan the first
	for (const device_state_entry *entry = m_state_list; entry != NULL; entry = entry->m_next)
		if (entry->m_index == index)
			return entry;

	// handle failure by returning NULL
	return NULL;
}


//-------------------------------------------------
//  interface_post_start - verify that state was
//  properly set up
//-------------------------------------------------

void device_state_interface::interface_post_start()
{
	// make sure we got something during startup
	if (m_state_list == NULL)
		throw emu_fatalerror("No state registered for device '%s' that supports it!", m_device.tag());
}
