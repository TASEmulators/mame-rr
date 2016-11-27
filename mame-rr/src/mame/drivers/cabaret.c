/***************************************************************************

Cabaret (AMT)
Driver by Mirko Buffoni, David Haywood

TODO:
- This game should have an NVRAM.  There is trace of System Reset so need
  to find how to reset its content.
- DSW3 is read, not sure where it's used
- Keyboard is mapped thru test mode, but some bits are unknown, and hopper
  is not emulated
- Map Leds and Coin counters
- Remove patches after finding why there are so many pitfalls.  Maybe the
  game expects to read inputs via an external device and expects certain
  timings

Press F1+F2 during reset to see 'pork*ish' test mode :P

Interesting thing: this game is copyright AMT 1992, but protection checks
are the same of IGS.  AMT may be previous IGS name.

***************************************************************************/

#include "emu.h"
#include "cpu/z180/z180.h"
#include "sound/2413intf.h"


/***************************************************************************
                                Video Hardware
***************************************************************************/


static UINT8   *bg_tile_ram;
static tilemap_t *bg_tilemap;
static UINT8   *bg_scroll;

static UINT8   *fg_tile_ram, *fg_color_ram;
static tilemap_t *fg_tilemap;

static WRITE8_HANDLER( bg_scroll_w )
{
	bg_scroll[offset] = data;
	tilemap_set_scrolly(bg_tilemap,offset,data);
}

static WRITE8_HANDLER( bg_tile_w )
{
	bg_tile_ram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap,offset);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int code = bg_tile_ram[tile_index];
	SET_TILE_INFO(1, code & 0xff, 0, 0);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	int code = fg_tile_ram[tile_index] | (fg_color_ram[tile_index] << 8);
	int tile = code & 0x1fff;
	SET_TILE_INFO(0, code, tile != 0x1fff ? ((code >> 12) & 0xe) + 1 : 0, 0);
}

static WRITE8_HANDLER( fg_tile_w )
{
	fg_tile_ram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap,offset);
}

static WRITE8_HANDLER( fg_color_w )
{
	fg_color_ram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap,offset);
}

static VIDEO_START(cabaret)
{
	bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows,	8,  32,	64, 8);
	fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows,	8,  8,	64, 32);
	tilemap_set_transparent_pen(fg_tilemap, 0);
	tilemap_set_scroll_cols(bg_tilemap, 64);
}


static VIDEO_UPDATE(cabaret)
{
	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));

	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);

	tilemap_draw(bitmap, cliprect, fg_tilemap, 0, 0);

	return 0;
}

/***************************************************************************
                                Memory Maps
***************************************************************************/

static int nmi_enable;

static UINT8 out[3];

static void show_out(void)
{
#ifdef MAME_DEBUG
	popmessage("%02x %02x %02x", out[0], out[1], out[2]);
#endif
}

static WRITE8_HANDLER( cabaret_nmi_and_coins_w )
{
	if ((nmi_enable ^ data) & (~0xdd))
	{
		logerror("PC %06X: nmi_and_coins = %02x\n",cpu_get_pc(space->cpu),data);
//      popmessage("%02x",data);
	}

	coin_counter_w(space->machine, 0,		data & 0x01);	// coin_a
	coin_counter_w(space->machine, 1,		data & 0x04);	// coin_c
	coin_counter_w(space->machine, 2,		data & 0x08);	// key in
	coin_counter_w(space->machine, 3,		data & 0x10);	// coin out mech

	set_led_status(space->machine, 6,		data & 0x40);	// led for coin out / hopper active

	nmi_enable = data;	//  data & 0x80     // nmi enable?

	out[0] = data;
	show_out();
}



static ADDRESS_MAP_START( cabaret_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE( 0x00000, 0x0efff ) AM_ROM
	AM_RANGE( 0x0f000, 0x0ffff ) AM_RAM AM_REGION("maincpu", 0xf000)
ADDRESS_MAP_END

static ADDRESS_MAP_START( cabaret_portmap, ADDRESS_SPACE_IO, 8 )
	AM_RANGE( 0x0000, 0x003f ) AM_RAM // Z180 internal regs

	AM_RANGE( 0x0080, 0x0080 ) AM_READ_PORT( "BUTTONS2" )
	AM_RANGE( 0x0081, 0x0081 ) AM_READ_PORT( "SERVICE" )
	AM_RANGE( 0x0082, 0x0082 ) AM_READ_PORT( "COINS" )
	AM_RANGE( 0x0090, 0x0090 ) AM_READ_PORT( "BUTTONS1" )
	AM_RANGE( 0x00a0, 0x00a0 ) AM_WRITE( cabaret_nmi_and_coins_w )

	AM_RANGE( 0x00a1, 0x00a1 ) AM_READ_PORT("DSW1")			/* DSW1 */
	AM_RANGE( 0x00a2, 0x00a2 ) AM_READ_PORT("DSW2")			/* DSW2 */
	AM_RANGE( 0x00b0, 0x00b0 ) AM_READ_PORT("DSW3")			/* DSW3 */

	AM_RANGE( 0x00e0, 0x00e1 ) AM_DEVWRITE( "ymsnd", ym2413_w )

	AM_RANGE( 0x2000, 0x27ff ) AM_RAM_WRITE( fg_tile_w )  AM_BASE( &fg_tile_ram )
	AM_RANGE( 0x2800, 0x2fff ) AM_RAM_WRITE( fg_color_w ) AM_BASE( &fg_color_ram )

	AM_RANGE( 0x3000, 0x37ff ) AM_RAM_WRITE( paletteram_xBBBBBGGGGGRRRRR_split1_w ) AM_BASE_GENERIC( paletteram )
	AM_RANGE( 0x3800, 0x3fff ) AM_RAM_WRITE( paletteram_xBBBBBGGGGGRRRRR_split2_w ) AM_BASE_GENERIC( paletteram2 )

	AM_RANGE( 0x1000, 0x103f ) AM_RAM_WRITE( bg_scroll_w ) AM_BASE( &bg_scroll )

	AM_RANGE( 0x1800, 0x19ff ) AM_RAM_WRITE( bg_tile_w )  AM_BASE( &bg_tile_ram )
	AM_RANGE( 0x8000, 0xffff ) AM_ROM AM_REGION("gfx3", 0)
ADDRESS_MAP_END


/***************************************************************************
                                Input Ports
***************************************************************************/

static INPUT_PORTS_START( cabaret )
	PORT_START("DSW1")		// OK
	PORT_DIPNAME( 0x07, 0x00, "Poke %" ) PORT_DIPLOCATION("SWA:1,2,3")
	PORT_DIPSETTING(    0x07, "60%" )
	PORT_DIPSETTING(    0x06, "65%" )
	PORT_DIPSETTING(    0x05, "70%" )
	PORT_DIPSETTING(    0x04, "75%" )
	PORT_DIPSETTING(    0x03, "82%" )
	PORT_DIPSETTING(    0x02, "85%" )
	PORT_DIPSETTING(    0x01, "88%" )
	PORT_DIPSETTING(    0x00, "92%" )
	PORT_DIPNAME( 0x08, 0x00, "Double %" ) PORT_DIPLOCATION("SWA:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x10, 0x00, "Coin Credit" ) PORT_DIPLOCATION("SWA:5")
	PORT_DIPSETTING(    0x10, "5/1" )
	PORT_DIPSETTING(    0x00, "10/1" )
	PORT_DIPNAME( 0x20, 0x00, "Held Method" ) PORT_DIPLOCATION("SWA:6")
	PORT_DIPSETTING(    0x20, "Discard" )
	PORT_DIPSETTING(    0x00, "Held" )
	PORT_DIPNAME( 0x40, 0x00, "Speed" ) PORT_DIPLOCATION("SWA:7")
	PORT_DIPSETTING(    0x40, "Slow" )
	PORT_DIPSETTING(    0x00, "Quick" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW2")		// OK
	PORT_DIPNAME( 0x03, 0x00, "Limit" ) PORT_DIPLOCATION("SWB:1,2")
	PORT_DIPSETTING(    0x03, "5000" )
	PORT_DIPSETTING(    0x02, "10000" )
	PORT_DIPSETTING(    0x01, "15000" )
	PORT_DIPSETTING(    0x00, "30000" )
	PORT_DIPNAME( 0x0c, 0x00, "Max Bet" ) PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x0c, "20" )
	PORT_DIPSETTING(    0x08, "30" )
	PORT_DIPSETTING(    0x04, "40" )
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPNAME( 0x10, 0x00, "Withdraw" ) PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_DIPLOCATION("SWB:6,7,8")

	PORT_START("DSW3")
	PORT_DIPUNKNOWN( 0xff, 0xff )

	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Memory Clear")	// stats, memory
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_SERVICE_NO_TOGGLE( 0x20, IP_ACTIVE_LOW )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Statistics")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN  )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1   )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2   )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Down")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("BUTTONS1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("BUTTONS2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) PORT_NAME("Decrement")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE ) PORT_NAME("Collect")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("W-Up")
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

/***************************************************************************
                                Graphics Layout
***************************************************************************/

static const gfx_layout layout_8x8x6 =
{
	8, 8,
	RGN_FRAC(1, 3),
	6,
	{ RGN_FRAC(0,3)+8,RGN_FRAC(0,3)+0,
	  RGN_FRAC(1,3)+8,RGN_FRAC(1,3)+0,
	  RGN_FRAC(2,3)+8,RGN_FRAC(2,3)+0 },
	{ STEP8(0,1) },
	{ STEP8(0,2*8) },
	8*8*2
};

static const gfx_layout layout_8x32x6i =
{
	8, 32,
	RGN_FRAC(1, 3),
	6,
	{ RGN_FRAC(0,3)+8,RGN_FRAC(0,3)+0,
	  RGN_FRAC(1,3)+8,RGN_FRAC(1,3)+0,
	  RGN_FRAC(2,3)+8,RGN_FRAC(2,3)+0 },
	{ STEP8(0,1) },
	{ STEP32(0,2*8) },
	8*32*2
};

static GFXDECODE_START( cabaret )
	GFXDECODE_ENTRY( "gfx1", 0x00000, layout_8x8x6,  0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x00000, layout_8x32x6i, 0, 16 )
GFXDECODE_END




/***************************************************************************
                                Machine Drivers
***************************************************************************/

static MACHINE_RESET( cabaret )
{
	nmi_enable		=	0;
}

static INTERRUPT_GEN( cabaret_interrupt )
{
	 if (nmi_enable & 0x80)
		cpu_set_input_line(device, INPUT_LINE_NMI, PULSE_LINE);
}

static MACHINE_DRIVER_START( cabaret )
	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z180, XTAL_12MHz / 2)
	MDRV_CPU_PROGRAM_MAP(cabaret_map)
	MDRV_CPU_IO_MAP(cabaret_portmap)
	MDRV_CPU_VBLANK_INT("screen",cabaret_interrupt)

	MDRV_MACHINE_RESET(cabaret)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(512, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-1)

	MDRV_GFXDECODE(cabaret)
	MDRV_PALETTE_LENGTH(0x800)

	MDRV_VIDEO_START(cabaret)
	MDRV_VIDEO_UPDATE(cabaret)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("ymsnd", YM2413, XTAL_3_579545MHz)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


static DRIVER_INIT( cabaret )
{
	UINT8 *rom = memory_region(machine, "maincpu");
	int i;

	/* decrypt the program ROM */
	for (i = 0;i < 0xf000;i++)
	{
		if ((i & 0x2206) == 0x2002) rom[i] ^= 0x01;
	}

	/* Patch pitfalls */
	rom[0x1012] =
	rom[0x1013] = 0;
	rom[0x13b8] = 0x18;
	rom[0x53a6] = 0x18;
	rom[0x73c6] = 0x18;
	rom[0xc46a] = 0x18;
	rom[0xc583] = 0x18;
	rom[0xc5fa] = 0x18;
	rom[0xc6c4] = 0x18;
}

ROM_START( cabaret )
	ROM_REGION( 0x10000, "maincpu", 0 )	/* 64k for code */
	ROM_LOAD( "cg-8v204.u97",  0x0000, 0x10000, CRC(44cebf77) SHA1(e3f4e4abf41388f0eed50cf9a0fd0b14aa2f8b93) )

	ROM_REGION( 0x60000, "gfx1", 0 )
	ROM_LOAD( "cg-4.u43",  0x40000, 0x20000, CRC(e509f50a) SHA1(7e68ca54642c92cdb348d5cf9466065938d0e027) )
	ROM_LOAD( "cg-5.u44",  0x20000, 0x20000, CRC(e2cbf489) SHA1(3a15ed7efd5696656e6d55b54ec0ff779bdb0d98) )
	ROM_LOAD( "cg-6.u45",  0x00000, 0x20000, CRC(4f2fced7) SHA1(b954856ffdc97fbc99fd3ec087376fbf466d2d5a) )

	ROM_REGION( 0xc000, "gfx2", 0 )
	ROM_LOAD( "cg-1.u40",  0x8000, 0x4000, CRC(7dee8b1f) SHA1(80dbdf6aab9b02cc000956b7894023552428e6a1) )
	ROM_LOAD( "cg-2.u41",  0x0000, 0x4000, CRC(ce8dea39) SHA1(b30d1678a7b98cd821d2ce7383a83cb7c9f31b5f) )
	ROM_LOAD( "cg-3.u42",  0x4000, 0x4000, CRC(7e1f821f) SHA1(b709d49f9d1890fe3b8ca7f90affc0017a0ad95e) )

	ROM_REGION( 0x8000, "gfx3", 0 )
	ROM_LOAD( "cg-7.u98",  0x0000, 0x8000, CRC(b93ae6f8) SHA1(accb87045c278d5d79fff65bb763aa6e8025a945) )	/* background maps, read by the CPU */
ROM_END

GAME( 1992, cabaret,  0, cabaret,  cabaret, cabaret,  ROT0, "AMT Co. Ltd.", "Cabaret", GAME_NOT_WORKING )
