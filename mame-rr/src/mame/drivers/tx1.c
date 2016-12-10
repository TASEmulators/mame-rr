/***************************************************************************

    Tatsumi TX-1/Buggy Boy hardware

    driver by Phil Bennett

    Games supported:
        * TX-1 (1983) [2 sets]
        * Buggy Boy (1985)
        * Buggy Boy Junior (1986)

    ROMs wanted:
        * TX-1 V8 (1984)

    Notes:
        * TX-1 tyre screech noises are not implemented yet.

****************************************************************************

    Buggy Boy Error Codes          TX-1 Error Codes
    =====================          ================

    1  Main CPU RAM                1  Main microprocessor RAM
    2  Video (character) RAM       2  Video RAM
    3  Road/common RAM             3  Common RAM
    4  Sound RAM                   4  Sound RAM
    5  Main CPU ROM                5  Main microprocessor ROM
    6  Sound ROM                   6  Sound ROM
    8  Auxillary ROM               10 Interface ROM (time-out error)
    12 Arithmetic unit             11 Common RAM (access for arithmetic CPU)
    22 Main 8086-Z80 time-out      12 Common RAM (access for arithmetic CPU)
                                   13 Arithmetic RAM
                                   14 Common RAM (access for arithmetic CPU)
                                   15 Object RAM
                                   16 Arithmetic ROM
                                   17 Data ROM (checksum)
                                   18 Arithmetic unit

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/i86/i86.h"
#include "machine/8255ppi.h"
#include "sound/ay8910.h"
#include "rendlay.h"
#include "includes/tx1.h"

#include "tx1.lh"
#include "buggyboy.lh"
#include "buggybjr.lh"


/*************************************
 *
 *  Globals
 *
 *************************************/

UINT16 *tx1_math_ram;
static UINT8  *z80_ram;
static UINT8  tx1_ppi_latch_a;
static UINT8  tx1_ppi_latch_b;
static UINT32 ts;


/*************************************
 *
 *  CPU Control
 *
 *************************************/

/* Main CPU and Z80 synchronisation */
static WRITE16_HANDLER( z80_busreq_w )
{
	cputag_set_input_line(space->machine, "audio_cpu", INPUT_LINE_HALT, (data & 1) ? CLEAR_LINE : ASSERT_LINE);
}

static WRITE16_HANDLER( resume_math_w )
{
	cputag_set_input_line(space->machine, "math_cpu", INPUT_LINE_TEST, ASSERT_LINE);
}

static WRITE16_HANDLER( halt_math_w )
{
	cputag_set_input_line(space->machine, "math_cpu", INPUT_LINE_TEST, CLEAR_LINE);
}

/* Z80 can trigger its own interrupts */
static WRITE8_HANDLER( z80_intreq_w )
{
	cputag_set_input_line(space->machine, "audio_cpu", 0, HOLD_LINE);
}

/* Periodic Z80 interrupt */
static INTERRUPT_GEN( z80_irq )
{
	cpu_set_input_line(device, 0, HOLD_LINE);
}

static READ16_HANDLER( z80_shared_r )
{
	const address_space *cpu2space = cputag_get_address_space(space->machine, "audio_cpu", ADDRESS_SPACE_PROGRAM);
	return memory_read_byte(cpu2space, offset);
}

static WRITE16_HANDLER( z80_shared_w )
{
	const address_space *cpu2space = cputag_get_address_space(space->machine, "audio_cpu", ADDRESS_SPACE_PROGRAM);
	memory_write_byte(cpu2space, offset, data & 0xff);
}


/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( tx1 )
	PORT_START("DSW")
	PORT_DIPNAME( 0x001c, 0x0000, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x001c, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0014, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_6C ) )

	PORT_DIPNAME( 0x00e0, 0x0000, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x00e0, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x00a0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_6C ) )

	PORT_DIPNAME( 0x0700, 0x0300, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0000, "A (Easiest)" )
	PORT_DIPSETTING(      0x0100, "B" )
	PORT_DIPSETTING(      0x0200, "C" )
	PORT_DIPSETTING(      0x0300, "D" )
	PORT_DIPSETTING(      0x0400, "E" )
	PORT_DIPSETTING(      0x0500, "F" )
	PORT_DIPSETTING(      0x0600, "G" )
	PORT_DIPSETTING(      0x0700, "H (Hardest)" )

	PORT_DIPNAME( 0x1800, 0x0800, DEF_STR( Game_Time ) )
	PORT_DIPSETTING(      0x0000, "A (Longest)" )
	PORT_DIPSETTING(      0x0800, "B" )
	PORT_DIPSETTING(      0x1000, "C" )
	PORT_DIPSETTING(      0x1800, "D (Shortest)" )

	PORT_DIPNAME( 0xe000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, "0" )
	PORT_DIPSETTING(      0x2000, "1" )
	PORT_DIPSETTING(      0x4000, "2" )
	PORT_DIPSETTING(      0x6000, "3" )
	PORT_DIPSETTING(      0x8000, "4" )
	PORT_DIPSETTING(      0xa000, "5" )
	PORT_DIPSETTING(      0xc000, "6" )
	PORT_DIPSETTING(      0xe000, "7" )

	PORT_START("AN_STEERING")
	PORT_BIT( 0x0f, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(10)

	PORT_START("AN_ACCELERATOR")
	PORT_BIT( 0x1f, 0x00, IPT_PEDAL ) PORT_MINMAX(0x00,0x1f) PORT_SENSITIVITY(25) PORT_KEYDELTA(10)

	PORT_START("AN_BRAKE")
	PORT_BIT( 0x1f, 0x00, IPT_PEDAL2 ) PORT_MINMAX(0x00,0x1f) PORT_SENSITIVITY(25) PORT_KEYDELTA(10)

	PORT_START("PPI_PORTC")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_SERVICE( 0x04, IP_ACTIVE_HIGH )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Gear Change") PORT_CODE(KEYCODE_SPACE) PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN3 )

	PORT_START("PPI_PORTD")
	/* Wire jumper setting on sound PCB */
	PORT_DIPNAME( 0xf0, 0x80,  DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x40, "3" )
	PORT_DIPSETTING(    0x80, "4" )
INPUT_PORTS_END

static INPUT_PORTS_START( tx1a )
	PORT_INCLUDE(tx1)

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x000c, 0x0000, "Game Cost" )
	PORT_DIPSETTING(      0x0000, "1 Coin Unit for 1 Credit" )
	PORT_DIPSETTING(      0x0004, "2 Coin Units for 1 Credit" )
	PORT_DIPSETTING(      0x0008, "3 Coin Units for 1 Credit" )
	PORT_DIPSETTING(      0x000c, "4 Coin Units for 1 Credit" )

	PORT_DIPNAME( 0x0010, 0x0000, "Left Coin Mechanism" )
	PORT_DIPSETTING(      0x0000, "1 Coin for 1 Coin Unit" )
	PORT_DIPSETTING(      0x0010, "1 Coin for 2 Coin Units" )

	PORT_DIPNAME( 0x0060, 0x0000, "Right Coin Mechanism" )
	PORT_DIPSETTING(      0x0000, "1 Coin for 1 Coin Unit" )
	PORT_DIPSETTING(      0x0020, "1 Coin for 4 Coin Units" )
	PORT_DIPSETTING(      0x0040, "1 Coin for 5 Coin Units" )
	PORT_DIPSETTING(      0x0060, "1 Coin for 6 Coin Units" )

	PORT_DIPNAME( 0xe000, 0xe000, "Bonus Adder" )
	PORT_DIPSETTING(      0x0000, "No Bonus" )
	PORT_DIPSETTING(      0x2000, "2 Coin Units for 1 Credit" )
	PORT_DIPSETTING(      0x4000, "3 Coin Units for 1 Credit" )
	PORT_DIPSETTING(      0x6000, "4 Coin Units for 1 Credit" )
	PORT_DIPSETTING(      0x8000, "5 Coin Units for 1 Credit" )
	PORT_DIPSETTING(      0xa000, "4 Coin Units for 2 Credit" )
	PORT_DIPSETTING(      0xc000, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(      0xe000, "No Bonus" )
INPUT_PORTS_END


static INPUT_PORTS_START( buggyboy )
	PORT_START("DSW")
	/* Dipswitch 0 is unconnected */
	PORT_DIPNAME( 0x0003, 0x0003, "Do not change 2" )
	PORT_DIPSETTING(      0x0000, "0" )
	PORT_DIPSETTING(      0x0001, "1" )
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0003, "3" )

	PORT_DIPNAME( 0x0004, 0x0004, "Message" )
	PORT_DIPSETTING(      0x0004, DEF_STR( English ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Japanese ) )

	PORT_DIPNAME( 0x0008, 0x0008, "Do not Change 3" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )

	PORT_DIPNAME( 0x0030, 0x0010, "Time Rank" )
	PORT_DIPSETTING(      0x0000, "A (Longest)" )
	PORT_DIPSETTING(      0x0010, "B" )
	PORT_DIPSETTING(      0x0020, "C" )
	PORT_DIPSETTING(      0x0030, "D (Shortest)" )

	PORT_DIPNAME( 0x00c0, 0x0040, "Game Rank" )
	PORT_DIPSETTING(      0x0000, "A (Easy)")
	PORT_DIPSETTING(      0x0040, "B" )
	PORT_DIPSETTING(      0x0080, "C" )
	PORT_DIPSETTING(      0x00c0, "D (Difficult)" )

	PORT_DIPNAME( 0xe000, 0x0000, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0xa000, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0xe000, "Free-Play" )

	PORT_DIPNAME( 0x1800, 0x0800, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 1C_5C ) )

	PORT_DIPNAME( 0x0700, 0x0700, "Do not change 1" )
	PORT_DIPSETTING(      0x0000, "0" )
	PORT_DIPSETTING(      0x0100, "1" )
	PORT_DIPSETTING(      0x0200, "2" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0400, "4" )
	PORT_DIPSETTING(      0x0500, "5" )
	PORT_DIPSETTING(      0x0600, "6" )
	PORT_DIPSETTING(      0x0700, "7" )

	PORT_START("PPI_PORTA")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Gear Change") PORT_CODE(KEYCODE_SPACE) PORT_TOGGLE
	PORT_SERVICE( 0x80, IP_ACTIVE_HIGH )

	PORT_START("PPI_PORTC")
	PORT_DIPNAME( 0xff, 0x80, "Sound PCB Jumper" )
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x20, "Speed Buggy/Data East" )
	PORT_DIPSETTING(    0x40, "Buggy Boy/Taito" )
	PORT_DIPSETTING(    0x80, "Buggy Boy/Tatsumi" )

	PORT_START("AN_STEERING")
	PORT_BIT( 0x0f, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(25)

	PORT_START("AN_ACCELERATOR")
	PORT_BIT( 0x1f, 0x00, IPT_PEDAL ) PORT_MINMAX(0x00,0x1f) PORT_SENSITIVITY(25) PORT_KEYDELTA(10)

	PORT_START("AN_BRAKE")
	PORT_BIT( 0x1f, 0x00, IPT_PEDAL2 ) PORT_MINMAX(0x00,0x1f) PORT_SENSITIVITY(25) PORT_KEYDELTA(10)
INPUT_PORTS_END

static INPUT_PORTS_START( buggybjr )
	PORT_START("DSW")
	/* Dipswitch 0 is unconnected */
	PORT_DIPNAME( 0x0003, 0x0003, "Do not change 2" )
	PORT_DIPSETTING(      0x0000, "0" )
	PORT_DIPSETTING(      0x0001, "1" )
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0003, "3" )

	PORT_DIPNAME( 0x0004, 0x0004, "Message" )
	PORT_DIPSETTING(      0x0004, DEF_STR( English ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Japanese ) )

	PORT_DIPNAME( 0x0008, 0x0008, "Do not Change 3" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )

	PORT_DIPNAME( 0x0030, 0x0010, "Time Rank" )
	PORT_DIPSETTING(      0x0000, "A (Longest)" )
	PORT_DIPSETTING(      0x0010, "B" )
	PORT_DIPSETTING(      0x0020, "C" )
	PORT_DIPSETTING(      0x0030, "D (Shortest)" )

	PORT_DIPNAME( 0x00c0, 0x0040, "Game Rank" )
	PORT_DIPSETTING(      0x0000, "A (Easy)")
	PORT_DIPSETTING(      0x0040, "B" )
	PORT_DIPSETTING(      0x0080, "C" )
	PORT_DIPSETTING(      0x00c0, "D (Difficult)" )

	PORT_DIPNAME( 0xe000, 0x0000, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0xa000, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0xe000, "Free-Play" )

	PORT_DIPNAME( 0x1800, 0x0800, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 1C_6C ) )

	PORT_DIPNAME( 0x0700, 0x0700, "Do not change 1" )
	PORT_DIPSETTING(      0x0000, "0" )
	PORT_DIPSETTING(      0x0100, "1" )
	PORT_DIPSETTING(      0x0200, "2" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0400, "4" )
	PORT_DIPSETTING(      0x0500, "5" )
	PORT_DIPSETTING(      0x0600, "6" )
	PORT_DIPSETTING(      0x0700, "7" )

	PORT_START("YM2149_IC19_A")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Gear Change") PORT_CODE(KEYCODE_SPACE) PORT_TOGGLE
	PORT_SERVICE( 0x80, IP_ACTIVE_HIGH )

	/* Wire jumper setting on sound PCB */
	PORT_START("YM2149_IC19_B")
	PORT_DIPNAME( 0xff, 0x80, "Sound PCB Jumper" )
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x20, "Speed Buggy/Data East" )
	PORT_DIPSETTING(    0x40, "Buggy Boy/Taito" )
	PORT_DIPSETTING(    0x80, "Buggy Boy/Tatsumi" )

	PORT_START("AN_STEERING")
	PORT_BIT( 0x0f, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(25)

	PORT_START("AN_ACCELERATOR")
	PORT_BIT( 0x1f, 0x00, IPT_PEDAL ) PORT_MINMAX(0x00, 0x1f) PORT_SENSITIVITY(25) PORT_KEYDELTA(10)

	PORT_START("AN_BRAKE")
	PORT_BIT( 0x1f, 0x00, IPT_PEDAL2 ) PORT_MINMAX(0x00, 0x1f) PORT_SENSITIVITY(25) PORT_KEYDELTA(10)
INPUT_PORTS_END

static READ16_HANDLER( dipswitches_r )
{
	return (input_port_read(space->machine, "DSW") & 0xfffe) | ts;
}

/*
    (TODO) TS: Connected in place of dipswitch A bit 0
    Accessed on startup as some sort of acknowledgement
*/
static WRITE8_HANDLER( ts_w )
{
//  TS = 1;
	z80_ram[offset] = data;
}

static READ8_HANDLER( ts_r )
{
//  TS = 1;
	return z80_ram[offset];
}


static WRITE8_DEVICE_HANDLER( tx1_coin_cnt_w )
{
	coin_counter_w(device->machine, 0, data & 0x80);
	coin_counter_w(device->machine, 1, data & 0x40);
//  coin_counter_w(device->machine, 2, data & 0x40);
}

static WRITE8_DEVICE_HANDLER( bb_coin_cnt_w )
{
	coin_counter_w(device->machine, 0, data & 0x01);
	coin_counter_w(device->machine, 1, data & 0x02);
//  coin_counter_w(device->machine, 2, data & 0x04);
}

static WRITE8_HANDLER( tx1_ppi_latch_w )
{
	tx1_ppi_latch_a = ((input_port_read(space->machine, "AN_BRAKE") & 0xf) << 4) | (input_port_read(space->machine, "AN_ACCELERATOR") & 0xf);
	tx1_ppi_latch_b = input_port_read(space->machine, "AN_STEERING");
}

static READ8_DEVICE_HANDLER( tx1_ppi_porta_r )
{
	return tx1_ppi_latch_a;
}

static READ8_DEVICE_HANDLER( tx1_ppi_portb_r )
{
	return input_port_read(device->machine, "PPI_PORTD") | tx1_ppi_latch_b;
}


static UINT8 bit_reverse8(UINT8 val)
{
	val = ((val & 0xF0) >>  4) | ((val & 0x0F) <<  4);
	val = ((val & 0xCC) >>  2) | ((val & 0x33) <<  2);
	val = ((val & 0xAA) >>  1) | ((val & 0x55) <<  1);

	return val;
}

static READ8_HANDLER( bb_analog_r )
{
	if (offset == 0)
		return bit_reverse8(((input_port_read(space->machine, "AN_ACCELERATOR") & 0xf) << 4) | input_port_read(space->machine, "AN_STEERING"));
	else
		return bit_reverse8((input_port_read(space->machine, "AN_BRAKE") & 0xf) << 4);
}

static READ8_HANDLER( bbjr_analog_r )
{
	if (offset == 0)
		return ((input_port_read(space->machine, "AN_ACCELERATOR") & 0xf) << 4) | input_port_read(space->machine, "AN_STEERING");
	else
		return (input_port_read(space->machine, "AN_BRAKE") & 0xf) << 4;
}


/*************************************
 *
 *  8255 PPI Interfaces
 *
 *************************************/

/* Buggy Boy uses an 8255 PPI instead of YM2149 ports for inputs! */
static const ppi8255_interface buggyboy_ppi8255_intf =
{
	DEVCB_INPUT_PORT("PPI_PORTA"),
	DEVCB_NULL,
	DEVCB_INPUT_PORT("PPI_PORTC"),
	DEVCB_NULL,
	DEVCB_HANDLER(bb_coin_cnt_w),
	DEVCB_NULL,
};


static const ppi8255_interface tx1_ppi8255_intf =
{
	DEVCB_HANDLER(tx1_ppi_porta_r),
	DEVCB_HANDLER(tx1_ppi_portb_r),
	DEVCB_INPUT_PORT("PPI_PORTC"),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_HANDLER(tx1_coin_cnt_w)
};


/*************************************
 *
 *  TX-1 Memory Maps
 *
 *************************************/

static ADDRESS_MAP_START( tx1_main, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x00000, 0x00fff) AM_MIRROR(0x1000) AM_RAM
	AM_RANGE(0x02000, 0x02fff) AM_MIRROR(0x1000) AM_RAM
	AM_RANGE(0x04000, 0x04fff) AM_MIRROR(0x1000) AM_RAM	AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE(0x06000, 0x06fff) AM_READWRITE(tx1_crtc_r, tx1_crtc_w)
	AM_RANGE(0x08000, 0x09fff) AM_RAM AM_BASE(&tx1_vram)
	AM_RANGE(0x0a000, 0x0afff) AM_RAM AM_SHARE("share1") AM_BASE(&tx1_rcram)
	AM_RANGE(0x0b000, 0x0b001) AM_READWRITE(dipswitches_r, z80_busreq_w)
	AM_RANGE(0x0c000, 0x0c001) AM_WRITE(tx1_scolst_w)
	AM_RANGE(0x0d000, 0x0d003) AM_WRITE(tx1_slincs_w)
	AM_RANGE(0x0e000, 0x0e001) AM_WRITE(tx1_slock_w)
	AM_RANGE(0x0f000, 0x0f001) AM_READWRITE(watchdog_reset16_r, resume_math_w)
	AM_RANGE(0x10000, 0x1ffff) AM_READWRITE(z80_shared_r, z80_shared_w)
	AM_RANGE(0xf0000, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( tx1_math, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x00000, 0x007ff) AM_RAM AM_BASE(&tx1_math_ram)
	AM_RANGE(0x00800, 0x00fff) AM_READWRITE(tx1_spcs_ram_r, tx1_spcs_ram_w)
	AM_RANGE(0x01000, 0x01fff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x02000, 0x022ff) AM_RAM AM_BASE(&tx1_objram) AM_SIZE(&tx1_objram_size)
	AM_RANGE(0x02400, 0x027ff) AM_WRITE(tx1_bankcs_w)
	AM_RANGE(0x02800, 0x02bff) AM_WRITE(halt_math_w)
	AM_RANGE(0x02C00, 0x02fff) AM_WRITE(tx1_flgcs_w)
	AM_RANGE(0x03000, 0x03fff) AM_READWRITE(tx1_math_r, tx1_math_w)
	AM_RANGE(0x05000, 0x07fff) AM_READ(tx1_spcs_rom_r)
	AM_RANGE(0x08000, 0x0bfff) AM_ROM
	AM_RANGE(0x0c000, 0x0ffff) AM_ROM
	AM_RANGE(0xfc000, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( tx1_sound_prg, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x3000, 0x37ff) AM_RAM AM_MIRROR(0x800) AM_BASE(&z80_ram)
	AM_RANGE(0x4000, 0x4000) AM_WRITE(z80_intreq_w)
	AM_RANGE(0x5000, 0x5003) AM_DEVREADWRITE("ppi8255", ppi8255_r, ppi8255_w)
	AM_RANGE(0x6000, 0x6003) AM_READWRITE(tx1_pit8253_r, tx1_pit8253_w)
	AM_RANGE(0x7000, 0x7fff) AM_WRITE(tx1_ppi_latch_w)
	AM_RANGE(0xb000, 0xbfff) AM_READWRITE(ts_r, ts_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( tx1_sound_io, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x40, 0x41) AM_DEVWRITE("aysnd", ay8910_data_address_w)
ADDRESS_MAP_END


/*************************************
 *
 *  Buggy Boy Memory Maps
 *
 *************************************/

static ADDRESS_MAP_START( buggyboy_main, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x00000, 0x03fff) AM_RAM AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE(0x04000, 0x04fff) AM_READWRITE(tx1_crtc_r, tx1_crtc_w)
	AM_RANGE(0x08000, 0x09fff) AM_RAM AM_BASE(&buggyboy_vram)
	AM_RANGE(0x0a000, 0x0afff) AM_RAM AM_SHARE("share1") AM_BASE(&buggyboy_rcram) AM_SIZE(&buggyboy_rcram_size)
	AM_RANGE(0x0b000, 0x0b001) AM_READWRITE(dipswitches_r, z80_busreq_w)
	AM_RANGE(0x0c000, 0x0c001) AM_WRITE(buggyboy_scolst_w)
	AM_RANGE(0x0d000, 0x0d003) AM_WRITE(buggyboy_slincs_w)
	AM_RANGE(0x0e000, 0x0e001) AM_WRITE(buggyboy_sky_w)
	AM_RANGE(0x0f000, 0x0f003) AM_READWRITE(watchdog_reset16_r, resume_math_w)
	AM_RANGE(0x10000, 0x1ffff) AM_READWRITE(z80_shared_r, z80_shared_w)
	AM_RANGE(0x20000, 0x2ffff) AM_ROM
	AM_RANGE(0xf0000, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( buggybjr_main, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x00000, 0x03fff) AM_RAM AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE(0x04000, 0x04fff) AM_READWRITE(tx1_crtc_r, tx1_crtc_w)
	AM_RANGE(0x08000, 0x08fff) AM_RAM AM_BASE(&buggyboy_vram)
	AM_RANGE(0x0a000, 0x0afff) AM_RAM AM_SHARE("share1") AM_BASE(&buggyboy_rcram) AM_SIZE(&buggyboy_rcram_size)
	AM_RANGE(0x0b000, 0x0b001) AM_READWRITE(dipswitches_r, z80_busreq_w)
	AM_RANGE(0x0c000, 0x0c001) AM_WRITE(buggyboy_scolst_w)
	AM_RANGE(0x0d000, 0x0d003) AM_WRITE(buggyboy_slincs_w)
	AM_RANGE(0x0e000, 0x0e001) AM_WRITE(buggyboy_sky_w)
	AM_RANGE(0x0f000, 0x0f003) AM_READWRITE(watchdog_reset16_r, resume_math_w)
	AM_RANGE(0x10000, 0x1ffff) AM_READWRITE(z80_shared_r, z80_shared_w)
	AM_RANGE(0x20000, 0x2ffff) AM_ROM
	AM_RANGE(0xf0000, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( buggyboy_math, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x00000, 0x007ff) AM_RAM AM_BASE(&tx1_math_ram)
	AM_RANGE(0x00800, 0x00fff) AM_READWRITE(buggyboy_spcs_ram_r, buggyboy_spcs_ram_w)
	AM_RANGE(0x01000, 0x01fff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x02000, 0x022ff) AM_RAM AM_BASE(&buggyboy_objram) AM_SIZE(&buggyboy_objram_size)
	AM_RANGE(0x02400, 0x024ff) AM_WRITE(buggyboy_gas_w)
	AM_RANGE(0x03000, 0x03fff) AM_READWRITE(buggyboy_math_r, buggyboy_math_w)
	AM_RANGE(0x04000, 0x04fff) AM_ROM
	AM_RANGE(0x05000, 0x07fff) AM_READ(buggyboy_spcs_rom_r)
	AM_RANGE(0x08000, 0x0bfff) AM_ROM
	AM_RANGE(0x0c000, 0x0ffff) AM_ROM
	AM_RANGE(0xfc000, 0xfffff) AM_ROM
ADDRESS_MAP_END

/* Buggy Boy Sound PCB TC033A */
static ADDRESS_MAP_START( buggyboy_sound_prg, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x47ff) AM_RAM AM_BASE(&z80_ram)
	AM_RANGE(0x6000, 0x6001) AM_READ(bb_analog_r)
	AM_RANGE(0x6800, 0x6803) AM_DEVREADWRITE("ppi8255", ppi8255_r, ppi8255_w)
	AM_RANGE(0x7000, 0x7003) AM_READWRITE(tx1_pit8253_r, tx1_pit8253_w)
	AM_RANGE(0x7800, 0x7800) AM_WRITE(z80_intreq_w)
	AM_RANGE(0xc000, 0xc7ff) AM_READWRITE(ts_r, ts_w)
ADDRESS_MAP_END

/* Buggy Boy Jr Sound PCB TC043 */
static ADDRESS_MAP_START( buggybjr_sound_prg, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x47ff) AM_RAM AM_BASE(&z80_ram)
	AM_RANGE(0x5000, 0x5003) AM_READWRITE(tx1_pit8253_r, tx1_pit8253_w)
	AM_RANGE(0x6000, 0x6001) AM_READ(bbjr_analog_r)
	AM_RANGE(0x7000, 0x7000) AM_WRITE(z80_intreq_w)
	AM_RANGE(0xc000, 0xc7ff) AM_READWRITE(ts_r, ts_w)
ADDRESS_MAP_END

/* Common */
static ADDRESS_MAP_START( buggyboy_sound_io, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x40, 0x40) AM_DEVREAD("ym1", ay8910_r)
	AM_RANGE(0x40, 0x41) AM_DEVWRITE("ym1", ay8910_data_address_w)
	AM_RANGE(0x80, 0x80) AM_DEVREAD("ym2", ay8910_r)
	AM_RANGE(0x80, 0x81) AM_DEVWRITE("ym2", ay8910_data_address_w)
ADDRESS_MAP_END


/*************************************
 *
 *  Sound Hardware
 *
 *************************************/

static const ay8910_interface tx1_ay8910_interface =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_HANDLER(tx1_ay8910_a_w),
	DEVCB_HANDLER(tx1_ay8910_b_w),
};


static const ay8910_interface buggyboy_ym2149_interface_1 =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_HANDLER(bb_ym1_a_w),
	DEVCB_NULL,
};

static const ay8910_interface buggyboy_ym2149_interface_2 =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_HANDLER(bb_ym2_a_w),
	DEVCB_HANDLER(bb_ym2_b_w),
};


/* YM2149 IC19 */
static const ay8910_interface buggybjr_ym2149_interface_1 =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_INPUT_PORT("YM2149_IC19_A"),
	DEVCB_INPUT_PORT("YM2149_IC19_B"),
	DEVCB_NULL,
	DEVCB_NULL,
};

/* YM2149 IC24 */
static const ay8910_interface buggybjr_ym2149_interface_2 =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_HANDLER(bb_ym2_a_w),
	DEVCB_HANDLER(bb_ym2_b_w),
};


/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_DRIVER_START( tx1 )
	MDRV_CPU_ADD("main_cpu", I8086, CPU_MASTER_CLOCK / 3)
	MDRV_CPU_PROGRAM_MAP(tx1_main)
//  MDRV_WATCHDOG_TIME_INIT(5)

	MDRV_CPU_ADD("math_cpu", I8086, CPU_MASTER_CLOCK / 3)
	MDRV_CPU_PROGRAM_MAP(tx1_math)

	MDRV_CPU_ADD("audio_cpu", Z80, TX1_PIXEL_CLOCK / 2)
	MDRV_CPU_PROGRAM_MAP(tx1_sound_prg)
	MDRV_CPU_IO_MAP(tx1_sound_io)
	MDRV_CPU_PERIODIC_INT(irq0_line_hold, TX1_PIXEL_CLOCK / 4 / 2048 / 2)

	MDRV_MACHINE_RESET(tx1)
	MDRV_NVRAM_HANDLER(generic_0fill)

	MDRV_PPI8255_ADD("ppi8255", tx1_ppi8255_intf)

	MDRV_PALETTE_LENGTH(256)
	MDRV_PALETTE_INIT(tx1)
	MDRV_VIDEO_EOF(tx1)

	MDRV_DEFAULT_LAYOUT(layout_triphsxs)

	MDRV_SCREEN_ADD("lscreen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_RAW_PARAMS(TX1_PIXEL_CLOCK, TX1_HTOTAL, TX1_HBEND, TX1_HBSTART, TX1_VTOTAL, TX1_VBEND, TX1_VBSTART)

	MDRV_SCREEN_ADD("cscreen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_RAW_PARAMS(TX1_PIXEL_CLOCK, TX1_HTOTAL, TX1_HBEND, TX1_HBSTART, TX1_VTOTAL, TX1_VBEND, TX1_VBSTART)

	MDRV_SCREEN_ADD("rscreen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_RAW_PARAMS(TX1_PIXEL_CLOCK, TX1_HTOTAL, TX1_HBEND, TX1_HBSTART, TX1_VTOTAL, TX1_VBEND, TX1_VBSTART)

	MDRV_VIDEO_START(tx1)
	MDRV_VIDEO_UPDATE(tx1)

	MDRV_SPEAKER_STANDARD_STEREO("frontleft", "frontright")
//  MDRV_SPEAKER_STANDARD_STEREO("rearleft", "rearright")

	MDRV_SOUND_ADD("aysnd", AY8910, TX1_PIXEL_CLOCK / 8)
	MDRV_SOUND_CONFIG(tx1_ay8910_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "frontleft", 0.1)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "frontright", 0.1)

	MDRV_SOUND_ADD("tx1", TX1, 0)
	MDRV_SOUND_ROUTE(0, "frontleft", 0.2)
	MDRV_SOUND_ROUTE(1, "frontright", 0.2)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( buggyboy )
	MDRV_CPU_ADD("main_cpu", I8086, CPU_MASTER_CLOCK / 3)
	MDRV_CPU_PROGRAM_MAP(buggyboy_main)
//  MDRV_WATCHDOG_TIME_INIT(5)

	MDRV_CPU_ADD("math_cpu", I8086, CPU_MASTER_CLOCK / 3)
	MDRV_CPU_PROGRAM_MAP(buggyboy_math)

	MDRV_CPU_ADD("audio_cpu", Z80, BUGGYBOY_ZCLK / 2)
	MDRV_CPU_PROGRAM_MAP(buggyboy_sound_prg)
	MDRV_CPU_PERIODIC_INT(z80_irq, BUGGYBOY_ZCLK / 2 / 4 / 2048)
	MDRV_CPU_IO_MAP(buggyboy_sound_io)

	MDRV_MACHINE_RESET(buggyboy)
	MDRV_NVRAM_HANDLER(generic_0fill)

	MDRV_PPI8255_ADD("ppi8255", buggyboy_ppi8255_intf)

	MDRV_DEFAULT_LAYOUT(layout_triphsxs)

	MDRV_SCREEN_ADD("lscreen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_RAW_PARAMS(BB_PIXEL_CLOCK, BB_HTOTAL, BB_HBEND, BB_HBSTART, BB_VTOTAL, BB_VBEND, BB_VBSTART)

	MDRV_SCREEN_ADD("cscreen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_RAW_PARAMS(BB_PIXEL_CLOCK, BB_HTOTAL, BB_HBEND, BB_HBSTART, BB_VTOTAL, BB_VBEND, BB_VBSTART)

	MDRV_SCREEN_ADD("rscreen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_RAW_PARAMS(BB_PIXEL_CLOCK, BB_HTOTAL, BB_HBEND, BB_HBSTART, BB_VTOTAL, BB_VBEND, BB_VBSTART)

	MDRV_PALETTE_LENGTH(256)
	MDRV_PALETTE_INIT(buggyboy)
	MDRV_VIDEO_START(buggyboy)
	MDRV_VIDEO_UPDATE(buggyboy)
	MDRV_VIDEO_EOF(buggyboy)

	MDRV_SPEAKER_STANDARD_STEREO("frontleft", "frontright")
//  MDRV_SPEAKER_STANDARD_STEREO("rearleft", "rearright")

	MDRV_SOUND_ADD("ym1", YM2149, BUGGYBOY_ZCLK / 4)
	MDRV_SOUND_CONFIG(buggyboy_ym2149_interface_1)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "frontleft", 0.15)

	MDRV_SOUND_ADD("ym2", YM2149, BUGGYBOY_ZCLK / 4)
	MDRV_SOUND_CONFIG(buggyboy_ym2149_interface_2)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "frontright", 0.15)

	MDRV_SOUND_ADD("buggyboy", BUGGYBOY, 0)
	MDRV_SOUND_ROUTE(0, "frontleft", 0.2)
	MDRV_SOUND_ROUTE(1, "frontright", 0.2)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( buggybjr )
	MDRV_CPU_ADD("main_cpu", I8086, CPU_MASTER_CLOCK / 3)
	MDRV_CPU_PROGRAM_MAP(buggybjr_main)
//  MDRV_WATCHDOG_TIME_INIT(5)

	MDRV_CPU_ADD("math_cpu", I8086, CPU_MASTER_CLOCK / 3)
	MDRV_CPU_PROGRAM_MAP(buggyboy_math)

	MDRV_CPU_ADD("audio_cpu", Z80, BUGGYBOY_ZCLK / 2)
	MDRV_CPU_PROGRAM_MAP(buggybjr_sound_prg)
	MDRV_CPU_IO_MAP(buggyboy_sound_io)
	MDRV_CPU_PERIODIC_INT(z80_irq, BUGGYBOY_ZCLK / 2 / 4 / 2048)

	MDRV_MACHINE_RESET(buggybjr)
	MDRV_NVRAM_HANDLER(generic_0fill)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)

	MDRV_SCREEN_RAW_PARAMS(BB_PIXEL_CLOCK, BB_HTOTAL, BB_HBEND, BB_HBSTART, BB_VTOTAL, BB_VBEND, BB_VBSTART)

	MDRV_PALETTE_LENGTH(256)
	MDRV_PALETTE_INIT(buggyboy)
	MDRV_VIDEO_START(buggybjr)
	MDRV_VIDEO_UPDATE(buggybjr)
	MDRV_VIDEO_EOF(buggyboy)

	MDRV_SPEAKER_STANDARD_STEREO("frontleft", "frontright")
//  MDRV_SPEAKER_STANDARD_STEREO("rearleft", "rearright")

	MDRV_SOUND_ADD("ym1", YM2149, BUGGYBOY_ZCLK / 4)
	MDRV_SOUND_CONFIG(buggybjr_ym2149_interface_1)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "frontleft", 0.15)

	MDRV_SOUND_ADD("ym2", YM2149, BUGGYBOY_ZCLK / 4)
	MDRV_SOUND_CONFIG(buggybjr_ym2149_interface_2)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "frontright", 0.15)

	MDRV_SOUND_ADD("buggyboy", BUGGYBOY, 0)
	MDRV_SOUND_ROUTE(0, "frontleft", 0.2)
	MDRV_SOUND_ROUTE(1, "frontright", 0.2)
MACHINE_DRIVER_END


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( tx1 )
	ROM_REGION( 0x100000, "main_cpu", 0 )
	ROM_LOAD16_BYTE( "tx1_1c.ic22", 0xf0000, 0x4000, CRC(eedcee83) SHA1(7fa0590b142fb13c6562126a9bdd5a1e032880c7) )
	ROM_LOAD16_BYTE( "tx1_2c.ic29", 0xf0001, 0x4000, CRC(294bf5bf) SHA1(02b425caba8a187c58211bab27988205eb044558) )
	ROM_LOAD16_BYTE( "tx1_4c.ic54", 0xf8001, 0x4000, CRC(15bb8ef2) SHA1(83968f010ec555fcd0548a80562fb23a892b5afb) )
	ROM_LOAD16_BYTE( "tx1_3c.ic45", 0xf8000, 0x4000, CRC(21a8aa55) SHA1(21bc4adefb22a95fcd7a4e305bf0b05e2cb34129) )

	ROM_REGION( 0x100000, "math_cpu", 0 )
	ROM_LOAD16_BYTE( "tx1_9c.ic146", 0xfc000, 0x2000, CRC(b65eeea2) SHA1(b5f26e17520c598132b93c5cd7af7ebd03b10012) )
	ROM_RELOAD(                  0x4000, 0x2000 )
	ROM_RELOAD(                  0x8000, 0x2000 )
	ROM_RELOAD(                  0xc000, 0x2000 )
	ROM_LOAD16_BYTE( "tx1_8c.ic132", 0xfc001, 0x2000, CRC(0d63dadc) SHA1(0954174b25c08967d3efb31f5721fd05502d66dd) )
	ROM_RELOAD(                  0x4001, 0x2000 )
	ROM_RELOAD(                  0x8001, 0x2000 )
	ROM_RELOAD(                  0xc001, 0x2000 )

	ROM_REGION( 0x10000, "audio_cpu", 0 )
	ROM_LOAD( "8411-136027-157.11", 0x00000, 0x2000, CRC(10ae3075) SHA1(69c5f62f2473aba848383eed3cecf15e273d86ca) )

	ROM_REGION( 0x20000, "char_tiles", 0 )
	ROM_LOAD( "tx1_21a.ic204", 0x0000, 0x4000, CRC(cd3441ad) SHA1(8e6597b3177b8aaa34ed3373d85fc4b6231e1333) )
	ROM_LOAD( "tx1_20a.ic174", 0x4000, 0x4000, CRC(dbe595fc) SHA1(1ed2f775f0a1b46a2ffbc056eb4ef732ed546d3c) )

	ROM_REGION( 0x40000, "obj_tiles", 0 )
	ROM_LOAD( "tx1_16b.ic203", 0x0000, 0x4000, CRC(1141c965) SHA1(4b90c1428bcbd72d0449c064856a5596269b3fc6) )
	ROM_LOAD( "tx1_18b.ic258", 0x4000, 0x4000, NO_DUMP )
	ROM_LOAD( "tx1_15b.ic173", 0x8000, 0x4000, CRC(30d1a8d5) SHA1(b4c585b7b8a8920bb3949d643e9e10c17d4009a0) )
	ROM_LOAD( "tx1_17b.ic232", 0xc000, 0x4000, CRC(364bb354) SHA1(a26581ca1088b979285471e2c6595048df84d75e) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "tx1_5a.ic56", 0x0000, 0x2000, CRC(5635b8c1) SHA1(5cc9437a2ff0843f1917f2451852d4561c240b24) )
	ROM_LOAD( "tx1_6a.ic66", 0x2000, 0x2000, CRC(03d83cf8) SHA1(5c0cfc6bf02ad2b3f37e1ceb493f69eb9829ab1e) )
	ROM_LOAD( "tx1_7a.ic76", 0x4000, 0x2000, CRC(ad56013a) SHA1(ae3a91f58f30daff334754476db33ad1d12569fc) )

	ROM_REGION( 0x10000, "au_data", ROMREGION_LE )
	ROM_LOAD16_BYTE( "tx1_10b.ic184", 0x0000, 0x4000, CRC(acf754e8) SHA1(06779e18636f0799efdaa09396e9ccd59f426257) )
	ROM_LOAD16_BYTE( "tx1_11b.ic185", 0x0001, 0x4000, CRC(f89d3e20) SHA1(4b4cf679b7e3d63cded9989d2b667941f718ff57) )
	ROM_LOAD16_BYTE( "xb02b.ic223",   0x8000, 0x0200, CRC(22c77af6) SHA1(1be8585b95316b4fc5712cdaef699e676320cd4d) )
	ROM_LOAD16_BYTE( "xb01b.ic213",   0x8001, 0x0200, CRC(f6b8b70b) SHA1(b79374acf11d71db1e4ad3c494ac5f500a52677b) )

	ROM_REGION( 0x50000, "obj_map", 0 )
	ROM_LOAD( "tx1_14b.ic106", 0x0000, 0x4000, CRC(68c63d6e) SHA1(110e02b99c44d31041be588bd14642e26890ecbd) )
	ROM_LOAD( "tx1_13b.ic73",  0x4000, 0x4000, CRC(b0c581b2) SHA1(20926bc15e7c97045b219b828acfcdd99b8712a6) )

	ROM_REGION( 0x50000, "user3", 0 )
	ROM_LOAD( "tx1_12b.ic48",  0x0000, 0x2000, CRC(4b3d7956) SHA1(fc2432dd69f3be7007d4fd6f7c86c7c19453b1ba) )
	ROM_LOAD( "tx1_19b.ic281", 0x2000, 0x4000, NO_DUMP )

	ROM_REGION( 0x10000, "proms", 0 )
	/* RGB palette (left) */
	ROM_LOAD( "xb05a.ic57", 0x0000, 0x100, CRC(3b387d01) SHA1(1229548e3052ad34eeee9598743091d19f6b8f88) )
	ROM_LOAD( "xb06a.ic58", 0x0100, 0x100, CRC(f6f4d7d9) SHA1(866024b76b26d6942bd4e1d2494686299414f6be) )
	ROM_LOAD( "xb07a.ic59", 0x0200, 0x100, CRC(824e7532) SHA1(917ce74d2bae6af90f2c4e41d12a69f884320915) )

	/* RGB palette (center) */
	ROM_LOAD( "xb05a.ic36", 0x0300, 0x100, CRC(3b387d01) SHA1(1229548e3052ad34eeee9598743091d19f6b8f88) )
	ROM_LOAD( "xb06a.ic37", 0x0400, 0x100, CRC(f6f4d7d9) SHA1(866024b76b26d6942bd4e1d2494686299414f6be) )
	ROM_LOAD( "xb07a.ic38", 0x0500, 0x100, CRC(824e7532) SHA1(917ce74d2bae6af90f2c4e41d12a69f884320915) )

	/* RGB palette (right) */
	ROM_LOAD( "xb05a.ic8",  0x0600, 0x100, CRC(3b387d01) SHA1(1229548e3052ad34eeee9598743091d19f6b8f88) )
	ROM_LOAD( "xb06a.ic9",  0x0700, 0x100, CRC(f6f4d7d9) SHA1(866024b76b26d6942bd4e1d2494686299414f6be) )
	ROM_LOAD( "xb07a.ic10", 0x0800, 0x100, CRC(824e7532) SHA1(917ce74d2bae6af90f2c4e41d12a69f884320915) )

	/* Character colour tables (L, C, R) */
	ROM_LOAD( "xb08.ic85",  0x0900, 0x100, CRC(5aeef5cc) SHA1(e123bf01d556178b0cf9d495bcce445f3f8421cd) )
	ROM_LOAD( "xb08.ic116", 0x0a00, 0x100, CRC(5aeef5cc) SHA1(e123bf01d556178b0cf9d495bcce445f3f8421cd) )
	ROM_LOAD( "xb08.ic148", 0x0b00, 0x100, CRC(5aeef5cc) SHA1(e123bf01d556178b0cf9d495bcce445f3f8421cd) )

	/* Object colour table */
	ROM_LOAD( "xb04a.ic276",0x0c00, 0x200, CRC(92bf5533) SHA1(4d9127417325af66099234178ab2641d23ee9d22) )
	ROM_LOAD( "xb04a.ic277",0x0e00, 0x200, CRC(92bf5533) SHA1(4d9127417325af66099234178ab2641d23ee9d22) )

	/* Object tile lookup */
	ROM_LOAD( "xb03a.ic25", 0x1000, 0x100, CRC(616a7a85) SHA1(b7c1060ecb128154092441212de64dc304aa3fcd) )

	/* Road graphics */
	ROM_LOAD( "xb09.ic33",  0x1100, 0x200, CRC(fafb6917) SHA1(30eb182c7623026dce7dba9e249bc8a9eb7a7f3e) )
	ROM_LOAD( "xb10.ic40",  0x1300, 0x200, CRC(93deb894) SHA1(5ae9a21298c836fe649a52f3df2b4067f9012b91) )
	ROM_LOAD( "xb11.ic49",  0x1500, 0x200, CRC(aa5ed232) SHA1(f33e7bc2dd33ac6d75fb06b93c4dd58e5d10010d) )

	/* Road stripes */
	ROM_LOAD( "xb12.ic50",  0x1700, 0x200, CRC(6b424cea) SHA1(83127326c20116b0a4be1126e163f9c6755e19dc) )
ROM_END

/* Some PROMs haven't been confirmed to be the same as the Tatsumi set (but are very likely identical) */
ROM_START( tx1a )
	ROM_REGION( 0x100000, "main_cpu", 0 )
	ROM_LOAD16_BYTE( "8412-136027-244.22", 0xf0000, 0x4000, CRC(2e9cefa2) SHA1(4ca04eae446e8df08ab793488a79217ed1a27875) )
	ROM_LOAD16_BYTE( "8412-136027-245.29", 0xf0001, 0x4000, CRC(ade7895c) SHA1(1c33a574cae46fddb4cadb85f5de17f02ae7a596) )
	ROM_LOAD16_BYTE( "8412-136027-250.54", 0xf8001, 0x4000, CRC(c8c9368f) SHA1(0972d54d506216eb2b204cf22ccdff9210fb7b10) )
	ROM_LOAD16_BYTE( "8412-136027-249.45", 0xf8000, 0x4000, CRC(9bcb82db) SHA1(d1528c9b9c4c2848bdba15e4632927476d544f40))

	ROM_REGION( 0x100000, "math_cpu", 0 )
	ROM_LOAD16_BYTE( "8411-136027-152.146", 0xfc000, 0x2000, CRC(b65eeea2) SHA1(b5f26e17520c598132b93c5cd7af7ebd03b10012) )
	ROM_RELOAD(                  0x4000, 0x2000 )
	ROM_RELOAD(                  0x8000, 0x2000 )
	ROM_RELOAD(                  0xc000, 0x2000 )
	ROM_LOAD16_BYTE( "8411-136027-151.132", 0xfc001, 0x2000, CRC(0d63dadc) SHA1(0954174b25c08967d3efb31f5721fd05502d66dd) )
	ROM_RELOAD(                  0x4001, 0x2000 )
	ROM_RELOAD(                  0x8001, 0x2000 )
	ROM_RELOAD(                  0xc001, 0x2000 )

	ROM_REGION( 0x10000, "audio_cpu", 0 )
	ROM_LOAD( "8411-136027-157.11", 0x00000, 0x2000, CRC(10ae3075) SHA1(69c5f62f2473aba848383eed3cecf15e273d86ca) )

	ROM_REGION( 0x20000, "char_tiles", 0 )
	ROM_LOAD( "8411-136027-156.204", 0x0000, 0x4000, CRC(60f3c616) SHA1(59c4361891e4274e27e6279c919e8fd6803af7cf) )
	ROM_LOAD( "8411-136027-155.174", 0x4000, 0x4000, CRC(e59a6b72) SHA1(c10efa77ab421ac60b97227a8d547f50f8415670) )

	ROM_REGION( 0x40000, "obj_tiles", 0 )
	ROM_LOAD( "8411-136027-114.203", 0x0000, 0x4000, CRC(fc91328b) SHA1(e57fd2056b65d37cf2e1f0af56616c6555df3006) )
	ROM_LOAD( "8411-136027-116.258", 0x4000, 0x4000, CRC(5745f671) SHA1(6e471633cd6de9926b3361a84430c088e1f6a097) )
	ROM_LOAD( "8411-136027-115.173", 0x8000, 0x4000, CRC(720e5873) SHA1(151d9063c35b26f5876cf94bdf0c2665ec701bbd) )
	ROM_LOAD( "8411-136027-117.232", 0xc000, 0x4000, CRC(3c68d0bc) SHA1(2dbaf2a268b90214fd61c016ac945d4371057826) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "8411-136027-146.56", 0x0000, 0x2000, CRC(5635b8c1) SHA1(5cc9437a2ff0843f1917f2451852d4561c240b24) )
	ROM_LOAD( "8411-136027-147.66", 0x2000, 0x2000, CRC(03d83cf8) SHA1(5c0cfc6bf02ad2b3f37e1ceb493f69eb9829ab1e) )
	ROM_LOAD( "8411-136027-148.76", 0x4000, 0x2000, CRC(ad56013a) SHA1(ae3a91f58f30daff334754476db33ad1d12569fc) )

	ROM_REGION( 0x10000, "au_data", ROMREGION_LE )
	ROM_LOAD16_BYTE( "8411-136027-153.184", 0x0000, 0x4000, CRC(acf754e8) SHA1(06779e18636f0799efdaa09396e9ccd59f426257) )
	ROM_LOAD16_BYTE( "8411-136027-154.185", 0x0001, 0x4000, CRC(f89d3e20) SHA1(4b4cf679b7e3d63cded9989d2b667941f718ff57) )
	ROM_LOAD16_BYTE( "xb02b.ic223",         0x8000, 0x0200, CRC(22c77af6) SHA1(1be8585b95316b4fc5712cdaef699e676320cd4d) )
	ROM_LOAD16_BYTE( "xb01b.ic213",         0x8001, 0x0200, CRC(f6b8b70b) SHA1(b79374acf11d71db1e4ad3c494ac5f500a52677b) )

	ROM_REGION( 0x50000, "obj_map", 0 )
	ROM_LOAD( "8411-136027-119.106", 0x0000, 0x4000, CRC(88eec0fb) SHA1(81d7a69dc1a4b3b81d7f28d97a3f80697cdcc6eb) )
	ROM_LOAD( "8411-136027-120.73",  0x4000, 0x4000, CRC(407cbe65) SHA1(e1c11b65f3c6abde6d55afeaffdb39cdd6d66377) )

	ROM_REGION( 0x50000, "user3", 0 )
	ROM_LOAD( "8411-136027-113.48",  0x0000, 0x2000, CRC(4b3d7956) SHA1(fc2432dd69f3be7007d4fd6f7c86c7c19453b1ba) )
	ROM_LOAD( "8411-136027-118.281", 0x2000, 0x4000, CRC(de418dc7) SHA1(1233e2f7499ec5a73a40ee336d3fe26c06187784) )

	ROM_REGION( 0x10000, "proms", 0 )
	/* RGB palette (left) */
	ROM_LOAD( "136027-133.57", 0x0000, 0x100, CRC(3b387d01) SHA1(1229548e3052ad34eeee9598743091d19f6b8f88) )
	ROM_LOAD( "136027-134.58", 0x0100, 0x100, CRC(f6f4d7d9) SHA1(866024b76b26d6942bd4e1d2494686299414f6be) )
	ROM_LOAD( "136027-135.59", 0x0200, 0x100, CRC(824e7532) SHA1(917ce74d2bae6af90f2c4e41d12a69f884320915) )

	/* RGB palette (center) */
	ROM_LOAD( "136027-133.36", 0x0300, 0x100, CRC(3b387d01) SHA1(1229548e3052ad34eeee9598743091d19f6b8f88) )
	ROM_LOAD( "136027-134.37", 0x0400, 0x100, CRC(f6f4d7d9) SHA1(866024b76b26d6942bd4e1d2494686299414f6be) )
	ROM_LOAD( "136027-135.38", 0x0500, 0x100, CRC(824e7532) SHA1(917ce74d2bae6af90f2c4e41d12a69f884320915) )

	/* RGB palette (right) */
	ROM_LOAD( "136027-133.8",  0x0600, 0x100, CRC(3b387d01) SHA1(1229548e3052ad34eeee9598743091d19f6b8f88) )
	ROM_LOAD( "136027-134.9",  0x0700, 0x100, CRC(f6f4d7d9) SHA1(866024b76b26d6942bd4e1d2494686299414f6be) )
	ROM_LOAD( "136027-135.10", 0x0800, 0x100, CRC(824e7532) SHA1(917ce74d2bae6af90f2c4e41d12a69f884320915) )

	/* Character colour tables (L, C, R) */
	ROM_LOAD( "136027-124.85",  0x0900, 0x100, CRC(5aeef5cc) SHA1(e123bf01d556178b0cf9d495bcce445f3f8421cd) )
	ROM_LOAD( "136027-124.116", 0x0a00, 0x100, CRC(5aeef5cc) SHA1(e123bf01d556178b0cf9d495bcce445f3f8421cd) )
	ROM_LOAD( "136027-124.148", 0x0b00, 0x100, CRC(5aeef5cc) SHA1(e123bf01d556178b0cf9d495bcce445f3f8421cd) )

	/* Object colour table */
	ROM_LOAD( "136027-136.276", 0x0c00, 0x200, CRC(7b675b8b) SHA1(3a7617d8ca29aa5d9832e317736598646a466b8b) )
	ROM_LOAD( "136027-136.277", 0x0e00, 0x200, CRC(7b675b8b) SHA1(3a7617d8ca29aa5d9832e317736598646a466b8b) )

	/* Object tile lookup */
	ROM_LOAD( "136027-123.25", 0x1000, 0x100, CRC(616a7a85) SHA1(b7c1060ecb128154092441212de64dc304aa3fcd) )

	/* Road graphics */
	ROM_LOAD( "xb09.ic33",  0x1100, 0x200, CRC(fafb6917) SHA1(30eb182c7623026dce7dba9e249bc8a9eb7a7f3e) )
	ROM_LOAD( "xb10.ic40",  0x1300, 0x200, CRC(93deb894) SHA1(5ae9a21298c836fe649a52f3df2b4067f9012b91) )
	ROM_LOAD( "xb11.ic49",  0x1500, 0x200, CRC(aa5ed232) SHA1(f33e7bc2dd33ac6d75fb06b93c4dd58e5d10010d) )

	/* Road stripes */
	ROM_LOAD( "xb12.ic50",  0x1700, 0x200, CRC(6b424cea) SHA1(83127326c20116b0a4be1126e163f9c6755e19dc) )
ROM_END

ROM_START( buggyboy )
	ROM_REGION( 0x100000, "main_cpu", 0 )
	ROM_LOAD16_BYTE( "bug1a.230", 0x20000, 0x8000, CRC(92797c25) SHA1(8f7434abbd7f557d3202abb01b1e4899c82c67a5) )
	ROM_LOAD16_BYTE( "bug4a.173", 0x20001, 0x8000, CRC(40ce3930) SHA1(4bf62ebeea1549a13a21a32cb860717f064b186a) )

	ROM_LOAD16_BYTE( "bug2d.231", 0xf0000, 0x4000, CRC(f67bd593) SHA1(b6e3f9d5534b0addbba4aa4c813dda21a27cafa2) )
	ROM_LOAD16_BYTE( "bug5d.174", 0xf0001, 0x4000, CRC(d0dd6ffc) SHA1(377581ace86ea0f713aa7dc8f96f27cbcda1b2ea) )

	ROM_LOAD16_BYTE( "bug3b.232", 0xf8000, 0x4000, CRC(43cce3f0) SHA1(17de3728d809d386f6a7a330c8c8701975d4ebed) )
	ROM_LOAD16_BYTE( "bug6b.175", 0xf8001, 0x4000, CRC(8f000dfa) SHA1(5fd78a03a00f547bbb431839f78a8d10a4ba8e3e) )

	ROM_REGION( 0x100000, "math_cpu", 0 )
	ROM_LOAD16_BYTE( "bug8a.061", 0x4000, 0x2000, CRC(512291cd) SHA1(60f87133c86b88b982ba4680f96d0ac55970cb8d) )
	ROM_RELOAD(                  0x8000,  0x2000 )
	ROM_RELOAD(                  0xc000,  0x2000 )
	ROM_RELOAD(                  0xfc000, 0x2000 )

	ROM_LOAD16_BYTE( "bug7a.060", 0x4001, 0x2000, CRC(d24dfdef) SHA1(37d05a8bf9567380523df01265afb9780e39ea2a) )
	ROM_RELOAD(                  0x8001,  0x2000 )
	ROM_RELOAD(                  0xc001,  0x2000 )
	ROM_RELOAD(                  0xfc001, 0x2000 )

	ROM_REGION( 0x10000, "audio_cpu", 0 )
	ROM_LOAD( "bug35.11", 0x00000, 0x4000,  CRC(7aa16e9e) SHA1(ea54e56270f70351a62a78fa32027bb41ef9861e) )

	ROM_REGION( 0x8000, "char_tiles", 0 )
	ROM_LOAD( "bug34a",     0x00000, 0x4000, CRC(8c64eb3c) SHA1(e0846f9a4811a0b6561d7a9aa7d50c23ced8d1d3) )
	ROM_LOAD( "bug33a.168", 0x04000, 0x4000, CRC(b500d683) SHA1(c9f92930c815a947fbd9a3846fd9580671da324f) )

	ROM_REGION( 0x40000, "obj_tiles", 0 )
	ROM_LOAD( "bug26.275", 0x00000, 0x4000, CRC(fc6fce64) SHA1(4cea5796c26980455bdc0f8fcdb8e719caa8041f) )
	ROM_LOAD( "bug27.245", 0x04000, 0x4000, CRC(54b9a9dd) SHA1(4f4f29a459991501013cdfffd34b50eed1cde850) )
	ROM_LOAD( "bug19.274", 0x08000, 0x4000, CRC(3392c2ef) SHA1(a4aa4f24517da16495be073aa590cda81d9fca49) )
	ROM_LOAD( "bug20.244", 0x0c000, 0x4000, CRC(9ee4c236) SHA1(51155193e470a82deb5e3f1979ce112ada39b705) )

	ROM_LOAD( "bug28.243", 0x10000, 0x4000, CRC(feef6c27) SHA1(61679d67a6ef85965078e3ddd11c178a1a55f223) )
	ROM_LOAD( "bug29.212", 0x14000, 0x4000, CRC(f570e00b) SHA1(ced4b9a4a324a4c92c08e088fa116469b2878f55) )
	ROM_LOAD( "bug21.242", 0x18000, 0x4000, CRC(088fef40) SHA1(a0c866c32857690915a33b0810219fb3cbf24f24) )
	ROM_LOAD( "bug22.211", 0x1c000, 0x4000, CRC(5ec02630) SHA1(75233de03461016d1bc7c4ece0502e16e53c3351) )

	ROM_LOAD( "bug30.186", 0x20000, 0x4000, CRC(5c2ecabf) SHA1(5be25ddc1e2aac4579a39e405f8eac919f4917bd) )
	ROM_RELOAD(            0x34000, 0x4000 )
	ROM_LOAD( "bug32.158", 0x24000, 0x4000, CRC(125461f2) SHA1(9dd94344cfc7a17670d6f512ecd5947f198154c0) )
	ROM_RELOAD(            0x30000, 0x4000 )
	ROM_LOAD( "bug23.185", 0x28000, 0x4000, CRC(cafb4d4a) SHA1(aa24becdf354abca507dbd77fa18d05bea685285) )
	ROM_RELOAD(            0x3c000, 0x4000 )
	ROM_LOAD( "bug25.157", 0x2c000, 0x4000, CRC(80c4e045) SHA1(be3b537d3ed3ee74fc51059aa744dca4d63431f6) )
	ROM_RELOAD(            0x38000, 0x4000 )

	ROM_REGION( 0x40000, "road", 0 )
	ROM_LOAD( "bug12.58", 0x0000, 0x2000, CRC(bd34d55c) SHA1(05a719a6eff5af3aaaa1e0ee783b18597582ed64) )
	ROM_LOAD( "bug11.57", 0x2000, 0x2000, CRC(a44d43eb) SHA1(c4d68c7e123506acaa6adc353579cac19ecb3a9d) )
	ROM_LOAD( "bb3.137",  0x4000, 0x0200, CRC(ad76f3fb) SHA1(bf96f903b32e009a2592df0f28cc3e20b039f4d4) )
	ROM_LOAD( "bb4.138",  0x4200, 0x0200, CRC(e4ca4ea0) SHA1(0c8dd6f87bddcc709de42e0c4a59be3c19c5aa8a) )
	ROM_LOAD( "bb5.139",  0x4400, 0x0200, CRC(e2577a9a) SHA1(6408f815a1357712473c54a8603137a58431781b) )
	ROM_LOAD( "bb6.94",   0x4600, 0x0200, CRC(ad43e02a) SHA1(c50a398020508f52ddf8d45881f211d17d096fa1) )

	ROM_REGION( 0x10000, "au_data", ROMREGION_LE )
	ROM_LOAD16_BYTE( "bug9.170",  0x0000, 0x4000, CRC(7d84135b) SHA1(3c669c4e796e83672aceeb6de1aeea28f9f2fef0) )
	ROM_LOAD16_BYTE( "bug10.171", 0x0001, 0x4000, CRC(b518dd6f) SHA1(7cefa2f9438306c81dc83cd260928c835eb9b712) )
	ROM_LOAD16_BYTE( "bb1.245",   0x8000, 0x0200, CRC(0ddbd36d) SHA1(7a08901a350c315d46ab8d0aa881db384b9f37d2) )
	ROM_LOAD16_BYTE( "bb2.220",   0x8001, 0x0200, CRC(71d47de1) SHA1(2da9aeb3f2ebb1114631c8042a37c4f4c18e741b) )

	ROM_REGION( 0x100000, "obj_map", 0 )
	ROM_LOAD( "bug16.210", 0x0000, 0x4000, CRC(8b64409e) SHA1(1fb4c6923e6a9e1f2a63a2c335b63e2bdc44b61f) )
	ROM_LOAD( "bug14.209", 0x4000, 0x4000, CRC(4e765282) SHA1(f7d69d39823a8b33bd0e5b1bd78a5d68a293e221) )
	ROM_LOAD( "bug17.182", 0x8000, 0x4000, CRC(a5d84df6) SHA1(4e33ef0bee383e0d47b0c679cd2a54edb7ca0e3e) )
	ROM_LOAD( "bug15.181", 0xc000, 0x4000, CRC(d519de10) SHA1(535d05e11af65be65f3d9924b0c48faf8dcfd1bf) )

	ROM_REGION( 0x10000, "obj_luts", 0 )
	ROM_LOAD( "bug13.124", 0x0000, 0x2000, CRC(53604d7a) SHA1(bfa304cd885162ece7a5f54988d9880fc541eb3a) )
	ROM_LOAD( "bug18.156", 0x2000, 0x4000, CRC(e58321a6) SHA1(81be87d3c6046bb375c74362dc940f0269b39d1d) )

	ROM_REGION( 0x10000, "proms", 0 )
	ROM_LOAD( "bb10.191", 0x000,  0x100, CRC(f2368398) SHA1(53f28dba11bb494d033bb279abf138975c84b20d) )
	ROM_LOAD( "bb11.192", 0x100,  0x100, CRC(bf77f624) SHA1(b042d293d2094dbabb32d628fd9addd832f084ef) )
	ROM_LOAD( "bb12.193", 0x200,  0x100, CRC(10a2e8d1) SHA1(51a8c51ecbbb7bd04ae46fb5598d2c8de8097581) )
	ROM_LOAD( "bb13.194", 0x300,  0x100, CRC(40d10dfa) SHA1(e40b4c424827937fec6df1a27b19b8dc09d3274a) )

	ROM_LOAD( "bb10.104", 0x000,  0x100, CRC(f2368398) SHA1(53f28dba11bb494d033bb279abf138975c84b20d) )
	ROM_LOAD( "bb11.105", 0x100,  0x100, CRC(bf77f624) SHA1(b042d293d2094dbabb32d628fd9addd832f084ef) )
	ROM_LOAD( "bb12.106", 0x200,  0x100, CRC(10a2e8d1) SHA1(51a8c51ecbbb7bd04ae46fb5598d2c8de8097581) )
	ROM_LOAD( "bb13.107", 0x300,  0x100, CRC(40d10dfa) SHA1(e40b4c424827937fec6df1a27b19b8dc09d3274a) )

	ROM_LOAD( "bb10.49",  0x000,  0x100, CRC(f2368398) SHA1(53f28dba11bb494d033bb279abf138975c84b20d) )
	ROM_LOAD( "bb11.50",  0x100,  0x100, CRC(bf77f624) SHA1(b042d293d2094dbabb32d628fd9addd832f084ef) )
	ROM_LOAD( "bb12.51",  0x200,  0x100, CRC(10a2e8d1) SHA1(51a8c51ecbbb7bd04ae46fb5598d2c8de8097581) )
	ROM_LOAD( "bb13.52",  0x300,  0x100, CRC(40d10dfa) SHA1(e40b4c424827937fec6df1a27b19b8dc09d3274a) )

	ROM_LOAD( "bb14.199", 0x400,  0x100, CRC(0b821e0b) SHA1(b9401b9364fb99e15f562df91dcfdec1b989af2d) )
	ROM_LOAD( "bb14.197", 0x400,  0x100, CRC(0b821e0b) SHA1(b9401b9364fb99e15f562df91dcfdec1b989af2d) )
	ROM_LOAD( "bb14.137", 0x400,  0x100, CRC(0b821e0b) SHA1(b9401b9364fb99e15f562df91dcfdec1b989af2d) )

	ROM_LOAD( "bb9.271",  0x500,  0x800, CRC(6fc807d1) SHA1(3442cbb21bbedf6291a3fe1747d479445f613d26) )
	ROM_LOAD( "bb9.238",  0xd00,  0x800, CRC(6fc807d1) SHA1(3442cbb21bbedf6291a3fe1747d479445f613d26) )

	ROM_LOAD( "bb7.16",   0x1500, 0x100, CRC(b57b609f) SHA1(2dea375437c62cb4c64b21d5e6ddc09397b6ab35) )
	ROM_LOAD( "bb7.18",   0x1500, 0x100, CRC(b57b609f) SHA1(2dea375437c62cb4c64b21d5e6ddc09397b6ab35) )
	ROM_LOAD( "bb7.20",   0x1500, 0x100, CRC(b57b609f) SHA1(2dea375437c62cb4c64b21d5e6ddc09397b6ab35) )

	ROM_LOAD( "bb8.152",  0x1600, 0x100, CRC(2330ff4f) SHA1(e86eb63ce47572bcbbf325f9bb749d10d96bf2e7) )

	/* TODO: PALs */
ROM_END

/*
    The game is comprised of three boards:
    - Sound Board (labeled TC043-1, top small board)
    - CPU Board (labeled TC041, middle board, uses 15.000 MHz xtal)
    - Video Board (labeled TC042, bottom board, uses 18.000 MHz xtal)
*/
ROM_START( buggyboyjr )
	ROM_REGION( 0x100000, "main_cpu", 0 )
	ROM_LOAD16_BYTE( "bug1a.214", 0x20000, 0x8000, CRC(92797c25) SHA1(8f7434abbd7f557d3202abb01b1e4899c82c67a5) )
	ROM_LOAD16_BYTE( "bug4a.175", 0x20001, 0x8000, CRC(40ce3930) SHA1(4bf62ebeea1549a13a21a32cb860717f064b186a) )

	ROM_LOAD16_BYTE( "bug2s.213", 0xf0000, 0x8000, CRC(abcc8ad2) SHA1(aeb695c3675d40a951fe8272cbf0f6673435dab8) )
	ROM_LOAD16_BYTE( "bug5s.174", 0xf0001, 0x8000, CRC(5e352d8d) SHA1(350c206b5241d5628e673ce1108f728c8c4f980c) )

	ROM_REGION( 0x100000, "math_cpu", 0 )
	ROM_LOAD16_BYTE( "bug8s.26", 0x4000,  0x2000, CRC(efd66282) SHA1(8355422c0732c92951659930eb399129fe8d6230) )
	ROM_RELOAD(                  0x8000,  0x2000 )
	ROM_RELOAD(                  0xc000,  0x2000 )
	ROM_RELOAD(                  0xfc000, 0x2000 )

	ROM_LOAD16_BYTE( "bug7s.25", 0x4001,  0x2000, CRC(bd75b5eb) SHA1(f2b55f84f4c968df177a56103924ac64705285cd) )
	ROM_RELOAD(                  0x8001,  0x2000 )
	ROM_RELOAD(                  0xc001,  0x2000 )
	ROM_RELOAD(                  0xfc001, 0x2000 )

	ROM_REGION( 0x10000, "audio_cpu", 0 )
	ROM_LOAD( "bug35s.21", 0x00000, 0x4000, CRC(65d9af57) SHA1(17b09404942d17e7254550c43b56ae96a8c55680) )

	ROM_REGION( 0x8000, "char_tiles", 0 )
	ROM_LOAD( "bug34s.46", 0x00000, 0x4000, CRC(8ea8fec4) SHA1(75e67c9a59a86fcdedf2a70fafd303baa552aa18) )
	ROM_LOAD( "bug33s.47", 0x04000, 0x4000, CRC(459c2b03) SHA1(ff62a86195042a349fbe799c638cf590fe9572bb) )

	ROM_REGION( 0x40000, "obj_tiles", 0 )
	ROM_LOAD( "bug26s.147", 0x00000, 0x8000, CRC(14033710) SHA1(e05afeb557ce14055fa8b4f6d8805307feaa1660) )
	ROM_LOAD( "bug19s.144", 0x08000, 0x8000, CRC(838e0697) SHA1(0e9aff2c4065d79350ddb55edff57a899c33ef1c) )
	ROM_LOAD( "bug28s.146", 0x10000, 0x8000, CRC(8b47d227) SHA1(a3e57594ad0085e8b1bd327c580eb36237f3e3d2) )
	ROM_LOAD( "bug21s.143", 0x18000, 0x8000, CRC(876a5666) SHA1(db485cdf35f63c080c919ee86374f63e577092c3) )
	ROM_LOAD( "bug30s.145", 0x20000, 0x8000, CRC(11d8e2a8) SHA1(9bf198229a12d331e8e7352b7ee3f39f6891f517) )
	ROM_LOAD( "bug23s.142", 0x28000, 0x8000, CRC(015db5d8) SHA1(39ef8b44f2eb9399fb1555cffa6763e06d59c181) )

	ROM_REGION( 0x40000, "road", 0 )
	ROM_LOAD( "bug11s.225",0x0000, 0x4000, CRC(771af4e1) SHA1(a42b164dd0567c78c0d308ee48d63e5a284897bb) )
	ROM_LOAD( "bb3s.195",  0x4000, 0x0200, CRC(2ab3d5ff) SHA1(9f8359cb4ba2e7d15dbb9dc21cd71c0902cd2153) )
	ROM_LOAD( "bb4s.193",  0x4200, 0x0200, CRC(630f68a4) SHA1(d730f050353c688f81d090e33e00cd35e7b7b6fa) )
	ROM_LOAD( "bb5s.194",  0x4400, 0x0200, CRC(65925c9e) SHA1(d1ff1cb9f83c09e52a96632945e4edfedc335fd4) )
	ROM_LOAD( "bb6.224",   0x4600, 0x0200, CRC(ad43e02a) SHA1(c50a398020508f52ddf8d45881f211d17d096fa1) )

	ROM_REGION( 0x10000, "au_data", ROMREGION_LE )
	ROM_LOAD16_BYTE( "bug9.138", 0x0000, 0x4000, CRC(7d84135b) SHA1(3c669c4e796e83672aceeb6de1aeea28f9f2fef0) )
	ROM_LOAD16_BYTE( "bug10.95", 0x0001, 0x4000, CRC(b518dd6f) SHA1(7cefa2f9438306c81dc83cd260928c835eb9b712) )
	ROM_LOAD16_BYTE( "bb1.163",  0x8000, 0x0200, CRC(0ddbd36d) SHA1(7a08901a350c315d46ab8d0aa881db384b9f37d2) )
	ROM_LOAD16_BYTE( "bb2.162",  0x8001, 0x0200, CRC(71d47de1) SHA1(2da9aeb3f2ebb1114631c8042a37c4f4c18e741b) )

	ROM_REGION( 0x100000, "obj_map", 0 )
	ROM_LOAD( "bug16s.139", 0x0000, 0x8000, CRC(1903a9ad) SHA1(526c404c15e3f04b4afb27dee66e9deb0a6b9704) )
	ROM_LOAD( "bug17s.140", 0x8000, 0x8000, CRC(82cabdd4) SHA1(94324fcf83c373621fc40553473ae3cb552ab704) )

	ROM_REGION( 0x10000, "obj_luts", 0 )
	ROM_LOAD( "bug13.32",   0x0000, 0x2000, CRC(53604d7a) SHA1(bfa304cd885162ece7a5f54988d9880fc541eb3a) )
	ROM_LOAD( "bug18s.141", 0x2000, 0x4000, CRC(67786327) SHA1(32cc1f5bc654497c968ddcd4af29720c6d659482) )

	ROM_REGION( 0x10000, "proms", 0 )
	/* RGBI */
	ROM_LOAD( "bb10.41", 0x000, 0x100, CRC(f2368398) SHA1(53f28dba11bb494d033bb279abf138975c84b20d) )
	ROM_LOAD( "bb11.40", 0x100, 0x100, CRC(bf77f624) SHA1(b042d293d2094dbabb32d628fd9addd832f084ef) )
	ROM_LOAD( "bb12.39", 0x200, 0x100, CRC(10a2e8d1) SHA1(51a8c51ecbbb7bd04ae46fb5598d2c8de8097581) )
	ROM_LOAD( "bb13.42", 0x300, 0x100, CRC(40d10dfa) SHA1(e40b4c424827937fec6df1a27b19b8dc09d3274a) )

	/* Character colour LUT */
	ROM_LOAD( "bb14.19", 0x400, 0x100, CRC(0b821e0b) SHA1(b9401b9364fb99e15f562df91dcfdec1b989af2d) )

	/* Object colour LUTs (odd and even pixels) */
	ROM_LOAD( "bb9.190", 0x500, 0x800, CRC(6fc807d1) SHA1(3442cbb21bbedf6291a3fe1747d479445f613d26) )
	ROM_LOAD( "bb9.162", 0xd00, 0x800, CRC(6fc807d1) SHA1(3442cbb21bbedf6291a3fe1747d479445f613d26) )

	/* Object tile LUT */
	ROM_LOAD( "bb8.31", 0x1600, 0x100, CRC(2330ff4f) SHA1(e86eb63ce47572bcbbf325f9bb749d10d96bf2e7) )

	/* Road */
	ROM_LOAD( "bb7.188", 0x1500, 0x100, CRC(b57b609f) SHA1(2dea375437c62cb4c64b21d5e6ddc09397b6ab35) )

	/* PALs located on the sound board */
	ROM_REGION( 0x00001, "pals_soundbd", 0 )
	ROM_LOAD( "pal10l8cn.ic16", 0x00000, 0x00001, NO_DUMP )

	/* PALs located on the video board */
	ROM_REGION( 0x00002, "pals_vidbd", 0 )
	ROM_LOAD( "pal10h8cn.ic82", 0x00000, 0x00001, NO_DUMP )
	ROM_LOAD( "pal14h4cn.ic83", 0x00000, 0x00001, NO_DUMP )

	/* PALs located on the cpu board */
	ROM_REGION( 0x00009, "pals_cpubd", 0 )
	ROM_LOAD( "pal16r4a-2cn.ic83", 0x00000, 0x00001, NO_DUMP )
	ROM_LOAD( "pal12l6cn.ic87",    0x00000, 0x00001, NO_DUMP )
	ROM_LOAD( "pal10l8cn.ic88",    0x00000, 0x00001, NO_DUMP )
	ROM_LOAD( "pal14h4cn.ic149",   0x00000, 0x00001, NO_DUMP )
	ROM_LOAD( "pal16l8cj.ic150",   0x00000, 0x00001, NO_DUMP )
	ROM_LOAD( "pal14l4cn.ic151",   0x00000, 0x00001, NO_DUMP )
	ROM_LOAD( "pal14l4cn.ic167",   0x00000, 0x00001, NO_DUMP )
	ROM_LOAD( "pal14h4cn.ic229",   0x00000, 0x00001, NO_DUMP )
	ROM_LOAD( "pal14h4cn.ic230",   0x00000, 0x00001, NO_DUMP )
ROM_END


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAMEL( 1983, tx1,        0,        tx1,      tx1,      0, ROT0, "Tatsumi", "TX-1",                                   GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND, layout_tx1 )
GAMEL( 1983, tx1a,       tx1,      tx1,      tx1a,     0, ROT0, "Tatsumi (Atari/Namco/Taito license)", "TX-1 (Atari/Namco/Taito license)", GAME_IMPERFECT_SOUND, layout_tx1 )
GAMEL( 1985, buggyboy,   0,        buggyboy, buggyboy, 0, ROT0, "Tatsumi", "Buggy Boy/Speed Buggy (cockpit)",        0, layout_buggyboy )
GAMEL( 1986, buggyboyjr, buggyboy, buggybjr, buggybjr, 0, ROT0, "Tatsumi", "Buggy Boy Junior/Speed Buggy (upright)", 0, layout_buggybjr )
