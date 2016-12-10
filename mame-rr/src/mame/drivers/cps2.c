/***************************************************************************

Driver by Paul Leaman (paul@vortexcomputing.demon.co.uk)

Thanks to Andreas Naive and Nicola Salmoria for understanding the
encryption mechanism.

Thanks to Raz, Crashtest and the CPS2 decryption team for their
initial work on extracting decrypted code.


  Capcom System 2
  ===============

CPS2 Hardware Overview
Capcom, 1993-2004

From 1993 to 2004 Capcom produced a generic system known as CPS2 (Capcom Play System 2). It
comprises a base board (known as the A-Board) and a top board (known as the B-Board). Both were
housed in separate plastic boxes and fitted together via four multi-pin connectors.
The boxes are colour-coded for release in specific country regions....
Green  - Japan
Blue   - U.S.A. and Europe
Orange - South America
Grey   - Asia
Yellow - All (Rent version; can be hired by operators for testing)

Grey and orange B-Boards require an A-Board with a matching colour to work. Green A/B and Blue A/B
boards are interchangeable. Yellow boards will fit any A-Board since they were used for renting to
operators for testing purposes in the hope that they would buy it.

The first game produced on this system was "Super Street Fighter II The New Challengers". The last
game (so far) is "Hyper Street Fighter II : The Anniversary Edition". All up, around 50 unique
games were developed on CPS2 hardware, with dozens of releases in different regions, totalling over
170 variations. Over the years Capcom have produced a lot of games using the same theme, making
numerous spin-offs of their titles with varying subtitles in an effort to prolong the life of the
aging CPS2 hardware (one would assume).

The base board contains the main CPU (a 68000), RAM, graphic generating hardware, sound hardware
(Q-Sound and Kabuki taken from CPS1) and I/O hardware. The top board contains the software
(EPROMS etc) and some custom ICs used to generate backgrounds and sprites.
The CPS2 hardware is very different from most other generic arcade systems in that it uses some
very complex encryption of the main program to combat bootleggers. The decryption key is held in
some SRAM inside one of the custom IC's and powered by a battery. If the battery dies the system
kills itself, showing only a blue or green screen on power-up. There are no known simple fixes so
far so it is then unusable, but can be returned to Capcom for a repair, providing the security
stickers have not been tampered with and the plastic box has not been opened.
The encryption is so involved that there were no known bootlegs of any of the CPS2 games.

The sound CPU is a standard Z80. On some PCBs a custom Z80 is used instead (called a "Kabuki"). This
is also encrypted and is powered by a battery. When this battery dies, the chip works as a regular
Z80 and can not decrypt the program so there is no sound, but the game continues to work. However,
in all CPS2 games, Capcom chose not to use the Kabuki encryption since none of the CPS2 games have
an encrypted sound program. There is a jumper pad on the PCB next to the Z80 (JP3). When this is
shorted, it sets a pin on the Kabuki to run in encrypted mode, but it has been found not shorted on
all CPS2 games so far. The Kabuki or a regular Z80 has been found in use for the sound CPU, so it
is possible Capcom are using the Kabuki chips from old stock as regular Z80 CPUs.

PCB Layouts
-----------

A-Board
-------
CAPCOM 93646A-6
   |----------------------------------------------------------------------------------------------|
   |SW3 SW2 SW1                    |-----------------------|     |-----------------------|        |
|--|                               |-----------------------|     |-----------------------|     CN6|
|                                8464    93C46(2)      CN3        CN4                             |
|                                                                                                 |
|                                  PAL1               |---------|                                 |
|                                                     |CAPCOM-Q1|                                 |
|                                  PAL2               |DL 1425  |               |---------|       |
|J                                                    |DSP-16A  |               |CAPCOM   |       |
|A              5863                                  |(PLCC84) |               |CPS-B-21 |       |
|M                                  |---------------| |---------|               |DL-0921  |       |
|M              5863                |DL-030P(or Z80)|                           |(QFP160) |       |
|A                                  |---------------|                           |---------|       |
|                                                    60MHz                                        |
|            |---------|            8MHz                                                          |
|            |CAPCOM   |                                                                          |
|            |DL-1123  |                                                 PAL3                     |
|    93C46(1)|I/O      |                                  |---------|                             |
|            |(QFP136) |                                  |CAPCOM   |                             |
|--|         |---------|                                  |CPS-A-01 |              |---------|    |
   |                                                      |DL-0311  |              |CAPCOM   |    |
   |                                                      |(QFP160) |              |DL-1625  |    |
   |                                                      |---------|   16MHz      |SPB      |    |
   |                                                                               |(QFP128) |    |
  |-|                                                                              |---------|    |
  | |                                                                                             |
  | |                                    M51953B                                                  |
  | |CN5    LM833    TC9176P   LM833                                         |-------|            |
  | |       LM833              NE5532                                        |CAPCOM |            |
  | |       LM833              TDA1543                 PAL4    HM658128      |DL-2227|  HM514260  |
  |-|                                                                        |DRC    |            |
   |            BATTERY     TC9185P                    PAL5    HM658128      |(QFP64)|  HM514260  |
  |- CNL                                     CN1                         CN2 |-------|            |
   |         TA8225L               |-----------------------|     |-----------------------|        |
  |- CNR                           |-----------------------|     |-----------------------|        |
   |----------------------------------------------------------------------------------------------|
Notes:
      5863      - Sony CXK5863BP-35 8k x8 SRAM (DIP28)
      8464      - Fujitsu MB8464A-10L 8k x8 SRAM (SOP28)
      HM514260  - Hitachi HM514260AJ8 256k x 16 DRAM (SOJ40)
      HM658128  - Hitachi HM658128ADFP-10 128k x8 SRAM (SOP32)
      M5195B    - Mitsubishi Electric Corp. M5195B Voltage Detection and System Reset IC (SIP5)
      LM833     - National Semiconductor LM833 Dual Audio Operational Amplifier (DIP8)
      TDA1543   - Philips TDA1543 Dual 16-bit DAC (DIP8)
      NE5532    - Philips NE5532 Internally-compensated Dual Low Noise Operational Amplifier (DIP8)
      TC9176P   - Toshiba TC9176 (purpose unknown, DIP16)
      TC9185P   - Toshiba TC9185P Electronic Volume Control IC (DIP20)
      TA8225L   - Toshiba TA8225L 45W BTL Audio Amplifier (ZIP17)
      93C46(1)  - Atmel 93C46 EEPROM (SOIC8, tied to the custom I/O chip)
      93C46(2)  - Atmel 93C46 EEPROM (SOIC8, tied to the Kabuki chip)
      SW1       - Test Switch
      SW2       - Volume Up
      SW3       - Volume Down
      CN1/2/3/4 - 96 Pin Connectors for connection of top B-board
      CN5       - 34 Pin Connector used for (generally) extra kick buttons on fighting games
      CN6       - 2 Pin Fan connector
      CN L/R    - RCA Connectors for Left/Right Audio when QSound (stereo output) is enabled
      PAL1      - MMI PAL16L8 (DIP20, stamped 'D8L1')
      PAL2      - MMI PAL16L8 (DIP20, stamped 'BGSA4')
      PAL3      - MMI PAL16R4 (DIP20, stamped 'BGSA5')
      PAL4      - MMI PAL16L8 (DIP20, stamped 'BGSA1')
      PAL5      - MMI PAL16L8 (DIP20, stamped 'BGSA2')
      VSync     - 59.6388Hz

      Custom IC's -
                   DL-030P - KABUKI Custom encrypted Z80, running at 8.000MHz, manufactured by VLSI
                             Technology (DIP40)
                             On most PCB's this is a regular Zilog Z80 (Z0840008PSC)
                   DL-1425 - CAPCOM-Q1 QSound Processor, DSP-16A (C) 92 AT&T, clock input of
                             60.000MHz (PLCC84)
                   DL-0921 \
                   DL-0311 / CPS-A/B Graphics Processors (QFP160)
                   DL-1625 - Custom 68000 CPU, running at 16.000MHz (QFP128)
                   DL-2227 - DRAM Refresh Controller (QFP64)
                   DL-1123 - I/O Controller (QFP136)

      Connector Pinouts -

                       JAMMA Connector                                     Extra Button Connector
                       ---------------                                     ----------------------
                    PART SIDE    SOLDER SIDE                                     TOP    BOTTOM
                ----------------------------                             --------------------------
                      GND  01    A  GND                                      GND  01    02  GND
                      GND  02    B  GND                                      +5V  03    04  +5V
                      +5V  03    C  +5V                                     +12V  05    06  +12V
                      +5V  04    D  +5V                             Configurable  07    08  Configurable
                       NC  05    E  NC                              Configurable  09    10  Configurable
                     +12V  06    F  +12V                            Configurable  11    12  Configurable
                           07    H                                  Configurable  13    14  Configurable
           Coin Counter 1  08    J  NC                              Configurable  15    16  Configurable
             Coin Lockout  09    K  Coin Lockout                    Configurable  17    18  Configurable
               Speaker (+) 10    L  Speaker (-)                     Configurable  19    20  Configurable
                       NC  11    M  NC                              Configurable  21    22  Configurable
                Video Red  12    N  Video Green                     Configurable  23    24  Configurable
               Video Blue  13    P  Video Composite Sync            Configurable  25    26  Configurable
             Video Ground  14    R  Service Switch                  Configurable  27    28  Configurable
                     Test  15    S  NC                               Volume Down  29    30  Volume UP
                   Coin A  16    T  Coin B                                   GND  31    32  GND
           Player 1 Start  17    U  Player 2 Start                           GND  33    34  GND
              Player 1 Up  18    V  Player 2 Up
            Player 1 Down  19    W  Player 2 Down
            Player 1 Left  20    X  Player 2 Left                Pins 07 to 29 can be configured to anything
           Player 1 Right  21    Y  Player 2 Right               as games require. This includes coin inputs,
        Player 1 Button 1  22    Z  Player 2 Button 1            coin lockouts, joysticks and buttons. There
        Player 1 Button 2  23    a  Player 2 Button 2            are at least 2 known configurations seen in
        Player 1 Button 3  24    b  Player 2 Button 3            CPS-2 games released so far (see below).
        Player 1 Button 4  25    c  Player 2 Button 4
                       NC  26    d  NC
                      GND  27    e  GND
                      GND  28    f  GND

Note that only some games use the player 1 and 2 button 4 output on the JAMMA connector.
Most games that require the use of button 4 get it from the extra button connector.

Known Extra Button Connector configurations -

             Most fighting type games                       More than 2 Player games
             ------------------------                       ------------------------
                   TOP    BOTTOM                                  TOP    BOTTOM
           ----------------------------                   ----------------------------
               GND  01    02  GND                             GND  01    02  GND
               +5V  03    04  +5V                             +5V  03    04  +5V
              +12V  05    06  +12V                           +12V  05    06  +12V
                NC  07    08  NC                    3Up Coin Lock  07    08  4Up Coin Lock
      2Up Button 6  09    10  NC                         3Up Coin  09    10  4Up Coin
                NC  11    12  NC                        3Up Start  11    12  4Up Start
                NC  13    14  NC                           3Up Up  13    14  4Up Up
      1Up Button 6  15    16  NC                         3Up Down  15    16  4Up Down
      1Up Button 5  17    18  NC                         3Up Left  17    18  4Up Left
      1Up Button 4  19    20  NC                        3Up Right  19    20  4Up Right
      2Up Button 4  21    22  NC                     3Up Button 1  21    22  4Up Button 1
      2Up Button 5  23    24  NC                     3Up Button 2  23    24  4Up Button 2
                NC  25    26  NC                     3Up Button 3  25    26  4Up Button 3
                NC  27    28  NC                     3Up Button 4  27    28  4Up Button 4
       Volume Down  29    30  Volume UP               Volume Down  29    30  Volume UP
               GND  31    32  GND                             GND  31    32  GND
               GND  33    34  GND                             GND  33    34  GND

Spinners -
There are 2 known games which use spinners, Puzz Loop 2 and Eco Fighters.
Puzz Loop 2 can come with an extra PCB that sits between the JAMMA harness and A-Board.
The spinners plug into this board and there is an option in the games settings to enable
it. If the extra PCB board is missing its not a problem because the game is still fully
playable using normal joysticks.
Eco Fighters has a special limited version of the game (not currently dumped) that uses
spinners. This version of the game did not support joysticks (only the normal version of
the game does). Not much more is known about how the spinners used here hook up to the
system or if the hardware required is the same as that used with Puzz Loop 2.


B-Boards
--------
CAPCOM 93646B-4  \  There are small variations between board revisions; changed
CAPCOM 93646B-6   | PALs, alternative location for an EEPROM, extra connectors
CAPCOM 93646B-7  /  (CN9) and other minor diferences.
 |-------------------------------------------------------------------------|
 |       CN3|-----------------------|     |-----------------------|CN4     |
 |          |-----------------------|     |-----------------------|        |
|-|      CN5|-----------------------|     |-----------------------|CN6     |
| |                                                                        |
| |  ROM1.1A                 ROM11.1E                                      |
| |           93C46                        ROM13.1J        ROM17.1M        |
| |  ROM2.2A                 ROM12.2E                                      |
| |                                        ROM14.2J        ROM18.2M        |
| |CN7                                                                     |
| |                                        ROM15.3J        ROM19.3M        |
| |                                                                        |
| | 8464     |---------|   |---------|     ROM16.4J        ROM20.4M        |
| |          |CAPCOM   |   |CAPCOM   |                                     |
| | 8464 CN9 |DL-1827  |   |DL-1525  |                                     |
|-|          |CIF      |   |SPA      |                                     |
 |           |(QFP160) |   |(QFP208) |                                     |
 |           |---------|   |---------|                                     |
 |                                                                         |
 |                                                                         |
 |                                                                         |
 |   ROM3.6A        ROM7.6D     |---------|   |---------|   |---------|    |
 |                              |CAPCOM   |   |CAPCOM   |   |CAPCOM   |    |
 |                              |DL-1727  |   |DL-2027  |   |DL-1927  |    |
 |   ROM4.7A        ROM8.7D     |MIF      |   |CGD      |   |CGA      |    |
 |                              |(QFP160) |   |(QFP120) |   |(QFP120) |    |
 |                              |---------|   |---------|   |---------|    |
 |   ROM5.8A        ROM9.8D                                                |
 |                                                                         |
 |                                    PAL1    PAL3               BATTERY   |
 |   ROM6.9A        ROM10.9D                                               |
 |                                    PAL2                                 |
 |                    CN1                         CN2                      |
 |          |-----------------------|     |-----------------------|        |
 |          |-----------------------|     |-----------------------|        |
 |-------------------------------------------------------------------------|
Notes:
      8464      - Fujitsu MB8464A-10L 8k x8 SRAM (SOP28)
      93C46     - Atmel 93C46 EEPROM (SOIC8, not populated on revision -7 board)
      CN1/2/3/4 - 96 Pin Connectors for connection to lower A-board (connectors below PCB)
      CN5/6     - 96 Pin Connectors for connection to optional daughter boards used by some games
                  (connectors above PCB)
      CN7       - 64 Pin Connector for connection of optional Communication Board
      CN9       - 6 Pin JTAG Connector (only on newer B-board revisions), it's used for reprogramming
                  custom IC's (probably encryption key information)
      PAL1      - MMI PAL16L8 (DIP20, stamped 'BGSB1')
      PAL2      - MMI PAL16L8 (DIP20, stamped 'BGSB2')
      PAL3      - MMI PAL16L8 (DIP20, stamped 'BGSB3F' on rev -4 board, stamped 'BGSB3G' on rev -7
                  board)

      Custom IC's -
                   DL-1827 CIF (QFP160)
                   DL-1525 SPA (QFP208)
                   DL-1727 MIF (QFP120)
                   DL-2027 CGD (QFP120)
                   DL-1927 CGA (QFP120)

      ROMs -
            Note, the ROM names shown on the above layout are generic. Each EPROM on every game has
            a unique sticker attached to it. All of the MASKROMs are also stamped with unique names
            for each game. The amount of EPROMs/MASKROMs used also differs per game, depending on
            requirements. The PCB is wired for certain sized ROMs by default, but via jumpers they
            can be reconfigured to allow accepting other sized devices.

                                                                           Some example ROM names
                                                                  (see the source below for full name details)
                                                                         /---------------------------\
            Location  Device type and size               Use               XMen COTA    Puzz Loop 2
            ------------------------------------------------------------------------------------------
            1.1A      27C010 (1M) or 27C1000 (1M)        Sound Program     XMN_01.1A    PL2_01.1A
            2.2A      27C010 (1M) or 27C1000 (1M)        Sound Program     XMN_02.2A    -
            3.6A      27C4096 (4M)                       Main Program      XMNE_03E.6A  PL2J_03.6A
            4.7A      27C4096 (4M)                       Main Program      XMNE_04E.7A  PL2J_04.7A
            5.8A      27C4096 (4M)                       Main Program      XMNE_05.8A   PL2J_05.8A
            6.9A      27C4096 (4M)                       Main Program      XMNE_06.9A   PL2J_06.9A
            7.6D      27C4096 (4M)                       Main Program      XMNE_07.6D   -
            8.7D      27C4096 (4M)                       Main Program      XMNE_08.7D   -
            9.8D      27C4096 (4M)                       Main Program      XMNE_09.8D   -
            10.9D     27C4096 (4M)                       Main Program      XMNE_10.9D   -
            11.1E     HN624316 (16M) or KM23C32000 (32M) QSound Samples    XMN_11M.1E   \
            12.2E     HN624316 (16M) or KM23C32000 (32M) QSound Samples    XMN_12M.2E   |
            13.1J     HN624316 (16M) or KM23C32000 (32M) Graphics          XMN_13M.1J   |
            14.2J     HN624316 (16M) or KM23C32000 (32M) Graphics          XMN_14M.2J   |
            15.3J     HN624316 (16M) or KM23C32000 (32M) Graphics          XMN_15M.3J   |
            16.4J     HN624316 (16M) or KM23C32000 (32M) Graphics          XMN_16M.4J   |  Located
            17.1M     HN624316 (16M) or KM23C32000 (32M) Graphics          XMN_17M.1M   |  on SIMMs
            18.2M     HN624316 (16M) or KM23C32000 (32M) Graphics          XMN_18M.2M   |
            19.3M     HN624316 (16M) or KM23C32000 (32M) Graphics          XMN_19M.3M   |
            20.4M     HN624316 (16M) or KM23C32000 (32M) Graphics          XMN_20M.4M   /

            Capcom have a unique ROM naming system for CPS-2 games as mentioned above. A typical ROM
            name is 'SSFA 03B' and is clearly printed on the sticker of each ROM. In most cases stickers
            have the ROM details over 2 lines, in this case 'SSFA' would be on the first line and '03B'
            on the second line. Each part of this label name is detailed below...

            SSF -  The game title shortened to 3 characters, this game is 'Super Street Fighter 2'.

            A   -  The region of the game, in this case 'Asia'. Known regions are...
                   J = Japan          E = ETC (World and Euro)
                   U = USA            A = Asia
                   H = Hispanic       N = Oceania
                   B = Brazil         O = Found in yellow rent versions

            03  -  Under each DIP socket is a large white number. The number on the sticker matches
                   this number as a location identifier.

            B   -  This is the revision of the ROM software. When a game is first released the
                   initial revision will not be noted on the label. During production, the software
                   may be updated and the sticker will then have the letter 'A/B/C/D/E' etc
                   appended to the end to denote a changed revision of the software for that particular
                   ROM. From the example we can see this ROM is the 3rd revision since release.
                   When the software is revised, not all ROMs have to be updated, in many cases only
                   some of the ROMs have a revision update, and the other ROMs will remain the same.


ROM Daughterboards -
                    As well as the 3 daughter boards shown below, there are other variations that
                    exist, but due to lack of information they are not documented here.

CAPCOM 93661G-2
|-------------------------------------------------------------------------|
|                                                                         |
|          |-----------------------|     |-----------------------|        |
|       CN1|-----------------------|     |-----------------------|CN2     |
|                                                                         |
|                                                                         |
| PAL.1A     ROMQ1.1C      ROMQ5.1D      ROM21.1E      ROM25.1F           |
| PAL.2A                                                                  |
|            ROMQ2.2C      ROMQ6.2D      ROM22.2E      ROM26.2F           |
|  93C46                                                                  |
|            ROMQ3.4C      ROMQ7.4D      ROM23.4E      ROM27.4F           |
|                                                                         |
| Jumpers    ROMQ4.5C      ROMQ8.5D      ROM24.5E      ROM28.5F           |
|                                                                         |
|                                                                         |
|-------------------------------------------------------------------------|
Notes:
      This board is known to be used with some versions of "Street Fighter Zero 2" but not all.
      When it is used with this game, only 4 graphics ROMs are on this board (ROM21 to ROM24),
      all the others are on the B-board.

      CN1/2    - 96 Pin Connectors for connection to B-Board (the connectors are below the PCB)
      PAL.1A   - MMI PAL16L8 (not populated)
      PAL.2A   - MMI PAL16L8 (DIP20, stamped 'BGSG2B')
      93C46    - Atmel 93C46 EEPROM (SOIC8, not populated)
      Jumpers  - 16 Jumper pads
      ROMs     - ROMQ1 to ROMQ8 are HN62344
                 ROM21 to ROM28 are HN624116

CAPCOM 93646C-3
|-------------------------------------------------------------------------|
|            93C46                                                        |
|          |-----------------------|     |-----------------------|        |
|       CN1|-----------------------|     |-----------------------|CN2     |
|                                                                         |
|                                                                         |
|                ROM59.4D       ROM69.4J       ROM79.4M       ROM89.4P    |
|                                                                         |
|                ROM60.5D       ROM70.5J       ROM80.5M       ROM90.5P    |
|                                                                         |
|  ROM51.6A      ROM61.6D       ROM71.6J       ROM81.6M       ROM91.6P    |
|                                                                         |
|  ROM52.7A      ROM62.7D       ROM72.7J       ROM82.7M       ROM92.7P    |
|                                                                         |
|  ROM53.8A      ROM63.8D       ROM73.8J       ROM83.8M       ROM93.8P    |
|                                                                         |
|  ROM54.9A      ROM64.9D       ROM74.9J       ROM85.9M       ROM94.9P    |
|                                                                         |
|  ROM55.10A     ROM65.10D      ROM75.10J      ROM85.10M      ROM95.10P   |
|                                                                         |
|  ROM56.11A     ROM66.11D      ROM76.11J      ROM86.11M      ROM96.11P   |
|                                                                         |
|  ROM57.12A     ROM67.12D      ROM77.12J      ROM87.12M      ROM97.12P   |
|                                                                         |
|  ROM58.13A     ROM68.13D      ROM78.13J      ROM88.13M      ROM98.13P   |
|                                                                         |
|-------------------------------------------------------------------------|
Notes:
      This board is known to be used with some yellow rent versions of games but not all. When
      it is used, no sound or graphics ROMs are used on the B-board.

      CN1/2 - 96 Pin Connectors for connection to B-Board (the connectors are below the PCB)
      93C46 - Atmel 93C46 EEPROM
      ROMs  - All sockets are for 27C4096 devices (ROM51 to ROM58 are for sound and
              ROM59 to ROM98 are for graphics.

CAPCOM 00716C-3
|-------------------------------------------------------------------------|
|                    CN1                            CN2                   |
|          |-----------------------|     |-----------------------|        |
|          |-----------------------|     |-----------------------|        |
|                                                                         |
|           74LS157   74LS157                                             |
|                                                                         |
|             SIMM5                                SIMM1                  |
|                                                                         |
|                                                  SIMM2                  |
|                              74LS04                                     |
|                                                  SIMM3                  |
|                                                                         |
|             SIMM6            M51953              SIMM4                  |
|                                                                         |
|-------------------------------------------------------------------------|
Notes:
      M5195B          - Mitsubishi Electric Corp. M5195B Voltage Detection & System Reset IC (SIP5)
                        (May not be populated on some boards)
      CN1/2           - 96 Pin Connectors for connection to B-Board (the connectors are below the PCB)
      SIMM1/2/3/4/5/6 - Generic 72-pin SIMM sockets (as used on early to mid 90's PC motherboards)
                        The SIMMs themselves are custom. They're not RAM sticks, but instead hold
                        16MBit FlashROMs of type Fujitsu 29F016 TSOP48. Some are standard pinout,
                        some are reverse pinout. This is done to allow for easy PCB wiring on the
                        SIMMs. So far, only SIMMs 1, 3 & 5 are used. 1 & 3 are single sided, have 4
                        FlashROMs on them and hold graphics data that was previously located on the
                        B-Board at locations 1J-4J & 1M-4M.
                        SIMM 5 & 6 can come in two varieties. The more common ones are double-sided
                        and can be populated with up to 8x 16MBit TSOP48 FlashROMs, but only have 2
                        FlashROMs on them (one on each side) and hold the QSound samples that were
                        previously located on the B-Board at locations 1E & 2E.
                        The other type is also double sided and holds up to 8x 16MBit TSOP56 FlashROMs,
                        and again has only 2 positions populated (both on the same side). Either type
                        of QSound SIMM can be used but the data is interleaved differently because of
                        the position of the FlashROM on the SIMM.
                        So far, the data held on the SIMMs is equal to or less than the ROM capacity of
                        the sockets on the B-Board, so the use of SIMMs is a mystery.
                        Some possible explanations are their use is a cost-cutting measure, or they're
                        more easily sourced from the supplier rather than using older 42-pin MASKROMs.
                        Another possibility is they are being re-used from left-over CPS3 boards, since
                        they're identical and are easily re-programmable. In comparision, the GFX SIMMs
                        are the same type as used in CPS3 boards for the main program and the QSound
                        SIMMs are the same type as used in CPS3 boards for the GFX data, but are only
                        populated with 2 FlashROMs.

                        Example SIMM Layout -
                          |----------------------------------------------------|
                          |                                                    |
                          |   |-------|   |-------|   |-------|   |-------|    |
                          |   |Flash_A|   |Flash_B|   |Flash_C|   |Flash_D|    |
                          |   |-------|   |-------|   |-------|   |-------|    |
                          |-                                                   |
                           |-------------------------/\------------------------|
                           Notes:
                                  Flash_A & Flash_C and regular pinout type Fujitsu 29F016A-90PFTN and
                                  for the TSOP56 SIMMs, the FlashROMs are type HN98F1600T10
                                  Flash_B & Flash_D are reverse pinout (Fujitsu 29F016A-90PFTR)
                                  and are mounted upside down also so that pin1 lines up with the normal
                                  pinout of FlashROMs A & C.
                                  For the TSOP48 QSound SIMMs, the 2 FlashROMs are populated at location D
                                  and C only. C is also located on the other side of the SIMM.
                                  For the TSOP56 QSound SIMMs, the 2 FlashROMs are populated at location A
                                  and C only. Both FlashROMs are located on the same side of the SIMM.


Communication Board

TOURNAMENT CAPCOM 93656D-3
|-------|-|--|-|----------------------------------------------------------|
|       |-|  |-|                                                 BATTERY  |
|      SCN1  SCN2          SCN3                                           |
|                                                                         |
|                                                                         |
|                                                                 8464    |
|    MAX232                                                               |
|                                                                         |
|    PAL                                                                  |
|                                                                         |
|                                                                         |
|    D71051C                                                              |
|                                                                         |
|                                         93C46                           |
|                                                                         |
|-----------------------------------------|              CN1              |
                                          |-------------------------------|
Notes:
      There is sufficient space next to the B-Board to enable this board to plug into the B-Board
      into CN7 and still be fully enclosed inside the housing. The housing has holes in it to allow
      the TX, RX and Register connectors to be accessed without opening the case.
      This board is known to be used with "Super Street Fighter 2 : The Tournament Battle" and some
      yellow rent boards also have this daughter board attached.

      SCN1    - Network Data IN
      SCN2    - Network Data OUT
      SCN3    - 8 Pin location for a 'Register' device, not populated (possibly for an online
                register of high scores)
      CN1     - 64 Pin Connector for connection to B-Board
      8464    - Fujitsu MB8464A-10L 8k x8 SRAM (SOP28)
      MAX232  - Maxim MAX232CPE Dual EIA-232 Driver/Receiver (DIP16)
      PAL1    - MMI PAL16L8 (DIP20, stamped 'SFSRD')
      D71051C - NEC uPD71051C Serial Control Unit (DIP28)
      93C46   - Atmel 93C46 EEPROM (SOIC8)
      BATTERY - This is a location for a battery but it is not populated

***************************************************************************

Known problems with this driver.

  - Rasters are not correctly emulated in places where more than one split happens
    per frame. A known place where this problem happens is during Shuma-Gorath's
    Chaos Dimension super move in both MSH and MSHVSF. The screen should split into
    around 6 or more strips and then scroll the gfx inside those strips up and down
    alternatly (as one stip moves gfx up the next strip moves the gfx down).

  - The network adapter used in Super Street Fighter II: The Tournament Battle is
    not currently emulated though the ports it uses are setup in the memory map.

  - Giga Wing's attract mode seems to loose sync with music. The problem seems to
    happen due to gfx drawing slowing to much when screen colors fade out. This
    problem could be due to the 68k being clocked at 11.8mhz when the hardware
    has a 16mhz crystal on it. Various timing loops show 11.8 being the average
    speed of the cpu and this does run true when comparing emulation and real
    hardware when timing is not based on Vsync (ssf2 and ssf2t for example). It is
    possible that what is slowing the cpu is read/write wait states when accessing
    RAM areas. This would mean that in places where lots of opcodes are being used
    in connetion with data registers only the code would end up running to slow.

  - Giga Wing's sprites are 1 frame out when compared to background scrolling. See
    the explanation above for the most likley cause of this problem.

  - Progear slows down more than it should when compared to real hardware. See
    the explanation above for the most likely cause of this problem.

  - Some Hispanic/Brazil region sets have settings adjustable for a card dispenser.
    Many times, this is defaulted to ON.  Since MAME at this time does not emulate
    this unique dispenser, you will get a "NO CARD" message flashing on the screen
    for these sets unless you enter Service Mode and adjust CONFIGURATION > SYSTEM >
    C. DISPENSER to OFF.  An example of a game which does this is Street Fighter 3
    Alpha.


Driver Note:

  - Any new region sets will need full encryption tables dumped to extract the proper
    keys or they will need to be brute forced. XORs are no longer supported nor wanted.


Stephh's inputs notes (based on some tests on the "parent" set) :

0) All games

  - All inputs have been mapped according to the "test mode"
    (even if some buttons don't physically exist on the machine).
  - Joysticks and buttons can be fully tested.
  - COINn sometimes don't show anything but are OK ingame.
  - Unless I notify something below for some games, there is no extra button !

1) 'mmancp2u'

  - BUTTON3 doesn't physically exist on the machine and has no effect ingame.

2) 'megaman2'

  - BUTTON3 doesn't physically exist on the machine and has no effect ingame.

3) 'mpang'

  - BUTTON2 doesn't physically exist on the machine and can't be seen in the "test mode".
    However, if you map it where it should be, it has the same effect as BUTTON1.

4) 'pzloop2'

  - Whatever your settings are, the paddle can't be tested in the "test mode" !
    I can't tell at the moment if it's an emulation or an ingame bug :(

5) 'dimahoo'

  - BUTTON3 doesn't physically exist on the machine.
    However, it acts like a rapid fire (keep button pressed).

6) 'progear'

  - BUTTON3 acts like a rapid fire (keep button pressed).
    It has to be enabled in the game settings as it is OFF by default.

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "deprecat.h"
#include "machine/eeprom.h"
#include "cpu/m68000/m68000.h"
#include "sound/qsound.h"
#include "sound/okim6295.h" // gigamn2 bootleg

#include "includes/cps1.h"       /* External CPS1 definitions */


/*************************************
 *
 *  Constants
 *
 *************************************/

/* Maximum size of Q Sound Z80 region */
#define QSOUND_SIZE 0x50000

/* Maximum 680000 code size */
#undef  CODE_SIZE
#define CODE_SIZE   0x0400000


/*************************************
 *
 *  Interrupt generation
 *
 *************************************/

static INTERRUPT_GEN( cps2_interrupt )
{
	cps_state *state = (cps_state *)device->machine->driver_data;

	/* 2 is vblank, 4 is some sort of scanline interrupt, 6 is both at the same time. */
	if (state->scancount >= 258)
	{
		state->scancount = -1;
		state->scancalls = 0;
	}
	state->scancount++;

	if (state->cps_b_regs[0x10 / 2] & 0x8000)
		state->cps_b_regs[0x10 / 2] &= 0x1ff;

	if (state->cps_b_regs[0x12 / 2] & 0x8000)
		state->cps_b_regs[0x12 / 2] &= 0x1ff;

//  popmessage("%04x %04x - %04x %04x",state->scanline1,state->scanline2,state->cps_b_regs[0x10/2],state->cps_b_regs[0x12/2]);

	/* raster effects */
	if (state->scanline1 == state->scancount || (state->scanline1 < state->scancount && !state->scancalls))
	{
		state->cps_b_regs[0x10/2] = 0;
		cpu_set_input_line(device, 4, HOLD_LINE);
		cps2_set_sprite_priorities(device->machine);
		device->machine->primary_screen->update_partial(16 - 10 + state->scancount);	/* visarea.min_y - [first visible line?] + scancount */
		state->scancalls++;
//          popmessage("IRQ4 scancounter = %04i", state->scancount);
	}

	/* raster effects */
	if(state->scanline2 == state->scancount || (state->scanline2 < state->scancount && !state->scancalls))
	{
		state->cps_b_regs[0x12 / 2] = 0;
		cpu_set_input_line(device, 4, HOLD_LINE);
		cps2_set_sprite_priorities(device->machine);
		device->machine->primary_screen->update_partial(16 - 10 + state->scancount);	/* visarea.min_y - [first visible line?] + scancount */
		state->scancalls++;
//          popmessage("IRQ4 scancounter = %04i",scancount);
	}

	if (state->scancount == 240)  /* VBlank */
	{
		state->cps_b_regs[0x10 / 2] = state->scanline1;
		state->cps_b_regs[0x12 / 2] = state->scanline2;
		cpu_set_input_line(device, 2, HOLD_LINE);
		if(state->scancalls)
		{
			cps2_set_sprite_priorities(device->machine);
			device->machine->primary_screen->update_partial(256);
		}
		cps2_objram_latch(device->machine);
	}
	//popmessage("Raster calls = %i", state->scancalls);
}


/*************************************
 *
 *  EEPROM
 *
 *************************************/

static const eeprom_interface cps2_eeprom_interface =
{
	6,		/* address bits */
	16,		/* data bits */
	"0110",	/*  read command */
	"0101",	/* write command */
	"0111"	/* erase command */
};

static WRITE16_HANDLER( cps2_eeprom_port_w )
{
	cps_state *state = (cps_state *)space->machine->driver_data;

	if (ACCESSING_BITS_8_15)
	{
		/* bit 0 - Unused */
		/* bit 1 - Unused */
		/* bit 2 - Unused */
		/* bit 3 - Unused? */
		/* bit 4 - Eeprom data  */
		/* bit 5 - Eeprom clock */
		/* bit 6 - */
		/* bit 7 - */

		/* EEPROM */
		input_port_write(space->machine, "EEPROMOUT", data, 0xffff);
	}

	if (ACCESSING_BITS_0_7)
	{
		/* bit 0 - coin counter 1 */
		/* bit 0 - coin counter 2 */
		/* bit 2 - Unused */
		/* bit 3 - Allows access to Z80 address space (Z80 reset) */
		/* bit 4 - lock 1  */
		/* bit 5 - lock 2  */
		/* bit 6 - */
		/* bit 7 - */

	        /* Z80 Reset */
		if (state->audiocpu != NULL)
			cpu_set_input_line(state->audiocpu, INPUT_LINE_RESET, (data & 0x0008) ? CLEAR_LINE : ASSERT_LINE);

		coin_counter_w(space->machine, 0, data & 0x0001);
		if ((strncmp(space->machine->gamedrv->name, "pzloop2", 8) == 0) ||
		    (strncmp(space->machine->gamedrv->name, "pzloop2j", 8) == 0))
		{
			// Puzz Loop 2 uses coin counter 2 input to switch between stick and paddle controls
			state->readpaddle = data & 0x0002;
		}
		else
		{
			coin_counter_w(space->machine, 1, data & 0x0002);
		}

		if (strncmp(space->machine->gamedrv->name, "mmatrix", 7) == 0)		// Mars Matrix seems to require the coin lockout bit to be reversed
		{
			coin_lockout_w(space->machine, 0, data & 0x0010);
			coin_lockout_w(space->machine, 1, data & 0x0020);
			coin_lockout_w(space->machine, 2, data & 0x0040);
			coin_lockout_w(space->machine, 3, data & 0x0080);
		}
		else
		{
			coin_lockout_w(space->machine, 0, ~data & 0x0010);
			coin_lockout_w(space->machine, 1, ~data & 0x0020);
			coin_lockout_w(space->machine, 2, ~data & 0x0040);
			coin_lockout_w(space->machine, 3, ~data & 0x0080);
		}

		/*
        set_led_status(space->machine, 0, data & 0x01);
        set_led_status(space->machine, 1, data & 0x10);
        set_led_status(space->machine, 2, data & 0x20);
        */
    }
}


/*************************************
 *
 *  Sound ?
 *
 *************************************/

static READ16_HANDLER( cps2_qsound_volume_r )
{
	cps_state *state = (cps_state *)space->machine->driver_data;

	/* Extra adapter memory (0x660000-0x663fff) available when bit 14 = 0 */
	/* Network adapter (ssf2tb) present when bit 15 = 0 */
	/* Only game known to use both these so far is SSF2TB */

	if (state->cps2networkpresent)
		return 0x2021;
	else
		return 0xe021;
}


/*************************************
 *
 *  Read handlers
 *
 *************************************/

static READ16_HANDLER( kludge_r )
{
	return 0xffff;
}

static READ16_HANDLER( joy_or_paddle_r )
{
	cps_state *state = (cps_state *)space->machine->driver_data;

	if (state->readpaddle != 0)
		return (input_port_read(space->machine, "IN0"));
	else
		return (input_port_read(space->machine, "PADDLE1") & 0xff) | (input_port_read(space->machine, "PADDLE2") << 8);
}


/*************************************
 *
 *  Memory map
 *
 *************************************/

static ADDRESS_MAP_START( cps2_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x3fffff) AM_ROM																			/* 68000 ROM */
	AM_RANGE(0x400000, 0x40000b) AM_RAM AM_BASE_SIZE_MEMBER(cps_state, output, output_size)						/* CPS2 object output */
	AM_RANGE(0x618000, 0x619fff) AM_READWRITE(qsound_sharedram1_r, qsound_sharedram1_w)     					/* Q RAM */
	AM_RANGE(0x662000, 0x662001) AM_RAM																			/* Network adapter related, accessed in SSF2TB */
	AM_RANGE(0x662008, 0x662009) AM_RAM																			/* Network adapter related, accessed in SSF2TB */
	AM_RANGE(0x662020, 0x662021) AM_RAM																			/* Network adapter related, accessed in SSF2TB */
	AM_RANGE(0x660000, 0x663fff) AM_RAM																			/* When bit 14 of 0x804030 equals 0 this space is available. Many games store highscores and other info here if available. */
	AM_RANGE(0x664000, 0x664001) AM_RAM																			/* Unknown - Only used if 0x660000-0x663fff available (could be RAM enable?) */
	AM_RANGE(0x700000, 0x701fff) AM_WRITE(cps2_objram1_w) AM_BASE_MEMBER(cps_state, objram1)    							/* Object RAM, no game seems to use it directly */
	AM_RANGE(0x708000, 0x709fff) AM_READWRITE(cps2_objram2_r, cps2_objram2_w) AM_BASE_MEMBER(cps_state, objram2)			/* Object RAM */
	AM_RANGE(0x70a000, 0x70bfff) AM_READWRITE(cps2_objram2_r, cps2_objram2_w) AM_BASE_MEMBER(cps_state, objram2)			/* mirror */
	AM_RANGE(0x70c000, 0x70dfff) AM_READWRITE(cps2_objram2_r, cps2_objram2_w) AM_BASE_MEMBER(cps_state, objram2)			/* mirror */
	AM_RANGE(0x70e000, 0x70ffff) AM_READWRITE(cps2_objram2_r, cps2_objram2_w) AM_BASE_MEMBER(cps_state, objram2)			/* mirror */
	AM_RANGE(0x800100, 0x80013f) AM_WRITE(cps1_cps_a_w) AM_BASE_MEMBER(cps_state, cps_a_regs)								/* mirror (sfa) */
	AM_RANGE(0x800140, 0x80017f) AM_READWRITE(cps1_cps_b_r, cps1_cps_b_w) AM_BASE_MEMBER(cps_state, cps_b_regs) 			/* mirror (sfa) */
	AM_RANGE(0x804000, 0x804001) AM_READ_PORT("IN0")                											/* IN0 */
	AM_RANGE(0x804010, 0x804011) AM_READ_PORT("IN1")                											/* IN1 */
	AM_RANGE(0x804020, 0x804021) AM_READ_PORT("IN2")                											/* IN2 + EEPROM */
	AM_RANGE(0x804030, 0x804031) AM_READ(cps2_qsound_volume_r)      											/* Master volume. Also when bit 14=0 addon memory is present, when bit 15=0 network adapter present. */
	AM_RANGE(0x804040, 0x804041) AM_WRITE(cps2_eeprom_port_w)													/* EEPROM */
	AM_RANGE(0x8040a0, 0x8040a1) AM_WRITENOP                													/* Unknown (reset once on startup) */
	AM_RANGE(0x8040b0, 0x8040b3) AM_READ(kludge_r)																/* unknown (xmcotaj hangs if this is 0) */
	AM_RANGE(0x8040e0, 0x8040e1) AM_WRITE(cps2_objram_bank_w)													/* bit 0 = Object ram bank swap */
	AM_RANGE(0x804100, 0x80413f) AM_WRITE(cps1_cps_a_w) AM_BASE_MEMBER(cps_state, cps_a_regs)							/* CPS-A custom */
	AM_RANGE(0x804140, 0x80417f) AM_READWRITE(cps1_cps_b_r, cps1_cps_b_w)           							/* CPS-B custom */
	AM_RANGE(0x900000, 0x92ffff) AM_RAM_WRITE(cps1_gfxram_w) AM_BASE_SIZE_MEMBER(cps_state, gfxram, gfxram_size)	/* Video RAM */
	AM_RANGE(0xff0000, 0xffffff) AM_RAM																			/* RAM */
ADDRESS_MAP_END


static ADDRESS_MAP_START( dead_cps2_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x3fffff) AM_ROM																			/* 68000 ROM */
	AM_RANGE(0x400000, 0x40000b) AM_RAM AM_BASE_SIZE_MEMBER(cps_state, output, output_size)						/* CPS2 object output */
	AM_RANGE(0x618000, 0x619fff) AM_READWRITE(qsound_sharedram1_r, qsound_sharedram1_w)     					/* Q RAM */
	AM_RANGE(0x662000, 0x662001) AM_RAM																			/* Network adapter related, accessed in SSF2TB */
	AM_RANGE(0x662008, 0x662009) AM_RAM																			/* Network adapter related, accessed in SSF2TB */
	AM_RANGE(0x662020, 0x662021) AM_RAM																			/* Network adapter related, accessed in SSF2TB */
	AM_RANGE(0x660000, 0x663fff) AM_RAM																			/* When bit 14 of 0x804030 equals 0 this space is available. Many games store highscores and other info here if available. */
	AM_RANGE(0x664000, 0x664001) AM_RAM																			/* Unknown - Only used if 0x660000-0x663fff available (could be RAM enable?) */
	AM_RANGE(0x700000, 0x701fff) AM_WRITE(cps2_objram1_w) AM_BASE_MEMBER(cps_state, objram1)    							/* Object RAM, no game seems to use it directly */
	AM_RANGE(0x708000, 0x709fff) AM_READWRITE(cps2_objram2_r, cps2_objram2_w) AM_BASE_MEMBER(cps_state, objram2)			/* Object RAM */
	AM_RANGE(0x70a000, 0x70bfff) AM_READWRITE(cps2_objram2_r, cps2_objram2_w) AM_BASE_MEMBER(cps_state, objram2)			/* mirror */
	AM_RANGE(0x70c000, 0x70dfff) AM_READWRITE(cps2_objram2_r, cps2_objram2_w) AM_BASE_MEMBER(cps_state, objram2)			/* mirror */
	AM_RANGE(0x70e000, 0x70ffff) AM_READWRITE(cps2_objram2_r, cps2_objram2_w) AM_BASE_MEMBER(cps_state, objram2)			/* mirror */
	AM_RANGE(0x800100, 0x80013f) AM_WRITE(cps1_cps_a_w) AM_BASE_MEMBER(cps_state, cps_a_regs)								/* mirror (sfa) */
	AM_RANGE(0x800140, 0x80017f) AM_READWRITE(cps1_cps_b_r, cps1_cps_b_w) AM_BASE_MEMBER(cps_state, cps_b_regs) 			/* mirror (sfa) */
	AM_RANGE(0x804000, 0x804001) AM_READ_PORT("IN0")                											/* IN0 */
	AM_RANGE(0x804010, 0x804011) AM_READ_PORT("IN1")                											/* IN1 */
	AM_RANGE(0x804020, 0x804021) AM_READ_PORT("IN2")                											/* IN2 + EEPROM */
	AM_RANGE(0x804030, 0x804031) AM_READ(cps2_qsound_volume_r)      											/* Master volume. Also when bit 14=0 addon memory is present, when bit 15=0 network adapter present. */
	AM_RANGE(0x804040, 0x804041) AM_WRITE(cps2_eeprom_port_w)													/* EEPROM */
	AM_RANGE(0x8040a0, 0x8040a1) AM_WRITENOP                													/* Unknown (reset once on startup) */
	AM_RANGE(0x8040b0, 0x8040b3) AM_READ(kludge_r)																/* unknown (xmcotaj hangs if this is 0) */
	AM_RANGE(0x8040e0, 0x8040e1) AM_WRITE(cps2_objram_bank_w)													/* bit 0 = Object ram bank swap */
	AM_RANGE(0x804100, 0x80413f) AM_WRITE(cps1_cps_a_w) AM_BASE_MEMBER(cps_state, cps_a_regs)								/* CPS-A custom */
	AM_RANGE(0x804140, 0x80417f) AM_READWRITE(cps1_cps_b_r, cps1_cps_b_w)           							/* CPS-B custom */
	AM_RANGE(0x900000, 0x92ffff) AM_RAM_WRITE(cps1_gfxram_w) AM_BASE_SIZE_MEMBER(cps_state, gfxram, gfxram_size)	/* Video RAM */
	AM_RANGE(0xff0000, 0xffffef) AM_RAM																			/* RAM */
	AM_RANGE(0xfffff0, 0xfffffb) AM_RAM AM_BASE_SIZE_MEMBER(cps_state, output, output_size)						/* CPS2 output */
ADDRESS_MAP_END


/*************************************
 *
 *  Generic port definitions
 *
 *************************************/

/* 4 players and 4 buttons */
static INPUT_PORTS_START( cps2_4p4b )
	PORT_START("IN0")      /* (0x00) */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)

	PORT_START("IN1")      /* (0x10) */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(3)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(4)

	PORT_START("IN2")      /* (0x20) */
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE("eeprom", eeprom_read_bit)
	PORT_SERVICE_NO_TOGGLE( 0x0002, IP_ACTIVE_LOW )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x00f8, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN4 )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_write_bit)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_set_clock_line)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_set_cs_line)
INPUT_PORTS_END

/* 4 players and 3 buttons */
static INPUT_PORTS_START( cps2_4p3b )
	PORT_INCLUDE(cps2_4p4b)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )           /* BUTTON4 PORT_PLAYER(1) */
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )           /* BUTTON4 PORT_PLAYER(2) */

	PORT_MODIFY("IN1")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )           /* BUTTON4 PORT_PLAYER(3) */
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )           /* BUTTON4 PORT_PLAYER(4) */
INPUT_PORTS_END

/* 4 players and 2 buttons */
static INPUT_PORTS_START( cps2_4p2b )
	PORT_INCLUDE(cps2_4p3b)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )           /* BUTTON3 PORT_PLAYER(1) */
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )           /* BUTTON3 PORT_PLAYER(2) */

	PORT_MODIFY("IN1")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )           /* BUTTON3 PORT_PLAYER(3) */
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )           /* BUTTON3 PORT_PLAYER(4) */
INPUT_PORTS_END

/* 3 players and 4 buttons */
static INPUT_PORTS_START( cps2_3p4b )
	PORT_INCLUDE(cps2_4p4b)

	PORT_MODIFY("IN1")
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )           /* PORT_PLAYER(4) inputs */

	PORT_MODIFY("IN2")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )           /* START4 */
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )           /* COIN4 */
INPUT_PORTS_END

/* 3 players and 3 buttons */
static INPUT_PORTS_START( cps2_3p3b )
	PORT_INCLUDE(cps2_3p4b)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )           /* BUTTON4 PORT_PLAYER(1) */
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )           /* BUTTON4 PORT_PLAYER(2) */

	PORT_MODIFY("IN1")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )           /* BUTTON4 PORT_PLAYER(3) */
INPUT_PORTS_END

/* 3 players and 2 buttons */
#ifdef UNUSED_DEFINITION
static INPUT_PORTS_START( cps2_3p2b )
	PORT_INCLUDE(cps2_3p3b)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )           /* BUTTON3 PORT_PLAYER(1) */
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )           /* BUTTON3 PORT_PLAYER(2) */

	PORT_MODIFY("IN1")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )           /* BUTTON3 PORT_PLAYER(3) */
INPUT_PORTS_END
#endif

/* 2 players and 4 buttons */
static INPUT_PORTS_START( cps2_2p4b )
	PORT_INCLUDE(cps2_3p4b)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )           /* PORT_PLAYER(3) inputs */

	PORT_MODIFY("IN2")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )           /* START3 */
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )           /* COIN3 */
INPUT_PORTS_END

/* 2 players and 3 buttons */
static INPUT_PORTS_START( cps2_2p3b )
	PORT_INCLUDE(cps2_2p4b)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )           /* BUTTON4 PORT_PLAYER(1) */
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )           /* BUTTON4 PORT_PLAYER(2) */
INPUT_PORTS_END

/* 2 players and 2 buttons */
static INPUT_PORTS_START( cps2_2p2b )
	PORT_INCLUDE(cps2_2p3b)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )           /* BUTTON3 PORT_PLAYER(1) */
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )           /* BUTTON3 PORT_PLAYER(2) */
INPUT_PORTS_END

/* 2 players and 1 button */
static INPUT_PORTS_START( cps2_2p1b )
	PORT_INCLUDE(cps2_2p2b)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )           /* BUTTON2 PORT_PLAYER(1) */
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNUSED )           /* BUTTON2 PORT_PLAYER(2) */
INPUT_PORTS_END

/* 2 players and 6 buttons (2 rows of 3 buttons) */
static INPUT_PORTS_START( cps2_2p6b )
	PORT_INCLUDE(cps2_2p3b)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)

	PORT_MODIFY("IN2")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
INPUT_PORTS_END

/* 1 player and 4 buttons */
static INPUT_PORTS_START( cps2_1p4b )
	PORT_INCLUDE(cps2_2p4b)

	PORT_MODIFY("IN0")
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )           /* PORT_PLAYER(2) inputs */

	PORT_MODIFY("IN2")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNUSED )           /* START2 */
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNUSED )           /* COIN2 */
INPUT_PORTS_END

/* 1 player and 3 buttons */
static INPUT_PORTS_START( cps2_1p3b )
	PORT_INCLUDE(cps2_1p4b)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )           /* BUTTON4 PORT_PLAYER(1) */
INPUT_PORTS_END

/* 1 player and 2 buttons */
static INPUT_PORTS_START( cps2_1p2b )
	PORT_INCLUDE(cps2_1p3b)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )           /* BUTTON3 PORT_PLAYER(1) */
INPUT_PORTS_END


/*************************************
 *
 *  Game-specific port definitions
 *
 *************************************/

/* According to the "test mode", buttons layout look like a 2 players 6 buttons machine where buttons have been removed */
static INPUT_PORTS_START( cybots )
	PORT_INCLUDE(cps2_2p6b)

	PORT_MODIFY("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNUSED )                     /* BUTTON5 PORT_PLAYER(1) */
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNUSED )                     /* BUTTON6 PORT_PLAYER(1) */
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )                     /* BUTTON5 PORT_PLAYER(2) */

	PORT_MODIFY("IN2")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )                     /* BUTTON6 PORT_PLAYER(2) */
INPUT_PORTS_END

/* 2 players, no joysticks which are replaced with 4 buttons, no other buttons */
static INPUT_PORTS_START( qndream )
	PORT_INCLUDE(cps2_2p4b)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x00f0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

/* 2 players and 1 button - either 2 8-way joysticks, 2 2-way joysticks, or 2 paddles */
static INPUT_PORTS_START( pzloop2 )
	PORT_INCLUDE(cps2_2p1b)

	PORT_START("PADDLE1")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("PADDLE2")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(15) PORT_PLAYER(2)
INPUT_PORTS_END

/* 1 player and 3 buttons, but 2 coins slots */
static INPUT_PORTS_START( choko )
	PORT_INCLUDE(cps2_1p3b)

	PORT_MODIFY("IN2")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_COIN2 )
INPUT_PORTS_END


/*************************************
 *
 *  Graphics layouts
 *
 *************************************/

static const gfx_layout cps1_layout8x8 =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 1*4, 0*4, 3*4, 2*4, 5*4, 4*4, 7*4, 6*4 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64 },
	64*8
};

static const gfx_layout cps1_layout8x8_2 =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 9*4, 8*4, 11*4, 10*4, 13*4, 12*4, 15*4, 14*4 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64 },
	64*8
};

static GFXLAYOUT_RAW( layout16x16, 4, 16, 16, 8*8, 128*8 )
static GFXLAYOUT_RAW( layout32x32, 4, 32, 32, 16*8, 512*8 )

static GFXDECODE_START( cps2 )
	GFXDECODE_ENTRY( "gfx", 0, cps1_layout8x8,   0, 0x100 )
	GFXDECODE_ENTRY( "gfx", 0, cps1_layout8x8_2, 0, 0x100 )
	GFXDECODE_ENTRY( "gfx", 0, layout16x16, 0, 0x100 )
	GFXDECODE_ENTRY( "gfx", 0, layout32x32, 0, 0x100 )
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_START( cps2 )
{
	cps_state *state = (cps_state *)machine->driver_data;

	state->maincpu = machine->device("maincpu");
	state->audiocpu = machine->device("audiocpu");

	state_save_register_global(machine, state->scancount);

	if (state->audiocpu != NULL)	// gigamn2 has no audiocpu
		memory_configure_bank(machine, "bank1", 0, (QSOUND_SIZE - 0x10000) / 0x4000, memory_region(machine, "audiocpu") + 0x10000, 0x4000);
}


static MACHINE_DRIVER_START( cps2 )

	/* driver data */
	MDRV_DRIVER_DATA(cps_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, XTAL_16MHz)
	MDRV_CPU_PROGRAM_MAP(cps2_map)
	MDRV_CPU_VBLANK_INT_HACK(cps2_interrupt,259)	// 262  /* ??? interrupts per frame */

	MDRV_CPU_ADD("audiocpu", Z80, 8000000)
	MDRV_CPU_PROGRAM_MAP(qsound_sub_map)
	MDRV_CPU_PERIODIC_INT(irq0_line_hold, 251)	/* 251 is good (see 'mercy mercy mercy'section of sgemf attract mode for accurate sound sync */

	MDRV_MACHINE_START(cps2)

	MDRV_EEPROM_ADD("eeprom", cps2_eeprom_interface)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_RAW_PARAMS(XTAL_8MHz, 518, 64, 448, 259, 16, 240)
/*
    Measured clocks:
        V = 59.6376Hz
        H = 15.4445kHz
        H/V = 258.973 ~ 259 lines

    Possible video clocks:
        60MHz / 15.4445kHz = 3884.878 / 8 = 485.610 -> unlikely
         8MHz / 15.4445kHz =  517.983 ~ 518 -> likely
        16MHz -> same as 8 but with a /2 divider; also a possibility
*/
	MDRV_GFXDECODE(cps2)
	MDRV_PALETTE_LENGTH(0xc00)

	MDRV_VIDEO_START(cps2)
	MDRV_VIDEO_EOF(cps1)
	MDRV_VIDEO_UPDATE(cps1)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("qsound", QSOUND, QSOUND_CLOCK)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( dead_cps2 )
	MDRV_IMPORT_FROM(cps2)

	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(dead_cps2_map)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( gigamn2 )
	MDRV_IMPORT_FROM(cps2)

	MDRV_DEVICE_REMOVE("audiocpu")

	MDRV_DEVICE_REMOVE("qsound")

	MDRV_OKIM6295_ADD("oki", XTAL_32MHz/32, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.47)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.47)
MACHINE_DRIVER_END

/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( 1944 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "nffu.03", 0x000000, 0x80000, CRC(9693cf8f) SHA1(c296cb008e282f77b44374d1c3638a3f4d5d5d4e) )
	ROM_LOAD16_WORD_SWAP( "nff.04",  0x080000, 0x80000, CRC(dba1c66e) SHA1(4764e77d4da5d19d9acded27df1e1bcba06b0fcf) )
	ROM_LOAD16_WORD_SWAP( "nffu.05", 0x100000, 0x80000, CRC(ea813eb7) SHA1(34e0175a5f22d08c3538369b4bfd077a7427a128) )

	ROM_REGION( 0x1400000, "gfx", 0 )
	ROMX_LOAD( "nff.13m",   0x0000000, 0x400000, CRC(c9fca741) SHA1(1781d4fc18b6d6f79b7b39d9bcace750fb61a5cf) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "nff.15m",   0x0000002, 0x400000, CRC(f809d898) SHA1(a0b6af49e1780678d808c317b875161cedddb314) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "nff.17m",   0x0000004, 0x400000, CRC(15ba4507) SHA1(bed6a82bf1dc1aa501d4c2d098115a15e18d446a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "nff.19m",   0x0000006, 0x400000, CRC(3dd41b8c) SHA1(676078baad789e25f6e5a79de29672587be7ff00) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "nff.14m",   0x1000000, 0x100000, CRC(3fe3a54b) SHA1(0a8e5cae141d24fd8b3cb11796c44728b0acd69e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "nff.16m",   0x1000002, 0x100000, CRC(565cd231) SHA1(0aecd433fb4ca2de1aca9fbb1e314fb1f6979321) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "nff.18m",   0x1000004, 0x100000, CRC(63ca5988) SHA1(30137fa77573c84bcc24570bccb7dba61ddb413c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "nff.20m",   0x1000006, 0x100000, CRC(21eb8f3b) SHA1(efa69f19a958047dd91a294c88857ed3133fcbef) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "nff.01",   0x00000, 0x08000, CRC(d2e44318) SHA1(33e45f6fe9fed098a4c072b8c39406aef1a949b2) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "nff.11m",   0x000000, 0x400000, CRC(243e4e05) SHA1(83281f7290ac105a3f9a7507cbc11317d45ba706) )
	ROM_LOAD16_WORD_SWAP( "nff.12m",   0x400000, 0x400000, CRC(4fcf1600) SHA1(36f18c5d92b79433bdf7088b29a244708929d48e) )
ROM_END

ROM_START( 1944j )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "nffj.03", 0x000000, 0x80000, CRC(247521ef) SHA1(c6a04f514dd5ab40d8813dcfb8430bce54e7aa28) )
	ROM_LOAD16_WORD_SWAP( "nff.04",  0x080000, 0x80000, CRC(dba1c66e) SHA1(4764e77d4da5d19d9acded27df1e1bcba06b0fcf) )
	ROM_LOAD16_WORD_SWAP( "nffj.05", 0x100000, 0x80000, CRC(7f20c2ef) SHA1(380dc54d94c29c049a4c00ed58013e04eec87086) )

	ROM_REGION( 0x1400000, "gfx", 0 )
	ROMX_LOAD( "nff.13m",   0x0000000, 0x400000, CRC(c9fca741) SHA1(1781d4fc18b6d6f79b7b39d9bcace750fb61a5cf) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "nff.15m",   0x0000002, 0x400000, CRC(f809d898) SHA1(a0b6af49e1780678d808c317b875161cedddb314) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "nff.17m",   0x0000004, 0x400000, CRC(15ba4507) SHA1(bed6a82bf1dc1aa501d4c2d098115a15e18d446a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "nff.19m",   0x0000006, 0x400000, CRC(3dd41b8c) SHA1(676078baad789e25f6e5a79de29672587be7ff00) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "nff.14m",   0x1000000, 0x100000, CRC(3fe3a54b) SHA1(0a8e5cae141d24fd8b3cb11796c44728b0acd69e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "nff.16m",   0x1000002, 0x100000, CRC(565cd231) SHA1(0aecd433fb4ca2de1aca9fbb1e314fb1f6979321) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "nff.18m",   0x1000004, 0x100000, CRC(63ca5988) SHA1(30137fa77573c84bcc24570bccb7dba61ddb413c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "nff.20m",   0x1000006, 0x100000, CRC(21eb8f3b) SHA1(efa69f19a958047dd91a294c88857ed3133fcbef) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "nff.01",   0x00000, 0x08000, CRC(d2e44318) SHA1(33e45f6fe9fed098a4c072b8c39406aef1a949b2) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "nff.11m",   0x000000, 0x400000, CRC(243e4e05) SHA1(83281f7290ac105a3f9a7507cbc11317d45ba706) )
	ROM_LOAD16_WORD_SWAP( "nff.12m",   0x400000, 0x400000, CRC(4fcf1600) SHA1(36f18c5d92b79433bdf7088b29a244708929d48e) )
ROM_END

ROM_START( 19xx )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "19xu.03", 0x000000, 0x80000, CRC(05955268) SHA1(d3b6b416f1f9eb1c1cbca6647630d1155647082d) )
	ROM_LOAD16_WORD_SWAP( "19xu.04", 0x080000, 0x80000, CRC(3111ab7f) SHA1(8bbce20ae7ba47949f4939b2f35014fb6decd283) )
	ROM_LOAD16_WORD_SWAP( "19xu.05", 0x100000, 0x80000, CRC(38df4a63) SHA1(1303f7ab6296f1454907a24d64878bdbd1ef88a7) )
	ROM_LOAD16_WORD_SWAP( "19xu.06", 0x180000, 0x80000, CRC(5c7e60d3) SHA1(26bf0936962051be871d7a7776cf78abfca5b5ee) )
	ROM_LOAD16_WORD_SWAP( "19x.07",  0x200000, 0x80000, CRC(61c0296c) SHA1(9e225beccffd14bb53a32f8c0f2aef7f331dae30) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROMX_LOAD( "19x.13m",   0x0000000, 0x080000, CRC(427aeb18) SHA1(901029b5423e4bda85f592735036c06b7d426680) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "19x.15m",   0x0000002, 0x080000, CRC(63bdbf54) SHA1(9beb64ef0a8c92490848599d5d979bf42532609d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "19x.17m",   0x0000004, 0x080000, CRC(2dfe18b5) SHA1(8a44364d9af6b9e1664b44b9235dc172182c9eb8) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "19x.19m",   0x0000006, 0x080000, CRC(cbef9579) SHA1(172413f220b242411218c7865e04014ec6417537) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "19x.14m",   0x0800000, 0x200000, CRC(e916967c) SHA1(3f937022166149a80585f91388de521055ca88ca) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "19x.16m",   0x0800002, 0x200000, CRC(6e75f3db) SHA1(4e1c8466eaa612102d0807d2e8bf1004e97476ea) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "19x.18m",   0x0800004, 0x200000, CRC(2213e798) SHA1(b1a9d5547f3f6c3ab59e8b761d224793c6ca33cb) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "19x.20m",   0x0800006, 0x200000, CRC(ab9d5b96) SHA1(52b755da401fde90c13181b02ab33e5e4b2aa1f7) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "19x.01",   0x00000, 0x08000, CRC(ef55195e) SHA1(813f465f2d392f6abeadbf661c54cf51171fa006) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "19x.11m",   0x000000, 0x200000, CRC(d38beef3) SHA1(134e961b926a97cca5e45d3558efb98f6f278e08) )
	ROM_LOAD16_WORD_SWAP( "19x.12m",   0x200000, 0x200000, CRC(d47c96e2) SHA1(3c1b5563f8e7ee1c450b3592fcb319e928caec3c) )
ROM_END

ROM_START( 19xxj )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "19xj.03a", 0x000000, 0x80000, CRC(ed08bdd1) SHA1(4b49f988faf4a6a99d3596bb12e4685862a20a3e) )
	ROM_LOAD16_WORD_SWAP( "19xj.04a", 0x080000, 0x80000, CRC(fb8e3f29) SHA1(513b85bfe5b86692faa4d20b755fe261a7f95bfc) )
	ROM_LOAD16_WORD_SWAP( "19xj.05a", 0x100000, 0x80000, CRC(aa508ac4) SHA1(c906f4a92872f4ecda662146690acbe5165ae79e) )
	ROM_LOAD16_WORD_SWAP( "19xj.06a", 0x180000, 0x80000, CRC(ff2d785b) SHA1(9294fb3ed378ecc9dccdeff05df09cbb4eeaa9b3) )
	ROM_LOAD16_WORD_SWAP( "19x.07",   0x200000, 0x80000, CRC(61c0296c) SHA1(9e225beccffd14bb53a32f8c0f2aef7f331dae30) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROMX_LOAD( "19x.13m",   0x0000000, 0x080000, CRC(427aeb18) SHA1(901029b5423e4bda85f592735036c06b7d426680) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "19x.15m",   0x0000002, 0x080000, CRC(63bdbf54) SHA1(9beb64ef0a8c92490848599d5d979bf42532609d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "19x.17m",   0x0000004, 0x080000, CRC(2dfe18b5) SHA1(8a44364d9af6b9e1664b44b9235dc172182c9eb8) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "19x.19m",   0x0000006, 0x080000, CRC(cbef9579) SHA1(172413f220b242411218c7865e04014ec6417537) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "19x.14m",   0x0800000, 0x200000, CRC(e916967c) SHA1(3f937022166149a80585f91388de521055ca88ca) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "19x.16m",   0x0800002, 0x200000, CRC(6e75f3db) SHA1(4e1c8466eaa612102d0807d2e8bf1004e97476ea) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "19x.18m",   0x0800004, 0x200000, CRC(2213e798) SHA1(b1a9d5547f3f6c3ab59e8b761d224793c6ca33cb) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "19x.20m",   0x0800006, 0x200000, CRC(ab9d5b96) SHA1(52b755da401fde90c13181b02ab33e5e4b2aa1f7) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "19x.01",   0x00000, 0x08000, CRC(ef55195e) SHA1(813f465f2d392f6abeadbf661c54cf51171fa006) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "19x.11m",   0x000000, 0x200000, CRC(d38beef3) SHA1(134e961b926a97cca5e45d3558efb98f6f278e08) )
	ROM_LOAD16_WORD_SWAP( "19x.12m",   0x200000, 0x200000, CRC(d47c96e2) SHA1(3c1b5563f8e7ee1c450b3592fcb319e928caec3c) )
ROM_END

ROM_START( 19xxjr1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "19xj.03", 0x000000, 0x80000, CRC(26a381ed) SHA1(9a6bd9a8c152096f653c0b5a161dd08314fdb5e7) )
	ROM_LOAD16_WORD_SWAP( "19xj.04", 0x080000, 0x80000, CRC(30100cca) SHA1(3fc964e6daffa5dd7b9f72c8ace3a4b9d515e9ce) )
	ROM_LOAD16_WORD_SWAP( "19xj.05", 0x100000, 0x80000, CRC(de67e938) SHA1(5f977c07c6ffa816ccfa2c7bab8a77b64c232610) )
	ROM_LOAD16_WORD_SWAP( "19xj.06", 0x180000, 0x80000, CRC(39f9a409) SHA1(45799204d2400a591c526f8c750e4728701372bf) )
	ROM_LOAD16_WORD_SWAP( "19x.07",  0x200000, 0x80000, CRC(61c0296c) SHA1(9e225beccffd14bb53a32f8c0f2aef7f331dae30) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROMX_LOAD( "19x.13m",   0x0000000, 0x080000, CRC(427aeb18) SHA1(901029b5423e4bda85f592735036c06b7d426680) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "19x.15m",   0x0000002, 0x080000, CRC(63bdbf54) SHA1(9beb64ef0a8c92490848599d5d979bf42532609d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "19x.17m",   0x0000004, 0x080000, CRC(2dfe18b5) SHA1(8a44364d9af6b9e1664b44b9235dc172182c9eb8) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "19x.19m",   0x0000006, 0x080000, CRC(cbef9579) SHA1(172413f220b242411218c7865e04014ec6417537) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "19x.14m",   0x0800000, 0x200000, CRC(e916967c) SHA1(3f937022166149a80585f91388de521055ca88ca) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "19x.16m",   0x0800002, 0x200000, CRC(6e75f3db) SHA1(4e1c8466eaa612102d0807d2e8bf1004e97476ea) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "19x.18m",   0x0800004, 0x200000, CRC(2213e798) SHA1(b1a9d5547f3f6c3ab59e8b761d224793c6ca33cb) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "19x.20m",   0x0800006, 0x200000, CRC(ab9d5b96) SHA1(52b755da401fde90c13181b02ab33e5e4b2aa1f7) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "19x.01",   0x00000, 0x08000, CRC(ef55195e) SHA1(813f465f2d392f6abeadbf661c54cf51171fa006) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "19x.11m",   0x000000, 0x200000, CRC(d38beef3) SHA1(134e961b926a97cca5e45d3558efb98f6f278e08) )
	ROM_LOAD16_WORD_SWAP( "19x.12m",   0x200000, 0x200000, CRC(d47c96e2) SHA1(3c1b5563f8e7ee1c450b3592fcb319e928caec3c) )
ROM_END

ROM_START( 19xxa )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "19xa.03", 0x000000, 0x80000, CRC(0c20fd50) SHA1(3aeb698ac67e6c8d0224e68d9258ef45f735432a) )
	ROM_LOAD16_WORD_SWAP( "19xa.04", 0x080000, 0x80000, CRC(1fc37508) SHA1(f4b858b5dc6243c5cd432d1a72d828831c8eca6f) )
	ROM_LOAD16_WORD_SWAP( "19xa.05", 0x100000, 0x80000, CRC(6c9cc4ed) SHA1(2b01ffe0bba41640ffc0c13dfdacf3cf0e3e131d) )
	ROM_LOAD16_WORD_SWAP( "19xa.06", 0x180000, 0x80000, CRC(ca5b9f76) SHA1(961aed25cb445722de5001ba687dbe85b80cba29) )
	ROM_LOAD16_WORD_SWAP( "19x.07",  0x200000, 0x80000, CRC(61c0296c) SHA1(9e225beccffd14bb53a32f8c0f2aef7f331dae30) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROMX_LOAD( "19x.13m",   0x0000000, 0x080000, CRC(427aeb18) SHA1(901029b5423e4bda85f592735036c06b7d426680) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "19x.15m",   0x0000002, 0x080000, CRC(63bdbf54) SHA1(9beb64ef0a8c92490848599d5d979bf42532609d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "19x.17m",   0x0000004, 0x080000, CRC(2dfe18b5) SHA1(8a44364d9af6b9e1664b44b9235dc172182c9eb8) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "19x.19m",   0x0000006, 0x080000, CRC(cbef9579) SHA1(172413f220b242411218c7865e04014ec6417537) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "19x.14m",   0x0800000, 0x200000, CRC(e916967c) SHA1(3f937022166149a80585f91388de521055ca88ca) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "19x.16m",   0x0800002, 0x200000, CRC(6e75f3db) SHA1(4e1c8466eaa612102d0807d2e8bf1004e97476ea) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "19x.18m",   0x0800004, 0x200000, CRC(2213e798) SHA1(b1a9d5547f3f6c3ab59e8b761d224793c6ca33cb) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "19x.20m",   0x0800006, 0x200000, CRC(ab9d5b96) SHA1(52b755da401fde90c13181b02ab33e5e4b2aa1f7) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "19x.01",   0x00000, 0x08000, CRC(ef55195e) SHA1(813f465f2d392f6abeadbf661c54cf51171fa006) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "19x.11m",   0x000000, 0x200000, CRC(d38beef3) SHA1(134e961b926a97cca5e45d3558efb98f6f278e08) )
	ROM_LOAD16_WORD_SWAP( "19x.12m",   0x200000, 0x200000, CRC(d47c96e2) SHA1(3c1b5563f8e7ee1c450b3592fcb319e928caec3c) )
ROM_END

ROM_START( 19xxh )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "19xh.03a", 0x000000, 0x80000, CRC(357be2ac) SHA1(660641d8cd2f7b574809badf99924f0a31a0cccd) )
	ROM_LOAD16_WORD_SWAP( "19xh.04a", 0x080000, 0x80000, CRC(bb13ea3b) SHA1(3ae0fa09ae031e2a0f1ea8645a9baced44289383) )
	ROM_LOAD16_WORD_SWAP( "19xh.05a", 0x100000, 0x80000, CRC(cbd76601) SHA1(a6b64e5f4b35a120dc463a6c9e98e2ec8e739e59) )
	ROM_LOAD16_WORD_SWAP( "19xh.06a", 0x180000, 0x80000, CRC(b362de8b) SHA1(0383a44efbfccdc78637995ed4f99740ef96cbad) )
	ROM_LOAD16_WORD_SWAP( "19x.07",   0x200000, 0x80000, CRC(61c0296c) SHA1(9e225beccffd14bb53a32f8c0f2aef7f331dae30) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROMX_LOAD( "19x.13m",   0x0000000, 0x080000, CRC(427aeb18) SHA1(901029b5423e4bda85f592735036c06b7d426680) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "19x.15m",   0x0000002, 0x080000, CRC(63bdbf54) SHA1(9beb64ef0a8c92490848599d5d979bf42532609d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "19x.17m",   0x0000004, 0x080000, CRC(2dfe18b5) SHA1(8a44364d9af6b9e1664b44b9235dc172182c9eb8) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "19x.19m",   0x0000006, 0x080000, CRC(cbef9579) SHA1(172413f220b242411218c7865e04014ec6417537) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "19x.14m",   0x0800000, 0x200000, CRC(e916967c) SHA1(3f937022166149a80585f91388de521055ca88ca) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "19x.16m",   0x0800002, 0x200000, CRC(6e75f3db) SHA1(4e1c8466eaa612102d0807d2e8bf1004e97476ea) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "19x.18m",   0x0800004, 0x200000, CRC(2213e798) SHA1(b1a9d5547f3f6c3ab59e8b761d224793c6ca33cb) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "19x.20m",   0x0800006, 0x200000, CRC(ab9d5b96) SHA1(52b755da401fde90c13181b02ab33e5e4b2aa1f7) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "19x.01",   0x00000, 0x08000, CRC(ef55195e) SHA1(813f465f2d392f6abeadbf661c54cf51171fa006) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "19x.11m",   0x000000, 0x200000, CRC(d38beef3) SHA1(134e961b926a97cca5e45d3558efb98f6f278e08) )
	ROM_LOAD16_WORD_SWAP( "19x.12m",   0x200000, 0x200000, CRC(d47c96e2) SHA1(3c1b5563f8e7ee1c450b3592fcb319e928caec3c) )
ROM_END

ROM_START( 19xxb )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "19xb.03a", 0x000000, 0x80000, CRC(341bdf4a) SHA1(e7deccd034f2a2664507dbb7fed9757c2424dbf7) )
	ROM_LOAD16_WORD_SWAP( "19xb.04a", 0x080000, 0x80000, CRC(dff8069e) SHA1(3b31d1f66680cee1da3f3d3fd822739e99f48ccd) )
	ROM_LOAD16_WORD_SWAP( "19xb.05a", 0x100000, 0x80000, CRC(a47a92a8) SHA1(20254ec70029ec027793d4fbd9a7067c2a756315) )
	ROM_LOAD16_WORD_SWAP( "19xb.06a", 0x180000, 0x80000, CRC(c52df10d) SHA1(88707c4d1ec8649a7e10ec60e5bbc6f5ffb26f73) )
	ROM_LOAD16_WORD_SWAP( "19x.07",   0x200000, 0x80000, CRC(61c0296c) SHA1(9e225beccffd14bb53a32f8c0f2aef7f331dae30) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROMX_LOAD( "19x.13m",   0x0000000, 0x080000, CRC(427aeb18) SHA1(901029b5423e4bda85f592735036c06b7d426680) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "19x.15m",   0x0000002, 0x080000, CRC(63bdbf54) SHA1(9beb64ef0a8c92490848599d5d979bf42532609d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "19x.17m",   0x0000004, 0x080000, CRC(2dfe18b5) SHA1(8a44364d9af6b9e1664b44b9235dc172182c9eb8) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "19x.19m",   0x0000006, 0x080000, CRC(cbef9579) SHA1(172413f220b242411218c7865e04014ec6417537) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "19x.14m",   0x0800000, 0x200000, CRC(e916967c) SHA1(3f937022166149a80585f91388de521055ca88ca) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "19x.16m",   0x0800002, 0x200000, CRC(6e75f3db) SHA1(4e1c8466eaa612102d0807d2e8bf1004e97476ea) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "19x.18m",   0x0800004, 0x200000, CRC(2213e798) SHA1(b1a9d5547f3f6c3ab59e8b761d224793c6ca33cb) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "19x.20m",   0x0800006, 0x200000, CRC(ab9d5b96) SHA1(52b755da401fde90c13181b02ab33e5e4b2aa1f7) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "19x.01",   0x00000, 0x08000, CRC(ef55195e) SHA1(813f465f2d392f6abeadbf661c54cf51171fa006) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "19x.11m",   0x000000, 0x200000, CRC(d38beef3) SHA1(134e961b926a97cca5e45d3558efb98f6f278e08) )
	ROM_LOAD16_WORD_SWAP( "19x.12m",   0x200000, 0x200000, CRC(d47c96e2) SHA1(3c1b5563f8e7ee1c450b3592fcb319e928caec3c) )
ROM_END

ROM_START( armwar )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "pwge.03c", 0x000000, 0x80000, CRC(31f74931) SHA1(66150fc9896acca1691c9d586abeb2c7299bb9ad) )
	ROM_LOAD16_WORD_SWAP( "pwge.04c", 0x080000, 0x80000, CRC(16f34f5f) SHA1(b831e3915d8cbffdfe4720d356e5196cdebdb6e7) )
	ROM_LOAD16_WORD_SWAP( "pwge.05b", 0x100000, 0x80000, CRC(4403ed08) SHA1(cc78c84105a58e43e7a6429281397d68b91e86e5) )
	ROM_LOAD16_WORD_SWAP( "pwg.06",   0x180000, 0x80000, CRC(87a60ce8) SHA1(e2085c7c8c6792d055dbbb023c7f4e4aa38ae924) )
	ROM_LOAD16_WORD_SWAP( "pwg.07",   0x200000, 0x80000, CRC(f7b148df) SHA1(f369669713cf647222094c570a2eacd48a8637cf) )
	ROM_LOAD16_WORD_SWAP( "pwg.08",   0x280000, 0x80000, CRC(cc62823e) SHA1(edaf9bebdfc65ae5414090abd6844176eec39a00) )
	ROM_LOAD16_WORD_SWAP( "pwg.09a",  0x300000, 0x80000, CRC(4c26baee) SHA1(685f050206b9b904ce6a1ae9a8e8f019012cea43) )
	ROM_LOAD16_WORD_SWAP( "pwg.10",   0x380000, 0x80000, CRC(07c4fb28) SHA1(58a1ff3d105be7df833dd4f32973766649efcbcf) )

	ROM_REGION( 0x1400000, "gfx", 0 )
	ROMX_LOAD( "pwg.13m",   0x0000000, 0x400000, CRC(ae8fe08e) SHA1(b6f09663dcda69b5d7ac13e4afaf1efd692fb61e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.15m",   0x0000002, 0x400000, CRC(db560f58) SHA1(0c3716b32eb24544ff5d16b5dcadce195cd10d00) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.17m",   0x0000004, 0x400000, CRC(bc475b94) SHA1(a157664450895a146a532581dd6f4b63dff21c86) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.19m",   0x0000006, 0x400000, CRC(07439ff7) SHA1(f71e07c6d77c32828f5e319268b24b13a1a4b0c2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.14m",   0x1000000, 0x100000, CRC(c3f9ba63) SHA1(66191a52c39daa89b17ede5804ee41c028036f14) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.16m",   0x1000002, 0x100000, CRC(815b0e7b) SHA1(549785daac3122253fb94f6541bc7016147f5306) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.18m",   0x1000004, 0x100000, CRC(0109c71b) SHA1(eb51284ee0c85ff8f605fe1d166b7aa202be1344) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.20m",   0x1000006, 0x100000, CRC(eb75ffbe) SHA1(e9d1deca60be696ac5bff2017fb5de3525e5239a) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "pwg.01",   0x00000, 0x08000, CRC(18a5c0e4) SHA1(bb1353dd74884aaeec9b5f1d0b284d9cad53c0ff) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "pwg.02",   0x28000, 0x20000, CRC(c9dfffa6) SHA1(64e71028befe9a2514074be765dd020e1d2ea70b) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "pwg.11m",   0x000000, 0x200000, CRC(a78f7433) SHA1(e47ffba7b9dac9d0dda985c5d966194be18260f7) )
	ROM_LOAD16_WORD_SWAP( "pwg.12m",   0x200000, 0x200000, CRC(77438ed0) SHA1(733ca6c6a792e66e2aa12c5fc06dd459527afe4b) )
ROM_END

ROM_START( armwarr1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "pwge.03b", 0x000000, 0x80000, CRC(e822e3e9) SHA1(dcd153bb70f6c2baffa2f3687def30d85fca44ba) )
	ROM_LOAD16_WORD_SWAP( "pwge.04b", 0x080000, 0x80000, CRC(4f89de39) SHA1(1e54ed70a6ed9330ec83fb189f76e9417c6dfc13) )
	ROM_LOAD16_WORD_SWAP( "pwge.05a", 0x100000, 0x80000, CRC(83df24e5) SHA1(39801452769569c3271b26c6be8d3ce5e72b0629) )
	ROM_LOAD16_WORD_SWAP( "pwg.06",   0x180000, 0x80000, CRC(87a60ce8) SHA1(e2085c7c8c6792d055dbbb023c7f4e4aa38ae924) )
	ROM_LOAD16_WORD_SWAP( "pwg.07",   0x200000, 0x80000, CRC(f7b148df) SHA1(f369669713cf647222094c570a2eacd48a8637cf) )
	ROM_LOAD16_WORD_SWAP( "pwg.08",   0x280000, 0x80000, CRC(cc62823e) SHA1(edaf9bebdfc65ae5414090abd6844176eec39a00) )
	ROM_LOAD16_WORD_SWAP( "pwg.09",   0x300000, 0x80000, CRC(ddc85ca6) SHA1(e794c679531632e2142c6a5e3b858494389ce65e) )
	ROM_LOAD16_WORD_SWAP( "pwg.10",   0x380000, 0x80000, CRC(07c4fb28) SHA1(58a1ff3d105be7df833dd4f32973766649efcbcf) )

	ROM_REGION( 0x1400000, "gfx", 0 )
	ROMX_LOAD( "pwg.13m",   0x0000000, 0x400000, CRC(ae8fe08e) SHA1(b6f09663dcda69b5d7ac13e4afaf1efd692fb61e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.15m",   0x0000002, 0x400000, CRC(db560f58) SHA1(0c3716b32eb24544ff5d16b5dcadce195cd10d00) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.17m",   0x0000004, 0x400000, CRC(bc475b94) SHA1(a157664450895a146a532581dd6f4b63dff21c86) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.19m",   0x0000006, 0x400000, CRC(07439ff7) SHA1(f71e07c6d77c32828f5e319268b24b13a1a4b0c2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.14m",   0x1000000, 0x100000, CRC(c3f9ba63) SHA1(66191a52c39daa89b17ede5804ee41c028036f14) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.16m",   0x1000002, 0x100000, CRC(815b0e7b) SHA1(549785daac3122253fb94f6541bc7016147f5306) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.18m",   0x1000004, 0x100000, CRC(0109c71b) SHA1(eb51284ee0c85ff8f605fe1d166b7aa202be1344) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.20m",   0x1000006, 0x100000, CRC(eb75ffbe) SHA1(e9d1deca60be696ac5bff2017fb5de3525e5239a) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "pwg.01",   0x00000, 0x08000, CRC(18a5c0e4) SHA1(bb1353dd74884aaeec9b5f1d0b284d9cad53c0ff) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "pwg.02",   0x28000, 0x20000, CRC(c9dfffa6) SHA1(64e71028befe9a2514074be765dd020e1d2ea70b) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "pwg.11m",   0x000000, 0x200000, CRC(a78f7433) SHA1(e47ffba7b9dac9d0dda985c5d966194be18260f7) )
	ROM_LOAD16_WORD_SWAP( "pwg.12m",   0x200000, 0x200000, CRC(77438ed0) SHA1(733ca6c6a792e66e2aa12c5fc06dd459527afe4b) )
ROM_END

ROM_START( armwaru )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "pwgu.03b", 0x000000, 0x80000, CRC(8b95497a) SHA1(0c037b8a484d69f5e8c9600de71177fb78e9ede0) )
	ROM_LOAD16_WORD_SWAP( "pwgu.04b", 0x080000, 0x80000, CRC(29eb5661) SHA1(7ee9150072882c9e158ca8231f26a9f62c8fa50e) )
	ROM_LOAD16_WORD_SWAP( "pwgu.05b", 0x100000, 0x80000, CRC(a54e9e44) SHA1(e235dcdbd0111f018519d9c8eef130121ea20a20) )
	ROM_LOAD16_WORD_SWAP( "pwg.06",   0x180000, 0x80000, CRC(87a60ce8) SHA1(e2085c7c8c6792d055dbbb023c7f4e4aa38ae924) )
	ROM_LOAD16_WORD_SWAP( "pwg.07",   0x200000, 0x80000, CRC(f7b148df) SHA1(f369669713cf647222094c570a2eacd48a8637cf) )
	ROM_LOAD16_WORD_SWAP( "pwg.08",   0x280000, 0x80000, CRC(cc62823e) SHA1(edaf9bebdfc65ae5414090abd6844176eec39a00) )
	ROM_LOAD16_WORD_SWAP( "pwg.09a",  0x300000, 0x80000, CRC(4c26baee) SHA1(685f050206b9b904ce6a1ae9a8e8f019012cea43) )
	ROM_LOAD16_WORD_SWAP( "pwg.10",   0x380000, 0x80000, CRC(07c4fb28) SHA1(58a1ff3d105be7df833dd4f32973766649efcbcf) )

	ROM_REGION( 0x1400000, "gfx", 0 )
	ROMX_LOAD( "pwg.13m",   0x0000000, 0x400000, CRC(ae8fe08e) SHA1(b6f09663dcda69b5d7ac13e4afaf1efd692fb61e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.15m",   0x0000002, 0x400000, CRC(db560f58) SHA1(0c3716b32eb24544ff5d16b5dcadce195cd10d00) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.17m",   0x0000004, 0x400000, CRC(bc475b94) SHA1(a157664450895a146a532581dd6f4b63dff21c86) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.19m",   0x0000006, 0x400000, CRC(07439ff7) SHA1(f71e07c6d77c32828f5e319268b24b13a1a4b0c2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.14m",   0x1000000, 0x100000, CRC(c3f9ba63) SHA1(66191a52c39daa89b17ede5804ee41c028036f14) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.16m",   0x1000002, 0x100000, CRC(815b0e7b) SHA1(549785daac3122253fb94f6541bc7016147f5306) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.18m",   0x1000004, 0x100000, CRC(0109c71b) SHA1(eb51284ee0c85ff8f605fe1d166b7aa202be1344) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.20m",   0x1000006, 0x100000, CRC(eb75ffbe) SHA1(e9d1deca60be696ac5bff2017fb5de3525e5239a) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "pwg.01",   0x00000, 0x08000, CRC(18a5c0e4) SHA1(bb1353dd74884aaeec9b5f1d0b284d9cad53c0ff) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "pwg.02",   0x28000, 0x20000, CRC(c9dfffa6) SHA1(64e71028befe9a2514074be765dd020e1d2ea70b) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "pwg.11m",   0x000000, 0x200000, CRC(a78f7433) SHA1(e47ffba7b9dac9d0dda985c5d966194be18260f7) )
	ROM_LOAD16_WORD_SWAP( "pwg.12m",   0x200000, 0x200000, CRC(77438ed0) SHA1(733ca6c6a792e66e2aa12c5fc06dd459527afe4b) )
ROM_END

ROM_START( armwaru1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "pwgu.03a", 0x000000, 0x80000, CRC(73d397b1) SHA1(43c33f5268e98411fbbb27f8b59c2ff5dcaf3c34) )
	ROM_LOAD16_WORD_SWAP( "pwgu.04a", 0x080000, 0x80000, CRC(1f1de215) SHA1(b0a74a4effddd30fbc972d94e4bf3848c4893363) )
	ROM_LOAD16_WORD_SWAP( "pwgu.05a", 0x100000, 0x80000, CRC(835fbe73) SHA1(6218aa1b480105ffabded980c92679fafb19b824) )
	ROM_LOAD16_WORD_SWAP( "pwg.06",   0x180000, 0x80000, CRC(87a60ce8) SHA1(e2085c7c8c6792d055dbbb023c7f4e4aa38ae924) )
	ROM_LOAD16_WORD_SWAP( "pwg.07",   0x200000, 0x80000, CRC(f7b148df) SHA1(f369669713cf647222094c570a2eacd48a8637cf) )
	ROM_LOAD16_WORD_SWAP( "pwg.08",   0x280000, 0x80000, CRC(cc62823e) SHA1(edaf9bebdfc65ae5414090abd6844176eec39a00) )
	ROM_LOAD16_WORD_SWAP( "pwg.09",   0x300000, 0x80000, CRC(ddc85ca6) SHA1(e794c679531632e2142c6a5e3b858494389ce65e) )
	ROM_LOAD16_WORD_SWAP( "pwg.10",   0x380000, 0x80000, CRC(07c4fb28) SHA1(58a1ff3d105be7df833dd4f32973766649efcbcf) )

	ROM_REGION( 0x1400000, "gfx", 0 )
	ROMX_LOAD( "pwg.13m",   0x0000000, 0x400000, CRC(ae8fe08e) SHA1(b6f09663dcda69b5d7ac13e4afaf1efd692fb61e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.15m",   0x0000002, 0x400000, CRC(db560f58) SHA1(0c3716b32eb24544ff5d16b5dcadce195cd10d00) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.17m",   0x0000004, 0x400000, CRC(bc475b94) SHA1(a157664450895a146a532581dd6f4b63dff21c86) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.19m",   0x0000006, 0x400000, CRC(07439ff7) SHA1(f71e07c6d77c32828f5e319268b24b13a1a4b0c2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.14m",   0x1000000, 0x100000, CRC(c3f9ba63) SHA1(66191a52c39daa89b17ede5804ee41c028036f14) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.16m",   0x1000002, 0x100000, CRC(815b0e7b) SHA1(549785daac3122253fb94f6541bc7016147f5306) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.18m",   0x1000004, 0x100000, CRC(0109c71b) SHA1(eb51284ee0c85ff8f605fe1d166b7aa202be1344) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.20m",   0x1000006, 0x100000, CRC(eb75ffbe) SHA1(e9d1deca60be696ac5bff2017fb5de3525e5239a) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "pwg.01",   0x00000, 0x08000, CRC(18a5c0e4) SHA1(bb1353dd74884aaeec9b5f1d0b284d9cad53c0ff) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "pwg.02",   0x28000, 0x20000, CRC(c9dfffa6) SHA1(64e71028befe9a2514074be765dd020e1d2ea70b) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "pwg.11m",   0x000000, 0x200000, CRC(a78f7433) SHA1(e47ffba7b9dac9d0dda985c5d966194be18260f7) )
	ROM_LOAD16_WORD_SWAP( "pwg.12m",   0x200000, 0x200000, CRC(77438ed0) SHA1(733ca6c6a792e66e2aa12c5fc06dd459527afe4b) )
ROM_END

ROM_START( pgear )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "pwgj.03a", 0x000000, 0x80000, CRC(c79c0c02) SHA1(4e24d34be255bb3886bd6b767779ee5fc81dca6e) )
	ROM_LOAD16_WORD_SWAP( "pwgj.04a", 0x080000, 0x80000, CRC(167c6ed8) SHA1(23a4a7faae817ffc6c5faa4db5b96b8c8c0dfe86) )
	ROM_LOAD16_WORD_SWAP( "pwgj.05a", 0x100000, 0x80000, CRC(a63fb400) SHA1(b27464b000cd12d9247254f843be27639fbf3a48) )
	ROM_LOAD16_WORD_SWAP( "pwg.06",   0x180000, 0x80000, CRC(87a60ce8) SHA1(e2085c7c8c6792d055dbbb023c7f4e4aa38ae924) )
	ROM_LOAD16_WORD_SWAP( "pwg.07",   0x200000, 0x80000, CRC(f7b148df) SHA1(f369669713cf647222094c570a2eacd48a8637cf) )
	ROM_LOAD16_WORD_SWAP( "pwg.08",   0x280000, 0x80000, CRC(cc62823e) SHA1(edaf9bebdfc65ae5414090abd6844176eec39a00) )
	ROM_LOAD16_WORD_SWAP( "pwg.09a",  0x300000, 0x80000, CRC(4c26baee) SHA1(685f050206b9b904ce6a1ae9a8e8f019012cea43) )
	ROM_LOAD16_WORD_SWAP( "pwg.10",   0x380000, 0x80000, CRC(07c4fb28) SHA1(58a1ff3d105be7df833dd4f32973766649efcbcf) )

	ROM_REGION( 0x1400000, "gfx", 0 )
	ROMX_LOAD( "pwg.13m",   0x0000000, 0x400000, CRC(ae8fe08e) SHA1(b6f09663dcda69b5d7ac13e4afaf1efd692fb61e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.15m",   0x0000002, 0x400000, CRC(db560f58) SHA1(0c3716b32eb24544ff5d16b5dcadce195cd10d00) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.17m",   0x0000004, 0x400000, CRC(bc475b94) SHA1(a157664450895a146a532581dd6f4b63dff21c86) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.19m",   0x0000006, 0x400000, CRC(07439ff7) SHA1(f71e07c6d77c32828f5e319268b24b13a1a4b0c2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.14m",   0x1000000, 0x100000, CRC(c3f9ba63) SHA1(66191a52c39daa89b17ede5804ee41c028036f14) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.16m",   0x1000002, 0x100000, CRC(815b0e7b) SHA1(549785daac3122253fb94f6541bc7016147f5306) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.18m",   0x1000004, 0x100000, CRC(0109c71b) SHA1(eb51284ee0c85ff8f605fe1d166b7aa202be1344) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.20m",   0x1000006, 0x100000, CRC(eb75ffbe) SHA1(e9d1deca60be696ac5bff2017fb5de3525e5239a) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "pwg.01",   0x00000, 0x08000, CRC(18a5c0e4) SHA1(bb1353dd74884aaeec9b5f1d0b284d9cad53c0ff) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "pwg.02",   0x28000, 0x20000, CRC(c9dfffa6) SHA1(64e71028befe9a2514074be765dd020e1d2ea70b) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "pwg.11m",   0x000000, 0x200000, CRC(a78f7433) SHA1(e47ffba7b9dac9d0dda985c5d966194be18260f7) )
	ROM_LOAD16_WORD_SWAP( "pwg.12m",   0x200000, 0x200000, CRC(77438ed0) SHA1(733ca6c6a792e66e2aa12c5fc06dd459527afe4b) )
ROM_END

ROM_START( pgearr1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "pwgj.03", 0x000000, 0x80000, CRC(f264e74b) SHA1(db0a675a9d46df9227334259db633e27f7dc79ab) )
	ROM_LOAD16_WORD_SWAP( "pwgj.04", 0x080000, 0x80000, CRC(23a84983) SHA1(a3ed606f6213bb6e447c4ff84d6d3435a0170762) )
	ROM_LOAD16_WORD_SWAP( "pwgj.05", 0x100000, 0x80000, CRC(bef58c62) SHA1(178c255171c4010cec758ee11d96bdcee85abee0) )
	ROM_LOAD16_WORD_SWAP( "pwg.06",  0x180000, 0x80000, CRC(87a60ce8) SHA1(e2085c7c8c6792d055dbbb023c7f4e4aa38ae924) )
	ROM_LOAD16_WORD_SWAP( "pwg.07",  0x200000, 0x80000, CRC(f7b148df) SHA1(f369669713cf647222094c570a2eacd48a8637cf) )
	ROM_LOAD16_WORD_SWAP( "pwg.08",  0x280000, 0x80000, CRC(cc62823e) SHA1(edaf9bebdfc65ae5414090abd6844176eec39a00) )
	ROM_LOAD16_WORD_SWAP( "pwg.09",  0x300000, 0x80000, CRC(ddc85ca6) SHA1(e794c679531632e2142c6a5e3b858494389ce65e) )
	ROM_LOAD16_WORD_SWAP( "pwg.10",  0x380000, 0x80000, CRC(07c4fb28) SHA1(58a1ff3d105be7df833dd4f32973766649efcbcf) )

	ROM_REGION( 0x1400000, "gfx", 0 )
	ROMX_LOAD( "pwg.13m",   0x0000000, 0x400000, CRC(ae8fe08e) SHA1(b6f09663dcda69b5d7ac13e4afaf1efd692fb61e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.15m",   0x0000002, 0x400000, CRC(db560f58) SHA1(0c3716b32eb24544ff5d16b5dcadce195cd10d00) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.17m",   0x0000004, 0x400000, CRC(bc475b94) SHA1(a157664450895a146a532581dd6f4b63dff21c86) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.19m",   0x0000006, 0x400000, CRC(07439ff7) SHA1(f71e07c6d77c32828f5e319268b24b13a1a4b0c2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.14m",   0x1000000, 0x100000, CRC(c3f9ba63) SHA1(66191a52c39daa89b17ede5804ee41c028036f14) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.16m",   0x1000002, 0x100000, CRC(815b0e7b) SHA1(549785daac3122253fb94f6541bc7016147f5306) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.18m",   0x1000004, 0x100000, CRC(0109c71b) SHA1(eb51284ee0c85ff8f605fe1d166b7aa202be1344) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.20m",   0x1000006, 0x100000, CRC(eb75ffbe) SHA1(e9d1deca60be696ac5bff2017fb5de3525e5239a) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "pwg.01",   0x00000, 0x08000, CRC(18a5c0e4) SHA1(bb1353dd74884aaeec9b5f1d0b284d9cad53c0ff) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "pwg.02",   0x28000, 0x20000, CRC(c9dfffa6) SHA1(64e71028befe9a2514074be765dd020e1d2ea70b) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "pwg.11m",   0x000000, 0x200000, CRC(a78f7433) SHA1(e47ffba7b9dac9d0dda985c5d966194be18260f7) )
	ROM_LOAD16_WORD_SWAP( "pwg.12m",   0x200000, 0x200000, CRC(77438ed0) SHA1(733ca6c6a792e66e2aa12c5fc06dd459527afe4b) )
ROM_END

ROM_START( armwara )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "pwga.03a", 0x000000, 0x80000, CRC(8d474ab1) SHA1(46baa3a263189001cfc6003fcb346a1996be8b24) )
	ROM_LOAD16_WORD_SWAP( "pwga.04a", 0x080000, 0x80000, CRC(81b5aec7) SHA1(f1371149a00e7c52d022d5c0cb6f8821c6474d35) )
	ROM_LOAD16_WORD_SWAP( "pwga.05a", 0x100000, 0x80000, CRC(2618e819) SHA1(58c857988e0ad2839d936d3e405637d8e2a45fe9) )
	ROM_LOAD16_WORD_SWAP( "pwg.06",   0x180000, 0x80000, CRC(87a60ce8) SHA1(e2085c7c8c6792d055dbbb023c7f4e4aa38ae924) )
	ROM_LOAD16_WORD_SWAP( "pwg.07",   0x200000, 0x80000, CRC(f7b148df) SHA1(f369669713cf647222094c570a2eacd48a8637cf) )
	ROM_LOAD16_WORD_SWAP( "pwg.08",   0x280000, 0x80000, CRC(cc62823e) SHA1(edaf9bebdfc65ae5414090abd6844176eec39a00) )
	ROM_LOAD16_WORD_SWAP( "pwg.09",   0x300000, 0x80000, CRC(ddc85ca6) SHA1(e794c679531632e2142c6a5e3b858494389ce65e) )
	ROM_LOAD16_WORD_SWAP( "pwg.10",   0x380000, 0x80000, CRC(07c4fb28) SHA1(58a1ff3d105be7df833dd4f32973766649efcbcf) )

	ROM_REGION( 0x1400000, "gfx", 0 )
	ROMX_LOAD( "pwg.13m",   0x0000000, 0x400000, CRC(ae8fe08e) SHA1(b6f09663dcda69b5d7ac13e4afaf1efd692fb61e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.15m",   0x0000002, 0x400000, CRC(db560f58) SHA1(0c3716b32eb24544ff5d16b5dcadce195cd10d00) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.17m",   0x0000004, 0x400000, CRC(bc475b94) SHA1(a157664450895a146a532581dd6f4b63dff21c86) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.19m",   0x0000006, 0x400000, CRC(07439ff7) SHA1(f71e07c6d77c32828f5e319268b24b13a1a4b0c2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.14m",   0x1000000, 0x100000, CRC(c3f9ba63) SHA1(66191a52c39daa89b17ede5804ee41c028036f14) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.16m",   0x1000002, 0x100000, CRC(815b0e7b) SHA1(549785daac3122253fb94f6541bc7016147f5306) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.18m",   0x1000004, 0x100000, CRC(0109c71b) SHA1(eb51284ee0c85ff8f605fe1d166b7aa202be1344) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.20m",   0x1000006, 0x100000, CRC(eb75ffbe) SHA1(e9d1deca60be696ac5bff2017fb5de3525e5239a) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "pwg.01",   0x00000, 0x08000, CRC(18a5c0e4) SHA1(bb1353dd74884aaeec9b5f1d0b284d9cad53c0ff) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "pwg.02",   0x28000, 0x20000, CRC(c9dfffa6) SHA1(64e71028befe9a2514074be765dd020e1d2ea70b) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "pwg.11m",   0x000000, 0x200000, CRC(a78f7433) SHA1(e47ffba7b9dac9d0dda985c5d966194be18260f7) )
	ROM_LOAD16_WORD_SWAP( "pwg.12m",   0x200000, 0x200000, CRC(77438ed0) SHA1(733ca6c6a792e66e2aa12c5fc06dd459527afe4b) )
ROM_END

ROM_START( avsp )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "avpe.03d", 0x000000, 0x80000, CRC(774334a9) SHA1(f60b0e39139ea40e0b0ba97ed01d4a757ed65e1a) )
	ROM_LOAD16_WORD_SWAP( "avpe.04d", 0x080000, 0x80000, CRC(7fa83769) SHA1(930f02e4d35686e80fbdd673380c4b2bd784a9e5) )
	ROM_LOAD16_WORD_SWAP( "avp.05d",  0x100000, 0x80000, CRC(fbfb5d7a) SHA1(5549bc9d780753bc9c10fba82588e5c3d4a2acb2) )
	ROM_LOAD16_WORD_SWAP( "avp.06",   0x180000, 0x80000, CRC(190b817f) SHA1(9bcfc0a015ffba9cdac25b6270939a9690de5da7) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROMX_LOAD( "avp.13m",   0x0000000, 0x200000, CRC(8f8b5ae4) SHA1(457ce959aa5db3a003de7dda2b3799b2f1ae279b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "avp.15m",   0x0000002, 0x200000, CRC(b00280df) SHA1(bc1291a4a222d410bc99b6f1ed392067d9c3999e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "avp.17m",   0x0000004, 0x200000, CRC(94403195) SHA1(efaad001527a5eba8f626aea9037ac6ef9a2c295) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "avp.19m",   0x0000006, 0x200000, CRC(e1981245) SHA1(809ccb7f10262e227d5e9d9f710e06f0e751f550) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "avp.14m",   0x0800000, 0x200000, CRC(ebba093e) SHA1(77aaf4197d1dae3321cf9c6d2b7967ee54cf3f30) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "avp.16m",   0x0800002, 0x200000, CRC(fb228297) SHA1(ebd02a4ba085dc70c0603662e14d61625fa04648) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "avp.18m",   0x0800004, 0x200000, CRC(34fb7232) SHA1(8b1f15bfa758a61e6ad519af24ca774edc70d194) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "avp.20m",   0x0800006, 0x200000, CRC(f90baa21) SHA1(20a900819a9d321316e3dfd241210725d7191ecf) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "avp.01",   0x00000, 0x08000, CRC(2d3b4220) SHA1(2b2d04d4282550fa9f6e1ad8528f20d1f2ac02eb) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "avp.11m",   0x000000, 0x200000, CRC(83499817) SHA1(e65b0ebd61ddc748842a9d4d92404b5305307623) )
	ROM_LOAD16_WORD_SWAP( "avp.12m",   0x200000, 0x200000, CRC(f4110d49) SHA1(f27538776cc1ba8213f19f98728ed8c02508d3ac) )
ROM_END

ROM_START( avspu )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "avpu.03d", 0x000000, 0x80000, CRC(42757950) SHA1(e6acae73a300c0e07c21f776e6aa87628184b152) )
	ROM_LOAD16_WORD_SWAP( "avpu.04d", 0x080000, 0x80000, CRC(5abcdee6) SHA1(205e1ac8f4e359fd04e3a1e12425ba0b8330b1c1) )
	ROM_LOAD16_WORD_SWAP( "avp.05d",  0x100000, 0x80000, CRC(fbfb5d7a) SHA1(5549bc9d780753bc9c10fba82588e5c3d4a2acb2) )
	ROM_LOAD16_WORD_SWAP( "avp.06",   0x180000, 0x80000, CRC(190b817f) SHA1(9bcfc0a015ffba9cdac25b6270939a9690de5da7) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROMX_LOAD( "avp.13m",   0x0000000, 0x200000, CRC(8f8b5ae4) SHA1(457ce959aa5db3a003de7dda2b3799b2f1ae279b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "avp.15m",   0x0000002, 0x200000, CRC(b00280df) SHA1(bc1291a4a222d410bc99b6f1ed392067d9c3999e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "avp.17m",   0x0000004, 0x200000, CRC(94403195) SHA1(efaad001527a5eba8f626aea9037ac6ef9a2c295) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "avp.19m",   0x0000006, 0x200000, CRC(e1981245) SHA1(809ccb7f10262e227d5e9d9f710e06f0e751f550) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "avp.14m",   0x0800000, 0x200000, CRC(ebba093e) SHA1(77aaf4197d1dae3321cf9c6d2b7967ee54cf3f30) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "avp.16m",   0x0800002, 0x200000, CRC(fb228297) SHA1(ebd02a4ba085dc70c0603662e14d61625fa04648) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "avp.18m",   0x0800004, 0x200000, CRC(34fb7232) SHA1(8b1f15bfa758a61e6ad519af24ca774edc70d194) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "avp.20m",   0x0800006, 0x200000, CRC(f90baa21) SHA1(20a900819a9d321316e3dfd241210725d7191ecf) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "avp.01",   0x00000, 0x08000, CRC(2d3b4220) SHA1(2b2d04d4282550fa9f6e1ad8528f20d1f2ac02eb) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "avp.11m",   0x000000, 0x200000, CRC(83499817) SHA1(e65b0ebd61ddc748842a9d4d92404b5305307623) )
	ROM_LOAD16_WORD_SWAP( "avp.12m",   0x200000, 0x200000, CRC(f4110d49) SHA1(f27538776cc1ba8213f19f98728ed8c02508d3ac) )
ROM_END

ROM_START( avspj )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "avpj.03d", 0x000000, 0x80000, CRC(49799119) SHA1(71a938b779291c3092ef6ef22935d89fd9c1186c) )
	ROM_LOAD16_WORD_SWAP( "avpj.04d", 0x080000, 0x80000, CRC(8cd2bba8) SHA1(1ea493d0d4b6e202ad38843b93035fa3f7e1b8c7) )
	ROM_LOAD16_WORD_SWAP( "avp.05d",  0x100000, 0x80000, CRC(fbfb5d7a) SHA1(5549bc9d780753bc9c10fba82588e5c3d4a2acb2) )
	ROM_LOAD16_WORD_SWAP( "avp.06",   0x180000, 0x80000, CRC(190b817f) SHA1(9bcfc0a015ffba9cdac25b6270939a9690de5da7) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROMX_LOAD( "avp.13m",   0x0000000, 0x200000, CRC(8f8b5ae4) SHA1(457ce959aa5db3a003de7dda2b3799b2f1ae279b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "avp.15m",   0x0000002, 0x200000, CRC(b00280df) SHA1(bc1291a4a222d410bc99b6f1ed392067d9c3999e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "avp.17m",   0x0000004, 0x200000, CRC(94403195) SHA1(efaad001527a5eba8f626aea9037ac6ef9a2c295) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "avp.19m",   0x0000006, 0x200000, CRC(e1981245) SHA1(809ccb7f10262e227d5e9d9f710e06f0e751f550) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "avp.14m",   0x0800000, 0x200000, CRC(ebba093e) SHA1(77aaf4197d1dae3321cf9c6d2b7967ee54cf3f30) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "avp.16m",   0x0800002, 0x200000, CRC(fb228297) SHA1(ebd02a4ba085dc70c0603662e14d61625fa04648) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "avp.18m",   0x0800004, 0x200000, CRC(34fb7232) SHA1(8b1f15bfa758a61e6ad519af24ca774edc70d194) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "avp.20m",   0x0800006, 0x200000, CRC(f90baa21) SHA1(20a900819a9d321316e3dfd241210725d7191ecf) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "avp.01",   0x00000, 0x08000, CRC(2d3b4220) SHA1(2b2d04d4282550fa9f6e1ad8528f20d1f2ac02eb) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "avp.11m",   0x000000, 0x200000, CRC(83499817) SHA1(e65b0ebd61ddc748842a9d4d92404b5305307623) )
	ROM_LOAD16_WORD_SWAP( "avp.12m",   0x200000, 0x200000, CRC(f4110d49) SHA1(f27538776cc1ba8213f19f98728ed8c02508d3ac) )
ROM_END

ROM_START( avspa )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "avpa.03d", 0x000000, 0x80000, CRC(6c1c1858) SHA1(29af268cf070ea2adc0aac0c5187debdd9706037) )
	ROM_LOAD16_WORD_SWAP( "avpa.04d", 0x080000, 0x80000, CRC(94f50b0c) SHA1(607b13e4cb4968c47a598f7dfec965c6d6ba68f0) )
	ROM_LOAD16_WORD_SWAP( "avp.05d",  0x100000, 0x80000, CRC(fbfb5d7a) SHA1(5549bc9d780753bc9c10fba82588e5c3d4a2acb2) )
	ROM_LOAD16_WORD_SWAP( "avp.06",   0x180000, 0x80000, CRC(190b817f) SHA1(9bcfc0a015ffba9cdac25b6270939a9690de5da7) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROMX_LOAD( "avp.13m",   0x0000000, 0x200000, CRC(8f8b5ae4) SHA1(457ce959aa5db3a003de7dda2b3799b2f1ae279b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "avp.15m",   0x0000002, 0x200000, CRC(b00280df) SHA1(bc1291a4a222d410bc99b6f1ed392067d9c3999e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "avp.17m",   0x0000004, 0x200000, CRC(94403195) SHA1(efaad001527a5eba8f626aea9037ac6ef9a2c295) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "avp.19m",   0x0000006, 0x200000, CRC(e1981245) SHA1(809ccb7f10262e227d5e9d9f710e06f0e751f550) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "avp.14m",   0x0800000, 0x200000, CRC(ebba093e) SHA1(77aaf4197d1dae3321cf9c6d2b7967ee54cf3f30) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "avp.16m",   0x0800002, 0x200000, CRC(fb228297) SHA1(ebd02a4ba085dc70c0603662e14d61625fa04648) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "avp.18m",   0x0800004, 0x200000, CRC(34fb7232) SHA1(8b1f15bfa758a61e6ad519af24ca774edc70d194) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "avp.20m",   0x0800006, 0x200000, CRC(f90baa21) SHA1(20a900819a9d321316e3dfd241210725d7191ecf) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "avp.01",   0x00000, 0x08000, CRC(2d3b4220) SHA1(2b2d04d4282550fa9f6e1ad8528f20d1f2ac02eb) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "avp.11m",   0x000000, 0x200000, CRC(83499817) SHA1(e65b0ebd61ddc748842a9d4d92404b5305307623) )
	ROM_LOAD16_WORD_SWAP( "avp.12m",   0x200000, 0x200000, CRC(f4110d49) SHA1(f27538776cc1ba8213f19f98728ed8c02508d3ac) )
ROM_END

ROM_START( avsph )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "avph.03d", 0x000000, 0x80000, CRC(3e440447) SHA1(5b7431de6b9e243f041d0e76b3a69002662c321a) )
	ROM_LOAD16_WORD_SWAP( "avph.04d", 0x080000, 0x80000, CRC(af6fc82f) SHA1(c0293d71a657dbbe14ce15121d0970ccc7e584cf) )
	ROM_LOAD16_WORD_SWAP( "avp.05d",  0x100000, 0x80000, CRC(fbfb5d7a) SHA1(5549bc9d780753bc9c10fba82588e5c3d4a2acb2) )
	ROM_LOAD16_WORD_SWAP( "avp.06",   0x180000, 0x80000, CRC(190b817f) SHA1(9bcfc0a015ffba9cdac25b6270939a9690de5da7) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROMX_LOAD( "avp.13m",   0x0000000, 0x200000, CRC(8f8b5ae4) SHA1(457ce959aa5db3a003de7dda2b3799b2f1ae279b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "avp.15m",   0x0000002, 0x200000, CRC(b00280df) SHA1(bc1291a4a222d410bc99b6f1ed392067d9c3999e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "avp.17m",   0x0000004, 0x200000, CRC(94403195) SHA1(efaad001527a5eba8f626aea9037ac6ef9a2c295) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "avp.19m",   0x0000006, 0x200000, CRC(e1981245) SHA1(809ccb7f10262e227d5e9d9f710e06f0e751f550) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "avp.14m",   0x0800000, 0x200000, CRC(ebba093e) SHA1(77aaf4197d1dae3321cf9c6d2b7967ee54cf3f30) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "avp.16m",   0x0800002, 0x200000, CRC(fb228297) SHA1(ebd02a4ba085dc70c0603662e14d61625fa04648) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "avp.18m",   0x0800004, 0x200000, CRC(34fb7232) SHA1(8b1f15bfa758a61e6ad519af24ca774edc70d194) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "avp.20m",   0x0800006, 0x200000, CRC(f90baa21) SHA1(20a900819a9d321316e3dfd241210725d7191ecf) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "avp.01",   0x00000, 0x08000, CRC(2d3b4220) SHA1(2b2d04d4282550fa9f6e1ad8528f20d1f2ac02eb) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "avp.11m",   0x000000, 0x200000, CRC(83499817) SHA1(e65b0ebd61ddc748842a9d4d92404b5305307623) )
	ROM_LOAD16_WORD_SWAP( "avp.12m",   0x200000, 0x200000, CRC(f4110d49) SHA1(f27538776cc1ba8213f19f98728ed8c02508d3ac) )
ROM_END

ROM_START( batcir )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "btce.03", 0x000000, 0x80000, CRC(bc60484b) SHA1(9b4e46d0f3d96edcd1c3707409507a5027c69039) )
	ROM_LOAD16_WORD_SWAP( "btce.04", 0x080000, 0x80000, CRC(457d55f6) SHA1(19a39ec30166d4b797babe9d70328ac572d1f916) )
	ROM_LOAD16_WORD_SWAP( "btce.05", 0x100000, 0x80000, CRC(e86560d7) SHA1(a978a7f5e0069cd78c8588c2d91b825796c723a5) )
	ROM_LOAD16_WORD_SWAP( "btce.06", 0x180000, 0x80000, CRC(f778e61b) SHA1(e8321dece8977131e41c9207946b627074c13ee7) )
	ROM_LOAD16_WORD_SWAP( "btc.07",  0x200000, 0x80000, CRC(7322d5db) SHA1(473be1f1bf603bdd82451661a6206507f50ed2b6) )
	ROM_LOAD16_WORD_SWAP( "btc.08",  0x280000, 0x80000, CRC(6aac85ab) SHA1(ad02d4185c2b3664fb96350d8ad317d3939a7554) )
	ROM_LOAD16_WORD_SWAP( "btc.09",  0x300000, 0x80000, CRC(1203db08) SHA1(fdbea14618b277132f9e010ef36c134a8ea42162) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROMX_LOAD( "btc.13m",   0x000000, 0x400000, CRC(dc705bad) SHA1(96e37147674bf9cd21c770897da59daac25d921a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "btc.15m",   0x000002, 0x400000, CRC(e5779a3c) SHA1(bbd7fbe061e751388d2f02434144daf9b1e36640) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "btc.17m",   0x000004, 0x400000, CRC(b33f4112) SHA1(e501fd921c8bcede69946b029e05d422714c1040) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "btc.19m",   0x000006, 0x400000, CRC(a6fcdb7e) SHA1(7a28d5d7aa036d23d97fad17d0cdb8210dc8153a) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "btc.01",   0x00000, 0x08000, CRC(1e194310) SHA1(3b29de0aca9dbca59d6b50fb2509e2a913c6b0af) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "btc.02",   0x28000, 0x20000, CRC(01aeb8e6) SHA1(50a5d1cce0caf7c5143d4904431e8f41e2a57464) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "btc.11m",   0x000000, 0x200000, CRC(c27f2229) SHA1(df2459493af40937b6656a16fad43ff51bed2204) )
	ROM_LOAD16_WORD_SWAP( "btc.12m",   0x200000, 0x200000, CRC(418a2e33) SHA1(0642ddff2ab9255f154419da24ba644ed63f34ab) )
ROM_END

ROM_START( batcirj )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "btcj.03", 0x000000, 0x80000, CRC(6b7e168d) SHA1(7e95cc436d53d1ce34b575bc7e2b6e2a7ae06cfb) )
	ROM_LOAD16_WORD_SWAP( "btcj.04", 0x080000, 0x80000, CRC(46ba3467) SHA1(0cc4a6c82f110d2334fd81f2d3abe5de882768bd) )
	ROM_LOAD16_WORD_SWAP( "btcj.05", 0x100000, 0x80000, CRC(0e23a859) SHA1(6c7eec9bf823c66fddbc6b297ea6aa883d03bff5) )
	ROM_LOAD16_WORD_SWAP( "btcj.06", 0x180000, 0x80000, CRC(a853b59c) SHA1(841c178722d4850994afee90ef3079607d8847ed) )
	ROM_LOAD16_WORD_SWAP( "btc.07",  0x200000, 0x80000, CRC(7322d5db) SHA1(473be1f1bf603bdd82451661a6206507f50ed2b6) )
	ROM_LOAD16_WORD_SWAP( "btc.08",  0x280000, 0x80000, CRC(6aac85ab) SHA1(ad02d4185c2b3664fb96350d8ad317d3939a7554) )
	ROM_LOAD16_WORD_SWAP( "btc.09",  0x300000, 0x80000, CRC(1203db08) SHA1(fdbea14618b277132f9e010ef36c134a8ea42162) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROMX_LOAD( "btc.13m",   0x000000, 0x400000, CRC(dc705bad) SHA1(96e37147674bf9cd21c770897da59daac25d921a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "btc.15m",   0x000002, 0x400000, CRC(e5779a3c) SHA1(bbd7fbe061e751388d2f02434144daf9b1e36640) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "btc.17m",   0x000004, 0x400000, CRC(b33f4112) SHA1(e501fd921c8bcede69946b029e05d422714c1040) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "btc.19m",   0x000006, 0x400000, CRC(a6fcdb7e) SHA1(7a28d5d7aa036d23d97fad17d0cdb8210dc8153a) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "btc.01",   0x00000, 0x08000, CRC(1e194310) SHA1(3b29de0aca9dbca59d6b50fb2509e2a913c6b0af) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "btc.02",   0x28000, 0x20000, CRC(01aeb8e6) SHA1(50a5d1cce0caf7c5143d4904431e8f41e2a57464) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "btc.11m",   0x000000, 0x200000, CRC(c27f2229) SHA1(df2459493af40937b6656a16fad43ff51bed2204) )
	ROM_LOAD16_WORD_SWAP( "btc.12m",   0x200000, 0x200000, CRC(418a2e33) SHA1(0642ddff2ab9255f154419da24ba644ed63f34ab) )
ROM_END

ROM_START( batcira )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "btca.03", 0x000000, 0x80000, CRC(1ad20d87) SHA1(0ad8b7725604a61314883cd4ed8599445fe1cbf8) )
	ROM_LOAD16_WORD_SWAP( "btca.04", 0x080000, 0x80000, CRC(2b3f4dbe) SHA1(be4ab2ac411523def5e05081a754b651ead52e1f) )
	ROM_LOAD16_WORD_SWAP( "btca.05", 0x100000, 0x80000, CRC(8238a3d9) SHA1(4b0fe0e6c6a8a6572fc3554f2ee77dc01c2f75c3) )
	ROM_LOAD16_WORD_SWAP( "btca.06", 0x180000, 0x80000, CRC(446c7c02) SHA1(2fda5d0fef3ca556976ec9126cb04af4fa883a38) )
	ROM_LOAD16_WORD_SWAP( "btc.07",  0x200000, 0x80000, CRC(7322d5db) SHA1(473be1f1bf603bdd82451661a6206507f50ed2b6) )
	ROM_LOAD16_WORD_SWAP( "btc.08",  0x280000, 0x80000, CRC(6aac85ab) SHA1(ad02d4185c2b3664fb96350d8ad317d3939a7554) )
	ROM_LOAD16_WORD_SWAP( "btc.09",  0x300000, 0x80000, CRC(1203db08) SHA1(fdbea14618b277132f9e010ef36c134a8ea42162) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROMX_LOAD( "btc.13m",   0x000000, 0x400000, CRC(dc705bad) SHA1(96e37147674bf9cd21c770897da59daac25d921a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "btc.15m",   0x000002, 0x400000, CRC(e5779a3c) SHA1(bbd7fbe061e751388d2f02434144daf9b1e36640) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "btc.17m",   0x000004, 0x400000, CRC(b33f4112) SHA1(e501fd921c8bcede69946b029e05d422714c1040) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "btc.19m",   0x000006, 0x400000, CRC(a6fcdb7e) SHA1(7a28d5d7aa036d23d97fad17d0cdb8210dc8153a) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "btc.01",   0x00000, 0x08000, CRC(1e194310) SHA1(3b29de0aca9dbca59d6b50fb2509e2a913c6b0af) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "btc.02",   0x28000, 0x20000, CRC(01aeb8e6) SHA1(50a5d1cce0caf7c5143d4904431e8f41e2a57464) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "btc.11m",   0x000000, 0x200000, CRC(c27f2229) SHA1(df2459493af40937b6656a16fad43ff51bed2204) )
	ROM_LOAD16_WORD_SWAP( "btc.12m",   0x200000, 0x200000, CRC(418a2e33) SHA1(0642ddff2ab9255f154419da24ba644ed63f34ab) )
ROM_END

ROM_START( choko )
	ROM_REGION( CODE_SIZE, "maincpu", ROMREGION_ERASEFF )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "tkoj.03", 0x000000, 0x80000, CRC(11f5452f) SHA1(1575729cdbb857a3a780df6e3e0efd6968926fb8) )
	ROM_LOAD16_WORD_SWAP( "tkoj.04", 0x080000, 0x80000, CRC(68655378) SHA1(a2d82996394cc28622e93f6c338f9b78aa798775) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROMX_LOAD( "tkoj1_d.simm1",   0x0000000, 0x200000, CRC(6933377d) SHA1(a79e129e5faaadd401379905ac76a24fa616d736) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "tkoj1_c.simm1",   0x0000001, 0x200000, CRC(7f668950) SHA1(247b2b3fa24afd43b0fe6cfb3df987a38c7385cf) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "tkoj1_b.simm1",   0x0000002, 0x200000, CRC(cfb68ca9) SHA1(36460724b8df36a4ccf88228d9d5f027714c6628) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "tkoj1_a.simm1",   0x0000003, 0x200000, CRC(437e21c5) SHA1(2c4ace6fa421c91effab8ab8db931b8451b8e6b1) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "tkoj3_d.simm3",   0x0000004, 0x200000, CRC(a9e32b57) SHA1(0ddbfefa0cc110e46297ecbfbf4b8bc87ce43c95) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "tkoj3_c.simm3",   0x0000005, 0x200000, CRC(b7ab9338) SHA1(0bb57640eed167e672b5f40cdff0a7b177ff2507) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "tkoj3_b.simm3",   0x0000006, 0x200000, CRC(4d3f919a) SHA1(eb7f90415a263f0a2b94d9699d72c1d14b4fdaad) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "tkoj3_a.simm3",   0x0000007, 0x200000, CRC(cfef17ab) SHA1(1de738c1e537c3df80171e82bc1cc05a25d9cc13) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "tko.01",   0x00000, 0x08000, CRC(6eda50c2) SHA1(7e67c104094a3ced8b3fdd81f52ee42483b30fc5) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_BYTE( "tkoj5_a.simm5",   0x000000, 0x200000, CRC(ab45d509) SHA1(c58cf87d3828dfe0643cf4c58615f3352bd45508) ) // ROM on a simm
	ROM_LOAD16_BYTE( "tkoj5_b.simm5",   0x000001, 0x200000, CRC(fa905c3d) SHA1(3eae65b01d50ec4ec4aeff49f434b9b88a50463c) ) // ROM on a simm
ROM_END

ROM_START( csclubj )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "cscj.03", 0x000000, 0x80000, CRC(ec4ddaa2) SHA1(f84af8bd01cc994ecd6ac24e829e2bd33817d862) )
	ROM_LOAD16_WORD_SWAP( "cscj.04", 0x080000, 0x80000, CRC(60c632bb) SHA1(0d42c33aa476d2cc4efcdad78667353b88225966) )
	ROM_LOAD16_WORD_SWAP( "cscj.05", 0x100000, 0x80000, CRC(ad042003) SHA1(1e167c88f3b0617c38c9f43bdc816045ac0296e0) )
	ROM_LOAD16_WORD_SWAP( "cscj.06", 0x180000, 0x80000, CRC(169e4d40) SHA1(6540d89df5e76189d32b696be7626087fe26e33b) )
	ROM_LOAD16_WORD_SWAP( "csc.07",  0x200000, 0x80000, CRC(01b05caa) SHA1(5b84487da68e6b6f2889c76bf9e070e25941988c) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_FILL(              0x000000, 0x800000, 0 )
	ROMX_LOAD( "csc.14m",  0x800000, 0x200000, CRC(e8904afa) SHA1(39713ffca4e3a754c7c44c0ef4d99fb5a77d8da7) , ROM_GROUPWORD | ROM_SKIP(6) ) /* roms 73 to 76 joined in all eprom version */
	ROMX_LOAD( "csc.16m",  0x800002, 0x200000, CRC(c98c8079) SHA1(22d68ba2ef62b51981bb3e99ec2cde8d1b36514b) , ROM_GROUPWORD | ROM_SKIP(6) ) /* roms 63 to 66 joined in all eprom version */
	ROMX_LOAD( "csc.18m",  0x800004, 0x200000, CRC(c030df5a) SHA1(6d5e5a05531e168d0d44c591f9185ae300908fc2) , ROM_GROUPWORD | ROM_SKIP(6) ) /* roms 83 to 86 joined in all eprom version */
	ROMX_LOAD( "csc.20m",  0x800006, 0x200000, CRC(b4e55863) SHA1(da66f0a36266b906e4c149aec152c323bb184c57) , ROM_GROUPWORD | ROM_SKIP(6) ) /* roms 93 to 96 joined in all eprom version */

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "csc.01",   0x00000, 0x08000, CRC(ee162111) SHA1(ce8d4bd32bb10ee8b0274ba6fcef05a583b39d48) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "csc.11m",   0x000000, 0x200000, CRC(a027b827) SHA1(6d58a63efc7bd5d07353d9b55826c01a3c416c33) ) /* roms 51 to 54 joined in all eprom version */
	ROM_LOAD16_WORD_SWAP( "csc.12m",   0x200000, 0x200000, CRC(cb7f6e55) SHA1(b64e6b663fd09e887d2dc0f4b545e88688c0af55) ) /* roms 55 to 58 joined in all eprom version */
ROM_END

ROM_START( csclub )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "csce.03a", 0x000000, 0x80000, CRC(824082be) SHA1(a44e7e17d73e089b4e7784317f2387d135d01482) )
	ROM_LOAD16_WORD_SWAP( "csce.04a", 0x080000, 0x80000, CRC(74e6a4fe) SHA1(2a732a6e57088885e77256eb17ddd3ae523db09f) )
	ROM_LOAD16_WORD_SWAP( "csce.05a", 0x100000, 0x80000, CRC(8ae0df19) SHA1(88e7bf1ee0e18d74748cecd875cb96524dfa01cf) )
	ROM_LOAD16_WORD_SWAP( "csce.06a", 0x180000, 0x80000, CRC(51f2f0d3) SHA1(067a0bded69767b9f30073012ad62f4608b7610a) )
	ROM_LOAD16_WORD_SWAP( "csce.07a", 0x200000, 0x80000, CRC(003968fd) SHA1(95f59a29a404a1c1e86d5f43526a80aa97f25621) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_FILL(              0x000000, 0x800000, 0 )
	ROMX_LOAD( "csc.73",  0x800000, 0x080000, CRC(335f07c3) SHA1(44e0385120e2c81fd1072e19b7e3ff05d42be226) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "csc.74",  0xa00000, 0x080000, CRC(ab215357) SHA1(c2600b5ba62a570f8a32de9c06f93ea8ae6cd854) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "csc.75",  0xc00000, 0x080000, CRC(a2367381) SHA1(49d562be42c10cdf4e55e3b76c388f0b0121a967) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "csc.76",  0xe00000, 0x080000, CRC(728aac1f) SHA1(2d04e8803f41af2372fc342fcd8a076c14338198) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "csc.63",  0x800002, 0x080000, CRC(3711b8ca) SHA1(ff7ba4f73d227212377327f4fcbe8f555ac4b9c9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "csc.64",  0xa00002, 0x080000, CRC(828a06d8) SHA1(ce1d147f1d747fcd90d93f7dcca0cdd24c85a971) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "csc.65",  0xc00002, 0x080000, CRC(86ee4569) SHA1(725d82a1669f6f78fb4ef8756ceac8b444dcbd43) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "csc.66",  0xe00002, 0x080000, CRC(c24f577f) SHA1(d3b97091e1f0171e087feb9d497c94a48872cd21) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "csc.83",  0x800004, 0x080000, CRC(0750d12a) SHA1(4c36cba88c58bdbfed923f56a6d489f42a4d0f6e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "csc.84",  0xa00004, 0x080000, CRC(90a92f39) SHA1(75bef2440147bac417b48d61ff64b71ed5b9eb67) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "csc.85",  0xc00004, 0x080000, CRC(d08ab012) SHA1(7f2e71b6bd85c6a3efbd417977261c21f3d6cb7c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "csc.86",  0xe00004, 0x080000, CRC(41652583) SHA1(7132647dc9a29ce98866f489140703c5ed3d6051) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "csc.93",  0x800006, 0x080000, CRC(a756c7f7) SHA1(d0f44f8ded12291d5c79282eac45d088cb365b09) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "csc.94",  0xa00006, 0x080000, CRC(fb7ccc73) SHA1(763e6fe1baf73341747c74d89df9443561fca4fa) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "csc.95",  0xc00006, 0x080000, CRC(4d014297) SHA1(df172a3723793b9955ff1f65e76fe5c20fc37b89) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "csc.96",  0xe00006, 0x080000, CRC(6754b1ef) SHA1(ab5d62056b19999ccecd0eeec7b7c5869ca8fea8) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "csc.01",   0x00000, 0x08000, CRC(ee162111) SHA1(ce8d4bd32bb10ee8b0274ba6fcef05a583b39d48) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "csc.51",   0x000000, 0x080000, CRC(5a52afd5) SHA1(ab873ec556933b75be82ba249357cceb01a7a0bd) )
	ROM_LOAD16_WORD_SWAP( "csc.52",   0x080000, 0x080000, CRC(1408a811) SHA1(1d31e3cd505245b5ff4d3c55d5fa7017c0f1e168) )
	ROM_LOAD16_WORD_SWAP( "csc.53",   0x100000, 0x080000, CRC(4fb9f57c) SHA1(093e8e3a03b62783a84fe4ae239e9eb46cbfd71e) )
	ROM_LOAD16_WORD_SWAP( "csc.54",   0x180000, 0x080000, CRC(9a8f40ec) SHA1(c8db1ecfd6b08e9c83ae53a1d25c1387ab95535c) )
	ROM_LOAD16_WORD_SWAP( "csc.55",   0x200000, 0x080000, CRC(91529a91) SHA1(183569100ae98e17688e0e25932850e73a41eb88) )
	ROM_LOAD16_WORD_SWAP( "csc.56",   0x280000, 0x080000, CRC(9a345334) SHA1(330291400a73215c9797457507a86c90ba415247))
	ROM_LOAD16_WORD_SWAP( "csc.57",   0x300000, 0x080000, CRC(aedc27f2) SHA1(55137f0f22c4823558e6a8ba76011695579a4f1f) )
	ROM_LOAD16_WORD_SWAP( "csc.58",   0x380000, 0x080000, CRC(2300b7b3) SHA1(f5ecbb45c24f7de1c1aa435870695551d4e343ca) )
ROM_END


ROM_START( csclub1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "csce.03", 0x000000, 0x80000, CRC(f2c852ef) SHA1(bc2d403958640d7ab0785d01a3df79ec31d0c239) )
	ROM_LOAD16_WORD_SWAP( "csce.04", 0x080000, 0x80000, CRC(1184530f) SHA1(18565f6a06e6078fc20dd9cf70802ac1da60c67a) )
	ROM_LOAD16_WORD_SWAP( "csce.05", 0x100000, 0x80000, CRC(804e2b6b) SHA1(e638f73442e3165ace84cdb1bd2a9d419e2d8c41) )
	ROM_LOAD16_WORD_SWAP( "csce.06", 0x180000, 0x80000, CRC(09277cb9) SHA1(51a0d335b5d6cde61c32f4e7ea49403f400db7fb) )
	ROM_LOAD16_WORD_SWAP( "csc.07",  0x200000, 0x80000, CRC(01b05caa) SHA1(5b84487da68e6b6f2889c76bf9e070e25941988c) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_FILL(              0x000000, 0x800000, 0 )
	ROMX_LOAD( "csc.14m",  0x800000, 0x200000, CRC(e8904afa) SHA1(39713ffca4e3a754c7c44c0ef4d99fb5a77d8da7) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "csc.16m",  0x800002, 0x200000, CRC(c98c8079) SHA1(22d68ba2ef62b51981bb3e99ec2cde8d1b36514b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "csc.18m",  0x800004, 0x200000, CRC(c030df5a) SHA1(6d5e5a05531e168d0d44c591f9185ae300908fc2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "csc.20m",  0x800006, 0x200000, CRC(b4e55863) SHA1(da66f0a36266b906e4c149aec152c323bb184c57) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "csc.01",   0x00000, 0x08000, CRC(ee162111) SHA1(ce8d4bd32bb10ee8b0274ba6fcef05a583b39d48) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "csc.11m",   0x000000, 0x200000, CRC(a027b827) SHA1(6d58a63efc7bd5d07353d9b55826c01a3c416c33) )
	ROM_LOAD16_WORD_SWAP( "csc.12m",   0x200000, 0x200000, CRC(cb7f6e55) SHA1(b64e6b663fd09e887d2dc0f4b545e88688c0af55) )
ROM_END

ROM_START( cscluba )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "csca.03", 0x000000, 0x80000, CRC(b6acd708) SHA1(27d316053b0e74b1e9db979d500767cfa49fbce3) )
	ROM_LOAD16_WORD_SWAP( "csca.04", 0x080000, 0x80000, CRC(d44ae35f) SHA1(cd464792fe777183b0b0587239fb1b52bd7f9ec7) )
	ROM_LOAD16_WORD_SWAP( "csca.05", 0x100000, 0x80000, CRC(8da76aec) SHA1(04552f2c9c424d808703136a7909df903aec290a) )
	ROM_LOAD16_WORD_SWAP( "csca.06", 0x180000, 0x80000, CRC(a1b7b1ee) SHA1(77ba745f094a29521bb686982399b8b9babd7cc6) )
	ROM_LOAD16_WORD_SWAP( "csc.07",  0x200000, 0x80000, CRC(01b05caa) SHA1(5b84487da68e6b6f2889c76bf9e070e25941988c) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_FILL(              0x000000, 0x800000, 0 )
	ROMX_LOAD( "csc.14m",  0x800000, 0x200000, CRC(e8904afa) SHA1(39713ffca4e3a754c7c44c0ef4d99fb5a77d8da7) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "csc.16m",  0x800002, 0x200000, CRC(c98c8079) SHA1(22d68ba2ef62b51981bb3e99ec2cde8d1b36514b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "csc.18m",  0x800004, 0x200000, CRC(c030df5a) SHA1(6d5e5a05531e168d0d44c591f9185ae300908fc2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "csc.20m",  0x800006, 0x200000, CRC(b4e55863) SHA1(da66f0a36266b906e4c149aec152c323bb184c57) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "csc.01",   0x00000, 0x08000, CRC(ee162111) SHA1(ce8d4bd32bb10ee8b0274ba6fcef05a583b39d48) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "csc.11m",   0x000000, 0x200000, CRC(a027b827) SHA1(6d58a63efc7bd5d07353d9b55826c01a3c416c33) )
	ROM_LOAD16_WORD_SWAP( "csc.12m",   0x200000, 0x200000, CRC(cb7f6e55) SHA1(b64e6b663fd09e887d2dc0f4b545e88688c0af55) )
ROM_END

ROM_START( csclubh )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "csch.03", 0x000000, 0x80000, CRC(0dd7e46d) SHA1(deacd350b8954998636065cf070c9955d08402b8) )
	ROM_LOAD16_WORD_SWAP( "csch.04", 0x080000, 0x80000, CRC(486e8143) SHA1(d50ab8a5fdc194a9cded74cff94e5b3b69069826) )
	ROM_LOAD16_WORD_SWAP( "csch.05", 0x100000, 0x80000, CRC(9e509dfb) SHA1(4a6cd8488a63ad3f7d5a08f2a6af4728dc147790) )
	ROM_LOAD16_WORD_SWAP( "csch.06", 0x180000, 0x80000, CRC(817ba313) SHA1(674e10e642c09d26886f3deb829dee330ff472be) )
	ROM_LOAD16_WORD_SWAP( "csc.07",  0x200000, 0x80000, CRC(01b05caa) SHA1(5b84487da68e6b6f2889c76bf9e070e25941988c) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_FILL(              0x000000, 0x800000, 0 )
	ROMX_LOAD( "csc.14m",  0x800000, 0x200000, CRC(e8904afa) SHA1(39713ffca4e3a754c7c44c0ef4d99fb5a77d8da7) , ROM_GROUPWORD | ROM_SKIP(6) ) /* roms 73 to 76 joined in all eprom version */
	ROMX_LOAD( "csc.16m",  0x800002, 0x200000, CRC(c98c8079) SHA1(22d68ba2ef62b51981bb3e99ec2cde8d1b36514b) , ROM_GROUPWORD | ROM_SKIP(6) ) /* roms 63 to 66 joined in all eprom version */
	ROMX_LOAD( "csc.18m",  0x800004, 0x200000, CRC(c030df5a) SHA1(6d5e5a05531e168d0d44c591f9185ae300908fc2) , ROM_GROUPWORD | ROM_SKIP(6) ) /* roms 83 to 86 joined in all eprom version */
	ROMX_LOAD( "csc.20m",  0x800006, 0x200000, CRC(b4e55863) SHA1(da66f0a36266b906e4c149aec152c323bb184c57) , ROM_GROUPWORD | ROM_SKIP(6) ) /* roms 93 to 96 joined in all eprom version */

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "csc.01",   0x00000, 0x08000, CRC(ee162111) SHA1(ce8d4bd32bb10ee8b0274ba6fcef05a583b39d48) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "csc.11m",   0x000000, 0x200000, CRC(a027b827) SHA1(6d58a63efc7bd5d07353d9b55826c01a3c416c33) ) /* roms 51 to 54 joined in all eprom version */
	ROM_LOAD16_WORD_SWAP( "csc.12m",   0x200000, 0x200000, CRC(cb7f6e55) SHA1(b64e6b663fd09e887d2dc0f4b545e88688c0af55) ) /* roms 55 to 58 joined in all eprom version */
ROM_END

ROM_START( cybots )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "cybe.03", 0x000000, 0x80000, CRC(234381cd) SHA1(6202a2a318feda525e16fd7b31c03af9ccc5d964) )
	ROM_LOAD16_WORD_SWAP( "cybe.04", 0x080000, 0x80000, CRC(80691061) SHA1(4f3ef24fc76d3a5b369aa6192ad390d9c3c9b0e8) )
	ROM_LOAD16_WORD_SWAP( "cyb.05",  0x100000, 0x80000, CRC(ec40408e) SHA1(dd611c1708e7ef86e4f7cac4b7b0dff7baaee5ed) )
	ROM_LOAD16_WORD_SWAP( "cyb.06",  0x180000, 0x80000, CRC(1ad0bed2) SHA1(2ea005f3e73b05f8f0ec006cd9e95f7731a73897) )
	ROM_LOAD16_WORD_SWAP( "cyb.07",  0x200000, 0x80000, CRC(6245a39a) SHA1(4f607e733e2dea80211497522be6d0f09571928d) )
	ROM_LOAD16_WORD_SWAP( "cyb.08",  0x280000, 0x80000, CRC(4b48e223) SHA1(9714579a7a78b9716e44bca6c18bf1a93aa4e482) )
	ROM_LOAD16_WORD_SWAP( "cyb.09",  0x300000, 0x80000, CRC(e15238f6) SHA1(16abd92ebed921a6a7e8eac4b098dc61f7e5485c) )
	ROM_LOAD16_WORD_SWAP( "cyb.10",  0x380000, 0x80000, CRC(75f4003b) SHA1(8a65026ae35247cda016ce85a34034c62b3aa1a6) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "cyb.13m",   0x0000000, 0x400000, CRC(f0dce192) SHA1(b743938dc8e772dc3f63ed88a4a54c34fffdba21) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "cyb.15m",   0x0000002, 0x400000, CRC(187aa39c) SHA1(80e3cf5c69f13343de667e1476bb716d45d3ff63) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "cyb.17m",   0x0000004, 0x400000, CRC(8a0e4b12) SHA1(40132f3cc79b0a74460ebd4e0d4ddbe240efc06f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "cyb.19m",   0x0000006, 0x400000, CRC(34b62612) SHA1(154bbceb7d303a208abb1b2f3d507d5afacc71ed) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "cyb.14m",   0x1000000, 0x400000, CRC(c1537957) SHA1(bfb1cc6786277b94ce28bfd464e2bbb6f6d3486e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "cyb.16m",   0x1000002, 0x400000, CRC(15349e86) SHA1(b0cde577d29a9f4e718b673c8645529ef0ababc9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "cyb.18m",   0x1000004, 0x400000, CRC(d83e977d) SHA1(e03f4a120c95a2f476ffc8492bca85e0c5cea068) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "cyb.20m",   0x1000006, 0x400000, CRC(77cdad5c) SHA1(94d0cc5f05de4bc2d43977d91f887005dc10310c) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "cyb.01",   0x00000, 0x08000, CRC(9c0fb079) SHA1(06d260875a76da08d56ea2b2ae277e8c2dbae6e3) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "cyb.02",   0x28000, 0x20000, CRC(51cb0c4e) SHA1(c322957558d8d3e9dad090aebbe485978cbce8f5) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "cyb.11m",   0x000000, 0x200000, CRC(362ccab2) SHA1(28e537067d4846f22657ee37166d18b8f05f4da1) )
	ROM_LOAD16_WORD_SWAP( "cyb.12m",   0x200000, 0x200000, CRC(7066e9cc) SHA1(eb6a9d4998b3311344d73bae88d661d81609c492) )
ROM_END

ROM_START( cybotsu )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "cybu.03", 0x000000, 0x80000, CRC(db4da8f4) SHA1(de9f3f261003f4f70ae32114a15e498387c23f6d) )
	ROM_LOAD16_WORD_SWAP( "cybu.04", 0x080000, 0x80000, CRC(1eec68ac) SHA1(b2b9379c84b121048cb83a8c48756b48cdbc3ea1) )
	ROM_LOAD16_WORD_SWAP( "cyb.05",  0x100000, 0x80000, CRC(ec40408e) SHA1(dd611c1708e7ef86e4f7cac4b7b0dff7baaee5ed) )
	ROM_LOAD16_WORD_SWAP( "cyb.06",  0x180000, 0x80000, CRC(1ad0bed2) SHA1(2ea005f3e73b05f8f0ec006cd9e95f7731a73897) )
	ROM_LOAD16_WORD_SWAP( "cyb.07",  0x200000, 0x80000, CRC(6245a39a) SHA1(4f607e733e2dea80211497522be6d0f09571928d) )
	ROM_LOAD16_WORD_SWAP( "cyb.08",  0x280000, 0x80000, CRC(4b48e223) SHA1(9714579a7a78b9716e44bca6c18bf1a93aa4e482) )
	ROM_LOAD16_WORD_SWAP( "cyb.09",  0x300000, 0x80000, CRC(e15238f6) SHA1(16abd92ebed921a6a7e8eac4b098dc61f7e5485c) )
	ROM_LOAD16_WORD_SWAP( "cyb.10",  0x380000, 0x80000, CRC(75f4003b) SHA1(8a65026ae35247cda016ce85a34034c62b3aa1a6) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "cyb.13m",   0x0000000, 0x400000, CRC(f0dce192) SHA1(b743938dc8e772dc3f63ed88a4a54c34fffdba21) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "cyb.15m",   0x0000002, 0x400000, CRC(187aa39c) SHA1(80e3cf5c69f13343de667e1476bb716d45d3ff63) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "cyb.17m",   0x0000004, 0x400000, CRC(8a0e4b12) SHA1(40132f3cc79b0a74460ebd4e0d4ddbe240efc06f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "cyb.19m",   0x0000006, 0x400000, CRC(34b62612) SHA1(154bbceb7d303a208abb1b2f3d507d5afacc71ed) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "cyb.14m",   0x1000000, 0x400000, CRC(c1537957) SHA1(bfb1cc6786277b94ce28bfd464e2bbb6f6d3486e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "cyb.16m",   0x1000002, 0x400000, CRC(15349e86) SHA1(b0cde577d29a9f4e718b673c8645529ef0ababc9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "cyb.18m",   0x1000004, 0x400000, CRC(d83e977d) SHA1(e03f4a120c95a2f476ffc8492bca85e0c5cea068) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "cyb.20m",   0x1000006, 0x400000, CRC(77cdad5c) SHA1(94d0cc5f05de4bc2d43977d91f887005dc10310c) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "cyb.01",   0x00000, 0x08000, CRC(9c0fb079) SHA1(06d260875a76da08d56ea2b2ae277e8c2dbae6e3) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "cyb.02",   0x28000, 0x20000, CRC(51cb0c4e) SHA1(c322957558d8d3e9dad090aebbe485978cbce8f5) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "cyb.11m",   0x000000, 0x200000, CRC(362ccab2) SHA1(28e537067d4846f22657ee37166d18b8f05f4da1) )
	ROM_LOAD16_WORD_SWAP( "cyb.12m",   0x200000, 0x200000, CRC(7066e9cc) SHA1(eb6a9d4998b3311344d73bae88d661d81609c492) )
ROM_END

ROM_START( cybotsj )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "cybj.03", 0x000000, 0x80000, CRC(6096eada) SHA1(ea3fa2e6229d90cc3f69c59f447b6b373d64d2aa) )
	ROM_LOAD16_WORD_SWAP( "cybj.04", 0x080000, 0x80000, CRC(7b0ffaa9) SHA1(595c3e679ea02282bf8a5aa6c7c09e5c30e839c7) )
	ROM_LOAD16_WORD_SWAP( "cyb.05",  0x100000, 0x80000, CRC(ec40408e) SHA1(dd611c1708e7ef86e4f7cac4b7b0dff7baaee5ed) )
	ROM_LOAD16_WORD_SWAP( "cyb.06",  0x180000, 0x80000, CRC(1ad0bed2) SHA1(2ea005f3e73b05f8f0ec006cd9e95f7731a73897) )
	ROM_LOAD16_WORD_SWAP( "cyb.07",  0x200000, 0x80000, CRC(6245a39a) SHA1(4f607e733e2dea80211497522be6d0f09571928d) )
	ROM_LOAD16_WORD_SWAP( "cyb.08",  0x280000, 0x80000, CRC(4b48e223) SHA1(9714579a7a78b9716e44bca6c18bf1a93aa4e482) )
	ROM_LOAD16_WORD_SWAP( "cyb.09",  0x300000, 0x80000, CRC(e15238f6) SHA1(16abd92ebed921a6a7e8eac4b098dc61f7e5485c) )
	ROM_LOAD16_WORD_SWAP( "cyb.10",  0x380000, 0x80000, CRC(75f4003b) SHA1(8a65026ae35247cda016ce85a34034c62b3aa1a6) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "cyb.13m",   0x0000000, 0x400000, CRC(f0dce192) SHA1(b743938dc8e772dc3f63ed88a4a54c34fffdba21) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "cyb.15m",   0x0000002, 0x400000, CRC(187aa39c) SHA1(80e3cf5c69f13343de667e1476bb716d45d3ff63) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "cyb.17m",   0x0000004, 0x400000, CRC(8a0e4b12) SHA1(40132f3cc79b0a74460ebd4e0d4ddbe240efc06f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "cyb.19m",   0x0000006, 0x400000, CRC(34b62612) SHA1(154bbceb7d303a208abb1b2f3d507d5afacc71ed) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "cyb.14m",   0x1000000, 0x400000, CRC(c1537957) SHA1(bfb1cc6786277b94ce28bfd464e2bbb6f6d3486e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "cyb.16m",   0x1000002, 0x400000, CRC(15349e86) SHA1(b0cde577d29a9f4e718b673c8645529ef0ababc9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "cyb.18m",   0x1000004, 0x400000, CRC(d83e977d) SHA1(e03f4a120c95a2f476ffc8492bca85e0c5cea068) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "cyb.20m",   0x1000006, 0x400000, CRC(77cdad5c) SHA1(94d0cc5f05de4bc2d43977d91f887005dc10310c) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "cyb.01",   0x00000, 0x08000, CRC(9c0fb079) SHA1(06d260875a76da08d56ea2b2ae277e8c2dbae6e3) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "cyb.02",   0x28000, 0x20000, CRC(51cb0c4e) SHA1(c322957558d8d3e9dad090aebbe485978cbce8f5) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "cyb.11m",   0x000000, 0x200000, CRC(362ccab2) SHA1(28e537067d4846f22657ee37166d18b8f05f4da1) )
	ROM_LOAD16_WORD_SWAP( "cyb.12m",   0x200000, 0x200000, CRC(7066e9cc) SHA1(eb6a9d4998b3311344d73bae88d661d81609c492) )
ROM_END

ROM_START( ddtod )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "dade.03c", 0x000000, 0x80000, CRC(8e73533d) SHA1(6eece222e562dd0c453d8dec188c9553c46dfe3c) )
	ROM_LOAD16_WORD_SWAP( "dade.04c", 0x080000, 0x80000, CRC(00c2e82e) SHA1(fad4dcdac8d6ef04b71e987936bf27e3d93809fc) )
	ROM_LOAD16_WORD_SWAP( "dade.05c", 0x100000, 0x80000, CRC(ea996008) SHA1(9f41679531e971e62483415c07ef4ee7489ff779) )
	ROM_LOAD16_WORD_SWAP( "dad.06a",  0x180000, 0x80000, CRC(6225495a) SHA1(a9a02abb072e3482ac92d7aed8ce9a5bcf636bc0) )
	ROM_LOAD16_WORD_SWAP( "dad.07a",  0x200000, 0x80000, CRC(b3480ec3) SHA1(a66f8dba67101fd71c2af4f3c3d71e55778a9f2c) )

	ROM_REGION( 0xc00000, "gfx", 0 )
	ROMX_LOAD( "dad.13m",   0x000000, 0x200000, CRC(da3cb7d6) SHA1(d59bb53d5f32889eb6eb7f8b1c8781948c97283d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.15m",   0x000002, 0x200000, CRC(92b63172) SHA1(9bed7dbbb17729f2ad3d318396f5335c0bd39937) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.17m",   0x000004, 0x200000, CRC(b98757f5) SHA1(3eead22e097906bf0e1e151cd0a9c75abc5a32d4) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.19m",   0x000006, 0x200000, CRC(8121ce46) SHA1(40c4dc969318d38f0c6d5401c9c64371f51aa12c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.14m",   0x800000, 0x100000, CRC(837e6f3f) SHA1(c060183474fba0e82d765b9f282b84838550dff6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.16m",   0x800002, 0x100000, CRC(f0916bdb) SHA1(9354d258dd26cbbf12c78ecfc277c357cbdb360e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.18m",   0x800004, 0x100000, CRC(cef393ef) SHA1(830b33c86cc24776d17ad65fa89a3b16c40446a1) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.20m",   0x800006, 0x100000, CRC(8953fe9e) SHA1(f4795beb006335d13e3934aa9760e775eb0bb950) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "dad.01",   0x00000, 0x08000, CRC(3f5e2424) SHA1(4aa744576bc6752c43a90a27a816ebd90076b248) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "dad.11m",   0x000000, 0x200000, CRC(0c499b67) SHA1(a8ebd8a1cd6dece8344b7cb0439d85843fb97616) )
	ROM_LOAD16_WORD_SWAP( "dad.12m",   0x200000, 0x200000, CRC(2f0b5a4e) SHA1(8d1ebbb811aa469b0f0d29d719d2b9af28fb63a2) )
ROM_END

ROM_START( ddtodr1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "dade.03a", 0x000000, 0x80000, CRC(665a035e) SHA1(4aa81f7055bc288be1282dcbf9a33c77d3c963f5) )
	ROM_LOAD16_WORD_SWAP( "dade.04a", 0x080000, 0x80000, CRC(02613207) SHA1(a29258848e8f6ac7469c88668a83e07bf325f96a) )
	ROM_LOAD16_WORD_SWAP( "dade.05a", 0x100000, 0x80000, CRC(36845996) SHA1(a767564b62c1e25c62e4d31201f10d8c4a368197) )
	ROM_LOAD16_WORD_SWAP( "dad.06",   0x180000, 0x80000, CRC(13aa3e56) SHA1(ccd3cda528d625bbf4dc0e8c5ad629af6080d705) )
	ROM_LOAD16_WORD_SWAP( "dad.07",   0x200000, 0x80000, CRC(431cb6dd) SHA1(ad3342e2fb8f0b3d7f57e845d5b80a871923324d) )

	ROM_REGION( 0xc00000, "gfx", 0 )
	ROMX_LOAD( "dad.13m",   0x000000, 0x200000, CRC(da3cb7d6) SHA1(d59bb53d5f32889eb6eb7f8b1c8781948c97283d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.15m",   0x000002, 0x200000, CRC(92b63172) SHA1(9bed7dbbb17729f2ad3d318396f5335c0bd39937) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.17m",   0x000004, 0x200000, CRC(b98757f5) SHA1(3eead22e097906bf0e1e151cd0a9c75abc5a32d4) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.19m",   0x000006, 0x200000, CRC(8121ce46) SHA1(40c4dc969318d38f0c6d5401c9c64371f51aa12c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.14m",   0x800000, 0x100000, CRC(837e6f3f) SHA1(c060183474fba0e82d765b9f282b84838550dff6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.16m",   0x800002, 0x100000, CRC(f0916bdb) SHA1(9354d258dd26cbbf12c78ecfc277c357cbdb360e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.18m",   0x800004, 0x100000, CRC(cef393ef) SHA1(830b33c86cc24776d17ad65fa89a3b16c40446a1) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.20m",   0x800006, 0x100000, CRC(8953fe9e) SHA1(f4795beb006335d13e3934aa9760e775eb0bb950) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "dad.01",   0x00000, 0x08000, CRC(3f5e2424) SHA1(4aa744576bc6752c43a90a27a816ebd90076b248) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "dad.11m",   0x000000, 0x200000, CRC(0c499b67) SHA1(a8ebd8a1cd6dece8344b7cb0439d85843fb97616) )
	ROM_LOAD16_WORD_SWAP( "dad.12m",   0x200000, 0x200000, CRC(2f0b5a4e) SHA1(8d1ebbb811aa469b0f0d29d719d2b9af28fb63a2) )
ROM_END

ROM_START( ddtodu )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "dadu.03b", 0x000000, 0x80000, CRC(a519905f) SHA1(7f846d7ac5d5e0d06657f712a7a09bee984a4f4b) )
	ROM_LOAD16_WORD_SWAP( "dadu.04b", 0x080000, 0x80000, CRC(52562d38) SHA1(3ee21399a19ee5e2db2a8c2a893d8a31a3419399) )
	ROM_LOAD16_WORD_SWAP( "dadu.05b", 0x100000, 0x80000, CRC(ee1cfbfe) SHA1(4107e495827ada1712a2393dffcdf52d98aca2e0) )
	ROM_LOAD16_WORD_SWAP( "dad.06",   0x180000, 0x80000, CRC(13aa3e56) SHA1(ccd3cda528d625bbf4dc0e8c5ad629af6080d705) )
	ROM_LOAD16_WORD_SWAP( "dad.07",   0x200000, 0x80000, CRC(431cb6dd) SHA1(ad3342e2fb8f0b3d7f57e845d5b80a871923324d) )

	ROM_REGION( 0xc00000, "gfx", 0 )
	ROMX_LOAD( "dad.13m",   0x000000, 0x200000, CRC(da3cb7d6) SHA1(d59bb53d5f32889eb6eb7f8b1c8781948c97283d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.15m",   0x000002, 0x200000, CRC(92b63172) SHA1(9bed7dbbb17729f2ad3d318396f5335c0bd39937) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.17m",   0x000004, 0x200000, CRC(b98757f5) SHA1(3eead22e097906bf0e1e151cd0a9c75abc5a32d4) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.19m",   0x000006, 0x200000, CRC(8121ce46) SHA1(40c4dc969318d38f0c6d5401c9c64371f51aa12c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.14m",   0x800000, 0x100000, CRC(837e6f3f) SHA1(c060183474fba0e82d765b9f282b84838550dff6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.16m",   0x800002, 0x100000, CRC(f0916bdb) SHA1(9354d258dd26cbbf12c78ecfc277c357cbdb360e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.18m",   0x800004, 0x100000, CRC(cef393ef) SHA1(830b33c86cc24776d17ad65fa89a3b16c40446a1) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.20m",   0x800006, 0x100000, CRC(8953fe9e) SHA1(f4795beb006335d13e3934aa9760e775eb0bb950) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "dad.01",   0x00000, 0x08000, CRC(3f5e2424) SHA1(4aa744576bc6752c43a90a27a816ebd90076b248) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "dad.11m",   0x000000, 0x200000, CRC(0c499b67) SHA1(a8ebd8a1cd6dece8344b7cb0439d85843fb97616) )
	ROM_LOAD16_WORD_SWAP( "dad.12m",   0x200000, 0x200000, CRC(2f0b5a4e) SHA1(8d1ebbb811aa469b0f0d29d719d2b9af28fb63a2) )
ROM_END

ROM_START( ddtodur1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "dadu.03a", 0x000000, 0x80000, CRC(4413f177) SHA1(26c8d06adc83ffc5bec4abf05aa64e874e85d539) )
	ROM_LOAD16_WORD_SWAP( "dadu.04a", 0x080000, 0x80000, CRC(168de230) SHA1(3f8af1625bb0d9097e538f8ba7cd23d95b0233aa) )
	ROM_LOAD16_WORD_SWAP( "dadu.05a", 0x100000, 0x80000, CRC(03d39e91) SHA1(92461b87c55cb41bbe89bcb3e3f2e9b1ed521067) )
	ROM_LOAD16_WORD_SWAP( "dad.06",   0x180000, 0x80000, CRC(13aa3e56) SHA1(ccd3cda528d625bbf4dc0e8c5ad629af6080d705) )
	ROM_LOAD16_WORD_SWAP( "dad.07",   0x200000, 0x80000, CRC(431cb6dd) SHA1(ad3342e2fb8f0b3d7f57e845d5b80a871923324d) )

	ROM_REGION( 0xc00000, "gfx", 0 )
	ROMX_LOAD( "dad.13m",   0x000000, 0x200000, CRC(da3cb7d6) SHA1(d59bb53d5f32889eb6eb7f8b1c8781948c97283d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.15m",   0x000002, 0x200000, CRC(92b63172) SHA1(9bed7dbbb17729f2ad3d318396f5335c0bd39937) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.17m",   0x000004, 0x200000, CRC(b98757f5) SHA1(3eead22e097906bf0e1e151cd0a9c75abc5a32d4) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.19m",   0x000006, 0x200000, CRC(8121ce46) SHA1(40c4dc969318d38f0c6d5401c9c64371f51aa12c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.14m",   0x800000, 0x100000, CRC(837e6f3f) SHA1(c060183474fba0e82d765b9f282b84838550dff6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.16m",   0x800002, 0x100000, CRC(f0916bdb) SHA1(9354d258dd26cbbf12c78ecfc277c357cbdb360e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.18m",   0x800004, 0x100000, CRC(cef393ef) SHA1(830b33c86cc24776d17ad65fa89a3b16c40446a1) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.20m",   0x800006, 0x100000, CRC(8953fe9e) SHA1(f4795beb006335d13e3934aa9760e775eb0bb950) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "dad.01",   0x00000, 0x08000, CRC(3f5e2424) SHA1(4aa744576bc6752c43a90a27a816ebd90076b248) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "dad.11m",   0x000000, 0x200000, CRC(0c499b67) SHA1(a8ebd8a1cd6dece8344b7cb0439d85843fb97616) )
	ROM_LOAD16_WORD_SWAP( "dad.12m",   0x200000, 0x200000, CRC(2f0b5a4e) SHA1(8d1ebbb811aa469b0f0d29d719d2b9af28fb63a2) )
ROM_END

ROM_START( ddtodj )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "dadj.03c", 0x000000, 0x80000, CRC(0b1b5798) SHA1(ce2749164a6cf4c99db4bfa7e515a9022006cf92) )
	ROM_LOAD16_WORD_SWAP( "dadj.04c", 0x080000, 0x80000, CRC(c6a2fbc8) SHA1(cb7105e9d35c9e64f5535eb5f491e3f2cf6de64e) )
	ROM_LOAD16_WORD_SWAP( "dadj.05c", 0x100000, 0x80000, CRC(189b15fe) SHA1(91baef189549a25122fd5ab238a849bad2766862) )
	ROM_LOAD16_WORD_SWAP( "dad.06a",  0x180000, 0x80000, CRC(6225495a) SHA1(a9a02abb072e3482ac92d7aed8ce9a5bcf636bc0) )
	ROM_LOAD16_WORD_SWAP( "dad.07a",  0x200000, 0x80000, CRC(b3480ec3) SHA1(a66f8dba67101fd71c2af4f3c3d71e55778a9f2c) )

	ROM_REGION( 0xc00000, "gfx", 0 )
	ROMX_LOAD( "dad.13m",   0x000000, 0x200000, CRC(da3cb7d6) SHA1(d59bb53d5f32889eb6eb7f8b1c8781948c97283d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.15m",   0x000002, 0x200000, CRC(92b63172) SHA1(9bed7dbbb17729f2ad3d318396f5335c0bd39937) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.17m",   0x000004, 0x200000, CRC(b98757f5) SHA1(3eead22e097906bf0e1e151cd0a9c75abc5a32d4) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.19m",   0x000006, 0x200000, CRC(8121ce46) SHA1(40c4dc969318d38f0c6d5401c9c64371f51aa12c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.14m",   0x800000, 0x100000, CRC(837e6f3f) SHA1(c060183474fba0e82d765b9f282b84838550dff6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.16m",   0x800002, 0x100000, CRC(f0916bdb) SHA1(9354d258dd26cbbf12c78ecfc277c357cbdb360e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.18m",   0x800004, 0x100000, CRC(cef393ef) SHA1(830b33c86cc24776d17ad65fa89a3b16c40446a1) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.20m",   0x800006, 0x100000, CRC(8953fe9e) SHA1(f4795beb006335d13e3934aa9760e775eb0bb950) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "dad.01",   0x00000, 0x08000, CRC(3f5e2424) SHA1(4aa744576bc6752c43a90a27a816ebd90076b248) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "dad.11m",   0x000000, 0x200000, CRC(0c499b67) SHA1(a8ebd8a1cd6dece8344b7cb0439d85843fb97616) )
	ROM_LOAD16_WORD_SWAP( "dad.12m",   0x200000, 0x200000, CRC(2f0b5a4e) SHA1(8d1ebbb811aa469b0f0d29d719d2b9af28fb63a2) )
ROM_END

ROM_START( ddtodjr1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "dadj.03b", 0x000000, 0x80000, CRC(87606b85) SHA1(1311c9ae321207db4632572f6c838b732974b087) )
	ROM_LOAD16_WORD_SWAP( "dadj.04b", 0x080000, 0x80000, CRC(24d49575) SHA1(419d7d2f970c23c39334a7f2e8c5caa237769c5d) )
	ROM_LOAD16_WORD_SWAP( "dadj.05b", 0x100000, 0x80000, CRC(56ce51f7) SHA1(3a15537c479e3798ec30d5e313a727d7f91955c0) )
	ROM_LOAD16_WORD_SWAP( "dad.06",   0x180000, 0x80000, CRC(13aa3e56) SHA1(ccd3cda528d625bbf4dc0e8c5ad629af6080d705) )
	ROM_LOAD16_WORD_SWAP( "dad.07",   0x200000, 0x80000, CRC(431cb6dd) SHA1(ad3342e2fb8f0b3d7f57e845d5b80a871923324d) )

	ROM_REGION( 0xc00000, "gfx", 0 )
	ROMX_LOAD( "dad.13m",   0x000000, 0x200000, CRC(da3cb7d6) SHA1(d59bb53d5f32889eb6eb7f8b1c8781948c97283d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.15m",   0x000002, 0x200000, CRC(92b63172) SHA1(9bed7dbbb17729f2ad3d318396f5335c0bd39937) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.17m",   0x000004, 0x200000, CRC(b98757f5) SHA1(3eead22e097906bf0e1e151cd0a9c75abc5a32d4) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.19m",   0x000006, 0x200000, CRC(8121ce46) SHA1(40c4dc969318d38f0c6d5401c9c64371f51aa12c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.14m",   0x800000, 0x100000, CRC(837e6f3f) SHA1(c060183474fba0e82d765b9f282b84838550dff6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.16m",   0x800002, 0x100000, CRC(f0916bdb) SHA1(9354d258dd26cbbf12c78ecfc277c357cbdb360e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.18m",   0x800004, 0x100000, CRC(cef393ef) SHA1(830b33c86cc24776d17ad65fa89a3b16c40446a1) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.20m",   0x800006, 0x100000, CRC(8953fe9e) SHA1(f4795beb006335d13e3934aa9760e775eb0bb950) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "dad.01",   0x00000, 0x08000, CRC(3f5e2424) SHA1(4aa744576bc6752c43a90a27a816ebd90076b248) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "dad.11m",   0x000000, 0x200000, CRC(0c499b67) SHA1(a8ebd8a1cd6dece8344b7cb0439d85843fb97616) )
	ROM_LOAD16_WORD_SWAP( "dad.12m",   0x200000, 0x200000, CRC(2f0b5a4e) SHA1(8d1ebbb811aa469b0f0d29d719d2b9af28fb63a2) )
ROM_END

ROM_START( ddtodjr2 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "dadj.03a", 0x000000, 0x80000, CRC(711638dc) SHA1(30c1d1a694aa8e51d072b26b47ba55aed6d77b7b) )
	ROM_LOAD16_WORD_SWAP( "dadj.04a", 0x080000, 0x80000, CRC(4869639c) SHA1(1544813e6712a78267c1d27b6b49148d42c11127) )
	ROM_LOAD16_WORD_SWAP( "dadj.05a", 0x100000, 0x80000, CRC(484c0efa) SHA1(d4ddef54149ef0141dcbe05df5f669fccf462559) )
	ROM_LOAD16_WORD_SWAP( "dad.06",   0x180000, 0x80000, CRC(13aa3e56) SHA1(ccd3cda528d625bbf4dc0e8c5ad629af6080d705) )
	ROM_LOAD16_WORD_SWAP( "dad.07",   0x200000, 0x80000, CRC(431cb6dd) SHA1(ad3342e2fb8f0b3d7f57e845d5b80a871923324d) )

	ROM_REGION( 0xc00000, "gfx", 0 )
	ROMX_LOAD( "dad.13m",   0x000000, 0x200000, CRC(da3cb7d6) SHA1(d59bb53d5f32889eb6eb7f8b1c8781948c97283d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.15m",   0x000002, 0x200000, CRC(92b63172) SHA1(9bed7dbbb17729f2ad3d318396f5335c0bd39937) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.17m",   0x000004, 0x200000, CRC(b98757f5) SHA1(3eead22e097906bf0e1e151cd0a9c75abc5a32d4) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.19m",   0x000006, 0x200000, CRC(8121ce46) SHA1(40c4dc969318d38f0c6d5401c9c64371f51aa12c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.14m",   0x800000, 0x100000, CRC(837e6f3f) SHA1(c060183474fba0e82d765b9f282b84838550dff6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.16m",   0x800002, 0x100000, CRC(f0916bdb) SHA1(9354d258dd26cbbf12c78ecfc277c357cbdb360e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.18m",   0x800004, 0x100000, CRC(cef393ef) SHA1(830b33c86cc24776d17ad65fa89a3b16c40446a1) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.20m",   0x800006, 0x100000, CRC(8953fe9e) SHA1(f4795beb006335d13e3934aa9760e775eb0bb950) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "dad.01",   0x00000, 0x08000, CRC(3f5e2424) SHA1(4aa744576bc6752c43a90a27a816ebd90076b248) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "dad.11m",   0x000000, 0x200000, CRC(0c499b67) SHA1(a8ebd8a1cd6dece8344b7cb0439d85843fb97616) )
	ROM_LOAD16_WORD_SWAP( "dad.12m",   0x200000, 0x200000, CRC(2f0b5a4e) SHA1(8d1ebbb811aa469b0f0d29d719d2b9af28fb63a2) )
ROM_END

ROM_START( ddtoda )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "dada.03a", 0x000000, 0x80000, CRC(fc6f2dd7) SHA1(82f59670ec77a11e9765e2acd0e846d1c768b542) )
	ROM_LOAD16_WORD_SWAP( "dada.04a", 0x080000, 0x80000, CRC(d4be4009) SHA1(c914ddc8f0c237efb52dd1a8f56395b17a6583be) )
	ROM_LOAD16_WORD_SWAP( "dada.05a", 0x100000, 0x80000, CRC(6712d1cf) SHA1(a716ee5ca434badc57f67e0802c6b184bf243dbb) )
	ROM_LOAD16_WORD_SWAP( "dad.06",   0x180000, 0x80000, CRC(13aa3e56) SHA1(ccd3cda528d625bbf4dc0e8c5ad629af6080d705) )
	ROM_LOAD16_WORD_SWAP( "dad.07",   0x200000, 0x80000, CRC(431cb6dd) SHA1(ad3342e2fb8f0b3d7f57e845d5b80a871923324d) )

	ROM_REGION( 0xc00000, "gfx", 0 )
	ROMX_LOAD( "dad.13m",   0x000000, 0x200000, CRC(da3cb7d6) SHA1(d59bb53d5f32889eb6eb7f8b1c8781948c97283d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.15m",   0x000002, 0x200000, CRC(92b63172) SHA1(9bed7dbbb17729f2ad3d318396f5335c0bd39937) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.17m",   0x000004, 0x200000, CRC(b98757f5) SHA1(3eead22e097906bf0e1e151cd0a9c75abc5a32d4) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.19m",   0x000006, 0x200000, CRC(8121ce46) SHA1(40c4dc969318d38f0c6d5401c9c64371f51aa12c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.14m",   0x800000, 0x100000, CRC(837e6f3f) SHA1(c060183474fba0e82d765b9f282b84838550dff6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.16m",   0x800002, 0x100000, CRC(f0916bdb) SHA1(9354d258dd26cbbf12c78ecfc277c357cbdb360e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.18m",   0x800004, 0x100000, CRC(cef393ef) SHA1(830b33c86cc24776d17ad65fa89a3b16c40446a1) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.20m",   0x800006, 0x100000, CRC(8953fe9e) SHA1(f4795beb006335d13e3934aa9760e775eb0bb950) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "dad.01",   0x00000, 0x08000, CRC(3f5e2424) SHA1(4aa744576bc6752c43a90a27a816ebd90076b248) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "dad.11m",   0x000000, 0x200000, CRC(0c499b67) SHA1(a8ebd8a1cd6dece8344b7cb0439d85843fb97616) )
	ROM_LOAD16_WORD_SWAP( "dad.12m",   0x200000, 0x200000, CRC(2f0b5a4e) SHA1(8d1ebbb811aa469b0f0d29d719d2b9af28fb63a2) )
ROM_END

ROM_START( ddtodh )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "dadh.03c", 0x000000, 0x80000, CRC(5750a861) SHA1(3b94999779f1b5f7cdaf78468256e2b60ba8c076) )
	ROM_LOAD16_WORD_SWAP( "dadh.04c", 0x080000, 0x80000, CRC(cfbf1b56) SHA1(d8919397f1d6dacddc6811889b69b65cd91be683) )
	ROM_LOAD16_WORD_SWAP( "dadh.05c", 0x100000, 0x80000, CRC(a6e562ba) SHA1(42998024a3bb4464843411ebe7283c6f5369694d) )
	ROM_LOAD16_WORD_SWAP( "dad.06a",  0x180000, 0x80000, CRC(6225495a) SHA1(a9a02abb072e3482ac92d7aed8ce9a5bcf636bc0) )
	ROM_LOAD16_WORD_SWAP( "dad.07a",  0x200000, 0x80000, CRC(b3480ec3) SHA1(a66f8dba67101fd71c2af4f3c3d71e55778a9f2c) )

	ROM_REGION( 0xc00000, "gfx", 0 )
	ROMX_LOAD( "dad.13m",   0x000000, 0x200000, CRC(da3cb7d6) SHA1(d59bb53d5f32889eb6eb7f8b1c8781948c97283d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.15m",   0x000002, 0x200000, CRC(92b63172) SHA1(9bed7dbbb17729f2ad3d318396f5335c0bd39937) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.17m",   0x000004, 0x200000, CRC(b98757f5) SHA1(3eead22e097906bf0e1e151cd0a9c75abc5a32d4) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.19m",   0x000006, 0x200000, CRC(8121ce46) SHA1(40c4dc969318d38f0c6d5401c9c64371f51aa12c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.14m",   0x800000, 0x100000, CRC(837e6f3f) SHA1(c060183474fba0e82d765b9f282b84838550dff6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.16m",   0x800002, 0x100000, CRC(f0916bdb) SHA1(9354d258dd26cbbf12c78ecfc277c357cbdb360e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.18m",   0x800004, 0x100000, CRC(cef393ef) SHA1(830b33c86cc24776d17ad65fa89a3b16c40446a1) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.20m",   0x800006, 0x100000, CRC(8953fe9e) SHA1(f4795beb006335d13e3934aa9760e775eb0bb950) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "dad.01",   0x00000, 0x08000, CRC(3f5e2424) SHA1(4aa744576bc6752c43a90a27a816ebd90076b248) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "dad.11m",   0x000000, 0x200000, CRC(0c499b67) SHA1(a8ebd8a1cd6dece8344b7cb0439d85843fb97616) )
	ROM_LOAD16_WORD_SWAP( "dad.12m",   0x200000, 0x200000, CRC(2f0b5a4e) SHA1(8d1ebbb811aa469b0f0d29d719d2b9af28fb63a2) )
ROM_END

ROM_START( ddtodhr1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "dadh.03b", 0x000000, 0x80000, CRC(ae0cb98e) SHA1(e85fb56d8f55fd1626a47301953b66597814e516) )
	ROM_LOAD16_WORD_SWAP( "dadh.04b", 0x080000, 0x80000, CRC(b5774363) SHA1(c91a6b257de4355a29d0a9742909592e69d287fb) )
	ROM_LOAD16_WORD_SWAP( "dadh.05b", 0x100000, 0x80000, CRC(6ce2a485) SHA1(7397105bbf88f6f2aa46614395df38b205e6461c) )
	ROM_LOAD16_WORD_SWAP( "dad.06",   0x180000, 0x80000, CRC(13aa3e56) SHA1(ccd3cda528d625bbf4dc0e8c5ad629af6080d705) )
	ROM_LOAD16_WORD_SWAP( "dad.07",   0x200000, 0x80000, CRC(431cb6dd) SHA1(ad3342e2fb8f0b3d7f57e845d5b80a871923324d) )

	ROM_REGION( 0xc00000, "gfx", 0 )
	ROMX_LOAD( "dad.13m",   0x000000, 0x200000, CRC(da3cb7d6) SHA1(d59bb53d5f32889eb6eb7f8b1c8781948c97283d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.15m",   0x000002, 0x200000, CRC(92b63172) SHA1(9bed7dbbb17729f2ad3d318396f5335c0bd39937) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.17m",   0x000004, 0x200000, CRC(b98757f5) SHA1(3eead22e097906bf0e1e151cd0a9c75abc5a32d4) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.19m",   0x000006, 0x200000, CRC(8121ce46) SHA1(40c4dc969318d38f0c6d5401c9c64371f51aa12c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.14m",   0x800000, 0x100000, CRC(837e6f3f) SHA1(c060183474fba0e82d765b9f282b84838550dff6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.16m",   0x800002, 0x100000, CRC(f0916bdb) SHA1(9354d258dd26cbbf12c78ecfc277c357cbdb360e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.18m",   0x800004, 0x100000, CRC(cef393ef) SHA1(830b33c86cc24776d17ad65fa89a3b16c40446a1) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.20m",   0x800006, 0x100000, CRC(8953fe9e) SHA1(f4795beb006335d13e3934aa9760e775eb0bb950) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "dad.01",   0x00000, 0x08000, CRC(3f5e2424) SHA1(4aa744576bc6752c43a90a27a816ebd90076b248) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "dad.11m",   0x000000, 0x200000, CRC(0c499b67) SHA1(a8ebd8a1cd6dece8344b7cb0439d85843fb97616) )
	ROM_LOAD16_WORD_SWAP( "dad.12m",   0x200000, 0x200000, CRC(2f0b5a4e) SHA1(8d1ebbb811aa469b0f0d29d719d2b9af28fb63a2) )
ROM_END

ROM_START( ddtodhr2 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "dadh.03a", 0x000000, 0x80000, CRC(43d04aa3) SHA1(550fcc8ebf48d704223347abee759d0ed903432b) )
	ROM_LOAD16_WORD_SWAP( "dadh.04a", 0x080000, 0x80000, CRC(8b8d296c) SHA1(4cd6612855317ba13dd4c6b0dd024243677b5fbe) )
	ROM_LOAD16_WORD_SWAP( "dadh.05a", 0x100000, 0x80000, CRC(daae6b14) SHA1(ee132b19f8d8c17da6fd2d2da24205f2404a62d1) )
	ROM_LOAD16_WORD_SWAP( "dad.06",   0x180000, 0x80000, CRC(13aa3e56) SHA1(ccd3cda528d625bbf4dc0e8c5ad629af6080d705) )
	ROM_LOAD16_WORD_SWAP( "dad.07",   0x200000, 0x80000, CRC(431cb6dd) SHA1(ad3342e2fb8f0b3d7f57e845d5b80a871923324d) )

	ROM_REGION( 0xc00000, "gfx", 0 )
	ROMX_LOAD( "dad.13m",   0x000000, 0x200000, CRC(da3cb7d6) SHA1(d59bb53d5f32889eb6eb7f8b1c8781948c97283d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.15m",   0x000002, 0x200000, CRC(92b63172) SHA1(9bed7dbbb17729f2ad3d318396f5335c0bd39937) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.17m",   0x000004, 0x200000, CRC(b98757f5) SHA1(3eead22e097906bf0e1e151cd0a9c75abc5a32d4) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.19m",   0x000006, 0x200000, CRC(8121ce46) SHA1(40c4dc969318d38f0c6d5401c9c64371f51aa12c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.14m",   0x800000, 0x100000, CRC(837e6f3f) SHA1(c060183474fba0e82d765b9f282b84838550dff6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.16m",   0x800002, 0x100000, CRC(f0916bdb) SHA1(9354d258dd26cbbf12c78ecfc277c357cbdb360e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.18m",   0x800004, 0x100000, CRC(cef393ef) SHA1(830b33c86cc24776d17ad65fa89a3b16c40446a1) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.20m",   0x800006, 0x100000, CRC(8953fe9e) SHA1(f4795beb006335d13e3934aa9760e775eb0bb950) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "dad.01",   0x00000, 0x08000, CRC(3f5e2424) SHA1(4aa744576bc6752c43a90a27a816ebd90076b248) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "dad.11m",   0x000000, 0x200000, CRC(0c499b67) SHA1(a8ebd8a1cd6dece8344b7cb0439d85843fb97616) )
	ROM_LOAD16_WORD_SWAP( "dad.12m",   0x200000, 0x200000, CRC(2f0b5a4e) SHA1(8d1ebbb811aa469b0f0d29d719d2b9af28fb63a2) )
ROM_END

ROM_START( ddsom )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "dd2e.03e", 0x000000, 0x80000, CRC(449361af) SHA1(14af2b35e6f43f92c9e071f1dc85b18cf73ecb35) )
	ROM_LOAD16_WORD_SWAP( "dd2e.04e", 0x080000, 0x80000, CRC(5b7052b6) SHA1(8a5f069f450da939d0f02518751cd9815d621d81) )
	ROM_LOAD16_WORD_SWAP( "dd2e.05e", 0x100000, 0x80000, CRC(788d5f60) SHA1(b8b42c11530a34c2878fb119c0a388e33067b66d) )
	ROM_LOAD16_WORD_SWAP( "dd2e.06e", 0x180000, 0x80000, CRC(e0807e1e) SHA1(4b978f5f647fff84d456eb14c9fd202d9a276997) )
	ROM_LOAD16_WORD_SWAP( "dd2e.07",  0x200000, 0x80000, CRC(bb777a02) SHA1(4b2c65a9129fc2262b35be1c10d06f60f5108981) )
	ROM_LOAD16_WORD_SWAP( "dd2e.08",  0x280000, 0x80000, CRC(30970890) SHA1(fd366a9323230f6997006ab4cc216f9a97865ebe) )
	ROM_LOAD16_WORD_SWAP( "dd2e.09",  0x300000, 0x80000, CRC(99e2194d) SHA1(cbcecdf5beeac3eac6c2c3fa395710e1b8347531) )
	ROM_LOAD16_WORD_SWAP( "dd2e.10",  0x380000, 0x80000, CRC(e198805e) SHA1(37ae9d88d98c59337b657cfa6feb56e4f9cae95f) )

	ROM_REGION( 0x1800000, "gfx", 0 )
	ROMX_LOAD( "dd2.13m",   0x0000000, 0x400000, CRC(a46b4e6e) SHA1(fb90f42868c581c481b4ceff9f692753fb186b30) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.15m",   0x0000002, 0x400000, CRC(d5fc50fc) SHA1(bc692f17b18bb47a724cd5152377cd5ccd6e184a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.17m",   0x0000004, 0x400000, CRC(837c0867) SHA1(3d6db290a8f76299a23543f0ccf6a7905e1088ac) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.19m",   0x0000006, 0x400000, CRC(bb0ec21c) SHA1(e43ccc1cf63ccd2b504cc9fd701af849a7321914) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.14m",   0x1000000, 0x200000, CRC(6d824ce2) SHA1(0ccfe6c8a944937718e28a1a373b5822c7b7001b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.16m",   0x1000002, 0x200000, CRC(79682ae5) SHA1(ee84f4791c29ce9e2bae06ba3ec47ff4d2cd7054) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.18m",   0x1000004, 0x200000, CRC(acddd149) SHA1(7f50de9b2d1cc733594c642be1804190519caffa) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.20m",   0x1000006, 0x200000, CRC(117fb0c0) SHA1(15c01fa1a71b6469b0e1bde0ce5835c5ff9d938c) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "dd2.01",   0x00000, 0x08000, CRC(99d657e5) SHA1(1528dd6b07a0e79951a35c0457c8a9c9770e9c78) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "dd2.02",   0x28000, 0x20000, CRC(117a3824) SHA1(14f3a12170b601c5466c93af9d2f24e0b386b4e4) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "dd2.11m",   0x000000, 0x200000, CRC(98d0c325) SHA1(7406e8d943d77c468eb418c4113261f4ab973bbf) )
	ROM_LOAD16_WORD_SWAP( "dd2.12m",   0x200000, 0x200000, CRC(5ea2e7fa) SHA1(0e6a9fd007f637adcb2226c902394f07de45e803) )
ROM_END

ROM_START( ddsomr1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "dd2e.03d", 0x000000, 0x80000, CRC(6c084ab5) SHA1(edfb4094086836c9ba47fed149b5756cf40d8bc1) )
	ROM_LOAD16_WORD_SWAP( "dd2e.04d", 0x080000, 0x80000, CRC(9b94a947) SHA1(6b170d9d07c5fe98a186c2eb7d65639cbbe828fc) )
	ROM_LOAD16_WORD_SWAP( "dd2e.05d", 0x100000, 0x80000, CRC(5d6a63c6) SHA1(2f1cefe2531688c305b3ddcc4c8adae39dcadb33) )
	ROM_LOAD16_WORD_SWAP( "dd2e.06d", 0x180000, 0x80000, CRC(31bde8ee) SHA1(ec409a38ce9b5d464e19a1c93caab2f2de191fa1) )
	ROM_LOAD16_WORD_SWAP( "dd2e.07",  0x200000, 0x80000, CRC(bb777a02) SHA1(4b2c65a9129fc2262b35be1c10d06f60f5108981) )
	ROM_LOAD16_WORD_SWAP( "dd2e.08",  0x280000, 0x80000, CRC(30970890) SHA1(fd366a9323230f6997006ab4cc216f9a97865ebe) )
	ROM_LOAD16_WORD_SWAP( "dd2e.09",  0x300000, 0x80000, CRC(99e2194d) SHA1(cbcecdf5beeac3eac6c2c3fa395710e1b8347531) )
	ROM_LOAD16_WORD_SWAP( "dd2e.10",  0x380000, 0x80000, CRC(e198805e) SHA1(37ae9d88d98c59337b657cfa6feb56e4f9cae95f) )

	ROM_REGION( 0x1800000, "gfx", 0 )
	ROMX_LOAD( "dd2.13m",   0x0000000, 0x400000, CRC(a46b4e6e) SHA1(fb90f42868c581c481b4ceff9f692753fb186b30) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.15m",   0x0000002, 0x400000, CRC(d5fc50fc) SHA1(bc692f17b18bb47a724cd5152377cd5ccd6e184a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.17m",   0x0000004, 0x400000, CRC(837c0867) SHA1(3d6db290a8f76299a23543f0ccf6a7905e1088ac) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.19m",   0x0000006, 0x400000, CRC(bb0ec21c) SHA1(e43ccc1cf63ccd2b504cc9fd701af849a7321914) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.14m",   0x1000000, 0x200000, CRC(6d824ce2) SHA1(0ccfe6c8a944937718e28a1a373b5822c7b7001b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.16m",   0x1000002, 0x200000, CRC(79682ae5) SHA1(ee84f4791c29ce9e2bae06ba3ec47ff4d2cd7054) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.18m",   0x1000004, 0x200000, CRC(acddd149) SHA1(7f50de9b2d1cc733594c642be1804190519caffa) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.20m",   0x1000006, 0x200000, CRC(117fb0c0) SHA1(15c01fa1a71b6469b0e1bde0ce5835c5ff9d938c) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "dd2.01",   0x00000, 0x08000, CRC(99d657e5) SHA1(1528dd6b07a0e79951a35c0457c8a9c9770e9c78) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "dd2.02",   0x28000, 0x20000, CRC(117a3824) SHA1(14f3a12170b601c5466c93af9d2f24e0b386b4e4) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "dd2.11m",   0x000000, 0x200000, CRC(98d0c325) SHA1(7406e8d943d77c468eb418c4113261f4ab973bbf) )
	ROM_LOAD16_WORD_SWAP( "dd2.12m",   0x200000, 0x200000, CRC(5ea2e7fa) SHA1(0e6a9fd007f637adcb2226c902394f07de45e803) )
ROM_END

ROM_START( ddsomr2 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "dd2e.03b", 0x000000, 0x80000, CRC(cd2deb66) SHA1(8a3fa5aca364f11bea76f69504e82416efc0ec11) )
	ROM_LOAD16_WORD_SWAP( "dd2e.04b", 0x080000, 0x80000, CRC(bfee43cc) SHA1(16cb34103bede42599ff3083a70ff918fdce9929) )
	ROM_LOAD16_WORD_SWAP( "dd2e.05b", 0x100000, 0x80000, CRC(049ab19d) SHA1(dfd2ed64c409389fed9b1d96955cbe0cf2abd2b7) )
	ROM_LOAD16_WORD_SWAP( "dd2e.06b", 0x180000, 0x80000, CRC(3994fb8b) SHA1(9b864f6cbd9b12d9409fcc2739e12f9a0775f205) )
	ROM_LOAD16_WORD_SWAP( "dd2e.07",  0x200000, 0x80000, CRC(bb777a02) SHA1(4b2c65a9129fc2262b35be1c10d06f60f5108981) )
	ROM_LOAD16_WORD_SWAP( "dd2e.08",  0x280000, 0x80000, CRC(30970890) SHA1(fd366a9323230f6997006ab4cc216f9a97865ebe) )
	ROM_LOAD16_WORD_SWAP( "dd2e.09",  0x300000, 0x80000, CRC(99e2194d) SHA1(cbcecdf5beeac3eac6c2c3fa395710e1b8347531) )
	ROM_LOAD16_WORD_SWAP( "dd2e.10",  0x380000, 0x80000, CRC(e198805e) SHA1(37ae9d88d98c59337b657cfa6feb56e4f9cae95f) )

	ROM_REGION( 0x1800000, "gfx", 0 )
	ROMX_LOAD( "dd2.13m",   0x0000000, 0x400000, CRC(a46b4e6e) SHA1(fb90f42868c581c481b4ceff9f692753fb186b30) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.15m",   0x0000002, 0x400000, CRC(d5fc50fc) SHA1(bc692f17b18bb47a724cd5152377cd5ccd6e184a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.17m",   0x0000004, 0x400000, CRC(837c0867) SHA1(3d6db290a8f76299a23543f0ccf6a7905e1088ac) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.19m",   0x0000006, 0x400000, CRC(bb0ec21c) SHA1(e43ccc1cf63ccd2b504cc9fd701af849a7321914) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.14m",   0x1000000, 0x200000, CRC(6d824ce2) SHA1(0ccfe6c8a944937718e28a1a373b5822c7b7001b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.16m",   0x1000002, 0x200000, CRC(79682ae5) SHA1(ee84f4791c29ce9e2bae06ba3ec47ff4d2cd7054) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.18m",   0x1000004, 0x200000, CRC(acddd149) SHA1(7f50de9b2d1cc733594c642be1804190519caffa) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.20m",   0x1000006, 0x200000, CRC(117fb0c0) SHA1(15c01fa1a71b6469b0e1bde0ce5835c5ff9d938c) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "dd2.01",   0x00000, 0x08000, CRC(99d657e5) SHA1(1528dd6b07a0e79951a35c0457c8a9c9770e9c78) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "dd2.02",   0x28000, 0x20000, CRC(117a3824) SHA1(14f3a12170b601c5466c93af9d2f24e0b386b4e4) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "dd2.11m",   0x000000, 0x200000, CRC(98d0c325) SHA1(7406e8d943d77c468eb418c4113261f4ab973bbf) )
	ROM_LOAD16_WORD_SWAP( "dd2.12m",   0x200000, 0x200000, CRC(5ea2e7fa) SHA1(0e6a9fd007f637adcb2226c902394f07de45e803) )
ROM_END

ROM_START( ddsomr3 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "dd2e.03a", 0x000000, 0x80000, CRC(6de67678) SHA1(df4846b963ea0449fbd16152600a9628d20b278a) )
	ROM_LOAD16_WORD_SWAP( "dd2e.04a", 0x080000, 0x80000, CRC(0e45739a) SHA1(4fa9dc8109fad30e4037047ddec1b367b5b7600f) )
	ROM_LOAD16_WORD_SWAP( "dd2e.05a", 0x100000, 0x80000, CRC(3dce8025) SHA1(5fb0a58b7cccc889507b0085b5e74d7aef507f08) )
	ROM_LOAD16_WORD_SWAP( "dd2e.06a", 0x180000, 0x80000, CRC(51bafbef) SHA1(08edeca2339e4b48e78ff9b71f576379ae03f5f4) )
	ROM_LOAD16_WORD_SWAP( "dd2e.07",  0x200000, 0x80000, CRC(bb777a02) SHA1(4b2c65a9129fc2262b35be1c10d06f60f5108981) )
	ROM_LOAD16_WORD_SWAP( "dd2e.08",  0x280000, 0x80000, CRC(30970890) SHA1(fd366a9323230f6997006ab4cc216f9a97865ebe) )
	ROM_LOAD16_WORD_SWAP( "dd2e.09",  0x300000, 0x80000, CRC(99e2194d) SHA1(cbcecdf5beeac3eac6c2c3fa395710e1b8347531) )
	ROM_LOAD16_WORD_SWAP( "dd2e.10",  0x380000, 0x80000, CRC(e198805e) SHA1(37ae9d88d98c59337b657cfa6feb56e4f9cae95f) )

	ROM_REGION( 0x1800000, "gfx", 0 )
	ROMX_LOAD( "dd2.13m",   0x0000000, 0x400000, CRC(a46b4e6e) SHA1(fb90f42868c581c481b4ceff9f692753fb186b30) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.15m",   0x0000002, 0x400000, CRC(d5fc50fc) SHA1(bc692f17b18bb47a724cd5152377cd5ccd6e184a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.17m",   0x0000004, 0x400000, CRC(837c0867) SHA1(3d6db290a8f76299a23543f0ccf6a7905e1088ac) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.19m",   0x0000006, 0x400000, CRC(bb0ec21c) SHA1(e43ccc1cf63ccd2b504cc9fd701af849a7321914) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.14m",   0x1000000, 0x200000, CRC(6d824ce2) SHA1(0ccfe6c8a944937718e28a1a373b5822c7b7001b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.16m",   0x1000002, 0x200000, CRC(79682ae5) SHA1(ee84f4791c29ce9e2bae06ba3ec47ff4d2cd7054) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.18m",   0x1000004, 0x200000, CRC(acddd149) SHA1(7f50de9b2d1cc733594c642be1804190519caffa) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.20m",   0x1000006, 0x200000, CRC(117fb0c0) SHA1(15c01fa1a71b6469b0e1bde0ce5835c5ff9d938c) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "dd2.01",   0x00000, 0x08000, CRC(99d657e5) SHA1(1528dd6b07a0e79951a35c0457c8a9c9770e9c78) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "dd2.02",   0x28000, 0x20000, CRC(117a3824) SHA1(14f3a12170b601c5466c93af9d2f24e0b386b4e4) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "dd2.11m",   0x000000, 0x200000, CRC(98d0c325) SHA1(7406e8d943d77c468eb418c4113261f4ab973bbf) )
	ROM_LOAD16_WORD_SWAP( "dd2.12m",   0x200000, 0x200000, CRC(5ea2e7fa) SHA1(0e6a9fd007f637adcb2226c902394f07de45e803) )
ROM_END

ROM_START( ddsomu )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "dd2u.03g", 0x000000, 0x80000, CRC(fb089b39) SHA1(2d00ad87d5e862745d730a84a8b9b9a8c9423282) )
	ROM_LOAD16_WORD_SWAP( "dd2u.04g", 0x080000, 0x80000, CRC(cd432b73) SHA1(7c5ddad66f9f08fef79efb01ccf230a9eae366c6) )
	ROM_LOAD16_WORD_SWAP( "dd2.05g",  0x100000, 0x80000, CRC(5eb1991c) SHA1(429a60b5396ff4192904867fbe0524268f0edbcb) )
	ROM_LOAD16_WORD_SWAP( "dd2.06g",  0x180000, 0x80000, CRC(c26b5e55) SHA1(9590206f30459941880ff4b56c7f276cc78e3a22) )
	ROM_LOAD16_WORD_SWAP( "dd2.07",   0x200000, 0x80000, CRC(909a0b8b) SHA1(58bda17c36063a79df8b5031755c7909a9bda221) )
	ROM_LOAD16_WORD_SWAP( "dd2.08",   0x280000, 0x80000, CRC(e53c4d01) SHA1(bad872e4e793a39f68bc0e580772e982714b5876) )
	ROM_LOAD16_WORD_SWAP( "dd2.09",   0x300000, 0x80000, CRC(5f86279f) SHA1(c2a454e5f821b1cdd49f2cf0602e9bfb7ba63340) )
	ROM_LOAD16_WORD_SWAP( "dd2.10",   0x380000, 0x80000, CRC(ad954c26) SHA1(468c01735dbdb1114b37060546a660678290a97f) )

	ROM_REGION( 0x1800000, "gfx", 0 )
	ROMX_LOAD( "dd2.13m",   0x0000000, 0x400000, CRC(a46b4e6e) SHA1(fb90f42868c581c481b4ceff9f692753fb186b30) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.15m",   0x0000002, 0x400000, CRC(d5fc50fc) SHA1(bc692f17b18bb47a724cd5152377cd5ccd6e184a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.17m",   0x0000004, 0x400000, CRC(837c0867) SHA1(3d6db290a8f76299a23543f0ccf6a7905e1088ac) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.19m",   0x0000006, 0x400000, CRC(bb0ec21c) SHA1(e43ccc1cf63ccd2b504cc9fd701af849a7321914) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.14m",   0x1000000, 0x200000, CRC(6d824ce2) SHA1(0ccfe6c8a944937718e28a1a373b5822c7b7001b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.16m",   0x1000002, 0x200000, CRC(79682ae5) SHA1(ee84f4791c29ce9e2bae06ba3ec47ff4d2cd7054) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.18m",   0x1000004, 0x200000, CRC(acddd149) SHA1(7f50de9b2d1cc733594c642be1804190519caffa) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.20m",   0x1000006, 0x200000, CRC(117fb0c0) SHA1(15c01fa1a71b6469b0e1bde0ce5835c5ff9d938c) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "dd2.01",   0x00000, 0x08000, CRC(99d657e5) SHA1(1528dd6b07a0e79951a35c0457c8a9c9770e9c78) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "dd2.02",   0x28000, 0x20000, CRC(117a3824) SHA1(14f3a12170b601c5466c93af9d2f24e0b386b4e4) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "dd2.11m",   0x000000, 0x200000, CRC(98d0c325) SHA1(7406e8d943d77c468eb418c4113261f4ab973bbf) )
	ROM_LOAD16_WORD_SWAP( "dd2.12m",   0x200000, 0x200000, CRC(5ea2e7fa) SHA1(0e6a9fd007f637adcb2226c902394f07de45e803) )
ROM_END

ROM_START( ddsomur1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "dd2u.03d", 0x000000, 0x80000, CRC(0f700d84) SHA1(f4788d4046e0f6aba146c18a930196f5f9f8f14a) )
	ROM_LOAD16_WORD_SWAP( "dd2u.04d", 0x080000, 0x80000, CRC(b99eb254) SHA1(507ad31b0d77dfbaaaf0fa5830c4ef14845a80de) )
	ROM_LOAD16_WORD_SWAP( "dd2.05d",  0x100000, 0x80000, CRC(b23061f3) SHA1(471a1238770a5109f34a0b450b214a5490cc6ecb) )
	ROM_LOAD16_WORD_SWAP( "dd2.06d",  0x180000, 0x80000, CRC(8bf1d8ce) SHA1(384dda9dfa2a851d30432f29bba456e138a5ca28) )
	ROM_LOAD16_WORD_SWAP( "dd2.07",   0x200000, 0x80000, CRC(909a0b8b) SHA1(58bda17c36063a79df8b5031755c7909a9bda221) )
	ROM_LOAD16_WORD_SWAP( "dd2.08",   0x280000, 0x80000, CRC(e53c4d01) SHA1(bad872e4e793a39f68bc0e580772e982714b5876) )
	ROM_LOAD16_WORD_SWAP( "dd2.09",   0x300000, 0x80000, CRC(5f86279f) SHA1(c2a454e5f821b1cdd49f2cf0602e9bfb7ba63340) )
	ROM_LOAD16_WORD_SWAP( "dd2.10",   0x380000, 0x80000, CRC(ad954c26) SHA1(468c01735dbdb1114b37060546a660678290a97f) )

	ROM_REGION( 0x1800000, "gfx", 0 )
	ROMX_LOAD( "dd2.13m",   0x0000000, 0x400000, CRC(a46b4e6e) SHA1(fb90f42868c581c481b4ceff9f692753fb186b30) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.15m",   0x0000002, 0x400000, CRC(d5fc50fc) SHA1(bc692f17b18bb47a724cd5152377cd5ccd6e184a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.17m",   0x0000004, 0x400000, CRC(837c0867) SHA1(3d6db290a8f76299a23543f0ccf6a7905e1088ac) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.19m",   0x0000006, 0x400000, CRC(bb0ec21c) SHA1(e43ccc1cf63ccd2b504cc9fd701af849a7321914) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.14m",   0x1000000, 0x200000, CRC(6d824ce2) SHA1(0ccfe6c8a944937718e28a1a373b5822c7b7001b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.16m",   0x1000002, 0x200000, CRC(79682ae5) SHA1(ee84f4791c29ce9e2bae06ba3ec47ff4d2cd7054) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.18m",   0x1000004, 0x200000, CRC(acddd149) SHA1(7f50de9b2d1cc733594c642be1804190519caffa) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.20m",   0x1000006, 0x200000, CRC(117fb0c0) SHA1(15c01fa1a71b6469b0e1bde0ce5835c5ff9d938c) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "dd2.01",   0x00000, 0x08000, CRC(99d657e5) SHA1(1528dd6b07a0e79951a35c0457c8a9c9770e9c78) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "dd2.02",   0x28000, 0x20000, CRC(117a3824) SHA1(14f3a12170b601c5466c93af9d2f24e0b386b4e4) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "dd2.11m",   0x000000, 0x200000, CRC(98d0c325) SHA1(7406e8d943d77c468eb418c4113261f4ab973bbf) )
	ROM_LOAD16_WORD_SWAP( "dd2.12m",   0x200000, 0x200000, CRC(5ea2e7fa) SHA1(0e6a9fd007f637adcb2226c902394f07de45e803) )
ROM_END

ROM_START( ddsomj )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "dd2j.03g", 0x000000, 0x80000, CRC(e6c8c985) SHA1(0736a84d7d9d37d51826eac6826a7728260bc625) )
	ROM_LOAD16_WORD_SWAP( "dd2j.04g", 0x080000, 0x80000, CRC(8386c0bd) SHA1(59bfc71914ec2bb7d1b9f327b25d2399181d4bb2) )
	ROM_LOAD16_WORD_SWAP( "dd2.05g",  0x100000, 0x80000, CRC(5eb1991c) SHA1(429a60b5396ff4192904867fbe0524268f0edbcb) )
	ROM_LOAD16_WORD_SWAP( "dd2.06g",  0x180000, 0x80000, CRC(c26b5e55) SHA1(9590206f30459941880ff4b56c7f276cc78e3a22) )
	ROM_LOAD16_WORD_SWAP( "dd2.07",   0x200000, 0x80000, CRC(909a0b8b) SHA1(58bda17c36063a79df8b5031755c7909a9bda221) )
	ROM_LOAD16_WORD_SWAP( "dd2.08",   0x280000, 0x80000, CRC(e53c4d01) SHA1(bad872e4e793a39f68bc0e580772e982714b5876) )
	ROM_LOAD16_WORD_SWAP( "dd2.09",   0x300000, 0x80000, CRC(5f86279f) SHA1(c2a454e5f821b1cdd49f2cf0602e9bfb7ba63340) )
	ROM_LOAD16_WORD_SWAP( "dd2.10",   0x380000, 0x80000, CRC(ad954c26) SHA1(468c01735dbdb1114b37060546a660678290a97f) )

	ROM_REGION( 0x1800000, "gfx", 0 )
	ROMX_LOAD( "dd2.13m",   0x0000000, 0x400000, CRC(a46b4e6e) SHA1(fb90f42868c581c481b4ceff9f692753fb186b30) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.15m",   0x0000002, 0x400000, CRC(d5fc50fc) SHA1(bc692f17b18bb47a724cd5152377cd5ccd6e184a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.17m",   0x0000004, 0x400000, CRC(837c0867) SHA1(3d6db290a8f76299a23543f0ccf6a7905e1088ac) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.19m",   0x0000006, 0x400000, CRC(bb0ec21c) SHA1(e43ccc1cf63ccd2b504cc9fd701af849a7321914) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.14m",   0x1000000, 0x200000, CRC(6d824ce2) SHA1(0ccfe6c8a944937718e28a1a373b5822c7b7001b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.16m",   0x1000002, 0x200000, CRC(79682ae5) SHA1(ee84f4791c29ce9e2bae06ba3ec47ff4d2cd7054) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.18m",   0x1000004, 0x200000, CRC(acddd149) SHA1(7f50de9b2d1cc733594c642be1804190519caffa) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.20m",   0x1000006, 0x200000, CRC(117fb0c0) SHA1(15c01fa1a71b6469b0e1bde0ce5835c5ff9d938c) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "dd2.01",   0x00000, 0x08000, CRC(99d657e5) SHA1(1528dd6b07a0e79951a35c0457c8a9c9770e9c78) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "dd2.02",   0x28000, 0x20000, CRC(117a3824) SHA1(14f3a12170b601c5466c93af9d2f24e0b386b4e4) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "dd2.11m",   0x000000, 0x200000, CRC(98d0c325) SHA1(7406e8d943d77c468eb418c4113261f4ab973bbf) )
	ROM_LOAD16_WORD_SWAP( "dd2.12m",   0x200000, 0x200000, CRC(5ea2e7fa) SHA1(0e6a9fd007f637adcb2226c902394f07de45e803) )
ROM_END

ROM_START( ddsomjr1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "dd2j.03b", 0x000000, 0x80000, CRC(965d74e5) SHA1(d7aa1b78043cdf09ee71a6dd5fe78e0588ca7875) )
	ROM_LOAD16_WORD_SWAP( "dd2j.04b", 0x080000, 0x80000, CRC(958eb8f3) SHA1(3d9747bc9091b0b42c953a19992b94cb2bf69159) )
	ROM_LOAD16_WORD_SWAP( "dd2.05b",  0x100000, 0x80000, CRC(d38571ca) SHA1(f0105a4f201e11f489e44c8061b0025de2e32f93) )
	ROM_LOAD16_WORD_SWAP( "dd2.06b",  0x180000, 0x80000, CRC(6d5a3bbb) SHA1(549e31398e706a80d41db6600555e27e902c335c) )
	ROM_LOAD16_WORD_SWAP( "dd2.07",   0x200000, 0x80000, CRC(909a0b8b) SHA1(58bda17c36063a79df8b5031755c7909a9bda221) )
	ROM_LOAD16_WORD_SWAP( "dd2.08",   0x280000, 0x80000, CRC(e53c4d01) SHA1(bad872e4e793a39f68bc0e580772e982714b5876) )
	ROM_LOAD16_WORD_SWAP( "dd2.09",   0x300000, 0x80000, CRC(5f86279f) SHA1(c2a454e5f821b1cdd49f2cf0602e9bfb7ba63340) )
	ROM_LOAD16_WORD_SWAP( "dd2.10",   0x380000, 0x80000, CRC(ad954c26) SHA1(468c01735dbdb1114b37060546a660678290a97f) )

	ROM_REGION( 0x1800000, "gfx", 0 )
	ROMX_LOAD( "dd2.13m",   0x0000000, 0x400000, CRC(a46b4e6e) SHA1(fb90f42868c581c481b4ceff9f692753fb186b30) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.15m",   0x0000002, 0x400000, CRC(d5fc50fc) SHA1(bc692f17b18bb47a724cd5152377cd5ccd6e184a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.17m",   0x0000004, 0x400000, CRC(837c0867) SHA1(3d6db290a8f76299a23543f0ccf6a7905e1088ac) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.19m",   0x0000006, 0x400000, CRC(bb0ec21c) SHA1(e43ccc1cf63ccd2b504cc9fd701af849a7321914) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.14m",   0x1000000, 0x200000, CRC(6d824ce2) SHA1(0ccfe6c8a944937718e28a1a373b5822c7b7001b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.16m",   0x1000002, 0x200000, CRC(79682ae5) SHA1(ee84f4791c29ce9e2bae06ba3ec47ff4d2cd7054) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.18m",   0x1000004, 0x200000, CRC(acddd149) SHA1(7f50de9b2d1cc733594c642be1804190519caffa) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.20m",   0x1000006, 0x200000, CRC(117fb0c0) SHA1(15c01fa1a71b6469b0e1bde0ce5835c5ff9d938c) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "dd2.01",   0x00000, 0x08000, CRC(99d657e5) SHA1(1528dd6b07a0e79951a35c0457c8a9c9770e9c78) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "dd2.02",   0x28000, 0x20000, CRC(117a3824) SHA1(14f3a12170b601c5466c93af9d2f24e0b386b4e4) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "dd2.11m",   0x000000, 0x200000, CRC(98d0c325) SHA1(7406e8d943d77c468eb418c4113261f4ab973bbf) )
	ROM_LOAD16_WORD_SWAP( "dd2.12m",   0x200000, 0x200000, CRC(5ea2e7fa) SHA1(0e6a9fd007f637adcb2226c902394f07de45e803) )
ROM_END

ROM_START( ddsoma )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "dd2a.03g", 0x000000, 0x80000, CRC(0b4fec22) SHA1(4dd69637898e0bc64d1b1dc34561ce1807da314b) )
	ROM_LOAD16_WORD_SWAP( "dd2a.04g", 0x080000, 0x80000, CRC(055b7019) SHA1(5dab39552fee20bd6f94c992c1c3a995595fdf94) )
	ROM_LOAD16_WORD_SWAP( "dd2.05g",  0x100000, 0x80000, CRC(5eb1991c) SHA1(429a60b5396ff4192904867fbe0524268f0edbcb) )
	ROM_LOAD16_WORD_SWAP( "dd2.06g",  0x180000, 0x80000, CRC(c26b5e55) SHA1(9590206f30459941880ff4b56c7f276cc78e3a22) )
	ROM_LOAD16_WORD_SWAP( "dd2.07",   0x200000, 0x80000, CRC(909a0b8b) SHA1(58bda17c36063a79df8b5031755c7909a9bda221) )
	ROM_LOAD16_WORD_SWAP( "dd2.08",   0x280000, 0x80000, CRC(e53c4d01) SHA1(bad872e4e793a39f68bc0e580772e982714b5876) )
	ROM_LOAD16_WORD_SWAP( "dd2.09",   0x300000, 0x80000, CRC(5f86279f) SHA1(c2a454e5f821b1cdd49f2cf0602e9bfb7ba63340) )
	ROM_LOAD16_WORD_SWAP( "dd2.10",   0x380000, 0x80000, CRC(ad954c26) SHA1(468c01735dbdb1114b37060546a660678290a97f) )

	ROM_REGION( 0x1800000, "gfx", 0 )
	ROMX_LOAD( "dd2.13m",   0x0000000, 0x400000, CRC(a46b4e6e) SHA1(fb90f42868c581c481b4ceff9f692753fb186b30) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.15m",   0x0000002, 0x400000, CRC(d5fc50fc) SHA1(bc692f17b18bb47a724cd5152377cd5ccd6e184a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.17m",   0x0000004, 0x400000, CRC(837c0867) SHA1(3d6db290a8f76299a23543f0ccf6a7905e1088ac) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.19m",   0x0000006, 0x400000, CRC(bb0ec21c) SHA1(e43ccc1cf63ccd2b504cc9fd701af849a7321914) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.14m",   0x1000000, 0x200000, CRC(6d824ce2) SHA1(0ccfe6c8a944937718e28a1a373b5822c7b7001b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.16m",   0x1000002, 0x200000, CRC(79682ae5) SHA1(ee84f4791c29ce9e2bae06ba3ec47ff4d2cd7054) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.18m",   0x1000004, 0x200000, CRC(acddd149) SHA1(7f50de9b2d1cc733594c642be1804190519caffa) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.20m",   0x1000006, 0x200000, CRC(117fb0c0) SHA1(15c01fa1a71b6469b0e1bde0ce5835c5ff9d938c) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "dd2.01",   0x00000, 0x08000, CRC(99d657e5) SHA1(1528dd6b07a0e79951a35c0457c8a9c9770e9c78) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "dd2.02",   0x28000, 0x20000, CRC(117a3824) SHA1(14f3a12170b601c5466c93af9d2f24e0b386b4e4) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "dd2.11m",   0x000000, 0x200000, CRC(98d0c325) SHA1(7406e8d943d77c468eb418c4113261f4ab973bbf) )
	ROM_LOAD16_WORD_SWAP( "dd2.12m",   0x200000, 0x200000, CRC(5ea2e7fa) SHA1(0e6a9fd007f637adcb2226c902394f07de45e803) )
ROM_END

ROM_START( ddsomb )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "dd2b.03a", 0x000000, 0x80000, CRC(e8ce7fbb) SHA1(645133fb07b34f663709896a0f55a9a51de4ee9b) )
	ROM_LOAD16_WORD_SWAP( "dd2b.04a", 0x080000, 0x80000, CRC(6b679664) SHA1(480d8b225c69d528528b6a4db86797a9d9e6ac80) )
	ROM_LOAD16_WORD_SWAP( "dd2b.05a", 0x100000, 0x80000, CRC(9b2534eb) SHA1(04a9f9b75f817dff1b94641aba399d487b57a9f7) )
	ROM_LOAD16_WORD_SWAP( "dd2b.06a", 0x180000, 0x80000, CRC(3b21ba59) SHA1(0b9be23253c42047ebfe3e656670ebf5e792766f) )
	ROM_LOAD16_WORD_SWAP( "dd2b.07",  0x200000, 0x80000, CRC(fce2558d) SHA1(67041b550bcb357f1c76e3ab703c7db3cc071515) )
	ROM_LOAD16_WORD_SWAP( "dd2e.08",  0x280000, 0x80000, CRC(30970890) SHA1(fd366a9323230f6997006ab4cc216f9a97865ebe) )
	ROM_LOAD16_WORD_SWAP( "dd2e.09",  0x300000, 0x80000, CRC(99e2194d) SHA1(cbcecdf5beeac3eac6c2c3fa395710e1b8347531) )
	ROM_LOAD16_WORD_SWAP( "dd2e.10",  0x380000, 0x80000, CRC(e198805e) SHA1(37ae9d88d98c59337b657cfa6feb56e4f9cae95f) )

	ROM_REGION( 0x1800000, "gfx", 0 )
	ROMX_LOAD( "dd2.13m",   0x0000000, 0x400000, CRC(a46b4e6e) SHA1(fb90f42868c581c481b4ceff9f692753fb186b30) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.15m",   0x0000002, 0x400000, CRC(d5fc50fc) SHA1(bc692f17b18bb47a724cd5152377cd5ccd6e184a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.17m",   0x0000004, 0x400000, CRC(837c0867) SHA1(3d6db290a8f76299a23543f0ccf6a7905e1088ac) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.19m",   0x0000006, 0x400000, CRC(bb0ec21c) SHA1(e43ccc1cf63ccd2b504cc9fd701af849a7321914) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.14m",   0x1000000, 0x200000, CRC(6d824ce2) SHA1(0ccfe6c8a944937718e28a1a373b5822c7b7001b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.16m",   0x1000002, 0x200000, CRC(79682ae5) SHA1(ee84f4791c29ce9e2bae06ba3ec47ff4d2cd7054) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.18m",   0x1000004, 0x200000, CRC(acddd149) SHA1(7f50de9b2d1cc733594c642be1804190519caffa) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.20m",   0x1000006, 0x200000, CRC(117fb0c0) SHA1(15c01fa1a71b6469b0e1bde0ce5835c5ff9d938c) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "dd2.01",   0x00000, 0x08000, CRC(99d657e5) SHA1(1528dd6b07a0e79951a35c0457c8a9c9770e9c78) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "dd2.02",   0x28000, 0x20000, CRC(117a3824) SHA1(14f3a12170b601c5466c93af9d2f24e0b386b4e4) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "dd2.11m",   0x000000, 0x200000, CRC(98d0c325) SHA1(7406e8d943d77c468eb418c4113261f4ab973bbf) )
	ROM_LOAD16_WORD_SWAP( "dd2.12m",   0x200000, 0x200000, CRC(5ea2e7fa) SHA1(0e6a9fd007f637adcb2226c902394f07de45e803) )
ROM_END

ROM_START( ddsomh )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "dd2h.03a", 0x000000, 0x80000, CRC(e472c9f3) SHA1(43e0767cca3ce0c151f1bd12d476cc2f0013b5e5) )
	ROM_LOAD16_WORD_SWAP( "dd2h.04a", 0x080000, 0x80000, CRC(315a7706) SHA1(de6ced251a483455b0d0371c60bce0c209879b43) )
	ROM_LOAD16_WORD_SWAP( "dd2h.05a", 0x100000, 0x80000, CRC(9b2534eb) SHA1(04a9f9b75f817dff1b94641aba399d487b57a9f7) ) /* Roms 05 through 10 are the same as the Brazil set */
	ROM_LOAD16_WORD_SWAP( "dd2h.06a", 0x180000, 0x80000, CRC(3b21ba59) SHA1(0b9be23253c42047ebfe3e656670ebf5e792766f) )
	ROM_LOAD16_WORD_SWAP( "dd2h.07a", 0x200000, 0x80000, CRC(fce2558d) SHA1(67041b550bcb357f1c76e3ab703c7db3cc071515) )
	ROM_LOAD16_WORD_SWAP( "dd2e.08",  0x280000, 0x80000, CRC(30970890) SHA1(fd366a9323230f6997006ab4cc216f9a97865ebe) )
	ROM_LOAD16_WORD_SWAP( "dd2e.09",  0x300000, 0x80000, CRC(99e2194d) SHA1(cbcecdf5beeac3eac6c2c3fa395710e1b8347531) )
	ROM_LOAD16_WORD_SWAP( "dd2e.10",  0x380000, 0x80000, CRC(e198805e) SHA1(37ae9d88d98c59337b657cfa6feb56e4f9cae95f) )

	ROM_REGION( 0x1800000, "gfx", 0 )
	ROMX_LOAD( "dd2.13m",   0x0000000, 0x400000, CRC(a46b4e6e) SHA1(fb90f42868c581c481b4ceff9f692753fb186b30) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.15m",   0x0000002, 0x400000, CRC(d5fc50fc) SHA1(bc692f17b18bb47a724cd5152377cd5ccd6e184a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.17m",   0x0000004, 0x400000, CRC(837c0867) SHA1(3d6db290a8f76299a23543f0ccf6a7905e1088ac) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.19m",   0x0000006, 0x400000, CRC(bb0ec21c) SHA1(e43ccc1cf63ccd2b504cc9fd701af849a7321914) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.14m",   0x1000000, 0x200000, CRC(6d824ce2) SHA1(0ccfe6c8a944937718e28a1a373b5822c7b7001b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.16m",   0x1000002, 0x200000, CRC(79682ae5) SHA1(ee84f4791c29ce9e2bae06ba3ec47ff4d2cd7054) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.18m",   0x1000004, 0x200000, CRC(acddd149) SHA1(7f50de9b2d1cc733594c642be1804190519caffa) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.20m",   0x1000006, 0x200000, CRC(117fb0c0) SHA1(15c01fa1a71b6469b0e1bde0ce5835c5ff9d938c) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "dd2.01",   0x00000, 0x08000, CRC(99d657e5) SHA1(1528dd6b07a0e79951a35c0457c8a9c9770e9c78) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "dd2.02",   0x28000, 0x20000, CRC(117a3824) SHA1(14f3a12170b601c5466c93af9d2f24e0b386b4e4) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "dd2.11m",   0x000000, 0x200000, CRC(98d0c325) SHA1(7406e8d943d77c468eb418c4113261f4ab973bbf) )
	ROM_LOAD16_WORD_SWAP( "dd2.12m",   0x200000, 0x200000, CRC(5ea2e7fa) SHA1(0e6a9fd007f637adcb2226c902394f07de45e803) )
ROM_END

ROM_START( dimahoo )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "gmde.03", 0x000000, 0x80000, CRC(968fcecd) SHA1(82d6eb6488df48bc7b977fc900c24b29bd6573a9) )
	ROM_LOAD16_WORD_SWAP( "gmd.04",  0x080000, 0x80000, CRC(37485567) SHA1(643c41fce6057bcaef0e0bedc62914c33d97eeaf) )
	ROM_LOAD16_WORD_SWAP( "gmd.05",  0x100000, 0x80000, CRC(da269ffb) SHA1(e99b04192030b6006cf67b563f40cea29c1b2e78) )
	ROM_LOAD16_WORD_SWAP( "gmd.06",  0x180000, 0x80000, CRC(55b483c9) SHA1(d47e077312f3c044d3647b79fa9e0581ccff5992) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROMX_LOAD( "gmd.13m",   0x000000, 0x400000, CRC(80dd19f0) SHA1(0fd8b1e8d73cc83e6c34f0d94487938da2344f76) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "gmd.15m",   0x000002, 0x400000, CRC(dfd93a78) SHA1(c343d5ddcc25bd0739491e7439d7c0d0a8881a04) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "gmd.17m",   0x000004, 0x400000, CRC(16356520) SHA1(058713bef30c1b1d8b7dd0ceaaa57a3ab9751a70) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "gmd.19m",   0x000006, 0x400000, CRC(dfc33031) SHA1(a1ceaeddc2a79d5b79f1b107cac2ef6a5e621e77) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "gmd.01",   0x00000, 0x08000, CRC(3f9bc985) SHA1(1616bbee82877b1052a07531066f5009a80706be) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "gmd.02",   0x28000, 0x20000, CRC(3fd39dde) SHA1(6a6e3ef9baa430ee83ab2312aa0221bae4d73dbd) )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "gmd.11m",   0x000000, 0x400000, CRC(06a65542) SHA1(a1b3df70c90055a3cd59d0149fd18a74eff5bcc9) )
	ROM_LOAD16_WORD_SWAP( "gmd.12m",   0x400000, 0x400000, CRC(50bc7a31) SHA1(7283569fc646c39f4c693f14e0ce7ff2ee49111a) )
ROM_END

ROM_START( dimahoou )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "gmdu.03", 0x000000, 0x80000, CRC(43bcb15f) SHA1(8cf758f9b3b416273e5b20e5d1c09c0a67029a01) )
	ROM_LOAD16_WORD_SWAP( "gmd.04",  0x080000, 0x80000, CRC(37485567) SHA1(643c41fce6057bcaef0e0bedc62914c33d97eeaf) )
	ROM_LOAD16_WORD_SWAP( "gmd.05",  0x100000, 0x80000, CRC(da269ffb) SHA1(e99b04192030b6006cf67b563f40cea29c1b2e78) )
	ROM_LOAD16_WORD_SWAP( "gmd.06",  0x180000, 0x80000, CRC(55b483c9) SHA1(d47e077312f3c044d3647b79fa9e0581ccff5992) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROMX_LOAD( "gmd.13m",   0x000000, 0x400000, CRC(80dd19f0) SHA1(0fd8b1e8d73cc83e6c34f0d94487938da2344f76) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "gmd.15m",   0x000002, 0x400000, CRC(dfd93a78) SHA1(c343d5ddcc25bd0739491e7439d7c0d0a8881a04) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "gmd.17m",   0x000004, 0x400000, CRC(16356520) SHA1(058713bef30c1b1d8b7dd0ceaaa57a3ab9751a70) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "gmd.19m",   0x000006, 0x400000, CRC(dfc33031) SHA1(a1ceaeddc2a79d5b79f1b107cac2ef6a5e621e77) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "gmd.01",   0x00000, 0x08000, CRC(3f9bc985) SHA1(1616bbee82877b1052a07531066f5009a80706be) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "gmd.02",   0x28000, 0x20000, CRC(3fd39dde) SHA1(6a6e3ef9baa430ee83ab2312aa0221bae4d73dbd) )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "gmd.11m",   0x000000, 0x400000, CRC(06a65542) SHA1(a1b3df70c90055a3cd59d0149fd18a74eff5bcc9) )
	ROM_LOAD16_WORD_SWAP( "gmd.12m",   0x400000, 0x400000, CRC(50bc7a31) SHA1(7283569fc646c39f4c693f14e0ce7ff2ee49111a) )
ROM_END

ROM_START( gmahou )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "gmdj.03", 0x000000, 0x80000, CRC(cd6979e3) SHA1(b033408f49299eac376fc798c3429e5db97dd4fe) )
	ROM_LOAD16_WORD_SWAP( "gmd.04",  0x080000, 0x80000, CRC(37485567) SHA1(643c41fce6057bcaef0e0bedc62914c33d97eeaf) )
	ROM_LOAD16_WORD_SWAP( "gmd.05",  0x100000, 0x80000, CRC(da269ffb) SHA1(e99b04192030b6006cf67b563f40cea29c1b2e78) )
	ROM_LOAD16_WORD_SWAP( "gmd.06",  0x180000, 0x80000, CRC(55b483c9) SHA1(d47e077312f3c044d3647b79fa9e0581ccff5992) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROMX_LOAD( "gmd.13m",   0x000000, 0x400000, CRC(80dd19f0) SHA1(0fd8b1e8d73cc83e6c34f0d94487938da2344f76) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "gmd.15m",   0x000002, 0x400000, CRC(dfd93a78) SHA1(c343d5ddcc25bd0739491e7439d7c0d0a8881a04) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "gmd.17m",   0x000004, 0x400000, CRC(16356520) SHA1(058713bef30c1b1d8b7dd0ceaaa57a3ab9751a70) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "gmd.19m",   0x000006, 0x400000, CRC(dfc33031) SHA1(a1ceaeddc2a79d5b79f1b107cac2ef6a5e621e77) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "gmd.01",   0x00000, 0x08000, CRC(3f9bc985) SHA1(1616bbee82877b1052a07531066f5009a80706be) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "gmd.02",   0x28000, 0x20000, CRC(3fd39dde) SHA1(6a6e3ef9baa430ee83ab2312aa0221bae4d73dbd) )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "gmd.11m",   0x000000, 0x400000, CRC(06a65542) SHA1(a1b3df70c90055a3cd59d0149fd18a74eff5bcc9) )
	ROM_LOAD16_WORD_SWAP( "gmd.12m",   0x400000, 0x400000, CRC(50bc7a31) SHA1(7283569fc646c39f4c693f14e0ce7ff2ee49111a) )
ROM_END

ROM_START( dstlk )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "vame.03a", 0x000000, 0x80000, CRC(004c9cff) SHA1(9cab8ab734b29abf321b47e46271dab549bf46df) )
	ROM_LOAD16_WORD_SWAP( "vame.04a", 0x080000, 0x80000, CRC(ae413ff2) SHA1(e9b85ac04d6d1a57368c70aa24e3ab8a8d67409f) )
	ROM_LOAD16_WORD_SWAP( "vame.05a", 0x100000, 0x80000, CRC(60678756) SHA1(5d10829ad7522b5de3b318dd8cbf1b506ba4c2d4) )
	ROM_LOAD16_WORD_SWAP( "vame.06a", 0x180000, 0x80000, CRC(912870b3) SHA1(9c7620c7e25d236050411ba94fbc5b3b501970a3) )
	ROM_LOAD16_WORD_SWAP( "vame.07a", 0x200000, 0x80000, CRC(dabae3e8) SHA1(126f8433491db36649f5e1908bbe45eb123048e4) )
	ROM_LOAD16_WORD_SWAP( "vame.08a", 0x280000, 0x80000, CRC(2c6e3077) SHA1(d8042312ec546e3e807e3ef0a14af9b4f716e415) )
	ROM_LOAD16_WORD_SWAP( "vame.09a", 0x300000, 0x80000, CRC(f16db74b) SHA1(7b7e31916a61e7fb35ec20849c6d22d74e169ec0) )
	ROM_LOAD16_WORD_SWAP( "vame.10a", 0x380000, 0x80000, CRC(701e2147) SHA1(c0a0603e01fbed67a600b83902091c1073e2ed27) )

	ROM_REGION( 0x1400000, "gfx", 0 )
	ROMX_LOAD( "vam.13m",   0x0000000, 0x400000, CRC(c51baf99) SHA1(2fb6642908e542e404391eb17392f8270e87bf48) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.15m",   0x0000002, 0x400000, CRC(3ce83c77) SHA1(93369b23c6d7d834297434691bb047ee3dd9539c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.17m",   0x0000004, 0x400000, CRC(4f2408e0) SHA1(cd49c6b3c7e6470c6058f98ccc5210b052bb13e2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.19m",   0x0000006, 0x400000, CRC(9ff60250) SHA1(d69ba4dc6bd37d003245f0cf3211d6e2623005b8) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.14m",   0x1000000, 0x100000, CRC(bd87243c) SHA1(87b33aeb72514e1228ffc27ec6dd534f14882760) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.16m",   0x1000002, 0x100000, CRC(afec855f) SHA1(cd117833b8d475489b90ff44b57e2c5cb1af3af5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.18m",   0x1000004, 0x100000, CRC(3a033625) SHA1(294238f30cba5cf4f8f1de951d54c2077bd95de9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.20m",   0x1000006, 0x100000, CRC(2bff6a89) SHA1(8f4e131e5ce0af48fb89f98026d9f0356c7c301f) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "vam.01",   0x00000, 0x08000, CRC(64b685d5) SHA1(6c180e7420db754eca5cad17a40f5a64f5c3bd15) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "vam.02",   0x28000, 0x20000, CRC(cf7c97c7) SHA1(109a4b56ecd59be9c3f5869de99d40619bdaef21) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "vam.11m",   0x000000, 0x200000, CRC(4a39deb2) SHA1(7e63e615869958db66a4e52a0272afee5a10e446) )
	ROM_LOAD16_WORD_SWAP( "vam.12m",   0x200000, 0x200000, CRC(1a3e5c03) SHA1(c5a556e125d6c3d68da745b4d56cd7a851f2a23d) )
ROM_END

ROM_START( dstlku )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "vamu.03b", 0x000000, 0x80000, CRC(68a6343f) SHA1(9e1b13e3419470b3c14065c85342b2dcf42eb4cd) )
	ROM_LOAD16_WORD_SWAP( "vamu.04b", 0x080000, 0x80000, CRC(58161453) SHA1(7b5674b8bdb7e3165e090105f6716073696d4bd0) )
	ROM_LOAD16_WORD_SWAP( "vamu.05b", 0x100000, 0x80000, CRC(dfc038b8) SHA1(1b8911033a458f2d20f740c1bd1b3a2157d24b8a) )
	ROM_LOAD16_WORD_SWAP( "vamu.06b", 0x180000, 0x80000, CRC(c3842c89) SHA1(38137ae2c4ec2a6523413c0891287ad7ae70f005) )
	ROM_LOAD16_WORD_SWAP( "vamu.07b", 0x200000, 0x80000, CRC(25b60b6e) SHA1(8b7dc014d1953a6f4c003811ef8813d46136959d) )
	ROM_LOAD16_WORD_SWAP( "vamu.08b", 0x280000, 0x80000, CRC(2113c596) SHA1(6c0e5c406c08af922920500679eaa89e0b83f029) )
	ROM_LOAD16_WORD_SWAP( "vamu.09b", 0x300000, 0x80000, CRC(2d1e9ae5) SHA1(1c4aced7dd0356ee445ca1e5db2c3a2ad4ee56c6) )
	ROM_LOAD16_WORD_SWAP( "vamu.10b", 0x380000, 0x80000, CRC(81145622) SHA1(66c5439b564cea4b49c47db7e095283481d962c7) )

	ROM_REGION( 0x1400000, "gfx", 0 )
	ROMX_LOAD( "vam.13m",   0x0000000, 0x400000, CRC(c51baf99) SHA1(2fb6642908e542e404391eb17392f8270e87bf48) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.15m",   0x0000002, 0x400000, CRC(3ce83c77) SHA1(93369b23c6d7d834297434691bb047ee3dd9539c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.17m",   0x0000004, 0x400000, CRC(4f2408e0) SHA1(cd49c6b3c7e6470c6058f98ccc5210b052bb13e2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.19m",   0x0000006, 0x400000, CRC(9ff60250) SHA1(d69ba4dc6bd37d003245f0cf3211d6e2623005b8) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.14m",   0x1000000, 0x100000, CRC(bd87243c) SHA1(87b33aeb72514e1228ffc27ec6dd534f14882760) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.16m",   0x1000002, 0x100000, CRC(afec855f) SHA1(cd117833b8d475489b90ff44b57e2c5cb1af3af5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.18m",   0x1000004, 0x100000, CRC(3a033625) SHA1(294238f30cba5cf4f8f1de951d54c2077bd95de9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.20m",   0x1000006, 0x100000, CRC(2bff6a89) SHA1(8f4e131e5ce0af48fb89f98026d9f0356c7c301f) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "vam.01",   0x00000, 0x08000, CRC(64b685d5) SHA1(6c180e7420db754eca5cad17a40f5a64f5c3bd15) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "vam.02",   0x28000, 0x20000, CRC(cf7c97c7) SHA1(109a4b56ecd59be9c3f5869de99d40619bdaef21) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "vam.11m",   0x000000, 0x200000, CRC(4a39deb2) SHA1(7e63e615869958db66a4e52a0272afee5a10e446) )
	ROM_LOAD16_WORD_SWAP( "vam.12m",   0x200000, 0x200000, CRC(1a3e5c03) SHA1(c5a556e125d6c3d68da745b4d56cd7a851f2a23d) )
ROM_END

ROM_START( dstlkur1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "vamu.03a", 0x000000, 0x80000, CRC(628899f9) SHA1(989414a62aed67504f15a542a148e32a4b349949) )
	ROM_LOAD16_WORD_SWAP( "vamu.04a", 0x080000, 0x80000, CRC(696d9b25) SHA1(743c53ac7fc27960ecc80fed3f2a3c506ee655a1) )
	ROM_LOAD16_WORD_SWAP( "vamu.05a", 0x100000, 0x80000, CRC(673ed50a) SHA1(7dff27dba1da55a18eb459e4a2d679cf699f2804) )
	ROM_LOAD16_WORD_SWAP( "vamu.06a", 0x180000, 0x80000, CRC(f2377be7) SHA1(4520d44f94a03bd40c27062344e56ba8718c2fb8) )
	ROM_LOAD16_WORD_SWAP( "vamu.07a", 0x200000, 0x80000, CRC(d8f498c4) SHA1(569d9c309e9d95d2501a7c0a2c1291b49320d767) )
	ROM_LOAD16_WORD_SWAP( "vamu.08a", 0x280000, 0x80000, CRC(e6a8a1a0) SHA1(adf621e12623a2af4dbf0858a8fa3816e7c7073b) )
	ROM_LOAD16_WORD_SWAP( "vamu.09a", 0x300000, 0x80000, CRC(8dd55b24) SHA1(d99c2cbc4a9899a3d187201e6e730b7b8fb13d1d) )
	ROM_LOAD16_WORD_SWAP( "vamu.10a", 0x380000, 0x80000, CRC(c1a3d9be) SHA1(82b4ce3325a7ecf3a60dd781f9b224fdde8daa65) )

	ROM_REGION( 0x1400000, "gfx", 0 )
	ROMX_LOAD( "vam.13m",   0x0000000, 0x400000, CRC(c51baf99) SHA1(2fb6642908e542e404391eb17392f8270e87bf48) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.15m",   0x0000002, 0x400000, CRC(3ce83c77) SHA1(93369b23c6d7d834297434691bb047ee3dd9539c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.17m",   0x0000004, 0x400000, CRC(4f2408e0) SHA1(cd49c6b3c7e6470c6058f98ccc5210b052bb13e2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.19m",   0x0000006, 0x400000, CRC(9ff60250) SHA1(d69ba4dc6bd37d003245f0cf3211d6e2623005b8) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.14m",   0x1000000, 0x100000, CRC(bd87243c) SHA1(87b33aeb72514e1228ffc27ec6dd534f14882760) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.16m",   0x1000002, 0x100000, CRC(afec855f) SHA1(cd117833b8d475489b90ff44b57e2c5cb1af3af5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.18m",   0x1000004, 0x100000, CRC(3a033625) SHA1(294238f30cba5cf4f8f1de951d54c2077bd95de9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.20m",   0x1000006, 0x100000, CRC(2bff6a89) SHA1(8f4e131e5ce0af48fb89f98026d9f0356c7c301f) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "vam.01",   0x00000, 0x08000, CRC(64b685d5) SHA1(6c180e7420db754eca5cad17a40f5a64f5c3bd15) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "vam.02",   0x28000, 0x20000, CRC(cf7c97c7) SHA1(109a4b56ecd59be9c3f5869de99d40619bdaef21) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "vam.11m",   0x000000, 0x200000, CRC(4a39deb2) SHA1(7e63e615869958db66a4e52a0272afee5a10e446) )
	ROM_LOAD16_WORD_SWAP( "vam.12m",   0x200000, 0x200000, CRC(1a3e5c03) SHA1(c5a556e125d6c3d68da745b4d56cd7a851f2a23d) )
ROM_END

ROM_START( vampj )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "vamj.03a", 0x000000, 0x80000, CRC(f55d3722) SHA1(e26bbcc47a2485914d567a6cf1cddd0f668689a1) )
	ROM_LOAD16_WORD_SWAP( "vamj.04b", 0x080000, 0x80000, CRC(4d9c43c4) SHA1(2087090306646fed959d503ee75e24996ad95b88) ) /* incomplete updated set, all */
	ROM_LOAD16_WORD_SWAP( "vamj.05a", 0x100000, 0x80000, CRC(6c497e92) SHA1(7c1ccdfd77fb50afe024c8402376daaeab641a24) ) /* roms should be "B" revision */
	ROM_LOAD16_WORD_SWAP( "vamj.06a", 0x180000, 0x80000, CRC(f1bbecb6) SHA1(6adba89393e05f16f70b57085cabd6b4c20f53e8) )
	ROM_LOAD16_WORD_SWAP( "vamj.07a", 0x200000, 0x80000, CRC(1067ad84) SHA1(5e4cc75cfdfd512b6230c656e7304262b5143aee) )
	ROM_LOAD16_WORD_SWAP( "vamj.08a", 0x280000, 0x80000, CRC(4b89f41f) SHA1(bd78f33a6d448655eecf7448921d282b302fa4cb) )
	ROM_LOAD16_WORD_SWAP( "vamj.09a", 0x300000, 0x80000, CRC(fc0a4aac) SHA1(a2c79eb4dc838c238e182a4da3567ac8db3488d8) )
	ROM_LOAD16_WORD_SWAP( "vamj.10a", 0x380000, 0x80000, CRC(9270c26b) SHA1(c2a7e199a74c9f27704cf935483ebddc6da256a1) )

	ROM_REGION( 0x1400000, "gfx", 0 )
	ROMX_LOAD( "vam.13m",   0x0000000, 0x400000, CRC(c51baf99) SHA1(2fb6642908e542e404391eb17392f8270e87bf48) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.15m",   0x0000002, 0x400000, CRC(3ce83c77) SHA1(93369b23c6d7d834297434691bb047ee3dd9539c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.17m",   0x0000004, 0x400000, CRC(4f2408e0) SHA1(cd49c6b3c7e6470c6058f98ccc5210b052bb13e2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.19m",   0x0000006, 0x400000, CRC(9ff60250) SHA1(d69ba4dc6bd37d003245f0cf3211d6e2623005b8) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.14m",   0x1000000, 0x100000, CRC(bd87243c) SHA1(87b33aeb72514e1228ffc27ec6dd534f14882760) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.16m",   0x1000002, 0x100000, CRC(afec855f) SHA1(cd117833b8d475489b90ff44b57e2c5cb1af3af5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.18m",   0x1000004, 0x100000, CRC(3a033625) SHA1(294238f30cba5cf4f8f1de951d54c2077bd95de9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.20m",   0x1000006, 0x100000, CRC(2bff6a89) SHA1(8f4e131e5ce0af48fb89f98026d9f0356c7c301f) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "vam.01",   0x00000, 0x08000, CRC(64b685d5) SHA1(6c180e7420db754eca5cad17a40f5a64f5c3bd15) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "vam.02",   0x28000, 0x20000, CRC(cf7c97c7) SHA1(109a4b56ecd59be9c3f5869de99d40619bdaef21) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "vam.11m",   0x000000, 0x200000, CRC(4a39deb2) SHA1(7e63e615869958db66a4e52a0272afee5a10e446) )
	ROM_LOAD16_WORD_SWAP( "vam.12m",   0x200000, 0x200000, CRC(1a3e5c03) SHA1(c5a556e125d6c3d68da745b4d56cd7a851f2a23d) )
ROM_END

ROM_START( vampja )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "vamj.03a", 0x000000, 0x80000, CRC(f55d3722) SHA1(e26bbcc47a2485914d567a6cf1cddd0f668689a1) )
	ROM_LOAD16_WORD_SWAP( "vamj.04a", 0x080000, 0x80000, CRC(fdcbdae3) SHA1(46a1251a2affbe13b200448cf77a455d840f3c9f) )
	ROM_LOAD16_WORD_SWAP( "vamj.05a", 0x100000, 0x80000, CRC(6c497e92) SHA1(7c1ccdfd77fb50afe024c8402376daaeab641a24) )
	ROM_LOAD16_WORD_SWAP( "vamj.06a", 0x180000, 0x80000, CRC(f1bbecb6) SHA1(6adba89393e05f16f70b57085cabd6b4c20f53e8) )
	ROM_LOAD16_WORD_SWAP( "vamj.07a", 0x200000, 0x80000, CRC(1067ad84) SHA1(5e4cc75cfdfd512b6230c656e7304262b5143aee) )
	ROM_LOAD16_WORD_SWAP( "vamj.08a", 0x280000, 0x80000, CRC(4b89f41f) SHA1(bd78f33a6d448655eecf7448921d282b302fa4cb) )
	ROM_LOAD16_WORD_SWAP( "vamj.09a", 0x300000, 0x80000, CRC(fc0a4aac) SHA1(a2c79eb4dc838c238e182a4da3567ac8db3488d8) )
	ROM_LOAD16_WORD_SWAP( "vamj.10a", 0x380000, 0x80000, CRC(9270c26b) SHA1(c2a7e199a74c9f27704cf935483ebddc6da256a1) )

	ROM_REGION( 0x1400000, "gfx", 0 )
	ROMX_LOAD( "vam.13m",   0x0000000, 0x400000, CRC(c51baf99) SHA1(2fb6642908e542e404391eb17392f8270e87bf48) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.15m",   0x0000002, 0x400000, CRC(3ce83c77) SHA1(93369b23c6d7d834297434691bb047ee3dd9539c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.17m",   0x0000004, 0x400000, CRC(4f2408e0) SHA1(cd49c6b3c7e6470c6058f98ccc5210b052bb13e2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.19m",   0x0000006, 0x400000, CRC(9ff60250) SHA1(d69ba4dc6bd37d003245f0cf3211d6e2623005b8) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.14m",   0x1000000, 0x100000, CRC(bd87243c) SHA1(87b33aeb72514e1228ffc27ec6dd534f14882760) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.16m",   0x1000002, 0x100000, CRC(afec855f) SHA1(cd117833b8d475489b90ff44b57e2c5cb1af3af5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.18m",   0x1000004, 0x100000, CRC(3a033625) SHA1(294238f30cba5cf4f8f1de951d54c2077bd95de9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.20m",   0x1000006, 0x100000, CRC(2bff6a89) SHA1(8f4e131e5ce0af48fb89f98026d9f0356c7c301f) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "vam.01",   0x00000, 0x08000, CRC(64b685d5) SHA1(6c180e7420db754eca5cad17a40f5a64f5c3bd15) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "vam.02",   0x28000, 0x20000, CRC(cf7c97c7) SHA1(109a4b56ecd59be9c3f5869de99d40619bdaef21) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "vam.11m",   0x000000, 0x200000, CRC(4a39deb2) SHA1(7e63e615869958db66a4e52a0272afee5a10e446) )
	ROM_LOAD16_WORD_SWAP( "vam.12m",   0x200000, 0x200000, CRC(1a3e5c03) SHA1(c5a556e125d6c3d68da745b4d56cd7a851f2a23d) )
ROM_END

ROM_START( vampjr1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "vamj.03", 0x000000, 0x80000, CRC(8895bf77) SHA1(7977dad8c4baf89f28668f54225233a8e759aa3e) )
	ROM_LOAD16_WORD_SWAP( "vamj.04", 0x080000, 0x80000, CRC(5027db3d) SHA1(64bd09f2b5fd2435d8ec86f64543b640ab08f82f) )
	ROM_LOAD16_WORD_SWAP( "vamj.05", 0x100000, 0x80000, CRC(97c66fdb) SHA1(fe5c099dd29797aef28a247913f8931aa8ce6160) )
	ROM_LOAD16_WORD_SWAP( "vamj.06", 0x180000, 0x80000, CRC(9b4c3426) SHA1(a527535e5d23c3d12bac7617fd5d8e15c2522bbd) )
	ROM_LOAD16_WORD_SWAP( "vamj.07", 0x200000, 0x80000, CRC(303bc4fd) SHA1(2e3b687c725d389afa7c3e1fe8720a53d0f40269) )
	ROM_LOAD16_WORD_SWAP( "vamj.08", 0x280000, 0x80000, CRC(3dea3646) SHA1(3b3f7105284a04b12b3de40633bc8f21a8d73f58) )
	ROM_LOAD16_WORD_SWAP( "vamj.09", 0x300000, 0x80000, CRC(c119a827) SHA1(422864dda2a12621175350b8a130f970ed690719) )
	ROM_LOAD16_WORD_SWAP( "vamj.10", 0x380000, 0x80000, CRC(46593b79) SHA1(ff003cc80ed4f3cfaff722b43a09076828c9a9d7) )

	ROM_REGION( 0x1400000, "gfx", 0 )
	ROMX_LOAD( "vam.13m",   0x0000000, 0x400000, CRC(c51baf99) SHA1(2fb6642908e542e404391eb17392f8270e87bf48) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.15m",   0x0000002, 0x400000, CRC(3ce83c77) SHA1(93369b23c6d7d834297434691bb047ee3dd9539c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.17m",   0x0000004, 0x400000, CRC(4f2408e0) SHA1(cd49c6b3c7e6470c6058f98ccc5210b052bb13e2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.19m",   0x0000006, 0x400000, CRC(9ff60250) SHA1(d69ba4dc6bd37d003245f0cf3211d6e2623005b8) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.14m",   0x1000000, 0x100000, CRC(bd87243c) SHA1(87b33aeb72514e1228ffc27ec6dd534f14882760) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.16m",   0x1000002, 0x100000, CRC(afec855f) SHA1(cd117833b8d475489b90ff44b57e2c5cb1af3af5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.18m",   0x1000004, 0x100000, CRC(3a033625) SHA1(294238f30cba5cf4f8f1de951d54c2077bd95de9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.20m",   0x1000006, 0x100000, CRC(2bff6a89) SHA1(8f4e131e5ce0af48fb89f98026d9f0356c7c301f) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "vam.01",   0x00000, 0x08000, CRC(64b685d5) SHA1(6c180e7420db754eca5cad17a40f5a64f5c3bd15) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "vam.02",   0x28000, 0x20000, CRC(cf7c97c7) SHA1(109a4b56ecd59be9c3f5869de99d40619bdaef21) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "vam.11m",   0x000000, 0x200000, CRC(4a39deb2) SHA1(7e63e615869958db66a4e52a0272afee5a10e446) )
	ROM_LOAD16_WORD_SWAP( "vam.12m",   0x200000, 0x200000, CRC(1a3e5c03) SHA1(c5a556e125d6c3d68da745b4d56cd7a851f2a23d) )
ROM_END

ROM_START( dstlka )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "vama.03a", 0x000000, 0x80000, CRC(294e0bec) SHA1(e90844cd18ad431e999e606d076738384b346b9d) )
	ROM_LOAD16_WORD_SWAP( "vama.04a", 0x080000, 0x80000, CRC(bc18e128) SHA1(53116cddb7123b573d76064640c3829fd978c67a) )
	ROM_LOAD16_WORD_SWAP( "vama.05a", 0x100000, 0x80000, CRC(e709fa59) SHA1(824d2b22c5627b9dba046b76c1ff5a46f577eddd) )
	ROM_LOAD16_WORD_SWAP( "vama.06a", 0x180000, 0x80000, CRC(55e4d387) SHA1(c8b9be072e5de44e6d50f7a80d4c79ae1449588e) )
	ROM_LOAD16_WORD_SWAP( "vama.07a", 0x200000, 0x80000, CRC(24e8f981) SHA1(5dd28efa325fded290d9eb1643be83ab84a2ac8e) )
	ROM_LOAD16_WORD_SWAP( "vama.08a", 0x280000, 0x80000, CRC(743f3a8e) SHA1(f7bde0f989582ba2cf93c9397cc38d3eec9ad92d) )
	ROM_LOAD16_WORD_SWAP( "vama.09a", 0x300000, 0x80000, CRC(67fa5573) SHA1(2dab32cf0d361d2c52cce5eb41b389a0e32dd192) )
	ROM_LOAD16_WORD_SWAP( "vama.10a", 0x380000, 0x80000, CRC(5e03d747) SHA1(044ef85ca927108f5e66967819dbf7c25bb34f77) )

	ROM_REGION( 0x1400000, "gfx", 0 )
	ROMX_LOAD( "vam.13m",   0x0000000, 0x400000, CRC(c51baf99) SHA1(2fb6642908e542e404391eb17392f8270e87bf48) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.15m",   0x0000002, 0x400000, CRC(3ce83c77) SHA1(93369b23c6d7d834297434691bb047ee3dd9539c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.17m",   0x0000004, 0x400000, CRC(4f2408e0) SHA1(cd49c6b3c7e6470c6058f98ccc5210b052bb13e2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.19m",   0x0000006, 0x400000, CRC(9ff60250) SHA1(d69ba4dc6bd37d003245f0cf3211d6e2623005b8) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.14m",   0x1000000, 0x100000, CRC(bd87243c) SHA1(87b33aeb72514e1228ffc27ec6dd534f14882760) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.16m",   0x1000002, 0x100000, CRC(afec855f) SHA1(cd117833b8d475489b90ff44b57e2c5cb1af3af5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.18m",   0x1000004, 0x100000, CRC(3a033625) SHA1(294238f30cba5cf4f8f1de951d54c2077bd95de9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.20m",   0x1000006, 0x100000, CRC(2bff6a89) SHA1(8f4e131e5ce0af48fb89f98026d9f0356c7c301f) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "vam.01",   0x00000, 0x08000, CRC(64b685d5) SHA1(6c180e7420db754eca5cad17a40f5a64f5c3bd15) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "vam.02",   0x28000, 0x20000, CRC(cf7c97c7) SHA1(109a4b56ecd59be9c3f5869de99d40619bdaef21) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "vam.11m",   0x000000, 0x200000, CRC(4a39deb2) SHA1(7e63e615869958db66a4e52a0272afee5a10e446) )
	ROM_LOAD16_WORD_SWAP( "vam.12m",   0x200000, 0x200000, CRC(1a3e5c03) SHA1(c5a556e125d6c3d68da745b4d56cd7a851f2a23d) )
ROM_END

ROM_START( dstlkh )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "vamh.03c", 0x000000, 0x80000, CRC(4d7b9e8f) SHA1(08555da1b024e6ab4dfe802352065c132ddc2abb) )
	ROM_LOAD16_WORD_SWAP( "vamh.04c", 0x080000, 0x80000, CRC(2217e9a0) SHA1(b86ee89457d8a0cf828f1bed247f3b2c0c91b170) )
	ROM_LOAD16_WORD_SWAP( "vamh.05c", 0x100000, 0x80000, CRC(3a05b13c) SHA1(14b58954bdff8dd699f867037a86f0bae8095e9d) )
	ROM_LOAD16_WORD_SWAP( "vamh.06c", 0x180000, 0x80000, CRC(11d70a1c) SHA1(e13c5afeb9cb64ec60d570b81d7fac4869c76d1d) )
	ROM_LOAD16_WORD_SWAP( "vamh.07c", 0x200000, 0x80000, CRC(db5a8767) SHA1(86274080e4423d09e10f2db56a4e685b32acfa18) )
	ROM_LOAD16_WORD_SWAP( "vamh.08c", 0x280000, 0x80000, CRC(2a4fd79b) SHA1(ff0398db43ef849365ad88b9b57661db3a3b65c6) )
	ROM_LOAD16_WORD_SWAP( "vamh.09c", 0x300000, 0x80000, CRC(15187632) SHA1(81b7166334dc3c331673822c31581e0e7809b698) )
	ROM_LOAD16_WORD_SWAP( "vamh.10c", 0x380000, 0x80000, CRC(192d2d81) SHA1(ea99f2ea3e28edfc203e967924500dad10abb43f) )

	ROM_REGION( 0x1400000, "gfx", 0 )
	ROMX_LOAD( "vam.13m",   0x0000000, 0x400000, CRC(c51baf99) SHA1(2fb6642908e542e404391eb17392f8270e87bf48) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.15m",   0x0000002, 0x400000, CRC(3ce83c77) SHA1(93369b23c6d7d834297434691bb047ee3dd9539c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.17m",   0x0000004, 0x400000, CRC(4f2408e0) SHA1(cd49c6b3c7e6470c6058f98ccc5210b052bb13e2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.19m",   0x0000006, 0x400000, CRC(9ff60250) SHA1(d69ba4dc6bd37d003245f0cf3211d6e2623005b8) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.14m",   0x1000000, 0x100000, CRC(bd87243c) SHA1(87b33aeb72514e1228ffc27ec6dd534f14882760) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.16m",   0x1000002, 0x100000, CRC(afec855f) SHA1(cd117833b8d475489b90ff44b57e2c5cb1af3af5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.18m",   0x1000004, 0x100000, CRC(3a033625) SHA1(294238f30cba5cf4f8f1de951d54c2077bd95de9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.20m",   0x1000006, 0x100000, CRC(2bff6a89) SHA1(8f4e131e5ce0af48fb89f98026d9f0356c7c301f) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "vam.01",   0x00000, 0x08000, CRC(64b685d5) SHA1(6c180e7420db754eca5cad17a40f5a64f5c3bd15) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "vam.02",   0x28000, 0x20000, CRC(cf7c97c7) SHA1(109a4b56ecd59be9c3f5869de99d40619bdaef21) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "vam.11m",   0x000000, 0x200000, CRC(4a39deb2) SHA1(7e63e615869958db66a4e52a0272afee5a10e446) )
	ROM_LOAD16_WORD_SWAP( "vam.12m",   0x200000, 0x200000, CRC(1a3e5c03) SHA1(c5a556e125d6c3d68da745b4d56cd7a851f2a23d) )
ROM_END

ROM_START( ecofghtr )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "uece.03", 0x000000, 0x80000, CRC(ec2c1137) SHA1(19c5b68cccd682d3996faf8c5f07a644b2384b1c) )
	ROM_LOAD16_WORD_SWAP( "uece.04", 0x080000, 0x80000, CRC(b35f99db) SHA1(4dd5c4840406a9323431f5bda7224cadacf8b419) )
	ROM_LOAD16_WORD_SWAP( "uece.05", 0x100000, 0x80000, CRC(d9d42d31) SHA1(58e7438fa212655ca56cbb477ea353e1083e0933) )
	ROM_LOAD16_WORD_SWAP( "uece.06", 0x180000, 0x80000, CRC(9d9771cf) SHA1(d1c76672f2e0437cd1204d5552d32ed3377c1356) )

	ROM_REGION( 0xc00000, "gfx", 0 )
	ROMX_LOAD( "uec.13m",   0x000000, 0x200000, CRC(dcaf1436) SHA1(ba124cc0bb10c1d1c07592a3623add4ed054182e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "uec.15m",   0x000002, 0x200000, CRC(2807df41) SHA1(66a9800af435055737ce50a0b0ced7c5718c2004) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "uec.17m",   0x000004, 0x200000, CRC(8a708d02) SHA1(95ec527edc904a66e325667521b4d07d72579211) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "uec.19m",   0x000006, 0x200000, CRC(de7be0ef) SHA1(bf8df9a31f8923f4b726ea12fe8327368463ebe1) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "uec.14m",   0x800000, 0x100000, CRC(1a003558) SHA1(64bbd89e65dc0cf6f4ab5ea93a4cc6312d0d0802) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "uec.16m",   0x800002, 0x100000, CRC(4ff8a6f9) SHA1(03968a301417e8843d42d4e0db42aa0a3a38664b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "uec.18m",   0x800004, 0x100000, CRC(b167ae12) SHA1(48c552d02caad27d680aa51170560794f2a51478) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "uec.20m",   0x800006, 0x100000, CRC(1064bdc2) SHA1(c51f75ac8d3f02a771feda0a933314a928555c4e) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "uec.01",   0x00000, 0x08000, CRC(c235bd15) SHA1(feb7cd7db9dc0b9887b33eed9796bb0205fb719d) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "uec.11m",   0x000000, 0x200000, CRC(81b25d39) SHA1(448adfcc7d98873a48c710d857225cdd1580e5c9) )
	ROM_LOAD16_WORD_SWAP( "uec.12m",   0x200000, 0x200000, CRC(27729e52) SHA1(a55c8159adf766dda70cb047f5ac85ce6bc0a3f3) )
ROM_END

ROM_START( ecofghtru )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "uecu.03a", 0x000000, 0x80000, CRC(22d88a4d) SHA1(0aa5a4b51ae98b8b3bfc65aef9449796ffad7f10) )
	ROM_LOAD16_WORD_SWAP( "uecu.04a", 0x080000, 0x80000, CRC(6436cfcd) SHA1(adb4e1ab2a01a1ea1b08a76ecf58654450e13cf9) )
	ROM_LOAD16_WORD_SWAP( "uecu.05a", 0x100000, 0x80000, CRC(336f121b) SHA1(93800c459b516382cc62cebeb456274f48322fab) )
	ROM_LOAD16_WORD_SWAP( "uecu.06a", 0x180000, 0x80000, CRC(6f99d984) SHA1(3f962197edab648bf87c1f2976956ea21e7ac3c4) )

	ROM_REGION( 0xc00000, "gfx", 0 )
	ROMX_LOAD( "uec.13m",   0x000000, 0x200000, CRC(dcaf1436) SHA1(ba124cc0bb10c1d1c07592a3623add4ed054182e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "uec.15m",   0x000002, 0x200000, CRC(2807df41) SHA1(66a9800af435055737ce50a0b0ced7c5718c2004) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "uec.17m",   0x000004, 0x200000, CRC(8a708d02) SHA1(95ec527edc904a66e325667521b4d07d72579211) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "uec.19m",   0x000006, 0x200000, CRC(de7be0ef) SHA1(bf8df9a31f8923f4b726ea12fe8327368463ebe1) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "uec.14m",   0x800000, 0x100000, CRC(1a003558) SHA1(64bbd89e65dc0cf6f4ab5ea93a4cc6312d0d0802) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "uec.16m",   0x800002, 0x100000, CRC(4ff8a6f9) SHA1(03968a301417e8843d42d4e0db42aa0a3a38664b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "uec.18m",   0x800004, 0x100000, CRC(b167ae12) SHA1(48c552d02caad27d680aa51170560794f2a51478) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "uec.20m",   0x800006, 0x100000, CRC(1064bdc2) SHA1(c51f75ac8d3f02a771feda0a933314a928555c4e) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "uec.01",   0x00000, 0x08000, CRC(c235bd15) SHA1(feb7cd7db9dc0b9887b33eed9796bb0205fb719d) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "uec.11m",   0x000000, 0x200000, CRC(81b25d39) SHA1(448adfcc7d98873a48c710d857225cdd1580e5c9) )
	ROM_LOAD16_WORD_SWAP( "uec.12m",   0x200000, 0x200000, CRC(27729e52) SHA1(a55c8159adf766dda70cb047f5ac85ce6bc0a3f3) )
ROM_END

ROM_START( ecofghtru1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "uecu.03", 0x000000, 0x80000, CRC(6792480c) SHA1(89d7095a36a1094237f9e1d82a0dc482409999ca) )
	ROM_LOAD16_WORD_SWAP( "uecu.04", 0x080000, 0x80000, CRC(95ce69d5) SHA1(d32c7e2a99ae29cbf9fee1e092a418f300a218ca) )
	ROM_LOAD16_WORD_SWAP( "uecu.05", 0x100000, 0x80000, CRC(3a1e78ad) SHA1(133b65cd4bfd45a9d9d3feec9ccb1e6fc2891818) )
	ROM_LOAD16_WORD_SWAP( "uecu.06", 0x180000, 0x80000, CRC(a3e2f3cc) SHA1(45272d49cf6927eb33af275c003ce6eb1766f045) )

	ROM_REGION( 0xc00000, "gfx", 0 )
	ROMX_LOAD( "uec.13m",   0x000000, 0x200000, CRC(dcaf1436) SHA1(ba124cc0bb10c1d1c07592a3623add4ed054182e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "uec.15m",   0x000002, 0x200000, CRC(2807df41) SHA1(66a9800af435055737ce50a0b0ced7c5718c2004) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "uec.17m",   0x000004, 0x200000, CRC(8a708d02) SHA1(95ec527edc904a66e325667521b4d07d72579211) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "uec.19m",   0x000006, 0x200000, CRC(de7be0ef) SHA1(bf8df9a31f8923f4b726ea12fe8327368463ebe1) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "uec.14m",   0x800000, 0x100000, CRC(1a003558) SHA1(64bbd89e65dc0cf6f4ab5ea93a4cc6312d0d0802) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "uec.16m",   0x800002, 0x100000, CRC(4ff8a6f9) SHA1(03968a301417e8843d42d4e0db42aa0a3a38664b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "uec.18m",   0x800004, 0x100000, CRC(b167ae12) SHA1(48c552d02caad27d680aa51170560794f2a51478) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "uec.20m",   0x800006, 0x100000, CRC(1064bdc2) SHA1(c51f75ac8d3f02a771feda0a933314a928555c4e) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "uec.01",   0x00000, 0x08000, CRC(c235bd15) SHA1(feb7cd7db9dc0b9887b33eed9796bb0205fb719d) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "uec.11m",   0x000000, 0x200000, CRC(81b25d39) SHA1(448adfcc7d98873a48c710d857225cdd1580e5c9) )
	ROM_LOAD16_WORD_SWAP( "uec.12m",   0x200000, 0x200000, CRC(27729e52) SHA1(a55c8159adf766dda70cb047f5ac85ce6bc0a3f3) )
ROM_END

ROM_START( uecology )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "uecj.03", 0x000000, 0x80000, CRC(94c40a4c) SHA1(6446b22a30a9a3c87b7a9fc2f15fbceccfbfb942) )
	ROM_LOAD16_WORD_SWAP( "uecj.04", 0x080000, 0x80000, CRC(8d6e3a09) SHA1(80167275f288a4c4b2bb61bdde956015f4206b78) )
	ROM_LOAD16_WORD_SWAP( "uecj.05", 0x100000, 0x80000, CRC(8604ecd7) SHA1(e1690565b40db84f4ce30e6eb2d7940b82989678) )
	ROM_LOAD16_WORD_SWAP( "uecj.06", 0x180000, 0x80000, CRC(b7e1d31f) SHA1(6567f14af9fd567dea963fda5cd37c55cab30704) )

	ROM_REGION( 0xc00000, "gfx", 0 )
	ROMX_LOAD( "uec.13m",   0x000000, 0x200000, CRC(dcaf1436) SHA1(ba124cc0bb10c1d1c07592a3623add4ed054182e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "uec.15m",   0x000002, 0x200000, CRC(2807df41) SHA1(66a9800af435055737ce50a0b0ced7c5718c2004) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "uec.17m",   0x000004, 0x200000, CRC(8a708d02) SHA1(95ec527edc904a66e325667521b4d07d72579211) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "uec.19m",   0x000006, 0x200000, CRC(de7be0ef) SHA1(bf8df9a31f8923f4b726ea12fe8327368463ebe1) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "uec.14m",   0x800000, 0x100000, CRC(1a003558) SHA1(64bbd89e65dc0cf6f4ab5ea93a4cc6312d0d0802) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "uec.16m",   0x800002, 0x100000, CRC(4ff8a6f9) SHA1(03968a301417e8843d42d4e0db42aa0a3a38664b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "uec.18m",   0x800004, 0x100000, CRC(b167ae12) SHA1(48c552d02caad27d680aa51170560794f2a51478) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "uec.20m",   0x800006, 0x100000, CRC(1064bdc2) SHA1(c51f75ac8d3f02a771feda0a933314a928555c4e) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "uec.01",   0x00000, 0x08000, CRC(c235bd15) SHA1(feb7cd7db9dc0b9887b33eed9796bb0205fb719d) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "uec.11m",   0x000000, 0x200000, CRC(81b25d39) SHA1(448adfcc7d98873a48c710d857225cdd1580e5c9) )
	ROM_LOAD16_WORD_SWAP( "uec.12m",   0x200000, 0x200000, CRC(27729e52) SHA1(a55c8159adf766dda70cb047f5ac85ce6bc0a3f3) )
ROM_END

ROM_START( ecofghtra )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "ueca.03", 0x000000, 0x80000, CRC(bd4589b1) SHA1(8ec03a750de155c6ce0a2c3a6b57e6a6dcaf9ebc) )
	ROM_LOAD16_WORD_SWAP( "ueca.04", 0x080000, 0x80000, CRC(1d134b7d) SHA1(c9dd725ff45f29a3fa68bfe6d5aea2e8c3c64bd8) )
	ROM_LOAD16_WORD_SWAP( "ueca.05", 0x100000, 0x80000, CRC(9c581fc7) SHA1(300983148da59da7d2fcbc5bc45b068fdfbcb512) )
	ROM_LOAD16_WORD_SWAP( "ueca.06", 0x180000, 0x80000, CRC(c92a7c50) SHA1(820dfa8fff32404caee65a7a5bcf7cafa9939f74) )

	ROM_REGION( 0xc00000, "gfx", 0 )
	ROMX_LOAD( "uec.13m",   0x000000, 0x200000, CRC(dcaf1436) SHA1(ba124cc0bb10c1d1c07592a3623add4ed054182e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "uec.15m",   0x000002, 0x200000, CRC(2807df41) SHA1(66a9800af435055737ce50a0b0ced7c5718c2004) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "uec.17m",   0x000004, 0x200000, CRC(8a708d02) SHA1(95ec527edc904a66e325667521b4d07d72579211) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "uec.19m",   0x000006, 0x200000, CRC(de7be0ef) SHA1(bf8df9a31f8923f4b726ea12fe8327368463ebe1) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "uec.14m",   0x800000, 0x100000, CRC(1a003558) SHA1(64bbd89e65dc0cf6f4ab5ea93a4cc6312d0d0802) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "uec.16m",   0x800002, 0x100000, CRC(4ff8a6f9) SHA1(03968a301417e8843d42d4e0db42aa0a3a38664b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "uec.18m",   0x800004, 0x100000, CRC(b167ae12) SHA1(48c552d02caad27d680aa51170560794f2a51478) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "uec.20m",   0x800006, 0x100000, CRC(1064bdc2) SHA1(c51f75ac8d3f02a771feda0a933314a928555c4e) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "uec.01",   0x00000, 0x08000, CRC(c235bd15) SHA1(feb7cd7db9dc0b9887b33eed9796bb0205fb719d) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "uec.11m",   0x000000, 0x200000, CRC(81b25d39) SHA1(448adfcc7d98873a48c710d857225cdd1580e5c9) )
	ROM_LOAD16_WORD_SWAP( "uec.12m",   0x200000, 0x200000, CRC(27729e52) SHA1(a55c8159adf766dda70cb047f5ac85ce6bc0a3f3) )
ROM_END

ROM_START( ecofghtrh )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "uech.03", 0x000000, 0x80000, CRC(14c9365e) SHA1(caba5e617a2c9516c6f24a327451edcff39b1bfd) )
	ROM_LOAD16_WORD_SWAP( "uech.04", 0x080000, 0x80000, CRC(579495dc) SHA1(4b0f59da30c86d6c433429c46b4f24c7c55d6731) )
	ROM_LOAD16_WORD_SWAP( "uech.05", 0x100000, 0x80000, CRC(96807a8e) SHA1(6f12d5fbff17797338f26e2b32e9f27d8288262e) )
	ROM_LOAD16_WORD_SWAP( "uech.06", 0x180000, 0x80000, CRC(682b9dbc) SHA1(80d86283ce850f1e9ac868a7e77819d2876df982) )

	ROM_REGION( 0xc00000, "gfx", 0 )
	ROMX_LOAD( "uec.13m",   0x000000, 0x200000, CRC(dcaf1436) SHA1(ba124cc0bb10c1d1c07592a3623add4ed054182e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "uec.15m",   0x000002, 0x200000, CRC(2807df41) SHA1(66a9800af435055737ce50a0b0ced7c5718c2004) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "uec.17m",   0x000004, 0x200000, CRC(8a708d02) SHA1(95ec527edc904a66e325667521b4d07d72579211) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "uec.19m",   0x000006, 0x200000, CRC(de7be0ef) SHA1(bf8df9a31f8923f4b726ea12fe8327368463ebe1) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "uec.14m",   0x800000, 0x100000, CRC(1a003558) SHA1(64bbd89e65dc0cf6f4ab5ea93a4cc6312d0d0802) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "uec.16m",   0x800002, 0x100000, CRC(4ff8a6f9) SHA1(03968a301417e8843d42d4e0db42aa0a3a38664b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "uec.18m",   0x800004, 0x100000, CRC(b167ae12) SHA1(48c552d02caad27d680aa51170560794f2a51478) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "uec.20m",   0x800006, 0x100000, CRC(1064bdc2) SHA1(c51f75ac8d3f02a771feda0a933314a928555c4e) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "uec.01",   0x00000, 0x08000, CRC(c235bd15) SHA1(feb7cd7db9dc0b9887b33eed9796bb0205fb719d) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "uec.11m",   0x000000, 0x200000, CRC(81b25d39) SHA1(448adfcc7d98873a48c710d857225cdd1580e5c9) )
	ROM_LOAD16_WORD_SWAP( "uec.12m",   0x200000, 0x200000, CRC(27729e52) SHA1(a55c8159adf766dda70cb047f5ac85ce6bc0a3f3) )
ROM_END

ROM_START( gigawing )
	ROM_REGION(CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "ggwu.03", 0x000000, 0x80000, CRC(ac725eb2) SHA1(a4be9fe537cdb47b37478c8397f6effe8a536233) )
	ROM_LOAD16_WORD_SWAP( "ggwu.04", 0x080000, 0x80000, CRC(392f4118) SHA1(3bb0bd9503ef60892d5abd8640af524cf71da848) )
	ROM_LOAD16_WORD_SWAP( "ggw.05",  0x100000, 0x80000, CRC(3239d642) SHA1(2fe3984c46a72aedb30a28e3db5af2612bdf0045) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROMX_LOAD( "ggw.13m",   0x000000, 0x400000, CRC(105530a4) SHA1(3be06c032985ea6bd3805d73a407bf748385087b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ggw.15m",   0x000002, 0x400000, CRC(9e774ab9) SHA1(adea1e844f3d9ccd5ad116ff8277f16a96e68d76) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ggw.17m",   0x000004, 0x400000, CRC(466e0ba4) SHA1(9563455b95d36fafe508290659088b153539cfdf) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ggw.19m",   0x000006, 0x400000, CRC(840c8dea) SHA1(ea04afce17f00b45d3d2cd5140d0dd7ab4bccc00) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION(QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ggw.01",   0x00000, 0x08000, CRC(4c6351d5) SHA1(cef81fb7c4b8cb2ef1f8f3c27982aefbcbe38160) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "ggw.11m",   0x000000, 0x400000, CRC(e172acf5) SHA1(d7b0963d66165f3607d887741c5e7ab952bcf2ff) )
	ROM_LOAD16_WORD_SWAP( "ggw.12m",   0x400000, 0x400000, CRC(4bee4e8f) SHA1(c440b5a38359ec3b8002f39690b79bf78703f5d0) )
ROM_END

ROM_START( gigawingj )
	ROM_REGION(CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "ggwj.03a", 0x000000, 0x80000, CRC(fdd23b91) SHA1(c805473d3dc6bdb7ce232a9d7181d213544b2e7b) )
	ROM_LOAD16_WORD_SWAP( "ggwj.04a", 0x080000, 0x80000, CRC(8c6e093c) SHA1(a4864b3b54cf648af81f74e2936d2bb8b99d68a9) )
	ROM_LOAD16_WORD_SWAP( "ggwj.05a", 0x100000, 0x80000, CRC(43811454) SHA1(2a9563c840bd934c7e94f434a01686b7ff92e6d2) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROMX_LOAD( "ggw.13m",   0x000000, 0x400000, CRC(105530a4) SHA1(3be06c032985ea6bd3805d73a407bf748385087b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ggw.15m",   0x000002, 0x400000, CRC(9e774ab9) SHA1(adea1e844f3d9ccd5ad116ff8277f16a96e68d76) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ggw.17m",   0x000004, 0x400000, CRC(466e0ba4) SHA1(9563455b95d36fafe508290659088b153539cfdf) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ggw.19m",   0x000006, 0x400000, CRC(840c8dea) SHA1(ea04afce17f00b45d3d2cd5140d0dd7ab4bccc00) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION(QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ggw.01",   0x00000, 0x08000, CRC(4c6351d5) SHA1(cef81fb7c4b8cb2ef1f8f3c27982aefbcbe38160) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "ggw.11m",   0x000000, 0x400000, CRC(e172acf5) SHA1(d7b0963d66165f3607d887741c5e7ab952bcf2ff) )
	ROM_LOAD16_WORD_SWAP( "ggw.12m",   0x400000, 0x400000, CRC(4bee4e8f) SHA1(c440b5a38359ec3b8002f39690b79bf78703f5d0) )
ROM_END

ROM_START( gigawinga )
	ROM_REGION(CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "ggwa.03a", 0x000000, 0x80000, CRC(116f8837) SHA1(9ade6c2fae319e0111e7afd3af10096d8d88f0c5) )
	ROM_LOAD16_WORD_SWAP( "ggwa.04a", 0x080000, 0x80000, CRC(e6e3f0c4) SHA1(3c28cc050f36fb070a8abf057f0972dc16bc5629) )
	ROM_LOAD16_WORD_SWAP( "ggwa.05a", 0x100000, 0x80000, CRC(465e8ac9) SHA1(da94fb64c7ea64ac9e4e847b69e2b870f716dd34) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROMX_LOAD( "ggw.13m",   0x000000, 0x400000, CRC(105530a4) SHA1(3be06c032985ea6bd3805d73a407bf748385087b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ggw.15m",   0x000002, 0x400000, CRC(9e774ab9) SHA1(adea1e844f3d9ccd5ad116ff8277f16a96e68d76) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ggw.17m",   0x000004, 0x400000, CRC(466e0ba4) SHA1(9563455b95d36fafe508290659088b153539cfdf) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ggw.19m",   0x000006, 0x400000, CRC(840c8dea) SHA1(ea04afce17f00b45d3d2cd5140d0dd7ab4bccc00) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION(QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ggw.01",   0x00000, 0x08000, CRC(4c6351d5) SHA1(cef81fb7c4b8cb2ef1f8f3c27982aefbcbe38160) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "ggw.11m",   0x000000, 0x400000, CRC(e172acf5) SHA1(d7b0963d66165f3607d887741c5e7ab952bcf2ff) )
	ROM_LOAD16_WORD_SWAP( "ggw.12m",   0x400000, 0x400000, CRC(4bee4e8f) SHA1(c440b5a38359ec3b8002f39690b79bf78703f5d0) )
ROM_END

ROM_START( gigawingh )
	ROM_REGION(CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "ggwh.03a", 0x000000, 0x80000, CRC(b9ee36eb) SHA1(7b977dcd27c1ed8f9d2ebaccf2497abeafcc6081) )
	ROM_LOAD16_WORD_SWAP( "ggwh.04a", 0x080000, 0x80000, CRC(72e548fe) SHA1(67d4870d6f7df4b947f1ac6f6605a2344bfc04c1) )
	ROM_LOAD16_WORD_SWAP( "ggw.05",   0x100000, 0x80000, CRC(3239d642) SHA1(2fe3984c46a72aedb30a28e3db5af2612bdf0045) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROMX_LOAD( "ggw.13m",   0x000000, 0x400000, CRC(105530a4) SHA1(3be06c032985ea6bd3805d73a407bf748385087b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ggw.15m",   0x000002, 0x400000, CRC(9e774ab9) SHA1(adea1e844f3d9ccd5ad116ff8277f16a96e68d76) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ggw.17m",   0x000004, 0x400000, CRC(466e0ba4) SHA1(9563455b95d36fafe508290659088b153539cfdf) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ggw.19m",   0x000006, 0x400000, CRC(840c8dea) SHA1(ea04afce17f00b45d3d2cd5140d0dd7ab4bccc00) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION(QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ggw.01",   0x00000, 0x08000, CRC(4c6351d5) SHA1(cef81fb7c4b8cb2ef1f8f3c27982aefbcbe38160) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "ggw.11m",   0x000000, 0x400000, CRC(e172acf5) SHA1(d7b0963d66165f3607d887741c5e7ab952bcf2ff) )
	ROM_LOAD16_WORD_SWAP( "ggw.12m",   0x400000, 0x400000, CRC(4bee4e8f) SHA1(c440b5a38359ec3b8002f39690b79bf78703f5d0) )
ROM_END

ROM_START( gigawingb )
	ROM_REGION(CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "ggwb.03", 0x000000, 0x80000, CRC(a1f8a448) SHA1(a88ed9fea9ec08fda59a1ca9622dfb9f8afdda78) )
	ROM_LOAD16_WORD_SWAP( "ggwb.04", 0x080000, 0x80000, CRC(6a423e76) SHA1(f6ed20f09e852b4fad78ee32617122d5deb98789) )
	ROM_LOAD16_WORD_SWAP( "ggw.05",  0x100000, 0x80000, CRC(3239d642) SHA1(2fe3984c46a72aedb30a28e3db5af2612bdf0045) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROMX_LOAD( "ggw.13m",   0x000000, 0x400000, CRC(105530a4) SHA1(3be06c032985ea6bd3805d73a407bf748385087b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ggw.15m",   0x000002, 0x400000, CRC(9e774ab9) SHA1(adea1e844f3d9ccd5ad116ff8277f16a96e68d76) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ggw.17m",   0x000004, 0x400000, CRC(466e0ba4) SHA1(9563455b95d36fafe508290659088b153539cfdf) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ggw.19m",   0x000006, 0x400000, CRC(840c8dea) SHA1(ea04afce17f00b45d3d2cd5140d0dd7ab4bccc00) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION(QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ggw.01",   0x00000, 0x08000, CRC(4c6351d5) SHA1(cef81fb7c4b8cb2ef1f8f3c27982aefbcbe38160) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "ggw.11m",   0x000000, 0x400000, CRC(e172acf5) SHA1(d7b0963d66165f3607d887741c5e7ab952bcf2ff) )
	ROM_LOAD16_WORD_SWAP( "ggw.12m",   0x400000, 0x400000, CRC(4bee4e8f) SHA1(c440b5a38359ec3b8002f39690b79bf78703f5d0) )
ROM_END

ROM_START( hsf2 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "hs2u.03",  0x000000, 0x80000, CRC(b308151e) SHA1(afdfd3b049c6435e2291bc35d8c26ff5bff223d8) )
	ROM_LOAD16_WORD_SWAP( "hs2u.04",  0x080000, 0x80000, CRC(327aa49c) SHA1(6719cd6ecc2a4487fdbf5cbcd47e35fc43000607) )
	ROM_LOAD16_WORD_SWAP( "hs2.05",   0x100000, 0x80000, CRC(dde34a35) SHA1(f5be2d2916db6e86e0886d61d55bddf138273ebc) )
	ROM_LOAD16_WORD_SWAP( "hs2.06",   0x180000, 0x80000, CRC(f4e56dda) SHA1(c6490707c2a416ab88612c2d73abbe5853d8cb92) )
	ROM_LOAD16_WORD_SWAP( "hs2.07",   0x200000, 0x80000, CRC(ee4420fc) SHA1(06cf76660b0c794d2460c52d9fe8334fff51e9de) )
	ROM_LOAD16_WORD_SWAP( "hs2.08",   0x280000, 0x80000, CRC(c9441533) SHA1(bf178fac1f060fcce3ff9118333c8517dadc9429) )
	ROM_LOAD16_WORD_SWAP( "hs2.09",   0x300000, 0x80000, CRC(3fc638a8) SHA1(2a42877b26c8abc437da46225701f0bba6e40058) )
	ROM_LOAD16_WORD_SWAP( "hs2.10",   0x380000, 0x80000, CRC(20d0f9e4) SHA1(80a5eeef9472e327b0d4ee26434bad109a9434ea) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "hs2.13m",   0x0000000, 0x800000, CRC(a6ecab17) SHA1(6749a4c8dc81f4b10f910c31c82cf6674e2a44eb) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "hs2.15m",   0x0000002, 0x800000, CRC(10a0ae4d) SHA1(701b4900fbc8bef20efa1a706891c8df4bf14641) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "hs2.17m",   0x0000004, 0x800000, CRC(adfa7726) SHA1(8d36ec125a8c91abfe5213893d794f8bc11c8acd) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "hs2.19m",   0x0000006, 0x800000, CRC(bb3ae322) SHA1(ecd289d7a0fe365fdd7c5527cb17796002beb553) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "hs2.01",   0x00000, 0x08000, CRC(c1a13786) SHA1(c7392c7efb15ea4042e75bd9007e974293d8935d) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "hs2.02",   0x28000, 0x20000, CRC(2d8794aa) SHA1(c634affdc2568020cce6af97b4fa79925d9943f3) )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "hs2.11m",   0x000000, 0x800000, CRC(0e15c359) SHA1(176108b0d76d821a849324680aba0cd04b5016c1) )
ROM_END

ROM_START( hsf2a )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "hs2a.03",  0x000000, 0x80000, CRC(d50a17e0) SHA1(5d8d6d309260cc2d862aa080d44a72886ee08c77) )
	ROM_LOAD16_WORD_SWAP( "hs2a.04",  0x080000, 0x80000, CRC(a27f42de) SHA1(7a355831b57a35e327b2618fd5dca11afed2a233) )
	ROM_LOAD16_WORD_SWAP( "hs2.05",   0x100000, 0x80000, CRC(dde34a35) SHA1(f5be2d2916db6e86e0886d61d55bddf138273ebc) )
	ROM_LOAD16_WORD_SWAP( "hs2.06",   0x180000, 0x80000, CRC(f4e56dda) SHA1(c6490707c2a416ab88612c2d73abbe5853d8cb92) )
	ROM_LOAD16_WORD_SWAP( "hs2.07",   0x200000, 0x80000, CRC(ee4420fc) SHA1(06cf76660b0c794d2460c52d9fe8334fff51e9de) )
	ROM_LOAD16_WORD_SWAP( "hs2.08",   0x280000, 0x80000, CRC(c9441533) SHA1(bf178fac1f060fcce3ff9118333c8517dadc9429) )
	ROM_LOAD16_WORD_SWAP( "hs2.09",   0x300000, 0x80000, CRC(3fc638a8) SHA1(2a42877b26c8abc437da46225701f0bba6e40058) )
	ROM_LOAD16_WORD_SWAP( "hs2.10",   0x380000, 0x80000, CRC(20d0f9e4) SHA1(80a5eeef9472e327b0d4ee26434bad109a9434ea) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "hs2.13m",   0x0000000, 0x800000, CRC(a6ecab17) SHA1(6749a4c8dc81f4b10f910c31c82cf6674e2a44eb) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "hs2.15m",   0x0000002, 0x800000, CRC(10a0ae4d) SHA1(701b4900fbc8bef20efa1a706891c8df4bf14641) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "hs2.17m",   0x0000004, 0x800000, CRC(adfa7726) SHA1(8d36ec125a8c91abfe5213893d794f8bc11c8acd) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "hs2.19m",   0x0000006, 0x800000, CRC(bb3ae322) SHA1(ecd289d7a0fe365fdd7c5527cb17796002beb553) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "hs2.01",   0x00000, 0x08000, CRC(c1a13786) SHA1(c7392c7efb15ea4042e75bd9007e974293d8935d) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "hs2.02",   0x28000, 0x20000, CRC(2d8794aa) SHA1(c634affdc2568020cce6af97b4fa79925d9943f3) )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "hs2.11m",   0x000000, 0x800000, CRC(0e15c359) SHA1(176108b0d76d821a849324680aba0cd04b5016c1) )
ROM_END

ROM_START( hsf2j )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "hs2j.03",  0x000000, 0x80000, CRC(00738f73) SHA1(99947a621f21a88dc4c425d9bfbc30b3c5f76ecd) )
	ROM_LOAD16_WORD_SWAP( "hs2j.04",  0x080000, 0x80000, CRC(40072c4a) SHA1(85b95bd3c907b4276a31777e092b8c40d3763257) )
	ROM_LOAD16_WORD_SWAP( "hs2.05",   0x100000, 0x80000, CRC(dde34a35) SHA1(f5be2d2916db6e86e0886d61d55bddf138273ebc) )
	ROM_LOAD16_WORD_SWAP( "hs2.06",   0x180000, 0x80000, CRC(f4e56dda) SHA1(c6490707c2a416ab88612c2d73abbe5853d8cb92) )
	ROM_LOAD16_WORD_SWAP( "hs2.07",   0x200000, 0x80000, CRC(ee4420fc) SHA1(06cf76660b0c794d2460c52d9fe8334fff51e9de) )
	ROM_LOAD16_WORD_SWAP( "hs2.08",   0x280000, 0x80000, CRC(c9441533) SHA1(bf178fac1f060fcce3ff9118333c8517dadc9429) )
	ROM_LOAD16_WORD_SWAP( "hs2.09",   0x300000, 0x80000, CRC(3fc638a8) SHA1(2a42877b26c8abc437da46225701f0bba6e40058) )
	ROM_LOAD16_WORD_SWAP( "hs2.10",   0x380000, 0x80000, CRC(20d0f9e4) SHA1(80a5eeef9472e327b0d4ee26434bad109a9434ea) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "hs2.13m",   0x0000000, 0x800000, CRC(a6ecab17) SHA1(6749a4c8dc81f4b10f910c31c82cf6674e2a44eb) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "hs2.15m",   0x0000002, 0x800000, CRC(10a0ae4d) SHA1(701b4900fbc8bef20efa1a706891c8df4bf14641) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "hs2.17m",   0x0000004, 0x800000, CRC(adfa7726) SHA1(8d36ec125a8c91abfe5213893d794f8bc11c8acd) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "hs2.19m",   0x0000006, 0x800000, CRC(bb3ae322) SHA1(ecd289d7a0fe365fdd7c5527cb17796002beb553) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "hs2.01",   0x00000, 0x08000, CRC(c1a13786) SHA1(c7392c7efb15ea4042e75bd9007e974293d8935d) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "hs2.02",   0x28000, 0x20000, CRC(2d8794aa) SHA1(c634affdc2568020cce6af97b4fa79925d9943f3) )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "hs2.11m",   0x000000, 0x800000, CRC(0e15c359) SHA1(176108b0d76d821a849324680aba0cd04b5016c1) )
ROM_END


ROM_START( jyangoku )
	ROM_REGION(CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "majj.03", 0x000000, 0x80000, CRC(4614a3b2) SHA1(f7226006feafaf561046ae7fce18bf62289d41df) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROMX_LOAD( "maj1_d.simm1",   0x0000000, 0x200000, CRC(ba0fe27b) SHA1(60a4fdee8da663777af1e126a1aa6308c9d9a5a9) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "maj1_c.simm1",   0x0000001, 0x200000, CRC(2cd141bf) SHA1(57ec73ea24d594fc1e4d2d194a3c548a7043666e) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "maj1_b.simm1",   0x0000002, 0x200000, CRC(e29e4c26) SHA1(51e99536f40481c4c208695354e90fb3fe9416d5) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "maj1_a.simm1",   0x0000003, 0x200000, CRC(7f68b88a) SHA1(944bf34dc998dffe39b25c3e9fcec17ad421ce81) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "maj3_d.simm3",   0x0000004, 0x200000, CRC(3aaeb90b) SHA1(d426d5c7ae5ca99321ec1280abdd1fdfe4882829) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "maj3_c.simm3",   0x0000005, 0x200000, CRC(97894cea) SHA1(a501cd80e6da75409e3381d66bd0a13e021e89f3) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "maj3_b.simm3",   0x0000006, 0x200000, CRC(ec737d9d) SHA1(cfff42cc24ac011fab2670dec42cab16f4e0d84d) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "maj3_a.simm3",   0x0000007, 0x200000, CRC(c23b6f22) SHA1(fb3120ea28c67ecb7c4a2b61a64feb62c033ef68) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm

	ROM_REGION(QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "maj.01",  0x00000, 0x08000, CRC(1fe8c213) SHA1(e0045566337851d8261ed65d5bea483f09ae96b4) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_BYTE( "maj5_a.simm5",   0x000000, 0x200000, CRC(5ad9ee53) SHA1(acfb7ec137209409c1a439ebc62d8fa3f87e7012) ) // ROM on a simm
	ROM_LOAD16_BYTE( "maj5_b.simm5",   0x000001, 0x200000, CRC(efb3dbfb) SHA1(3b32b9890f79805b6a2e1ec63f2cadaca14cf11a) ) // ROM on a simm
ROM_END

ROM_START( megaman2 )
	ROM_REGION(CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "rm2u.03", 0x000000, 0x80000, CRC(8ffc2cd1) SHA1(919ef08311008288b31ed42fb13172580d50433a) )
	ROM_LOAD16_WORD_SWAP( "rm2u.04", 0x080000, 0x80000, CRC(bb30083a) SHA1(466b818a01ad367a8df6df8661f616f5a0236714) )
	ROM_LOAD16_WORD_SWAP( "rm2.05",  0x100000, 0x80000, CRC(02ee9efc) SHA1(1b80c40389b51a03b930051f232630616c12e6c5) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_FILL(              0x000000, 0x800000, 0 )
	ROMX_LOAD( "rm2.14m",  0x800000, 0x200000, CRC(9b1f00b4) SHA1(c1c5c2d9d00121425ae6598444d704f420ef4eef) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rm2.16m",  0x800002, 0x200000, CRC(c2bb0c24) SHA1(38724c49d9db49765a4ed9bc2dc8f57cec45ec7c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rm2.18m",  0x800004, 0x200000, CRC(12257251) SHA1(20cb58afda0e6200991277817485340a6a41ae2b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rm2.20m",  0x800006, 0x200000, CRC(f9b6e786) SHA1(aeb4acff7208e66a35198143fd2478039fdaa3a6) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION(QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "rm2.01a",  0x00000, 0x08000, CRC(d18e7859) SHA1(0939fac70042d0b4db5c2fdcef1f79b95febd45e) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "rm2.02",   0x28000, 0x20000, CRC(c463ece0) SHA1(5c3e41eb61610b3f8c431206f6672907e3a0bdb0) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "rm2.11m",   0x000000, 0x200000, CRC(2106174d) SHA1(0a35d9ca8ebcad74904b20648d5320f839d6377e) )
	ROM_LOAD16_WORD_SWAP( "rm2.12m",   0x200000, 0x200000, CRC(546c1636) SHA1(f96b172ab899f2c6ee17a5dd1fb61af9432e3cd2) )
ROM_END

ROM_START( megaman2a )
	ROM_REGION(CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "rm2a.03", 0x000000, 0x80000, CRC(2b330ca7) SHA1(afa86ef73f5660600d18ff221ed135c026042e05) )
	ROM_LOAD16_WORD_SWAP( "rm2a.04", 0x080000, 0x80000, CRC(8b47942b) SHA1(160574a38e89d31b975c56264f3f5a7a68ce760c) )
	ROM_LOAD16_WORD_SWAP( "rm2.05",  0x100000, 0x80000, CRC(02ee9efc) SHA1(1b80c40389b51a03b930051f232630616c12e6c5) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_FILL(              0x000000, 0x800000, 0 )
	ROMX_LOAD( "rm2.14m",  0x800000, 0x200000, CRC(9b1f00b4) SHA1(c1c5c2d9d00121425ae6598444d704f420ef4eef) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rm2.16m",  0x800002, 0x200000, CRC(c2bb0c24) SHA1(38724c49d9db49765a4ed9bc2dc8f57cec45ec7c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rm2.18m",  0x800004, 0x200000, CRC(12257251) SHA1(20cb58afda0e6200991277817485340a6a41ae2b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rm2.20m",  0x800006, 0x200000, CRC(f9b6e786) SHA1(aeb4acff7208e66a35198143fd2478039fdaa3a6) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION(QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "rm2.01a",  0x00000, 0x08000, CRC(d18e7859) SHA1(0939fac70042d0b4db5c2fdcef1f79b95febd45e) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "rm2.02",   0x28000, 0x20000, CRC(c463ece0) SHA1(5c3e41eb61610b3f8c431206f6672907e3a0bdb0) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "rm2.11m",   0x000000, 0x200000, CRC(2106174d) SHA1(0a35d9ca8ebcad74904b20648d5320f839d6377e) )
	ROM_LOAD16_WORD_SWAP( "rm2.12m",   0x200000, 0x200000, CRC(546c1636) SHA1(f96b172ab899f2c6ee17a5dd1fb61af9432e3cd2) )
ROM_END

ROM_START( rockman2j )
	ROM_REGION(CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "rm2j.03", 0x000000, 0x80000, CRC(dbaa1437) SHA1(849572090bdbde7d9f191959f4b6ad26f46811f4) )
	ROM_LOAD16_WORD_SWAP( "rm2j.04", 0x080000, 0x80000, CRC(cf5ba612) SHA1(f0b56db8df7ad676e00325c97cf16791f409e35a) )
	ROM_LOAD16_WORD_SWAP( "rm2.05",  0x100000, 0x80000, CRC(02ee9efc) SHA1(1b80c40389b51a03b930051f232630616c12e6c5) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_FILL(              0x000000, 0x800000, 0 )
	ROMX_LOAD( "rm2.14m",  0x800000, 0x200000, CRC(9b1f00b4) SHA1(c1c5c2d9d00121425ae6598444d704f420ef4eef) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rm2.16m",  0x800002, 0x200000, CRC(c2bb0c24) SHA1(38724c49d9db49765a4ed9bc2dc8f57cec45ec7c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rm2.18m",  0x800004, 0x200000, CRC(12257251) SHA1(20cb58afda0e6200991277817485340a6a41ae2b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rm2.20m",  0x800006, 0x200000, CRC(f9b6e786) SHA1(aeb4acff7208e66a35198143fd2478039fdaa3a6) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION(QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "rm2.01a",  0x00000, 0x08000, CRC(d18e7859) SHA1(0939fac70042d0b4db5c2fdcef1f79b95febd45e) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "rm2.02",   0x28000, 0x20000, CRC(c463ece0) SHA1(5c3e41eb61610b3f8c431206f6672907e3a0bdb0) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "rm2.11m",   0x000000, 0x200000, CRC(2106174d) SHA1(0a35d9ca8ebcad74904b20648d5320f839d6377e) )
	ROM_LOAD16_WORD_SWAP( "rm2.12m",   0x200000, 0x200000, CRC(546c1636) SHA1(f96b172ab899f2c6ee17a5dd1fb61af9432e3cd2) )
ROM_END

ROM_START( megaman2h )
	ROM_REGION(CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "rm2h.03", 0x000000, 0x80000, CRC(bb180378) SHA1(ecf5e9907500139073e3b1b4e384039957dca354) )
	ROM_LOAD16_WORD_SWAP( "rm2h.04", 0x080000, 0x80000, CRC(205ffcd6) SHA1(d1e3a164f4fd4f372ad2f3b4e2026d6487395503) )
	ROM_LOAD16_WORD_SWAP( "rm2.05",  0x100000, 0x80000, CRC(02ee9efc) SHA1(1b80c40389b51a03b930051f232630616c12e6c5) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_FILL(              0x000000, 0x800000, 0 )
	ROMX_LOAD( "rm2.14m",  0x800000, 0x200000, CRC(9b1f00b4) SHA1(c1c5c2d9d00121425ae6598444d704f420ef4eef) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rm2.16m",  0x800002, 0x200000, CRC(c2bb0c24) SHA1(38724c49d9db49765a4ed9bc2dc8f57cec45ec7c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rm2.18m",  0x800004, 0x200000, CRC(12257251) SHA1(20cb58afda0e6200991277817485340a6a41ae2b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rm2.20m",  0x800006, 0x200000, CRC(f9b6e786) SHA1(aeb4acff7208e66a35198143fd2478039fdaa3a6) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION(QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "rm2.01a",  0x00000, 0x08000, CRC(d18e7859) SHA1(0939fac70042d0b4db5c2fdcef1f79b95febd45e) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "rm2.02",   0x28000, 0x20000, CRC(c463ece0) SHA1(5c3e41eb61610b3f8c431206f6672907e3a0bdb0) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "rm2.11m",   0x000000, 0x200000, CRC(2106174d) SHA1(0a35d9ca8ebcad74904b20648d5320f839d6377e) )
	ROM_LOAD16_WORD_SWAP( "rm2.12m",   0x200000, 0x200000, CRC(546c1636) SHA1(f96b172ab899f2c6ee17a5dd1fb61af9432e3cd2) )
ROM_END

/*

Gigaman 2 - 2004 Chinese rebuild Bootleg

Just dumped the program roms. Other 3 are soldered and are MX26L6420MC-90
Probably a rebuild for chinese market
Copyrighted J-TECH 2004 on game :)


CPU : Motorola 68000 16 mhz
video : Actel A54SX16A-F
Sound : Atmel AT89C4051-24PI + M6295 (noted AD-65)

*/

ROM_START( gigamn2 )
	ROM_REGION(CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "prog.bin", 0x000000, 0x400000, CRC(2eaa5e10) SHA1(79f9a137bf5b3317579c548f346c1dc1cccdb771) )

	ROM_REGION(0x10000, "mcu", 0 )      /* sound MCU code */
	ROM_LOAD( "89c4051.bin", 0x000000, 0x10000, NO_DUMP )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_FILL(              0x000000, 0x800000, 0 )
	/* you can use the original CPS2 gfx roms, but the 'GIGA part of GIGAMAN2 is missing, on real HW it isn't, roms are different */
//  ROMX_LOAD( "rm2.14m",  0x800000, 0x200000, CRC(9b1f00b4) SHA1(c1c5c2d9d00121425ae6598444d704f420ef4eef) , ROM_GROUPWORD | ROM_SKIP(6) )
//  ROMX_LOAD( "rm2.16m",  0x800002, 0x200000, CRC(c2bb0c24) SHA1(38724c49d9db49765a4ed9bc2dc8f57cec45ec7c) , ROM_GROUPWORD | ROM_SKIP(6) )
//  ROMX_LOAD( "rm2.18m",  0x800004, 0x200000, CRC(12257251) SHA1(20cb58afda0e6200991277817485340a6a41ae2b) , ROM_GROUPWORD | ROM_SKIP(6) )
//  ROMX_LOAD( "rm2.20m",  0x800006, 0x200000, CRC(f9b6e786) SHA1(aeb4acff7208e66a35198143fd2478039fdaa3a6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROM_LOAD( "gfx1.bin", 0x800000, 0x400000, NO_DUMP )
	ROM_LOAD( "gfx2.bin", 0xc00000, 0x400000, NO_DUMP )

	ROM_REGION( 0x400000, "oki", 0 ) /* QSound samples */
	/* No Qsound, OKI instead.. */
	ROM_LOAD( "oki.bin", 0x000000, 0x400000, NO_DUMP )
ROM_END

ROM_START( mmatrix )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "mmxu.03", 0x000000, 0x80000, CRC(ab65b599) SHA1(d4c35f5a6cf0b37a35c466f0e347a660b2e0b21b) )
	ROM_LOAD16_WORD_SWAP( "mmxu.04", 0x080000, 0x80000, CRC(0135fc6c) SHA1(e40c8fa51dcb300b3ee72dc7de137e0b39dea490) )
	ROM_LOAD16_WORD_SWAP( "mmxu.05", 0x100000, 0x80000, CRC(f1fd2b84) SHA1(d34816eff4af98009f94f5dd14097b39426e0468) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "mmx.13m",   0x0000000, 0x400000, CRC(04748718) SHA1(d2e84d9dcc779c08469d815ccd709f30705954b8) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mmx.15m",   0x0000002, 0x400000, CRC(38074f44) SHA1(2002c4862c156b314bc4f3372b713c48e0667ec3) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mmx.17m",   0x0000004, 0x400000, CRC(e4635e35) SHA1(48ef7a82df83b981ddd6138c241ca129ab770e8e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mmx.19m",   0x0000006, 0x400000, CRC(4400a3f2) SHA1(d0aa805ccbb153896e5983da1c398d1df4f40371) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mmx.14m",   0x1000000, 0x400000, CRC(d52bf491) SHA1(2398895cfdcf86fc485472e33df2cc446539e977) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mmx.16m",   0x1000002, 0x400000, CRC(23f70780) SHA1(691ee8964815b0ce54704e7feb59ca79b634f26d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mmx.18m",   0x1000004, 0x400000, CRC(2562c9d5) SHA1(e7defc3d33db632c4035ae069f2f2332c58afaf5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mmx.20m",   0x1000006, 0x400000, CRC(583a9687) SHA1(1d0b08b1e88509245db3c2090f0201938fd750b4) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "mmx.01",   0x00000, 0x08000, CRC(c57e8171) SHA1(dedb92af1910d38727f816e6f507d25148f31b74) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "mmx.11m",   0x000000, 0x400000, CRC(4180b39f) SHA1(cabb1c358eae1bb6cfed07f5b92e4acd38650667) )
	ROM_LOAD16_WORD_SWAP( "mmx.12m",   0x400000, 0x400000, CRC(95e22a59) SHA1(b3431d170c0a1a0d826ad0af21300b9180e3f114) )
ROM_END

ROM_START( mmatrixj )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "mmxj.03", 0x000000, 0x80000, CRC(1d5de213) SHA1(2d7ad9cb50540a14aa0ac564d4ab84a3779d595c) )
	ROM_LOAD16_WORD_SWAP( "mmxj.04", 0x080000, 0x80000, CRC(d943a339) SHA1(ae3d217b35f92fc727bda3b14f13f3658dab3dd8) )
	ROM_LOAD16_WORD_SWAP( "mmxj.05", 0x100000, 0x80000, CRC(0c8b4abb) SHA1(c136186b9f57d68c0b36f5a4273347f696a227c0) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "mmx.13m",   0x0000000, 0x400000, CRC(04748718) SHA1(d2e84d9dcc779c08469d815ccd709f30705954b8) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mmx.15m",   0x0000002, 0x400000, CRC(38074f44) SHA1(2002c4862c156b314bc4f3372b713c48e0667ec3) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mmx.17m",   0x0000004, 0x400000, CRC(e4635e35) SHA1(48ef7a82df83b981ddd6138c241ca129ab770e8e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mmx.19m",   0x0000006, 0x400000, CRC(4400a3f2) SHA1(d0aa805ccbb153896e5983da1c398d1df4f40371) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mmx.14m",   0x1000000, 0x400000, CRC(d52bf491) SHA1(2398895cfdcf86fc485472e33df2cc446539e977) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mmx.16m",   0x1000002, 0x400000, CRC(23f70780) SHA1(691ee8964815b0ce54704e7feb59ca79b634f26d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mmx.18m",   0x1000004, 0x400000, CRC(2562c9d5) SHA1(e7defc3d33db632c4035ae069f2f2332c58afaf5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mmx.20m",   0x1000006, 0x400000, CRC(583a9687) SHA1(1d0b08b1e88509245db3c2090f0201938fd750b4) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "mmx.01",   0x00000, 0x08000, CRC(c57e8171) SHA1(dedb92af1910d38727f816e6f507d25148f31b74) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "mmx.11m",   0x000000, 0x400000, CRC(4180b39f) SHA1(cabb1c358eae1bb6cfed07f5b92e4acd38650667) )
	ROM_LOAD16_WORD_SWAP( "mmx.12m",   0x400000, 0x400000, CRC(95e22a59) SHA1(b3431d170c0a1a0d826ad0af21300b9180e3f114) )
ROM_END

ROM_START( msh )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "mshe.03e", 0x000000, 0x80000, CRC(bd951414) SHA1(5585bdd1484dc18c7630d689f60d91c068aafc97) )
	ROM_LOAD16_WORD_SWAP( "mshe.04e", 0x080000, 0x80000, CRC(19dd42f2) SHA1(48bd3e4d2d7e9e07275bd9c00530719deb100090) )
	ROM_LOAD16_WORD_SWAP( "msh.05",   0x100000, 0x80000, CRC(6a091b9e) SHA1(7fa54e69e1a1ca348cb08d892d55023e9a3ff4cb) )
	ROM_LOAD16_WORD_SWAP( "msh.06b",  0x180000, 0x80000, CRC(803e3fa4) SHA1(0acdeda65002521bf24130cbf06f9faa1dcef9e5) )
	ROM_LOAD16_WORD_SWAP( "msh.07a",  0x200000, 0x80000, CRC(c45f8e27) SHA1(4d28e0782c31ce56e728ac6ef5edd10437f00637) )
	ROM_LOAD16_WORD_SWAP( "msh.08a",  0x280000, 0x80000, CRC(9ca6f12c) SHA1(26ad682667b983b805e1f577426e5fca8ee3c82b) )
	ROM_LOAD16_WORD_SWAP( "msh.09a",  0x300000, 0x80000, CRC(82ec27af) SHA1(caf76268063ba91d28e8af684d60c2d71f29b9b9) )
	ROM_LOAD16_WORD_SWAP( "msh.10b",  0x380000, 0x80000, CRC(8d931196) SHA1(983e62efcdb4c8db6bce6acf4f86acb9447b565d) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "msh.13m",   0x0000000, 0x400000, CRC(09d14566) SHA1(c96463654043f22da5e844c6da17aa9273dc3439) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.15m",   0x0000002, 0x400000, CRC(ee962057) SHA1(24e359accb5f71a5863d7bad4088719fa547f88c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.17m",   0x0000004, 0x400000, CRC(604ece14) SHA1(880fb62b33ba4cceb38635e4ec056fac11a3c70f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.19m",   0x0000006, 0x400000, CRC(94a731e8) SHA1(1e784a3412e7361e3001494e1daf840ef8c20449) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.14m",   0x1000000, 0x400000, CRC(4197973e) SHA1(93aeea1a480b5f452c8a40ae3fff956796b859fa) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.16m",   0x1000002, 0x400000, CRC(438da4a0) SHA1(ca93b14c3a570f9dd582efbb3f0536a92e535042) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.18m",   0x1000004, 0x400000, CRC(4db92d94) SHA1(f1b25ccc0627139ad5b287a8f2ab3b4a2fb8b8e4) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.20m",   0x1000006, 0x400000, CRC(a2b0c6c0) SHA1(71016c01c1a706b73cf5b9ac7e384a030c6cf08d) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "msh.01",   0x00000, 0x08000, CRC(c976e6f9) SHA1(281025e5aaf97c0aeddc8bd0f737d092daadad9e) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "msh.02",   0x28000, 0x20000, CRC(ce67d0d9) SHA1(324226597cc5a11603f04085fef7715a314ecc05) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "msh.11m",   0x000000, 0x200000, CRC(37ac6d30) SHA1(ec67421fbf4a08a686e76792cb35e9cbf04d022d) )
	ROM_LOAD16_WORD_SWAP( "msh.12m",   0x200000, 0x200000, CRC(de092570) SHA1(a03d0df901f6ea79685eaed67db65bee14ec29c6) )
ROM_END

ROM_START( mshu )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "mshu.03", 0x000000, 0x80000, CRC(d2805bdd) SHA1(a6f78c31a82168bb5f7d614dcebbeab8231e2d75) )
	ROM_LOAD16_WORD_SWAP( "mshu.04", 0x080000, 0x80000, CRC(743f96ff) SHA1(abb82359bb68966028ea33e94996803599f3e273) )
	ROM_LOAD16_WORD_SWAP( "msh.05",  0x100000, 0x80000, CRC(6a091b9e) SHA1(7fa54e69e1a1ca348cb08d892d55023e9a3ff4cb) )
	ROM_LOAD16_WORD_SWAP( "msh.06b", 0x180000, 0x80000, CRC(803e3fa4) SHA1(0acdeda65002521bf24130cbf06f9faa1dcef9e5) )
	ROM_LOAD16_WORD_SWAP( "msh.07a", 0x200000, 0x80000, CRC(c45f8e27) SHA1(4d28e0782c31ce56e728ac6ef5edd10437f00637) )
	ROM_LOAD16_WORD_SWAP( "msh.08a", 0x280000, 0x80000, CRC(9ca6f12c) SHA1(26ad682667b983b805e1f577426e5fca8ee3c82b) )
	ROM_LOAD16_WORD_SWAP( "msh.09a", 0x300000, 0x80000, CRC(82ec27af) SHA1(caf76268063ba91d28e8af684d60c2d71f29b9b9) )
	ROM_LOAD16_WORD_SWAP( "msh.10b", 0x380000, 0x80000, CRC(8d931196) SHA1(983e62efcdb4c8db6bce6acf4f86acb9447b565d) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "msh.13m",   0x0000000, 0x400000, CRC(09d14566) SHA1(c96463654043f22da5e844c6da17aa9273dc3439) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.15m",   0x0000002, 0x400000, CRC(ee962057) SHA1(24e359accb5f71a5863d7bad4088719fa547f88c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.17m",   0x0000004, 0x400000, CRC(604ece14) SHA1(880fb62b33ba4cceb38635e4ec056fac11a3c70f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.19m",   0x0000006, 0x400000, CRC(94a731e8) SHA1(1e784a3412e7361e3001494e1daf840ef8c20449) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.14m",   0x1000000, 0x400000, CRC(4197973e) SHA1(93aeea1a480b5f452c8a40ae3fff956796b859fa) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.16m",   0x1000002, 0x400000, CRC(438da4a0) SHA1(ca93b14c3a570f9dd582efbb3f0536a92e535042) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.18m",   0x1000004, 0x400000, CRC(4db92d94) SHA1(f1b25ccc0627139ad5b287a8f2ab3b4a2fb8b8e4) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.20m",   0x1000006, 0x400000, CRC(a2b0c6c0) SHA1(71016c01c1a706b73cf5b9ac7e384a030c6cf08d) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "msh.01",   0x00000, 0x08000, CRC(c976e6f9) SHA1(281025e5aaf97c0aeddc8bd0f737d092daadad9e) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "msh.02",   0x28000, 0x20000, CRC(ce67d0d9) SHA1(324226597cc5a11603f04085fef7715a314ecc05) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "msh.11m",   0x000000, 0x200000, CRC(37ac6d30) SHA1(ec67421fbf4a08a686e76792cb35e9cbf04d022d) )
	ROM_LOAD16_WORD_SWAP( "msh.12m",   0x200000, 0x200000, CRC(de092570) SHA1(a03d0df901f6ea79685eaed67db65bee14ec29c6) )
ROM_END

ROM_START( mshj )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "mshj.03g", 0x000000, 0x80000, CRC(261f4091) SHA1(f4509780768e3601720d0d78c8a9824d410d59da) )
	ROM_LOAD16_WORD_SWAP( "mshj.04g", 0x080000, 0x80000, CRC(61d791c6) SHA1(9f883bcc48058a99c4ba653d0855c58c5d081243) )
	ROM_LOAD16_WORD_SWAP( "msh.05a",  0x100000, 0x80000, CRC(f37539e6) SHA1(770febc25ca5615b6c2023727edab3c68b15b2c4) )
	ROM_LOAD16_WORD_SWAP( "msh.06b",  0x180000, 0x80000, CRC(803e3fa4) SHA1(0acdeda65002521bf24130cbf06f9faa1dcef9e5) )
	ROM_LOAD16_WORD_SWAP( "msh.07a",  0x200000, 0x80000, CRC(c45f8e27) SHA1(4d28e0782c31ce56e728ac6ef5edd10437f00637) )
	ROM_LOAD16_WORD_SWAP( "msh.08a",  0x280000, 0x80000, CRC(9ca6f12c) SHA1(26ad682667b983b805e1f577426e5fca8ee3c82b) )
	ROM_LOAD16_WORD_SWAP( "msh.09a",  0x300000, 0x80000, CRC(82ec27af) SHA1(caf76268063ba91d28e8af684d60c2d71f29b9b9) )
	ROM_LOAD16_WORD_SWAP( "msh.10b",  0x380000, 0x80000, CRC(8d931196) SHA1(983e62efcdb4c8db6bce6acf4f86acb9447b565d) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "msh.13m",   0x0000000, 0x400000, CRC(09d14566) SHA1(c96463654043f22da5e844c6da17aa9273dc3439) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.15m",   0x0000002, 0x400000, CRC(ee962057) SHA1(24e359accb5f71a5863d7bad4088719fa547f88c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.17m",   0x0000004, 0x400000, CRC(604ece14) SHA1(880fb62b33ba4cceb38635e4ec056fac11a3c70f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.19m",   0x0000006, 0x400000, CRC(94a731e8) SHA1(1e784a3412e7361e3001494e1daf840ef8c20449) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.14m",   0x1000000, 0x400000, CRC(4197973e) SHA1(93aeea1a480b5f452c8a40ae3fff956796b859fa) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.16m",   0x1000002, 0x400000, CRC(438da4a0) SHA1(ca93b14c3a570f9dd582efbb3f0536a92e535042) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.18m",   0x1000004, 0x400000, CRC(4db92d94) SHA1(f1b25ccc0627139ad5b287a8f2ab3b4a2fb8b8e4) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.20m",   0x1000006, 0x400000, CRC(a2b0c6c0) SHA1(71016c01c1a706b73cf5b9ac7e384a030c6cf08d) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "msh.01",   0x00000, 0x08000, CRC(c976e6f9) SHA1(281025e5aaf97c0aeddc8bd0f737d092daadad9e) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "msh.02",   0x28000, 0x20000, CRC(ce67d0d9) SHA1(324226597cc5a11603f04085fef7715a314ecc05) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "msh.11m",   0x000000, 0x200000, CRC(37ac6d30) SHA1(ec67421fbf4a08a686e76792cb35e9cbf04d022d) )
	ROM_LOAD16_WORD_SWAP( "msh.12m",   0x200000, 0x200000, CRC(de092570) SHA1(a03d0df901f6ea79685eaed67db65bee14ec29c6) )
ROM_END

ROM_START( mshjr1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "mshj.03f", 0x000000, 0x80000, CRC(ff172fd2) SHA1(2dd507e3fcf1a30fde1e6ce63d4233a67e7bfc9e) )
	ROM_LOAD16_WORD_SWAP( "mshj.04f", 0x080000, 0x80000, CRC(ebbb205a) SHA1(0b110ea4c71bdab819b72e6f9736368575e4cccf) )
	ROM_LOAD16_WORD_SWAP( "msh.05",   0x100000, 0x80000, CRC(6a091b9e) SHA1(7fa54e69e1a1ca348cb08d892d55023e9a3ff4cb) )
	ROM_LOAD16_WORD_SWAP( "msh.06b",  0x180000, 0x80000, CRC(803e3fa4) SHA1(0acdeda65002521bf24130cbf06f9faa1dcef9e5) )
	ROM_LOAD16_WORD_SWAP( "msh.07a",  0x200000, 0x80000, CRC(c45f8e27) SHA1(4d28e0782c31ce56e728ac6ef5edd10437f00637) )
	ROM_LOAD16_WORD_SWAP( "msh.08a",  0x280000, 0x80000, CRC(9ca6f12c) SHA1(26ad682667b983b805e1f577426e5fca8ee3c82b) )
	ROM_LOAD16_WORD_SWAP( "msh.09a",  0x300000, 0x80000, CRC(82ec27af) SHA1(caf76268063ba91d28e8af684d60c2d71f29b9b9) )
	ROM_LOAD16_WORD_SWAP( "msh.10b",  0x380000, 0x80000, CRC(8d931196) SHA1(983e62efcdb4c8db6bce6acf4f86acb9447b565d) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "msh.13m",   0x0000000, 0x400000, CRC(09d14566) SHA1(c96463654043f22da5e844c6da17aa9273dc3439) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.15m",   0x0000002, 0x400000, CRC(ee962057) SHA1(24e359accb5f71a5863d7bad4088719fa547f88c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.17m",   0x0000004, 0x400000, CRC(604ece14) SHA1(880fb62b33ba4cceb38635e4ec056fac11a3c70f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.19m",   0x0000006, 0x400000, CRC(94a731e8) SHA1(1e784a3412e7361e3001494e1daf840ef8c20449) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.14m",   0x1000000, 0x400000, CRC(4197973e) SHA1(93aeea1a480b5f452c8a40ae3fff956796b859fa) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.16m",   0x1000002, 0x400000, CRC(438da4a0) SHA1(ca93b14c3a570f9dd582efbb3f0536a92e535042) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.18m",   0x1000004, 0x400000, CRC(4db92d94) SHA1(f1b25ccc0627139ad5b287a8f2ab3b4a2fb8b8e4) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.20m",   0x1000006, 0x400000, CRC(a2b0c6c0) SHA1(71016c01c1a706b73cf5b9ac7e384a030c6cf08d) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "msh.01",   0x00000, 0x08000, CRC(c976e6f9) SHA1(281025e5aaf97c0aeddc8bd0f737d092daadad9e) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "msh.02",   0x28000, 0x20000, CRC(ce67d0d9) SHA1(324226597cc5a11603f04085fef7715a314ecc05) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "msh.11m",   0x000000, 0x200000, CRC(37ac6d30) SHA1(ec67421fbf4a08a686e76792cb35e9cbf04d022d) )
	ROM_LOAD16_WORD_SWAP( "msh.12m",   0x200000, 0x200000, CRC(de092570) SHA1(a03d0df901f6ea79685eaed67db65bee14ec29c6) )
ROM_END

ROM_START( msha )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "msha.03e", 0x000000, 0x80000, CRC(ec84ec44) SHA1(4d434df6cf5c961f0dbba352d1353db0a8f353dc) )
	ROM_LOAD16_WORD_SWAP( "msha.04e", 0x080000, 0x80000, CRC(098b8503) SHA1(4cc74754796d5e41f13bf5cd4e8868b0d0c7852c) )
	ROM_LOAD16_WORD_SWAP( "msh.05",   0x100000, 0x80000, CRC(6a091b9e) SHA1(7fa54e69e1a1ca348cb08d892d55023e9a3ff4cb) )
	ROM_LOAD16_WORD_SWAP( "msh.06b",  0x180000, 0x80000, CRC(803e3fa4) SHA1(0acdeda65002521bf24130cbf06f9faa1dcef9e5) )
	ROM_LOAD16_WORD_SWAP( "msh.07a",  0x200000, 0x80000, CRC(c45f8e27) SHA1(4d28e0782c31ce56e728ac6ef5edd10437f00637) )
	ROM_LOAD16_WORD_SWAP( "msh.08a",  0x280000, 0x80000, CRC(9ca6f12c) SHA1(26ad682667b983b805e1f577426e5fca8ee3c82b) )
	ROM_LOAD16_WORD_SWAP( "msh.09a",  0x300000, 0x80000, CRC(82ec27af) SHA1(caf76268063ba91d28e8af684d60c2d71f29b9b9) )
	ROM_LOAD16_WORD_SWAP( "msh.10b",  0x380000, 0x80000, CRC(8d931196) SHA1(983e62efcdb4c8db6bce6acf4f86acb9447b565d) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "msh.13m",   0x0000000, 0x400000, CRC(09d14566) SHA1(c96463654043f22da5e844c6da17aa9273dc3439) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.15m",   0x0000002, 0x400000, CRC(ee962057) SHA1(24e359accb5f71a5863d7bad4088719fa547f88c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.17m",   0x0000004, 0x400000, CRC(604ece14) SHA1(880fb62b33ba4cceb38635e4ec056fac11a3c70f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.19m",   0x0000006, 0x400000, CRC(94a731e8) SHA1(1e784a3412e7361e3001494e1daf840ef8c20449) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.14m",   0x1000000, 0x400000, CRC(4197973e) SHA1(93aeea1a480b5f452c8a40ae3fff956796b859fa) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.16m",   0x1000002, 0x400000, CRC(438da4a0) SHA1(ca93b14c3a570f9dd582efbb3f0536a92e535042) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.18m",   0x1000004, 0x400000, CRC(4db92d94) SHA1(f1b25ccc0627139ad5b287a8f2ab3b4a2fb8b8e4) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.20m",   0x1000006, 0x400000, CRC(a2b0c6c0) SHA1(71016c01c1a706b73cf5b9ac7e384a030c6cf08d) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "msh.01",   0x00000, 0x08000, CRC(c976e6f9) SHA1(281025e5aaf97c0aeddc8bd0f737d092daadad9e) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "msh.02",   0x28000, 0x20000, CRC(ce67d0d9) SHA1(324226597cc5a11603f04085fef7715a314ecc05) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "msh.11m",   0x000000, 0x200000, CRC(37ac6d30) SHA1(ec67421fbf4a08a686e76792cb35e9cbf04d022d) )
	ROM_LOAD16_WORD_SWAP( "msh.12m",   0x200000, 0x200000, CRC(de092570) SHA1(a03d0df901f6ea79685eaed67db65bee14ec29c6) )
ROM_END

ROM_START( mshh )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "mshh.03c", 0x000000, 0x80000, CRC(8d84b0fa) SHA1(e1fd2869abbe4f8736e496f194e23a1ab0526811) )
	ROM_LOAD16_WORD_SWAP( "mshh.04c", 0x080000, 0x80000, CRC(d638f601) SHA1(cbdd9776f71c6ef8d80be23a57cba3529d53a070) )
	ROM_LOAD16_WORD_SWAP( "msh.05a",  0x100000, 0x80000, CRC(f37539e6) SHA1(770febc25ca5615b6c2023727edab3c68b15b2c4) )
	ROM_LOAD16_WORD_SWAP( "msh.06b",  0x180000, 0x80000, CRC(803e3fa4) SHA1(0acdeda65002521bf24130cbf06f9faa1dcef9e5) )
	ROM_LOAD16_WORD_SWAP( "msh.07a",  0x200000, 0x80000, CRC(c45f8e27) SHA1(4d28e0782c31ce56e728ac6ef5edd10437f00637) )
	ROM_LOAD16_WORD_SWAP( "msh.08a",  0x280000, 0x80000, CRC(9ca6f12c) SHA1(26ad682667b983b805e1f577426e5fca8ee3c82b) )
	ROM_LOAD16_WORD_SWAP( "msh.09a",  0x300000, 0x80000, CRC(82ec27af) SHA1(caf76268063ba91d28e8af684d60c2d71f29b9b9) )
	ROM_LOAD16_WORD_SWAP( "msh.10b",  0x380000, 0x80000, CRC(8d931196) SHA1(983e62efcdb4c8db6bce6acf4f86acb9447b565d) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "msh.13m",   0x0000000, 0x400000, CRC(09d14566) SHA1(c96463654043f22da5e844c6da17aa9273dc3439) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.15m",   0x0000002, 0x400000, CRC(ee962057) SHA1(24e359accb5f71a5863d7bad4088719fa547f88c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.17m",   0x0000004, 0x400000, CRC(604ece14) SHA1(880fb62b33ba4cceb38635e4ec056fac11a3c70f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.19m",   0x0000006, 0x400000, CRC(94a731e8) SHA1(1e784a3412e7361e3001494e1daf840ef8c20449) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.14m",   0x1000000, 0x400000, CRC(4197973e) SHA1(93aeea1a480b5f452c8a40ae3fff956796b859fa) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.16m",   0x1000002, 0x400000, CRC(438da4a0) SHA1(ca93b14c3a570f9dd582efbb3f0536a92e535042) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.18m",   0x1000004, 0x400000, CRC(4db92d94) SHA1(f1b25ccc0627139ad5b287a8f2ab3b4a2fb8b8e4) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.20m",   0x1000006, 0x400000, CRC(a2b0c6c0) SHA1(71016c01c1a706b73cf5b9ac7e384a030c6cf08d) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "msh.01",   0x00000, 0x08000, CRC(c976e6f9) SHA1(281025e5aaf97c0aeddc8bd0f737d092daadad9e) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "msh.02",   0x28000, 0x20000, CRC(ce67d0d9) SHA1(324226597cc5a11603f04085fef7715a314ecc05) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "msh.11m",   0x000000, 0x200000, CRC(37ac6d30) SHA1(ec67421fbf4a08a686e76792cb35e9cbf04d022d) )
	ROM_LOAD16_WORD_SWAP( "msh.12m",   0x200000, 0x200000, CRC(de092570) SHA1(a03d0df901f6ea79685eaed67db65bee14ec29c6) )
ROM_END

ROM_START( mshb )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "mshb.03c", 0x000000, 0x80000, CRC(19697f74) SHA1(c3809ecbdb242bdbb57f8d9b029264e9c0ed8a13) )
	ROM_LOAD16_WORD_SWAP( "mshb.04c", 0x080000, 0x80000, CRC(95317a6f) SHA1(143a26e349f21d3a720320bb7010a26f767e5e73) )
	ROM_LOAD16_WORD_SWAP( "msh.05a",  0x100000, 0x80000, CRC(f37539e6) SHA1(770febc25ca5615b6c2023727edab3c68b15b2c4) )
	ROM_LOAD16_WORD_SWAP( "msh.06b",  0x180000, 0x80000, CRC(803e3fa4) SHA1(0acdeda65002521bf24130cbf06f9faa1dcef9e5) )
	ROM_LOAD16_WORD_SWAP( "msh.07a",  0x200000, 0x80000, CRC(c45f8e27) SHA1(4d28e0782c31ce56e728ac6ef5edd10437f00637) )
	ROM_LOAD16_WORD_SWAP( "msh.08a",  0x280000, 0x80000, CRC(9ca6f12c) SHA1(26ad682667b983b805e1f577426e5fca8ee3c82b) )
	ROM_LOAD16_WORD_SWAP( "msh.09a",  0x300000, 0x80000, CRC(82ec27af) SHA1(caf76268063ba91d28e8af684d60c2d71f29b9b9) )
	ROM_LOAD16_WORD_SWAP( "msh.10b",  0x380000, 0x80000, CRC(8d931196) SHA1(983e62efcdb4c8db6bce6acf4f86acb9447b565d) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "msh.13m",   0x0000000, 0x400000, CRC(09d14566) SHA1(c96463654043f22da5e844c6da17aa9273dc3439) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.15m",   0x0000002, 0x400000, CRC(ee962057) SHA1(24e359accb5f71a5863d7bad4088719fa547f88c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.17m",   0x0000004, 0x400000, CRC(604ece14) SHA1(880fb62b33ba4cceb38635e4ec056fac11a3c70f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.19m",   0x0000006, 0x400000, CRC(94a731e8) SHA1(1e784a3412e7361e3001494e1daf840ef8c20449) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.14m",   0x1000000, 0x400000, CRC(4197973e) SHA1(93aeea1a480b5f452c8a40ae3fff956796b859fa) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.16m",   0x1000002, 0x400000, CRC(438da4a0) SHA1(ca93b14c3a570f9dd582efbb3f0536a92e535042) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.18m",   0x1000004, 0x400000, CRC(4db92d94) SHA1(f1b25ccc0627139ad5b287a8f2ab3b4a2fb8b8e4) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.20m",   0x1000006, 0x400000, CRC(a2b0c6c0) SHA1(71016c01c1a706b73cf5b9ac7e384a030c6cf08d) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "msh.01",   0x00000, 0x08000, CRC(c976e6f9) SHA1(281025e5aaf97c0aeddc8bd0f737d092daadad9e) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "msh.02",   0x28000, 0x20000, CRC(ce67d0d9) SHA1(324226597cc5a11603f04085fef7715a314ecc05) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "msh.11m",   0x000000, 0x200000, CRC(37ac6d30) SHA1(ec67421fbf4a08a686e76792cb35e9cbf04d022d) )
	ROM_LOAD16_WORD_SWAP( "msh.12m",   0x200000, 0x200000, CRC(de092570) SHA1(a03d0df901f6ea79685eaed67db65bee14ec29c6) )
ROM_END

ROM_START( mshvsf )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "mvse.03f", 0x000000, 0x80000, CRC(b72dc199) SHA1(61bd581ea4b969298f8a39fe03023b5456cac750) )
	ROM_LOAD16_WORD_SWAP( "mvse.04f", 0x080000, 0x80000, CRC(6ef799f9) SHA1(2d45dbf7bc277b84c6bcd9615ab3b80c42af7781) )
	ROM_LOAD16_WORD_SWAP( "mvs.05a",  0x100000, 0x80000, CRC(1a5de0cb) SHA1(738a27f83704c208d36d73bf766d861ef2d51a89) )
	ROM_LOAD16_WORD_SWAP( "mvs.06a",  0x180000, 0x80000, CRC(959f3030) SHA1(fbbaa915324815246738f3426232e623f039ce26) )
	ROM_LOAD16_WORD_SWAP( "mvs.07b",  0x200000, 0x80000, CRC(7f915bdb) SHA1(683da09c5ba55e31b59aa95a8e13c45dc574ab3c) )
	ROM_LOAD16_WORD_SWAP( "mvs.08a",  0x280000, 0x80000, CRC(c2813884) SHA1(49e5d4bc48f90c8146cb6aafb9240aff0119f1a7) )
	ROM_LOAD16_WORD_SWAP( "mvs.09b",  0x300000, 0x80000, CRC(3ba08818) SHA1(9ab132a3cac55fcccebe6c99b6fb0ba1305f8f6e) )
	ROM_LOAD16_WORD_SWAP( "mvs.10b",  0x380000, 0x80000, CRC(cf0dba98) SHA1(f4c1f8a6e7a79ecc6241d5268b3039f8a09ea516) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "mvs.13m",   0x0000000, 0x400000, CRC(29b05fd9) SHA1(e8fdb1ee5515a560eb4256ae4fd99bb1192e1a87) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.15m",   0x0000002, 0x400000, CRC(faddccf1) SHA1(4ed03ea91883a0413325f57edcc1614120b5922c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.17m",   0x0000004, 0x400000, CRC(97aaf4c7) SHA1(6a054921cc14fe080cb3f62c391f8ae3cc7e8ba9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.19m",   0x0000006, 0x400000, CRC(cb70e915) SHA1(da4d2480d348ac6dfd01256a88f4f3db8357ae46) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.14m",   0x1000000, 0x400000, CRC(b3b1972d) SHA1(0f2c3fb7de014181ee481ec35d0578b2c116c2dc) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.16m",   0x1000002, 0x400000, CRC(08aadb5d) SHA1(3a2c222eca3e7df80ce69951b3db6442312751a4) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.18m",   0x1000004, 0x400000, CRC(c1228b35) SHA1(7afdfb552888c79d0fbb30242b3d917b87fad57a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.20m",   0x1000006, 0x400000, CRC(366cc6c2) SHA1(6f2a789087c8e404c5227b927fa8328c03593243) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "mvs.01",   0x00000, 0x08000, CRC(68252324) SHA1(138ef320ef27956b2ab5591d49a1315b7b0a194c) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "mvs.02",   0x28000, 0x20000, CRC(b34e773d) SHA1(3bcf44bf06c35814cff29d244142db7abe05bd39) )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "mvs.11m",   0x000000, 0x400000, CRC(86219770) SHA1(4e5b68d382a5aa37f8b0b6434c53a2b95f5f9a4d) )
	ROM_LOAD16_WORD_SWAP( "mvs.12m",   0x400000, 0x400000, CRC(f2fd7f68) SHA1(28a30d55d3eaf963006c7cbe7c288099cd3ba536) )
ROM_END

ROM_START( mshvsfu )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "mvsu.03g", 0x000000, 0x80000, CRC(0664ab15) SHA1(939fb1e3c06c33fc212b26ecfceac3180e108e9d) )
	ROM_LOAD16_WORD_SWAP( "mvsu.04g", 0x080000, 0x80000, CRC(97e060ee) SHA1(787924e04508c83ecd4c3a872882d2be9e57eb50) )
	ROM_LOAD16_WORD_SWAP( "mvs.05d",  0x100000, 0x80000, CRC(921fc542) SHA1(b813082a480d42d663c713062892245faabe9101) )
	ROM_LOAD16_WORD_SWAP( "mvs.06a",  0x180000, 0x80000, CRC(959f3030) SHA1(fbbaa915324815246738f3426232e623f039ce26) )
	ROM_LOAD16_WORD_SWAP( "mvs.07b",  0x200000, 0x80000, CRC(7f915bdb) SHA1(683da09c5ba55e31b59aa95a8e13c45dc574ab3c) )
	ROM_LOAD16_WORD_SWAP( "mvs.08a",  0x280000, 0x80000, CRC(c2813884) SHA1(49e5d4bc48f90c8146cb6aafb9240aff0119f1a7) )
	ROM_LOAD16_WORD_SWAP( "mvs.09b",  0x300000, 0x80000, CRC(3ba08818) SHA1(9ab132a3cac55fcccebe6c99b6fb0ba1305f8f6e) )
	ROM_LOAD16_WORD_SWAP( "mvs.10b",  0x380000, 0x80000, CRC(cf0dba98) SHA1(f4c1f8a6e7a79ecc6241d5268b3039f8a09ea516) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "mvs.13m",   0x0000000, 0x400000, CRC(29b05fd9) SHA1(e8fdb1ee5515a560eb4256ae4fd99bb1192e1a87) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.15m",   0x0000002, 0x400000, CRC(faddccf1) SHA1(4ed03ea91883a0413325f57edcc1614120b5922c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.17m",   0x0000004, 0x400000, CRC(97aaf4c7) SHA1(6a054921cc14fe080cb3f62c391f8ae3cc7e8ba9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.19m",   0x0000006, 0x400000, CRC(cb70e915) SHA1(da4d2480d348ac6dfd01256a88f4f3db8357ae46) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.14m",   0x1000000, 0x400000, CRC(b3b1972d) SHA1(0f2c3fb7de014181ee481ec35d0578b2c116c2dc) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.16m",   0x1000002, 0x400000, CRC(08aadb5d) SHA1(3a2c222eca3e7df80ce69951b3db6442312751a4) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.18m",   0x1000004, 0x400000, CRC(c1228b35) SHA1(7afdfb552888c79d0fbb30242b3d917b87fad57a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.20m",   0x1000006, 0x400000, CRC(366cc6c2) SHA1(6f2a789087c8e404c5227b927fa8328c03593243) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "mvs.01",   0x00000, 0x08000, CRC(68252324) SHA1(138ef320ef27956b2ab5591d49a1315b7b0a194c) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "mvs.02",   0x28000, 0x20000, CRC(b34e773d) SHA1(3bcf44bf06c35814cff29d244142db7abe05bd39) )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "mvs.11m",   0x000000, 0x400000, CRC(86219770) SHA1(4e5b68d382a5aa37f8b0b6434c53a2b95f5f9a4d) )
	ROM_LOAD16_WORD_SWAP( "mvs.12m",   0x400000, 0x400000, CRC(f2fd7f68) SHA1(28a30d55d3eaf963006c7cbe7c288099cd3ba536) )
ROM_END

ROM_START( mshvsfu1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "mvsu.03d", 0x000000, 0x80000, CRC(ae60a66a) SHA1(1fa7e6534d02ec8059153705b1161a55b9cfe803) )
	ROM_LOAD16_WORD_SWAP( "mvsu.04d", 0x080000, 0x80000, CRC(91f67d8a) SHA1(e95f7a3fb281e1bafdbe7a1b22532c4fab5ec89d) )
	ROM_LOAD16_WORD_SWAP( "mvs.05a",  0x100000, 0x80000, CRC(1a5de0cb) SHA1(738a27f83704c208d36d73bf766d861ef2d51a89) )
	ROM_LOAD16_WORD_SWAP( "mvs.06a",  0x180000, 0x80000, CRC(959f3030) SHA1(fbbaa915324815246738f3426232e623f039ce26) )
	ROM_LOAD16_WORD_SWAP( "mvs.07b",  0x200000, 0x80000, CRC(7f915bdb) SHA1(683da09c5ba55e31b59aa95a8e13c45dc574ab3c) )
	ROM_LOAD16_WORD_SWAP( "mvs.08a",  0x280000, 0x80000, CRC(c2813884) SHA1(49e5d4bc48f90c8146cb6aafb9240aff0119f1a7) )
	ROM_LOAD16_WORD_SWAP( "mvs.09b",  0x300000, 0x80000, CRC(3ba08818) SHA1(9ab132a3cac55fcccebe6c99b6fb0ba1305f8f6e) )
	ROM_LOAD16_WORD_SWAP( "mvs.10b",  0x380000, 0x80000, CRC(cf0dba98) SHA1(f4c1f8a6e7a79ecc6241d5268b3039f8a09ea516) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "mvs.13m",   0x0000000, 0x400000, CRC(29b05fd9) SHA1(e8fdb1ee5515a560eb4256ae4fd99bb1192e1a87) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.15m",   0x0000002, 0x400000, CRC(faddccf1) SHA1(4ed03ea91883a0413325f57edcc1614120b5922c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.17m",   0x0000004, 0x400000, CRC(97aaf4c7) SHA1(6a054921cc14fe080cb3f62c391f8ae3cc7e8ba9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.19m",   0x0000006, 0x400000, CRC(cb70e915) SHA1(da4d2480d348ac6dfd01256a88f4f3db8357ae46) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.14m",   0x1000000, 0x400000, CRC(b3b1972d) SHA1(0f2c3fb7de014181ee481ec35d0578b2c116c2dc) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.16m",   0x1000002, 0x400000, CRC(08aadb5d) SHA1(3a2c222eca3e7df80ce69951b3db6442312751a4) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.18m",   0x1000004, 0x400000, CRC(c1228b35) SHA1(7afdfb552888c79d0fbb30242b3d917b87fad57a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.20m",   0x1000006, 0x400000, CRC(366cc6c2) SHA1(6f2a789087c8e404c5227b927fa8328c03593243) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "mvs.01",   0x00000, 0x08000, CRC(68252324) SHA1(138ef320ef27956b2ab5591d49a1315b7b0a194c) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "mvs.02",   0x28000, 0x20000, CRC(b34e773d) SHA1(3bcf44bf06c35814cff29d244142db7abe05bd39) )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "mvs.11m",   0x000000, 0x400000, CRC(86219770) SHA1(4e5b68d382a5aa37f8b0b6434c53a2b95f5f9a4d) )
	ROM_LOAD16_WORD_SWAP( "mvs.12m",   0x400000, 0x400000, CRC(f2fd7f68) SHA1(28a30d55d3eaf963006c7cbe7c288099cd3ba536) )
ROM_END

ROM_START( mshvsfj )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "mvsj.03i", 0x000000, 0x80000, CRC(d8cbb691) SHA1(16820cf3bc7285477e61bd598a3ed4ea5e0e770d) )
	ROM_LOAD16_WORD_SWAP( "mvsj.04i", 0x080000, 0x80000, CRC(32741ace) SHA1(36db3a3aeaf29369977593c051bf5665cffefb2d) )
	ROM_LOAD16_WORD_SWAP( "mvs.05h",  0x100000, 0x80000, CRC(77870dc3) SHA1(924a7c82456bb44d7b0be65af11dbe1a2420a3f0) )
	ROM_LOAD16_WORD_SWAP( "mvs.06a",  0x180000, 0x80000, CRC(959f3030) SHA1(fbbaa915324815246738f3426232e623f039ce26) )
	ROM_LOAD16_WORD_SWAP( "mvs.07b",  0x200000, 0x80000, CRC(7f915bdb) SHA1(683da09c5ba55e31b59aa95a8e13c45dc574ab3c) )
	ROM_LOAD16_WORD_SWAP( "mvs.08a",  0x280000, 0x80000, CRC(c2813884) SHA1(49e5d4bc48f90c8146cb6aafb9240aff0119f1a7) )
	ROM_LOAD16_WORD_SWAP( "mvs.09b",  0x300000, 0x80000, CRC(3ba08818) SHA1(9ab132a3cac55fcccebe6c99b6fb0ba1305f8f6e) )
	ROM_LOAD16_WORD_SWAP( "mvs.10b",  0x380000, 0x80000, CRC(cf0dba98) SHA1(f4c1f8a6e7a79ecc6241d5268b3039f8a09ea516) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "mvs.13m",   0x0000000, 0x400000, CRC(29b05fd9) SHA1(e8fdb1ee5515a560eb4256ae4fd99bb1192e1a87) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.15m",   0x0000002, 0x400000, CRC(faddccf1) SHA1(4ed03ea91883a0413325f57edcc1614120b5922c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.17m",   0x0000004, 0x400000, CRC(97aaf4c7) SHA1(6a054921cc14fe080cb3f62c391f8ae3cc7e8ba9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.19m",   0x0000006, 0x400000, CRC(cb70e915) SHA1(da4d2480d348ac6dfd01256a88f4f3db8357ae46) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.14m",   0x1000000, 0x400000, CRC(b3b1972d) SHA1(0f2c3fb7de014181ee481ec35d0578b2c116c2dc) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.16m",   0x1000002, 0x400000, CRC(08aadb5d) SHA1(3a2c222eca3e7df80ce69951b3db6442312751a4) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.18m",   0x1000004, 0x400000, CRC(c1228b35) SHA1(7afdfb552888c79d0fbb30242b3d917b87fad57a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.20m",   0x1000006, 0x400000, CRC(366cc6c2) SHA1(6f2a789087c8e404c5227b927fa8328c03593243) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "mvs.01",   0x00000, 0x08000, CRC(68252324) SHA1(138ef320ef27956b2ab5591d49a1315b7b0a194c) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "mvs.02",   0x28000, 0x20000, CRC(b34e773d) SHA1(3bcf44bf06c35814cff29d244142db7abe05bd39) )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "mvs.11m",   0x000000, 0x400000, CRC(86219770) SHA1(4e5b68d382a5aa37f8b0b6434c53a2b95f5f9a4d) )
	ROM_LOAD16_WORD_SWAP( "mvs.12m",   0x400000, 0x400000, CRC(f2fd7f68) SHA1(28a30d55d3eaf963006c7cbe7c288099cd3ba536) )
ROM_END

ROM_START( mshvsfj1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "mvsj.03h", 0x000000, 0x80000, CRC(fbe2115f) SHA1(b2d8a62e394c2eb4070cac742b0f403252e46a25) )
	ROM_LOAD16_WORD_SWAP( "mvsj.04h", 0x080000, 0x80000, CRC(b528a367) SHA1(ecac71b032b431c63a4cf73a1d1d1be1faebc12b) )
	ROM_LOAD16_WORD_SWAP( "mvs.05g",  0x100000, 0x80000, CRC(9515a245) SHA1(eafa877fd4a4e58e7c98336658e986a4a27d6b91) )
	ROM_LOAD16_WORD_SWAP( "mvs.06a",  0x180000, 0x80000, CRC(959f3030) SHA1(fbbaa915324815246738f3426232e623f039ce26) )
	ROM_LOAD16_WORD_SWAP( "mvs.07b",  0x200000, 0x80000, CRC(7f915bdb) SHA1(683da09c5ba55e31b59aa95a8e13c45dc574ab3c) )
	ROM_LOAD16_WORD_SWAP( "mvs.08a",  0x280000, 0x80000, CRC(c2813884) SHA1(49e5d4bc48f90c8146cb6aafb9240aff0119f1a7) )
	ROM_LOAD16_WORD_SWAP( "mvs.09b",  0x300000, 0x80000, CRC(3ba08818) SHA1(9ab132a3cac55fcccebe6c99b6fb0ba1305f8f6e) )
	ROM_LOAD16_WORD_SWAP( "mvs.10b",  0x380000, 0x80000, CRC(cf0dba98) SHA1(f4c1f8a6e7a79ecc6241d5268b3039f8a09ea516) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "mvs.13m",   0x0000000, 0x400000, CRC(29b05fd9) SHA1(e8fdb1ee5515a560eb4256ae4fd99bb1192e1a87) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.15m",   0x0000002, 0x400000, CRC(faddccf1) SHA1(4ed03ea91883a0413325f57edcc1614120b5922c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.17m",   0x0000004, 0x400000, CRC(97aaf4c7) SHA1(6a054921cc14fe080cb3f62c391f8ae3cc7e8ba9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.19m",   0x0000006, 0x400000, CRC(cb70e915) SHA1(da4d2480d348ac6dfd01256a88f4f3db8357ae46) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.14m",   0x1000000, 0x400000, CRC(b3b1972d) SHA1(0f2c3fb7de014181ee481ec35d0578b2c116c2dc) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.16m",   0x1000002, 0x400000, CRC(08aadb5d) SHA1(3a2c222eca3e7df80ce69951b3db6442312751a4) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.18m",   0x1000004, 0x400000, CRC(c1228b35) SHA1(7afdfb552888c79d0fbb30242b3d917b87fad57a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.20m",   0x1000006, 0x400000, CRC(366cc6c2) SHA1(6f2a789087c8e404c5227b927fa8328c03593243) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "mvs.01",   0x00000, 0x08000, CRC(68252324) SHA1(138ef320ef27956b2ab5591d49a1315b7b0a194c) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "mvs.02",   0x28000, 0x20000, CRC(b34e773d) SHA1(3bcf44bf06c35814cff29d244142db7abe05bd39) )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "mvs.11m",   0x000000, 0x400000, CRC(86219770) SHA1(4e5b68d382a5aa37f8b0b6434c53a2b95f5f9a4d) )
	ROM_LOAD16_WORD_SWAP( "mvs.12m",   0x400000, 0x400000, CRC(f2fd7f68) SHA1(28a30d55d3eaf963006c7cbe7c288099cd3ba536) )
ROM_END

ROM_START( mshvsfj2 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "mvsj.03g", 0x000000, 0x80000, CRC(fdfa7e26) SHA1(e9fb93249e48e1bb7c769c3ce674dd4be404574f) )
	ROM_LOAD16_WORD_SWAP( "mvsj.04g", 0x080000, 0x80000, CRC(c921825f) SHA1(471e44268cebba631b81f131bf31e27b8a28c548) )
	ROM_LOAD16_WORD_SWAP( "mvs.05a",  0x100000, 0x80000, CRC(1a5de0cb) SHA1(738a27f83704c208d36d73bf766d861ef2d51a89) )
	ROM_LOAD16_WORD_SWAP( "mvs.06a",  0x180000, 0x80000, CRC(959f3030) SHA1(fbbaa915324815246738f3426232e623f039ce26) )
	ROM_LOAD16_WORD_SWAP( "mvs.07b",  0x200000, 0x80000, CRC(7f915bdb) SHA1(683da09c5ba55e31b59aa95a8e13c45dc574ab3c) )
	ROM_LOAD16_WORD_SWAP( "mvs.08a",  0x280000, 0x80000, CRC(c2813884) SHA1(49e5d4bc48f90c8146cb6aafb9240aff0119f1a7) )
	ROM_LOAD16_WORD_SWAP( "mvs.09b",  0x300000, 0x80000, CRC(3ba08818) SHA1(9ab132a3cac55fcccebe6c99b6fb0ba1305f8f6e) )
	ROM_LOAD16_WORD_SWAP( "mvs.10b",  0x380000, 0x80000, CRC(cf0dba98) SHA1(f4c1f8a6e7a79ecc6241d5268b3039f8a09ea516) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "mvs.13m",   0x0000000, 0x400000, CRC(29b05fd9) SHA1(e8fdb1ee5515a560eb4256ae4fd99bb1192e1a87) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.15m",   0x0000002, 0x400000, CRC(faddccf1) SHA1(4ed03ea91883a0413325f57edcc1614120b5922c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.17m",   0x0000004, 0x400000, CRC(97aaf4c7) SHA1(6a054921cc14fe080cb3f62c391f8ae3cc7e8ba9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.19m",   0x0000006, 0x400000, CRC(cb70e915) SHA1(da4d2480d348ac6dfd01256a88f4f3db8357ae46) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.14m",   0x1000000, 0x400000, CRC(b3b1972d) SHA1(0f2c3fb7de014181ee481ec35d0578b2c116c2dc) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.16m",   0x1000002, 0x400000, CRC(08aadb5d) SHA1(3a2c222eca3e7df80ce69951b3db6442312751a4) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.18m",   0x1000004, 0x400000, CRC(c1228b35) SHA1(7afdfb552888c79d0fbb30242b3d917b87fad57a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.20m",   0x1000006, 0x400000, CRC(366cc6c2) SHA1(6f2a789087c8e404c5227b927fa8328c03593243) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "mvs.01",   0x00000, 0x08000, CRC(68252324) SHA1(138ef320ef27956b2ab5591d49a1315b7b0a194c) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "mvs.02",   0x28000, 0x20000, CRC(b34e773d) SHA1(3bcf44bf06c35814cff29d244142db7abe05bd39) )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "mvs.11m",   0x000000, 0x400000, CRC(86219770) SHA1(4e5b68d382a5aa37f8b0b6434c53a2b95f5f9a4d) )
	ROM_LOAD16_WORD_SWAP( "mvs.12m",   0x400000, 0x400000, CRC(f2fd7f68) SHA1(28a30d55d3eaf963006c7cbe7c288099cd3ba536) )
ROM_END

ROM_START( mshvsfh )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "mvsh.03f", 0x000000, 0x80000, CRC(4f60f41e) SHA1(dd926a9cac4bff05845615d0b61948e2dc4b1ed8) )
	ROM_LOAD16_WORD_SWAP( "mvsh.04f", 0x080000, 0x80000, CRC(dc08ec12) SHA1(594e4383eb776c09075577cd1f4e42ef11748f0f) )
	ROM_LOAD16_WORD_SWAP( "mvs.05a",  0x100000, 0x80000, CRC(1a5de0cb) SHA1(738a27f83704c208d36d73bf766d861ef2d51a89) )
	ROM_LOAD16_WORD_SWAP( "mvs.06a",  0x180000, 0x80000, CRC(959f3030) SHA1(fbbaa915324815246738f3426232e623f039ce26) )
	ROM_LOAD16_WORD_SWAP( "mvs.07b",  0x200000, 0x80000, CRC(7f915bdb) SHA1(683da09c5ba55e31b59aa95a8e13c45dc574ab3c) )
	ROM_LOAD16_WORD_SWAP( "mvs.08a",  0x280000, 0x80000, CRC(c2813884) SHA1(49e5d4bc48f90c8146cb6aafb9240aff0119f1a7) )
	ROM_LOAD16_WORD_SWAP( "mvs.09b",  0x300000, 0x80000, CRC(3ba08818) SHA1(9ab132a3cac55fcccebe6c99b6fb0ba1305f8f6e) )
	ROM_LOAD16_WORD_SWAP( "mvs.10b",  0x380000, 0x80000, CRC(cf0dba98) SHA1(f4c1f8a6e7a79ecc6241d5268b3039f8a09ea516) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "mvs.13m",   0x0000000, 0x400000, CRC(29b05fd9) SHA1(e8fdb1ee5515a560eb4256ae4fd99bb1192e1a87) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.15m",   0x0000002, 0x400000, CRC(faddccf1) SHA1(4ed03ea91883a0413325f57edcc1614120b5922c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.17m",   0x0000004, 0x400000, CRC(97aaf4c7) SHA1(6a054921cc14fe080cb3f62c391f8ae3cc7e8ba9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.19m",   0x0000006, 0x400000, CRC(cb70e915) SHA1(da4d2480d348ac6dfd01256a88f4f3db8357ae46) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.14m",   0x1000000, 0x400000, CRC(b3b1972d) SHA1(0f2c3fb7de014181ee481ec35d0578b2c116c2dc) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.16m",   0x1000002, 0x400000, CRC(08aadb5d) SHA1(3a2c222eca3e7df80ce69951b3db6442312751a4) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.18m",   0x1000004, 0x400000, CRC(c1228b35) SHA1(7afdfb552888c79d0fbb30242b3d917b87fad57a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.20m",   0x1000006, 0x400000, CRC(366cc6c2) SHA1(6f2a789087c8e404c5227b927fa8328c03593243) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "mvs.01",   0x00000, 0x08000, CRC(68252324) SHA1(138ef320ef27956b2ab5591d49a1315b7b0a194c) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "mvs.02",   0x28000, 0x20000, CRC(b34e773d) SHA1(3bcf44bf06c35814cff29d244142db7abe05bd39) )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "mvs.11m",   0x000000, 0x400000, CRC(86219770) SHA1(4e5b68d382a5aa37f8b0b6434c53a2b95f5f9a4d) )
	ROM_LOAD16_WORD_SWAP( "mvs.12m",   0x400000, 0x400000, CRC(f2fd7f68) SHA1(28a30d55d3eaf963006c7cbe7c288099cd3ba536) )
ROM_END

ROM_START( mshvsfa )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "mvsa.03f", 0x000000, 0x80000, CRC(5b863716) SHA1(6a129274711765bbf5acbb225e3fce6d93d7f421) )
	ROM_LOAD16_WORD_SWAP( "mvsa.04f", 0x080000, 0x80000, CRC(4886e65f) SHA1(758fc9c453a864e32588c7fb33166c93e798a39c) )
	ROM_LOAD16_WORD_SWAP( "mvs.05a",  0x100000, 0x80000, CRC(1a5de0cb) SHA1(738a27f83704c208d36d73bf766d861ef2d51a89) )
	ROM_LOAD16_WORD_SWAP( "mvs.06a",  0x180000, 0x80000, CRC(959f3030) SHA1(fbbaa915324815246738f3426232e623f039ce26) )
	ROM_LOAD16_WORD_SWAP( "mvs.07b",  0x200000, 0x80000, CRC(7f915bdb) SHA1(683da09c5ba55e31b59aa95a8e13c45dc574ab3c) )
	ROM_LOAD16_WORD_SWAP( "mvs.08a",  0x280000, 0x80000, CRC(c2813884) SHA1(49e5d4bc48f90c8146cb6aafb9240aff0119f1a7) )
	ROM_LOAD16_WORD_SWAP( "mvs.09b",  0x300000, 0x80000, CRC(3ba08818) SHA1(9ab132a3cac55fcccebe6c99b6fb0ba1305f8f6e) )
	ROM_LOAD16_WORD_SWAP( "mvs.10b",  0x380000, 0x80000, CRC(cf0dba98) SHA1(f4c1f8a6e7a79ecc6241d5268b3039f8a09ea516) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "mvs.13m",   0x0000000, 0x400000, CRC(29b05fd9) SHA1(e8fdb1ee5515a560eb4256ae4fd99bb1192e1a87) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.15m",   0x0000002, 0x400000, CRC(faddccf1) SHA1(4ed03ea91883a0413325f57edcc1614120b5922c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.17m",   0x0000004, 0x400000, CRC(97aaf4c7) SHA1(6a054921cc14fe080cb3f62c391f8ae3cc7e8ba9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.19m",   0x0000006, 0x400000, CRC(cb70e915) SHA1(da4d2480d348ac6dfd01256a88f4f3db8357ae46) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.14m",   0x1000000, 0x400000, CRC(b3b1972d) SHA1(0f2c3fb7de014181ee481ec35d0578b2c116c2dc) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.16m",   0x1000002, 0x400000, CRC(08aadb5d) SHA1(3a2c222eca3e7df80ce69951b3db6442312751a4) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.18m",   0x1000004, 0x400000, CRC(c1228b35) SHA1(7afdfb552888c79d0fbb30242b3d917b87fad57a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.20m",   0x1000006, 0x400000, CRC(366cc6c2) SHA1(6f2a789087c8e404c5227b927fa8328c03593243) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "mvs.01",   0x00000, 0x08000, CRC(68252324) SHA1(138ef320ef27956b2ab5591d49a1315b7b0a194c) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "mvs.02",   0x28000, 0x20000, CRC(b34e773d) SHA1(3bcf44bf06c35814cff29d244142db7abe05bd39) )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "mvs.11m",   0x000000, 0x400000, CRC(86219770) SHA1(4e5b68d382a5aa37f8b0b6434c53a2b95f5f9a4d) )
	ROM_LOAD16_WORD_SWAP( "mvs.12m",   0x400000, 0x400000, CRC(f2fd7f68) SHA1(28a30d55d3eaf963006c7cbe7c288099cd3ba536) )
ROM_END

ROM_START( mshvsfa1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "mvsa.03", 0x000000, 0x80000, CRC(92ef1933) SHA1(34e6a074734032af74afa52bfebbc213a9c886d7) )
	ROM_LOAD16_WORD_SWAP( "mvsa.04", 0x080000, 0x80000, CRC(4b24373c) SHA1(f340dda7d5339645fd1ea523e72783fb7bb7aba1) )
	ROM_LOAD16_WORD_SWAP( "mvs.05",  0x100000, 0x80000, CRC(ac180c1c) SHA1(1b368ebe7680796dc068b511b72359eec546cd9f) )
	ROM_LOAD16_WORD_SWAP( "mvs.06a", 0x180000, 0x80000, CRC(959f3030) SHA1(fbbaa915324815246738f3426232e623f039ce26) )
	ROM_LOAD16_WORD_SWAP( "mvs.07b", 0x200000, 0x80000, CRC(7f915bdb) SHA1(683da09c5ba55e31b59aa95a8e13c45dc574ab3c) )
	ROM_LOAD16_WORD_SWAP( "mvs.08a", 0x280000, 0x80000, CRC(c2813884) SHA1(49e5d4bc48f90c8146cb6aafb9240aff0119f1a7) )
	ROM_LOAD16_WORD_SWAP( "mvs.09b", 0x300000, 0x80000, CRC(3ba08818) SHA1(9ab132a3cac55fcccebe6c99b6fb0ba1305f8f6e) )
	ROM_LOAD16_WORD_SWAP( "mvs.10b", 0x380000, 0x80000, CRC(cf0dba98) SHA1(f4c1f8a6e7a79ecc6241d5268b3039f8a09ea516) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "mvs.13m",   0x0000000, 0x400000, CRC(29b05fd9) SHA1(e8fdb1ee5515a560eb4256ae4fd99bb1192e1a87) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.15m",   0x0000002, 0x400000, CRC(faddccf1) SHA1(4ed03ea91883a0413325f57edcc1614120b5922c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.17m",   0x0000004, 0x400000, CRC(97aaf4c7) SHA1(6a054921cc14fe080cb3f62c391f8ae3cc7e8ba9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.19m",   0x0000006, 0x400000, CRC(cb70e915) SHA1(da4d2480d348ac6dfd01256a88f4f3db8357ae46) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.14m",   0x1000000, 0x400000, CRC(b3b1972d) SHA1(0f2c3fb7de014181ee481ec35d0578b2c116c2dc) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.16m",   0x1000002, 0x400000, CRC(08aadb5d) SHA1(3a2c222eca3e7df80ce69951b3db6442312751a4) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.18m",   0x1000004, 0x400000, CRC(c1228b35) SHA1(7afdfb552888c79d0fbb30242b3d917b87fad57a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.20m",   0x1000006, 0x400000, CRC(366cc6c2) SHA1(6f2a789087c8e404c5227b927fa8328c03593243) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "mvs.01",   0x00000, 0x08000, CRC(68252324) SHA1(138ef320ef27956b2ab5591d49a1315b7b0a194c) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "mvs.02",   0x28000, 0x20000, CRC(b34e773d) SHA1(3bcf44bf06c35814cff29d244142db7abe05bd39) )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "mvs.11m",   0x000000, 0x400000, CRC(86219770) SHA1(4e5b68d382a5aa37f8b0b6434c53a2b95f5f9a4d) )
	ROM_LOAD16_WORD_SWAP( "mvs.12m",   0x400000, 0x400000, CRC(f2fd7f68) SHA1(28a30d55d3eaf963006c7cbe7c288099cd3ba536) )
ROM_END

ROM_START( mshvsfb )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "mvsb.03g", 0x000000, 0x80000, CRC(143895ef) SHA1(0664fad64996118df86e9887bd6e301d04d84978) )
	ROM_LOAD16_WORD_SWAP( "mvsb.04g", 0x080000, 0x80000, CRC(dd8a886c) SHA1(a16f262fd14e726c7837980d0556a9c3bdc7fb11) )
	ROM_LOAD16_WORD_SWAP( "mvs.05d",  0x100000, 0x80000, CRC(921fc542) SHA1(b813082a480d42d663c713062892245faabe9101) )
	ROM_LOAD16_WORD_SWAP( "mvs.06a",  0x180000, 0x80000, CRC(959f3030) SHA1(fbbaa915324815246738f3426232e623f039ce26) )
	ROM_LOAD16_WORD_SWAP( "mvs.07b",  0x200000, 0x80000, CRC(7f915bdb) SHA1(683da09c5ba55e31b59aa95a8e13c45dc574ab3c) )
	ROM_LOAD16_WORD_SWAP( "mvs.08a",  0x280000, 0x80000, CRC(c2813884) SHA1(49e5d4bc48f90c8146cb6aafb9240aff0119f1a7) )
	ROM_LOAD16_WORD_SWAP( "mvs.09b",  0x300000, 0x80000, CRC(3ba08818) SHA1(9ab132a3cac55fcccebe6c99b6fb0ba1305f8f6e) )
	ROM_LOAD16_WORD_SWAP( "mvs.10b",  0x380000, 0x80000, CRC(cf0dba98) SHA1(f4c1f8a6e7a79ecc6241d5268b3039f8a09ea516) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "mvs.13m",   0x0000000, 0x400000, CRC(29b05fd9) SHA1(e8fdb1ee5515a560eb4256ae4fd99bb1192e1a87) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.15m",   0x0000002, 0x400000, CRC(faddccf1) SHA1(4ed03ea91883a0413325f57edcc1614120b5922c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.17m",   0x0000004, 0x400000, CRC(97aaf4c7) SHA1(6a054921cc14fe080cb3f62c391f8ae3cc7e8ba9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.19m",   0x0000006, 0x400000, CRC(cb70e915) SHA1(da4d2480d348ac6dfd01256a88f4f3db8357ae46) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.14m",   0x1000000, 0x400000, CRC(b3b1972d) SHA1(0f2c3fb7de014181ee481ec35d0578b2c116c2dc) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.16m",   0x1000002, 0x400000, CRC(08aadb5d) SHA1(3a2c222eca3e7df80ce69951b3db6442312751a4) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.18m",   0x1000004, 0x400000, CRC(c1228b35) SHA1(7afdfb552888c79d0fbb30242b3d917b87fad57a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.20m",   0x1000006, 0x400000, CRC(366cc6c2) SHA1(6f2a789087c8e404c5227b927fa8328c03593243) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "mvs.01",   0x00000, 0x08000, CRC(68252324) SHA1(138ef320ef27956b2ab5591d49a1315b7b0a194c) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "mvs.02",   0x28000, 0x20000, CRC(b34e773d) SHA1(3bcf44bf06c35814cff29d244142db7abe05bd39) )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "mvs.11m",   0x000000, 0x400000, CRC(86219770) SHA1(4e5b68d382a5aa37f8b0b6434c53a2b95f5f9a4d) )
	ROM_LOAD16_WORD_SWAP( "mvs.12m",   0x400000, 0x400000, CRC(f2fd7f68) SHA1(28a30d55d3eaf963006c7cbe7c288099cd3ba536) )
ROM_END

ROM_START( mshvsfb1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "mvsb.03f", 0x000000, 0x80000, CRC(9c4bb950) SHA1(85d0026a691682c195d6e617bf28def50029cb03) )
	ROM_LOAD16_WORD_SWAP( "mvsb.04f", 0x080000, 0x80000, CRC(d3320d13) SHA1(c6fa2b8b727a1192fd21131496067447053b5547) )
	ROM_LOAD16_WORD_SWAP( "mvs.05a",  0x100000, 0x80000, CRC(1a5de0cb) SHA1(738a27f83704c208d36d73bf766d861ef2d51a89) )
	ROM_LOAD16_WORD_SWAP( "mvs.06a",  0x180000, 0x80000, CRC(959f3030) SHA1(fbbaa915324815246738f3426232e623f039ce26) )
	ROM_LOAD16_WORD_SWAP( "mvs.07b",  0x200000, 0x80000, CRC(7f915bdb) SHA1(683da09c5ba55e31b59aa95a8e13c45dc574ab3c) )
	ROM_LOAD16_WORD_SWAP( "mvs.08a",  0x280000, 0x80000, CRC(c2813884) SHA1(49e5d4bc48f90c8146cb6aafb9240aff0119f1a7) )
	ROM_LOAD16_WORD_SWAP( "mvs.09b",  0x300000, 0x80000, CRC(3ba08818) SHA1(9ab132a3cac55fcccebe6c99b6fb0ba1305f8f6e) )
	ROM_LOAD16_WORD_SWAP( "mvs.10b",  0x380000, 0x80000, CRC(cf0dba98) SHA1(f4c1f8a6e7a79ecc6241d5268b3039f8a09ea516) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "mvs.13m",   0x0000000, 0x400000, CRC(29b05fd9) SHA1(e8fdb1ee5515a560eb4256ae4fd99bb1192e1a87) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.15m",   0x0000002, 0x400000, CRC(faddccf1) SHA1(4ed03ea91883a0413325f57edcc1614120b5922c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.17m",   0x0000004, 0x400000, CRC(97aaf4c7) SHA1(6a054921cc14fe080cb3f62c391f8ae3cc7e8ba9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.19m",   0x0000006, 0x400000, CRC(cb70e915) SHA1(da4d2480d348ac6dfd01256a88f4f3db8357ae46) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.14m",   0x1000000, 0x400000, CRC(b3b1972d) SHA1(0f2c3fb7de014181ee481ec35d0578b2c116c2dc) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.16m",   0x1000002, 0x400000, CRC(08aadb5d) SHA1(3a2c222eca3e7df80ce69951b3db6442312751a4) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.18m",   0x1000004, 0x400000, CRC(c1228b35) SHA1(7afdfb552888c79d0fbb30242b3d917b87fad57a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvs.20m",   0x1000006, 0x400000, CRC(366cc6c2) SHA1(6f2a789087c8e404c5227b927fa8328c03593243) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "mvs.01",   0x00000, 0x08000, CRC(68252324) SHA1(138ef320ef27956b2ab5591d49a1315b7b0a194c) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "mvs.02",   0x28000, 0x20000, CRC(b34e773d) SHA1(3bcf44bf06c35814cff29d244142db7abe05bd39) )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "mvs.11m",   0x000000, 0x400000, CRC(86219770) SHA1(4e5b68d382a5aa37f8b0b6434c53a2b95f5f9a4d) )
	ROM_LOAD16_WORD_SWAP( "mvs.12m",   0x400000, 0x400000, CRC(f2fd7f68) SHA1(28a30d55d3eaf963006c7cbe7c288099cd3ba536) )
ROM_END

ROM_START( mvsc )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "mvce.03a", 0x000000, 0x80000, CRC(824e4a90) SHA1(5c79c166d988d8a75d9941f4ee6fa4d6476e55e1) )
	ROM_LOAD16_WORD_SWAP( "mvce.04a", 0x080000, 0x80000, BAD_DUMP CRC(cac02153) SHA1(ee9f9da6fda53f21ba7b74367612c90281269690) )
	ROM_LOAD16_WORD_SWAP( "mvc.05a",  0x100000, 0x80000, CRC(2d8c8e86) SHA1(b07d640a734c5d336054ed05195786224c9a6cd4) )
	ROM_LOAD16_WORD_SWAP( "mvc.06a",  0x180000, 0x80000, CRC(8528e1f5) SHA1(cd065c05268ab581b05676da544baf6af642acac) )
	ROM_LOAD16_WORD_SWAP( "mvc.07",   0x200000, 0x80000, CRC(c3baa32b) SHA1(d35589847e0753e869ffcd7c3abed925bfdb0fa2) )
	ROM_LOAD16_WORD_SWAP( "mvc.08",   0x280000, 0x80000, CRC(bc002fcd) SHA1(0b6735a071a9274f7ab25c743271fc30411fe819) )
	ROM_LOAD16_WORD_SWAP( "mvc.09",   0x300000, 0x80000, CRC(c67b26df) SHA1(6e9969246c57269d7ba0992a5cc319c8910bf8a9) )
	ROM_LOAD16_WORD_SWAP( "mvc.10",   0x380000, 0x80000, CRC(0fdd1e26) SHA1(5fa684d823b4f4eec61ed9e9b8938af5272ae1ed) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "mvc.13m",   0x0000000, 0x400000, CRC(fa5f74bc) SHA1(79a619248938a85ce4f7794a704647b9cf564fbc) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.15m",   0x0000002, 0x400000, CRC(71938a8f) SHA1(6982f7203458c1c46a1c1c13c0d0f2a5e109d271) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.17m",   0x0000004, 0x400000, CRC(92741d07) SHA1(ddfd70eab7c983ab452194b1860059f8ad694459) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.19m",   0x0000006, 0x400000, CRC(bcb72fc6) SHA1(46ab98dcdf6f5d611646a22a7355939ef5b2bbe5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.14m",   0x1000000, 0x400000, CRC(7f1df4e4) SHA1(ede92b31c1fe87f91b4fe74ac211f2fb5f863bc2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.16m",   0x1000002, 0x400000, CRC(90bd3203) SHA1(ed83208c486ea0f407b7e5d16a8cf242a6f73774) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.18m",   0x1000004, 0x400000, CRC(67aaf727) SHA1(e0e69104e31d2c41e18c0d24e9ab962406a7ca9a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.20m",   0x1000006, 0x400000, CRC(8b0bade8) SHA1(c5732361bb4bf284c4d12a82ac2c5750b1f9d441) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "mvc.01",   0x00000, 0x08000, CRC(41629e95) SHA1(36925c05b5fdcbe43283a882d021e5360c947061) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "mvc.02",   0x28000, 0x20000, CRC(963abf6b) SHA1(6b784870e338701cefabbbe4669984b5c4e8a9a5) )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "mvc.11m",   0x000000, 0x400000, CRC(850fe663) SHA1(81e519d05a08855f242ea2e17ee0859b449db895) )
	ROM_LOAD16_WORD_SWAP( "mvc.12m",   0x400000, 0x400000, CRC(7ccb1896) SHA1(74caadf3282fcc6acffb1bbe3734106f81124121) )
ROM_END

ROM_START( mvscr1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "mvce.03", 0x000000, 0x80000, CRC(e0633fc0) SHA1(d2feffb5505a2f8940192ae267f49561fc580b87) )
	ROM_LOAD16_WORD_SWAP( "mvce.04", 0x080000, 0x80000, CRC(a450a251) SHA1(1e34fa55bb93c7573ab0205f8c5620a51765d3eb) )
	ROM_LOAD16_WORD_SWAP( "mvc.05",  0x100000, 0x80000, CRC(7db71ce9) SHA1(a0097109e9f4aba40791932269d600c0ffa099a7) )
	ROM_LOAD16_WORD_SWAP( "mvc.06",  0x180000, 0x80000, CRC(4b0b6d3e) SHA1(375372adf0a508bb6fc6a79326b2d4171db9ca0f) )
	ROM_LOAD16_WORD_SWAP( "mvc.07",  0x200000, 0x80000, CRC(c3baa32b) SHA1(d35589847e0753e869ffcd7c3abed925bfdb0fa2) )
	ROM_LOAD16_WORD_SWAP( "mvc.08",  0x280000, 0x80000, CRC(bc002fcd) SHA1(0b6735a071a9274f7ab25c743271fc30411fe819) )
	ROM_LOAD16_WORD_SWAP( "mvc.09",  0x300000, 0x80000, CRC(c67b26df) SHA1(6e9969246c57269d7ba0992a5cc319c8910bf8a9) )
	ROM_LOAD16_WORD_SWAP( "mvc.10",  0x380000, 0x80000, CRC(0fdd1e26) SHA1(5fa684d823b4f4eec61ed9e9b8938af5272ae1ed) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "mvc.13m",   0x0000000, 0x400000, CRC(fa5f74bc) SHA1(79a619248938a85ce4f7794a704647b9cf564fbc) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.15m",   0x0000002, 0x400000, CRC(71938a8f) SHA1(6982f7203458c1c46a1c1c13c0d0f2a5e109d271) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.17m",   0x0000004, 0x400000, CRC(92741d07) SHA1(ddfd70eab7c983ab452194b1860059f8ad694459) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.19m",   0x0000006, 0x400000, CRC(bcb72fc6) SHA1(46ab98dcdf6f5d611646a22a7355939ef5b2bbe5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.14m",   0x1000000, 0x400000, CRC(7f1df4e4) SHA1(ede92b31c1fe87f91b4fe74ac211f2fb5f863bc2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.16m",   0x1000002, 0x400000, CRC(90bd3203) SHA1(ed83208c486ea0f407b7e5d16a8cf242a6f73774) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.18m",   0x1000004, 0x400000, CRC(67aaf727) SHA1(e0e69104e31d2c41e18c0d24e9ab962406a7ca9a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.20m",   0x1000006, 0x400000, CRC(8b0bade8) SHA1(c5732361bb4bf284c4d12a82ac2c5750b1f9d441) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "mvc.01",   0x00000, 0x08000, CRC(41629e95) SHA1(36925c05b5fdcbe43283a882d021e5360c947061) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "mvc.02",   0x28000, 0x20000, CRC(963abf6b) SHA1(6b784870e338701cefabbbe4669984b5c4e8a9a5) )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "mvc.11m",   0x000000, 0x400000, CRC(850fe663) SHA1(81e519d05a08855f242ea2e17ee0859b449db895) )
	ROM_LOAD16_WORD_SWAP( "mvc.12m",   0x400000, 0x400000, CRC(7ccb1896) SHA1(74caadf3282fcc6acffb1bbe3734106f81124121) )
ROM_END

ROM_START( mvscu )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "mvcu.03d", 0x000000, 0x80000, CRC(c6007557) SHA1(c027c1a204345ce611cb042d60939e4de156763f) )
	ROM_LOAD16_WORD_SWAP( "mvcu.04d", 0x080000, 0x80000, CRC(724b2b20) SHA1(872bbcf5d344d634f3523318fa4763e6d6302bb5) )
	ROM_LOAD16_WORD_SWAP( "mvc.05a",  0x100000, 0x80000, CRC(2d8c8e86) SHA1(b07d640a734c5d336054ed05195786224c9a6cd4) )
	ROM_LOAD16_WORD_SWAP( "mvc.06a",  0x180000, 0x80000, CRC(8528e1f5) SHA1(cd065c05268ab581b05676da544baf6af642acac) )
	ROM_LOAD16_WORD_SWAP( "mvc.07",   0x200000, 0x80000, CRC(c3baa32b) SHA1(d35589847e0753e869ffcd7c3abed925bfdb0fa2) )
	ROM_LOAD16_WORD_SWAP( "mvc.08",   0x280000, 0x80000, CRC(bc002fcd) SHA1(0b6735a071a9274f7ab25c743271fc30411fe819) )
	ROM_LOAD16_WORD_SWAP( "mvc.09",   0x300000, 0x80000, CRC(c67b26df) SHA1(6e9969246c57269d7ba0992a5cc319c8910bf8a9) )
	ROM_LOAD16_WORD_SWAP( "mvc.10",   0x380000, 0x80000, CRC(0fdd1e26) SHA1(5fa684d823b4f4eec61ed9e9b8938af5272ae1ed) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "mvc.13m",   0x0000000, 0x400000, CRC(fa5f74bc) SHA1(79a619248938a85ce4f7794a704647b9cf564fbc) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.15m",   0x0000002, 0x400000, CRC(71938a8f) SHA1(6982f7203458c1c46a1c1c13c0d0f2a5e109d271) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.17m",   0x0000004, 0x400000, CRC(92741d07) SHA1(ddfd70eab7c983ab452194b1860059f8ad694459) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.19m",   0x0000006, 0x400000, CRC(bcb72fc6) SHA1(46ab98dcdf6f5d611646a22a7355939ef5b2bbe5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.14m",   0x1000000, 0x400000, CRC(7f1df4e4) SHA1(ede92b31c1fe87f91b4fe74ac211f2fb5f863bc2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.16m",   0x1000002, 0x400000, CRC(90bd3203) SHA1(ed83208c486ea0f407b7e5d16a8cf242a6f73774) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.18m",   0x1000004, 0x400000, CRC(67aaf727) SHA1(e0e69104e31d2c41e18c0d24e9ab962406a7ca9a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.20m",   0x1000006, 0x400000, CRC(8b0bade8) SHA1(c5732361bb4bf284c4d12a82ac2c5750b1f9d441) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "mvc.01",   0x00000, 0x08000, CRC(41629e95) SHA1(36925c05b5fdcbe43283a882d021e5360c947061) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "mvc.02",   0x28000, 0x20000, CRC(963abf6b) SHA1(6b784870e338701cefabbbe4669984b5c4e8a9a5) )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "mvc.11m",   0x000000, 0x400000, CRC(850fe663) SHA1(81e519d05a08855f242ea2e17ee0859b449db895) )
	ROM_LOAD16_WORD_SWAP( "mvc.12m",   0x400000, 0x400000, CRC(7ccb1896) SHA1(74caadf3282fcc6acffb1bbe3734106f81124121) )
ROM_END

ROM_START( mvscur1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "mvcu.03",  0x000000, 0x80000, CRC(13f2be57) SHA1(229520c5f4171abae1acdfb9544b356b568b5f2d) )
	ROM_LOAD16_WORD_SWAP( "mvcu.04",  0x080000, 0x80000, CRC(5e9b380d) SHA1(8989192b575fb717c13bfb8175e83d7f1a285310) )
	ROM_LOAD16_WORD_SWAP( "mvcu.05",  0x100000, 0x80000, CRC(12f321be) SHA1(32942c4690253c1545ce05dc084de227e46d942a) )
	ROM_LOAD16_WORD_SWAP( "mvcu.06",  0x180000, 0x80000, CRC(2f1524bc) SHA1(b6543d40fb98eabec82787e0fd60fbc59069e72e) )
	ROM_LOAD16_WORD_SWAP( "mvcu.07",  0x200000, 0x80000, CRC(5fdecadb) SHA1(59726e535d1fd9cc4ddfdd02c936121d70723ced) )
	ROM_LOAD16_WORD_SWAP( "mvc.08",   0x280000, 0x80000, CRC(bc002fcd) SHA1(0b6735a071a9274f7ab25c743271fc30411fe819) )
	ROM_LOAD16_WORD_SWAP( "mvc.09",   0x300000, 0x80000, CRC(c67b26df) SHA1(6e9969246c57269d7ba0992a5cc319c8910bf8a9) )
	ROM_LOAD16_WORD_SWAP( "mvcu.10",  0x380000, 0x80000, CRC(4f36cd63) SHA1(ee6bdbf14bd524f76f4f756f332315331d5e1e4b) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "mvc.13m",   0x0000000, 0x400000, CRC(fa5f74bc) SHA1(79a619248938a85ce4f7794a704647b9cf564fbc) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.15m",   0x0000002, 0x400000, CRC(71938a8f) SHA1(6982f7203458c1c46a1c1c13c0d0f2a5e109d271) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.17m",   0x0000004, 0x400000, CRC(92741d07) SHA1(ddfd70eab7c983ab452194b1860059f8ad694459) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.19m",   0x0000006, 0x400000, CRC(bcb72fc6) SHA1(46ab98dcdf6f5d611646a22a7355939ef5b2bbe5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.14m",   0x1000000, 0x400000, CRC(7f1df4e4) SHA1(ede92b31c1fe87f91b4fe74ac211f2fb5f863bc2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.16m",   0x1000002, 0x400000, CRC(90bd3203) SHA1(ed83208c486ea0f407b7e5d16a8cf242a6f73774) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.18m",   0x1000004, 0x400000, CRC(67aaf727) SHA1(e0e69104e31d2c41e18c0d24e9ab962406a7ca9a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.20m",   0x1000006, 0x400000, CRC(8b0bade8) SHA1(c5732361bb4bf284c4d12a82ac2c5750b1f9d441) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "mvc.01",   0x00000, 0x08000, CRC(41629e95) SHA1(36925c05b5fdcbe43283a882d021e5360c947061) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "mvc.02",   0x28000, 0x20000, CRC(963abf6b) SHA1(6b784870e338701cefabbbe4669984b5c4e8a9a5) )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "mvc.11m",   0x000000, 0x400000, CRC(850fe663) SHA1(81e519d05a08855f242ea2e17ee0859b449db895) )
	ROM_LOAD16_WORD_SWAP( "mvc.12m",   0x400000, 0x400000, CRC(7ccb1896) SHA1(74caadf3282fcc6acffb1bbe3734106f81124121) )
ROM_END

ROM_START( mvscj )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "mvcj.03a", 0x000000, 0x80000, CRC(3df18879) SHA1(2b91da6e5dd792967337e873ebb08ecf5194a97b) )
	ROM_LOAD16_WORD_SWAP( "mvcj.04a", 0x080000, 0x80000, CRC(07d212e8) SHA1(c5420e9bd580910c1f1d0264240aeef20aac30a7) )
	ROM_LOAD16_WORD_SWAP( "mvc.05a",  0x100000, 0x80000, CRC(2d8c8e86) SHA1(b07d640a734c5d336054ed05195786224c9a6cd4) )
	ROM_LOAD16_WORD_SWAP( "mvc.06a",  0x180000, 0x80000, CRC(8528e1f5) SHA1(cd065c05268ab581b05676da544baf6af642acac) )
	ROM_LOAD16_WORD_SWAP( "mvc.07",   0x200000, 0x80000, CRC(c3baa32b) SHA1(d35589847e0753e869ffcd7c3abed925bfdb0fa2) )
	ROM_LOAD16_WORD_SWAP( "mvc.08",   0x280000, 0x80000, CRC(bc002fcd) SHA1(0b6735a071a9274f7ab25c743271fc30411fe819) )
	ROM_LOAD16_WORD_SWAP( "mvc.09",   0x300000, 0x80000, CRC(c67b26df) SHA1(6e9969246c57269d7ba0992a5cc319c8910bf8a9) )
	ROM_LOAD16_WORD_SWAP( "mvc.10",   0x380000, 0x80000, CRC(0fdd1e26) SHA1(5fa684d823b4f4eec61ed9e9b8938af5272ae1ed) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "mvc.13m",   0x0000000, 0x400000, CRC(fa5f74bc) SHA1(79a619248938a85ce4f7794a704647b9cf564fbc) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.15m",   0x0000002, 0x400000, CRC(71938a8f) SHA1(6982f7203458c1c46a1c1c13c0d0f2a5e109d271) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.17m",   0x0000004, 0x400000, CRC(92741d07) SHA1(ddfd70eab7c983ab452194b1860059f8ad694459) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.19m",   0x0000006, 0x400000, CRC(bcb72fc6) SHA1(46ab98dcdf6f5d611646a22a7355939ef5b2bbe5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.14m",   0x1000000, 0x400000, CRC(7f1df4e4) SHA1(ede92b31c1fe87f91b4fe74ac211f2fb5f863bc2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.16m",   0x1000002, 0x400000, CRC(90bd3203) SHA1(ed83208c486ea0f407b7e5d16a8cf242a6f73774) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.18m",   0x1000004, 0x400000, CRC(67aaf727) SHA1(e0e69104e31d2c41e18c0d24e9ab962406a7ca9a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.20m",   0x1000006, 0x400000, CRC(8b0bade8) SHA1(c5732361bb4bf284c4d12a82ac2c5750b1f9d441) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "mvc.01",   0x00000, 0x08000, CRC(41629e95) SHA1(36925c05b5fdcbe43283a882d021e5360c947061) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "mvc.02",   0x28000, 0x20000, CRC(963abf6b) SHA1(6b784870e338701cefabbbe4669984b5c4e8a9a5) )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "mvc.11m",   0x000000, 0x400000, CRC(850fe663) SHA1(81e519d05a08855f242ea2e17ee0859b449db895) )
	ROM_LOAD16_WORD_SWAP( "mvc.12m",   0x400000, 0x400000, CRC(7ccb1896) SHA1(74caadf3282fcc6acffb1bbe3734106f81124121) )
ROM_END

ROM_START( mvscjr1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "mvcj.03", 0x000000, 0x80000, CRC(2164213f) SHA1(00e3500ed334bb80d4d159eacf439860a2bfc3b7) )
	ROM_LOAD16_WORD_SWAP( "mvcj.04", 0x080000, 0x80000, CRC(c905c86f) SHA1(965fa3bdc29bd901e9efcc53b195c6be3a74c9f9) )
	ROM_LOAD16_WORD_SWAP( "mvc.05",  0x100000, 0x80000, CRC(7db71ce9) SHA1(a0097109e9f4aba40791932269d600c0ffa099a7) )
	ROM_LOAD16_WORD_SWAP( "mvc.06",  0x180000, 0x80000, CRC(4b0b6d3e) SHA1(375372adf0a508bb6fc6a79326b2d4171db9ca0f) )
	ROM_LOAD16_WORD_SWAP( "mvc.07",  0x200000, 0x80000, CRC(c3baa32b) SHA1(d35589847e0753e869ffcd7c3abed925bfdb0fa2) )
	ROM_LOAD16_WORD_SWAP( "mvc.08",  0x280000, 0x80000, CRC(bc002fcd) SHA1(0b6735a071a9274f7ab25c743271fc30411fe819) )
	ROM_LOAD16_WORD_SWAP( "mvc.09",  0x300000, 0x80000, CRC(c67b26df) SHA1(6e9969246c57269d7ba0992a5cc319c8910bf8a9) )
	ROM_LOAD16_WORD_SWAP( "mvc.10",  0x380000, 0x80000, CRC(0fdd1e26) SHA1(5fa684d823b4f4eec61ed9e9b8938af5272ae1ed) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "mvc.13m",   0x0000000, 0x400000, CRC(fa5f74bc) SHA1(79a619248938a85ce4f7794a704647b9cf564fbc) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.15m",   0x0000002, 0x400000, CRC(71938a8f) SHA1(6982f7203458c1c46a1c1c13c0d0f2a5e109d271) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.17m",   0x0000004, 0x400000, CRC(92741d07) SHA1(ddfd70eab7c983ab452194b1860059f8ad694459) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.19m",   0x0000006, 0x400000, CRC(bcb72fc6) SHA1(46ab98dcdf6f5d611646a22a7355939ef5b2bbe5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.14m",   0x1000000, 0x400000, CRC(7f1df4e4) SHA1(ede92b31c1fe87f91b4fe74ac211f2fb5f863bc2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.16m",   0x1000002, 0x400000, CRC(90bd3203) SHA1(ed83208c486ea0f407b7e5d16a8cf242a6f73774) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.18m",   0x1000004, 0x400000, CRC(67aaf727) SHA1(e0e69104e31d2c41e18c0d24e9ab962406a7ca9a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.20m",   0x1000006, 0x400000, CRC(8b0bade8) SHA1(c5732361bb4bf284c4d12a82ac2c5750b1f9d441) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "mvc.01",   0x00000, 0x08000, CRC(41629e95) SHA1(36925c05b5fdcbe43283a882d021e5360c947061) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "mvc.02",   0x28000, 0x20000, CRC(963abf6b) SHA1(6b784870e338701cefabbbe4669984b5c4e8a9a5) )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "mvc.11m",   0x000000, 0x400000, CRC(850fe663) SHA1(81e519d05a08855f242ea2e17ee0859b449db895) )
	ROM_LOAD16_WORD_SWAP( "mvc.12m",   0x400000, 0x400000, CRC(7ccb1896) SHA1(74caadf3282fcc6acffb1bbe3734106f81124121) )
ROM_END

ROM_START( mvsca )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "mvca.03a", 0x000000, 0x80000, CRC(2ff4ae25) SHA1(74cc3656a265f912c72cc6e591de14791fa9a50d) )
	ROM_LOAD16_WORD_SWAP( "mvca.04a", 0x080000, 0x80000, CRC(f28427ef) SHA1(3e4c91753b19c6307abd0ad87a0184730a418efb) )
	ROM_LOAD16_WORD_SWAP( "mvc.05a",  0x100000, 0x80000, CRC(2d8c8e86) SHA1(b07d640a734c5d336054ed05195786224c9a6cd4) )
	ROM_LOAD16_WORD_SWAP( "mvc.06a",  0x180000, 0x80000, CRC(8528e1f5) SHA1(cd065c05268ab581b05676da544baf6af642acac) )
	ROM_LOAD16_WORD_SWAP( "mvc.07",   0x200000, 0x80000, CRC(c3baa32b) SHA1(d35589847e0753e869ffcd7c3abed925bfdb0fa2) )
	ROM_LOAD16_WORD_SWAP( "mvc.08",   0x280000, 0x80000, CRC(bc002fcd) SHA1(0b6735a071a9274f7ab25c743271fc30411fe819) )
	ROM_LOAD16_WORD_SWAP( "mvc.09",   0x300000, 0x80000, CRC(c67b26df) SHA1(6e9969246c57269d7ba0992a5cc319c8910bf8a9) )
	ROM_LOAD16_WORD_SWAP( "mvc.10",   0x380000, 0x80000, CRC(0fdd1e26) SHA1(5fa684d823b4f4eec61ed9e9b8938af5272ae1ed) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "mvc.13m",   0x0000000, 0x400000, CRC(fa5f74bc) SHA1(79a619248938a85ce4f7794a704647b9cf564fbc) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.15m",   0x0000002, 0x400000, CRC(71938a8f) SHA1(6982f7203458c1c46a1c1c13c0d0f2a5e109d271) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.17m",   0x0000004, 0x400000, CRC(92741d07) SHA1(ddfd70eab7c983ab452194b1860059f8ad694459) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.19m",   0x0000006, 0x400000, CRC(bcb72fc6) SHA1(46ab98dcdf6f5d611646a22a7355939ef5b2bbe5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.14m",   0x1000000, 0x400000, CRC(7f1df4e4) SHA1(ede92b31c1fe87f91b4fe74ac211f2fb5f863bc2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.16m",   0x1000002, 0x400000, CRC(90bd3203) SHA1(ed83208c486ea0f407b7e5d16a8cf242a6f73774) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.18m",   0x1000004, 0x400000, CRC(67aaf727) SHA1(e0e69104e31d2c41e18c0d24e9ab962406a7ca9a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.20m",   0x1000006, 0x400000, CRC(8b0bade8) SHA1(c5732361bb4bf284c4d12a82ac2c5750b1f9d441) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "mvc.01",   0x00000, 0x08000, CRC(41629e95) SHA1(36925c05b5fdcbe43283a882d021e5360c947061) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "mvc.02",   0x28000, 0x20000, CRC(963abf6b) SHA1(6b784870e338701cefabbbe4669984b5c4e8a9a5) )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "mvc.11m",   0x000000, 0x400000, CRC(850fe663) SHA1(81e519d05a08855f242ea2e17ee0859b449db895) )
	ROM_LOAD16_WORD_SWAP( "mvc.12m",   0x400000, 0x400000, CRC(7ccb1896) SHA1(74caadf3282fcc6acffb1bbe3734106f81124121) )
ROM_END

ROM_START( mvscar1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "mvca.03", 0x000000, 0x80000, CRC(fe5fa7b9) SHA1(c27b987ffb631c3433aa32a29989dbf2b3e53f1e) )
	ROM_LOAD16_WORD_SWAP( "mvca.04", 0x080000, 0x80000, CRC(082b701c) SHA1(363770ecd5f4e160db6448845ba6d7fd0beea291) )
	ROM_LOAD16_WORD_SWAP( "mvc.05",  0x100000, 0x80000, CRC(7db71ce9) SHA1(a0097109e9f4aba40791932269d600c0ffa099a7) )
	ROM_LOAD16_WORD_SWAP( "mvc.06",  0x180000, 0x80000, CRC(4b0b6d3e) SHA1(375372adf0a508bb6fc6a79326b2d4171db9ca0f) )
	ROM_LOAD16_WORD_SWAP( "mvc.07",  0x200000, 0x80000, CRC(c3baa32b) SHA1(d35589847e0753e869ffcd7c3abed925bfdb0fa2) )
	ROM_LOAD16_WORD_SWAP( "mvc.08",  0x280000, 0x80000, CRC(bc002fcd) SHA1(0b6735a071a9274f7ab25c743271fc30411fe819) )
	ROM_LOAD16_WORD_SWAP( "mvc.09",  0x300000, 0x80000, CRC(c67b26df) SHA1(6e9969246c57269d7ba0992a5cc319c8910bf8a9) )
	ROM_LOAD16_WORD_SWAP( "mvc.10",  0x380000, 0x80000, CRC(0fdd1e26) SHA1(5fa684d823b4f4eec61ed9e9b8938af5272ae1ed) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "mvc.13m",   0x0000000, 0x400000, CRC(fa5f74bc) SHA1(79a619248938a85ce4f7794a704647b9cf564fbc) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.15m",   0x0000002, 0x400000, CRC(71938a8f) SHA1(6982f7203458c1c46a1c1c13c0d0f2a5e109d271) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.17m",   0x0000004, 0x400000, CRC(92741d07) SHA1(ddfd70eab7c983ab452194b1860059f8ad694459) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.19m",   0x0000006, 0x400000, CRC(bcb72fc6) SHA1(46ab98dcdf6f5d611646a22a7355939ef5b2bbe5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.14m",   0x1000000, 0x400000, CRC(7f1df4e4) SHA1(ede92b31c1fe87f91b4fe74ac211f2fb5f863bc2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.16m",   0x1000002, 0x400000, CRC(90bd3203) SHA1(ed83208c486ea0f407b7e5d16a8cf242a6f73774) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.18m",   0x1000004, 0x400000, CRC(67aaf727) SHA1(e0e69104e31d2c41e18c0d24e9ab962406a7ca9a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.20m",   0x1000006, 0x400000, CRC(8b0bade8) SHA1(c5732361bb4bf284c4d12a82ac2c5750b1f9d441) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "mvc.01",   0x00000, 0x08000, CRC(41629e95) SHA1(36925c05b5fdcbe43283a882d021e5360c947061) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "mvc.02",   0x28000, 0x20000, CRC(963abf6b) SHA1(6b784870e338701cefabbbe4669984b5c4e8a9a5) )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "mvc.11m",   0x000000, 0x400000, CRC(850fe663) SHA1(81e519d05a08855f242ea2e17ee0859b449db895) )
	ROM_LOAD16_WORD_SWAP( "mvc.12m",   0x400000, 0x400000, CRC(7ccb1896) SHA1(74caadf3282fcc6acffb1bbe3734106f81124121) )
ROM_END

ROM_START( mvsch )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "mvch.03", 0x000000, 0x80000, CRC(6a0ec9f7) SHA1(62d7b28cc9ddf975ccdc8992d51bd3d085e3e136) )
	ROM_LOAD16_WORD_SWAP( "mvch.04", 0x080000, 0x80000, CRC(00f03fa4) SHA1(3a79400a7ac6e7594ca7e0fbb2486ddf6c759d18) )
	ROM_LOAD16_WORD_SWAP( "mvc.05a", 0x100000, 0x80000, CRC(2d8c8e86) SHA1(b07d640a734c5d336054ed05195786224c9a6cd4) )
	ROM_LOAD16_WORD_SWAP( "mvc.06a", 0x180000, 0x80000, CRC(8528e1f5) SHA1(cd065c05268ab581b05676da544baf6af642acac) )
	ROM_LOAD16_WORD_SWAP( "mvc.07",  0x200000, 0x80000, CRC(c3baa32b) SHA1(d35589847e0753e869ffcd7c3abed925bfdb0fa2) )
	ROM_LOAD16_WORD_SWAP( "mvc.08",  0x280000, 0x80000, CRC(bc002fcd) SHA1(0b6735a071a9274f7ab25c743271fc30411fe819) )
	ROM_LOAD16_WORD_SWAP( "mvc.09",  0x300000, 0x80000, CRC(c67b26df) SHA1(6e9969246c57269d7ba0992a5cc319c8910bf8a9) )
	ROM_LOAD16_WORD_SWAP( "mvc.10",  0x380000, 0x80000, CRC(0fdd1e26) SHA1(5fa684d823b4f4eec61ed9e9b8938af5272ae1ed) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "mvc.13m",   0x0000000, 0x400000, CRC(fa5f74bc) SHA1(79a619248938a85ce4f7794a704647b9cf564fbc) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.15m",   0x0000002, 0x400000, CRC(71938a8f) SHA1(6982f7203458c1c46a1c1c13c0d0f2a5e109d271) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.17m",   0x0000004, 0x400000, CRC(92741d07) SHA1(ddfd70eab7c983ab452194b1860059f8ad694459) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.19m",   0x0000006, 0x400000, CRC(bcb72fc6) SHA1(46ab98dcdf6f5d611646a22a7355939ef5b2bbe5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.14m",   0x1000000, 0x400000, CRC(7f1df4e4) SHA1(ede92b31c1fe87f91b4fe74ac211f2fb5f863bc2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.16m",   0x1000002, 0x400000, CRC(90bd3203) SHA1(ed83208c486ea0f407b7e5d16a8cf242a6f73774) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.18m",   0x1000004, 0x400000, CRC(67aaf727) SHA1(e0e69104e31d2c41e18c0d24e9ab962406a7ca9a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.20m",   0x1000006, 0x400000, CRC(8b0bade8) SHA1(c5732361bb4bf284c4d12a82ac2c5750b1f9d441) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "mvc.01",   0x00000, 0x08000, CRC(41629e95) SHA1(36925c05b5fdcbe43283a882d021e5360c947061) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "mvc.02",   0x28000, 0x20000, CRC(963abf6b) SHA1(6b784870e338701cefabbbe4669984b5c4e8a9a5) )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "mvc.11m",   0x000000, 0x400000, CRC(850fe663) SHA1(81e519d05a08855f242ea2e17ee0859b449db895) )
	ROM_LOAD16_WORD_SWAP( "mvc.12m",   0x400000, 0x400000, CRC(7ccb1896) SHA1(74caadf3282fcc6acffb1bbe3734106f81124121) )
ROM_END

ROM_START( mvscb )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "mvcb.03a", 0x000000, 0x80000, CRC(7155953b) SHA1(acd1f86bc24636a02aeef566994d38ffa6dbf72f) )
	ROM_LOAD16_WORD_SWAP( "mvcb.04a", 0x080000, 0x80000, CRC(fb117d0e) SHA1(c513f430e7c4f32e220a800c53ffdcea68c6cfcf) )
	ROM_LOAD16_WORD_SWAP( "mvc.05a",  0x100000, 0x80000, CRC(2d8c8e86) SHA1(b07d640a734c5d336054ed05195786224c9a6cd4) )
	ROM_LOAD16_WORD_SWAP( "mvc.06a",  0x180000, 0x80000, CRC(8528e1f5) SHA1(cd065c05268ab581b05676da544baf6af642acac) )
	ROM_LOAD16_WORD_SWAP( "mvc.07",   0x200000, 0x80000, CRC(c3baa32b) SHA1(d35589847e0753e869ffcd7c3abed925bfdb0fa2) )
	ROM_LOAD16_WORD_SWAP( "mvc.08",   0x280000, 0x80000, CRC(bc002fcd) SHA1(0b6735a071a9274f7ab25c743271fc30411fe819) )
	ROM_LOAD16_WORD_SWAP( "mvc.09",   0x300000, 0x80000, CRC(c67b26df) SHA1(6e9969246c57269d7ba0992a5cc319c8910bf8a9) )
	ROM_LOAD16_WORD_SWAP( "mvc.10",   0x380000, 0x80000, CRC(0fdd1e26) SHA1(5fa684d823b4f4eec61ed9e9b8938af5272ae1ed) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "mvc.13m",   0x0000000, 0x400000, CRC(fa5f74bc) SHA1(79a619248938a85ce4f7794a704647b9cf564fbc) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.15m",   0x0000002, 0x400000, CRC(71938a8f) SHA1(6982f7203458c1c46a1c1c13c0d0f2a5e109d271) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.17m",   0x0000004, 0x400000, CRC(92741d07) SHA1(ddfd70eab7c983ab452194b1860059f8ad694459) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.19m",   0x0000006, 0x400000, CRC(bcb72fc6) SHA1(46ab98dcdf6f5d611646a22a7355939ef5b2bbe5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.14m",   0x1000000, 0x400000, CRC(7f1df4e4) SHA1(ede92b31c1fe87f91b4fe74ac211f2fb5f863bc2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.16m",   0x1000002, 0x400000, CRC(90bd3203) SHA1(ed83208c486ea0f407b7e5d16a8cf242a6f73774) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.18m",   0x1000004, 0x400000, CRC(67aaf727) SHA1(e0e69104e31d2c41e18c0d24e9ab962406a7ca9a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.20m",   0x1000006, 0x400000, CRC(8b0bade8) SHA1(c5732361bb4bf284c4d12a82ac2c5750b1f9d441) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "mvc.01",   0x00000, 0x08000, CRC(41629e95) SHA1(36925c05b5fdcbe43283a882d021e5360c947061) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "mvc.02",   0x28000, 0x20000, CRC(963abf6b) SHA1(6b784870e338701cefabbbe4669984b5c4e8a9a5) )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "mvc.11m",   0x000000, 0x400000, CRC(850fe663) SHA1(81e519d05a08855f242ea2e17ee0859b449db895) )
	ROM_LOAD16_WORD_SWAP( "mvc.12m",   0x400000, 0x400000, CRC(7ccb1896) SHA1(74caadf3282fcc6acffb1bbe3734106f81124121) )
ROM_END

ROM_START( mpang )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "mpne.03c", 0x000000, 0x80000, CRC(fe16fc9f) SHA1(be22bf8a0abd53d2e7ebc12d3d0020bf799a25e9) )
	ROM_LOAD16_WORD_SWAP( "mpne.04c", 0x080000, 0x80000, CRC(2cc5ec22) SHA1(c188349c26a64bad325cfa218849ed1e94303087) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROMX_LOAD( "mpn-simm.01c",   0x0000000, 0x200000, CRC(388db66b) SHA1(7416cce3d0dbea71c92ea9f72f5536146f757b45) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "mpn-simm.01d",   0x0000001, 0x200000, CRC(aff1b494) SHA1(d376c02ce01e71a7707d3d3fe5b0ae59ce781686) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "mpn-simm.01a",   0x0000002, 0x200000, CRC(a9c4857b) SHA1(66f538105c710d1480141e48a15b1a760f5ce985) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "mpn-simm.01b",   0x0000003, 0x200000, CRC(f759df22) SHA1(1678e3e819dd808f3a6fdd52b7c933eac4777b5b) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "mpn-simm.03c",   0x0000004, 0x200000, CRC(dec6b720) SHA1(331776e1cba3fb82071e7c2195dc4ae27b3613a2) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "mpn-simm.03d",   0x0000005, 0x200000, CRC(f8774c18) SHA1(58e0ea4dd62e39bcfaa3a2be4ef08eb2f0bd3c00) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "mpn-simm.03a",   0x0000006, 0x200000, CRC(c2aea4ec) SHA1(f5e2a815fa802598611efa48e5de97e929155e77) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "mpn-simm.03b",   0x0000007, 0x200000, CRC(84d6dc33) SHA1(f5ababb479facc08c425381570644230c09334e7) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "mpn.01",   0x00000, 0x08000, CRC(90c7adb6) SHA1(a2653e977e5e0457b249e098e5ca0abc93dac336) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "mpn-simm.05a",   0x000000, 0x200000, CRC(318a2e21) SHA1(c573cd88f8279a062c73ef1d79cd8421dbdcd93e) ) // ROM on a simm
	ROM_LOAD16_WORD_SWAP( "mpn-simm.05b",   0x200000, 0x200000, CRC(5462f4e8) SHA1(299fbdab700e735e6395c5d9e3f079bb2e3dbd73) ) // ROM on a simm
ROM_END

ROM_START( mpangr1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "mpne.03b", 0x000000, 0x80000, CRC(6ef0f9b2) SHA1(fd1c76e151466fe27b02a6d07683fdd9c6a4816d) )
	ROM_LOAD16_WORD_SWAP( "mpne.04b", 0x080000, 0x80000, CRC(30a468bb) SHA1(f2f8f600a079ff050c9a0c12e1a6368943bdc536) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROMX_LOAD( "mpn-simm.01c",   0x0000000, 0x200000, CRC(388db66b) SHA1(7416cce3d0dbea71c92ea9f72f5536146f757b45) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "mpn-simm.01d",   0x0000001, 0x200000, CRC(aff1b494) SHA1(d376c02ce01e71a7707d3d3fe5b0ae59ce781686) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "mpn-simm.01a",   0x0000002, 0x200000, CRC(a9c4857b) SHA1(66f538105c710d1480141e48a15b1a760f5ce985) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "mpn-simm.01b",   0x0000003, 0x200000, CRC(f759df22) SHA1(1678e3e819dd808f3a6fdd52b7c933eac4777b5b) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "mpn-simm.03c",   0x0000004, 0x200000, CRC(dec6b720) SHA1(331776e1cba3fb82071e7c2195dc4ae27b3613a2) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "mpn-simm.03d",   0x0000005, 0x200000, CRC(f8774c18) SHA1(58e0ea4dd62e39bcfaa3a2be4ef08eb2f0bd3c00) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "mpn-simm.03a",   0x0000006, 0x200000, CRC(c2aea4ec) SHA1(f5e2a815fa802598611efa48e5de97e929155e77) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "mpn-simm.03b",   0x0000007, 0x200000, CRC(84d6dc33) SHA1(f5ababb479facc08c425381570644230c09334e7) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "mpn.01",   0x00000, 0x08000, CRC(90c7adb6) SHA1(a2653e977e5e0457b249e098e5ca0abc93dac336) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "mpn-simm.05a",   0x000000, 0x200000, CRC(318a2e21) SHA1(c573cd88f8279a062c73ef1d79cd8421dbdcd93e) ) // ROM on a simm
	ROM_LOAD16_WORD_SWAP( "mpn-simm.05b",   0x200000, 0x200000, CRC(5462f4e8) SHA1(299fbdab700e735e6395c5d9e3f079bb2e3dbd73) ) // ROM on a simm
ROM_END

ROM_START( mpangu )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "mpnu.03", 0x000000, 0x80000, CRC(6e7ed03c) SHA1(3562362d9573252d4d19dbfd0ec7e47e9eaa5f46) ) /* USA version, but has no "u" in label code */
	ROM_LOAD16_WORD_SWAP( "mpnu.04", 0x080000, 0x80000, CRC(de079131) SHA1(95da2a03cb642963aabfebd8337003961ca4db71) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_FILL(              0x000000, 0x800000, 0 )
	ROMX_LOAD( "mpn.13m",  0x800000, 0x200000, CRC(c5f123dc) SHA1(e459571416ab64f0280cda7dfa0d8836faa745c9) , ROM_GROUPWORD | ROM_SKIP(6) ) /* This version uses roms chips */
	ROMX_LOAD( "mpn.15m",  0x800002, 0x200000, CRC(8e033265) SHA1(d05ca8fb825423ff6b099c06aaa500c0a947454a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mpn.17m",  0x800004, 0x200000, CRC(cfcd73d2) SHA1(3415c063ac3632159df0fa74899bde8c90f4a2a1) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mpn.19m",  0x800006, 0x200000, CRC(2db1ffbc) SHA1(ca58c9359c2c629896c40e78c14907f562e2cdad) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "mpn.01",   0x00000, 0x08000, CRC(90c7adb6) SHA1(a2653e977e5e0457b249e098e5ca0abc93dac336) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "mpn.q1",   0x000000, 0x100000, CRC(d21c1f5a) SHA1(94cfcf01e656c0fb690e6204964ac70fbc89064d) ) /* This version uses roms chips */
	ROM_LOAD16_WORD_SWAP( "mpn.q2",   0x100000, 0x100000, CRC(d22090b1) SHA1(0ba65c0efb46af3cfb8ea3fe5087186248c57420) )
	ROM_LOAD16_WORD_SWAP( "mpn.q3",   0x200000, 0x100000, CRC(60aa5ef2) SHA1(03fa994d7f5b43e05e8417a8769e07c22548a27a) )
	ROM_LOAD16_WORD_SWAP( "mpn.q4",   0x300000, 0x100000, CRC(3a67d203) SHA1(7213364745d0c4c7fe11573afb9678483e24acb4) )
ROM_END

ROM_START( mpangj )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "mpnj.03a", 0x000000, 0x80000, CRC(bf597b1c) SHA1(0412e826eec7a9f3e70c84b64c9fbcecf7e0c56a) )
	ROM_LOAD16_WORD_SWAP( "mpnj.04a", 0x080000, 0x80000, CRC(f4a3ab0f) SHA1(2e54bbc95304827fcd24dab35e4895f4e6566be0) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROMX_LOAD( "mpn-simm.01c",   0x0000000, 0x200000, CRC(388db66b) SHA1(7416cce3d0dbea71c92ea9f72f5536146f757b45) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "mpn-simm.01d",   0x0000001, 0x200000, CRC(aff1b494) SHA1(d376c02ce01e71a7707d3d3fe5b0ae59ce781686) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "mpn-simm.01a",   0x0000002, 0x200000, CRC(a9c4857b) SHA1(66f538105c710d1480141e48a15b1a760f5ce985) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "mpn-simm.01b",   0x0000003, 0x200000, CRC(f759df22) SHA1(1678e3e819dd808f3a6fdd52b7c933eac4777b5b) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "mpn-simm.03c",   0x0000004, 0x200000, CRC(dec6b720) SHA1(331776e1cba3fb82071e7c2195dc4ae27b3613a2) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "mpn-simm.03d",   0x0000005, 0x200000, CRC(f8774c18) SHA1(58e0ea4dd62e39bcfaa3a2be4ef08eb2f0bd3c00) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "mpn-simm.03a",   0x0000006, 0x200000, CRC(c2aea4ec) SHA1(f5e2a815fa802598611efa48e5de97e929155e77) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "mpn-simm.03b",   0x0000007, 0x200000, CRC(84d6dc33) SHA1(f5ababb479facc08c425381570644230c09334e7) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "mpn.01",   0x00000, 0x08000, CRC(90c7adb6) SHA1(a2653e977e5e0457b249e098e5ca0abc93dac336) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "mpn-simm.05a",   0x000000, 0x200000, CRC(318a2e21) SHA1(c573cd88f8279a062c73ef1d79cd8421dbdcd93e) ) // ROM on a simm
	ROM_LOAD16_WORD_SWAP( "mpn-simm.05b",   0x200000, 0x200000, CRC(5462f4e8) SHA1(299fbdab700e735e6395c5d9e3f079bb2e3dbd73) ) // ROM on a simm
ROM_END

ROM_START( nwarr )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "vphe.03f", 0x000000, 0x80000, CRC(a922c44f) SHA1(aa06c7ee9b9878d999a21201f4ff3134cef1fff0) )
	ROM_LOAD16_WORD_SWAP( "vphe.04c", 0x080000, 0x80000, CRC(7312d890) SHA1(8cf2ff6eb2c13e308e65d7ac7a5f9664c44170df) )
	ROM_LOAD16_WORD_SWAP( "vphe.05d", 0x100000, 0x80000, CRC(cde8b506) SHA1(1c4f7fa6799be1aa444ed31464cddb79fee22928) )
	ROM_LOAD16_WORD_SWAP( "vphe.06c", 0x180000, 0x80000, CRC(be99e7d0) SHA1(863c713ba7b6120a50835ee7a0153021fbbcef81) )
	ROM_LOAD16_WORD_SWAP( "vphe.07b", 0x200000, 0x80000, CRC(69e0e60c) SHA1(8b9b7280dfbbb45a875d78d40703503ffce663bb) )
	ROM_LOAD16_WORD_SWAP( "vphe.08b", 0x280000, 0x80000, CRC(d95a3849) SHA1(9ec77670fb83cf9beba95439b709c138e800bcc8) )
	ROM_LOAD16_WORD_SWAP( "vphe.09b", 0x300000, 0x80000, CRC(9882561c) SHA1(cc6d4a50819cd4e6b8c39c60e7c8ce46ba0f05d8) )
	ROM_LOAD16_WORD_SWAP( "vphe.10b", 0x380000, 0x80000, CRC(976fa62f) SHA1(ede8cb58365795f7068f3be52639383e226cf751) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "vph.13m",   0x0000000, 0x400000, CRC(c51baf99) SHA1(2fb6642908e542e404391eb17392f8270e87bf48) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.15m",   0x0000002, 0x400000, CRC(3ce83c77) SHA1(93369b23c6d7d834297434691bb047ee3dd9539c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.17m",   0x0000004, 0x400000, CRC(4f2408e0) SHA1(cd49c6b3c7e6470c6058f98ccc5210b052bb13e2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.19m",   0x0000006, 0x400000, CRC(9ff60250) SHA1(d69ba4dc6bd37d003245f0cf3211d6e2623005b8) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.14m",   0x1000000, 0x400000, CRC(7a0e1add) SHA1(6b28a91bd59bba97886fdea30116a5b1071109ed) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.16m",   0x1000002, 0x400000, CRC(2f41ca75) SHA1(f4a67e60b62001e6fe75cb05b9c81040a8a09f54) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.18m",   0x1000004, 0x400000, CRC(64498eed) SHA1(d64e54a9ad1cbb927b7bac2eb16e1487834c5706) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.20m",   0x1000006, 0x400000, CRC(17f2433f) SHA1(0cbf8c96f92016fefb4a9c668ce5fd260342d712) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "vph.01",   0x00000, 0x08000, CRC(5045dcac) SHA1(fd1a6586fbdd48a707df1fa52309b4cf50e3cc4c) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "vph.02",   0x28000, 0x20000, CRC(86b60e59) SHA1(197d07ced8b9850729c83fa59b7afc283500bdee) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "vph.11m",   0x000000, 0x200000, CRC(e1837d33) SHA1(e3cb69f64767bacbec7286d0b4cd0ce7a0ba13d8) )
	ROM_LOAD16_WORD_SWAP( "vph.12m",   0x200000, 0x200000, CRC(fbd3cd90) SHA1(4813c25802ad71b77ca04fd8f3a86344f99f0d6a) )
ROM_END

ROM_START( nwarru )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "vphu.03f", 0x000000, 0x80000, CRC(85d6a359) SHA1(38f4bb5cf2e1e82ce9673b911329fdc8220ce0dc) )
	ROM_LOAD16_WORD_SWAP( "vphu.04c", 0x080000, 0x80000, CRC(cb7fce77) SHA1(85a8c8b1c71df0eee5f23e0bf28b2d95af2ce830) )
	ROM_LOAD16_WORD_SWAP( "vphu.05e", 0x100000, 0x80000, CRC(e08f2bba) SHA1(0f6d01a3b05085df23ead4c09c5363a1587b527e) )
	ROM_LOAD16_WORD_SWAP( "vphu.06c", 0x180000, 0x80000, CRC(08c04cdb) SHA1(b78d87631a13c26cc1580d2ecc0d137105c23f0a) )
	ROM_LOAD16_WORD_SWAP( "vphu.07b", 0x200000, 0x80000, CRC(b5a5ab19) SHA1(f7b35b8cba81f88a6bdfea7e2dc12eca480c276c) )
	ROM_LOAD16_WORD_SWAP( "vphu.08b", 0x280000, 0x80000, CRC(51bb20fb) SHA1(a98c569dd45b4bd2275f9bd1df060d6eaead53df) )
	ROM_LOAD16_WORD_SWAP( "vphu.09b", 0x300000, 0x80000, CRC(41a64205) SHA1(1f5af658b7c3fb09cab3dd10d6dc433a0605f81a) )
	ROM_LOAD16_WORD_SWAP( "vphu.10b", 0x380000, 0x80000, CRC(2b1d43ae) SHA1(fa9a456fe92783c7cb93ca231b24387cf56644d7) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "vph.13m",   0x0000000, 0x400000, CRC(c51baf99) SHA1(2fb6642908e542e404391eb17392f8270e87bf48) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.15m",   0x0000002, 0x400000, CRC(3ce83c77) SHA1(93369b23c6d7d834297434691bb047ee3dd9539c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.17m",   0x0000004, 0x400000, CRC(4f2408e0) SHA1(cd49c6b3c7e6470c6058f98ccc5210b052bb13e2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.19m",   0x0000006, 0x400000, CRC(9ff60250) SHA1(d69ba4dc6bd37d003245f0cf3211d6e2623005b8) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.14m",   0x1000000, 0x400000, CRC(7a0e1add) SHA1(6b28a91bd59bba97886fdea30116a5b1071109ed) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.16m",   0x1000002, 0x400000, CRC(2f41ca75) SHA1(f4a67e60b62001e6fe75cb05b9c81040a8a09f54) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.18m",   0x1000004, 0x400000, CRC(64498eed) SHA1(d64e54a9ad1cbb927b7bac2eb16e1487834c5706) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.20m",   0x1000006, 0x400000, CRC(17f2433f) SHA1(0cbf8c96f92016fefb4a9c668ce5fd260342d712) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "vph.01",   0x00000, 0x08000, CRC(5045dcac) SHA1(fd1a6586fbdd48a707df1fa52309b4cf50e3cc4c) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "vph.02",   0x28000, 0x20000, CRC(86b60e59) SHA1(197d07ced8b9850729c83fa59b7afc283500bdee) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "vph.11m",   0x000000, 0x200000, CRC(e1837d33) SHA1(e3cb69f64767bacbec7286d0b4cd0ce7a0ba13d8) )
	ROM_LOAD16_WORD_SWAP( "vph.12m",   0x200000, 0x200000, CRC(fbd3cd90) SHA1(4813c25802ad71b77ca04fd8f3a86344f99f0d6a) )
ROM_END

ROM_START( nwarrh )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "vphh.03d", 0x000000, 0x80000, CRC(6029c7be) SHA1(687c382d02d18ef5781c9d928e74f161461c2641) )
	ROM_LOAD16_WORD_SWAP( "vphh.04a", 0x080000, 0x80000, CRC(d26625ee) SHA1(2b415a28ee39949a1e80e7e65b89f3d707bdfae7) )
	ROM_LOAD16_WORD_SWAP( "vphh.05c", 0x100000, 0x80000, CRC(73ee0b8a) SHA1(d31ccb2cbc6133972f6dc76f0a40154368953625) )
	ROM_LOAD16_WORD_SWAP( "vphh.06a", 0x180000, 0x80000, CRC(a5b3a50a) SHA1(de382ab707eeb4ec7ffbc637611296ee35acdce1) )
	ROM_LOAD16_WORD_SWAP( "vphh.07",  0x200000, 0x80000, CRC(5fc2bdc1) SHA1(5936f2d3eb6becefa3ede98107eb03723555cc22) )
	ROM_LOAD16_WORD_SWAP( "vphh.08",  0x280000, 0x80000, CRC(e65588d9) SHA1(4b15009d5aa2d91736af1ee7c52d6b49cc696724) )
	ROM_LOAD16_WORD_SWAP( "vphh.09",  0x300000, 0x80000, CRC(a2ce6d63) SHA1(52aed61d0c7a6191016f1ec4b0a4372fbf55bf49) )
	ROM_LOAD16_WORD_SWAP( "vphh.10",  0x380000, 0x80000, CRC(e2f4f4b9) SHA1(8d3e857ccd4654d2801ce6830c0d556a81c4d433) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "vph.13m",   0x0000000, 0x400000, CRC(c51baf99) SHA1(2fb6642908e542e404391eb17392f8270e87bf48) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.15m",   0x0000002, 0x400000, CRC(3ce83c77) SHA1(93369b23c6d7d834297434691bb047ee3dd9539c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.17m",   0x0000004, 0x400000, CRC(4f2408e0) SHA1(cd49c6b3c7e6470c6058f98ccc5210b052bb13e2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.19m",   0x0000006, 0x400000, CRC(9ff60250) SHA1(d69ba4dc6bd37d003245f0cf3211d6e2623005b8) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.14m",   0x1000000, 0x400000, CRC(7a0e1add) SHA1(6b28a91bd59bba97886fdea30116a5b1071109ed) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.16m",   0x1000002, 0x400000, CRC(2f41ca75) SHA1(f4a67e60b62001e6fe75cb05b9c81040a8a09f54) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.18m",   0x1000004, 0x400000, CRC(64498eed) SHA1(d64e54a9ad1cbb927b7bac2eb16e1487834c5706) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.20m",   0x1000006, 0x400000, CRC(17f2433f) SHA1(0cbf8c96f92016fefb4a9c668ce5fd260342d712) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "vph.01",   0x00000, 0x08000, CRC(5045dcac) SHA1(fd1a6586fbdd48a707df1fa52309b4cf50e3cc4c) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "vph.02",   0x28000, 0x20000, CRC(86b60e59) SHA1(197d07ced8b9850729c83fa59b7afc283500bdee) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "vph.11m",   0x000000, 0x200000, CRC(e1837d33) SHA1(e3cb69f64767bacbec7286d0b4cd0ce7a0ba13d8) )
	ROM_LOAD16_WORD_SWAP( "vph.12m",   0x200000, 0x200000, CRC(fbd3cd90) SHA1(4813c25802ad71b77ca04fd8f3a86344f99f0d6a) )
ROM_END

ROM_START( nwarrb )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "vphb.03d", 0x000000, 0x80000, CRC(3a426d3f) SHA1(76d7c39c901aa768bb1600179509752d1fc0d558) )
	ROM_LOAD16_WORD_SWAP( "vphb.04a", 0x080000, 0x80000, CRC(51c4bb2f) SHA1(c885813ff13bfd251accf38da1bc0bd9c526e4c5) )
	ROM_LOAD16_WORD_SWAP( "vphb.05c", 0x100000, 0x80000, CRC(ac44d997) SHA1(b28e55d83a33e885125f2def76259d0ab21b0f9b) )
	ROM_LOAD16_WORD_SWAP( "vphb.06a", 0x180000, 0x80000, CRC(5072a5fe) SHA1(78b3f2ef8bc16441d0d977dbec2246c9f9b28dbc) )
	ROM_LOAD16_WORD_SWAP( "vphb.07",  0x200000, 0x80000, CRC(9b355192) SHA1(10b5542fcc0af936868af9abf70d3303be543f21) )
	ROM_LOAD16_WORD_SWAP( "vphb.08",  0x280000, 0x80000, CRC(42220f84) SHA1(f6ef52b1dff86c25852aa05be4a5b39995c26dd7) )
	ROM_LOAD16_WORD_SWAP( "vphb.09",  0x300000, 0x80000, CRC(029e015d) SHA1(441d0ea36484cbffe783cb0a1133537c09783022) )
	ROM_LOAD16_WORD_SWAP( "vphb.10",  0x380000, 0x80000, CRC(37b3ce37) SHA1(5919b44415e4d5b242fcdd69efd0ab1722e4da8c) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "vph.13m",   0x0000000, 0x400000, CRC(c51baf99) SHA1(2fb6642908e542e404391eb17392f8270e87bf48) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.15m",   0x0000002, 0x400000, CRC(3ce83c77) SHA1(93369b23c6d7d834297434691bb047ee3dd9539c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.17m",   0x0000004, 0x400000, CRC(4f2408e0) SHA1(cd49c6b3c7e6470c6058f98ccc5210b052bb13e2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.19m",   0x0000006, 0x400000, CRC(9ff60250) SHA1(d69ba4dc6bd37d003245f0cf3211d6e2623005b8) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.14m",   0x1000000, 0x400000, CRC(7a0e1add) SHA1(6b28a91bd59bba97886fdea30116a5b1071109ed) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.16m",   0x1000002, 0x400000, CRC(2f41ca75) SHA1(f4a67e60b62001e6fe75cb05b9c81040a8a09f54) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.18m",   0x1000004, 0x400000, CRC(64498eed) SHA1(d64e54a9ad1cbb927b7bac2eb16e1487834c5706) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.20m",   0x1000006, 0x400000, CRC(17f2433f) SHA1(0cbf8c96f92016fefb4a9c668ce5fd260342d712) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "vph.01",   0x00000, 0x08000, CRC(5045dcac) SHA1(fd1a6586fbdd48a707df1fa52309b4cf50e3cc4c) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "vph.02",   0x28000, 0x20000, CRC(86b60e59) SHA1(197d07ced8b9850729c83fa59b7afc283500bdee) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "vph.11m",   0x000000, 0x200000, CRC(e1837d33) SHA1(e3cb69f64767bacbec7286d0b4cd0ce7a0ba13d8) )
	ROM_LOAD16_WORD_SWAP( "vph.12m",   0x200000, 0x200000, CRC(fbd3cd90) SHA1(4813c25802ad71b77ca04fd8f3a86344f99f0d6a) )
ROM_END

ROM_START( nwarra )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "vpha.03b", 0x000000, 0x80000, CRC(0a70cdd6) SHA1(08d3f3892bfc8e741c59071150e6c3a8bf5ffbef) )
	ROM_LOAD16_WORD_SWAP( "vpha.04b", 0x080000, 0x80000, CRC(70ce62e4) SHA1(8bc9066071585805b6420bcc40eb03e641e3f027) )
	ROM_LOAD16_WORD_SWAP( "vpha.05b", 0x100000, 0x80000, CRC(5692a03f) SHA1(7c2ba1022ee4045d985477a6c9026140b7809234) )
	ROM_LOAD16_WORD_SWAP( "vpha.06b", 0x180000, 0x80000, CRC(b810fe66) SHA1(787198f546a0d3dbf0f2d54a4328aaaf4e3cdcae) )
	ROM_LOAD16_WORD_SWAP( "vpha.07b", 0x200000, 0x80000, CRC(1be264f3) SHA1(b08d0771963e6b8c1a2d32e6db3edd9bffea39e1) )
	ROM_LOAD16_WORD_SWAP( "vpha.08b", 0x280000, 0x80000, CRC(86f1ed52) SHA1(95095ef7037cf5f90f00b15f224e4b35e3ea675e) )
	ROM_LOAD16_WORD_SWAP( "vpha.09b", 0x300000, 0x80000, CRC(7e96bd0a) SHA1(f7750209b157a405710c0797e9eb30f980db582d) )
	ROM_LOAD16_WORD_SWAP( "vpha.10b", 0x380000, 0x80000, CRC(58bce2fd) SHA1(2df1460cb349d64ca84fd3372a9a64b6cdbe078f) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "vph.13m",   0x0000000, 0x400000, CRC(c51baf99) SHA1(2fb6642908e542e404391eb17392f8270e87bf48) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.15m",   0x0000002, 0x400000, CRC(3ce83c77) SHA1(93369b23c6d7d834297434691bb047ee3dd9539c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.17m",   0x0000004, 0x400000, CRC(4f2408e0) SHA1(cd49c6b3c7e6470c6058f98ccc5210b052bb13e2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.19m",   0x0000006, 0x400000, CRC(9ff60250) SHA1(d69ba4dc6bd37d003245f0cf3211d6e2623005b8) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.14m",   0x1000000, 0x400000, CRC(7a0e1add) SHA1(6b28a91bd59bba97886fdea30116a5b1071109ed) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.16m",   0x1000002, 0x400000, CRC(2f41ca75) SHA1(f4a67e60b62001e6fe75cb05b9c81040a8a09f54) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.18m",   0x1000004, 0x400000, CRC(64498eed) SHA1(d64e54a9ad1cbb927b7bac2eb16e1487834c5706) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.20m",   0x1000006, 0x400000, CRC(17f2433f) SHA1(0cbf8c96f92016fefb4a9c668ce5fd260342d712) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "vph.01",   0x00000, 0x08000, CRC(5045dcac) SHA1(fd1a6586fbdd48a707df1fa52309b4cf50e3cc4c) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "vph.02",   0x28000, 0x20000, CRC(86b60e59) SHA1(197d07ced8b9850729c83fa59b7afc283500bdee) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "vph.11m",   0x000000, 0x200000, CRC(e1837d33) SHA1(e3cb69f64767bacbec7286d0b4cd0ce7a0ba13d8) )
	ROM_LOAD16_WORD_SWAP( "vph.12m",   0x200000, 0x200000, CRC(fbd3cd90) SHA1(4813c25802ad71b77ca04fd8f3a86344f99f0d6a) )
ROM_END

ROM_START( vhuntj )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "vphj.03f", 0x000000, 0x80000, CRC(3de2e333) SHA1(2f9f756c5c91646625d70debd5b19b8dbd13a62f) )
	ROM_LOAD16_WORD_SWAP( "vphj.04c", 0x080000, 0x80000, CRC(c95cf304) SHA1(0544ab9d0f398b558e1119d94885058ad4a7d929) )
	ROM_LOAD16_WORD_SWAP( "vphj.05d", 0x100000, 0x80000, CRC(50de5ddd) SHA1(2bcc6c254ead06e9ea0a9ae4348195d3d55de277) )
	ROM_LOAD16_WORD_SWAP( "vphj.06c", 0x180000, 0x80000, CRC(ac3bd3d5) SHA1(c0aa04c43dba2876d97d95fffd4766a28193b300) )
	ROM_LOAD16_WORD_SWAP( "vphj.07b", 0x200000, 0x80000, CRC(0761309f) SHA1(7c6f9ec4d93ea9dbd634142558baaaf170cd4c76) )
	ROM_LOAD16_WORD_SWAP( "vphj.08b", 0x280000, 0x80000, CRC(5a5c2bf5) SHA1(296c6a5a0062b58bc71a297bc8b27eea099c8518) )
	ROM_LOAD16_WORD_SWAP( "vphj.09b", 0x300000, 0x80000, CRC(823d6d99) SHA1(17be75b2ebfbf60a2141aef67c386454d23565f2) )
	ROM_LOAD16_WORD_SWAP( "vphj.10b", 0x380000, 0x80000, CRC(32c7d8f0) SHA1(47075fa80ceff6adfa6cc58dbe32ed4ee01ba4fc) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "vph.13m",   0x0000000, 0x400000, CRC(c51baf99) SHA1(2fb6642908e542e404391eb17392f8270e87bf48) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.15m",   0x0000002, 0x400000, CRC(3ce83c77) SHA1(93369b23c6d7d834297434691bb047ee3dd9539c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.17m",   0x0000004, 0x400000, CRC(4f2408e0) SHA1(cd49c6b3c7e6470c6058f98ccc5210b052bb13e2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.19m",   0x0000006, 0x400000, CRC(9ff60250) SHA1(d69ba4dc6bd37d003245f0cf3211d6e2623005b8) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.14m",   0x1000000, 0x400000, CRC(7a0e1add) SHA1(6b28a91bd59bba97886fdea30116a5b1071109ed) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.16m",   0x1000002, 0x400000, CRC(2f41ca75) SHA1(f4a67e60b62001e6fe75cb05b9c81040a8a09f54) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.18m",   0x1000004, 0x400000, CRC(64498eed) SHA1(d64e54a9ad1cbb927b7bac2eb16e1487834c5706) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.20m",   0x1000006, 0x400000, CRC(17f2433f) SHA1(0cbf8c96f92016fefb4a9c668ce5fd260342d712) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "vph.01",   0x00000, 0x08000, CRC(5045dcac) SHA1(fd1a6586fbdd48a707df1fa52309b4cf50e3cc4c) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "vph.02",   0x28000, 0x20000, CRC(86b60e59) SHA1(197d07ced8b9850729c83fa59b7afc283500bdee) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "vph.11m",   0x000000, 0x200000, CRC(e1837d33) SHA1(e3cb69f64767bacbec7286d0b4cd0ce7a0ba13d8) )
	ROM_LOAD16_WORD_SWAP( "vph.12m",   0x200000, 0x200000, CRC(fbd3cd90) SHA1(4813c25802ad71b77ca04fd8f3a86344f99f0d6a) )
ROM_END

ROM_START( vhuntjr1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "vphj.03c", 0x000000, 0x80000, CRC(606b682a) SHA1(dd5c1b90a050e344b4f29fc468cfbd92aa392edf) )
	ROM_LOAD16_WORD_SWAP( "vphj.04b", 0x080000, 0x80000, CRC(a3b40393) SHA1(04929e24c14b40f55745e02f07ff6af61739a8b4) )
	ROM_LOAD16_WORD_SWAP( "vphj.05b", 0x100000, 0x80000, CRC(fccd5558) SHA1(a4d0f4cad7666e61c11e2571850df79b48332f28) )
	ROM_LOAD16_WORD_SWAP( "vphj.06b", 0x180000, 0x80000, CRC(07e10a73) SHA1(d3f4f8d52dc3230fb3e8986adb2f00909782160a) )
	ROM_LOAD16_WORD_SWAP( "vphj.07b", 0x200000, 0x80000, CRC(0761309f) SHA1(7c6f9ec4d93ea9dbd634142558baaaf170cd4c76) )
	ROM_LOAD16_WORD_SWAP( "vphj.08b", 0x280000, 0x80000, CRC(5a5c2bf5) SHA1(296c6a5a0062b58bc71a297bc8b27eea099c8518) )
	ROM_LOAD16_WORD_SWAP( "vphj.09b", 0x300000, 0x80000, CRC(823d6d99) SHA1(17be75b2ebfbf60a2141aef67c386454d23565f2) )
	ROM_LOAD16_WORD_SWAP( "vphj.10b", 0x380000, 0x80000, CRC(32c7d8f0) SHA1(47075fa80ceff6adfa6cc58dbe32ed4ee01ba4fc) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "vph.13m",   0x0000000, 0x400000, CRC(c51baf99) SHA1(2fb6642908e542e404391eb17392f8270e87bf48) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.15m",   0x0000002, 0x400000, CRC(3ce83c77) SHA1(93369b23c6d7d834297434691bb047ee3dd9539c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.17m",   0x0000004, 0x400000, CRC(4f2408e0) SHA1(cd49c6b3c7e6470c6058f98ccc5210b052bb13e2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.19m",   0x0000006, 0x400000, CRC(9ff60250) SHA1(d69ba4dc6bd37d003245f0cf3211d6e2623005b8) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.14m",   0x1000000, 0x400000, CRC(7a0e1add) SHA1(6b28a91bd59bba97886fdea30116a5b1071109ed) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.16m",   0x1000002, 0x400000, CRC(2f41ca75) SHA1(f4a67e60b62001e6fe75cb05b9c81040a8a09f54) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.18m",   0x1000004, 0x400000, CRC(64498eed) SHA1(d64e54a9ad1cbb927b7bac2eb16e1487834c5706) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.20m",   0x1000006, 0x400000, CRC(17f2433f) SHA1(0cbf8c96f92016fefb4a9c668ce5fd260342d712) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "vph.01",   0x00000, 0x08000, CRC(5045dcac) SHA1(fd1a6586fbdd48a707df1fa52309b4cf50e3cc4c) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "vph.02",   0x28000, 0x20000, CRC(86b60e59) SHA1(197d07ced8b9850729c83fa59b7afc283500bdee) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "vph.11m",   0x000000, 0x200000, CRC(e1837d33) SHA1(e3cb69f64767bacbec7286d0b4cd0ce7a0ba13d8) )
	ROM_LOAD16_WORD_SWAP( "vph.12m",   0x200000, 0x200000, CRC(fbd3cd90) SHA1(4813c25802ad71b77ca04fd8f3a86344f99f0d6a) )
ROM_END

ROM_START( vhuntjr2 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "vphj.03b", 0x000000, 0x80000, CRC(679c3fa9) SHA1(25c3f595e4d93c16ac483e4f9ba20ad714ecf4ef) )
	ROM_LOAD16_WORD_SWAP( "vphj.04a", 0x080000, 0x80000, CRC(eb6e71e4) SHA1(7a7cd34f7a70d87b817c0a4242844103db3e9f66) )
	ROM_LOAD16_WORD_SWAP( "vphj.05a", 0x100000, 0x80000, CRC(eaf634ea) SHA1(d46cb9d5172bb626396354ff2742d4394f0816f1) )
	ROM_LOAD16_WORD_SWAP( "vphj.06a", 0x180000, 0x80000, CRC(b70cc6be) SHA1(02fc8070bb75a2075de01b891249e6891687440a) )
	ROM_LOAD16_WORD_SWAP( "vphj.07a", 0x200000, 0x80000, CRC(46ab907d) SHA1(18215ff19e2b0c6505b5b5dfe24ef09fc8539ae5) )
	ROM_LOAD16_WORD_SWAP( "vphj.08a", 0x280000, 0x80000, CRC(1c00355e) SHA1(72b94b6c5a10ecd11169048d991bcb7550968cc9) )
	ROM_LOAD16_WORD_SWAP( "vphj.09a", 0x300000, 0x80000, CRC(026e6f82) SHA1(4dffda5e2bcd2fbc9084782e9a79ebd2be1338e7) )
	ROM_LOAD16_WORD_SWAP( "vphj.10a", 0x380000, 0x80000, CRC(aadfb3ea) SHA1(f42b76a98f657ba67aee69025476e8114acce4c5) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "vph.13m",   0x0000000, 0x400000, CRC(c51baf99) SHA1(2fb6642908e542e404391eb17392f8270e87bf48) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.15m",   0x0000002, 0x400000, CRC(3ce83c77) SHA1(93369b23c6d7d834297434691bb047ee3dd9539c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.17m",   0x0000004, 0x400000, CRC(4f2408e0) SHA1(cd49c6b3c7e6470c6058f98ccc5210b052bb13e2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.19m",   0x0000006, 0x400000, CRC(9ff60250) SHA1(d69ba4dc6bd37d003245f0cf3211d6e2623005b8) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.14m",   0x1000000, 0x400000, CRC(7a0e1add) SHA1(6b28a91bd59bba97886fdea30116a5b1071109ed) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.16m",   0x1000002, 0x400000, CRC(2f41ca75) SHA1(f4a67e60b62001e6fe75cb05b9c81040a8a09f54) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.18m",   0x1000004, 0x400000, CRC(64498eed) SHA1(d64e54a9ad1cbb927b7bac2eb16e1487834c5706) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.20m",   0x1000006, 0x400000, CRC(17f2433f) SHA1(0cbf8c96f92016fefb4a9c668ce5fd260342d712) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "vph.01",   0x00000, 0x08000, CRC(5045dcac) SHA1(fd1a6586fbdd48a707df1fa52309b4cf50e3cc4c) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "vph.02",   0x28000, 0x20000, CRC(86b60e59) SHA1(197d07ced8b9850729c83fa59b7afc283500bdee) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "vph.11m",   0x000000, 0x200000, CRC(e1837d33) SHA1(e3cb69f64767bacbec7286d0b4cd0ce7a0ba13d8) )
	ROM_LOAD16_WORD_SWAP( "vph.12m",   0x200000, 0x200000, CRC(fbd3cd90) SHA1(4813c25802ad71b77ca04fd8f3a86344f99f0d6a) )
ROM_END

ROM_START( progear )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "pgau.03", 0x000000, 0x80000, CRC(343a783e) SHA1(7ba8ae041b062767bf64328adf22ef100c38cdfd) )
	ROM_LOAD16_WORD_SWAP( "pgau.04", 0x080000, 0x80000, CRC(16208d79) SHA1(c477de7f31df44144a60d10dc4d933f3a7c20722) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROMX_LOAD( "pga-simm.01c",   0x0000000, 0x200000,  CRC(452f98b0) SHA1(a10e615c32098f6d25becd466da8faa967523a7b) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "pga-simm.01d",   0x0000001, 0x200000,  CRC(9e672092) SHA1(fce0b8b43a1c069262f4e3e81c1a04621e232c88) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "pga-simm.01a",   0x0000002, 0x200000,  CRC(ae9ddafe) SHA1(afbb26fed6cd0cb5c0099a10d35aeb453318c14d) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "pga-simm.01b",   0x0000003, 0x200000,  CRC(94d72d94) SHA1(df6a3fe49c008f73b160eb6f2a44dc371ff73cba) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "pga-simm.03c",   0x0000004, 0x200000,  CRC(48a1886d) SHA1(ebf44b42d784924e08a832a7e5f66a887bab244b) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "pga-simm.03d",   0x0000005, 0x200000,  CRC(172d7e37) SHA1(0eaedd24cd3fa87b6f35fbd63078d40c493c92d0) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "pga-simm.03a",   0x0000006, 0x200000,  CRC(9ee33d98) SHA1(85d1bd31940e35ac8c732165020881a2d65cd6b1) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "pga-simm.03b",   0x0000007, 0x200000,  CRC(848dee32) SHA1(c591288e86ad1624d0fe66563808af9fac786e64) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "pga.01",   0x00000, 0x08000, CRC(bdbfa992) SHA1(7c5496c1daaea6a7ab95c0b25625d325ec3427cc) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "pga-simm.05a",   0x000000, 0x200000, CRC(c0aac80c) SHA1(91784d35d4f7e113529bb5be6081b67094b150ea) ) // ROM on a simm
	ROM_LOAD16_WORD_SWAP( "pga-simm.05b",   0x200000, 0x200000, CRC(37a65d86) SHA1(374d562a4648734f82aa2ddb6d258e870896dd45) ) // ROM on a simm
	ROM_LOAD16_WORD_SWAP( "pga-simm.06a",   0x400000, 0x200000, CRC(d3f1e934) SHA1(5dcea28c873d0d472f5b94e07d97cd77ace2b252) ) // ROM on a simm
	ROM_LOAD16_WORD_SWAP( "pga-simm.06b",   0x600000, 0x200000, CRC(8b39489a) SHA1(fd790efaf37dc2c4c16f657941044e3e2d3c2711) ) // ROM on a simm
ROM_END

ROM_START( progearj )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "pgaj.03", 0x000000, 0x80000, CRC(06dbba54) SHA1(b0b808e9974c727bd187f2cdcba71a301b78c759) )
	ROM_LOAD16_WORD_SWAP( "pgaj.04", 0x080000, 0x80000, CRC(a1f1f1bc) SHA1(839cdc89d9483632883c185951c76deb4ff7657e) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROMX_LOAD( "pga-simm.01c",   0x0000000, 0x200000,  CRC(452f98b0) SHA1(a10e615c32098f6d25becd466da8faa967523a7b) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "pga-simm.01d",   0x0000001, 0x200000,  CRC(9e672092) SHA1(fce0b8b43a1c069262f4e3e81c1a04621e232c88) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "pga-simm.01a",   0x0000002, 0x200000,  CRC(ae9ddafe) SHA1(afbb26fed6cd0cb5c0099a10d35aeb453318c14d) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "pga-simm.01b",   0x0000003, 0x200000,  CRC(94d72d94) SHA1(df6a3fe49c008f73b160eb6f2a44dc371ff73cba) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "pga-simm.03c",   0x0000004, 0x200000,  CRC(48a1886d) SHA1(ebf44b42d784924e08a832a7e5f66a887bab244b) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "pga-simm.03d",   0x0000005, 0x200000,  CRC(172d7e37) SHA1(0eaedd24cd3fa87b6f35fbd63078d40c493c92d0) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "pga-simm.03a",   0x0000006, 0x200000,  CRC(9ee33d98) SHA1(85d1bd31940e35ac8c732165020881a2d65cd6b1) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "pga-simm.03b",   0x0000007, 0x200000,  CRC(848dee32) SHA1(c591288e86ad1624d0fe66563808af9fac786e64) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "pga.01",   0x00000, 0x08000, CRC(bdbfa992) SHA1(7c5496c1daaea6a7ab95c0b25625d325ec3427cc) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "pga-simm.05a",   0x000000, 0x200000, CRC(c0aac80c) SHA1(91784d35d4f7e113529bb5be6081b67094b150ea) ) // ROM on a simm
	ROM_LOAD16_WORD_SWAP( "pga-simm.05b",   0x200000, 0x200000, CRC(37a65d86) SHA1(374d562a4648734f82aa2ddb6d258e870896dd45) ) // ROM on a simm
	ROM_LOAD16_WORD_SWAP( "pga-simm.06a",   0x400000, 0x200000, CRC(d3f1e934) SHA1(5dcea28c873d0d472f5b94e07d97cd77ace2b252) ) // ROM on a simm
	ROM_LOAD16_WORD_SWAP( "pga-simm.06b",   0x600000, 0x200000, CRC(8b39489a) SHA1(fd790efaf37dc2c4c16f657941044e3e2d3c2711) ) // ROM on a simm
ROM_END

ROM_START( progeara )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "pgaa.03", 0x000000, 0x80000, CRC(25e6e2ce) SHA1(15b702ccc2bab7f2f9e7724ed83931c686763ffe) )
	ROM_LOAD16_WORD_SWAP( "pgaa.04", 0x080000, 0x80000, CRC(8104307e) SHA1(3ed134fcbf5c1c9f068f59b49b96d01a1ea33eeb) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROMX_LOAD( "pga-simm.01c",   0x0000000, 0x200000,  CRC(452f98b0) SHA1(a10e615c32098f6d25becd466da8faa967523a7b) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "pga-simm.01d",   0x0000001, 0x200000,  CRC(9e672092) SHA1(fce0b8b43a1c069262f4e3e81c1a04621e232c88) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "pga-simm.01a",   0x0000002, 0x200000,  CRC(ae9ddafe) SHA1(afbb26fed6cd0cb5c0099a10d35aeb453318c14d) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "pga-simm.01b",   0x0000003, 0x200000,  CRC(94d72d94) SHA1(df6a3fe49c008f73b160eb6f2a44dc371ff73cba) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "pga-simm.03c",   0x0000004, 0x200000,  CRC(48a1886d) SHA1(ebf44b42d784924e08a832a7e5f66a887bab244b) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "pga-simm.03d",   0x0000005, 0x200000,  CRC(172d7e37) SHA1(0eaedd24cd3fa87b6f35fbd63078d40c493c92d0) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "pga-simm.03a",   0x0000006, 0x200000,  CRC(9ee33d98) SHA1(85d1bd31940e35ac8c732165020881a2d65cd6b1) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "pga-simm.03b",   0x0000007, 0x200000,  CRC(848dee32) SHA1(c591288e86ad1624d0fe66563808af9fac786e64) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "pga.01",   0x00000, 0x08000, CRC(bdbfa992) SHA1(7c5496c1daaea6a7ab95c0b25625d325ec3427cc) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "pga-simm.05a",   0x000000, 0x200000, CRC(c0aac80c) SHA1(91784d35d4f7e113529bb5be6081b67094b150ea) ) // ROM on a simm
	ROM_LOAD16_WORD_SWAP( "pga-simm.05b",   0x200000, 0x200000, CRC(37a65d86) SHA1(374d562a4648734f82aa2ddb6d258e870896dd45) ) // ROM on a simm
	ROM_LOAD16_WORD_SWAP( "pga-simm.06a",   0x400000, 0x200000, CRC(d3f1e934) SHA1(5dcea28c873d0d472f5b94e07d97cd77ace2b252) ) // ROM on a simm
	ROM_LOAD16_WORD_SWAP( "pga-simm.06b",   0x600000, 0x200000, CRC(8b39489a) SHA1(fd790efaf37dc2c4c16f657941044e3e2d3c2711) ) // ROM on a simm
ROM_END

ROM_START( pzloop2 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "pl2e.03", 0x000000, 0x80000, CRC(3b1285b2) SHA1(f90f98fb15068306a57109ad954845be0a99e8ab) )
	ROM_LOAD16_WORD_SWAP( "pl2e.04", 0x080000, 0x80000, CRC(40a2d647) SHA1(d1c5fa87b368efe0d2cc3f614d0165bd95748b81) )
	ROM_LOAD16_WORD_SWAP( "pl2e.05", 0x100000, 0x80000, CRC(0f11d818) SHA1(ca2d5ea892aebfa1a2a825fb45c57b7923936917) )
	ROM_LOAD16_WORD_SWAP( "pl2e.06", 0x180000, 0x80000, CRC(86fbbdf4) SHA1(325ba5dc54f3d82f39e1299d5f27de7227a458b2) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROMX_LOAD( "pl2-simm.01c",   0x0000000, 0x200000, CRC(137b13a7) SHA1(a1ca1bc8699ddfc54d5de1b39a9db9a5ac8b12e5) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "pl2-simm.01d",   0x0000001, 0x200000, CRC(a2db1507) SHA1(61c84c8d698a846d54a571b5f7b4824e22136db7) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "pl2-simm.01a",   0x0000002, 0x200000, CRC(7e80ff8e) SHA1(afcebfa995ace8b8973e75f1589980c5c4535bca) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "pl2-simm.01b",   0x0000003, 0x200000, CRC(cd93e6ed) SHA1(e4afce48fe481d8291ed2475d5de446afad65351) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "pl2-simm.03c",   0x0000004, 0x200000, CRC(0f52bbca) SHA1(e76c29d445062f5e16d06bdc4ab44640ba35aaac) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "pl2-simm.03d",   0x0000005, 0x200000, CRC(a62712c3) SHA1(2abfe0209e188010a0ae969f0d9eb7d28820b3f2) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "pl2-simm.03a",   0x0000006, 0x200000, CRC(b60c9f8e) SHA1(40c7985e04463fb2bd59b3bb6aa5897328d37ff3) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "pl2-simm.03b",   0x0000007, 0x200000, CRC(83fef284) SHA1(ef4429f54c456d6485a7d642d49dffafef4435fe) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "pl2.01",   0x00000, 0x08000, CRC(35697569) SHA1(13718923cffb9ec53cef9e22d8875370b5f3dd74) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "pl2-simm.05a",   0x000000, 0x200000, CRC(85d8fbe8) SHA1(c19d5e9084d07e610379b6e1b6be7bdf0b9b7f7f) ) // ROM on a simm
	ROM_LOAD16_WORD_SWAP( "pl2-simm.05b",   0x200000, 0x200000, CRC(1ed62584) SHA1(28411f610f48cca6424a2d53e2a4ac691e826317) ) // ROM on a simm
ROM_END

ROM_START( pzloop2j )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "pl2j.03a", 0x000000, 0x80000, CRC(0a751bd0) SHA1(a5a0b60387aacdafdf46ecd1acd764c9cb086b90) )
	ROM_LOAD16_WORD_SWAP( "pl2j.04a", 0x080000, 0x80000, CRC(c3f72afe) SHA1(597a302e4bba50193c53f239e715962fcc4e3e5e) )
	ROM_LOAD16_WORD_SWAP( "pl2j.05a", 0x100000, 0x80000, CRC(6ea9dbfc) SHA1(c3065e02516755e8b94a741dd2ab960c96d0ff8c) )
	ROM_LOAD16_WORD_SWAP( "pl2j.06a", 0x180000, 0x80000, CRC(0f14848d) SHA1(94a3ee00d65cd9a310b3a330e2c37467b5863c64) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROMX_LOAD( "pl2-simm.01c",   0x0000000, 0x200000, CRC(137b13a7) SHA1(a1ca1bc8699ddfc54d5de1b39a9db9a5ac8b12e5) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "pl2-simm.01d",   0x0000001, 0x200000, CRC(a2db1507) SHA1(61c84c8d698a846d54a571b5f7b4824e22136db7) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "pl2-simm.01a",   0x0000002, 0x200000, CRC(7e80ff8e) SHA1(afcebfa995ace8b8973e75f1589980c5c4535bca) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "pl2-simm.01b",   0x0000003, 0x200000, CRC(cd93e6ed) SHA1(e4afce48fe481d8291ed2475d5de446afad65351) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "pl2-simm.03c",   0x0000004, 0x200000, CRC(0f52bbca) SHA1(e76c29d445062f5e16d06bdc4ab44640ba35aaac) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "pl2-simm.03d",   0x0000005, 0x200000, CRC(a62712c3) SHA1(2abfe0209e188010a0ae969f0d9eb7d28820b3f2) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "pl2-simm.03a",   0x0000006, 0x200000, CRC(b60c9f8e) SHA1(40c7985e04463fb2bd59b3bb6aa5897328d37ff3) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "pl2-simm.03b",   0x0000007, 0x200000, CRC(83fef284) SHA1(ef4429f54c456d6485a7d642d49dffafef4435fe) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "pl2.01",   0x00000, 0x08000, CRC(35697569) SHA1(13718923cffb9ec53cef9e22d8875370b5f3dd74) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "pl2-simm.05a",   0x000000, 0x200000, CRC(85d8fbe8) SHA1(c19d5e9084d07e610379b6e1b6be7bdf0b9b7f7f) ) // ROM on a simm
	ROM_LOAD16_WORD_SWAP( "pl2-simm.05b",   0x200000, 0x200000, CRC(1ed62584) SHA1(28411f610f48cca6424a2d53e2a4ac691e826317) ) // ROM on a simm
ROM_END

ROM_START( qndream )
	ROM_REGION(CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "tqzj.03a", 0x000000, 0x80000, CRC(7acf3e30) SHA1(5e2a697f98185731afc4130286a2699033dd02af) )
	ROM_LOAD16_WORD_SWAP( "tqzj.04",  0x080000, 0x80000, CRC(f1044a87) SHA1(3fd6e5dd6be8c037c8a77cb840bf7d387497a98b) )
	ROM_LOAD16_WORD_SWAP( "tqzj.05",  0x100000, 0x80000, CRC(4105ba0e) SHA1(73aacdf4176029f8e21506319e41ce03ed480122) )
	ROM_LOAD16_WORD_SWAP( "tqzj.06",  0x180000, 0x80000, CRC(c371e8a5) SHA1(5a93e46e46acfdc93fcb069e2426627e948655bf) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_FILL(              0x000000, 0x800000, 0 )
	ROMX_LOAD( "tqz.14m",  0x800000, 0x200000, CRC(98af88a2) SHA1(d3620faf2162a1f3a62a238715da4da429376d3c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "tqz.16m",  0x800002, 0x200000, CRC(df82d491) SHA1(fd3c8303cbcacb132a90398ff61f47d2d68157ae) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "tqz.18m",  0x800004, 0x200000, CRC(42f132ff) SHA1(0e0a128524010dba033a9b9ab2c56fe92a1767da) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "tqz.20m",  0x800006, 0x200000, CRC(b2e128a3) SHA1(8ae3161749d5206f16b755c29466cd5ca249b665) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION(QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "tqz.01",   0x00000, 0x08000, CRC(e9ce9d0a) SHA1(29f2987788e914e0a55f9130a99e411d15a7cc9b) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "tqz.11m",   0x000000, 0x200000, CRC(78e7884f) SHA1(82fbbf704ac4bc0e0e7a6f407686861aa3693c23) )
	ROM_LOAD16_WORD_SWAP( "tqz.12m",   0x200000, 0x200000, CRC(2e049b13) SHA1(e026f444b905e679e8240c7dd371658c4a3fd713) )
ROM_END

ROM_START( ringdest )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "smbe.03b", 0x000000, 0x80000, CRC(b8016278) SHA1(f744b08b27c11b8567ca7a94fbd75e398563c008) )
	ROM_LOAD16_WORD_SWAP( "smbe.04b", 0x080000, 0x80000, CRC(18c4c447) SHA1(3723ad6d6939fa1ac7dd016254b32017b5e7b24e) )
	ROM_LOAD16_WORD_SWAP( "smbe.05b", 0x100000, 0x80000, CRC(18ebda7f) SHA1(eacc3e76af5c47abe0a778be7a7beacf0924884e) )
	ROM_LOAD16_WORD_SWAP( "smbe.06b", 0x180000, 0x80000, CRC(89c80007) SHA1(4c85aa5b224fdbb64f719a7b8b5b2e7413107c70) )
	ROM_LOAD16_WORD_SWAP( "smb.07",   0x200000, 0x80000, CRC(b9a11577) SHA1(e9b58ef8acd1fedd3c9e0a3489593c7e931106c0) )
	ROM_LOAD16_WORD_SWAP( "smb.08",   0x280000, 0x80000, CRC(f931b76b) SHA1(0b7e8d1278dcba89f0063bd09cda96d6ae1bc282) )

	ROM_REGION( 0x1200000, "gfx", 0 )
	ROMX_LOAD( "smb.13m",   0x0000000, 0x200000, CRC(d9b2d1de) SHA1(e8658983070dadcd1300a680a42c8431579e2b4f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "smb.15m",   0x0000002, 0x200000, CRC(9a766d92) SHA1(afdf88afbec527268d63c11ea32f861b52e11489) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "smb.17m",   0x0000004, 0x200000, CRC(51800f0f) SHA1(9526cd69a23340a81841271b51de03d9bf2b979f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "smb.19m",   0x0000006, 0x200000, CRC(35757e96) SHA1(c915f3b9e4fdec3defc7eecb2c1f7377e6072228) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "smb.14m",   0x0800000, 0x200000, CRC(e5bfd0e7) SHA1(327e626df4c2152f921f15535c01dda6c4437527) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "smb.16m",   0x0800002, 0x200000, CRC(c56c0866) SHA1(1e2218e852ae72a9a95861dd37129fe78d4b1329) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "smb.18m",   0x0800004, 0x200000, CRC(4ded3910) SHA1(d883541ce4d83f4e7ab095f2ef273408d9911f9a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "smb.20m",   0x0800006, 0x200000, CRC(26ea1ec5) SHA1(22be249b1f73272feacf4026f09fc877f5d86353) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "smb.21m",   0x1000000, 0x080000, CRC(0a08c5fc) SHA1(ff3fad4fbc98e3013291c7ba7ee5e057a2628b36) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "smb.23m",   0x1000002, 0x080000, CRC(0911b6c4) SHA1(e7a7061b192658724d98cae8693f63dd5bc40c00) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "smb.25m",   0x1000004, 0x080000, CRC(82d6c4ec) SHA1(ed8ed02a00f59a048b9891ec2a77720bb6a5e03d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "smb.27m",   0x1000006, 0x080000, CRC(9b48678b) SHA1(4fa300d356c538947983ae85bb5c5cfd1fb835e7) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "smb.01",   0x00000, 0x08000, CRC(0abc229a) SHA1(967f574e6358dfc1b01e6a4a4df1a8f34eb3d814) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "smb.02",   0x28000, 0x20000, CRC(d051679a) SHA1(583c2521a30db1740d95dd94a38751fbeff3aae5) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "smb.11m",   0x000000, 0x200000, CRC(c56935f9) SHA1(ca1705e48e31ddc13505e6297bceca2bec1bb209) )
	ROM_LOAD16_WORD_SWAP( "smb.12m",   0x200000, 0x200000, CRC(955b0782) SHA1(ee09500e7b44e923126533613bfe26cdabc7ab5f) )
ROM_END

ROM_START( smbomb )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "smbj.03a", 0x000000, 0x80000, CRC(1c5613de) SHA1(e6257078ad2e18537aa606b0d0c5e04806244386) )
	ROM_LOAD16_WORD_SWAP( "smbj.04a", 0x080000, 0x80000, CRC(29071ed7) SHA1(eb438fcb42e3fbe38e20bc021be079a3dd7a89fa) )
	ROM_LOAD16_WORD_SWAP( "smbj.05a", 0x100000, 0x80000, CRC(eb20bce4) SHA1(b78a447d3d1d3f9a62a6b5abcd893f5e091f1bbc) )
	ROM_LOAD16_WORD_SWAP( "smbj.06a", 0x180000, 0x80000, CRC(94b420cd) SHA1(4cc43d3f7fed224443e26df5b0076bd24e6cd04b) )
	ROM_LOAD16_WORD_SWAP( "smb.07",  0x200000, 0x80000, CRC(b9a11577) SHA1(e9b58ef8acd1fedd3c9e0a3489593c7e931106c0) )
	ROM_LOAD16_WORD_SWAP( "smb.08",  0x280000, 0x80000, CRC(f931b76b) SHA1(0b7e8d1278dcba89f0063bd09cda96d6ae1bc282) )

	ROM_REGION( 0x1200000, "gfx", 0 )
	ROMX_LOAD( "smb.13m",   0x0000000, 0x200000, CRC(d9b2d1de) SHA1(e8658983070dadcd1300a680a42c8431579e2b4f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "smb.15m",   0x0000002, 0x200000, CRC(9a766d92) SHA1(afdf88afbec527268d63c11ea32f861b52e11489) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "smb.17m",   0x0000004, 0x200000, CRC(51800f0f) SHA1(9526cd69a23340a81841271b51de03d9bf2b979f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "smb.19m",   0x0000006, 0x200000, CRC(35757e96) SHA1(c915f3b9e4fdec3defc7eecb2c1f7377e6072228) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "smb.14m",   0x0800000, 0x200000, CRC(e5bfd0e7) SHA1(327e626df4c2152f921f15535c01dda6c4437527) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "smb.16m",   0x0800002, 0x200000, CRC(c56c0866) SHA1(1e2218e852ae72a9a95861dd37129fe78d4b1329) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "smb.18m",   0x0800004, 0x200000, CRC(4ded3910) SHA1(d883541ce4d83f4e7ab095f2ef273408d9911f9a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "smb.20m",   0x0800006, 0x200000, CRC(26ea1ec5) SHA1(22be249b1f73272feacf4026f09fc877f5d86353) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "smb.21m",   0x1000000, 0x080000, CRC(0a08c5fc) SHA1(ff3fad4fbc98e3013291c7ba7ee5e057a2628b36) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "smb.23m",   0x1000002, 0x080000, CRC(0911b6c4) SHA1(e7a7061b192658724d98cae8693f63dd5bc40c00) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "smb.25m",   0x1000004, 0x080000, CRC(82d6c4ec) SHA1(ed8ed02a00f59a048b9891ec2a77720bb6a5e03d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "smb.27m",   0x1000006, 0x080000, CRC(9b48678b) SHA1(4fa300d356c538947983ae85bb5c5cfd1fb835e7) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "smb.01",   0x00000, 0x08000, CRC(0abc229a) SHA1(967f574e6358dfc1b01e6a4a4df1a8f34eb3d814) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "smb.02",   0x28000, 0x20000, CRC(d051679a) SHA1(583c2521a30db1740d95dd94a38751fbeff3aae5) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "smb.11m",   0x000000, 0x200000, CRC(c56935f9) SHA1(ca1705e48e31ddc13505e6297bceca2bec1bb209) )
	ROM_LOAD16_WORD_SWAP( "smb.12m",   0x200000, 0x200000, CRC(955b0782) SHA1(ee09500e7b44e923126533613bfe26cdabc7ab5f) )
ROM_END

ROM_START( smbombr1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "smbj.03", 0x000000, 0x80000, CRC(52eafb10) SHA1(5abfe07e948748eba982dc8f2e21462aec187590) )
	ROM_LOAD16_WORD_SWAP( "smbj.04", 0x080000, 0x80000, CRC(aa6e8078) SHA1(58b4e15e7e3209e59a37ce48d8b9f0dc8b933cdc) )
	ROM_LOAD16_WORD_SWAP( "smbj.05", 0x100000, 0x80000, CRC(b69e7d5f) SHA1(e66430ef05ed0d1c848d24c7436ee5f1b511dcea) )
	ROM_LOAD16_WORD_SWAP( "smbj.06", 0x180000, 0x80000, CRC(8d857b56) SHA1(48c4e5f195e4343a8b7b9ec496fa1a77d659f72e) )
	ROM_LOAD16_WORD_SWAP( "smb.07",  0x200000, 0x80000, CRC(b9a11577) SHA1(e9b58ef8acd1fedd3c9e0a3489593c7e931106c0) )
	ROM_LOAD16_WORD_SWAP( "smb.08",  0x280000, 0x80000, CRC(f931b76b) SHA1(0b7e8d1278dcba89f0063bd09cda96d6ae1bc282) )

	ROM_REGION( 0x1200000, "gfx", 0 )
	ROMX_LOAD( "smb.13m",   0x0000000, 0x200000, CRC(d9b2d1de) SHA1(e8658983070dadcd1300a680a42c8431579e2b4f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "smb.15m",   0x0000002, 0x200000, CRC(9a766d92) SHA1(afdf88afbec527268d63c11ea32f861b52e11489) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "smb.17m",   0x0000004, 0x200000, CRC(51800f0f) SHA1(9526cd69a23340a81841271b51de03d9bf2b979f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "smb.19m",   0x0000006, 0x200000, CRC(35757e96) SHA1(c915f3b9e4fdec3defc7eecb2c1f7377e6072228) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "smb.14m",   0x0800000, 0x200000, CRC(e5bfd0e7) SHA1(327e626df4c2152f921f15535c01dda6c4437527) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "smb.16m",   0x0800002, 0x200000, CRC(c56c0866) SHA1(1e2218e852ae72a9a95861dd37129fe78d4b1329) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "smb.18m",   0x0800004, 0x200000, CRC(4ded3910) SHA1(d883541ce4d83f4e7ab095f2ef273408d9911f9a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "smb.20m",   0x0800006, 0x200000, CRC(26ea1ec5) SHA1(22be249b1f73272feacf4026f09fc877f5d86353) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "smb.21m",   0x1000000, 0x080000, CRC(0a08c5fc) SHA1(ff3fad4fbc98e3013291c7ba7ee5e057a2628b36) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "smb.23m",   0x1000002, 0x080000, CRC(0911b6c4) SHA1(e7a7061b192658724d98cae8693f63dd5bc40c00) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "smb.25m",   0x1000004, 0x080000, CRC(82d6c4ec) SHA1(ed8ed02a00f59a048b9891ec2a77720bb6a5e03d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "smb.27m",   0x1000006, 0x080000, CRC(9b48678b) SHA1(4fa300d356c538947983ae85bb5c5cfd1fb835e7) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "smb.01",   0x00000, 0x08000, CRC(0abc229a) SHA1(967f574e6358dfc1b01e6a4a4df1a8f34eb3d814) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "smb.02",   0x28000, 0x20000, CRC(d051679a) SHA1(583c2521a30db1740d95dd94a38751fbeff3aae5) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "smb.11m",   0x000000, 0x200000, CRC(c56935f9) SHA1(ca1705e48e31ddc13505e6297bceca2bec1bb209) )
	ROM_LOAD16_WORD_SWAP( "smb.12m",   0x200000, 0x200000, CRC(955b0782) SHA1(ee09500e7b44e923126533613bfe26cdabc7ab5f) )
ROM_END

ROM_START( mmancp2u )
	ROM_REGION(CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "rcmu.03b", 0x000000, 0x80000, CRC(c39f037f) SHA1(eefc5aa0fde322c6f895a37399424060c702a459) )
	ROM_LOAD16_WORD_SWAP( "rcmu.04b", 0x080000, 0x80000, CRC(cd6f5e99) SHA1(46d5298bdf7dd3ccfe5d491c61f3c2e2da011e3b) )
	ROM_LOAD16_WORD_SWAP( "rcm.05b",  0x100000, 0x80000, CRC(4376ea95) SHA1(7370ceffca513aa9f68a74f6869d561476589200) ) // == rcma_21a.rom from CPS1 version

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_FILL(              0x000000, 0x800000, 0 )
	ROMX_LOAD( "rcm.73",   0x800000, 0x080000, CRC(774c6e04) SHA1(6bd14960218e31d5a043b20a1b9d2d69cace761e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm.63",   0x800002, 0x080000, CRC(acad7c62) SHA1(eec10990339c9fd8fdae896a5fd98d5bf0220ed1) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm.83",   0x800004, 0x080000, CRC(6af30499) SHA1(a97bf2f382b6edc1e920e18d8ad5ca18131a2f21) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm.93",   0x800006, 0x080000, CRC(7a5a5166) SHA1(2160015e2e43e2c024a3af56da961ac02ffc74e9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm.74",   0xa00000, 0x080000, CRC(004ec725) SHA1(b1d3bcf920b0a1d0f4b59c77f5962e8162bbef65) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm.64",   0xa00002, 0x080000, CRC(65c0464e) SHA1(4a035f4d28fc8aa72bfe0a06392b93b0abfba458) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm.84",   0xa00004, 0x080000, CRC(fb3097cc) SHA1(7bff2372809cdfa4dcd8537a448b177c0e86a94e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm.94",   0xa00006, 0x080000, CRC(2e16557a) SHA1(f4a916c1524a8de23fc6afabc8c724a89530c631) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm.75",   0xc00000, 0x080000, CRC(70a73f99) SHA1(6dd126b4e64e34d5911ed5877b1b94b503404249) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm.65",   0xc00002, 0x080000, CRC(ecedad3d) SHA1(dea1377f086ea3a45ced983e258beb6607b295c9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm.85",   0xc00004, 0x080000, CRC(3d6186d8) SHA1(70f1ff678dd1ec533360458a564953fc5634cbc5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm.95",   0xc00006, 0x080000, CRC(8c7700f1) SHA1(84846af535e9e333e911486d545988568554b67b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm.76",   0xe00000, 0x080000, CRC(89a889ad) SHA1(1ffe112051a3afc94df1326f17ef58dc2fc531aa) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm.66",   0xe00002, 0x080000, CRC(1300eb7b) SHA1(db4f53b1cf521df99d073dcf0bfafe7b113e95d5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm.86",   0xe00004, 0x080000, CRC(6d974ebd) SHA1(41de66481a64fa8a6471c512fc5dcaf96d95ee69) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm.96",   0xe00006, 0x080000, CRC(7da4cd24) SHA1(dd44377ff9f83f15cac032c4e7ef2071adcfa196) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION(QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "rcm.01",   0x00000, 0x08000, CRC(d60cf8a3) SHA1(dccd84b93e62489c703011422d0fe84444c7f7db) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "rcm.51",   0x000000, 0x80000, CRC(b6d07080) SHA1(b8f07553c01b1f67b0696110cd4e35e4cf4fa158) )
	ROM_LOAD16_WORD_SWAP( "rcm.52",   0x080000, 0x80000, CRC(dfddc493) SHA1(56b5129f24d05d2c85a767b0a632bf260f5425b2) )
	ROM_LOAD16_WORD_SWAP( "rcm.53",   0x100000, 0x80000, CRC(6062ae3a) SHA1(28a4d59bce0c341c240ee8cc92f85850ea8ffb10) )
	ROM_LOAD16_WORD_SWAP( "rcm.54",   0x180000, 0x80000, CRC(08c6f3bf) SHA1(6b8175748ff25b90572f914e4565935e27aa09a5) )
	ROM_LOAD16_WORD_SWAP( "rcm.55",   0x200000, 0x80000, CRC(f97dfccc) SHA1(752a3855d78e55dc31291e14d0223104691784f4) )
	ROM_LOAD16_WORD_SWAP( "rcm.56",   0x280000, 0x80000, CRC(ade475bc) SHA1(02ae6bc21d5e41e05595845f264c9ad040d70b37) )
	ROM_LOAD16_WORD_SWAP( "rcm.57",   0x300000, 0x80000, CRC(075effb3) SHA1(4be02d966d933c0d92908c5e05842c6b33c9703b) )
	ROM_LOAD16_WORD_SWAP( "rcm.58",   0x380000, 0x80000, CRC(f6c1f87b) SHA1(d6cd8d2bac96404b6983d738093b7177d478f12e) )
ROM_END

ROM_START( rmancp2j )
	ROM_REGION(CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "rcmj.03a", 0x000000, 0x80000, CRC(30559f60) SHA1(7a7c7c00613e379317383d68cac47dfbbb2200c9) )
	ROM_LOAD16_WORD_SWAP( "rcmj.04a", 0x080000, 0x80000, CRC(5efc9366) SHA1(33420ac6ccf3c4982ce7644c574414574f706bd3) )
	ROM_LOAD16_WORD_SWAP( "rcm.05a", 0x100000, 0x80000, CRC(517ccde2) SHA1(492256c192f0c4814efa1ee1dd390453dd2e5865) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_FILL(              0x000000, 0x800000, 0 )
	ROMX_LOAD( "rcm.73",   0x800000, 0x080000, CRC(774c6e04) SHA1(6bd14960218e31d5a043b20a1b9d2d69cace761e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm.63",   0x800002, 0x080000, CRC(acad7c62) SHA1(eec10990339c9fd8fdae896a5fd98d5bf0220ed1) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm.83",   0x800004, 0x080000, CRC(6af30499) SHA1(a97bf2f382b6edc1e920e18d8ad5ca18131a2f21) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm.93",   0x800006, 0x080000, CRC(7a5a5166) SHA1(2160015e2e43e2c024a3af56da961ac02ffc74e9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm.74",   0xa00000, 0x080000, CRC(004ec725) SHA1(b1d3bcf920b0a1d0f4b59c77f5962e8162bbef65) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm.64",   0xa00002, 0x080000, CRC(65c0464e) SHA1(4a035f4d28fc8aa72bfe0a06392b93b0abfba458) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm.84",   0xa00004, 0x080000, CRC(fb3097cc) SHA1(7bff2372809cdfa4dcd8537a448b177c0e86a94e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm.94",   0xa00006, 0x080000, CRC(2e16557a) SHA1(f4a916c1524a8de23fc6afabc8c724a89530c631) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm.75",   0xc00000, 0x080000, CRC(70a73f99) SHA1(6dd126b4e64e34d5911ed5877b1b94b503404249) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm.65",   0xc00002, 0x080000, CRC(ecedad3d) SHA1(dea1377f086ea3a45ced983e258beb6607b295c9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm.85",   0xc00004, 0x080000, CRC(3d6186d8) SHA1(70f1ff678dd1ec533360458a564953fc5634cbc5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm.95",   0xc00006, 0x080000, CRC(8c7700f1) SHA1(84846af535e9e333e911486d545988568554b67b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm.76",   0xe00000, 0x080000, CRC(89a889ad) SHA1(1ffe112051a3afc94df1326f17ef58dc2fc531aa) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm.66",   0xe00002, 0x080000, CRC(1300eb7b) SHA1(db4f53b1cf521df99d073dcf0bfafe7b113e95d5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm.86",   0xe00004, 0x080000, CRC(6d974ebd) SHA1(41de66481a64fa8a6471c512fc5dcaf96d95ee69) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rcm.96",   0xe00006, 0x080000, CRC(7da4cd24) SHA1(dd44377ff9f83f15cac032c4e7ef2071adcfa196) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION(QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "rcm.01",   0x00000, 0x08000, CRC(d60cf8a3) SHA1(dccd84b93e62489c703011422d0fe84444c7f7db) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "rcm.51",   0x000000, 0x80000, CRC(b6d07080) SHA1(b8f07553c01b1f67b0696110cd4e35e4cf4fa158) )
	ROM_LOAD16_WORD_SWAP( "rcm.52",   0x080000, 0x80000, CRC(dfddc493) SHA1(56b5129f24d05d2c85a767b0a632bf260f5425b2) )
	ROM_LOAD16_WORD_SWAP( "rcm.53",   0x100000, 0x80000, CRC(6062ae3a) SHA1(28a4d59bce0c341c240ee8cc92f85850ea8ffb10) )
	ROM_LOAD16_WORD_SWAP( "rcm.54",   0x180000, 0x80000, CRC(08c6f3bf) SHA1(6b8175748ff25b90572f914e4565935e27aa09a5) )
	ROM_LOAD16_WORD_SWAP( "rcm.55",   0x200000, 0x80000, CRC(f97dfccc) SHA1(752a3855d78e55dc31291e14d0223104691784f4) )
	ROM_LOAD16_WORD_SWAP( "rcm.56",   0x280000, 0x80000, CRC(ade475bc) SHA1(02ae6bc21d5e41e05595845f264c9ad040d70b37) )
	ROM_LOAD16_WORD_SWAP( "rcm.57",   0x300000, 0x80000, CRC(075effb3) SHA1(4be02d966d933c0d92908c5e05842c6b33c9703b) )
	ROM_LOAD16_WORD_SWAP( "rcm.58",   0x380000, 0x80000, CRC(f6c1f87b) SHA1(d6cd8d2bac96404b6983d738093b7177d478f12e) )
ROM_END

ROM_START( sfa )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "sfze.03d", 0x000000, 0x80000, CRC(ebf2054d) SHA1(6e7b9e4202b86ab237ea5634c98b71b82d812ef2) )
	ROM_LOAD16_WORD_SWAP( "sfz.04b",  0x080000, 0x80000, CRC(8b73b0e5) SHA1(5318761f615c21395366b5333e75eaaa73ef2073) )
	ROM_LOAD16_WORD_SWAP( "sfz.05a",  0x100000, 0x80000, CRC(0810544d) SHA1(5f39bda3e7b16508eb58e5a2e0cc58c09cf428ce) )
	ROM_LOAD16_WORD_SWAP( "sfz.06",   0x180000, 0x80000, CRC(806e8f38) SHA1(b6d6912aa8f2f590335d7ff9a8214648e7131ebb) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_FILL(              0x000000, 0x800000, 0 )
	ROMX_LOAD( "sfz.14m",  0x800000, 0x200000, CRC(90fefdb3) SHA1(5eb28c8de57acfeaefebdd01509c7d9ba5244705) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz.16m",  0x800002, 0x200000, CRC(5354c948) SHA1(07588f1ba6addc04fef3274c971174aaf3e632ab) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz.18m",  0x800004, 0x200000, CRC(41a1e790) SHA1(ce25dad542308691dbe9606b26279bbd59ea4b81) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz.20m",  0x800006, 0x200000, CRC(a549df98) SHA1(f054e95df650a891ef56da8bfb31cb2c945a9aed) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sfz.01",   0x00000, 0x08000, CRC(ffffec7d) SHA1(75b4aef001b72a0f571b51b2b97803facc1832dd) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "sfz.02",   0x28000, 0x20000, CRC(45f46a08) SHA1(e32dbd27b52ab708278045b5a829376e55a4ca81) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "sfz.11m",   0x000000, 0x200000, CRC(c4b093cd) SHA1(256526bb693a0b72443f047e060304c9b739acd1) )
	ROM_LOAD16_WORD_SWAP( "sfz.12m",   0x200000, 0x200000, CRC(8bdbc4b4) SHA1(0e21c9a75a17a7e7dfd8bb51098c2b9dc4c933ec) )
ROM_END

ROM_START( sfar1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "sfze.03c", 0x000000, 0x80000, CRC(a1b69dd7) SHA1(b41440eba8f33eed955c987a04d99fca6c5c90e5) )  // Rom name dosnt appear to follow normal capcom naming system and was written on rom by hand
	ROM_LOAD16_WORD_SWAP( "sfze.04b", 0x080000, 0x80000, CRC(bb90acd5) SHA1(a19795963b90f1152f44cae29e78dd2ce67a41d6) )  // Rom name dosnt appear to follow normal capcom naming system and was written on rom by hand
	ROM_LOAD16_WORD_SWAP( "sfz.05a",  0x100000, 0x80000, CRC(0810544d) SHA1(5f39bda3e7b16508eb58e5a2e0cc58c09cf428ce) )
	ROM_LOAD16_WORD_SWAP( "sfz.06",   0x180000, 0x80000, CRC(806e8f38) SHA1(b6d6912aa8f2f590335d7ff9a8214648e7131ebb) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_FILL(              0x000000, 0x800000, 0 )
	ROMX_LOAD( "sfz.14m",  0x800000, 0x200000, CRC(90fefdb3) SHA1(5eb28c8de57acfeaefebdd01509c7d9ba5244705) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz.16m",  0x800002, 0x200000, CRC(5354c948) SHA1(07588f1ba6addc04fef3274c971174aaf3e632ab) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz.18m",  0x800004, 0x200000, CRC(41a1e790) SHA1(ce25dad542308691dbe9606b26279bbd59ea4b81) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz.20m",  0x800006, 0x200000, CRC(a549df98) SHA1(f054e95df650a891ef56da8bfb31cb2c945a9aed) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sfz.01",   0x00000, 0x08000, CRC(ffffec7d) SHA1(75b4aef001b72a0f571b51b2b97803facc1832dd) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "sfz.02",   0x28000, 0x20000, CRC(45f46a08) SHA1(e32dbd27b52ab708278045b5a829376e55a4ca81) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "sfz.11m",   0x000000, 0x200000, CRC(c4b093cd) SHA1(256526bb693a0b72443f047e060304c9b739acd1) )
	ROM_LOAD16_WORD_SWAP( "sfz.12m",   0x200000, 0x200000, CRC(8bdbc4b4) SHA1(0e21c9a75a17a7e7dfd8bb51098c2b9dc4c933ec) )
ROM_END

ROM_START( sfar2 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "sfze.03b", 0x000000, 0x80000, CRC(2bf5708e) SHA1(6ce55082e0befef47f25b4be76c607d79ec0828c) )  // Rom name dosnt appear to follow normal capcom naming system and was written on rom by hand
	ROM_LOAD16_WORD_SWAP( "sfz.04a",  0x080000, 0x80000, CRC(5f99e9a5) SHA1(e9f286315d17096adc08e6b4e6ff7c5351f5bef3) )
	ROM_LOAD16_WORD_SWAP( "sfz.05a",  0x100000, 0x80000, CRC(0810544d) SHA1(5f39bda3e7b16508eb58e5a2e0cc58c09cf428ce) )
	ROM_LOAD16_WORD_SWAP( "sfz.06",   0x180000, 0x80000, CRC(806e8f38) SHA1(b6d6912aa8f2f590335d7ff9a8214648e7131ebb) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_FILL(              0x000000, 0x800000, 0 )
	ROMX_LOAD( "sfz.14m",  0x800000, 0x200000, CRC(90fefdb3) SHA1(5eb28c8de57acfeaefebdd01509c7d9ba5244705) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz.16m",  0x800002, 0x200000, CRC(5354c948) SHA1(07588f1ba6addc04fef3274c971174aaf3e632ab) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz.18m",  0x800004, 0x200000, CRC(41a1e790) SHA1(ce25dad542308691dbe9606b26279bbd59ea4b81) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz.20m",  0x800006, 0x200000, CRC(a549df98) SHA1(f054e95df650a891ef56da8bfb31cb2c945a9aed) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sfz.01",   0x00000, 0x08000, CRC(ffffec7d) SHA1(75b4aef001b72a0f571b51b2b97803facc1832dd) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "sfz.02",   0x28000, 0x20000, CRC(45f46a08) SHA1(e32dbd27b52ab708278045b5a829376e55a4ca81) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "sfz.11m",   0x000000, 0x200000, CRC(c4b093cd) SHA1(256526bb693a0b72443f047e060304c9b739acd1) )
	ROM_LOAD16_WORD_SWAP( "sfz.12m",   0x200000, 0x200000, CRC(8bdbc4b4) SHA1(0e21c9a75a17a7e7dfd8bb51098c2b9dc4c933ec) )
ROM_END

ROM_START( sfar3 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "sfze.03a", 0x000000, 0x80000, CRC(fdbcd434) SHA1(1d5f9b821d9e0d45be61896969500b877a112fad) )
	ROM_LOAD16_WORD_SWAP( "sfz.04",   0x080000, 0x80000, CRC(0c436d30) SHA1(84229896c99bb2a4fbbab33644f779c9f86704fb) )
	ROM_LOAD16_WORD_SWAP( "sfz.05",   0x100000, 0x80000, CRC(1f363612) SHA1(87203b5db2d3887762da431d6fc2f2b76d4feedb) )
	ROM_LOAD16_WORD_SWAP( "sfz.06",   0x180000, 0x80000, CRC(806e8f38) SHA1(b6d6912aa8f2f590335d7ff9a8214648e7131ebb) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_FILL(              0x000000, 0x800000, 0 )
	ROMX_LOAD( "sfz.14m",  0x800000, 0x200000, CRC(90fefdb3) SHA1(5eb28c8de57acfeaefebdd01509c7d9ba5244705) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz.16m",  0x800002, 0x200000, CRC(5354c948) SHA1(07588f1ba6addc04fef3274c971174aaf3e632ab) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz.18m",  0x800004, 0x200000, CRC(41a1e790) SHA1(ce25dad542308691dbe9606b26279bbd59ea4b81) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz.20m",  0x800006, 0x200000, CRC(a549df98) SHA1(f054e95df650a891ef56da8bfb31cb2c945a9aed) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sfz.01",   0x00000, 0x08000, CRC(ffffec7d) SHA1(75b4aef001b72a0f571b51b2b97803facc1832dd) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "sfz.02",   0x28000, 0x20000, CRC(45f46a08) SHA1(e32dbd27b52ab708278045b5a829376e55a4ca81) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "sfz.11m",   0x000000, 0x200000, CRC(c4b093cd) SHA1(256526bb693a0b72443f047e060304c9b739acd1) )
	ROM_LOAD16_WORD_SWAP( "sfz.12m",   0x200000, 0x200000, CRC(8bdbc4b4) SHA1(0e21c9a75a17a7e7dfd8bb51098c2b9dc4c933ec) )
ROM_END

ROM_START( sfau )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "sfzu.03a", 0x000000, 0x80000, CRC(49fc7db9) SHA1(2a13d987fade88e0372f418cf451f34de67372d5) )
	ROM_LOAD16_WORD_SWAP( "sfz.04a",  0x080000, 0x80000, CRC(5f99e9a5) SHA1(e9f286315d17096adc08e6b4e6ff7c5351f5bef3) )
	ROM_LOAD16_WORD_SWAP( "sfz.05a",  0x100000, 0x80000, CRC(0810544d) SHA1(5f39bda3e7b16508eb58e5a2e0cc58c09cf428ce) )
	ROM_LOAD16_WORD_SWAP( "sfz.06",   0x180000, 0x80000, CRC(806e8f38) SHA1(b6d6912aa8f2f590335d7ff9a8214648e7131ebb) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_FILL(              0x000000, 0x800000, 0 )
	ROMX_LOAD( "sfz.14m",  0x800000, 0x200000, CRC(90fefdb3) SHA1(5eb28c8de57acfeaefebdd01509c7d9ba5244705) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz.16m",  0x800002, 0x200000, CRC(5354c948) SHA1(07588f1ba6addc04fef3274c971174aaf3e632ab) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz.18m",  0x800004, 0x200000, CRC(41a1e790) SHA1(ce25dad542308691dbe9606b26279bbd59ea4b81) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz.20m",  0x800006, 0x200000, CRC(a549df98) SHA1(f054e95df650a891ef56da8bfb31cb2c945a9aed) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sfz.01",   0x00000, 0x08000, CRC(ffffec7d) SHA1(75b4aef001b72a0f571b51b2b97803facc1832dd) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "sfz.02",   0x28000, 0x20000, CRC(45f46a08) SHA1(e32dbd27b52ab708278045b5a829376e55a4ca81) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "sfz.11m",   0x000000, 0x200000, CRC(c4b093cd) SHA1(256526bb693a0b72443f047e060304c9b739acd1) )
	ROM_LOAD16_WORD_SWAP( "sfz.12m",   0x200000, 0x200000, CRC(8bdbc4b4) SHA1(0e21c9a75a17a7e7dfd8bb51098c2b9dc4c933ec) )
ROM_END

ROM_START( sfza )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "sfza.03a", 0x000000, 0x80000, CRC(ca91bed9) SHA1(af238a4a1b87fa09ae7da7e0c41964c95dae6513) )
	ROM_LOAD16_WORD_SWAP( "sfz.04a",  0x080000, 0x80000, CRC(5f99e9a5) SHA1(e9f286315d17096adc08e6b4e6ff7c5351f5bef3) )
	ROM_LOAD16_WORD_SWAP( "sfz.05a",  0x100000, 0x80000, CRC(0810544d) SHA1(5f39bda3e7b16508eb58e5a2e0cc58c09cf428ce) )
	ROM_LOAD16_WORD_SWAP( "sfz.06",   0x180000, 0x80000, CRC(806e8f38) SHA1(b6d6912aa8f2f590335d7ff9a8214648e7131ebb) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_FILL(              0x000000, 0x800000, 0 )
	ROMX_LOAD( "sfz.14m",  0x800000, 0x200000, CRC(90fefdb3) SHA1(5eb28c8de57acfeaefebdd01509c7d9ba5244705) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz.16m",  0x800002, 0x200000, CRC(5354c948) SHA1(07588f1ba6addc04fef3274c971174aaf3e632ab) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz.18m",  0x800004, 0x200000, CRC(41a1e790) SHA1(ce25dad542308691dbe9606b26279bbd59ea4b81) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz.20m",  0x800006, 0x200000, CRC(a549df98) SHA1(f054e95df650a891ef56da8bfb31cb2c945a9aed) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sfz.01",   0x00000, 0x08000, CRC(ffffec7d) SHA1(75b4aef001b72a0f571b51b2b97803facc1832dd) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "sfz.02",   0x28000, 0x20000, CRC(45f46a08) SHA1(e32dbd27b52ab708278045b5a829376e55a4ca81) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "sfz.11m",   0x000000, 0x200000, CRC(c4b093cd) SHA1(256526bb693a0b72443f047e060304c9b739acd1) )
	ROM_LOAD16_WORD_SWAP( "sfz.12m",   0x200000, 0x200000, CRC(8bdbc4b4) SHA1(0e21c9a75a17a7e7dfd8bb51098c2b9dc4c933ec) )
ROM_END

ROM_START( sfzj )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "sfzj.03c", 0x000000, 0x80000, CRC(f5444120) SHA1(22158894971754ad83b8eeb8bdfb9874794b98c0) )
	ROM_LOAD16_WORD_SWAP( "sfz.04b",  0x080000, 0x80000, CRC(8b73b0e5) SHA1(5318761f615c21395366b5333e75eaaa73ef2073) )
	ROM_LOAD16_WORD_SWAP( "sfz.05a",  0x100000, 0x80000, CRC(0810544d) SHA1(5f39bda3e7b16508eb58e5a2e0cc58c09cf428ce) )
	ROM_LOAD16_WORD_SWAP( "sfz.06",   0x180000, 0x80000, CRC(806e8f38) SHA1(b6d6912aa8f2f590335d7ff9a8214648e7131ebb) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_FILL(              0x000000, 0x800000, 0 )
	ROMX_LOAD( "sfz.14m",  0x800000, 0x200000, CRC(90fefdb3) SHA1(5eb28c8de57acfeaefebdd01509c7d9ba5244705) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz.16m",  0x800002, 0x200000, CRC(5354c948) SHA1(07588f1ba6addc04fef3274c971174aaf3e632ab) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz.18m",  0x800004, 0x200000, CRC(41a1e790) SHA1(ce25dad542308691dbe9606b26279bbd59ea4b81) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz.20m",  0x800006, 0x200000, CRC(a549df98) SHA1(f054e95df650a891ef56da8bfb31cb2c945a9aed) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sfz.01",   0x00000, 0x08000, CRC(ffffec7d) SHA1(75b4aef001b72a0f571b51b2b97803facc1832dd) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "sfz.02",   0x28000, 0x20000, CRC(45f46a08) SHA1(e32dbd27b52ab708278045b5a829376e55a4ca81) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "sfz.11m",   0x000000, 0x200000, CRC(c4b093cd) SHA1(256526bb693a0b72443f047e060304c9b739acd1) )
	ROM_LOAD16_WORD_SWAP( "sfz.12m",   0x200000, 0x200000, CRC(8bdbc4b4) SHA1(0e21c9a75a17a7e7dfd8bb51098c2b9dc4c933ec) )
ROM_END

ROM_START( sfzjr1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "sfzj.03b", 0x000000, 0x80000, CRC(844220c2) SHA1(ff295207e0f9679285d805aa494537ae7daf1634) )
	ROM_LOAD16_WORD_SWAP( "sfz.04a",  0x080000, 0x80000, CRC(5f99e9a5) SHA1(e9f286315d17096adc08e6b4e6ff7c5351f5bef3) )
	ROM_LOAD16_WORD_SWAP( "sfz.05a",  0x100000, 0x80000, CRC(0810544d) SHA1(5f39bda3e7b16508eb58e5a2e0cc58c09cf428ce) )
	ROM_LOAD16_WORD_SWAP( "sfz.06",   0x180000, 0x80000, CRC(806e8f38) SHA1(b6d6912aa8f2f590335d7ff9a8214648e7131ebb) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_FILL(              0x000000, 0x800000, 0 )
	ROMX_LOAD( "sfz.14m",  0x800000, 0x200000, CRC(90fefdb3) SHA1(5eb28c8de57acfeaefebdd01509c7d9ba5244705) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz.16m",  0x800002, 0x200000, CRC(5354c948) SHA1(07588f1ba6addc04fef3274c971174aaf3e632ab) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz.18m",  0x800004, 0x200000, CRC(41a1e790) SHA1(ce25dad542308691dbe9606b26279bbd59ea4b81) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz.20m",  0x800006, 0x200000, CRC(a549df98) SHA1(f054e95df650a891ef56da8bfb31cb2c945a9aed) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sfz.01",   0x00000, 0x08000, CRC(ffffec7d) SHA1(75b4aef001b72a0f571b51b2b97803facc1832dd) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "sfz.02",   0x28000, 0x20000, CRC(45f46a08) SHA1(e32dbd27b52ab708278045b5a829376e55a4ca81) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "sfz.11m",   0x000000, 0x200000, CRC(c4b093cd) SHA1(256526bb693a0b72443f047e060304c9b739acd1) )
	ROM_LOAD16_WORD_SWAP( "sfz.12m",   0x200000, 0x200000, CRC(8bdbc4b4) SHA1(0e21c9a75a17a7e7dfd8bb51098c2b9dc4c933ec) )
ROM_END

ROM_START( sfzjr2 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "sfzj.03a", 0x000000, 0x80000, CRC(3cfce93c) SHA1(5f64e9707cb3d911f44e041d980e4b2250f49d75) )
	ROM_LOAD16_WORD_SWAP( "sfz.04",   0x080000, 0x80000, CRC(0c436d30) SHA1(84229896c99bb2a4fbbab33644f779c9f86704fb) )
	ROM_LOAD16_WORD_SWAP( "sfz.05",   0x100000, 0x80000, CRC(1f363612) SHA1(87203b5db2d3887762da431d6fc2f2b76d4feedb) )
	ROM_LOAD16_WORD_SWAP( "sfz.06",   0x180000, 0x80000, CRC(806e8f38) SHA1(b6d6912aa8f2f590335d7ff9a8214648e7131ebb) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_FILL(              0x000000, 0x800000, 0 )
	ROMX_LOAD( "sfz.14m",  0x800000, 0x200000, CRC(90fefdb3) SHA1(5eb28c8de57acfeaefebdd01509c7d9ba5244705) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz.16m",  0x800002, 0x200000, CRC(5354c948) SHA1(07588f1ba6addc04fef3274c971174aaf3e632ab) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz.18m",  0x800004, 0x200000, CRC(41a1e790) SHA1(ce25dad542308691dbe9606b26279bbd59ea4b81) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz.20m",  0x800006, 0x200000, CRC(a549df98) SHA1(f054e95df650a891ef56da8bfb31cb2c945a9aed) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sfz.01",   0x00000, 0x08000, CRC(ffffec7d) SHA1(75b4aef001b72a0f571b51b2b97803facc1832dd) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "sfz.02",   0x28000, 0x20000, CRC(45f46a08) SHA1(e32dbd27b52ab708278045b5a829376e55a4ca81) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "sfz.11m",   0x000000, 0x200000, CRC(c4b093cd) SHA1(256526bb693a0b72443f047e060304c9b739acd1) )
	ROM_LOAD16_WORD_SWAP( "sfz.12m",   0x200000, 0x200000, CRC(8bdbc4b4) SHA1(0e21c9a75a17a7e7dfd8bb51098c2b9dc4c933ec) )
ROM_END

ROM_START( sfzh )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "sfzh.03d", 0x000000, 0x80000, CRC(6e08cbe0) SHA1(c9d0e709004677b51efe0b62e0ae9b681dde1744) )
	ROM_LOAD16_WORD_SWAP( "sfz.04c",  0x080000, 0x80000, CRC(bb90acd5) SHA1(a19795963b90f1152f44cae29e78dd2ce67a41d6) )
	ROM_LOAD16_WORD_SWAP( "sfz.05c",  0x100000, 0x80000, CRC(0810544d) SHA1(5f39bda3e7b16508eb58e5a2e0cc58c09cf428ce) ) /* Same as revision "A" below */
	ROM_LOAD16_WORD_SWAP( "sfz.06",   0x180000, 0x80000, CRC(806e8f38) SHA1(b6d6912aa8f2f590335d7ff9a8214648e7131ebb) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_FILL(              0x000000, 0x800000, 0 )
	ROMX_LOAD( "sfz.14m",  0x800000, 0x200000, CRC(90fefdb3) SHA1(5eb28c8de57acfeaefebdd01509c7d9ba5244705) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz.16m",  0x800002, 0x200000, CRC(5354c948) SHA1(07588f1ba6addc04fef3274c971174aaf3e632ab) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz.18m",  0x800004, 0x200000, CRC(41a1e790) SHA1(ce25dad542308691dbe9606b26279bbd59ea4b81) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz.20m",  0x800006, 0x200000, CRC(a549df98) SHA1(f054e95df650a891ef56da8bfb31cb2c945a9aed) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sfz.01",   0x00000, 0x08000, CRC(ffffec7d) SHA1(75b4aef001b72a0f571b51b2b97803facc1832dd) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "sfz.02",   0x28000, 0x20000, CRC(45f46a08) SHA1(e32dbd27b52ab708278045b5a829376e55a4ca81) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "sfz.11m",   0x000000, 0x200000, CRC(c4b093cd) SHA1(256526bb693a0b72443f047e060304c9b739acd1) )
	ROM_LOAD16_WORD_SWAP( "sfz.12m",   0x200000, 0x200000, CRC(8bdbc4b4) SHA1(0e21c9a75a17a7e7dfd8bb51098c2b9dc4c933ec) )
ROM_END

ROM_START( sfzhr1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "sfzh.03c", 0x000000, 0x80000, CRC(bce635aa) SHA1(323da2de6c3ff6fd8c2c66ce6bd1d287873db9b1) )
	ROM_LOAD16_WORD_SWAP( "sfz.04a",  0x080000, 0x80000, CRC(5f99e9a5) SHA1(e9f286315d17096adc08e6b4e6ff7c5351f5bef3) )
	ROM_LOAD16_WORD_SWAP( "sfz.05a",  0x100000, 0x80000, CRC(0810544d) SHA1(5f39bda3e7b16508eb58e5a2e0cc58c09cf428ce) )
	ROM_LOAD16_WORD_SWAP( "sfz.06",   0x180000, 0x80000, CRC(806e8f38) SHA1(b6d6912aa8f2f590335d7ff9a8214648e7131ebb) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_FILL(              0x000000, 0x800000, 0 )
	ROMX_LOAD( "sfz.14m",  0x800000, 0x200000, CRC(90fefdb3) SHA1(5eb28c8de57acfeaefebdd01509c7d9ba5244705) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz.16m",  0x800002, 0x200000, CRC(5354c948) SHA1(07588f1ba6addc04fef3274c971174aaf3e632ab) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz.18m",  0x800004, 0x200000, CRC(41a1e790) SHA1(ce25dad542308691dbe9606b26279bbd59ea4b81) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz.20m",  0x800006, 0x200000, CRC(a549df98) SHA1(f054e95df650a891ef56da8bfb31cb2c945a9aed) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sfz.01",   0x00000, 0x08000, CRC(ffffec7d) SHA1(75b4aef001b72a0f571b51b2b97803facc1832dd) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "sfz.02",   0x28000, 0x20000, CRC(45f46a08) SHA1(e32dbd27b52ab708278045b5a829376e55a4ca81) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "sfz.11m",   0x000000, 0x200000, CRC(c4b093cd) SHA1(256526bb693a0b72443f047e060304c9b739acd1) )
	ROM_LOAD16_WORD_SWAP( "sfz.12m",   0x200000, 0x200000, CRC(8bdbc4b4) SHA1(0e21c9a75a17a7e7dfd8bb51098c2b9dc4c933ec) )
ROM_END

ROM_START( sfzb )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "sfzb.03g", 0x000000, 0x80000, CRC(348862d4) SHA1(b48c7df17f8b681fc726931dbf81f5aeb762a5b3) )
	ROM_LOAD16_WORD_SWAP( "sfzb.04e", 0x080000, 0x80000, CRC(8d9b2480) SHA1(405305c1572908d00eab735f28676fbbadb4fac6) )
	ROM_LOAD16_WORD_SWAP( "sfz.05a", 0x100000,  0x80000, CRC(0810544d) SHA1(5f39bda3e7b16508eb58e5a2e0cc58c09cf428ce) )
	ROM_LOAD16_WORD_SWAP( "sfz.06",  0x180000,  0x80000, CRC(806e8f38) SHA1(b6d6912aa8f2f590335d7ff9a8214648e7131ebb) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_FILL(              0x000000, 0x800000, 0 )
	ROMX_LOAD( "sfz.14m",  0x800000, 0x200000, CRC(90fefdb3) SHA1(5eb28c8de57acfeaefebdd01509c7d9ba5244705) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz.16m",  0x800002, 0x200000, CRC(5354c948) SHA1(07588f1ba6addc04fef3274c971174aaf3e632ab) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz.18m",  0x800004, 0x200000, CRC(41a1e790) SHA1(ce25dad542308691dbe9606b26279bbd59ea4b81) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz.20m",  0x800006, 0x200000, CRC(a549df98) SHA1(f054e95df650a891ef56da8bfb31cb2c945a9aed) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sfz.01",   0x00000, 0x08000, CRC(ffffec7d) SHA1(75b4aef001b72a0f571b51b2b97803facc1832dd) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "sfz.02",   0x28000, 0x20000, CRC(45f46a08) SHA1(e32dbd27b52ab708278045b5a829376e55a4ca81) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "sfz.11m",   0x000000, 0x200000, CRC(c4b093cd) SHA1(256526bb693a0b72443f047e060304c9b739acd1) )
	ROM_LOAD16_WORD_SWAP( "sfz.12m",   0x200000, 0x200000, CRC(8bdbc4b4) SHA1(0e21c9a75a17a7e7dfd8bb51098c2b9dc4c933ec) )
ROM_END

ROM_START( sfzbr1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "sfzb.03e", 0x000000, 0x80000, CRC(ecba89a3) SHA1(5a3d7a978b6dc1f334eddf8e065318d60501f223) )
	ROM_LOAD16_WORD_SWAP( "sfz.04b",  0x080000, 0x80000, CRC(8b73b0e5) SHA1(5318761f615c21395366b5333e75eaaa73ef2073) )
	ROM_LOAD16_WORD_SWAP( "sfz.05a", 0x100000,  0x80000, CRC(0810544d) SHA1(5f39bda3e7b16508eb58e5a2e0cc58c09cf428ce) )
	ROM_LOAD16_WORD_SWAP( "sfz.06",  0x180000,  0x80000, CRC(806e8f38) SHA1(b6d6912aa8f2f590335d7ff9a8214648e7131ebb) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_FILL(              0x000000, 0x800000, 0 )
	ROMX_LOAD( "sfz.14m",  0x800000, 0x200000, CRC(90fefdb3) SHA1(5eb28c8de57acfeaefebdd01509c7d9ba5244705) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz.16m",  0x800002, 0x200000, CRC(5354c948) SHA1(07588f1ba6addc04fef3274c971174aaf3e632ab) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz.18m",  0x800004, 0x200000, CRC(41a1e790) SHA1(ce25dad542308691dbe9606b26279bbd59ea4b81) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz.20m",  0x800006, 0x200000, CRC(a549df98) SHA1(f054e95df650a891ef56da8bfb31cb2c945a9aed) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sfz.01",   0x00000, 0x08000, CRC(ffffec7d) SHA1(75b4aef001b72a0f571b51b2b97803facc1832dd) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "sfz.02",   0x28000, 0x20000, CRC(45f46a08) SHA1(e32dbd27b52ab708278045b5a829376e55a4ca81) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "sfz.11m",   0x000000, 0x200000, CRC(c4b093cd) SHA1(256526bb693a0b72443f047e060304c9b739acd1) )
	ROM_LOAD16_WORD_SWAP( "sfz.12m",   0x200000, 0x200000, CRC(8bdbc4b4) SHA1(0e21c9a75a17a7e7dfd8bb51098c2b9dc4c933ec) )
ROM_END

ROM_START( sfa2 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "sz2e.03", 0x000000, 0x80000, CRC(1061e6bb) SHA1(f9b05e5cbcb1dc874de6874b01058defd6e4c407) )
	ROM_LOAD16_WORD_SWAP( "sz2e.04", 0x080000, 0x80000, CRC(22d17b26) SHA1(2d7a9cedae1bb2b7cfb80d2bfcf24cb7738df0bf) )
	ROM_LOAD16_WORD_SWAP( "sz2.05",  0x100000, 0x80000, CRC(4b442a7c) SHA1(a0d7d229cff8efb2a253ff06270258b0b4d2761e) )
	ROM_LOAD16_WORD_SWAP( "sz2.06",  0x180000, 0x80000, CRC(5b1d49c0) SHA1(f0a0c894c9cbe2b18e7f59058665949ee0025732) )
	ROM_LOAD16_WORD_SWAP( "sz2.07",  0x200000, 0x80000, CRC(8e184246) SHA1(c51f6480cfa1dcec6c4713fd38c7a27338c3fa65) )
	ROM_LOAD16_WORD_SWAP( "sz2.08",  0x280000, 0x80000, CRC(0fe8585d) SHA1(0cd5369a5aa90c98d8dc1ff3342cd4d990631cff) )

	ROM_REGION( 0x1400000, "gfx", 0 )
	ROMX_LOAD( "sz2.13m",   0x0000000, 0x400000, CRC(4d1f1f22) SHA1(659fb4305bcf0cbbbbec97ede6e68a8323b13308) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.15m",   0x0000002, 0x400000, CRC(19cea680) SHA1(4cb88963a0fbcef191c8419b6379387c01b4c81e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.17m",   0x0000004, 0x400000, CRC(e01b4588) SHA1(c2936608fd75ff6cd5fa94c6d6d6f0c77c44a450) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.19m",   0x0000006, 0x400000, CRC(0feeda64) SHA1(f5b350601437bd94b70d97feb23d791df19da6b3) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.14m",   0x1000000, 0x100000, CRC(0560c6aa) SHA1(f2bed3a8efef18052b51a7f0f6a888a18db813a1) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.16m",   0x1000002, 0x100000, CRC(ae940f87) SHA1(39ee26333abbe302ba76dced0196a2e6b3b1d02a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.18m",   0x1000004, 0x100000, CRC(4bc3c8bc) SHA1(6256963c515bf56f39b6e559afefd653ead56c54) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.20m",   0x1000006, 0x100000, CRC(39e674c0) SHA1(8e771a2d8c2accad0463bccd21d7b23af0c895a1) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sz2.01a",   0x00000, 0x08000, CRC(1bc323cf) SHA1(83fbd6e9b327700dc9f1c59700b7385bc3705749) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "sz2.02a",   0x28000, 0x20000, CRC(ba6a5013) SHA1(7814f3e56b69529b9860dd61c3b1e8d700244b03) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "sz2.11m",   0x000000, 0x200000, CRC(aa47a601) SHA1(a4d1ee89c84a3b9db06469bb66e85293b5aa9ac9) )
	ROM_LOAD16_WORD_SWAP( "sz2.12m",   0x200000, 0x200000, CRC(2237bc53) SHA1(96d5693047e4cf1ed10a8ee1905cea267a278e92) )
ROM_END

ROM_START( sfa2u )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "sz2u.03", 0x000000, 0x80000, CRC(84a09006) SHA1(334c33f9eb324d71443cc9c44e94f5a72451fa3f) )
	ROM_LOAD16_WORD_SWAP( "sz2u.04", 0x080000, 0x80000, CRC(ac46e5ed) SHA1(a01b57daba4c255d5f07465c553bcbfe51d9ab0d) )
	ROM_LOAD16_WORD_SWAP( "sz2u.05", 0x100000, 0x80000, CRC(6c0c79d3) SHA1(ae2a4e2903beec1f10fff6edac1a2385d6ac1c38) )
	ROM_LOAD16_WORD_SWAP( "sz2u.06", 0x180000, 0x80000, CRC(c5c8eb63) SHA1(4ea033834c7b260877335296f88c0db484dea289) )
	ROM_LOAD16_WORD_SWAP( "sz2u.07", 0x200000, 0x80000, CRC(5de01cc5) SHA1(b19bfe970b217c96e782860fc3ae3fcb976ed30d) )
	ROM_LOAD16_WORD_SWAP( "sz2u.08", 0x280000, 0x80000, CRC(bea11d56) SHA1(a1d475066d36de7cc5d931671ccdcd89737bc7ee) )

	ROM_REGION( 0x1400000, "gfx", 0 )
	ROMX_LOAD( "sz2.13m",   0x0000000, 0x400000, CRC(4d1f1f22) SHA1(659fb4305bcf0cbbbbec97ede6e68a8323b13308) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.15m",   0x0000002, 0x400000, CRC(19cea680) SHA1(4cb88963a0fbcef191c8419b6379387c01b4c81e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.17m",   0x0000004, 0x400000, CRC(e01b4588) SHA1(c2936608fd75ff6cd5fa94c6d6d6f0c77c44a450) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.19m",   0x0000006, 0x400000, CRC(0feeda64) SHA1(f5b350601437bd94b70d97feb23d791df19da6b3) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.14m",   0x1000000, 0x100000, CRC(0560c6aa) SHA1(f2bed3a8efef18052b51a7f0f6a888a18db813a1) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.16m",   0x1000002, 0x100000, CRC(ae940f87) SHA1(39ee26333abbe302ba76dced0196a2e6b3b1d02a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.18m",   0x1000004, 0x100000, CRC(4bc3c8bc) SHA1(6256963c515bf56f39b6e559afefd653ead56c54) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.20m",   0x1000006, 0x100000, CRC(39e674c0) SHA1(8e771a2d8c2accad0463bccd21d7b23af0c895a1) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sz2.01a",   0x00000, 0x08000, CRC(1bc323cf) SHA1(83fbd6e9b327700dc9f1c59700b7385bc3705749) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "sz2.02a",   0x28000, 0x20000, CRC(ba6a5013) SHA1(7814f3e56b69529b9860dd61c3b1e8d700244b03) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "sz2.11m",   0x000000, 0x200000, CRC(aa47a601) SHA1(a4d1ee89c84a3b9db06469bb66e85293b5aa9ac9) )
	ROM_LOAD16_WORD_SWAP( "sz2.12m",   0x200000, 0x200000, CRC(2237bc53) SHA1(96d5693047e4cf1ed10a8ee1905cea267a278e92) )
ROM_END

ROM_START( sfz2j )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "sz2j.03a", 0x000000, 0x80000, CRC(97461e28) SHA1(8fbe4c9a59f51612f86adb8ef5057e43be0348bf) )
	ROM_LOAD16_WORD_SWAP( "sz2j.04a", 0x080000, 0x80000, CRC(ae4851a9) SHA1(4771bc22fe1b376b753a68506c012c52bd4b886d) )
	ROM_LOAD16_WORD_SWAP( "sz2.05a",  0x100000, 0x80000, CRC(98e8e992) SHA1(41745b63e6b3888081d189b8315ed3b7526b3d20) )
	ROM_LOAD16_WORD_SWAP( "sz2.06",   0x180000, 0x80000, CRC(5b1d49c0) SHA1(f0a0c894c9cbe2b18e7f59058665949ee0025732) )
	ROM_LOAD16_WORD_SWAP( "sz2j.07a", 0x200000, 0x80000, CRC(d910b2a2) SHA1(aa201660caa9cef993c147a1077c9e7767b34a78) )
	ROM_LOAD16_WORD_SWAP( "sz2.08",   0x280000, 0x80000, CRC(0fe8585d) SHA1(0cd5369a5aa90c98d8dc1ff3342cd4d990631cff) )

	ROM_REGION( 0x1400000, "gfx", 0 )
	ROMX_LOAD( "sz2.13m",   0x0000000, 0x400000, CRC(4d1f1f22) SHA1(659fb4305bcf0cbbbbec97ede6e68a8323b13308) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.15m",   0x0000002, 0x400000, CRC(19cea680) SHA1(4cb88963a0fbcef191c8419b6379387c01b4c81e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.17m",   0x0000004, 0x400000, CRC(e01b4588) SHA1(c2936608fd75ff6cd5fa94c6d6d6f0c77c44a450) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.19m",   0x0000006, 0x400000, CRC(0feeda64) SHA1(f5b350601437bd94b70d97feb23d791df19da6b3) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.14m",   0x1000000, 0x100000, CRC(0560c6aa) SHA1(f2bed3a8efef18052b51a7f0f6a888a18db813a1) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.16m",   0x1000002, 0x100000, CRC(ae940f87) SHA1(39ee26333abbe302ba76dced0196a2e6b3b1d02a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.18m",   0x1000004, 0x100000, CRC(4bc3c8bc) SHA1(6256963c515bf56f39b6e559afefd653ead56c54) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.20m",   0x1000006, 0x100000, CRC(39e674c0) SHA1(8e771a2d8c2accad0463bccd21d7b23af0c895a1) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sz2.01a",   0x00000, 0x08000, CRC(1bc323cf) SHA1(83fbd6e9b327700dc9f1c59700b7385bc3705749) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "sz2.02a",   0x28000, 0x20000, CRC(ba6a5013) SHA1(7814f3e56b69529b9860dd61c3b1e8d700244b03) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "sz2.11m",   0x000000, 0x200000, CRC(aa47a601) SHA1(a4d1ee89c84a3b9db06469bb66e85293b5aa9ac9) )
	ROM_LOAD16_WORD_SWAP( "sz2.12m",   0x200000, 0x200000, CRC(2237bc53) SHA1(96d5693047e4cf1ed10a8ee1905cea267a278e92) )
ROM_END

ROM_START( sfz2a )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "sz2a.03a", 0x000000, 0x80000, CRC(30d2099f) SHA1(d4c7d8c2ad08cae228544bd692aedecd4fab829c) )
	ROM_LOAD16_WORD_SWAP( "sz2a.04a", 0x080000, 0x80000, CRC(1cc94db1) SHA1(518151f443ff5219b20c9fd59b7614920302aecd) )
	ROM_LOAD16_WORD_SWAP( "sz2.05a",  0x100000, 0x80000, CRC(98e8e992) SHA1(41745b63e6b3888081d189b8315ed3b7526b3d20) )
	ROM_LOAD16_WORD_SWAP( "sz2.06",   0x180000, 0x80000, CRC(5b1d49c0) SHA1(f0a0c894c9cbe2b18e7f59058665949ee0025732) )
	ROM_LOAD16_WORD_SWAP( "sz2a.07a", 0x200000, 0x80000, CRC(0aed2494) SHA1(7beb1a394f17cd78a27128292b626aae28754ca2) )
	ROM_LOAD16_WORD_SWAP( "sz2.08",   0x280000, 0x80000, CRC(0fe8585d) SHA1(0cd5369a5aa90c98d8dc1ff3342cd4d990631cff) )

	ROM_REGION( 0x1400000, "gfx", 0 )
	ROMX_LOAD( "sz2.13m",   0x0000000, 0x400000, CRC(4d1f1f22) SHA1(659fb4305bcf0cbbbbec97ede6e68a8323b13308) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.15m",   0x0000002, 0x400000, CRC(19cea680) SHA1(4cb88963a0fbcef191c8419b6379387c01b4c81e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.17m",   0x0000004, 0x400000, CRC(e01b4588) SHA1(c2936608fd75ff6cd5fa94c6d6d6f0c77c44a450) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.19m",   0x0000006, 0x400000, CRC(0feeda64) SHA1(f5b350601437bd94b70d97feb23d791df19da6b3) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.14m",   0x1000000, 0x100000, CRC(0560c6aa) SHA1(f2bed3a8efef18052b51a7f0f6a888a18db813a1) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.16m",   0x1000002, 0x100000, CRC(ae940f87) SHA1(39ee26333abbe302ba76dced0196a2e6b3b1d02a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.18m",   0x1000004, 0x100000, CRC(4bc3c8bc) SHA1(6256963c515bf56f39b6e559afefd653ead56c54) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.20m",   0x1000006, 0x100000, CRC(39e674c0) SHA1(8e771a2d8c2accad0463bccd21d7b23af0c895a1) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sz2.01a",   0x00000, 0x08000, CRC(1bc323cf) SHA1(83fbd6e9b327700dc9f1c59700b7385bc3705749) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "sz2.02a",   0x28000, 0x20000, CRC(ba6a5013) SHA1(7814f3e56b69529b9860dd61c3b1e8d700244b03) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "sz2.11m",   0x000000, 0x200000, CRC(aa47a601) SHA1(a4d1ee89c84a3b9db06469bb66e85293b5aa9ac9) )
	ROM_LOAD16_WORD_SWAP( "sz2.12m",   0x200000, 0x200000, CRC(2237bc53) SHA1(96d5693047e4cf1ed10a8ee1905cea267a278e92) )
ROM_END

ROM_START( sfz2b )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "sz2b.03b", 0x000000, 0x80000, CRC(1ac12812) SHA1(b948b939021ffe20437b19325fe94daa072c1271) )
	ROM_LOAD16_WORD_SWAP( "sz2b.04b", 0x080000, 0x80000, CRC(e4ffaf68) SHA1(e22bb4f92a965108570c2beee1fd533380838d90) )
	ROM_LOAD16_WORD_SWAP( "sz2b.05a", 0x100000, 0x80000, CRC(dd224156) SHA1(85d29f2a288430d51c53b88130f255131e5dc601) )
	ROM_LOAD16_WORD_SWAP( "sz2b.06a", 0x180000, 0x80000, CRC(a45a75a6) SHA1(e9cd4ad08ac0d058e9e1660acb07eb350a141fd6) )
	ROM_LOAD16_WORD_SWAP( "sz2b.07a", 0x200000, 0x80000, CRC(7d19d5ec) SHA1(ab88dfcb2029499578837b8f97fbf55412c8f756) )
	ROM_LOAD16_WORD_SWAP( "sz2b.08",  0x280000, 0x80000, CRC(92b66e01) SHA1(f09cb38aa49b22a9c98219fb2ad8a66b11fa5872) )

	ROM_REGION( 0x1400000, "gfx", 0 )
	ROMX_LOAD( "sz2.13m",   0x0000000, 0x400000, CRC(4d1f1f22) SHA1(659fb4305bcf0cbbbbec97ede6e68a8323b13308) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.15m",   0x0000002, 0x400000, CRC(19cea680) SHA1(4cb88963a0fbcef191c8419b6379387c01b4c81e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.17m",   0x0000004, 0x400000, CRC(e01b4588) SHA1(c2936608fd75ff6cd5fa94c6d6d6f0c77c44a450) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.19m",   0x0000006, 0x400000, CRC(0feeda64) SHA1(f5b350601437bd94b70d97feb23d791df19da6b3) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.14m",   0x1000000, 0x100000, CRC(0560c6aa) SHA1(f2bed3a8efef18052b51a7f0f6a888a18db813a1) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.16m",   0x1000002, 0x100000, CRC(ae940f87) SHA1(39ee26333abbe302ba76dced0196a2e6b3b1d02a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.18m",   0x1000004, 0x100000, CRC(4bc3c8bc) SHA1(6256963c515bf56f39b6e559afefd653ead56c54) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.20m",   0x1000006, 0x100000, CRC(39e674c0) SHA1(8e771a2d8c2accad0463bccd21d7b23af0c895a1) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sz2.01a",   0x00000, 0x08000, CRC(1bc323cf) SHA1(83fbd6e9b327700dc9f1c59700b7385bc3705749) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "sz2.02a",   0x28000, 0x20000, CRC(ba6a5013) SHA1(7814f3e56b69529b9860dd61c3b1e8d700244b03) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "sz2.11m",   0x000000, 0x200000, CRC(aa47a601) SHA1(a4d1ee89c84a3b9db06469bb66e85293b5aa9ac9) )
	ROM_LOAD16_WORD_SWAP( "sz2.12m",   0x200000, 0x200000, CRC(2237bc53) SHA1(96d5693047e4cf1ed10a8ee1905cea267a278e92) )
ROM_END

ROM_START( sfz2br1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "sz2b.03", 0x000000, 0x80000, CRC(e6ce530b) SHA1(044c3f6f6c64d18f4f9ce96b67ff86b3c8bcd065) )
	ROM_LOAD16_WORD_SWAP( "sz2b.04", 0x080000, 0x80000, CRC(1605a0cb) SHA1(5041c87dbb3ed41fe1cb3e9eade195bc2f7cba2a) )
	ROM_LOAD16_WORD_SWAP( "sz2.05",  0x100000, 0x80000, CRC(4b442a7c) SHA1(a0d7d229cff8efb2a253ff06270258b0b4d2761e) )
	ROM_LOAD16_WORD_SWAP( "sz2.06",  0x180000, 0x80000, CRC(5b1d49c0) SHA1(f0a0c894c9cbe2b18e7f59058665949ee0025732) )
	ROM_LOAD16_WORD_SWAP( "sz2b.07", 0x200000, 0x80000, CRC(947e8ac6) SHA1(da82be7cba9cd557da3ee35be9194130a959d5cb) )
	ROM_LOAD16_WORD_SWAP( "sz2b.08", 0x280000, 0x80000, CRC(92b66e01) SHA1(f09cb38aa49b22a9c98219fb2ad8a66b11fa5872) )

	ROM_REGION( 0x1400000, "gfx", 0 )
	ROMX_LOAD( "sz2.13m",   0x0000000, 0x400000, CRC(4d1f1f22) SHA1(659fb4305bcf0cbbbbec97ede6e68a8323b13308) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.15m",   0x0000002, 0x400000, CRC(19cea680) SHA1(4cb88963a0fbcef191c8419b6379387c01b4c81e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.17m",   0x0000004, 0x400000, CRC(e01b4588) SHA1(c2936608fd75ff6cd5fa94c6d6d6f0c77c44a450) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.19m",   0x0000006, 0x400000, CRC(0feeda64) SHA1(f5b350601437bd94b70d97feb23d791df19da6b3) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.14m",   0x1000000, 0x100000, CRC(0560c6aa) SHA1(f2bed3a8efef18052b51a7f0f6a888a18db813a1) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.16m",   0x1000002, 0x100000, CRC(ae940f87) SHA1(39ee26333abbe302ba76dced0196a2e6b3b1d02a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.18m",   0x1000004, 0x100000, CRC(4bc3c8bc) SHA1(6256963c515bf56f39b6e559afefd653ead56c54) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.20m",   0x1000006, 0x100000, CRC(39e674c0) SHA1(8e771a2d8c2accad0463bccd21d7b23af0c895a1) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sz2.01a",   0x00000, 0x08000, CRC(1bc323cf) SHA1(83fbd6e9b327700dc9f1c59700b7385bc3705749) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "sz2.02a",   0x28000, 0x20000, CRC(ba6a5013) SHA1(7814f3e56b69529b9860dd61c3b1e8d700244b03) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "sz2.11m",   0x000000, 0x200000, CRC(aa47a601) SHA1(a4d1ee89c84a3b9db06469bb66e85293b5aa9ac9) )
	ROM_LOAD16_WORD_SWAP( "sz2.12m",   0x200000, 0x200000, CRC(2237bc53) SHA1(96d5693047e4cf1ed10a8ee1905cea267a278e92) )
ROM_END

ROM_START( sfz2h )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "sz2h.03", 0x000000, 0x80000, CRC(bfeddf5b) SHA1(dd4a748ba8674725a399b78e721ff9c7adaaf890) )
	ROM_LOAD16_WORD_SWAP( "sz2h.04", 0x080000, 0x80000, CRC(ea5009fb) SHA1(9186c702994f99488d52d4dbccb3823d2b9a6dd9) )
	ROM_LOAD16_WORD_SWAP( "sz2.05",  0x100000, 0x80000, CRC(4b442a7c) SHA1(a0d7d229cff8efb2a253ff06270258b0b4d2761e) )
	ROM_LOAD16_WORD_SWAP( "sz2.06",  0x180000, 0x80000, CRC(5b1d49c0) SHA1(f0a0c894c9cbe2b18e7f59058665949ee0025732) )
	ROM_LOAD16_WORD_SWAP( "sz2b.07", 0x200000, 0x80000, CRC(947e8ac6) SHA1(da82be7cba9cd557da3ee35be9194130a959d5cb) )
	ROM_LOAD16_WORD_SWAP( "sz2b.08", 0x280000, 0x80000, CRC(92b66e01) SHA1(f09cb38aa49b22a9c98219fb2ad8a66b11fa5872) )

	ROM_REGION( 0x1400000, "gfx", 0 )
	ROMX_LOAD( "sz2.13m",   0x0000000, 0x400000, CRC(4d1f1f22) SHA1(659fb4305bcf0cbbbbec97ede6e68a8323b13308) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.15m",   0x0000002, 0x400000, CRC(19cea680) SHA1(4cb88963a0fbcef191c8419b6379387c01b4c81e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.17m",   0x0000004, 0x400000, CRC(e01b4588) SHA1(c2936608fd75ff6cd5fa94c6d6d6f0c77c44a450) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.19m",   0x0000006, 0x400000, CRC(0feeda64) SHA1(f5b350601437bd94b70d97feb23d791df19da6b3) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.14m",   0x1000000, 0x100000, CRC(0560c6aa) SHA1(f2bed3a8efef18052b51a7f0f6a888a18db813a1) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.16m",   0x1000002, 0x100000, CRC(ae940f87) SHA1(39ee26333abbe302ba76dced0196a2e6b3b1d02a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.18m",   0x1000004, 0x100000, CRC(4bc3c8bc) SHA1(6256963c515bf56f39b6e559afefd653ead56c54) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.20m",   0x1000006, 0x100000, CRC(39e674c0) SHA1(8e771a2d8c2accad0463bccd21d7b23af0c895a1) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sz2.01a",   0x00000, 0x08000, CRC(1bc323cf) SHA1(83fbd6e9b327700dc9f1c59700b7385bc3705749) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "sz2.02a",   0x28000, 0x20000, CRC(ba6a5013) SHA1(7814f3e56b69529b9860dd61c3b1e8d700244b03) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "sz2.11m",   0x000000, 0x200000, CRC(aa47a601) SHA1(a4d1ee89c84a3b9db06469bb66e85293b5aa9ac9) )
	ROM_LOAD16_WORD_SWAP( "sz2.12m",   0x200000, 0x200000, CRC(2237bc53) SHA1(96d5693047e4cf1ed10a8ee1905cea267a278e92) )
ROM_END

ROM_START( sfz2n )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "sz2n.03", 0x000000, 0x80000, CRC(58924741) SHA1(b077867a6a601b5f87a644ebfe9b8d0c87d0471f) )
	ROM_LOAD16_WORD_SWAP( "sz2n.04", 0x080000, 0x80000, CRC(592a17c5) SHA1(7262f9017834e932858e81dadfba9d9feb946530) )
	ROM_LOAD16_WORD_SWAP( "sz2.05",  0x100000, 0x80000, CRC(4b442a7c) SHA1(a0d7d229cff8efb2a253ff06270258b0b4d2761e) )
	ROM_LOAD16_WORD_SWAP( "sz2.06",  0x180000, 0x80000, CRC(5b1d49c0) SHA1(f0a0c894c9cbe2b18e7f59058665949ee0025732) )
	ROM_LOAD16_WORD_SWAP( "sz2.07",  0x200000, 0x80000, CRC(8e184246) SHA1(c51f6480cfa1dcec6c4713fd38c7a27338c3fa65) )
	ROM_LOAD16_WORD_SWAP( "sz2.08",  0x280000, 0x80000, CRC(0fe8585d) SHA1(0cd5369a5aa90c98d8dc1ff3342cd4d990631cff) )

	ROM_REGION( 0x1400000, "gfx", 0 )
	ROMX_LOAD( "sz2.13m",   0x0000000, 0x400000, CRC(4d1f1f22) SHA1(659fb4305bcf0cbbbbec97ede6e68a8323b13308) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.15m",   0x0000002, 0x400000, CRC(19cea680) SHA1(4cb88963a0fbcef191c8419b6379387c01b4c81e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.17m",   0x0000004, 0x400000, CRC(e01b4588) SHA1(c2936608fd75ff6cd5fa94c6d6d6f0c77c44a450) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.19m",   0x0000006, 0x400000, CRC(0feeda64) SHA1(f5b350601437bd94b70d97feb23d791df19da6b3) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.14m",   0x1000000, 0x100000, CRC(0560c6aa) SHA1(f2bed3a8efef18052b51a7f0f6a888a18db813a1) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.16m",   0x1000002, 0x100000, CRC(ae940f87) SHA1(39ee26333abbe302ba76dced0196a2e6b3b1d02a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.18m",   0x1000004, 0x100000, CRC(4bc3c8bc) SHA1(6256963c515bf56f39b6e559afefd653ead56c54) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.20m",   0x1000006, 0x100000, CRC(39e674c0) SHA1(8e771a2d8c2accad0463bccd21d7b23af0c895a1) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sz2.01a",   0x00000, 0x08000, CRC(1bc323cf) SHA1(83fbd6e9b327700dc9f1c59700b7385bc3705749) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "sz2.02a",   0x28000, 0x20000, CRC(ba6a5013) SHA1(7814f3e56b69529b9860dd61c3b1e8d700244b03) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "sz2.11m",   0x000000, 0x200000, CRC(aa47a601) SHA1(a4d1ee89c84a3b9db06469bb66e85293b5aa9ac9) )
	ROM_LOAD16_WORD_SWAP( "sz2.12m",   0x200000, 0x200000, CRC(2237bc53) SHA1(96d5693047e4cf1ed10a8ee1905cea267a278e92) )
ROM_END

ROM_START( sfz2alj )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "szaj.03a", 0x000000, 0x80000, CRC(a3802fe3) SHA1(c983a15ed675b22aebfe6ac55890b4e0b5eb8d48) )
	ROM_LOAD16_WORD_SWAP( "szaj.04a", 0x080000, 0x80000, CRC(e7ca87c7) SHA1(e44c930b27431dd2b983d93471a440d292e7a8bb) )
	ROM_LOAD16_WORD_SWAP( "szaj.05a", 0x100000, 0x80000, CRC(c88ebf88) SHA1(e37cf232fc70b9a3254dea99754e288232f04e25) )
	ROM_LOAD16_WORD_SWAP( "szaj.06a", 0x180000, 0x80000, CRC(35ed5b7a) SHA1(b03cb92f594eb35fa374445f74930e9040a2baff) )
	ROM_LOAD16_WORD_SWAP( "szaj.07a", 0x200000, 0x80000, CRC(975dcb3e) SHA1(a2ca8e5a768e49cce9e2137ec0dcba9337ed2ad5) )
	ROM_LOAD16_WORD_SWAP( "szaj.08a", 0x280000, 0x80000, CRC(dc73f2d7) SHA1(09fa10e7d1ff5f0dac87a6cf3d66730e3ab9ad25) )

	ROM_REGION( 0x1400000, "gfx", 0 )
	ROMX_LOAD( "sza.13m",   0x0000000, 0x400000, CRC(4d1f1f22) SHA1(659fb4305bcf0cbbbbec97ede6e68a8323b13308) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sza.15m",   0x0000002, 0x400000, CRC(19cea680) SHA1(4cb88963a0fbcef191c8419b6379387c01b4c81e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sza.17m",   0x0000004, 0x400000, CRC(e01b4588) SHA1(c2936608fd75ff6cd5fa94c6d6d6f0c77c44a450) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sza.19m",   0x0000006, 0x400000, CRC(0feeda64) SHA1(f5b350601437bd94b70d97feb23d791df19da6b3) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sza.14m",   0x1000000, 0x100000, CRC(0560c6aa) SHA1(f2bed3a8efef18052b51a7f0f6a888a18db813a1) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sza.16m",   0x1000002, 0x100000, CRC(ae940f87) SHA1(39ee26333abbe302ba76dced0196a2e6b3b1d02a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sza.18m",   0x1000004, 0x100000, CRC(4bc3c8bc) SHA1(6256963c515bf56f39b6e559afefd653ead56c54) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sza.20m",   0x1000006, 0x100000, CRC(39e674c0) SHA1(8e771a2d8c2accad0463bccd21d7b23af0c895a1) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sza.01",   0x00000, 0x08000, CRC(1bc323cf) SHA1(83fbd6e9b327700dc9f1c59700b7385bc3705749) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "sza.02",   0x28000, 0x20000, CRC(ba6a5013) SHA1(7814f3e56b69529b9860dd61c3b1e8d700244b03) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "sza.11m",   0x000000, 0x200000, CRC(aa47a601) SHA1(a4d1ee89c84a3b9db06469bb66e85293b5aa9ac9) )
	ROM_LOAD16_WORD_SWAP( "sza.12m",   0x200000, 0x200000, CRC(2237bc53) SHA1(96d5693047e4cf1ed10a8ee1905cea267a278e92) )
ROM_END

ROM_START( sfz2al )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "szaa.03", 0x000000, 0x80000, CRC(88e7023e) SHA1(34e74ec54c05d75e5cf207abb6e536fcca233b8b) )
	ROM_LOAD16_WORD_SWAP( "szaa.04", 0x080000, 0x80000, CRC(ae8ec36e) SHA1(b2f3de9e33169f6266aaabd5eae6c057ea10dcab) )
	ROM_LOAD16_WORD_SWAP( "szaa.05", 0x100000, 0x80000, CRC(f053a55e) SHA1(f98a8af5cd33a543a5596d59381f9adafed38854) )
	ROM_LOAD16_WORD_SWAP( "szaa.06", 0x180000, 0x80000, CRC(cfc0e7a8) SHA1(31ed58451c7a6ac88a8fccab369167694698f044) )
	ROM_LOAD16_WORD_SWAP( "szaa.07", 0x200000, 0x80000, CRC(5feb8b20) SHA1(13c79c9b72c3abf0a0b75d507d91ece71e460c06) )
	ROM_LOAD16_WORD_SWAP( "szaa.08", 0x280000, 0x80000, CRC(6eb6d412) SHA1(c858fec9c1dfea70dfcca629c1c24306f8ae6d81) )

	ROM_REGION( 0x1400000, "gfx", 0 )
	ROMX_LOAD( "sza.13m",   0x0000000, 0x400000, CRC(4d1f1f22) SHA1(659fb4305bcf0cbbbbec97ede6e68a8323b13308) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sza.15m",   0x0000002, 0x400000, CRC(19cea680) SHA1(4cb88963a0fbcef191c8419b6379387c01b4c81e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sza.17m",   0x0000004, 0x400000, CRC(e01b4588) SHA1(c2936608fd75ff6cd5fa94c6d6d6f0c77c44a450) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sza.19m",   0x0000006, 0x400000, CRC(0feeda64) SHA1(f5b350601437bd94b70d97feb23d791df19da6b3) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sza.14m",   0x1000000, 0x100000, CRC(0560c6aa) SHA1(f2bed3a8efef18052b51a7f0f6a888a18db813a1) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sza.16m",   0x1000002, 0x100000, CRC(ae940f87) SHA1(39ee26333abbe302ba76dced0196a2e6b3b1d02a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sza.18m",   0x1000004, 0x100000, CRC(4bc3c8bc) SHA1(6256963c515bf56f39b6e559afefd653ead56c54) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sza.20m",   0x1000006, 0x100000, CRC(39e674c0) SHA1(8e771a2d8c2accad0463bccd21d7b23af0c895a1) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sza.01",   0x00000, 0x08000, CRC(1bc323cf) SHA1(83fbd6e9b327700dc9f1c59700b7385bc3705749) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "sza.02",   0x28000, 0x20000, CRC(ba6a5013) SHA1(7814f3e56b69529b9860dd61c3b1e8d700244b03) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "sza.11m",   0x000000, 0x200000, CRC(aa47a601) SHA1(a4d1ee89c84a3b9db06469bb66e85293b5aa9ac9) )
	ROM_LOAD16_WORD_SWAP( "sza.12m",   0x200000, 0x200000, CRC(2237bc53) SHA1(96d5693047e4cf1ed10a8ee1905cea267a278e92) )
ROM_END

ROM_START( sfz2alh )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "szah.03", 0x000000, 0x80000, CRC(06f93d1d) SHA1(495de176ba55b35270fc05f19edf17a0f249ff0e) )
	ROM_LOAD16_WORD_SWAP( "szah.04", 0x080000, 0x80000, CRC(e62ee914) SHA1(def4f27c1b64be5143234f1f402260adae66cdde) )
	ROM_LOAD16_WORD_SWAP( "szah.05", 0x100000, 0x80000, CRC(2b7f4b20) SHA1(5511263f5f6e532ee7fe1995f08f16651a1d45a1) )
	ROM_LOAD16_WORD_SWAP( "sza.06",  0x180000, 0x80000, CRC(0abda2fc) SHA1(830da40f6a9bb3bc866ee9c5cab1b0eb3c4dcb71) )
	ROM_LOAD16_WORD_SWAP( "sza.07",  0x200000, 0x80000, CRC(e9430762) SHA1(923aea8db5f9b59212ec6dbc35be0808ea015140) )
	ROM_LOAD16_WORD_SWAP( "sza.08",  0x280000, 0x80000, CRC(b65711a9) SHA1(3918f44e1bb189e2a115625b35f477eb91a65f04) )

	ROM_REGION( 0x1400000, "gfx", 0 )
	ROMX_LOAD( "sza.13m",   0x0000000, 0x400000, CRC(4d1f1f22) SHA1(659fb4305bcf0cbbbbec97ede6e68a8323b13308) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sza.15m",   0x0000002, 0x400000, CRC(19cea680) SHA1(4cb88963a0fbcef191c8419b6379387c01b4c81e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sza.17m",   0x0000004, 0x400000, CRC(e01b4588) SHA1(c2936608fd75ff6cd5fa94c6d6d6f0c77c44a450) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sza.19m",   0x0000006, 0x400000, CRC(0feeda64) SHA1(f5b350601437bd94b70d97feb23d791df19da6b3) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sza.14m",   0x1000000, 0x100000, CRC(0560c6aa) SHA1(f2bed3a8efef18052b51a7f0f6a888a18db813a1) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sza.16m",   0x1000002, 0x100000, CRC(ae940f87) SHA1(39ee26333abbe302ba76dced0196a2e6b3b1d02a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sza.18m",   0x1000004, 0x100000, CRC(4bc3c8bc) SHA1(6256963c515bf56f39b6e559afefd653ead56c54) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sza.20m",   0x1000006, 0x100000, CRC(39e674c0) SHA1(8e771a2d8c2accad0463bccd21d7b23af0c895a1) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sza.01",   0x00000, 0x08000, CRC(1bc323cf) SHA1(83fbd6e9b327700dc9f1c59700b7385bc3705749) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "sza.02",   0x28000, 0x20000, CRC(ba6a5013) SHA1(7814f3e56b69529b9860dd61c3b1e8d700244b03) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "sza.11m",   0x000000, 0x200000, CRC(aa47a601) SHA1(a4d1ee89c84a3b9db06469bb66e85293b5aa9ac9) )
	ROM_LOAD16_WORD_SWAP( "sza.12m",   0x200000, 0x200000, CRC(2237bc53) SHA1(96d5693047e4cf1ed10a8ee1905cea267a278e92) )
ROM_END

ROM_START( sfz2alb )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "szab.03", 0x000000, 0x80000, CRC(cb436eca) SHA1(406bea614429f78c0150c2f5042abc8673a6722e) )
	ROM_LOAD16_WORD_SWAP( "szab.04", 0x080000, 0x80000, CRC(14534bea) SHA1(8fff2cd9221ef12de9364cc15429b6df6bacc48e) )
	ROM_LOAD16_WORD_SWAP( "szab.05", 0x100000, 0x80000, CRC(7fb10658) SHA1(f9eba0271d92d6d29156a7b4dd8b1cdb3dd8aa48) )
	ROM_LOAD16_WORD_SWAP( "sza.06",  0x180000, 0x80000, CRC(0abda2fc) SHA1(830da40f6a9bb3bc866ee9c5cab1b0eb3c4dcb71) )
	ROM_LOAD16_WORD_SWAP( "sza.07",  0x200000, 0x80000, CRC(e9430762) SHA1(923aea8db5f9b59212ec6dbc35be0808ea015140) )
	ROM_LOAD16_WORD_SWAP( "sza.08",  0x280000, 0x80000, CRC(b65711a9) SHA1(3918f44e1bb189e2a115625b35f477eb91a65f04) )

	ROM_REGION( 0x1400000, "gfx", 0 )
	ROMX_LOAD( "sza.13m",   0x0000000, 0x400000, CRC(4d1f1f22) SHA1(659fb4305bcf0cbbbbec97ede6e68a8323b13308) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sza.15m",   0x0000002, 0x400000, CRC(19cea680) SHA1(4cb88963a0fbcef191c8419b6379387c01b4c81e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sza.17m",   0x0000004, 0x400000, CRC(e01b4588) SHA1(c2936608fd75ff6cd5fa94c6d6d6f0c77c44a450) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sza.19m",   0x0000006, 0x400000, CRC(0feeda64) SHA1(f5b350601437bd94b70d97feb23d791df19da6b3) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sza.14m",   0x1000000, 0x100000, CRC(0560c6aa) SHA1(f2bed3a8efef18052b51a7f0f6a888a18db813a1) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sza.16m",   0x1000002, 0x100000, CRC(ae940f87) SHA1(39ee26333abbe302ba76dced0196a2e6b3b1d02a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sza.18m",   0x1000004, 0x100000, CRC(4bc3c8bc) SHA1(6256963c515bf56f39b6e559afefd653ead56c54) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sza.20m",   0x1000006, 0x100000, CRC(39e674c0) SHA1(8e771a2d8c2accad0463bccd21d7b23af0c895a1) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sza.01",   0x00000, 0x08000, CRC(1bc323cf) SHA1(83fbd6e9b327700dc9f1c59700b7385bc3705749) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "sza.02",   0x28000, 0x20000, CRC(ba6a5013) SHA1(7814f3e56b69529b9860dd61c3b1e8d700244b03) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "sza.11m",   0x000000, 0x200000, CRC(aa47a601) SHA1(a4d1ee89c84a3b9db06469bb66e85293b5aa9ac9) )
	ROM_LOAD16_WORD_SWAP( "sza.12m",   0x200000, 0x200000, CRC(2237bc53) SHA1(96d5693047e4cf1ed10a8ee1905cea267a278e92) )
ROM_END

ROM_START( sfa3 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "sz3e.03c", 0x000000, 0x80000, CRC(9762b206) SHA1(fc4561ca990dd11ed2c5203540102078b721db2f) )
	ROM_LOAD16_WORD_SWAP( "sz3e.04c", 0x080000, 0x80000, CRC(5ad3f721) SHA1(fec11cc5f63593f0181eb28ae85ad916686793eb) )
	ROM_LOAD16_WORD_SWAP( "sz3.05c",  0x100000, 0x80000, CRC(57fd0a40) SHA1(bc2d5f4d57117bbf58b1adb088e00424ef489e92) )
	ROM_LOAD16_WORD_SWAP( "sz3.06c",  0x180000, 0x80000, CRC(f6305f8b) SHA1(3fd1ebdbad96103aca604e950b488e52460a71ec) )
	ROM_LOAD16_WORD_SWAP( "sz3.07c",  0x200000, 0x80000, CRC(6eab0f6f) SHA1(f8d093dda65cf4e8a3000dc1b96355bb03dcb495) )
	ROM_LOAD16_WORD_SWAP( "sz3.08c",  0x280000, 0x80000, CRC(910c4a3b) SHA1(dbd41280f9b16ad6a5b8f12092549970349395f1) )
	ROM_LOAD16_WORD_SWAP( "sz3.09c",  0x300000, 0x80000, CRC(b29e5199) SHA1(c6c215eb5aa37f678a9cafcbd8620969fb5ca12f) )
	ROM_LOAD16_WORD_SWAP( "sz3.10b",  0x380000, 0x80000, CRC(deb2ff52) SHA1(0aa4722aad68a04164946c78bf05752f947b4322) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "sz3.13m",   0x0000000, 0x400000, CRC(0f7a60d9) SHA1(c69e0ee22537312909dacc86d2e4be319d54e426) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.15m",   0x0000002, 0x400000, CRC(8e933741) SHA1(f4ac4bfe830dc7df9fe4f680e4e0c053e7cbd8fe) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.17m",   0x0000004, 0x400000, CRC(d6e98147) SHA1(37f331fbb1284db446faecade6f484f58c0e1b2a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.19m",   0x0000006, 0x400000, CRC(f31a728a) SHA1(f14136564648f006c1b74afda78349f260524b5f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.14m",   0x1000000, 0x400000, CRC(5ff98297) SHA1(9e0ce43380b776c7a03872bafd4856f6fa60bda7) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.16m",   0x1000002, 0x400000, CRC(52b5bdee) SHA1(7918204dc457f7a146d8fb8cf7242dfed3109fd8) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.18m",   0x1000004, 0x400000, CRC(40631ed5) SHA1(c18c56822b90a71ca5fbdf3440eb2671011f3d8f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.20m",   0x1000006, 0x400000, CRC(763409b4) SHA1(af60a5116c1ca9050366a35ea29128921867f3cc) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sz3.01",   0x00000, 0x08000, CRC(de810084) SHA1(fd0b969b732921ed8b40c16fbfa30ee09c7a7cbd) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "sz3.02",   0x28000, 0x20000, CRC(72445dc4) SHA1(14fca7596ac45ba655016eef5b6120f9f9671c23) )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "sz3.11m",   0x000000, 0x400000, CRC(1c89eed1) SHA1(649a0b0a3eb72e2e69e9fb1ac51a58b70daa39f3) )
	ROM_LOAD16_WORD_SWAP( "sz3.12m",   0x400000, 0x400000, CRC(f392b13a) SHA1(fa04ce0370144a49bd1d5acd873eef87b0dc9d15) )
ROM_END

ROM_START( sfa3u )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "sz3u.03c", 0x000000, 0x80000, CRC(e007da2e) SHA1(d190ac7ca2c27f11b9b4f96860b226bbea0ee403) )
	ROM_LOAD16_WORD_SWAP( "sz3u.04c", 0x080000, 0x80000, CRC(5f78f0e7) SHA1(f4df30fd3515fe9f1125f470b96028052c61f57b) )
	ROM_LOAD16_WORD_SWAP( "sz3.05c",  0x100000, 0x80000, CRC(57fd0a40) SHA1(bc2d5f4d57117bbf58b1adb088e00424ef489e92) )
	ROM_LOAD16_WORD_SWAP( "sz3.06c",  0x180000, 0x80000, CRC(f6305f8b) SHA1(3fd1ebdbad96103aca604e950b488e52460a71ec) )
	ROM_LOAD16_WORD_SWAP( "sz3.07c",  0x200000, 0x80000, CRC(6eab0f6f) SHA1(f8d093dda65cf4e8a3000dc1b96355bb03dcb495) )
	ROM_LOAD16_WORD_SWAP( "sz3.08c",  0x280000, 0x80000, CRC(910c4a3b) SHA1(dbd41280f9b16ad6a5b8f12092549970349395f1) )
	ROM_LOAD16_WORD_SWAP( "sz3.09c",  0x300000, 0x80000, CRC(b29e5199) SHA1(c6c215eb5aa37f678a9cafcbd8620969fb5ca12f) )
	ROM_LOAD16_WORD_SWAP( "sz3.10b",  0x380000, 0x80000, CRC(deb2ff52) SHA1(0aa4722aad68a04164946c78bf05752f947b4322) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "sz3.13m",   0x0000000, 0x400000, CRC(0f7a60d9) SHA1(c69e0ee22537312909dacc86d2e4be319d54e426) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.15m",   0x0000002, 0x400000, CRC(8e933741) SHA1(f4ac4bfe830dc7df9fe4f680e4e0c053e7cbd8fe) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.17m",   0x0000004, 0x400000, CRC(d6e98147) SHA1(37f331fbb1284db446faecade6f484f58c0e1b2a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.19m",   0x0000006, 0x400000, CRC(f31a728a) SHA1(f14136564648f006c1b74afda78349f260524b5f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.14m",   0x1000000, 0x400000, CRC(5ff98297) SHA1(9e0ce43380b776c7a03872bafd4856f6fa60bda7) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.16m",   0x1000002, 0x400000, CRC(52b5bdee) SHA1(7918204dc457f7a146d8fb8cf7242dfed3109fd8) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.18m",   0x1000004, 0x400000, CRC(40631ed5) SHA1(c18c56822b90a71ca5fbdf3440eb2671011f3d8f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.20m",   0x1000006, 0x400000, CRC(763409b4) SHA1(af60a5116c1ca9050366a35ea29128921867f3cc) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sz3.01",   0x00000, 0x08000, CRC(de810084) SHA1(fd0b969b732921ed8b40c16fbfa30ee09c7a7cbd) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "sz3.02",   0x28000, 0x20000, CRC(72445dc4) SHA1(14fca7596ac45ba655016eef5b6120f9f9671c23) )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "sz3.11m",   0x000000, 0x400000, CRC(1c89eed1) SHA1(649a0b0a3eb72e2e69e9fb1ac51a58b70daa39f3) )
	ROM_LOAD16_WORD_SWAP( "sz3.12m",   0x400000, 0x400000, CRC(f392b13a) SHA1(fa04ce0370144a49bd1d5acd873eef87b0dc9d15) )
ROM_END

ROM_START( sfa3ur1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "sz3u.03", 0x000000, 0x80000, CRC(b5984a19) SHA1(e225dd1d3a5d1b94adcfc5f720775e9ba321996e) )
	ROM_LOAD16_WORD_SWAP( "sz3u.04", 0x080000, 0x80000, CRC(7e8158ba) SHA1(a9984d7c9d02a9ebaf98cfd0dcbcf26e82e904de) )
	ROM_LOAD16_WORD_SWAP( "sz3.05",  0x100000, 0x80000, CRC(9b21518a) SHA1(5a928307cb90a98a62e7598cb101fb66d62b85f9) )
	ROM_LOAD16_WORD_SWAP( "sz3.06",  0x180000, 0x80000, CRC(e7a6c3a7) SHA1(63441eb19efcbf9149f4b723d3e9191fa972de2a) )
	ROM_LOAD16_WORD_SWAP( "sz3.07",  0x200000, 0x80000, CRC(ec4c0cfd) SHA1(1a5148e77bf633c728a8179dacb59c776f981bc4) )
	ROM_LOAD16_WORD_SWAP( "sz3.08",  0x280000, 0x80000, CRC(5c7e7240) SHA1(33bdcdd1889f8fa77916373ed33b0854410d0263) )
	ROM_LOAD16_WORD_SWAP( "sz3.09",  0x300000, 0x80000, CRC(c5589553) SHA1(cda1fdc2ab2f390a2358defd9923a2796093926d) )
	ROM_LOAD16_WORD_SWAP( "sz3.10",  0x380000, 0x80000, CRC(a9717252) SHA1(7ee94ace2a49e4e5d30474e49c0da04a488010fe) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "sz3.13m",   0x0000000, 0x400000, CRC(0f7a60d9) SHA1(c69e0ee22537312909dacc86d2e4be319d54e426) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.15m",   0x0000002, 0x400000, CRC(8e933741) SHA1(f4ac4bfe830dc7df9fe4f680e4e0c053e7cbd8fe) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.17m",   0x0000004, 0x400000, CRC(d6e98147) SHA1(37f331fbb1284db446faecade6f484f58c0e1b2a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.19m",   0x0000006, 0x400000, CRC(f31a728a) SHA1(f14136564648f006c1b74afda78349f260524b5f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.14m",   0x1000000, 0x400000, CRC(5ff98297) SHA1(9e0ce43380b776c7a03872bafd4856f6fa60bda7) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.16m",   0x1000002, 0x400000, CRC(52b5bdee) SHA1(7918204dc457f7a146d8fb8cf7242dfed3109fd8) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.18m",   0x1000004, 0x400000, CRC(40631ed5) SHA1(c18c56822b90a71ca5fbdf3440eb2671011f3d8f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.20m",   0x1000006, 0x400000, CRC(763409b4) SHA1(af60a5116c1ca9050366a35ea29128921867f3cc) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sz3.01",   0x00000, 0x08000, CRC(de810084) SHA1(fd0b969b732921ed8b40c16fbfa30ee09c7a7cbd) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "sz3.02",   0x28000, 0x20000, CRC(72445dc4) SHA1(14fca7596ac45ba655016eef5b6120f9f9671c23) )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "sz3.11m",   0x000000, 0x400000, CRC(1c89eed1) SHA1(649a0b0a3eb72e2e69e9fb1ac51a58b70daa39f3) )
	ROM_LOAD16_WORD_SWAP( "sz3.12m",   0x400000, 0x400000, CRC(f392b13a) SHA1(fa04ce0370144a49bd1d5acd873eef87b0dc9d15) )
ROM_END

ROM_START( sfz3j )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "sz3j.03c", 0x000000, 0x80000, CRC(cadf4a51) SHA1(a0511512f55c0befa4a905ceff8c6f5775cf40ba) )
	ROM_LOAD16_WORD_SWAP( "sz3j.04c", 0x080000, 0x80000, CRC(fcb31228) SHA1(093f40083b5a4e4cae433d5856c48014063fe6ad) )
	ROM_LOAD16_WORD_SWAP( "sz3.05c",  0x100000, 0x80000, CRC(57fd0a40) SHA1(bc2d5f4d57117bbf58b1adb088e00424ef489e92) )
	ROM_LOAD16_WORD_SWAP( "sz3.06c",  0x180000, 0x80000, CRC(f6305f8b) SHA1(3fd1ebdbad96103aca604e950b488e52460a71ec) )
	ROM_LOAD16_WORD_SWAP( "sz3.07c",  0x200000, 0x80000, CRC(6eab0f6f) SHA1(f8d093dda65cf4e8a3000dc1b96355bb03dcb495) )
	ROM_LOAD16_WORD_SWAP( "sz3.08c",  0x280000, 0x80000, CRC(910c4a3b) SHA1(dbd41280f9b16ad6a5b8f12092549970349395f1) )
	ROM_LOAD16_WORD_SWAP( "sz3.09c",  0x300000, 0x80000, CRC(b29e5199) SHA1(c6c215eb5aa37f678a9cafcbd8620969fb5ca12f) )
	ROM_LOAD16_WORD_SWAP( "sz3.10b",  0x380000, 0x80000, CRC(deb2ff52) SHA1(0aa4722aad68a04164946c78bf05752f947b4322) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "sz3.13m",   0x0000000, 0x400000, CRC(0f7a60d9) SHA1(c69e0ee22537312909dacc86d2e4be319d54e426) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.15m",   0x0000002, 0x400000, CRC(8e933741) SHA1(f4ac4bfe830dc7df9fe4f680e4e0c053e7cbd8fe) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.17m",   0x0000004, 0x400000, CRC(d6e98147) SHA1(37f331fbb1284db446faecade6f484f58c0e1b2a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.19m",   0x0000006, 0x400000, CRC(f31a728a) SHA1(f14136564648f006c1b74afda78349f260524b5f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.14m",   0x1000000, 0x400000, CRC(5ff98297) SHA1(9e0ce43380b776c7a03872bafd4856f6fa60bda7) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.16m",   0x1000002, 0x400000, CRC(52b5bdee) SHA1(7918204dc457f7a146d8fb8cf7242dfed3109fd8) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.18m",   0x1000004, 0x400000, CRC(40631ed5) SHA1(c18c56822b90a71ca5fbdf3440eb2671011f3d8f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.20m",   0x1000006, 0x400000, CRC(763409b4) SHA1(af60a5116c1ca9050366a35ea29128921867f3cc) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sz3.01",   0x00000, 0x08000, CRC(de810084) SHA1(fd0b969b732921ed8b40c16fbfa30ee09c7a7cbd) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "sz3.02",   0x28000, 0x20000, CRC(72445dc4) SHA1(14fca7596ac45ba655016eef5b6120f9f9671c23) )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "sz3.11m",   0x000000, 0x400000, CRC(1c89eed1) SHA1(649a0b0a3eb72e2e69e9fb1ac51a58b70daa39f3) )
	ROM_LOAD16_WORD_SWAP( "sz3.12m",   0x400000, 0x400000, CRC(f392b13a) SHA1(fa04ce0370144a49bd1d5acd873eef87b0dc9d15) )
ROM_END

ROM_START( sfz3jr1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "sz3j.03a", 0x000000, 0x80000, CRC(6ee0beae) SHA1(243436fb64628f70cd130c7029d365ae97f3f42d) )
	ROM_LOAD16_WORD_SWAP( "sz3j.04a", 0x080000, 0x80000, CRC(a6e2978d) SHA1(27e350e78aa204670c0ee6c60baddee46a92a584) )
	ROM_LOAD16_WORD_SWAP( "sz3.05a",  0x100000, 0x80000, CRC(05964b7d) SHA1(ac9fa2c69c712a01647f0572381d875b1eb90886) )
	ROM_LOAD16_WORD_SWAP( "sz3.06a",  0x180000, 0x80000, CRC(78ce2179) SHA1(98a6f55bbdc45167fcc04cd6c3b7d71ffab31911) )
	ROM_LOAD16_WORD_SWAP( "sz3.07a",  0x200000, 0x80000, CRC(398bf52f) SHA1(2c8880b65b83724b956294b903b5038091b543c5) )
	ROM_LOAD16_WORD_SWAP( "sz3.08a",  0x280000, 0x80000, CRC(866d0588) SHA1(f2e9ca1bb606e4d2e3c9b62dd80074670a2e8e45) )
	ROM_LOAD16_WORD_SWAP( "sz3.09a",  0x300000, 0x80000, CRC(2180892c) SHA1(65a44c612b1c6dd527b306c262caa5040897ce7b) )
	ROM_LOAD16_WORD_SWAP( "sz3.10",   0x380000, 0x80000, CRC(a9717252) SHA1(7ee94ace2a49e4e5d30474e49c0da04a488010fe) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "sz3.13m",   0x0000000, 0x400000, CRC(0f7a60d9) SHA1(c69e0ee22537312909dacc86d2e4be319d54e426) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.15m",   0x0000002, 0x400000, CRC(8e933741) SHA1(f4ac4bfe830dc7df9fe4f680e4e0c053e7cbd8fe) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.17m",   0x0000004, 0x400000, CRC(d6e98147) SHA1(37f331fbb1284db446faecade6f484f58c0e1b2a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.19m",   0x0000006, 0x400000, CRC(f31a728a) SHA1(f14136564648f006c1b74afda78349f260524b5f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.14m",   0x1000000, 0x400000, CRC(5ff98297) SHA1(9e0ce43380b776c7a03872bafd4856f6fa60bda7) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.16m",   0x1000002, 0x400000, CRC(52b5bdee) SHA1(7918204dc457f7a146d8fb8cf7242dfed3109fd8) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.18m",   0x1000004, 0x400000, CRC(40631ed5) SHA1(c18c56822b90a71ca5fbdf3440eb2671011f3d8f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.20m",   0x1000006, 0x400000, CRC(763409b4) SHA1(af60a5116c1ca9050366a35ea29128921867f3cc) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sz3.01",   0x00000, 0x08000, CRC(de810084) SHA1(fd0b969b732921ed8b40c16fbfa30ee09c7a7cbd) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "sz3.02",   0x28000, 0x20000, CRC(72445dc4) SHA1(14fca7596ac45ba655016eef5b6120f9f9671c23) )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "sz3.11m",   0x000000, 0x400000, CRC(1c89eed1) SHA1(649a0b0a3eb72e2e69e9fb1ac51a58b70daa39f3) )
	ROM_LOAD16_WORD_SWAP( "sz3.12m",   0x400000, 0x400000, CRC(f392b13a) SHA1(fa04ce0370144a49bd1d5acd873eef87b0dc9d15) )
ROM_END

ROM_START( sfz3jr2 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "sz3j.03", 0x000000, 0x80000, CRC(f7cb4b13) SHA1(5f86d23cf3725d9440200732405b437545ac8dd7) )
	ROM_LOAD16_WORD_SWAP( "sz3j.04", 0x080000, 0x80000, CRC(0846c29d) SHA1(f2e96b4f6e0187c382411276ff3a485cdc2df289) )
	ROM_LOAD16_WORD_SWAP( "sz3.05",  0x100000, 0x80000, CRC(9b21518a) SHA1(5a928307cb90a98a62e7598cb101fb66d62b85f9) )
	ROM_LOAD16_WORD_SWAP( "sz3.06",  0x180000, 0x80000, CRC(e7a6c3a7) SHA1(63441eb19efcbf9149f4b723d3e9191fa972de2a) )
	ROM_LOAD16_WORD_SWAP( "sz3.07",  0x200000, 0x80000, CRC(ec4c0cfd) SHA1(1a5148e77bf633c728a8179dacb59c776f981bc4) )
	ROM_LOAD16_WORD_SWAP( "sz3.08",  0x280000, 0x80000, CRC(5c7e7240) SHA1(33bdcdd1889f8fa77916373ed33b0854410d0263) )
	ROM_LOAD16_WORD_SWAP( "sz3.09",  0x300000, 0x80000, CRC(c5589553) SHA1(cda1fdc2ab2f390a2358defd9923a2796093926d) )
	ROM_LOAD16_WORD_SWAP( "sz3.10",  0x380000, 0x80000, CRC(a9717252) SHA1(7ee94ace2a49e4e5d30474e49c0da04a488010fe) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "sz3.13m",   0x0000000, 0x400000, CRC(0f7a60d9) SHA1(c69e0ee22537312909dacc86d2e4be319d54e426) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.15m",   0x0000002, 0x400000, CRC(8e933741) SHA1(f4ac4bfe830dc7df9fe4f680e4e0c053e7cbd8fe) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.17m",   0x0000004, 0x400000, CRC(d6e98147) SHA1(37f331fbb1284db446faecade6f484f58c0e1b2a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.19m",   0x0000006, 0x400000, CRC(f31a728a) SHA1(f14136564648f006c1b74afda78349f260524b5f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.14m",   0x1000000, 0x400000, CRC(5ff98297) SHA1(9e0ce43380b776c7a03872bafd4856f6fa60bda7) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.16m",   0x1000002, 0x400000, CRC(52b5bdee) SHA1(7918204dc457f7a146d8fb8cf7242dfed3109fd8) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.18m",   0x1000004, 0x400000, CRC(40631ed5) SHA1(c18c56822b90a71ca5fbdf3440eb2671011f3d8f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.20m",   0x1000006, 0x400000, CRC(763409b4) SHA1(af60a5116c1ca9050366a35ea29128921867f3cc) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sz3.01",   0x00000, 0x08000, CRC(de810084) SHA1(fd0b969b732921ed8b40c16fbfa30ee09c7a7cbd) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "sz3.02",   0x28000, 0x20000, CRC(72445dc4) SHA1(14fca7596ac45ba655016eef5b6120f9f9671c23) )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "sz3.11m",   0x000000, 0x400000, CRC(1c89eed1) SHA1(649a0b0a3eb72e2e69e9fb1ac51a58b70daa39f3) )
	ROM_LOAD16_WORD_SWAP( "sz3.12m",   0x400000, 0x400000, CRC(f392b13a) SHA1(fa04ce0370144a49bd1d5acd873eef87b0dc9d15) )
ROM_END

ROM_START( sfz3a )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "sz3a.03d", 0x000000, 0x80000, CRC(d7e140d6) SHA1(7eae4dc61432e1aaf73194377f787093379d53a9) )
	ROM_LOAD16_WORD_SWAP( "sz3a.04d", 0x080000, 0x80000, CRC(e06869a2) SHA1(9442f7dda95d20c5eff549bfdd60b89eea5483a9) )
	ROM_LOAD16_WORD_SWAP( "sz3.05c",  0x100000, 0x80000, CRC(57fd0a40) SHA1(bc2d5f4d57117bbf58b1adb088e00424ef489e92) )
	ROM_LOAD16_WORD_SWAP( "sz3.06c",  0x180000, 0x80000, CRC(f6305f8b) SHA1(3fd1ebdbad96103aca604e950b488e52460a71ec) )
	ROM_LOAD16_WORD_SWAP( "sz3.07c",  0x200000, 0x80000, CRC(6eab0f6f) SHA1(f8d093dda65cf4e8a3000dc1b96355bb03dcb495) )
	ROM_LOAD16_WORD_SWAP( "sz3.08c",  0x280000, 0x80000, CRC(910c4a3b) SHA1(dbd41280f9b16ad6a5b8f12092549970349395f1) )
	ROM_LOAD16_WORD_SWAP( "sz3.09c",  0x300000, 0x80000, CRC(b29e5199) SHA1(c6c215eb5aa37f678a9cafcbd8620969fb5ca12f) )
	ROM_LOAD16_WORD_SWAP( "sz3.10b",  0x380000, 0x80000, CRC(deb2ff52) SHA1(0aa4722aad68a04164946c78bf05752f947b4322) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "sz3.13m",   0x0000000, 0x400000, CRC(0f7a60d9) SHA1(c69e0ee22537312909dacc86d2e4be319d54e426) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.15m",   0x0000002, 0x400000, CRC(8e933741) SHA1(f4ac4bfe830dc7df9fe4f680e4e0c053e7cbd8fe) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.17m",   0x0000004, 0x400000, CRC(d6e98147) SHA1(37f331fbb1284db446faecade6f484f58c0e1b2a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.19m",   0x0000006, 0x400000, CRC(f31a728a) SHA1(f14136564648f006c1b74afda78349f260524b5f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.14m",   0x1000000, 0x400000, CRC(5ff98297) SHA1(9e0ce43380b776c7a03872bafd4856f6fa60bda7) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.16m",   0x1000002, 0x400000, CRC(52b5bdee) SHA1(7918204dc457f7a146d8fb8cf7242dfed3109fd8) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.18m",   0x1000004, 0x400000, CRC(40631ed5) SHA1(c18c56822b90a71ca5fbdf3440eb2671011f3d8f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.20m",   0x1000006, 0x400000, CRC(763409b4) SHA1(af60a5116c1ca9050366a35ea29128921867f3cc) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sz3.01",   0x00000, 0x08000, CRC(de810084) SHA1(fd0b969b732921ed8b40c16fbfa30ee09c7a7cbd) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "sz3.02",   0x28000, 0x20000, CRC(72445dc4) SHA1(14fca7596ac45ba655016eef5b6120f9f9671c23) )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "sz3.11m",   0x000000, 0x400000, CRC(1c89eed1) SHA1(649a0b0a3eb72e2e69e9fb1ac51a58b70daa39f3) )
	ROM_LOAD16_WORD_SWAP( "sz3.12m",   0x400000, 0x400000, CRC(f392b13a) SHA1(fa04ce0370144a49bd1d5acd873eef87b0dc9d15) )
ROM_END

ROM_START( sfz3ar1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "sz3a.03a", 0x000000, 0x80000, CRC(29c681fd) SHA1(5ee4c4e282789e4cdba5a317c7049e8c0d8b774b) )
	ROM_LOAD16_WORD_SWAP( "sz3a.04",  0x080000, 0x80000, CRC(9ddd1484) SHA1(d484b93d1653f522ee33285e58139167b3214902) )
	ROM_LOAD16_WORD_SWAP( "sz3.05",   0x100000, 0x80000, CRC(9b21518a) SHA1(5a928307cb90a98a62e7598cb101fb66d62b85f9) )
	ROM_LOAD16_WORD_SWAP( "sz3.06",   0x180000, 0x80000, CRC(e7a6c3a7) SHA1(63441eb19efcbf9149f4b723d3e9191fa972de2a) )
	ROM_LOAD16_WORD_SWAP( "sz3.07",   0x200000, 0x80000, CRC(ec4c0cfd) SHA1(1a5148e77bf633c728a8179dacb59c776f981bc4) )
	ROM_LOAD16_WORD_SWAP( "sz3.08",   0x280000, 0x80000, CRC(5c7e7240) SHA1(33bdcdd1889f8fa77916373ed33b0854410d0263) )
	ROM_LOAD16_WORD_SWAP( "sz3.09",   0x300000, 0x80000, CRC(c5589553) SHA1(cda1fdc2ab2f390a2358defd9923a2796093926d) )
	ROM_LOAD16_WORD_SWAP( "sz3.10",   0x380000, 0x80000, CRC(a9717252) SHA1(7ee94ace2a49e4e5d30474e49c0da04a488010fe) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "sz3.13m",   0x0000000, 0x400000, CRC(0f7a60d9) SHA1(c69e0ee22537312909dacc86d2e4be319d54e426) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.15m",   0x0000002, 0x400000, CRC(8e933741) SHA1(f4ac4bfe830dc7df9fe4f680e4e0c053e7cbd8fe) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.17m",   0x0000004, 0x400000, CRC(d6e98147) SHA1(37f331fbb1284db446faecade6f484f58c0e1b2a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.19m",   0x0000006, 0x400000, CRC(f31a728a) SHA1(f14136564648f006c1b74afda78349f260524b5f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.14m",   0x1000000, 0x400000, CRC(5ff98297) SHA1(9e0ce43380b776c7a03872bafd4856f6fa60bda7) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.16m",   0x1000002, 0x400000, CRC(52b5bdee) SHA1(7918204dc457f7a146d8fb8cf7242dfed3109fd8) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.18m",   0x1000004, 0x400000, CRC(40631ed5) SHA1(c18c56822b90a71ca5fbdf3440eb2671011f3d8f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.20m",   0x1000006, 0x400000, CRC(763409b4) SHA1(af60a5116c1ca9050366a35ea29128921867f3cc) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sz3.01",   0x00000, 0x08000, CRC(de810084) SHA1(fd0b969b732921ed8b40c16fbfa30ee09c7a7cbd) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "sz3.02",   0x28000, 0x20000, CRC(72445dc4) SHA1(14fca7596ac45ba655016eef5b6120f9f9671c23) )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "sz3.11m",   0x000000, 0x400000, CRC(1c89eed1) SHA1(649a0b0a3eb72e2e69e9fb1ac51a58b70daa39f3) )
	ROM_LOAD16_WORD_SWAP( "sz3.12m",   0x400000, 0x400000, CRC(f392b13a) SHA1(fa04ce0370144a49bd1d5acd873eef87b0dc9d15) )
ROM_END

ROM_START( sfa3h )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "sz3h.03", 0x000000, 0x80000, CRC(4b16cb3e) SHA1(2e1f45c9076ce231a545146d475f410397f20e27) )
	ROM_LOAD16_WORD_SWAP( "sz3h.04", 0x080000, 0x80000, CRC(88ad2e6a) SHA1(2a1de667ebeffd247ab9ee11687813b3397fc8de) )
	ROM_LOAD16_WORD_SWAP( "sz3.05",  0x100000, 0x80000, CRC(9b21518a) SHA1(5a928307cb90a98a62e7598cb101fb66d62b85f9) )
	ROM_LOAD16_WORD_SWAP( "sz3.06",  0x180000, 0x80000, CRC(e7a6c3a7) SHA1(63441eb19efcbf9149f4b723d3e9191fa972de2a) )
	ROM_LOAD16_WORD_SWAP( "sz3.07",  0x200000, 0x80000, CRC(ec4c0cfd) SHA1(1a5148e77bf633c728a8179dacb59c776f981bc4) )
	ROM_LOAD16_WORD_SWAP( "sz3.08",  0x280000, 0x80000, CRC(5c7e7240) SHA1(33bdcdd1889f8fa77916373ed33b0854410d0263) )
	ROM_LOAD16_WORD_SWAP( "sz3.09",  0x300000, 0x80000, CRC(c5589553) SHA1(cda1fdc2ab2f390a2358defd9923a2796093926d) )
	ROM_LOAD16_WORD_SWAP( "sz3.10",  0x380000, 0x80000, CRC(a9717252) SHA1(7ee94ace2a49e4e5d30474e49c0da04a488010fe) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "sz3.13m",   0x0000000, 0x400000, CRC(0f7a60d9) SHA1(c69e0ee22537312909dacc86d2e4be319d54e426) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.15m",   0x0000002, 0x400000, CRC(8e933741) SHA1(f4ac4bfe830dc7df9fe4f680e4e0c053e7cbd8fe) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.17m",   0x0000004, 0x400000, CRC(d6e98147) SHA1(37f331fbb1284db446faecade6f484f58c0e1b2a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.19m",   0x0000006, 0x400000, CRC(f31a728a) SHA1(f14136564648f006c1b74afda78349f260524b5f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.14m",   0x1000000, 0x400000, CRC(5ff98297) SHA1(9e0ce43380b776c7a03872bafd4856f6fa60bda7) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.16m",   0x1000002, 0x400000, CRC(52b5bdee) SHA1(7918204dc457f7a146d8fb8cf7242dfed3109fd8) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.18m",   0x1000004, 0x400000, CRC(40631ed5) SHA1(c18c56822b90a71ca5fbdf3440eb2671011f3d8f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.20m",   0x1000006, 0x400000, CRC(763409b4) SHA1(af60a5116c1ca9050366a35ea29128921867f3cc) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sz3.01",   0x00000, 0x08000, CRC(de810084) SHA1(fd0b969b732921ed8b40c16fbfa30ee09c7a7cbd) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "sz3.02",   0x28000, 0x20000, CRC(72445dc4) SHA1(14fca7596ac45ba655016eef5b6120f9f9671c23) )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "sz3.11m",   0x000000, 0x400000, CRC(1c89eed1) SHA1(649a0b0a3eb72e2e69e9fb1ac51a58b70daa39f3) )
	ROM_LOAD16_WORD_SWAP( "sz3.12m",   0x400000, 0x400000, CRC(f392b13a) SHA1(fa04ce0370144a49bd1d5acd873eef87b0dc9d15) )
ROM_END

ROM_START( sfa3b )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "sz3b.03", 0x000000, 0x80000, CRC(046c9b4d) SHA1(aa2b19f2d4a9bab6e273635b43da7538025f9d77) )
	ROM_LOAD16_WORD_SWAP( "sz3b.04", 0x080000, 0x80000, CRC(da211919) SHA1(bffeca36c9c78168f44e288ba34bb682a1626f8f) )
	ROM_LOAD16_WORD_SWAP( "sz3.05",  0x100000, 0x80000, CRC(9b21518a) SHA1(5a928307cb90a98a62e7598cb101fb66d62b85f9) )
	ROM_LOAD16_WORD_SWAP( "sz3.06",  0x180000, 0x80000, CRC(e7a6c3a7) SHA1(63441eb19efcbf9149f4b723d3e9191fa972de2a) )
	ROM_LOAD16_WORD_SWAP( "sz3.07",  0x200000, 0x80000, CRC(ec4c0cfd) SHA1(1a5148e77bf633c728a8179dacb59c776f981bc4) )
	ROM_LOAD16_WORD_SWAP( "sz3.08",  0x280000, 0x80000, CRC(5c7e7240) SHA1(33bdcdd1889f8fa77916373ed33b0854410d0263) )
	ROM_LOAD16_WORD_SWAP( "sz3.09",  0x300000, 0x80000, CRC(c5589553) SHA1(cda1fdc2ab2f390a2358defd9923a2796093926d) )
	ROM_LOAD16_WORD_SWAP( "sz3.10",  0x380000, 0x80000, CRC(a9717252) SHA1(7ee94ace2a49e4e5d30474e49c0da04a488010fe) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "sz3.13m",   0x0000000, 0x400000, CRC(0f7a60d9) SHA1(c69e0ee22537312909dacc86d2e4be319d54e426) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.15m",   0x0000002, 0x400000, CRC(8e933741) SHA1(f4ac4bfe830dc7df9fe4f680e4e0c053e7cbd8fe) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.17m",   0x0000004, 0x400000, CRC(d6e98147) SHA1(37f331fbb1284db446faecade6f484f58c0e1b2a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.19m",   0x0000006, 0x400000, CRC(f31a728a) SHA1(f14136564648f006c1b74afda78349f260524b5f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.14m",   0x1000000, 0x400000, CRC(5ff98297) SHA1(9e0ce43380b776c7a03872bafd4856f6fa60bda7) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.16m",   0x1000002, 0x400000, CRC(52b5bdee) SHA1(7918204dc457f7a146d8fb8cf7242dfed3109fd8) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.18m",   0x1000004, 0x400000, CRC(40631ed5) SHA1(c18c56822b90a71ca5fbdf3440eb2671011f3d8f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.20m",   0x1000006, 0x400000, CRC(763409b4) SHA1(af60a5116c1ca9050366a35ea29128921867f3cc) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sz3.01",   0x00000, 0x08000, CRC(de810084) SHA1(fd0b969b732921ed8b40c16fbfa30ee09c7a7cbd) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "sz3.02",   0x28000, 0x20000, CRC(72445dc4) SHA1(14fca7596ac45ba655016eef5b6120f9f9671c23) )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "sz3.11m",   0x000000, 0x400000, CRC(1c89eed1) SHA1(649a0b0a3eb72e2e69e9fb1ac51a58b70daa39f3) )
	ROM_LOAD16_WORD_SWAP( "sz3.12m",   0x400000, 0x400000, CRC(f392b13a) SHA1(fa04ce0370144a49bd1d5acd873eef87b0dc9d15) )
ROM_END

ROM_START( sgemf )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "pcfu.03", 0x000000, 0x80000, CRC(ac2e8566) SHA1(5975aae46bded231c0f478f40c7257434ade36b0) )
	ROM_LOAD16_WORD_SWAP( "pcf.04",  0x080000, 0x80000, CRC(f4314c96) SHA1(c40ed74039bf0096eb3648b7243a8e697638e0a6) )
	ROM_LOAD16_WORD_SWAP( "pcf.05",  0x100000, 0x80000, CRC(215655f6) SHA1(242c0f4401520f2a3b0deafc3a807b18b987e496) )
	ROM_LOAD16_WORD_SWAP( "pcf.06",  0x180000, 0x80000, CRC(ea6f13ea) SHA1(1bc924a8a9da1d2ad7667685cdb92fe317a39aba) )
	ROM_LOAD16_WORD_SWAP( "pcf.07",  0x200000, 0x80000, CRC(5ac6d5ea) SHA1(9ce8e4668b565658597a868830545fb75a5eeaa6) )

	ROM_REGION( 0x1400000, "gfx", 0 )
	ROMX_LOAD( "pcf.13m",   0x0000000, 0x400000, CRC(22d72ab9) SHA1(653efd95c34b4b9d2ab0d219f41a99ca84e12214) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pcf.15m",   0x0000002, 0x400000, CRC(16a4813c) SHA1(bf5fce6008214f353414d1b64bea4ed0c7673670) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pcf.17m",   0x0000004, 0x400000, CRC(1097e035) SHA1(4bd51e4e9447af27d2cac1f6d2201e37c949912b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pcf.19m",   0x0000006, 0x400000, CRC(d362d874) SHA1(30c42af18440496cc05e4418e4efa41172ae4ced) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pcf.14m",   0x1000000, 0x100000, CRC(0383897c) SHA1(aba14afa1d0c971afcee4317f480e88117d77b5e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pcf.16m",   0x1000002, 0x100000, CRC(76f91084) SHA1(3d1e32467f2aa5dd6fb96bd5c866ecc9691660fc) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pcf.18m",   0x1000004, 0x100000, CRC(756c3754) SHA1(be2f709b90222a567f198f851cf07ffb0ad433d7) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pcf.20m",   0x1000006, 0x100000, CRC(9ec9277d) SHA1(b7ceeaca30dfcdf498b61a6961f0aa1a068b8ec4) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "pcf.01",   0x00000, 0x08000, CRC(254e5f33) SHA1(c413ec0630b9bdd15e64f42893eba8958a09b573) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "pcf.02",   0x28000, 0x20000, CRC(6902f4f9) SHA1(9bfe4ddade3c666076d26a2b545120f6d059fd7c) )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "pcf.11m",   0x000000, 0x400000, CRC(a5dea005) SHA1(3ae79baf6ff5bd527f82b26f164c7e3c65423ae2) )
	ROM_LOAD16_WORD_SWAP( "pcf.12m",   0x400000, 0x400000, CRC(4ce235fe) SHA1(795b94557e954cc0e45fd3778b609064d57a34a2) )
ROM_END

ROM_START( pfghtj )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "pcfj.03", 0x000000, 0x80000, CRC(681da43e) SHA1(1bd4b6b395ac7208c0208b254455276719e98c4b) )
	ROM_LOAD16_WORD_SWAP( "pcf.04",  0x080000, 0x80000, CRC(f4314c96) SHA1(c40ed74039bf0096eb3648b7243a8e697638e0a6) )
	ROM_LOAD16_WORD_SWAP( "pcf.05",  0x100000, 0x80000, CRC(215655f6) SHA1(242c0f4401520f2a3b0deafc3a807b18b987e496) )
	ROM_LOAD16_WORD_SWAP( "pcf.06",  0x180000, 0x80000, CRC(ea6f13ea) SHA1(1bc924a8a9da1d2ad7667685cdb92fe317a39aba) )
	ROM_LOAD16_WORD_SWAP( "pcf.07",  0x200000, 0x80000, CRC(5ac6d5ea) SHA1(9ce8e4668b565658597a868830545fb75a5eeaa6) )

	ROM_REGION( 0x1400000, "gfx", 0 )
	ROMX_LOAD( "pcf.13m",   0x0000000, 0x400000, CRC(22d72ab9) SHA1(653efd95c34b4b9d2ab0d219f41a99ca84e12214) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pcf.15m",   0x0000002, 0x400000, CRC(16a4813c) SHA1(bf5fce6008214f353414d1b64bea4ed0c7673670) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pcf.17m",   0x0000004, 0x400000, CRC(1097e035) SHA1(4bd51e4e9447af27d2cac1f6d2201e37c949912b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pcf.19m",   0x0000006, 0x400000, CRC(d362d874) SHA1(30c42af18440496cc05e4418e4efa41172ae4ced) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pcf.14m",   0x1000000, 0x100000, CRC(0383897c) SHA1(aba14afa1d0c971afcee4317f480e88117d77b5e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pcf.16m",   0x1000002, 0x100000, CRC(76f91084) SHA1(3d1e32467f2aa5dd6fb96bd5c866ecc9691660fc) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pcf.18m",   0x1000004, 0x100000, CRC(756c3754) SHA1(be2f709b90222a567f198f851cf07ffb0ad433d7) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pcf.20m",   0x1000006, 0x100000, CRC(9ec9277d) SHA1(b7ceeaca30dfcdf498b61a6961f0aa1a068b8ec4) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "pcf.01",   0x00000, 0x08000, CRC(254e5f33) SHA1(c413ec0630b9bdd15e64f42893eba8958a09b573) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "pcf.02",   0x28000, 0x20000, CRC(6902f4f9) SHA1(9bfe4ddade3c666076d26a2b545120f6d059fd7c) )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "pcf.11m",   0x000000, 0x400000, CRC(a5dea005) SHA1(3ae79baf6ff5bd527f82b26f164c7e3c65423ae2) )
	ROM_LOAD16_WORD_SWAP( "pcf.12m",   0x400000, 0x400000, CRC(4ce235fe) SHA1(795b94557e954cc0e45fd3778b609064d57a34a2) )
ROM_END

ROM_START( sgemfa )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "pcfa.03", 0x000000, 0x80000, CRC(e17c089a) SHA1(59529957aeb430df48a88414637e67848fdaaaca) )
	ROM_LOAD16_WORD_SWAP( "pcf.04",  0x080000, 0x80000, CRC(f4314c96) SHA1(c40ed74039bf0096eb3648b7243a8e697638e0a6) )
	ROM_LOAD16_WORD_SWAP( "pcf.05",  0x100000, 0x80000, CRC(215655f6) SHA1(242c0f4401520f2a3b0deafc3a807b18b987e496) )
	ROM_LOAD16_WORD_SWAP( "pcf.06",  0x180000, 0x80000, CRC(ea6f13ea) SHA1(1bc924a8a9da1d2ad7667685cdb92fe317a39aba) )
	ROM_LOAD16_WORD_SWAP( "pcf.07",  0x200000, 0x80000, CRC(5ac6d5ea) SHA1(9ce8e4668b565658597a868830545fb75a5eeaa6) )

	ROM_REGION( 0x1400000, "gfx", 0 )
	ROMX_LOAD( "pcf.13m",   0x0000000, 0x400000, CRC(22d72ab9) SHA1(653efd95c34b4b9d2ab0d219f41a99ca84e12214) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pcf.15m",   0x0000002, 0x400000, CRC(16a4813c) SHA1(bf5fce6008214f353414d1b64bea4ed0c7673670) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pcf.17m",   0x0000004, 0x400000, CRC(1097e035) SHA1(4bd51e4e9447af27d2cac1f6d2201e37c949912b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pcf.19m",   0x0000006, 0x400000, CRC(d362d874) SHA1(30c42af18440496cc05e4418e4efa41172ae4ced) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pcf.14m",   0x1000000, 0x100000, CRC(0383897c) SHA1(aba14afa1d0c971afcee4317f480e88117d77b5e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pcf.16m",   0x1000002, 0x100000, CRC(76f91084) SHA1(3d1e32467f2aa5dd6fb96bd5c866ecc9691660fc) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pcf.18m",   0x1000004, 0x100000, CRC(756c3754) SHA1(be2f709b90222a567f198f851cf07ffb0ad433d7) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pcf.20m",   0x1000006, 0x100000, CRC(9ec9277d) SHA1(b7ceeaca30dfcdf498b61a6961f0aa1a068b8ec4) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "pcf.01",   0x00000, 0x08000, CRC(254e5f33) SHA1(c413ec0630b9bdd15e64f42893eba8958a09b573) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "pcf.02",   0x28000, 0x20000, CRC(6902f4f9) SHA1(9bfe4ddade3c666076d26a2b545120f6d059fd7c) )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "pcf.11m",   0x000000, 0x400000, CRC(a5dea005) SHA1(3ae79baf6ff5bd527f82b26f164c7e3c65423ae2) )
	ROM_LOAD16_WORD_SWAP( "pcf.12m",   0x400000, 0x400000, CRC(4ce235fe) SHA1(795b94557e954cc0e45fd3778b609064d57a34a2) )
ROM_END

ROM_START( sgemfh )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "pcfh.03", 0x000000, 0x80000, CRC(e9103347) SHA1(7a32a151146a15bf5fb5ed993fee2f616077a58c) )
	ROM_LOAD16_WORD_SWAP( "pcf.04",  0x080000, 0x80000, CRC(f4314c96) SHA1(c40ed74039bf0096eb3648b7243a8e697638e0a6) )
	ROM_LOAD16_WORD_SWAP( "pcf.05",  0x100000, 0x80000, CRC(215655f6) SHA1(242c0f4401520f2a3b0deafc3a807b18b987e496) )
	ROM_LOAD16_WORD_SWAP( "pcf.06",  0x180000, 0x80000, CRC(ea6f13ea) SHA1(1bc924a8a9da1d2ad7667685cdb92fe317a39aba) )
	ROM_LOAD16_WORD_SWAP( "pcf.07",  0x200000, 0x80000, CRC(5ac6d5ea) SHA1(9ce8e4668b565658597a868830545fb75a5eeaa6) )

	ROM_REGION( 0x1400000, "gfx", 0 )
	ROMX_LOAD( "pcf.13m",   0x0000000, 0x400000, CRC(22d72ab9) SHA1(653efd95c34b4b9d2ab0d219f41a99ca84e12214) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pcf.15m",   0x0000002, 0x400000, CRC(16a4813c) SHA1(bf5fce6008214f353414d1b64bea4ed0c7673670) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pcf.17m",   0x0000004, 0x400000, CRC(1097e035) SHA1(4bd51e4e9447af27d2cac1f6d2201e37c949912b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pcf.19m",   0x0000006, 0x400000, CRC(d362d874) SHA1(30c42af18440496cc05e4418e4efa41172ae4ced) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pcf.14m",   0x1000000, 0x100000, CRC(0383897c) SHA1(aba14afa1d0c971afcee4317f480e88117d77b5e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pcf.16m",   0x1000002, 0x100000, CRC(76f91084) SHA1(3d1e32467f2aa5dd6fb96bd5c866ecc9691660fc) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pcf.18m",   0x1000004, 0x100000, CRC(756c3754) SHA1(be2f709b90222a567f198f851cf07ffb0ad433d7) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pcf.20m",   0x1000006, 0x100000, CRC(9ec9277d) SHA1(b7ceeaca30dfcdf498b61a6961f0aa1a068b8ec4) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "pcf.01",   0x00000, 0x08000, CRC(254e5f33) SHA1(c413ec0630b9bdd15e64f42893eba8958a09b573) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "pcf.02",   0x28000, 0x20000, CRC(6902f4f9) SHA1(9bfe4ddade3c666076d26a2b545120f6d059fd7c) )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "pcf.11m",   0x000000, 0x400000, CRC(a5dea005) SHA1(3ae79baf6ff5bd527f82b26f164c7e3c65423ae2) )
	ROM_LOAD16_WORD_SWAP( "pcf.12m",   0x400000, 0x400000, CRC(4ce235fe) SHA1(795b94557e954cc0e45fd3778b609064d57a34a2) )
ROM_END

ROM_START( spf2t )
	ROM_REGION(CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "pzfu.03a", 0x000000, 0x80000, CRC(346e62ef) SHA1(9db5ea0aac2d459be957f8b6e2e0d18421587d4d) )
	ROM_LOAD16_WORD_SWAP( "pzf.04",   0x080000, 0x80000, CRC(b80649e2) SHA1(5bfccd656aea7ff82e9a20bb5856f4ab99b5a007) )

	ROM_REGION( 0xC00000, "gfx", 0 )
	ROM_FILL(             0x000000, 0x800000, 0 )
	ROMX_LOAD( "pzf.14m", 0x800000, 0x100000, CRC(2d4881cb) SHA1(fd3baa183c25bed153b19c251980e2fb761600e2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pzf.16m", 0x800002, 0x100000, CRC(4b0fd1be) SHA1(377aafdcdb7a866b1c8487670e3598d8197976e4) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pzf.18m", 0x800004, 0x100000, CRC(e43aac33) SHA1(d041e0688c3807d3363861a7f216de43b34d846c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pzf.20m", 0x800006, 0x100000, CRC(7f536ff1) SHA1(905b9d62ef7bef47297c7f4a4dd697aed6df38a5) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION(QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "pzf.01",   0x00000, 0x08000, CRC(600fb2a3) SHA1(1fab1c2a23bf6ad8309d29ddbbc29435a8aeea13) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "pzf.02",   0x28000, 0x20000, CRC(496076e0) SHA1(1ee4e135140afd0e8e03231e570cd77d140f6367) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "pzf.11m",   0x000000, 0x200000, CRC(78442743) SHA1(b61190bb586871de6d54af580e3e1d9cc0de0acb) )
	ROM_LOAD16_WORD_SWAP( "pzf.12m",   0x200000, 0x200000, CRC(399d2c7b) SHA1(e849dea97b8d16540415c0d9bbc4f9f4eb755ec4) )
ROM_END

ROM_START( spf2xj )
	ROM_REGION(CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "pzfj.03a", 0x000000, 0x80000, CRC(2070554a) SHA1(fa818e6bd2e11667345d3d8f2397b60802ef72f9) )
	ROM_LOAD16_WORD_SWAP( "pzf.04",   0x080000, 0x80000, CRC(b80649e2) SHA1(5bfccd656aea7ff82e9a20bb5856f4ab99b5a007) )

	ROM_REGION( 0xC00000, "gfx", 0 )
	ROM_FILL(             0x000000, 0x800000, 0 )
	ROMX_LOAD( "pzf.14m", 0x800000, 0x100000, CRC(2d4881cb) SHA1(fd3baa183c25bed153b19c251980e2fb761600e2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pzf.16m", 0x800002, 0x100000, CRC(4b0fd1be) SHA1(377aafdcdb7a866b1c8487670e3598d8197976e4) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pzf.18m", 0x800004, 0x100000, CRC(e43aac33) SHA1(d041e0688c3807d3363861a7f216de43b34d846c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pzf.20m", 0x800006, 0x100000, CRC(7f536ff1) SHA1(905b9d62ef7bef47297c7f4a4dd697aed6df38a5) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION(QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "pzf.01",   0x00000, 0x08000, CRC(600fb2a3) SHA1(1fab1c2a23bf6ad8309d29ddbbc29435a8aeea13) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "pzf.02",   0x28000, 0x20000, CRC(496076e0) SHA1(1ee4e135140afd0e8e03231e570cd77d140f6367) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "pzf.11m",   0x000000, 0x200000, CRC(78442743) SHA1(b61190bb586871de6d54af580e3e1d9cc0de0acb) )
	ROM_LOAD16_WORD_SWAP( "pzf.12m",   0x200000, 0x200000, CRC(399d2c7b) SHA1(e849dea97b8d16540415c0d9bbc4f9f4eb755ec4) )
ROM_END

ROM_START( spf2ta )
	ROM_REGION(CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "pzfa.03",  0x000000, 0x80000, CRC(3cecfa78) SHA1(c315531de87f7dc579f744e84ad2dbf068e61f46) )
	ROM_LOAD16_WORD_SWAP( "pzf.04",   0x080000, 0x80000, CRC(b80649e2) SHA1(5bfccd656aea7ff82e9a20bb5856f4ab99b5a007) )

	ROM_REGION( 0xC00000, "gfx", 0 )
	ROM_FILL(             0x000000, 0x800000, 0 )
	ROMX_LOAD( "pzf.14m", 0x800000, 0x100000, CRC(2d4881cb) SHA1(fd3baa183c25bed153b19c251980e2fb761600e2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pzf.16m", 0x800002, 0x100000, CRC(4b0fd1be) SHA1(377aafdcdb7a866b1c8487670e3598d8197976e4) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pzf.18m", 0x800004, 0x100000, CRC(e43aac33) SHA1(d041e0688c3807d3363861a7f216de43b34d846c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pzf.20m", 0x800006, 0x100000, CRC(7f536ff1) SHA1(905b9d62ef7bef47297c7f4a4dd697aed6df38a5) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION(QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "pzf.01",   0x00000, 0x08000, CRC(600fb2a3) SHA1(1fab1c2a23bf6ad8309d29ddbbc29435a8aeea13) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "pzf.02",   0x28000, 0x20000, CRC(496076e0) SHA1(1ee4e135140afd0e8e03231e570cd77d140f6367) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "pzf.11m",   0x000000, 0x200000, CRC(78442743) SHA1(b61190bb586871de6d54af580e3e1d9cc0de0acb) )
	ROM_LOAD16_WORD_SWAP( "pzf.12m",   0x200000, 0x200000, CRC(399d2c7b) SHA1(e849dea97b8d16540415c0d9bbc4f9f4eb755ec4) )
ROM_END

ROM_START( spf2th )
	ROM_REGION(CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "pzfh.03",  0x000000, 0x80000, CRC(20510f2d) SHA1(dcdfc0f6b849499732b76811edb682fea758f530) )
	ROM_LOAD16_WORD_SWAP( "pzf.04",   0x080000, 0x80000, CRC(b80649e2) SHA1(5bfccd656aea7ff82e9a20bb5856f4ab99b5a007) )

	ROM_REGION( 0xC00000, "gfx", 0 )
	ROM_FILL(             0x000000, 0x800000, 0 )
	ROMX_LOAD( "pzf.14m", 0x800000, 0x100000, CRC(2d4881cb) SHA1(fd3baa183c25bed153b19c251980e2fb761600e2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pzf.16m", 0x800002, 0x100000, CRC(4b0fd1be) SHA1(377aafdcdb7a866b1c8487670e3598d8197976e4) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pzf.18m", 0x800004, 0x100000, CRC(e43aac33) SHA1(d041e0688c3807d3363861a7f216de43b34d846c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pzf.20m", 0x800006, 0x100000, CRC(7f536ff1) SHA1(905b9d62ef7bef47297c7f4a4dd697aed6df38a5) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION(QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "pzf.01",   0x00000, 0x08000, CRC(600fb2a3) SHA1(1fab1c2a23bf6ad8309d29ddbbc29435a8aeea13) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "pzf.02",   0x28000, 0x20000, CRC(496076e0) SHA1(1ee4e135140afd0e8e03231e570cd77d140f6367) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "pzf.11m",   0x000000, 0x200000, CRC(78442743) SHA1(b61190bb586871de6d54af580e3e1d9cc0de0acb) )
	ROM_LOAD16_WORD_SWAP( "pzf.12m",   0x200000, 0x200000, CRC(399d2c7b) SHA1(e849dea97b8d16540415c0d9bbc4f9f4eb755ec4) )
ROM_END

ROM_START( ssf2 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "ssfe.03",  0x000000, 0x80000, CRC(a597745d) SHA1(5b12e09c14f0ea93b668b97ca2d27a686c85f641) )
	ROM_LOAD16_WORD_SWAP( "ssfe.04",  0x080000, 0x80000, CRC(b082aa67) SHA1(ca26b4bb1947cb30eaf6b61f606b859d18da4c4c) )
	ROM_LOAD16_WORD_SWAP( "ssfe.05",  0x100000, 0x80000, CRC(02b9c137) SHA1(ba624441e1b4bfb67c71f6a116fe43539eaa4a15) )
	ROM_LOAD16_WORD_SWAP( "ssfe.06",  0x180000, 0x80000, CRC(70d470c5) SHA1(ba03c8f4c76f72f4483e91547e03d1a0cf6db485) )
	ROM_LOAD16_WORD_SWAP( "ssfe.07",  0x200000, 0x80000, CRC(2409001d) SHA1(f532ebb2efbb8f8ba311d10ff897490352c87f97) )

	ROM_REGION( 0xc00000, "gfx", 0 )
	ROMX_LOAD( "ssf.13m",   0x000000, 0x200000, CRC(cf94d275) SHA1(bf2a6d98a656d1cb5734da7836686242d3211137) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.15m",   0x000002, 0x200000, CRC(5eb703af) SHA1(4b302dbb66e8a5c2ad92798699391e981bada427) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.17m",   0x000004, 0x200000, CRC(ffa60e0f) SHA1(b21b1c749a8241440879bf8e7cb33968ccef97e5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.19m",   0x000006, 0x200000, CRC(34e825c5) SHA1(4d320fc96d1ef0b9928a8ce801734245a4c097a5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.14m",   0x800000, 0x100000, CRC(b7cc32e7) SHA1(0f4d26af338dab5dce5b7b34d32ad0c573434ace) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.16m",   0x800002, 0x100000, CRC(8376ad18) SHA1(f4456833fb396e6501f4174c0fe5fd63ea40a188) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.18m",   0x800004, 0x100000, CRC(f5b1b336) SHA1(4b060501e56b9d61294748da5387cdae5280ec4d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.20m",   0x800006, 0x100000, CRC(459d5c6b) SHA1(32b11ba7a12004aff810d719bff7508204c7b7c0) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ssf.01",   0x00000, 0x08000, CRC(eb247e8c) SHA1(24296c18d9b1136d69712bf1c9d9d15463041e83) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD( "ssf.q01",  0x000000, 0x080000, CRC(a6f9da5c) SHA1(6d19f83a01bd25b838d5c2871f7964529d926c98) )
	ROM_LOAD( "ssf.q02",  0x080000, 0x080000, CRC(8c66ae26) SHA1(32a82aee6ed4480e5a990f9af161734c7c0a1403) )
	ROM_LOAD( "ssf.q03",  0x100000, 0x080000, CRC(695cc2ca) SHA1(c2675f0233608b76de528d2a6ef19846d1348060) )
	ROM_LOAD( "ssf.q04",  0x180000, 0x080000, CRC(9d9ebe32) SHA1(9b26329370041374f1a90b479a172d2bc2801c4d) )
	ROM_LOAD( "ssf.q05",  0x200000, 0x080000, CRC(4770e7b7) SHA1(0e764f0befb9227b0b36508d8ca8ec9be31bcb05) )
	ROM_LOAD( "ssf.q06",  0x280000, 0x080000, CRC(4e79c951) SHA1(1144781d7dc57ef8a6458d982f5c91548ff59e27) )
	ROM_LOAD( "ssf.q07",  0x300000, 0x080000, CRC(cdd14313) SHA1(ebe767a9d4b51dba4282fa0a36a546a88620dd59) )
	ROM_LOAD( "ssf.q08",  0x380000, 0x080000, CRC(6f5a088c) SHA1(6c0b4690479647a99d76335f174be8455b4ff118) )
ROM_END

ROM_START( ssf2u )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "ssfu.03a", 0x000000, 0x80000, CRC(72f29c33) SHA1(c24769ca9568d4f6847979929b2a059e57dae6b3) )
	ROM_LOAD16_WORD_SWAP( "ssfu.04a", 0x080000, 0x80000, CRC(935cea44) SHA1(1360254debf179919def1485b5758f529c94f65a) )
	ROM_LOAD16_WORD_SWAP( "ssfu.05",  0x100000, 0x80000, CRC(a0acb28a) SHA1(55c0c0ea9b9e6ef8d7c12f888cf42b6418bbf82e) )
	ROM_LOAD16_WORD_SWAP( "ssfu.06",  0x180000, 0x80000, CRC(47413dcf) SHA1(1a94e38ee899e6356ad22bde4f85e99dd3b6a934) )
	ROM_LOAD16_WORD_SWAP( "ssfu.07",  0x200000, 0x80000, CRC(e6066077) SHA1(889e2cad30b16bfaf0c54f3a38d04dd02deac6f9) )

	ROM_REGION( 0xc00000, "gfx", 0 )
	ROMX_LOAD( "ssf.13m",   0x000000, 0x200000, CRC(cf94d275) SHA1(bf2a6d98a656d1cb5734da7836686242d3211137) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.15m",   0x000002, 0x200000, CRC(5eb703af) SHA1(4b302dbb66e8a5c2ad92798699391e981bada427) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.17m",   0x000004, 0x200000, CRC(ffa60e0f) SHA1(b21b1c749a8241440879bf8e7cb33968ccef97e5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.19m",   0x000006, 0x200000, CRC(34e825c5) SHA1(4d320fc96d1ef0b9928a8ce801734245a4c097a5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.14m",   0x800000, 0x100000, CRC(b7cc32e7) SHA1(0f4d26af338dab5dce5b7b34d32ad0c573434ace) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.16m",   0x800002, 0x100000, CRC(8376ad18) SHA1(f4456833fb396e6501f4174c0fe5fd63ea40a188) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.18m",   0x800004, 0x100000, CRC(f5b1b336) SHA1(4b060501e56b9d61294748da5387cdae5280ec4d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.20m",   0x800006, 0x100000, CRC(459d5c6b) SHA1(32b11ba7a12004aff810d719bff7508204c7b7c0) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ssf.01",   0x00000, 0x08000, CRC(eb247e8c) SHA1(24296c18d9b1136d69712bf1c9d9d15463041e83) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD( "ssf.q01",  0x000000, 0x080000, CRC(a6f9da5c) SHA1(6d19f83a01bd25b838d5c2871f7964529d926c98) )
	ROM_LOAD( "ssf.q02",  0x080000, 0x080000, CRC(8c66ae26) SHA1(32a82aee6ed4480e5a990f9af161734c7c0a1403) )
	ROM_LOAD( "ssf.q03",  0x100000, 0x080000, CRC(695cc2ca) SHA1(c2675f0233608b76de528d2a6ef19846d1348060) )
	ROM_LOAD( "ssf.q04",  0x180000, 0x080000, CRC(9d9ebe32) SHA1(9b26329370041374f1a90b479a172d2bc2801c4d) )
	ROM_LOAD( "ssf.q05",  0x200000, 0x080000, CRC(4770e7b7) SHA1(0e764f0befb9227b0b36508d8ca8ec9be31bcb05) )
	ROM_LOAD( "ssf.q06",  0x280000, 0x080000, CRC(4e79c951) SHA1(1144781d7dc57ef8a6458d982f5c91548ff59e27) )
	ROM_LOAD( "ssf.q07",  0x300000, 0x080000, CRC(cdd14313) SHA1(ebe767a9d4b51dba4282fa0a36a546a88620dd59) )
	ROM_LOAD( "ssf.q08",  0x380000, 0x080000, CRC(6f5a088c) SHA1(6c0b4690479647a99d76335f174be8455b4ff118) )
ROM_END

ROM_START( ssf2a )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "ssfa.03b", 0x000000, 0x80000, CRC(83a059bf) SHA1(3279a792fb884f856cd5bac59eaae7d9e3be286c) )
	ROM_LOAD16_WORD_SWAP( "ssfa.04a", 0x080000, 0x80000, CRC(5d873642) SHA1(74e3541ed586454a8b56e331bc9ffdb8d69f7983) )
	ROM_LOAD16_WORD_SWAP( "ssfa.05",  0x100000, 0x80000, CRC(f8fb4de2) SHA1(e3cde329405d4d59b7c234a30a7c178afb22deef) )
	ROM_LOAD16_WORD_SWAP( "ssfa.06b", 0x180000, 0x80000, CRC(3185d19d) SHA1(9a354b0ee6243a3aaaa0027cce438dcfd9f93a74) )
	ROM_LOAD16_WORD_SWAP( "ssfa.07",  0x200000, 0x80000, CRC(36e29217) SHA1(86563b42676c923c6e3d760e22621e687de3a991) )

	ROM_REGION( 0xc00000, "gfx", 0 )
	ROMX_LOAD( "ssf.13m",   0x000000, 0x200000, CRC(cf94d275) SHA1(bf2a6d98a656d1cb5734da7836686242d3211137) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.15m",   0x000002, 0x200000, CRC(5eb703af) SHA1(4b302dbb66e8a5c2ad92798699391e981bada427) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.17m",   0x000004, 0x200000, CRC(ffa60e0f) SHA1(b21b1c749a8241440879bf8e7cb33968ccef97e5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.19m",   0x000006, 0x200000, CRC(34e825c5) SHA1(4d320fc96d1ef0b9928a8ce801734245a4c097a5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.14m",   0x800000, 0x100000, CRC(b7cc32e7) SHA1(0f4d26af338dab5dce5b7b34d32ad0c573434ace) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.16m",   0x800002, 0x100000, CRC(8376ad18) SHA1(f4456833fb396e6501f4174c0fe5fd63ea40a188) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.18m",   0x800004, 0x100000, CRC(f5b1b336) SHA1(4b060501e56b9d61294748da5387cdae5280ec4d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.20m",   0x800006, 0x100000, CRC(459d5c6b) SHA1(32b11ba7a12004aff810d719bff7508204c7b7c0) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ssf.01",   0x00000, 0x08000, CRC(eb247e8c) SHA1(24296c18d9b1136d69712bf1c9d9d15463041e83) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD( "ssf.q01",  0x000000, 0x080000, CRC(a6f9da5c) SHA1(6d19f83a01bd25b838d5c2871f7964529d926c98) )
	ROM_LOAD( "ssf.q02",  0x080000, 0x080000, CRC(8c66ae26) SHA1(32a82aee6ed4480e5a990f9af161734c7c0a1403) )
	ROM_LOAD( "ssf.q03",  0x100000, 0x080000, CRC(695cc2ca) SHA1(c2675f0233608b76de528d2a6ef19846d1348060) )
	ROM_LOAD( "ssf.q04",  0x180000, 0x080000, CRC(9d9ebe32) SHA1(9b26329370041374f1a90b479a172d2bc2801c4d) )
	ROM_LOAD( "ssf.q05",  0x200000, 0x080000, CRC(4770e7b7) SHA1(0e764f0befb9227b0b36508d8ca8ec9be31bcb05) )
	ROM_LOAD( "ssf.q06",  0x280000, 0x080000, CRC(4e79c951) SHA1(1144781d7dc57ef8a6458d982f5c91548ff59e27) )
	ROM_LOAD( "ssf.q07",  0x300000, 0x080000, CRC(cdd14313) SHA1(ebe767a9d4b51dba4282fa0a36a546a88620dd59) )
	ROM_LOAD( "ssf.q08",  0x380000, 0x080000, CRC(6f5a088c) SHA1(6c0b4690479647a99d76335f174be8455b4ff118) )
ROM_END

ROM_START( ssf2ar1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "ssfa.03a", 0x000000, 0x80000, CRC(d2a3c520) SHA1(514131f0d8a7c6b5bf68630250e4b1b5983d490d) )
	ROM_LOAD16_WORD_SWAP( "ssfa.04a", 0x080000, 0x80000, CRC(5d873642) SHA1(74e3541ed586454a8b56e331bc9ffdb8d69f7983) )
	ROM_LOAD16_WORD_SWAP( "ssfa.05",  0x100000, 0x80000, CRC(f8fb4de2) SHA1(e3cde329405d4d59b7c234a30a7c178afb22deef) )
	ROM_LOAD16_WORD_SWAP( "ssfa.06", 0x180000, 0x80000, CRC(aa8acee7) SHA1(e696b0391e41728f0cc7f190681c5fa7c96a3f81) )
	ROM_LOAD16_WORD_SWAP( "ssfa.07",  0x200000, 0x80000, CRC(36e29217) SHA1(86563b42676c923c6e3d760e22621e687de3a991) )

	ROM_REGION( 0xc00000, "gfx", 0 )
	ROMX_LOAD( "ssf.13m",   0x000000, 0x200000, CRC(cf94d275) SHA1(bf2a6d98a656d1cb5734da7836686242d3211137) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.15m",   0x000002, 0x200000, CRC(5eb703af) SHA1(4b302dbb66e8a5c2ad92798699391e981bada427) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.17m",   0x000004, 0x200000, CRC(ffa60e0f) SHA1(b21b1c749a8241440879bf8e7cb33968ccef97e5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.19m",   0x000006, 0x200000, CRC(34e825c5) SHA1(4d320fc96d1ef0b9928a8ce801734245a4c097a5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.14m",   0x800000, 0x100000, CRC(b7cc32e7) SHA1(0f4d26af338dab5dce5b7b34d32ad0c573434ace) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.16m",   0x800002, 0x100000, CRC(8376ad18) SHA1(f4456833fb396e6501f4174c0fe5fd63ea40a188) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.18m",   0x800004, 0x100000, CRC(f5b1b336) SHA1(4b060501e56b9d61294748da5387cdae5280ec4d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.20m",   0x800006, 0x100000, CRC(459d5c6b) SHA1(32b11ba7a12004aff810d719bff7508204c7b7c0) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ssf.01",   0x00000, 0x08000, CRC(eb247e8c) SHA1(24296c18d9b1136d69712bf1c9d9d15463041e83) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD( "ssf.q01",  0x000000, 0x080000, CRC(a6f9da5c) SHA1(6d19f83a01bd25b838d5c2871f7964529d926c98) )
	ROM_LOAD( "ssf.q02",  0x080000, 0x080000, CRC(8c66ae26) SHA1(32a82aee6ed4480e5a990f9af161734c7c0a1403) )
	ROM_LOAD( "ssf.q03",  0x100000, 0x080000, CRC(695cc2ca) SHA1(c2675f0233608b76de528d2a6ef19846d1348060) )
	ROM_LOAD( "ssf.q04",  0x180000, 0x080000, CRC(9d9ebe32) SHA1(9b26329370041374f1a90b479a172d2bc2801c4d) )
	ROM_LOAD( "ssf.q05",  0x200000, 0x080000, CRC(4770e7b7) SHA1(0e764f0befb9227b0b36508d8ca8ec9be31bcb05) )
	ROM_LOAD( "ssf.q06",  0x280000, 0x080000, CRC(4e79c951) SHA1(1144781d7dc57ef8a6458d982f5c91548ff59e27) )
	ROM_LOAD( "ssf.q07",  0x300000, 0x080000, CRC(cdd14313) SHA1(ebe767a9d4b51dba4282fa0a36a546a88620dd59) )
	ROM_LOAD( "ssf.q08",  0x380000, 0x080000, CRC(6f5a088c) SHA1(6c0b4690479647a99d76335f174be8455b4ff118) )
ROM_END

ROM_START( ssf2j )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "ssfj.03b", 0x000000, 0x80000, CRC(5c2e356d) SHA1(379f1e508778adda4a4087ec52c89b2253265f82) )
	ROM_LOAD16_WORD_SWAP( "ssfj.04a", 0x080000, 0x80000, CRC(013bd55c) SHA1(2482f823a980d45baeea8009dadae7f996bcdb5d) )
	ROM_LOAD16_WORD_SWAP( "ssfj.05",  0x100000, 0x80000, CRC(0918d19a) SHA1(c23be61dd193058eb1391d39fbc22fbcf0640ee0) )
	ROM_LOAD16_WORD_SWAP( "ssfj.06b", 0x180000, 0x80000, CRC(014e0c6d) SHA1(4a5689a05900564c2544c95741cd450ce55da0a7) )
	ROM_LOAD16_WORD_SWAP( "ssfj.07",  0x200000, 0x80000, CRC(eb6a9b1b) SHA1(daedb669b0025f6efb0f3302a40d88dcde2fc76f) )

	ROM_REGION( 0xc00000, "gfx", 0 )
	ROMX_LOAD( "ssf.13m",   0x000000, 0x200000, CRC(cf94d275) SHA1(bf2a6d98a656d1cb5734da7836686242d3211137) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.15m",   0x000002, 0x200000, CRC(5eb703af) SHA1(4b302dbb66e8a5c2ad92798699391e981bada427) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.17m",   0x000004, 0x200000, CRC(ffa60e0f) SHA1(b21b1c749a8241440879bf8e7cb33968ccef97e5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.19m",   0x000006, 0x200000, CRC(34e825c5) SHA1(4d320fc96d1ef0b9928a8ce801734245a4c097a5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.14m",   0x800000, 0x100000, CRC(b7cc32e7) SHA1(0f4d26af338dab5dce5b7b34d32ad0c573434ace) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.16m",   0x800002, 0x100000, CRC(8376ad18) SHA1(f4456833fb396e6501f4174c0fe5fd63ea40a188) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.18m",   0x800004, 0x100000, CRC(f5b1b336) SHA1(4b060501e56b9d61294748da5387cdae5280ec4d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.20m",   0x800006, 0x100000, CRC(459d5c6b) SHA1(32b11ba7a12004aff810d719bff7508204c7b7c0) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ssf.01",   0x00000, 0x08000, CRC(eb247e8c) SHA1(24296c18d9b1136d69712bf1c9d9d15463041e83) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD( "ssf.q01",  0x000000, 0x080000, CRC(a6f9da5c) SHA1(6d19f83a01bd25b838d5c2871f7964529d926c98) )
	ROM_LOAD( "ssf.q02",  0x080000, 0x080000, CRC(8c66ae26) SHA1(32a82aee6ed4480e5a990f9af161734c7c0a1403) )
	ROM_LOAD( "ssf.q03",  0x100000, 0x080000, CRC(695cc2ca) SHA1(c2675f0233608b76de528d2a6ef19846d1348060) )
	ROM_LOAD( "ssf.q04",  0x180000, 0x080000, CRC(9d9ebe32) SHA1(9b26329370041374f1a90b479a172d2bc2801c4d) )
	ROM_LOAD( "ssf.q05",  0x200000, 0x080000, CRC(4770e7b7) SHA1(0e764f0befb9227b0b36508d8ca8ec9be31bcb05) )
	ROM_LOAD( "ssf.q06",  0x280000, 0x080000, CRC(4e79c951) SHA1(1144781d7dc57ef8a6458d982f5c91548ff59e27) )
	ROM_LOAD( "ssf.q07",  0x300000, 0x080000, CRC(cdd14313) SHA1(ebe767a9d4b51dba4282fa0a36a546a88620dd59) )
	ROM_LOAD( "ssf.q08",  0x380000, 0x080000, CRC(6f5a088c) SHA1(6c0b4690479647a99d76335f174be8455b4ff118) )
ROM_END

ROM_START( ssf2jr1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "ssfj.03a", 0x000000, 0x80000, CRC(0bbf1304) SHA1(be93b559ebfcc0fd72cde787c5ea4f50eac52bbf) )
	ROM_LOAD16_WORD_SWAP( "ssfj.04a", 0x080000, 0x80000, CRC(013bd55c) SHA1(2482f823a980d45baeea8009dadae7f996bcdb5d) )
	ROM_LOAD16_WORD_SWAP( "ssfj.05",  0x100000, 0x80000, CRC(0918d19a) SHA1(c23be61dd193058eb1391d39fbc22fbcf0640ee0) )
	ROM_LOAD16_WORD_SWAP( "ssfj.06",  0x180000, 0x80000, CRC(d808a6cd) SHA1(214a4281abacdf6b74b7f51379a93cc64b4c1d7d) )
	ROM_LOAD16_WORD_SWAP( "ssfj.07",  0x200000, 0x80000, CRC(eb6a9b1b) SHA1(daedb669b0025f6efb0f3302a40d88dcde2fc76f) )

	ROM_REGION( 0xc00000, "gfx", 0 )
	ROMX_LOAD( "ssf.13m",   0x000000, 0x200000, CRC(cf94d275) SHA1(bf2a6d98a656d1cb5734da7836686242d3211137) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.15m",   0x000002, 0x200000, CRC(5eb703af) SHA1(4b302dbb66e8a5c2ad92798699391e981bada427) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.17m",   0x000004, 0x200000, CRC(ffa60e0f) SHA1(b21b1c749a8241440879bf8e7cb33968ccef97e5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.19m",   0x000006, 0x200000, CRC(34e825c5) SHA1(4d320fc96d1ef0b9928a8ce801734245a4c097a5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.14m",   0x800000, 0x100000, CRC(b7cc32e7) SHA1(0f4d26af338dab5dce5b7b34d32ad0c573434ace) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.16m",   0x800002, 0x100000, CRC(8376ad18) SHA1(f4456833fb396e6501f4174c0fe5fd63ea40a188) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.18m",   0x800004, 0x100000, CRC(f5b1b336) SHA1(4b060501e56b9d61294748da5387cdae5280ec4d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.20m",   0x800006, 0x100000, CRC(459d5c6b) SHA1(32b11ba7a12004aff810d719bff7508204c7b7c0) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ssf.01",   0x00000, 0x08000, CRC(eb247e8c) SHA1(24296c18d9b1136d69712bf1c9d9d15463041e83) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD( "ssf.q01",  0x000000, 0x080000, CRC(a6f9da5c) SHA1(6d19f83a01bd25b838d5c2871f7964529d926c98) )
	ROM_LOAD( "ssf.q02",  0x080000, 0x080000, CRC(8c66ae26) SHA1(32a82aee6ed4480e5a990f9af161734c7c0a1403) )
	ROM_LOAD( "ssf.q03",  0x100000, 0x080000, CRC(695cc2ca) SHA1(c2675f0233608b76de528d2a6ef19846d1348060) )
	ROM_LOAD( "ssf.q04",  0x180000, 0x080000, CRC(9d9ebe32) SHA1(9b26329370041374f1a90b479a172d2bc2801c4d) )
	ROM_LOAD( "ssf.q05",  0x200000, 0x080000, CRC(4770e7b7) SHA1(0e764f0befb9227b0b36508d8ca8ec9be31bcb05) )
	ROM_LOAD( "ssf.q06",  0x280000, 0x080000, CRC(4e79c951) SHA1(1144781d7dc57ef8a6458d982f5c91548ff59e27) )
	ROM_LOAD( "ssf.q07",  0x300000, 0x080000, CRC(cdd14313) SHA1(ebe767a9d4b51dba4282fa0a36a546a88620dd59) )
	ROM_LOAD( "ssf.q08",  0x380000, 0x080000, CRC(6f5a088c) SHA1(6c0b4690479647a99d76335f174be8455b4ff118) )
ROM_END

ROM_START( ssf2jr2 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "ssfj.03", 0x000000, 0x80000, CRC(7eb0efed) SHA1(c389301cf26cc72ef10c20a7d37223622d05e9ca) )
	ROM_LOAD16_WORD_SWAP( "ssfj.04", 0x080000, 0x80000, CRC(d7322164) SHA1(b83c8523d152384a3eb9f459685b11c6e77cd6d4) )
	ROM_LOAD16_WORD_SWAP( "ssfj.05", 0x100000, 0x80000, CRC(0918d19a) SHA1(c23be61dd193058eb1391d39fbc22fbcf0640ee0) )
	ROM_LOAD16_WORD_SWAP( "ssfj.06", 0x180000, 0x80000, CRC(d808a6cd) SHA1(214a4281abacdf6b74b7f51379a93cc64b4c1d7d) )
	ROM_LOAD16_WORD_SWAP( "ssfj.07", 0x200000, 0x80000, CRC(eb6a9b1b) SHA1(daedb669b0025f6efb0f3302a40d88dcde2fc76f) )

	ROM_REGION( 0xc00000, "gfx", 0 )
	ROMX_LOAD( "ssf.13m",   0x000000, 0x200000, CRC(cf94d275) SHA1(bf2a6d98a656d1cb5734da7836686242d3211137) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.15m",   0x000002, 0x200000, CRC(5eb703af) SHA1(4b302dbb66e8a5c2ad92798699391e981bada427) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.17m",   0x000004, 0x200000, CRC(ffa60e0f) SHA1(b21b1c749a8241440879bf8e7cb33968ccef97e5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.19m",   0x000006, 0x200000, CRC(34e825c5) SHA1(4d320fc96d1ef0b9928a8ce801734245a4c097a5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.14m",   0x800000, 0x100000, CRC(b7cc32e7) SHA1(0f4d26af338dab5dce5b7b34d32ad0c573434ace) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.16m",   0x800002, 0x100000, CRC(8376ad18) SHA1(f4456833fb396e6501f4174c0fe5fd63ea40a188) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.18m",   0x800004, 0x100000, CRC(f5b1b336) SHA1(4b060501e56b9d61294748da5387cdae5280ec4d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.20m",   0x800006, 0x100000, CRC(459d5c6b) SHA1(32b11ba7a12004aff810d719bff7508204c7b7c0) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ssf.01",   0x00000, 0x08000, CRC(eb247e8c) SHA1(24296c18d9b1136d69712bf1c9d9d15463041e83) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD( "ssf.q01",  0x000000, 0x080000, CRC(a6f9da5c) SHA1(6d19f83a01bd25b838d5c2871f7964529d926c98) )
	ROM_LOAD( "ssf.q02",  0x080000, 0x080000, CRC(8c66ae26) SHA1(32a82aee6ed4480e5a990f9af161734c7c0a1403) )
	ROM_LOAD( "ssf.q03",  0x100000, 0x080000, CRC(695cc2ca) SHA1(c2675f0233608b76de528d2a6ef19846d1348060) )
	ROM_LOAD( "ssf.q04",  0x180000, 0x080000, CRC(9d9ebe32) SHA1(9b26329370041374f1a90b479a172d2bc2801c4d) )
	ROM_LOAD( "ssf.q05",  0x200000, 0x080000, CRC(4770e7b7) SHA1(0e764f0befb9227b0b36508d8ca8ec9be31bcb05) )
	ROM_LOAD( "ssf.q06",  0x280000, 0x080000, CRC(4e79c951) SHA1(1144781d7dc57ef8a6458d982f5c91548ff59e27) )
	ROM_LOAD( "ssf.q07",  0x300000, 0x080000, CRC(cdd14313) SHA1(ebe767a9d4b51dba4282fa0a36a546a88620dd59) )
	ROM_LOAD( "ssf.q08",  0x380000, 0x080000, CRC(6f5a088c) SHA1(6c0b4690479647a99d76335f174be8455b4ff118) )
ROM_END

ROM_START( ssf2h )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "ssfh.03",  0x000000, 0x80000, CRC(b086b355) SHA1(c0ee70fa44081ebf09a72910ce1b733b019aaf71) )
	ROM_LOAD16_WORD_SWAP( "ssfh.04",  0x080000, 0x80000, CRC(1e629b29) SHA1(1c1043610cae9ea6cb3139a1c14d53a6fcd91d02) )
	ROM_LOAD16_WORD_SWAP( "ssfh.05",  0x100000, 0x80000, CRC(b5997e10) SHA1(fe9502a5dfc1dfba80d0246056eae5f3b47897a7) )
	ROM_LOAD16_WORD_SWAP( "ssfh.06",  0x180000, 0x80000, CRC(793b8fad) SHA1(e0e30dbd7a95636e592fc58577179f12b7b4ea76) )
	ROM_LOAD16_WORD_SWAP( "ssfh.07",  0x200000, 0x80000, CRC(cbb92ac3) SHA1(f5dd189757b11ea39ff8ead8d7e9bd3b97934a94) )

	ROM_REGION( 0xc00000, "gfx", 0 )
	ROMX_LOAD( "ssf.13m",   0x000000, 0x200000, CRC(cf94d275) SHA1(bf2a6d98a656d1cb5734da7836686242d3211137) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.15m",   0x000002, 0x200000, CRC(5eb703af) SHA1(4b302dbb66e8a5c2ad92798699391e981bada427) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.17m",   0x000004, 0x200000, CRC(ffa60e0f) SHA1(b21b1c749a8241440879bf8e7cb33968ccef97e5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.19m",   0x000006, 0x200000, CRC(34e825c5) SHA1(4d320fc96d1ef0b9928a8ce801734245a4c097a5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.14m",   0x800000, 0x100000, CRC(b7cc32e7) SHA1(0f4d26af338dab5dce5b7b34d32ad0c573434ace) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.16m",   0x800002, 0x100000, CRC(8376ad18) SHA1(f4456833fb396e6501f4174c0fe5fd63ea40a188) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.18m",   0x800004, 0x100000, CRC(f5b1b336) SHA1(4b060501e56b9d61294748da5387cdae5280ec4d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.20m",   0x800006, 0x100000, CRC(459d5c6b) SHA1(32b11ba7a12004aff810d719bff7508204c7b7c0) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ssf.01",   0x00000, 0x08000, CRC(eb247e8c) SHA1(24296c18d9b1136d69712bf1c9d9d15463041e83) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD( "ssf.q01",  0x000000, 0x080000, CRC(a6f9da5c) SHA1(6d19f83a01bd25b838d5c2871f7964529d926c98) )
	ROM_LOAD( "ssf.q02",  0x080000, 0x080000, CRC(8c66ae26) SHA1(32a82aee6ed4480e5a990f9af161734c7c0a1403) )
	ROM_LOAD( "ssf.q03",  0x100000, 0x080000, CRC(695cc2ca) SHA1(c2675f0233608b76de528d2a6ef19846d1348060) )
	ROM_LOAD( "ssf.q04",  0x180000, 0x080000, CRC(9d9ebe32) SHA1(9b26329370041374f1a90b479a172d2bc2801c4d) )
	ROM_LOAD( "ssf.q05",  0x200000, 0x080000, CRC(4770e7b7) SHA1(0e764f0befb9227b0b36508d8ca8ec9be31bcb05) )
	ROM_LOAD( "ssf.q06",  0x280000, 0x080000, CRC(4e79c951) SHA1(1144781d7dc57ef8a6458d982f5c91548ff59e27) )
	ROM_LOAD( "ssf.q07",  0x300000, 0x080000, CRC(cdd14313) SHA1(ebe767a9d4b51dba4282fa0a36a546a88620dd59) )
	ROM_LOAD( "ssf.q08",  0x380000, 0x080000, CRC(6f5a088c) SHA1(6c0b4690479647a99d76335f174be8455b4ff118) )
ROM_END

ROM_START( ssf2tb )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "ssfe.3tc", 0x000000, 0x80000, CRC(496a8409) SHA1(3101689e86ab78c544524e31057478fce336ddaa) )
	ROM_LOAD16_WORD_SWAP( "ssfe.4tc", 0x080000, 0x80000, CRC(4b45c18b) SHA1(9c7ecb6fee70e317d1005bcadadf59cf11f58050) )
	ROM_LOAD16_WORD_SWAP( "ssfe.5t",  0x100000, 0x80000, CRC(6a9c6444) SHA1(76ba626136268a48b139f6aacd6eeded94d1354d) )
	ROM_LOAD16_WORD_SWAP( "ssfe.6tb", 0x180000, 0x80000, CRC(e4944fc3) SHA1(2d77bc19140c8895eca445b6a290bc793946ccfb) )
	ROM_LOAD16_WORD_SWAP( "ssfe.7t",  0x200000, 0x80000, CRC(2c9f4782) SHA1(de046e6bd9823129fb3d1bfff3710689816a6b0a) )

	ROM_REGION( 0xc00000, "gfx", 0 )
	ROMX_LOAD( "ssf.13m",   0x000000, 0x200000, CRC(cf94d275) SHA1(bf2a6d98a656d1cb5734da7836686242d3211137) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.15m",   0x000002, 0x200000, CRC(5eb703af) SHA1(4b302dbb66e8a5c2ad92798699391e981bada427) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.17m",   0x000004, 0x200000, CRC(ffa60e0f) SHA1(b21b1c749a8241440879bf8e7cb33968ccef97e5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.19m",   0x000006, 0x200000, CRC(34e825c5) SHA1(4d320fc96d1ef0b9928a8ce801734245a4c097a5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.14m",   0x800000, 0x100000, CRC(b7cc32e7) SHA1(0f4d26af338dab5dce5b7b34d32ad0c573434ace) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.16m",   0x800002, 0x100000, CRC(8376ad18) SHA1(f4456833fb396e6501f4174c0fe5fd63ea40a188) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.18m",   0x800004, 0x100000, CRC(f5b1b336) SHA1(4b060501e56b9d61294748da5387cdae5280ec4d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.20m",   0x800006, 0x100000, CRC(459d5c6b) SHA1(32b11ba7a12004aff810d719bff7508204c7b7c0) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ssf.01",   0x00000, 0x08000, CRC(eb247e8c) SHA1(24296c18d9b1136d69712bf1c9d9d15463041e83) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD( "ssf.q01",  0x000000, 0x080000, CRC(a6f9da5c) SHA1(6d19f83a01bd25b838d5c2871f7964529d926c98) )
	ROM_LOAD( "ssf.q02",  0x080000, 0x080000, CRC(8c66ae26) SHA1(32a82aee6ed4480e5a990f9af161734c7c0a1403) )
	ROM_LOAD( "ssf.q03",  0x100000, 0x080000, CRC(695cc2ca) SHA1(c2675f0233608b76de528d2a6ef19846d1348060) )
	ROM_LOAD( "ssf.q04",  0x180000, 0x080000, CRC(9d9ebe32) SHA1(9b26329370041374f1a90b479a172d2bc2801c4d) )
	ROM_LOAD( "ssf.q05",  0x200000, 0x080000, CRC(4770e7b7) SHA1(0e764f0befb9227b0b36508d8ca8ec9be31bcb05) )
	ROM_LOAD( "ssf.q06",  0x280000, 0x080000, CRC(4e79c951) SHA1(1144781d7dc57ef8a6458d982f5c91548ff59e27) )
	ROM_LOAD( "ssf.q07",  0x300000, 0x080000, CRC(cdd14313) SHA1(ebe767a9d4b51dba4282fa0a36a546a88620dd59) )
	ROM_LOAD( "ssf.q08",  0x380000, 0x080000, CRC(6f5a088c) SHA1(6c0b4690479647a99d76335f174be8455b4ff118) )
ROM_END

ROM_START( ssf2tbr1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "ssfe.3t", 0x000000, 0x80000, CRC(1e018e34) SHA1(b9ca6655f0fc67a4f047df7ec944c9b4b85ab8ef) )
	ROM_LOAD16_WORD_SWAP( "ssfe.4t", 0x080000, 0x80000, CRC(ac92efaf) SHA1(340b2b6a60bf3195c0d42e214dcec49f8cc35f35) )
	ROM_LOAD16_WORD_SWAP( "ssfe.5t", 0x100000, 0x80000, CRC(6a9c6444) SHA1(76ba626136268a48b139f6aacd6eeded94d1354d) )
	ROM_LOAD16_WORD_SWAP( "ssfe.6t", 0x180000, 0x80000, CRC(f442562b) SHA1(bced425b291c1b90f988e59d3639701874271cb2) )
	ROM_LOAD16_WORD_SWAP( "ssfe.7t", 0x200000, 0x80000, CRC(2c9f4782) SHA1(de046e6bd9823129fb3d1bfff3710689816a6b0a) )

	ROM_REGION( 0xc00000, "gfx", 0 )
	ROMX_LOAD( "ssf.13m",   0x000000, 0x200000, CRC(cf94d275) SHA1(bf2a6d98a656d1cb5734da7836686242d3211137) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.15m",   0x000002, 0x200000, CRC(5eb703af) SHA1(4b302dbb66e8a5c2ad92798699391e981bada427) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.17m",   0x000004, 0x200000, CRC(ffa60e0f) SHA1(b21b1c749a8241440879bf8e7cb33968ccef97e5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.19m",   0x000006, 0x200000, CRC(34e825c5) SHA1(4d320fc96d1ef0b9928a8ce801734245a4c097a5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.14m",   0x800000, 0x100000, CRC(b7cc32e7) SHA1(0f4d26af338dab5dce5b7b34d32ad0c573434ace) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.16m",   0x800002, 0x100000, CRC(8376ad18) SHA1(f4456833fb396e6501f4174c0fe5fd63ea40a188) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.18m",   0x800004, 0x100000, CRC(f5b1b336) SHA1(4b060501e56b9d61294748da5387cdae5280ec4d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.20m",   0x800006, 0x100000, CRC(459d5c6b) SHA1(32b11ba7a12004aff810d719bff7508204c7b7c0) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ssf.01",   0x00000, 0x08000, CRC(eb247e8c) SHA1(24296c18d9b1136d69712bf1c9d9d15463041e83) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD( "ssf.q01",  0x000000, 0x080000, CRC(a6f9da5c) SHA1(6d19f83a01bd25b838d5c2871f7964529d926c98) )
	ROM_LOAD( "ssf.q02",  0x080000, 0x080000, CRC(8c66ae26) SHA1(32a82aee6ed4480e5a990f9af161734c7c0a1403) )
	ROM_LOAD( "ssf.q03",  0x100000, 0x080000, CRC(695cc2ca) SHA1(c2675f0233608b76de528d2a6ef19846d1348060) )
	ROM_LOAD( "ssf.q04",  0x180000, 0x080000, CRC(9d9ebe32) SHA1(9b26329370041374f1a90b479a172d2bc2801c4d) )
	ROM_LOAD( "ssf.q05",  0x200000, 0x080000, CRC(4770e7b7) SHA1(0e764f0befb9227b0b36508d8ca8ec9be31bcb05) )
	ROM_LOAD( "ssf.q06",  0x280000, 0x080000, CRC(4e79c951) SHA1(1144781d7dc57ef8a6458d982f5c91548ff59e27) )
	ROM_LOAD( "ssf.q07",  0x300000, 0x080000, CRC(cdd14313) SHA1(ebe767a9d4b51dba4282fa0a36a546a88620dd59) )
	ROM_LOAD( "ssf.q08",  0x380000, 0x080000, CRC(6f5a088c) SHA1(6c0b4690479647a99d76335f174be8455b4ff118) )
ROM_END

ROM_START( ssf2tbj )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "ssfj.3t", 0x000000, 0x80000, CRC(980d4450) SHA1(1a7a7000dc11473d06e2bb552c7a506eb0019235) )
	ROM_LOAD16_WORD_SWAP( "ssfj.4t", 0x080000, 0x80000, CRC(b4dc1906) SHA1(b29497c8562d004c6f0393eb61ba80978f4b3aff) )
	ROM_LOAD16_WORD_SWAP( "ssfj.5t", 0x100000, 0x80000, CRC(a7e35fbc) SHA1(c59737f4dbd9ccde30b0a1e2f151a78f162ceafc) )
	ROM_LOAD16_WORD_SWAP( "ssfj.6t", 0x180000, 0x80000, CRC(cb592b30) SHA1(d9464c99f813ee50041adfc077ebe998c6e9a5f7) )
	ROM_LOAD16_WORD_SWAP( "ssfj.7t", 0x200000, 0x80000, CRC(1f239515) SHA1(e5e314e7fe8d1448cc452e515415adf8aa62056d) )

	ROM_REGION( 0xc00000, "gfx", 0 )
	ROMX_LOAD( "ssf.13m",   0x000000, 0x200000, CRC(cf94d275) SHA1(bf2a6d98a656d1cb5734da7836686242d3211137) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.15m",   0x000002, 0x200000, CRC(5eb703af) SHA1(4b302dbb66e8a5c2ad92798699391e981bada427) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.17m",   0x000004, 0x200000, CRC(ffa60e0f) SHA1(b21b1c749a8241440879bf8e7cb33968ccef97e5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.19m",   0x000006, 0x200000, CRC(34e825c5) SHA1(4d320fc96d1ef0b9928a8ce801734245a4c097a5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.14m",   0x800000, 0x100000, CRC(b7cc32e7) SHA1(0f4d26af338dab5dce5b7b34d32ad0c573434ace) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.16m",   0x800002, 0x100000, CRC(8376ad18) SHA1(f4456833fb396e6501f4174c0fe5fd63ea40a188) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.18m",   0x800004, 0x100000, CRC(f5b1b336) SHA1(4b060501e56b9d61294748da5387cdae5280ec4d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.20m",   0x800006, 0x100000, CRC(459d5c6b) SHA1(32b11ba7a12004aff810d719bff7508204c7b7c0) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ssf.01",   0x00000, 0x08000, CRC(eb247e8c) SHA1(24296c18d9b1136d69712bf1c9d9d15463041e83) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD( "ssf.q01",  0x000000, 0x080000, CRC(a6f9da5c) SHA1(6d19f83a01bd25b838d5c2871f7964529d926c98) )
	ROM_LOAD( "ssf.q02",  0x080000, 0x080000, CRC(8c66ae26) SHA1(32a82aee6ed4480e5a990f9af161734c7c0a1403) )
	ROM_LOAD( "ssf.q03",  0x100000, 0x080000, CRC(695cc2ca) SHA1(c2675f0233608b76de528d2a6ef19846d1348060) )
	ROM_LOAD( "ssf.q04",  0x180000, 0x080000, CRC(9d9ebe32) SHA1(9b26329370041374f1a90b479a172d2bc2801c4d) )
	ROM_LOAD( "ssf.q05",  0x200000, 0x080000, CRC(4770e7b7) SHA1(0e764f0befb9227b0b36508d8ca8ec9be31bcb05) )
	ROM_LOAD( "ssf.q06",  0x280000, 0x080000, CRC(4e79c951) SHA1(1144781d7dc57ef8a6458d982f5c91548ff59e27) )
	ROM_LOAD( "ssf.q07",  0x300000, 0x080000, CRC(cdd14313) SHA1(ebe767a9d4b51dba4282fa0a36a546a88620dd59) )
	ROM_LOAD( "ssf.q08",  0x380000, 0x080000, CRC(6f5a088c) SHA1(6c0b4690479647a99d76335f174be8455b4ff118) )
ROM_END

ROM_START( ssf2t )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "sfxe.03c", 0x000000, 0x80000, CRC(2fa1f396) SHA1(2aa58309811f34901554b84396556630a22ce9bc) )
	ROM_LOAD16_WORD_SWAP( "sfxe.04a", 0x080000, 0x80000, CRC(d0bc29c6) SHA1(d9f89bcd79cba26db2100a00dd7bd8ee6ecb75f3) )
	ROM_LOAD16_WORD_SWAP( "sfxe.05",  0x100000, 0x80000, CRC(65222964) SHA1(025bb708dc5b6365cc7fe60fc3f242511ad8f384) )
	ROM_LOAD16_WORD_SWAP( "sfxe.06a", 0x180000, 0x80000, CRC(8fe9f531) SHA1(b5d9ed498f730fdb968992bdec33605db1a007f4) )
	ROM_LOAD16_WORD_SWAP( "sfxe.07",  0x200000, 0x80000, CRC(8a7d0cb6) SHA1(27ea0cea73a93c27257bf2a170d1deaf938cc311) )
	ROM_LOAD16_WORD_SWAP( "sfxe.08",  0x280000, 0x80000, CRC(74c24062) SHA1(f3eca09e0544c6aa46b0c4bead2246ab1e9a97d9) )
	ROM_LOAD16_WORD_SWAP( "sfx.09",   0x300000, 0x80000, CRC(642fae3f) SHA1(746df99b826b9837bba267104132161153c1daff) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROMX_LOAD( "sfx.13m",   0x000000, 0x200000, CRC(cf94d275) SHA1(bf2a6d98a656d1cb5734da7836686242d3211137) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.15m",   0x000002, 0x200000, CRC(5eb703af) SHA1(4b302dbb66e8a5c2ad92798699391e981bada427) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.17m",   0x000004, 0x200000, CRC(ffa60e0f) SHA1(b21b1c749a8241440879bf8e7cb33968ccef97e5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.19m",   0x000006, 0x200000, CRC(34e825c5) SHA1(4d320fc96d1ef0b9928a8ce801734245a4c097a5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.14m",   0x800000, 0x100000, CRC(b7cc32e7) SHA1(0f4d26af338dab5dce5b7b34d32ad0c573434ace) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.16m",   0x800002, 0x100000, CRC(8376ad18) SHA1(f4456833fb396e6501f4174c0fe5fd63ea40a188) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.18m",   0x800004, 0x100000, CRC(f5b1b336) SHA1(4b060501e56b9d61294748da5387cdae5280ec4d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.20m",   0x800006, 0x100000, CRC(459d5c6b) SHA1(32b11ba7a12004aff810d719bff7508204c7b7c0) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.21m",   0xc00000, 0x100000, CRC(e32854af) SHA1(1a5e11e9caa2b96108d89ae660ef1f6bcb469a74) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.23m",   0xc00002, 0x100000, CRC(760f2927) SHA1(491e28e14ee06821fc9e709efa7b91313bc0c2db) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.25m",   0xc00004, 0x100000, CRC(1ee90208) SHA1(83df1d9953560edddc2951ea426d29fb014e6a8a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.27m",   0xc00006, 0x100000, CRC(f814400f) SHA1(ad6921af36d0bd5dfb89b1fb53c3ca3fd92d7204) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sfx.01",   0x00000, 0x08000, CRC(b47b8835) SHA1(c8b2d50fe3a329bd0592ea160d505155d873dab1) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "sfx.02",   0x28000, 0x20000, CRC(0022633f) SHA1(cab3afc79da53e3887eb1ccd1f4d19790728e6cd) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "sfx.11m",   0x000000, 0x200000, CRC(9bdbd476) SHA1(a8520f77f30b97aae36408e0c4ca1ebbde1808a5) )
	ROM_LOAD16_WORD_SWAP( "sfx.12m",   0x200000, 0x200000, CRC(a05e3aab) SHA1(d4eb9cae66c74e956569fea8b815156fbd420f83) )
ROM_END

ROM_START( ssf2tu )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "sfxu.03e", 0x000000, 0x80000, CRC(d6ff689e) SHA1(bea1a8aafbbbe9cb0895561a57dead1579361a8e) )
	ROM_LOAD16_WORD_SWAP( "sfxu.04a", 0x080000, 0x80000, CRC(532b5ffd) SHA1(769a8a9d4e04e291ce7427b89e537bba2258ca82) )
	ROM_LOAD16_WORD_SWAP( "sfxu.05",  0x100000, 0x80000, CRC(ffa3c6de) SHA1(7cce55a3e07b5ba2e2e37e4c66a52678a1b19a63) )
	ROM_LOAD16_WORD_SWAP( "sfxu.06b", 0x180000, 0x80000, CRC(83f9382b) SHA1(273ff4d4242ce22b755d35e5d2cf2517d625bdd2) )
	ROM_LOAD16_WORD_SWAP( "sfxu.07a", 0x200000, 0x80000, CRC(6ab673e8) SHA1(840af0d0ce634fb98e4f89173c4f1f95ec2cf94b) )
	ROM_LOAD16_WORD_SWAP( "sfxu.08",  0x280000, 0x80000, CRC(b3c71810) SHA1(b51515f4f4aee5bbbfc8b79372d0bc6e0c140912) )
	ROM_LOAD16_WORD_SWAP( "sfx.09",   0x300000, 0x80000, CRC(642fae3f) SHA1(746df99b826b9837bba267104132161153c1daff) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROMX_LOAD( "sfx.13m",   0x000000, 0x200000, CRC(cf94d275) SHA1(bf2a6d98a656d1cb5734da7836686242d3211137) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.15m",   0x000002, 0x200000, CRC(5eb703af) SHA1(4b302dbb66e8a5c2ad92798699391e981bada427) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.17m",   0x000004, 0x200000, CRC(ffa60e0f) SHA1(b21b1c749a8241440879bf8e7cb33968ccef97e5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.19m",   0x000006, 0x200000, CRC(34e825c5) SHA1(4d320fc96d1ef0b9928a8ce801734245a4c097a5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.14m",   0x800000, 0x100000, CRC(b7cc32e7) SHA1(0f4d26af338dab5dce5b7b34d32ad0c573434ace) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.16m",   0x800002, 0x100000, CRC(8376ad18) SHA1(f4456833fb396e6501f4174c0fe5fd63ea40a188) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.18m",   0x800004, 0x100000, CRC(f5b1b336) SHA1(4b060501e56b9d61294748da5387cdae5280ec4d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.20m",   0x800006, 0x100000, CRC(459d5c6b) SHA1(32b11ba7a12004aff810d719bff7508204c7b7c0) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.21m",   0xc00000, 0x100000, CRC(e32854af) SHA1(1a5e11e9caa2b96108d89ae660ef1f6bcb469a74) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.23m",   0xc00002, 0x100000, CRC(760f2927) SHA1(491e28e14ee06821fc9e709efa7b91313bc0c2db) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.25m",   0xc00004, 0x100000, CRC(1ee90208) SHA1(83df1d9953560edddc2951ea426d29fb014e6a8a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.27m",   0xc00006, 0x100000, CRC(f814400f) SHA1(ad6921af36d0bd5dfb89b1fb53c3ca3fd92d7204) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sfx.01",   0x00000, 0x08000, CRC(b47b8835) SHA1(c8b2d50fe3a329bd0592ea160d505155d873dab1) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "sfx.02",   0x28000, 0x20000, CRC(0022633f) SHA1(cab3afc79da53e3887eb1ccd1f4d19790728e6cd) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "sfx.11m",   0x000000, 0x200000, CRC(9bdbd476) SHA1(a8520f77f30b97aae36408e0c4ca1ebbde1808a5) )
	ROM_LOAD16_WORD_SWAP( "sfx.12m",   0x200000, 0x200000, CRC(a05e3aab) SHA1(d4eb9cae66c74e956569fea8b815156fbd420f83) )
ROM_END

ROM_START( ssf2tur1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "sfxu.03c", 0x000000, 0x80000, CRC(86e4a335) SHA1(04db3fd519973aeb7b32be62871e0fb4605946eb) )
	ROM_LOAD16_WORD_SWAP( "sfxu.04a", 0x080000, 0x80000, CRC(532b5ffd) SHA1(769a8a9d4e04e291ce7427b89e537bba2258ca82) )
	ROM_LOAD16_WORD_SWAP( "sfxu.05",  0x100000, 0x80000, CRC(ffa3c6de) SHA1(7cce55a3e07b5ba2e2e37e4c66a52678a1b19a63) )
	ROM_LOAD16_WORD_SWAP( "sfxu.06a", 0x180000, 0x80000, CRC(e4c04c99) SHA1(01fe284363e4795e7bdf4206f54a6108fcdac18b) )
	ROM_LOAD16_WORD_SWAP( "sfxu.07",  0x200000, 0x80000, CRC(d8199e41) SHA1(aa5647446f7e076cdf895dd5cbc5b30a8d4fdac2) )
	ROM_LOAD16_WORD_SWAP( "sfxu.08",  0x280000, 0x80000, CRC(b3c71810) SHA1(b51515f4f4aee5bbbfc8b79372d0bc6e0c140912) )
	ROM_LOAD16_WORD_SWAP( "sfx.09",   0x300000, 0x80000, CRC(642fae3f) SHA1(746df99b826b9837bba267104132161153c1daff) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROMX_LOAD( "sfx.13m",   0x000000, 0x200000, CRC(cf94d275) SHA1(bf2a6d98a656d1cb5734da7836686242d3211137) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.15m",   0x000002, 0x200000, CRC(5eb703af) SHA1(4b302dbb66e8a5c2ad92798699391e981bada427) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.17m",   0x000004, 0x200000, CRC(ffa60e0f) SHA1(b21b1c749a8241440879bf8e7cb33968ccef97e5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.19m",   0x000006, 0x200000, CRC(34e825c5) SHA1(4d320fc96d1ef0b9928a8ce801734245a4c097a5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.14m",   0x800000, 0x100000, CRC(b7cc32e7) SHA1(0f4d26af338dab5dce5b7b34d32ad0c573434ace) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.16m",   0x800002, 0x100000, CRC(8376ad18) SHA1(f4456833fb396e6501f4174c0fe5fd63ea40a188) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.18m",   0x800004, 0x100000, CRC(f5b1b336) SHA1(4b060501e56b9d61294748da5387cdae5280ec4d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.20m",   0x800006, 0x100000, CRC(459d5c6b) SHA1(32b11ba7a12004aff810d719bff7508204c7b7c0) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.21m",   0xc00000, 0x100000, CRC(e32854af) SHA1(1a5e11e9caa2b96108d89ae660ef1f6bcb469a74) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.23m",   0xc00002, 0x100000, CRC(760f2927) SHA1(491e28e14ee06821fc9e709efa7b91313bc0c2db) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.25m",   0xc00004, 0x100000, CRC(1ee90208) SHA1(83df1d9953560edddc2951ea426d29fb014e6a8a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.27m",   0xc00006, 0x100000, CRC(f814400f) SHA1(ad6921af36d0bd5dfb89b1fb53c3ca3fd92d7204) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sfx.01",   0x00000, 0x08000, CRC(b47b8835) SHA1(c8b2d50fe3a329bd0592ea160d505155d873dab1) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "sfx.02",   0x28000, 0x20000, CRC(0022633f) SHA1(cab3afc79da53e3887eb1ccd1f4d19790728e6cd) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "sfx.11m",   0x000000, 0x200000, CRC(9bdbd476) SHA1(a8520f77f30b97aae36408e0c4ca1ebbde1808a5) )
	ROM_LOAD16_WORD_SWAP( "sfx.12m",   0x200000, 0x200000, CRC(a05e3aab) SHA1(d4eb9cae66c74e956569fea8b815156fbd420f83) )
ROM_END

ROM_START( ssf2ta )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "sfxa.03c", 0x000000, 0x80000, CRC(04b9ff34) SHA1(69feb2c9c03634e6f964dae310d7b72b4c76140d) )
	ROM_LOAD16_WORD_SWAP( "sfxa.04a", 0x080000, 0x80000, CRC(16ea5f7a) SHA1(08404c6a79b9a36eceb06e0d3e1d747a21fac186) )
	ROM_LOAD16_WORD_SWAP( "sfxa.05",  0x100000, 0x80000, CRC(53d61f0c) SHA1(b30e666d0dae7b738a76a27d1d68fbb9a630c27c) )
	ROM_LOAD16_WORD_SWAP( "sfxa.06a", 0x180000, 0x80000, CRC(066d09b5) SHA1(221972629b094809f7c431f86b3f3b10354487b5) )
	ROM_LOAD16_WORD_SWAP( "sfxa.07",  0x200000, 0x80000, CRC(a428257b) SHA1(620f3a264b2c82ef1af0e33310d110e1f3e6fddf) )
	ROM_LOAD16_WORD_SWAP( "sfxa.08",  0x280000, 0x80000, CRC(39be596c) SHA1(f7ab80e64cbb703535dd39b875273eefa57df489) )
	ROM_LOAD16_WORD_SWAP( "sfx.09",   0x300000, 0x80000, CRC(642fae3f) SHA1(746df99b826b9837bba267104132161153c1daff) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROMX_LOAD( "sfx.13m",   0x000000, 0x200000, CRC(cf94d275) SHA1(bf2a6d98a656d1cb5734da7836686242d3211137) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.15m",   0x000002, 0x200000, CRC(5eb703af) SHA1(4b302dbb66e8a5c2ad92798699391e981bada427) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.17m",   0x000004, 0x200000, CRC(ffa60e0f) SHA1(b21b1c749a8241440879bf8e7cb33968ccef97e5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.19m",   0x000006, 0x200000, CRC(34e825c5) SHA1(4d320fc96d1ef0b9928a8ce801734245a4c097a5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.14m",   0x800000, 0x100000, CRC(b7cc32e7) SHA1(0f4d26af338dab5dce5b7b34d32ad0c573434ace) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.16m",   0x800002, 0x100000, CRC(8376ad18) SHA1(f4456833fb396e6501f4174c0fe5fd63ea40a188) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.18m",   0x800004, 0x100000, CRC(f5b1b336) SHA1(4b060501e56b9d61294748da5387cdae5280ec4d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.20m",   0x800006, 0x100000, CRC(459d5c6b) SHA1(32b11ba7a12004aff810d719bff7508204c7b7c0) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.21m",   0xc00000, 0x100000, CRC(e32854af) SHA1(1a5e11e9caa2b96108d89ae660ef1f6bcb469a74) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.23m",   0xc00002, 0x100000, CRC(760f2927) SHA1(491e28e14ee06821fc9e709efa7b91313bc0c2db) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.25m",   0xc00004, 0x100000, CRC(1ee90208) SHA1(83df1d9953560edddc2951ea426d29fb014e6a8a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.27m",   0xc00006, 0x100000, CRC(f814400f) SHA1(ad6921af36d0bd5dfb89b1fb53c3ca3fd92d7204) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sfx.01",   0x00000, 0x08000, CRC(b47b8835) SHA1(c8b2d50fe3a329bd0592ea160d505155d873dab1) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "sfx.02",   0x28000, 0x20000, CRC(0022633f) SHA1(cab3afc79da53e3887eb1ccd1f4d19790728e6cd) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "sfx.11m",   0x000000, 0x200000, CRC(9bdbd476) SHA1(a8520f77f30b97aae36408e0c4ca1ebbde1808a5) )
	ROM_LOAD16_WORD_SWAP( "sfx.12m",   0x200000, 0x200000, CRC(a05e3aab) SHA1(d4eb9cae66c74e956569fea8b815156fbd420f83) )
ROM_END

ROM_START( ssf2xj )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "sfxj.03c", 0x000000, 0x80000, CRC(a7417b79) SHA1(189c3ed546bb2844e9fa9fe7e9aacef728bc8939) )
	ROM_LOAD16_WORD_SWAP( "sfxj.04a", 0x080000, 0x80000, CRC(af7767b4) SHA1(61e7364408bf07c01634913c112b6245acce48ab) )
	ROM_LOAD16_WORD_SWAP( "sfxj.05",  0x100000, 0x80000, CRC(f4ff18f5) SHA1(aa713c9e1a2eba35bf1c9b40bb262ff7e46b9ce4) )
	ROM_LOAD16_WORD_SWAP( "sfxj.06a", 0x180000, 0x80000, CRC(260d0370) SHA1(5339cf87000caef74d491815391be59cfd701c8b) )
	ROM_LOAD16_WORD_SWAP( "sfxj.07",  0x200000, 0x80000, CRC(1324d02a) SHA1(c23a6ea09819bd33b6e2f58aa28c317ce53a46a0) )
	ROM_LOAD16_WORD_SWAP( "sfxj.08",  0x280000, 0x80000, CRC(2de76f10) SHA1(8cbe96dfeaa41306caa2819b82272ce3b0b9f926) )
	ROM_LOAD16_WORD_SWAP( "sfx.09",   0x300000, 0x80000, CRC(642fae3f) SHA1(746df99b826b9837bba267104132161153c1daff) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROMX_LOAD( "sfx.13m",   0x000000, 0x200000, CRC(cf94d275) SHA1(bf2a6d98a656d1cb5734da7836686242d3211137) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.15m",   0x000002, 0x200000, CRC(5eb703af) SHA1(4b302dbb66e8a5c2ad92798699391e981bada427) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.17m",   0x000004, 0x200000, CRC(ffa60e0f) SHA1(b21b1c749a8241440879bf8e7cb33968ccef97e5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.19m",   0x000006, 0x200000, CRC(34e825c5) SHA1(4d320fc96d1ef0b9928a8ce801734245a4c097a5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.14m",   0x800000, 0x100000, CRC(b7cc32e7) SHA1(0f4d26af338dab5dce5b7b34d32ad0c573434ace) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.16m",   0x800002, 0x100000, CRC(8376ad18) SHA1(f4456833fb396e6501f4174c0fe5fd63ea40a188) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.18m",   0x800004, 0x100000, CRC(f5b1b336) SHA1(4b060501e56b9d61294748da5387cdae5280ec4d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.20m",   0x800006, 0x100000, CRC(459d5c6b) SHA1(32b11ba7a12004aff810d719bff7508204c7b7c0) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.21m",   0xc00000, 0x100000, CRC(e32854af) SHA1(1a5e11e9caa2b96108d89ae660ef1f6bcb469a74) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.23m",   0xc00002, 0x100000, CRC(760f2927) SHA1(491e28e14ee06821fc9e709efa7b91313bc0c2db) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.25m",   0xc00004, 0x100000, CRC(1ee90208) SHA1(83df1d9953560edddc2951ea426d29fb014e6a8a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.27m",   0xc00006, 0x100000, CRC(f814400f) SHA1(ad6921af36d0bd5dfb89b1fb53c3ca3fd92d7204) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sfx.01",   0x00000, 0x08000, CRC(b47b8835) SHA1(c8b2d50fe3a329bd0592ea160d505155d873dab1) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "sfx.02",   0x28000, 0x20000, CRC(0022633f) SHA1(cab3afc79da53e3887eb1ccd1f4d19790728e6cd) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "sfx.11m",   0x000000, 0x200000, CRC(9bdbd476) SHA1(a8520f77f30b97aae36408e0c4ca1ebbde1808a5) )
	ROM_LOAD16_WORD_SWAP( "sfx.12m",   0x200000, 0x200000, CRC(a05e3aab) SHA1(d4eb9cae66c74e956569fea8b815156fbd420f83) )
ROM_END

ROM_START( vhunt2 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "vh2j.03a", 0x000000, 0x80000, CRC(9ae8f186) SHA1(f4e3a1b6ae823737d3b18561469f206921b48587) )
	ROM_LOAD16_WORD_SWAP( "vh2j.04a", 0x080000, 0x80000, CRC(e2fabf53) SHA1(78c53f8e984b00245486b751515248879df77437) )
	ROM_LOAD16_WORD_SWAP( "vh2j.05",  0x100000, 0x80000, CRC(de34f624) SHA1(60bbbd1765e76839b01c38765da2368c5188ec61) )
	ROM_LOAD16_WORD_SWAP( "vh2j.06",  0x180000, 0x80000, CRC(6a3b9897) SHA1(4f3b37004db8a3d3dde709b51c94c392615134b5) )
	ROM_LOAD16_WORD_SWAP( "vh2j.07",  0x200000, 0x80000, CRC(b021c029) SHA1(de4299197600608e83fe50775e3f352f5add844d) )
	ROM_LOAD16_WORD_SWAP( "vh2j.08",  0x280000, 0x80000, CRC(ac873dff) SHA1(ad9a085b8403801035683b6f63eee33daf4e97ae) )
	ROM_LOAD16_WORD_SWAP( "vh2j.09",  0x300000, 0x80000, CRC(eaefce9c) SHA1(d842a824f0d0adc13a86f780084164c1273c45a4) )
	ROM_LOAD16_WORD_SWAP( "vh2j.10",  0x380000, 0x80000, CRC(11730952) SHA1(2966b80b99ab065614a6ddb546110f482b998e32) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "vh2.13m",   0x0000000, 0x400000, CRC(3b02ddaa) SHA1(a73b0554afbfc7ace41bdf8e6cafd4c1ef0b0a08) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vh2.15m",   0x0000002, 0x400000, CRC(4e40de66) SHA1(e8b80eadffad6070aa04c8ab426311c44e7c5507) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vh2.17m",   0x0000004, 0x400000, CRC(b31d00c9) SHA1(7e7be64690663f52d10c8946aabec4250c8a8740) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vh2.19m",   0x0000006, 0x400000, CRC(149be3ab) SHA1(afc8e96e6aa3cf1db6dfd8075030a6c50b4419a9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vh2.14m",   0x1000000, 0x400000, CRC(cd09bd63) SHA1(e582b20a948ae54f52590496051688dbfae2bc9c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vh2.16m",   0x1000002, 0x400000, CRC(e0182c15) SHA1(a924d53ab39f4d85173bdb92a197dde2db0dc3f7) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vh2.18m",   0x1000004, 0x400000, CRC(778dc4f6) SHA1(8d0cd1c387b4b6ac7f92bb2e5a25983856328cdc) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vh2.20m",   0x1000006, 0x400000, CRC(605d9d1d) SHA1(99bc27557741527ca678d7b6307164bc04ebedc6) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "vh2.01",  0x00000, 0x08000, CRC(67b9f779) SHA1(3994c65f888004b56ea9f478b1feaa81e306347e) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "vh2.02",  0x28000, 0x20000, CRC(aaf15fcb) SHA1(6f61daa162c835165a8aabaf1d0ea8816fbfbd40) )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "vh2.11m",  0x000000, 0x400000, CRC(38922efd) SHA1(8cfb36bdce3a524d0a81fec12ca0cba82222fa30) )
	ROM_LOAD16_WORD_SWAP( "vh2.12m",  0x400000, 0x400000, CRC(6e2430af) SHA1(b475faf943bec4171ba0130f287e1948743ca273) )
ROM_END

ROM_START( vhunt2r1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "vh2j.03", 0x000000, 0x80000, CRC(1a5feb13) SHA1(a6dd6af2601e2da14032bcbf17e9f79c7a4ba2db) )
	ROM_LOAD16_WORD_SWAP( "vh2j.04", 0x080000, 0x80000, CRC(434611a5) SHA1(ee093017405db6c16bfee3fe446bae659c6accc2) )
	ROM_LOAD16_WORD_SWAP( "vh2j.05", 0x100000, 0x80000, CRC(de34f624) SHA1(60bbbd1765e76839b01c38765da2368c5188ec61) )
	ROM_LOAD16_WORD_SWAP( "vh2j.06", 0x180000, 0x80000, CRC(6a3b9897) SHA1(4f3b37004db8a3d3dde709b51c94c392615134b5) )
	ROM_LOAD16_WORD_SWAP( "vh2j.07", 0x200000, 0x80000, CRC(b021c029) SHA1(de4299197600608e83fe50775e3f352f5add844d) )
	ROM_LOAD16_WORD_SWAP( "vh2j.08", 0x280000, 0x80000, CRC(ac873dff) SHA1(ad9a085b8403801035683b6f63eee33daf4e97ae) )
	ROM_LOAD16_WORD_SWAP( "vh2j.09", 0x300000, 0x80000, CRC(eaefce9c) SHA1(d842a824f0d0adc13a86f780084164c1273c45a4) )
	ROM_LOAD16_WORD_SWAP( "vh2j.10", 0x380000, 0x80000, CRC(11730952) SHA1(2966b80b99ab065614a6ddb546110f482b998e32) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "vh2.13m",   0x0000000, 0x400000, CRC(3b02ddaa) SHA1(a73b0554afbfc7ace41bdf8e6cafd4c1ef0b0a08) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vh2.15m",   0x0000002, 0x400000, CRC(4e40de66) SHA1(e8b80eadffad6070aa04c8ab426311c44e7c5507) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vh2.17m",   0x0000004, 0x400000, CRC(b31d00c9) SHA1(7e7be64690663f52d10c8946aabec4250c8a8740) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vh2.19m",   0x0000006, 0x400000, CRC(149be3ab) SHA1(afc8e96e6aa3cf1db6dfd8075030a6c50b4419a9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vh2.14m",   0x1000000, 0x400000, CRC(cd09bd63) SHA1(e582b20a948ae54f52590496051688dbfae2bc9c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vh2.16m",   0x1000002, 0x400000, CRC(e0182c15) SHA1(a924d53ab39f4d85173bdb92a197dde2db0dc3f7) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vh2.18m",   0x1000004, 0x400000, CRC(778dc4f6) SHA1(8d0cd1c387b4b6ac7f92bb2e5a25983856328cdc) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vh2.20m",   0x1000006, 0x400000, CRC(605d9d1d) SHA1(99bc27557741527ca678d7b6307164bc04ebedc6) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "vh2.01",  0x00000, 0x08000, CRC(67b9f779) SHA1(3994c65f888004b56ea9f478b1feaa81e306347e) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "vh2.02",  0x28000, 0x20000, CRC(aaf15fcb) SHA1(6f61daa162c835165a8aabaf1d0ea8816fbfbd40) )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "vh2.11m",  0x000000, 0x400000, CRC(38922efd) SHA1(8cfb36bdce3a524d0a81fec12ca0cba82222fa30) )
	ROM_LOAD16_WORD_SWAP( "vh2.12m",  0x400000, 0x400000, CRC(6e2430af) SHA1(b475faf943bec4171ba0130f287e1948743ca273) )
ROM_END

ROM_START( vsav )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "vm3e.03d", 0x000000, 0x80000, CRC(f5962a8c) SHA1(e37d48b78186c7c097894d6c17faf7c9333f61eb) )
	ROM_LOAD16_WORD_SWAP( "vm3e.04d", 0x080000, 0x80000, CRC(21b40ea2) SHA1(6790fa3e618850f518cbd470f44434a71be6f29f) )
	ROM_LOAD16_WORD_SWAP( "vm3.05a",  0x100000, 0x80000, CRC(4118e00f) SHA1(94ce8abc5ff547667f4c6022d84d0ed4cd062d7e) )
	ROM_LOAD16_WORD_SWAP( "vm3.06a",  0x180000, 0x80000, CRC(2f4fd3a9) SHA1(48549ff0121312ea4a18d0fa167a32f905c14c9f) )
	ROM_LOAD16_WORD_SWAP( "vm3.07b",  0x200000, 0x80000, CRC(cbda91b8) SHA1(31b20aa92422384b1d7a4706ad4c01ea2bd0e0d1) )
	ROM_LOAD16_WORD_SWAP( "vm3.08a",  0x280000, 0x80000, CRC(6ca47259) SHA1(485d8f3a132ccb3f7930cae74de8662d2d44e412) )
	ROM_LOAD16_WORD_SWAP( "vm3.09b",  0x300000, 0x80000, CRC(f4a339e3) SHA1(abd101a55f7d9ddb8aba04fe8d3f0f5d2006c925) )
	ROM_LOAD16_WORD_SWAP( "vm3.10b",  0x380000, 0x80000, CRC(fffbb5b8) SHA1(38aecb820bd1cbd17287848c3ffb013e1d464ddf) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "vm3.13m",   0x0000000, 0x400000, CRC(fd8a11eb) SHA1(21b9773959e17976ff46b75a6a405042836b2c5f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vm3.15m",   0x0000002, 0x400000, CRC(dd1e7d4e) SHA1(30476e061cdebdb1838b83f4ebd5efae12b7dbfb) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vm3.17m",   0x0000004, 0x400000, CRC(6b89445e) SHA1(2abd489839d143c46e25f4fc3db476b70607dc03) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vm3.19m",   0x0000006, 0x400000, CRC(3830fdc7) SHA1(ebd3f559c254d349e256c9feb3477f1ed7518206) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vm3.14m",   0x1000000, 0x400000, CRC(c1a28e6c) SHA1(012803af33174c0602649d2a2d84f6ee79f54ad2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vm3.16m",   0x1000002, 0x400000, CRC(194a7304) SHA1(a19a9a6fb829953b054dc5c3b0dc017f60d37928) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vm3.18m",   0x1000004, 0x400000, CRC(df9a9f47) SHA1(ce29ff00cf4b6fdd9b3b1ed87823534f1d364eab) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vm3.20m",   0x1000006, 0x400000, CRC(c22fc3d9) SHA1(df7538c05b03a4ad94d369f8083799979e6fac42) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "vm3.01",   0x00000, 0x08000, CRC(f778769b) SHA1(788ce1ad8a322179f634df9e62a31ad776b96762) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "vm3.02",   0x28000, 0x20000, CRC(cc09faa1) SHA1(2962ef0ceaf7e7279de3c421ea998763330eb43e) )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "vm3.11m",   0x000000, 0x400000, CRC(e80e956e) SHA1(74181fca4b764fb3c56ceef2cb4c6fd6c18ec4b6) )
	ROM_LOAD16_WORD_SWAP( "vm3.12m",   0x400000, 0x400000, CRC(9cd71557) SHA1(7059db25698a0b286314c5961c618f6d2e6f24a1) )
ROM_END

ROM_START( vsavu )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "vm3u.03d", 0x000000, 0x80000, CRC(1f295274) SHA1(c926d8af4fccee5104507ee0196b05dcd419ee20) )
	ROM_LOAD16_WORD_SWAP( "vm3u.04d", 0x080000, 0x80000, CRC(c46adf81) SHA1(85ffb9b3282874d6ce9318a88429666e98f67cea) )
	ROM_LOAD16_WORD_SWAP( "vm3.05a",  0x100000, 0x80000, CRC(4118e00f) SHA1(94ce8abc5ff547667f4c6022d84d0ed4cd062d7e) )
	ROM_LOAD16_WORD_SWAP( "vm3.06a",  0x180000, 0x80000, CRC(2f4fd3a9) SHA1(48549ff0121312ea4a18d0fa167a32f905c14c9f) )
	ROM_LOAD16_WORD_SWAP( "vm3.07b",  0x200000, 0x80000, CRC(cbda91b8) SHA1(31b20aa92422384b1d7a4706ad4c01ea2bd0e0d1) )
	ROM_LOAD16_WORD_SWAP( "vm3.08a",  0x280000, 0x80000, CRC(6ca47259) SHA1(485d8f3a132ccb3f7930cae74de8662d2d44e412) )
	ROM_LOAD16_WORD_SWAP( "vm3.09b",  0x300000, 0x80000, CRC(f4a339e3) SHA1(abd101a55f7d9ddb8aba04fe8d3f0f5d2006c925) )
	ROM_LOAD16_WORD_SWAP( "vm3.10b",  0x380000, 0x80000, CRC(fffbb5b8) SHA1(38aecb820bd1cbd17287848c3ffb013e1d464ddf) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "vm3.13m",   0x0000000, 0x400000, CRC(fd8a11eb) SHA1(21b9773959e17976ff46b75a6a405042836b2c5f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vm3.15m",   0x0000002, 0x400000, CRC(dd1e7d4e) SHA1(30476e061cdebdb1838b83f4ebd5efae12b7dbfb) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vm3.17m",   0x0000004, 0x400000, CRC(6b89445e) SHA1(2abd489839d143c46e25f4fc3db476b70607dc03) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vm3.19m",   0x0000006, 0x400000, CRC(3830fdc7) SHA1(ebd3f559c254d349e256c9feb3477f1ed7518206) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vm3.14m",   0x1000000, 0x400000, CRC(c1a28e6c) SHA1(012803af33174c0602649d2a2d84f6ee79f54ad2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vm3.16m",   0x1000002, 0x400000, CRC(194a7304) SHA1(a19a9a6fb829953b054dc5c3b0dc017f60d37928) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vm3.18m",   0x1000004, 0x400000, CRC(df9a9f47) SHA1(ce29ff00cf4b6fdd9b3b1ed87823534f1d364eab) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vm3.20m",   0x1000006, 0x400000, CRC(c22fc3d9) SHA1(df7538c05b03a4ad94d369f8083799979e6fac42) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "vm3.01",   0x00000, 0x08000, CRC(f778769b) SHA1(788ce1ad8a322179f634df9e62a31ad776b96762) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "vm3.02",   0x28000, 0x20000, CRC(cc09faa1) SHA1(2962ef0ceaf7e7279de3c421ea998763330eb43e) )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "vm3.11m",   0x000000, 0x400000, CRC(e80e956e) SHA1(74181fca4b764fb3c56ceef2cb4c6fd6c18ec4b6) )
	ROM_LOAD16_WORD_SWAP( "vm3.12m",   0x400000, 0x400000, CRC(9cd71557) SHA1(7059db25698a0b286314c5961c618f6d2e6f24a1) )
ROM_END

ROM_START( vsavj )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "vm3j.03d", 0x000000, 0x80000, CRC(2a2e74a4) SHA1(e9e7bce3c2ad0c9eebcbcd5139979d1fa19187ad) )
	ROM_LOAD16_WORD_SWAP( "vm3j.04d", 0x080000, 0x80000, CRC(1c2427bc) SHA1(9047ac0ccd875d91a8ceafdf1ccf9d21c4c71644) )
	ROM_LOAD16_WORD_SWAP( "vm3j.05a", 0x100000, 0x80000, CRC(95ce88d5) SHA1(ba5e64c2551d97a71d2f4d7a78663aede4b722e8) )
	ROM_LOAD16_WORD_SWAP( "vm3j.06b", 0x180000, 0x80000, CRC(2c4297e0) SHA1(3a7103456ba3937f63c28dd42020cac1955b5741) )
	ROM_LOAD16_WORD_SWAP( "vm3j.07b", 0x200000, 0x80000, CRC(a38aaae7) SHA1(0a5719eb2b0bbde955f605b1057ed6a8eb54ad80) )
	ROM_LOAD16_WORD_SWAP( "vm3j.08a", 0x280000, 0x80000, CRC(5773e5c9) SHA1(551afc5d921f9ef1fe928ca83d072b6a6105ab0e) )
	ROM_LOAD16_WORD_SWAP( "vm3j.09b", 0x300000, 0x80000, CRC(d064f8b9) SHA1(09f77f7b466c147a5d894a4ec3b40bd068dfab26) )
	ROM_LOAD16_WORD_SWAP( "vm3j.10b", 0x380000, 0x80000, CRC(434518e9) SHA1(ce1c8557a9e6c5451ab41a96f01b0cd4ba02ea3e) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "vm3.13m",   0x0000000, 0x400000, CRC(fd8a11eb) SHA1(21b9773959e17976ff46b75a6a405042836b2c5f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vm3.15m",   0x0000002, 0x400000, CRC(dd1e7d4e) SHA1(30476e061cdebdb1838b83f4ebd5efae12b7dbfb) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vm3.17m",   0x0000004, 0x400000, CRC(6b89445e) SHA1(2abd489839d143c46e25f4fc3db476b70607dc03) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vm3.19m",   0x0000006, 0x400000, CRC(3830fdc7) SHA1(ebd3f559c254d349e256c9feb3477f1ed7518206) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vm3.14m",   0x1000000, 0x400000, CRC(c1a28e6c) SHA1(012803af33174c0602649d2a2d84f6ee79f54ad2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vm3.16m",   0x1000002, 0x400000, CRC(194a7304) SHA1(a19a9a6fb829953b054dc5c3b0dc017f60d37928) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vm3.18m",   0x1000004, 0x400000, CRC(df9a9f47) SHA1(ce29ff00cf4b6fdd9b3b1ed87823534f1d364eab) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vm3.20m",   0x1000006, 0x400000, CRC(c22fc3d9) SHA1(df7538c05b03a4ad94d369f8083799979e6fac42) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "vm3.01",   0x00000, 0x08000, CRC(f778769b) SHA1(788ce1ad8a322179f634df9e62a31ad776b96762) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "vm3.02",   0x28000, 0x20000, CRC(cc09faa1) SHA1(2962ef0ceaf7e7279de3c421ea998763330eb43e) )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "vm3.11m",   0x000000, 0x400000, CRC(e80e956e) SHA1(74181fca4b764fb3c56ceef2cb4c6fd6c18ec4b6) )
	ROM_LOAD16_WORD_SWAP( "vm3.12m",   0x400000, 0x400000, CRC(9cd71557) SHA1(7059db25698a0b286314c5961c618f6d2e6f24a1) )
ROM_END

ROM_START( vsava )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "vm3a.03d", 0x000000, 0x80000, CRC(44c1198f) SHA1(35714b2f6ebeafea93be6467b5b22ea41b9f3826) )
	ROM_LOAD16_WORD_SWAP( "vm3a.04d", 0x080000, 0x80000, CRC(2218b781) SHA1(5dd28cc1b70b2953fbd4d5fd14abeeb3b83b193e) )
	ROM_LOAD16_WORD_SWAP( "vm3.05a",  0x100000, 0x80000, CRC(4118e00f) SHA1(94ce8abc5ff547667f4c6022d84d0ed4cd062d7e) )
	ROM_LOAD16_WORD_SWAP( "vm3.06a",  0x180000, 0x80000, CRC(2f4fd3a9) SHA1(48549ff0121312ea4a18d0fa167a32f905c14c9f) )
	ROM_LOAD16_WORD_SWAP( "vm3.07b",  0x200000, 0x80000, CRC(cbda91b8) SHA1(31b20aa92422384b1d7a4706ad4c01ea2bd0e0d1) )
	ROM_LOAD16_WORD_SWAP( "vm3.08a",  0x280000, 0x80000, CRC(6ca47259) SHA1(485d8f3a132ccb3f7930cae74de8662d2d44e412) )
	ROM_LOAD16_WORD_SWAP( "vm3.09b",  0x300000, 0x80000, CRC(f4a339e3) SHA1(abd101a55f7d9ddb8aba04fe8d3f0f5d2006c925) )
	ROM_LOAD16_WORD_SWAP( "vm3.10b",  0x380000, 0x80000, CRC(fffbb5b8) SHA1(38aecb820bd1cbd17287848c3ffb013e1d464ddf) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "vm3.13m",   0x0000000, 0x400000, CRC(fd8a11eb) SHA1(21b9773959e17976ff46b75a6a405042836b2c5f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vm3.15m",   0x0000002, 0x400000, CRC(dd1e7d4e) SHA1(30476e061cdebdb1838b83f4ebd5efae12b7dbfb) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vm3.17m",   0x0000004, 0x400000, CRC(6b89445e) SHA1(2abd489839d143c46e25f4fc3db476b70607dc03) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vm3.19m",   0x0000006, 0x400000, CRC(3830fdc7) SHA1(ebd3f559c254d349e256c9feb3477f1ed7518206) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vm3.14m",   0x1000000, 0x400000, CRC(c1a28e6c) SHA1(012803af33174c0602649d2a2d84f6ee79f54ad2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vm3.16m",   0x1000002, 0x400000, CRC(194a7304) SHA1(a19a9a6fb829953b054dc5c3b0dc017f60d37928) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vm3.18m",   0x1000004, 0x400000, CRC(df9a9f47) SHA1(ce29ff00cf4b6fdd9b3b1ed87823534f1d364eab) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vm3.20m",   0x1000006, 0x400000, CRC(c22fc3d9) SHA1(df7538c05b03a4ad94d369f8083799979e6fac42) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "vm3.01",   0x00000, 0x08000, CRC(f778769b) SHA1(788ce1ad8a322179f634df9e62a31ad776b96762) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "vm3.02",   0x28000, 0x20000, CRC(cc09faa1) SHA1(2962ef0ceaf7e7279de3c421ea998763330eb43e) )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "vm3.11m",   0x000000, 0x400000, CRC(e80e956e) SHA1(74181fca4b764fb3c56ceef2cb4c6fd6c18ec4b6) )
	ROM_LOAD16_WORD_SWAP( "vm3.12m",   0x400000, 0x400000, CRC(9cd71557) SHA1(7059db25698a0b286314c5961c618f6d2e6f24a1) )
ROM_END

ROM_START( vsavh )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "vm3h.03a", 0x000000, 0x80000, CRC(7cc62df8) SHA1(716ad31f0e253868a5b1b89943ddc980f130d5b7) )
	ROM_LOAD16_WORD_SWAP( "vm3h.04d", 0x080000, 0x80000, CRC(d716f3b5) SHA1(7900440071eafa4d1559b1fa8faefaa0588a65d5) )
	ROM_LOAD16_WORD_SWAP( "vm3.05a",  0x100000, 0x80000, CRC(4118e00f) SHA1(94ce8abc5ff547667f4c6022d84d0ed4cd062d7e) )
	ROM_LOAD16_WORD_SWAP( "vm3.06a",  0x180000, 0x80000, CRC(2f4fd3a9) SHA1(48549ff0121312ea4a18d0fa167a32f905c14c9f) )
	ROM_LOAD16_WORD_SWAP( "vm3.07b",  0x200000, 0x80000, CRC(cbda91b8) SHA1(31b20aa92422384b1d7a4706ad4c01ea2bd0e0d1) )
	ROM_LOAD16_WORD_SWAP( "vm3.08a",  0x280000, 0x80000, CRC(6ca47259) SHA1(485d8f3a132ccb3f7930cae74de8662d2d44e412) )
	ROM_LOAD16_WORD_SWAP( "vm3.09b",  0x300000, 0x80000, CRC(f4a339e3) SHA1(abd101a55f7d9ddb8aba04fe8d3f0f5d2006c925) )
	ROM_LOAD16_WORD_SWAP( "vm3.10b",  0x380000, 0x80000, CRC(fffbb5b8) SHA1(38aecb820bd1cbd17287848c3ffb013e1d464ddf) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "vm3.13m",   0x0000000, 0x400000, CRC(fd8a11eb) SHA1(21b9773959e17976ff46b75a6a405042836b2c5f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vm3.15m",   0x0000002, 0x400000, CRC(dd1e7d4e) SHA1(30476e061cdebdb1838b83f4ebd5efae12b7dbfb) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vm3.17m",   0x0000004, 0x400000, CRC(6b89445e) SHA1(2abd489839d143c46e25f4fc3db476b70607dc03) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vm3.19m",   0x0000006, 0x400000, CRC(3830fdc7) SHA1(ebd3f559c254d349e256c9feb3477f1ed7518206) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vm3.14m",   0x1000000, 0x400000, CRC(c1a28e6c) SHA1(012803af33174c0602649d2a2d84f6ee79f54ad2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vm3.16m",   0x1000002, 0x400000, CRC(194a7304) SHA1(a19a9a6fb829953b054dc5c3b0dc017f60d37928) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vm3.18m",   0x1000004, 0x400000, CRC(df9a9f47) SHA1(ce29ff00cf4b6fdd9b3b1ed87823534f1d364eab) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vm3.20m",   0x1000006, 0x400000, CRC(c22fc3d9) SHA1(df7538c05b03a4ad94d369f8083799979e6fac42) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "vm3.01",   0x00000, 0x08000, CRC(f778769b) SHA1(788ce1ad8a322179f634df9e62a31ad776b96762) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "vm3.02",   0x28000, 0x20000, CRC(cc09faa1) SHA1(2962ef0ceaf7e7279de3c421ea998763330eb43e) )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "vm3.11m",   0x000000, 0x400000, CRC(e80e956e) SHA1(74181fca4b764fb3c56ceef2cb4c6fd6c18ec4b6) )
	ROM_LOAD16_WORD_SWAP( "vm3.12m",   0x400000, 0x400000, CRC(9cd71557) SHA1(7059db25698a0b286314c5961c618f6d2e6f24a1) )
ROM_END

ROM_START( vsav2 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "vs2j.03", 0x000000, 0x80000, CRC(89fd86b4) SHA1(a52f40618d7f12f1df5862ad8e15fea60bef22a2) )
	ROM_LOAD16_WORD_SWAP( "vs2j.04", 0x080000, 0x80000, CRC(107c091b) SHA1(bf5c2e4339e1a66b3c819900cc9b723a537adf6b) )
	ROM_LOAD16_WORD_SWAP( "vs2j.05", 0x100000, 0x80000, CRC(61979638) SHA1(4d5625a9a06926c1a42c8f6e3a4c943f17750ec2) )
	ROM_LOAD16_WORD_SWAP( "vs2j.06", 0x180000, 0x80000, CRC(f37c5bc2) SHA1(d8c1040a6ee6b9fc677a6a32b99bf02b6a707812) )
	ROM_LOAD16_WORD_SWAP( "vs2j.07", 0x200000, 0x80000, CRC(8f885809) SHA1(69dac07e1f483b6478f792d20a137d6a081fbea3) )
	ROM_LOAD16_WORD_SWAP( "vs2j.08", 0x280000, 0x80000, CRC(2018c120) SHA1(de1184ab771c6f075cdefa744a28b09f78d91643) )
	ROM_LOAD16_WORD_SWAP( "vs2j.09", 0x300000, 0x80000, CRC(fac3c217) SHA1(0e9dd54e401e6d7c4fe81107ffd27e42ca810fcb) )
	ROM_LOAD16_WORD_SWAP( "vs2j.10", 0x380000, 0x80000, CRC(eb490213) SHA1(bf0416df66a33c7a4678ab4a047de334dfd3b31e) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "vs2.13m",   0x0000000, 0x400000, CRC(5c852f52) SHA1(528ce7fc9a0451e2e2d221dbf5e4a5796584e053) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vs2.15m",   0x0000002, 0x400000, CRC(a20f58af) SHA1(e873ad3e0fc8a06a5029113faf991f5c1b765316) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vs2.17m",   0x0000004, 0x400000, CRC(39db59ad) SHA1(da94f1529da82a6bf2129f51548412e1ab2b001a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vs2.19m",   0x0000006, 0x400000, CRC(00c763a7) SHA1(0ff528e12e255ebf699101ac71f05b1f6bef7165) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vs2.14m",   0x1000000, 0x400000, CRC(cd09bd63) SHA1(e582b20a948ae54f52590496051688dbfae2bc9c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vs2.16m",   0x1000002, 0x400000, CRC(e0182c15) SHA1(a924d53ab39f4d85173bdb92a197dde2db0dc3f7) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vs2.18m",   0x1000004, 0x400000, CRC(778dc4f6) SHA1(8d0cd1c387b4b6ac7f92bb2e5a25983856328cdc) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vs2.20m",   0x1000006, 0x400000, CRC(605d9d1d) SHA1(99bc27557741527ca678d7b6307164bc04ebedc6) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "vs2.01",   0x00000, 0x08000, CRC(35190139) SHA1(07f8e53ea398461de5dcda9814dde7c09faf9f65) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "vs2.02",   0x28000, 0x20000, CRC(c32dba09) SHA1(1fe337ff334fab79847f9677ba0e168e93daa1c8) )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "vs2.11m",   0x000000, 0x400000, CRC(d67e47b7) SHA1(15a3f6779eccb10551ed94edf7e6e406a79b3de7) )
	ROM_LOAD16_WORD_SWAP( "vs2.12m",   0x400000, 0x400000, CRC(6d020a14) SHA1(e98f862fac1e357c90949768bb2646263d9981a0) )
ROM_END

ROM_START( xmcota )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "xmne.03e", 0x000000, 0x80000, CRC(a9a09b09) SHA1(e316f443d393139894592dbb1b676f3a2385ed14) )
	ROM_LOAD16_WORD_SWAP( "xmne.04e", 0x080000, 0x80000, CRC(52fa2106) SHA1(6904eef0fb11e44046e160a1c0ff6ea48337f630) )
	ROM_LOAD16_WORD_SWAP( "xmn.05a",  0x100000, 0x80000, CRC(ac0d7759) SHA1(650d4474b13f16af7910a0f721fcda2ddb2414fd) )
	ROM_LOAD16_WORD_SWAP( "xmn.06a",  0x180000, 0x80000, CRC(1b86a328) SHA1(2469cd705139ee9f1142e6e379e68d0c9675b37e) )
	ROM_LOAD16_WORD_SWAP( "xmn.07a",  0x200000, 0x80000, CRC(2c142a44) SHA1(7624875f9c39b361fc83e52e87e0fd5e96279713) )
	ROM_LOAD16_WORD_SWAP( "xmn.08a",  0x280000, 0x80000, CRC(f712d44f) SHA1(0d18d4a4eacad94a66beca6ec509ac7f690c6882) )
	ROM_LOAD16_WORD_SWAP( "xmn.09a",  0x300000, 0x80000, CRC(9241cae8) SHA1(bb6980abf25aaf3eb14e230ca6942f3e2ab2c660) )
	ROM_LOAD16_WORD_SWAP( "xmn.10a",  0x380000, 0x80000, CRC(53c0eab9) SHA1(e3b1ec1fd517735f7801cfebb257c43185c6d3fb) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "xmn.13m",   0x0000000, 0x400000, CRC(bf4df073) SHA1(4d2740c3a827f0ec2cf75ad99c65e393c6a11c23) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.15m",   0x0000002, 0x400000, CRC(4d7e4cef) SHA1(50b8797b8099a8d76ad063ba1201a13dbb88ae3a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.17m",   0x0000004, 0x400000, CRC(513eea17) SHA1(a497477ad9ac13180911d8745ef6ee1955c0b877) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.19m",   0x0000006, 0x400000, CRC(d23897fc) SHA1(1e31627999736652252164d32662779a1ac6ca29) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.14m",   0x1000000, 0x400000, CRC(778237b7) SHA1(89a759ec383518ec52f5059d10ec342f2247aa20) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.16m",   0x1000002, 0x400000, CRC(67b36948) SHA1(692fb6e4096b880aa22996d554b160f664bbd907) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.18m",   0x1000004, 0x400000, CRC(015a7c4c) SHA1(cccc95dafd076a1a9fa004710006149c42d058ba) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.20m",   0x1000006, 0x400000, CRC(9dde2758) SHA1(17ba259cad03c7b5d56c0a5eda9ab53521665729) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "xmn.01a",  0x00000, 0x08000, CRC(40f479ea) SHA1(f29e15f537675305264ae2138a0a537fb9e2008b) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "xmn.02a",  0x28000, 0x20000, CRC(39d9b5ad) SHA1(af502debfd36100d4fc971ed25fdf9d7121d6f18) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "xmn.11m",   0x000000, 0x200000, CRC(c848a6bc) SHA1(ac8ac564d3c43225822f8bc330eba9f35b24b0a4) )
	ROM_LOAD16_WORD_SWAP( "xmn.12m",   0x200000, 0x200000, CRC(729c188f) SHA1(3279774ad8aebbcf0fc779cdfcbe21044dd192ad) )
ROM_END

ROM_START( xmcotau )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "xmnu.03e", 0x000000, 0x80000, CRC(0bafeb0e) SHA1(170c819bd7ffafefb9b2a587509bdf2c0415474b) )
	ROM_LOAD16_WORD_SWAP( "xmnu.04e", 0x080000, 0x80000, CRC(c29bdae3) SHA1(c605a4fd90336459c7b24cd7b2b243eef10f6407) )
	ROM_LOAD16_WORD_SWAP( "xmn.05a",  0x100000, 0x80000, CRC(ac0d7759) SHA1(650d4474b13f16af7910a0f721fcda2ddb2414fd) )
	ROM_LOAD16_WORD_SWAP( "xmn.06a",  0x180000, 0x80000, CRC(1b86a328) SHA1(2469cd705139ee9f1142e6e379e68d0c9675b37e) )
	ROM_LOAD16_WORD_SWAP( "xmn.07a",  0x200000, 0x80000, CRC(2c142a44) SHA1(7624875f9c39b361fc83e52e87e0fd5e96279713) )
	ROM_LOAD16_WORD_SWAP( "xmn.08a",  0x280000, 0x80000, CRC(f712d44f) SHA1(0d18d4a4eacad94a66beca6ec509ac7f690c6882) )
	ROM_LOAD16_WORD_SWAP( "xmn.09a",  0x300000, 0x80000, CRC(9241cae8) SHA1(bb6980abf25aaf3eb14e230ca6942f3e2ab2c660) )
	ROM_LOAD16_WORD_SWAP( "xmn.10a",  0x380000, 0x80000, CRC(53c0eab9) SHA1(e3b1ec1fd517735f7801cfebb257c43185c6d3fb) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "xmn.13m",   0x0000000, 0x400000, CRC(bf4df073) SHA1(4d2740c3a827f0ec2cf75ad99c65e393c6a11c23) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.15m",   0x0000002, 0x400000, CRC(4d7e4cef) SHA1(50b8797b8099a8d76ad063ba1201a13dbb88ae3a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.17m",   0x0000004, 0x400000, CRC(513eea17) SHA1(a497477ad9ac13180911d8745ef6ee1955c0b877) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.19m",   0x0000006, 0x400000, CRC(d23897fc) SHA1(1e31627999736652252164d32662779a1ac6ca29) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.14m",   0x1000000, 0x400000, CRC(778237b7) SHA1(89a759ec383518ec52f5059d10ec342f2247aa20) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.16m",   0x1000002, 0x400000, CRC(67b36948) SHA1(692fb6e4096b880aa22996d554b160f664bbd907) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.18m",   0x1000004, 0x400000, CRC(015a7c4c) SHA1(cccc95dafd076a1a9fa004710006149c42d058ba) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.20m",   0x1000006, 0x400000, CRC(9dde2758) SHA1(17ba259cad03c7b5d56c0a5eda9ab53521665729) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "xmn.01a",  0x00000, 0x08000, CRC(40f479ea) SHA1(f29e15f537675305264ae2138a0a537fb9e2008b) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "xmn.02a",  0x28000, 0x20000, CRC(39d9b5ad) SHA1(af502debfd36100d4fc971ed25fdf9d7121d6f18) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "xmn.11m",   0x000000, 0x200000, CRC(c848a6bc) SHA1(ac8ac564d3c43225822f8bc330eba9f35b24b0a4) )
	ROM_LOAD16_WORD_SWAP( "xmn.12m",   0x200000, 0x200000, CRC(729c188f) SHA1(3279774ad8aebbcf0fc779cdfcbe21044dd192ad) )
ROM_END

ROM_START( xmcotah )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "xmnh.03",  0x000000, 0x80000, CRC(e4b85a90) SHA1(1eaf94ce42438eea45cd5c813f2859abf258dd3a) )  /* These are later date, but no revision leter code? */
	ROM_LOAD16_WORD_SWAP( "xmnh.04",  0x080000, 0x80000, CRC(7dfe1406) SHA1(4ddc0a8947d78ce587220f8188c8a8f00c7372c4) )  /* These are later date, but no revision leter code? */
	ROM_LOAD16_WORD_SWAP( "xmnh.05",  0x100000, 0x80000, CRC(87b0ed0f) SHA1(f4d78fdd9fcf864e909d9a2bb351b49a5f8ec7a0) )  /* These are later date, but no revision leter code? */
	ROM_LOAD16_WORD_SWAP( "xmn.06a",  0x180000, 0x80000, CRC(1b86a328) SHA1(2469cd705139ee9f1142e6e379e68d0c9675b37e) )
	ROM_LOAD16_WORD_SWAP( "xmn.07a",  0x200000, 0x80000, CRC(2c142a44) SHA1(7624875f9c39b361fc83e52e87e0fd5e96279713) )
	ROM_LOAD16_WORD_SWAP( "xmn.08a",  0x280000, 0x80000, CRC(f712d44f) SHA1(0d18d4a4eacad94a66beca6ec509ac7f690c6882) )
	ROM_LOAD16_WORD_SWAP( "xmn.09a",  0x300000, 0x80000, CRC(9241cae8) SHA1(bb6980abf25aaf3eb14e230ca6942f3e2ab2c660) )
	ROM_LOAD16_WORD_SWAP( "xmnh.10",  0x380000, 0x80000, CRC(cb36b0a4) SHA1(f21e3f2da405dfe43843ad32d381ea51f5d2fdd7) )  /* These are later date, but no revision leter code? */

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "xmn.13m",   0x0000000, 0x400000, CRC(bf4df073) SHA1(4d2740c3a827f0ec2cf75ad99c65e393c6a11c23) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.15m",   0x0000002, 0x400000, CRC(4d7e4cef) SHA1(50b8797b8099a8d76ad063ba1201a13dbb88ae3a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.17m",   0x0000004, 0x400000, CRC(513eea17) SHA1(a497477ad9ac13180911d8745ef6ee1955c0b877) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.19m",   0x0000006, 0x400000, CRC(d23897fc) SHA1(1e31627999736652252164d32662779a1ac6ca29) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.14m",   0x1000000, 0x400000, CRC(778237b7) SHA1(89a759ec383518ec52f5059d10ec342f2247aa20) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.16m",   0x1000002, 0x400000, CRC(67b36948) SHA1(692fb6e4096b880aa22996d554b160f664bbd907) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.18m",   0x1000004, 0x400000, CRC(015a7c4c) SHA1(cccc95dafd076a1a9fa004710006149c42d058ba) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.20m",   0x1000006, 0x400000, CRC(9dde2758) SHA1(17ba259cad03c7b5d56c0a5eda9ab53521665729) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "xmn.01a",  0x00000, 0x08000, CRC(40f479ea) SHA1(f29e15f537675305264ae2138a0a537fb9e2008b) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "xmn.02a",  0x28000, 0x20000, CRC(39d9b5ad) SHA1(af502debfd36100d4fc971ed25fdf9d7121d6f18) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "xmn.11m",   0x000000, 0x200000, CRC(c848a6bc) SHA1(ac8ac564d3c43225822f8bc330eba9f35b24b0a4) )
	ROM_LOAD16_WORD_SWAP( "xmn.12m",   0x200000, 0x200000, CRC(729c188f) SHA1(3279774ad8aebbcf0fc779cdfcbe21044dd192ad) )
ROM_END

ROM_START( xmcotahr1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "xmnh.03d", 0x000000, 0x80000, CRC(63b0a84f) SHA1(4a0d604ff11e68313a0711702803390cb0d2f234) )
	ROM_LOAD16_WORD_SWAP( "xmnh.04d", 0x080000, 0x80000, CRC(b1b9b727) SHA1(b04ae3b7aab88cbb1a55068343ff9a9806d331a4) )
	ROM_LOAD16_WORD_SWAP( "xmn.05a",  0x100000, 0x80000, CRC(ac0d7759) SHA1(650d4474b13f16af7910a0f721fcda2ddb2414fd) )
	ROM_LOAD16_WORD_SWAP( "xmn.06a",  0x180000, 0x80000, CRC(1b86a328) SHA1(2469cd705139ee9f1142e6e379e68d0c9675b37e) )
	ROM_LOAD16_WORD_SWAP( "xmn.07a",  0x200000, 0x80000, CRC(2c142a44) SHA1(7624875f9c39b361fc83e52e87e0fd5e96279713) )
	ROM_LOAD16_WORD_SWAP( "xmn.08a",  0x280000, 0x80000, CRC(f712d44f) SHA1(0d18d4a4eacad94a66beca6ec509ac7f690c6882) )
	ROM_LOAD16_WORD_SWAP( "xmn.09a",  0x300000, 0x80000, CRC(9241cae8) SHA1(bb6980abf25aaf3eb14e230ca6942f3e2ab2c660) )
	ROM_LOAD16_WORD_SWAP( "xmn.10a",  0x380000, 0x80000, CRC(53c0eab9) SHA1(e3b1ec1fd517735f7801cfebb257c43185c6d3fb) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "xmn.13m",   0x0000000, 0x400000, CRC(bf4df073) SHA1(4d2740c3a827f0ec2cf75ad99c65e393c6a11c23) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.15m",   0x0000002, 0x400000, CRC(4d7e4cef) SHA1(50b8797b8099a8d76ad063ba1201a13dbb88ae3a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.17m",   0x0000004, 0x400000, CRC(513eea17) SHA1(a497477ad9ac13180911d8745ef6ee1955c0b877) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.19m",   0x0000006, 0x400000, CRC(d23897fc) SHA1(1e31627999736652252164d32662779a1ac6ca29) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.14m",   0x1000000, 0x400000, CRC(778237b7) SHA1(89a759ec383518ec52f5059d10ec342f2247aa20) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.16m",   0x1000002, 0x400000, CRC(67b36948) SHA1(692fb6e4096b880aa22996d554b160f664bbd907) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.18m",   0x1000004, 0x400000, CRC(015a7c4c) SHA1(cccc95dafd076a1a9fa004710006149c42d058ba) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.20m",   0x1000006, 0x400000, CRC(9dde2758) SHA1(17ba259cad03c7b5d56c0a5eda9ab53521665729) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "xmn.01a",  0x00000, 0x08000, CRC(40f479ea) SHA1(f29e15f537675305264ae2138a0a537fb9e2008b) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "xmn.02a",  0x28000, 0x20000, CRC(39d9b5ad) SHA1(af502debfd36100d4fc971ed25fdf9d7121d6f18) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "xmn.11m",   0x000000, 0x200000, CRC(c848a6bc) SHA1(ac8ac564d3c43225822f8bc330eba9f35b24b0a4) )
	ROM_LOAD16_WORD_SWAP( "xmn.12m",   0x200000, 0x200000, CRC(729c188f) SHA1(3279774ad8aebbcf0fc779cdfcbe21044dd192ad) )
ROM_END

ROM_START( xmcotaj )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "xmnj.03e", 0x000000, 0x80000, CRC(0df29f5f) SHA1(83993ea90e7a602c3db137d08c008dcd9bee3055) )
	ROM_LOAD16_WORD_SWAP( "xmnj.04e", 0x080000, 0x80000, CRC(4a65833b) SHA1(cd899674ba6448fb3841247d3f434e82b19c5399) )
	ROM_LOAD16_WORD_SWAP( "xmn.05a",  0x100000, 0x80000, CRC(ac0d7759) SHA1(650d4474b13f16af7910a0f721fcda2ddb2414fd) )
	ROM_LOAD16_WORD_SWAP( "xmn.06a",  0x180000, 0x80000, CRC(1b86a328) SHA1(2469cd705139ee9f1142e6e379e68d0c9675b37e) )
	ROM_LOAD16_WORD_SWAP( "xmn.07a",  0x200000, 0x80000, CRC(2c142a44) SHA1(7624875f9c39b361fc83e52e87e0fd5e96279713) )
	ROM_LOAD16_WORD_SWAP( "xmn.08a",  0x280000, 0x80000, CRC(f712d44f) SHA1(0d18d4a4eacad94a66beca6ec509ac7f690c6882) )
	ROM_LOAD16_WORD_SWAP( "xmn.09a",  0x300000, 0x80000, CRC(9241cae8) SHA1(bb6980abf25aaf3eb14e230ca6942f3e2ab2c660) )
	ROM_LOAD16_WORD_SWAP( "xmn.10a",  0x380000, 0x80000, CRC(53c0eab9) SHA1(e3b1ec1fd517735f7801cfebb257c43185c6d3fb) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "xmn.13m",   0x0000000, 0x400000, CRC(bf4df073) SHA1(4d2740c3a827f0ec2cf75ad99c65e393c6a11c23) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.15m",   0x0000002, 0x400000, CRC(4d7e4cef) SHA1(50b8797b8099a8d76ad063ba1201a13dbb88ae3a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.17m",   0x0000004, 0x400000, CRC(513eea17) SHA1(a497477ad9ac13180911d8745ef6ee1955c0b877) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.19m",   0x0000006, 0x400000, CRC(d23897fc) SHA1(1e31627999736652252164d32662779a1ac6ca29) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.14m",   0x1000000, 0x400000, CRC(778237b7) SHA1(89a759ec383518ec52f5059d10ec342f2247aa20) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.16m",   0x1000002, 0x400000, CRC(67b36948) SHA1(692fb6e4096b880aa22996d554b160f664bbd907) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.18m",   0x1000004, 0x400000, CRC(015a7c4c) SHA1(cccc95dafd076a1a9fa004710006149c42d058ba) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.20m",   0x1000006, 0x400000, CRC(9dde2758) SHA1(17ba259cad03c7b5d56c0a5eda9ab53521665729) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "xmn.01a",  0x00000, 0x08000, CRC(40f479ea) SHA1(f29e15f537675305264ae2138a0a537fb9e2008b) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "xmn.02a",  0x28000, 0x20000, CRC(39d9b5ad) SHA1(af502debfd36100d4fc971ed25fdf9d7121d6f18) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "xmn.11m",   0x000000, 0x200000, CRC(c848a6bc) SHA1(ac8ac564d3c43225822f8bc330eba9f35b24b0a4) )
	ROM_LOAD16_WORD_SWAP( "xmn.12m",   0x200000, 0x200000, CRC(729c188f) SHA1(3279774ad8aebbcf0fc779cdfcbe21044dd192ad) )
ROM_END

ROM_START( xmcotaj1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "xmnj.03d", 0x000000, 0x80000, CRC(79086d62) SHA1(d99a48bd40593aa9c1ff1fcbfc40cfe3bc882fc5) )
	ROM_LOAD16_WORD_SWAP( "xmnj.04d", 0x080000, 0x80000, CRC(38eed613) SHA1(86ecb58bf03adda705a39a9779862fe2c85cadad) )
	ROM_LOAD16_WORD_SWAP( "xmn.05",   0x100000, 0x80000, CRC(c3ed62a2) SHA1(4e3317d7ca981e33318822103a16e59f4ce20deb) )
	ROM_LOAD16_WORD_SWAP( "xmn.06",   0x180000, 0x80000, CRC(f03c52e1) SHA1(904b2312ee594f5ece0484cad0eed25cc758185e) )
	ROM_LOAD16_WORD_SWAP( "xmn.07",   0x200000, 0x80000, CRC(325626b1) SHA1(3f3a0aabbe5ffad8136ac91e0de785103b16059b) )
	ROM_LOAD16_WORD_SWAP( "xmn.08",   0x280000, 0x80000, CRC(7194ea10) SHA1(40a5892d816f24cbfd4c310792eeabf689c6fa7e) )
	ROM_LOAD16_WORD_SWAP( "xmn.09",   0x300000, 0x80000, CRC(ae946df3) SHA1(733671f76d766bda7110df9d338791cc5202b050) )
	ROM_LOAD16_WORD_SWAP( "xmn.10",   0x380000, 0x80000, CRC(32a6be1d) SHA1(8f5fcb33b528abed670b4fc3fa62431a6e033c56) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "xmn.13m",   0x0000000, 0x400000, CRC(bf4df073) SHA1(4d2740c3a827f0ec2cf75ad99c65e393c6a11c23) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.15m",   0x0000002, 0x400000, CRC(4d7e4cef) SHA1(50b8797b8099a8d76ad063ba1201a13dbb88ae3a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.17m",   0x0000004, 0x400000, CRC(513eea17) SHA1(a497477ad9ac13180911d8745ef6ee1955c0b877) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.19m",   0x0000006, 0x400000, CRC(d23897fc) SHA1(1e31627999736652252164d32662779a1ac6ca29) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.14m",   0x1000000, 0x400000, CRC(778237b7) SHA1(89a759ec383518ec52f5059d10ec342f2247aa20) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.16m",   0x1000002, 0x400000, CRC(67b36948) SHA1(692fb6e4096b880aa22996d554b160f664bbd907) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.18m",   0x1000004, 0x400000, CRC(015a7c4c) SHA1(cccc95dafd076a1a9fa004710006149c42d058ba) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.20m",   0x1000006, 0x400000, CRC(9dde2758) SHA1(17ba259cad03c7b5d56c0a5eda9ab53521665729) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "xmn.01a",  0x00000, 0x08000, CRC(40f479ea) SHA1(f29e15f537675305264ae2138a0a537fb9e2008b) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "xmn.02a",  0x28000, 0x20000, CRC(39d9b5ad) SHA1(af502debfd36100d4fc971ed25fdf9d7121d6f18) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "xmn.11m",   0x000000, 0x200000, CRC(c848a6bc) SHA1(ac8ac564d3c43225822f8bc330eba9f35b24b0a4) )
	ROM_LOAD16_WORD_SWAP( "xmn.12m",   0x200000, 0x200000, CRC(729c188f) SHA1(3279774ad8aebbcf0fc779cdfcbe21044dd192ad) )
ROM_END

ROM_START( xmcotaj2 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "xmnj.03b", 0x000000, 0x80000, CRC(c8175fb3) SHA1(ea25bd165f8794324a1e07719312798cf9742924) )
	ROM_LOAD16_WORD_SWAP( "xmnj.04b", 0x080000, 0x80000, CRC(54b3fba3) SHA1(47eaff5d36a45e4196f87ed3d02e54d5407e7962) )
	ROM_LOAD16_WORD_SWAP( "xmn.05",   0x100000, 0x80000, CRC(c3ed62a2) SHA1(4e3317d7ca981e33318822103a16e59f4ce20deb) )
	ROM_LOAD16_WORD_SWAP( "xmn.06",   0x180000, 0x80000, CRC(f03c52e1) SHA1(904b2312ee594f5ece0484cad0eed25cc758185e) )
	ROM_LOAD16_WORD_SWAP( "xmn.07",   0x200000, 0x80000, CRC(325626b1) SHA1(3f3a0aabbe5ffad8136ac91e0de785103b16059b) )
	ROM_LOAD16_WORD_SWAP( "xmn.08",   0x280000, 0x80000, CRC(7194ea10) SHA1(40a5892d816f24cbfd4c310792eeabf689c6fa7e) )
	ROM_LOAD16_WORD_SWAP( "xmn.09",   0x300000, 0x80000, CRC(ae946df3) SHA1(733671f76d766bda7110df9d338791cc5202b050) )
	ROM_LOAD16_WORD_SWAP( "xmn.10",   0x380000, 0x80000, CRC(32a6be1d) SHA1(8f5fcb33b528abed670b4fc3fa62431a6e033c56) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "xmn.13m",   0x0000000, 0x400000, CRC(bf4df073) SHA1(4d2740c3a827f0ec2cf75ad99c65e393c6a11c23) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.15m",   0x0000002, 0x400000, CRC(4d7e4cef) SHA1(50b8797b8099a8d76ad063ba1201a13dbb88ae3a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.17m",   0x0000004, 0x400000, CRC(513eea17) SHA1(a497477ad9ac13180911d8745ef6ee1955c0b877) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.19m",   0x0000006, 0x400000, CRC(d23897fc) SHA1(1e31627999736652252164d32662779a1ac6ca29) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.14m",   0x1000000, 0x400000, CRC(778237b7) SHA1(89a759ec383518ec52f5059d10ec342f2247aa20) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.16m",   0x1000002, 0x400000, CRC(67b36948) SHA1(692fb6e4096b880aa22996d554b160f664bbd907) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.18m",   0x1000004, 0x400000, CRC(015a7c4c) SHA1(cccc95dafd076a1a9fa004710006149c42d058ba) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.20m",   0x1000006, 0x400000, CRC(9dde2758) SHA1(17ba259cad03c7b5d56c0a5eda9ab53521665729) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "xmn.01a",  0x00000, 0x08000, CRC(40f479ea) SHA1(f29e15f537675305264ae2138a0a537fb9e2008b) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "xmn.02a",  0x28000, 0x20000, CRC(39d9b5ad) SHA1(af502debfd36100d4fc971ed25fdf9d7121d6f18) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "xmn.11m",   0x000000, 0x200000, CRC(c848a6bc) SHA1(ac8ac564d3c43225822f8bc330eba9f35b24b0a4) )
	ROM_LOAD16_WORD_SWAP( "xmn.12m",   0x200000, 0x200000, CRC(729c188f) SHA1(3279774ad8aebbcf0fc779cdfcbe21044dd192ad) )
ROM_END

ROM_START( xmcotaj3 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "xmnj.03a", 0x000000, 0x80000, CRC(00761611) SHA1(e780dbe1d21a0d5b6981f0395942c9fa59688113) )
	ROM_LOAD16_WORD_SWAP( "xmnj.04a", 0x080000, 0x80000, CRC(614d3f60) SHA1(2272ae243557562a0bc85d2cd2b37dd876f6902c) )
	ROM_LOAD16_WORD_SWAP( "xmn.05",   0x100000, 0x80000, CRC(c3ed62a2) SHA1(4e3317d7ca981e33318822103a16e59f4ce20deb) )
	ROM_LOAD16_WORD_SWAP( "xmn.06",   0x180000, 0x80000, CRC(f03c52e1) SHA1(904b2312ee594f5ece0484cad0eed25cc758185e) )
	ROM_LOAD16_WORD_SWAP( "xmn.07",   0x200000, 0x80000, CRC(325626b1) SHA1(3f3a0aabbe5ffad8136ac91e0de785103b16059b) )
	ROM_LOAD16_WORD_SWAP( "xmn.08",   0x280000, 0x80000, CRC(7194ea10) SHA1(40a5892d816f24cbfd4c310792eeabf689c6fa7e) )
	ROM_LOAD16_WORD_SWAP( "xmn.09",   0x300000, 0x80000, CRC(ae946df3) SHA1(733671f76d766bda7110df9d338791cc5202b050) )
	ROM_LOAD16_WORD_SWAP( "xmn.10",   0x380000, 0x80000, CRC(32a6be1d) SHA1(8f5fcb33b528abed670b4fc3fa62431a6e033c56) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "xmn.13m",   0x0000000, 0x400000, CRC(bf4df073) SHA1(4d2740c3a827f0ec2cf75ad99c65e393c6a11c23) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.15m",   0x0000002, 0x400000, CRC(4d7e4cef) SHA1(50b8797b8099a8d76ad063ba1201a13dbb88ae3a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.17m",   0x0000004, 0x400000, CRC(513eea17) SHA1(a497477ad9ac13180911d8745ef6ee1955c0b877) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.19m",   0x0000006, 0x400000, CRC(d23897fc) SHA1(1e31627999736652252164d32662779a1ac6ca29) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.14m",   0x1000000, 0x400000, CRC(778237b7) SHA1(89a759ec383518ec52f5059d10ec342f2247aa20) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.16m",   0x1000002, 0x400000, CRC(67b36948) SHA1(692fb6e4096b880aa22996d554b160f664bbd907) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.18m",   0x1000004, 0x400000, CRC(015a7c4c) SHA1(cccc95dafd076a1a9fa004710006149c42d058ba) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.20m",   0x1000006, 0x400000, CRC(9dde2758) SHA1(17ba259cad03c7b5d56c0a5eda9ab53521665729) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "xmn.01a",  0x00000, 0x08000, CRC(40f479ea) SHA1(f29e15f537675305264ae2138a0a537fb9e2008b) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "xmn.02a",  0x28000, 0x20000, CRC(39d9b5ad) SHA1(af502debfd36100d4fc971ed25fdf9d7121d6f18) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "xmn.11m",   0x000000, 0x200000, CRC(c848a6bc) SHA1(ac8ac564d3c43225822f8bc330eba9f35b24b0a4) )
	ROM_LOAD16_WORD_SWAP( "xmn.12m",   0x200000, 0x200000, CRC(729c188f) SHA1(3279774ad8aebbcf0fc779cdfcbe21044dd192ad) )
ROM_END

ROM_START( xmcotajr )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "xmno.03a", 0x000000, 0x80000, CRC(7ab19acf) SHA1(ca02e58f1d713ee74c6c1515772da0ca26f9deb9) )
	ROM_LOAD16_WORD_SWAP( "xmno.04a", 0x080000, 0x80000, CRC(7615dd21) SHA1(f393c985ae1c7f378f9184fd2c8530b7494ba06d) )
	ROM_LOAD16_WORD_SWAP( "xmno.05a", 0x100000, 0x80000, CRC(0303d672) SHA1(4816b5ac6a9bf78665112d54a8f3569d590721b2) )
	ROM_LOAD16_WORD_SWAP( "xmno.06a", 0x180000, 0x80000, CRC(332839a5) SHA1(c7b80fad1130cc025de3fad372b727d360adc47b) )
	ROM_LOAD16_WORD_SWAP( "xmno.07",  0x200000, 0x80000, CRC(6255e8d5) SHA1(159f7983b93ee82c2012a3a6a9f451a521f98ed6) )
	ROM_LOAD16_WORD_SWAP( "xmno.08",  0x280000, 0x80000, CRC(b8ebe77c) SHA1(3ef06f19f2ba0aee8be9d9a9f0b1742f9ee1282a) )
	ROM_LOAD16_WORD_SWAP( "xmno.09",  0x300000, 0x80000, CRC(5440d950) SHA1(d5338718964b6f36655ddd62dbbe2bbfb44db114) )
	ROM_LOAD16_WORD_SWAP( "xmno.10a", 0x380000, 0x80000, CRC(b8296966) SHA1(b13496956d8288302bf5c9a7478d4791e41e1bfd) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "xmn.13m",   0x0000000, 0x400000, CRC(bf4df073) SHA1(4d2740c3a827f0ec2cf75ad99c65e393c6a11c23) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.15m",   0x0000002, 0x400000, CRC(4d7e4cef) SHA1(50b8797b8099a8d76ad063ba1201a13dbb88ae3a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.17m",   0x0000004, 0x400000, CRC(513eea17) SHA1(a497477ad9ac13180911d8745ef6ee1955c0b877) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.19m",   0x0000006, 0x400000, CRC(d23897fc) SHA1(1e31627999736652252164d32662779a1ac6ca29) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.14m",   0x1000000, 0x400000, CRC(778237b7) SHA1(89a759ec383518ec52f5059d10ec342f2247aa20) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.16m",   0x1000002, 0x400000, CRC(67b36948) SHA1(692fb6e4096b880aa22996d554b160f664bbd907) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.18m",   0x1000004, 0x400000, CRC(015a7c4c) SHA1(cccc95dafd076a1a9fa004710006149c42d058ba) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.20m",   0x1000006, 0x400000, CRC(9dde2758) SHA1(17ba259cad03c7b5d56c0a5eda9ab53521665729) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "xmn.01",   0x00000, 0x08000, CRC(7178336e) SHA1(d94cddcc144336fa3ee2778b3531badcc4646e9d) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "xmn.02",   0x28000, 0x20000, CRC(0ec58501) SHA1(3af500049f901897086bd35b83ca83f4bbc8b3f6) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "xmn.11m",   0x000000, 0x200000, CRC(c848a6bc) SHA1(ac8ac564d3c43225822f8bc330eba9f35b24b0a4) )
	ROM_LOAD16_WORD_SWAP( "xmn.12m",   0x200000, 0x200000, CRC(729c188f) SHA1(3279774ad8aebbcf0fc779cdfcbe21044dd192ad) )
ROM_END

ROM_START( xmcotaa )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "xmna.03a", 0x000000, 0x80000, CRC(7df8b27e) SHA1(2f0ce6b10857e04ddaf7a76edf126282c53511b3) )
	ROM_LOAD16_WORD_SWAP( "xmna.04a", 0x080000, 0x80000, CRC(b44e30a7) SHA1(27b0a8b06aa11673dd145717c6286eb27186cf79) )
	ROM_LOAD16_WORD_SWAP( "xmn.05",   0x100000, 0x80000, CRC(c3ed62a2) SHA1(4e3317d7ca981e33318822103a16e59f4ce20deb) )
	ROM_LOAD16_WORD_SWAP( "xmn.06",   0x180000, 0x80000, CRC(f03c52e1) SHA1(904b2312ee594f5ece0484cad0eed25cc758185e) )
	ROM_LOAD16_WORD_SWAP( "xmn.07",   0x200000, 0x80000, CRC(325626b1) SHA1(3f3a0aabbe5ffad8136ac91e0de785103b16059b) )
	ROM_LOAD16_WORD_SWAP( "xmn.08",   0x280000, 0x80000, CRC(7194ea10) SHA1(40a5892d816f24cbfd4c310792eeabf689c6fa7e) )
	ROM_LOAD16_WORD_SWAP( "xmn.09",   0x300000, 0x80000, CRC(ae946df3) SHA1(733671f76d766bda7110df9d338791cc5202b050) )
	ROM_LOAD16_WORD_SWAP( "xmn.10",   0x380000, 0x80000, CRC(32a6be1d) SHA1(8f5fcb33b528abed670b4fc3fa62431a6e033c56) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "xmn.13m",   0x0000000, 0x400000, CRC(bf4df073) SHA1(4d2740c3a827f0ec2cf75ad99c65e393c6a11c23) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.15m",   0x0000002, 0x400000, CRC(4d7e4cef) SHA1(50b8797b8099a8d76ad063ba1201a13dbb88ae3a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.17m",   0x0000004, 0x400000, CRC(513eea17) SHA1(a497477ad9ac13180911d8745ef6ee1955c0b877) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.19m",   0x0000006, 0x400000, CRC(d23897fc) SHA1(1e31627999736652252164d32662779a1ac6ca29) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.14m",   0x1000000, 0x400000, CRC(778237b7) SHA1(89a759ec383518ec52f5059d10ec342f2247aa20) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.16m",   0x1000002, 0x400000, CRC(67b36948) SHA1(692fb6e4096b880aa22996d554b160f664bbd907) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.18m",   0x1000004, 0x400000, CRC(015a7c4c) SHA1(cccc95dafd076a1a9fa004710006149c42d058ba) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.20m",   0x1000006, 0x400000, CRC(9dde2758) SHA1(17ba259cad03c7b5d56c0a5eda9ab53521665729) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "xmn.01a",  0x00000, 0x08000, CRC(40f479ea) SHA1(f29e15f537675305264ae2138a0a537fb9e2008b) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "xmn.02a",  0x28000, 0x20000, CRC(39d9b5ad) SHA1(af502debfd36100d4fc971ed25fdf9d7121d6f18) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "xmn.11m",   0x000000, 0x200000, CRC(c848a6bc) SHA1(ac8ac564d3c43225822f8bc330eba9f35b24b0a4) )
	ROM_LOAD16_WORD_SWAP( "xmn.12m",   0x200000, 0x200000, CRC(729c188f) SHA1(3279774ad8aebbcf0fc779cdfcbe21044dd192ad) )
ROM_END

ROM_START( xmvsf )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "xvse.03f", 0x000000, 0x80000, CRC(db06413f) SHA1(c6d8aa1e43fc541e5b4e938258f27ab9ee30ca33) )
	ROM_LOAD16_WORD_SWAP( "xvse.04f", 0x080000, 0x80000, CRC(ef015aef) SHA1(d3504cb8c38f720b1f4528157266db60c8c6c075) )
	ROM_LOAD16_WORD_SWAP( "xvs.05a",  0x100000, 0x80000, CRC(7db6025d) SHA1(2d74f48f83f45359bfaca28ab686625766af12ee) )
	ROM_LOAD16_WORD_SWAP( "xvs.06a",  0x180000, 0x80000, CRC(e8e2c75c) SHA1(929408cb5d98e95cec75ea58e4701b0cbdbcd016) )
	ROM_LOAD16_WORD_SWAP( "xvs.07",   0x200000, 0x80000, CRC(08f0abed) SHA1(ef16c376232dba63b0b9bc3aa0640f9001ccb68a) )
	ROM_LOAD16_WORD_SWAP( "xvs.08",   0x280000, 0x80000, CRC(81929675) SHA1(19cf7afbc1daaefec40195e40ba74970f3906a1c) )
	ROM_LOAD16_WORD_SWAP( "xvs.09",   0x300000, 0x80000, CRC(9641f36b) SHA1(dcba3482d1ba37ccfb30d402793ee063c6621aed) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "xvs.13m",   0x0000000, 0x400000, CRC(f6684efd) SHA1(c0a2f3a9e82ab8b084a500aec71ac633e947328c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.15m",   0x0000002, 0x400000, CRC(29109221) SHA1(898b8f678fd03c462ce0d8eb7fb3441ef601085b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.17m",   0x0000004, 0x400000, CRC(92db3474) SHA1(7b6f4c8ebfdac167b25f35029068b6253c141fe6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.19m",   0x0000006, 0x400000, CRC(3733473c) SHA1(6579da7145c95b3ad00844a5fc8c2e22c23365e2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.14m",   0x1000000, 0x400000, CRC(bcac2e41) SHA1(838ff24f7e8543a787a55a5d592c9517ce3b8b93) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.16m",   0x1000002, 0x400000, CRC(ea04a272) SHA1(cd7c79037b5b4a39bef5156433e984dc4dc2c081) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.18m",   0x1000004, 0x400000, CRC(b0def86a) SHA1(da3a6705ea7050fc5c2c10d33400ed67be9f455d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.20m",   0x1000006, 0x400000, CRC(4b40ff9f) SHA1(9a981d442132efff09a27408d74646ba357c7357) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "xvs.01",   0x00000, 0x08000, CRC(3999e93a) SHA1(fefcff8a9a5c83df7655a16187cf9ba3e7efbb25) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "xvs.02",   0x28000, 0x20000, CRC(101bdee9) SHA1(75920e88bf46fcd33a7957777a1d799818ffb0d6) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "xvs.11m",   0x000000, 0x200000, CRC(9cadcdbc) SHA1(64d3bd53b04daec84c9af4aa3ff010867b3d306d) )
	ROM_LOAD16_WORD_SWAP( "xvs.12m",   0x200000, 0x200000, CRC(7b11e460) SHA1(a581c84acaaf0ce056841c15a6f36889e88be68d) )
ROM_END

ROM_START( xmvsfr1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "xvse.03d", 0x000000, 0x80000, CRC(5ae5bd3b) SHA1(f687f018008cef24f86f53373c3f5547741a4c5b) )
	ROM_LOAD16_WORD_SWAP( "xvse.04d", 0x080000, 0x80000, CRC(5eb9c02e) SHA1(25a392913213b98ce1bbd463bf5e5e10729bde0c) )
	ROM_LOAD16_WORD_SWAP( "xvs.05a",  0x100000, 0x80000, CRC(7db6025d) SHA1(2d74f48f83f45359bfaca28ab686625766af12ee) )
	ROM_LOAD16_WORD_SWAP( "xvs.06a",  0x180000, 0x80000, CRC(e8e2c75c) SHA1(929408cb5d98e95cec75ea58e4701b0cbdbcd016) )
	ROM_LOAD16_WORD_SWAP( "xvs.07",   0x200000, 0x80000, CRC(08f0abed) SHA1(ef16c376232dba63b0b9bc3aa0640f9001ccb68a) )
	ROM_LOAD16_WORD_SWAP( "xvs.08",   0x280000, 0x80000, CRC(81929675) SHA1(19cf7afbc1daaefec40195e40ba74970f3906a1c) )
	ROM_LOAD16_WORD_SWAP( "xvs.09",   0x300000, 0x80000, CRC(9641f36b) SHA1(dcba3482d1ba37ccfb30d402793ee063c6621aed) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "xvs.13m",   0x0000000, 0x400000, CRC(f6684efd) SHA1(c0a2f3a9e82ab8b084a500aec71ac633e947328c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.15m",   0x0000002, 0x400000, CRC(29109221) SHA1(898b8f678fd03c462ce0d8eb7fb3441ef601085b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.17m",   0x0000004, 0x400000, CRC(92db3474) SHA1(7b6f4c8ebfdac167b25f35029068b6253c141fe6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.19m",   0x0000006, 0x400000, CRC(3733473c) SHA1(6579da7145c95b3ad00844a5fc8c2e22c23365e2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.14m",   0x1000000, 0x400000, CRC(bcac2e41) SHA1(838ff24f7e8543a787a55a5d592c9517ce3b8b93) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.16m",   0x1000002, 0x400000, CRC(ea04a272) SHA1(cd7c79037b5b4a39bef5156433e984dc4dc2c081) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.18m",   0x1000004, 0x400000, CRC(b0def86a) SHA1(da3a6705ea7050fc5c2c10d33400ed67be9f455d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.20m",   0x1000006, 0x400000, CRC(4b40ff9f) SHA1(9a981d442132efff09a27408d74646ba357c7357) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "xvs.01",   0x00000, 0x08000, CRC(3999e93a) SHA1(fefcff8a9a5c83df7655a16187cf9ba3e7efbb25) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "xvs.02",   0x28000, 0x20000, CRC(101bdee9) SHA1(75920e88bf46fcd33a7957777a1d799818ffb0d6) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "xvs.11m",   0x000000, 0x200000, CRC(9cadcdbc) SHA1(64d3bd53b04daec84c9af4aa3ff010867b3d306d) )
	ROM_LOAD16_WORD_SWAP( "xvs.12m",   0x200000, 0x200000, CRC(7b11e460) SHA1(a581c84acaaf0ce056841c15a6f36889e88be68d) )
ROM_END

ROM_START( xmvsfu )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "xvsu.03k", 0x000000, 0x80000, CRC(8739ef61) SHA1(2eb5912d3026bed0f720d28e1bf3a7ceb5b80803) )
	ROM_LOAD16_WORD_SWAP( "xvsu.04k", 0x080000, 0x80000, CRC(e11d35c1) SHA1(d838199b2767d9f02fa0f103c5d587a4c78c0d21) )
	ROM_LOAD16_WORD_SWAP( "xvs.05a",  0x100000, 0x80000, CRC(7db6025d) SHA1(2d74f48f83f45359bfaca28ab686625766af12ee) )
	ROM_LOAD16_WORD_SWAP( "xvs.06a",  0x180000, 0x80000, CRC(e8e2c75c) SHA1(929408cb5d98e95cec75ea58e4701b0cbdbcd016) )
	ROM_LOAD16_WORD_SWAP( "xvs.07",   0x200000, 0x80000, CRC(08f0abed) SHA1(ef16c376232dba63b0b9bc3aa0640f9001ccb68a) )
	ROM_LOAD16_WORD_SWAP( "xvs.08",   0x280000, 0x80000, CRC(81929675) SHA1(19cf7afbc1daaefec40195e40ba74970f3906a1c) )
	ROM_LOAD16_WORD_SWAP( "xvs.09",   0x300000, 0x80000, CRC(9641f36b) SHA1(dcba3482d1ba37ccfb30d402793ee063c6621aed) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "xvs.13m",   0x0000000, 0x400000, CRC(f6684efd) SHA1(c0a2f3a9e82ab8b084a500aec71ac633e947328c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.15m",   0x0000002, 0x400000, CRC(29109221) SHA1(898b8f678fd03c462ce0d8eb7fb3441ef601085b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.17m",   0x0000004, 0x400000, CRC(92db3474) SHA1(7b6f4c8ebfdac167b25f35029068b6253c141fe6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.19m",   0x0000006, 0x400000, CRC(3733473c) SHA1(6579da7145c95b3ad00844a5fc8c2e22c23365e2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.14m",   0x1000000, 0x400000, CRC(bcac2e41) SHA1(838ff24f7e8543a787a55a5d592c9517ce3b8b93) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.16m",   0x1000002, 0x400000, CRC(ea04a272) SHA1(cd7c79037b5b4a39bef5156433e984dc4dc2c081) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.18m",   0x1000004, 0x400000, CRC(b0def86a) SHA1(da3a6705ea7050fc5c2c10d33400ed67be9f455d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.20m",   0x1000006, 0x400000, CRC(4b40ff9f) SHA1(9a981d442132efff09a27408d74646ba357c7357) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "xvs.01",   0x00000, 0x08000, CRC(3999e93a) SHA1(fefcff8a9a5c83df7655a16187cf9ba3e7efbb25) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "xvs.02",   0x28000, 0x20000, CRC(101bdee9) SHA1(75920e88bf46fcd33a7957777a1d799818ffb0d6) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "xvs.11m",   0x000000, 0x200000, CRC(9cadcdbc) SHA1(64d3bd53b04daec84c9af4aa3ff010867b3d306d) )
	ROM_LOAD16_WORD_SWAP( "xvs.12m",   0x200000, 0x200000, CRC(7b11e460) SHA1(a581c84acaaf0ce056841c15a6f36889e88be68d) )
ROM_END

ROM_START( xmvsfur1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "xvsu.03h", 0x000000, 0x80000, CRC(5481155a) SHA1(799a2488684cbead33206498d13261b79624a46e) )
	ROM_LOAD16_WORD_SWAP( "xvsu.04h", 0x080000, 0x80000, CRC(1e236388) SHA1(329c08103840fadbc4176785c4b24013a7a2b1bc) )
	ROM_LOAD16_WORD_SWAP( "xvs.05a",  0x100000, 0x80000, CRC(7db6025d) SHA1(2d74f48f83f45359bfaca28ab686625766af12ee) )
	ROM_LOAD16_WORD_SWAP( "xvs.06a",  0x180000, 0x80000, CRC(e8e2c75c) SHA1(929408cb5d98e95cec75ea58e4701b0cbdbcd016) )
	ROM_LOAD16_WORD_SWAP( "xvs.07",   0x200000, 0x80000, CRC(08f0abed) SHA1(ef16c376232dba63b0b9bc3aa0640f9001ccb68a) )
	ROM_LOAD16_WORD_SWAP( "xvs.08",   0x280000, 0x80000, CRC(81929675) SHA1(19cf7afbc1daaefec40195e40ba74970f3906a1c) )
	ROM_LOAD16_WORD_SWAP( "xvs.09",   0x300000, 0x80000, CRC(9641f36b) SHA1(dcba3482d1ba37ccfb30d402793ee063c6621aed) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "xvs.13m",   0x0000000, 0x400000, CRC(f6684efd) SHA1(c0a2f3a9e82ab8b084a500aec71ac633e947328c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.15m",   0x0000002, 0x400000, CRC(29109221) SHA1(898b8f678fd03c462ce0d8eb7fb3441ef601085b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.17m",   0x0000004, 0x400000, CRC(92db3474) SHA1(7b6f4c8ebfdac167b25f35029068b6253c141fe6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.19m",   0x0000006, 0x400000, CRC(3733473c) SHA1(6579da7145c95b3ad00844a5fc8c2e22c23365e2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.14m",   0x1000000, 0x400000, CRC(bcac2e41) SHA1(838ff24f7e8543a787a55a5d592c9517ce3b8b93) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.16m",   0x1000002, 0x400000, CRC(ea04a272) SHA1(cd7c79037b5b4a39bef5156433e984dc4dc2c081) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.18m",   0x1000004, 0x400000, CRC(b0def86a) SHA1(da3a6705ea7050fc5c2c10d33400ed67be9f455d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.20m",   0x1000006, 0x400000, CRC(4b40ff9f) SHA1(9a981d442132efff09a27408d74646ba357c7357) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "xvs.01",   0x00000, 0x08000, CRC(3999e93a) SHA1(fefcff8a9a5c83df7655a16187cf9ba3e7efbb25) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "xvs.02",   0x28000, 0x20000, CRC(101bdee9) SHA1(75920e88bf46fcd33a7957777a1d799818ffb0d6) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "xvs.11m",   0x000000, 0x200000, CRC(9cadcdbc) SHA1(64d3bd53b04daec84c9af4aa3ff010867b3d306d) )
	ROM_LOAD16_WORD_SWAP( "xvs.12m",   0x200000, 0x200000, CRC(7b11e460) SHA1(a581c84acaaf0ce056841c15a6f36889e88be68d) )
ROM_END

ROM_START( xmvsfj )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "xvsj.03i", 0x000000, 0x80000, CRC(ef24da96) SHA1(8f4a2a626a059bcf36048770153a9ffc85bba304) )
	ROM_LOAD16_WORD_SWAP( "xvsj.04i", 0x080000, 0x80000, CRC(70a59b35) SHA1(786d9b243373024735848f785503c6aa883b1c2f) )
	ROM_LOAD16_WORD_SWAP( "xvs.05a",  0x100000, 0x80000, CRC(7db6025d) SHA1(2d74f48f83f45359bfaca28ab686625766af12ee) )
	ROM_LOAD16_WORD_SWAP( "xvs.06a",  0x180000, 0x80000, CRC(e8e2c75c) SHA1(929408cb5d98e95cec75ea58e4701b0cbdbcd016) )
	ROM_LOAD16_WORD_SWAP( "xvs.07",   0x200000, 0x80000, CRC(08f0abed) SHA1(ef16c376232dba63b0b9bc3aa0640f9001ccb68a) )
	ROM_LOAD16_WORD_SWAP( "xvs.08",   0x280000, 0x80000, CRC(81929675) SHA1(19cf7afbc1daaefec40195e40ba74970f3906a1c) )
	ROM_LOAD16_WORD_SWAP( "xvs.09",   0x300000, 0x80000, CRC(9641f36b) SHA1(dcba3482d1ba37ccfb30d402793ee063c6621aed) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "xvs.13m",   0x0000000, 0x400000, CRC(f6684efd) SHA1(c0a2f3a9e82ab8b084a500aec71ac633e947328c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.15m",   0x0000002, 0x400000, CRC(29109221) SHA1(898b8f678fd03c462ce0d8eb7fb3441ef601085b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.17m",   0x0000004, 0x400000, CRC(92db3474) SHA1(7b6f4c8ebfdac167b25f35029068b6253c141fe6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.19m",   0x0000006, 0x400000, CRC(3733473c) SHA1(6579da7145c95b3ad00844a5fc8c2e22c23365e2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.14m",   0x1000000, 0x400000, CRC(bcac2e41) SHA1(838ff24f7e8543a787a55a5d592c9517ce3b8b93) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.16m",   0x1000002, 0x400000, CRC(ea04a272) SHA1(cd7c79037b5b4a39bef5156433e984dc4dc2c081) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.18m",   0x1000004, 0x400000, CRC(b0def86a) SHA1(da3a6705ea7050fc5c2c10d33400ed67be9f455d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.20m",   0x1000006, 0x400000, CRC(4b40ff9f) SHA1(9a981d442132efff09a27408d74646ba357c7357) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "xvs.01",   0x00000, 0x08000, CRC(3999e93a) SHA1(fefcff8a9a5c83df7655a16187cf9ba3e7efbb25) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "xvs.02",   0x28000, 0x20000, CRC(101bdee9) SHA1(75920e88bf46fcd33a7957777a1d799818ffb0d6) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "xvs.11m",   0x000000, 0x200000, CRC(9cadcdbc) SHA1(64d3bd53b04daec84c9af4aa3ff010867b3d306d) )
	ROM_LOAD16_WORD_SWAP( "xvs.12m",   0x200000, 0x200000, CRC(7b11e460) SHA1(a581c84acaaf0ce056841c15a6f36889e88be68d) )
ROM_END

ROM_START( xmvsfjr1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "xvsj.03d", 0x000000, 0x80000, CRC(beb81de9) SHA1(fce0d43b193a521d026be6508a91be6e2d03f480) )
	ROM_LOAD16_WORD_SWAP( "xvsj.04d", 0x080000, 0x80000, CRC(23d11271) SHA1(45e4ac52001f0c2b6cd6e07413b5e503c2b90329) )
	ROM_LOAD16_WORD_SWAP( "xvs.05a",  0x100000, 0x80000, CRC(7db6025d) SHA1(2d74f48f83f45359bfaca28ab686625766af12ee) )
	ROM_LOAD16_WORD_SWAP( "xvs.06a",  0x180000, 0x80000, CRC(e8e2c75c) SHA1(929408cb5d98e95cec75ea58e4701b0cbdbcd016) )
	ROM_LOAD16_WORD_SWAP( "xvs.07",   0x200000, 0x80000, CRC(08f0abed) SHA1(ef16c376232dba63b0b9bc3aa0640f9001ccb68a) )
	ROM_LOAD16_WORD_SWAP( "xvs.08",   0x280000, 0x80000, CRC(81929675) SHA1(19cf7afbc1daaefec40195e40ba74970f3906a1c) )
	ROM_LOAD16_WORD_SWAP( "xvs.09",   0x300000, 0x80000, CRC(9641f36b) SHA1(dcba3482d1ba37ccfb30d402793ee063c6621aed) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "xvs.13m",   0x0000000, 0x400000, CRC(f6684efd) SHA1(c0a2f3a9e82ab8b084a500aec71ac633e947328c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.15m",   0x0000002, 0x400000, CRC(29109221) SHA1(898b8f678fd03c462ce0d8eb7fb3441ef601085b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.17m",   0x0000004, 0x400000, CRC(92db3474) SHA1(7b6f4c8ebfdac167b25f35029068b6253c141fe6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.19m",   0x0000006, 0x400000, CRC(3733473c) SHA1(6579da7145c95b3ad00844a5fc8c2e22c23365e2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.14m",   0x1000000, 0x400000, CRC(bcac2e41) SHA1(838ff24f7e8543a787a55a5d592c9517ce3b8b93) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.16m",   0x1000002, 0x400000, CRC(ea04a272) SHA1(cd7c79037b5b4a39bef5156433e984dc4dc2c081) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.18m",   0x1000004, 0x400000, CRC(b0def86a) SHA1(da3a6705ea7050fc5c2c10d33400ed67be9f455d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.20m",   0x1000006, 0x400000, CRC(4b40ff9f) SHA1(9a981d442132efff09a27408d74646ba357c7357) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "xvs.01",   0x00000, 0x08000, CRC(3999e93a) SHA1(fefcff8a9a5c83df7655a16187cf9ba3e7efbb25) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "xvs.02",   0x28000, 0x20000, CRC(101bdee9) SHA1(75920e88bf46fcd33a7957777a1d799818ffb0d6) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "xvs.11m",   0x000000, 0x200000, CRC(9cadcdbc) SHA1(64d3bd53b04daec84c9af4aa3ff010867b3d306d) )
	ROM_LOAD16_WORD_SWAP( "xvs.12m",   0x200000, 0x200000, CRC(7b11e460) SHA1(a581c84acaaf0ce056841c15a6f36889e88be68d) )
ROM_END

ROM_START( xmvsfjr2 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "xvsj.03c", 0x000000, 0x80000, CRC(180656a1) SHA1(aec2dfcfe8bcab03a48f749977e6f08fc21558bc) )
	ROM_LOAD16_WORD_SWAP( "xvsj.04c", 0x080000, 0x80000, CRC(5832811c) SHA1(e900b343241310d4dd1b45f42573e1e90f2dcbda) )
	ROM_LOAD16_WORD_SWAP( "xvs.05",   0x100000, 0x80000, CRC(030e0e1e) SHA1(164e3023bb1965768448e1bf6c45ff9e0ac964c7) )
	ROM_LOAD16_WORD_SWAP( "xvs.06",   0x180000, 0x80000, CRC(5d04a8ff) SHA1(3b5a524f3f1c4b540c88275418bdaf50c7186713) )
	ROM_LOAD16_WORD_SWAP( "xvs.07",   0x200000, 0x80000, CRC(08f0abed) SHA1(ef16c376232dba63b0b9bc3aa0640f9001ccb68a) )
	ROM_LOAD16_WORD_SWAP( "xvs.08",   0x280000, 0x80000, CRC(81929675) SHA1(19cf7afbc1daaefec40195e40ba74970f3906a1c) )
	ROM_LOAD16_WORD_SWAP( "xvs.09",   0x300000, 0x80000, CRC(9641f36b) SHA1(dcba3482d1ba37ccfb30d402793ee063c6621aed) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "xvs.13m",   0x0000000, 0x400000, CRC(f6684efd) SHA1(c0a2f3a9e82ab8b084a500aec71ac633e947328c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.15m",   0x0000002, 0x400000, CRC(29109221) SHA1(898b8f678fd03c462ce0d8eb7fb3441ef601085b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.17m",   0x0000004, 0x400000, CRC(92db3474) SHA1(7b6f4c8ebfdac167b25f35029068b6253c141fe6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.19m",   0x0000006, 0x400000, CRC(3733473c) SHA1(6579da7145c95b3ad00844a5fc8c2e22c23365e2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.14m",   0x1000000, 0x400000, CRC(bcac2e41) SHA1(838ff24f7e8543a787a55a5d592c9517ce3b8b93) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.16m",   0x1000002, 0x400000, CRC(ea04a272) SHA1(cd7c79037b5b4a39bef5156433e984dc4dc2c081) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.18m",   0x1000004, 0x400000, CRC(b0def86a) SHA1(da3a6705ea7050fc5c2c10d33400ed67be9f455d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.20m",   0x1000006, 0x400000, CRC(4b40ff9f) SHA1(9a981d442132efff09a27408d74646ba357c7357) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "xvs.01",   0x00000, 0x08000, CRC(3999e93a) SHA1(fefcff8a9a5c83df7655a16187cf9ba3e7efbb25) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "xvs.02",   0x28000, 0x20000, CRC(101bdee9) SHA1(75920e88bf46fcd33a7957777a1d799818ffb0d6) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "xvs.11m",   0x000000, 0x200000, CRC(9cadcdbc) SHA1(64d3bd53b04daec84c9af4aa3ff010867b3d306d) )
	ROM_LOAD16_WORD_SWAP( "xvs.12m",   0x200000, 0x200000, CRC(7b11e460) SHA1(a581c84acaaf0ce056841c15a6f36889e88be68d) )
ROM_END

ROM_START( xmvsfa )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "xvsa.03k", 0x000000, 0x80000, CRC(d0cca7a8) SHA1(70e0dd0725a52208e9e71fed82fba1d851a6bb42) )
	ROM_LOAD16_WORD_SWAP( "xvsa.04k", 0x080000, 0x80000, CRC(8c8e76fd) SHA1(ac1c8200951131bea0bda417b6bc2f77130b5fdd) )
	ROM_LOAD16_WORD_SWAP( "xvs.05a",  0x100000, 0x80000, CRC(7db6025d) SHA1(2d74f48f83f45359bfaca28ab686625766af12ee) )
	ROM_LOAD16_WORD_SWAP( "xvs.06a",  0x180000, 0x80000, CRC(e8e2c75c) SHA1(929408cb5d98e95cec75ea58e4701b0cbdbcd016) )
	ROM_LOAD16_WORD_SWAP( "xvs.07",   0x200000, 0x80000, CRC(08f0abed) SHA1(ef16c376232dba63b0b9bc3aa0640f9001ccb68a) )
	ROM_LOAD16_WORD_SWAP( "xvs.08",   0x280000, 0x80000, CRC(81929675) SHA1(19cf7afbc1daaefec40195e40ba74970f3906a1c) )
	ROM_LOAD16_WORD_SWAP( "xvs.09",   0x300000, 0x80000, CRC(9641f36b) SHA1(dcba3482d1ba37ccfb30d402793ee063c6621aed) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "xvs.13m",   0x0000000, 0x400000, CRC(f6684efd) SHA1(c0a2f3a9e82ab8b084a500aec71ac633e947328c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.15m",   0x0000002, 0x400000, CRC(29109221) SHA1(898b8f678fd03c462ce0d8eb7fb3441ef601085b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.17m",   0x0000004, 0x400000, CRC(92db3474) SHA1(7b6f4c8ebfdac167b25f35029068b6253c141fe6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.19m",   0x0000006, 0x400000, CRC(3733473c) SHA1(6579da7145c95b3ad00844a5fc8c2e22c23365e2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.14m",   0x1000000, 0x400000, CRC(bcac2e41) SHA1(838ff24f7e8543a787a55a5d592c9517ce3b8b93) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.16m",   0x1000002, 0x400000, CRC(ea04a272) SHA1(cd7c79037b5b4a39bef5156433e984dc4dc2c081) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.18m",   0x1000004, 0x400000, CRC(b0def86a) SHA1(da3a6705ea7050fc5c2c10d33400ed67be9f455d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.20m",   0x1000006, 0x400000, CRC(4b40ff9f) SHA1(9a981d442132efff09a27408d74646ba357c7357) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "xvs.01",   0x00000, 0x08000, CRC(3999e93a) SHA1(fefcff8a9a5c83df7655a16187cf9ba3e7efbb25) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "xvs.02",   0x28000, 0x20000, CRC(101bdee9) SHA1(75920e88bf46fcd33a7957777a1d799818ffb0d6) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "xvs.11m",   0x000000, 0x200000, CRC(9cadcdbc) SHA1(64d3bd53b04daec84c9af4aa3ff010867b3d306d) )
	ROM_LOAD16_WORD_SWAP( "xvs.12m",   0x200000, 0x200000, CRC(7b11e460) SHA1(a581c84acaaf0ce056841c15a6f36889e88be68d) )
ROM_END

ROM_START( xmvsfar1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "xvsa.03e", 0x000000, 0x80000, CRC(9bdde21c) SHA1(56c295d9e908a1496d6a08ff1cd10c87de1d4ff5) )
	ROM_LOAD16_WORD_SWAP( "xvsa.04e", 0x080000, 0x80000, CRC(33300edf) SHA1(8ec4203c1bc23a6284c47ee873b151da9f072edc) )
	ROM_LOAD16_WORD_SWAP( "xvs.05a",  0x100000, 0x80000, CRC(7db6025d) SHA1(2d74f48f83f45359bfaca28ab686625766af12ee) )
	ROM_LOAD16_WORD_SWAP( "xvs.06a",  0x180000, 0x80000, CRC(e8e2c75c) SHA1(929408cb5d98e95cec75ea58e4701b0cbdbcd016) )
	ROM_LOAD16_WORD_SWAP( "xvs.07",   0x200000, 0x80000, CRC(08f0abed) SHA1(ef16c376232dba63b0b9bc3aa0640f9001ccb68a) )
	ROM_LOAD16_WORD_SWAP( "xvs.08",   0x280000, 0x80000, CRC(81929675) SHA1(19cf7afbc1daaefec40195e40ba74970f3906a1c) )
	ROM_LOAD16_WORD_SWAP( "xvs.09",   0x300000, 0x80000, CRC(9641f36b) SHA1(dcba3482d1ba37ccfb30d402793ee063c6621aed) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "xvs.13m",   0x0000000, 0x400000, CRC(f6684efd) SHA1(c0a2f3a9e82ab8b084a500aec71ac633e947328c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.15m",   0x0000002, 0x400000, CRC(29109221) SHA1(898b8f678fd03c462ce0d8eb7fb3441ef601085b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.17m",   0x0000004, 0x400000, CRC(92db3474) SHA1(7b6f4c8ebfdac167b25f35029068b6253c141fe6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.19m",   0x0000006, 0x400000, CRC(3733473c) SHA1(6579da7145c95b3ad00844a5fc8c2e22c23365e2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.14m",   0x1000000, 0x400000, CRC(bcac2e41) SHA1(838ff24f7e8543a787a55a5d592c9517ce3b8b93) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.16m",   0x1000002, 0x400000, CRC(ea04a272) SHA1(cd7c79037b5b4a39bef5156433e984dc4dc2c081) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.18m",   0x1000004, 0x400000, CRC(b0def86a) SHA1(da3a6705ea7050fc5c2c10d33400ed67be9f455d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.20m",   0x1000006, 0x400000, CRC(4b40ff9f) SHA1(9a981d442132efff09a27408d74646ba357c7357) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "xvs.01",   0x00000, 0x08000, CRC(3999e93a) SHA1(fefcff8a9a5c83df7655a16187cf9ba3e7efbb25) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "xvs.02",   0x28000, 0x20000, CRC(101bdee9) SHA1(75920e88bf46fcd33a7957777a1d799818ffb0d6) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "xvs.11m",   0x000000, 0x200000, CRC(9cadcdbc) SHA1(64d3bd53b04daec84c9af4aa3ff010867b3d306d) )
	ROM_LOAD16_WORD_SWAP( "xvs.12m",   0x200000, 0x200000, CRC(7b11e460) SHA1(a581c84acaaf0ce056841c15a6f36889e88be68d) )
ROM_END

ROM_START( xmvsfar2 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "xvsa.03d", 0x000000, 0x80000, CRC(2b164fd7) SHA1(90eefa309202978c914897f30b2e6caf23fcd9f3) ) /* Came in a Blue cart on Euro MB, maybe Rent??? */
	ROM_LOAD16_WORD_SWAP( "xvsa.04d", 0x080000, 0x80000, CRC(2d32f039) SHA1(515bdb1ad99106a5b8aa3a94be4fc4ceb31a6711) )
	ROM_LOAD16_WORD_SWAP( "xvs.05a",  0x100000, 0x80000, CRC(7db6025d) SHA1(2d74f48f83f45359bfaca28ab686625766af12ee) )
	ROM_LOAD16_WORD_SWAP( "xvs.06a",  0x180000, 0x80000, CRC(e8e2c75c) SHA1(929408cb5d98e95cec75ea58e4701b0cbdbcd016) )
	ROM_LOAD16_WORD_SWAP( "xvs.07",   0x200000, 0x80000, CRC(08f0abed) SHA1(ef16c376232dba63b0b9bc3aa0640f9001ccb68a) )
	ROM_LOAD16_WORD_SWAP( "xvs.08",   0x280000, 0x80000, CRC(81929675) SHA1(19cf7afbc1daaefec40195e40ba74970f3906a1c) )
	ROM_LOAD16_WORD_SWAP( "xvs.09",   0x300000, 0x80000, CRC(9641f36b) SHA1(dcba3482d1ba37ccfb30d402793ee063c6621aed) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "xvs.13m",   0x0000000, 0x400000, CRC(f6684efd) SHA1(c0a2f3a9e82ab8b084a500aec71ac633e947328c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.15m",   0x0000002, 0x400000, CRC(29109221) SHA1(898b8f678fd03c462ce0d8eb7fb3441ef601085b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.17m",   0x0000004, 0x400000, CRC(92db3474) SHA1(7b6f4c8ebfdac167b25f35029068b6253c141fe6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.19m",   0x0000006, 0x400000, CRC(3733473c) SHA1(6579da7145c95b3ad00844a5fc8c2e22c23365e2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.14m",   0x1000000, 0x400000, CRC(bcac2e41) SHA1(838ff24f7e8543a787a55a5d592c9517ce3b8b93) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.16m",   0x1000002, 0x400000, CRC(ea04a272) SHA1(cd7c79037b5b4a39bef5156433e984dc4dc2c081) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.18m",   0x1000004, 0x400000, CRC(b0def86a) SHA1(da3a6705ea7050fc5c2c10d33400ed67be9f455d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.20m",   0x1000006, 0x400000, CRC(4b40ff9f) SHA1(9a981d442132efff09a27408d74646ba357c7357) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "xvs.01",   0x00000, 0x08000, CRC(3999e93a) SHA1(fefcff8a9a5c83df7655a16187cf9ba3e7efbb25) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "xvsa.02",  0x28000, 0x20000, CRC(19272e4c) SHA1(8a4a85cbdfb867a2014af2405cc8214541250b50) ) /* Different sound code then the other sets */

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "xvs.11m",   0x000000, 0x200000, CRC(9cadcdbc) SHA1(64d3bd53b04daec84c9af4aa3ff010867b3d306d) )
	ROM_LOAD16_WORD_SWAP( "xvs.12m",   0x200000, 0x200000, CRC(7b11e460) SHA1(a581c84acaaf0ce056841c15a6f36889e88be68d) )
ROM_END

ROM_START( xmvsfh )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "xvsh.03a", 0x000000, 0x80000, CRC(d4fffb04) SHA1(989ed975cfc1318998c2da26f450949bdac41d0c) )
	ROM_LOAD16_WORD_SWAP( "xvsh.04a", 0x080000, 0x80000, CRC(1b4ea638) SHA1(7523be63c1eef153e47fc8e1c10eb99ab40b94a0) )
	ROM_LOAD16_WORD_SWAP( "xvs.05a",  0x100000, 0x80000, CRC(7db6025d) SHA1(2d74f48f83f45359bfaca28ab686625766af12ee) )
	ROM_LOAD16_WORD_SWAP( "xvs.06a",  0x180000, 0x80000, CRC(e8e2c75c) SHA1(929408cb5d98e95cec75ea58e4701b0cbdbcd016) )
	ROM_LOAD16_WORD_SWAP( "xvs.07",   0x200000, 0x80000, CRC(08f0abed) SHA1(ef16c376232dba63b0b9bc3aa0640f9001ccb68a) )
	ROM_LOAD16_WORD_SWAP( "xvs.08",   0x280000, 0x80000, CRC(81929675) SHA1(19cf7afbc1daaefec40195e40ba74970f3906a1c) )
	ROM_LOAD16_WORD_SWAP( "xvs.09",   0x300000, 0x80000, CRC(9641f36b) SHA1(dcba3482d1ba37ccfb30d402793ee063c6621aed) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "xvs.13m",   0x0000000, 0x400000, CRC(f6684efd) SHA1(c0a2f3a9e82ab8b084a500aec71ac633e947328c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.15m",   0x0000002, 0x400000, CRC(29109221) SHA1(898b8f678fd03c462ce0d8eb7fb3441ef601085b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.17m",   0x0000004, 0x400000, CRC(92db3474) SHA1(7b6f4c8ebfdac167b25f35029068b6253c141fe6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.19m",   0x0000006, 0x400000, CRC(3733473c) SHA1(6579da7145c95b3ad00844a5fc8c2e22c23365e2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.14m",   0x1000000, 0x400000, CRC(bcac2e41) SHA1(838ff24f7e8543a787a55a5d592c9517ce3b8b93) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.16m",   0x1000002, 0x400000, CRC(ea04a272) SHA1(cd7c79037b5b4a39bef5156433e984dc4dc2c081) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.18m",   0x1000004, 0x400000, CRC(b0def86a) SHA1(da3a6705ea7050fc5c2c10d33400ed67be9f455d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.20m",   0x1000006, 0x400000, CRC(4b40ff9f) SHA1(9a981d442132efff09a27408d74646ba357c7357) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "xvs.01",   0x00000, 0x08000, CRC(3999e93a) SHA1(fefcff8a9a5c83df7655a16187cf9ba3e7efbb25) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "xvs.02",   0x28000, 0x20000, CRC(101bdee9) SHA1(75920e88bf46fcd33a7957777a1d799818ffb0d6) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "xvs.11m",   0x000000, 0x200000, CRC(9cadcdbc) SHA1(64d3bd53b04daec84c9af4aa3ff010867b3d306d) )
	ROM_LOAD16_WORD_SWAP( "xvs.12m",   0x200000, 0x200000, CRC(7b11e460) SHA1(a581c84acaaf0ce056841c15a6f36889e88be68d) )
ROM_END

ROM_START( xmvsfb )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "xvsb.03h", 0x000000, 0x80000, CRC(05baccca) SHA1(7124e49e2180f77674ae014257a816cd4409d613) )
	ROM_LOAD16_WORD_SWAP( "xvsb.04h", 0x080000, 0x80000, CRC(e350c755) SHA1(5e615fd4b9954410c05b34151fae70d910340a6c) )
	ROM_LOAD16_WORD_SWAP( "xvs.05a",  0x100000, 0x80000, CRC(7db6025d) SHA1(2d74f48f83f45359bfaca28ab686625766af12ee) )
	ROM_LOAD16_WORD_SWAP( "xvs.06a",  0x180000, 0x80000, CRC(e8e2c75c) SHA1(929408cb5d98e95cec75ea58e4701b0cbdbcd016) )
	ROM_LOAD16_WORD_SWAP( "xvs.07",   0x200000, 0x80000, CRC(08f0abed) SHA1(ef16c376232dba63b0b9bc3aa0640f9001ccb68a) )
	ROM_LOAD16_WORD_SWAP( "xvs.08",   0x280000, 0x80000, CRC(81929675) SHA1(19cf7afbc1daaefec40195e40ba74970f3906a1c) )
	ROM_LOAD16_WORD_SWAP( "xvs.09",   0x300000, 0x80000, CRC(9641f36b) SHA1(dcba3482d1ba37ccfb30d402793ee063c6621aed) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "xvs.13m",   0x0000000, 0x400000, CRC(f6684efd) SHA1(c0a2f3a9e82ab8b084a500aec71ac633e947328c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.15m",   0x0000002, 0x400000, CRC(29109221) SHA1(898b8f678fd03c462ce0d8eb7fb3441ef601085b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.17m",   0x0000004, 0x400000, CRC(92db3474) SHA1(7b6f4c8ebfdac167b25f35029068b6253c141fe6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.19m",   0x0000006, 0x400000, CRC(3733473c) SHA1(6579da7145c95b3ad00844a5fc8c2e22c23365e2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.14m",   0x1000000, 0x400000, CRC(bcac2e41) SHA1(838ff24f7e8543a787a55a5d592c9517ce3b8b93) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.16m",   0x1000002, 0x400000, CRC(ea04a272) SHA1(cd7c79037b5b4a39bef5156433e984dc4dc2c081) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.18m",   0x1000004, 0x400000, CRC(b0def86a) SHA1(da3a6705ea7050fc5c2c10d33400ed67be9f455d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.20m",   0x1000006, 0x400000, CRC(4b40ff9f) SHA1(9a981d442132efff09a27408d74646ba357c7357) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "xvs.01",   0x00000, 0x08000, CRC(3999e93a) SHA1(fefcff8a9a5c83df7655a16187cf9ba3e7efbb25) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "xvs.02",   0x28000, 0x20000, CRC(101bdee9) SHA1(75920e88bf46fcd33a7957777a1d799818ffb0d6) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "xvs.11m",   0x000000, 0x200000, CRC(9cadcdbc) SHA1(64d3bd53b04daec84c9af4aa3ff010867b3d306d) )
	ROM_LOAD16_WORD_SWAP( "xvs.12m",   0x200000, 0x200000, CRC(7b11e460) SHA1(a581c84acaaf0ce056841c15a6f36889e88be68d) )
ROM_END


/*************************************
 *
 *  Games initialization
 *
 *************************************/

static DRIVER_INIT( cps2 )
{
	cps_state *state = (cps_state *)machine->driver_data;

	/* Decrypt the game - see machine/cps2crpt.c */
	DRIVER_INIT_CALL(cps2crpt);

	/* Initialize some video elements */
	DRIVER_INIT_CALL(cps2_video);

	state->scancount = 0;
	state->cps2networkpresent = 0;

	machine->device("maincpu")->set_clock_scale(0.7375f); /* RAM access waitstates etc. aren't emulated - slow the CPU to compensate */
}

static DRIVER_INIT( ssf2tb )
{
	cps_state *state = (cps_state *)machine->driver_data;

	DRIVER_INIT_CALL(cps2);

	state->cps2networkpresent = 0;

	/* we don't emulate the network board, so don't say it's present for now, otherwise the game will
       attempt to boot in tournament mode and fail */
	//state->cps2networkpresent = 1;

}

static DRIVER_INIT ( pzloop2 )
{
	cps_state *state = (cps_state *)machine->driver_data;

	DRIVER_INIT_CALL(cps2);

	state->readpaddle = 0;

	state_save_register_global(machine, state->readpaddle);

	memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x804000, 0x804001, 0, 0, joy_or_paddle_r);
}

static READ16_HANDLER( gigamn2_dummyqsound_r )
{
	cps_state *state = (cps_state *)space->machine->driver_data;
	return state->gigamn2_dummyqsound_ram[offset];
};

static WRITE16_HANDLER( gigamn2_dummyqsound_w )
{
	cps_state *state = (cps_state *)space->machine->driver_data;
	state->gigamn2_dummyqsound_ram[offset] = data;
};

static DRIVER_INIT( gigamn2 )
{
	cps_state *state = (cps_state *)machine->driver_data;
	const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);
	UINT16 *rom = (UINT16 *)memory_region(machine, "maincpu");
	int length = memory_region_length(machine, "maincpu");

	DRIVER_INIT_CALL(cps2);

	state->gigamn2_dummyqsound_ram = auto_alloc_array(machine, UINT16, 0x20000 / 2);
	state_save_register_global_pointer(machine, state->gigamn2_dummyqsound_ram, 0x20000 / 2);

	memory_install_readwrite16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x618000, 0x619fff, 0, 0, gigamn2_dummyqsound_r, gigamn2_dummyqsound_w); // no qsound..
	memory_set_decrypted_region(space, 0x000000, (length) - 1, &rom[length/4]);
	m68k_set_encrypted_opcode_range(machine->device("maincpu"), 0, length);
}


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1993, ssf2,       0,        cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Super Street Fighter II: The New Challengers (World 930911)", GAME_SUPPORTS_SAVE )
GAME( 1993, ssf2u,      ssf2,     cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Super Street Fighter II: The New Challengers (USA 930911)", GAME_SUPPORTS_SAVE )
GAME( 1993, ssf2a,      ssf2,     cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Super Street Fighter II: The New Challengers (Asia 931005)", GAME_SUPPORTS_SAVE )
GAME( 1993, ssf2ar1,    ssf2,     cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Super Street Fighter II: The New Challengers (Asia 930914)", GAME_SUPPORTS_SAVE )
GAME( 1993, ssf2j,      ssf2,     cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Super Street Fighter II: The New Challengers (Japan 931005)", GAME_SUPPORTS_SAVE )
GAME( 1993, ssf2jr1,    ssf2,     cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Super Street Fighter II: The New Challengers (Japan 930911)", GAME_SUPPORTS_SAVE )
GAME( 1993, ssf2jr2,    ssf2,     cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Super Street Fighter II: The New Challengers (Japan 930910)", GAME_SUPPORTS_SAVE )
GAME( 1993, ssf2h,      ssf2,     cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Super Street Fighter II: The New Challengers (Hispanic 930911)", GAME_SUPPORTS_SAVE )
GAME( 1993, ssf2tb,     ssf2,     cps2, cps2_2p6b, ssf2tb,   ROT0,   "Capcom", "Super Street Fighter II: The Tournament Battle (World 931119)", GAME_SUPPORTS_SAVE )	// works, but not in tournament mode
GAME( 1993, ssf2tbr1,   ssf2,     cps2, cps2_2p6b, ssf2tb,   ROT0,   "Capcom", "Super Street Fighter II: The Tournament Battle (World 930911)", GAME_SUPPORTS_SAVE )	// works, but not in tournament mode
GAME( 1993, ssf2tbj,    ssf2,     cps2, cps2_2p6b, ssf2tb,   ROT0,   "Capcom", "Super Street Fighter II: The Tournament Battle (Japan 930911)", GAME_SUPPORTS_SAVE )	// works, but not in tournament mode
GAME( 1993, ecofghtr,   0,        cps2, cps2_2p3b, cps2,     ROT0,   "Capcom", "Eco Fighters (World 931203)", GAME_SUPPORTS_SAVE )
GAME( 1993, ecofghtru,  ecofghtr, cps2, cps2_2p3b, cps2,     ROT0,   "Capcom", "Eco Fighters (USA 940215)", GAME_SUPPORTS_SAVE )
GAME( 1993, ecofghtru1, ecofghtr, cps2, cps2_2p3b, cps2,     ROT0,   "Capcom", "Eco Fighters (USA 931203)", GAME_SUPPORTS_SAVE )
GAME( 1993, uecology,   ecofghtr, cps2, cps2_2p3b, cps2,     ROT0,   "Capcom", "Ultimate Ecology (Japan 931203)", GAME_SUPPORTS_SAVE )
GAME( 1993, ecofghtra,  ecofghtr, cps2, cps2_2p3b, cps2,     ROT0,   "Capcom", "Eco Fighters (Asia 931203)", GAME_SUPPORTS_SAVE )
GAME( 1993, ecofghtrh,  ecofghtr, cps2, cps2_2p3b, cps2,     ROT0,   "Capcom", "Eco Fighters (Hispanic 931203)", GAME_SUPPORTS_SAVE )
GAME( 1993, ddtod,      0,        cps2, cps2_4p4b, cps2,     ROT0,   "Capcom", "Dungeons & Dragons: Tower of Doom (Euro 940412)", GAME_SUPPORTS_SAVE )
GAME( 1993, ddtodr1,    ddtod,    cps2, cps2_4p4b, cps2,     ROT0,   "Capcom", "Dungeons & Dragons: Tower of Doom (Euro 940113)", GAME_SUPPORTS_SAVE )
GAME( 1993, ddtodu,     ddtod,    cps2, cps2_4p4b, cps2,     ROT0,   "Capcom", "Dungeons & Dragons: Tower of Doom (USA 940125)", GAME_SUPPORTS_SAVE )
GAME( 1993, ddtodur1,   ddtod,    cps2, cps2_4p4b, cps2,     ROT0,   "Capcom", "Dungeons & Dragons: Tower of Doom (USA 940113)", GAME_SUPPORTS_SAVE )
GAME( 1993, ddtodj,     ddtod,    cps2, cps2_4p4b, cps2,     ROT0,   "Capcom", "Dungeons & Dragons: Tower of Doom (Japan 940412)", GAME_SUPPORTS_SAVE )
GAME( 1993, ddtodjr1,   ddtod,    cps2, cps2_4p4b, cps2,     ROT0,   "Capcom", "Dungeons & Dragons: Tower of Doom (Japan 940125)", GAME_SUPPORTS_SAVE )
GAME( 1993, ddtodjr2,   ddtod,    cps2, cps2_4p4b, cps2,     ROT0,   "Capcom", "Dungeons & Dragons: Tower of Doom (Japan 940113)", GAME_SUPPORTS_SAVE )
GAME( 1993, ddtoda,     ddtod,    cps2, cps2_4p4b, cps2,     ROT0,   "Capcom", "Dungeons & Dragons: Tower of Doom (Asia 940113)", GAME_SUPPORTS_SAVE )
GAME( 1993, ddtodh,     ddtod,    cps2, cps2_4p4b, cps2,     ROT0,   "Capcom", "Dungeons & Dragons: Tower of Doom (Hispanic 940412)", GAME_SUPPORTS_SAVE )
GAME( 1993, ddtodhr1,   ddtod,    cps2, cps2_4p4b, cps2,     ROT0,   "Capcom", "Dungeons & Dragons: Tower of Doom (Hispanic 940125)", GAME_SUPPORTS_SAVE )
GAME( 1993, ddtodhr2,   ddtod,    cps2, cps2_4p4b, cps2,     ROT0,   "Capcom", "Dungeons & Dragons: Tower of Doom (Hispanic 940113)", GAME_SUPPORTS_SAVE )
GAME( 1994, ssf2t,      0,        cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Super Street Fighter II Turbo (World 940223)", GAME_SUPPORTS_SAVE )
GAME( 1994, ssf2ta,     ssf2t,    cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Super Street Fighter II Turbo (Asia 940223)", GAME_SUPPORTS_SAVE )
GAME( 1994, ssf2tu,     ssf2t,    cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Super Street Fighter II Turbo (USA 940323)", GAME_SUPPORTS_SAVE )
GAME( 1994, ssf2tur1,   ssf2t,    cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Super Street Fighter II Turbo (USA 940223)", GAME_SUPPORTS_SAVE )
GAME( 1994, ssf2xj,     ssf2t,    cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Super Street Fighter II X: Grand Master Challenge (Japan 940223)", GAME_SUPPORTS_SAVE )
GAME( 1994, avsp,       0,        cps2, cps2_3p3b, cps2,     ROT0,   "Capcom", "Alien vs. Predator (Euro 940520)", GAME_SUPPORTS_SAVE )
GAME( 1994, avspu,      avsp,     cps2, cps2_3p3b, cps2,     ROT0,   "Capcom", "Alien vs. Predator (USA 940520)", GAME_SUPPORTS_SAVE )
GAME( 1994, avspj,      avsp,     cps2, cps2_3p3b, cps2,     ROT0,   "Capcom", "Alien vs. Predator (Japan 940520)", GAME_SUPPORTS_SAVE )
GAME( 1994, avspa,      avsp,     cps2, cps2_3p3b, cps2,     ROT0,   "Capcom", "Alien vs. Predator (Asia 940520)", GAME_SUPPORTS_SAVE )
GAME( 1994, avsph,      avsp,     cps2, cps2_3p3b, cps2,     ROT0,   "Capcom", "Alien vs. Predator (Hispanic 940520)", GAME_SUPPORTS_SAVE )
GAME( 1994, dstlk,      0,        cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Darkstalkers: The Night Warriors (Euro 940705)", GAME_SUPPORTS_SAVE )
GAME( 1994, dstlku,     dstlk,    cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Darkstalkers: The Night Warriors (USA 940818)", GAME_SUPPORTS_SAVE )
GAME( 1994, dstlkur1,   dstlk,    cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Darkstalkers: The Night Warriors (USA 940705)", GAME_SUPPORTS_SAVE )
GAME( 1994, dstlka,     dstlk,    cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Darkstalkers: The Night Warriors (Asia 940705)", GAME_SUPPORTS_SAVE )
GAME( 1994, dstlkh,     dstlk,    cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Darkstalkers: The Night Warriors (Hispanic 940818)", GAME_SUPPORTS_SAVE )
GAME( 1994, vampj,      dstlk,    cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Vampire: The Night Warriors (Japan 940705)", GAME_SUPPORTS_SAVE )	// partial update set? Only rom 04 is "B" revision
GAME( 1994, vampja,     dstlk,    cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Vampire: The Night Warriors (Japan 940705 alt)", GAME_SUPPORTS_SAVE )
GAME( 1994, vampjr1,    dstlk,    cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Vampire: The Night Warriors (Japan 940630)", GAME_SUPPORTS_SAVE )
GAME( 1994, ringdest,   0,        cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Ring of Destruction: Slammasters II (Euro 940902)", GAME_SUPPORTS_SAVE )
GAME( 1994, smbomb,     ringdest, cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Super Muscle Bomber: The International Blowout (Japan 940831)", GAME_SUPPORTS_SAVE )
GAME( 1994, smbombr1,   ringdest, cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Super Muscle Bomber: The International Blowout (Japan 940808)", GAME_SUPPORTS_SAVE )
GAME( 1994, armwar,     0,        cps2, cps2_3p3b, cps2,     ROT0,   "Capcom", "Armored Warriors (Euro 941024)", GAME_SUPPORTS_SAVE )
GAME( 1994, armwarr1,   armwar,   cps2, cps2_3p3b, cps2,     ROT0,   "Capcom", "Armored Warriors (Euro 941011)", GAME_SUPPORTS_SAVE )
GAME( 1994, armwaru,    armwar,   cps2, cps2_3p3b, cps2,     ROT0,   "Capcom", "Armored Warriors (USA 941024)", GAME_SUPPORTS_SAVE )
GAME( 1994, armwaru1,   armwar,   cps2, cps2_3p3b, cps2,     ROT0,   "Capcom", "Armored Warriors (USA 940920)", GAME_SUPPORTS_SAVE )
GAME( 1994, pgear,      armwar,   cps2, cps2_3p3b, cps2,     ROT0,   "Capcom", "Powered Gear: Strategic Variant Armor Equipment (Japan 941024)", GAME_SUPPORTS_SAVE )
GAME( 1994, pgearr1,    armwar,   cps2, cps2_3p3b, cps2,     ROT0,   "Capcom", "Powered Gear: Strategic Variant Armor Equipment (Japan 940916)", GAME_SUPPORTS_SAVE )
GAME( 1994, armwara,    armwar,   cps2, cps2_3p3b, cps2,     ROT0,   "Capcom", "Armored Warriors (Asia 940920)", GAME_SUPPORTS_SAVE )
GAME( 1994, xmcota,     0,        cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "X-Men: Children of the Atom (Euro 950105)", GAME_SUPPORTS_SAVE )
GAME( 1994, xmcotau,    xmcota,   cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "X-Men: Children of the Atom (USA 950105)", GAME_SUPPORTS_SAVE )
GAME( 1994, xmcotah,    xmcota,   cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "X-Men: Children of the Atom (Hispanic 950331)", GAME_SUPPORTS_SAVE )
GAME( 1994, xmcotahr1,  xmcota,   cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "X-Men: Children of the Atom (Hispanic 950105)", GAME_SUPPORTS_SAVE )
GAME( 1994, xmcotaj,    xmcota,   cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "X-Men: Children of the Atom (Japan 950105)", GAME_SUPPORTS_SAVE )
GAME( 1994, xmcotaj1,   xmcota,   cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "X-Men: Children of the Atom (Japan 941222)", GAME_SUPPORTS_SAVE )
GAME( 1994, xmcotaj2,   xmcota,   cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "X-Men: Children of the Atom (Japan 941219)", GAME_SUPPORTS_SAVE )
GAME( 1994, xmcotaj3,   xmcota,   cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "X-Men: Children of the Atom (Japan 941217)", GAME_SUPPORTS_SAVE )
GAME( 1994, xmcotajr,   xmcota,   cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "X-Men: Children of the Atom (Japan 941208 rent version)", GAME_SUPPORTS_SAVE )
GAME( 1994, xmcotaa,    xmcota,   cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "X-Men: Children of the Atom (Asia 941217)", GAME_SUPPORTS_SAVE )
GAME( 1995, nwarr,      0,        cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Night Warriors: Darkstalkers' Revenge (Euro 950316)", GAME_SUPPORTS_SAVE )
GAME( 1995, nwarru,     nwarr,    cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Night Warriors: Darkstalkers' Revenge (USA 950406)", GAME_SUPPORTS_SAVE )
GAME( 1995, nwarrh,     nwarr,    cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Night Warriors: Darkstalkers' Revenge (Hispanic 950403)", GAME_SUPPORTS_SAVE )
GAME( 1995, nwarrb,     nwarr,    cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Night Warriors: Darkstalkers' Revenge (Brazil 950403)", GAME_SUPPORTS_SAVE )
GAME( 1995, nwarra,     nwarr,    cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Night Warriors: Darkstalkers' Revenge (Asia 950302)", GAME_SUPPORTS_SAVE )
GAME( 1995, vhuntj,     nwarr,    cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Vampire Hunter: Darkstalkers' Revenge (Japan 950316)", GAME_SUPPORTS_SAVE )
GAME( 1995, vhuntjr1,   nwarr,    cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Vampire Hunter: Darkstalkers' Revenge (Japan 950307)", GAME_SUPPORTS_SAVE )
GAME( 1995, vhuntjr2,   nwarr,    cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Vampire Hunter: Darkstalkers' Revenge (Japan 950302)", GAME_SUPPORTS_SAVE )
GAME( 1995, cybots,     0,        cps2, cybots,    cps2,     ROT0,   "Capcom", "Cyberbots: Fullmetal Madness (Euro 950424)", GAME_SUPPORTS_SAVE )
GAME( 1995, cybotsu,    cybots,   cps2, cybots,    cps2,     ROT0,   "Capcom", "Cyberbots: Fullmetal Madness (USA 950424)", GAME_SUPPORTS_SAVE )
GAME( 1995, cybotsj,    cybots,   cps2, cybots,    cps2,     ROT0,   "Capcom", "Cyberbots: Fullmetal Madness (Japan 950420)", GAME_SUPPORTS_SAVE )
GAME( 1995, sfa,        0,        cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Street Fighter Alpha: Warriors' Dreams (Euro 950727)", GAME_SUPPORTS_SAVE )
GAME( 1995, sfar1,      sfa,      cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Street Fighter Alpha: Warriors' Dreams (Euro 950718)", GAME_SUPPORTS_SAVE )
GAME( 1995, sfar2,      sfa,      cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Street Fighter Alpha: Warriors' Dreams (Euro 950627)", GAME_SUPPORTS_SAVE )
GAME( 1995, sfar3,      sfa,      cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Street Fighter Alpha: Warriors' Dreams (Euro 950605)", GAME_SUPPORTS_SAVE )
GAME( 1995, sfau,       sfa,      cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Street Fighter Alpha: Warriors' Dreams (USA 950627)", GAME_SUPPORTS_SAVE )
GAME( 1995, sfza,       sfa,      cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Street Fighter Zero (Asia 950627)", GAME_SUPPORTS_SAVE )
GAME( 1995, sfzj,       sfa,      cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Street Fighter Zero (Japan 950727)", GAME_SUPPORTS_SAVE )
GAME( 1995, sfzjr1,     sfa,      cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Street Fighter Zero (Japan 950627)", GAME_SUPPORTS_SAVE )
GAME( 1995, sfzjr2,     sfa,      cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Street Fighter Zero (Japan 950605)", GAME_SUPPORTS_SAVE )
GAME( 1995, sfzh,       sfa,      cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Street Fighter Zero (Hispanic 950718)", GAME_SUPPORTS_SAVE )
GAME( 1995, sfzhr1,     sfa,      cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Street Fighter Zero (Hispanic 950627)", GAME_SUPPORTS_SAVE )
GAME( 1995, sfzb,       sfa,      cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Street Fighter Zero (Brazil 951109)", GAME_SUPPORTS_SAVE )
GAME( 1995, sfzbr1,     sfa,      cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Street Fighter Zero (Brazil 950727)", GAME_SUPPORTS_SAVE )
GAME( 1995, mmancp2u,   megaman,  cps2, cps2_2p3b, cps2,     ROT0,   "Capcom", "Mega Man - The Power Battle (CPS2, USA 951006, SAMPLE Version)", GAME_SUPPORTS_SAVE )
GAME( 1995, rmancp2j,   megaman,  cps2, cps2_2p3b, cps2,     ROT0,   "Capcom", "Rockman: The Power Battle (CPS2, Japan 950922)", GAME_SUPPORTS_SAVE )
GAME( 1995, msh,        0,        cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Marvel Super Heroes (Euro 951024)", GAME_SUPPORTS_SAVE )
GAME( 1995, mshu,       msh,      cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Marvel Super Heroes (USA 951024)", GAME_SUPPORTS_SAVE )
GAME( 1995, mshj,       msh,      cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Marvel Super Heroes (Japan 951117)", GAME_SUPPORTS_SAVE )
GAME( 1995, mshjr1,     msh,      cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Marvel Super Heroes (Japan 951024)", GAME_SUPPORTS_SAVE )
GAME( 1995, msha,       msh,      cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Marvel Super Heroes (Asia 951024)", GAME_SUPPORTS_SAVE )
GAME( 1995, mshh,       msh,      cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Marvel Super Heroes (Hispanic 951117)", GAME_SUPPORTS_SAVE )
GAME( 1995, mshb,       msh,      cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Marvel Super Heroes (Brazil 951117)", GAME_SUPPORTS_SAVE )
GAME( 1996, 19xx,       0,        cps2, cps2_2p2b, cps2,     ROT270, "Capcom", "19XX: The War Against Destiny (USA 951207)", GAME_SUPPORTS_SAVE )
GAME( 1996, 19xxa,      19xx,     cps2, cps2_2p2b, cps2,     ROT270, "Capcom", "19XX: The War Against Destiny (Asia 951207)", GAME_SUPPORTS_SAVE )
GAME( 1996, 19xxj,      19xx,     cps2, cps2_2p2b, cps2,     ROT270, "Capcom", "19XX: The War Against Destiny (Japan 951225)", GAME_SUPPORTS_SAVE )
GAME( 1996, 19xxjr1,    19xx,     cps2, cps2_2p2b, cps2,     ROT270, "Capcom", "19XX: The War Against Destiny (Japan 951207)", GAME_SUPPORTS_SAVE )
GAME( 1996, 19xxh,      19xx,     cps2, cps2_2p2b, cps2,     ROT270, "Capcom", "19XX: The War Against Destiny (Hispanic 951218)", GAME_SUPPORTS_SAVE )
GAME( 1996, 19xxb,      19xx,     cps2, cps2_2p2b, cps2,     ROT270, "Capcom", "19XX: The War Against Destiny (Brazil 951218)", GAME_SUPPORTS_SAVE )
GAME( 1996, ddsom,      0,        cps2, cps2_4p4b, cps2,     ROT0,   "Capcom", "Dungeons & Dragons: Shadow over Mystara (Euro 960619)", GAME_SUPPORTS_SAVE )
GAME( 1996, ddsomr1,    ddsom,    cps2, cps2_4p4b, cps2,     ROT0,   "Capcom", "Dungeons & Dragons: Shadow over Mystara (Euro 960223)", GAME_SUPPORTS_SAVE )
GAME( 1996, ddsomr2,    ddsom,    cps2, cps2_4p4b, cps2,     ROT0,   "Capcom", "Dungeons & Dragons: Shadow over Mystara (Euro 960209)", GAME_SUPPORTS_SAVE )
GAME( 1996, ddsomr3,    ddsom,    cps2, cps2_4p4b, cps2,     ROT0,   "Capcom", "Dungeons & Dragons: Shadow over Mystara (Euro 960208)", GAME_SUPPORTS_SAVE )
GAME( 1996, ddsomu,     ddsom,    cps2, cps2_4p4b, cps2,     ROT0,   "Capcom", "Dungeons & Dragons: Shadow over Mystara (USA 960619)", GAME_SUPPORTS_SAVE )
GAME( 1996, ddsomur1,   ddsom,    cps2, cps2_4p4b, cps2,     ROT0,   "Capcom", "Dungeons & Dragons: Shadow over Mystara (USA 960209)", GAME_SUPPORTS_SAVE )
GAME( 1996, ddsomj,     ddsom,    cps2, cps2_4p4b, cps2,     ROT0,   "Capcom", "Dungeons & Dragons: Shadow over Mystara (Japan 960619)", GAME_SUPPORTS_SAVE )
GAME( 1996, ddsomjr1,   ddsom,    cps2, cps2_4p4b, cps2,     ROT0,   "Capcom", "Dungeons & Dragons: Shadow over Mystara (Japan 960206)", GAME_SUPPORTS_SAVE )
GAME( 1996, ddsoma,     ddsom,    cps2, cps2_4p4b, cps2,     ROT0,   "Capcom", "Dungeons & Dragons: Shadow over Mystara (Asia 960619)", GAME_SUPPORTS_SAVE )
GAME( 1996, ddsomh,     ddsom,    cps2, cps2_4p4b, cps2,     ROT0,   "Capcom", "Dungeons & Dragons: Shadow over Mystara (Hispanic 960223)", GAME_SUPPORTS_SAVE )
GAME( 1996, ddsomb,     ddsom,    cps2, cps2_4p4b, cps2,     ROT0,   "Capcom", "Dungeons & Dragons: Shadow over Mystara (Brazil 960223)", GAME_SUPPORTS_SAVE )
GAME( 1996, sfa2,       0,        cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Street Fighter Alpha 2 (Euro 960229)", GAME_SUPPORTS_SAVE )
GAME( 1996, sfa2u,      sfa2,     cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Street Fighter Alpha 2 (USA 960306)", GAME_SUPPORTS_SAVE )
GAME( 1996, sfz2j,      sfa2,     cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Street Fighter Zero 2 (Japan 960227)", GAME_SUPPORTS_SAVE )
GAME( 1996, sfz2a,      sfa2,     cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Street Fighter Zero 2 (Asia 960227)", GAME_SUPPORTS_SAVE )
GAME( 1996, sfz2b,      sfa2,     cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Street Fighter Zero 2 (Brazil 960531)", GAME_SUPPORTS_SAVE )
GAME( 1996, sfz2br1,    sfa2,     cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Street Fighter Zero 2 (Brazil 960304)", GAME_SUPPORTS_SAVE )
GAME( 1996, sfz2h,      sfa2,     cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Street Fighter Zero 2 (Hispanic 960304)", GAME_SUPPORTS_SAVE )
GAME( 1996, sfz2n,      sfa2,     cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Street Fighter Zero 2 (Oceania 960229)", GAME_SUPPORTS_SAVE )
GAME( 1996, sfz2al,     0,        cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Street Fighter Zero 2 Alpha (Asia 960826)", GAME_SUPPORTS_SAVE )
GAME( 1996, sfz2alj,    sfz2al,   cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Street Fighter Zero 2 Alpha (Japan 960805)", GAME_SUPPORTS_SAVE )
GAME( 1996, sfz2alh,    sfz2al,   cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Street Fighter Zero 2 Alpha (Hispanic 960813)", GAME_SUPPORTS_SAVE )
GAME( 1996, sfz2alb,    sfz2al,   cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Street Fighter Zero 2 Alpha (Brazil 960813)", GAME_SUPPORTS_SAVE )
GAME( 1996, spf2t,      0,        cps2, cps2_2p2b, cps2,     ROT0,   "Capcom", "Super Puzzle Fighter II Turbo (USA 960620)", GAME_SUPPORTS_SAVE )
GAME( 1996, spf2xj,     spf2t,    cps2, cps2_2p2b, cps2,     ROT0,   "Capcom", "Super Puzzle Fighter II X (Japan 960531)", GAME_SUPPORTS_SAVE )
GAME( 1996, spf2ta,     spf2t,    cps2, cps2_2p2b, cps2,     ROT0,   "Capcom", "Super Puzzle Fighter II Turbo (Asia 960529)", GAME_SUPPORTS_SAVE )
GAME( 1996, spf2th,     spf2t,    cps2, cps2_2p2b, cps2,     ROT0,   "Capcom", "Super Puzzle Fighter II Turbo (Hispanic 960531)", GAME_SUPPORTS_SAVE )
GAME( 1996, megaman2,   0,        cps2, cps2_2p3b, cps2,     ROT0,   "Capcom", "Mega Man 2: The Power Fighters (USA 960708)", GAME_SUPPORTS_SAVE )
GAME( 1996, megaman2a,  megaman2, cps2, cps2_2p3b, cps2,     ROT0,   "Capcom", "Mega Man 2: The Power Fighters (Asia 960708)", GAME_SUPPORTS_SAVE )
GAME( 1996, rockman2j,  megaman2, cps2, cps2_2p3b, cps2,     ROT0,   "Capcom", "Rockman 2: The Power Fighters (Japan 960708)", GAME_SUPPORTS_SAVE )
GAME( 1996, megaman2h,  megaman2, cps2, cps2_2p3b, cps2,     ROT0,   "Capcom", "Mega Man 2: The Power Fighters (Hispanic 960712)", GAME_SUPPORTS_SAVE )
GAME( 1996, qndream,    0,        cps2, qndream,   cps2,     ROT0,   "Capcom", "Quiz Nanairo Dreams: Nijiirochou no Kiseki (Japan 960826)", GAME_SUPPORTS_SAVE )
GAME( 1996, xmvsf,      0,        cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "X-Men Vs. Street Fighter (Euro 961004)", GAME_SUPPORTS_SAVE )
GAME( 1996, xmvsfr1,    xmvsf,    cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "X-Men Vs. Street Fighter (Euro 960910)", GAME_SUPPORTS_SAVE )
GAME( 1996, xmvsfu,     xmvsf,    cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "X-Men Vs. Street Fighter (USA 961023)", GAME_SUPPORTS_SAVE )
GAME( 1996, xmvsfur1,   xmvsf,    cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "X-Men Vs. Street Fighter (USA 961004)", GAME_SUPPORTS_SAVE )
GAME( 1996, xmvsfj,     xmvsf,    cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "X-Men Vs. Street Fighter (Japan 961004)", GAME_SUPPORTS_SAVE )
GAME( 1996, xmvsfjr1,   xmvsf,    cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "X-Men Vs. Street Fighter (Japan 960910)", GAME_SUPPORTS_SAVE )
GAME( 1996, xmvsfjr2,   xmvsf,    cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "X-Men Vs. Street Fighter (Japan 960909)", GAME_SUPPORTS_SAVE )
GAME( 1996, xmvsfa,     xmvsf,    cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "X-Men Vs. Street Fighter (Asia 961023)", GAME_SUPPORTS_SAVE )
GAME( 1996, xmvsfar1,   xmvsf,    cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "X-Men Vs. Street Fighter (Asia 960919)", GAME_SUPPORTS_SAVE )
GAME( 1996, xmvsfar2,   xmvsf,    cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "X-Men Vs. Street Fighter (Asia 960910)", GAME_SUPPORTS_SAVE )
GAME( 1996, xmvsfh,     xmvsf,    cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "X-Men Vs. Street Fighter (Hispanic 961004)", GAME_SUPPORTS_SAVE )
GAME( 1996, xmvsfb,     xmvsf,    cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "X-Men Vs. Street Fighter (Brazil 961023)", GAME_SUPPORTS_SAVE )
GAME( 1997, batcir,     0,        cps2, cps2_4p2b, cps2,     ROT0,   "Capcom", "Battle Circuit (Euro 970319)", GAME_SUPPORTS_SAVE )
GAME( 1997, batcira,    batcir,   cps2, cps2_4p2b, cps2,     ROT0,   "Capcom", "Battle Circuit (Asia 970319)", GAME_SUPPORTS_SAVE )
GAME( 1997, batcirj,    batcir,   cps2, cps2_4p2b, cps2,     ROT0,   "Capcom", "Battle Circuit (Japan 970319)", GAME_SUPPORTS_SAVE )
GAME( 1997, vsav,       0,        cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Vampire Savior: The Lord of Vampire (Euro 970519)", GAME_SUPPORTS_SAVE )
GAME( 1997, vsavu,      vsav,     cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Vampire Savior: The Lord of Vampire (USA 970519)", GAME_SUPPORTS_SAVE )
GAME( 1997, vsavj,      vsav,     cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Vampire Savior: The Lord of Vampire (Japan 970519)", GAME_SUPPORTS_SAVE )
GAME( 1997, vsava,      vsav,     cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Vampire Savior: The Lord of Vampire (Asia 970519)", GAME_SUPPORTS_SAVE )
GAME( 1997, vsavh,      vsav,     cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Vampire Savior: The Lord of Vampire (Hispanic 970519)", GAME_SUPPORTS_SAVE )
GAME( 1997, mshvsf,     0,        cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Marvel Super Heroes Vs. Street Fighter (Euro 970625)", GAME_SUPPORTS_SAVE )
GAME( 1997, mshvsfu,    mshvsf,   cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Marvel Super Heroes Vs. Street Fighter (USA 970827)", GAME_SUPPORTS_SAVE )
GAME( 1997, mshvsfu1,   mshvsf,   cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Marvel Super Heroes Vs. Street Fighter (USA 970625)", GAME_SUPPORTS_SAVE )
GAME( 1997, mshvsfj,    mshvsf,   cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Marvel Super Heroes Vs. Street Fighter (Japan 970707)", GAME_SUPPORTS_SAVE )
GAME( 1997, mshvsfj1,   mshvsf,   cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Marvel Super Heroes Vs. Street Fighter (Japan 970702)", GAME_SUPPORTS_SAVE )
GAME( 1997, mshvsfj2,   mshvsf,   cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Marvel Super Heroes Vs. Street Fighter (Japan 970625)", GAME_SUPPORTS_SAVE )
GAME( 1997, mshvsfh,    mshvsf,   cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Marvel Super Heroes Vs. Street Fighter (Hispanic 970625)", GAME_SUPPORTS_SAVE )
GAME( 1997, mshvsfa,    mshvsf,   cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Marvel Super Heroes Vs. Street Fighter (Asia 970625)", GAME_SUPPORTS_SAVE )
GAME( 1997, mshvsfa1,   mshvsf,   cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Marvel Super Heroes Vs. Street Fighter (Asia 970620)", GAME_SUPPORTS_SAVE )
GAME( 1997, mshvsfb,    mshvsf,   cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Marvel Super Heroes Vs. Street Fighter (Brazil 970827)", GAME_SUPPORTS_SAVE )
GAME( 1997, mshvsfb1,   mshvsf,   cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Marvel Super Heroes Vs. Street Fighter (Brazil 970625)", GAME_SUPPORTS_SAVE )
GAME( 1997, csclub,     0,        cps2, cps2_2p3b, cps2,     ROT0,   "Capcom", "Capcom Sports Club (Euro 971017)", GAME_SUPPORTS_SAVE )
GAME( 1997, csclub1,    csclub,   cps2, cps2_2p3b, cps2,     ROT0,   "Capcom", "Capcom Sports Club (Euro 970722)", GAME_SUPPORTS_SAVE )
GAME( 1997, cscluba,    csclub,   cps2, cps2_2p3b, cps2,     ROT0,   "Capcom", "Capcom Sports Club (Asia 970722)", GAME_SUPPORTS_SAVE )
GAME( 1997, csclubj,    csclub,   cps2, cps2_2p3b, cps2,     ROT0,   "Capcom", "Capcom Sports Club (Japan 970722)", GAME_SUPPORTS_SAVE )
GAME( 1997, csclubh,    csclub,   cps2, cps2_2p3b, cps2,     ROT0,   "Capcom", "Capcom Sports Club (Hispanic 970722)", GAME_SUPPORTS_SAVE )
GAME( 1997, sgemf,      0,        cps2, cps2_2p3b, cps2,     ROT0,   "Capcom", "Super Gem Fighter Mini Mix (USA 970904)", GAME_SUPPORTS_SAVE )
GAME( 1997, pfghtj,     sgemf,    cps2, cps2_2p3b, cps2,     ROT0,   "Capcom", "Pocket Fighter (Japan 970904)", GAME_SUPPORTS_SAVE )
GAME( 1997, sgemfa,     sgemf,    cps2, cps2_2p3b, cps2,     ROT0,   "Capcom", "Super Gem Fighter: Mini Mix (Asia 970904)", GAME_SUPPORTS_SAVE )
GAME( 1997, sgemfh,     sgemf,    cps2, cps2_2p3b, cps2,     ROT0,   "Capcom", "Super Gem Fighter: Mini Mix (Hispanic 970904)", GAME_SUPPORTS_SAVE )
GAME( 1997, vhunt2,     0,        cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Vampire Hunter 2: Darkstalkers Revenge (Japan 970929)", GAME_SUPPORTS_SAVE )
GAME( 1997, vhunt2r1,   vhunt2,   cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Vampire Hunter 2: Darkstalkers Revenge (Japan 970913)", GAME_SUPPORTS_SAVE )
GAME( 1997, vsav2,      0,        cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Vampire Savior 2: The Lord of Vampire (Japan 970913)", GAME_SUPPORTS_SAVE )
GAME( 1998, mvsc,       0,        cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Marvel Vs. Capcom: Clash of Super Heroes (Euro 980123)", GAME_SUPPORTS_SAVE )
GAME( 1998, mvscr1,     mvsc,     cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Marvel Vs. Capcom: Clash of Super Heroes (Euro 980112)", GAME_SUPPORTS_SAVE )
GAME( 1998, mvscu,      mvsc,     cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Marvel Vs. Capcom: Clash of Super Heroes (USA 980123)", GAME_SUPPORTS_SAVE )
GAME( 1998, mvscur1,    mvsc,     cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Marvel Vs. Capcom: Clash of Super Heroes (USA 971222)", GAME_SUPPORTS_SAVE )
GAME( 1998, mvscj,      mvsc,     cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Marvel Vs. Capcom: Clash of Super Heroes (Japan 980123)", GAME_SUPPORTS_SAVE )
GAME( 1998, mvscjr1,    mvsc,     cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Marvel Vs. Capcom: Clash of Super Heroes (Japan 980112)", GAME_SUPPORTS_SAVE )
GAME( 1998, mvsca,      mvsc,     cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Marvel Vs. Capcom: Clash of Super Heroes (Asia 980123)", GAME_SUPPORTS_SAVE )
GAME( 1998, mvscar1,    mvsc,     cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Marvel Vs. Capcom: Clash of Super Heroes (Asia 980112)", GAME_SUPPORTS_SAVE )
GAME( 1998, mvsch,      mvsc,     cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Marvel Vs. Capcom: Clash of Super Heroes (Hispanic 980123)", GAME_SUPPORTS_SAVE )
GAME( 1998, mvscb,      mvsc,     cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Marvel Vs. Capcom: Clash of Super Heroes (Brazil 980123)", GAME_SUPPORTS_SAVE )
GAME( 1998, sfa3,       0,        cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Street Fighter Alpha 3 (Euro 980904)", GAME_SUPPORTS_SAVE )
GAME( 1998, sfa3u,      sfa3,     cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Street Fighter Alpha 3 (USA 980904)", GAME_SUPPORTS_SAVE )
GAME( 1998, sfa3ur1,    sfa3,     cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Street Fighter Alpha 3 (USA 980629)", GAME_SUPPORTS_SAVE )
GAME( 1998, sfa3h,      sfa3,     cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Street Fighter Alpha 3 (Hispanic 980629)", GAME_SUPPORTS_SAVE )
GAME( 1998, sfa3b,      sfa3,     cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Street Fighter Alpha 3 (Brazil 980629)", GAME_SUPPORTS_SAVE )
GAME( 1998, sfz3j,      sfa3,     cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Street Fighter Zero 3 (Japan 980904)", GAME_SUPPORTS_SAVE )
GAME( 1998, sfz3jr1,    sfa3,     cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Street Fighter Zero 3 (Japan 980727)", GAME_SUPPORTS_SAVE )
GAME( 1998, sfz3jr2,    sfa3,     cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Street Fighter Zero 3 (Japan 980629)", GAME_SUPPORTS_SAVE )
GAME( 1998, sfz3a,      sfa3,     cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Street Fighter Zero 3 (Asia 980904)", GAME_SUPPORTS_SAVE )
GAME( 1998, sfz3ar1,    sfa3,     cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Street Fighter Zero 3 (Asia 980701)", GAME_SUPPORTS_SAVE )
GAME( 1999, jyangoku,   0,        cps2, cps2_1p2b, cps2,     ROT0,   "Capcom", "Jyangokushi: Haoh no Saihai (Japan 990527)", GAME_SUPPORTS_SAVE )
GAME( 2004, hsf2,       0,        cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Hyper Street Fighter 2: The Anniversary Edition (USA 040202)", GAME_SUPPORTS_SAVE )
GAME( 2004, hsf2a,      hsf2,     cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Hyper Street Fighter 2: The Anniversary Edition (Asia 040202)", GAME_SUPPORTS_SAVE )
GAME( 2004, hsf2j,      hsf2,     cps2, cps2_2p6b, cps2,     ROT0,   "Capcom", "Hyper Street Fighter 2: The Anniversary Edition (Japan 031222)", GAME_SUPPORTS_SAVE )

/* Games released on CPS-2 hardware by Takumi */

GAME( 1999, gigawing,   0,        cps2, cps2_2p2b, cps2,     ROT0,   "Takumi (Capcom license)", "Giga Wing (USA 990222)", GAME_SUPPORTS_SAVE )
GAME( 1999, gigawingj,  gigawing, cps2, cps2_2p2b, cps2,     ROT0,   "Takumi (Capcom license)", "Giga Wing (Japan 990223)", GAME_SUPPORTS_SAVE )
GAME( 1999, gigawinga,  gigawing, cps2, cps2_2p2b, cps2,     ROT0,   "Takumi (Capcom license)", "Giga Wing (Asia 990222)", GAME_SUPPORTS_SAVE )
GAME( 1999, gigawingh,  gigawing, cps2, cps2_2p2b, cps2,     ROT0,   "Takumi (Capcom license)", "Giga Wing (Hispanic 990222)", GAME_SUPPORTS_SAVE )
GAME( 1999, gigawingb,  gigawing, cps2, cps2_2p2b, cps2,     ROT0,   "Takumi (Capcom license)", "Giga Wing (Brazil 990222)", GAME_SUPPORTS_SAVE )
GAME( 2000, mmatrix,    0,        cps2, cps2_2p1b, cps2,     ROT0,   "Takumi (Capcom license)", "Mars Matrix: Hyper Solid Shooting (USA 000412)", GAME_SUPPORTS_SAVE )
GAME( 2000, mmatrixj,   mmatrix,  cps2, cps2_2p1b, cps2,     ROT0,   "Takumi (Capcom license)", "Mars Matrix: Hyper Solid Shooting (Japan 000412)", GAME_SUPPORTS_SAVE )

/* Games released on CPS-2 hardware by Mitchell */

GAME( 2000, mpang,      0,        cps2, cps2_2p1b, cps2,     ROT0,   "Mitchell (Capcom license)", "Mighty! Pang (Euro 001010)", GAME_SUPPORTS_SAVE )
GAME( 2000, mpangr1,    mpang,    cps2, cps2_2p1b, cps2,     ROT0,   "Mitchell (Capcom license)", "Mighty! Pang (Euro 000925)", GAME_SUPPORTS_SAVE )
GAME( 2000, mpangu,     mpang,    cps2, cps2_2p1b, cps2,     ROT0,   "Mitchell (Capcom license)", "Mighty! Pang (USA 001010)", GAME_SUPPORTS_SAVE )
GAME( 2000, mpangj,     mpang,    cps2, cps2_2p1b, cps2,     ROT0,   "Mitchell (Capcom license)", "Mighty! Pang (Japan 001011)", GAME_SUPPORTS_SAVE )
GAME( 2001, pzloop2,    0,        cps2, pzloop2,   pzloop2,  ROT0,   "Mitchell (Capcom license)", "Puzz Loop 2 (Euro 010302)", GAME_SUPPORTS_SAVE )
GAME( 2001, pzloop2j,   pzloop2,  cps2, pzloop2,   pzloop2,  ROT0,   "Mitchell (Capcom license)", "Puzz Loop 2 (Japan 010205)", GAME_SUPPORTS_SAVE )
GAME( 2001, choko,      0,        cps2, choko,     cps2,     ROT0,   "Mitchell (Capcom license)", "Janpai Puzzle Choukou (Japan 010820)", GAME_SUPPORTS_SAVE )

/* Games released on CPS-2 hardware by Eighting/Raizing */

GAME( 2000, dimahoo,    0,        cps2, cps2_2p3b, cps2,     ROT270, "Eighting / Raizing (Capcom license)", "Dimahoo (Euro 000121)", GAME_SUPPORTS_SAVE )
GAME( 2000, dimahoou,   dimahoo,  cps2, cps2_2p3b, cps2,     ROT270, "Eighting / Raizing (Capcom license)", "Dimahoo (USA 000121)", GAME_SUPPORTS_SAVE )
GAME( 2000, gmahou,     dimahoo,  cps2, cps2_2p3b, cps2,     ROT270, "Eighting / Raizing (Capcom license)", "Great Mahou Daisakusen (Japan 000121)", GAME_SUPPORTS_SAVE )
GAME( 2000, 1944,       0,        cps2, cps2_2p2b, cps2,     ROT0,   "Eighting / Raizing (Capcom license)", "1944: The Loop Master (USA 000620)", GAME_SUPPORTS_SAVE )
GAME( 2000, 1944j,      1944,     cps2, cps2_2p2b, cps2,     ROT0,   "Eighting / Raizing (Capcom license)", "1944: The Loop Master (Japan 000620)", GAME_SUPPORTS_SAVE )

/* Games released on CPS-2 hardware by Cave */

GAME( 2001, progear,    0,        cps2, cps2_2p3b, cps2,     ROT0,   "Cave (Capcom license)", "Progear (USA 010117)", GAME_SUPPORTS_SAVE )
GAME( 2001, progearj,   progear,  cps2, cps2_2p3b, cps2,     ROT0,   "Cave (Capcom license)", "Progear no Arashi (Japan 010117)", GAME_SUPPORTS_SAVE )
GAME( 2001, progeara,   progear,  cps2, cps2_2p3b, cps2,     ROT0,   "Cave (Capcom license)", "Progear (Asia 010117)", GAME_SUPPORTS_SAVE )

/*
 ------------------------
 Phoenix bootleg sets
 ------------------------

 The Phoenix sets were created by Razoola as a method of allowing the games to run on
 CPS2 boards where the battery had died.  When this happens the boards run non-encrypted
 code, but the memory mapping is changed.  As the original games have encrypted code
 mixed with decrypted data the program roms must be carefully modified in order to
 correctly contain only decrypted code and data, as well as modification to compensate
 for the memory map changes that occur on the dead boards.  Due nature of this process
 there were sometimes errors introduced into the 'Phoenix' sets.

 Unfortunately the 'Phoenix' sets also ended up forming the basis of a mass cps2
 bootlegging operation whereby cheap CPS2 B boards were purchased, the encryption keys
 killed, and the boards converted to more desirable games.  These started off as single
 game bootlegs of in-demand titles, but soon started also forming the basis of xx-in-1
 bootlegs running on heavily customized B-boards.  These are not legitimate Capcom
 products despite appearing to be so.

 These bootlegs are often sold as 'Phoenix Edition' after Razoola's name, 'xx-in-1', or
 simply 'Suicide-Free' to further artificially inflate the price. Buyer Beware!

 All sets are marked as bootleg because they're unauthorized modifications of the
 original Capcom rom data, and were used for bootleg conversions.

 This may not be a complete list of sets, it was taken from MamePlus.  Other sets, and
 further customized bootlegs boards are known to exist.

 ------------------------
 Other bootlegs
 ------------------------

 There is a bootleg of Megaman 2 called 'Gigaman 2' which has SMT roms, and replaces
 the Qsound hardware with an OKI6295 / AD-65 chip.  No known complete dump exists.

*/


ROM_START( 1944d )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "nffud.03", 0x000000, 0x80000, CRC(28e8aae4) SHA1(b2ae11bddbf156cbf38eafdc705067bff9256752) )
	ROM_LOAD16_WORD_SWAP( "nff.04",   0x080000, 0x80000, CRC(dba1c66e) SHA1(4764e77d4da5d19d9acded27df1e1bcba06b0fcf) )
	ROM_LOAD16_WORD_SWAP( "nffu.05",  0x100000, 0x80000, CRC(ea813eb7) SHA1(34e0175a5f22d08c3538369b4bfd077a7427a128) )

	ROM_REGION16_BE( CODE_SIZE, "user1", 0 )
	ROM_FILL( 0x000000, 0x100000, 0 )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "nff.13m",   0x0000000, 0x400000, CRC(c9fca741) SHA1(1781d4fc18b6d6f79b7b39d9bcace750fb61a5cf) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "nff.15m",   0x0000002, 0x400000, CRC(f809d898) SHA1(a0b6af49e1780678d808c317b875161cedddb314) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "nff.17m",   0x0000004, 0x400000, CRC(15ba4507) SHA1(bed6a82bf1dc1aa501d4c2d098115a15e18d446a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "nff.19m",   0x0000006, 0x400000, CRC(3dd41b8c) SHA1(676078baad789e25f6e5a79de29672587be7ff00) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "nff.14m",   0x1000000, 0x100000, CRC(3fe3a54b) SHA1(0a8e5cae141d24fd8b3cb11796c44728b0acd69e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "nff.16m",   0x1000002, 0x100000, CRC(565cd231) SHA1(0aecd433fb4ca2de1aca9fbb1e314fb1f6979321) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "nff.18m",   0x1000004, 0x100000, CRC(63ca5988) SHA1(30137fa77573c84bcc24570bccb7dba61ddb413c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "nff.20m",   0x1000006, 0x100000, CRC(21eb8f3b) SHA1(efa69f19a958047dd91a294c88857ed3133fcbef) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "nff.01",   0x00000, 0x08000, CRC(d2e44318) SHA1(33e45f6fe9fed098a4c072b8c39406aef1a949b2) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "nff.11m",   0x000000, 0x400000, CRC(243e4e05) SHA1(83281f7290ac105a3f9a7507cbc11317d45ba706) )
	ROM_LOAD16_WORD_SWAP( "nff.12m",   0x400000, 0x400000, CRC(4fcf1600) SHA1(36f18c5d92b79433bdf7088b29a244708929d48e) )
ROM_END

ROM_START( 19xxd )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "19xud.03", 0x000000, 0x80000, CRC(f81b60e5) SHA1(6f246ed7779a3a75e7ad7fa8c913c901ea423828) )
	ROM_LOAD16_WORD_SWAP( "19xud.04", 0x080000, 0x80000, CRC(cc44638c) SHA1(a11df36b98bb59cf64c47bab29721e80d64054ff) )
	ROM_LOAD16_WORD_SWAP( "19xud.05", 0x100000, 0x80000, CRC(33a168de) SHA1(11e1977cda6269f1a7d6a0f53b73e3094649bd2f) )
	ROM_LOAD16_WORD_SWAP( "19xud.06", 0x180000, 0x80000, CRC(e0111282) SHA1(233bf939f79e5dd861a01d6d76ae57861d0438fd) )
	ROM_LOAD16_WORD_SWAP( "19x.07",   0x200000, 0x80000, CRC(61c0296c) SHA1(9e225beccffd14bb53a32f8c0f2aef7f331dae30) )

	ROM_REGION16_BE( CODE_SIZE, "user1", 0 )
	ROM_FILL( 0x000000, 0x100000, 0 )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROMX_LOAD( "19x.13m",   0x0000000, 0x080000, CRC(427aeb18) SHA1(901029b5423e4bda85f592735036c06b7d426680) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "19x.15m",   0x0000002, 0x080000, CRC(63bdbf54) SHA1(9beb64ef0a8c92490848599d5d979bf42532609d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "19x.17m",   0x0000004, 0x080000, CRC(2dfe18b5) SHA1(8a44364d9af6b9e1664b44b9235dc172182c9eb8) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "19x.19m",   0x0000006, 0x080000, CRC(cbef9579) SHA1(172413f220b242411218c7865e04014ec6417537) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "19x.14m",   0x0800000, 0x200000, CRC(e916967c) SHA1(3f937022166149a80585f91388de521055ca88ca) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "19x.16m",   0x0800002, 0x200000, CRC(6e75f3db) SHA1(4e1c8466eaa612102d0807d2e8bf1004e97476ea) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "19x.18m",   0x0800004, 0x200000, CRC(2213e798) SHA1(b1a9d5547f3f6c3ab59e8b761d224793c6ca33cb) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "19x.20m",   0x0800006, 0x200000, CRC(ab9d5b96) SHA1(52b755da401fde90c13181b02ab33e5e4b2aa1f7) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "19x.01",   0x00000, 0x08000, CRC(ef55195e) SHA1(813f465f2d392f6abeadbf661c54cf51171fa006) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "19x.11m",   0x000000, 0x200000, CRC(d38beef3) SHA1(134e961b926a97cca5e45d3558efb98f6f278e08) )
	ROM_LOAD16_WORD_SWAP( "19x.12m",   0x200000, 0x200000, CRC(d47c96e2) SHA1(3c1b5563f8e7ee1c450b3592fcb319e928caec3c) )
ROM_END

ROM_START( sfz2ad )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "sz2ad.03a", 0x000000, 0x80000, CRC(017f8fab) SHA1(5546d935c569464c29999914c697c2a171659f42) )
	ROM_LOAD16_WORD_SWAP( "sz2ad.04a", 0x080000, 0x80000, CRC(f50e5ea2) SHA1(ec75aa69bd18cdfac4f5783d9c6c3691bb4914c9) )
	ROM_LOAD16_WORD_SWAP( "sz2.05a",   0x100000, 0x80000, CRC(98e8e992) SHA1(41745b63e6b3888081d189b8315ed3b7526b3d20) )
	ROM_LOAD16_WORD_SWAP( "sz2.06",   0x180000, 0x80000, CRC(5b1d49c0) SHA1(f0a0c894c9cbe2b18e7f59058665949ee0025732) )
	ROM_LOAD16_WORD_SWAP( "sz2a.07a", 0x200000, 0x80000, CRC(0aed2494) SHA1(7beb1a394f17cd78a27128292b626aae28754ca2) )
	ROM_LOAD16_WORD_SWAP( "sz2.08",   0x280000, 0x80000, CRC(0fe8585d) SHA1(0cd5369a5aa90c98d8dc1ff3342cd4d990631cff) )

	ROM_REGION( 0x1400000, "gfx", 0 )
	ROMX_LOAD( "sz2.13m",   0x0000000, 0x400000, CRC(4d1f1f22) SHA1(659fb4305bcf0cbbbbec97ede6e68a8323b13308) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.15m",   0x0000002, 0x400000, CRC(19cea680) SHA1(4cb88963a0fbcef191c8419b6379387c01b4c81e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.17m",   0x0000004, 0x400000, CRC(e01b4588) SHA1(c2936608fd75ff6cd5fa94c6d6d6f0c77c44a450) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.19m",   0x0000006, 0x400000, CRC(0feeda64) SHA1(f5b350601437bd94b70d97feb23d791df19da6b3) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.14m",   0x1000000, 0x100000, CRC(0560c6aa) SHA1(f2bed3a8efef18052b51a7f0f6a888a18db813a1) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.16m",   0x1000002, 0x100000, CRC(ae940f87) SHA1(39ee26333abbe302ba76dced0196a2e6b3b1d02a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.18m",   0x1000004, 0x100000, CRC(4bc3c8bc) SHA1(6256963c515bf56f39b6e559afefd653ead56c54) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.20m",   0x1000006, 0x100000, CRC(39e674c0) SHA1(8e771a2d8c2accad0463bccd21d7b23af0c895a1) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sz2.01a",   0x00000, 0x08000, CRC(1bc323cf) SHA1(83fbd6e9b327700dc9f1c59700b7385bc3705749) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "sz2.02a",   0x28000, 0x20000, CRC(ba6a5013) SHA1(7814f3e56b69529b9860dd61c3b1e8d700244b03) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "sz2.11m",   0x000000, 0x200000, CRC(aa47a601) SHA1(a4d1ee89c84a3b9db06469bb66e85293b5aa9ac9) )
	ROM_LOAD16_WORD_SWAP( "sz2.12m",   0x200000, 0x200000, CRC(2237bc53) SHA1(96d5693047e4cf1ed10a8ee1905cea267a278e92) )
ROM_END

ROM_START( avspd )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "avped.03d", 0x000000, 0x80000, CRC(66aa8aad) SHA1(eb1928393d0dd4cc1a96c00324508f83f36a7622) )
	ROM_LOAD16_WORD_SWAP( "avped.04d", 0x080000, 0x80000, CRC(579306c2) SHA1(cabee3fdb624e681013a5a57d2a37339b96518fb) )
	ROM_LOAD16_WORD_SWAP( "avp.05d",   0x100000, 0x80000, CRC(fbfb5d7a) SHA1(5549bc9d780753bc9c10fba82588e5c3d4a2acb2) )
	ROM_LOAD16_WORD_SWAP( "avpd.06",   0x180000, 0x80000, CRC(63094539) SHA1(f1b776cf4334fa7fa1ee0e5ce81a5996b930996b) )

	ROM_REGION16_BE( CODE_SIZE, "user1", 0 )
	ROM_FILL( 0x000000, 0x100000, 0 )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROMX_LOAD( "avp.13m",   0x0000000, 0x200000, CRC(8f8b5ae4) SHA1(457ce959aa5db3a003de7dda2b3799b2f1ae279b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "avp.15m",   0x0000002, 0x200000, CRC(b00280df) SHA1(bc1291a4a222d410bc99b6f1ed392067d9c3999e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "avp.17m",   0x0000004, 0x200000, CRC(94403195) SHA1(efaad001527a5eba8f626aea9037ac6ef9a2c295) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "avp.19m",   0x0000006, 0x200000, CRC(e1981245) SHA1(809ccb7f10262e227d5e9d9f710e06f0e751f550) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "avp.14m",   0x0800000, 0x200000, CRC(ebba093e) SHA1(77aaf4197d1dae3321cf9c6d2b7967ee54cf3f30) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "avp.16m",   0x0800002, 0x200000, CRC(fb228297) SHA1(ebd02a4ba085dc70c0603662e14d61625fa04648) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "avp.18m",   0x0800004, 0x200000, CRC(34fb7232) SHA1(8b1f15bfa758a61e6ad519af24ca774edc70d194) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "avp.20m",   0x0800006, 0x200000, CRC(f90baa21) SHA1(20a900819a9d321316e3dfd241210725d7191ecf) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "avp.01",   0x00000, 0x08000, CRC(2d3b4220) SHA1(2b2d04d4282550fa9f6e1ad8528f20d1f2ac02eb) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "avp.11m",   0x000000, 0x200000, CRC(83499817) SHA1(e65b0ebd61ddc748842a9d4d92404b5305307623) )
	ROM_LOAD16_WORD_SWAP( "avp.12m",   0x200000, 0x200000, CRC(f4110d49) SHA1(f27538776cc1ba8213f19f98728ed8c02508d3ac) )
ROM_END

ROM_START( batcird )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "btced.03", 0x000000, 0x80000, CRC(0737db6d) SHA1(e7f02f568a8013d5abf32e0e7dc120c004adb6c9) )
	ROM_LOAD16_WORD_SWAP( "btced.04", 0x080000, 0x80000, CRC(ef1a8823) SHA1(9326e031b937af18b98dc3f236caaac632dac66d) )
	ROM_LOAD16_WORD_SWAP( "btced.05", 0x100000, 0x80000, CRC(20bdbb14) SHA1(fe3a202741ca657b2b67e89050788b67d709a36d) )
	ROM_LOAD16_WORD_SWAP( "btced.06", 0x180000, 0x80000, CRC(b4d8f5bc) SHA1(dc5ca580ecfb051ded551663ea4e9f161f820f81) )
	ROM_LOAD16_WORD_SWAP( "btc.07",   0x200000, 0x80000, CRC(7322d5db) SHA1(473be1f1bf603bdd82451661a6206507f50ed2b6) )
	ROM_LOAD16_WORD_SWAP( "btc.08",   0x280000, 0x80000, CRC(6aac85ab) SHA1(ad02d4185c2b3664fb96350d8ad317d3939a7554) )
	ROM_LOAD16_WORD_SWAP( "btc.09",   0x300000, 0x80000, CRC(1203db08) SHA1(fdbea14618b277132f9e010ef36c134a8ea42162) )

	ROM_REGION16_BE( CODE_SIZE, "user1", 0 )
	ROM_FILL( 0x000000, 0x100000, 0 )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROMX_LOAD( "btc.13m",   0x000000, 0x400000, CRC(dc705bad) SHA1(96e37147674bf9cd21c770897da59daac25d921a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "btc.15m",   0x000002, 0x400000, CRC(e5779a3c) SHA1(bbd7fbe061e751388d2f02434144daf9b1e36640) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "btc.17m",   0x000004, 0x400000, CRC(b33f4112) SHA1(e501fd921c8bcede69946b029e05d422714c1040) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "btc.19m",   0x000006, 0x400000, CRC(a6fcdb7e) SHA1(7a28d5d7aa036d23d97fad17d0cdb8210dc8153a) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "btc.01",   0x00000, 0x08000, CRC(1e194310) SHA1(3b29de0aca9dbca59d6b50fb2509e2a913c6b0af) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "btc.02",   0x28000, 0x20000, CRC(01aeb8e6) SHA1(50a5d1cce0caf7c5143d4904431e8f41e2a57464) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "btc.11m",   0x000000, 0x200000, CRC(c27f2229) SHA1(df2459493af40937b6656a16fad43ff51bed2204) )
	ROM_LOAD16_WORD_SWAP( "btc.12m",   0x200000, 0x200000, CRC(418a2e33) SHA1(0642ddff2ab9255f154419da24ba644ed63f34ab) )
ROM_END

ROM_START( ddtodd )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "daded.03c", 0x000000, 0x80000, CRC(843330f4) SHA1(3f77876ac5d61b595c22697724a16034b16954af) )
	ROM_LOAD16_WORD_SWAP( "daded.04c", 0x080000, 0x80000, CRC(306f14fc) SHA1(b684ee72acc3087ada20b2d13202366c3cff1014) )
	ROM_LOAD16_WORD_SWAP( "daded.05c", 0x100000, 0x80000, CRC(8c6b8328) SHA1(ab5d2c608bd3cdb298a9743d819616bf2df02ddd) )
	ROM_LOAD16_WORD_SWAP( "dad.06a",   0x180000, 0x80000, CRC(6225495a) SHA1(a9a02abb072e3482ac92d7aed8ce9a5bcf636bc0) )
	ROM_LOAD16_WORD_SWAP( "dadd.07a",  0x200000, 0x80000, CRC(0f0df6cc) SHA1(5da5f989ed71faf5e2950fdf9650d94918616ae4) )

	ROM_REGION16_BE( CODE_SIZE, "user1", 0 )
	ROM_FILL( 0x000000, 0x100000, 0 )

	ROM_REGION( 0xc00000, "gfx", 0 )
	ROMX_LOAD( "dad.13m",   0x000000, 0x200000, CRC(da3cb7d6) SHA1(d59bb53d5f32889eb6eb7f8b1c8781948c97283d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.15m",   0x000002, 0x200000, CRC(92b63172) SHA1(9bed7dbbb17729f2ad3d318396f5335c0bd39937) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.17m",   0x000004, 0x200000, CRC(b98757f5) SHA1(3eead22e097906bf0e1e151cd0a9c75abc5a32d4) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.19m",   0x000006, 0x200000, CRC(8121ce46) SHA1(40c4dc969318d38f0c6d5401c9c64371f51aa12c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.14m",   0x800000, 0x100000, CRC(837e6f3f) SHA1(c060183474fba0e82d765b9f282b84838550dff6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.16m",   0x800002, 0x100000, CRC(f0916bdb) SHA1(9354d258dd26cbbf12c78ecfc277c357cbdb360e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.18m",   0x800004, 0x100000, CRC(cef393ef) SHA1(830b33c86cc24776d17ad65fa89a3b16c40446a1) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dad.20m",   0x800006, 0x100000, CRC(8953fe9e) SHA1(f4795beb006335d13e3934aa9760e775eb0bb950) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "dad.01",   0x00000, 0x08000, CRC(3f5e2424) SHA1(4aa744576bc6752c43a90a27a816ebd90076b248) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "dad.11m",   0x000000, 0x200000, CRC(0c499b67) SHA1(a8ebd8a1cd6dece8344b7cb0439d85843fb97616) )
	ROM_LOAD16_WORD_SWAP( "dad.12m",   0x200000, 0x200000, CRC(2f0b5a4e) SHA1(8d1ebbb811aa469b0f0d29d719d2b9af28fb63a2) )
ROM_END

ROM_START( ddsomud )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "dd2ud.03g", 0x000000, 0x80000, CRC(816f695a) SHA1(5e590d36c04ceac1b48954c0554a733b4c3a5277) )
	ROM_LOAD16_WORD_SWAP( "dd2ud.04g", 0x080000, 0x80000, CRC(7cc81c6b) SHA1(2619cb25d5ae62adc5c2292e25c0d33674f5cc13) )
	ROM_LOAD16_WORD_SWAP( "dd2.05g",   0x100000, 0x80000, CRC(5eb1991c) SHA1(429a60b5396ff4192904867fbe0524268f0edbcb) )
	ROM_LOAD16_WORD_SWAP( "dd2.06g",   0x180000, 0x80000, CRC(c26b5e55) SHA1(9590206f30459941880ff4b56c7f276cc78e3a22) )
	ROM_LOAD16_WORD_SWAP( "dd2.07",    0x200000, 0x80000, CRC(909a0b8b) SHA1(58bda17c36063a79df8b5031755c7909a9bda221) )
	ROM_LOAD16_WORD_SWAP( "dd2.08",    0x280000, 0x80000, CRC(e53c4d01) SHA1(bad872e4e793a39f68bc0e580772e982714b5876) )
	ROM_LOAD16_WORD_SWAP( "dd2.09",    0x300000, 0x80000, CRC(5f86279f) SHA1(c2a454e5f821b1cdd49f2cf0602e9bfb7ba63340) )
	ROM_LOAD16_WORD_SWAP( "dd2d.10",   0x380000, 0x80000, CRC(0c172f8f) SHA1(4f0ad9ab401f9f2d7d8f605a5ef78add4f4ced38) )

	ROM_REGION16_BE( CODE_SIZE, "user1", 0 )
	ROM_FILL( 0x000000, 0x100000, 0 )

	ROM_REGION( 0x1800000, "gfx", 0 )
	ROMX_LOAD( "dd2.13m",   0x0000000, 0x400000, CRC(a46b4e6e) SHA1(fb90f42868c581c481b4ceff9f692753fb186b30) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.15m",   0x0000002, 0x400000, CRC(d5fc50fc) SHA1(bc692f17b18bb47a724cd5152377cd5ccd6e184a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.17m",   0x0000004, 0x400000, CRC(837c0867) SHA1(3d6db290a8f76299a23543f0ccf6a7905e1088ac) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.19m",   0x0000006, 0x400000, CRC(bb0ec21c) SHA1(e43ccc1cf63ccd2b504cc9fd701af849a7321914) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.14m",   0x1000000, 0x200000, CRC(6d824ce2) SHA1(0ccfe6c8a944937718e28a1a373b5822c7b7001b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.16m",   0x1000002, 0x200000, CRC(79682ae5) SHA1(ee84f4791c29ce9e2bae06ba3ec47ff4d2cd7054) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.18m",   0x1000004, 0x200000, CRC(acddd149) SHA1(7f50de9b2d1cc733594c642be1804190519caffa) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "dd2.20m",   0x1000006, 0x200000, CRC(117fb0c0) SHA1(15c01fa1a71b6469b0e1bde0ce5835c5ff9d938c) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "dd2.01",   0x00000, 0x08000, CRC(99d657e5) SHA1(1528dd6b07a0e79951a35c0457c8a9c9770e9c78) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "dd2.02",   0x28000, 0x20000, CRC(117a3824) SHA1(14f3a12170b601c5466c93af9d2f24e0b386b4e4) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "dd2.11m",   0x000000, 0x200000, CRC(98d0c325) SHA1(7406e8d943d77c468eb418c4113261f4ab973bbf) )
	ROM_LOAD16_WORD_SWAP( "dd2.12m",   0x200000, 0x200000, CRC(5ea2e7fa) SHA1(0e6a9fd007f637adcb2226c902394f07de45e803) )
ROM_END

ROM_START( gwingjd )
	ROM_REGION(CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "ggwjd.03a", 0x000000, 0x80000, CRC(cb1c756e) SHA1(f8b37120b429e14aad6e7d2106f8f8c422f2ce2b) )
	ROM_LOAD16_WORD_SWAP( "ggwjd.04a", 0x080000, 0x80000, CRC(fa158e04) SHA1(bd0f0351fabe376944c28e327bcf83a8d9229441) )
	ROM_LOAD16_WORD_SWAP( "ggwjd.05a", 0x100000, 0x80000, CRC(1c5bc4e7) SHA1(7f338cb45686b24b9136dd1e575fd842f9fd0b05) )

	ROM_REGION16_BE( CODE_SIZE, "user1", 0 )
	ROM_FILL( 0x000000, 0x100000, 0 )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROMX_LOAD( "ggw.13m",   0x000000, 0x400000, CRC(105530a4) SHA1(3be06c032985ea6bd3805d73a407bf748385087b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ggw.15m",   0x000002, 0x400000, CRC(9e774ab9) SHA1(adea1e844f3d9ccd5ad116ff8277f16a96e68d76) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ggw.17m",   0x000004, 0x400000, CRC(466e0ba4) SHA1(9563455b95d36fafe508290659088b153539cfdf) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ggw.19m",   0x000006, 0x400000, CRC(840c8dea) SHA1(ea04afce17f00b45d3d2cd5140d0dd7ab4bccc00) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION(QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ggw.01",   0x00000, 0x08000, CRC(4c6351d5) SHA1(cef81fb7c4b8cb2ef1f8f3c27982aefbcbe38160) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "ggw.11m",   0x000000, 0x400000, CRC(e172acf5) SHA1(d7b0963d66165f3607d887741c5e7ab952bcf2ff) )
	ROM_LOAD16_WORD_SWAP( "ggw.12m",   0x400000, 0x400000, CRC(4bee4e8f) SHA1(c440b5a38359ec3b8002f39690b79bf78703f5d0) )
ROM_END

ROM_START( hsf2d )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "hs2ad.03", 0x000000, 0x80000, CRC(0153d371) SHA1(137f89b4ca41346abd8d1ef4a17605f6622b741e) )
	ROM_LOAD16_WORD_SWAP( "hs2ad.04", 0x080000, 0x80000, CRC(0276b78a) SHA1(3f5502f77eb9889ca4658eb323579e05b35c9868) )
	ROM_LOAD16_WORD_SWAP( "hs2.05",   0x100000, 0x80000, CRC(dde34a35) SHA1(f5be2d2916db6e86e0886d61d55bddf138273ebc) )
	ROM_LOAD16_WORD_SWAP( "hs2.06",   0x180000, 0x80000, CRC(f4e56dda) SHA1(c6490707c2a416ab88612c2d73abbe5853d8cb92) )
	ROM_LOAD16_WORD_SWAP( "hs2.07",   0x200000, 0x80000, CRC(ee4420fc) SHA1(06cf76660b0c794d2460c52d9fe8334fff51e9de) )
	ROM_LOAD16_WORD_SWAP( "hs2.08",   0x280000, 0x80000, CRC(c9441533) SHA1(bf178fac1f060fcce3ff9118333c8517dadc9429) )
	ROM_LOAD16_WORD_SWAP( "hs2.09",   0x300000, 0x80000, CRC(3fc638a8) SHA1(2a42877b26c8abc437da46225701f0bba6e40058) )
	ROM_LOAD16_WORD_SWAP( "hs2.10",   0x380000, 0x80000, CRC(20d0f9e4) SHA1(80a5eeef9472e327b0d4ee26434bad109a9434ea) )

	ROM_REGION16_BE( CODE_SIZE, "user1", 0 )
	ROM_FILL( 0x000000, 0x100000, 0 )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "hs2.13m",   0x0000000, 0x800000, CRC(a6ecab17) SHA1(6749a4c8dc81f4b10f910c31c82cf6674e2a44eb) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "hs2.15m",   0x0000002, 0x800000, CRC(10a0ae4d) SHA1(701b4900fbc8bef20efa1a706891c8df4bf14641) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "hs2.17m",   0x0000004, 0x800000, CRC(adfa7726) SHA1(8d36ec125a8c91abfe5213893d794f8bc11c8acd) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "hs2.19m",   0x0000006, 0x800000, CRC(bb3ae322) SHA1(ecd289d7a0fe365fdd7c5527cb17796002beb553) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "hs2.01",   0x00000, 0x08000, CRC(c1a13786) SHA1(c7392c7efb15ea4042e75bd9007e974293d8935d) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "hs2.02",   0x28000, 0x20000, CRC(2d8794aa) SHA1(c634affdc2568020cce6af97b4fa79925d9943f3) )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "hs2.11m",   0x000000, 0x800000, CRC(0e15c359) SHA1(176108b0d76d821a849324680aba0cd04b5016c1) )
ROM_END

ROM_START( megamn2d )
	ROM_REGION(CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "rm2ud.03", 0x000000, 0x80000, CRC(d3635f25) SHA1(69a5a44bdc040754efa7fb96a267f8dcc2c5a23f) )
	ROM_LOAD16_WORD_SWAP( "rm2ud.04", 0x080000, 0x80000, CRC(768a1705) SHA1(9b0f206ef15d72c9d0cb496845353ce7fdf2d25e) )
	ROM_LOAD16_WORD_SWAP( "rm2.05",  0x100000, 0x80000, CRC(02ee9efc) SHA1(1b80c40389b51a03b930051f232630616c12e6c5) )

	ROM_REGION16_BE( CODE_SIZE, "user1", 0 )
	ROM_FILL( 0x000000, 0x100000, 0 )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_FILL(               0x000000, 0x800000, 0 )
	ROMX_LOAD( "rm2.14m",   0x800000, 0x200000, CRC(9b1f00b4) SHA1(c1c5c2d9d00121425ae6598444d704f420ef4eef) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rm2.16m",   0x800002, 0x200000, CRC(c2bb0c24) SHA1(38724c49d9db49765a4ed9bc2dc8f57cec45ec7c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rm2.18m",   0x800004, 0x200000, CRC(12257251) SHA1(20cb58afda0e6200991277817485340a6a41ae2b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "rm2.20m",   0x800006, 0x200000, CRC(f9b6e786) SHA1(aeb4acff7208e66a35198143fd2478039fdaa3a6) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION(QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "rm2.01a",  0x00000, 0x08000, CRC(d18e7859) SHA1(0939fac70042d0b4db5c2fdcef1f79b95febd45e) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "rm2.02",   0x28000, 0x20000, CRC(c463ece0) SHA1(5c3e41eb61610b3f8c431206f6672907e3a0bdb0) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "rm2.11m",   0x000000, 0x200000, CRC(2106174d) SHA1(0a35d9ca8ebcad74904b20648d5320f839d6377e) )
	ROM_LOAD16_WORD_SWAP( "rm2.12m",   0x200000, 0x200000, CRC(546c1636) SHA1(f96b172ab899f2c6ee17a5dd1fb61af9432e3cd2) )
ROM_END

ROM_START( mvscud )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "mvcud.03d", 0x000000, 0x80000, CRC(75cde3e5) SHA1(5453056da68cdd4675cb585a9c6ae85e073193f5) )
	ROM_LOAD16_WORD_SWAP( "mvcud.04d", 0x080000, 0x80000, CRC(b32ea484) SHA1(742b35a45eadea3457bfb93808cbba599e9744e3) )
	ROM_LOAD16_WORD_SWAP( "mvc.05a",   0x100000, 0x80000, CRC(2d8c8e86) SHA1(b07d640a734c5d336054ed05195786224c9a6cd4) )
	ROM_LOAD16_WORD_SWAP( "mvc.06a",   0x180000, 0x80000, CRC(8528e1f5) SHA1(cd065c05268ab581b05676da544baf6af642acac) )
	ROM_LOAD16_WORD_SWAP( "mvcd.07",   0x200000, 0x80000, CRC(205293e9) SHA1(b47a7057abd18d85ff9d86483ecb7ab783b3e791) )
	ROM_LOAD16_WORD_SWAP( "mvc.08",    0x280000, 0x80000, CRC(bc002fcd) SHA1(0b6735a071a9274f7ab25c743271fc30411fe819) )
	ROM_LOAD16_WORD_SWAP( "mvc.09",    0x300000, 0x80000, CRC(c67b26df) SHA1(6e9969246c57269d7ba0992a5cc319c8910bf8a9) )
	ROM_LOAD16_WORD_SWAP( "mvc.10",    0x380000, 0x80000, CRC(0fdd1e26) SHA1(5fa684d823b4f4eec61ed9e9b8938af5272ae1ed) )

	ROM_REGION16_BE( CODE_SIZE, "user1", 0 )
	ROM_FILL( 0x000000, 0x100000, 0 )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "mvc.13m",   0x0000000, 0x400000, CRC(fa5f74bc) SHA1(79a619248938a85ce4f7794a704647b9cf564fbc) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.15m",   0x0000002, 0x400000, CRC(71938a8f) SHA1(6982f7203458c1c46a1c1c13c0d0f2a5e109d271) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.17m",   0x0000004, 0x400000, CRC(92741d07) SHA1(ddfd70eab7c983ab452194b1860059f8ad694459) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.19m",   0x0000006, 0x400000, CRC(bcb72fc6) SHA1(46ab98dcdf6f5d611646a22a7355939ef5b2bbe5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.14m",   0x1000000, 0x400000, CRC(7f1df4e4) SHA1(ede92b31c1fe87f91b4fe74ac211f2fb5f863bc2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.16m",   0x1000002, 0x400000, CRC(90bd3203) SHA1(ed83208c486ea0f407b7e5d16a8cf242a6f73774) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.18m",   0x1000004, 0x400000, CRC(67aaf727) SHA1(e0e69104e31d2c41e18c0d24e9ab962406a7ca9a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mvc.20m",   0x1000006, 0x400000, CRC(8b0bade8) SHA1(c5732361bb4bf284c4d12a82ac2c5750b1f9d441) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "mvc.01",   0x00000, 0x08000, CRC(41629e95) SHA1(36925c05b5fdcbe43283a882d021e5360c947061) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "mvc.02",   0x28000, 0x20000, CRC(963abf6b) SHA1(6b784870e338701cefabbbe4669984b5c4e8a9a5) )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "mvc.11m",   0x000000, 0x400000, CRC(850fe663) SHA1(81e519d05a08855f242ea2e17ee0859b449db895) )
	ROM_LOAD16_WORD_SWAP( "mvc.12m",   0x400000, 0x400000, CRC(7ccb1896) SHA1(74caadf3282fcc6acffb1bbe3734106f81124121) )
ROM_END

ROM_START( nwarrud )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "vphud.03f", 0x000000, 0x80000, CRC(20d4d5a8) SHA1(420396c77a9f77feaba4be2e0faaab995842fb57) )
	ROM_LOAD16_WORD_SWAP( "vphud.04c", 0x080000, 0x80000, CRC(61be9b42) SHA1(679381f279f51f28fdc4c4dbefabe6139855303b) )
	ROM_LOAD16_WORD_SWAP( "vphud.05e", 0x100000, 0x80000, CRC(1ba906d8) SHA1(9e1db8af4070c68f25b8fb898e9d29a97f775e4c) )
	ROM_LOAD16_WORD_SWAP( "vphu.06c",  0x180000, 0x80000, CRC(08c04cdb) SHA1(b78d87631a13c26cc1580d2ecc0d137105c23f0a) )
	ROM_LOAD16_WORD_SWAP( "vphu.07b",  0x200000, 0x80000, CRC(b5a5ab19) SHA1(f7b35b8cba81f88a6bdfea7e2dc12eca480c276c) )
	ROM_LOAD16_WORD_SWAP( "vphu.08b",  0x280000, 0x80000, CRC(51bb20fb) SHA1(a98c569dd45b4bd2275f9bd1df060d6eaead53df) )
	ROM_LOAD16_WORD_SWAP( "vphu.09b",  0x300000, 0x80000, CRC(41a64205) SHA1(1f5af658b7c3fb09cab3dd10d6dc433a0605f81a) )
	ROM_LOAD16_WORD_SWAP( "vphud.10b", 0x380000, 0x80000, CRC(9619adad) SHA1(abbbe28659c031f34be23a38950d9b56f7f7ca86) )

	ROM_REGION16_BE( CODE_SIZE, "user1", 0 )
	ROM_FILL( 0x000000, 0x100000, 0 )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "vph.13m",   0x0000000, 0x400000, CRC(c51baf99) SHA1(2fb6642908e542e404391eb17392f8270e87bf48) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.15m",   0x0000002, 0x400000, CRC(3ce83c77) SHA1(93369b23c6d7d834297434691bb047ee3dd9539c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.17m",   0x0000004, 0x400000, CRC(4f2408e0) SHA1(cd49c6b3c7e6470c6058f98ccc5210b052bb13e2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.19m",   0x0000006, 0x400000, CRC(9ff60250) SHA1(d69ba4dc6bd37d003245f0cf3211d6e2623005b8) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.14m",   0x1000000, 0x400000, CRC(7a0e1add) SHA1(6b28a91bd59bba97886fdea30116a5b1071109ed) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.16m",   0x1000002, 0x400000, CRC(2f41ca75) SHA1(f4a67e60b62001e6fe75cb05b9c81040a8a09f54) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.18m",   0x1000004, 0x400000, CRC(64498eed) SHA1(d64e54a9ad1cbb927b7bac2eb16e1487834c5706) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vph.20m",   0x1000006, 0x400000, CRC(17f2433f) SHA1(0cbf8c96f92016fefb4a9c668ce5fd260342d712) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "vph.01",   0x00000, 0x08000, CRC(5045dcac) SHA1(fd1a6586fbdd48a707df1fa52309b4cf50e3cc4c) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "vph.02",   0x28000, 0x20000, CRC(86b60e59) SHA1(197d07ced8b9850729c83fa59b7afc283500bdee) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "vph.11m",   0x000000, 0x200000, CRC(e1837d33) SHA1(e3cb69f64767bacbec7286d0b4cd0ce7a0ba13d8) )
	ROM_LOAD16_WORD_SWAP( "vph.12m",   0x200000, 0x200000, CRC(fbd3cd90) SHA1(4813c25802ad71b77ca04fd8f3a86344f99f0d6a) )
ROM_END

ROM_START( ringdstd )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "smbed.03b", 0x000000, 0x80000, CRC(f6fba4cd) SHA1(aecea7a4c3ed2153017b465b075c7558d914cdc8) )
	ROM_LOAD16_WORD_SWAP( "smbed.04b", 0x080000, 0x80000, CRC(193bc493) SHA1(d2c6af56d3514c6d392aa3f2ea4778bbe5ef513f) )
	ROM_LOAD16_WORD_SWAP( "smbed.05b", 0x100000, 0x80000, CRC(168cccbb) SHA1(c3e1695e013f1197f7093aa499482ff8d7f75b5b) )
	ROM_LOAD16_WORD_SWAP( "smbed.06b", 0x180000, 0x80000, CRC(04673262) SHA1(6ebdfb2adfa0039f6db4d1e81e9ea0a692be4fdc) )
	ROM_LOAD16_WORD_SWAP( "smb.07",    0x200000, 0x80000, CRC(b9a11577) SHA1(e9b58ef8acd1fedd3c9e0a3489593c7e931106c0) )
	ROM_LOAD16_WORD_SWAP( "smb.08",    0x280000, 0x80000, CRC(f931b76b) SHA1(0b7e8d1278dcba89f0063bd09cda96d6ae1bc282) )

	ROM_REGION16_BE( CODE_SIZE, "user1", 0 )
	ROM_FILL( 0x000000, 0x100000, 0 )

	ROM_REGION( 0x1200000, "gfx", 0 )
	ROMX_LOAD( "smb.13m",   0x0000000, 0x200000, CRC(d9b2d1de) SHA1(e8658983070dadcd1300a680a42c8431579e2b4f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "smb.15m",   0x0000002, 0x200000, CRC(9a766d92) SHA1(afdf88afbec527268d63c11ea32f861b52e11489) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "smb.17m",   0x0000004, 0x200000, CRC(51800f0f) SHA1(9526cd69a23340a81841271b51de03d9bf2b979f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "smb.19m",   0x0000006, 0x200000, CRC(35757e96) SHA1(c915f3b9e4fdec3defc7eecb2c1f7377e6072228) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "smb.14m",   0x0800000, 0x200000, CRC(e5bfd0e7) SHA1(327e626df4c2152f921f15535c01dda6c4437527) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "smb.16m",   0x0800002, 0x200000, CRC(c56c0866) SHA1(1e2218e852ae72a9a95861dd37129fe78d4b1329) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "smb.18m",   0x0800004, 0x200000, CRC(4ded3910) SHA1(d883541ce4d83f4e7ab095f2ef273408d9911f9a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "smb.20m",   0x0800006, 0x200000, CRC(26ea1ec5) SHA1(22be249b1f73272feacf4026f09fc877f5d86353) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "smb.21m",   0x1000000, 0x080000, CRC(0a08c5fc) SHA1(ff3fad4fbc98e3013291c7ba7ee5e057a2628b36) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "smb.23m",   0x1000002, 0x080000, CRC(0911b6c4) SHA1(e7a7061b192658724d98cae8693f63dd5bc40c00) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "smb.25m",   0x1000004, 0x080000, CRC(82d6c4ec) SHA1(ed8ed02a00f59a048b9891ec2a77720bb6a5e03d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "smb.27m",   0x1000006, 0x080000, CRC(9b48678b) SHA1(4fa300d356c538947983ae85bb5c5cfd1fb835e7) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "smb.01",   0x00000, 0x08000, CRC(0abc229a) SHA1(967f574e6358dfc1b01e6a4a4df1a8f34eb3d814) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "smb.02",   0x28000, 0x20000, CRC(d051679a) SHA1(583c2521a30db1740d95dd94a38751fbeff3aae5) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "smb.11m",   0x000000, 0x200000, CRC(c56935f9) SHA1(ca1705e48e31ddc13505e6297bceca2bec1bb209) )
	ROM_LOAD16_WORD_SWAP( "smb.12m",   0x200000, 0x200000, CRC(955b0782) SHA1(ee09500e7b44e923126533613bfe26cdabc7ab5f) )
ROM_END

ROM_START( sfad )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "sfzed.03d", 0x000000, 0x80000, CRC(a1a54827) SHA1(85dd1a6f807af56b63cd2880e6e32794eb11c61e) )
	ROM_LOAD16_WORD_SWAP( "sfz.04b",   0x080000, 0x80000, CRC(8b73b0e5) SHA1(5318761f615c21395366b5333e75eaaa73ef2073) )
	ROM_LOAD16_WORD_SWAP( "sfz.05a",   0x100000, 0x80000, CRC(0810544d) SHA1(5f39bda3e7b16508eb58e5a2e0cc58c09cf428ce) )
	ROM_LOAD16_WORD_SWAP( "sfz.06",    0x180000, 0x80000, CRC(806e8f38) SHA1(b6d6912aa8f2f590335d7ff9a8214648e7131ebb) )

	ROM_REGION16_BE( CODE_SIZE, "user1", 0 )
	ROM_FILL( 0x000000, 0x100000, 0 )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROM_FILL(               0x000000, 0x800000, 0 )
	ROMX_LOAD( "sfz.14m",   0x800000, 0x200000, CRC(90fefdb3) SHA1(5eb28c8de57acfeaefebdd01509c7d9ba5244705) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz.16m",   0x800002, 0x200000, CRC(5354c948) SHA1(07588f1ba6addc04fef3274c971174aaf3e632ab) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz.18m",   0x800004, 0x200000, CRC(41a1e790) SHA1(ce25dad542308691dbe9606b26279bbd59ea4b81) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfz.20m",   0x800006, 0x200000, CRC(a549df98) SHA1(f054e95df650a891ef56da8bfb31cb2c945a9aed) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sfz.01",   0x00000, 0x08000, CRC(ffffec7d) SHA1(75b4aef001b72a0f571b51b2b97803facc1832dd) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "sfz.02",   0x28000, 0x20000, CRC(45f46a08) SHA1(e32dbd27b52ab708278045b5a829376e55a4ca81) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "sfz.11m",  0x000000, 0x200000, CRC(c4b093cd) SHA1(256526bb693a0b72443f047e060304c9b739acd1) )
	ROM_LOAD16_WORD_SWAP( "sfz.12m",  0x200000, 0x200000, CRC(8bdbc4b4) SHA1(0e21c9a75a17a7e7dfd8bb51098c2b9dc4c933ec) )
ROM_END


ROM_START( mshud )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "mshud.03",0x000000, 0x80000, CRC(c1d8c4c6) SHA1(9d367d23a438fd22edce58ce814a4b3b36e5fc2a) )
	ROM_LOAD16_WORD_SWAP( "mshud.04",0x080000, 0x80000, CRC(e73dda16) SHA1(f86cd74bdfa82bf0249770694de9419ffc3d3f63) )
	ROM_LOAD16_WORD_SWAP( "mshud.05",0x100000, 0x80000, CRC(3b493e84) SHA1(875e616270e839218c924e09627bcf79211ee694) )
	ROM_LOAD16_WORD_SWAP( "msh.06b", 0x180000, 0x80000, CRC(803e3fa4) SHA1(0acdeda65002521bf24130cbf06f9faa1dcef9e5) )
	ROM_LOAD16_WORD_SWAP( "msh.07a", 0x200000, 0x80000, CRC(c45f8e27) SHA1(4d28e0782c31ce56e728ac6ef5edd10437f00637) )
	ROM_LOAD16_WORD_SWAP( "msh.08a", 0x280000, 0x80000, CRC(9ca6f12c) SHA1(26ad682667b983b805e1f577426e5fca8ee3c82b) )
	ROM_LOAD16_WORD_SWAP( "msh.09a", 0x300000, 0x80000, CRC(82ec27af) SHA1(caf76268063ba91d28e8af684d60c2d71f29b9b9) )
	ROM_LOAD16_WORD_SWAP( "msh.10b", 0x380000, 0x80000, CRC(8d931196) SHA1(983e62efcdb4c8db6bce6acf4f86acb9447b565d) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "msh.13m",   0x0000000, 0x400000, CRC(09d14566) SHA1(c96463654043f22da5e844c6da17aa9273dc3439) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.15m",   0x0000002, 0x400000, CRC(ee962057) SHA1(24e359accb5f71a5863d7bad4088719fa547f88c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.17m",   0x0000004, 0x400000, CRC(604ece14) SHA1(880fb62b33ba4cceb38635e4ec056fac11a3c70f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.19m",   0x0000006, 0x400000, CRC(94a731e8) SHA1(1e784a3412e7361e3001494e1daf840ef8c20449) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.14m",   0x1000000, 0x400000, CRC(4197973e) SHA1(93aeea1a480b5f452c8a40ae3fff956796b859fa) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.16m",   0x1000002, 0x400000, CRC(438da4a0) SHA1(ca93b14c3a570f9dd582efbb3f0536a92e535042) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.18m",   0x1000004, 0x400000, CRC(4db92d94) SHA1(f1b25ccc0627139ad5b287a8f2ab3b4a2fb8b8e4) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "msh.20m",   0x1000006, 0x400000, CRC(a2b0c6c0) SHA1(71016c01c1a706b73cf5b9ac7e384a030c6cf08d) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "msh.01",   0x00000, 0x08000, CRC(c976e6f9) SHA1(281025e5aaf97c0aeddc8bd0f737d092daadad9e) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "msh.02",   0x28000, 0x20000, CRC(ce67d0d9) SHA1(324226597cc5a11603f04085fef7715a314ecc05) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "msh.11m",   0x000000, 0x200000, CRC(37ac6d30) SHA1(ec67421fbf4a08a686e76792cb35e9cbf04d022d) )
	ROM_LOAD16_WORD_SWAP( "msh.12m",   0x200000, 0x200000, CRC(de092570) SHA1(a03d0df901f6ea79685eaed67db65bee14ec29c6) )
ROM_END

ROM_START( sfz2ald )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "szaad.03", 0x000000, 0x80000, CRC(89f9483b) SHA1(5da4f82ddc79d9b5697ba2fea96dcaf460118248) )
	ROM_LOAD16_WORD_SWAP( "szaad.04", 0x080000, 0x80000, CRC(aef27ae5) SHA1(5877a05e7355195d686a4d0a97062aacbd3277ee) )
	ROM_LOAD16_WORD_SWAP( "szaa.05",  0x100000, 0x80000, CRC(f053a55e) SHA1(f98a8af5cd33a543a5596d59381f9adafed38854) )
	ROM_LOAD16_WORD_SWAP( "szaa.06",  0x180000, 0x80000, CRC(cfc0e7a8) SHA1(31ed58451c7a6ac88a8fccab369167694698f044) )
	ROM_LOAD16_WORD_SWAP( "szaa.07",  0x200000, 0x80000, CRC(5feb8b20) SHA1(13c79c9b72c3abf0a0b75d507d91ece71e460c06) )
	ROM_LOAD16_WORD_SWAP( "szaa.08",  0x280000, 0x80000, CRC(6eb6d412) SHA1(c858fec9c1dfea70dfcca629c1c24306f8ae6d81) )

	ROM_REGION16_BE( CODE_SIZE, "user1", 0 )
	ROM_FILL( 0x000000, 0x100000, 0 )

	ROM_REGION( 0x1400000, "gfx", 0 )
	ROMX_LOAD( "sz2.13m",   0x0000000, 0x400000, CRC(4d1f1f22) SHA1(659fb4305bcf0cbbbbec97ede6e68a8323b13308) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.15m",   0x0000002, 0x400000, CRC(19cea680) SHA1(4cb88963a0fbcef191c8419b6379387c01b4c81e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.17m",   0x0000004, 0x400000, CRC(e01b4588) SHA1(c2936608fd75ff6cd5fa94c6d6d6f0c77c44a450) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.19m",   0x0000006, 0x400000, CRC(0feeda64) SHA1(f5b350601437bd94b70d97feb23d791df19da6b3) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.14m",   0x1000000, 0x100000, CRC(0560c6aa) SHA1(f2bed3a8efef18052b51a7f0f6a888a18db813a1) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.16m",   0x1000002, 0x100000, CRC(ae940f87) SHA1(39ee26333abbe302ba76dced0196a2e6b3b1d02a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.18m",   0x1000004, 0x100000, CRC(4bc3c8bc) SHA1(6256963c515bf56f39b6e559afefd653ead56c54) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz2.20m",   0x1000006, 0x100000, CRC(39e674c0) SHA1(8e771a2d8c2accad0463bccd21d7b23af0c895a1) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sz2.01a",   0x00000, 0x08000, CRC(1bc323cf) SHA1(83fbd6e9b327700dc9f1c59700b7385bc3705749) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "sz2.02a",   0x28000, 0x20000, CRC(ba6a5013) SHA1(7814f3e56b69529b9860dd61c3b1e8d700244b03) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "sz2.11m",   0x000000, 0x200000, CRC(aa47a601) SHA1(a4d1ee89c84a3b9db06469bb66e85293b5aa9ac9) )
	ROM_LOAD16_WORD_SWAP( "sz2.12m",   0x200000, 0x200000, CRC(2237bc53) SHA1(96d5693047e4cf1ed10a8ee1905cea267a278e92) )
ROM_END

ROM_START( sfa3ud )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "sz3ud.03c", 0x000000, 0x80000, CRC(6db8add7) SHA1(f6dc8ed330254acab16e42a8e729bca7713c8dc1) )
	ROM_LOAD16_WORD_SWAP( "sz3ud.04c", 0x080000, 0x80000, CRC(d9c65a26) SHA1(8293ef112eaa534b58a3b56721af5d2f53fd1576) )
	ROM_LOAD16_WORD_SWAP( "sz3.05c",   0x100000, 0x80000, CRC(57fd0a40) SHA1(bc2d5f4d57117bbf58b1adb088e00424ef489e92) )
	ROM_LOAD16_WORD_SWAP( "sz3.06c",   0x180000, 0x80000, CRC(f6305f8b) SHA1(3fd1ebdbad96103aca604e950b488e52460a71ec) )
	ROM_LOAD16_WORD_SWAP( "sz3.07c",   0x200000, 0x80000, CRC(6eab0f6f) SHA1(f8d093dda65cf4e8a3000dc1b96355bb03dcb495) )
	ROM_LOAD16_WORD_SWAP( "sz3.08c",   0x280000, 0x80000, CRC(910c4a3b) SHA1(dbd41280f9b16ad6a5b8f12092549970349395f1) )
	ROM_LOAD16_WORD_SWAP( "sz3.09c",   0x300000, 0x80000, CRC(b29e5199) SHA1(c6c215eb5aa37f678a9cafcbd8620969fb5ca12f) )
	ROM_LOAD16_WORD_SWAP( "sz3.10b",   0x380000, 0x80000, CRC(deb2ff52) SHA1(0aa4722aad68a04164946c78bf05752f947b4322) )

	ROM_REGION16_BE( CODE_SIZE, "user1", 0 )
	ROM_FILL( 0x000000, 0x100000, 0 )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "sz3.13m",   0x0000000, 0x400000, CRC(0f7a60d9) SHA1(c69e0ee22537312909dacc86d2e4be319d54e426) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.15m",   0x0000002, 0x400000, CRC(8e933741) SHA1(f4ac4bfe830dc7df9fe4f680e4e0c053e7cbd8fe) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.17m",   0x0000004, 0x400000, CRC(d6e98147) SHA1(37f331fbb1284db446faecade6f484f58c0e1b2a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.19m",   0x0000006, 0x400000, CRC(f31a728a) SHA1(f14136564648f006c1b74afda78349f260524b5f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.14m",   0x1000000, 0x400000, CRC(5ff98297) SHA1(9e0ce43380b776c7a03872bafd4856f6fa60bda7) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.16m",   0x1000002, 0x400000, CRC(52b5bdee) SHA1(7918204dc457f7a146d8fb8cf7242dfed3109fd8) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.18m",   0x1000004, 0x400000, CRC(40631ed5) SHA1(c18c56822b90a71ca5fbdf3440eb2671011f3d8f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sz3.20m",   0x1000006, 0x400000, CRC(763409b4) SHA1(af60a5116c1ca9050366a35ea29128921867f3cc) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sz3.01",   0x00000, 0x08000, CRC(de810084) SHA1(fd0b969b732921ed8b40c16fbfa30ee09c7a7cbd) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "sz3.02",   0x28000, 0x20000, CRC(72445dc4) SHA1(14fca7596ac45ba655016eef5b6120f9f9671c23) )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "sz3.11m",   0x000000, 0x400000, CRC(1c89eed1) SHA1(649a0b0a3eb72e2e69e9fb1ac51a58b70daa39f3) )
	ROM_LOAD16_WORD_SWAP( "sz3.12m",   0x400000, 0x400000, CRC(f392b13a) SHA1(fa04ce0370144a49bd1d5acd873eef87b0dc9d15) )
ROM_END

ROM_START( spf2xjd )
	ROM_REGION(CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "pzfjd.03a", 0x000000, 0x80000, CRC(5e85ed08) SHA1(8242030daf24d8058368d2e7bfe8199db62966c3) )
	ROM_LOAD16_WORD_SWAP( "pzf.04",    0x080000, 0x80000, CRC(b80649e2) SHA1(5bfccd656aea7ff82e9a20bb5856f4ab99b5a007) )

	ROM_REGION16_BE( CODE_SIZE, "user1", 0 )
	ROM_FILL( 0x000000, 0x100000, 0 )

	ROM_REGION( 0xC00000, "gfx", 0 )
	ROM_FILL(              0x000000, 0x800000, 0 )
	ROMX_LOAD( "pzf.14m",  0x800000, 0x100000, CRC(2d4881cb) SHA1(fd3baa183c25bed153b19c251980e2fb761600e2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pzf.16m",  0x800002, 0x100000, CRC(4b0fd1be) SHA1(377aafdcdb7a866b1c8487670e3598d8197976e4) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pzf.18m",  0x800004, 0x100000, CRC(e43aac33) SHA1(d041e0688c3807d3363861a7f216de43b34d846c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pzf.20m",  0x800006, 0x100000, CRC(7f536ff1) SHA1(905b9d62ef7bef47297c7f4a4dd697aed6df38a5) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION(QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "pzf.01",   0x00000, 0x08000, CRC(600fb2a3) SHA1(1fab1c2a23bf6ad8309d29ddbbc29435a8aeea13) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "pzf.02",   0x28000, 0x20000, CRC(496076e0) SHA1(1ee4e135140afd0e8e03231e570cd77d140f6367) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "pzf.11m",   0x000000, 0x200000, CRC(78442743) SHA1(b61190bb586871de6d54af580e3e1d9cc0de0acb) )
	ROM_LOAD16_WORD_SWAP( "pzf.12m",   0x200000, 0x200000, CRC(399d2c7b) SHA1(e849dea97b8d16540415c0d9bbc4f9f4eb755ec4) )
ROM_END

ROM_START( vsavd )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "vm3ed.03d", 0x000000, 0x80000, CRC(97d805e3) SHA1(394398fbeb0b375a79cefb48c57d14b330311247) )
	ROM_LOAD16_WORD_SWAP( "vm3ed.04d", 0x080000, 0x80000, CRC(5e07fdce) SHA1(897d1c10fb0de2a62360d9b43cb78faaa9976eb9) )
	ROM_LOAD16_WORD_SWAP( "vm3.05a",   0x100000, 0x80000, CRC(4118e00f) SHA1(94ce8abc5ff547667f4c6022d84d0ed4cd062d7e) )
	ROM_LOAD16_WORD_SWAP( "vm3.06a",   0x180000, 0x80000, CRC(2f4fd3a9) SHA1(48549ff0121312ea4a18d0fa167a32f905c14c9f) )
	ROM_LOAD16_WORD_SWAP( "vm3.07b",   0x200000, 0x80000, CRC(cbda91b8) SHA1(31b20aa92422384b1d7a4706ad4c01ea2bd0e0d1) )
	ROM_LOAD16_WORD_SWAP( "vm3.08a",   0x280000, 0x80000, CRC(6ca47259) SHA1(485d8f3a132ccb3f7930cae74de8662d2d44e412) )
	ROM_LOAD16_WORD_SWAP( "vm3.09b",   0x300000, 0x80000, CRC(f4a339e3) SHA1(abd101a55f7d9ddb8aba04fe8d3f0f5d2006c925) )
	ROM_LOAD16_WORD_SWAP( "vm3.10b",   0x380000, 0x80000, CRC(fffbb5b8) SHA1(38aecb820bd1cbd17287848c3ffb013e1d464ddf) )

	ROM_REGION16_BE( CODE_SIZE, "user1", 0 )
	ROM_FILL( 0x000000, 0x100000, 0 )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "vm3.13m",   0x0000000, 0x400000, CRC(fd8a11eb) SHA1(21b9773959e17976ff46b75a6a405042836b2c5f) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vm3.15m",   0x0000002, 0x400000, CRC(dd1e7d4e) SHA1(30476e061cdebdb1838b83f4ebd5efae12b7dbfb) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vm3.17m",   0x0000004, 0x400000, CRC(6b89445e) SHA1(2abd489839d143c46e25f4fc3db476b70607dc03) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vm3.19m",   0x0000006, 0x400000, CRC(3830fdc7) SHA1(ebd3f559c254d349e256c9feb3477f1ed7518206) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vm3.14m",   0x1000000, 0x400000, CRC(c1a28e6c) SHA1(012803af33174c0602649d2a2d84f6ee79f54ad2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vm3.16m",   0x1000002, 0x400000, CRC(194a7304) SHA1(a19a9a6fb829953b054dc5c3b0dc017f60d37928) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vm3.18m",   0x1000004, 0x400000, CRC(df9a9f47) SHA1(ce29ff00cf4b6fdd9b3b1ed87823534f1d364eab) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vm3.20m",   0x1000006, 0x400000, CRC(c22fc3d9) SHA1(df7538c05b03a4ad94d369f8083799979e6fac42) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "vm3.01",   0x00000, 0x08000, CRC(f778769b) SHA1(788ce1ad8a322179f634df9e62a31ad776b96762) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "vm3.02",   0x28000, 0x20000, CRC(cc09faa1) SHA1(2962ef0ceaf7e7279de3c421ea998763330eb43e) )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "vm3.11m",   0x000000, 0x400000, CRC(e80e956e) SHA1(74181fca4b764fb3c56ceef2cb4c6fd6c18ec4b6) )
	ROM_LOAD16_WORD_SWAP( "vm3.12m",   0x400000, 0x400000, CRC(9cd71557) SHA1(7059db25698a0b286314c5961c618f6d2e6f24a1) )
ROM_END

ROM_START( xmcotad )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "xmned.03e", 0x000000, 0x80000, CRC(bef56003) SHA1(4264f9d7236b00e513664932685b3d93ea636f21) )
	ROM_LOAD16_WORD_SWAP( "xmned.04e", 0x080000, 0x80000, CRC(b1a21fa6) SHA1(cbb577b180f28e2af5d6518679f3b16967129ef5) )
	ROM_LOAD16_WORD_SWAP( "xmn.05a",   0x100000, 0x80000, CRC(ac0d7759) SHA1(650d4474b13f16af7910a0f721fcda2ddb2414fd) )
	ROM_LOAD16_WORD_SWAP( "xmn.06a",   0x180000, 0x80000, CRC(1b86a328) SHA1(2469cd705139ee9f1142e6e379e68d0c9675b37e) )
	ROM_LOAD16_WORD_SWAP( "xmn.07a",   0x200000, 0x80000, CRC(2c142a44) SHA1(7624875f9c39b361fc83e52e87e0fd5e96279713) )
	ROM_LOAD16_WORD_SWAP( "xmn.08a",   0x280000, 0x80000, CRC(f712d44f) SHA1(0d18d4a4eacad94a66beca6ec509ac7f690c6882) )
	ROM_LOAD16_WORD_SWAP( "xmn.09a",   0x300000, 0x80000, CRC(9241cae8) SHA1(bb6980abf25aaf3eb14e230ca6942f3e2ab2c660) )
	ROM_LOAD16_WORD_SWAP( "xmn.10a",   0x380000, 0x80000, CRC(53c0eab9) SHA1(e3b1ec1fd517735f7801cfebb257c43185c6d3fb) )

	ROM_REGION16_BE( CODE_SIZE, "user1", 0 )
	ROM_FILL( 0x000000, 0x100000, 0 )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "xmn.13m",   0x0000000, 0x400000, CRC(bf4df073) SHA1(4d2740c3a827f0ec2cf75ad99c65e393c6a11c23) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.15m",   0x0000002, 0x400000, CRC(4d7e4cef) SHA1(50b8797b8099a8d76ad063ba1201a13dbb88ae3a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.17m",   0x0000004, 0x400000, CRC(513eea17) SHA1(a497477ad9ac13180911d8745ef6ee1955c0b877) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.19m",   0x0000006, 0x400000, CRC(d23897fc) SHA1(1e31627999736652252164d32662779a1ac6ca29) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.14m",   0x1000000, 0x400000, CRC(778237b7) SHA1(89a759ec383518ec52f5059d10ec342f2247aa20) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.16m",   0x1000002, 0x400000, CRC(67b36948) SHA1(692fb6e4096b880aa22996d554b160f664bbd907) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.18m",   0x1000004, 0x400000, CRC(015a7c4c) SHA1(cccc95dafd076a1a9fa004710006149c42d058ba) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xmn.20m",   0x1000006, 0x400000, CRC(9dde2758) SHA1(17ba259cad03c7b5d56c0a5eda9ab53521665729) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "xmn.01a",  0x00000, 0x08000, CRC(40f479ea) SHA1(f29e15f537675305264ae2138a0a537fb9e2008b) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "xmn.02a",  0x28000, 0x20000, CRC(39d9b5ad) SHA1(af502debfd36100d4fc971ed25fdf9d7121d6f18) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "xmn.11m",   0x000000, 0x200000, CRC(c848a6bc) SHA1(ac8ac564d3c43225822f8bc330eba9f35b24b0a4) )
	ROM_LOAD16_WORD_SWAP( "xmn.12m",   0x200000, 0x200000, CRC(729c188f) SHA1(3279774ad8aebbcf0fc779cdfcbe21044dd192ad) )
ROM_END

ROM_START( xmvsfu1d )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "xvsud.03h", 0x000000, 0x80000, CRC(4e2e76b7) SHA1(812ebe4063a1c5d8c86200a51e6ab00e57e02869) )
	ROM_LOAD16_WORD_SWAP( "xvsud.04h", 0x080000, 0x80000, CRC(290c61a7) SHA1(f0d409048c9d477ee98e6df92febcd4492a291ee) )
	ROM_LOAD16_WORD_SWAP( "xvsd.05a",  0x100000, 0x80000, CRC(de347b11) SHA1(297ae207811df9a4973de1df00b2efaa14a0137d) )
	ROM_LOAD16_WORD_SWAP( "xvs.06a",   0x180000, 0x80000, CRC(e8e2c75c) SHA1(929408cb5d98e95cec75ea58e4701b0cbdbcd016) )
	ROM_LOAD16_WORD_SWAP( "xvsd.07",   0x200000, 0x80000, CRC(f761ded7) SHA1(e49277398734dea044e7c8ec16800db196905e6f) )
	ROM_LOAD16_WORD_SWAP( "xvs.08",    0x280000, 0x80000, CRC(81929675) SHA1(19cf7afbc1daaefec40195e40ba74970f3906a1c) )
	ROM_LOAD16_WORD_SWAP( "xvs.09",    0x300000, 0x80000, CRC(9641f36b) SHA1(dcba3482d1ba37ccfb30d402793ee063c6621aed) )

	ROM_REGION16_BE( CODE_SIZE, "user1", 0 )
	ROM_FILL( 0x000000, 0x100000, 0 )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "xvs.13m",   0x0000000, 0x400000, CRC(f6684efd) SHA1(c0a2f3a9e82ab8b084a500aec71ac633e947328c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.15m",   0x0000002, 0x400000, CRC(29109221) SHA1(898b8f678fd03c462ce0d8eb7fb3441ef601085b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.17m",   0x0000004, 0x400000, CRC(92db3474) SHA1(7b6f4c8ebfdac167b25f35029068b6253c141fe6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.19m",   0x0000006, 0x400000, CRC(3733473c) SHA1(6579da7145c95b3ad00844a5fc8c2e22c23365e2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.14m",   0x1000000, 0x400000, CRC(bcac2e41) SHA1(838ff24f7e8543a787a55a5d592c9517ce3b8b93) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.16m",   0x1000002, 0x400000, CRC(ea04a272) SHA1(cd7c79037b5b4a39bef5156433e984dc4dc2c081) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.18m",   0x1000004, 0x400000, CRC(b0def86a) SHA1(da3a6705ea7050fc5c2c10d33400ed67be9f455d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "xvs.20m",   0x1000006, 0x400000, CRC(4b40ff9f) SHA1(9a981d442132efff09a27408d74646ba357c7357) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "xvs.01",   0x00000, 0x08000, CRC(3999e93a) SHA1(fefcff8a9a5c83df7655a16187cf9ba3e7efbb25) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "xvs.02",   0x28000, 0x20000, CRC(101bdee9) SHA1(75920e88bf46fcd33a7957777a1d799818ffb0d6) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "xvs.11m",   0x000000, 0x200000, CRC(9cadcdbc) SHA1(64d3bd53b04daec84c9af4aa3ff010867b3d306d) )
	ROM_LOAD16_WORD_SWAP( "xvs.12m",   0x200000, 0x200000, CRC(7b11e460) SHA1(a581c84acaaf0ce056841c15a6f36889e88be68d) )
ROM_END

ROM_START( dstlku1d )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "vamud.03a", 0x000000, 0x80000, CRC(47b7a680) SHA1(EA51D784F1ED49B70AA8ADDCE987A7C09A5C653C) )
	ROM_LOAD16_WORD_SWAP( "vamud.04a", 0x080000, 0x80000, CRC(3b7a4939) SHA1(020F9768F4D3AFB193B1D5A824674F7FB3434369) )
	ROM_LOAD16_WORD_SWAP( "vamu.05a",  0x100000, 0x80000, CRC(673ed50a) SHA1(7dff27dba1da55a18eb459e4a2d679cf699f2804) )
	ROM_LOAD16_WORD_SWAP( "vamu.06a",  0x180000, 0x80000, CRC(f2377be7) SHA1(4520d44f94a03bd40c27062344e56ba8718c2fb8) )
	ROM_LOAD16_WORD_SWAP( "vamu.07a",  0x200000, 0x80000, CRC(d8f498c4) SHA1(569d9c309e9d95d2501a7c0a2c1291b49320d767) )
	ROM_LOAD16_WORD_SWAP( "vamu.08a",  0x280000, 0x80000, CRC(e6a8a1a0) SHA1(adf621e12623a2af4dbf0858a8fa3816e7c7073b) )
	ROM_LOAD16_WORD_SWAP( "vamud.09a", 0x300000, 0x80000, CRC(8b333a19) SHA1(5274510491433AC4A0BA3A0A120E95205D291FFE) )
	ROM_LOAD16_WORD_SWAP( "vamu.10a",  0x380000, 0x80000, CRC(c1a3d9be) SHA1(82b4ce3325a7ecf3a60dd781f9b224fdde8daa65) )

	ROM_REGION( 0x1400000, "gfx", 0 )
	ROMX_LOAD( "vam.13m",   0x0000000, 0x400000, CRC(c51baf99) SHA1(2fb6642908e542e404391eb17392f8270e87bf48) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.15m",   0x0000002, 0x400000, CRC(3ce83c77) SHA1(93369b23c6d7d834297434691bb047ee3dd9539c) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.17m",   0x0000004, 0x400000, CRC(4f2408e0) SHA1(cd49c6b3c7e6470c6058f98ccc5210b052bb13e2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.19m",   0x0000006, 0x400000, CRC(9ff60250) SHA1(d69ba4dc6bd37d003245f0cf3211d6e2623005b8) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.14m",   0x1000000, 0x100000, CRC(bd87243c) SHA1(87b33aeb72514e1228ffc27ec6dd534f14882760) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.16m",   0x1000002, 0x100000, CRC(afec855f) SHA1(cd117833b8d475489b90ff44b57e2c5cb1af3af5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.18m",   0x1000004, 0x100000, CRC(3a033625) SHA1(294238f30cba5cf4f8f1de951d54c2077bd95de9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "vam.20m",   0x1000006, 0x100000, CRC(2bff6a89) SHA1(8f4e131e5ce0af48fb89f98026d9f0356c7c301f) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "vam.01",   0x00000, 0x08000, CRC(64b685d5) SHA1(6c180e7420db754eca5cad17a40f5a64f5c3bd15) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "vam.02",   0x28000, 0x20000, CRC(cf7c97c7) SHA1(109a4b56ecd59be9c3f5869de99d40619bdaef21) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "vam.11m",   0x000000, 0x200000, CRC(4a39deb2) SHA1(7e63e615869958db66a4e52a0272afee5a10e446) )
	ROM_LOAD16_WORD_SWAP( "vam.12m",   0x200000, 0x200000, CRC(1a3e5c03) SHA1(c5a556e125d6c3d68da745b4d56cd7a851f2a23d) )
ROM_END

ROM_START( progerjd )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "pgajd.03", 0x000000, 0x80000, CRC(4fef676c) SHA1(f50f32403315f77e115141bbee6a1b9a800821eb) )
	ROM_LOAD16_WORD_SWAP( "pgajd.04", 0x080000, 0x80000, CRC(a069bd3b) SHA1(6b7e20c883221da9b8eccb4f86017bb93e1fc11f) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROMX_LOAD( "pga-simm.01c",   0x0000000, 0x200000,  CRC(452f98b0) SHA1(a10e615c32098f6d25becd466da8faa967523a7b) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "pga-simm.01d",   0x0000001, 0x200000,  CRC(9e672092) SHA1(fce0b8b43a1c069262f4e3e81c1a04621e232c88) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "pga-simm.01a",   0x0000002, 0x200000,  CRC(ae9ddafe) SHA1(afbb26fed6cd0cb5c0099a10d35aeb453318c14d) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "pga-simm.01b",   0x0000003, 0x200000,  CRC(94d72d94) SHA1(df6a3fe49c008f73b160eb6f2a44dc371ff73cba) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "pga-simm.03c",   0x0000004, 0x200000,  CRC(48a1886d) SHA1(ebf44b42d784924e08a832a7e5f66a887bab244b) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "pga-simm.03d",   0x0000005, 0x200000,  CRC(172d7e37) SHA1(0eaedd24cd3fa87b6f35fbd63078d40c493c92d0) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "pga-simm.03a",   0x0000006, 0x200000,  CRC(9ee33d98) SHA1(85d1bd31940e35ac8c732165020881a2d65cd6b1) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm
	ROMX_LOAD( "pga-simm.03b",   0x0000007, 0x200000,  CRC(848dee32) SHA1(c591288e86ad1624d0fe66563808af9fac786e64) , ROM_GROUPBYTE | ROM_SKIP(7) ) // ROM on a simm

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "pga.01",   0x00000, 0x08000, CRC(bdbfa992) SHA1(7c5496c1daaea6a7ab95c0b25625d325ec3427cc) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "pga-simm.05a",   0x000000, 0x200000, CRC(c0aac80c) SHA1(91784d35d4f7e113529bb5be6081b67094b150ea) ) // ROM on a simm
	ROM_LOAD16_WORD_SWAP( "pga-simm.05b",   0x200000, 0x200000, CRC(37a65d86) SHA1(374d562a4648734f82aa2ddb6d258e870896dd45) ) // ROM on a simm
	ROM_LOAD16_WORD_SWAP( "pga-simm.06a",   0x400000, 0x200000, CRC(d3f1e934) SHA1(5dcea28c873d0d472f5b94e07d97cd77ace2b252) ) // ROM on a simm
	ROM_LOAD16_WORD_SWAP( "pga-simm.06b",   0x600000, 0x200000, CRC(8b39489a) SHA1(fd790efaf37dc2c4c16f657941044e3e2d3c2711) ) // ROM on a simm
ROM_END

ROM_START( ssf2ud )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "ssfud.03a", 0x000000, 0x80000, CRC(fad5daf8) SHA1(5bed5d5af8dffc54d3b3371274a3905f46384f79) )
	ROM_LOAD16_WORD_SWAP( "ssfud.04a", 0x080000, 0x80000, CRC(0d31af65) SHA1(7e903e18cb899627fedd8ad92594b75b5d8ee8fd) )
	ROM_LOAD16_WORD_SWAP( "ssfud.05",  0x100000, 0x80000, CRC(75c651ef) SHA1(8c6f60d3cf10d802190438c403a719bc30ccbeaa) )
	ROM_LOAD16_WORD_SWAP( "ssfud.06",  0x180000, 0x80000, CRC(85c3ec00) SHA1(29ab3eccc5f9c0dd6bd19eaa83c72e1a1d44b320) )
	ROM_LOAD16_WORD_SWAP( "ssfud.07",  0x200000, 0x80000, CRC(247e2504) SHA1(b2f13524fe7f75e69d2d8f2dd212340d75030149) )

	ROM_REGION( 0xc00000, "gfx", 0 )
	ROMX_LOAD( "ssf.13m",   0x000000, 0x200000, CRC(cf94d275) SHA1(bf2a6d98a656d1cb5734da7836686242d3211137) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.15m",   0x000002, 0x200000, CRC(5eb703af) SHA1(4b302dbb66e8a5c2ad92798699391e981bada427) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.17m",   0x000004, 0x200000, CRC(ffa60e0f) SHA1(b21b1c749a8241440879bf8e7cb33968ccef97e5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.19m",   0x000006, 0x200000, CRC(34e825c5) SHA1(4d320fc96d1ef0b9928a8ce801734245a4c097a5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.14m",   0x800000, 0x100000, CRC(b7cc32e7) SHA1(0f4d26af338dab5dce5b7b34d32ad0c573434ace) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.16m",   0x800002, 0x100000, CRC(8376ad18) SHA1(f4456833fb396e6501f4174c0fe5fd63ea40a188) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.18m",   0x800004, 0x100000, CRC(f5b1b336) SHA1(4b060501e56b9d61294748da5387cdae5280ec4d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.20m",   0x800006, 0x100000, CRC(459d5c6b) SHA1(32b11ba7a12004aff810d719bff7508204c7b7c0) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ssf.01",   0x00000, 0x08000, CRC(eb247e8c) SHA1(24296c18d9b1136d69712bf1c9d9d15463041e83) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD( "ssf.q01",  0x000000, 0x080000, CRC(a6f9da5c) SHA1(6d19f83a01bd25b838d5c2871f7964529d926c98) )
	ROM_LOAD( "ssf.q02",  0x080000, 0x080000, CRC(8c66ae26) SHA1(32a82aee6ed4480e5a990f9af161734c7c0a1403) )
	ROM_LOAD( "ssf.q03",  0x100000, 0x080000, CRC(695cc2ca) SHA1(c2675f0233608b76de528d2a6ef19846d1348060) )
	ROM_LOAD( "ssf.q04",  0x180000, 0x080000, CRC(9d9ebe32) SHA1(9b26329370041374f1a90b479a172d2bc2801c4d) )
	ROM_LOAD( "ssf.q05",  0x200000, 0x080000, CRC(4770e7b7) SHA1(0e764f0befb9227b0b36508d8ca8ec9be31bcb05) )
	ROM_LOAD( "ssf.q06",  0x280000, 0x080000, CRC(4e79c951) SHA1(1144781d7dc57ef8a6458d982f5c91548ff59e27) )
	ROM_LOAD( "ssf.q07",  0x300000, 0x080000, CRC(cdd14313) SHA1(ebe767a9d4b51dba4282fa0a36a546a88620dd59) )
	ROM_LOAD( "ssf.q08",  0x380000, 0x080000, CRC(6f5a088c) SHA1(6c0b4690479647a99d76335f174be8455b4ff118) )
ROM_END

ROM_START( ssf2tbd )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "ssfed.3tc", 0x000000, 0x80000, CRC(5d86caf8) SHA1(6209caad6613f0b35002fb13350ae3f81c8c9c5f) )
	ROM_LOAD16_WORD_SWAP( "ssfed.4tc", 0x080000, 0x80000, CRC(f6e1f98d) SHA1(2d7506fdf12c8ca8766da1458420e0764f159a9b) )
	ROM_LOAD16_WORD_SWAP( "ssfed.5t",  0x100000, 0x80000, CRC(75c651ef) SHA1(8c6f60d3cf10d802190438c403a719bc30ccbeaa) )
	ROM_LOAD16_WORD_SWAP( "ssfed.6tb", 0x180000, 0x80000, CRC(9adac7d7) SHA1(8677f882f84e6db9ed39ae89ffc3bf9b6b53e4e5) )
	ROM_LOAD16_WORD_SWAP( "ssfed.7t",  0x200000, 0x80000, CRC(84f54db3) SHA1(7c2016f9c1839f096c9a39bad882eed95bf552b4) )

	ROM_REGION( 0xc00000, "gfx", 0 )
	ROMX_LOAD( "ssf.13m",   0x000000, 0x200000, CRC(cf94d275) SHA1(bf2a6d98a656d1cb5734da7836686242d3211137) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.15m",   0x000002, 0x200000, CRC(5eb703af) SHA1(4b302dbb66e8a5c2ad92798699391e981bada427) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.17m",   0x000004, 0x200000, CRC(ffa60e0f) SHA1(b21b1c749a8241440879bf8e7cb33968ccef97e5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.19m",   0x000006, 0x200000, CRC(34e825c5) SHA1(4d320fc96d1ef0b9928a8ce801734245a4c097a5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.14m",   0x800000, 0x100000, CRC(b7cc32e7) SHA1(0f4d26af338dab5dce5b7b34d32ad0c573434ace) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.16m",   0x800002, 0x100000, CRC(8376ad18) SHA1(f4456833fb396e6501f4174c0fe5fd63ea40a188) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.18m",   0x800004, 0x100000, CRC(f5b1b336) SHA1(4b060501e56b9d61294748da5387cdae5280ec4d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "ssf.20m",   0x800006, 0x100000, CRC(459d5c6b) SHA1(32b11ba7a12004aff810d719bff7508204c7b7c0) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "ssf.01",   0x00000, 0x08000, CRC(eb247e8c) SHA1(24296c18d9b1136d69712bf1c9d9d15463041e83) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD( "ssf.q01",  0x000000, 0x080000, CRC(a6f9da5c) SHA1(6d19f83a01bd25b838d5c2871f7964529d926c98) )
	ROM_LOAD( "ssf.q02",  0x080000, 0x080000, CRC(8c66ae26) SHA1(32a82aee6ed4480e5a990f9af161734c7c0a1403) )
	ROM_LOAD( "ssf.q03",  0x100000, 0x080000, CRC(695cc2ca) SHA1(c2675f0233608b76de528d2a6ef19846d1348060) )
	ROM_LOAD( "ssf.q04",  0x180000, 0x080000, CRC(9d9ebe32) SHA1(9b26329370041374f1a90b479a172d2bc2801c4d) )
	ROM_LOAD( "ssf.q05",  0x200000, 0x080000, CRC(4770e7b7) SHA1(0e764f0befb9227b0b36508d8ca8ec9be31bcb05) )
	ROM_LOAD( "ssf.q06",  0x280000, 0x080000, CRC(4e79c951) SHA1(1144781d7dc57ef8a6458d982f5c91548ff59e27) )
	ROM_LOAD( "ssf.q07",  0x300000, 0x080000, CRC(cdd14313) SHA1(ebe767a9d4b51dba4282fa0a36a546a88620dd59) )
	ROM_LOAD( "ssf.q08",  0x380000, 0x080000, CRC(6f5a088c) SHA1(6c0b4690479647a99d76335f174be8455b4ff118) )
ROM_END

ROM_START( ssf2xjd )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "sfxjd.03c", 0x000000, 0x80000, CRC(316de996) SHA1(4036539a554a9ccd8b5fc364dfc4c97f3d5efa96) )
	ROM_LOAD16_WORD_SWAP( "sfxjd.04a", 0x080000, 0x80000, CRC(9bf3bb2e) SHA1(4bdc6fa585cc67d3b6695f390c95c518cba2bea6) )
	ROM_LOAD16_WORD_SWAP( "sfxjd.05",  0x100000, 0x80000, CRC(c63358d0) SHA1(dde4d9e9adce4dee02322c0fd71615eed0af62e6) )
	ROM_LOAD16_WORD_SWAP( "sfxjd.06a", 0x180000, 0x80000, CRC(ccb29808) SHA1(10e7b135a936409fe7c4d7959ea375634a8c68e4) )
	ROM_LOAD16_WORD_SWAP( "sfxjd.07",  0x200000, 0x80000, CRC(61f94982) SHA1(d2f22f50c21393deda7d5838dbd2b265722acd38) )
	ROM_LOAD16_WORD_SWAP( "sfxjd.08",  0x280000, 0x80000, CRC(d399c36c) SHA1(b4c4217843e5c3ef00ab04d58ca9368d2d734065) )
	ROM_LOAD16_WORD_SWAP( "sfxd.09",   0x300000, 0x80000, CRC(0b3a6196) SHA1(a0480ac878e82ef6ec0f64dbbd621f10bc7906ea) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROMX_LOAD( "sfx.13m",   0x000000, 0x200000, CRC(cf94d275) SHA1(bf2a6d98a656d1cb5734da7836686242d3211137) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.15m",   0x000002, 0x200000, CRC(5eb703af) SHA1(4b302dbb66e8a5c2ad92798699391e981bada427) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.17m",   0x000004, 0x200000, CRC(ffa60e0f) SHA1(b21b1c749a8241440879bf8e7cb33968ccef97e5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.19m",   0x000006, 0x200000, CRC(34e825c5) SHA1(4d320fc96d1ef0b9928a8ce801734245a4c097a5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.14m",   0x800000, 0x100000, CRC(b7cc32e7) SHA1(0f4d26af338dab5dce5b7b34d32ad0c573434ace) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.16m",   0x800002, 0x100000, CRC(8376ad18) SHA1(f4456833fb396e6501f4174c0fe5fd63ea40a188) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.18m",   0x800004, 0x100000, CRC(f5b1b336) SHA1(4b060501e56b9d61294748da5387cdae5280ec4d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.20m",   0x800006, 0x100000, CRC(459d5c6b) SHA1(32b11ba7a12004aff810d719bff7508204c7b7c0) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.21m",   0xc00000, 0x100000, CRC(e32854af) SHA1(1a5e11e9caa2b96108d89ae660ef1f6bcb469a74) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.23m",   0xc00002, 0x100000, CRC(760f2927) SHA1(491e28e14ee06821fc9e709efa7b91313bc0c2db) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.25m",   0xc00004, 0x100000, CRC(1ee90208) SHA1(83df1d9953560edddc2951ea426d29fb014e6a8a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "sfx.27m",   0xc00006, 0x100000, CRC(f814400f) SHA1(ad6921af36d0bd5dfb89b1fb53c3ca3fd92d7204) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "sfx.01",   0x00000, 0x08000, CRC(b47b8835) SHA1(c8b2d50fe3a329bd0592ea160d505155d873dab1) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "sfx.02",   0x28000, 0x20000, CRC(0022633f) SHA1(cab3afc79da53e3887eb1ccd1f4d19790728e6cd) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "sfx.11m",   0x000000, 0x200000, CRC(9bdbd476) SHA1(a8520f77f30b97aae36408e0c4ca1ebbde1808a5) )
	ROM_LOAD16_WORD_SWAP( "sfx.12m",   0x200000, 0x200000, CRC(a05e3aab) SHA1(d4eb9cae66c74e956569fea8b815156fbd420f83) )
ROM_END

ROM_START( sgemfd )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "pcfud.03",0x000000, 0x80000, CRC(8b83674a) SHA1(053a2cb6aab9aa43f4c0d4ba9f0eb5a964133b28) )
	ROM_LOAD16_WORD_SWAP( "pcfd.04", 0x080000, 0x80000, CRC(b58f1d03) SHA1(592649956471363967b9920f47bde23da3e9cc2b) )
	ROM_LOAD16_WORD_SWAP( "pcf.05",  0x100000, 0x80000, CRC(215655f6) SHA1(242c0f4401520f2a3b0deafc3a807b18b987e496) )
	ROM_LOAD16_WORD_SWAP( "pcf.06",  0x180000, 0x80000, CRC(ea6f13ea) SHA1(1bc924a8a9da1d2ad7667685cdb92fe317a39aba) )
	ROM_LOAD16_WORD_SWAP( "pcf.07",  0x200000, 0x80000, CRC(5ac6d5ea) SHA1(9ce8e4668b565658597a868830545fb75a5eeaa6) )

	ROM_REGION( 0x1400000, "gfx", 0 )
	ROMX_LOAD( "pcf.13m",   0x0000000, 0x400000, CRC(22d72ab9) SHA1(653efd95c34b4b9d2ab0d219f41a99ca84e12214) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pcf.15m",   0x0000002, 0x400000, CRC(16a4813c) SHA1(bf5fce6008214f353414d1b64bea4ed0c7673670) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pcf.17m",   0x0000004, 0x400000, CRC(1097e035) SHA1(4bd51e4e9447af27d2cac1f6d2201e37c949912b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pcf.19m",   0x0000006, 0x400000, CRC(d362d874) SHA1(30c42af18440496cc05e4418e4efa41172ae4ced) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pcf.14m",   0x1000000, 0x100000, CRC(0383897c) SHA1(aba14afa1d0c971afcee4317f480e88117d77b5e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pcf.16m",   0x1000002, 0x100000, CRC(76f91084) SHA1(3d1e32467f2aa5dd6fb96bd5c866ecc9691660fc) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pcf.18m",   0x1000004, 0x100000, CRC(756c3754) SHA1(be2f709b90222a567f198f851cf07ffb0ad433d7) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pcf.20m",   0x1000006, 0x100000, CRC(9ec9277d) SHA1(b7ceeaca30dfcdf498b61a6961f0aa1a068b8ec4) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "pcf.01",   0x00000, 0x08000, CRC(254e5f33) SHA1(c413ec0630b9bdd15e64f42893eba8958a09b573) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "pcf.02",   0x28000, 0x20000, CRC(6902f4f9) SHA1(9bfe4ddade3c666076d26a2b545120f6d059fd7c) )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "pcf.11m",   0x000000, 0x400000, CRC(a5dea005) SHA1(3ae79baf6ff5bd527f82b26f164c7e3c65423ae2) )
	ROM_LOAD16_WORD_SWAP( "pcf.12m",   0x400000, 0x400000, CRC(4ce235fe) SHA1(795b94557e954cc0e45fd3778b609064d57a34a2) )
ROM_END

ROM_START( armwar1d )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "pwged.03b", 0x000000, 0x80000, CRC(496bd483) SHA1(c3a116ea019843aba666a0d7e7bd2975622306e3) )
	ROM_LOAD16_WORD_SWAP( "pwged.04b", 0x080000, 0x80000, CRC(9bd6a38f) SHA1(da90162b7bff223df59ac362a5f61c580a86d967) )
	ROM_LOAD16_WORD_SWAP( "pwged.05a", 0x100000, 0x80000, CRC(4c11d30f) SHA1(463ba1845b1239c839e9419cbc6762d52b7918db) )
	ROM_LOAD16_WORD_SWAP( "pwg.06",   0x180000, 0x80000, CRC(87a60ce8) SHA1(e2085c7c8c6792d055dbbb023c7f4e4aa38ae924) )
	ROM_LOAD16_WORD_SWAP( "pwg.07",   0x200000, 0x80000, CRC(f7b148df) SHA1(f369669713cf647222094c570a2eacd48a8637cf) )
	ROM_LOAD16_WORD_SWAP( "pwg.08",   0x280000, 0x80000, CRC(cc62823e) SHA1(edaf9bebdfc65ae5414090abd6844176eec39a00) )
	ROM_LOAD16_WORD_SWAP( "pwg.09",   0x300000, 0x80000, CRC(ddc85ca6) SHA1(e794c679531632e2142c6a5e3b858494389ce65e) )
	ROM_LOAD16_WORD_SWAP( "pwg.10",   0x380000, 0x80000, CRC(07c4fb28) SHA1(58a1ff3d105be7df833dd4f32973766649efcbcf) )

	ROM_REGION( 0x1400000, "gfx", 0 )
	ROMX_LOAD( "pwg.13m",   0x0000000, 0x400000, CRC(ae8fe08e) SHA1(b6f09663dcda69b5d7ac13e4afaf1efd692fb61e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.15m",   0x0000002, 0x400000, CRC(db560f58) SHA1(0c3716b32eb24544ff5d16b5dcadce195cd10d00) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.17m",   0x0000004, 0x400000, CRC(bc475b94) SHA1(a157664450895a146a532581dd6f4b63dff21c86) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.19m",   0x0000006, 0x400000, CRC(07439ff7) SHA1(f71e07c6d77c32828f5e319268b24b13a1a4b0c2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.14m",   0x1000000, 0x100000, CRC(c3f9ba63) SHA1(66191a52c39daa89b17ede5804ee41c028036f14) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.16m",   0x1000002, 0x100000, CRC(815b0e7b) SHA1(549785daac3122253fb94f6541bc7016147f5306) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.18m",   0x1000004, 0x100000, CRC(0109c71b) SHA1(eb51284ee0c85ff8f605fe1d166b7aa202be1344) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "pwg.20m",   0x1000006, 0x100000, CRC(eb75ffbe) SHA1(e9d1deca60be696ac5bff2017fb5de3525e5239a) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "pwg.01",   0x00000, 0x08000, CRC(18a5c0e4) SHA1(bb1353dd74884aaeec9b5f1d0b284d9cad53c0ff) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "pwg.02",   0x28000, 0x20000, CRC(c9dfffa6) SHA1(64e71028befe9a2514074be765dd020e1d2ea70b) )

	ROM_REGION( 0x400000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "pwg.11m",   0x000000, 0x200000, CRC(a78f7433) SHA1(e47ffba7b9dac9d0dda985c5d966194be18260f7) )
	ROM_LOAD16_WORD_SWAP( "pwg.12m",   0x200000, 0x200000, CRC(77438ed0) SHA1(733ca6c6a792e66e2aa12c5fc06dd459527afe4b) )
ROM_END


ROM_START( dimahoud )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "gmdud.03", 0x000000, 0x80000, CRC(12888435) SHA1(38204359c505ae861341936cd0fd2517411b8995) )
	ROM_LOAD16_WORD_SWAP( "gmd.04",   0x080000, 0x80000, CRC(37485567) SHA1(643c41fce6057bcaef0e0bedc62914c33d97eeaf) )
	ROM_LOAD16_WORD_SWAP( "gmd.05",   0x100000, 0x80000, CRC(da269ffb) SHA1(e99b04192030b6006cf67b563f40cea29c1b2e78) )
	ROM_LOAD16_WORD_SWAP( "gmdud.06", 0x180000, 0x80000, CRC(d825efda) SHA1(7299f3629d5136f567f5cf373754ea13b2190533) )

	ROM_REGION( 0x1000000, "gfx", 0 )
	ROMX_LOAD( "gmd.13m",   0x000000, 0x400000, CRC(80dd19f0) SHA1(0fd8b1e8d73cc83e6c34f0d94487938da2344f76) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "gmd.15m",   0x000002, 0x400000, CRC(dfd93a78) SHA1(c343d5ddcc25bd0739491e7439d7c0d0a8881a04) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "gmd.17m",   0x000004, 0x400000, CRC(16356520) SHA1(058713bef30c1b1d8b7dd0ceaaa57a3ab9751a70) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "gmd.19m",   0x000006, 0x400000, CRC(dfc33031) SHA1(a1ceaeddc2a79d5b79f1b107cac2ef6a5e621e77) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "gmd.01",   0x00000, 0x08000, CRC(3f9bc985) SHA1(1616bbee82877b1052a07531066f5009a80706be) )
	ROM_CONTINUE(         0x10000, 0x18000 )
	ROM_LOAD( "gmd.02",   0x28000, 0x20000, CRC(3fd39dde) SHA1(6a6e3ef9baa430ee83ab2312aa0221bae4d73dbd) )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "gmd.11m",   0x000000, 0x400000, CRC(06a65542) SHA1(a1b3df70c90055a3cd59d0149fd18a74eff5bcc9) )
	ROM_LOAD16_WORD_SWAP( "gmd.12m",   0x400000, 0x400000, CRC(50bc7a31) SHA1(7283569fc646c39f4c693f14e0ce7ff2ee49111a) )
ROM_END

ROM_START( mmatrixd )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "mmxjd.03", 0x000000, 0x80000, CRC(36711e60) SHA1(a192e786798f69320761017437ee4d2d47dc80c5) )
	ROM_LOAD16_WORD_SWAP( "mmxjd.04", 0x080000, 0x80000, CRC(4687226f) SHA1(b831582f578eb1e40bce1d1cbf231e4c27f510cd) )
	ROM_LOAD16_WORD_SWAP( "mmxjd.05", 0x100000, 0x80000, CRC(52124398) SHA1(0a18b9d2a7e3335ddf7ff9ac5c5f8298951f8c67) )

	ROM_REGION( 0x2000000, "gfx", 0 )
	ROMX_LOAD( "mmx.13m",   0x0000000, 0x400000, CRC(04748718) SHA1(d2e84d9dcc779c08469d815ccd709f30705954b8) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mmx.15m",   0x0000002, 0x400000, CRC(38074f44) SHA1(2002c4862c156b314bc4f3372b713c48e0667ec3) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mmx.17m",   0x0000004, 0x400000, CRC(e4635e35) SHA1(48ef7a82df83b981ddd6138c241ca129ab770e8e) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mmx.19m",   0x0000006, 0x400000, CRC(4400a3f2) SHA1(d0aa805ccbb153896e5983da1c398d1df4f40371) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mmx.14m",   0x1000000, 0x400000, CRC(d52bf491) SHA1(2398895cfdcf86fc485472e33df2cc446539e977) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mmx.16m",   0x1000002, 0x400000, CRC(23f70780) SHA1(691ee8964815b0ce54704e7feb59ca79b634f26d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mmx.18m",   0x1000004, 0x400000, CRC(2562c9d5) SHA1(e7defc3d33db632c4035ae069f2f2332c58afaf5) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "mmx.20m",   0x1000006, 0x400000, CRC(583a9687) SHA1(1d0b08b1e88509245db3c2090f0201938fd750b4) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( QSOUND_SIZE, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "mmx.01",   0x00000, 0x08000, CRC(c57e8171) SHA1(dedb92af1910d38727f816e6f507d25148f31b74) )
	ROM_CONTINUE(         0x10000, 0x18000 )

	ROM_REGION( 0x800000, "qsound", 0 ) /* QSound samples */
	ROM_LOAD16_WORD_SWAP( "mmx.11m",   0x000000, 0x400000, CRC(4180b39f) SHA1(cabb1c358eae1bb6cfed07f5b92e4acd38650667) )
	ROM_LOAD16_WORD_SWAP( "mmx.12m",   0x400000, 0x400000, CRC(95e22a59) SHA1(b3431d170c0a1a0d826ad0af21300b9180e3f114) )
ROM_END



GAME( 1993, ssf2ud,   ssf2,     dead_cps2, cps2_2p6b, cps2,    ROT0,   "bootleg", "Super Street Fighter II: The New Challengers (USA 930911 Phoenix Edition) (bootleg)", GAME_SUPPORTS_SAVE )
GAME( 1993, ddtodd,   ddtod,    dead_cps2, cps2_4p4b, cps2,    ROT0,   "bootleg", "Dungeons & Dragons: Tower of Doom (Euro 940412 Phoenix Edition) (bootleg)", GAME_SUPPORTS_SAVE )
GAME( 1994, armwar1d, armwar,   dead_cps2, cps2_3p3b, cps2,    ROT0,   "bootleg", "Armored Warriors (Euro 941011 Phoenix Edition) (bootleg)", GAME_SUPPORTS_SAVE )
GAME( 1994, avspd,    avsp,     dead_cps2, cps2_3p3b, cps2,    ROT0,   "bootleg", "Alien vs. Predator (Euro 940520 Phoenix Edition) (bootleg)", GAME_SUPPORTS_SAVE )
GAME( 1994, dstlku1d, dstlk,    dead_cps2, cps2_2p6b, cps2,    ROT0,   "bootleg", "Darkstalkers: The Night Warriors (USA 940705 Phoenix Edition) (bootleg)", GAME_SUPPORTS_SAVE )
GAME( 1994, ringdstd, ringdest, dead_cps2, cps2_2p6b, cps2,    ROT0,   "bootleg", "Ring of Destruction: Slammasters II (Euro 940902 Phoenix Edition) (bootleg)", GAME_SUPPORTS_SAVE )
GAME( 1994, xmcotad,  xmcota,   dead_cps2, cps2_2p6b, cps2,    ROT0,   "bootleg", "X-Men: Children of the Atom (Euro 950105 Phoenix Edition) (bootleg)", GAME_SUPPORTS_SAVE )
GAME( 1995, nwarrud,  nwarr,    dead_cps2, cps2_2p6b, cps2,    ROT0,   "bootleg", "Night Warriors: Darkstalkers' Revenge (USA 950406 Phoenix Edition) (bootleg)", GAME_SUPPORTS_SAVE )
GAME( 1995, sfad,     sfa,      dead_cps2, cps2_2p6b, cps2,    ROT0,   "bootleg", "Street Fighter Alpha: Warriors' Dreams (Euro 950727 Phoenix Edition) (bootleg)", GAME_SUPPORTS_SAVE )
GAME( 1995, mshud,    msh,      dead_cps2, cps2_2p6b, cps2,    ROT0,   "bootleg", "Marvel Super Heroes (US 951024 Phoenix Edition) (bootleg)", GAME_SUPPORTS_SAVE )
GAME( 1996, 19xxd,    19xx,     dead_cps2, cps2_2p2b, cps2,    ROT270, "bootleg", "19XX: The War Against Destiny (USA 951207 Phoenix Edition) (bootleg)", GAME_SUPPORTS_SAVE )
GAME( 1996, sfz2ad,   sfa2,     dead_cps2, cps2_2p6b, cps2,    ROT0,   "bootleg", "Street Fighter Zero 2 (Asia 960227 Phoenix Edition) (bootleg)", GAME_SUPPORTS_SAVE )
GAME( 1996, spf2xjd,  spf2t,    dead_cps2, cps2_2p2b, cps2,    ROT0,   "bootleg", "Super Puzzle Fighter II X (Japan 960531 Phoenix Edition) (bootleg)", GAME_SUPPORTS_SAVE )
GAME( 1996, ddsomud,  ddsom,    dead_cps2, cps2_4p4b, cps2,    ROT0,   "bootleg", "Dungeons & Dragons: Shadow over Mystara (USA 960619 Phoenix Edition) (bootleg)", GAME_SUPPORTS_SAVE )
GAME( 1996, gigamn2,  megaman2, gigamn2,   cps2_2p3b, gigamn2, ROT0,   "bootleg", "Giga Man 2: The Power Fighters (bootleg of Mega Man 2: The Power Fighters)", GAME_NOT_WORKING | GAME_SUPPORTS_SAVE ) // flash roms aren't dumped, layer offsets different, different sound system
GAME( 1996, megamn2d, megaman2, dead_cps2, cps2_2p3b, cps2,    ROT0,   "bootleg", "Mega Man 2: The Power Fighters (USA 960708 Phoenix Edition) (bootleg)", GAME_SUPPORTS_SAVE )
GAME( 1996, sfz2ald,  sfz2al,   dead_cps2, cps2_2p6b, cps2,    ROT0,   "bootleg", "Street Fighter Zero 2 Alpha (Asia 960826 Phoenix Edition) (bootleg)", GAME_SUPPORTS_SAVE )
GAME( 1996, xmvsfu1d, xmvsf,    dead_cps2, cps2_2p6b, cps2,    ROT0,   "bootleg", "X-Men Vs. Street Fighter (USA 961004 Phoenix Edition) (bootleg)", GAME_SUPPORTS_SAVE )
GAME( 1997, batcird,  batcir,   dead_cps2, cps2_4p2b, cps2,    ROT0,   "bootleg", "Battle Circuit (Euro 970319 Phoenix Edition) (bootleg)", GAME_SUPPORTS_SAVE )
GAME( 1997, vsavd,    vsav,     dead_cps2, cps2_2p6b, cps2,    ROT0,   "bootleg", "Vampire Savior: The Lord of Vampire (Euro 970519 Phoenix Edition) (bootleg)", GAME_SUPPORTS_SAVE )
GAME( 1998, mvscud,   mvsc,     dead_cps2, cps2_2p6b, cps2,    ROT0,   "bootleg", "Marvel Vs. Capcom: Clash of Super Heroes (USA 980123 Phoenix Edition) (bootleg)", GAME_SUPPORTS_SAVE )
GAME( 1998, sfa3ud,   sfa3,     dead_cps2, cps2_2p6b, cps2,    ROT0,   "bootleg", "Street Fighter Alpha 3 (USA 980904 Phoenix Edition) (bootleg)", GAME_SUPPORTS_SAVE )
GAME( 1999, gwingjd,  gigawing, dead_cps2, cps2_2p2b, cps2,    ROT0,   "bootleg", "Giga Wing (Japan 990223 Phoenix Edition) (bootleg)", GAME_SUPPORTS_SAVE )
GAME( 2000, 1944d,    1944,     dead_cps2, cps2_2p2b, cps2,    ROT0,   "bootleg", "1944: The Loop Master (USA 000620 Phoenix Edition) (bootleg)", GAME_SUPPORTS_SAVE )
GAME( 2001, progerjd, progear,  dead_cps2, cps2_2p3b, cps2,    ROT0,   "bootleg", "Progear no Arashi (Japan 010117 Phoenix Edition) (bootleg)", GAME_SUPPORTS_SAVE ) // doesn't display phoenix edition screen, hacked bootleg?
GAME( 2004, hsf2d,    hsf2,     dead_cps2, cps2_2p6b, cps2,    ROT0,   "bootleg", "Hyper Street Fighter II: The Anniversary Edition (Asia 040202 Phoenix Edition) (bootleg)", GAME_SUPPORTS_SAVE )
GAME( 1993, ssf2tbd,  ssf2,     dead_cps2, cps2_2p6b, ssf2tb,  ROT0,   "bootleg", "Super Street Fighter II: The Tournament Battle (World 931119 Phoenix Edition) (bootleg)", GAME_SUPPORTS_SAVE )
GAME( 1994, ssf2xjd,  ssf2t,    dead_cps2, cps2_2p6b, cps2,    ROT0,   "bootleg", "Super Street Fighter II X: Grand Master Challenge (Japan 940223 Phoenix Edition) (bootleg)", GAME_SUPPORTS_SAVE )
GAME( 1997, sgemfd,   sgemf,    dead_cps2, cps2_2p3b, cps2,    ROT0,   "bootleg", "Super Gem Fighter Mini Mix (USA 970904 Phoenix Edition) (bootleg)", GAME_SUPPORTS_SAVE )
GAME( 2000, mmatrixd, mmatrix,  dead_cps2, cps2_2p1b, cps2,    ROT0,   "bootleg", "Mars Matrix: Hyper Solid Shooting (Japan 000412 Phoenix Edition) (bootleg)", GAME_SUPPORTS_SAVE )
GAME( 2000, dimahoud, dimahoo,  dead_cps2, cps2_2p3b, cps2,    ROT270, "bootleg", "Dimahoo (USA 000121 Phoenix Edition) (bootleg)", GAME_SUPPORTS_SAVE )
