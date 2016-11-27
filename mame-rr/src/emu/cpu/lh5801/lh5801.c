/*****************************************************************************
 *
 *   Copyright Peter Trauner, all rights reserved.
 *
 *   - This source code is released as freeware for non-commercial purposes.
 *   - You are free to use and redistribute this code in modified or
 *     unmodified form, provided you list me in the credits.
 *   - If you modify this source code, you must add a notice to each modified
 *     source file that it has been changed.  If you're a nice person, you
 *     will clearly mark each change too.  :)
 *   - If you wish to use this for commercial purposes, please contact me at
 *     peter.trauner@jk.uni-linz.ac.at
 *   - The author of this copywritten work reserves the right to change the
 *     terms of its usage and license at any time, including retroactively
 *   - This entire notice must remain in the source code.
 *
 * based on info found on an artikel for the tandy trs80 pc2
 *
 *****************************************************************************/

#include "emu.h"
#include "debugger.h"

#include "lh5801.h"

#define VERBOSE 0

#define LOG(x)	do { if (VERBOSE) logerror x; } while (0)

enum
{
	LH5801_T=1,
	LH5801_P,
	LH5801_S,
	LH5801_U,
	LH5801_X,
	LH5801_Y,
	LH5801_A,

	LH5801_TM,
	LH5801_IN,
	LH5801_BF,
	LH5801_PU,
	LH5801_PV,
	LH5801_DP,
	LH5801_IRQ_STATE
};

typedef struct _lh5810_state lh5801_state;
struct _lh5810_state
{
	const lh5801_cpu_core *config;
	legacy_cpu_device *device;
	const address_space *program;

	PAIR s, p, u, x, y;
	int tm; //9 bit

	UINT8 t, a;

	int bf, dp, pu, pv;

	UINT16 oldpc;

	int irq_state;

	int idle;
	int icount;
};

INLINE lh5801_state *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->type() == LH5801);
	return (lh5801_state *)downcast<legacy_cpu_device *>(device)->token();
}

#define P cpustate->p.w.l
#define S cpustate->s.w.l
#define U cpustate->u.w.l
#define UL cpustate->u.b.l
#define UH cpustate->u.b.h
#define X cpustate->x.w.l
#define XL cpustate->x.b.l
#define XH cpustate->x.b.h
#define Y cpustate->y.w.l
#define YL cpustate->y.b.l
#define YH cpustate->y.b.h

#define C 0x01
#define IE 0x02
#define Z 0x04
#define V 0x08
#define H 0x10

/***************************************************************
 * include the opcode macros, functions and tables
 ***************************************************************/
#include "5801tbl.c"

static CPU_INIT( lh5801 )
{
	lh5801_state *cpustate = get_safe_token(device);

	memset(cpustate, 0, sizeof(*cpustate));
	cpustate->config = (const lh5801_cpu_core *) device->baseconfig().static_config();
	cpustate->device = device;
	cpustate->program = device->space(AS_PROGRAM);
}

static CPU_RESET( lh5801 )
{
	lh5801_state *cpustate = get_safe_token(device);

	P = (memory_read_byte(cpustate->program, 0xfffe)<<8) | memory_read_byte(cpustate->program, 0xffff);

	cpustate->idle=0;
}

static CPU_EXECUTE( lh5801 )
{
	lh5801_state *cpustate = get_safe_token(device);

	if (cpustate->idle) {
		cpustate->icount=0;
	} else {
		do
		{
			cpustate->oldpc = P;

			debugger_instruction_hook(device, P);
			lh5801_instruction(cpustate);

		} while (cpustate->icount > 0);
	}
}

static void set_irq_line(lh5801_state *cpustate, int irqline, int state)
{
	cpustate->idle=0;
}



/**************************************************************************
 * Generic set_info
 **************************************************************************/

static CPU_SET_INFO( lh5801 )
{
	lh5801_state *cpustate = get_safe_token(device);
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE:					set_irq_line(cpustate, 0, info->i);				break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + LH5801_P:			P = info->i;							break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + LH5801_S:			S = info->i;							break;
		case CPUINFO_INT_REGISTER + LH5801_U:			U = info->i;							break;
		case CPUINFO_INT_REGISTER + LH5801_X:			X = info->i;							break;
		case CPUINFO_INT_REGISTER + LH5801_Y:			Y = info->i;							break;
		case CPUINFO_INT_REGISTER + LH5801_T:			cpustate->t = info->i;						break;
		case CPUINFO_INT_REGISTER + LH5801_TM:			cpustate->tm = info->i;					break;
		case CPUINFO_INT_REGISTER + LH5801_BF:			cpustate->bf = info->i;					break;
		case CPUINFO_INT_REGISTER + LH5801_PV:			cpustate->pv = info->i;					break;
		case CPUINFO_INT_REGISTER + LH5801_PU:			cpustate->pu = info->i;					break;
		case CPUINFO_INT_REGISTER + LH5801_DP:			cpustate->dp = info->i;					break;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

CPU_GET_INFO( lh5801 )
{
	lh5801_state *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(lh5801_state);			break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 2;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case DEVINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 5;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 2;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 19;							break;

		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 8;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 16;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;					break;
		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO:		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE:					info->i = cpustate->irq_state;				break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = cpustate->oldpc;					break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + LH5801_P:			info->i = P;							break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + LH5801_S:			info->i = S;							break;
		case CPUINFO_INT_REGISTER + LH5801_U:			info->i = U;							break;
		case CPUINFO_INT_REGISTER + LH5801_X:			info->i = X;							break;
		case CPUINFO_INT_REGISTER + LH5801_Y:			info->i = Y;							break;
		case CPUINFO_INT_REGISTER + LH5801_T:			info->i = cpustate->t;						break;
		case CPUINFO_INT_REGISTER + LH5801_TM:			info->i = cpustate->tm;					break;
		case CPUINFO_INT_REGISTER + LH5801_IN:			info->i = cpustate->config->in(device);			break;
		case CPUINFO_INT_REGISTER + LH5801_BF:			info->i = cpustate->bf;					break;
		case CPUINFO_INT_REGISTER + LH5801_PV:			info->i = cpustate->pv;					break;
		case CPUINFO_INT_REGISTER + LH5801_PU:			info->i = cpustate->pu;					break;
		case CPUINFO_INT_REGISTER + LH5801_DP:			info->i = cpustate->dp;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(lh5801);		break;
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(lh5801);				break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(lh5801);				break;
		case CPUINFO_FCT_EXIT:							info->exit = NULL;						break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(lh5801);			break;
		case CPUINFO_FCT_BURN:							info->burn = NULL;						break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(lh5801);		break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cpustate->icount;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "LH5801"); break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "LH5801"); break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0alpha"); break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__); break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Peter Trauner, all rights reserved."); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%s%s%s%s%s%s%s%s",
				cpustate->t&0x80?"1":"0",
				cpustate->t&0x40?"1":"0",
				cpustate->t&0x20?"1":"0",
				cpustate->t&0x10?"H":".",
				cpustate->t&8?"V":".",
				cpustate->t&4?"Z":".",
				cpustate->t&2?"I":".",
				cpustate->t&1?"C":".");
			break;

		case CPUINFO_STR_REGISTER + LH5801_P:			sprintf(info->s, "P:%04X", cpustate->p.w.l); break;
		case CPUINFO_STR_REGISTER + LH5801_S:			sprintf(info->s, "S:%04X", cpustate->s.w.l); break;
		case CPUINFO_STR_REGISTER + LH5801_U:			sprintf(info->s, "U:%04X", cpustate->u.w.l); break;
		case CPUINFO_STR_REGISTER + LH5801_X:			sprintf(info->s, "X:%04X", cpustate->x.w.l); break;
		case CPUINFO_STR_REGISTER + LH5801_Y:			sprintf(info->s, "Y:%04X", cpustate->y.w.l); break;
		case CPUINFO_STR_REGISTER + LH5801_T:			sprintf(info->s, "T:%02X", cpustate->t); break;
		case CPUINFO_STR_REGISTER + LH5801_A:			sprintf(info->s, "A:%02X", cpustate->a); break;
		case CPUINFO_STR_REGISTER + LH5801_TM:			sprintf(info->s, "TM:%03X", cpustate->tm); break;
		case CPUINFO_STR_REGISTER + LH5801_IN:			sprintf(info->s, "IN:%02X", cpustate->config->in(device)); break;
		case CPUINFO_STR_REGISTER + LH5801_PV:			sprintf(info->s, "PV:%04X", cpustate->pv); break;
		case CPUINFO_STR_REGISTER + LH5801_PU:			sprintf(info->s, "PU:%04X", cpustate->pu); break;
		case CPUINFO_STR_REGISTER + LH5801_BF:			sprintf(info->s, "BF:%04X", cpustate->bf); break;
		case CPUINFO_STR_REGISTER + LH5801_DP:			sprintf(info->s, "DP:%04X", cpustate->dp); break;
	}
}

DEFINE_LEGACY_CPU_DEVICE(LH5801, lh5801);
