/***************************************************************************

    dsp56k.c
    Core implementation for the portable DSP56k emulator.
    Written by Andrew Gardner

****************************************************************************

    Note:
    This CPU emulator is very much a work-in-progress.

    DONE:
    1:  1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
       11,  ,  ,  ,  ,  ,  ,18,  ,   ,

    TODO:
    X 1-6 Explore CORE naming scheme.
    - 1-9 paragraph 1 : memory access timings
    - 1-9 Data ALU arithmetic operations generally use fractional two's complement arithmetic
          (Unsigned numbers are only supported by the multiply and multiply-accumulate instruction)
    - 1-9 For fractional arithmetic, the 31-bit product is added to the 40-bit contents of A or B.  No pipeline!
    - 1-10 Two types of rounding: convergent rounding and two's complement rounding.  See status register bit R.
    - 1-10 Logic unit is 16-bits wide and works on MSP portion of accum register
    - 1-10 The AGU can implement three types of arithmetic: linear, modulo, and reverse carry.
    - 1-12 "Two external interrupt pins!!!"
    - 1-12 Take care of all interrupt priority (IPR) stuff!
    - 1-19 Memory WAIT states
    - 1-20 The timer's interesting!
    - 1-21 Vectored exception requests on the Host Interface!
***************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "dsp56k.h"

#include "dsp56def.h"

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/
static CPU_RESET( dsp56k );


/***************************************************************************
    ONBOARD MEMORY ALLOCATION
***************************************************************************/
// TODO: Put these in the cpustate!!!
static UINT16 *dsp56k_peripheral_ram;
static UINT16 *dsp56k_program_ram;


/***************************************************************************
    COMPONENT FUNCTIONALITY
***************************************************************************/
/* 1-9 ALU */
// #include "dsp56alu.c"

/* 1-10 Address Generation Unit (AGU) */
// #include "dsp56agu.c"

/* 1-11 Program Control Unit (PCU) */
#include "dsp56pcu.c"

/* 5-1 Host Interface (HI) */
//#include "dsp56hi.c"

/* 4-8 Memory handlers for on-chip peripheral memory. */
#include "dsp56mem.c"


/***************************************************************************
    Direct Update Handler
***************************************************************************/
static DIRECT_UPDATE_HANDLER( dsp56k_direct_handler )
{
	if (address >= (0x0000<<1) && address <= (0x07ff<<1))
	{
		direct->raw = direct->decrypted = (UINT8 *)(dsp56k_program_ram - (0x0000<<1));
		return ~0;
	}

	return address;
}


/***************************************************************************
    MEMORY ACCESSORS
***************************************************************************/
#define ROPCODE(pc)   memory_decrypted_read_word(cpustate->program, pc)


/***************************************************************************
    IRQ HANDLING
***************************************************************************/
static void set_irq_line(dsp56k_core* cpustate, int irqline, int state)
{
	//logerror("DSP56k set irq line %d %d\n", irqline, state);

	switch(irqline)
	{
		case DSP56K_IRQ_MODA:
			// TODO: 1-12 Get this triggering right
			if (irqa_trigger(cpustate))
				logerror("DSP56k IRQA is set to fire on the \"Negative Edge\".\n");

			if (state != CLEAR_LINE)
				cpustate->modA_state = TRUE;
			else
				cpustate->modA_state = FALSE;

			if (cpustate->reset_state != TRUE)
				dsp56k_add_pending_interrupt(cpustate, "IRQA");
			break;

		case DSP56K_IRQ_MODB:
			// TODO: 1-12 Get this triggering right
			if (irqb_trigger(cpustate))
				logerror("DSP56k IRQB is set to fire on the \"Negative Edge\".\n");

			if (state != CLEAR_LINE)
				cpustate->modB_state = TRUE;
			else
				cpustate->modB_state = FALSE;

			if (cpustate->reset_state != TRUE)
				dsp56k_add_pending_interrupt(cpustate, "IRQB");
			break;

		case DSP56K_IRQ_MODC:
			if (state != CLEAR_LINE)
				cpustate->modC_state = TRUE;
			else
				cpustate->modC_state = FALSE;

			// TODO : Set bus mode or whatever
			break;

		case DSP56K_IRQ_RESET:
			if (state != CLEAR_LINE)
				cpustate->reset_state = TRUE;
			else
			{
				/* If it changes state from asserted to cleared.  Call the reset function. */
				if (cpustate->reset_state == TRUE)
					CPU_RESET_NAME(dsp56k)(cpustate->device);

				cpustate->reset_state = FALSE;
			}

			// dsp56k_add_pending_interrupt("Hardware RESET");
			break;

		default:
			logerror("DSP56k setting some weird irq line : %d", irqline);
			break;
	}

	/* If the reset line isn't asserted, service interrupts */
	// TODO: Is it right to immediately service interrupts?
	//if (cpustate->reset_state != TRUE)
	//  pcu_service_interrupts();
}


/***************************************************************************
    INITIALIZATION AND SHUTDOWN
***************************************************************************/
static void agu_init(dsp56k_core* cpustate, running_device *device)
{
	/* save states - dsp56k_agu members */
	state_save_register_device_item(device, 0, cpustate->AGU.r0);
	state_save_register_device_item(device, 0, cpustate->AGU.r1);
	state_save_register_device_item(device, 0, cpustate->AGU.r2);
	state_save_register_device_item(device, 0, cpustate->AGU.r3);
	state_save_register_device_item(device, 0, cpustate->AGU.n0);
	state_save_register_device_item(device, 0, cpustate->AGU.n1);
	state_save_register_device_item(device, 0, cpustate->AGU.n2);
	state_save_register_device_item(device, 0, cpustate->AGU.n3);
	state_save_register_device_item(device, 0, cpustate->AGU.m0);
	state_save_register_device_item(device, 0, cpustate->AGU.m1);
	state_save_register_device_item(device, 0, cpustate->AGU.m2);
	state_save_register_device_item(device, 0, cpustate->AGU.m3);
	state_save_register_device_item(device, 0, cpustate->AGU.temp);
}

static void alu_init(dsp56k_core* cpustate, running_device *device)
{
	/* save states - dsp56k_alu members */
	state_save_register_device_item(device, 0, cpustate->ALU.x);
	state_save_register_device_item(device, 0, cpustate->ALU.y);
	state_save_register_device_item(device, 0, cpustate->ALU.a);
	state_save_register_device_item(device, 0, cpustate->ALU.b);
}

static CPU_INIT( dsp56k )
{
	dsp56k_core* cpustate = get_safe_token(device);

	/* Call specific module inits */
	pcu_init(cpustate, device);
	agu_init(cpustate, device);
	alu_init(cpustate, device);

	/* HACK - You're not in bootstrap mode upon bootup */
	cpustate->bootstrap_mode = BOOTSTRAP_OFF;

	/* Clear the irq states */
	cpustate->modA_state = FALSE;
	cpustate->modB_state = FALSE;
	cpustate->modC_state = FALSE;
	cpustate->reset_state = FALSE;

	/* save states - dsp56k_core members */
	state_save_register_device_item(device, 0, cpustate->modA_state);
	state_save_register_device_item(device, 0, cpustate->modB_state);
	state_save_register_device_item(device, 0, cpustate->modC_state);
	state_save_register_device_item(device, 0, cpustate->reset_state);
	state_save_register_device_item(device, 0, cpustate->bootstrap_mode);
	state_save_register_device_item(device, 0, cpustate->repFlag);
	state_save_register_device_item(device, 0, cpustate->repAddr);
	state_save_register_device_item(device, 0, cpustate->icount);
	state_save_register_device_item(device, 0, cpustate->ppc);
	state_save_register_device_item(device, 0, cpustate->op);
	state_save_register_device_item(device, 0, cpustate->interrupt_cycles);

	/* save states - dsp56k_host_interface members */
	state_save_register_device_item(device, 0, cpustate->HI.icr);
	state_save_register_device_item(device, 0, cpustate->HI.cvr);
	state_save_register_device_item(device, 0, cpustate->HI.isr);
	state_save_register_device_item(device, 0, cpustate->HI.ivr);
	state_save_register_device_item(device, 0, cpustate->HI.trxh);
	state_save_register_device_item(device, 0, cpustate->HI.trxl);
	state_save_register_device_item(device, 0, cpustate->HI.bootstrap_offset);

	//cpustate->config = device->baseconfig().static_config();
	//cpustate->irq_callback = irqcallback;
	cpustate->device = device;
	cpustate->program = device->space(AS_PROGRAM);
	cpustate->data = device->space(AS_DATA);

	/* Setup the direct memory handler for this CPU */
	/* NOTE: Be sure to grab this guy and call him if you ever install another direct_update_hander in a driver! */
	memory_set_direct_update_handler(cpustate->program, dsp56k_direct_handler);
}


/***************************************************************************
    RESET BEHAVIOR
***************************************************************************/
static void agu_reset(dsp56k_core* cpustate)
{
	/* FM.4-3 */
	R0 = 0x0000;
	R1 = 0x0000;
	R2 = 0x0000;
	R3 = 0x0000;

	N0 = 0x0000;
	N1 = 0x0000;
	N2 = 0x0000;
	N3 = 0x0000;

	M0 = 0xffff;
	M1 = 0xffff;
	M2 = 0xffff;
	M3 = 0xffff;

	TEMP = 0x0000;
}

static void alu_reset(dsp56k_core* cpustate)
{
	X = 0x00000000;
	Y = 0x00000000;
	A = 0x0000000000;
	B = 0x0000000000;
}

static CPU_RESET( dsp56k )
{
	dsp56k_core* cpustate = get_safe_token(device);
	logerror("Dsp56k reset\n");

	cpustate->interrupt_cycles = 0;
	cpustate->ppc = 0x0000;

	cpustate->repFlag = 0;
	cpustate->repAddr = 0x0000;

	pcu_reset(cpustate);
	mem_reset(cpustate);
	agu_reset(cpustate);
	alu_reset(cpustate);

	/* HACK - Put a jump to 0x0000 at 0x0000 - this keeps the CPU locked to the instruction at address 0x0000 */
	memory_write_word_16le(cpustate->program, 0x0000, 0x0124);
}


static CPU_EXIT( dsp56k )
{
}



/***************************************************************************
    CORE INCLUDE
***************************************************************************/
#define OP (cpustate->op)
#include "dsp56ops.c"


/***************************************************************************
    CORE EXECUTION LOOP
***************************************************************************/
static CPU_EXECUTE( dsp56k )
{
	dsp56k_core* cpustate = get_safe_token(device);

	/* If reset line is asserted, do nothing */
	if (cpustate->reset_state)
	{
		cpustate->icount = 0;
		return;
	}

	/* HACK - if you're in bootstrap mode, simply pretend you ate up all your cycles waiting for data. */
	if (cpustate->bootstrap_mode != BOOTSTRAP_OFF)
	{
		cpustate->icount = 0;
		return;
	}

	cpustate->icount -= cpustate->interrupt_cycles;
	cpustate->interrupt_cycles = 0;

	while(cpustate->icount > 0)
	{
		execute_one(cpustate);
		pcu_service_interrupts(cpustate);		/* TODO: There is definitely something un-right about this */
	}

	cpustate->icount -= cpustate->interrupt_cycles;
	cpustate->interrupt_cycles = 0;
}



/***************************************************************************
    DISASSEMBLY HOOK
***************************************************************************/
extern CPU_DISASSEMBLE( dsp56k );


/****************************************************************************
 *  Internal Memory Maps
 ****************************************************************************/
static ADDRESS_MAP_START( dsp56156_program_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x0000,0x07ff) AM_RAM AM_BASE(&dsp56k_program_ram)	/* 1-5 */
//  AM_RANGE(0x2f00,0x2fff) AM_ROM                              /* 1-5 PROM reserved memory.  Is this the right spot for it? */
ADDRESS_MAP_END

static ADDRESS_MAP_START( dsp56156_x_data_map, ADDRESS_SPACE_DATA, 16 )
	AM_RANGE(0x0000,0x07ff) AM_RAM								/* 1-5 */
	AM_RANGE(0xffc0,0xffff) AM_READWRITE(peripheral_register_r, peripheral_register_w) AM_BASE(&dsp56k_peripheral_ram)	/* 1-5 On-chip peripheral registers memory mapped in data space */
ADDRESS_MAP_END


/**************************************************************************
 * Generic set_info/get_info
 **************************************************************************/
static CPU_SET_INFO( dsp56k )
{
	dsp56k_core* cpustate = get_safe_token(device);

	switch (state)
	{
		case CPUINFO_INT_INPUT_STATE + DSP56K_IRQ_MODA:   set_irq_line(cpustate, DSP56K_IRQ_MODA, info->i);  break;
		case CPUINFO_INT_INPUT_STATE + DSP56K_IRQ_MODB:   set_irq_line(cpustate, DSP56K_IRQ_MODB, info->i);  break;
		case CPUINFO_INT_INPUT_STATE + DSP56K_IRQ_MODC:   set_irq_line(cpustate, DSP56K_IRQ_MODC, info->i);  break;
		case CPUINFO_INT_INPUT_STATE + DSP56K_IRQ_RESET:  set_irq_line(cpustate, DSP56K_IRQ_RESET, info->i); break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + DSP56K_PC:			PC  = info->i & 0xffff;					break;

		case CPUINFO_INT_REGISTER + DSP56K_SR:			SR  = info->i & 0xffff;					break;
		case CPUINFO_INT_REGISTER + DSP56K_LC:			LC  = info->i & 0xffff;					break;
		case CPUINFO_INT_REGISTER + DSP56K_LA:			LA  = info->i & 0xffff;					break;

		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + DSP56K_SP:			SP  = info->i & 0xff;					break;

		case CPUINFO_INT_REGISTER + DSP56K_OMR:			OMR = info->i & 0xff;					break;

		case CPUINFO_INT_REGISTER + DSP56K_X:			X   = info->i & 0xffffffff;				break;
		case CPUINFO_INT_REGISTER + DSP56K_Y:			Y   = info->i & 0xffffffff;				break;

		case CPUINFO_INT_REGISTER + DSP56K_A:			A   = info->i & (UINT64)U64(0xffffffffffffffff); break;	/* could benefit from a better mask? */
		case CPUINFO_INT_REGISTER + DSP56K_B:			B   = info->i & (UINT64)U64(0xffffffffffffffff); break;	/* could benefit from a better mask? */

		case CPUINFO_INT_REGISTER + DSP56K_R0:			R0  = info->i & 0xffff;					break;
		case CPUINFO_INT_REGISTER + DSP56K_R1:			R1  = info->i & 0xffff;					break;
		case CPUINFO_INT_REGISTER + DSP56K_R2:			R2  = info->i & 0xffff;					break;
		case CPUINFO_INT_REGISTER + DSP56K_R3:			R3  = info->i & 0xffff;					break;

		case CPUINFO_INT_REGISTER + DSP56K_N0:			N0  = info->i & 0xffff;					break;
		case CPUINFO_INT_REGISTER + DSP56K_N1:			N1  = info->i & 0xffff;					break;
		case CPUINFO_INT_REGISTER + DSP56K_N2:			N2  = info->i & 0xffff;					break;
		case CPUINFO_INT_REGISTER + DSP56K_N3:			N3  = info->i & 0xffff;					break;

		case CPUINFO_INT_REGISTER + DSP56K_M0:			M0  = info->i & 0xffff;					break;
		case CPUINFO_INT_REGISTER + DSP56K_M1:			M1  = info->i & 0xffff;					break;
		case CPUINFO_INT_REGISTER + DSP56K_M2:			M2  = info->i & 0xffff;					break;
		case CPUINFO_INT_REGISTER + DSP56K_M3:			M3  = info->i & 0xffff;					break;

		/*  case CPUINFO_INT_REGISTER + DSP56K_TEMP:        TEMP   = info->i & 0xffff;          break;  */
		/*  case CPUINFO_INT_REGISTER + DSP56K_STATUS:      STATUS = info->i & 0xff;            break;  */

		/* The CPU stack */
		case CPUINFO_INT_REGISTER + DSP56K_ST0:			ST0 = info->i  & 0xffffffff;			break;
		case CPUINFO_INT_REGISTER + DSP56K_ST1:			ST1 = info->i  & 0xffffffff;			break;
		case CPUINFO_INT_REGISTER + DSP56K_ST2:			ST2 = info->i  & 0xffffffff;			break;
		case CPUINFO_INT_REGISTER + DSP56K_ST3:			ST3 = info->i  & 0xffffffff;			break;
		case CPUINFO_INT_REGISTER + DSP56K_ST4:			ST4 = info->i  & 0xffffffff;			break;
		case CPUINFO_INT_REGISTER + DSP56K_ST5:			ST5 = info->i  & 0xffffffff;			break;
		case CPUINFO_INT_REGISTER + DSP56K_ST6:			ST6 = info->i  & 0xffffffff;			break;
		case CPUINFO_INT_REGISTER + DSP56K_ST7:			ST7 = info->i  & 0xffffffff;			break;
		case CPUINFO_INT_REGISTER + DSP56K_ST8:			ST8 = info->i  & 0xffffffff;			break;
		case CPUINFO_INT_REGISTER + DSP56K_ST9:			ST9 = info->i  & 0xffffffff;			break;
		case CPUINFO_INT_REGISTER + DSP56K_ST10:		ST10 = info->i & 0xffffffff;			break;
		case CPUINFO_INT_REGISTER + DSP56K_ST11:		ST11 = info->i & 0xffffffff;			break;
		case CPUINFO_INT_REGISTER + DSP56K_ST12:		ST12 = info->i & 0xffffffff;			break;
		case CPUINFO_INT_REGISTER + DSP56K_ST13:		ST13 = info->i & 0xffffffff;			break;
		case CPUINFO_INT_REGISTER + DSP56K_ST14:		ST14 = info->i & 0xffffffff;			break;
		case CPUINFO_INT_REGISTER + DSP56K_ST15:		ST15 = info->i & 0xffffffff;			break;
	}
}


CPU_GET_INFO( dsp56k )
{
	dsp56k_core* cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(dsp56k_core);			break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 4;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0x0000;						break;
		case DEVINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;			break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 2;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 2;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;	// ?
		case CPUINFO_INT_MAX_CYCLES:					info->i = 8;							break;	// ?

		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:			info->i = 16;							break;	/* 1-5 */
		case DEVINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: 		info->i = 16;							break;	/* 1-5 */
		case DEVINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: 		info->i = -1;							break;
		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:			info->i = 16;							break;
		case DEVINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA:			info->i = 16;							break;
		case DEVINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA:			info->i = -1;							break;
		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:				info->i = 0;							break;
		case DEVINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO:				info->i = 0;							break;
		case DEVINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO:				info->i = 0;							break;

		case CPUINFO_INT_INPUT_STATE + DSP56K_IRQ_MODA:	info->i = DSP56K_IRQ_MODA;				break;
		case CPUINFO_INT_INPUT_STATE + DSP56K_IRQ_MODB:	info->i = DSP56K_IRQ_MODB;				break;
		case CPUINFO_INT_INPUT_STATE + DSP56K_IRQ_MODC:	info->i = DSP56K_IRQ_MODC;				break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + DSP56K_PC:			info->i = PC;							break;
		case CPUINFO_INT_PREVIOUSPC:					info->i = cpustate->ppc;				break;

		case CPUINFO_INT_REGISTER + DSP56K_SR:			info->i = SR;							break;
		case CPUINFO_INT_REGISTER + DSP56K_LC:			info->i = LC;							break;
		case CPUINFO_INT_REGISTER + DSP56K_LA:			info->i = LA;							break;

		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + DSP56K_SP:			info->i = SP;							break;

		case CPUINFO_INT_REGISTER + DSP56K_OMR:			info->i = OMR;							break;

		case CPUINFO_INT_REGISTER + DSP56K_X:			info->i = X;							break;
		case CPUINFO_INT_REGISTER + DSP56K_Y:			info->i = Y;							break;

		case CPUINFO_INT_REGISTER + DSP56K_A:			info->i = A;							break;
		case CPUINFO_INT_REGISTER + DSP56K_B:			info->i = B;							break;

		case CPUINFO_INT_REGISTER + DSP56K_R0:			info->i = R0;							break;
		case CPUINFO_INT_REGISTER + DSP56K_R1:			info->i = R1;							break;
		case CPUINFO_INT_REGISTER + DSP56K_R2:			info->i = R2;							break;
		case CPUINFO_INT_REGISTER + DSP56K_R3:			info->i = R3;							break;

		case CPUINFO_INT_REGISTER + DSP56K_N0:			info->i = N0;							break;
		case CPUINFO_INT_REGISTER + DSP56K_N1:			info->i = N1;							break;
		case CPUINFO_INT_REGISTER + DSP56K_N2:			info->i = N2;							break;
		case CPUINFO_INT_REGISTER + DSP56K_N3:			info->i = N3;							break;

		case CPUINFO_INT_REGISTER + DSP56K_M0:			info->i = M0;							break;
		case CPUINFO_INT_REGISTER + DSP56K_M1:			info->i = M1;							break;
		case CPUINFO_INT_REGISTER + DSP56K_M2:			info->i = M2;							break;
		case CPUINFO_INT_REGISTER + DSP56K_M3:			info->i = M3;							break;

		/*  case CPUINFO_INT_REGISTER + DSP56K_TEMP:    info->i = TEMP;                         break;  */
		/*  case CPUINFO_INT_REGISTER + DSP56K_STATUS:  info->i = STATUS;                       break;  */

		/* The CPU stack */
		case CPUINFO_INT_REGISTER + DSP56K_ST0:			info->i = ST0;							break;
		case CPUINFO_INT_REGISTER + DSP56K_ST1:			info->i = ST1;							break;
		case CPUINFO_INT_REGISTER + DSP56K_ST2:			info->i = ST2;							break;
		case CPUINFO_INT_REGISTER + DSP56K_ST3:			info->i = ST3;							break;
		case CPUINFO_INT_REGISTER + DSP56K_ST4:			info->i = ST4;							break;
		case CPUINFO_INT_REGISTER + DSP56K_ST5:			info->i = ST5;							break;
		case CPUINFO_INT_REGISTER + DSP56K_ST6:			info->i = ST6;							break;
		case CPUINFO_INT_REGISTER + DSP56K_ST7:			info->i = ST7;							break;
		case CPUINFO_INT_REGISTER + DSP56K_ST8:			info->i = ST8;							break;
		case CPUINFO_INT_REGISTER + DSP56K_ST9:			info->i = ST9;							break;
		case CPUINFO_INT_REGISTER + DSP56K_ST10:		info->i = ST10;							break;
		case CPUINFO_INT_REGISTER + DSP56K_ST11:		info->i = ST11;							break;
		case CPUINFO_INT_REGISTER + DSP56K_ST12:		info->i = ST12;							break;
		case CPUINFO_INT_REGISTER + DSP56K_ST13:		info->i = ST13;							break;
		case CPUINFO_INT_REGISTER + DSP56K_ST14:		info->i = ST14;							break;
		case CPUINFO_INT_REGISTER + DSP56K_ST15:		info->i = ST15;							break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(dsp56k); break;
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(dsp56k);		break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(dsp56k);	break;
		case CPUINFO_FCT_EXIT:							info->exit = CPU_EXIT_NAME(dsp56k);		break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(dsp56k); break;
		case CPUINFO_FCT_BURN:							info->burn = NULL;						break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(dsp56k); break;
		case CPUINFO_FCT_DEBUG_INIT:					info->debug_init = NULL;				break;
		case CPUINFO_FCT_TRANSLATE:						info->translate = NULL;					break;
		case CPUINFO_FCT_READ:							info->read = NULL;						break;
		case CPUINFO_FCT_WRITE:							info->write = NULL;						break;
		case CPUINFO_FCT_READOP:						info->readop = NULL;					break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cpustate->icount;		break;
		case DEVINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_DATA:
			info->internal_map16 = ADDRESS_MAP_NAME(dsp56156_x_data_map);						break;
		case DEVINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_PROGRAM:
			info->internal_map16 = ADDRESS_MAP_NAME(dsp56156_program_map);						break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "DSP56156");			break;
		case DEVINFO_STR_FAMILY:						strcpy(info->s, "Motorola DSP56156");	break;
		case DEVINFO_STR_VERSION:						strcpy(info->s, "0.1");					break;
		case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);				break;
		case DEVINFO_STR_CREDITS:						strcpy(info->s, "Andrew Gardner");		break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%s%s %s%s%s%s%s%s%s%s %s%s",
				/* Status Register */
				LF_bit(cpustate) ? "L" : ".",
				FV_bit(cpustate) ? "F" : ".",

				S_bit(cpustate) ? "S" : ".",
				L_bit(cpustate) ? "L" : ".",
				E_bit(cpustate) ? "E" : ".",
				U_bit(cpustate) ? "U" : ".",
				N_bit(cpustate) ? "N" : ".",
				Z_bit(cpustate) ? "Z" : ".",
				V_bit(cpustate) ? "V" : ".",
				C_bit(cpustate) ? "C" : ".",

				/* Stack Pointer */
				UF_bit(cpustate) ? "U" : ".",
				SE_bit(cpustate) ? "S" : ".");
            break;

		case CPUINFO_STR_REGISTER + DSP56K_PC:			sprintf(info->s, "PC : %04x", PC);		break;
		case CPUINFO_STR_REGISTER + DSP56K_SR:			sprintf(info->s, "SR : %04x", SR);		break;
		case CPUINFO_STR_REGISTER + DSP56K_LC:			sprintf(info->s, "LC : %04x", LC);		break;
		case CPUINFO_STR_REGISTER + DSP56K_LA:			sprintf(info->s, "LA : %04x", LA);		break;
		case CPUINFO_STR_REGISTER + DSP56K_SP:			sprintf(info->s, "SP : %02x", SP);		break;
		case CPUINFO_STR_REGISTER + DSP56K_OMR:			sprintf(info->s, "OMR: %02x", OMR);		break;

		case CPUINFO_STR_REGISTER + DSP56K_X:			sprintf(info->s, "X  : %04x %04x", X1, X0); break;
		case CPUINFO_STR_REGISTER + DSP56K_Y:			sprintf(info->s, "Y  : %04x %04x", Y1, Y0); break;

		case CPUINFO_STR_REGISTER + DSP56K_A:			sprintf(info->s, "A  : %02x %04x %04x", A2,A1,A0); break;
		case CPUINFO_STR_REGISTER + DSP56K_B:			sprintf(info->s, "B  : %02x %04x %04x", B2,B1,B0); break;

		case CPUINFO_STR_REGISTER + DSP56K_R0:			sprintf(info->s, "R0 : %04x", R0);		break;
		case CPUINFO_STR_REGISTER + DSP56K_R1:			sprintf(info->s, "R1 : %04x", R1);		break;
		case CPUINFO_STR_REGISTER + DSP56K_R2:			sprintf(info->s, "R2 : %04x", R2);		break;
		case CPUINFO_STR_REGISTER + DSP56K_R3:			sprintf(info->s, "R3 : %04x", R3);		break;

		case CPUINFO_STR_REGISTER + DSP56K_N0:			sprintf(info->s, "N0 : %04x", N0);		break;
		case CPUINFO_STR_REGISTER + DSP56K_N1:			sprintf(info->s, "N1 : %04x", N1);		break;
		case CPUINFO_STR_REGISTER + DSP56K_N2:			sprintf(info->s, "N2 : %04x", N2);		break;
		case CPUINFO_STR_REGISTER + DSP56K_N3:			sprintf(info->s, "N3 : %04x", N3);		break;

		case CPUINFO_STR_REGISTER + DSP56K_M0:			sprintf(info->s, "M0 : %04x", M0);		break;
		case CPUINFO_STR_REGISTER + DSP56K_M1:			sprintf(info->s, "M1 : %04x", M1);		break;
		case CPUINFO_STR_REGISTER + DSP56K_M2:			sprintf(info->s, "M2 : %04x", M2);		break;
		case CPUINFO_STR_REGISTER + DSP56K_M3:			sprintf(info->s, "M3 : %04x", M3);		break;

		/*  case CPUINFO_STR_REGISTER + DSP56K_TEMP:    sprintf(info->s, "TMP: %04x", TEMP);    break;  */
		/*  case CPUINFO_STR_REGISTER + DSP56K_STATUS:  sprintf(info->s, "STS: %02x", STATUS);  break;  */

		/* The CPU stack */
		case CPUINFO_STR_REGISTER + DSP56K_ST0:			sprintf(info->s, "ST0 : %08x", ST0);	break;
		case CPUINFO_STR_REGISTER + DSP56K_ST1:			sprintf(info->s, "ST1 : %08x", ST1);	break;
		case CPUINFO_STR_REGISTER + DSP56K_ST2:			sprintf(info->s, "ST2 : %08x", ST2);	break;
		case CPUINFO_STR_REGISTER + DSP56K_ST3:			sprintf(info->s, "ST3 : %08x", ST3);	break;
		case CPUINFO_STR_REGISTER + DSP56K_ST4:			sprintf(info->s, "ST4 : %08x", ST4);	break;
		case CPUINFO_STR_REGISTER + DSP56K_ST5:			sprintf(info->s, "ST5 : %08x", ST5);	break;
		case CPUINFO_STR_REGISTER + DSP56K_ST6:			sprintf(info->s, "ST6 : %08x", ST6);	break;
		case CPUINFO_STR_REGISTER + DSP56K_ST7:			sprintf(info->s, "ST7 : %08x", ST7);	break;
		case CPUINFO_STR_REGISTER + DSP56K_ST8:			sprintf(info->s, "ST8 : %08x", ST8);	break;
		case CPUINFO_STR_REGISTER + DSP56K_ST9:			sprintf(info->s, "ST9 : %08x", ST9);	break;
		case CPUINFO_STR_REGISTER + DSP56K_ST10:		sprintf(info->s, "ST10: %08x", ST10);	break;
		case CPUINFO_STR_REGISTER + DSP56K_ST11:		sprintf(info->s, "ST11: %08x", ST11);	break;
		case CPUINFO_STR_REGISTER + DSP56K_ST12:		sprintf(info->s, "ST12: %08x", ST12);	break;
		case CPUINFO_STR_REGISTER + DSP56K_ST13:		sprintf(info->s, "ST13: %08x", ST13);	break;
		case CPUINFO_STR_REGISTER + DSP56K_ST14:		sprintf(info->s, "ST14: %08x", ST14);	break;
		case CPUINFO_STR_REGISTER + DSP56K_ST15:		sprintf(info->s, "ST15: %08x", ST15);	break;
	}
}

DEFINE_LEGACY_CPU_DEVICE(DSP56156, dsp56k);
