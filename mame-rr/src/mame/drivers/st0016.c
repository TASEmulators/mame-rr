/*******************************************
  Seta custom ST-0016 chip based games.
    driver by Tomasz Slanina
********************************************

Todo:
- find NMI source, and NMI enable/disable (timer ? video hw ?)

Dips verified for Neratte Chu (nratechu) from manual
*/

#include "emu.h"
#include "cpu/v810/v810.h"
#include "deprecat.h"
#include "cpu/z80/z80.h"
#include "sound/st0016.h"
#include "includes/st0016.h"



static int mux_port;
UINT32 st0016_rom_bank;

/*************************************
 *
 *  Machine's structure ST0016
 *
 *************************************/

static ADDRESS_MAP_START( st0016_mem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xcfff) AM_READ(st0016_sprite_ram_r) AM_WRITE(st0016_sprite_ram_w)
	AM_RANGE(0xd000, 0xdfff) AM_READ(st0016_sprite2_ram_r) AM_WRITE(st0016_sprite2_ram_w)
	AM_RANGE(0xe000, 0xe7ff) AM_RAM
	AM_RANGE(0xe800, 0xe87f) AM_RAM /* common ram */
	AM_RANGE(0xe900, 0xe9ff) AM_DEVREADWRITE("stsnd", st0016_snd_r, st0016_snd_w) /* sound regs 8 x $20 bytes, see notes */
	AM_RANGE(0xea00, 0xebff) AM_READ(st0016_palette_ram_r) AM_WRITE(st0016_palette_ram_w)
	AM_RANGE(0xec00, 0xec1f) AM_READ(st0016_character_ram_r) AM_WRITE(st0016_character_ram_w)
	AM_RANGE(0xf000, 0xffff) AM_RAM /* work ram */
ADDRESS_MAP_END

static READ8_HANDLER(mux_r)
{
/*
    76543210
        xxxx - input port #2
    xxxx     - dip switches (2x8 bits) (multiplexed)
*/
	int retval = input_port_read(space->machine, "SYSTEM") & 0x0f;
	switch(mux_port & 0x30)
	{
		case 0x00: retval |= ((input_port_read(space->machine, "DSW1") & 1) << 4) | ((input_port_read(space->machine, "DSW1") & 0x10) << 1)
								| ((input_port_read(space->machine, "DSW2") & 1) << 6) | ((input_port_read(space->machine, "DSW2") & 0x10) <<3); break;
		case 0x10: retval |= ((input_port_read(space->machine, "DSW1") & 2) << 3) | ((input_port_read(space->machine, "DSW1") & 0x20)   )
								| ((input_port_read(space->machine, "DSW2") & 2) << 5) | ((input_port_read(space->machine, "DSW2") & 0x20) <<2); break;
		case 0x20: retval |= ((input_port_read(space->machine, "DSW1") & 4) << 2) | ((input_port_read(space->machine, "DSW1") & 0x40) >> 1)
								| ((input_port_read(space->machine, "DSW2") & 4) << 4) | ((input_port_read(space->machine, "DSW2") & 0x40) <<1); break;
		case 0x30: retval |= ((input_port_read(space->machine, "DSW1") & 8) << 1) | ((input_port_read(space->machine, "DSW1") & 0x80) >> 2)
								| ((input_port_read(space->machine, "DSW2") & 8) << 3) | ((input_port_read(space->machine, "DSW2") & 0x80)    ); break;
	}
	return retval;
}

static WRITE8_HANDLER(mux_select_w)
{
	mux_port=data;
}

WRITE8_HANDLER(st0016_rom_bank_w)
{
	memory_set_bankptr(space->machine,  "bank1", memory_region(space->machine, "maincpu") + (data* 0x4000) + 0x10000 );
	st0016_rom_bank=data;
}

READ8_HANDLER(st0016_dma_r);

static ADDRESS_MAP_START( st0016_io, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0xbf) AM_READ(st0016_vregs_r) AM_WRITE(st0016_vregs_w) /* video/crt regs ? */
	AM_RANGE(0xc0, 0xc0) AM_READ_PORT("P1") AM_WRITE(mux_select_w)
	AM_RANGE(0xc1, 0xc1) AM_READ_PORT("P2") AM_WRITENOP
	AM_RANGE(0xc2, 0xc2) AM_READ(mux_r) AM_WRITENOP
	AM_RANGE(0xc3, 0xc3) AM_READ_PORT("P2") AM_WRITENOP
	AM_RANGE(0xe0, 0xe0) AM_WRITENOP /* renju = $40, neratte = 0 */
	AM_RANGE(0xe1, 0xe1) AM_WRITE(st0016_rom_bank_w)
	AM_RANGE(0xe2, 0xe2) AM_WRITE(st0016_sprite_bank_w)
	AM_RANGE(0xe3, 0xe4) AM_WRITE(st0016_character_bank_w)
	AM_RANGE(0xe5, 0xe5) AM_WRITE(st0016_palette_bank_w)
	AM_RANGE(0xe6, 0xe6) AM_WRITENOP /* banking ? ram bank ? shared rambank ? */
	AM_RANGE(0xe7, 0xe7) AM_WRITENOP /* watchdog */
	AM_RANGE(0xf0, 0xf0) AM_READ(st0016_dma_r)
ADDRESS_MAP_END


/*************************************
 *
 *  Machine's structure ST0016 + V810
 *
 *************************************/

static UINT32 latches[8];

static READ32_HANDLER(latch32_r)
{
	if(!offset)
		latches[2]&=~2;
	return latches[offset];
}

static WRITE32_HANDLER(latch32_w)
{
	if(!offset)
		latches[2]|=1;
	COMBINE_DATA(&latches[offset]);
	timer_call_after_resynch(space->machine, NULL, 0, NULL);
}

static READ8_HANDLER(latch8_r)
{
	if(!offset)
		latches[2]&=~1;
	return latches[offset];
}

static WRITE8_HANDLER(latch8_w)
{
	if(!offset)
		latches[2]|=2;
	latches[offset]=data;
	timer_call_after_resynch(space->machine, NULL, 0, NULL);
}

static ADDRESS_MAP_START( v810_mem,ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x0001ffff) AM_RAM
	AM_RANGE(0x80000000, 0x8001ffff) AM_RAM
	AM_RANGE(0xc0000000, 0xc001ffff) AM_RAM
	AM_RANGE(0x40000000, 0x4000000f) AM_READ(latch32_r) AM_WRITE(latch32_w)
	AM_RANGE(0xfff80000, 0xffffffff) AM_ROMBANK("bank2")
ADDRESS_MAP_END

static ADDRESS_MAP_START( st0016_m2_io, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0xbf) AM_READ(st0016_vregs_r) AM_WRITE(st0016_vregs_w)
	AM_RANGE(0xc0, 0xc3) AM_READ(latch8_r) AM_WRITE(latch8_w)
	AM_RANGE(0xd0, 0xd0) AM_READ_PORT("P1") AM_WRITE(mux_select_w)
	AM_RANGE(0xd1, 0xd1) AM_READ_PORT("P2") AM_WRITENOP
	AM_RANGE(0xd2, 0xd2) AM_READ(mux_r) AM_WRITENOP
	AM_RANGE(0xd3, 0xd3) AM_READ_PORT("P2") AM_WRITENOP
	AM_RANGE(0xe0, 0xe0) AM_WRITENOP
	AM_RANGE(0xe1, 0xe1) AM_WRITE(st0016_rom_bank_w)
	AM_RANGE(0xe2, 0xe2) AM_WRITE(st0016_sprite_bank_w)
	AM_RANGE(0xe3, 0xe4) AM_WRITE(st0016_character_bank_w)
	AM_RANGE(0xe5, 0xe5) AM_WRITE(st0016_palette_bank_w)
	AM_RANGE(0xe6, 0xe6) AM_WRITENOP /* banking ? ram bank ? shared rambank ? */
	AM_RANGE(0xe7, 0xe7) AM_WRITENOP /* watchdog */
	AM_RANGE(0xf0, 0xf0) AM_READ(st0016_dma_r)
ADDRESS_MAP_END

/*************************************
 *
 *  Generic port definitions
 *
 *************************************/
static INPUT_PORTS_START( st0016 )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW)

	PORT_START("UNK")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* unused ? */

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(	0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(	0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(	0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(	0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(	0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(	0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(	0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(	0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(	0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(	0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(	0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(	0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(	0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(	0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(	0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(	0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
INPUT_PORTS_END

/*************************************
 *
 *  Game-specific port definitions
 *
 *************************************/

static INPUT_PORTS_START( renju )
	PORT_INCLUDE( st0016 )

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("DSW1")	/* Dip switch A  */
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(	0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x04, IP_ACTIVE_LOW ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(	0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(	0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(	0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(	0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(	0x10, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(	0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(	0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(	0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(	0x80, DEF_STR( 1C_2C ) )

	PORT_MODIFY("DSW2")	/* Dip switch B */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(	0x00, DEF_STR( Very_Hard ) )
	PORT_DIPSETTING(	0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(	0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(	0x03, DEF_STR( Normal ) )
INPUT_PORTS_END

static INPUT_PORTS_START( koikois )
	PORT_INCLUDE( st0016 )

	PORT_MODIFY("P1")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("DSW1")	/* Dip switch A  */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(	0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Crt Mode" ) PORT_DIPLOCATION("SW1:2") // flip screen ?
	PORT_DIPSETTING(	0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x04, IP_ACTIVE_LOW ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(	0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(	0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(	0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(	0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(	0x20, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x40, 0x40,  DEF_STR( Controls ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(	0x00, "Majyan Panel" )
	PORT_DIPSETTING(	0x40, DEF_STR( Joystick ) )

	PORT_MODIFY("DSW2")	/* Dip switch B */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(	0x00, DEF_STR( Very_Hard ) )
	PORT_DIPSETTING(	0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(	0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(	0x03, DEF_STR( Normal ) )
INPUT_PORTS_END

static INPUT_PORTS_START( nratechu )
	PORT_INCLUDE( st0016 )

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("DSW1")	/* Dip switch A  */
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(	0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(	0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(	0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(	0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(	0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(	0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(	0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(	0x04, DEF_STR( 1C_4C ) )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW1:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW1:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW1:6" )
	PORT_DIPNAME( 0x40, 0x40, "How To Play" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(	0x00, DEF_STR( No ) )
	PORT_DIPSETTING(	0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Language ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(	0x00, DEF_STR( English ) )
	PORT_DIPSETTING(	0x80, DEF_STR( Japanese ) )

	PORT_MODIFY("DSW2")	/* Dip switch B */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2") //  speed / time..
	PORT_DIPSETTING(	0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(	0x03, DEF_STR( Normal ))
	PORT_DIPSETTING(	0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0C, 0x0c, "VS Round" ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(	0x00, "First one to win" )
	PORT_DIPSETTING(	0x04, "Best 4 out of 7" )
	PORT_DIPSETTING(	0x08, "Best 3 out of 5" )
	PORT_DIPSETTING(	0x0C, "Best 2 out of 3" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:6") // Manual has this Defaulted OFF
	PORT_DIPSETTING(	0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(	0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW ) PORT_DIPLOCATION("SW2:8")
INPUT_PORTS_END

static INPUT_PORTS_START( mayjisn2 )
	PORT_INCLUDE( st0016 )

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("DSW1")	/* Dip switch A  */
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(	0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(	0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(	0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(	0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(	0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(	0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(	0x04, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x18, 0x18, "Timer" ) PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(	0x00, "6:00" )
	PORT_DIPSETTING(	0x08, "5:00" )
	PORT_DIPSETTING(	0x18, "4:00" )
	PORT_DIPSETTING(	0x10, "3:00" )

	PORT_MODIFY("DSW2")	/* Dip switch B */
	PORT_DIPNAME( 0x18, 0x18, "Music in Game"  ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(	0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x08, "Remixed" )
	PORT_DIPSETTING(	0x18, "Only Intro" )
	PORT_DIPSETTING(	0x10, "Classic" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(	0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Position of Title" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(	0x00, "B" )
	PORT_DIPSETTING(	0x40, "A" )
INPUT_PORTS_END

static GFXDECODE_START( st0016 )
//  GFXDECODE_ENTRY( NULL, 0, charlayout,      0, 16*4  )
GFXDECODE_END

static INTERRUPT_GEN(st0016_int)
{
	if(!cpu_getiloops(device))
		cpu_set_input_line(device,0,HOLD_LINE);
	else
		if(cpu_get_reg(device, Z80_IFF1)) /* dirty hack ... */
			cpu_set_input_line(device, INPUT_LINE_NMI, PULSE_LINE );
}

static const st0016_interface st0016_config =
{
	&st0016_charram
};

/*************************************
 *
 *  Machine driver(s)
 *
 *************************************/

static MACHINE_DRIVER_START( st0016 )
	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu",Z80,8000000) /* 8 MHz ? */
	MDRV_CPU_PROGRAM_MAP(st0016_mem)
	MDRV_CPU_IO_MAP(st0016_io)

	MDRV_CPU_VBLANK_INT_HACK(st0016_int,5) /*  4*nmi + int0 */

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(48*8, 48*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 48*8-1, 0*8, 48*8-1)

	MDRV_GFXDECODE(st0016)
	MDRV_PALETTE_LENGTH(16*16*4+1)

	MDRV_VIDEO_START(st0016)
	MDRV_VIDEO_UPDATE(st0016)

	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("stsnd", ST0016, 0)
	MDRV_SOUND_CONFIG(st0016_config)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( mayjinsn )
	MDRV_IMPORT_FROM(st0016)
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_IO_MAP(st0016_m2_io)
	MDRV_CPU_ADD("sub", V810, 10000000)//25 Mhz ?
	MDRV_CPU_PROGRAM_MAP(v810_mem)
	MDRV_QUANTUM_TIME(HZ(60))
MACHINE_DRIVER_END

/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/
/*
Renjyu Kizoku
Visco, 1994

PCB Layout

E51-00001-A
|------------------------------------|
|AMP       UPD6376    424400  62256  |
|    VOL      RESET   424400  62256  |
|                                    |
| TD62064 74273                      |
|J        74273                      |
|A        74273            42.9545MHz|
|M                                   |
|M        74245         UNKNOWN      |
|A        74245          QFP208      |
|         74245                      |
|         74245                 48MHz|
|                                    |
|  74138  74138                      |
|  74253  74253       RNJ2           |
|                                    |
|  DSW1   DSW2        RENJYU-1  6264 |
|------------------------------------|

Note:
      Unknown QFP (surface scratched off) is the CPU
        + GFX generator - it's possibly NEC V70/V810
*/

ROM_START( renju )
	ROM_REGION( 0x290000, "maincpu", 0 )
	ROM_LOAD( "renjyu-1.u31",0x010000, 0x200000, CRC(e0fdbe9b) SHA1(52d31024d1a88b8fcca1f87366fcaf80e3c387a1) )
	ROM_LOAD( "rnj2.u32",    0x210000, 0x080000, CRC(2015289c) SHA1(5223b6d3dbe4657cd63cf5b527eaab84cf23587a ) )
	ROM_COPY( "maincpu",   0x210000, 0x000000, 0x08000 )
ROM_END

ROM_START( nratechu )
	ROM_REGION( 0x210000, "maincpu", 0 )
	ROM_LOAD( "sx012-01",   0x10000, 0x80000,   CRC(6ca01d57) SHA1(065848f19ecf2dc1f7bbc7ddd87bca502e4b8b16) )
	ROM_LOAD( "sx012-02",   0x110000, 0x100000, CRC(40a4e354) SHA1(8120ce8deee6805050a5b083a334c3743c09566b) )
	ROM_COPY( "maincpu",  0x10000, 0x00000, 0x08000 )
ROM_END

/*
Koi Koi Shimasho
Visco

PCB Layout
----------

E63-00001
|---------------------------------------|
|VOL     RESET     TC514400   62256     |
|        UPD6376   TC514400   62256     |
|MJM2904                                |
|                                       |
|M                            42.9545MHz|
|A                   |----------|       |
|H                   |          |       |
|J                   | ST-0016  |       |
|O                   |          |  48MHz|
|N                   |          |       |
|G                   |          |       |
|5                   |----------|       |
|6                          BATTERY     |
|                                       |
|                                       |
|                              KOI-5    |
|   KOI-4   KOI-2   KOI-3  KOI-1    6264|
|---------------------------------------|

*/

ROM_START( koikois )
	ROM_REGION( 0x410000, "maincpu", 0 )

	ROM_LOAD16_BYTE( "koi-2.6c", 0x010001, 0x080000, CRC(2722be71) SHA1(1aa3d819eef01db042ee04a01c1b18c4d9dae65e) )
	ROM_LOAD16_BYTE( "koi-1.4c", 0x010000, 0x080000, CRC(c79e2b43) SHA1(868174f7ab8e68e31d3302ae94dd742048deed9f) )
	ROM_LOAD16_BYTE( "koi-4.8c", 0x110001, 0x080000, CRC(ace236df) SHA1(4bf56affe5b6d0ba3cc677eaa91f9be77f26c654) )
	ROM_LOAD16_BYTE( "koi-3.5c", 0x110000, 0x080000, CRC(6fd88149) SHA1(87b1be32770232eb041e3ef9d1da45282af8a5d4) )
	ROM_LOAD( "koi-5.2c", 0x210000, 0x200000, CRC(561e12c8) SHA1(a7aedf549bc3141fc01bc4a10c235af265ba4ee9) )
	ROM_COPY( "maincpu",  0x10000, 0x00000, 0x08000 )
ROM_END


/*
Mayjinsen (JPN Ver.)
(c)1994 Seta

CPU:    UPD70732-25 V810 ?
Sound:  Custom (ST-0016 ?)

sx003.01    main prg
sx003.02
sx003.03
sx003.04    /

sx003.05d   chr
sx003.06
sx003.07d   /

-----------

Mayjinsen II
Seta, 1994

This game runs on Seta hardware. The game is similar to Shougi.

PCB Layout
----------

E52-00001
|----------------------------------------------------|
|                  62256    62256    62256    62256  |
| D70732GD-25                                        |
| NEC 1991 V810    62256    62256    62256    62256  |
|                                                    |
|                  62256    62256    62256    62256  |
|                                                    |
|                SX007-01 SX007-02  SX007-03 SX007-04|
|                                                    |
|                                   6264             |
|                                                    |
|                   62256      42.9545MHz  48MHz     |
|      PAL                                           |
|                   62256                            |
| 46MHz                          ST-0016   SX007-05  |
|                                TC6210AF            |
|                                                    |
|                   TC514800                         |
|                                                    |
|                                           DSW1-8   |
|                                                    |
|                                           DSW2-8   |
|                           JAMMA                    |
|----------------------------------------------------|
*/

ROM_START(mayjinsn )
	ROM_REGION( 0x190000, "maincpu", 0 )
	ROM_LOAD( "sx003.05d",   0x010000, 0x80000,  CRC(2be6d620) SHA1(113db888fb657d45be55708bbbf9a9ac159a9636) )
	ROM_LOAD( "sx003.06",    0x090000, 0x80000,  CRC(f0553386) SHA1(8915cb3ce03b9a12612694caec9bbec6de4dd070) )
	ROM_LOAD( "sx003.07d",   0x110000, 0x80000, CRC(8db281c3) SHA1(f8b488dd28010f01f789217a4d62ba2116e06e94) )
	ROM_COPY( "maincpu",   0x010000, 0x00000, 0x08000 )

	ROM_REGION32_LE( 0x20000*4, "user1", 0 ) /* V810 code */
	ROM_LOAD32_BYTE( "sx003.04",   0x00003, 0x20000,   CRC(fa15459f) SHA1(4163ab842943705c550f137abbdd2cb51ba5390f) )
	ROM_LOAD32_BYTE( "sx003.03",   0x00002, 0x20000,   CRC(71a438ea) SHA1(613bab6a59aa1bced2ab37177c61a0fd7ce7e64f) )
	ROM_LOAD32_BYTE( "sx003.02",   0x00001, 0x20000,   CRC(61911eed) SHA1(1442b3867b85120ba652ec8205d74332addffb67) )
	ROM_LOAD32_BYTE( "sx003.01",   0x00000, 0x20000,   CRC(d210bfe5) SHA1(96d9f2b198d98125df4bd6b15705646d472a8a87) )
ROM_END

ROM_START(mayjisn2 )
	ROM_REGION( 0x110000, "maincpu", 0 )
	ROM_LOAD( "sx007-05.8b",   0x10000, 0x100000,  CRC(b13ea605) SHA1(75c067df02c988f170c24153d3852c472355fc9d) )
	ROM_COPY( "maincpu",  0x10000, 0x00000, 0x08000 )

	ROM_REGION32_LE( 0x20000*4, "user1", 0 ) /* V810 code */
	ROM_LOAD32_BYTE( "sx007-04.4b",   0x00003, 0x20000,   CRC(fa15459f) SHA1(4163ab842943705c550f137abbdd2cb51ba5390f) )
	ROM_LOAD32_BYTE( "sx007-03.4j",   0x00002, 0x20000,   CRC(71a438ea) SHA1(613bab6a59aa1bced2ab37177c61a0fd7ce7e64f) )
	ROM_LOAD32_BYTE( "sx007-02.4m",   0x00001, 0x20000,   CRC(61911eed) SHA1(1442b3867b85120ba652ec8205d74332addffb67) )
	ROM_LOAD32_BYTE( "sx007-01.4s",   0x00000, 0x20000,   CRC(d210bfe5) SHA1(96d9f2b198d98125df4bd6b15705646d472a8a87) )
ROM_END

/*************************************
 *
 *  Game-specific driver inits
 *
 *************************************/

static DRIVER_INIT(renju)
{
	st0016_game=0;
}

static DRIVER_INIT(nratechu)
{
	st0016_game=1;
}

static DRIVER_INIT(mayjinsn)
{
	st0016_game=4|0x80;
	memory_set_bankptr(machine, "bank2", memory_region(machine, "user1"));
}

static DRIVER_INIT(mayjisn2)
{
	st0016_game=4;
	memory_set_bankptr(machine, "bank2", memory_region(machine, "user1"));
}

/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME(  1994, renju,	0,	  st0016,   renju,    renju,    ROT0, "Visco", "Renju Kizoku", 0)
GAME(  1996, nratechu,	0,	  st0016,   nratechu, nratechu, ROT0, "Seta",  "Neratte Chu", 0)
GAME(  1994, mayjisn2,	0,	  mayjinsn, mayjisn2, mayjisn2, ROT0, "Seta",  "Mayjinsen 2", 0)
GAME(  1995, koikois,	 0,	  st0016, koikois, renju, ROT0, "Visco",  "Koi Koi Shimasho", GAME_IMPERFECT_GRAPHICS)
/* Not working */
GAME( 1994, mayjinsn,	0,	  mayjinsn, st0016,   mayjinsn, ROT0, "Seta",  "Mayjinsen",GAME_IMPERFECT_GRAPHICS|GAME_NOT_WORKING)
