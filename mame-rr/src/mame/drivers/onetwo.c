/*

 One + Two
 (c) Barko 1997

 Driver by David Haywood and Pierpaolo Prazzoli

PCB has D.G.R.M. silkscreened on it.  Dated 1997.4.11

+----------------------------------+
|  YM3014 YM3812  Z80   24MHz      |
|   M6295 sample  sound_prog       |
|                 6116       3_grfx|
|J       6116                      |
|A COR_B 6116                4_grfx|
|M COR_G                           |
|M COR_R       A1020B              |
|A                           5_grfx|
|DSW   62256         6264          |
|     main_prog                    |
|DSW   Z80  4MHz                   |
+----------------------------------+

Goldstar Z8400A PS (4 MHz rated) both CPUs
Actel A1020B PL84C
YM3812/YM3014 (badged as UA011 & UA010)
OKI M6295

 main_prog 27c010
    x_grfx 27c040
    sample 27c020
sound_prog 27512

COR_x are LN60G resitor packs

-------------------------------------

Note: this is quite clearly a 'Korean bootleg' of Shisensho - Joshiryo-Hen / Match-It

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/okim6295.h"
#include "sound/3812intf.h"

#define MASTER_CLOCK        XTAL_4MHz

class onetwo_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, onetwo_state(machine)); }

	onetwo_state(running_machine &machine) { }

	/* memory pointers */
	UINT8 *  fgram;
	UINT8 *  paletteram;
	UINT8 *  paletteram2;

	/* video-related */
	tilemap_t *fg_tilemap;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
};



/*************************************
 *
 *  Video emulation
 *
 *************************************/

static TILE_GET_INFO( get_fg_tile_info )
{
	onetwo_state *state = (onetwo_state *)machine->driver_data;
	int code = (state->fgram[tile_index * 2 + 1] << 8) | state->fgram[tile_index * 2];
	int color = (state->fgram[tile_index * 2 + 1] & 0x80) >> 7;

	code &= 0x7fff;

	SET_TILE_INFO(0, code, color, 0);
}

static VIDEO_START( onetwo )
{
	onetwo_state *state = (onetwo_state *)machine->driver_data;
	state->fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 8, 8, 64, 32);
}

static VIDEO_UPDATE( onetwo )
{
	onetwo_state *state = (onetwo_state *)screen->machine->driver_data;
	tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0);
	return 0;
}

/*************************************
 *
 *  Memory handlers
 *
 *************************************/

static WRITE8_HANDLER( onetwo_fgram_w )
{
	onetwo_state *state = (onetwo_state *)space->machine->driver_data;
	state->fgram[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap, offset / 2);
}

static WRITE8_HANDLER( onetwo_cpubank_w )
{
	memory_set_bank(space->machine, "bank1", data);
}

static WRITE8_HANDLER( onetwo_coin_counters_w )
{
	watchdog_reset(space->machine);
	coin_counter_w(space->machine, 0, BIT(data, 1));
	coin_counter_w(space->machine, 1, BIT(data, 2));
}

static WRITE8_HANDLER( onetwo_soundlatch_w )
{
	onetwo_state *state = (onetwo_state *)space->machine->driver_data;
	soundlatch_w(space, 0, data);
	cpu_set_input_line(state->audiocpu, INPUT_LINE_NMI, PULSE_LINE);
}

static void set_color(running_machine *machine, int offset)
{
	onetwo_state *state = (onetwo_state *)machine->driver_data;
	int r, g, b;

	r = state->paletteram[offset] & 0x1f;
	g = state->paletteram2[offset] & 0x1f;
	b = ((state->paletteram[offset] & 0x60) >> 2) | ((state->paletteram2[offset] & 0xe0) >> 5);
	palette_set_color_rgb(machine, offset, pal5bit(r), pal5bit(g), pal5bit(b));
}

static WRITE8_HANDLER(palette1_w)
{
	onetwo_state *state = (onetwo_state *)space->machine->driver_data;
	state->paletteram[offset] = data;
	set_color(space->machine, offset);
}

static WRITE8_HANDLER(palette2_w)
{
	onetwo_state *state = (onetwo_state *)space->machine->driver_data;
	state->paletteram2[offset] = data;
	set_color(space->machine, offset);
}

/*************************************
 *
 *  Address maps
 *
 *************************************/

static ADDRESS_MAP_START( main_cpu, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM AM_REGION("maincpu", 0x10000)
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc800, 0xc87f) AM_RAM_WRITE(palette1_w) AM_BASE_MEMBER(onetwo_state, paletteram)
	AM_RANGE(0xc900, 0xc97f) AM_RAM_WRITE(palette2_w) AM_BASE_MEMBER(onetwo_state, paletteram2)
	AM_RANGE(0xd000, 0xdfff) AM_RAM_WRITE(onetwo_fgram_w) AM_BASE_MEMBER(onetwo_state, fgram)
	AM_RANGE(0xe000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( main_cpu_io, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("DSW1") AM_WRITE(onetwo_coin_counters_w)
	AM_RANGE(0x01, 0x01) AM_READ_PORT("DSW2") AM_WRITE(onetwo_soundlatch_w)
	AM_RANGE(0x02, 0x02) AM_READ_PORT("P1") AM_WRITE(onetwo_cpubank_w)
	AM_RANGE(0x03, 0x03) AM_READ_PORT("P2")
	AM_RANGE(0x04, 0x04) AM_READ_PORT("SYSTEM")
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_cpu, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0xf000, 0xf7ff) AM_RAM
	AM_RANGE(0xf800, 0xf800) AM_READ(soundlatch_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_cpu_io, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_DEVREADWRITE("ymsnd", ym3812_status_port_r, ym3812_control_port_w)
	AM_RANGE(0x20, 0x20) AM_DEVWRITE("ymsnd", ym3812_write_port_w)
	AM_RANGE(0x40, 0x40) AM_DEVREADWRITE("oki", okim6295_r, okim6295_w)
	AM_RANGE(0xc0, 0xc0) AM_WRITE(soundlatch_clear_w)
ADDRESS_MAP_END

/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( onetwo )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, "Timer" ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x0c, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coinage ) ) PORT_CONDITION("DSW2",0x04,PORTCOND_EQUALS,0x04) PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(    0xa0, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 8C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) ) PORT_CONDITION("DSW2",0x04,PORTCOND_NOTEQUALS,0x04) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_B ) ) PORT_CONDITION("DSW2",0x04,PORTCOND_NOTEQUALS,0x04) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown) ) PORT_DIPLOCATION("SW2:1") /* Flip Sreen? */
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Coin Chute" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, "Common" )
	PORT_DIPSETTING(    0x00, "Separate" )
	PORT_DIPNAME( 0x08, 0x08, "Nude Pictures" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Women Select" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	/* In stop mode, press 2 to stop and 1 to restart */
	PORT_DIPNAME( 0x20, 0x20, "Stop Mode (Cheat)") PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Play Mode" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, "1 Player" )
	PORT_DIPSETTING(    0x40, "2 Player" )
	PORT_DIPNAME( 0x80, 0x00, "Freeze" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout tiles8x8x6_layout =
{
	8,8,
	RGN_FRAC(1,3),
	6,
	{ RGN_FRAC(2,3)+0, RGN_FRAC(2,3)+4, RGN_FRAC(0,3)+0, RGN_FRAC(0,3)+4, RGN_FRAC(1,3)+0, RGN_FRAC(1,3)+4 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8
};

static GFXDECODE_START( onetwo )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8x6_layout, 0, 2 )
GFXDECODE_END

/*************************************
 *
 *  Sound interface
 *
 *************************************/

static void irqhandler(running_device *device, int linestate)
{
	onetwo_state *state = (onetwo_state *)device->machine->driver_data;
	cpu_set_input_line(state->audiocpu, 0, linestate);
}

static const ym3812_interface ym3812_config =
{
	irqhandler	/* IRQ Line */
};

/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_START( onetwo )
{
	onetwo_state *state = (onetwo_state *)machine->driver_data;
	UINT8 *ROM = memory_region(machine, "maincpu");

	memory_configure_bank(machine, "bank1", 0, 8, &ROM[0x10000], 0x4000);

	state->maincpu = machine->device("maincpu");
	state->audiocpu = machine->device("audiocpu");
}

static MACHINE_DRIVER_START( onetwo )

	/* driver data */
	MDRV_DRIVER_DATA(onetwo_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80,MASTER_CLOCK)	/* 4 MHz */
	MDRV_CPU_PROGRAM_MAP(main_cpu)
	MDRV_CPU_IO_MAP(main_cpu_io)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_CPU_ADD("audiocpu", Z80,MASTER_CLOCK)	/* 4 MHz */
	MDRV_CPU_PROGRAM_MAP(sound_cpu)
	MDRV_CPU_IO_MAP(sound_cpu_io)

	MDRV_MACHINE_START(onetwo)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(16))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(512, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-1)

	MDRV_GFXDECODE(onetwo)
	MDRV_PALETTE_LENGTH(0x80)

	MDRV_VIDEO_START(onetwo)
	MDRV_VIDEO_UPDATE(onetwo)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ymsnd", YM3812, MASTER_CLOCK)
	MDRV_SOUND_CONFIG(ym3812_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MDRV_OKIM6295_ADD("oki", 1056000*2, OKIM6295_PIN7_LOW) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END

/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( onetwo )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* main z80 */
	ROM_LOAD( "maincpu", 0x10000,  0x20000, CRC(83431e6e) SHA1(61ab386a1d0af050f091f5df28c55ad5ad1a0d4b) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* sound z80 */
	ROM_LOAD( "sound_prog",  0x00000,  0x10000, CRC(90aba4f3) SHA1(914b1c8684993ddc7200a3d61e07f4f6d59e9d02) )

	ROM_REGION( 0x180000, "gfx1", 0 )
	ROM_LOAD( "3_graphics", 0x000000, 0x80000, CRC(c72ff3a0) SHA1(17394d8a8b5ef4aee9522d87ba92ef1285f4d76a) )
	ROM_LOAD( "4_graphics", 0x080000, 0x80000, CRC(0ca40557) SHA1(ca2db57d64ece90f2066f15b276c8d5827dcb4fa) )
	ROM_LOAD( "5_graphics",   0x100000, 0x80000, CRC(664b6679) SHA1(f9f78bd34fb58e24f890a540382392e1c9d01220) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "sample", 0x000000, 0x40000, CRC(b10d3132) SHA1(42613e17b6a1300063b8355596a2dc7bcd903777) )
ROM_END

ROM_START( onetwoe )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* main z80 */
	ROM_LOAD( "main_prog", 0x10000,  0x20000, CRC(6c1936e9) SHA1(d8fb3056299c9b45e0b537e77dc0d633882705dd) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* sound z80 */
	ROM_LOAD( "sound_prog",  0x00000,  0x10000, CRC(90aba4f3) SHA1(914b1c8684993ddc7200a3d61e07f4f6d59e9d02) )

	ROM_REGION( 0x180000, "gfx1", 0 )
	ROM_LOAD( "3_grfx", 0x000000, 0x80000, CRC(0f9f39ff) SHA1(85d107306c8c5718da3b751221791404cfe12a3d) )
	ROM_LOAD( "4_grfx", 0x080000, 0x80000, CRC(2b0e0564) SHA1(092bf0bb7be12ed1aa8a4ed1e88143ea88819497) )
	ROM_LOAD( "5_grfx", 0x100000, 0x80000, CRC(69807a9b) SHA1(6c1d79e86e3575da29bc299670e38019eef53493) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "sample", 0x000000, 0x40000, CRC(b10d3132) SHA1(42613e17b6a1300063b8355596a2dc7bcd903777) )
ROM_END

/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1997, onetwo,       0, onetwo, onetwo, 0, ROT0, "Barko", "One + Two", GAME_SUPPORTS_SAVE )
GAME( 1997, onetwoe, onetwo, onetwo, onetwo, 0, ROT0, "Barko", "One + Two (earlier)", GAME_SUPPORTS_SAVE )
