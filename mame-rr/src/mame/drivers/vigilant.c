/***************************************************************************

  Vigilante

If you have any questions about how this driver works, don't hesitate to
ask.  - Mike Balfour (mab22@po.cwru.edu)


TS 2004.12.26.:
- Buccaneers - incomplete dump, different sound hw (YM2203x2)
    (to enter test mode press any button durning memory test)

Buccaneers has a 5.6888 Mhz and a 18.432 Mhz OSC

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "deprecat.h"
#include "audio/m72.h"
#include "sound/dac.h"
#include "sound/2203intf.h"
#include "sound/2151intf.h"

/* video/vigilant.c */
VIDEO_START( vigilant );
VIDEO_RESET( vigilant );
WRITE8_HANDLER( vigilant_paletteram_w );
WRITE8_HANDLER( vigilant_horiz_scroll_w );
WRITE8_HANDLER( vigilant_rear_horiz_scroll_w );
WRITE8_HANDLER( vigilant_rear_color_w );
VIDEO_UPDATE( vigilant );
VIDEO_UPDATE( kikcubic );


static WRITE8_HANDLER( vigilant_bank_select_w )
{
	int bankaddress;
	UINT8 *RAM = memory_region(space->machine, "maincpu");

	bankaddress = 0x10000 + (data & 0x07) * 0x4000;
	memory_set_bankptr(space->machine, "bank1",&RAM[bankaddress]);
}

/***************************************************************************
 vigilant_out2_w
 **************************************************************************/
static WRITE8_HANDLER( vigilant_out2_w )
{
	/* D0 = FILP = Flip screen? */
	/* D1 = COA1 = Coin Counter A? */
	/* D2 = COB1 = Coin Counter B? */

	/* The hardware has both coin counters hooked up to a single meter. */
	coin_counter_w(space->machine, 0,data & 0x02);
	coin_counter_w(space->machine, 1,data & 0x04);

//  data & 0x01 cocktail mode
}

static WRITE8_HANDLER( kikcubic_coin_w )
{
	/* bits 0 is flip screen */

	/* bit 1 is used but unknown */

	/* bits 4/5 are coin counters */
	coin_counter_w(space->machine, 0,data & 0x10);
	coin_counter_w(space->machine, 1,data & 0x20);
}



static ADDRESS_MAP_START( vigilant_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")		/* Fallthrough */
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xc020, 0xc0df) AM_RAM AM_BASE_SIZE_GENERIC(spriteram)
	AM_RANGE(0xc800, 0xcfff) AM_RAM_WRITE(vigilant_paletteram_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0xd000, 0xdfff) AM_RAM AM_BASE_GENERIC(videoram) AM_SIZE_GENERIC(videoram)
	AM_RANGE(0xe000, 0xefff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( vigilant_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("IN0") AM_WRITE(m72_sound_command_byte_w)	/* SD */
	AM_RANGE(0x01, 0x01) AM_READ_PORT("IN1") AM_WRITE(vigilant_out2_w)			/* OUT2 */
	AM_RANGE(0x02, 0x02) AM_READ_PORT("IN2")
	AM_RANGE(0x03, 0x03) AM_READ_PORT("DSW1")
	AM_RANGE(0x04, 0x04) AM_READ_PORT("DSW2") AM_WRITE(vigilant_bank_select_w)	/* PBANK */
	AM_RANGE(0x80, 0x81) AM_WRITE(vigilant_horiz_scroll_w)		/* HSPL, HSPH */
	AM_RANGE(0x82, 0x83) AM_WRITE(vigilant_rear_horiz_scroll_w) /* RHSPL, RHSPH */
	AM_RANGE(0x84, 0x84) AM_WRITE(vigilant_rear_color_w)		/* RCOD */
ADDRESS_MAP_END

static ADDRESS_MAP_START( kikcubic_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")		/* Fallthrough */
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xc000, 0xc0ff) AM_RAM AM_BASE_SIZE_GENERIC(spriteram)
	AM_RANGE(0xc800, 0xcaff) AM_RAM_WRITE(vigilant_paletteram_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0xd000, 0xdfff) AM_RAM AM_BASE_GENERIC(videoram) AM_SIZE_GENERIC(videoram)
	AM_RANGE(0xe000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( kikcubic_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("DSW1") AM_WRITE(kikcubic_coin_w)	/* also flip screen, and...? */
	AM_RANGE(0x01, 0x01) AM_READ_PORT("DSW2")
	AM_RANGE(0x02, 0x02) AM_READ_PORT("IN0")
	AM_RANGE(0x03, 0x03) AM_READ_PORT("IN1")
	AM_RANGE(0x04, 0x04) AM_READ_PORT("IN2") AM_WRITE(vigilant_bank_select_w)
	AM_RANGE(0x06, 0x06) AM_WRITE(m72_sound_command_byte_w)
//  AM_RANGE(0x07, 0x07) AM_WRITENOP /* ?? */
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xf000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVREADWRITE("ymsnd", ym2151_r, ym2151_w)
	AM_RANGE(0x80, 0x81) AM_READWRITE(soundlatch_r, vigilant_sample_addr_w)	/* STL / STH */
	AM_RANGE(0x82, 0x82) AM_DEVWRITE("dac", m72_sample_w)			/* COUNT UP */
	AM_RANGE(0x83, 0x83) AM_WRITE(m72_sound_irq_ack_w)	/* IRQ clear */
	AM_RANGE(0x84, 0x84) AM_READ(m72_sample_r)	/* S ROM C */
ADDRESS_MAP_END

static ADDRESS_MAP_START( buccanrs_sound_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVREADWRITE("ym1", ym2203_r, ym2203_w)
	AM_RANGE(0x02, 0x03) AM_DEVREADWRITE("ym2", ym2203_r, ym2203_w)
	AM_RANGE(0x80, 0x80) AM_READ(soundlatch_r)				/* SDRE */
	AM_RANGE(0x80, 0x81) AM_WRITE(vigilant_sample_addr_w)	/* STL / STH */
	AM_RANGE(0x82, 0x82) AM_DEVWRITE("dac", m72_sample_w)				/* COUNT UP */
	AM_RANGE(0x83, 0x83) AM_WRITE(m72_sound_irq_ack_w)		/* IRQ clear */
	AM_RANGE(0x84, 0x84) AM_READ(m72_sample_r)				/* S ROM C */
ADDRESS_MAP_END


static INPUT_PORTS_START( vigilant )
	PORT_START("IN0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT(0xF0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON2 )

	PORT_START("IN2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )
	PORT_DIPSETTING(	0x02, "2" )
	PORT_DIPSETTING(	0x03, "3" )
	PORT_DIPSETTING(	0x01, "4" )
	PORT_DIPSETTING(	0x00, "5" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(	0x04, DEF_STR( Normal ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x08, 0x08, "Decrease of Energy" )
	PORT_DIPSETTING(	0x08, "Slow" )
	PORT_DIPSETTING(	0x00, "Fast" )
	/* TODO: support the different settings which happen in Coin Mode 2 */
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coinage ) )
	PORT_DIPSETTING(	0xa0, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(	0xb0, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(	0xc0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(	0xd0, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(	0x10, DEF_STR( 8C_3C ) )
	PORT_DIPSETTING(	0xe0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(	0x20, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(	0x30, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(	0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(	0x40, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(	0x90, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(	0x80, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(	0x70, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(	0x60, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(	0x50, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Free_Play ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(	0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Cocktail ) )
/* This activates a different coin mode. Look at the dip switch setting schematic */
	PORT_DIPNAME( 0x04, 0x04, "Coin Mode" )
	PORT_DIPSETTING(	0x04, "Mode 1" )
	PORT_DIPSETTING(	0x00, "Mode 2" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(	0x00, DEF_STR( No ) )
	PORT_DIPSETTING(	0x10, DEF_STR( Yes ) )
	/* In stop mode, press 2 to stop and 1 to restart */
	PORT_DIPNAME( 0x20, 0x20, "Stop Mode (Cheat)")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Invulnerability (Cheat)")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( kikcubic )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(19)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	/* TODO: support the different settings which happen in Coin Mode 2 */
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coinage ) )
	PORT_DIPSETTING(	0xa0, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(	0xb0, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(	0xc0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(	0xd0, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(	0xe0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(	0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(	0x70, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(	0x60, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(	0x50, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(	0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(	0x30, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Free_Play ) )
//  PORT_DIPSETTING(    0x10, "Undefined" )
//  PORT_DIPSETTING(    0x20, "Undefined" )
//  PORT_DIPSETTING(    0x80, "Undefined" )
//  PORT_DIPSETTING(    0x90, "Undefined" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(	0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Cocktail ) )
/* This activates a different coin mode. Look at the dip switch setting schematic */
	PORT_DIPNAME( 0x04, 0x04, "Coin Mode" )
	PORT_DIPSETTING(	0x04, "Mode 1" )
	PORT_DIPSETTING(	0x00, "Mode 2" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(	0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Invulnerability (Cheat)")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Level_Select ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Player Adding" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )
INPUT_PORTS_END

static INPUT_PORTS_START( buccanrs )
	PORT_START("IN0")
	PORT_SERVICE( 0x2f, IP_ACTIVE_LOW ) // any of these bits while booting will enable service mode
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("IN2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(	0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(	0x07, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(	0x00, "5 Coins/2 Credits" )
	PORT_DIPSETTING(	0x0a, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(	0x06, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(	0x03, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(	0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(	0x05, "3 Coins/5 Credits" )
	PORT_DIPSETTING(	0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(	0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(	0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(	0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(	0x40, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(	0x70, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(	0x00, "5 Coins/2 Credits" )
	PORT_DIPSETTING(	0xa0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(	0x60, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(	0x30, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(	0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(	0x50, "3 Coins/5 Credits" )
	PORT_DIPSETTING(	0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(	0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(	0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(	0xb0, DEF_STR( 1C_5C ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Invicibility (time still decrease)" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
INPUT_PORTS_END

static INPUT_PORTS_START( buccanra )
	PORT_INCLUDE( buccanrs )

	PORT_MODIFY("IN0") /* this port is reversed on this set.. */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_SERVICE( 0xf4, IP_ACTIVE_LOW ) // any of these bits while booting will enable service mode
INPUT_PORTS_END


static const gfx_layout text_layout =
{
	8,8, /* tile size */
	RGN_FRAC(1,2), /* number of tiles */
	4, /* bits per pixel */
	{RGN_FRAC(1,2),RGN_FRAC(1,2)+4,0,4}, /* plane offsets */
	{ 0,1,2,3, 64+0,64+1,64+2,64+3 }, /* x offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 }, /* y offsets */
	128
};

static const gfx_layout sprite_layout =
{
	16,16,	/* tile size */
	RGN_FRAC(1,2),	/* number of sprites ($1000) */
	4,		/* bits per pixel */
	{RGN_FRAC(1,2),RGN_FRAC(1,2)+4,0,4}, /* plane offsets */
	{ /* x offsets */
		0x00*8+0,0x00*8+1,0x00*8+2,0x00*8+3,
		0x10*8+0,0x10*8+1,0x10*8+2,0x10*8+3,
		0x20*8+0,0x20*8+1,0x20*8+2,0x20*8+3,
		0x30*8+0,0x30*8+1,0x30*8+2,0x30*8+3
	},
	{ /* y offsets */
		0x00*8, 0x01*8, 0x02*8, 0x03*8,
		0x04*8, 0x05*8, 0x06*8, 0x07*8,
		0x08*8, 0x09*8, 0x0A*8, 0x0B*8,
		0x0C*8, 0x0D*8, 0x0E*8, 0x0F*8
	},
	0x40*8
};

static const gfx_layout sprite_layout_buccanrs =
{
	16,16,	/* tile size */
	RGN_FRAC(1,2),	/* number of sprites ($1000) */
	4,		/* bits per pixel */
	{RGN_FRAC(1,2),RGN_FRAC(1,2)+4,0,4}, /* plane offsets */
	{ /* x offsets */
		0x00*8+3,0x00*8+2,0x00*8+1,0x00*8+0,
		0x10*8+3,0x10*8+2,0x10*8+1,0x10*8+0,
		0x20*8+3,0x20*8+2,0x20*8+1,0x20*8+0,
		0x30*8+3,0x30*8+2,0x30*8+1,0x30*8+0
	},
	{ /* y offsets */
		0x00*8, 0x01*8, 0x02*8, 0x03*8,
		0x04*8, 0x05*8, 0x06*8, 0x07*8,
		0x08*8, 0x09*8, 0x0A*8, 0x0B*8,
		0x0C*8, 0x0D*8, 0x0E*8, 0x0F*8
	},
	0x40*8
};


static const gfx_layout back_layout =
{
	32,1, /* tile size */
	RGN_FRAC(1,1), /* number of tiles */
	4, /* bits per pixel */
	{0,2,4,6}, /* plane offsets */
	{ 0*8+1, 0*8,  1*8+1, 1*8, 2*8+1, 2*8, 3*8+1, 3*8, 4*8+1, 4*8, 5*8+1, 5*8,
	6*8+1, 6*8, 7*8+1, 7*8, 8*8+1, 8*8, 9*8+1, 9*8, 10*8+1, 10*8, 11*8+1, 11*8,
	12*8+1, 12*8, 13*8+1, 13*8, 14*8+1, 14*8, 15*8+1, 15*8 }, /* x offsets */
	{ 0 }, /* y offsets */
	16*8
};

static const gfx_layout buccaneer_back_layout =
{
	32,1, /* tile size */
	RGN_FRAC(1,1), /* number of tiles */
	4, /* bits per pixel */
	{6,4,2,0}, /* plane offsets */
	{ 0*8+1, 0*8,  1*8+1, 1*8, 2*8+1, 2*8, 3*8+1, 3*8, 4*8+1, 4*8, 5*8+1, 5*8,
	6*8+1, 6*8, 7*8+1, 7*8, 8*8+1, 8*8, 9*8+1, 9*8, 10*8+1, 10*8, 11*8+1, 11*8,
	12*8+1, 12*8, 13*8+1, 13*8, 14*8+1, 14*8, 15*8+1, 15*8 }, /* x offsets */
	{ 0 }, /* y offsets */
	16*8
};

static GFXDECODE_START( vigilant )
	GFXDECODE_ENTRY( "gfx1", 0, text_layout,   256, 16 )	/* colors 256-511 */
	GFXDECODE_ENTRY( "gfx2", 0, sprite_layout,   0, 16 )	/* colors   0-255 */
	GFXDECODE_ENTRY( "gfx3", 0, back_layout,   512,  2 )	/* actually the background uses colors */
													/* 256-511, but giving it exclusive */
													/* pens we can handle it more easily. */
GFXDECODE_END

static GFXDECODE_START( buccanrs )
	GFXDECODE_ENTRY( "gfx1", 0, text_layout,   256, 16 )	/* colors 256-511 */
	GFXDECODE_ENTRY( "gfx2", 0, sprite_layout_buccanrs,   0, 16 )	/* colors   0-255 */
	GFXDECODE_ENTRY( "gfx3", 0, buccaneer_back_layout,   512,  2 )	/* actually the background uses colors */
													/* 256-511, but giving it exclusive */
													/* pens we can handle it more easily. */
GFXDECODE_END

static GFXDECODE_START( kikcubic )
	GFXDECODE_ENTRY( "gfx1", 0, text_layout,   0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, sprite_layout, 0, 16 )
GFXDECODE_END



static const ym2151_interface ym2151_config =
{
	m72_ym2151_irq_handler
};

static const ym2203_interface ym2203_config =
{
	{
		AY8910_LEGACY_OUTPUT,
		AY8910_DEFAULT_LOADS,
		DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL
	},
	m72_ym2151_irq_handler
};


static MACHINE_DRIVER_START( vigilant )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, 3579645)		   /* 3.579645 MHz */
	MDRV_CPU_PROGRAM_MAP(vigilant_map)
	MDRV_CPU_IO_MAP(vigilant_io_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_CPU_ADD("soundcpu", Z80, 3579645)		   /* 3.579645 MHz */
	MDRV_CPU_PROGRAM_MAP(sound_map)
	MDRV_CPU_IO_MAP(sound_io_map)
	MDRV_CPU_VBLANK_INT_HACK(nmi_line_pulse,128)	/* clocked by V1 */
								/* IRQs are generated by main Z80 and YM2151 */
	MDRV_SOUND_START(m72)
	MDRV_SOUND_RESET(m72)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(55)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(16*8, (64-16)*8-1, 0*8, 32*8-1 )

	MDRV_GFXDECODE(vigilant)
	MDRV_PALETTE_LENGTH(512+32)	/* 512 real palette, 32 virtual palette */

	MDRV_VIDEO_START(vigilant)
	MDRV_VIDEO_RESET(vigilant)
	MDRV_VIDEO_UPDATE(vigilant)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2151, 3579645)
	MDRV_SOUND_CONFIG(ym2151_config)
	MDRV_SOUND_ROUTE(0, "lspeaker", 0.55)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.55)

	MDRV_SOUND_ADD("dac", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( buccanrs )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, 5688800)		   /* 5.688800 MHz */
	MDRV_CPU_PROGRAM_MAP(vigilant_map)
	MDRV_CPU_IO_MAP(vigilant_io_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_CPU_ADD("soundcpu", Z80, 18432000/6)		   /* 3.072000 MHz */
	MDRV_CPU_PROGRAM_MAP(sound_map)
	MDRV_CPU_IO_MAP(buccanrs_sound_io_map)
	MDRV_CPU_VBLANK_INT_HACK(nmi_line_pulse,128)	/* clocked by V1 */
								/* IRQs are generated by main Z80 and YM2151 */
	MDRV_SOUND_START(m72)
	MDRV_SOUND_RESET(m72)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(55)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(16*8, (64-16)*8-1, 0*8, 32*8-1 )

	MDRV_GFXDECODE(buccanrs)
	MDRV_PALETTE_LENGTH(512+32)	/* 512 real palette, 32 virtual palette */

	MDRV_VIDEO_START(vigilant)
	MDRV_VIDEO_RESET(vigilant)
	MDRV_VIDEO_UPDATE(vigilant)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ym1", YM2203, 18432000/6)
	MDRV_SOUND_CONFIG(ym2203_config)
	MDRV_SOUND_ROUTE(0, "lspeaker",  0.35)
	MDRV_SOUND_ROUTE(0, "rspeaker", 0.35)
	MDRV_SOUND_ROUTE(1, "lspeaker",  0.35)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.35)
	MDRV_SOUND_ROUTE(2, "lspeaker",  0.35)
	MDRV_SOUND_ROUTE(2, "rspeaker", 0.35)
	MDRV_SOUND_ROUTE(3, "lspeaker",  0.50)
	MDRV_SOUND_ROUTE(3, "rspeaker", 0.50)

	MDRV_SOUND_ADD("ym2", YM2203, 18432000/6)
	MDRV_SOUND_ROUTE(0, "lspeaker",  0.35)
	MDRV_SOUND_ROUTE(0, "rspeaker", 0.35)
	MDRV_SOUND_ROUTE(1, "lspeaker",  0.35)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.35)
	MDRV_SOUND_ROUTE(2, "lspeaker",  0.35)
	MDRV_SOUND_ROUTE(2, "rspeaker", 0.35)
	MDRV_SOUND_ROUTE(3, "lspeaker",  0.50)
	MDRV_SOUND_ROUTE(3, "rspeaker", 0.50)

	MDRV_SOUND_ADD("dac", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.35)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.35)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( kikcubic )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, 3579645)		   /* 3.579645 MHz */
	MDRV_CPU_PROGRAM_MAP(kikcubic_map)
	MDRV_CPU_IO_MAP(kikcubic_io_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_CPU_ADD("soundcpu", Z80, 3579645)		   /* 3.579645 MHz */
	MDRV_CPU_PROGRAM_MAP(sound_map)
	MDRV_CPU_IO_MAP(sound_io_map)
	MDRV_CPU_VBLANK_INT_HACK(nmi_line_pulse,128)	/* clocked by V1 */
								/* IRQs are generated by main Z80 and YM2151 */
	MDRV_SOUND_START(m72)
	MDRV_SOUND_RESET(m72)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(55)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(8*8, (64-8)*8-1, 0*8, 32*8-1 )

	MDRV_GFXDECODE(kikcubic)
	MDRV_PALETTE_LENGTH(256)

	MDRV_VIDEO_START(vigilant)
	MDRV_VIDEO_RESET(vigilant)
	MDRV_VIDEO_UPDATE(kikcubic)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2151, 3579645)
	MDRV_SOUND_CONFIG(ym2151_config)
	MDRV_SOUND_ROUTE(0, "lspeaker", 0.55)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.55)

	MDRV_SOUND_ADD("dac", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
MACHINE_DRIVER_END



/***************************************************************************

  Game ROMs

***************************************************************************/

ROM_START( vigilant )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* 64k for code + 128k for bankswitching */
	ROM_LOAD( "g07_c03.bin",  0x00000, 0x08000, CRC(9dcca081) SHA1(6d086b70e6bf1fbafa746ef5c82334645f199be9) )
	ROM_LOAD( "j07_c04.bin",  0x10000, 0x10000, CRC(e0159105) SHA1(da6d74ec075863c67c0ce21b07a54029d138f688) )
	/* 0x20000-0x2ffff empty */

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* 64k for sound */
	ROM_LOAD( "g05_c02.bin",  0x00000, 0x10000, CRC(10582b2d) SHA1(6e7e5f07c49b347b427572efeb180c89f49bf2c7) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "f05_c08.bin",  0x00000, 0x10000, CRC(01579d20) SHA1(e58d8ca0ea0ac9d77225bf55faa499d1565924f9) )
	ROM_LOAD( "h05_c09.bin",  0x10000, 0x10000, CRC(4f5872f0) SHA1(6af21ba1c94097eecce30585983b4b07528c8635) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "n07_c12.bin",  0x00000, 0x10000, CRC(10af8eb2) SHA1(664b178b248babc43a9af0fe140fe57bc7367762) )
	ROM_LOAD( "k07_c10.bin",  0x10000, 0x10000, CRC(9576f304) SHA1(0ec2a7d3d82208e2a9a4ef9ab2824e6fe26ebbe5) )
	ROM_LOAD( "o07_c13.bin",  0x20000, 0x10000, CRC(b1d9d4dc) SHA1(1aacf6b0ff8d102880d3dce3b55cd1488edb90cf) )
	ROM_LOAD( "l07_c11.bin",  0x30000, 0x10000, CRC(4598be4a) SHA1(6b68ec94bdee0e58133a8d3891054ef44a8ff0e5) )
	ROM_LOAD( "t07_c16.bin",  0x40000, 0x10000, CRC(f5425e42) SHA1(c401263b6a266d3e9cd23133f1d823fb4b095e3d) )
	ROM_LOAD( "p07_c14.bin",  0x50000, 0x10000, CRC(cb50a17c) SHA1(eb15704f715b6475ae7096f8d82f1b20f8277c71) )
	ROM_LOAD( "v07_c17.bin",  0x60000, 0x10000, CRC(959ba3c7) SHA1(dcd2a885ae7b61210cbd55a38ccbe91c73d071b0) )
	ROM_LOAD( "s07_c15.bin",  0x70000, 0x10000, CRC(7f2e91c5) SHA1(27dcc9b696834897c36c0b7a1c6202d93f41ad8d) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "d01_c05.bin",  0x00000, 0x10000, CRC(81b1ee5c) SHA1(2014165ec71f089fecb5a3e60b939cc0f565d7f1) )
	ROM_LOAD( "e01_c06.bin",  0x10000, 0x10000, CRC(d0d33673) SHA1(39761d97a71deaf7f17233d5bd5a55dbb1e6b30e) )
	ROM_LOAD( "f01_c07.bin",  0x20000, 0x10000, CRC(aae81695) SHA1(ca8e136eca3543b27f3a61b105d4a280711cd6ea) )

	ROM_REGION( 0x10000, "samples", 0 ) /* samples */
	ROM_LOAD( "d04_c01.bin",  0x00000, 0x10000, CRC(9b85101d) SHA1(6b8a0f33b9b66bb968f7b61e49d19a6afad8db95) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "pal16l8.8r", 0x0000, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8.4m", 0x0200, 0x0104, CRC(6c628a26) SHA1(65a45e762ce218e4552c30302c1ac4272c099fb5) )
	ROM_LOAD( "pal16l8.1b", 0x0400, 0x0104, NO_DUMP ) /* PAL is read protected */
ROM_END

ROM_START( vigilantu )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* 64k for code + 128k for bankswitching */
	ROM_LOAD( "a-8h",  0x00000, 0x08000, CRC(8d15109e) SHA1(9ef57047a0b53cd0143a260193b33e3d5680ca71) )
	ROM_LOAD( "a-8l",  0x10000, 0x10000, CRC(7f95799b) SHA1(a371671c3c26976314aaac4e410bff0f13a8a085) )
	/* 0x20000-0x2ffff empty */

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* 64k for sound */
	ROM_LOAD( "g05_c02.bin",  0x00000, 0x10000, CRC(10582b2d) SHA1(6e7e5f07c49b347b427572efeb180c89f49bf2c7) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "f05_c08.bin",  0x00000, 0x10000, CRC(01579d20) SHA1(e58d8ca0ea0ac9d77225bf55faa499d1565924f9) )
	ROM_LOAD( "h05_c09.bin",  0x10000, 0x10000, CRC(4f5872f0) SHA1(6af21ba1c94097eecce30585983b4b07528c8635) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "n07_c12.bin",  0x00000, 0x10000, CRC(10af8eb2) SHA1(664b178b248babc43a9af0fe140fe57bc7367762) )
	ROM_LOAD( "k07_c10.bin",  0x10000, 0x10000, CRC(9576f304) SHA1(0ec2a7d3d82208e2a9a4ef9ab2824e6fe26ebbe5) )
	ROM_LOAD( "o07_c13.bin",  0x20000, 0x10000, CRC(b1d9d4dc) SHA1(1aacf6b0ff8d102880d3dce3b55cd1488edb90cf) )
	ROM_LOAD( "l07_c11.bin",  0x30000, 0x10000, CRC(4598be4a) SHA1(6b68ec94bdee0e58133a8d3891054ef44a8ff0e5) )
	ROM_LOAD( "t07_c16.bin",  0x40000, 0x10000, CRC(f5425e42) SHA1(c401263b6a266d3e9cd23133f1d823fb4b095e3d) )
	ROM_LOAD( "p07_c14.bin",  0x50000, 0x10000, CRC(cb50a17c) SHA1(eb15704f715b6475ae7096f8d82f1b20f8277c71) )
	ROM_LOAD( "v07_c17.bin",  0x60000, 0x10000, CRC(959ba3c7) SHA1(dcd2a885ae7b61210cbd55a38ccbe91c73d071b0) )
	ROM_LOAD( "s07_c15.bin",  0x70000, 0x10000, CRC(7f2e91c5) SHA1(27dcc9b696834897c36c0b7a1c6202d93f41ad8d) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "d01_c05.bin",  0x00000, 0x10000, CRC(81b1ee5c) SHA1(2014165ec71f089fecb5a3e60b939cc0f565d7f1) )
	ROM_LOAD( "e01_c06.bin",  0x10000, 0x10000, CRC(d0d33673) SHA1(39761d97a71deaf7f17233d5bd5a55dbb1e6b30e) )
	ROM_LOAD( "f01_c07.bin",  0x20000, 0x10000, CRC(aae81695) SHA1(ca8e136eca3543b27f3a61b105d4a280711cd6ea) )

	ROM_REGION( 0x10000, "samples", 0 ) /* samples */
	ROM_LOAD( "d04_c01.bin",  0x00000, 0x10000, CRC(9b85101d) SHA1(6b8a0f33b9b66bb968f7b61e49d19a6afad8db95) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "pal16l8.8r", 0x0000, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8.4m", 0x0200, 0x0104, CRC(6c628a26) SHA1(65a45e762ce218e4552c30302c1ac4272c099fb5) )
	ROM_LOAD( "pal16l8.1b", 0x0400, 0x0104, NO_DUMP ) /* PAL is read protected */
ROM_END

ROM_START( vigilantj )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* 64k for code + 128k for bankswitching */
	ROM_LOAD( "vg_a-8h.rom",  0x00000, 0x08000, CRC(ba848713) SHA1(b357cbf404fb1874d555797ed9fb37f946cc4340) )
	ROM_LOAD( "vg_a-8l.rom",  0x10000, 0x10000, CRC(3b12b1d8) SHA1(2f9207f8d8ec41ea1b8f5bf3c69a97d1d09f6c3f) )
	/* 0x20000-0x2ffff empty */

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* 64k for sound */
	ROM_LOAD( "g05_c02.bin",  0x00000, 0x10000, CRC(10582b2d) SHA1(6e7e5f07c49b347b427572efeb180c89f49bf2c7) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "f05_c08.bin",  0x00000, 0x10000, CRC(01579d20) SHA1(e58d8ca0ea0ac9d77225bf55faa499d1565924f9) )
	ROM_LOAD( "h05_c09.bin",  0x10000, 0x10000, CRC(4f5872f0) SHA1(6af21ba1c94097eecce30585983b4b07528c8635) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "n07_c12.bin",  0x00000, 0x10000, CRC(10af8eb2) SHA1(664b178b248babc43a9af0fe140fe57bc7367762) )
	ROM_LOAD( "k07_c10.bin",  0x10000, 0x10000, CRC(9576f304) SHA1(0ec2a7d3d82208e2a9a4ef9ab2824e6fe26ebbe5) )
	ROM_LOAD( "o07_c13.bin",  0x20000, 0x10000, CRC(b1d9d4dc) SHA1(1aacf6b0ff8d102880d3dce3b55cd1488edb90cf) )
	ROM_LOAD( "l07_c11.bin",  0x30000, 0x10000, CRC(4598be4a) SHA1(6b68ec94bdee0e58133a8d3891054ef44a8ff0e5) )
	ROM_LOAD( "t07_c16.bin",  0x40000, 0x10000, CRC(f5425e42) SHA1(c401263b6a266d3e9cd23133f1d823fb4b095e3d) )
	ROM_LOAD( "p07_c14.bin",  0x50000, 0x10000, CRC(cb50a17c) SHA1(eb15704f715b6475ae7096f8d82f1b20f8277c71) )
	ROM_LOAD( "v07_c17.bin",  0x60000, 0x10000, CRC(959ba3c7) SHA1(dcd2a885ae7b61210cbd55a38ccbe91c73d071b0) )
	ROM_LOAD( "s07_c15.bin",  0x70000, 0x10000, CRC(7f2e91c5) SHA1(27dcc9b696834897c36c0b7a1c6202d93f41ad8d) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "d01_c05.bin",  0x00000, 0x10000, CRC(81b1ee5c) SHA1(2014165ec71f089fecb5a3e60b939cc0f565d7f1) )
	ROM_LOAD( "e01_c06.bin",  0x10000, 0x10000, CRC(d0d33673) SHA1(39761d97a71deaf7f17233d5bd5a55dbb1e6b30e) )
	ROM_LOAD( "f01_c07.bin",  0x20000, 0x10000, CRC(aae81695) SHA1(ca8e136eca3543b27f3a61b105d4a280711cd6ea) )

	ROM_REGION( 0x10000, "samples", 0 ) /* samples */
	ROM_LOAD( "d04_c01.bin",  0x00000, 0x10000, CRC(9b85101d) SHA1(6b8a0f33b9b66bb968f7b61e49d19a6afad8db95) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "pal16l8.8r", 0x0000, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8.4m", 0x0200, 0x0104, CRC(6c628a26) SHA1(65a45e762ce218e4552c30302c1ac4272c099fb5) )
	ROM_LOAD( "pal16l8.1b", 0x0400, 0x0104, NO_DUMP ) /* PAL is read protected */
ROM_END

ROM_START( kikcubic )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* 64k for code + 128k for bankswitching */
	ROM_LOAD( "mqj-p0",       0x00000, 0x08000, CRC(9cef394a) SHA1(be9cc78420b4c35f8f9523b529bd56315749762c) )
	ROM_LOAD( "mqj-b0",       0x10000, 0x10000, CRC(d9bcf4cd) SHA1(f1f1cb8609343dae8637f115e5c96fd88a00f5eb) )
	ROM_LOAD( "mqj-b1",       0x20000, 0x10000, CRC(54a0abe1) SHA1(0fb1d050c1e299394609214c903bcf4cf11329ff) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* 64k for sound */
	ROM_LOAD( "mqj-sp",       0x00000, 0x10000, CRC(bbcf3582) SHA1(4a5b9d4161b26e3ca400573fa78268893e42d5db) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "mqj-c0",       0x00000, 0x10000, CRC(975585c5) SHA1(eb8245e458a5d4880add5b4a305a4468fa8f6491) )
	ROM_LOAD( "mqj-c1",       0x10000, 0x10000, CRC(49d9936d) SHA1(c4169ddd481c19e8e24457e2fe011db1b34db6d3) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "mqj-00",       0x00000, 0x40000, CRC(7fb0c58f) SHA1(f70ff39e2d648606686c87cf1a7a3ffb46c2656a) )
	ROM_LOAD( "mqj-10",       0x40000, 0x40000, CRC(3a189205) SHA1(063d664d4cf709931b5e3a5b6eb7c75bcd57b518) )

	ROM_REGION( 0x10000, "samples", 0 ) /* samples */
	ROM_LOAD( "mqj-v0",       0x00000, 0x10000, CRC(54762956) SHA1(f08e983af28b16d27505d465ca64e7c7a93373a4) )

	ROM_REGION( 0x0140, "proms", 0 )
	ROM_LOAD( "8d",           0x0000, 0x0100, CRC(7379bb12) SHA1(cf0c4e27911505f937004ea5eac1154956ec5d3b) )	/* unknown (timing?) */
	ROM_LOAD( "6h",           0x0100, 0x0020, CRC(face0cbb) SHA1(c56aea3b7aaabbd4ff1b4546fcad94f51b473cde) )	/* unknown (bad read?) */
	ROM_LOAD( "7s",           0x0120, 0x0020, CRC(face0cbb) SHA1(c56aea3b7aaabbd4ff1b4546fcad94f51b473cde) )	/* unknown (bad read?) */
ROM_END

ROM_START( kikcubicb )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* 64k for code + 128k for bankswitching */
	ROM_LOAD( "1.bin",        0x00000, 0x08000, CRC(d3a589ba) SHA1(be2fa4515ed3510fec2b182a3ffcf5ddb9d7256d) )
	ROM_LOAD( "4.bin",        0x10000, 0x10000, CRC(9ae1e1a6) SHA1(7f3099206300eaa275b003e829dff0b7b91d8cc8) )
	ROM_LOAD( "5.bin",        0x20000, 0x08000, CRC(a5a6bffd) SHA1(372452c8c9b2c65307434af19eddcb60e7cd0fa3) )
	ROM_RELOAD(				  0x28000, 0x08000 )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* 64k for sound */
	ROM_LOAD( "mqj-sp",       0x00000, 0x10000, CRC(bbcf3582) SHA1(4a5b9d4161b26e3ca400573fa78268893e42d5db) ) /* 2.bin */

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "7.bin",        0x00000, 0x10000, CRC(1788d51a) SHA1(bf7182379a34c366f192cb7d2494b26f6e27d97f) )
	ROM_LOAD( "mqj-c1",       0x10000, 0x10000, CRC(49d9936d) SHA1(c4169ddd481c19e8e24457e2fe011db1b34db6d3) ) /* 6.bin */

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "11.bin",       0x00000, 0x10000, CRC(0f0cac92) SHA1(32cf4b274b61d69a6d9f0ad39aa903c7a99b981d) )
	ROM_RELOAD(				  0x20000, 0x10000 )
	ROM_LOAD( "10.bin",       0x10000, 0x10000, CRC(7d3822a8) SHA1(20e07a6edd46abf46b0d101a0ccee72f087f63b2) )
	ROM_RELOAD(				  0x30000, 0x10000 )
	ROM_LOAD( "9.bin",        0x40000, 0x10000, CRC(56fb4fa3) SHA1(ed82602bfe98e60208d50f29f064c11cec01b3a7) )
	ROM_RELOAD(				  0x60000, 0x10000 )
	ROM_LOAD( "8.bin",        0x50000, 0x10000, CRC(947dbd4e) SHA1(278ad7126bacb752886800cf48c6fe704427149d) )
	ROM_RELOAD(				  0x70000, 0x10000 )

	ROM_REGION( 0x10000, "samples", 0 ) /* samples */
	ROM_LOAD( "mqj-v0",       0x00000, 0x10000, CRC(54762956) SHA1(f08e983af28b16d27505d465ca64e7c7a93373a4) )

	ROM_REGION( 0x0140, "proms", 0 )
	ROM_LOAD( "8d",           0x0000, 0x0100, CRC(7379bb12) SHA1(cf0c4e27911505f937004ea5eac1154956ec5d3b) )	/* unknown (timing?) */
	ROM_LOAD( "6h",           0x0100, 0x0020, CRC(face0cbb) SHA1(c56aea3b7aaabbd4ff1b4546fcad94f51b473cde) )	/* unknown (bad read?) */
	ROM_LOAD( "7s",           0x0120, 0x0020, CRC(face0cbb) SHA1(c56aea3b7aaabbd4ff1b4546fcad94f51b473cde) )	/* unknown (bad read?) */
ROM_END

ROM_START( buccanrs )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* 64k for code + 128k for bankswitching */
	ROM_LOAD( "11.u58",  0x00000, 0x10000, CRC(bf1d7e6f) SHA1(55dcf993515b57c3eb1fab98097a2171df3e38ed) ) // both halves are identical (correct for rom type on this board tho)
	ROM_LOAD( "12.u25",  0x10000, 0x10000, CRC(87303ba8) SHA1(49a25393e853b9adf7df00a6f9c38a526a02ea4e) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* 64k for sound */
	ROM_LOAD( "1.u128",  0x00000, 0x10000, CRC(eb65f8c3) SHA1(82566becb630ce92303905dc0c5bef9e80e9caad) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "7.u212",  0x00000, 0x10000, CRC(95e3c517) SHA1(9954830ebc3a6414a3236f4e41981db082e5ea19) )
	ROM_LOAD( "8.u189",  0x10000, 0x10000, CRC(fe2377ab) SHA1(8578c5466d98f140fdfc41e91cd841e725786e32) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "3.u100",  0x00000, 0x10000, CRC(16dc435f) SHA1(0c13e9786b356770c84f94684697e43d0ea9e7cc) )
	ROM_CONTINUE(        0x20000, 0x10000 )
	ROM_LOAD( "4.u80",   0x10000, 0x10000, CRC(4fe3bf97) SHA1(7910ace1eed80bfafa1f9f057ed67e23aa446a22) )
	ROM_LOAD( "6.u52",   0x40000, 0x10000, CRC(078aef7f) SHA1(72e60d39d8af8bd31e9ae019b12620797eb0af7f) )
	ROM_CONTINUE(        0x60000, 0x10000 )
	ROM_LOAD( "5.u70",   0x50000, 0x10000, CRC(f650fa90) SHA1(c87081b4d6b09f865d08c5120da3d0fb3196a2c3) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "9.u49",   0x20000, 0x20000, CRC(0c6188fb) SHA1(d49034384c6d0e94db2890223b32a2a49e79a639) )
	ROM_LOAD( "10.u27",  0x00000, 0x20000, CRC(2d383ff8) SHA1(3062baac27feba69c6ed94935c5ced72d89ed4fb) )

	ROM_REGION( 0x10000, "samples", 0 ) /* samples */
	ROM_LOAD( "2.u74",  0x00000, 0x10000, CRC(36ee1dac) SHA1(6dfd2a885c0b1c9347abc4b204ade66551c4b404) )

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "prom1.u54",  0x0000, 0x0100, CRC(c324835e) SHA1(cf6ffe38523badfda211d341410e93e647de87a9) )
	ROM_LOAD( "prom4.u79",  0x0100, 0x0100, CRC(e6506ef4) SHA1(079841da7640b14d94aaaeb572bf018932b58293) )
	ROM_LOAD( "prom3.u88",  0x0200, 0x0100, CRC(b43d094f) SHA1(2bed4892d8a91d7faac5a07bf858d9294eb30606) )
	ROM_LOAD( "prom2.u99",  0x0300, 0x0100, CRC(e0aa8869) SHA1(ac8bdfeba69420ba56ec561bf3d0f1229d02cea2) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "pal16l8.u103", 0x0000, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8.u156", 0x0200, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8.u42",  0x0400, 0x0104, NO_DUMP ) /* PAL is read protected */
ROM_END

ROM_START( buccanrsa )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* 64k for code + 128k for bankswitching */
	ROM_LOAD( "bc-011",  0x00000, 0x08000, CRC(6b657ef1) SHA1(a3356654d4b04177af23b39e924cc5ad64930bb6) )
	ROM_LOAD( "12.u25",  0x10000, 0x10000, CRC(87303ba8) SHA1(49a25393e853b9adf7df00a6f9c38a526a02ea4e) ) // not from this set, hopefully its only a data rom

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* 64k for sound */
	ROM_LOAD( "1.u128",  0x00000, 0x10000, CRC(eb65f8c3) SHA1(82566becb630ce92303905dc0c5bef9e80e9caad) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "7.u212",  0x00000, 0x10000, CRC(95e3c517) SHA1(9954830ebc3a6414a3236f4e41981db082e5ea19) )
	ROM_LOAD( "8.u189",  0x10000, 0x10000, CRC(fe2377ab) SHA1(8578c5466d98f140fdfc41e91cd841e725786e32) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "3.u100",  0x00000, 0x10000, CRC(16dc435f) SHA1(0c13e9786b356770c84f94684697e43d0ea9e7cc) )
	ROM_CONTINUE(        0x20000, 0x10000 )
	ROM_LOAD( "4.u80",   0x10000, 0x10000, CRC(4fe3bf97) SHA1(7910ace1eed80bfafa1f9f057ed67e23aa446a22) ) // was double size (2nd half blank) in this set)
	ROM_LOAD( "6.u52",   0x40000, 0x10000, CRC(078aef7f) SHA1(72e60d39d8af8bd31e9ae019b12620797eb0af7f) )
	ROM_CONTINUE(        0x60000, 0x10000 )
	ROM_LOAD( "5.u70",   0x50000, 0x10000, CRC(f650fa90) SHA1(c87081b4d6b09f865d08c5120da3d0fb3196a2c3) ) // was double size (2nd half blank) in this set)

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "9.u49",   0x20000, 0x20000, CRC(0c6188fb) SHA1(d49034384c6d0e94db2890223b32a2a49e79a639) )
	ROM_LOAD( "10.u27",  0x00000, 0x20000, CRC(2d383ff8) SHA1(3062baac27feba69c6ed94935c5ced72d89ed4fb) )

	ROM_REGION( 0x10000, "samples", 0 ) /* samples */
	ROM_LOAD( "2.u74",  0x00000, 0x10000, CRC(36ee1dac) SHA1(6dfd2a885c0b1c9347abc4b204ade66551c4b404) )

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "prom1.u54",  0x0000, 0x0100, CRC(c324835e) SHA1(cf6ffe38523badfda211d341410e93e647de87a9) )
	ROM_LOAD( "prom4.u79",  0x0100, 0x0100, CRC(e6506ef4) SHA1(079841da7640b14d94aaaeb572bf018932b58293) )
	ROM_LOAD( "prom3.u88",  0x0200, 0x0100, CRC(b43d094f) SHA1(2bed4892d8a91d7faac5a07bf858d9294eb30606) )
	ROM_LOAD( "prom2.u99",  0x0300, 0x0100, CRC(e0aa8869) SHA1(ac8bdfeba69420ba56ec561bf3d0f1229d02cea2) )
ROM_END

GAME( 1988, vigilant, 0,        vigilant, vigilant, 0, ROT0, "Irem", "Vigilante (World)", GAME_NO_COCKTAIL )
GAME( 1988, vigilantu,vigilant, vigilant, vigilant, 0, ROT0, "Irem (Data East USA license)", "Vigilante (US)", GAME_NO_COCKTAIL )
GAME( 1988, vigilantj,vigilant, vigilant, vigilant, 0, ROT0, "Irem", "Vigilante (Japan)", GAME_NO_COCKTAIL )
GAME( 1988, kikcubic, 0,        kikcubic, kikcubic, 0, ROT0, "Irem", "Meikyu Jima (Japan)", GAME_NO_COCKTAIL )	/* English title is Kickle Cubicle */
GAME( 1988, kikcubicb,kikcubic, kikcubic, kikcubic, 0, ROT0, "bootleg", "Kickle Cubele", GAME_NO_COCKTAIL )
GAME( 1989, buccanrs, 0,        buccanrs, buccanrs, 0, ROT0, "Duintronic", "Buccaneers (set 1)", GAME_NO_COCKTAIL )
GAME( 1989, buccanrsa,buccanrs, buccanrs, buccanra, 0, ROT0, "Duintronic", "Buccaneers (set 2)", GAME_NO_COCKTAIL )
