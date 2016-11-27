/**********************************************************************

    8259 PIC interface and emulation

    The 8259 is a programmable interrupt controller used to multiplex
    interrupts for x86 and other computers.  The chip is set up by
    writing a series of Initialization Command Words (ICWs) after which
    the chip is operational and capable of dispatching interrupts.  After
    this, Operation Command Words (OCWs) can be written to control further
    behavior.

**********************************************************************/

#include "emu.h"
#include "machine/pic8259.h"

#define IRQ_COUNT	8

#define LOG_ICW		0
#define LOG_OCW		0
#define LOG_GENERAL	0

typedef enum
{
	STATE_ICW1,
	STATE_ICW2,
	STATE_ICW3,
	STATE_ICW4,
	STATE_READY
} pic8259_state_t;

typedef struct pic8259	pic8259_t;

struct pic8259
{
	devcb_resolved_write_line out_int_func;

	emu_timer *timer;

	pic8259_state_t state;

	UINT8 irq_lines;
	UINT8 esr;
	UINT8 isr;
	UINT8 irr;
	UINT8 prio;
	UINT8 imr;

	UINT8 input;
	UINT8 ocw3;

	/* ICW1 state */
	UINT32 level_trig_mode : 1;
	UINT32 vector_size : 1;
	UINT32 cascade : 1;
	UINT32 icw4_needed : 1;
	UINT32 vector_addr_low;
	/* ICW2 state */
	UINT8 base;
	UINT8 vector_addr_high;

	/* ICW3 state */
	UINT8 slave;

	/* ICW4 state */
	UINT32 nested : 1;
	UINT32 mode : 2;
	UINT32 auto_eoi : 1;
	UINT32 is_x86 : 1;
};


INLINE pic8259_t *get_safe_token(running_device *device) {
	assert( device != NULL );
	assert( device->type() == PIC8259 );
	return ( pic8259_t *) downcast<legacy_device_base *>(device)->token();
}


static TIMER_CALLBACK( pic8259_timerproc )
{
	running_device *device = (running_device *)ptr;
	pic8259_t	*pic8259 = get_safe_token(device);
	int irq;
	UINT8 mask;

	/* check the various IRQs */
	for (irq = 0; irq < IRQ_COUNT; irq++)
	{
		mask = 1 << irq;

		/* is this IRQ in service? */
		if (pic8259->isr & mask)
		{
			if (LOG_GENERAL)
				logerror("pic8259_timerproc(): PIC IRQ #%d still in service\n", irq);
			break;
		}

		/* is this IRQ pending and enabled? */
		if ((pic8259->state == STATE_READY) && (pic8259->irr & mask) && !(pic8259->imr & mask))
		{
			if (LOG_GENERAL)
				logerror("pic8259_timerproc(): PIC triggering IRQ #%d\n", irq);
			if (!BIT(pic8259->ocw3, 2))
				devcb_call_write_line(&pic8259->out_int_func, 1);
			return;
		}
	}
	if (!BIT(pic8259->ocw3, 2))
		devcb_call_write_line(&pic8259->out_int_func, 0);
}


INLINE void pic8259_set_timer(pic8259_t *pic8259)
{
	timer_adjust_oneshot(pic8259->timer, attotime_zero, 0);
}


static void pic8259_set_irq_line(running_device *device, int irq, int state)
{
	pic8259_t	*pic8259 = get_safe_token(device);
	UINT8		old_irq_lines = pic8259->irq_lines;

	if (state)
	{
		/* setting IRQ line */
		if (LOG_GENERAL)
			logerror("pic8259_set_irq_line(): PIC set IRQ line #%d\n", irq);

		pic8259->irq_lines |= 1 << irq;

		/* Set ESR bit if we see a 0 -> 1 transition */
		if ( ! ( old_irq_lines & ( 1 << irq ) ) )
			pic8259->esr |= ( 1 << irq );
	}
	else
	{
		/* clearing IRQ line */
		if (LOG_GENERAL)
			logerror("pic8259_set_irq_line(): PIC cleared IRQ line #%d\n", irq);

		pic8259->irq_lines &= ~(1 << irq);
	}

	pic8259->irr = ( pic8259->level_trig_mode ) ? pic8259->esr & pic8259->irq_lines : pic8259->irq_lines;
	pic8259_set_timer(pic8259);
}

WRITE_LINE_DEVICE_HANDLER( pic8259_ir0_w ) { pic8259_set_irq_line(device, 0, state); }
WRITE_LINE_DEVICE_HANDLER( pic8259_ir1_w ) { pic8259_set_irq_line(device, 1, state); }
WRITE_LINE_DEVICE_HANDLER( pic8259_ir2_w ) { pic8259_set_irq_line(device, 2, state); }
WRITE_LINE_DEVICE_HANDLER( pic8259_ir3_w ) { pic8259_set_irq_line(device, 3, state); }
WRITE_LINE_DEVICE_HANDLER( pic8259_ir4_w ) { pic8259_set_irq_line(device, 4, state); }
WRITE_LINE_DEVICE_HANDLER( pic8259_ir5_w ) { pic8259_set_irq_line(device, 5, state); }
WRITE_LINE_DEVICE_HANDLER( pic8259_ir6_w ) { pic8259_set_irq_line(device, 6, state); }
WRITE_LINE_DEVICE_HANDLER( pic8259_ir7_w ) { pic8259_set_irq_line(device, 7, state); }

int pic8259_acknowledge(running_device *device)
{
	pic8259_t	*pic8259 = get_safe_token(device);
	UINT8 mask;
	int irq;

	for (irq = 0; irq < IRQ_COUNT; irq++)
	{
		mask = 1 << irq;

		/* is this IRQ pending and enabled? */
		if ((pic8259->irr & mask) && !(pic8259->imr & mask))
		{
			if (LOG_GENERAL)
				logerror("pic8259_acknowledge(): PIC acknowledge IRQ #%d\n", irq);
			pic8259->irr &= ~mask;
			pic8259->esr &= ~mask;
			if (!pic8259->auto_eoi)
				pic8259->isr |= mask;
			pic8259_set_timer(pic8259);
			if (pic8259->is_x86) {
				/* For x86 mode*/
				return irq + pic8259->base;
			} else {
				/* in case of 8080/85) */
				return 0xcd0000 + (pic8259->vector_addr_high << 8) + pic8259->vector_addr_low + (irq << (3-pic8259->vector_size));
			}
		}
	}
	return 0;
}



READ8_DEVICE_HANDLER( pic8259_r )
{
	pic8259_t	*pic8259 = get_safe_token(device);

	/* NPW 18-May-2003 - Changing 0xFF to 0x00 as per Ruslan */
	UINT8 data = 0x00;

	switch(offset)
	{
		case 0: /* PIC acknowledge IRQ */
			if ( pic8259->ocw3 & 0x04 )
			{
				/* Polling mode */
				if ( pic8259->isr & ~pic8259->imr )
				{
					int irq;

					pic8259_acknowledge( device );

					for ( irq = 0; irq < IRQ_COUNT; irq++ )
					{
						if ( ( 1 << irq ) & pic8259->isr & ~pic8259->imr )
						{
							data = 0x80 | irq;
							break;
						}
					}
				}
			}
			else
			{
				switch ( pic8259->ocw3 & 0x03 )
				{
				case 2:
					data = pic8259->irr;
					break;
				case 3:
					data = pic8259->isr & ~pic8259->imr;
					break;
				default:
					data = 0x00;
					break;
				}
			}
			break;

		case 1: /* PIC mask register */
			data = pic8259->imr;
			break;
	}
	return data;
}



WRITE8_DEVICE_HANDLER( pic8259_w )
{
	pic8259_t	*pic8259 = get_safe_token(device);

	switch(offset)
	{
		case 0:    /* PIC acknowledge IRQ */
			if (data & 0x10)
			{
				/* write ICW1 - this pretty much resets the chip */
				if (LOG_ICW)
					logerror("pic8259_w(): ICW1; data=0x%02X\n", data);

				pic8259->imr				= 0x00;
				pic8259->esr				= 0x00;
				pic8259->isr				= 0x00;
				pic8259->irr				= 0x00;
				pic8259->level_trig_mode	= (data & 0x08) ? 1 : 0;
				pic8259->vector_size		= (data & 0x04) ? 1 : 0;
				pic8259->cascade			= (data & 0x02) ? 0 : 1;
				pic8259->icw4_needed		= (data & 0x01) ? 1 : 0;
				pic8259->vector_addr_low	= (data & 0xe0);
				pic8259->state			= STATE_ICW2;
			}
			else if (pic8259->state == STATE_READY)
			{
				if ((data & 0x98) == 0x08)
				{
					/* write OCW3 */
					if (LOG_OCW)
						logerror("pic8259_w(): OCW3; data=0x%02X\n", data);

					pic8259->ocw3 = data;
				}
				else if ((data & 0x18) == 0x00)
				{
					int n = data & 7;
					UINT8 mask = 1 << n;

					/* write OCW2 */
					if (LOG_OCW)
						logerror("pic8259_w(): OCW2; data=0x%02X\n", data);

					switch (data & 0xe0)
					{
						case 0x00:
							pic8259->prio = 0;
							break;
						case 0x20:
							for (n = 0, mask = 1<<pic8259->prio; n < 8; n++, mask = (mask<<1) | (mask>>7))
							{
								if (pic8259->isr & mask)
								{
									pic8259->isr &= ~mask;
									break;
								}
							}
							break;
						case 0x40:
							break;
						case 0x60:
							if( pic8259->isr & mask )
							{
								pic8259->isr &= ~mask;
							}
							break;
						case 0x80:
							pic8259->prio = (pic8259->prio + 1) & 7;
							break;
						case 0xa0:
							for (n = 0, mask = 1<<pic8259->prio; n < 8; n++, mask = (mask<<1) | (mask>>7))
							{
								if( pic8259->isr & mask )
								{
									pic8259->isr &= ~mask;
									pic8259->prio = (pic8259->prio + 1) & 7;
									break;
								}
							}
							break;
						case 0xc0:
							pic8259->prio = n & 7;
							break;
						case 0xe0:
							if( pic8259->isr & mask )
							{
								pic8259->isr &= ~mask;
								pic8259->irr &= ~mask;
								pic8259->prio = (pic8259->prio + 1) & 7;
							}
							break;
					}
				}
			}
			break;

		case 1:
			switch(pic8259->state)
			{
				case STATE_ICW1:
					break;

				case STATE_ICW2:
					/* write ICW2 */
					if (LOG_ICW)
						logerror("pic8259_w(): ICW2; data=0x%02X\n", data);

					pic8259->base = data & 0xf8;
					pic8259->vector_addr_high = data ;
					if (pic8259->cascade)
						pic8259->state = STATE_ICW3;
					else
						pic8259->state = pic8259->icw4_needed ? STATE_ICW4 : STATE_READY;
					break;

				case STATE_ICW3:
					/* write ICW3 */
					if (LOG_ICW)
						logerror("pic8259_w(): ICW3; data=0x%02X\n", data);

					pic8259->slave = data;
					pic8259->state = pic8259->icw4_needed ? STATE_ICW4 : STATE_READY;
					break;

				case STATE_ICW4:
					/* write ICW4 */
					if (LOG_ICW)
						logerror("pic8259_w(): ICW4; data=0x%02X\n", data);

					pic8259->nested	= (data & 0x10) ? 1 : 0;
					pic8259->mode = (data >> 2) & 3;
					pic8259->auto_eoi = (data & 0x02) ? 1 : 0;
					pic8259->is_x86 = (data & 0x01) ? 1 : 0;
					pic8259->state = STATE_READY;
					break;

				case STATE_READY:
					/* write OCW1 - set interrupt mask register */
					if (LOG_OCW)
						logerror("pic8259_w(): OCW1; data=0x%02X\n", data);

					pic8259->imr = data;
					break;
			}
			break;
    }
	pic8259_set_timer(pic8259);
}



static DEVICE_START( pic8259 )
{
	pic8259_t	*pic8259 = get_safe_token(device);
	const struct pic8259_interface *intf = (const struct pic8259_interface *)device->baseconfig().static_config();

	assert(intf != NULL);

	pic8259->timer = timer_alloc( device->machine, pic8259_timerproc, (void *)device );

	/* resolve callbacks */
	devcb_resolve_write_line(&pic8259->out_int_func, &intf->out_int_func, device);
}


static DEVICE_RESET( pic8259 ) {
	pic8259_t	*pic8259 = get_safe_token(device);

	pic8259->state = STATE_READY;
	pic8259->irq_lines = 0;
	pic8259->isr = 0;
	pic8259->esr = 0;
	pic8259->irr = 0;
	pic8259->prio = 0;
	pic8259->imr = 0;
	pic8259->input = 0;
	pic8259->ocw3 = 2;
	pic8259->level_trig_mode = 0;
	pic8259->vector_size = 0;
	pic8259->cascade = 0;
	pic8259->icw4_needed = 0;
	pic8259->base = 0;
	pic8259->slave = 0;
	pic8259->nested = 0;
	pic8259->mode = 0;
	pic8259->auto_eoi = 0;
	pic8259->is_x86 = 0;
	pic8259->vector_addr_low = 0;
	pic8259->vector_addr_high = 0;
}


DEVICE_GET_INFO( pic8259 ) {
	switch ( state ) {
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:				info->i = sizeof(pic8259_t);				break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:		info->i = 0;								break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:						info->start = DEVICE_START_NAME(pic8259);	break;
		case DEVINFO_FCT_STOP:						/* nothing */								break;
		case DEVINFO_FCT_RESET:						info->reset = DEVICE_RESET_NAME(pic8259);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:						strcpy(info->s, "Intel PIC8259");			break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "PIC8259");					break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.00");					break;
		case DEVINFO_STR_SOURCE_FILE:				strcpy(info->s, __FILE__);					break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright the MAME and MESS Teams");	break;
	}
}


DEFINE_LEGACY_DEVICE(PIC8259, pic8259);
