/******************************************************************************

    Game Driver for Nichibutsu Mahjong series.

    Pastel Gal
    (c)1985 Nihon Bussan Co.,Ltd.

    Driver by Takahiro Nogi <nogi@kt.rim.or.jp> 2000/06/07 -

******************************************************************************/
/******************************************************************************
Memo:

- Custom chip used by pastelg PCB is 1411M1.

- Some games display "GFXROM BANK OVER!!" or "GFXROM ADDRESS OVER!!"
  in Debug build.

- Screen flip is not perfect.

******************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "includes/nb1413m3.h"
#include "sound/ay8910.h"
#include "sound/dac.h"


#define SIGNED_DAC	0		// 0:unsigned DAC, 1:signed DAC
#if SIGNED_DAC
#define DAC_WRITE	dac_signed_w
#else
#define DAC_WRITE	dac_w
#endif


extern PALETTE_INIT( pastelg );
extern VIDEO_UPDATE( pastelg );
extern VIDEO_START( pastelg );

extern WRITE8_HANDLER( pastelg_clut_w );
extern WRITE8_HANDLER( pastelg_romsel_w );
extern WRITE8_HANDLER( threeds_romsel_w );
extern WRITE8_HANDLER( threeds_output_w );
extern WRITE8_HANDLER( pastelg_blitter_w );
extern READ8_HANDLER( threeds_rom_readback_r );
extern int pastelg_blitter_src_addr_r(void);


static DRIVER_INIT( pastelg )
{
	nb1413m3_type = NB1413M3_PASTELG;
}

static READ8_HANDLER( pastelg_sndrom_r )
{
	UINT8 *ROM = memory_region(space->machine, "voice");

	return ROM[pastelg_blitter_src_addr_r() & 0x7fff];
}

static ADDRESS_MAP_START( pastelg_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xe000, 0xe7ff) AM_RAM AM_BASE(&nb1413m3_nvram) AM_SIZE(&nb1413m3_nvram_size)
ADDRESS_MAP_END

static READ8_HANDLER( pastelg_irq_ack_r )
{
	cpu_set_input_line(space->cpu, 0, CLEAR_LINE);
	return 0;
}

static ADDRESS_MAP_START( pastelg_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
//  AM_RANGE(0x00, 0x00) AM_WRITENOP
	AM_RANGE(0x00, 0x7f) AM_READ(nb1413m3_sndrom_r)
	AM_RANGE(0x81, 0x81) AM_DEVREAD("aysnd", ay8910_r)
	AM_RANGE(0x82, 0x83) AM_DEVWRITE("aysnd", ay8910_data_address_w)
	AM_RANGE(0x90, 0x90) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x90, 0x96) AM_WRITE(pastelg_blitter_w)
	AM_RANGE(0xa0, 0xa0) AM_READWRITE(nb1413m3_inputport1_r, nb1413m3_inputportsel_w)
	AM_RANGE(0xb0, 0xb0) AM_READWRITE(nb1413m3_inputport2_r, pastelg_romsel_w)
	AM_RANGE(0xc0, 0xc0) AM_READ(pastelg_sndrom_r)
	AM_RANGE(0xc0, 0xcf) AM_WRITE(pastelg_clut_w)
	AM_RANGE(0xd0, 0xd0) AM_READ(pastelg_irq_ack_r) AM_DEVWRITE("dac", DAC_WRITE)
	AM_RANGE(0xe0, 0xe0) AM_READ_PORT("DSWC")
ADDRESS_MAP_END

static UINT8 mux_data;

static READ8_HANDLER( threeds_inputport1_r )
{
	switch(mux_data)
	{
		case 0x01: return input_port_read(space->machine,"KEY0_PL1");
		case 0x02: return input_port_read(space->machine,"KEY1_PL1");
		case 0x04: return input_port_read(space->machine,"KEY2_PL1");
		case 0x08: return input_port_read(space->machine,"KEY3_PL1");
		case 0x10: return input_port_read(space->machine,"KEY4_PL1");
	}

	return 0xff;
}

static READ8_HANDLER( threeds_inputport2_r )
{
	switch(mux_data)
	{
		case 0x01: return input_port_read(space->machine,"KEY0_PL2");
		case 0x02: return input_port_read(space->machine,"KEY1_PL2");
		case 0x04: return input_port_read(space->machine,"KEY2_PL2");
		case 0x08: return input_port_read(space->machine,"KEY3_PL2");
		case 0x10: return input_port_read(space->machine,"KEY4_PL2");
	}

	return 0xff;
}

static WRITE8_HANDLER( threeds_inputportsel_w )
{
	mux_data = ~data;
}

static ADDRESS_MAP_START( threeds_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x81, 0x81) AM_DEVREAD("aysnd", ay8910_r)
	AM_RANGE(0x82, 0x83) AM_DEVWRITE("aysnd", ay8910_data_address_w)
	AM_RANGE(0x90, 0x90) AM_READ_PORT("SYSTEM") AM_WRITE( threeds_romsel_w )
	AM_RANGE(0xf0, 0xf6) AM_WRITE(pastelg_blitter_w)
	AM_RANGE(0xa0, 0xa0) AM_READWRITE(threeds_inputport1_r, threeds_inputportsel_w)
	AM_RANGE(0xb0, 0xb0) AM_READ(threeds_inputport2_r) AM_WRITE(threeds_output_w)//writes: bit 3 is coin lockout, bit 1 is coin counter
	AM_RANGE(0xc0, 0xcf) AM_WRITE(pastelg_clut_w)
	AM_RANGE(0xc0, 0xc0) AM_READ(threeds_rom_readback_r)
	AM_RANGE(0xd0, 0xd0) AM_READ(pastelg_irq_ack_r) AM_DEVWRITE("dac", DAC_WRITE)
ADDRESS_MAP_END

static INPUT_PORTS_START( pastelg )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x03, "1 (Easy)" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "4 (Hard)" )
	PORT_DIPNAME( 0x04, 0x04, "DIPSW 1-3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIPSW 1-4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DIPSW 1-5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DIPSW 1-6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DIPSW 1-7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DIPSW 1-8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x03, 0x00, "Number of last chance" )
	PORT_DIPSETTING(    0x03, "0" )
	PORT_DIPSETTING(    0x02, "1" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x04, 0x04, "No. of tiles on final match" )
	PORT_DIPSETTING(    0x04, "20" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x08, 0x08, "DIPSW 2-4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DIPSW 2-5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x00, "SANGEN Rush" )
	PORT_DIPSETTING(    0x60, "0" )
	PORT_DIPSETTING(    0x40, "1" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Infinite ) )
	PORT_DIPNAME( 0x80, 0x80, "DIPSW 2-8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWC")
	PORT_DIPNAME( 0x03, 0x03, "Change Rate" )
	PORT_DIPSETTING(    0x03, "Type-A" )
	PORT_DIPSETTING(    0x02, "Type-B" )
	PORT_DIPSETTING(    0x01, "Type-C" )
	PORT_DIPSETTING(    0x00, "Type-D" )
	PORT_DIPNAME( 0x04, 0x00, "Open CPU's hand on Player's Reach" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIPSW 3-4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DIPSW 3-5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x60, "YAKUMAN cut" )
	PORT_DIPSETTING(    0x60, "10%" )
	PORT_DIPSETTING(    0x40, "30%" )
	PORT_DIPSETTING(    0x20, "50%" )
	PORT_DIPSETTING(    0x00, "90%" )
	PORT_DIPNAME( 0x80, 0x00, "Nudity" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(nb1413m3_busyflag_r, NULL)	// DRAW BUSY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )			//
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE3 )		// MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )		// ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )					// TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )			// COIN1
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE4 )		// CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )		// SERVICE

	PORT_INCLUDE( nbmjcontrols )
INPUT_PORTS_END

// stops the game hanging..
static CUSTOM_INPUT( nb1413m3_hackbusyflag_r )
{
	return mame_rand(field->port->machine) & 3;
}

static INPUT_PORTS_START( threeds )
	PORT_START("DSWA")
    PORT_DIPNAME( 0x01,   0x01, "0" )
    PORT_DIPSETTING(      0x01, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x02,   0x02, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x02, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x04,   0x04, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x04, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x08,   0x08, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x08, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x10,   0x10, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x10, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x20,   0x20, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x20, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x40,   0x40, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x40, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x80,   0x80, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x80, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x00, DEF_STR( On ) )

	PORT_START("DSWB")
    PORT_DIPNAME( 0x01,   0x01, "1" )
    PORT_DIPSETTING(      0x01, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x02,   0x02, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x02, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x04,   0x04, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x04, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x08,   0x08, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x08, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x10,   0x10, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x10, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x20,   0x20, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x20, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x40,   0x40, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x40, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x80,   0x80, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x80, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x00, DEF_STR( On ) )

	PORT_START("DSWC")
    PORT_DIPNAME( 0x01,   0x01, "2" )
    PORT_DIPSETTING(      0x01, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x02,   0x02, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x02, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x04,   0x04, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x04, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x08,   0x08, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x08, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x10,   0x10, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x10, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x20,   0x20, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x20, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x40,   0x40, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x40, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x80,   0x80, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x80, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x00, DEF_STR( On ) )

	PORT_START("KEY0_PL1")
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("1P Start / Deal")
	PORT_BIT( 0x38, IP_ACTIVE_LOW, IPT_UNUSED )
    PORT_DIPNAME( 0x40,   0x40, "1P-Side Character Test Mode" ) //only combined with the service mode
    PORT_DIPSETTING(      0x40, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x00, DEF_STR( On ) )
    PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY1_PL1")
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_BET ) PORT_NAME("1P Bet") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("1P Hold 5") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("1P Hold 3") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("1P Hold 1") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY2_PL1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("1P Change Dealer") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY3_PL1")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("1P Hold 4") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("1P Hold 2") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY4_PL1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL ) PORT_NAME("1P Small")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_BIG ) PORT_NAME("1P Big")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_NAME("1P Flip Flop")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP ) PORT_NAME("1P Double Up")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE ) PORT_NAME("1P Take Score")
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY0_PL2")
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 ) PORT_NAME("2P Start / Deal")
	PORT_BIT( 0x38, IP_ACTIVE_LOW, IPT_UNUSED )
    PORT_DIPNAME( 0x40,   0x40, "2P-Side Character Test Mode" ) //only combined with the service mode
    PORT_DIPSETTING(      0x40, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x00, DEF_STR( On ) )
    PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY1_PL2")
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_BET ) PORT_NAME("2P Bet") PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("2P Hold 5") PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("2P Hold 3") PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("2P Hold 1") PORT_PLAYER(2)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY2_PL2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("2P Change Dealer") PORT_PLAYER(2)
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY3_PL2")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("2P Hold 4") PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("2P Hold 2") PORT_PLAYER(2)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY4_PL2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL ) PORT_NAME("2P Small") PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_BIG ) PORT_NAME("2P Big") PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_NAME("2P Flip Flop") PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP ) PORT_NAME("2P Double Up") PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE ) PORT_NAME("2P Take Score") PORT_PLAYER(2)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(nb1413m3_hackbusyflag_r, NULL)	// DRAW BUSY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE3 )		// MEMORY RESET
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )		// ANALYZER
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )					// TEST
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )			// COIN1
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE4 )		// CREDIT CLEAR
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )		// SERVICE
INPUT_PORTS_END


static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_INPUT_PORT("DSWB"),
	DEVCB_INPUT_PORT("DSWA"),
	DEVCB_NULL,
	DEVCB_NULL
};


static MACHINE_DRIVER_START( pastelg )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, 19968000/8)	/* 2.496 MHz ? */
	MDRV_CPU_PROGRAM_MAP(pastelg_map)
	MDRV_CPU_IO_MAP(pastelg_io_map)
//  MDRV_CPU_VBLANK_INT_HACK(nb1413m3_interrupt,96)  // nmiclock not written, chip is 1411M1 instead of 1413M3
	MDRV_CPU_VBLANK_INT("screen", irq0_line_assert)

	MDRV_MACHINE_RESET(nb1413m3)
	MDRV_NVRAM_HANDLER(nb1413m3)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 256-1, 16, 240-1)

	MDRV_PALETTE_LENGTH(32)

	MDRV_PALETTE_INIT(pastelg)
	MDRV_VIDEO_START(pastelg)
	MDRV_VIDEO_UPDATE(pastelg)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("aysnd", AY8910, 1250000)
	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.35)

	MDRV_SOUND_ADD("dac", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END

/*

Produttore  Nichibutsu
N.revisione TD-1412a
CPU
1x custom Nichibutsu PG14111 (DIL40)(main?)
1x custom Nichibutsu PG14112 (DIL40)(sound?)
1x custom Nichibutsu PG14113 (DIL20)(PAL)
1x custom Nichibutsu PG14114 (DIL20)(PAL)
1x custom Nichibutsu PG1411M1XBA (DIL28)(maybe it's ram)
1x oscillator 19.968MHz
ROMs
7x MBM27256
3x MBM27128
2x PROM MB7112E
Note
1x 18x2 edge connector
1x 10x2 edge connector
2x trimmer (MAIN, SUB)
2x 8 switches dip

*/

static MACHINE_DRIVER_START( threeds )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, 19968000/8)	/* 2.496 MHz ? */
	MDRV_CPU_PROGRAM_MAP(pastelg_map)
	MDRV_CPU_IO_MAP(threeds_io_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_assert)

	MDRV_MACHINE_RESET(nb1413m3)
	MDRV_NVRAM_HANDLER(nb1413m3)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 256-1, 16, 240-1)

	MDRV_PALETTE_LENGTH(32)

	MDRV_PALETTE_INIT(pastelg)
	MDRV_VIDEO_START(pastelg)
	MDRV_VIDEO_UPDATE(pastelg)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("aysnd", AY8910, 1250000)
	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.35)

	MDRV_SOUND_ADD("dac", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END


ROM_START( pastelg )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "pgal_09.bin",  0x00000, 0x04000, CRC(1e494af3) SHA1(1597a7da22ecfbb1df83cf9d0acc7a8be461bc2c) )
	ROM_LOAD( "pgal_10.bin",  0x04000, 0x04000, CRC(677cccea) SHA1(a294bf4e3c5e74291160a0858371961868afc1d1) )
	ROM_LOAD( "pgal_11.bin",  0x08000, 0x04000, CRC(c2ccea38) SHA1(0374e8aa0e7961426e417ffe6e1a0d8dc7fd9ecf) )

	ROM_REGION( 0x08000, "voice", 0 ) /* voice */
	ROM_LOAD( "pgal_08.bin",  0x00000, 0x08000, CRC(895961a1) SHA1(f02d517f46cc490db02c4feb369e2a386c764297) )

	ROM_REGION( 0x38000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "pgal_01.bin",  0x00000, 0x08000, CRC(1bb14d52) SHA1(b3974e3c9b56a752ddcb206f7bb2bc658b0e77f1) )
	ROM_LOAD( "pgal_02.bin",  0x08000, 0x08000, CRC(ea85673a) SHA1(85ef2bb736fe5229ce4153197db8a57bca982a8b) )
	ROM_LOAD( "pgal_03.bin",  0x10000, 0x08000, CRC(40011248) SHA1(935f442a47e02bf8c6ccb324c7fad1b481b8b19a) )
	ROM_LOAD( "pgal_04.bin",  0x18000, 0x08000, CRC(10613a66) SHA1(ad11f99f402e5b247d086cfccafea351da30c084) )
	ROM_LOAD( "pgal_05.bin",  0x20000, 0x08000, CRC(6a152703) SHA1(5dd46d876453c5c79f5a382d77234c690da75001) )
	ROM_LOAD( "pgal_06.bin",  0x28000, 0x08000, CRC(f56acfe8) SHA1(2f4ad3990f2d4d4a9fcec7adab119459423b308b) )
	ROM_LOAD( "pgal_07.bin",  0x30000, 0x08000, CRC(fa4226dc) SHA1(2313449521f81a191e87f1e4c0f3473f3c27dd9d) )

	ROM_REGION( 0x0040, "proms", 0 ) /* color */
	ROM_LOAD( "pgal_bp1.bin", 0x0000, 0x0020, CRC(2b7fc61a) SHA1(278830e8728ea143208376feb20fff56de88ae1c) )
	ROM_LOAD( "pgal_bp2.bin", 0x0020, 0x0020, CRC(4433021e) SHA1(e0d6619a193d26ad24788d4af5ef01ee89cffacd) )
ROM_END

ROM_START( 3ds )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* program */
	ROM_LOAD( "up9.9a",    0x00000, 0x04000, CRC(bc0e7cfd) SHA1(4e84f573fb2c1228757d34b8bc69649b145d9707) )
	ROM_LOAD( "up10.10a",  0x04000, 0x04000, CRC(e185d9f5) SHA1(98d4a824ed6a89e42543fb87daed33ef606bcced) )
	ROM_LOAD( "up11.11a",  0x08000, 0x04000, CRC(d1fb728b) SHA1(46e8e6ccdc1b78da29c969cd9290158c96bac4c4) )

	ROM_REGION( 0x08000, "voice", ROMREGION_ERASE00 ) /* voice */

	ROM_REGION( 0x38000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "1.1a",  0x00000, 0x08000, CRC(5734ca7d) SHA1(d22b9e604cc4e2c0bb4eb32ded06bb5fa519965f) )
	ROM_LOAD( "2.2a",  0x08000, 0x08000, CRC(c7f21718) SHA1(4b2956d499e4db63e7f2329420e3d0313e6360ed) )
	ROM_LOAD( "3.3a",  0x10000, 0x08000, CRC(87bd0a9e) SHA1(a0443017ef4c19f0135c4f764a96457f02cda743) )
	ROM_LOAD( "4.4a",  0x18000, 0x08000, CRC(b75ecf2b) SHA1(50b8f27988dd24ff475a500d361db3c7a7051f40) )
	ROM_LOAD( "5.5a",  0x20000, 0x08000, CRC(22ee5cf6) SHA1(09725a73f5f107e6fcb1994d94a50748726318b0) )
	ROM_LOAD( "6.6a",  0x28000, 0x08000, CRC(d86ebe8d) SHA1(2ede43899501ae27db26b48f53f010a4f0df0307) )
	ROM_LOAD( "7.7a",  0x30000, 0x08000, CRC(6704950a) SHA1(fd60ff2351deb87f19e517cfaedc7ac3dd4aac8d) )

	ROM_REGION( 0x0040, "proms", 0 ) /* color */
	ROM_LOAD( "mb7112e.7h", 0x0000, 0x0020, CRC(2c4f7343) SHA1(7b069c4a4d68ef308d1c1f773ece4b124428da3f) )
	ROM_LOAD( "mb7112e.7j", 0x0020, 0x0020, CRC(181f2a88) SHA1(a75ea981127fc667bb6b9f2ae2766aa2147ff04a) )
ROM_END


GAME( 1985, pastelg, 0, pastelg, pastelg, pastelg, ROT0, "Nichibutsu", "Pastel Gal (Japan 851224)", 0 )
GAME( 1985, 3ds,     0, threeds, threeds, pastelg, ROT0, "Nichibutsu", "Three Ds - Three Dealers Casino House", 0 )
