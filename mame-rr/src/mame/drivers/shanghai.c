/***************************************************************************

Shanghai

driver by Nicola Salmoria

The HD63484 emulation is incomplete, it implements the bare minimum required
to run these games.

The end of round animation in Shanghai is wrong; change the opcode at 0xfb1f2
to a NOP to jump to it immediately at the beginning of a round.

I'm not sure about the refresh rate, 60Hz makes time match the dip switch
settings, but music runs too fast.


* kothello

Notes: If you use the key labeled as 'Service Coin' you can start the game
with a single 'coin' no matter the Coingae Setting, but the credit is not
displayed.

***************************************************************************/

#include "emu.h"
#include "cpu/nec/nec.h"
#include "audio/seibu.h"
#include "video/hd63484.h"

static PALETTE_INIT( shanghai )
{
	int i;


	for (i = 0;i < machine->total_colors();i++)
	{
		int bit0,bit1,bit2,r,g,b;


		/* red component */
		bit0 = (i >> 2) & 0x01;
		bit1 = (i >> 3) & 0x01;
		bit2 = (i >> 4) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (i >> 5) & 0x01;
		bit1 = (i >> 6) & 0x01;
		bit2 = (i >> 7) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = 0;
		bit1 = (i >> 0) & 0x01;
		bit2 = (i >> 1) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}
}

static VIDEO_START( shanghai )
{
}

static VIDEO_UPDATE( shanghai )
{
	running_device *hd63484 = screen->machine->device("hd63484");
	int x, y, b, src;

	b = ((hd63484_regs_r(hd63484, 0xcc/2, 0xffff) & 0x000f) << 16) + hd63484_regs_r(hd63484, 0xce/2, 0xffff);
	for (y = 0; y < 280; y++)
	{
		for (x = 0 ; x < (hd63484_regs_r(hd63484, 0xca/2, 0xffff) & 0x0fff) * 2 ; x += 2)
		{
			b &= (HD63484_RAM_SIZE - 1);
			src = hd63484_ram_r(hd63484, b, 0xffff);
			*BITMAP_ADDR16(bitmap, y, x)     = src & 0x00ff;
			*BITMAP_ADDR16(bitmap, y, x + 1) = (src & 0xff00) >> 8;
			b++;
		}
	}

	if ((hd63484_regs_r(hd63484, 0x06/2, 0xffff) & 0x0300) == 0x0300)
	{
		int sy = (hd63484_regs_r(hd63484, 0x94/2, 0xffff) & 0x0fff) - (hd63484_regs_r(hd63484, 0x88/2, 0xffff) >> 8);
		int h = hd63484_regs_r(hd63484, 0x96/2, 0xffff) & 0x0fff;
		int sx = ((hd63484_regs_r(hd63484, 0x92/2, 0xffff) >> 8) - (hd63484_regs_r(hd63484, 0x84/2, 0xffff) >> 8)) * 4;
		int w = (hd63484_regs_r(hd63484, 0x92/2, 0xffff) & 0xff) * 4;
		if (sx < 0) sx = 0;	// not sure about this (shangha2 title screen)

		b = (((hd63484_regs_r(hd63484, 0xdc/2, 0xffff) & 0x000f) << 16) + hd63484_regs_r(hd63484, 0xde/2, 0xffff));

		for (y = sy ; y <= sy + h && y < 280 ; y++)
		{
			for (x = 0 ; x < (hd63484_regs_r(hd63484, 0xca/2, 0xffff) & 0x0fff) * 2 ; x += 2)
			{
				b &= (HD63484_RAM_SIZE - 1);
				src = hd63484_ram_r(hd63484, b, 0xffff);
				if (x <= w && x + sx >= 0 && x + sx < (hd63484_regs_r(hd63484, 0xca/2, 0xffff) & 0x0fff) * 2)
				{
					*BITMAP_ADDR16(bitmap, y, x + sx)     = src & 0x00ff;
					*BITMAP_ADDR16(bitmap, y, x + sx + 1) = (src & 0xff00) >> 8;
				}
				b++;
			}
		}
	}

	return 0;
}

static INTERRUPT_GEN( shanghai_interrupt )
{
	cpu_set_input_line_and_vector(device,0,HOLD_LINE,0x80);
}

static WRITE16_HANDLER( shanghai_coin_w )
{
	if (ACCESSING_BITS_0_7)
	{
		coin_counter_w(space->machine, 0,data & 1);
		coin_counter_w(space->machine, 1,data & 2);
	}
}

static ADDRESS_MAP_START( shanghai_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x00000, 0x03fff) AM_RAM
	AM_RANGE(0x80000, 0xfffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( shangha2_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x00000, 0x03fff) AM_RAM
	AM_RANGE(0x04000, 0x041ff) AM_WRITE(paletteram16_xxxxBBBBGGGGRRRR_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x80000, 0xfffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( shanghai_portmap, ADDRESS_SPACE_IO, 16 )
	AM_RANGE(0x00, 0x01) AM_DEVREADWRITE("hd63484", hd63484_status_r, hd63484_address_w)
	AM_RANGE(0x02, 0x03) AM_DEVREADWRITE("hd63484", hd63484_data_r, hd63484_data_w)
	AM_RANGE(0x20, 0x23) AM_DEVREADWRITE8("ymsnd", ym2203_r, ym2203_w, 0x00ff)
	AM_RANGE(0x40, 0x41) AM_READ_PORT("P1")
	AM_RANGE(0x44, 0x45) AM_READ_PORT("P2")
	AM_RANGE(0x48, 0x49) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x4c, 0x4d) AM_WRITE(shanghai_coin_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( shangha2_portmap, ADDRESS_SPACE_IO, 16 )
	AM_RANGE(0x00, 0x01) AM_READ_PORT("P1")
	AM_RANGE(0x10, 0x11) AM_READ_PORT("P2")
	AM_RANGE(0x20, 0x21) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x30, 0x31) AM_DEVREADWRITE("hd63484", hd63484_status_r, hd63484_address_w)
	AM_RANGE(0x32, 0x33) AM_DEVREADWRITE("hd63484", hd63484_data_r, hd63484_data_w)
	AM_RANGE(0x40, 0x43) AM_DEVREADWRITE8("ymsnd", ym2203_r, ym2203_w, 0x00ff)
	AM_RANGE(0x50, 0x51) AM_WRITE(shanghai_coin_w)
ADDRESS_MAP_END

static READ16_HANDLER( kothello_hd63484_status_r )
{
	return 0xff22;	/* write FIFO ready + command end + read FIFO ready */
}

static ADDRESS_MAP_START( kothello_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x00000, 0x07fff) AM_RAM
	AM_RANGE(0x08010, 0x08011) AM_READ(kothello_hd63484_status_r) AM_DEVWRITE("hd63484", hd63484_address_w)
	AM_RANGE(0x08012, 0x08013) AM_DEVREADWRITE("hd63484", hd63484_data_r, hd63484_data_w)
	AM_RANGE(0x09010, 0x09011) AM_READ_PORT("P1")
	AM_RANGE(0x09012, 0x09013) AM_READ_PORT("P2")
	AM_RANGE(0x09014, 0x09015) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x09016, 0x0901f) AM_WRITENOP // 0x9016 is set to 0 at the boot
	AM_RANGE(0x0a000, 0x0a1ff) AM_WRITE(paletteram16_xxxxBBBBGGGGRRRR_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x0b010, 0x0b01f) AM_READWRITE(seibu_main_word_r, seibu_main_word_w)
	AM_RANGE(0x80000, 0xfffff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( kothello )
	SEIBU_COIN_INPUTS	/* coin inputs read through sound cpu */

	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( shanghai )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x1c, 0x1c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x14, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_4C ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Confirmation" )
	PORT_DIPSETTING(    0x01, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, "Help" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x08, "2 Players Move Time" )
	PORT_DIPSETTING(    0x0c, "8" )
	PORT_DIPSETTING(    0x08, "10" )
	PORT_DIPSETTING(    0x04, "12" )
	PORT_DIPSETTING(    0x00, "14" )
	PORT_DIPNAME( 0x30, 0x20, "Bonus Time for Making Pair" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0xc0, 0x40, "Start Time" )
	PORT_DIPSETTING(    0xc0, "30" )
	PORT_DIPSETTING(    0x80, "60" )
	PORT_DIPSETTING(    0x40, "90" )
	PORT_DIPSETTING(    0x00, "120" )
INPUT_PORTS_END

static INPUT_PORTS_START( shangha2 )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x06, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x08, 0x00, "2 Players Move Time" )
	PORT_DIPSETTING(    0x08, "8" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x30, 0x20, "Bonus Time for Making Pair" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0xc0, 0x40, "Start Time" )
	PORT_DIPSETTING(    0xc0, "30" )
	PORT_DIPSETTING(    0x80, "60" )
	PORT_DIPSETTING(    0x40, "90" )
	PORT_DIPSETTING(    0x00, "120" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, "Mystery Tiles" )
	PORT_DIPSETTING(    0x03, "0" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "6" )
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPNAME( 0x1c, 0x1c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x14, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_4C ) )
INPUT_PORTS_END



static const ym2203_interface sh_ym2203_interface =
{
	{
		AY8910_LEGACY_OUTPUT,
		AY8910_DEFAULT_LOADS,
		DEVCB_INPUT_PORT("DSW1"),
		DEVCB_INPUT_PORT("DSW2"),
		DEVCB_NULL,
		DEVCB_NULL
	},
	NULL
};


static const ym2203_interface kothello_ym2203_interface =
{
	{
		AY8910_LEGACY_OUTPUT,
		AY8910_DEFAULT_LOADS,
		DEVCB_INPUT_PORT("DSW"),
		DEVCB_NULL, DEVCB_NULL, DEVCB_NULL
	},
	seibu_ym2203_irqhandler
};

static const hd63484_interface shanghai_hd63484_intf = { 0 };

static MACHINE_DRIVER_START( shanghai )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", V30,16000000/2)	/* ? */
	MDRV_CPU_PROGRAM_MAP(shanghai_map)
	MDRV_CPU_IO_MAP(shanghai_portmap)
	MDRV_CPU_VBLANK_INT("screen", shanghai_interrupt)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(30)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(384, 280)
	MDRV_SCREEN_VISIBLE_AREA(0, 384-1, 0, 280-1) // Base Screen is 384 pixel

	MDRV_PALETTE_LENGTH(256)

	MDRV_PALETTE_INIT(shanghai)
	MDRV_VIDEO_START(shanghai)
	MDRV_VIDEO_UPDATE(shanghai)

	MDRV_HD63484_ADD("hd63484", shanghai_hd63484_intf)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ymsnd", YM2203, 16000000/4)
	MDRV_SOUND_CONFIG(sh_ym2203_interface)
	MDRV_SOUND_ROUTE(0, "mono", 0.15)
	MDRV_SOUND_ROUTE(1, "mono", 0.15)
	MDRV_SOUND_ROUTE(2, "mono", 0.15)
	MDRV_SOUND_ROUTE(3, "mono", 0.80)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( shangha2 )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", V30,16000000/2)	/* ? */
	MDRV_CPU_PROGRAM_MAP(shangha2_map)
	MDRV_CPU_IO_MAP(shangha2_portmap)
	MDRV_CPU_VBLANK_INT("screen", shanghai_interrupt)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(30)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(384, 280)
	MDRV_SCREEN_VISIBLE_AREA(0, 384-1, 0, 280-1) // Base Screen is 384 pixel

	MDRV_PALETTE_LENGTH(256)

	MDRV_VIDEO_START(shanghai)
	MDRV_VIDEO_UPDATE(shanghai)

	MDRV_HD63484_ADD("hd63484", shanghai_hd63484_intf)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ymsnd", YM2203, 16000000/4)
	MDRV_SOUND_CONFIG(sh_ym2203_interface)
	MDRV_SOUND_ROUTE(0, "mono", 0.15)
	MDRV_SOUND_ROUTE(1, "mono", 0.15)
	MDRV_SOUND_ROUTE(2, "mono", 0.15)
	MDRV_SOUND_ROUTE(3, "mono", 0.80)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( kothello )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", V30,16000000/2)	/* ? */
	MDRV_CPU_PROGRAM_MAP(kothello_map)
	MDRV_CPU_VBLANK_INT("screen", shanghai_interrupt)

	SEIBU3A_SOUND_SYSTEM_CPU(14318180/4)

	MDRV_QUANTUM_TIME(HZ(12000))

	MDRV_MACHINE_RESET(seibu_sound)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(30)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(384, 280)
	MDRV_SCREEN_VISIBLE_AREA(8, 384-1, 0, 250-1) // Base Screen is 376 pixel

	MDRV_PALETTE_LENGTH(256)

	MDRV_VIDEO_START(shanghai)
	MDRV_VIDEO_UPDATE(shanghai)

	MDRV_HD63484_ADD("hd63484", shanghai_hd63484_intf)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	/* same as standard seibu ym2203, but "ym1" also reads "DSW" */
	MDRV_SOUND_ADD("ym1", YM2203, 14318180/4)
	MDRV_SOUND_CONFIG(kothello_ym2203_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)

	MDRV_SOUND_ADD("ym2", YM2203, 14318180/4)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)

	SEIBU_SOUND_SYSTEM_ADPCM_INTERFACE
MACHINE_DRIVER_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( shanghai )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "shg-22a.rom", 0xa0001, 0x10000, CRC(e0a085be) SHA1(e281043f97c4cd34a33eb1ec7154abbe67a9aa03) )
	ROM_LOAD16_BYTE( "shg-21a.rom", 0xa0000, 0x10000, CRC(4ab06d32) SHA1(02667d1270b101386b947d5b9bfe64052e498041) )
	ROM_LOAD16_BYTE( "shg-28a.rom", 0xc0001, 0x10000, CRC(983ec112) SHA1(110e120e35815d055d6108a7603e83d2d990c666) )
	ROM_LOAD16_BYTE( "shg-27a.rom", 0xc0000, 0x10000, CRC(41af0945) SHA1(dfc4638a17f716ccc8e59f275571d6dc1093a745) )
	ROM_LOAD16_BYTE( "shg-37b.rom", 0xe0001, 0x10000, CRC(3f192da0) SHA1(e70d5da5d702e9bf9ac6b77df62bcf51894aadcf) )
	ROM_LOAD16_BYTE( "shg-36b.rom", 0xe0000, 0x10000, CRC(a1d6af96) SHA1(01c4c22bf03b3d260fffcbc6dfc5f2dd2bcba14a) )
ROM_END

ROM_START( shangha2 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sht-27j", 0x80001, 0x20000, CRC(969cbf00) SHA1(350025f4e39c7d89cb72e46b52fb467e3e9056f4) )
	ROM_LOAD16_BYTE( "sht-26j", 0x80000, 0x20000, CRC(4bf01ab4) SHA1(6928374db080212a371991ee98cd563e158907f0) )
	ROM_LOAD16_BYTE( "sht-31j", 0xc0001, 0x20000, CRC(312e3b9d) SHA1(f15f76a087d4972aa72145eced8d1fb15329b359) )
	ROM_LOAD16_BYTE( "sht-30j", 0xc0000, 0x20000, CRC(2861a894) SHA1(6da99d15f41e900735f8943f2710487817f98579) )
ROM_END

/*

Kyuukyoku no Othello
Success Corp. 1990

PCB Layout

SSS8906
|------------------------------------------|
| YM3014 DSW2  DSW1           4464    4464 |
|                             4464    4464 |
|              YM2203         4464    4464 |
|J      M5205  Z80            4464    4464 |
|A             6116           4464    4464 |
|M             PAL            4464    4464 |
|M                            4464    4464 |
|A  ROM6                      4464    4464 |
|              ROM5           HD63484      |
| YM3931                             898-3 |
|                  SIS6091                 |
|                                          |
|                                          |
| PAL                                      |
| PAL              ROM3  ROM4              |
|                                    PAL   |
|                  ROM2  ROM1              |
| CXQ70116                                 |
| D71011           6264  6264              |
|16MHz     PAL     6264  6264              |
|------------------------------------------|
Notes:
      Z80 clock     : 4.000MHz
      CXQ70116 clock: 16.000MHz
      YM3931 clock  : 4.000MHz
      YM2203 clock  : 4.000MHz
      M5205 clock   : 444598Hz; sample rate = M5205 clock / 48
      HD63484 clock : pin50- 4.000MHz, pin52- 2.000MHz
      VSync         : 57Hz

      HD63484  : CRT Controller
      CXQ70116 : Compatible with NEC V30 (DIP40)
      D71011   : ? (DIP18) 710xx is series of common NEC ICs; timers, counters, parallel interface, interrupt controllers etc
      898-3    : BI 898-3-R 22  8920  (?, DIP16, tied to hd63484)
      YM3931   : Also printed 'SEI0100BU' (SDIP64)
      SIS6091  : Custom QFP80 (Graphics controller?)
      4464     : 64K x4 DRAM
      6264       8K x8 SRAM
      6116     : 2K x8 SRAM

*/


ROM_START( kothello )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rom1.3e", 0x80001, 0x20000, CRC(8601dcfa) SHA1(e7ffc6da0bfb5cec5a543a2a5223b235c3428eb3) )
	ROM_LOAD16_BYTE( "rom2.5e", 0x80000, 0x20000, CRC(68f6b7a3) SHA1(9f7e217e07bc79b1e95551cd0fe107294bf5889f) )
	ROM_LOAD16_BYTE( "rom3.3f", 0xc0001, 0x20000, CRC(2f3dacd1) SHA1(35bfdc1f377b87a80c3abbb48f9f0b52108fbfc0) )
	ROM_LOAD16_BYTE( "rom4.5f", 0xc0000, 0x20000, CRC(ee8bbea7) SHA1(35dfa7aa89cecba6482b18a5233511bacc4bf331) )

	ROM_REGION( 0x20000, "audiocpu", 0 )
	ROM_LOAD( "rom5.5l",   0x00000, 0x02000, CRC(7eb6e697) SHA1(4476e13f9a9e04472581f2c069760f53b33d5672))
	ROM_CONTINUE(          0x10000, 0x0e000 )

	ROM_REGION( 0x10000, "adpcm", 0 )
	ROM_LOAD( "rom6.7m",   0x00000, 0x10000, CRC(4ab1335d) SHA1(3a803e8a7e9b0c2a26ee23e7ac9c89c70cf2504b))
ROM_END



GAME( 1988, shanghai, 0, shanghai, shanghai, 0, ROT0, "Sunsoft", "Shanghai (Japan)", GAME_IMPERFECT_GRAPHICS )
GAME( 1989, shangha2, 0, shangha2, shangha2, 0, ROT0, "Sunsoft", "Shanghai II (Japan)", 0 )
GAME( 1990, kothello, 0, kothello, kothello, 0, ROT0, "Success", "Kyuukyoku no Othello", GAME_IMPERFECT_GRAPHICS )

