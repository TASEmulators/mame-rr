/***************************************************************************

    devcpu.h

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

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __DEVCPU_H__
#define __DEVCPU_H__


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// alternate address space names for common use
enum
{
	ADDRESS_SPACE_PROGRAM = ADDRESS_SPACE_0,	// program address space
	ADDRESS_SPACE_DATA = ADDRESS_SPACE_1,		// data address space
	ADDRESS_SPACE_IO = ADDRESS_SPACE_2			// I/O address space
};

// CPU information constants
const int MAX_REGS = 256;
enum
{
	// --- the following bits of info are returned as 64-bit signed integers ---
	CPUINFO_INT_FIRST = DEVINFO_INT_FIRST,

		// CPU-specific additionsg
		CPUINFO_INT_CONTEXT_SIZE = DEVINFO_INT_CLASS_SPECIFIC,	// R/O: size of CPU context in bytes
		CPUINFO_INT_INPUT_LINES,							// R/O: number of input lines
		CPUINFO_INT_DEFAULT_IRQ_VECTOR,						// R/O: default IRQ vector
		CPUINFO_INT_CLOCK_MULTIPLIER,						// R/O: internal clock multiplier
		CPUINFO_INT_CLOCK_DIVIDER,							// R/O: internal clock divider
		CPUINFO_INT_MIN_INSTRUCTION_BYTES,					// R/O: minimum bytes per instruction
		CPUINFO_INT_MAX_INSTRUCTION_BYTES,					// R/O: maximum bytes per instruction
		CPUINFO_INT_MIN_CYCLES,								// R/O: minimum cycles for a single instruction
		CPUINFO_INT_MAX_CYCLES,								// R/O: maximum cycles for a single instruction

		CPUINFO_INT_LOGADDR_WIDTH,							// R/O: address bus size for logical accesses in each space (0=same as physical)
		CPUINFO_INT_LOGADDR_WIDTH_PROGRAM = CPUINFO_INT_LOGADDR_WIDTH + ADDRESS_SPACE_PROGRAM,
		CPUINFO_INT_LOGADDR_WIDTH_DATA = CPUINFO_INT_LOGADDR_WIDTH + ADDRESS_SPACE_DATA,
		CPUINFO_INT_LOGADDR_WIDTH_IO = CPUINFO_INT_LOGADDR_WIDTH + ADDRESS_SPACE_IO,
		CPUINFO_INT_LOGADDR_WIDTH_LAST = CPUINFO_INT_LOGADDR_WIDTH + ADDRESS_SPACES - 1,
		CPUINFO_INT_PAGE_SHIFT,								// R/O: size of a page log 2 (i.e., 12=4096), or 0 if paging not supported
		CPUINFO_INT_PAGE_SHIFT_PROGRAM = CPUINFO_INT_PAGE_SHIFT + ADDRESS_SPACE_PROGRAM,
		CPUINFO_INT_PAGE_SHIFT_DATA = CPUINFO_INT_PAGE_SHIFT + ADDRESS_SPACE_DATA,
		CPUINFO_INT_PAGE_SHIFT_IO = CPUINFO_INT_PAGE_SHIFT + ADDRESS_SPACE_IO,
		CPUINFO_INT_PAGE_SHIFT_LAST = CPUINFO_INT_PAGE_SHIFT + ADDRESS_SPACES - 1,

		CPUINFO_INT_INPUT_STATE,							// R/W: states for each input line
		CPUINFO_INT_INPUT_STATE_LAST = CPUINFO_INT_INPUT_STATE + MAX_INPUT_LINES - 1,
		CPUINFO_INT_REGISTER = CPUINFO_INT_INPUT_STATE_LAST + 10,								// R/W: values of up to MAX_REGs registers
		CPUINFO_INT_SP = CPUINFO_INT_REGISTER + STATE_GENSP,		// R/W: the current stack pointer value
		CPUINFO_INT_PC = CPUINFO_INT_REGISTER + STATE_GENPC,		// R/W: the current PC value
		CPUINFO_INT_PREVIOUSPC = CPUINFO_INT_REGISTER + STATE_GENPCBASE,	// R/W: the previous PC value
		CPUINFO_INT_REGISTER_LAST = CPUINFO_INT_REGISTER + MAX_REGS - 1,

	CPUINFO_INT_CPU_SPECIFIC = 0x08000,						// R/W: CPU-specific values start here

	// --- the following bits of info are returned as pointers to data or functions ---
	CPUINFO_PTR_FIRST = DEVINFO_PTR_FIRST,

		// CPU-specific additions
		CPUINFO_PTR_INSTRUCTION_COUNTER = DEVINFO_PTR_CLASS_SPECIFIC,
															// R/O: int *icount

	CPUINFO_PTR_CPU_SPECIFIC = DEVINFO_PTR_DEVICE_SPECIFIC,	// R/W: CPU-specific values start here

	// --- the following bits of info are returned as pointers to functions ---
	CPUINFO_FCT_FIRST = DEVINFO_FCT_FIRST,

		// CPU-specific additions
		CPUINFO_FCT_SET_INFO = DEVINFO_FCT_CLASS_SPECIFIC,	// R/O: void (*set_info)(legacy_cpu_device *device, UINT32 state, INT64 data, void *ptr)
		CPUINFO_FCT_INIT,									// R/O: void (*init)(legacy_cpu_device *device, int index, int clock, int (*irqcallback)(legacy_cpu_device *device, int))
		CPUINFO_FCT_RESET,									// R/O: void (*reset)(legacy_cpu_device *device)
		CPUINFO_FCT_EXIT,									// R/O: void (*exit)(legacy_cpu_device *device)
		CPUINFO_FCT_EXECUTE,								// R/O: int (*execute)(legacy_cpu_device *device, int cycles)
		CPUINFO_FCT_BURN,									// R/O: void (*burn)(legacy_cpu_device *device, int cycles)
		CPUINFO_FCT_DISASSEMBLE,							// R/O: offs_t (*disassemble)(legacy_cpu_device *device, char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, int options)
		CPUINFO_FCT_TRANSLATE,								// R/O: int (*translate)(legacy_cpu_device *device, int space, int intention, offs_t *address)
		CPUINFO_FCT_READ,									// R/O: int (*read)(legacy_cpu_device *device, int space, UINT32 offset, int size, UINT64 *value)
		CPUINFO_FCT_WRITE,									// R/O: int (*write)(legacy_cpu_device *device, int space, UINT32 offset, int size, UINT64 value)
		CPUINFO_FCT_READOP,									// R/O: int (*readop)(legacy_cpu_device *device, UINT32 offset, int size, UINT64 *value)
		CPUINFO_FCT_DEBUG_INIT,								// R/O: void (*debug_init)(legacy_cpu_device *device)
		CPUINFO_FCT_IMPORT_STATE,							// R/O: void (*import_state)(legacy_cpu_device *device, const device_state_entry &entry)
		CPUINFO_FCT_EXPORT_STATE,							// R/O: void (*export_state)(legacy_cpu_device *device, const device_state_entry &entry)
		CPUINFO_FCT_IMPORT_STRING,							// R/O: void (*import_string)(legacy_cpu_device *device, const device_state_entry &entry, astring &string)
		CPUINFO_FCT_EXPORT_STRING,							// R/O: void (*export_string)(legacy_cpu_device *device, const device_state_entry &entry, astring &string)

	CPUINFO_FCT_CPU_SPECIFIC = DEVINFO_FCT_DEVICE_SPECIFIC,	// R/W: CPU-specific values start here

	// --- the following bits of info are returned as NULL-terminated strings ---
	CPUINFO_STR_FIRST = DEVINFO_STR_FIRST,

		// CPU-specific additions
		CPUINFO_STR_REGISTER = DEVINFO_STR_CLASS_SPECIFIC + 10,			// R/O: string representation of up to MAX_REGs registers
		CPUINFO_STR_FLAGS = CPUINFO_STR_REGISTER + STATE_GENFLAGS,		// R/O: string representation of the main flags value
		CPUINFO_STR_REGISTER_LAST = CPUINFO_STR_REGISTER + MAX_REGS - 1,

	CPUINFO_STR_CPU_SPECIFIC = DEVINFO_STR_DEVICE_SPECIFIC	// R/W: CPU-specific values start here
};



//**************************************************************************
//  CPU DEVICE CONFIGURATION MACROS
//**************************************************************************

#define MDRV_CPU_ADD MDRV_DEVICE_ADD
#define MDRV_CPU_MODIFY MDRV_DEVICE_MODIFY
#define MDRV_CPU_REPLACE MDRV_DEVICE_REPLACE

#define MDRV_CPU_CLOCK MDRV_DEVICE_CLOCK
#define MDRV_CPU_CONFIG MDRV_DEVICE_CONFIG

#define MDRV_CPU_PROGRAM_MAP MDRV_DEVICE_PROGRAM_MAP
#define MDRV_CPU_DATA_MAP MDRV_DEVICE_DATA_MAP
#define MDRV_CPU_IO_MAP MDRV_DEVICE_IO_MAP

#define MDRV_CPU_VBLANK_INT MDRV_DEVICE_VBLANK_INT
#define MDRV_CPU_PERIODIC_INT MDRV_DEVICE_PERIODIC_INT



//**************************************************************************
//  MACROS
//**************************************************************************

// macro for declaring the configuration and device classes of a legacy CPU device
#define DECLARE_LEGACY_CPU_DEVICE(name, basename)											\
																							\
CPU_GET_INFO( basename );																	\
																							\
class basename##_device_config;																\
																							\
class basename##_device : public legacy_cpu_device											\
{																							\
	friend class basename##_device_config;													\
	basename##_device(running_machine &_machine, const basename##_device_config &config);	\
};																							\
																							\
class basename##_device_config : public legacy_cpu_device_config							\
{																							\
	basename##_device_config(const machine_config &mconfig, device_type type, const char *tag, const device_config *owner, UINT32 clock); \
																							\
public:																						\
	static device_config *static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock); \
	virtual device_t *alloc_device(running_machine &machine) const; 						\
};																							\
																							\
extern const device_type name

// macro for defining the implementation needed for configuration and device classes
#define DEFINE_LEGACY_CPU_DEVICE(name, basename)											\
																							\
basename##_device::basename##_device(running_machine &_machine, const basename##_device_config &config)	\
	: legacy_cpu_device(_machine, config)													\
{																							\
}																							\
																							\
basename##_device_config::basename##_device_config(const machine_config &mconfig, device_type type, const char *tag, const device_config *owner, UINT32 clock) \
	: legacy_cpu_device_config(mconfig, type, tag, owner, clock, CPU_GET_INFO_NAME(basename)) \
{																							\
}																							\
																							\
device_config *basename##_device_config::static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock) \
{																							\
	return global_alloc(basename##_device_config(mconfig, static_alloc_device_config, tag, owner, clock)); \
}																							\
																							\
device_t *basename##_device_config::alloc_device(running_machine &machine) const			\
{																							\
	return pool_alloc(machine_get_pool(machine), basename##_device(machine, *this));		\
}																							\
const device_type name = basename##_device_config::static_alloc_device_config


// CPU interface functions
#define CPU_GET_INFO_NAME(name)			cpu_get_info_##name
#define CPU_GET_INFO(name)				void CPU_GET_INFO_NAME(name)(const device_config *devconfig, legacy_cpu_device *device, UINT32 state, cpuinfo *info)
#define CPU_GET_INFO_CALL(name)			CPU_GET_INFO_NAME(name)(devconfig, device, state, info)

#define CPU_SET_INFO_NAME(name)			cpu_set_info_##name
#define CPU_SET_INFO(name)				void CPU_SET_INFO_NAME(name)(legacy_cpu_device *device, UINT32 state, cpuinfo *info)
#define CPU_SET_INFO_CALL(name)			CPU_SET_INFO_NAME(name)(device, state, info)

#define CPU_INIT_NAME(name)				cpu_init_##name
#define CPU_INIT(name)					void CPU_INIT_NAME(name)(legacy_cpu_device *device, device_irq_callback irqcallback)
#define CPU_INIT_CALL(name)				CPU_INIT_NAME(name)(device, irqcallback)

#define CPU_RESET_NAME(name)			cpu_reset_##name
#define CPU_RESET(name)					void CPU_RESET_NAME(name)(legacy_cpu_device *device)
#define CPU_RESET_CALL(name)			CPU_RESET_NAME(name)(device)

#define CPU_EXIT_NAME(name)				cpu_exit_##name
#define CPU_EXIT(name)					void CPU_EXIT_NAME(name)(legacy_cpu_device *device)
#define CPU_EXIT_CALL(name)				CPU_EXIT_NAME(name)(device)

#define CPU_EXECUTE_NAME(name)			cpu_execute_##name
#define CPU_EXECUTE(name)				void CPU_EXECUTE_NAME(name)(legacy_cpu_device *device)
#define CPU_EXECUTE_CALL(name)			CPU_EXECUTE_NAME(name)(device, cycles)

#define CPU_BURN_NAME(name)				cpu_burn_##name
#define CPU_BURN(name)					void CPU_BURN_NAME(name)(legacy_cpu_device *device, int cycles)
#define CPU_BURN_CALL(name)				CPU_BURN_NAME(name)(device, cycles)

#define CPU_TRANSLATE_NAME(name)		cpu_translate_##name
#define CPU_TRANSLATE(name)				int CPU_TRANSLATE_NAME(name)(legacy_cpu_device *device, int space, int intention, offs_t *address)
#define CPU_TRANSLATE_CALL(name)		CPU_TRANSLATE_NAME(name)(device, space, intention, address)

#define CPU_READ_NAME(name)				cpu_read_##name
#define CPU_READ(name)					int CPU_READ_NAME(name)(legacy_cpu_device *device, int space, UINT32 offset, int size, UINT64 *value)
#define CPU_READ_CALL(name)				CPU_READ_NAME(name)(device, space, offset, size, value)

#define CPU_WRITE_NAME(name)			cpu_write_##name
#define CPU_WRITE(name)					int CPU_WRITE_NAME(name)(legacy_cpu_device *device, int space, UINT32 offset, int size, UINT64 value)
#define CPU_WRITE_CALL(name)			CPU_WRITE_NAME(name)(device, space, offset, size, value)

#define CPU_READOP_NAME(name)			cpu_readop_##name
#define CPU_READOP(name)				int CPU_READOP_NAME(name)(legacy_cpu_device *device, UINT32 offset, int size, UINT64 *value)
#define CPU_READOP_CALL(name)			CPU_READOP_NAME(name)(device, offset, size, value)

#define CPU_DEBUG_INIT_NAME(name)		cpu_debug_init_##name
#define CPU_DEBUG_INIT(name)			void CPU_DEBUG_INIT_NAME(name)(legacy_cpu_device *device)
#define CPU_DEBUG_INIT_CALL(name)		CPU_DEBUG_INIT_NAME(name)(device)

#define CPU_DISASSEMBLE_NAME(name)		cpu_disassemble_##name
#define CPU_DISASSEMBLE(name)			offs_t CPU_DISASSEMBLE_NAME(name)(legacy_cpu_device *device, char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, int options)
#define CPU_DISASSEMBLE_CALL(name)		CPU_DISASSEMBLE_NAME(name)(device, buffer, pc, oprom, opram, options)

#define CPU_IMPORT_STATE_NAME(name)		cpu_state_import_##name
#define CPU_IMPORT_STATE(name)			void CPU_IMPORT_STATE_NAME(name)(legacy_cpu_device *device, const device_state_entry &entry)
#define CPU_IMPORT_STATE_CALL(name)		CPU_IMPORT_STATE_NAME(name)(device, entry)

#define CPU_EXPORT_STATE_NAME(name)		cpu_state_export_##name
#define CPU_EXPORT_STATE(name)			void CPU_EXPORT_STATE_NAME(name)(legacy_cpu_device *device, const device_state_entry &entry)
#define CPU_EXPORT_STATE_CALL(name)		CPU_EXPORT_STATE_NAME(name)(device, entry)

#define CPU_EXPORT_STRING_NAME(name)	cpu_string_export_##name
#define CPU_EXPORT_STRING(name)			void CPU_EXPORT_STRING_NAME(name)(legacy_cpu_device *device, const device_state_entry &entry, astring &string)
#define CPU_EXPORT_STRING_CALL(name)	CPU_EXPORT_STRING_NAME(name)(device, entry, string)


// helpers for accessing common CPU state

// CPU scheduling
#define cpu_suspend							device_suspend
#define cpu_resume							device_resume

// synchronization helpers
#define cpu_yield							device_yield
#define cpu_spin							device_spin
#define cpu_spinuntil_trigger				device_spin_until_trigger
#define cpu_spinuntil_time					device_spin_until_time
#define cpu_spinuntil_int					device_spin_until_interrupt

// CPU timing
#define cpu_eat_cycles						device_eat_cycles
#define cpu_adjust_icount					device_adjust_icount

#define cpu_triggerint						device_triggerint
#define cpu_set_input_line					device_set_input_line
#define cpu_set_input_line_vector			device_set_input_line_vector
#define cpu_set_input_line_and_vector		device_set_input_line_and_vector
#define cpu_set_irq_callback				device_set_irq_callback
#define cpu_spin_until_int					device_spin_until_int

#define cpu_get_address_space				device_get_space

#define cpu_get_reg(cpu, _reg)				device_state(cpu)->state(_reg)
#define	cpu_get_previouspc(cpu)				((offs_t)device_state(cpu)->state(STATE_GENPCBASE))
#define	cpu_get_pc(cpu)						((offs_t)device_state(cpu)->state(STATE_GENPC))

#define cpu_set_reg(cpu, _reg, val)			device_state(cpu)->set_state(_reg, val)

#define INTERRUPT_GEN(func)		void func(device_t *device)

// helpers for using machine/cputag instead of cpu objects
#define cputag_get_address_space(mach, tag, spc)						downcast<cpu_device *>((mach)->device(tag))->space(spc)
#define cputag_get_clock(mach, tag)										(mach)->device(tag)->unscaled_clock()
#define cputag_set_clock(mach, tag, clock)								(mach)->device(tag)->set_unscaled_clock(clock)
#define cputag_clocks_to_attotime(mach, tag, clocks)					(mach)->device(tag)->clocks_to_attotime(clocks)
#define cputag_attotime_to_clocks(mach, tag, duration)					(mach)->device(tag)->attotime_to_clocks(duration)

#define cputag_suspend(mach, tag, reason, eat)							device_suspend((mach)->device(tag), reason, eat)
#define cputag_resume(mach, tag, reason)								device_resume((mach)->device(tag), reason)

#define cputag_set_input_line(mach, tag, line, state)					downcast<cpu_device *>((mach)->device(tag))->set_input_line(line, state)
#define cputag_set_input_line_and_vector(mach, tag, line, state, vec)	downcast<cpu_device *>((mach)->device(tag))->set_input_line_and_vector(line, state, vec)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward declaration of types
union cpuinfo;
class cpu_device;
class legacy_cpu_device;


// CPU interface functions
typedef void (*cpu_get_info_func)(const device_config *devconfig, legacy_cpu_device *device, UINT32 state, cpuinfo *info);
typedef void (*cpu_set_info_func)(legacy_cpu_device *device, UINT32 state, cpuinfo *info);
typedef void (*cpu_init_func)(legacy_cpu_device *device, device_irq_callback irqcallback);
typedef void (*cpu_reset_func)(legacy_cpu_device *device);
typedef void (*cpu_exit_func)(legacy_cpu_device *device);
typedef void (*cpu_execute_func)(legacy_cpu_device *device);
typedef void (*cpu_burn_func)(legacy_cpu_device *device, int cycles);
typedef int	(*cpu_translate_func)(legacy_cpu_device *device, int space, int intention, offs_t *address);
typedef int	(*cpu_read_func)(legacy_cpu_device *device, int space, UINT32 offset, int size, UINT64 *value);
typedef int	(*cpu_write_func)(legacy_cpu_device *device, int space, UINT32 offset, int size, UINT64 value);
typedef int	(*cpu_readop_func)(legacy_cpu_device *device, UINT32 offset, int size, UINT64 *value);
typedef void (*cpu_debug_init_func)(legacy_cpu_device *device);
typedef offs_t (*cpu_disassemble_func)(legacy_cpu_device *device, char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, int options);
typedef void (*cpu_state_io_func)(legacy_cpu_device *device, const device_state_entry &entry);
typedef void (*cpu_string_io_func)(legacy_cpu_device *device, const device_state_entry &entry, astring &string);


// cpuinfo union used to pass data to/from the get_info/set_info functions
union cpuinfo
{
	INT64					i;							// generic integers
	void *					p;							// generic pointers
	genf *  				f;							// generic function pointers
	char *					s;							// generic strings

	cpu_set_info_func		setinfo;					// CPUINFO_FCT_SET_INFO
	cpu_init_func			init;						// CPUINFO_FCT_INIT
	cpu_reset_func			reset;						// CPUINFO_FCT_RESET
	cpu_exit_func			exit;						// CPUINFO_FCT_EXIT
	cpu_execute_func		execute;					// CPUINFO_FCT_EXECUTE
	cpu_burn_func			burn;						// CPUINFO_FCT_BURN
	cpu_translate_func		translate;					// CPUINFO_FCT_TRANSLATE
	cpu_read_func			read;						// CPUINFO_FCT_READ
	cpu_write_func			write;						// CPUINFO_FCT_WRITE
	cpu_readop_func			readop;						// CPUINFO_FCT_READOP
	cpu_debug_init_func		debug_init;					// CPUINFO_FCT_DEBUG_INIT
	cpu_disassemble_func	disassemble;				// CPUINFO_FCT_DISASSEMBLE
	cpu_state_io_func		import_state;				// CPUINFO_FCT_IMPORT_STATE
	cpu_state_io_func		export_state;				// CPUINFO_FCT_EXPORT_STATE
	cpu_string_io_func		import_string;				// CPUINFO_FCT_IMPORT_STRING
	cpu_string_io_func		export_string;				// CPUINFO_FCT_EXPORT_STRING
	int *					icount;						// CPUINFO_PTR_INSTRUCTION_COUNTER
	const addrmap8_token *	internal_map8;				// DEVINFO_PTR_INTERNAL_MEMORY_MAP
	const addrmap16_token *	internal_map16;				// DEVINFO_PTR_INTERNAL_MEMORY_MAP
	const addrmap32_token *	internal_map32;				// DEVINFO_PTR_INTERNAL_MEMORY_MAP
	const addrmap64_token *	internal_map64;				// DEVINFO_PTR_INTERNAL_MEMORY_MAP
	const addrmap8_token *	default_map8;				// DEVINFO_PTR_DEFAULT_MEMORY_MAP
	const addrmap16_token *	default_map16;				// DEVINFO_PTR_DEFAULT_MEMORY_MAP
	const addrmap32_token *	default_map32;				// DEVINFO_PTR_DEFAULT_MEMORY_MAP
	const addrmap64_token *	default_map64;				// DEVINFO_PTR_DEFAULT_MEMORY_MAP
};



// ======================> cpu_device_config

class cpu_device_config :	public device_config,
							public device_config_execute_interface,
							public device_config_memory_interface,
							public device_config_state_interface,
							public device_config_disasm_interface
{
	friend class cpu_device;

protected:
	// construction/destruction
	cpu_device_config(const machine_config &mconfig, device_type type, const char *name, const char *tag, const device_config *owner, UINT32 clock);
};



// ======================> legacy_cpu_device_config

class legacy_cpu_device_config : public cpu_device_config
{
	friend class legacy_cpu_device;

protected:
	// construction/destruction
	legacy_cpu_device_config(const machine_config &mconfig, device_type _type, const char *_tag, const device_config *_owner, UINT32 _clock, cpu_get_info_func get_info);

public:
	// basic information getters
	virtual const rom_entry *rom_region() const { return reinterpret_cast<const rom_entry *>(get_legacy_config_ptr(DEVINFO_PTR_ROM_REGION)); }
	virtual const machine_config_token *machine_config_tokens() const { return reinterpret_cast<const machine_config_token *>(get_legacy_config_ptr(DEVINFO_PTR_MACHINE_CONFIG)); }

protected:
	// device_config_execute_interface overrides
	virtual UINT64 execute_clocks_to_cycles(UINT64 clocks) const;
	virtual UINT64 execute_cycles_to_clocks(UINT64 cycles) const;
	virtual UINT32 execute_min_cycles() const { return get_legacy_config_int(CPUINFO_INT_MIN_CYCLES); }
	virtual UINT32 execute_max_cycles() const { return get_legacy_config_int(CPUINFO_INT_MAX_CYCLES); }
	virtual UINT32 execute_input_lines() const { return get_legacy_config_int(CPUINFO_INT_INPUT_LINES); }
	virtual UINT32 execute_default_irq_vector() const { return get_legacy_config_int(CPUINFO_INT_DEFAULT_IRQ_VECTOR); }

	// device_config_memory_interface overrides
	virtual const address_space_config *memory_space_config(int spacenum = 0) const { return (spacenum < ARRAY_LENGTH(m_space_config) && m_space_config[spacenum].m_addrbus_width != 0) ? &m_space_config[spacenum] : NULL; }

	// device_config_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const { return get_legacy_config_int(CPUINFO_INT_MIN_INSTRUCTION_BYTES); }
	virtual UINT32 disasm_max_opcode_bytes() const { return get_legacy_config_int(CPUINFO_INT_MAX_INSTRUCTION_BYTES); }

	// legacy interface helpers
	INT64 get_legacy_config_int(UINT32 state) const;
	void *get_legacy_config_ptr(UINT32 state) const;
	genf *get_legacy_config_fct(UINT32 state) const;
	const char *get_legacy_config_string(UINT32 state) const;

	// internal state
	cpu_get_info_func	m_get_info;
	address_space_config m_space_config[3];				// array of address space configs
};



// ======================> cpu_device

class cpu_device :	public device_t,
					public device_execute_interface,
					public device_memory_interface,
					public device_state_interface,
					public device_disasm_interface
{
	friend class cpu_device_config;
	friend resource_pool_object<cpu_device>::~resource_pool_object();

protected:
	// construction/destruction
	cpu_device(running_machine &machine, const cpu_device_config &config);
	virtual ~cpu_device();
};



// ======================> legacy_cpu_device

class legacy_cpu_device : public cpu_device
{
	friend class legacy_cpu_device_config;
	friend resource_pool_object<legacy_cpu_device>::~resource_pool_object();

protected:
	// construction/destruction
	legacy_cpu_device(running_machine &machine, const legacy_cpu_device_config &config);
	virtual ~legacy_cpu_device();

public:
	void *token() const { return m_token; }

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_debug_setup();

	// device_execute_interface overrides
	virtual void execute_run();
	virtual void execute_burn(INT32 cycles);
	virtual void execute_set_input(int inputnum, int state) { set_legacy_runtime_int(CPUINFO_INT_INPUT_STATE + inputnum, state); }

	// device_memory_interface overrides
	virtual bool memory_translate(int spacenum, int intention, offs_t &address);
	virtual bool memory_read(int spacenum, offs_t offset, int size, UINT64 &value);
	virtual bool memory_write(int spacenum, offs_t offset, int size, UINT64 value);
	virtual bool memory_readop(offs_t offset, int size, UINT64 &value);

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry);
	virtual void state_export(const device_state_entry &entry);
	virtual void state_string_export(const device_state_entry &entry, astring &string);

	// device_disasm_interface overrides
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);

	// helpers to access data via the legacy get_info functions
	INT64 get_legacy_runtime_int(UINT32 state);
	void *get_legacy_runtime_ptr(UINT32 state);
	const char *get_legacy_runtime_string(UINT32 state);
	void set_legacy_runtime_int(UINT32 state, INT64 value);

protected:
	// internal state
	const legacy_cpu_device_config &m_cpu_config;		// reference to the config
	void *					m_token;					// pointer to our state

	cpu_set_info_func		m_set_info;					// extracted legacy function pointers
	cpu_execute_func		m_execute;					//
	cpu_burn_func			m_burn;						//
	cpu_translate_func		m_translate;				//
	cpu_read_func			m_read;						//
	cpu_write_func			m_write;					//
	cpu_readop_func			m_readop;					//
	cpu_disassemble_func	m_disassemble;				//
	cpu_state_io_func		m_state_import;				//
	cpu_state_io_func		m_state_export;				//
	cpu_string_io_func		m_string_export;			//
	cpu_exit_func			m_exit;						//

	UINT64					m_state_io;					// temporary buffer for state I/O
	bool					m_using_legacy_state;		// true if we are using the old-style state access
};


#endif	/* __CPUINTRF_H__ */
