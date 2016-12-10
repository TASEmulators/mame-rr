/**********************************************************************************


  AMATIC - MULTI GAME SYSTEM
  --------------------------

  Preliminary driver by Roberto Fresca.


  Encrypted gambling hardware based on a custom CPU.


***********************************************************************************


  Hardware Notes
  --------------

  ------------------------------------------------
    Board #1 (unknown slots game)
  ------------------------------------------------

  1x 40-pin custom CPU labeled:

     0288
     8012 (last digit is hard to read)
     11.12.96

  1x Unknown 40-pins IC (maybe 6845).
  1x Altera EPM5130LC (84-pins).
  1x KS82C55A (2x PPI).
  1x Unknown 40-pins IC (maybe another PPI).

  1x Dallas DS1236-10 (micro manager).
  1x Push button.

  1x Unknown 24-pin IC labeled SM64.
  1x Unknown 8-pin IC labeled SM65 (looks like a DAC).
  1x Unknown 8-pin IC no labeled (looks like a DAC).
  1x MC14538BCL (Dual precision monostable multivibrator).
  1x TDA2003 Audio Amp.
  1x Pot.

  1x HY6264ALP-10 (RAM).
  1x HY62256ALP-10 (RAM).

  1x 1mb ROM, near CPU.
  2x 27C512 ROMs.
  1x N82S147AN bipolar PROM.

  1x Xtal 16 MHz.
  1x DIP switches bank (x8).
  1x Battery.

  1x 2x17 male connector (like IDE ones).
  1x 2x8 contacts edge connector.
  1x 2x22 contacts edge connector.


  ------------------------------------------------
     Multi-Game I v2.4
  ------------------------------------------------

  1x 40-pin custom CPU labeled:

     Amatic Trading GMBH
     Lfnd. Nr. 1000
     Type:     801 I
     Datum:    12.02.96

  1x GoldStar 6845S.
  1x Altera EPM5130LC (84-pins).
  3x PPI 8255AC-2.

  1x Dallas DS1236-10 (micro manager).
  1x Push button.

  1x Unknown 24-pin IC labeled K-666 9330.
  1x Unknown 8-pin IC labeled K-664 9432 (looks like a DAC).
  1x LM358P (8-pin).
  1x MC14538BCL (Dual precision monostable multivibrator).
  1x TDA2003 Audio Amp.
  1x Pot.

  1x KM6264BL-7 (RAM).
  1x HY62256A-10 (RAM).

  1x 27C2001 ROM labeled MGI V GER 3.9/I/8201.
  1x 27C4000 ROM labeled Multi 2.4 ZG1.
  1x 27C4001 ROM labeled Multi 2.4 ZG2.
  1x 27C4001 ROM labeled Multi 2.4 ZG3.
  2x N82S147AN bipolar PROMs.

  1x Xtal 16 MHz.
  1x DIP switches bank (x8).
  1x Battery.

  1x 2x17 male connector (like IDE ones).
  1x 2x36 contacts edge connector.


  PCB Layout:
              ________________________________________________________________________
             | |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | |
   __________| |  |  |  |  |  |  |  |  |36x2 edge connector |  |  |  |  |  |  |  |  | |__________
  |    ____ ____                                                                                 |
  |   |  :::::  |                                                                                |
  |   |_________|                DIP1                                                            |
  | __________   __________   __________                                             __________  |
  ||ULN 2803A | |ULN 2803A | | 12345678 |                                           |ULN 2803A | |
  ||__________| |__________| |__________|                                           |__________| |
  |                                                                                              |
  |    __________________________    __________________________    __________________________    |
  |   |        NEC JAPAN         |  |        NEC JAPAN         |  |        NEC JAPAN         |   |
  |   |        D8255AC_2         |  |        D8255AC_2         |  |        D8255AC_2         |   |
  |   |        9150xD006         |  |        9150xD006         |  |        9150xD006         |   |
  |   |                          |  |                          |  |                          |   |
  |   |__________________________|  |__________________________|  |__________________________|   |
  |                                                                                              |
  |   _________________________                                                                  |
  |  |AMATIC TRADING GMBH      |                  __________    _______________                  |
  |  |Lfnd. Nr. 1000           |                 |   XTAL   |  |     K_666     |                 |
  |  |Type: 80 / I             |                 |  16 Mhz  |  |     9330      |                 |
  |  |Datum: 12.02.96          |                 |__________|  |               |                 |
  |  |_________________________|                               |_______________|                 |
  |                                                                                              |
  |  ? ____      ___________________       __________________    __                              |
  |   /    \    |MGI V GER          |     |                  |  |H |                             |
  |  | Batt |   |3.9 / I / 8201     |     |      ALTERA      |  |  |                             |
  |  | ery  |   |M27C2001           |     |                  |  |__|                             |
  |   \____/    |___________________|     |    EPM5130LC     |                                   |
  |                                       |      I9542       |                                   |
  |      __        _________________      |                  |   __                 ______       |
  |     |  |      |HY62256A         |     |                  |  |G |               /______\      |
  |     |  |      |LP_10            |     |                  |  |  |              |DALLAS  |     |
  |     |A |      |9506C  Korea     |     |                  |  |__|              |DS1994_F|     |
  |     |  |      |_________________|     |                  |                    |_______5|     |
  |     |  |                              |__________________|                     \______/      |
  |     |__|                                                                                     |
  |       _______________________            __    __    __    __    __    __                    |
  |      |   :::::::::::::::::   |          |  |  |  |  |  |  |  |  |  |  |  |                   |
  |      |___________ ___________|          |  |  |  |  |  |  |  |  |  |  |  |    ____________   |
  |                                         |C |  |B |  |B |  |B |  |B |  |B |   | GD74HCT273 |  |
  |          __________    _______          |  |  |  |  |  |  |  |  |  |  |  |   |____________|  |
  |         |MC14538BCP|  |       |         |  |  |  |  |  |  |  |  |  |  |  |    ____________   |
  |         |__________|  |SEC    |   __    |__|  |__|  |__|  |__|  |__|  |__|   | N82S147AN  |  |
  |                       |  KOREA|  |  |                                        |____________|  |
  |                       |       |  |  |     _______    _______    _______                      |
  |   __   __   __   __   |   506Y|  |E |    |       |  |       |  |       |                     |
  |  |  | |  | |  | |  |  |       |  |  |    |MULTI  |  |MULTI  |  |MULTI  |                     |
  |  |D | |D | |D | |D |  |KM6264B|  |  |    |    2.4|  |    2.4|  |    2.4|                     |
  |  |  | |  | |  | |  |  |L_7    |  |__|    |       |  |       |  |       |      ____________   |
  |  |  | |  | |  | |  |  |       |          |  ZG1  |  |  ZG2  |  |  ZG3  |     | GD74HCT273 |  |
  |  |__| |__| |__| |__|  |_______|          |       |  |       |  |       |     |____________|  |
  |                                          |       |  |       |  |       |      ____________   |
  |  ________________________    __   __     |27C4000|  |27C4001|  |27C4001|     | N82S147AN  |  |
  | |        GOLDSTAR        |  |  | |  |    |       |  |       |  |       |     |____________|  |
  | |        GM68B45S        |  |  | |  |    |       |  |       |  |       |      _________      |
  | |        9512            |  |F | |E |    |_______|  |_______|  |_______|     |GD74HC174|     |
  | |                        |  |  | |  |                                        |_________|     |
  | |________________________|  |  | |  |                                      _________         |
  |                             |  | |  |                                     |GD74HC174|        |
  |                             |__| |__|        AMATIC AMA_8000_2            |_________|        |
  |______________________________________________________________________________________________|

  A = Dallas / DS1236_10 / 9443A5
  B = MALAYSIA 114CS / SN74LS194AN
  C = MALAYSIA 544CS / SN74LS194AN
  D = GS 9447 / GD74LS157
  E = 53A9TKK / SN74HC374N
  F = GS 9504 / GD74HC244
  G = LM358P
  H = K_664 / 9432


  DIP1:
   ___________________
  | ON                |
  |  _______________  |
  | |_|_|_|_|_|_|_|_| |
  | |#|#|#|#|#|#|#|#| |
  | |_______________| |
  |  1 2 3 4 5 6 7 8  |
  |___________________|


  ------------------------------------------------
     Multi-Game III v3.5
  ------------------------------------------------

  1x 40-pin custom CPU labeled:

     Amatic
     Lfnd. Nr. 99/5070 467
     Type:     80(1?) I
     Datum:    10.01.00

  1x F68B45P.
  1x Altera EPM5130LC (84-pins).
  3x PPI NEC D71055C.

  1x Dallas DS1236-5 (micro manager).
  1x Push button.

  1x Yamaha YM3812.
  1x Yamaha Y3014B (DAC).
  1x LM358M (8-pin).
  1x MC14538 (Dual precision monostable multivibrator).
  1x TDA2003 (Audio Amp, Heatsinked).
  1x Pot.

  1x HY6264A (RAM).
  1x KM62256 (RAM).

  1x 27C2000 ROM labeled 'MG III VGer 3.5/I/8205'.
  1x 27C4000 ROM labeled 'MG III 51 ZG1'.
  1x 27C040 ROM labeled 'MG III 51 ZG2'.
  1x 27C040 ROM labeled 'MG III 51 ZG3'.
  1x 27C1024 labeled 'V'

  1x Xtal 16 MHz.
  1x DIP switches bank (x8).
  1x Battery.

  2x 2x17 male connector (like IDE ones).
  1x 2x36 contacts edge connector.


***********************************************************************************


    Memory Map
    ----------

    $00000 - $FFFFF   Still unknown...


***********************************************************************************

  +------------------------------------------------------------------------------+
  |             AMATIC Standard Edge Connector (Videomat, Multigame)             |
  +----------------------------------------+-------------------------------------+
  |            Component Side              |              Solder Side            |
  +-------------------+---------------+----+----+---------------+----------------+
  |     Function      |   Direction   | Nr | Nr |   Direction   |    Function    |
  +-------------------+---------------+----+----+---------------+----------------+
  | Lamp-HOLD3        |    OUTPUT     | 36 | r  |    ------     | N/C            |
  | REMOTE-PL         |    OUTPUT     | 35 | p  |    OUTPUT     | COIN-INVERTER  |
  | REMOTE-CLOCK      |    OUTPUT     | 34 | n  |    OUTPUT     | D-OUT          |
  | D-IN              |    INPUT      | 33 | m  |    SUPPLY     | +5V            |
  | REMOTE-SELECT     |    INPUT      | 32 | l  |    INPUT      | ???            |
  | N/C               |    ------     | 31 | k  |    INPUT      | ???            |
  | Coin-INPUT4       |    INPUT      | 30 | j  |    INPUT      | ???            |
  | GND               |    SUPPLY     | 29 | h  |    SUPPLY     | GND            |
  | HOPPER-RELAIS     |    OUTPUT     | 28 | f  |    INPUT      | ANTENNE        |
  | N/C               |    ------     | 27 | e  |    ------     | N/C            |
  | TICKET-OUT        |    OUTPUT     | 26 | d  |    ------     | N/C            |
  | N/C               |    ------     | 25 | c  |    ------     | N/C            |
  | KEY-DATA          |   TTLINPUT    | 24 | b  |   TTLINPUT    | KEY-DATA       |
  | N/C               |    ------     | 23 | a  |    INPUT      | KEY-CONTACT    |
  | GND               |    SUPPLY     | 22 | Z  |    SUPPLY     | GND            |
  | GND               |    SUPPLY     | 21 | Y  |    SUPPLY     | GND            |
  | GND               |    SUPPLY     | 20 | X  |    SUPPLY     | GND            |
  | +5V               |    SUPPLY     | 19 | W  |    SUPPLY     | +5V            |
  | +12V              |    SUPPLY     | 18 | V  |    SUPPLY     | +12V           |
  | Lamp-HOLD1        |    OUTPUT     | 17 | U  |    OUTPUT     | Lamp-START     |
  | Lamp-HOLD2        |    OUTPUT     | 16 | T  |    OUTPUT     | Lamp-HOLD5     |
  | Lamp-CANCEL       |    OUTPUT     | 15 | S  |    OUTPUT     | Lamp-HOLD4     |
  | Coin-INPUT1       |    INPUT      | 14 | R  |    OUTPUT     | HOPPER-OUT     |
  | Mech. Counter-IN  |    OUTPUT     | 13 | P  |    INPUT      | REMOTE         |
  | Mech. Counter-OUT |    OUTPUT     | 12 | N  |    INPUT      | Button-HOLD1   |
  | Mech. Counter-EXT |    OUTPUT     | 11 | M  |    INPUT      | Button-CANCEL  |
  | Button-HOLD5      |    INPUT      | 10 | L  |    INPUT      | Button-START   |
  | Bookkeeping 2     |    INPUT      | 09 | K  |    INPUT      | Bookkeeping 1  |
  | Button-HOLD2      |    INPUT      | 08 | J  |    INPUT      | Button-HOLD4   |
  | Button-HOPPER-OUT |    INPUT      | 07 | H  |    INPUT      | Button-HOLD3   |
  | HOPPER-COUNT      |    INPUT      | 06 | F  |    ------     | N/C            |
  | Coin-INPUT3       |    INPUT      | 05 | E  |    INPUT      | Coin-INPUT2    |
  | Monitor-GREEN     | TTLOUT-Analog | 04 | D  | TTLOUT-Analog | Monitor-RED    |
  | Monitor-SYNC      | TTLOUT-Analog | 03 | C  | TTLOUT-Analog | Monitor-BLUE   |
  | SPEAKER           |  OUT-Analog   | 02 | B  |    SUPPLY     | Monitor-GND    |
  | CREDIT-CLEAR      |    INPUT      | 01 | A  |    SUPPLY     | SPEAKER-GND    |
  +-------------------+---------------+----+----+---------------+----------------+

***********************************************************************************


  Findings about the encryption scheme
  ------------------------------------

  The program ROM is encrypted, but... The programmers left the cow out...
  They left some blank spaces that allow see some encryption patterns.


  Example:


  1) A string of 4 consecutive values with relation between them, is repeated once.
     Then the whole string is repeated again, but with bit 3 XOR'ed.

         +------------ value1
         |  +--------- value2 = value1 ^ 0x88
         |  |  +------ value3 = value1 ^ 0x22
         |  |  |  +--- value4 = value1 ^ 0x22 ^ 0x88
         |  |  |  |
         |  |  |  |

  $E000: AF 27 8D 05   AF 27 8D 05   A7 2F 85 0D  A7 2F 85 0D
        +-----------+ +-----------+ |                        |
           string1       string1    |                        |
        +-------------------------+ +------------------------+
                  string2                string2 ^ 0x08
        +----------------------------------------------------+
                                string3


  3) Then, all the E000-E00F range repeats in E010-E01F, but with bit 2 XOR'ed.

  $E010: AB 23 89 01 AB 23 89 01 A3 2B 81 09 A3 2B 81 09
        +-----------------------------------------------+
                        string3 ^ 0x04


  4) Then repeat all the E000-E01F range...

  $E000: AF 27 8D 05 AF 27 8D 05 A7 2F 85 0D A7 2F 85 0D
  $E010: AB 23 89 01 AB 23 89 01 A3 2B 81 09 A3 2B 81 09

  $E020: AF 27 8D 05 AF 27 8D 05 A7 2F 85 0D A7 2F 85 0D
  $E030: AB 23 89 01 AB 23 89 01 A3 2B 81 09 A3 2B 81 09


  5) Then the original value changes (0xAF -> 0x63), using the same algorithm
     (steps 1-4) for the next 0x40 bytes...

  $E040: 63 EB 41 C9 63 EB 41 C9 6B E3 49 C1 6B E3 49 C1
  $E050: 67 EF 45 CD 67 EF 45 CD 6F E7 4D C5 6F E7 4D C5
  $E060: 63 EB 41 C9 63 EB 41 C9 6B E3 49 C1 6B E3 49 C1
  $E070: 67 EF 45 CD 67 EF 45 CD 6F E7 4D C5 6F E7 4D C5

  Repeat step (5) ...till $E0FF.


  The encryption pattern repeats for the next 0x100 bytes, so the E000-E0FF range is repeated
  in E100-E1FF. Then the original values changes again, creating another block of 0x200 bytes.

  And so on....


***********************************************************************************


  [2009/09/11]

  - Initial release.
  - Added hardware specs from PCB pictures. Figured out some components.
  - Added findings about the encryption scheme.
  - Decoded 2 graphics bitplanes.
  - Added pre-defined clocks.
  - Added technical notes.


  *** TO DO ***

  - Decrypt the program ROMs.
  - Identify the CPU type.
  - Memory map.
  - Hook the remaining GFX bitplanes.
  - CRTC
  - Proper inputs.
  - Color decode routines.
  - Sound support.


***********************************************************************************/


#define MASTER_CLOCK	XTAL_16MHz
#define CPU_CLOCK		MASTER_CLOCK/4	/* guess */
#define SND_CLOCK		MASTER_CLOCK/4	/* guess */
#define CRTC_CLOCK		MASTER_CLOCK/8	/* guess */


#include "emu.h"
#include "cpu/z80/z80.h"
#include "video/mc6845.h"
#include "machine/8255ppi.h"
#include "sound/3812intf.h"
#include "sound/dac.h"


/************************************
*          Video Hardware           *
************************************/

static VIDEO_START( amaticmg )
{
}

static VIDEO_UPDATE( amaticmg )
{
	return 0;
}

static PALETTE_INIT( amaticmg )
{
}


/************************************
*       Read/Write Handlers         *
************************************/



/************************************
*      Memory Map Information       *
************************************/

static ADDRESS_MAP_START( amaticmg_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x00000, 0x3ffff) AM_ROM
//  AM_RANGE(0x0000, 0x0000) AM_RAM // AM_BASE_SIZE_GENERIC(nvram)
//  AM_RANGE(0x0000, 0x0000) AM_DEVWRITE("crtc", mc6845_address_w)
//  AM_RANGE(0x0000, 0x0000) AM_DEVREADWRITE("crtc", mc6845_register_r, mc6845_register_w)
//  AM_RANGE(0x0000, 0x0000) AM_RAM_WRITE(amaticmg_videoram_w) AM_BASE(&amaticmg_videoram)
//  AM_RANGE(0x0000, 0x0000) AM_RAM_WRITE(amaticmg_colorram_w) AM_BASE(&amaticmg_colorram)
ADDRESS_MAP_END

static ADDRESS_MAP_START( amaticmg_portmap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
//  AM_RANGE(0x00, 0x00) AM_DEVREADWRITE("ppi8255_0", ppi8255_r, ppi8255_w)
//  AM_RANGE(0x00, 0x00) AM_DEVREADWRITE("ppi8255_1", ppi8255_r, ppi8255_w)
//  AM_RANGE(0x00, 0x00) AM_DEVREADWRITE("ppi8255_2", ppi8255_r, ppi8255_w)
//  AM_RANGE(0x00, 0x00) AM_DEVWRITE("ymsnd", ym3812_w)
//  AM_RANGE(0x00, 0x00) AM_DEVWRITE("dac1", dac_signed_w)
//  AM_RANGE(0x00, 0x00) AM_DEVWRITE("dac2", dac_signed_w)

ADDRESS_MAP_END

/*
    Unknown R/W
    -----------


*/


/************************************
*           Input ports             *
************************************/

static INPUT_PORTS_START( amaticmg )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-1") PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-2") PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-3") PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-4") PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-5") PORT_CODE(KEYCODE_5)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-6") PORT_CODE(KEYCODE_6)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-7") PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-8") PORT_CODE(KEYCODE_8)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-1") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-2") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-3") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-4") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-5") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-6") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-7") PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-8") PORT_CODE(KEYCODE_I)

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-1") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-2") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-3") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-4") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-5") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-6") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-7") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-8") PORT_CODE(KEYCODE_K)

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-1") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-2") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-3") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-4") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-5") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-6") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-7") PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("3-8") PORT_CODE(KEYCODE_L)

	PORT_START("SW1")
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


/************************************
*         Graphics Layouts          *
************************************/

static const gfx_layout charlayout =
{
/* Only 2 planes are hooked. This need to be fixed */

	4,8,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(0,2), RGN_FRAC(0,2) + 4 },
	{ 3, 2, 1, 0 },	/* tiles are x-flipped */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*4*2
};


/************************************
*    Graphics Decode Information    *
************************************/

static GFXDECODE_START( amaticmg )
	GFXDECODE_ENTRY( "gfx1", 0x0000, charlayout, 0, 16 )
GFXDECODE_END


/************************************
*          Sound Interface          *
************************************/

//static const ym3812_interface ym3812_config =
//{
//  sound_irq
//};


/************************************
*          CRTC Interface           *
************************************/

//static const mc6845_interface mc6845_intf =
//{
//  "screen",   /* screen we are acting on */
//  8,          /* number of pixels per video memory address */
//  NULL,       /* before pixel update callback */
//  NULL,       /* row update callback */
//  NULL,       /* after pixel update callback */
//  DEVCB_NULL, /* callback for display state changes */
//  DEVCB_NULL, /* callback for cursor state changes */
//  DEVCB_NULL, /* HSYNC callback */
//  DEVCB_NULL, /* VSYNC callback */
//  NULL        /* update address callback */
//};


/************************************
*      PPI 8255 (x3) Interface      *
************************************/

//static const ppi8255_interface ppi8255_intf[3] =
//{
//  {   /* (00-00) Mode X - Port X set as input */
//      DEVCB_NULL,                     /* Port A read */
//      DEVCB_NULL,                     /* Port B read */
//      DEVCB_NULL,                     /* Port C read */
//      DEVCB_NULL,                     /* Port A write */
//      DEVCB_NULL,                     /* Port B write */
//      DEVCB_NULL,                     /* Port C write */
//  },
//  {   /* (00-00) Mode X - Port X set as input */
//      DEVCB_NULL,                     /* Port A read */
//      DEVCB_NULL,                     /* Port B read */
//      DEVCB_NULL,                     /* Port C read */
//      DEVCB_NULL,                     /* Port A write */
//      DEVCB_NULL,                     /* Port B write */
//      DEVCB_NULL,                     /* Port C write */
//  },
//  {   /* (00-00) Mode X - Port X set as input */
//      DEVCB_NULL,                     /* Port A read */
//      DEVCB_NULL,                     /* Port B read */
//      DEVCB_NULL,                     /* Port C read */
//      DEVCB_NULL,                     /* Port A write */
//      DEVCB_NULL,                     /* Port B write */
//      DEVCB_NULL,                     /* Port C write */
//  }
//};

/************************************
*          Machine Drivers          *
************************************/

static MACHINE_DRIVER_START( amaticmg )
	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, CPU_CLOCK)		/* WRONG! */
	MDRV_CPU_PROGRAM_MAP(amaticmg_map)
	MDRV_CPU_IO_MAP(amaticmg_portmap)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

//  MDRV_NVRAM_HANDLER(generic_0fill)

	/* 3x 8255 */
//  MDRV_PPI8255_ADD( "ppi8255_0", ppi8255_intf[0] )
//  MDRV_PPI8255_ADD( "ppi8255_1", ppi8255_intf[1] )
//  MDRV_PPI8255_ADD( "ppi8255_2", ppi8255_intf[2] )

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(512, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-1)

//  MDRV_MC6845_ADD("crtc", MC6845, CRTC_CLOCK, mc6845_intf)

	MDRV_GFXDECODE(amaticmg)

	MDRV_PALETTE_INIT(amaticmg)
	MDRV_PALETTE_LENGTH(0x100)
	MDRV_VIDEO_START(amaticmg)
	MDRV_VIDEO_UPDATE(amaticmg)

	/* sound hardware */
//  MDRV_SPEAKER_STANDARD_MONO("mono")

//  MDRV_SOUND_ADD("ymsnd", YM3812, SND_CLOCK)
//  MDRV_SOUND_CONFIG(ym3812_config)
//  MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

//  MDRV_SOUND_ADD("dac", DAC, 0)   /* Y3014B */
//  MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

MACHINE_DRIVER_END


/************************************
*             Rom Load              *
************************************/

ROM_START( am_uslot )
	ROM_REGION( 0x40000, "maincpu", 0 )	/* encrypted program ROM...*/
	ROM_LOAD( "u3.bin",  0x00000, 0x20000, CRC(29bf4a95) SHA1(a73873f7cd1fdf5accc3e79f4619949f261400b8) )

	ROM_REGION( 0x20000, "gfx1", 0 )	/* There are only slots tiles. No chars inside */
	ROM_LOAD( "u9.bin",  0x00000, 0x10000, CRC(823a736a) SHA1(a5227e3080367736aac1198d9dbb55efc4114624) )
	ROM_LOAD( "u10.bin", 0x00000, 0x10000, CRC(6a811c81) SHA1(af01cd9b1ce6aca92df71febb05fe216b18cf42a) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "n82s147a.bin", 0x0000, 0x0200, CRC(dfeabd11) SHA1(21e8bbcf4aba5e4d672e5585890baf8c5bc77c98) )
ROM_END


ROM_START( am_mg24 )
	ROM_REGION( 0x40000, "maincpu", 0 )	/* encrypted program ROM...*/
	ROM_LOAD( "mgi_vger_3.9-i-8201.i6.bin", 0x00000, 0x40000, CRC(9ce159f7) SHA1(101c277d579a69cb03f879288b2cecf838cf1741) )

	ROM_REGION( 0x180000, "gfx1", 0 )
	ROM_LOAD( "multi_2.4_zg1.i17.bin", 0x000000, 0x80000, CRC(4a60a718) SHA1(626991abee768da58e87c7cdfc4fcbae86c6ea2a) )
	ROM_LOAD( "multi_2.4_zg2.i18.bin", 0x080000, 0x80000, CRC(b504e1b8) SHA1(ffa17a2c212eb2fffb89b131868e69430cb41203) )
	ROM_LOAD( "multi_2.4_zg3.i33.bin", 0x100000, 0x80000, CRC(9b66bb4d) SHA1(64035d2028a9b68164c87475a1ec9754453ad572) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "n82s147a_1.bin", 0x0000, 0x0200, NO_DUMP )
	ROM_LOAD( "n82s147a_2.bin", 0x0200, 0x0200, NO_DUMP )
ROM_END

ROM_START( am_mg3 )
	ROM_REGION( 0x40000, "maincpu", 0 )	/* encrypted program ROM...*/
	ROM_LOAD( "mg_iii_vger_3.5-i-8205.bin", 0x00000, 0x40000, CRC(21d64029) SHA1(d5c3fde02833a96dd7a43481a489bfc4a5c9609d) )

	ROM_REGION( 0x180000, "gfx1", 0 )
	ROM_LOAD( "mg_iii_51_zg1.bin", 0x000000, 0x80000, CRC(84f86874) SHA1(c483a50df6a9a71ddfdf8530a894135f9b852b89) )
	ROM_LOAD( "mg_iii_51_zg2.bin", 0x080000, 0x80000, CRC(4425e535) SHA1(726c322c5d0b391b82e49dd1797ebf0abfa4a65a) )
	ROM_LOAD( "mg_iii_51_zg3.bin", 0x100000, 0x80000, CRC(36d4c0fa) SHA1(20352dbbb2ce2233be0f4f694ddf49b8f5d6a64f) )

	ROM_REGION( 0x20000, "other", 0 )
	ROM_LOAD( "v.bin", 0x00000, 0x20000, CRC(524767e2) SHA1(03a108494f42365c820fdfbcba9496bda86f3081) )
ROM_END


/************************************
*       Driver Initialization       *
************************************/

static DRIVER_INIT( amaticmg )
{

}


/************************************
*           Game Drivers            *
************************************/

/*    YEAR  NAME      PARENT  MACHINE   INPUT     INIT      ROT    COMPANY                FULLNAME                     FLAGS  */
GAME( 1996, am_uslot, 0,      amaticmg, amaticmg, amaticmg, ROT0, "Amatic Trading GmbH", "Amatic Unknown Slots Game",  GAME_IMPERFECT_GRAPHICS | GAME_WRONG_COLORS | GAME_UNEMULATED_PROTECTION | GAME_NO_SOUND | GAME_NOT_WORKING )
GAME( 2000, am_mg24,  0,      amaticmg, amaticmg, amaticmg, ROT0, "Amatic Trading GmbH", "Multi Game I (V.Ger 2.4)",   GAME_IMPERFECT_GRAPHICS | GAME_WRONG_COLORS | GAME_UNEMULATED_PROTECTION | GAME_NO_SOUND | GAME_NOT_WORKING )
GAME( 2000, am_mg3,   0,      amaticmg, amaticmg, amaticmg, ROT0, "Amatic Trading GmbH", "Multi Game III (V.Ger 3.5)", GAME_IMPERFECT_GRAPHICS | GAME_WRONG_COLORS | GAME_UNEMULATED_PROTECTION | GAME_NO_SOUND | GAME_NOT_WORKING )
