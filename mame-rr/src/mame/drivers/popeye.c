/***************************************************************************

Popeye  (c) 1982 Nintendo

driver by Marc Lafontaine

To enter service mode, reset keeping the service button pressed.

Notes:
- The main set has a protection device mapped at E000/E001. The second set
  (which is the same revision of the program code) has the protection disabled
  in a very clean way, so I don't know if it's an original (without the
  protection device to save costs), or a very well done bootleg.
- The bootleg derives from a different revision of the program code which we
  don't have.

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"


extern UINT8 *popeye_videoram;
extern UINT8 *popeye_colorram;
extern UINT8 *popeye_background_pos;
extern UINT8 *popeye_palettebank;

extern WRITE8_HANDLER( popeye_videoram_w );
extern WRITE8_HANDLER( popeye_colorram_w );
extern WRITE8_HANDLER( popeye_bitmap_w );
extern WRITE8_HANDLER( skyskipr_bitmap_w );

extern PALETTE_INIT( popeye );
extern PALETTE_INIT( popeyebl );
extern VIDEO_START( skyskipr );
extern VIDEO_START( popeye );
extern VIDEO_UPDATE( popeye );



static INTERRUPT_GEN( popeye_interrupt )
{
	/* NMIs are enabled by the I register?? How can that be? */
	if (cpu_get_reg(device, Z80_I) & 1)	/* skyskipr: 0/1, popeye: 2/3 but also 0/1 */
		cpu_set_input_line(device, INPUT_LINE_NMI, PULSE_LINE);
}


/* the protection device simply returns the last two values written shifted left */
/* by a variable amount. */
static UINT8 prot0,prot1,prot_shift;

static READ8_HANDLER( protection_r )
{
	if (offset == 0)
	{
		return ((prot1 << prot_shift) | (prot0 >> (8-prot_shift))) & 0xff;
	}
	else	/* offset == 1 */
	{
		/* the game just checks if bit 2 is clear. Returning 0 seems to be enough. */
		return 0;
	}
}

static WRITE8_HANDLER( protection_w )
{
	if (offset == 0)
	{
		/* this is the same as the level number (1-3) */
		prot_shift = data & 0x07;
	}
	else	/* offset == 1 */
	{
		prot0 = prot1;
		prot1 = data;
	}
}




static ADDRESS_MAP_START( skyskipr_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x8c00, 0x8c02) AM_RAM AM_BASE(&popeye_background_pos)
	AM_RANGE(0x8c03, 0x8c03) AM_RAM AM_BASE(&popeye_palettebank)
	AM_RANGE(0x8c04, 0x8e7f) AM_RAM AM_BASE_SIZE_GENERIC(spriteram)
	AM_RANGE(0x8e80, 0x8fff) AM_RAM
	AM_RANGE(0xa000, 0xa3ff) AM_WRITE(popeye_videoram_w) AM_BASE(&popeye_videoram)
	AM_RANGE(0xa400, 0xa7ff) AM_WRITE(popeye_colorram_w) AM_BASE(&popeye_colorram)
	AM_RANGE(0xc000, 0xcfff) AM_WRITE(skyskipr_bitmap_w)
	AM_RANGE(0xe000, 0xe001) AM_READWRITE(protection_r,protection_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( popeye_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x8800, 0x8bff) AM_RAM
	AM_RANGE(0x8c00, 0x8c02) AM_RAM AM_BASE(&popeye_background_pos)
	AM_RANGE(0x8c03, 0x8c03) AM_RAM AM_BASE(&popeye_palettebank)
	AM_RANGE(0x8c04, 0x8e7f) AM_RAM AM_BASE_SIZE_GENERIC(spriteram)
	AM_RANGE(0x8e80, 0x8fff) AM_RAM
	AM_RANGE(0xa000, 0xa3ff) AM_WRITE(popeye_videoram_w) AM_BASE(&popeye_videoram)
	AM_RANGE(0xa400, 0xa7ff) AM_WRITE(popeye_colorram_w) AM_BASE(&popeye_colorram)
	AM_RANGE(0xc000, 0xdfff) AM_WRITE(popeye_bitmap_w)
	AM_RANGE(0xe000, 0xe001) AM_READWRITE(protection_r,protection_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( popeyebl_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x8c00, 0x8c02) AM_RAM AM_BASE(&popeye_background_pos)
	AM_RANGE(0x8c03, 0x8c03) AM_RAM AM_BASE(&popeye_palettebank)
	AM_RANGE(0x8c04, 0x8e7f) AM_RAM AM_BASE_SIZE_GENERIC(spriteram)
	AM_RANGE(0x8e80, 0x8fff) AM_RAM
	AM_RANGE(0xa000, 0xa3ff) AM_WRITE(popeye_videoram_w) AM_BASE(&popeye_videoram)
	AM_RANGE(0xa400, 0xa7ff) AM_WRITE(popeye_colorram_w) AM_BASE(&popeye_colorram)
	AM_RANGE(0xc000, 0xcfff) AM_WRITE(skyskipr_bitmap_w)
	AM_RANGE(0xe000, 0xe01f) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( popeye_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVWRITE("aysnd", ay8910_address_data_w)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("P1")
	AM_RANGE(0x01, 0x01) AM_READ_PORT("P2")
	AM_RANGE(0x02, 0x02) AM_READ_PORT("IN0")
	AM_RANGE(0x03, 0x03) AM_DEVREAD("aysnd", ay8910_r)
ADDRESS_MAP_END


static INPUT_PORTS_START( skyskipr )
	PORT_START("P1")	/* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON2 )

	PORT_START("P2")	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL

	PORT_START("IN0")	/* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("DSW0")	/* DSW0 */
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, "A 3/1 B 1/2" )
	PORT_DIPSETTING(    0x0e, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, "A 2/1 B 2/5" )
	PORT_DIPSETTING(    0x04, "A 2/1 B 1/3" )
	PORT_DIPSETTING(    0x07, "A 1/1 B 2/1" )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0c, "A 1/1 B 1/2" )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, "A 1/2 B 1/4" )
	PORT_DIPSETTING(    0x0b, "A 1/2 B 1/5" )
	PORT_DIPSETTING(    0x02, "A 2/5 B 1/1" )
	PORT_DIPSETTING(    0x0a, "A 1/3 B 1/1" )
	PORT_DIPSETTING(    0x09, "A 1/4 B 1/1" )
	PORT_DIPSETTING(    0x05, "A 1/5 B 1/1" )
	PORT_DIPSETTING(    0x08, "A 1/6 B 1/1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL )	/* scans DSW1 one bit at a time */

	PORT_START("DSW1")	/* DSW1 (FAKE - appears as bit 7 of DSW0, see code below) */
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x20, "15000" )
	PORT_DIPSETTING(    0x00, "30000" )
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )
INPUT_PORTS_END


static INPUT_PORTS_START( popeye )
	PORT_START("P1")	/* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* probably unused */

	PORT_START("P2")	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* probably unused */

	PORT_START("IN0")	/* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("DSW0")	/* DSW0 */
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coinage ) )    PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x08, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x40, "Copyright" )
	PORT_DIPSETTING(    0x40, "Nintendo" )
	PORT_DIPSETTING(    0x20, "Nintendo Co.,Ltd" )
	PORT_DIPSETTING(    0x60, "Nintendo of America" )
//  PORT_DIPSETTING(    0x00, "Nintendo of America" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL )	/* scans DSW1 one bit at a time */

	PORT_START("DSW1")	/* DSW1 (FAKE - appears as bit 7 of DSW0, see code below) */
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )       PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )  PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )  PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, "40000" )
	PORT_DIPSETTING(    0x20, "60000" )
	PORT_DIPSETTING(    0x10, "80000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )     PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )
INPUT_PORTS_END


static INPUT_PORTS_START( popeyef )
	PORT_START("P1")	/* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* probably unused */

	PORT_START("P2")	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* probably unused */

	PORT_START("IN0")	/* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("DSW0")	/* DSW0 */
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, "A 3/1 B 1/2" )
	PORT_DIPSETTING(    0x0e, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, "A 2/1 B 2/5" )
	PORT_DIPSETTING(    0x04, "A 2/1 B 1/3" )
	PORT_DIPSETTING(    0x07, "A 1/1 B 2/1" )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0c, "A 1/1 B 1/2" )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, "A 1/2 B 1/4" )
	PORT_DIPSETTING(    0x0b, "A 1/2 B 1/5" )
	PORT_DIPSETTING(    0x02, "A 2/5 B 1/1" )
	PORT_DIPSETTING(    0x0a, "A 1/3 B 1/1" )
	PORT_DIPSETTING(    0x09, "A 1/4 B 1/1" )
	PORT_DIPSETTING(    0x05, "A 1/5 B 1/1" )
	PORT_DIPSETTING(    0x08, "A 1/6 B 1/1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x40, "Copyright" )
	PORT_DIPSETTING(    0x40, "Nintendo" )
	PORT_DIPSETTING(    0x20, "Nintendo Co.,Ltd" )
	PORT_DIPSETTING(    0x60, "Nintendo of America" )
//  PORT_DIPSETTING(    0x00, "Nintendo of America" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL )	/* scans DSW1 one bit at a time */

	PORT_START("DSW1")	/* DSW1 (FAKE - appears as bit 7 of DSW0, see code below) */
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x30, "20000" )
	PORT_DIPSETTING(    0x20, "30000" )
	PORT_DIPSETTING(    0x10, "50000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	16,16,	/* 16*16 characters (8*8 doubled) */
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 7,7, 6,6, 5,5, 4,4, 3,3, 2,2, 1,1, 0,0 },
	{ 0*8,0*8, 1*8,1*8, 2*8,2*8, 3*8,3*8, 4*8,4*8, 5*8,5*8, 6*8,6*8, 7*8,7*8 },
	8*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,4),
	2,
	{ 0, RGN_FRAC(1,2) },
	{RGN_FRAC(1,4)+7,RGN_FRAC(1,4)+6,RGN_FRAC(1,4)+5,RGN_FRAC(1,4)+4,
	 RGN_FRAC(1,4)+3,RGN_FRAC(1,4)+2,RGN_FRAC(1,4)+1,RGN_FRAC(1,4)+0,
	 7,6,5,4,3,2,1,0 },
	{ 15*8, 14*8, 13*8, 12*8, 11*8, 10*8, 9*8, 8*8,
	  7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 },
	16*8
};

static GFXDECODE_START( popeye )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,        16, 16 )	/* chars */
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 16+16*2, 64 )	/* sprites */
GFXDECODE_END



static int dswbit;

static WRITE8_DEVICE_HANDLER( popeye_portB_w )
{
	/* bit 0 flips screen */
	flip_screen_set(device->machine, data & 1);

	/* bits 1-3 select DSW1 bit to read */
	dswbit = (data & 0x0e) >> 1;
}

static READ8_DEVICE_HANDLER( popeye_portA_r )
{
	int res;


	res = input_port_read(device->machine, "DSW0");
	res |= (input_port_read(device->machine, "DSW1") << (7-dswbit)) & 0x80;

	return res;
}

static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_HANDLER(popeye_portA_r),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_HANDLER(popeye_portB_w)
};



static MACHINE_DRIVER_START( skyskipr )
	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, XTAL_8MHz/2)	/* 4 MHz */
	MDRV_CPU_PROGRAM_MAP(skyskipr_map)
	MDRV_CPU_IO_MAP(popeye_io_map)
	MDRV_CPU_VBLANK_INT("screen", popeye_interrupt)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*16, 32*16)
	MDRV_SCREEN_VISIBLE_AREA(0*16, 32*16-1, 2*16, 30*16-1)

	MDRV_GFXDECODE(popeye)
	MDRV_PALETTE_LENGTH(16+16*2+64*4)

	MDRV_PALETTE_INIT(popeye)
	MDRV_VIDEO_START(skyskipr)
	MDRV_VIDEO_UPDATE(popeye)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("aysnd", AY8910, XTAL_8MHz/4)
	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( popeye )
	MDRV_IMPORT_FROM(skyskipr)
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(popeye_map)

	MDRV_VIDEO_START(popeye)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( popeyebl )
	MDRV_IMPORT_FROM(skyskipr)
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(popeyebl_map)

	MDRV_PALETTE_INIT(popeyebl)
	MDRV_VIDEO_START(popeye)
MACHINE_DRIVER_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( skyskipr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tnx1-c.2a",    0x0000, 0x1000, CRC(bdc7f218) SHA1(9a5f5959b9228912f810568ddad70daa81c4daac) )
	ROM_LOAD( "tnx1-c.2b",    0x1000, 0x1000, CRC(cbe601a8) SHA1(78edc384b75b7958906f887d11eb7cf235d6dc44) )
	ROM_LOAD( "tnx1-c.2c",    0x2000, 0x1000, CRC(5ca79abf) SHA1(0712364ad8785a146c4a146cc688c4892dd59c93) )
	ROM_LOAD( "tnx1-c.2d",    0x3000, 0x1000, CRC(6b7a7071) SHA1(949a8106a5b750bc15a5919786fb99f15cd9424e) )
	ROM_LOAD( "tnx1-c.2e",    0x4000, 0x1000, CRC(6b0c0525) SHA1(e4e12ea9e3140736d7543a274f3b266e58059356) )
	ROM_LOAD( "tnx1-c.2f",    0x5000, 0x1000, CRC(d1712424) SHA1(2de42c379f18bfbd68fc64db24c9b0d38de26c29) )
	ROM_LOAD( "tnx1-c.2g",    0x6000, 0x1000, CRC(8b33c4cf) SHA1(86d51b5098dffc69330b28662b04bd906d962792) )
	/* 7000-7fff empty */

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "tnx1-v.3h",    0x0000, 0x0800, CRC(ecb6a046) SHA1(7fd2312d39fefe6237699e2916e0c313165755ad) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "tnx1-t.1e",    0x0000, 0x1000, CRC(01c1120e) SHA1(c1ab2af9c582e647e98836a87cb09fe4bcdaf19f) )
	ROM_LOAD( "tnx1-t.2e",    0x1000, 0x1000, CRC(70292a71) SHA1(51a5677351e438e7ef2b3812ef5e0e610da6ef4d) )
	ROM_LOAD( "tnx1-t.3e",    0x2000, 0x1000, CRC(92b6a0e8) SHA1(d11ff39932cec9b5408dc957bf825dcf1d00c027) )
	ROM_LOAD( "tnx1-t.5e",    0x3000, 0x1000, CRC(cc5f0ac3) SHA1(3374b1c387df3a3eba6c523f7c13711260074a89) )

	ROM_REGION( 0x0340, "proms", 0 )
	ROM_LOAD( "tnx1-t.4a",    0x0000, 0x0020, CRC(98846924) SHA1(89688d05c4c2d6f389da4e9c0c13e0f022f2d376) ) /* background palette */
	ROM_LOAD( "tnx1-t.1a",    0x0020, 0x0020, CRC(c2bca435) SHA1(e7d3e68d153d646eaf12b263d58b07da2615399c) ) /* char palette */
	ROM_LOAD( "tnx1-t.3a",    0x0040, 0x0100, CRC(8abf9de4) SHA1(6e5500639a2dca3c288619fb8bdd120eb49bf8e0) ) /* sprite palette - low 4 bits */
	ROM_LOAD( "tnx1-t.2a",    0x0140, 0x0100, CRC(aa7ff322) SHA1(522d21854aa11e24f3679163354ae4fb35619eff) ) /* sprite palette - high 4 bits */
	ROM_LOAD( "tnx1-t.3j",    0x0240, 0x0100, CRC(1c5c8dea) SHA1(5738303b2a9c79b7d06bcf20fdb4d9b29f6e2d96) ) /* timing for the protection ALU */
ROM_END

/*
    Popeye

    CPU/Sound Board: TPP2-01-CPU
    Video Board:     TPP2-01-VIDEO
*/

ROM_START( popeye )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tpp2-c.7a", 0x0000, 0x2000, CRC(9af7c821) SHA1(592acfe221b5c3bd9b920f639b141f37a56d6997) )
	ROM_LOAD( "tpp2-c.7b", 0x2000, 0x2000, CRC(c3704958) SHA1(af96d10fa9bdb86b00c89d10f67cb5ca5586f446) )
	ROM_LOAD( "tpp2-c.7c", 0x4000, 0x2000, CRC(5882ebf9) SHA1(5531229b37f9ba0ede7fdc24909e3c3efbc8ade4) )
	ROM_LOAD( "tpp2-c.7e", 0x6000, 0x2000, CRC(ef8649ca) SHA1(a0157f91600e56e2a953dadbd76da4330652e5c8) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "tpp2-v.5n", 0x0000, 0x0800, CRC(cca61ddd) SHA1(239f87947c3cc8c6693c295ebf5ea0b7638b781c) )	/* first half is empty */
	ROM_CONTINUE(          0x0000, 0x0800 )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "tpp2-v.1e", 0x0000, 0x2000, CRC(0f2cd853) SHA1(426c9b4f6579bfcebe72b976bfe4f05147d53f96) )
	ROM_LOAD( "tpp2-v.1f", 0x2000, 0x2000, CRC(888f3474) SHA1(ddee56b2b49bd50aaf9c98d8ef6e905e3f6ab859) )
	ROM_LOAD( "tpp2-v.1j", 0x4000, 0x2000, CRC(7e864668) SHA1(8e275dbb1c586f4ebca7548db05246ef0f56d7b1) )
	ROM_LOAD( "tpp2-v.1k", 0x6000, 0x2000, CRC(49e1d170) SHA1(bd51a4e34ce8109f26954760156e3cf05fb9db57) )

	ROM_REGION( 0x0340, "proms", 0 )
	ROM_LOAD( "tpp2-c.4a", 0x0000, 0x0020, CRC(375e1602) SHA1(d84159a0af5db577821c43712bc733329a43af80) ) /* background palette */
	ROM_LOAD( "tpp2-c.3a", 0x0020, 0x0020, CRC(e950bea1) SHA1(0b48082fe79d9fcdca7e80caff1725713d0c3163) ) /* char palette */
	ROM_LOAD( "tpp2-c.5b", 0x0040, 0x0100, CRC(c5826883) SHA1(f2c4d3473b3bfa55bffad003dc1fd79540e7e0d1) ) /* sprite palette - low 4 bits */
	ROM_LOAD( "tpp2-c.5a", 0x0140, 0x0100, CRC(c576afba) SHA1(013c8e8db08a03c7ba156cfefa671d26155fe835) ) /* sprite palette - high 4 bits */
	ROM_LOAD( "tpp2-v.7j", 0x0240, 0x0100, CRC(a4655e2e) SHA1(2a620932fccb763c6c667278c0914f31b9f00ddf) ) /* timing for the protection ALU */
ROM_END

ROM_START( popeyeu )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "7a",        0x0000, 0x2000, CRC(0bd04389) SHA1(3b08186c9b20dd4dfb92df98941b18999f23aece) )
	ROM_LOAD( "7b",        0x2000, 0x2000, CRC(efdf02c3) SHA1(4fa616bdb4e21f752e46890d007c911fff9ceadc) )
	ROM_LOAD( "7c",        0x4000, 0x2000, CRC(8eee859e) SHA1(a597d5655d06d565653c64b18ed8842625e15088) )
	ROM_LOAD( "7e",        0x6000, 0x2000, CRC(b64aa314) SHA1(b5367f518350223e191d94434dc535873efb4c74) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "tpp2-v.5n", 0x0000, 0x0800, CRC(cca61ddd) SHA1(239f87947c3cc8c6693c295ebf5ea0b7638b781c) )	/* first half is empty */
	ROM_CONTINUE(          0x0000, 0x0800 )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "tpp2-v.1e", 0x0000, 0x2000, CRC(0f2cd853) SHA1(426c9b4f6579bfcebe72b976bfe4f05147d53f96) )
	ROM_LOAD( "tpp2-v.1f", 0x2000, 0x2000, CRC(888f3474) SHA1(ddee56b2b49bd50aaf9c98d8ef6e905e3f6ab859) )
	ROM_LOAD( "tpp2-v.1j", 0x4000, 0x2000, CRC(7e864668) SHA1(8e275dbb1c586f4ebca7548db05246ef0f56d7b1) )
	ROM_LOAD( "tpp2-v.1k", 0x6000, 0x2000, CRC(49e1d170) SHA1(bd51a4e34ce8109f26954760156e3cf05fb9db57) )

	ROM_REGION( 0x0340, "proms", 0 )
	ROM_LOAD( "tpp2-c.4a", 0x0000, 0x0020, CRC(375e1602) SHA1(d84159a0af5db577821c43712bc733329a43af80) ) /* background palette */
	ROM_LOAD( "tpp2-c.3a", 0x0020, 0x0020, CRC(e950bea1) SHA1(0b48082fe79d9fcdca7e80caff1725713d0c3163) ) /* char palette */
	ROM_LOAD( "tpp2-c.5b", 0x0040, 0x0100, CRC(c5826883) SHA1(f2c4d3473b3bfa55bffad003dc1fd79540e7e0d1) ) /* sprite palette - low 4 bits */
	ROM_LOAD( "tpp2-c.5a", 0x0140, 0x0100, CRC(c576afba) SHA1(013c8e8db08a03c7ba156cfefa671d26155fe835) ) /* sprite palette - high 4 bits */
	ROM_LOAD( "tpp2-v.7j", 0x0240, 0x0100, CRC(a4655e2e) SHA1(2a620932fccb763c6c667278c0914f31b9f00ddf) ) /* timing for the protection ALU */
ROM_END

ROM_START( popeyef )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tpp2-c_f.7a", 0x0000, 0x2000, CRC(5fc5264d) SHA1(6c3d4df748c55293b6de58bd874a08f8164b878d) )
	ROM_LOAD( "tpp2-c_f.7b", 0x2000, 0x2000, CRC(51de48e8) SHA1(7573931c6fcb53ee5ab9408906cd8eb2ba271c64) )
	ROM_LOAD( "tpp2-c_f.7c", 0x4000, 0x2000, CRC(62df9647) SHA1(65d043b4142aa3ad2db7a1d4e1a2c22656ca7ade) )
	ROM_LOAD( "tpp2-c_f.7e", 0x6000, 0x2000, CRC(f31e7916) SHA1(0f54ea7b1691b7789067fe880ffc56fac1d9523a) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "tpp2-v.5n",   0x0000, 0x0800, CRC(cca61ddd) SHA1(239f87947c3cc8c6693c295ebf5ea0b7638b781c) )	/* first half is empty */
	ROM_CONTINUE(            0x0000, 0x0800 )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "tpp2-v.1e",   0x0000, 0x2000, CRC(0f2cd853) SHA1(426c9b4f6579bfcebe72b976bfe4f05147d53f96) )
	ROM_LOAD( "tpp2-v.1f",   0x2000, 0x2000, CRC(888f3474) SHA1(ddee56b2b49bd50aaf9c98d8ef6e905e3f6ab859) )
	ROM_LOAD( "tpp2-v.1j",   0x4000, 0x2000, CRC(7e864668) SHA1(8e275dbb1c586f4ebca7548db05246ef0f56d7b1) )
	ROM_LOAD( "tpp2-v.1k",   0x6000, 0x2000, CRC(49e1d170) SHA1(bd51a4e34ce8109f26954760156e3cf05fb9db57) )

	ROM_REGION( 0x0340, "proms", 0 )
	ROM_LOAD( "tpp2-c.4a",   0x0000, 0x0020, CRC(375e1602) SHA1(d84159a0af5db577821c43712bc733329a43af80) ) /* background palette */
	ROM_LOAD( "tpp2-c.3a",   0x0020, 0x0020, CRC(e950bea1) SHA1(0b48082fe79d9fcdca7e80caff1725713d0c3163) ) /* char palette */
	ROM_LOAD( "tpp2-c.5b",   0x0040, 0x0100, CRC(c5826883) SHA1(f2c4d3473b3bfa55bffad003dc1fd79540e7e0d1) ) /* sprite palette - low 4 bits */
	ROM_LOAD( "tpp2-c.5a",   0x0140, 0x0100, CRC(c576afba) SHA1(013c8e8db08a03c7ba156cfefa671d26155fe835) ) /* sprite palette - high 4 bits */
	ROM_LOAD( "tpp2-v.7j",   0x0240, 0x0100, CRC(a4655e2e) SHA1(2a620932fccb763c6c667278c0914f31b9f00ddf) ) /* timing for the protection ALU */
ROM_END

ROM_START( popeyebl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "po1",          0x0000, 0x2000, CRC(b14a07ca) SHA1(b8666a4c6b833f60905692774e30e73f0795df11) )
	ROM_LOAD( "po2",          0x2000, 0x2000, CRC(995475ff) SHA1(5cd5ac23a73722e32c80cd6ffc435584750a46c9) )
	ROM_LOAD( "po3",          0x4000, 0x2000, CRC(99d6a04a) SHA1(b683a5bb1ac4f6bec7478760c8ad0ff7c00bc652) )
	ROM_LOAD( "po4",          0x6000, 0x2000, CRC(548a6514) SHA1(006e076781a3e5c3afa084c723247365358e3187) )
	ROM_LOAD( "po_d1-e1.bin", 0xe000, 0x0020, CRC(8de22998) SHA1(e3a232ff85fb207afbe23049a65e828420589342) )	/* protection PROM */

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "v-5n",         0x0000, 0x0800, CRC(cca61ddd) SHA1(239f87947c3cc8c6693c295ebf5ea0b7638b781c) )	/* first half is empty */
	ROM_CONTINUE(             0x0000, 0x0800 )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "v-1e",         0x0000, 0x2000, CRC(0f2cd853) SHA1(426c9b4f6579bfcebe72b976bfe4f05147d53f96) )
	ROM_LOAD( "v-1f",         0x2000, 0x2000, CRC(888f3474) SHA1(ddee56b2b49bd50aaf9c98d8ef6e905e3f6ab859) )
	ROM_LOAD( "v-1j",         0x4000, 0x2000, CRC(7e864668) SHA1(8e275dbb1c586f4ebca7548db05246ef0f56d7b1) )
	ROM_LOAD( "v-1k",         0x6000, 0x2000, CRC(49e1d170) SHA1(bd51a4e34ce8109f26954760156e3cf05fb9db57) )

	ROM_REGION( 0x0240, "proms", 0 )
	ROM_LOAD( "popeye.pr1",   0x0000, 0x0020, CRC(d138e8a4) SHA1(eba7f870ccab72105593007f5cd7e0b863912402) ) /* background palette */
	ROM_LOAD( "popeye.pr2",   0x0020, 0x0020, CRC(0f364007) SHA1(b77d71df391a9ac9e778e84475627e72de2b8507) ) /* char palette */
	ROM_LOAD( "popeye.pr3",   0x0040, 0x0100, CRC(ca4d7b6a) SHA1(ec979fffea9db5a327a5270241e376c21516e343) ) /* sprite palette - low 4 bits */
	ROM_LOAD( "popeye.pr4",   0x0140, 0x0100, CRC(cab9bc53) SHA1(e63ba8856190187996e405f6fcee254c8ca6e81f) ) /* sprite palette - high 4 bits */
ROM_END



static DRIVER_INIT( skyskipr )
{
	UINT8 *buffer;
	UINT8 *rom = memory_region(machine, "maincpu");
	int len = 0x10000;

	/* decrypt the program ROMs */
	buffer = auto_alloc_array(machine, UINT8, len);
	{
		int i;
		for (i = 0;i < len; i++)
			buffer[i] = BITSWAP8(rom[BITSWAP16(i,15,14,13,12,11,10,8,7,0,1,2,4,5,9,3,6) ^ 0xfc],3,4,2,5,1,6,0,7);
		memcpy(rom,buffer,len);
		auto_free(machine, buffer);
	}

    state_save_register_global(machine, prot0);
    state_save_register_global(machine, prot1);
    state_save_register_global(machine, prot_shift);
}

static DRIVER_INIT( popeye )
{
	UINT8 *buffer;
	UINT8 *rom = memory_region(machine, "maincpu");
	int len = 0x10000;

	/* decrypt the program ROMs */
	buffer = auto_alloc_array(machine, UINT8, len);
	{
		int i;
		for (i = 0;i < len; i++)
			buffer[i] = BITSWAP8(rom[BITSWAP16(i,15,14,13,12,11,10,8,7,6,3,9,5,4,2,1,0) ^ 0x3f],3,4,2,5,1,6,0,7);
		memcpy(rom,buffer,len);
		auto_free(machine, buffer);
	}

    state_save_register_global(machine, prot0);
    state_save_register_global(machine, prot1);
    state_save_register_global(machine, prot_shift);
}


GAME( 1981, skyskipr, 0,      skyskipr, skyskipr, skyskipr, ROT0, "Nintendo", "Sky Skipper", GAME_SUPPORTS_SAVE )
GAME( 1982, popeye,   0,      popeye,   popeye,   popeye,   ROT0, "Nintendo", "Popeye (revision D)", GAME_SUPPORTS_SAVE )
GAME( 1982, popeyeu,  popeye, popeye,   popeye,   popeye,   ROT0, "Nintendo", "Popeye (revision D not protected)", GAME_SUPPORTS_SAVE )
GAME( 1982, popeyef,  popeye, popeye,   popeyef,  popeye,   ROT0, "Nintendo", "Popeye (revision F)", GAME_SUPPORTS_SAVE )
GAME( 1982, popeyebl, popeye, popeyebl, popeye,   0,        ROT0, "bootleg",  "Popeye (bootleg)", GAME_SUPPORTS_SAVE )
