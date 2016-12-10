/***************************************************************************

Free Kick  - (c) 1987 Sega / Nihon System (made by Nihon, licensed to Sega)

Driver by Tomasz Slanina  dox@space.pl
based on initial work made by David Haywood

Notes:
- Quite interestingly, Free Kick's sound ROM contains a Z80 program, but
  there isn't a sound CPU and that program isn't executed. Instead, the main
  CPU reads the sound program through an 8255 PPI and plays sounds directly.
- Gigas: according to a photo of a genuine board:
    Sega Game ID#: 834-6167
    NEC MC-8123 with Sega security number 317-5002

TODO:
- Gigas cocktail mode / flipscreen

****************************************************************************

 ---

 $78d        - checksum calculations
 $d290,$d291 - level (color set , level number  - BCD)
 $d292       - lives

 To enter Test Mode - press Button1 durning RESET (code at $79d)

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/8255ppi.h"
#include "machine/mc8123.h"
#include "sound/sn76496.h"
#include "includes/freekick.h"


/*************************************
 *
 *  Machines' structure
 *
 *************************************/

static WRITE8_HANDLER( flipscreen_w )
{
	/* flip Y/X could be the other way round... */
	if (offset)
		flip_screen_y_set(space->machine, ~data & 1);
	else
		flip_screen_x_set(space->machine, ~data & 1);
}

static WRITE8_HANDLER( coin_w )
{
	coin_counter_w(space->machine, offset, ~data & 1);
}

static WRITE8_HANDLER( spinner_select_w )
{
	freekick_state *state = (freekick_state *)space->machine->driver_data;
	state->spinner = data & 1;
}

static READ8_HANDLER( spinner_r )
{
	freekick_state *state = (freekick_state *)space->machine->driver_data;
	return input_port_read(space->machine, state->spinner ? "IN3" : "IN2");
}

static WRITE8_HANDLER( pbillrd_bankswitch_w )
{
	memory_set_bank(space->machine, "bank1", data & 1);
}

static WRITE8_HANDLER( nmi_enable_w )
{
	freekick_state *state = (freekick_state *)space->machine->driver_data;
	state->nmi_en = data & 1;
}

static INTERRUPT_GEN( freekick_irqgen )
{
	freekick_state *state = (freekick_state *)device->machine->driver_data;
	if (state->nmi_en)
		cpu_set_input_line(device, INPUT_LINE_NMI, PULSE_LINE);
}

static WRITE8_HANDLER( oigas_5_w )
{
	freekick_state *state = (freekick_state *)space->machine->driver_data;
	if (data > 0xc0 && data < 0xe0)
		state->cnt = 1;

	switch (state->cnt)
	{
		case 1: state->inval = data << 8  ; break;
		case 2: state->inval |= data      ; break;
	}
}

static READ8_HANDLER( oigas_3_r )
{
	freekick_state *state = (freekick_state *)space->machine->driver_data;
	switch (++state->cnt)
	{
	case 2: return ~(state->inval >> 8);
	case 3: return ~(state->inval & 0xff);
	case 4:
		switch (state->inval)
		{
		case 0xc500: state->outval = 0x17ef; break;
		case 0xc520:
		case 0xc540: state->outval = 0x19c1; break;
		case 0xc560: state->outval = 0x1afc; break;
		case 0xc580:
		case 0xc5a0:
		case 0xc5c0: state->outval = 0x1f28; break;
		case 0xc680: state->outval = 0x2e8a; break;
		case 0xc5e0:
		case 0xc600:
		case 0xc620:
		case 0xc640:
		case 0xc660: state->outval = 0x25cc; break;
		case 0xc6c0:
		case 0xc6e0: state->outval = 0x09d7; break;
		case 0xc6a0: state->outval = 0x3168; break;
		case 0xc720: state->outval = 0x2207; break;
		case 0xc700: state->outval = 0x0e34; break;
		case 0xc710: state->outval = 0x0fdd; break;
		case 0xc4f0: state->outval = 0x05b6; break;
		case 0xc4e0: state->outval = 0xae1e; break;
		}
		return state->outval>>8;
	case 5: state->cnt=0;return state->outval&0xff;
	}
	return 0;
}

static READ8_HANDLER( oigas_2_r )
{
	return 1;
}

static READ8_HANDLER( freekick_ff_r )
{
	freekick_state *state = (freekick_state *)space->machine->driver_data;
	return state->ff_data;
}

static WRITE8_HANDLER( freekick_ff_w )
{
	freekick_state *state = (freekick_state *)space->machine->driver_data;
	state->ff_data = data;
}



/*************************************
 *
 *  Memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( pbillrd_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xcfff) AM_RAM
	AM_RANGE(0xd000, 0xd7ff) AM_RAM_WRITE(freek_videoram_w) AM_BASE_MEMBER(freekick_state, videoram)
	AM_RANGE(0xd800, 0xd8ff) AM_RAM AM_BASE_SIZE_MEMBER(freekick_state, spriteram, spriteram_size)
	AM_RANGE(0xd900, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xe000) AM_READ_PORT("IN0")
	AM_RANGE(0xe000, 0xe001) AM_WRITE(flipscreen_w)
	AM_RANGE(0xe002, 0xe003) AM_WRITE(coin_w)
	AM_RANGE(0xe004, 0xe004) AM_WRITE(nmi_enable_w)
	AM_RANGE(0xe800, 0xe800) AM_READ_PORT("IN1")
	AM_RANGE(0xf000, 0xf000) AM_READ_PORT("DSW1") AM_WRITE(pbillrd_bankswitch_w)
	AM_RANGE(0xf800, 0xf800) AM_READ_PORT("DSW2")
	AM_RANGE(0xfc00, 0xfc00) AM_DEVWRITE("sn1", sn76496_w)
	AM_RANGE(0xfc01, 0xfc01) AM_DEVWRITE("sn2", sn76496_w)
	AM_RANGE(0xfc02, 0xfc02) AM_DEVWRITE("sn3", sn76496_w)
	AM_RANGE(0xfc03, 0xfc03) AM_DEVWRITE("sn4", sn76496_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( freekickb_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xcfff) AM_ROM
	AM_RANGE(0xd000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xe7ff) AM_RAM_WRITE(freek_videoram_w) AM_BASE_MEMBER(freekick_state, videoram)	// tilemap
	AM_RANGE(0xe800, 0xe8ff) AM_RAM AM_BASE_SIZE_MEMBER(freekick_state, spriteram, spriteram_size)	// sprites
	AM_RANGE(0xec00, 0xec03) AM_DEVREADWRITE("ppi8255_0", ppi8255_r, ppi8255_w)
	AM_RANGE(0xf000, 0xf003) AM_DEVREADWRITE("ppi8255_1", ppi8255_r, ppi8255_w)
	AM_RANGE(0xf800, 0xf800) AM_READ_PORT("IN0") AM_WRITE(flipscreen_w)
	AM_RANGE(0xf801, 0xf801) AM_READ_PORT("IN1")
	AM_RANGE(0xf802, 0xf802) AM_READNOP	//MUST return bit 0 = 0, otherwise game resets
	AM_RANGE(0xf803, 0xf803) AM_READ(spinner_r)
	AM_RANGE(0xf802, 0xf803) AM_WRITE(coin_w)
	AM_RANGE(0xf804, 0xf804) AM_WRITE(nmi_enable_w)
	AM_RANGE(0xf806, 0xf806) AM_WRITE(spinner_select_w)
	AM_RANGE(0xfc00, 0xfc00) AM_DEVWRITE("sn1", sn76496_w)
	AM_RANGE(0xfc01, 0xfc01) AM_DEVWRITE("sn2", sn76496_w)
	AM_RANGE(0xfc02, 0xfc02) AM_DEVWRITE("sn3", sn76496_w)
	AM_RANGE(0xfc03, 0xfc03) AM_DEVWRITE("sn4", sn76496_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( gigas_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xcfff) AM_RAM
	AM_RANGE(0xd000, 0xd7ff) AM_RAM_WRITE(freek_videoram_w) AM_BASE_MEMBER(freekick_state, videoram)
	AM_RANGE(0xd800, 0xd8ff) AM_RAM AM_BASE_SIZE_MEMBER(freekick_state, spriteram, spriteram_size)
	AM_RANGE(0xd900, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xe000) AM_READ_PORT("IN0") AM_WRITENOP // probably not flipscreen
	AM_RANGE(0xe002, 0xe003) AM_WRITE(coin_w)
	AM_RANGE(0xe004, 0xe004) AM_WRITE(nmi_enable_w)
	AM_RANGE(0xe005, 0xe005) AM_WRITENOP
	AM_RANGE(0xe800, 0xe800) AM_READ_PORT("IN1")
	AM_RANGE(0xf000, 0xf000) AM_READ_PORT("DSW1") AM_WRITENOP //bankswitch ?
	AM_RANGE(0xf800, 0xf800) AM_READ_PORT("DSW2")
	AM_RANGE(0xfc00, 0xfc00) AM_DEVWRITE("sn1", sn76496_w)
	AM_RANGE(0xfc01, 0xfc01) AM_DEVWRITE("sn2", sn76496_w)
	AM_RANGE(0xfc02, 0xfc02) AM_DEVWRITE("sn3", sn76496_w)
	AM_RANGE(0xfc03, 0xfc03) AM_DEVWRITE("sn4", sn76496_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( gigas_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READWRITE(spinner_r, spinner_select_w)
	AM_RANGE(0x01, 0x01) AM_READNOP //unused dip 3
ADDRESS_MAP_END

static ADDRESS_MAP_START( oigas_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READWRITE(spinner_r, spinner_select_w)
	AM_RANGE(0x01, 0x01) AM_READNOP //unused dip 3
	AM_RANGE(0x02, 0x02) AM_READ(oigas_2_r)
	AM_RANGE(0x03, 0x03) AM_READ(oigas_3_r)
	AM_RANGE(0x05, 0x05) AM_WRITE(oigas_5_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( freekickb_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xff, 0xff) AM_READWRITE(freekick_ff_r, freekick_ff_w)
ADDRESS_MAP_END


/*************************************
 *
 *  Game-specific port definitions
 *
 *************************************/

static INPUT_PORTS_START( pbillrd )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Balls" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x06, 0x06, "Bonus Ball" )
	PORT_DIPSETTING(    0x06, "10000, 30000 & 50000 Points"  )
	PORT_DIPSETTING(    0x02, "20000 & 60000 Points" )
	PORT_DIPSETTING(    0x04, "30000 & 80000 Points" )
	PORT_DIPSETTING(    0x00, "Only 20000 Points" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, "Shot" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x02, "3 Coins/5 Credits" )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x20, "3 Coins/5 Credits" )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_5C ) )
INPUT_PORTS_END

static INPUT_PORTS_START( gigas )
	PORT_INCLUDE( pbillrd )

	PORT_START("IN2")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(30) PORT_KEYDELTA(15) PORT_REVERSE

	PORT_START("IN3")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(30) PORT_KEYDELTA(15) PORT_REVERSE PORT_COCKTAIL

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x06, "20000 & 60000, Every 60000 Points" )
	PORT_DIPSETTING(    0x02, "20000 & 60000 Points" )
	PORT_DIPSETTING(    0x04, "30000 & 80000, Every 80000 Points" )
	PORT_DIPSETTING(    0x00, "Only 20000 Points" )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Easy ) )    /* level 1 */
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) ) /* level 4 */
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
INPUT_PORTS_END

static INPUT_PORTS_START( gigasm2 )
	PORT_INCLUDE( gigas )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x06, "20000 & 60000, Every 60000 Points" )
	PORT_DIPSETTING(    0x02, "20000 & 60000 Points" )
	PORT_DIPSETTING(    0x04, "30000 & 90000, Every 90000 Points" )
	PORT_DIPSETTING(    0x00, "Only 20000 Points" )
INPUT_PORTS_END

static INPUT_PORTS_START( freekck )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x06, "2-3-4-5-60000 Points" )
	PORT_DIPSETTING(    0x02, "3-4-5-6-7-80000 Points" )
	PORT_DIPSETTING(    0x04, "20000 & 60000 Points" )
	PORT_DIPSETTING(    0x00, "ONLY 20000 Points" )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Easy ) )    /* level 1 */
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) ) /* level 4 */
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x02, "3 Coins/5 Credits" )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x20, "3 Coins/5 Credits" )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xc0, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x40, "1 Coin/25 Credits" )
	PORT_DIPSETTING(    0x80, "1 Coin/50 Credits" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x00, "Manufacturer" )
	PORT_DIPSETTING(    0x00, "Nihon System" )
	PORT_DIPSETTING(    0x01, "Sega/Nihon System" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Coin Slots" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x80, "2" )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("IN2")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(30) PORT_KEYDELTA(15) PORT_REVERSE

	PORT_START("IN3")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(30) PORT_KEYDELTA(15) PORT_REVERSE PORT_COCKTAIL
INPUT_PORTS_END

static INPUT_PORTS_START( countrun )
	PORT_INCLUDE( freekck )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x06, "20000, 60000 and every 60000 Points" )
	PORT_DIPSETTING(    0x02, "30000, 80000 and every 80000 Points" )
	PORT_DIPSETTING(    0x04, "20000 & 60000 Points" )
	PORT_DIPSETTING(    0x00, "ONLY 20000 Points" )

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END



/*************************************
 *
 *  Sound definitions
 *
 *************************************/

static WRITE8_DEVICE_HANDLER( snd_rom_addr_l_w )
{
	freekick_state *state = (freekick_state *)device->machine->driver_data;
	state->romaddr = (state->romaddr & 0xff00) | data;
}

static WRITE8_DEVICE_HANDLER( snd_rom_addr_h_w )
{
	freekick_state *state = (freekick_state *)device->machine->driver_data;
	state->romaddr = (state->romaddr & 0x00ff) | (data << 8);
}

static READ8_DEVICE_HANDLER( snd_rom_r )
{
	freekick_state *state = (freekick_state *)device->machine->driver_data;
	return memory_region(device->machine, "user1")[state->romaddr & 0x7fff];
}

static const ppi8255_interface ppi8255_intf[2] =
{
	{
		DEVCB_NULL,							/* Port A read */
		DEVCB_NULL,							/* Port B read */
		DEVCB_HANDLER(snd_rom_r),			/* Port C read */
		DEVCB_HANDLER(snd_rom_addr_l_w),	/* Port A write */
		DEVCB_HANDLER(snd_rom_addr_h_w),	/* Port B write */
		DEVCB_NULL							/* Port C write */
	},
	{
		DEVCB_INPUT_PORT("DSW1"),			/* Port A read */
		DEVCB_INPUT_PORT("DSW2"),			/* Port B read */
		DEVCB_INPUT_PORT("DSW3"),			/* Port C read */
		DEVCB_NULL,							/* Port A write */
		DEVCB_NULL,							/* Port B write */
		DEVCB_NULL							/* Port C write */
	}
};



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ 0,1,2,3, 4,5,6,7 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8 },
	8*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3),RGN_FRAC(2,3),RGN_FRAC(1,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
	128+0,128+1,128+2,128+3,128+4,128+5,128+6,128+7
	},
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
	  8*8, 9*8, 10*8, 11*8,12*8,13*8,14*8,15*8
	},
	16*16
};

static GFXDECODE_START( freekick )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0x000, 32 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 0x100, 32 )
GFXDECODE_END

/*************************************
 *
 *  Machine driver(s)
 *
 *************************************/

static MACHINE_START( freekick )
{
	freekick_state *state = (freekick_state *)machine->driver_data;

	state_save_register_global(machine, state->romaddr);
	state_save_register_global(machine, state->spinner);
	state_save_register_global(machine, state->nmi_en);
	state_save_register_global(machine, state->ff_data);
}

static MACHINE_RESET( freekick )
{
	freekick_state *state = (freekick_state *)machine->driver_data;

	state->romaddr = 0;
	state->spinner = 0;
	state->nmi_en = 0;
	state->ff_data = 0;
}

static MACHINE_START( pbillrd )
{
	memory_configure_bank(machine, "bank1", 0, 2, memory_region(machine, "maincpu") + 0x10000, 0x4000);

	MACHINE_START_CALL(freekick);
}

static MACHINE_START( oigas )
{
	freekick_state *state = (freekick_state *)machine->driver_data;

	state_save_register_global(machine, state->inval);
	state_save_register_global(machine, state->outval);
	state_save_register_global(machine, state->cnt);

	MACHINE_START_CALL(freekick);
}

static MACHINE_RESET( oigas )
{
	freekick_state *state = (freekick_state *)machine->driver_data;

	MACHINE_RESET_CALL(freekick);

	state->inval = 0;
	state->outval = 0;
	state->cnt = 0;
}

static MACHINE_DRIVER_START( base )
	MDRV_DRIVER_DATA(freekick_state)

	MDRV_CPU_ADD("maincpu",Z80, 18432000/6)	//confirmed
	MDRV_CPU_PROGRAM_MAP(pbillrd_map)
	MDRV_CPU_PERIODIC_INT(irq0_line_hold, 50*3) //??
	MDRV_CPU_VBLANK_INT("screen", freekick_irqgen)

	MDRV_GFXDECODE(freekick)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)

	MDRV_PALETTE_LENGTH(0x200)
	MDRV_PALETTE_INIT(RRRR_GGGG_BBBB)

	MDRV_VIDEO_START(freekick)
	MDRV_VIDEO_UPDATE(pbillrd)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("sn1", SN76496, 12000000/4)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD("sn2", SN76496, 12000000/4)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD("sn3", SN76496, 12000000/4)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD("sn4", SN76496, 12000000/4)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( pbillrd )
	MDRV_IMPORT_FROM(base)

	MDRV_MACHINE_START(pbillrd)
	MDRV_MACHINE_RESET(freekick)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( freekickb )
	MDRV_IMPORT_FROM(base)

	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(freekickb_map)
	MDRV_CPU_IO_MAP(freekickb_io_map)

	MDRV_MACHINE_START(freekick)
	MDRV_MACHINE_RESET(freekick)

	MDRV_PPI8255_ADD( "ppi8255_0", ppi8255_intf[0] )
	MDRV_PPI8255_ADD( "ppi8255_1", ppi8255_intf[1] )

	MDRV_VIDEO_UPDATE(freekick)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( gigas )
	MDRV_IMPORT_FROM(base)

	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(gigas_map)
	MDRV_CPU_IO_MAP(gigas_io_map)

	MDRV_MACHINE_START(freekick)
	MDRV_MACHINE_RESET(freekick)

	MDRV_VIDEO_UPDATE(gigas)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( oigas )
	MDRV_IMPORT_FROM(gigas)
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_IO_MAP(oigas_io_map)

	MDRV_MACHINE_START(oigas)
	MDRV_MACHINE_RESET(oigas)
MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( pbillrd )
	ROM_REGION( 0x18000, "maincpu", 0 ) /* Z80 Code */
	ROM_LOAD( "pb.18", 0x00000, 0x4000, CRC(9e6275ac) SHA1(482e845e7fb4190da483155bd908ad470373cd5c) )
	ROM_LOAD( "pb.7",  0x04000, 0x4000, CRC(dd438431) SHA1(07a950e38b3f627ecf95e5831e5480abb337a010) )
	ROM_CONTINUE(      0x10000, 0x4000 )
	ROM_LOAD( "pb.9",  0x14000, 0x4000, CRC(089ce80a) SHA1(779be9ba2277a26fbebf4acf9e2f5319a934b0f5) )

	ROM_REGION( 0xc000, "gfx1", 0 ) /* GFX */
	ROM_LOAD( "pb.4", 0x000000, 0x04000, CRC(2f4d4dd3) SHA1(ee4facabf591c235c270db4f4d3f612b8c474e57) )
	ROM_LOAD( "pb.5", 0x004000, 0x04000, CRC(9dfccbd3) SHA1(66ad8882f36630312b488d5d67ae554477574c31) )
	ROM_LOAD( "pb.6", 0x008000, 0x04000, CRC(b5c3f6f6) SHA1(586b47587619a766cf977b74978550aff41a58cc) )

	ROM_REGION( 0x6000, "gfx2", 0 ) /* GFX */
	ROM_LOAD( "10619.3r", 0x000000, 0x02000, CRC(3296b9d9) SHA1(51393306f74394de96c4097b6244e8eb36114dac) )
	ROM_LOAD( "10621.3m", 0x002000, 0x02000, CRC(3dca8e4b) SHA1(ca0416d8faba0bb5e6b8c0a8fc227b57caa75f71) )
	ROM_LOAD( "10620.3n", 0x004000, 0x02000, CRC(ee76b079) SHA1(99abe2c5b1889d20bc3f5720b168690e3979fb2f) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "82s129.3a", 0x0000, 0x0100, CRC(44802169) SHA1(f181d80185e0f87ee906d2b40e3a5deb6f563aa2) )
	ROM_LOAD( "82s129.4d", 0x0100, 0x0100, CRC(69ca07cc) SHA1(38ab08174633b53d70a38aacb40059a25cf12069) )
	ROM_LOAD( "82s129.4a", 0x0200, 0x0100, CRC(145f950a) SHA1(b007d0c1cc9545e0e241b39b79a48593d457f826) )
	ROM_LOAD( "82s129.3d", 0x0300, 0x0100, CRC(43d24e17) SHA1(de5c9391574781dcd8f244794010e8eddffa1c1e) )
	ROM_LOAD( "82s129.3b", 0x0400, 0x0100, CRC(7fdc872c) SHA1(98572560aa524490489d4202dba292a5af9f15e7) )
	ROM_LOAD( "82s129.3c", 0x0500, 0x0100, CRC(cc1657e5) SHA1(358f20dce376c2389009f9673ce38b297af863f6) )
ROM_END

ROM_START( pbillrds ) /* Encrytped with a Sega MC-8123 (317-0030) CPU module */
	ROM_REGION( 0x18000, "maincpu", 0 ) /* Z80 Code */
	ROM_LOAD( "10626.8n",  0x00000, 0x4000, CRC(51d725e6) SHA1(d7007c983530780e7fa3686cb7a6d7c382c802fa) ) /* encrypted */
	ROM_LOAD( "10625.8r",  0x04000, 0x4000, CRC(8977c724) SHA1(f00835a04dc6fa7d8c1e382dace515f2aa7d6f44) ) /* encrypted */
	ROM_CONTINUE(          0x10000, 0x4000 )
	ROM_LOAD( "10627.10n", 0x14000, 0x4000, CRC(2335e6dd) SHA1(82352b6f4abea88aad3a96ca63cccccb6e278f48) ) /* encrypted */

	ROM_REGION( 0x2000, "user1", 0 ) /* MC8123 key */
	ROM_LOAD( "317-0030.key", 0x0000, 0x2000, CRC(9223f06d) SHA1(51a22a4c80fe273526bde68918c13c6476cec383) )

	ROM_REGION( 0xc000, "gfx1", 0 ) /* GFX */
	ROM_LOAD( "10622.3h", 0x000000, 0x04000, CRC(23b864ac) SHA1(5a13ad6f2278761967269eed8c07077293c921d6) )
	ROM_LOAD( "10623.3h", 0x004000, 0x04000, CRC(3dbfb790) SHA1(81a2645b7b3addf8f5b83043c967647cea476002) )
	ROM_LOAD( "10624.3g", 0x008000, 0x04000, CRC(b80032a9) SHA1(20096bdae1aad8913d8d7b1045912ea5ae7fce6f) )

	ROM_REGION( 0x6000, "gfx2", 0 ) /* GFX */
	ROM_LOAD( "10619.3r", 0x000000, 0x02000, CRC(3296b9d9) SHA1(51393306f74394de96c4097b6244e8eb36114dac) )
	ROM_LOAD( "10621.3m", 0x002000, 0x02000, CRC(3dca8e4b) SHA1(ca0416d8faba0bb5e6b8c0a8fc227b57caa75f71) )
	ROM_LOAD( "10620.3n", 0x004000, 0x02000, CRC(ee76b079) SHA1(99abe2c5b1889d20bc3f5720b168690e3979fb2f) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "82s129.3a", 0x0000, 0x0100, CRC(44802169) SHA1(f181d80185e0f87ee906d2b40e3a5deb6f563aa2) )
	ROM_LOAD( "82s129.4d", 0x0100, 0x0100, CRC(69ca07cc) SHA1(38ab08174633b53d70a38aacb40059a25cf12069) )
	ROM_LOAD( "82s129.4a", 0x0200, 0x0100, CRC(145f950a) SHA1(b007d0c1cc9545e0e241b39b79a48593d457f826) )
	ROM_LOAD( "82s129.3d", 0x0300, 0x0100, CRC(43d24e17) SHA1(de5c9391574781dcd8f244794010e8eddffa1c1e) )
	ROM_LOAD( "82s129.3b", 0x0400, 0x0100, CRC(7fdc872c) SHA1(98572560aa524490489d4202dba292a5af9f15e7) )
	ROM_LOAD( "82s129.3c", 0x0500, 0x0100, CRC(cc1657e5) SHA1(358f20dce376c2389009f9673ce38b297af863f6) )
ROM_END

/*

original sets don't work, they're missing the main cpu code which is probably inside
the custom cpu, battery backed too so hopefully somebody can work out how to get it
from an original counter run board before they all die :-(

*/

ROM_START( freekick )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* Z80 Code */
	// Custom CPU (pack) No. NS6201-A 1987.9 FREE KICK (also found NS6201-A 1987.10 FREE KICK)
	ROM_LOAD( "freekick.cpu", 0x00000, 0x10000, NO_DUMP ) // missing, might be the same as the bootleg but not confirmed

	ROM_REGION( 0x08000, "user1", 0 ) /* sound data */
	ROM_LOAD( "11.1e", 0x00000, 0x08000, CRC(a6030ba9) SHA1(f363100f54a7a80701a6395c7539b8daa60db054) )

	ROM_REGION( 0xc000, "gfx1", 0 ) /* GFX */
	ROM_LOAD( "12.1h", 0x000000, 0x04000, CRC(fb82e486) SHA1(bc672272dc32b2aa64e991992172c44bea1ca65c) )
	ROM_LOAD( "13.1j", 0x004000, 0x04000, CRC(3ad78ee2) SHA1(033285d4ab7d6f46abf4c1bd4671c874738f0ac1) )
	ROM_LOAD( "14.1l", 0x008000, 0x04000, CRC(0185695f) SHA1(126994c69de157fc7c452ccc7f1a767f5085da27) )

	ROM_REGION( 0xc000, "gfx2", 0 ) /* GFX */
	ROM_LOAD( "15.1m", 0x000000, 0x04000, CRC(0fa7c13c) SHA1(24b0ca73b0e35474e2392d8e729bcd44b80f9135) )
	ROM_LOAD( "16.1p", 0x004000, 0x04000, CRC(2b996e89) SHA1(c6900449d27e89c3b444fb028694fdcda8e79322) )
	ROM_LOAD( "17.1r", 0x008000, 0x04000, CRC(e7894def) SHA1(5c97b7cce43d1e51c709603a0d2394b8119764bd) )

	ROM_REGION( 0x0600, "proms", 0 ) /* verified correct */
	ROM_LOAD( "24s10n.8j", 0x0000, 0x0100, CRC(53a6bc21) SHA1(d4beedc226004c1aa9b6aae29bee9c8a9b0fff7c) ) /* Or compaitble type prom like the 82S129 */
	ROM_LOAD( "24s10n.7j", 0x0100, 0x0100, CRC(38dd97d8) SHA1(468a0f87a704982dc1bce1ca21f9bb252ac241a0) )
	ROM_LOAD( "24s10n.8k", 0x0200, 0x0100, CRC(18e66087) SHA1(54857526179b738862d11ce87e9d0edcb7878488) )
	ROM_LOAD( "24s10n.7k", 0x0300, 0x0100, CRC(bc21797a) SHA1(4d6cf05e51b7ef9147eeff051c3728764021cfdb) )
	ROM_LOAD( "24s10n.8h", 0x0400, 0x0100, CRC(8aac5fd0) SHA1(07a179603c0167c1f998b2337d66be95db9911cc) )
	ROM_LOAD( "24s10n.7h", 0x0500, 0x0100, CRC(a507f941) SHA1(97619959ee4c366cb010525636ab5eefe5a3127a) )
ROM_END

ROM_START( freekickb )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* Z80 Code */
	ROM_LOAD( "freekbl8.q7", 0x00000, 0x10000, CRC(4208cfe5) SHA1(21628cbe8a217fbae30a6c24c9cc4c790fe45d65) ) // this was on the bootleg, would normally be battery backed inside cpu?

	ROM_REGION( 0x08000, "user1", 0 ) /* sound data */
	ROM_LOAD( "11.1e", 0x00000, 0x08000, CRC(a6030ba9) SHA1(f363100f54a7a80701a6395c7539b8daa60db054) ) // freekbl1.e2

	ROM_REGION( 0xc000, "gfx1", 0 ) /* GFX */
	ROM_LOAD( "12.1h", 0x000000, 0x04000, CRC(fb82e486) SHA1(bc672272dc32b2aa64e991992172c44bea1ca65c) ) // freekbl2.f2
	ROM_LOAD( "13.1j", 0x004000, 0x04000, CRC(3ad78ee2) SHA1(033285d4ab7d6f46abf4c1bd4671c874738f0ac1) ) // freekbl3.j2
	ROM_LOAD( "14.1l", 0x008000, 0x04000, CRC(0185695f) SHA1(126994c69de157fc7c452ccc7f1a767f5085da27) ) // freekbl4.k2

	ROM_REGION( 0xc000, "gfx2", 0 ) /* GFX */
	ROM_LOAD( "15.1m", 0x000000, 0x04000, CRC(0fa7c13c) SHA1(24b0ca73b0e35474e2392d8e729bcd44b80f9135) ) // freekbl5.m2
	ROM_LOAD( "16.1p", 0x004000, 0x04000, CRC(2b996e89) SHA1(c6900449d27e89c3b444fb028694fdcda8e79322) ) // freekbl6.n2
	ROM_LOAD( "17.1r", 0x008000, 0x04000, CRC(e7894def) SHA1(5c97b7cce43d1e51c709603a0d2394b8119764bd) ) // freekbl7.r2

	ROM_REGION( 0x0600, "proms", 0 ) /* verified correct */
	ROM_LOAD( "24s10n.8j", 0x0000, 0x0100, CRC(53a6bc21) SHA1(d4beedc226004c1aa9b6aae29bee9c8a9b0fff7c) ) /* Or compaitble type prom like the 82S129 */
	ROM_LOAD( "24s10n.7j", 0x0100, 0x0100, CRC(38dd97d8) SHA1(468a0f87a704982dc1bce1ca21f9bb252ac241a0) )
	ROM_LOAD( "24s10n.8k", 0x0200, 0x0100, CRC(18e66087) SHA1(54857526179b738862d11ce87e9d0edcb7878488) )
	ROM_LOAD( "24s10n.7k", 0x0300, 0x0100, CRC(bc21797a) SHA1(4d6cf05e51b7ef9147eeff051c3728764021cfdb) )
	ROM_LOAD( "24s10n.8h", 0x0400, 0x0100, CRC(8aac5fd0) SHA1(07a179603c0167c1f998b2337d66be95db9911cc) )
	ROM_LOAD( "24s10n.7h", 0x0500, 0x0100, CRC(a507f941) SHA1(97619959ee4c366cb010525636ab5eefe5a3127a) )
ROM_END

ROM_START( freekickb2 )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* Z80 Code */
	ROM_LOAD( "15.bin", 0x00000, 0x10000, CRC(6569f2b0) SHA1(9306e316e6ae659bae3759a12a4e445b555a8893) ) // this was on the bootleg, would normally be battery backed inside cpu?

	ROM_REGION( 0x08000, "user1", 0 ) /* sound data */
	ROM_LOAD( "1.bin", 0x00000, 0x08000, CRC(a6030ba9) SHA1(f363100f54a7a80701a6395c7539b8daa60db054) )

	/* the first half of the gfx roms isn't used on this bootleg (roms are double size)
       - the content is otherwise identical */
	ROM_REGION( 0xc000, "gfx1", 0 ) /* GFX */
	ROM_LOAD( "2.bin", 0x000000, 0x04000, CRC(96aeae91) SHA1(073ca6c9fbe14760ee10293791254da3bcb43940) )
	ROM_CONTINUE(0x0000,0x4000)
	ROM_LOAD( "3.bin", 0x004000, 0x04000, CRC(489f538c) SHA1(47e5b3db1bb9efe0e432be83220d9fc09695fdf1) )
	ROM_CONTINUE(0x4000,0x4000)
	ROM_LOAD( "4.bin", 0x008000, 0x04000, CRC(73cdb431) SHA1(97f62d8cb21dfd905713b40d54da270d9ff275b9) )
	ROM_CONTINUE(0x8000,0x4000)

	ROM_REGION( 0xc000, "gfx2", 0 ) /* GFX */
	ROM_LOAD( "5.bin", 0x000000, 0x04000, CRC(7def1c52) SHA1(dbe8c7b7c45ea681870b2039cb80728530a057c3) )
	ROM_CONTINUE(0x0000,0x4000)
	ROM_LOAD( "6.bin", 0x004000, 0x04000, CRC(59d1b3e7) SHA1(de82dcbaa0fa4f5b5f0cb0c925c12fac8828fa01) )
	ROM_CONTINUE(0x4000,0x4000)
	ROM_LOAD( "7.bin", 0x008000, 0x04000, CRC(95c19081) SHA1(d8abc6f8da9188ca7b322f17c16e0494d771df39) )
	ROM_CONTINUE(0x8000,0x4000)

	/* no proms in the dump, but almost certainly identical */
	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "24s10n.8j", 0x0000, 0x0100, CRC(53a6bc21) SHA1(d4beedc226004c1aa9b6aae29bee9c8a9b0fff7c) ) /* Or compaitble type prom like the 82S129 */
	ROM_LOAD( "24s10n.7j", 0x0100, 0x0100, CRC(38dd97d8) SHA1(468a0f87a704982dc1bce1ca21f9bb252ac241a0) )
	ROM_LOAD( "24s10n.8k", 0x0200, 0x0100, CRC(18e66087) SHA1(54857526179b738862d11ce87e9d0edcb7878488) )
	ROM_LOAD( "24s10n.7k", 0x0300, 0x0100, CRC(bc21797a) SHA1(4d6cf05e51b7ef9147eeff051c3728764021cfdb) )
	ROM_LOAD( "24s10n.8h", 0x0400, 0x0100, CRC(8aac5fd0) SHA1(07a179603c0167c1f998b2337d66be95db9911cc) )
	ROM_LOAD( "24s10n.7h", 0x0500, 0x0100, CRC(a507f941) SHA1(97619959ee4c366cb010525636ab5eefe5a3127a) )
ROM_END


ROM_START( countrun )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* Z80 Code */
	//  Custom CPU (pack) No. NS6201-A 1988.3 COUNTER RUN
	ROM_LOAD( "countrun.cpu", 0x00000, 0x10000, NO_DUMP ) // missing

	ROM_REGION( 0x08000, "user1", 0 ) /* sound data */
	ROM_LOAD( "c-run.e1", 0x00000, 0x08000, CRC(2c3b6f8f) SHA1(ee7d71e6d8bb7138d5d029a10a95471d387b5f29) ) // rom 1

	ROM_REGION( 0xc000, "gfx1", 0 ) /* GFX */
	ROM_LOAD( "c-run.h1", 0x000000, 0x04000, CRC(3385b7b5) SHA1(3f8f96f2a5406369dd56a9fe9f509ebee4a0179a) ) // rom 2
	ROM_LOAD( "c-run.j1", 0x004000, 0x04000, CRC(58dc148d) SHA1(3b2e5c6ced885d945f6c02fbab7c6d40db78c66a) ) // rom 3
	ROM_LOAD( "c-run.l1", 0x008000, 0x04000, CRC(3201f1e9) SHA1(72bd35600bf6e38741730f39bfd2a19f359bfb93) ) // rom 4

	ROM_REGION( 0xc000, "gfx2", 0 ) /* GFX */
	ROM_LOAD( "c-run.m1", 0x000000, 0x04000, CRC(1efab3b4) SHA1(7ce39cecf2809d3a7cbca5c6dffee738ba6f7b11) ) // rom 5
	ROM_LOAD( "c-run.p1", 0x004000, 0x04000, CRC(d0bf8d42) SHA1(b8d1bd155dba065475c84db768f14a3562fe21e0) ) // rom 6
	ROM_LOAD( "c-run.r1", 0x008000, 0x04000, CRC(4bb4a3e3) SHA1(179696464fce548ec333eec233025840fdb1eac2) ) // rom 7

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "24s10n.8j", 0x0000, 0x0100, CRC(63c114ad) SHA1(db881c4ff92cb04a94988587503346a44eb89b69) ) /* Or compaitble type prom like the 82S129 */
	ROM_LOAD( "24s10n.7j", 0x0100, 0x0100, CRC(d16f95cc) SHA1(041bb84576bd8492c1ad3e492d8cb3e04d316527) )
	ROM_LOAD( "24s10n.8k", 0x0200, 0x0100, CRC(217db2c1) SHA1(f2af1a74b0ce56290b1c119e1a9707287132194a) )
	ROM_LOAD( "24s10n.7k", 0x0300, 0x0100, CRC(8d983949) SHA1(d7331900d18a53ceb133f8a8848d3c108e03323a) )
	ROM_LOAD( "24s10n.8h", 0x0400, 0x0100, CRC(33e87550) SHA1(951ce0dc975b799c1056ce8eb005256cbb43a112) )
	ROM_LOAD( "24s10n.7h", 0x0500, 0x0100, CRC(c77d0077) SHA1(4cbbf625ad5e45d00ca6aebe9566538ff0a3348d) )
ROM_END

ROM_START( countrunb2 )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* Z80 Code */
	ROM_LOAD( "crunbl.8", 0x00000, 0x10000, CRC(318f95d9) SHA1(f2386b9d26d1bc98728aad9e257363b381043dc9) ) // encrypted? bad? its strange anyway

	ROM_REGION( 0x01000, "cpu1", 0 )
	ROM_LOAD( "68705.uc",  0x00000, 0x01000, NO_DUMP )

	ROM_REGION( 0x08000, "user1", 0 ) /* sound data */
	ROM_LOAD( "c-run.e1", 0x00000, 0x08000, CRC(2c3b6f8f) SHA1(ee7d71e6d8bb7138d5d029a10a95471d387b5f29) ) // rom 1

	ROM_REGION( 0xc000, "gfx1", 0 ) /* GFX */
	ROM_LOAD( "c-run.h1", 0x000000, 0x04000, CRC(3385b7b5) SHA1(3f8f96f2a5406369dd56a9fe9f509ebee4a0179a) ) // rom 2
	ROM_LOAD( "c-run.j1", 0x004000, 0x04000, CRC(58dc148d) SHA1(3b2e5c6ced885d945f6c02fbab7c6d40db78c66a) ) // rom 3
	ROM_LOAD( "c-run.l1", 0x008000, 0x04000, CRC(3201f1e9) SHA1(72bd35600bf6e38741730f39bfd2a19f359bfb93) ) // rom 4

	ROM_REGION( 0xc000, "gfx2", 0 ) /* GFX */
	ROM_LOAD( "c-run.m1", 0x000000, 0x04000, CRC(1efab3b4) SHA1(7ce39cecf2809d3a7cbca5c6dffee738ba6f7b11) ) // rom 5
	ROM_LOAD( "c-run.p1", 0x004000, 0x04000, CRC(d0bf8d42) SHA1(b8d1bd155dba065475c84db768f14a3562fe21e0) ) // rom 6
	ROM_LOAD( "c-run.r1", 0x008000, 0x04000, CRC(4bb4a3e3) SHA1(179696464fce548ec333eec233025840fdb1eac2) ) // rom 7

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "24s10n.8j", 0x0000, 0x0100, CRC(63c114ad) SHA1(db881c4ff92cb04a94988587503346a44eb89b69) ) /* Or compaitble type prom like the 82S129 */
	ROM_LOAD( "24s10n.7j", 0x0100, 0x0100, CRC(d16f95cc) SHA1(041bb84576bd8492c1ad3e492d8cb3e04d316527) )
	ROM_LOAD( "24s10n.8k", 0x0200, 0x0100, CRC(217db2c1) SHA1(f2af1a74b0ce56290b1c119e1a9707287132194a) )
	ROM_LOAD( "24s10n.7k", 0x0300, 0x0100, CRC(8d983949) SHA1(d7331900d18a53ceb133f8a8848d3c108e03323a) )
	ROM_LOAD( "24s10n.8h", 0x0400, 0x0100, CRC(33e87550) SHA1(951ce0dc975b799c1056ce8eb005256cbb43a112) )
	ROM_LOAD( "24s10n.7h", 0x0500, 0x0100, CRC(c77d0077) SHA1(4cbbf625ad5e45d00ca6aebe9566538ff0a3348d) )
ROM_END

ROM_START( countrunb )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* Z80 Code */
	ROM_LOAD( "rom_cpu.bin", 0x00000, 0x10000, CRC(f65639ae) SHA1(faa81607858d49559098c887ac847722df955a76) )

	ROM_REGION( 0x08000, "user1", 0 ) /* sound data */
	ROM_LOAD( "c-run.e1", 0x00000, 0x08000, CRC(2c3b6f8f) SHA1(ee7d71e6d8bb7138d5d029a10a95471d387b5f29) ) // rom 1

	ROM_REGION( 0xc000, "gfx1", 0 ) /* GFX */
	ROM_LOAD( "c-run.h1", 0x000000, 0x04000, CRC(3385b7b5) SHA1(3f8f96f2a5406369dd56a9fe9f509ebee4a0179a) ) // rom 2
	ROM_LOAD( "c-run.j1", 0x004000, 0x04000, CRC(58dc148d) SHA1(3b2e5c6ced885d945f6c02fbab7c6d40db78c66a) ) // rom 3
	ROM_LOAD( "c-run.l1", 0x008000, 0x04000, CRC(3201f1e9) SHA1(72bd35600bf6e38741730f39bfd2a19f359bfb93) ) // rom 4

	ROM_REGION( 0xc000, "gfx2", 0 ) /* GFX */
	ROM_LOAD( "c-run.m1", 0x000000, 0x04000, CRC(1efab3b4) SHA1(7ce39cecf2809d3a7cbca5c6dffee738ba6f7b11) ) // rom 5
	ROM_LOAD( "c-run.p1", 0x004000, 0x04000, CRC(d0bf8d42) SHA1(b8d1bd155dba065475c84db768f14a3562fe21e0) ) // rom 6
	ROM_LOAD( "c-run.r1", 0x008000, 0x04000, CRC(4bb4a3e3) SHA1(179696464fce548ec333eec233025840fdb1eac2) ) // rom 7

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "24s10n.8j", 0x0000, 0x0100, CRC(63c114ad) SHA1(db881c4ff92cb04a94988587503346a44eb89b69) ) /* Or compaitble type prom like the 82S129 */
	ROM_LOAD( "24s10n.7j", 0x0100, 0x0100, CRC(d16f95cc) SHA1(041bb84576bd8492c1ad3e492d8cb3e04d316527) )
	ROM_LOAD( "24s10n.8k", 0x0200, 0x0100, CRC(217db2c1) SHA1(f2af1a74b0ce56290b1c119e1a9707287132194a) )
	ROM_LOAD( "24s10n.7k", 0x0300, 0x0100, CRC(8d983949) SHA1(d7331900d18a53ceb133f8a8848d3c108e03323a) )
	ROM_LOAD( "24s10n.8h", 0x0400, 0x0100, CRC(33e87550) SHA1(951ce0dc975b799c1056ce8eb005256cbb43a112) )
	ROM_LOAD( "24s10n.7h", 0x0500, 0x0100, CRC(c77d0077) SHA1(4cbbf625ad5e45d00ca6aebe9566538ff0a3348d) )
ROM_END

ROM_START( gigasm2b )
	ROM_REGION( 2*0x10000, "maincpu", 0 )
	ROM_LOAD( "8.rom", 0x10000, 0x4000, CRC(c00a4a6c) SHA1(0d1bb849c9bfe4e92ad70e4ef19da494c0bd7ba8) )
	ROM_CONTINUE(      0x00000, 0x4000 )
	ROM_LOAD( "7.rom", 0x14000, 0x4000, CRC(92bd9045) SHA1(e4d8a94deeb795bb284ca0bd211ed40ed498b172) )
	ROM_CONTINUE(      0x04000, 0x4000 )
	ROM_LOAD( "9.rom", 0x18000, 0x4000, CRC(a3ef809c) SHA1(6d4098658aa124e10e5edb8e8e3abe0aa26741a1) )
	ROM_CONTINUE(      0x08000, 0x4000 )

	ROM_REGION( 0xc000, "gfx1", 0 ) /* GFX */
	ROM_LOAD( "4.rom", 0x00000, 0x04000, CRC(20b3405f) SHA1(32120d7b40e74648eb4ac4ab3ad3d2125033f6b1) )
	ROM_LOAD( "5.rom", 0x04000, 0x04000, CRC(d04ecfa8) SHA1(10bfb1d075da768f31a8c34cdfe1a1bf01e89f94) )
	ROM_LOAD( "6.rom", 0x08000, 0x04000, CRC(33776801) SHA1(29952818038a08c98b95ac801b8929cf1647049c) )

	ROM_REGION( 0xc000, "gfx2", 0 ) /* GFX */
	ROM_LOAD( "1.rom", 0x00000, 0x04000, CRC(f64cbd1e) SHA1(f8d9b110cdac6ef524e35bec9a5d406651cd7bab) )
	ROM_LOAD( "3.rom", 0x04000, 0x04000, CRC(c228df19) SHA1(584f269f7de2d531f2b038b4b7318f813c329f7f) )
	ROM_LOAD( "2.rom", 0x08000, 0x04000, CRC(a6ad9ce2) SHA1(db0338385208df9e9cf43efc11383412dec493e6) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "1.pr", 0x0000, 0x0100, CRC(a784e71f) SHA1(1741ce98d719bad6cc5ea42337ef897f2435bbab) )
	ROM_LOAD( "6.pr", 0x0100, 0x0100, CRC(376df30c) SHA1(cc95920cd1c133da1becc7d92f4b187b56a90ec7) )
	ROM_LOAD( "5.pr", 0x0200, 0x0100, CRC(4edff5bd) SHA1(305efc7ad7f86635489a655e214e216ac02b904d) )
	ROM_LOAD( "4.pr", 0x0300, 0x0100, CRC(fe201a4e) SHA1(15f8ecfcf6c63ffbf9777bec9b203c319ba1b96c) )
	ROM_LOAD( "2.pr", 0x0400, 0x0100, CRC(5796cc4a) SHA1(39576c4e48fd7ac52fc652a1ae0573db3d878878) )
	ROM_LOAD( "3.pr", 0x0500, 0x0100, CRC(28b5ee4c) SHA1(e21b9c38f433dca1e8894619b1d9f0389a81b48a) )
ROM_END



ROM_START( gigas ) /* From an actual Sega board 834-6167 with mc-8123: 317-5002 */
/* The MC-8123 is located on a small daughterboard which plugs into the z80 socket;
 * the daughterboard also has an input for the spinner control the game uses,
 * It is also possible that the 7 and 8 labels were switched on the files submitted;
 * the load order as it is now is definitely correct, and opposite of the bootlegs.
 * 7 is at location 8p, 8 at location 8n according to pcb pics.
 * An empty socket marked 27256 is at location 10n */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "8.8n",   0x00000, 0x4000, NO_DUMP ) /* 27128, dumped as all 0x00s, bad eprom */
	ROM_LOAD( "7.8p",   0x04000, 0x8000, CRC(43653909) SHA1(30f6666ba5c0f016299f462c4c07c81ee4832808) ) /* 27256 */

	ROM_REGION( 0x2000, "user1", 0 ) /* MC8123 key */
	ROM_LOAD( "317-5002.key", 0x0000, 0x2000, CRC(86a7e5f6) SHA1(3ff3a17c02eb5610182b6febfada4e8eca0c5eea) )

	ROM_REGION( 0xc000, "gfx1", 0 ) /* GFX */
	ROM_LOAD( "4.3k", 0x00000, 0x04000, CRC(8ed78981) SHA1(1f2c0584fcc6d04b042638c7b9a7e21fc560ca3d) )
	ROM_LOAD( "5.3h", 0x04000, 0x04000, CRC(0645ec2d) SHA1(ecf8b1ce98f845b5b32e7fc959cea7679a149d74) )
	ROM_LOAD( "6.3g", 0x08000, 0x04000, CRC(99e9cb27) SHA1(d141d6caa077e3cd182eb64cf803613ac17e7d09) )

	ROM_REGION( 0xc000, "gfx2", 0 ) /* GFX */
	ROM_LOAD( "1.3p", 0x00000, 0x04000, CRC(d78fae6e) SHA1(a7bf3b213f2a3a51b964959bd45003351670575a) )
	ROM_LOAD( "3.3l", 0x04000, 0x04000, CRC(37df4a4c) SHA1(ab996db636d89845474529ba2573307046fb96ee) )
	ROM_LOAD( "2.3n", 0x08000, 0x04000, CRC(3a46e354) SHA1(ebd6a5db4c9cdfc6fabe6b412a704aaf03c32d7c) )

	ROM_REGION( 0x0600, "proms", 0 ) /* not dumped yet; assumed to be the same */
	/* fill in which locations for each prom when redumping; the chips have
     * no labels, but are clearly located at 3a 3b 3c 3d 4a and 4d */
	ROM_LOAD( "1.pr", 0x0000, 0x0100, CRC(a784e71f) SHA1(1741ce98d719bad6cc5ea42337ef897f2435bbab) )
	ROM_LOAD( "6.pr", 0x0100, 0x0100, CRC(376df30c) SHA1(cc95920cd1c133da1becc7d92f4b187b56a90ec7) )
	ROM_LOAD( "5.pr", 0x0200, 0x0100, CRC(4edff5bd) SHA1(305efc7ad7f86635489a655e214e216ac02b904d) )
	ROM_LOAD( "4.pr", 0x0300, 0x0100, CRC(fe201a4e) SHA1(15f8ecfcf6c63ffbf9777bec9b203c319ba1b96c) )
	ROM_LOAD( "2.pr", 0x0400, 0x0100, CRC(5796cc4a) SHA1(39576c4e48fd7ac52fc652a1ae0573db3d878878) )
	ROM_LOAD( "3.pr", 0x0500, 0x0100, CRC(28b5ee4c) SHA1(e21b9c38f433dca1e8894619b1d9f0389a81b48a) )
ROM_END

/*
Gigas (bootleg)

CPU: Z80 (on a small plug in board), 8748 MCU (on same plug-in board)
SND: 76489 x3
DIPSW: 8 position x3
RAM: 6116 x2 (near ROMs 1-6), 6264 x1 (near ROMs 7-8), 2018 x2 (located in center of board)
XTAL: 18.432MHz
PROMs: 82s129 x6 (not dumped yet, probably match existing archives....)

Note: MCU dump has fixed bits, but read is good. If not correct, it's protected.
*/

ROM_START( gigasb )
	ROM_REGION( 2*0x10000, "maincpu", 0 )
	ROM_LOAD( "g-7",   0x10000, 0x4000, CRC(daf4e88d) SHA1(391dff914ce8e9b7975fc8827c066d7db16c4171) )
	ROM_CONTINUE(      0x00000, 0x4000 )
	ROM_LOAD( "g-8",   0x14000, 0x8000, CRC(4ab4c1f1) SHA1(63d8f489c7a8271e99a66d97e6eb0eb252cb2b67) )
	ROM_CONTINUE(      0x04000, 0x8000 )

	ROM_REGION( 0xc000, "gfx1", 0 ) /* GFX */
	ROM_LOAD( "g-4", 0x00000, 0x04000, CRC(8ed78981) SHA1(1f2c0584fcc6d04b042638c7b9a7e21fc560ca3d) )
	ROM_LOAD( "g-5", 0x04000, 0x04000, CRC(0645ec2d) SHA1(ecf8b1ce98f845b5b32e7fc959cea7679a149d74) )
	ROM_LOAD( "g-6", 0x08000, 0x04000, CRC(99e9cb27) SHA1(d141d6caa077e3cd182eb64cf803613ac17e7d09) )

	ROM_REGION( 0xc000, "gfx2", 0 ) /* GFX */
	ROM_LOAD( "g-1", 0x00000, 0x04000, CRC(d78fae6e) SHA1(a7bf3b213f2a3a51b964959bd45003351670575a) )
	ROM_LOAD( "g-3", 0x04000, 0x04000, CRC(37df4a4c) SHA1(ab996db636d89845474529ba2573307046fb96ee) )
	ROM_LOAD( "g-2", 0x08000, 0x04000, CRC(3a46e354) SHA1(ebd6a5db4c9cdfc6fabe6b412a704aaf03c32d7c) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "1.pr", 0x0000, 0x0100, CRC(a784e71f) SHA1(1741ce98d719bad6cc5ea42337ef897f2435bbab) )
	ROM_LOAD( "6.pr", 0x0100, 0x0100, CRC(376df30c) SHA1(cc95920cd1c133da1becc7d92f4b187b56a90ec7) )
	ROM_LOAD( "5.pr", 0x0200, 0x0100, CRC(4edff5bd) SHA1(305efc7ad7f86635489a655e214e216ac02b904d) )
	ROM_LOAD( "4.pr", 0x0300, 0x0100, CRC(fe201a4e) SHA1(15f8ecfcf6c63ffbf9777bec9b203c319ba1b96c) )
	ROM_LOAD( "2.pr", 0x0400, 0x0100, CRC(5796cc4a) SHA1(39576c4e48fd7ac52fc652a1ae0573db3d878878) )
	ROM_LOAD( "3.pr", 0x0500, 0x0100, CRC(28b5ee4c) SHA1(e21b9c38f433dca1e8894619b1d9f0389a81b48a) )
ROM_END

ROM_START( oigas )
	ROM_REGION( 2*0x10000, "maincpu", 0 )
	ROM_LOAD( "rom.7",   0x10000, 0x4000, CRC(e5bc04cc) SHA1(ffbd416313a9e49d2f9a7268d5ef48a8b641e480) )
	ROM_CONTINUE(        0x00000, 0x4000)
	ROM_LOAD( "rom.8",   0x04000, 0x8000, CRC(c199060d) SHA1(de8f1e0f941533abbbed25b595b1d51fadbb428d) )

	ROM_REGION( 0x0800, "cpu1", 0 )
	ROM_LOAD( "8748.bin", 0x0000, 0x0800, NO_DUMP )	/* missing */

	ROM_REGION( 0xc000, "gfx1", 0 ) /* GFX */
	ROM_LOAD( "g-4", 0x00000, 0x04000, CRC(8ed78981) SHA1(1f2c0584fcc6d04b042638c7b9a7e21fc560ca3d) )
	ROM_LOAD( "g-5", 0x04000, 0x04000, CRC(0645ec2d) SHA1(ecf8b1ce98f845b5b32e7fc959cea7679a149d74) )
	ROM_LOAD( "g-6", 0x08000, 0x04000, CRC(99e9cb27) SHA1(d141d6caa077e3cd182eb64cf803613ac17e7d09) )

	ROM_REGION( 0xc000, "gfx2", 0 ) /* GFX */
	ROM_LOAD( "g-1", 0x00000, 0x04000, CRC(d78fae6e) SHA1(a7bf3b213f2a3a51b964959bd45003351670575a) )
	ROM_LOAD( "g-3", 0x04000, 0x04000, CRC(37df4a4c) SHA1(ab996db636d89845474529ba2573307046fb96ee) )
	ROM_LOAD( "g-2", 0x08000, 0x04000, CRC(3a46e354) SHA1(ebd6a5db4c9cdfc6fabe6b412a704aaf03c32d7c) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "1.pr", 0x0000, 0x0100, CRC(a784e71f) SHA1(1741ce98d719bad6cc5ea42337ef897f2435bbab) )
	ROM_LOAD( "6.pr", 0x0100, 0x0100, CRC(376df30c) SHA1(cc95920cd1c133da1becc7d92f4b187b56a90ec7) )
	ROM_LOAD( "5.pr", 0x0200, 0x0100, CRC(4edff5bd) SHA1(305efc7ad7f86635489a655e214e216ac02b904d) )
	ROM_LOAD( "4.pr", 0x0300, 0x0100, CRC(fe201a4e) SHA1(15f8ecfcf6c63ffbf9777bec9b203c319ba1b96c) )
	ROM_LOAD( "2.pr", 0x0400, 0x0100, CRC(5796cc4a) SHA1(39576c4e48fd7ac52fc652a1ae0573db3d878878) )
	ROM_LOAD( "3.pr", 0x0500, 0x0100, CRC(28b5ee4c) SHA1(e21b9c38f433dca1e8894619b1d9f0389a81b48a) )
ROM_END



/*************************************
 *
 *  Game-specific driver inits
 *
 *************************************/

static DRIVER_INIT(gigasb)
{
	const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);
	memory_set_decrypted_region(space, 0x0000, 0xbfff, memory_region(machine, "maincpu") + 0x10000);
}


static DRIVER_INIT( pbillrds )
{
	mc8123_decrypt_rom(machine, "maincpu", "user1", "bank1", 2);
}

static DRIVER_INIT( gigas )
{
	mc8123_decrypt_rom(machine, "maincpu", "user1", NULL, 1);
}




/*************************************
 *
 *  Game driver(s)
 *
 *************************************/
/*    YEAR  NAME       PARENT    MACHINE    INPUT     INIT      ROT     COMPANY   FULLNAME        FLAGS  */
GAME( 1986, gigas,     0,        gigas,     gigas,    gigas,    ROT270, "Sega", "Gigas (MC-8123, 317-5002)", GAME_NOT_WORKING | GAME_SUPPORTS_SAVE )
GAME( 1986, gigasb,    gigas,    gigas,     gigas,    gigasb,   ROT270, "bootleg", "Gigas (bootleg)", GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
GAME( 1986, oigas,     gigas ,   oigas,     gigas,    gigasb,   ROT270, "bootleg", "Oigas (bootleg)", GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
GAME( 1986, gigasm2b,  0,        gigas,     gigasm2,  gigasb,   ROT270, "bootleg", "Gigas Mark II (bootleg)", GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
GAME( 1987, pbillrd,   0,        pbillrd,   pbillrd,  0,        ROT0,   "Nihon System", "Perfect Billiard", GAME_SUPPORTS_SAVE )
GAME( 1987, pbillrds,  pbillrd,  pbillrd,   pbillrd,  pbillrds, ROT0,   "Nihon System", "Perfect Billiard (MC-8123, 317-0030)", GAME_SUPPORTS_SAVE )
GAME( 1987, freekick,  0,        freekickb, freekck,  0,        ROT270, "Nihon System (Sega license)", "Free Kick", GAME_NOT_WORKING | GAME_SUPPORTS_SAVE )
GAME( 1987, freekickb, freekick, freekickb, freekck,  0,        ROT270, "bootleg", "Free Kick (bootleg set 1)", GAME_SUPPORTS_SAVE )
GAME( 1987, freekickb2,freekick, freekickb, freekck,  0,        ROT270, "bootleg", "Free Kick (bootleg set 2)", GAME_SUPPORTS_SAVE )
GAME( 1988, countrun,  0,        freekickb, countrun, 0,        ROT0,   "Nihon System (Sega license)", "Counter Run", GAME_NOT_WORKING | GAME_SUPPORTS_SAVE )
GAME( 1988, countrunb, countrun, freekickb, countrun, 0,        ROT0,   "bootleg", "Counter Run (bootleg set 1)", GAME_SUPPORTS_SAVE )
GAME( 1988, countrunb2,countrun, freekickb, countrun, 0,        ROT0,   "bootleg", "Counter Run (bootleg set 2)", GAME_NOT_WORKING | GAME_SUPPORTS_SAVE )

