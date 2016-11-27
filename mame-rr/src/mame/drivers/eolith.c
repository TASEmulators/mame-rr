/********************************************************************
 Eolith 32 bits hardware: Gradation 2D system

 driver by Tomasz Slanina    analog [at] op.pl
 and       Pierpaolo Prazzoli

 Main CPU:
  Hyperstone E1-32N @ 45 or 50 MHz

  Sound CPU:
   80c301/AT89c52

  Sound:
   General MIDI Chipset QDSP 1000 MIDI Player (80c32 CPU)
   MIDI 16th Channel(32 Poly) using as Effect EPROM : 512Kbytes
   MIDI Background Music EPROM : 512Kbyte
     TDA1519A(Philips)Stereo Power AMP

  Video:
   16 bit True Color/Dot
   Resolution - 320x340 or 512x384

  Memory:
    256KByte x2 VRAM
    512Kbyte/1Mega main RAM

 Games dumped
 - Hidden Catch (pcb ver 3.03)
 - New Hidden Catch (pcb ver 3.02)
 - Hidden Catch 2 (pcb ver 3.03)
 - Hidden Catch 3 (v. 1.00 / pcb ver 3.05)
 - Land Breaker (pcb ver 3.03) (MCU internal flash dump is missing)
 - Land Breaker (pcb ver 3.02)
 - Raccoon World
 - Fortress 2 Blue Arcade (v. 1.01 / pcb ver 3.05)
 - Fortress 2 Blue Arcade (v. 1.00 / pcb ver 3.05)
 - Puzzle King (Dance & Puzzle)
 - Iron Fortress

 Known games not dumped
 - Hidden Catch (pcb ver 3.02)
 - Fortress 2 Blue Arcade (v. 1.02)
 - Ribbon (Step1. Mild Mind) (c) 1999

 TODO:
 - sound & sound cpu

 Original Bugs:
 - Hidden Catch 3: the text shown when you start a game is flipped or
                   clipped wrongly

 *********************************************************************/

#include "emu.h"
#include "cpu/e132xs/e132xs.h"
#include "deprecat.h"
#include "machine/eeprom.h"

READ32_HANDLER(eolith_vram_r);
WRITE32_HANDLER(eolith_vram_w);
VIDEO_START(eolith);
VIDEO_UPDATE(eolith);

extern int eolith_buffer;

static int coin_counter_bit = 0;

#include "includes/eolithsp.h"

// It's configured for 512 bytes
static const eeprom_interface eeprom_interface_93C66 =
{
	9,				// address bits 9
	8,				// data bits    8
	"*110",			// read         110 aaaaaaaaa
	"*101",			// write        101 aaaaaaaaa dddddddd
	"*111",			// erase        111 aaaaaaaaa
	"*10000xxxxxx",	// lock         100 00xxxxxxx
	"*10011xxxxxx"	// unlock       100 11xxxxxxx
};



static READ32_HANDLER( eolith_custom_r )
{
	/*
        bit 3 = eeprom bit
        bit 6 = vblank flag

        Are these used only in landbrka ?
        bit 8 = ???
        bit 9 = ???
    */
	eolith_speedup_read(space);

	return (input_port_read(space->machine, "IN0") & ~0x300) | (mame_rand(space->machine) & 0x300);
}

static WRITE32_HANDLER( systemcontrol_w )
{
	eolith_buffer = (data & 0x80) >> 7;
	coin_counter_w(space->machine, 0, data & coin_counter_bit);
	set_led_status(space->machine, 0, data & 1);

	input_port_write(space->machine, "EEPROMOUT", data, 0xff);

	// bit 0x100 and 0x040 ?
}

static READ32_HANDLER( hidctch3_pen1_r )
{
	//320 x 240
	int xpos = input_port_read(space->machine, "PEN_X_P1");
	int ypos = input_port_read(space->machine, "PEN_Y_P1");

	return xpos + (ypos*168*2);
}

static READ32_HANDLER( hidctch3_pen2_r )
{
	//320 x 240
	int xpos = input_port_read(space->machine, "PEN_X_P2");
	int ypos = input_port_read(space->machine, "PEN_Y_P2");

	return xpos + (ypos*168*2);
}

static ADDRESS_MAP_START( eolith_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x001fffff) AM_RAM // fort2b wants ram here
	AM_RANGE(0x40000000, 0x401fffff) AM_RAM
	AM_RANGE(0x90000000, 0x9003ffff) AM_READWRITE(eolith_vram_r, eolith_vram_w)
	AM_RANGE(0xfc000000, 0xfc000003) AM_READ(eolith_custom_r)
	AM_RANGE(0xfc400000, 0xfc400003) AM_WRITE(systemcontrol_w)
	AM_RANGE(0xfc800000, 0xfc800003) AM_WRITENOP // sound latch
	AM_RANGE(0xfca00000, 0xfca00003) AM_READ_PORT("DSW1")
	AM_RANGE(0xfcc00000, 0xfcc0005b) AM_WRITENOP // crt registers ?
	AM_RANGE(0xfd000000, 0xfeffffff) AM_ROM AM_REGION("user1", 0)
	AM_RANGE(0xfff80000, 0xffffffff) AM_ROM AM_REGION("maincpu", 0)
ADDRESS_MAP_END



static INPUT_PORTS_START( common )
	PORT_START("IN0")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000008, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE("eeprom", eeprom_read_bit)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM(eolith_speedup_getvblank, NULL)
	PORT_BIT( 0x00003f80, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x00008000, IP_ACTIVE_LOW )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP	) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN	) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT	) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT	) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP	) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN	) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT	) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT	) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_BIT( 0xffffffff, IP_ACTIVE_LOW, IPT_UNUSED	)

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_set_cs_line)
	PORT_BIT( 0x00000004, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_set_clock_line)
	PORT_BIT( 0x00000008, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_write_bit)
INPUT_PORTS_END

static INPUT_PORTS_START( ironfort )
	PORT_START("IN0")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000008, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE("eeprom", eeprom_read_bit)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM(eolith_speedup_getvblank, NULL)
	PORT_BIT( 0x00003f80, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP	) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN	) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT	) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT	) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP	) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN	) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT	) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT	) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x00000001, 0x00000001, "Show Dip-switch Information" ) PORT_DIPLOCATION("SW4:1")
	PORT_DIPSETTING(          0x00000001, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000002, 0x00000000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW4:2")
	PORT_DIPSETTING(          0x00000002, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0000000c, 0x0000000c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW4:3,4")
	PORT_DIPSETTING(          0x00000008, "1" )
	PORT_DIPSETTING(          0x00000004, "2" )
	PORT_DIPSETTING(          0x0000000c, "3" )
	PORT_DIPSETTING(          0x00000000, "4" )
	PORT_DIPNAME( 0x00000030, 0x00000030, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW3:1,2")
	PORT_DIPSETTING(          0x00000010, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(          0x00000020, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(          0x00000030, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x000000c0, 0x000000c0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW3:3,4")
	PORT_DIPSETTING(          0x00000000, DEF_STR( Very_Hard ) )
	PORT_DIPSETTING(          0x00000040, DEF_STR( Hard ) )
	PORT_DIPSETTING(          0x000000c0, DEF_STR( Normal ) )
	PORT_DIPSETTING(          0x00000080, DEF_STR( Easy ) )
	PORT_BIT( 0xffffff00, IP_ACTIVE_LOW, IPT_UNUSED	)

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_set_cs_line)
	PORT_BIT( 0x00000004, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_set_clock_line)
	PORT_BIT( 0x00000008, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_write_bit)
INPUT_PORTS_END

static INPUT_PORTS_START( ironfortj )
	PORT_INCLUDE(ironfort)

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x00000002, 0x00000000, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW4:2")
	PORT_DIPSETTING(          0x00000002, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000030, 0x00000030, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW3:1,2")
	PORT_DIPSETTING(          0x00000000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(          0x00000010, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(          0x00000020, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(          0x00000030, DEF_STR( 1C_1C ) )
INPUT_PORTS_END

static INPUT_PORTS_START( hidnctch )
	PORT_INCLUDE(common)
	PORT_MODIFY("IN0")
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x00000001, 0x00000001, "Show Settings" )
	PORT_DIPSETTING(          0x00000001, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000002, 0x00000002, "Show Counters" )
	PORT_DIPSETTING(          0x00000002, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_BIT( 0xfffffffc, IP_ACTIVE_LOW, IPT_UNUSED	)
INPUT_PORTS_END

static INPUT_PORTS_START( hidctch3 )
	PORT_INCLUDE(common)
	PORT_MODIFY("IN0")
	PORT_BIT( 0x00ff0000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0xff000000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x00000001, 0x00000001, "Show Settings" )
	PORT_DIPSETTING(          0x00000001, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_BIT( 0xfffffffe, IP_ACTIVE_LOW, IPT_UNUSED	)

	PORT_START("PEN_X_P1")
	PORT_BIT( 0xffff, 0, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX(0,159) PORT_SENSITIVITY(25) PORT_KEYDELTA(1) PORT_PLAYER(1)

	PORT_START("PEN_Y_P1")
	PORT_BIT( 0xffff, 0, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX(0,119) PORT_SENSITIVITY(25) PORT_KEYDELTA(1) PORT_PLAYER(1)

	PORT_START("PEN_X_P2")
	PORT_BIT( 0xffff, 0, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX(0,159) PORT_SENSITIVITY(25) PORT_KEYDELTA(1) PORT_PLAYER(2)

	PORT_START("PEN_Y_P2")
	PORT_BIT( 0xffff, 0, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX(0,119) PORT_SENSITIVITY(25) PORT_KEYDELTA(1) PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( raccoon )
	PORT_INCLUDE(common)
	PORT_MODIFY("IN0")
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x0000000f, 0x0000000f, DEF_STR( Coinage ) )
	PORT_DIPSETTING(          0x0000000d, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(          0x0000000e, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(          0x0000000c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(          0x0000000f, DEF_STR( 1C_1C ) )
	// other values are just mirrors
	PORT_BIT( 0xfffffff0, IP_ACTIVE_LOW, IPT_UNUSED	)
INPUT_PORTS_END

static INPUT_PORTS_START( landbrk )
	PORT_INCLUDE(common)
	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x00000001, 0x00000001, "Show Settings" )
	PORT_DIPSETTING(          0x00000001, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000002, 0x00000002, "Show Counters" )
	PORT_DIPSETTING(          0x00000002, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000004, 0x00000004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(          0x00000004, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000008, 0x00000008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(          0x00000008, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_BIT( 0xfffffff0, IP_ACTIVE_LOW, IPT_UNUSED	)
INPUT_PORTS_END

static INPUT_PORTS_START( puzzlekg )
	PORT_INCLUDE(common)
	PORT_MODIFY("IN0")
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("DSW1")
	PORT_SERVICE( 0x0000000f, IP_ACTIVE_LOW ) // every bit enables it
	PORT_BIT( 0xfffffff0, IP_ACTIVE_LOW, IPT_UNUSED	)
INPUT_PORTS_END



static MACHINE_DRIVER_START( eolith45 )
	MDRV_CPU_ADD("maincpu", E132N, 45000000)		 /* 45 MHz */
	MDRV_CPU_PROGRAM_MAP(eolith_map)
	MDRV_CPU_VBLANK_INT_HACK(eolith_speedup,262)

	/* sound cpu */

	MDRV_EEPROM_ADD("eeprom", eeprom_interface_93C66)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(512, 512)
	MDRV_SCREEN_VISIBLE_AREA(0, 319, 0, 239)

	MDRV_PALETTE_INIT(RRRRR_GGGGG_BBBBB)
	MDRV_PALETTE_LENGTH(32768)

	MDRV_VIDEO_START(eolith)
	MDRV_VIDEO_UPDATE(eolith)

	/* sound hardware */
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( eolith50 )
	MDRV_IMPORT_FROM(eolith45)
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_CLOCK(50000000)		 /* 50 MHz */
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( ironfort )
	MDRV_IMPORT_FROM(eolith45)
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_CLOCK(44900000) /* Normaly 45MHz??? but PCB actually had a 44.9MHz OSC, so it's value is used */
MACHINE_DRIVER_END



/*

Iron Fortress
Eolith, 1998

This game runs on hardware that looks exactly like the Gradation 2D PCB
but there's no text or labelling on the PCB to say that. However, it is
an original Eolith PCB.

Printed on the back of the pcb is "9752 12-133"

PCB Layout
----------

(No PCB number)
|-------------------------------------------------|
|            VOL    KM6161002 KM6161002  IS61C1024|
|            VOL                                  |
|                   KM6161002 KM6161002  IS61C1024|
|     24MHz       QS1001A  DSW4 DSW3              |
|          QS1000                        IS61C1024|
|            U107   U97  EV0514-001               |
|J                                       IC61C1024|
|A                     14.31818MHz                |
|M                                    E1-32N      |
|M                                                |
|A     93C66                        44.9MHz       |
|                                                 |
|                                                 |
|            GMS80C301                            |
|       12MHz                                     |
|                                                 |
|             U108    U41*  U39   U36*  U34       |
|        U111      U42*  U40   U37*  U35    U43   |
|                                                 |
|-------------------------------------------------|
Notes:
      E1-32N       - Hyperstone E1-32N CPU, clock 44.900MHz (QFP160)
      80C301 clock - 12.000MHz
      IS61C1024    - ISSI 128k x4 High Speed CMOS Static RAM (SOJ32)
      KM6161002    - Samsung 64k x4 Ultra High Speed CMOS Video Static RAM (SOJ44)
      QS1000       - QDSP QS1000 AdMOS 9638R, Wavetable Audio chip, clock input of 24.000MHz (QFP100)
      QS1001A      - QDSP QS1001A 512k x8 MaskROM (SOP32)
      EV0514-001   - Custom Eolith IC (QFP100)
      VSync        - 60Hz
      HSync        - 15.64kHz

      DSW4 & DSW3 are 4 switch dipswitches

      U107, U97, U111, U108 & U43 are populated with EPROMs and are not labeled in any way
      U34 - U42 are C32000 MASK roms read as 27C322 those marked with '*' are unpopulated

      qs1001a.u96 was not dumped from this PCB, but is a standard sample rom found on many Eolith gradation PCBs
*/

ROM_START( ironfort )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* Hyperstone CPU Code */
	ROM_LOAD( "u43", 0x00000, 0x80000, CRC(29f55825) SHA1(e048ec0f5d83d4b64aa48d706fa0947afcdc1a3d) ) /* 27C040 eprom with no label */

	ROM_REGION32_BE( 0x2000000, "user1", ROMREGION_ERASE00 ) /* Game Data - banked ROM, swapping necessary */
	ROM_LOAD32_WORD_SWAP( "if00-00.u39", 0x0000000, 0x400000, CRC(63b74601) SHA1(c111ecf55359e9005a3ec1fe1202a34624f8b242) )
	ROM_LOAD32_WORD_SWAP( "if00-01.u34", 0x0000002, 0x400000, CRC(890470b3) SHA1(57df122ab01744b47ebd38554eb6a7d780977be2) )
	ROM_LOAD32_WORD_SWAP( "if00-02.u40", 0x0800000, 0x400000, CRC(63b5cca5) SHA1(4ec8b813c7e465f659a4a2361ddfbad763bf6e6a) )
	ROM_LOAD32_WORD_SWAP( "if00-03.u35", 0x0800002, 0x400000, CRC(54a76cb5) SHA1(21fb3bedf065079d59f642b19487f76590f97558) )

	ROM_REGION( 0x008000, "cpu1", 0 ) /* QDSP ('51) Code */
	ROM_LOAD( "u107", 0x0000, 0x8000, CRC(89450a2f) SHA1(d58efa805f497bec179fdbfb8c5860ac5438b4ec) ) /* 27C256 eprom with no label */

	ROM_REGION( 0x08000, "cpu2", 0 ) /* Sound (80c301) CPU Code */
	ROM_LOAD( "u111", 0x0000, 0x8000, CRC(5d1d1387) SHA1(91c8aa4c7472b91c149bef9da64569a97df35298) ) /* 27C256 eprom with no label */

	ROM_REGION( 0x080000, "music", 0 ) /* Music data */
	ROM_LOAD( "u108", 0x00000, 0x80000, CRC(89233144) SHA1(74e87679a7559450934b80fcfcb667d9845977a7) ) /* 27C040 eprom with no label */

	ROM_REGION( 0x080000, "sfx", 0 ) /* QDSP samples (SFX) */
	ROM_LOAD( "u97", 0x00000, 0x80000, CRC(47b9d43a) SHA1(e0bc42892480cb563dc694fcefa8ca0b984749dd) ) /* 27C040 eprom with no label */

	ROM_REGION( 0x080000, "wavetable", 0 ) /* QDSP wavetable rom */
	ROM_LOAD( "qs1001a.u96",  0x000000, 0x80000, CRC(d13c6407) SHA1(57b14f97c7d4f9b5d9745d3571a0b7115fbe3176) )
ROM_END

ROM_START( ironfortj )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* Hyperstone CPU Code */
	ROM_LOAD( "u43.bin", 0x00000, 0x80000, CRC(f1f19c9a) SHA1(98531ecedd1277e6d10395794a66d615df7ddbd6) ) /* 27C040 eprom with no label */

	ROM_REGION32_BE( 0x2000000, "user1", ROMREGION_ERASE00 ) /* Game Data - banked ROM, swapping necessary */
	ROM_LOAD32_WORD_SWAP( "if00-00.u39", 0x0000000, 0x400000, CRC(63b74601) SHA1(c111ecf55359e9005a3ec1fe1202a34624f8b242) )
	ROM_LOAD32_WORD_SWAP( "if00-01.u34", 0x0000002, 0x400000, CRC(890470b3) SHA1(57df122ab01744b47ebd38554eb6a7d780977be2) )
	ROM_LOAD32_WORD_SWAP( "if00-02.u40", 0x0800000, 0x400000, CRC(63b5cca5) SHA1(4ec8b813c7e465f659a4a2361ddfbad763bf6e6a) )
	ROM_LOAD32_WORD_SWAP( "if00-03.u35", 0x0800002, 0x400000, CRC(54a76cb5) SHA1(21fb3bedf065079d59f642b19487f76590f97558) )

	ROM_REGION( 0x008000, "cpu1", 0 ) /* QDSP ('51) Code */
	ROM_LOAD( "u107", 0x0000, 0x8000, CRC(89450a2f) SHA1(d58efa805f497bec179fdbfb8c5860ac5438b4ec) ) /* 27C256 eprom with no label */

	ROM_REGION( 0x08000, "cpu2", 0 ) /* Sound (80c301) CPU Code */
	ROM_LOAD( "u111", 0x0000, 0x8000, CRC(5d1d1387) SHA1(91c8aa4c7472b91c149bef9da64569a97df35298) ) /* 27C256 eprom with no label */

	ROM_REGION( 0x080000, "music", 0 ) /* Music data */
	ROM_LOAD( "u108", 0x00000, 0x80000, CRC(89233144) SHA1(74e87679a7559450934b80fcfcb667d9845977a7) ) /* 27C040 eprom with no label */

	ROM_REGION( 0x080000, "sfx", 0 ) /* QDSP samples (SFX) */
	ROM_LOAD( "u97", 0x00000, 0x80000, CRC(47b9d43a) SHA1(e0bc42892480cb563dc694fcefa8ca0b984749dd) ) /* 27C040 eprom with no label */

	ROM_REGION( 0x080000, "wavetable", 0 ) /* QDSP wavetable rom */
	ROM_LOAD( "qs1001a.u96",  0x000000, 0x80000, CRC(d13c6407) SHA1(57b14f97c7d4f9b5d9745d3571a0b7115fbe3176) )
ROM_END

/* Hidden Catch */

ROM_START( hidnctch )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* Hyperstone CPU Code */
	ROM_LOAD( "hc_u43.bin", 0x00000, 0x80000, CRC(635b4478) SHA1(31ea4a9725e0c329447c7d221c22494c905f6940) )

	ROM_REGION32_BE( 0x2000000, "user1", ROMREGION_ERASE00 ) /* Game Data - banked ROM, swapping necessary */
	ROM_LOAD32_WORD_SWAP( "hc0_u39.bin", 0x0000000, 0x400000, CRC(eefb6add) SHA1(a0f6f2cf86699a666be0647274d8c9381782640d) )
	ROM_LOAD32_WORD_SWAP( "hc1_u34.bin", 0x0000002, 0x400000, CRC(482f3e52) SHA1(7a527c6af4c80e10cc25219a04ccf7c7ea1b23af) )
	ROM_LOAD32_WORD_SWAP( "hc2_u40.bin", 0x0800000, 0x400000, CRC(914a1544) SHA1(683cb007ace50d1ba88253da6ad71dc3a395299d) )
	ROM_LOAD32_WORD_SWAP( "hc3_u35.bin", 0x0800002, 0x400000, CRC(80c59133) SHA1(66ca4c2c014c4a1c87c46a3971732f0a2be95408) )
	ROM_LOAD32_WORD_SWAP( "hc4_u41.bin", 0x1000000, 0x400000, CRC(9a9e2203) SHA1(a90f5842b63696753e6c16114b1893bbeb91e45c) )
	ROM_LOAD32_WORD_SWAP( "hc5_u36.bin", 0x1000002, 0x400000, CRC(74b1719d) SHA1(fe2325259117598ad7c23217426ac9c28440e3a0) )
	// 0x1800000 - 0x1ffffff empty

	ROM_REGION( 0x008000, "cpu1", 0 ) /* QDSP ('51) Code */
	ROM_LOAD( "hc_u107.bin", 0x0000, 0x8000, CRC(afd5263d) SHA1(71ace1b749d8a6b84d08b97185e7e512d04e4b8d) )

	ROM_REGION( 0x008000, "cpu2", 0 )  /* Sound (80c301) CPU Code */
	ROM_LOAD( "hc_u111.bin", 0x0000, 0x8000, CRC(79012474) SHA1(09a2d5705d7bc52cc2d1644c87c1e31ee44813ef) )

	ROM_REGION( 0x080000, "music", 0 ) /* Music data */
	ROM_LOAD( "hc_u108.bin", 0x00000, 0x80000, CRC(2bae46cb) SHA1(7c43f1002dfc20b9c1bb1647f7261dfa7ed2b4f9) )

	ROM_REGION( 0x080000, "sfx", 0 ) /* QDSP samples (SFX) */
	ROM_LOAD( "hc_u97.bin", 0x00000, 0x80000, CRC(ebf9f77b) SHA1(5d472aeb84fc011e19b9e61d34aeddfe7d6ac216) )

	ROM_REGION( 0x080000, "wavetable", 0 ) /* QDSP wavetable rom */
	ROM_LOAD( "qs1001a.u96",  0x000000, 0x80000, CRC(d13c6407) SHA1(57b14f97c7d4f9b5d9745d3571a0b7115fbe3176) )
ROM_END

/*

New Hidden Catch
Eolith, 1999

PCB Layout
----------

GRADATION J.VER 3.02
|-------------------------------------------------|
|    TDA1519 VOL    KM6161002 KM6161002  IS61C1024|
|            VOL                                  |
|                   KM6161002 KM6161002  IS61C1024|
|     24MHz       QS1001A  DSW(4)                 |
|          QS1000              DSW(4)    IS61C1024|
|            U107   U97  EV0514-001               |
|J                                       IC61C1024|
|A                     14.31818MHz                |
|M  SERVICE_SW                        E1-32N      |
|M  TEST_SW                                       |
|A     93C66                        45MHz         |
|                                                 |
|                                                 |
|            GMS80C301                            |
|       12MHz                                     |
|                                                 |
|           U108    U41   U39   U37   U35    U43  |
|      U111      U42   U40   U38   U36   U34      |
|                                                 |
|-------------------------------------------------|
Notes:
      E1-32N       - Hyperstone E1-32N CPU, clock 45.000MHz (QFP160)
      80C301 clock - 12.000MHz
      IS61C1024    - ISSI 128k x8 High Speed CMOS Static RAM (SOJ32)
      KM6161002    - Samsung 64k x16 Ultra High Speed CMOS Video Static RAM (SOJ44)
      TDA1519      - Audio Power AMP
      QS1000       - QDSP QS1000 AdMOS 9638R, Wavetable Audio chip, clock input of 24.000MHz (QFP100)
      QS1001A      - QDSP QS1001A 512k x8 MaskROM (SOP32)
      EV0514-001   - Custom Eolith IC (QFP100)
      VSync        - 60Hz
      HSync        - 15.64kHz
*/

ROM_START( nhidctch )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* Hyperstone CPU Code */
	ROM_LOAD( "u43",        0x00000, 0x80000, CRC(44296fdb) SHA1(1faf7061342d4c86f6ca416d922cb98ffb72f250) )

	ROM_REGION32_BE( 0x2000000, "user1", ROMREGION_ERASE00 ) /* Game Data - banked ROM, swapping necessary */
	ROM_LOAD32_WORD_SWAP( "hc000.u39",   0x0000000, 0x400000, CRC(eefb6add) SHA1(a0f6f2cf86699a666be0647274d8c9381782640d) )
	ROM_LOAD32_WORD_SWAP( "hc001.u34",   0x0000002, 0x400000, CRC(482f3e52) SHA1(7a527c6af4c80e10cc25219a04ccf7c7ea1b23af) )
	ROM_LOAD32_WORD_SWAP( "hc002.u40",   0x0800000, 0x400000, CRC(914a1544) SHA1(683cb007ace50d1ba88253da6ad71dc3a395299d) )
	ROM_LOAD32_WORD_SWAP( "hc003.u35",   0x0800002, 0x400000, CRC(80c59133) SHA1(66ca4c2c014c4a1c87c46a3971732f0a2be95408) )
	ROM_LOAD32_WORD_SWAP( "hc004.u41",   0x1000000, 0x400000, CRC(9a9e2203) SHA1(a90f5842b63696753e6c16114b1893bbeb91e45c) )
	ROM_LOAD32_WORD_SWAP( "hc005.u36",   0x1000002, 0x400000, CRC(74b1719d) SHA1(fe2325259117598ad7c23217426ac9c28440e3a0) )
	ROM_LOAD32_WORD_SWAP( "hc006.u42",   0x1800000, 0x400000, CRC(2ec58049) SHA1(2a95d615f397cc4befdf92fcfca64a2726f6a791) )
	ROM_LOAD32_WORD_SWAP( "hc007.u37",   0x1800002, 0x400000, CRC(07e25def) SHA1(6e52a897cb2894625721010b8468ff237930b19b) )

	ROM_REGION( 0x008000, "cpu1", 0 ) /* QDSP ('51) Code */
	ROM_LOAD( "u107",        0x0000, 0x8000, CRC(afd5263d) SHA1(71ace1b749d8a6b84d08b97185e7e512d04e4b8d) )

	ROM_REGION( 0x008000, "cpu2", 0 )  /* Sound (80c301) CPU Code */
	ROM_LOAD( "u111",        0x0000, 0x8000, CRC(79012474) SHA1(09a2d5705d7bc52cc2d1644c87c1e31ee44813ef) )

	ROM_REGION( 0x080000, "music", 0 ) /* Music data */
	ROM_LOAD( "u108",        0x00000, 0x80000, CRC(2bae46cb) SHA1(7c43f1002dfc20b9c1bb1647f7261dfa7ed2b4f9) )

	ROM_REGION( 0x080000, "sfx", 0 ) /* QDSP samples (SFX) */
	ROM_LOAD( "u97",        0x00000, 0x80000, CRC(ebf9f77b) SHA1(5d472aeb84fc011e19b9e61d34aeddfe7d6ac216) )

	ROM_REGION( 0x080000, "wavetable", 0 ) /* QDSP wavetable rom */
	ROM_LOAD( "qs1001a.u96",  0x000000, 0x80000, CRC(d13c6407) SHA1(57b14f97c7d4f9b5d9745d3571a0b7115fbe3176) )
ROM_END

/*
Hidden Catch 2
Eolith, 1999

PCB Layout
----------

GRADATION VER 3.03M
|-------------------------------------------------|
|    TDA1519 VOL    KM6161002 KM6161002  IS61C1024|
|            VOL                                  |
|         DA1311    KM6161002 KM6161002  IS61C1024|
|     24MHz       QS1001A  DSW(4)                 |
|          QS1000              DSW(4)    IS61C1024|
|            U107   U97  EV0514-001               |
|J                                       IC61C1024|
|A                     14.31818MHz                |
|M  SERVICE_SW                        E1-32N      |
|M  TEST_SW                                       |
|A     93C66                        50MHz         |
|                                                 |
|              |---------Sub Board (above)--------|
|            AT89C52                         7705 |
|       12MHz  |                                  |
|              |                                  |
|           U108    U41   U39   U37   U35    U43  |
|      U111    | U42   U40   U38   U36   U34      |
|              |                                  |
|--------------|----------------------------------|
Notes:
      E1-32N       - Hyperstone E1-32N CPU, clock 50.000MHz (QFP160)
      AT89C52      - Atmel AT89C52 Microcontroller w/8k internal FlashROM, clock 12.000MHz
      IS61C1024    - ISSI 128k x8 High Speed CMOS Static RAM (SOJ32)
      KM6161002    - Samsung 64k x16 Ultra High Speed CMOS Video Static RAM (SOJ44)
      TDA1519      - Audio Power AMP
      QS1000       - QDSP QS1000 AdMOS 9638R, Wavetable Audio chip, clock input of 24.000MHz (QFP100)
      QS1001A      - QDSP QS1001A 512k x8 MaskROM (SOP32)
      EV0514-001   - Custom Eolith IC (QFP100)
      VSync        - 60Hz
      HSync        - 15.64kHz

      Main board has ROMs populated at U108, U43, U107 & U97 ONLY
      Sub Board contains 16x MX29F1610 16M FlashROMs. The filename is the location stamped on the PCB (SOP44)
*/

ROM_START( hidctch2 )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* Hyperstone CPU Code */
	ROM_LOAD( "u43",        0x00000, 0x80000, CRC(326d1dbc) SHA1(b33434cd263dc40ee2b6562f72a87a0439a9833e) )

	ROM_REGION32_BE( 0x2000000, "user1", ROMREGION_ERASE00 ) /* Game Data - banked ROM, swapping necessary */
	ROM_LOAD32_WORD_SWAP( "00", 0x0000000, 0x200000, CRC(c6b1bc84) SHA1(205d1dc5079562b11cef72fef25a3c570eaecf78) )
	ROM_LOAD32_WORD_SWAP( "01", 0x0000002, 0x200000, CRC(5a1c1ab3) SHA1(3c07a98f9ea8b30bac5a260403e688314fd12abb) )
	ROM_LOAD32_WORD_SWAP( "02", 0x0400000, 0x200000, CRC(3f7815aa) SHA1(ed46cbe03fde5cab15e004812036b0aaa00fc628) )
	ROM_LOAD32_WORD_SWAP( "03", 0x0400002, 0x200000, CRC(d686c59b) SHA1(f82d97f17d9cb10cae4d47f4efdd3ba6c5baace3) )
	ROM_LOAD32_WORD_SWAP( "04", 0x0800000, 0x200000, CRC(d35cb515) SHA1(75943b6c8052232c8b01ab57b4d32d54a22e7279) )
	ROM_LOAD32_WORD_SWAP( "05", 0x0800002, 0x200000, CRC(7870e5c6) SHA1(e36a62cb205d396337abdea8e553d70b6da32f8f) )
	ROM_LOAD32_WORD_SWAP( "06", 0x0c00000, 0x200000, CRC(10184a21) SHA1(fc99bc003ba69b651691ca37a5490beae29e36bc) )
	ROM_LOAD32_WORD_SWAP( "07", 0x0c00002, 0x200000, CRC(b6c4879f) SHA1(b672d5d7fc3dd1b32cd2f1084a71d1a225594866) )
	ROM_LOAD32_WORD_SWAP( "08", 0x1000000, 0x200000, CRC(670204d1) SHA1(efdead17ea1d1796f47abf65dc4792a10406b45c) )
	ROM_LOAD32_WORD_SWAP( "09", 0x1000002, 0x200000, CRC(28c0f55c) SHA1(0390aaf272deb1338293823d9d3a27306b2d9db6) )
	ROM_LOAD32_WORD_SWAP( "10", 0x1400000, 0x200000, CRC(45f374f4) SHA1(a5cec2fcb58445e0ff74c86b20aba7accd132ea5) )
	ROM_LOAD32_WORD_SWAP( "11", 0x1400002, 0x200000, CRC(cac54db3) SHA1(c31366e22d242ec0c0bb9f79869352894173db88) )
	ROM_LOAD32_WORD_SWAP( "12", 0x1800000, 0x200000, CRC(66e681ff) SHA1(8f78dec1dc3e5c6825f559cef69e413c98ed9aab) )
	ROM_LOAD32_WORD_SWAP( "13", 0x1800002, 0x200000, CRC(14bd38a9) SHA1(a3ff48de8150616a8ca49abf60225455397b938c) )
	ROM_LOAD32_WORD_SWAP( "14", 0x1c00000, 0x200000, CRC(8eb1b01b) SHA1(e144ba01aa65dc96e22d1d41c3ca87ae19e874d5) )
	ROM_LOAD32_WORD_SWAP( "15", 0x1c00002, 0x200000, CRC(3b06fe4e) SHA1(35356a116e6b825b8ed93c8ea1e016491ac1863a) )

	ROM_REGION( 0x008000, "cpu1", 0 ) /* QDSP ('51) Code */
	ROM_LOAD( "u107",        0x0000, 0x8000, CRC(89450a2f) SHA1(d58efa805f497bec179fdbfb8c5860ac5438b4ec) )

	ROM_REGION( 0x000800, "cpu2", 0 ) /* AT89c52 */
	ROM_LOAD( "hc2.103",     0x0000, 0x0800, NO_DUMP ) /* MCU internal 8K flash */

	ROM_REGION( 0x080000, "music", 0 ) /* Music data */
	ROM_LOAD( "u108",        0x00000, 0x80000, CRC(75fc7a65) SHA1(308715ab62d28787ee894ddcf7304464e2543b2e) )

	ROM_REGION( 0x080000, "sfx", 0 ) /* QDSP samples (SFX) */
	ROM_LOAD( "u97",        0x00000, 0x80000, CRC(a7a1627e) SHA1(a93ced858d839daac1fa9a85f4f8c89cb179bad5) )

	ROM_REGION( 0x080000, "wavetable", 0 ) /* QDSP wavetable rom */
	ROM_LOAD( "qs1001a.u96",  0x000000, 0x80000, CRC(d13c6407) SHA1(57b14f97c7d4f9b5d9745d3571a0b7115fbe3176) )
ROM_END


/*

Raccoon World
Eolith, 1998

This game runs on hardware that looks exactly like the Gradation 2D PCB
but there's no text or labelling on the PCB to say that. However, it is
an original Eolith PCB.

PCB Layout
----------

(No PCB number)
|-------------------------------------------------|
|    KA22065 VOL    KM6161002 KM6161002  IS61C1024|
|            VOL                                  |
|                   KM6161002 KM6161002  IS61C1024|
|     24MHz       QS1001A  DSW(4)                 |
|          QS1000                        IS61C1024|
|            U107   U97  EV0514-001               |
|J                                       IC61C1024|
|A                     14.31818MHz                |
|M                                    E1-32N      |
|M                                                |
|A     93C66                        45MHz         |
|             |-----ROM SUB BOARD------|          |
|             |                        |          |
|            GMS80C301                 |          |
|       12MHz |                        |          |
|             |                        |          |
|           U108    U41   U39   U37   U35    U43  |
|      U111   |  U42   U40   U38   U36 | U34      |
|             |                        |          |
|-------------|------------------------|----------|
Notes:
      E1-32N       - Hyperstone E1-32N CPU, clock 45.000MHz (QFP160)
      80C301 clock - 12.000MHz
      IS61C1024    - ISSI 128k x8 High Speed CMOS Static RAM (SOJ32)
      KM6161002    - Samsung 64k x16 Ultra High Speed CMOS Video Static RAM (SOJ44)
      KA22065      - Audio Power AMP
      QS1000       - QDSP QS1000 AdMOS 9638R, Wavetable Audio chip, clock input of 24.000MHz (QFP100)
      QS1001A      - QDSP QS1001A 512k x8 MaskROM (SOP32)
      EV0514-001   - Custom Eolith IC (QFP100)
      VSync        - 60Hz
      HSync        - 15.64kHz
      Note only U111, U108 & U43 are populated with EPROMs


ROM SUB BOARD
|------------------------|
|   U2    U4    U6    U8 |
|U1    U3    U5    U7    |
|                        |
|                        |
|U10   U12   U14   U16   |
|   U11   U13   U15   U17|
|                        |
|------------------------|
Only U1, U2 U5, U10, U11 & U14 populated
All ROMs 27C160 EPROM

*/

ROM_START( raccoon )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* Hyperstone CPU Code */
	ROM_LOAD( "racoon-u.43", 0x00000, 0x80000, CRC(711ee026) SHA1(c55dfaa24cbaa7a613657cfb25e7f0085f1e4cbf) )

	ROM_REGION32_BE( 0x2000000, "user1", ROMREGION_ERASE00 ) /* Game Data - banked ROM, swapping necessary */
	ROM_LOAD32_WORD_SWAP( "racoon.u10", 0x0000000, 0x200000, CRC(f702390e) SHA1(47520ba0e6d3f044136a517ebbec7426a66ce33d) )
	ROM_LOAD32_WORD_SWAP( "racoon.u1",  0x0000002, 0x200000, CRC(49775125) SHA1(2b8ee9dd767465999c828d65bb02b8aaad94177c) )
	ROM_LOAD32_WORD_SWAP( "racoon.u11", 0x0400000, 0x200000, CRC(3f23f368) SHA1(eb1ea51def2cde5e7e4f334888294b794aa03dfc) )
	ROM_LOAD32_WORD_SWAP( "racoon.u2",  0x0400002, 0x200000, CRC(1eb00529) SHA1(d9af75e116f5237a3c6812538b77155b9c08dd5c) )
	ROM_LOAD32_WORD_SWAP( "racoon.u14", 0x0800000, 0x200000, CRC(870fe45e) SHA1(f8d800b92eb1ee9ef4663319fd3cb1f5e52d0e72) )
	ROM_LOAD32_WORD_SWAP( "racoon.u5",  0x0800002, 0x200000, CRC(5fbac174) SHA1(1d3e3f40a737d61ff688627891dec183af7fa19a) )
	// 0x0c00000 - 0x1ffffff empty

	ROM_REGION( 0x08000, "cpu1", 0 ) /* QDSP ('51) Code */
	ROM_LOAD( "racoon-u.107", 0x0000, 0x8000, CRC(89450a2f) SHA1(d58efa805f497bec179fdbfb8c5860ac5438b4ec) )

	ROM_REGION( 0x08000, "cpu2", 0 ) /* Sound (80c301) CPU Code */
	ROM_LOAD( "racoon-u.111", 0x0000, 0x8000, CRC(52f419ea) SHA1(79c9f135b0cf8b1928411faed9b447cd98a83287) )

	ROM_REGION( 0x080000, "music", 0 ) /* Music data */
	ROM_LOAD( "racoon-u.108", 0x00000, 0x80000, CRC(fc4f30ee) SHA1(74b9e60cceb03ad572e0e080fbe1de5cffa1b2c3) )

	ROM_REGION( 0x080000, "sfx", 0 ) /* QDSP samples (SFX) */
	ROM_LOAD( "racoon-u.97", 0x00000, 0x80000, CRC(fef828b1) SHA1(38352b67d18300db40113df9426c2aceec12a29b) )

	ROM_REGION( 0x080000, "wavetable", 0 ) /* QDSP wavetable rom */
	ROM_LOAD( "qs1001a.u96",  0x000000, 0x80000, CRC(d13c6407) SHA1(57b14f97c7d4f9b5d9745d3571a0b7115fbe3176) )
ROM_END

/* Land Breaker */

ROM_START( landbrk )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* Hyperstone CPU Code */
	ROM_LOAD( "rom3.u43", 0x00000, 0x80000, CRC(8429189a) SHA1(f38e4792426ca2c138c44053a6c7525906281dff) )

	ROM_REGION32_BE( 0x2000000, "user1", ROMREGION_ERASE00 ) /* Game Data - banked ROM, swapping necessary */
	ROM_LOAD32_WORD_SWAP( "00.bin", 0x0000000, 0x200000, CRC(a803aace) SHA1(52a9e27b4f400767a953aa01321a9fa7cdbf3976) )
	ROM_LOAD32_WORD_SWAP( "01.bin", 0x0000002, 0x200000, CRC(439c95cc) SHA1(18830024b2e43f0a89a7bd32841fbcb574e50fc6) )
	ROM_LOAD32_WORD_SWAP( "02.bin", 0x0400000, 0x200000, CRC(a0c2828c) SHA1(967dc467d25f093749aa0146ebf54959c9803b92) )
	ROM_LOAD32_WORD_SWAP( "03.bin", 0x0400002, 0x200000, CRC(5106b61a) SHA1(5e584ce5247d598e59d071579edb1da87fb2c9d6) )
	ROM_LOAD32_WORD_SWAP( "04.bin", 0x0800000, 0x200000, CRC(fbe4a215) SHA1(2dec06a8641ef3db2a5448c95677ee8d7b0dc0ad) )
	ROM_LOAD32_WORD_SWAP( "05.bin", 0x0800002, 0x200000, CRC(61d422b2) SHA1(c9774747c94cac44ffd3c1781374c009950b2add) )
	ROM_LOAD32_WORD_SWAP( "06.bin", 0x0c00000, 0x200000, CRC(bc4f7f30) SHA1(31e7a6077a823f16a645547c9c11640535e02eb3) )
	ROM_LOAD32_WORD_SWAP( "07.bin", 0x0c00002, 0x200000, CRC(d4d108ce) SHA1(d462690b782b363db5a062e6e04fe0c00e21bd46) )
	ROM_LOAD32_WORD_SWAP( "08.bin", 0x1000000, 0x200000, CRC(36237654) SHA1(38a4d6d195683128c05d8869ff4184d79d26fead) )
	ROM_LOAD32_WORD_SWAP( "09.bin", 0x1000002, 0x200000, CRC(9171828b) SHA1(648a398654bfba2e7681a3eee61e3803538929b5) )
	ROM_LOAD32_WORD_SWAP( "10.bin", 0x1400000, 0x200000, CRC(fe51744d) SHA1(2f6fb5cc7a52d2fa3459b9b2cf3450a227c19643) )
	ROM_LOAD32_WORD_SWAP( "11.bin", 0x1400002, 0x200000, CRC(fe6fd5e0) SHA1(66442b0d308517ebf0e13653cf4289ee9e9e0741) )
	ROM_LOAD32_WORD_SWAP( "12.bin", 0x1800000, 0x200000, CRC(9f0f17f3) SHA1(c280dd5127ea6d1585f8b7ad753de45e1beaca8f) )
	ROM_LOAD32_WORD_SWAP( "13.bin", 0x1800002, 0x200000, CRC(c7d51bbe) SHA1(a399087496f1c3f4c4b9e9b97e97585302ff29e5) )
	ROM_LOAD32_WORD_SWAP( "14.bin", 0x1c00000, 0x200000, CRC(f30f7bc5) SHA1(8b3e002b773a88229d013e1d95336736a259c698) )
	ROM_LOAD32_WORD_SWAP( "15.bin", 0x1c00002, 0x200000, CRC(bc1664e3) SHA1(a13a4e1df8d9825a72ecca1552ee52958c0d33d8) )

	ROM_REGION( 0x008000, "cpu1", 0 ) /* QDSP ('51) Code */
	ROM_LOAD( "rom5.u107", 0x0000, 0x8000, CRC(afd5263d) SHA1(71ace1b749d8a6b84d08b97185e7e512d04e4b8d) )

	ROM_REGION( 0x008000, "cpu2", 0 ) /* Sound (80c301) CPU Code */
	ROM_LOAD( "rom1.u111", 0x0000, 0x8000, CRC(79012474) SHA1(09a2d5705d7bc52cc2d1644c87c1e31ee44813ef) )

	ROM_REGION( 0x080000, "music", 0 ) /* Music data */
	ROM_LOAD( "rom2.u108", 0x00000, 0x80000, CRC(f3b327ef) SHA1(4b359171afd6ca10275961f795f3fe64f9df9897) )

	ROM_REGION( 0x080000, "sfx", 0 ) /* QDSP samples (SFX) */
	ROM_LOAD( "rom4.u97", 0x00000, 0x80000, CRC(5b34dff0) SHA1(1668763e977e272781ddcc74beba97b53477cc9d) )

	ROM_REGION( 0x080000, "wavetable", 0 ) /* QDSP wavetable rom */
	ROM_LOAD( "qs1001a.u96",  0x000000, 0x80000, CRC(d13c6407) SHA1(57b14f97c7d4f9b5d9745d3571a0b7115fbe3176) )
ROM_END

ROM_START( landbrka )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* Hyperstone CPU Code */
	ROM_LOAD( "lb_1.u43", 0x00000, 0x80000, CRC(f8bbcf44) SHA1(ad85a890ae2f921cd08c1897b4d9a230ccf9e072) )

	ROM_REGION32_BE( 0x2000000, "user1", ROMREGION_ERASE00 ) /* Game Data - banked ROM, swapping necessary */
	ROM_LOAD32_WORD_SWAP( "lb2-000.u39", 0x0000000, 0x400000, CRC(b37faf7a) SHA1(30e9af3957ada7c72d85f55add221c2e9b3ea823) )
	ROM_LOAD32_WORD_SWAP( "lb2-001.u34", 0x0000002, 0x400000, CRC(07e620c9) SHA1(19f95316208fb4e52cef78f18c5d93460a644566) )
	ROM_LOAD32_WORD_SWAP( "lb2-002.u40", 0x0800000, 0x400000, CRC(3bb4bca6) SHA1(115029be4a4e322549a35f3ae5093ec161e9a421) )
	ROM_LOAD32_WORD_SWAP( "lb2-003.u35", 0x0800002, 0x400000, CRC(28ce863a) SHA1(1ba7d8be0ed4459dbdf99df18a2ad817904b9f04) )
	ROM_LOAD32_WORD_SWAP( "lb2-004.u41", 0x1000000, 0x400000, CRC(cbe84b06) SHA1(52505939fb88cd24f409c795fe5ceed5b41a52c2) )
	ROM_LOAD32_WORD_SWAP( "lb2-005.u36", 0x1000002, 0x400000, CRC(350c77a3) SHA1(231e65ea7db19019615a8aa4444922bcd5cf9e5c) )
	ROM_LOAD32_WORD_SWAP( "lb2-006.u42", 0x1800000, 0x400000, CRC(22c57cd8) SHA1(c9eb745523005876395ff7f0b3e996994b3f1220) )
	ROM_LOAD32_WORD_SWAP( "lb2-007.u37", 0x1800002, 0x400000, CRC(31f957b3) SHA1(ab1c4c50c2d5361ba8db047feb714423d84e6df4) )

	ROM_REGION( 0x008000, "cpu1", 0 ) /* QDSP ('51) Code */
	ROM_LOAD( "lb.107", 0x0000, 0x8000, CRC(afd5263d) SHA1(71ace1b749d8a6b84d08b97185e7e512d04e4b8d) )

	ROM_REGION( 0x000800, "cpu2", 0 ) /* AT89c52 */
	ROM_LOAD( "lb.103", 0x0000, 0x0800, NO_DUMP ) /* MCU internal 8K flash */

	ROM_REGION( 0x080000, "music", 0 ) /* Music data */
	ROM_LOAD( "lb_2.108", 0x00000, 0x80000, CRC(a99182d7) SHA1(628c8d09efb3917a4e97d9e02b6b0ca1f339825d) )

	ROM_REGION( 0x080000, "sfx", 0 ) /* QDSP samples (SFX) */
	ROM_LOAD( "lb_3.u97", 0x00000, 0x80000, CRC(5b34dff0) SHA1(1668763e977e272781ddcc74beba97b53477cc9d) )

	ROM_REGION( 0x080000, "wavetable", 0 ) /* QDSP wavetable rom */
	ROM_LOAD( "qs1001a.u96",  0x000000, 0x80000, CRC(d13c6407) SHA1(57b14f97c7d4f9b5d9745d3571a0b7115fbe3176) )
ROM_END


/* Fortress 2 Blue */

ROM_START( fort2b )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* Hyperstone CPU Code */
	ROM_LOAD( "1.u43",        0x00000, 0x80000, CRC(b2279485) SHA1(022591b260be28820f04a1c1fdd61cb9b68d6703) )

	ROM_REGION32_BE( 0x2000000, "user1", ROMREGION_ERASE00 ) /* Game Data - banked ROM, swapping necessary */
	ROM_LOAD32_WORD_SWAP( "00.u5", 0x0000000, 0x200000, CRC(4437b595) SHA1(b87518110955947264d93b1f377289f1741ce5dc) )
	ROM_LOAD32_WORD_SWAP( "01.u1", 0x0000002, 0x200000, CRC(2a410aed) SHA1(def822ead339180aa3e0ebb266b6a6eb1271a2ae) )
	ROM_LOAD32_WORD_SWAP( "02.u6", 0x0400000, 0x200000, CRC(12f0e4c0) SHA1(fa1e1c3510af61b4058507f1aca801377cafffb4) )
	ROM_LOAD32_WORD_SWAP( "03.u2", 0x0400002, 0x200000, CRC(aaa7c45a) SHA1(fa06932ab1d41eddd384785f1b2b4dc70046da0f) )
	ROM_LOAD32_WORD_SWAP( "04.u7", 0x0800000, 0x200000, CRC(428070d2) SHA1(3e25c644be28fd64e8ea60a5b6c675ce2a84ec91) )
	ROM_LOAD32_WORD_SWAP( "05.u3", 0x0800002, 0x200000, CRC(a66f9ba9) SHA1(3984c42358403b692b7e09b5849c2444578305f0) )
	ROM_LOAD32_WORD_SWAP( "06.u8", 0x0c00000, 0x200000, CRC(899d318e) SHA1(3bd552eae9985928fe211186f1913c155cbde1a5) )
	ROM_LOAD32_WORD_SWAP( "07.u4", 0x0c00002, 0x200000, CRC(c4644798) SHA1(3debeef3abc6ce4aea5eb0c6be9f99689a9f111c) )
	ROM_LOAD32_WORD_SWAP( "08.u15", 0x1000000, 0x200000, CRC(ce0cccfc) SHA1(42cb6698e9168712699616d46ece5a6482825e87) )
	ROM_LOAD32_WORD_SWAP( "09.u10", 0x1000002, 0x200000, CRC(5b7de0f1) SHA1(f0ee1c6d0766fec95d77d46a9c68b8c3b0d09dde) )
	ROM_LOAD32_WORD_SWAP( "10.u16", 0x1400000, 0x200000, CRC(b47fc014) SHA1(e1094bc8a3edff635c4abe311a7902a714b9102c) )
	ROM_LOAD32_WORD_SWAP( "11.u11", 0x1400002, 0x200000, CRC(7113d3f9) SHA1(5c34b2b21bb09c5dec075fabfd426c143a0a8dc2) )
	ROM_LOAD32_WORD_SWAP( "12.u17", 0x1800000, 0x200000, CRC(8c4b63a6) SHA1(2493c9e9547cb06fbdc6240fb67b15047b66a5fd) )
	ROM_LOAD32_WORD_SWAP( "13.u12", 0x1800002, 0x200000, CRC(1d9b9995) SHA1(e18b93c244d8e959053dff66e2e5e4341e2b8034) )
	ROM_LOAD32_WORD_SWAP( "14.u18", 0x1c00000, 0x200000, CRC(450fa784) SHA1(d1244b94be6571f1f8930154778362af5cd4c334) )
	ROM_LOAD32_WORD_SWAP( "15.u13", 0x1c00002, 0x200000, CRC(c1f02d5c) SHA1(73fe9e654c097cd57863b49545c6aa05996a7645) )

	ROM_REGION( 0x008000, "cpu1", 0 ) /* QDSP ('51) Code */
	ROM_LOAD( "5.u107",       0x00000, 0x08000, CRC(afd5263d) SHA1(71ace1b749d8a6b84d08b97185e7e512d04e4b8d) )

	ROM_REGION( 0x08000, "cpu2", 0 ) /* Sound (80c301) CPU Code */
	ROM_LOAD( "4.u111",       0x00000, 0x08000, CRC(79012474) SHA1(09a2d5705d7bc52cc2d1644c87c1e31ee44813ef) )

	ROM_REGION( 0x080000, "music", 0 ) /* Music data */
	ROM_LOAD( "3.u108",       0x00000, 0x80000, CRC(9b996b60) SHA1(c4e34601f754ae2908dd6d59ea9da0c5c6f56f2d) )

	ROM_REGION( 0x080000, "sfx", 0 ) /* QDSP samples (SFX) */
	ROM_LOAD( "2.u97",        0x00000, 0x80000, CRC(8a431b14) SHA1(5a9824280f30ef2e7b7f16652b2f9f9559cb764f) )

	ROM_REGION( 0x080000, "wavetable", 0 ) /* QDSP wavetable rom */
	ROM_LOAD( "qs1001a.u96",  0x000000, 0x80000, CRC(d13c6407) SHA1(57b14f97c7d4f9b5d9745d3571a0b7115fbe3176) )
ROM_END

ROM_START( fort2ba )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* Hyperstone CPU Code */
	ROM_LOAD( "ftii012.u43", 0x00000, 0x80000, CRC(6424e05f) SHA1(2f02f103de180561e372ce897f8410a11c4cb58d) )

	ROM_REGION32_BE( 0x2000000, "user1", ROMREGION_ERASE00 ) /* Game Data - banked ROM, swapping necessary */
	ROM_LOAD32_WORD_SWAP( "ftii000.u39", 0x0000000, 0x400000, CRC(be74121d) SHA1(ee072044f84e11c48537d79bd9766bf8cc28f002) )
	ROM_LOAD32_WORD_SWAP( "ftii004.u34", 0x0000002, 0x400000, CRC(d4399f98) SHA1(88f5a1097f44d2070cfc96c9cd83342d1975dcfe) )
	ROM_LOAD32_WORD_SWAP( "ftii001.u40", 0x0800000, 0x400000, CRC(35c396ff) SHA1(d05dee021e1a93e224b05949c18a5107e0aceb4d) )
	ROM_LOAD32_WORD_SWAP( "ftii005.u35", 0x0800002, 0x400000, CRC(ff553679) SHA1(d9f1ebc28098a20a59b76ee859527e946c82a1df) )
	ROM_LOAD32_WORD_SWAP( "ftii002.u41", 0x1000000, 0x400000, CRC(1d79ed5a) SHA1(0319392dbfe4a20be462ca9cc5a66575ba0a32b4) )
	ROM_LOAD32_WORD_SWAP( "ftii006.u36", 0x1000002, 0x400000, CRC(c6049bbc) SHA1(bd9231b5fe1a7307668960c1f9f188f4a49f1c45) )
	ROM_LOAD32_WORD_SWAP( "ftii003.u42", 0x1800000, 0x400000, CRC(3cac1efe) SHA1(aca009a39b5d6049e3cf234f4412868a569ffb18) )
	ROM_LOAD32_WORD_SWAP( "ftii007.u37", 0x1800002, 0x400000, CRC(a583a672) SHA1(b013fbfaa1e3f573a305e6346e50b930766daa1d) )

	ROM_REGION( 0x008000, "cpu1", 0 ) /* QDSP ('51) Code */
	ROM_LOAD( "ftii010.u107", 0x0000, 0x8000, CRC(afd5263d) SHA1(71ace1b749d8a6b84d08b97185e7e512d04e4b8d) )

	ROM_REGION( 0x08000, "cpu2", 0 ) /* Sound (80c301) CPU Code */
	ROM_LOAD( "ftii008.u111", 0x0000, 0x8000, CRC(79012474) SHA1(09a2d5705d7bc52cc2d1644c87c1e31ee44813ef) )

	ROM_REGION( 0x080000, "music", 0 ) /* Music data */
	ROM_LOAD( "ftii009.u108", 0x00000, 0x80000, CRC(9b996b60) SHA1(c4e34601f754ae2908dd6d59ea9da0c5c6f56f2d) )

	ROM_REGION( 0x080000, "sfx", 0 ) /* QDSP samples (SFX) */
	ROM_LOAD( "ftii011.u97", 0x00000, 0x80000, CRC(8a431b14) SHA1(5a9824280f30ef2e7b7f16652b2f9f9559cb764f) )

	ROM_REGION( 0x080000, "wavetable", 0 ) /* QDSP wavetable rom */
	ROM_LOAD( "qs1001a.u96",  0x000000, 0x80000, CRC(d13c6407) SHA1(57b14f97c7d4f9b5d9745d3571a0b7115fbe3176) )
ROM_END

/* Puzzle King */

ROM_START( puzzlekg )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* Hyperstone CPU Code */
	ROM_LOAD( "u43.bin",      0x00000, 0x80000, CRC(c3db7424) SHA1(5ee2be0f06fddb0c74fc6e82679b275cc4e86bcc) )

	ROM_REGION32_BE( 0x2000000, "user1", ROMREGION_ERASE00 ) /* Game Data - banked ROM, swapping necessary */
	ROM_LOAD32_WORD_SWAP( "u10.bin", 0x0000000, 0x200000, CRC(c9c3064b) SHA1(10a46d4674c1ef64e50dfcb5eb44953206fe6163) )
	ROM_LOAD32_WORD_SWAP( "u1.bin",  0x0000002, 0x200000, CRC(6b4b369d) SHA1(3f528e557f2846d7c50afa332797e8bc541eeba8) )
	ROM_LOAD32_WORD_SWAP( "u11.bin", 0x0400000, 0x200000, CRC(92615236) SHA1(dc602cb4c2a3d671cc60b075b399cf3efb67d3d3) )
	ROM_LOAD32_WORD_SWAP( "u2.bin",  0x0400002, 0x200000, CRC(e76bbd1d) SHA1(6c191e3c4c363132abf5f62882a40a0fbf0fb7ad) )
	ROM_LOAD32_WORD_SWAP( "u14.bin", 0x0800000, 0x200000, CRC(f5aa39d1) SHA1(6dde4c63cde388313370ed0cbf410965cfdfdaaa) )
	ROM_LOAD32_WORD_SWAP( "u5.bin",  0x0800002, 0x200000, CRC(88bb70cf) SHA1(9f6e7a97c7ee98d80a362395bcdcde25ceccec09) )
	ROM_LOAD32_WORD_SWAP( "u15.bin", 0x0c00000, 0x200000, CRC(bcc1f74d) SHA1(b2ee9b3761f77663a8f412fb3d3a724746e77c75) )
	ROM_LOAD32_WORD_SWAP( "u6.bin",  0x0c00002, 0x200000, CRC(ab2248ff) SHA1(fcef9028973e7e8cc3a0b8bb1f4a261f24f49081) )
	ROM_LOAD32_WORD_SWAP( "u12.bin", 0x1000000, 0x200000, CRC(1794973d) SHA1(09000781c87fa407a49addd35660517870fc997b) )
	ROM_LOAD32_WORD_SWAP( "u3.bin",  0x1000002, 0x200000, CRC(0980e877) SHA1(325687d8f9f8bfe55a53d3bd9a857504738a4633) )
	ROM_LOAD32_WORD_SWAP( "u13.bin", 0x1400000, 0x200000, CRC(31de6d19) SHA1(6d270d83f6c49fa8e0fc5a8ec63d8ba9e6c99105) )
	ROM_LOAD32_WORD_SWAP( "u4.bin",  0x1400002, 0x200000, CRC(2706f23c) SHA1(a4ade1c243640a26b673e3f9d83de8c0b6927c74) )
	ROM_LOAD32_WORD_SWAP( "u16.bin", 0x1800000, 0x200000, CRC(c2d09171) SHA1(d74ccc92cd11fef52a13f3bd2c4835131f3262c2) )
	ROM_LOAD32_WORD_SWAP( "u7.bin",  0x1800002, 0x200000, CRC(52405e69) SHA1(e27648bb1e02c539758bf9223efab13360f6cd55) )
	ROM_LOAD32_WORD_SWAP( "u17.bin", 0x1c00000, 0x200000, CRC(234b7261) SHA1(6bb95b8258133cc802f076c15e69c5412272e960) )
	ROM_LOAD32_WORD_SWAP( "u8.bin",  0x1c00002, 0x200000, CRC(8f4e50d7) SHA1(78808f0193a13467b45c84aef7f6a8f1cfe24feb) )

	ROM_REGION( 0x008000, "cpu1", 0 ) /* QDSP ('51) Code */
	ROM_LOAD( "u107.bin",    0x0000, 0x8000, CRC(f3add818) SHA1(96e77950154ced9f3234200de2aa29060c00d47f) )

	ROM_REGION( 0x08000, "cpu2", 0 ) /* Sound (80c301) CPU Code */
	ROM_LOAD( "u111.bin",    0x0000, 0x8000, CRC(79012474) SHA1(09a2d5705d7bc52cc2d1644c87c1e31ee44813ef) )

	ROM_REGION( 0x080000, "music", 0 ) /* Music data */
	ROM_LOAD( "u108.bin",    0x00000, 0x80000, CRC(e4555c6b) SHA1(128196a5b47d13ee7163981043b96f7b4b27204b) )

	ROM_REGION( 0x080000, "sfx", 0 ) /* QDSP samples (SFX) */
	ROM_LOAD( "u97.bin",     0x00000, 0x80000, CRC(f4604ce8) SHA1(e061b203ef5df386120dbf089ece094d16a1b59b) )

	ROM_REGION( 0x080000, "wavetable", 0 ) /* QDSP wavetable rom */
	ROM_LOAD( "qs1001a.u96",  0x000000, 0x80000, CRC(d13c6407) SHA1(57b14f97c7d4f9b5d9745d3571a0b7115fbe3176) )
ROM_END

/*
Hidden Catch 3
Eolith, 2000

PCB Layout
----------

GRADATION J. VER 3.05
|-------------------------------------------------|
|    TDA1519 VOL    KM6161002 KM6161002  IS61C1024|
|            VOL                                  |
|         DA1311    KM6161002 KM6161002  IS61C1024|
|     24MHz       QS1001A  DSW(4)                 |
|          QS1000              DSW(4)    IS61C1024|
|            U107   U97  EV0514-001               |
|J                                       IC61C1024|
|A                     14.31818MHz                |
|M  SERVICE_SW                        E1-32N      |
|M  TEST_SW                                       |
|A     93C66                        50MHz         |
|                                                 |
|              |---------Sub Board (above)--------|
|           GMS90C32                         7705 |
|       12MHz  |                                  |
|              |                                  |
|           U108    U41   U39   U37   U35    U43  |
|      U111    | U42   U40   U38   U36   U34      |
|              |                                  |
|--------------|----------------------------------|
Notes:
      E1-32N       - Hyperstone E1-32N CPU, clock 50.000MHz (QFP160)
      GMS90C32     - Hyundai GMS90C32 MCS-51 Compatible Microcontroller w/8k internal EPROM, clock 12.000MHz (PLCC44)
                     Note EA (External Access Enable on pin35) is LOW, thus it is configured to use the External EPROM at U111.
                     The chip is configured by placing a small smt zero Ohm resistor across 2 pads adjacent to pin35. So a romless
                     condition can be easily checked by looking to see if the resistor is present. If the resistor is not present,
                     EA is configured to HIGH and the chip is using it's internal ROM.
      IS61C1024    - ISSI 128k x8 High Speed CMOS Static RAM (SOJ32)
      KM6161002    - Samsung 64k x16 Ultra High Speed CMOS Video Static RAM (SOJ44)
      TDA1519      - Audio Power AMP
      QS1000       - QDSP QS1000 AdMOS 9638R, Wavetable Audio chip, clock input of 24.000MHz (QFP100)
      QS1001A      - QDSP QS1001A 512k x8 MaskROM (SOP32)
      EV0514-001   - Custom Eolith IC (QFP100)
      VSync        - 60Hz
      HSync        - 15.64kHz

      Main board has ROMs populated at U111, U108, U43, U107 & U97 ONLY and locations from U34 to U42 do not have sockets populated.
      Sub Board contains 16x MX29F1610 16M FlashROMs. The filename is the location stamped on the PCB (SOP44)

*/

ROM_START( hidctch3 )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* Hyperstone CPU Code */
	ROM_LOAD( "u43",        0x00000, 0x80000, CRC(7b861339) SHA1(fca7d47d7d538774ec6462deebc0a367ac073b67) )

	ROM_REGION32_BE( 0x2000000, "user1", ROMREGION_ERASE00 ) /* Game Data - banked ROM, swapping necessary */
	ROM_LOAD32_WORD_SWAP( "00", 0x0000000, 0x200000, CRC(7fe5cd46) SHA1(8190614a1cf1df6472590b43036200a1075bfe58) )
	ROM_LOAD32_WORD_SWAP( "01", 0x0000002, 0x200000, CRC(09463ec9) SHA1(0287da2e65521a61c06ad927e913243d023f0d72) )
	ROM_LOAD32_WORD_SWAP( "02", 0x0400000, 0x200000, CRC(e5a08ebf) SHA1(0ce2414b547a027710ee4ea8f890cc2fa23dff9a) )
	ROM_LOAD32_WORD_SWAP( "03", 0x0400002, 0x200000, CRC(b942b041) SHA1(e8b17766bc6ae966109cbfdc0d682c5bc19f196c) )
	ROM_LOAD32_WORD_SWAP( "04", 0x0800000, 0x200000, CRC(d25256de) SHA1(4f46f37b8245aea39a44cb43de36eefdbb8d8697) )
	ROM_LOAD32_WORD_SWAP( "05", 0x0800002, 0x200000, CRC(6ea89d41) SHA1(24df7802c2756bb8b03632287b17361924e5cb86) )
	ROM_LOAD32_WORD_SWAP( "06", 0x0c00000, 0x200000, CRC(67d3df6c) SHA1(2026c77fc5f621e5f3b2696d5acb91fabab4b439) )
	ROM_LOAD32_WORD_SWAP( "07", 0x0c00002, 0x200000, CRC(74698a22) SHA1(75370c2804c0f1b0ded3a487c2fc0107716d3e0c) )
	ROM_LOAD32_WORD_SWAP( "08", 0x1000000, 0x200000, CRC(0bf28c1f) SHA1(058e5157314687b6af3bd58495f3e2eaee9bf7f1) )
	ROM_LOAD32_WORD_SWAP( "09", 0x1000002, 0x200000, CRC(8a5960ce) SHA1(308f7aa586f781dbc280c288594d9a588d22220a) )
	ROM_LOAD32_WORD_SWAP( "10", 0x1400000, 0x200000, CRC(58999d26) SHA1(4f7a0050a4b2452563bb6af26236443ef2e24eb2) )
	ROM_LOAD32_WORD_SWAP( "11", 0x1400002, 0x200000, CRC(25f21007) SHA1(dc7d77ad35c34583d0dc08908535d94afcf9d10c) )
	ROM_LOAD32_WORD_SWAP( "12", 0x1800000, 0x200000, CRC(c39eba49) SHA1(eb9e15b7087d16029bfaddde8e8c2c447cb3cf73) )
	ROM_LOAD32_WORD_SWAP( "13", 0x1800002, 0x200000, CRC(20feaaf1) SHA1(b591b99e02596a94ad0aeab830dc550eb5c9a8a6) )
	ROM_LOAD32_WORD_SWAP( "14", 0x1c00000, 0x200000, CRC(6c042967) SHA1(e81ec2bf4fe5880d283e76bebf5e22d60f0588f9) )
	ROM_LOAD32_WORD_SWAP( "15", 0x1c00002, 0x200000, CRC(a49c0834) SHA1(64ca242cbf3ad6160b79ea2cb8ca4e4958d40e59) )

	ROM_REGION( 0x008000, "cpu1", 0 ) /* QDSP ('51) Code */
	ROM_LOAD( "u107",        0x0000, 0x8000, CRC(afd5263d) SHA1(71ace1b749d8a6b84d08b97185e7e512d04e4b8d) )

	ROM_REGION( 0x008000, "cpu2", 0 ) /* GMS90C32 */
	ROM_LOAD( "u111",        0x0000, 0x8000, CRC(79012474) SHA1(09a2d5705d7bc52cc2d1644c87c1e31ee44813ef) )

	ROM_REGION( 0x080000, "music", 0 ) /* Music data */
	ROM_LOAD( "u108",        0x00000, 0x80000, CRC(4a7de2e1) SHA1(89da2423f22f98886d7cac807964b1e52398ee19) )

	ROM_REGION( 0x080000, "sfx", 0 ) /* QDSP samples (SFX) */
	ROM_LOAD( "u97",        0x00000, 0x80000, CRC(6d37aa1a) SHA1(6827e500d9bf66e2e9236be563456ff88c78db91) )

	ROM_REGION( 0x080000, "wavetable", 0 ) /* QDSP wavetable rom */
	ROM_LOAD( "qs1001a.u96",  0x000000, 0x80000, CRC(d13c6407) SHA1(57b14f97c7d4f9b5d9745d3571a0b7115fbe3176) )
ROM_END

static DRIVER_INIT( eolith )
{
	init_eolith_speedup(machine);
}

static DRIVER_INIT( landbrk )
{
	coin_counter_bit = 0x1000;
	init_eolith_speedup(machine);
}

static DRIVER_INIT( landbrka )
{
	//it fails compares with memories:
	//$4002d338 -> $4002d348 .... $4002d33f -> $4002d34f
	//related with bits 0x100 - 0x200 read at startup from input(0) ?
	UINT32 *rombase = (UINT32*)memory_region(machine, "maincpu");
	rombase[0x14f00/4] = (rombase[0x14f00/4] & 0xffff) | 0x03000000; /* Change BR to NOP */

	coin_counter_bit = 0x2000;
	init_eolith_speedup(machine);
}

static DRIVER_INIT( hidctch2 )
{
	//it fails compares in memory like in landbrka
	UINT32 *rombase = (UINT32*)memory_region(machine, "maincpu");
	rombase[0xbcc8/4] = (rombase[0xbcc8/4] & 0xffff) | 0x03000000; /* Change BR to NOP */
	init_eolith_speedup(machine);
}

static DRIVER_INIT( hidctch3 )
{
	memory_nop_write(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xfc200000, 0xfc200003, 0, 0); // this generates pens vibration

	// It is not clear why the first reads are needed too

	memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xfce00000, 0xfce00003, 0, 0, hidctch3_pen1_r);
	memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xfce80000, 0xfce80003, 0, 0, hidctch3_pen1_r);

	memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xfcf00000, 0xfcf00003, 0, 0, hidctch3_pen2_r);
	memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xfcf80000, 0xfcf80003, 0, 0, hidctch3_pen2_r);

	init_eolith_speedup(machine);
}

GAME( 1998, ironfort,  0,        ironfort, ironfort,  eolith,   ROT0, "Eolith", "Iron Fortress", GAME_NO_SOUND )
GAME( 1998, ironfortj, ironfort, ironfort, ironfortj, eolith,   ROT0, "Eolith", "Iron Fortress (Japan)", GAME_NO_SOUND )
GAME( 1998, hidnctch,  0,        eolith45, hidnctch,  eolith,   ROT0, "Eolith", "Hidden Catch (World) / Tul Lin Gu Lim Chat Ki '98 (Korea) (pcb ver 3.03)",  GAME_NO_SOUND ) // or Teurrin Geurim Chajgi '98
GAME( 1998, raccoon,   0,        eolith45, raccoon,   eolith,   ROT0, "Eolith", "Raccoon World", GAME_NO_SOUND )
GAME( 1998, puzzlekg,  0,        eolith45, puzzlekg,  eolith,   ROT0, "Eolith", "Puzzle King (Dance & Puzzle)",  GAME_NO_SOUND )
GAME( 1999, hidctch2,  0,        eolith50, hidnctch,  hidctch2, ROT0, "Eolith", "Hidden Catch 2 (pcb ver 3.03)", GAME_NO_SOUND )
GAME( 1999, landbrk,   0,        eolith45, landbrk,   landbrk,  ROT0, "Eolith", "Land Breaker (World) / Miss Tang Ja Ru Gi (Korea) (pcb ver 3.02)",  GAME_NO_SOUND ) // or Miss Ttang Jjareugi
GAME( 1999, landbrka,  landbrk,  eolith45, landbrk,   landbrka, ROT0, "Eolith", "Land Breaker (World) / Miss Tang Ja Ru Gi (Korea) (pcb ver 3.03)",  GAME_NO_SOUND ) // or Miss Ttang Jjareugi
GAME( 1999, nhidctch,  0,        eolith45, hidnctch,  eolith,   ROT0, "Eolith", "New Hidden Catch (World) / New Tul Lin Gu Lim Chat Ki '98 (Korea) (pcb ver 3.02)", GAME_NO_SOUND ) // or New Teurrin Geurim Chajgi '98
GAME( 2000, hidctch3,  0,        eolith50, hidctch3,  hidctch3, ROT0, "Eolith", "Hidden Catch 3 (ver 1.00 / pcb ver 3.05)", GAME_NO_SOUND )
GAME( 2001, fort2b,    0,        eolith50, common,    eolith,   ROT0, "Eolith", "Fortress 2 Blue Arcade (ver 1.01 / pcb ver 3.05)",  GAME_NO_SOUND )
GAME( 2001, fort2ba,   fort2b,   eolith50, common,    eolith,   ROT0, "Eolith", "Fortress 2 Blue Arcade (ver 1.00 / pcb ver 3.05)",  GAME_NO_SOUND )
