/***************************************************************************

Tokio          (c) 1986 Taito
Bubble Bobble  (c) 1986 Taito

driver by Chris Moore, Nicola Salmoria
also based on Tokio driver by Marcelo de G. Malheiros <malheiro@dca.fee.unicamp.br>


Main clock: XTAL = 24 MHz
Horizontal video frequency: HSYNC = XTAL/4/384 = 15.625 kHz
Video frequency: VSYNC = HSYNC/264 = 59.185606 Hz
VBlank duration: 1/VSYNC * (40/264) = 2560 us

****************************************************************************

Bubble Bobble ROM info

CPU Board
---------
           | Taito  |Romstar | ?????  |Romstar |
           |        |        |missing |mode sel|
17  CU1    | A78-01 |   ->   |   ->   |   ->   |   protection mcu (JPH1011P)
49  PAL1   | A78-02 |   ->   |   ->   |   ->   |   address decoder
43  PAL2   | A78-03 |   ->   |   ->   |   ->   |   address decoder
12  PAL3   | A78-04 |   ->   |   ->   |   ->   |   address decoder
53  empty  |        |        |        |        |   main prg
52  ROM1   | A78-05 | A78-21 | A78-22 | A78-24 |   main prg
51  ROM2   | A78-06 |   ->   | A78-23 | A78-25 |   main prg
46  ROM4   | A78-07 |   ->   |   ->   |   ->   |   sound prg
37  ROM3   | A78-08 |   ->   |   ->   |   ->   |   sub prg

Video Board
-----------
12  ROM1   | A78-09 |   ->   |   ->   |   ->   |   gfx
13  ROM2   | A78-10 |   ->   |   ->   |   ->   |   gfx
14  ROM3   | A78-11 |   ->   |   ->   |   ->   |   gfx
15  ROM4   | A78-12 |   ->   |   ->   |   ->   |   gfx
16  ROM5   | A78-13 |   ->   |   ->   |   ->   |   gfx
17  ROM6   | A78-14 |   ->   |   ->   |   ->   |   gfx
18  empty  |        |        |        |        |   gfx
19  empty  |        |        |        |        |   gfx
30  ROM7   | A78-15 |   ->   |   ->   |   ->   |   gfx
31  ROM8   | A78-16 |   ->   |   ->   |   ->   |   gfx
32  ROM9   | A78-17 |   ->   |   ->   |   ->   |   gfx
33  ROM10  | A78-18 |   ->   |   ->   |   ->   |   gfx
34  ROM11  | A78-19 |   ->   |   ->   |   ->   |   gfx
35  ROM12  | A78-20 |   ->   |   ->   |   ->   |   gfx
36  empty  |        |        |        |        |   gfx
37  empty  |        |        |        |        |   gfx
41  ROM13  | A71-25 |   ->   |   ->   |   ->   |   video timing


Bobble Bobble memory map

most of the address decoding is done by various PALs which haven't been read, so
the memory map is inferred by program behaviour

CPU #1

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
0xxxxxxxxxxxxxxx R   xxxxxxxx ROM 51    program ROM
10xxxxxxxxxxxxxx R   xxxxxxxx ROM 52/53 program ROM (banked)
110xxxxxxxxxxxxx R/W xxxxxxxx VRAM      video RAM
11100xxxxxxxxxxx R/W xxxxxxxx WORK      RAM shared with CPU #2
11101xxxxxxxxxxx R/W xxxxxxxx WORK      RAM shared with CPU #2
11110xxxxxxxxxxx R/W xxxxxxxx WORK      RAM shared with CPU #2
1111100xxxxxxxxx R/W xxxxxxxx COLOR     palette RAM
111110100-----00   W xxxxxxxx SOUND     command for sound CPU
111110100-----01   W --------           n.c.
111110100-----10   W --------           n.c.
111110100-----11   W -------x SRESET    reset sound CPU and sound chips
111110100-----00 R   xxxxxxxx           answer from sound CPU (not used)
111110100-----01 R   -------x           message pending from sound CPU to CPU #1 (not used)
111110100-----01 R   ------x-           message pending from CPU #1 to sound CPU (not used)
111110100-----10 R                      n.c.
111110100-----11 R                      n.c.
111110101-------   W -------- TRES?     watchdog reset
1111101100------   W -------- NMIRQ     trigger NMI on CPU #2 (not used)
1111101101------   W -----xxx           ROM bank
1111101101------   W ----x---           n.c.
1111101101------   W ---x---- SBRES     reset CPU #2
1111101101------   W --x----- SEQRES    reset MCU
1111101101------   W -x------ BLACK     blank screen
1111101101------   W x------- VHINV     flip screen
1111101110------   W --------           n.c.
1111101111------   W --------           n.c.
111110111-------   W --------           n.c.
111111xxxxxxxxxx R/W xxxxxxxx MCRAM     RAM shared with MCU


CPU #2

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
0xxxxxxxxxxxxxxx R   xxxxxxxx ROM 37    program ROM
111xxxxxxxxxxxxx R/W xxxxxxxx WORK      RAM shared with CPU #1 [1]

[1] The last 2kB could be used exclusively by this CPU, but they are not used at all.


Sound CPU

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
0xxxxxxxxxxxxxxx R   xxxxxxxx ROM 46    program ROM
1000xxxxxxxxxxxx R/W xxxxxxxx RAM 47    work RAM
1001-----------x R/W xxxxxxxx FMA       YM2203
1010-----------x R/W xxxxxxxx FMB       YM3526
1011----------00 R   xxxxxxxx           command from CPU #1
1011----------01 R   -------x           message pending from sound CPU to CPU #1
1011----------01 R   ------x-           message pending from CPU #1 to sound CPU
1011----------10 R                      n.c.
1011----------11 R                      n.c.
1011----------00   W xxxxxxxx           answer to CPU #1
1011----------01   W --------           sound NMI enable
1011----------10   W --------           sound NMI disable
1011----------11   W                    n.c.
111------------- R   xxxxxxxx           space for diagnostic ROM?


****************************************************************************


Tokio
1986, Taito (Romstar license)

PCB Layout                                                                   G Pinout
----------                                                                   --------
                                                                   Component Side  Solder Side
Top Board                                                         ----------------|----------------
                                                                       Ground   1 | A  Ground
J1100069A CPU BOARD                                                 Video Red   2 | B  Video Ground
K1100157A                                                         Video Green   3 | C  Video Blue
M4300053A (sticker)                                                Video Sync   4 | D
  |------------------------------------------------------|          Speaker +   5 | E  Speaker -
  |   VOL      TL074          YM2203              Z80A   |                Key   6 | F  Key
|-|                                                      |                      7 | H
|     MB3731  PC010SA                      A71_07.IC10   |            Coin Sw   8 | J
|H                                                       |         Coin Meter   9 | K
|     TL7700                                      2016   |                     10 | L
|-|                                                      |            Service  11 | M  Slam Switch
  |   MC68705P5                           A71_06-1.IC8  |-|          1P Start  12 | N  2P Start
|-| (A71_24.IC57)                                       | |                    13 | P
|                                           A71_05.IC7  | |                    14 | R
|                                                       | |          1P Right  15 | S  2P Right
|                                           A71_04.IC6  | |          1P Left   16 | T  2P Left
|   PC030CM                   PAL                       | |                    17 | U
|                                         A71_28-1.IC5  | |                    18 | V
|G                                                      | |                    19 | W
|                                         A71_27-1.IC4  | |                    20 | X
|                                                       | |           1P Fire  21 | Y  2P Fire
|                                                       | |                    22 | Z
|                                                 Z80B  | |
|-|                                                     |-|
  |                                                      |                   H Pinout
  |                    2016                       Z80B   |                   --------
  |                                                      |                   1   Ground
  |                    2016                              |                   2   Ground
  |                                         A71_01.IC1   |                   3   Ground
  |   DSWB(8) DSWA(8)  2016                              |                   4   Ground
  |------------------------------------------------------|                   5   +5VDC
Notes:                                                                       6   +5VDC
        H - 12-pin Connector for Power Input                                 7   +5VDC
        G - 22-Way Edge Connector                                            8   -5VDC
      PAL - MMI PAL16L8B stamped 'A71-26' (DIP20)                            9   +12VDC
  PC030CM - Taito Custom Ceramic IC, possibly contains a smt logic IC,       10  Key
            smt resistors and smt capacitors (SIP20)                         11  +12VDC
  PC010SA - Taito Custom Ceramic IC, possibly contains                       12  +12VDC
            smt resistors and smt capacitors, sound DAC (SIP14)
   MB3731 - Fujitsu MB3731 18W Power Amplifier (SIP12)
   TL7700 - Texas Instruments TL7700 Supply-Voltage Supervisor/Power-On Reset IC (DIP8)
MC68705P5 - Motorola MC68705P5 Microcontroller with 2K Internal EPROM,
            clock input 3.000MHz [24/8] (DIP28)
     2016 - Toshiba TMM2016BP-10 2K x8 SRAM (DIP24)
     Z80A - Zilog Z8400APS Z80A CPU, clock input 3.000MHz [24/8] (DIP40)
     Z80B - Zilog Z8400BPS Z80B CPU, clock input 6.000MHz [24/4] (DIP40)
   YM2203 - Yamaha YM2203 Sound Chip, clock input 3.000MHz [24/8] (DIP40)
    VSync - 60Hz

     ROMs - (All EPROMs are 27C256)
            A71_07.IC10    Sound CPU Program

            A71_06-1.IC8 \
            A71_05.IC7   |
            A71_04.IC6   | Main CPU Program
            A71_28-1.IC5 |
            A71_27-1.IC4 /

            A71_01.IC1     Sub CPU Program

            A71_24.IC57    68705 Microcontroller Program (Protected, Not Dumped)



Bottom Board

J1100070A VIDEO BOARD
K1100158A
K1100172A (sticker)
  |------------------------------------------------------|                   T Pinout
  |                                     24MHz            |                   --------
  |                                                      |         Component Side  Solder Side
  |                                     2018             |        ----------------|-----------
  |                                     2018             |             Ground   1 | A  Ground
  |                     A71_25.IC41                      |             Ground   2 | B  Ground
  |                                     2018             |             Ground   3 | C  Ground
  |                                     2018            |-|            Ground   4 | D  Ground
  |                                                     | |                     5 | E
|-|                                                     | |                     6 | F
|                                                       | |                     7 | H
|    A71_08.IC12  A71_16.IC30                           | |                     8 | J
|                                                       | |                     9 | K
|T   A71_09.IC13  A71_17.IC31                           | |                    10 | L
|                                                       | |                    11 | M
|    A71_10.IC14  A71_18.IC32                           | |                    12 | N
|                                                       | |                    13 | P
|    A71_11.IC15  A71_19.IC33                           | |                    14 | R
|-|                                     2018            | |               +5V  15 | S  +5V
  |  A71_12.IC16  A71_20.IC34           2018            |-|               +5V  16 | T  +5V
  |                                                      |                +5V  17 | U  +5V
  |  A71_13.IC17  A71_21.IC35           2018             |                +5V  18 | V  +5V
  |                                     2018             |
  |  A71_14.IC18  A71_22.IC36           PC040DA          |
  |                                     PC040DA          |
  |  A71_15.IC19  A71_23.IC37           PC040DA          |
  |------------------------------------------------------|
Notes:
        T - 36-Way Edge Connector
  PC040DA - Taito Custom Ceramic IC, Video DAC (SIP19)
     2018 - Toshiba TMM2018D-45 2K x8 SRAM (DIP24)

     ROMs -
            A71_25.IC41    MMI 63S141 Bipolar PROM (DIP16)
            All EPROMs are 27C256


****************************************************************************


Notes:
- The coin inputs are handled by a custom called PC030. It would be responsible of
  handling the coin counters.
- There is a weird dip switch in Bubble Bobble (SWB #7). When it is on, the game
  takes the player score and the level number (increased by 1) and writes them,
  byte by byte, to $F7FE and $F7FF ($F7FF receives the same data but with the
  bit order reversed). After doing that, it sometimes hangs because it expects
  the value at $F7FF to change.
  This is done by routines $0F26 (player 1) and $0F74 (player 2).
  Frankly I don't know what this could be. The schematics don't show anything
  special there. A debug feature seems unlikely - why care about the score?
  Could it be provision for some kind of externally controlled redemption scheme?
- "Attract Sound" in Tokio is a very relative term - it just plays a very short
  sound every four rounds of demo play.

TODO:
- the Bubble Bobble MCU is actually an MC6801U4, which has additional features like
    internal timers (similar to the HD63701). I've just mapped the I/O ports since
    that's the only thing required for normal operation, but the program does
    use some of the additional features in its special "test" mode.
- tokio: doesn't work due to missing MC68705 MCU protection emulation.
- tokio: sound support is probably incomplete. There are a couple of unknown
  accesses done by the CPU, including to the YM2203 I/O ports. At the
  very least, there should be some filters.

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m6800/m6800.h"
#include "deprecat.h"
#include "sound/2203intf.h"
#include "sound/3526intf.h"
#include "cpu/m6805/m6805.h"
#include "includes/bublbobl.h"


#define MAIN_XTAL	XTAL_24MHz


/*************************************
 *
 *  Address maps
 *
 *************************************/

static ADDRESS_MAP_START( master_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xdcff) AM_RAM AM_BASE_SIZE_MEMBER(bublbobl_state, videoram, videoram_size)
	AM_RANGE(0xdd00, 0xdfff) AM_RAM AM_BASE_SIZE_MEMBER(bublbobl_state, objectram, objectram_size)
	AM_RANGE(0xe000, 0xf7ff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0xf800, 0xf9ff) AM_RAM_WRITE(paletteram_RRRRGGGGBBBBxxxx_be_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0xfa00, 0xfa00) AM_READWRITE(bublbobl_sound_status_r, bublbobl_sound_command_w)
	AM_RANGE(0xfa03, 0xfa03) AM_WRITE(bublbobl_soundcpu_reset_w)
	AM_RANGE(0xfa80, 0xfa80) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0xfb40, 0xfb40) AM_WRITE(bublbobl_bankswitch_w)
	AM_RANGE(0xfc00, 0xffff) AM_RAM AM_BASE_MEMBER(bublbobl_state, mcu_sharedram)
ADDRESS_MAP_END

static ADDRESS_MAP_START( slave_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xe000, 0xf7ff) AM_RAM AM_SHARE("share1")
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x8fff) AM_RAM
	AM_RANGE(0x9000, 0x9001) AM_DEVREADWRITE("ym1", ym2203_r, ym2203_w)
	AM_RANGE(0xa000, 0xa001) AM_DEVREADWRITE("ym2", ym3526_r, ym3526_w)
	AM_RANGE(0xb000, 0xb000) AM_READWRITE(soundlatch_r, bublbobl_sound_status_w)
	AM_RANGE(0xb001, 0xb001) AM_WRITE(bublbobl_sh_nmi_enable_w) AM_READNOP
	AM_RANGE(0xb002, 0xb002) AM_WRITE(bublbobl_sh_nmi_disable_w)
	AM_RANGE(0xe000, 0xffff) AM_ROM	// space for diagnostic ROM?
ADDRESS_MAP_END

static ADDRESS_MAP_START( mcu_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0000) AM_READWRITE(bublbobl_mcu_ddr1_r, bublbobl_mcu_ddr1_w)
	AM_RANGE(0x0001, 0x0001) AM_READWRITE(bublbobl_mcu_ddr2_r, bublbobl_mcu_ddr2_w)
	AM_RANGE(0x0002, 0x0002) AM_READWRITE(bublbobl_mcu_port1_r, bublbobl_mcu_port1_w)
	AM_RANGE(0x0003, 0x0003) AM_READWRITE(bublbobl_mcu_port2_r, bublbobl_mcu_port2_w)
	AM_RANGE(0x0004, 0x0004) AM_READWRITE(bublbobl_mcu_ddr3_r, bublbobl_mcu_ddr3_w)
	AM_RANGE(0x0005, 0x0005) AM_READWRITE(bublbobl_mcu_ddr4_r, bublbobl_mcu_ddr4_w)
	AM_RANGE(0x0006, 0x0006) AM_READWRITE(bublbobl_mcu_port3_r, bublbobl_mcu_port3_w)
	AM_RANGE(0x0007, 0x0007) AM_READWRITE(bublbobl_mcu_port4_r, bublbobl_mcu_port4_w)
	AM_RANGE(0x0040, 0x00ff) AM_RAM
	AM_RANGE(0xf000, 0xffff) AM_ROM
ADDRESS_MAP_END

// The 68705 is from a bootleg, the original MCU is a 6801U4
static ADDRESS_MAP_START( bootlegmcu_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0x7ff)
	AM_RANGE(0x000, 0x000) AM_READWRITE(bublbobl_68705_port_a_r, bublbobl_68705_port_a_w)
	AM_RANGE(0x001, 0x001) AM_READWRITE(bublbobl_68705_port_b_r, bublbobl_68705_port_b_w)
	AM_RANGE(0x002, 0x002) AM_READ_PORT("IN0")	// COIN
	AM_RANGE(0x004, 0x004) AM_WRITE(bublbobl_68705_ddr_a_w)
	AM_RANGE(0x005, 0x005) AM_WRITE(bublbobl_68705_ddr_b_w)
	AM_RANGE(0x006, 0x006) AM_WRITENOP // ???
	AM_RANGE(0x010, 0x07f) AM_RAM
	AM_RANGE(0x080, 0x7ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( bootleg_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xdcff) AM_RAM AM_BASE_SIZE_MEMBER(bublbobl_state, videoram, videoram_size)
	AM_RANGE(0xdd00, 0xdfff) AM_RAM AM_BASE_SIZE_MEMBER(bublbobl_state, objectram, objectram_size)
	AM_RANGE(0xe000, 0xf7ff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0xf800, 0xf9ff) AM_RAM_WRITE(paletteram_RRRRGGGGBBBBxxxx_be_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0xfa00, 0xfa00) AM_READWRITE(bublbobl_sound_status_r, bublbobl_sound_command_w)
	AM_RANGE(0xfa03, 0xfa03) AM_WRITE(bublbobl_soundcpu_reset_w)
	AM_RANGE(0xfa80, 0xfa80) AM_WRITENOP // ???
	AM_RANGE(0xfb40, 0xfb40) AM_WRITE(bublbobl_bankswitch_w)
	AM_RANGE(0xfc00, 0xfcff) AM_RAM
	AM_RANGE(0xfd00, 0xfdff) AM_RAM
	AM_RANGE(0xfe00, 0xfe03) AM_READWRITE(boblbobl_ic43_a_r, boblbobl_ic43_a_w)
	AM_RANGE(0xfe80, 0xfe83) AM_READWRITE(boblbobl_ic43_b_r, boblbobl_ic43_b_w)
	AM_RANGE(0xff00, 0xff00) AM_READ_PORT("DSW0")
	AM_RANGE(0xff01, 0xff01) AM_READ_PORT("DSW1")
	AM_RANGE(0xff02, 0xff02) AM_READ_PORT("IN0")
	AM_RANGE(0xff03, 0xff03) AM_READ_PORT("IN1")
	AM_RANGE(0xff94, 0xff94) AM_WRITENOP // ???
	AM_RANGE(0xff98, 0xff98) AM_WRITENOP // ???
ADDRESS_MAP_END


static ADDRESS_MAP_START( tokio_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xdcff) AM_RAM AM_BASE_SIZE_MEMBER(bublbobl_state, videoram, videoram_size)
	AM_RANGE(0xdd00, 0xdfff) AM_RAM AM_BASE_SIZE_MEMBER(bublbobl_state, objectram, objectram_size)
	AM_RANGE(0xe000, 0xf7ff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0xf800, 0xf9ff) AM_RAM_WRITE(paletteram_RRRRGGGGBBBBxxxx_be_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0xfa00, 0xfa00) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0xfa03, 0xfa03) AM_READ_PORT("DSW0")
	AM_RANGE(0xfa04, 0xfa04) AM_READ_PORT("DSW1")
	AM_RANGE(0xfa05, 0xfa05) AM_READ_PORT("IN0")
	AM_RANGE(0xfa06, 0xfa06) AM_READ_PORT("IN1")
	AM_RANGE(0xfa07, 0xfa07) AM_READ_PORT("IN2")
	AM_RANGE(0xfa80, 0xfa80) AM_WRITE(tokio_bankswitch_w)
	AM_RANGE(0xfb00, 0xfb00) AM_WRITE(tokio_videoctrl_w)
	AM_RANGE(0xfb80, 0xfb80) AM_WRITE(bublbobl_nmitrigger_w)
	AM_RANGE(0xfc00, 0xfc00) AM_READWRITE(bublbobl_sound_status_r, bublbobl_sound_command_w)
	AM_RANGE(0xfe00, 0xfe00) AM_READ(tokio_mcu_r) AM_WRITENOP // ???
ADDRESS_MAP_END

static ADDRESS_MAP_START( tokio_slave_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x97ff) AM_RAM AM_SHARE("share1")
ADDRESS_MAP_END

static ADDRESS_MAP_START( tokio_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x8fff) AM_RAM
	AM_RANGE(0x9000, 0x9000) AM_READWRITE(soundlatch_r, bublbobl_sound_status_w)
	AM_RANGE(0x9800, 0x9800) AM_READNOP	// ???
	AM_RANGE(0xa000, 0xa000) AM_WRITE(bublbobl_sh_nmi_disable_w)
	AM_RANGE(0xa800, 0xa800) AM_WRITE(bublbobl_sh_nmi_enable_w)
	AM_RANGE(0xb000, 0xb001) AM_DEVREADWRITE("ymsnd", ym2203_r, ym2203_w)
	AM_RANGE(0xe000, 0xffff) AM_ROM	// space for diagnostic ROM?
ADDRESS_MAP_END


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( bublbobl )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SPECIAL )	// output: coin lockout
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SPECIAL )	// output: select 1-way or 2-way coin counter
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SPECIAL )	// output: trigger IRQ on main CPU (jumper switchable to vblank)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SPECIAL )	// output: select read or write shared RAM

	PORT_START("DSW0")
	PORT_DIPNAME( 0x05, 0x04, "Mode" )						PORT_DIPLOCATION("DSW-A:1,3")
	PORT_DIPSETTING(    0x04, "Game, English" )
	PORT_DIPSETTING(    0x05, "Game, Japanese" )
	PORT_DIPSETTING(    0x01, "Test (Grid and Inputs)" )
	PORT_DIPSETTING(    0x00, "Test (RAM and Sound)/Pause" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )		PORT_DIPLOCATION("DSW-A:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )		PORT_DIPLOCATION("DSW-A:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) )			PORT_DIPLOCATION("DSW-A:5,6")
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_B ) )			PORT_DIPLOCATION("DSW-A:7,8")
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )		PORT_DIPLOCATION("DSW-B:1,2")
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )		PORT_DIPLOCATION("DSW-B:3,4")
	PORT_DIPSETTING(    0x08, "20K 80K 300K" )
	PORT_DIPSETTING(    0x0c, "30K 100K 400K" )
	PORT_DIPSETTING(    0x04, "40K 200K 500K" )
	PORT_DIPSETTING(    0x00, "50K 250K 500K" )
	// then more bonus lives at 1M 2M 3M 4M 5M - for all dip switch settings
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )			PORT_DIPLOCATION("DSW-B:5,6")
	PORT_DIPSETTING(    0x10, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )			PORT_DIPLOCATION("DSW-B:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )				// must be off (see notes)
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "ROM Type" )					PORT_DIPLOCATION("DSW-B:8")
	PORT_DIPSETTING(    0x80, "IC52=512kb, IC53=none" )		// will hang on startup if set to wrong type
	PORT_DIPSETTING(    0x00, "IC52=256kb, IC53=256kb" )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_2WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( boblbobl )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x05, 0x04, "Mode" )
	PORT_DIPSETTING(    0x04, "Game, English" )
	PORT_DIPSETTING(    0x05, "Game, Japanese" )
	PORT_DIPSETTING(    0x01, "Test (Grid and Inputs)" )
	PORT_DIPSETTING(    0x00, "Test (RAM and Sound)" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x08, "20K 80K 300K" )
	PORT_DIPSETTING(    0x0c, "30K 100K 400K" )
	PORT_DIPSETTING(    0x04, "40K 200K 500K" )
	PORT_DIPSETTING(    0x00, "50K 250K 500K" )
	// then more bonus lives at 1M 2M 3M 4M 5M - for all dip switch settings
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x10, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPNAME( 0xc0, 0x00, "Monster Speed" )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x80, DEF_STR( High ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Very_High ) )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_2WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_TILT ) // ???
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( sboblbob )
	PORT_INCLUDE( boblbobl )

	PORT_MODIFY( "DSW0" )
	PORT_DIPNAME( 0x01, 0x00, "Game" )
	PORT_DIPSETTING(    0x01, "Bobble Bobble" )
	PORT_DIPSETTING(    0x00, "Super Bobble Bobble" )
	PORT_SERVICE( 0x04, IP_ACTIVE_LOW )

	PORT_MODIFY( "DSW1" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x10, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "100 (Cheat)")
INPUT_PORTS_END

// default to 'Dream Land' not 'Super Dream Land'
static INPUT_PORTS_START( dland )
	PORT_INCLUDE( boblbobl )

	PORT_MODIFY( "DSW0" )
	PORT_DIPNAME( 0x01, 0x01, "Game" )
	PORT_DIPSETTING(    0x01, "Dream Land" )
	PORT_DIPSETTING(    0x00, "Super Dream Land" )
	PORT_SERVICE( 0x04, IP_ACTIVE_LOW )

	PORT_MODIFY( "DSW1" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x10, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "100 (Cheat)")
INPUT_PORTS_END

static INPUT_PORTS_START( tokio )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )		PORT_DIPLOCATION("SW A:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )	PORT_DIPLOCATION("SW A:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW A:3" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )	PORT_DIPLOCATION("SW A:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) )		PORT_DIPLOCATION("SW A:5,6")
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_B ) )		PORT_DIPLOCATION("SW A:7,8")
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "Enemies" )				PORT_DIPLOCATION("SW B:1")
	PORT_DIPSETTING(    0x01, "Few (Easy)" )
	PORT_DIPSETTING(    0x00, "Many (Hard)" )
	PORT_DIPNAME( 0x02, 0x02, "Enemy Shots" )			PORT_DIPLOCATION("SW B:2")
	PORT_DIPSETTING(    0x02, "Few (Easy)" )
	PORT_DIPSETTING(    0x00, "Many (Hard)" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )	PORT_DIPLOCATION("SW B:3,4")
	PORT_DIPSETTING(    0x0c, "100K 400K" )
	PORT_DIPSETTING(    0x08, "200K 400K" )
	PORT_DIPSETTING(    0x04, "300K 400K" )
	PORT_DIPSETTING(    0x00, "400K 400K" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )		PORT_DIPLOCATION("SW B:5,6")
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x00, "99 (Cheat)")	// 6 in original version
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW B:7" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Language ) )		PORT_DIPLOCATION("SW B:8")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Japanese ) )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SPECIAL )	// data ready from MCU
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout charlayout =
{
	8, 8,
	RGN_FRAC(1,2),
	4,
	{ 0, 4, RGN_FRAC(1,2), RGN_FRAC(1,2)+4 },
	{ 3, 2, 1, 0, 8+3, 8+2, 8+1, 8+0 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static GFXDECODE_START( bublbobl )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 16 )
GFXDECODE_END


/*************************************
 *
 *  Sound interface
 *
 *************************************/

// handler called by the 2203 emulator when the internal timers cause an IRQ
static void irqhandler(running_device *device, int irq)
{
	bublbobl_state *state = (bublbobl_state *)device->machine->driver_data;
	cpu_set_input_line(state->audiocpu, 0, irq ? ASSERT_LINE : CLEAR_LINE);
}

static const ym2203_interface ym2203_config =
{
	{
		AY8910_LEGACY_OUTPUT,
		AY8910_DEFAULT_LOADS,
		DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL
	},
	irqhandler
};



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_START( common )
{
	bublbobl_state *state = (bublbobl_state *)machine->driver_data;

	state->maincpu = machine->device("maincpu");
	state->mcu = machine->device("mcu");
	state->audiocpu = machine->device("audiocpu");
	state->slave = machine->device("slave");

	state_save_register_global(machine, state->sound_nmi_enable);
	state_save_register_global(machine, state->pending_nmi);
	state_save_register_global(machine, state->sound_status);
	state_save_register_global(machine, state->video_enable);
}

static MACHINE_RESET( common )
{
	bublbobl_state *state = (bublbobl_state *)machine->driver_data;

	state->sound_nmi_enable = 0;
	state->pending_nmi = 0;
	state->sound_status = 0;
}


static MACHINE_START( tokio )
{
	bublbobl_state *state = (bublbobl_state *)machine->driver_data;

	MACHINE_START_CALL(common);

	state_save_register_global(machine, state->tokio_prot_count);
}

static MACHINE_RESET( tokio )
{
	bublbobl_state *state = (bublbobl_state *)machine->driver_data;

	MACHINE_RESET_CALL(common);

	state->tokio_prot_count = 0;
}

static MACHINE_DRIVER_START( tokio )

	/* driver data */
	MDRV_DRIVER_DATA(bublbobl_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, MAIN_XTAL/4)	// 6 MHz
	MDRV_CPU_PROGRAM_MAP(tokio_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_CPU_ADD("slave", Z80, MAIN_XTAL/4)	// 6 MHz
	MDRV_CPU_PROGRAM_MAP(tokio_slave_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_CPU_ADD("audiocpu", Z80, MAIN_XTAL/8)	// 3 MHz
	MDRV_CPU_PROGRAM_MAP(tokio_sound_map) // NMIs are triggered by the main CPU, IRQs are triggered by the YM2203

	MDRV_QUANTUM_TIME(HZ(6000)) // 100 CPU slices per frame - a high value to ensure proper synchronization of the CPUs

	MDRV_MACHINE_START(tokio)
	MDRV_MACHINE_RESET(tokio)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_RAW_PARAMS(MAIN_XTAL/4, 384, 0, 256, 264, 16, 240)

	MDRV_GFXDECODE(bublbobl)
	MDRV_PALETTE_LENGTH(256)

	MDRV_VIDEO_UPDATE(bublbobl)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ymsnd", YM2203, MAIN_XTAL/8)
	MDRV_SOUND_CONFIG(ym2203_config)
	MDRV_SOUND_ROUTE(0, "mono", 0.08)
	MDRV_SOUND_ROUTE(1, "mono", 0.08)
	MDRV_SOUND_ROUTE(2, "mono", 0.08)
	MDRV_SOUND_ROUTE(3, "mono", 1.0)
MACHINE_DRIVER_END


static MACHINE_START( bublbobl )
{
	bublbobl_state *state = (bublbobl_state *)machine->driver_data;

	MACHINE_START_CALL(common);

	state_save_register_global(machine, state->ddr1);
	state_save_register_global(machine, state->ddr2);
	state_save_register_global(machine, state->ddr3);
	state_save_register_global(machine, state->ddr4);
	state_save_register_global(machine, state->port1_in);
	state_save_register_global(machine, state->port2_in);
	state_save_register_global(machine, state->port3_in);
	state_save_register_global(machine, state->port4_in);
	state_save_register_global(machine, state->port1_out);
	state_save_register_global(machine, state->port2_out);
	state_save_register_global(machine, state->port3_out);
	state_save_register_global(machine, state->port4_out);
}

static MACHINE_RESET( bublbobl )
{
	bublbobl_state *state = (bublbobl_state *)machine->driver_data;

	MACHINE_RESET_CALL(common);

	state->ddr1 = 0;
	state->ddr2 = 0;
	state->ddr3 = 0;
	state->ddr4 = 0;
	state->port1_in = 0;
	state->port2_in = 0;
	state->port3_in = 0;
	state->port4_in = 0;
	state->port1_out = 0;
	state->port2_out = 0;
	state->port3_out = 0;
	state->port4_out = 0;
}

static MACHINE_DRIVER_START( bublbobl )

	/* driver data */
	MDRV_DRIVER_DATA(bublbobl_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, MAIN_XTAL/4)	// 6 MHz
	MDRV_CPU_PROGRAM_MAP(master_map)
	// IRQs are triggered by the MCU

	MDRV_CPU_ADD("slave", Z80, MAIN_XTAL/4)	// 6 MHz
	MDRV_CPU_PROGRAM_MAP(slave_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_CPU_ADD("audiocpu", Z80, MAIN_XTAL/8)	// 3 MHz
	MDRV_CPU_PROGRAM_MAP(sound_map) // IRQs are triggered by the YM2203

	MDRV_CPU_ADD("mcu", M6801, 4000000)	// actually 6801U4  // xtal is 4MHz, divided by 4 internally
	MDRV_CPU_PROGRAM_MAP(mcu_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_pulse) // comes from the same clock that latches the INT pin on the second Z80

	MDRV_QUANTUM_TIME(HZ(6000)) // 100 CPU slices per frame - a high value to ensure proper synchronization of the CPUs

	MDRV_MACHINE_START(bublbobl)
	MDRV_MACHINE_RESET(bublbobl)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_RAW_PARAMS(MAIN_XTAL/4, 384, 0, 256, 264, 16, 240)

	MDRV_GFXDECODE(bublbobl)
	MDRV_PALETTE_LENGTH(256)

	MDRV_VIDEO_UPDATE(bublbobl)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ym1", YM2203, MAIN_XTAL/8)
	MDRV_SOUND_CONFIG(ym2203_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MDRV_SOUND_ADD("ym2", YM3526, MAIN_XTAL/8)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END


static MACHINE_START( boblbobl )
{
	bublbobl_state *state = (bublbobl_state *)machine->driver_data;

	MACHINE_START_CALL(common);

	state_save_register_global(machine, state->ic43_a);
	state_save_register_global(machine, state->ic43_b);
}

static MACHINE_RESET( boblbobl )
{
	bublbobl_state *state = (bublbobl_state *)machine->driver_data;

	MACHINE_RESET_CALL(common);

	state->ic43_a = 0;
	state->ic43_b = 0;
}

static MACHINE_DRIVER_START( boblbobl )
	MDRV_IMPORT_FROM(bublbobl)

	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(bootleg_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)	// interrupt mode 1, unlike Bubble Bobble

	MDRV_MACHINE_START(boblbobl)
	MDRV_MACHINE_RESET(boblbobl)

	MDRV_DEVICE_REMOVE("mcu")
MACHINE_DRIVER_END


static MACHINE_START( bub68705 )
{
	bublbobl_state *state = (bublbobl_state *)machine->driver_data;

	MACHINE_START_CALL(common);

	state_save_register_global(machine, state->port_a_in);
	state_save_register_global(machine, state->port_a_out);
	state_save_register_global(machine, state->ddr_a);
	state_save_register_global(machine, state->port_b_in);
	state_save_register_global(machine, state->port_b_out);
	state_save_register_global(machine, state->ddr_b);
	state_save_register_global(machine, state->address);
	state_save_register_global(machine, state->latch);
}

static MACHINE_RESET( bub68705 )
{
	bublbobl_state *state = (bublbobl_state *)machine->driver_data;

	MACHINE_RESET_CALL(common);

	state->port_a_in = 0;
	state->port_a_out = 0;
	state->ddr_a = 0;
	state->port_b_in = 0;
	state->port_b_out = 0;
	state->ddr_b = 0;
	state->address = 0;
	state->latch = 0;
}

static MACHINE_DRIVER_START( bub68705 )
	MDRV_IMPORT_FROM(bublbobl)
	MDRV_DEVICE_REMOVE("mcu")

	MDRV_CPU_ADD("mcu", M68705, 4000000)	// xtal is 4MHz, divided by 4 internally
	MDRV_CPU_PROGRAM_MAP(bootlegmcu_map)
	MDRV_CPU_VBLANK_INT_HACK(bublbobl_m68705_interrupt, 2) // ??? should come from the same clock which latches the INT pin on the second Z80

	MDRV_MACHINE_START(bub68705)
	MDRV_MACHINE_RESET(bub68705)
MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/
ROM_START( tokio ) // newer japan set, has -1 revision of roms 02, 03 and 06
	ROM_REGION( 0x30000, "maincpu", 0 )	/* main CPU */
	ROM_LOAD( "a71-02-1.ic4", 0x00000, 0x8000, CRC(BB8DABD7) SHA1(141E9F0C19BCF316477681369E2D98DFFDD8435D) )
	/* ROMs banked at 8000-bfff */
	ROM_LOAD( "a71-03-1.ic5", 0x10000, 0x8000, CRC(EE49B383) SHA1(D510A1D168542DF6A87C7D7C67A47CF776A51F29) )
	ROM_LOAD( "a71-04.ic6",   0x18000, 0x8000, CRC(a0a4ce0e) SHA1(c49bdcd85c760a5e7327d1b424772e1560f1a318) )
	ROM_LOAD( "a71-05.ic7",   0x20000, 0x8000, CRC(6da0b945) SHA1(6c80b8333dd95657f99e6ba5b6e877733ac02a8c) )
	ROM_LOAD( "a71-06-1.ic8", 0x28000, 0x8000, CRC(56927B3F) SHA1(33FB4E71B95664ECFF1F35F6782A14101982A56D) )

	ROM_REGION( 0x10000, "slave", 0 )	/* video CPU */
	ROM_LOAD( "a71-01.ic1",   0x00000, 0x8000, CRC(0867c707) SHA1(7129974f1252b28e9e338bd3c7fcb87210dcf412) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* audio CPU */
	ROM_LOAD( "a71-07.ic10",  0x0000, 0x08000, CRC(f298cc7b) SHA1(ebf5c804aa07b7f198ec3e1f8d1e111cd89ebdf3) )

	ROM_REGION( 0x0800, "cpu3", 0 )	/* 2k for the microcontroller (68705P5) */
	ROM_LOAD( "a71-24.ic57",  0x0000, 0x0800, NO_DUMP )

	ROM_REGION( 0x80000, "gfx1", ROMREGION_INVERT ) /* gfx roms, on gfx board */
	ROM_LOAD( "a71-08.ic12",  0x00000, 0x8000, CRC(0439ab13) SHA1(84142220a6a29f0e34f7c7c751b583bf394df8ce) )    /* 1st plane */
	ROM_LOAD( "a71-09.ic13",  0x08000, 0x8000, CRC(edb3d2ff) SHA1(0c6e4bbc786a097f9d99220e72f98c1c795a7292) )
	ROM_LOAD( "a71-10.ic14",  0x10000, 0x8000, CRC(69f0888c) SHA1(1704ab6339981195cd09d581e83094c75037d18e) )
	ROM_LOAD( "a71-11.ic15",  0x18000, 0x8000, CRC(4ae07c31) SHA1(452d1eb5a70e7853791cd05e4578c1454477bdec) )
	ROM_LOAD( "a71-12.ic16",  0x20000, 0x8000, CRC(3f6bd706) SHA1(b03c534a95b71941331d3ffd9aa7069b5f05687e) )
	ROM_LOAD( "a71-13.ic17",  0x28000, 0x8000, CRC(f2c92aaa) SHA1(7dfdc473794a298032405ba918df8085b0bbe174) )
	ROM_LOAD( "a71-14.ic18",  0x30000, 0x8000, CRC(c574b7b2) SHA1(9839adce60c0017ae3997603a2aece511af226d2) )
	ROM_LOAD( "a71-15.ic19",  0x38000, 0x8000, CRC(12d87e7f) SHA1(327a80f08207ee66721738f7e1c53f75b5659be0) )
	ROM_LOAD( "a71-16.ic30",  0x40000, 0x8000, CRC(0bce35b6) SHA1(3f0496db6681c7be1e36ba41296115d158d7457a) )    /* 2nd plane */
	ROM_LOAD( "a71-17.ic31",  0x48000, 0x8000, CRC(deda6387) SHA1(40f0be3a71b0a03f0275da72f4124424b162318a) )
	ROM_LOAD( "a71-18.ic32",  0x50000, 0x8000, CRC(330cd9d7) SHA1(919f78036b760938d6aa72754be1a615f568b470) )
	ROM_LOAD( "a71-19.ic33",  0x58000, 0x8000, CRC(fc4b29e0) SHA1(d11393a24b5c6c04f5058b299e4b0fc773a03e4b) )
	ROM_LOAD( "a71-20.ic34",  0x60000, 0x8000, CRC(65acb265) SHA1(2ef940f994e76d4387be6e0d53a565813cc59636) )
	ROM_LOAD( "a71-21.ic35",  0x68000, 0x8000, CRC(33cde9b2) SHA1(9b227ab609e3c7c6be90c29739a57ea4959cd68e) )
	ROM_LOAD( "a71-22.ic36",  0x70000, 0x8000, CRC(fb98eac0) SHA1(57615c3934de5510eeeb0ba16024abda8ee95303) )
	ROM_LOAD( "a71-23.ic37",  0x78000, 0x8000, CRC(30bd46ad) SHA1(6e1618ed237c769d1a8d329fbd7a9f7216993215) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "a71-25.ic41",  0x0000, 0x0100, CRC(2d0f8545) SHA1(089c31e2f614145ef2743164f7b52ae35bc06808) )	/* video timing, on gfx board */

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "a71-26.ic19",  0x0000, 0x0117, CRC(4e1f119c) SHA1(0ac8eb2fdb202951e5f7145f92dfd10fe96b294b) )
ROM_END

ROM_START( tokioo ) // older japan set, has older roms 02, 03, 06
	ROM_REGION( 0x30000, "maincpu", 0 )	/* main CPU */
	ROM_LOAD( "a71-02.ic4",   0x00000, 0x8000, CRC(d556c908) SHA1(d5d8afb7f7888d77aa9a372dfbab75fbd0358cc3) )
	/* ROMs banked at 8000-bfff */
	ROM_LOAD( "a71-03.ic5",   0x10000, 0x8000, CRC(69dacf44) SHA1(ee8c33702749c0e2562951f9f80c897d3fbd7dd7) )
	ROM_LOAD( "a71-04.ic6",   0x18000, 0x8000, CRC(a0a4ce0e) SHA1(c49bdcd85c760a5e7327d1b424772e1560f1a318) )
	ROM_LOAD( "a71-05.ic7",   0x20000, 0x8000, CRC(6da0b945) SHA1(6c80b8333dd95657f99e6ba5b6e877733ac02a8c) )
	ROM_LOAD( "a71-06.ic8",   0x28000, 0x8000, CRC(447d6779) SHA1(5b329b221357a9cea777415d409a6423529a925c) )

	ROM_REGION( 0x10000, "slave", 0 )	/* video CPU */
	ROM_LOAD( "a71-01.ic1",   0x00000, 0x8000, CRC(0867c707) SHA1(7129974f1252b28e9e338bd3c7fcb87210dcf412) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* audio CPU */
	ROM_LOAD( "a71-07.ic10",  0x0000, 0x08000, CRC(f298cc7b) SHA1(ebf5c804aa07b7f198ec3e1f8d1e111cd89ebdf3) )

	ROM_REGION( 0x0800, "cpu3", 0 )	/* 2k for the microcontroller (68705P5) */
	ROM_LOAD( "a71-24.ic57",  0x0000, 0x0800, NO_DUMP )

	ROM_REGION( 0x80000, "gfx1", ROMREGION_INVERT ) /* gfx roms, on gfx board */
	ROM_LOAD( "a71-08.ic12",  0x00000, 0x8000, CRC(0439ab13) SHA1(84142220a6a29f0e34f7c7c751b583bf394df8ce) )    /* 1st plane */
	ROM_LOAD( "a71-09.ic13",  0x08000, 0x8000, CRC(edb3d2ff) SHA1(0c6e4bbc786a097f9d99220e72f98c1c795a7292) )
	ROM_LOAD( "a71-10.ic14",  0x10000, 0x8000, CRC(69f0888c) SHA1(1704ab6339981195cd09d581e83094c75037d18e) )
	ROM_LOAD( "a71-11.ic15",  0x18000, 0x8000, CRC(4ae07c31) SHA1(452d1eb5a70e7853791cd05e4578c1454477bdec) )
	ROM_LOAD( "a71-12.ic16",  0x20000, 0x8000, CRC(3f6bd706) SHA1(b03c534a95b71941331d3ffd9aa7069b5f05687e) )
	ROM_LOAD( "a71-13.ic17",  0x28000, 0x8000, CRC(f2c92aaa) SHA1(7dfdc473794a298032405ba918df8085b0bbe174) )
	ROM_LOAD( "a71-14.ic18",  0x30000, 0x8000, CRC(c574b7b2) SHA1(9839adce60c0017ae3997603a2aece511af226d2) )
	ROM_LOAD( "a71-15.ic19",  0x38000, 0x8000, CRC(12d87e7f) SHA1(327a80f08207ee66721738f7e1c53f75b5659be0) )
	ROM_LOAD( "a71-16.ic30",  0x40000, 0x8000, CRC(0bce35b6) SHA1(3f0496db6681c7be1e36ba41296115d158d7457a) )    /* 2nd plane */
	ROM_LOAD( "a71-17.ic31",  0x48000, 0x8000, CRC(deda6387) SHA1(40f0be3a71b0a03f0275da72f4124424b162318a) )
	ROM_LOAD( "a71-18.ic32",  0x50000, 0x8000, CRC(330cd9d7) SHA1(919f78036b760938d6aa72754be1a615f568b470) )
	ROM_LOAD( "a71-19.ic33",  0x58000, 0x8000, CRC(fc4b29e0) SHA1(d11393a24b5c6c04f5058b299e4b0fc773a03e4b) )
	ROM_LOAD( "a71-20.ic34",  0x60000, 0x8000, CRC(65acb265) SHA1(2ef940f994e76d4387be6e0d53a565813cc59636) )
	ROM_LOAD( "a71-21.ic35",  0x68000, 0x8000, CRC(33cde9b2) SHA1(9b227ab609e3c7c6be90c29739a57ea4959cd68e) )
	ROM_LOAD( "a71-22.ic36",  0x70000, 0x8000, CRC(fb98eac0) SHA1(57615c3934de5510eeeb0ba16024abda8ee95303) )
	ROM_LOAD( "a71-23.ic37",  0x78000, 0x8000, CRC(30bd46ad) SHA1(6e1618ed237c769d1a8d329fbd7a9f7216993215) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "a71-25.ic41",  0x0000, 0x0100, CRC(2d0f8545) SHA1(089c31e2f614145ef2743164f7b52ae35bc06808) )	/* video timing, on gfx board */

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "a71-26.ic19",  0x0000, 0x0117, CRC(4e1f119c) SHA1(0ac8eb2fdb202951e5f7145f92dfd10fe96b294b) )
ROM_END

ROM_START( tokiou )
	ROM_REGION( 0x30000, "maincpu", 0 )	/* main CPU */
	ROM_LOAD( "a71-27-1.ic4", 0x00000, 0x8000, CRC(8c180896) SHA1(bc8aeb42da4bae7db6f65b9874224f60a9bc4500) )
	/* ROMs banked at 8000-bfff */
	ROM_LOAD( "a71-28-1.ic5", 0x10000, 0x8000, CRC(1b447527) SHA1(6939e6c1b8492825d18f4e96f39ff45f4c96eea2) )
	ROM_LOAD( "a71-04.ic6",   0x18000, 0x8000, CRC(a0a4ce0e) SHA1(c49bdcd85c760a5e7327d1b424772e1560f1a318) )
	ROM_LOAD( "a71-05.ic7",   0x20000, 0x8000, CRC(6da0b945) SHA1(6c80b8333dd95657f99e6ba5b6e877733ac02a8c) )
	ROM_LOAD( "a71-06-1.ic8", 0x28000, 0x8000, CRC(56927b3f) SHA1(33fb4e71b95664ecff1f35f6782a14101982a56d) )

	ROM_REGION( 0x10000, "slave", 0 )	/* video CPU */
	ROM_LOAD( "a71-01.ic1",   0x00000, 0x8000, CRC(0867c707) SHA1(7129974f1252b28e9e338bd3c7fcb87210dcf412) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* audio CPU */
	ROM_LOAD( "a71-07.ic10",  0x0000, 0x08000, CRC(f298cc7b) SHA1(ebf5c804aa07b7f198ec3e1f8d1e111cd89ebdf3) )

	ROM_REGION( 0x0800, "cpu3", 0 )	/* 2k for the microcontroller (68705P5) */
	ROM_LOAD( "a71-24.ic57",  0x0000, 0x0800, NO_DUMP )

	ROM_REGION( 0x80000, "gfx1", ROMREGION_INVERT ) /* gfx roms, on gfx board */
	ROM_LOAD( "a71-08.ic12",  0x00000, 0x8000, CRC(0439ab13) SHA1(84142220a6a29f0e34f7c7c751b583bf394df8ce) )    /* 1st plane */
	ROM_LOAD( "a71-09.ic13",  0x08000, 0x8000, CRC(edb3d2ff) SHA1(0c6e4bbc786a097f9d99220e72f98c1c795a7292) )
	ROM_LOAD( "a71-10.ic14",  0x10000, 0x8000, CRC(69f0888c) SHA1(1704ab6339981195cd09d581e83094c75037d18e) )
	ROM_LOAD( "a71-11.ic15",  0x18000, 0x8000, CRC(4ae07c31) SHA1(452d1eb5a70e7853791cd05e4578c1454477bdec) )
	ROM_LOAD( "a71-12.ic16",  0x20000, 0x8000, CRC(3f6bd706) SHA1(b03c534a95b71941331d3ffd9aa7069b5f05687e) )
	ROM_LOAD( "a71-13.ic17",  0x28000, 0x8000, CRC(f2c92aaa) SHA1(7dfdc473794a298032405ba918df8085b0bbe174) )
	ROM_LOAD( "a71-14.ic18",  0x30000, 0x8000, CRC(c574b7b2) SHA1(9839adce60c0017ae3997603a2aece511af226d2) )
	ROM_LOAD( "a71-15.ic19",  0x38000, 0x8000, CRC(12d87e7f) SHA1(327a80f08207ee66721738f7e1c53f75b5659be0) )
	ROM_LOAD( "a71-16.ic30",  0x40000, 0x8000, CRC(0bce35b6) SHA1(3f0496db6681c7be1e36ba41296115d158d7457a) )    /* 2nd plane */
	ROM_LOAD( "a71-17.ic31",  0x48000, 0x8000, CRC(deda6387) SHA1(40f0be3a71b0a03f0275da72f4124424b162318a) )
	ROM_LOAD( "a71-18.ic32",  0x50000, 0x8000, CRC(330cd9d7) SHA1(919f78036b760938d6aa72754be1a615f568b470) )
	ROM_LOAD( "a71-19.ic33",  0x58000, 0x8000, CRC(fc4b29e0) SHA1(d11393a24b5c6c04f5058b299e4b0fc773a03e4b) )
	ROM_LOAD( "a71-20.ic34",  0x60000, 0x8000, CRC(65acb265) SHA1(2ef940f994e76d4387be6e0d53a565813cc59636) )
	ROM_LOAD( "a71-21.ic35",  0x68000, 0x8000, CRC(33cde9b2) SHA1(9b227ab609e3c7c6be90c29739a57ea4959cd68e) )
	ROM_LOAD( "a71-22.ic36",  0x70000, 0x8000, CRC(fb98eac0) SHA1(57615c3934de5510eeeb0ba16024abda8ee95303) )
	ROM_LOAD( "a71-23.ic37",  0x78000, 0x8000, CRC(30bd46ad) SHA1(6e1618ed237c769d1a8d329fbd7a9f7216993215) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "a71-25.ic41",  0x0000, 0x0100, CRC(2d0f8545) SHA1(089c31e2f614145ef2743164f7b52ae35bc06808) )	/* video timing, on gfx board */

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "a71-26.ic19",  0x0000, 0x0117, CRC(4e1f119c) SHA1(0ac8eb2fdb202951e5f7145f92dfd10fe96b294b) )
ROM_END

ROM_START( tokiob )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* main CPU */
	ROM_LOAD( "2.ic4",        0x00000, 0x8000, CRC(f583b1ef) SHA1(a97b36299b51792953516224191f11decc579a38) )
	/* ROMs banked at 8000-bfff */
	ROM_LOAD( "a71-03.ic5",   0x10000, 0x8000, CRC(69dacf44) SHA1(ee8c33702749c0e2562951f9f80c897d3fbd7dd7) )
	ROM_LOAD( "a71-04.ic6",   0x18000, 0x8000, CRC(a0a4ce0e) SHA1(c49bdcd85c760a5e7327d1b424772e1560f1a318) )
	ROM_LOAD( "a71-05.ic7",   0x20000, 0x8000, CRC(6da0b945) SHA1(6c80b8333dd95657f99e6ba5b6e877733ac02a8c) )
	ROM_LOAD( "6.ic8",        0x28000, 0x8000, CRC(1490e95b) SHA1(a73e1857a1029156f0b5f7f7fe34a37870e72209) )

	ROM_REGION( 0x10000, "slave", 0 )	/* video CPU */
	ROM_LOAD( "a71-01.ic1",   0x00000, 0x8000, CRC(0867c707) SHA1(7129974f1252b28e9e338bd3c7fcb87210dcf412) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* audio CPU */
	ROM_LOAD( "a71-07.ic10",  0x0000, 0x08000, CRC(f298cc7b) SHA1(ebf5c804aa07b7f198ec3e1f8d1e111cd89ebdf3) )

	ROM_REGION( 0x80000, "gfx1", ROMREGION_INVERT ) /* gfx roms, on gfx board */
	ROM_LOAD( "a71-08.ic12",  0x00000, 0x8000, CRC(0439ab13) SHA1(84142220a6a29f0e34f7c7c751b583bf394df8ce) )    /* 1st plane */
	ROM_LOAD( "a71-09.ic13",  0x08000, 0x8000, CRC(edb3d2ff) SHA1(0c6e4bbc786a097f9d99220e72f98c1c795a7292) )
	ROM_LOAD( "a71-10.ic14",  0x10000, 0x8000, CRC(69f0888c) SHA1(1704ab6339981195cd09d581e83094c75037d18e) )
	ROM_LOAD( "a71-11.ic15",  0x18000, 0x8000, CRC(4ae07c31) SHA1(452d1eb5a70e7853791cd05e4578c1454477bdec) )
	ROM_LOAD( "a71-12.ic16",  0x20000, 0x8000, CRC(3f6bd706) SHA1(b03c534a95b71941331d3ffd9aa7069b5f05687e) )
	ROM_LOAD( "a71-13.ic17",  0x28000, 0x8000, CRC(f2c92aaa) SHA1(7dfdc473794a298032405ba918df8085b0bbe174) )
	ROM_LOAD( "a71-14.ic18",  0x30000, 0x8000, CRC(c574b7b2) SHA1(9839adce60c0017ae3997603a2aece511af226d2) )
	ROM_LOAD( "a71-15.ic19",  0x38000, 0x8000, CRC(12d87e7f) SHA1(327a80f08207ee66721738f7e1c53f75b5659be0) )
	ROM_LOAD( "a71-16.ic30",  0x40000, 0x8000, CRC(0bce35b6) SHA1(3f0496db6681c7be1e36ba41296115d158d7457a) )    /* 2nd plane */
	ROM_LOAD( "a71-17.ic31",  0x48000, 0x8000, CRC(deda6387) SHA1(40f0be3a71b0a03f0275da72f4124424b162318a) )
	ROM_LOAD( "a71-18.ic32",  0x50000, 0x8000, CRC(330cd9d7) SHA1(919f78036b760938d6aa72754be1a615f568b470) )
	ROM_LOAD( "a71-19.ic33",  0x58000, 0x8000, CRC(fc4b29e0) SHA1(d11393a24b5c6c04f5058b299e4b0fc773a03e4b) )
	ROM_LOAD( "a71-20.ic34",  0x60000, 0x8000, CRC(65acb265) SHA1(2ef940f994e76d4387be6e0d53a565813cc59636) )
	ROM_LOAD( "a71-21.ic35",  0x68000, 0x8000, CRC(33cde9b2) SHA1(9b227ab609e3c7c6be90c29739a57ea4959cd68e) )
	ROM_LOAD( "a71-22.ic36",  0x70000, 0x8000, CRC(fb98eac0) SHA1(57615c3934de5510eeeb0ba16024abda8ee95303) )
	ROM_LOAD( "a71-23.ic37",  0x78000, 0x8000, CRC(30bd46ad) SHA1(6e1618ed237c769d1a8d329fbd7a9f7216993215) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "a71-25.ic41",  0x0000, 0x0100, CRC(2d0f8545) SHA1(089c31e2f614145ef2743164f7b52ae35bc06808) )	/* video timing, on gfx board */
ROM_END


/*

bublbobl.zip - a78-05-1.52
TAITO CORPORATION 1986
ALL RIGHTS RESERVED
VER 0.1 4.SEP,1986 SUMMER

Name          Size    CRC32       Chip Type
-------------------------------------------
a78-05-1.52    65536  0x9f8ee242  Fujitsu MBM27C512
a78-06-1.51    32768  0x567934b6  Intel D27256

*/

ROM_START( bublbobl )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "a78-06-1.51",    0x00000, 0x08000, CRC(567934b6) SHA1(b0c4d49fd551f465d148c25c3e80b278835e2f0d) )
	/* ROMs banked at 8000-bfff */
	ROM_LOAD( "a78-05-1.52",    0x10000, 0x10000, CRC(9f8ee242) SHA1(924150d4e7e087a9b2b0a294c2d0e9903a266c6c) )
	/* 20000-2ffff empty */

	ROM_REGION( 0x10000, "slave", 0 )	/* 64k for the second CPU */
	ROM_LOAD( "a78-08.37",    0x0000, 0x08000, CRC(ae11a07b) SHA1(af7a335c8da637103103cc274e077f123908ebb7) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* 64k for the third CPU */
	ROM_LOAD( "a78-07.46",    0x0000, 0x08000, CRC(4f9a26e8) SHA1(3105b34b88a7134493c2b3f584729f8b0407a011) )

	ROM_REGION( 0x10000, "mcu", 0 )	/* 64k for the MCU */
	ROM_LOAD( "a78-01.17",    0xf000, 0x1000, CRC(b1bfb53d) SHA1(31b8f31acd3aa394acd80db362774749842e1285) )

	ROM_REGION( 0x80000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "a78-09.12",    0x00000, 0x8000, CRC(20358c22) SHA1(2297af6c53d5807bf90a8e081075b8c72a994fc5) )    /* 1st plane */
	ROM_LOAD( "a78-10.13",    0x08000, 0x8000, CRC(930168a9) SHA1(fd358c3c3b424bca285f67a1589eb98a345ff670) )
	ROM_LOAD( "a78-11.14",    0x10000, 0x8000, CRC(9773e512) SHA1(33c1687ee575d66bf0e98add45d06da827813765) )
	ROM_LOAD( "a78-12.15",    0x18000, 0x8000, CRC(d045549b) SHA1(0c12077d3ddc2ce6aa45a0224ad5540f3f218446) )
	ROM_LOAD( "a78-13.16",    0x20000, 0x8000, CRC(d0af35c5) SHA1(c5a89f4d73acc0db86654540b3abfd77b3757db5) )
	ROM_LOAD( "a78-14.17",    0x28000, 0x8000, CRC(7b5369a8) SHA1(1307b26d80e6f36ebe6c442bebec41d20066eaf9) )
	/* 0x30000-0x3ffff empty */
	ROM_LOAD( "a78-15.30",    0x40000, 0x8000, CRC(6b61a413) SHA1(44eddf12fb46fceca2addbe6da929aaea7636b13) )    /* 2nd plane */
	ROM_LOAD( "a78-16.31",    0x48000, 0x8000, CRC(b5492d97) SHA1(d5b045e3ebaa44809757a4220cefb3c6815470da) )
	ROM_LOAD( "a78-17.32",    0x50000, 0x8000, CRC(d69762d5) SHA1(3326fef4e0bd86681a3047dc11886bb171ecb609) )
	ROM_LOAD( "a78-18.33",    0x58000, 0x8000, CRC(9f243b68) SHA1(32dce8d311a4be003693182a999e4053baa6bb0a) )
	ROM_LOAD( "a78-19.34",    0x60000, 0x8000, CRC(66e9438c) SHA1(b94e62b6fbe7f4e08086d0365afc5cff6e0ccafd) )
	ROM_LOAD( "a78-20.35",    0x68000, 0x8000, CRC(9ef863ad) SHA1(29f91b5a3765e4d6e6c3382db1d8d8297b6e56c8) )
	/* 0x70000-0x7ffff empty */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "a71-25.41",    0x0000, 0x0100, CRC(2d0f8545) SHA1(089c31e2f614145ef2743164f7b52ae35bc06808) )	/* video timing */

	/* Located on CPU/Sound Board */
	ROM_REGION( 0x0003, "plds", 0 )
	ROM_LOAD( "pal16l8.bin",  0x0000, 0x0001, NO_DUMP ) /* Located at IC49 */
	ROM_LOAD( "pal16l8.bin",  0x0000, 0x0001, NO_DUMP ) /* Located at IC43 */
	ROM_LOAD( "pal16r4.bin",  0x0000, 0x0001, NO_DUMP ) /* Located at IC12 */
ROM_END

/*
bublbob1.zip - a78-05.52
TAITO CORPORATION 1986
ALL RIGHTS RESERVED
VER 0.018.AUG,1986 SUMMER
*/

ROM_START( bublbobl1 )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "a78-06.51",    0x00000, 0x08000, CRC(32c8305b) SHA1(6bf69b3edfbefd33cd670a762b4bf0b39629a220) )
	/* ROMs banked at 8000-bfff */
	ROM_LOAD( "a78-05.52",    0x10000, 0x10000, CRC(53f4bc6e) SHA1(15a2e6d83438d4136b154b3d90dd2cf9f1ce572c) )
	/* 20000-2ffff empty */

	ROM_REGION( 0x10000, "slave", 0 )	/* 64k for the second CPU */
	ROM_LOAD( "a78-08.37",    0x0000, 0x08000, CRC(ae11a07b) SHA1(af7a335c8da637103103cc274e077f123908ebb7) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* 64k for the third CPU */
	ROM_LOAD( "a78-07.46",    0x0000, 0x08000, CRC(4f9a26e8) SHA1(3105b34b88a7134493c2b3f584729f8b0407a011) )

	ROM_REGION( 0x10000, "mcu", 0 )	/* 64k for the MCU */
	ROM_LOAD( "a78-01.17",    0xf000, 0x1000, CRC(b1bfb53d) SHA1(31b8f31acd3aa394acd80db362774749842e1285) )

	ROM_REGION( 0x80000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "a78-09.12",    0x00000, 0x8000, CRC(20358c22) SHA1(2297af6c53d5807bf90a8e081075b8c72a994fc5) )    /* 1st plane */
	ROM_LOAD( "a78-10.13",    0x08000, 0x8000, CRC(930168a9) SHA1(fd358c3c3b424bca285f67a1589eb98a345ff670) )
	ROM_LOAD( "a78-11.14",    0x10000, 0x8000, CRC(9773e512) SHA1(33c1687ee575d66bf0e98add45d06da827813765) )
	ROM_LOAD( "a78-12.15",    0x18000, 0x8000, CRC(d045549b) SHA1(0c12077d3ddc2ce6aa45a0224ad5540f3f218446) )
	ROM_LOAD( "a78-13.16",    0x20000, 0x8000, CRC(d0af35c5) SHA1(c5a89f4d73acc0db86654540b3abfd77b3757db5) )
	ROM_LOAD( "a78-14.17",    0x28000, 0x8000, CRC(7b5369a8) SHA1(1307b26d80e6f36ebe6c442bebec41d20066eaf9) )
	/* 0x30000-0x3ffff empty */
	ROM_LOAD( "a78-15.30",    0x40000, 0x8000, CRC(6b61a413) SHA1(44eddf12fb46fceca2addbe6da929aaea7636b13) )    /* 2nd plane */
	ROM_LOAD( "a78-16.31",    0x48000, 0x8000, CRC(b5492d97) SHA1(d5b045e3ebaa44809757a4220cefb3c6815470da) )
	ROM_LOAD( "a78-17.32",    0x50000, 0x8000, CRC(d69762d5) SHA1(3326fef4e0bd86681a3047dc11886bb171ecb609) )
	ROM_LOAD( "a78-18.33",    0x58000, 0x8000, CRC(9f243b68) SHA1(32dce8d311a4be003693182a999e4053baa6bb0a) )
	ROM_LOAD( "a78-19.34",    0x60000, 0x8000, CRC(66e9438c) SHA1(b94e62b6fbe7f4e08086d0365afc5cff6e0ccafd) )
	ROM_LOAD( "a78-20.35",    0x68000, 0x8000, CRC(9ef863ad) SHA1(29f91b5a3765e4d6e6c3382db1d8d8297b6e56c8) )
	/* 0x70000-0x7ffff empty */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "a71-25.41",    0x0000, 0x0100, CRC(2d0f8545) SHA1(089c31e2f614145ef2743164f7b52ae35bc06808) )	/* video timing */

	/* Located on CPU/Sound Board */
	ROM_REGION( 0x0003, "plds", 0 )
	ROM_LOAD( "pal16l8.bin",  0x0000, 0x0001, NO_DUMP ) /* Located at IC49 */
	ROM_LOAD( "pal16l8.bin",  0x0000, 0x0001, NO_DUMP ) /* Located at IC43 */
	ROM_LOAD( "pal16r4.bin",  0x0000, 0x0001, NO_DUMP ) /* Located at IC12 */
ROM_END

/*
bublbobr.zip - a78-24.52
1986 TAITO AMERICA CORP.
LICENSED TO ROMSTAR FOR U.S.A.
VER 5.1 8.NOV,1986 SUMMER
*/

ROM_START( bublboblr )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "a78-25.51",    0x00000, 0x08000, CRC(2d901c9d) SHA1(72504225d3a26212e8f35508a79200eeb91138b6) )
	/* ROMs banked at 8000-bfff */
	ROM_LOAD( "a78-24.52",    0x10000, 0x10000, CRC(b7afedc4) SHA1(6e4c8712f1fdf000e231cfd622dd3b514c61a6fd) )
	/* 20000-2ffff empty */

	ROM_REGION( 0x10000, "slave", 0 )	/* 64k for the second CPU */
	ROM_LOAD( "a78-08.37",    0x0000, 0x08000, CRC(ae11a07b) SHA1(af7a335c8da637103103cc274e077f123908ebb7) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* 64k for the third CPU */
	ROM_LOAD( "a78-07.46",    0x0000, 0x08000, CRC(4f9a26e8) SHA1(3105b34b88a7134493c2b3f584729f8b0407a011) )

	ROM_REGION( 0x10000, "mcu", 0 )	/* 64k for the MCU */
	ROM_LOAD( "a78-01.17",    0xf000, 0x1000, CRC(b1bfb53d) SHA1(31b8f31acd3aa394acd80db362774749842e1285) )

	ROM_REGION( 0x80000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "a78-09.12",    0x00000, 0x8000, CRC(20358c22) SHA1(2297af6c53d5807bf90a8e081075b8c72a994fc5) )    /* 1st plane */
	ROM_LOAD( "a78-10.13",    0x08000, 0x8000, CRC(930168a9) SHA1(fd358c3c3b424bca285f67a1589eb98a345ff670) )
	ROM_LOAD( "a78-11.14",    0x10000, 0x8000, CRC(9773e512) SHA1(33c1687ee575d66bf0e98add45d06da827813765) )
	ROM_LOAD( "a78-12.15",    0x18000, 0x8000, CRC(d045549b) SHA1(0c12077d3ddc2ce6aa45a0224ad5540f3f218446) )
	ROM_LOAD( "a78-13.16",    0x20000, 0x8000, CRC(d0af35c5) SHA1(c5a89f4d73acc0db86654540b3abfd77b3757db5) )
	ROM_LOAD( "a78-14.17",    0x28000, 0x8000, CRC(7b5369a8) SHA1(1307b26d80e6f36ebe6c442bebec41d20066eaf9) )
	/* 0x30000-0x3ffff empty */
	ROM_LOAD( "a78-15.30",    0x40000, 0x8000, CRC(6b61a413) SHA1(44eddf12fb46fceca2addbe6da929aaea7636b13) )    /* 2nd plane */
	ROM_LOAD( "a78-16.31",    0x48000, 0x8000, CRC(b5492d97) SHA1(d5b045e3ebaa44809757a4220cefb3c6815470da) )
	ROM_LOAD( "a78-17.32",    0x50000, 0x8000, CRC(d69762d5) SHA1(3326fef4e0bd86681a3047dc11886bb171ecb609) )
	ROM_LOAD( "a78-18.33",    0x58000, 0x8000, CRC(9f243b68) SHA1(32dce8d311a4be003693182a999e4053baa6bb0a) )
	ROM_LOAD( "a78-19.34",    0x60000, 0x8000, CRC(66e9438c) SHA1(b94e62b6fbe7f4e08086d0365afc5cff6e0ccafd) )
	ROM_LOAD( "a78-20.35",    0x68000, 0x8000, CRC(9ef863ad) SHA1(29f91b5a3765e4d6e6c3382db1d8d8297b6e56c8) )
	/* 0x70000-0x7ffff empty */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "a71-25.41",    0x0000, 0x0100, CRC(2d0f8545) SHA1(089c31e2f614145ef2743164f7b52ae35bc06808) )	/* video timing */

	/* Located on CPU/Sound Board */
	ROM_REGION( 0x0003, "plds", 0 )
	ROM_LOAD( "pal16l8.bin",  0x0000, 0x0001, NO_DUMP ) /* Located at IC49 */
	ROM_LOAD( "pal16l8.bin",  0x0000, 0x0001, NO_DUMP ) /* Located at IC43 */
	ROM_LOAD( "pal16r4.bin",  0x0000, 0x0001, NO_DUMP ) /* Located at IC12 */
ROM_END

/*
bubbobr1.zip - a78-21.52
1986 TAITO AMERICA CORP.
LICENSED TO ROMSTAR FOR U.S.A.
VER 1.0 26.AUG,1986 SUMMER
*/

ROM_START( bublboblr1 )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "a78-06.51",    0x00000, 0x08000, CRC(32c8305b) SHA1(6bf69b3edfbefd33cd670a762b4bf0b39629a220) )
	/* ROMs banked at 8000-bfff */
	ROM_LOAD( "a78-21.52",    0x10000, 0x10000, CRC(2844033d) SHA1(6ac0b09d0325990cf18935f35b0adbc033758947) )
	/* 20000-2ffff empty */

	ROM_REGION( 0x10000, "slave", 0 )	/* 64k for the second CPU */
	ROM_LOAD( "a78-08.37",    0x0000, 0x08000, CRC(ae11a07b) SHA1(af7a335c8da637103103cc274e077f123908ebb7) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* 64k for the third CPU */
	ROM_LOAD( "a78-07.46",    0x0000, 0x08000, CRC(4f9a26e8) SHA1(3105b34b88a7134493c2b3f584729f8b0407a011) )

	ROM_REGION( 0x10000, "mcu", 0 )	/* 64k for the MCU */
	ROM_LOAD( "a78-01.17",    0xf000, 0x1000, CRC(b1bfb53d) SHA1(31b8f31acd3aa394acd80db362774749842e1285) )

	ROM_REGION( 0x80000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "a78-09.12",    0x00000, 0x8000, CRC(20358c22) SHA1(2297af6c53d5807bf90a8e081075b8c72a994fc5) )    /* 1st plane */
	ROM_LOAD( "a78-10.13",    0x08000, 0x8000, CRC(930168a9) SHA1(fd358c3c3b424bca285f67a1589eb98a345ff670) )
	ROM_LOAD( "a78-11.14",    0x10000, 0x8000, CRC(9773e512) SHA1(33c1687ee575d66bf0e98add45d06da827813765) )
	ROM_LOAD( "a78-12.15",    0x18000, 0x8000, CRC(d045549b) SHA1(0c12077d3ddc2ce6aa45a0224ad5540f3f218446) )
	ROM_LOAD( "a78-13.16",    0x20000, 0x8000, CRC(d0af35c5) SHA1(c5a89f4d73acc0db86654540b3abfd77b3757db5) )
	ROM_LOAD( "a78-14.17",    0x28000, 0x8000, CRC(7b5369a8) SHA1(1307b26d80e6f36ebe6c442bebec41d20066eaf9) )
	/* 0x30000-0x3ffff empty */
	ROM_LOAD( "a78-15.30",    0x40000, 0x8000, CRC(6b61a413) SHA1(44eddf12fb46fceca2addbe6da929aaea7636b13) )    /* 2nd plane */
	ROM_LOAD( "a78-16.31",    0x48000, 0x8000, CRC(b5492d97) SHA1(d5b045e3ebaa44809757a4220cefb3c6815470da) )
	ROM_LOAD( "a78-17.32",    0x50000, 0x8000, CRC(d69762d5) SHA1(3326fef4e0bd86681a3047dc11886bb171ecb609) )
	ROM_LOAD( "a78-18.33",    0x58000, 0x8000, CRC(9f243b68) SHA1(32dce8d311a4be003693182a999e4053baa6bb0a) )
	ROM_LOAD( "a78-19.34",    0x60000, 0x8000, CRC(66e9438c) SHA1(b94e62b6fbe7f4e08086d0365afc5cff6e0ccafd) )
	ROM_LOAD( "a78-20.35",    0x68000, 0x8000, CRC(9ef863ad) SHA1(29f91b5a3765e4d6e6c3382db1d8d8297b6e56c8) )
	/* 0x70000-0x7ffff empty */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "a71-25.41",    0x0000, 0x0100, CRC(2d0f8545) SHA1(089c31e2f614145ef2743164f7b52ae35bc06808) )	/* video timing */

	/* Located on CPU/Sound Board */
	ROM_REGION( 0x0003, "plds", 0 )
	ROM_LOAD( "pal16l8.bin",  0x0000, 0x0001, NO_DUMP ) /* Located at IC49 */
	ROM_LOAD( "pal16l8.bin",  0x0000, 0x0001, NO_DUMP ) /* Located at IC43 */
	ROM_LOAD( "pal16r4.bin",  0x0000, 0x0001, NO_DUMP ) /* Located at IC12 */
ROM_END

ROM_START( boblbobl )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "bb3",          0x00000, 0x08000, CRC(01f81936) SHA1(a48489a13bfd01949e7fd273029d9cb8bfd7be48) )
	/* ROMs banked at 8000-bfff */
	ROM_LOAD( "bb5",          0x10000, 0x08000, CRC(13118eb1) SHA1(5a5da40c2cc82420f70bc58ffa32de1088c6c82f) )
	ROM_LOAD( "bb4",          0x18000, 0x08000, CRC(afda99d8) SHA1(304324074ae726501bbb08e683850639d69939fb) )
	/* 20000-2ffff empty */

	ROM_REGION( 0x10000, "slave", 0 )	/* 64k for the second CPU */
	ROM_LOAD( "a78-08.37",    0x0000, 0x08000, CRC(ae11a07b) SHA1(af7a335c8da637103103cc274e077f123908ebb7) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* 64k for the third CPU */
	ROM_LOAD( "a78-07.46",    0x0000, 0x08000, CRC(4f9a26e8) SHA1(3105b34b88a7134493c2b3f584729f8b0407a011) )

	ROM_REGION( 0x80000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "a78-09.12",    0x00000, 0x8000, CRC(20358c22) SHA1(2297af6c53d5807bf90a8e081075b8c72a994fc5) )    /* 1st plane */
	ROM_LOAD( "a78-10.13",    0x08000, 0x8000, CRC(930168a9) SHA1(fd358c3c3b424bca285f67a1589eb98a345ff670) )
	ROM_LOAD( "a78-11.14",    0x10000, 0x8000, CRC(9773e512) SHA1(33c1687ee575d66bf0e98add45d06da827813765) )
	ROM_LOAD( "a78-12.15",    0x18000, 0x8000, CRC(d045549b) SHA1(0c12077d3ddc2ce6aa45a0224ad5540f3f218446) )
	ROM_LOAD( "a78-13.16",    0x20000, 0x8000, CRC(d0af35c5) SHA1(c5a89f4d73acc0db86654540b3abfd77b3757db5) )
	ROM_LOAD( "a78-14.17",    0x28000, 0x8000, CRC(7b5369a8) SHA1(1307b26d80e6f36ebe6c442bebec41d20066eaf9) )
	/* 0x30000-0x3ffff empty */
	ROM_LOAD( "a78-15.30",    0x40000, 0x8000, CRC(6b61a413) SHA1(44eddf12fb46fceca2addbe6da929aaea7636b13) )    /* 2nd plane */
	ROM_LOAD( "a78-16.31",    0x48000, 0x8000, CRC(b5492d97) SHA1(d5b045e3ebaa44809757a4220cefb3c6815470da) )
	ROM_LOAD( "a78-17.32",    0x50000, 0x8000, CRC(d69762d5) SHA1(3326fef4e0bd86681a3047dc11886bb171ecb609) )
	ROM_LOAD( "a78-18.33",    0x58000, 0x8000, CRC(9f243b68) SHA1(32dce8d311a4be003693182a999e4053baa6bb0a) )
	ROM_LOAD( "a78-19.34",    0x60000, 0x8000, CRC(66e9438c) SHA1(b94e62b6fbe7f4e08086d0365afc5cff6e0ccafd) )
	ROM_LOAD( "a78-20.35",    0x68000, 0x8000, CRC(9ef863ad) SHA1(29f91b5a3765e4d6e6c3382db1d8d8297b6e56c8) )
	/* 0x70000-0x7ffff empty */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "a71-25.41",    0x0000, 0x0100, CRC(2d0f8545) SHA1(089c31e2f614145ef2743164f7b52ae35bc06808) )	/* video timing */

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "pal16r4.u36", 0x0000, 0x0104, CRC(22fe26ac) SHA1(bbbfcbe6faded4af7ceec57b800297c054a997da) )
	ROM_LOAD( "pal16l8.u38", 0x0200, 0x0104, CRC(c02d9663) SHA1(5d23cfd96f072981fd5fcf0dd7e98459da58b662) )
	ROM_LOAD( "pal16l8.u4",  0x0400, 0x0104, CRC(077d20a8) SHA1(8e568ffd6f66c3dd61708dd0f3be9c2ed488ae4b) )
ROM_END


ROM_START( sboblboa )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "1c.bin",    0x00000, 0x08000, CRC(f304152a) SHA1(103d9beddccef289ed739d28ebda69bbad3d42f9) )
	/* ROMs banked at 8000-bfff */
	ROM_LOAD( "1a.bin",          0x10000, 0x08000, CRC(0865209c) SHA1(eddd3547ae675b73376e5c284e0b15830f4a6645) )
	ROM_LOAD( "1b.bin",          0x18000, 0x08000, CRC(1f29b5c0) SHA1(c15c84ca11cc10edac6340468bca463ecb2d89e6) )
	/* 20000-2ffff empty */

	ROM_REGION( 0x10000, "slave", 0 )	/* 64k for the second CPU */
	ROM_LOAD( "1e.rom",    0x0000, 0x08000, CRC(ae11a07b) SHA1(af7a335c8da637103103cc274e077f123908ebb7) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* 64k for the third CPU */
	ROM_LOAD( "1d.rom",    0x0000, 0x08000, CRC(4f9a26e8) SHA1(3105b34b88a7134493c2b3f584729f8b0407a011) )

	ROM_REGION( 0x80000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "1l.rom",    0x00000, 0x8000, CRC(20358c22) SHA1(2297af6c53d5807bf90a8e081075b8c72a994fc5) )    /* 1st plane */
	ROM_LOAD( "1m.rom",    0x08000, 0x8000, CRC(930168a9) SHA1(fd358c3c3b424bca285f67a1589eb98a345ff670) )
	ROM_LOAD( "1n.rom",    0x10000, 0x8000, CRC(9773e512) SHA1(33c1687ee575d66bf0e98add45d06da827813765) )
	ROM_LOAD( "1o.rom",    0x18000, 0x8000, CRC(d045549b) SHA1(0c12077d3ddc2ce6aa45a0224ad5540f3f218446) )
	ROM_LOAD( "1p.rom",    0x20000, 0x8000, CRC(d0af35c5) SHA1(c5a89f4d73acc0db86654540b3abfd77b3757db5) )
	ROM_LOAD( "1q.rom",    0x28000, 0x8000, CRC(7b5369a8) SHA1(1307b26d80e6f36ebe6c442bebec41d20066eaf9) )
	/* 0x30000-0x3ffff empty */
	ROM_LOAD( "1f.rom",    0x40000, 0x8000, CRC(6b61a413) SHA1(44eddf12fb46fceca2addbe6da929aaea7636b13) )    /* 2nd plane */
	ROM_LOAD( "1g.rom",    0x48000, 0x8000, CRC(b5492d97) SHA1(d5b045e3ebaa44809757a4220cefb3c6815470da) )
	ROM_LOAD( "1h.rom",    0x50000, 0x8000, CRC(d69762d5) SHA1(3326fef4e0bd86681a3047dc11886bb171ecb609) )
	ROM_LOAD( "1i.rom",    0x58000, 0x8000, CRC(9f243b68) SHA1(32dce8d311a4be003693182a999e4053baa6bb0a) )
	ROM_LOAD( "1j.rom",    0x60000, 0x8000, CRC(66e9438c) SHA1(b94e62b6fbe7f4e08086d0365afc5cff6e0ccafd) )
	ROM_LOAD( "1k.rom",    0x68000, 0x8000, CRC(9ef863ad) SHA1(29f91b5a3765e4d6e6c3382db1d8d8297b6e56c8) )
	/* 0x70000-0x7ffff empty */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "a71-25.41",    0x0000, 0x0100, CRC(2d0f8545) SHA1(089c31e2f614145ef2743164f7b52ae35bc06808) )	/* video timing */
ROM_END

ROM_START( sboblbob )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "bbb-3.rom",    0x00000, 0x08000, CRC(f304152a) SHA1(103d9beddccef289ed739d28ebda69bbad3d42f9) )
	/* ROMs banked at 8000-bfff */
	ROM_LOAD( "bb5",          0x10000, 0x08000, CRC(13118eb1) SHA1(5a5da40c2cc82420f70bc58ffa32de1088c6c82f) )
	ROM_LOAD( "bbb-4.rom",    0x18000, 0x08000, CRC(94c75591) SHA1(7698bc4b7d20e554a73a489cd3a15ae61b350e37) )
	/* 20000-2ffff empty */

	ROM_REGION( 0x10000, "slave", 0 )	/* 64k for the second CPU */
	ROM_LOAD( "a78-08.37",    0x0000, 0x08000, CRC(ae11a07b) SHA1(af7a335c8da637103103cc274e077f123908ebb7) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* 64k for the third CPU */
	ROM_LOAD( "a78-07.46",    0x0000, 0x08000, CRC(4f9a26e8) SHA1(3105b34b88a7134493c2b3f584729f8b0407a011) )

	ROM_REGION( 0x80000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "a78-09.12",    0x00000, 0x8000, CRC(20358c22) SHA1(2297af6c53d5807bf90a8e081075b8c72a994fc5) )    /* 1st plane */
	ROM_LOAD( "a78-10.13",    0x08000, 0x8000, CRC(930168a9) SHA1(fd358c3c3b424bca285f67a1589eb98a345ff670) )
	ROM_LOAD( "a78-11.14",    0x10000, 0x8000, CRC(9773e512) SHA1(33c1687ee575d66bf0e98add45d06da827813765) )
	ROM_LOAD( "a78-12.15",    0x18000, 0x8000, CRC(d045549b) SHA1(0c12077d3ddc2ce6aa45a0224ad5540f3f218446) )
	ROM_LOAD( "a78-13.16",    0x20000, 0x8000, CRC(d0af35c5) SHA1(c5a89f4d73acc0db86654540b3abfd77b3757db5) )
	ROM_LOAD( "a78-14.17",    0x28000, 0x8000, CRC(7b5369a8) SHA1(1307b26d80e6f36ebe6c442bebec41d20066eaf9) )
	/* 0x30000-0x3ffff empty */
	ROM_LOAD( "a78-15.30",    0x40000, 0x8000, CRC(6b61a413) SHA1(44eddf12fb46fceca2addbe6da929aaea7636b13) )    /* 2nd plane */
	ROM_LOAD( "a78-16.31",    0x48000, 0x8000, CRC(b5492d97) SHA1(d5b045e3ebaa44809757a4220cefb3c6815470da) )
	ROM_LOAD( "a78-17.32",    0x50000, 0x8000, CRC(d69762d5) SHA1(3326fef4e0bd86681a3047dc11886bb171ecb609) )
	ROM_LOAD( "a78-18.33",    0x58000, 0x8000, CRC(9f243b68) SHA1(32dce8d311a4be003693182a999e4053baa6bb0a) )
	ROM_LOAD( "a78-19.34",    0x60000, 0x8000, CRC(66e9438c) SHA1(b94e62b6fbe7f4e08086d0365afc5cff6e0ccafd) )
	ROM_LOAD( "a78-20.35",    0x68000, 0x8000, CRC(9ef863ad) SHA1(29f91b5a3765e4d6e6c3382db1d8d8297b6e56c8) )
	/* 0x70000-0x7ffff empty */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "a71-25.41",    0x0000, 0x0100, CRC(2d0f8545) SHA1(089c31e2f614145ef2743164f7b52ae35bc06808) )	/* video timing */
ROM_END



ROM_START( bub68705 )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* Program roms match Bubble Bobble (older) */
	ROM_LOAD( "2.bin",    0x00000, 0x08000, CRC(32c8305b) SHA1(6bf69b3edfbefd33cd670a762b4bf0b39629a220) )
	/* ROMs banked at 8000-bfff */
	ROM_LOAD( "3-1.bin",        0x10000, 0x08000, CRC(980c2615) SHA1(3670cf3e4e73028aadf4460ad887a0b544bcdbc4) )
	ROM_LOAD( "3.bin",          0x18000, 0x08000, CRC(e6c698f2) SHA1(8df116075f5891f74d0da8966ed11c597b5f544f) )
	/* 20000-2ffff empty */

	ROM_REGION( 0x10000, "slave", 0 )	/* 64k for the second CPU */
	ROM_LOAD( "4.bin",    0x0000, 0x08000, CRC(ae11a07b) SHA1(af7a335c8da637103103cc274e077f123908ebb7) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* 64k for the third CPU */
	ROM_LOAD( "1.bin",    0x0000, 0x08000, CRC(4f9a26e8) SHA1(3105b34b88a7134493c2b3f584729f8b0407a011) )

	ROM_REGION( 0x800, "mcu", 0 )	/* 64k for the MCU */
	ROM_LOAD( "68705.bin",    0x000, 0x800, CRC(78caa635) SHA1(a756e45b25b007843ba4f2204cad6081cf7260e9) )

	ROM_REGION( 0x80000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "a78-09.12",    0x00000, 0x8000, CRC(20358c22) SHA1(2297af6c53d5807bf90a8e081075b8c72a994fc5) )    /* 1st plane */
	ROM_LOAD( "a78-10.13",    0x08000, 0x8000, CRC(930168a9) SHA1(fd358c3c3b424bca285f67a1589eb98a345ff670) )
	ROM_LOAD( "a78-11.14",    0x10000, 0x8000, CRC(9773e512) SHA1(33c1687ee575d66bf0e98add45d06da827813765) )
	ROM_LOAD( "a78-12.15",    0x18000, 0x8000, CRC(d045549b) SHA1(0c12077d3ddc2ce6aa45a0224ad5540f3f218446) )
	ROM_LOAD( "a78-13.16",    0x20000, 0x8000, CRC(d0af35c5) SHA1(c5a89f4d73acc0db86654540b3abfd77b3757db5) )
	ROM_LOAD( "a78-14.17",    0x28000, 0x8000, CRC(7b5369a8) SHA1(1307b26d80e6f36ebe6c442bebec41d20066eaf9) )
	/* 0x30000-0x3ffff empty */
	ROM_LOAD( "a78-15.30",    0x40000, 0x8000, CRC(6b61a413) SHA1(44eddf12fb46fceca2addbe6da929aaea7636b13) )    /* 2nd plane */
	ROM_LOAD( "a78-16.31",    0x48000, 0x8000, CRC(b5492d97) SHA1(d5b045e3ebaa44809757a4220cefb3c6815470da) )
	ROM_LOAD( "a78-17.32",    0x50000, 0x8000, CRC(d69762d5) SHA1(3326fef4e0bd86681a3047dc11886bb171ecb609) )
	ROM_LOAD( "a78-18.33",    0x58000, 0x8000, CRC(9f243b68) SHA1(32dce8d311a4be003693182a999e4053baa6bb0a) )
	ROM_LOAD( "a78-19.34",    0x60000, 0x8000, CRC(66e9438c) SHA1(b94e62b6fbe7f4e08086d0365afc5cff6e0ccafd) )
	ROM_LOAD( "a78-20.35",    0x68000, 0x8000, CRC(9ef863ad) SHA1(29f91b5a3765e4d6e6c3382db1d8d8297b6e56c8) )
	/* 0x70000-0x7ffff empty */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "a71-25.41",    0x0000, 0x0100, CRC(2d0f8545) SHA1(089c31e2f614145ef2743164f7b52ae35bc06808) )	/* video timing */
ROM_END


ROM_START( dland )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "dl_3.u69",    0x00000, 0x08000, CRC(01eb3e4f) SHA1(8edc0f2e98b928b1d9d508948bcff947df697b6f) )
	/* ROMs banked at 8000-bfff */
	ROM_LOAD( "dl_5.u67",    0x10000, 0x08000, CRC(75740b61) SHA1(7ebfbb9abfcc44c31b31f146d7bc37004a1b528c) )
	ROM_LOAD( "dl_4.u68",    0x18000, 0x08000, CRC(c6a3776f) SHA1(473fc8c990046f90517f2506f1ca59eeb7ea13e5) )
	/* 20000-2ffff empty */

	ROM_REGION( 0x10000, "slave", 0 )	/* 64k for the second CPU */
	ROM_LOAD( "dl_1.u42",    0x0000, 0x08000, CRC(ae11a07b) SHA1(af7a335c8da637103103cc274e077f123908ebb7) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* 64k for the third CPU */
	ROM_LOAD( "dl_2.u74",    0x0000, 0x08000, CRC(4f9a26e8) SHA1(3105b34b88a7134493c2b3f584729f8b0407a011) )

	ROM_REGION( 0x80000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "dl_6.58",     0x00000, 0x10000, CRC(6352d3fa) SHA1(eacaddd476952f3c048138184e712ca1fbba4ce2) )
	ROM_LOAD( "dl_7.59",     0x10000, 0x10000, CRC(37a38b69) SHA1(3d28fbf1725b35f664836ee45f705c05f6ccd78a) )
	ROM_LOAD( "dl_8.60",     0x20000, 0x10000, CRC(509ee5b1) SHA1(b5edc7346d43db0157deadece60e478ba6d63eab) )
	/* 0x30000-0x3ffff empty */
	ROM_LOAD( "dl_9.61",     0x40000, 0x10000, CRC(ae8514d7) SHA1(5205a3faf354f5b5616be4494f5bd553de4c7965) )
	ROM_LOAD( "dl_10.62",    0x50000, 0x10000, CRC(6d406fb7) SHA1(26d9236d259f8b3876087797b77994b299eeea63) )
	ROM_LOAD( "dl_11.63",    0x60000, 0x10000, CRC(bdf9c0ab) SHA1(d5afc5205e8e391a4a095a4e2efbeee96e780638) )
	/* 0x70000-0x7ffff empty */

	ROM_REGION( 0x0100, "proms", 0 ) // not on this? (but needed for the bublbobl video driver to work)
	ROM_LOAD( "a71-25.41",    0x0000, 0x0100, CRC(2d0f8545) SHA1(089c31e2f614145ef2743164f7b52ae35bc06808) )	/* video timing */
ROM_END



/*************************************
 *
 *  Driver initialization
 *
 *************************************/

static void configure_banks( running_machine* machine )
{
	UINT8 *ROM = memory_region(machine, "maincpu");
	memory_configure_bank(machine, "bank1", 0, 8, &ROM[0x10000], 0x4000);
}

static DRIVER_INIT( bublbobl )
{
	bublbobl_state *state = (bublbobl_state *)machine->driver_data;

	configure_banks(machine);

	/* we init this here, so that it does not conflict with tokio init, below */
	state->video_enable = 0;
}

static DRIVER_INIT( tokio )
{
	bublbobl_state *state = (bublbobl_state *)machine->driver_data;
	configure_banks(machine);

	/* preemptively enable video, the bit is not mapped for this game and */
	/* I don't know if it even has it. */
	state->video_enable = 1;
}

static DRIVER_INIT( tokiob )
{
	DRIVER_INIT_CALL(tokio);

	memory_install_read8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xfe00, 0xfe00, 0, 0, tokiob_mcu_r );
}

static DRIVER_INIT( dland )
{
	// rearrange gfx to original format
	int i;
	UINT8* src = memory_region(machine,"gfx1");
	for (i = 0; i < 0x40000; i++)
		src[i] = BITSWAP8(src[i],7,6,5,4,0,1,2,3);

	for (i = 0x40000; i < 0x80000; i++)
		src[i] = BITSWAP8(src[i],7,4,5,6,3,0,1,2);

	DRIVER_INIT_CALL(bublbobl);
}


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1986, tokio,      0,        tokio,    tokio,    tokio,    ROT90, "Taito Corporation", "Tokio / Scramble Formation (newer)", GAME_NOT_WORKING | GAME_UNEMULATED_PROTECTION | GAME_SUPPORTS_SAVE )
GAME( 1986, tokioo,     tokio,        tokio,    tokio,    tokio,    ROT90, "Taito Corporation", "Tokio / Scramble Formation (older)", GAME_NOT_WORKING | GAME_UNEMULATED_PROTECTION | GAME_SUPPORTS_SAVE )
GAME( 1986, tokiou,     tokio,    tokio,    tokio,    tokio,    ROT90, "Taito America Corporation (Romstar license)", "Tokio / Scramble Formation (US)", GAME_NOT_WORKING | GAME_UNEMULATED_PROTECTION | GAME_SUPPORTS_SAVE )
GAME( 1986, tokiob,     tokio,    tokio,    tokio,    tokiob,   ROT90, "bootleg", "Tokio / Scramble Formation (bootleg)", GAME_SUPPORTS_SAVE )

GAME( 1986, bublbobl,   0,        bublbobl, bublbobl, bublbobl, ROT0,  "Taito Corporation", "Bubble Bobble", GAME_SUPPORTS_SAVE )
GAME( 1986, bublbobl1,  bublbobl, bublbobl, bublbobl, bublbobl, ROT0,  "Taito Corporation", "Bubble Bobble (older)", GAME_SUPPORTS_SAVE )
GAME( 1986, bublboblr,  bublbobl, bublbobl, bublbobl, bublbobl, ROT0,  "Taito America Corporation (Romstar license)", "Bubble Bobble (US with mode select)", GAME_SUPPORTS_SAVE )
GAME( 1986, bublboblr1, bublbobl, bublbobl, bublbobl, bublbobl, ROT0,  "Taito America Corporation (Romstar license)", "Bubble Bobble (US)", GAME_SUPPORTS_SAVE )

GAME( 1986, boblbobl,   bublbobl, boblbobl, boblbobl, bublbobl, ROT0,  "bootleg", "Bobble Bobble", GAME_SUPPORTS_SAVE )
GAME( 1986, sboblboa,   bublbobl, boblbobl, boblbobl, bublbobl, ROT0,  "bootleg", "Super Bobble Bobble (set 1)", GAME_SUPPORTS_SAVE )
GAME( 1986, sboblbob,   bublbobl, boblbobl, sboblbob, bublbobl, ROT0,  "bootleg", "Super Bobble Bobble (set 2)", GAME_SUPPORTS_SAVE )
GAME( 1986, bub68705,   bublbobl, bub68705, bublbobl, bublbobl, ROT0,  "bootleg", "Bubble Bobble (bootleg with 68705)", GAME_SUPPORTS_SAVE )

GAME( 1987, dland,      bublbobl, boblbobl, dland,    dland,    ROT0,  "bootleg", "Dream Land / Super Dream Land (bootleg of Bubble Bobble)", GAME_SUPPORTS_SAVE )
