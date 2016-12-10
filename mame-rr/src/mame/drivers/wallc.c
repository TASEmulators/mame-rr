/****************************************************************************

Wall Crash by Midcoin (c) 1984


Driver by Jarek Burczynski
2002.12.23




     DIPSW-8     AY-3-8912                               DIPSW-4
                                                         DIPSW-4

                        74s288


                                               WAC1   WAC2   WAC3
                                               (2532) (2532) (2532)
12.288MHz

   +------------+        2114  2114  2114  2114
   + EPOXY WITH +                                +-------+
   + LS08       +      WAC05  WAC1/52   EMPTY    + SMALL +
   +LS240, LS245+      (2764) (2764)    SOCKET   + EPOXY +
   + Z80        +                                +-------+
   +------------+

The bigger Epoxy brick contains three standard 74LSxxx chips and is used as
DATA lines decoder for all READS from addresses in range: 0..0x7fff.
The pinout (of the whole brick) is 1:1 Z80 and it can be replaced with
a plain Z80, given that decoded ROMS are put in place of WAC05 and WAC1/52.

The smaller Epoxy contains:
 5 chips (names sanded off...): 20 pins, 8 pins, 14 pins, 16 pins, 16 pins,
 1 resistor: 120 Ohm
 1 probably resistor: measured: 1000 Ohm
 1 diode: standard 1N4148 (info from HIGHWAYMAN)
 4 capacitors: 3 same: blue ones probably 10n , 1 smaller 1.3n (measured by HIGHWAYMAN)
It's mapped as ROM at 0x6000-0x7fff but is NOT accessed by the CPU.
It's also not needed for emulation.


Thanks to Dox for donating PCB.
Thanks to HIGHWAYMAN for providing info on how to get to these epoxies
(heat gun) and for info (very close one) on decoding.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "video/resnet.h"
#include "sound/ay8910.h"

static tilemap_t *bg_tilemap;

/***************************************************************************

  Convert the color PROMs into a more useable format.

  Wall Crash has one 32 bytes palette PROM, connected to the RGB output this
  way:

  bit 6 -- 330 ohm resistor --+-- 330 ohm pulldown resistor -- RED
  bit 5 -- 220 ohm resistor --/

  bit 4 -- NC

  bit 3 -- 330 ohm resistor --+-- 330 ohm pulldown resistor -- GREEN
  bit 2 -- 220 ohm resistor --/

  bit 1 -- 330 ohm resistor --+--+-- 330 ohm pulldown resistor -- BLUE
  bit 0 -- 220 ohm resistor --/  |
                                 |
  bit 7 -+- diode(~655 Ohm)------/
         \------220 ohm pullup (+5V) resistor


***************************************************************************/

static PALETTE_INIT( wallc )
{
	int i;

	static const int resistances_rg[2] = { 330, 220 };
	static const int resistances_b[3] = { 655, 330, 220 };
	double weights_r[2], weights_g[2], weights_b[3];

	compute_resistor_weights(0,	255,	-1.0,
			2,	resistances_rg,	weights_r,	330,	0,
			2,	resistances_rg,	weights_g,	330,	0,
			3,	resistances_b,	weights_b,	330,	655+220);

	for (i = 0;i < machine->total_colors();i++)
	{
		int bit0,bit1,bit7,r,g,b;

		/* red component */
		bit0 = (color_prom[i] >> 5) & 0x01;
		bit1 = (color_prom[i] >> 6) & 0x01;
		r = combine_2_weights(weights_r, bit1, bit0);

		/* green component */
		bit0 = (color_prom[i] >> 2) & 0x01;
		bit1 = (color_prom[i] >> 3) & 0x01;
		g = combine_2_weights(weights_g, bit1, bit0);

		/* blue component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit7 = (color_prom[i] >> 7) & 0x01;
		b = combine_3_weights(weights_b, bit7, bit1, bit0);

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}
}

static WRITE8_HANDLER( wallc_videoram_w )
{
	space->machine->generic.videoram.u8[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	SET_TILE_INFO(0, machine->generic.videoram.u8[tile_index] + 0x100, 1, 0);
}

static VIDEO_START( wallc )
{
	bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_cols_flip_y,	8, 8, 32, 32);
}

static VIDEO_UPDATE( wallc )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	return 0;
}

static WRITE8_HANDLER( wallc_coin_counter_w )
{
	coin_counter_w(space->machine, 0,data & 2);
}

static ADDRESS_MAP_START( wallc_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x83ff) AM_RAM_WRITE(wallc_videoram_w) AM_MIRROR(0xc00) AM_BASE_GENERIC(videoram)	/* 2114, 2114 */
	AM_RANGE(0xa000, 0xa3ff) AM_RAM		/* 2114, 2114 */

	AM_RANGE(0xb000, 0xb000) AM_READ_PORT("DSW1")
	AM_RANGE(0xb200, 0xb200) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xb400, 0xb400) AM_READ_PORT("DIAL")
	AM_RANGE(0xb600, 0xb600) AM_READ_PORT("DSW2")

	AM_RANGE(0xb000, 0xb000) AM_WRITENOP
	AM_RANGE(0xb100, 0xb100) AM_WRITE(wallc_coin_counter_w)
	AM_RANGE(0xb200, 0xb200) AM_WRITENOP
	AM_RANGE(0xb500, 0xb500) AM_DEVWRITE("aysnd", ay8910_address_w)
	AM_RANGE(0xb600, 0xb600) AM_DEVWRITE("aysnd", ay8910_data_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( wallc )
	PORT_START("SYSTEM")	/* b200 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 )	//Right curve button; select current playfield in test mode
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )	//not used ?
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )	//service?? plays loud,high-pitched sound
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )	//Left curve button; browse playfields in test mode
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )	//ok
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )	//ok
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN3 )	//ok
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )	//ok

	PORT_START("DIAL")		/* b400 - player position 8 bit analog input - value read is used as position of the player directly - what type of input is that ? DIAL ?*/
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(3) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("DSW1")		/* b000 */
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(	0x03, "5" )
	PORT_DIPSETTING(	0x02, "4" )
	PORT_DIPSETTING(	0x01, "3" )
	PORT_DIPSETTING(	0x00, "2" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life) )
	PORT_DIPSETTING(	0x0c, "100K/200K/400K/800K" )
	PORT_DIPSETTING(	0x08, "80K/160K/320K/640K" )
	PORT_DIPSETTING(	0x04, "60K/120K/240K/480K" )
	PORT_DIPSETTING(	0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x10, 0x00, "Curve Effect" )
	PORT_DIPSETTING(	0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(	0x00, "More" )
	PORT_DIPNAME( 0x60, 0x60, "Timer Speed" )
	PORT_DIPSETTING(	0x60, "Slow" )
	PORT_DIPSETTING(	0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(	0x20, "Fast" )
	PORT_DIPSETTING(	0x00, "Super Fast" )
	PORT_DIPNAME( 0x80, 0x00, "Service" )
	PORT_DIPSETTING(	0x80, "Free Play With Level Select" )
	PORT_DIPSETTING(	0x00, DEF_STR( Normal ) )

	PORT_START("DSW2")		/* b600 */
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x30, 0x00, "Coin C" )
	PORT_DIPSETTING(    0x30, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_5C ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,	/* 8*8 characters */
	RGN_FRAC(1,3),
	3,	/* 3 bits per pixel */
	{ RGN_FRAC(0,3),RGN_FRAC(1,3),RGN_FRAC(2,3) }, /* the bitplanes are separated */
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};

static GFXDECODE_START( wallc )
	GFXDECODE_ENTRY( "gfx1", 0     , charlayout, 0, 4 )
GFXDECODE_END

static DRIVER_INIT( wallc )
{
	UINT8 c;
	UINT32 i;

	UINT8 *ROM = memory_region(machine, "maincpu");

	for (i=0; i<0x2000*2; i++)
	{
		c = ROM[ i ] ^ 0x55 ^ 0xff; /* NOTE: this can be shortened but now it fully reflects what the bigger module really does */
		c = BITSWAP8(c, 4,2,6,0,7,1,3,5); /* also swapped inside of the bigger module */
		ROM[ i ] = c;
	}
}

static DRIVER_INIT( wallca )
{
	UINT8 c;
	UINT32 i;

	UINT8 *ROM = memory_region(machine, "maincpu");

	for (i=0; i<0x4000; i++)
	{
		if(i & 0x100)
		{
			c = ROM[ i ] ^ 0x4a;
			c = BITSWAP8(c, 4,7,1,3,2,0,5,6);
		}
		else
		{
			c = ROM[ i ] ^ 0xa5;
			c = BITSWAP8(c, 0,2,3,6,1,5,7,4);
		}

		ROM[ i ] = c;
	}
}



static MACHINE_DRIVER_START( wallc )
	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, 12288000 / 4)	/* 3.072 MHz ? */
	MDRV_CPU_PROGRAM_MAP(wallc_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)

	MDRV_GFXDECODE(wallc)
	MDRV_PALETTE_LENGTH(32)

	MDRV_PALETTE_INIT(wallc)
	MDRV_VIDEO_START(wallc)
	MDRV_VIDEO_UPDATE(wallc)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("aysnd", AY8910, 12288000 / 8)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_DRIVER_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( wallc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wac05.h7",   0x0000, 0x2000, CRC(ab6e472e) SHA1(a387fec24fb899df349a35d1d3a91e897b074712) )
	ROM_LOAD( "wac1-52.h6", 0x2000, 0x2000, CRC(988eaa6d) SHA1(d5e5dbee6e7e0488fdecfb864198c686cbd5d59c) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_LOAD( "wc1.e3",		0x0000, 0x1000, CRC(ca5c4b53) SHA1(5d2e14fe81cca4ec7dbe0c98eaa26890fca28e58) )
	ROM_LOAD( "wc2.e2",		0x1000, 0x1000, CRC(b7f52a59) SHA1(737e7616d7295762057fbdb69d65c8c1edc773dc) )
	ROM_LOAD( "wc3.e1",		0x2000, 0x1000, CRC(f6854b3a) SHA1(bc1e7f785c338c1afa4ab61c07c61397b3de0b01) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "74s288.c2",  0x0000, 0x0020, CRC(83e3e293) SHA1(a98c5e63b688de8d175adb6539e0cdc668f313fd) )
ROM_END

/* this set uses a different encryption, but the decrypted code is the same */
ROM_START( wallca )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rom4.rom",     0x0000, 0x2000, CRC(ce43af1b) SHA1(c05419cb4aa57c6187b469573a3787d9123c4a05) )
	ROM_LOAD( "rom5.rom",     0x2000, 0x2000, CRC(b789a705) SHA1(2b62b14d1a3ad5eff5b8d502d7891e58379ee820) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_LOAD( "rom3.rom",     0x0800, 0x0800, CRC(6634db73) SHA1(fe6104f974495a250e0cd14c0745eec8e44b8d3a) )
	ROM_LOAD( "rom2.rom",     0x1800, 0x0800, CRC(79f49c2c) SHA1(485fdba5ebdb4c01306f3ef26c992a513aa6b5dc) )
	ROM_LOAD( "rom1.rom",     0x2800, 0x0800, CRC(3884fd4f) SHA1(47254c8828128ac48fc15f05b52fe4d42d4919e7) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "74s288.c2",  0x0000, 0x0020, CRC(83e3e293) SHA1(a98c5e63b688de8d175adb6539e0cdc668f313fd) )
ROM_END

GAME( 1984, wallc,  0,      wallc,  wallc, wallc,  ROT0, "Midcoin", "Wall Crash (set 1)", 0 )
GAME( 1984, wallca, wallc,  wallc,  wallc, wallca, ROT0, "Midcoin", "Wall Crash (set 2)", 0 )
