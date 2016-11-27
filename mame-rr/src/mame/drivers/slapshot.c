/***************************************************************************

Slapshot (c) Taito 1994
Operation Wolf 3 (c) Taito 1994
--------

David Graves

(this is based on the F2 driver by Bryan McPhail, Brad Oliver, Andrew Prime,
Nicola Salmoria.)

                *****

Slapshot uses one or two newer Taito custom ics, but the hardware is
very similar to the Taito F2 system, especially F2 games using the same
TC0480SCP tilemap generator (e.g. Metal Black).

This game has 6 separate layers of graphics - four 32x32 tiled scrolling
zoomable background planes of 16x16 tiles, a text plane with 64x64 8x8
character tiles with character definitions held in ram, and a sprite
plane with zoomable 16x16 sprites. This sprite system appears to be
identical to the one used in F2 and F3 games.

Slapshot switches in and out of the double-width tilemap mode of the
TC0480SCP. This is unusual, as most games stick to one width.

The palette generator is 8 bits per color gun like the Taito F3 system.
Like Metal Black the palette space is doubled, and the first half used
for sprites only so the second half can be devoted to tilemaps.

The main cpu is a 68000.

There is a slave Z80 which interfaces with a YM2610B for sound.
Commands are written to it by the 68000 (as in the Taito F2 games).


Slapshot custom ics
-------------------

TC0480SCP (IC61)    - known tilemap chip
TC0640FIO (IC83)    - new version of TC0510NIO io chip?
TC0650FDA (IC84)    - (palette?)
TC0360PRI (IC56)    - (common in pri/color combo on F2 boards)
TC0530SYC (IC58)    - known sound comm chip
TC0520TBC (IC36)    - known object chip
TC0540OBN (IC54)    - known object chip


TODO
====

Some hanging notes (try F2 while music is playing).

Sprite colors issue: when you do a super-shot, on the cut
screen the man (it's always the American) should be black.

Col $f8 is used for the man, col $fc for the red/pink
"explosion" under the puck. (Use this to track where they are
in spriteram quickly.) Both of these colors are only set
when the first super-shot happens, so it's clear those
colors are for the super-shot... but screenshot evidence
proves the man should be entirely black.

Extract common sprite stuff from this and taito_f2 ?


Code
----
$854 marks start of service mode

-----------------

Operation Wolf 3 is on almost identical hardware to Slapshot. It uses
far more graphics data and samples than Slapshot.

Compared to Taito's gun game Under Fire (1993), the hardware here is
obviously underpowered. Large data roms help the 68000 throw around the
gfx (a method used in Dino Rex) but can't disguise that it should have
been done using enhanced Z system or F3 system hardware.

***************************************************************************

Operation Wolf 3 (US Version) - (c) 1994 Taito America Corp.

Main Board K11E0801A - Not to Scale:-)

       D74 17                       Sub Board Connector
  D74 20      MC68000P12F                                    D74-05     SW2
       D74 18                                                D74-06
  D74 16         MK48T08B-10        TCO480SCP                84256A-70L
                                                             84256A-70L
MB8421-90LP  D74-02  84256A-70L             26.6860MHz  TCO640FIO
             D74-03  84256A-70L
             D74-04
                     TCO540OBN    TCO360PRI              TCO650FDA
                 TCO520TBC
                                    32.0000MHz    Y3016-F
               D74-01     TCO530SYC     D74 19    YM2610B
                                                  Z0840004PSC



Sub Board K91X0488A
 Basicly a few connectors, Caps, resistors & ADC0809CNN

Chips:
 Main: MC68000P12F
Sound: Z084004PSC, YM2610B, Y3016-F
  OSC: 32.000MHz, 26.6860MHz

Taito Custom:
  TCO480SCP
  TCO640FIO
  TCO650FDA
  TCO530SYC
  TCO520TBC
  TCO540OBN
  TCO360PRI

ST TimeKeeper Ram MK48T08B-10 - Lithuim Batery backed RAM chip
MB8421-90LP - Dual Port SRAM
ADC0809CNN - 8-bit Microprocessor Compatible A/D Converter
             With 8-Channel Multiplexer
 DataSheet:  http://www.national.com/ds/AD/ADC0808.pdf

Region byte at offset 0x031:
    d74_21.1  0x02  World Version
    d74_20.1  0x01  US Version
    d74_??.1  0x00  Will Produce a Japanese Version, but it's unknown if the
                    actual sound CPU code is the same as the World version,
                    US versions or different then both.
***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "video/taitoic.h"
#include "audio/taitosnd.h"
#include "sound/2610intf.h"
#include "machine/timekpr.h"
#include "machine/taitoio.h"
#include "includes/slapshot.h"


/******************************************************
                COLOR
******************************************************/

static READ16_HANDLER( color_ram_word_r )
{
	slapshot_state *state = (slapshot_state *)space->machine->driver_data;
	return state->color_ram[offset];
}

static WRITE16_HANDLER( color_ram_word_w )
{
	slapshot_state *state = (slapshot_state *)space->machine->driver_data;
	int r,g,b;
	COMBINE_DATA(&state->color_ram[offset]);

	if ((offset % 2) == 1)	/* assume words written sequentially */
	{
		r = (state->color_ram[offset- 1 ] & 0xff);
		g = (state->color_ram[offset] & 0xff00) >> 8;
		b = (state->color_ram[offset] & 0xff);

		palette_set_color(space->machine, offset / 2, MAKE_RGB(r,g,b));
	}
}


/***********************************************************
                INTERRUPTS
***********************************************************/

static TIMER_CALLBACK( slapshot_interrupt6 )
{
	slapshot_state *state = (slapshot_state *)machine->driver_data;
	cpu_set_input_line(state->maincpu, 6, HOLD_LINE);
}


static INTERRUPT_GEN( slapshot_interrupt )
{
	timer_set(device->machine, downcast<cpu_device *>(device)->cycles_to_attotime(200000 - 500), NULL, 0, slapshot_interrupt6);
	cpu_set_input_line(device, 5, HOLD_LINE);
}


/**********************************************************
                GAME INPUTS
**********************************************************/

static READ16_HANDLER( slapshot_service_input_r )
{
	slapshot_state *state = (slapshot_state *)space->machine->driver_data;
	switch (offset)
	{
		case 0x03:
			return ((input_port_read(space->machine, "SYSTEM") & 0xef) |
				  (input_port_read(space->machine, "SERVICE") & 0x10))  << 8;	/* IN3 + service switch */

		default:
			return tc0640fio_r(state->tc0640fio, offset) << 8;
	}
}


static READ16_HANDLER( opwolf3_adc_r )
{
	static const char *const adcnames[] = { "GUN1X", "GUN1Y", "GUN2X", "GUN2Y" };

	return input_port_read(space->machine, adcnames[offset]) << 8;
}

static WRITE16_HANDLER( opwolf3_adc_req_w )
{
	switch (offset)
	{
	case 0:
	/* gun outputs... not 100% sure they are correct yet */
	/* p2 gun recoil seems ever so slighty slower than p1 */
	/* also you get a false fire every once in a while on the p1 gun */

	if (((data & 0x100) == 0x100) && ((data & 0x400)==0))
		output_set_value("Player1_Gun_Recoil",1);
	else
		output_set_value("Player1_Gun_Recoil",0);

	if (((data & 0x100) == 0x100) && ((data & 0x400)==0x400))
		output_set_value("Player2_Gun_Recoil",1);
	else
		output_set_value("Player2_Gun_Recoil",0);
	break;
	}
	slapshot_state *state = (slapshot_state *)space->machine->driver_data;

	/* 4 writes a frame - one for each analogue port */
	cpu_set_input_line(state->maincpu, 3, HOLD_LINE);
}

/*****************************************************
                SOUND
*****************************************************/

static void reset_sound_region( running_machine *machine )
{
	slapshot_state *state = (slapshot_state *)machine->driver_data;
	memory_set_bank(machine, "bank10", state->banknum);
}

static WRITE8_HANDLER( sound_bankswitch_w )
{
	slapshot_state *state = (slapshot_state *)space->machine->driver_data;
	state->banknum = data & 7;
	reset_sound_region(space->machine);
}






static WRITE16_HANDLER( slapshot_msb_sound_w )
{
	slapshot_state *state = (slapshot_state *)space->machine->driver_data;
	if (offset == 0)
		tc0140syt_port_w(state->tc0140syt, 0, (data >> 8) & 0xff);
	else if (offset == 1)
		tc0140syt_comm_w(state->tc0140syt, 0, (data >> 8) & 0xff);

#ifdef MAME_DEBUG
	if (data & 0xff)
		popmessage("taito_msb_sound_w to low byte: %04x",data);
#endif
}

static READ16_HANDLER( slapshot_msb_sound_r )
{
	slapshot_state *state = (slapshot_state *)space->machine->driver_data;
	if (offset == 1)
		return ((tc0140syt_comm_r(state->tc0140syt, 0) & 0xff) << 8);
	else
		return 0;
}


/***********************************************************
             MEMORY STRUCTURES
***********************************************************/

static ADDRESS_MAP_START( slapshot_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x500000, 0x50ffff) AM_RAM	/* main RAM */
	AM_RANGE(0x600000, 0x60ffff) AM_RAM AM_BASE_SIZE_MEMBER(slapshot_state, spriteram, spriteram_size)	/* sprite ram */
	AM_RANGE(0x700000, 0x701fff) AM_RAM AM_BASE_SIZE_MEMBER(slapshot_state, spriteext, spriteext_size)	/* debugging */
	AM_RANGE(0x800000, 0x80ffff) AM_DEVREADWRITE("tc0480scp", tc0480scp_word_r, tc0480scp_word_w)	/* tilemaps */
	AM_RANGE(0x830000, 0x83002f) AM_DEVREADWRITE("tc0480scp", tc0480scp_ctrl_word_r, tc0480scp_ctrl_word_w)
	AM_RANGE(0x900000, 0x907fff) AM_READWRITE(color_ram_word_r, color_ram_word_w) AM_BASE_MEMBER(slapshot_state, color_ram)	/* 8bpg palette */
	AM_RANGE(0xa00000, 0xa03fff) AM_DEVREADWRITE8("mk48t08", timekeeper_r, timekeeper_w, 0xff00)	/* nvram (only low bytes used) */
	AM_RANGE(0xb00000, 0xb0001f) AM_DEVWRITE8("tc0360pri", tc0360pri_w, 0xff00)	/* priority chip */
	AM_RANGE(0xc00000, 0xc0000f) AM_DEVREADWRITE("tc0640fio", tc0640fio_halfword_byteswap_r, tc0640fio_halfword_byteswap_w)
	AM_RANGE(0xc00020, 0xc0002f) AM_READ(slapshot_service_input_r)	/* service mirror */
	AM_RANGE(0xd00000, 0xd00003) AM_READWRITE(slapshot_msb_sound_r, slapshot_msb_sound_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( opwolf3_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM
	AM_RANGE(0x500000, 0x50ffff) AM_RAM	/* main RAM */
	AM_RANGE(0x600000, 0x60ffff) AM_RAM AM_BASE_SIZE_MEMBER(slapshot_state, spriteram, spriteram_size)	/* sprite ram */
	AM_RANGE(0x700000, 0x701fff) AM_RAM AM_BASE_SIZE_MEMBER(slapshot_state, spriteext, spriteext_size)	/* debugging */
	AM_RANGE(0x800000, 0x80ffff) AM_DEVREADWRITE("tc0480scp", tc0480scp_word_r, tc0480scp_word_w)	/* tilemaps */
	AM_RANGE(0x830000, 0x83002f) AM_DEVREADWRITE("tc0480scp", tc0480scp_ctrl_word_r, tc0480scp_ctrl_word_w)
	AM_RANGE(0x900000, 0x907fff) AM_READWRITE(color_ram_word_r, color_ram_word_w) AM_BASE_MEMBER(slapshot_state, color_ram)	/* 8bpg palette */
	AM_RANGE(0xa00000, 0xa03fff) AM_DEVREADWRITE8("mk48t08", timekeeper_r, timekeeper_w, 0xff00)	/* nvram (only low bytes used) */
	AM_RANGE(0xb00000, 0xb0001f) AM_DEVWRITE8("tc0360pri", tc0360pri_w, 0xff00)	/* priority chip */
	AM_RANGE(0xc00000, 0xc0000f) AM_DEVREADWRITE("tc0640fio", tc0640fio_halfword_byteswap_r, tc0640fio_halfword_byteswap_w)
	AM_RANGE(0xc00020, 0xc0002f) AM_READ(slapshot_service_input_r)	 /* service mirror */
	AM_RANGE(0xd00000, 0xd00003) AM_READWRITE(slapshot_msb_sound_r, slapshot_msb_sound_w)
	AM_RANGE(0xe00000, 0xe00007) AM_READWRITE(opwolf3_adc_r, opwolf3_adc_req_w)
ADDRESS_MAP_END


/***************************************************************************/

static ADDRESS_MAP_START( opwolf3_z80_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank10")
	AM_RANGE(0xc000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xe003) AM_DEVREADWRITE("ymsnd", ym2610_r,ym2610_w)
	AM_RANGE(0xe200, 0xe200) AM_READNOP AM_DEVWRITE("tc0140syt", tc0140syt_slave_port_w)
	AM_RANGE(0xe201, 0xe201) AM_DEVREADWRITE("tc0140syt", tc0140syt_slave_comm_r, tc0140syt_slave_comm_w)
	AM_RANGE(0xe400, 0xe403) AM_WRITENOP /* pan */
	AM_RANGE(0xea00, 0xea00) AM_READNOP
	AM_RANGE(0xee00, 0xee00) AM_WRITENOP /* ? */
	AM_RANGE(0xf000, 0xf000) AM_WRITENOP /* ? */
	AM_RANGE(0xf200, 0xf200) AM_WRITE(sound_bankswitch_w)
ADDRESS_MAP_END


/***********************************************************
             INPUT PORTS (DIPs in nvram)
***********************************************************/

/* Tags below are the ones expected by TC0640FIO_halfword_byteswap_r */
static INPUT_PORTS_START( slapshot )
	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )	/* bit is service switch at c0002x */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("JOY")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)

	PORT_START("SERVICE")      /* IN5, so we can OR in service switch */
	PORT_SERVICE_NO_TOGGLE(0x10, IP_ACTIVE_LOW)
INPUT_PORTS_END

/* Tags below are the ones expected by TC0640FIO_halfword_byteswap_r */
static INPUT_PORTS_START( opwolf3 )
	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN4 )

	PORT_START("BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )	PORT_NAME("1 Player Start/Button3")// also button 3
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )	PORT_NAME("2 Player Start/Button3")// also button 3
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* bit is service switch at c0002x */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("JOY")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SERVICE")
	PORT_SERVICE_NO_TOGGLE(0x10, IP_ACTIVE_LOW)

	PORT_START("GUN1X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(30) PORT_KEYDELTA(20) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("GUN1Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(30) PORT_KEYDELTA(20) PORT_PLAYER(1)

	PORT_START("GUN2X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(30) PORT_KEYDELTA(20) PORT_REVERSE PORT_PLAYER(2)

	PORT_START("GUN2Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(30) PORT_KEYDELTA(20) PORT_PLAYER(2)
INPUT_PORTS_END

/***********************************************************
                GFX DECODING

***********************************************************/

static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,2),
	6,
	{ RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+1, 0, 1, 2, 3 },
	{
	4, 0, 12, 8,
	16+4, 16+0, 16+12, 16+8,
	32+4, 32+0, 32+12, 32+8,
	48+4, 48+0, 48+12, 48+8 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
			8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	128*8	/* every sprite takes 128 consecutive bytes */
};

static const gfx_layout slapshot_charlayout =
{
	16,16,    /* 16*16 characters */
	RGN_FRAC(1,1),
	4,        /* 4 bits per pixel */
	{ 0, 1, 2, 3 },
	{ 1*4, 0*4, 5*4, 4*4, 3*4, 2*4, 7*4, 6*4, 9*4, 8*4, 13*4, 12*4, 11*4, 10*4, 15*4, 14*4 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64, 8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	128*8     /* every sprite takes 128 consecutive bytes */
};

static GFXDECODE_START( slapshot )
	GFXDECODE_ENTRY( "gfx2", 0x0, tilelayout,  0, 256 )	/* sprite parts */
	GFXDECODE_ENTRY( "gfx1", 0x0, slapshot_charlayout,  0, 256 )	/* sprites & playfield */
GFXDECODE_END



/**************************************************************
                 YM2610B (SOUND)
**************************************************************/

/* handler called by the YM2610 emulator when the internal timers cause an IRQ */
static void irqhandler( running_device *device, int irq )
{
	slapshot_state *state = (slapshot_state *)device->machine->driver_data;
	cpu_set_input_line(state->audiocpu, 0, irq ? ASSERT_LINE : CLEAR_LINE);
}

static const ym2610_interface ym2610_config =
{
	irqhandler
};


/***********************************************************
                 MACHINE DRIVERS
***********************************************************/

static const tc0480scp_interface slapshot_tc0480scp_intf =
{
	1, 2,		/* gfxnum, txnum */
	3,		/* pixels */
	30, 9,		/* x_offset, y_offset */
	-1, 1,		/* text_xoff, text_yoff */
	0, 2,		/* flip_xoff, flip_yoff */
	256		/* col_base */
};

static const tc0640fio_interface slapshot_io_intf =
{
	DEVCB_NULL, DEVCB_INPUT_PORT("COINS"),
	DEVCB_INPUT_PORT("BUTTONS"), DEVCB_INPUT_PORT("SYSTEM"), DEVCB_INPUT_PORT("JOY")	/* port read handlers */
};

static const tc0140syt_interface slapshot_tc0140syt_intf =
{
	"maincpu", "audiocpu"
};

static STATE_POSTLOAD( slapshot_postload )
{
	reset_sound_region(machine);
}

static MACHINE_START( slapshot )
{
	slapshot_state *state = (slapshot_state *)machine->driver_data;

	memory_configure_bank(machine, "bank10", 0, 4, memory_region(machine, "audiocpu") + 0xc000, 0x4000);

	state->maincpu = machine->device("maincpu");
	state->audiocpu = machine->device("audiocpu");
	state->tc0140syt = machine->device("tc0140syt");
	state->tc0480scp = machine->device("tc0480scp");
	state->tc0360pri = machine->device("tc0360pri");
	state->tc0640fio = machine->device("tc0640fio");

	state->banknum = 0;
	state_save_register_global(machine, state->banknum);
	state_save_register_postload(machine, slapshot_postload, NULL);
}


static MACHINE_DRIVER_START( slapshot )

	/* driver data */
	MDRV_DRIVER_DATA(slapshot_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 14346000)	/* 28.6860 MHz / 2 ??? */
	MDRV_CPU_PROGRAM_MAP(slapshot_map)
	MDRV_CPU_VBLANK_INT("screen", slapshot_interrupt)

	MDRV_CPU_ADD("audiocpu", Z80,32000000/8)	/* 4 MHz */
	MDRV_CPU_PROGRAM_MAP(opwolf3_z80_sound_map)

	MDRV_QUANTUM_TIME(HZ(600))

	MDRV_MACHINE_START(slapshot)

	MDRV_TC0640FIO_ADD("tc0640fio", slapshot_io_intf)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(40*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(slapshot)
	MDRV_PALETTE_LENGTH(8192)

	MDRV_VIDEO_START(slapshot)
	MDRV_VIDEO_EOF(taito_no_buffer)
	MDRV_VIDEO_UPDATE(slapshot)

	MDRV_TC0480SCP_ADD("tc0480scp", slapshot_tc0480scp_intf)
	MDRV_TC0360PRI_ADD("tc0360pri")

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2610B, 16000000/2)
	MDRV_SOUND_CONFIG(ym2610_config)
	MDRV_SOUND_ROUTE(0, "lspeaker",  0.25)
	MDRV_SOUND_ROUTE(0, "rspeaker", 0.25)
	MDRV_SOUND_ROUTE(1, "lspeaker",  1.0)
	MDRV_SOUND_ROUTE(2, "rspeaker", 1.0)

	MDRV_MK48T08_ADD( "mk48t08" )

	MDRV_TC0140SYT_ADD("tc0140syt", slapshot_tc0140syt_intf)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( opwolf3 )

	/* driver data */
	MDRV_DRIVER_DATA(slapshot_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 14346000)	/* 28.6860 MHz / 2 ??? */
	MDRV_CPU_PROGRAM_MAP(opwolf3_map)
	MDRV_CPU_VBLANK_INT("screen", slapshot_interrupt)

	MDRV_CPU_ADD("audiocpu", Z80,32000000/8)	/* 4 MHz */
	MDRV_CPU_PROGRAM_MAP(opwolf3_z80_sound_map)

	MDRV_QUANTUM_TIME(HZ(600))

	MDRV_MACHINE_START(slapshot)

	MDRV_TC0640FIO_ADD("tc0640fio", slapshot_io_intf)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(40*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(slapshot)
	MDRV_PALETTE_LENGTH(8192)

	MDRV_VIDEO_START(slapshot)
	MDRV_VIDEO_EOF(taito_no_buffer)
	MDRV_VIDEO_UPDATE(slapshot)

	MDRV_TC0480SCP_ADD("tc0480scp", slapshot_tc0480scp_intf)
	MDRV_TC0360PRI_ADD("tc0360pri")

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2610B, 16000000/2)
	MDRV_SOUND_CONFIG(ym2610_config)
	MDRV_SOUND_ROUTE(0, "lspeaker",  0.25)
	MDRV_SOUND_ROUTE(0, "rspeaker", 0.25)
	MDRV_SOUND_ROUTE(1, "lspeaker",  1.0)
	MDRV_SOUND_ROUTE(2, "rspeaker", 1.0)

	MDRV_MK48T08_ADD( "mk48t08" )

	MDRV_TC0140SYT_ADD("tc0140syt", slapshot_tc0140syt_intf)
MACHINE_DRIVER_END

/***************************************************************************
                    DRIVERS
***************************************************************************/

ROM_START( slapshot )
	ROM_REGION( 0x100000, "maincpu", 0 )	/* 1024K for 68000 code */
	ROM_LOAD16_BYTE( "d71-15.3",  0x00000, 0x80000, CRC(1470153f) SHA1(63fd5314fcaafba7326fd9481e3c686901dde65c) )
	ROM_LOAD16_BYTE( "d71-16.1",  0x00001, 0x80000, CRC(f13666e0) SHA1(e8b475163ea7da5ee3f2b900004cc67c684bab75) )

	ROM_REGION( 0x1c000, "audiocpu", 0 )	/* sound cpu */
	ROM_LOAD    ( "d71-07.77",    0x00000, 0x4000, CRC(dd5f670c) SHA1(743a9563c40fe40178c9ec8eece71a08380c2239) )
	ROM_CONTINUE(                 0x10000, 0xc000 )	/* banked stuff */

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "d71-04.79", 0x00000, 0x80000, CRC(b727b81c) SHA1(9f56160e2b3e4d59cfa96b5c013f4e368781666e) )	/* SCR */
	ROM_LOAD16_BYTE( "d71-05.80", 0x00001, 0x80000, CRC(7b0f5d6d) SHA1(a54e4a651dc7cdc160286afb3d38531c7b9396b1) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "d71-01.23", 0x000000, 0x100000, CRC(0b1e8c27) SHA1(ffa452f7414f3d61edb69bb61b29a0cc8d9176d0) )	/* OBJ 6bpp */
	ROM_LOAD16_BYTE( "d71-02.24", 0x000001, 0x100000, CRC(ccaaea2d) SHA1(71b507f215f37e991abae5523642417a6b23a70d) )
	ROM_LOAD       ( "d71-03.25", 0x300000, 0x100000, CRC(dccef9ec) SHA1(ee7a49727b822cf4c1d7acff994b77ea6191c423) )
	ROM_FILL       (              0x200000, 0x100000, 0 )

	ROM_REGION( 0x80000, "ymsnd", 0 )	/* ADPCM samples */
	ROM_LOAD( "d71-06.37", 0x00000, 0x80000, CRC(f3324188) SHA1(70dd724441eae8614218bc7f0f51860bd2462f0c) )

	/* no Delta-T samples */

//  Pals (not dumped)
//  ROM_LOAD( "d71-08.40",  0x00000, 0x00???, NO_DUMP )
//  ROM_LOAD( "d71-09.57",  0x00000, 0x00???, NO_DUMP )
//  ROM_LOAD( "d71-10.60",  0x00000, 0x00???, NO_DUMP )
//  ROM_LOAD( "d71-11.42",  0x00000, 0x00???, NO_DUMP )
//  ROM_LOAD( "d71-12.59",  0x00000, 0x00???, NO_DUMP )
//  ROM_LOAD( "d71-13.8",   0x00000, 0x00???, NO_DUMP )
ROM_END

ROM_START( opwolf3 )
	ROM_REGION( 0x200000, "maincpu", 0 )	/* 1024K for 68000 code */
	ROM_LOAD16_BYTE( "d74_16.3",  0x000000, 0x80000, CRC(198ff1f6) SHA1(f5b51e39cd73ea56cbf53731d3c885bfcecbd696) )
	ROM_LOAD16_BYTE( "d74_21.1",  0x000001, 0x80000, CRC(c61c558b) SHA1(6340eb83ba4cd8d7c63b22ea738c8367c87c1de1) )
	ROM_LOAD16_BYTE( "d74_18.18", 0x100000, 0x80000, CRC(bd5d7cdb) SHA1(29f1cd7b86bc05f873e93f088194113da87a3b86) )	// data ???
	ROM_LOAD16_BYTE( "d74_17.17", 0x100001, 0x80000, CRC(ac35a672) SHA1(8136bd076443bfaeb3d339971d88951e8b2b59b4) )	// data ???

	ROM_REGION( 0x1c000, "audiocpu", 0 )	/* sound cpu */
	ROM_LOAD    ( "d74_22.77",    0x00000, 0x4000, CRC(118374a6) SHA1(cc1d0d28efdf1df3e648e7d932405811854ba4ee) )
	ROM_CONTINUE(                 0x10000, 0xc000 )	/* banked stuff */

	ROM_REGION( 0x400000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "d74_05.80", 0x000000, 0x200000, CRC(85ea64cc) SHA1(1960a934191c451df1554323d47f6fc64939b0ce) )	/* SCR */
	ROM_LOAD16_BYTE( "d74_06.81", 0x000001, 0x200000, CRC(2fa1e08d) SHA1(f1f34b308202fe08e73535424b5b4e3d91295224) )

	ROM_REGION( 0x800000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "d74_02.23", 0x000000, 0x200000, CRC(aab86332) SHA1(b9133407504e9ef4fd5ae7d284cdb0c7f78f9a99) )	/* OBJ 6bpp */
	ROM_LOAD16_BYTE( "d74_03.24", 0x000001, 0x200000, CRC(3f398916) SHA1(4b6a3ee0baf5f32e24e5040f233300f1ca347fe7) )
	ROM_LOAD       ( "d74_04.25", 0x600000, 0x200000, CRC(2f385638) SHA1(1ba2ec7d9b1c491e1cc6d7e646e09ef2bc063f25) )
	ROM_FILL       (              0x400000, 0x200000, 0 )

	ROM_REGION( 0x200000, "ymsnd", 0 )	/* ADPCM samples */
	ROM_LOAD( "d74_01.37",  0x000000, 0x200000, CRC(115313e0) SHA1(51a69e7a26960b1328ccefeaec0fb26bdccc39f2) )

	/* no Delta-T samples */
ROM_END

ROM_START( opwolf3u )
	ROM_REGION( 0x200000, "maincpu", 0 )	/* 1024K for 68000 code */
	ROM_LOAD16_BYTE( "d74_16.3",  0x000000, 0x80000, CRC(198ff1f6) SHA1(f5b51e39cd73ea56cbf53731d3c885bfcecbd696) )
	ROM_LOAD16_BYTE( "d74_20.1",  0x000001, 0x80000, CRC(960fd892) SHA1(2584a048d29a96b69428fba2b71269ea6ccf9010) )
	ROM_LOAD16_BYTE( "d74_18.18", 0x100000, 0x80000, CRC(bd5d7cdb) SHA1(29f1cd7b86bc05f873e93f088194113da87a3b86) )	// data ???
	ROM_LOAD16_BYTE( "d74_17.17", 0x100001, 0x80000, CRC(ac35a672) SHA1(8136bd076443bfaeb3d339971d88951e8b2b59b4) )	// data ???

	ROM_REGION( 0x1c000, "audiocpu", 0 )	/* sound cpu */
	ROM_LOAD    ( "d74_19.77",    0x00000, 0x4000, CRC(05d53f06) SHA1(48b0cd68ad3758f424552a4e3833c5a1c2f1825b) )
	ROM_CONTINUE(                 0x10000, 0xc000 )	/* banked stuff */

	ROM_REGION( 0x400000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "d74_05.80", 0x000000, 0x200000, CRC(85ea64cc) SHA1(1960a934191c451df1554323d47f6fc64939b0ce) )	/* SCR */
	ROM_LOAD16_BYTE( "d74_06.81", 0x000001, 0x200000, CRC(2fa1e08d) SHA1(f1f34b308202fe08e73535424b5b4e3d91295224) )

	ROM_REGION( 0x800000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "d74_02.23", 0x000000, 0x200000, CRC(aab86332) SHA1(b9133407504e9ef4fd5ae7d284cdb0c7f78f9a99) )	/* OBJ 6bpp */
	ROM_LOAD16_BYTE( "d74_03.24", 0x000001, 0x200000, CRC(3f398916) SHA1(4b6a3ee0baf5f32e24e5040f233300f1ca347fe7) )
	ROM_LOAD       ( "d74_04.25", 0x600000, 0x200000, CRC(2f385638) SHA1(1ba2ec7d9b1c491e1cc6d7e646e09ef2bc063f25) )
	ROM_FILL       (              0x400000, 0x200000, 0 )

	ROM_REGION( 0x200000, "ymsnd", 0 )	/* ADPCM samples */
	ROM_LOAD( "d74_01.37",  0x000000, 0x200000, CRC(115313e0) SHA1(51a69e7a26960b1328ccefeaec0fb26bdccc39f2) )

	/* no Delta-T samples */
ROM_END


static DRIVER_INIT( slapshot )
{
	UINT32 offset,i;
	UINT8 *gfx = memory_region(machine, "gfx2");
	int size = memory_region_length(machine, "gfx2");
	int data;

	offset = size / 2;
	for (i = size / 2 + size / 4; i < size; i++)
	{
		int d1, d2, d3, d4;

		/* Expand 2bits into 4bits format */
		data = gfx[i];
		d1 = (data >> 0) & 3;
		d2 = (data >> 2) & 3;
		d3 = (data >> 4) & 3;
		d4 = (data >> 6) & 3;

		gfx[offset] = (d1 << 2) | (d2 << 6);
		offset++;

		gfx[offset] = (d3 << 2) | (d4 << 6);
		offset++;
	}
}

GAME( 1994, slapshot, 0,       slapshot, slapshot, slapshot, ROT0, "Taito Corporation",         "Slap Shot (Japan)", 0 )
GAME( 1994, opwolf3,  0,       opwolf3,  opwolf3,  slapshot, ROT0, "Taito Corporation Japan",   "Operation Wolf 3 (World)", 0 )
GAME( 1994, opwolf3u, opwolf3, opwolf3,  opwolf3,  slapshot, ROT0, "Taito America Corporation", "Operation Wolf 3 (US)", 0 )
