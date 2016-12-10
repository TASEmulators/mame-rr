/*

Ping Pong Masters '93
Electronic Devices, 1993

PCB Layout
----------

|----------------------------------------------|
|                                              |
|          2018                                |
|          1.UE7                 YM2413        |
|                    |----------|              |
|          Z80       |Unknown   | 24MHz   PROM1|
|                    |PLCC84    |              |
|J  DSW2             |          |         PROM2|
|A           2.UP7   |          |      PAL     |
|M                   |          | PAL     PROM3|
|M  DSW1      2018   |----------|              |
|A                                   2018      |
|             PAL                    2018      |
|                          5MHz                |
|                                              |
|                                              |
|                                              |
|          Z80             3.UG16     4.UG15   |
|----------------------------------------------|
Notes:
      Z80 clock    : 5.000MHz (both)
      YM2413 clock : 2.500MHz (5/2)
      VSync        : 55Hz


Dip Switch Settings
-------------------

SW1                1     2     3     4     5     6     7     8
|-------|-------|-----|-----|-----|-----|-----|-----|-----|-----|
|Coin1  | 1C 1P | OFF | OFF | OFF | OFF |     |     |     |     |
|       |-------|-----|-----|-----|-----|-----|-----|-----|-----|
|       | 1C 2P | ON  | OFF | OFF | OFF |     |     |     |     |
|       |-------|-----|-----|-----|-----|-----|-----|-----|-----|
|       | 1C 3P | OFF | ON  | OFF | OFF |     |     |     |     |
|       |-------|-----|-----|-----|-----|-----|-----|-----|-----|
|       | 1C 4P | ON  | ON  | OFF | OFF |     |     |     |     |
|       |-------|-----|-----|-----|-----|-----|-----|-----|-----|
|       | 1C 5P | OFF | OFF | ON  | OFF |     |     |     |     |
|       |-------|-----|-----|-----|-----|-----|-----|-----|-----|
|       | 2C 1P | ON  | OFF | ON  | OFF |     |     |     |     |
|       |-------|-----|-----|-----|-----|-----|-----|-----|-----|
|       | 2C 3P | OFF | ON  | ON  | OFF |     |     |     |     |
|       |-------|-----|-----|-----|-----|-----|-----|-----|-----|
|       | 2C 5P | ON  | ON  | ON  | OFF |     |     |     |     |
|       |-------|-----|-----|-----|-----|-----|-----|-----|-----|
|       | 3C 1P | OFF | OFF | OFF | ON  |     |     |     |     |
|       |-------|-----|-----|-----|-----|-----|-----|-----|-----|
|       | 3C 2P | ON  | OFF | OFF | ON  |     |     |     |     |
|       |-------|-----|-----|-----|-----|-----|-----|-----|-----|
|       | 3C 5P | OFF | ON  | OFF | ON  |     |     |     |     |
|       |-------|-----|-----|-----|-----|-----|-----|-----|-----|
|       | 4C 1P | ON  | ON  | OFF | ON  |     |     |     |     |
|       |-------|-----|-----|-----|-----|-----|-----|-----|-----|
|       | 4C 3P | OFF | OFF | ON  | ON  |     |     |     |     |
|       |-------|-----|-----|-----|-----|-----|-----|-----|-----|
|       | 4C 5P | ON  | OFF | ON  | ON  |     |     |     |     |
|       |-------|-----|-----|-----|-----|-----|-----|-----|-----|
|       | 5C 1P | OFF | ON  | ON  | ON  |     |     |     |     |
|       |-------|-----|-----|-----|-----|-----|-----|-----|-----|
|       | 5C 2P | ON  | ON  | ON  | ON  |     |     |     |     |
|-------|-------|-----|-----|-----|-----|-----|-----|-----|-----|
|Coin2  | 1C 1P |     |     |     |     | OFF | OFF | OFF | OFF |
|       |-------|-----|-----|-----|-----|-----|-----|-----|-----|
|       | 1C 2P |     |     |     |     | ON  | OFF | OFF | OFF |
|       |-------|-----|-----|-----|-----|-----|-----|-----|-----|
|       | 1C 3P |     |     |     |     | OFF | ON  | OFF | OFF |
|       |-------|-----|-----|-----|-----|-----|-----|-----|-----|
|       | 1C 4P |     |     |     |     | ON  | ON  | OFF | OFF |
|       |-------|-----|-----|-----|-----|-----|-----|-----|-----|
|       | 1C 5P |     |     |     |     | OFF | OFF | ON  | OFF |
|       |-------|-----|-----|-----|-----|-----|-----|-----|-----|
|       | 2C 1P |     |     |     |     | ON  | OFF | ON  | OFF |
|       |-------|-----|-----|-----|-----|-----|-----|-----|-----|
|       | 2C 3P |     |     |     |     | OFF | ON  | ON  | OFF |
|       |-------|-----|-----|-----|-----|-----|-----|-----|-----|
|       | 2C 5P |     |     |     |     | ON  | ON  | ON  | OFF |
|       |-------|-----|-----|-----|-----|-----|-----|-----|-----|
|       | 3C 1P |     |     |     |     | OFF | OFF | OFF | ON  |
|       |-------|-----|-----|-----|-----|-----|-----|-----|-----|
|       | 3C 2P |     |     |     |     | ON  | OFF | OFF | ON  |
|       |-------|-----|-----|-----|-----|-----|-----|-----|-----|
|       | 3C 5P |     |     |     |     | OFF | ON  | OFF | ON  |
|       |-------|-----|-----|-----|-----|-----|-----|-----|-----|
|       | 4C 1P |     |     |     |     | ON  | ON  | OFF | ON  |
|       |-------|-----|-----|-----|-----|-----|-----|-----|-----|
|       | 4C 3P |     |     |     |     | OFF | OFF | ON  | ON  |
|       |-------|-----|-----|-----|-----|-----|-----|-----|-----|
|       | 4C 5P |     |     |     |     | ON  | OFF | ON  | ON  |
|       |-------|-----|-----|-----|-----|-----|-----|-----|-----|
|       | 5C 1P |     |     |     |     | OFF | ON  | ON  | ON  |
|       |-------|-----|-----|-----|-----|-----|-----|-----|-----|
|       | 5C 2P |     |     |     |     | ON  | ON  | ON  | ON  |
|-------|-------|-----|-----|-----|-----|-----|-----|-----|-----|

SW2                1     2     3     4     5     6     7     8
|-------|-------|-----|-----|-----|-----|-----|-----|-----|-----|
|Diffic-| Easy  | OFF | OFF |     |     |     |     |     |     |
|ulty   |-------|-----|-----|-----|-----|-----|-----|-----|-----|
|       | Normal| ON  | OFF |     |     |     |     |     |     |
|       |-------|-----|-----|-----|-----|-----|-----|-----|-----|
|       | Hard  | OFF | ON  |     |     |     |     |     |     |
|       |-------|-----|-----|-----|-----|-----|-----|-----|-----|
|       | V.Hard| ON  | ON  |     |     |     |     |     |     |
|-------|-------|-----|-----|-----|-----|-----|-----|-----|-----|
|Demo   |Without|     |     | OFF |     |     |     |     |     |
|Sound  |-------|-----|-----|-----|-----|-----|-----|-----|-----|
|       | With  |     |     | ON  |     |     |     |     |     |
|-------|-------|-----|-----|-----|-----|-----|-----|-----|-----|
|Test   | No    |     |     |     | OFF |     |     |     |     |
|Mode   |-------|-----|-----|-----|-----|-----|-----|-----|-----|
|       | Yes   |     |     |     | ON  |     |     |     |     |
|-------|-------|-----|-----|-----|-----|-----|-----|-----|-----|

The DIP sheet also seems to suggest the use of a 4-way joystick and 2 buttons,
one for shoot and one for select.


2008-08
Dip locations added based on the notes above.

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/3812intf.h"
#include "sound/2413intf.h"
#include "sound/dac.h"


static tilemap_t *ppmast93_fg_tilemap, *ppmast93_bg_tilemap;
static UINT8 *ppmast93_fgram, *ppmast93_bgram;

static WRITE8_HANDLER( ppmast93_fgram_w )
{
	ppmast93_fgram[offset] = data;
	tilemap_mark_tile_dirty(ppmast93_fg_tilemap,offset/2);
}

static WRITE8_HANDLER( ppmast93_bgram_w )
{
	ppmast93_bgram[offset] = data;
	tilemap_mark_tile_dirty(ppmast93_bg_tilemap,offset/2);
}

static WRITE8_HANDLER( ppmast93_port4_w )
{
	UINT8 *rom = memory_region(space->machine, "maincpu");
	int bank;

	coin_counter_w(space->machine, 0, data & 0x08);
	coin_counter_w(space->machine, 1, data & 0x10);

	bank = data & 0x07;
	memory_set_bankptr(space->machine, "bank1",&rom[0x10000+(bank*0x4000)]);
}

static ADDRESS_MAP_START( ppmast93_cpu1_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM AM_WRITENOP AM_REGION("maincpu", 0x10000)
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xd000, 0xd7ff) AM_RAM_WRITE(ppmast93_bgram_w) AM_BASE(&ppmast93_bgram) AM_SHARE("share1")
	AM_RANGE(0xd800, 0xdfff) AM_WRITENOP
	AM_RANGE(0xf000, 0xf7ff) AM_RAM_WRITE(ppmast93_fgram_w) AM_BASE(&ppmast93_fgram) AM_SHARE("share2")
	AM_RANGE(0xf800, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( ppmast93_cpu1_io, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("P1")
	AM_RANGE(0x02, 0x02) AM_READ_PORT("P2")
	AM_RANGE(0x04, 0x04) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x06, 0x06) AM_READ_PORT("DSW1")
	AM_RANGE(0x08, 0x08) AM_READ_PORT("DSW2")

	AM_RANGE(0x00, 0x00) AM_WRITE(soundlatch_w)
	AM_RANGE(0x04, 0x04) AM_WRITE(ppmast93_port4_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( ppmast93_cpu2_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xfbff) AM_ROM AM_REGION("sub", 0x10000)
	AM_RANGE(0xfc00, 0xfc00) AM_READ(soundlatch_r)
	AM_RANGE(0xfd00, 0xffff) AM_RAM
ADDRESS_MAP_END


static WRITE8_HANDLER(ppmast_sound_w)
{
	switch(offset&0xff)
	{
		case 0:
		case 1: ym2413_w(space->machine->device("ymsnd"),offset,data); break;
		case 2: dac_data_w(space->machine->device("dac"),data);break;
		default: logerror("%x %x - %x\n",offset,data,cpu_get_previouspc(space->cpu));
	}
}

static ADDRESS_MAP_START( ppmast93_cpu2_io, ADDRESS_SPACE_IO, 8 )
	  AM_RANGE(0x0000, 0xffff) AM_ROM AM_WRITE(ppmast_sound_w) AM_REGION("sub", 0x20000)
ADDRESS_MAP_END

static INPUT_PORTS_START( ppmast93 )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) // nothing?
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) // nothing?
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL ) // or it always goes to test mode
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x01, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x00, "5 Coins/2 Credits" )
	PORT_DIPSETTING(    0x0a, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x05, "3 Coins/5 Credits" )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(    0x10, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x00, "5 Coins/2 Credits" )
	PORT_DIPSETTING(    0xa0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x50, "3 Coins/5 Credits" )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW2:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW2:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW2:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW2:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW2:8" )
INPUT_PORTS_END

static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 2, 4, 6 },
	{ 1, 0, 9, 8, 17, 16, 25, 24 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static GFXDECODE_START( ppmast93 )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0, 16 )
GFXDECODE_END

static TILE_GET_INFO( get_ppmast93_bg_tile_info )
{
	int code = (ppmast93_bgram[tile_index*2+1] << 8) | ppmast93_bgram[tile_index*2];
	SET_TILE_INFO(
			0,
			code & 0x0fff,
			(code & 0xf000) >> 12,
			0);
}

static TILE_GET_INFO( get_ppmast93_fg_tile_info )
{
	int code = (ppmast93_fgram[tile_index*2+1] << 8) | ppmast93_fgram[tile_index*2];
	SET_TILE_INFO(
			0,
			(code & 0x0fff)+0x1000,
			(code & 0xf000) >> 12,
			0);
}

static VIDEO_START( ppmast93 )
{
	ppmast93_bg_tilemap = tilemap_create(machine, get_ppmast93_bg_tile_info,tilemap_scan_rows,8,8,32, 32);
	ppmast93_fg_tilemap = tilemap_create(machine, get_ppmast93_fg_tile_info,tilemap_scan_rows,8,8,32, 32);

	tilemap_set_transparent_pen(ppmast93_fg_tilemap,0);
}

static VIDEO_UPDATE( ppmast93 )
{
	tilemap_draw(bitmap,cliprect,ppmast93_bg_tilemap,0,0);
	tilemap_draw(bitmap,cliprect,ppmast93_fg_tilemap,0,0);
	return 0;
}

static MACHINE_DRIVER_START( ppmast93 )
	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80,5000000)		 /* 5 MHz */
	MDRV_CPU_PROGRAM_MAP(ppmast93_cpu1_map)
	MDRV_CPU_IO_MAP(ppmast93_cpu1_io)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_CPU_ADD("sub", Z80,5000000)		 /* 5 MHz */
	MDRV_CPU_PROGRAM_MAP(ppmast93_cpu2_map)
	MDRV_CPU_IO_MAP(ppmast93_cpu2_io)
	MDRV_CPU_PERIODIC_INT(irq0_line_hold,8000)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(55)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 256-1, 0, 256-1)

	MDRV_GFXDECODE(ppmast93)

	MDRV_PALETTE_INIT(RRRR_GGGG_BBBB)
	MDRV_PALETTE_LENGTH(0x100)

	MDRV_VIDEO_START(ppmast93)
	MDRV_VIDEO_UPDATE(ppmast93)

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ymsnd", YM2413, 5000000/2)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	MDRV_SOUND_ADD("dac", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.60)

MACHINE_DRIVER_END

ROM_START( ppmast93 )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "2.up7", 0x10000, 0x20000, CRC(8854d8db) SHA1(9d93ddfb44d533772af6519747a6cb50b42065cd) )

	ROM_REGION( 0x30000, "sub", 0 )
	ROM_LOAD( "1.ue7", 0x10000, 0x20000, CRC(8e26939e) SHA1(e62441e523f5be6a3889064cc5e0f44545260e93) )

	ROM_REGION( 0x40000, "gfx1", 0 )
	ROM_LOAD( "3.ug16", 0x00000, 0x20000, CRC(8ab24641) SHA1(c0ebee90bf3fe208947ae5ea56f31469ed24d198) )
	ROM_LOAD( "4.ug15", 0x20000, 0x20000, CRC(b16e9fb6) SHA1(53aa962c63319cd649e0c8cf0c26e2308598e1aa) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "prom3.ug24", 0x000, 0x100, CRC(b1a4415a) SHA1(1dd22260f7dbdc9c812a2349069ed5f3c9c92826) )
	ROM_LOAD( "prom2.ug25", 0x100, 0x100, CRC(4b5055ba) SHA1(6213e79492d35593c643ef5c01ce6a58a77866aa) )
	ROM_LOAD( "prom1.ug26", 0x200, 0x100, CRC(d979c64e) SHA1(172c9579013d58e35a5b4f732e360811ac36295e) )
ROM_END

GAME( 1993, ppmast93, 0, ppmast93, ppmast93, 0, ROT0, "Electronic Devices S.R.L.", "Ping Pong Masters '93", GAME_IMPERFECT_SOUND )
