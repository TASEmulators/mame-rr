/********************************************************************

 F-E1-32 driver


 Supported Games      PCB-ID
 ----------------------------------
 Mosaic               F-E1-32-009

 driver by Pierpaolo Prazzoli

*********************************************************************/

#include "emu.h"
#include "cpu/e132xs/e132xs.h"
#include "machine/eeprom.h"
#include "sound/2151intf.h"
#include "sound/okim6295.h"

class mosaicf2_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, mosaicf2_state(machine)); }

	mosaicf2_state(running_machine &machine)
		: maincpu(machine.device<cpu_device>("maincpu")) { }

	/* memory pointers */
	cpu_device *	maincpu;
	UINT32 *  videoram;
};


static VIDEO_UPDATE( mosaicf2 )
{
	mosaicf2_state *state = (mosaicf2_state *)screen->machine->driver_data;
	offs_t offs;

	for (offs = 0; offs < 0x10000; offs++)
	{
		int y = offs >> 8;
		int x = offs & 0xff;

		if ((x < 0xa0) && (y < 0xe0))
		{
			*BITMAP_ADDR16(bitmap, y, (x * 2) + 0) = (state->videoram[offs] >> 16) & 0x7fff;
			*BITMAP_ADDR16(bitmap, y, (x * 2) + 1) = (state->videoram[offs] >>  0) & 0x7fff;
		}
	}

	return 0;
}



static ADDRESS_MAP_START( common_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x001fffff) AM_RAM
	AM_RANGE(0x40000000, 0x4003ffff) AM_RAM AM_BASE_MEMBER(mosaicf2_state, videoram)
	AM_RANGE(0x80000000, 0x80ffffff) AM_ROM AM_REGION("user2",0)
	AM_RANGE(0xfff00000, 0xffffffff) AM_ROM AM_REGION("user1",0)
ADDRESS_MAP_END

static READ32_HANDLER( f32_input_port_1_r )
{
	/* burn a bunch of cycles because this is polled frequently during busy loops */
	mosaicf2_state *state = (mosaicf2_state *)space->machine->driver_data;
	offs_t pc = state->maincpu->pc();
	if ((pc == 0x000379de) || (pc == 0x000379cc) )
		state->maincpu->eat_cycles(100);
	//else printf("PC %08x\n", pc );
	return input_port_read(space->machine, "SYSTEM_P2");
}


static ADDRESS_MAP_START( mosaicf2_io, ADDRESS_SPACE_IO, 32 )
	AM_RANGE(0x4000, 0x4003) AM_DEVREAD8("oki", okim6295_r, 0x000000ff)
	AM_RANGE(0x4810, 0x4813) AM_DEVREAD8("ymsnd", ym2151_status_port_r, 0x000000ff)
	AM_RANGE(0x5000, 0x5003) AM_READ_PORT("P1")
	AM_RANGE(0x5200, 0x5203) AM_READ(f32_input_port_1_r)
	AM_RANGE(0x5400, 0x5403) AM_READ_PORT("EEPROMIN")
	AM_RANGE(0x6000, 0x6003) AM_DEVWRITE8("oki", okim6295_w, 0x000000ff)
	AM_RANGE(0x6800, 0x6803) AM_DEVWRITE8("ymsnd", ym2151_data_port_w, 0x000000ff)
	AM_RANGE(0x6810, 0x6813) AM_DEVWRITE8("ymsnd", ym2151_register_port_w, 0x000000ff)
	AM_RANGE(0x7000, 0x7003) AM_WRITE_PORT("EEPROMCLK")
	AM_RANGE(0x7200, 0x7203) AM_WRITE_PORT("EEPROMCS")
	AM_RANGE(0x7400, 0x7403) AM_WRITE_PORT("EEPROMOUT")
ADDRESS_MAP_END


static INPUT_PORTS_START( mosaicf2 )
	PORT_START("P1")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0xff000000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM_P2")
	PORT_BIT( 0x000000ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x00000400, IP_ACTIVE_LOW )
	PORT_BIT( 0x00007800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_VBLANK )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xff000000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START( "EEPROMIN" )
	PORT_BIT( 0x00000001, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE("eeprom", eeprom_read_bit)

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x00000001, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_write_bit)

	PORT_START( "EEPROMCLK" )
	PORT_BIT( 0x00000001, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_set_clock_line)

	PORT_START( "EEPROMCS" )
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_set_cs_line)
INPUT_PORTS_END

static MACHINE_DRIVER_START( mosaicf2 )

	/* driver data */
	MDRV_DRIVER_DATA(mosaicf2_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", E132XN, 20000000*4)	/* 4x internal multiplier */
	MDRV_CPU_PROGRAM_MAP(common_map)
	MDRV_CPU_IO_MAP(mosaicf2_io)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_EEPROM_93C46_ADD("eeprom")

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(512, 512)
	MDRV_SCREEN_VISIBLE_AREA(0, 319, 0, 223)

	MDRV_PALETTE_INIT(RRRRR_GGGGG_BBBBB)
	MDRV_PALETTE_LENGTH(32768)

	MDRV_VIDEO_UPDATE(mosaicf2)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2151, 14318180/4)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)

	MDRV_OKIM6295_ADD("oki", 1789772.5, OKIM6295_PIN7_HIGH)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
MACHINE_DRIVER_END

/*

Mosaic (c) 1999 F2 System

   CPU: Hyperstone E1-32XN
 Video: QuickLogic QL2003-XPL84C
 Sound: OKI 6295, BS901 (YM2151) & BS902 (YM3012)
   OSC: 20MHz & 14.31818MHz
EEPROM: 93C46

F-E1-32-009
+------------------------------------------------------------------+
|            VOL                               +---------+         |
+-+                              YM3812        |   SND   |         |
  |                                            +---------+         |
+-+                              YM2151            OKI6295         |
|                                                                  |
|                                   +---------------+              |
|                                   |               |              |
|J                   +-------+      |               |              |
|A                   | VRAML |      | QuickLogic    |  14.31818MHz |
|M                   +-------+      | QL2003-XPL84C |              |
|M                   +-------+      | 9819 BA       |   +-----+    |
|A                   | VRAMU |      |               |   |93C46|    |
|                    +-------+      +---------------+   +-----+    |
|C                                                                 |
|O                                      +---------+   +---------+  |
|N                                      |   L00   |   |   U00   |  |
|N                                      |         |   |         |  |
|E                                      +---------+   +---------+  |
|C                   +------------+     +---------+   +---------+  |
|T                   |            |     |   L01   |   |   U01   |  |
|O                   |            |     |         |   |         |  |
|R                   | HyperStone |     +---------+   +---------+  |
|                    |  E1-32XN   |     +---------+   +---------+  |
|                    |            |     |   L02   |   |   U02   |  |
|          +-----+   |            |     |         |   |         |  |
|          |DRAML|   +------------+     +---------+   +---------+  |
+-+        +-----+                      +---------+   +---------+  |
  |        +-----+               20MHz  |   L03   |   |   U03   |  |
+-+        |DRAMU|                      |         |   |         |  |
|          +-----+    +----------+      +---------+   +---------+  |
|  +--+ +--+          |   ROM1   |                                 |
|  |S3| |S1|          +----------+                                 |
+------------------------------------------------------------------+

S3 is a reset button
S1 is the setup button

VRAML & VRAMU are KM6161002CJ-12
DRAML & DRAMU are GM71C18163CJ6

ROM1 & SND are stardard 27C040 and/or 27C020 eproms
L00-L03 & U00-U03 are 29F1610ML Flash roms
*/

ROM_START( mosaicf2 )
	ROM_REGION32_BE( 0x100000, "user1", ROMREGION_ERASE00 ) /* Hyperstone CPU Code */
	/* 0 - 0x80000 empty */
	ROM_LOAD( "rom1.bin",            0x80000, 0x080000, CRC(fceb6f83) SHA1(b98afb477627c3b2d584c0f0fb26c4dd5b1a31e2) )

	ROM_REGION32_BE( 0x1000000, "user2", 0 )  /* gfx data */
	ROM_LOAD32_WORD_SWAP( "u00.bin", 0x000000, 0x200000, CRC(a2329675) SHA1(bff8974fab9120274821c9c9646744317f47c79c) )
	ROM_LOAD32_WORD_SWAP( "l00.bin", 0x000002, 0x200000, CRC(d96fe93b) SHA1(005d9889077825fc0e308d2981f6fca5e6b51fe8) )
	ROM_LOAD32_WORD_SWAP( "u01.bin", 0x400000, 0x200000, CRC(6379e73f) SHA1(fe5abafbcbd828795cb06a08763fae1bbe2a75ad) )
	ROM_LOAD32_WORD_SWAP( "l01.bin", 0x400002, 0x200000, CRC(a269ea82) SHA1(d962a8b3293c6f46dbefa49859b2b3e594e7a386) )
	ROM_LOAD32_WORD_SWAP( "u02.bin", 0x800000, 0x200000, CRC(c17f95cd) SHA1(1c701185be138b615d2851866288647f40809c28) )
	ROM_LOAD32_WORD_SWAP( "l02.bin", 0x800002, 0x200000, CRC(69cd9c5c) SHA1(6b4d204a6ab5f36dfba9053bb3be2d094fcfdd00) )
	ROM_LOAD32_WORD_SWAP( "u03.bin", 0xc00000, 0x200000, CRC(0e47df20) SHA1(6f6c3e7fc8c99db7ddc73d8d10a661373bb72a1a) )
	ROM_LOAD32_WORD_SWAP( "l03.bin", 0xc00002, 0x200000, CRC(d79f6ca8) SHA1(4735dda9269aa05ba1251d335dc73914f5cb43b0) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Oki Samples */
	ROM_LOAD( "snd.bin",             0x000000, 0x040000, CRC(4584589c) SHA1(5f9824724f840767c3dc1dc04b203ddf3d78b84c) )
ROM_END

GAME( 1999, mosaicf2, 0, mosaicf2, mosaicf2, 0,        ROT0, "F2 System", "Mosaic (F2 System)", GAME_SUPPORTS_SAVE )
