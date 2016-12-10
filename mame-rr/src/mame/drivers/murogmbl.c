/* Unknown - Poker (morugem

driver by Roberto Zandona'
thanks to Angelo Salese for some precious advice

TO DO:
- check sound

Anno: 1982
Produttore:
N.revisione:

CPU:
1x unknown DIP40 (1ef)(missing)
1x TBA820 (14e)(sound)
1x oscillator 12.000 (2f)

ROMs:
2x TMS2532 (5b,5e)
1x TMS2516 (8b)
1x PROM SN74S288N (8a)
1x RAM MWS5101AEL2 (11e)
4x RAM AM9114EPC (2b,3b,8e,9e)

Note:
1x 22x2 edge connector
1x trimmer (volume)
1x 8x2 switches dip
1x empty DIP14 socket (close to sound)

Funzionamento: Non Funzionante

Dumped: 06/04/2009 f205v
*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/dac.h"

static UINT8* murogmbl_video;

static PALETTE_INIT( murogmbl )
{
	int	bit0, bit1, bit2 , r, g, b;
	int	i;

	for (i = 0; i < 0x20; ++i)
	{
		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (color_prom[0] >> 3) & 0x01;
		bit1 = (color_prom[0] >> 4) & 0x01;
		bit2 = (color_prom[0] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = 0;
		bit1 = (color_prom[0] >> 6) & 0x01;
		bit2 = (color_prom[0] >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
		color_prom++;
	}
}

static ADDRESS_MAP_START( murogmbl_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fFf) AM_ROM
	AM_RANGE(0x4000, 0x43ff) AM_RAM
	AM_RANGE(0x4800, 0x4bff) AM_RAM
	AM_RANGE(0x5800, 0x5bff) AM_RAM AM_BASE(&murogmbl_video)
	AM_RANGE(0x5c00, 0x5fff) AM_RAM
	AM_RANGE(0x6000, 0x6000) AM_READ_PORT("IN0")
	AM_RANGE(0x6800, 0x6800) AM_READ_PORT("DSW")
	AM_RANGE(0x7000, 0x7000) AM_READ_PORT("IN1")
	AM_RANGE(0x7800, 0x7800) AM_READNOP AM_DEVWRITE("dac1", dac_w) /* read is always discarded */
ADDRESS_MAP_END

static VIDEO_START(murogmbl)
{

}

static VIDEO_UPDATE(murogmbl)
{
	const gfx_element *gfx = screen->machine->gfx[0];
	int count = 0;

	int y, x;

	for (y = 0; y < 32; y++)
	{
		for (x = 0; x < 32; x++)
		{
			int tile = murogmbl_video[count];
			drawgfx_opaque(bitmap, cliprect, gfx, tile, 0, 0, 0, x * 8, y * 8);

			count++;
		}
	}
	return 0;
}

static INPUT_PORTS_START( murogmbl )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POKER_CANCEL ) PORT_NAME("Clear")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_GAMBLE_BET ) PORT_NAME("Replay")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_GAMBLE_DEAL )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_POKER_HOLD5 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Coin clear") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_NAME("Coin 1")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_NAME("Coin 2")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_POKER_HOLD4 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Coin 2" )
	PORT_DIPSETTING(    0x08, "10 credits" )
	PORT_DIPSETTING(    0x00, "5 credits" )
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

static const gfx_layout layout8x8x2 =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{
		RGN_FRAC(0,2),RGN_FRAC(1,2)
	},
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static GFXDECODE_START( murogmbl )
	GFXDECODE_ENTRY( "gfx1", 0, layout8x8x2,  0x0, 1 )
GFXDECODE_END

static MACHINE_DRIVER_START( murogmbl )
	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, 1000000) /* Z80? */
	MDRV_CPU_PROGRAM_MAP(murogmbl_map)

	MDRV_GFXDECODE(murogmbl)

	MDRV_PALETTE_INIT(murogmbl)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)

	MDRV_PALETTE_LENGTH(0x100)

	MDRV_VIDEO_START(murogmbl)
	MDRV_VIDEO_UPDATE(murogmbl)

	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("dac1", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END



ROM_START(murogmbl)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("2532.5e",	0x0000, 0x1000, CRC(093d4560) SHA1(d5401b5f7a2ebe5099fefc5b09f8710886e243b2) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD("2532.8b",	0x0000, 0x0800, CRC(4427ffc0) SHA1(45f5fd0ae967cdb6abbf2e6c6d12d787556488ef) )
	ROM_CONTINUE(0x0000, 0x0800)
	ROM_LOAD("2516.5b", 	0x0800, 0x0800, CRC(496ad48c) SHA1(28380c9d02b64e7d5ef2763de92cd2ca8861eceb) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "74s288.a8",	0x0000, 0x0020, CRC(fc35201c) SHA1(4549e228c48992e0d10957f029b89a547392e72b) )
ROM_END

GAME( 1982, murogmbl,  murogem,   murogmbl, murogmbl, 0, ROT0, "bootleg?", "Muroge Monaco (bootleg?)", GAME_NO_SOUND )
