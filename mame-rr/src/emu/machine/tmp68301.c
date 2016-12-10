/***************************************************************************

    TMP68301 basic emulation + Interrupt Handling

    The Toshiba TMP68301 is a 68HC000 + serial I/O, parallel I/O,
    3 timers, address decoder, wait generator, interrupt controller,
    all integrated in a single chip.

***************************************************************************/

#include "emu.h"
#include "machine/tmp68301.h"

UINT16 *tmp68301_regs;

static UINT8 tmp68301_IE[3];		// 3 External Interrupt Lines
static emu_timer *tmp68301_timer[3];		// 3 Timers

static int tmp68301_irq_vector[8];

static void tmp68301_update_timer( running_machine *machine, int i );

static IRQ_CALLBACK(tmp68301_irq_callback)
{
	int vector = tmp68301_irq_vector[irqline];
//  logerror("%s: irq callback returns %04X for level %x\n",cpuexec_describe_context(machine),vector,int_level);
	return vector;
}

static TIMER_CALLBACK( tmp68301_timer_callback )
{
	int i = param;
	UINT16 TCR	=	tmp68301_regs[(0x200 + i * 0x20)/2];
	UINT16 IMR	=	tmp68301_regs[0x94/2];		// Interrupt Mask Register (IMR)
	UINT16 ICR	=	tmp68301_regs[0x8e/2+i];	// Interrupt Controller Register (ICR7..9)
	UINT16 IVNR	=	tmp68301_regs[0x9a/2];		// Interrupt Vector Number Register (IVNR)

//  logerror("s: callback timer %04X, j = %d\n",cpuexec_describe_context(machine),i,tcount);

	if	(	(TCR & 0x0004) &&	// INT
			!(IMR & (0x100<<i))
		)
	{
		int level = ICR & 0x0007;

		// Interrupt Vector Number Register (IVNR)
		tmp68301_irq_vector[level]	=	IVNR & 0x00e0;
		tmp68301_irq_vector[level]	+=	4+i;

		cpu_set_input_line(machine->firstcpu,level,HOLD_LINE);
	}

	if (TCR & 0x0080)	// N/1
	{
		// Repeat
		tmp68301_update_timer(machine, i);
	}
	else
	{
		// One Shot
	}
}

static void tmp68301_update_timer( running_machine *machine, int i )
{
	UINT16 TCR	=	tmp68301_regs[(0x200 + i * 0x20)/2];
	UINT16 MAX1	=	tmp68301_regs[(0x204 + i * 0x20)/2];
	UINT16 MAX2	=	tmp68301_regs[(0x206 + i * 0x20)/2];

	int max = 0;
	attotime duration = attotime_zero;

	timer_adjust_oneshot(tmp68301_timer[i],attotime_never,i);

	// timers 1&2 only
	switch( (TCR & 0x0030)>>4 )						// MR2..1
	{
	case 1:
		max = MAX1;
		break;
	case 2:
		max = MAX2;
		break;
	}

	switch ( (TCR & 0xc000)>>14 )					// CK2..1
	{
	case 0:	// System clock (CLK)
		if (max)
		{
			int scale = (TCR & 0x3c00)>>10;			// P4..1
			if (scale > 8) scale = 8;
			duration = attotime_mul(ATTOTIME_IN_HZ(machine->firstcpu->unscaled_clock()), (1 << scale) * max);
		}
		break;
	}

//  logerror("%s: TMP68301 Timer %d, duration %lf, max %04X\n",cpuexec_describe_context(machine),i,duration,max);

	if (!(TCR & 0x0002))				// CS
	{
		if (attotime_compare(duration, attotime_zero))
			timer_adjust_oneshot(tmp68301_timer[i],duration,i);
		else
			logerror("%s: TMP68301 error, timer %d duration is 0\n",cpuexec_describe_context(machine),i);
	}
}

MACHINE_START( tmp68301 )
{
	int i;
	for (i = 0; i < 3; i++)
		tmp68301_timer[i] = timer_alloc(machine, tmp68301_timer_callback, NULL);
}

MACHINE_RESET( tmp68301 )
{
	int i;

	for (i = 0; i < 3; i++)
		tmp68301_IE[i] = 0;

	cpu_set_irq_callback(machine->firstcpu, tmp68301_irq_callback);
}

/* Update the IRQ state based on all possible causes */
static void update_irq_state(running_machine *machine)
{
	int i;

	/* Take care of external interrupts */

	UINT16 IMR	=	tmp68301_regs[0x94/2];		// Interrupt Mask Register (IMR)
	UINT16 IVNR	=	tmp68301_regs[0x9a/2];		// Interrupt Vector Number Register (IVNR)

	for (i = 0; i < 3; i++)
	{
		if	(	(tmp68301_IE[i]) &&
				!(IMR & (1<<i))
			)
		{
			UINT16 ICR	=	tmp68301_regs[0x80/2+i];	// Interrupt Controller Register (ICR0..2)

			// Interrupt Controller Register (ICR0..2)
			int level = ICR & 0x0007;

			// Interrupt Vector Number Register (IVNR)
			tmp68301_irq_vector[level]	=	IVNR & 0x00e0;
			tmp68301_irq_vector[level]	+=	i;

			tmp68301_IE[i] = 0;		// Interrupts are edge triggerred

			cpu_set_input_line(machine->firstcpu,level,HOLD_LINE);
		}
	}
}

WRITE16_HANDLER( tmp68301_regs_w )
{
	COMBINE_DATA(&tmp68301_regs[offset]);

	if (!ACCESSING_BITS_0_7)	return;

//  logerror("CPU #0 PC %06X: TMP68301 Reg %04X<-%04X & %04X\n",cpu_get_pc(space->cpu),offset*2,data,mem_mask^0xffff);

	switch( offset * 2 )
	{
		// Timers
		case 0x200:
		case 0x220:
		case 0x240:
		{
			int i = ((offset*2) >> 5) & 3;

			tmp68301_update_timer( space->machine, i );
		}
		break;
	}
}

void tmp68301_external_interrupt_0(running_machine *machine)	{ tmp68301_IE[0] = 1;	update_irq_state(machine); }
void tmp68301_external_interrupt_1(running_machine *machine)	{ tmp68301_IE[1] = 1;	update_irq_state(machine); }
void tmp68301_external_interrupt_2(running_machine *machine)	{ tmp68301_IE[2] = 1;	update_irq_state(machine); }

