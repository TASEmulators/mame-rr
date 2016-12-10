/*************************************************************************

    Driver for Midway Zeus games

    driver by Aaron Giles

    Games supported:
        * Invasion - The Abductors
        * Mortal Kombat 4
        * Cruis'n Exotica
        * The Grid

    Known bugs:
        * not done yet

    To Do
        * make version 1.0 of MK4 work

According to a Midway service bulletin
As of 2/12/2001 the lastest software levels:

Game Title       Level  Released
----------------------------------
Cruis'n Exotica  v2.4   08/23/2000
Invasion         v5.0   12/14/1999
The Grid         v1.2   10/18/2000

**************************************************************************/

#include "emu.h"
#include "cpu/tms32031/tms32031.h"
#include "cpu/tms34010/tms34010.h"
#include "cpu/adsp2100/adsp2100.h"
#include "cpu/pic16c5x/pic16c5x.h"
#include "includes/midzeus.h"
#include "machine/midwayic.h"
#include "machine/timekpr.h"
#include "audio/dcs.h"

#include "crusnexo.lh"


#define CPU_CLOCK		XTAL_60MHz

#define BEAM_DY			3
#define BEAM_DX			3
#define BEAM_XOFFS		40		/* table in the code indicates an offset of 20 with a beam height of 7 */

static UINT32			gun_control;
static UINT8			gun_irq_state;
static emu_timer *		gun_timer[2];
static INT32			gun_x[2], gun_y[2];

static UINT8			crusnexo_leds_select;
static UINT8			keypad_select;
static UINT8			bitlatch[10];

static UINT32 *ram_base;
static UINT32 *zpram;
static size_t zpram_size;
static UINT8 cmos_protected;

static UINT32 *linkram;

static emu_timer *timer[2];

static UINT32 *tms32031_control;


static TIMER_CALLBACK( invasn_gun_callback );



/*************************************
 *
 *  Machine init
 *
 *************************************/

static MACHINE_START( midzeus )
{
	timer[0] = timer_alloc(machine, NULL, NULL);
	timer[1] = timer_alloc(machine, NULL, NULL);

	gun_timer[0] = timer_alloc(machine, invasn_gun_callback, NULL);
	gun_timer[1] = timer_alloc(machine, invasn_gun_callback, NULL);

	state_save_register_global(machine, gun_control);
	state_save_register_global(machine, gun_irq_state);
	state_save_register_global_array(machine, gun_x);
	state_save_register_global_array(machine, gun_y);
	state_save_register_global(machine, crusnexo_leds_select);
	state_save_register_global(machine, keypad_select);
}


static MACHINE_RESET( midzeus )
{
	memcpy(ram_base, memory_region(machine, "user1"), 0x40000*4);
	*ram_base <<= 1;
	machine->device("maincpu")->reset();

	cmos_protected = TRUE;
}



/*************************************
 *
 *  Display interrupt generation
 *
 *************************************/

static TIMER_CALLBACK( display_irq_off )
{
	cputag_set_input_line(machine, "maincpu", 0, CLEAR_LINE);
}

static INTERRUPT_GEN( display_irq )
{
	cpu_set_input_line(device, 0, ASSERT_LINE);
	timer_set(device->machine, ATTOTIME_IN_HZ(30000000), NULL, 0, display_irq_off);
}



/*************************************
 *
 *  CMOS access (Zeus only)
 *
 *************************************/

static WRITE32_HANDLER( cmos_w )
{
	if (bitlatch[2] && !cmos_protected)
		COMBINE_DATA(&space->machine->generic.nvram.u32[offset]);
	else
		logerror("%06X:timekeeper_w with bitlatch[2] = %d, cmos_protected = %d\n", cpu_get_pc(space->cpu), bitlatch[2], cmos_protected);
	cmos_protected = TRUE;
}


static READ32_HANDLER( cmos_r )
{
	return space->machine->generic.nvram.u32[offset] | 0xffffff00;
}


static WRITE32_HANDLER( cmos_protect_w )
{
	cmos_protected = FALSE;
}



/*************************************
 *
 *  Timekeeper and ZPRAM access
 *  (Zeus 2 only)
 *
 *************************************/

static READ32_DEVICE_HANDLER( zeus2_timekeeper_r )
{
	return timekeeper_r(device, offset) | 0xffffff00;
}


static WRITE32_DEVICE_HANDLER( zeus2_timekeeper_w )
{
	if (bitlatch[2] && !cmos_protected)
		timekeeper_w(device, offset, data);
	else
		logerror("%s:zeus2_timekeeper_w with bitlatch[2] = %d, cmos_protected = %d\n", cpuexec_describe_context(device->machine), bitlatch[2], cmos_protected);
	cmos_protected = TRUE;
}


static READ32_HANDLER( zpram_r )
{
	return zpram[offset] | 0xffffff00;
}


static WRITE32_HANDLER( zpram_w )
{
	if (bitlatch[2])
		COMBINE_DATA(&zpram[offset]);
	else
		logerror("%06X:zpram_w with bitlatch[2] = %d\n", cpu_get_pc(space->cpu), bitlatch[2]);
}



/*************************************
 *
 *  NVRAM handler (Zeus 2 only)
 *
 *************************************/

static NVRAM_HANDLER( midzeus2 )
{
	if (read_or_write)
		mame_fwrite(file, zpram, zpram_size);
	else if (file)
		mame_fread(file, zpram, zpram_size);
	else
		memset(zpram, 0xff, zpram_size);
}



/*************************************
 *
 *  Miscellaneous bit latches
 *
 *************************************/

static READ32_HANDLER( bitlatches_r )
{
	switch (offset)
	{
		/* unknown purpose; two bits are apparently used */
		case 1:
			return bitlatch[offset] | ~3;

		/* CMOS/ZPRAM extra enable latch; only low bit is used */
		case 2:
			return bitlatch[offset] | ~1;

		/* unknown purpose; mk4/invasn/thegrid read at startup; invasn freaks if it is 1 at startup */
		/* only low bit is used */
		case 3:
			return bitlatch[offset] | ~1;

		/* ROM bank selection on Zeus 2; two bits are used */
		case 5:
			return bitlatch[offset] | ~3;

		/* unknown purpose; crusnexo reads at startup: if (val & 0xf0) == 0xa0 it affects */
		/* how the Zeus is used (reg 0x5d is set to 0x54580006) */
		/* thegrid does the same, writing either 0xD4580006 or 0xC4180006 depending */
		/* this is the value reported as DISK JR ASIC version in thegrid startup test */
		case 6:
			return 0xa0 | ~0xff;

		/* unknown purpose */
		default:
			logerror("%06X:bitlatches_r(%X)\n", cpu_get_pc(space->cpu), offset);
			break;
	}
	return ~0;
}


static WRITE32_HANDLER( bitlatches_w )
{
	UINT32 oldval = bitlatch[offset];
	bitlatch[offset] = data;

	switch (offset)
	{
		/* unknown purpose */
		default:
			if (oldval ^ data)
				logerror("%06X:bitlatches_w(%X) = %X\n", cpu_get_pc(space->cpu), offset, data);
			break;

		/* unknown purpose; crusnexo toggles this between 0 and 1 every 20 frames; thegrid writes 1 */
		case 0:
			if (data != 0 && data != 1)
				logerror("%06X:bitlatches_w(%X) = %X (unexpected)\n", cpu_get_pc(space->cpu), offset, data);
			break;

		/* unknown purpose; mk4/invasn write 1 here at initialization; crusnexo/thegrid write 3 */
		case 1:
			if (data != 1 && data != 3)
				logerror("%06X:bitlatches_w(%X) = %X (unexpected)\n", cpu_get_pc(space->cpu), offset, data);
			break;

		/* CMOS/ZPRAM extra enable latch; only low bit is used */
		case 2:
			break;

		/* unknown purpose; invasn writes 2 here at startup */
		case 4:
			if (data != 2)
				logerror("%06X:bitlatches_w(%X) = %X (unexpected)\n", cpu_get_pc(space->cpu), offset, data);
			break;

		/* ROM bank selection on Zeus 2 */
		case 5:
			memory_set_bank(space->machine, "bank1", bitlatch[offset] & 3);
			break;

		/* unknown purpose; crusnexo/thegrid write 1 at startup */
		case 7:
			if (data != 1)
				logerror("%06X:bitlatches_w(%X) = %X (unexpected)\n", cpu_get_pc(space->cpu), offset, data);
			break;

		/* unknown purpose; crusnexo writes 4 at startup; thegrid writes 6 */
		case 8:
			if (data != 4 && data != 6)
				logerror("%06X:bitlatches_w(%X) = %X (unexpected)\n", cpu_get_pc(space->cpu), offset, data);
			break;

		/* unknown purpose; thegrid writes 1 at startup */
		case 9:
			if (data != 1)
				logerror("%06X:bitlatches_w(%X) = %X (unexpected)\n", cpu_get_pc(space->cpu), offset, data);
			break;
	}
}



/*************************************
 *
 *  7-segment LED controls
 *
 *************************************/

static READ32_HANDLER( crusnexo_leds_r )
{
	/* reads appear to just be for synchronization */
	return ~0;
}


static WRITE32_HANDLER( crusnexo_leds_w )
{
	int bit, led;

	switch (offset)
	{
		case 0:	/* unknown purpose */
			break;

		case 1:	/* controls lamps */
			for (bit = 0; bit < 8; bit++)
				output_set_lamp_value(bit, (data >> bit) & 1);
			break;

		case 2:	/* sets state of selected LEDs */

			/* selection bits 4-6 select the 3 7-segment LEDs */
			for (bit = 4; bit < 7; bit++)
				if ((crusnexo_leds_select & (1 << bit)) == 0)
					output_set_digit_value(bit, ~data & 0xff);

			/* selection bits 0-2 select the tachometer LEDs */
			for (bit = 0; bit < 3; bit++)
				if ((crusnexo_leds_select & (1 << bit)) == 0)
					for (led = 0; led < 8; led++)
						output_set_led_value(bit * 8 + led, (~data >> led) & 1);
			break;

		case 3:	/* selects which set of LEDs we are addressing */
			crusnexo_leds_select = data;
			break;
	}
}


/*************************************
 *
 *  Link controller access (Zeus 2 only)
 *
 *************************************/

// read 8d0003, check bit 1, skip some stuff if 0
// write junk to 9e0000

static READ32_HANDLER( linkram_r )
{
	logerror("%06X:unknown_8a000_r(%02X)\n", cpu_get_pc(space->cpu), offset);
	if (offset == 0)
		return 0x30313042;
	else if (offset == 0x3c)
		return 0xffffffff;
	return linkram[offset];
}

static WRITE32_HANDLER( linkram_w )
{
	logerror("%06X:unknown_8a000_w(%02X) = %08X\n", cpu_get_pc(space->cpu),  offset, data);
	COMBINE_DATA(&linkram[offset]);
}



/*************************************
 *
 *  TMS32031 I/O accesses
 *
 *************************************/

static READ32_HANDLER( tms32031_control_r )
{
	/* watch for accesses to the timers */
	if (offset == 0x24 || offset == 0x34)
	{
		/* timer is clocked at 100ns */
		int which = (offset >> 4) & 1;
		INT32 result = attotime_to_double(attotime_mul(timer_timeelapsed(timer[which]), 10000000));
		return result;
	}

	/* log anything else except the memory control register */
	if (offset != 0x64)
		logerror("%06X:tms32031_control_r(%02X)\n", cpu_get_pc(space->cpu), offset);

	return tms32031_control[offset];
}


static WRITE32_HANDLER( tms32031_control_w )
{
	COMBINE_DATA(&tms32031_control[offset]);

	/* ignore changes to the memory control register */
	if (offset == 0x64)
		;

	/* watch for accesses to the timers */
	else if (offset == 0x20 || offset == 0x30)
	{
		int which = (offset >> 4) & 1;
		if (data & 0x40)
			timer_adjust_oneshot(timer[which], attotime_never, 0);
	}
	else
		logerror("%06X:tms32031_control_w(%02X) = %08X\n", cpu_get_pc(space->cpu), offset, data);
}



/*************************************
 *
 *  49-way joystick handling
 *
 *************************************/

static CUSTOM_INPUT( custom_49way_r )
{
	static const UINT8 translate49[7] = { 0x8, 0xc, 0xe, 0xf, 0x3, 0x1, 0x0 };
	const char *namex = (const char *)param;
	const char *namey = namex + strlen(namex) + 1;
	return (translate49[input_port_read(field->port->machine, namey) >> 4] << 4) | translate49[input_port_read(field->port->machine, namex) >> 4];
}


static WRITE32_HANDLER( keypad_select_w )
{
	if (offset == 1)
		keypad_select = data;
}


static CUSTOM_INPUT( keypad_r )
{
	UINT32 bits = input_port_read(field->port->machine, (const char *)param);
	UINT8 select = keypad_select;
	while ((select & 1) != 0)
	{
		select >>= 1;
		bits >>= 4;
	}
	return bits;
}



/*************************************
 *
 *  Analog input handling
 *
 *************************************/

static READ32_HANDLER( analog_r )
{
	static const char * const tags[] = { "ANALOG0", "ANALOG1", "ANALOG2", "ANALOG3" };
	if (offset < 8 || offset > 11)
		logerror("%06X:analog_r(%X)\n", cpu_get_pc(space->cpu), offset);
	return input_port_read(space->machine, tags[offset & 3]);
}


static WRITE32_HANDLER( analog_w )
{
	/* 16 writes to the location before a read */
}



/*************************************
 *
 *  Lightgun handling
 *
 *************************************/

static void update_gun_irq(running_machine *machine)
{
	/* low 2 bits of gun_control seem to enable IRQs */
	if (gun_irq_state & gun_control & 0x03)
		cputag_set_input_line(machine, "maincpu", 3, ASSERT_LINE);
	else
		cputag_set_input_line(machine, "maincpu", 3, CLEAR_LINE);
}


static TIMER_CALLBACK( invasn_gun_callback )
{
	int player = param;
	int beamy = machine->primary_screen->vpos();

	/* set the appropriate IRQ in the internal gun control and update */
	gun_irq_state |= 0x01 << player;
	update_gun_irq(machine);

	/* generate another interrupt on the next scanline while we are within the BEAM_DY */
	beamy++;
	if (beamy <= machine->primary_screen->visible_area().max_y && beamy <= gun_y[player] + BEAM_DY)
		timer_adjust_oneshot(gun_timer[player], machine->primary_screen->time_until_pos(beamy, MAX(0, gun_x[player] - BEAM_DX)), player);
}


static WRITE32_HANDLER( invasn_gun_w )
{
	UINT32 old_control = gun_control;
	int player;

	COMBINE_DATA(&gun_control);

	/* bits 0-1 enable IRQs (?) */
	/* bits 2-3 reset IRQ states */
	gun_irq_state &= ~((gun_control >> 2) & 3);
	update_gun_irq(space->machine);

	for (player = 0; player < 2; player++)
	{
		UINT8 pmask = 0x04 << player;
		if (((old_control ^ gun_control) & pmask) != 0 && (gun_control & pmask) == 0)
		{
			const rectangle &visarea = space->machine->primary_screen->visible_area();
			static const char *const names[2][2] =
			{
				{ "GUNX1", "GUNY1" },
				{ "GUNX2", "GUNY2" }
			};
			gun_x[player] = input_port_read(space->machine, names[player][0]) * (visarea.max_x + 1 - visarea.min_x) / 255 + visarea.min_x + BEAM_XOFFS;
			gun_y[player] = input_port_read(space->machine, names[player][1]) * (visarea.max_y + 1 - visarea.min_y) / 255 + visarea.min_y;
			timer_adjust_oneshot(gun_timer[player], space->machine->primary_screen->time_until_pos(MAX(0, gun_y[player] - BEAM_DY), MAX(0, gun_x[player] - BEAM_DX)), player);
		}
	}
}


static READ32_HANDLER( invasn_gun_r )
{
	int beamx = space->machine->primary_screen->hpos();
	int beamy = space->machine->primary_screen->vpos();
	UINT32 result = 0xffff;
	int player;

	for (player = 0; player < 2; player++)
	{
		int diffx = beamx - gun_x[player];
		int diffy = beamy - gun_y[player];
		if (diffx >= -BEAM_DX && diffx <= BEAM_DX && diffy >= -BEAM_DY && diffy <= BEAM_DY)
			result ^= 0x1000 << player;
	}
	return result;
}



/*************************************
 *
 *  Memory maps
 *
 *************************************/

static ADDRESS_MAP_START( zeus_map, ADDRESS_SPACE_PROGRAM, 32 )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x03ffff) AM_RAM AM_BASE(&ram_base)
	AM_RANGE(0x400000, 0x41ffff) AM_RAM
	AM_RANGE(0x808000, 0x80807f) AM_READWRITE(tms32031_control_r, tms32031_control_w) AM_BASE(&tms32031_control)
	AM_RANGE(0x880000, 0x8803ff) AM_READWRITE(zeus_r, zeus_w) AM_BASE(&zeusbase)
	AM_RANGE(0x8d0000, 0x8d0004) AM_READWRITE(bitlatches_r, bitlatches_w)
	AM_RANGE(0x990000, 0x99000f) AM_READWRITE(midway_ioasic_r, midway_ioasic_w)
	AM_RANGE(0x9e0000, 0x9e0000) AM_WRITENOP		// watchdog?
	AM_RANGE(0x9f0000, 0x9f7fff) AM_READWRITE(cmos_r, cmos_w) AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE(0x9f8000, 0x9f8000) AM_WRITE(cmos_protect_w)
	AM_RANGE(0xa00000, 0xffffff) AM_ROM AM_REGION("user1", 0)
ADDRESS_MAP_END


static ADDRESS_MAP_START( zeus2_map, ADDRESS_SPACE_PROGRAM, 32 )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x03ffff) AM_RAM AM_BASE(&ram_base)
	AM_RANGE(0x400000, 0x43ffff) AM_RAM
	AM_RANGE(0x808000, 0x80807f) AM_READWRITE(tms32031_control_r, tms32031_control_w) AM_BASE(&tms32031_control)
	AM_RANGE(0x880000, 0x88007f) AM_READWRITE(zeus2_r, zeus2_w) AM_BASE(&zeusbase)
	AM_RANGE(0x8a0000, 0x8a003f) AM_READWRITE(linkram_r, linkram_w) AM_BASE(&linkram)
	AM_RANGE(0x8d0000, 0x8d000a) AM_READWRITE(bitlatches_r, bitlatches_w)
	AM_RANGE(0x900000, 0x91ffff) AM_READWRITE(zpram_r, zpram_w) AM_BASE(&zpram) AM_SIZE(&zpram_size) AM_MIRROR(0x020000)
	AM_RANGE(0x990000, 0x99000f) AM_READWRITE(midway_ioasic_r, midway_ioasic_w)
	AM_RANGE(0x9c0000, 0x9c000f) AM_READWRITE(analog_r, analog_w)
	AM_RANGE(0x9e0000, 0x9e0000) AM_WRITENOP		// watchdog?
	AM_RANGE(0x9f0000, 0x9f7fff) AM_DEVREADWRITE("m48t35", zeus2_timekeeper_r, zeus2_timekeeper_w)
	AM_RANGE(0x9f8000, 0x9f8000) AM_WRITE(cmos_protect_w)
	AM_RANGE(0xa00000, 0xbfffff) AM_ROM AM_REGION("user1", 0)
	AM_RANGE(0xc00000, 0xffffff) AM_ROMBANK("bank1") AM_REGION("user2", 0)
ADDRESS_MAP_END

/*

    mk4:

        writes to 9D0000: 00000009, FFFFFFFF
        reads from 9D0000
        writes to 9D0001: 00000000
        writes to 9D0003: 00000374
        writes to 9D0005: 00000000

    crusnexo:

        reads from 8A0000

        writes to 9D0000: 00000000, 00000008, 00000009, FFFFFFFF
        reads from 9D0000
        writes to 9D0001: 00000000, 00000004, 00000204
        writes to 9D0003: 00000374
            -- hard coded to $374 at startup
        writes to 9D0004: 0000000F
            -- hard coded to $F at startup

        writes to 9E0008: 00000000

        writes to 9E8000: 00810081

    thegrid:

        writes to 9D0000: 00000008, 00000009, 0000008D
        writes to 9D0001: 00000000, 00000004, 00000204
        writes to 9D0003: 00000354
        reads from 9D0003
        writes to 9D0004: FFFFFFFF
        writes to 9D0005: 00000000

        writes to 9E8000: 00810081
*/


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( mk4 )
	PORT_START("DIPS")		/* DS1 */
	PORT_DIPNAME( 0x0001, 0x0001, "Coinage Source" )
	PORT_DIPSETTING(      0x0001, "Dipswitch" )
	PORT_DIPSETTING(      0x0000, "CMOS" )
	PORT_DIPNAME( 0x003e, 0x003e, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0x003e, "USA-1" )
	PORT_DIPSETTING(      0x003c, "USA-2" )
	PORT_DIPSETTING(      0x003a, "USA-3" )
	PORT_DIPSETTING(      0x0038, "USA-4" )
	PORT_DIPSETTING(      0x0034, "USA-9" )
	PORT_DIPSETTING(      0x0032, "USA-11" )
	PORT_DIPSETTING(      0x0036, "USA-ECA" )
	PORT_DIPSETTING(      0x002e, "German-1" )
	PORT_DIPSETTING(      0x002c, "German-2" )
	PORT_DIPSETTING(      0x002a, "German-3" )
	PORT_DIPSETTING(      0x0028, "German-4" )
	PORT_DIPSETTING(      0x0026, "German-ECA" )
	PORT_DIPSETTING(      0x001e, "French-1" )
	PORT_DIPSETTING(      0x001c, "French-2" )
	PORT_DIPSETTING(      0x001a, "French-3" )
	PORT_DIPSETTING(      0x0018, "French-4" )
	PORT_DIPSETTING(      0x0016, "French-ECA" )
	PORT_DIPSETTING(      0x0030, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )	/* Manual lists this dip as Unused */
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Test Switch" )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "Fatalities" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, "Blood" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) ) /* Manual states that switches 3-7 are Unused */
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT ) /* Slam Switch */
	PORT_SERVICE_NO_TOGGLE( 0x0010, IP_ACTIVE_LOW )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0x6000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BILL1 )	/* Bill */

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0xff80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( invasn )
	PORT_START("DIPS")		/* DS1 */
	PORT_DIPNAME( 0x0001, 0x0001, "Coinage Source" )
	PORT_DIPSETTING(      0x0001, "Dipswitch" )
	PORT_DIPSETTING(      0x0000, "CMOS" )
	PORT_DIPNAME( 0x003e, 0x003e, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0x003e, "USA-1" )
	PORT_DIPSETTING(      0x003c, "USA-2" )
	PORT_DIPSETTING(      0x003a, "USA-3" )
	PORT_DIPSETTING(      0x0038, "USA-4" )
	PORT_DIPSETTING(      0x0034, "USA-9" )
	PORT_DIPSETTING(      0x0032, "USA-10" )
	PORT_DIPSETTING(      0x0036, "USA-ECA" )
	PORT_DIPSETTING(      0x002e, "German-1" )
	PORT_DIPSETTING(      0x002c, "German-2" )
	PORT_DIPSETTING(      0x002a, "German-3" )
	PORT_DIPSETTING(      0x0028, "German-4" )
	PORT_DIPSETTING(      0x0024, "German-5" )
	PORT_DIPSETTING(      0x0026, "German-ECA" )
	PORT_DIPSETTING(      0x001e, "French-1" )
	PORT_DIPSETTING(      0x001c, "French-2" )
	PORT_DIPSETTING(      0x001a, "French-3" )
	PORT_DIPSETTING(      0x0018, "French-4" )
	PORT_DIPSETTING(      0x0014, "French-11" )
	PORT_DIPSETTING(      0x0012, "French-12" )
	PORT_DIPSETTING(      0x0016, "French-ECA" )
	PORT_DIPSETTING(      0x0030, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Flip Y" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Test Switch" )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "Mirrored Display" )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, "Show Blood" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT ) /* Slam Switch */
	PORT_SERVICE_NO_TOGGLE( 0x0010, IP_ACTIVE_LOW )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0x6000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BILL1 )	/* Bill */

	PORT_START("IN1")
	PORT_BIT( 0x000f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x00e0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0f00, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0xe000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("GUNX1")		/* fake analog X */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10)

	PORT_START("GUNY1")		/* fake analog Y */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(10)

	PORT_START("GUNX2")		/* fake analog X */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("GUNY2")		/* fake analog Y */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(10) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( crusnexo )
	PORT_START("DIPS")		/* DS1 */
	PORT_DIPNAME( 0x001f, 0x001f, "Country Code" )
	PORT_DIPSETTING(      0x001f, DEF_STR( USA ) )
	PORT_DIPSETTING(      0x001e, "Germany" )
	PORT_DIPSETTING(      0x001d, "France" )
	PORT_DIPSETTING(      0x001c, "Canada" )
	PORT_DIPSETTING(      0x001b, "Switzerland" )
	PORT_DIPSETTING(      0x001a, "Italy" )
	PORT_DIPSETTING(      0x0019, "UK" )
	PORT_DIPSETTING(      0x0018, "Spain" )
	PORT_DIPSETTING(      0x0017, "Austrilia" )
	PORT_DIPSETTING(      0x0016, DEF_STR( Japan ) )
	PORT_DIPSETTING(      0x0015, "Taiwan" )
	PORT_DIPSETTING(      0x0014, "Austria" )
	PORT_DIPSETTING(      0x0013, "Belgium" )
	PORT_DIPSETTING(      0x000f, "Sweden" )
	PORT_DIPSETTING(      0x000e, "Findland" )
	PORT_DIPSETTING(      0x000d, "Netherlands" )
	PORT_DIPSETTING(      0x000c, "Norway" )
	PORT_DIPSETTING(      0x000b, "Denmark" )
	PORT_DIPSETTING(      0x000a, "Hungary" )
	PORT_DIPSETTING(      0x0008, "General" )
	PORT_DIPNAME( 0x0060, 0x0060, "Coin Mode" )
	PORT_DIPSETTING(      0x0060, "Mode 1" ) /* USA1/GER1/FRA1/SPN1/AUSTRIA1/GEN1/CAN1/SWI1/ITL1/JPN1/TWN1/BLGN1/NTHRLND1/FNLD1/NRWY1/DNMK1/HUN1 */
	PORT_DIPSETTING(      0x0040, "Mode 2" ) /* USA3/GER1/FRA1/SPN1/AUSTRIA1/GEN3/CAN2/SWI2/ITL2/JPN2/TWN2/BLGN2/NTHRLND2 */
	PORT_DIPSETTING(      0x0020, "Mode 3" ) /* USA7/GER1/FRA1/SPN1/AUSTRIA1/GEN5/CAN3/SWI3/ITL3/JPN3/TWN3/BLGN3 */
	PORT_DIPSETTING(      0x0000, "Mode 4" ) /* USA8/GER1/FRA1/SPN1/AUSTRIA1/GEN7 */
	PORT_DIPNAME( 0x0080, 0x0080, "Test Switch" )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "Game Type" )	/* Manual states "*DIP 1, Switch 1 MUST be set */
	PORT_DIPSETTING(      0x0100, "Dedicated" )	/*   to OFF position for proper operation" */
	PORT_DIPSETTING(      0x0000, "Kit" )
	PORT_DIPNAME( 0x0200, 0x0200, "Seat Motion" )	/* For dedicated Sit Down models with Motion Seat */
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(      0x0400, "Stand Up" )
	PORT_DIPSETTING(      0x0000, "Sit Down" )
	PORT_DIPNAME( 0x0800, 0x0800, "Wheel Invert" )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, "ROM Configuration" )	/* Manual lists this dip as Unused */
	PORT_DIPSETTING(      0x1000, "32M ROM Normal" )
	PORT_DIPSETTING(      0x0000, "16M ROM Split Active" )
	PORT_DIPNAME( 0x2000, 0x2000, "Link" )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0xc000, 0xc000, "Linking I.D.")
	PORT_DIPSETTING(      0xc000, "Master #1" )
	PORT_DIPSETTING(      0x8000, "Slave #2" )
	PORT_DIPSETTING(      0x4000, "Slave #3" )
	PORT_DIPSETTING(      0x0000, "Slave #4" )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT ) /* Slam Switch */
	PORT_SERVICE_NO_TOGGLE( 0x0010, IP_ACTIVE_LOW )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0x6000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BILL1 )

	PORT_START("IN1")	/* Listed "names" are via the manual's "JAMMA" pinout sheet" */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )							/* Not Used */
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Radio")		/* Radio Switch */
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )							/* Not Used */
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )							/* Not Used */
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("View 1")		/* View 1 */
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("View 2")		/* View 2 */
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("View 3")		/* View 3 */
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("View 4")		/* View 4 */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("1st Gear")	/* Gear 1 */
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("2nd Gear")	/* Gear 2 */
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("3rd Gear")	/* Gear 3 */
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("4th Gear")	/* Gear 4 */
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )							/* Not Used */
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )							/* Not Used */
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )							/* Not Used */
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x0007, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_CUSTOM( keypad_r, "KEYPAD" )
	PORT_BIT( 0xfff8, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEYPAD")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_NAME("Keypad 3") PORT_CODE(KEYCODE_3_PAD)	/* keypad 3 */
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_NAME("Keypad 1") PORT_CODE(KEYCODE_1_PAD)	/* keypad 1 */
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_NAME("Keypad 2") PORT_CODE(KEYCODE_2_PAD)	/* keypad 2 */
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_NAME("Keypad 6") PORT_CODE(KEYCODE_6_PAD)	/* keypad 6 */
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_NAME("Keypad 4") PORT_CODE(KEYCODE_4_PAD)	/* keypad 4 */
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_NAME("Keypad 5") PORT_CODE(KEYCODE_5_PAD)	/* keypad 5 */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_NAME("Keypad 9") PORT_CODE(KEYCODE_9_PAD)	/* keypad 9 */
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_NAME("Keypad 7") PORT_CODE(KEYCODE_7_PAD)	/* keypad 7 */
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_NAME("Keypad 8") PORT_CODE(KEYCODE_8_PAD)	/* keypad 8 */
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_NAME("Keypad #") PORT_CODE(KEYCODE_PLUS_PAD)	/* keypad # */
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_NAME("Keypad *") PORT_CODE(KEYCODE_MINUS_PAD)	/* keypad * */
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_NAME("Keypad 0") PORT_CODE(KEYCODE_0_PAD)	/* keypad 0 */

	PORT_START("ANALOG3")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(25) PORT_KEYDELTA(20)

	PORT_START("ANALOG2")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(20)

	PORT_START("ANALOG1")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_SENSITIVITY(25) PORT_KEYDELTA(20)

	PORT_START("ANALOG0")
	PORT_BIT( 0xff, 0x00, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( thegrid )
	PORT_START("DIPS")		/* DS1 */
	PORT_DIPNAME( 0x0001, 0x0001, "Show Blood" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) ) /* Manual states that switches 2-7 are Unused */
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "Coinage Source" )
	PORT_DIPSETTING(      0x0100, "Dipswitch" )
	PORT_DIPSETTING(      0x0000, "CMOS" )
	PORT_DIPNAME( 0x3e00, 0x3e00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0x3e00, "USA-1" )
	PORT_DIPSETTING(      0x3800, "USA-2" )
	PORT_DIPSETTING(      0x3c00, "USA-10" )
	PORT_DIPSETTING(      0x3a00, "USA-14" )
	PORT_DIPSETTING(      0x3600, "USA-DC1" )
	PORT_DIPSETTING(      0x3000, "USA-DC2" )
	PORT_DIPSETTING(      0x3200, "USA-DC4" )
	PORT_DIPSETTING(      0x3400, "USA-DC5" )
	PORT_DIPSETTING(      0x2e00, "French-ECA1" )
	PORT_DIPSETTING(      0x2c00, "French-ECA2" )
	PORT_DIPSETTING(      0x2a00, "French-ECA3" )
	PORT_DIPSETTING(      0x2800, "French-ECA4" )
	PORT_DIPSETTING(      0x2600, "French-ECA5" )
	PORT_DIPSETTING(      0x2400, "French-ECA6" )
	PORT_DIPSETTING(      0x2200, "French-ECA7" )
	PORT_DIPSETTING(      0x2000, "French-ECA8" )
	PORT_DIPSETTING(      0x1e00, "German-1" )
	PORT_DIPSETTING(      0x1c00, "German-2" )
	PORT_DIPSETTING(      0x1a00, "German-3" )
	PORT_DIPSETTING(      0x1800, "German-4" )
	PORT_DIPSETTING(      0x1600, "German-5" )
	PORT_DIPSETTING(      0x1400, "German-ECA1" )
	PORT_DIPSETTING(      0x1200, "German-ECA2" )
	PORT_DIPSETTING(      0x1000, "German-ECA3" )
	PORT_DIPSETTING(      0x0800, "UK-4" )
	PORT_DIPSETTING(      0x0600, "UK-5" )
	PORT_DIPSETTING(      0x0e00, "UK-1 ECA" )
	PORT_DIPSETTING(      0x0c00, "UK-2 ECA" )
	PORT_DIPSETTING(      0x0a00, "UK-3 ECA" )
	PORT_DIPSETTING(      0x0400, "UK-6 ECA" )
	PORT_DIPSETTING(      0x0200, "UK-7 ECA" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )	/* Manual states switches 7 & 8 are Unused */
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT ) /* Slam Switch */
	PORT_SERVICE_NO_TOGGLE( 0x0010, IP_ACTIVE_LOW )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0x6000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BILL1 )	/* Bill */

	PORT_START("IN1")	/* Listed "names" are via the manual's "JAMMA" pinout sheet" */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(1) PORT_8WAY /* Not Used */
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(1) PORT_8WAY /* Not Used */
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(1) PORT_8WAY /* Not Used */
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY /* Not Used */
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) /* Trigger */
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) /* Fire */
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) /* Action */
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(2) PORT_8WAY /* No Connection */
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(2) PORT_8WAY /* No Connection */
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(2) PORT_8WAY /* No Connection */
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY /* No Connection */
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) /* No Connection */
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) /* No Connection */
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) /* No Connection */
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM(custom_49way_r, "49WAYX\0" "49WAYY")
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("49WAYX")
	PORT_BIT( 0xff, 0x38, IPT_AD_STICK_X ) PORT_MINMAX(0x00,0x6f) PORT_SENSITIVITY(100) PORT_KEYDELTA(10)

	PORT_START("49WAYY")
	PORT_BIT( 0xff, 0x38, IPT_AD_STICK_Y ) PORT_MINMAX(0x00,0x6f) PORT_SENSITIVITY(100) PORT_KEYDELTA(10)
INPUT_PORTS_END



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static MACHINE_DRIVER_START( midzeus )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", TMS32032, CPU_CLOCK)
	MDRV_CPU_PROGRAM_MAP(zeus_map)
	MDRV_CPU_VBLANK_INT("screen", display_irq)

	MDRV_MACHINE_START(midzeus)
	MDRV_MACHINE_RESET(midzeus)
	MDRV_NVRAM_HANDLER(generic_1fill)

	/* video hardware */
	MDRV_PALETTE_LENGTH(32768)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_RAW_PARAMS(MIDZEUS_VIDEO_CLOCK/8, 529, 0, 400, 278, 0, 256)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)

	MDRV_VIDEO_START(midzeus)
	MDRV_VIDEO_UPDATE(midzeus)

	/* sound hardware */
	MDRV_IMPORT_FROM(dcs2_audio_2104)
MACHINE_DRIVER_END

static READ8_HANDLER( PIC16C5X_T0_clk_r )
{
	return 0;
}

static ADDRESS_MAP_START( pic_io_map, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(PIC16C5x_T0, PIC16C5x_T0) AM_READ(PIC16C5X_T0_clk_r)
ADDRESS_MAP_END


static MACHINE_DRIVER_START( invasn )

	MDRV_IMPORT_FROM(midzeus)

	MDRV_CPU_ADD("pic", PIC16C57, 8000000)	/* ? */
	MDRV_CPU_IO_MAP(pic_io_map)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( midzeus2 )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", TMS32032, CPU_CLOCK)
	MDRV_CPU_PROGRAM_MAP(zeus2_map)
	MDRV_CPU_VBLANK_INT("screen", display_irq)

	MDRV_MACHINE_START(midzeus)
	MDRV_MACHINE_RESET(midzeus)
	MDRV_NVRAM_HANDLER(midzeus2)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_RAW_PARAMS(MIDZEUS_VIDEO_CLOCK/4, 666, 0, 512, 438, 0, 400)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)

	MDRV_VIDEO_START(midzeus2)
	MDRV_VIDEO_UPDATE(midzeus2)

	/* sound hardware */
	MDRV_IMPORT_FROM(dcs2_audio_2104)

	MDRV_M48T35_ADD( "m48t35" )
MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( mk4 )
	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )	/* sound data */
	ROM_LOAD16_BYTE( "mk4_l2.u2", 0x000000, 0x100000, CRC(f9d410b4) SHA1(49bcacf83430ed26c08789b2f3ed9f946c3a0e5e) ) /* Labeled as v2.0, rom type M27C800 */
	ROM_LOAD16_BYTE( "mk4_l2.u3", 0x400000, 0x200000, CRC(8fbcf0ac) SHA1(c53704e72cfcba800c7af3a03267041f1e29a784) ) /* Labeled as v2.0, rom type M27C160 */
	ROM_LOAD16_BYTE( "mk4_l1.u4", 0x800000, 0x200000, CRC(dee91696) SHA1(00a182a36a414744cd014fcfc53c2e1a66ab5189) ) /* Labeled as v1.0, rom type M27C160 */
	ROM_LOAD16_BYTE( "mk4_l1.u5", 0xc00000, 0x200000, CRC(44d072be) SHA1(8a636c2801d799dfb84e69607ade76d2b49cf09f) ) /* Labeled as v1.0, rom type M27C160 */

	ROM_REGION32_LE( 0x1800000, "user1", 0 )
	ROM_LOAD32_WORD( "mk4_l3.u10", 0x0000000, 0x200000, CRC(84efe5a9) SHA1(e2a9bf6fab971691017371a87ab87b1bf66f96d0) ) /* Roms U10 & U11 were labeled as v3.0 */
	ROM_LOAD32_WORD( "mk4_l3.u11", 0x0000002, 0x200000, CRC(0c026ccb) SHA1(7531fe81ff8d8dd9ec3cd915acaf14cbe6bdc90a) )
	ROM_LOAD32_WORD( "mk4_l2.u12", 0x0400000, 0x200000, CRC(7816c07f) SHA1(da94b4391e671f915c61b5eb9bece4acb3382e31) ) /* Roms U12 through U17 were all labeled as v2.0 */
	ROM_LOAD32_WORD( "mk4_l2.u13", 0x0400002, 0x200000, CRC(b3c237cd) SHA1(9e71e60cc92c17524f85f36543c174ca138104cd) )
	ROM_LOAD32_WORD( "mk4_l2.u14", 0x0800000, 0x200000, CRC(fd33eb1a) SHA1(59d9d2e5251679d19cab031f51731c85f429ba18) ) /* It is possible that in late production, these  */
	ROM_LOAD32_WORD( "mk4_l2.u15", 0x0800002, 0x200000, CRC(b907518f) SHA1(cfb56538746895bdca779957fec6a872019b23c3) ) /* rom were also labeled as v3.0, but the labels  */
	ROM_LOAD32_WORD( "mk4_l2.u16", 0x0c00000, 0x200000, CRC(24371d57) SHA1(c90134b17c23a182d391d1679bf457d251e641f7) ) /* with v2.0 have been verified on several boards */
	ROM_LOAD32_WORD( "mk4_l2.u17", 0x0c00002, 0x200000, CRC(3a1a082c) SHA1(5f8e8ce760d8ebadd1240ef08f1382a37cf11d0b) ) /* Some PCBs may have all MASK ROMs instead.      */
ROM_END

ROM_START( mk4a )
	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )	/* sound data */
	ROM_LOAD16_BYTE( "mk4_l2.u2", 0x000000, 0x100000, CRC(f9d410b4) SHA1(49bcacf83430ed26c08789b2f3ed9f946c3a0e5e) ) /* Labeled as v2.0, rom type M27C800 */
	ROM_LOAD16_BYTE( "mk4_l2.u3", 0x400000, 0x200000, CRC(8fbcf0ac) SHA1(c53704e72cfcba800c7af3a03267041f1e29a784) ) /* Labeled as v2.0, rom type M27C160 */
	ROM_LOAD16_BYTE( "mk4_l1.u4", 0x800000, 0x200000, CRC(dee91696) SHA1(00a182a36a414744cd014fcfc53c2e1a66ab5189) ) /* Labeled as v1.0, rom type M27C160 */
	ROM_LOAD16_BYTE( "mk4_l1.u5", 0xc00000, 0x200000, CRC(44d072be) SHA1(8a636c2801d799dfb84e69607ade76d2b49cf09f) ) /* Labeled as v1.0, rom type M27C160 */

	ROM_REGION32_LE( 0x1800000, "user1", 0 )
	ROM_LOAD32_WORD( "mk4_l2.1.u10", 0x000000, 0x200000, CRC(42d0f1c9) SHA1(5ac0ded8bf6e756319be2691e3b555eac079ebdc) ) /* Roms U10 & U11 were labeled as v2.1 */
	ROM_LOAD32_WORD( "mk4_l2.1.u11", 0x000002, 0x200000, CRC(6e21b243) SHA1(6d4768a5972db05c1409e0d16e79df9eff8918a0) )
	ROM_LOAD32_WORD( "mk4_l2.u12", 0x0400000, 0x200000, CRC(7816c07f) SHA1(da94b4391e671f915c61b5eb9bece4acb3382e31) ) /* Roms U12 through U17 were all labeled as v2.0 */
	ROM_LOAD32_WORD( "mk4_l2.u13", 0x0400002, 0x200000, CRC(b3c237cd) SHA1(9e71e60cc92c17524f85f36543c174ca138104cd) )
	ROM_LOAD32_WORD( "mk4_l2.u14", 0x0800000, 0x200000, CRC(fd33eb1a) SHA1(59d9d2e5251679d19cab031f51731c85f429ba18) )
	ROM_LOAD32_WORD( "mk4_l2.u15", 0x0800002, 0x200000, CRC(b907518f) SHA1(cfb56538746895bdca779957fec6a872019b23c3) )
	ROM_LOAD32_WORD( "mk4_l2.u16", 0x0c00000, 0x200000, CRC(24371d57) SHA1(c90134b17c23a182d391d1679bf457d251e641f7) )
	ROM_LOAD32_WORD( "mk4_l2.u17", 0x0c00002, 0x200000, CRC(3a1a082c) SHA1(5f8e8ce760d8ebadd1240ef08f1382a37cf11d0b) )
ROM_END

ROM_START( mk4b )
	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )	/* sound data */
	ROM_LOAD16_BYTE( "mk4_l1.u2", 0x000000, 0x200000, CRC(daac8ab5) SHA1(b93aa205868212077a9b6ac8e93205e1ebf8c05e) ) /* All sound roms were labeled as v1.0 & are M27C160 type */
	ROM_LOAD16_BYTE( "mk4_l1.u3", 0x400000, 0x200000, CRC(cb59413e) SHA1(f7e5c589a8f6a2e7dceee4881594e7403be4d4ad) )
	ROM_LOAD16_BYTE( "mk4_l1.u4", 0x800000, 0x200000, CRC(dee91696) SHA1(00a182a36a414744cd014fcfc53c2e1a66ab5189) )
	ROM_LOAD16_BYTE( "mk4_l1.u5", 0xc00000, 0x200000, CRC(44d072be) SHA1(8a636c2801d799dfb84e69607ade76d2b49cf09f) )

	ROM_REGION32_LE( 0x1800000, "user1", 0 )
	ROM_LOAD32_WORD( "mk4_l1.u10", 0x0000000, 0x200000, CRC(6fcc86dd) SHA1(b3b2b463daf51450fbcd5d2922ac1b091bd91c4a) ) /* All roms were labeled as v1.0 */
	ROM_LOAD32_WORD( "mk4_l1.u11", 0x0000002, 0x200000, CRC(04895940) SHA1(55d368905f5986587c4e3da236401fdd5e2c269c) )
	ROM_LOAD32_WORD( "mk4_l1.u12", 0x0400000, 0x200000, CRC(323ddc5c) SHA1(4303c109c68a7cc15ff6fe91b6d34383b6066351) )
	ROM_LOAD32_WORD( "mk4_l1.u13", 0x0400002, 0x200000, CRC(0b95bdf0) SHA1(a25d48b33a861b5e52736720c7a79291fa837f78) )
	ROM_LOAD32_WORD( "mk4_l1.u14", 0x0800000, 0x200000, CRC(cb6816ef) SHA1(9c828c188d297aee0f211acc283035289e80b5a8) )
	ROM_LOAD32_WORD( "mk4_l1.u15", 0x0800002, 0x200000, CRC(cde47df7) SHA1(63383d983c03703b2f3f1973ce2a7553654836d4) )
	/* No U16 or U17 roms present in this version */
ROM_END

ROM_START( invasnab ) /* Version 5.0 Program roms, v4.0 Graphics roms, v2.0 Sound roms */
	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )	/* sound data */
	ROM_LOAD16_BYTE( "invasion2.u2", 0x000000, 0x200000, CRC(59d2e1d6) SHA1(994a4311ac4841d4341449c0c7480952b6f3855d) ) /* These four sound roms were labeled as v2.0 */
	ROM_LOAD16_BYTE( "invasion2.u3", 0x400000, 0x200000, CRC(86b956ae) SHA1(f7fd4601a2ce3e7e9b67e7d77908bfa206ee7e62) )
	ROM_LOAD16_BYTE( "invasion2.u4", 0x800000, 0x200000, CRC(5ef1fab5) SHA1(987afa0672fa89b18cf20d28644848a9e5ee9b17) )
	ROM_LOAD16_BYTE( "invasion2.u5", 0xc00000, 0x200000, CRC(e42805c9) SHA1(e5b71eb1852809a649ac43a82168b3bdaf4b1526) )

	ROM_REGION( 0x2000, "pic", 0 ) /* PIC16c57 Code */
	ROM_LOAD( "pic16c57.u76", 0x00000, 0x2000, CRC(f62729c9) SHA1(9642c53dd7eceeb7eb178497d367691c44abc5c5) ) // is this even a valid dump?

	ROM_REGION32_LE( 0x1800000, "user1", 0 )
	ROM_LOAD32_WORD( "invasion5.u10", 0x0000000, 0x200000, CRC(8c7785d9) SHA1(701602314cd4eba4215c47ea0ae75fd4eddad43b) ) /* Roms U10 & U11 were labeled as v5.0 */
	ROM_LOAD32_WORD( "invasion5.u11", 0x0000002, 0x200000, CRC(8ceb1f32) SHA1(82d01f25cba25d77b11c347632e8b72776e12984) )
	ROM_LOAD32_WORD( "invasion4.u12", 0x0400000, 0x200000, CRC(ce1eb06a) SHA1(ff17690a0cbca6dcccccde70e2c5812ae03db5bb) ) /* Roms U12 through U19 were all labeled as v4.0 */
	ROM_LOAD32_WORD( "invasion4.u13", 0x0400002, 0x200000, CRC(33fc6707) SHA1(11a39ad980ec320547319eca6ffa5aef3ab8b010) )
	ROM_LOAD32_WORD( "invasion4.u14", 0x0800000, 0x200000, CRC(760682a1) SHA1(ff91210225d4aa750115c6219d4c35c9521a3f0b) )
	ROM_LOAD32_WORD( "invasion4.u15", 0x0800002, 0x200000, CRC(90467d7a) SHA1(a143a3d3605e5626852e75937160ba6bcd891608) )
	ROM_LOAD32_WORD( "invasion4.u16", 0x0c00000, 0x200000, CRC(3ef1b28d) SHA1(6f9a071b8830194fea84daa62aadabae86977c5f) )
	ROM_LOAD32_WORD( "invasion4.u17", 0x0c00002, 0x200000, CRC(97aa677a) SHA1(4d21cc59e0ffd4985f89c97c71d605c3b404a8a3) )
	ROM_LOAD32_WORD( "invasion4.u18", 0x1000000, 0x200000, CRC(6930c656) SHA1(28054ff9a6c6f5764a371f8defe4c1f5730618f3) )
	ROM_LOAD32_WORD( "invasion4.u19", 0x1000002, 0x200000, CRC(89fa6ee5) SHA1(572565e1308142b0b062aa72315c68e928f2419c) )
ROM_END

ROM_START( invasnv4 ) /* Version 4.0 Program roms & Graphics roms, v2.0 Sound roms */
	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )	/* sound data */
	ROM_LOAD16_BYTE( "invasion2.u2", 0x000000, 0x200000, CRC(59d2e1d6) SHA1(994a4311ac4841d4341449c0c7480952b6f3855d) ) /* These four sound roms were labeled as v2.0 */
	ROM_LOAD16_BYTE( "invasion2.u3", 0x400000, 0x200000, CRC(86b956ae) SHA1(f7fd4601a2ce3e7e9b67e7d77908bfa206ee7e62) )
	ROM_LOAD16_BYTE( "invasion2.u4", 0x800000, 0x200000, CRC(5ef1fab5) SHA1(987afa0672fa89b18cf20d28644848a9e5ee9b17) )
	ROM_LOAD16_BYTE( "invasion2.u5", 0xc00000, 0x200000, CRC(e42805c9) SHA1(e5b71eb1852809a649ac43a82168b3bdaf4b1526) )

	ROM_REGION( 0x2000, "pic", 0 ) /* PIC16c57 Code */
	ROM_LOAD( "pic16c57.u76", 0x00000, 0x2000, CRC(f62729c9) SHA1(9642c53dd7eceeb7eb178497d367691c44abc5c5) ) // is this even a valid dump?

	ROM_REGION32_LE( 0x1800000, "user1", 0 )
	ROM_LOAD32_WORD( "invasion4.u10", 0x0000000, 0x200000, CRC(b3ce958b) SHA1(ed51c167d85bc5f6155b8046ec056a4f4ad5cf9d) ) /* These rom were all labeled as v4.0 */
	ROM_LOAD32_WORD( "invasion4.u11", 0x0000002, 0x200000, CRC(0bd09359) SHA1(f40886bd2e5f5fbf506580e5baa2f733be200852) )
	ROM_LOAD32_WORD( "invasion4.u12", 0x0400000, 0x200000, CRC(ce1eb06a) SHA1(ff17690a0cbca6dcccccde70e2c5812ae03db5bb) )
	ROM_LOAD32_WORD( "invasion4.u13", 0x0400002, 0x200000, CRC(33fc6707) SHA1(11a39ad980ec320547319eca6ffa5aef3ab8b010) )
	ROM_LOAD32_WORD( "invasion4.u14", 0x0800000, 0x200000, CRC(760682a1) SHA1(ff91210225d4aa750115c6219d4c35c9521a3f0b) )
	ROM_LOAD32_WORD( "invasion4.u15", 0x0800002, 0x200000, CRC(90467d7a) SHA1(a143a3d3605e5626852e75937160ba6bcd891608) )
	ROM_LOAD32_WORD( "invasion4.u16", 0x0c00000, 0x200000, CRC(3ef1b28d) SHA1(6f9a071b8830194fea84daa62aadabae86977c5f) )
	ROM_LOAD32_WORD( "invasion4.u17", 0x0c00002, 0x200000, CRC(97aa677a) SHA1(4d21cc59e0ffd4985f89c97c71d605c3b404a8a3) )
	ROM_LOAD32_WORD( "invasion4.u18", 0x1000000, 0x200000, CRC(6930c656) SHA1(28054ff9a6c6f5764a371f8defe4c1f5730618f3) )
	ROM_LOAD32_WORD( "invasion4.u19", 0x1000002, 0x200000, CRC(89fa6ee5) SHA1(572565e1308142b0b062aa72315c68e928f2419c) )
ROM_END

ROM_START( crusnexo )
	ROM_REGION16_LE( 0xc00000, "dcs", ROMREGION_ERASEFF )	/* sound data */
	ROM_LOAD( "exotica.u2", 0x000000, 0x200000, CRC(d2d54acf) SHA1(2b4d6fda30af807228bb281335939dfb6df9b530) )
	ROM_RELOAD(             0x200000, 0x200000 )
	ROM_LOAD( "exotica.u3", 0x400000, 0x400000, CRC(28a3a13d) SHA1(8d7d641b883df089adefdd144229afef79db9e8a) )
	ROM_LOAD( "exotica.u4", 0x800000, 0x400000, CRC(213f7fd8) SHA1(8528d524a62bc41a8e3b39f0dbeeba33c862ee27) )

	ROM_REGION32_LE( 0x0800000, "user1", 0 )
	ROM_LOAD32_WORD( "exotica-24.u10", 0x0000000, 0x200000, CRC(5e702f7c) SHA1(98c76fb46b304d4d21656d0505d5e5e99c8335bf) ) /* Version 2.4  Wed Aug 23, 2000  17:26:53 */
	ROM_LOAD32_WORD( "exotica-24.u11", 0x0000002, 0x200000, CRC(5ecb2cbc) SHA1(57283167e48ca96579d0712d9fec23a36fa2b496) )
	ROM_LOAD32_WORD( "exotica.u12",    0x0400000, 0x200000, CRC(21f122b2) SHA1(5473401ec954bf9ab66a8283bd08d17c7960cd29) )
	ROM_LOAD32_WORD( "exotica.u13",    0x0400002, 0x200000, CRC(cf9d3609) SHA1(6376891f478185d26370466bef92f0c5304d58d3) )

	ROM_REGION32_LE( 0x3000000, "user2", 0 )
	ROM_LOAD32_WORD( "exotica.u14", 0x0000000, 0x400000, CRC(84452fc2) SHA1(06d87263f83ef079e6c5fb9de620e0135040c858) )
	ROM_LOAD32_WORD( "exotica.u15", 0x0000002, 0x400000, CRC(b6aaebdb) SHA1(6ede6ea123be6a88d1ff38e90f059c9d1f822d6d) )
	ROM_LOAD32_WORD( "exotica.u16", 0x0800000, 0x400000, CRC(aac6d2a5) SHA1(6c336520269d593b46b82414d9352a3f16955cc3) )
	ROM_LOAD32_WORD( "exotica.u17", 0x0800002, 0x400000, CRC(71cf5404) SHA1(a6eed1a66fb4f4ddd749e4272a2cdb8e3e354029) )
	ROM_LOAD32_WORD( "exotica.u22", 0x1000000, 0x400000, CRC(ad6dcda7) SHA1(5c9291753e1659f9adbe7e59fa2d0e030efae5bc) )
	ROM_LOAD32_WORD( "exotica.u23", 0x1000002, 0x400000, CRC(1f103a68) SHA1(3b3acc63a461677cd424e75e7211fa6f063a37ef) )
	ROM_LOAD32_WORD( "exotica.u24", 0x1800000, 0x400000, CRC(6312feef) SHA1(4113e4e5d39c99e8131d41a57c973df475b67d18) )
	ROM_LOAD32_WORD( "exotica.u25", 0x1800002, 0x400000, CRC(b8277b16) SHA1(1355e87affd78e195906aedc9aed9e230374e2bf) )
	ROM_LOAD32_WORD( "exotica.u18", 0x2000000, 0x200000, CRC(60cf5caa) SHA1(629870a305802d632bd2681131d1ffc0086280d2) )
	ROM_LOAD32_WORD( "exotica.u19", 0x2000002, 0x200000, CRC(6b919a18) SHA1(20e40e195554146ed1d3fad54f7280823ae89d4b) )
	ROM_LOAD32_WORD( "exotica.u20", 0x2400000, 0x200000, CRC(4855b68b) SHA1(1f6e557590b2621d0d5c782b95577f1be5cbc51d) )
	ROM_LOAD32_WORD( "exotica.u21", 0x2400002, 0x200000, CRC(0011b9d6) SHA1(231d768c964a16b905857b0814d758fe93c2eefb) )
ROM_END

ROM_START( crusnexoa )
	ROM_REGION16_LE( 0xc00000, "dcs", ROMREGION_ERASEFF )	/* sound data */
	ROM_LOAD( "exotica.u2", 0x000000, 0x200000, CRC(d2d54acf) SHA1(2b4d6fda30af807228bb281335939dfb6df9b530) )
	ROM_RELOAD(             0x200000, 0x200000 )
	ROM_LOAD( "exotica.u3", 0x400000, 0x400000, CRC(28a3a13d) SHA1(8d7d641b883df089adefdd144229afef79db9e8a) )
	ROM_LOAD( "exotica.u4", 0x800000, 0x400000, CRC(213f7fd8) SHA1(8528d524a62bc41a8e3b39f0dbeeba33c862ee27) )

	ROM_REGION32_LE( 0x0800000, "user1", 0 )
	ROM_LOAD32_WORD( "exotica-20.u10", 0x0000000, 0x200000, CRC(43d80f54) SHA1(25683d835f3ed3dee99da33280ae6e21865801e4) ) /* Version 2.0  Fri Apr 07, 2000  17:55:07 */
	ROM_LOAD32_WORD( "exotica-20.u11", 0x0000002, 0x200000, CRC(dba26b69) SHA1(4900ac3fe67664a543dcd66e41793874f6cdc07f) )
	ROM_LOAD32_WORD( "exotica.u12",    0x0400000, 0x200000, CRC(21f122b2) SHA1(5473401ec954bf9ab66a8283bd08d17c7960cd29) )
	ROM_LOAD32_WORD( "exotica.u13",    0x0400002, 0x200000, CRC(cf9d3609) SHA1(6376891f478185d26370466bef92f0c5304d58d3) )

	ROM_REGION32_LE( 0x3000000, "user2", 0 )
	ROM_LOAD32_WORD( "exotica.u14", 0x0000000, 0x400000, CRC(84452fc2) SHA1(06d87263f83ef079e6c5fb9de620e0135040c858) )
	ROM_LOAD32_WORD( "exotica.u15", 0x0000002, 0x400000, CRC(b6aaebdb) SHA1(6ede6ea123be6a88d1ff38e90f059c9d1f822d6d) )
	ROM_LOAD32_WORD( "exotica.u16", 0x0800000, 0x400000, CRC(aac6d2a5) SHA1(6c336520269d593b46b82414d9352a3f16955cc3) )
	ROM_LOAD32_WORD( "exotica.u17", 0x0800002, 0x400000, CRC(71cf5404) SHA1(a6eed1a66fb4f4ddd749e4272a2cdb8e3e354029) )
	ROM_LOAD32_WORD( "exotica.u22", 0x1000000, 0x400000, CRC(ad6dcda7) SHA1(5c9291753e1659f9adbe7e59fa2d0e030efae5bc) )
	ROM_LOAD32_WORD( "exotica.u23", 0x1000002, 0x400000, CRC(1f103a68) SHA1(3b3acc63a461677cd424e75e7211fa6f063a37ef) )
	ROM_LOAD32_WORD( "exotica.u24", 0x1800000, 0x400000, CRC(6312feef) SHA1(4113e4e5d39c99e8131d41a57c973df475b67d18) )
	ROM_LOAD32_WORD( "exotica.u25", 0x1800002, 0x400000, CRC(b8277b16) SHA1(1355e87affd78e195906aedc9aed9e230374e2bf) )
	ROM_LOAD32_WORD( "exotica.u18", 0x2000000, 0x200000, CRC(60cf5caa) SHA1(629870a305802d632bd2681131d1ffc0086280d2) )
	ROM_LOAD32_WORD( "exotica.u19", 0x2000002, 0x200000, CRC(6b919a18) SHA1(20e40e195554146ed1d3fad54f7280823ae89d4b) )
	ROM_LOAD32_WORD( "exotica.u20", 0x2400000, 0x200000, CRC(4855b68b) SHA1(1f6e557590b2621d0d5c782b95577f1be5cbc51d) )
	ROM_LOAD32_WORD( "exotica.u21", 0x2400002, 0x200000, CRC(0011b9d6) SHA1(231d768c964a16b905857b0814d758fe93c2eefb) )
ROM_END

ROM_START( crusnexob )
	ROM_REGION16_LE( 0xc00000, "dcs", ROMREGION_ERASEFF )	/* sound data */
	ROM_LOAD( "exotica.u2", 0x000000, 0x200000, CRC(d2d54acf) SHA1(2b4d6fda30af807228bb281335939dfb6df9b530) )
	ROM_RELOAD(             0x200000, 0x200000 )
	ROM_LOAD( "exotica.u3", 0x400000, 0x400000, CRC(28a3a13d) SHA1(8d7d641b883df089adefdd144229afef79db9e8a) )
	ROM_LOAD( "exotica.u4", 0x800000, 0x400000, CRC(213f7fd8) SHA1(8528d524a62bc41a8e3b39f0dbeeba33c862ee27) )

	ROM_REGION32_LE( 0x0800000, "user1", 0 )
	ROM_LOAD32_WORD( "exotica-16.u10", 0x0000000, 0x200000, CRC(65450140) SHA1(cad41a2cad48426de01feb78d3f71f768e3fc872) ) /* Version 1.6  Tue Feb 22, 2000  10:25:01 */
	ROM_LOAD32_WORD( "exotica-16.u11", 0x0000002, 0x200000, CRC(e994891f) SHA1(bb088729b665864c7f3b79b97c3c86f9c8f68770) )
	ROM_LOAD32_WORD( "exotica.u12",    0x0400000, 0x200000, CRC(21f122b2) SHA1(5473401ec954bf9ab66a8283bd08d17c7960cd29) )
	ROM_LOAD32_WORD( "exotica.u13",    0x0400002, 0x200000, CRC(cf9d3609) SHA1(6376891f478185d26370466bef92f0c5304d58d3) )

	ROM_REGION32_LE( 0x3000000, "user2", 0 )
	ROM_LOAD32_WORD( "exotica.u14", 0x0000000, 0x400000, CRC(84452fc2) SHA1(06d87263f83ef079e6c5fb9de620e0135040c858) )
	ROM_LOAD32_WORD( "exotica.u15", 0x0000002, 0x400000, CRC(b6aaebdb) SHA1(6ede6ea123be6a88d1ff38e90f059c9d1f822d6d) )
	ROM_LOAD32_WORD( "exotica.u16", 0x0800000, 0x400000, CRC(aac6d2a5) SHA1(6c336520269d593b46b82414d9352a3f16955cc3) )
	ROM_LOAD32_WORD( "exotica.u17", 0x0800002, 0x400000, CRC(71cf5404) SHA1(a6eed1a66fb4f4ddd749e4272a2cdb8e3e354029) )
	ROM_LOAD32_WORD( "exotica.u22", 0x1000000, 0x400000, CRC(ad6dcda7) SHA1(5c9291753e1659f9adbe7e59fa2d0e030efae5bc) )
	ROM_LOAD32_WORD( "exotica.u23", 0x1000002, 0x400000, CRC(1f103a68) SHA1(3b3acc63a461677cd424e75e7211fa6f063a37ef) )
	ROM_LOAD32_WORD( "exotica.u24", 0x1800000, 0x400000, CRC(6312feef) SHA1(4113e4e5d39c99e8131d41a57c973df475b67d18) )
	ROM_LOAD32_WORD( "exotica.u25", 0x1800002, 0x400000, CRC(b8277b16) SHA1(1355e87affd78e195906aedc9aed9e230374e2bf) )
	ROM_LOAD32_WORD( "exotica.u18", 0x2000000, 0x200000, CRC(60cf5caa) SHA1(629870a305802d632bd2681131d1ffc0086280d2) )
	ROM_LOAD32_WORD( "exotica.u19", 0x2000002, 0x200000, CRC(6b919a18) SHA1(20e40e195554146ed1d3fad54f7280823ae89d4b) )
	ROM_LOAD32_WORD( "exotica.u20", 0x2400000, 0x200000, CRC(4855b68b) SHA1(1f6e557590b2621d0d5c782b95577f1be5cbc51d) )
	ROM_LOAD32_WORD( "exotica.u21", 0x2400002, 0x200000, CRC(0011b9d6) SHA1(231d768c964a16b905857b0814d758fe93c2eefb) )
ROM_END


ROM_START( thegrid ) /* Version 1.2 Program roms */
	ROM_REGION16_LE( 0xc00000, "dcs", ROMREGION_ERASEFF )	/* sound data */
	ROM_LOAD( "the_grid.u2", 0x000000, 0x400000, CRC(e6a39ee9) SHA1(4ddc62f5d278ea9791205098fa5f018ab1e698b4) )
	ROM_LOAD( "the_grid.u3", 0x400000, 0x400000, CRC(40be7585) SHA1(e481081edffa07945412a6eab17b4d3e7b42cfd3) )
	ROM_LOAD( "the_grid.u4", 0x800000, 0x400000, CRC(7a15c203) SHA1(a0a49dd08bba92402640ed2d1fb4fee112c4ab5f) )

	ROM_REGION32_LE( 0x0800000, "user1", 0 )
	ROM_LOAD32_WORD( "thegrid-12.u10", 0x0000000, 0x100000, CRC(eb6c2d54) SHA1(ddd32757a9be011988b7add3c091e93292a0867c) )
	ROM_LOAD32_WORD( "thegrid-12.u11", 0x0000002, 0x100000, CRC(b9b5f92b) SHA1(36e16f109af9a5172869344f09b337b67e0b3e11) )
	ROM_LOAD32_WORD( "thegrid-12.u12", 0x0200000, 0x100000, CRC(2810c207) SHA1(d244eaf85473ed49442a906d437af1a9f91a2f9d) )
	ROM_LOAD32_WORD( "thegrid-12.u13", 0x0200002, 0x100000, CRC(8b721848) SHA1(d82f39045437ada2061587176e24f558a5e203fe) )

	ROM_REGION32_LE( 0x3000000, "user2", 0 )
	ROM_LOAD32_WORD( "the_grid.u18",   0x0000000, 0x400000, CRC(3a3460be) SHA1(e719dae8a2e54584cb6a074ed42e35e3debef2f6) )
	ROM_LOAD32_WORD( "the_grid.u19",   0x0000002, 0x400000, CRC(af262d5b) SHA1(3eb3980fa81a360a70aa74e793b2bc3028f68cf2) )
	ROM_LOAD32_WORD( "the_grid.u20",   0x0800000, 0x400000, CRC(e6ad1917) SHA1(acab25e1251fd07b374badebe79f6ec1772b3589) )
	ROM_LOAD32_WORD( "the_grid.u21",   0x0800002, 0x400000, CRC(48c03f8e) SHA1(50790bdae9f2234ffb4914c2c5c16374e3508b47) )
	ROM_LOAD32_WORD( "the_grid.u22",   0x1000000, 0x400000, CRC(84c3a8b6) SHA1(de0dcf9daf7ada7a6952b9e29a29571b2aa9d0b2) )
	ROM_LOAD32_WORD( "the_grid.u23",   0x1000002, 0x400000, CRC(f48ef409) SHA1(79d74b4fe38b06a02ae0351d13d7f0a7ed0f0c87) )
ROM_END


ROM_START( thegrida ) /* Version 1.1 Program roms */
	ROM_REGION16_LE( 0xc00000, "dcs", ROMREGION_ERASEFF )	/* sound data */
	ROM_LOAD( "the_grid.u2", 0x000000, 0x400000, CRC(e6a39ee9) SHA1(4ddc62f5d278ea9791205098fa5f018ab1e698b4) )
	ROM_LOAD( "the_grid.u3", 0x400000, 0x400000, CRC(40be7585) SHA1(e481081edffa07945412a6eab17b4d3e7b42cfd3) )
	ROM_LOAD( "the_grid.u4", 0x800000, 0x400000, CRC(7a15c203) SHA1(a0a49dd08bba92402640ed2d1fb4fee112c4ab5f) )

	ROM_REGION32_LE( 0x0800000, "user1", 0 )
	ROM_LOAD32_WORD( "thegrid-11.u10", 0x0000000, 0x100000, CRC(87ea0e9e) SHA1(618de2ca87b7a3e0225d1f7e65f8fc1356de1421) )
	ROM_LOAD32_WORD( "thegrid-11.u11", 0x0000002, 0x100000, CRC(73d84b1a) SHA1(8dcfcab5ff64f46f8486e6439a10d91ad26fd48a) )
	ROM_LOAD32_WORD( "thegrid-11.u12", 0x0200000, 0x100000, CRC(78d16ca1) SHA1(7b893ec8af2f44d8bc293861fd8622d68d41ccbe) )
	ROM_LOAD32_WORD( "thegrid-11.u13", 0x0200002, 0x100000, CRC(8e00b400) SHA1(96581c5da62afc19e6d69b2352b3166665cb9918) )

	ROM_REGION32_LE( 0x3000000, "user2", 0 )
	ROM_LOAD32_WORD( "the_grid.u18",   0x0000000, 0x400000, CRC(3a3460be) SHA1(e719dae8a2e54584cb6a074ed42e35e3debef2f6) )
	ROM_LOAD32_WORD( "the_grid.u19",   0x0000002, 0x400000, CRC(af262d5b) SHA1(3eb3980fa81a360a70aa74e793b2bc3028f68cf2) )
	ROM_LOAD32_WORD( "the_grid.u20",   0x0800000, 0x400000, CRC(e6ad1917) SHA1(acab25e1251fd07b374badebe79f6ec1772b3589) )
	ROM_LOAD32_WORD( "the_grid.u21",   0x0800002, 0x400000, CRC(48c03f8e) SHA1(50790bdae9f2234ffb4914c2c5c16374e3508b47) )
	ROM_LOAD32_WORD( "the_grid.u22",   0x1000000, 0x400000, CRC(84c3a8b6) SHA1(de0dcf9daf7ada7a6952b9e29a29571b2aa9d0b2) )
	ROM_LOAD32_WORD( "the_grid.u23",   0x1000002, 0x400000, CRC(f48ef409) SHA1(79d74b4fe38b06a02ae0351d13d7f0a7ed0f0c87) )
ROM_END



/*************************************
 *
 *  Driver init
 *
 *************************************/

static DRIVER_INIT( mk4 )
{
	dcs2_init(machine, 0, 0);
	midway_ioasic_init(machine, MIDWAY_IOASIC_STANDARD, 461/* or 474 */, 94, NULL);
	midway_ioasic_set_shuffle_state(1);
}


static DRIVER_INIT( invasn )
{
	dcs2_init(machine, 0, 0);
	midway_ioasic_init(machine, MIDWAY_IOASIC_STANDARD, 468/* or 488 */, 94, NULL);
	memory_install_readwrite32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x9c0000, 0x9c0000, 0, 0, invasn_gun_r, invasn_gun_w);
}


static DRIVER_INIT( crusnexo )
{
	dcs2_init(machine, 0, 0);
	midway_ioasic_init(machine, MIDWAY_IOASIC_STANDARD, 472/* or 476,477,478,110 */, 99, NULL);
	memory_configure_bank(machine, "bank1", 0, 3, memory_region(machine, "user2"), 0x400000*4);

	memory_install_readwrite32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x9b0004, 0x9b0007, 0, 0, crusnexo_leds_r, crusnexo_leds_w);
	memory_install_write32_handler    (cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x8d0009, 0x8d000a, 0, 0, keypad_select_w);
}


static DRIVER_INIT( thegrid )
{
	dcs2_init(machine, 0, 0);
	midway_ioasic_init(machine, MIDWAY_IOASIC_STANDARD, 474/* or 491 */, 99, NULL);
	memory_configure_bank(machine, "bank1", 0, 3, memory_region(machine, "user2"), 0x400000*4);
}



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME(  1997, mk4,      0,        midzeus,  mk4,      mk4,      ROT0, "Midway", "Mortal Kombat 4 (version 3.0)", GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE )
GAME(  1997, mk4a,     mk4,      midzeus,  mk4,      mk4,      ROT0, "Midway", "Mortal Kombat 4 (version 2.1)", GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE )
GAME(  1997, mk4b,     mk4,      midzeus,  mk4,      mk4,      ROT0, "Midway", "Mortal Kombat 4 (version 1.0)", GAME_NOT_WORKING | GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE )
GAME(  1999, invasnab, 0,        invasn,   invasn,   invasn,   ROT0, "Midway", "Invasion - The Abductors (version 5.0)", GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE )
GAME(  1999, invasnv4, invasnab, invasn,   invasn,   invasn,   ROT0, "Midway", "Invasion - The Abductors (version 4.0)", GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE )
GAMEL( 1999, crusnexo, 0,        midzeus2, crusnexo, crusnexo, ROT0, "Midway", "Cruis'n Exotica (version 2.4)", GAME_NOT_WORKING | GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE, layout_crusnexo )
GAMEL( 1999, crusnexoa,crusnexo, midzeus2, crusnexo, crusnexo, ROT0, "Midway", "Cruis'n Exotica (version 2.0)", GAME_NOT_WORKING | GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE, layout_crusnexo )
GAMEL( 1999, crusnexob,crusnexo, midzeus2, crusnexo, crusnexo, ROT0, "Midway", "Cruis'n Exotica (version 1.6)", GAME_NOT_WORKING | GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE, layout_crusnexo )
GAME(  2001, thegrid,  0,        midzeus2, thegrid,  thegrid,  ROT0, "Midway", "The Grid (version 1.2)", GAME_NOT_WORKING | GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE )
GAME(  2001, thegrida, thegrid,  midzeus2, thegrid,  thegrid,  ROT0, "Midway", "The Grid (version 1.1)", GAME_NOT_WORKING | GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE )
