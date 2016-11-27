/***************************************************************************

    devcpu.c

    CPU device definitions.

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
#include "debugger.h"
#include <ctype.h>


//**************************************************************************
//  CPU DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  cpu_device_config - constructor
//-------------------------------------------------

cpu_device_config::cpu_device_config(const machine_config &mconfig, device_type type, const char *name, const char *tag, const device_config *owner, UINT32 clock)
	: device_config(mconfig, type, name, tag, owner, clock),
	  device_config_execute_interface(mconfig, *this),
	  device_config_memory_interface(mconfig, *this),
	  device_config_state_interface(mconfig, *this),
	  device_config_disasm_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  legacy_cpu_device_config - constructor
//-------------------------------------------------

legacy_cpu_device_config::legacy_cpu_device_config(const machine_config &mconfig, device_type type, const char *tag, const device_config *owner, UINT32 clock, cpu_get_info_func get_info)
	: cpu_device_config(mconfig, type, "CPU", tag, owner, clock),
	  m_get_info(get_info)
{
	// build up our address spaces; legacy devices don't have logical spaces
	memset(m_space_config, 0, sizeof(m_space_config));
	for (int spacenum = 0; spacenum < ARRAY_LENGTH(m_space_config); spacenum++)
	{
		m_space_config[spacenum].m_name = (spacenum == 1) ? "data" : (spacenum == 2) ? "i/o" : "program";
		m_space_config[spacenum].m_endianness = static_cast<endianness_t>(get_legacy_config_int(DEVINFO_INT_ENDIANNESS));
		m_space_config[spacenum].m_databus_width = get_legacy_config_int(DEVINFO_INT_DATABUS_WIDTH + spacenum);
		m_space_config[spacenum].m_addrbus_width = get_legacy_config_int(DEVINFO_INT_ADDRBUS_WIDTH + spacenum);
		m_space_config[spacenum].m_addrbus_shift = get_legacy_config_int(DEVINFO_INT_ADDRBUS_SHIFT + spacenum);
		m_space_config[spacenum].m_logaddr_width = get_legacy_config_int(CPUINFO_INT_LOGADDR_WIDTH + spacenum);
		m_space_config[spacenum].m_page_shift = get_legacy_config_int(CPUINFO_INT_PAGE_SHIFT + spacenum);
		m_space_config[spacenum].m_internal_map = reinterpret_cast<const addrmap_token *>(get_legacy_config_ptr(DEVINFO_PTR_INTERNAL_MEMORY_MAP + spacenum));
		m_space_config[spacenum].m_default_map = reinterpret_cast<const addrmap_token *>(get_legacy_config_ptr(DEVINFO_PTR_DEFAULT_MEMORY_MAP + spacenum));
	}

	// set the real name
	m_name = get_legacy_config_string(DEVINFO_STR_NAME);
}


//-------------------------------------------------
//  execute_clocks_to_cycles - convert the raw
//  clock into cycles per second
//-------------------------------------------------

UINT64 legacy_cpu_device_config::execute_clocks_to_cycles(UINT64 clocks) const
{
	UINT32 multiplier = get_legacy_config_int(CPUINFO_INT_CLOCK_MULTIPLIER);
	UINT32 divider = get_legacy_config_int(CPUINFO_INT_CLOCK_DIVIDER);

	if (multiplier == 0) multiplier = 1;
	if (divider == 0) divider = 1;

	return (clocks * multiplier + divider - 1) / divider;
}


//-------------------------------------------------
//  execute_cycles_to_clocks - convert a cycle
//  count back to raw clocks
//-------------------------------------------------

UINT64 legacy_cpu_device_config::execute_cycles_to_clocks(UINT64 cycles) const
{
	UINT32 multiplier = get_legacy_config_int(CPUINFO_INT_CLOCK_MULTIPLIER);
	UINT32 divider = get_legacy_config_int(CPUINFO_INT_CLOCK_DIVIDER);

	if (multiplier == 0) multiplier = 1;
	if (divider == 0) divider = 1;

	return (cycles * divider + multiplier - 1) / multiplier;
}


//-------------------------------------------------
//  get_legacy_config_int - return a legacy
//  integer value
//-------------------------------------------------

INT64 legacy_cpu_device_config::get_legacy_config_int(UINT32 state) const
{
	cpuinfo info = { 0 };
	(*m_get_info)(this, NULL, state, &info);
	return info.i;
}


//-------------------------------------------------
//  get_legacy_config_ptr - return a legacy
//  pointer value
//-------------------------------------------------

void *legacy_cpu_device_config::get_legacy_config_ptr(UINT32 state) const
{
	cpuinfo info = { 0 };
	(*m_get_info)(this, NULL, state, &info);
	return info.p;
}


//-------------------------------------------------
//  get_legacy_config_fct - return a legacy
//  function value
//-------------------------------------------------

genf *legacy_cpu_device_config::get_legacy_config_fct(UINT32 state) const
{
	cpuinfo info = { 0 };
	(*m_get_info)(this, NULL, state, &info);
	return info.f;
}


//-------------------------------------------------
//  get_legacy_config_string - return a legacy
//  string value
//-------------------------------------------------

const char *legacy_cpu_device_config::get_legacy_config_string(UINT32 state) const
{
	cpuinfo info;
	info.s = get_temp_string_buffer();
	(*m_get_info)(this, NULL, state, &info);
	return info.s;
}



//**************************************************************************
//  CPU RUNNING DEVICE
//**************************************************************************

//-------------------------------------------------
//  cpu_device - constructor
//-------------------------------------------------

cpu_device::cpu_device(running_machine &machine, const cpu_device_config &config)
	: device_t(machine, config),
	  device_execute_interface(machine, config, *this),
	  device_memory_interface(machine, config, *this),
	  device_state_interface(machine, config, *this),
	  device_disasm_interface(machine, config, *this)
{
}


//-------------------------------------------------
//  cpu_device - destructor
//-------------------------------------------------

cpu_device::~cpu_device()
{
}


//-------------------------------------------------
//  legacy_cpu_device - constructor
//-------------------------------------------------

legacy_cpu_device::legacy_cpu_device(running_machine &machine, const legacy_cpu_device_config &config)
	: cpu_device(machine, config),
	  m_cpu_config(config),
	  m_token(NULL),
	  m_set_info(reinterpret_cast<cpu_set_info_func>(m_cpu_config.get_legacy_config_fct(CPUINFO_FCT_SET_INFO))),
	  m_execute(reinterpret_cast<cpu_execute_func>(m_cpu_config.get_legacy_config_fct(CPUINFO_FCT_EXECUTE))),
	  m_burn(reinterpret_cast<cpu_burn_func>(m_cpu_config.get_legacy_config_fct(CPUINFO_FCT_BURN))),
	  m_translate(reinterpret_cast<cpu_translate_func>(m_cpu_config.get_legacy_config_fct(CPUINFO_FCT_TRANSLATE))),
	  m_read(reinterpret_cast<cpu_read_func>(m_cpu_config.get_legacy_config_fct(CPUINFO_FCT_READ))),
	  m_write(reinterpret_cast<cpu_write_func>(m_cpu_config.get_legacy_config_fct(CPUINFO_FCT_WRITE))),
	  m_readop(reinterpret_cast<cpu_readop_func>(m_cpu_config.get_legacy_config_fct(CPUINFO_FCT_READOP))),
	  m_disassemble(reinterpret_cast<cpu_disassemble_func>(m_cpu_config.get_legacy_config_fct(CPUINFO_FCT_DISASSEMBLE))),
	  m_state_import(reinterpret_cast<cpu_state_io_func>(m_cpu_config.get_legacy_config_fct(CPUINFO_FCT_IMPORT_STATE))),
	  m_state_export(reinterpret_cast<cpu_state_io_func>(m_cpu_config.get_legacy_config_fct(CPUINFO_FCT_EXPORT_STATE))),
	  m_string_export(reinterpret_cast<cpu_string_io_func>(m_cpu_config.get_legacy_config_fct(CPUINFO_FCT_EXPORT_STRING))),
	  m_exit(reinterpret_cast<cpu_exit_func>(m_cpu_config.get_legacy_config_fct(CPUINFO_FCT_EXIT))),
	  m_using_legacy_state(false)
{
	memset(&m_partial_frame_period, 0, sizeof(m_partial_frame_period));

	int tokenbytes = m_cpu_config.get_legacy_config_int(CPUINFO_INT_CONTEXT_SIZE);
	if (tokenbytes == 0)
		throw emu_fatalerror("Device %s specifies a 0 context size!\n", tag());

	// allocate memory for the token
	m_token = auto_alloc_array_clear(&machine, UINT8, tokenbytes);
}


//-------------------------------------------------
//  legacy_cpu_device - destructor
//-------------------------------------------------

legacy_cpu_device::~legacy_cpu_device()
{
	// call the CPU's exit function if present
	if (m_exit != NULL)
		(*m_exit)(this);
}


//-------------------------------------------------
//  device_start - start up the device
//-------------------------------------------------

void legacy_cpu_device::device_start()
{
	// standard init
	cpu_init_func init = reinterpret_cast<cpu_init_func>(m_cpu_config.get_legacy_config_fct(CPUINFO_FCT_INIT));
	(*init)(this, static_standard_irq_callback);

	// fetch information about the CPU states
	if (m_state_list == NULL)
	{
		m_using_legacy_state = true;
		for (int index = 0; index < MAX_REGS; index++)
		{
			const char *string = get_legacy_runtime_string(CPUINFO_STR_REGISTER + index);
			if (strchr(string, ':') != NULL)
			{
				astring tempstr(string);
				bool noshow = (tempstr.chr(0, '~') == 0);
				if (noshow)
					tempstr.substr(1, -1);

				int colon = tempstr.chr(0, ':');
				int length = tempstr.len() - colon - 1;

				tempstr.substr(0, colon).trimspace();

				astring formatstr;
				formatstr.printf("%%%ds", length);
				device_state_entry &entry = state_add(index, tempstr, m_state_io).callimport().callexport().formatstr(formatstr);
				if (noshow)
					entry.noshow();
			}
		}
		state_add(STATE_GENPC, "curpc", m_state_io).callimport().callexport().formatstr("%8s").noshow();
		state_add(STATE_GENPCBASE, "curpcbase", m_state_io).callimport().callexport().formatstr("%8s").noshow();

		const char *string = get_legacy_runtime_string(CPUINFO_STR_FLAGS);
		if (string != NULL && string[0] != 0)
		{
			astring flagstr;
			flagstr.printf("%%%ds", strlen(string));
			state_add(STATE_GENFLAGS, "GENFLAGS", m_state_io).callimport().callexport().formatstr(flagstr).noshow();
		}
	}

	// get our icount pointer
	m_icount = reinterpret_cast<int *>(get_legacy_runtime_ptr(CPUINFO_PTR_INSTRUCTION_COUNTER));
	assert(m_icount != 0);
	*m_icount = 0;
}


//-------------------------------------------------
//  device_reset - reset up the device
//-------------------------------------------------

void legacy_cpu_device::device_reset()
{
	cpu_reset_func reset = reinterpret_cast<cpu_reset_func>(m_cpu_config.get_legacy_config_fct(CPUINFO_FCT_RESET));
	if (reset != NULL)
		(*reset)(this);
}


//-------------------------------------------------
//  execute - execute for the provided number of
//  cycles
//-------------------------------------------------

void legacy_cpu_device::execute_run()
{
	(*m_execute)(this);
}


//-------------------------------------------------
//  execute_burn - burn the requested number of cycles
//-------------------------------------------------

void legacy_cpu_device::execute_burn(INT32 cycles)
{
	if (m_burn != NULL)
		(*m_burn)(this, cycles);
}


//-------------------------------------------------
//  memory_translate - perform address translation
//  on the provided address
//-------------------------------------------------

bool legacy_cpu_device::memory_translate(int spacenum, int intention, offs_t &address)
{
	if (m_translate != NULL)
		return (*m_translate)(this, spacenum, intention, &address) ? true : false;
	return true;
}


//-------------------------------------------------
//  memory_read - read device memory, allowing for
//  device specific overrides
//-------------------------------------------------

bool legacy_cpu_device::memory_read(int spacenum, offs_t offset, int size, UINT64 &value)
{
	if (m_read != NULL)
		return (*m_read)(this, spacenum, offset, size, &value) ? true : false;
	return false;
}


//-------------------------------------------------
//  memory_write - write device memory, allowing
//  for device specific overrides
//-------------------------------------------------

bool legacy_cpu_device::memory_write(int spacenum, offs_t offset, int size, UINT64 value)
{
	if (m_write != NULL)
		return (*m_write)(this, spacenum, offset, size, value) ? true : false;
	return false;
}


//-------------------------------------------------
//  memory_read - read device opcode memory,
//  allowing for device specific overrides
//-------------------------------------------------

bool legacy_cpu_device::memory_readop(offs_t offset, int size, UINT64 &value)
{
	if (m_readop != NULL)
		return (*m_readop)(this, offset, size, &value) ? true : false;
	return false;
}


//-------------------------------------------------
//  debug_setup - set up any device-specific
//  debugging commands or state
//-------------------------------------------------

void legacy_cpu_device::device_debug_setup()
{
	cpu_debug_init_func init = reinterpret_cast<cpu_debug_init_func>(m_cpu_config.get_legacy_config_fct(CPUINFO_FCT_DEBUG_INIT));
	if (init != NULL)
		(*init)(this);
}


//-------------------------------------------------
//  disassemble - disassemble the provided opcode
//  data to a buffer
//-------------------------------------------------

offs_t legacy_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	// if we have a callback, just use that
	if (m_disassemble != NULL)
		return (*m_disassemble)(this, buffer, pc, oprom, opram, options);

	// if not, just output vanilla bytes
	int width = min_opcode_bytes();
	switch (width)
	{
		case 1:
		default:
			sprintf(buffer, "$%02X", *(UINT8 *)oprom);
			break;

		case 2:
			sprintf(buffer, "$%04X", *(UINT16 *)oprom);
			break;

		case 4:
			sprintf(buffer, "$%08X", *(UINT32 *)oprom);
			break;

		case 8:
			sprintf(buffer, "$%08X%08X", (UINT32)(*(UINT64 *)oprom >> 32), (UINT32)(*(UINT64 *)oprom >> 0));
			break;
	}
	return width;
}


//-------------------------------------------------
//  get_legacy_runtime_int - call the get info
//  function to retrieve an integer value
//-------------------------------------------------

INT64 legacy_cpu_device::get_legacy_runtime_int(UINT32 state)
{
	cpuinfo info = { 0 };
	(*m_cpu_config.m_get_info)(&m_cpu_config, this, state, &info);
	return info.i;
}


//-------------------------------------------------
//  get_legacy_runtime_ptr - call the get info
//  function to retrieve a pointer value
//-------------------------------------------------

void *legacy_cpu_device::get_legacy_runtime_ptr(UINT32 state)
{
	cpuinfo info = { 0 };
	(*m_cpu_config.m_get_info)(&m_cpu_config, this, state, &info);
	return info.p;
}


//-------------------------------------------------
//  get_legacy_runtime_string - call the get info
//  function to retrieve a string value
//-------------------------------------------------

const char *legacy_cpu_device::get_legacy_runtime_string(UINT32 state)
{
	cpuinfo info;
	info.s = get_temp_string_buffer();
	(*m_cpu_config.m_get_info)(&m_cpu_config, this, state, &info);
	return info.s;
}


//-------------------------------------------------
//  set_legacy_runtime_int - call the get info
//  function to set an integer value
//-------------------------------------------------

void legacy_cpu_device::set_legacy_runtime_int(UINT32 state, INT64 value)
{
	cpuinfo info = { 0 };
	info.i = value;
	(*m_set_info)(this, state, &info);
}



void legacy_cpu_device::state_import(const device_state_entry &entry)
{
	if (m_using_legacy_state)
	{
		if (entry.index() == STATE_GENFLAGS)
			;	// do nothing
		else
			set_legacy_runtime_int(CPUINFO_INT_REGISTER + entry.index(), m_state_io);
	}
	else if (m_state_import != NULL)
		(*m_state_import)(this, entry);
}


void legacy_cpu_device::state_export(const device_state_entry &entry)
{
	if (m_using_legacy_state)
	{
		if (entry.index() == STATE_GENFLAGS)
		{
			const char *temp = get_legacy_runtime_string(CPUINFO_STR_FLAGS);
			m_state_io = 0;
			while (*temp != 0)
				m_state_io = ((m_state_io << 5) | (m_state_io >> (64-5))) ^ *temp++;
		}
		else
			m_state_io = get_legacy_runtime_int(CPUINFO_INT_REGISTER + entry.index());
	}
	else if (m_state_export != NULL)
		(*m_state_export)(this, entry);
}


void legacy_cpu_device::state_string_export(const device_state_entry &entry, astring &string)
{
	if (m_using_legacy_state)
	{
		if (entry.index() == STATE_GENFLAGS)
			string.cpy(get_legacy_runtime_string(CPUINFO_STR_FLAGS));
		else
			string.cpy(strchr(get_legacy_runtime_string(CPUINFO_STR_REGISTER + entry.index()), ':') + 1);
	}
	else if (m_string_export != NULL)
		(*m_string_export)(this, entry, string);
}
