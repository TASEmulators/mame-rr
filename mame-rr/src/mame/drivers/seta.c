/***************************************************************************

                            -= Seta Hardware =-

                    driver by   Luca Elia (l.elia@tin.it)


CPU    :    68000 + [65C02] (only in the earlier games)
Custom :    X1-001A  X1-002A (SDIP64)   Sprites
            X1-001
            X1-002
            X1-003
            X1-004           (SDIP52)   Inputs
            X1-005   X0-005
            X1-006   X0-006
            X1-007           (SDIP42)   Vide DAC
            X1-010           (QFP80)    Sound: 16 Bit PCM
            X1-011   X1-012  (QFP100)   Tilemaps
            X1-014                      Sprites?

-------------------------------------------------------------------------------
Ordered by Board        Year + Game                             Licensed To
-------------------------------------------------------------------------------
P0-029-A (M6100287A)    88 Thundercade / Twin Formation (1)     Taito
?        (M6100326A)    88 Twin Eagle                           Taito
P0-045-A (M6100429A)    89 DownTown                             Taito / RomStar
?        (M6100430A)    89 U.S. Classic(2)                      Taito / RomStar
?                       88 Caliber 50                           Taito / RomStar
?                       89 Arbalester                           Taito / RomStar
PO-047A                 ?? Seta Roulette                        Seta / Visco
P1-036-A + P0-045-A +
P1-049-A                89 Meta Fox                             Taito / RomStar
P0-053-1                89 Castle of Dragon/Dragon Unit         Taito / RomStar / Athena
P0-053-A                91 Strike Gunner S.T.G                  Athena / Tecmo
P0-053-A                92 Quiz Kokology                        Tecmo
P0-055-B                89 Wit's                                Athena
P0-055-D                90 Thunder & Lightning                  Romstar / Visco
Promat PCB              94 Wiggie Waggie(5)                     --
Promat PCB              94 Super Bar(5)                         --
P0-058-C                98 Internation Toote (6)                Coinmaster
P0-063-A                91 Rezon                                Allumer
P0-068-B (M6100723A)    92 Block Carnival                       Visco
P0-072-2 (prototype)    92 Blandia (prototype)                  Allumer
P0-077-A (BP922)        92 Ultraman Club                        Banpresto
PO-078-A                92 Blandia                              Allumer
P0-079-A                92 Zing Zing Zip                        Allumer / Tecmo
P0-079-A                94 Eight Forces                         Tecmo
PO-080-A (BP923)        92 SD Gundam Neo Battling (3)           Banpresto
?                       93 Athena no Hatena?                    Athena
?                       93 J.J.Squawkers                        Athena / Able
?        (93111A)       93 War Of Aero                          Yang Cheng
P0-081-A (BP933KA)      93 Mobile Suit Gundam                   Banpresto
PO-083-A (BP931)        93 Ultra Toukon Densetsu                Banpresto / Tsuburaya Prod.
PO-092-A                93 Daioh                                Athena
PO-096-A (BP934KA)      93 Kamen Rider                          Banpresto
P0-097-A                93 Oishii Puzzle ..                     Sunsoft + Atlus
bootleg                 9? Triple Fun (4)                       bootleg (Comad?)
P0-100-A                93 Quiz Kokology 2                      Tecmo
P0-101-1                94 Pro Mahjong Kiwame                   Athena
PO-102-A                93 Mad Shark                            Allumer
PO-107-A (prototype?)   94 Orbs (prototype?)                    American Sammy
PO-107-A                93 Kero Kero Keroppi no Issyoni Asobou  Sammy Industries  [added Chack'n, Hau]
P0-114-A (SKB-001)      94 Krazy Bowl                           American Sammy
P0-117-A (DH-01)        95 Extreme Downhill                     Sammy Japan
P0-117-A?               95 Sokonuke Taisen Game                 Sammy Industries
P0-120-A (BP954KA)      95 Gundhara                             Banpresto
PO-122-A (SZR-001)      95 Zombie Raid                          American Sammy
?                       96 Crazy Fight                          Subsino
-------------------------------------------------------------------------------
(1) YM2203 + YM3812 instead of X1-010
(2) wrong colors
(3) Same board as "Ultraman Club" despite the different PCB number
(4) this is a bootleg of Oishii Puzzle, in english, is there an official
    version?  the sound system has been replaced with an OKI M6295
    hardware is definitely bootleg. standard simple layout board with no
    custom chips and no manufacturer on the pcb.
(5) The game code is based on Thunder and Lightning but the PCB is custom
    there are a few gfx emulation bugs (flipping of some border tiles and
    sprites not leaving the screen correctly) its possible the custom hw
    doesn't behave *exactly* the same as the original seta hw
(6) To enter test mode press 9 (open door), then F2 (turn function key), then E (bet 3-4).

Notes:
- The NEC D4701 used by Caliber 50 is a mouse interface IC (uPD4701c).
  Of course it's used to control the spinner. DownTown probably has it as well.

DIP Locations verified from manuals for:
- Zing Zing Zip
- Extreme Downhill

TODO:
- I think the best way to correctly align tilemaps and sprites and account for
  both flipping and different visible areas is to have a table with per game
  vertical and horizontal offsets for sprites, tilemaps and possibly the "floating
  tilemaps" (made of sprites) for both the flipped and normal screen cases.
  Current issues: metafox test grid not aligned when screen flipped, madshark & utoukond
  ("floating tilemaps" sprites when flipped)
  krzybowl not aligned vertically when screen flipped
  zombraid not aligned when flipped vertically or horizantally

- bad sound in sokonuke?
- in msgunda1, colors for the score display screw up after the second animation
  in attract mode. The end of the animation also has garbled sprites.
  Note that the animation is not present in msgundam.
- Some games: battery backed portion of RAM (e.g. downtown, kiwame, zombraid)
- the zombraid crosshair hack can go if the nvram regions are figured out.
- Some games: programmable timer that generates IRQ. See e.g. gundhara:
  lev 4 is triggerd by writes at d00000-6 and drives the sound.
  See also msgundam.

- tndrcade: lots of flickering sprites
- drgnunit sprite/bg unaligned when screen flipped (check I/O test in service mode)
- extdwnhl has some wrong colored tiles in one of the attract mode images and in
  later tracks.
- oisipuzl doesn't support screen flip? tilemap flipping is also kludged in the video driver.
- eightfrc has alignment problems both flipped and not
- flip screen and mirror support not working correctly in zombraid
- gundhara visible area might be smaller (zombraid uses the same MachineDriver, and
  the current area is right for it)
- crazyfgt: emulate protection & tickets, fix graphics glitches, find correct clocks,
  level 2 interrupt should probably be triggered by the 3812 but sound tends to die that way.
- jjsquawk: Nuts don't fall from the trees shaken by white animal.
  Player's shot sound is missing (not requested to X1-010?).
  Many sounds are wrong since MAME 0.62.
  i.e.
  all scene: when you beat enemies or yellow walking eggs
  stage 1: weasels throw eggs, white animals (shaking trees) are damaged, rabbit jump
  stage 2: when BOX-MEN gets angry

***************************************************************************/

/***************************************************************************

Thundercade / Twin Formation
Taito America Corp./Romstar USA/Seta, 1987

PCB Layout
---------
M6100287A
P0-029-A
|------------------------------------------------------------------|
|M    L    K     J     H     G     F     E     D     C     B     A1|
|A UA0-4  UA0-3  4364  UA0-2  UA0-1  4364           16MHz          |
|                                         X1-001           X1-002 2|
|  68000                                                           |
|                                                                  |
|                                                                 3|
|                                                                  |
|                                                                  |
|                       4364 4364   UA0-9  UA0-8  UA0-7  UA0-6    4|
|                                                                  |
|                                                                  |
|                                                                 5|
|                            4584                                  |
|                                                                  |
|                        RESET_SW   UA0-13 UA0-12 UA0-11 UA0-10   6|
|                        4050  TL7705                              |
|                                                                  |
|                                                                 7|
|      X0-006                                                      |
|                                                                  |
|                            SW1                                  8|
| UA10-5 2016 YM3812 YM2203                                        |
|            YM3014          SW2                   X1-006         9|
|VOL   4558  YM3014 M54528            X1-004                       |
|                                                                10|
|                         X2-003 X2-003 X2-003     X1-003          |
|                                                                11|
|            |--|              JAMMA               |--|            |
|------------|  |----------------------------------|  |------------|
Notes:
      68000  - clock 8.000MHz [16/2]
      YM3812 - clock 4.000MHz [16/4]
      YM2203 - clock 4.000MHz [16/4]
      4364   - 8kx8 SRAM
      2016   - 2kx8 SRAM
      VSync  - 59.1845Hz
      HSync  - 15.21kHz

      Custom Chips -
                    X1-001 (SDIP64)
                    X1-002 (SDIP64)
                    X1-006 (SDIP64)
                    X0-006 (SDIP64), also marked 'RP5A10-0001'. This is a 65C02 in disguise,
                                     possibly with additional ROM or logic acting as a protection chip.
                                     clocks - pin1 16MHz, pin2 2MHz, pin3 59.1845Hz [VSYNC),
                                     pin63 2MHz, pin62 2MHz
                    X1-003 (SDIP42)
                    X1-004 (SDIP52)
                    X2-003 (DIP16) - Resistor network IC for inputs

      ROMs -
             UA0-6 to AU0-13 DIP42 maskROM, read as MX27C2100 (2 Meg)
             UA0-2 DIP32 27C1000 EPROM
             All other ROMs DIP32 read as 27C1000
             All ROMs have IC locations, but the PCB also has IC locations so components
             can be referenced with IC# or location. See above diagram for info.

***************************************************************************/

/***************************************************************************

                                Twin Eagle

M6100326A   Taito (Seta)

ua2-4              68000
ua2-3
ua2-6
ua2-5
ua2-8
ua2-10
ua2-7               ua2-1
ua2-9
ua2-12
ua2-11              ua2-2

***************************************************************************/

/***************************************************************************

                                U.S. Classic

M6100430A (Taito 1989)

       u7 119  u6 118   u5 117   u4 116
                                         68000-8
u13  120                                 000
u19  121                                 001
u21  122                                 002
u29  123                                 003
u33  124
u40  125
u44  126
u51  127
u58  128
u60  129                                 65c02
u68  130
u75  131                                 u61 004

                                         u83 132

***************************************************************************/
/***************************************************************************

                                Caliber 50

CPU:   TMP 68000N-8, 65C02
Other: NEC D4701

UH-001-006        SW2  SW1
UH-001-007
UH-001-008                    8464         68000-8
UH-001-009  X1-002A X1-001A   8464         Uh-002-001=T01
UH-001-010                    8464            51832
UH-001-011                    8464            51832
                                           UH-001-002
UH-001-012            X1-012               UH-001-003
UH-001-013                               UH-002-004-T02
                      X1-011               5116-10
                                           BAT
                         16MHz
             X1-010   65C02      X1-006
                      UH-001-005 X1-007
                      4701       X1-004

***************************************************************************/
/***************************************************************************

                                    Meta Fox

(Seta 1990)

P0-045A

P1-006-163                    8464   68000-8
P1-007-164    X1-002A X1-001A 8464
P1-008-165                    8464
P1-009-166                    8464     256K-12
                                       256K-12

                 X1-012
                 X1-011


   2063    X1-010     X1-006     X0-006
                      X1-007
                      X1-004     X1-004

----------------------
P1-036-A

UP-001-010
UP-001-011
UP-001-012
UP-001-013


UP-001-014
UP-001-015

-----------------------
P1-049-A

              UP-001-001
              UP-001-002
              P1-003-161
              P1-004-162


              UP-001-005
              x

***************************************************************************/


/***************************************************************************

                                Dragon Unit
                     [Prototype of "Castle Of Dragon"]

PCB:    P0-053-1
CPU:    68000-8
Sound:  X1-010
OSC:    16.0000MHz

Chips:  X1-001A, X1-002A, X1-004, X1-006, X1-007, X1-010, X1-011, X1-012

***************************************************************************/
/***************************************************************************

                                    Wit's

(c)1989 Athena (distributed by Visco)
P0-055B (board is made by Seta)

CPU  : TMP68000N-8
Sound: X1-010
OSC  : 16.000MHz

ROMs:
UN001001.U1 - Main program (27256)
UN001002.U4 - Main program (27256)

UN001003.10A - Samples (28pin mask)
UN001004.12A /

UN001005.2L - Graphics (28pin mask)
UN001006.4L |
UN001007.5L |
UN001008.7L /

Custom chips:   X1-001A     X1-002A
                X1-004 (x2)
                X1-006
                X1-007
                X1-010

***************************************************************************/
/***************************************************************************

                            Thunder & Lightning

Location      Device      File ID      Checksum
-----------------------------------------------
U1  1A        27C256        M4           C18C   [ MAIN PROG ] [ EVEN ]
U4  3A        27C256        M5           12E1   [ MAIN PROG ] [ ODD  ]
U29 10A      23C4001        R27          37F2   [   HIGH    ]
U39 12A      23C4001        R28          0070   [   LOW     ]
U6  2K       23C1000        T14          1F7D   [   C40     ]
U9  4K       23C1000        T15          7A15   [   C30     ]
U14 5K       23C1000        T16          BFFD   [   C20     ]
U20 7K       23C1000        T17          7AE7   [   C10     ]

PCB: PO055D

CPU: 68000 8MHz

Custom: X1-001A     X1-002A
        X1-004
        X1-006
        X1-007
        X1-010

***************************************************************************/



/***************************************************************************

                                Athena no Hatena?

CPU  : 68000-16
Sound: X1-010
OSC  : 16.0000MHz

ROMs:
fs001001.evn - Main programs (27c4001)
fs001002.odd /

fs001004.pcm - Samples (8M mask - read as 27c800)
fs001003.gfx - Graphics (16M mask - read as 27c160)

Chips:  X1-001A X1-002A
        X1-004
        X1-006
        X1-007
        X1-010

***************************************************************************/

/***************************************************************************

                                Blandia

Blandia by Allumer

This set is coming from an original Blandia PCB ref : PO-078A

As usually, it use a lot of customs allumer chips !

***************************************************************************/
/***************************************************************************

                                Blandia (prototype)

PCB:    P0-072-2
CPU:    68000-16
Sound:  X1-010
OSC:    16.0000MHz

Chips:  X1-001A     X1-002A
        X1-004
        X1-007
        X1-010
        X1-011 x2   X1-012 x2

***************************************************************************/

/***************************************************************************

                    Block Carnival / Thunder & Lightning 2

P0-068B, M6100723A

CPU  : MC68000B8
Sound: X1-010
OSC  : 16.000MHz

ROMs:
u1.a1 - Main programs (27c010)
u4.a3 /

bl-chr-0.j3 - Graphics (4M mask)
bl-chr-1.l3 /

bl-snd-0.a13 - Sound (4M mask)

Custom chips:   X1-001A X1-002A
                X1-004
                X1-006
                X1-007
                X1-009
                X1-010

Other:
Lithium battery x1

***************************************************************************/




/***************************************************************************

                            Daioh

DAIOH
Alumer 1993, Sammy license
PO-092A


FG-001-003
FG-001-004  X1-002A X1-001A             FG-001-001
                                        FG-001-002
FG-001-005   X1-11 X1-12
FG-001-006   X1-11 X1-12
                                       68000-16
FG-001-007

   X1-10                           16MHz

                            X1-007  X1-004

***************************************************************************/


/***************************************************************************

                                Eight Forces

PO-079A (Same board as ZingZingZip)

CPU  : MC68000B16
Sound: X1-010
OSC  : 16.000MHz

ROMs:
uy2-u4.u3 - Main program (even)(27c2001)
uy2-u3.u4 - Main program (odd) (27c2001)

u63.bin - Sprites (HN62434, read as 27c4200)
u64.bin /

u69.bin - Samples (HN62318, read as 27c8001)
u70.bin /

u66.bin - Layer 1 (HN62418, read as 27c800)
u68.bin - Layer 2 (HN62418, read as 27c800)

PALs (not dumped):
uy-012.206 (PAL16L8A)
uy-013.14  (PAL16L8A)
uy-014.35  (PAL16L8A)
uy-015.36  (PALCE16V8)
uy-016.76  (PAL16L8A)
uy-017.116 (PAL16L8A)

Custom:     X1-001A X1-002A
            X1-004
            X1-007
            X1-010
            X1-011 (x2)     X1-012 (x2)

***************************************************************************/

/***************************************************************************

                                Extreme Downhill

(c)1995 Sammy
DH-01
PO-117A (board is made by Seta/Allumer)

CPU  : MC68HC000B16
Sound: X1-010
OSC: 16.0000MHz (X1), 14.3180MHz (X2)

ROMs:
fw001002.201 - Main program (even) (Macronics 27c4000)
fw001001.200 - Main program (odd)  (Macronics 27c4000)

fw001005.205 - (32pin mask, read as 27c8001)
fw001007.026 /

fw001003.202 - (42pin mask, read as 27c160)
fw001004.206 |
fw001006.152 /

PALs (16L8ACN, not dumped):
FW-001
FW-002
FW-003
FW-005

Custom chips:   X1-001A     X1-002A
                X1-004
                X1-007
                X1-010
                X1-011 (x2) X1-012 (x2)

***************************************************************************/


/***************************************************************************

                                    GundHara

(C) 1995 Banpresto
Seta/Allumer Hardware

PCB: BP954KA
PCB: PO-120A
CPU: TMP68HC000N16 (68000, 64 pin DIP)
SND: ?
OSC: 16.000MHz
RAM: 6264 x 8, 62256 x 4
DIPS: 2 x 8 position
Other Chips:    PALs x 6 (not dumped)
                NEC 71054C
                X1-004
                X1-007
                X1-010
                X1-011 x2   X1-012 x2
                X1-001A     X1-002A

On PCB near JAMMA connector is a small push button to reset the PCB.

ROMS:
BPGH-001.102    27C040
BPGH-002.103    27C4000
BPGH-003.U3     27C4000
BPGH-004.U4     23C4000
BPGH-005.200    23C16000
BPGH-006.201    23C16000
BPGH-007.U63    23C16000
BPGH-008.U64    23C16000
BPGH-009.U65    27C4000
BPGH-010.U66    TC538200
BPGH-011.U67    TC538000
BPGH-012.U68    TC5316200
BPGH-013.U70    TC538000

***************************************************************************/

/***************************************************************************

Zombie Raid
Sammy, 1996

This is a gun shooting game using Seta/Allumer hardware.

PCB Layout
----------

SZR-001
PO-122A
------------------------------------------------------------------
        FY001012.12L*       FY001009.U67      FY001007.U65
   FY001011.13L*    FY001010.U68     FY001008.U66    FY001006.U200
   X1-010    6264

    CONN1          X1-011(x2)     X1-002A
                   X1-012(x2)     X1-001A

                                            3V_BATT
                                                    4464

X1-007   6264(x2)   6264(x2)    6264(x2)    6264(x2)

       16.000MHz                                 HM9253101(x2)

X1-004 DSW2        D71054C
       DSW1    TMP68HC000N-16  FY001004.U4     FY001001.U102
                                      FY001003.U3    FY001002.U103
------------------------------------------------------------------

Notes:
*     = These ROMs located on a small daughterboard. Main PCB locations used as filename extension.
CONN1 = 8 pin header for gun connection

***************************************************************************/


/***************************************************************************

                                J.J. Squawkers

68HC000N -16N

2)   Alumer  X1-012
2)   Alumer  X1-011
2)   Alumer  X1-014

X1-010
X1-007
X1-004
16.000MHz

NEC 71054C  ----???

***************************************************************************/

/***************************************************************************

                                Kamen Rider
Kamen Riderclub Battleracer
Banpresto, 1993

Runs on Seta/Allumer hardware

PCB No: BP934KA   PO-096A
CPU   : MC68HC000B16
OSC   : 16.000MHz
RAM   : LH5160D-10L (x9), CXK58257AP-10L (x2)
DIPSW : 8 position (x2)
CUSTOM: X1-010
        X1-007
        X1-004
        X1-011 (x2)
        X1-012 (x2)
        X1-002A
        X1-001A
OTHER : NEC71054C, some PALs

ROMs  :
        FJ001007.152    27c4096     near X1-011 & X1-010 (sound program?)
        FJ001008.26     8M Mask     connected to X1-010, near FJ001007
        FJ001003.25     27c4096     main program for 68k
        FJ001006.22     16M Mask    gfx
        FJ001005.21     16M Mask    gfx

***************************************************************************/

/***************************************************************************

                                Krazy Bowl

PCB:    SKB-001
        PO-114A

FV   FV                           2465
001  001                          2465           X1-005
004  003      X1-002A  X1-001A
                                       58257     FV
                                                 001
                                                 002 (even)
                                       58257
                  14.318MHz                      FV
                                                 001
FV 001 005                                       001 (odd)
FV 001 006
  2465                                      68HC000B16
                 NEC4701  NEC4701

X1-010           X1-006
                 X1-007      X1-004

***************************************************************************/


/***************************************************************************


                                    Mad Shark

Allumer, 1993
This game is a vertical shoot'em-up and runs on fairly standard Allumer hardware.

PCB Layout
----------

PO-102A
----------------------------------------------------
|     X1-010   FQ001007 FQ001006 FQ001005 FQ001004 |
|           LH5160                                 |
|                    X1-011  X1-011       X1-002A  |
|                                                  |
|J                   X1-012  X1-012       X1-001A  |
|A X1-007                                          |
|M   LH5160           LH5160 LH5160 LH5160         |
|M                                                 |
|A   LH5160           LH5160 LH5160 LH5160         |
|                                         FQ001002 |
|*           MC68HC000B16                          |
|  X1-004                                          |
|                                         FQ001001 |
|                        LH52250                   |
| DSW2(8) DSW1(8) 16MHz  LH52250    D71054         |
----------------------------------------------------

Notes:
      *: 4 jumper pads for region selection (hardwired)

***************************************************************************/

/***************************************************************************

                            Mobile Suit Gundam

Banpresto 1993
P0-081A
                               SW2  SW1

FA-001-008                          FA-001-001
FA-001-007    X1-002A X1-001A       FA-002-002
                              5160
                              5160
                                        71054
FA-001-006                    5160     62256
FA-001-005    X1-011  X1-012  5160     62256

FA-001-004    X1-011  X1-012  5160
5160                          5160

                                68000-16

                                         16MHz
  X1-010
                    X1-007   X1-004     X1-005

***************************************************************************/

/***************************************************************************

                            Oishii Puzzle Ha Irimasenka

PCB  : PO-097A
CPU  : 68000
Sound: X1-010
OSC  : 14.31818MHz

All ROMs are 23c4000

Custom chips:   X1-001A X1-002A
                X1-004
                X1-007
                X1-010
                X1-011 (x2) X1-012 (x2)

***************************************************************************/

/***************************************************************************

                            Triple Fun

Triple Fun
??, 19??


CPU   : TMP68HC000P-16 (68000)
SOUND : OKI M6295
DIPSW : 8 position (x2)
XTAL  : 16.000 MHz (8MHz written on PCB, located near OKI chip)
        14.31818MHz (near 68000)
RAM   : 62256 (x2), 6264 (x8), 2018 (x14)
PROMs : None
PALs  : PALCE16V8H (x13)
OTHER : TPC1020AFN-084C (84 pin PLCC)

ROMs  :

04.bin + 05.bin    Main Program
01.bin             Sound Program
02.bin + 03.bin    OKI Samples
06.bin to 11.bin   GFX



***************************************************************************/

/***************************************************************************

                            Pro Mahjong Kiwame

PCB  : PO-101-1 (the board is made by Allumer/Seta)
CPU  : TMP68301AF-16 (68000 core)
Sound: X1-010
OSC  : 20.0000MHz

ROMs:
fp001001.bin - Main program (27c2001, even)
fp001002.bin - Main program (27c2001, odd)
fp001003.bin - Graphics (23c4000)
fp001005.bin - Samples (27c4000, high)
fp001006.bin - Samples (27c4000, low)

Chips:  X1-001A
        X1-002A
        X1-004
        X1-006
        X1-007
        X1-010

- To initialize high scores, power-on holding start button in service mode

***************************************************************************/


/***************************************************************************

                                Quiz Kokology

(c)1992 Tecmo

PO-053A

CPU  : MC68000B8
Sound: X1-010
OSC  : 16.000MHz

Custom chips:   X1-001A X1-002A
                X1-004
                X1-006  X1-007
                X1-010
                X1-011  X1-012

***************************************************************************/


/***************************************************************************

                                Quiz Koko-logy 2

(c)1992 Tecmo

P0-100A

CPU  : MC68HC000B16
Sound: X1-010
OSC  : 16.000MHz

FN001001.106 - Main program (27C4096)
FN001003.107 / (40pin 2M mask)

FN001004.100 - OBJ chr. (42pin mask)
FN001005.104 - BG chr. (42pin mask)
FN001006.105 - Samples (32pin mask)

Custom chips:   X1-001A     X1-002A
                X1-004
                X1-006
                X1-007
                X1-010
                X1-011      X1-012

***************************************************************************/


/***************************************************************************

                                Rezon (Japan)

PCB     : PO-063A
CPU     : TOSHIBA TMP68HC000N-16
Sound   : X1-010
OSC     : 16.000MHz
Other   : Allumer
            X1-001A         X1-002A
            X1-004
            X1-007
            X1-011 x 2      X1-012 x 2

***************************************************************************/


/***************************************************************************

                            SD Gundam Neo Battling

Banpresto, 1992
This game runs on Seta/Allumer hardware

PCB Layout
----------

PO-080A
BP923
|----------------------------------------------
|DSW1  DSW2                   LH5168  62256   |
|                             LH5168  62256   |
|LH5168                                       |
|                                             |
|         BP923004                   BP923001 |
|BP923005 BP923003 X1-002A X1-001A   BP923002 |
|                                  16MHz      |
|                              TMP68HC000N-16 |
|X1-010                           PAL         |
|                                 PAL  X1-006 |
|                                             |
|                                             |
|                              X1-004  X1-007 |
|                                             |
|                                             |
|                                             |
|                                             |
|                 J A M M A                   |
-----------------------------------------------

Notes:
      68k clock: 16.000MHz
      VSync: 58Hz
      HSync: 15.22kHz


***************************************************************************/


/***************************************************************************

                            Sokonuke Taisen Game (Japan)

(c)1995 Sammy

CPU:    68HC000
Sound:  All PCM ?
OSC:    16MHz

***************************************************************************/


/***************************************************************************

                                Strike Gunner

(c)1991 Athena (distributed by Tecmo)

PO-053A

CPU  : TMP68000N-8
Sound: X1-010
OSC  : 16.000MHz

Custom chips:   X1-001A X1-002A
                X1-004
                X1-006  X1-007
                X1-010
                X1-011  X1-012

***************************************************************************/

/***************************************************************************

                                Ultraman Club

Banpresto, 1992
Board looks similar to Castle of Dragon PCB.

PCB No: PO-077A (Seta Number)
        BP922   (Banpresto Number)

CPU: MC68HC000B16
OSC: 16.000MHz
DIP SW x 2 (8 position)

RAM: Sharp LH5160D-10L x 3, Hitachi S256KLP-12 x 2
PALs (2 x PAL16L8, not dumped)
SETA Chips: X1-010
            X1-004
            X1-007
            X1-006
            X1-002A
            X1-001A

Controls are 8 way Joystick and 2 buttons.

ROMs:

UW001006.U48      27C010                                               \  Main Program
UW001007.U49      27C010                                               /

BP-U-001.U1       4M mask (40 pin, 512k x 8), read as MX27C4100        \  GFX
BP-U-002.U2       4M mask (40 pin, 512k x 8), read as MX27C4100        /

BP-U-003.U13      8M mask (32 pin, 1M x 8),   read as MX27C8000           Sound


***************************************************************************/

/***************************************************************************
Ultra Toukon Densetsu
Banpresto, 1993

This game runs on fairly standard Allumer hardware.

PCB Layout
----------

PO-083A
BP931
----------------------------------------------------
|     X1-010  93UTA08  93UTA06 93UTA04  93UTA02    |
|                93UTA07 93UTA05  93UTA03  93UTA01 |
|  YM3438   LH5116                                 |
|  LH5116            X1-011  X1-011       X1-002A  |
|  Z80 93UTA009                                    |
|J                   X1-012  X1-012       X1-001A  |
|A X1-007                                          |
|M   LH5116           LH5160 LH5160 LH5160         |
|M                                                 |
|A   LH5116           LH5160 LH5160 LH5160         |
|                                                  |
|*      16MHz                                      |
|  X1-004                                   62256  |
|                                           62256  |
| DSW1(8)               93UTA011  93UTA010         |
| DSW2(8)   68HC000N-16                            |
----------------------------------------------------

Notes:
      *: 4 jumper pads for region selection (hardwired)
      Z80 clock = 4.000MHz
      VSync: 60Hz
      HSync: 15.21kHz


1.048.576 93uta03.63
1.048.576 93uta04.64
1.048.576 93uta05.66
1.048.576 93uta06.67
1.048.576 93uta07.68
1.048.576 93uta08.69

***************************************************************************/

/***************************************************************************

                               War of Aero
                            Project M E I O U

93111A  YANG CHENG

CPU   : TOSHIBA TMP68HC000N-16
Sound : Allumer X1-010
OSC   : 16.000000MHz
Other : Allumer
            X1-001A  X1-002A
            X1-004
            X1-007
            X1-011 x 2
            X1-012 x 2
        NEC
            C324C
            D71054C

***************************************************************************/



/***************************************************************************

                                Zing Zing Zip

P0-079A

UY-001-005   X1-002A   X1-001A   5168-10      256k-12
UY-001-006                       5168-10      UY-001-001
UY-001-007                                    UY-001-002
UY-001-008   X1-011 X1-012                    58257-12
                                 5168-10
UY-001-010   X1-011 X1-012       5168-10
UY-001-017
UY-001-018
                                 5168-10
X1-010                           5168-10       68000-16


                           8464-80
                           8464-80       16MHz


                             X1-007    X1-004

***************************************************************************


Pairs Love
Allumer, 199x

PCB Layout
----------

PO-068B
|-----------------------------------------|
|             X1-007  X1-006   UT2-001-005|
|                                         |
|     4050                     UT2-001-004|
|                                         |
|                                         |
|                             X1-002A     |
|                                         |
|J                                        |
|A   X1-004                               |
|M                            X1-001A     |
|M           DSW1                         |
|A                                        |
|            DSW2                  6264   |
|                                  6264   |
|    X1-009                               |
|                      62256              |
|                                         |
|                    68000                |
|    UT2-001-003       62256  UT2-001-002 |
|                   6264                  |
|    X1-010  16MHz            UT2-001-001 |
|-----------------------------------------|
Notes:
      68000 clock: 8.000MHz
      VSync: 60Hz


***************************************************************************

Rezon (Taito License)
Allumer / Taito, 1992

This game runs on fairly standard Allumer hardware.

PCB Layout
----------

P0-063A (Allumer code printed on the PCB)
M6100627A REZON (Taito sticker)
|-----------------------------------------------------------|
|  VOL   3404   6264 US001009     US001007      US001005    |
| MB3730                    US001008      US001006          |
|                                                           |
|                                                           |
|                                                           |
|  PAL4           X1-010       X1-011       X1-011          |
|                                                           |
|                                                           |
|J                                                          |
|A                             X1-012       X1-012          |
|M                                                          |
|M  X1-007                                                  |
|A                                                          |
|        6116  6116                           PAL2   PAL3   |
|                                                           |
|                         62256       62256                 |
|  16MHZ                  62256       62256     6264  6264  |
|         DSW2(8)                                           |
|  X1-004                 PAL1                              |
|         DSW1(8)  |------------|US001004      REZON_1_P    |
|                  |   68000    |   US001003      REZON_0_P |
|  RESET_SW        |------------|       62256          62256|
|-----------------------------------------------------------|
Notes:
      68000 clock  - 16.000MHz
      X1-010 clocks - pin1 16.000MHz, pin2 8.000MHz, pin79 4.000MHz, pin80 2.000MHz
      VSync - 57.5Hz
      PAL1  - PAL16L8 labelled 'US-010'
      PAL2  - PAL16L8 labelled 'US-011'
      PAL3  - PAL16L8 labelled 'US-012'
      PAL4  - PAL16L8 labelled 'US-013'
      62256 - 32K x8 SRAM
      6264  - 8K x8 SRAM
      6116  - 2K x8 SRAM

      Custom IC's -
                    X1-001A (SDIP64)    \ Sprite Generators
                    X1-002A (SDIP64)    /
                    X1-004  (SDIP52)      Input Related Functions (connected to joystick/input controls)
                    X1-007  (SDIP42)      Video DAC? (connected to RGB output)
                    X1-010  (QFP80)       Sound Chip, 16Bit PCM
                    X1-011  (x2, QFP100)\ Tilemap Generators
                    X1-012  (x2, QFP100)/

      ROMs -
            Filename         Type               Use
            ---------------------------------------------------
            REZON_0_P.U3     27C1000 (DIP32)    \
            REZON_1_P.U4     27C1000 (DIP32)    | 68000 Program
            US001003.U102    27C1000 (DIP32)    |
            US001004.U103    27C1000 (DIP32)    /

            US001005.U63     4M MaskROM (DIP42) \ Sprites
            US001006.U64     4M MaskROM (DIP42) /

            US001007.U66     4M MaskROM (DIP42) \ Tiles
            US001008.U68     4M MaskROM (DIP42) /

            US001009.U70     4M MaskROM (DIP32)   PCM Samples

***************************************************************************/

/***************************************************************************

Crazy Fight

x1-11
x1-11
x1-12
x1-12
x1-001a
x1-002a
x1-007
Lattlice PLSI 1032 FPGA
oki 6295
ym3812
68000

***************************************************************************/

/***************************************************************************
                                Internation Toote

Main PCB (P0-058C):

    TOSHIBA TMP68HC000N-16
    X1-010
    X1-001A
    X1-002A
    X1-004 (x2)
    X1-007
    X1-011
    X1-012
    HD63B50P

Horse Race I/O Expansion (PCB-HRE-000):

    MC68B50CP (x2)
    EF68B21P  (x2)

Note: on screen copyright is (c)1998 Coinmaster.
      The I/O board has      (c)1993 Coinmaster.

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "cpu/m6502/m6502.h"
#include "deprecat.h"
#include "includes/seta.h"
#include "machine/6821pia.h"
#include "machine/6850acia.h"
#include "machine/msm6242.h"
#include "sound/2203intf.h"
#include "sound/2612intf.h"
#include "sound/3812intf.h"
#include "sound/okim6295.h"
#include "sound/x1_010.h"

/* Variables and functions only used here */

static UINT8 port_select;     /* for muxed controls in 'usclssic' */

static UINT8 *sharedram;


#if __uPD71054_TIMER

#define	USED_TIMER_NUM	1
/*------------------------------
    timer(uPD71054) struct
------------------------------*/
static struct st_chip {
	emu_timer *timer[3];			// Timer
	UINT16	max[3];				// Max counter
	UINT16	write_select;		// Max counter write select
	UINT8	reg[4];				//
} uPD71054;

/*------------------------------
    uppdate timer
------------------------------*/
static void uPD71054_update_timer( running_machine *machine, running_device *cpu, int no )
{
	UINT16 max = uPD71054.max[no]&0xffff;

	if( max != 0 ) {
		attotime period = attotime_mul(ATTOTIME_IN_HZ(cputag_get_clock(machine, "maincpu")), 16 * max);
		timer_adjust_oneshot( uPD71054.timer[no], period, no );
	} else {
		timer_adjust_oneshot( uPD71054.timer[no], attotime_never, no);
		logerror( "CPU #0 PC %06X: uPD71054 error, timer %d duration is 0\n",
				(cpu != NULL) ? cpu_get_pc(cpu) : -1, no );
	}
}



/*------------------------------
    callback
------------------------------*/
static TIMER_CALLBACK( uPD71054_timer_callback )
{
	cputag_set_input_line(machine, "maincpu", 4, HOLD_LINE );
	uPD71054_update_timer( machine, NULL, param );
}



/*------------------------------
    initialize
------------------------------*/
static void uPD71054_timer_init( running_machine *machine )
{
	int no;

	uPD71054.write_select = 0;

	for( no = 0; no < USED_TIMER_NUM; no++ ) {
		uPD71054.max[no] = 0xffff;
	}
	for( no = 0; no < USED_TIMER_NUM; no++ ) {
		uPD71054.timer[no] = timer_alloc( machine, uPD71054_timer_callback , NULL);
	}
}



/*------------------------------
    timer write handler
------------------------------*/
static WRITE16_HANDLER( timer_regs_w )
{
	data &= 0xff;

	uPD71054.reg[offset] = data;

	switch( offset ) {
	  case 0x0000:
	  case 0x0001:
	  case 0x0002:
		if( uPD71054.write_select == 0 ) {
			uPD71054.max[offset] = (uPD71054.max[offset]&0xff00)+data;
			if( ((uPD71054.reg[3]>>4)&3) == 3 ) {
				uPD71054.write_select = 1;
			}
		} else {
			uPD71054.max[offset] = (uPD71054.max[offset]&0x00ff)+(data<<8);
		}
		if( uPD71054.max[offset] != 0 ) {
			uPD71054_update_timer( space->machine, space->cpu, offset );
		}
		break;
	  case 0x0003:
		switch( (data>>4)&3 ) {
		  case 2: uPD71054.write_select = 1; break;
		  case 1:
		  case 3: uPD71054.write_select = 0; break;
		}
		break;
	}
}
#endif	// __uPD71054_TIMER




/***************************************************************************


                                    Sound


***************************************************************************/

static const x1_010_interface seta_sound_intf =
{
	0x0000,		/* address */
};
static const x1_010_interface seta_sound_intf2 =
{
	0x1000,		/* address */
};

static void utoukond_ym3438_interrupt(running_device *device, int linestate)
{
	cputag_set_input_line(device->machine, "audiocpu", INPUT_LINE_NMI, linestate);
}

static const ym3438_interface utoukond_ym3438_intf =
{
	utoukond_ym3438_interrupt	// IRQ handler
};

/***************************************************************************


                                Common Routines


***************************************************************************/


/*

 Shared RAM:

 The 65c02 sees a linear array of bytes that is mapped, for the 68000,
 to a linear array of words whose low order bytes hold the data

*/

static READ16_HANDLER( sharedram_68000_r )
{
	return ((UINT16) sharedram[offset]) & 0xff;
}

static WRITE16_HANDLER( sharedram_68000_w )
{
	if (ACCESSING_BITS_0_7)	sharedram[offset] = data & 0xff;
}




/*

 Sub CPU Control

*/

static WRITE16_HANDLER( sub_ctrl_w )
{
	static int old_data = 0;
	switch(offset)
	{
		case 0/2:	// bit 0: reset sub cpu?
			if (ACCESSING_BITS_0_7)
			{
				if ( !(old_data & 1) && (data & 1) )
					cputag_set_input_line(space->machine, "sub", INPUT_LINE_RESET, PULSE_LINE);
				old_data = data;
			}
			break;

		case 2/2:	// ?
			break;

		case 4/2:	// not sure
			if (ACCESSING_BITS_0_7)	soundlatch_w(space, 0, data & 0xff);
			break;

		case 6/2:	// not sure
			if (ACCESSING_BITS_0_7)	soundlatch2_w(space, 0, data & 0xff);
			break;
	}

}


/* DSW reading for 16 bit CPUs */
static READ16_HANDLER( seta_dsw_r )
{
	UINT16 dsw = input_port_read(space->machine, "DSW");
	if (offset == 0)	return (dsw >> 8) & 0xff;
	else				return (dsw >> 0) & 0xff;
}


/* DSW reading for 8 bit CPUs */

static READ8_DEVICE_HANDLER( dsw1_r )
{
	return (input_port_read(device->machine, "DSW") >> 8) & 0xff;
}

static READ8_DEVICE_HANDLER( dsw2_r )
{
	return (input_port_read(device->machine, "DSW") >> 0) & 0xff;
}


/*

 Sprites Buffering

*/
static VIDEO_EOF( seta_buffer_sprites )
{
	UINT16 *spriteram16 = machine->generic.spriteram.u16;
	int ctrl2	=	spriteram16[ 0x602/2 ];
	if (~ctrl2 & 0x20)
	{
		if (ctrl2 & 0x40)
			memcpy(&machine->generic.spriteram2.u16[0x0000/2],&machine->generic.spriteram2.u16[0x2000/2],0x2000/2);
		else
			memcpy(&machine->generic.spriteram2.u16[0x2000/2],&machine->generic.spriteram2.u16[0x0000/2],0x2000/2);
	}
}


/***************************************************************************


                                    Main CPU

(for debugging it is useful to be able to peek at some memory regions that
 the game writes to but never reads from. I marked this regions with an empty
 comment to distinguish them, since there's always the possibility that some
 games actually read from this kind of regions, expecting some hardware
 register's value there, instead of the data they wrote)

***************************************************************************/


/***************************************************************************
                                Thundercade
***************************************************************************/

/* Mirror RAM seems necessary since the e00000-e03fff area is not cleared
   on startup. Level 2 int uses $e0000a as a counter that controls when
   to write a value to the sub cpu, and when to read the result back.
   If the check fails "error x0-006" is displayed. Hence if the counter
   is not cleared at startup the game could check for the result before
   writing to sharedram! */


static ADDRESS_MAP_START( tndrcade_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM								// ROM
	AM_RANGE(0x200000, 0x200001) AM_WRITENOP						// ? 0
	AM_RANGE(0x280000, 0x280001) AM_WRITENOP						// ? 0 / 1 (sub cpu related?)
	AM_RANGE(0x300000, 0x300001) AM_WRITENOP						// ? 0 / 1
	AM_RANGE(0x380000, 0x3803ff) AM_RAM AM_BASE_GENERIC(paletteram) AM_SIZE(&seta_paletteram_size) // Palette
/**/AM_RANGE(0x400000, 0x400001) AM_WRITENOP						// ? $4000
/**/AM_RANGE(0x600000, 0x600607) AM_RAM	AM_BASE_GENERIC(spriteram)		// Sprites Y
	AM_RANGE(0x800000, 0x800007) AM_WRITE(sub_ctrl_w)				// Sub CPU Control?
	AM_RANGE(0xa00000, 0xa00fff) AM_READWRITE(sharedram_68000_r,sharedram_68000_w)	// Shared RAM
	AM_RANGE(0xc00000, 0xc03fff) AM_RAM	AM_BASE_GENERIC(spriteram2)		// Sprites Code + X + Attr
	AM_RANGE(0xe00000, 0xe03fff) AM_RAM AM_SHARE("share1")					// RAM (Mirrored?)
	AM_RANGE(0xffc000, 0xffffff) AM_RAM AM_SHARE("share1")					// RAM (Mirrored?)
ADDRESS_MAP_END


/***************************************************************************
                Twin Eagle, DownTown, Arbalester, Meta Fox
        (with slight variations, and Meta Fox protection hooked in)
***************************************************************************/

static ADDRESS_MAP_START( downtown_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x09ffff) AM_ROM								// ROM
	AM_RANGE(0x100000, 0x103fff) AM_DEVREADWRITE("x1snd", seta_sound_word_r,seta_sound_word_w)	// Sound
	AM_RANGE(0x200000, 0x200001) AM_NOP								// watchdog? (twineagl)
	AM_RANGE(0x300000, 0x300001) AM_WRITENOP						// IRQ enable/acknowledge?
	AM_RANGE(0x400000, 0x400007) AM_WRITE(twineagl_tilebank_w)		// special tile banking to animate water in twineagl
	AM_RANGE(0x500000, 0x500001) AM_WRITENOP						// ?
	AM_RANGE(0x600000, 0x600003) AM_READ(seta_dsw_r)				// DSW
	AM_RANGE(0x700000, 0x7003ff) AM_RAM AM_BASE_GENERIC(paletteram) AM_SIZE(&seta_paletteram_size)	// Palette
	AM_RANGE(0x800000, 0x800005) AM_WRITEONLY AM_BASE(&seta_vctrl_0)// VRAM Ctrl
	AM_RANGE(0x900000, 0x903fff) AM_RAM_WRITE(seta_vram_0_w) AM_BASE(&seta_vram_0)	// VRAM
	AM_RANGE(0xa00000, 0xa00007) AM_WRITE(sub_ctrl_w)				// Sub CPU Control?
	AM_RANGE(0xb00000, 0xb00fff) AM_READWRITE(sharedram_68000_r,sharedram_68000_w)	// Shared RAM
	AM_RANGE(0xc00000, 0xc00001) AM_WRITENOP						// ? $4000
	AM_RANGE(0xd00000, 0xd00607) AM_RAM AM_BASE_GENERIC(spriteram)		// Sprites Y
	AM_RANGE(0xe00000, 0xe03fff) AM_RAM AM_BASE_GENERIC(spriteram2)		// Sprites Code + X + Attr
	AM_RANGE(0xf00000, 0xffffff) AM_RAM								// RAM
ADDRESS_MAP_END


/***************************************************************************
                                Caliber 50
***************************************************************************/

static READ16_HANDLER ( calibr50_ip_r )
{
	int dir1 = input_port_read(space->machine, "ROT1");	// analog port
	int dir2 = input_port_read(space->machine, "ROT2");	// analog port

	switch (offset)
	{
		case 0x00/2:	return input_port_read(space->machine, "P1");	// p1
		case 0x02/2:	return input_port_read(space->machine, "P2");	// p2

		case 0x08/2:	return input_port_read(space->machine, "COINS");	// Coins

		case 0x10/2:	return (dir1 & 0xff);		// lower 8 bits of p1 rotation
		case 0x12/2:	return (dir1 >> 8);			// upper 4 bits of p1 rotation
		case 0x14/2:	return (dir2 & 0xff);		// lower 8 bits of p2 rotation
		case 0x16/2:	return (dir2 >> 8);			// upper 4 bits of p2 rotation
		case 0x18/2:	return 0xffff;				// ? (value's read but not used)
		default:
			logerror("PC %06X - Read input %02X !\n", cpu_get_pc(space->cpu), offset*2);
			return 0;
	}
}

static WRITE16_HANDLER( calibr50_soundlatch_w )
{
	if (ACCESSING_BITS_0_7)
	{
		soundlatch_word_w(space, 0, data, mem_mask);
		cputag_set_input_line(space->machine, "sub", INPUT_LINE_NMI, PULSE_LINE);
		cpu_spinuntil_time(space->cpu, ATTOTIME_IN_USEC(50));	// Allow the other cpu to reply
	}
}

static ADDRESS_MAP_START( calibr50_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x09ffff) AM_ROM								// ROM
	AM_RANGE(0xff0000, 0xffffff) AM_RAM								// RAM
	AM_RANGE(0x100000, 0x100007) AM_READNOP							// ? (same as a00010-a00017?)
	AM_RANGE(0x200000, 0x200fff) AM_RAM								// NVRAM
	AM_RANGE(0x300000, 0x300001) AM_READNOP							// ? (value's read but not used)
	AM_RANGE(0x300000, 0x300001) AM_WRITENOP						// ? (random value)
	AM_RANGE(0x400000, 0x400001) AM_READ(watchdog_reset16_r)		// Watchdog
	AM_RANGE(0x500000, 0x500001) AM_WRITENOP						// ?
	AM_RANGE(0x600000, 0x600003) AM_READ(seta_dsw_r)				// DSW
	AM_RANGE(0x700000, 0x7003ff) AM_RAM AM_BASE_GENERIC(paletteram) AM_SIZE(&seta_paletteram_size)	// Palette
	AM_RANGE(0x800000, 0x800005) AM_WRITEONLY AM_BASE(&seta_vctrl_0)// VRAM Ctrl
	AM_RANGE(0x900000, 0x903fff) AM_RAM_WRITE(seta_vram_0_w) AM_BASE(&seta_vram_0)	// VRAM

	AM_RANGE(0x904000, 0x904fff) AM_RAM								//
	AM_RANGE(0xa00000, 0xa00019) AM_READ(calibr50_ip_r)				// Input Ports
/**/AM_RANGE(0xd00000, 0xd00607) AM_RAM AM_BASE_GENERIC(spriteram)		// Sprites Y
	AM_RANGE(0xe00000, 0xe03fff) AM_RAM	AM_BASE_GENERIC(spriteram2)		// Sprites Code + X + Attr
	AM_RANGE(0xb00000, 0xb00001) AM_READWRITE(soundlatch2_word_r,calibr50_soundlatch_w)	// From Sub CPU
/**/AM_RANGE(0xc00000, 0xc00001) AM_RAM								// ? $4000
ADDRESS_MAP_END


/***************************************************************************
                                U.S. Classic
***************************************************************************/

static READ16_HANDLER( usclssic_dsw_r )
{
	switch (offset)
	{
		case 0/2:	return (input_port_read(space->machine, "DSW") >>  8) & 0xf;
		case 2/2:	return (input_port_read(space->machine, "DSW") >> 12) & 0xf;
		case 4/2:	return (input_port_read(space->machine, "DSW") >>  0) & 0xf;
		case 6/2:	return (input_port_read(space->machine, "DSW") >>  4) & 0xf;
	}
	return 0;
}

static READ16_HANDLER( usclssic_trackball_x_r )
{
	static const char *const portx_name[2] = { "P1X", "P2X" };
	switch (offset)
	{
		case 0/2:	return (input_port_read(space->machine, portx_name[port_select]) >> 0) & 0xff;
		case 2/2:	return (input_port_read(space->machine, portx_name[port_select]) >> 8) & 0xff;
	}
	return 0;
}

static READ16_HANDLER( usclssic_trackball_y_r )
{
	static const char *const porty_name[2] = { "P1Y", "P2Y" };
	switch (offset)
	{
		case 0/2:	return (input_port_read(space->machine, porty_name[port_select]) >> 0) & 0xff;
		case 2/2:	return (input_port_read(space->machine, porty_name[port_select]) >> 8) & 0xff;
	}
	return 0;
}


static WRITE16_HANDLER( usclssic_lockout_w )
{
	static int old_tiles_offset = 0;

	if (ACCESSING_BITS_0_7)
	{
		port_select = (data & 0x40) >> 6;

		seta_tiles_offset = (data & 0x10) ? 0x4000: 0;
		if (old_tiles_offset != seta_tiles_offset)	tilemap_mark_all_tiles_dirty_all(space->machine);
		old_tiles_offset = seta_tiles_offset;

		seta_coin_lockout_w(space->machine, data);
	}
}


static ADDRESS_MAP_START( usclssic_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM									// ROM
	AM_RANGE(0xff0000, 0xffffff) AM_RAM									// RAM
	AM_RANGE(0x800000, 0x800607) AM_RAM	 AM_BASE_GENERIC(spriteram)			// Sprites Y
/**/AM_RANGE(0x900000, 0x900001) AM_RAM									// ? $4000
	AM_RANGE(0xa00000, 0xa00005) AM_RAM AM_BASE(&seta_vctrl_0)			// VRAM Ctrl
/**/AM_RANGE(0xb00000, 0xb003ff) AM_RAM AM_BASE_GENERIC(paletteram) AM_SIZE(&seta_paletteram_size)	// Palette
	AM_RANGE(0xb40000, 0xb40003) AM_READ(usclssic_trackball_x_r)		// TrackBall X
	AM_RANGE(0xb40000, 0xb40001) AM_WRITE(usclssic_lockout_w)			// Coin Lockout + Tiles Banking
	AM_RANGE(0xb40004, 0xb40007) AM_READ(usclssic_trackball_y_r)		// TrackBall Y + Buttons
	AM_RANGE(0xb4000a, 0xb4000b) AM_WRITENOP							// ? (value's not important. In lev2&6)
	AM_RANGE(0xb40010, 0xb40011) AM_READ_PORT("COINS")					// Coins
	AM_RANGE(0xb40010, 0xb40011) AM_WRITE(calibr50_soundlatch_w)		// To Sub CPU
	AM_RANGE(0xb40018, 0xb4001f) AM_READ(usclssic_dsw_r)				// 2 DSWs
	AM_RANGE(0xb40018, 0xb40019) AM_WRITE(watchdog_reset16_w)			// Watchdog
	AM_RANGE(0xb80000, 0xb80001) AM_READNOP								// Watchdog (value is discarded)?
	AM_RANGE(0xc00000, 0xc03fff) AM_RAM AM_BASE_GENERIC(spriteram2)			// Sprites Code + X + Attr
	AM_RANGE(0xd00000, 0xd03fff) AM_RAM_WRITE(seta_vram_0_w) AM_BASE(&seta_vram_0)	// VRAM
	AM_RANGE(0xd04000, 0xd04fff) AM_RAM									//
	AM_RANGE(0xe00000, 0xe00fff) AM_RAM									// NVRAM? (odd bytes)
ADDRESS_MAP_END


/***************************************************************************
                                Athena no Hatena?
***************************************************************************/

static ADDRESS_MAP_START( atehate_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM								// ROM
	AM_RANGE(0x900000, 0x9fffff) AM_RAM								// RAM
	AM_RANGE(0x100000, 0x103fff) AM_DEVREADWRITE("x1snd", seta_sound_word_r,seta_sound_word_w)	// Sound
	AM_RANGE(0x200000, 0x200001) AM_WRITENOP						// ? watchdog ?
	AM_RANGE(0x300000, 0x300001) AM_WRITENOP						// ? 0 (irq ack lev 2?)
	AM_RANGE(0x500000, 0x500001) AM_WRITENOP						// ? (end of lev 1: bit 4 goes 1,0,1)
	AM_RANGE(0x600000, 0x600003) AM_READ(seta_dsw_r)				// DSW
	AM_RANGE(0x700000, 0x7003ff) AM_RAM AM_BASE_GENERIC(paletteram) AM_SIZE(&seta_paletteram_size)	// Palette
	AM_RANGE(0xa00000, 0xa00607) AM_RAM	AM_BASE_GENERIC(spriteram)		// Sprites Y
	AM_RANGE(0xb00000, 0xb00001) AM_READ_PORT("P1")					// P1
	AM_RANGE(0xb00002, 0xb00003) AM_READ_PORT("P2")					// P2
	AM_RANGE(0xb00004, 0xb00005) AM_READ_PORT("COINS")				// Coins
/**/AM_RANGE(0xc00000, 0xc00001) AM_RAM								// ? 0x4000
	AM_RANGE(0xe00000, 0xe03fff) AM_RAM	AM_BASE_GENERIC(spriteram2)		// Sprites Code + X + Attr
ADDRESS_MAP_END


/***************************************************************************
                        Blandia
***************************************************************************/

static ADDRESS_MAP_START( blandia_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM								// ROM (up to 2MB)
	AM_RANGE(0x200000, 0x20ffff) AM_RAM								// RAM (main ram for zingzip, wrofaero writes to 20f000-20ffff)
	AM_RANGE(0x210000, 0x21ffff) AM_RAM								// RAM (gundhara)
	AM_RANGE(0x300000, 0x30ffff) AM_RAM								// RAM (wrofaero only?)
	AM_RANGE(0x400000, 0x400001) AM_READ_PORT("P1")					// P1
	AM_RANGE(0x400002, 0x400003) AM_READ_PORT("P2")					// P2
	AM_RANGE(0x400004, 0x400005) AM_READ_PORT("COINS")				// Coins
	AM_RANGE(0x500000, 0x500005) AM_RAM_WRITE(seta_vregs_w) AM_BASE(&seta_vregs)	// (gundhara) Coin Lockout + Video Registers
	AM_RANGE(0x600000, 0x600003) AM_READ(seta_dsw_r)				// DSW
	AM_RANGE(0x700000, 0x7003ff) AM_RAM								// (rezon,jjsquawk)
	AM_RANGE(0x700400, 0x700fff) AM_RAM AM_BASE_GENERIC(paletteram) AM_SIZE(&seta_paletteram_size)	// Palette
	AM_RANGE(0x701000, 0x70ffff) AM_RAM								//
/**/AM_RANGE(0x800000, 0x800607) AM_RAM AM_BASE_GENERIC(spriteram)		// Sprites Y
	AM_RANGE(0x880000, 0x880001) AM_RAM								// ? 0xc000
	AM_RANGE(0x900000, 0x903fff) AM_RAM AM_BASE_GENERIC(spriteram2)		// Sprites Code + X + Attr
/**/AM_RANGE(0xa00000, 0xa00005) AM_RAM AM_BASE(&seta_vctrl_0)		// VRAM 0&1 Ctrl
/**/AM_RANGE(0xa80000, 0xa80005) AM_RAM AM_BASE(&seta_vctrl_2)		// VRAM 2&3 Ctrl
	AM_RANGE(0xb00000, 0xb03fff) AM_RAM_WRITE(seta_vram_0_w) AM_BASE(&seta_vram_0)	// VRAM 0&1
	AM_RANGE(0xb04000, 0xb0ffff) AM_RAM								// (jjsquawk)
	AM_RANGE(0xb80000, 0xb83fff) AM_RAM_WRITE(seta_vram_2_w) AM_BASE(&seta_vram_2)	// VRAM 2&3
	AM_RANGE(0xb84000, 0xb8ffff) AM_RAM								// (jjsquawk)
	AM_RANGE(0xc00000, 0xc03fff) AM_DEVREADWRITE("x1snd", seta_sound_word_r,seta_sound_word_w)	// Sound
	AM_RANGE(0xd00000, 0xd00007) AM_WRITENOP						// ?
	AM_RANGE(0xe00000, 0xe00001) AM_WRITENOP						// ? VBlank IRQ Ack
	AM_RANGE(0xf00000, 0xf00001) AM_WRITENOP						// ? Sound  IRQ Ack
ADDRESS_MAP_END


/***************************************************************************
    Blandia (proto), Gundhara, J.J.Squawkers, Rezon, War of Aero, Zing Zing Zip
                        (with slight variations)
***************************************************************************/

static ADDRESS_MAP_START( blandiap_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM								// ROM (up to 2MB)
	AM_RANGE(0x200000, 0x20ffff) AM_RAM								// RAM (main ram for zingzip, wrofaero writes to 20f000-20ffff)
	AM_RANGE(0x210000, 0x21ffff) AM_RAM								// RAM (gundhara)
	AM_RANGE(0x300000, 0x30ffff) AM_RAM								// RAM (wrofaero only?)
	AM_RANGE(0x400000, 0x400001) AM_READ_PORT("P1")					// P1
	AM_RANGE(0x400002, 0x400003) AM_READ_PORT("P2")					// P2
	AM_RANGE(0x400004, 0x400005) AM_READ_PORT("COINS")				// Coins
	AM_RANGE(0x500000, 0x500005) AM_RAM_WRITE(seta_vregs_w) AM_BASE(&seta_vregs) // (gundhara) Coin Lockout + Video Registers
	AM_RANGE(0x600000, 0x600003) AM_READ(seta_dsw_r)				// DSW
	AM_RANGE(0x700000, 0x7003ff) AM_RAM								// (rezon,jjsquawk)
	AM_RANGE(0x700400, 0x700fff) AM_RAM AM_BASE_GENERIC(paletteram) AM_SIZE(&seta_paletteram_size)	// Palette
	AM_RANGE(0x701000, 0x70ffff) AM_RAM								//
	AM_RANGE(0x800000, 0x803fff) AM_RAM_WRITE(seta_vram_0_w) AM_BASE(&seta_vram_0)	// VRAM 0&1
	AM_RANGE(0x804000, 0x80ffff) AM_RAM								// (jjsquawk)
	AM_RANGE(0x880000, 0x883fff) AM_RAM_WRITE(seta_vram_2_w) AM_BASE(&seta_vram_2)	// VRAM 2&3
	AM_RANGE(0x884000, 0x88ffff) AM_RAM								// (jjsquawk)
/**/AM_RANGE(0x900000, 0x900005) AM_RAM AM_BASE(&seta_vctrl_0)		// VRAM 0&1 Ctrl
/**/AM_RANGE(0x980000, 0x980005) AM_RAM AM_BASE(&seta_vctrl_2)		// VRAM 2&3 Ctrl
/**/AM_RANGE(0xa00000, 0xa00607) AM_RAM AM_BASE_GENERIC(spriteram)		// Sprites Y
/**/AM_RANGE(0xa80000, 0xa80001) AM_RAM								// ? 0x4000
	AM_RANGE(0xb00000, 0xb03fff) AM_RAM AM_BASE_GENERIC(spriteram2)		// Sprites Code + X + Attr
	AM_RANGE(0xc00000, 0xc03fff) AM_DEVREADWRITE("x1snd", seta_sound_word_r,seta_sound_word_w)	// Sound
	AM_RANGE(0xd00000, 0xd00007) AM_WRITENOP						// ?
	AM_RANGE(0xe00000, 0xe00001) AM_WRITENOP						// ? VBlank IRQ Ack
	AM_RANGE(0xf00000, 0xf00001) AM_WRITENOP						// ? Sound  IRQ Ack
ADDRESS_MAP_END


/***************************************************************************
    Blandia, Gundhara, J.J.Squawkers, Rezon, War of Aero, Zing Zing Zip
                    and Zombie Raid (with slight variations)
***************************************************************************/

static int gun_input_bit = 0, gun_input_src = 0;

static READ16_HANDLER( zombraid_gun_r ) // Serial interface
{
	static const char *const portnames[] = { "GUNX1", "GUNY1", "GUNX2", "GUNY2" };

	int data = input_port_read(space->machine, portnames[gun_input_src]);	// Input Ports 5-8
	return (data >> gun_input_bit) & 1;
}

// Bit 0 is clock, 1 is data, 2 is reset
static WRITE16_HANDLER( zombraid_gun_w )
{
	static int bit_count = 0, old_clock = 0;

	if(data&4) { bit_count = 0; return; } // Reset

	if((data&1) == old_clock) return; // No change

	if(old_clock == 0) // Rising edge
	{
		switch (bit_count)
		{
			case 0:
			case 1: // Starting sequence 2,3,2,3. Other inputs?
				break;
			case 2: // First bit of source
				gun_input_src = (gun_input_src&2) | (data>>1);
				break;
			case 3: // Second bit of source
				gun_input_src = (gun_input_src&1) | (data&2);
				break;
			default:
				/* Gun Recoils */
				/* Note:  In debug menu recoil solenoids strobe when held down.  Is this correct?? */
				output_set_value("Player1_Gun_Recoil", (data & 0x10)>>4 );
				output_set_value("Player2_Gun_Recoil", (data & 0x8)>>3 );

				gun_input_bit = bit_count - 4;
				gun_input_bit = 8 - gun_input_bit; // Reverse order
				break;
		}
		bit_count++;
	}

	old_clock = data & 1;
}

static ADDRESS_MAP_START( wrofaero_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM								// ROM (up to 2MB)
	AM_RANGE(0x200000, 0x20ffff) AM_RAM AM_BASE(&seta_workram)		// RAM (pointer for zombraid crosshair hack)
	AM_RANGE(0x210000, 0x21ffff) AM_RAM								// RAM (gundhara)
	AM_RANGE(0x300000, 0x30ffff) AM_RAM								// RAM (wrofaero only?)
	AM_RANGE(0x400000, 0x400001) AM_READ_PORT("P1")					// P1
	AM_RANGE(0x400002, 0x400003) AM_READ_PORT("P2")					// P2
	AM_RANGE(0x400004, 0x400005) AM_READ_PORT("COINS")				// Coins
	AM_RANGE(0x500000, 0x500005) AM_RAM_WRITE(seta_vregs_w) AM_BASE(&seta_vregs)	// (gundhara) Coin Lockout + Video Registers

	AM_RANGE(0x600000, 0x600003) AM_READ(seta_dsw_r)				// DSW
	AM_RANGE(0x700000, 0x7003ff) AM_RAM								// (rezon,jjsquawk)
	AM_RANGE(0x700400, 0x700fff) AM_RAM AM_BASE_GENERIC(paletteram) AM_SIZE(&seta_paletteram_size)	// Palette
	AM_RANGE(0x701000, 0x70ffff) AM_RAM								//
	AM_RANGE(0x800000, 0x803fff) AM_RAM_WRITE(seta_vram_0_w) AM_BASE(&seta_vram_0)	// VRAM 0&1
	AM_RANGE(0x804000, 0x80ffff) AM_RAM								// (jjsquawk)
	AM_RANGE(0x880000, 0x883fff) AM_RAM_WRITE(seta_vram_2_w) AM_BASE(&seta_vram_2)	// VRAM 2&3
	AM_RANGE(0x884000, 0x88ffff) AM_RAM								// (jjsquawk)
/**/AM_RANGE(0x900000, 0x900005) AM_RAM AM_BASE(&seta_vctrl_0)		// VRAM 0&1 Ctrl
/**/AM_RANGE(0x980000, 0x980005) AM_RAM AM_BASE(&seta_vctrl_2)		// VRAM 2&3 Ctrl
/**/AM_RANGE(0xa00000, 0xa00607) AM_RAM AM_BASE_GENERIC(spriteram)		// Sprites Y
/**/AM_RANGE(0xa80000, 0xa80001) AM_RAM								// ? 0x4000
	AM_RANGE(0xb00000, 0xb03fff) AM_RAM	AM_BASE_GENERIC(spriteram2)		// Sprites Code + X + Attr
	AM_RANGE(0xc00000, 0xc03fff) AM_DEVREADWRITE("x1snd", seta_sound_word_r,seta_sound_word_w)	// Sound
#if __uPD71054_TIMER
	AM_RANGE(0xd00000, 0xd00007) AM_WRITE(timer_regs_w)				// ?
#else
	AM_RANGE(0xd00000, 0xd00007) AM_WRITENOP						// ?
#endif
	AM_RANGE(0xe00000, 0xe00001) AM_WRITENOP						// ? VBlank IRQ Ack
	AM_RANGE(0xf00000, 0xf00001) AM_WRITENOP						// ? Sound  IRQ Ack
ADDRESS_MAP_END

static ADDRESS_MAP_START( jjsquawb_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM								// ROM (up to 2MB)
	AM_RANGE(0x200000, 0x20ffff) AM_RAM AM_BASE(&seta_workram)		// RAM (pointer for zombraid crosshair hack)
	AM_RANGE(0x210000, 0x21ffff) AM_RAM								// RAM (gundhara)
	AM_RANGE(0x300000, 0x30ffff) AM_RAM								// RAM (wrofaero only?)
	AM_RANGE(0x400000, 0x400001) AM_READ_PORT("P1")					// P1
	AM_RANGE(0x400002, 0x400003) AM_READ_PORT("P2")					// P2
	AM_RANGE(0x400004, 0x400005) AM_READ_PORT("COINS")				// Coins
	AM_RANGE(0x500000, 0x500005) AM_RAM_WRITE(seta_vregs_w) AM_BASE(&seta_vregs)	// (gundhara) Coin Lockout + Video Registers

	AM_RANGE(0x600000, 0x600003) AM_READ(seta_dsw_r)				// DSW
	AM_RANGE(0x700000, 0x70b3ff) AM_RAM								// RZ: (rezon,jjsquawk)
	AM_RANGE(0x70b400, 0x70bfff) AM_RAM AM_BASE_GENERIC(paletteram) AM_SIZE(&seta_paletteram_size)	// Palette
	AM_RANGE(0x70c000, 0x70ffff) AM_RAM								//
	AM_RANGE(0x800000, 0x803fff) AM_RAM_WRITE(seta_vram_0_w) AM_BASE(&seta_vram_0)	// VRAM 0
	AM_RANGE(0x804000, 0x807fff) AM_RAM_WRITE(seta_vram_2_w) AM_BASE(&seta_vram_2)	// VRAM 2
	AM_RANGE(0x884000, 0x88ffff) AM_RAM								// (jjsquawk)
	AM_RANGE(0x908000, 0x908005) AM_RAM AM_BASE(&seta_vctrl_0)		// VRAM 0&1 Ctrl
	AM_RANGE(0x909000, 0x909005) AM_RAM AM_BASE(&seta_vctrl_2)		// VRAM 2&3 Ctrl
	AM_RANGE(0xa0a000, 0xa0a607) AM_RAM AM_BASE_GENERIC(spriteram)		// RZ: Sprites Y
// AM_RANGE(0xa80000, 0xa80001) AM_RAM                              // ? 0x4000
	AM_RANGE(0xb0c000, 0xb0ffff) AM_RAM AM_BASE_GENERIC(spriteram2)		// RZ: Sprites Code + X + Attr
	AM_RANGE(0xc00000, 0xc03fff) AM_DEVREADWRITE("x1snd", seta_sound_word_r,seta_sound_word_w)	// Sound
#if __uPD71054_TIMER
	AM_RANGE(0xd00000, 0xd00007) AM_WRITE(timer_regs_w)				// ?
#else
	AM_RANGE(0xd00000, 0xd00007) AM_WRITENOP						// ?
#endif
	AM_RANGE(0xe00000, 0xe00001) AM_WRITENOP						// ? VBlank IRQ Ack
	AM_RANGE(0xf00000, 0xf00001) AM_WRITENOP						// ? Sound  IRQ Ack
ADDRESS_MAP_END

/***************************************************************************
        Orbs
***************************************************************************/

static ADDRESS_MAP_START( orbs_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM								// ROM
	AM_RANGE(0xf00000, 0xf0ffff) AM_RAM								// RAM
	AM_RANGE(0x100000, 0x100001) AM_READNOP							// ?
	AM_RANGE(0x200000, 0x200001) AM_READNOP							// ?
	AM_RANGE(0x300000, 0x300003) AM_READ(seta_dsw_r)				// DSW
	AM_RANGE(0x400000, 0x400001) AM_WRITENOP						// ?
	AM_RANGE(0x500000, 0x500001) AM_READ_PORT("P1")					// P1
	AM_RANGE(0x500002, 0x500003) AM_READ_PORT("P2")					// P2
	AM_RANGE(0x500004, 0x500005) AM_READ_PORT("COINS")				// Coins
	//AM_RANGE(0x600000, 0x60000f) AM_READ(krzybowl_input_r     )   // P1
	AM_RANGE(0x8000f0, 0x8000f1) AM_RAM								// NVRAM
	AM_RANGE(0x800100, 0x8001ff) AM_RAM								// NVRAM
	AM_RANGE(0xa00000, 0xa03fff) AM_DEVREADWRITE("x1snd", seta_sound_word_r,seta_sound_word_w)	// Sound
	AM_RANGE(0xb00000, 0xb003ff) AM_RAM AM_BASE_GENERIC(paletteram) AM_SIZE(&seta_paletteram_size)	// Palette
	AM_RANGE(0xc00000, 0xc03fff) AM_RAM AM_BASE_GENERIC(spriteram2)		// Sprites Code + X + Attr
/**/AM_RANGE(0xd00000, 0xd00001) AM_RAM								// ? 0x4000
/**/AM_RANGE(0xe00000, 0xe00607) AM_RAM AM_BASE_GENERIC(spriteram)		// Sprites Y
ADDRESS_MAP_END

/***************************************************************************
                  Kero Kero Keroppi no Issyoni Asobou
***************************************************************************/

static int keroppi_prize_hop;
static int keroppi_protection_count;
static const UINT16 keroppi_protection_word[] = {
	0x0000,
	0x0000, 0x0000, 0x0000,
	0x2000, 0x2000, 0x2000,
	0x2000, 0x2000, 0x2000,
	0x0400, 0x0400, 0x0400,
	0x0000, 0x0000, 0x0000
};


static READ16_HANDLER( keroppi_protection_r )
{
	UINT16 result = keroppi_protection_word[keroppi_protection_count];

	keroppi_protection_count++;
	if (keroppi_protection_count > 15)
		keroppi_protection_count = 15;

	return result;
}

static READ16_HANDLER( keroppi_protection_init_r )
{
	keroppi_protection_count = 0;

	return 0x00;
}

static READ16_HANDLER( keroppi_coin_r )
{
	UINT16 result = input_port_read(space->machine, "COINS");

	if (keroppi_prize_hop == 2)
	{
		result &= ~0x0002;		/* prize hopper */
		keroppi_prize_hop = 0;
	}

	return result;
}

static TIMER_CALLBACK( keroppi_prize_hop_callback )
{
	keroppi_prize_hop = 2;
}

static WRITE16_HANDLER( keroppi_prize_w )
{
	if ((data & 0x0010) && !keroppi_prize_hop)
	{
		keroppi_prize_hop = 1;
		timer_set(space->machine, ATTOTIME_IN_SEC(3), NULL, 0x20, keroppi_prize_hop_callback);		/* 3 seconds */
	}
}

static ADDRESS_MAP_START( keroppi_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM								// ROM
	AM_RANGE(0xf00000, 0xf0ffff) AM_RAM								// RAM
	AM_RANGE(0x100000, 0x100001) AM_READ(keroppi_protection_r)		//
	AM_RANGE(0x200000, 0x200001) AM_READ(keroppi_protection_init_r)	//
	AM_RANGE(0x300000, 0x300003) AM_READ(seta_dsw_r)				// DSW
	AM_RANGE(0x400000, 0x400001) AM_WRITENOP						// ?
	AM_RANGE(0x500000, 0x500001) AM_READ_PORT("P1")					// P1
	AM_RANGE(0x500002, 0x500003) AM_READ_PORT("P2")					// P2
	AM_RANGE(0x500004, 0x500005) AM_READ(keroppi_coin_r)			// Coins
	AM_RANGE(0x8000f0, 0x8000f1) AM_RAM								// NVRAM
	AM_RANGE(0x800100, 0x8001ff) AM_RAM								// NVRAM
	AM_RANGE(0x900000, 0x900001) AM_WRITENOP						// ?
	AM_RANGE(0x900002, 0x900003) AM_WRITE(keroppi_prize_w)			//
	AM_RANGE(0xa00000, 0xa03fff) AM_DEVREADWRITE("x1snd", seta_sound_word_r,seta_sound_word_w)	// Sound
	AM_RANGE(0xb00000, 0xb003ff) AM_RAM AM_BASE_GENERIC(paletteram) AM_SIZE(&seta_paletteram_size)	// Palette
	AM_RANGE(0xc00000, 0xc03fff) AM_RAM AM_BASE_GENERIC(spriteram2)		// Sprites Code + X + Attr
/**/AM_RANGE(0xd00000, 0xd00001) AM_RAM								// ? 0x4000
/**/AM_RANGE(0xe00000, 0xe00607) AM_RAM AM_BASE_GENERIC(spriteram)		// Sprites Y
ADDRESS_MAP_END

static MACHINE_START( keroppi )
{
	keroppi_prize_hop = 0;
	keroppi_protection_count = 0;
}

/***************************************************************************
                                Block Carnival
***************************************************************************/

/* similar to krzybowl */
static ADDRESS_MAP_START( blockcar_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM								// ROM
	AM_RANGE(0xf00000, 0xf03fff) AM_RAM								// RAM
	AM_RANGE(0xf04000, 0xf041ff) AM_RAM								// Backup RAM?
	AM_RANGE(0xf05000, 0xf050ff) AM_RAM								// Backup RAM?
	AM_RANGE(0x100000, 0x100001) AM_WRITENOP						// ? 1 (start of interrupts, main loop: watchdog?)
	AM_RANGE(0x200000, 0x200001) AM_WRITENOP						// ? 0/1 (IRQ acknowledge?)
	AM_RANGE(0x300000, 0x300003) AM_READ(seta_dsw_r)				// DSW
	AM_RANGE(0x400000, 0x400001) AM_RAM_WRITE(seta_vregs_w) AM_BASE(&seta_vregs)	// Coin Lockout + Sound Enable (bit 4?)
	AM_RANGE(0x500000, 0x500001) AM_READ_PORT("P1")					// P1
	AM_RANGE(0x500002, 0x500003) AM_READ_PORT("P2")					// P2
	AM_RANGE(0x500004, 0x500005) AM_READ_PORT("COINS")				// Coins
	AM_RANGE(0xa00000, 0xa03fff) AM_DEVREADWRITE("x1snd", seta_sound_word_r,seta_sound_word_w)	// Sound
	AM_RANGE(0xb00000, 0xb003ff) AM_RAM AM_BASE_GENERIC(paletteram) AM_SIZE(&seta_paletteram_size)	// Palette
	AM_RANGE(0xc00000, 0xc03fff) AM_RAM AM_BASE_GENERIC(spriteram2)	// Sprites Code + X + Attr
/**/AM_RANGE(0xd00000, 0xd00001) AM_RAM	// ? 0x4000
/**/AM_RANGE(0xe00000, 0xe00607) AM_RAM AM_BASE_GENERIC(spriteram)	// Sprites Y
ADDRESS_MAP_END


/***************************************************************************
                                Daioh
***************************************************************************/

static ADDRESS_MAP_START( daioh_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM								// ROM
	AM_RANGE(0x100000, 0x10ffff) AM_RAM								// RAM
	AM_RANGE(0x400000, 0x400001) AM_READ_PORT("P1")					// P1
	AM_RANGE(0x400002, 0x400003) AM_READ_PORT("P2")					// P2
	AM_RANGE(0x400004, 0x400005) AM_READ_PORT("COINS")				// Coins
	AM_RANGE(0x500000, 0x500005) AM_RAM_WRITE(seta_vregs_w) AM_BASE(&seta_vregs)	// Coin Lockout + Video Registers
	AM_RANGE(0x500006, 0x500007) AM_READ_PORT("EXTRA")				// Buttons 4,5,6
	AM_RANGE(0x600000, 0x600003) AM_READ(seta_dsw_r				)	// DSW
	AM_RANGE(0x700000, 0x7003ff) AM_RAM
	AM_RANGE(0x700400, 0x700fff) AM_RAM AM_BASE_GENERIC(paletteram) AM_SIZE(&seta_paletteram_size)	// Palette
	AM_RANGE(0x701000, 0x70ffff) AM_RAM								//
	AM_RANGE(0x800000, 0x803fff) AM_RAM_WRITE(seta_vram_0_w) AM_BASE(&seta_vram_0	)	// VRAM 0&1
	AM_RANGE(0x804000, 0x80ffff) AM_RAM								//
	AM_RANGE(0x880000, 0x883fff) AM_RAM_WRITE(seta_vram_2_w) AM_BASE(&seta_vram_2	)	// VRAM 2&3
	AM_RANGE(0x884000, 0x88ffff) AM_RAM								//
	AM_RANGE(0x900000, 0x900005) AM_RAM AM_BASE(&seta_vctrl_0)		// VRAM 0&1 Ctrl
	AM_RANGE(0x980000, 0x980005) AM_RAM AM_BASE(&seta_vctrl_2)		// VRAM 2&3 Ctrl
	AM_RANGE(0xa00000, 0xa00607) AM_RAM AM_BASE_GENERIC(spriteram	)	// Sprites Y
	AM_RANGE(0xa80000, 0xa80001) AM_RAM	// ? 0x4000
	AM_RANGE(0xb00000, 0xb03fff) AM_RAM AM_BASE_GENERIC(spriteram2		)	// Sprites Code + X + Attr
	AM_RANGE(0xb04000, 0xb13fff) AM_RAM
	AM_RANGE(0xc00000, 0xc03fff) AM_DEVREADWRITE("x1snd", seta_sound_word_r,seta_sound_word_w)	// Sound
	AM_RANGE(0xe00000, 0xe00001) AM_WRITENOP	//
ADDRESS_MAP_END


/***************************************************************************
        Dragon Unit, Quiz Kokology, Quiz Kokology 2, Strike Gunner
***************************************************************************/

static ADDRESS_MAP_START( drgnunit_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0bffff) AM_ROM								// ROM
	AM_RANGE(0xf00000, 0xf0ffff) AM_RAM								// RAM (qzkklogy)
	AM_RANGE(0xffc000, 0xffffff) AM_RAM								// RAM (drgnunit,stg)
	AM_RANGE(0x100000, 0x103fff) AM_DEVREADWRITE("x1snd", seta_sound_word_r,seta_sound_word_w)	// Sound
	AM_RANGE(0x200000, 0x200001) AM_WRITENOP						// Watchdog
	AM_RANGE(0x300000, 0x300001) AM_WRITENOP						// ? IRQ Ack
	AM_RANGE(0x500000, 0x500001) AM_RAM_WRITE(seta_vregs_w) AM_BASE(&seta_vregs)	// Coin Lockout + Video Registers
	AM_RANGE(0x600000, 0x600003) AM_READ(seta_dsw_r)				// DSW
	AM_RANGE(0x700000, 0x7003ff) AM_RAM AM_BASE_GENERIC(paletteram) AM_SIZE(&seta_paletteram_size)	// Palette
	AM_RANGE(0x800000, 0x800005) AM_RAM AM_BASE(&seta_vctrl_0)		// VRAM Ctrl
	AM_RANGE(0x900000, 0x903fff) AM_RAM_WRITE(seta_vram_0_w) AM_BASE(&seta_vram_0	)	// VRAM
	AM_RANGE(0x904000, 0x90ffff) AM_WRITENOP						// unused (qzkklogy)
	AM_RANGE(0xb00000, 0xb00001) AM_READ_PORT("P1")					// P1
	AM_RANGE(0xb00002, 0xb00003) AM_READ_PORT("P2")					// P2
	AM_RANGE(0xb00004, 0xb00005) AM_READ_PORT("COINS")				// Coins
	AM_RANGE(0xb00006, 0xb00007) AM_READNOP							// unused (qzkklogy)
/**/AM_RANGE(0xc00000, 0xc00001) AM_RAM								// ? $4000
/**/AM_RANGE(0xd00000, 0xd00607) AM_RAM AM_BASE_GENERIC(spriteram)		// Sprites Y
	AM_RANGE(0xe00000, 0xe03fff) AM_RAM AM_BASE_GENERIC(spriteram2)		// Sprites Code + X + Attr
ADDRESS_MAP_END

/***************************************************************************
        Seta Roulette
***************************************************************************/

static READ16_HANDLER( setaroul_c0_r )
{
	return 0x00;//mame_rand(space->machine);
}

static READ16_HANDLER( setaroul_d4_0_r )
{
	return 0x00;//mame_rand(space->machine);
}

static READ16_HANDLER( setaroul_d4_4_r )
{
	return 0x00;//mame_rand(space->machine);
}

static READ16_HANDLER( setaroul_d4_6_r )
{
	return 0xffff;//mame_rand(space->machine);
}

static READ16_HANDLER( setaroul_d4_8_r )
{
	return mame_rand(space->machine)&0x000f;
}

static READ16_HANDLER( setaroul_d4_a_r )
{
	return 0x00;//mame_rand(space->machine);
}

static READ16_HANDLER( setaroul_d4_10_r )
{
	return 0x00;// mame_rand(space->machine);
}


// ?? looks like sprite ram access is 8-bit not 16?
static WRITE16_HANDLER( setaroul_spr_w )
{
	int realoffs = offset;
	realoffs >>=1;

	if (!(offset & 1))
	{
		data = data >> 8;
		mem_mask = mem_mask >> 8;
	}

	COMBINE_DATA(&space->machine->generic.spriteram.u16[realoffs]);

}

static ADDRESS_MAP_START( setaroul_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x01ffff) AM_ROM								// ROM

	AM_RANGE(0xc00000, 0xc03fff) AM_RAM AM_BASE_GENERIC(spriteram2)		// Sprites Code + X + Attr (maybe not)

	AM_RANGE(0xc40000, 0xc40001) AM_RAM //AM_BASE(&seta_vregs)
	AM_RANGE(0xc80000, 0xc80001) AM_NOP


	AM_RANGE(0xcc0000, 0xcc0019) AM_READ(setaroul_c0_r)

	AM_RANGE(0xd00000, 0xd00001) AM_NOP // watchdog?
	AM_RANGE(0xd40000, 0xd40001) AM_READ(setaroul_d4_0_r)
	AM_RANGE(0xd40004, 0xd40005) AM_READ(setaroul_d4_4_r)
	AM_RANGE(0xd40006, 0xd40007) AM_READ(setaroul_d4_6_r)
	AM_RANGE(0xd40008, 0xd40009) AM_READ(setaroul_d4_8_r)
	AM_RANGE(0xd4000a, 0xd4000b) AM_READ(setaroul_d4_a_r)
	AM_RANGE(0xd40010, 0xd40011) AM_READ(setaroul_d4_10_r) AM_WRITENOP // multiplex?

	AM_RANGE(0xdc0000, 0xdc3fff) AM_RAM

	AM_RANGE(0xe00000, 0xe03fff) AM_RAM_WRITE(seta_vram_0_w) AM_BASE(&seta_vram_0	)	// VRAM - draws wheel if you reset enough times..
	AM_RANGE(0xe40000, 0xe40005) AM_RAM AM_BASE(&seta_vctrl_0)		// VRAM Ctrl
	AM_RANGE(0xf00000, 0xf03fff) AM_RAM
	AM_RANGE(0xf40000, 0xf40c11) AM_WRITE(setaroul_spr_w) AM_BASE_GENERIC(spriteram)		// Sprites Y
ADDRESS_MAP_END

/***************************************************************************
                        Extreme Downhill / Sokonuke
***************************************************************************/

static ADDRESS_MAP_START( extdwnhl_map, ADDRESS_SPACE_PROGRAM, 16 )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x0fffff) AM_ROM								// ROM
	AM_RANGE(0x200000, 0x20ffff) AM_RAM								// RAM
	AM_RANGE(0x210000, 0x21ffff) AM_RAM								// RAM
	AM_RANGE(0x220000, 0x23ffff) AM_RAM								// RAM (sokonuke)
	AM_RANGE(0x400000, 0x400001) AM_READ_PORT("P1")					// P1
	AM_RANGE(0x400002, 0x400003) AM_READ_PORT("P2")					// P2
	AM_RANGE(0x400004, 0x400005) AM_READ_PORT("COINS")				// Coins
	AM_RANGE(0x400008, 0x40000b) AM_READ(seta_dsw_r)				// DSW
	AM_RANGE(0x40000c, 0x40000d) AM_READWRITE(watchdog_reset16_r,watchdog_reset16_w)	// Watchdog (extdwnhl (R) & sokonuke (W) MUST RETURN $FFFF)
	AM_RANGE(0x500000, 0x500003) AM_RAM_WRITE(seta_vregs_w) AM_BASE(&seta_vregs)	// Coin Lockout + Video Registers
	AM_RANGE(0x500004, 0x500007) AM_NOP								// IRQ Ack  (extdwnhl (R) & sokonuke (W))
	AM_RANGE(0x600400, 0x600fff) AM_RAM AM_BASE_GENERIC(paletteram) AM_SIZE(&seta_paletteram_size)	// Palette
	AM_RANGE(0x601000, 0x610bff) AM_RAM								//
	AM_RANGE(0x800000, 0x803fff) AM_RAM_WRITE(seta_vram_0_w) AM_BASE(&seta_vram_0)	// VRAM 0&1
	AM_RANGE(0x804000, 0x80ffff) AM_RAM								//
	AM_RANGE(0x880000, 0x883fff) AM_RAM_WRITE(seta_vram_2_w) AM_BASE(&seta_vram_2)	// VRAM 2&3
	AM_RANGE(0x884000, 0x88ffff) AM_RAM								//
/**/AM_RANGE(0x900000, 0x900005) AM_RAM AM_BASE(&seta_vctrl_0)		// VRAM 0&1 Ctrl
/**/AM_RANGE(0x980000, 0x980005) AM_RAM AM_BASE(&seta_vctrl_2)		// VRAM 2&3 Ctrl
/**/AM_RANGE(0xa00000, 0xa00607) AM_RAM AM_BASE_GENERIC(spriteram)		// Sprites Y
/**/AM_RANGE(0xa80000, 0xa80001) AM_RAM								// ? 0x4000
	AM_RANGE(0xb00000, 0xb03fff) AM_RAM AM_BASE_GENERIC(spriteram2)		// Sprites Code + X + Attr
	AM_RANGE(0xb04000, 0xb13fff) AM_RAM								//
	AM_RANGE(0xe00000, 0xe03fff) AM_DEVREADWRITE("x1snd", seta_sound_word_r,seta_sound_word_w)	// Sound
ADDRESS_MAP_END


/***************************************************************************
        (Kamen) Masked Riders Club Battle Race / Mad Shark
***************************************************************************/

static ADDRESS_MAP_START( kamenrid_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM								// ROM
	AM_RANGE(0x200000, 0x20ffff) AM_RAM								// RAM
	AM_RANGE(0x500000, 0x500001) AM_READ_PORT("P1")					// P1
	AM_RANGE(0x500002, 0x500003) AM_READ_PORT("P2")					// P2
	AM_RANGE(0x500004, 0x500007) AM_READ(seta_dsw_r)				// DSW
	AM_RANGE(0x500008, 0x500009) AM_READ_PORT("COINS")				// Coins
	AM_RANGE(0x50000c, 0x50000d) AM_READWRITE(watchdog_reset16_r,watchdog_reset16_w)	// xx Watchdog? (sokonuke)
	AM_RANGE(0x600000, 0x600005) AM_RAM_WRITE(seta_vregs_w) AM_BASE(&seta_vregs)	// ? Coin Lockout + Video Registers
	AM_RANGE(0x600006, 0x600007) AM_WRITENOP						// ?
	AM_RANGE(0x700000, 0x7003ff) AM_RAM								// Palette RAM (tested)
	AM_RANGE(0x700400, 0x700fff) AM_RAM AM_BASE_GENERIC(paletteram) AM_SIZE(&seta_paletteram_size)	// Palette
	AM_RANGE(0x701000, 0x703fff) AM_RAM 							// Palette
	AM_RANGE(0x800000, 0x803fff) AM_RAM_WRITE(seta_vram_0_w) AM_BASE(&seta_vram_0	)	// VRAM 0&1
	AM_RANGE(0x804000, 0x807fff) AM_RAM	// tested
	AM_RANGE(0x880000, 0x883fff) AM_RAM_WRITE(seta_vram_2_w) AM_BASE(&seta_vram_2	)	// VRAM 2&3
	AM_RANGE(0x884000, 0x887fff) AM_RAM	// tested
	AM_RANGE(0x900000, 0x900005) AM_RAM AM_BASE(&seta_vctrl_0)		// VRAM 0&1 Ctrl
	AM_RANGE(0x980000, 0x980005) AM_RAM AM_BASE(&seta_vctrl_2)		// VRAM 2&3 Ctrl
	AM_RANGE(0xa00000, 0xa00607) AM_RAM AM_BASE_GENERIC(spriteram)		// Sprites Y
	AM_RANGE(0xa80000, 0xa80001) AM_RAM								// ? $4000
	AM_RANGE(0xb00000, 0xb03fff) AM_RAM AM_BASE_GENERIC(spriteram2)		// Sprites Code + X + Attr
	AM_RANGE(0xb04000, 0xb07fff) AM_RAM								// tested
#if __uPD71054_TIMER
	AM_RANGE(0xc00000, 0xc00007) AM_WRITE(timer_regs_w)				// ?
#else
	AM_RANGE(0xc00000, 0xc00007) AM_WRITENOP						// ?
#endif
	AM_RANGE(0xd00000, 0xd03fff) AM_DEVREADWRITE("x1snd", seta_sound_word_r,seta_sound_word_w)	// Sound
ADDRESS_MAP_END

/* almost identical to kamenrid */
static ADDRESS_MAP_START( madshark_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM								// ROM
	AM_RANGE(0x200000, 0x20ffff) AM_RAM								// RAM
	AM_RANGE(0x500000, 0x500001) AM_READ_PORT("P1")					// P1
	AM_RANGE(0x500002, 0x500003) AM_READ_PORT("P2")					// P2
	AM_RANGE(0x500004, 0x500005) AM_READ_PORT("COINS")				// Coins
	AM_RANGE(0x500008, 0x50000b) AM_READ(seta_dsw_r)				// DSW
	AM_RANGE(0x50000c, 0x50000d) AM_WRITE(watchdog_reset16_w)		// Watchdog
	AM_RANGE(0x600000, 0x600005) AM_RAM_WRITE(seta_vregs_w) AM_BASE(&seta_vregs)	// ? Coin Lockout + Video Registers
	AM_RANGE(0x600006, 0x600007) AM_WRITENOP						// ?
	AM_RANGE(0x700400, 0x700fff) AM_RAM AM_BASE_GENERIC(paletteram) AM_SIZE(&seta_paletteram_size)	// Palette
	AM_RANGE(0x800000, 0x803fff) AM_RAM_WRITE(seta_vram_0_w) AM_BASE(&seta_vram_0)	// VRAM 0&1
	AM_RANGE(0x880000, 0x883fff) AM_RAM_WRITE(seta_vram_2_w) AM_BASE(&seta_vram_2)	// VRAM 2&3
	AM_RANGE(0x900000, 0x900005) AM_RAM AM_BASE(&seta_vctrl_0)		// VRAM 0&1 Ctrl
	AM_RANGE(0x980000, 0x980005) AM_RAM AM_BASE(&seta_vctrl_2)		// VRAM 2&3 Ctrl

	AM_RANGE(0xa00000, 0xa00607) AM_RAM	AM_BASE_GENERIC(spriteram)		// Sprites Y
	AM_RANGE(0xa80000, 0xa80001) AM_RAM								// ? $4000
	AM_RANGE(0xb00000, 0xb03fff) AM_RAM	AM_BASE_GENERIC(spriteram2)		// Sprites Code + X + Attr
#if __uPD71054_TIMER
	AM_RANGE(0xc00000, 0xc00007) AM_WRITE(timer_regs_w)				// ?
#else
	AM_RANGE(0xc00000, 0xc00007) AM_WRITENOP						// ?
#endif
	AM_RANGE(0xd00000, 0xd03fff) AM_DEVREADWRITE("x1snd", seta_sound_word_r,seta_sound_word_w)	// Sound
ADDRESS_MAP_END


/***************************************************************************
                                Krazy Bowl
***************************************************************************/

static READ16_HANDLER( krzybowl_input_r )
{
	// analog ports
	int dir1x = input_port_read(space->machine, "TRACK1_X") & 0xfff;
	int dir1y = input_port_read(space->machine, "TRACK1_Y") & 0xfff;
	int dir2x = input_port_read(space->machine, "TRACK2_X") & 0xfff;
	int dir2y = input_port_read(space->machine, "TRACK2_Y") & 0xfff;

	switch (offset)
	{
		case 0x0/2:	return dir1x & 0xff;
		case 0x2/2:	return dir1x >> 8;
		case 0x4/2:	return dir1y & 0xff;
		case 0x6/2:	return dir1y >> 8;
		case 0x8/2:	return dir2x & 0xff;
		case 0xa/2:	return dir2x >> 8;
		case 0xc/2:	return dir2y & 0xff;
		case 0xe/2:	return dir2y >> 8;
		default:
			logerror("PC %06X - Read input %02X !\n", cpu_get_pc(space->cpu), offset*2);
			return 0;
	}
}

static ADDRESS_MAP_START( krzybowl_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM 							// ROM
	AM_RANGE(0xf00000, 0xf0ffff) AM_RAM								// RAM
	AM_RANGE(0x100000, 0x100001) AM_READNOP							// ?
	AM_RANGE(0x200000, 0x200001) AM_READNOP							// ?
	AM_RANGE(0x300000, 0x300003) AM_READ(seta_dsw_r)				// DSW
	AM_RANGE(0x400000, 0x400001) AM_WRITENOP						// ?
	AM_RANGE(0x500000, 0x500001) AM_READ_PORT("P1")					// P1
	AM_RANGE(0x500002, 0x500003) AM_READ_PORT("P2")					// P2
	AM_RANGE(0x500004, 0x500005) AM_READ_PORT("COINS")				// Coins
	AM_RANGE(0x600000, 0x60000f) AM_READ(krzybowl_input_r)			// P1
	AM_RANGE(0x8000f0, 0x8000f1) AM_RAM								// NVRAM
	AM_RANGE(0x800100, 0x8001ff) AM_RAM								// NVRAM
	AM_RANGE(0xa00000, 0xa03fff) AM_DEVREADWRITE("x1snd", seta_sound_word_r,seta_sound_word_w)	// Sound
	AM_RANGE(0xb00000, 0xb003ff) AM_RAM AM_BASE_GENERIC(paletteram) AM_SIZE(&seta_paletteram_size)	// Palette
	AM_RANGE(0xc00000, 0xc03fff) AM_RAM AM_BASE_GENERIC(spriteram2)		// Sprites Code + X + Attr
/**/AM_RANGE(0xd00000, 0xd00001) AM_RAM								// ? 0x4000
/**/AM_RANGE(0xe00000, 0xe00607) AM_RAM AM_BASE_GENERIC(spriteram)		// Sprites Y
ADDRESS_MAP_END


/***************************************************************************
                            Mobile Suit Gundam
***************************************************************************/

static WRITE16_HANDLER( msgundam_vregs_w )
{
	// swap $500002 with $500004
	switch( offset )
	{
		case 1:	offset = 2;	break;
		case 2:	offset = 1;	break;
	}
	seta_vregs_w(space,offset,data,mem_mask);
}

/* Mirror RAM is necessary or startup, to clear Work RAM after the test */

static ADDRESS_MAP_START( msgundam_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM								// ROM
	AM_RANGE(0x100000, 0x1fffff) AM_ROM								// ROM
	AM_RANGE(0x200000, 0x20ffff) AM_RAM AM_MIRROR(0x70000)			// RAM
	AM_RANGE(0x400000, 0x400001) AM_READ_PORT("P1")					// P1
	AM_RANGE(0x400002, 0x400003) AM_READ_PORT("P2")					// P2
	AM_RANGE(0x400004, 0x400005) AM_READ_PORT("COINS")				// Coins
	AM_RANGE(0x400000, 0x400001) AM_WRITENOP						// Lev 2 IRQ Ack
	AM_RANGE(0x400004, 0x400005) AM_WRITENOP						// Lev 4 IRQ Ack
	AM_RANGE(0x500000, 0x500005) AM_RAM_WRITE(msgundam_vregs_w) AM_BASE(&seta_vregs)	// Coin Lockout + Video Registers
	AM_RANGE(0x600000, 0x600003) AM_READ(seta_dsw_r)				// DSW
	AM_RANGE(0x700400, 0x700fff) AM_RAM AM_BASE_GENERIC(paletteram) AM_SIZE(&seta_paletteram_size)	// Palette
	AM_RANGE(0x800000, 0x800607) AM_RAM AM_BASE_GENERIC(spriteram)		// Sprites Y
	AM_RANGE(0x880000, 0x880001) AM_RAM								// ? 0x4000
	AM_RANGE(0x900000, 0x903fff) AM_RAM AM_BASE_GENERIC(spriteram2)		// Sprites Code + X + Attr
	AM_RANGE(0xa00000, 0xa03fff) AM_RAM_WRITE(seta_vram_0_w) AM_BASE(&seta_vram_0)	// VRAM 0&1
	AM_RANGE(0xa80000, 0xa83fff) AM_RAM_WRITE(seta_vram_2_w) AM_BASE(&seta_vram_2)	// VRAM 2&3
	AM_RANGE(0xb00000, 0xb00005) AM_RAM AM_BASE(&seta_vctrl_0)		// VRAM 0&1 Ctrl
	AM_RANGE(0xb80000, 0xb80005) AM_RAM AM_BASE(&seta_vctrl_2)		// VRAM 2&3 Ctrl
	AM_RANGE(0xc00000, 0xc03fff) AM_DEVREADWRITE("x1snd", seta_sound_word_r,seta_sound_word_w)	// Sound
#if __uPD71054_TIMER
	AM_RANGE(0xd00000, 0xd00007) AM_WRITE(timer_regs_w)	// ?
#else
	AM_RANGE(0xd00000, 0xd00007) AM_WRITENOP	// ?
#endif
ADDRESS_MAP_END


/***************************************************************************
                                Oishii Puzzle
***************************************************************************/

/* similar to wrofaero */
static ADDRESS_MAP_START( oisipuzl_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM								// ROM
	AM_RANGE(0x100000, 0x17ffff) AM_ROM								// ROM
	AM_RANGE(0x200000, 0x20ffff) AM_RAM								// RAM
	AM_RANGE(0x300000, 0x300003) AM_READ(seta_dsw_r)				// DSW
	AM_RANGE(0x400000, 0x400001) AM_READ_PORT("P1")					// P1
	AM_RANGE(0x400002, 0x400003) AM_READ_PORT("P2")					// P2
	AM_RANGE(0x400004, 0x400005) AM_READ_PORT("COINS")				// Coins
	AM_RANGE(0x400000, 0x400001) AM_WRITENOP						// ? IRQ Ack
	AM_RANGE(0x500000, 0x500005) AM_RAM_WRITE(seta_vregs_w) AM_BASE(&seta_vregs)	// Coin Lockout + Video Registers
	AM_RANGE(0x700000, 0x703fff) AM_DEVREADWRITE("x1snd", seta_sound_word_r,seta_sound_word_w)	// Sound
	AM_RANGE(0x800000, 0x803fff) AM_RAM_WRITE(seta_vram_0_w) AM_BASE(&seta_vram_0)	// VRAM 0&1
	AM_RANGE(0x880000, 0x883fff) AM_RAM_WRITE(seta_vram_2_w) AM_BASE(&seta_vram_2)	// VRAM 2&3
/**/AM_RANGE(0x900000, 0x900005) AM_RAM AM_BASE(&seta_vctrl_0)		// VRAM 0&1 Ctrl
/**/AM_RANGE(0x980000, 0x980005) AM_RAM AM_BASE(&seta_vctrl_2)		// VRAM 2&3 Ctrl
/**/AM_RANGE(0xa00000, 0xa00607) AM_RAM AM_BASE_GENERIC(spriteram)		// Sprites Y
/**/AM_RANGE(0xa80000, 0xa80001) AM_RAM 							// ? 0x4000
	AM_RANGE(0xb00000, 0xb03fff) AM_RAM AM_BASE_GENERIC(spriteram2)		// Sprites Code + X + Attr
	AM_RANGE(0xc00400, 0xc00fff) AM_RAM AM_BASE_GENERIC(paletteram) AM_SIZE(&seta_paletteram_size)	// Palette
ADDRESS_MAP_END


/***************************************************************************
                                Triple Fun
***************************************************************************/

/* Same as oisipuzl but with the sound system replaced */

static ADDRESS_MAP_START( triplfun_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM								// ROM
	AM_RANGE(0x100000, 0x17ffff) AM_ROM								// ROM
	AM_RANGE(0x200000, 0x20ffff) AM_RAM								// RAM
	AM_RANGE(0x300000, 0x300003) AM_READ(seta_dsw_r)				// DSW
	AM_RANGE(0x400000, 0x400001) AM_READ_PORT("P1")					// P1
	AM_RANGE(0x400002, 0x400003) AM_READ_PORT("P2")					// P2
	AM_RANGE(0x400004, 0x400005) AM_READ_PORT("COINS")				// Coins
	AM_RANGE(0x400000, 0x400001) AM_WRITENOP						// ? IRQ Ack
	AM_RANGE(0x500000, 0x500005) AM_RAM_WRITE(seta_vregs_w) AM_BASE(&seta_vregs)	// Coin Lockout + Video Registers
	AM_RANGE(0x500006, 0x500007) AM_DEVREADWRITE8("oki", okim6295_r,okim6295_w, 0x00ff) // tfun sound
	AM_RANGE(0x800000, 0x803fff) AM_RAM_WRITE(seta_vram_0_w) AM_BASE(&seta_vram_0)	// VRAM 0&1
	AM_RANGE(0x880000, 0x883fff) AM_RAM_WRITE(seta_vram_2_w) AM_BASE(&seta_vram_2)	// VRAM 2&3
/**/AM_RANGE(0x900000, 0x900005) AM_RAM AM_BASE(&seta_vctrl_0)		// VRAM 0&1 Ctrl
/**/AM_RANGE(0x980000, 0x980005) AM_RAM AM_BASE(&seta_vctrl_2)		// VRAM 2&3 Ctrl
/**/AM_RANGE(0xa00000, 0xa00607) AM_RAM AM_BASE_GENERIC(spriteram)		// Sprites Y
/**/AM_RANGE(0xa80000, 0xa80001) AM_RAM 							// ? 0x4000
	AM_RANGE(0xb00000, 0xb03fff) AM_RAM AM_BASE_GENERIC(spriteram2)		// Sprites Code + X + Attr
	AM_RANGE(0xc00400, 0xc00fff) AM_RAM AM_BASE_GENERIC(paletteram) AM_SIZE(&seta_paletteram_size)	// Palette
ADDRESS_MAP_END


/***************************************************************************
                            Pro Mahjong Kiwame
***************************************************************************/

static UINT16 *kiwame_nvram;

static READ16_HANDLER( kiwame_nvram_r )
{
	return kiwame_nvram[offset] & 0xff;
}

static WRITE16_HANDLER( kiwame_nvram_w )
{
	if (ACCESSING_BITS_0_7)	COMBINE_DATA( &kiwame_nvram[offset] );
}

static READ16_HANDLER( kiwame_input_r )
{
	int row_select = kiwame_nvram_r( space, 0x10a/2,0x00ff ) & 0x1f;
	int i;
	static const char *const keynames[] = { "KEY0", "KEY1", "KEY2", "KEY3", "KEY4" };

	for(i = 0; i < 5; i++)
		if (row_select & (1<<i))	break;

	switch( offset )
	{
		case 0x00/2:	return input_port_read(space->machine, keynames[i]);
		case 0x02/2:	return 0xffff;
		case 0x04/2:	return input_port_read(space->machine, "COINS");
//      case 0x06/2:
		case 0x08/2:	return 0xffff;

		default:
			logerror("PC %06X - Read input %02X !\n", cpu_get_pc(space->cpu), offset*2);
			return 0x0000;
	}
}

static ADDRESS_MAP_START( kiwame_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM								// ROM
	AM_RANGE(0x200000, 0x20ffff) AM_RAM								// RAM
	AM_RANGE(0xfffc00, 0xffffff) AM_READWRITE(kiwame_nvram_r,kiwame_nvram_w) AM_BASE(&kiwame_nvram)	// NVRAM + Regs ?
	AM_RANGE(0x800000, 0x803fff) AM_RAM AM_BASE_GENERIC(spriteram2)		// Sprites Code + X + Attr
/**/AM_RANGE(0x900000, 0x900001) AM_RAM								// ? 0x4000
/**/AM_RANGE(0xa00000, 0xa00607) AM_RAM AM_BASE_GENERIC(spriteram)		// Sprites Y
	AM_RANGE(0xb00000, 0xb003ff) AM_RAM AM_BASE_GENERIC(paletteram) AM_SIZE(&seta_paletteram_size)	// Palette
	AM_RANGE(0xc00000, 0xc03fff) AM_DEVREADWRITE("x1snd", seta_sound_word_r,seta_sound_word_w)	// Sound
	AM_RANGE(0xd00000, 0xd00009) AM_READ(kiwame_input_r)			// mahjong panel
	AM_RANGE(0xe00000, 0xe00003) AM_READ(seta_dsw_r)				// DSW
ADDRESS_MAP_END


/***************************************************************************
                        Thunder & Lightning / Wit's
***************************************************************************/

static READ16_HANDLER( thunderl_protection_r )
{
//  logerror("PC %06X - Protection Read\n", cpu_get_pc(space->cpu));
	return 0x00dd;
}
static WRITE16_HANDLER( thunderl_protection_w )
{
//  logerror("PC %06X - Protection Written: %04X <- %04X\n", cpu_get_pc(space->cpu), offset*2, data);
}

/* Similar to downtown etc. */

static ADDRESS_MAP_START( thunderl_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x00ffff) AM_ROM								// ROM
	AM_RANGE(0xffc000, 0xffffff) AM_RAM								// RAM
	AM_RANGE(0x100000, 0x103fff) AM_DEVREADWRITE("x1snd", seta_sound_word_r,seta_sound_word_w)	// Sound
	AM_RANGE(0x200000, 0x200001) AM_WRITENOP						// ?
	AM_RANGE(0x300000, 0x300001) AM_WRITENOP						// ?
	AM_RANGE(0x400000, 0x40ffff) AM_WRITE(thunderl_protection_w)	// Protection (not in wits)
	AM_RANGE(0x500000, 0x500001) AM_RAM_WRITE(seta_vregs_w) AM_BASE(&seta_vregs	)	// Coin Lockout
	AM_RANGE(0x600000, 0x600003) AM_READ(seta_dsw_r)				// DSW
	AM_RANGE(0x700000, 0x7003ff) AM_RAM AM_BASE_GENERIC(paletteram) AM_SIZE(&seta_paletteram_size)	// Palette
	AM_RANGE(0xb00000, 0xb00001) AM_READ_PORT("P1")					// P1
	AM_RANGE(0xb00002, 0xb00003) AM_READ_PORT("P2")					// P2
	AM_RANGE(0xb00004, 0xb00005) AM_READ_PORT("COINS")				// Coins
	AM_RANGE(0xb0000c, 0xb0000d) AM_READ(thunderl_protection_r	)	// Protection (not in wits)
	AM_RANGE(0xb00008, 0xb00009) AM_READ_PORT("P3")					// P3 (wits)
	AM_RANGE(0xb0000a, 0xb0000b) AM_READ_PORT("P4")					// P4 (wits)
/**/AM_RANGE(0xc00000, 0xc00001) AM_RAM								// ? 0x4000
/**/AM_RANGE(0xd00000, 0xd00607) AM_RAM AM_BASE_GENERIC(spriteram)		// Sprites Y
	AM_RANGE(0xe00000, 0xe03fff) AM_RAM AM_BASE_GENERIC(spriteram2)		// Sprites Code + X + Attr
	AM_RANGE(0xe04000, 0xe07fff) AM_RAM								// (wits)
ADDRESS_MAP_END


/***************************************************************************
                    Wiggie Waggie
***************************************************************************/

static int wiggie_soundlatch;

static READ8_HANDLER( wiggie_soundlatch_r )
{
	return wiggie_soundlatch;
}

static WRITE16_HANDLER( wiggie_soundlatch_w )
{
	wiggie_soundlatch = data >> 8;
	cputag_set_input_line(space->machine, "audiocpu", 0, HOLD_LINE);
}


static ADDRESS_MAP_START( wiggie_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x01ffff) AM_ROM								// ROM
	AM_RANGE(0xffc000, 0xffffff) AM_RAM								// RAM
	AM_RANGE(0x200000, 0x200001) AM_WRITENOP						// ?
	AM_RANGE(0x300000, 0x300001) AM_WRITENOP						// ?
	AM_RANGE(0x400000, 0x40ffff) AM_WRITE(thunderl_protection_w)	// Protection (not in wits)
	AM_RANGE(0x500000, 0x500001) AM_RAM_WRITE(seta_vregs_w) AM_BASE(&seta_vregs)	// Coin Lockout
	AM_RANGE(0x600000, 0x600003) AM_READ(seta_dsw_r)				// DSW
	AM_RANGE(0x700000, 0x7003ff) AM_RAM AM_BASE_GENERIC(paletteram) AM_SIZE(&seta_paletteram_size)	// Palette
	AM_RANGE(0xb00000, 0xb00001) AM_READ_PORT("P1")					// P1
	AM_RANGE(0xb00002, 0xb00003) AM_READ_PORT("P2")					// P2
	AM_RANGE(0xb00004, 0xb00005) AM_READ_PORT("COINS")				// Coins
	AM_RANGE(0xb0000c, 0xb0000d) AM_READ(thunderl_protection_r)		// Protection (not in wits)
	AM_RANGE(0xb00008, 0xb00009) AM_READ_PORT("P3")					// P3 (wits)
	AM_RANGE(0xb0000a, 0xb0000b) AM_READ_PORT("P4")					// P4 (wits)
/**/AM_RANGE(0xc00000, 0xc00001) AM_RAM								// ? 0x4000
/**/AM_RANGE(0xd00000, 0xd00607) AM_RAM AM_BASE_GENERIC(spriteram)		// Sprites Y
	AM_RANGE(0xe00000, 0xe03fff) AM_RAM AM_BASE_GENERIC(spriteram2)		// Sprites Code + X + Attr
	AM_RANGE(0xe04000, 0xe07fff) AM_RAM	// (wits)
ADDRESS_MAP_END

static ADDRESS_MAP_START( wiggie_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x9800, 0x9800) AM_DEVREADWRITE("oki", okim6295_r,okim6295_w)
	AM_RANGE(0xa000, 0xa000) AM_READ(wiggie_soundlatch_r)
ADDRESS_MAP_END


/***************************************************************************
                    Ultraman Club / SD Gundam Neo Battling
***************************************************************************/

static ADDRESS_MAP_START( umanclub_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM								// ROM
	AM_RANGE(0x200000, 0x20ffff) AM_RAM								// RAM
	AM_RANGE(0x300000, 0x3003ff) AM_RAM AM_BASE_GENERIC(paletteram) AM_SIZE(&seta_paletteram_size)	// Palette
	AM_RANGE(0x300400, 0x300fff) AM_RAM								//
	AM_RANGE(0x400000, 0x400001) AM_READ_PORT("P1")					// P1
	AM_RANGE(0x400002, 0x400003) AM_READ_PORT("P2")					// P2
	AM_RANGE(0x400004, 0x400005) AM_READ_PORT("COINS")				// Coins
	AM_RANGE(0x400000, 0x400001) AM_WRITENOP						// ? (end of lev 2)
	AM_RANGE(0x400004, 0x400005) AM_WRITENOP						// ? (end of lev 2)
	AM_RANGE(0x500000, 0x500001) AM_RAM_WRITE(seta_vregs_w) AM_BASE(&seta_vregs	)	// Coin Lockout + Video Registers
	AM_RANGE(0x600000, 0x600003) AM_READ(seta_dsw_r)				// DSW
	AM_RANGE(0xa00000, 0xa00607) AM_RAM AM_BASE_GENERIC(spriteram)		// Sprites Y
/**/AM_RANGE(0xa80000, 0xa80001) AM_RAM								// ? 0x4000
	AM_RANGE(0xb00000, 0xb03fff) AM_RAM AM_BASE_GENERIC(spriteram2)		// Sprites Code + X + Attr
	AM_RANGE(0xc00000, 0xc03fff) AM_DEVREADWRITE("x1snd", seta_sound_word_r,seta_sound_word_w)	// Sound
ADDRESS_MAP_END


/***************************************************************************
                            Ultra Toukond Densetsu
***************************************************************************/

static WRITE16_HANDLER( utoukond_soundlatch_w )
{
	if (ACCESSING_BITS_0_7)
	{
		cputag_set_input_line(space->machine, "audiocpu", 0, HOLD_LINE);
		soundlatch_w(space, 0, data & 0xff);
	}
}

static ADDRESS_MAP_START( utoukond_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM								// ROM
	AM_RANGE(0x200000, 0x20ffff) AM_RAM								// RAM
	AM_RANGE(0x400000, 0x400001) AM_READ_PORT("P1")					// P1
	AM_RANGE(0x400002, 0x400003) AM_READ_PORT("P2")					// P2
	AM_RANGE(0x400004, 0x400005) AM_READ_PORT("COINS")				// Coins
	AM_RANGE(0x500000, 0x500005) AM_RAM_WRITE(seta_vregs_w) AM_BASE(&seta_vregs)	// ? Coin Lockout + Video Registers
	AM_RANGE(0x600000, 0x600003) AM_READ(seta_dsw_r)				// DSW
	AM_RANGE(0x700400, 0x700fff) AM_RAM	AM_BASE_GENERIC(paletteram) AM_SIZE(&seta_paletteram_size)	// Palette
	AM_RANGE(0x800000, 0x803fff) AM_RAM_WRITE(seta_vram_0_w) AM_BASE(&seta_vram_0)	// VRAM 0&1
	AM_RANGE(0x880000, 0x883fff) AM_RAM_WRITE(seta_vram_2_w) AM_BASE(&seta_vram_2)	// VRAM 2&3
	AM_RANGE(0x900000, 0x900005) AM_WRITEONLY AM_BASE(&seta_vctrl_0)// VRAM 0&1 Ctrl
	AM_RANGE(0x980000, 0x980005) AM_WRITEONLY AM_BASE(&seta_vctrl_2)// VRAM 2&3 Ctrl
	AM_RANGE(0xa00000, 0xa00607) AM_RAM AM_BASE_GENERIC(spriteram)		// Sprites Y
	AM_RANGE(0xb00000, 0xb03fff) AM_RAM AM_BASE_GENERIC(spriteram2)		// Sprites Code + X + Attr
	AM_RANGE(0xc00000, 0xc00001) AM_WRITE(utoukond_soundlatch_w)	// To Sound CPU (cause an IRQ)
	AM_RANGE(0xe00000, 0xe00001) AM_WRITENOP						// ? ack
ADDRESS_MAP_END

/***************************************************************************
                                Pairs Love
***************************************************************************/

static UINT16 pairslove_protram[0x200];
static UINT16 pairslove_protram_old[0x200];

static READ16_HANDLER( pairlove_prot_r )
{
	int retdata;
	retdata = pairslove_protram[offset];
	//mame_printf_debug("pairs love protection? read %06x %04x %04x\n",cpu_get_pc(space->cpu), offset,retdata);
	pairslove_protram[offset]=pairslove_protram_old[offset];
	return retdata;
}

static WRITE16_HANDLER( pairlove_prot_w )
{
//  mame_printf_debug("pairs love protection? write %06x %04x %04x\n",cpu_get_pc(space->cpu), offset,data);
	pairslove_protram_old[offset]=pairslove_protram[offset];
	pairslove_protram[offset]=data;
}

static ADDRESS_MAP_START( pairlove_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM								// ROM
	AM_RANGE(0x100000, 0x100001) AM_WRITENOP						// ? 1 (start of interrupts, main loop: watchdog?)
	AM_RANGE(0x200000, 0x200001) AM_WRITENOP						// ? 0/1 (IRQ acknowledge?)
	AM_RANGE(0x300000, 0x300003) AM_READ(seta_dsw_r)				// DSW
	AM_RANGE(0x400000, 0x400001) AM_RAM_WRITE(seta_vregs_w) AM_BASE(&seta_vregs)	// Coin Lockout + Sound Enable (bit 4?)
	AM_RANGE(0x500000, 0x500001) AM_READ_PORT("P1")					// P1
	AM_RANGE(0x500002, 0x500003) AM_READ_PORT("P2")					// P2
	AM_RANGE(0x500004, 0x500005) AM_READ_PORT("COINS")				// Coins
	AM_RANGE(0x900000, 0x9001ff) AM_READWRITE(pairlove_prot_r,pairlove_prot_w)
	AM_RANGE(0xa00000, 0xa03fff) AM_DEVREADWRITE("x1snd", seta_sound_word_r,seta_sound_word_w)	// Sound
	AM_RANGE(0xb00000, 0xb00fff) AM_RAM AM_BASE_GENERIC(paletteram) AM_SIZE(&seta_paletteram_size)	// Palette
	AM_RANGE(0xc00000, 0xc03fff) AM_RAM AM_BASE_GENERIC(spriteram2)		// Sprites Code + X + Attr
	AM_RANGE(0xd00000, 0xd00001) AM_RAM								// ? 0x4000
	AM_RANGE(0xe00000, 0xe00607) AM_RAM AM_BASE_GENERIC(spriteram)		// Sprites Y
	AM_RANGE(0xf00000, 0xf0ffff) AM_RAM								// RAM
ADDRESS_MAP_END


/***************************************************************************
                            Crazy Fight
***************************************************************************/

static ADDRESS_MAP_START( crazyfgt_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x400000, 0x40ffff) AM_RAM
	AM_RANGE(0x610000, 0x610001) AM_READ_PORT("COINS")
	AM_RANGE(0x610002, 0x610003) AM_READ_PORT("UNK")
	AM_RANGE(0x610004, 0x610005) AM_READ_PORT("INPUT")
	AM_RANGE(0x610006, 0x610007) AM_WRITENOP
	AM_RANGE(0x620000, 0x620003) AM_WRITENOP	// protection
	AM_RANGE(0x630000, 0x630003) AM_READ(seta_dsw_r)
	AM_RANGE(0x640400, 0x640fff) AM_WRITEONLY AM_BASE_GENERIC(paletteram) AM_SIZE(&seta_paletteram_size)	// Palette
	AM_RANGE(0x650000, 0x650003) AM_DEVWRITE8("ymsnd", ym3812_w, 0x00ff)
	AM_RANGE(0x658000, 0x658001) AM_DEVWRITE8("oki", okim6295_w, 0x00ff)
	AM_RANGE(0x670000, 0x670001) AM_READNOP		// watchdog?
	AM_RANGE(0x800000, 0x803fff) AM_WRITE(seta_vram_2_w) AM_BASE(&seta_vram_2) // VRAM 2
	AM_RANGE(0x880000, 0x883fff) AM_WRITE(seta_vram_0_w) AM_BASE(&seta_vram_0) // VRAM 0
	AM_RANGE(0x900000, 0x900005) AM_RAM AM_BASE(&seta_vctrl_2)	// VRAM 2&3 Ctrl
	AM_RANGE(0x980000, 0x980005) AM_RAM AM_BASE(&seta_vctrl_0)	// VRAM 0&1 Ctrl
	AM_RANGE(0xa00000, 0xa00607) AM_RAM AM_BASE_GENERIC(spriteram)	// Sprites Y
	AM_RANGE(0xa80000, 0xa80001) AM_WRITENOP	// ? 0x4000
	AM_RANGE(0xb00000, 0xb03fff) AM_RAM AM_BASE_GENERIC(spriteram2)	// Sprites Code + X + Attr
ADDRESS_MAP_END


/***************************************************************************
                             International Toote
***************************************************************************/

static READ16_HANDLER( inttoote_dsw_r )
{
	int shift = offset * 4;
	return	((((input_port_read(space->machine, "DSW1") >> shift)       & 0xf)) << 0) |
			((((input_port_read(space->machine, "DSW2_3") >> shift)     & 0xf)) << 4) |
			((((input_port_read(space->machine, "DSW2_3") >> (shift+8)) & 0xf)) << 8) ;
}

static UINT16 *inttoote_key_select;
static READ16_HANDLER( inttoote_key_r )
{
	switch( *inttoote_key_select )
	{
		case 0x08:	return input_port_read(space->machine, "BET0");
		case 0x10:	return input_port_read(space->machine, "BET1");
		case 0x20:	return input_port_read(space->machine, "BET2");
		case 0x40:	return input_port_read(space->machine, "BET3");
		case 0x80:	return input_port_read(space->machine, "BET4");
	}
	logerror("%06X: unknown read, select = %04x\n",cpu_get_pc(space->cpu),*inttoote_key_select);
	return 0xffff;
}

static UINT16 *inttoote_700000;
static READ16_HANDLER( inttoote_700000_r )
{
	return inttoote_700000[offset] & 0x3f;
}

static ADDRESS_MAP_START( inttoote_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM	// ROM (up to 2MB)

	AM_RANGE(0x200000, 0x200001) AM_RAM_READ(inttoote_key_r) AM_BASE(&inttoote_key_select)
	AM_RANGE(0x200002, 0x200003) AM_READ_PORT("P1")
	AM_RANGE(0x200010, 0x200011) AM_READ_PORT("P2") AM_WRITENOP

	AM_RANGE(0x300000, 0x300001) AM_WRITE(watchdog_reset16_w)	// Watchdog

	AM_RANGE(0x300010, 0x300011) AM_WRITENOP	// lev1 ack
	AM_RANGE(0x300020, 0x300021) AM_WRITENOP	// lev2 ack
	AM_RANGE(0x300040, 0x300041) AM_WRITENOP	// lev4 ack
	AM_RANGE(0x300060, 0x300061) AM_WRITENOP	// lev6 ack

	AM_RANGE(0x500000, 0x500003) AM_READ(inttoote_dsw_r)	// DSW x 3

	AM_RANGE(0x700000, 0x700101) AM_RAM_READ(inttoote_700000_r) AM_BASE(&inttoote_700000)

	AM_RANGE(0x800000, 0x80001f) AM_DEVREADWRITE8("rtc", msm6242_r, msm6242_w, 0x00ff)   // 6242RTC

	AM_RANGE(0x900000, 0x903fff) AM_DEVREADWRITE( "x1snd", seta_sound_word_r, seta_sound_word_w		)	// Sound

	AM_RANGE(0xa00000, 0xa00005) AM_WRITEONLY AM_BASE(&seta_vctrl_0		)	// VRAM 0&1 Ctrl
	AM_RANGE(0xb00000, 0xb03fff) AM_RAM_WRITE(seta_vram_0_w) AM_BASE(&seta_vram_0	)	// VRAM 0&1

	AM_RANGE(0xc00000, 0xc00001) AM_RAM		// ? 0x4000

	AM_RANGE(0xd00000, 0xd00607) AM_RAM	AM_BASE_GENERIC(spriteram	)	// Sprites Y
	AM_RANGE(0xe00000, 0xe03fff) AM_RAM	AM_BASE_GENERIC(spriteram2		)	// Sprites Code + X + Attr

	AM_RANGE(0xffc000, 0xffffff) AM_RAM	// RAM
ADDRESS_MAP_END

static UINT8 jockeyc_key_select;

static READ16_HANDLER( jockeyc_mux_r )
{
	switch( jockeyc_key_select )
	{
		case 0x08:	return input_port_read(space->machine, "BET0");
		case 0x10:	return input_port_read(space->machine, "BET1");
		case 0x20:	return input_port_read(space->machine, "BET2");
		case 0x40:	return input_port_read(space->machine, "BET3");
		case 0x80:	return input_port_read(space->machine, "BET4");
	}

	return 0xffff;
}

static WRITE16_HANDLER( jockeyc_mux_w )
{
	/* other bits used too */
	jockeyc_key_select = data & 0xf8;
}

static READ16_HANDLER( unk_r )
{
	return 0xffff;//mame_rand(space->machine);
}

/* same as International Toote but without the protection and different RTC hook-up */
static ADDRESS_MAP_START( jockeyc_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM	// ROM (up to 2MB)

	AM_RANGE(0x200000, 0x200001) AM_READWRITE(jockeyc_mux_r,jockeyc_mux_w)
	AM_RANGE(0x200002, 0x200003) AM_READ_PORT("P1")
	AM_RANGE(0x200010, 0x200011) AM_READ_PORT("P2") AM_WRITENOP

	AM_RANGE(0x300000, 0x300001) AM_WRITE(watchdog_reset16_w)	// Watchdog

	AM_RANGE(0x300002, 0x300003) AM_WRITENOP

	AM_RANGE(0x300010, 0x300011) AM_WRITENOP	// lev1 ack
	AM_RANGE(0x300020, 0x300021) AM_WRITENOP	// lev2 ack
	AM_RANGE(0x300040, 0x300041) AM_WRITENOP	// lev4 ack
	AM_RANGE(0x300060, 0x300061) AM_WRITENOP	// lev6 ack

	AM_RANGE(0x500000, 0x500003) AM_READ(inttoote_dsw_r)	// DSW x 3
	AM_RANGE(0x600000, 0x600003) AM_READ(unk_r)

	AM_RANGE(0x800000, 0x80001f) AM_DEVREADWRITE8("rtc", msm6242_r, msm6242_w, 0x00ff)   // 6242RTC

	AM_RANGE(0x900000, 0x903fff) AM_DEVREADWRITE( "x1snd", seta_sound_word_r, seta_sound_word_w)	// Sound

	AM_RANGE(0xa00000, 0xa00005) AM_WRITEONLY AM_BASE(&seta_vctrl_0)	// VRAM 0&1 Ctrl
	AM_RANGE(0xb00000, 0xb03fff) AM_RAM_WRITE(seta_vram_0_w) AM_BASE(&seta_vram_0)	// VRAM 0&1
	AM_RANGE(0xb04000, 0xb0ffff) AM_WRITENOP // likely left-over

	AM_RANGE(0xc00000, 0xc00001) AM_RAM		// ? 0x4000

	AM_RANGE(0xd00000, 0xd00607) AM_RAM	AM_BASE_GENERIC(spriteram)	// Sprites Y
	AM_RANGE(0xe00000, 0xe03fff) AM_RAM	AM_BASE_GENERIC(spriteram2)	// Sprites Code + X + Attr

	AM_RANGE(0xffc000, 0xffffff) AM_RAM	// RAM
ADDRESS_MAP_END

/***************************************************************************


                                Sub / Sound CPU


***************************************************************************/

static WRITE8_HANDLER( sub_bankswitch_w )
{
	UINT8 *rom = memory_region(space->machine, "sub");
	int bank = data >> 4;

	memory_set_bankptr(space->machine, "bank1", &rom[bank * 0x4000 + 0xc000]);
}

static WRITE8_HANDLER( sub_bankswitch_lockout_w )
{
	sub_bankswitch_w(space,offset,data);
	seta_coin_lockout_w(space->machine, data);
}


/***************************************************************************
                                Thundercade
***************************************************************************/

static READ8_HANDLER( ff_r )	{return 0xff;}

static ADDRESS_MAP_START( tndrcade_sub_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x01ff) AM_RAM 							// RAM
	AM_RANGE(0x0800, 0x0800) AM_READ(ff_r)						// ? (bits 0/1/2/3: 1 -> do test 0-ff/100-1e0/5001-57ff/banked rom)
//  AM_RANGE(0x0800, 0x0800) AM_READ(soundlatch_r)              //
//  AM_RANGE(0x0801, 0x0801) AM_READ(soundlatch2_r)             //
	AM_RANGE(0x1000, 0x1000) AM_READ_PORT("P1")					// P1
	AM_RANGE(0x1000, 0x1000) AM_WRITE(sub_bankswitch_lockout_w)	// ROM Bank + Coin Lockout
	AM_RANGE(0x1001, 0x1001) AM_READ_PORT("P2")					// P2
	AM_RANGE(0x1002, 0x1002) AM_READ_PORT("COINS")				// Coins
	AM_RANGE(0x2000, 0x2001) AM_DEVREADWRITE("ym1", ym2203_r,ym2203_w)
	AM_RANGE(0x3000, 0x3001) AM_DEVWRITE("ym2", ym3812_w)
	AM_RANGE(0x5000, 0x57ff) AM_RAM	 AM_BASE(&sharedram)		// Shared RAM
	AM_RANGE(0x6000, 0x7fff) AM_ROM								// ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")						// Banked ROM
	AM_RANGE(0xc000, 0xffff) AM_ROM								// ROM
ADDRESS_MAP_END


/***************************************************************************
                                Twin Eagle
***************************************************************************/

static ADDRESS_MAP_START( twineagl_sub_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x01ff) AM_RAM							// RAM
	AM_RANGE(0x0800, 0x0800) AM_READ(soundlatch_r)			//
	AM_RANGE(0x0801, 0x0801) AM_READ(soundlatch2_r)			//
	AM_RANGE(0x1000, 0x1000) AM_READ_PORT("P1")				// P1
	AM_RANGE(0x1000, 0x1000) AM_WRITE(sub_bankswitch_lockout_w)	// ROM Bank + Coin Lockout
	AM_RANGE(0x1001, 0x1001) AM_READ_PORT("P2")				// P2
	AM_RANGE(0x1002, 0x1002) AM_READ_PORT("COINS")			// Coins
	AM_RANGE(0x5000, 0x57ff) AM_RAM AM_BASE(&sharedram)		// Shared RAM
	AM_RANGE(0x7000, 0x7fff) AM_ROM							// ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")					// Banked ROM
	AM_RANGE(0xc000, 0xffff) AM_ROM							// ROM
ADDRESS_MAP_END


/***************************************************************************
                                DownTown
***************************************************************************/

static READ8_HANDLER( downtown_ip_r )
{
	int dir1 = input_port_read(space->machine, "ROT1");	// analog port
	int dir2 = input_port_read(space->machine, "ROT2");	// analog port

	dir1 = (~ (0x800 >> dir1)) & 0xfff;
	dir2 = (~ (0x800 >> dir2)) & 0xfff;

	switch (offset)
	{
		case 0:	return (input_port_read(space->machine, "COINS") & 0xf0) + (dir1 >> 8);	// upper 4 bits of p1 rotation + coins
		case 1:	return (dir1 & 0xff);					// lower 8 bits of p1 rotation
		case 2:	return input_port_read(space->machine, "P1");	// p1
		case 3:	return 0xff;							// ?
		case 4:	return (dir2 >> 8);						// upper 4 bits of p2 rotation + ?
		case 5:	return (dir2 & 0xff);					// lower 8 bits of p2 rotation
		case 6:	return input_port_read(space->machine, "P2");	// p2
		case 7:	return 0xff;							// ?
	}

	return 0;
}

static ADDRESS_MAP_START( downtown_sub_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x01ff) AM_RAM							// RAM
	AM_RANGE(0x0800, 0x0800) AM_READ(soundlatch_r)			//
	AM_RANGE(0x0801, 0x0801) AM_READ(soundlatch2_r)			//
	AM_RANGE(0x1000, 0x1007) AM_READ(downtown_ip_r)			// Input Ports
	AM_RANGE(0x1000, 0x1000) AM_WRITE(sub_bankswitch_lockout_w)	// ROM Bank + Coin Lockout
	AM_RANGE(0x5000, 0x57ff) AM_RAM AM_BASE(&sharedram)		// Shared RAM
	AM_RANGE(0x7000, 0x7fff) AM_ROM							// ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")					// Banked ROM
	AM_RANGE(0xc000, 0xffff) AM_ROM							// ROM
ADDRESS_MAP_END


/***************************************************************************
                        Caliber 50 / U.S. Classic
***************************************************************************/

static MACHINE_RESET(calibr50)
{
	const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);
	sub_bankswitch_w(space, 0, 0);
}

static WRITE8_HANDLER( calibr50_soundlatch2_w )
{
	soundlatch2_w(space,0,data);
	cpu_spinuntil_time(space->cpu, ATTOTIME_IN_USEC(50));	// Allow the other cpu to reply
}

static ADDRESS_MAP_START( calibr50_sub_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_DEVREADWRITE("x1snd", seta_sound_r,seta_sound_w)	// Sound
	AM_RANGE(0x4000, 0x4000) AM_READ(soundlatch_r)				// From Main CPU
	AM_RANGE(0x4000, 0x4000) AM_WRITE(sub_bankswitch_w)			// Bankswitching
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")						// Banked ROM
	AM_RANGE(0xc000, 0xffff) AM_ROM								// ROM
	AM_RANGE(0xc000, 0xc000) AM_WRITE(calibr50_soundlatch2_w)	// To Main CPU
ADDRESS_MAP_END


/***************************************************************************
                                Meta Fox
***************************************************************************/

static ADDRESS_MAP_START( metafox_sub_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x01ff) AM_RAM							// RAM
	AM_RANGE(0x0800, 0x0800) AM_READ(soundlatch_r)			//
	AM_RANGE(0x0801, 0x0801) AM_READ(soundlatch2_r)			//
	AM_RANGE(0x1000, 0x1000) AM_READ_PORT("COINS")			// Coins
	AM_RANGE(0x1000, 0x1000) AM_WRITE(sub_bankswitch_lockout_w)	// ROM Bank + Coin Lockout
	AM_RANGE(0x1002, 0x1002) AM_READ_PORT("P1")				// P1
//  AM_RANGE(0x1004, 0x1004) AM_READNOP                     // ?
	AM_RANGE(0x1006, 0x1006) AM_READ_PORT("P2")				// P2
	AM_RANGE(0x5000, 0x57ff) AM_RAM AM_BASE(&sharedram)		// Shared RAM
	AM_RANGE(0x7000, 0x7fff) AM_ROM							// ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")					// Banked ROM
	AM_RANGE(0xc000, 0xffff) AM_ROM							// ROM
ADDRESS_MAP_END


/***************************************************************************
                            Ultra Toukon Densetsu
***************************************************************************/

static ADDRESS_MAP_START( utoukond_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xdfff) AM_ROM
	AM_RANGE(0xe000, 0xefff) AM_RAM
	AM_RANGE(0xf000, 0xffff) AM_DEVREADWRITE("x1snd", seta_sound_r,seta_sound_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( utoukond_sound_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("ymsnd", ym3438_r, ym3438_w)
	AM_RANGE(0x80, 0x80) AM_WRITENOP //?
	AM_RANGE(0xc0, 0xc0) AM_READ(soundlatch_r)
ADDRESS_MAP_END


/***************************************************************************


                                Input Ports


***************************************************************************/

#define	JOY_TYPE1_1BUTTON(_n_) \
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT	) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP	) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN	) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1		) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN						) \
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN						) \
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START##_n_					)

#define	JOY_TYPE1_2BUTTONS(_n_) \
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT	) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP	) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN	) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1		) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2		) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN						) \
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START##_n_					)

#define	JOY_TYPE1_3BUTTONS(_n_) \
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT	) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP	) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN	) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1		) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2		) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3		) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START##_n_					)


#define	JOY_TYPE2_1BUTTON(_n_) \
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP	) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN	) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT	) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1		) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN						) \
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN						) \
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START##_n_					)

#define	JOY_TYPE2_2BUTTONS(_n_) \
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP	) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN	) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT	) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1		) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2		) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN						) \
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START##_n_					)

#define	JOY_TYPE2_3BUTTONS(_n_) \
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP	) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN	) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT	) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1		) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2		) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3		) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START##_n_					)


#define JOY_ROTATION(_n_, _left_, _right_ ) \
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_PLAYER(_n_) PORT_SENSITIVITY(15) PORT_KEYDELTA(15) PORT_CODE_DEC(KEYCODE_##_left_) PORT_CODE_INC(KEYCODE_##_right_)



/***************************************************************************
                                Arbalester
***************************************************************************/

static INPUT_PORTS_START( arbalest )
	PORT_START("P1")	// Player 1
	JOY_TYPE2_2BUTTONS(1)

	PORT_START("P2")	// Player 2
	JOY_TYPE2_2BUTTONS(2)

	PORT_START("COINS")	// Coins
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_TILT     )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)

	PORT_START("DSW")	// 2 DSWs - $600001 & 3.b
	PORT_DIPNAME( 0x4001, 0x0000, "Licensed To" )
	PORT_DIPSETTING(      0x0000, "Taito" )
//  PORT_DIPSETTING(      0x4000, "Taito" )
	PORT_DIPSETTING(      0x4001, "Romstar" )
	PORT_DIPSETTING(      0x0001, "Jordan" )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE( 0x0004, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_2C ) )

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Harder ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(      0x0c00, "Never" )
	PORT_DIPSETTING(      0x0800, "300k Only" )
	PORT_DIPSETTING(      0x0400, "600k Only" )
	PORT_DIPSETTING(      0x0000, "300k & 600k" )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x1000, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x3000, "3" )
	PORT_DIPSETTING(      0x2000, "5" )
//                        0x4000  License (see first dsw)
	PORT_DIPNAME( 0x8000, 0x8000, "Coinage Type" )	// not supported
	PORT_DIPSETTING(      0x8000, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
INPUT_PORTS_END


/***************************************************************************
                                Athena no Hatena?
***************************************************************************/

static INPUT_PORTS_START( atehate )
	PORT_START("P1")	// Player 1
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1  )

	PORT_START("P2")	// Player 2
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2  )

	PORT_START("COINS")	// Coins
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_UNKNOWN ) // 4 Bits Called "Cut DSW"
	PORT_BIT( 0x0020, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("DSW")	// 2 DSWs - $e00001 & 3.b
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(      0x0008, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0080, "SW2:8" ) /* Listed as "Unused" */

	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0200, IP_ACTIVE_LOW, "SW1:2" )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy )    )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal )  )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard )    )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x3000, "3" )
	PORT_DIPSETTING(      0x1000, "4" )
	PORT_DIPSETTING(      0x2000, "5" )
	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0xc000, DEF_STR( None ) )
	PORT_DIPSETTING(      0x0000, "20K Only" )
	PORT_DIPSETTING(      0x8000, "20K, Every 30K" )
	PORT_DIPSETTING(      0x4000, "30K, Every 40K" )
INPUT_PORTS_END


/***************************************************************************
                                Blandia
***************************************************************************/

static INPUT_PORTS_START( blandia )
	PORT_START("P1")	//Player 1 - $400000.w
	JOY_TYPE1_3BUTTONS(1)

	PORT_START("P2")	//Player 2 - $400002.w
	JOY_TYPE1_3BUTTONS(2)

	PORT_START("COINS")	//Coins - $400004.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT     )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN  )

	PORT_START("DSW")	//2 DSWs - $600001 & 3.b
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "Coinage Type" ) PORT_DIPLOCATION("SW2:2")	// not supported
	PORT_DIPSETTING(      0x0002, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPNAME( 0x001c, 0x001c, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW2:3,4,5")
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x001c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(      0x0014, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x00e0, 0x00e0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW2:6,7,8")
	PORT_DIPSETTING(      0x00e0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 2C_4C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0080, "3 Coins/7 Credits" )
	PORT_DIPSETTING(      0x0020, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 2C_6C ) )
	PORT_DIPSETTING(      0x00a0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0200, "1" )
	PORT_DIPSETTING(      0x0300, "2" )
	PORT_DIPSETTING(      0x0100, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy )    )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal )  )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard )    )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x1000, 0x1000, "2 Player Game" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x1000, "2 Credits" )
	PORT_DIPSETTING(      0x0000, "1 Credit"  )
	PORT_DIPNAME( 0x2000, 0x2000, "Continue" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x2000, "1 Credit" )
	PORT_DIPSETTING(      0x0000, "1 Coin"   )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x8000, IP_ACTIVE_LOW, "SW1:8" )
INPUT_PORTS_END



/***************************************************************************
                                Block Carnival
***************************************************************************/

static INPUT_PORTS_START( blockcar )
	PORT_START("P1")	//Player 1 - $500001.b
	JOY_TYPE1_2BUTTONS(1)	// button2 = speed up

	PORT_START("P2")	//Player 2 - $500003.b
	JOY_TYPE1_2BUTTONS(2)

	PORT_START("COINS")	//Coins + DSW - $500005.b
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT     )
	PORT_DIPNAME( 0x0010, 0x0000, "Title" )		/* This is a jumper pad */
	PORT_DIPSETTING(      0x0010, "Thunder & Lightning 2" )
	PORT_DIPSETTING(      0x0000, "Block Carnival" )

	PORT_START("DSW")	//2 DSWs - $300003 & 1.b
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy )    )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal )  )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard )    )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x000c, "20K, Every 50K" )
	PORT_DIPSETTING(      0x0004, "20K, Every 70K" )
	PORT_DIPSETTING(      0x0008, "30K, Every 60K" )
	PORT_DIPSETTING(      0x0000, "30K, Every 90K" )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0030, "2" )
	PORT_DIPSETTING(      0x0020, "3" )
	PORT_DIPSETTING(      0x0010, "4" )
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "SW2:7" ) /* Listed as "Unused" */
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW2:8") /* Listed as "Unused" */
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPUNUSED_DIPLOC( 0x0100, 0x0100, "SW1:1" ) /* Listed as "Unused" */
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, "Unknown 1-3" ) PORT_DIPLOCATION("SW1:4")	// service mode, according to a file in the archive
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x1000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0x4000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 1C_2C ) )
INPUT_PORTS_END



/***************************************************************************
                                Caliber 50
***************************************************************************/

static INPUT_PORTS_START( calibr50 )
	PORT_START("P1")	//Player 1
	JOY_TYPE2_2BUTTONS(1)

	PORT_START("P2")	//Player 2
	JOY_TYPE2_2BUTTONS(2)

	PORT_START("COINS")	//Coins
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_TILT     )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)

	PORT_START("DSW")	//2 DSWs - $600001 & 3.b
	PORT_DIPNAME( 0x4001, 0x4000, "Licensed To" )
	PORT_DIPSETTING(      0x0001, "Romstar"       )
	PORT_DIPSETTING(      0x4001, "Taito America" )
	PORT_DIPSETTING(      0x4000, "Taito"         )
	PORT_DIPSETTING(      0x0000, "None (Japan)"  )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE( 0x0004, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_2C ) )

	PORT_DIPNAME( 0x0300, 0x0100, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Easiest ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x0400, 0x0400, "Score Digits" )
	PORT_DIPSETTING(      0x0400, "7" )
	PORT_DIPSETTING(      0x0000, "3" )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0800, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x1000, 0x1000, "Display Score" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, "Erase Backup Ram" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( On ) )
	//                    0x4000  Country / License (see first dsw)
	PORT_DIPNAME( 0x8000, 0x8000, "Unknown 2-7" )	/* manual: "Don't Touch" */
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("ROT1")	// Rotation Player 1
	PORT_BIT( 0xfff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(15) PORT_KEYDELTA(15) PORT_CODE_DEC(KEYCODE_Z) PORT_CODE_INC(KEYCODE_X)

	PORT_START("ROT2")	// Rotation Player 2
	PORT_BIT( 0xfff, 0x00, IPT_DIAL ) PORT_PLAYER(2) PORT_SENSITIVITY(15) PORT_KEYDELTA(15) PORT_CODE_DEC(KEYCODE_N) PORT_CODE_INC(KEYCODE_M)
INPUT_PORTS_END

/***************************************************************************
                                Daioh
***************************************************************************/

static INPUT_PORTS_START( daioh )
	PORT_START("P1")
	JOY_TYPE1_3BUTTONS(1)

	PORT_START("P2")
	JOY_TYPE1_3BUTTONS(2)

	PORT_START("COINS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	/* These are NOT Dip Switches but jumpers */
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unused ) )	// JP9
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unused ) )	// JP8
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unused ) )	// JP7
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Country" )			// JP6
	PORT_DIPSETTING(      0x0080, "USA (6 buttons)" )
	PORT_DIPSETTING(      0x0000, "Japan (2 buttons)" )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(      0x0008, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, "Auto Shot" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0200, IP_ACTIVE_LOW, "SW1:2" )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x1000, "2" )
	PORT_DIPSETTING(      0x3000, "3" )
	PORT_DIPSETTING(      0x2000, "5" )
	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0x8000, "300k and every 800k" )
	PORT_DIPSETTING(      0xc000, "500k and every 1000k" )
	PORT_DIPSETTING(      0x4000, "800k and 2000k only" )
	PORT_DIPSETTING(      0x0000, "1000k Only" )

	PORT_START("EXTRA")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

/***************************************************************************
                                Dragon Unit
***************************************************************************/

static INPUT_PORTS_START( drgnunit )
	PORT_START("P1")	//Player 1
	JOY_TYPE1_3BUTTONS(1)

	PORT_START("P2")	//Player 2
	JOY_TYPE1_3BUTTONS(2)

	PORT_START("COINS")	//Coins
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT     )
	PORT_DIPNAME( 0x0010, 0x0010, "Coinage Type" ) // not supported
	PORT_DIPSETTING(      0x0010, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPNAME( 0x0020, 0x0020, "Title" )
	PORT_DIPSETTING(      0x0020, "Dragon Unit" )
	PORT_DIPSETTING(      0x0000, "Castle of Dragon" )
	PORT_DIPNAME( 0x00c0, 0x00c0, "(C) / License" )
	PORT_DIPSETTING(      0x00c0, "Athena (Japan)" )
	PORT_DIPSETTING(      0x0080, "Athena / Taito (Japan)" )
	PORT_DIPSETTING(      0x0040, "Seta USA / Taito America" )
	PORT_DIPSETTING(      0x0000, "Seta USA / Romstar" )

	PORT_START("DSW")	//2 DSWs - $600001 & 3.b
	PORT_DIPNAME( 0x0003, 0x0002, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0003, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0008, "150K, Every 300K" )
	PORT_DIPSETTING(      0x000c, "200K, Every 400K" )
	PORT_DIPSETTING(      0x0004, "300K, Every 500K" )
	PORT_DIPSETTING(      0x0000, "400K Only" )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0010, "2" )
	PORT_DIPSETTING(      0x0030, "3" )
	PORT_DIPSETTING(      0x0020, "5" )
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "SW1:7" )	/* Labeled "Don't Touch" in manual */
	PORT_DIPNAME( 0x0080, 0x0080, "Unknown 1-8*" ) PORT_DIPLOCATION("SW1:8")	/* Labeled "Don't Touch" in manual (seems to be used though) */
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPUNUSED_DIPLOC( 0x0100, 0x0100, "SW2:1" )	/* Labeled "Don't Touch" in manual */
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0400, "1 of 4 Scenes" )
	PORT_DIPSETTING(      0x0000, "1 of 8 Scenes" )
	PORT_SERVICE_DIPLOC(  0x0800, IP_ACTIVE_LOW, "SW2:4" )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x1000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(      0x4000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 1C_2C ) )
INPUT_PORTS_END


/***************************************************************************
                                Seta Roulette
***************************************************************************/

static INPUT_PORTS_START( setaroul )
INPUT_PORTS_END


/***************************************************************************
                                DownTown
***************************************************************************/

static INPUT_PORTS_START( downtown )
	PORT_START("P1")	//Player 1
	JOY_TYPE2_2BUTTONS(1)

	PORT_START("P2")	//Player 2
	JOY_TYPE2_2BUTTONS(2)

	PORT_START("COINS")	//Coins
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_TILT     )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)

	PORT_START("DSW")	//2 DSWs - $600001 & 3.b
	PORT_DIPNAME( 0x0001, 0x0000, "Sales" )	PORT_DIPLOCATION("SW1:1")		/* Manual for USA version says "Always Off" */
	PORT_DIPSETTING(      0x0001, "Japan Only" )
	PORT_DIPSETTING(      0x0000, DEF_STR( World ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0004, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPNAME( 0x0008, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0x0040, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_2C ) )
// other coinage
#if 0
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
#endif

	PORT_DIPNAME( 0x0300, 0x0100, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0c00, "Never" )
	PORT_DIPSETTING(      0x0800, "50K Only" )
	PORT_DIPSETTING(      0x0400, "100K Only" )
	PORT_DIPSETTING(      0x0000, "50K, Every 150K" )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x1000, "2" )
	PORT_DIPSETTING(      0x3000, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPSETTING(      0x2000, "5" )
	PORT_DIPNAME( 0x4000, 0x0000, "World License" ) PORT_DIPLOCATION("SW2:7") /* Manual for USA version says "Unused" */
	PORT_DIPSETTING(      0x4000, "Romstar" )
	PORT_DIPSETTING(      0x0000, "Taito" )
	PORT_DIPNAME( 0x8000, 0x8000, "Coinage Type" ) PORT_DIPLOCATION("SW2:8") /* Manual for USA version says "Unused", but currently not implemented */
	PORT_DIPSETTING(      0x8000, "1" )
	PORT_DIPSETTING(      0x0000, "2" )

	PORT_START("ROT1")	//Rotation Player 1
	PORT_BIT( 0xff, 0x00, IPT_POSITIONAL ) PORT_POSITIONS(12) PORT_WRAPS PORT_SENSITIVITY(15) PORT_KEYDELTA(1) PORT_CODE_DEC(KEYCODE_Z) PORT_CODE_INC(KEYCODE_X) PORT_FULL_TURN_COUNT(12)

	PORT_START("ROT2")	//Rotation Player 2
	PORT_BIT( 0xff, 0x00, IPT_POSITIONAL ) PORT_POSITIONS(12) PORT_WRAPS PORT_SENSITIVITY(15) PORT_KEYDELTA(1) PORT_CODE_DEC(KEYCODE_N) PORT_CODE_INC(KEYCODE_M) PORT_PLAYER(2) PORT_FULL_TURN_COUNT(12)
INPUT_PORTS_END



/***************************************************************************
                                Eight Force
***************************************************************************/

static INPUT_PORTS_START( eightfrc )
	PORT_START("P1")	//Player 1
	JOY_TYPE1_2BUTTONS(1)

	PORT_START("P2")	//Player 2
	JOY_TYPE1_2BUTTONS(2)

	PORT_START("COINS")	//Coins
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT     )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")	//2 DSWs - $600001 & 3.b
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(      0x0004, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(      0x0020, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Shared Credits" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Credits To Start" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, "1" )
	PORT_DIPSETTING(      0x0000, "2" )

	PORT_SERVICE_DIPLOC(  0x0100, IP_ACTIVE_LOW, "SW1:1" )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1800, 0x1800, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x6000, 0x6000, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(      0x4000, "2" )
	PORT_DIPSETTING(      0x6000, "3" )
	PORT_DIPSETTING(      0x2000, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Language ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( English ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Japanese ) )
INPUT_PORTS_END



/***************************************************************************
                                Extreme Downhill
***************************************************************************/

static INPUT_PORTS_START( extdwnhl )
	PORT_START("P1") //Player 1
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1  )

	PORT_START("P2") //Player 2
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2  )

	PORT_START("COINS") //Coins
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE )	// "test"
	/* These are NOT Dip Switches but jumpers */
	PORT_DIPNAME( 0x0030, 0x0030, "Country" )
	PORT_DIPSETTING(      0x0030, DEF_STR( World ) )
//  PORT_DIPSETTING(      0x0020, DEF_STR( World ) )    // duplicated settings
	PORT_DIPSETTING(      0x0010, DEF_STR( USA ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Japan ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW") //2 DSWs - $400009 & b.b
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Controls ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0040, "2" )
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW2:8" )

	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0500, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x3800, 0x3800, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(      0x2800, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x3800, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Continue Coin" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x4000, "Normal: Start 1C / Continue 1C" )
	PORT_DIPSETTING(      0x0000, "Half Continue: Start 2C / Continue 1C" )
	PORT_DIPNAME( 0x8000, 0x8000, "Game Mode" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x8000, "Finals Only" )
	PORT_DIPSETTING(      0x0000, "Semi-Finals & Finals" )
INPUT_PORTS_END



/***************************************************************************
                                Gundhara
***************************************************************************/

static INPUT_PORTS_START( gundhara )
	PORT_START("P1") //Player 1
	JOY_TYPE1_3BUTTONS(1)

	PORT_START("P2") //Player 2
	JOY_TYPE1_3BUTTONS(2)

	PORT_START("COINS") //Coins
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0008, IP_ACTIVE_LOW)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN  )

	PORT_START("DSW") //2 DSWs - $600001 & 3.b
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
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Language ) ) PORT_DIPLOCATION("SW2:7,8") /* Yes, the manual shows it takes both switches */
	PORT_DIPSETTING(      0x00c0, DEF_STR( Japanese ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( English ) )

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0800, "1" )
	PORT_DIPSETTING(      0x0c00, "2" )
	PORT_DIPSETTING(      0x0400, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x3000, "200K" )
	PORT_DIPSETTING(      0x2000, "200K, Every 200K" )
	PORT_DIPSETTING(      0x1000, "400K" )
	PORT_DIPSETTING(      0x0000, DEF_STR( None ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x8000, IP_ACTIVE_LOW, "SW1:8" )
INPUT_PORTS_END

/***************************************************************************
                                Zombie Raid
***************************************************************************/

static INPUT_PORTS_START( zombraid )
	PORT_START("P1") //Player 1
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1  ) PORT_PLAYER(1)	PORT_NAME("P1 Trigger")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2  ) PORT_PLAYER(1)	PORT_NAME("P1 Reload")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1   )

	PORT_START("P2") //Player 2
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1  ) PORT_PLAYER(2)	PORT_NAME("P2 Trigger")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2  ) PORT_PLAYER(2)	PORT_NAME("P2 Reload")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2   )

	PORT_START("COINS") //Coins
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN  )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0001, 0x0001, "Vertical Screen Flip" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0000, "Horizontal Screen Flip" ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0018, 0x0018, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Easy )    )
	PORT_DIPSETTING(      0x0018, DEF_STR( Normal )  )
	PORT_DIPSETTING(      0x0008, DEF_STR( Hard )    )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "SW2:7" ) /* Listed as "Unused" */
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW2:8" )

	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0500, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x3800, 0x3800, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(      0x2800, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x3800, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x4000, 0x4000, "2 Coins to Start, 1 to Continue" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
	PORT_DIPUNUSED_DIPLOC( 0x8000, 0x8000, "SW1:8" ) /* Listed as "Unused" */

	/* The gun calibration defaults to: left=0xc0, right=0x40, top=0x48, bottom=0xa8 */
	/* The user calibrated values are lost each time MAME starts, so the gun always needs to be re-calibrated. */
	/* Either NVRAM or battery backed up RAM is not emulated. */
	/* For now it is best to just use a Save State after calibration to remember the setting. */
	PORT_START("GUNX1")   /* Player 1 Gun X       ($f00003) */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, -1, 0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_REVERSE PORT_PLAYER(1)
	PORT_START("GUNY1")   /* Player 1 Gun Y       ($f00003) */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y,  1, 0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("GUNX2")   /* Player 2 Gun X       ($f00003) */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, -1, 0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_REVERSE PORT_PLAYER(2)
	PORT_START("GUNY2")   /* Player 2 Gun Y       ($f00003) */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y,  1, 0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(2)
INPUT_PORTS_END


/***************************************************************************
                                J.J.Squawkers
***************************************************************************/

static INPUT_PORTS_START( jjsquawk )
	PORT_START("P1") //Player 1 - $400000.w
	JOY_TYPE1_2BUTTONS(1)

	PORT_START("P2") //Player 2 - $400002.w
	JOY_TYPE1_2BUTTONS(2)

	PORT_START("COINS") //Coins - $400004.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN  )

	PORT_START("DSW") //2 DSWs - $600001 & 3.b
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(      0x0008, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Unknown 2-8" ) PORT_DIPLOCATION("SW2:8")	// ?? screen related
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0200, IP_ACTIVE_LOW, "SW1:2" )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy )    )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal )  )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard )    )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x3000, 0x2000, "Energy" ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x2000, "2" )	// factory default
	PORT_DIPSETTING(      0x3000, "3" )
	PORT_DIPSETTING(      0x1000, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0x8000, "20K, Every 100K" )	//                          TYPO on manual "20000 200000"
	PORT_DIPSETTING(      0xc000, "50K, Every 200K" )	// manufacturer setting //  TYPO on manual "50000 100000"
	PORT_DIPSETTING(      0x4000, "70K, 200K Only" )
	PORT_DIPSETTING(      0x0000, "100K Only" )
INPUT_PORTS_END

/***************************************************************************
                (Kamen) Masked Riders Club Battle Race
***************************************************************************/

static INPUT_PORTS_START( kamenrid )
	PORT_START("P1") //Player 1
	JOY_TYPE1_2BUTTONS(1)	// BUTTON3 in "test mode" only

	PORT_START("P2") //Player 2
	JOY_TYPE1_2BUTTONS(2)	// BUTTON3 in "test mode" only

	PORT_START("COINS") //Coins
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT     )
	/* These are NOT Dip Switches but jumpers */
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Country" )
	PORT_DIPSETTING(      0x0080, DEF_STR( USA ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Japan ) )

	PORT_START("DSW")	// IN3 - 2 DSWs - $500005 & 7.b
	PORT_SERVICE( 0x0001, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unused ) )		// masked at 0x001682
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unused ) )		// masked at 0x001682
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )			// (displays debug infos)
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unused ) )		// masked at 0x001682
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )			// (unknown effect at 0x00606a, 0x0060de, 0x00650a)
	PORT_DIPNAME( 0x0040, 0x0040, "Intro Music" )		// check code at 0x001792
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0f00, 0x0f00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(      0x0d00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0b00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 8C_3C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0f00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0900, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0e00, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0a00, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

/***************************************************************************
                                Krazy Bowl
***************************************************************************/

#define KRZYBOWL_TRACKBALL(_dir_, _n_ ) \
	PORT_BIT( 0x0fff, 0x0000, IPT_TRACKBALL_##_dir_ ) PORT_PLAYER(_n_) PORT_SENSITIVITY(70) PORT_KEYDELTA(30) PORT_REVERSE

static INPUT_PORTS_START( krzybowl )
	PORT_START("P1") //Player 1
	JOY_TYPE1_3BUTTONS(1)

	PORT_START("P2") //Player 2
	JOY_TYPE1_3BUTTONS(2)

	PORT_START("COINS") //Coins
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("DSW") //2 DSWs - $600001 & 3.b
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Frames" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0010, "10" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Controls ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Trackball ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Joystick ) )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:8" )

	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(      0x0500, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x3800, 0x3800, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(      0x2800, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x3800, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Force Coinage" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_1C ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Unknown 2-8" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("TRACK1_X") //Rotation X Player 1
	KRZYBOWL_TRACKBALL(X,1)

	PORT_START("TRACK1_Y") //Rotation Y Player 1
	KRZYBOWL_TRACKBALL(Y,1)

	PORT_START("TRACK2_X") //Rotation X Player 2
	KRZYBOWL_TRACKBALL(X,2) PORT_REVERSE

	PORT_START("TRACK2_Y") //Rotation Y Player 2
	KRZYBOWL_TRACKBALL(Y,2)
INPUT_PORTS_END


/***************************************************************************
                                Mad Shark
***************************************************************************/

static INPUT_PORTS_START( madshark )
	PORT_START("P1") //Player 1
	JOY_TYPE1_2BUTTONS(1)	// BUTTON3 in "test mode" only

	PORT_START("P2") //Player 2
	JOY_TYPE1_2BUTTONS(2)	// BUTTON3 in "test mode" only

	PORT_START("COINS") //Coins
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT     )
	/* These are NOT Dip Switches but jumpers */
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, "Country" )
	PORT_DIPSETTING(      0x0080, DEF_STR( Japan ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( World ) )

	PORT_START("DSW") //2 DSWs
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0004, 0x0004, "SW2:3" )
	PORT_DIPNAME( 0x0018, 0x0018, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(      0x0018, "1000k" )
	PORT_DIPSETTING(      0x0008, "1000k 2000k" )
	PORT_DIPSETTING(      0x0010, "1500k 3000k" )
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPNAME( 0x0060, 0x0060, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW2:8" )

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0200, "2" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0100, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x1c00, 0x1c00, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:3,4,5")
	PORT_DIPSETTING(      0x0400, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x1c00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x1400, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0xe000, 0xe000, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:6,7,8")
	PORT_DIPSETTING(      0x2000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0xe000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0xa000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_4C ) )
INPUT_PORTS_END


/***************************************************************************
                                Meta Fox
***************************************************************************/

static INPUT_PORTS_START( metafox )
	PORT_START("P1") //Player 1
	JOY_TYPE2_2BUTTONS(1)

	PORT_START("P2") //Player 2
	JOY_TYPE2_2BUTTONS(2)

	PORT_START("COINS") //Coins
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_TILT     )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)

	PORT_START("DSW") //$600001 & 3.b
	PORT_DIPNAME( 0x4001, 0x4001, "Licensed To"    )
	PORT_DIPSETTING(      0x0001, "Jordan"        )
	PORT_DIPSETTING(      0x4001, "Romstar"       )
	PORT_DIPSETTING(      0x4000, "Taito"         )
	PORT_DIPSETTING(      0x0000, "Taito America" )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE( 0x0004, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_2C ) )

	PORT_DIPNAME( 0x0300, 0x0100, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal )  )
	PORT_DIPSETTING(      0x0200, DEF_STR( Easy )    )
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard )    )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c00, 0x0000, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( None ) )
	PORT_DIPSETTING(      0x0008, "60K Only" )
	PORT_DIPSETTING(      0x0000, "60k & 90k" )
	PORT_DIPSETTING(      0x0004, "90K Only" )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x1000, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x3000, "3" )
	PORT_DIPSETTING(      0x2000, "5" )
//  PORT_DIPNAME( 0x4000, 0x4000, "License" )
	PORT_DIPNAME( 0x8000, 0x8000, "Coinage Type" )	// not supported
	PORT_DIPSETTING(      0x8000, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
INPUT_PORTS_END



/***************************************************************************
                            Mobile Suit Gundam
***************************************************************************/


static INPUT_PORTS_START( msgundam )
	PORT_START("P1") //Player 1 - $400000.w
	JOY_TYPE1_2BUTTONS(1)

	PORT_START("P2") //Player 2 - $400002.w
	JOY_TYPE1_2BUTTONS(2)

	PORT_START("COINS") //Coins - $400004.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT     )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Language ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( English ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Japanese ) )

	PORT_START("DSW") //2 DSWs - $600001 & 3.b
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
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Unknown 2-7" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( On ) )
	PORT_DIPNAME( 0x0600, 0x0600, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:2,3")	// unverified, from the manual
	PORT_DIPSETTING(      0x0400, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0800, 0x0800, "Unknown 1-4" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, "Memory Check" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Unknown 1-7" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x8000, IP_ACTIVE_LOW, "SW1:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( msgunda1 )
	PORT_INCLUDE(msgundam)

	PORT_MODIFY("COINS") // IN2 - Coins - $400004.w
	/* this set seems to be a japan set, english mode doesn't work correctly */
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Language ) )
//  PORT_DIPSETTING(      0x0080, DEF_STR( English ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Japanese ) )
INPUT_PORTS_END


/***************************************************************************
                            Oishii Puzzle
***************************************************************************/

static INPUT_PORTS_START( oisipuzl )
	PORT_START("P1") //Player 1
	JOY_TYPE1_2BUTTONS(1)

	PORT_START("P2") //Player 2
	JOY_TYPE1_2BUTTONS(2)

	PORT_START("COINS") //Coins
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0008, IP_ACTIVE_LOW )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW") //2 DSWs - $600001 & 3.b
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0004, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0008, 0x0008, "SW2:4" ) /* Manual States dips 4-7 are unused */
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0010, "SW2:5" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0020, "SW2:6" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "SW2:7" ) /* Listed as "Unused" */
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW2:8" )

	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x3800, 0x3800, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x3800, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x2800, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_4C ) )
	PORT_DIPUNUSED_DIPLOC( 0x4000, 0x4000, "SW1:7" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x8000, 0x8000, "SW1:8" ) /* Listed as "Unused" */
INPUT_PORTS_END



/***************************************************************************
                            Pro Mahjong Kiwame
***************************************************************************/

static INPUT_PORTS_START( kiwame )
	PORT_START("P1") //Unused
	PORT_START("P2") //Unused

	PORT_START("COINS") //Coins
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_IMPULSE(5)

	PORT_START("DSW") //2 DSWs - $e00001 & 3.b
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(      0x0008, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Player's TSUMO" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, "Manual" )
	PORT_DIPSETTING(      0x0000, "Auto"   )

	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0200, IP_ACTIVE_LOW, "SW1:2" )
	PORT_DIPNAME( 0x1c00, 0x1c00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4,5")
	PORT_DIPSETTING(      0x1c00, DEF_STR( None ) )
	PORT_DIPSETTING(      0x1800, "Prelim  1" )
	PORT_DIPSETTING(      0x1400, "Prelim  2" )
	PORT_DIPSETTING(      0x1000, "Final   1" )
	PORT_DIPSETTING(      0x0c00, "Final   2" )
	PORT_DIPSETTING(      0x0800, "Final   3" )
	PORT_DIPSETTING(      0x0400, "Qrt Final" )
	PORT_DIPSETTING(      0x0000, "SemiFinal" )
	PORT_DIPNAME( 0xe000, 0xe000, "Points Gap" ) PORT_DIPLOCATION("SW1:6,7,8")
	PORT_DIPSETTING(      0xe000, DEF_STR( None ) )
	PORT_DIPSETTING(      0xc000, "+6000" )
	PORT_DIPSETTING(      0xa000, "+4000" )
	PORT_DIPSETTING(      0x8000, "+2000" )
	PORT_DIPSETTING(      0x6000, "-2000" )
	PORT_DIPSETTING(      0x4000, "-4000" )
	PORT_DIPSETTING(      0x2000, "-6000" )
	PORT_DIPSETTING(      0x0000, "-8000" )

/*
        row 0   1   2   3   4
bit 0       a   b   c   d   lc
    1       e   f   g   h
    2       i   j   k   l
    3       m   n   ch  po  ff
    4       ka  re  ro
    5       st  bt
*/

	PORT_START("KEY0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0xffc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START1  )
	PORT_BIT( 0xffc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0xffe0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY4")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END



/***************************************************************************
                                Quiz Kokology
***************************************************************************/

static INPUT_PORTS_START( qzkklogy )
	PORT_START("P1") //Player 1 - $b00001.b
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON5 )	PORT_PLAYER(1) PORT_NAME("P1 Pause (Cheat)")// pause (cheat)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1  )

	PORT_START("P2") //Player 2 - $b00003.b
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )	PORT_PLAYER(2) PORT_NAME("P2 Pause (Cheat)")// pause (cheat)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2  )

	PORT_START("COINS") //Coins - $b00005.b
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN  )

	PORT_START("DSW") //2 DSWs - $600001 & 3.b
	PORT_DIPNAME( 0x0003, 0x0003, "Unknown 1-0&1*" ) /* Manual States dips 1-5 are unused */
	PORT_DIPSETTING(      0x0000, "0" )
	PORT_DIPSETTING(      0x0001, "1" )
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPNAME( 0x0004, 0x0004, "Highlight Right Answer (Cheat)")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Unknown 1-3" )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Unknown 1-4" )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE( 0x0020, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unused ) ) /* Manual States this dip is unused */
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x8000, "2" )
	PORT_DIPSETTING(      0xc000, "3" )
	PORT_DIPSETTING(      0x4000, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
INPUT_PORTS_END



/***************************************************************************
                                Quiz Kokology 2
***************************************************************************/

static INPUT_PORTS_START( qzkklgy2 )
	PORT_START("P1") //Player 1 - $b00001.b
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1  )

	PORT_START("P2") //Player 2 - $b00003.b
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2  )

	PORT_START("COINS") //Coins - $b00005.b
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN  )

	PORT_START("DSW") //2 DSWs - $600001 & 3.b
	PORT_DIPNAME( 0x0003, 0x0003, "Unknown 1-0&1*" ) /* Manual States dips 1-5 are unused */
	PORT_DIPSETTING(      0x0000, "0" )
	PORT_DIPSETTING(      0x0001, "1" )
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPNAME( 0x0004, 0x0004, "Highlight Right Answer (Cheat)")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Skip Real DAT Rom Check?" ) // 'ON' it will pass DAT rom even if it isn't mapped(!)
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Unknown 1-4" )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE( 0x0020, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x8000, "2" )
	PORT_DIPSETTING(      0xc000, "3" )
	PORT_DIPSETTING(      0x4000, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
INPUT_PORTS_END


/***************************************************************************
                                    Rezon
***************************************************************************/

static INPUT_PORTS_START( rezon )
	PORT_START("P1") //Player 1
	JOY_TYPE1_3BUTTONS(1)	// 1 used??

	PORT_START("P2") //Player 2
	JOY_TYPE1_3BUTTONS(2)	// 1 used ??

	PORT_START("COINS") //Coins
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0008, IP_ACTIVE_LOW)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no taito logo
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW") //2 DSWs - $600001 & 3.b
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0004, 0x0004, "SW2:3" ) /* Listed as "Unused" */
	PORT_DIPNAME( 0x0018, 0x0018, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(      0x0000, "Upright 1 Controller" )
	PORT_DIPSETTING(      0x0018, "Upright 2 Controllers" )
	PORT_DIPSETTING(      0x0008, DEF_STR( Cocktail ) )
//  PORT_DIPSETTING(      0x0010, "10" )                /* Unused / Not Defined */
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0020, "SW2:6" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "SW2:7" ) /* Listed as "Unused" */
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW2:8" )

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0200, "2" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0100, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xf000, 0xf000, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(      0xa000, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(      0xb000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0xd000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 8C_3C ) )
	PORT_DIPSETTING(      0xe000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0xf000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x9000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x7000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x5000, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
INPUT_PORTS_END

static INPUT_PORTS_START( rezont )

	PORT_INCLUDE( rezon )

	PORT_MODIFY("COINS")
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // gives the taito logo
INPUT_PORTS_END

/***************************************************************************
                            SD Gundam Neo Battling
***************************************************************************/

/*
    When the "Stage Select" dip switch is on and button1 is pressed during boot,
    pressing P1's button3 freezes the game (pressing P2's button3 resumes it).
*/
static INPUT_PORTS_START( neobattl )
	PORT_START("P1") // Player 1 - $400000.w
	JOY_TYPE1_1BUTTON(1)	// bump to 3 buttons for freezing to work

	PORT_START("P2") //Player 2 - $400002.w
	JOY_TYPE1_1BUTTON(2)	// bump to 3 buttons for freezing to work

	PORT_START("COINS") //Coins - $400004.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT     )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN  )

	PORT_START("DSW") //2 DSWs - $600001 & 3.b
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )	// used
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Stage Select (Cheat)")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )	// unused?
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )	// unused?
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )	// unused?
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE( 0x0080, IP_ACTIVE_LOW )

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0200, "1" )
	PORT_DIPSETTING(      0x0300, "2" )
	PORT_DIPSETTING(      0x0100, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy )    )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal )  )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard )    )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xf000, 0xf000, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0xa000, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(      0xb000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0xd000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 8C_3C ) )
	PORT_DIPSETTING(      0xe000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0xf000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x9000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x7000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x5000, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
INPUT_PORTS_END



/***************************************************************************
                                Sokonuke
***************************************************************************/

static INPUT_PORTS_START( sokonuke )
	PORT_START("P1") //Player 1
	JOY_TYPE1_1BUTTON(1)

	PORT_START("P2") //Player 2
	JOY_TYPE1_1BUTTON(2)

	PORT_START("COINS") //Coins
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0008, IP_ACTIVE_LOW )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW") //2 DSWs - $400009 & b.b
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Unknown 1-5*" )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )	// unused?
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE( 0x0080, IP_ACTIVE_LOW )

	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x3800, 0x3800, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x2800, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x3800, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Cheap Continue" )
	PORT_DIPSETTING(      0x4000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )	// unused?
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END



/***************************************************************************
                                Strike Gunner
***************************************************************************/

static INPUT_PORTS_START( stg )
	PORT_START("P1") //Player 1 - $b00001.b
	JOY_TYPE1_3BUTTONS(1)

	PORT_START("P2") //Player 2 - $b00003.b
	JOY_TYPE1_3BUTTONS(2)

	PORT_START("COINS") //Coins - $b00005.b
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN  )
//  PORT_DIPNAME( 0x00f0, 0x00f0, "Title" )
	/* This is the index in a table with pointers to the
       title logo, but the table is filled with just 1 value */

	PORT_START("DSW") //2 DSWs - $600001 & 3.b
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy )    ) // 0
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal )  ) // 4
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard )    ) // 8
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) ) // b
	PORT_DIPUNUSED_DIPLOC( 0x0004, 0x0004, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x0008, 0x0008, "SW2:4" )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x0010, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0030, "3" )
	PORT_DIPSETTING(      0x0020, "5" )
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0080, "SW2:8" )

	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0400, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPUNUSED_DIPLOC( 0x0800, 0x0800, "SW1:4" )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x1000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x8000, 0x8000, "SW1:8" )
INPUT_PORTS_END



/***************************************************************************
                            Thunder & Lightning
***************************************************************************/

static INPUT_PORTS_START( thunderl )
	PORT_START("P1") //Player 1
	JOY_TYPE1_2BUTTONS(1)	// button2 = speed up

	PORT_START("P2") //Player 2
	JOY_TYPE1_2BUTTONS(2)

	PORT_START("COINS") //Coins + DSW
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT     )
	PORT_DIPNAME( 0x0010, 0x0000, "Force 1 Life" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x00e0, 0x00e0, "Copyright" )
	PORT_DIPSETTING(      0x0080, "Romstar" )
	PORT_DIPSETTING(      0x00c0, "Seta (Romstar License)" )
	PORT_DIPSETTING(      0x00e0, "Seta (Visco License)" )
	PORT_DIPSETTING(      0x00a0, "Visco" )
	PORT_DIPSETTING(      0x0060, DEF_STR( None ) )
//  PORT_DIPSETTING(      0x0040, DEF_STR( None ) )
//  PORT_DIPSETTING(      0x0020, DEF_STR( None ) )
//  PORT_DIPSETTING(      0x0000, DEF_STR( None ) )

	PORT_START("DSW") //2 DSWs - $600003 & 1.b
	PORT_DIPNAME( 0x000f, 0x000f, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW2:1,2,3,4")
	PORT_DIPSETTING(      0x000c, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x000d, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 4C_2C ) )
	PORT_DIPSETTING(      0x000e, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0009, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_4C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 3C_3C ) )
	PORT_DIPSETTING(      0x000a, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(      0x000f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 2C_4C ) )
	PORT_DIPSETTING(      0x000b, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x00f0, 0x00f0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW2:5,6,7,8")
	PORT_DIPSETTING(      0x00c0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x00d0, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 4C_2C ) )
	PORT_DIPSETTING(      0x00e0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0090, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_4C ) )
	PORT_DIPSETTING(      0x0050, DEF_STR( 3C_3C ) )
	PORT_DIPSETTING(      0x00a0, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(      0x00f0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 2C_4C ) )
	PORT_DIPSETTING(      0x00b0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0070, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_4C ) )

	PORT_SERVICE_DIPLOC(  0x0100, IP_ACTIVE_LOW, "SW1:1" )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )		// WEIRD!
	PORT_DIPSETTING(      0x0200, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Controls ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0800, "2" )
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x2000, "3" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )

	/* These are needed due to the memory map shared with Wit's */
	PORT_START("P3")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P4") //Player 4
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



/***************************************************************************
                                Thundercade (US)
***************************************************************************/

static INPUT_PORTS_START( tndrcade )
	PORT_START("P1") //Player 1
	JOY_TYPE1_2BUTTONS(1)

	PORT_START("P2") //Player 2
	JOY_TYPE1_2BUTTONS(2)

	PORT_START("COINS") //Coins
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1   )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START2   )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_TILT     )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN  )

	PORT_START("DSW") //2 DSWs - $600001 & 3.b
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy )    )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal )  )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard )    )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x000c, "50K  Only" )
	PORT_DIPSETTING(      0x0004, "50K, Every 150K" )
	PORT_DIPSETTING(      0x0000, "70K, Every 200K" )
	PORT_DIPSETTING(      0x0008, "100K Only" )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x0010, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0030, "3" )
	PORT_DIPSETTING(      0x0020, "5" )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, "Licensed To" ) PORT_DIPLOCATION("SW2:8")	// + coin mode (not supported)
	PORT_DIPSETTING(      0x0080, "Taito America Corp." )
	PORT_DIPSETTING(      0x0000, "Taito Corp. Japan" )

	PORT_DIPNAME( 0x0100, 0x0100, "Title" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0100, "Thundercade" )
	PORT_DIPSETTING(      0x0000, "Twin Formation" )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0400, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPNAME( 0x0800, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x1000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0x4000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 1C_2C ) )
INPUT_PORTS_END


/***************************************************************************
                                Thundercade (Japan)
***************************************************************************/

static INPUT_PORTS_START( tndrcadj )
	PORT_INCLUDE(tndrcade)

	PORT_MODIFY("DSW") //2 DSWs - $600001 & 3.b
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x0100, 0x0100, "Invulnerability (Cheat)") PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END



/***************************************************************************
                                Twin Eagle
***************************************************************************/

static INPUT_PORTS_START( twineagl )
	PORT_START("P1") //Player 1
	JOY_TYPE1_2BUTTONS(1)

	PORT_START("P2") //Player 2
	JOY_TYPE1_2BUTTONS(2)

	PORT_START("COINS") //Coins
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1   )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START2   )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_TILT     )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN  )

	PORT_START("DSW") //2 DSWs - $600001 & 3.b
	PORT_DIPNAME( 0x4001, 0x4001, "Copyright" )		// Always "Seta" if sim. players = 1
	PORT_DIPSETTING(      0x4001, "Seta (Taito license)" )
	PORT_DIPSETTING(      0x0001, "Taito" )
	PORT_DIPSETTING(      0x4000, "Taito America" )
	PORT_DIPSETTING(      0x0000, "Taito America (Romstar license)" )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE( 0x0004, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0008, 0x0000, DEF_STR( Cabinet ) )	// Only if simultaneous players = 1
	PORT_DIPSETTING(      0x0000, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal )  )
	PORT_DIPSETTING(      0x0200, DEF_STR( Easy )    )
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard )    )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c00, 0x0000, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(      0x0c00, "Never" )
	PORT_DIPSETTING(      0x0800, "500K Only" )
	PORT_DIPSETTING(      0x0400, "1000K Only" )
	PORT_DIPSETTING(      0x0000, "500K, Every 1500K" )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x1000, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x3000, "3" )
	PORT_DIPSETTING(      0x2000, "5" )
	PORT_DIPNAME( 0x8000, 0x8000, "Coinage Type" )	// not supported
	PORT_DIPSETTING(      0x8000, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
INPUT_PORTS_END



/***************************************************************************
                                Ultraman Club
***************************************************************************/

static INPUT_PORTS_START( umanclub )
	PORT_START("P1") //Player 1
	JOY_TYPE1_2BUTTONS(1)

	PORT_START("P2") //Player 2
	JOY_TYPE1_2BUTTONS(2)

	PORT_START("COINS") //Coins
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT     )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN  )

	PORT_START("DSW") //2 DSWs
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Unknown 2-2*" )	//?
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Stage Select (Cheat)")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Unknown 2-4" )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Unknown 2-5" )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Unknown 2-6" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE( 0x0080, IP_ACTIVE_LOW )

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0200, "1" )
	PORT_DIPSETTING(      0x0300, "2" )
	PORT_DIPSETTING(      0x0100, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy )    )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal )  )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard )    )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xf000, 0xf000, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0xa000, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(      0xb000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0xd000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 8C_3C ) )
	PORT_DIPSETTING(      0xe000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0xf000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x9000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x7000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x5000, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
INPUT_PORTS_END


/***************************************************************************
                            Ultra Toukon Densetsu
***************************************************************************/

static INPUT_PORTS_START( utoukond )
	PORT_START("P1") //Player 1
	JOY_TYPE1_3BUTTONS(1)

	PORT_START("P2") //Player 2
	JOY_TYPE1_3BUTTONS(2)

	PORT_START("COINS") //Coins
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	/* These are NOT Dip Switches but jumpers */
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW") //2 DSWs
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
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_3C ) )
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

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0200, "2" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0100, "4" )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x4000, "100k" )
	PORT_DIPSETTING(      0x0000, "150k" )
	PORT_SERVICE_DIPLOC(  0x8000, IP_ACTIVE_LOW, "SW1:8" )
INPUT_PORTS_END

/***************************************************************************
                                U.S. Classic
***************************************************************************/

static INPUT_PORTS_START( usclssic )
	PORT_START("P1X")     /* muxed port 0 */
	PORT_BIT( 0x0fff, 0x0000, IPT_TRACKBALL_X ) PORT_SENSITIVITY(70) PORT_KEYDELTA(30) PORT_RESET
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1Y")     /* muxed port 0 */
	PORT_BIT( 0x0fff, 0x0000, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(70) PORT_KEYDELTA(30) PORT_RESET
	PORT_BIT( 0x1000, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("P2X")     /* muxed port 1 */
	PORT_BIT( 0x0fff, 0x0000, IPT_TRACKBALL_X ) PORT_SENSITIVITY(70) PORT_KEYDELTA(30) PORT_RESET PORT_COCKTAIL
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2Y")     /* muxed port 1 */
	PORT_BIT( 0x0fff, 0x0000, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(70) PORT_KEYDELTA(30) PORT_RESET PORT_COCKTAIL
	PORT_BIT( 0x1000, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("COINS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW,  IPT_UNKNOWN  )	// tested (sound related?)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW,  IPT_UNKNOWN  )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_UNKNOWN  )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_UNKNOWN  )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_TILT     )

	PORT_START("DSW") //2 DSWs - $600001 & 3.b
	PORT_DIPNAME( 0x0001, 0x0001, "Credits For 9-Hole" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0001, "2" )
	PORT_DIPSETTING(      0x0000, "3" )
	PORT_DIPNAME( 0x0002, 0x0002, "Game Type" ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0002, "Domestic" )
	PORT_DIPSETTING(      0x0000, "Foreign" )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0004, "1" )
	PORT_DIPSETTING(      0x0008, "2" )
	PORT_DIPSETTING(      0x000c, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(      0x0040, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_2C ) )

	PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0000, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0400, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPNAME( 0x3800, 0x3800, "Flight Distance" ) PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(      0x3800, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x3000, "-30 Yards" )
	PORT_DIPSETTING(      0x2800, "+10 Yards" )
	PORT_DIPSETTING(      0x2000, "+20 Yards" )
	PORT_DIPSETTING(      0x1800, "+30 Yards" )
	PORT_DIPSETTING(      0x1000, "+40 Yards" )
	PORT_DIPSETTING(      0x0800, "+50 Yards" )
	PORT_DIPSETTING(      0x0000, "+60 Yards" )
	PORT_DIPNAME( 0xc000, 0xc000, "Licensed To" ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0xc000, "Romstar" )
	PORT_DIPSETTING(      0x8000, "None (Japan)" )
	PORT_DIPSETTING(      0x4000, "Taito" )
	PORT_DIPSETTING(      0x0000, "Taito America" )
INPUT_PORTS_END



/***************************************************************************
                                War of Aero
***************************************************************************/

static INPUT_PORTS_START( wrofaero )
	PORT_START("P1") //Player 1 - $400000.w
	JOY_TYPE1_3BUTTONS(1)	// 3rd button selects the weapon
							// when the dsw for cheating is on

	PORT_START("P2") //Player 2 - $400002.w
	JOY_TYPE1_3BUTTONS(2)

	PORT_START("COINS") //Coins - $400004.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT     )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN  )

	PORT_START("DSW") //2 DSWs - $600001 & 3.b
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Unknown 1-3*" ) PORT_DIPLOCATION("SW1:3")	// tested...Manual states "Unused"
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Stage & Weapon Select (Cheat)") PORT_DIPLOCATION("SW1:4") // P2 Start Is Freeze Screen...Manual states "Unused"
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0010, "SW1:5" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0020, "SW1:6" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "SW1:7" ) /* Listed as "Unused" */
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:8" )

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0200, "2" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0100, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xf000, 0xf000, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW2:5,6,7,8")
	PORT_DIPSETTING(      0xa000, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(      0xb000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0xd000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 8C_3C ) )
	PORT_DIPSETTING(      0xe000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0xf000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x9000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x7000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x5000, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
INPUT_PORTS_END



/***************************************************************************
                                    Wit's
***************************************************************************/

static INPUT_PORTS_START( wits )
	PORT_START("P1") //Player 1
	JOY_TYPE1_2BUTTONS(1)

	PORT_START("P2") //Player 2
	JOY_TYPE1_2BUTTONS(2)

	PORT_START("COINS") //Coins + DSW
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT     )
	PORT_DIPNAME( 0x0010, 0x0010, "Unknown 3-4*" )	// Jumpers, I guess
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Unknown 3-5*" )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00c0, 0x0040, "License" )
	PORT_DIPSETTING(      0x00c0, "Romstar" )
	PORT_DIPSETTING(      0x0080, "Seta U.S.A" )
	PORT_DIPSETTING(      0x0040, "Visco (Japan Only)" )
	PORT_DIPSETTING(      0x0000, "Athena (Japan Only)" )

	PORT_START("DSW") //2 DSWs - $600003 & 1.b
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0008, "150k, 350k" )
	PORT_DIPSETTING(      0x000c, "200k, 500k" )
	PORT_DIPSETTING(      0x0004, "300k, 600k" )
	PORT_DIPSETTING(      0x0000, "400k" )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0010, "2" )
	PORT_DIPSETTING(      0x0030, "3" )
	PORT_DIPSETTING(      0x0020, "5" )
	PORT_DIPNAME( 0x0040, 0x0040, "Play Mode" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0040, "2 Players" )
	PORT_DIPSETTING(      0x0000, "4 Players" )
	PORT_DIPNAME( 0x0080, 0x0080, "CPU Player During Multi-Player Game" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Yes ) )

	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0400, "Every 3rd Loop" )
	PORT_DIPSETTING(      0x0000, "Every 7th Loop" )
	PORT_SERVICE_DIPLOC(  0x0800, IP_ACTIVE_LOW, "SW1:4" )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0xc000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )

	PORT_START("P3") //Player 3
	JOY_TYPE1_2BUTTONS(3)

	PORT_START("P4") //Player 4
	JOY_TYPE1_2BUTTONS(4)
INPUT_PORTS_END


/***************************************************************************
                                Zing Zing Zip
***************************************************************************/

static INPUT_PORTS_START( zingzip )
	PORT_START("P1") //Player 1 - $400000.w
	JOY_TYPE1_2BUTTONS(1)

	PORT_START("P2") //Player 2 - $400002.w
	JOY_TYPE1_2BUTTONS(2)

	PORT_START("COINS") //Coins - $400004.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN  ) // no coin 2
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT     )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN  )

	PORT_START("DSW") //2 DSWs - $600001 & 3.b
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0004, 0x0004, "SW2:3" ) /* Manual States dips 3-7 are unused */
	PORT_DIPUNUSED_DIPLOC( 0x0008, 0x0008, "SW2:4" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0010, "SW2:5" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0020, "SW2:6" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "SW2:7" ) /* Listed as "Unused" */
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW2:8" )

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0200, "2" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0100, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy )    )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal )  )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard )    )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xf000, 0xf000, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(      0xa000, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(      0xb000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0xd000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 8C_3C ) )
	PORT_DIPSETTING(      0xe000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0xf000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x9000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x7000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x5000, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
INPUT_PORTS_END

/*************************************
        Pairs Love
*************************************/

static INPUT_PORTS_START( pairlove )
	PORT_START("P1") //Player 1 - $500001.b
	JOY_TYPE1_2BUTTONS(1)	// button2 = speed up

	PORT_START("P2") //Player 2 - $500003.b
	JOY_TYPE1_2BUTTONS(2)

	PORT_START("COINS") //Coins + DSW - $500005.b
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT     )

	PORT_START("DSW")    // 2 DIP switches
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0004, 0x0004, "SW2:3" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0008, 0x0008, "SW2:4" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0010, "SW2:5" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0020, "SW2:6" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "SW2:7" ) /* Listed as "Unused" */
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW2:8" )

	PORT_DIPUNUSED_DIPLOC( 0x0100, 0x0100, "SW1:1" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0200, 0x0200, "SW1:2" ) /* Listed as "Unused" */
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xf000, 0xf000, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(      0xa000, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(      0xb000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0xd000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 8C_3C ) )
	PORT_DIPSETTING(      0xe000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0xf000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x9000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x7000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x5000, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
INPUT_PORTS_END

/***************************************************************************
                                Orbs
***************************************************************************/

static INPUT_PORTS_START( orbs )
	PORT_START("P1") //Player 1
	JOY_TYPE1_1BUTTON(1)

	PORT_START("P2") //Player 2 ??
	JOY_TYPE1_1BUTTON(2)

	PORT_START("COINS") //Coins
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("DSW") //2 DSWs - $600001 & 3.b
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Stock" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0010, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Level_Select ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Timer speed" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, "Fast" )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW2:8" )

	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0500, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x3800, 0x3800, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(      0x2800, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x3800, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Force Coinage (Half Coin)" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_1C ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Unknown 1-8" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

/***************************************************************************
                  Kero Kero Keroppi no Issyoni Asobou
***************************************************************************/

static INPUT_PORTS_START( keroppi )
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Prize Out") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("COINS") //Coins
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(5)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Prize Hopper") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("DSW") //2 DSWs - $600001 & 3.b
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(      0x0003, "1" )
	PORT_DIPSETTING(      0x0005, "2" )
	PORT_DIPSETTING(      0x0001, "3" )
	PORT_DIPSETTING(      0x0007, "4" )
	PORT_DIPSETTING(      0x0006, "5" )
	PORT_DIPSETTING(      0x0002, "6" )
	PORT_DIPSETTING(      0x0004, "7" )
	PORT_DIPSETTING(      0x0000, "8" )
	PORT_DIPNAME( 0x0038, 0x0038, "Game Select" ) PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(      0x0038, "No. 1,2,3" )
	PORT_DIPSETTING(      0x0030, "No. 1" )
	PORT_DIPSETTING(      0x0028, "No. 2,3" )
	PORT_DIPSETTING(      0x0020, "No. 3" )
	PORT_DIPSETTING(      0x0018, "No. 1,2" )
	PORT_DIPSETTING(      0x0010, "No. 2" )
	PORT_DIPSETTING(      0x0008, "No. 1,3" )
	PORT_DIPSETTING(      0x0000, "No. 1,2,3" )
	PORT_DIPNAME( 0x0040, 0x0040, "Unknown 2-7" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Unknown 2-8" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0800, 0x0800, "Unknown 1-4" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, "Unknown 1-5" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x8000, IP_ACTIVE_LOW, "SW1:8" )
INPUT_PORTS_END

/***************************************************************************
                                Crazy Fight
***************************************************************************/

static INPUT_PORTS_START( crazyfgt )
	PORT_START("COINS") //Coins - $610000.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_SPECIAL )	// protection

	PORT_START("UNK") //? - $610002.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("INPUT") //Player - $610004.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("top-center")    PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("bottom-center") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("top-left")      PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("bottom-left")   PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("top-right")     PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("bottom-right")  PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_SERVICE1 )	// ticket

	PORT_START("DSW") //2 DSWs - $630001 & 3.b
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x00c0, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(      0x00c0, "5" )
	PORT_DIPSETTING(      0x0080, "10" )
	PORT_DIPSETTING(      0x0040, "15" )
	PORT_DIPSETTING(      0x0000, "20" )

	PORT_SERVICE_DIPLOC(  0x0100, IP_ACTIVE_LOW, "SW1:1" )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c00, 0x0c00, "Difficulty?" ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0c00, "0" )
	PORT_DIPSETTING(      0x0800, "1" )
	PORT_DIPSETTING(      0x0400, "2" )
	PORT_DIPSETTING(      0x0000, "3" )
	PORT_DIPNAME( 0x3000, 0x3000, "Energy" ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x1000, "24" )
	PORT_DIPSETTING(      0x2000, "32" )
	PORT_DIPSETTING(      0x3000, "48" )
	PORT_DIPSETTING(      0x0000, "100" )
	PORT_DIPNAME( 0xc000, 0xc000, "Bonus?" ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0xc000, "0" )
	PORT_DIPSETTING(      0x8000, "1" )
	PORT_DIPSETTING(      0x4000, "2" )
	PORT_DIPSETTING(      0x0000, "3" )
INPUT_PORTS_END

/***************************************************************************
                             International Toote
***************************************************************************/

static INPUT_PORTS_START( inttoote )
	PORT_START("DSW1")	// DSW 1
	PORT_DIPNAME( 0x03, 0x03, "Max Bet" )
	PORT_DIPSETTING(    0x03, "10" )
	PORT_DIPSETTING(    0x02, "20" )
	PORT_DIPSETTING(    0x01, "99" )
//  PORT_DIPSETTING(    0x00, "99" )
	PORT_DIPNAME( 0x1c, 0x1c, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x14, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x08, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin/20 Credits" )
	PORT_DIPSETTING(    0x00, "1 Coin/50 Credits" )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 1-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 1-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 1-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2_3")	// DSW 2&3
	PORT_DIPNAME( 0x0001, 0x0001, "Unknown 2-0" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "Unknown 2-1" )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Unknown 2-2" )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0018, 0x0018, "Betting Clock Speed" )
	PORT_DIPSETTING(      0x0018, "Slowest" )
	PORT_DIPSETTING(      0x0010, "Slower" )
	PORT_DIPSETTING(      0x0008, "Faster" )
	PORT_DIPSETTING(      0x0000, "Fastest" )
	PORT_DIPNAME( 0x01e0, 0x01e0, "Payout Rate" )
	PORT_DIPSETTING(      0x01e0, "80%" )
	PORT_DIPSETTING(      0x01c0, "81%" )
	PORT_DIPSETTING(      0x01a0, "82%" )
	PORT_DIPSETTING(      0x0180, "83%" )
	PORT_DIPSETTING(      0x0160, "84%" )
	PORT_DIPSETTING(      0x0140, "85%" )
	PORT_DIPSETTING(      0x0120, "86%" )
	PORT_DIPSETTING(      0x0100, "87%" )
	PORT_DIPSETTING(      0x00e0, "88%" )
	PORT_DIPSETTING(      0x00c0, "89%" )
	PORT_DIPSETTING(      0x00a0, "90%" )
	PORT_DIPSETTING(      0x0080, "91%" )
	PORT_DIPSETTING(      0x0060, "92%" )
	PORT_DIPSETTING(      0x0040, "93%" )
	PORT_DIPSETTING(      0x0020, "94%" )
	PORT_DIPSETTING(      0x0000, "95%" )
	PORT_DIPNAME( 0x0200, 0x0200, "Unknown 3-1" )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, "Payout" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, "Horses" )
	PORT_DIPSETTING(      0x0800, "Random" )
	PORT_DIPSETTING(      0x0000, "Cyclic" )
	PORT_DIPNAME( 0x1000, 0x1000, "Higher Odds" )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, "Unknown 3-5" )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Unknown 3-6" )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Unknown 3-7" )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SPECIAL ) // P1 coin out
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_SPECIAL )	// P2 coin out
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Door Open") PORT_TOGGLE
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Question Mark")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE( 0x1000, IP_ACTIVE_LOW )	// Function menu (electronic key switch, fitted beneath the front panel)
	PORT_BIT( 0x6000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_SPECIAL ) // Battery (0 = OK, 1 = NG)
/*
    Betting Panel         (keys)

    1 1-2 2-3 3-4 4-5 5-6 (1QWERT)
    2 1-3 2-4 3-5 4-6     (2ASDF)
    3 1-4 2-5 3-6         (3ZXC)
    4 1-5 2-6             (4YU)
    5 1-6                 (5H)
    6                     (6)
*/
	PORT_START("BET0")	// 200000.w (0x08)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 3") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 1") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 2") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 3") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 4") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 5") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("BET1")	// 200000.w (0x10)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Collect")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Cancel")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 6") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Collect") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Cancel") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("BET2")	// 200000.w (0x20)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 1-2") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 1-3") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 1-4") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 1-5") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 1-6") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 1-2") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 1-3") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 1-4") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 1-5") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 1-6") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("BET3")	// 200000.w (0x40)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 2-3") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 2-4") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 2-5") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 2-6") PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 3-4") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 2-3") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 2-4") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 2-5") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 2-6") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 3-4") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("BET4")	// 200000.w (0x80)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 3-5") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 3-6") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 4-5") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 4-6") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Bet 5-6") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 3-5") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 3-6") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 4-5") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 4-6") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Bet 5-6") //PORT_CODE(KEYCODE_)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( jockeyc )
	PORT_INCLUDE( inttoote )

	PORT_MODIFY("BET1")	// 200000.w (0x10)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Credit")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Credit")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_START2 )

	/* many service switches actually */
	PORT_MODIFY("P1")
	PORT_DIPNAME( 0x0001, 0x0001, "Coin Drop - 1P" )
	PORT_DIPSETTING(    0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "Hopper Overflow - 1P" )
	PORT_DIPSETTING(    0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0000, "Coin Out" )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Att Pay - 1P" )
	PORT_DIPSETTING(    0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("Coin B - 1P")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_NAME("Coin A - 1P")
	PORT_DIPNAME( 0x0040, 0x0040, "Coin Sense 2 - 1P" )
	PORT_DIPSETTING(    0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Coin Sense 1 - 1P" )
	PORT_DIPSETTING(    0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "Coin Drop - 2P" )
	PORT_DIPSETTING(    0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, "Sel Sense" )
	PORT_DIPSETTING(    0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, "Att Pay - 2P" )
	PORT_DIPSETTING(    0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_NAME("Coin B - 2P")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_NAME("Coin A - 2P")
	PORT_DIPNAME( 0x4000, 0x4000, "Coin Sense 2 - 2P" )
	PORT_DIPSETTING(    0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Coin Sense 1 - 2P" )
	PORT_DIPSETTING(    0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )

	PORT_MODIFY("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Door Open") PORT_TOGGLE
	PORT_DIPNAME( 0x0002, 0x0002, "SYSTEM" )
	PORT_DIPSETTING(    0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Tilt ) )
	PORT_DIPSETTING(    0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, "Special Test Mode Item?" )
	PORT_DIPSETTING(    0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, "Call SW" )
	PORT_DIPSETTING(    0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_SERVICE( 0x1000, IP_ACTIVE_LOW )	// Function menu (electronic key switch, fitted beneath the front panel)
	PORT_DIPNAME( 0x2000, 0x2000, "Analyzer" )
	PORT_DIPSETTING(    0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Reset SW")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_SPECIAL ) // Battery (0 = OK, 1 = NG)
INPUT_PORTS_END


/***************************************************************************


                                Graphics Layouts

Sprites and layers use 16x16 tile, made of four 8x8 tiles. They can be 4
or 6 planes deep and are stored in a wealth of formats.

***************************************************************************/

						/* First the 4 bit tiles */


/* The bitplanes are packed togheter */
static const gfx_layout layout_packed =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{2*4,3*4,0*4,1*4},
	{256+128,256+129,256+130,256+131, 256+0,256+1,256+2,256+3,
	 128,129,130,131, 0,1,2,3},
	{0*16,1*16,2*16,3*16,4*16,5*16,6*16,7*16,
	 32*16,33*16,34*16,35*16,36*16,37*16,38*16,39*16},
	16*16*4
};


/* The bitplanes are separated */
static const gfx_layout layout_planes =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(0,4),RGN_FRAC(1,4),RGN_FRAC(2,4),RGN_FRAC(3,4) },
	{ STEP8(0,1), STEP8(8*8,1) },
	{ STEP8(0,8), STEP8(8*8*2,8) },
	16*16
};


/* The bitplanes are separated (but there are 2 per rom) */
static const gfx_layout layout_planes_2roms =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{RGN_FRAC(1,2)+8, RGN_FRAC(1,2)+0, 8, 0},
	{0,1,2,3,4,5,6,7, 128,129,130,131,132,133,134,135},
	{0*16,1*16,2*16,3*16,4*16,5*16,6*16,7*16,
	 16*16,17*16,18*16,19*16,20*16,21*16,22*16,23*16 },
	16*16*2
};


/* The bitplanes are separated (but there are 2 per rom).
   Each 8x8 tile is additionally split in 2 vertical halves four bits wide,
   stored one after the other */
static const gfx_layout layout_planes_2roms_split =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{0,4, RGN_FRAC(1,2)+0,RGN_FRAC(1,2)+4},
	{128+64,128+65,128+66,128+67, 128+0,128+1,128+2,128+3,
	 8*8+0,8*8+1,8*8+2,8*8+3, 0,1,2,3},
	{0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8,
	 32*8,33*8,34*8,35*8,36*8,37*8,38*8,39*8},
	16*16*2
};

static const gfx_layout layout_8bpp =
{
	16,16,
	RGN_FRAC(1,4),
	8,
	{ RGN_FRAC(0,4)+0,RGN_FRAC(0,4)+4, RGN_FRAC(1,4)+0,RGN_FRAC(1,4)+4,RGN_FRAC(2,4)+0,RGN_FRAC(2,4)+4, RGN_FRAC(3,4)+0,RGN_FRAC(3,4)+4},
	{128+64,128+65,128+66,128+67, 128+0,128+1,128+2,128+3,
	 8*8+0,8*8+1,8*8+2,8*8+3, 0,1,2,3},
	{0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8,
	 32*8,33*8,34*8,35*8,36*8,37*8,38*8,39*8},
	16*16*2
};



						/* Then the 6 bit tiles */


/* The bitplanes are packed together: 3 roms with 2 bits in each */
static const gfx_layout layout_packed_6bits_3roms =
{
	16,16,
	RGN_FRAC(1,3),
	6,
	{RGN_FRAC(0,3)+0,RGN_FRAC(0,3)+4,  RGN_FRAC(1,3)+0,RGN_FRAC(1,3)+4,  RGN_FRAC(2,3)+0,RGN_FRAC(2,3)+4},
	{128+64,128+65,128+66,128+67, 128+0,128+1,128+2,128+3,
	 64,65,66,67, 0,1,2,3},
	{0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8,
	 32*8,33*8,34*8,35*8,36*8,37*8,38*8,39*8},
	16*16*2
};


/* The bitplanes are packed togheter: 4 bits in one rom, 2 bits in another.
   Since there isn't simmetry between the two roms, we load the latter with
   ROM_LOAD16_BYTE. This way we can think of it as a 4 planes rom, with the
   upper 2 planes unused.    */

static const gfx_layout layout_packed_6bits_2roms =
{
	16,16,
	RGN_FRAC(1,2),
	6,
	{RGN_FRAC(1,2)+0*4, RGN_FRAC(1,2)+1*4, 2*4,3*4,0*4,1*4},
	{256+128,256+129,256+130,256+131, 256+0,256+1,256+2,256+3,
	 128,129,130,131, 0,1,2,3},
	{0*16,1*16,2*16,3*16,4*16,5*16,6*16,7*16,
	 32*16,33*16,34*16,35*16,36*16,37*16,38*16,39*16},
	16*16*4
};



/***************************************************************************
                                Blandia
***************************************************************************/

static GFXDECODE_START( blandia )
	GFXDECODE_ENTRY( "gfx1", 0, layout_planes_2roms,       0,           32 ) // [0] Sprites
	GFXDECODE_ENTRY( "gfx2", 0, layout_packed_6bits_3roms, 16*32+64*32, 32 ) // [1] Layer 1
	GFXDECODE_ENTRY( "gfx3", 0, layout_packed_6bits_3roms, 16*32,       32 ) // [2] Layer 2
GFXDECODE_END

/***************************************************************************
                                DownTown
***************************************************************************/

static GFXDECODE_START( downtown )
	GFXDECODE_ENTRY( "gfx1", 0, layout_planes_2roms,       512*0, 32 ) // [0] Sprites
	GFXDECODE_ENTRY( "gfx2", 0, layout_planes_2roms_split, 512*0, 32 ) // [1] Layer 1
GFXDECODE_END

/***************************************************************************
                                Seta Roulette
***************************************************************************/

static GFXDECODE_START( setaroul )
	GFXDECODE_ENTRY( "gfx1", 0, layout_planes_2roms,       512*0, 2 ) // [0] Sprites
	GFXDECODE_ENTRY( "gfx2", 0, layout_8bpp, 512*0, 2 ) // [1] Layer 1
GFXDECODE_END

/***************************************************************************
                                J.J.Squawkers
***************************************************************************/

static GFXDECODE_START( jjsquawk )
	GFXDECODE_ENTRY( "gfx1", 0, layout_planes_2roms,       0,             32 ) // [0] Sprites
	GFXDECODE_ENTRY( "gfx2", 0, layout_packed_6bits_2roms, 512 + 64*32*0, 32 ) // [1] Layer 1
	GFXDECODE_ENTRY( "gfx3", 0, layout_packed_6bits_2roms, 512 + 64*32*1, 32 ) // [2] Layer 2
GFXDECODE_END

/***************************************************************************
                            Mobile Suit Gundam
***************************************************************************/

static GFXDECODE_START( msgundam )
	GFXDECODE_ENTRY( "gfx1", 0, layout_planes_2roms, 512*0, 32 ) // [0] Sprites
	GFXDECODE_ENTRY( "gfx2", 0, layout_packed,       512*2, 32 ) // [1] Layer 1
	GFXDECODE_ENTRY( "gfx3", 0, layout_packed,       512*1, 32 ) // [2] Layer 2
GFXDECODE_END

/***************************************************************************
                                Quiz Kokology 2
***************************************************************************/

static GFXDECODE_START( qzkklgy2 )
	GFXDECODE_ENTRY( "gfx1", 0, layout_planes_2roms,	512*0, 32 ) // [0] Sprites
	GFXDECODE_ENTRY( "gfx2", 0, layout_packed,		512*0, 32 ) // [1] Layer 1
GFXDECODE_END

/***************************************************************************
                                Thundercade
***************************************************************************/

static GFXDECODE_START( tndrcade )
	GFXDECODE_ENTRY( "gfx1", 0, layout_planes_2roms, 512*0, 32 ) // [0] Sprites
GFXDECODE_END

/***************************************************************************
                                Orbs
***************************************************************************/

static GFXDECODE_START( orbs )
	GFXDECODE_ENTRY( "gfx1", 0, layout_planes, 512*0, 32 ) // [0] Sprites
GFXDECODE_END

/***************************************************************************
                                Wiggie Waggle / Super Bar
****************************************************************************/

/* these seem to have some silly address swapping, different on each game
  we handle it here, but we could also handle it in the init instead */
static const gfx_layout wiggie_layout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(0,4),RGN_FRAC(1,4),RGN_FRAC(2,4),RGN_FRAC(3,4) },
	{ 0,1,2,3,4,5,6,7,
	 64,65,66,67,68,69,70,71 },
	{ 0*8, 16*8, 4*8, 20*8,
	  2*8, 18*8, 6*8, 22*8,
	  1*8, 17*8, 5*8, 21*8,
	  3*8, 19*8, 7*8, 23*8 },
	16*16
};


static GFXDECODE_START( wiggie )
	GFXDECODE_ENTRY( "gfx1", 0, wiggie_layout,   0x0, 32  ) /* bg tiles */
GFXDECODE_END

static const gfx_layout superbar_layout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(0,4),RGN_FRAC(1,4),RGN_FRAC(2,4),RGN_FRAC(3,4) },
	{ 0,1,2,3,4,5,6,7,
	 64,65,66,67,68,69,70,71 },
	{ 0*8, 2*8,   16*8, 18*8,
	  1*8, 3*8,	  17*8, 19*8,
	  4*8, 6*8,	  20*8, 22*8,
	  5*8, 7*8,	  21*8, 23*8 },
	16*16
};


static GFXDECODE_START( superbar )
	GFXDECODE_ENTRY( "gfx1", 0, superbar_layout,   0x0, 32  ) /* bg tiles */
GFXDECODE_END

/***************************************************************************
                                U.S. Classic
***************************************************************************/

static GFXDECODE_START( usclssic )
	GFXDECODE_ENTRY( "gfx1", 0, layout_planes_2roms,       0, 32 ) // [0] Sprites
	GFXDECODE_ENTRY( "gfx2", 0, layout_packed_6bits_3roms, 512, 32 ) // [1] Layer 1
GFXDECODE_END


/***************************************************************************
                                Zing Zing Zip
***************************************************************************/

static GFXDECODE_START( zingzip )
	GFXDECODE_ENTRY( "gfx1", 0, layout_planes_2roms,       512*0, 32 ) // [0] Sprites
	GFXDECODE_ENTRY( "gfx2", 0, layout_packed_6bits_2roms, 512*2, 32 ) // [1] Layer 1
	GFXDECODE_ENTRY( "gfx3", 0, layout_packed,             512*1, 32 ) // [2] Layer 2
GFXDECODE_END


/***************************************************************************
                                Crazy Fight
***************************************************************************/

static GFXDECODE_START( crazyfgt )
	GFXDECODE_ENTRY( "gfx1", 0, layout_planes,             0,           32 ) // [0] Sprites
	GFXDECODE_ENTRY( "gfx2", 0, layout_packed_6bits_3roms, 16*32+64*32, 32 ) // [1] Layer 1
	GFXDECODE_ENTRY( "gfx3", 0, layout_packed_6bits_3roms, 16*32,       32 ) // [2] Layer 2
GFXDECODE_END

/***************************************************************************
                             International Toote
***************************************************************************/

static GFXDECODE_START( inttoote )
	GFXDECODE_ENTRY( "gfx1", 0, layout_planes,             0,       32 ) // [0] Sprites
	GFXDECODE_ENTRY( "gfx2", 0, layout_planes_2roms_split, 16*32*0, 32 ) // [1] Layer 1
GFXDECODE_END

/***************************************************************************

                                Machine drivers

***************************************************************************/

#define SETA_INTERRUPTS_NUM 2

static INTERRUPT_GEN( seta_interrupt_1_and_2 )
{
	switch (cpu_getiloops(device))
	{
		case 0:		cpu_set_input_line(device, 1, HOLD_LINE);	break;
		case 1:		cpu_set_input_line(device, 2, HOLD_LINE);	break;
	}
}

static INTERRUPT_GEN( seta_interrupt_2_and_4 )
{
	switch (cpu_getiloops(device))
	{
		case 0:		cpu_set_input_line(device, 2, HOLD_LINE);	break;
		case 1:		cpu_set_input_line(device, 4, HOLD_LINE);	break;
	}
}


#define SETA_SUB_INTERRUPTS_NUM 2

static INTERRUPT_GEN( seta_sub_interrupt )
{
	switch (cpu_getiloops(device))
	{
		case 0:		cpu_set_input_line(device, INPUT_LINE_NMI, PULSE_LINE);	break;
		case 1:		cpu_set_input_line(device, 0, HOLD_LINE);				break;
	}
}


/***************************************************************************
                                Thundercade
***************************************************************************/

static const ym2203_interface tndrcade_ym2203_interface =
{
	{
		AY8910_LEGACY_OUTPUT,
		AY8910_DEFAULT_LOADS,
		DEVCB_HANDLER(dsw1_r),		/* input A: DSW 1 */
		DEVCB_HANDLER(dsw2_r),		/* input B: DSW 2 */
		DEVCB_NULL,
		DEVCB_NULL
	},
	NULL
};


#define TNDRCADE_SUB_INTERRUPTS_NUM	32	/* 16 IRQ, 1 NMI */
static INTERRUPT_GEN( tndrcade_sub_interrupt )
{
	if (cpu_getiloops(device) & 1)
		cpu_set_input_line(device, 0, HOLD_LINE);
	else if (cpu_getiloops(device) == 0)
		cpu_set_input_line(device, INPUT_LINE_NMI, PULSE_LINE);
}

static MACHINE_DRIVER_START( tndrcade )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 16000000/2)	/* 8 MHz */
	MDRV_CPU_PROGRAM_MAP(tndrcade_map)
	MDRV_CPU_VBLANK_INT("screen", irq2_line_hold)

	MDRV_CPU_ADD("sub", M65C02, 16000000/8)	/* 2 MHz */
	MDRV_CPU_PROGRAM_MAP(tndrcade_sub_map)
	MDRV_CPU_VBLANK_INT_HACK(tndrcade_sub_interrupt,TNDRCADE_SUB_INTERRUPTS_NUM)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 48*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(tndrcade)
	MDRV_PALETTE_LENGTH(512)	/* sprites only */

	MDRV_VIDEO_START(seta_no_layers)
	MDRV_VIDEO_UPDATE(seta_no_layers) /* just draw the sprites */

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ym1", YM2203, 16000000/4)	/* 4 MHz */
	MDRV_SOUND_CONFIG(tndrcade_ym2203_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.35)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.35)

	MDRV_SOUND_ADD("ym2", YM3812, 16000000/4)	/* 4 MHz */
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
MACHINE_DRIVER_END


/***************************************************************************
                                Twin Eagle
***************************************************************************/

/* Just like metafox, but:
   the sub cpu reads the ip at different locations,
   the visible area seems different. */

/* twineagl lev 3 = lev 2 + lev 1 ! */

static MACHINE_DRIVER_START( twineagl )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 16000000/2)	/* 8 MHz */
	MDRV_CPU_PROGRAM_MAP(downtown_map)
	MDRV_CPU_VBLANK_INT("screen", irq3_line_hold)

	MDRV_CPU_ADD("sub", M65C02, 16000000/8)	/* 2 MHz */
	MDRV_CPU_PROGRAM_MAP(twineagl_sub_map)
	MDRV_CPU_VBLANK_INT_HACK(seta_sub_interrupt,SETA_SUB_INTERRUPTS_NUM)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 48*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(downtown)
	MDRV_PALETTE_LENGTH(512)

	MDRV_VIDEO_START(twineagl_1_layer)
	MDRV_VIDEO_UPDATE(seta)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("x1snd", X1_010, 16000000)	/* 16 MHz */
	MDRV_SOUND_CONFIG(seta_sound_intf)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END


/***************************************************************************
                                DownTown
***************************************************************************/

/* downtown lev 3 = lev 2 + lev 1 ! */

static MACHINE_DRIVER_START( downtown )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, XTAL_16MHz/2) /* verified on pcb */
	MDRV_CPU_PROGRAM_MAP(downtown_map)
	MDRV_CPU_VBLANK_INT_HACK(seta_interrupt_1_and_2,SETA_INTERRUPTS_NUM)

	MDRV_CPU_ADD("sub", M65C02, XTAL_16MHz/8) /* verified on pcb */
	MDRV_CPU_PROGRAM_MAP(downtown_sub_map)
	MDRV_CPU_VBLANK_INT_HACK(seta_sub_interrupt,SETA_SUB_INTERRUPTS_NUM)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(57.42) /* verified on pcb */
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 48*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(downtown)
	MDRV_PALETTE_LENGTH(512)

	MDRV_VIDEO_START(seta_1_layer)
	MDRV_VIDEO_UPDATE(seta)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("x1snd", X1_010, 16000000)	/* 16 MHz */
	MDRV_SOUND_CONFIG(seta_sound_intf)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END


/***************************************************************************
                                U.S. Classic
***************************************************************************/


/*  usclssic lev 6 = lev 2+4 !
    Test mode shows a 16ms and 4ms counters. I wonder if every game has
    5 ints per frame
*/

#define calibr50_INTERRUPTS_NUM (4+1)
static INTERRUPT_GEN( calibr50_interrupt )
{
	switch (cpu_getiloops(device))
	{
		case 0:
		case 1:
		case 2:
		case 3:		cpu_set_input_line(device, 4, HOLD_LINE);	break;
		case 4:		cpu_set_input_line(device, 2, HOLD_LINE);	break;
	}
}


static MACHINE_DRIVER_START( usclssic )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 16000000/2)	/* 8 MHz */
	MDRV_CPU_PROGRAM_MAP(usclssic_map)
	MDRV_CPU_VBLANK_INT_HACK(calibr50_interrupt,calibr50_INTERRUPTS_NUM)

	MDRV_CPU_ADD("sub", M65C02, 16000000/8)	/* 2 MHz */
	MDRV_CPU_PROGRAM_MAP(calibr50_sub_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)	/* NMI caused by main cpu when writing to the sound latch */

	MDRV_MACHINE_RESET(calibr50)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 48*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(usclssic)
	MDRV_PALETTE_LENGTH(16*32 + 64*32)		/* sprites, layer */

	MDRV_PALETTE_INIT(usclssic)	/* layer is 6 planes deep */
	MDRV_VIDEO_START(seta_1_layer)
	MDRV_VIDEO_UPDATE(usclssic)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("x1snd", X1_010, 16000000)	/* 16 MHz */
	MDRV_SOUND_CONFIG(seta_sound_intf2)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END


/***************************************************************************
                                Caliber 50
***************************************************************************/

/*  calibr50 lev 6 = lev 2 + lev 4 !
             lev 3 = lev 2 + lev 1 !
    Test mode shows a 16ms and 4ms counters. I wonder if every game has
    5 ints per frame */

static MACHINE_DRIVER_START( calibr50 )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, XTAL_16MHz/2) /* verified on pcb */
	MDRV_CPU_PROGRAM_MAP(calibr50_map)
	MDRV_CPU_VBLANK_INT_HACK(calibr50_interrupt,calibr50_INTERRUPTS_NUM)

	MDRV_CPU_ADD("sub", M65C02, XTAL_16MHz/8) /* verified on pcb */
	MDRV_CPU_PROGRAM_MAP(calibr50_sub_map)
	MDRV_CPU_VBLANK_INT_HACK(irq0_line_hold,4)	/* IRQ: 4/frame
                               NMI: when the 68k writes the sound latch */

	MDRV_MACHINE_RESET(calibr50)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(57.42)  /* verified on pcb */
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 48*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(downtown)
	MDRV_PALETTE_LENGTH(512)

	MDRV_VIDEO_START(seta_1_layer)
	MDRV_VIDEO_UPDATE(seta)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("x1snd", X1_010, 16000000)	/* 16 MHz */
	MDRV_SOUND_CONFIG(seta_sound_intf2)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END


/***************************************************************************
                                Meta Fox
***************************************************************************/

/* metafox lev 3 = lev 2 + lev 1 ! */

static MACHINE_DRIVER_START( metafox )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 16000000/2)	/* 8 MHz */
	MDRV_CPU_PROGRAM_MAP(downtown_map)
	MDRV_CPU_VBLANK_INT("screen", irq3_line_hold)

	MDRV_CPU_ADD("sub", M65C02, 16000000/8)	/* 2 MHz */
	MDRV_CPU_PROGRAM_MAP(metafox_sub_map)
	MDRV_CPU_VBLANK_INT_HACK(seta_sub_interrupt,SETA_SUB_INTERRUPTS_NUM)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 48*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(downtown)
	MDRV_PALETTE_LENGTH(512)

	MDRV_VIDEO_START(seta_1_layer)
	MDRV_VIDEO_UPDATE(seta)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("x1snd", X1_010, 16000000)	/* 16 MHz */
	MDRV_SOUND_CONFIG(seta_sound_intf)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END


/***************************************************************************
                                Athena no Hatena?
***************************************************************************/

static MACHINE_DRIVER_START( atehate )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 16000000)	/* 16 MHz */
	MDRV_CPU_PROGRAM_MAP(atehate_map)
	MDRV_CPU_VBLANK_INT_HACK(seta_interrupt_1_and_2,SETA_INTERRUPTS_NUM)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 48*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(tndrcade)
	MDRV_PALETTE_LENGTH(512)	/* sprites only */

	MDRV_VIDEO_START(seta_no_layers)
	MDRV_VIDEO_UPDATE(seta_no_layers) /* just draw the sprites */

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("x1snd", X1_010, 16000000)	/* 16 MHz */
	MDRV_SOUND_CONFIG(seta_sound_intf)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END


/***************************************************************************
                                Blandia
***************************************************************************/

/*
    Similar to wrofaero, but the layers are 6 planes deep (and
    the pens are strangely mapped to palette entries) + the
    samples are bankswitched
*/

static MACHINE_DRIVER_START( blandia )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 16000000)	/* 16 MHz */
	MDRV_CPU_PROGRAM_MAP(blandia_map)
	MDRV_CPU_VBLANK_INT_HACK(seta_interrupt_2_and_4,SETA_INTERRUPTS_NUM)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 48*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(blandia)
	MDRV_PALETTE_LENGTH(16*32+64*32+64*32)	/* sprites, layer1, layer2 */

	MDRV_PALETTE_INIT(blandia)				/* layers 1&2 are 6 planes deep */
	MDRV_VIDEO_START(seta_2_layers)
	MDRV_VIDEO_EOF(seta_buffer_sprites)		/* Blandia uses sprite buffering */
	MDRV_VIDEO_UPDATE(seta)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("x1snd", X1_010, 16000000)	/* 16 MHz */
	MDRV_SOUND_CONFIG(seta_sound_intf)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( blandiap )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 16000000)	/* 16 MHz */
	MDRV_CPU_PROGRAM_MAP(blandiap_map)
	MDRV_CPU_VBLANK_INT_HACK(seta_interrupt_2_and_4,SETA_INTERRUPTS_NUM)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 48*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(blandia)
	MDRV_PALETTE_LENGTH(16*32+64*32+64*32)	/* sprites, layer1, layer2 */

	MDRV_PALETTE_INIT(blandia)				/* layers 1&2 are 6 planes deep */
	MDRV_VIDEO_START(seta_2_layers)
	MDRV_VIDEO_EOF(seta_buffer_sprites)		/* Blandia uses sprite buffering */
	MDRV_VIDEO_UPDATE(seta)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("x1snd", X1_010, 16000000)	/* 16 MHz */
	MDRV_SOUND_CONFIG(seta_sound_intf)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END


/***************************************************************************
                                Block Carnival
***************************************************************************/

static MACHINE_DRIVER_START( blockcar )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 16000000/2)	/* 8 MHz */
	MDRV_CPU_PROGRAM_MAP(blockcar_map)
	MDRV_CPU_VBLANK_INT("screen", irq3_line_hold)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 48*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(tndrcade)
	MDRV_PALETTE_LENGTH(512)	/* sprites only */

	MDRV_VIDEO_START(seta_no_layers)
	MDRV_VIDEO_UPDATE(seta_no_layers) /* just draw the sprites */

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("x1snd", X1_010, 16000000)	/* 16 MHz */
	MDRV_SOUND_CONFIG(seta_sound_intf)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END


/***************************************************************************
                                Daioh
***************************************************************************/

static MACHINE_DRIVER_START( daioh )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 16000000)	/* 16 MHz */
	MDRV_CPU_PROGRAM_MAP(daioh_map)
	MDRV_CPU_VBLANK_INT_HACK(seta_interrupt_1_and_2,SETA_INTERRUPTS_NUM)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 48*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(msgundam)
	MDRV_PALETTE_LENGTH(512 * 3)	/* sprites, layer1, layer2 */

	MDRV_VIDEO_START(seta_2_layers)
	MDRV_VIDEO_UPDATE(seta)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("x1snd", X1_010, 16000000)	/* 16 MHz */
	MDRV_SOUND_CONFIG(seta_sound_intf)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END

/***************************************************************************
                Dragon Unit, Quiz Kokology, Strike Gunner
***************************************************************************/

/*
    drgnunit,qzkklogy,stg:
    lev 1 == lev 3 (writes to $500000, bit 4 -> 1 then 0)
    lev 2 drives the game
*/

static MACHINE_DRIVER_START( drgnunit )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 16000000/2)	/* 8 MHz */
	MDRV_CPU_PROGRAM_MAP(drgnunit_map)
	MDRV_CPU_VBLANK_INT_HACK(seta_interrupt_1_and_2,SETA_INTERRUPTS_NUM)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 48*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(downtown)
	MDRV_PALETTE_LENGTH(512)

	MDRV_VIDEO_START(seta_1_layer)
	MDRV_VIDEO_EOF(seta_buffer_sprites)	/* qzkklogy uses sprite buffering */
	MDRV_VIDEO_UPDATE(seta)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("x1snd", X1_010, 16000000)	/* 16 MHz */
	MDRV_SOUND_CONFIG(seta_sound_intf)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END

/*  Same as qzkklogy, but with a 16MHz CPU and different
    layout for the layer's tiles    */

static MACHINE_DRIVER_START( qzkklgy2 )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 16000000)	/* 16 MHz */
	MDRV_CPU_PROGRAM_MAP(drgnunit_map)
	MDRV_CPU_VBLANK_INT_HACK(seta_interrupt_1_and_2,SETA_INTERRUPTS_NUM)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 48*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(qzkklgy2)
	MDRV_PALETTE_LENGTH(512)

	MDRV_VIDEO_START(seta_1_layer)
	MDRV_VIDEO_EOF(seta_buffer_sprites)	/* qzkklogy uses sprite buffering */
	MDRV_VIDEO_UPDATE(seta)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("x1snd", X1_010, 16000000)	/* 16 MHz */
	MDRV_SOUND_CONFIG(seta_sound_intf)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END

/***************************************************************************
                                Seta Roulette
***************************************************************************/

#define SETAROUL_INTERRUPTS_NUM (2)
static INTERRUPT_GEN( setaroul_interrupt )
{
	switch (cpu_getiloops(device))
	{
		case 0:		cpu_set_input_line(device, 4, HOLD_LINE);	break;
		case 1:		cpu_set_input_line(device, 2, HOLD_LINE);	break;
	}
}


static MACHINE_DRIVER_START( setaroul )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 16000000/2)	/* 8 MHz */
	MDRV_CPU_PROGRAM_MAP(setaroul_map)
	MDRV_CPU_VBLANK_INT_HACK(setaroul_interrupt,SETAROUL_INTERRUPTS_NUM) // and 6?

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 48*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(setaroul)
	MDRV_PALETTE_LENGTH(512)
	MDRV_PALETTE_INIT(setaroul)

	MDRV_VIDEO_START(seta_1_layer)
	MDRV_VIDEO_EOF(seta_buffer_sprites)	/* qzkklogy uses sprite buffering */
	MDRV_VIDEO_UPDATE(seta)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("x1snd", X1_010, 16000000)	/* 16 MHz */
	MDRV_SOUND_CONFIG(seta_sound_intf)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END


/***************************************************************************
                                Eight Force
***************************************************************************/

static MACHINE_DRIVER_START( eightfrc )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 16000000)	/* 16 MHz */
	MDRV_CPU_PROGRAM_MAP(wrofaero_map)
	MDRV_CPU_VBLANK_INT_HACK(seta_interrupt_1_and_2,SETA_INTERRUPTS_NUM)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 48*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(msgundam)
	MDRV_PALETTE_LENGTH(512 * 3)	/* sprites, layer1, layer2 */

	MDRV_VIDEO_START(seta_2_layers)
	MDRV_VIDEO_UPDATE(seta)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("x1snd", X1_010, 16000000)	/* 16 MHz */
	MDRV_SOUND_CONFIG(seta_sound_intf)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END


/***************************************************************************
                        Extreme Downhill / Sokonuke
***************************************************************************/

/*
    extdwnhl:
    lev 1 == lev 3 (writes to $500000, bit 4 -> 1 then 0)
    lev 2 drives the game
*/
static MACHINE_DRIVER_START( extdwnhl )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 16000000)	/* 16 MHz */
	MDRV_CPU_PROGRAM_MAP(extdwnhl_map)
	MDRV_CPU_VBLANK_INT_HACK(seta_interrupt_1_and_2,SETA_INTERRUPTS_NUM)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(zingzip)
	MDRV_PALETTE_LENGTH(16*32+16*32+64*32)	/* sprites, layer2, layer1 */

	MDRV_PALETTE_INIT(zingzip)			/* layer 1 gfx is 6 planes deep */
	MDRV_VIDEO_START(seta_2_layers)
	MDRV_VIDEO_UPDATE(seta)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("x1snd", X1_010, 16000000)	/* 16 MHz */
	MDRV_SOUND_CONFIG(seta_sound_intf)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END


/***************************************************************************
                                Gundhara
***************************************************************************/
#if __uPD71054_TIMER
static INTERRUPT_GEN( wrofaero_interrupt )
{
	cpu_set_input_line(device, 2, HOLD_LINE );
}

static MACHINE_START( wrofaero ) { uPD71054_timer_init(machine); }
#endif	// __uPD71054_TIMER



/*
    lev 1: sample end? (needed in zombraid otherwise music stops)
           gundhara's debug code calls it "BUT_IPL" and does nothing
    lev 2: VBlank
    lev 4: Sound (generated by a timer mapped at $d00000-6 ?)
*/
static MACHINE_DRIVER_START( gundhara )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 16000000)	/* 16 MHz */
	MDRV_CPU_PROGRAM_MAP(wrofaero_map)
#if	__uPD71054_TIMER
//  MDRV_CPU_VBLANK_INT("screen", wrofaero_interrupt)
	MDRV_CPU_VBLANK_INT_HACK(seta_interrupt_1_and_2,SETA_INTERRUPTS_NUM)
#else
	MDRV_CPU_VBLANK_INT_HACK(seta_interrupt_2_and_4,SETA_INTERRUPTS_NUM)
#endif	// __uPD71054_TIMER

#if	__uPD71054_TIMER
	MDRV_MACHINE_START( wrofaero )
#endif	// __uPD71054_TIMER

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 48*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(jjsquawk)
	MDRV_PALETTE_LENGTH(16*32+64*32+64*32)	/* sprites, layer2, layer1 */

	MDRV_PALETTE_INIT(gundhara)				/* layers are 6 planes deep (but have only 4 palettes) */
	MDRV_VIDEO_START(seta_2_layers)
	MDRV_VIDEO_UPDATE(seta)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("x1snd", X1_010, 16000000)	/* 16 MHz */
	MDRV_SOUND_CONFIG(seta_sound_intf)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END


/***************************************************************************
                                J.J.Squawkers
***************************************************************************/

/*
    lev 1 == lev 3 (writes to $500000, bit 4 -> 1 then 0)
    lev 2 drives the game
*/
static MACHINE_DRIVER_START( jjsquawk )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 16000000)	/* 16 MHz */
	MDRV_CPU_PROGRAM_MAP(wrofaero_map)
	MDRV_CPU_VBLANK_INT_HACK(seta_interrupt_1_and_2,SETA_INTERRUPTS_NUM)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 48*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(jjsquawk)
	MDRV_PALETTE_LENGTH(16*32+64*32+64*32)	/* sprites, layer2, layer1 */

	MDRV_PALETTE_INIT(jjsquawk)				/* layers are 6 planes deep */
	MDRV_VIDEO_START(seta_2_layers)
	MDRV_VIDEO_UPDATE(seta)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("x1snd", X1_010, 16000000)	/* 16 MHz */
	MDRV_SOUND_CONFIG(seta_sound_intf)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( jjsquawb )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 16000000)	/* 16 MHz */
	MDRV_CPU_PROGRAM_MAP(jjsquawb_map)
	MDRV_CPU_VBLANK_INT_HACK(seta_interrupt_1_and_2,SETA_INTERRUPTS_NUM)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 48*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(jjsquawk)
	MDRV_PALETTE_LENGTH(16*32+64*32+64*32)	/* sprites, layer2, layer1 */

	MDRV_PALETTE_INIT(jjsquawk)				/* layers are 6 planes deep */
	MDRV_VIDEO_START(seta_2_layers)
	MDRV_VIDEO_UPDATE(seta)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("x1snd", X1_010, 16000000)	/* 16 MHz */
	MDRV_SOUND_CONFIG(seta_sound_intf)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END

/***************************************************************************
                (Kamen) Masked Riders Club Battle Race
***************************************************************************/

/*  kamenrid: lev 2 by vblank, lev 4 by timer */
static MACHINE_DRIVER_START( kamenrid )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 16000000)	/* 16 MHz */
	MDRV_CPU_PROGRAM_MAP(kamenrid_map)
	MDRV_CPU_VBLANK_INT("screen", wrofaero_interrupt)

#if	__uPD71054_TIMER
	MDRV_MACHINE_START( wrofaero )
#endif	// __uPD71054_TIMER

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 48*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(msgundam)
	MDRV_PALETTE_LENGTH(512 * 3)	/* sprites, layer2, layer1 */

	MDRV_VIDEO_START(seta_2_layers)
	MDRV_VIDEO_UPDATE(seta)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("x1snd", X1_010, 16000000)	/* 16 MHz */
	MDRV_SOUND_CONFIG(seta_sound_intf)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END

/***************************************************************************
                                Orbs
***************************************************************************/

/* The CPU clock has been verified/measured, PCB only has one OSC and it's 14.318180 MHz */

static MACHINE_DRIVER_START( orbs )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 14318180/2)	/* 7.143 MHz */
	MDRV_CPU_PROGRAM_MAP(orbs_map)
	MDRV_CPU_VBLANK_INT_HACK(seta_interrupt_1_and_2,SETA_INTERRUPTS_NUM)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(1*8, 39*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(orbs)
	MDRV_PALETTE_LENGTH(512)	/* sprites only */

	MDRV_VIDEO_START(seta_no_layers)
	MDRV_VIDEO_UPDATE(seta_no_layers) /* just draw the sprites */

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("x1snd", X1_010, 14318180)	/* 14.318180 MHz */
	MDRV_SOUND_CONFIG(seta_sound_intf)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END


/***************************************************************************
                  Kero Kero Keroppi no Issyoni Asobou
***************************************************************************/

static MACHINE_DRIVER_START( keroppi )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 14318180/2)	/* 7.143 MHz */
	MDRV_CPU_PROGRAM_MAP(keroppi_map)
	MDRV_CPU_VBLANK_INT_HACK(seta_interrupt_1_and_2,SETA_INTERRUPTS_NUM)

	MDRV_MACHINE_START(keroppi)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(orbs)
	MDRV_PALETTE_LENGTH(512)	/* sprites only */

	MDRV_VIDEO_START(seta_no_layers)
	MDRV_VIDEO_UPDATE(seta_no_layers) /* just draw the sprites */

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("x1snd", X1_010, 14318180)	/* 14.318180 MHz */
	MDRV_SOUND_CONFIG(seta_sound_intf)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END


/***************************************************************************
                                Krazy Bowl
***************************************************************************/

static MACHINE_DRIVER_START( krzybowl )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 16000000)	/* 16 MHz */
	MDRV_CPU_PROGRAM_MAP(krzybowl_map)
	MDRV_CPU_VBLANK_INT_HACK(seta_interrupt_1_and_2,SETA_INTERRUPTS_NUM)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(1*8, 39*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(tndrcade)
	MDRV_PALETTE_LENGTH(512)	/* sprites only */

	MDRV_VIDEO_START(seta_no_layers)
	MDRV_VIDEO_UPDATE(seta_no_layers) /* just draw the sprites */

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("x1snd", X1_010, 16000000)	/* 16 MHz */
	MDRV_SOUND_CONFIG(seta_sound_intf)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END


/***************************************************************************
                                Mad Shark
***************************************************************************/

/*  madshark: lev 2 by vblank, lev 4 by timer */
static MACHINE_DRIVER_START( madshark )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 16000000)	/* 16 MHz */
	MDRV_CPU_PROGRAM_MAP(madshark_map)
	MDRV_CPU_VBLANK_INT("screen", wrofaero_interrupt)

#if	__uPD71054_TIMER
	MDRV_MACHINE_START( wrofaero )
#endif	// __uPD71054_TIMER

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 48*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(jjsquawk)
	MDRV_PALETTE_LENGTH(16*32+64*32+64*32)	/* sprites, layer2, layer1 */

	MDRV_PALETTE_INIT(jjsquawk)				/* layers are 6 planes deep */

	MDRV_VIDEO_START(seta_2_layers)
	MDRV_VIDEO_UPDATE(seta)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("x1snd", X1_010, 16000000)	/* 16 MHz */
	MDRV_SOUND_CONFIG(seta_sound_intf)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END

/***************************************************************************
                            Mobile Suit Gundam
***************************************************************************/

/* msgundam lev 2 == lev 6 ! */

static MACHINE_DRIVER_START( msgundam )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 16000000)	/* 16 MHz */
	MDRV_CPU_PROGRAM_MAP(msgundam_map)
#if	__uPD71054_TIMER
	MDRV_CPU_VBLANK_INT("screen", wrofaero_interrupt)
#else
	MDRV_CPU_VBLANK_INT_HACK(seta_interrupt_2_and_4,SETA_INTERRUPTS_NUM)
#endif	// __uPD71054_TIMER

#if	__uPD71054_TIMER
	MDRV_MACHINE_START( wrofaero )
#endif	// __uPD71054_TIMER

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(56.66)	/* between 56 and 57 to match a real PCB's game speed */
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 48*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(msgundam)
	MDRV_PALETTE_LENGTH(512 * 3)	/* sprites, layer2, layer1 */

	MDRV_VIDEO_START(seta_2_layers)
	MDRV_VIDEO_EOF(seta_buffer_sprites)	/* msgundam uses sprite buffering */
	MDRV_VIDEO_UPDATE(seta)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("x1snd", X1_010, 16000000)	/* 16 MHz */
	MDRV_SOUND_CONFIG(seta_sound_intf)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END



/***************************************************************************
                            Oishii Puzzle
***************************************************************************/

static MACHINE_DRIVER_START( oisipuzl )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 16000000)	/* 16 MHz */
	MDRV_CPU_PROGRAM_MAP(oisipuzl_map)
	MDRV_CPU_VBLANK_INT_HACK(seta_interrupt_1_and_2,SETA_INTERRUPTS_NUM)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(msgundam)
	MDRV_PALETTE_LENGTH(512 * 3)	/* sprites, layer2, layer1 */

	MDRV_VIDEO_START(oisipuzl_2_layers)	// flip is inverted for the tilemaps
	MDRV_VIDEO_UPDATE(seta)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("x1snd", X1_010, 16000000)	/* 16 MHz */
	MDRV_SOUND_CONFIG(seta_sound_intf)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END

/***************************************************************************
                            Triple Fun
***************************************************************************/

/* same as oisipuzl but with different interrupts and sound */

static MACHINE_DRIVER_START( triplfun )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 16000000)	/* 16 MHz */
	MDRV_CPU_PROGRAM_MAP(triplfun_map)
	MDRV_CPU_VBLANK_INT("screen", irq3_line_hold)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(msgundam)
	MDRV_PALETTE_LENGTH(512 * 3)	/* sprites, layer2, layer1 */

	MDRV_VIDEO_START(oisipuzl_2_layers)	// flip is inverted for the tilemaps
	MDRV_VIDEO_UPDATE(seta)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_OKIM6295_ADD("oki", 792000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
MACHINE_DRIVER_END

/***************************************************************************
                            Pro Mahjong Kiwame
***************************************************************************/

static MACHINE_DRIVER_START( kiwame )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 16000000)	/* 16 MHz */
	MDRV_CPU_PROGRAM_MAP(kiwame_map)
	MDRV_CPU_VBLANK_INT("screen", irq1_line_hold)/* lev 1-7 are the same. WARNING:
                                   the interrupt table is written to. */

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 56*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(tndrcade)
	MDRV_PALETTE_LENGTH(512)	/* sprites only */

	MDRV_VIDEO_START(seta_no_layers)
	MDRV_VIDEO_UPDATE(seta_no_layers) /* just draw the sprites */

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("x1snd", X1_010, 16000000)	/* 16 MHz */
	MDRV_SOUND_CONFIG(seta_sound_intf)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END



/***************************************************************************
                                    Rezon
***************************************************************************/

/* pretty much like wrofaero, but ints are 1&2, not 2&4 */

static MACHINE_DRIVER_START( rezon )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 16000000)	/* 16 MHz */
	MDRV_CPU_PROGRAM_MAP(wrofaero_map)
	MDRV_CPU_VBLANK_INT_HACK(seta_interrupt_1_and_2,SETA_INTERRUPTS_NUM)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 48*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(msgundam)
	MDRV_PALETTE_LENGTH(512 * 3)	/* sprites, layer1, layer2 */

	MDRV_VIDEO_START(seta_2_layers)
	MDRV_VIDEO_UPDATE(seta)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("x1snd", X1_010, 16000000)	/* 16 MHz */
	MDRV_SOUND_CONFIG(seta_sound_intf)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END



/***************************************************************************
                        Thunder & Lightning / Wit's
***************************************************************************/

/*  thunderl lev 2 = lev 3 - other levels lead to an error */

static MACHINE_DRIVER_START( thunderl )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 16000000/2)	/* 8 MHz */
	MDRV_CPU_PROGRAM_MAP(thunderl_map)
	MDRV_CPU_VBLANK_INT("screen", irq2_line_hold)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 48*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(tndrcade)
	MDRV_PALETTE_LENGTH(512)	/* sprites only */

	MDRV_VIDEO_START(seta_no_layers)
	MDRV_VIDEO_UPDATE(seta_no_layers) /* just draw the sprites */

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("x1snd", X1_010, 16000000)	/* 16 MHz */
	MDRV_SOUND_CONFIG(seta_sound_intf)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( wiggie )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 16000000/2)	/* 8 MHz */
	MDRV_CPU_PROGRAM_MAP(wiggie_map)
	MDRV_CPU_VBLANK_INT("screen", irq2_line_hold)

	MDRV_CPU_ADD("audiocpu", Z80, 16000000/4)	/* 4 MHz */
	MDRV_CPU_PROGRAM_MAP(wiggie_sound_map)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 48*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(wiggie)
	MDRV_PALETTE_LENGTH(512)	/* sprites only */

	MDRV_VIDEO_START(seta_no_layers)
	MDRV_VIDEO_UPDATE(seta_no_layers) /* just draw the sprites */

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_OKIM6295_ADD("oki", 1000000, OKIM6295_PIN7_HIGH)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( superbar )
	MDRV_IMPORT_FROM( wiggie )

	MDRV_GFXDECODE(superbar)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( wits )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 16000000/2)	/* 8 MHz */
	MDRV_CPU_PROGRAM_MAP(thunderl_map)
	MDRV_CPU_VBLANK_INT("screen", irq2_line_hold)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 48*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(tndrcade)
	MDRV_PALETTE_LENGTH(512)	/* sprites only */

	MDRV_VIDEO_START(seta_no_layers)
	MDRV_VIDEO_UPDATE(seta_no_layers) /* just draw the sprites */

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("x1snd", X1_010, 16000000)	/* 16 MHz */
	MDRV_SOUND_CONFIG(seta_sound_intf)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END


/***************************************************************************
                    Ultraman Club / SD Gundam Neo Battling
***************************************************************************/

static MACHINE_DRIVER_START( umanclub )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 16000000)	/* 16 MHz */
	MDRV_CPU_PROGRAM_MAP(umanclub_map)
	MDRV_CPU_VBLANK_INT("screen", irq3_line_hold)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 48*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(tndrcade)
	MDRV_PALETTE_LENGTH(512)

	MDRV_VIDEO_START(seta_no_layers)
	MDRV_VIDEO_UPDATE(seta_no_layers) /* just draw the sprites */

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("x1snd", X1_010, 16000000)	/* 16 MHz */
	MDRV_SOUND_CONFIG(seta_sound_intf)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END


/***************************************************************************
                            Ultra Toukond Densetsu
***************************************************************************/

static MACHINE_DRIVER_START( utoukond )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 16000000)	/* 16 MHz */
	MDRV_CPU_PROGRAM_MAP(utoukond_map)
	MDRV_CPU_VBLANK_INT_HACK(seta_interrupt_1_and_2,SETA_INTERRUPTS_NUM)

	MDRV_CPU_ADD("audiocpu", Z80, 16000000/4)	/* 4 MHz */
	MDRV_CPU_PROGRAM_MAP(utoukond_sound_map)
	MDRV_CPU_IO_MAP(utoukond_sound_io_map)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 48*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(msgundam)
	MDRV_PALETTE_LENGTH(512 * 3)	/* sprites, layer2, layer1 */

	MDRV_VIDEO_START(seta_2_layers)
	MDRV_VIDEO_UPDATE(seta)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("x1snd", X1_010, 16000000)
	MDRV_SOUND_CONFIG(seta_sound_intf)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)

	MDRV_SOUND_ADD("ymsnd", YM3438, 16000000/4)	/* 4 MHz */
	MDRV_SOUND_CONFIG(utoukond_ym3438_intf)
	MDRV_SOUND_ROUTE(0, "lspeaker", 0.30)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.30)
MACHINE_DRIVER_END


/***************************************************************************
                                War of Aero
***************************************************************************/

static MACHINE_DRIVER_START( wrofaero )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 16000000)	/* 16 MHz */
	MDRV_CPU_PROGRAM_MAP(wrofaero_map)
#if	__uPD71054_TIMER
	MDRV_CPU_VBLANK_INT("screen", wrofaero_interrupt)
#else
	MDRV_CPU_VBLANK_INT_HACK(seta_interrupt_2_and_4,SETA_INTERRUPTS_NUM)
#endif	// __uPD71054_TIMER

#if	__uPD71054_TIMER
	MDRV_MACHINE_START( wrofaero )
#endif	// __uPD71054_TIMER

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 48*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(msgundam)
	MDRV_PALETTE_LENGTH(512 * 3)	/* sprites, layer1, layer2 */

	MDRV_VIDEO_START(seta_2_layers)
	MDRV_VIDEO_UPDATE(seta)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("x1snd", X1_010, 16000000)	/* 16 MHz */
	MDRV_SOUND_CONFIG(seta_sound_intf)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END




/***************************************************************************
                                Zing Zing Zip
***************************************************************************/

/* zingzip lev 3 = lev 2 + lev 1 !
   SR = 2100 -> lev1 is ignored so we must supply int 3, since the routine
   at int 1 is necessary: it plays the background music.
*/

static MACHINE_DRIVER_START( zingzip )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 16000000)	/* 16 MHz */
	MDRV_CPU_PROGRAM_MAP(wrofaero_map)
	MDRV_CPU_VBLANK_INT("screen", irq3_line_hold)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 48*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(zingzip)
	MDRV_PALETTE_LENGTH(16*32+16*32+64*32)	/* sprites, layer2, layer1 */

	MDRV_PALETTE_INIT(zingzip)				/* layer 1 gfx is 6 planes deep */
	MDRV_VIDEO_START(seta_2_layers)
	MDRV_VIDEO_UPDATE(seta)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("x1snd", X1_010, 16000000)	/* 16 MHz */
	MDRV_SOUND_CONFIG(seta_sound_intf)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END

/***************************************************************************
                                Pairs Love
***************************************************************************/

static MACHINE_DRIVER_START( pairlove )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 16000000/2)	/* 8 MHz */
	MDRV_CPU_PROGRAM_MAP(pairlove_map)
	MDRV_CPU_VBLANK_INT_HACK(seta_interrupt_1_and_2,SETA_INTERRUPTS_NUM)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 48*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(tndrcade)
	MDRV_PALETTE_LENGTH(2048)	/* sprites only */

	MDRV_VIDEO_START(seta_no_layers)
	MDRV_VIDEO_UPDATE(seta_no_layers) /* just draw the sprites */

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("x1snd", X1_010, 16000000)	/* 16 MHz */
	MDRV_SOUND_CONFIG(seta_sound_intf)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END


/***************************************************************************
                                Crazy Fight
***************************************************************************/

static INTERRUPT_GEN( crazyfgt_interrupt )
{
	switch (cpu_getiloops(device))
	{
		case 0:		cpu_set_input_line(device, 1, HOLD_LINE);	break;
		default:	cpu_set_input_line(device, 2, HOLD_LINE);	break;	// should this be triggered by the 3812?
	}
}

static MACHINE_DRIVER_START( crazyfgt )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 16000000)	/* 16 MHz */
	MDRV_CPU_PROGRAM_MAP(crazyfgt_map)
	MDRV_CPU_VBLANK_INT_HACK(crazyfgt_interrupt,1+5)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 48*8-1, 2*8-4, 30*8-1-4)

	MDRV_GFXDECODE(crazyfgt)
	MDRV_PALETTE_LENGTH(16*32+64*32+64*32)	/* sprites, layer1, layer2 */

	MDRV_PALETTE_INIT(gundhara)				/* layers are 6 planes deep (but have only 4 palettes) */
	MDRV_VIDEO_START(seta_2_layers)
	MDRV_VIDEO_UPDATE(seta)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ymsnd", YM3812, 16000000/4)	/* 4 MHz */
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MDRV_OKIM6295_ADD("oki", 1000000, OKIM6295_PIN7_HIGH)	// clock?
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END

/***************************************************************************
                             International Toote
***************************************************************************/

// Test mode shows a 16ms and 2ms counters
static INTERRUPT_GEN( inttoote_interrupt )
{
	switch (cpu_getiloops(device))
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:		cpu_set_input_line(device, 6, HOLD_LINE);	break;

		case 8:		cpu_set_input_line(device, 2, HOLD_LINE);	break;

		case 9:		cpu_set_input_line(device, 1, HOLD_LINE);	break;

		case 10:	cpu_set_input_line(device, 4, HOLD_LINE);	break;
	}
}

static const pia6821_interface inttoote_pia0_intf =
{
	DEVCB_NULL,		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_NULL,		/* port A out */
	DEVCB_NULL,		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_NULL,		/* port CB2 out */
	DEVCB_NULL,		/* IRQA */
	DEVCB_NULL		/* IRQB */
};

static const pia6821_interface inttoote_pia1_intf =
{
	DEVCB_NULL,		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_NULL,		/* port A out */
	DEVCB_NULL,		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_NULL,		/* port CB2 out */
	DEVCB_NULL,		/* IRQA */
	DEVCB_NULL		/* IRQB */
};

static MACHINE_DRIVER_START( inttoote )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 16000000)
	MDRV_CPU_PROGRAM_MAP(inttoote_map)
	MDRV_CPU_VBLANK_INT_HACK(inttoote_interrupt,8+1+1+1)

	MDRV_PIA6821_ADD("pia0", inttoote_pia0_intf)
	MDRV_PIA6821_ADD("pia1", inttoote_pia1_intf)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 48*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(inttoote)
	MDRV_PALETTE_LENGTH(512 * 1)

	MDRV_PALETTE_INIT(inttoote)
	MDRV_VIDEO_START(seta_1_layer)
	MDRV_VIDEO_UPDATE(inttoote)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("x1snd", X1_010, 16000000)
	MDRV_SOUND_CONFIG(seta_sound_intf)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)

	/* devices */
	MDRV_MSM6242_ADD("rtc")
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( jockeyc )
	MDRV_IMPORT_FROM( inttoote )
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(jockeyc_map)
MACHINE_DRIVER_END



/***************************************************************************

                                ROMs Loading

***************************************************************************/

ROM_START( tndrcade )
	ROM_REGION( 0x080000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "ua0-4.u19", 0x000000, 0x020000, CRC(73bd63eb) SHA1(5d410d2a77f1c3c4c37a9fe1e56019335891fe67) )
	ROM_LOAD16_BYTE( "ua0-2.u17", 0x000001, 0x020000, CRC(e96194b1) SHA1(c5084d06a2e4f7ba3112be1ccc314f7d712bb45e) )
	ROM_LOAD16_BYTE( "ua0-3.u18", 0x040000, 0x020000, CRC(0a7b1c41) SHA1(ede14ac08d7e63972c21fd2d0717276e73153f18) )
	ROM_LOAD16_BYTE( "ua0-1.u16", 0x040001, 0x020000, CRC(fa906626) SHA1(a1d28328afa8dda98dd20f3f5a19c0dbf2ebaf36) )

	ROM_REGION( 0x02c000, "sub", 0 )		/* 65c02 Code */
	ROM_LOAD( "ua10-5.u24", 0x004000, 0x020000, CRC(8eff6122) SHA1(1adc1643018e612df85643014b78525106478889) )	// $1fffd=2 (country code)
	ROM_RELOAD(             0x00c000, 0x020000  )

	ROM_REGION( 0x200000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "ua0-10.u12", 0x000000, 0x040000, CRC(aa7b6757) SHA1(9157cc930760c846cce95e18bf38e7ea241f7a8e) )
	ROM_LOAD( "ua0-11.u13", 0x040000, 0x040000, CRC(11eaf931) SHA1(ba1dfc4b0f87b1bbdc6c2e36deaecda2b4655d57) )
	ROM_LOAD( "ua0-12.u14", 0x080000, 0x040000, CRC(00b5381c) SHA1(6fc3138dd0e2b3f99872b1f0d177094df5bed39d) )
	ROM_LOAD( "ua0-13.u15", 0x0c0000, 0x040000, CRC(8f9a0ed3) SHA1(61315312fdb2fe090cd8e99a1ce3ecba46b466e9) )
	ROM_LOAD( "ua0-6.u8",   0x100000, 0x040000, CRC(14ecc7bb) SHA1(920983f5086462f8f06dc9cf7bebffeeb7187977) )
	ROM_LOAD( "ua0-7.u9",   0x140000, 0x040000, CRC(ff1a4e68) SHA1(d732df7d139995814969a603c9c4e9f8b068b1a3) )
	ROM_LOAD( "ua0-8.u10",  0x180000, 0x040000, CRC(936e1884) SHA1(9ad495b88e124d08a7141611ed1897b6e2abd412) )
	ROM_LOAD( "ua0-9.u11",  0x1c0000, 0x040000, CRC(e812371c) SHA1(b0e1e0c143da743bf9f7b48d657594e76f4970ed) )
ROM_END

ROM_START( tndrcadej )
	ROM_REGION( 0x080000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "ua0-4.u19", 0x000000, 0x020000, CRC(73bd63eb) SHA1(5d410d2a77f1c3c4c37a9fe1e56019335891fe67) )
	ROM_LOAD16_BYTE( "ua0-2.u17", 0x000001, 0x020000, CRC(e96194b1) SHA1(c5084d06a2e4f7ba3112be1ccc314f7d712bb45e) )
	ROM_LOAD16_BYTE( "ua0-3.u18", 0x040000, 0x020000, CRC(0a7b1c41) SHA1(ede14ac08d7e63972c21fd2d0717276e73153f18) )
	ROM_LOAD16_BYTE( "ua0-1.u16", 0x040001, 0x020000, CRC(fa906626) SHA1(a1d28328afa8dda98dd20f3f5a19c0dbf2ebaf36) )

	ROM_REGION( 0x02c000, "sub", 0 )		/* 65c02 Code */
	ROM_LOAD( "thcade5.u24", 0x004000, 0x020000, CRC(8cb9df7b) SHA1(5b504657f4cc1ea265913ff670aed108ceddba46) )	// $1fffd=1 (country code jp)
	ROM_RELOAD(              0x00c000, 0x020000  )

	ROM_REGION( 0x200000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "ua0-10.u12", 0x000000, 0x040000, CRC(aa7b6757) SHA1(9157cc930760c846cce95e18bf38e7ea241f7a8e) )
	ROM_LOAD( "ua0-11.u13", 0x040000, 0x040000, CRC(11eaf931) SHA1(ba1dfc4b0f87b1bbdc6c2e36deaecda2b4655d57) )
	ROM_LOAD( "ua0-12.u14", 0x080000, 0x040000, CRC(00b5381c) SHA1(6fc3138dd0e2b3f99872b1f0d177094df5bed39d) )
	ROM_LOAD( "ua0-13.u15", 0x0c0000, 0x040000, CRC(8f9a0ed3) SHA1(61315312fdb2fe090cd8e99a1ce3ecba46b466e9) )
	ROM_LOAD( "ua0-6.u8",   0x100000, 0x040000, CRC(14ecc7bb) SHA1(920983f5086462f8f06dc9cf7bebffeeb7187977) )
	ROM_LOAD( "ua0-7.u9",   0x140000, 0x040000, CRC(ff1a4e68) SHA1(d732df7d139995814969a603c9c4e9f8b068b1a3) )
	ROM_LOAD( "ua0-8.u10",  0x180000, 0x040000, CRC(936e1884) SHA1(9ad495b88e124d08a7141611ed1897b6e2abd412) )
	ROM_LOAD( "ua0-9.u11",  0x1c0000, 0x040000, CRC(e812371c) SHA1(b0e1e0c143da743bf9f7b48d657594e76f4970ed) )
ROM_END

ROM_START( twineagl )
	ROM_REGION( 0x0a0000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_WORD( "ua2-1", 0x000000, 0x080000, CRC(5c3fe531) SHA1(e484dad25cda906fb1b0606fb10ae50056c64e6a) )

	ROM_REGION( 0x010000, "sub", 0 )		/* 65c02 Code */
	ROM_LOAD( "ua2-2", 0x006000, 0x002000, CRC(783ca84e) SHA1(21e19f74812de50e98b755dd1f68c187dd1e7e81) )
	ROM_RELOAD(        0x008000, 0x002000  )
	ROM_RELOAD(        0x00a000, 0x002000  )
	ROM_RELOAD(        0x00c000, 0x002000  )
	ROM_RELOAD(        0x00e000, 0x002000  )

	ROM_REGION( 0x100000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD16_BYTE( "ua2-4",  0x000000, 0x040000, CRC(8b7532d6) SHA1(ec42d21bc44f004282f822b3da36b5442eabd87a) )
	ROM_LOAD16_BYTE( "ua2-3",  0x000001, 0x040000, CRC(1124417a) SHA1(c908f51b943188946486c639a0cb9712114b5437) )
	ROM_LOAD16_BYTE( "ua2-6",  0x080000, 0x040000, CRC(99d8dbba) SHA1(ac2a3c5cad23e0207eba52935c72e23203c8e0af) )
	ROM_LOAD16_BYTE( "ua2-5",  0x080001, 0x040000, CRC(6e450d28) SHA1(d0050afcc3f425ac70768271c9d2d55ab7fba622) )

	ROM_REGION( 0x200000, "gfx2", 0 )	/* Layer 1 */
	ROM_LOAD( "ua2-8",  0x000000, 0x080000, CRC(7d3a8d73) SHA1(d6a0bea124d7d228818f8ea8c804ad2ba8cead4b) )
	ROM_LOAD( "ua2-10", 0x080000, 0x080000, CRC(5bbe1f56) SHA1(309bc43884816dafeb0f47e71ff5272d4d7cac54) )
	ROM_LOAD( "ua2-7",  0x100000, 0x080000, CRC(fce56907) SHA1(5d0d2d6dfdbadb21f1d61d84b8992ec0e527e18d) )
	ROM_LOAD( "ua2-9",  0x180000, 0x080000, CRC(a451eae9) SHA1(c236c92d9ecf56f8d8f4a5ee493e3791be0d3db4) )

	ROM_REGION( 0x100000, "x1snd", 0 )	/* Samples */
	ROM_LOAD( "ua2-11", 0x000000, 0x080000, CRC(624e6057) SHA1(0e8e4d4b6bc5febf5ca83eea92e3ed06f16e7df0) )
	ROM_LOAD( "ua2-12", 0x080000, 0x080000, CRC(3068ff64) SHA1(7c06a48a99ebb9e7f3709f25bd0caa4c9d7a2688) )
ROM_END

ROM_START( downtown )
	ROM_REGION( 0x0a0000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "ud2001.000", 0x000000, 0x040000, CRC(f1965260) SHA1(c0560342238d75f9b81ae9f3408cacfbcd331529) )
	ROM_LOAD16_BYTE( "ud2001.003", 0x000001, 0x040000, CRC(e7d5fa5f) SHA1(48612514598711aa73bf75243c842f0aca72f3d0) )
	ROM_LOAD16_BYTE( "ud2001.002", 0x080000, 0x010000, CRC(a300e3ac) SHA1(958cb121787444cdc6938fc5aad1e92238e39c13) )
	ROM_LOAD16_BYTE( "ud2001.001", 0x080001, 0x010000, CRC(d2918094) SHA1(c135939ad12e3cf0688db148c49f99e757ad7b0d) )

	ROM_REGION( 0x04c000, "sub", 0 )		/* 65c02 Code */
	ROM_LOAD( "ud2002.004", 0x004000, 0x040000, CRC(bbd538b1) SHA1(de4c43bfc4004a14f9f66b5e8ff192b00c45c003) )
	ROM_RELOAD(             0x00c000, 0x040000  )

	ROM_REGION( 0x200000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD16_BYTE( "ud2005.t01", 0x000000, 0x080000, CRC(77e6d249) SHA1(cdf67211cd447858293188511e826640fe24078b) )
	ROM_LOAD16_BYTE( "ud2006.t02", 0x000001, 0x080000, CRC(6e381bf2) SHA1(ba46e019d2991dec539444ef7376fe0e9a6a8b75) )
	ROM_LOAD16_BYTE( "ud2007.t03", 0x100000, 0x080000, CRC(737b4971) SHA1(2a034011b0ac03d532a89b544f4eec497ac7ee80) )
	ROM_LOAD16_BYTE( "ud2008.t04", 0x100001, 0x080000, CRC(99b9d757) SHA1(c3a763993305110ec2a0b231d75fbef4c385d21b) )

	ROM_REGION( 0x100000, "gfx2", 0 )	/* Layer 1 */
	ROM_LOAD( "ud2009.t05", 0x000000, 0x080000, CRC(aee6c581) SHA1(5b2150a308ca12eea8148d0bbff663b3baf0c831) )
	ROM_LOAD( "ud2010.t06", 0x080000, 0x080000, CRC(3d399d54) SHA1(7d9036e73fbf0e9c3b976336e3e4786b17b2f4fc) )

	ROM_REGION( 0x080000, "x1snd", 0 )	/* Samples */
	ROM_LOAD( "ud2011.t07", 0x000000, 0x080000, CRC(9c9ff69f) SHA1(3840b654f4f709bc4c03dfe4ee79369d5c70dd62) )
ROM_END

ROM_START( downtown2 )
	ROM_REGION( 0x0a0000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "ud2001.000", 0x000000, 0x040000, CRC(f1965260) SHA1(c0560342238d75f9b81ae9f3408cacfbcd331529) )
	ROM_LOAD16_BYTE( "ud2001.003", 0x000001, 0x040000, CRC(e7d5fa5f) SHA1(48612514598711aa73bf75243c842f0aca72f3d0) )
	ROM_LOAD16_BYTE( "ud2000.002", 0x080000, 0x010000, CRC(ca976b24) SHA1(3b2e362f414b0103dd02c9af6a5d480ec2cf9ca3) )
	ROM_LOAD16_BYTE( "ud2000.001", 0x080001, 0x010000, CRC(1708aebd) SHA1(337a9e8d5da5b13a7ea4ee728de6b82fe92e16c5) )

	ROM_REGION( 0x04c000, "sub", 0 )		/* 65c02 Code */
	ROM_LOAD( "ud2002.004", 0x004000, 0x040000, CRC(bbd538b1) SHA1(de4c43bfc4004a14f9f66b5e8ff192b00c45c003) )
	ROM_RELOAD(             0x00c000, 0x040000  )

	ROM_REGION( 0x200000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD16_BYTE( "ud2005.t01", 0x000000, 0x080000, CRC(77e6d249) SHA1(cdf67211cd447858293188511e826640fe24078b) )
	ROM_LOAD16_BYTE( "ud2006.t02", 0x000001, 0x080000, CRC(6e381bf2) SHA1(ba46e019d2991dec539444ef7376fe0e9a6a8b75) )
	ROM_LOAD16_BYTE( "ud2007.t03", 0x100000, 0x080000, CRC(737b4971) SHA1(2a034011b0ac03d532a89b544f4eec497ac7ee80) )
	ROM_LOAD16_BYTE( "ud2008.t04", 0x100001, 0x080000, CRC(99b9d757) SHA1(c3a763993305110ec2a0b231d75fbef4c385d21b) )

	ROM_REGION( 0x100000, "gfx2", 0 )	/* Layer 1 */
	ROM_LOAD( "ud2009.t05", 0x000000, 0x080000, CRC(aee6c581) SHA1(5b2150a308ca12eea8148d0bbff663b3baf0c831) )
	ROM_LOAD( "ud2010.t06", 0x080000, 0x080000, CRC(3d399d54) SHA1(7d9036e73fbf0e9c3b976336e3e4786b17b2f4fc) )

	ROM_REGION( 0x080000, "x1snd", 0 )	/* Samples */
	ROM_LOAD( "ud2011.t07", 0x000000, 0x080000, CRC(9c9ff69f) SHA1(3840b654f4f709bc4c03dfe4ee79369d5c70dd62) )
ROM_END

ROM_START( downtownj )
	ROM_REGION( 0x0a0000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "ud2001.000", 0x000000, 0x040000, CRC(f1965260) SHA1(c0560342238d75f9b81ae9f3408cacfbcd331529) )
	ROM_LOAD16_BYTE( "ud2001.003", 0x000001, 0x040000, CRC(e7d5fa5f) SHA1(48612514598711aa73bf75243c842f0aca72f3d0) )
	ROM_LOAD16_BYTE( "u37.9a",     0x080000, 0x010000, CRC(73047657) SHA1(731663101d809170aad3cd39e901ef494494c5a1) )
	ROM_LOAD16_BYTE( "u31.8a",     0x080001, 0x010000, CRC(6a050240) SHA1(6a1a305b7d32bb2ad17842b4eeabc891fce02160) )

	ROM_REGION( 0x04c000, "sub", 0 )		/* 65c02 Code */
	ROM_LOAD( "ud2002.004", 0x004000, 0x040000, CRC(bbd538b1) SHA1(de4c43bfc4004a14f9f66b5e8ff192b00c45c003) )
	ROM_RELOAD(             0x00c000, 0x040000  )

	ROM_REGION( 0x200000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD16_BYTE( "ud2005.t01", 0x000000, 0x080000, CRC(77e6d249) SHA1(cdf67211cd447858293188511e826640fe24078b) )
	ROM_LOAD16_BYTE( "ud2006.t02", 0x000001, 0x080000, CRC(6e381bf2) SHA1(ba46e019d2991dec539444ef7376fe0e9a6a8b75) )
	ROM_LOAD16_BYTE( "ud2007.t03", 0x100000, 0x080000, CRC(737b4971) SHA1(2a034011b0ac03d532a89b544f4eec497ac7ee80) )
	ROM_LOAD16_BYTE( "ud2008.t04", 0x100001, 0x080000, CRC(99b9d757) SHA1(c3a763993305110ec2a0b231d75fbef4c385d21b) )

	ROM_REGION( 0x100000, "gfx2", 0 )	/* Layer 1 */
	ROM_LOAD( "ud2009.t05", 0x000000, 0x080000, CRC(aee6c581) SHA1(5b2150a308ca12eea8148d0bbff663b3baf0c831) )
	ROM_LOAD( "ud2010.t06", 0x080000, 0x080000, CRC(3d399d54) SHA1(7d9036e73fbf0e9c3b976336e3e4786b17b2f4fc) )

	ROM_REGION( 0x080000, "x1snd", 0 )	/* Samples */
	ROM_LOAD( "ud2011.t07", 0x000000, 0x080000, CRC(9c9ff69f) SHA1(3840b654f4f709bc4c03dfe4ee79369d5c70dd62) )
ROM_END

ROM_START( downtownp )
	ROM_REGION( 0x0a0000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "ud2001.000",   0x000000, 0x040000, CRC(f1965260) SHA1(c0560342238d75f9b81ae9f3408cacfbcd331529) )
	ROM_LOAD16_BYTE( "ud2001.003",   0x000001, 0x040000, CRC(e7d5fa5f) SHA1(48612514598711aa73bf75243c842f0aca72f3d0) )
	ROM_LOAD16_BYTE( "ud2_061e.bin", 0x080000, 0x010000, CRC(251d6552) SHA1(0f78bf142db826e956f670ba81102804e88fa2ed) )
	ROM_LOAD16_BYTE( "ud2_061o.bin", 0x080001, 0x010000, CRC(6394a7c0) SHA1(9f5099b32b3c3e100441f6c0ccbe88c19b01a9e5) )

	ROM_REGION( 0x04c000, "sub", 0 )		/* 65c02 Code */
	ROM_LOAD( "ud2002.004", 0x004000, 0x040000, CRC(bbd538b1) SHA1(de4c43bfc4004a14f9f66b5e8ff192b00c45c003) )
	ROM_RELOAD(             0x00c000, 0x040000  )

	ROM_REGION( 0x200000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD16_BYTE( "ud2005.t01", 0x000000, 0x080000, CRC(77e6d249) SHA1(cdf67211cd447858293188511e826640fe24078b) )
	ROM_LOAD16_BYTE( "ud2006.t02", 0x000001, 0x080000, CRC(6e381bf2) SHA1(ba46e019d2991dec539444ef7376fe0e9a6a8b75) )
	ROM_LOAD16_BYTE( "ud2007.t03", 0x100000, 0x080000, CRC(737b4971) SHA1(2a034011b0ac03d532a89b544f4eec497ac7ee80) )
	ROM_LOAD16_BYTE( "ud2008.t04", 0x100001, 0x080000, CRC(99b9d757) SHA1(c3a763993305110ec2a0b231d75fbef4c385d21b) )

	ROM_REGION( 0x100000, "gfx2", 0 )	/* Layer 1 */
	ROM_LOAD( "ud2009.t05", 0x000000, 0x080000, CRC(aee6c581) SHA1(5b2150a308ca12eea8148d0bbff663b3baf0c831) )
	ROM_LOAD( "ud2010.t06", 0x080000, 0x080000, CRC(3d399d54) SHA1(7d9036e73fbf0e9c3b976336e3e4786b17b2f4fc) )

	ROM_REGION( 0x080000, "x1snd", 0 )	/* Samples */
	ROM_LOAD( "ud2011.t07", 0x000000, 0x080000, CRC(9c9ff69f) SHA1(3840b654f4f709bc4c03dfe4ee79369d5c70dd62) )
ROM_END

ROM_START( usclssic )
	ROM_REGION( 0x080000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "ue2001.u20", 0x000000, 0x020000, CRC(18b41421) SHA1(74e96071d46eda152aaa82cf87d09203f225b504) )
	ROM_LOAD16_BYTE( "ue2000.u14", 0x000001, 0x020000, CRC(69454bc2) SHA1(19a3b6ca65770353401544c50e04d895e391612c) )
	ROM_LOAD16_BYTE( "ue2002.u22", 0x040000, 0x020000, CRC(a7bbe248) SHA1(8f7ffeffb8b6ef0e1ab5e7fbba31a1b97bbd7f8c) )
	ROM_LOAD16_BYTE( "ue2003.u30", 0x040001, 0x020000, CRC(29601906) SHA1(9cdf2d80a72317a4eb7a335aaaae381822da24b1) )

	ROM_REGION( 0x04c000, "sub", 0 )		/* 65c02 Code */
	ROM_LOAD( "ue002u61.004", 0x004000, 0x040000, CRC(476e9f60) SHA1(940c09eb472652a88d5d34130270ff55a5f5ba27) )
	ROM_RELOAD(               0x00c000, 0x040000  )

	ROM_REGION( 0x200000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD16_BYTE( "ue001009.119", 0x000000, 0x080000, CRC(dc065204) SHA1(0478b8126cd3ce3dee64cb7de2b30b509636eb1a) )
	ROM_LOAD16_BYTE( "ue001008.118", 0x000001, 0x080000, CRC(5947d9b5) SHA1(de3a63c55b558451bbbe98bf8d71561ba32c5e60) )
	ROM_LOAD16_BYTE( "ue001007.117", 0x100000, 0x080000, CRC(b48a885c) SHA1(8c0d458d6967c2ff4bdcf37aaa8025341fe90bbc) )
	ROM_LOAD16_BYTE( "ue001006.116", 0x100001, 0x080000, CRC(a6ab6ef4) SHA1(9f54f116d1d8e54d64ba541195baa66c5ca960bd) )

	ROM_REGION( 0x600000, "gfx2", 0 )	/* Layer 1 */
	ROM_LOAD( "ue001010.120", 0x000000, 0x080000, CRC(dd683031) SHA1(06ed38a243666c1acaf8eb3fdba51d18fc2a70bc) )	// planes 01
	ROM_LOAD( "ue001011.121", 0x080000, 0x080000, CRC(0e27bc49) SHA1(f9ec4f4c15c86f608607a5ec916f5182a8e265fa) )
	ROM_LOAD( "ue001012.122", 0x100000, 0x080000, CRC(961dfcdc) SHA1(9de95692860abd4206db22ad7ade9f02f0c03506) )
	ROM_LOAD( "ue001013.123", 0x180000, 0x080000, CRC(03e9eb79) SHA1(e7cabfd73b73c7df8d79c113db5eca110bf2f05e) )

	ROM_LOAD( "ue001014.124", 0x200000, 0x080000, CRC(9576ace7) SHA1(a5350934533241daf63c561a88d952bb6976b81b) )	// planes 23
	ROM_LOAD( "ue001015.125", 0x280000, 0x080000, CRC(631d6eb1) SHA1(df342c20e2b3c29eab3c72440c11be60d14d3557) )
	ROM_LOAD( "ue001016.126", 0x300000, 0x080000, CRC(f44a8686) SHA1(649f6f95cc67fa2f4551af19a2b607c811318820) )
	ROM_LOAD( "ue001017.127", 0x380000, 0x080000, CRC(7f568258) SHA1(ac36e87386f9d5c68c66a9469e1b30ee66c4cb7f) )

	ROM_LOAD( "ue001018.128", 0x400000, 0x080000, CRC(4bd98f23) SHA1(be6483253a5ea1efe7c7f6b4432fe819b906894c) )	// planes 45
	ROM_LOAD( "ue001019.129", 0x480000, 0x080000, CRC(6d9f5a33) SHA1(8d300adf2b3299df78e274c4c7f2ee2d8e1e2575) )
	ROM_LOAD( "ue001020.130", 0x500000, 0x080000, CRC(bc07403f) SHA1(f994b6d1dee23f5dabdb328f955f4380a8ca9d52) )
	ROM_LOAD( "ue001021.131", 0x580000, 0x080000, CRC(98c03efd) SHA1(761c51d5573e6f35c48b8b9ee5d88cbde02e92a7) )

	ROM_REGION( 0x400, "proms", 0 )	/* Extra Colours */
	ROM_LOAD16_BYTE( "ue1-022.prm", 0x000, 0x200, CRC(1a23129e) SHA1(110eb54ab83ecb8375164a5c96f522b2737c379c) )
	ROM_LOAD16_BYTE( "ue1-023.prm", 0x001, 0x200, CRC(a13192a4) SHA1(86e312e0f7400b7fa08fbe8fced1eb95a32502ca) )

	ROM_REGION( 0x080000, "x1snd", 0 )	/* Samples */
	ROM_LOAD( "ue001005.132", 0x000000, 0x080000, CRC(c5fea37c) SHA1(af4f09dd36af06e50262f607ff14eedc33beffd2) )
ROM_END

ROM_START( calibr50 )
	ROM_REGION( 0x0a0000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "uh002001.u45", 0x000000, 0x040000, CRC(eb92e7ed) SHA1(2aee8a7bce549ef7d7b35d1c248ebbdbc906e38d) )
	ROM_LOAD16_BYTE( "uh002004.u41", 0x000001, 0x040000, CRC(5a0ed31e) SHA1(d6ee7654354ac9f1dc7add1ef9f68a147b6f2953) )
	ROM_LOAD16_BYTE( "uh001003.9a",  0x080000, 0x010000, CRC(0d30d09f) SHA1(8a48511b628e85b72fda0968d813f4faebd0c418) )
	ROM_LOAD16_BYTE( "uh001002.7a",  0x080001, 0x010000, CRC(7aecc3f9) SHA1(2454d9c758fa623d4d81a9230871b67d31d16cef) )

	ROM_REGION( 0x04c000, "sub", 0 )		/* 65c02 Code */
	ROM_LOAD( "uh001005.u61", 0x004000, 0x040000, CRC(4a54c085) SHA1(f53ff257ce3d95f945a6befcfb61f1b570f0eafe) )
	ROM_RELOAD(               0x00c000, 0x040000  )

	ROM_REGION( 0x200000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD16_BYTE( "uh001006.ux2", 0x000000, 0x080000, CRC(fff52f91) SHA1(fd7807e9a8dd5a88df1fcd13746b44a33adbc0fa) )
	ROM_LOAD16_BYTE( "uh001007.ux1", 0x000001, 0x080000, CRC(b6c19f71) SHA1(eb8bbaeaf4af07e178100ff16b228b537aa36272) )
	ROM_LOAD16_BYTE( "uh001008.ux6", 0x100000, 0x080000, CRC(7aae07ef) SHA1(1db666db20efce1efe5b5769b8e3c78bbf508466) )
	ROM_LOAD16_BYTE( "uh001009.ux0", 0x100001, 0x080000, CRC(f85da2c5) SHA1(d090e49b3a897729c7fb05f9386939448fe1d3d9) )

	ROM_REGION( 0x100000, "gfx2", 0 )	/* Layer 1 */
	ROM_LOAD( "uh001010.u3x", 0x000000, 0x080000, CRC(f986577a) SHA1(8f6c2fca271fed21a1c04e93c3f50dc41348ae30) )
	ROM_LOAD( "uh001011.u50", 0x080000, 0x080000, CRC(08620052) SHA1(e2ab49dbabc139e6b276401340085ccab1ae3892) )

	ROM_REGION( 0x100000, "x1snd", 0 )	/* Samples */
	ROM_LOAD( "uh001013.u60", 0x000000, 0x080000, CRC(09ec0df6) SHA1(57c68d05074ea4a1e133be2ce6e25c594f04a712) )
	ROM_LOAD( "uh001012.u46", 0x080000, 0x080000, CRC(bb996547) SHA1(0c8f570ef4454b10a023e0c463001c22a8cf99cd) )
ROM_END

ROM_START( arbalest )
	ROM_REGION( 0x0a0000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "uk001.03",  0x000000, 0x040000, CRC(ee878a2c) SHA1(f7d5817015382ce6af317c02746b473ec798bb4f) )
	ROM_LOAD16_BYTE( "uk001.04",  0x000001, 0x040000, CRC(902bb4e3) SHA1(e37a361a7c03aee2d6ac8c96c2dd6c1e411b46fb) )

	ROM_REGION( 0x010000, "sub", 0 )		/* 65c02 Code */
	ROM_LOAD( "uk001.05", 0x006000, 0x002000, CRC(0339fc53) SHA1(9078f70a319132ef12c59b85e6306d5549a858f1) )
	ROM_RELOAD(           0x008000, 0x002000  )
	ROM_RELOAD(           0x00a000, 0x002000  )
	ROM_RELOAD(           0x00c000, 0x002000  )
	ROM_RELOAD(           0x00e000, 0x002000  )

	ROM_REGION( 0x100000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD16_BYTE( "uk001.06", 0x000000, 0x040000, CRC(11c75746) SHA1(7faf9a26534397d21211d5ef25ca53c4eb286474) )
	ROM_LOAD16_BYTE( "uk001.07", 0x000001, 0x040000, CRC(01b166c7) SHA1(d1b5b73a55025a264a22dd950ea79ba8172c4bed) )
	ROM_LOAD16_BYTE( "uk001.08", 0x080000, 0x040000, CRC(78d60ba3) SHA1(c4fa546e4ca637d67ecc1b085b91c753606ccdb3) )
	ROM_LOAD16_BYTE( "uk001.09", 0x080001, 0x040000, CRC(b4748ae0) SHA1(a71e671754ed5bba737f0b5f7be510a23d5e925c) )

	ROM_REGION( 0x200000, "gfx2", 0 )	/* Layer 1 */
	ROM_LOAD( "uk001.10", 0x000000, 0x080000, CRC(c1e2f823) SHA1(892473351e7b590c59c578047a67fc235bd31e02) )
	ROM_LOAD( "uk001.11", 0x080000, 0x080000, CRC(09dfe56a) SHA1(077627627d3cb8f79ffdd83e46157bd3c473c4a1) )
	ROM_LOAD( "uk001.12", 0x100000, 0x080000, CRC(818a4085) SHA1(fd8b5658fc7f5fa6d3daebb4be17aeabd60c9028) )
	ROM_LOAD( "uk001.13", 0x180000, 0x080000, CRC(771fa164) SHA1(a91214318808f991846a828f0e309c0ff430245e) )

	ROM_REGION( 0x100000, "x1snd", 0 )	/* Samples */
	ROM_LOAD( "uk001.15", 0x000000, 0x080000, CRC(ce9df5dd) SHA1(91d879b774b5b367adb5bd511fda827bc0bae0a9) )
	ROM_LOAD( "uk001.14", 0x080000, 0x080000, CRC(016b844a) SHA1(1fe091233746ced358292014393896af730f5940) )
ROM_END

ROM_START( metafox )
	ROM_REGION( 0x0a0000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "p1003161", 0x000000, 0x040000, CRC(4fd6e6a1) SHA1(11a830d76ef737bcfac73d0958425fe4329f0dcd) )
	ROM_LOAD16_BYTE( "p1004162", 0x000001, 0x040000, CRC(b6356c9a) SHA1(182a1ea9f0643b05b14ad2a2cd820f5ca2086c4c) )
	ROM_LOAD16_BYTE( "up001002", 0x080000, 0x010000, CRC(ce91c987) SHA1(63546fa1342371a7080ac3cf59b41a01ac313c8c) )
	ROM_LOAD16_BYTE( "up001001", 0x080001, 0x010000, CRC(0db7a505) SHA1(d593da2f7d8b54724cae017cbabc3c0909893da1) )

	ROM_REGION( 0x010000, "sub", 0 )		/* 65c02 Code */
	ROM_LOAD( "up001005", 0x006000, 0x002000, CRC(2ac5e3e3) SHA1(b794554cd25bdd48a21a0a2861daf8369e798ce8) )
	ROM_RELOAD(           0x008000, 0x002000  )
	ROM_RELOAD(           0x00a000, 0x002000  )
	ROM_RELOAD(           0x00c000, 0x002000  )
	ROM_RELOAD(           0x00e000, 0x002000  )

	ROM_REGION( 0x100000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD16_BYTE( "p1006163", 0x000000, 0x040000, CRC(80f69c7c) SHA1(df323e801ebec6316ba17fe0371f7c87fad19295) )
	ROM_LOAD16_BYTE( "p1007164", 0x000001, 0x040000, CRC(d137e1a3) SHA1(0e0234f1d0adb7db6d0508263e3b0b31fe7071b9) )
	ROM_LOAD16_BYTE( "p1008165", 0x080000, 0x040000, CRC(57494f2b) SHA1(28d620254e81d7e63dfed07b29b252b975a81248) )
	ROM_LOAD16_BYTE( "p1009166", 0x080001, 0x040000, CRC(8344afd2) SHA1(7348b423405ad00b9240d152b119cf5341754815) )

	ROM_REGION( 0x200000, "gfx2", 0 )	/* Layer 1 */
	ROM_LOAD( "up001010", 0x000000, 0x080000, CRC(bfbab472) SHA1(d3e7b20d14de48134c4fbe3da31feb928c1c655b) )
	ROM_LOAD( "up001011", 0x080000, 0x080000, CRC(26cea381) SHA1(b4bfd2a13ef6051376fe3ed57e2331a072970f86) )
	ROM_LOAD( "up001012", 0x100000, 0x080000, CRC(fed2c5f9) SHA1(81f0f19a500b665c937f5431000ebde7abd97c30) )
	ROM_LOAD( "up001013", 0x180000, 0x080000, CRC(adabf9ea) SHA1(db28e4e565e567a97a6b05a4803a55a403e24a0e) )

	ROM_REGION( 0x100000, "x1snd", 0 )	/* Samples */
	ROM_LOAD( "up001015", 0x000000, 0x080000, CRC(2e20e39f) SHA1(6f8bd4a76ed5c2150015698e7a98044d060157be) )
	ROM_LOAD( "up001014", 0x080000, 0x080000, CRC(fca6315e) SHA1(cef2385ec43f8b7a2d655b42c18ef44e46ff7364) )
ROM_END


ROM_START( drgnunit )
	ROM_REGION( 0x0c0000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "prg-e.bin", 0x000000, 0x020000, CRC(728447df) SHA1(8bdc52a4cc5f36794a47f963545bdaa26c9acd6b) )
	ROM_LOAD16_BYTE( "prg-o.bin", 0x000001, 0x020000, CRC(b2f58ecf) SHA1(5198e75b22bab630b458797988f2e443c601351f) )

	ROM_REGION( 0x100000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD16_BYTE( "obj-2.bin", 0x000000, 0x020000, CRC(d7f6ab5a) SHA1(a32f1705e833c339bd0c426a395cc706da96dad7) )
	ROM_LOAD16_BYTE( "obj-1.bin", 0x000001, 0x020000, CRC(53a95b13) SHA1(b7c7994441aafcea49662dc0fbebd6db836723f5) )
	ROM_LOAD16_BYTE( "obj-6.bin", 0x040000, 0x020000, CRC(80b801f7) SHA1(5b5635903137e50bc982d05b73c2648bbf182e71) )
	ROM_LOAD16_BYTE( "obj-5.bin", 0x040001, 0x020000, CRC(6b87bc20) SHA1(9a0e3e18339d6c12e63960fb940a56c16dcb87cf) )
	ROM_LOAD16_BYTE( "obj-4.bin", 0x080000, 0x020000, CRC(60d17771) SHA1(0874c10a2527293715db95bd7c83886d94f810cf) )
	ROM_LOAD16_BYTE( "obj-3.bin", 0x080001, 0x020000, CRC(0bccd4d5) SHA1(7139ef793efe7c6477f78b50207227b1be223755) )
	ROM_LOAD16_BYTE( "obj-8.bin", 0x0c0000, 0x020000, CRC(826c1543) SHA1(f669f255b4596da5648592b5993b02671e404102) )
	ROM_LOAD16_BYTE( "obj-7.bin", 0x0c0001, 0x020000, CRC(cbaa7f6a) SHA1(060f0651b8ca07d239ef1b7c63943cdd433e1ae9) )

	ROM_REGION( 0x100000, "gfx2", 0 )	/* Layer 1 */
	ROM_LOAD( "scr-1o.bin",  0x000000, 0x020000, CRC(671525db) SHA1(e230e99754c9f40af7da2054bd5ea09823e0b1b7) )
	ROM_LOAD( "scr-2o.bin",  0x020000, 0x020000, CRC(2a3f2ed8) SHA1(9d188100437a10eb3d3097f28e28e3cb2dc1b21d) )
	ROM_LOAD( "scr-3o.bin",  0x040000, 0x020000, CRC(4d33a92d) SHA1(8b09768abb460446405224565eb6652d2dc1c571) )
	ROM_LOAD( "scr-4o.bin",  0x060000, 0x020000, CRC(79a0aa61) SHA1(9905d90afb759b1d983856d7bef17c139d4f0e4f) )
	ROM_LOAD( "scr-1e.bin",  0x080000, 0x020000, CRC(dc9cd8c9) SHA1(04450a5cfde5d6b69fdd745cd930309863e1aadd) )
	ROM_LOAD( "scr-2e.bin",  0x0a0000, 0x020000, CRC(b6126b41) SHA1(13275f05868d93af95ebb162d229b69ddd660438) )
	ROM_LOAD( "scr-3e.bin",  0x0c0000, 0x020000, CRC(1592b8c2) SHA1(d337de280c5ea3704dec9baa04c45e1c837924a9) )
	ROM_LOAD( "scr-4e.bin",  0x0e0000, 0x020000, CRC(8201681c) SHA1(7784a68828d728107b0228bb3568129c543cbf40) )

	ROM_REGION( 0x100000, "x1snd", 0 )	/* Samples */
	ROM_LOAD( "snd-1.bin", 0x000000, 0x020000, CRC(8f47bd0d) SHA1(c55e22ac4294931cfb72ac88a2128891d9f8ee93) )
	ROM_LOAD( "snd-2.bin", 0x020000, 0x020000, CRC(65c40ef5) SHA1(726b46144e2216d17b0828abad2f5e7c2305c174) )
	ROM_LOAD( "snd-3.bin", 0x040000, 0x020000, CRC(71fbd54e) SHA1(bdaf7ecf1c79c6c8fc82d959186ca2f3304729c8) )
	ROM_LOAD( "snd-4.bin", 0x060000, 0x020000, CRC(ac50133f) SHA1(d56a9569bd72c7bc13d09dcea9789cdc7252ffb4) )
	ROM_LOAD( "snd-5.bin", 0x080000, 0x020000, CRC(70652f2c) SHA1(04ff016a087a230efe4644eb76f68886aae26978) )
	ROM_LOAD( "snd-6.bin", 0x0a0000, 0x020000, CRC(10a1039d) SHA1(a1160fe600d39ae6fdbf247f634c2e094b3a675f) )
	ROM_LOAD( "snd-7.bin", 0x0c0000, 0x020000, CRC(decbc8b0) SHA1(9d315d1119fbc2bf889efdb174ebc5e26ecad859) )
	ROM_LOAD( "snd-8.bin", 0x0e0000, 0x020000, CRC(3ac51bee) SHA1(cb2ee501895b848d434991152dea293685f8ed22) )
ROM_END

ROM_START( wits )
	ROM_REGION( 0x010000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "un001001.u1", 0x000000, 0x008000, CRC(416c567e) SHA1(f0898ce4457efc272e0fec3447c9d4598684219e) )
	ROM_LOAD16_BYTE( "un001002.u4", 0x000001, 0x008000, CRC(497a3fa6) SHA1(cf035efddc2a90013e83dcb81687ba1896ba6055) )

	ROM_REGION( 0x080000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD16_BYTE( "un001008.7l", 0x000000, 0x020000, CRC(1d5d0b2b) SHA1(12cf1be316012e8ee910edfd1b892b7ce1383535) )
	ROM_LOAD16_BYTE( "un001007.5l", 0x000001, 0x020000, CRC(9e1e6d51) SHA1(9a87f0f18ac0b3d267fe8655d01750d693745c1f) )
	ROM_LOAD16_BYTE( "un001006.4l", 0x040000, 0x020000, CRC(98a980d4) SHA1(ab2c1ed83bccffabfacc8a185d1fbc3e8aaf210d) )
	ROM_LOAD16_BYTE( "un001005.2l", 0x040001, 0x020000, CRC(6f2ce3c0) SHA1(8086b44c7025bc0bffff75cc6c6c7846cc56e8d0) )

	ROM_REGION( 0x40000, "x1snd", 0 )	/* Samples */
	ROM_LOAD( "un001004.12a", 0x000000, 0x020000, CRC(a15ff938) SHA1(fdfdf73e85d89a39cfc5b3c3048a64178200f942) )
	ROM_LOAD( "un001003.10a", 0x020000, 0x020000, CRC(3f4b9e55) SHA1(3cecf89ae6a056622affcddec9e10be08761e56d) )
ROM_END

ROM_START( thunderl )
	ROM_REGION( 0x010000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "m4", 0x000000, 0x008000, CRC(1e6b9462) SHA1(f7f93479117e97d4e38632fef83c10345587f77f) )
	ROM_LOAD16_BYTE( "m5", 0x000001, 0x008000, CRC(7e82793e) SHA1(3e487f465d64af8c1c4852567b2fd9190363570c) )

	ROM_REGION( 0x080000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD16_BYTE( "t17", 0x000000, 0x020000, CRC(599a632a) SHA1(29da423dfe1f971cbb205767cf902d199d968d85) )
	ROM_LOAD16_BYTE( "t16", 0x000001, 0x020000, CRC(3aeef91c) SHA1(a5dc8c22a7bcc1199bdd09c7d0f1f8a378e757c5) )
	ROM_LOAD16_BYTE( "t15", 0x040000, 0x020000, CRC(b97a7b56) SHA1(c08d3586d489947af21f3493356e3a88d79746e8) )
	ROM_LOAD16_BYTE( "t14", 0x040001, 0x020000, CRC(79c707be) SHA1(f67fa40c8f6ab0fbce44997fdfbf699fea1f0df6) )

	ROM_REGION( 0x100000, "x1snd", 0 )	/* Samples */
	ROM_LOAD( "r28", 0x000000, 0x080000, CRC(a043615d) SHA1(e483fa9fd8e922578a9d7b6ced0750643089ca78) )
	ROM_LOAD( "r27", 0x080000, 0x080000, CRC(cb8425a3) SHA1(655afa295fbe99acc79c4004f03ed832560cff5b) )
ROM_END

/*

Wiggie Waggie & Super Bar run on a bootleg SETA board with an OKI M6295 replacing the X1-010 sound chip.
Both games are based on Thunder & Lightning code.

PCB:

+--------------------------------------------------------+
| YM3012* YM2151*  2M-1                                  |
|                  2M-2*      6116             6116      |
| VOL  6116 M6295  2M-3*      6116             6116      |
|       A5   16MHz                             6116      |
+-+   Z80A               6264                  6116      |
  |                      6264                            |
+-+                                                      |
|                6116                                    |
|J               6116                                    |
|A                                                       |
|M                                    PAL                |
|M                                       +------+        |
|A   DSW2                                |Actel |        |
|              +-+                       |A1020A|        |
+-+  DSW1      |6|                       |PL84C |        |
  |            |8| 6264                  +------+        |
+-+            |0| 6264           6116     1M-4          |
|              |0|                6116     1M-5          |
|              |0| 512-1          6116     1M-6          |
|              +-+ 512-2          6116     1M-7          |
+--------------------------------------------------------+

  CPU: 68000P10 (8MHz)
       Z80A (4MHz)
Sound: OKI M6295
  OSC: 16MHz
Other: Actel A1020A PLC84C (used for graphics and graphic rom decode)

* Denotes unpopulated sockets

*/

ROM_START( wiggie )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "wiggie.e19", 0x00000, 0x10000, CRC(24b58f16) SHA1(96ef92ab79258da9322dd7e706bf05ac5143f7b7) )
	ROM_LOAD16_BYTE( "wiggie.e21", 0x00001, 0x10000, CRC(83ba6edb) SHA1(fa74fb39599ed877317db73d02d14df5b475fc35) )

	ROM_REGION( 0x40000, "audiocpu", 0 ) /* sound cpu code */
	ROM_LOAD( "wiggie.a5", 0x00000, 0x10000, CRC(8078d77b) SHA1(4e6855d396a1bace2810b13b7dd08ccf5de89bd8) )

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "wiggie.d1", 0x00000, 0x40000, CRC(27fbe12a) SHA1(73f476a03b321ed1ae89104f5b32d77153fabb82) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "wiggie.j16", 0x00000, 0x20000, CRC(4fb40b8a) SHA1(120c9fd677071485a9f8accc2385117baf542b9c) ) /* Drawn nude girls for the backgrounds */
	ROM_LOAD( "wiggie.j18", 0x20000, 0x20000, CRC(ebc418e9) SHA1(a9af9bebce56608b0533d7d147191ebdceaca4e4) )
	ROM_LOAD( "wiggie.j20", 0x40000, 0x20000, CRC(c073501b) SHA1(4b4cd0fed5efe12bcd10f98a71becc212e7e753a) )
	ROM_LOAD( "wiggie.j21", 0x60000, 0x20000, CRC(22f6fa39) SHA1(d3e86e156434153335c5d2ce71417f35097f5ab7) )
ROM_END

ROM_START( superbar ) /* All roms had a "PROMAT" label with no other information.  Rom size was silkscreened on the PCB */
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "promat_512-1.e19", 0x00000, 0x10000, CRC(cc7f9e87) SHA1(6c63ee5ac1c145a151a972a2b6bcb29036dad02d) )
	ROM_LOAD16_BYTE( "promat_512-2.e21", 0x00001, 0x10000, CRC(5e8c7231) SHA1(16efbaa871335143490ca897e0573bbbcf16ff16) )

	ROM_REGION( 0x40000, "audiocpu", 0 ) /* sound cpu code */
	ROM_LOAD( "promat.a5", 0x00000, 0x10000, CRC(8078d77b) SHA1(4e6855d396a1bace2810b13b7dd08ccf5de89bd8) ) /* Same as Wiggie Waggie */

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "promat_2m-1.d1", 0x00000, 0x40000, CRC(27fbe12a) SHA1(73f476a03b321ed1ae89104f5b32d77153fabb82) ) /* Same as Wiggie Waggie */
	/* 2M-2 sample rom not populated */
	/* 2M-3 sample rom not populated */

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "promat_1m-4.j16", 0x00000, 0x20000, CRC(43dbc99f) SHA1(36ac4df9286d8661c61e0dfc9788b936c5596c31) ) /* Drawn clothed girls for the backgrounds */
	ROM_LOAD( "promat_1m-5.j18", 0x20000, 0x20000, CRC(c09344b0) SHA1(4c54dbc602fa2ccddd232f145d3844a4d145611c) )
	ROM_LOAD( "promat_1m-6.j20", 0x40000, 0x20000, CRC(7d83f8ba) SHA1(55d026a3b98dd0e9a6263a0c913a1d9b6c30cfd1) )
	ROM_LOAD( "promat_1m-7.j21", 0x60000, 0x20000, CRC(734df92a) SHA1(0dfd58a3f47fa8dfa315df7adfad25ade97c2a3b) )
ROM_END

ROM_START( rezon )
	ROM_REGION( 0x200000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "us001001.u3",  0x000000, 0x020000, CRC(ab923052) SHA1(26761c228b63c300f635787e63e1276b6e3083f0) )
	ROM_LOAD16_BYTE( "us001002.u4",  0x000001, 0x020000, CRC(3dafa0d5) SHA1(80cdff86b99d364acbbf1322c73b2f26b1a93167) )
	/* empty gap */
	ROM_LOAD16_BYTE( "us001004.103", 0x100000, 0x020000, CRC(54871c7c) SHA1(2f807b15760b1e712fa69eee6f33cc8a36ee1c02) ) // 1xxxxxxxxxxxxxxxx = 0x00
	ROM_LOAD16_BYTE( "us001003.102", 0x100001, 0x020000, CRC(1ac3d272) SHA1(0f19bc9c19e355dad5b463b0fa33127523bf141b) ) // 1xxxxxxxxxxxxxxxx = 0x00

	ROM_REGION( 0x100000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "us001006.u64",  0x000000, 0x080000, CRC(a4916e96) SHA1(bfb63b72273e4fbf0843b3201bb4fddaf54909a7) )
	ROM_LOAD( "us001005.u63",  0x080000, 0x080000, CRC(e6251ebc) SHA1(f02a4c8373e33fc57e18e39f1b5ecff3f6d9ca9e) )

	ROM_REGION( 0x080000, "gfx2", 0 )	/* Layer 1 */
	ROM_LOAD( "us001007.u66",  0x000000, 0x080000, CRC(3760b935) SHA1(f5fe69f7e93c90a5b6c1dff236402b962821e33f) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x080000, "gfx3", 0 )	/* Layer 2 */
	ROM_LOAD( "us001008.u68",  0x000000, 0x080000, CRC(0ab73910) SHA1(78e2c0570c5c6f5e1cdb2fbeae73376923127024) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x100000, "x1snd", 0 )	/* Samples */
	ROM_LOAD16_WORD_SWAP( "us001009.u70",  0x000000, 0x100000, CRC(0d7d2e2b) SHA1(cfba19314ecb0a49ed9ff8df32cd6a3fe37ff526) )
ROM_END

/* note the ONLY byte that changes is the year, 1992 instead of 1991.  The actual license is controlled by a jumper but
   since Taito released the game in 1992 this is the Taito version and we hardcode the jumper in the input ports */

ROM_START( rezont )
	ROM_REGION( 0x200000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "us001001.u3",  0x000000, 0x020000, CRC(ab923052) SHA1(26761c228b63c300f635787e63e1276b6e3083f0) )
	ROM_LOAD16_BYTE( "rezon_1_p.u4",  0x000001, 0x020000,  CRC(9ed32f8c) SHA1(68b926de4cb5f2632ab78b2cdf7409411fadbb1d) )
	/* empty gap */
	ROM_LOAD16_BYTE( "us001004.103", 0x100000, 0x020000, CRC(54871c7c) SHA1(2f807b15760b1e712fa69eee6f33cc8a36ee1c02) ) // 1xxxxxxxxxxxxxxxx = 0x00
	ROM_LOAD16_BYTE( "us001003.102", 0x100001, 0x020000, CRC(1ac3d272) SHA1(0f19bc9c19e355dad5b463b0fa33127523bf141b) ) // 1xxxxxxxxxxxxxxxx = 0x00

	ROM_REGION( 0x100000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "us001006.u64",  0x000000, 0x080000, CRC(a4916e96) SHA1(bfb63b72273e4fbf0843b3201bb4fddaf54909a7) )
	ROM_LOAD( "us001005.u63",  0x080000, 0x080000, CRC(e6251ebc) SHA1(f02a4c8373e33fc57e18e39f1b5ecff3f6d9ca9e) )

	ROM_REGION( 0x080000, "gfx2", 0 )	/* Layer 1 */
	ROM_LOAD( "us001007.u66",  0x000000, 0x080000, CRC(3760b935) SHA1(f5fe69f7e93c90a5b6c1dff236402b962821e33f) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x080000, "gfx3", 0 )	/* Layer 2 */
	ROM_LOAD( "us001008.u68",  0x000000, 0x080000, CRC(0ab73910) SHA1(78e2c0570c5c6f5e1cdb2fbeae73376923127024) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x100000, "x1snd", 0 )	/* Samples */
	ROM_LOAD16_WORD_SWAP( "us001009.u70",  0x000000, 0x100000, CRC(0d7d2e2b) SHA1(cfba19314ecb0a49ed9ff8df32cd6a3fe37ff526) )
ROM_END

ROM_START( stg )
	ROM_REGION( 0x0c0000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "att01003.u27", 0x000000, 0x020000, CRC(7a640a93) SHA1(28c54eca9502d06ca55c2db91bfe7d149af006ed) )
	ROM_LOAD16_BYTE( "att01001.u9",  0x000001, 0x020000, CRC(4fa88ad3) SHA1(55e0e689758511cdf514a633ffe3d7729e281b52) )
	ROM_LOAD16_BYTE( "att01004.u33", 0x040000, 0x020000, CRC(bbd45ca1) SHA1(badb11faf5779e8444dd95eb08a94fbf9f73cc2c) ) // 1xxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "att01002.u17", 0x040001, 0x020000, CRC(2f8fd80c) SHA1(b8e16adc84b918b5eee05d032a7841e8d726eeeb) ) // 1xxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x100000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "att01006.u32", 0x000000, 0x080000, CRC(6ad78ea2) SHA1(eb8fc9833fb1c7041f5e0a3b37c8de9156a034b6) )
	ROM_LOAD( "att01005.u26", 0x080000, 0x080000, CRC(a347ff00) SHA1(8455c5e7dfa25646b1782ab3bcf62fca91ca03ad) )

	ROM_REGION( 0x100000, "gfx2", 0 )	/* Layer 1 */
	ROM_LOAD( "att01008.u39", 0x000000, 0x080000, CRC(20c47457) SHA1(53ddf8c076aa35fb87edc739bc9e9612a5a1526b) ) // FIRST AND SECOND HALF IDENTICAL
	ROM_LOAD( "att01007.u42", 0x080000, 0x080000, CRC(ac975544) SHA1(5cdd2c7aada7179d4bdaf8578134c0ef672a2704) ) // FIRST AND SECOND HALF IDENTICAL

	ROM_REGION( 0x100000, "x1snd", 0 )	/* Samples */
	ROM_LOAD( "att01009.u47", 0x000000, 0x080000, CRC(4276b58d) SHA1(a2e77dc3295791520c6cb25dea4e910b5a7bc137) )
	ROM_LOAD( "att01010.u55", 0x080000, 0x080000, CRC(fffb2f53) SHA1(0aacb24437e9a6874850313163922d834da27611) )
ROM_END

ROM_START( blandia )
	ROM_REGION( 0x200000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "ux001001.003",  0x000000, 0x040000, CRC(2376a1f3) SHA1(705a3c5cc1137d14ffded6c949bf9aa650133eb7) )
	ROM_LOAD16_BYTE( "ux001002.004",  0x000001, 0x040000, CRC(b915e172) SHA1(e43e50a664dc1286ece42a5ff8629b2da7fb49b4) )
	ROM_LOAD16_WORD_SWAP( "ux001003.202",     0x100000, 0x100000, CRC(98052c63) SHA1(b523596de29038b3ec9f1b6e1f7374a6a8709d42) )

	ROM_REGION( 0x400000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "ux001005.200",  0x300000, 0x100000, CRC(bea0c4a5) SHA1(a690c17fb7cbdab533c1dfad13abbad9359b9631) )
	ROM_LOAD( "ux001007.201",  0x100000, 0x100000, CRC(4440fdd1) SHA1(7bfee90f81a2c867bd487abcf5905393ad400902) )
	ROM_LOAD( "ux001006.063",  0x200000, 0x100000, CRC(abc01cf7) SHA1(c3f26e75eeb68073d2825be8df82cc6afcfbfb26) )
	ROM_LOAD( "ux001008.064",  0x000000, 0x100000, CRC(413647b6) SHA1(594e010ca6f49ec82cc6d44fe23ac3427c4c3dbd) )

	ROM_REGION( 0x0c0000, "gfx2", 0 )	/* Layer 1 */
	ROM_LOAD( "ux001009.065",  0x000000, 0x080000, CRC(bc6f6aea) SHA1(673efa0c70587b5650ccf0a3c4bc316f53d52ba6) )
	ROM_LOAD( "ux001010.066",  0x040000, 0x080000, CRC(bd7f7614) SHA1(dc865ff0f327f460956915b2018aaac815e8fce5) )

	ROM_REGION( 0x0c0000, "gfx3", 0 )	/* Layer 2 */
	ROM_LOAD( "ux001011.067",  0x000000, 0x080000, CRC(5efe0397) SHA1(a294a2dae9a10e93912543a8614a7f960a011f27) )
	ROM_LOAD( "ux001012.068",  0x040000, 0x080000, CRC(f29959f6) SHA1(edccea3d0bf972a07edd6339e18792d089033bff) )

	/* The c0000-fffff region is bankswitched */
	ROM_REGION( 0x240000, "x1snd", 0 )	/* Samples */
	ROM_LOAD( "ux001013.069",  0x000000, 0x0c0000, CRC(5cd273cd) SHA1(602e1f10454e2b1c941f2e6983872bb9ca77a542) )
	// skip c0000-fffff (banked region)
	ROM_CONTINUE(              0x100000, 0x040000  )
	ROM_LOAD( "ux001014.070",  0x140000, 0x080000, CRC(86b49b4e) SHA1(045b352950d848907af4c22b817d154b2cfff382) )
ROM_END

ROM_START( blandiap )
	ROM_REGION( 0x200000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "prg-even.bin", 0x000000, 0x040000, CRC(7ecd30e8) SHA1(25e555a45bbb154170189b065206f7536a5dec1b) )
	ROM_LOAD16_BYTE( "prg-odd.bin",  0x000001, 0x040000, CRC(42b86c15) SHA1(9a4adcc16c35f84826a6effed5ebe439483ab856) )
	ROM_LOAD16_BYTE( "tbl0.bin",     0x100000, 0x080000, CRC(69b79eb8) SHA1(f7b33c99744d8b7f6e2991b4d2b35719eebd0b43) )
	ROM_LOAD16_BYTE( "tbl1.bin",     0x100001, 0x080000, CRC(cf2fd350) SHA1(4d0fb720af544f746eeaaad499be00e0d1c6f129) )

	ROM_REGION( 0x400000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD16_BYTE( "o-1.bin",  0x000000, 0x080000, CRC(4c67b7f0) SHA1(ad6bd4b880f0f63a803c097795a4b70f24c94848) )
	ROM_LOAD16_BYTE( "o-0.bin",  0x000001, 0x080000, CRC(5e7b8555) SHA1(040599db77041765f582aa99d6f616a7a2c4dd5c) )
	ROM_LOAD16_BYTE( "o-5.bin",  0x100000, 0x080000, CRC(40bee78b) SHA1(1ec0b1854c26ba300a3a54077332a9af55677dca) )
	ROM_LOAD16_BYTE( "o-4.bin",  0x100001, 0x080000, CRC(7c634784) SHA1(047287c630336001d2b1e21f7273ccc3d5278e3c) )
	ROM_LOAD16_BYTE( "o-3.bin",  0x200000, 0x080000, CRC(387fc7c4) SHA1(4e5bd3985f16aa7295110a9902adc8e1453c03ab) )
	ROM_LOAD16_BYTE( "o-2.bin",  0x200001, 0x080000, CRC(c669bb49) SHA1(db5051ea8b08672b6079004060e20fb250560d9f) )
	ROM_LOAD16_BYTE( "o-7.bin",  0x300000, 0x080000, CRC(fc77b04a) SHA1(b3c7b2cb9407cac261890e0355cbb87ac8e2e93c) )
	ROM_LOAD16_BYTE( "o-6.bin",  0x300001, 0x080000, CRC(92882943) SHA1(460f3ae37d6f88d3a6068e2fb8d0d330be7c786f) )

	ROM_REGION( 0x0c0000, "gfx2", 0 )	/* Layer 1 */
	ROM_LOAD( "v1-2.bin",  0x000000, 0x020000, CRC(d524735e) SHA1(4d17e7896b6c6451effe8a19bf7a4919db0cc06d) )
	ROM_LOAD( "v1-5.bin",  0x020000, 0x020000, CRC(eb440cdb) SHA1(180ed9d616c66b7fae1a3d1156028c8476e45bde) )
	ROM_LOAD( "v1-1.bin",  0x040000, 0x020000, CRC(09bdf75f) SHA1(33bda046092d5bc3d8e8ffec25c745a2fda16a5c) )
	ROM_LOAD( "v1-4.bin",  0x060000, 0x020000, CRC(803911e5) SHA1(a93cac42eda69698b393a1a49e3615ee60868838) )
	ROM_LOAD( "v1-0.bin",  0x080000, 0x020000, CRC(73617548) SHA1(9c04d0179cb93e9fb78cc8af1006ef3edfcde707) )
	ROM_LOAD( "v1-3.bin",  0x0a0000, 0x020000, CRC(7f18e4fb) SHA1(0e51e3f88b90c07a1352bcd42e6438c947d4856e) )

	ROM_REGION( 0x0c0000, "gfx3", 0 )	/* Layer 2 */
	ROM_LOAD( "v2-2.bin",  0x000000, 0x020000, CRC(c4f15638) SHA1(6be0f3e90ab23189caadfd99b3e4ded74749ebbc) )	// identical to v2-1
	ROM_LOAD( "v2-5.bin",  0x020000, 0x020000, CRC(c2e57622) SHA1(994a4774d68f2d562d985951b06216d59f38afe9) )
	ROM_LOAD( "v2-1.bin",  0x040000, 0x020000, CRC(c4f15638) SHA1(6be0f3e90ab23189caadfd99b3e4ded74749ebbc) )
	ROM_LOAD( "v2-4.bin",  0x060000, 0x020000, CRC(16ec2130) SHA1(187f548563577ca36cced9ae184d27e6fcdd7e6a) )
	ROM_LOAD( "v2-0.bin",  0x080000, 0x020000, CRC(5b05eba9) SHA1(665001cdb3c9977f8f4c7ce551549f7fc640c6a9) )
	ROM_LOAD( "v2-3.bin",  0x0a0000, 0x020000, CRC(80ad0c3b) SHA1(00fcbcf7805784d7298b92136e7f256d65029c44) )

	/* The c0000-fffff region is bankswitched */
	ROM_REGION( 0x240000, "x1snd", 0 )	/* Samples */
	ROM_LOAD( "s-0.bin",  0x000000, 0x020000, CRC(a5fde408) SHA1(89efcd37ef6c5b313169d74a962a7c074a09b12a) )
	ROM_CONTINUE(         0x140000, 0x020000  )
	ROM_LOAD( "s-1.bin",  0x020000, 0x020000, CRC(3083f9c4) SHA1(f5d2297c3d680eb1f128fa42a3a7f61badb9853a) )
	ROM_CONTINUE(         0x160000, 0x020000  )
	ROM_LOAD( "s-2.bin",  0x040000, 0x020000, CRC(a591c9ef) SHA1(83e665e342c42fd3582c83becfacc27a3a3e5a54) )
	ROM_CONTINUE(         0x180000, 0x020000  )
	ROM_LOAD( "s-3.bin",  0x060000, 0x020000, CRC(68826c9d) SHA1(a860b7b2140a5a506bf25110c08c6ea59db25743) )
	ROM_CONTINUE(         0x1a0000, 0x020000  )
	ROM_LOAD( "s-4.bin",  0x080000, 0x020000, CRC(1c7dc8c2) SHA1(006459a23de83fe48e11bdd6ebe23ef6a18a87e8) )
	ROM_CONTINUE(         0x1c0000, 0x020000  )
	ROM_LOAD( "s-5.bin",  0x0a0000, 0x020000, CRC(4bb0146a) SHA1(1e3c1739ea3c85296573426e55f25dce11f0ed2b) )
	ROM_CONTINUE(         0x1e0000, 0x020000  )
	ROM_LOAD( "s-6.bin",  0x100000, 0x020000, CRC(9f8f34ee) SHA1(60abb70ae87595ebae23df68d62f3b0ed4a2e768) )	// skip c0000-fffff (banked region)
	ROM_CONTINUE(         0x200000, 0x020000  )	// this half is 0
	ROM_LOAD( "s-7.bin",  0x120000, 0x020000, CRC(e077dd39) SHA1(a6f0881a026161710adc132bcf7cb95c4c8f2528) )
	ROM_CONTINUE(         0x220000, 0x020000  )	// this half is 0
ROM_END

ROM_START( blockcar )
	ROM_REGION( 0x040000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "u1.a1",  0x000000, 0x020000, CRC(4313fb00) SHA1(f5b9e212436282284fa344e1c4200bc38ca3c50a) )
	ROM_LOAD16_BYTE( "u4.a3",  0x000001, 0x020000, CRC(2237196d) SHA1(5a9d972fac94e62f026c36bca0c2f5fe8e0e1a1d) )

	ROM_REGION( 0x100000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "bl-chr-0.j3",  0x000000, 0x080000, CRC(a33300ca) SHA1(b0a7ccb77c3e8e33c12b83e254924f30209a4c2c) )
	ROM_LOAD( "bl-chr-1.l3",  0x080000, 0x080000, CRC(563de808) SHA1(40b2f9f4a4cb1a019f6419572ee21d66dda7d4af) )

	ROM_REGION( 0x100000, "x1snd", 0 )	/* Samples */
	ROM_LOAD( "bl-snd-0.a13",  0x000000, 0x080000, CRC(a92dabaf) SHA1(610c1dc0467753dfddaa4b27bc40cb118b0bc7a3) )
	ROM_RELOAD(                0x080000, 0x080000  )
ROM_END

ROM_START( qzkklogy )
	ROM_REGION( 0x0c0000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "3.u27", 0x000000, 0x020000, CRC(b8c27cde) SHA1(4c36076801b6c915888b925c1e37d772bab1bb02) )
	ROM_LOAD16_BYTE( "1.u9",  0x000001, 0x020000, CRC(ce01cd54) SHA1(ef91aecdf7b5586a6870ff237372d65f85cd4cd3) )
	ROM_LOAD16_BYTE( "4.u33", 0x040000, 0x020000, CRC(4f5c554c) SHA1(0a10cefdf2dd876e6cb78023c3c15af24ba3c39a) )
	ROM_LOAD16_BYTE( "2.u17", 0x040001, 0x020000, CRC(65fa1b8d) SHA1(81fd7785f138a189de978fd30dbfca36687cda17) )

	ROM_REGION( 0x100000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "t2709u32.u32", 0x000000, 0x080000, CRC(900f196c) SHA1(b60741c3242ce56cb61ea68093b571489db0c6fa) ) // FIRST AND SECOND HALF IDENTICAL
	ROM_LOAD( "t2709u26.u26", 0x080000, 0x080000, CRC(416ac849) SHA1(3bd5dd13a8f2693e8f160a4ecfff3b7610644f5f) ) // FIRST AND SECOND HALF IDENTICAL

	ROM_REGION( 0x100000, "gfx2", 0 )	/* Layer 1 */
	ROM_LOAD( "t2709u42.u39", 0x000000, 0x080000, CRC(194d5704) SHA1(ab2833f7427d0608850c158b813bc49935ac7d6d) )
	ROM_LOAD( "t2709u39.u42", 0x080000, 0x080000, CRC(6f95a76d) SHA1(925f5880fb5153c1215d1f5ee1eff5b53a84abea) )

	ROM_REGION( 0x100000, "x1snd", 0 )	/* Samples */
	ROM_LOAD( "t2709u47.u47", 0x000000, 0x080000, CRC(0ebdad40) SHA1(6558eeaac76d98d91b0be6faa78f531f1e3b9f84) )
	ROM_LOAD( "t2709u55.u55", 0x080000, 0x080000, CRC(43960c68) SHA1(9a1901b65f989aa57ab8736ef0be3bac492c081c) )
ROM_END

ROM_START( umanclub )
	ROM_REGION( 0x040000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "uw001006.u48", 0x000000, 0x020000, CRC(3dae1e9d) SHA1(91a738c299d134d198bad648383be87345f4f475) )
	ROM_LOAD16_BYTE( "uw001007.u49", 0x000001, 0x020000, CRC(5c21e702) SHA1(c69e9dd7dfac82f116885610f90878f865e629b3) )

	ROM_REGION( 0x100000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "bp-u-002.u2", 0x000000, 0x080000, CRC(936cbaaa) SHA1(f7932ee310eb792b2776ae8a9d29e1a492761b11) )
	ROM_LOAD( "bp-u-001.u1", 0x080000, 0x080000, CRC(87813c48) SHA1(7ec9b08fe0490d277c531e2b6394862df4d5678d) )

	ROM_REGION( 0x100000, "x1snd", 0 )	/* Samples */
	ROM_LOAD( "uw003.u13", 0x000000, 0x100000, CRC(e2f718eb) SHA1(fd085b68f76c8778816a1b7d47783b9dc20bff12) )
ROM_END

ROM_START( zingzip )
	ROM_REGION( 0x200000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "uy001001.3",  0x000000, 0x040000, CRC(1a1687ec) SHA1(c840752dd87d8c1c30e6b31452173148e20538b1) )
	ROM_LOAD16_BYTE( "uy001002.4",  0x000001, 0x040000, CRC(62e3b0c4) SHA1(51a27fbf68a142dd132157bed1dc22acda3fa044) )

	ROM_REGION( 0x100000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "uy001006.64",  0x000000, 0x080000, CRC(46e4a7d8) SHA1(2c829e52d9aead351702335bf06aa0f337528306) )
	ROM_LOAD( "uy001005.63",  0x080000, 0x080000, CRC(4aac128e) SHA1(3ac64c84a40f86e29e33a218babcd21cae6dbfdb) )

	ROM_REGION( 0x200000, "gfx2", 0 )	/* Layer 1 */
	ROM_LOAD       ( "uy001008.66", 0x000000, 0x100000, CRC(1dff7c4b) SHA1(94f581f4aae1ef417dce6e62a611a523205e8c27) ) // FIRST AND SECOND HALF IDENTICAL
	ROM_LOAD16_BYTE( "uy001007.65",	0x100000, 0x080000, CRC(ec5b3ab9) SHA1(e82fb050ae4e2486e43418fcb5fa726d92c5cd21) )

	ROM_REGION( 0x200000, "gfx3", 0 )	/* Layer 2 */
	ROM_LOAD( "uy001010.68",  0x000000, 0x100000, CRC(bdbcdf03) SHA1(857f541697f76086ac6c761a3505678a3d3499df) ) // FIRST AND SECOND HALF IDENTICAL

	ROM_REGION( 0x100000, "x1snd", 0 )	/* Samples */
	ROM_LOAD( "uy001011.70",  0x000000, 0x100000, CRC(bd845f55) SHA1(345b79cfcd8c924d6ba365814286e518438f10bc) ) // uy001017 + uy001018
ROM_END

ROM_START( atehate )
	ROM_REGION( 0x100000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "fs001001.evn", 0x000000, 0x080000, CRC(4af1f273) SHA1(79b28fe768aa634c31ee4e7687e62ebe78cf4014) )
	ROM_LOAD16_BYTE( "fs001002.odd", 0x000001, 0x080000, CRC(c7ca7a85) SHA1(1221f57d4aa3d2cb6662bc059978eafd65c1858f) )

	ROM_REGION( 0x200000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "fs001003.gfx", 0x000000, 0x200000, CRC(8b17e431) SHA1(643fc62d5bad9941630ab621ecb3c69ded9d4536) )

	ROM_REGION( 0x100000, "x1snd", 0 )	/* Samples */
	ROM_LOAD( "fs001004.pcm", 0x000000, 0x100000, CRC(f9344ce5) SHA1(cffbc235f3a8e9a5004e671d924affd321ec9eed) )
ROM_END

ROM_START( daioh )
	ROM_REGION( 0x100000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "fg1-001",  0x000000, 0x080000, CRC(104ae74a) SHA1(928c467e3ff98285a4828a927d851fcdf296849b) )
	ROM_LOAD16_BYTE( "fg1-002",  0x000001, 0x080000, CRC(e39a4e67) SHA1(c3f47e9d407f32dbfaf209d29b4446e4de8829a2) )

	ROM_REGION( 0x200000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "fg1-004", 0x000000, 0x100000, CRC(9ab0533e) SHA1(b260ceb2b3e140971419329bee07a020171794f7) )
	ROM_LOAD( "fg1-003", 0x100000, 0x100000, CRC(1c9d51e2) SHA1(1d6236ab28d11676386834fd6e405fd40198e924) )

	ROM_REGION( 0x200000, "gfx2", 0 ) /* Layer 1 */
	ROM_LOAD( "fg1-005",  0x000000, 0x200000, CRC(c25159b9) SHA1(4c9da3233223508389c3c0f277a00aedfc860da4) )

	ROM_REGION( 0x200000, "gfx3", 0 ) /* Layer 2 */
	ROM_LOAD( "fg1-006",  0x000000, 0x200000, CRC(2052c39a) SHA1(83a444a76e68aa711b0e25a5aa963ca876a6357e) )

	ROM_REGION( 0x100000, "x1snd", 0 )	/* Samples */
	ROM_LOAD( "fg1-007",  0x000000, 0x100000, CRC(4a2fe9e0) SHA1(e55b6f301f842ff5d3c7a0041856695ac1d8a78f) )
ROM_END

ROM_START( msgundam )
	ROM_REGION( 0x200000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "fa003002.u25",  0x000000, 0x080000, CRC(1cc72d4c) SHA1(5043d693b5a8116a077d5b6997b658cb287e2aa7) )
	ROM_LOAD16_WORD_SWAP( "fa001001.u20",  0x100000, 0x100000, CRC(fca139d0) SHA1(b56282c69f7ec64c697a48e42d59a2565401c032) )

	ROM_REGION( 0x400000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "fa001008.u21",  0x000000, 0x200000, CRC(e7accf48) SHA1(dca9d53bc9cf0ecb661358d5a3f388c4ce9388e7) )
	ROM_LOAD( "fa001007.u22",  0x200000, 0x200000, CRC(793198a6) SHA1(45f53870e74b14126680d18dd58dbbe01a6ef509) )

	ROM_REGION( 0x100000, "gfx2", 0 )	/* Layer 1 */
	ROM_LOAD( "fa001006.u23",  0x000000, 0x100000, CRC(3b60365c) SHA1(bdf5a0b1b45eb75dbbb6725d1e5303716321aeb9) )

	ROM_REGION( 0x080000, "gfx3", 0 )	/* Layer 2 */
	ROM_LOAD( "fa001005.u24",  0x000000, 0x080000, CRC(8cd7ff86) SHA1(ce7eb90776e21239f8f52e822c636143506c6f9b) )

	ROM_REGION( 0x100000, "x1snd", 0 )	/* Samples */
	ROM_LOAD( "fa001004.u26",  0x000000, 0x100000, CRC(b965f07c) SHA1(ff7827cc80655465ffbb732d55ba81f21f51a5ca) )
ROM_END

ROM_START( msgundam1 )
	ROM_REGION( 0x200000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "fa002002.u25",  0x000000, 0x080000, CRC(dee3b083) SHA1(e2ad626aa0109906846dd9e9053ffc83b7bf4d2e) )
	ROM_LOAD16_WORD_SWAP( "fa001001.u20",  0x100000, 0x100000, CRC(fca139d0) SHA1(b56282c69f7ec64c697a48e42d59a2565401c032) )

	ROM_REGION( 0x400000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "fa001008.u21",  0x000000, 0x200000, CRC(e7accf48) SHA1(dca9d53bc9cf0ecb661358d5a3f388c4ce9388e7) )
	ROM_LOAD( "fa001007.u22",  0x200000, 0x200000, CRC(793198a6) SHA1(45f53870e74b14126680d18dd58dbbe01a6ef509) )

	ROM_REGION( 0x100000, "gfx2", 0 )	/* Layer 1 */
	ROM_LOAD( "fa001006.u23",  0x000000, 0x100000, CRC(3b60365c) SHA1(bdf5a0b1b45eb75dbbb6725d1e5303716321aeb9) )

	ROM_REGION( 0x080000, "gfx3", 0 )	/* Layer 2 */
	ROM_LOAD( "fa001005.u24",  0x000000, 0x080000, CRC(8cd7ff86) SHA1(ce7eb90776e21239f8f52e822c636143506c6f9b) )

	ROM_REGION( 0x100000, "x1snd", 0 )	/* Samples */
	ROM_LOAD( "fa001004.u26",  0x000000, 0x100000, CRC(b965f07c) SHA1(ff7827cc80655465ffbb732d55ba81f21f51a5ca) )
ROM_END

ROM_START( oisipuzl )
	ROM_REGION( 0x180000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "ss1u200.v10", 0x000000, 0x080000, CRC(f5e53baf) SHA1(057e8b35bc6f65634685b5d0cf38e12f2e62d72c) )
	/* Gap of 0x80000 bytes */
	ROM_LOAD16_WORD_SWAP( "ss1u201.v10", 0x100000, 0x080000, CRC(7a7ff5ae) SHA1(9e4da7ecc4d833c3ba4ddc6e5870fad53b9b2d2b) )

	ROM_REGION( 0x200000, "gfx1", ROMREGION_INVERT )	/* Sprites */
	ROM_LOAD( "ss1u306.v10", 0x000000, 0x080000, CRC(ce43a754) SHA1(3991042678badafee716b084c1768a794f144b1e) )
	ROM_LOAD( "ss1u307.v10", 0x080000, 0x080000, CRC(2170b7ec) SHA1(c9f3d12646d4e877bc2b656f977e21d927f241f6) )
	ROM_LOAD( "ss1u304.v10", 0x100000, 0x080000, CRC(546ab541) SHA1(aa96a79e3b0ba71f5e0fbb15e190d219630c2ba3) )
	ROM_LOAD( "ss1u305.v10", 0x180000, 0x080000, CRC(2a33e08b) SHA1(780cfe44a4d57b254bd0cfae8727dc77358027a8) )

	ROM_REGION( 0x100000, "gfx2", 0 )	/* Layer 1 */
	ROM_LOAD( "ss1u23.v10",  0x000000, 0x080000, CRC(9fa60901) SHA1(3d42e4174ad566b6eeb488c7a4c51db9c1fef7af) )
	ROM_LOAD( "ss1u24.v10",  0x080000, 0x080000, CRC(c10eb4b3) SHA1(70a82a750b1d9c849cd92d4f73769bbf5962c771) )

	ROM_REGION( 0x080000, "gfx3", 0 )	/* Layer 2 */
	ROM_LOAD( "ss1u25.v10",  0x000000, 0x080000, CRC(56840728) SHA1(db61539fd84f0de35ee2077238ba3646c4960cc6) )

	ROM_REGION( 0x100000, "x1snd", 0 )	/* Samples */
	ROM_LOAD( "ss1u26.v10", 0x000000, 0x080000, CRC(d452336b) SHA1(d3bf3cb383c40911758a60546f121c48087868e3) )
	ROM_LOAD( "ss1u27.v10", 0x080000, 0x080000, CRC(17fe921d) SHA1(7fc176b8eefad4f2b8532bfe62e7852d2be185ca) )
ROM_END

ROM_START( triplfun )
	ROM_REGION( 0x180000, "maincpu", 0 )
	/* the program fails its self-check but thats probably because
       its a bootleg, it does the same on the real board */
	ROM_LOAD16_BYTE( "05.bin", 0x000000, 0x40000, CRC(06eb3821) SHA1(51c67c87b5c28e693dfffd32d25cdb6d2a9448cf) )
	ROM_CONTINUE(0x100000,0x40000)
	ROM_LOAD16_BYTE( "04.bin", 0x000001, 0x40000, CRC(37a5c46e) SHA1(80b49b422a7db64d9ba5896da2b01a4588a6cf62) )
	ROM_CONTINUE(0x100001,0x40000)

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "08.bin", 0x000001, 0x80000, CRC(63a8f10f) SHA1(0a045d559b9edc9f335e6ec2d214d70c4959ec50) )
	ROM_LOAD16_BYTE( "09.bin", 0x000000, 0x80000, CRC(98cc8ca5) SHA1(1bd9d2d860e02ee4fea3d9592172690cb9d3acf2) )
	ROM_LOAD16_BYTE( "10.bin", 0x100001, 0x80000, CRC(20b0f282) SHA1(c98de63c1ad9dfe9b24f55966ccc5392c5ae82ba) )
	ROM_LOAD16_BYTE( "11.bin", 0x100000, 0x80000, CRC(276ef724) SHA1(e0c642dfd19542234abb0de68a66f8c36d9cb827) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "02.bin", 0x000000, 0x80000, CRC(4c0d1068) SHA1(cb77309474938765fd0582ab132f19fb5e21fca3) )
	ROM_LOAD16_BYTE( "03.bin", 0x000001, 0x80000, CRC(dba94e18) SHA1(3f54d874287e4ab96b2791503235488164d90cb1) )

	ROM_REGION( 0x80000, "gfx3", 0 )
	ROM_LOAD16_BYTE( "06.bin", 0x000000, 0x40000, CRC(8944bb72) SHA1(37cd0e2c8e99fb23ea70dc183a8aa0670c5f6b65) )
	ROM_LOAD16_BYTE( "07.bin", 0x000001, 0x40000, CRC(934a5d91) SHA1(aa19d2699b5ebdd99d59004005b0ce0c5140d192) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "01.bin", 0x000000, 0x40000, CRC(c186a930) SHA1(e17e1a620e380f0737b80c7f160ad643979b2799) )
ROM_END

ROM_START( qzkklgy2 )
	ROM_REGION( 0x0c0000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "fn001001.106", 0x000000, 0x080000, CRC(7bf8eb17) SHA1(f2d1666e22f564d59b37ca00c8db34ca822fd142) )
	ROM_LOAD16_WORD_SWAP( "fn001003.107", 0x080000, 0x040000, CRC(ee6ef111) SHA1(6d9efac46ba01fff8784034801cba10e38b2c923) )

	ROM_REGION( 0x100000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "fn001004.100", 0x000000, 0x100000, CRC(5ba139a2) SHA1(24fe19a7e5d2cd53bf3b1c71bf05020067f5e956) )

	ROM_REGION( 0x200000, "gfx2", 0 )	/* Layer 1 */
	ROM_LOAD( "fn001005.104", 0x000000, 0x200000, CRC(95726a63) SHA1(e53ffc2815c4858bbfb5ff452c581bccb41854c9) )

	ROM_REGION( 0x100000, "x1snd", 0 )	/* Samples */
	ROM_LOAD( "fn001006.105", 0x000000, 0x100000, CRC(83f201e6) SHA1(536e74788ad0e07451300a1ad3b127bc9d2d9063) )
ROM_END

ROM_START( wrofaero )
	ROM_REGION( 0x200000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "u3.bin",  0x000000, 0x040000, CRC(9b896a97) SHA1(f4e768911705e6def5dc4a43cfc4146c48c80caf) )
	ROM_LOAD16_BYTE( "u4.bin",  0x000001, 0x040000, CRC(dda84846) SHA1(50142692e13190900bc752908b105b65c48ea911) )

	ROM_REGION( 0x100000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "u64.bin",  0x000000, 0x080000, CRC(f06ccd78) SHA1(1701bdac2c826327441cfe0039b4cadf8f3a4803) )
	ROM_LOAD( "u63.bin",  0x080000, 0x080000, CRC(2a602a1b) SHA1(b04fa743200d62bc25a6aa34efae53209f185f79) )

	ROM_REGION( 0x080000, "gfx2", 0 )	/* Layer 1 */
	ROM_LOAD( "u66.bin",  0x000000, 0x080000, CRC(c9fc6a0c) SHA1(85ac0726221e3fedd80bd9b426d61471eb20ce46) )

	ROM_REGION( 0x080000, "gfx3", 0 )	/* Layer 2 */
	ROM_LOAD( "u68.bin",  0x000000, 0x080000, CRC(25c0c483) SHA1(2e705e7f0c66c3bc73e78ffb526606ab8be61d99) )

	ROM_REGION( 0x100000, "x1snd", 0 )	/* Samples */
	ROM_LOAD( "u69.bin",  0x000000, 0x080000, CRC(957ecd41) SHA1(3b37ba44b8b8f0f0de41c8c26c3dfdb391ba572c) )
	ROM_LOAD( "u70.bin",  0x080000, 0x080000, CRC(8d756fdf) SHA1(d66712a6aa19252f2c915ac66fc27df031fa9512) )
ROM_END

ROM_START( jjsquawk )
	ROM_REGION( 0x200000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "jj-rom1.040", 0x000000, 0x040000, CRC(7b9af960) SHA1(1718d54b0c12ae148de44f9ccccf90c0182f7b4f) )
	ROM_CONTINUE   (                0x100000, 0x040000  )
	ROM_LOAD16_BYTE( "jj-rom2.040", 0x000001, 0x040000, CRC(47dd71a3) SHA1(e219d984a1cac484ce1e570b7849562a88e0903e) )
	ROM_CONTINUE   (                0x100001, 0x040000  )

	ROM_REGION( 0x200000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "jj-rom9",  0x000000, 0x080000, CRC(27441cd3) SHA1(5867fc30c158e07f2d36ecab97b1d304383e6f35) )
	ROM_LOAD( "jj-rom10", 0x080000, 0x080000, CRC(ca2b42c4) SHA1(9b99b6618fe44a6c29a255e89dab72a0a56214df) )
	ROM_LOAD( "jj-rom7",  0x100000, 0x080000, CRC(62c45658) SHA1(82b1ea138e8f4b4ade7e44b31843aa2023c9dd71) )
	ROM_LOAD( "jj-rom8",  0x180000, 0x080000, CRC(2690c57b) SHA1(b880ded7715dffe12c4fea7ad7cb9c5133b73250) )

	ROM_REGION( 0x200000, "gfx2", 0 )	/* Layer 1 */
	ROM_LOAD       ( "jj-rom11",    0x000000, 0x080000, CRC(98b9f4b4) SHA1(de96708aebb428ddc413c3649caaec80c0c155bd) )
	ROM_LOAD       ( "jj-rom12",    0x080000, 0x080000, CRC(d4aa916c) SHA1(d619d20c33f16ab06b529fc1717ad9b703acbabf) )
	ROM_LOAD16_BYTE( "jj-rom3.040", 0x100000, 0x080000, CRC(a5a35caf) SHA1(da4bdb7f0b319f8ff972a552d0134a73e5ac1b87) )

	ROM_REGION( 0x200000, "gfx3", 0 )	/* Layer 2 */
	ROM_LOAD       ( "jj-rom14",    0x000000, 0x080000, CRC(274bbb48) SHA1(b8db632a9bbb7232d0b1debd67b3b453fd4989e6) )
	ROM_LOAD       ( "jj-rom13",    0x080000, 0x080000, CRC(51e29871) SHA1(9d33283bd9a3f57602a55cfc9fafa49edd0be8c5) )
	ROM_LOAD16_BYTE( "jj-rom4.040", 0x100000, 0x080000, CRC(a235488e) SHA1(a45d02a4451defbef7fbdab15671955fab8ed76b) )

	ROM_REGION( 0x100000, "x1snd", 0 )	/* Samples */
	ROM_LOAD( "jj-rom5.040", 0x000000, 0x080000, CRC(d99f2879) SHA1(66e83a6bc9093d19c72bd8ef1ec0523cfe218250) )
	ROM_LOAD( "jj-rom6.040", 0x080000, 0x080000, CRC(9df1e478) SHA1(f41b55821187b417ad09e4a1f439c01a107d2674) )
ROM_END

ROM_START( jjsquawkb )
	ROM_REGION( 0x200000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "3", 0x000000, 0x080000, CRC(afd5bd07) SHA1(eee231f596ce5cb9bbf41c7c9e18c11a399d7dfd) )
	ROM_LOAD16_WORD_SWAP( "2", 0x100000, 0x080000, CRC(740a7366) SHA1(2539f9a9b4fed1a1e2c354d144b8d455ed4bc144) )

	ROM_REGION( 0x400000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "4.bin",  0x000000, 0x200000, CRC(969502f7) SHA1(d6cecb38e8b73c61537cc1bdc843fc7cd695c771) )
	ROM_LOAD( "2.bin",  0x200000, 0x200000, CRC(765253d1) SHA1(4cbc6f093c87280ef9c17fecfc319cb780d755cc) )

	ROM_REGION( 0x400000, "gfxtemp", 0  )
	ROM_LOAD( "3.bin",  0x000000, 0x200000, CRC(b1e3a4bb) SHA1(be2241a4fbb99444487e7b550faac4ee1ee1ad15) )
	ROM_LOAD( "1.bin",  0x200000, 0x200000, CRC(a5d37cf7) SHA1(9573777f3cdd6b25f0bd56f65f583fddda21c900) )

	ROM_REGION( 0x200000, "gfx2", 0 )	/* Layer 1 */
	ROM_COPY( "gfxtemp", 0x000000, 0x000000, 0x100000 )
	ROM_COPY( "gfxtemp", 0x200000, 0x100000, 0x100000 )


	ROM_REGION( 0x200000, "gfx3", 0 )	/* Layer 2 */
	ROM_COPY( "gfxtemp", 0x100000, 0x000000, 0x100000 )
	ROM_COPY( "gfxtemp", 0x300000, 0x100000, 0x100000 )


	ROM_REGION( 0x100000, "x1snd", 0 )	/* Samples */
	ROM_LOAD( "1", 0x000000, 0x100000, CRC(181a55b8) SHA1(6fa404f85bad93cc15e80feb61d19bed84602b82) ) /* jj-rom5.040 + jj-rom6.040 from jjsquawk */
ROM_END

ROM_START( kamenrid )
	ROM_REGION( 0x080000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "fj001003.25", 0x000000, 0x080000, CRC(9b65d1b9) SHA1(a9183f817dbd1721cbb1a9049ca2bfc6acdf9f4a) )

	ROM_REGION( 0x200000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "fj001005.21", 0x000000, 0x100000, CRC(5d031333) SHA1(7b603e7e79c9439b526687021c0be4a5965b4c11) )
	ROM_LOAD( "fj001006.22", 0x100000, 0x100000, CRC(cf28eb78) SHA1(b1b34e0e50b5d54ff3cff908c579031a326890a2) )

	ROM_REGION( 0x80000, "user1", 0 )	/* Layers 1+2 */
	ROM_LOAD( "fj001007.152", 0x000000, 0x080000, CRC(d9ffe80b) SHA1(c1f919b53cd1b9874a5e5dc5640891e1b227cfc6) )

	ROM_REGION( 0x40000, "gfx2", 0 )	/* Layer 1 */
	ROM_COPY( "user1", 0x000000, 0x000000, 0x040000 )

	ROM_REGION( 0x40000, "gfx3", 0 )	/* Layer 2 */
	ROM_COPY( "user1", 0x040000, 0x000000, 0x040000 )

	ROM_REGION( 0x100000, "x1snd", 0 )	/* Samples */
	ROM_LOAD( "fj001008.26", 0x000000, 0x100000, CRC(45e2b329) SHA1(8526afae1aa9178570c906eb96438f174d174f4d) )
ROM_END

ROM_START( eightfrc )
	ROM_REGION( 0x200000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "uy2-u4.u3",  0x000000, 0x040000, CRC(f1f249c5) SHA1(5277b7a15934e60e0ca305c318fb02d0ffb99d42) )
	ROM_LOAD16_BYTE( "uy2-u3.u4",  0x000001, 0x040000, CRC(6f2d8618) SHA1(ea243e6064b76bc5d6e831362ac9611a48ac94a7) )

	ROM_REGION( 0x100000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "u64.bin",  0x000000, 0x080000, CRC(f561ff2e) SHA1(1ed78c90bf876f24c2859a73a71764189cebddbe) )
	ROM_LOAD( "u63.bin",  0x080000, 0x080000, CRC(4c3f8366) SHA1(b25a27a67ae828d8fcf2c8d9d373ebdaacce9c4e) )

	ROM_REGION( 0x100000, "gfx2", 0 )	/* Layer 1 */
	ROM_LOAD( "u66.bin",  0x000000, 0x100000, CRC(6fad2b7f) SHA1(469d185dc942bd4b54babf1d528e0e420f31d88b) )

	ROM_REGION( 0x100000, "gfx3", 0 )	/* Layer 2 */
	ROM_LOAD( "u68.bin",  0x000000, 0x100000, CRC(c17aad22) SHA1(eabbae2142cad3eef6a94d542ea03221c8228e94) )

	ROM_REGION( 0x240000, "x1snd", 0 )	/* Samples */
	ROM_LOAD( "u70.bin",  0x000000, 0x0c0000, CRC(dfdb67a3) SHA1(0fed6fb498dcfc1276facd0ecd2dfde45ff671f2) )
	// skip c0000-fffff (banked region)
	ROM_CONTINUE(         0x100000, 0x040000  )
	ROM_LOAD( "u69.bin",  0x140000, 0x100000, CRC(82ec08f1) SHA1(f17300d3cf990ef5c11056fd922f8cae0b2c918f) )
ROM_END

ROM_START( kiwame )
	ROM_REGION( 0x080000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "fp001001.bin", 0x000000, 0x040000, CRC(31b17e39) SHA1(4f001bf365d6c259ac8a13894e207a44c15e1d8b) )
	ROM_LOAD16_BYTE( "fp001002.bin", 0x000001, 0x040000, CRC(5a6e2efb) SHA1(a3b2ecfb5b91c6013370b359f89db0da8f120ad9) )

	ROM_REGION( 0x080000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "fp001003.bin", 0x000000, 0x080000, CRC(0f904421) SHA1(de5810746cfab1a4a7d1b055b1a97bc7fbc173dd) )

	ROM_REGION( 0x100000, "x1snd", 0 )	/* Samples */
	ROM_LOAD( "fp001006.bin", 0x000000, 0x080000, CRC(96cf395d) SHA1(877b291598e3a42e5003b2f50a16d162348ce72d) )
	ROM_LOAD( "fp001005.bin", 0x080000, 0x080000, CRC(65b5fe9a) SHA1(35605be00c7c455551d18386fcb5ad013aa2907e) )
ROM_END

ROM_START( krzybowl )
	ROM_REGION( 0x080000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "fv001.002", 0x000000, 0x040000, CRC(8c03c75f) SHA1(e56c50440681a0b06d785000018c4213266f2a4e) )
	ROM_LOAD16_BYTE( "fv001.001", 0x000001, 0x040000, CRC(f0630beb) SHA1(1ddd4ab1bc5ab2b6461eb35c8093884185828d7b) )

	ROM_REGION( 0x100000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "fv001.003", 0x000000, 0x080000, CRC(7de22749) SHA1(933a11f2d45667348b136d72806fc2e2f6f8d944) )
	ROM_LOAD( "fv001.004", 0x080000, 0x080000, CRC(c7d2fe32) SHA1(37291fa78c28be274e1240e081ea253ebe487e5c) )

	ROM_REGION( 0x100000, "x1snd", 0 )	/* Samples */
	ROM_LOAD( "fv001.005", 0x000000, 0x080000, CRC(5e206062) SHA1(e47cfb6947df178f3547dfe61907571bcb84e4ac) )
	ROM_LOAD( "fv001.006", 0x080000, 0x080000, CRC(572a15e7) SHA1(b6a3e99e14a473b78ff48d1a46b20a0862d128e9) )
ROM_END

ROM_START( orbs )	/* All eproms are socketed and labelled (handwritten) "ORBS 10\7\94" */
			/* most of rom space is unused (filled with sound samples - same data in all roms) */

	ROM_REGION( 0x100000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "orbs.u10",  0x000000, 0x080000, CRC(10f079c8) SHA1(0baf2b7e1e8be116a6fab609481c87fc7c86f305) )
	ROM_LOAD16_BYTE( "orbs.u9",   0x000001, 0x080000, CRC(f269d16f) SHA1(34f38789cb3256e334b0ac8acd9f339d14481578) )

	ROM_REGION( 0x200000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "orbs.u14",  0x000000, 0x080000, CRC(1cc76541) SHA1(d8a233212bfb9a9c686a40e470524f95b34417fa) )
	ROM_LOAD( "orbs.u13",  0x080000, 0x080000, CRC(784bdc1a) SHA1(de2c5b38561b8ba6bd800126d010b734c2751575) )
	ROM_LOAD( "orbs.u12",  0x100000, 0x080000, CRC(b8c352c2) SHA1(7d6fd1425d9d5cf6a14a1ddceba0ad10e472dfa5) )
	ROM_LOAD( "orbs.u11",  0x180000, 0x080000, CRC(58cb38ba) SHA1(1c6c5f7ccb9c81b71bc1cbad080799b97962f262) )

	ROM_REGION( 0x100000, "x1snd", 0 )	/* Samples */
	ROM_LOAD( "orbs.u15",  0x000000, 0x080000, CRC(bc0e9fe3) SHA1(758a44d07d59af8bbc87602df25dfcdc6cb8d9b3) )
	ROM_LOAD( "orbs.u16",  0x080000, 0x080000, CRC(aecd8373) SHA1(5620bcb281a9ea4920cfe81d163827013289c5bf) )
ROM_END

ROM_START( keroppi )
	ROM_REGION( 0x100000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "ft-001-001.u10",  0x000000, 0x080000, CRC(37861e7d) SHA1(4bf75f119b0ef1420c96844224850867fa8e273f) )
	ROM_LOAD16_BYTE( "ft-001-002.u9",   0x000001, 0x080000, CRC(f531d4ef) SHA1(75a99695679de083765700c250bd1fdfd8be9981) )

	ROM_REGION( 0x200000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "ft-001-003.u14",  0x000000, 0x080000, CRC(62fb22fb) SHA1(a67cb46152b73a47c2287c4058d0a22fb7064e7e) )
	ROM_LOAD( "ft-001-004.u13",  0x080000, 0x080000, CRC(69908c98) SHA1(1af069e9330a33cd2f0e1365e05c72eb23c3244e) )
	ROM_LOAD( "ft-001-005.u12",  0x100000, 0x080000, CRC(de6432a8) SHA1(afee9b29e0b3db4815fc29456044532aee03597e) )
	ROM_LOAD( "ft-001-006.u11",  0x180000, 0x080000, CRC(9c500eae) SHA1(3448adef04c9ad2e0b39a283e4eb9c9bac7d4967) )

	ROM_REGION( 0x100000, "x1snd", 0 )	/* Samples */
	ROM_LOAD( "ft-001-007.u15",  0x000000, 0x080000, CRC(c98dacf0) SHA1(b508433e2383af1e8bd5fda253c9925c48443490) )
	ROM_LOAD( "ft-001-008.u16",  0x080000, 0x080000, CRC(b9c4b637) SHA1(82977d10de1048f71525bab5431b031cca510114) )
ROM_END

ROM_START( extdwnhl )
	ROM_REGION( 0x100000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "fw001002.201",  0x000000, 0x080000, CRC(24d21924) SHA1(9914a68a578f884b06305ffcd9aeed7d83df1c7b) )
	ROM_LOAD16_BYTE( "fw001001.200",  0x000001, 0x080000, CRC(fb12a28b) SHA1(89167c042dc535b5f639057ff04a8e28824790f2) )

	ROM_REGION( 0x200000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "fw001003.202", 0x000000, 0x200000, CRC(ac9b31d5) SHA1(d362217ea0c474994e3c79ddcf87ee6688428ea5) )

	ROM_REGION( 0x400000, "gfx2", 0 )	/* Layer 1 */
	ROM_LOAD       ( "fw001004.206", 0x000000, 0x200000, CRC(0dcb1d72) SHA1(ffc84f46f06f46750bddd1a303ed83a28fa9572f) )
	ROM_LOAD16_BYTE( "fw001005.205", 0x200000, 0x100000, CRC(5c33b2f1) SHA1(9ea848aeaccbba0b71e60b39cf844665bd97928f) )

	ROM_REGION( 0x200000, "gfx3", 0 )	/* Layer 2 */
	ROM_LOAD( "fw001006.152",  0x000000, 0x200000, CRC(d00e8ddd) SHA1(e13692034afec1a0e86d19abfb9efa518b374147) )	// FIRST AND SECOND HALF IDENTICAL

	ROM_REGION( 0x100000, "x1snd", 0 )	/* Samples */
	ROM_LOAD( "fw001007.026",  0x080000, 0x080000, CRC(16d84d7a) SHA1(fdc13776ba1ec9c48a33a9f2dfe8a0e55c54d89e) )	// swapped halves
	ROM_CONTINUE(              0x000000, 0x080000  )
ROM_END

ROM_START( gundhara )
	ROM_REGION( 0x200000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "bpgh-003.u3",  0x000000, 0x080000, CRC(14e9970a) SHA1(31964bd290cc94c40684adf3a5d129b1c3addc3b) )
	ROM_LOAD16_BYTE( "bpgh-004.u4",  0x000001, 0x080000, CRC(96dfc658) SHA1(f570bc49758535eb00d93ecce9f75832f97a0d8d) )
	ROM_LOAD16_BYTE( "bpgh-002.103", 0x100000, 0x080000, CRC(312f58e2) SHA1(a74819d2f84a00c233489893f12c9ab1a98459cf) )
	ROM_LOAD16_BYTE( "bpgh-001.102", 0x100001, 0x080000, CRC(8d23a23c) SHA1(9e9a6488db424c81a97edcb7115cc070fe35c077) )

	ROM_REGION( 0x800000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "bpgh-008.u64", 0x000000, 0x200000, CRC(7ed9d272) SHA1(2e9243b3ecee27c175234f9bb1893ab498090fce) )
	ROM_LOAD( "bpgh-006.201", 0x200000, 0x200000, CRC(5a81411d) SHA1(ebf90afe027a0dc0fa3022978677fb071b9083d1) )
	ROM_LOAD( "bpgh-007.u63", 0x400000, 0x200000, CRC(aa49ce7b) SHA1(fe0064d533bd895657b88a0ef96e835443a4077f) )
	ROM_LOAD( "bpgh-005.200", 0x600000, 0x200000, CRC(74138266) SHA1(c859acff358a61a32e5810ff369b9d5528137337) )

	ROM_REGION( 0x200000, "gfx2", 0 )	/* Layer 1 */
	ROM_LOAD       ( "bpgh-010.u66", 0x000000, 0x100000, CRC(b742f0b8) SHA1(9246846c9ee839d5d84f5e02cf4605afcfd6bf7a) )
	ROM_LOAD16_BYTE( "bpgh-009.u65", 0x100000, 0x080000, CRC(b768e666) SHA1(473fa52c16c0a9f321e6429947a3e0fc1ef22f7e) )

	ROM_REGION( 0x400000, "gfx3", 0 )	/* Layer 2 */
	ROM_LOAD       ( "bpgh-012.u68", 0x000000, 0x200000, CRC(edfda595) SHA1(5942181430d59c0c303cd1cbe753910c26c109a2) )
	ROM_LOAD16_BYTE( "bpgh-011.u67", 0x200000, 0x100000, CRC(49aff270) SHA1(de25209e520cd8747042078440ee20866097d0cb) )

	ROM_REGION( 0x100000, "x1snd", 0 )	/* Samples */
	ROM_LOAD( "bpgh-013.u70",  0x080000, 0x080000, CRC(0fa5d503) SHA1(fd7a80cd25c23e737cc2c3d11de2291e22313b58) )	// swapped halves
	ROM_CONTINUE(              0x000000, 0x080000  )
ROM_END

ROM_START( sokonuke )
	ROM_REGION( 0x100000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "001-001.bin",  0x000000, 0x080000, CRC(9d0aa3ca) SHA1(f641c46f2c6e7f82bb9184daac62938afb607c09) )
	ROM_LOAD16_BYTE( "001-002.bin",  0x000001, 0x080000, CRC(96f2ef5f) SHA1(264e82e192089230f208edf609dee575bf5c6513) )

	ROM_REGION( 0x200000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "001-003.bin", 0x000000, 0x200000, CRC(ab9ba897) SHA1(650c1eadf82f6e2b4c598495c867118277565411) )

	ROM_REGION( 0x200000, "gfx2", 0 )	/* Layer 1 */
	ROM_LOAD       ( "001-004.bin", 0x000000, 0x100000, CRC(34ca3540) SHA1(a9b6b395037870033a2a422453e304fd4666b99e) )
	ROM_LOAD16_BYTE( "001-005.bin", 0x100000, 0x080000, CRC(2b95d68d) SHA1(2fb480c31a6a7e180a68bd774b5f86348bea0761) )

	ROM_REGION( 0x100, "gfx3", ROMREGION_ERASE )	/* Layer 2 */
	/* Unused */

	ROM_REGION( 0x100000, "x1snd", 0 )	/* Samples */
	ROM_LOAD( "001-006.bin",   0x080000, 0x080000, CRC(ecfac767) SHA1(3d05bdb2c2a8c7eb5fa77b0c4482f98d3947c6d6) )
	ROM_CONTINUE(              0x000000, 0x080000  )
ROM_END

ROM_START( zombraid )
	ROM_REGION( 0x200000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "fy001003.3",   0x000000, 0x080000, CRC(0b34b8f7) SHA1(8c6d7d208ece08695169f2e06806e7e55c595eb2) )
	ROM_LOAD16_BYTE( "fy001004.4",   0x000001, 0x080000, CRC(71bfeb1a) SHA1(75747b0c6e655624a5dc2e4fa8f16a6a51bd8769) )
	ROM_LOAD16_BYTE( "fy001002.103", 0x100000, 0x080000, CRC(313fd68f) SHA1(792733acc72b4719b3f7f79b57fb874c71e8abfb) )
	ROM_LOAD16_BYTE( "fy001001.102", 0x100001, 0x080000, CRC(a0f61f13) SHA1(ba14c5ae0d0b3f217c130eeebd987dfde4c64c0d) )

	ROM_REGION( 0x200000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "fy001006.200", 0x000000, 0x200000, CRC(e9ae99f7) SHA1(7ffd62e5db4a48d362c90e8fca991c5b63f22bd8) )

	ROM_REGION( 0x400000, "gfx2", 0 )	/* Layer 1 */
	ROM_LOAD       ( "fy001008.66", 0x000000, 0x200000, CRC(73d7b0e1) SHA1(aa332b563005edb1a6e20fbceaba68b56761a634) )
	ROM_LOAD16_BYTE( "fy001007.65", 0x200000, 0x100000, CRC(b2fc2c81) SHA1(2c529beccea353c3e90563215ddf3d8931e0fb83) )

	ROM_REGION( 0x400000, "gfx3", 0 )	/* Layer 2 */
	ROM_LOAD       ( "fy001010.68", 0x000000, 0x200000, CRC(8b40ed7a) SHA1(05fcd7947a8419cab5ed2305fba9a671911e4850) )
	ROM_LOAD16_BYTE( "fy001009.67", 0x200000, 0x100000, CRC(6bcca641) SHA1(49c9106e6f23e25e5b5917af11fc48d34457c61a) )

	ROM_REGION( 0x480000, "x1snd", 0 )	/* Samples */
	ROM_LOAD( "fy001012.b",  0x000000, 0x080000, CRC(fd30e102) SHA1(ae02f94f69aa301b0c37921ca1117e3ad20467b5) )
	// skip 80000-fffff (banked region)
	ROM_CONTINUE(            0x100000, 0x180000  )
	ROM_LOAD( "fy001011.a",  0x280000, 0x200000, CRC(e3c431de) SHA1(1030adacbbfabc00231417e09f3de40e3052f65c) )
ROM_END

ROM_START( madshark )
	ROM_REGION( 0x100000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "fq001002.201",  0x000000, 0x080000, CRC(4286a811) SHA1(c8d4a28008548fe7d1d70758462205862142c56b) )
	ROM_LOAD16_BYTE( "fq001001.200",  0x000001, 0x080000, CRC(38bfa0ad) SHA1(59398ef69caa01f51cdfb20db23af494db658e5e) )

	ROM_REGION( 0x200000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "fq001004.202", 0x100000, 0x100000, CRC(e56a1b5e) SHA1(f0dd34122fd7db15cc56714b72b60d07ccb59222) )
	ROM_CONTINUE(             0x000000, 0x100000 )

	ROM_REGION( 0x400000, "user1", 0 )	/* Layers 1+2 */
	ROM_LOAD       ( "fq001006.152", 0x000000, 0x200000, CRC(3bc5e8e4) SHA1(74cdf1bb2e58bef29c6f4371ff40f64472bff3ce) )
	ROM_LOAD16_BYTE( "fq001005.205", 0x200000, 0x100000, CRC(5f6c6d4a) SHA1(eed5661738282a14ce89917335fd1b695eb7351e) )

	ROM_REGION( 0x200000, "gfx2", 0 )	/* Layer 1 */
	ROM_COPY( "user1", 0x000000, 0x000000, 0x100000 )
	ROM_COPY( "user1", 0x200000, 0x100000, 0x100000 )

	ROM_REGION( 0x200000, "gfx3", 0 )	/* Layer 2 */
	ROM_COPY( "user1", 0x100000, 0x000000, 0x100000 )
	ROM_COPY( "user1", 0x300000, 0x100000, 0x100000 )

	ROM_REGION( 0x100000, "x1snd", 0 )	/* Samples */
	ROM_LOAD( "fq001007.26", 0x000000, 0x100000, CRC(e4b33c13) SHA1(c4f9532de7a09c80f5a74c3a386e99a0f546846f) )
ROM_END

ROM_START( utoukond )
	ROM_REGION( 0x100000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "93uta010.3",  0x000000, 0x080000, CRC(c486ef5e) SHA1(36e4ef4805d543216269f1161028d8a436f72284) )
	ROM_LOAD16_BYTE( "93uta011.4",  0x000001, 0x080000, CRC(978978f7) SHA1(a7fd3a4ce3a7c6c9d9bdd60df29b4f427abf4f92) )

	ROM_REGION( 0x10000, "audiocpu", 0 )		/* Z80 Code */
	ROM_LOAD( "93uta009.112", 0x0000, 0x10000, CRC(67f18483) SHA1(d9af58dec09c317ccab65553d53d82c8cce2bfb9) )

	ROM_REGION( 0x400000, "gfx1", ROMREGION_INVERT )	/* Sprites */
	ROM_LOAD( "93uta04.64",  0x000000, 0x100000, CRC(9cba0538) SHA1(83278918b6ad160d3e53c178b3cad252e7b0edfb) )
	ROM_LOAD( "93uta02.201", 0x100000, 0x100000, CRC(884fedfa) SHA1(3710003bd2e55bba03e2720fcab0fe080163222d) )
	ROM_LOAD( "93uta03.63",  0x200000, 0x100000, CRC(818484a5) SHA1(642252abe56e26aa8376db2e25b192b11586d1e4) )
	ROM_LOAD( "93uta01.200", 0x300000, 0x100000, CRC(364de841) SHA1(a025bd57f60eac05c0d7b4fb69b4b4979f357e6b) )

	ROM_REGION( 0x100000, "gfx2", 0 )	/* Layer 1 */
	ROM_LOAD( "93uta05.66",  0x000000, 0x100000, CRC(5e640bfb) SHA1(37f30ae6ab9d7860da6ca6a343fa9adf4b3d355c) )

	ROM_REGION( 0x200000, "gfx3", 0 )	/* Layer 2 */
	ROM_LOAD( "93uta07.68",  0x000000, 0x100000, CRC(67bdd036) SHA1(527b6a67e7a62263bee738dc82d6ff289ab54853) )
	ROM_LOAD( "93uta06.67",  0x100000, 0x100000, CRC(294c26e4) SHA1(459ec7f8c8db4f1e3906d5db240298405bda991c) )

	ROM_REGION( 0x100000, "x1snd", 0 )	/* Samples */
	ROM_LOAD( "93uta08.69", 0x000000, 0x100000, CRC(3d50bbcd) SHA1(e9b78d08466e1f9b42f11999bb53b6deceb81a12) )
ROM_END

ROM_START( neobattl )
	ROM_REGION( 0x100000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "bp923001.u45", 0x000000, 0x020000, CRC(0d0aeb73) SHA1(5ca631d5d68e53029f379d9877a056997c6c6afa) )	// 1CC74: "SD GUNDAM v0.00. 1992/11/04 10:04:33"
	ROM_LOAD16_BYTE( "bp923002.u46", 0x000001, 0x020000, CRC(9731fbbc) SHA1(196c913fb67496f9da2943ad1e69edf89cb65fdf) )

	ROM_REGION( 0x100000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "bp923003.u15", 0x00000, 0x80000, CRC(91ca98a1) SHA1(b02b362e3a6118f52d9e1a262ca11aecef887b00) )
	ROM_LOAD( "bp923004.u9",  0x80000, 0x80000, CRC(15c678e3) SHA1(8c0fa41a1f4e7b4e1c90faaeec7f6c910cc3ad0b) )

	ROM_REGION( 0x100000, "x1snd", 0 )	/* Samples */
	ROM_LOAD( "bp923005.u4", 0x000000, 0x100000, CRC(7c0e37be) SHA1(5d5779de948f986971a82db2a5a4302044c3257a) )
ROM_END

ROM_START( pairlove )
	ROM_REGION( 0x040000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "ut2-001-001.1a",  0x000000, 0x010000, CRC(083338b7) SHA1(d775c1618272967713bd3f3164fdfc42dc5c36ca) )
	ROM_LOAD16_BYTE( "ut2-001-002.3a",  0x000001, 0x010000, CRC(39d88aae) SHA1(8498dfb221e9b34a889594fe5ed0431814b733e6) )

	ROM_REGION( 0x100000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "ut2-001-004.5j",  0x000000, 0x080000, CRC(fdc47b26) SHA1(0de51bcf67b909ac9578f0d1b14af8a4c758aacf) )
	ROM_LOAD( "ut2-001-005.5l",  0x080000, 0x080000, CRC(076f94a2) SHA1(94b4b41a497dea1b6db5396bd7cd81ebcb217735) )

	ROM_REGION( 0x100000, "x1snd", 0 )	/* Samples */
	ROM_LOAD( "ut2-001-003.12a",  0x000000, 0x080000, CRC(900219a9) SHA1(3260a900df25beba597bf947a9fbb6f7392827d7) )
	ROM_RELOAD(                   0x080000, 0x080000  )
ROM_END

ROM_START( crazyfgt )
	ROM_REGION( 0x80000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "rom.u3", 0x00000, 0x40000, CRC(bf333e75) SHA1(be124558ca49963cc56d3255c546587558b61926) )
	ROM_LOAD16_BYTE( "rom.u4", 0x00001, 0x40000, CRC(505e9d47) SHA1(3797d396a24e46b891de4c40aafe960d1cf5f161) )

	ROM_REGION( 0x200000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "rom.u228",  0x000000, 0x80000, CRC(7181618e) SHA1(57c5aced95b0a11a43dc9bd532290f067113e65a) )
	ROM_LOAD( "rom.u227",  0x080000, 0x80000, CRC(7905b5f2) SHA1(633f86bf2be620afbe8012ade5d1e59c359a25d4) )
	ROM_LOAD( "rom.u226",  0x100000, 0x80000, CRC(ef210e34) SHA1(99241ffcbc8af889c8ab6f0bc67eedef27d455f0) )
	ROM_LOAD( "rom.u225",  0x180000, 0x80000, CRC(451b4419) SHA1(ab32b3c452b566ddfc64c0a80a257c3baadd8f41) )

	ROM_REGION( 0xc0000, "gfx2", 0 )	/* Layer 1 */
	ROM_LOAD( "rom.u67",  0x000000, 0x40000, CRC(ec8c6831) SHA1(e0ef1c2e539c1780fc5816ec950d33cb2a69d55e) )
	ROM_LOAD( "rom.u68",  0x040000, 0x80000, CRC(2124312e) SHA1(1c6053c87a975bfdf910e75bd3e38d0898806ea0) )

	ROM_REGION( 0xc0000, "gfx3", 0 )	/* Layer 2 */
	ROM_LOAD( "rom.u65",  0x000000, 0x40000, CRC(58448231) SHA1(711f24831777719f6a7b143f4f1bfd14f5a9ed4c) )
	ROM_LOAD( "rom.u66",  0x040000, 0x80000, CRC(c6f7735b) SHA1(0e77045f82d0bf659be5dbfe21cfc8f223faeee9) )

	ROM_REGION( 0x40000, "oki", 0 ) /* OKI samples */
	ROM_LOAD( "rom.u85",  0x00000, 0x40000, CRC(7b95d0bb) SHA1(f16dfd639eed6856e3ab93704caef592a07ba367) )
ROM_END

ROM_START( jockeyc )
	ROM_REGION( 0x200000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "ya-007-002-u23.bin", 0x000000, 0x10000, CRC(c499bf4d) SHA1(2417eac2972bbb0f8f0a4a1fd72c9d78537367c7) )
	ROM_LOAD16_BYTE( "ya-007-003-u33.bin", 0x000001, 0x10000, CRC(e7b0677e) SHA1(90dbd710623ff57b953483240e1006c9bda3fc91) )
	ROM_FILL(                                     0x020000, 0x60000, 0xff )
	ROM_LOAD16_WORD_SWAP( "ya_002_001.u18",       0x100000, 0x80000, CRC(dd108016) SHA1(1554de4cc1a9436a1e62400cd96c9752a2098f99) )
	ROM_FILL(                                     0x180000, 0x80000, 0xff )

	ROM_REGION( 0x200000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "ya_011_004.u10",             0x000000, 0x80000, CRC(eb74d2e0) SHA1(221ff6cc03ce57a7fcbe418f1c12a293990f8a7d) )
	//ROM_LOAD( "p523.v01_horse_fore_1.u135", 0x070000, 0x10000, CRC(3a75df30) SHA1(f3b3a7428e3e125921686bc9aacde6b28b1947b5) )

	ROM_LOAD( "ya_011_005.u17",             0x080000, 0x80000, CRC(4a6c804b) SHA1(b596b9b0b3b453c26f9c7f976ff4d56eac4fac04) )
	//ROM_LOAD( "p523.v01_horse_fore_2.u134", 0x0f0000, 0x10000, CRC(26fb0339) SHA1(a134ecef00f690c82c8bddf26498b357ccf8d5c3) )

	ROM_LOAD( "ya_011_006.u22",             0x100000, 0x80000, CRC(bfae01a5) SHA1(3be83972c3987e9bf722cd6db7770f074587301c) )
	//ROM_LOAD( "p523.v01_horse_fore_3.u133", 0x170000, 0x10000, CRC(c38596af) SHA1(d27141e28d8f8352f065c55121412e604c199a9a) )

	ROM_LOAD( "ya_011_007.u27",             0x180000, 0x80000, CRC(2dc7a294) SHA1(97f2aa9939a45aaa94d4aeb2fcd5b7f30204b942) )
	//ROM_LOAD( "p523.v01_horse_fore_4.u132", 0x1f0000, 0x10000, CRC(64ef345e) SHA1(ef5d9f293ded44a2be91278549f5db8673fc7571) )

	ROM_REGION( 0xc0000, "gfx2", 0 )	/* Layer 1 */
	ROM_LOAD( "ya_011_008.u35",             0x000000, 0x40000, CRC(4b890f83) SHA1(fde6544898a0691b550f3045803f2e81cfeb5fe9) )
	//ROM_LOAD( "p523.v01_horse_back_1.u137", 0x040000, 0x20000, CRC(39b221ea) SHA1(3b3367430733ed36d6a981cd2ec6df731d07c089) )

	ROM_LOAD( "ya_011_009.u41",             0x060000, 0x40000, CRC(caa5e3c1) SHA1(63cccc5479040a02872febc8d7f2d46096e138d1) )
	//ROM_LOAD( "p523.v01_horse_back_2.u136", 0x0a0000, 0x20000, CRC(9c5e32a0) SHA1(964734a626b5c7b9d7130addc642895df520dcb7) )

	ROM_REGION( 0x400, "proms", 0 )	/* Colours */
	ROM_LOAD16_BYTE( "ya-010.prom",  0x000, 0x200, CRC(778094b3) SHA1(270329a0d544dc7a8240d6dab08ccd54ea87ab70) )
	ROM_LOAD16_BYTE( "ya-011.prom",  0x001, 0x200, CRC(bd4fe2f6) SHA1(83d9f9db3fbfa2d172f5227c397ea4d5a9687015) )

	ROM_REGION( 0x100000, "x1snd", 0 )	/* Samples */
	ROM_LOAD( "ya_011_013.u71", 0x00000, 0x80000, CRC(2bccaf47) SHA1(1658643444d575410f11b648e0d7ae6c43fcf1ea) )
	ROM_LOAD( "ya_011_012.u64", 0x80000, 0x80000, CRC(a8015ce6) SHA1(bb0b589856ec82e1fd42be9af89b07ba1d17e595) )
ROM_END

ROM_START( inttoote )
	ROM_REGION( 0x200000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "p523.v01_horse_prog_2.002", 0x000000, 0x10000, CRC(6ce6f1ad) SHA1(82e7100721ca5b1a736f6523610b1f1edf225c12) )
	ROM_LOAD16_BYTE( "p523.v01_horse_prog_1.003", 0x000001, 0x10000, CRC(921fcff5) SHA1(cabc4e9936621132a6fbaa1a925d205c5f04a2ae) )
	ROM_FILL(                                     0x020000, 0x60000, 0xff )
	ROM_LOAD16_WORD_SWAP( "ya_002_001.u18",       0x100000, 0x80000, CRC(dd108016) SHA1(1554de4cc1a9436a1e62400cd96c9752a2098f99) )
	ROM_FILL(                                     0x180000, 0x80000, 0xff )

	ROM_REGION( 0x200000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "ya_011_004.u10",             0x000000, 0x80000, CRC(eb74d2e0) SHA1(221ff6cc03ce57a7fcbe418f1c12a293990f8a7d) )
	ROM_LOAD( "p523.v01_horse_fore_1.u135", 0x070000, 0x10000, CRC(3a75df30) SHA1(f3b3a7428e3e125921686bc9aacde6b28b1947b5) )

	ROM_LOAD( "ya_011_005.u17",             0x080000, 0x80000, CRC(4a6c804b) SHA1(b596b9b0b3b453c26f9c7f976ff4d56eac4fac04) )
	ROM_LOAD( "p523.v01_horse_fore_2.u134", 0x0f0000, 0x10000, CRC(26fb0339) SHA1(a134ecef00f690c82c8bddf26498b357ccf8d5c3) )

	ROM_LOAD( "ya_011_006.u22",             0x100000, 0x80000, CRC(bfae01a5) SHA1(3be83972c3987e9bf722cd6db7770f074587301c) )
	ROM_LOAD( "p523.v01_horse_fore_3.u133", 0x170000, 0x10000, CRC(c38596af) SHA1(d27141e28d8f8352f065c55121412e604c199a9a) )

	ROM_LOAD( "ya_011_007.u27",             0x180000, 0x80000, CRC(2dc7a294) SHA1(97f2aa9939a45aaa94d4aeb2fcd5b7f30204b942) )
	ROM_LOAD( "p523.v01_horse_fore_4.u132", 0x1f0000, 0x10000, CRC(64ef345e) SHA1(ef5d9f293ded44a2be91278549f5db8673fc7571) )

	ROM_REGION( 0xc0000, "gfx2", 0 )	/* Layer 1 */
	ROM_LOAD( "ya_011_008.u35",             0x000000, 0x40000, CRC(4b890f83) SHA1(fde6544898a0691b550f3045803f2e81cfeb5fe9) )
	ROM_LOAD( "p523.v01_horse_back_1.u137", 0x040000, 0x20000, CRC(39b221ea) SHA1(3b3367430733ed36d6a981cd2ec6df731d07c089) )

	ROM_LOAD( "ya_011_009.u41",             0x060000, 0x40000, CRC(caa5e3c1) SHA1(63cccc5479040a02872febc8d7f2d46096e138d1) )
	ROM_LOAD( "p523.v01_horse_back_2.u136", 0x0a0000, 0x20000, CRC(9c5e32a0) SHA1(964734a626b5c7b9d7130addc642895df520dcb7) )

	ROM_REGION( 0x400, "proms", 0 )	/* Colours */
	ROM_LOAD16_BYTE( "ya-010.prom",  0x000, 0x200, CRC(778094b3) SHA1(270329a0d544dc7a8240d6dab08ccd54ea87ab70) )
	ROM_LOAD16_BYTE( "ya-011.prom",  0x001, 0x200, CRC(bd4fe2f6) SHA1(83d9f9db3fbfa2d172f5227c397ea4d5a9687015) )

	ROM_REGION( 0x100000, "x1snd", 0 )	/* Samples */
	ROM_LOAD( "ya_011_013.u71", 0x00000, 0x80000, CRC(2bccaf47) SHA1(1658643444d575410f11b648e0d7ae6c43fcf1ea) )
	ROM_LOAD( "ya_011_012.u64", 0x80000, 0x80000, CRC(a8015ce6) SHA1(bb0b589856ec82e1fd42be9af89b07ba1d17e595) )
ROM_END

// set only contained 2 program roms, others are potentially incorrect
ROM_START( inttootea )
	ROM_REGION( 0x200000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "p387.v01_horse_prog_2.002", 0x000000, 0x10000, CRC(1ced885e) SHA1(7bb444bbfa3c07c0c54378432186ff3b056b6090) )
	ROM_LOAD16_BYTE( "p387.v01_horse_prog_1.003", 0x000001, 0x10000, CRC(e24592af) SHA1(86ab84cb1c5cbb0dcc73e75c05ce446411fab08a) )
	ROM_FILL(                                     0x020000, 0x60000, 0xff )
	ROM_LOAD16_WORD_SWAP( "ya_002_001.u18",       0x100000, 0x80000, CRC(dd108016) SHA1(1554de4cc1a9436a1e62400cd96c9752a2098f99) )
	ROM_FILL(                                     0x180000, 0x80000, 0xff )

	ROM_REGION( 0x200000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "ya_011_004.u10",             0x000000, 0x80000, CRC(eb74d2e0) SHA1(221ff6cc03ce57a7fcbe418f1c12a293990f8a7d) )
	ROM_LOAD( "p523.v01_horse_fore_1.u135", 0x070000, 0x10000, CRC(3a75df30) SHA1(f3b3a7428e3e125921686bc9aacde6b28b1947b5) )

	ROM_LOAD( "ya_011_005.u17",             0x080000, 0x80000, CRC(4a6c804b) SHA1(b596b9b0b3b453c26f9c7f976ff4d56eac4fac04) )
	ROM_LOAD( "p523.v01_horse_fore_2.u134", 0x0f0000, 0x10000, CRC(26fb0339) SHA1(a134ecef00f690c82c8bddf26498b357ccf8d5c3) )

	ROM_LOAD( "ya_011_006.u22",             0x100000, 0x80000, CRC(bfae01a5) SHA1(3be83972c3987e9bf722cd6db7770f074587301c) )
	ROM_LOAD( "p523.v01_horse_fore_3.u133", 0x170000, 0x10000, CRC(c38596af) SHA1(d27141e28d8f8352f065c55121412e604c199a9a) )

	ROM_LOAD( "ya_011_007.u27",             0x180000, 0x80000, CRC(2dc7a294) SHA1(97f2aa9939a45aaa94d4aeb2fcd5b7f30204b942) )
	ROM_LOAD( "p523.v01_horse_fore_4.u132", 0x1f0000, 0x10000, CRC(64ef345e) SHA1(ef5d9f293ded44a2be91278549f5db8673fc7571) )

	ROM_REGION( 0xc0000, "gfx2", 0 )	/* Layer 1 */
	ROM_LOAD( "ya_011_008.u35",             0x000000, 0x40000, CRC(4b890f83) SHA1(fde6544898a0691b550f3045803f2e81cfeb5fe9) )
	ROM_LOAD( "p523.v01_horse_back_1.u137", 0x040000, 0x20000, CRC(39b221ea) SHA1(3b3367430733ed36d6a981cd2ec6df731d07c089) )

	ROM_LOAD( "ya_011_009.u41",             0x060000, 0x40000, CRC(caa5e3c1) SHA1(63cccc5479040a02872febc8d7f2d46096e138d1) )
	ROM_LOAD( "p523.v01_horse_back_2.u136", 0x0a0000, 0x20000, CRC(9c5e32a0) SHA1(964734a626b5c7b9d7130addc642895df520dcb7) )

	ROM_REGION( 0x400, "proms", 0 )	/* Colours */
	ROM_LOAD16_BYTE( "ya-010.prom",  0x000, 0x200, CRC(778094b3) SHA1(270329a0d544dc7a8240d6dab08ccd54ea87ab70) )
	ROM_LOAD16_BYTE( "ya-011.prom",  0x001, 0x200, CRC(bd4fe2f6) SHA1(83d9f9db3fbfa2d172f5227c397ea4d5a9687015) )

	ROM_REGION( 0x100000, "x1snd", 0 )	/* Samples */
	ROM_LOAD( "ya_011_013.u71", 0x00000, 0x80000, CRC(2bccaf47) SHA1(1658643444d575410f11b648e0d7ae6c43fcf1ea) )
	ROM_LOAD( "ya_011_012.u64", 0x80000, 0x80000, CRC(a8015ce6) SHA1(bb0b589856ec82e1fd42be9af89b07ba1d17e595) )
ROM_END

/*
PCB PO-047A

3x8 DSW
SETA X1-004 (826100) (input)
SETA X1-010 (811101) (sound)
MC68B50P (timer?)
M68000
16Mhz OSC, near CPU
SETA X1-002A (sprites? - near 005 - 008)
SETA X1-001A (sprites? ^)
SETA X1-007 (737100)
SETA X1-011
SETA X1-012
*/

ROM_START( setaroul )
	ROM_REGION( 0x0c0000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "uf1002.u14", 0x000000, 0x010000, CRC(b3a622b0) SHA1(bc4a02167002579149c19640e65e679b7c19fa66) )
	ROM_LOAD16_BYTE( "uf1003.u16", 0x000001, 0x010000, CRC(a6afd769) SHA1(82c54c8a2219f20d08faf9f7afcf821d83511660) )

	ROM_REGION( 0x020000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD16_BYTE( "uf0005.u3", 0x000000, 0x008000, CRC(383c2d57) SHA1(3bbf0464f80f657dfa275e885fbce064a0a08f4a) )
	ROM_LOAD16_BYTE( "uf0006.u4", 0x000001, 0x008000, CRC(90c9dae6) SHA1(a226aab82f5b8174644281fa3efab4f8a8f8d827) )
	ROM_LOAD16_BYTE( "uf0007.u5", 0x010000, 0x008000, CRC(e72c3dba) SHA1(aaebb484e76d8f3da0ecff26c3c1bad4f3f11ac0) )
	ROM_LOAD16_BYTE( "uf0008.u6", 0x010001, 0x008000, CRC(e198e602) SHA1(f53fa36d1ea51239e71fe1ea7432bb4b7b8b3466) )

	ROM_REGION( 0x400000, "gfx2", 0 )	/* Layer 1 - 8bpp? */
	ROM_LOAD( "uf0010.u15",  0x000000, 0x080000, CRC(0af13a56) SHA1(c294b7947d004c0e0b280ca44636e4059e05a57e) )
	ROM_LOAD( "uf0009.u13",  0x080000, 0x080000, CRC(20f2d7f5) SHA1(343a8fac76d6ee7f845f9988c491698ebd0150d4) )
	ROM_LOAD( "uf0012.u29",  0x100000, 0x080000, CRC(cba2a6b7) SHA1(8627eda24c6980a0e786fd9dc06176893a33c58f) )
	ROM_LOAD( "uf0011.u22",  0x180000, 0x080000, CRC(af60adf9) SHA1(6505cbce6e066d75b779fdbe2c034ba4daabbefe) )
	ROM_LOAD( "uf0014.u38",  0x200000, 0x080000, CRC(da2bd4e4) SHA1(244af8705f2fa4ab3f3a002af16a0e4d60e03de8) )
	ROM_LOAD( "uf0013.u37",  0x280000, 0x080000, CRC(645ec3c3) SHA1(e9b8056c68bf33b0b7130a5ce2bafd11dfd6c29b) )
	ROM_LOAD( "uf0015.u40",  0x300000, 0x080000, CRC(11dc19fa) SHA1(e7084f61d075a61249d924a523c32e7993d9ae46) )
	ROM_LOAD( "uf0016.u48",  0x380000, 0x080000, CRC(10f99fa8) SHA1(7ef9a3f71dd071483cf3513ef57e2fcfe8702994) )

	ROM_REGION( 0x100000, "x1snd", 0 )	/* Samples */
	ROM_LOAD( "uf1004.u52", 0x000000, 0x020000, CRC(d63ea334) SHA1(93aaf58c90c4f704caae19b63785e471b2c1281a) )

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD16_BYTE( "ufo017.bin", 0x000, 0x200, CRC(bf50c303) SHA1(31685ed4849e5c27654f02945678db425d54bf5e) )
	ROM_LOAD16_BYTE( "ufo018.bin", 0x001, 0x200, CRC(1c584d5f) SHA1(f1c7e3da8b108d78b459cae53fabb6e28d3a7ee8) )
ROM_END

static READ16_HANDLER( twineagl_debug_r )
{
	/*  At several points in the code, the program checks if four
        consecutive bytes in this range are equal to a string, and if they
        are, it fetches an address from the following 4 bytes and jumps there.
        They are probably hooks for debug routines to be found in an extra ROM.

        0000 "MT00" + jump address
        0008 "MT01" + jump address
        0010 "MT02" + jump address
        0018 "MT03" + jump address
        0020 "MT04" + jump address
        0028 "MT05" + jump address
        0030 "MT06" + jump address
        0038 "MT07" + jump address
        0040 "WZ08" + jump address
        0080 "KB00" + jump address
        00C0 "MT18" + jump address
        00C8 "MT19" + jump address
        00D0 "MT1a" + jump address
        00D8 "MT1b" + jump address
        00E0 "MT1c" + jump address
        00E8 "MT1d" + jump address
        00F0 "MT1e" + jump address
        00F8 "MT1f" + jump address
    */

	return 0;
}

/* Extra RAM ? Check code at 0x00ba90 */
/* 2000F8 = A3 enables it, 2000F8 = 00 disables? see downtown too */
static UINT8 xram[8];
static READ16_HANDLER( twineagl_200100_r )
{
logerror("%04x: twineagl_200100_r %d\n",cpu_get_pc(space->cpu),offset);
	return xram[offset];
}
static WRITE16_HANDLER( twineagl_200100_w )
{
logerror("%04x: twineagl_200100_w %d = %02x\n",cpu_get_pc(space->cpu),offset,data);
	if (ACCESSING_BITS_0_7)
		xram[offset] = data & 0xff;
}

static DRIVER_INIT( twineagl )
{
	/* debug? */
	memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x800000, 0x8000ff, 0, 0, twineagl_debug_r);

	/* This allows 2 simultaneous players and the use of the "Copyright" Dip Switch. */
	memory_install_readwrite16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x200100, 0x20010f, 0, 0, twineagl_200100_r, twineagl_200100_w);
}


/* Protection? NVRAM is handled writing commands here */
static UINT16 downtown_protection[0x200/2];
static READ16_HANDLER( downtown_protection_r )
{
	int job = downtown_protection[0xf8/2] & 0xff;

	switch (job)
	{
		case 0xa3:
		{
			static const UINT8 word[] = "WALTZ0";
			if (offset >= 0x100/2 && offset <= 0x10a/2)	return word[offset-0x100/2];
			else										return 0;
		}
		default:
			return downtown_protection[offset] & 0xff;
	}
}

static WRITE16_HANDLER( downtown_protection_w )
{
	COMBINE_DATA(&downtown_protection[offset]);
}

static DRIVER_INIT( downtown )
{
	memory_install_readwrite16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x200000, 0x2001ff, 0, 0, downtown_protection_r, downtown_protection_w);
}


static READ16_HANDLER( arbalest_debug_r )
{
	/*  At some points in the code, the program checks if four
        consecutive bytes in this range are equal to a string, and if they
        are, it fetches an address from the following 4 bytes and jumps there.
        They are probably hooks for debug routines to be found in an extra ROM.

        0000 "CHK1" + jump address
        0008 "CHK2" + jump address
    */

	return 0;
}

static DRIVER_INIT( arbalest )
{
	memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x80000, 0x8000f, 0, 0, arbalest_debug_r);
}


static DRIVER_INIT( metafox )
{
	UINT16 *RAM = (UINT16 *) memory_region(machine, "maincpu");

	/* This game uses the 21c000-21ffff area for protection? */
//  memory_nop_readwrite(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x21c000, 0x21ffff, 0, 0);

	RAM[0x8ab1c/2] = 0x4e71;	// patch protection test: "cp error"
	RAM[0x8ab1e/2] = 0x4e71;
	RAM[0x8ab20/2] = 0x4e71;
}


static DRIVER_INIT ( blandia )
{
	/* rearrange the gfx data so it can be decoded in the same way as the other set */

	int rom_size;
	UINT8 *buf;
	UINT8 *rom;
	int rpos;

	rom_size = 0x80000;
	buf = auto_alloc_array(machine, UINT8, rom_size);

	rom = memory_region(machine, "gfx2") + 0x40000;

	for (rpos = 0; rpos < rom_size/2; rpos++) {
		buf[rpos+0x40000] = rom[rpos*2];
		buf[rpos] = rom[rpos*2+1];
	}

	memcpy( rom, buf, rom_size );

	rom = memory_region(machine, "gfx3") + 0x40000;

	for (rpos = 0; rpos < rom_size/2; rpos++) {
		buf[rpos+0x40000] = rom[rpos*2];
		buf[rpos] = rom[rpos*2+1];
	}

	memcpy( rom, buf, rom_size );

	auto_free(machine, buf);
}


static DRIVER_INIT( eightfrc )
{
	memory_nop_read(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x500004, 0x500005, 0, 0);	// watchdog??
}


static DRIVER_INIT( zombraid )
{
	memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xf00002, 0xf00003, 0, 0, zombraid_gun_r);
	memory_install_write16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xf00000, 0xf00001, 0, 0, zombraid_gun_w);
}


static DRIVER_INIT( kiwame )
{
	UINT16 *RAM = (UINT16 *) memory_region(machine, "maincpu");

	/* WARNING: This game writes to the interrupt vector
       table. Lev 1 routine address is stored at $100 */

	RAM[0x64/2] = 0x0000;
	RAM[0x66/2] = 0x0dca;
}


static DRIVER_INIT( rezon )
{
	memory_nop_read(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x500006, 0x500007, 0, 0);	// irq ack?
}

static DRIVER_INIT(wiggie)
{
	UINT8 *src;
	int len;
	UINT8 temp[16];
	int i,j;

	src = memory_region(machine, "maincpu");
	len = memory_region_length(machine, "maincpu");
	for (i = 0;i < len;i += 16)
	{
		memcpy(temp,&src[i],16);
		for (j = 0;j < 16;j++)
		{
			static const int convtable[16] =
			{
				0x0, 0x1, 0x8, 0x9,
				0x2, 0x3, 0xa, 0xb,
				0x4, 0x5, 0xc, 0xd,
				0x6, 0x7, 0xe, 0xf
			};

			src[i+j] = temp[convtable[j]];
		}


	}

	/* X1_010 is not used. */
	memory_nop_readwrite(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x100000, 0x103fff, 0, 0);

	memory_install_write16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xB00008, 0xB00009, 0, 0, wiggie_soundlatch_w);

}

static DRIVER_INIT( crazyfgt )
{
	// protection check at boot
	UINT16 *RAM = (UINT16 *) memory_region(machine, "maincpu");
	RAM[0x1078/2] = 0x4e71;

	// fixed priorities?
	seta_vregs = auto_alloc_array(machine, UINT16, 3);
	seta_vregs[0] = seta_vregs[1] = seta_vregs[2] = 0;

	DRIVER_INIT_CALL(blandia);
}

/***************************************************************************
                             International Toote
***************************************************************************/

static DRIVER_INIT( inttoote )
{
	UINT16 *ROM = (UINT16 *)memory_region( machine, "maincpu" );
	static UINT16 seta_vregs_unused[3] = {0,0,0};

	// missing / unused video regs
	seta_vregs = seta_vregs_unused;

	// code patches (to be removed...)
	ROM[0x4de0/2] = 0x4e71;	// hardware test errors
	ROM[0x4de2/2] = 0x4e71;

	ROM[0x368a/2] = 0x50f9;	// betting count down
}

static DRIVER_INIT( inttootea )
{
	//UINT16 *ROM = (UINT16 *)memory_region( machine, "maincpu" );
	static UINT16 seta_vregs_unused[3] = {0,0,0};

	// missing / unused video regs
	seta_vregs = seta_vregs_unused;

	// code patches (to be removed...)
	//ROM[0x4de0/2] = 0x4e71;   // hardware test errors
	//ROM[0x4de2/2] = 0x4e71;

	//ROM[0x368a/2] = 0x50f9;   // betting count down
}



/***************************************************************************

                                Game Drivers

***************************************************************************/

/* 68000 + 65C02 */
GAME( 1987, tndrcade, 0,        tndrcade, tndrcade, 0,        ROT270, "Seta (Taito license)",   "Thundercade / Twin Formation" , 0) // Title/License: DSW
GAME( 1987, tndrcadej,tndrcade, tndrcade, tndrcadj, 0,        ROT270, "Seta (Taito license)",   "Tokusyu Butai U.A.G. (Japan)" , 0) // License: DSW
GAME( 1988, twineagl, 0,        twineagl, twineagl, twineagl, ROT270, "Seta (Taito license)",   "Twin Eagle - Revenge Joe's Brother" , 0) // Country/License: DSW
GAME( 1989, downtown, 0,        downtown, downtown, downtown, ROT270, "Seta",                   "DownTown / Mokugeki (Set 1)" , 0) // Country/License: DSW
GAME( 1989, downtown2,downtown, downtown, downtown, downtown, ROT270, "Seta",                   "DownTown / Mokugeki (Set 2)" , 0) // Country/License: DSW
GAME( 1989, downtownj,downtown, downtown, downtown, downtown, ROT270, "Seta",                   "DownTown / Mokugeki (Joystick Hack)" , 0) // Country/License: DSW
GAME( 1989, downtownp,downtown, downtown, downtown, downtown, ROT270, "Seta",                   "DownTown / Mokugeki (prototype)" , 0) // Country/License: DSW
GAME( 1989, usclssic, 0,        usclssic, usclssic, 0,        ROT270, "Seta",                   "U.S. Classic" , 0) // Country/License: DSW
GAME( 1989, calibr50, 0,        calibr50, calibr50, 0,        ROT270, "Athena / Seta",          "Caliber 50" , 0) // Country/License: DSW
GAME( 1989, arbalest, 0,        metafox,  arbalest, arbalest, ROT270, "Seta",                   "Arbalester" , 0) // Country/License: DSW
GAME( 1989, metafox,  0,        metafox,  metafox,  metafox,  ROT270, "Seta",                   "Meta Fox" , 0) // Country/License: DSW

/* 68000 */
GAME( 198?, setaroul, 0,        setaroul, setaroul, 0,        ROT270, "Seta / Visco",           "Seta / Visco Roulette?", GAME_NOT_WORKING ) // I can't see a title in the GFX roms.  If you reset it enough times you'll get a flickery roulette wheel
GAME( 1989, drgnunit, 0,        drgnunit, drgnunit, 0,        ROT0,   "Seta",                   "Dragon Unit / Castle of Dragon", 0 )
GAME( 1989, wits,     0,        wits,     wits,     0,        ROT0,   "Athena (Visco license)", "Wit's (Japan)" , 0) // Country/License: DSW
GAME( 1990, thunderl, 0,        thunderl, thunderl, 0,        ROT270, "Seta",                   "Thunder & Lightning" , 0) // Country/License: DSW
GAME( 1994, wiggie,   0,        wiggie,   thunderl, wiggie,   ROT270, "Promat",                 "Wiggie Waggie", GAME_IMPERFECT_GRAPHICS ) // hack of Thunder & Lightning
GAME( 1994, superbar, wiggie,   superbar, thunderl, wiggie,   ROT270, "Promat",                 "Super Bar", GAME_IMPERFECT_GRAPHICS ) // hack of Thunder & Lightning
GAME( 1990, jockeyc,  0,        jockeyc,  jockeyc,  0,        ROT0,   "Seta (Visco license)",   "Jockey Club", 0 )
GAME( 1998, inttoote, jockeyc,  inttoote, inttoote, inttoote, ROT0,   "Coinmaster",             "International Toote (Germany)", GAME_NOT_WORKING | GAME_UNEMULATED_PROTECTION )
GAME( 1993, inttootea,jockeyc,  inttoote, inttoote, inttootea,ROT0,   "Coinmaster",             "International Toote II (World?)", GAME_NOT_WORKING | GAME_UNEMULATED_PROTECTION )
GAME( 1991, rezon,    0,        rezon,    rezon,    rezon,    ROT0,   "Allumer",                "Rezon", 0 )
GAME( 1992, rezont,   rezon,    rezon,    rezont,   rezon,    ROT0,   "Allumer (Taito license)","Rezon (Taito)", 0 )
GAME( 1991, stg,      0,        drgnunit, stg,      0,        ROT270, "Athena / Tecmo",         "Strike Gunner S.T.G", 0 )
GAME( 1991, pairlove, 0,        pairlove, pairlove, 0,        ROT270, "Athena",                 "Pairs Love", 0 )
GAME( 1992, blandia,  0,        blandia,  blandia,  blandia,  ROT0,   "Allumer",                "Blandia", GAME_IMPERFECT_GRAPHICS )
GAME( 1992, blandiap, blandia,  blandiap, blandia,  0,        ROT0,   "Allumer",                "Blandia (prototype)", GAME_IMPERFECT_GRAPHICS )
GAME( 1992, blockcar, 0,        blockcar, blockcar, 0,        ROT90,  "Visco",                  "Block Carnival / Thunder & Lightning 2" , 0) // Title: DSW
GAME( 1992, qzkklogy, 0,        drgnunit, qzkklogy, 0,        ROT0,   "Tecmo",                  "Quiz Kokology", 0 )
GAME( 1992, neobattl, 0,        umanclub, neobattl, 0,        ROT270, "Banpresto / Sotsu Agency. Sunrise", "SD Gundam Neo Battling (Japan)", 0 )
GAME( 1992, umanclub, 0,        umanclub, umanclub, 0,        ROT0,   "Banpresto / Tsuburaya Productions", "Ultraman Club - Tatakae! Ultraman Kyoudai!!", 0 )
GAME( 1992, zingzip,  0,        zingzip,  zingzip,  0,        ROT270, "Allumer / Tecmo",        "Zing Zing Zip", 0 )
GAME( 1993, atehate,  0,        atehate,  atehate,  0,        ROT0,   "Athena",                 "Athena no Hatena ?", 0 )
GAME( 1993, daioh,    0,        daioh,    daioh,    0,        ROT270, "Athena",                 "Daioh", 0 )
GAME( 1993, jjsquawk, 0,        jjsquawk, jjsquawk, 0,        ROT0,   "Athena / Able",          "J. J. Squawkers", GAME_IMPERFECT_SOUND )
GAME( 1993, jjsquawkb,jjsquawk, jjsquawb, jjsquawk, 0,        ROT0,   "bootleg",                "J. J. Squawkers (bootleg)", GAME_IMPERFECT_SOUND )
GAME( 1993, kamenrid, 0,        kamenrid, kamenrid, 0,        ROT0,   "Banpresto / Toei",       "Masked Riders Club Battle Race", 0 )
GAME( 1993, madshark, 0,        madshark, madshark, 0,        ROT270, "Allumer",                "Mad Shark", 0 )
GAME( 1993, msgundam, 0,        msgundam, msgundam, 0,        ROT0,   "Banpresto",              "Mobile Suit Gundam", 0 )
GAME( 1993, msgundam1,msgundam, msgundam, msgunda1, 0,        ROT0,   "Banpresto",              "Mobile Suit Gundam (Japan)", 0 )
GAME( 1993, oisipuzl, 0,        oisipuzl, oisipuzl, 0,        ROT0,   "Sunsoft / Atlus",        "Oishii Puzzle Ha Irimasenka", 0 )
GAME( 1993, triplfun, oisipuzl, triplfun, oisipuzl, 0,        ROT0,   "bootleg",                "Triple Fun", 0 )
GAME( 1993, qzkklgy2, 0,        qzkklgy2, qzkklgy2, 0,        ROT0,   "Tecmo",                  "Quiz Kokology 2", 0 )
GAME( 1993, utoukond, 0,        utoukond, utoukond, 0,        ROT0,   "Banpresto / Tsuburaya Productions", "Ultra Toukon Densetsu (Japan)", 0 )
GAME( 1993, wrofaero, 0,        wrofaero, wrofaero, 0,        ROT270, "Yang Cheng",             "War of Aero - Project MEIOU", 0 )
GAME( 1994, eightfrc, 0,        eightfrc, eightfrc, eightfrc, ROT90,  "Tecmo",                  "Eight Forces", 0 )
GAME( 1994, kiwame,   0,        kiwame,   kiwame,   kiwame,   ROT0,   "Athena",                 "Pro Mahjong Kiwame", 0 )
GAME( 1994, krzybowl, 0,        krzybowl, krzybowl, 0,        ROT270, "American Sammy",         "Krazy Bowl", 0 )
GAME( 1994, orbs,     0,        orbs,     orbs,     0,        ROT0,   "American Sammy",         "Orbs (10/7/94 prototype?)", 0 )
GAME( 1993, keroppi,  0,        keroppi,  keroppi,  0,        ROT0,   "Sammy Industries",       "Kero Kero Keroppi no Issyoni Asobou (Japan)", 0 )
GAME( 1995, extdwnhl, 0,        extdwnhl, extdwnhl, 0,        ROT0,   "Sammy Industries Japan", "Extreme Downhill (v1.5)", GAME_IMPERFECT_GRAPHICS )
GAME( 1995, gundhara, 0,        gundhara, gundhara, 0,        ROT270, "Banpresto",              "Gundhara", 0 )
GAME( 1995, sokonuke, 0,        extdwnhl, sokonuke, 0,        ROT0,   "Sammy Industries",       "Sokonuke Taisen Game (Japan)", GAME_IMPERFECT_SOUND )
GAME( 1995, zombraid, 0,        gundhara, zombraid, zombraid, ROT0,   "American Sammy",         "Zombie Raid (US)", GAME_NO_COCKTAIL )
GAME( 1996, crazyfgt, 0,        crazyfgt, crazyfgt, crazyfgt, ROT0,   "Subsino",                "Crazy Fight", GAME_UNEMULATED_PROTECTION | GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND )

