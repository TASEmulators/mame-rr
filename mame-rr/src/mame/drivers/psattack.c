/*
    P's Attack
    using VRender0 System on a Chip

    This is basically the same hardware as Crystal System, but with a CF card for the game

    Skeleton driver only


P's Attack (c) 2004 Uniana Co., Ltd

+----------1||||---1|||||--1|||||---------------------------+
|VOL       TICKET  GUN_1P  GUN_2P                 +---------|
|                                                 |         |
+-+                                               |  256MB  |
  |       CC-DAC                                  | Compact |
+-+                                  EMUL*        |  Flash  |
|                                                 |         |
|J          +---+                                 +---------|
|A          |   |                                           |
|M          | R |   25.1750MHz              +--------------+|
|M          | A |                           |     42Pin*   ||
|A          | M |                           +--------------+|
|           |   |                           +--------------+|
|C          +---+       +------------+      |     SYS      ||
|O                      |            |      +--------------+|
|N          +---+       |            |                      |
|N          |   |       |VRenderZERO+|                      |
|E SERVICE  | R |       | MagicEyes  |  +-------+    62256* |
|C          | A |       |            |  |  RAM  |           |
|T TEST     | M |       |            |  +-------+    62256* |
|O          |   |       +------------+                      |
|R RESET    +---+                                           |
|                                   14.31818MHz             |
+-+                                                         |
  |                                EEPROM                   |
+-+                GAL                                 DSW  |
|                                                           |
|  VGA                           PIC               BAT3.6V* |
+-----------------------------------------------------------+

* denotes unpopulated device

RAM are Samsung K4S641632H-TC75
VGA is a standard PC 15 pin VGA connection
DSW is 2 switch dipswitch (switches 3-8 are unpopulated)
PIC is a Microchip PIC16C711-041/P (silkscreened on the PCB as COSTOM)
SYS is a ST M27C160 EPROM (silkscreened on the PCB as SYSTEM_ROM_32M)
GAL is a GAL16V8B (not dumped)
EMUL is an unpopulated 8 pin connector
EEPROM is a 93C86 16K 5.0v Serial EEPROM (2048x8-bit or 1024x16-bit)
CC-DAC is a TDA1311A Stereo Continuous Calibration DAC

TICKET is a 5 pin connector:

  1| +12v
  2| IN
  3| OUT
  4| GND
  5| LED

GUN_xP are 6 pin gun connectors (pins 1-4 match the UNICO sytle guns):

  1| GND
  2| SW
  3| +5v
  4| SENS
  5| SOL
  6| GND


*/

#include "emu.h"
#include "cpu/se3208/se3208.h"
#include "video/vrender0.h"
#include "machine/ds1302.h"
#include "sound/vrender0.h"

static READ32_HANDLER( psattack_unk_r )
{
	return 0xffffffff;
}

static ADDRESS_MAP_START( psattack_mem, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x001fffff) AM_ROM
	AM_RANGE(0x01402204, 0x01402207) AM_READ(psattack_unk_r)
	AM_RANGE(0x01402804, 0x01402807) AM_READ(psattack_unk_r)


	AM_RANGE(0x02000000, 0x027fffff) AM_RAM
ADDRESS_MAP_END


static MACHINE_START(psattack)
{

}

static MACHINE_RESET(psattack)
{

}

static VIDEO_START(psattack)
{

}


static VIDEO_UPDATE(psattack)
{
	return 0;
}

static VIDEO_EOF(psattack)
{

}

static INTERRUPT_GEN(psattack_interrupt)
{

}

static INPUT_PORTS_START(psattack)
	PORT_START("Unknown")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
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

static const vr0_interface vr0_config =
{
	0x04800000
};


static MACHINE_DRIVER_START( psattack )
	MDRV_CPU_ADD("maincpu", SE3208, 43000000)
	MDRV_CPU_PROGRAM_MAP(psattack_mem)
	MDRV_CPU_VBLANK_INT("screen", psattack_interrupt)

	MDRV_MACHINE_START(psattack)
	MDRV_MACHINE_RESET(psattack)

	//MDRV_NVRAM_HANDLER(generic_0fill)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(320, 240)
	MDRV_SCREEN_VISIBLE_AREA(0, 319, 0, 239)

	MDRV_VIDEO_START(psattack)
	MDRV_VIDEO_UPDATE(psattack)
	MDRV_VIDEO_EOF(psattack)

	MDRV_PALETTE_INIT(RRRRR_GGGGGG_BBBBB)
	MDRV_PALETTE_LENGTH(65536)

	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("vrender", VRENDER0, 0)
	MDRV_SOUND_CONFIG(vr0_config)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END


ROM_START( psattack )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD("5.sys",  0x000000, 0x200000, CRC(f09878e4) SHA1(25b8dbac47d3911615c8874746e420ece13e7181) )

	ROM_REGION( 0x4010, "pic16c711", 0 )
	ROM_LOAD("16c711.pic",  0x0000, 0x137b, CRC(617d8292) SHA1(d32d6054ce9db2e31efaf41015afcc78ed32f6aa) ) // raw dump
	ROM_LOAD("16c711.bin",  0x0000, 0x4010, CRC(b316693f) SHA1(eba1f75043bd415268eedfdb95c475e73c14ff86) ) // converted to binary

	DISK_REGION( "cfcard" )
	DISK_IMAGE_READONLY( "psattack", 0, SHA1(e99cd0dafc33ec13bf56061f81dc7c0a181594ee) )
ROM_END



static DRIVER_INIT(psattack)
{

}

GAME( 2004, psattack, 0, psattack, psattack, psattack, ROT0, "Uniana", "P's Attack", GAME_NOT_WORKING )

