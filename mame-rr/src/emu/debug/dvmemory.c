/*********************************************************************

    dvmemory.c

    Memory debugger view.

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
#include "debugvw.h"
#include "dvmemory.h"
#include "debugcpu.h"
#include <ctype.h>



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const debug_view_memory::memory_view_pos debug_view_memory::s_memory_pos_table[9] =
{
	/* 0 bytes per chunk:                         */ {  0, { 0 } },
	/* 1 byte  per chunk: 00 11 22 33 44 55 66 77 */ {  3, { 0x04, 0x00, 0x80 } },
	/* 2 bytes per chunk:  0011  2233  4455  6677 */ {  6, { 0x8c, 0x0c, 0x08, 0x04, 0x00, 0x80 } },
	/* 3 bytes per chunk:                         */ {  0, { 0 } },
	/* 4 bytes per chunk:   00112233    44556677  */ { 12, { 0x9c, 0x9c, 0x1c, 0x18, 0x14, 0x10, 0x0c, 0x08, 0x04, 0x00, 0x80, 0x80 } },
	/* 5 bytes per chunk:                         */ {  0, { 0 } },
	/* 6 bytes per chunk:                         */ {  0, { 0 } },
	/* 7 bytes per chunk:                         */ {  0, { 0 } },
	/* 8 bytes per chunk:     0011223344556677    */ { 24, { 0xbc, 0xbc, 0xbc, 0xbc, 0x3c, 0x38, 0x34, 0x30, 0x2c, 0x28, 0x24, 0x20, 0x1c, 0x18, 0x14, 0x10, 0x0c, 0x08, 0x04, 0x00, 0x80, 0x80, 0x80, 0x80 } }
};



//**************************************************************************
//  DEBUG VIEW MEMORY SOURCE
//**************************************************************************

//-------------------------------------------------
//  debug_view_memory_source - constructors
//-------------------------------------------------

debug_view_memory_source::debug_view_memory_source(const char *name, const address_space &space)
	: debug_view_source(name, space.cpu),
	  m_space(&space),
	  m_memintf(dynamic_cast<device_memory_interface *>(space.cpu)),
	  m_base(NULL),
	  m_length(0),
	  m_offsetxor(0),
	  m_endianness(space.endianness),
	  m_prefsize(space.dbits / 8)
{
}

debug_view_memory_source::debug_view_memory_source(const char *name, const region_info &region)
	: debug_view_source(name),
	  m_space(NULL),
	  m_memintf(NULL),
	  m_base(region),
	  m_length(region.bytes()),
	  m_offsetxor(NATIVE_ENDIAN_VALUE_LE_BE(region.width() - 1, 0)),
	  m_endianness(region.endianness()),
	  m_prefsize(MIN(region.width(), 8))
{
}

debug_view_memory_source::debug_view_memory_source(const char *name, void *base, int element_size, int num_elements)
	: debug_view_source(name),
	  m_space(NULL),
	  m_memintf(NULL),
	  m_base(base),
	  m_length(element_size * num_elements),
	  m_offsetxor(0),
	  m_endianness(ENDIANNESS_NATIVE),
	  m_prefsize(MIN(element_size, 8))
{
}



//**************************************************************************
//  DEBUG VIEW MEMORY
//**************************************************************************

//-------------------------------------------------
//  debug_view_memory - constructor
//-------------------------------------------------

debug_view_memory::debug_view_memory(running_machine &machine, debug_view_osd_update_func osdupdate, void *osdprivate)
	: debug_view(machine, DVT_MEMORY, osdupdate, osdprivate),
	  m_expression(machine),
	  m_chunks_per_row(16),
	  m_bytes_per_chunk(1),
	  m_reverse_view(false),
	  m_ascii_view(true),
	  m_no_translation(false),
	  m_maxaddr(0),
	  m_bytes_per_row(16),
	  m_byte_offset(0)
{
	// fail if no available sources
	enumerate_sources();
	if (m_source_list.count() == 0)
		throw std::bad_alloc();

	// configure the view
	m_supports_cursor = true;
}


//-------------------------------------------------
//  enumerate_sources - enumerate all possible
//  sources for a memory view
//-------------------------------------------------

void debug_view_memory::enumerate_sources()
{
	// start with an empty list
	m_source_list.reset();
	astring name;

	// first add all the devices' address spaces
	device_memory_interface *memintf = NULL;
	for (bool gotone = m_machine.m_devicelist.first(memintf); gotone; gotone = memintf->next(memintf))
		for (int spacenum = 0; spacenum < ADDRESS_SPACES; spacenum++)
		{
			const address_space *space = memintf->space(spacenum);
			if (space != NULL)
			{
				name.printf("%s '%s' %s space memory", memintf->device().name(), memintf->device().tag(), space->name);
				m_source_list.append(*auto_alloc(&m_machine, debug_view_memory_source(name, *space)));
			}
		}

	// then add all the memory regions
	for (const region_info *region = m_machine.m_regionlist.first(); region != NULL; region = region->next())
	{
		name.printf("Region '%s'", region->name());
		m_source_list.append(*auto_alloc(&m_machine, debug_view_memory_source(name, *region)));
	}

	// finally add all global array symbols
	for (int itemnum = 0; itemnum < 10000; itemnum++)
	{
		// stop when we run out of items
		UINT32 valsize, valcount;
		void *base;
		const char *itemname = state_save_get_indexed_item(&m_machine, itemnum, &base, &valsize, &valcount);
		if (itemname == NULL)
			break;

		// if this is a single-entry global, add it
		if (valcount > 1 && strstr(itemname, "globals/"))
		{
			name.cpy(strrchr(itemname, '/') + 1);
			m_source_list.append(*auto_alloc(&m_machine, debug_view_memory_source(name, base, valsize, valcount)));
		}
	}

	// reset the source to a known good entry
	set_source(*m_source_list.head());
}


//-------------------------------------------------
//  view_notify - handle notification of updates
//  to cursor changes
//-------------------------------------------------

void debug_view_memory::view_notify(debug_view_notification type)
{
	if (type == VIEW_NOTIFY_CURSOR_CHANGED)
	{
		// normalize the cursor
		set_cursor_pos(get_cursor_pos());
	}
	else if (type == VIEW_NOTIFY_SOURCE_CHANGED)
	{
		// update for the new source
		const debug_view_memory_source &source = downcast<const debug_view_memory_source &>(*m_source);
		m_chunks_per_row = m_bytes_per_chunk * m_chunks_per_row / source.m_prefsize;
		m_bytes_per_chunk = source.m_prefsize;
		if (source.m_space != NULL)
			m_expression.set_context(source.m_space->cpu->debug()->symtable());
		else
			m_expression.set_context(NULL);
	}
}


//-------------------------------------------------
//  view_update - update the contents of the
//  memory view
//-------------------------------------------------

void debug_view_memory::view_update()
{
	const debug_view_memory_source &source = downcast<const debug_view_memory_source &>(*m_source);

	// if we need to recompute, do it now
	if (needs_recompute())
		recompute();

	// get positional data
	const memory_view_pos &posdata = s_memory_pos_table[m_bytes_per_chunk];

	// loop over visible rows
	for (UINT32 row = 0; row < m_visible.y; row++)
	{
		debug_view_char *destmin = m_viewdata + row * m_visible.x;
		debug_view_char *destmax = destmin + m_visible.x;
		debug_view_char *destrow = destmin - m_topleft.x;
		UINT32 effrow = m_topleft.y + row;

		// reset the line of data; section 1 is normal, others are ancillary, cursor is selected
		debug_view_char *dest = destmin;
		for (int ch = 0; ch < m_visible.x; ch++, dest++)
		{
			UINT32 effcol = m_topleft.x + ch;
			dest->byte = ' ';
			dest->attrib = DCA_ANCILLARY;
			if (m_section[1].contains(effcol))
			{
				dest->attrib = DCA_NORMAL;
				if (m_cursor_visible && effrow == m_cursor.y && effcol == m_cursor.x)
					dest->attrib |= DCA_SELECTED;
			}
		}

		// if this visible row is valid, add it to the buffer
		if (effrow < m_total.y)
		{
			offs_t addrbyte = m_byte_offset + effrow * m_bytes_per_row;
			offs_t address = (source.m_space != NULL) ? memory_byte_to_address(source.m_space, addrbyte) : addrbyte;
			char addrtext[20];

			// generate the address
			sprintf(addrtext, m_addrformat, address);
			dest = destrow + m_section[0].m_pos + 1;
			for (int ch = 0; addrtext[ch] != 0 && ch < m_section[0].m_width - 1; ch++, dest++)
				if (dest >= destmin && dest < destmax)
					dest->byte = addrtext[ch];

			// generate the data
			for (int chunknum = 0; chunknum < m_chunks_per_row; chunknum++)
			{
				int chunkindex = m_reverse_view ? (m_chunks_per_row - 1 - chunknum) : chunknum;

				UINT64 chunkdata;
				bool ismapped = read(m_bytes_per_chunk, addrbyte + chunknum * m_bytes_per_chunk, chunkdata);
				dest = destrow + m_section[1].m_pos + 1 + chunkindex * posdata.m_spacing;
				for (int ch = 0; ch < posdata.m_spacing; ch++, dest++)
					if (dest >= destmin && dest < destmax)
					{
						UINT8 shift = posdata.m_shift[ch];
						if (shift < 64)
							dest->byte = ismapped ? "0123456789ABCDEF"[(chunkdata >> shift) & 0x0f] : '*';
					}
			}

			// generate the ASCII data
			if (m_section[2].m_width > 0)
			{
				dest = destrow + m_section[2].m_pos + 1;
				for (int ch = 0; ch < m_bytes_per_row; ch++, dest++)
					if (dest >= destmin && dest < destmax)
					{
						UINT64 chval;
						bool ismapped = read(1, addrbyte + ch, chval);
						dest->byte = (ismapped && isprint(chval)) ? chval : '.';
					}
			}
		}
	}
}


//-------------------------------------------------
//  view_char - handle a character typed within
//  the current view
//-------------------------------------------------

void debug_view_memory::view_char(int chval)
{
	// get the position
	cursor_pos pos = get_cursor_pos();

	// handle the incoming key
	switch (chval)
	{
		case DCH_UP:
			if (pos.m_address >= m_byte_offset + m_bytes_per_row)
				pos.m_address -= m_bytes_per_row;
			break;

		case DCH_DOWN:
			if (pos.m_address <= m_maxaddr - m_bytes_per_row)
				pos.m_address += m_bytes_per_row;
			break;

		case DCH_PUP:
			for (UINT32 delta = (m_visible.y - 2) * m_bytes_per_row; delta > 0; delta -= m_bytes_per_row)
				if (pos.m_address >= m_byte_offset + delta)
				{
					pos.m_address -= delta;
					break;
				}
			break;

		case DCH_PDOWN:
			for (UINT32 delta = (m_visible.y - 2) * m_bytes_per_row; delta > 0; delta -= m_bytes_per_row)
				if (pos.m_address <= m_maxaddr - delta)
				{
					pos.m_address += delta;
					break;
				}
			break;

		case DCH_HOME:
			pos.m_address -= pos.m_address % m_bytes_per_row;
			pos.m_shift = (m_bytes_per_chunk * 8) - 4;
			break;

		case DCH_CTRLHOME:
			pos.m_address = m_byte_offset;
			pos.m_shift = (m_bytes_per_chunk * 8) - 4;
			break;

		case DCH_END:
			pos.m_address += (m_bytes_per_row - (pos.m_address % m_bytes_per_row) - 1);
			pos.m_shift = 0;
			break;

		case DCH_CTRLEND:
			pos.m_address = m_maxaddr;
			pos.m_shift = 0;
			break;

		case DCH_CTRLLEFT:
			if (pos.m_address >= m_byte_offset + m_bytes_per_chunk)
				pos.m_address -= m_bytes_per_chunk;
			break;

		case DCH_CTRLRIGHT:
			if (pos.m_address <= m_maxaddr - m_bytes_per_chunk)
				pos.m_address += m_bytes_per_chunk;
			break;

		default:
		{
			static const char hexvals[] = "0123456789abcdef";
			char *hexchar = (char *)strchr(hexvals, tolower(chval));
			if (hexchar == NULL)
				break;

			UINT64 data;
			bool ismapped = read(m_bytes_per_chunk, pos.m_address, data);
			if (!ismapped)
				break;

			data &= ~((UINT64)0x0f << pos.m_shift);
			data |= (UINT64)(hexchar - hexvals) << pos.m_shift;
			write(m_bytes_per_chunk, pos.m_address, data);
			// fall through...
		}

		case DCH_RIGHT:
			if (pos.m_shift == 0 && pos.m_address != m_maxaddr)
			{
				pos.m_shift = m_bytes_per_chunk * 8 - 4;
				pos.m_address += m_bytes_per_chunk;
			}
			else
				pos.m_shift -= 4;
			break;

		case DCH_LEFT:
			if (pos.m_shift == m_bytes_per_chunk * 8 - 4 && pos.m_address != m_byte_offset)
			{
				pos.m_shift = 0;
				pos.m_address -= m_bytes_per_chunk;
			}
			else
				pos.m_shift += 4;
			break;
	}

	// set a new position
	begin_update();
	set_cursor_pos(pos);
	m_update_pending = true;
	end_update();
}


//-------------------------------------------------
//  recompute - recompute the internal data and
//  structure of the memory view
//-------------------------------------------------

void debug_view_memory::recompute()
{
	const debug_view_memory_source &source = downcast<const debug_view_memory_source &>(*m_source);

	// get the current cursor position
	cursor_pos pos = get_cursor_pos();

	// determine the maximum address and address format string from the raw information
	int addrchars;
	if (source.m_space != NULL)
	{
		m_maxaddr = m_no_translation ? source.m_space->bytemask : source.m_space->logbytemask;
		addrchars = m_no_translation ? source.m_space->addrchars : source.m_space->logaddrchars;
	}
	else
	{
		m_maxaddr = source.m_length - 1;
		addrchars = m_addrformat.printf("%X", m_maxaddr);
	}

	// generate an 8-byte aligned format for the address
	if (!m_reverse_view)
		m_addrformat.printf("%*s%%0%dX", 8 - addrchars, "", addrchars);
	else
		m_addrformat.printf("%%0%dX%*s", addrchars, 8 - addrchars, "");

	// if we are viewing a space with a minimum chunk size, clamp the bytes per chunk
	if (source.m_space != NULL && source.m_space->ashift < 0)
	{
		UINT32 min_bytes_per_chunk = 1 << -source.m_space->ashift;
		while (m_bytes_per_chunk < min_bytes_per_chunk)
		{
			m_bytes_per_chunk *= 2;
			m_chunks_per_row /= 2;
		}
		m_chunks_per_row = MAX(1, m_chunks_per_row);
	}

	// recompute the byte offset based on the most recent expression result
	m_bytes_per_row = m_bytes_per_chunk * m_chunks_per_row;
	m_byte_offset = m_expression.value() % m_bytes_per_row;

	// compute the section widths
	m_section[0].m_width = 1 + 8 + 1;
	m_section[1].m_width = 1 + 3 * m_bytes_per_row + 1;
	m_section[2].m_width = m_ascii_view ? (1 + m_bytes_per_row + 1) : 0;

	// compute the section positions
	if (!m_reverse_view)
	{
		m_section[0].m_pos = 0;
		m_section[1].m_pos = m_section[0].m_pos + m_section[0].m_width;
		m_section[2].m_pos = m_section[1].m_pos + m_section[1].m_width;
		m_total.x = m_section[2].m_pos + m_section[2].m_width;
	}
	else
	{
		m_section[2].m_pos = 0;
		m_section[1].m_pos = m_section[2].m_pos + m_section[2].m_width;
		m_section[0].m_pos = m_section[1].m_pos + m_section[1].m_width;
		m_total.x = m_section[0].m_pos + m_section[0].m_width;
	}

	// derive total sizes from that
	m_total.y = ((UINT64)m_maxaddr - (UINT64)m_byte_offset + (UINT64)m_bytes_per_row - 1) / m_bytes_per_row;

	// reset the current cursor position
	set_cursor_pos(pos);
}


//-------------------------------------------------
//  needs_recompute - determine if anything has
//  changed that requires a recomputation
//-------------------------------------------------

bool debug_view_memory::needs_recompute()
{
	bool recompute = m_recompute;

	// handle expression changes
	if (m_expression.dirty())
	{
		recompute = true;
		m_topleft.y = (m_expression.value() - m_byte_offset) / m_bytes_per_row;
		m_topleft.y = MAX(m_topleft.y, 0);
		m_topleft.y = MIN(m_topleft.y, m_total.y - 1);

		const debug_view_memory_source &source = downcast<const debug_view_memory_source &>(*m_source);
		offs_t resultbyte;
		if (source.m_space != NULL)
			resultbyte  = memory_address_to_byte(source.m_space, m_expression.value()) & source.m_space->logbytemask;
		else
			resultbyte = m_expression.value();

		set_cursor_pos(cursor_pos(resultbyte, m_bytes_per_chunk * 8 - 4));
	}

	// expression is clean at this point, and future recomputation is not necessary
	m_recompute = false;
	return recompute;
}


//-------------------------------------------------
//  get_cursor_pos - return the cursor position as
//  an address and a shift value
//-------------------------------------------------

debug_view_memory::cursor_pos debug_view_memory::get_cursor_pos()
{
	// start with the base address for this row
	cursor_pos pos;
	pos.m_address = m_byte_offset + m_cursor.y * m_bytes_per_chunk * m_chunks_per_row;

	// determine the X position within the middle section, clamping as necessary
	const memory_view_pos &posdata = s_memory_pos_table[m_bytes_per_chunk];
	int xposition = m_cursor.x - m_section[1].m_pos - 1;
	if (xposition < 0)
		xposition = 0;
	else if (xposition >= posdata.m_spacing * m_chunks_per_row)
		xposition = posdata.m_spacing * m_chunks_per_row - 1;

	// compute chunk number and offset within that chunk
	int chunknum = xposition / posdata.m_spacing;
	int chunkoffs = xposition % posdata.m_spacing;

	// reverse the chunknum if we're reversed
	if (m_reverse_view)
		chunknum = m_chunks_per_row - 1 - chunknum;

	// compute the address and shift
	pos.m_address += chunknum * m_bytes_per_chunk;
	pos.m_shift = posdata.m_shift[chunkoffs] & 0x7f;
	return pos;
}


//-------------------------------------------------
//  set_cursor_pos - set the cursor position as a
//  function of an address and a shift value
//-------------------------------------------------

void debug_view_memory::set_cursor_pos(cursor_pos pos)
{
	const memory_view_pos &posdata = s_memory_pos_table[m_bytes_per_chunk];

	// offset the address by the byte offset
	if (pos.m_address < m_byte_offset)
		pos.m_address = m_byte_offset;
	pos.m_address -= m_byte_offset;

	// compute the Y coordinate and chunk index
	m_cursor.y = pos.m_address / m_bytes_per_row;
	int chunknum = (pos.m_address % m_bytes_per_row) / m_bytes_per_chunk;

	// reverse the chunknum if we're reversed
	if (m_reverse_view)
		chunknum = m_chunks_per_row - 1 - chunknum;

	// scan within the chunk to find the shift
	for (m_cursor.x = 0; m_cursor.x < posdata.m_spacing; m_cursor.x++)
		if (posdata.m_shift[m_cursor.x] == pos.m_shift)
			break;

	// add in the chunk offset and shift to the right of divider1
	m_cursor.x += m_section[1].m_pos + 1 + posdata.m_spacing * chunknum;

	// clamp to the window bounds
	m_cursor.x = MIN(m_cursor.x, m_total.x);
	m_cursor.y = MIN(m_cursor.y, m_total.y);

	// scroll if out of range
	adjust_visible_x_for_cursor();
	adjust_visible_y_for_cursor();
}


//-------------------------------------------------
//  read - generic memory view data reader
//-------------------------------------------------

bool debug_view_memory::read(UINT8 size, offs_t offs, UINT64 &data)
{
	const debug_view_memory_source &source = downcast<const debug_view_memory_source &>(*m_source);

	// if no raw data, just use the standard debug routines
	if (source.m_space != NULL)
	{
		offs_t dummyaddr = offs;

		bool ismapped = m_no_translation ? true : source.m_memintf->translate(source.m_space->spacenum, TRANSLATE_READ_DEBUG, dummyaddr);
		data = ~(UINT64)0;
		if (ismapped)
		{
			switch (size)
			{
				case 1:	data = debug_read_byte(source.m_space, offs, !m_no_translation); break;
				case 2:	data = debug_read_word(source.m_space, offs, !m_no_translation); break;
				case 4:	data = debug_read_dword(source.m_space, offs, !m_no_translation); break;
				case 8:	data = debug_read_qword(source.m_space, offs, !m_no_translation); break;
			}
		}
		return ismapped;
	}

	// if larger than a byte, reduce by half and recurse
	if (size > 1)
	{
		size /= 2;

		UINT64 data0, data1;
		bool ismapped = read(size, offs + 0 * size, data0);
		ismapped |= read(size, offs + 1 * size, data1);

		if (source.m_endianness == ENDIANNESS_LITTLE)
			data = data0 | (data1 << (size * 8));
		else
			data = data1 | (data0 << (size * 8));
		return ismapped;
	}

	// all 0xff if out of bounds
	offs ^= source.m_offsetxor;
	if (offs >= source.m_length)
		return false;
	data = *((UINT8 *)source.m_base + offs);
	return true;
}


//-------------------------------------------------
//  write - generic memory view data writer
//-------------------------------------------------

void debug_view_memory::write(UINT8 size, offs_t offs, UINT64 data)
{
	const debug_view_memory_source &source = downcast<const debug_view_memory_source &>(*m_source);

	// if no raw data, just use the standard debug routines
	if (source.m_space != NULL)
	{
		switch (size)
		{
			case 1:	debug_write_byte(source.m_space, offs, data, !m_no_translation); break;
			case 2:	debug_write_word(source.m_space, offs, data, !m_no_translation); break;
			case 4:	debug_write_dword(source.m_space, offs, data, !m_no_translation); break;
			case 8:	debug_write_qword(source.m_space, offs, data, !m_no_translation); break;
		}
		return;
	}

	// if larger than a byte, reduce by half and recurse
	if (size > 1)
	{
		size /= 2;
		if (source.m_endianness == ENDIANNESS_LITTLE)
		{
			write(size, offs + 0 * size, data);
			write(size, offs + 1 * size, data >> (8 * size));
		}
		else
		{
			write(size, offs + 1 * size, data);
			write(size, offs + 0 * size, data >> (8 * size));
		}
		return;
	}

	// ignore if out of bounds
	offs ^= source.m_offsetxor;
	if (offs >= source.m_length)
		return;
	*((UINT8 *)source.m_base + offs) = data;

// hack for FD1094 editing
#ifdef FD1094_HACK
	if (source.m_base == *m_machine.region("user2"))
	{
		extern void fd1094_regenerate_key(running_machine *machine);
		fd1094_regenerate_key(&m_machine);
	}
#endif
}


//-------------------------------------------------
//  set_expression - set the expression string
//  describing the home address
//-------------------------------------------------

void debug_view_memory::set_expression(const char *expression)
{
	begin_update();
	m_expression.set_string(expression);
	m_recompute = m_update_pending = true;
	end_update();
}


//-------------------------------------------------
//  set_bytes_per_chunk - specify the number of
//  bytes displayed per chunk
//-------------------------------------------------

void debug_view_memory::set_bytes_per_chunk(UINT8 chunkbytes)
{
	const debug_view_memory_source &source = downcast<const debug_view_memory_source &>(*m_source);

	cursor_pos pos = begin_update_and_get_cursor_pos();
	pos.m_address += (pos.m_shift / 8) ^ ((source.m_endianness == ENDIANNESS_LITTLE) ? 0 : (m_bytes_per_chunk - 1));
	pos.m_shift %= 8;

	m_bytes_per_chunk = chunkbytes;
	m_chunks_per_row = m_bytes_per_row / chunkbytes;
	m_recompute = m_update_pending = true;

	pos.m_shift += 8 * ((pos.m_address % m_bytes_per_chunk) ^ ((source.m_endianness == ENDIANNESS_LITTLE) ? 0 : (m_bytes_per_chunk - 1)));
	pos.m_address -= pos.m_address % m_bytes_per_chunk;

	m_recompute = m_update_pending = true;
	end_update_and_set_cursor_pos(pos);
}


//-------------------------------------------------
//  set_chunks_per_row - specify the number of
//  chunks displayed across a row
//-------------------------------------------------

void debug_view_memory::set_chunks_per_row(UINT32 rowchunks)
{
	if (rowchunks < 1)
		return;

	cursor_pos pos = begin_update_and_get_cursor_pos();
	m_chunks_per_row = rowchunks;
	m_recompute = m_update_pending = true;
	end_update_and_set_cursor_pos(pos);
}


//-------------------------------------------------
//  set_reverse - specify true if the memory view
//  is displayed reverse
//-------------------------------------------------

void debug_view_memory::set_reverse(bool reverse)
{
	cursor_pos pos = begin_update_and_get_cursor_pos();
	m_reverse_view = reverse;
	m_recompute = m_update_pending = true;
	end_update_and_set_cursor_pos(pos);
}


//-------------------------------------------------
//  set_ascii - specify TRUE if the memory view
//  should display an ASCII representation
//-------------------------------------------------

void debug_view_memory::set_ascii(bool ascii)
{
	cursor_pos pos = begin_update_and_get_cursor_pos();
	m_ascii_view = ascii;
	m_recompute = m_update_pending = true;
	end_update_and_set_cursor_pos(pos);
}


//-------------------------------------------------
//  set_physical - specify true if the memory view
//  should display physical addresses versus
//  logical addresses
//-------------------------------------------------

void debug_view_memory::set_physical(bool physical)
{
	cursor_pos pos = begin_update_and_get_cursor_pos();
	m_no_translation = physical;
	m_recompute = m_update_pending = true;
	end_update_and_set_cursor_pos(pos);
}
