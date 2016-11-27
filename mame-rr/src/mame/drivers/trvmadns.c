/*

Trivia Madness

PCB Thunderhead, Inc. 1983

CPU board:
  CPU: Z84000ABI Z80 cpu
Sound: AY-3-8910
  RAM: AMD AM9128-15PC (2048x8 Static RAM)
  OSC: 10.000MHz
Video: F6845P 40 pin dil (8 bit CRT Controller)
 Misc: RCA X (CDM6116E2) 24 pin dil (General-Purpose Static RAM - Multiplexed I/O)
 Roms: u7f lat green - type 2764
       u6f lat green - type 2764
       u5f lat la trivia - type 2764
2 50-pin Ribon cable connectors


Rom board:
 Ram: HM6264P-15 (64 K SRAM ( 8-kword X 8-bit )
 Ram: 8437 (SY2128-3 2Kx8 SRAM 150NS)
Roms: row d-e sex a1       - type 27128
      row c-d sex a2       - type 27128
      row b-c sex a3       - type 27128
      row a sex a4         - type 27128
      row d-e soaps a1     - type 27128
      row c-d soaps a2     - type 27128
      row b-c soaps a3     - type 27128
      row d-e 2 str trk a1 - type 27256
      row c-d 2 str trk a2 - type 27256
      row d-e 2 rebel a1   - type 27256
      row c-d 2 rebel a2   - type 27256
      row triva madness    - type 27128
2 50-pin Ribon cable connectors

Google for F6845P shows this info from DatasheetArchive.com:

Part Number = F6845P
Description = Video Output Graphics Controller - CRT Controller
Manufacturer = Various
Micro Processor Family = 6800
Maximum Clock Frequency (Hz) = 1.0M
Vsup Nom.(V) Supply Voltage = 5.0
Package = DIP
Pins = 40
Military = N
Technology = NMOS


 driver by Pierpaolo Prazzoli
 thanks to David Haywood and Tomasz Slanina for their assistance

 TODO:
 - fix question banks (sometimes the game hangs)
 - fix palette
 - fix tilemap colors
 - remove hack for irq0 firing
 - add nvram

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"

static tilemap_t *bg_tilemap;
static UINT8 *trvmadns_gfxram;
static UINT8 *trvmadns_tileram;
static int old_data;

static WRITE8_HANDLER( trvmadns_banking_w )
{

	UINT8 *rom;
	int address = 0;

	if((data & 0xf0) == 0xa0)
	{
		/* FIXME: selects GFX char RAM bank here */
	}
	else if((data & 0xf0) == 0x80 || (data & 0xf0) == 0x90)
	{
		rom = memory_region(space->machine, "user2");

		switch(data & 0xf)
		{
			case 0x00: address = 0x6000; break;
			case 0x04: address = 0x4000; break;
			case 0x06: address = 0x2000; break;
			case 0x07: address = 0x0000; break;
			case 0x08: address = 0xe000; break;
			case 0x0c: address = 0xc000; break;
			case 0x0e: address = 0xa000; break;
			case 0x0f: address = 0x8000; break;
		}

		address |= (data & 0x10) ? 0x10000 : 0;

		memory_set_bankptr(space->machine, "bank1", &rom[address]);
		memory_set_bankptr(space->machine, "bank2", &rom[address + 0x1000]);
	}
	else
	{
			if(data != old_data)
			{
				old_data = data;
				logerror("port80 = %02X\n",data);
				//logerror("port80 = %02X\n",data);
			}

		rom = memory_region(space->machine, "user1");

		/*
        7
        6
        4
        0
        */

		//switch(data & 0xf)
		switch(data & 7)
		{
			case 0x00: address = 0x6000; break;
			case 0x04: address = 0x0000; break;
			case 0x06: address = 0x2000; break;
			case 0x07: address = 0x4000; break;

		}
//24: 1st rom star trek
		address |= ((data & 0x60) >> 5) * 0x10000;

		//not sure about (((data & 7) ^ 7) * 0x2000)
		//address = (((data & 0x60) >> 5) * 0x10000) | (((data & 0x0f) ^ 7) * 0x2000);

		//address |= (data & 0x08) ? 0x8000 : 0;

//      logerror("add = %X\n",address);

		memory_set_bankptr(space->machine, "bank1", &rom[address]);
	}
}

static WRITE8_HANDLER( trvmadns_gfxram_w )
{
	trvmadns_gfxram[offset] = data;
	gfx_element_mark_dirty(space->machine->gfx[0], offset/16);
}

static WRITE8_HANDLER( trvmadns_palette_w )
{
	int r,g,b,datax;
	space->machine->generic.paletteram.u8[offset] = data;
	offset>>=1;
	datax=space->machine->generic.paletteram.u8[offset*2+1]+256*space->machine->generic.paletteram.u8[offset*2];

	b = (((datax & 0x0007)>>0) | ((datax & 0x0200)>>6)) ^ 0xf;
	r = (((datax & 0x0038)>>3) | ((datax & 0x0400)>>7)) ^ 0xf;
	g = (((datax & 0x01c0)>>6) | ((datax & 0x0800)>>8)) ^ 0xf;

	palette_set_color_rgb(space->machine, offset, pal4bit(r), pal4bit(g), pal4bit(b));
}


static WRITE8_HANDLER( w2 )
{
/*  static int old = -1;
    if(data!=old)
        logerror("w2 = %02X\n",old=data);
*/
}

static WRITE8_HANDLER( w3 )
{
/*  static int old = -1;
    if(data!=old)
        logerror("w3 = %02X\n",old=data);
*/
}

static WRITE8_HANDLER( trvmadns_tileram_w )
{
	if(offset==0)
	{
		if(cpu_get_previouspc(space->cpu)==0x29e9)// || cpu_get_previouspc(space->cpu)==0x1b3f) //29f5
		{
			cputag_set_input_line(space->machine, "maincpu", 0, HOLD_LINE);
		}
//      else
//          logerror("%x \n", cpu_get_previouspc(space->cpu));

	}

	trvmadns_tileram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap,offset >> 1);
}


static ADDRESS_MAP_START( cpu_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0x6fff) AM_ROMBANK("bank1")
	AM_RANGE(0x7000, 0x7fff) AM_ROMBANK("bank2")
	AM_RANGE(0x6000, 0x7fff) AM_WRITE(trvmadns_gfxram_w) AM_BASE(&trvmadns_gfxram)
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0xa000, 0xa7ff) AM_RAM_WRITE(trvmadns_tileram_w) AM_BASE(&trvmadns_tileram)
	AM_RANGE(0xc000, 0xc01f) AM_RAM_WRITE(trvmadns_palette_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0xe000, 0xe000) AM_WRITE(w2)//NOP
	AM_RANGE(0xe004, 0xe004) AM_WRITE(w3)//NOP
ADDRESS_MAP_END

static ADDRESS_MAP_START( io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVWRITE("aysnd", ay8910_address_data_w)
	AM_RANGE(0x02, 0x02) AM_READ_PORT("IN0")
	AM_RANGE(0x80, 0x80) AM_WRITE(trvmadns_banking_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( trvmadns )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	0x200,
	2,
	{ 0, 1 },
	{ 6, 4, 2, 0, 14, 12, 10, 8 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	8*16
};

static GFXDECODE_START( trvmadns )
	GFXDECODE_ENTRY( NULL, 0x6000, charlayout, 0, 4 ) // doesn't matter where we point this, all the tiles are decoded while the game runs
GFXDECODE_END

static TILE_GET_INFO( get_bg_tile_info )
{
	int tile,attr,color,flag;

	attr = trvmadns_tileram[tile_index*2 + 0];
	tile = trvmadns_tileram[tile_index*2 + 1] + ((attr & 0x01) << 8);
	color = (attr & 0x18) >> 3;
	flag = TILE_FLIPXY((attr & 0x06) >> 1);

//  if((~attr & 0x20) || (~attr & 0x40))
//      flag |= TILE_FORCE_LAYER0;

	//0x20? tile transparent pen 1?
	//0x40? tile transparent pen 1?

	SET_TILE_INFO(0,tile,color,flag);

	tileinfo->category = (attr & 0x20)>>5;
}

static VIDEO_START( trvmadns )
{
	bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);

//  tilemap_set_transparent_pen(fg_tilemap,1);

	gfx_element_set_source(machine->gfx[0], trvmadns_gfxram);
}

static VIDEO_UPDATE( trvmadns )
{
	int x,y,count;
	const gfx_element *gfx = screen->machine->gfx[0];

	bitmap_fill(bitmap,cliprect,0xd);

	count = 0;

	for (y=0;y<32;y++)
	{
		for (x=0;x<32;x++)
		{
			int attr = trvmadns_tileram[count*2+0];
			int tile = trvmadns_tileram[count*2+1] | ((attr & 0x01) << 8);
			int color = (attr & 0x18) >> 3;
			int flipx = attr & 4;
			int flipy = attr & 2;

			if(!(attr & 0x20))
				drawgfx_opaque(bitmap,cliprect,gfx,tile,color,flipx,flipy,(x*8),(y*8));
			count++;
		}
	}

	count = 0;

	for (y=0;y<32;y++)
	{
		for (x=0;x<32;x++)
		{
			int attr = trvmadns_tileram[count*2+0];
			int tile = trvmadns_tileram[count*2+1] | ((attr & 0x01) << 8);
			int color = (attr & 0x18) >> 3;
			int flipx = attr & 4;
			int flipy = attr & 2;

			if(attr & 0x20)
				drawgfx_transpen(bitmap,cliprect,gfx,tile,color,flipx,flipy,(x*8),(y*8),1);
			count++;
		}
	}

	return 0;
}

static MACHINE_RESET( trvmadns )
{
	old_data = -1;
}

static MACHINE_DRIVER_START( trvmadns )
	MDRV_CPU_ADD("maincpu", Z80,10000000/2) // ?
	MDRV_CPU_PROGRAM_MAP(cpu_map)
	MDRV_CPU_IO_MAP(io_map)
	MDRV_CPU_VBLANK_INT("screen", nmi_line_pulse)

	MDRV_MACHINE_RESET(trvmadns)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 31*8-1, 0*8, 30*8-1)

	MDRV_GFXDECODE(trvmadns)
	MDRV_PALETTE_LENGTH(16)

	MDRV_VIDEO_START(trvmadns)
	MDRV_VIDEO_UPDATE(trvmadns)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("aysnd", AY8910, 10000000/2/4) //?
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


ROM_START( trvmadns )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u5f lat la trivia.bin", 0x0000, 0x2000, CRC(a8fb07ea) SHA1(dcf2cccd8b98087d30b3347e69b1bf8565f95ad6) )
	ROM_LOAD( "u6f lat green.bin",     0x2000, 0x2000, CRC(40f816f1) SHA1(a1a6a9af99edb1860bc4c8eb51859bbfbf91cae2) )
	ROM_LOAD( "u7f lat green.bin",     0x4000, 0x2000, CRC(3e45feb0) SHA1(5ffc18ab3f6ace844242d4be52b3946c1469944a) )

	ROM_REGION( 0x40000, "user1", ROMREGION_ERASEFF ) /* Question roms 1st set */
	ROM_LOAD( "row d-e 2 rebel a1.bin",   0x00000, 0x8000, CRC(92e6dcf8) SHA1(e8429fe60fadfc841ed0d69b4a815765e82723db) )
	ROM_LOAD( "row c-d 2 rebel a2.bin",   0x08000, 0x8000, CRC(b45429be) SHA1(9285f12bc0bceb0a91fc1f2f2941825b73bdb02c) )
	ROM_LOAD( "row d-e 2 str trk a1.bin", 0x10000, 0x8000, CRC(dc6fc7e1) SHA1(86ba730123bbb06d8290d68c042dd215bddf7629) )
	ROM_LOAD( "row c-d 2 str trk a2.bin", 0x18000, 0x8000, CRC(0133c462) SHA1(e6928880cb7916408579fa9d67ad3adc558de133) )
	ROM_LOAD( "row d-e soaps a1.bin",     0x20000, 0x4000, CRC(df451b8a) SHA1(23a5f953018e5401fe26eb638e13caacf6fa628f) )
	ROM_LOAD( "row c-d soaps a2.bin",     0x24000, 0x4000, CRC(13685dac) SHA1(f5f6103404c846decf32eae5d504e00e17629b03) )
	ROM_LOAD( "row b-c soaps a3.bin",     0x28000, 0x4000, CRC(b42f3294) SHA1(eac4e26ed48e7de80a60fbb2ee4e661619700d1d) )
	// 0x2c000 - 0x2ffff empty
	ROM_LOAD( "row d-e sex a1.bin",       0x30000, 0x4000, CRC(857c1332) SHA1(fdb08080143e170441b3db9e69b21ac9da10d499) )
	ROM_LOAD( "row c-d sex a2.bin",       0x34000, 0x4000, CRC(8fba2e07) SHA1(cbe7f9b973bd2a127cae736df39112b050ec98d8) )
	ROM_LOAD( "row b-c sex a3.bin",       0x38000, 0x4000, CRC(3fea2c2a) SHA1(fa403e14b057f0e6d607871adcaba85a6c77f1f9) )
	ROM_LOAD( "row a sex a4.bin",         0x3c000, 0x4000, CRC(2d179c7b) SHA1(153240f1fcc4f53b6840eafdd9ce0fb3e52ec1aa) )

	ROM_REGION( 0x20000, "user2", ROMREGION_ERASEFF ) /* Question roms 2nd set */
	ROM_LOAD( "trivia madness.bin",       0x00000, 0x2000, CRC(5aec7cfa) SHA1(09e4eac78d975aef3af224b42b60499d759e7749) )
	ROM_CONTINUE(                         0x0e000, 0x2000 )
	// empty space, for 3 roms (each one max 0x8000 bytes long)
ROM_END

GAME( 1985, trvmadns, 0, trvmadns, trvmadns, 0, ROT0, "Thunderhead Inc.", "Trivia Madness", GAME_WRONG_COLORS | GAME_NOT_WORKING )
