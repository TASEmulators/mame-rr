/*********************************************************************

    dvdisasm.c

    Disassembly debugger view.

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
#include "dvdisasm.h"
#include "debugcmt.h"
#include "debugcpu.h"



//**************************************************************************
//  DEBUG VIEW DISASM SOURCE
//**************************************************************************

//-------------------------------------------------
//  debug_view_disasm_source - constructor
//-------------------------------------------------

debug_view_disasm_source::debug_view_disasm_source(const char *name, device_t &device)
	: debug_view_source(name, &device),
	  m_device(device),
	  m_disasmintf(dynamic_cast<device_disasm_interface *>(&device)),
	  m_space(dynamic_cast<device_memory_interface *>(&device)->space(AS_PROGRAM))
{
}



//**************************************************************************
//  DEBUG VIEW DISASM
//**************************************************************************

//-------------------------------------------------
//  debug_view_disasm - constructor
//-------------------------------------------------

debug_view_disasm::debug_view_disasm(running_machine &machine, debug_view_osd_update_func osdupdate, void *osdprivate)
	: debug_view(machine, DVT_DISASSEMBLY, osdupdate, osdprivate),
	  m_right_column(DASM_RIGHTCOL_RAW),
	  m_backwards_steps(3),
	  m_dasm_width(DEFAULT_DASM_WIDTH),
	  m_last_direct_raw(NULL),
	  m_last_direct_decrypted(NULL),
	  m_last_change_count(0),
	  m_last_pcbyte(0),
	  m_divider1(0),
	  m_divider2(0),
	  m_divider3(0),
	  m_expression(machine),
	  m_allocated(0,0),
	  m_byteaddress(NULL),
	  m_dasm(NULL)
{
	// fail if no available sources
	enumerate_sources();
	if (m_source_list.count() == 0)
		throw std::bad_alloc();

	// count the number of comments
	int total_comments = 0;
	for (const debug_view_source *source = m_source_list.head(); source != NULL; source = source->next())
	{
		const debug_view_disasm_source &dasmsource = downcast<const debug_view_disasm_source &>(*source);
		total_comments += debug_comment_get_count(&dasmsource.m_device);
	}

	// initialize
	if (total_comments > 0)
		m_right_column = DASM_RIGHTCOL_COMMENTS;

	// configure the view
	m_total.y = DEFAULT_DASM_LINES;
	m_supports_cursor = true;
}


//-------------------------------------------------
//  ~debug_view_disasm - destructor
//-------------------------------------------------

debug_view_disasm::~debug_view_disasm()
{
	auto_free(&m_machine, m_byteaddress);
	auto_free(&m_machine, m_dasm);
}


//-------------------------------------------------
//  enumerate_sources - enumerate all possible
//  sources for a disassembly view
//-------------------------------------------------

void debug_view_disasm::enumerate_sources()
{
	// start with an empty list
	m_source_list.reset();

	// iterate over devices with disassembly interfaces
	device_disasm_interface *dasm = NULL;
	astring name;
	for (bool gotone = m_machine.m_devicelist.first(dasm); gotone; gotone = dasm->next(dasm))
	{
		name.printf("%s '%s'", dasm->device().name(), dasm->device().tag());
		m_source_list.append(*auto_alloc(&m_machine, debug_view_disasm_source(name, dasm->device())));
	}

	// reset the source to a known good entry
	set_source(*m_source_list.head());
}


//-------------------------------------------------
//  view_notify - handle notification of updates
//  to cursor changes
//-------------------------------------------------

void debug_view_disasm::view_notify(debug_view_notification type)
{
	if (type == VIEW_NOTIFY_CURSOR_CHANGED)
		adjust_visible_y_for_cursor();

	else if (type == VIEW_NOTIFY_SOURCE_CHANGED)
		m_expression.set_context(downcast<const debug_view_disasm_source *>(m_source)->device().debug()->symtable());
}


//-------------------------------------------------
//  view_char - handle a character typed within
//  the current view
//-------------------------------------------------

void debug_view_disasm::view_char(int chval)
{
	debug_view_xy origcursor = m_cursor;
	UINT8 end_buffer = 3;
	INT32 temp;

	switch (chval)
	{
		case DCH_UP:
			if (m_cursor.y > 0)
				m_cursor.y--;
			break;

		case DCH_DOWN:
			if (m_cursor.y < m_total.y - 1)
				m_cursor.y++;
			break;

		case DCH_PUP:
			temp = m_cursor.y - (m_visible.y - end_buffer);
			if (temp < 0)
				m_cursor.y = 0;
			else
				m_cursor.y = temp;
			break;

		case DCH_PDOWN:
			temp = m_cursor.y + (m_visible.y - end_buffer);
			if (temp > m_total.y - 1)
				m_cursor.y = m_total.y - 1;
			else
				m_cursor.y = temp;
			break;

		case DCH_HOME:				// set the active column to the PC
		{
			const debug_view_disasm_source &source = downcast<const debug_view_disasm_source &>(*m_source);
			offs_t pc = memory_address_to_byte(source.m_space, cpu_get_pc(&source.m_device)) & source.m_space->logbytemask;

			// figure out which row the pc is on
			for (int curline = 0; curline < m_allocated.y; curline++)
				if (m_byteaddress[curline] == pc)
					m_cursor.y = curline;
			break;
		}

		case DCH_CTRLHOME:
			m_cursor.y = 0;
			break;

		case DCH_CTRLEND:
			m_cursor.y = m_total.y - 1;
			break;
	}

	/* send a cursor changed notification */
	if (m_cursor.y != origcursor.y)
	{
		begin_update();
		view_notify(VIEW_NOTIFY_CURSOR_CHANGED);
		m_update_pending = true;
		end_update();
	}
}


//-------------------------------------------------
//  find_pc_backwards - back up the specified
//  number of instructions from the given PC
//-------------------------------------------------

offs_t debug_view_disasm::find_pc_backwards(offs_t targetpc, int numinstrs)
{
	const debug_view_disasm_source &source = downcast<const debug_view_disasm_source &>(*m_source);

	// compute the increment
	int minlen = memory_byte_to_address(source.m_space, source.m_disasmintf->min_opcode_bytes());
	if (minlen == 0) minlen = 1;
	int maxlen = memory_byte_to_address(source.m_space, source.m_disasmintf->max_opcode_bytes());
	if (maxlen == 0) maxlen = 1;

	// start off numinstrs back
	offs_t curpc = targetpc - minlen * numinstrs;
	if (curpc > targetpc)
		curpc = 0;

	/* loop until we find what we are looking for */
	offs_t targetpcbyte = memory_address_to_byte(source.m_space, targetpc) & source.m_space->logbytemask;
	offs_t fillpcbyte = targetpcbyte;
	offs_t lastgoodpc = targetpc;
	while (1)
	{
		// fill the buffer up to the target
		offs_t curpcbyte = memory_address_to_byte(source.m_space, curpc) & source.m_space->logbytemask;
		UINT8 opbuf[1024], argbuf[1024];
		while (curpcbyte < fillpcbyte)
		{
			fillpcbyte--;
			opbuf[1000 + fillpcbyte - targetpcbyte] = debug_read_opcode(source.m_space, fillpcbyte, 1, FALSE);
			argbuf[1000 + fillpcbyte - targetpcbyte] = debug_read_opcode(source.m_space, fillpcbyte, 1, TRUE);
		}

		// loop until we get past the target instruction
		int instcount = 0;
		int instlen;
		offs_t scanpc;
		for (scanpc = curpc; scanpc < targetpc; scanpc += instlen)
		{
			offs_t scanpcbyte = memory_address_to_byte(source.m_space, scanpc) & source.m_space->logbytemask;
			offs_t physpcbyte = scanpcbyte;

			// get the disassembly, but only if mapped
			instlen = 1;
			if (debug_cpu_translate(source.m_space, TRANSLATE_FETCH, &physpcbyte))
			{
				char dasmbuffer[100];
				instlen = source.m_disasmintf->disassemble(dasmbuffer, scanpc, &opbuf[1000 + scanpcbyte - targetpcbyte], &argbuf[1000 + scanpcbyte - targetpcbyte]) & DASMFLAG_LENGTHMASK;
			}

			// count this one
			instcount++;
		}

		// if we ended up right on targetpc, this is a good candidate
		if (scanpc == targetpc && instcount <= numinstrs)
			lastgoodpc = curpc;

		// we're also done if we go back too far
		if (targetpc - curpc >= numinstrs * maxlen)
			break;

		// and if we hit 0, we're done
		if (curpc == 0)
			break;

		// back up one more and try again
		curpc -= minlen;
		if (curpc > targetpc)
			curpc = 0;
	}

	return lastgoodpc;
}


//-------------------------------------------------
//  generate_bytes - generate the opcode byte
//  values
//-------------------------------------------------

void debug_view_disasm::generate_bytes(offs_t pcbyte, int numbytes, int minbytes, char *string, int maxchars, bool encrypted)
{
	const debug_view_disasm_source &source = downcast<const debug_view_disasm_source &>(*m_source);

	// output the first value
	int offset = 0;
	if (maxchars >= 2 * minbytes)
		offset = sprintf(string, "%s", core_i64_hex_format(debug_read_opcode(source.m_space, pcbyte, minbytes, FALSE), minbytes * 2));

	// output subsequent values
	int byte;
	for (byte = minbytes; byte < numbytes && offset + 1 + 2 * minbytes < maxchars; byte += minbytes)
		offset += sprintf(&string[offset], " %s", core_i64_hex_format(debug_read_opcode(source.m_space, pcbyte + byte, minbytes, encrypted), minbytes * 2));

	// if we ran out of room, indicate more
	string[maxchars - 1] = 0;
	if (byte < numbytes && maxchars > 3)
		string[maxchars - 2] = string[maxchars - 3] = string[maxchars - 4] = '.';
}


//-------------------------------------------------
//  recompute - recompute selected info for the
//  disassembly view
//-------------------------------------------------

bool debug_view_disasm::recompute(offs_t pc, int startline, int lines)
{
	bool changed = false;
	const debug_view_disasm_source &source = downcast<const debug_view_disasm_source &>(*m_source);

	// determine how many characters we need for an address and set the divider
	m_divider1 = 1 + source.m_space->logaddrchars + 1;

	// assume a fixed number of characters for the disassembly
	m_divider2 = m_divider1 + 1 + m_dasm_width + 1;

	// determine how many bytes we might need to display
	int minbytes = source.m_disasmintf->min_opcode_bytes();
	int maxbytes = source.m_disasmintf->max_opcode_bytes();

	// ensure that the PC is aligned to the minimum opcode size
	pc &= ~memory_byte_to_address_end(source.m_space, minbytes - 1);

	// set the width of the third column according to display mode
	if (m_right_column == DASM_RIGHTCOL_RAW || m_right_column == DASM_RIGHTCOL_ENCRYPTED)
	{
		int maxbytes_clamped = MIN(maxbytes, DASM_MAX_BYTES);
		m_total.x = m_divider2 + 1 + 2 * maxbytes_clamped + (maxbytes_clamped / minbytes - 1) + 1;
	}
	else if (m_right_column == DASM_RIGHTCOL_COMMENTS)
		m_total.x = m_divider2 + 1 + 50;		// DEBUG_COMMENT_MAX_LINE_LENGTH
	else
		m_total.x = m_divider2 + 1;

	// reallocate memory if we don't have enough
	if (m_allocated.x < m_total.x || m_allocated.y < m_total.y)
	{
		// update our values
		m_allocated = m_total;

		// allocate address array
		auto_free(&m_machine, m_byteaddress);
		m_byteaddress = auto_alloc_array(&m_machine, offs_t, m_allocated.y);

		// allocate disassembly buffer
		auto_free(&m_machine, m_dasm);
		m_dasm = auto_alloc_array(&m_machine, char, m_allocated.x * m_allocated.y);
	}

	// iterate over lines
	for (int line = 0; line < lines; line++)
	{
		// convert PC to a byte offset
		offs_t pcbyte = memory_address_to_byte(source.m_space, pc) & source.m_space->logbytemask;

		// save a copy of the previous line as a backup if we're only doing one line
		int instr = startline + line;
		char *destbuf = &m_dasm[instr * m_allocated.x];
		char oldbuf[100];
		if (lines == 1)
			strncpy(oldbuf, destbuf, MIN(sizeof(oldbuf), m_allocated.x));

		// convert back and set the address of this instruction
		m_byteaddress[instr] = pcbyte;
		sprintf(&destbuf[0], " %s  ", core_i64_hex_format(memory_byte_to_address(source.m_space, pcbyte), source.m_space->logaddrchars));

		// make sure we can translate the address, and then disassemble the result
		char buffer[100];
		int numbytes = 0;
		offs_t physpcbyte = pcbyte;
		if (debug_cpu_translate(source.m_space, TRANSLATE_FETCH_DEBUG, &physpcbyte))
		{
			UINT8 opbuf[64], argbuf[64];

			// fetch the bytes up to the maximum
			for (numbytes = 0; numbytes < maxbytes; numbytes++)
			{
				opbuf[numbytes] = debug_read_opcode(source.m_space, pcbyte + numbytes, 1, FALSE);
				argbuf[numbytes] = debug_read_opcode(source.m_space, pcbyte + numbytes, 1, TRUE);
			}

			// disassemble the result
			pc += numbytes = source.m_disasmintf->disassemble(buffer, pc & source.m_space->logaddrmask, opbuf, argbuf) & DASMFLAG_LENGTHMASK;
		}
		else
			strcpy(buffer, "<unmapped>");

		// append the disassembly to the buffer
		sprintf(&destbuf[m_divider1 + 1], "%-*s  ", m_dasm_width, buffer);

		// output the right column
		if (m_right_column == DASM_RIGHTCOL_RAW || m_right_column == DASM_RIGHTCOL_ENCRYPTED)
		{
			// get the bytes
			numbytes = memory_address_to_byte(source.m_space, numbytes) & source.m_space->logbytemask;
			generate_bytes(pcbyte, numbytes, minbytes, &destbuf[m_divider2], m_allocated.x - m_divider2, m_right_column == DASM_RIGHTCOL_ENCRYPTED);
		}
		else if (m_right_column == DASM_RIGHTCOL_COMMENTS)
		{
			// get and add the comment, if present
			offs_t comment_address = memory_byte_to_address(source.m_space, m_byteaddress[instr]);
			const char *text = debug_comment_get_text(&source.m_device, comment_address, debug_comment_get_opcode_crc32(&source.m_device, comment_address));
			if (text != NULL)
				sprintf(&destbuf[m_divider2], "// %.*s", m_allocated.x - m_divider2 - 1, text);
		}

		// see if the line changed at all
		if (lines == 1 && strncmp(oldbuf, destbuf, MIN(sizeof(oldbuf), m_allocated.x)) != 0)
			changed = true;
	}

	// update opcode base information
	m_last_direct_decrypted = source.m_space->direct.decrypted;
	m_last_direct_raw = source.m_space->direct.raw;
	m_last_change_count = debug_comment_all_change_count(&m_machine);

	// now longer need to recompute
	m_recompute = false;
	return changed;
}


//-------------------------------------------------
//  view_update - update the contents of the
//  disassembly view
//-------------------------------------------------

void debug_view_disasm::view_update()
{
	const debug_view_disasm_source &source = downcast<const debug_view_disasm_source &>(*m_source);

	offs_t pc = cpu_get_pc(&source.m_device);
	offs_t pcbyte = memory_address_to_byte(source.m_space, pc) & source.m_space->logbytemask;

	// update our context; if the expression is dirty, recompute
	if (m_expression.dirty())
		m_recompute = true;

	// if we're tracking a value, make sure it is visible
	UINT64 previous = m_expression.last_value();
	UINT64 result = m_expression.value();
	if (result != previous)
	{
		offs_t resultbyte = memory_address_to_byte(source.m_space, result) & source.m_space->logbytemask;

		// see if the new result is an address we already have
		UINT32 row;
		for (row = 0; row < m_allocated.y; row++)
			if (m_byteaddress[row] == resultbyte)
				break;

		// if we didn't find it, or if it's really close to the bottom, recompute
		if (row == m_allocated.y || row >= m_total.y - m_visible.y)
			m_recompute = true;

		// otherwise, if it's not visible, adjust the view so it is
		else if (row < m_topleft.y || row >= m_topleft.y + m_visible.y - 2)
			m_topleft.y = (row > 3) ? row - 3 : 0;
	}

	// if the opcode base has changed, rework things
	if (source.m_space->direct.decrypted != m_last_direct_decrypted || source.m_space->direct.raw != m_last_direct_raw)
		m_recompute = true;

	// if the comments have changed, redo it
	if (m_last_change_count != debug_comment_all_change_count(&m_machine))
		m_recompute = true;

	// if we need to recompute, do it
	bool recomputed_this_time = false;
recompute:
	if (m_recompute)
	{
		// recompute the view
		if (m_byteaddress != NULL && m_last_change_count != debug_comment_all_change_count(&m_machine))
		{
			// smoosh us against the left column, but not the top row
			m_topleft.x = 0;

			// recompute from where we last recomputed!
			recompute(memory_byte_to_address(source.m_space, m_byteaddress[0]), 0, m_total.y);
		}
		else
		{
			// determine the addresses of what we will display
			offs_t backpc = find_pc_backwards((UINT32)m_expression.value(), m_backwards_steps);

			// put ourselves back in the top left
			m_topleft.y = 0;
			m_topleft.x = 0;

			recompute(backpc, 0, m_total.y);
		}
		recomputed_this_time = true;
	}

	// figure out the row where the PC is and recompute the disassembly
	if (pcbyte != m_last_pcbyte)
	{
		// find the row with the PC on it
		for (UINT32 row = 0; row < m_visible.y; row++)
		{
			UINT32 effrow = m_topleft.y + row;
			if (effrow >= m_allocated.y)
				break;
			if (pcbyte == m_byteaddress[effrow])
			{
				// see if we changed
				bool changed = recompute(pc, effrow, 1);
				if (changed && !recomputed_this_time)
				{
					m_recompute = true;
					goto recompute;
				}

				// set the effective row and PC
				m_cursor.y = effrow;
				view_notify(VIEW_NOTIFY_CURSOR_CHANGED);
			}
		}
		m_last_pcbyte = pcbyte;
	}

	// loop over visible rows
	debug_view_char *dest = m_viewdata;
	for (UINT32 row = 0; row < m_visible.y; row++)
	{
		UINT32 effrow = m_topleft.y + row;
		UINT32 col = 0;

		// if this visible row is valid, add it to the buffer
		UINT8 attrib = DCA_NORMAL;
		if (effrow < m_allocated.y)
		{
			// if we're on the line with the PC, recompute and hilight it
			if (pcbyte == m_byteaddress[effrow])
				attrib = DCA_CURRENT;

			// if we're on a line with a breakpoint, tag it changed
			else
			{
				for (device_debug::breakpoint *bp = source.m_device.debug()->breakpoint_first(); bp != NULL; bp = bp->next())
					if (m_byteaddress[effrow] == (memory_address_to_byte(source.m_space, bp->address()) & source.m_space->logbytemask))
						attrib = DCA_CHANGED;
			}

			// if we're on the active column and everything is couth, highlight it
			if (m_cursor_visible && effrow == m_cursor.y)
				attrib |= DCA_SELECTED;

			// get the effective string
			const char *data = &m_dasm[effrow * m_allocated.x];
			UINT32 len = (UINT32)strlen(data);

			// copy data
			UINT32 effcol = m_topleft.x;
			while (col < m_visible.x && effcol < len)
			{
				dest->byte = data[effcol++];
				dest->attrib = (effcol <= m_divider1 || effcol >= m_divider2) ? (attrib | DCA_ANCILLARY) : attrib;

				// comments are just green for now - maybe they shouldn't even be this?
				if (effcol >= m_divider2 && m_right_column == DASM_RIGHTCOL_COMMENTS)
					attrib |= DCA_COMMENT;

				dest++;
				col++;
			}
		}

		// fill the rest with blanks
		while (col < m_visible.x)
		{
			dest->byte = ' ';
			dest->attrib = (effrow < m_total.y) ? (attrib | DCA_ANCILLARY) : attrib;
			dest++;
			col++;
		}
	}
}


//-------------------------------------------------
//  selected_address - return the PC of the
//  currently selected address in the view
//-------------------------------------------------

offs_t debug_view_disasm::selected_address()
{
	flush_updates();
	return memory_byte_to_address(downcast<const debug_view_disasm_source &>(*m_source).m_space, m_byteaddress[m_cursor.y]);
}


//-------------------------------------------------
//  set_sexpression - set the expression string
//  describing the home address
//-------------------------------------------------

void debug_view_disasm::set_expression(const char *expression)
{
	begin_update();
	m_expression.set_string(expression);
	m_recompute = m_update_pending = true;
	end_update();
}


//-------------------------------------------------
//  set_right_column - set the contents of the
//  right column
//-------------------------------------------------

void debug_view_disasm::set_right_column(disasm_right_column contents)
{
	begin_update();
	m_right_column = contents;
	m_recompute = m_update_pending = true;
	end_update();
}


//-------------------------------------------------
//  set_backward_steps - set the number of
//  instructions displayed before the home address
//-------------------------------------------------

void debug_view_disasm::set_backward_steps(UINT32 steps)
{
	begin_update();
	m_backwards_steps = steps;
	m_recompute = m_update_pending = true;
	end_update();
}


//-------------------------------------------------
//  set_disasm_width - set the width in characters
//  of the main disassembly section
//-------------------------------------------------

void debug_view_disasm::set_disasm_width(UINT32 width)
{
	begin_update();
	m_dasm_width = width;
	m_recompute = m_update_pending = true;
	end_update();
}


//-------------------------------------------------
//  set_selected_address - set the PC of the
//  currently selected address in the view
//-------------------------------------------------

void debug_view_disasm::set_selected_address(offs_t address)
{
	const debug_view_disasm_source &source = downcast<const debug_view_disasm_source &>(*m_source);
	offs_t byteaddress = memory_address_to_byte(source.m_space, address) & source.m_space->logbytemask;
	for (int line = 0; line < m_total.y; line++)
		if (m_byteaddress[line] == byteaddress)
		{
			m_cursor.y = line;
			set_cursor_position(m_cursor);
			break;
		}
}
