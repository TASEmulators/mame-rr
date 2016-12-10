/************************************************************************

  Super Draw Poker (c) Stern Electronics 1983

  Driver by Pierpaolo Prazzoli.
  Additional work by Angelo Salese & Roberto Fresca.

*************************************************************************

  Notes:

  - According to some original screenshots floating around the net, this game
    uses a weird coloring scheme for the char text that dynamically changes
    the color of an entire column. For now I can only guess about how it
    truly works.

  - An original snap of these can be seen at:
    http://mamedev.emulab.it/kale/fast/files/A00000211628-007.jpg


*************************************************************************


  Updates:

  - Reworked inputs to match the standard poker inputs names/layout.
  - Hooked the payout switch.
  - Hooked a watchdog circuitery, that seems intended to reset
     the game and/or an external device.
  - Added machine start & reset.
  - All clocks pre defined.
  - Added ay8910 interfase as a preliminary attempt to analyze the unknown
     port writes when these ports are set as input.
  - Figured out the following DIP switches:
     Auto Bet (No, Yes).
     Allow Raise (No, Yes).
     Double-Up (No, Yes).
     Minimal Winner Hand (Jacks or Better, Two Pair).
     Deal Speed (Slow, Fast).
     Aces Type (Normal Aces, Number 1).
     Cards Deck Type (english cards, french cards).
     Max Bet (5, 10, 15, 20).
  - Added NVRAM support.
  - Reorganized and cleaned-up the driver.


  To do:

  - Needs schematics to check if the current implementation of the
    "global column coloring" is correct.
  - Check unknown read/writes, too many of them.
  - Check the correct CPU and AY8910 clocks from PCB.
  - A workaround for writes to ay8910 ports when these are set as input.


************************************************************************/


#define MASTER_CLOCK	XTAL_12MHz
#define CPU_CLOCK		MASTER_CLOCK/4	/* guess */
#define SND_CLOCK		MASTER_CLOCK/8	/* guess */

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"

class supdrapo_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, supdrapo_state(machine)); }

	supdrapo_state(running_machine &machine) { }

	UINT8 *char_bank;
	UINT8 *col_line;
	UINT8 *videoram;
	UINT8 wdog;
};


/*********************************************************************
                           Video Hardware
**********************************************************************/

static VIDEO_START( supdrapo )
{
}


static VIDEO_UPDATE( supdrapo )
{
	supdrapo_state *state = (supdrapo_state *)screen->machine->driver_data;
	int x, y;
	int count;
	int color;

	count = 0;

	for(y = 0; y < 32; y++)
	{
		for(x = 0; x < 32; x++)
		{
			int tile = state->videoram[count] + state->char_bank[count] * 0x100;
			/* Global Column Coloring, GUESS! */
			color = state->col_line[(x*2) + 1] ? (state->col_line[(x*2) + 1] - 1) & 7 : 0;

			drawgfx_opaque(bitmap, cliprect, screen->machine->gfx[0], tile,color, 0, 0, x*8, y*8);

			count++;
		}
	}

	return 0;
}


/*Maybe bit 2 & 3 of the second color prom are intensity bits? */
static PALETTE_INIT( sdpoker )
{
	int	bit0, bit1, bit2 , r, g, b;
	int	i;

	for (i = 0; i < 0x100; ++i)
	{
		bit0 = 0;//(color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 0) & 0x01;
		bit2 = (color_prom[0] >> 1) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit0 = 0;//(color_prom[0] >> 3) & 0x01;
		bit1 = (color_prom[0] >> 2) & 0x01;
		bit2 = (color_prom[0] >> 3) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit0 = 0;//(color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0x100] >> 0) & 0x01;
		bit2 = (color_prom[0x100] >> 1) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
		color_prom++;
	}
}


/*********************************************************************
                            R/W Handlers
**********************************************************************/

static READ8_HANDLER( sdpoker_rng_r )
{
	return mame_rand(space->machine);
}

static WRITE8_HANDLER( wdog8000_w )
{
/*  Kind of state watchdog alternating 0x00 & 0x01 writes.
    Used when exit the test mode (writes 2 consecutive 0's).
    Seems to be intended to reset some external device.

  Watchdog: 01
  Watchdog: 00
  Watchdog: 01
  Watchdog: 00
  Watchdog: 01
  Watchdog: 00
  Watchdog: 00 ---> Exit from test mode.

  Debug3: 00
  Watchdog: 00
  Watchdog: 00

  Debug3: 00
  Watchdog: 01
  Watchdog: 00
  Watchdog: 01
  Watchdog: 00
  Watchdog: 01
  Watchdog: 00

*/
	supdrapo_state *state = (supdrapo_state *)space->machine->driver_data;

	if (state->wdog == data)
	{
		watchdog_reset_w(space, 0, 0);	/* Reset */
	}

	state->wdog = data;
//  logerror("Watchdog: %02X\n", data);
}


static WRITE8_HANDLER( debug8004_w )
{
/*  Writes 0x00 each time the machine is initialized */

	logerror("debug8004: %02X\n", data);
//  popmessage("written : %02X", data);
}

static WRITE8_HANDLER( debug7c00_w )
{
/*  This one write 0's constantly when the input test mode is running */
	logerror("debug7c00: %02X\n", data);
}


/*********************************************************************
                         Coin I/O Counters
**********************************************************************/

static WRITE8_HANDLER( coinin_w )
{
	coin_counter_w(space->machine, 0, data & 0x01);	/* Coin In */
}

static WRITE8_HANDLER( payout_w )
{
	coin_counter_w(space->machine, 1, data & 0x01);	/* Payout */
}


/*********************************************************************
                        Machine Start & Reset
**********************************************************************/

static MACHINE_START( supdrapo )
{
}


static MACHINE_RESET( supdrapo )
{
	supdrapo_state *state = (supdrapo_state *)machine->driver_data;
	state->wdog = 1;
}


/*********************************************************************
                              Memory Map
**********************************************************************/

static ADDRESS_MAP_START( sdpoker_mem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x4fff) AM_ROM
	AM_RANGE(0x5000, 0x50ff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x57ff, 0x57ff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x5800, 0x58ff) AM_RAM AM_SHARE("share1") AM_BASE_MEMBER(supdrapo_state,col_line)
	AM_RANGE(0x6000, 0x67ff) AM_RAM //work ram
	AM_RANGE(0x6800, 0x6bff) AM_RAM AM_BASE_MEMBER(supdrapo_state,videoram)
	AM_RANGE(0x6c00, 0x6fff) AM_RAM AM_BASE_MEMBER(supdrapo_state,char_bank)
	AM_RANGE(0x7000, 0x7bff) AM_RAM //$7600 seems watchdog
	AM_RANGE(0x7c00, 0x7c00) AM_WRITE(debug7c00_w)
	AM_RANGE(0x8000, 0x8000) AM_READ_PORT("IN4") AM_WRITE(wdog8000_w)
	AM_RANGE(0x8001, 0x8001) AM_READ_PORT("IN0")
	AM_RANGE(0x8002, 0x8002) AM_READ_PORT("IN1") AM_WRITE(payout_w)
	AM_RANGE(0x8003, 0x8003) AM_READ_PORT("IN2") AM_WRITE(coinin_w)
	AM_RANGE(0x8004, 0x8004) AM_READ_PORT("IN3") AM_WRITE(debug8004_w)
	AM_RANGE(0x8005, 0x8005) AM_READ_PORT("SW2")
	AM_RANGE(0x8006, 0x8006) AM_READ_PORT("SW1")
	AM_RANGE(0x9000, 0x90ff) AM_RAM AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE(0x9400, 0x9400) AM_READ(sdpoker_rng_r)
	AM_RANGE(0x9800, 0x9801) AM_DEVWRITE("aysnd", ay8910_data_address_w)
ADDRESS_MAP_END


/*********************************************************************
                             Input Ports
**********************************************************************/

static INPUT_PORTS_START( supdrapo )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_NAME("P1 Win/Take") PORT_CODE(KEYCODE_4) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_NAME("P1 Cancel") PORT_CODE(KEYCODE_N) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )  PORT_NAME("P1 Deal") PORT_CODE(KEYCODE_2) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON11 ) PORT_NAME("P1 Bet/Play") PORT_CODE(KEYCODE_1) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_NAME("Coin 4: 10")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_NAME("Coin 3:  5")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_NAME("Coin 2:  2")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_NAME("Coin 1:  1")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("P1 Hold 5") PORT_CODE(KEYCODE_B) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P1 Hold 4") PORT_CODE(KEYCODE_V) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P1 Hold 3") PORT_CODE(KEYCODE_C) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Hold 2") PORT_CODE(KEYCODE_X) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Hold 1") PORT_CODE(KEYCODE_Z) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("P1 Black") PORT_CODE(KEYCODE_A) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("P1 Red") PORT_CODE(KEYCODE_S) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON10 ) PORT_NAME("P1 Double Up") PORT_CODE(KEYCODE_3) PORT_PLAYER(1)

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Win/Take") PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Cancel") PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Deal") PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Bet/Play") PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Coin : 10")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Coin :  5")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Coin :  2")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Coin :  1")

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Hold 5") PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Hold 4") PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Hold 3") PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Hold 2") PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Hold 1") PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Black") PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Red") PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Double Up") PORT_PLAYER(2)

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 Flip") PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Flip") PORT_PLAYER(2)
	PORT_DIPNAME( 0x04, 0x00, "4-3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "4-4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "4-5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_NAME("Payout") PORT_CODE(KEYCODE_W)
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_HIGH )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, "1 Player" )
	PORT_DIPSETTING(    0x80, "2 Players" )

	PORT_START("SW1")
	PORT_DIPNAME( 0x01, 0x01, "Auto Bet" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, "Allow Raise" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x04, "Double-Up" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, "Minimal Winner Hand" )
	PORT_DIPSETTING(    0x08, "Jacks or Better" )
	PORT_DIPSETTING(    0x00, "Two Pair" )
	PORT_DIPNAME( 0x10, 0x00, "Deal Speed" )
	PORT_DIPSETTING(    0x00, "Slow" )
	PORT_DIPSETTING(    0x10, "Fast" )
	PORT_DIPNAME( 0x20, 0x00, "Aces Type" )
	PORT_DIPSETTING(    0x00, "Normal Aces" )
	PORT_DIPSETTING(    0x20, "Number 1" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Language ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x40, DEF_STR( French ) )
	PORT_DIPNAME( 0x80, 0x00, "Cards Deck Type" )
	PORT_DIPSETTING(    0x00, "English Cards" )
	PORT_DIPSETTING(    0x80, "French Cards" )

	PORT_START("SW2")
	PORT_DIPNAME( 0x01, 0x00, "6-1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "6-2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "6-3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "6-4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "6-5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "6-6" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Max Bet" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x80, "15" )
	PORT_DIPSETTING(    0xc0, "30" )
INPUT_PORTS_END


/*********************************************************************
                      Graphics Decode / Layout
**********************************************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{ 0, RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( supdrapo )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0, 16 )
GFXDECODE_END


/*********************************************************************
                         Sound Interface
**********************************************************************/

static WRITE8_DEVICE_HANDLER( ay8910_outputa_w )
{
//  popmessage("ay8910_outputa_w %02x",data);
}

static WRITE8_DEVICE_HANDLER( ay8910_outputb_w )
{
//  popmessage("ay8910_outputb_w %02x",data);
}


static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_HANDLER(ay8910_outputa_w),
	DEVCB_HANDLER(ay8910_outputb_w)
};


/*********************************************************************
                           Machine Driver
**********************************************************************/

static MACHINE_DRIVER_START( supdrapo )

	MDRV_DRIVER_DATA( supdrapo_state )

	MDRV_CPU_ADD("maincpu", Z80, CPU_CLOCK)	/* guess */
	MDRV_CPU_PROGRAM_MAP(sdpoker_mem)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_MACHINE_START(supdrapo)
	MDRV_MACHINE_RESET(supdrapo)

	MDRV_NVRAM_HANDLER(generic_0fill)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)

	MDRV_GFXDECODE(supdrapo)
	MDRV_PALETTE_LENGTH(0x100)
	MDRV_PALETTE_INIT(sdpoker)

	MDRV_VIDEO_START(supdrapo)
	MDRV_VIDEO_UPDATE(supdrapo)

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("aysnd", AY8910, SND_CLOCK)	/* guess */
	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END


/*********************************************************************
                            ROMs Load
**********************************************************************/

/*
A2-1C   8910
A2-1D   Z80
A1-1E
A1-1H
A3-1J

        A1-4K
        A1-4L
        A1-4N
        A1-4P           A1-9N (6301)
                        A1-9P
*/
ROM_START( supdrapo )
	ROM_REGION( 0x010000, "maincpu", 0 )
	ROM_LOAD( "a2-1c",        0x0000, 0x1000, CRC(b65af689) SHA1(b45cd15ca8f9c931d83a90f3cdbebf218070b195) )
	ROM_LOAD( "a2-1d",        0x1000, 0x1000, CRC(9ccc4347) SHA1(ea8f4d17aaacc7091ca0a66247b55eb12155c9d7) )
	ROM_LOAD( "a1-1e",        0x2000, 0x1000, CRC(44f2b75d) SHA1(615d0acd3f8a109334f415732b6b4fe7b810d91c) ) //a2-1e
	ROM_LOAD( "a1-1h",        0x3000, 0x1000, CRC(9c1a10ff) SHA1(243dd64f0b29f9bed4cfa8ecb801ddd973d9e3c3) )
	ROM_LOAD( "a3-1j",        0x4000, 0x1000, CRC(71c2bf1c) SHA1(cb98bbf88b8a410075a074ec8619c6e703c6c582) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "a1-4p",        0x0000, 0x1000, CRC(5ac096cc) SHA1(60173a83d0e60fd4d0eb40b7b4e80a74ac5fb23d) )
	ROM_LOAD( "a1-4n",        0x1000, 0x1000, CRC(6985fac9) SHA1(c6357fe0f042b67f8559ec9da03106d1ff08dc66) )
	ROM_LOAD( "a1-4l",        0x2000, 0x1000, CRC(534f7b94) SHA1(44b83053827b27b9e45f6fc50d3878984ef5c5cc) )
	ROM_LOAD( "a1-4k",        0x3000, 0x1000, CRC(3d881f5b) SHA1(53d8800a098e4393224de0b82f8e516f73fd6b62) )

	ROM_REGION( 0x00200, "proms", 0 )
	ROM_LOAD( "a1-9n",        0x0000, 0x0100, CRC(e62529e3) SHA1(176f2069b0c06c1d088909e81658652af06c8eec) )
	ROM_LOAD( "a1-9p",        0x0100, 0x0100, CRC(a0547746) SHA1(747c8aef5afa26124fe0763e7f96c4ff6be31863) )
ROM_END

/*
------------------------------------------

  Jeutel (bootleg?)

  1x MOSTEK 8236 / MK3880N-IRL / Z80-CPU
  1x SOUND AY-3-8910
  1x X-TAL 12,000
  2x 8 DIP switches banks.

------------------------------------------
*/
ROM_START( supdrapoa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "0.c1",	0x0000, 0x1000, CRC(63e2775a) SHA1(742e8db5378631fd93a22d2131f9523ee74c03a5) )
	ROM_LOAD( "1.d1",	0x1000, 0x1000, CRC(aa1578de) SHA1(8f1a33864b2c8e09a25c7603522ebfc7e0757d56) )
	ROM_LOAD( "2.e1",	0x2000, 0x1000, CRC(ffe0415c) SHA1(0233d192814ced0b32abd4b7d2e93431a339732f) )
	ROM_LOAD( "3.h1",	0x3000, 0x1000, CRC(1bae52fa) SHA1(f89d48d67e52d0fca51eb23fee2d5cb94afcf7f4) )
	ROM_LOAD( "4.j1",	0x4000, 0x1000, CRC(7af26f63) SHA1(aeeca69ef1c21acae4283183e4b073ec0d303f4a) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "8.p4",	0x0000, 0x1000, CRC(ef0700c5) SHA1(53f49d99f310fdf675e3b7339bdca1115e4a1935) )
	ROM_LOAD( "7.n4",	0x1000, 0x1000, CRC(3f77031b) SHA1(2d282d39ea568aa44af8b56228b6e096c2713a15) )
	ROM_LOAD( "6.l4",	0x2000, 0x1000, CRC(d70cd50e) SHA1(c3e3dcf79f8a25df5b878ef8734a6d0dc22004ba) )
	ROM_LOAD( "5.k4",	0x3000, 0x1000, CRC(34564917) SHA1(90b49fe8a5371159388839d42913352cf58c60e6) )

	ROM_REGION( 0x00200, "proms", 0 )	/* using the color PROMs from the parent set - no reason to think they differ */
	ROM_LOAD( "a1-9n",        0x0000, 0x0100, CRC(e62529e3) SHA1(176f2069b0c06c1d088909e81658652af06c8eec) )
	ROM_LOAD( "a1-9p",        0x0100, 0x0100, CRC(a0547746) SHA1(747c8aef5afa26124fe0763e7f96c4ff6be31863) )
ROM_END

/*
Poker Relance Gamble
EMU Infos dumper    f205v
manufacturer    Valadon Automation

Technical references

CPUs
QTY     Type        clock       position    function
1x      NEC D780C               2c          8-bit Microprocessor - main
1x      AY-3-8910               2a          Programmable Sound Generator - sound
1x      LM380N                  10b         Audio Amplifier - sound
1x      oscillator  12.000MHz   5b

ROMs
QTY     Type                    position    status
9x      ET2732Q                 0-8         dumped
1x      DM74S287N               9n,9p       dumped

RAMs
QTY     Type                    position
8x      MM2114N-3               1k,1l,1m,1n,2f,3f,3h,3j
1x      MWS5101AEL2             2p

Others

1x 22x2 edge connector
1x 31x2 thin edge connector
1x trimmer (volume)(10a)
2x 8x2 switches DIP (8a,9a)

Notes

At 2l there is an empty space with "batt." handwritten on the PCB
At 1p there is an unmarked DIP20 mil.300 chip.

*/

ROM_START( supdrapob )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pok0.1c",	0x0000, 0x1000, CRC(b53f0470) SHA1(79003cc957e22d5bde720b6f4caed5481edd2c7e) )
	ROM_LOAD( "pok1.1d",	0x1000, 0x1000, CRC(9797a42d) SHA1(65446317e6f1a2de53dd10146338fb63bd5b0a99) )
	ROM_LOAD( "pok2.1ef",	0x2000, 0x1000, CRC(2b7a5baa) SHA1(dd86bb35692eabc1482768cf0bc082f3e0bd90fe) )
	ROM_LOAD( "pok3.1h",	0x3000, 0x1000, CRC(9c3ea609) SHA1(612f455515f367b7d59608528d06221665da8876) )
	ROM_LOAD( "pok4.1j",	0x4000, 0x1000, CRC(52025ba3) SHA1(923de6110a3608698a31baf552d4854b1053cc0e) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "pok8.4p",	0x0000, 0x1000, CRC(82b387e1) SHA1(d7b7e4f7b5b8082438444ce1fa585917ae737bcf) )
	ROM_LOAD( "pok7.4n",	0x1000, 0x1000, CRC(6ab0ad02) SHA1(86b22ab3ceb69f981aa32247c93411631c33a6e8) )
	ROM_LOAD( "pok6.4lm",	0x2000, 0x1000, CRC(c8eab65c) SHA1(c4d37fed9675d8bb051e6f97e56f27450a24ddb8) )
	ROM_LOAD( "pok5.4k",	0x3000, 0x1000, CRC(2c0bb656) SHA1(aa2f309afcdefda5e40be0a354d0b3e5548c44bb) )

	ROM_REGION( 0x00200, "proms", 0 )
	ROM_LOAD( "dm74s287n.9n",        0x0000, 0x0100, CRC(e62529e3) SHA1(176f2069b0c06c1d088909e81658652af06c8eec) )
	ROM_LOAD( "dm74s287n.9p",        0x0100, 0x0100, CRC(a0547746) SHA1(747c8aef5afa26124fe0763e7f96c4ff6be31863) )
ROM_END



/*********************************************************************
                           Games Drivers
**********************************************************************/

/*    YEAR  NAME       PARENT    MACHINE   INPUT     INIT  ROT     COMPANY                               FULLNAME                   FLAGS... */
GAME( 1983, supdrapo,  0,        supdrapo, supdrapo, 0,    ROT90, "Valadon Automation (Stern Electronics license)", "Super Draw Poker (set 1)", 0 )
GAME( 1983, supdrapoa, supdrapo, supdrapo, supdrapo, 0,    ROT90, "Valadon Automation / Jeutel", "Super Draw Poker (set 2)", 0 )
GAME( 1983, supdrapob, supdrapo, supdrapo, supdrapo, 0,    ROT90, "bootleg", "Super Draw Poker (bootleg)", 0 )
