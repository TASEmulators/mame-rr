/***************************************************************************

    Amiga Computer / Arcadia Game System

    Driver by:

    Aaron Giles, Ernesto Corvi & Mariusz Wojcieszek

***************************************************************************/

#include "emu.h"
#include "includes/amiga.h"
#include "cpu/m68000/m68000.h"
#include "machine/6526cia.h"


/*************************************
 *
 *  Debugging
 *
 *************************************/

#define LOG_CUSTOM	0
#define LOG_CIA		0
#define LOG_BLITS	0



/*************************************
 *
 *  Constants
 *
 *************************************/

/* How many CPU cycles we delay until we fire a pending interrupt */
#define AMIGA_IRQ_DELAY_CYCLES		24

/* How many CPU cycles we wait until we process a blit when the blitter-nasty bit is set */
#define BLITTER_NASTY_DELAY			16



/*************************************
 *
 *  Type definitions
 *
 *************************************/

typedef struct _autoconfig_device autoconfig_device;
struct _autoconfig_device
{
	autoconfig_device *		next;
	amiga_autoconfig_device	device;
	offs_t					base;
};



/*************************************
 *
 *  Globals
 *
 *************************************/

UINT16 *amiga_chip_ram;
UINT32 *amiga_chip_ram32;
size_t amiga_chip_ram_size;

UINT16 *amiga_custom_regs;
UINT16 *amiga_expansion_ram;
UINT16 *amiga_autoconfig_mem;

static const amiga_machine_interface *amiga_intf;

static autoconfig_device *autoconfig_list;
static autoconfig_device *cur_autoconfig;
static emu_timer * amiga_irq_timer;
static emu_timer * amiga_blitter_timer;

const char *const amiga_custom_names[0x100] =
{
	/* 0x000 */
	"BLTDDAT",		"DMACONR",		"VPOSR",		"VHPOSR",
	"DSKDATR",		"JOY0DAT",		"JOY1DAT",		"CLXDAT",
	"ADKCONR",		"POT0DAT",		"POT1DAT",		"POTGOR",
	"SERDATR",		"DSKBYTR",		"INTENAR",		"INTREQR",
	/* 0x020 */
	"DSKPTH",		"DSKPTL",		"DSKLEN",		"DSKDAT",
	"REFPTR",		"VPOSW",		"VHPOSW",		"COPCON",
	"SERDAT",		"SERPER",		"POTGO",		"JOYTEST",
	"STREQU",		"STRVBL",		"STRHOR",		"STRLONG",
	/* 0x040 */
	"BLTCON0",		"BLTCON1",		"BLTAFWM",		"BLTALWM",
	"BLTCPTH",		"BLTCPTL",		"BLTBPTH",		"BLTBPTL",
	"BLTAPTH",		"BLTAPTL",		"BLTDPTH",		"BLTDPTL",
	"BLTSIZE",		"BLTCON0L",		"BLTSIZV",		"BLTSIZH",
	/* 0x060 */
	"BLTCMOD",		"BLTBMOD",		"BLTAMOD",		"BLTDMOD",
	"UNK068",		"UNK06A",		"UNK06C",		"UNK06E",
	"BLTCDAT",		"BLTBDAT",		"BLTADAT",		"UNK076",
	"SPRHDAT",		"BPLHDAT",		"LISAID",		"DSRSYNC",
	/* 0x080 */
	"COP1LCH",		"COP1LCL",		"COP2LCH",		"COP2LCL",
	"COPJMP1",		"COPJMP2",		"COPINS",		"DIWSTRT",
	"DIWSTOP",		"DDFSTRT",		"DDFSTOP",		"DMACON",
	"CLXCON",		"INTENA",		"INTREQ",		"ADKCON",
	/* 0x0A0 */
	"AUD0LCH",		"AUD0LCL",		"AUD0LEN",		"AUD0PER",
	"AUD0VOL",		"AUD0DAT",		"UNK0AC",		"UNK0AE",
	"AUD1LCH",		"AUD1LCL",		"AUD1LEN",		"AUD1PER",
	"AUD1VOL",		"AUD1DAT",		"UNK0BC",		"UNK0BE",
	/* 0x0C0 */
	"AUD2LCH",		"AUD2LCL",		"AUD2LEN",		"AUD2PER",
	"AUD2VOL",		"AUD2DAT",		"UNK0CC",		"UNK0CE",
	"AUD3LCH",		"AUD3LCL",		"AUD3LEN",		"AUD3PER",
	"AUD3VOL",		"AUD3DAT",		"UNK0DC",		"UNK0DE",
	/* 0x0E0 */
	"BPL1PTH",		"BPL1PTL",		"BPL2PTH",		"BPL2PTL",
	"BPL3PTH",		"BPL3PTL",		"BPL4PTH",		"BPL4PTL",
	"BPL5PTH",		"BPL5PTL",		"BPL6PTH",		"BPL6PTL",
	"BPL7PTH",		"BPL7PTL",		"BPL8PTH",		"BPL8PTL",
	/* 0x100 */
	"BPLCON0",		"BPLCON1",		"BPLCON2",		"BPLCON3",
	"BPL1MOD",		"BPL2MOD",		"BPLCON4",		"CLXCON2",
	"BPL1DAT",		"BPL2DAT",		"BPL3DAT",		"BPL4DAT",
	"BPL5DAT",		"BPL6DAT",		"BPL7DAT",		"BPL8DAT",
	/* 0x120 */
	"SPR0PTH",		"SPR0PTL",		"SPR1PTH",		"SPR1PTL",
	"SPR2PTH",		"SPR2PTL",		"SPR3PTH",		"SPR3PTL",
	"SPR4PTH",		"SPR4PTL",		"SPR5PTH",		"SPR5PTL",
	"SPR6PTH",		"SPR6PTL",		"SPR7PTH",		"SPR7PTL",
	/* 0x140 */
	"SPR0POS",		"SPR0CTL",		"SPR0DATA", 	"SPR0DATB",
	"SPR1POS",		"SPR1CTL",		"SPR1DATA", 	"SPR1DATB",
	"SPR2POS",		"SPR2CTL",		"SPR2DATA", 	"SPR2DATB",
	"SPR3POS",		"SPR3CTL",		"SPR3DATA", 	"SPR3DATB",
	/* 0x160 */
	"SPR4POS",		"SPR4CTL",		"SPR4DATA", 	"SPR4DATB",
	"SPR5POS",		"SPR5CTL",		"SPR5DATA", 	"SPR5DATB",
	"SPR6POS",		"SPR6CTL",		"SPR6DATA", 	"SPR6DATB",
	"SPR7POS",		"SPR7CTL",		"SPR7DATA", 	"SPR7DATB",
	/* 0x180 */
	"COLOR00",		"COLOR01",		"COLOR02",		"COLOR03",
	"COLOR04",		"COLOR05",		"COLOR06",		"COLOR07",
	"COLOR08",		"COLOR09",		"COLOR10",		"COLOR11",
	"COLOR12",		"COLOR13",		"COLOR14",		"COLOR15",
	/* 0x1A0 */
	"COLOR16",		"COLOR17",		"COLOR18",		"COLOR19",
	"COLOR20",		"COLOR21",		"COLOR22",		"COLOR23",
	"COLOR24",		"COLOR25",		"COLOR26",		"COLOR27",
	"COLOR28",		"COLOR29",		"COLOR30",		"COLOR31",
	/* 0x1C0 */
	"HTOTAL",		"HSSTOP",		"HBSTRT",		"HBSTOP",
	"VTOTAL",		"VSSTOP",		"VBSTRT",		"VBSTOP",
	"SPRHSTRT",		"SPRHSTOP",		"BPLHSTRT",		"BPLHSTOP",
	"HHPOSW",		"HHPOSR",		"BEAMCON0",		"HSSTRT",
	/* 0x1E0 */
	"VSSTRT",		"HCENTER",		"DIWHIGH",		"BPLHMOD",
	"SPRHPTH",		"SPRHPTL",		"BPLHPTH",		"BPLHPTL",
	"UNK1F0",		"UNK1F2",		"UNK1F4",		"UNK1F6",
	"UNK1F8",		"UNK1FA",		"FMODE",		"UNK1FE"
};



/*************************************
 *
 *  Prototypes
 *
 *************************************/

static void custom_reset(running_machine *machine);
static void autoconfig_reset(running_machine *machine);
static TIMER_CALLBACK( amiga_irq_proc );
static TIMER_CALLBACK( amiga_blitter_proc );
static TIMER_CALLBACK( scanline_callback );



/*************************************
 *
 *  Chipmem 16/32 bit access
 *
 *************************************/

UINT16 (*amiga_chip_ram_r)(offs_t offset);
void (*amiga_chip_ram_w)(offs_t offset, UINT16 data);

static UINT16 amiga_chip_ram16_r(offs_t offset)
{
	extern const amiga_machine_interface *amiga_intf;
	offset &= amiga_intf->chip_ram_mask;
	return (offset < amiga_chip_ram_size) ? amiga_chip_ram[offset/2] : 0xffff;
}

static UINT16 amiga_chip_ram32_r(offs_t offset)
{
	extern const amiga_machine_interface *amiga_intf;
	offset &= amiga_intf->chip_ram_mask;

	if ( offset < amiga_chip_ram_size )
	{
		UINT32	dat = amiga_chip_ram32[offset / 4];

		if ( offset & 2 )
			return (dat & 0xffff);

		return (dat >> 16);
	}

	return 0xffff;
}

static void amiga_chip_ram16_w(offs_t offset, UINT16 data)
{
	extern const amiga_machine_interface *amiga_intf;
	offset &= amiga_intf->chip_ram_mask;

	if (offset < amiga_chip_ram_size)
		amiga_chip_ram[offset/2] = data;
}

static void amiga_chip_ram32_w(offs_t offset, UINT16 data)
{
	extern const amiga_machine_interface *amiga_intf;
	offset &= amiga_intf->chip_ram_mask;

	if ( offset < amiga_chip_ram_size )
	{
		UINT32	dat = amiga_chip_ram32[offset / 4];

		if ( offset & 2 )
		{
			dat &= 0xffff0000;
			dat |= data;
		}
		else
		{
			dat &= 0x0000ffff;
			dat |= ((UINT32)data) << 16;
		}

		amiga_chip_ram32[offset / 4] = dat;
	}
}


void amiga_chip_ram_w8(offs_t offset, UINT8 data)
{
	UINT16 dat;

	dat = amiga_chip_ram_r(offset);
	if (offset & 0x01)
	{
		dat &= 0xff00;
		dat |= data;
	}
	else
	{
		dat &= 0x00ff;
		dat |= ((UINT16)data) << 8;
	}
	amiga_chip_ram_w(offset,dat);
}

/*************************************
 *
 *  Machine config/reset
 *
 *************************************/

void amiga_machine_config(running_machine *machine, const amiga_machine_interface *intf)
{
	amiga_intf = intf;

	/* setup chipmem handlers */
	if ( IS_AGA(intf) )
	{
		amiga_chip_ram_r = amiga_chip_ram32_r;
		amiga_chip_ram_w = amiga_chip_ram32_w;
	}
	else
	{
		amiga_chip_ram_r = amiga_chip_ram16_r;
		amiga_chip_ram_w = amiga_chip_ram16_w;
	}

	/* setup the timers */
	amiga_irq_timer = timer_alloc(machine, amiga_irq_proc, NULL);
	amiga_blitter_timer = timer_alloc(machine, amiga_blitter_proc, NULL);
}


static void amiga_m68k_reset(running_device *device)
{
	const address_space *space = cpu_get_address_space(device, ADDRESS_SPACE_PROGRAM);

	logerror("Executed RESET at PC=%06x\n", cpu_get_pc(space->cpu));

	/* Initialize the various chips */
	devtag_reset(device->machine, "cia_0");
	devtag_reset(device->machine, "cia_1");
	custom_reset(device->machine);
	autoconfig_reset(device->machine);

	/* set the overlay bit */
	if ( IS_AGA(amiga_intf) )
	{
		memory_write_byte( space, 0xbfa001, 1 );
	}
	else
	{
		amiga_cia_w(space, 0x1001/2, 1, 0xffff);
	}
}


MACHINE_RESET( amiga )
{
	/* set m68k reset  function */
	m68k_set_reset_callback(machine->device("maincpu"), amiga_m68k_reset);

	amiga_m68k_reset(machine->device("maincpu"));

	/* call the system-specific callback */
	if (amiga_intf->reset_callback)
		(*amiga_intf->reset_callback)(machine);

	/* start the scanline timer */
	timer_set(machine, machine->primary_screen->time_until_pos(0), NULL, 0, scanline_callback);
}



/*************************************
 *
 *  Per scanline callback
 *
 *************************************/

static TIMER_CALLBACK( scanline_callback )
{
	int scanline = param;
	running_device *cia_0 = machine->device("cia_0");
	running_device *cia_1 = machine->device("cia_1");

	/* on the first scanline, we do some extra bookkeeping */
	if (scanline == 0)
	{
		/* signal VBLANK IRQ */
		amiga_custom_w(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), REG_INTREQ, 0x8000 | INTENA_VERTB, 0xffff);

		/* clock the first CIA TOD */
		mos6526_tod_w(cia_0, 1);

		/* call the system-specific callback */
		if (amiga_intf->scanline0_callback != NULL)
			(*amiga_intf->scanline0_callback)(machine);
	}

	/* on every scanline, clock the second CIA TOD */
	mos6526_tod_w(cia_1, 1);

	/* render up to this scanline */
	if (!machine->primary_screen->update_partial(scanline))
	{
		if (IS_AGA(amiga_intf))
			amiga_aga_render_scanline(machine, NULL, scanline);
		else
			amiga_render_scanline(machine, NULL, scanline);
	}

	/* force a sound update */
	amiga_audio_update();

	/* set timer for next line */
	scanline = (scanline + 1) % machine->primary_screen->height();
	timer_set(machine, machine->primary_screen->time_until_pos(scanline), NULL, scanline, scanline_callback);
}



/*************************************
 *
 *  Interrupt management
 *
 *************************************/

static void update_irqs(running_machine *machine)
{
	int ints = CUSTOM_REG(REG_INTENA) & CUSTOM_REG(REG_INTREQ);

	/* Master interrupt switch */
	if (CUSTOM_REG(REG_INTENA) & 0x4000)
	{
		/* Serial transmit buffer empty, disk block finished, software interrupts */
		cputag_set_input_line(machine, "maincpu", 1, ints & 0x0007 ? ASSERT_LINE : CLEAR_LINE);

		/* I/O ports and timer interrupts */
		cputag_set_input_line(machine, "maincpu", 2, ints & 0x0008 ? ASSERT_LINE : CLEAR_LINE);

		/* Copper, VBLANK, blitter interrupts */
		cputag_set_input_line(machine, "maincpu", 3, ints & 0x0070 ? ASSERT_LINE : CLEAR_LINE);

		/* Audio interrupts */
		cputag_set_input_line(machine, "maincpu", 4, ints & 0x0780 ? ASSERT_LINE : CLEAR_LINE);

		/* Serial receive buffer full, disk sync match */
		cputag_set_input_line(machine, "maincpu", 5, ints & 0x1800 ? ASSERT_LINE : CLEAR_LINE);

		/* External interrupts */
		cputag_set_input_line(machine, "maincpu", 6, ints & 0x2000 ? ASSERT_LINE : CLEAR_LINE);
	}
	else
	{
		cputag_set_input_line(machine, "maincpu", 1, CLEAR_LINE);
		cputag_set_input_line(machine, "maincpu", 2, CLEAR_LINE);
		cputag_set_input_line(machine, "maincpu", 3, CLEAR_LINE);
		cputag_set_input_line(machine, "maincpu", 4, CLEAR_LINE);
		cputag_set_input_line(machine, "maincpu", 5, CLEAR_LINE);
		cputag_set_input_line(machine, "maincpu", 6, CLEAR_LINE);
	}
}


static TIMER_CALLBACK( amiga_irq_proc )
{
	update_irqs(machine);
	timer_reset( amiga_irq_timer, attotime_never);
}



/*************************************
 *
 *  Standard joystick conversion
 *
 *************************************/

CUSTOM_INPUT( amiga_joystick_convert )
{
	UINT8 bits = input_port_read(field->port->machine, (const char *)param);
	int up = (bits >> 0) & 1;
	int down = (bits >> 1) & 1;
	int left = (bits >> 2) & 1;
	int right = (bits >> 3) & 1;

	if (left) up ^= 1;
	if (right) down ^= 1;

	return down | (right << 1) | (up << 8) | (left << 9);
}



/*************************************
 *
 *  Ascending blitter variant
 *
 *************************************/

static UINT32 blit_ascending(void)
{
	UINT32 shifta = (CUSTOM_REG(REG_BLTCON0) >> 12) & 0xf;
	UINT32 shiftb = (CUSTOM_REG(REG_BLTCON1) >> 12) & 0xf;
	UINT32 height = CUSTOM_REG(REG_BLTSIZV);
	UINT32 width = CUSTOM_REG(REG_BLTSIZH);
	UINT32 acca = 0, accb = 0;
	UINT32 blitsum = 0;
	UINT32 x, y;

	/* iterate over the height */
	for (y = 0; y < height; y++)
	{
		/* iterate over the width */
		for (x = 0; x < width; x++)
		{
			UINT16 abc0, abc1, abc2, abc3;
			UINT32 tempa, tempd = 0;
			UINT32 b;

			/* fetch data for A */
			if (CUSTOM_REG(REG_BLTCON0) & 0x0800)
			{
				CUSTOM_REG(REG_BLTADAT) = amiga_chip_ram_r(CUSTOM_REG_LONG(REG_BLTAPTH));
				CUSTOM_REG_LONG(REG_BLTAPTH) += 2;
			}

			/* fetch data for B */
			if (CUSTOM_REG(REG_BLTCON0) & 0x0400)
			{
				CUSTOM_REG(REG_BLTBDAT) = amiga_chip_ram_r(CUSTOM_REG_LONG(REG_BLTBPTH));
				CUSTOM_REG_LONG(REG_BLTBPTH) += 2;
			}

			/* fetch data for C */
			if (CUSTOM_REG(REG_BLTCON0) & 0x0200)
			{
				CUSTOM_REG(REG_BLTCDAT) = amiga_chip_ram_r(CUSTOM_REG_LONG(REG_BLTCPTH));
				CUSTOM_REG_LONG(REG_BLTCPTH) += 2;
			}

			/* apply start/end masks to the A data */
			tempa = CUSTOM_REG(REG_BLTADAT);
		    if (x == 0)
		    	tempa &= CUSTOM_REG(REG_BLTAFWM);
		    if (x == width - 1)
		    	tempa &= CUSTOM_REG(REG_BLTALWM);

			/* update the B accumulator applying shifts */
			acca = (acca << 16) | (tempa << (16 - shifta));
			accb = (accb << 16) | (CUSTOM_REG(REG_BLTBDAT) << (16 - shiftb));

			/* build up 4 16-bit words containing 4 pixels each in 0ABC bit order */
			abc0 = ((acca >> 17) & 0x4444) | ((accb >> 18) & 0x2222) | ((CUSTOM_REG(REG_BLTCDAT) >> 3) & 0x1111);
			abc1 = ((acca >> 16) & 0x4444) | ((accb >> 17) & 0x2222) | ((CUSTOM_REG(REG_BLTCDAT) >> 2) & 0x1111);
			abc2 = ((acca >> 15) & 0x4444) | ((accb >> 16) & 0x2222) | ((CUSTOM_REG(REG_BLTCDAT) >> 1) & 0x1111);
			abc3 = ((acca >> 14) & 0x4444) | ((accb >> 15) & 0x2222) | ((CUSTOM_REG(REG_BLTCDAT) >> 0) & 0x1111);

			/* now loop over bits and compute the destination value */
			for (b = 0; b < 4; b++)
			{
				UINT32 bit;

				/* shift previous data up 4 bits */
				tempd <<= 4;

				/* lookup first bit in series */
				bit = (CUSTOM_REG(REG_BLTCON0) >> (abc0 >> 12)) & 1;
				abc0 <<= 4;
				tempd |= bit << 3;

				/* lookup second bit in series */
				bit = (CUSTOM_REG(REG_BLTCON0) >> (abc1 >> 12)) & 1;
				abc1 <<= 4;
				tempd |= bit << 2;

				/* lookup third bit in series */
				bit = (CUSTOM_REG(REG_BLTCON0) >> (abc2 >> 12)) & 1;
				abc2 <<= 4;
				tempd |= bit << 1;

				/* lookup fourth bit in series */
				bit = (CUSTOM_REG(REG_BLTCON0) >> (abc3 >> 12)) & 1;
				abc3 <<= 4;
				tempd |= bit << 0;
			}

			/* accumulate the sum */
			blitsum |= tempd;

			/* write to the destination */
			if (CUSTOM_REG(REG_BLTCON0) & 0x0100)
			{
				amiga_chip_ram_w(CUSTOM_REG_LONG(REG_BLTDPTH), tempd);
				CUSTOM_REG_LONG(REG_BLTDPTH) += 2;
			}
		}

		/* apply end of line modulos */
		if (CUSTOM_REG(REG_BLTCON0) & 0x0800)
			CUSTOM_REG_LONG(REG_BLTAPTH) += CUSTOM_REG_SIGNED(REG_BLTAMOD) & ~1;
		if (CUSTOM_REG(REG_BLTCON0) & 0x0400)
			CUSTOM_REG_LONG(REG_BLTBPTH) += CUSTOM_REG_SIGNED(REG_BLTBMOD) & ~1;
		if (CUSTOM_REG(REG_BLTCON0) & 0x0200)
			CUSTOM_REG_LONG(REG_BLTCPTH) += CUSTOM_REG_SIGNED(REG_BLTCMOD) & ~1;
		if (CUSTOM_REG(REG_BLTCON0) & 0x0100)
			CUSTOM_REG_LONG(REG_BLTDPTH) += CUSTOM_REG_SIGNED(REG_BLTDMOD) & ~1;
	}

	/* return the blit sum */
	return blitsum;
}



/*************************************
 *
 *  Descending blitter variant
 *
 *************************************/

static UINT32 blit_descending(void)
{
	UINT32 fill_exclusive = (CUSTOM_REG(REG_BLTCON1) >> 4);
	UINT32 fill_inclusive = (CUSTOM_REG(REG_BLTCON1) >> 3);
	UINT32 shifta = (CUSTOM_REG(REG_BLTCON0) >> 12) & 0xf;
	UINT32 shiftb = (CUSTOM_REG(REG_BLTCON1) >> 12) & 0xf;
	UINT32 height = CUSTOM_REG(REG_BLTSIZV);
	UINT32 width = CUSTOM_REG(REG_BLTSIZH);
	UINT32 acca = 0, accb = 0;
	UINT32 blitsum = 0;
	UINT32 x, y;

	/* iterate over the height */
	for (y = 0; y < height; y++)
	{
		UINT32 fill_state = (CUSTOM_REG(REG_BLTCON1) >> 2) & 1;

		/* iterate over the width */
		for (x = 0; x < width; x++)
		{
			UINT16 abc0, abc1, abc2, abc3;
			UINT32 tempa, tempd = 0;
			UINT32 b;

			/* fetch data for A */
			if (CUSTOM_REG(REG_BLTCON0) & 0x0800)
			{
				CUSTOM_REG(REG_BLTADAT) = amiga_chip_ram_r(CUSTOM_REG_LONG(REG_BLTAPTH));
				CUSTOM_REG_LONG(REG_BLTAPTH) -= 2;
			}

			/* fetch data for B */
			if (CUSTOM_REG(REG_BLTCON0) & 0x0400)
			{
				CUSTOM_REG(REG_BLTBDAT) = amiga_chip_ram_r(CUSTOM_REG_LONG(REG_BLTBPTH));
				CUSTOM_REG_LONG(REG_BLTBPTH) -= 2;
			}

			/* fetch data for C */
			if (CUSTOM_REG(REG_BLTCON0) & 0x0200)
			{
				CUSTOM_REG(REG_BLTCDAT) = amiga_chip_ram_r(CUSTOM_REG_LONG(REG_BLTCPTH));
				CUSTOM_REG_LONG(REG_BLTCPTH) -= 2;
			}

			/* apply start/end masks to the A data */
			tempa = CUSTOM_REG(REG_BLTADAT);
		    if (x == 0)
		    	tempa &= CUSTOM_REG(REG_BLTAFWM);
		    if (x == width - 1)
		    	tempa &= CUSTOM_REG(REG_BLTALWM);

			/* update the B accumulator applying shifts */
			acca = (acca >> 16) | (tempa << shifta);
			accb = (accb >> 16) | (CUSTOM_REG(REG_BLTBDAT) << shiftb);

			/* build up 4 16-bit words containing 4 pixels each in 0ABC bit order */
			abc0 = ((acca >> 1) & 0x4444) | ((accb >> 2) & 0x2222) | ((CUSTOM_REG(REG_BLTCDAT) >> 3) & 0x1111);
			abc1 = ((acca >> 0) & 0x4444) | ((accb >> 1) & 0x2222) | ((CUSTOM_REG(REG_BLTCDAT) >> 2) & 0x1111);
			abc2 = ((acca << 1) & 0x4444) | ((accb >> 0) & 0x2222) | ((CUSTOM_REG(REG_BLTCDAT) >> 1) & 0x1111);
			abc3 = ((acca << 2) & 0x4444) | ((accb << 1) & 0x2222) | ((CUSTOM_REG(REG_BLTCDAT) >> 0) & 0x1111);

			/* now loop over bits and compute the destination value */
			for (b = 0; b < 4; b++)
			{
				UINT32 prev_fill_state;
				UINT32 bit;

				/* shift previous data up 4 bits */
				tempd >>= 4;

				/* lookup fourth bit in series */
				bit = (CUSTOM_REG(REG_BLTCON0) >> (abc3 & 0xf)) & 1;
				abc3 >>= 4;
				prev_fill_state = fill_state;
				fill_state ^= bit;
				bit ^= prev_fill_state & fill_exclusive;
				bit |= prev_fill_state & fill_inclusive;
				tempd |= bit << 12;

				/* lookup third bit in series */
				bit = (CUSTOM_REG(REG_BLTCON0) >> (abc2 & 0xf)) & 1;
				abc2 >>= 4;
				prev_fill_state = fill_state;
				fill_state ^= bit;
				bit ^= prev_fill_state & fill_exclusive;
				bit |= prev_fill_state & fill_inclusive;
				tempd |= bit << 13;

				/* lookup second bit in series */
				bit = (CUSTOM_REG(REG_BLTCON0) >> (abc1 & 0xf)) & 1;
				abc1 >>= 4;
				prev_fill_state = fill_state;
				fill_state ^= bit;
				bit ^= prev_fill_state & fill_exclusive;
				bit |= prev_fill_state & fill_inclusive;
				tempd |= bit << 14;

				/* lookup first bit in series */
				bit = (CUSTOM_REG(REG_BLTCON0) >> (abc0 & 0xf)) & 1;
				abc0 >>= 4;
				prev_fill_state = fill_state;
				fill_state ^= bit;
				bit ^= prev_fill_state & fill_exclusive;
				bit |= prev_fill_state & fill_inclusive;
				tempd |= bit << 15;
			}

			/* accumulate the sum */
			blitsum |= tempd;

			/* write to the destination */
			if (CUSTOM_REG(REG_BLTCON0) & 0x0100)
			{
				amiga_chip_ram_w(CUSTOM_REG_LONG(REG_BLTDPTH), tempd);
				CUSTOM_REG_LONG(REG_BLTDPTH) -= 2;
			}
		}

		/* apply end of line modulos */
		if (CUSTOM_REG(REG_BLTCON0) & 0x0800)
			CUSTOM_REG_LONG(REG_BLTAPTH) -= CUSTOM_REG_SIGNED(REG_BLTAMOD) & ~1;
		if (CUSTOM_REG(REG_BLTCON0) & 0x0400)
			CUSTOM_REG_LONG(REG_BLTBPTH) -= CUSTOM_REG_SIGNED(REG_BLTBMOD) & ~1;
		if (CUSTOM_REG(REG_BLTCON0) & 0x0200)
			CUSTOM_REG_LONG(REG_BLTCPTH) -= CUSTOM_REG_SIGNED(REG_BLTCMOD) & ~1;
		if (CUSTOM_REG(REG_BLTCON0) & 0x0100)
			CUSTOM_REG_LONG(REG_BLTDPTH) -= CUSTOM_REG_SIGNED(REG_BLTDMOD) & ~1;
	}

	/* return the blit sum */
	return blitsum;
}



/*************************************
 *
 *  Line drawing blitter variant
 *
 *************************************/

/*
    The exact line drawing algorithm is not known, but based on the cryptic
    setup instructions, it is clear that it is a basic Bresenham line
    algorithm. A standard Bresenham algorithm looks like this:

    epsilon = 0;
    while (length--)
    {
        plot(x, y);
        x++;
        epsilon += dy;
        if ((2 * epsilon) >= dx)
        {
            y++;
            epsilon -= dx;
        }
    }

    If you multiply the epsilon term by 4 and shuffle the logic a bit, the
    equivalent logic is:

    epsilon = 4 * dy - 2 * dx;
    while (length--)
    {
        plot(x, y);
        x++;
        if (epsilon >= 0)
        {
            y++;
            epsilon += 4 * (dy - dx);
        }
        else
            epsilon += 4 * dy;
    }

    With this refactoring, you can see that BLTAPT = epsilon,
    BLTAMOD = 4 * (dy - dx) and BLTBMOD = 4 * dy.
*/

static UINT32 blit_line(void)
{
	UINT32 singlemode = (CUSTOM_REG(REG_BLTCON1) & 0x0002) ? 0x0000 : 0xffff;
	UINT32 singlemask = 0xffff;
	UINT32 blitsum = 0;
	UINT32 height;

	/* see if folks are breaking the rules */
	if (CUSTOM_REG(REG_BLTSIZH) != 0x0002)
		logerror("Blitter: Blit width != 2 in line mode!\n");
	if ((CUSTOM_REG(REG_BLTCON0) & 0x0a00) != 0x0a00)
		logerror("Blitter: Channel selection incorrect in line mode!\n" );

	/* extract the length of the line */
	height = CUSTOM_REG(REG_BLTSIZV);

	/* iterate over the line height */
	while (height--)
	{
		UINT16 abc0, abc1, abc2, abc3;
		UINT32 tempa, tempb, tempd = 0;
		int b, dx, dy;

		/* fetch data for C */
		if (CUSTOM_REG(REG_BLTCON0) & 0x0200)
			CUSTOM_REG(REG_BLTCDAT) = amiga_chip_ram_r(CUSTOM_REG_LONG(REG_BLTCPTH));

		/* rotate the A data according to the shift */
		tempa = CUSTOM_REG(REG_BLTADAT) >> (CUSTOM_REG(REG_BLTCON0) >> 12);

		/* apply single bit mask */
		tempa &= singlemask;
		singlemask &= singlemode;

		/* rotate the B data according to the shift and expand to 16 bits */
		tempb = -((CUSTOM_REG(REG_BLTBDAT) >> (CUSTOM_REG(REG_BLTCON1) >> 12)) & 1);

		/* build up 4 16-bit words containing 4 pixels each in 0ABC bit order */
		abc0 = ((tempa >> 1) & 0x4444) | (tempb & 0x2222) | ((CUSTOM_REG(REG_BLTCDAT) >> 3) & 0x1111);
		abc1 = ((tempa >> 0) & 0x4444) | (tempb & 0x2222) | ((CUSTOM_REG(REG_BLTCDAT) >> 2) & 0x1111);
		abc2 = ((tempa << 1) & 0x4444) | (tempb & 0x2222) | ((CUSTOM_REG(REG_BLTCDAT) >> 1) & 0x1111);
		abc3 = ((tempa << 2) & 0x4444) | (tempb & 0x2222) | ((CUSTOM_REG(REG_BLTCDAT) >> 0) & 0x1111);

		/* now loop over bits and compute the destination value */
		for (b = 0; b < 4; b++)
		{
			UINT32 bit;

			/* shift previous data up 4 bits */
			tempd <<= 4;

			/* lookup first bit in series */
			bit = (CUSTOM_REG(REG_BLTCON0) >> (abc0 >> 12)) & 1;
			abc0 <<= 4;
			tempd |= bit << 3;

			/* lookup second bit in series */
			bit = (CUSTOM_REG(REG_BLTCON0) >> (abc1 >> 12)) & 1;
			abc1 <<= 4;
			tempd |= bit << 2;

			/* lookup third bit in series */
			bit = (CUSTOM_REG(REG_BLTCON0) >> (abc2 >> 12)) & 1;
			abc2 <<= 4;
			tempd |= bit << 1;

			/* lookup fourth bit in series */
			bit = (CUSTOM_REG(REG_BLTCON0) >> (abc3 >> 12)) & 1;
			abc3 <<= 4;
			tempd |= bit << 0;
		}

		/* accumulate the sum */
		blitsum |= tempd;

		/* write to the destination */
		amiga_chip_ram_w(CUSTOM_REG_LONG(REG_BLTDPTH), tempd);

		/* always increment along the major axis */
		if (CUSTOM_REG(REG_BLTCON1) & 0x0010)
		{
			dx = (CUSTOM_REG(REG_BLTCON1) & 0x0004) ? -1 : 1;
			dy = 0;
		}
		else
		{
			dx = 0;
			dy = (CUSTOM_REG(REG_BLTCON1) & 0x0004) ? -1 : 1;
		}

		/* is the sign bit clear? */
		if (!(CUSTOM_REG(REG_BLTCON1) & 0x0040))
		{
			/* add 4 * (dy-dx) */
			CUSTOM_REG_LONG(REG_BLTAPTH) += CUSTOM_REG_SIGNED(REG_BLTAMOD) & ~1;

			/* increment along the minor axis */
			if (CUSTOM_REG(REG_BLTCON1) & 0x0010)
				dy = (CUSTOM_REG(REG_BLTCON1) & 0x0008) ? -1 : 1;
			else
				dx = (CUSTOM_REG(REG_BLTCON1) & 0x0008) ? -1 : 1;
		}

		/* else add 4 * dy and don't increment along the minor axis */
		else
			CUSTOM_REG_LONG(REG_BLTAPTH) += CUSTOM_REG_SIGNED(REG_BLTBMOD) & ~1;

		/* adjust X if necessary */
		if (dx)
		{
			/* adjust the A shift value */
			UINT32 temp = CUSTOM_REG(REG_BLTCON0) + (INT32)(dx << 12);
			CUSTOM_REG(REG_BLTCON0) = temp;

			/* if we went from 0xf to 0x0 or vice-versa, adjust the actual pointers */
			if (temp & 0x10000)
			{
				CUSTOM_REG_LONG(REG_BLTCPTH) += 2 * dx;
				CUSTOM_REG_LONG(REG_BLTDPTH) += 2 * dx;
			}
		}

		/* adjust Y if necessary */
		if (dy)
		{
			/* BLTCMOD seems to be used for both C and D pointers */
			CUSTOM_REG_LONG(REG_BLTCPTH) += dy * (INT16)(CUSTOM_REG_SIGNED(REG_BLTCMOD) & ~1);
			CUSTOM_REG_LONG(REG_BLTDPTH) += dy * (INT16)(CUSTOM_REG_SIGNED(REG_BLTCMOD) & ~1);

			/* reset the single mask since we're on a new line */
			singlemask = 0xffff;
		}

		/* set the new sign bit value */
		CUSTOM_REG(REG_BLTCON1) = (CUSTOM_REG(REG_BLTCON1) & ~0x0040) | ((CUSTOM_REG(REG_BLTAPTL) >> 9) & 0x0040);

		/* increment texture shift on every pixel */
		CUSTOM_REG(REG_BLTCON1) += 0x1000;
	}

	return blitsum;
}



/*************************************
 *
 *  Blitter deferred callback
 *
 *************************************/

static TIMER_CALLBACK( amiga_blitter_proc )
{
	UINT32 blitsum = 0;

	/* logging */
	if (LOG_BLITS)
	{
		static const char *const type[] = { "ASCENDING", "LINE", "DESCENDING", "LINE" };
		logerror("BLIT %s: %dx%d  %04x %04x\n", type[CUSTOM_REG(REG_BLTCON1) & 0x0003], CUSTOM_REG(REG_BLTSIZH), CUSTOM_REG(REG_BLTSIZV), CUSTOM_REG(REG_BLTCON0), CUSTOM_REG(REG_BLTCON1));
		if (CUSTOM_REG(REG_BLTCON0) & 0x0800)
			logerror("  A: addr=%06X mod=%3d shift=%2d maskl=%04x maskr=%04x\n", CUSTOM_REG_LONG(REG_BLTAPTH), CUSTOM_REG_SIGNED(REG_BLTAMOD), CUSTOM_REG(REG_BLTCON0) >> 12, CUSTOM_REG(REG_BLTAFWM), CUSTOM_REG(REG_BLTALWM));
		if (CUSTOM_REG(REG_BLTCON0) & 0x0400)
			logerror("  B: addr=%06X mod=%3d shift=%2d\n", CUSTOM_REG_LONG(REG_BLTBPTH), CUSTOM_REG_SIGNED(REG_BLTBMOD), CUSTOM_REG(REG_BLTCON1) >> 12);
		if (CUSTOM_REG(REG_BLTCON0) & 0x0200)
			logerror("  C: addr=%06X mod=%3d\n", CUSTOM_REG_LONG(REG_BLTCPTH), CUSTOM_REG_SIGNED(REG_BLTCMOD));
		if (CUSTOM_REG(REG_BLTCON0) & 0x0100)
			logerror("  D: addr=%06X mod=%3d\n", CUSTOM_REG_LONG(REG_BLTDPTH), CUSTOM_REG_SIGNED(REG_BLTDMOD));
	}

	/* set the zero flag */
	CUSTOM_REG(REG_DMACON) |= 0x2000;

	/* switch off the type of blit */
	switch (CUSTOM_REG(REG_BLTCON1) & 0x0003)
	{
		case 0:	/* ascending */
			blitsum = blit_ascending();
			break;

		case 2:	/* descending */
			blitsum = blit_descending();
			break;

		case 1:	/* line */
		case 3:
			blitsum = blit_line();
			break;
	}

	/* clear the zero flag if we actually wrote data */
	if (blitsum)
		CUSTOM_REG(REG_DMACON) &= ~0x2000;

	/* no longer busy */
	CUSTOM_REG(REG_DMACON) &= ~0x4000;

	/* signal an interrupt */
	amiga_custom_w(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), REG_INTREQ, 0x8000 | INTENA_BLIT, 0xffff);

	/* reset the blitter timer */
	timer_reset( amiga_blitter_timer, attotime_never);
}



/*************************************
 *
 *  Blitter setup
 *
 *************************************/

static void blitter_setup(const address_space *space)
{
	int ticks, width, height, blittime;

	/* is there another blitting in progress? */
	if (CUSTOM_REG(REG_DMACON) & 0x4000)
	{
		logerror("%s - This program is playing tricks with the blitter\n", cpuexec_describe_context(space->machine) );
		return;
	}

	/* line mode is 8 ticks/pixel */
	if (CUSTOM_REG(REG_BLTCON1) & 1)
		ticks = 8;

	/* standard mode is 4 ticks base */
	else
	{
		ticks = 4;

		/* plus 2 ticks if channel B is involved */
		if (CUSTOM_REG(REG_BLTCON0) & 0x0400)
			ticks += 2;

		/* plus 2 ticks if both channel C and D are involved */
		if ((CUSTOM_REG(REG_BLTCON0) & 0x0300) == 0x0300)
			ticks += 2;
	}

	/* extract height/width */
	width = CUSTOM_REG(REG_BLTSIZH);
	height = CUSTOM_REG(REG_BLTSIZV);

	/* compute the blit time */
	blittime = ticks * height * width;

	/* if 'blitter-nasty' is set, then the blitter takes over the bus. Make the blit semi-immediate */
	if ( CUSTOM_REG(REG_DMACON) & 0x0400 )
	{
		/* simulate the 68k not running while the blit is going */
		cpu_adjust_icount( space->cpu, -(blittime/2) );

		blittime = BLITTER_NASTY_DELAY;
	}

	/* AGA has twice the bus bandwidth, so blits take half the time */
	if ( IS_AGA(amiga_intf) )
		blittime /= 2;

	/* signal blitter busy */
	CUSTOM_REG(REG_DMACON) |= 0x4000;

	/* set a timer */
	timer_adjust_oneshot( amiga_blitter_timer, downcast<cpu_device *>(space->cpu)->cycles_to_attotime( blittime ), 0);
}



/*************************************
 *
 *  8520 CIA read handler
 *
 *************************************/

READ16_HANDLER( amiga_cia_r )
{
	UINT8 data;
	int shift;
	running_device *cia;

	/* offsets 0000-07ff reference CIA B, and are accessed via the MSB */
	if ((offset & 0x0800) == 0)
	{
		cia = space->machine->device("cia_1");
		shift = 8;
	}

	/* offsets 0800-0fff reference CIA A, and are accessed via the LSB */
	else
	{
		cia = space->machine->device("cia_0");
		shift = 0;
	}

	/* handle the reads */
	data = mos6526_r(cia, offset >> 7);

	if (LOG_CIA)
		logerror("%06x:cia_%c_read(%03x) = %04x & %04x\n", cpu_get_pc(space->cpu), 'A' + ((~offset & 0x0800) >> 11), offset * 2, data << shift, mem_mask);

	return data << shift;
}



/*************************************
 *
 *  8520 CIA write handler
 *
 *************************************/

WRITE16_HANDLER( amiga_cia_w )
{
	running_device *cia;

	if (LOG_CIA)
		logerror("%06x:cia_%c_write(%03x) = %04x & %04x\n", cpu_get_pc(space->cpu), 'A' + ((~offset & 0x0800) >> 11), offset * 2, data, mem_mask);

	/* offsets 0000-07ff reference CIA B, and are accessed via the MSB */
	if ((offset & 0x0800) == 0)
	{
		if (!ACCESSING_BITS_8_15)
			return;
		cia = space->machine->device("cia_1");
		data >>= 8;
	}

	/* offsets 0800-0fff reference CIA A, and are accessed via the LSB */
	else
	{
		if (!ACCESSING_BITS_0_7)
			return;
		cia = space->machine->device("cia_0");
		data &= 0xff;
	}

	/* handle the writes */
	mos6526_w(cia, offset >> 7, (UINT8) data);
}



/*************************************
 *
 *  CIA interrupt callbacks
 *
 *************************************/

void amiga_cia_0_irq(running_device *device, int state)
{
	amiga_custom_w(cputag_get_address_space(device->machine, "maincpu", ADDRESS_SPACE_PROGRAM), REG_INTREQ, (state ? 0x8000 : 0x0000) | INTENA_PORTS, 0xffff);
}


void amiga_cia_1_irq(running_device *device, int state)
{
	amiga_custom_w(cputag_get_address_space(device->machine, "maincpu", ADDRESS_SPACE_PROGRAM), REG_INTREQ, (state ? 0x8000 : 0x0000) | INTENA_EXTER, 0xffff);
}



/*************************************
 *
 *  Custom chip reset
 *
 *************************************/

static void custom_reset(running_machine *machine)
{
	int clock = cputag_get_clock(machine, "maincpu");
	UINT16	vidmode = (clock == AMIGA_68000_NTSC_CLOCK || clock == AMIGA_68EC020_NTSC_CLOCK ) ? 0x1000 : 0x0000; /* NTSC or PAL? */

	CUSTOM_REG(REG_DDFSTRT) = 0x18;
	CUSTOM_REG(REG_DDFSTOP) = 0xd8;
	CUSTOM_REG(REG_INTENA) = 0x0000;
	CUSTOM_REG(REG_VPOSR) = vidmode;
	CUSTOM_REG(REG_SERDATR) = 0x3000;

	switch (amiga_intf->chip_ram_mask)
	{
		case ANGUS_CHIP_RAM_MASK:
		case FAT_ANGUS_CHIP_RAM_MASK:
			CUSTOM_REG(REG_DENISEID) = 0x00FF;
			break;

		case ECS_CHIP_RAM_MASK:
			CUSTOM_REG(REG_VPOSR) |= 0x2000;
			CUSTOM_REG(REG_DENISEID) = 0x00FC;
			if (IS_AGA(amiga_intf))
			{
				CUSTOM_REG(REG_VPOSR) |= 0x0300;
				CUSTOM_REG(REG_DENISEID) = 0x00F8;
			}
			break;
	}
}



/*************************************
 *
 *  Custom chip register read
 *
 *************************************/

READ16_HANDLER( amiga_custom_r )
{
	UINT16 temp;

	switch (offset & 0xff)
	{
		case REG_BLTDDAT:
			return CUSTOM_REG(REG_BLTDDAT);

		case REG_DMACONR:
			return CUSTOM_REG(REG_DMACON);

		case REG_VPOSR:
			CUSTOM_REG(REG_VPOSR) &= 0xff00;
			if (IS_AGA(amiga_intf))
				CUSTOM_REG(REG_VPOSR) |= amiga_aga_gethvpos(*space->machine->primary_screen) >> 16;
			else
				CUSTOM_REG(REG_VPOSR) |= amiga_gethvpos(*space->machine->primary_screen) >> 16;
			return CUSTOM_REG(REG_VPOSR);

		case REG_VHPOSR:
			if (IS_AGA(amiga_intf))
				return amiga_aga_gethvpos(*space->machine->primary_screen) & 0xffff;
			else
				return amiga_gethvpos(*space->machine->primary_screen) & 0xffff;

		case REG_SERDATR:
			CUSTOM_REG(REG_SERDATR) &= ~0x4000;
			CUSTOM_REG(REG_SERDATR) |= (CUSTOM_REG(REG_INTREQ) & INTENA_RBF) ? 0x4000 : 0x0000;
			return CUSTOM_REG(REG_SERDATR);

		case REG_JOY0DAT:
			if (amiga_intf->joy0dat_r != NULL)
				return (*amiga_intf->joy0dat_r)(space->machine);
			return input_port_read_safe(space->machine, "JOY0DAT", 0xffff);

		case REG_JOY1DAT:
			if (amiga_intf->joy1dat_r != NULL)
				return (*amiga_intf->joy1dat_r)(space->machine);
			return input_port_read_safe(space->machine, "JOY1DAT", 0xffff);

		case REG_ADKCONR:
			return CUSTOM_REG(REG_ADKCON);

		case REG_POTGOR:
			return input_port_read_safe(space->machine, "POTGO", 0x5500);

		case REG_POT0DAT:
			return input_port_read_safe(space->machine, "POT0DAT", 0x0000);

		case REG_POT1DAT:
			return input_port_read_safe(space->machine, "POT1DAT", 0x0000);

		case REG_DSKBYTR:
			if (amiga_intf->dskbytr_r != NULL)
				return (*amiga_intf->dskbytr_r)(space->machine);
			return 0x0000;

		case REG_INTENAR:
			return CUSTOM_REG(REG_INTENA);

		case REG_INTREQR:
			return CUSTOM_REG(REG_INTREQ);

		case REG_COPJMP1:
			if (IS_AGA(amiga_intf))
				aga_copper_setpc(CUSTOM_REG_LONG(REG_COP1LCH));
			else
				copper_setpc(CUSTOM_REG_LONG(REG_COP1LCH));
			break;

		case REG_COPJMP2:
			if (IS_AGA(amiga_intf))
				aga_copper_setpc(CUSTOM_REG_LONG(REG_COP2LCH));
			else
				copper_setpc(CUSTOM_REG_LONG(REG_COP2LCH));
			break;

		case REG_CLXDAT:
			temp = CUSTOM_REG(REG_CLXDAT);
			CUSTOM_REG(REG_CLXDAT) = 0;
			return temp;

		case REG_DENISEID:
			return CUSTOM_REG(REG_DENISEID);
	}

	if (LOG_CUSTOM)
		logerror("%06X:read from custom %s\n", cpu_get_pc(space->cpu), amiga_custom_names[offset & 0xff]);

	return 0xffff;
}



/*************************************
 *
 *  Custom chip register write
 *
 *************************************/

static TIMER_CALLBACK( finish_serial_write )
{
	/* mark the transfer buffer empty */
	CUSTOM_REG(REG_SERDATR) |= 0x3000;

	/* signal an interrupt */
	amiga_custom_w(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), REG_INTREQ, 0x8000 | INTENA_TBE, 0xffff);
}


WRITE16_HANDLER( amiga_custom_w )
{
	running_device *cia_0;
	running_device *cia_1;
	UINT16 temp;
	offset &= 0xff;

	if (LOG_CUSTOM)
		logerror("%06X:write to custom %s = %04X\n", cpu_get_pc(space->cpu), amiga_custom_names[offset & 0xff], data);

	switch (offset)
	{
		case REG_BLTDDAT:	case REG_DMACONR:	case REG_VPOSR:		case REG_VHPOSR:
		case REG_DSKDATR:	case REG_JOY0DAT:	case REG_JOY1DAT:	case REG_CLXDAT:
		case REG_ADKCONR:	case REG_POT0DAT:	case REG_POT1DAT:	case REG_POTGOR:
		case REG_SERDATR:	case REG_DSKBYTR:	case REG_INTENAR:	case REG_INTREQR:
			/* read-only registers */
			break;

		case REG_DSKLEN:
			if (amiga_intf->dsklen_w != NULL)
				(*amiga_intf->dsklen_w)(space->machine, data);
			break;

		case REG_POTGO:
			if (amiga_intf->potgo_w != NULL)
				(*amiga_intf->potgo_w)(space->machine, data);
			break;

		case REG_SERDAT:
			if (amiga_intf->serdat_w != NULL)
				(*amiga_intf->serdat_w)(space->machine, data);
			CUSTOM_REG(REG_SERDATR) &= ~0x3000;
			timer_set(space->machine, amiga_get_serial_char_period(space->machine), NULL, 0, finish_serial_write);
			break;

		case REG_BLTSIZE:
			CUSTOM_REG(REG_BLTSIZE) = data;
			CUSTOM_REG(REG_BLTSIZV) = (data >> 6) & 0x3ff;
			CUSTOM_REG(REG_BLTSIZH) = data & 0x3f;
			if ( CUSTOM_REG(REG_BLTSIZV) == 0 ) CUSTOM_REG(REG_BLTSIZV) = 0x400;
			if ( CUSTOM_REG(REG_BLTSIZH) == 0 ) CUSTOM_REG(REG_BLTSIZH) = 0x40;
			blitter_setup(space);
			break;

		case REG_BLTSIZV:	/* ECS-AGA only */
			if ( IS_ECS_OR_AGA(amiga_intf) )
			{
				CUSTOM_REG(REG_BLTSIZV) = data & 0x7fff;
				if ( CUSTOM_REG(REG_BLTSIZV) == 0 ) CUSTOM_REG(REG_BLTSIZV) = 0x8000;
			}
			break;

		case REG_BLTSIZH:	/* ECS-AGA only */
			if ( IS_ECS_OR_AGA(amiga_intf) )
			{
				CUSTOM_REG(REG_BLTSIZH) = data & 0x7ff;
				if ( CUSTOM_REG(REG_BLTSIZH) == 0 ) CUSTOM_REG(REG_BLTSIZH) = 0x800;
				blitter_setup(space);
			}
			break;

		case REG_BLTCON0L:	/* ECS-AGA only */
			if ( IS_ECS_OR_AGA(amiga_intf) )
			{
				CUSTOM_REG(REG_BLTCON0) &= 0xff00;
				CUSTOM_REG(REG_BLTCON0) |= data & 0xff;
			}
			break;

		case REG_SPR0PTH:	case REG_SPR1PTH:	case REG_SPR2PTH:	case REG_SPR3PTH:
		case REG_SPR4PTH:	case REG_SPR5PTH:	case REG_SPR6PTH:	case REG_SPR7PTH:
			data &= ( amiga_intf->chip_ram_mask >> 16 );
			break;

		case REG_SPR0PTL:	case REG_SPR1PTL:	case REG_SPR2PTL:	case REG_SPR3PTL:
		case REG_SPR4PTL:	case REG_SPR5PTL:	case REG_SPR6PTL:	case REG_SPR7PTL:
			if (IS_AGA(amiga_intf))
				amiga_aga_sprite_dma_reset((offset - REG_SPR0PTL) / 2);
			else
				amiga_sprite_dma_reset((offset - REG_SPR0PTL) / 2);
			break;

		case REG_SPR0CTL:	case REG_SPR1CTL:	case REG_SPR2CTL:	case REG_SPR3CTL:
		case REG_SPR4CTL:	case REG_SPR5CTL:	case REG_SPR6CTL:	case REG_SPR7CTL:
			/* disable comparitor on writes here */
			if (IS_AGA(amiga_intf))
				amiga_aga_sprite_enable_comparitor((offset - REG_SPR0CTL) / 4, FALSE);
			else
				amiga_sprite_enable_comparitor((offset - REG_SPR0CTL) / 4, FALSE);
			break;

		case REG_SPR0DATA:	case REG_SPR1DATA:	case REG_SPR2DATA:	case REG_SPR3DATA:
		case REG_SPR4DATA:	case REG_SPR5DATA:	case REG_SPR6DATA:	case REG_SPR7DATA:
			/* enable comparitor on writes here */
			if (IS_AGA(amiga_intf))
				amiga_aga_sprite_enable_comparitor((offset - REG_SPR0DATA) / 4, TRUE);
			else
				amiga_sprite_enable_comparitor((offset - REG_SPR0DATA) / 4, TRUE);
			break;

		case REG_COP1LCH:	case REG_COP2LCH:
			data &= ( amiga_intf->chip_ram_mask >> 16 );
			break;

		case REG_COPJMP1:
			if (IS_AGA(amiga_intf))
				aga_copper_setpc(CUSTOM_REG_LONG(REG_COP1LCH));
			else
				copper_setpc(CUSTOM_REG_LONG(REG_COP1LCH));
			break;

		case REG_COPJMP2:
			if (IS_AGA(amiga_intf))
				aga_copper_setpc(CUSTOM_REG_LONG(REG_COP2LCH));
			else
				copper_setpc(CUSTOM_REG_LONG(REG_COP2LCH));
			break;

		case REG_DDFSTRT:
			/* impose hardware limits ( HRM, page 75 ) */
			data &= 0xfe;
			if (data < 0x18)
				data = 0x18;
			break;

		case REG_DDFSTOP:
			/* impose hardware limits ( HRM, page 75 ) */
			data &= 0xfe;
			if (data > 0xd8)
				data = 0xd8;
			break;

		case REG_DMACON:
			amiga_audio_update();

			/* bits BBUSY (14) and BZERO (13) are read-only */
			data &= 0x9fff;
			data = (data & 0x8000) ? (CUSTOM_REG(offset) | (data & 0x7fff)) : (CUSTOM_REG(offset) & ~(data & 0x7fff));

			/* if 'blitter-nasty' has been turned on and we have a blit pending, reschedule it */
			if ( ( data & 0x400 ) && ( CUSTOM_REG(REG_DMACON) & 0x4000 ) )
				timer_adjust_oneshot( amiga_blitter_timer, downcast<cpu_device *>(space->cpu)->cycles_to_attotime( BLITTER_NASTY_DELAY ), 0);

			break;

		case REG_INTENA:
			temp = data;

			data = (data & 0x8000) ? (CUSTOM_REG(offset) | (data & 0x7fff)) : (CUSTOM_REG(offset) & ~(data & 0x7fff));
			CUSTOM_REG(offset) = data;

			if ( temp & 0x8000  ) /* if we're enabling irq's, delay a bit */
				timer_adjust_oneshot( amiga_irq_timer, downcast<cpu_device *>(space->cpu)->cycles_to_attotime( AMIGA_IRQ_DELAY_CYCLES ), 0);
			else /* if we're disabling irq's, process right away */
				update_irqs(space->machine);
			break;

		case REG_INTREQ:
			temp = data;
			/* Update serial data line status if appropiate */
			if (!(data & 0x8000) && (data & INTENA_RBF))
				CUSTOM_REG(REG_SERDATR) &= ~0x8000;

			data = (data & 0x8000) ? (CUSTOM_REG(offset) | (data & 0x7fff)) : (CUSTOM_REG(offset) & ~(data & 0x7fff));
			cia_0 = space->machine->device("cia_0");
			cia_1 = space->machine->device("cia_1");
			if ( mos6526_irq_r( cia_0 ) ) data |= INTENA_PORTS;
			if ( mos6526_irq_r( cia_1 ) ) data |= INTENA_EXTER;
			CUSTOM_REG(offset) = data;

			if ( temp & 0x8000  ) /* if we're generating irq's, delay a bit */
				timer_adjust_oneshot( amiga_irq_timer, cputag_clocks_to_attotime(space->machine, "maincpu", AMIGA_IRQ_DELAY_CYCLES ), 0);
			else /* if we're clearing irq's, process right away */
				update_irqs(space->machine);
			break;

		case REG_ADKCON:
			amiga_audio_update();
			data = (data & 0x8000) ? (CUSTOM_REG(offset) | (data & 0x7fff)) : (CUSTOM_REG(offset) & ~(data & 0x7fff));
			break;

		case REG_AUD0LCL:	case REG_AUD0LCH:	case REG_AUD0LEN:	case REG_AUD0PER:	case REG_AUD0VOL:
		case REG_AUD1LCL:	case REG_AUD1LCH:	case REG_AUD1LEN:	case REG_AUD1PER:	case REG_AUD1VOL:
		case REG_AUD2LCL:	case REG_AUD2LCH:	case REG_AUD2LEN:	case REG_AUD2PER:	case REG_AUD2VOL:
		case REG_AUD3LCL:	case REG_AUD3LCH:	case REG_AUD3LEN:	case REG_AUD3PER:	case REG_AUD3VOL:
			amiga_audio_update();
			break;

		case REG_AUD0DAT:	case REG_AUD1DAT:	case REG_AUD2DAT:	case REG_AUD3DAT:
			amiga_audio_data_w((offset - REG_AUD0DAT) / 8, data);
			break;

		case REG_BPL1PTH:	case REG_BPL2PTH:	case REG_BPL3PTH:	case REG_BPL4PTH:
		case REG_BPL5PTH:	case REG_BPL6PTH:
			data &= ( amiga_intf->chip_ram_mask >> 16 );
			break;

		case REG_BPLCON0:
			if ((data & (BPLCON0_BPU0 | BPLCON0_BPU1 | BPLCON0_BPU2)) == (BPLCON0_BPU0 | BPLCON0_BPU1 | BPLCON0_BPU2))
			{
				/* planes go from 0 to 6, inclusive */
				logerror( "This game is doing funky planes stuff. (planes > 6)\n" );
				data &= ~BPLCON0_BPU0;
			}
			break;

		case REG_COLOR00:	case REG_COLOR01:	case REG_COLOR02:	case REG_COLOR03:
		case REG_COLOR04:	case REG_COLOR05:	case REG_COLOR06:	case REG_COLOR07:
		case REG_COLOR08:	case REG_COLOR09:	case REG_COLOR10:	case REG_COLOR11:
		case REG_COLOR12:	case REG_COLOR13:	case REG_COLOR14:	case REG_COLOR15:
		case REG_COLOR16:	case REG_COLOR17:	case REG_COLOR18:	case REG_COLOR19:
		case REG_COLOR20:	case REG_COLOR21:	case REG_COLOR22:	case REG_COLOR23:
		case REG_COLOR24:	case REG_COLOR25:	case REG_COLOR26:	case REG_COLOR27:
		case REG_COLOR28:	case REG_COLOR29:	case REG_COLOR30:	case REG_COLOR31:
			if ( IS_AGA(amiga_intf))
			{
				aga_palette_write(offset - REG_COLOR00, data);
			}
			else
			{
				data &= 0xfff;
				CUSTOM_REG(offset + 32) = (data >> 1) & 0x777;
			}
			break;
		case REG_DIWSTRT:
		case REG_DIWSTOP:
			if (IS_AGA(amiga_intf))
				aga_diwhigh_written(0);
			break;
		case REG_DIWHIGH:
			if (IS_AGA(amiga_intf))
				aga_diwhigh_written(1);
			break;

		default:
			break;
	}

	if (IS_AGA(amiga_intf))
		CUSTOM_REG(offset) = data;
	else
		if (offset <= REG_COLOR31)
			CUSTOM_REG(offset) = data;
}



/*************************************
 *
 *  Serial writes
 *
 *************************************/

void amiga_serial_in_w(running_machine *machine, UINT16 data)
{
	const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);
	int mask = (CUSTOM_REG(REG_SERPER) & 0x8000) ? 0x1ff : 0xff;

	/* copy the data to the low 8 bits of SERDATR and set RBF */
	CUSTOM_REG(REG_SERDATR) &= ~0x3ff;
	CUSTOM_REG(REG_SERDATR) |= (data & mask) | (mask + 1) | 0x4000;

	/* set overrun if we weren't cleared */
	if (CUSTOM_REG(REG_INTREQ) & INTENA_RBF)
	{
		mame_printf_debug("Serial data overflow\n");
		CUSTOM_REG(REG_SERDATR) |= 0x8000;
	}

	/* signal an interrupt */
	amiga_custom_w(space, REG_INTREQ, 0x8000 | INTENA_RBF, 0xffff);
}


attotime amiga_get_serial_char_period(running_machine *machine)
{
	UINT32 divisor = (CUSTOM_REG(REG_SERPER) & 0x7fff) + 1;
	UINT32 baud = cputag_get_clock(machine, "maincpu") / 2 / divisor;
	UINT32 numbits = 2 + ((CUSTOM_REG(REG_SERPER) & 0x8000) ? 9 : 8);
	return attotime_mul(ATTOTIME_IN_HZ(baud), numbits);
}



/*************************************
 *
 *  Autoconfig registration
 *
 *************************************/

void amiga_add_autoconfig(running_machine *machine, const amiga_autoconfig_device *device)
{
	autoconfig_device *dev, **d;

	/* validate the data */
	assert_always(machine->phase() == MACHINE_PHASE_INIT, "Can only call amiga_add_autoconfig at init time!");
	assert_always((device->size & (device->size - 1)) == 0, "device->size must be power of 2!");

	/* allocate memory and link it in at the end of the list */
	dev = auto_alloc(machine, autoconfig_device);
	dev->next = NULL;
	for (d = &autoconfig_list; *d; d = &(*d)->next) ;
	*d = dev;

	/* fill in the data */
	dev->device = *device;
	dev->base = 0;
}



/*************************************
 *
 *  Autoconfig reset
 *
 *************************************/

static void autoconfig_reset(running_machine *machine)
{
	autoconfig_device *dev;

	/* uninstall any installed devices */
	for (dev = autoconfig_list; dev; dev = dev->next)
		if (dev->base && dev->device.uninstall)
		{
			(*dev->device.uninstall)(machine, dev->base);
			dev->base = 0;
		}

	/* reset the current autoconfig */
	cur_autoconfig = autoconfig_list;
}



/*************************************
 *
 *  Autoconfig space read
 *
 *************************************/

READ16_HANDLER( amiga_autoconfig_r )
{
	UINT8 byte;
	int i;

	/* if nothing present, just return */
	if (!cur_autoconfig)
	{
		logerror("autoconfig_r(%02X) but no device selected\n", offset);
		return 0;
	}

	/* switch off of the base offset */
	switch (offset/2)
	{
		/*
           00/02        1  1  x  x     x  0  0  0 = 8 Megabytes
                              ^  ^     ^  0  0  1 = 64 Kbytes
                              |  |     |  0  1  0 = 128 Kbytes
                              |  |     |  0  1  1 = 256 Kbytes
                              |  |     |  1  0  0 = 1 Megabyte
                              |  |     |  1  1  0 = 2 Megabytes
                              |  |     |  1  1  1 = 4 Megabytes
                              |  |     |
                              |  |     `-- 1 = multiple devices on this card
                              |  `-------- 1 = ROM vector offset is valid
                              `----------- 1 = link into free memory list
        */
		case 0x00/4:
			byte = 0xc0;
			if (cur_autoconfig->device.link_memory)
				byte |= 0x20;
			if (cur_autoconfig->device.rom_vector_valid)
				byte |= 0x10;
			if (cur_autoconfig->device.multi_device)
				byte |= 0x08;
			for (i = 0; i < 8; i++)
				if (cur_autoconfig->device.size & (1 << i))
					break;
			byte |= (i + 1) & 7;
			break;

		/*
           04/06          product number (all bits inverted)
        */
		case 0x04/4:
			byte = ~cur_autoconfig->device.product_number;
			break;

		/*
           08/0a        x  x  1  1     1  1  1  1
                        ^  ^
                        |  |
                        |  `-- 1 = this board can be shut up
                        `----- 0 = prefer 8 Meg address space
        */
		case 0x08/4:
			byte = 0x3f;
			if (!cur_autoconfig->device.prefer_8meg)
				byte |= 0x80;
			if (cur_autoconfig->device.can_shutup)
				byte |= 0x40;
			break;

		/*
           10/12         manufacturers number (high byte, all inverted)
           14/16                  ''          (low byte, all inverted)
        */
		case 0x10/4:
			byte = ~cur_autoconfig->device.mfr_number >> 8;
			break;

		case 0x14/4:
			byte = ~cur_autoconfig->device.mfr_number >> 0;
			break;

		/*
           18/1a         optional serial number (all bits inverted) byte0
           1c/1e                              ''                    byte1
           20/22                              ''                    byte2
           24/26                              ''                    byte3
        */
		case 0x18/4:
			byte = ~cur_autoconfig->device.serial_number >> 24;
			break;

		case 0x1c/4:
			byte = ~cur_autoconfig->device.serial_number >> 16;
			break;

		case 0x20/4:
			byte = ~cur_autoconfig->device.serial_number >> 8;
			break;

		case 0x24/4:
			byte = ~cur_autoconfig->device.serial_number >> 0;
			break;

		/*
           28/2a         optional ROM vector offset (all bits inverted) high byte
           2c/2e                              ''                        low byte
        */
		case 0x28/4:
			byte = ~cur_autoconfig->device.rom_vector >> 8;
			break;

		case 0x2c/4:
			byte = ~cur_autoconfig->device.rom_vector >> 0;
			break;

		/*
           40/42   optional interrupt control and status register
        */
		case 0x40/4:
			byte = 0x00;
			if (cur_autoconfig->device.int_control_r)
				byte = (*cur_autoconfig->device.int_control_r)(space->machine);
			break;

		default:
			byte = 0xff;
			break;
	}

	/* return the appropriate nibble */
	logerror("autoconfig_r(%02X) = %04X\n", offset, (offset & 1) ? ((byte << 12) | 0xfff) : ((byte << 8) | 0xfff));
	return (offset & 1) ? ((byte << 12) | 0xfff) : ((byte << 8) | 0xfff);
}



/*************************************
 *
 *  Autoconfig space write
 *
 *************************************/

WRITE16_HANDLER( amiga_autoconfig_w )
{
	int move_to_next = FALSE;

	logerror("autoconfig_w(%02X) = %04X & %04X\n", offset, data, mem_mask);

	/* if no current device, bail */
	if (!cur_autoconfig || !ACCESSING_BITS_8_15)
		return;

	/* switch off of the base offset */
	switch (offset/2)
	{
		/*
           48/4a        write-only register for base address (A23-A16)
        */
		case 0x48/4:
			if ((offset & 1) == 0)
				cur_autoconfig->base = (cur_autoconfig->base & ~0xf00000) | ((data & 0xf000) << 8);
			else
				cur_autoconfig->base = (cur_autoconfig->base & ~0x0f0000) | ((data & 0xf000) << 4);
			move_to_next = TRUE;
			break;

		/*
           4c/4e        optional write-only 'shutup' trigger
        */
		case 0x4c/4:
			cur_autoconfig->base = 0;
			move_to_next = TRUE;
			break;
	}

	/* install and move to the next device if requested */
	if (move_to_next && (offset & 1) == 0)
	{
		logerror("Install to %06X\n", cur_autoconfig->base);
		if (cur_autoconfig->base && cur_autoconfig->device.install)
			(*cur_autoconfig->device.install)(space->machine, cur_autoconfig->base);
		cur_autoconfig = cur_autoconfig->next;
	}
}



/*************************************
 *
 *  Get interface
 *
 *************************************/

const amiga_machine_interface *amiga_get_interface(void)
{
	return amiga_intf;
}
