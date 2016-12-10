/*************************************************************************

    Cinematronics Dragon's Lair system

    Games supported:
        * Dragon's Lair
        * Dragon's Lair (European version)
        * Space Ace
        * Space Ace (European version)

**************************************************************************

    There are two revisions of the Cinematronics board used in the
    U.S. Rev A


    ROM Revisions
    -------------
    Revision A, B, and C EPROMs use the Pioneer PR-7820 only.
    Revision D EPROMs used the Pioneer LD-V1000 only.
    Revisions E, F, and F2 EPROMs used either player.

    Revisions A, B, C, and D are a five chip set.
    Revisions E, F, and F2 are a four chip set.


    Laserdisc Players Used
    ----------------------
    Pioneer PR-7820 (USA / Cinematronics)
    Pioneer LD-V1000 (USA / Cinematronics)
    Philips 22VP932 (Europe / Atari) and (Italian / Sidam)

*************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "render.h"
#include "cpu/z80/z80daisy.h"
#include "machine/laserdsc.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"
#include "sound/ay8910.h"
#include "sound/beep.h"
#include "dlair.lh"



/*************************************
 *
 *  Constants
 *
 *************************************/

#define MASTER_CLOCK_US				16000000
#define MASTER_CLOCK_EURO			14318180

#define LASERDISC_TYPE_FIXED		0
#define LASERDISC_TYPE_VARIABLE		1



/*************************************
 *
 *  Globals
 *
 *************************************/

static laserdisc_device *laserdisc;
static UINT8 last_misc;

static UINT8 laserdisc_type;
static UINT8 laserdisc_data;

static const UINT8 led_map[16] =
	{ 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7c,0x07,0x7f,0x67,0x77,0x7c,0x39,0x5e,0x79,0x00 };



/*************************************
 *
 *  Z80 peripheral interfaces
 *
 *************************************/

static void dleuro_interrupt(running_device *device, int state)
{
	cputag_set_input_line(device->machine, "maincpu", 0, state);
}


static WRITE8_DEVICE_HANDLER( serial_transmit )
{
	laserdisc_data_w(laserdisc, data);
}


static int serial_receive(running_device *device, int channel)
{
	/* if we still have data to send, do it now */
	if (channel == 0 && laserdisc_line_r(laserdisc, LASERDISC_LINE_DATA_AVAIL) == ASSERT_LINE)
		return laserdisc_data_r(laserdisc);

	return -1;
}


static Z80CTC_INTERFACE( ctc_intf )
{
	0,              	/* timer disables */
	DEVCB_CPU_INPUT_LINE("maincpu", INPUT_LINE_IRQ0),	/* interrupt handler */
	DEVCB_NULL,			/* ZC/TO0 callback */
	DEVCB_NULL,         /* ZC/TO1 callback */
	DEVCB_NULL          /* ZC/TO2 callback */
};


static const z80sio_interface sio_intf =
{
	dleuro_interrupt,	/* interrupt handler */
	0,					/* DTR changed handler */
	0,					/* RTS changed handler */
	0,					/* BREAK changed handler */
	serial_transmit,	/* transmit handler */
	serial_receive		/* receive handler */
};


static const z80_daisy_config dleuro_daisy_chain[] =
{
	{ "sio" },
	{ "ctc" },
	{ NULL }
};



/*************************************
 *
 *  Video startup/shutdown
 *
 *************************************/

static PALETTE_INIT( dleuro )
{
	int i;

	for (i = 0; i < 8; i++)
	{
		palette_set_color(machine, 2 * i + 0, MAKE_RGB(0, 0, 0));
		palette_set_color_rgb(machine, 2 * i + 1, pal1bit(i >> 0), pal1bit(i >> 1), pal1bit(i >> 2));
	}
}



/*************************************
 *
 *  Video update
 *
 *************************************/

static VIDEO_UPDATE( dleuro )
{
	int x, y;

	/* redraw the overlay */
	for (y = 0; y < 32; y++)
		for (x = 0; x < 32; x++)
		{
			UINT8 *base = &screen->machine->generic.videoram.u8[y * 64 + x * 2 + 1];
			drawgfx_opaque(bitmap, cliprect, screen->machine->gfx[0], base[0], base[1], 0, 0, 10 * x, 16 * y);
		}

	return 0;
}



/*************************************
 *
 *  Machine startup/reset
 *
 *************************************/

static MACHINE_START( dlair )
{
	laserdisc = machine->device<laserdisc_device>("laserdisc");
}


static MACHINE_RESET( dlair )
{
	/* determine the laserdisc player from the DIP switches */
	if (laserdisc_type == LASERDISC_TYPE_VARIABLE)
	{
		int newtype = (input_port_read(machine, "DSW2") & 0x08) ? LASERDISC_TYPE_PIONEER_LDV1000 : LASERDISC_TYPE_PIONEER_PR7820;
		laserdisc_set_type(laserdisc, newtype);
	}
}



/*************************************
 *
 *  VBLANK callback
 *
 *************************************/

static INTERRUPT_GEN( vblank_callback )
{
	/* also update the speaker on the European version */
	beep_sound_device *beep = device->machine->device<beep_sound_device>("beep");
	if (beep != NULL)
	{
		z80ctc_device *ctc = device->machine->device<z80ctc_device>("ctc");
		beep_set_state(beep, 1);
		beep_set_frequency(beep, ATTOSECONDS_TO_HZ(ctc->period(0).attoseconds));
	}
}



/*************************************
 *
 *  Outputs
 *
 *************************************/

static WRITE8_HANDLER( misc_w )
{
	/*
        D0-D3 = B0-B3
           D4 = coin counter
           D5 = OUT DISC DATA
           D6 = ENTER
           D7 = INT/EXT
    */
	UINT8 diff = data ^ last_misc;
	last_misc = data;

	coin_counter_w(space->machine, 0, (~data >> 4) & 1);

	/* on bit 5 going low, push the data out to the laserdisc player */
	if ((diff & 0x20) && !(data & 0x20))
		laserdisc_data_w(laserdisc, laserdisc_data);

	/* on bit 6 going low, we need to signal enter */
	laserdisc_line_w(laserdisc, LASERDISC_LINE_ENTER, (data & 0x40) ? CLEAR_LINE : ASSERT_LINE);
}


static WRITE8_HANDLER( dleuro_misc_w )
{
	/*
           D0 = CHAR GEN ON+
           D1 = KILL VIDEO+
           D2 = SEL CHAR GEN VIDEO+
           D3 = counter 2
           D4 = coin counter
           D5 = OUT DISC DATA
           D6 = ENTER
           D7 = INT/EXT
    */
	UINT8 diff = data ^ last_misc;
	last_misc = data;

	coin_counter_w(space->machine, 1, (~data >> 3) & 1);
	coin_counter_w(space->machine, 0, (~data >> 4) & 1);

	/* on bit 5 going low, push the data out to the laserdisc player */
	if ((diff & 0x20) && !(data & 0x20))
		laserdisc_data_w(laserdisc, laserdisc_data);

	/* on bit 6 going low, we need to signal enter */
	laserdisc_line_w(laserdisc, LASERDISC_LINE_ENTER, (data & 0x40) ? CLEAR_LINE : ASSERT_LINE);
}


static WRITE8_HANDLER( led_den1_w )
{
	output_set_digit_value(0 + (offset & 7), led_map[data & 0x0f]);
}


static WRITE8_HANDLER( led_den2_w )
{
	output_set_digit_value(8 + (offset & 7), led_map[data & 0x0f]);
}



/*************************************
 *
 *  Laserdisc communication
 *
 *************************************/

static CUSTOM_INPUT( laserdisc_status_r )
{
	switch (laserdisc_get_type(laserdisc))
	{
		case LASERDISC_TYPE_PIONEER_PR7820:
			return 0;

		case LASERDISC_TYPE_PIONEER_LDV1000:
			return (laserdisc_line_r(laserdisc, LASERDISC_LINE_STATUS) == ASSERT_LINE) ? 0 : 1;

		case LASERDISC_TYPE_PHILLIPS_22VP932:
			return 0;
	}
	return 0;
}


static CUSTOM_INPUT( laserdisc_command_r )
{
	switch (laserdisc_get_type(laserdisc))
	{
		case LASERDISC_TYPE_PIONEER_PR7820:
			return (laserdisc_line_r(laserdisc, LASERDISC_LINE_READY) == ASSERT_LINE) ? 0 : 1;

		case LASERDISC_TYPE_PIONEER_LDV1000:
			return (laserdisc_line_r(laserdisc, LASERDISC_LINE_COMMAND) == ASSERT_LINE) ? 0 : 1;

		case LASERDISC_TYPE_PHILLIPS_22VP932:
			return 0;
	}
	return 0;
}


static READ8_HANDLER( laserdisc_r )
{
	UINT8 result = laserdisc_data_r(laserdisc);
	mame_printf_debug("laserdisc_r = %02X\n", result);
	return result;
}


static WRITE8_HANDLER( laserdisc_w )
{
	laserdisc_data = data;
}



/*************************************
 *
 *  U.S. version memory map
 *
 *************************************/

/* complete memory map derived from schematics */
static ADDRESS_MAP_START( dlus_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x9fff) AM_ROM
	AM_RANGE(0xa000, 0xa7ff) AM_MIRROR(0x1800) AM_RAM
	AM_RANGE(0xc000, 0xc000) AM_MIRROR(0x1fc7) AM_DEVREAD("aysnd", ay8910_r)
	AM_RANGE(0xc008, 0xc008) AM_MIRROR(0x1fc7) AM_READ_PORT("CONTROLS")
	AM_RANGE(0xc010, 0xc010) AM_MIRROR(0x1fc7) AM_READ_PORT("SERVICE")
	AM_RANGE(0xc020, 0xc020) AM_MIRROR(0x1fc7) AM_READ(laserdisc_r)
	AM_RANGE(0xe000, 0xe000) AM_MIRROR(0x1fc7) AM_DEVWRITE("aysnd", ay8910_data_w)
	AM_RANGE(0xe008, 0xe008) AM_MIRROR(0x1fc7) AM_WRITE(misc_w)
	AM_RANGE(0xe010, 0xe010) AM_MIRROR(0x1fc7) AM_DEVWRITE("aysnd", ay8910_address_w)
	AM_RANGE(0xe020, 0xe020) AM_MIRROR(0x1fc7) AM_WRITE(laserdisc_w)
	AM_RANGE(0xe030, 0xe037) AM_MIRROR(0x1fc0) AM_WRITE(led_den2_w)
	AM_RANGE(0xe038, 0xe03f) AM_MIRROR(0x1fc0) AM_WRITE(led_den1_w)
ADDRESS_MAP_END



/*************************************
 *
 *  European version memory map
 *
 *************************************/

/* complete memory map derived from schematics */
static ADDRESS_MAP_START( dleuro_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x9fff) AM_ROM
	AM_RANGE(0xa000, 0xa7ff) AM_MIRROR(0x1800) AM_RAM
	AM_RANGE(0xc000, 0xc7ff) AM_MIRROR(0x1800) AM_RAM AM_BASE_GENERIC(videoram)
	AM_RANGE(0xe000, 0xe000) AM_MIRROR(0x1f47) // WT LED 1
	AM_RANGE(0xe008, 0xe008) AM_MIRROR(0x1f47) // WT LED 2
	AM_RANGE(0xe010, 0xe010) AM_MIRROR(0x1f47) AM_WRITE(led_den1_w)			// WT EXT LED 1
	AM_RANGE(0xe018, 0xe018) AM_MIRROR(0x1f47) AM_WRITE(led_den2_w)			// WT EXT LED 2
	AM_RANGE(0xe020, 0xe020) AM_MIRROR(0x1f47) AM_WRITE(laserdisc_w)		// DISC WT
	AM_RANGE(0xe028, 0xe028) AM_MIRROR(0x1f47) AM_WRITE(dleuro_misc_w)		// WT MISC
	AM_RANGE(0xe030, 0xe030) AM_MIRROR(0x1f47) AM_WRITE(watchdog_reset_w)	// CLR WDOG
	AM_RANGE(0xe080, 0xe080) AM_MIRROR(0x1f47) AM_READ_PORT("P1")			// CP A
	AM_RANGE(0xe088, 0xe088) AM_MIRROR(0x1f47) AM_READ_PORT("SYSTEM")		// CP B
	AM_RANGE(0xe090, 0xe090) AM_MIRROR(0x1f47) AM_READ_PORT("DSW1")			// OPT SW A
	AM_RANGE(0xe098, 0xe098) AM_MIRROR(0x1f47) AM_READ_PORT("DSW2")			// OPT SW B
	AM_RANGE(0xe0a0, 0xe0a0) AM_MIRROR(0x1f47) AM_READ(laserdisc_r)			// RD DISC DATA
ADDRESS_MAP_END


/* complete memory map derived from schematics */
static ADDRESS_MAP_START( dleuro_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_MIRROR(0x7c) AM_DEVREADWRITE("ctc", z80ctc_r, z80ctc_w)
	AM_RANGE(0x80, 0x83) AM_MIRROR(0x7c) AM_DEVREADWRITE("sio", z80sio_ba_cd_r, z80sio_ba_cd_w)
ADDRESS_MAP_END


/*
WT LED 1 -> LS273 -> DS1 = 7segment
WT LED 2 -> LS273 -> DS2 = 7segment

Character generator:
  decade counter LS90
  cleared on HSYNC
  clocked each pixel
  on 0 -> VOUTLD- -> load RGB color info and character bits (D0=red, D1=green, D2=blue)
  on 3 -> RADRA- -> advance RA0-RA5
  on 6 -> LCHAR- -> load next character into address lines on ROM
  on 7 -> RADRB- -> advance RA0-RA5
  on 9 -> 0

Serial in on the shift register is 0, so extra bits are 0

Video clock:
7.16MHz 0 1 0 1 0 1 0 1 0 1
ACLK+   0 0 1 1 0 0 1 1 0 0 = 3.58Mhz
ACLK-   1 1 0 0 1 1 0 0 1 1 = 3.58Mhz
BCLK+   0 0 0 1 1 0 0 1 1 0 = 3.58Mhz
BCLK-   1 1 1 0 0 1 1 0 0 1 = 3.58Mhz

Row clock:
Cleared on VSYNC
Clocked on HSYNC
ROWCLK- = Qa (i.e., row / 2)

Line clock:
Cleared on VSYNC
Clocked on ROWCLK-
LINECLK- = Qc (i.e., ROWCLK / 8 = row / 16)

Video RAM = 11-bit = 2048 bytes
If CHAR GEN ON+, RAM address = RA0-RA10
If not CHAR GEN ON+, RAM address = BA0-BA10

RA0-RA5 = counters (64 across)
  Cleared to 0 on HSYNC
  Clocked on !(RADRA- & RADRB-)

RA6-RA10 = counters (32 up & down)
  Cleared to 0 on VSYNC
  Clocked on LINECLK- (every 16 rows)
  RA6 = Qb (i.e., counts by 2, not 1)

MA0-MA3 = counters
  Cleared to 0 on VSYNC
  Clocked by ROWCLK- (every 2 rows)

Address in ROM:
  MA0-MA3     = A0-A3
  D0-D7       = A4-A11 (data from VRAM)
  KILL VIDEO+ = A12
*/



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( dlair )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("A:2,1")
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
//  PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Unused ) )
	PORT_DIPNAME( 0x04, 0x00, "Difficulty Mode" ) PORT_DIPLOCATION("A:3")
	PORT_DIPSETTING(    0x04, "Mode 1" )
	PORT_DIPSETTING(    0x00, "Mode 2" )
	PORT_DIPNAME( 0x08, 0x00, "Engineering mode" ) PORT_DIPLOCATION("A:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "2 Credits/Free play" ) PORT_DIPLOCATION("A:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Lives ) ) PORT_DIPLOCATION("A:6")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPNAME( 0x40, 0x00, "Pay as you go" ) PORT_DIPLOCATION("A:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_HIGH, "A:8" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Sound every 8 attracts" ) PORT_DIPLOCATION("B:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("B:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Unlimited Dirks" ) PORT_DIPLOCATION("B:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Joystick Feedback Sound" ) PORT_DIPLOCATION("B:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x60, "Pay as you go options" ) PORT_DIPLOCATION("B:7,6")
	PORT_DIPSETTING(    0x00, "PAYG1" )
	PORT_DIPSETTING(    0x20, "PAYG2" )
	PORT_DIPSETTING(    0x40, "PAYG3" )
	PORT_DIPSETTING(    0x60, "PAYG4" )
	PORT_DIPNAME( 0x90, 0x10, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("B:8,5")
	PORT_DIPSETTING(	0x00, "Increase after 5" ) PORT_CONDITION("DSW1", 0x04, PORTCOND_EQUALS, 0x00)
	PORT_DIPSETTING(	0x10, "Increase after 9" ) PORT_CONDITION("DSW1", 0x04, PORTCOND_EQUALS, 0x00)
	PORT_DIPSETTING(	0x80, DEF_STR( Easy ) ) PORT_CONDITION("DSW1", 0x04, PORTCOND_EQUALS, 0x00)
	PORT_DIPSETTING(	0x90, DEF_STR( Easy ) ) PORT_CONDITION("DSW1", 0x04, PORTCOND_EQUALS, 0x00)
	PORT_DIPSETTING(	0x00, DEF_STR( Hard ) ) PORT_CONDITION("DSW1", 0x04, PORTCOND_EQUALS, 0x04)
	PORT_DIPSETTING(	0x10, DEF_STR( Easy ) ) PORT_CONDITION("DSW1", 0x04, PORTCOND_EQUALS, 0x04)
	PORT_DIPSETTING(	0x80, DEF_STR( Easy ) ) PORT_CONDITION("DSW1", 0x04, PORTCOND_EQUALS, 0x04)
	PORT_DIPSETTING(	0x90, DEF_STR( Easy ) ) PORT_CONDITION("DSW1", 0x04, PORTCOND_EQUALS, 0x04)

	PORT_START("CONTROLS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */

	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(laserdisc_status_r, NULL) 	/* status strobe */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(laserdisc_command_r, NULL)	/* command strobe */
INPUT_PORTS_END


static INPUT_PORTS_START( dlaire )
	PORT_INCLUDE(dlair)

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x08, 0x08, "LD Player" )		/* In Rev F, F2 and so on... before it was Joystick Sound Feedback */
	PORT_DIPSETTING(    0x00, "LD-PR7820" )
	PORT_DIPSETTING(    0x08, "LDV-1000" )
INPUT_PORTS_END


static INPUT_PORTS_START( dleuro )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(laserdisc_status_r, NULL) 	/* status strobe */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(laserdisc_command_r, NULL)	/* command strobe */

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("A:2,1")
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
//  PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Unused ) )
	PORT_DIPNAME( 0x04, 0x00, "Difficulty Mode" ) PORT_DIPLOCATION("A:3")
	PORT_DIPSETTING(    0x04, "Mode 1" )
	PORT_DIPSETTING(    0x00, "Mode 2" )
	PORT_DIPNAME( 0x08, 0x00, "Engineering mode" ) PORT_DIPLOCATION("A:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "2 Credits/Free play" ) PORT_DIPLOCATION("A:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Lives ) ) PORT_DIPLOCATION("A:6")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPNAME( 0x40, 0x00, "Pay as you go" ) PORT_DIPLOCATION("A:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_HIGH, "A:8" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Sound every 8 attracts" ) PORT_DIPLOCATION("B:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("B:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Unlimited Dirks" ) PORT_DIPLOCATION("B:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Joystick Feedback Sound" ) PORT_DIPLOCATION("B:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x60, "Pay as you go options" ) PORT_DIPLOCATION("B:7,6")
	PORT_DIPSETTING(    0x00, "PAYG1" )
	PORT_DIPSETTING(    0x20, "PAYG2" )
	PORT_DIPSETTING(    0x40, "PAYG3" )
	PORT_DIPSETTING(    0x60, "PAYG4" )
	PORT_DIPNAME( 0x90, 0x10, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("B:8,5s")
	PORT_DIPSETTING(	0x00, "Increase after 5" ) PORT_CONDITION("DSW1", 0x04, PORTCOND_EQUALS, 0x00)
	PORT_DIPSETTING(	0x10, "Increase after 9" ) PORT_CONDITION("DSW1", 0x04, PORTCOND_EQUALS, 0x00)
	PORT_DIPSETTING(	0x80, DEF_STR( Easy ) ) PORT_CONDITION("DSW1", 0x04, PORTCOND_EQUALS, 0x00)
	PORT_DIPSETTING(	0x90, DEF_STR( Easy ) ) PORT_CONDITION("DSW1", 0x04, PORTCOND_EQUALS, 0x00)
	PORT_DIPSETTING(	0x00, DEF_STR( Hard ) ) PORT_CONDITION("DSW1", 0x04, PORTCOND_EQUALS, 0x04)
	PORT_DIPSETTING(	0x10, DEF_STR( Easy ) ) PORT_CONDITION("DSW1", 0x04, PORTCOND_EQUALS, 0x04)
	PORT_DIPSETTING(	0x80, DEF_STR( Easy ) ) PORT_CONDITION("DSW1", 0x04, PORTCOND_EQUALS, 0x04)
	PORT_DIPSETTING(	0x90, DEF_STR( Easy ) ) PORT_CONDITION("DSW1", 0x04, PORTCOND_EQUALS, 0x04)
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP8(7,-1) },
	{ STEP16(0,8) },
	16*8
};


static GFXDECODE_START( dlair )
	GFXDECODE_ENTRY( "gfx1", 0x0000, charlayout, 0, 8 )
GFXDECODE_END



/*************************************
 *
 *  Sound chip definitions
 *
 *************************************/

static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_INPUT_PORT("DSW1"),
	DEVCB_INPUT_PORT("DSW2")
};



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static MACHINE_DRIVER_START( dlair_base )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, MASTER_CLOCK_US/4)
	MDRV_CPU_PROGRAM_MAP(dlus_map)
	MDRV_CPU_VBLANK_INT("screen", vblank_callback)
	MDRV_CPU_PERIODIC_INT(irq0_line_hold, (double)MASTER_CLOCK_US/8/16/16/16/16)

	MDRV_MACHINE_START(dlair)
	MDRV_MACHINE_RESET(dlair)

	/* video hardware */
	MDRV_LASERDISC_SCREEN_ADD_NTSC("screen", BITMAP_FORMAT_RGB32)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("aysnd", AY8910, MASTER_CLOCK_US/8)
	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.33)

	MDRV_SOUND_ADD("ldsound", LASERDISC, 0)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( dlair_pr7820 )
	MDRV_IMPORT_FROM(dlair_base)
	MDRV_LASERDISC_ADD("laserdisc", PIONEER_PR7820, "screen", "ldsound")
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( dlair_ldv1000 )
	MDRV_IMPORT_FROM(dlair_base)
	MDRV_LASERDISC_ADD("laserdisc", PIONEER_LDV1000, "screen", "ldsound")
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( dleuro )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, MASTER_CLOCK_EURO/4)
	MDRV_CPU_CONFIG(dleuro_daisy_chain)
	MDRV_CPU_PROGRAM_MAP(dleuro_map)
	MDRV_CPU_IO_MAP(dleuro_io_map)
	MDRV_CPU_VBLANK_INT("screen", vblank_callback)

	MDRV_Z80CTC_ADD("ctc", MASTER_CLOCK_EURO/4 /* same as "maincpu" */, ctc_intf)
	MDRV_Z80SIO_ADD("sio", MASTER_CLOCK_EURO/4 /* same as "maincpu" */, sio_intf)

	MDRV_WATCHDOG_TIME_INIT(HZ(MASTER_CLOCK_EURO/(16*16*16*16*16*8)))

	MDRV_MACHINE_START(dlair)
	MDRV_MACHINE_RESET(dlair)

	MDRV_LASERDISC_ADD("laserdisc", PHILLIPS_22VP932, "screen", "ldsound")
	MDRV_LASERDISC_OVERLAY(dleuro, 256, 256, BITMAP_FORMAT_INDEXED16)

	/* video hardware */
	MDRV_LASERDISC_SCREEN_ADD_PAL("screen", BITMAP_FORMAT_INDEXED16)

	MDRV_GFXDECODE(dlair)
	MDRV_PALETTE_LENGTH(16)

	MDRV_PALETTE_INIT(dleuro)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("beep", BEEP, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.33)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.33)

	MDRV_SOUND_ADD("ldsound", LASERDISC, 0)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( dlair )		/* revision F2 */
	ROM_REGION( 0xa000, "maincpu", 0 )
	ROM_LOAD( "dl_f2_u1.bin", 0x0000, 0x2000,  CRC(f5ea3b9d) SHA1(c0cafff8b2982125fd3314ffc66681e47f027fc9) )
	ROM_LOAD( "dl_f2_u2.bin", 0x2000, 0x2000,  CRC(dcc1dff2) SHA1(614ca8f6c5b6fa1d590f6b80d731377faa3a65a9) )
	ROM_LOAD( "dl_f2_u3.bin", 0x4000, 0x2000,  CRC(ab514e5b) SHA1(29d1015b951f0f2d4e5257497f3bf007c5e2262c) )
	ROM_LOAD( "dl_f2_u4.bin", 0x6000, 0x2000,  CRC(f5ec23d2) SHA1(71149e2d359cc5944fbbb53dd7d0c2b42fbc9bb4) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "dlair", 0, NO_DUMP )
ROM_END

ROM_START( dlaira )		/* revision A */
	ROM_REGION( 0xa000, "maincpu", 0 )
	ROM_LOAD( "dl_a_u1.bin", 0x0000, 0x2000,  CRC(d76e83ec) SHA1(fc7ff5d883de9b38a9e0532c35990f4b319ba1d3) )
	ROM_LOAD( "dl_a_u2.bin", 0x2000, 0x2000,  CRC(a6a723d8) SHA1(5c71cb0b6be7331083adaf6fac6bdfc8445cb485) )
	ROM_LOAD( "dl_a_u3.bin", 0x4000, 0x2000,  CRC(52c59014) SHA1(d4015046bf1c1f51c29d9d9f8e8d008519b61cd1) )
	ROM_LOAD( "dl_a_u4.bin", 0x6000, 0x2000,  CRC(924d12f2) SHA1(05b487e651a4817991dfc2308834b8f2fae918b4) )
	ROM_LOAD( "dl_a_u5.bin", 0x8000, 0x2000,  CRC(6ec2f9c1) SHA1(0b8026927697a99fe8fa0dd4bd643418779a1d45) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "dlair", 0, NO_DUMP )
ROM_END

ROM_START( dlairb )		/* revision B */
	ROM_REGION( 0xa000, "maincpu", 0 )
	ROM_LOAD( "dl_b_u1.bin", 0x0000, 0x2000,  CRC(d76e83ec) SHA1(fc7ff5d883de9b38a9e0532c35990f4b319ba1d3) )
	ROM_LOAD( "dl_b_u2.bin", 0x2000, 0x2000,  CRC(6751103d) SHA1(e94e19f738e0eb69700e56c6069c7f3c0911303f) )
	ROM_LOAD( "dl_b_u3.bin", 0x4000, 0x2000,  CRC(52c59014) SHA1(d4015046bf1c1f51c29d9d9f8e8d008519b61cd1) )
	ROM_LOAD( "dl_b_u4.bin", 0x6000, 0x2000,  CRC(924d12f2) SHA1(05b487e651a4817991dfc2308834b8f2fae918b4) )
	ROM_LOAD( "dl_b_u5.bin", 0x8000, 0x2000,  CRC(6ec2f9c1) SHA1(0b8026927697a99fe8fa0dd4bd643418779a1d45) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "dlair", 0, NO_DUMP )
ROM_END

ROM_START( dlairc )		/* revision C */
	ROM_REGION( 0xa000, "maincpu", 0 )
	ROM_LOAD( "dl_c_u1.bin", 0x0000, 0x2000,  CRC(cebfe26a) SHA1(1c808de5c92fef67d8088621fbd743c1a0a3bb5e) )
	ROM_LOAD( "dl_c_u2.bin", 0x2000, 0x2000,  CRC(6751103d) SHA1(e94e19f738e0eb69700e56c6069c7f3c0911303f) )
	ROM_LOAD( "dl_c_u3.bin", 0x4000, 0x2000,  CRC(52c59014) SHA1(d4015046bf1c1f51c29d9d9f8e8d008519b61cd1) )
	ROM_LOAD( "dl_c_u4.bin", 0x6000, 0x2000,  CRC(924d12f2) SHA1(05b487e651a4817991dfc2308834b8f2fae918b4) )
	ROM_LOAD( "dl_c_u5.bin", 0x8000, 0x2000,  CRC(6ec2f9c1) SHA1(0b8026927697a99fe8fa0dd4bd643418779a1d45) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "dlair", 0, NO_DUMP )
ROM_END

ROM_START( dlaird )		/* revision D */
	ROM_REGION( 0xa000, "maincpu", 0 )
	ROM_LOAD( "dl_d_u1.bin", 0x0000, 0x2000,  CRC(0b5ab120) SHA1(6ec59d6aaa27994d8de4f5635935fd6c1d42d2f6) )
	ROM_LOAD( "dl_d_u2.bin", 0x2000, 0x2000,  CRC(93ebfffb) SHA1(2a8f6d7ab18845e22a2ba238b44d7c636908a125) )
	ROM_LOAD( "dl_d_u3.bin", 0x4000, 0x2000,  CRC(22e6591f) SHA1(3176c07af6d942496c9ae338e3b93e28e2ce7982) )
	ROM_LOAD( "dl_d_u4.bin", 0x6000, 0x2000,  CRC(5f7212cb) SHA1(69c34de1bb44b6cd2adc2947d00d8823d3e87130) )
	ROM_LOAD( "dl_d_u5.bin", 0x8000, 0x2000,  CRC(2b469c89) SHA1(646394b51325ca9163221a43b5af64a8067eb80b) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "dlair", 0, NO_DUMP )
ROM_END

ROM_START( dlaire )		/* revision E */
	ROM_REGION( 0xa000, "maincpu", 0 )
	ROM_LOAD( "dl_e_u1.bin", 0x0000, 0x2000,  CRC(02980426) SHA1(409de05045adbd054bc1fda24d4a9672832e2fae) )
	ROM_LOAD( "dl_e_u2.bin", 0x2000, 0x2000,  CRC(979d4c97) SHA1(5da6ceab5029ac5f5846bf52841675c5c70b17af) )
	ROM_LOAD( "dl_e_u3.bin", 0x4000, 0x2000,  CRC(897bf075) SHA1(d2ff9c2fec37544cfe8fb60273524c6610488502) )
	ROM_LOAD( "dl_e_u4.bin", 0x6000, 0x2000,  CRC(4ebffba5) SHA1(d04711247ffa88e371ec461465dd75a8158d90bc) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "dlair", 0, NO_DUMP )
ROM_END

ROM_START( dlairf )		/* revision F */
	ROM_REGION( 0xa000, "maincpu", 0 )
	ROM_LOAD( "dl_f_u1.bin", 0x0000, 0x2000,  CRC(06fc6941) SHA1(ea8cf6d370f89d60721ab00ec58ff24027b5252f) )
	ROM_LOAD( "dl_f_u2.bin", 0x2000, 0x2000,  CRC(dcc1dff2) SHA1(614ca8f6c5b6fa1d590f6b80d731377faa3a65a9) )
	ROM_LOAD( "dl_f_u3.bin", 0x4000, 0x2000,  CRC(ab514e5b) SHA1(29d1015b951f0f2d4e5257497f3bf007c5e2262c) )
	ROM_LOAD( "dl_f_u4.bin", 0x6000, 0x2000,  CRC(a817324e) SHA1(1299c83342fc70932f67bda8ae60bace91d66429) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "dlair", 0, NO_DUMP )
ROM_END

ROM_START( dleuro )		/* European Atari version */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "elu45.bin", 0x0000, 0x2000, CRC(4d3a9eac) SHA1(e6cd274b4a0f92b1fb1f013f80f6fd2db3212431) )
	ROM_LOAD( "elu46.bin", 0x2000, 0x2000, CRC(8479612b) SHA1(b5543a06928274bde0e1bdda0747d936feaff177) )
	ROM_LOAD( "elu47.bin", 0x4000, 0x2000, CRC(6a66f6b4) SHA1(2bee981870e61977565439c34568952043656cfa) )
	ROM_LOAD( "elu48.bin", 0x6000, 0x2000, CRC(36575106) SHA1(178e26e7d5c7f879bc55c2fb170f3bb47a709610) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "elu33.bin", 0x0000, 0x2000, CRC(e7506d96) SHA1(610ae25bd8db13b18b9e681e855ffa978043255b) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "dleuro", 0, NO_DUMP )
ROM_END

ROM_START( dlital )		/* Italian Sidam version */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dlita45.bin", 0x0000, 0x2000, CRC(2ed85958) SHA1(9651989775d215b279716b8c3e30d8e799d91b37) ) /* Label: ELU 45 SID */
	ROM_LOAD( "dlita46.bin", 0x2000, 0x2000, CRC(8479612b) SHA1(b5543a06928274bde0e1bdda0747d936feaff177) ) /* Label: ELU 46 REV.B */
	ROM_LOAD( "dlita47.bin", 0x4000, 0x2000, CRC(6a66f6b4) SHA1(2bee981870e61977565439c34568952043656cfa) ) /* Label: ELU 47 REV.B */
	ROM_LOAD( "dlita48.bin", 0x6000, 0x2000, CRC(0c0b3011) SHA1(c3ac7bf870dd4ef12609c9cf25a7da68204b2889) ) /* Label: ELU 48 SID */

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "dlita33.bin", 0x0000, 0x2000, CRC(e7506d96) SHA1(610ae25bd8db13b18b9e681e855ffa978043255b) ) /* Label: ELU 33 REV.B */

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "dleuro", 0, NO_DUMP )
ROM_END


ROM_START( spaceace )		/* revision A3 */
	ROM_REGION( 0xa000, "maincpu", 0 )
	ROM_LOAD( "sa_a3_u1.bin", 0x0000, 0x2000,  CRC(427522d0) SHA1(de4d5353af0be3e60afe1ed13d1d531c425cdb4d) )
	ROM_LOAD( "sa_a3_u2.bin", 0x2000, 0x2000,  CRC(18d0262d) SHA1(c3920e3cabfe2b2add51881e262f090c5018e508) )
	ROM_LOAD( "sa_a3_u3.bin", 0x4000, 0x2000,  CRC(4646832d) SHA1(9f1370b13cca9857b0ed13f58641ef4ba3c7326d) )
	ROM_LOAD( "sa_a3_u4.bin", 0x6000, 0x2000,  CRC(57db2a79) SHA1(5286905d9bde697845a98bd77f31f2a96a8874fc) )
	ROM_LOAD( "sa_a3_u5.bin", 0x8000, 0x2000,  CRC(85cbcdc4) SHA1(97e01e96c885ab7af4c3a3b586eb40374d54f12f) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "spaceace", 0, NO_DUMP )
ROM_END

ROM_START( spaceaa2 )		/* revision A2 */
	ROM_REGION( 0xa000, "maincpu", 0 )
	ROM_LOAD( "sa_a2_u1.bin", 0x0000, 0x2000,  CRC(71b39e27) SHA1(15a34eee9d541b186761a78b5c97449c7b496e4f) )
	ROM_LOAD( "sa_a2_u2.bin", 0x2000, 0x2000,  CRC(18d0262d) SHA1(c3920e3cabfe2b2add51881e262f090c5018e508) )
	ROM_LOAD( "sa_a2_u3.bin", 0x4000, 0x2000,  CRC(4646832d) SHA1(9f1370b13cca9857b0ed13f58641ef4ba3c7326d) )
	ROM_LOAD( "sa_a2_u4.bin", 0x6000, 0x2000,  CRC(57db2a79) SHA1(5286905d9bde697845a98bd77f31f2a96a8874fc) )
	ROM_LOAD( "sa_a2_u5.bin", 0x8000, 0x2000,  CRC(85cbcdc4) SHA1(97e01e96c885ab7af4c3a3b586eb40374d54f12f) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "spaceace", 0, NO_DUMP )
ROM_END

ROM_START( spaceaa )		/* revision A */
	ROM_REGION( 0xa000, "maincpu", 0 )
	ROM_LOAD( "sa_a_u1.bin", 0x0000, 0x2000,  CRC(8eb1889e) SHA1(bfa2c5fc139c448b7b6b5c5757d4f2f74e610b85) )
	ROM_LOAD( "sa_a_u2.bin", 0x2000, 0x2000,  CRC(18d0262d) SHA1(c3920e3cabfe2b2add51881e262f090c5018e508) )
	ROM_LOAD( "sa_a_u3.bin", 0x4000, 0x2000,  CRC(4646832d) SHA1(9f1370b13cca9857b0ed13f58641ef4ba3c7326d) )
	ROM_LOAD( "sa_a_u4.bin", 0x6000, 0x2000,  CRC(57db2a79) SHA1(5286905d9bde697845a98bd77f31f2a96a8874fc) )
	ROM_LOAD( "sa_a_u5.bin", 0x8000, 0x2000,  CRC(85cbcdc4) SHA1(97e01e96c885ab7af4c3a3b586eb40374d54f12f) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "spaceace", 0, NO_DUMP )
ROM_END

ROM_START( saeuro )		/* Italian Sidam version */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sa_u45a.bin", 0x0000, 0x2000, CRC(41264d46) SHA1(3e0ecfb3249f857a29fe58a3853a55d31cbd63d6) )
	ROM_LOAD( "sa_u46a.bin", 0x2000, 0x2000, CRC(bc1c70cf) SHA1(cd6d2456ac2fbbfb86e1f31bd7cbd0cec0d31b45) )
	ROM_LOAD( "sa_u47a.bin", 0x4000, 0x2000, CRC(ff3f77c7) SHA1(d10ffd14ab9853cef8085c70aedfabea4059657e) )
	ROM_LOAD( "sa_u48a.bin", 0x6000, 0x2000, CRC(8c83ac81) SHA1(12818ee51ae8028a84bbbf3e43904b62942c76e3) )
	ROM_LOAD( "sa_u49a.bin", 0x6000, 0x2000, CRC(03b58fc3) SHA1(25e4c1df74e2d7cbb9e252e34f007bc1c9f015b2) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "sa_u33a.bin", 0x0000, 0x2000, CRC(a8c14612) SHA1(dbcf90b929e714f328bdcb0d8cd7c9e7d08a8be7) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "saeuro", 0, NO_DUMP )
ROM_END




/*************************************
 *
 *  Driver initialization
 *
 *************************************/

static DRIVER_INIT( fixed )
{
	laserdisc_type = LASERDISC_TYPE_FIXED;
}


static DRIVER_INIT( variable )
{
	laserdisc_type = LASERDISC_TYPE_VARIABLE;
}



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAMEL( 1983, dlair,    0,        dlair_ldv1000, dlaire, variable, ROT0, "Cinematronics", "Dragon's Lair (US Rev. F2)", GAME_NOT_WORKING, layout_dlair )
GAMEL( 1983, dlairf,   dlair,    dlair_ldv1000, dlaire, variable, ROT0, "Cinematronics", "Dragon's Lair (US Rev. F)",  GAME_NOT_WORKING, layout_dlair )
GAMEL( 1983, dlaire,   dlair,    dlair_ldv1000, dlaire, variable, ROT0, "Cinematronics", "Dragon's Lair (US Rev. E)",  GAME_NOT_WORKING, layout_dlair )
GAMEL( 1983, dlaird,   dlair,    dlair_ldv1000, dlair,  fixed,    ROT0, "Cinematronics", "Dragon's Lair (US Rev. D, Pioneer LD-V1000)",  GAME_NOT_WORKING, layout_dlair )
GAMEL( 1983, dlairc,   dlair,    dlair_pr7820,  dlair,  fixed,    ROT0, "Cinematronics", "Dragon's Lair (US Rev. C, Pioneer PR-7820)",  GAME_NOT_WORKING, layout_dlair )
GAMEL( 1983, dlairb,   dlair,    dlair_pr7820,  dlair,  fixed,    ROT0, "Cinematronics", "Dragon's Lair (US Rev. B, Pioneer PR-7820)",  GAME_NOT_WORKING, layout_dlair )
GAMEL( 1983, dlaira,   dlair,    dlair_pr7820,  dlair,  fixed,    ROT0, "Cinematronics", "Dragon's Lair (US Rev. A, Pioneer PR-7820)",  GAME_NOT_WORKING, layout_dlair )
GAMEL( 1983, dleuro,   dlair,    dleuro,        dleuro, fixed,    ROT0, "Cinematronics (Atari license)", "Dragon's Lair (European)",  GAME_NOT_WORKING, layout_dlair )
GAMEL( 1983, dlital,   dlair,    dleuro,        dleuro, fixed,    ROT0, "Cinematronics (Sidam license?)","Dragon's Lair (Italian)",  GAME_NOT_WORKING, layout_dlair )

GAMEL( 1983, spaceace, 0,        dlair_ldv1000, dlaire, variable, ROT0, "Cinematronics", "Space Ace (US Rev. A3)", GAME_NOT_WORKING, layout_dlair )
GAMEL( 1983, spaceaa2, spaceace, dlair_ldv1000, dlaire, variable, ROT0, "Cinematronics", "Space Ace (US Rev. A2)", GAME_NOT_WORKING, layout_dlair )
GAMEL( 1983, spaceaa,  spaceace, dlair_ldv1000, dlaire, variable, ROT0, "Cinematronics", "Space Ace (US Rev. A)", GAME_NOT_WORKING, layout_dlair )
GAMEL( 1983, saeuro,   spaceace, dleuro,        dleuro, fixed,    ROT0, "Cinematronics (Atari license)", "Space Ace (European)",  GAME_NOT_WORKING, layout_dlair )
