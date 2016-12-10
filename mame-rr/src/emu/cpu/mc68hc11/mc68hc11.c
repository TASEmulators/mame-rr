/*
   Motorola MC68HC11 emulator

   Written by Ville Linde & Angelo Salese

TODO:
- Interrupts handling is really bare-bones, just to make Hit Poker happy;
- Complete opcodes hook-up;
- Emulate the MC68HC12 (same as HC11 with a bunch of new opcodes);

 */

#include "emu.h"
#include "debugger.h"
#include "mc68hc11.h"

enum
{
	HC11_PC = 1,
	HC11_SP,
	HC11_A,
	HC11_B,
	HC11_IX,
	HC11_IY
};

#define CC_S	0x80
#define CC_X	0x40
#define CC_H	0x20
#define CC_I	0x10
#define CC_N	0x08
#define CC_Z	0x04
#define CC_V	0x02
#define CC_C	0x01

typedef struct _hc11_state hc11_state;
struct _hc11_state
{
	union {
		struct {
#ifdef LSB_FIRST
			UINT8 b;
			UINT8 a;
#else
			UINT8 a;
			UINT8 b;
#endif
		} d8;
		UINT16 d16;
	} d;

	UINT16 ix;
	UINT16 iy;
	UINT16 sp;
	UINT16 pc;
	UINT16 ppc;
	UINT8 ccr;

	UINT8 adctl;
	int ad_channel;

	device_irq_callback irq_callback;
	UINT8 irq_state[2];
	legacy_cpu_device *device;
	const address_space *program;
	const address_space *io;
	int icount;

	int ram_position;
	int reg_position;
	UINT8 *internal_ram;

	int has_extended_io; // extended I/O enable flag
	int internal_ram_size;

	UINT8 wait_state,stop_state;

	UINT8 tflg1;
};

INLINE hc11_state *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->type() == MC68HC11);
	return (hc11_state *)downcast<legacy_cpu_device *>(device)->token();
}

#define HC11OP(XX)		hc11_##XX

/*****************************************************************************/
/* Internal registers */

static UINT8 hc11_regs_r(hc11_state *cpustate, UINT32 address)
{
	int reg = address & 0xff;

	switch(reg)
	{
		case 0x00:		/* PORTA */
			return memory_read_byte(cpustate->io, MC68HC11_IO_PORTA);
		case 0x01:		/* DDRA */
			return 0;
		case 0x02:		/* PIOC */
			return 0;
		case 0x03:		/* PORTC */
			return memory_read_byte(cpustate->io, MC68HC11_IO_PORTC);
		case 0x04:		/* PORTB */
			return memory_read_byte(cpustate->io, MC68HC11_IO_PORTB);
		case 0x08:		/* PORTD */
			return memory_read_byte(cpustate->io, MC68HC11_IO_PORTD);
		case 0x09:		/* DDRD */
			return 0;
		case 0x0a:		/* PORTE */
			return memory_read_byte(cpustate->io, MC68HC11_IO_PORTE);
		case 0x23:
			return cpustate->tflg1;
		case 0x28:		/* SPCR1 */
			return 0;
		case 0x30:		/* ADCTL */
			return 0x80;
		case 0x31:		/* ADR1 */
		{
			if (cpustate->adctl & 0x10)
			{
				return memory_read_byte(cpustate->io, (cpustate->adctl & 0x4) + MC68HC11_IO_AD0);
			}
			else
			{
				return memory_read_byte(cpustate->io, (cpustate->adctl & 0x7) + MC68HC11_IO_AD0);
			}
		}
		case 0x32:		/* ADR2 */
		{
			if (cpustate->adctl & 0x10)
			{
				return memory_read_byte(cpustate->io, (cpustate->adctl & 0x4) + MC68HC11_IO_AD1);
			}
			else
			{
				return memory_read_byte(cpustate->io, (cpustate->adctl & 0x7) + MC68HC11_IO_AD0);
			}
		}
		case 0x33:		/* ADR3 */
		{
			if (cpustate->adctl & 0x10)
			{
				return memory_read_byte(cpustate->io, (cpustate->adctl & 0x4) + MC68HC11_IO_AD2);
			}
			else
			{
				return memory_read_byte(cpustate->io, (cpustate->adctl & 0x7) + MC68HC11_IO_AD0);
			}
		}
		case 0x34:		/* ADR4 */
		{
			if (cpustate->adctl & 0x10)
			{
				return memory_read_byte(cpustate->io, (cpustate->adctl & 0x4) + MC68HC11_IO_AD3);
			}
			else
			{
				return memory_read_byte(cpustate->io, (cpustate->adctl & 0x7) + MC68HC11_IO_AD0);
			}
		}
		case 0x38:		/* OPT2 */
			return 0;
		case 0x70:		/* SCBDH */
			return 0;
		case 0x71:		/* SCBDL */
			return 0;
		case 0x72:		/* SCCR1 */
			return 0;
		case 0x73:		/* SCCR2 */
			return 0;
		case 0x74:		/* SCSR1 */
			return 0x40;
		case 0x7c:		/* PORTH */
			return memory_read_byte(cpustate->io, MC68HC11_IO_PORTH);
		case 0x7e:		/* PORTG */
			return memory_read_byte(cpustate->io, MC68HC11_IO_PORTG);
		case 0x7f:		/* DDRG */
			return 0;

		case 0x88:		/* SPCR2 */
			return 0;
		case 0x89:		/* SPSR2 */
			return 0x80;
		case 0x8a:		/* SPDR2 */
			return memory_read_byte(cpustate->io, MC68HC11_IO_SPI2_DATA);

		case 0x8b:		/* OPT4 */
			return 0;
	}

	logerror("HC11: regs_r %02X\n", reg);
	return 0; // Dummy
}

static void hc11_regs_w(hc11_state *cpustate, UINT32 address, UINT8 value)
{
	int reg = address & 0xff;

	switch(reg)
	{
		case 0x00:		/* PORTA */
			memory_write_byte(cpustate->io, MC68HC11_IO_PORTA, value);
			return;
		case 0x01:		/* DDRA */
			//mame_printf_debug("HC11: ddra = %02X\n", value);
			return;
		case 0x03:		/* PORTC */
			memory_write_byte(cpustate->io, MC68HC11_IO_PORTC, value);
			return;
		case 0x04:		/* PORTC */
			memory_write_byte(cpustate->io, MC68HC11_IO_PORTB, value);
			return;
		case 0x08:		/* PORTD */
			memory_write_byte(cpustate->io, MC68HC11_IO_PORTD, value); //mask & 0x3f?
			return;
		case 0x09:		/* DDRD */
			//mame_printf_debug("HC11: ddrd = %02X\n", value);
			return;
		case 0x0a:		/* PORTE */
			memory_write_byte(cpustate->io, MC68HC11_IO_PORTE, value);
			return;
		case 0x22:		/* TMSK1 */
			return;
		case 0x23:
			cpustate->tflg1 = value;
			return;
		case 0x24:		/* TMSK2 */
			return;
		case 0x28:		/* SPCR1 */
			return;
		case 0x30:		/* ADCTL */
			cpustate->adctl = value;
			return;
		case 0x38:		/* OPT2 */
			return;
		case 0x39:		/* OPTION */
			return;
		case 0x3a:		/* COPRST (watchdog) */
			return;

		case 0x3d:		/* INIT */
		{
			int reg_page = value & 0xf;
			int ram_page = (value >> 4) & 0xf;

			if (reg_page == ram_page) {
				cpustate->reg_position = reg_page << 12;
				cpustate->ram_position = (ram_page << 12) + 0x100;
			} else {
				cpustate->reg_position = reg_page << 12;
				cpustate->ram_position = ram_page << 12;
			}
			return;
		}

		case 0x3f:		/* CONFIG */
			return;

		case 0x70:		/* SCBDH */
			return;
		case 0x71:		/* SCBDL */
			return;
		case 0x72:		/* SCCR1 */
			return;
		case 0x73:		/* SCCR2 */
			return;
		case 0x77:		/* SCDRL */
			return;
		case 0x7c:		/* PORTH */
			memory_write_byte(cpustate->io, MC68HC11_IO_PORTH, value);
			return;
		case 0x7d:		/* DDRH */
			//mame_printf_debug("HC11: ddrh = %02X at %04X\n", value, cpustate->pc);
			return;
		case 0x7e:		/* PORTG */
			memory_write_byte(cpustate->io, MC68HC11_IO_PORTG, value);
			return;
		case 0x7f:		/* DDRG */
			//mame_printf_debug("HC11: ddrg = %02X at %04X\n", value, cpustate->pc);
			return;

		case 0x88:		/* SPCR2 */
			return;
		case 0x89:		/* SPSR2 */
			return;
		case 0x8a:		/* SPDR2 */
			memory_write_byte(cpustate->io, MC68HC11_IO_SPI2_DATA, value);
			return;

		case 0x8b:		/* OPT4 */
			return;

	}

	logerror("HC11: regs_w %02X, %02X\n", reg, value);
}

/*****************************************************************************/

INLINE UINT8 FETCH(hc11_state *cpustate)
{
	return memory_decrypted_read_byte(cpustate->program, cpustate->pc++);
}

INLINE UINT16 FETCH16(hc11_state *cpustate)
{
	UINT16 w;
	w = (memory_decrypted_read_byte(cpustate->program, cpustate->pc) << 8) | (memory_decrypted_read_byte(cpustate->program, cpustate->pc+1));
	cpustate->pc += 2;
	return w;
}

INLINE UINT8 READ8(hc11_state *cpustate, UINT32 address)
{
	if(address >= cpustate->reg_position && address < cpustate->reg_position+(cpustate->has_extended_io ? 0x100 : 0x40))
	{
		return hc11_regs_r(cpustate, address);
	}
	else if(address >= cpustate->ram_position && address < cpustate->ram_position+cpustate->internal_ram_size)
	{
		return cpustate->internal_ram[address-cpustate->ram_position];
	}
	return memory_read_byte(cpustate->program, address);
}

INLINE void WRITE8(hc11_state *cpustate, UINT32 address, UINT8 value)
{
	if(address >= cpustate->reg_position && address < cpustate->reg_position+(cpustate->has_extended_io ? 0x100 : 0x40))
	{
		hc11_regs_w(cpustate, address, value);
		return;
	}
	else if(address >= cpustate->ram_position && address < cpustate->ram_position+cpustate->internal_ram_size)
	{
		cpustate->internal_ram[address-cpustate->ram_position] = value;
		return;
	}
	memory_write_byte(cpustate->program, address, value);
}

INLINE UINT16 READ16(hc11_state *cpustate, UINT32 address)
{
	return (READ8(cpustate, address) << 8) | (READ8(cpustate, address+1));
}

INLINE void WRITE16(hc11_state *cpustate, UINT32 address, UINT16 value)
{
	WRITE8(cpustate, address+0, (value >> 8) & 0xff);
	WRITE8(cpustate, address+1, (value >> 0) & 0xff);
}

/*****************************************************************************/

static void (*hc11_optable[256])(hc11_state *cpustate);
static void (*hc11_optable_page2[256])(hc11_state *cpustate);
static void (*hc11_optable_page3[256])(hc11_state *cpustate);
static void (*hc11_optable_page4[256])(hc11_state *cpustate);

#include "hc11ops.c"
#include "hc11ops.h"

static CPU_INIT( hc11 )
{
	hc11_state *cpustate = get_safe_token(device);
	int i;

	const hc11_config *conf = (const hc11_config *)device->baseconfig().static_config();

	/* clear the opcode tables */
	for(i=0; i < 256; i++) {
		hc11_optable[i] = HC11OP(invalid);
		hc11_optable_page2[i] = HC11OP(invalid);
		hc11_optable_page3[i] = HC11OP(invalid);
		hc11_optable_page4[i] = HC11OP(invalid);
	}
	/* fill the opcode tables */
	for(i=0; i < sizeof(hc11_opcode_list)/sizeof(hc11_opcode_list_struct); i++)
	{
		switch(hc11_opcode_list[i].page)
		{
			case 0x00:
				hc11_optable[hc11_opcode_list[i].opcode] = hc11_opcode_list[i].handler;
				break;
			case 0x18:
				hc11_optable_page2[hc11_opcode_list[i].opcode] = hc11_opcode_list[i].handler;
				break;
			case 0x1A:
				hc11_optable_page3[hc11_opcode_list[i].opcode] = hc11_opcode_list[i].handler;
				break;
			case 0xCD:
				hc11_optable_page4[hc11_opcode_list[i].opcode] = hc11_opcode_list[i].handler;
				break;
		}
	}

	if(conf)
	{
		cpustate->has_extended_io = conf->has_extended_io;
		cpustate->internal_ram_size = conf->internal_ram_size;
	}
	else
	{
		/* defaults it to the HC11M0 version for now (I might strip this down on a later date) */
		cpustate->has_extended_io = 1;
		cpustate->internal_ram_size = 1280;
	}

	cpustate->internal_ram = auto_alloc_array(device->machine, UINT8, cpustate->internal_ram_size);

	cpustate->reg_position = 0;
	cpustate->ram_position = 0x100;
	cpustate->irq_callback = irqcallback;
	cpustate->device = device;
	cpustate->program = device->space(AS_PROGRAM);
	cpustate->io = device->space(AS_IO);
}

static CPU_RESET( hc11 )
{
	hc11_state *cpustate = get_safe_token(device);
	cpustate->pc = READ16(cpustate, 0xfffe);
	cpustate->wait_state = 0;
	cpustate->stop_state = 0;
	cpustate->ccr = CC_X | CC_I | CC_S;
	cpustate->reg_position = 0x1000;
	cpustate->ram_position = 0;
}

static CPU_EXIT( hc11 )
{

}

/*
IRQ table vectors:
0xffd6: SCI
0xffd8: SPI
0xffda: Pulse Accumulator Input Edge
0xffdc: Pulse Accumulator Overflow
0xffde: Timer Overflow
0xffe0: Timer Output Capture 5
0xffe2: Timer Output Capture 4
0xffe4: Timer Output Capture 3
0xffe6: Timer Output Capture 2
0xffe8: Timer Output Capture 1
0xffea: Timer Input Capture 3
0xffec: Timer Input Capture 2
0xffee: Timer Input Capture 1
0xfff0: Real Time Int
0xfff2: IRQ
0xfff4: XIRQ
0xfff6: SWI (Trap IRQ)
0xfff8: Illegal Opcode (NMI)
0xfffa: CO-Processor Fail
0xfffc: Clock Monitor
0xfffe: RESET
*/

static void check_irq_lines(hc11_state *cpustate)
{
	if( cpustate->irq_state[MC68HC11_IRQ_LINE]!=CLEAR_LINE && (!(cpustate->ccr & CC_I)) )
	{
		UINT16 pc_vector;

		if(cpustate->wait_state == 0)
		{
			PUSH16(cpustate, cpustate->pc);
			PUSH16(cpustate, cpustate->iy);
			PUSH16(cpustate, cpustate->ix);
			PUSH8(cpustate, REG_A);
			PUSH8(cpustate, REG_B);
			PUSH8(cpustate, cpustate->ccr);
		}
		pc_vector = READ16(cpustate, 0xfff2);
		SET_PC(cpustate, pc_vector);
		cpustate->ccr |= CC_I; //irq taken, mask the flag
		if(cpustate->wait_state == 1) { cpustate->wait_state = 2; }
		if(cpustate->stop_state == 1) { cpustate->stop_state = 2; }
		(void)(*cpustate->irq_callback)(cpustate->device, MC68HC11_IRQ_LINE);
	}
}

static void set_irq_line(hc11_state *cpustate, int irqline, int state)
{
	cpustate->irq_state[irqline] = state;
	if (state == CLEAR_LINE) return;
	check_irq_lines(cpustate);
}

static CPU_EXECUTE( hc11 )
{
	hc11_state *cpustate = get_safe_token(device);

	while(cpustate->icount > 0)
	{
		UINT8 op;

		check_irq_lines(cpustate);

		cpustate->ppc = cpustate->pc;
		debugger_instruction_hook(device, cpustate->pc);

		op = FETCH(cpustate);
		hc11_optable[op](cpustate);
	}
}

/*****************************************************************************/

static CPU_SET_INFO( mc68hc11 )
{
	hc11_state *cpustate = get_safe_token(device);

	switch (state)
	{
		case CPUINFO_INT_INPUT_STATE + MC68HC11_IRQ_LINE:	set_irq_line(cpustate, MC68HC11_IRQ_LINE, info->i);		break;

		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_PC:							cpustate->pc = info->i;						break;
		case CPUINFO_INT_REGISTER + HC11_PC:			cpustate->pc = info->i; 					break;
		case CPUINFO_INT_REGISTER + HC11_SP:			cpustate->sp = info->i;						break;
		case CPUINFO_INT_REGISTER + HC11_A:				cpustate->d.d8.a = info->i;					break;
		case CPUINFO_INT_REGISTER + HC11_B:				cpustate->d.d8.b = info->i;					break;
		case CPUINFO_INT_REGISTER + HC11_IX:			cpustate->ix = info->i;						break;
		case CPUINFO_INT_REGISTER + HC11_IY:			cpustate->iy = info->i;						break;
	}
}

CPU_GET_INFO( mc68hc11 )
{
	hc11_state *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	switch(state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:						info->i = sizeof(hc11_state);	break;
		case CPUINFO_INT_INPUT_LINES:						info->i = 1;					break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:				info->i = 0;					break;
		case DEVINFO_INT_ENDIANNESS:						info->i = ENDIANNESS_BIG;		break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:					info->i = 1;					break;
		case CPUINFO_INT_CLOCK_DIVIDER:						info->i = 1;					break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:				info->i = 1;					break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:				info->i = 5;					break;
		case CPUINFO_INT_MIN_CYCLES:						info->i = 1;					break;
		case CPUINFO_INT_MAX_CYCLES:						info->i = 41;					break;

		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:				info->i = 8;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: 			info->i = 16;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: 			info->i = 0;					break;
		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:				info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA:				info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA:				info->i = 0;					break;
		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:					info->i = 8;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO:					info->i = 8;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO:					info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + MC68HC11_IRQ_LINE:	info->i = cpustate->irq_state[MC68HC11_IRQ_LINE]; break;

		case CPUINFO_INT_PREVIOUSPC:						/* not implemented */			break;

		case CPUINFO_INT_PC:	/* intentional fallthrough */
		case CPUINFO_INT_REGISTER + HC11_PC:			info->i = cpustate->pc;				break;
		case CPUINFO_INT_REGISTER + HC11_SP:			info->i = cpustate->sp;				break;
		case CPUINFO_INT_REGISTER + HC11_A:				info->i = cpustate->d.d8.a;			break;
		case CPUINFO_INT_REGISTER + HC11_B:				info->i = cpustate->d.d8.b;			break;
		case CPUINFO_INT_REGISTER + HC11_IX:			info->i = cpustate->ix;				break;
		case CPUINFO_INT_REGISTER + HC11_IY:			info->i = cpustate->iy;				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(mc68hc11);		break;
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(hc11);					break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(hc11);				break;
		case CPUINFO_FCT_EXIT:							info->exit = CPU_EXIT_NAME(hc11);					break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(hc11);			break;
		case CPUINFO_FCT_BURN:							info->burn = NULL;						break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(hc11);		break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cpustate->icount;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "MC68HC11");			break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Motorola MC68HC11");	break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");					break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);				break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Ville Linde"); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%c%c%c%c%c%c%c%c",
				(cpustate->ccr & CC_S) ? 'S' : '.',
				(cpustate->ccr & CC_X) ? 'X' : '.',
				(cpustate->ccr & CC_H) ? 'H' : '.',
				(cpustate->ccr & CC_I) ? 'I' : '.',
				(cpustate->ccr & CC_N) ? 'N' : '.',
				(cpustate->ccr & CC_Z) ? 'Z' : '.',
				(cpustate->ccr & CC_V) ? 'V' : '.',
				(cpustate->ccr & CC_C) ? 'C' : '.');
			break;

		case CPUINFO_STR_REGISTER + HC11_PC:			sprintf(info->s, "PC: %04X", cpustate->pc);	break;
		case CPUINFO_STR_REGISTER + HC11_SP:			sprintf(info->s, "SP: %04X", cpustate->sp);	break;
		case CPUINFO_STR_REGISTER + HC11_A:				sprintf(info->s, "A: %02X", cpustate->d.d8.a); break;
		case CPUINFO_STR_REGISTER + HC11_B:				sprintf(info->s, "B: %02X", cpustate->d.d8.b); break;
		case CPUINFO_STR_REGISTER + HC11_IX:			sprintf(info->s, "IX: %04X", cpustate->ix);	break;
		case CPUINFO_STR_REGISTER + HC11_IY:			sprintf(info->s, "IY: %04X", cpustate->iy);	break;
	}
}

DEFINE_LEGACY_CPU_DEVICE(MC68HC11, mc68hc11);
