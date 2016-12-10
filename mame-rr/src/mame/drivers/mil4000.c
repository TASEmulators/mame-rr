/*******************************************************************************************

Millenium Nuovo 4000 / Nuovo Millenium 4000 (c) 2000 Sure Milano

driver by David Haywood and Angelo Salese


Notes:

- At first start-up,an Italian msg pops up: "(translated) pcb has been hacked from external
  agent,it's advised to add an anti-spark device". Press F2 to enter Service Mode,then press
  Hold 5 to exit Service Mode.

- This game is supposed to have 3 kind of graphics: billiard/pool balls, numbers and cans.
  If you go to the settings mode (F2), and then in "Impostazioni del Gioco" you disable all
  3 graphics (Simboli Biliardo, Simboli Numeri, and Simboli Barattoli --> "Non Abilitato"),
  The game will start using normal poker cards. A "illegal easter egg"... ;-)

- HW name (stamped on the pcb) is "CHP4";


TODO:

- Add Touch Screen support;
- H/V-blank bits emulation;
- Protection PIC is unused?


============================================================================================

Manufacturer: Sure
Revision number: CHP4 1.5

CPU
1x PIC16C65B (u60)(read protected)
1x MC68HC000FN12 (u61)
1x U6295 (u53)(equivalent to M6295)
1x resonator 1000j (close to 6295)
1x oscillator 12.000MHz
1x oscillator 14.31818MHz

ROMs
1x 27C020 (1)
7x V29C51001T (2,3,4,5,6,27,28)
1x PALCE22V10H (u74)(read protected)
2x A40MX04-PL840010 (u2,u3)(read protected)

RAM:
4x CY62256L-70PC - 32K x 8 Static RAM.
2x CY7C199-15PC  - 32K x 8 Static RAM

Note
1x 28x2 edge connector
1x RS232 9pins connector
1x trimmer (volume)
1x 2positon jumper
1x pushbutton (reset)

============================================================================================

CHAMPION 4000 V 1.4
(CMP4 1.3 on PCB)
12.000000MHz
14.31818MHz
1000J
MC68HC000FN12
PIC16C74B
PALCE22V10H
U6295

============================================================================================

Changes (2008-12-10, Roberto Fresca):

- Completed normal Inputs/Outputs.
- Added button-lamps calculation.
- Created button-lamps layout.
- Documented the PCB RAM.
- Fixed NVRAM size based on PCB picture (2x CY62256L-70PC near the battery).
- Added notes about the method to make appear the real poker cards.
- Fixed the OKI 6295 frequency (1000 kHz resonator near). Now the game has more decent sounds.
- Corrected CPU clock to 12 MHz. (main Xtal).


*******************************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"
#include "mil4000.lh"


static UINT16 *sc0_vram,*sc1_vram,*sc2_vram,*sc3_vram;
static tilemap_t *sc0_tilemap,*sc1_tilemap,*sc2_tilemap,*sc3_tilemap;

static TILE_GET_INFO( get_sc0_tile_info )
{
	UINT32 data = (sc0_vram[tile_index*2]<<16) | sc0_vram[tile_index*2+1];
	int tile = data >> 14;
	int color = (sc0_vram[tile_index*2+1] & 0x1f)+0;

	SET_TILE_INFO(
			0,
			tile,
			color,
			0);
}

static TILE_GET_INFO( get_sc1_tile_info )
{
	UINT32 data = (sc1_vram[tile_index*2]<<16) | sc1_vram[tile_index*2+1];
	int tile = data >> 14;
	int color = (sc1_vram[tile_index*2+1] & 0x1f)+0x10;

	SET_TILE_INFO(
			0,
			tile,
			color,
			0);
}

static TILE_GET_INFO( get_sc2_tile_info )
{
	UINT32 data = (sc2_vram[tile_index*2]<<16) | sc2_vram[tile_index*2+1];
	int tile = data >> 14;
	int color = (sc2_vram[tile_index*2+1] & 0x1f)+0x20;

	SET_TILE_INFO(
			0,
			tile,
			color,
			0);
}

static TILE_GET_INFO( get_sc3_tile_info )
{
	UINT32 data = (sc3_vram[tile_index*2]<<16) | sc3_vram[tile_index*2+1];
	int tile = data >> 14;
	int color = (sc3_vram[tile_index*2+1] & 0x1f)+0x30;

	SET_TILE_INFO(
			0,
			tile,
			color,
			0);
}

static VIDEO_START(mil4000)
{
	const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);
	int i;

	// game doesn't clear the palette, so to avoid seeing mame defaults we clear it
	for (i=0;i<0x800;i++)
		palette_set_color(space->machine, i, MAKE_RGB(0, 0, 0));

	sc0_tilemap = tilemap_create(machine, get_sc0_tile_info,tilemap_scan_rows,8,8,64,64);
	sc1_tilemap = tilemap_create(machine, get_sc1_tile_info,tilemap_scan_rows,8,8,64,64);
	sc2_tilemap = tilemap_create(machine, get_sc2_tile_info,tilemap_scan_rows,8,8,64,64);
	sc3_tilemap = tilemap_create(machine, get_sc3_tile_info,tilemap_scan_rows,8,8,64,64);

	tilemap_set_transparent_pen(sc1_tilemap,0);
	tilemap_set_transparent_pen(sc2_tilemap,0);
	tilemap_set_transparent_pen(sc3_tilemap,0);
}

static VIDEO_UPDATE(mil4000)
{
	tilemap_draw(bitmap,cliprect,sc0_tilemap,0,0);
	tilemap_draw(bitmap,cliprect,sc1_tilemap,0,0);
	tilemap_draw(bitmap,cliprect,sc2_tilemap,0,0);
	tilemap_draw(bitmap,cliprect,sc3_tilemap,0,0);
	return 0;
}

/*TODO*/
static READ16_HANDLER( hvretrace_r )
{
	static UINT16 res;
	static UINT16 vblank = 0,hblank = 0;

	res = 0;

	vblank^=1;
	hblank^=1;

	/*V-Blank*/
	if (vblank)
		res|= 0x80;

	/*H-Blank*/
	if (hblank)
		res|= 0x40;

	return res;
}


static WRITE16_HANDLER( sc0_vram_w )
{
	sc0_vram[offset] = data;
	tilemap_mark_tile_dirty(sc0_tilemap,offset/2);
}

static WRITE16_HANDLER( sc1_vram_w )
{
	sc1_vram[offset] = data;
	tilemap_mark_tile_dirty(sc1_tilemap,offset/2);
}

static WRITE16_HANDLER( sc2_vram_w )
{
	sc2_vram[offset] = data;
	tilemap_mark_tile_dirty(sc2_tilemap,offset/2);
}

static WRITE16_HANDLER( sc3_vram_w )
{
	sc3_vram[offset] = data;
	tilemap_mark_tile_dirty(sc3_tilemap,offset/2);
}

/*end of video stuff*/

/*
    --x- ---- ---- ---- Coin Counter
    ---- ---- -x-- ---- Prize
    ---- ---- --x- ---- Start
    ---- ---- ---x ---- Hold 5
    ---- ---- ---- x--- Hold 4
    ---- ---- ---- -x-- Hold 3
    ---- ---- ---- --x- Hold 2
    ---- ---- ---- ---x Hold 1
*/
static WRITE16_HANDLER( output_w )
{
	static int i;

	for(i=0;i<3;i++)
		coin_counter_w(space->machine, i, data & 0x2000);

	output_set_lamp_value(0, (data) & 1);		/* HOLD1 */
	output_set_lamp_value(1, (data >> 1) & 1);	/* HOLD2 */
	output_set_lamp_value(2, (data >> 2) & 1);	/* HOLD3 */
	output_set_lamp_value(3, (data >> 3) & 1);	/* HOLD4 */
	output_set_lamp_value(4, (data >> 4) & 1);	/* HOLD5 */
	output_set_lamp_value(5, (data >> 5) & 1);	/* START */
	output_set_lamp_value(6, (data >> 6) & 1);	/* PREMIO */

//  popmessage("%04x\n",data);
}

static ADDRESS_MAP_START( mil4000_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x500000, 0x503fff) AM_RAM_WRITE(sc0_vram_w) AM_BASE(&sc0_vram)	// CY62256L-70, U77
	AM_RANGE(0x504000, 0x507fff) AM_RAM_WRITE(sc1_vram_w) AM_BASE(&sc1_vram)	// CY62256L-70, U77
	AM_RANGE(0x508000, 0x50bfff) AM_RAM_WRITE(sc2_vram_w) AM_BASE(&sc2_vram)	// CY62256L-70, U78
	AM_RANGE(0x50c000, 0x50ffff) AM_RAM_WRITE(sc3_vram_w) AM_BASE(&sc3_vram)	// CY62256L-70, U78

	AM_RANGE(0x708000, 0x708001) AM_READ_PORT("IN0")
	AM_RANGE(0x708002, 0x708003) AM_READ_PORT("IN1")
	AM_RANGE(0x708004, 0x708005) AM_READ(hvretrace_r)
	AM_RANGE(0x708006, 0x708007) AM_READ_PORT("IN2")
	AM_RANGE(0x708008, 0x708009) AM_WRITE(output_w)
	AM_RANGE(0x708010, 0x708011) AM_NOP //touch screen
	AM_RANGE(0x70801e, 0x70801f) AM_DEVREADWRITE8("oki", okim6295_r, okim6295_w, 0x00ff)

	AM_RANGE(0x780000, 0x780fff) AM_RAM_WRITE(paletteram16_RRRRRGGGGGBBBBBx_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0xff0000, 0xffffff) AM_RAM AM_BASE_SIZE_GENERIC(nvram) // 2x CY62256L-70 (U7 & U8).

ADDRESS_MAP_END

static INPUT_PORTS_START( mil4000 )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x0040, IP_ACTIVE_LOW )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Premio") PORT_CODE(KEYCODE_T)	//premio / prize (ticket?)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static const gfx_layout tilelayout =
{
	8,8,    /* 8*8 characters */
	RGN_FRAC(1,5),
	5,
	{  RGN_FRAC(0,5), RGN_FRAC(1,5), RGN_FRAC(2,5),RGN_FRAC(3,5),RGN_FRAC(4,5) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


static GFXDECODE_START( mil4000 )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout,     0, 0x800/32 )
GFXDECODE_END


static MACHINE_DRIVER_START( mil4000 )
	MDRV_CPU_ADD("maincpu", M68000, 12000000 )	// ?
	MDRV_CPU_PROGRAM_MAP(mil4000_map)
	// irq 2/4/5 point to the same place, others invalid
	MDRV_CPU_VBLANK_INT("screen", irq5_line_hold)

	MDRV_NVRAM_HANDLER(generic_0fill)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(320, 240)
	MDRV_SCREEN_VISIBLE_AREA(0, 320-1, 0, 240-1)

	MDRV_PALETTE_LENGTH(0x800)
	MDRV_GFXDECODE(mil4000)
	MDRV_VIDEO_START(mil4000)
	MDRV_VIDEO_UPDATE(mil4000)

	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_OKIM6295_ADD("oki", 1000000, OKIM6295_PIN7_HIGH) // frequency from 1000 kHz resonator. pin 7 high not verified.
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END




ROM_START( mil4000 )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "9.u75", 0x000001, 0x20000, CRC(e3e520df) SHA1(16ee86deb75bd711c846a647e3a0a4293b5685a8) )
	ROM_LOAD16_BYTE( "10.u76", 0x000000, 0x20000, CRC(9020e19a) SHA1(e9ba0b69e8cb1fc35d024ae702d4670d78bf5cc8) )

	ROM_REGION( 0xa0000, "gfx1", 0 ) // 5bpp?
	ROM_LOAD( "2.u36",     0x000000, 0x20000, CRC(bb4fcfde) SHA1(7e19722ce42b9ec86faac32a526429b0e56639b5) )
	ROM_LOAD( "3.u35_alt", 0x020000, 0x20000, CRC(3fd93c2f) SHA1(5217e328e51a2e00dc85a662dab6e339bd7f336f) ) // one of these is probably bad
	ROM_LOAD( "4.u34",     0x040000, 0x20000, CRC(372a67a4) SHA1(c1c1352dd3152603827224d8970e6cb04aa1e858) )
	ROM_LOAD( "5.u33",     0x060000, 0x20000, CRC(8058882e) SHA1(2de7b1e6e39d89913b2d6c1290d3cf326d2527d4) )
	ROM_LOAD( "6.u32",     0x080000, 0x20000, CRC(7217a8c2) SHA1(275c2d5a128960dd6cd56d5e3647354b17129a12) )

	ROM_REGION( 0x40000, "oki", 0 ) // 6295 samples
	ROM_LOAD( "1.u54",   0x000000, 0x40000, CRC(e4a89163) SHA1(c0622c4e97b23daf9775137a2754bf9c47a29385) )

	ROM_REGION( 0x4d4c, "mcu", 0 ) // MCU code
	ROM_LOAD( "pic16c65a.u60.bad.dump", 0x000, 0x4d4c, BAD_DUMP CRC(c5e260ec) SHA1(d6e41de8a7db27382757ed7edfd7985090896e39) )

// palce22v10h.u74.bad.dump= palce22v10h-ch-jin-u27.u27  Jingle Bell (Italy, V133I)
ROM_END

ROM_START( mil4000a )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "27.u75", 0x000001, 0x20000, CRC(2a090f82) SHA1(c70295de25a99ec78752f2bd63e6ef0714141c84) )
	ROM_LOAD16_BYTE( "28.u76", 0x000000, 0x20000, CRC(009e1f16) SHA1(33014ccd33abf2de8e83ec964192ebb9cbda8a08) )

	ROM_REGION( 0xa0000, "gfx1", 0 ) // 5bpp?
	ROM_LOAD( "2.u36",   0x000000, 0x20000, CRC(bb4fcfde) SHA1(7e19722ce42b9ec86faac32a526429b0e56639b5) )
	ROM_LOAD( "3.u35",   0x020000, 0x20000, CRC(21c43d81) SHA1(a266b85378723ad8e219dd63a639add64624de13) )
	ROM_LOAD( "4.u34",   0x040000, 0x20000, CRC(372a67a4) SHA1(c1c1352dd3152603827224d8970e6cb04aa1e858) )
	ROM_LOAD( "5.u33",   0x060000, 0x20000, CRC(8058882e) SHA1(2de7b1e6e39d89913b2d6c1290d3cf326d2527d4) )
	ROM_LOAD( "6.u32",   0x080000, 0x20000, CRC(7217a8c2) SHA1(275c2d5a128960dd6cd56d5e3647354b17129a12) )

	ROM_REGION( 0x40000, "oki", 0 ) // 6295 samples
	ROM_LOAD( "1.u54",   0x000000, 0x40000, CRC(e4a89163) SHA1(c0622c4e97b23daf9775137a2754bf9c47a29385) )

	ROM_REGION( 0x4d4c, "mcu", 0 ) // MCU code
	ROM_LOAD( "pic16c65b_millennium4000.u60", 0x000, 0x4d4c, BAD_DUMP CRC(4f3f7b90) SHA1(fdf689dda57960820315dcf0138d2ade28248681) )

// palce22v10h.u74.bad.dump= palce22v10h-ch-jin-u27.u27  Jingle Bell (Italy, V133I)
ROM_END

ROM_START( mil4000b )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "27.u75", 0x000001, 0x20000, CRC(a5ca8a1e) SHA1(c42244e27031175c37e83995f548d960708eabab) )
	ROM_LOAD16_BYTE( "28.u76", 0x000000, 0x20000, CRC(5bf4e681) SHA1(818d0ec1b2cc544334b0349ae15fd53ff32ef8c1) )

	ROM_REGION( 0xa0000, "gfx1", 0 ) // 5bpp?
	ROM_LOAD( "2.u36",   0x000000, 0x20000, CRC(bb4fcfde) SHA1(7e19722ce42b9ec86faac32a526429b0e56639b5) )
	ROM_LOAD( "3.u35",   0x020000, 0x20000, CRC(21c43d81) SHA1(a266b85378723ad8e219dd63a639add64624de13) )
	ROM_LOAD( "4.u34",   0x040000, 0x20000, CRC(372a67a4) SHA1(c1c1352dd3152603827224d8970e6cb04aa1e858) )
	ROM_LOAD( "5.u33",   0x060000, 0x20000, CRC(8058882e) SHA1(2de7b1e6e39d89913b2d6c1290d3cf326d2527d4) )
	ROM_LOAD( "6.u32",   0x080000, 0x20000, CRC(7217a8c2) SHA1(275c2d5a128960dd6cd56d5e3647354b17129a12) )

	ROM_REGION( 0x40000, "oki", 0 ) // 6295 samples
	ROM_LOAD( "1.u54",   0x000000, 0x40000, CRC(e4a89163) SHA1(c0622c4e97b23daf9775137a2754bf9c47a29385) )

	ROM_REGION( 0x4d4c, "mcu", 0 ) // MCU code
	ROM_LOAD( "pic16c65b_millennium4000.u60", 0x000, 0x4d4c, BAD_DUMP CRC(4f3f7b90) SHA1(fdf689dda57960820315dcf0138d2ade28248681) )
ROM_END

ROM_START( mil4000c )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "0.u75", 0x000001, 0x20000, CRC(2f84883e) SHA1(8b016eb586db59517f6a5d7e98a53c2002a6ed0a) )
	ROM_LOAD16_BYTE( "e.u76", 0x000000, 0x20000, CRC(f162018f) SHA1(43ac82d5828e57fb7ae83d88e1ed287a2e2060a3) )

	ROM_REGION( 0xa0000, "gfx1", 0 ) // 5bpp?
	ROM_LOAD( "5.u36",     0x000000, 0x20000, CRC(bb4fcfde) SHA1(7e19722ce42b9ec86faac32a526429b0e56639b5) )
	ROM_LOAD( "4.u35",     0x020000, 0x20000, CRC(3fd93c2f) SHA1(5217e328e51a2e00dc85a662dab6e339bd7f336f) )
	ROM_LOAD( "3.u34",     0x040000, 0x20000, CRC(372a67a4) SHA1(c1c1352dd3152603827224d8970e6cb04aa1e858) )
	ROM_LOAD( "2.u33",     0x060000, 0x20000, CRC(8058882e) SHA1(2de7b1e6e39d89913b2d6c1290d3cf326d2527d4) )
	ROM_LOAD( "1.u32",     0x080000, 0x20000, CRC(7217a8c2) SHA1(275c2d5a128960dd6cd56d5e3647354b17129a12) )

	ROM_REGION( 0x40000, "oki", 0 ) // 6295 samples
	ROM_LOAD( "red.u54",   0x000000, 0x40000, CRC(e4a89163) SHA1(c0622c4e97b23daf9775137a2754bf9c47a29385) )

	ROM_REGION( 0x4d4c, "mcu", 0 ) // MCU code
	ROM_LOAD( "pic16c74b_ch4000.u60", 0x000, 0x4d4c, NO_DUMP )
ROM_END

GAMEL( 2000, mil4000,    0,        mil4000,    mil4000,    0, ROT0,  "Sure Milano", "Millennium Nuovo 4000 (Version 2.0)", 0, layout_mil4000 )
GAMEL( 2000, mil4000a,   mil4000,  mil4000,    mil4000,    0, ROT0,  "Sure Milano", "Millennium Nuovo 4000 (Version 1.8)", 0, layout_mil4000 )
GAMEL( 2000, mil4000b,   mil4000,  mil4000,    mil4000,    0, ROT0,  "Sure Milano", "Millennium Nuovo 4000 (Version 1.5)", 0, layout_mil4000 )
GAMEL( 2000, mil4000c,   mil4000,  mil4000,    mil4000,    0, ROT0,  "Sure Milano", "Millennium Nuovo 4000 (Version 1.6)", 0, layout_mil4000 )
