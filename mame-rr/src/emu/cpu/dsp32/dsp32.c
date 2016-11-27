/***************************************************************************

    dsp32.c
    Core implementation for the portable DSP32 emulator.
    Written by Aaron Giles

****************************************************************************

    Important note:

    At this time, the emulator is rather incomplete. However, it is
    sufficiently complete to run both Race Drivin' and Hard Drivin's
    Airborne, which is all I was after.

    Things that still need to be implemented:

        * interrupts
        * carry-reverse add operations
        * do loops
        * ieee/dsp conversions
        * input/output conversion
        * serial I/O

    In addition, there are several optimizations enabled which make
    assumptions about the code which may not be valid for other
    applications. Check dsp32ops.c for details.

***************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "dsp32.h"

CPU_DISASSEMBLE( dsp32c );


/***************************************************************************
    DEBUGGING
***************************************************************************/

#define DETECT_MISALIGNED_MEMORY	0



/***************************************************************************
    CONSTANTS
***************************************************************************/

/* internal register numbering for PIO registers */
#define PIO_PAR			0
#define PIO_PDR			1
#define PIO_EMR			2
#define PIO_ESR			3
#define PIO_PCR			4
#define PIO_PIR			5
#define PIO_PARE		6
#define PIO_PDR2		7
#define PIO_RESERVED	8

#define UPPER			(0x00ff << 8)
#define LOWER			(0xff00 << 8)

/* bits in the PCR register */
#define PCR_RESET		0x001
#define PCR_REGMAP		0x002
#define PCR_ENI			0x004
#define PCR_DMA			0x008
#define PCR_AUTO		0x010
#define PCR_PDFs		0x020
#define PCR_PIFs		0x040
#define PCR_RES			0x080
#define PCR_DMA32		0x100
#define PCR_PIO16		0x200
#define PCR_FLG			0x400

/* internal flag bits */
#define UFLAGBIT		1
#define VFLAGBIT		2



/***************************************************************************
    MACROS
***************************************************************************/

/* register mapping */
#define R0				r[0]
#define R1				r[1]
#define R2				r[2]
#define R3				r[3]
#define R4				r[4]
#define R5				r[5]
#define R6				r[6]
#define R7				r[7]
#define R8				r[8]
#define R9				r[9]
#define R10				r[10]
#define R11				r[11]
#define R12				r[12]
#define R13				r[13]
#define R14				r[14]
#define PC				r[15]
#define R0_ALT			r[16]
#define R15				r[17]
#define R16				r[18]
#define R17				r[19]
#define R18				r[20]
#define R19				r[21]
#define RMM				r[22]
#define RPP				r[23]
#define R20				r[24]
#define R21				r[25]
#define DAUC			r[26]
#define IOC				r[27]
#define R22				r[29]
#define PCSH			r[30]

#define A0				a[0]
#define A1				a[1]
#define A2				a[2]
#define A3				a[3]
#define A_0				a[4]
#define A_1				a[5]

#define zFLAG			((cpustate->nzcflags & 0xffffff) == 0)
#define nFLAG			((cpustate->nzcflags & 0x800000) != 0)
#define cFLAG			((cpustate->nzcflags & 0x1000000) != 0)
#define vFLAG			((cpustate->vflags & 0x800000) != 0)
#define ZFLAG			(cpustate->NZflags == 0)
#define NFLAG			(cpustate->NZflags < 0)
#define UFLAG			(cpustate->VUflags & UFLAGBIT)
#define VFLAG			(cpustate->VUflags & VFLAGBIT)



/***************************************************************************
    STRUCTURES & TYPEDEFS
***************************************************************************/

/* DSP32 Registers */
typedef struct _dsp32_state dsp32_state;
struct _dsp32_state
{
	/* core registers */
	UINT32			r[32];
	UINT32			pin, pout;
	UINT32			ivtp;
	UINT32			nzcflags;
	UINT32			vflags;

	double			a[6];
	double			NZflags;
	UINT8			VUflags;

	double			abuf[4];
	UINT8			abufreg[4];
	UINT8			abufVUflags[4];
	UINT8			abufNZflags[4];
	int				abufcycle[4];
	int				abuf_index;

	INT32			mbufaddr[4];
	UINT32			mbufdata[4];
	int				mbuf_index;

	UINT16			par;
	UINT8			pare;
	UINT16			pdr;
	UINT16			pdr2;
	UINT16			pir;
	UINT16			pcr;
	UINT16			emr;
	UINT8			esr;
	UINT16			pcw;
	UINT8			piop;

	UINT32			ibuf;
	UINT32			isr;
	UINT32			obuf;
	UINT32			osr;

	/* internal stuff */
	int				icount;
	UINT8			lastpins;
	UINT32			ppc;
	void			(*output_pins_changed)(running_device *device, UINT32 pins);
	legacy_cpu_device *	device;
	const address_space *program;
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static CPU_RESET( dsp32c );



/***************************************************************************
    STATE ACCESSORS
***************************************************************************/

INLINE dsp32_state *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->type() == DSP32C);
	return (dsp32_state *)downcast<legacy_cpu_device *>(device)->token();
}



/***************************************************************************
    MEMORY ACCESSORS
***************************************************************************/

#define ROPCODE(cs,pc)			memory_decrypted_read_dword((cs)->program, pc)

#define RBYTE(cs,addr)			memory_read_byte_32le((cs)->program, addr)
#define WBYTE(cs,addr,data)		memory_write_byte_32le((cs)->program, (addr), data)

#if (!DETECT_MISALIGNED_MEMORY)

#define RWORD(cs,addr)			memory_read_word_32le((cs)->program, addr)
#define WWORD(cs,addr,data)		memory_write_word_32le((cs)->program, (addr), data)
#define RLONG(cs,addr)			memory_read_dword_32le((cs)->program, addr)
#define WLONG(cs,addr,data)		memory_write_dword_32le((cs)->program, (addr), data)

#else

INLINE UINT16 RWORD(dsp32_state *cpustate, offs_t addr)
{
	UINT16 data;
	if (addr & 1) fprintf(stderr, "Unaligned word read @ %06X, PC=%06X\n", addr, cpustate->PC);
	data = memory_read_word_32le(cpustate->program, addr);
	return data;
}

INLINE UINT32 RLONG(dsp32_state *cpustate, offs_t addr)
{
	UINT32 data;
	if (addr & 3) fprintf(stderr, "Unaligned long read @ %06X, PC=%06X\n", addr, cpustate->PC);
	data = memory_write_word_32le(cpustate->program, addr);
	return data;
}

INLINE void WWORD(dsp32_state *cpustate, offs_t addr, UINT16 data)
{
	if (addr & 1) fprintf(stderr, "Unaligned word write @ %06X, PC=%06X\n", addr, cpustate->PC);
	memory_read_dword_32le(cpustate->program, (addr), data);
}

INLINE void WLONG(dsp32_state *cpustate, offs_t addr, UINT32 data)
{
	if (addr & 3) fprintf(stderr, "Unaligned long write @ %06X, PC=%06X\n", addr, cpustate->PC);
	memory_write_dword_32le(cpustate->program, (addr), data);
}

#endif



/***************************************************************************
    IRQ HANDLING
***************************************************************************/

static void check_irqs(dsp32_state *cpustate)
{
	/* finish me! */
}


static void set_irq_line(dsp32_state *cpustate, int irqline, int state)
{
	/* finish me! */
}



/***************************************************************************
    REGISTER HANDLING
***************************************************************************/

static void update_pcr(dsp32_state *cpustate, UINT16 newval)
{
	UINT16 oldval = cpustate->pcr;
	cpustate->pcr = newval;

	/* reset the chip if we get a reset */
	if ((oldval & PCR_RESET) == 0 && (newval & PCR_RESET) != 0)
		CPU_RESET_NAME(dsp32c)(cpustate->device);

	/* track the state of the output pins */
	if (cpustate->output_pins_changed)
	{
		UINT16 newoutput = ((newval & (PCR_PIFs | PCR_ENI)) == (PCR_PIFs | PCR_ENI)) ? DSP32_OUTPUT_PIF : 0;
		if (newoutput != cpustate->lastpins)
		{
			cpustate->lastpins = newoutput;
			(*cpustate->output_pins_changed)(cpustate->device, newoutput);
		}
	}
}



/***************************************************************************
    INITIALIZATION AND SHUTDOWN
***************************************************************************/

static void dsp32_register_save( running_device *device )
{
	dsp32_state *cpustate = get_safe_token(device);

	state_save_register_device_item_array(device, 0, cpustate->r);
	state_save_register_device_item(device, 0, cpustate->pin);
	state_save_register_device_item(device, 0, cpustate->pout);
	state_save_register_device_item(device, 0, cpustate->ivtp);
	state_save_register_device_item(device, 0, cpustate->nzcflags);
	state_save_register_device_item(device, 0, cpustate->vflags);
	state_save_register_device_item_array(device, 0, cpustate->a);
	state_save_register_device_item(device, 0, cpustate->NZflags);
	state_save_register_device_item(device, 0, cpustate->VUflags);
	state_save_register_device_item_array(device, 0, cpustate->abuf);
	state_save_register_device_item_array(device, 0, cpustate->abufreg);
	state_save_register_device_item_array(device, 0, cpustate->abufVUflags);
	state_save_register_device_item_array(device, 0, cpustate->abufNZflags);
	state_save_register_device_item_array(device, 0, cpustate->abufcycle);
	state_save_register_device_item(device, 0, cpustate->abuf_index);
	state_save_register_device_item_array(device, 0, cpustate->mbufaddr);
	state_save_register_device_item_array(device, 0, cpustate->mbufdata);
	state_save_register_device_item(device, 0, cpustate->par);
	state_save_register_device_item(device, 0, cpustate->pare);
	state_save_register_device_item(device, 0, cpustate->pdr);
	state_save_register_device_item(device, 0, cpustate->pdr2);
	state_save_register_device_item(device, 0, cpustate->pir);
	state_save_register_device_item(device, 0, cpustate->pcr);
	state_save_register_device_item(device, 0, cpustate->emr);
	state_save_register_device_item(device, 0, cpustate->esr);
	state_save_register_device_item(device, 0, cpustate->pcw);
	state_save_register_device_item(device, 0, cpustate->piop);
	state_save_register_device_item(device, 0, cpustate->ibuf);
	state_save_register_device_item(device, 0, cpustate->isr);
	state_save_register_device_item(device, 0, cpustate->obuf);
	state_save_register_device_item(device, 0, cpustate->osr);
	state_save_register_device_item(device, 0, cpustate->lastpins);
	state_save_register_device_item(device, 0, cpustate->ppc);
}

static CPU_INIT( dsp32c )
{
	const dsp32_config *configdata = (const dsp32_config *)device->baseconfig().static_config();
	dsp32_state *cpustate = get_safe_token(device);

	/* copy in config data */
	if (configdata != NULL)
		cpustate->output_pins_changed = configdata->output_pins_changed;

	cpustate->device = device;
	cpustate->program = device->space(AS_PROGRAM);

	dsp32_register_save(device);
}


static CPU_RESET( dsp32c )
{
	dsp32_state *cpustate = get_safe_token(device);

	/* reset goes to 0 */
	cpustate->PC = 0;

	/* clear some registers */
	cpustate->pcw &= 0x03ff;
	update_pcr(cpustate, cpustate->pcr & PCR_RESET);
	cpustate->esr = 0;
	cpustate->emr = 0xffff;

	/* initialize fixed registers */
	cpustate->R0 = cpustate->R0_ALT = 0;
	cpustate->RMM = -1;
	cpustate->RPP = 1;
	cpustate->A_0 = 0.0;
	cpustate->A_1 = 1.0;

	/* init internal stuff */
	cpustate->abufcycle[0] = cpustate->abufcycle[1] = cpustate->abufcycle[2] = cpustate->abufcycle[3] = 12345678;
	cpustate->mbufaddr[0] = cpustate->mbufaddr[1] = cpustate->mbufaddr[2] = cpustate->mbufaddr[3] = 1;
}


static CPU_EXIT( dsp32c )
{
}



/***************************************************************************
    CORE INCLUDE
***************************************************************************/

#include "dsp32ops.c"



/***************************************************************************
    CORE EXECUTION LOOP
***************************************************************************/

static CPU_EXECUTE( dsp32c )
{
	dsp32_state *cpustate = get_safe_token(device);

	/* skip if halted */
	if ((cpustate->pcr & PCR_RESET) == 0)
	{
		cpustate->icount = 0;
		return;
	}

	/* update buffered accumulator values */
	cpustate->abufcycle[0] += cpustate->icount;
	cpustate->abufcycle[1] += cpustate->icount;
	cpustate->abufcycle[2] += cpustate->icount;
	cpustate->abufcycle[3] += cpustate->icount;

	/* handle interrupts */
	check_irqs(cpustate);

	while (cpustate->icount > 0)
		execute_one(cpustate);

	/* normalize buffered accumulator values */
	cpustate->abufcycle[0] -= cpustate->icount;
	cpustate->abufcycle[1] -= cpustate->icount;
	cpustate->abufcycle[2] -= cpustate->icount;
	cpustate->abufcycle[3] -= cpustate->icount;
}



/***************************************************************************
    PARALLEL INTERFACE WRITES
***************************************************************************/

static const UINT32 regmap[4][16] =
{
	{	/* DSP32 compatible mode */
		PIO_PAR|LOWER, PIO_PAR|UPPER, PIO_PDR|LOWER, PIO_PDR|UPPER,
		PIO_EMR|LOWER, PIO_EMR|UPPER, PIO_ESR|LOWER, PIO_PCR|LOWER,
		PIO_PIR|UPPER, PIO_PIR|UPPER, PIO_PIR|UPPER, PIO_PIR|UPPER,
		PIO_PIR|UPPER, PIO_PIR|UPPER, PIO_PIR|UPPER, PIO_PIR|UPPER
	},
	{	/* DSP32C 8-bit mode */
		PIO_PAR|LOWER, PIO_PAR|UPPER, PIO_PDR|LOWER, PIO_PDR|UPPER,
		PIO_EMR|LOWER, PIO_EMR|UPPER, PIO_ESR|LOWER, PIO_PCR|LOWER,
		PIO_PIR|LOWER, PIO_PIR|UPPER, PIO_PCR|UPPER, PIO_PARE|LOWER,
		PIO_PDR2|LOWER,PIO_PDR2|UPPER,PIO_RESERVED,  PIO_RESERVED
	},
	{	/* DSP32C illegal mode */
		PIO_RESERVED,  PIO_RESERVED,  PIO_RESERVED,  PIO_RESERVED,
		PIO_RESERVED,  PIO_RESERVED,  PIO_RESERVED,  PIO_RESERVED,
		PIO_RESERVED,  PIO_RESERVED,  PIO_RESERVED,  PIO_RESERVED,
		PIO_RESERVED,  PIO_RESERVED,  PIO_RESERVED,  PIO_RESERVED
	},
	{	/* DSP32C 16-bit mode */
		PIO_PAR,       PIO_RESERVED,  PIO_PDR,       PIO_RESERVED,
		PIO_EMR,       PIO_RESERVED,  PIO_ESR|LOWER, PIO_PCR,
		PIO_PIR,       PIO_RESERVED,  PIO_RESERVED,  PIO_PARE|LOWER,
		PIO_PDR2,      PIO_RESERVED,  PIO_RESERVED,  PIO_RESERVED
	}
};



/***************************************************************************
    PARALLEL INTERFACE WRITES
***************************************************************************/

INLINE void dma_increment(dsp32_state *cpustate)
{
	if (cpustate->pcr & PCR_AUTO)
	{
		int amount = (cpustate->pcr & PCR_DMA32) ? 4 : 2;
		cpustate->par += amount;
		if (cpustate->par < amount)
			cpustate->pare++;
	}
}


INLINE void dma_load(dsp32_state *cpustate)
{
	/* only process if DMA is enabled */
	if (cpustate->pcr & PCR_DMA)
	{
		UINT32 addr = cpustate->par | (cpustate->pare << 16);

		/* 16-bit case */
		if (!(cpustate->pcr & PCR_DMA32))
			cpustate->pdr = RWORD(cpustate, addr & 0xfffffe);

		/* 32-bit case */
		else
		{
			UINT32 temp = RLONG(cpustate, addr & 0xfffffc);
			cpustate->pdr = temp >> 16;
			cpustate->pdr2 = temp & 0xffff;
		}

		/* set the PDF flag to indicate we have data ready */
		update_pcr(cpustate, cpustate->pcr | PCR_PDFs);
	}
}


INLINE void dma_store(dsp32_state *cpustate)
{
	/* only process if DMA is enabled */
	if (cpustate->pcr & PCR_DMA)
	{
		UINT32 addr = cpustate->par | (cpustate->pare << 16);

		/* 16-bit case */
		if (!(cpustate->pcr & PCR_DMA32))
			WWORD(cpustate, addr & 0xfffffe, cpustate->pdr);

		/* 32-bit case */
		else
			WLONG(cpustate, addr & 0xfffffc, (cpustate->pdr << 16) | cpustate->pdr2);

		/* clear the PDF flag to indicate we have taken the data */
		update_pcr(cpustate, cpustate->pcr & ~PCR_PDFs);
	}
}


void dsp32c_pio_w(running_device *device, int reg, int data)
{
	dsp32_state *cpustate = get_safe_token(device);
	UINT16 mask;
	UINT8 mode;

	/* look up register and mask */
	mode = ((cpustate->pcr >> 8) & 2) | ((cpustate->pcr >> 1) & 1);
	reg = regmap[mode][reg];
	mask = reg >> 8;
	if (mask == 0x00ff) data <<= 8;
	data &= ~mask;
	reg &= 0xff;

	/* switch off the register */
	switch (reg)
	{
		case PIO_PAR:
			cpustate->par = (cpustate->par & mask) | data;

			/* trigger a load on the upper half */
			if (!(mask & 0xff00))
				dma_load(cpustate);
			break;

		case PIO_PARE:
			cpustate->pare = (cpustate->pare & mask) | data;
			break;

		case PIO_PDR:
			cpustate->pdr = (cpustate->pdr & mask) | data;

			/* trigger a write and PDF setting on the upper half */
			if (!(mask & 0xff00))
			{
				dma_store(cpustate);
				dma_increment(cpustate);
			}
			break;

		case PIO_PDR2:
			cpustate->pdr2 = (cpustate->pdr2 & mask) | data;
			break;

		case PIO_EMR:
			cpustate->emr = (cpustate->emr & mask) | data;
			break;

		case PIO_ESR:
			cpustate->esr = (cpustate->esr & mask) | data;
			break;

		case PIO_PCR:
			mask |= 0x0060;
			data &= ~mask;
			update_pcr(cpustate, (cpustate->pcr & mask) | data);
			break;

		case PIO_PIR:
			cpustate->pir = (cpustate->pir & mask) | data;

			/* set PIF on upper half */
			if (!(mask & 0xff00))
				update_pcr(cpustate, cpustate->pcr | PCR_PIFs);
			break;

		/* error case */
		default:
			logerror("dsp32_pio_w called on invalid register %d\n", reg);
			break;
	}
}



/***************************************************************************
    PARALLEL INTERFACE READS
***************************************************************************/

int dsp32c_pio_r(running_device *device, int reg)
{
	dsp32_state *cpustate = get_safe_token(device);
	UINT16 mask, result = 0xffff;
	UINT8 mode, shift = 0;

	/* look up register and mask */
	mode = ((cpustate->pcr >> 8) & 2) | ((cpustate->pcr >> 1) & 1);
	reg = regmap[mode][reg];
	mask = reg >> 8;
	if (mask == 0x00ff) mask = 0xff00, shift = 8;
	reg &= 0xff;

	/* switch off the register */
	switch (reg)
	{
		case PIO_PAR:
			result = cpustate->par | 1;
			break;

		case PIO_PARE:
			result = cpustate->pare;
			break;

		case PIO_PDR:
			result = cpustate->pdr;

			/* trigger an increment on the lower half */
			if (shift != 8)
				dma_increment(cpustate);

			/* trigger a fetch on the upper half */
			if (!(mask & 0xff00))
				dma_load(cpustate);
			break;

		case PIO_PDR2:
			result = cpustate->pdr2;
			break;

		case PIO_EMR:
			result = cpustate->emr;
			break;

		case PIO_ESR:
			result = cpustate->esr;
			break;

		case PIO_PCR:
			result = cpustate->pcr;
			break;

		case PIO_PIR:
			if (!(mask & 0xff00))
				update_pcr(cpustate, cpustate->pcr & ~PCR_PIFs);	/* clear PIFs */
			result = cpustate->pir;
			break;

		/* error case */
		default:
			logerror("dsp32_pio_w called on invalid register %d\n", reg);
			break;
	}

	return (result >> shift) & ~mask;
}



/**************************************************************************
 * Generic set_info
 **************************************************************************/

static CPU_SET_INFO( dsp32c )
{
	dsp32_state *cpustate = get_safe_token(device);
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + DSP32_IRQ0:	set_irq_line(cpustate, DSP32_IRQ0, info->i);			break;
		case CPUINFO_INT_INPUT_STATE + DSP32_IRQ1:	set_irq_line(cpustate, DSP32_IRQ1, info->i);			break;

		/* CAU */
		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + DSP32_PC:		cpustate->PC = info->i & 0xffffff;				break;
		case CPUINFO_INT_REGISTER + DSP32_R0:		cpustate->R0 = info->i & 0xffffff;				break;
		case CPUINFO_INT_REGISTER + DSP32_R1:		cpustate->R1 = info->i & 0xffffff;				break;
		case CPUINFO_INT_REGISTER + DSP32_R2:		cpustate->R2 = info->i & 0xffffff;				break;
		case CPUINFO_INT_REGISTER + DSP32_R3:		cpustate->R3 = info->i & 0xffffff;				break;
		case CPUINFO_INT_REGISTER + DSP32_R4:		cpustate->R4 = info->i & 0xffffff;				break;
		case CPUINFO_INT_REGISTER + DSP32_R5:		cpustate->R5 = info->i & 0xffffff;				break;
		case CPUINFO_INT_REGISTER + DSP32_R6:		cpustate->R6 = info->i & 0xffffff;				break;
		case CPUINFO_INT_REGISTER + DSP32_R7:		cpustate->R7 = info->i & 0xffffff;				break;
		case CPUINFO_INT_REGISTER + DSP32_R8:		cpustate->R8 = info->i & 0xffffff;				break;
		case CPUINFO_INT_REGISTER + DSP32_R9:		cpustate->R9 = info->i & 0xffffff;				break;
		case CPUINFO_INT_REGISTER + DSP32_R10:		cpustate->R10 = info->i & 0xffffff;				break;
		case CPUINFO_INT_REGISTER + DSP32_R11:		cpustate->R11 = info->i & 0xffffff;				break;
		case CPUINFO_INT_REGISTER + DSP32_R12:		cpustate->R12 = info->i & 0xffffff;				break;
		case CPUINFO_INT_REGISTER + DSP32_R13:		cpustate->R13 = info->i & 0xffffff;				break;
		case CPUINFO_INT_REGISTER + DSP32_R14:		cpustate->R14 = info->i & 0xffffff;				break;
		case CPUINFO_INT_REGISTER + DSP32_R15:		cpustate->R15 = info->i & 0xffffff;				break;
		case CPUINFO_INT_REGISTER + DSP32_R16:		cpustate->R16 = info->i & 0xffffff;				break;
		case CPUINFO_INT_REGISTER + DSP32_R17:		cpustate->R17 = info->i & 0xffffff;				break;
		case CPUINFO_INT_REGISTER + DSP32_R18:		cpustate->R18 = info->i & 0xffffff;				break;
		case CPUINFO_INT_REGISTER + DSP32_R19:		cpustate->R19 = info->i & 0xffffff;				break;
		case CPUINFO_INT_REGISTER + DSP32_R20:		cpustate->R20 = info->i & 0xffffff;				break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + DSP32_R21:		cpustate->R21 = info->i & 0xffffff;				break;
		case CPUINFO_INT_REGISTER + DSP32_R22:		cpustate->R22 = info->i & 0xffffff;				break;
		case CPUINFO_INT_REGISTER + DSP32_PIN:		cpustate->pin = info->i & 0xffffff;				break;
		case CPUINFO_INT_REGISTER + DSP32_POUT:		cpustate->pout = info->i & 0xffffff;			break;
		case CPUINFO_INT_REGISTER + DSP32_IVTP:		cpustate->ivtp = info->i & 0xffffff;			break;

		/* DAU */
		case CPUINFO_INT_REGISTER + DSP32_A0:		cpustate->A0 = info->i;	/* fix me -- very wrong */ break;
		case CPUINFO_INT_REGISTER + DSP32_A1:		cpustate->A1 = info->i;	/* fix me -- very wrong */ break;
		case CPUINFO_INT_REGISTER + DSP32_A2:		cpustate->A2 = info->i;	/* fix me -- very wrong */ break;
		case CPUINFO_INT_REGISTER + DSP32_A3:		cpustate->A3 = info->i;	/* fix me -- very wrong */ break;
		case CPUINFO_INT_REGISTER + DSP32_DAUC:		cpustate->DAUC = info->i;						break;

		/* PIO */
		case CPUINFO_INT_REGISTER + DSP32_PAR:		cpustate->par = info->i;						break;
		case CPUINFO_INT_REGISTER + DSP32_PDR:		cpustate->pdr = info->i;						break;
		case CPUINFO_INT_REGISTER + DSP32_PIR:		cpustate->pir = info->i;						break;
		case CPUINFO_INT_REGISTER + DSP32_PCR:		update_pcr(cpustate, info->i & 0x3ff);			break;
		case CPUINFO_INT_REGISTER + DSP32_EMR:		cpustate->emr = info->i;						break;
		case CPUINFO_INT_REGISTER + DSP32_ESR:		cpustate->esr = info->i;						break;
		case CPUINFO_INT_REGISTER + DSP32_PCW:		cpustate->pcw = info->i;						break;
		case CPUINFO_INT_REGISTER + DSP32_PIOP:		cpustate->piop = info->i;						break;

		/* SIO */
		case CPUINFO_INT_REGISTER + DSP32_IBUF:		cpustate->ibuf = info->i;						break;
		case CPUINFO_INT_REGISTER + DSP32_ISR:		cpustate->isr = info->i;						break;
		case CPUINFO_INT_REGISTER + DSP32_OBUF:		cpustate->obuf = info->i;						break;
		case CPUINFO_INT_REGISTER + DSP32_OSR:		cpustate->osr = info->i;						break;
		case CPUINFO_INT_REGISTER + DSP32_IOC:		cpustate->IOC = info->i & 0xfffff;				break;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

CPU_GET_INFO( dsp32c )
{
	dsp32_state *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(dsp32_state);			break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 2;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case DEVINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;			break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 4;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 4;							break;

		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 32;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 24;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;					break;
		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO:		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + DSP32_IRQ0:		info->i = 0;							break;
		case CPUINFO_INT_INPUT_STATE + DSP32_IRQ1:		info->i = 0;							break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = cpustate->ppc;					break;

		/* CAU */
		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + DSP32_PC:			info->i = cpustate->PC;						break;
		case CPUINFO_INT_REGISTER + DSP32_R0:			info->i = cpustate->R0;						break;
		case CPUINFO_INT_REGISTER + DSP32_R1:			info->i = cpustate->R1;						break;
		case CPUINFO_INT_REGISTER + DSP32_R2:			info->i = cpustate->R2;						break;
		case CPUINFO_INT_REGISTER + DSP32_R3:			info->i = cpustate->R3;						break;
		case CPUINFO_INT_REGISTER + DSP32_R4:			info->i = cpustate->R4;						break;
		case CPUINFO_INT_REGISTER + DSP32_R5:			info->i = cpustate->R5;						break;
		case CPUINFO_INT_REGISTER + DSP32_R6:			info->i = cpustate->R6;						break;
		case CPUINFO_INT_REGISTER + DSP32_R7:			info->i = cpustate->R7;						break;
		case CPUINFO_INT_REGISTER + DSP32_R8:			info->i = cpustate->R8;						break;
		case CPUINFO_INT_REGISTER + DSP32_R9:			info->i = cpustate->R9;						break;
		case CPUINFO_INT_REGISTER + DSP32_R10:			info->i = cpustate->R10;					break;
		case CPUINFO_INT_REGISTER + DSP32_R11:			info->i = cpustate->R11;					break;
		case CPUINFO_INT_REGISTER + DSP32_R12:			info->i = cpustate->R12;					break;
		case CPUINFO_INT_REGISTER + DSP32_R13:			info->i = cpustate->R13;					break;
		case CPUINFO_INT_REGISTER + DSP32_R14:			info->i = cpustate->R14;					break;
		case CPUINFO_INT_REGISTER + DSP32_R15:			info->i = cpustate->R15;					break;
		case CPUINFO_INT_REGISTER + DSP32_R16:			info->i = cpustate->R16;					break;
		case CPUINFO_INT_REGISTER + DSP32_R17:			info->i = cpustate->R17;					break;
		case CPUINFO_INT_REGISTER + DSP32_R18:			info->i = cpustate->R18;					break;
		case CPUINFO_INT_REGISTER + DSP32_R19:			info->i = cpustate->R19;					break;
		case CPUINFO_INT_REGISTER + DSP32_R20:			info->i = cpustate->R20;					break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + DSP32_R21:			info->i = cpustate->R21;					break;
		case CPUINFO_INT_REGISTER + DSP32_R22:			info->i = cpustate->R22;					break;
		case CPUINFO_INT_REGISTER + DSP32_PIN:			info->i = cpustate->pin;					break;
		case CPUINFO_INT_REGISTER + DSP32_POUT:			info->i = cpustate->pout;					break;
		case CPUINFO_INT_REGISTER + DSP32_IVTP:			info->i = cpustate->ivtp;					break;

		/* DAU */
		case CPUINFO_INT_REGISTER + DSP32_A0:			info->i = cpustate->A0;	/* fix me -- very wrong */ break;
		case CPUINFO_INT_REGISTER + DSP32_A1:			info->i = cpustate->A1;	/* fix me -- very wrong */ break;
		case CPUINFO_INT_REGISTER + DSP32_A2:			info->i = cpustate->A2;	/* fix me -- very wrong */ break;
		case CPUINFO_INT_REGISTER + DSP32_A3:			info->i = cpustate->A3;	/* fix me -- very wrong */ break;
		case CPUINFO_INT_REGISTER + DSP32_DAUC:			info->i = cpustate->DAUC;					break;

		/* PIO */
		case CPUINFO_INT_REGISTER + DSP32_PAR:			info->i = cpustate->par;					break;
		case CPUINFO_INT_REGISTER + DSP32_PDR:			info->i = cpustate->pdr;					break;
		case CPUINFO_INT_REGISTER + DSP32_PIR:			info->i = cpustate->pir;					break;
		case CPUINFO_INT_REGISTER + DSP32_PCR:			info->i = cpustate->pcr;					break;
		case CPUINFO_INT_REGISTER + DSP32_EMR:			info->i = cpustate->emr;					break;
		case CPUINFO_INT_REGISTER + DSP32_ESR:			info->i = cpustate->esr;					break;
		case CPUINFO_INT_REGISTER + DSP32_PCW:			info->i = cpustate->pcw;					break;
		case CPUINFO_INT_REGISTER + DSP32_PIOP:			info->i = cpustate->piop;					break;

		/* SIO */
		case CPUINFO_INT_REGISTER + DSP32_IBUF:			info->i = cpustate->ibuf;					break;
		case CPUINFO_INT_REGISTER + DSP32_ISR:			info->i = cpustate->isr;					break;
		case CPUINFO_INT_REGISTER + DSP32_OBUF:			info->i = cpustate->obuf;					break;
		case CPUINFO_INT_REGISTER + DSP32_OSR:			info->i = cpustate->osr;					break;
		case CPUINFO_INT_REGISTER + DSP32_IOC:			info->i = cpustate->IOC;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(dsp32c);		break;
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(dsp32c);				break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(dsp32c);				break;
		case CPUINFO_FCT_EXIT:							info->exit = CPU_EXIT_NAME(dsp32c);				break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(dsp32c);			break;
		case CPUINFO_FCT_BURN:							info->burn = NULL;						break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(dsp32c);		break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cpustate->icount;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "DSP32C");				break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Lucent DSP32");		break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");					break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);				break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Aaron Giles");			break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%c%c%c%c%c%c%c%c",
				NFLAG ? 'N':'.',
				ZFLAG ? 'Z':'.',
                UFLAG ? 'U':'.',
                VFLAG ? 'V':'.',
                nFLAG ? 'n':'.',
                zFLAG ? 'z':'.',
                cFLAG ? 'c':'.',
                vFLAG ? 'v':'.');
            break;

		/* CAU */
		case CPUINFO_STR_REGISTER + DSP32_PC:			sprintf(info->s, "PC: %06X", cpustate->PC); break;
		case CPUINFO_STR_REGISTER + DSP32_R0:			sprintf(info->s, "R0: %06X", cpustate->R0); break;
		case CPUINFO_STR_REGISTER + DSP32_R1:			sprintf(info->s, "R1: %06X", cpustate->R1); break;
		case CPUINFO_STR_REGISTER + DSP32_R2:			sprintf(info->s, "R2: %06X", cpustate->R2); break;
		case CPUINFO_STR_REGISTER + DSP32_R3:			sprintf(info->s, "R3: %06X", cpustate->R3); break;
		case CPUINFO_STR_REGISTER + DSP32_R4:			sprintf(info->s, "R4: %06X", cpustate->R4); break;
		case CPUINFO_STR_REGISTER + DSP32_R5:			sprintf(info->s, "R5: %06X", cpustate->R5); break;
		case CPUINFO_STR_REGISTER + DSP32_R6:			sprintf(info->s, "R6: %06X", cpustate->R6); break;
		case CPUINFO_STR_REGISTER + DSP32_R7:			sprintf(info->s, "R7: %06X", cpustate->R7); break;
		case CPUINFO_STR_REGISTER + DSP32_R8:			sprintf(info->s, "R8: %06X", cpustate->R8); break;
		case CPUINFO_STR_REGISTER + DSP32_R9:			sprintf(info->s, "R9: %06X", cpustate->R9); break;
		case CPUINFO_STR_REGISTER + DSP32_R10:			sprintf(info->s, "R10:%06X", cpustate->R10); break;
		case CPUINFO_STR_REGISTER + DSP32_R11:			sprintf(info->s, "R11:%06X", cpustate->R11); break;
		case CPUINFO_STR_REGISTER + DSP32_R12:			sprintf(info->s, "R12:%06X", cpustate->R12); break;
		case CPUINFO_STR_REGISTER + DSP32_R13:			sprintf(info->s, "R13:%06X", cpustate->R13); break;
		case CPUINFO_STR_REGISTER + DSP32_R14:			sprintf(info->s, "R14:%06X", cpustate->R14); break;
		case CPUINFO_STR_REGISTER + DSP32_R15:			sprintf(info->s, "R15:%06X", cpustate->R15); break;
		case CPUINFO_STR_REGISTER + DSP32_R16:			sprintf(info->s, "R16:%06X", cpustate->R16); break;
		case CPUINFO_STR_REGISTER + DSP32_R17:			sprintf(info->s, "R17:%06X", cpustate->R17); break;
		case CPUINFO_STR_REGISTER + DSP32_R18:			sprintf(info->s, "R18:%06X", cpustate->R18); break;
		case CPUINFO_STR_REGISTER + DSP32_R19:			sprintf(info->s, "R19:%06X", cpustate->R19); break;
		case CPUINFO_STR_REGISTER + DSP32_R20:			sprintf(info->s, "R20:%06X", cpustate->R20); break;
		case CPUINFO_STR_REGISTER + DSP32_R21:			sprintf(info->s, "R21:%06X", cpustate->R21); break;
		case CPUINFO_STR_REGISTER + DSP32_R22:			sprintf(info->s, "R22:%06X", cpustate->R22); break;
		case CPUINFO_STR_REGISTER + DSP32_PIN:			sprintf(info->s, "PIN:%06X", cpustate->pin); break;
		case CPUINFO_STR_REGISTER + DSP32_POUT:			sprintf(info->s, "POUT:%06X", cpustate->pout); break;
		case CPUINFO_STR_REGISTER + DSP32_IVTP:			sprintf(info->s, "IVTP:%06X", cpustate->ivtp); break;

		/* DAU */
		case CPUINFO_STR_REGISTER + DSP32_A0:			sprintf(info->s, "A0:%8g", cpustate->A0); break;
		case CPUINFO_STR_REGISTER + DSP32_A1:			sprintf(info->s, "A1:%8g", cpustate->A1); break;
		case CPUINFO_STR_REGISTER + DSP32_A2:			sprintf(info->s, "A2:%8g", cpustate->A2); break;
		case CPUINFO_STR_REGISTER + DSP32_A3:			sprintf(info->s, "A3:%8g", cpustate->A3); break;
		case CPUINFO_STR_REGISTER + DSP32_DAUC:			sprintf(info->s, "DAUC:%02X", cpustate->DAUC); break;

		/* PIO */
		case CPUINFO_STR_REGISTER + DSP32_PAR:			sprintf(info->s, "PAR:%08X", cpustate->par); break;
		case CPUINFO_STR_REGISTER + DSP32_PDR:			sprintf(info->s, "PDR:%08X", cpustate->pdr); break;
		case CPUINFO_STR_REGISTER + DSP32_PIR:			sprintf(info->s, "PIR:%04X", cpustate->pir); break;
		case CPUINFO_STR_REGISTER + DSP32_PCR:			sprintf(info->s, "PCR:%03X", cpustate->pcr); break;
		case CPUINFO_STR_REGISTER + DSP32_EMR:			sprintf(info->s, "EMR:%04X", cpustate->emr); break;
		case CPUINFO_STR_REGISTER + DSP32_ESR:			sprintf(info->s, "ESR:%02X", cpustate->esr); break;
		case CPUINFO_STR_REGISTER + DSP32_PCW:			sprintf(info->s, "PCW:%04X", cpustate->pcw); break;
		case CPUINFO_STR_REGISTER + DSP32_PIOP:			sprintf(info->s, "PIOP:%02X", cpustate->piop); break;

		/* SIO */
		case CPUINFO_STR_REGISTER + DSP32_IBUF:			sprintf(info->s, "IBUF:%08X", cpustate->ibuf); break;
		case CPUINFO_STR_REGISTER + DSP32_ISR:			sprintf(info->s, "ISR:%08X", cpustate->isr); break;
		case CPUINFO_STR_REGISTER + DSP32_OBUF:			sprintf(info->s, "OBUF:%08X", cpustate->obuf); break;
		case CPUINFO_STR_REGISTER + DSP32_OSR:			sprintf(info->s, "OSR:%08X", cpustate->osr); break;
		case CPUINFO_STR_REGISTER + DSP32_IOC:			sprintf(info->s, "IOC:%05X", cpustate->IOC); break;
	}
}

DEFINE_LEGACY_CPU_DEVICE(DSP32C, dsp32c);
