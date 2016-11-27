/* ======================================================================== */
/* =============================== COPYRIGHT ============================== */
/* ======================================================================== */
/*

G65C816 CPU Emulator V1.00

Copyright Karl Stenerud
All rights reserved.

Permission is granted to use this source code for non-commercial purposes.
To use this code for commercial purposes, you must get permission from the
author (Karl Stenerud) at karl@higashiyama-unet.ocn.ne.jp.


*/
/* ======================================================================== */
/* ================================= NOTES ================================ */
/* ======================================================================== */
/*

Changes:
    1.01 (2010-04-04):
        Angelo Salese
        - Added boundary checks for MVP and MVN in M mode.

    1.00 (2008-11-27):
        R. Belmont
        - Reworked for modern MAME

    0.94 (2007-06-14):
            Zsolt Vasvari
            - Removed unneccessary checks from MVP and MVN

    0.93 (2003-07-05):
            Angelo Salese
            - Fixed the BCD conversion when using the Decimal Flag in ADC and SBC.
            - Removed the two conversion tables for ADC and SBC as they aren't
              needed anymore.

    0.92 (2000-05-28):
            Lee Hammerton <lee-hammerton@hammerhead.ltd.uk>
            - Fixed debugger bug that caused D to be misrepresented.
            - Fixed MVN and MVP (they were reversed)

    0.91 (2000-05-22):
            Lee Hammerton <lee-hammerton@hammerhead.ltd.uk>
            - Fixed reset vector fetch to be little endian
            - Fixed disassembler call bug
            - Fixed C flag in SBC (should be inverted before operation)
            - Fixed JSR to stack PC-1 and RTS to pull PC and add 1

            Karl Stenerud <karl@higashiyama-unet.ocn.ne.jp>
            - Added correct timing for absolute indexed operations
            - SBC: fixed corruption of interim values

    0.90 (2000-05-17):
            Karl Stenerud <karl@higashiyama-unet.ocn.ne.jp>
            - first public release


Note on timings:
    - For instructions that write to memory (ASL, ASR, LSL, ROL, ROR, DEC,
      INC, STA, STZ), the absolute indexed addressing mode takes 1 extra
      cycle to complete.
    - The spec says fc (JMP axi) is 6 cyles, but elsewhere says 8 cycles
      (which is what it should be)


TODO general:
    - WAI will not stop if RDY is held high.

    - RDY internally held low when WAI executed and returned to hi when RES,
      ABORT, NMI, or IRQ asserted.

    - ABORT will terminate WAI instruction but wil not restart the processor

    - If interrupt occurs after ABORT of WAI, processor returns to WAI
      instruction.

    - Add one cycle when indexing across page boundary and E=1 except for STA
      and STZ instructions.

    - Add 1 cycle if branch is taken. In Emulation (E= 1 ) mode only --add 1
      cycle if the branch is taken and crosses a page boundary.

    - Add 1 cycle in Emulation mode (E=1) for (dir),y; abs,x; and abs,y
      addressing modes.

*/
/* ======================================================================== */
/* ================================= DATA ================================= */
/* ======================================================================== */

#include "emu.h"
#include "g65816.h"

INLINE g65816i_cpu_struct *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->type() == G65816 || device->type() == _5A22);
	return (g65816i_cpu_struct *)downcast<legacy_cpu_device *>(device)->token();
}

/* Temporary Variables */

static CPU_READOP( g65816 )
{
	g65816i_cpu_struct *cpustate = get_safe_token(device);

	*value = g65816_read_8_immediate(offset);

	return 1;
}

extern void (*const g65816i_opcodes_M0X0[])(g65816i_cpu_struct *cpustate);
extern uint g65816i_get_reg_M0X0(g65816i_cpu_struct *cpustate, int regnum);
extern void g65816i_set_reg_M0X0(g65816i_cpu_struct *cpustate, int regnum, uint val);
extern void g65816i_set_line_M0X0(g65816i_cpu_struct *cpustate, int line, int state);
extern int  g65816i_execute_M0X0(g65816i_cpu_struct *cpustate, int cycles);

extern void (*const g65816i_opcodes_M0X1[])(g65816i_cpu_struct *cpustate);
extern uint g65816i_get_reg_M0X1(g65816i_cpu_struct *cpustate, int regnum);
extern void g65816i_set_reg_M0X1(g65816i_cpu_struct *cpustate, int regnum, uint val);
extern void g65816i_set_line_M0X1(g65816i_cpu_struct *cpustate, int line, int state);
extern int  g65816i_execute_M0X1(g65816i_cpu_struct *cpustate, int cycles);

extern void (*const g65816i_opcodes_M1X0[])(g65816i_cpu_struct *cpustate);
extern uint g65816i_get_reg_M1X0(g65816i_cpu_struct *cpustate, int regnum);
extern void g65816i_set_reg_M1X0(g65816i_cpu_struct *cpustate, int regnum, uint val);
extern void g65816i_set_line_M1X0(g65816i_cpu_struct *cpustate, int line, int state);
extern int  g65816i_execute_M1X0(g65816i_cpu_struct *cpustate, int cycles);

extern void (*const g65816i_opcodes_M1X1[])(g65816i_cpu_struct *cpustate);
extern uint g65816i_get_reg_M1X1(g65816i_cpu_struct *cpustate, int regnum);
extern void g65816i_set_reg_M1X1(g65816i_cpu_struct *cpustate, int regnum, uint val);
extern void g65816i_set_line_M1X1(g65816i_cpu_struct *cpustate, int line, int state);
extern int  g65816i_execute_M1X1(g65816i_cpu_struct *cpustate, int cycles);

extern void (*const g65816i_opcodes_E[])(g65816i_cpu_struct *cpustate);
extern uint g65816i_get_reg_E(g65816i_cpu_struct *cpustate, int regnum);
extern void g65816i_set_reg_E(g65816i_cpu_struct *cpustate, int regnum, uint val);
extern void g65816i_set_line_E(g65816i_cpu_struct *cpustate, int line, int state);
extern int  g65816i_execute_E(g65816i_cpu_struct *cpustate, int cycles);

void (*const *const g65816i_opcodes[5])(g65816i_cpu_struct *cpustate) =
{
	g65816i_opcodes_M0X0,
	g65816i_opcodes_M0X1,
	g65816i_opcodes_M1X0,
	g65816i_opcodes_M1X1,
	g65816i_opcodes_E
};

uint (*const g65816i_get_reg[5])(g65816i_cpu_struct *cpustate, int regnum) =
{
	g65816i_get_reg_M0X0,
	g65816i_get_reg_M0X1,
	g65816i_get_reg_M1X0,
	g65816i_get_reg_M1X1,
	g65816i_get_reg_E
};

void (*const g65816i_set_reg[5])(g65816i_cpu_struct *cpustate, int regnum, uint val) =
{
	g65816i_set_reg_M0X0,
	g65816i_set_reg_M0X1,
	g65816i_set_reg_M1X0,
	g65816i_set_reg_M1X1,
	g65816i_set_reg_E
};

void (*const g65816i_set_line[5])(g65816i_cpu_struct *cpustate, int line, int state) =
{
	g65816i_set_line_M0X0,
	g65816i_set_line_M0X1,
	g65816i_set_line_M1X0,
	g65816i_set_line_M1X1,
	g65816i_set_line_E
};

int (*const g65816i_execute[5])(g65816i_cpu_struct *cpustate, int cycles) =
{
	g65816i_execute_M0X0,
	g65816i_execute_M0X1,
	g65816i_execute_M1X0,
	g65816i_execute_M1X1,
	g65816i_execute_E
};

/* ======================================================================== */
/* ================================= API ================================== */
/* ======================================================================== */


static CPU_RESET( g65816 )
{
	g65816i_cpu_struct *cpustate = get_safe_token(device);

	/* Start the CPU */
	CPU_STOPPED = 0;

	/* Put into emulation mode */
	REGISTER_D = 0;
	REGISTER_PB = 0;
	REGISTER_DB = 0;
	REGISTER_S = (REGISTER_S & 0xff) | 0x100;
	REGISTER_X &= 0xff;
	REGISTER_Y &= 0xff;
	if(!FLAG_M)
	{
		REGISTER_B = REGISTER_A & 0xff00;
		REGISTER_A &= 0xff;
	}
	FLAG_E = EFLAG_SET;
	FLAG_M = MFLAG_SET;
	FLAG_X = XFLAG_SET;

	/* Clear D and set I */
	FLAG_D = DFLAG_CLEAR;
	FLAG_I = IFLAG_SET;

	/* Clear all pending interrupts (should we really do this?) */
	LINE_IRQ = 0;
	LINE_NMI = 0;
	IRQ_DELAY = 0;

	/* Set the function tables to emulation mode */
	g65816i_set_execution_mode(cpustate, EXECUTION_MODE_E);

	/* 6502 expects these, but its not in the 65816 spec */
	FLAG_Z = ZFLAG_CLEAR;
	REGISTER_S = 0x1ff;

	/* Fetch the reset vector */
	REGISTER_PC = g65816_read_8(VECTOR_RESET) | (g65816_read_8(VECTOR_RESET+1)<<8);
	g65816i_jumping(REGISTER_PB | REGISTER_PC);
}

/* Exit and clean up */
static CPU_EXIT( g65816 )
{
	/* nothing to do yet */
}

/* Execute some instructions */
static CPU_EXECUTE( g65816 )
{
	g65816i_cpu_struct *cpustate = get_safe_token(device);

	int clocks = cpustate->ICount;
	cpustate->ICount = clocks - FTABLE_EXECUTE(cpustate, cpustate->ICount);
}


/* Get the current Program Counter */
static unsigned g65816_get_pc(g65816i_cpu_struct *cpustate)
{
	return REGISTER_PB | REGISTER_PC;
}

/* Set the Program Counter */
static void g65816_set_pc(g65816i_cpu_struct *cpustate, unsigned val)
{
	REGISTER_PC = MAKE_UINT_16(val);
	REGISTER_PB = (val >> 16) & 0xFF;
	g65816_jumping(REGISTER_PB | REGISTER_PC);
}

/* Get the current Stack Pointer */
static unsigned g65816_get_sp(g65816i_cpu_struct *cpustate)
{
	return REGISTER_S;
}

/* Set the Stack Pointer */
static void g65816_set_sp(g65816i_cpu_struct *cpustate, unsigned val)
{
	REGISTER_S = FLAG_E ? MAKE_UINT_8(val) | 0x100 : MAKE_UINT_16(val);
}

/* Get a register */
static unsigned g65816_get_reg(g65816i_cpu_struct *cpustate, int regnum)
{
	/* Set the function tables to emulation mode if the FTABLE is NULL */
	if( FTABLE_GET_REG == NULL )
		g65816i_set_execution_mode(cpustate, EXECUTION_MODE_E);

	return FTABLE_GET_REG(cpustate, regnum);
}

/* Set a register */
static void g65816_set_reg(g65816i_cpu_struct *cpustate, int regnum, unsigned value)
{
	FTABLE_SET_REG(cpustate, regnum, value);
}

/* Set an interrupt line */
static void g65816_set_irq_line(g65816i_cpu_struct *cpustate, int line, int state)
{
	FTABLE_SET_LINE(cpustate, line, state);
}

/* Set the callback that is called when servicing an interrupt */
static void g65816_set_irq_callback(g65816i_cpu_struct *cpustate, device_irq_callback callback)
{
	INT_ACK = callback;
}


/* Disassemble an instruction */
#include "g65816ds.h"

static CPU_DISASSEMBLE( g65816 )
{
	g65816i_cpu_struct *cpustate = get_safe_token(device);

	return g65816_disassemble(buffer, (pc & 0x00ffff), (pc & 0xff0000) >> 16, oprom, FLAG_M, FLAG_X);
}

static STATE_POSTLOAD( g65816_restore_state )
{
	g65816i_cpu_struct *cpustate = (g65816i_cpu_struct *)param;

	// restore proper function pointers
	g65816i_set_execution_mode(cpustate, (FLAG_M>>4) | (FLAG_X>>4));

	// make sure the memory system can keep up
	g65816i_jumping(REGISTER_PB | REGISTER_PC);
}

static CPU_INIT( g65816 )
{
	g65816i_cpu_struct *cpustate = get_safe_token(device);

	memset(cpustate, 0, sizeof(cpustate));

	g65816_set_irq_callback(cpustate, irqcallback);
	cpustate->device = device;
	cpustate->program = device->space(AS_PROGRAM);
	cpustate->cpu_type = CPU_TYPE_G65816;

	state_save_register_device_item(device, 0, cpustate->a);
	state_save_register_device_item(device, 0, cpustate->b);
	state_save_register_device_item(device, 0, cpustate->x);
	state_save_register_device_item(device, 0, cpustate->y);
	state_save_register_device_item(device, 0, cpustate->s);
	state_save_register_device_item(device, 0, cpustate->pc);
	state_save_register_device_item(device, 0, cpustate->ppc);
	state_save_register_device_item(device, 0, cpustate->pb);
	state_save_register_device_item(device, 0, cpustate->db);
	state_save_register_device_item(device, 0, cpustate->d);
	state_save_register_device_item(device, 0, cpustate->flag_e);
	state_save_register_device_item(device, 0, cpustate->flag_m);
	state_save_register_device_item(device, 0, cpustate->flag_x);
	state_save_register_device_item(device, 0, cpustate->flag_n);
	state_save_register_device_item(device, 0, cpustate->flag_v);
	state_save_register_device_item(device, 0, cpustate->flag_d);
	state_save_register_device_item(device, 0, cpustate->flag_i);
	state_save_register_device_item(device, 0, cpustate->flag_z);
	state_save_register_device_item(device, 0, cpustate->flag_c);
	state_save_register_device_item(device, 0, cpustate->line_irq);
	state_save_register_device_item(device, 0, cpustate->line_nmi);
	state_save_register_device_item(device, 0, cpustate->ir);
	state_save_register_device_item(device, 0, cpustate->irq_delay);
	state_save_register_device_item(device, 0, cpustate->stopped);

	state_save_register_postload(device->machine, g65816_restore_state, cpustate);
}

/**************************************************************************
 * Generic set_info
 **************************************************************************/

static CPU_SET_INFO( g65816 )
{
	g65816i_cpu_struct *cpustate = get_safe_token(device);

	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + G65816_LINE_IRQ:		g65816_set_irq_line(cpustate, G65816_LINE_IRQ, info->i); break;
		case CPUINFO_INT_INPUT_STATE + G65816_LINE_NMI:		g65816_set_irq_line(cpustate, G65816_LINE_NMI, info->i); break;
		case CPUINFO_INT_INPUT_STATE + G65816_LINE_ABORT:	g65816_set_irq_line(cpustate, G65816_LINE_ABORT, info->i); break;
		case CPUINFO_INT_INPUT_STATE + G65816_LINE_SO:		g65816_set_irq_line(cpustate, G65816_LINE_SO, info->i); break;
		case CPUINFO_INT_INPUT_STATE + G65816_LINE_RDY:		g65816_set_irq_line(cpustate, G65816_LINE_RDY, info->i); break;
		case CPUINFO_INT_INPUT_STATE + G65816_LINE_RESET:	g65816_set_irq_line(cpustate, G65816_LINE_RESET, info->i); break;

		case CPUINFO_INT_PC:							g65816_set_pc(cpustate, info->i);					break;
		case CPUINFO_INT_SP:							g65816_set_sp(cpustate, info->i);					break;

		case CPUINFO_INT_REGISTER + G65816_PC:			g65816_set_reg(cpustate, G65816_PC, info->i);		break;
		case CPUINFO_INT_REGISTER + G65816_S:			g65816_set_reg(cpustate, G65816_S, info->i);		break;
		case CPUINFO_INT_REGISTER + G65816_P:			g65816_set_reg(cpustate, G65816_P, info->i);		break;
		case CPUINFO_INT_REGISTER + G65816_A:			g65816_set_reg(cpustate, G65816_A, info->i);		break;
		case CPUINFO_INT_REGISTER + G65816_X:			g65816_set_reg(cpustate, G65816_X, info->i);		break;
		case CPUINFO_INT_REGISTER + G65816_Y:			g65816_set_reg(cpustate, G65816_Y, info->i);		break;
		case CPUINFO_INT_REGISTER + G65816_PB:			g65816_set_reg(cpustate, G65816_PB, info->i);		break;
		case CPUINFO_INT_REGISTER + G65816_DB:			g65816_set_reg(cpustate, G65816_DB, info->i);		break;
		case CPUINFO_INT_REGISTER + G65816_D:			g65816_set_reg(cpustate, G65816_D, info->i);		break;
		case CPUINFO_INT_REGISTER + G65816_E:			g65816_set_reg(cpustate, G65816_E, info->i);		break;
		case CPUINFO_INT_REGISTER + G65816_NMI_STATE:	g65816_set_reg(cpustate, G65816_NMI_STATE, info->i); break;
		case CPUINFO_INT_REGISTER + G65816_IRQ_STATE:	g65816_set_reg(cpustate, G65816_IRQ_STATE, info->i); break;
	}
}



void g65816_set_read_vector_callback(running_device *device, read8_space_func read_vector)
{
	g65816i_cpu_struct *cpustate = get_safe_token(device);
	READ_VECTOR = read_vector;
}

/**************************************************************************
 * Generic get_info
 **************************************************************************/

CPU_GET_INFO( g65816 )
{
	g65816i_cpu_struct *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:				info->i = sizeof(g65816i_cpu_struct);			break;
		case CPUINFO_INT_INPUT_LINES:				info->i = 1;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case DEVINFO_INT_ENDIANNESS:				info->i = ENDIANNESS_LITTLE;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:			info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:				info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MIN_CYCLES:				info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:				info->i = 20; /* rough guess */			break;

		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 8;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 24;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;					break;
		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:	info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO:	info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO:	info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + G65816_LINE_IRQ:		info->i = LINE_IRQ;					break;
		case CPUINFO_INT_INPUT_STATE + G65816_LINE_NMI:		info->i = LINE_NMI;					break;
		case CPUINFO_INT_INPUT_STATE + G65816_LINE_ABORT:	info->i = 0;						break;
		case CPUINFO_INT_INPUT_STATE + G65816_LINE_SO:		info->i = 0;						break;
		case CPUINFO_INT_INPUT_STATE + G65816_LINE_RDY:		info->i = 0;						break;
		case CPUINFO_INT_INPUT_STATE + G65816_LINE_RESET:	info->i = 0;						break;

		case CPUINFO_INT_PREVIOUSPC:				/* not supported */						break;

		case CPUINFO_INT_PC:					info->i = g65816_get_pc(cpustate);				break;
		case CPUINFO_INT_SP:					info->i = g65816_get_sp(cpustate);				break;

		case CPUINFO_INT_REGISTER + G65816_PC:			info->i = g65816_get_pc(cpustate);				break;
		case CPUINFO_INT_REGISTER + G65816_S:			info->i = g65816_get_reg(cpustate, G65816_S);		break;
		case CPUINFO_INT_REGISTER + G65816_P:			info->i = g65816_get_reg(cpustate, G65816_P);		break;
		case CPUINFO_INT_REGISTER + G65816_A:			info->i = g65816_get_reg(cpustate, G65816_A);		break;
		case CPUINFO_INT_REGISTER + G65816_X:			info->i = g65816_get_reg(cpustate, G65816_X);		break;
		case CPUINFO_INT_REGISTER + G65816_Y:			info->i = g65816_get_reg(cpustate, G65816_Y);		break;
		case CPUINFO_INT_REGISTER + G65816_PB:			info->i = g65816_get_reg(cpustate, G65816_PB);	break;
		case CPUINFO_INT_REGISTER + G65816_DB:			info->i = g65816_get_reg(cpustate, G65816_DB);	break;
		case CPUINFO_INT_REGISTER + G65816_D:			info->i = g65816_get_reg(cpustate, G65816_D);		break;
		case CPUINFO_INT_REGISTER + G65816_E:			info->i = g65816_get_reg(cpustate, G65816_E);		break;
		case CPUINFO_INT_REGISTER + G65816_NMI_STATE:	info->i = g65816_get_reg(cpustate, G65816_NMI_STATE); break;
		case CPUINFO_INT_REGISTER + G65816_IRQ_STATE:	info->i = g65816_get_reg(cpustate, G65816_IRQ_STATE); break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(g65816);		break;
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(g65816);				break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(g65816);				break;
		case CPUINFO_FCT_EXIT:							info->exit = CPU_EXIT_NAME(g65816);				break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(g65816);			break;
		case CPUINFO_FCT_BURN:							info->burn = NULL;						break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(g65816);		break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cpustate->ICount;			break;

		case CPUINFO_FCT_READOP:						info->readop = CPU_READOP_NAME(g65816);			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "G65C816");				break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "6500");				break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");				break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);				break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Karl Stenerud, all rights reserved."); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%c%c%c%c%c%c%c%c",
				cpustate->flag_n & NFLAG_SET ? 'N':'.',
				cpustate->flag_v & VFLAG_SET ? 'V':'.',
				cpustate->flag_m & MFLAG_SET ? 'M':'.',
				cpustate->flag_x & XFLAG_SET ? 'X':'.',
				cpustate->flag_d & DFLAG_SET ? 'D':'.',
				cpustate->flag_i & IFLAG_SET ? 'I':'.',
				cpustate->flag_z == 0        ? 'Z':'.',
				cpustate->flag_c & CFLAG_SET ? 'C':'.');
			break;

		case CPUINFO_STR_REGISTER + G65816_PC:			sprintf(info->s, "PC:%06X", g65816_get_pc(cpustate)); break;
		case CPUINFO_STR_REGISTER + G65816_PB:			sprintf(info->s, "PB:%02X", cpustate->pb>>16); break;
		case CPUINFO_STR_REGISTER + G65816_DB:			sprintf(info->s, "DB:%02X", cpustate->db>>16); break;
		case CPUINFO_STR_REGISTER + G65816_D:			sprintf(info->s, "D:%04X", cpustate->d); break;
		case CPUINFO_STR_REGISTER + G65816_S:			sprintf(info->s, "S:%04X", cpustate->s); break;
		case CPUINFO_STR_REGISTER + G65816_P:			sprintf(info->s, "P:%02X",
																 (cpustate->flag_n&0x80)		|
																((cpustate->flag_v>>1)&0x40)	|
																cpustate->flag_m				|
																cpustate->flag_x				|
																cpustate->flag_d				|
																cpustate->flag_i				|
																((!cpustate->flag_z)<<1)		|
																((cpustate->flag_c>>8)&1)); break;
		case CPUINFO_STR_REGISTER + G65816_E:			sprintf(info->s, "E:%d", cpustate->flag_e); break;
		case CPUINFO_STR_REGISTER + G65816_A:			sprintf(info->s, "A:%04X", cpustate->a | cpustate->b); break;
		case CPUINFO_STR_REGISTER + G65816_X:			sprintf(info->s, "X:%04X", cpustate->x); break;
		case CPUINFO_STR_REGISTER + G65816_Y:			sprintf(info->s, "Y:%04X", cpustate->y); break;
		case CPUINFO_STR_REGISTER + G65816_NMI_STATE:	sprintf(info->s, "NMI:%X", cpustate->line_nmi); break;
		case CPUINFO_STR_REGISTER + G65816_IRQ_STATE:	sprintf(info->s, "IRQ:%X", cpustate->line_irq); break;
	}
}

/*
SNES specific, used to handle master cycles
*/

static CPU_INIT( 5a22 )
{
	g65816i_cpu_struct *cpustate = get_safe_token(device);

	CPU_INIT_CALL(g65816);

	cpustate->cpu_type = CPU_TYPE_5A22;
}


CPU_GET_INFO( _5a22 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(5a22);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "5A22");			break;

		default:										CPU_GET_INFO_CALL(g65816);				break;
	}
}

DEFINE_LEGACY_CPU_DEVICE(G65816, g65816);
DEFINE_LEGACY_CPU_DEVICE(_5A22, _5a22);

/* ======================================================================== */
/* ============================== END OF FILE ============================= */
/* ======================================================================== */
