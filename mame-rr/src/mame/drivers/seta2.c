/***************************************************************************

                          -= Newer Seta Hardware =-

                    driver by   Luca Elia (l.elia@tin.it)


CPU    :    TMP68301*
            ColdFire & H8/3007 (for FUNCUBE)

Custom :    X1-010              Sound: 8 Bit PCM
            DX-101              Sprites
            DX-102 x3

OSC:    50.00000MHz
        32.53047MHz

*   The Toshiba TMP68301 is a 68HC000 + serial I/O, parallel I/O,
    3 timers, address decoder, wait generator, interrupt controller,
    all integrated in a single chip.

-------------------------------------------------------------------------------------------
Ordered by Board        Year    Game                                    By
-------------------------------------------------------------------------------------------
P-FG01-1                1995    Guardians / Denjin Makai II             Banpresto
PO-113A                 1994    Mobile Suit Gundam EX Revue             Banpresto
P0-123A                 1996    Wakakusamonogatari Mahjong Yonshimai    Maboroshi Ware
P0-125A ; KE (Namco)    1996    Kosodate Quiz My Angel                  Namco
P0-136A ; KL (Namco)    1997    Kosodate Quiz My Angel 2                Namco
P0-142A                 1999    Puzzle De Bowling                       Nihon System / Moss
P0-142A + extra parts   2000    Penguin Brothers                        Subsino
B0-003A (or B0-003B)    2000    Deer Hunting USA                        Sammy
B0-003A (or B0-003B)    2001    Turkey Hunting USA                      Sammy
B0-006B                 2001    Funcube 2                               Namco
B0-006B                 2001    Funcube 4                               Namco
B0-010A                 2001    Wing Shooting Championship              Sammy
B0-010A                 2002    Trophy Hunting - Bear & Moose           Sammy
-------------------------------------------------------------------------------------------

TODO:

- Proper emulation of the TMP68301 CPU, in a core file.
- Proper emulation of the ColdFire CPU, in a core file.
- Flip screen / Zooming support.
- Fix some graphics imperfections (e.g. color depth selection,
  "tilemap" sprites) [all done? - NS]
- I added a kludge involving a -0x10 yoffset, this fixes the lifeline in myangel.
  I didn't find a better way to do it without breaking pzlbowl's title screen.

mj4simai:
- test mode doesn't work correctly, the grid is ok but when you press a key to go to the
  next screen (input test) it stays up a second and then drops back into the game

myangel:
- some gfx at the end of the game (rays just before fireworks, and the border during
  the wedding) have wrong colors. You can see the rays red, green and yellow because
  that's how the palette is preinitialized by MAME, but the game never sets up those
  palette entries. The game selects color depth "1", whose meaning is uncertain, and
  color code 0 so I see no way to point to a different section of palette RAM.

- there are glitches in the bg horizontal scroll in the wedding sequence at the end of
  the game. It looks like "scrollx" should be delayed one frame wrt "xoffs".

myangel2:
- before each level, the background image is shown with completely wrong colors. It
  corrects itself when the level starts.

grdians:
- the map screen after the character selection needs zooming. There is a global
  zoom register that should affect the background map and the level picture but
  not the frontmost frame. This latter should use color 7ff (the last one)
  and ignore the individual color codes in the tiles data. Zooming is also
  used briefly in pengbros.

deerhunt,wschamp:
- offset tilemap sprite during demo

trophyh:
- mame hangs for around 15 seconds every now and then, at scene changes.
  This is probably due to a couple of frames with an odd or corrupt sprites list,
  taking a long time to render.

funcube:
- Hacked to run, as they use a ColdFire CPU.
- Pay-out key causes "unknown error".

***************************************************************************/

/***************************************************************************

MS Gundam Ex Revue
Banpresto, 1994

This game runs on Seta/Allumer hardware

PCB Layout
----------

PO-113A   BP949KA
|----------------------------------|
|  X1-010  6264  U28               |
|                     581001   U19 |
|     U3  U5  U2  U4  581001   U17 |
|      62256   62256           U15 |
|J                             U20 |
|A    U77  68301               U18 |
|M                     *       U16 |
|M    93C46                    U23 |
|A                             U22 |
|                              U21 |
|  DSW1            50MHz           |
|  DSW2       PAL  32.5304MHz      |
|       20MHz PAL                  |
|----------------------------------|

Notes:
      *: unknown QFP208 (has large heatsink on it). Should be similar to other known
         graphics chips used on Seta hardware of this era.
      68301 clock: 16.000MHz (?? From what OSC + divider??)
      VSync: 60Hz

***************************************************************************/

/***************************************************************************

Guardians
Banpresto, 1995

This hardware is not common Banpresto hardware. Possibly licensed
to them from another manufacturer? Or an early design that they decided
not to use for future games? Either way, this game is _extremely_ rare :-)

PCB Layout
----------

P-FG01-1
------------------------------------------------------
|        X1-010 6264          U32 CXK581000          |
|                                 CXK581000      U16 |
|                                                    |
|                                                U20 |
|    U3 U5 U2 U4 62256 CXK58257                      |
|                62256 CXK58257                  U15 |
|                                                    |
|J                                               U19 |
|A    TMP68301AF-16                                  |
|M                                               U18 |
|M                           NEC                     |
|A          NEC              DX-101              U22 |
|           DX-102                                   |
|                                                U17 |
|                   PAL   50MHz                      |
|                                                U21 |
|           DSW1(8)                                  |
|           DSW2(8)                   CXK58257 NEC   |
|                                     CXK58257 DX-102|
------------------------------------------------------

Notes:
      HSync: 15.23kHz
      VSync: 58.5Hz

***************************************************************************/

/***************************************************************************

                            Penguin Brothers (Japan)

(c)2000 Subsino

   CPU: Toshiba TMP68301AF-16 (100 Pin PQFP)
 Video: NEC DX-101 (240 Pin PQFP)
        NEC DX-102 (52 Pin PQFP x3)
 Sound: X1-010 (Mitsubishi M60016 Gate Array, 80 Pin PQFP)
   OSC: 50MHz, 32.53047MHz & 28MHz
 Other: 8 Position Dipswitch x 2
        Lattice ispLSI2032

PCB Number: P0-142A
+-----------------------------------------------------------+
|                VOL                       +------+         |
|                                          |Seta  |     M1  |
|   +---+ +---+                            |X1-010|         |
|   |   | |   |   M   M                    |      |  +---+  |
+-+ | U | | U |   1   1                    +------+  |   |  |
  | | 0 | | 0 |                    74HC00            |   |  |
+-+ | 7 | | 6 |   M   M                              | U |  |
|   |   | |   |   1   1                              | 1 |  |
|   +---+ +---+                                      | 8 |  |
|                                          Lattice   |   |  |
|J  D D  +---+                            ispLSI2032 |   |  |
|A  S S  |DX |                  +-------+            +---+  |
|M  W W  |102|                  |Toshiba|     CN2           |
|M  1 2  +---+      BAT1*       |  TMP  |                   |
|A                              | 68301 |  U50*             |
|                               +-------+                   |
|C                                                          |
|o                 50MHz        +----------+     28MHz      |
|n    +---+                     |          |                |
|n    |DX |     SW1             |   NEC    |    M   M       |
|e    |102|                     |  DX-101  |    2   2       |
|c    +---+         M  M        |          |                |
|t                  1  1        |          |                |
|e                              +----------+                |
|r                                                          |
|                             +---+      +---++---++---+    |
|                             |   |      |   ||   ||   |    |
|     +---+                   |   |      |   ||   ||   |    |
+-+   |DX |                   | U |      | U || U || U |    |
  |   |102|     32.53047MHz   | 4 |      | 4 || 3 || 3 |    |
+-+   +---+                   | 0 |      | 1 || 8 || 9 |    |
|                             |   |      |   ||   ||   |    |
|                             |   |      |   ||   ||   |    |
|                             +---+      +---++---++---+    |
+-----------------------------------------------------------+

Notes:  pzlbowl PCB with these extra parts:
        28MHz OSC
        2x 62256 SRAM
        74HC00

U50*  Unpopulated 93LC46BX EEPROM
BAT1* Unpopulated CR2032 3Volt battery

Ram M1 are NEC D43001GU-70LL
Ram M2 are LGS GM76C8128ALLFW70

***************************************************************************/

/***************************************************************************

                            Puzzle De Bowling (Japan)

(c)1999 Nihon System / Moss

   CPU: Toshiba TMP68301AF-16 (100 Pin PQFP)
 Video: NEC DX-101 (240 Pin PQFP)
        NEC DX-102 (52 Pin PQFP x3)
 Sound: X1-010 (Mitsubishi M60016 Gate Array, 80 Pin PQFP)
   OSC: 50MHz & 32.53047MHz
 Other: 8 Position Dipswitch x 2
        Lattice ispLSI2032 - stamped "KUDEC"

PCB Number: P0-142A
+-----------------------------------------------------------+
|                VOL                       +------+         |
|                                          |Seta  |     M1  |
|   +---+ +---+                            |X1-010|         |
|   |   | |   |  U4*  M                    |      |  +---+  |
+-+ | U | | U |       1                    +------+  | K |  |
  | | 0 | | 0 |                     U30*             | U |  |
+-+ | 7 | | 6 |  U5*  M                              | S |  |
|   |   | |   |       1                              |   |  |
|   +---+ +---+                                      | U |  |
|                                          Lattice   | 1 |  |
|J  D D  +---+                            ispLSI2032 | 8 |  |
|A  S S  |DX |                  +-------+            +---+  |
|M  W W  |102|                  |Toshiba|     CN2           |
|M  1 2  +---+      BAT1*       |  TMP  |                   |
|A                              | 68301 |  U50*             |
|                               +-------+                   |
|C                                                          |
|o                 50MHz        +----------+     XM2*       |
|n    +---+                     |          |                |
|n    |DX |     SW1             |   NEC    |    M   M       |
|e    |102|                     |  DX-101  |    2   2       |
|c    +---+         M  M        |          |                |
|t                  1  1        |          |                |
|e                              +----------+                |
|r                                                          |
|                             +---+      +---++---++---+    |
|                             | K |      | K || K || K |    |
|     +---+                   | U |      | U || U || U |    |
+-+   |DX |                   | C |      | C || C || C |    |
  |   |102|     32.53047MHz   |   |      |   ||   ||   |    |
+-+   +---+                   | U |      | U || U || U |    |
|                             | 4 |      | 4 || 3 || 3 |    |
|                             | 0 |      | 1 || 8 || 9 |    |
|                             +---+      +---++---++---+    |
+-----------------------------------------------------------+

* Unpopulated:
  U4 & U5 RAM HM62256 equivalent
  U50 93LC46BX EEPROM
  U30 74HC00
  BAT1 CR2032 3Volt battery
  XM2 OSC

Ram M1 are NEC D43001GU-70LL
Ram M2 are LGS GM76C8128ALLFW70

KUP-U06-I03 U06 Program rom ST27C4001 (even)
KUP-U07-I03 U07 Program rom ST27C4001 (odd)

KUS-U18-I00 U18 Mask rom (Samples 23C32000 32Mbit)

KUC-U38-I00 U38 Mask rom (Graphics 23C32000 32Mbit)
KUC-U39-I00 U39 Mask rom (Graphics 23C32000 32Mbit)
KUC-U40-I00 U40 Mask rom (Graphics 23C32000 32Mbit)
KUC-U41-I00 U41 Mask rom (Graphics 23C32000 32Mbit)

***************************************************************************/

/***************************************************************************

Sammy USA Outdoor Shooting Series PCB

PCB B0-003A (or B0-003B):
   Deer Hunting USA (c) 2000 Sammy USA
   Turkey Hunting USA (c) 2001 Sammy USA

PCB B0-010A:
   Wing Shooting Championship (c) 2001 Sammy USA
   Trophy Hunting - Bear & Moose (c) 2002 Sammy USA


   CPU: Toshiba TMP68301AF-16 (100 Pin PQFP)
 Video: NEC DX-101 (240 Pin PQFP)
        NEC DX-102 (52 Pin PQFP x3)
 Sound: X1-010 (Mitsubishi M60016 Gate Array, 80 Pin PQFP)
EEPROM: 93LC46BX (1K Low-power 64 x 16-bit organization serial EEPROM)
   OSC: 50MHz & 28MHz
 Other: 8 Position Dipswitch x 2
        Lattice ispLSI2032 - stamped "KW001"
        Lattice isp1016E - stamped "GUN" (2 for PCB B0-010A, used for light gun input)
        BAT1 - CR2032 3Volt

PCB Number: B0-003A (or B0-003B)
+-----------------------------------------------------------+
|             VOL                          +------+         |
|                                          |X1-010|     M1  |
|   +---+ +---+                            |M60016|         |
|   |   | |   |  M    M                    |CALRUA|  +---+  |
+-+ | U | | U |  2    1                    +------+  |   |  |
  | | 0 | | 0 |                                      |   |  |
+-+ | 7 | | 6 |  M    M                              | U |  |
|   |   | |   |  2    1                              | 1 |  |
|   +---+ +---+                                      | 8 |  |
|                                          Lattice   |   |  |
|J  D +---+  C                            ispLSI2032 |   |  |
|A  S |DX |  N   BAT1           +-------+            +---+  |
|M  W |102|  5                  |Toshiba|  D                |
|M  1 +---+                     |  TMP  |  S EEPROM       C |
|A           C                  | 68301 |  W              N |
|            N  Lattice         +-------+  2              2 |
|C           6  isp1016E                                    |
|o                              +----------+    50MHz       |
|n    +---+                     |          |                |
|n    |DX |  SW1                |   NEC    |    M   M       |
|e    |102|                     |  DX-101  |    3   3       |
|c    +---+         M  M        |          |                |
|t                  1  1        |          |                |
|e                              +----------+                |
|r                                                          |
|                             +---+      +---++---++---+    |
|                  28MHz      |   |      |   ||   ||   |    |
|     +---+                   |   |      |   ||   ||   |    |
+-+   |DX |                   | U |      | U || U || U |    |
  |   |102|                   | 4 |      | 4 || 3 || 3 |    |
+-+   +---+                   | 0 |      | 1 || 8 || 9 |    |
|                             |   |      |   ||   ||   |    |
|                             |   |      |   ||   ||   |    |
|                             +---+      +---++---++---+    |
+-----------------------------------------------------------+

PCB Number: B0-010A - This PCB is slightly revised for 2 player play
+-----------------------------------------------------------+
|             VOL                          +------+         |
|                                          |X1-010|     M1  |
|   +---+ +---+                            |M60016|         |
|   |   | |   |  M    M                    |CALRUA|  +---+  |
+-+ | U | | U |  2    1                    +------+  |   |  |
  | | 0 | | 0 |                                      |   |  |
+-+ | 7 | | 6 |  M    M                              | U |  |
|   |   | |   |  2    1                              | 1 |  |
|   +---+ +---+                                      | 8 |  |
|                                          Lattice   |   |  |
|J  D +---+  C                            ispLSI2032 |   |  |
|A  S |DX |  N   BAT1           +-------+            +---+  |
|M  W |102|  5                  |Toshiba|  D                |
|M  1 +---+                     |  TMP  |  S EEPROM       C |
|A           C                  | 68301 |  W              N |
|            N  Lattice         +-------+  2              2 |
|C           6  isp1016E                                    |
|o                              +----------+    50MHz       |
|n    +---+                     |          |                |
|n    |DX |  SW1                |   NEC    |    M   M       |
|e    |102|                     |  DX-101  |    3   3       |
|c    +---+         M  M        |          |                |
|t                  1  1        |          |                |
|e                              +----------+                |
|r                                                          |
|                             +---+      +---++---++---+    |
|                  28MHz      |   |      |   ||   ||   |    |
|     +---+              C    |   |      |   ||   ||   |    |
+-+   |DX |              N    | U |      | U || U || U |    |
  |   |102|              7    | 4 |      | 4 || 3 || 3 |    |
+-+   +---+                   | 0 |      | 1 || 8 || 9 |    |
|             Lattice    C    |   |      |   ||   ||   |    |
|             isp1016E   N    |   |      |   ||   ||   |    |
|                        8    +---+      +---++---++---+    |
+-----------------------------------------------------------+

Ram M1 are Toshiba TC55257DFL-70L
Ram M2 are NEC D43001GU-70L
Ram M3 are ISSI IS62C1024L-70Q

U06 Program rom ST27C801 (even)
U07 Program rom ST27C801 (odd)

U18 Mask rom (Samples 23C32000 32Mbit (read as 27C322))

U38 - U40 Mask roms (Graphics 23c64020 64Mbit) - 23C64020 read as 27C322 with pin11 +5v & 27C322 with pin11 GND

--------------------------------------------------------------------------

From the WSC upgrade instruction sheet:

 Wing Shooting Championship
      Game Echancement
          1/23/02

New Program chip Ver. 2.00 For Wing Shooting Championship
We are announcing NEW GAME FEATURES to enhance game play. Please refer below.

NEW FEATURES
------------

 * Easier play for the first 3 hunting spots in every state with the addition of more birds.
 * The "BEGINNER" weapon has been changed to the 5-shot PUMP SHOTGUN plus the "hit area"
    for each shot has been increased. Same as the 3-shot SEMI-AUTO SHOTGUN.
 * Player can now advance through all result screens faster by pulling gun trigger.
 * The Auto Select bird is now GOOSE (easiest target) if player fails to choose bird at start of game.

***************************************************************************/

/***************************************************************************

               FUNCUBE (BET) Series, includes 2 through 5?

PCB Number: B0-006B (also known as EVA3_A system and is non-JAMMA)
+------------------------------------------------+
|+--+ S +---+ +---+                CN5           |
||  | W |   | |   |                          CN6?|
||  | 4 | U | | U |                              |
||  |   | 4 | | 4 |     +---+                CN2?|
||  |   | 2 | | 3 |     |DX |                    |
||  |   |   | |   |     |102|                    |
||C |   |   | |   |     +---+                    |
||N |   +---+ +---+                              |
||4 |                                            |
||  |      +----------+   M1                     |
||  |  M3  |          |                        C |
||  |      |   NEC    |   M1                   N |
||  |  M3  |  DX-101  |                        3 |
||  |      |          |                          |
||  |      |          |   50MHz                  |
|+--+      +----------+                          |
| PIC  25.447MHz         +-----------+           |
|  CN7                   |    U47    |           |
|                        +-----------+           |
|          +-----------+  +---+ +---+       D    |
|          |     U3    |  |OKI| |DX |       S    |
|    M2    +-----------+  |   | |102|       W    |
|                         +---+ +---+       1    |
|                 ispLSI2032                     |
|    M1                      +---+               |
|          +----------+      |IDT|           +--+|
|          |          |  C   |   |           |  ||
| C        | ColdFire |  N   +---+           |  ||
| N  M2    | XCF5206E |  8                   |  ||
| 1        |          |        +---+         |C ||
|          |          |        |H8 |         |N ||
|    M1    +----------+        +---+      D  |9 ||
|                         14.7456MHz      S  |  ||
|                            +-----------+W  |  ||
|            SW1      BAT1   |    U49    |2  +--+|
|                            +-----------+       |
+------------------------------------------------+

   CPU: ColdFire XCF5206EFT54 (160 Pin PQFP)
        Hitachi H8/3007 (64130007F20) used for touch screen I/O
 Video: NEC DX-101 (240 Pin PQFP)
        NEC DX-102 (52 Pin PQFP x2)
 Sound: OKI MSM9810B 8-Channel Mixing ADPCM Type Voice Synthesis LSI
   OSC: 50MHz, 25.447MHz & 14.7456MHz
 Other: Lattice ispLSI2032 - stamped "EVA3A"
        BAT1 - CR2032 3Volt

ColdFire XCF5206EFT54:
  68K/ColdFire V2 core family
  8K internal SRAM
  54MHz (max) Bus Frequency
  32bit External Bus Width
  2 UART Serial Interfaces
  2 Timer Channels

PIC - PIC12C508 MCU used for security
       Labeled FC21A for Funcube 2
       Labeled FC41A for Funcube 4

Ram M1 are Toshiba TC55257DFL-70L
Ram M2 are NEC D43001GU-70L
Ram M3 are ISSI IS62C1024L-70Q
IDT - IDT 7130 64-pin TQFP High-speed 1K x 8 Dual-Port Static RAM

CN1 - Unused 64 pin double row connecter
CN2?  2x2 connecter
CN3 - Unused 50 pin double row connecter
CN4 - 96 pin triple row connecter
CN5 - 2x3 pin connecter
CN6?  3x3 connecter
CN7 - Unused 20 pin connecter
CN8 - 8 pin single row connecter
CN9 - 40 pin double row connecter

DSW1 - 8 position dipswitch
DSW2 - 2 position dipswitch
SW1  - Pushbutton
SW4  - Single position slider switch

U3  - Is a 27C4002 EPROM
U49 - Is a 27C1001 EPROM
U42, U43 & U47 are MASK ROMs read as 27C322

The same H8/3007 code "FC21 IOPR-0" at U49 is used for FUNCUBE 2,3,4 & 5

***************************************************************************/

#include "emu.h"
#include "memconv.h"
#include "deprecat.h"
#include "cpu/m68000/m68000.h"
#include "machine/tmp68301.h"
#include "cpu/h83002/h8.h"
#include "machine/eeprom.h"
#include "sound/x1_010.h"
#include "includes/seta.h"

/***************************************************************************


                            Memory Maps - Main CPU


***************************************************************************/

static WRITE16_HANDLER( seta2_sound_bank_w )
{
	if (ACCESSING_BITS_0_7)
	{
		UINT8 *ROM = memory_region( space->machine, "x1snd" );
		int banks = (memory_region_length( space->machine, "x1snd" ) - 0x100000) / 0x20000;
		if (data >= banks)
		{
			logerror("CPU #0 PC %06X: invalid sound bank %04X\n",cpu_get_pc(space->cpu),data);
			data %= banks;
		}
		memcpy(ROM + offset * 0x20000, ROM + 0x100000 + data * 0x20000, 0x20000);
	}
}


/***************************************************************************
                                Guardians
***************************************************************************/

static WRITE16_HANDLER( grdians_lockout_w )
{
	if (ACCESSING_BITS_0_7)
	{
		// initially 0, then either $25 (coin 1) or $2a (coin 2)
		coin_counter_w(space->machine, 0,data & 0x01);	// or 0x04
		coin_counter_w(space->machine, 1,data & 0x02);	// or 0x08
	}
//  popmessage("%04X", data & 0xffff);
}

static ADDRESS_MAP_START( grdians_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM								// ROM
	AM_RANGE(0x200000, 0x20ffff) AM_RAM								// RAM
	AM_RANGE(0x304000, 0x30ffff) AM_RAM								// ? seems tile data
	AM_RANGE(0x600000, 0x600001) AM_READ_PORT("DSW1")				// DSW 1
	AM_RANGE(0x600002, 0x600003) AM_READ_PORT("DSW2")				// DSW 2
	AM_RANGE(0x700000, 0x700001) AM_READ_PORT("P1")					// P1
	AM_RANGE(0x700002, 0x700003) AM_READ_PORT("P2")					// P2
	AM_RANGE(0x700004, 0x700005) AM_READ_PORT("SYSTEM")				// Coins
	AM_RANGE(0x70000c, 0x70000d) AM_READ(watchdog_reset16_r)		// Watchdog
	AM_RANGE(0x800000, 0x800001) AM_WRITE(grdians_lockout_w)
	AM_RANGE(0xb00000, 0xb03fff) AM_DEVREADWRITE("x1snd", seta_sound_word_r,seta_sound_word_w)	// Sound
	AM_RANGE(0xc00000, 0xc3ffff) AM_RAM AM_BASE_SIZE_GENERIC(spriteram)		// Sprites
	AM_RANGE(0xc40000, 0xc4ffff) AM_RAM_WRITE(paletteram16_xRRRRRGGGGGBBBBB_word_w) AM_BASE_GENERIC(paletteram)	// Palette
	AM_RANGE(0xc50000, 0xc5ffff) AM_RAM								// cleared
	AM_RANGE(0xc60000, 0xc6003f) AM_WRITE(seta2_vregs_w) AM_BASE(&seta2_vregs)	// Video Registers
	AM_RANGE(0xe00010, 0xe0001f) AM_WRITE(seta2_sound_bank_w)		// Samples Banks
	AM_RANGE(0xfffc00, 0xffffff) AM_RAM_WRITE(tmp68301_regs_w) AM_BASE(&tmp68301_regs)	// TMP68301 Registers
ADDRESS_MAP_END

/***************************************************************************
                        Mobile Suit Gundam EX Revue
***************************************************************************/

static READ16_DEVICE_HANDLER( gundamex_eeprom_r )
{
	return ((eeprom_read_bit(device) & 1)) << 3;
}

static WRITE16_DEVICE_HANDLER( gundamex_eeprom_w )
{
		eeprom_set_clock_line(device, (data & 0x2) ? ASSERT_LINE : CLEAR_LINE);
		eeprom_write_bit(device, data & 0x1);
		eeprom_set_cs_line(device, (data & 0x4) ? CLEAR_LINE : ASSERT_LINE);
}

static ADDRESS_MAP_START( gundamex_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM								// ROM
	AM_RANGE(0x200000, 0x20ffff) AM_RAM								// RAM
	AM_RANGE(0x500000, 0x57ffff) AM_ROM								// ROM
	AM_RANGE(0x600000, 0x600001) AM_READ_PORT("DSW1")				// DSW 1
	AM_RANGE(0x600002, 0x600003) AM_READ_PORT("DSW2")				// DSW 2
	AM_RANGE(0x700000, 0x700001) AM_READ_PORT("P1")					// P1
	AM_RANGE(0x700002, 0x700003) AM_READ_PORT("P2")					// P2
	AM_RANGE(0x700004, 0x700005) AM_READ_PORT("SYSTEM")				// Coins
	AM_RANGE(0x700008, 0x700009) AM_READ_PORT("IN0")				// P1
	AM_RANGE(0x70000a, 0x70000b) AM_READ_PORT("IN1")				// P2
	AM_RANGE(0x70000c, 0x70000d) AM_WRITE(watchdog_reset16_w)
	AM_RANGE(0x800000, 0x800001) AM_WRITE(grdians_lockout_w)
	AM_RANGE(0xb00000, 0xb03fff) AM_DEVREADWRITE("x1snd", seta_sound_word_r,seta_sound_word_w)	// Sound
	AM_RANGE(0xc00000, 0xc3ffff) AM_RAM AM_BASE_SIZE_GENERIC(spriteram)	// Sprites
	AM_RANGE(0xc40000, 0xc4ffff) AM_RAM_WRITE(paletteram16_xRRRRRGGGGGBBBBB_word_w) AM_BASE_GENERIC(paletteram)	// Palette
	AM_RANGE(0xc50000, 0xc5ffff) AM_RAM								// cleared
	AM_RANGE(0xc60000, 0xc6003f) AM_WRITE(seta2_vregs_w) AM_BASE(&seta2_vregs)	// Video Registers
	AM_RANGE(0xe00010, 0xe0001f) AM_WRITE(seta2_sound_bank_w)		// Samples Banks
	AM_RANGE(0xfffd0a, 0xfffd0b) AM_DEVREADWRITE("eeprom", gundamex_eeprom_r,gundamex_eeprom_w)	// parallel data register
	AM_RANGE(0xfffc00, 0xffffff) AM_RAM_WRITE(tmp68301_regs_w) AM_BASE(&tmp68301_regs)	// TMP68301 Registers
ADDRESS_MAP_END


/***************************************************************************
                      Wakakusamonogatari Mahjong Yonshimai
***************************************************************************/

static int keyboard_row;

static READ16_HANDLER( mj4simai_p1_r )
{
	switch (keyboard_row)
	{
		case 0x01: return input_port_read(space->machine, "P1_KEY0");
		case 0x02: return input_port_read(space->machine, "P1_KEY1");
		case 0x04: return input_port_read(space->machine, "P1_KEY2");
		case 0x08: return input_port_read(space->machine, "P1_KEY3");
		case 0x10: return input_port_read(space->machine, "P1_KEY4");
		default:   logerror("p1_r with keyboard_row = %02x\n",keyboard_row); return 0xffff;
	}
}

static READ16_HANDLER( mj4simai_p2_r )
{
	switch (keyboard_row)
	{
		case 0x01: return input_port_read(space->machine, "P2_KEY0");
		case 0x02: return input_port_read(space->machine, "P2_KEY1");
		case 0x04: return input_port_read(space->machine, "P2_KEY2");
		case 0x08: return input_port_read(space->machine, "P2_KEY3");
		case 0x10: return input_port_read(space->machine, "P2_KEY4");
		default:   logerror("p2_r with keyboard_row = %02x\n",keyboard_row); return 0xffff;
	}
}

static WRITE16_HANDLER( mj4simai_keyboard_w )
{
	if (ACCESSING_BITS_0_7)
		keyboard_row = data & 0xff;
}

static ADDRESS_MAP_START( mj4simai_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM								// ROM
	AM_RANGE(0x200000, 0x20ffff) AM_RAM								// RAM
	AM_RANGE(0x600000, 0x600001) AM_READ(mj4simai_p1_r)				// P1
	AM_RANGE(0x600002, 0x600003) AM_READ(mj4simai_p2_r)				// P2
	AM_RANGE(0x600004, 0x600005) AM_WRITE(mj4simai_keyboard_w)		// select keyboard row to read
	AM_RANGE(0x600006, 0x600007) AM_READ(watchdog_reset16_r)		// Watchdog
	AM_RANGE(0x600100, 0x600101) AM_READ_PORT("SYSTEM")				//
	AM_RANGE(0x600200, 0x600201) AM_WRITENOP						// Leds? Coins?
	AM_RANGE(0x600300, 0x600301) AM_READ_PORT("DSW1")				// DSW 1
	AM_RANGE(0x600302, 0x600303) AM_READ_PORT("DSW2")				// DSW 2
	AM_RANGE(0x600300, 0x60030f) AM_WRITE(seta2_sound_bank_w)		// Samples Banks
	AM_RANGE(0xb00000, 0xb03fff) AM_DEVREADWRITE("x1snd", seta_sound_word_r,seta_sound_word_w)	// Sound
	AM_RANGE(0xc00000, 0xc3ffff) AM_RAM AM_BASE_SIZE_GENERIC(spriteram)	// Sprites
	AM_RANGE(0xc40000, 0xc4ffff) AM_RAM_WRITE(paletteram16_xRRRRRGGGGGBBBBB_word_w) AM_BASE_GENERIC(paletteram)	// Palette
	AM_RANGE(0xc60000, 0xc6003f) AM_WRITE(seta2_vregs_w) AM_BASE(&seta2_vregs)	// Video Registers
	AM_RANGE(0xfffc00, 0xffffff) AM_RAM_WRITE(tmp68301_regs_w) AM_BASE(&tmp68301_regs)	// TMP68301 Registers
ADDRESS_MAP_END


/***************************************************************************
                            Kosodate Quiz My Angel
***************************************************************************/

static ADDRESS_MAP_START( myangel_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM								// ROM
	AM_RANGE(0x200000, 0x20ffff) AM_RAM								// RAM
	AM_RANGE(0x700000, 0x700001) AM_READ_PORT("P1")					// P1
	AM_RANGE(0x700002, 0x700003) AM_READ_PORT("P2")					// P2
	AM_RANGE(0x700004, 0x700005) AM_READ_PORT("SYSTEM")				// Coins
	AM_RANGE(0x700006, 0x700007) AM_READ(watchdog_reset16_r)		// Watchdog
	AM_RANGE(0x700200, 0x700201) AM_WRITENOP						// Leds? Coins?
	AM_RANGE(0x700300, 0x700301) AM_READ_PORT("DSW1")				// DSW 1
	AM_RANGE(0x700302, 0x700303) AM_READ_PORT("DSW2")				// DSW 2
	AM_RANGE(0x700310, 0x70031f) AM_WRITE(seta2_sound_bank_w)		// Samples Banks
	AM_RANGE(0xb00000, 0xb03fff) AM_DEVREADWRITE("x1snd", seta_sound_word_r,seta_sound_word_w)	// Sound
	AM_RANGE(0xc00000, 0xc3ffff) AM_RAM AM_BASE_SIZE_GENERIC(spriteram)		// Sprites
	AM_RANGE(0xc40000, 0xc4ffff) AM_RAM_WRITE(paletteram16_xRRRRRGGGGGBBBBB_word_w) AM_BASE_GENERIC(paletteram)	// Palette
	AM_RANGE(0xc60000, 0xc6003f) AM_WRITE(seta2_vregs_w) AM_BASE(&seta2_vregs)				// Video Registers
	AM_RANGE(0xfffc00, 0xffffff) AM_RAM_WRITE(tmp68301_regs_w) AM_BASE(&tmp68301_regs)		// TMP68301 Registers
ADDRESS_MAP_END


/***************************************************************************
                            Kosodate Quiz My Angel 2
***************************************************************************/

static ADDRESS_MAP_START( myangel2_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM								// ROM
	AM_RANGE(0x200000, 0x20ffff) AM_RAM								// RAM
	AM_RANGE(0x600000, 0x600001) AM_READ_PORT("P1")					// P1
	AM_RANGE(0x600002, 0x600003) AM_READ_PORT("P2")					// P2
	AM_RANGE(0x600004, 0x600005) AM_READ_PORT("SYSTEM")				// Coins
	AM_RANGE(0x600006, 0x600007) AM_READ(watchdog_reset16_r)		// Watchdog
	AM_RANGE(0x600200, 0x600201) AM_WRITENOP						// Leds? Coins?
	AM_RANGE(0x600300, 0x600301) AM_READ_PORT("DSW1")				// DSW 1
	AM_RANGE(0x600302, 0x600303) AM_READ_PORT("DSW2")				// DSW 2
	AM_RANGE(0x600300, 0x60030f) AM_WRITE(seta2_sound_bank_w)		// Samples Banks
	AM_RANGE(0xb00000, 0xb03fff) AM_DEVREADWRITE("x1snd", seta_sound_word_r,seta_sound_word_w)	// Sound
	AM_RANGE(0xd00000, 0xd3ffff) AM_RAM AM_BASE_SIZE_GENERIC(spriteram)		// Sprites
	AM_RANGE(0xd40000, 0xd4ffff) AM_RAM_WRITE(paletteram16_xRRRRRGGGGGBBBBB_word_w) AM_BASE_GENERIC(paletteram)	// Palette
	AM_RANGE(0xd60000, 0xd6003f) AM_WRITE(seta2_vregs_w) AM_BASE(&seta2_vregs	)			// Video Registers
	AM_RANGE(0xfffc00, 0xffffff) AM_RAM_WRITE(tmp68301_regs_w) AM_BASE(&tmp68301_regs)		// TMP68301 Registers
ADDRESS_MAP_END


/***************************************************************************
                                Puzzle De Bowling
***************************************************************************/

/*  The game checks for a specific value read from the ROM region.
    The offset to use is stored in RAM at address 0x20BA16 */
static READ16_HANDLER( pzlbowl_protection_r )
{
	UINT32 address = (memory_read_word(space, 0x20ba16) << 16) | memory_read_word(space, 0x20ba18);
	return memory_region(space->machine, "maincpu")[address - 2];
}

static READ16_HANDLER( pzlbowl_coins_r )
{
	return input_port_read(space->machine, "SYSTEM") | (mame_rand(space->machine) & 0x80 );
}

static WRITE16_HANDLER( pzlbowl_coin_counter_w )
{
	if (ACCESSING_BITS_0_7)
	{
		coin_counter_w(space->machine, 0,data & 0x10);
		coin_counter_w(space->machine, 1,data & 0x20);
	}
}

static ADDRESS_MAP_START( pzlbowl_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM									// ROM
	AM_RANGE(0x200000, 0x20ffff) AM_RAM									// RAM
	AM_RANGE(0x400300, 0x400301) AM_READ_PORT("DSW1")					// DSW 1
	AM_RANGE(0x400302, 0x400303) AM_READ_PORT("DSW2")					// DSW 2
	AM_RANGE(0x400300, 0x40030f) AM_WRITE(seta2_sound_bank_w)			// Samples Banks
	AM_RANGE(0x500000, 0x500001) AM_READ_PORT("P1")						// P1
	AM_RANGE(0x500002, 0x500003) AM_READ_PORT("P2")						// P2
	AM_RANGE(0x500004, 0x500005) AM_READWRITE(pzlbowl_coins_r,pzlbowl_coin_counter_w)	// Coins + Protection?
	AM_RANGE(0x500006, 0x500007) AM_READ(watchdog_reset16_r)			// Watchdog
	AM_RANGE(0x700000, 0x700001) AM_READ(pzlbowl_protection_r)			// Protection
	AM_RANGE(0x800000, 0x83ffff) AM_RAM AM_BASE_SIZE_GENERIC(spriteram)		// Sprites
	AM_RANGE(0x840000, 0x84ffff) AM_RAM_WRITE(paletteram16_xRRRRRGGGGGBBBBB_word_w) AM_BASE_GENERIC(paletteram)	// Palette
	AM_RANGE(0x860000, 0x86003f) AM_WRITE(seta2_vregs_w) AM_BASE(&seta2_vregs)				// Video Registers
	AM_RANGE(0x900000, 0x903fff) AM_DEVREADWRITE("x1snd", seta_sound_word_r,seta_sound_word_w)	// Sound
	AM_RANGE(0xfffc00, 0xffffff) AM_RAM_WRITE(tmp68301_regs_w) AM_BASE(&tmp68301_regs)		// TMP68301 Registers
ADDRESS_MAP_END


/***************************************************************************
                            Penguin Bros
***************************************************************************/

static ADDRESS_MAP_START( penbros_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM								// ROM
	AM_RANGE(0x200000, 0x20ffff) AM_RAM								// RAM
	AM_RANGE(0x210000, 0x23ffff) AM_RAM								// RAM
	AM_RANGE(0x300000, 0x30ffff) AM_RAM								// RAM
	AM_RANGE(0x500300, 0x500301) AM_READ_PORT("DSW1")				// DSW 1
	AM_RANGE(0x500302, 0x500303) AM_READ_PORT("DSW2")				// DSW 2
	AM_RANGE(0x500300, 0x50030f) AM_WRITE(seta2_sound_bank_w)		// Samples Banks
	AM_RANGE(0x600000, 0x600001) AM_READ_PORT("P1")					// P1
	AM_RANGE(0x600002, 0x600003) AM_READ_PORT("P2")					// P2
	AM_RANGE(0x600004, 0x600005) AM_READ_PORT("SYSTEM")				// Coins
	AM_RANGE(0x600004, 0x600005) AM_WRITE(pzlbowl_coin_counter_w)	// Coins Counter
	AM_RANGE(0x600006, 0x600007) AM_READ(watchdog_reset16_r)		// Watchdog
//  AM_RANGE(0x700000, 0x700001) AM_READ(pzlbowl_protection_r)      // Protection
	AM_RANGE(0xb00000, 0xb3ffff) AM_RAM AM_BASE_SIZE_GENERIC(spriteram)		// Sprites
	AM_RANGE(0xb40000, 0xb4ffff) AM_RAM_WRITE(paletteram16_xRRRRRGGGGGBBBBB_word_w) AM_BASE_GENERIC(paletteram)	// Palette
	AM_RANGE(0xb60000, 0xb6003f) AM_WRITE(seta2_vregs_w) AM_BASE(&seta2_vregs	)
	AM_RANGE(0xa00000, 0xa03fff) AM_DEVREADWRITE("x1snd", seta_sound_word_r,seta_sound_word_w)	// Sound
	AM_RANGE(0xfffc00, 0xffffff) AM_RAM_WRITE(tmp68301_regs_w) AM_BASE(&tmp68301_regs)		// TMP68301 Registers
ADDRESS_MAP_END


/***************************************************************************
                            Sammy Outdoor Shooting
***************************************************************************/

static WRITE16_HANDLER( samshoot_coin_w )
{
	if (ACCESSING_BITS_0_7)
	{
		coin_counter_w(space->machine, 0, data & 0x10);
		coin_counter_w(space->machine, 1, data & 0x20);
		// Are these connected? They are set in I/O test
		coin_lockout_w(space->machine, 0,~data & 0x40);
		coin_lockout_w(space->machine, 1,~data & 0x80);
	}
//  popmessage("%04x",data);
}

static ADDRESS_MAP_START( samshoot_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE( 0x000000, 0x1fffff ) AM_ROM
	AM_RANGE( 0x200000, 0x20ffff ) AM_RAM
	AM_RANGE( 0x300000, 0x30ffff ) AM_RAM AM_BASE_SIZE_GENERIC(nvram)

	AM_RANGE( 0x400000, 0x400001 ) AM_READ_PORT("DSW1")				// DSW 1
	AM_RANGE( 0x400002, 0x400003 ) AM_READ_PORT("BUTTONS")			// Buttons

	AM_RANGE( 0x400300, 0x40030f ) AM_WRITE( seta2_sound_bank_w )	// Samples Banks

	AM_RANGE( 0x500000, 0x500001 ) AM_READ_PORT("GUN1")				// P1
	AM_RANGE( 0x580000, 0x580001 ) AM_READ_PORT("GUN2")				// P2

	AM_RANGE( 0x700000, 0x700001 ) AM_READ_PORT("TRIGGER")			// Trigger
	AM_RANGE( 0x700002, 0x700003 ) AM_READ_PORT("PUMP")				// Pump
	AM_RANGE( 0x700004, 0x700005 ) AM_READ_PORT("COIN")	AM_WRITE( samshoot_coin_w )	// Coins
	AM_RANGE( 0x700006, 0x700007 ) AM_READ( watchdog_reset16_r )	// Watchdog?

	AM_RANGE( 0x800000, 0x83ffff ) AM_RAM AM_BASE_SIZE_GENERIC(spriteram)	// Sprites
	AM_RANGE( 0x840000, 0x84ffff ) AM_RAM_WRITE(paletteram16_xRRRRRGGGGGBBBBB_word_w) AM_BASE_GENERIC(paletteram)	// Palette
	AM_RANGE( 0x860000, 0x86003f ) AM_WRITE(seta2_vregs_w) AM_BASE(&seta2_vregs)	// Video Registers

	AM_RANGE( 0x900000, 0x903fff ) AM_DEVREADWRITE( "x1snd", seta_sound_word_r, seta_sound_word_w	)	// Sound

	AM_RANGE( 0xfffd0a, 0xfffd0b ) AM_READ_PORT("DSW2")				// parallel data register (DSW 2)
	AM_RANGE( 0xfffc00, 0xffffff ) AM_RAM_WRITE( tmp68301_regs_w) AM_BASE(&tmp68301_regs )	// TMP68301 Registers
ADDRESS_MAP_END


/***************************************************************************
                                  Funcube
***************************************************************************/

static UINT8 *funcube_outputs;
static UINT8 *funcube_leds;

static UINT64 funcube_coin_start_cycles;
static UINT8 funcube_hopper_motor;
static UINT8 funcube_press;

static UINT8 funcube_serial_fifo[4];
static UINT8 funcube_serial_count;

// Bus conversion functions:

// RAM shared with the sub CPU
static READ32_HANDLER( funcube_nvram_dword_r )
{
	UINT16 val = space->machine->generic.nvram.u16[offset];
	return ((val & 0xff00) << 8) | (val & 0x00ff);
}

static WRITE32_HANDLER( funcube_nvram_dword_w )
{
	if (ACCESSING_BITS_0_7)
	{
		space->machine->generic.nvram.u16[offset] = (space->machine->generic.nvram.u16[offset] & 0xff00) | (data & 0x000000ff);
	}
	if (ACCESSING_BITS_16_23)
	{
		space->machine->generic.nvram.u16[offset] = (space->machine->generic.nvram.u16[offset] & 0x00ff) | ((data & 0x00ff0000) >> 8);
	}
}

static WRITE16_HANDLER( spriteram16_word_w )
{
	COMBINE_DATA( &space->machine->generic.spriteram.u16[offset] );
}

static READ16_HANDLER( spriteram16_word_r )
{
	return space->machine->generic.spriteram.u16[offset];
}

static READ16_HANDLER( paletteram16_word_r )
{
	return space->machine->generic.paletteram.u16[offset];
}

static READ16BETO32BE( spriteram32_dword, spriteram16_word_r );
static WRITE16BETO32BE( spriteram32_dword, spriteram16_word_w );

static READ16BETO32BE( paletteram32_dword, paletteram16_word_r );
static WRITE16BETO32BE( paletteram32_dword, paletteram16_xRRRRRGGGGGBBBBB_word_w );

static WRITE16BETO32BE( seta2_vregs_dword, seta2_vregs_w );

// Main CPU

// ColdFire peripherals

enum {
	CF_PPDAT	=	0x1c8/4,
	CF_MBSR		=	0x1ec/4
};

static UINT32 *coldfire_regs;

static WRITE32_HANDLER( coldfire_regs_w )
{
	COMBINE_DATA( &coldfire_regs[offset] );
}

static READ32_HANDLER( coldfire_regs_r )
{
	switch( offset )
	{
		case CF_MBSR:
			return mame_rand(space->machine);

		case CF_PPDAT:
			return input_port_read(space->machine, "BATTERY") << 16;
	}

	return coldfire_regs[offset];
}

static READ32_HANDLER( funcube_debug_r )
{
	UINT32 ret = input_port_read(space->machine,"DEBUG");

	// This bits let you move the crosshair in the inputs / touch panel test with a joystick
	if (!(space->machine->primary_screen->frame_number() % 3))
		ret |= 0x3f;

	return ret;
}


static ADDRESS_MAP_START( funcube_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE( 0x00000000, 0x0007ffff ) AM_ROM
	AM_RANGE( 0x00200000, 0x0020ffff ) AM_RAM

	AM_RANGE( 0x00500000, 0x00500003 ) AM_READ( funcube_debug_r )
	AM_RANGE( 0x00500004, 0x00500007 ) AM_READ( watchdog_reset32_r ) AM_WRITENOP

	AM_RANGE( 0x00600000, 0x00600003 ) AM_WRITENOP	// sound chip

	AM_RANGE( 0x00800000, 0x0083ffff ) AM_READWRITE( spriteram32_dword_r,  spriteram32_dword_w  ) AM_BASE_GENERIC(spriteram) AM_SIZE_GENERIC(spriteram)
	AM_RANGE( 0x00840000, 0x0084ffff ) AM_READWRITE( paletteram32_dword_r, paletteram32_dword_w ) AM_BASE_GENERIC(paletteram)
	AM_RANGE( 0x00860000, 0x0086003f ) AM_WRITE( seta2_vregs_dword_w )                            AM_BASE((UINT32**)&seta2_vregs)

	AM_RANGE( 0x00c00000, 0x00c002ff ) AM_READWRITE( funcube_nvram_dword_r, funcube_nvram_dword_w )

	AM_RANGE(0xf0000000, 0xf00001ff ) AM_READWRITE( coldfire_regs_r, coldfire_regs_w ) AM_BASE(&coldfire_regs)	// Module
	AM_RANGE(0xffffe000, 0xffffffff ) AM_RAM	// SRAM
ADDRESS_MAP_END

// Sub CPU

static ADDRESS_MAP_START( funcube_sub_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE( 0x000000, 0x01ffff ) AM_ROM
	AM_RANGE( 0x200000, 0x20017f ) AM_RAM AM_BASE_SIZE_GENERIC(nvram)
ADDRESS_MAP_END




// Simulate coin drop through two sensors

#define FUNCUBE_SUB_CPU_CLOCK (XTAL_14_7456MHz)

static READ8_HANDLER( funcube_coins_r )
{
	UINT8 ret = input_port_read(space->machine,"SWITCH");
	UINT8 coin_bit0 = 1;	// active low
	UINT8 coin_bit1 = 1;

	UINT8 hopper_bit = (funcube_hopper_motor && !(space->machine->primary_screen->frame_number()%20)) ? 1 : 0;

	const UINT64 coin_total_cycles = FUNCUBE_SUB_CPU_CLOCK / (1000/20);

	if ( funcube_coin_start_cycles )
	{
		UINT64 elapsed = downcast<cpu_device *>(space->cpu)->total_cycles() - funcube_coin_start_cycles;

		if ( elapsed < coin_total_cycles/2 )
			coin_bit0 = 0;
		else if ( elapsed < coin_total_cycles )
			coin_bit1 = 0;
		else
			funcube_coin_start_cycles = 0;
	}
	else
	{
		if (!(ret & 1))
			funcube_coin_start_cycles = downcast<cpu_device *>(space->cpu)->total_cycles();
	}

	return (ret & ~7) | (hopper_bit << 2) | (coin_bit1 << 1) | coin_bit0;
}

static READ8_HANDLER( funcube_serial_r )
{
	UINT8 ret = 0xff;

	switch( funcube_serial_count )
	{
		case 4:	ret = funcube_serial_fifo[0];	break;
		case 3:	ret = funcube_serial_fifo[1];	break;
		case 2:	ret = funcube_serial_fifo[2];	break;
		case 1:	ret = funcube_serial_fifo[3];	break;
	}

	if (funcube_serial_count)
		funcube_serial_count--;

	return ret;
}

static void funcube_debug_outputs(void)
{
#ifdef MAME_DEBUG
//  popmessage("LED: %02x OUT: %02x", (int)*funcube_leds, (int)*funcube_outputs);
#endif
}

static WRITE8_HANDLER( funcube_leds_w )
{
	*funcube_leds = data;

	set_led_status( space->machine, 0, (~data) & 0x01 );	// win lamp (red)
	set_led_status( space->machine, 1, (~data) & 0x02 );	// win lamp (green)

	// Set in a moving pattern: 0111 -> 1011 -> 1101 -> 1110
	set_led_status( space->machine, 2, (~data) & 0x10 );
	set_led_status( space->machine, 3, (~data) & 0x20 );
	set_led_status( space->machine, 4, (~data) & 0x40 );
	set_led_status( space->machine, 5, (~data) & 0x80 );

	funcube_debug_outputs();
}

static READ8_HANDLER( funcube_outputs_r )
{
	// Bits 1,2,3 read
	return *funcube_outputs;
}

static WRITE8_HANDLER( funcube_outputs_w )
{
	*funcube_outputs = data;

	// Bits 0,1,3 written

	// Bit 0: hopper motor
	funcube_hopper_motor = (~data) & 0x01;

	// Bit 1: high on pay out

	// Bit 3: low after coining up, blinks on pay out
	set_led_status( space->machine, 6, (~data) & 0x08 );

	funcube_debug_outputs();
}


static ADDRESS_MAP_START( funcube_sub_io, ADDRESS_SPACE_IO, 8 )
	AM_RANGE( H8_PORT_7,   H8_PORT_7   )	AM_READ( funcube_coins_r )
	AM_RANGE( H8_PORT_4,   H8_PORT_4   )	AM_NOP	// unused
	AM_RANGE( H8_PORT_A,   H8_PORT_A   )	AM_READWRITE( funcube_outputs_r, funcube_outputs_w ) AM_BASE( &funcube_outputs )
	AM_RANGE( H8_PORT_B,   H8_PORT_B   )	AM_WRITE( funcube_leds_w )                           AM_BASE( &funcube_leds )
//  AM_RANGE( H8_SERIAL_0, H8_SERIAL_0 )    // cabinets linking (jpunit)
	AM_RANGE( H8_SERIAL_1, H8_SERIAL_1 )	AM_READ( funcube_serial_r )
ADDRESS_MAP_END




/***************************************************************************

                                Input Ports

***************************************************************************/

/***************************************************************************
                        Mobile Suit Gundam EX Revue
***************************************************************************/

static INPUT_PORTS_START( gundamex )
	PORT_START("DSW1")	// $600000.w
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( On ) )
	PORT_DIPNAME( 0x0006, 0x0006, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:2,3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPUNUSED_DIPLOC( 0x0008, 0x0008, "SW1:4" ) /* Listed as "Unused" */
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Freeze" ) PORT_DIPLOCATION("SW1:6")	/* Listed as "Unused" */
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Show Targets" ) PORT_DIPLOCATION("SW1:7") /* Listed as "Unused" */
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:8" )
	PORT_BIT(             0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW2")	// $600002.w
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0000, "3 Coins/5 Credits" )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Debug Mode" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT(             0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1")	// $700000.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")	// $700002.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")	// $700004.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN ) //jumper pad
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Language ) ) 		 //jumper pad
	PORT_DIPSETTING(      0x0020, DEF_STR( English ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Japanese ) )
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0")	// $700008.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")	// $70000a.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

/***************************************************************************
                                Guardians
***************************************************************************/

static INPUT_PORTS_START( grdians )
	PORT_START("DSW1")	// $600000.w
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy )    )	// 0
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal )  )	// 1
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard )    )	// 2
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )	// 3
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Title" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0008, "Guardians" )
	PORT_DIPSETTING(      0x0000, "Denjin Makai II" )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0020, "1" )
	PORT_DIPSETTING(      0x0030, "2" )
	PORT_DIPSETTING(      0x0010, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_SERVICE_DIPLOC(  0x0040, IP_ACTIVE_LOW, "SW1:7" ) /* NOTE: Test mode shows player 3 & 4 controls, but it's a two player game */
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )
	PORT_BIT(             0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW2")	// $600002.w
	PORT_DIPNAME( 0x000f, 0x000f, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW2:1,2,3,4")
	PORT_DIPSETTING(      0x0002, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x000f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x000e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(      0x000d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x000b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x000a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0009, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x00f0, 0x00f0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW2:5,6,7,8")
	PORT_DIPSETTING(      0x0020, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0050, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x00f0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x0070, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x00e0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(      0x00d0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x00b0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x00a0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0090, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_BIT(             0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1")	// $700000.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")	// $700002.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")	// $700004.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F1)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************
                      Wakakusamonogatari Mahjong Yonshimai
***************************************************************************/

static INPUT_PORTS_START( mj4simai )
	PORT_START("DSW1")	// $600300.w
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Tumo Pin" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:8" )
	PORT_BIT(             0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW2")	// $600302.w
	PORT_DIPNAME( 0x0007, 0x0004, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(      0x0004, "0" )
	PORT_DIPSETTING(      0x0003, "1" )
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0001, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPSETTING(      0x0007, "5" )
	PORT_DIPSETTING(      0x0006, "6" )
	PORT_DIPSETTING(      0x0005, "7" )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0010, 0x0000, "Select Girl" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0020, 0x0000, "Com Put" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT(             0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")	// $600100.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1_KEY0")	// $600000(0)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START1  )
	PORT_BIT( 0xffc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1_KEY1")	// $600000(1)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0xffc0, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("P1_KEY2")	// $600000(2)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0xffe0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1_KEY3")	// $600000(3)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1_KEY4")	// $600000(4)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )
	PORT_BIT( 0x00c0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2_KEY0")	// $600000(0)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2  )
	PORT_BIT( 0xffc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2_KEY1")	// $600000(1)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_BET ) PORT_PLAYER(2)
	PORT_BIT( 0xffc0, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("P2_KEY2")	// $600000(2)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(2)
	PORT_BIT( 0xffe0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2_KEY3")	// $600000(3)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(2)
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2_KEY4")	// $600000(4)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_BIG ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL ) PORT_PLAYER(2)
	PORT_BIT( 0x00c0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************
                            Kosodate Quiz My Angel
***************************************************************************/

static INPUT_PORTS_START( myangel )
	PORT_START("DSW1")	// $700300.w
	PORT_SERVICE_DIPLOC(  0x0001, IP_ACTIVE_LOW, "SW1:1" )
	PORT_DIPUNUSED_DIPLOC( 0x0002, 0x0002, "SW1:2" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0004, 0x0004, "SW1:3" ) /* Listed as "Unused" */
	PORT_DIPNAME( 0x0008, 0x0008, "Increase Lives While Playing" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0020, "2" )
	PORT_DIPSETTING(      0x0030, "3" )
	PORT_DIPSETTING(      0x0010, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT(             0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW2")	// $700302.w
	PORT_DIPNAME( 0x000f, 0x000f, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW2:1,2,3,4")
	PORT_DIPSETTING(      0x0002, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x000f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x000e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(      0x000d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x000b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x000a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0009, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0010, "SW2:5" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0020, "SW2:6" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "SW2:7" ) /* Listed as "Unused" */
	PORT_DIPNAME( 0x0080, 0x0080, "Push Start To Freeze (Cheat)") PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
	PORT_BIT(             0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1") //$700000.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_START1  )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2") //$700002.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_START2  )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM") //$700004.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0008, IP_ACTIVE_LOW )
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************
                            Kosodate Quiz My Angel 2
***************************************************************************/

static INPUT_PORTS_START( myangel2 )
	PORT_START("DSW1") //$600300.w
	PORT_SERVICE_DIPLOC(  0x0001, IP_ACTIVE_LOW, "SW1:1" )
	PORT_DIPUNUSED_DIPLOC( 0x0002, 0x0002, "SW1:2" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0004, 0x0004, "SW1:3" ) /* Listed as "Unused" */
	PORT_DIPNAME( 0x0008, 0x0008, "Increase Lives While Playing" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0020, "2" )
	PORT_DIPSETTING(      0x0030, "3" )
	PORT_DIPSETTING(      0x0010, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT(     0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW2") //$600302.w
	PORT_DIPNAME( 0x000f, 0x000f, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW2:1,2,3,4")
	PORT_DIPSETTING(      0x0002, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x000f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x000e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(      0x000d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x000b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x000a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0009, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0010, "SW2:5" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0020, "SW2:6" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "SW2:7" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0080, "SW2:8" ) /* Listed as "Unused" */
	PORT_BIT(             0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1") //$600000.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_START1  )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2") //$600002.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_START2  )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM") //$600004.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0008, IP_ACTIVE_LOW )
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************
                                Puzzle De Bowling
***************************************************************************/

static INPUT_PORTS_START( pzlbowl )
	PORT_START("DSW1") //$400300.w
	PORT_SERVICE_DIPLOC(  0x0001, IP_ACTIVE_LOW, "SW1:1" )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(      0x0030, DEF_STR( Easiest ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( Easier ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Harder ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Very_Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x00c0, 0x00c0, "Winning Rounds (Player VS Player)" ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0x0040, "1" )
	PORT_DIPSETTING(      0x00c0, "2" )		/* This setting is not defined in the manual */
	PORT_DIPSETTING(      0x0080, "3" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_BIT(             0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW2") //$400302.w
	PORT_DIPNAME( 0x000f, 0x000f, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW2:1,2,3,4")
	PORT_DIPSETTING(      0x0005, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 3C_2C ) )
//  PORT_DIPSETTING(      0x0002, DEF_STR( 1C_1C ) )        /* This setting is not defined in the manual */
	PORT_DIPSETTING(      0x000f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x000e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(      0x000d, DEF_STR( 1C_3C ) )
//  PORT_DIPSETTING(      0x0001, DEF_STR( 1C_3C ) )        /* This setting is not defined in the manual */
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x000b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x000a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0009, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Join In" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Yes ) )
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "SW2:7" )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Language ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( English ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Japanese ) )
	PORT_BIT(             0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1") //$500000.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2") //$500002.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM") //$500004.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)	// unused, test mode shows it
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F1)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0080, IP_ACTIVE_HIGH, IPT_SPECIAL )	// Protection?
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************
                            Penguin Bros
***************************************************************************/

static INPUT_PORTS_START( penbros )
	PORT_START("DSW1") //$500300.w
	PORT_SERVICE_DIPLOC(  0x0001, IP_ACTIVE_LOW, "SW1:1" )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0004, 0x0004, "SW1:3" )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0020, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0x0080, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_2C ) )
	PORT_BIT(             0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW2") //$500302.w
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x000c, "3" )
	PORT_DIPSETTING(      0x0004, "4" )
	PORT_DIPSETTING(      0x0008, "5" )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x0010, "150k and 500k" )
	PORT_DIPSETTING(      0x0030, "200k and 700k" )
	PORT_DIPSETTING(      0x0000, "Every 250k" )	// no extra life after the one at 1500k
	PORT_DIPSETTING(      0x0020, DEF_STR( None ) )
	PORT_DIPNAME( 0x00c0, 0x00c0, "Winning Rounds (Player VS Player)" ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(      0x00c0, "2" )
	PORT_DIPSETTING(      0x0040, "3" )
	PORT_DIPSETTING(      0x0080, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_BIT(             0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1") //$600000.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* Player 1 button 3 is unused */
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2") //$600002.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* Player 2 button 3 is unused */
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM") //$600004.w
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)	// unused, test mode shows it
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F1)
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************
                            Sammy Outdoor Shooting
***************************************************************************/

static INPUT_PORTS_START( deerhunt )
	PORT_START("DSW1") // $400000.w
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0005, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(      0x0028, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Discount To Continue" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:8" )
	PORT_BIT(     0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW2") // fffd0a.w
	PORT_DIPNAME( 0x0001, 0x0001, "Vert. Flip Screen" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "Horiz. Flip Screen" ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0018, 0x0018, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Blood Color" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0020, "Red" )
	PORT_DIPSETTING(      0x0000, "Yellow" )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0040, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x0080, 0x0080, "Gun Type" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, "Pump Action" )
	PORT_DIPSETTING(      0x0000, "Hand Gun" )
	PORT_BIT(     0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("GUN1") // $500000
	PORT_BIT( 0x00ff, 0x0080, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1, 0, 0) PORT_MINMAX(0x0025,0x00c5) PORT_SENSITIVITY(35) PORT_KEYDELTA(10) PORT_PLAYER(1)
	PORT_BIT( 0xff00, 0x8000, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1, 0, 0) PORT_MINMAX(0x0800,0xf800) PORT_SENSITIVITY(35) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("GUN2")	// $580000.b
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )	// P2 gun, read but not used

	PORT_START("TRIGGER")	// $700000
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SPECIAL )	// trigger
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1  )
	PORT_BIT( 0xff3f, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("PUMP")	// $700003.b
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SPECIAL )	// pump
	PORT_BIT( 0xffbf, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("COIN")	// $700005.b
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("BUTTONS")	// $400002
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 )	// trigger
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 )	// pump
	PORT_BIT( 0xfffc, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( turkhunt )
	PORT_INCLUDE(deerhunt)

	PORT_MODIFY("DSW2") // fffd0a.w
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0040, "2" )
	PORT_DIPSETTING(      0x0000, "3" )
INPUT_PORTS_END


static INPUT_PORTS_START( wschamp )
	PORT_INCLUDE(deerhunt)

	PORT_MODIFY("DSW1")	// $400000.w
	PORT_DIPNAME( 0x000f, 0x000f, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(      0x0009, "4 Coins Start, 4 Coins Continue" )
	PORT_DIPSETTING(      0x0008, "4 Coins Start, 3 Coins Continue" )
	PORT_DIPSETTING(      0x0007, "4 Coins Start, 2 Coins Continue" )
	PORT_DIPSETTING(      0x0006, "4 Coins Start, 1 Coin Continue" )
	PORT_DIPSETTING(      0x000c, "3 Coins Start, 3 Coins Continue" )
	PORT_DIPSETTING(      0x000b, "3 Coins Start, 2 Coins Continue" )
	PORT_DIPSETTING(      0x000a, "3 Coins Start, 1 Coin Continue" )
	PORT_DIPSETTING(      0x000e, "2 Coins Start, 2 Coins Continue" )
	PORT_DIPSETTING(      0x000d, "2 Coins Start, 1 Coin Continue" )
	PORT_DIPSETTING(      0x000f, "1 Coin Start, 1 Coin Continue" )
	PORT_DIPSETTING(      0x0005, "1 Coin 2 Credits, 1 Credit Start & Continue" )
	PORT_DIPSETTING(      0x0004, "1 Coin 3 Credits, 1 Credit Start & Continue" )
	PORT_DIPSETTING(      0x0003, "1 Coin 4 Credits, 1 Credit Start & Continue" )
	PORT_DIPSETTING(      0x0002, "1 Coin 5 Credits, 1 Credit Start & Continue" )
	PORT_DIPSETTING(      0x0001, "1 Coin 6 Credits, 1 Credit Start & Continue" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0010, "SW1:5" )
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0020, "SW1:6" )
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "SW1:7" )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:8" )
	PORT_BIT(     0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("DSW2") // fffd0a.w
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0020, "SW2:6" )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0040, "2" )
	PORT_DIPSETTING(      0x0000, "3" )

	PORT_MODIFY("GUN2") // $580000
	PORT_BIT( 0x00ff, 0x0080, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1, 0, 0) PORT_MINMAX(0x0025,0x00c5) PORT_SENSITIVITY(35) PORT_KEYDELTA(10) PORT_PLAYER(2)
	PORT_BIT( 0xff00, 0x8000, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1, 0, 0) PORT_MINMAX(0x0800,0xf800) PORT_SENSITIVITY(35) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_MODIFY("TRIGGER")	// $700000
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SPECIAL )	// trigger P2
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SPECIAL )	// trigger P1
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1  )
	PORT_BIT( 0xff1f, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("PUMP")	// $700003.b
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SPECIAL )	// pump P2
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SPECIAL )	// pump P1
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2  )
	PORT_BIT( 0xff1f, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("COIN")	// $700005.b
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("BUTTONS")	// $400002
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 )	// trigger P1
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 )	// pump P1
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )	PORT_PLAYER(2)	// trigger P2
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )	PORT_PLAYER(2)	// pump P2
	PORT_BIT( 0xffcc, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( trophyh )
	PORT_INCLUDE(wschamp)

	PORT_MODIFY("DSW2") // fffd0a.w
	PORT_DIPNAME( 0x0020, 0x0020, "Blood Color" ) PORT_DIPLOCATION("SW2:6") /* WSChamp doesn't use Blood Color, so add it back in */
	PORT_DIPSETTING(      0x0020, "Red" )
	PORT_DIPSETTING(      0x0000, "Yellow" )
INPUT_PORTS_END


/***************************************************************************
                                  Funcube
***************************************************************************/

static INPUT_PORTS_START( funcube )
	PORT_START("TOUCH_PRESS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME( "Touch Screen" )

	PORT_START("TOUCH_X")
	PORT_BIT( 0xff, 0x00, IPT_LIGHTGUN_X ) PORT_MINMAX(0,0x5c+1) PORT_CROSSHAIR(X, -(1.0 * 0x05d/0x5c), -1.0/0x5c, 0) PORT_SENSITIVITY(45) PORT_KEYDELTA(5) PORT_REVERSE

	PORT_START("TOUCH_Y")
	PORT_BIT( 0xff, 0x00, IPT_LIGHTGUN_Y ) PORT_MINMAX(0,0x46+1) PORT_CROSSHAIR(Y, -(0xf0-8.0)/0xf0*0x047/0x46, -1.0/0x46, 0) PORT_SENSITIVITY(45) PORT_KEYDELTA(5) PORT_REVERSE

	PORT_START("SWITCH")	// c00030.l
	PORT_BIT(     0x01, IP_ACTIVE_LOW,  IPT_COIN1    ) PORT_IMPULSE(1)	// coin solenoid 1
	PORT_BIT(     0x02, IP_ACTIVE_HIGH, IPT_SPECIAL  )					// coin solenoid 2
	PORT_BIT(     0x04, IP_ACTIVE_HIGH, IPT_SPECIAL  )	// hopper sensor
	PORT_BIT(     0x08, IP_ACTIVE_LOW,  IPT_BUTTON2  )	// game select
	PORT_BIT(     0x10, IP_ACTIVE_LOW,  IPT_SPECIAL  ) PORT_CODE(KEYCODE_O) PORT_NAME( "Pay Out" )
	PORT_BIT(     0x20, IP_ACTIVE_LOW,  IPT_SERVICE1 ) PORT_NAME( "Reset Key" )
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW   )
	PORT_BIT(     0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN  )

	PORT_START("BATTERY")
	PORT_DIPNAME( 0x10, 0x10, "Battery" )
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x10, DEF_STR( On ) )

	PORT_START("DEBUG")
	// 500002.w
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(2)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(2)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(2)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1        ) PORT_PLAYER(2)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2        ) PORT_PLAYER(2)

	// 500000.w
	PORT_DIPNAME(    0x00010000, 0x00010000, "Debug 0" )
	PORT_DIPSETTING( 0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00010000, DEF_STR( On ) )
	PORT_DIPNAME(    0x00020000, 0x00020000, "Debug 1" )
	PORT_DIPSETTING( 0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00020000, DEF_STR( On ) )
	PORT_DIPNAME(    0x00040000, 0x00040000, "Debug 2" )	// Touch-Screen
	PORT_DIPSETTING( 0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00040000, DEF_STR( On ) )
	PORT_DIPNAME(    0x00080000, 0x00080000, "Debug 3" )
	PORT_DIPSETTING( 0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00080000, DEF_STR( On ) )
	PORT_DIPNAME(    0x00100000, 0x00100000, "Debug 4" )
	PORT_DIPSETTING( 0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00100000, DEF_STR( On ) )
	PORT_DIPNAME(    0x00200000, 0x00200000, "Debug 5" )
	PORT_DIPSETTING( 0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00200000, DEF_STR( On ) )
	PORT_DIPNAME(    0x00400000, 0x00400000, "Debug 6" )
	PORT_DIPSETTING( 0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00400000, DEF_STR( On ) )
	PORT_DIPNAME(    0x00800000, 0x00800000, "Debug 7" )
	PORT_DIPSETTING( 0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00800000, DEF_STR( On ) )
INPUT_PORTS_END


/***************************************************************************


                            Graphics Layouts


***************************************************************************/

static const gfx_layout layout_4bpp_lo =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{	RGN_FRAC(1,4)+8,RGN_FRAC(1,4)+0,
		RGN_FRAC(0,4)+8,RGN_FRAC(0,4)+0		},
	{	STEP8(0,1)		},
	{	STEP8(0,8*2)	},
	8*8*2
};

static const gfx_layout layout_4bpp_hi =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{	RGN_FRAC(3,4)+8,RGN_FRAC(3,4)+0,
		RGN_FRAC(2,4)+8,RGN_FRAC(2,4)+0		},
	{	STEP8(0,1)		},
	{	STEP8(0,8*2)	},
	8*8*2
};

static const gfx_layout layout_6bpp =
{
	8,8,
	RGN_FRAC(1,4),
	6,
	{
		RGN_FRAC(2,4)+8,RGN_FRAC(2,4)+0,
		RGN_FRAC(1,4)+8,RGN_FRAC(1,4)+0,
		RGN_FRAC(0,4)+8,RGN_FRAC(0,4)+0		},
	{	STEP8(0,1)		},
	{	STEP8(0,8*2)	},
	8*8*2
};

static const gfx_layout layout_8bpp =
{
	8,8,
	RGN_FRAC(1,4),
	8,
	{	RGN_FRAC(3,4)+8,RGN_FRAC(3,4)+0,
		RGN_FRAC(2,4)+8,RGN_FRAC(2,4)+0,
		RGN_FRAC(1,4)+8,RGN_FRAC(1,4)+0,
		RGN_FRAC(0,4)+8,RGN_FRAC(0,4)+0		},
	{	STEP8(0,1)		},
	{	STEP8(0,8*2)	},
	8*8*2
};

static const gfx_layout layout_3bpp_lo =
{
	8,8,
	RGN_FRAC(1,4),
	3,
	{	                RGN_FRAC(1,4)+0,
		RGN_FRAC(0,4)+8,RGN_FRAC(0,4)+0		},
	{	STEP8(0,1)		},
	{	STEP8(0,8*2)	},
	8*8*2
};

static const gfx_layout layout_2bpp_hi =
{
	8,8,
	RGN_FRAC(1,4),
	2,
	{	RGN_FRAC(2,4)+8,RGN_FRAC(2,4)+0 },
	{	STEP8(0,1)		},
	{	STEP8(0,8*2)	},
	8*8*2
};

/*  Tiles are 8bpp, but the hardware is additionally able to discard
    some bitplanes and use the low 4 bits only, or the high 4 bits only */
static GFXDECODE_START( seta2 )
	GFXDECODE_ENTRY( "gfx1", 0, layout_4bpp_lo, 0, 0x8000/16 )
	GFXDECODE_ENTRY( "gfx1", 0, layout_4bpp_hi, 0, 0x8000/16 )
	GFXDECODE_ENTRY( "gfx1", 0, layout_6bpp,    0, 0x8000/16 )	/* 6bpp, but 4bpp granularity */
	GFXDECODE_ENTRY( "gfx1", 0, layout_8bpp,    0, 0x8000/16 )	/* 8bpp, but 4bpp granularity */
	GFXDECODE_ENTRY( "gfx1", 0, layout_3bpp_lo, 0, 0x8000/16 )	/* 3bpp, but 4bpp granularity */
	GFXDECODE_ENTRY( "gfx1", 0, layout_2bpp_hi, 0, 0x8000/16 )	/* ??? */
GFXDECODE_END

/***************************************************************************
                                  Funcube
***************************************************************************/

static const gfx_layout funcube_layout_4bpp_lo =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ STEP4(7*8, -8) },
	{ STEP8(0, 1) },
	{ STEP8(0, 8*8) },
	8*8*8
};

static const gfx_layout funcube_layout_4bpp_hi =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ STEP4(4*8, -8) },
	{ STEP8(0, 1) },
	{ STEP8(0, 8*8) },
	8*8*8
};

static const gfx_layout funcube_layout_6bpp =
{
	8,8,
	RGN_FRAC(1,1),
	6,
	{ STEP4(7*8, -8), STEP2(3*8, -8) },
	{ STEP8(0, 1) },
	{ STEP8(0, 8*8) },
	8*8*8
};

static const gfx_layout funcube_layout_8bpp =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ STEP8(7*8, -8) },
	{ STEP8(0, 1) },
	{ STEP8(0, 8*8) },
	8*8*8
};

static const gfx_layout funcube_layout_3bpp_lo =
{
	8,8,
	RGN_FRAC(1,1),
	3,
	{ 7*8,6*8,5*8 },
	{ STEP8(0, 1) },
	{ STEP8(0, 8*8) },
	8*8*8
};

static const gfx_layout funcube_layout_2bpp_hi =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ STEP2(5*8, -8) },
	{ STEP8(0, 1) },
	{ STEP8(0, 8*8) },
	8*8*8
};

/*  Tiles are 8bpp, but the hardware is additionally able to discard
    some bitplanes and use the low 4 bits only, or the high 4 bits only */
static GFXDECODE_START( funcube )
	GFXDECODE_ENTRY( "gfx1", 0, funcube_layout_4bpp_lo, 0, 0x8000/16 )
	GFXDECODE_ENTRY( "gfx1", 0, funcube_layout_4bpp_hi, 0, 0x8000/16 )
	GFXDECODE_ENTRY( "gfx1", 0, funcube_layout_6bpp,    0, 0x8000/16 )	// 6bpp, but 4bpp granularity
	GFXDECODE_ENTRY( "gfx1", 0, funcube_layout_8bpp,    0, 0x8000/16 )	// 8bpp, but 4bpp granularity
	GFXDECODE_ENTRY( "gfx1", 0, funcube_layout_3bpp_lo, 0, 0x8000/16 )	// 3bpp, but 4bpp granularity
	GFXDECODE_ENTRY( "gfx1", 0, funcube_layout_2bpp_hi, 0, 0x8000/16 )	// ???
GFXDECODE_END


/***************************************************************************

                                Machine Drivers

***************************************************************************/

static INTERRUPT_GEN( seta2_interrupt )
{
	switch ( cpu_getiloops(device) )
	{
		case 0:
			/* VBlank is connected to INT0 (external interrupts pin 0) */
			tmp68301_external_interrupt_0(device->machine);
			break;
	}
}

static INTERRUPT_GEN( samshoot_interrupt )
{
	switch ( cpu_getiloops(device) )
	{
		case 0:
			tmp68301_external_interrupt_0(device->machine);	// vblank
			break;
		case 1:
			tmp68301_external_interrupt_2(device->machine);	// to do: hook up x1-10 interrupts
			break;
	}
}

static const x1_010_interface x1_010_sound_intf =
{
	0x0000,		/* address */
};


static MACHINE_DRIVER_START( mj4simai )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu",M68000,50000000/3)			/* !! TMP68301 @ 16.666666MHz !! */
	MDRV_CPU_PROGRAM_MAP(mj4simai_map)
	MDRV_CPU_VBLANK_INT("screen", seta2_interrupt)

	MDRV_MACHINE_START( tmp68301 )
	MDRV_MACHINE_RESET( tmp68301 )

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(0x200, 0x200)
	MDRV_SCREEN_VISIBLE_AREA(0x40, 0x1c0-1, 0x80, 0x170-1)

	MDRV_GFXDECODE(seta2)
	MDRV_PALETTE_LENGTH(0x8000+0xf0)	/* extra 0xf0 because we might draw 256-color object with 16-color granularity */

	MDRV_VIDEO_START(seta2)
	MDRV_VIDEO_UPDATE(seta2)
	MDRV_VIDEO_EOF(seta2)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("x1snd", X1_010, 50000000/3)
	MDRV_SOUND_CONFIG(x1_010_sound_intf)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( gundamex )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(mj4simai)
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(gundamex_map)

	MDRV_EEPROM_93C46_ADD("eeprom")

	/* video hardware */
	MDRV_SCREEN_MODIFY("screen")
	MDRV_SCREEN_VISIBLE_AREA(0x00, 0x180-1, 0x100, 0x1e0-1)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( grdians )

	MDRV_IMPORT_FROM(mj4simai)
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(grdians_map)

	/* video hardware */
	MDRV_SCREEN_MODIFY("screen")
	MDRV_SCREEN_VISIBLE_AREA(0x80, 0x80 + 0x130 -1, 0x80, 0x80 + 0xe8 -1)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( myangel )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(mj4simai)
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(myangel_map)

	/* video hardware */
	MDRV_SCREEN_MODIFY("screen")
	MDRV_SCREEN_VISIBLE_AREA(0, 0x178-1, 0x00, 0xf0-1)

	MDRV_VIDEO_START(seta2_offset)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( myangel2 )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(mj4simai)
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(myangel2_map)

	/* video hardware */
	MDRV_SCREEN_MODIFY("screen")
	MDRV_SCREEN_VISIBLE_AREA(0, 0x178-1, 0x00, 0xf0-1)

	MDRV_VIDEO_START(seta2_offset)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( pzlbowl )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(mj4simai)
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(pzlbowl_map)

	/* video hardware */
	MDRV_SCREEN_MODIFY("screen")
	MDRV_SCREEN_VISIBLE_AREA(0x10, 0x190-1, 0x100, 0x1f0-1)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( penbros )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(mj4simai)
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(penbros_map)

	/* video hardware */
	MDRV_SCREEN_MODIFY("screen")
	MDRV_SCREEN_VISIBLE_AREA(0, 0x140-1, 0x80, 0x160-1)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( samshoot )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(mj4simai)
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(samshoot_map)
	MDRV_CPU_VBLANK_INT_HACK(samshoot_interrupt,2)

	MDRV_NVRAM_HANDLER(generic_0fill)

	/* video hardware */
	MDRV_SCREEN_MODIFY("screen")
	MDRV_SCREEN_VISIBLE_AREA(0x40, 0x180-1, 0x40, 0x130-1)
MACHINE_DRIVER_END


/***************************************************************************
                                  Funcube
***************************************************************************/

static INTERRUPT_GEN( funcube_interrupt )
{
	switch ( cpu_getiloops(device) )
	{
		case 1:  cpu_set_input_line(device, 2, HOLD_LINE); break;
		case 0:  cpu_set_input_line(device, 1, HOLD_LINE); break;
	}
}

static INTERRUPT_GEN( funcube_sub_timer_irq )
{
	if ( funcube_serial_count )
	{
		cpu_set_input_line(device, H8_SCI_1_RX, HOLD_LINE);
	}
	else
	{
		UINT8 press   = input_port_read(device->machine,"TOUCH_PRESS");
		UINT8 release = funcube_press && !press;

		if ( press || release )
		{
			funcube_serial_fifo[0] = press ? 0xfe : 0xfd;
			funcube_serial_fifo[1] = input_port_read(device->machine,"TOUCH_X");
			funcube_serial_fifo[2] = input_port_read(device->machine,"TOUCH_Y");
			funcube_serial_fifo[3] = 0xff;
			funcube_serial_count = 4;
		}

		funcube_press = press;
	}

	cpu_set_input_line(device, H8_METRO_TIMER_HACK, HOLD_LINE);
}

static MACHINE_RESET( funcube )
{
	funcube_coin_start_cycles = 0;
	funcube_serial_count = 0;
	funcube_press = 0;
	funcube_hopper_motor = 0;
}

static MACHINE_DRIVER_START( funcube )
	MDRV_CPU_ADD("maincpu", M68040, XTAL_25_447MHz) // !! XCF5206 actually !!
	MDRV_CPU_PROGRAM_MAP(funcube_map)
	MDRV_CPU_VBLANK_INT_HACK(funcube_interrupt,2)

	MDRV_CPU_ADD("sub", H83007, FUNCUBE_SUB_CPU_CLOCK)
	MDRV_CPU_PROGRAM_MAP(funcube_sub_map)
	MDRV_CPU_IO_MAP(funcube_sub_io)
	MDRV_CPU_PERIODIC_INT(funcube_sub_timer_irq, 60*10 )

	MDRV_NVRAM_HANDLER(generic_0fill)

	MDRV_MACHINE_RESET( funcube )

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))	// not accurate
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(0x200, 0x200)
	MDRV_SCREEN_VISIBLE_AREA(0x0, 0x140-1, 0x80, 0x170-1)

	MDRV_GFXDECODE(funcube)
	MDRV_PALETTE_LENGTH(0x8000+0xf0)	/* extra 0xf0 because we might draw 256-color object with 16-color granularity */

	MDRV_VIDEO_START(seta2)
	MDRV_VIDEO_UPDATE(seta2)
	MDRV_VIDEO_EOF(seta2)

	/* sound hardware */

	// MSM9810B

MACHINE_DRIVER_END


/***************************************************************************

                                ROMs Loading

***************************************************************************/

ROM_START( gundamex )
	ROM_REGION( 0x600000, "maincpu", 0 )		/* TMP68301 Code */
	ROM_LOAD16_BYTE(	  "ka002002.u2",  0x000000, 0x080000, CRC(e850f6d8) SHA1(026325e305676b1f8d3d9e7573920f8b70d7bccb) )
	ROM_LOAD16_BYTE(	  "ka002004.u3",  0x000001, 0x080000, CRC(c0fb1208) SHA1(84b25e4c73cb8e023ee5dbf69f588be98700b43f) )
	ROM_LOAD16_BYTE(	  "ka002001.u4",  0x100000, 0x080000, CRC(553ebe6b) SHA1(7fb8a159513d31a1d60520ff14e4c4d133fd3e19) )
	ROM_LOAD16_BYTE(	  "ka002003.u5",  0x100001, 0x080000, CRC(946185aa) SHA1(524911c4c510d6c3e17a7ab42c7077c2fffbf06b) )
	ROM_LOAD16_WORD_SWAP( "ka001005.u77", 0x500000, 0x080000, CRC(f01d3d00) SHA1(ff12834e99a76261d619f10d186f4b329fb9cb7a) )

	ROM_REGION( 0x2000000, "gfx1", ROMREGION_ERASE)	/* Sprites */
	ROM_LOAD( "ka001009.u16",  0x0000000, 0x200000, CRC(997d8d93) SHA1(4cb4cdb7e8208af4b14483610d9d6aa5e13acd89) )
	ROM_LOAD( "ka001010.u18",  0x0200000, 0x200000, CRC(811b67ca) SHA1(c8cfae6f54c76d63bd625ff011c872ffb75fd2e2) )
	ROM_LOAD( "ka001011.u20",  0x0400000, 0x200000, CRC(08a72700) SHA1(fb8003aa02dd249c30a757cb43b516260b41c1bf) )
	ROM_LOAD( "ka001012.u15",  0x0800000, 0x200000, CRC(b789e4a8) SHA1(400b773f24d677a9d47466fdbbe68cb6efc1ad37) )
	ROM_LOAD( "ka001013.u17",  0x0a00000, 0x200000, CRC(d8a0201f) SHA1(fe8a2407c872adde8aec8e9340b00be4f00a2872) )
	ROM_LOAD( "ka001014.u19",  0x0c00000, 0x200000, CRC(7635e026) SHA1(116a3daab14a17faca85c4a956b356aaf0fc2276) )
	ROM_LOAD( "ka001006.u21",  0x1000000, 0x200000, CRC(6aac2f2f) SHA1(fac5478ca2941a93c57f670a058ff626e537bcde) )
	ROM_LOAD( "ka001007.u22",  0x1200000, 0x200000, CRC(588f9d63) SHA1(ed5148d09d02e3bc12c50c39c5c86e6356b2dd7a) )
	ROM_LOAD( "ka001008.u23",  0x1400000, 0x200000, CRC(db55a60a) SHA1(03d118c7284ca86219891c473e2a89489710ea27) )
	ROM_FILL(                  0x1800000, 0x600000, 0 )	/* 6bpp instead of 8bpp */

	ROM_REGION( 0x300000, "x1snd", 0 )	/* Samples */
	/* Leave 1MB empty (addressable by the chip) */
	ROM_LOAD( "ka001015.u28", 0x100000, 0x200000, CRC(ada2843b) SHA1(09d06026031bc7558da511c3c0e29187ea0a0099) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "eeprom.bin", 0x0000, 0x0080, CRC(80f8e248) SHA1(1a9787811e56d95f7acbedfb00225b6e7df265eb) )
ROM_END

ROM_START( grdians )
	ROM_REGION( 0x200000, "maincpu", 0 )		/* TMP68301 Code */
	ROM_LOAD16_BYTE( "u2.bin", 0x000000, 0x080000, CRC(36adc6f2) SHA1(544e87f88179fe1342e7a06a8948ac1828e85108) )
	ROM_LOAD16_BYTE( "u3.bin", 0x000001, 0x080000, CRC(2704f416) SHA1(9081a12cbb9927d36e1c50b52aa2c6003810ee42) )
	ROM_LOAD16_BYTE( "u4.bin", 0x100000, 0x080000, CRC(bb52447b) SHA1(61433f683210ab2bc2cf1cc4b5b7a39cc5b6493d) )
	ROM_LOAD16_BYTE( "u5.bin", 0x100001, 0x080000, CRC(9c164a3b) SHA1(6d688c7af9e7e8e8d54b2e4dfbf41f59c79242eb) )

	ROM_REGION( 0x2000000, "gfx1", ROMREGION_ERASE)	/* Sprites */
	ROM_LOAD( "u16.bin",  0x0000000, 0x400000, CRC(6a65f265) SHA1(6cad11f718f8bbcff464d41eb4717460769237ed) )
	ROM_LOAD( "u20.bin",  0x0600000, 0x200000, CRC(a7226ab7) SHA1(408580dd35c568ffef1ebbd87359e3ec1f867020) )
	ROM_CONTINUE(         0x0400000, 0x200000 )

	ROM_LOAD( "u15.bin",  0x0800000, 0x400000, CRC(01672dcd) SHA1(f61f60e3343cc5b6ccee391ee529966a141566db) )
	ROM_LOAD( "u19.bin",  0x0e00000, 0x200000, CRC(c0c998a0) SHA1(498fb1877527ed37412537f06a2c39ff0c60f146) )
	ROM_CONTINUE(         0x0c00000, 0x200000 )

	ROM_LOAD( "u18.bin",  0x1000000, 0x400000, CRC(967babf4) SHA1(42a6311576417c44aeaceb8ba6bb3cd7794e4882) )
	ROM_LOAD( "u22.bin",  0x1600000, 0x200000, CRC(6239997a) SHA1(87b6d6f30f152f625f82fd858c1290176c7e156e) )
	ROM_CONTINUE(         0x1400000, 0x200000 )

	ROM_LOAD( "u17.bin",  0x1800000, 0x400000, CRC(0fad0629) SHA1(1bdc8e7c5e39e83d327f14a672ec81b049112da6) )
	ROM_LOAD( "u21.bin",  0x1e00000, 0x200000, CRC(6f95e466) SHA1(28482fad16a3ac9302f152d81552e6f84a44f3e4) )
	ROM_CONTINUE(         0x1c00000, 0x200000 )

	ROM_REGION( 0x200000, "x1snd", 0 )	/* Samples */
	/* Leave 1MB empty (addressable by the chip) */
	ROM_LOAD( "u32.bin", 0x100000, 0x100000, CRC(cf0f3017) SHA1(8376d3a674f71aec72f52c72758fbc53d9feb1a1) )
ROM_END

ROM_START( mj4simai )
	ROM_REGION( 0x200000, "maincpu", 0 )		/* TMP68301 Code */
	ROM_LOAD16_BYTE( "ll.u2",       0x000000, 0x080000, CRC(7be9c781) SHA1(d29e579706d98909933f6bed2ee292c88ed10d2c) )
	ROM_LOAD16_BYTE( "lh1.u3",      0x000001, 0x080000, CRC(82aa3f72) SHA1(a93d5dc7cdf12f852a692759d91f6f2951b6b5b5) )
	ROM_LOAD16_BYTE( "hl.u4",       0x100000, 0x080000, CRC(226063b7) SHA1(1737baffc16ff7261f887911187ece96925fa6ff) )
	ROM_LOAD16_BYTE( "hh.u5",       0x100001, 0x080000, CRC(23aaf8df) SHA1(b3d678afce4ddef32e48d690c6d07b723dd0c28f) )

	ROM_REGION( 0x2000000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "cha-03.u16",  0x0000000, 0x400000, CRC(d367429a) SHA1(b32c215ef85c3d0a4c5550cef4f5c4c0e7030b7c) )
	ROM_LOAD( "cha-04.u18",  0x0400000, 0x400000, CRC(7f2008c3) SHA1(e45d863540eb2381f5d7660d64cdfef87c890768) )
	ROM_LOAD( "cha-05.u15",  0x0800000, 0x400000, CRC(e94ec40a) SHA1(2685dbc5680b5f76688c6b4fbe40ae682c525bfe) )
	ROM_LOAD( "cha-06.u17",  0x0c00000, 0x400000, CRC(5cb0b3a9) SHA1(92fb82d45b4c46326d5796981f812e20a8ddb4f2) )
	ROM_LOAD( "cha-01.u21",  0x1000000, 0x400000, CRC(35f47b37) SHA1(4a8eb088890272f2a069e2c3f00fadf6421f7b0e) )
	ROM_LOAD( "cha-02.u22",  0x1400000, 0x400000, CRC(f6346860) SHA1(4eebd3fa315b97964fa39b88224f9de7622ba881) )
	ROM_FILL(                0x1800000, 0x800000, 0 )	/* 6bpp instead of 8bpp */

	ROM_REGION( 0x500000, "x1snd", 0 )	/* Samples */
	/* Leave 1MB empty (addressable by the chip) */
	ROM_LOAD( "cha-07.u32",  0x100000, 0x400000, CRC(817519ee) SHA1(ed09740cdbf61a328f7b50eb569cf498fb749416) )
ROM_END

ROM_START( myangel )
	ROM_REGION( 0x200000, "maincpu", 0 )		/* TMP68301 Code */
	ROM_LOAD16_BYTE( "kq1-prge.u2", 0x000000, 0x080000, CRC(6137d4c0) SHA1(762341e11b56e4a7787a0662833b702b78aee0a9) )
	ROM_LOAD16_BYTE( "kq1-prgo.u3", 0x000001, 0x080000, CRC(4aad10d8) SHA1(a08e1c4f57c64be829e0807ae2791da947fd60aa) )
	ROM_LOAD16_BYTE( "kq1-tble.u4", 0x100000, 0x080000, CRC(e332a514) SHA1(dfd255239c80c48c9865e70681b9ddd175b8bf55) )
	ROM_LOAD16_BYTE( "kq1-tblo.u5", 0x100001, 0x080000, CRC(760cab15) SHA1(fa7ea85ec2ebfaab3111b8631ea6ea3d794d449c) )

	ROM_REGION( 0x1000000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "kq1-cg2.u20", 0x000000, 0x200000, CRC(80b4e8de) SHA1(c8685c4f4e3c0415ce0ec88e0288835e504cab00) )
	ROM_LOAD( "kq1-cg0.u16", 0x200000, 0x200000, CRC(f8ae9a05) SHA1(4f3b41386a48a1608aa96b911e6b74ca775260fb) )
	ROM_LOAD( "kq1-cg3.u19", 0x400000, 0x200000, CRC(9bdc35c9) SHA1(fd0a1eb3dd10705bce5462263667353632558b58) )
	ROM_LOAD( "kq1-cg1.u15", 0x600000, 0x200000, CRC(23bd7ea4) SHA1(e925bbadc33fc2586bb18283cf989ab35f28c1e9) )
	ROM_LOAD( "kq1-cg6.u22", 0x800000, 0x200000, CRC(b25acf12) SHA1(5cca35921f3b376c3cc36f5b009eb845db2e1897) )
	ROM_LOAD( "kq1-cg4.u18", 0xa00000, 0x200000, CRC(dca7f8f2) SHA1(20595c7940a28d01bdc6610b67aaaeac61ba92e2) )
	ROM_LOAD( "kq1-cg7.u21", 0xc00000, 0x200000, CRC(9f48382c) SHA1(80dfc33a55123b5d3cdb3ed97b43a527f0254d61) )
	ROM_LOAD( "kq1-cg5.u17", 0xe00000, 0x200000, CRC(a4bc4516) SHA1(0eb11fa54d16bba1b96f9dd943a68949a3bb9a2f) )

	ROM_REGION( 0x300000, "x1snd", 0 )	/* Samples */
	/* Leave 1MB empty (addressable by the chip) */
	ROM_LOAD( "kq1-snd.u32", 0x100000, 0x200000, CRC(8ca1b449) SHA1(f54096fb5400843af4879135c96760485b6cb319) )
ROM_END

ROM_START( myangel2 )
	ROM_REGION( 0x200000, "maincpu", 0 )		/* TMP68301 Code */
	ROM_LOAD16_BYTE( "kqs1ezpr.u2", 0x000000, 0x080000, CRC(2469aac2) SHA1(7dade2de31252e305d24c659c4801dd4687ad1f6) )
	ROM_LOAD16_BYTE( "kqs1ozpr.u3", 0x000001, 0x080000, CRC(6336375c) SHA1(72089f77e94832e74e0512944acadeccd0dec8b0) )
	ROM_LOAD16_BYTE( "kqs1e-tb.u4", 0x100000, 0x080000, CRC(e759b4cc) SHA1(4f806a144a47935b2710f8af800ec0d771f12a18) )
	ROM_LOAD16_BYTE( "kqs1o-tb.u5", 0x100001, 0x080000, CRC(b6168737) SHA1(4c3de877c0c1dca1c43ac737a0bf231335237d3a) )

	ROM_REGION( 0x1800000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "kqs1-cg4.u20", 0x0000000, 0x200000, CRC(d1802241) SHA1(52c45a13d46f7ee8043e85b99d07b1765ca93dcc) )
	ROM_LOAD( "kqs1-cg0.u16", 0x0200000, 0x400000, CRC(c21a33a7) SHA1(bc6f479a8f4c716ba79a725f160ddeb95fdedbcb) )
	ROM_LOAD( "kqs1-cg5.u19", 0x0600000, 0x200000, CRC(d86cf19c) SHA1(da5a5b576ce107433605b24d8b9dcd0abd46bcde) )
	ROM_LOAD( "kqs1-cg1.u15", 0x0800000, 0x400000, CRC(dca799ba) SHA1(8379b11472c27b1945fe7fc274c7fedf756accba) )
	ROM_LOAD( "kqs1-cg6.u22", 0x0c00000, 0x200000, CRC(3f08886b) SHA1(054546ae44ffa5d0973f4ead080fe720a340e144) )
	ROM_LOAD( "kqs1-cg2.u18", 0x0e00000, 0x400000, CRC(f7f92c7e) SHA1(24a525a15fded0de6e382b346da6bd5e7b9eced5) )
	ROM_LOAD( "kqs1-cg7.u21", 0x1200000, 0x200000, CRC(2c977904) SHA1(2589447f2471cdc414266b34aff552044c680d93) )
	ROM_LOAD( "kqs1-cg3.u17", 0x1400000, 0x400000, CRC(de3b2191) SHA1(d7d6ea07b665cfd834747d3c0776b968ce03bc6a) )

	ROM_REGION( 0x500000, "x1snd", 0 )	/* Samples */
	/* Leave 1MB empty (addressable by the chip) */
	ROM_LOAD( "kqs1-snd.u32", 0x100000, 0x400000, CRC(792a6b49) SHA1(341b4e8f248b5032217733bada32e353c67e3888) )
ROM_END

ROM_START( pzlbowl )
	ROM_REGION( 0x100000, "maincpu", 0 )		/* TMP68301 Code */
	ROM_LOAD16_BYTE( "kup-u06.i03", 0x000000, 0x080000, CRC(314e03ac) SHA1(999398e55161dd75570d418f4c9899e3bf311cc8) )
	ROM_LOAD16_BYTE( "kup-u07.i03", 0x000001, 0x080000, CRC(a0423a04) SHA1(9539023c5c2f2bf72ee3fb6105443ffd3d61e2f8) )

	ROM_REGION( 0x1000000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "kuc-u38.i00", 0x000000, 0x400000, CRC(3db24172) SHA1(89c39963e15c53b799994185d0c8b2e795478939) )
	ROM_LOAD( "kuc-u39.i00", 0x400000, 0x400000, CRC(9b26619b) SHA1(ea7a0bf46641d15353217b01e761d1a148bee4e7) )
	ROM_LOAD( "kuc-u40.i00", 0x800000, 0x400000, CRC(7e49a2cf) SHA1(d24683addbc54515c33fb620ac500e6702bd9e17) )
	ROM_LOAD( "kuc-u41.i00", 0xc00000, 0x400000, CRC(2febf19b) SHA1(8081ac590c0463529777b5e4817305a1a6f6ea41) )

	ROM_REGION( 0x500000, "x1snd", 0 )	/* Samples */
	/* Leave 1MB empty (addressable by the chip) */
	ROM_LOAD( "kus-u18.i00", 0x100000, 0x400000, CRC(e2b1dfcf) SHA1(fb0b8be119531a1a27efa46ed7b86b05a37ed585) )
ROM_END

ROM_START( penbros )
	ROM_REGION( 0x100000, "maincpu", 0 )		/* TMP68301 Code */
	ROM_LOAD16_BYTE( "u06.bin", 0x000000, 0x080000, CRC(7bbdffac) SHA1(d5766cb171b8d2e4c04a6bae37181fa5ada9d797) )
	ROM_LOAD16_BYTE( "u07.bin", 0x000001, 0x080000, CRC(d50cda5f) SHA1(fc66f55f2070b447c5db85c948ce40adc37512f7) )

	ROM_REGION( 0x1000000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "u38.bin", 0x000000, 0x400000, CRC(4247b39e) SHA1(f273931293beced312e02c870bf35e9cf0c91a8b) )
	ROM_LOAD( "u39.bin", 0x400000, 0x400000, CRC(f9f07faf) SHA1(66fc4a9ad422fb384d2c775e43619137226898fc) )
	ROM_LOAD( "u40.bin", 0x800000, 0x400000, CRC(dc9e0a96) SHA1(c2c8ccf9039ee0e179b08fdd2d37f29899349cda) )
	ROM_FILL(            0xc00000, 0x400000, 0 )	/* 6bpp instead of 8bpp */

	ROM_REGION( 0x300000, "x1snd", 0 )	/* Samples */
	/* Leave 1MB empty (addressable by the chip) */
	ROM_LOAD( "u18.bin", 0x100000, 0x200000, CRC(de4e65e2) SHA1(82d4e590c714b3e9bf0ffaf1500deb24fd315595) )
ROM_END

ROM_START( deerhunt ) /* Deer Hunting USA V4.3 (11/1/2000) - The "E05" breaks version label conventions but is correct & verified */
	ROM_REGION( 0x200000, "maincpu", 0 )		/* TMP68301 Code */
	ROM_LOAD16_BYTE( "as0906e05.u06", 0x000000, 0x100000, CRC(20c81f17) SHA1(d41d93d6ee88738cec55f7bf3ce6be1dbec68e09) ) /* checksum 694E printed on label */
	ROM_LOAD16_BYTE( "as0907e05.u07", 0x000001, 0x100000, CRC(1731aa2a) SHA1(cffae7a99a7f960a62ef0c4454884df17a93c1a6) ) /* checksum 5D89 printed on label */

	ROM_REGION( 0x2000000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "as0901m01.u38", 0x0000000, 0x800000, CRC(1d6acf8f) SHA1(6f61fe21bebb7c87e8e6c3ef3ba73b8cf327dde9) )
	ROM_LOAD( "as0902m01.u39", 0x0800000, 0x800000, CRC(c7ca2128) SHA1(86be3a3ec2f86f61acfa3d4d261faea3c27dc378) )
	ROM_LOAD( "as0903m01.u40", 0x1000000, 0x800000, CRC(e8ef81b3) SHA1(97666942ca6cca5b8ea6451314a2aaabad9e06ba) )
	ROM_LOAD( "as0904m01.u41", 0x1800000, 0x800000, CRC(d0f97fdc) SHA1(776c9d42d03a9f61155521212305e1ed696eaf47) )

	ROM_REGION( 0x500000, "x1snd", 0 )	/* Samples */
	/* Leave 1MB empty (addressable by the chip) */
	ROM_LOAD( "as0905m01.u18", 0x100000, 0x400000, CRC(8d8165bb) SHA1(aca7051613d260734ee787b4c3db552c336bd600) )
ROM_END

ROM_START( deerhunta ) /* Deer Hunting USA V4.2 (xx/x/2000) */
	ROM_REGION( 0x200000, "maincpu", 0 )		/* TMP68301 Code */
	ROM_LOAD16_BYTE( "as0906e04-v4_2.u06", 0x000000, 0x100000, CRC(bb3af36f) SHA1(f04071347e8ad361bf666fcb6c0136e522f19d47) ) /* checksum 6640 printed on label */
	ROM_LOAD16_BYTE( "as0907e04-v4_2.u07", 0x000001, 0x100000, CRC(83f02117) SHA1(70fc2291bc93af3902aae88688be6a8078f7a07e) ) /* checksum 595A printed on label */

	ROM_REGION( 0x2000000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "as0901m01.u38", 0x0000000, 0x800000, CRC(1d6acf8f) SHA1(6f61fe21bebb7c87e8e6c3ef3ba73b8cf327dde9) )
	ROM_LOAD( "as0902m01.u39", 0x0800000, 0x800000, CRC(c7ca2128) SHA1(86be3a3ec2f86f61acfa3d4d261faea3c27dc378) )
	ROM_LOAD( "as0903m01.u40", 0x1000000, 0x800000, CRC(e8ef81b3) SHA1(97666942ca6cca5b8ea6451314a2aaabad9e06ba) )
	ROM_LOAD( "as0904m01.u41", 0x1800000, 0x800000, CRC(d0f97fdc) SHA1(776c9d42d03a9f61155521212305e1ed696eaf47) )

	ROM_REGION( 0x500000, "x1snd", 0 )	/* Samples */
	/* Leave 1MB empty (addressable by the chip) */
	ROM_LOAD( "as0905m01.u18", 0x100000, 0x400000, CRC(8d8165bb) SHA1(aca7051613d260734ee787b4c3db552c336bd600) )
ROM_END

ROM_START( deerhuntb ) /* Deer Hunting USA V4.0 (6/15/2000) */
	ROM_REGION( 0x200000, "maincpu", 0 )		/* TMP68301 Code */
	ROM_LOAD16_BYTE( "as0906e04.u06", 0x000000, 0x100000, CRC(07d9b64a) SHA1(f9aac644aab920bbac84b14836ee589ccd51f6db) ) /* checksum 7BBB printed on label */
	ROM_LOAD16_BYTE( "as0907e04.u07", 0x000001, 0x100000, CRC(19973d08) SHA1(da1cc02ce480a62ccaf94d0af1246a340f054b43) ) /* checksum 4C78 printed on label */

	ROM_REGION( 0x2000000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "as0901m01.u38", 0x0000000, 0x800000, CRC(1d6acf8f) SHA1(6f61fe21bebb7c87e8e6c3ef3ba73b8cf327dde9) )
	ROM_LOAD( "as0902m01.u39", 0x0800000, 0x800000, CRC(c7ca2128) SHA1(86be3a3ec2f86f61acfa3d4d261faea3c27dc378) )
	ROM_LOAD( "as0903m01.u40", 0x1000000, 0x800000, CRC(e8ef81b3) SHA1(97666942ca6cca5b8ea6451314a2aaabad9e06ba) )
	ROM_LOAD( "as0904m01.u41", 0x1800000, 0x800000, CRC(d0f97fdc) SHA1(776c9d42d03a9f61155521212305e1ed696eaf47) )

	ROM_REGION( 0x500000, "x1snd", 0 )	/* Samples */
	/* Leave 1MB empty (addressable by the chip) */
	ROM_LOAD( "as0905m01.u18", 0x100000, 0x400000, CRC(8d8165bb) SHA1(aca7051613d260734ee787b4c3db552c336bd600) )
ROM_END

	/* There are known versions 3.x of Deer Hunting USA.... just none are currently dumped.  roms should be "AS0906 E03 U06" & "AS0907 E03 U07" */

ROM_START( deerhuntc ) /* Deer Hunting USA V2.x - No version number is printed to screen but "E02" in EPROM label signifies V2 */
	ROM_REGION( 0x200000, "maincpu", 0 )		/* TMP68301 Code */
	ROM_LOAD16_BYTE( "as0906e02.u06", 0x000000, 0x100000, CRC(190cca42) SHA1(aef63f5e8c71ed0156b8b0104c5d23872c119167) ) /* Version in program code is listed as 0.00 */
	ROM_LOAD16_BYTE( "as0907e02.u07", 0x000001, 0x100000, CRC(9de2b901) SHA1(d271bc54c41e30c0d9962eedd22f3ef2b7b8c9e5) ) /* Verified with two different sets of chips */

	ROM_REGION( 0x2000000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "as0901m01.u38", 0x0000000, 0x800000, CRC(1d6acf8f) SHA1(6f61fe21bebb7c87e8e6c3ef3ba73b8cf327dde9) )
	ROM_LOAD( "as0902m01.u39", 0x0800000, 0x800000, CRC(c7ca2128) SHA1(86be3a3ec2f86f61acfa3d4d261faea3c27dc378) )
	ROM_LOAD( "as0903m01.u40", 0x1000000, 0x800000, CRC(e8ef81b3) SHA1(97666942ca6cca5b8ea6451314a2aaabad9e06ba) )
	ROM_LOAD( "as0904m01.u41", 0x1800000, 0x800000, CRC(d0f97fdc) SHA1(776c9d42d03a9f61155521212305e1ed696eaf47) )

	ROM_REGION( 0x500000, "x1snd", 0 )	/* Samples */
	/* Leave 1MB empty (addressable by the chip) */
	ROM_LOAD( "as0905m01.u18", 0x100000, 0x400000, CRC(8d8165bb) SHA1(aca7051613d260734ee787b4c3db552c336bd600) )
ROM_END

ROM_START( turkhunt ) /* V1.0 is currently the only known version */
	ROM_REGION( 0x200000, "maincpu", 0 )		/* TMP68301 Code */
	ROM_LOAD16_BYTE( "asx906e01.u06", 0x000000, 0x100000, CRC(c96266e1) SHA1(0ca462b3b0f27198e36384eee6ea5c5d4e7e1293) ) /* checksum E510 printed on label */
	ROM_LOAD16_BYTE( "asx907e01.u07", 0x000001, 0x100000, CRC(7c67b502) SHA1(6a0e8883a115dac4095d86897e7eca2a007a1c71) ) /* checksum AB40 printed on label */

	ROM_REGION( 0x2000000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "asx901m01.u38", 0x0000000, 0x800000, CRC(eabd3f44) SHA1(5a1ac986d11a8b019e18761cf4ea0a6f49fbdbfc) )
	ROM_LOAD( "asx902m01.u39", 0x0800000, 0x800000, CRC(c32130c8) SHA1(70d56ebed1f51657aaee02f95ac51589733e6eb7) )
	ROM_LOAD( "asx903m01.u40", 0x1000000, 0x800000, CRC(5f86c322) SHA1(5a72adb99eea176199f172384cb051e2b045ab94) )
	ROM_LOAD( "asx904m01.u41", 0x1800000, 0x800000, CRC(c77e0b66) SHA1(0eba30e62e4bd38c198fa6cb69fb94d002ded77a) )

	ROM_REGION( 0x500000, "x1snd", 0 )	/* Samples */
	/* Leave 1MB empty (addressable by the chip) */
	ROM_LOAD( "asx905m01.u18", 0x100000, 0x400000, CRC(8d9dd9a9) SHA1(1fc2f3688d2c24c720dca7357bca6bf5f4016c53) )
ROM_END

ROM_START( wschamp ) /* Wing Shootiong Championship V2.00 (01/23/2002) */
	ROM_REGION( 0x200000, "maincpu", 0 )		/* TMP68301 Code */
	ROM_LOAD16_BYTE( "as1006e03.u06", 0x000000, 0x100000, CRC(0ad01677) SHA1(63e09b9f7cc8b781af1756f86caa0cc0962ae584) ) /* checksum 421E printed on label */
	ROM_LOAD16_BYTE( "as1007e03.u07", 0x000001, 0x100000, CRC(572624f0) SHA1(0c2f67daa22f4edd66a2be990dc6cd999faff0fa) ) /* checksum A48F printed on label */

	ROM_REGION( 0x2000000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "as1001m01.u38", 0x0000000, 0x800000, CRC(92595579) SHA1(75a7131aedb18b7103677340c3cca7c91aaca2bf) )
	ROM_LOAD( "as1002m01.u39", 0x0800000, 0x800000, CRC(16c2bb08) SHA1(63926464c8bd8db7d05905a953765e645942beb4) )
	ROM_LOAD( "as1003m01.u40", 0x1000000, 0x800000, CRC(89618858) SHA1(a8bd07f233482e8f5a256af7ff9577648eb58ef4) )
	ROM_LOAD( "as1004m01.u41", 0x1800000, 0x800000, CRC(500c0909) SHA1(73ff27d46b9285f34a50a81c21c54437f21e1939) )

	ROM_REGION( 0x500000, "x1snd", 0 )	/* Samples */
	/* Leave 1MB empty (addressable by the chip) */
	ROM_LOAD( "as1005m01.u18", 0x100000, 0x400000, CRC(e4b137b8) SHA1(4d8d15073c51f7d383282cc5755ae5b2eab6226c) )
ROM_END

ROM_START( wschampa ) /* Wing Shootiong Championship V1.01 */
	ROM_REGION( 0x200000, "maincpu", 0 )		/* TMP68301 Code */
	ROM_LOAD16_BYTE( "as1006e02.u06", 0x000000, 0x100000, CRC(d3d3b2b5) SHA1(2d036d795b40a4ed78bb9f7751f875cfc76276a9) ) /* checksum 31EF printed on label */
	ROM_LOAD16_BYTE( "as1007e02.u07", 0x000001, 0x100000, CRC(78ede6d9) SHA1(e6d10f52cd4c6bf97288df44911f23bb64fc012c) ) /* checksum 615E printed on label */

	ROM_REGION( 0x2000000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "as1001m01.u38", 0x0000000, 0x800000, CRC(92595579) SHA1(75a7131aedb18b7103677340c3cca7c91aaca2bf) )
	ROM_LOAD( "as1002m01.u39", 0x0800000, 0x800000, CRC(16c2bb08) SHA1(63926464c8bd8db7d05905a953765e645942beb4) )
	ROM_LOAD( "as1003m01.u40", 0x1000000, 0x800000, CRC(89618858) SHA1(a8bd07f233482e8f5a256af7ff9577648eb58ef4) )
	ROM_LOAD( "as1004m01.u41", 0x1800000, 0x800000, CRC(500c0909) SHA1(73ff27d46b9285f34a50a81c21c54437f21e1939) )

	ROM_REGION( 0x500000, "x1snd", 0 )	/* Samples */
	/* Leave 1MB empty (addressable by the chip) */
	ROM_LOAD( "as1005m01.u18", 0x100000, 0x400000, CRC(e4b137b8) SHA1(4d8d15073c51f7d383282cc5755ae5b2eab6226c) )
ROM_END

ROM_START( trophyh ) /* V1.0 is currently the only known version */
	ROM_REGION( 0x200000, "maincpu", 0 )		/* TMP68301 Code */
	ROM_LOAD16_BYTE( "as1106e01.u06", 0x000000, 0x100000, CRC(b4950882) SHA1(2749f7ffc5b543c9f39815f0913a1d1e385b63f4) ) /* checksum D8DA printed on label */
	ROM_LOAD16_BYTE( "as1107e01.u07", 0x000001, 0x100000, CRC(19ee67cb) SHA1(e75ce66d3ff5aad46ba997c09d6514260e617f55) ) /* checksum CEEF printed on label */

	ROM_REGION( 0x2000000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "as1101m01.u38", 0x0000000, 0x800000, CRC(855ed675) SHA1(84ce229a9feb6331413253a5aed10b362e8102e5) )
	ROM_LOAD( "as1102m01.u39", 0x0800000, 0x800000, CRC(d186d271) SHA1(3c54438b35adfab8be91df0a633270d6db49beef) )
	ROM_LOAD( "as1103m01.u40", 0x1000000, 0x800000, CRC(adf8a54e) SHA1(bb28bf219d18082246f7964851a5c49b9c0ba7f5) )
	ROM_LOAD( "as1104m01.u41", 0x1800000, 0x800000, CRC(387882e9) SHA1(0fdd0c77dabd1066c6f3bd64e357236a76f524ab) )

	ROM_REGION( 0x500000, "x1snd", 0 )	/* Samples */
	/* Leave 1MB empty (addressable by the chip) */
	ROM_LOAD( "as1105m01.u18", 0x100000, 0x400000, CRC(633d0df8) SHA1(3401c424f5c207ef438a9269e0c0e7d482771fed) )
ROM_END

ROM_START( funcube2 )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* XCF5206 Code */
	ROM_LOAD( "fc21_prg-0b.u3", 0x00000, 0x80000, CRC(add1c8a6) SHA1(bf91518da659098a4bad4e756533525fcc910570) )

	ROM_REGION( 0x20000, "sub", 0 ) /* H8/3007 Code */
	ROM_LOAD( "fc21_iopr-0.u49", 0x00000, 0x20000, CRC(314555ef) SHA1(b17e3926c8ef7f599856c198c330d2051aae13ad) )

	ROM_REGION( 0x300, "pic", 0 ) /* PIC12C508? Code */
	ROM_LOAD( "fc21a", 0x000, 0x300, NO_DUMP )

	ROM_REGION( 0x800000, "gfx1", 0 )
	ROM_LOAD32_WORD( "fc21_obj-0.u43", 0x000000, 0x400000, CRC(08cfe6d9) SHA1(d10f362dcde01f7a9855d8f76af3084b5dd1573a) )
	ROM_LOAD32_WORD( "fc21_obj-1.u42", 0x000002, 0x400000, CRC(4c1fbc20) SHA1(ff83691c19ce3600b31c494eaec26d2ac79e0028) )

	ROM_REGION( 0x400000, "samples", 0 )
	ROM_LOAD( "fc21_voi0.u47", 0x00000, 0x400000, CRC(25b5fc3f) SHA1(18b16a14e9ee62f3fea382e9d3fdcd43bdb165f5) )
ROM_END

ROM_START( funcube4 )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* XCF5206 Code */
	ROM_LOAD( "fc41_prg-0.u3", 0x00000, 0x80000, CRC(ef870874) SHA1(dcb8dc3f780ca135df55e4b4f3c95620597ad28f) )

	ROM_REGION( 0x20000, "sub", 0 ) /* H8/3007 Code */
	ROM_LOAD( "fc21_iopr-0.u49", 0x00000, 0x20000, CRC(314555ef) SHA1(b17e3926c8ef7f599856c198c330d2051aae13ad) )

	ROM_REGION( 0x300, "pic", 0 ) /* PIC12C508? Code */
	ROM_LOAD( "fc41a", 0x000, 0x300, NO_DUMP )

	ROM_REGION( 0x800000, "gfx1", 0 )
	ROM_LOAD32_WORD( "fc41_obj-0.u43", 0x000000, 0x400000, CRC(9ff029d5) SHA1(e057f4929aa745ecaf9d4ff7e39974c82e440146) )
	ROM_LOAD32_WORD( "fc41_obj-1.u42", 0x000002, 0x400000, CRC(5ab7b087) SHA1(c600158b2358cdf947357170044dda2deacd4f37) )

	ROM_REGION( 0x400000, "samples", 0 )
	ROM_LOAD( "fc41_snd0.u47", 0x00000, 0x400000, CRC(48337257) SHA1(d1755024b824100070b489f48f6ae921765329e8) )
ROM_END

static DRIVER_INIT( funcube2 )
{
	UINT32 *main_cpu = (UINT32 *) memory_region(machine, "maincpu");
	UINT16 *sub_cpu  = (UINT16 *) memory_region(machine, "sub");

	main_cpu[0x810/4] = 0xe0214e71;
	main_cpu[0x814/4] = 0x4e71203c;

	main_cpu[0x81c/4] = 0x4e714e71;

	main_cpu[0xa5c/4] = 0x4e713e3c;
	main_cpu[0xa74/4] = 0x4e713e3c;
	main_cpu[0xa8c/4] = 0x4e7141f9;

	// Sub CPU

	sub_cpu[0x4d4/2] = 0x5470;	// rte -> rts
}

// Note: same as funcube2
static DRIVER_INIT( funcube4 )
{
	UINT32 *main_cpu = (UINT32 *) memory_region(machine, "maincpu");
	UINT16 *sub_cpu  = (UINT16 *) memory_region(machine, "sub");

	main_cpu[0x810/4] = 0xe0214e71;
	main_cpu[0x814/4] = 0x4e71203c;

	main_cpu[0x81c/4] = 0x4e714e71;

	main_cpu[0xa5c/4] = 0x4e713e3c;
	main_cpu[0xa74/4] = 0x4e713e3c;
	main_cpu[0xa8c/4] = 0x4e7141f9;

	// Sub CPU

	sub_cpu[0x4d4/2] = 0x5470;	// rte -> rts
}

GAME( 1994, gundamex, 0,        gundamex, gundamex, 0,        ROT0, "Banpresto",             "Mobile Suit Gundam EX Revue",                  0 )
GAME( 1995, grdians,  0,        grdians,  grdians,  0,        ROT0, "Banpresto",             "Guardians / Denjin Makai II",                  GAME_NO_COCKTAIL | GAME_IMPERFECT_GRAPHICS )	// Displays (c) Winky Soft at game's end.
GAME( 1996, mj4simai, 0,        mj4simai, mj4simai, 0,        ROT0, "Maboroshi Ware",        "Wakakusamonogatari Mahjong Yonshimai (Japan)", GAME_NO_COCKTAIL )
GAME( 1996, myangel,  0,        myangel,  myangel,  0,        ROT0, "Namco",                 "Kosodate Quiz My Angel (Japan)",               GAME_NO_COCKTAIL | GAME_IMPERFECT_GRAPHICS )
GAME( 1997, myangel2, 0,        myangel2, myangel2, 0,        ROT0, "Namco",                 "Kosodate Quiz My Angel 2 (Japan)",             GAME_NO_COCKTAIL | GAME_IMPERFECT_GRAPHICS )
GAME( 1999, pzlbowl,  0,        pzlbowl,  pzlbowl,  0,        ROT0, "Nihon System / Moss",   "Puzzle De Bowling (Japan)",                    GAME_NO_COCKTAIL )
GAME( 2000, penbros,  0,        penbros,  penbros,  0,        ROT0, "Subsino",               "Penguin Brothers (Japan)",                     GAME_NO_COCKTAIL )
GAME( 2000, deerhunt, 0,        samshoot, deerhunt, 0,        ROT0, "Sammy USA Corporation", "Deer Hunting USA V4.3",                        GAME_NO_COCKTAIL | GAME_IMPERFECT_GRAPHICS )
GAME( 2000, deerhunta,deerhunt, samshoot, deerhunt, 0,        ROT0, "Sammy USA Corporation", "Deer Hunting USA V4.2",                        GAME_NO_COCKTAIL | GAME_IMPERFECT_GRAPHICS )
GAME( 2000, deerhuntb,deerhunt, samshoot, deerhunt, 0,        ROT0, "Sammy USA Corporation", "Deer Hunting USA V4.0",                        GAME_NO_COCKTAIL | GAME_IMPERFECT_GRAPHICS )
GAME( 2000, deerhuntc,deerhunt, samshoot, deerhunt, 0,        ROT0, "Sammy USA Corporation", "Deer Hunting USA V2",                          GAME_NO_COCKTAIL | GAME_IMPERFECT_GRAPHICS )
GAME( 2001, turkhunt, 0,        samshoot, turkhunt, 0,        ROT0, "Sammy USA Corporation", "Turkey Hunting USA V1.0",                      GAME_NO_COCKTAIL | GAME_IMPERFECT_GRAPHICS )
GAME( 2001, wschamp,  0,        samshoot, wschamp,  0,        ROT0, "Sammy USA Corporation", "Wing Shooting Championship V2.00",             GAME_NO_COCKTAIL | GAME_IMPERFECT_GRAPHICS )
GAME( 2001, wschampa, wschamp,  samshoot, wschamp,  0,        ROT0, "Sammy USA Corporation", "Wing Shooting Championship V1.01",             GAME_NO_COCKTAIL | GAME_IMPERFECT_GRAPHICS )
GAME( 2002, trophyh,  0,        samshoot, trophyh,  0,        ROT0, "Sammy USA Corporation", "Trophy Hunting - Bear & Moose V1.0",           GAME_NO_COCKTAIL | GAME_IMPERFECT_GRAPHICS )
GAME( 2001, funcube2, 0,        funcube,  funcube,  funcube2, ROT0, "Namco",                 "Funcube 2 (v1.1)",                             GAME_NO_SOUND )
GAME( 2001, funcube4, 0,        funcube,  funcube,  funcube4, ROT0, "Namco",                 "Funcube 4 (v1.0)",                             GAME_NO_SOUND )
