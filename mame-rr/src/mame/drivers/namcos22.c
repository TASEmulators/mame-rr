/**
 * @file src/drivers/namcos22.c
 *
 * This driver describes Namco's System22 and Super System 22 hardware.
 *
 * driver provided with thanks to:
 * - pstroffo@yahoo.com (Phil Stroffolino)
 * - R. Belmont
 * - trackmaster@gmx.net (Bjorn Sunder)
 * - team vivanonno
 *
 * Input
 *      - input ports require manual calibration through built-in diagnostics (or canned EEPROM)
 *      - new input port type needed for bicycle pedal speed
 *      - new input port needed for multi-value stick shift
 *      - text layer row placement may be incorrect (emulated Prop Cycle differs from real game)
 *
 * Output Devices
 *      - Prop Cycle fan (outputs noted at the right MCU port)
 *      - lamps/LEDs on some cabinets
 *      - time crisis has force feedback for the guns
 *
 * Link
 *      - SCI (link) feature is not yet hooked up
 *
 * CPU Emulation issues
 *      - slave DSP is not yet used in-game
 *      - "point RAM" is not yet used in-game
 *
 * Notes:
 *      The "dipswitch" settings are ignored in many games - this isn't a bug.  For example, Prop Cycle software
 *      explicitly clears the chunk of work RAM used to cache the 8 bit dipswitch value immediately after
 *      populating it.  This is apparently done to hide secret debug routines from release versions of games.
 *
 * Self Test Info:
 *    Prop Cycle DSP tests:
 *        MD ROM-PROGRAM      (not-yet-working)
 *        MD-DOWNLOAD       - confirms that main CPU can upload custom code to master DSP
 *        MD-EXTERNAL-RAM   - tests private RAM used by master DSP
 *        C-RAM CHECK BY MD - tests communications RAM, shared by master DSP and main CPU
 *        PDP LOOP TEST     - exercises multi-command DMA transfer
 *        PDP BLOCK MOVE    - exercises DMA block transfer
 *        POINT RAM         - RAM test for "Point RAM" (not directly accessible by any CPU)
 *        CUSTOM IC STATUS  - currently hacked to always report "good"
 *        SD ROM-PRG        - confirms that master DSP can upload custom code to run on slave DSP
 *        SD EXTERNAL-RAM   - tests private RAM used by slave DSP
 *        DATA FLOW TEST 1    (not-yet-working)
 *        DATA FLOW TEST 2    (not-yet-working)
 *        POINT ROM TEST    - checksums the "Point ROMs"
 *
 *    Ridge Racer (Japan) tests:
 *        TEST 1a: MD-EXTERNAL-RAM
 *        TEST 1b: C-RAM CHECK BY MD
 *        TEST 2:  (not-yet-working)
 *        TEST 3:  SD EXTERNAL-RAM
 *        TEST 4:  DATA FLOW TEST 1 (not-yet-working)
 *        TEST 5:  DATA FLOW TEST 2 (not-yet-working)
 *        TEST 6:  POINT RAM
 *
 * IO MCU:
 * - generates sound/music
 * - provides input port management (copying to shared RAM)
 * - coinage handling in most games
 * - manages external physical devices (i.e. lamps, fans, force feedback)
 * - C74 is sound MCU, Mitsubishi M37702 MCU with mask ROM
 * - some external subroutines for C74 are also embedded
 *
 * Master DSP:
 * - S22 has two TI320C25 DSP (printed as C71)
 * - the master DSP provides display list parsing
 *
 * Slave DSP:
 * - serves as a calculation engine for lighting
 *
 * Communications RAM
 * - seen as 32 bit memory by main 68k CPU
 * - seen as 16 bit memory by master DSP (addr 0x8000..0xffff); upper/lower word is selectable
 * - not addressable by slave DSP
 *
 * Point ROMs
 * - encodes 3d model data
 * - not directly addressable by any CPU
 *
 * Point RAM
 * - same address space as Point ROMs
 * - not directly addressable by any CPU
 *
 * Link Feature:
 * - some (typically racing) games may be linked together
 * - serial controller is C139 SCI (same as System21).
 *
 * "Super" System22
 * - different memory map
 * - different CPU controller register layout
 * - sound CPU uses external ROM (i.e. pr1data.8k) instead of internal BIOS (C74)
 * - additional 2d sprite layer
 * - Point RAM starts at 0xf80000 rather than 0xf00000
 * - "PDP" device, for automated block memory transfers (dsp ram, point ram)
 *
 **********************************************************************************************************
 * SYSTEM22 Known Custom Chips
 *
 * CPU PCB:
 *
 *  C71 TI TMS320C25 DSP
 *  C71 WEYW40116 (TMS320C25 Main/Sub DSP)
 *  C71 D72260FN 980 FE-5CA891W
 *
 *  M5M5178AP-25 (CPU 16R, 17R, 19R, 20R) DSP Work RAM (8K x 8bit x 2 x 2)
 *
 *  C74 Mitsubishi M37702 MCU
 *  C74 159 408600 (OLD SUB)
 *  C74 159 543100 (NEW SUB)
 *  C74 159 414600 (OLD I/O)
 *  C74 159 437600 (NEW I/O)
 *
 * C195 (Shared SRAM Controller)
 * C196 CPP x 6
 * C199 (CPU 18K) x 1
 * C317 IDC (CPU 15E) x 1 (S21B)
 * C337 PFP x 1
 * C342 x 1 (S21B)
 * C352 (32ch PCM)
 * C353 x 1
 *
 *
 * VIDEO PCB:
 *
 * C304 - 4 chips on SS22 Video PCB
 * C305 (Palette)
 * C335
 *  9C, 10C
 *  12D, 12E
 *  14C
 * C300
 *  18B, 18C, 20B, 20C, 22B, 22C
 * Cxxx
 *  34R, 35R
 *
 * TI TBP28L22N (256 x 8bit PROM)
 *  VIDEO 2D, 3D, 4D (RGB Gamma LUT ROM)
 *
 *  RR1.GAM (for Ridge Racer 1/2, Rave Racer)
 **********************************************************************************************************
 *
 * Namco Super System 22 Hardware Overview (last updated 26th April 2009 at 4:11pm)
 * ---------------------------------------
 *
 * Note! This document will be updated from time to time when more dumps are available.
 *
 * This document covers all the known Namco Super System 22 games, including....
 * Air Combat 22    (C) Namco, 1995
 * Alpine Racer 1   (C) Namco, 1995
 * Alpine Racer 2   (C) Namco, 1996
 * Alpine Surfer    (C) Namco, 1996
 * Aqua Jet         (C) Namco, 1996
 * *Armidillo Racing(C) Namco, 1997
 * Cyber Cycles     (C) Namco, 1995
 * Dirt Dash        (C) Namco, 1995
 * Prop Cycle       (C) Namco, 1996
 * Time Crisis      (C) Namco, 1995
 * Tokyo Wars       (C) Namco, 1996
 *
 * * - denotes not available. If you can help with the remaining undumped SS22 game other other revisions of the
 *     dumped games, please contact http://guru.mameworld.info/
 *
 * The Namco Super System 22 System comprises 4 PCB's plugged into a motherboard. The motherboard contains only
 * some slots and connectors. The 4 PCB's are housed in a metal box with a large fan on the side. The fan mostly cools
 * the video board as these are known to run hot and commonly fail, especially now the system is 10 years+ old.
 *
 * CPU PCB   - There are four known revisions of this PCB. Three of them have an extra connector for an
 *             auxillary PCB. One of the others doesn't have that connector but is are otherwise identical.
 *             All PCBs can be swapped to any game and it will work. However, ALL required IC's must be swapped.
 *             This includes Program ROM PCB, socketed Keycus IC, socketed DATA ROM and socketed WAVE ROM(s).
 *             On most games the EEPROM will re-init itself on bootup. On the others, the EEPROM can re-init itself
 *             to defaults by holding down SERVICE + TEST on power-up. All games are swappable to ANY CPU PCB and will
 *             run ok (all dumped games have been swapped/tested and work fine)
 * DSP PCB   - There is only 1 revision of this PCB. All games use the exact same PCB. The DSP PCB can be swapped to
 *             any other game and works fine (all dumped games tested). Note that some games use different parts of the
 *             DSP PCB and some do more thorough tests on bootup so a DSP PCB that works fine in one game may come
 *             up as faulty in a different SS22 game.
 * MROM PCB  - These PCB's have many SOP44 ROMs on them and are identical for each game, but the contents of the ROMs
 *             and the number of ROMs vary per game. (a few of the dumped games have had their surface mounted ROMs
 *             swapped to other PCBs and worked fine)
 * FLASH PCB - Flash ROM board used only for Armidillo Racing and contains many TSOP56 16M FlashROMs
 * VIDEO PCB - There are three known revisions of this PCB. They're mostly identical apart from some component shuffling
 *             and in the earlier versions (A & B), an Altera FPGA chip is used instead of a Namco custom chip.
 *             All revisions of the Video PCBs are swappable and fully compatible with any SS22 game. The Altera FPGA runs
 *             very hot and almost always fails even if heatsinked!
 * MOTHER PCB- There are probably 3 revisions of this PCB, but only the original revision and (C) are documented here.
 *             The differences are very minor, just the amount of connectors on the PCB. The Mother PCB is swappable to
 *             any game as long as the required connectors for that game are present on the PCB. (all dumped games tested
 *             and worked fine using any MOTHER PCB)
 *
 * Each game has a 2 or 3 digit letter code assigned to it. Then a number 1 or 2, Then a Rev. A/B/C/D which denotes the
 * software revision.
 * The 1 denotes a Japanese version. 2 denotes a World version. So far there are no other numbers used other than 1 or 2.
 * For World versions, usually only the main program uses a '2', the rest of the ROM labels use the Japanese region code '1'.
 * There is one exception so far. The World version of Alpine Racer 2, which uses a World version DATA ROM, and also one
 * of the WAVE ROMs is a World version, but one Japanese WAVE ROM is also used.
 * See the CPU PCB, Program ROM Daughterboard and MROM PCB texts below for more details on ROM usage.
 *
 * CPU PCB
 * -------
 * 1st Revision
 * SYSTEM SUPER22 CPU PCB 8646960102 (8646970102)
 *
 * 2nd Revision
 * SYSTEM SUPER22 CPU(B) PCB 8646962600 (8646972600)
 *
 * 3rd Revision
 * SYSTEM SUPER22 CPU(B) PCB 8646962600 (8646972601) <-- very minor?
 *
 * 4th Revision
 * SYSTEM SUPER22 CPU(B) PCB 8646962601 (8646972601) <-- very minor?
 * |--------------------------------------------------------------|
 * |      J6                         JC410               3771     |
 * |  N341256(x4)                                           DSW(4)|
 * |                               |-------|                     |--|
 * |                               |MC68EC |                     |  |
 * |                               |020FG25|                     |  |
 * |   *1                          |-------|                     |  |
 * |           DSW(8)        SS22C2         |------|             |  |
 * |                                        | C383 |             |  |
 * |   CAT28C64    KEYCUS    SS22C1         |      |             |  |
 * |                                        |------|             |  |
 * |              |-------|                                      |  |
 * |     N341256  |       |                 |------|             |  |
 * | *3  N341256  | C405  |                 | 139  |             |--|
 * |              |       |       CY7C182   |      |              |
 * | *2           |-------|                 |------|              |
 * |                                                             |--|
 * |        40.000MHz                         137                |  |
 * |                                                             |  |
 * |              DATA.8K         JP1        49.152MHz           |  |
 * |     |------|                 |-------|             4558     |  |
 * |     | C352 |      JP2        |M37710 |                      |  |
 * |     |      |                 |S4BFP  |                      |  |
 * |     |------|                 |-------|                      |  |
 * |LED(x8)                                                      |  |
 * |        J11         SS22C4                MB87078            |  |
 * |      WAVEA.2L                                      4558     |--|
 * | JP3                                                          |
 * |      WAVEB.1L                LC78815M      LC78815M          |
 * |--------------------------------------------------------------|
 * (logic chips omitted from the PCB layout)
 *
 * Notes:
 *       J6           : Custom Namco connector for plug-in program ROM PCB
 *       J11          : Custom Namco connector for optional plug-in WAVE ROM PCB (holds some SOP44 MASKROMs)
 *       JC410        : Custom Namco connector for Optional plug-in Auxillary PCB (e.g. Gun Control PCB used in Time Crisis
 *                      etc)
 *                      The connector is populated only on the 2nd revision CPU (B) PCB 8646962600 (8646972600)
 *                      and 3rd Revision CPU (B) PCB 8646962600 (8646972601)
 *       JP1          : Jumper for configuration of M37710 (tied to pins 26 & 27 of M37710). Set to 1-2
 *       JP2          : Jumper for configuration of DATA ROM. Set to 1-2 (labelled '4M'), alt. setting 2-3 (labelled '1M/2M')
 *       JP3          : Jumper for configuration of WAVE ROM. Set to 2-3 (labelled '42P32M'), alt. setting 1-2
 *                      (labelled 'OTHER'). 'OTHER' is used when the WAVE ROMs are 16MBit. If the WAVE ROMs are 32MBit
 *                      (i.e. JP3 = 2-3), they're programmed in BYTE mode.
 *       M37710       : Mitsubishi M37710 CPU. Used on SS22 as the sound CPU. Does not have internal ROM (QFP80)
 *       MB87078      : Fujitsu MB87078 electronic volume control IC (SOIC24)
 *       4558         : Op Amp (SOIC8)
 *       LC78815M     : Sanyo LC78815M 2-channel 16-bit D/A converter (x2, SOIC20)
 *       3771         : Fujitsu MB3771 master reset IC (SOIC8)
 *       DSW(4)       : 4 position DIP Switch (1,2,3 are ON, 4 is OFF)
 *       DSW(8)       : 8 position DIP Switch (All OFF)
 *       N341256      : NKK N341256 32k x8 SRAM (x6, SOJ28)
 *       CY7C182      : Cypress 8k x9 SRAM (SOJ28)
 *       MC68EC020FG25: Main CPU, Motorola 68EC020FG25 (QFP100), running at 24.576MHz. Clock source is Namco
 *                      custom clock divider 137. 68EC020 clock = Master Clock of 49.152MHz / 2
 *       C383         : Namco custom C383 (QFP100)
 *       139          : Namco custom 139 Serial Controller Interface IC (QFP64). NOTE! On some PCB's this chip
 *                      has been replaced by a custom C422 chip, though the PCB number is the same. Which means probably
 *                      the function of this chip matches 139
 *       137          : Namco custom clock divider IC (DIP28)
 *       C405         : Namco custom C405 (QFP176)
 *       C352         : Namco custom C352 PCM sound chip (QFP100)
 *       SS22C1       : PALCE 22V10H (PLCC28, labelled 'SS22C1')
 *       SS22C2       : PALCE 22V10H (PLCC28, labelled 'SS22C2')
 *       SS22C4       : PALCE 22V10H (PLCC28, labelled 'SS22C4')
 *       CAT28C64     : Catalyst Semiconductor Inc. CAT28C64 8k x8 EEPROM (DIP28)
 *       LEDS         : 8 LEDs of various colors flash (in various pretty patterns) when the CPU PCB is active.
 *       KEYCUS       : Namco custom (DIP32)
 *                                          Air Combat 22    = none
 *                                          Alpine Racer 1   = C391
 *                                          Alpine Racer 2   = C434
 *                                          Alpine Surfer    = C425
 *                                          Aqua Jet         = C429
 *                                          Armidillo Racing = C433
 *                                          Cyber Cycles     = C389
 *                                          Dirt Dash        = C418
 *                                          Prop Cycle       = C428
 *                                          Time Crisis      = C419
 *                                          Tokyo Wars       = C424
 *
 *      *1            : Unpopulated position for PAL16V8 (PLCC20)
 *      *2            : Unpopulated position for Fujitsu MB86601 (QFP100)
 *      *3            : Unpopulated position for 32MHz OSC
 *      DATA.8K       : ST Microelectronics M27C4096 256k x16 EPROM (DIP40, labelled with the gamecode + 'DATA')
 *                      Game                EPROM label
 *                      -------------------------------
 *                      Air Combat 22      'ACS1 DATA'
 *                      Alpine Racer 1     'AR1 DATA B'
 *                      Alpine Racer 2     'ARS2 DATA'
 *                      Alpine Surfer      'AF1 DATA'
 *                      Aqua Jet           'AJ1 DATA'
 *                      Armidillo Racing   'AR DATA'
 *                      Cyber Cycles       'CB1 DATA B'
 *                      Dirt Dash          'DT1 DATA A'
 *                      Prop Cycle         'PR1 DATA'
 *                      Time Crisis        'TS1 DATA'
 *                      Tokyo Wars         'TW1 DATA'
 *
 *      WAVEA.2L   \
 *      WAVEB.1L   /  : 16M/32M WAVE MASKROMs. If 32MBit DIP42, they're programmed in BYTE mode (DIP42/SOP44)
 *                      Game                Wave A        Wave B        Type
 *                      ----------------------------------------------------------------------------
 *                      Air Combat 22      'ACS1 WAVE0', 'ACS1 WAVE1' , both SOP44 32M MASKROMs
 *                      Alpine Racer 1     'AR1 WAVEA' ,              , DIP42 16M MASKROM
 *                      Alpine Racer 2     'ARS1 WAVEA', 'ARS2 WAVE B', both DIP42 32M MASKROMs
 *                      Alpine Surfer      'AF1 WAVEA' ,              , DIP42 32M MASKROM
 *                      Aqua Jet           'AJ1 WAVEA' , 'AJ1 WAVEB'  , both DIP42 32M MASKROMs
 *                      Armidillo Racing   'AR WAVEA'  , 'AR WAVEB'   , both TSOP56, mounted on a DIP48 adapter board
 *                      Cyber Cycles       'CB1 WAVEA' , 'CB1 WAVEB'  , WAVE A DIP42 32M MASKROM, WAVE B DIP42 16M MASKROM
 *                      Dirt Dash          'DT1 WAVEA' , 'DT1 WAVEB'  , both DIP42 32M MASKROMs
 *                      Prop Cycle         'PR1 WAVE A', 'PR1 WAVE B' , both DIP42 32M MASKROM
 *                      Time Crisis        'TS1 WAVE A', 'TS1 WAVE B' , WAVE A DIP42 32M MASKROM, WAVE B DIP42 16M MASKROM
 *                      Tokyo Wars         'TW1 WAVE A',              , DIP42 32M MASKROM
 *
 *
 * PROGRAM ROM Daughterboard PCB
 * -----------------------------
 * This PCB holds the main program ROMs. There is a small sticker on each PCB stating the game code and software revision.
 * The actual ROMs are not marked with any codes except the manufacturer's part number.
 * There are 4 known types of program daughterboards used on SS22 games (so far). The most common is the first type.
 * The PCB is very small (approx 2" x 3") containing one custom connector and some FlashROMs, and a PAL (in some cases).
 * The ones that contain a PAL are approx 3" x 3".
 *
 * Type 1
 * SYSTEM SUPER22 MPM(F) PCB 8646961600 (8646971600)
 * |-------------------------|
 * |                         |
 * |                         |
 * | ROM1  ROM2  ROM3  ROM4  |
 * |                         |
 * |                         |
 * |  |-------------------|  |
 * |  |-------------------|  |
 * |-------------------------|
 * Notes:
 *       ROMx: Intel E28F008SA 8MBit FlashROM (x4, TSOP40)
 *
 *       This PCB is used on:
 *                           Game          Software revision
 *                           -------------------------------
 *                           Air Combat 22    'ACS1 Ver.B'
 *                           Alpine Racer     'AR2 Ver.C'
 *                           Alpine Racer     'AR2 Ver.D'
 *                           Aqua Jet         'AJ2 Ver.B'
 *                           Armidillo Racing 'AR' Ver.?
 *                           Cyber Cycles     'CB2 Ver.C'
 *                           Prop Cycle       'PR2 Ver.A'
 *                           Time Crisis      'TS2 Ver.B'
 *                           Tokyo Wars       'TW2 Ver.A'
 *
 * Type 2
 * SYSTEM SUPER22 MPM(F16) PCB 8646962500 (8646972500)
 * |-------------------------|
 * |                         |
 * |                         |
 * |      ROM1     ROM2      |
 * |                         |
 * |                         |
 * |                         |
 * |  |-------------------|  |
 * |  |-------------------|  |
 * |-------------------------|
 * Notes:
 *       ROMx: Intel E28F016SA 16MBit FlashROMs (x2, TSOP56)
 *
 *       This PCB is used on:
 *                           Game          Software revision
 *                           -------------------------------
 *                           Time Crisis   'TS2 Ver.A'
 *                           Dirt Dash     'DT2 Ver.A'
 *
 * Type 3
 * SYSTEM SUPER22 MPM(F16X4) PCB 8646962901 (8646972901)
 * |-------------------------|
 * |SS22P1B                  |
 * |                         |
 * |        IC4_LM    IC5_UU |
 * |                         |
 * |                         |
 * |  IC2_LL     IC3_UM      |
 * |                         |
 * |                         |
 * |  |-------------------|  |
 * |  |-------------------|  |
 * |-------------------------|
 * Notes:
 *       ICx*   : Intel E28F016SA 16MBit FlashROMs (x4, TSOP56)
 *       SS22P1B: PALCE16V8H (PLCC20, labelled 'SS22P1B')
 *
 *       This PCB is used on:
 *                           Game           Software revision
 *                           -------------------------------
 *                           Alpine Racer 2 'ARS2 Ver.B'
 *
 * Type 4
 * SYSTEM SUPER22 MPM(F16X4F) PCB 8646963500 (8646973500)
 * SYSTEM SUPER22 MPM(F16X4F) PCB 8646963501 (8646973501) <-- very minor?
 * |-------------------------|
 * |SS22P1B                  |
 * |                         |
 * |                         |
 * | IC2_LL     IC4_UM       |
 * |                         |
 * |      IC3_LM      IC5_UU |
 * |                         |
 * |                         |
 * |  |-------------------|  |
 * |  |-------------------|  |
 * |-------------------------|
 * Notes:
 *       ICx    : Fujutsu 29F016 16MBit FlashROMs (x4, TSOP48)
 *       SS22P1B: PALCE16V8H (PLCC20, labelled 'SS22P1B')
 *                There is a small mod on this PAL as follows....
 *                - Pin 8 is lifted and the pad on the PCB at pin 8
 *                  is tied to pin 16 of the PAL (no other pins are lifted)
 *                - The lifted pin 8 is tied to the MPM PCB connector on pin 1
 *                  That pin traces to pin 6 (OUTPUT Y) of a 74F08 at 15F on the CPU board
 *                  Pins 4 (INPUT A) & 5 (INPUT B) of the 74F08 at 15F trace to Namco custom
 *                  IC C383 pins 53 (A INPUT) & 52 (B INPUT)
 *
 *       This PCB is used on:
 *                           Game           Software revision
 *                           --------------------------------
 *                           Alpine Surfer  'AF2 Ver.A'   note: with PAL modification and using 8646963500 PCB
 *                           Alpine Racer 2 'ARS2 Ver.A'  note: without PAL modification and using 8646963501 PCB
 *
 *
 * Auxillary PCB (connector JC410 on the CPU PCB is used only for Time Crisis)
 * -------------
 * V159 GUN POINT PCB 244790102 (2447970102)
 * |---------------------------------|
 * |                                 |
 * |                      25.175MHz  |
 * |                                 |
 * |                                 |
 * |                                 |
 * |                                 |
 * |                                 |
 * |                                 |
 * |                               J2|
 * |                                 |
 * |                                 |
 * |                                 |
 * |                                 |
 * |                                 |
 * |                                 |
 * |                J1               |
 * |---------------------------------|
 * Notes:
 *       There isn't much on this PCB other than 2 connectors and an oscillator plus a large amount of logic chips.
 *       J1 : Connector joining Gun PCB to a connector on the metal box (which joins to the gun interface PCB in the
 *            cab which supplies 24V for the solenoid in the guns)
 *       J2 : Connector joining to the CPU PCB (to JC410)
 *
 *
 * DSP PCB
 * -------
 * SYSTEM SUPER22 DSP PCB 8646960302 (8646970302)
 *   |--------------------------------------------------------------|
 *   | N341256     |-------|   SS22D1    N341256     |-------|      |
 *   | N341256     |       |             N341256     |       |      |
 *   | |---------| | C71   |                         | C405  |     |--|
 * |--||         | |       |                         |       |     |  |
 * |  ||  C396   | |-------|                         |-------|     |  |
 * |  ||         |                                                 |  |
 * |J ||         |           N341256                 |-------|     |  |
 * |D ||---------|           N341256                 |       |     |  |
 * |3 | |-------|                          N341256   | C405  |     |  |
 * |  | |       |                                    |       |     |  |
 * |  | | C71   |  |---------|                       |-------|     |  |
 * |--| |       |  |         |             |------|                |  |
 *   |  |-------|  |  342    |             | C379 |                |--|
 *   |  |------|   |         |             |      |                 |
 *   |  | C199 |   |         |             |------|  N341256        |
 *   |  |      |   |---------|                       N341256       |--|
 *   |  |------|                                                   |  |
 *   |   |-----|      SS22D5  SS22D4B    40MHz     |---------|     |  |
 *   |   |C353 |                                   |         |     |  |
 *   |   |-----|                                   |  C300   |     |  |
 *   |LED(x8)         SS22D2  SS22D3               |         |     |  |
 *   |   |-----|                                   |         |     |  |
 *   |   | 402 |  |---------|                      |---------|     |  |
 *   |   |-----|  |         |                                      |  |
 *   |            |  C403   |    KM681000   KM681000   KM681000    |  |
 *   |   |-----|  |         |                                      |--|
 *   |   | 402 |  |         |    KM681000   KM681000   KM681000     |
 *   |   |-----|  |---------|                                       |
 *   |--------------------------------------------------------------|
 * (logic chips omitted from the PCB layout)
 *
 * Notes:
 *       JD3          : Custom Namco connector joining this PCB to the MROM PCB with a special flat cable known as a
 *                      'DHD harness'
 *       N341256      : NKK N341256 32k x8 SRAM (x9, SOJ28)
 *       KM681000     : Samsung Electronics KM681000 128k x8 SRAM (x6, SOP32)
 *       342          : Namco custom 342  (QFP160)
 *       402          : Namco custom 402  (x2, TQFP144)
 *       C71          : Namco custom C71, actually a Texas Instruments 320C25 DSP (x2, PLCC68)
 *       C199         : Namco custom C199 (QFP100)
 *       C300         : Namco custom C300 (QFP160)
 *       C353         : Namco custom C353 (QFP120)
 *       C379         : Namco custom C379 (QFP64)
 *       C396         : Namco custom C396 (QFP160)
 *       C403         : Namco custom C403 (QFP136)
 *       C405         : Namco custom C396 (x2, QFP176)
 *       SS22D1       : PALCE 20V8H (PLCC28, labelled 'SS22D1')
 *       SS22D2       : PALCE 16V8H (PLCC20, labelled 'SS22D2')
 *       SS22D3       : PALCE 16V8H (PLCC20, labelled 'SS22D3')
 *       SS22D4B      : PALCE 16V8H (PLCC20, labelled 'SS22D4B')
 *       SS22D5       : PALCE 16V8H (PLCC20, labelled 'SS22D5')
 *       LEDS         : 8 red LEDs flash (in various pretty patterns) when the DSP PCB is active.
 *
 *
 * ROM PCB (type 1)
 * -------
 * SYSTEM SUPER22 MROM PCB 8646960400 (8646970400)
 *   |--------------------------------------------------------------|
 *   | SS22M3      TC551001(x3)                                     |
 *   | JP13                                 CG7.19D    CG5.19B      |
 *   |  PTR-L0.18K  PTR-M0.18J  PTR-U0.18F                          |
 * |--|                                     CG6.18D    CG4.18B      |
 * |  | PTR-L1.16K  PTR-M1.16J  PTR-U1.16F                   CG6.18A|
 * |  |                                     CG5.16D    CG3.16B      |
 * |J | PTR-L2.15K  PTR-M2.15J  PTR-U2.15F                   CG7.15A|
 * |R |                                     CG4.14D    CG2.14B      |
 * |4 | PTR-L3.14K  PTR-M3.14J  PTR-U3.14F                      JP1 |
 * |  |  SCG0.12L              SCG0.12F JP4 CG3.13D    CG1.13B  JP2 |
 * |  |                                 JP5                     JP3 |
 * |--|              SCG7.10J           JP6 CG2.12D    CG0.12B      |
 *   |   SCG1.10L              SCG1.10F                      SS22M1 |
 *   |                                      CG1.10D    SS22M2       |
 *   |                                                              |
 *   |   SCG2.8L     JP10      SCG2.8F      CG0.8D                 |--|
 * |--|              JP11                                          |  |
 * |  |              JP12                                          |  |
 * |  |  SCG3.7L               SCG3.7F      SS22M2     CCR-L.7B    |  |
 * |J |                                                            |  |
 * |R |                                                CCR-H.5B    |  |
 * |3 |  SCG4.5L               SCG4.5F                             |  |
 * |  |              JP7                                           |  |
 * |  |              JP8                                           |  |
 * |--|  SCG5.3L     JP9       SCG5.3F      CCR-L.3D               |  |
 *   |                                                             |--|
 *   |               SCG7.1J                CCR-H.1D                |
 *   |   SCG6.1L               SCG6.1F                              |
 *   |--------------------------------------------------------------|
 * (logic chips omitted from the PCB layout)
 *
 * Notes:
 *       Namco SS22 MROM PCBs have 2 identical sets of CG*, SCG* and CCR-L/CCR-H ROMs on the PCB.
 *       The Japanese region code '1' is appended to all game codes on all MROMs.
 *
 *       JR3, JR4     : Custom Namco connector joining this PCB to the VIDEO & DSP PCBs with a special flat cable known
 *                      as a 'DHD harness'
 *       SS22M1       : PALCE 16V8H (PLCC20, labelled 'SS22M1')
 *       SS22M2       : PALCE 20V8H (x2, PLCC28, labelled 'SS22M2')
 *       SS22M3       : PALCE 16V8H (PLCC20, labelled 'SS22M3')
 *       TC551001     : Toshiba TC551001 128k x8 SRAM (SOP32)
 *       JP1, JP2, JP3: Jumpers to configure CG* ROMs.  Hardwired to '16M' on the PCB. Alt. setting '32M'
 *       JP4, JP5, JP6: Jumpers to configure CG* ROMs.  Hardwired to '16M' on the PCB. Alt. setting '32M'
 *       JP7, JP8, JP9: Jumpers to configure SCG* ROMs. Hardwired to '16M' on the PCB. Alt. setting '32M'
 *    JP10, JP11, JP12: Jumpers to configure SCG* ROMs. Hardwired to '16M' on the PCB. Alt. setting '32M'
 *       JP13         : Jumper  to configure PTR* ROMs. Hardwired to '4M'  on the PCB. Alt. setting '8M'
 *
 * Game               ROMs populated
 * ---------------------------------------------------------
 * Air Combat 22      ACS1CCRH.5B, ACS1CCRH.1D    4M SOP32
 *                    ACS1CCRL.7B, ACS1CCRL.3D   16M SOP44
 *                    ACS1CG0.12B, ACS1CG0.8D        "
 *                    ACS1CG1.13B, ACS1CG1.10D       "
 *                    ACS1CG2.14B, ACS1CG2.12D       "
 *                    ACS1CG3.16B, ACS1CG3.13D       "
 *                    ACS1CG4.18B, ACS1CG4.14D       "
 *                    ACS1CG5.19B, ACS1CG5.16D       "
 *                    ACS1CG6.18A, ACS1CG6.18D       "
 *                    ACS1CG7.15A, ACS1CG7.19D       "
 *                    ACS1SCG0.12F,ACS1SCG0.12L      "
 *                    ACS1SCG1.10F,ACS1SCG1.10L      "
 *                    ACS1PTRU0.18F               4M SOP32
 *                    ACS1PTRU1.16F                  "
 *                    ACS1PTRU2.15F                  "
 *                    ACS1PTRU3.14F                  "
 *                    ACS1PTRM0.18J                  "
 *                    ACS1PTRM1.16J                  "
 *                    ACS1PTRM2.15J                  "
 *                    ACS1PTRM3.14J                  "
 *                    ACS1PTRL0.18K                  "
 *                    ACS1PTRL1.16K                  "
 *                    ACS1PTRL2.15K                  "
 *                    ACS1PTRL3.14K                  "
 *
 * Alpine Racer 1     AR1CCRH.5B, AR1CCRH.1D      4M SOP32
 *                    AR1CCRL.7B, AR1CCRL.3D     16M SOP44
 *                    AR1CG0.12B, AR1CG0.8D          "
 *                    AR1CG1.13B, AR1CG1.10D         "
 *                    AR1CG2.14B, AR1CG2.12D         "
 *                    AR1CG3.16B, AR1CG3.13D         "
 *                    AR1CG4.18B, AR1CG4.14D         "
 *                    AR1CG5.19B, AR1CG5.16D         "
 *                    AR1CG6.18A, AR1CG6.18D         "
 *                    AR1CG7.15A, AR1CG7.19D         "
 *                    AR1SCG0.12F,AR1SCG0.12L        "
 *                    AR1SCG1.10F,AR1SCG1.10L        "
 *                    AR1PTRU0.18F                4M SOP32
 *                    AR1PTRU1.16F                   "
 *                    AR1PTRU2.15F                   "
 *                    AR1PTRU3.14F                   "
 *                    AR1PTRM0.18J                   "
 *                    AR1PTRM1.16J                   "
 *                    AR1PTRM2.15J                   "
 *                    AR1PTRM3.14J                   "
 *                    AR1PTRL0.18K                   "
 *                    AR1PTRL1.16K                   "
 *                    AR1PTRL2.15K                   "
 *                    AR1PTRL3.14K                   "
 *
 * Alpine Racer 2     ARS1CCRH.5B, ARS1CCRH.1D    4M SOP32
 *                    ARS1CCRL.7B, ARS1CCRL.3D   16M SOP44
 *                    ARS1CG0.12B, ARS1CG0.8D        "
 *                    ARS1CG1.13B, ARS1CG1.10D       "
 *                    ARS1CG2.14B, ARS1CG2.12D       "
 *                    ARS1CG3.16B, ARS1CG3.13D       "
 *                    ARS1CG4.18B, ARS1CG4.14D       "
 *                    ARS1CG5.19B, ARS1CG5.16D       "
 *                    ARS1SCG0.12F,ARS1SCG0.12L      "
 *                    ARS1PTRU0.18F               4M SOP32
 *                    ARS1PTRU1.16F                  "
 *                    ARS1PTRU2.15F                  "
 *                    ARS1PTRU3.14F                  "
 *                    ARS1PTRM0.18J                  "
 *                    ARS1PTRM1.16J                  "
 *                    ARS1PTRM2.15J                  "
 *                    ARS1PTRM3.14J                  "
 *                    ARS1PTRL0.18K                  "
 *                    ARS1PTRL1.16K                  "
 *                    ARS1PTRL2.15K                  "
 *                    ARS1PTRL3.14K                  "
 *
 * Alpine Surfer      AF1CCRH.5B, AF1CCRH.1D      4M SOP32
 *                    AF1CCRL.7B, AF1CCRL.3D     16M SOP44
 *                    AF1CG0.12B, AF1CG0.8D          "
 *                    AF1CG1.13B, AF1CG1.10D         "
 *                    AF1CG2.14B, AF1CG2.12D         "
 *                    AF1CG3.16B, AF1CG3.13D         "
 *                    AF1CG4.18B, AF1CG4.14D         "
 *                    AF1SCG0B.12F,AF1SCG0B.12L      "
 *                    AF1PTRU0.18F                4M SOP32
 *                    AF1PTRU1.16F                   "
 *                    AF1PTRM0.18J                   "
 *                    AF1PTRM1.16J                   "
 *                    AF1PTRL0.18K                   "
 *                    AF1PTRL1.16K                   "
 *
 * Aqua Jet           AJ1CCRH.5B, AJ1CCRH.1D      4M SOP32
 *                    AJ1CCRL.7B, AJ1CCRL.3D     16M SOP44
 *                    AJ1CG0.12B, AJ1CG0.8D          "
 *                    AJ1CG1.13B, AJ1CG1.10D         "
 *                    AJ1CG2.14B, AJ1CG2.12D         "
 *                    AJ1CG3.16B, AJ1CG3.13D         "
 *                    AJ1CG4.18B, AJ1CG4.14D         "
 *                    AJ1CG5.19B, AJ1CG5.16D         "
 *                    AJ1CG6.18A, AJ1CG6.18D         "
 *                    AJ1CG7.15A, AJ1CG7.19D         "
 *                    AJ1SCG0.12F,AJ1SCG0.12L        "
 *                    AJ1SCG1.10F,AJ1SCG1.10L        "
 *                    AJ1SCG2.8F, AJ1SCG2.8L         "
 *                    AJ1PTRU0.18F                4M SOP32
 *                    AJ1PTRU1.16F                   "
 *                    AJ1PTRU2.15F                   "
 *                    AJ1PTRU3.14F                   "
 *                    AJ1PTRM0.18J                   "
 *                    AJ1PTRM1.16J                   "
 *                    AJ1PTRM2.15J                   "
 *                    AJ1PTRM3.14J                   "
 *                    AJ1PTRL0.18K                   "
 *                    AJ1PTRL1.16K                   "
 *                    AJ1PTRL2.15K                   "
 *                    AJ1PTRL3.14K                   "
 *
 * Cyber Cycles       CB1CCRH.5B, CB1CCRH.1D      4M SOP32
 *                    CB1CCRL.7B, CB1CCRL.3D     16M SOP44
 *                    CB1CG0.12B, CB1CG0.8D          "
 *                    CB1CG1.13B, CB1CG1.10D         "
 *                    CB1CG2.14B, CB1CG2.12D         "
 *                    CB1CG3.16B, CB1CG3.13D         "
 *                    CB1CG4.18B, CB1CG4.14D         "
 *                    CB1CG5.19B, CB1CG5.16D         "
 *                    CB1CG6.18A, CB1CG6.18D         "
 *                    CB1SCG0.12F,CB1SCG0.12L        "
 *                    CB1SCG1.10F,CB1SCG1.10L        "
 *                    CB1PTRU0.18F                4M SOP32
 *                    CB1PTRU1.16F                   "
 *                    CB1PTRU2.15F                   "
 *                    CB1PTRU3.14F                   "
 *                    CB1PTRM0.18J                   "
 *                    CB1PTRM1.16J                   "
 *                    CB1PTRM2.15J                   "
 *                    CB1PTRM3.14J                   "
 *                    CB1PTRL0.18K                   "
 *                    CB1PTRL1.16K                   "
 *                    CB1PTRL2.15K                   "
 *                    CB1PTRL3.14K                   "
 *
 * Dirt Dash          DT1CCRH.5B, DT1CCRH.1D      4M SOP32
 *                    DT1CCRL.7B, DT1CCRL.3D     16M SOP44
 *                    DT1CG0.12B, DT1CG0.8D          "
 *                    DT1CG1.13B, DT1CG1.10D         "
 *                    DT1CG2.14B, DT1CG2.12D         "
 *                    DT1CG3.16B, DT1CG3.13D         "
 *                    DT1CG4.18B, DT1CG4.14D         "
 *                    DT1CG5.19B, DT1CG5.16D         "
 *                    DT1CG6.18A, DT1CG6.18D         "
 *                    DT1CG7.15A, DT1CG7.19D         "
 *                    DT1SCG0.12F,DT1SCG0.12L        "
 *                    DT1SCG1.10F,DT1SCG1.10L        "
 *                    DT1PTRU0.18F                4M SOP32
 *                    DT1PTRU1.16F                   "
 *                    DT1PTRU2.15F                   "
 *                    DT1PTRU3.14F                   "
 *                    DT1PTRM0.18J                   "
 *                    DT1PTRM1.16J                   "
 *                    DT1PTRM2.15J                   "
 *                    DT1PTRM3.14J                   "
 *                    DT1PTRL0.18K                   "
 *                    DT1PTRL1.16K                   "
 *                    DT1PTRL2.15K                   "
 *                    DT1PTRL3.14K                   "
 *
 * Prop Cycle         PR1CCRH.5B, PR1CCRH.1D      4M SOP32
 *                    PR1CCRL.7B, PR1CCRL.3D     16M SOP44
 *                    PR1CG0.12B, PR1CG0.8D          "
 *                    PR1CG1.13B, PR1CG1.10D         "
 *                    PR1CG2.14B, PR1CG2.12D         "
 *                    PR1CG3.16B, PR1CG3.13D         "
 *                    PR1CG4.18B, PR1CG4.14D         "
 *                    PR1CG5.19B, PR1CG5.16D         "
 *                    PR1CG6.18A, PR1CG6.18D         "
 *                    PR1CG7.15A, PR1CG7.19D         "
 *                    PR1SCG0.12F,PR1SCG0.12L        "
 *                    PR1SCG1.10F,PR1SCG1.10L        "
 *                    PR1PTRU0.18F                4M SOP32
 *                    PR1PTRU1.16F                   "
 *                    PR1PTRU2.15F                   "
 *                    PR1PTRM0.18J                   "
 *                    PR1PTRM1.16J                   "
 *                    PR1PTRM2.15J                   "
 *                    PR1PTRL0.18K                   "
 *                    PR1PTRL1.16K                   "
 *                    PR1PTRL2.15K                   "
 *
 * Time Crisis        TS1CCRH.5B, TS1CCRH.1D      4M SOP32
 *                    TS1CCRL.7B, TS1CCRL.3D     16M SOP44
 *                    TS1CG0.12B, TS1CG0.8D          "
 *                    TS1CG1.13B, TS1CG1.10D         "
 *                    TS1CG2.14B, TS1CG2.12D         "
 *                    TS1CG3.16B, TS1CG3.13D         "
 *                    TS1CG4.18B, TS1CG4.14D         "
 *                    TS1CG5.19B, TS1CG5.16D         "
 *                    TS1CG6.18A, TS1CG6.18D         "
 *                    TS1CG7.15A, TS1CG7.19D         "
 *                    TS1SCG0.12F,TS1SCG0.12L        "
 *                    TS1SCG1.10F,TS1SCG1.10L        "
 *                    TS1SCG2.8F, TS1SCG2.8L         "
 *                    TS1SCG3.7F, TS1SCG3.7L         "
 *                    TS1SCG4.5F, TS1SCG4.5L         "
 *                    TS1SCG5.3F, TS1SCG5.3L         "
 *                    TS1PTRU0.18F                4M SOP32
 *                    TS1PTRU1.16F                   "
 *                    TS1PTRU2.15F                   "
 *                    TS1PTRM0.18J                   "
 *                    TS1PTRM1.16J                   "
 *                    TS1PTRM2.15J                   "
 *                    TS1PTRL0.18K                   "
 *                    TS1PTRL1.16K                   "
 *                    TS1PTRL2.15K                   "
 *
 * Tokyo Wars         TW1CCRH.5B, TW1CCRH.1D      4M SOP32
 *                    TW1CCRL.7B, TW1CCRL.3D     16M SOP44
 *                    TW1CG0.12B, TW1CG0.8D          "
 *                    TW1CG1.13B, TW1CG1.10D         "
 *                    TW1CG2.14B, TW1CG2.12D         "
 *                    TW1CG3.16B, TW1CG3.13D         "
 *                    TW1CG4.18B, TW1CG4.14D         "
 *                    TW1CG5.19B, TW1CG5.16D         "
 *                    TW1CG6.18A, TW1CG6.18D         "
 *                    TW1CG7.15A, TW1CG7.19D         "
 *                    TW1SCG0.12F,TW1SCG0.12L        "
 *                    TW1SCG1.10F,TW1SCG1.10L        "
 *                    TW1SCG2.8F, TW1SCG2.8L         "
 *                    TW1SCG3.7F, TW1SCG3.7L         "
 *                    TW1PTRU0.18F                4M SOP32
 *                    TW1PTRU1.16F                   "
 *                    TW1PTRU2.15F                   "
 *                    TW1PTRU3.14F                   "
 *                    TW1PTRM0.18J                   "
 *                    TW1PTRM1.16J                   "
 *                    TW1PTRM2.15J                   "
 *                    TW1PTRM3.14J                   "
 *                    TW1PTRL0.18K                   "
 *                    TW1PTRL1.16K                   "
 *                    TW1PTRL2.15K                   "
 *                    TW1PTRL3.14K                   "
 *
 *
 * ROM PCB (type 2)
 * -------
 * SS22DS FLASH PCB 8650961300 (8650971300)
 *   |--------------------------------------------------------------|
 *   |     F13M  F13L                      F12J   F12E    |------|  |
 *   |                                                    |ALTERA|  |
 *   |     F12M  F12L                                     |EPMXXXX  |
 * |--|                                    F11J   F11E    |------|  |
 * |  | HM628128 F11L                                               |
 * |  | HM628128                                             DSF4   |
 * |J | HM628128 F9L                       F9J     F9E              |
 * |R |                                                             |
 * |4 | DSF5A                                                       |
 * |  |                                    F8J     F8E              |
 * |  |                                                             |
 * |--|                                                    SS22DSF3 |
 *   |           F7L   F7M                 F7J     F7E              |
 *   |                                                              |
 *   |                                                              |
 *   |                                     F6J     F6E     SS22DSF3|--|
 * |--|                                                            |  |
 * |  |                                                            |  |
 * |  |          F5L   F5M                 F5J     F5E             |  |
 * |J |                                                            |  |
 * |R |                                                            |  |
 * |3 |                                    F4J     F4E             |  |
 * |  |                                                            |  |
 * |  |          F3L   F3M                                         |  |
 * |--|                                                            |  |
 *   | SS22DSF2                            F2J     F2E             |--|
 *   |                                                              |
 *   | SS22DSF2  F1L   F1M                 F1J     F1E              |
 *   |--------------------------------------------------------------|
 * (logic chips omitted from the PCB layout)
 *
 * Notes:
 *       Namco SS22 FLASH PCBs have 2 identical sets of CG*, SCG* and CCR-L/CCR-H ROMs on the PCB.
 *
 *       JR3, JR4     : Custom Namco connector joining this PCB to the VIDEO & DSP PCBs with a special flat cable known
 *                      as a 'DHD harness'
 *       EPMXXXX      : Altera EPM??? (PLCC84, unknown chip model, possibly EPM7064, sticker on top of it blocking ID markings)
 *       DSF5A        : PALCE 16V8H (PLCC20, labelled 'DSF5A')
 *       DSF4         : PALCE 16V8H (PLCC20, labelled 'DSF4')
 *       SS22DSF3     : EPM7032 (x2, PLCC44, labelled 'SS22DSF3')
 *       SS22DSF2     : EPM7032 (x2, PLCC44, labelled 'SS22DSF2')
 *       HM628128     : Hitachi HM628128 128k x8 SRAM (TSOP32)
 *
 * Game               ROMs populated (All Intel E28F016SA TSOP56 16M FlashROMs)
 * -----------------------------------------------------------------
 * Armidillo Racing   F1E, F1J, F2E, F2J            - CCRL/CCRH ROMs
 *
 *                    F4E, F4J, F5E, F5J, F6E, F6J, \
 *                    F7E, F7J, F8E, F8J, F9E, F9J, \ CGx ROMs
 *                    F11E, F11J, F12E, F12J        /
 *
 *                    F1L, F1M, F3L, F3M, F5L, F5M, \
 *                    F7L, F7M                      / SCGx ROMs
 *
 *                    F9L, F11L, F12L, F13L,        \
 *                    F12M, F13M                    / PTR ROMs
 *
 *
 * VIDEO PCB
 * ---------
 * 1st Revision
 * SYSTEM SUPER22 VIDEO 8646960204 (8646970204)
 *
 * 2nd Revision
 * SYSTEM SUPER22 VIDEO(B) 8646961200 (8646971200)
 *
 * 3rd Revision (PCB layout shown below)
 * SYSTEM SUPER22 VIDEO(C) 8646962700 (8646972700)
 *   |--------------------------------------------------------------|
 *   |                                                              |
 *   |                    HM534251BJ-8(x22)                         |
 *   |                                                   |-----|   |--|
 *   |  |---------| |---------| |---------| |---------|  |C401 |   |  |
 *   |  |         | |         | |         | |         |  |-----|   |  |
 *   |  |   304   | |   304   | |   304   | |   304   |            |  |
 *   |  |         | |         | |         | |         |  |-----|   |  |
 *   |  |         | |         | |         | |         |  |C400 |   |  |
 *   |  |---------| |---------| |---------| |---------|  |-----|   |  |
 *   |  N341256      |-----| |-----| |-----| |-----|               |  |
 *   |  N341256      |C407 | |C401 | |C401 | |C401 |  |---------|  |  |
 *   |               |-----| |-----| |-----| |-----|  |         |  |  |
 *   |   |---------|                                  |  C399   |  |--|
 *   |   |         |         |-----| |-----| |-----|  |         |   |
 *   |   |  C387   |         |C400 | |C400 | |C400 |  |         |   |
 *   |   |         |         |-----| |-----| |-----|  |---------|  |--|
 * |--|  |         |                    |-----|            N341256 |  |
 * |  |  |---------|                    |C406 |  |-----|   N341256 |  |
 * |  |  |-----|                        |-----|  |C381 |           |  |
 * |J |  |C381 |                |---------|      |-----|           |  |
 * |V |  |-----|      N341256   |         |                LC321664|  |
 * |3 |     51.2MHz   N341256   |  C404   |      |---------|       |  |
 * |  | |---------| |---------| |         |      |         |       |  |
 * |  | |         | |         | |         |      |  C361   |       |  |
 * |--| |  C374   | |  C395   | |---------|      |         |       |  |
 *   |  |         | |         | N341256          |         |       |--|
 *   |  |         | |         | N341256          |---------|        |
 *   |  |---------| |---------| N341256                    CXD1178Q |
 *   |--------------------------------------------------------------|
 * (logic chips omitted from the PCB layout)
 *
 * Notes:
 *       JV3          : Custom Namco connector joining this PCB to the MROM PCB with a special flat cable known
 *                      as a 'DHD harness'
 *       N341256      : NKK N341256 32k x8 SRAM (x9, SOJ28)
 *       HM534251BJ-8 : Hitachi HM534251BJ-8 256k x4 FASTPAGE DRAM (x22, SOJ28)
 *       LC321664     : Sanyo LC321664 64k x16 DRAM (SOJ40)
 *       CXD1178Q     : Sony CXD1178Q 8-bit RGB 3-channel D/A converter (QFP48)
 *       304          : Namco custom 304  (x4, QFP120)
 *       C361         : Namco custom C361 (QFP120)
 *       C374         : Namco custom C374 (QFP160)
 *       C381         : Namco custom C381 (x2, QFP144)
 *       C387         : Namco custom C387 (QFP160)
 *       C395         : Namco custom C395 (QFP168)
 *       C399         : Namco custom C399 (QFP160)
 *       C400         : Namco custom C400 (x4, QFP100)
 *                      - x3 on 1st Revision
 *       C401         : Namco custom C401 (x4, QFP64)
 *                      - x5 on 1st Revision
 *       C404         : Namco custom C404 (QFP208)
 *       C406         : Namco custom C406 (TQFP120)
 *       C407         : Namco custom C407 (QFP64) NOTE! On Revision A & B, this position is populated by an
 *                                                      Altera EPM7064 PLCC84 FPGA labelled 'SS22V1B'
 *                                                      The Altera chip runs very hot and fails quite often.
 *                                                      Even if a heaksink is added to the chip it still fails.
 *                                                      The failure of this chip is the primary cause of
 *                                                      video faults on Namco Super System 22 PCBs.
 *                                                      (Second reason for video faults is generally attributed
 *                                                      to failure of RAM on this PCB and/or the DSP PCB)
 *
 *
 * Motherboard PCB
 * ---------------
 * 1st Revision
 * SYSTEM SUPER22 MOTHER PCB 8646960602 (8646970602)
 *
 * 2nd Revision
 * SYSTEM SUPER22 MOTHER(B) PCB (number not known)
 *
 * 3rd Revision
 * SYSTEM SUPER22 MOTHER(C) PCB 8646960602 (8646970602)
 * |------------------------------------------------------------------|
 * | J1                  J2           J4           J6           J8    |
 * |           J10                           J5   IC2     J7          |
 * |                     J3          IC3          IC1              J9 |
 * |                                                                  |
 * | JC2 |---------------------|        |---------------------|JC1    |
 * |     |---------------------|        |---------------------|       |
 * |                                                                  |
 * | JD2 |---------------------|        |---------------------|JD1    |
 * |     |---------------------|        |---------------------|       |
 * |                                                                  |
 * |                                    |---------------------|JR1    |
 * |                                    |---------------------|       |
 * |                                                                  |
 * | JV2 |---------------------|        |---------------------|JV1    |
 * |     |---------------------|        |---------------------|       |
 * |------------------------------------------------------------------|
 * Notes:
 *       IC1     : LB1233 (DIP8)
 *       IC2, IC3: LB1235 (DIP8)
 *
 *       JC1, JC2: Connectors to plug in CPU PCB
 *       JD1, JD2: Connectors to plug in DSP PCB
 *       JR1     : Connector  to plug in MROM PCB
 *       JV1, JV2: Connectors to plug in VIDEO PCB
 *
 *       J1 : 9 pin power input socket   Pin  Use
 *                                      -----------
 *                                       1    +5V
 *                                       2    +5V
 *                                       3    +5V
 *                                       4    NC
 *                                       5    Ground
 *                                       6    Ground
 *                                       7    Ground
 *                                       8    NC
 *                                       9    +12V
 *
 *       J2 : 9 pin link connector   Pin  Use
 *                                   --------------
 *                                    1   Ring In+
 *                                    2   Ring In-
 *                                    3   Ring Out2+
 *                                    4   Ground
 *                                    5   /RXD+ (NC)
 *                                    6   /RXD- (NC)
 *                                    7   /TXD  (NC)
 *                                    8   GND   (NC)
 *                                    9   +5V   (NC)
 *
 *       J3 : 9 pin socket   Pin  Use
 *                           -------------------
 *                            1   Service Credit
 *                            2   Test
 *                            3   NC
 *                            4   +12V
 *                            5   Counter
 *                            6   NC
 *                            7   Ground
 *                            8   Coin Sw
 *                            9   NC
 *
 *       J4 : 8 pin connector  Pin  Use
 *                             -------------------------------------
 *                              1   Start
 *                              2   Left Select
 *                              3   Right Select
 *                              4   Safety Free Controls (Ski's etc)
 *                              5   Safety Lock Controls (Ski's etc)
 *                              6   NC
 *                              7   Ground
 *                              8   Ground
 *
 *       J5 : 15 pin socket   Pin  Use
 *                            ---------------
 *                             1   Relay1
 *                             2   Relay1
 *                             3   Mute
 *                             4   Mid Light
 *                             5   Left Light
 *                             6   Right Light
 *                             7   NC
 *                             8   NC
 *                             9   NC
 *                             10  NC
 *                             11  NC
 *                             12  NC
 *                             13  +12V
 *                             14  +5V
 *                             15  NC
 *
 *       J6 : 12 pin audio output connector  Pin  Use
 *                                           ----------
 *                                            1   SPKL+
 *                                            2   SPKL-
 *                                            3   SPKR+
 *                                            4   SPKR-
 *                                            5   NC
 *                                            6   NC
 *                                            7   NC
 *                                            8   NC
 *                                            9   NC
 *                                            10  NC
 *                                            11  NC
 *                                            12  NC
 *
 *       J7 : 12 pin analog controls socket   Pin  Use
 *                                            ---------------------------
 *                                             1   +5V
 *                                             2   5K Ohm Potentiometer 1 (Side Swing on Alpine Racer, Alpine Surfer etc)
 *                                             3   5K Ohm Potentiometer 2 (Tilt on Alpine Racer, Alpine Surfer etc)
 *                                             4   GND                    (Other games may use more/different pots or no pots)
 *                                             5   NC
 *                                             6   NC
 *                                             7   NC
 *                                             8   NC
 *                                             9   NC
 *                                             10  NC
 *                                             11  NC
 *                                             12  NC
 *
 *       J8 : 10 pin connector (not used?)
 *
 *       J9 : 6 pin video output socket   Pin  Use
 *                                        ---------
 *                                         1   Red
 *                                         2   Green
 *                                         3   Blue
 *                                         4   Composite Sync (VSync 15kHz interlaced)
 *                                         5   Ground
 *                                         6   NC
 *
 *       J10: 16 pin flat cable connector (only populated on Mother(C) PCB, use not known)
 *
 *
 * AMP PCB
 * -------
 * SYSTEM SUPER22 AMP(4) PCB 8647960100 (8647970100) (sticker 'AMP(2) PCB 8647961100')
 * |-----------------------------------|
 * |J1  J2   LA4705  J3   *1    *2   J5|
 * |                                   |
 * |-----------------------------------|
 * Notes:
 *       LA4705: 2-channel 15W Power Amp
 *       J1    : 3 pin power input socket   Pin  Use
 *                                          -----------
 *                                           1   +12V
 *                                           2   Ground
 *                                           3   NC
 *
 *       J2    : 4 pin dual speaker output socket   Pin  Use
 *                                                  ---------
 *                                                   1   SP1+
 *                                                   2   SP1-
 *                                                   3   SP2+
 *                                                   4   SP2-
 *
 *       J3    : 8 pin sound data input connector from Mother PCB J6   Pin  Use
 *                                                                     ----------
 *                                                                      1   SPKL+
 *                                                                      2   SPKL-
 *                                                                      3   SPKR+
 *                                                                      4   SPKR-
 *                                                                      5   NC
 *                                                                      6   NC
 *                                                                      7   NC
 *                                                                      8   NC
 *
 *       *1    : Unpopulated position for a 2nd LA4705 Power Amp
 *       *2    : J4 - Unpopulated position for another 4 pin dual speaker output socket
 *       J5    : 2 pin connector used for sound mute  Pin  Use
 *                                                    -----------
 *                                                     1   Mute
 *                                                     2   Ground
 */

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "deprecat.h"
#include "includes/namcos22.h"
#include "cpu/tms32025/tms32025.h"
#include "cpu/m37710/m37710.h"
#include "sound/c352.h"

#define SS22_MASTER_CLOCK (49152000)	/* info from Guru */

enum namcos22_gametype namcos22_gametype; /* TBR: used for game-specific hacks */

static int mbSuperSystem22; /* used to dispatch Sys22/SuperSys22 differences */
static int mbEnableDspIrqs;
static UINT32 *namcos22_shareram;
static UINT32 *namcos22_system_controller;
static UINT32 *namcos22_nvmem;
static size_t namcos22_nvmem_size;
static UINT16 mMasterBIOZ;
static UINT32 *mpPointRAM;
static UINT32 old_coin_state, credits1, credits2;
static UINT16 *mpSlaveExternalRAM;

static UINT32 mPointAddr;
static UINT32 mPointData;
static UINT16 *mpMasterExternalRAM;

static UINT16 mSerialDataSlaveToMasterNext;
static UINT16 mSerialDataSlaveToMasterCurrent;

#define MAX_RENDER_CMD_SEQ 0x1c
static int mRenderBufSize;
static UINT16 mRenderBufData[MAX_RENDER_CMD_SEQ];

static UINT32 mSys22PortBits;

static int mFrameCount;

static UINT8 stick_input, prev_stick_state;

/**
 * helper function used to read a byte from a chunk of 32 bit memory
 */
static UINT8
nthbyte( const UINT32 *pSource, int offs )
{
	pSource += offs/4;
	return (pSource[0]<<((offs&3)*8))>>24;
} /* nthbyte */

/*********************************************************************************************/

/* mask,default,type,sensitivity,delta,min,max */
#define DRIVING_ANALOG_PORTS \
	PORT_START("GAS") \
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) \
	PORT_START("BRAKE") \
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) \
	PORT_START("STEER") \
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10)

/* TODO: REMOVE (THIS IS HANDLED BY "SUBCPU") */
static void
ReadAnalogDrivingPorts( running_machine *machine, UINT16 *gas, UINT16 *brake, UINT16 *steer )
{
	*gas   = input_port_read(machine, "GAS");
	*brake = input_port_read(machine, "BRAKE");
	*steer = input_port_read(machine, "STEER");
}

/* TODO: REMOVE (THIS IS HANDLED BY "SUBCPU") */
static UINT16
AnalogAsDigital( running_machine *machine )
{
	UINT16 stick = input_port_read_safe(machine, "INPUTS", 0);
	UINT16 gas   = input_port_read_safe(machine, "GAS", 0);
	UINT16 steer = input_port_read_safe(machine, "STEER", 0);
	UINT16 result = 0xffff;

	switch( namcos22_gametype )
	{
	case NAMCOS22_RAVE_RACER:
	case NAMCOS22_RIDGE_RACER:
	case NAMCOS22_RIDGE_RACER2:
		if( gas == 0xff )
		{
			result ^= 0x0100; /* CHOOSE */
		}
		if( steer == 0x00 )
		{
			result ^= 0x0040; /* PREV */
		}
		else if( steer == 0xff )
		{
			result ^= 0x0080; /* NEXT */
		}
		return result;

	case NAMCOS22_VICTORY_LAP:
	case NAMCOS22_ACE_DRIVER:
		if( gas > 0xf0 )
		{
			result ^= 0x0001; /* CHOOSE */
		}
		stick &= 3;
		if( stick == 1 )
		{ /* Stick Shift Up */
			result ^= 0x0040; /* PREV */
		}
		if( stick == 2 )
		{ /* Stick Shift Down */
			result ^= 0x0080; /* NEXT */
		}
		return result;

	default:
		break;
	}
	return result;
} /* AnalogAsDigital */

/* TODO: REMOVE (THIS IS HANDLED BY "SUBCPU") */
static void
HandleCoinage(running_machine *machine, int slots)
{
	UINT16 *share16 = (UINT16 *)namcos22_shareram;
	UINT32 coin_state;

	coin_state = input_port_read(machine, "INPUTS") & 0x1200;

	if (!(coin_state & 0x1000) && (old_coin_state & 0x1000))
	{
		credits1++;
	}

	if (!(coin_state & 0x0200) && (old_coin_state & 0x0200))
	{
		credits2++;
	}

	old_coin_state = coin_state;

	share16[BYTE_XOR_LE(0x38/2)] = credits1;

	if (slots == 2)
	{
		share16[BYTE_XOR_LE(0x3e/2)] = credits2;
	}
} /* HandleCoinage */

/* TODO: REMOVE (THIS IS HANDLED BY "SUBCPU") */
static void
HandleDrivingIO( running_machine *machine )
{
	if( nthbyte(namcos22_system_controller, 0x18) != 0 )
	{
		UINT16 flags = input_port_read(machine, "INPUTS");
		UINT16 gas, brake, steer;
		ReadAnalogDrivingPorts( machine, &gas, &brake, &steer );

		HandleCoinage(machine, 2);

		switch (namcos22_gametype)
		{
			case NAMCOS22_RIDGE_RACER:
			case NAMCOS22_RIDGE_RACER2:
				gas <<= 3;
				gas += 884;
				brake <<= 3;
				brake += 809;
				steer <<= 4;
				steer += 0x160;
				break;

			case NAMCOS22_RAVE_RACER:
				gas <<= 3;
				gas += 992;
				brake <<= 3;
				brake += 3008;
				steer <<= 4;
				steer += 32;
				break;

			case NAMCOS22_ACE_DRIVER:
			case NAMCOS22_VICTORY_LAP:
				gas <<= 3;
				gas += 992;
				brake <<= 3;
				brake += 3008;
				steer <<= 4;
				steer += 2048;
				break;

			default:
				gas <<= 3;
				brake <<= 3;
				steer <<= 4;
				break;
		}

		namcos22_shareram[0x000/4] = 0x10<<16; /* SUB CPU ready */
		namcos22_shareram[0x030/4] = (flags<<16)|steer;
		namcos22_shareram[0x034/4] = (gas<<16)|brake;
	}
} /* HandleDrivingIO */

/* TODO: REMOVE (THIS IS HANDLED BY "SUBCPU") */
static void
HandleCyberCommandoIO( running_machine *machine )
{
	if( nthbyte(namcos22_system_controller, 0x18) != 0 )
	{
		UINT16 flags = input_port_read(machine, "INPUTS");

		UINT16 volume0 = input_port_read(machine, "STICKY1") * 0x10;
		UINT16 volume1 = input_port_read(machine, "STICKY2") * 0x10;
		UINT16 volume2 = input_port_read(machine, "STICKX1") * 0x10;
		UINT16 volume3 = input_port_read(machine, "STICKX2") * 0x10;

		namcos22_shareram[0x030/4] = (flags<<16) | volume0;
		namcos22_shareram[0x034/4] = (volume1<<16) | volume2;
		namcos22_shareram[0x038/4] = volume3<<16;

		HandleCoinage(machine, 1);
	}
} /* HandleCyberCommandoIO */

/*********************************************************************************************/

static void
InitDSP( running_machine *machine, int bSuperSystem22 )
{
	mbSuperSystem22 = bSuperSystem22;
	cputag_set_input_line(machine, "master",INPUT_LINE_RESET,ASSERT_LINE); /* master DSP */
	cputag_set_input_line(machine, "slave",INPUT_LINE_RESET,ASSERT_LINE); /* slave DSP */
	cputag_set_input_line(machine, "mcu",INPUT_LINE_RESET,ASSERT_LINE); /* MCU */
} /* InitDSP */

static READ16_HANDLER( pdp_status_r )
{
	return mMasterBIOZ;
} /* pdp_status_r */

static void
WriteToPointRAM( offs_t offs, UINT32 data )
{
	offs &= 0xffffff; /* 24 bit addressing */
	if( mbSuperSystem22 )
	{
		if( offs>=0xf80000 && offs<=0xf9ffff )
		{
			mpPointRAM[offs-0xf80000] = data;
		}
	}
	else
	{
		if( offs>=0xf00000 && offs<=0xf1ffff )
		{
			mpPointRAM[offs-0xf00000] = data;
		}
	}
} /* WriteToPointRAM */

static UINT32
ReadFromPointRAM( offs_t offs )
{
	offs &= 0xffffff; /* 24 bit addressing */
	if( mbSuperSystem22 )
	{
		if( offs>=0xf80000 && offs<=0xf9ffff )
		{
			return mpPointRAM[offs-0xf80000];
		}
	}
	else
	{
		if( offs>=0xf00000 && offs<=0xf1ffff )
		{
			return mpPointRAM[offs-0xf00000];
		}
	}
	return namcos22_point_rom_r(offs);
} /* ReadFromPointRAM */

static UINT32
ReadFromCommRAM( offs_t offs )
{
	return namcos22_polygonram[offs&0x7fff];
} /* ReadFromCommRAM */

static void
WriteToCommRAM( offs_t offs, UINT32 data )
{
	namcos22_polygonram[offs&0x7fff] = data;
} /* WriteToCommRAM */

static READ16_HANDLER( pdp_begin_r )
{
	/* this feature appears to be only used on Super System22 hardware */
	if( mbSuperSystem22 )
	{
		UINT16 offs = namcos22_polygonram[0x20000/4-1];
		mMasterBIOZ = 1;
		for(;;)
		{
			UINT16 start = offs;
			UINT16 cmd = ReadFromCommRAM(offs++);
			UINT32 srcAddr;
			UINT32 dstAddr;
			UINT32 numWords;
			UINT32 data;
			switch( cmd )
			{
			case 0xfff0:
				/* NOP? used in 'PDP LOOP TEST' */
				break;

			case 0xfff5: /* write to point ram */
				dstAddr = ReadFromCommRAM(offs++); /* 32 bit PointRAM address */
				data    = ReadFromCommRAM(offs++);    /* 24 bit data */
				WriteToPointRAM( dstAddr, data );
				break;

			case 0xfff6: /* read word from point ram */
				srcAddr = ReadFromCommRAM(offs++); /* 32 bit PointRAM address */
				dstAddr = ReadFromCommRAM(offs++); /* CommRAM address; receives 24 bit PointRAM data */
				data    = ReadFromPointRAM( srcAddr );
				WriteToCommRAM( dstAddr, data );
				break;

			case 0xfff7: /* block move (CommRAM to CommRAM) */
				srcAddr  = ReadFromCommRAM(offs++);
				dstAddr  = ReadFromCommRAM(offs++);
				numWords = ReadFromCommRAM(offs++);
				while( numWords-- )
				{
					data = ReadFromCommRAM(srcAddr++);
					WriteToCommRAM( dstAddr++, data );
				}
				break;

			case 0xfffa: /* read block from point ram */
				srcAddr  = ReadFromCommRAM(offs++); /* 32 bit PointRAM address */
				dstAddr  = ReadFromCommRAM(offs++); /* CommRAM address; receives data */
				numWords = ReadFromCommRAM(offs++); /* block size */
				while( numWords-- )
				{
					data = ReadFromPointRAM( srcAddr++ );
					WriteToCommRAM( dstAddr++, data );
				}
				break;

			case 0xfffb: /* write block to point ram */
				dstAddr  = ReadFromCommRAM(offs++);  /* 32 bit PointRAM address */
				numWords = ReadFromCommRAM(offs++); /* block size */
				while( numWords-- )
				{
					data = ReadFromCommRAM( offs++ ); /* 24 bit source data */
					WriteToPointRAM( dstAddr++, data );
				}
				break;

			case 0xfffc: /* point ram to point ram */
				srcAddr  = ReadFromCommRAM(offs++);
				dstAddr  = ReadFromCommRAM(offs++);
				numWords = ReadFromCommRAM(offs++);
				while( numWords-- )
				{
					data = ReadFromPointRAM( srcAddr++ );
					WriteToPointRAM( dstAddr++, data );
				}
				break;

			case 0xfffd: /* unknown */
				numWords = ReadFromCommRAM(offs++);
				while( numWords-- )
				{
					data = ReadFromCommRAM(offs++);
					//namcos22_WriteDataToRenderDevice( data );
				}
				break;

			case 0xfffe: /* unknown */
				data = ReadFromCommRAM(offs++); /* ??? */
				break;

			case 0xffff: /* "goto" command */
				offs = ReadFromCommRAM(offs);
				if( offs == start )
				{ /* most commands end with a "goto self" */
					return 0;
				}
				break;

			default:
				logerror( "unknown PDP cmd = 0x%04x!\n", cmd );
				return 0;
			}
		} /* for(;;) */
	} /* mbSuperSystem22 */
	return 0;
} /* pdp_begin_r */

static READ16_HANDLER( slave_external_ram_r )
{
	return mpSlaveExternalRAM[offset];
}

static WRITE16_HANDLER( slave_external_ram_w )
{
	COMBINE_DATA( &mpSlaveExternalRAM[offset] );
}

static void HaltSlaveDSP( running_machine *machine )
{
	cputag_set_input_line(machine, "slave", INPUT_LINE_RESET, ASSERT_LINE);
}

static void EnableSlaveDSP( void )
{
//  cputag_set_input_line(machine, "slave", INPUT_LINE_RESET, CLEAR_LINE);
}

static READ16_HANDLER( dsp_HOLD_signal_r )
{ /* STUB */
	return 0;
}

static WRITE16_HANDLER( dsp_HOLD_ACK_w )
{ /* STUB */
}

static WRITE16_HANDLER( dsp_XF_output_w )
{ /* STUB */
}

/************************************************************/

static WRITE16_HANDLER( point_ram_idx_w )
{
	mPointAddr<<=16;
	mPointAddr |= data;
}

static WRITE16_HANDLER( point_ram_loword_iw )
{
	mPointData |= data;
	WriteToPointRAM( mPointAddr++, mPointData );
}

static WRITE16_HANDLER( point_ram_hiword_w )
{
	mPointData = (data<<16);
}

static READ16_HANDLER( point_ram_loword_r )
{
	return ReadFromPointRAM(mPointAddr)&0xffff;
}

static READ16_HANDLER( point_ram_hiword_ir )
{
	return ReadFromPointRAM(mPointAddr++)>>16;
}

static WRITE16_HANDLER( dsp_unk2_w )
{
	/**
     * Used by Ridge Racer (Japan) to specify baseaddr
    * for post-processed display-list output.
    *
    * Prop Cycle doesn't use this; instead it writes this
    * addr to the uppermost word of CommRAM.
    */
}

static enum
{
	eDSP_UPLOAD_READY,
	eDSP_UPLOAD_DEST,
	eDSP_UPLOAD_DATA
} mDspUploadState;

static READ16_HANDLER( dsp_unk_port3_r )
{
	mMasterBIOZ = 0;
	mDspUploadState = eDSP_UPLOAD_READY;
	return 0;
}

static WRITE16_HANDLER( upload_code_to_slave_dsp_w )
{
	static int mUploadDestIdx;

	switch( mDspUploadState )
	{
	case eDSP_UPLOAD_READY:
		logerror( "UPLOAD_READY; cmd = 0x%x\n", data );
		if( data==0 )
		{
			HaltSlaveDSP(space->machine);
		}
		else if( data==1 )
		{
			mDspUploadState = eDSP_UPLOAD_DEST;
		}
		else if( data==2 )
		{
			/* custom IC poke */
		}
		else if( data==3 )
		{
			EnableSlaveDSP();
		}
		else if( data==4 )
		{
		}
		else if( data == 0x10 )
		{ /* serial i/o related? */
			EnableSlaveDSP();
		}
		else
		{
			logerror( "%08x: master port#7: 0x%04x\n",
				cpu_get_previouspc(space->cpu), data );
		}
		break;

	case eDSP_UPLOAD_DEST:
		mUploadDestIdx = data-0x8000;
		mDspUploadState = eDSP_UPLOAD_DATA;
		break;

	case eDSP_UPLOAD_DATA:
		mpSlaveExternalRAM[mUploadDestIdx++] = data;
		break;

	default:
		break;
	}
}

static READ16_HANDLER( dsp_unk8_r )
{
	/* bit 0x0001 is busy signal */
	return 0;
}

static READ16_HANDLER( custom_ic_status_r )
{
	/* bit 0x0001 signals completion */
	return 0x0063;
}

static READ16_HANDLER( dsp_upload_status_r )
{
	/**
     * bit 0x0001 is polled to confirm that code/data has been
     * successfully uploaded to the slave dsp via port 0x7.
     */
	return 0x0000;
}

static READ16_HANDLER( master_external_ram_r )
{
	return mpMasterExternalRAM[offset];
}

static WRITE16_HANDLER( master_external_ram_w )
{
	COMBINE_DATA( &mpMasterExternalRAM[offset] );
}

#ifdef USE_NAMCOS22_SPEED_HACK
#define SERIAL_IO_PERIOD 256
#else
#define SERIAL_IO_PERIOD 1
#endif

static WRITE16_HANDLER( slave_serial_io_w )
{
	mSerialDataSlaveToMasterNext = data;
	logerror( "slave_serial_io_w(%04x)\n", data );
}

static READ16_HANDLER( master_serial_io_r )
{
	logerror( "master_serial_io_r() == %04x\n",
		mSerialDataSlaveToMasterCurrent );
	return mSerialDataSlaveToMasterCurrent;
}

static INTERRUPT_GEN( dsp_serial_pulse1 )
{
	if( mbEnableDspIrqs )
	{
		mSerialDataSlaveToMasterCurrent = mSerialDataSlaveToMasterNext;

		if( cpu_getiloops(device) == 0 )
		{
			cputag_set_input_line(device->machine, "master", TMS32025_INT0, HOLD_LINE);
		}
		cputag_set_input_line(device->machine, "master", TMS32025_RINT, HOLD_LINE);
		cputag_set_input_line(device->machine, "master", TMS32025_XINT, HOLD_LINE);
		cputag_set_input_line(device->machine, "slave", TMS32025_RINT, HOLD_LINE);
		cputag_set_input_line(device->machine, "slave", TMS32025_XINT, HOLD_LINE);
	}
}

static WRITE16_HANDLER( dsp_unk_porta_w )
{
}

static WRITE16_HANDLER( dsp_led_w )
{
	/* I believe this port controls diagnostic LEDs on the DSP PCB. */
}

/**
 * master dsp usage pattern:
 *
 * 4059: out  $10,PA$8
 * 405A: in   $09,PA$8; lac  $09,0h; andk 0001h,0h; bnz  $405A *
 * 4060: out  $11,PA$8
 *
 * 4061: out  $10,PA$F
 * 4062: nop; rpt  *+; out  *+,PA$C // send data to 'render device'
 * 4065: out  $11,PA$F
 *
 * 4066: out  $10,PA$9
 * 4067: nop
 * 4068: out  $11,PA$9
 **************************************************************
 * 0x03a2 // 0x0fff zcode lo
 * 0x0001 // 0x000f zcode hi
 * 0xbd00 // color
 * 0x13a2 // flags
 *
 * 0x0100 0x009c // u,v
 * 0x0072 0xf307 // sx,sy
 * 0x602b 0x9f28 // i,zpos
 *
 * 0x00bf 0x0060 // u,v
 * 0x0040 0xf3ec // sx,sy
 * 0x602b 0xad48 // i,zpos
 *
 * 0x00fb 0x00ca // u,v
 * 0x0075 0xf205 // sx,sy
 * 0x602b 0x93e8 // i,zpos
 *
 * 0x00fb 0x00ca // u,v
 * 0x0075 0xf205 // sx,sy
 * 0x602b 0x93e8 // i,zpos
 */
static WRITE16_HANDLER( dsp_unk8_w )
{
	mRenderBufSize = 0;
}

static WRITE16_HANDLER( master_render_device_w )
{
	if( mRenderBufSize<MAX_RENDER_CMD_SEQ )
	{
		mRenderBufData[mRenderBufSize++] = data;
		if( mRenderBufSize == MAX_RENDER_CMD_SEQ )
		{
			namcos22_draw_direct_poly( space->machine, mRenderBufData );
		}
	}
}

static ADDRESS_MAP_START( master_dsp_program, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x0000, 0x0fff) AM_ROM /* internal ROM (4k words) */
	AM_RANGE(0x4000, 0x7fff) AM_ROM AM_BASE(&mpMasterExternalRAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( master_dsp_data, ADDRESS_SPACE_DATA, 16 )
	AM_RANGE(0x1000, 0x3fff) AM_RAM
	AM_RANGE(0x4000, 0x7fff) AM_READ(master_external_ram_r) AM_WRITE(master_external_ram_w)
	AM_RANGE(0x8000, 0xffff) AM_READ(namcos22_dspram16_r) AM_WRITE(namcos22_dspram16_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( master_dsp_io, ADDRESS_SPACE_IO, 16 )
	AM_RANGE(0x0,0x0) AM_WRITE(point_ram_loword_iw) AM_READ(point_ram_loword_r)
	AM_RANGE(0x1,0x1) AM_WRITE(point_ram_hiword_w) AM_READ(point_ram_hiword_ir)
	AM_RANGE(0x2,0x2) AM_WRITE(dsp_unk2_w) AM_READ(pdp_begin_r)
	AM_RANGE(0x3,0x3) AM_WRITE(point_ram_idx_w) AM_READ(dsp_unk_port3_r)
	AM_RANGE(0x4,0x4) AM_WRITENOP /* unknown */
	AM_RANGE(0x7,0x7) AM_WRITE(upload_code_to_slave_dsp_w)
	AM_RANGE(0x8,0x8) AM_WRITE(dsp_unk8_w) AM_READ(dsp_unk8_r)         /* trigger irq? */
	AM_RANGE(0x9,0x9) AM_WRITENOP AM_READ(custom_ic_status_r) /* trigger irq? */
	AM_RANGE(0xa,0xa) AM_WRITE(dsp_unk_porta_w)
	AM_RANGE(0xb,0xb) AM_WRITENOP /* RINT-related? */
	AM_RANGE(0xc,0xc) AM_WRITE(master_render_device_w)
	AM_RANGE(0xd,0xd) AM_WRITE(namcos22_dspram16_bank_w)
	AM_RANGE(0xe,0xe) AM_WRITE(dsp_led_w)
	AM_RANGE(0xf,0xf) AM_WRITENOP AM_READ(dsp_upload_status_r)
	AM_RANGE(TMS32025_HOLD,  TMS32025_HOLD)  AM_READ(dsp_HOLD_signal_r)
	AM_RANGE(TMS32025_HOLDA, TMS32025_HOLDA) AM_WRITE(dsp_HOLD_ACK_w)
	AM_RANGE(TMS32025_XF,    TMS32025_XF)    AM_WRITE(dsp_XF_output_w)
	AM_RANGE(TMS32025_BIO,   TMS32025_BIO)   AM_READ(pdp_status_r)
	AM_RANGE(TMS32025_DR,    TMS32025_DR)    AM_READ(master_serial_io_r)
ADDRESS_MAP_END

static READ16_HANDLER( dsp_BIOZ_r )
{ /* STUB */
	return 1;
}

static READ16_HANDLER( dsp_slave_port3_r )
{
	return 0x0010; /* ? */
}

static READ16_HANDLER( dsp_slave_port4_r )
{
	return 0;
//  return ReadDataFromSlaveBuf();
}

static READ16_HANDLER( dsp_slave_port5_r )
{
/*  int numWords = SlaveBufSize();
    int mode = 2;
    return (numWords<<4) | mode;
*/
	return 0;
}

static READ16_HANDLER( dsp_slave_port6_r )
{
	/**
     * bit 0x9 indicates whether device at port2 is ready to receive data
     * bit 0xd indicates whether data is available from port4
     */
	return 0;
}

static WRITE16_HANDLER( dsp_slave_portc_w )
{
	/* Unknown; used before transmitting a command sequence. */
}

static READ16_HANDLER( dsp_slave_port8_r )
{
	/* This reports  status of the device mapped at port 0xb.
     *
     * The slave dsp waits for bit 0x0001 to be zero before writing
     * a new command sequence.
     */
	return 0; /* status */
}

static READ16_HANDLER( dsp_slave_portb_r )
{
	/* The slave DSP reads before transmitting a command sequence. */
	return 0;
}

static WRITE16_HANDLER( dsp_slave_portb_w )
{
	/* The slave dsp uses this to transmit a command sequence to an external device. */
}

static ADDRESS_MAP_START( slave_dsp_program, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x0000, 0x0fff) AM_ROM /* internal ROM */
	AM_RANGE(0x8000, 0x9fff) AM_ROM AM_BASE(&mpSlaveExternalRAM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( slave_dsp_data, ADDRESS_SPACE_DATA, 16 )
	AM_RANGE(0x8000, 0x9fff) AM_READ(slave_external_ram_r) AM_WRITE(slave_external_ram_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( slave_dsp_io, ADDRESS_SPACE_IO, 16 )
	/* unknown signal */
	AM_RANGE(0x3,0x3) AM_READ(dsp_slave_port3_r)

	AM_RANGE(0x4,0x4) AM_READ(dsp_slave_port4_r)
	AM_RANGE(0x5,0x5) AM_READ(dsp_slave_port5_r)
	AM_RANGE(0x6,0x6) AM_WRITENOP AM_READ(dsp_slave_port6_r)

	/* render device state */
	AM_RANGE(0x8,0x8) AM_WRITENOP AM_READ(dsp_slave_port8_r)

	/* render device */
	AM_RANGE(0xb,0xb) AM_WRITE(dsp_slave_portb_w) AM_READ(dsp_slave_portb_r)

	AM_RANGE(0xc,0xc) AM_WRITE(dsp_slave_portc_w)

	AM_RANGE(TMS32025_HOLD,  TMS32025_HOLD)  AM_READ(dsp_HOLD_signal_r)
	AM_RANGE(TMS32025_HOLDA, TMS32025_HOLDA) AM_WRITE(dsp_HOLD_ACK_w)
	AM_RANGE(TMS32025_XF,    TMS32025_XF)    AM_WRITE(dsp_XF_output_w)
	AM_RANGE(TMS32025_BIO,   TMS32025_BIO)   AM_READ(dsp_BIOZ_r)
	AM_RANGE(TMS32025_DX,    TMS32025_DX)    AM_WRITE(slave_serial_io_w)
ADDRESS_MAP_END

static NVRAM_HANDLER( namcos22 )
{
	int i;
	UINT8 data[4];
	if( read_or_write )
	{
		for( i=0; i<namcos22_nvmem_size/4; i++ )
		{
			UINT32 dword = namcos22_nvmem[i];
			data[0] = dword>>24;
			data[1] = (dword&0x00ff0000)>>16;
			data[2] = (dword&0x0000ff00)>>8;
			data[3] = dword&0xff;
			mame_fwrite( file, data, 4 );
		}
	}
	else
	{
		if( file )
		{
			for( i=0; i<namcos22_nvmem_size/4; i++ )
			{
				mame_fread( file, data, 4 );
				namcos22_nvmem[i] = (data[0]<<24)|(data[1]<<16)|(data[2]<<8)|data[3];
			}
		}
		else
		{
			memset( namcos22_nvmem, 0x00, namcos22_nvmem_size );
			if (memory_region_length(machine, "nvram") == namcos22_nvmem_size)
			{
				UINT8* nvram = memory_region(machine,"nvram");

				for( i=0; i<namcos22_nvmem_size/4; i++ )
				{
					namcos22_nvmem[i] = (nvram[0+i*4]<<24)|(nvram[1+i*4]<<16)|(nvram[2+i*4]<<8)|nvram[3+i*4];
				}
			}
		}
	}
}

/* Super System22 supports a sprite layer.
 * Sprites are rendered as part of the polygon draw list, based on a per-sprite Z attribute.
 * Each sprite has explicit placement/color/zoom controls.
 */
static const gfx_layout sprite_layout =
{
	32,32,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{
		0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8,
		8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8,
		16*8,17*8,18*8,19*8,20*8,21*8,22*8,23*8,
		24*8,25*8,26*8,27*8,28*8,29*8,30*8,31*8 },
	{
		0*32*8,1*32*8,2*32*8,3*32*8,4*32*8,5*32*8,6*32*8,7*32*8,
		8*32*8,9*32*8,10*32*8,11*32*8,12*32*8,13*32*8,14*32*8,15*32*8,
		16*32*8,17*32*8,18*32*8,19*32*8,20*32*8,21*32*8,22*32*8,23*32*8,
		24*32*8,25*32*8,26*32*8,27*32*8,28*32*8,29*32*8,30*32*8,31*32*8 },
	32*32*8
};

static const gfx_layout texture_tile_layout =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{
		0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8,8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8
   },
	{
		0*16*8,1*16*8,2*16*8,3*16*8,4*16*8,5*16*8,6*16*8,7*16*8,
		8*16*8,9*16*8,10*16*8,11*16*8,12*16*8,13*16*8,14*16*8,15*16*8
   },
	16*16*8
};

#define XOR(a) WORD2_XOR_BE(a)

/* text layer uses a set of 16x16x8bpp tiles defined in RAM */
static const gfx_layout namcos22_cg_layout =
{
	16,16,
	0x400, /* 0x3c0 */
	4,
	{ 0,1,2,3 },
	{ XOR(0)*4,  XOR(1)*4,  XOR(2)*4,  XOR(3)*4,  XOR(4)*4,  XOR(5)*4,  XOR(6)*4,  XOR(7)*4,
	  XOR(8)*4,  XOR(9)*4, XOR(10)*4, XOR(11)*4, XOR(12)*4, XOR(13)*4, XOR(14)*4, XOR(15)*4 },
	{ 64*0,64*1,64*2,64*3,64*4,64*5,64*6,64*7,64*8,64*9,64*10,64*11,64*12,64*13,64*14,64*15 },
	64*16
}; /* cg_layout */

static GFXDECODE_START( namcos22 )
	GFXDECODE_ENTRY( NULL,                   0, namcos22_cg_layout,   0, 0x800 )
	GFXDECODE_ENTRY( "textile", 0, texture_tile_layout,  0, 0x80 )
GFXDECODE_END

static GFXDECODE_START( super )
	GFXDECODE_ENTRY( NULL,                   0, namcos22_cg_layout,   0, 0x800 )
	GFXDECODE_ENTRY( "textile", 0, texture_tile_layout,  0, 0x80 )
	GFXDECODE_ENTRY( "sprite",       0, sprite_layout,        0, 0x80 )
GFXDECODE_END

/* prelim! */
static READ32_HANDLER( namcos22_C139_SCI_r )
{
	switch( offset )
	{
	case 0x0/4: return 0x0004<<16;
	default: return 0;
	}
}

#if 0
static WRITE32_HANDLER( namcos22_C139_SCI_w )
{
	COMBINE_DATA( &namcos22_C139_SCI[offset] );
	/*
    20020000  2 R/W RX Status
                0x01 : Frame Error
                0x02 : Frame Received
                0x04 : ?

    20020002  2 R/W Status/Control Flags
                0x01 :
                0x02 : RX flag? (cleared every vsync)
                0x04 : RX flag? (cleared every vsync)
                0x08 :

    20020004  2 W   FIFO Control Register
                0x01 : sync bit enable?
                0x02 : TX FIFO sync bit (bit-8)

    20020006  2 W   TX Control Register
                0x01 : TX start/stop
                0x02 : ?
                0x10 : ?

    20020008  2 W   -
    2002000a  2 W   TX Frame Size
    2002000c  2 R/W RX FIFO Pointer (0x0000 - 0x0fff)
    2002000e  2 W   TX FIFO Pointer (0x0000 - 0x1fff)
    */
}
#endif

static READ32_HANDLER( namcos22_system_controller_r )
{
	return namcos22_system_controller[offset];
}

static int
GetDspControlRegister( void )
{
	int mode;
	if( mbSuperSystem22 )
	{
		mode = nthbyte(namcos22_system_controller,0x1c);
	}
	else
	{
		mode = nthbyte(namcos22_system_controller,0x1a);
	}
	return mode;
} /* GetDspControlRegister */

/*
0x00: vblank irq level
0x01: hblank irq level
0x02: sci irq level
0x03: unk irq level (unused?)

0x04: vblank irq ack
0x05: hblank irq ack
0x06: sci irq ack
0x07: unk irq ack

0x08: unknown

//memory map?
0x09: 0x62 or 0x61
0x0a: 0x62
0x0b: 0x57
0x0c: 0x40
0x0d: 0x12
0x0e: 0x52 or 0x50
0x0f: 0x72 or 0x71
0x10: 0xe0
0x11: 0x2c
0x12: 0x50
0x13: 0xff

0x14: watchdog
0x16: subcpu enable
0x17: 0x0f
0x1b: 0x01
0x1c: dsp control
*/
static TIMER_CALLBACK( start_subcpu )
{
	cputag_set_input_line(machine, "mcu", INPUT_LINE_RESET, CLEAR_LINE);
}

static WRITE32_HANDLER( namcos22_system_controller_w )
{
	int oldReg = GetDspControlRegister();
	int newReg;

	if( 0 )
	{ /* trace use of these registers */
		UINT32 mask = mem_mask;
		UINT32 dat = data;
		int i;
		for( i=0; i<4; i++ )
		{
			if( (mask&0xff000000)!=0 )
			{
				int offs = offset*4+i;
				if(offs<4 || offs>=8)
				{
					mame_printf_debug( "%08x: sys[0x%02x] := 0x%02x\n",
						cpu_get_previouspc(space->cpu),
						offs,
						dat>>24 );
				}
			}
			dat<<=8;
			mask<<=8;
		}
	}

	if( mbSuperSystem22 )
	{
		if (offset == 0x14/4 && mem_mask == 0x0000ff00)
		{ /* SUBCPU enable for Super System 22 */
			if (data)
			{
				cputag_set_input_line(space->machine, "mcu", INPUT_LINE_RESET, CLEAR_LINE);
			}
			else
			{
				cputag_set_input_line(space->machine, "mcu",INPUT_LINE_RESET,ASSERT_LINE); /* M37710 MCU */
			}
		}
	}
	else
	{
		if (offset == 0x18/4 && mem_mask == 0x0000ff00)
		{ /* SUBCPU enable on System 22 (guessed, but too early crashes Rave Racer so it's a good test) */
			if (data == 0xff00)
			{
				timer_set(space->machine, ATTOTIME_IN_MSEC(50), NULL, 0, start_subcpu);
			}
		}
	}

	COMBINE_DATA( &namcos22_system_controller[offset] );

	newReg = GetDspControlRegister();
	if( newReg != oldReg )
	{
		if( newReg == 0 )
		{ /* disable DSPs */
			cputag_set_input_line(space->machine, "master", INPUT_LINE_RESET, ASSERT_LINE); /* master DSP */
			cputag_set_input_line(space->machine, "slave", INPUT_LINE_RESET, ASSERT_LINE); /* slave DSP */
			mbEnableDspIrqs = 0;
		}
		else if( newReg == 1 )
		{ /*enable dsp and rendering subsystem */
			cputag_set_input_line(space->machine, "master", INPUT_LINE_RESET, CLEAR_LINE);
			namcos22_enable_slave_simulation();
			mbEnableDspIrqs = 1;
		}
		else if( newReg == 0xff )
		{ /* used to upload game-specific code to master/slave dsps */
			cputag_set_input_line(space->machine, "master", INPUT_LINE_RESET, CLEAR_LINE);
			mbEnableDspIrqs = 0;
		}
	}
} /* namcos22_system_controller_w */

/*
000064: 0000 8C9A  (1)
000068: 0000 8CA2  (2)
00006C: 0000 8CAA  (3)
000070: 0000 8CB2  (4)

00008bd8: sys[0x01] := 0x01 // scanline
00008be0: sys[0x02] := 0x02 // sci
00008be8: sys[0x03] := 0x03 // unk
00008bf0: sys[0x00] := 0x04 // vblank
00008bf8: sys[0x08] := 0xff // ?
*/
static INTERRUPT_GEN( namcos22s_interrupt )
{
	if( cpu_getiloops(device) == 0 )
	{
		int vblank_level   = nthbyte(namcos22_system_controller,0x00)&0x7; /* $700004: ack */
		cpu_set_input_line(device, vblank_level, HOLD_LINE);
		mFrameCount++;
	}
	else
	{
		//int scanline_level = nthbyte(namcos22_system_controller,0x01)&0x7; /* $700005: ack */
		//int sci_level      = nthbyte(namcos22_system_controller,0x02)&0x7; /* $700006: ack */
		//int unk_irq        = nthbyte(namcos22_system_controller,0x03)&0x7; /* $700007: ack */
		//cpu_set_input_line(device, sci_level, HOLD_LINE);
	}
}

static READ32_HANDLER( namcos22_keycus_r )
{
//  printf("Hit keycus mask %x PC=%x\n", mem_mask, cpu_get_pc(space->cpu));

	switch( namcos22_gametype )
	{
	case NAMCOS22_RIDGE_RACER:
	case NAMCOS22_RIDGE_RACER2:
		return 0x0172<<16;

	case NAMCOS22_ACE_DRIVER:
		return 0x0173;

	case NAMCOS22_CYBER_COMMANDO:
		return 0x0185;

	case NAMCOS22_ALPINE_RACER:
	case NAMCOS22_ALPINE_RACER_2:
		return 0x0187;

	case NAMCOS22_VICTORY_LAP:
		return 0x0188<<16;

	case NAMCOS22_CYBER_CYCLES:
		return 0x0387;

	case NAMCOS22_DIRT_DASH:
		return 0x1a2<<16;

	case NAMCOS22_TOKYO_WARS:
		return 0x01a8<<16;

	case NAMCOS22_ALPINE_SURFER:
		return 0x01a9;

	default:
		/* unknown/unused */
		return 0;
	}
}

/**
 * Some port values are read serially one bit at a time via word reads at
 * 0x50000008 and 0x5000000a
 *
 * Writes to 0x50000008 and 0x5000000a reset the state of the input buffer.
 *
 * Note that only the values read at 0x50000008 seem to be used in-game.
 *
 * Some of these values are redundant with respects to the work-RAM supplied input port
 * values supplied by the IO CPU.  For example, the position of the stick shift is digital,
 * and may be read through this mechanism or through shared IO RAM at 0x60004030.
 *
 * Other values seem to be digital versions of analog ports, for example "the gas pedal is
 * pressed" as a boolean flag.  IO RAM supplies it as an analog value.
 */
static READ32_HANDLER( namcos22_portbit_r )
{
	UINT32 data = mSys22PortBits;
	mSys22PortBits>>=1;
	return data&0x10001;
}
static WRITE32_HANDLER( namcos22_portbit_w )
{
	unsigned dat50000008 = AnalogAsDigital(space->machine);
	unsigned dat5000000a = 0xffff;
	mSys22PortBits = (dat50000008<<16)|dat5000000a;
}

static READ32_HANDLER( namcos22_dipswitch_r )
{
	return input_port_read(space->machine, "DSW0")<<16;
}

static READ32_HANDLER( namcos22_mcuram_r )
{
	return namcos22_shareram[offset];
}

static WRITE32_HANDLER( namcos22_mcuram_w )
{
	COMBINE_DATA(&namcos22_shareram[offset]);
}

/**
 * I don't know how "SPOT RAM" affects the display, yet.
 * I believe it's to support a "spotlight" effect, orthogonal to the polygon layer.
 * It isn't directly memory mapped, but rather ports are used to populate and poll it.
 *
 * See Time Crisis "SPOT RAM" self test for sample use.
 */
#define SPOTRAM_SIZE (320*4)

static struct
{
	int portR; /* next address for read */
	int portW; /* next address for write */
	UINT16 *RAM;//[SPOTRAM_SIZE];
} mSpotRAM;

static READ32_HANDLER( spotram_r )
{ /* 0x860004: read */
	if( offset==1 )
	{
		if( mSpotRAM.portR>=SPOTRAM_SIZE )
		{
			mSpotRAM.portR = 0;
		}
		return mSpotRAM.RAM[mSpotRAM.portR++]<<16;
	}
	return 0;
} /* spotram_r */

static WRITE32_HANDLER( spotram_w )
{ /**
   * 0x860000: set read and write address (TRUSTED by Tokyo Wars POST)
   * 0x860002: append data
   *
   * 0x860006: enable
   */
	if( offset==0 )
	{
		if( !ACCESSING_BITS_16_31 )
		{
			if( mSpotRAM.portW>=SPOTRAM_SIZE )
			{
				mSpotRAM.portW = 0;
			}
			mSpotRAM.RAM[mSpotRAM.portW++] = data;
		}
		else
		{
			mSpotRAM.portR = (data>>19)*3;
			mSpotRAM.portW = (data>>19)*3;
		}
	}
} /* spotram_w */

static READ32_HANDLER( namcos22_gun_r )
{
	int xpos = input_port_read_safe(space->machine, "LIGHTX", 0) * 640 / 0xff;
	int ypos = input_port_read_safe(space->machine, "LIGHTY", 0) * 480 / 0xff;
	switch( offset )
	{
	case 0: /* 430000 */
		return xpos<<16;

	case 1: /* 430004 */
	case 2: /* 430008 */
		return ((ypos>>1)+0x10)<<16;

	case 3:
	default:
		return 0;
	}
} /* namcos22_gun_r */

static UINT32 mAlpineSurferProtData;

static READ32_HANDLER( alpinesa_prot_r )
{
	return mAlpineSurferProtData;
} /* alpinesa_prot_r */

static WRITE32_HANDLER( alpinesa_prot_w )
{
#if 1
	switch( data )
	{
	case 0:
		mAlpineSurferProtData = 0;
		break;
	case 1:
		mAlpineSurferProtData = 1;
		break;
	case 3:
		mAlpineSurferProtData = 2;
		break;
	default:
		break;
	}
#else
	int i;
	unsigned sptr = downcast<cpu_device *>(space->cpu)->sp();
	mAlpineSurferProtData = 0;
	for(i=0;i<4;i++)
	{
		mAlpineSurferProtData<<=8;
		mAlpineSurferProtData |= memory_read_byte(space, sptr+4+i );
	}
#endif
} /* alpinesa_prot_w */

/* Namco Super System 22 */
static ADDRESS_MAP_START( namcos22s_am, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x200000, 0x200003) AM_READ(alpinesa_prot_r)
	AM_RANGE(0x300000, 0x300003) AM_WRITE(alpinesa_prot_w)
	AM_RANGE(0x000000, 0x3fffff) AM_ROM
	AM_RANGE(0x400000, 0x40001f) AM_READ(namcos22_keycus_r) AM_WRITENOP
	AM_RANGE(0x410000, 0x413fff) AM_RAM /* C139 SCI buffer */
	AM_RANGE(0x420000, 0x42000f) AM_READNOP AM_WRITENOP /* C139 SCI registers */
	AM_RANGE(0x430000, 0x43000f) AM_READ(namcos22_gun_r) AM_WRITENOP /* LEDs? */
	AM_RANGE(0x440000, 0x440003) AM_READ(namcos22_dipswitch_r)
	AM_RANGE(0x450008, 0x45000b) AM_READ(namcos22_portbit_r) AM_WRITE(namcos22_portbit_w)
	AM_RANGE(0x460000, 0x463fff) AM_RAM AM_BASE(&namcos22_nvmem) AM_SIZE(&namcos22_nvmem_size)
	AM_RANGE(0x700000, 0x70001f) AM_READ(namcos22_system_controller_r) AM_WRITE(namcos22_system_controller_w) AM_BASE(&namcos22_system_controller)
	AM_RANGE(0x800000, 0x800003) AM_WRITE(namcos22_port800000_w) /* (C304 C399)  40380000 during SPOT test */
	AM_RANGE(0x810000, 0x81000f) AM_RAM AM_BASE(&namcos22_czattr)
	AM_RANGE(0x810200, 0x8103ff) AM_READ(namcos22_czram_r) AM_WRITE(namcos22_czram_w)
	AM_RANGE(0x820000, 0x8202ff) AM_RAM /* unknown (air combat) */
	AM_RANGE(0x824000, 0x8243ff) AM_READ(namcos22_gamma_r) AM_WRITE(namcos22_gamma_w) AM_BASE(&namcos22_gamma)
	AM_RANGE(0x828000, 0x83ffff) AM_READ(namcos22_paletteram_r) AM_WRITE(namcos22_paletteram_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x860000, 0x860007) AM_READ(spotram_r) AM_WRITE(spotram_w)
	AM_RANGE(0x880000, 0x89dfff) AM_RAM_WRITE(namcos22_cgram_w) AM_BASE(&namcos22_cgram)
	AM_RANGE(0x89e000, 0x89ffff) AM_READ(namcos22_textram_r) AM_WRITE(namcos22_textram_w) AM_BASE(&namcos22_textram)
	AM_RANGE(0x8a0000, 0x8a000f) AM_RAM AM_BASE(&namcos22_tilemapattr)
	AM_RANGE(0x900000, 0x90ffff) AM_RAM AM_BASE(&namcos22_vics_data)
	AM_RANGE(0x940000, 0x94007f) AM_RAM AM_BASE(&namcos22_vics_control)
	AM_RANGE(0x980000, 0x9affff) AM_RAM AM_BASE_GENERIC(spriteram) /* C374 */
	AM_RANGE(0xa04000, 0xa0bfff) AM_READ(namcos22_mcuram_r) AM_WRITE(namcos22_mcuram_w) AM_BASE(&namcos22_shareram) /* COM RAM */
	AM_RANGE(0xc00000, 0xc1ffff) AM_READ(namcos22_dspram_r) AM_WRITE(namcos22_dspram_w) AM_BASE(&namcos22_polygonram)
	AM_RANGE(0xe00000, 0xe3ffff) AM_RAM /* workram */
ADDRESS_MAP_END

static READ16_HANDLER( s22mcu_shared_r )
{
	UINT16 *share16 = (UINT16 *)namcos22_shareram;

	return share16[BYTE_XOR_BE(offset)];
}

static WRITE16_HANDLER( s22mcu_shared_w )
{
	UINT16 *share16 = (UINT16 *)namcos22_shareram;

	COMBINE_DATA(&share16[BYTE_XOR_BE(offset)]);
}

static MACHINE_RESET(namcoss22)
{
	InitDSP(machine, 1/*super*/);
}

/*
  MCU memory map
  --------------
  000000-00027f: internal MCU registers and RAM
  002000-002fff: C352 PCM chip
  004000-00bfff: shared RAM with host CPU
  00c000-00ffff: BIOS ROM (internal on System 22, external on Super)
  200000-27ffff: data ROM
  301000-301001: watchdog?
  308000-308003: unknown (I/O?)

  pin hookups
  -----------
  5 (IRQ0): C383 custom (probably vsync)
  7 (IRQ2): 74F244 at 8c, pin 3

  port assignments
  ----------------
  port 4: "bankswitches" the controls read at port 5, probably other functions too
  port 5: on read, digital controls (buttons, coins, start, service switch)
          on write, various outputs (lamps, the fan in prop cycle, etc)
  ADC   : analog inputs

*/

// Super System 22 M37710
static ADDRESS_MAP_START( mcu_program, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x002000, 0x002fff) AM_DEVREADWRITE( "c352", c352_r, c352_w )
	AM_RANGE(0x004000, 0x00bfff) AM_READWRITE( s22mcu_shared_r, s22mcu_shared_w )
	AM_RANGE(0x00c000, 0x00ffff) AM_ROM AM_REGION("user4", 0xc000)
	AM_RANGE(0x080000, 0x0fffff) AM_ROM AM_REGION("user4", 0)
	AM_RANGE(0x200000, 0x27ffff) AM_ROM AM_REGION("user4", 0)
	AM_RANGE(0x280000, 0x2fffff) AM_ROM AM_REGION("user4", 0)
	AM_RANGE(0x301000, 0x301001) AM_NOP	// watchdog? LEDs?
	AM_RANGE(0x308000, 0x308003) AM_NOP	// volume control IC?
ADDRESS_MAP_END

// System 22 37702
static ADDRESS_MAP_START( mcu_s22_program, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x002000, 0x002fff) AM_DEVREADWRITE( "c352", c352_r, c352_w )
	AM_RANGE(0x004000, 0x00bfff) AM_READWRITE( s22mcu_shared_r, s22mcu_shared_w )
	AM_RANGE(0x00c000, 0x00ffff) AM_ROM AM_REGION("mcu", 0)
	AM_RANGE(0x080000, 0x0fffff) AM_ROM AM_REGION("user4", 0)
	AM_RANGE(0x200000, 0x27ffff) AM_ROM AM_REGION("user4", 0)
	AM_RANGE(0x280000, 0x2fffff) AM_ROM AM_REGION("user4", 0)
	AM_RANGE(0x301000, 0x301001) AM_NOP	// watchdog? LEDs?
	AM_RANGE(0x308000, 0x308003) AM_NOP	// volume control IC?
ADDRESS_MAP_END

static int p4;

static READ8_HANDLER( mcu_port5_r )
{
	// hack for motor status
	if ((namcos22_gametype == NAMCOS22_ALPINE_RACER) || (namcos22_gametype == NAMCOS22_ALPINE_RACER_2) || (namcos22_gametype == NAMCOS22_ALPINE_SURFER))
	{
		if (p4 & 8)
		{
			if (mFrameCount & 1)
			{
				return input_port_read_safe(space->machine, "MCUP5A", 0xff) | 0x80;
			}
			else
			{
				return input_port_read_safe(space->machine, "MCUP5A", 0xff) & 0x7f;
			}
		}
		else
		{
			if (mFrameCount & 1)
			{
				return 0xfe;
			}
			else
			{
				return 0xff;
			}
		}
	}
	else
	{
		if (p4 & 8)
		{
			return input_port_read_safe(space->machine, "MCUP5A", 0xff);
		}
		else
		{
			return input_port_read_safe(space->machine, "MCUP5B", 0xff);
		}
	}
}

static WRITE8_HANDLER( mcu_port4_w )
{
	p4 = data;
}

static READ8_HANDLER( mcu_port4_r )
{
	return p4;
}

static WRITE8_HANDLER( mcu_port5_w )
{
	// prop cycle outputs:
	// bit 1 = fan
	// bit 2 = button light

	if (!strcmp(space->machine->gamedrv->name, "propcycl"))
	{
		output_set_value("fan0", data & 1);
		set_led_status(space->machine, 0, data & 2);
	}
}

static READ8_HANDLER( mcu_port6_r )
{
	return 0;
}

static READ8_HANDLER( mcu_port7_r )
{
	return 0;
}

// H+L = horizontal, 1 H+L = vertical
static READ8_HANDLER( propcycle_mcu_adc_r )
{
	static UINT16 ddx, ddy;

	ddx = input_port_read(space->machine, "STICKX")^0xff;
	if (ddx > 0) ddx -= 1;
	ddy = input_port_read(space->machine, "STICKY");
	if (ddy > 0) ddy -= 1;

	ddx <<= 2;
	ddy <<= 2;

	switch (offset)
	{
		case 0:
			// also update the pedal here
			//
			// this is a wee bit hackish: the way it actually works is like so:
			// the pedal has a simple 1-bit "light interrupted" sensor.  the faster you pedal,
			// the faster it pulses.  this is connected to the clock input for timer A3,
			// and timer A3 is configured by the MCU program to cause an interrupt each time
			// it's clocked.  by counting the number of interrupts in a frame, we can determine
			// how fast the user is pedaling.
			if( input_port_read(space->machine, "JOY") & 0x10 )
			{
				int i;
				for (i = 0; i < 16; i++)
				{
					generic_pulse_irq_line(space->machine->device("mcu"), M37710_LINE_TIMERA3TICK);
				}
			}

			return (ddx & 0xff);
		case 1:
			return (ddx>>8);
		case 2:
			return (ddy & 0xff);
		case 3:
			return (ddy>>8);
		default:
			return 0;
	}
}

// 0 H+L = swing, 1 H+L = edge
static READ8_HANDLER( alpineracer_mcu_adc_r )
{
	UINT16 swing = (0xff - input_port_read(space->machine, "SWING"))<<2;
	UINT16 edge = (0xff - input_port_read(space->machine, "EDGE"))<<2;

	// fake out the centering a bit
	if (input_port_read(space->machine, "SWING") == 0x80) swing = 0x200;
	if (input_port_read(space->machine, "EDGE") == 0x80) edge = 0x200;

	switch (offset)
	{
		case 0:
			return swing & 0xff;

		case 1:
			return (swing>>8);

		case 2:
			return edge & 0xff;

		case 3:
			return (edge>>8);

		default:
			return 0;
	}
}

static READ8_HANDLER( cybrcycc_mcu_adc_r )
{
	UINT16 gas,brake,steer;
	ReadAnalogDrivingPorts( space->machine, &gas, &brake, &steer );

	gas <<= 2;
	brake <<= 2;
	steer <<= 2;

	switch (offset)
	{
		case 0:
			return steer & 0xff;

		case 1:
			return (steer>>8);

		case 2:
			return gas & 0xff;

		case 3:
			return (gas>>8);

		case 4:
			return brake & 0xff;

		case 5:
			return (brake>>8);

		default:
			return 0;
	}
}

static READ8_HANDLER( tokyowar_mcu_adc_r )
{
	UINT16 gas,brake,steer;
	ReadAnalogDrivingPorts( space->machine, &gas, &brake, &steer );

	gas <<= 2;
	brake <<= 2;
	steer <<= 2;

	switch (offset)
	{
		case 0:
			return steer & 0xff;

		case 1:
			return (steer>>8);

		case 4:
			return gas & 0xff;

		case 5:
			return (gas>>8);

		case 6:
			return brake & 0xff;

		case 7:
			return (brake>>8);

		default:
			return 0;
	}
}

static READ8_HANDLER( aquajet_mcu_adc_r )
{
	UINT16 gas, steer, ddy;

	gas   = input_port_read(space->machine, "GAS") ^ 0x7f;
	steer = input_port_read(space->machine, "STEER") ^ 0xff;
	ddy = input_port_read(space->machine, "STICKY");
	if (ddy > 0) ddy -= 1;

	gas <<= 2;
	steer <<= 2;
	ddy <<= 2;

	/*
        0 & 1 = handle left/right
        2 & 3 = accelerator
        4 & 5 = handle pole (Y axis)

    */

	switch (offset)
	{
		case 0:
			return steer & 0xff;

		case 1:
			return (steer>>8);

		case 2:
			return gas & 0xff;

		case 3:
			return (gas>>8);

		case 4:
			return ddy & 0xff;

		case 5:
		    	return (ddy>>8);

		default:
			return 0;
	}
}

static READ8_HANDLER( airco22_mcu_adc_r )
{
	UINT16 pedal, x, y;

	pedal = input_port_read(space->machine, "PEDAL")<<2;
	x = input_port_read(space->machine, "STICKX")<<2;
	y = input_port_read(space->machine, "STICKY")<<2;


	switch (offset)
	{
		case 0:
			return x & 0xff;

		case 1:
			return (x>>8);

		case 2:
			return y & 0xff;

		case 3:
			return (y>>8);

		case 4:
			return pedal & 0xff;

		case 5:
			return (pedal>>8);

		default:
			return 0;
	}
}

static ADDRESS_MAP_START( mcu_io, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(M37710_PORT4, M37710_PORT4) AM_READ( mcu_port4_r ) AM_WRITE( mcu_port4_w )
	AM_RANGE(M37710_PORT5, M37710_PORT5) AM_READ( mcu_port5_r ) AM_WRITE( mcu_port5_w )
	AM_RANGE(M37710_PORT6, M37710_PORT6) AM_READ( mcu_port6_r ) AM_WRITENOP
	AM_RANGE(M37710_PORT7, M37710_PORT7) AM_READ( mcu_port7_r )
ADDRESS_MAP_END

static READ8_HANDLER( mcu_port4_s22_r )
{
	return p4 | 0x10;	// for C74, 0x10 selects sound MCU role, 0x00 selects control-reading role
}

static ADDRESS_MAP_START( mcu_s22_io, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(M37710_PORT4, M37710_PORT4) AM_READ( mcu_port4_s22_r )
ADDRESS_MAP_END

static INTERRUPT_GEN( mcu_interrupt )
{
	if (cpu_getiloops(device) == 0)
	{
		cpu_set_input_line(device, M37710_LINE_IRQ0, HOLD_LINE);
	}
	else if (cpu_getiloops(device) == 1)
	{
		cpu_set_input_line(device, M37710_LINE_IRQ2, HOLD_LINE);
	}
	else
	{
		cpu_set_input_line(device, M37710_LINE_ADC, HOLD_LINE);
	}
}

static MACHINE_DRIVER_START( namcos22s )
	MDRV_CPU_ADD("maincpu", M68EC020,SS22_MASTER_CLOCK/2)
	MDRV_CPU_PROGRAM_MAP(namcos22s_am)
	MDRV_CPU_VBLANK_INT_HACK(namcos22s_interrupt,2)

	MDRV_CPU_ADD("master", TMS32025,SS22_MASTER_CLOCK)
	MDRV_CPU_PROGRAM_MAP(master_dsp_program)
	MDRV_CPU_DATA_MAP(master_dsp_data)
	MDRV_CPU_IO_MAP(master_dsp_io)
	MDRV_CPU_VBLANK_INT_HACK(dsp_serial_pulse1,SERIAL_IO_PERIOD)

	MDRV_CPU_ADD("slave", TMS32025,SS22_MASTER_CLOCK)
	MDRV_CPU_PROGRAM_MAP(slave_dsp_program)
	MDRV_CPU_DATA_MAP(slave_dsp_data)
	MDRV_CPU_IO_MAP(slave_dsp_io)

	MDRV_CPU_ADD("mcu", M37710, SS22_MASTER_CLOCK/3)
	MDRV_CPU_PROGRAM_MAP(mcu_program)
	MDRV_CPU_IO_MAP( mcu_io)
	MDRV_CPU_VBLANK_INT_HACK(mcu_interrupt, 3)

	MDRV_QUANTUM_TIME(HZ(6000))

	MDRV_NVRAM_HANDLER(namcos22)
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(NAMCOS22_NUM_COLS*16,NAMCOS22_NUM_ROWS*16)
	MDRV_SCREEN_VISIBLE_AREA(0,NAMCOS22_NUM_COLS*16-1,0,NAMCOS22_NUM_ROWS*16-1)

	MDRV_PALETTE_LENGTH(NAMCOS22_PALETTE_SIZE)
	MDRV_GFXDECODE(super)
	MDRV_VIDEO_START(namcos22s)
	MDRV_VIDEO_UPDATE(namcos22s)
	MDRV_MACHINE_RESET(namcoss22)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("c352", C352, SS22_MASTER_CLOCK/3)
	MDRV_SOUND_ROUTE(0, "rspeaker", 1.00)
	MDRV_SOUND_ROUTE(1, "lspeaker", 1.00)
	MDRV_SOUND_ROUTE(2, "rspeaker", 1.00)
	MDRV_SOUND_ROUTE(3, "lspeaker", 1.00)
MACHINE_DRIVER_END

/*********************************************************************************/

/* Namco System 22 */

static ADDRESS_MAP_START( namcos22_am, ADDRESS_SPACE_PROGRAM, 32 )
	/**
     * Program ROM (2M bytes)
     * Mounted position: LLB: CPU 4D, LMB: CPU 2D, UMB: CPU 8D, UUB: CPU 6D
     * Known ROM chip type: TI TMS27C040-10, ST M27C4001-10, M27C4001-12Z
     */
	AM_RANGE(0x00000000, 0x001fffff) AM_ROM

	/**
     * Main RAM (128K bytes)
     * Mounted position: CPU 3D, 5D, 7D, 9D
     * Known DRAM chip type: TC55328P-25, N3441256P-15
     */
	AM_RANGE(0x10000000, 0x1001ffff) AM_RAM

	/**
     * Main RAM (Mirror or Another Bank)
     * Mounted position: unknown
     */
	AM_RANGE(0x18000000, 0x1801ffff) AM_RAM

	/**
     * KEYCUS
     * Mounted position: CPU 13R
     * Known chip type:
     *     C370  (Ridge Racer, Ridge Racer 2)
     *     C388  (Rave Racer)
     *     C389? (Cyber Cycles)
     *     C392? (Ace Driver Victory Lap)
     */
	AM_RANGE(0x20000000, 0x2000000f) AM_READ(namcos22_keycus_r) AM_WRITENOP

	/**
     * C139 SCI Buffer
     * Mounted position: CPU 4N
     * Known chip type: M5M5179AP-25 (8k x 9bit SRAM)
     *
     * Note: Boot time check: 20010000 - 20011fff / bits=0x000001ff
     *
     *     20010000 - 20011fff  TX Buffer
     *     20012000 - 20013fff  RX FIFO Buffer (also used for TX Buffer)
     */
	AM_RANGE(0x20010000, 0x20013fff) AM_RAM

	/**
     * C139 SCI Register
     * Mounted position: CPU 4R
     *
     *     20020000  2  R/W RX Status
     *         0x01 : Frame Error
     *         0x02 : Frame Received
     *         0x04 : ?
     *
     *     20020002  2  R/W Status/Control Flags
     *         0x01 :
     *         0x02 : RX flag? (cleared every vsync)
     *         0x04 : RX flag? (cleared every vsync)
     *         0x08 :
     *
     *     20020004  2  W   FIFO Control Register
     *         0x01 : sync bit enable?
     *         0x02 : TX FIFO sync bit (bit-8)
     *
     *     20020006  2  W   TX Control Register
     *         0x01 : TX start/stop
     *         0x02 : ?
     *         0x10 : ?
     *
     *     20020008  2  W   -
     *     2002000a  2  W   TX Frame Size
     *     2002000c  2  R/W RX FIFO Pointer (0x0000 - 0x0fff)
     *     2002000e  2  W   TX FIFO Pointer (0x0000 - 0x1fff)
     */
	AM_RANGE(0x20020000, 0x2002000f) AM_READ(namcos22_C139_SCI_r) AM_WRITEONLY

	/**
     * System Controller: Interrupt Control, Peripheral Control
     *
     * 40000000 IRQ (unknown)
     * 40000001
     * 40000002 SCI IRQ level
     * 40000003 IRQ (unknown)

     * 40000004 VSYNC IRQ level
     * 40000005 IRQ (unknown) acknowledge
     * 40000006
     * 40000007 SCI IRQ acknowledge

     * 40000008 IRQ (unknown) acknowledge
     * 40000009 VSYNC IRQ acknowledge
     * 4000000a
     * 4000000b ?

     * 4000000c ?
     * 4000000d
     * 4000000e ?
     * 4000000f

     * 40000010 ?
     * 40000011 ?
     * 40000012 ?
     * 40000013 ?

     * 40000014 ?
     * 40000015 ? (cyc1)
     * 40000016 Watchdog timer reset
     * 40000017

     * 40000018 0 or 1 -> DSP control (reset?)
     * 40000019 sub cpu reset?
     * 4000001a 0 or 1 or 0xff -> DSP control
     * 4000001b ?

     * 4000001c
     * 4000001d
     * 4000001e
     * 4000001f
     */
	AM_RANGE(0x40000000, 0x4000001f) AM_READ(namcos22_system_controller_r) AM_WRITE(namcos22_system_controller_w) AM_BASE(&namcos22_system_controller)

	/**
     * Unknown Device (optional for diagnostics?)
     *
     * zero means not-connected.
     * may be related to device at 0x94000000
     */
	AM_RANGE(0x48000000, 0x4800003f) AM_READNOP AM_WRITENOP

	/**
     * DIPSW
     *     0x50000000 - DIPSW3
     *     0x50000001 - DIPSW2
     */
	AM_RANGE(0x50000000, 0x50000003) AM_READ(namcos22_dipswitch_r) AM_WRITENOP
	AM_RANGE(0x50000008, 0x5000000b) AM_READ(namcos22_portbit_r) AM_WRITE(namcos22_portbit_w)

	/**
     * EEPROM
     * Mounted position: CPU 9E
     * Known chip type: HN58C65P-25 (8k x 8bit EEPROM)
     */
	AM_RANGE(0x58000000, 0x58001fff) AM_RAM AM_BASE(&namcos22_nvmem) AM_SIZE(&namcos22_nvmem_size)

	/**
     * C74 (Mitsubishi M37702 MCU) Shared RAM (0x60004000 - 0x6000bfff)
     * Mounted position: CPU 11L, 12L
     * Known chip type: TC55328P-25, N341256P-15
     *
     * DATA ROM for C74 (SEQ data and external code):
     * Known chip type: NEC D27C4096D-12
     * Notes: C74(CPU PCB) sends/receives I/O data from C74(I/O PCB) by SIO.
     *
     * 0x60004020 b4 = 1 : ???
     * 0x60004022.w     Volume(R)
     * 0x60004024.w     Volume(L)
     * 0x60004026.w     Volume(R) (maybe rear channels, not put on real PCB)
     * 0x60004028.w     Volume(L) (maybe rear channels, not put on real PCB)
     * 0x60004030 b0     : system type 0
     * 0x60004030 b1 = 0 : COIN2
     * 0x60004030 b2 = 0 : TEST SW
     * 0x60004030 b3 = 0 : SERVICE SW
     * 0x60004030 b4 = 0 : COIN1
     * 0x60004030 b5     : system type 1
     * (system type on RR2 (00:50inch, 01:two in one, 20:standard, 21:deluxe))
     * 0x60004031 b0 = 0 : SWITCH1 (for manual transmission)
     * 0x60004031 b1 = 0 : SWITCH2
     * 0x60004031 b2 = 0 : SWITCH3
     * 0x60004031 b3 = 0 : SWITCH4
     * 0x60004031 b4 = 0 : CLUTCH
     * 0x60004031 b6 = 0 : VIEW SW
     * 0x60004032.w     Handle A/D (=steering wheel, default of center value is different in each game)
     * 0x60004034.w     Gas A/D
     * 0x60004036.w     Brake A/D
     * 0x60004038.w     A/D3 (reserved)
     * (some GOUT (general outputs for lamps, etc) is also mapped this area)
     * 0x60004080       Data/Code for Sub-CPU
     * 0x60004200       Data/Code for Sub-CPU
     * 0x60005000 - 0x6000bfff  Sound Work
     * +0x0000 - 0x003f Song Request #00 to 31
     * +0x0100 - 0x02ff Parameter RAM from Main MPU (for SEs)
     * +0x0300 - 0x03ff?    Song Title (put messages here from Sound CPU)
     */
	AM_RANGE(0x60000000, 0x60003fff) AM_WRITENOP
	AM_RANGE(0x60004000, 0x6000bfff) AM_READ(namcos22_mcuram_r) AM_WRITE(namcos22_mcuram_w) AM_BASE(&namcos22_shareram)

	/**
     * C71 (TI TMS320C25 DSP) Shared RAM (0x70000000 - 0x70020000)
     * Mounted position:
     *     C71: CPU 15R, 21R
     *     RAM: CPU 15K, 13E, 12E
     * Known chip type: TC55328P-25, N341256P-15
     * Notes: connected bits = 0x00ffffff (24bit)
     */
	AM_RANGE(0x70000000, 0x7001ffff) AM_READ(namcos22_dspram_r) AM_WRITE(namcos22_dspram_w) AM_BASE(&namcos22_polygonram)

	/**
     * LED on PCB(?)
     */
	AM_RANGE(0x90000000, 0x90000003) AM_RAM

	/**
     * Depth-cueing Look-up Table (fog density between near to far)
     * Mounted position: VIDEO 8P
     * Known chip type: TC55328P-25
     */
	AM_RANGE(0x90010000, 0x90017fff) AM_RAM /* AM_BASE(&namcos22_czram) */

	/**
     * C305 (Display Controller)
     * Mounted position: VIDEO 7D (C305)
     *
     * +0x0002.w    Fader Enable(?) (0: disabled)
     * +0x0011.w    Display Fader (R) (0x0100 = 1.0)
     * +0x0013.w    Display Fader (G) (0x0100 = 1.0)
     * +0x0015.w    Display Fader (B) (0x0100 = 1.0)
     * +0x0100.b    Fog1 Color (R) (world fogging)
     * +0x0101.b    Fog2 Color (R) (used for heating of brake-disc on RV1)
     * +0x0180.b    Fog1 Color (G)
     * +0x0181.b    Fog2 Color (G)
     * +0x0200.b    Fog1 Color (B)
     * +0x0201.b    Fog2 Color (B)
     * (many unknown registers are here)
     *
     * Notes: Boot time check: 0x90020100 - 0x9002027f
     */
	AM_RANGE(0x90020000, 0x90027fff) /*AM_RAM*/ AM_READ(namcos22_gamma_r) AM_WRITE(namcos22_gamma_w) AM_BASE(&namcos22_gamma)

	/**
     * 0x90028000 - 0x9002ffff  Palette (R)
     * 0x90030000 - 0x90037fff  Palette (G)
     * 0x90038000 - 0x9003ffff  Palette (B)
     *
     * Mounted position: VIDEO 6B, 7B, 8B (near C305)
     * Note: 0xff00-0xffff are for Tilemap (16 x 16)
     */
	AM_RANGE(0x90028000, 0x9003ffff) AM_RAM_WRITE(namcos22_paletteram_w) AM_BASE_GENERIC(paletteram)

	/**
     * unknown (option)
     * Note: This device may be optional. This may relate to device at 0x40000000
     */
	AM_RANGE(0x90040000, 0x9007ffff) AM_RAM /* diagnostic ROM? */

	/**
     * Tilemap PCG Memory
     */
	AM_RANGE(0x90080000, 0x9009dfff) AM_RAM_WRITE(namcos22_cgram_w) AM_BASE(&namcos22_cgram)

	/**
     * Tilemap Memory (64 x 64)
     * Mounted position: VIDEO  2K
     * Known chip type: HM511664 (64k x 16bit SRAM)
     * Note: Self test: 90084000 - 9009ffff
     */
	AM_RANGE(0x9009e000, 0x9009ffff) AM_RAM_WRITE(namcos22_textram_w) AM_BASE(&namcos22_textram)

	/**
     * Tilemap Register
     * Mounted position: unknown
     * +0x0000 Position X
     * +0x0002 Position Y
     */
	AM_RANGE(0x900a0000, 0x900a000f) AM_RAM AM_BASE(&namcos22_tilemapattr)
ADDRESS_MAP_END

static INTERRUPT_GEN( namcos22_interrupt )
{
	int irq_level1 = 5;
	int irq_level2 = 6;

	switch( namcos22_gametype )
	{
	case NAMCOS22_RIDGE_RACER:
		HandleDrivingIO(device->machine);
		irq_level1 = 4;
		irq_level2 = 5;
		// 1:0a0b6
		// 2:09fe8 (rte)
		// 3:09fe8 (rte)
		// 4:09f9a (vblank)
		// 5:14dee (SCI)
		// 6:09fe8 (rte)
		// 7:09fe8 (rte)
		break;

	case NAMCOS22_RIDGE_RACER2:
		HandleDrivingIO(device->machine);
		irq_level1 = 1;
		irq_level2 = 5;
		//1:0d10c  40000005
		//2:0cfa2 (rte)
		//3:0cfa2 (rte)
		//4:0cfa2 (rte)
		//5:0cef0
		//6:1bbcc
		//7:0cfa2 (rte)
		break;

	case NAMCOS22_RAVE_RACER:
		HandleDrivingIO(device->machine);
		break;

	case NAMCOS22_VICTORY_LAP:
		HandleDrivingIO(device->machine);
		// a54 indir to 21c2 (hblank?)
		// a5a (rte)
		// a5c (rte)
		// a5e (rte)
		// a60 irq
		// abe indirect to 27f1e (SCI)
		// ac4 (rte)
		irq_level1 = nthbyte(namcos22_system_controller,0x04)&0x7;
		irq_level2 = nthbyte(namcos22_system_controller,0x02)&0x7;
		break;

	case NAMCOS22_ACE_DRIVER:
		HandleDrivingIO(device->machine);
		// 9f8 (rte)
		// 9fa (rte)
		// 9fc (rte)
		// 9fe (rte)
		// a00
		// a46
		// a4c (rte)
		irq_level1 = 5;
		irq_level2 = 6;
		break;

	case NAMCOS22_CYBER_COMMANDO:
		//move.b  #$36, $40000002.l
		//move.b  # $0, $40000003.l
		//move.b  #$35, $40000004.l
		//
		//move.b  #$34, $40000004.l
		HandleCyberCommandoIO(device->machine);
		irq_level1 = nthbyte(namcos22_system_controller,0x04)&0x7;
		irq_level2 = nthbyte(namcos22_system_controller,0x02)&0x7;
		break;

	default:
		break;
	}

	switch( cpu_getiloops(device) )
	{
	case 0:
		if( irq_level1 )
		{
			cpu_set_input_line(device, irq_level1, HOLD_LINE); /* vblank */
		}
		break;
	case 1:
		if( irq_level2 )
		{
			cpu_set_input_line(device, irq_level2, HOLD_LINE); /* SCI */
		}
		break;
	}
}

static MACHINE_RESET(namcos22)
{
	InitDSP(machine, 0);
}

static MACHINE_DRIVER_START( namcos22 )
	MDRV_CPU_ADD("maincpu", M68020,SS22_MASTER_CLOCK/2) /* 25 MHz? */
	MDRV_CPU_PROGRAM_MAP(namcos22_am)
	MDRV_CPU_VBLANK_INT_HACK(namcos22_interrupt,2)

	MDRV_CPU_ADD("master", TMS32025,SS22_MASTER_CLOCK) /* ? */
	MDRV_CPU_PROGRAM_MAP(master_dsp_program)
	MDRV_CPU_DATA_MAP(master_dsp_data)
	MDRV_CPU_IO_MAP(master_dsp_io)
	MDRV_CPU_VBLANK_INT_HACK(dsp_serial_pulse1,SERIAL_IO_PERIOD)

	MDRV_CPU_ADD("slave", TMS32025,SS22_MASTER_CLOCK) /* ? */
	MDRV_CPU_PROGRAM_MAP(slave_dsp_program)
	MDRV_CPU_DATA_MAP(slave_dsp_data)
	MDRV_CPU_IO_MAP(slave_dsp_io)

	MDRV_CPU_ADD("mcu", M37702, SS22_MASTER_CLOCK/3)	// C74 on the CPU board has no periodic interrupts, it runs entirely off Timer A0
	MDRV_CPU_PROGRAM_MAP( mcu_s22_program)
	MDRV_CPU_IO_MAP( mcu_s22_io)

	MDRV_NVRAM_HANDLER(namcos22)
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(NAMCOS22_NUM_COLS*16,NAMCOS22_NUM_ROWS*16)
	MDRV_SCREEN_VISIBLE_AREA(0,NAMCOS22_NUM_COLS*16-1,0,NAMCOS22_NUM_ROWS*16-1)

	MDRV_PALETTE_LENGTH(NAMCOS22_PALETTE_SIZE)
	MDRV_GFXDECODE(namcos22)
	MDRV_VIDEO_START(namcos22)
	MDRV_VIDEO_UPDATE(namcos22)
	MDRV_MACHINE_RESET(namcos22)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("c352", C352, SS22_MASTER_CLOCK/3)
	MDRV_SOUND_ROUTE(0, "rspeaker", 1.00)
	MDRV_SOUND_ROUTE(1, "lspeaker", 1.00)
	MDRV_SOUND_ROUTE(2, "rspeaker", 1.00)
	MDRV_SOUND_ROUTE(3, "lspeaker", 1.00)
MACHINE_DRIVER_END

/*********************************************************************************/

ROM_START( airco22b )
	ROM_REGION( 0x400000, "maincpu", 0 ) /* main program */
	ROM_LOAD32_BYTE( "acs1verb.1", 0x00003, 0x100000, CRC(062c4f61) SHA1(98e1c75dd0f493eb6ebb64b46543217c1d40116e) )
	ROM_LOAD32_BYTE( "acs1verb.2", 0x00002, 0x100000, CRC(8ae69711) SHA1(4c5323fa8f0419275e330fec66d1fb2b89bb3795) )
	ROM_LOAD32_BYTE( "acs1verb.3", 0x00001, 0x100000, CRC(71738e67) SHA1(eb8c66dedbeff911b6166ebbda466fb9656ef0fb) )
	ROM_LOAD32_BYTE( "acs1verb.4", 0x00000, 0x100000, CRC(3b193add) SHA1(5e3bca13905bfa3a2947f4f16ca01878b0a14a3a) )

	ROM_REGION( 0x10000*2, "master", 0 ) /* Master DSP */
	ROM_LOAD16_WORD( "c71.bin", 0,0x1000*2, CRC(47c623ab) SHA1(e363ac50f5556f83308d4cc191b455e9b62bcfc8) )

	ROM_REGION( 0x10000*2, "slave", 0 ) /* Slave DSP */
	ROM_LOAD16_WORD( "c71.bin", 0,0x1000*2, CRC(47c623ab) SHA1(e363ac50f5556f83308d4cc191b455e9b62bcfc8) )

	ROM_REGION( 0x080000, "mcu", ROMREGION_ERASE00 ) /* S22-BIOS ver1.30 */

	ROM_REGION( 0x080000, "user4", 0 ) /* MCU BIOS */
	ROM_LOAD( "acs1data.8k", 0, 0x080000, CRC(33824bc9) SHA1(80ec63883770e5eec1f5f1ddc16a85ef8f22a48b) )

	ROM_REGION( 0x200000*2, "sprite", 0 ) /* 32x32x8bpp sprite tiles */
	ROM_LOAD( "acs1scg0.12l", 0x200000*0, 0x200000,CRC(e5235404) SHA1(3133b71d1bde3a9815cd02e97382b8078b62b0bb) )
	ROM_LOAD( "acs1scg1.10l", 0x200000*1, 0x200000,CRC(828e91e7) SHA1(8383b029cd29fbad107fd49e866defb50c11c99a) )

	ROM_REGION( 0x200000*8, "textile", 0) /* 16x16x8bpp texture tiles */
	ROM_LOAD( "acs1cg0.8d",  0x200000*0x0, 0x200000,CRC(1f31343e) SHA1(25ba730cec74e0ed0b404f5c4430b7c3368c9b52) )
	ROM_LOAD( "acs1cg1.10d", 0x200000*0x1, 0x200000,CRC(ccd5481d) SHA1(050e6fc7d4e0591f8ffc9552d140b6bd4533c06d) )
	ROM_LOAD( "acs1cg2.12d", 0x200000*0x2, 0x200000,CRC(14e5d0d2) SHA1(3147ad11098030e9cfd93fbc0a1b3aafa8b8aba6) )
	ROM_LOAD( "acs1cg3.13d", 0x200000*0x3, 0x200000,CRC(1a7bcc16) SHA1(bbc4ca5b208bea8394d1679e4e2d17d22331e2c8) )
	ROM_LOAD( "acs1cg4.14d", 0x200000*0x4, 0x200000,CRC(1920b7fb) SHA1(56318f2a96c55998bb9a8d791d56be3dfb39867e) )
	ROM_LOAD( "acs1cg5.16d", 0x200000*0x5, 0x200000,CRC(3dd109b7) SHA1(a7f914b9b80f1bca1afb6144698578a29ca74676) )
	ROM_LOAD( "acs1cg6.18d", 0x200000*0x6, 0x200000,CRC(ec71c8a3) SHA1(86892a91883d483ca0d422b78fa36042e02f3ad3) )
	ROM_LOAD( "acs1cg7.19d", 0x200000*0x7, 0x200000,CRC(82271757) SHA1(023c935e78b14da310e4c29da8785b82aa3241ac) )

	ROM_REGION16_LE( 0x280000, "textilemap", 0 ) /* texture tilemap */
	ROM_LOAD( "acs1ccrl.3d",	 0x000000, 0x200000,CRC(07088ba1) SHA1(a962c0821d5af28ed508cfdbd613675454e306e3) )
	ROM_LOAD( "acs1ccrh.1d",	 0x200000, 0x080000,CRC(62936af6) SHA1(ca80b68415aa2cd2ce4e90404f10640d0ae38be9) )

	ROM_REGION( 0x80000*12, "pointrom", 0 ) /* 3d model data */
	ROM_LOAD( "acs1ptl0.18k", 0x80000*0x0, 0x80000,CRC(bd5896c7) SHA1(58ec7d0f1e0bfdbf4908e1d920bbd7f094993777) )
	ROM_LOAD( "acs1ptl1.16k", 0x80000*0x1, 0x80000,CRC(e583b975) SHA1(beb0cc2b44bc69af057c2bb744cd7e1b95de577a) )
	ROM_LOAD( "acs1ptl2.15k", 0x80000*0x2, 0x80000,CRC(802d737a) SHA1(3d99a369db70d13fb87c2ff26c82b4b39afe94d9) )
	ROM_LOAD( "acs1ptl3.14k", 0x80000*0x3, 0x80000,CRC(fe556ecb) SHA1(9d9dbbb4f1d3688fb763001834640d79d9987d47) )

	ROM_LOAD( "acs1ptm0.18j", 0x80000*0x4, 0x80000,CRC(949b6c58) SHA1(6ea016551b10f5d5764921dcc5a4b81d2b93d701) )
	ROM_LOAD( "acs1ptm1.16j", 0x80000*0x5, 0x80000,CRC(8b2b99d9) SHA1(89c3545c4035509307728a9577018c1100ce3a54) )
	ROM_LOAD( "acs1ptm2.15j", 0x80000*0x6, 0x80000,CRC(f1515080) SHA1(27a87217a140477a6840a610c95ae57abc0d01a6) )
	ROM_LOAD( "acs1ptm3.14j", 0x80000*0x7, 0x80000,CRC(e364f4aa) SHA1(3af6a864765871664cccad82c4795f677be68d51) )

	ROM_LOAD( "acs1ptu0.18f", 0x80000*0x8, 0x80000,CRC(746b3084) SHA1(73397d1f22300fb3a81a0a068da4d0a8cfdc0a36) )
	ROM_LOAD( "acs1ptu1.16f", 0x80000*0x9, 0x80000,CRC(b44f1d3b) SHA1(f3f1a85c082053653e4da7d7f01f1baef1a013c8) )
	ROM_LOAD( "acs1ptu2.15f", 0x80000*0xa, 0x80000,CRC(fdd2d778) SHA1(0269f971d778e908a1efb5a63b08fb3365d98c2a) )
	ROM_LOAD( "acs1ptu3.14f", 0x80000*0xb, 0x80000,CRC(38b425d4) SHA1(8ff6dd6775d42afdff4c9fb2232e4d72b38e515a) )

	ROM_REGION( 0xc00000, "c352", 0 ) /* sound samples */
	ROM_LOAD( "acs1wav0.1", 0x400000, 0x400000, CRC(52fb9762) SHA1(125c163e62d701c2e17ba0b572ed27c944ca0412) )
	ROM_LOAD( "acs1wav1.2", 0x800000, 0x400000, CRC(b568dca2) SHA1(503deb277691d801acac1380ded2868a5d5ac501) )
ROM_END

ROM_START( alpinerc )
	ROM_REGION( 0x400000, "maincpu", 0 ) /* main program */
	ROM_LOAD32_BYTE( "ar2ver-c.1", 0x00003, 0x100000, CRC(61323842) SHA1(e3c33248340bee252f230124fa9b7fa935a60565) )
	ROM_LOAD32_BYTE( "ar2ver-c.2", 0x00002, 0x100000, CRC(43795b2d) SHA1(e060f3259661279a36300431c5ca7347bde8b6ec) )
	ROM_LOAD32_BYTE( "ar2ver-c.3", 0x00001, 0x100000, CRC(acb3003b) SHA1(ea0cbf3a1607b06b108df051f38fec1f214f42d2) )
        ROM_LOAD32_BYTE( "ar2ver-c.4", 0x00000, 0x100000, CRC(800acc21) SHA1(41d26766da2db46954a2351bbc50aea94bc1d564) )

	ROM_REGION( 0x10000*2, "master", 0 ) /* Master DSP */
	ROM_LOAD16_WORD( "c71.bin", 0,0x1000*2, CRC(47c623ab) SHA1(e363ac50f5556f83308d4cc191b455e9b62bcfc8) )

	ROM_REGION( 0x10000*2, "slave", 0 ) /* Slave DSP */
	ROM_LOAD16_WORD( "c71.bin", 0,0x1000*2, CRC(47c623ab) SHA1(e363ac50f5556f83308d4cc191b455e9b62bcfc8) )

	ROM_REGION( 0x080000, "mcu", ROMREGION_ERASE00 ) /* S22-BIOS ver1.30 */

	ROM_REGION( 0x080000, "user4", 0 ) /* MCU BIOS */
	ROM_LOAD( "ar1datab.8k", 0, 0x080000, CRC(c26306f8) SHA1(6d8d993c076d5ced523143a86bd0938b3794478d) )

	ROM_REGION( 0x200000*2, "sprite", 0 ) /* 32x32x8bpp sprite tiles */
	ROM_LOAD( "ar1scg0.12f", 0x200000*0, 0x200000,CRC(e7be830a) SHA1(60e2162eecd7401a0c26c525de2715cbfb10c1c5) ) /* identical to "ar1scg0.12l" */
	ROM_LOAD( "ar1scg1.10f", 0x200000*1, 0x200000,CRC(8f15a686) SHA1(bce2d4380c6c39aa402566ddb0f62bbe6d7bfa1d) ) /* identical to "ar1scg1.10l" */

	ROM_REGION( 0x200000*8, "textile", 0) /* 16x16x8bpp texture tiles */
	ROM_LOAD( "ar1cg0.12b",  0x200000*0x0, 0x200000,CRC(93f3a9d9) SHA1(7e94c81ad5ace98a2f0d00d101d464883d38c197) ) /* identical to "ar1cg0.8d" */
	ROM_LOAD( "ar1cg1.10d",  0x200000*0x1, 0x200000,CRC(39828c8b) SHA1(424aa67eb0b898c9cab8a4749893a9c5696ac430) ) /* identical to "ar1cg1.13b" */
	ROM_LOAD( "ar1cg2.12d",  0x200000*0x2, 0x200000,CRC(f7b058d1) SHA1(fffd0f01724a26dd47b1ecceecf4a139d5746f81) ) /* identical to "ar1cg2.14b" */
	ROM_LOAD( "ar1cg3.13d",  0x200000*0x3, 0x200000,CRC(c28a3d2a) SHA1(cdc44fdbc99274e860c834e42b4cfafb478d4d26) ) /* identical to "ar1cg3.16b" */
	ROM_LOAD( "ar1cg4.14d",  0x200000*0x4, 0x200000,CRC(abdb161f) SHA1(260bff9b0e94c1b2ea4b9d7fa170fbca212e85ee) ) /* identical to "ar1cg4.18b" */
	ROM_LOAD( "ar1cg5.16d",  0x200000*0x5, 0x200000,CRC(2381cfea) SHA1(1de4c8b94df233fd74771fa47843290a3d8df0c8) ) /* identical to "ar1cg5.19b" */
	ROM_LOAD( "ar1cg6.18a",  0x200000*0x6, 0x200000,CRC(ca0b6d23) SHA1(df969e0eeec557a95584b06995b0d55f2c6ec70a) ) /* identical to "ar1cg6.18d" */
	ROM_LOAD( "ar1cg7.15a",	 0x200000*0x7, 0x200000,CRC(ffb9f9f9) SHA1(2b8c75b580f77e887df7d50909a3a95cda570e20) ) /* identical to "ar1cg7.19d" */

	ROM_REGION16_LE( 0x280000, "textilemap", 0 ) /* texture tilemap */
	ROM_LOAD( "ar1ccrl.3d",	 0x000000, 0x200000,CRC(17387b2c) SHA1(dfd7cadaf97917347c0fa98f395364a543e49612) ) /* identical to "ar1ccrl.7b" */
	ROM_LOAD( "ar1ccrh.1d",	 0x200000, 0x080000,CRC(ee7a4803) SHA1(8383c9a8ef5ed94df13446ca5cefa5f9e518f175) ) /* identical to "pr1ccrh.5b" */

	ROM_REGION( 0x80000*12, "pointrom", 0 ) /* 3d model data */
	ROM_LOAD( "ar1ptrl0.18k", 0x80000*0x0, 0x80000,CRC(82405108) SHA1(0a40882a9bc8621c620bede404c78f6b1333f223) )
	ROM_LOAD( "ar1ptrl1.16k", 0x80000*0x1, 0x80000,CRC(8739b09c) SHA1(cd603c4dc2f9ffc4185f891eb83e4c383c564294) )
	ROM_LOAD( "ar1ptrl2.15k", 0x80000*0x2, 0x80000,CRC(bda693a9) SHA1(fe71dd3c63198737aa2d39527f0004e977e3be20) )
	ROM_LOAD( "ar1ptrl3.14k", 0x80000*0x3, 0x80000,CRC(82797405) SHA1(2f205fee2d33e183c80a906fb38900167c011240) )

	ROM_LOAD( "ar1ptrm0.18j", 0x80000*0x4, 0x80000,CRC(64bd6620) SHA1(2e33ff22208805ece304128be8887646fc890f6d) )
	ROM_LOAD( "ar1ptrm1.16j", 0x80000*0x5, 0x80000,CRC(2232f0a5) SHA1(3fccf6d4a0c4100cc85e3051024d659c4a1c769e) )
	ROM_LOAD( "ar1ptrm2.15j", 0x80000*0x6, 0x80000,CRC(8ee14e6f) SHA1(f6f1cbb748b109b365255378c18e710ba6270c1c) )
	ROM_LOAD( "ar1ptrm3.14j", 0x80000*0x7, 0x80000,CRC(1094a970) SHA1(d41b10f48e1ef312bcaf09f27fabc7252c30e648) )

	ROM_LOAD( "ar1ptru0.18f", 0x80000*0x8, 0x80000,CRC(26d88467) SHA1(d528f989fab4dd5ac1aec9b596a05fbadcc0587a) )
	ROM_LOAD( "ar1ptru1.16f", 0x80000*0x9, 0x80000,CRC(c5e2c208) SHA1(152fde0b95a5df8c781e4a83577cfbbc7672ae0d) )
	ROM_LOAD( "ar1ptru2.15f", 0x80000*0xa, 0x80000,CRC(1321ec59) SHA1(dbd3687a4c6b1aa0b18e336f99dabb9010d36639) )
	ROM_LOAD( "ar1ptru3.14f", 0x80000*0xb, 0x80000,CRC(139d7dc1) SHA1(6d25e6ad552a91a0c5fc03db7e1a801ccf9c9556) )

	ROM_REGION( 0x200000, "c352", 0 ) /* sound samples */
	ROM_LOAD( "ar1wavea.2l", 0, 0x200000, CRC(dbf64562) SHA1(454fd7d5b860f0e5557d8900393be95d6c992ad1) )
ROM_END

ROM_START( alpinerd )
	ROM_REGION( 0x400000, "maincpu", 0 ) /* main program */
	ROM_LOAD32_BYTE( "ar2ver-d.1", 0x00003, 0x100000, CRC(fa3380b9) SHA1(2a46988745bd2672f8082399a68ae0d0ab3d28f2) )
	ROM_LOAD32_BYTE( "ar2ver-d.2", 0x00002, 0x100000, CRC(76141352) SHA1(0f7230dd9cd6f1b83d499034affc7bc2c4385ab5) )
	ROM_LOAD32_BYTE( "ar2ver-d.3", 0x00001, 0x100000, CRC(9beffe6a) SHA1(d8efd1e3829d32bb06537d7cecb59f8df9b6d663) )
	ROM_LOAD32_BYTE( "ar2ver-d.4", 0x00000, 0x100000, CRC(1f3f1134) SHA1(0afa78444d1463d214f1afd7ec500af76d567489) )

	ROM_REGION( 0x10000*2, "master", 0 ) /* Master DSP */
	ROM_LOAD16_WORD( "c71.bin", 0,0x1000*2, CRC(47c623ab) SHA1(e363ac50f5556f83308d4cc191b455e9b62bcfc8) )

	ROM_REGION( 0x10000*2, "slave", 0 ) /* Slave DSP */
	ROM_LOAD16_WORD( "c71.bin", 0,0x1000*2, CRC(47c623ab) SHA1(e363ac50f5556f83308d4cc191b455e9b62bcfc8) )

	ROM_REGION( 0x080000, "mcu", ROMREGION_ERASE00 ) /* S22-BIOS ver1.30 */

	ROM_REGION16_LE( 0x080000, "user4", 0 ) /* MCU BIOS */
	ROM_LOAD( "ar1datab.8k", 0, 0x080000, CRC(c26306f8) SHA1(6d8d993c076d5ced523143a86bd0938b3794478d) )

	ROM_REGION( 0x200000*2, "sprite", 0 ) /* 32x32x8bpp sprite tiles */
	ROM_LOAD( "ar1scg0.12f", 0x200000*0, 0x200000,CRC(e7be830a) SHA1(60e2162eecd7401a0c26c525de2715cbfb10c1c5) ) /* identical to "ar1scg0.12l" */
	ROM_LOAD( "ar1scg1.10f", 0x200000*1, 0x200000,CRC(8f15a686) SHA1(bce2d4380c6c39aa402566ddb0f62bbe6d7bfa1d) ) /* identical to "ar1scg1.10l" */

	ROM_REGION( 0x200000*8, "textile", 0) /* 16x16x8bpp texture tiles */
	ROM_LOAD( "ar1cg0.12b",  0x200000*0x0, 0x200000,CRC(93f3a9d9) SHA1(7e94c81ad5ace98a2f0d00d101d464883d38c197) ) /* identical to "ar1cg0.8d" */
	ROM_LOAD( "ar1cg1.10d",  0x200000*0x1, 0x200000,CRC(39828c8b) SHA1(424aa67eb0b898c9cab8a4749893a9c5696ac430) ) /* identical to "ar1cg1.13b" */
	ROM_LOAD( "ar1cg2.12d",  0x200000*0x2, 0x200000,CRC(f7b058d1) SHA1(fffd0f01724a26dd47b1ecceecf4a139d5746f81) ) /* identical to "ar1cg2.14b" */
	ROM_LOAD( "ar1cg3.13d",  0x200000*0x3, 0x200000,CRC(c28a3d2a) SHA1(cdc44fdbc99274e860c834e42b4cfafb478d4d26) ) /* identical to "ar1cg3.16b" */
	ROM_LOAD( "ar1cg4.14d",  0x200000*0x4, 0x200000,CRC(abdb161f) SHA1(260bff9b0e94c1b2ea4b9d7fa170fbca212e85ee) ) /* identical to "ar1cg4.18b" */
	ROM_LOAD( "ar1cg5.16d",  0x200000*0x5, 0x200000,CRC(2381cfea) SHA1(1de4c8b94df233fd74771fa47843290a3d8df0c8) ) /* identical to "ar1cg5.19b" */
	ROM_LOAD( "ar1cg6.18a",  0x200000*0x6, 0x200000,CRC(ca0b6d23) SHA1(df969e0eeec557a95584b06995b0d55f2c6ec70a) ) /* identical to "ar1cg6.18d" */
	ROM_LOAD( "ar1cg7.15a",	 0x200000*0x7, 0x200000,CRC(ffb9f9f9) SHA1(2b8c75b580f77e887df7d50909a3a95cda570e20) ) /* identical to "ar1cg7.19d" */

	ROM_REGION16_LE( 0x280000, "textilemap", 0 ) /* texture tilemap */
	ROM_LOAD( "ar1ccrl.3d",	 0x000000, 0x200000,CRC(17387b2c) SHA1(dfd7cadaf97917347c0fa98f395364a543e49612) ) /* identical to "ar1ccrl.7b" */
	ROM_LOAD( "ar1ccrh.1d",	 0x200000, 0x080000,CRC(ee7a4803) SHA1(8383c9a8ef5ed94df13446ca5cefa5f9e518f175) ) /* identical to "pr1ccrh.5b" */

	ROM_REGION( 0x80000*12, "pointrom", 0 ) /* 3d model data */
	ROM_LOAD( "ar1ptrl0.18k", 0x80000*0x0, 0x80000,CRC(82405108) SHA1(0a40882a9bc8621c620bede404c78f6b1333f223) )
	ROM_LOAD( "ar1ptrl1.16k", 0x80000*0x1, 0x80000,CRC(8739b09c) SHA1(cd603c4dc2f9ffc4185f891eb83e4c383c564294) )
	ROM_LOAD( "ar1ptrl2.15k", 0x80000*0x2, 0x80000,CRC(bda693a9) SHA1(fe71dd3c63198737aa2d39527f0004e977e3be20) )
	ROM_LOAD( "ar1ptrl3.14k", 0x80000*0x3, 0x80000,CRC(82797405) SHA1(2f205fee2d33e183c80a906fb38900167c011240) )

	ROM_LOAD( "ar1ptrm0.18j", 0x80000*0x4, 0x80000,CRC(64bd6620) SHA1(2e33ff22208805ece304128be8887646fc890f6d) )
	ROM_LOAD( "ar1ptrm1.16j", 0x80000*0x5, 0x80000,CRC(2232f0a5) SHA1(3fccf6d4a0c4100cc85e3051024d659c4a1c769e) )
	ROM_LOAD( "ar1ptrm2.15j", 0x80000*0x6, 0x80000,CRC(8ee14e6f) SHA1(f6f1cbb748b109b365255378c18e710ba6270c1c) )
	ROM_LOAD( "ar1ptrm3.14j", 0x80000*0x7, 0x80000,CRC(1094a970) SHA1(d41b10f48e1ef312bcaf09f27fabc7252c30e648) )

	ROM_LOAD( "ar1ptru0.18f", 0x80000*0x8, 0x80000,CRC(26d88467) SHA1(d528f989fab4dd5ac1aec9b596a05fbadcc0587a) )
	ROM_LOAD( "ar1ptru1.16f", 0x80000*0x9, 0x80000,CRC(c5e2c208) SHA1(152fde0b95a5df8c781e4a83577cfbbc7672ae0d) )
	ROM_LOAD( "ar1ptru2.15f", 0x80000*0xa, 0x80000,CRC(1321ec59) SHA1(dbd3687a4c6b1aa0b18e336f99dabb9010d36639) )
	ROM_LOAD( "ar1ptru3.14f", 0x80000*0xb, 0x80000,CRC(139d7dc1) SHA1(6d25e6ad552a91a0c5fc03db7e1a801ccf9c9556) )

	ROM_REGION( 0x200000, "c352", 0 ) /* sound samples */
	ROM_LOAD( "ar1wavea.2l", 0, 0x200000, CRC(dbf64562) SHA1(454fd7d5b860f0e5557d8900393be95d6c992ad1) )
ROM_END

ROM_START( alpinr2b )
	ROM_REGION( 0x800000, "maincpu", 0 ) /* main program */
        ROM_LOAD32_BYTE( "ars2ver-b.2",  0x000003, 0x200000, CRC(ed977f83) SHA1(26c57cdfc15f799a999ee22f141e1c0cabfc91dc) )
        ROM_LOAD32_BYTE( "ars2ver-b.3",  0x000001, 0x200000, CRC(8e7a9983) SHA1(34c82e5f080efe04d6b77a77a8391cb48b69c1af) )
        ROM_LOAD32_BYTE( "ars2ver-b.4",  0x000002, 0x200000, CRC(610e49c2) SHA1(433c6d2216551bac31584306f748af1c912c3b07) )
        ROM_LOAD32_BYTE( "ars2ver-b.5",  0x000000, 0x200000, CRC(7f3517b0) SHA1(3e6ba1a51bf235f40f933aae1f00638b88bba522) )

	ROM_REGION( 0x10000*2, "master", 0 ) /* Master DSP */
	ROM_LOAD16_WORD( "c71.bin", 0,0x1000*2, CRC(47c623ab) SHA1(e363ac50f5556f83308d4cc191b455e9b62bcfc8) )

	ROM_REGION( 0x10000*2, "slave", 0 ) /* Slave DSP */
	ROM_LOAD16_WORD( "c71.bin", 0,0x1000*2, CRC(47c623ab) SHA1(e363ac50f5556f83308d4cc191b455e9b62bcfc8) )

	ROM_REGION( 0x080000, "mcu", ROMREGION_ERASE00 ) /* S22-BIOS ver1.30 */

	ROM_REGION16_LE( 0x080000, "user4", 0 ) /* MCU BIOS */
        ROM_LOAD( "ars2data.8k",  0x000000, 0x080000, CRC(29b36dcb) SHA1(70fde130c11789c822829493a70ecefb077c0c15) )

	ROM_REGION( 0x200000*2, "sprite", 0 ) /* 32x32x8bpp sprite tiles */
        ROM_LOAD( "ars1scg0.12f", 0x000000, 0x200000, CRC(bc49ed86) SHA1(289b39f2cb21c723dbe4ddd64ee4b2c5fa65c368) )

	ROM_REGION( 0xc00000, "textile", 0) /* 16x16x8bpp texture tiles */
        ROM_LOAD( "ars1cg0.8d",   0x000000, 0x200000, CRC(74f4515c) SHA1(1e3a96281d543213d10c962b4d387c414d76e0c3) )
        ROM_LOAD( "ars1cg1.10d",  0x200000, 0x200000, CRC(329a95c1) SHA1(2cad7fd9e5ca7c64729ca2548ef4f873a0b8de64) )
        ROM_LOAD( "ars1cg2.12d",  0x400000, 0x200000, CRC(5648345a) SHA1(0dc7aedba65b7d97687a9e38a63597f16cee6179) )
        ROM_LOAD( "ars1cg3.13d",  0x600000, 0x200000, CRC(a752f205) SHA1(373b5a69e4488bc30763568ceae512ab7039f5f8) )
        ROM_LOAD( "ars1cg4.14d",  0x800000, 0x200000, CRC(54bf35b6) SHA1(aec43b66e7597ad7d113ae785417bf26164c1bca) )
        ROM_LOAD( "ars1cg5.16d",  0xa00000, 0x200000, CRC(e24a19a2) SHA1(34c1b51eea954ae3000602e550eb1cef0a10e651) )

	ROM_REGION16_LE( 0x280000, "textilemap", 0 ) /* texture tilemap */
        ROM_LOAD( "ars1ccrl.3d",  0x000000, 0x200000, CRC(fc8c9161) SHA1(ad0fcfae27e02d68a6f8d1d03c514dc2f12d9ee8) )
        ROM_LOAD( "ars1ccrh.1d",  0x200000, 0x080000, CRC(a17660bb) SHA1(bae2c3f20772c6cea99f271ee3f39b1f999038c6) )

	ROM_REGION( 0x80000*12, "pointrom", 0 ) /* 3d model data */
        ROM_LOAD( "ars1ptrl0.18k", 0x000000, 0x080000, CRC(f04e0e61) SHA1(d58a1d3ca1d0922e134db21a04feaee7dc97b020) )
        ROM_LOAD( "ars1ptrl1.16k", 0x080000, 0x080000, CRC(8bdb4970) SHA1(a504cd4beca4bedd1a7a228c83dd6b844ca3a1e0) )
        ROM_LOAD( "ars1ptrl2.15k", 0x100000, 0x080000, CRC(ec993a4f) SHA1(883f64e3e8d951415e9cef589c354eba9406c0aa) )
        ROM_LOAD( "ars1ptrl3.14k", 0x180000, 0x080000, CRC(4d453f3c) SHA1(7a82e5d8f974d9e56d0031b35e73647fe6aeec2e) )

        ROM_LOAD( "ars1ptrm0.18j", 0x200000, 0x080000, CRC(d1bdc524) SHA1(b898bb38de397551ada4da4677dd733bf8fa5010) )
        ROM_LOAD( "ars1ptrm1.16j", 0x280000, 0x080000, CRC(86b81c81) SHA1(45096abf46794f06a4b647f5e4222798d8467632) )
        ROM_LOAD( "ars1ptrm2.15j", 0x300000, 0x080000, CRC(24116b83) SHA1(41c6a880abce7b543c409fda767682b2537b0d99) )
        ROM_LOAD( "ars1ptrm3.14j", 0x380000, 0x080000, CRC(772bede3) SHA1(f9565b7a40f0bbf11081d619fe5a46feafce2e56) )

        ROM_LOAD( "ars1ptru0.18f", 0x400000, 0x080000, CRC(a4cf197a) SHA1(ee78cc259e87395df75179bbe5b6e521e762b582) )
        ROM_LOAD( "ars1ptru1.16f", 0x480000, 0x080000, CRC(1deb1fc0) SHA1(bfd1dfcaccf5a0f851b6757995fa7195452a3965) )
        ROM_LOAD( "ars1ptru2.15f", 0x500000, 0x080000, CRC(bcfad0ba) SHA1(bee7f2f9ecd2b289c6706e19fa86863913b286b5) )
        ROM_LOAD( "ars1ptru3.14f", 0x580000, 0x080000, CRC(73ce6958) SHA1(918b0fb0fca33dbe3be3ac679b8b28f58213f75b) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* sound samples */
        ROM_LOAD( "ars1wavea.2l", 0x000000, 0x400000, CRC(f8d107e9) SHA1(5c418691f0b35403553f21f5570eda8bbb66890f) )
        ROM_LOAD( "ars2waveb.1l", 0x800000, 0x400000, CRC(deab4ad1) SHA1(580ad88d516280baaf6cc92b2e07cdc0cfc486f3) )
ROM_END

ROM_START( alpinr2a )
	ROM_REGION( 0x800000, "maincpu", 0 ) /* main program */
        ROM_LOAD32_BYTE( "ars2ver-a.2",  0x000003, 0x200000, CRC(b07b15a4) SHA1(ea3b2d7b4ef4ccf3aafeef7e7eac92e8d446f4e7) )
        ROM_LOAD32_BYTE( "ars2ver-a.3",  0x000002, 0x200000, CRC(90a92e40) SHA1(bf8083256e56e7e33e61b4cdaf9fd03dabfb36ba) )
        ROM_LOAD32_BYTE( "ars2ver-a.4",  0x000001, 0x200000, CRC(9e9d771d) SHA1(6fb983e3f4f8233544667b1bbf87864e4fb8698c) )
        ROM_LOAD32_BYTE( "ars2ver-a.5",  0x000000, 0x200000, CRC(e93c7771) SHA1(305f35488a55be1b845702df972bba8334c0726c) )

	ROM_REGION( 0x10000*2, "master", 0 ) /* Master DSP */
	ROM_LOAD16_WORD( "c71.bin", 0,0x1000*2, CRC(47c623ab) SHA1(e363ac50f5556f83308d4cc191b455e9b62bcfc8) )

	ROM_REGION( 0x10000*2, "slave", 0 ) /* Slave DSP */
	ROM_LOAD16_WORD( "c71.bin", 0,0x1000*2, CRC(47c623ab) SHA1(e363ac50f5556f83308d4cc191b455e9b62bcfc8) )

	ROM_REGION( 0x080000, "mcu", ROMREGION_ERASE00 ) /* S22-BIOS ver1.30 */

	ROM_REGION16_LE( 0x080000, "user4", 0 ) /* MCU BIOS */
        ROM_LOAD( "ars2data.8k",  0x000000, 0x080000, CRC(29b36dcb) SHA1(70fde130c11789c822829493a70ecefb077c0c15) )

	ROM_REGION( 0x200000*2, "sprite", 0 ) /* 32x32x8bpp sprite tiles */
        ROM_LOAD( "ars1scg0.12f", 0x000000, 0x200000, CRC(bc49ed86) SHA1(289b39f2cb21c723dbe4ddd64ee4b2c5fa65c368) )

	ROM_REGION( 0xc00000, "textile", 0) /* 16x16x8bpp texture tiles */
        ROM_LOAD( "ars1cg0.8d",   0x000000, 0x200000, CRC(74f4515c) SHA1(1e3a96281d543213d10c962b4d387c414d76e0c3) )
        ROM_LOAD( "ars1cg1.10d",  0x200000, 0x200000, CRC(329a95c1) SHA1(2cad7fd9e5ca7c64729ca2548ef4f873a0b8de64) )
        ROM_LOAD( "ars1cg2.12d",  0x400000, 0x200000, CRC(5648345a) SHA1(0dc7aedba65b7d97687a9e38a63597f16cee6179) )
        ROM_LOAD( "ars1cg3.13d",  0x600000, 0x200000, CRC(a752f205) SHA1(373b5a69e4488bc30763568ceae512ab7039f5f8) )
        ROM_LOAD( "ars1cg4.14d",  0x800000, 0x200000, CRC(54bf35b6) SHA1(aec43b66e7597ad7d113ae785417bf26164c1bca) )
        ROM_LOAD( "ars1cg5.16d",  0xa00000, 0x200000, CRC(e24a19a2) SHA1(34c1b51eea954ae3000602e550eb1cef0a10e651) )

	ROM_REGION16_LE( 0x280000, "textilemap", 0 ) /* texture tilemap */
        ROM_LOAD( "ars1ccrl.3d",  0x000000, 0x200000, CRC(fc8c9161) SHA1(ad0fcfae27e02d68a6f8d1d03c514dc2f12d9ee8) )
        ROM_LOAD( "ars1ccrh.1d",  0x200000, 0x080000, CRC(a17660bb) SHA1(bae2c3f20772c6cea99f271ee3f39b1f999038c6) )

	ROM_REGION( 0x80000*12, "pointrom", 0 ) /* 3d model data */
        ROM_LOAD( "ars1ptrl0.18k", 0x000000, 0x080000, CRC(f04e0e61) SHA1(d58a1d3ca1d0922e134db21a04feaee7dc97b020) )
        ROM_LOAD( "ars1ptrl1.16k", 0x080000, 0x080000, CRC(8bdb4970) SHA1(a504cd4beca4bedd1a7a228c83dd6b844ca3a1e0) )
        ROM_LOAD( "ars1ptrl2.15k", 0x100000, 0x080000, CRC(ec993a4f) SHA1(883f64e3e8d951415e9cef589c354eba9406c0aa) )
        ROM_LOAD( "ars1ptrl3.14k", 0x180000, 0x080000, CRC(4d453f3c) SHA1(7a82e5d8f974d9e56d0031b35e73647fe6aeec2e) )

        ROM_LOAD( "ars1ptrm0.18j", 0x200000, 0x080000, CRC(d1bdc524) SHA1(b898bb38de397551ada4da4677dd733bf8fa5010) )
        ROM_LOAD( "ars1ptrm1.16j", 0x280000, 0x080000, CRC(86b81c81) SHA1(45096abf46794f06a4b647f5e4222798d8467632) )
        ROM_LOAD( "ars1ptrm2.15j", 0x300000, 0x080000, CRC(24116b83) SHA1(41c6a880abce7b543c409fda767682b2537b0d99) )
        ROM_LOAD( "ars1ptrm3.14j", 0x380000, 0x080000, CRC(772bede3) SHA1(f9565b7a40f0bbf11081d619fe5a46feafce2e56) )

        ROM_LOAD( "ars1ptru0.18f", 0x400000, 0x080000, CRC(a4cf197a) SHA1(ee78cc259e87395df75179bbe5b6e521e762b582) )
        ROM_LOAD( "ars1ptru1.16f", 0x480000, 0x080000, CRC(1deb1fc0) SHA1(bfd1dfcaccf5a0f851b6757995fa7195452a3965) )
        ROM_LOAD( "ars1ptru2.15f", 0x500000, 0x080000, CRC(bcfad0ba) SHA1(bee7f2f9ecd2b289c6706e19fa86863913b286b5) )
        ROM_LOAD( "ars1ptru3.14f", 0x580000, 0x080000, CRC(73ce6958) SHA1(918b0fb0fca33dbe3be3ac679b8b28f58213f75b) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* sound samples */
        ROM_LOAD( "ars1wavea.2l", 0x000000, 0x400000, CRC(f8d107e9) SHA1(5c418691f0b35403553f21f5570eda8bbb66890f) )
        ROM_LOAD( "ars2waveb.1l", 0x800000, 0x400000, CRC(deab4ad1) SHA1(580ad88d516280baaf6cc92b2e07cdc0cfc486f3) )
ROM_END

ROM_START( alpinesa )
	ROM_REGION( 0x800000, "maincpu", 0 ) /* main program */
        ROM_LOAD32_BYTE( "af2ver-a_ll.ic2", 0x000003, 0x200000, CRC(e776159d) SHA1(5110364afb7ec606074d58a1d216d7d687b9df62) )
        ROM_LOAD32_BYTE( "af2ver-a_lm.ic3", 0x000002, 0x200000, CRC(c5333d38) SHA1(9486cead964f95f8e56dac2f88486f3b98561aa6) )
        ROM_LOAD32_BYTE( "af2ver-a_um.ic4", 0x000001, 0x200000, CRC(5977fc6e) SHA1(19b8041789f8987934fa461972976a3570b1b87b) )
        ROM_LOAD32_BYTE( "af2ver-a_uu.ic5", 0x000000, 0x200000, CRC(54ee33a1) SHA1(0eaa8707ab13a0a66551f61a08986c98f5c9e446) )

	ROM_REGION( 0x10000*2, "master", 0 ) /* Master DSP */
	ROM_LOAD16_WORD( "c71.bin", 0,0x1000*2, CRC(47c623ab) SHA1(e363ac50f5556f83308d4cc191b455e9b62bcfc8) )

	ROM_REGION( 0x10000*2, "slave", 0 ) /* Slave DSP */
	ROM_LOAD16_WORD( "c71.bin", 0,0x1000*2, CRC(47c623ab) SHA1(e363ac50f5556f83308d4cc191b455e9b62bcfc8) )

	ROM_REGION( 0x080000, "mcu", ROMREGION_ERASE00 ) /* S22-BIOS ver1.41 */

	ROM_REGION16_LE( 0x080000, "user4", 0 ) /* MCU BIOS */
        ROM_LOAD( "af1data.8k",   0x000000, 0x080000, CRC(ef13ebe8) SHA1(5d3f697994d4b5b19ee7fea1e2aef8e39449b68e) )

   ROM_REGION( 0x200000*2, "sprite", 0 ) /* 32x32x8bpp sprite tiles */
   ROM_LOAD( "af1scg0b.12f", 0x000000, 0x200000, CRC(46a6222a) SHA1(5322ef60690625b9b8dbe1cfe0c49dcd9c8b1a4c) )

	ROM_REGION( 0xa00000, "textile", 0) /* 16x16x8bpp texture tiles */
        ROM_LOAD( "af1cg0.8d",    0x000000, 0x200000, CRC(7423f3ff) SHA1(6a2fd44823ef46111deb57d328b1b75cc355d413) )
        ROM_LOAD( "af1cg1.10d",   0x200000, 0x200000, CRC(ea76689a) SHA1(73dd3af737a3e9903abe5ed9c9ae7eded51d8350) )
        ROM_LOAD( "af1cg2.12d",   0x400000, 0x200000, CRC(2a38943a) SHA1(15d737996f49bf6374ef6191bbfbe0298d398378) )
        ROM_LOAD( "af1cg3.13d",   0x600000, 0x200000, CRC(7f5a3e0f) SHA1(241f9995323b28df23d20a75e1f43ce6e05434cd) )
        ROM_LOAD( "af1cg4.14d",   0x800000, 0x200000, CRC(a5ee13e2) SHA1(48fd3c912690f21cbbc2a39bed0a82be41a0d011) )

	ROM_REGION16_LE( 0x280000, "textilemap", 0 ) /* texture tilemap */
        ROM_LOAD( "af1ccrl.3d",   0x000000, 0x200000, CRC(6c054698) SHA1(8537607646b183883c5aa4060fb0af640da4af87) )
        ROM_LOAD( "af1ccrh.1d",   0x200000, 0x080000, CRC(95a02a27) SHA1(32ee87b76ae9fcec6d825e3cf4d5cbb97db39544) )

	ROM_REGION( 0x80000*6, "pointrom", 0 ) /* 3d model data */
        ROM_LOAD( "af1ptrl0.18k", 0x80000*0, 0x80000, CRC(31ce46d3) SHA1(568fb9ee9ac14e613a4fd7668cb38315c10be62b) )
        ROM_LOAD( "af1ptrl1.16k", 0x80000*1, 0x80000, CRC(e869bf00) SHA1(b3c3026891ae3958d1774c905e97c3b57a414ea7) )
        ROM_LOAD( "af1ptrm0.18j", 0x80000*2, 0x80000, CRC(ef7f4d8a) SHA1(02f77c68004b7dccc99b61126e7d07960eb15028) )
        ROM_LOAD( "af1ptrm1.16j", 0x80000*3, 0x80000, CRC(7dd01d52) SHA1(adc1087435d31ed6163ad046466955f01517450f) )
        ROM_LOAD( "af1ptru0.18f", 0x80000*4, 0x80000, CRC(177f1591) SHA1(3969e780e5603eca0a65f65c1ad14d1cef918b39) )
        ROM_LOAD( "af1ptru1.16f", 0x80000*5, 0x80000, CRC(7521d18e) SHA1(dc03ef369db16f59c138ff4e22260d1c04782d1f) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* sound samples */
        ROM_LOAD( "af1wavea.2l",  0x000000, 0x400000, CRC(28cca494) SHA1(4ff87ab85fd17bf8dbee5b03d99cc5c31dd6349a) )
ROM_END

ROM_START( cybrcomm )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* main program */
	ROM_LOAD32_BYTE( "cy1prgll.4d", 0x00003, 0x80000, CRC(b3eab156) SHA1(2a5c4e0360c3b9500687a4d70f7110a0c30da2a5) )
	ROM_LOAD32_BYTE( "cy1prglm.2d", 0x00002, 0x80000, CRC(884a5b0e) SHA1(0e27ae366b8a2695fe112b4740c8c9013bb97e26) )
	ROM_LOAD32_BYTE( "cy1prgum.8d", 0x00001, 0x80000, CRC(c9c4a921) SHA1(76a52461165a8bd8d984a34063fbeb4cb73624af) )
	ROM_LOAD32_BYTE( "cy1prguu.6d", 0x00000, 0x80000, CRC(5f22975b) SHA1(a1a5cb66358d64a3c564b912f2eeafa182786b1e) )

	ROM_REGION( 0x10000*2, "master", 0 ) /* Master DSP */
	ROM_LOAD16_WORD( "c71.bin", 0,0x1000*2, CRC(47c623ab) SHA1(e363ac50f5556f83308d4cc191b455e9b62bcfc8) )

	ROM_REGION( 0x10000*2, "slave", 0 ) /* Slave DSP */
	ROM_LOAD16_WORD( "c71.bin", 0,0x1000*2, CRC(47c623ab) SHA1(e363ac50f5556f83308d4cc191b455e9b62bcfc8) )

	ROM_REGION( 0x080000, "mcu", ROMREGION_ERASE00 ) /* BIOS */
	ROM_LOAD( "c74.bin", 0x0000, 0x4000, CRC(a3dce360) SHA1(8f3248b1890abb2e649927240ae46f73bb171e3b) )

	ROM_REGION16_LE( 0x100000, "user4", 0 ) /* MCU BIOS */
	ROM_LOAD( "cy1data.6r", 0, 0x020000, CRC(10d0005b) SHA1(10508eeaf74d24a611b44cd3bb12417ceb78904f) )

	ROM_REGION( 0x200000*8, "textile", 0) /* 16x16x8bpp texture tiles */
	ROM_LOAD( "cyc1cg0.1a", 0x200000*0x4, 0x200000,CRC(e839b9bd) SHA1(fee43d37dcca7f1fb952a6bfb886b7ee30b7d75c) ) /* cyc1cg0.6a */
	ROM_LOAD( "cyc1cg1.2a", 0x200000*0x5, 0x200000,CRC(7d13993f) SHA1(96ac82bcc63afe395bae73f005eb66dad7742d48) ) /* cyc1cg1.7a */
	ROM_LOAD( "cyc1cg2.3a", 0x200000*0x6, 0x200000,CRC(7c464566) SHA1(69817ac3a7c6e43b960e8a904962b58b23417163) ) /* cyc1cg2.8a */
	ROM_LOAD( "cyc1cg3.5a", 0x200000*0x7, 0x200000,CRC(2222e16f) SHA1(562bcd4d43b1543303d8fd66d9f0d9a8e3702492) ) /* cyc1cg3.9a */

	ROM_REGION16_LE( 0x280000, "textilemap", 0 ) /* texture tilemap */

	//cyc1ccrl.1c FIXED BITS (xxxxxxxx11xxxxxx)
	ROM_LOAD( "cyc1ccrl.1c",	 0x000000, 0x100000,CRC(1a0dc5f0) SHA1(bf0093d9cbdcb45a82705e966c48a1f408fa344e) ) /* cyc1ccrl.8c */

	//cyc1ccrh.2c 1xxxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD( "cyc1ccrh.2c",	 0x200000, 0x080000,CRC(8c4090b8) SHA1(456d548a48833e840c5d39d47b2dcca03f8d6321) ) /* cyc1ccrh.7c */

	ROM_REGION( 0x80000*9, "pointrom", 0 ) /* 3d model data */
	ROM_LOAD( "cyc1ptl0.5b", 0x80000*0x0, 0x80000,CRC(d91de03d) SHA1(05819d285f6111867c41337bda9c4b9ad5394b6b) )
	ROM_LOAD( "cyc1ptl1.4b", 0x80000*0x1, 0x80000,CRC(e5b98021) SHA1(7416cbf74da969f822e0363ced7a25b967277e28) )
	ROM_LOAD( "cyc1ptl2.3b", 0x80000*0x2, 0x80000,CRC(7ba786c6) SHA1(1a5319dec495453bab9d70ae773a807f0036b355) )
	ROM_LOAD( "cyc1ptm0.5c", 0x80000*0x3, 0x80000,CRC(d454b5c6) SHA1(95ae6f0455e9fd7dff066e74cd4343c94d1bc212) )
	ROM_LOAD( "cyc1ptm1.4c", 0x80000*0x4, 0x80000,CRC(74fdf8cc) SHA1(f2627f400e247b6d4c4157eaf0ec69d57212e566) )
	ROM_LOAD( "cyc1ptm2.3c", 0x80000*0x5, 0x80000,CRC(b9c99a45) SHA1(c86cf594b416776eaf9a32c3cb9d34acc79777e9) )
	ROM_LOAD( "cyc1ptu0.5d", 0x80000*0x6, 0x80000,CRC(4d40897f) SHA1(ffe2a0ab66443553c83512f9a1be94b2e385cf2f) )
	ROM_LOAD( "cyc1ptu1.4d", 0x80000*0x7, 0x80000,CRC(3bdaeeeb) SHA1(826f97e2165af8569cfec03874b16030a9486559) )
	ROM_LOAD( "cyc1ptu2.3d", 0x80000*0x8, 0x80000,CRC(a0e73674) SHA1(1e22142a564e664031c12b250664fc82e3b3d43b) )

	ROM_REGION( 0x400000, "c352", 0 ) /* sound samples */
	ROM_LOAD( "cy1wav0.10r", 0x000000, 0x100000, CRC(c6f366a2) SHA1(795dbee09df159d3501c748fb3de16cca81742d6) )
	ROM_LOAD( "cy1wav1.10p", 0x200000, 0x100000, CRC(f30b5e37) SHA1(9f5a94d82741ef9688c6e415ebb9009c906737c9) )
	ROM_LOAD( "cy1wav2.10n", 0x100000, 0x100000, CRC(b98c1ca6) SHA1(4b66aa05f82be5ef3315acc30031872698ff4391) )
	ROM_LOAD( "cy1wav3.10l", 0x300000, 0x100000, CRC(43dbac19) SHA1(83fd4ae4e7ec264fc217ed18caf59bf438af0c3d) )

	ROM_REGION( 0x300, "user1", 0 )
	ROM_LOAD( "rr1gam.2d",   0x0000, 0x0100, CRC(b2161bce) SHA1(d2681cc0cf8e68df0d942d392b4eb4458c4bb356) )
	ROM_LOAD( "rr1gam.3d",   0x0100, 0x0100, CRC(b2161bce) SHA1(d2681cc0cf8e68df0d942d392b4eb4458c4bb356) )
	ROM_LOAD( "rr1gam.4d",   0x0200, 0x0100, CRC(b2161bce) SHA1(d2681cc0cf8e68df0d942d392b4eb4458c4bb356) )

	ROM_REGION( 0x2000, "nvram", 0 )
//  ROM_LOAD( "cy1eeprm.9e", 0x0000, 0x2000, CRC(4e1d294b) SHA1(954ce04dcdba65214f5d0690ca59264f9090a1d6) ) /* EPROM (dumped?) */
	ROM_LOAD( "cybrcomm.nv", 0x0000, 0x2000, CRC(8432c066) SHA1(99d4bfda3f8aec288dbeaf291bce85fe9009a1de) ) /* EPROM (useful defaults) */
ROM_END

ROM_START( cybrcycc )
	ROM_REGION( 0x400000, "maincpu", 0 ) /* main program */
	ROM_LOAD32_BYTE( "cb2ver-c.1", 0x00003, 0x100000, CRC(a8e07a14) SHA1(9bef7068c9bf792960df922ea79e4565d7680433) )
	ROM_LOAD32_BYTE( "cb2ver-c.2", 0x00002, 0x100000, CRC(054c504f) SHA1(9bde803ff09be0402f9b0388e55407362a2508e3) )
	ROM_LOAD32_BYTE( "cb2ver-c.3", 0x00001, 0x100000, CRC(47e6306c) SHA1(39d6fc2c3cb9b4c9d3569cedb79b916a90537115) )
	ROM_LOAD32_BYTE( "cb2ver-c.4", 0x00000, 0x100000, CRC(398426e4) SHA1(f20cd4892420e7b978baa51c9129b362422a3895) )

	ROM_REGION( 0x10000*2, "master", 0 ) /* Master DSP */
	ROM_LOAD16_WORD( "c71.bin", 0,0x1000*2, CRC(47c623ab) SHA1(e363ac50f5556f83308d4cc191b455e9b62bcfc8) )

	ROM_REGION( 0x10000*2, "slave", 0 ) /* Slave DSP */
	ROM_LOAD16_WORD( "c71.bin", 0,0x1000*2, CRC(47c623ab) SHA1(e363ac50f5556f83308d4cc191b455e9b62bcfc8) )

	ROM_REGION( 0x080000, "mcu", ROMREGION_ERASE00 ) /* S22-BIOS ver1.30 */

	ROM_REGION16_LE( 0x080000, "user4", 0 ) /* MCU BIOS */
	ROM_LOAD( "cb1datab.8k", 0, 0x080000, CRC(e2404221) SHA1(b88810dd45aee8a5475c30806cdfded25fa14e0e) )

   ROM_REGION( 0x200000, "sprite", 0 ) /* 32x32x8bpp sprite tiles */
	ROM_LOAD( "cb1scg0.12f", 0x200000*0, 0x200000,CRC(7aaca90d) SHA1(9808819db5d86d555a03bb20a2fbedf060d04f0e) ) /* identical to "cb1scg0.12l" */

	ROM_REGION( 0x200000*7, "textile", 0) /* 16x16x8bpp texture tiles */
	ROM_LOAD( "cb1cg0.12b",  0x200000*0x0, 0x200000,CRC(762a47a0) SHA1(8a49c700dca7afec5d8d6a38fedcd3ad4b0e6713) ) /* identical to "cb1cg0.8d" */
	ROM_LOAD( "cb1cg1.10d",  0x200000*0x1, 0x200000,CRC(df92c3e6) SHA1(302d7ee7e073a45e7baa948543bd30251f903a6d) ) /* identical to "cb1cg1.13b" */
	ROM_LOAD( "cb1cg2.12d",  0x200000*0x2, 0x200000,CRC(07bc508e) SHA1(7675694d10b50e57bb10b350559bd321df75d1ea) ) /* identical to "cb1cg2.14b" */
	ROM_LOAD( "cb1cg3.13d",  0x200000*0x3, 0x200000,CRC(50c86dea) SHA1(7837a1d2bd3ade470f7fbc732513dd598badd219) ) /* identical to "cb1cg3.16b" */
	ROM_LOAD( "cb1cg4.14d",  0x200000*0x4, 0x200000,CRC(e93b8894) SHA1(4d28b557b7ed2667e6af9f970f3e99cda785b940) ) /* identical to "cb1cg4.18b" */
	ROM_LOAD( "cb1cg5.16d",  0x200000*0x5, 0x200000,CRC(9ee610a1) SHA1(ebc7892b6a66461ca6b6b912a264da1594340b2d) ) /* identical to "cb1cg5.19b" */
	ROM_LOAD( "cb1cg6.18a",  0x200000*0x6, 0x200000,CRC(ddc3b5cc) SHA1(34edffee9eb6fbf4a00fce0da34d9354b1a1155f) ) /* identical to "cb1cg6.18d" */

	ROM_REGION16_LE( 0x280000, "textilemap", 0 ) /* texture tilemap */
	ROM_LOAD( "cb1ccrl.3d",	 0x000000, 0x200000,CRC(2f171c48) SHA1(52b76213e37379b4a5cea7de40cf5396dc2998d8) ) /* identical to "cb1ccrl.7b" */
	ROM_LOAD( "cb1ccrh.1d",	 0x200000, 0x080000,CRC(86124b93) SHA1(f2cfd726313cbeff162d402a15de2360377630e7) ) /* identical to "cb1ccrh.5b" */

	ROM_REGION( 0x80000*12, "pointrom", 0 ) /* 3d model data */
	ROM_LOAD( "cb1ptrl0.18k", 0x80000*0x0, 0x80000,CRC(f1393a03) SHA1(c9e808601eef5839e6bff630e5f83380e073c5c0) )
	ROM_LOAD( "cb1ptrl1.16k", 0x80000*0x1, 0x80000,CRC(2ad51de7) SHA1(efd102b960ca10cda70da84661acf61e4bbb9f00) )
	ROM_LOAD( "cb1ptrl2.15k", 0x80000*0x2, 0x80000,CRC(78f77c0d) SHA1(5183a8909c2ac0a3d80e707393bcbb4441d79a3c) )
	ROM_LOAD( "cb1ptrl3.14k", 0x80000*0x3, 0x80000,CRC(804bfb4a) SHA1(74b3fc3931265398e23605d3da7ca84a002da632) )
	ROM_LOAD( "cb1ptrm0.18j", 0x80000*0x4, 0x80000,CRC(f4eece49) SHA1(3f34d1ae5986f0d340563ab0fb637bfdacb8712c) )
	ROM_LOAD( "cb1ptrm1.16j", 0x80000*0x5, 0x80000,CRC(5f3cbd7d) SHA1(d00d0a96b71d6a3b98907c4ba7c702e549dd0adb) )
	ROM_LOAD( "cb1ptrm2.15j", 0x80000*0x6, 0x80000,CRC(02c7e4af) SHA1(6a541a28163b1026a824f6f8aed05d0eb0c8ae93) )
	ROM_LOAD( "cb1ptrm3.14j", 0x80000*0x7, 0x80000,CRC(ace3123b) SHA1(2b590ed967572d77b3cc6b37e341a5bdc55c762f) )
	ROM_LOAD( "cb1ptru0.18f", 0x80000*0x8, 0x80000,CRC(58d35341) SHA1(a5fe00bdcf39521f0465743664ff0dd78be5d6e8) )
	ROM_LOAD( "cb1ptru1.16f", 0x80000*0x9, 0x80000,CRC(f4d005b0) SHA1(0862ed1dd0818bfb765d97f1f9d996c321b0ec83) )
	ROM_LOAD( "cb1ptru2.15f", 0x80000*0xa, 0x80000,CRC(68ffcd50) SHA1(5ca5f71b6b079fde14d76c869d211a815bffae68) )
	ROM_LOAD( "cb1ptru3.14f", 0x80000*0xb, 0x80000,CRC(d89c1c2b) SHA1(9c25df696b2d120ce33d7774381460297740007a) )

	ROM_REGION( 0xa00000, "c352", 0 ) /* sound samples */
	ROM_LOAD( "cb1wavea.2l", 0x000000, 0x400000, CRC(b79a624d) SHA1(c0ee358a183ba6d0835731dbdd191b64718fde6e) )
	ROM_LOAD( "cb1waveb.1l", 0x800000, 0x200000, CRC(33bf08f6) SHA1(bf9d68b26a8158ea1abfe8428b7454cac25242c5) )
ROM_END

ROM_START( propcycl )
	ROM_REGION( 0x400000, "maincpu", 0 ) /* main program */
	ROM_LOAD32_BYTE( "pr2ver-a.1", 0x00003, 0x100000, CRC(3f58594c) SHA1(5fdd8c61b47b51088a201799ce0c2f08c32ef852) )
	ROM_LOAD32_BYTE( "pr2ver-a.2", 0x00002, 0x100000, CRC(c0da354a) SHA1(f27a71a62385b842404fcd8ed6513158e3639b8f) )
	ROM_LOAD32_BYTE( "pr2ver-a.3", 0x00001, 0x100000, CRC(74bf4b74) SHA1(02713aa07238cc9e30163ae24d12c034aa972ff3) )
	ROM_LOAD32_BYTE( "pr2ver-a.4", 0x00000, 0x100000, CRC(cf4d5638) SHA1(2ddd00d6ec3b85c234820507650d201e176c94a2) )

	ROM_REGION( 0x10000*2, "master", 0 ) /* Master DSP */
	ROM_LOAD16_WORD( "c71.bin", 0,0x1000*2, CRC(47c623ab) SHA1(e363ac50f5556f83308d4cc191b455e9b62bcfc8) )

	ROM_REGION( 0x10000*2, "slave", 0 ) /* Slave DSP */
	ROM_LOAD16_WORD( "c71.bin", 0,0x1000*2, CRC(47c623ab) SHA1(e363ac50f5556f83308d4cc191b455e9b62bcfc8) )

	ROM_REGION( 0x080000, "mcu", ROMREGION_ERASE00 ) /* SS22-BIOS ver1.41 */

	ROM_REGION16_LE( 0x080000, "user4", 0 ) /* MCU BIOS */
	ROM_LOAD( "pr1data.8k", 0, 0x080000, CRC(2e5767a4) SHA1(390bf05c90044d841fe2dd4a427177fa1570b9a6) )

	ROM_REGION( 0x200000*2, "sprite", 0 ) /* 32x32x8bpp sprite tiles */
	ROM_LOAD( "pr1scg0.12f", 0x200000*0, 0x200000,CRC(2d09a869) SHA1(ce8beabaac255e1de29d944c9866022bad713519) ) /* identical to "pr1scg0.12l" */
	ROM_LOAD( "pr1scg1.10f", 0x200000*1, 0x200000,CRC(7433c5bd) SHA1(a8fd4e73de66e3d443c0cb5b5beef8f467014815) ) /* identical to "pr1scg1.10l" */

	ROM_REGION( 0x200000*8, "textile", 0) /* 16x16x8bpp texture tiles */
	ROM_LOAD( "pr1cg0.12b",  0x200000*0x0, 0x200000,CRC(0a041238) SHA1(da5688970432f7fe39337ee9fb46ca25a53fdb11) ) /* identical to "pr1cg0.8d" */
	ROM_LOAD( "pr1cg1.10d",  0x200000*0x1, 0x200000,CRC(7d09e6a7) SHA1(892317ee0bd796fa5c70d64912ef2e696792a2d4) ) /* identical to "pr1cg1.13b" */
	ROM_LOAD( "pr1cg2.12d",  0x200000*0x2, 0x200000,CRC(659f006e) SHA1(23362a922cb1100950181fac4858b953d8fc0794) ) /* identical to "pr1cg2.14b" */
	ROM_LOAD( "pr1cg3.13d",  0x200000*0x3, 0x200000,CRC(d30bffa3) SHA1(2f05227d91d257db9fa8cae114974de602d98729) ) /* identical to "pr1cg3.16b" */
	ROM_LOAD( "pr1cg4.14d",  0x200000*0x4, 0x200000,CRC(f4636cc9) SHA1(4e01a476e418e5790878572e83a8a11536ce30ae) ) /* identical to "pr1cg4.18b" */
	ROM_LOAD( "pr1cg5.16d",  0x200000*0x5, 0x200000,CRC(97d333de) SHA1(e8f8383f49aae834dd8b57231b25899703cef966) ) /* identical to "pr1cg5.19b" */
	ROM_LOAD( "pr1cg6.18a",  0x200000*0x6, 0x200000,CRC(3e081c03) SHA1(6ccb162952f6076359b2785b5d800b39a9a3c5ce) ) /* identical to "pr1cg6.18d" */
	ROM_LOAD( "pr1cg7.15a",	 0x200000*0x7, 0x200000,CRC(ec9fc5c8) SHA1(16de614b26f06bbddae3ab56cebba45efd6fe81b) ) /* identical to "pr1cg7.19d" */

	ROM_REGION16_LE( 0x280000, "textilemap", 0 ) /* texture tilemap */
	ROM_LOAD( "pr1ccrl.3d",	 0x000000, 0x200000,CRC(e01321fd) SHA1(5938c6eff8e1b3642728c3be733f567a97cb5aad) ) /* identical to "pr1ccrl.7b" */
	ROM_LOAD( "pr1ccrh.1d",	 0x200000, 0x080000,CRC(1d68bc31) SHA1(d534d0daebe7018e83b57cc7919c294ab89bddc8) ) /* identical to "pr1ccrh.5b" */
	/* These two ROMs define a huge texture tilemap using the tiles from "textile".
     * The tilemap has 0x100 columns.
     *
     * pr1ccrl contains little endian 16 bit words.  Each word references a 16x16 tile.
     *
     * pr1ccrh.1d contains packed nibbles.  Each nibble encodes three tile attributes:
     *  0x8 = swapxy
     *  0x4 = flipx
     *  0x2 = flipy
     *  0x1 = tile bank (used in some sys22 games)
     */

	ROM_REGION( 0x80000*9, "pointrom", 0 ) /* 3d model data */
	ROM_LOAD( "pr1ptrl0.18k", 0x80000*0, 0x80000,CRC(fddb27a2) SHA1(6e837b45e3f9ed7ca3d1a457d0f0124de5618d1f) )
	ROM_LOAD( "pr1ptrl1.16k", 0x80000*1, 0x80000,CRC(6964dd06) SHA1(f38a550165504693d20892a7dcfaf01db19b04ef) )
	ROM_LOAD( "pr1ptrl2.15k", 0x80000*2, 0x80000,CRC(4d7ed1d4) SHA1(8f72864a06ff8962e640cb36d062bddf5d110308) )

	ROM_LOAD( "pr1ptrm0.18j", 0x80000*3, 0x80000,CRC(b6f204b7) SHA1(3b34f240b399b6406faaf338ae0ab536247e64a6) )
	ROM_LOAD( "pr1ptrm1.16j", 0x80000*4, 0x80000,CRC(949588b7) SHA1(fdaf50ff2496200b9c981efc18b035f3c0a96ace) )
	ROM_LOAD( "pr1ptrm2.15j", 0x80000*5, 0x80000,CRC(dc1cef0a) SHA1(8cbc02cf73fac3cc110b676d77602ae628385eae) )

	ROM_LOAD( "pr1ptru0.18f", 0x80000*6, 0x80000,CRC(5d66a7c4) SHA1(c9ed1c18724192d45c1f6b40096f15d02baf2401) )
	ROM_LOAD( "pr1ptru1.16f", 0x80000*7, 0x80000,CRC(e9a3f72b) SHA1(f967e1adf8eee4fffdf4288d36a93c5bb4f9a126) )
	ROM_LOAD( "pr1ptru2.15f", 0x80000*8, 0x80000,CRC(c346a842) SHA1(299bc0a30d0e74d8adfa3dc605aebf6439f5bc18) )

	ROM_REGION( 0xc00000, "c352", 0 ) /* sound samples */
	ROM_LOAD( "pr1wavea.2l", 0x000000, 0x400000, CRC(320f3913) SHA1(3887b7334ca7762794c14198dd24bab47fcd9505) )
	ROM_LOAD( "pr1waveb.1l", 0x800000, 0x400000, CRC(d91acb26) SHA1(c2161e2d70e08aed15cbc875ffee685190611daf) )
ROM_END

ROM_START( acedrvrw )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* main program */
	ROM_LOAD32_BYTE( "ad2prgll.4d", 0x00003, 0x80000, CRC(808c5ff8) SHA1(119c90ecb5aa099a0e5d1d7944c004beacead367) )
	ROM_LOAD32_BYTE( "ad2prglm.2d", 0x00002, 0x80000, CRC(5f726a10) SHA1(d077312c6a387fbdf906d278c73c6a3730687f32) )
	ROM_LOAD32_BYTE( "ad2prgum.8d", 0x00001, 0x80000, CRC(d5042d6e) SHA1(9ae93e7ea7126302831a879ba0aadcb6e5b842f5) )
	ROM_LOAD32_BYTE( "ad2prguu.6d", 0x00000, 0x80000, CRC(86d4661d) SHA1(2a1529a51ca5466994a2d0d84c7aab13cef95a11) )

	ROM_REGION( 0x10000*2, "master", 0 ) /* Master DSP */
	ROM_LOAD16_WORD( "c71.bin", 0,0x1000*2, CRC(47c623ab) SHA1(e363ac50f5556f83308d4cc191b455e9b62bcfc8) )

	ROM_REGION( 0x10000*2, "slave", 0 ) /* Slave DSP */
	ROM_LOAD16_WORD( "c71.bin", 0,0x1000*2, CRC(47c623ab) SHA1(e363ac50f5556f83308d4cc191b455e9b62bcfc8) )

	ROM_REGION( 0x080000, "mcu", 0 ) /* BIOS */
	ROM_LOAD( "c74.bin", 0x0000, 0x4000, CRC(a3dce360) SHA1(8f3248b1890abb2e649927240ae46f73bb171e3b) )

	ROM_REGION( 0x100000, "user4", 0 ) /* sound data and MCU BIOS */
	ROM_LOAD( "ad1data.6r", 0, 0x080000, CRC(82024f74) SHA1(711ab0c4f027716aeab18e3a5d3d06fa82af8007) )

	ROM_REGION( 0x200000*8, "textile", 0) /* 16x16x8bpp texture tiles */
	ROM_LOAD( "ad1cg0.1a", 0x200000*0x4, 0x200000,CRC(faaa1ee2) SHA1(878f2b74587ed4d06c5110a0eb0020c49ddc5dfa) )
	ROM_LOAD( "ad1cg1.2a", 0x200000*0x5, 0x200000,CRC(1aab1eb7) SHA1(b8f9eeafec7e0de340cf48e38fa55dd14404c867) )
	ROM_LOAD( "ad1cg2.3a", 0x200000*0x6, 0x200000,CRC(cdcd1874) SHA1(5a7a4a0d897cca4956b0a4f178f39f618c921861) )
	ROM_LOAD( "ad1cg3.5a", 0x200000*0x7, 0x200000,CRC(effdd2cd) SHA1(9ff156e7e38c103b8fa6f3c29776dd38482d9cf2) )

	ROM_REGION16_LE( 0x280000, "textilemap", 0 ) /* texture tilemap */
	ROM_LOAD( "ad1ccrl.1c", 0x000000, 0x200000,CRC(bc3c9b12) SHA1(088e861e5c4b37c54b7f72963113a10870bf7927) )
	ROM_LOAD( "ad1ccrh.2c", 0x200000, 0x080000,CRC(71f44526) SHA1(bb4811fc5de626380ce6a17bee73e5e47926d850) )

	ROM_REGION( 0x80000*6, "pointrom", 0 ) /* 3d model data */
	ROM_LOAD( "ad1potl0.5b", 0x80000*0, 0x80000,CRC(dfc7e729) SHA1(5e3deef66d0a5dd2ff0584b8c8be4bf5e798e4d0) )
	ROM_LOAD( "ad1potl1.4b", 0x80000*1, 0x80000,CRC(5914ef8e) SHA1(f6db9c3061ceda76eef0a9538d9c048366b71124) )
	ROM_LOAD( "ad1potm0.5c", 0x80000*2, 0x80000,CRC(844bcd6b) SHA1(629b8dc0a7e94410c08c8874b69d9f4bc22f3e4f) )
	ROM_LOAD( "ad1potm1.4c", 0x80000*3, 0x80000,CRC(515cf541) SHA1(db1522813ea3e982d479cc1903d18799bf75aea9) )
	ROM_LOAD( "ad1potu0.5d", 0x80000*4, 0x80000,CRC(e0f44949) SHA1(ffdb64d600883974b05edaa9ed3071af355ee17f) )
	ROM_LOAD( "ad1potu1.4d", 0x80000*5, 0x80000,CRC(f2cd2cbb) SHA1(19fe6e3454a1e4353c7fe0a0d7a71742fea946de) )

	ROM_REGION( 0x400000, "c352", 0 ) /* sound samples */
	ROM_LOAD( "ad1wave0.10r", 0x100000*0, 0x100000,CRC(c7879a72) SHA1(ae04d664858b0944583590ed0003a9420032d5ca) )
	ROM_LOAD( "ad1wave1.10p", 0x100000*2, 0x100000,CRC(69c1d41e) SHA1(b5cdfe7b75075c585dfd842347f8e4e692bb2781) )
	ROM_LOAD( "ad1wave2.10n", 0x100000*1, 0x100000,CRC(365a6831) SHA1(ddaa44a4436d6de120b64a5d130b1ee18f872e19) )
	ROM_LOAD( "ad1wave3.10l", 0x100000*3, 0x100000,CRC(cd8ecb0b) SHA1(7950b5a3a81f5554f57accabc7a623b8265a21a1) )

	ROM_REGION( 0x300, "user1", 0 )
	ROM_LOAD( "rr1gam.2d",   0x0000, 0x0100, CRC(b2161bce) SHA1(d2681cc0cf8e68df0d942d392b4eb4458c4bb356) )
	ROM_LOAD( "rr1gam.3d",   0x0100, 0x0100, CRC(b2161bce) SHA1(d2681cc0cf8e68df0d942d392b4eb4458c4bb356) )
	ROM_LOAD( "rr1gam.4d",   0x0200, 0x0100, CRC(b2161bce) SHA1(d2681cc0cf8e68df0d942d392b4eb4458c4bb356) )

	ROM_REGION( 0x2000, "user2", 0 )
	ROM_LOAD( "eeprom.9e",  0x0000, 0x2000, CRC(483d9237) SHA1(a696b22433a26f40f0839fa958fb26ad5cef9163) )
ROM_END

ROM_START( victlapw )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* main program */
	ROM_LOAD32_BYTE( "advprgll.4d", 0x00003, 0x80000, CRC(4dc1b0ab) SHA1(b5913388d16f824af6dbb01b5b0350d510667a87) )
	ROM_LOAD32_BYTE( "advprglm.2d", 0x00002, 0x80000, CRC(7b658bef) SHA1(cf982b49fde0c1897c4c16e77f9eb2a145d8cd42) )
	ROM_LOAD32_BYTE( "advprgum.8d", 0x00001, 0x80000, CRC(af67f2fb) SHA1(f391843ee0d053e33660c60e3718871142d932f2) )
	ROM_LOAD32_BYTE( "advprguu.6d", 0x00000, 0x80000, CRC(b60e5d2b) SHA1(f5740615c2864c5c6433275cf4388bda5122b7a7) )

	ROM_REGION( 0x10000*2, "master", 0 ) /* Master DSP */
	ROM_LOAD16_WORD( "c71.bin", 0,0x1000*2, CRC(47c623ab) SHA1(e363ac50f5556f83308d4cc191b455e9b62bcfc8) )

	ROM_REGION( 0x10000*2, "slave", 0 ) /* Slave DSP */
	ROM_LOAD16_WORD( "c71.bin", 0,0x1000*2, CRC(47c623ab) SHA1(e363ac50f5556f83308d4cc191b455e9b62bcfc8) )

	ROM_REGION( 0x080000, "mcu", 0 ) /* BIOS */
	ROM_LOAD( "c74.bin", 0x0000, 0x4000, CRC(a3dce360) SHA1(8f3248b1890abb2e649927240ae46f73bb171e3b) )

	ROM_REGION( 0x100000, "user4", 0 ) /* sound data and MCU BIOS */
	ROM_LOAD( "adv1data.6r", 0, 0x080000, CRC(10eecdb4) SHA1(aaedeed166614e6670e765e0d7e4e9eb5f38ad10) )

	ROM_REGION( 0x200000*8, "textile", 0) /* 16x16x8bpp texture tiles */
	ROM_LOAD( "adv1cg0.2a",  0x200000*0x0, 0x200000,CRC(13353848) SHA1(c6c7693e3cb086919daf9fcaf6bf602142213073) )
	ROM_LOAD( "adv1cg1.1c",  0x200000*0x1, 0x200000,CRC(1542066c) SHA1(20a053e919b7a81da2a17d31dc7482832a4d4ffe) )
	ROM_LOAD( "adv1cg2.2d",  0x200000*0x2, 0x200000,CRC(111f371c) SHA1(29d8062daae51b3c1712bd30baa9813a2b5b374d) )
	ROM_LOAD( "adv1cg3.1e",  0x200000*0x3, 0x200000,CRC(a077831f) SHA1(71bb95199b368e48bc474123ca84d19213f73137) )
	ROM_LOAD( "adv1cg4.2f",  0x200000*0x4, 0x200000,CRC(71abdacf) SHA1(64409e6aa40dd9e5a6dd1dc306860fbbf6ee7c3e) )
	ROM_LOAD( "adv1cg5.1j",  0x200000*0x5, 0x200000,CRC(cd6cd798) SHA1(51070997a457c0ace078174569cd548ac2226b2d) )
	ROM_LOAD( "adv1cg6.2k",  0x200000*0x6, 0x200000,CRC(94bdafba) SHA1(41e64fa99b342edd8b0ed95ae9869c23e03399e6) )
	ROM_LOAD( "adv1cg7.1n",	 0x200000*0x7, 0x200000,CRC(18823475) SHA1(a3244d665b59c352593de21f5cb8d55ddf8cee5c) )

	ROM_REGION16_LE( 0x280000, "textilemap", 0 ) /* texture tilemap */
	ROM_LOAD( "adv1ccrl.5a",	 0x000000, 0x200000,CRC(dd2b96ae) SHA1(6337ce17e617234c27ebad578ba82451649aad9c) ) /* ident to adv1ccrl.5l */
	ROM_LOAD( "adv1ccrh.5c",	 0x200000, 0x080000,CRC(5719844a) SHA1(a17d7bc239235e9f566931ba4fee1d6ad7964d83) ) /* ident to adv1ccrh.5j */

	ROM_REGION( 0x80000*9, "pointrom", 0 ) /* 3d model data */
	ROM_LOAD( "adv1pot.l0", 0x80000*0, 0x80000,CRC(3b85b2a4) SHA1(84c92ed0105618d4aa5508af344b4b6cfa772567) )
	ROM_LOAD( "adv1pot.l1", 0x80000*1, 0x80000,CRC(601d6488) SHA1(c7932103ba6070e17deb3cc06060eed7789f938e) )
	ROM_LOAD( "adv1pot.l2", 0x80000*2, 0x80000,CRC(a0323a84) SHA1(deadf9a47461df7b137759d6886e676137b39fd2) )
	ROM_LOAD( "adv1pot.m0", 0x80000*3, 0x80000,CRC(20951aa2) SHA1(3de55bded443a5b78699cec4845470b53b22301a) )
	ROM_LOAD( "adv1pot.m1", 0x80000*4, 0x80000,CRC(5aed6fbf) SHA1(8cee781d8a12e00635b9a1e5cc8d82e64b17e8f1) )
	ROM_LOAD( "adv1pot.m2", 0x80000*5, 0x80000,CRC(00cbff92) SHA1(09a11ba064aafc921a7ca0add5898d91b773f10a) )
	ROM_LOAD( "adv1pot.u0", 0x80000*6, 0x80000,CRC(6b73dd2a) SHA1(e3654ab2b62e4f3314558209e37c5636f871a6c7) )
	ROM_LOAD( "adv1pot.u1", 0x80000*7, 0x80000,CRC(c8788f74) SHA1(606e10b05146e3db824aa608745de80584420d12) )
	ROM_LOAD( "adv1pot.u2", 0x80000*8, 0x80000,CRC(e67f29c5) SHA1(16222afb4f1f494711dd00ebb347c824db333bae) )

	ROM_REGION( 0x400000, "c352", 0 ) /* sound samples */
	ROM_LOAD( "adv1wav0.10r", 0x000000, 0x100000, CRC(f07b2d9d) SHA1(fd46c23b336d5e9a748f7f8d825c19737125d2fb) )
	ROM_LOAD( "adv1wav1.10p", 0x200000, 0x100000, CRC(737f3c7a) SHA1(4737994f146c0076e7270785f41f3a85c53c7c5f) )
	ROM_LOAD( "adv1wav2.10n", 0x100000, 0x100000, CRC(c1a5ca5e) SHA1(27e6f9256d5fe5966e91d6be1e6e80900a764af1) )
	ROM_LOAD( "adv1wav3.10l", 0x300000, 0x100000, CRC(fc6b8004) SHA1(5c9e0805895931ec2b6a43376059bdbf5777222f) )

	ROM_REGION( 0x300, "user1", 0 )
	ROM_LOAD( "rr1gam.2d",   0x0000, 0x0100, CRC(b2161bce) SHA1(d2681cc0cf8e68df0d942d392b4eb4458c4bb356) )
	ROM_LOAD( "rr1gam.3d",   0x0100, 0x0100, CRC(b2161bce) SHA1(d2681cc0cf8e68df0d942d392b4eb4458c4bb356) )
	ROM_LOAD( "rr1gam.4d",   0x0200, 0x0100, CRC(b2161bce) SHA1(d2681cc0cf8e68df0d942d392b4eb4458c4bb356) )

	ROM_REGION( 0x2000, "user2", 0 )
	ROM_LOAD( "eeprom.9e", 0, 8*1024, CRC(35fd9f7a) SHA1(7dc542795a6b0b9580c5fd1bf80e1e6f2c402078) )
ROM_END

ROM_START( raveracw )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* main program */
	ROM_LOAD32_BYTE( "rv2prllb.4d", 0x00003, 0x80000, CRC(3017cd1e) SHA1(ccd648b4a5dfc74fd141815af2969f423311042f) )
	ROM_LOAD32_BYTE( "rv2prlmb.2d", 0x00002, 0x80000, CRC(894be0c3) SHA1(4dba93dc3ca1cf502c5f54018b64ad79bb2a632b) )
	ROM_LOAD32_BYTE( "rv2prumb.8d", 0x00001, 0x80000, CRC(6414a800) SHA1(c278ff644909d12a43ba6fc2bf8d2092e469c3e6) )
	ROM_LOAD32_BYTE( "rv2pruub.6d", 0x00000, 0x80000, CRC(a9f18714) SHA1(8e7b17749d151f92020f68d1ac06003cf1f5c573) )

	ROM_REGION( 0x10000*2, "master", 0 ) /* Master DSP */
	ROM_LOAD16_WORD( "c71.bin", 0,0x1000*2, CRC(47c623ab) SHA1(e363ac50f5556f83308d4cc191b455e9b62bcfc8) )

	ROM_REGION( 0x10000*2, "slave", 0 ) /* Slave DSP */
	ROM_LOAD16_WORD( "c71.bin", 0,0x1000*2, CRC(47c623ab) SHA1(e363ac50f5556f83308d4cc191b455e9b62bcfc8) )

	ROM_REGION( 0x80000, "mcu", 0 ) /* BIOS */
	ROM_LOAD( "c74.bin", 0x0000, 0x4000, CRC(a3dce360) SHA1(8f3248b1890abb2e649927240ae46f73bb171e3b) )

	ROM_REGION( 0x100000, "user4", 0 ) /* sound data and MCU BIOS */
	ROM_LOAD( "rv1data.6r", 0, 0x080000, CRC(d358ec20) SHA1(140c513349240417bb546dd2d151f3666b818e91) )

  ROM_REGION( 0x200000*8, "textile", 0) /* 16x16x8bpp texture tiles */
	ROM_LOAD( "rv1cg0.1a", 0x200000*0x0, 0x200000,CRC(c518f06b) SHA1(4c01d453244192dd13087bdc72a7f7be80b47cbc) ) /* rv1cg0.2a */
	ROM_LOAD( "rv1cg1.1c", 0x200000*0x1, 0x200000,CRC(6628f792) SHA1(7a5405c5fcb2f3f001ae17df393c31e61a834f2b) ) /* rv1cg1.2c */
	ROM_LOAD( "rv1cg2.1d", 0x200000*0x2, 0x200000,CRC(0b707cc5) SHA1(38e1a554b278062edc369565353497ac4b016f78) ) /* rv1cg2.2d */
	ROM_LOAD( "rv1cg3.1e", 0x200000*0x3, 0x200000,CRC(39b62921) SHA1(873287d81338baf10dd85214d82f6c38bfdf199e) ) /* rv1cg3.2e */
	ROM_LOAD( "rv1cg4.1f", 0x200000*0x4, 0x200000,CRC(a9791ea2) SHA1(245b2ebbadd1fbca90dc241f88e9f6f341b2a01a) ) /* rv1cg4.2f */
	ROM_LOAD( "rv1cg5.1j", 0x200000*0x5, 0x200000,CRC(b2c79ec1) SHA1(6f669996863bdf1fe09b0c1a2a876625029d3d43) ) /* rv1cg5.2j */
	ROM_LOAD( "rv1cg6.1k", 0x200000*0x6, 0x200000,CRC(8cddedc2) SHA1(e3993f5505bc7e61bec7be5b48c873572e1220f7) ) /* rv1cg6.2k */
	ROM_LOAD( "rv1cg7.1n", 0x200000*0x7, 0x200000,CRC(b39147ca) SHA1(50ca6691fc809c95e6999dd52e39f2b8c2d22f3b) ) /* rv1cg7.2n */

	ROM_REGION16_LE( 0x280000, "textilemap", 0 ) /* texture tilemap */
	ROM_LOAD( "rv1ccrl.5a",	 0x000000, 0x200000,CRC(bc634f72) SHA1(b5c504ed92bca7682614fc4c51f38cff607e6f2a) ) /* rv1ccrl.5l */
	ROM_LOAD( "rv1ccrh.5c",	 0x200000, 0x080000,CRC(a741b262) SHA1(363076220a0eacc67befda05f8253963e8ffbcaa) ) /* rv1ccrh.5j */

	ROM_REGION( 0x80000*12, "pointrom", 0 ) /* 3d model data */
	ROM_LOAD( "rv1potl0.5b", 0x80000*0x0, 0x80000,CRC(de2ce519) SHA1(2fe0dd000571f76d1a4df6a439d40119125170ef) )
	ROM_LOAD( "rv1potl1.4b", 0x80000*0x1, 0x80000,CRC(2215cb5a) SHA1(d48ee692ab3dbcffdc49d22f6f232ca9390da766) )
	ROM_LOAD( "rv1potl2.3b", 0x80000*0x2, 0x80000,CRC(ddb15bf7) SHA1(4c54ec98e0cba10841d43a4ce593cdacfd7f90f8) )
	ROM_LOAD( "rv1potl3.2b", 0x80000*0x3, 0x80000,CRC(fa9361ca) SHA1(35a5c2712bca9c62400b724754de3a931ad21561) )
	ROM_LOAD( "rv1potm0.5c", 0x80000*0x4, 0x80000,CRC(3c024f3a) SHA1(711f0442823797b2d410352796a5cca66af98dce) )
	ROM_LOAD( "rv1potm1.4c", 0x80000*0x5, 0x80000,CRC(b1a32a68) SHA1(e24abb3a7e35d098abae5420bf8ef5c975718987) )
	ROM_LOAD( "rv1potm2.3c", 0x80000*0x6, 0x80000,CRC(a414fe15) SHA1(eb27cdca045ab2ab27dec179043328847fb65e11) )
	ROM_LOAD( "rv1potm3.2c", 0x80000*0x7, 0x80000,CRC(2953bbb4) SHA1(aca1acd87f7130d2522d0c6f8e60beeb7ab7495a) )
	ROM_LOAD( "rv1potu0.5d", 0x80000*0x8, 0x80000,CRC(b9eaf3cc) SHA1(3b2a9041f1fa90706ecf7d4fbff918516f891a07) )
	ROM_LOAD( "rv1potu1.4d", 0x80000*0x9, 0x80000,CRC(a5c55258) SHA1(826d4dde761aec7d848456f7bc4ba6268fe99605) )
	ROM_LOAD( "rv1potu2.3d", 0x80000*0xa, 0x80000,CRC(c18fcb74) SHA1(a4009ae2b014dc89aed4741fd97f84350117c2f4) )
	ROM_LOAD( "rv1potu3.2d", 0x80000*0xb, 0x80000,CRC(79735aaa) SHA1(1cf14274669b916a7641f7a16785da1b72347485) )

	ROM_REGION( 0x400000, "c352", 0 ) /* sound samples */
	ROM_LOAD( "rv1wav0.10r", 0x000000, 0x100000, CRC(5aef8143) SHA1(a75d31298e3ff9b290f238976a11e8b85cfb72d3) )
	ROM_LOAD( "rv1wav1.10p", 0x200000, 0x100000, CRC(9ed9e6b3) SHA1(dd1da2b08d1b6aa0912daacc77744c9799aabb78) )
	ROM_LOAD( "rv1wav2.10n", 0x100000, 0x100000, CRC(5af9dc83) SHA1(9aeb7f8217b806a6f3ed93056513af9fbcb6b372) )
	ROM_LOAD( "rv1wav3.10l", 0x300000, 0x100000, CRC(ffb9ad75) SHA1(a9a61a597bd3bbe9732f92747d82264fe4d9af48) )

	ROM_REGION( 0x300, "user1", 0 )
	ROM_LOAD( "rr1gam.2d",   0x0000, 0x0100, CRC(b2161bce) SHA1(d2681cc0cf8e68df0d942d392b4eb4458c4bb356) )
	ROM_LOAD( "rr1gam.3d",   0x0100, 0x0100, CRC(b2161bce) SHA1(d2681cc0cf8e68df0d942d392b4eb4458c4bb356) )
	ROM_LOAD( "rr1gam.4d",   0x0200, 0x0100, CRC(b2161bce) SHA1(d2681cc0cf8e68df0d942d392b4eb4458c4bb356) )

	ROM_REGION( 0x2000, "user2", 0 )
	ROM_LOAD( "rv1eeprm.9e", 0x0000, 0x2000, CRC(801222e6) SHA1(a97ba76ad73f75fe7289e2c0d60b2dfdf2a99604) ) /* EPROM */
ROM_END

ROM_START( raveracj )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* main program */
	ROM_LOAD32_BYTE( "rv1prllb.4d", 0x00003, 0x80000, CRC(71da3eea) SHA1(8a641bb23e0ad89cae5ee1570f8a3627b2434d20) )
	ROM_LOAD32_BYTE( "rv1prlmb.2d", 0x00002, 0x80000, CRC(6ab7e9ce) SHA1(0c6376ca5a63409aeea344bbc201af6c47afe9ab) )
	ROM_LOAD32_BYTE( "rv1prumb.8d", 0x00001, 0x80000, CRC(375fabcf) SHA1(448e3db3e3fab8c7c27e214ab5a5fa84e5f84366) )
	ROM_LOAD32_BYTE( "rv1pruub.6d", 0x00000, 0x80000, CRC(92f834d6) SHA1(028368790f0293fcfea5c7b12f7f315e27a62f77) )

	ROM_REGION( 0x10000*2, "master", 0 ) /* Master DSP */
	ROM_LOAD16_WORD( "c71.bin", 0,0x1000*2, CRC(47c623ab) SHA1(e363ac50f5556f83308d4cc191b455e9b62bcfc8) )

	ROM_REGION( 0x10000*2, "slave", 0 ) /* Slave DSP */
	ROM_LOAD16_WORD( "c71.bin", 0,0x1000*2, CRC(47c623ab) SHA1(e363ac50f5556f83308d4cc191b455e9b62bcfc8) )

	ROM_REGION( 0x80000, "mcu", 0 ) /* BIOS */
	ROM_LOAD( "c74.bin", 0x0000, 0x4000, CRC(a3dce360) SHA1(8f3248b1890abb2e649927240ae46f73bb171e3b) )

	ROM_REGION( 0x100000, "user4", 0 ) /* sound data and MCU BIOS */
	ROM_LOAD( "rv1data.6r", 0, 0x080000, CRC(d358ec20) SHA1(140c513349240417bb546dd2d151f3666b818e91) )

	ROM_REGION( 0x200000*8, "textile", 0) /* 16x16x8bpp texture tiles */
	ROM_LOAD( "rv1cg0.1a", 0x200000*0x0, 0x200000,CRC(c518f06b) SHA1(4c01d453244192dd13087bdc72a7f7be80b47cbc) ) /* rv1cg0.2a */
	ROM_LOAD( "rv1cg1.1c", 0x200000*0x1, 0x200000,CRC(6628f792) SHA1(7a5405c5fcb2f3f001ae17df393c31e61a834f2b) ) /* rv1cg1.2c */
	ROM_LOAD( "rv1cg2.1d", 0x200000*0x2, 0x200000,CRC(0b707cc5) SHA1(38e1a554b278062edc369565353497ac4b016f78) ) /* rv1cg2.2d */
	ROM_LOAD( "rv1cg3.1e", 0x200000*0x3, 0x200000,CRC(39b62921) SHA1(873287d81338baf10dd85214d82f6c38bfdf199e) ) /* rv1cg3.2e */
	ROM_LOAD( "rv1cg4.1f", 0x200000*0x4, 0x200000,CRC(a9791ea2) SHA1(245b2ebbadd1fbca90dc241f88e9f6f341b2a01a) ) /* rv1cg4.2f */
	ROM_LOAD( "rv1cg5.1j", 0x200000*0x5, 0x200000,CRC(b2c79ec1) SHA1(6f669996863bdf1fe09b0c1a2a876625029d3d43) ) /* rv1cg5.2j */
	ROM_LOAD( "rv1cg6.1k", 0x200000*0x6, 0x200000,CRC(8cddedc2) SHA1(e3993f5505bc7e61bec7be5b48c873572e1220f7) ) /* rv1cg6.2k */
	ROM_LOAD( "rv1cg7.1n", 0x200000*0x7, 0x200000,CRC(b39147ca) SHA1(50ca6691fc809c95e6999dd52e39f2b8c2d22f3b) ) /* rv1cg7.2n */

	ROM_REGION16_LE( 0x280000, "textilemap", 0 ) /* texture tilemap */
	ROM_LOAD( "rv1ccrl.5a",	 0x000000, 0x200000,CRC(bc634f72) SHA1(b5c504ed92bca7682614fc4c51f38cff607e6f2a) ) /* rv1ccrl.5l */
	ROM_LOAD( "rv1ccrh.5c",	 0x200000, 0x080000,CRC(a741b262) SHA1(363076220a0eacc67befda05f8253963e8ffbcaa) ) /* rv1ccrh.5j */

	ROM_REGION( 0x80000*12, "pointrom", 0 ) /* 3d model data */
	ROM_LOAD( "rv1potl0.5b", 0x80000*0x0, 0x80000,CRC(de2ce519) SHA1(2fe0dd000571f76d1a4df6a439d40119125170ef) )
	ROM_LOAD( "rv1potl1.4b", 0x80000*0x1, 0x80000,CRC(2215cb5a) SHA1(d48ee692ab3dbcffdc49d22f6f232ca9390da766) )
	ROM_LOAD( "rv1potl2.3b", 0x80000*0x2, 0x80000,CRC(ddb15bf7) SHA1(4c54ec98e0cba10841d43a4ce593cdacfd7f90f8) )
	ROM_LOAD( "rv1potl3.2b", 0x80000*0x3, 0x80000,CRC(fa9361ca) SHA1(35a5c2712bca9c62400b724754de3a931ad21561) )
	ROM_LOAD( "rv1potm0.5c", 0x80000*0x4, 0x80000,CRC(3c024f3a) SHA1(711f0442823797b2d410352796a5cca66af98dce) )
	ROM_LOAD( "rv1potm1.4c", 0x80000*0x5, 0x80000,CRC(b1a32a68) SHA1(e24abb3a7e35d098abae5420bf8ef5c975718987) )
	ROM_LOAD( "rv1potm2.3c", 0x80000*0x6, 0x80000,CRC(a414fe15) SHA1(eb27cdca045ab2ab27dec179043328847fb65e11) )
	ROM_LOAD( "rv1potm3.2c", 0x80000*0x7, 0x80000,CRC(2953bbb4) SHA1(aca1acd87f7130d2522d0c6f8e60beeb7ab7495a) )
	ROM_LOAD( "rv1potu0.5d", 0x80000*0x8, 0x80000,CRC(b9eaf3cc) SHA1(3b2a9041f1fa90706ecf7d4fbff918516f891a07) )
	ROM_LOAD( "rv1potu1.4d", 0x80000*0x9, 0x80000,CRC(a5c55258) SHA1(826d4dde761aec7d848456f7bc4ba6268fe99605) )
	ROM_LOAD( "rv1potu2.3d", 0x80000*0xa, 0x80000,CRC(c18fcb74) SHA1(a4009ae2b014dc89aed4741fd97f84350117c2f4) )
	ROM_LOAD( "rv1potu3.2d", 0x80000*0xb, 0x80000,CRC(79735aaa) SHA1(1cf14274669b916a7641f7a16785da1b72347485) )

	ROM_REGION( 0x400000, "c352", 0 ) /* sound samples */
	ROM_LOAD( "rv1wav0.10r", 0x000000, 0x100000, CRC(5aef8143) SHA1(a75d31298e3ff9b290f238976a11e8b85cfb72d3) )
	ROM_LOAD( "rv1wav1.10p", 0x200000, 0x100000, CRC(9ed9e6b3) SHA1(dd1da2b08d1b6aa0912daacc77744c9799aabb78) )
	ROM_LOAD( "rv1wav2.10n", 0x100000, 0x100000, CRC(5af9dc83) SHA1(9aeb7f8217b806a6f3ed93056513af9fbcb6b372) )
	ROM_LOAD( "rv1wav3.10l", 0x300000, 0x100000, CRC(ffb9ad75) SHA1(a9a61a597bd3bbe9732f92747d82264fe4d9af48) )

	ROM_REGION( 0x300, "user1", 0 )
	ROM_LOAD( "rr1gam.2d",   0x0000, 0x0100, CRC(b2161bce) SHA1(d2681cc0cf8e68df0d942d392b4eb4458c4bb356) )
	ROM_LOAD( "rr1gam.3d",   0x0100, 0x0100, CRC(b2161bce) SHA1(d2681cc0cf8e68df0d942d392b4eb4458c4bb356) )
	ROM_LOAD( "rr1gam.4d",   0x0200, 0x0100, CRC(b2161bce) SHA1(d2681cc0cf8e68df0d942d392b4eb4458c4bb356) )

	ROM_REGION( 0x2000, "user2", 0 )
	ROM_LOAD( "rv1eeprm.9e", 0x0000, 0x2000, CRC(801222e6) SHA1(a97ba76ad73f75fe7289e2c0d60b2dfdf2a99604) ) /* EPROM */
ROM_END

ROM_START( raveracja )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* main program */
	ROM_LOAD32_BYTE( "rv1prll.4d", 0x00003, 0x80000, CRC(5dfce6cd) SHA1(1aeeca1e507ae4cbe3d39ca5efd1cc4fe1ab03a8) )
	ROM_LOAD32_BYTE( "rv1prlm.2d", 0x00002, 0x80000, CRC(0d4d9f74) SHA1(f886b0629cbf5a369af1f44e53c6fd3f51b3fbc9) )
	ROM_LOAD32_BYTE( "rv1prum.8d", 0x00001, 0x80000, CRC(28e503e3) SHA1(a3071461f840f28c65c660de215c73f812f356b3) )
	ROM_LOAD32_BYTE( "rv1pruu.6d", 0x00000, 0x80000, CRC(c47d9ff4) SHA1(4d7c4ac4151a3b306e7277937add8eee26e561a6) )

	ROM_REGION( 0x10000*2, "master", 0 ) /* Master DSP */
	ROM_LOAD16_WORD( "c71.bin", 0,0x1000*2, CRC(47c623ab) SHA1(e363ac50f5556f83308d4cc191b455e9b62bcfc8) )

	ROM_REGION( 0x10000*2, "slave", 0 ) /* Slave DSP */
	ROM_LOAD16_WORD( "c71.bin", 0,0x1000*2, CRC(47c623ab) SHA1(e363ac50f5556f83308d4cc191b455e9b62bcfc8) )

	ROM_REGION( 0x80000, "mcu", 0 ) /* BIOS */
	ROM_LOAD( "c74.bin", 0x0000, 0x4000, CRC(a3dce360) SHA1(8f3248b1890abb2e649927240ae46f73bb171e3b) )

	ROM_REGION( 0x100000, "user4", 0 ) /* sound data and MCU BIOS */
	ROM_LOAD( "rv1data.6r", 0, 0x080000, CRC(d358ec20) SHA1(140c513349240417bb546dd2d151f3666b818e91) )

	ROM_REGION( 0x200000*8, "textile", 0) /* 16x16x8bpp texture tiles */
	ROM_LOAD( "rv1cg0.1a", 0x200000*0x0, 0x200000,CRC(c518f06b) SHA1(4c01d453244192dd13087bdc72a7f7be80b47cbc) ) /* rv1cg0.2a */
	ROM_LOAD( "rv1cg1.1c", 0x200000*0x1, 0x200000,CRC(6628f792) SHA1(7a5405c5fcb2f3f001ae17df393c31e61a834f2b) ) /* rv1cg1.2c */
	ROM_LOAD( "rv1cg2.1d", 0x200000*0x2, 0x200000,CRC(0b707cc5) SHA1(38e1a554b278062edc369565353497ac4b016f78) ) /* rv1cg2.2d */
	ROM_LOAD( "rv1cg3.1e", 0x200000*0x3, 0x200000,CRC(39b62921) SHA1(873287d81338baf10dd85214d82f6c38bfdf199e) ) /* rv1cg3.2e */
	ROM_LOAD( "rv1cg4.1f", 0x200000*0x4, 0x200000,CRC(a9791ea2) SHA1(245b2ebbadd1fbca90dc241f88e9f6f341b2a01a) ) /* rv1cg4.2f */
	ROM_LOAD( "rv1cg5.1j", 0x200000*0x5, 0x200000,CRC(b2c79ec1) SHA1(6f669996863bdf1fe09b0c1a2a876625029d3d43) ) /* rv1cg5.2j */
	ROM_LOAD( "rv1cg6.1k", 0x200000*0x6, 0x200000,CRC(8cddedc2) SHA1(e3993f5505bc7e61bec7be5b48c873572e1220f7) ) /* rv1cg6.2k */
	ROM_LOAD( "rv1cg7.1n", 0x200000*0x7, 0x200000,CRC(b39147ca) SHA1(50ca6691fc809c95e6999dd52e39f2b8c2d22f3b) ) /* rv1cg7.2n */

	ROM_REGION16_LE( 0x280000, "textilemap", 0 ) /* texture tilemap */
	ROM_LOAD( "rv1ccrl.5a",	 0x000000, 0x200000,CRC(bc634f72) SHA1(b5c504ed92bca7682614fc4c51f38cff607e6f2a) ) /* rv1ccrl.5l */
	ROM_LOAD( "rv1ccrh.5c",	 0x200000, 0x080000,CRC(a741b262) SHA1(363076220a0eacc67befda05f8253963e8ffbcaa) ) /* rv1ccrh.5j */

	ROM_REGION( 0x80000*12, "pointrom", 0 ) /* 3d model data */
	ROM_LOAD( "rv1potl0.5b", 0x80000*0x0, 0x80000,CRC(de2ce519) SHA1(2fe0dd000571f76d1a4df6a439d40119125170ef) )
	ROM_LOAD( "rv1potl1.4b", 0x80000*0x1, 0x80000,CRC(2215cb5a) SHA1(d48ee692ab3dbcffdc49d22f6f232ca9390da766) )
	ROM_LOAD( "rv1potl2.3b", 0x80000*0x2, 0x80000,CRC(ddb15bf7) SHA1(4c54ec98e0cba10841d43a4ce593cdacfd7f90f8) )
	ROM_LOAD( "rv1potl3.2b", 0x80000*0x3, 0x80000,CRC(fa9361ca) SHA1(35a5c2712bca9c62400b724754de3a931ad21561) )
	ROM_LOAD( "rv1potm0.5c", 0x80000*0x4, 0x80000,CRC(3c024f3a) SHA1(711f0442823797b2d410352796a5cca66af98dce) )
	ROM_LOAD( "rv1potm1.4c", 0x80000*0x5, 0x80000,CRC(b1a32a68) SHA1(e24abb3a7e35d098abae5420bf8ef5c975718987) )
	ROM_LOAD( "rv1potm2.3c", 0x80000*0x6, 0x80000,CRC(a414fe15) SHA1(eb27cdca045ab2ab27dec179043328847fb65e11) )
	ROM_LOAD( "rv1potm3.2c", 0x80000*0x7, 0x80000,CRC(2953bbb4) SHA1(aca1acd87f7130d2522d0c6f8e60beeb7ab7495a) )
	ROM_LOAD( "rv1potu0.5d", 0x80000*0x8, 0x80000,CRC(b9eaf3cc) SHA1(3b2a9041f1fa90706ecf7d4fbff918516f891a07) )
	ROM_LOAD( "rv1potu1.4d", 0x80000*0x9, 0x80000,CRC(a5c55258) SHA1(826d4dde761aec7d848456f7bc4ba6268fe99605) )
	ROM_LOAD( "rv1potu2.3d", 0x80000*0xa, 0x80000,CRC(c18fcb74) SHA1(a4009ae2b014dc89aed4741fd97f84350117c2f4) )
	ROM_LOAD( "rv1potu3.2d", 0x80000*0xb, 0x80000,CRC(79735aaa) SHA1(1cf14274669b916a7641f7a16785da1b72347485) )

	ROM_REGION( 0x400000, "c352", 0 ) /* sound samples */
	ROM_LOAD( "rv1wav0.10r", 0x000000, 0x100000, CRC(5aef8143) SHA1(a75d31298e3ff9b290f238976a11e8b85cfb72d3) )
	ROM_LOAD( "rv1wav1.10p", 0x200000, 0x100000, CRC(9ed9e6b3) SHA1(dd1da2b08d1b6aa0912daacc77744c9799aabb78) )
	ROM_LOAD( "rv1wav2.10n", 0x100000, 0x100000, CRC(5af9dc83) SHA1(9aeb7f8217b806a6f3ed93056513af9fbcb6b372) )
	ROM_LOAD( "rv1wav3.10l", 0x300000, 0x100000, CRC(ffb9ad75) SHA1(a9a61a597bd3bbe9732f92747d82264fe4d9af48) )

	ROM_REGION( 0x300, "user1", 0 )
	ROM_LOAD( "rr1gam.2d",   0x0000, 0x0100, CRC(b2161bce) SHA1(d2681cc0cf8e68df0d942d392b4eb4458c4bb356) )
	ROM_LOAD( "rr1gam.3d",   0x0100, 0x0100, CRC(b2161bce) SHA1(d2681cc0cf8e68df0d942d392b4eb4458c4bb356) )
	ROM_LOAD( "rr1gam.4d",   0x0200, 0x0100, CRC(b2161bce) SHA1(d2681cc0cf8e68df0d942d392b4eb4458c4bb356) )

	ROM_REGION( 0x2000, "user2", 0 )
	ROM_LOAD( "rv1eeprm.9e", 0x0000, 0x2000, CRC(801222e6) SHA1(a97ba76ad73f75fe7289e2c0d60b2dfdf2a99604) ) /* EPROM */
ROM_END

ROM_START( ridgera2 )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* main program */
	ROM_LOAD32_BYTE( "rrs2prll.4d",  0x00003, 0x80000, CRC(88199c0f) SHA1(5cf5bb714c3d209943a8d815eaea60afd34641ff) )
	ROM_LOAD32_BYTE( "rrs1prlmb.2d", 0x00002, 0x80000, CRC(8e86f199) SHA1(7bd9ec9147ef0380864508f66203ef2c6ad1f7f6) )
	ROM_LOAD32_BYTE( "rrs1prumb.8d", 0x00001, 0x80000, CRC(78c360b6) SHA1(8ee502291359cbc8aef39145c8fe7538311cc58f) )
	ROM_LOAD32_BYTE( "rrs1pruub.6d", 0x00000, 0x80000, CRC(60d6d4a4) SHA1(759762a9b7d7aee7ee1b44b1721e5356898aa7ea) )

	ROM_REGION( 0x10000*2, "master", 0 ) /* Master DSP */
	ROM_LOAD16_WORD( "c71.bin", 0,0x1000*2, CRC(47c623ab) SHA1(e363ac50f5556f83308d4cc191b455e9b62bcfc8) )

	ROM_REGION( 0x10000*2, "slave", 0 ) /* Slave DSP */
	ROM_LOAD16_WORD( "c71.bin", 0,0x1000*2, CRC(47c623ab) SHA1(e363ac50f5556f83308d4cc191b455e9b62bcfc8) )

	ROM_REGION( 0x80000, "mcu", 0 ) /* BIOS */
	ROM_LOAD( "c74.bin", 0x0000, 0x4000, CRC(a3dce360) SHA1(8f3248b1890abb2e649927240ae46f73bb171e3b) )

	ROM_REGION( 0x100000, "user4", 0 ) /* sound data and MCU BIOS */
	ROM_LOAD( "rrs1data.6r", 0, 0x080000, CRC(b7063aa8) SHA1(08ff689e8dd529b91eee423c93f084945c6de417) )

   ROM_REGION( 0x200000*8, "textile", 0) /* 16x16x8bpp texture tiles */
	ROM_LOAD( "rrs1cg0.1a", 0x200000*0x4, 0x200000,CRC(714c0091) SHA1(df29512bd6e64827660c40304051366d2c4d7977) )
	ROM_LOAD( "rrs1cg1.2a", 0x200000*0x5, 0x200000,CRC(836545c1) SHA1(05e3346463d8d42b5d33216207e855033a65510d) )
	ROM_LOAD( "rrs1cg2.3a", 0x200000*0x6, 0x200000,CRC(00e9799d) SHA1(280184451138420f64080efe13e5e2795f7b61d4) )
	ROM_LOAD( "rrs1cg3.5a", 0x200000*0x7, 0x200000,CRC(3858983f) SHA1(feda270b71f1310ecf4c17823bc8827ca2951b40) )

	ROM_REGION16_LE( 0x280000, "textilemap", 0 ) /* texture tilemap */
	ROM_LOAD( "rrs1ccrl.5a", 0x000000, 0x200000,CRC(304a8b57) SHA1(f4f3e7c194697d754375f36a0e41d0941fa5d225) )
	ROM_LOAD( "rrs1ccrh.5c", 0x200000, 0x080000,CRC(bd3c86ab) SHA1(cd3a8774843c5864e651fa8989c80e2d975a13e8) )

	ROM_REGION( 0x80000*6, "pointrom", 0 ) /* 3d model data */
	ROM_LOAD( "rrs1pol0.5b", 0x80000*0, 0x80000,CRC(9376c384) SHA1(cde0e36db1beab1523607098a760d81fac2ea90e) )
	ROM_LOAD( "rrs1pol1.4b", 0x80000*1, 0x80000,CRC(094fa832) SHA1(cc59442540b1cdef068c4408b6e048c11042beb8) )
	ROM_LOAD( "rrs1pom0.5c", 0x80000*2, 0x80000,CRC(b47a7f8b) SHA1(0fa0456ad8b4864a7071b5b5ba1a78877c1ac0f0) )
	ROM_LOAD( "rrs1pom1.4c", 0x80000*3, 0x80000,CRC(27260361) SHA1(8775cc779eb8b6a0d79fa84d606c970ec2d6ea8d) )
	ROM_LOAD( "rrs1pou0.5d", 0x80000*4, 0x80000,CRC(74d6ec84) SHA1(63f5beee51443c98100330ec04291f71e10716c4) )
	ROM_LOAD( "rrs1pou1.4d", 0x80000*5, 0x80000,CRC(f527caaa) SHA1(f92bdd15323239d593ddac92a11d23a27e6635ed) )

	ROM_REGION( 0x400000, "c352", 0 ) /* sound samples */
	ROM_LOAD( "rrs1wav0.10r", 0x100000*0, 0x100000,CRC(99d11a2d) SHA1(1f3db98a99be0f07c03b0a7817561459a58f310e) )
	ROM_LOAD( "rrs1wav1.10p", 0x100000*2, 0x100000,CRC(ad28444a) SHA1(c31bbf3cae5015e5494fe4988b9b01d822224c69) )
	ROM_LOAD( "rrs1wav2.10n", 0x100000*1, 0x100000,CRC(6f0d4619) SHA1(cd3d57f2ea21497f388ffa29ec7d2665647a01c0) )
	ROM_LOAD( "rrs1wav3.10l", 0x100000*3, 0x100000,CRC(106e761f) SHA1(97f47b857bdcbc79b0aface53dd385e67fcc9108) )

	ROM_REGION( 0x300, "user1", 0 )
	ROM_LOAD( "rr1gam.2d",   0x0000, 0x0100, CRC(b2161bce) SHA1(d2681cc0cf8e68df0d942d392b4eb4458c4bb356) )
	ROM_LOAD( "rr1gam.3d",   0x0100, 0x0100, CRC(b2161bce) SHA1(d2681cc0cf8e68df0d942d392b4eb4458c4bb356) )
	ROM_LOAD( "rr1gam.4d",   0x0200, 0x0100, CRC(b2161bce) SHA1(d2681cc0cf8e68df0d942d392b4eb4458c4bb356) )

	ROM_REGION( 0x2000, "user2", ROMREGION_ERASE00 )
ROM_END

ROM_START( ridgera2j )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* main program */
	ROM_LOAD32_BYTE( "rrs1prllb.4d", 0x00003, 0x80000, CRC(22f069e5) SHA1(fcaec3aa83853c39d713ed01c511060663027ccd) )
	ROM_LOAD32_BYTE( "rrs1prlmb.2d", 0x00002, 0x80000, CRC(8e86f199) SHA1(7bd9ec9147ef0380864508f66203ef2c6ad1f7f6) )
	ROM_LOAD32_BYTE( "rrs1prumb.8d", 0x00001, 0x80000, CRC(78c360b6) SHA1(8ee502291359cbc8aef39145c8fe7538311cc58f) )
	ROM_LOAD32_BYTE( "rrs1pruub.6d", 0x00000, 0x80000, CRC(60d6d4a4) SHA1(759762a9b7d7aee7ee1b44b1721e5356898aa7ea) )

	ROM_REGION( 0x10000*2, "master", 0 ) /* Master DSP */
	ROM_LOAD16_WORD( "c71.bin", 0,0x1000*2, CRC(47c623ab) SHA1(e363ac50f5556f83308d4cc191b455e9b62bcfc8) )

	ROM_REGION( 0x10000*2, "slave", 0 ) /* Slave DSP */
	ROM_LOAD16_WORD( "c71.bin", 0,0x1000*2, CRC(47c623ab) SHA1(e363ac50f5556f83308d4cc191b455e9b62bcfc8) )

	ROM_REGION( 0x80000, "mcu", 0 ) /* BIOS */
	ROM_LOAD( "c74.bin", 0x0000, 0x4000, CRC(a3dce360) SHA1(8f3248b1890abb2e649927240ae46f73bb171e3b) )

	ROM_REGION( 0x100000, "user4", 0 ) /* sound data and MCU BIOS */
	ROM_LOAD( "rrs1data.6r", 0, 0x080000, CRC(b7063aa8) SHA1(08ff689e8dd529b91eee423c93f084945c6de417) )

   ROM_REGION( 0x200000*8, "textile", 0) /* 16x16x8bpp texture tiles */
	ROM_LOAD( "rrs1cg0.1a", 0x200000*0x4, 0x200000,CRC(714c0091) SHA1(df29512bd6e64827660c40304051366d2c4d7977) )
	ROM_LOAD( "rrs1cg1.2a", 0x200000*0x5, 0x200000,CRC(836545c1) SHA1(05e3346463d8d42b5d33216207e855033a65510d) )
	ROM_LOAD( "rrs1cg2.3a", 0x200000*0x6, 0x200000,CRC(00e9799d) SHA1(280184451138420f64080efe13e5e2795f7b61d4) )
	ROM_LOAD( "rrs1cg3.5a", 0x200000*0x7, 0x200000,CRC(3858983f) SHA1(feda270b71f1310ecf4c17823bc8827ca2951b40) )

	ROM_REGION16_LE( 0x280000, "textilemap", 0 ) /* texture tilemap */
	ROM_LOAD( "rrs1ccrl.5a", 0x000000, 0x200000,CRC(304a8b57) SHA1(f4f3e7c194697d754375f36a0e41d0941fa5d225) )
	ROM_LOAD( "rrs1ccrh.5c", 0x200000, 0x080000,CRC(bd3c86ab) SHA1(cd3a8774843c5864e651fa8989c80e2d975a13e8) )

	ROM_REGION( 0x80000*6, "pointrom", 0 ) /* 3d model data */
	ROM_LOAD( "rrs1pol0.5b", 0x80000*0, 0x80000,CRC(9376c384) SHA1(cde0e36db1beab1523607098a760d81fac2ea90e) )
	ROM_LOAD( "rrs1pol1.4b", 0x80000*1, 0x80000,CRC(094fa832) SHA1(cc59442540b1cdef068c4408b6e048c11042beb8) )
	ROM_LOAD( "rrs1pom0.5c", 0x80000*2, 0x80000,CRC(b47a7f8b) SHA1(0fa0456ad8b4864a7071b5b5ba1a78877c1ac0f0) )
	ROM_LOAD( "rrs1pom1.4c", 0x80000*3, 0x80000,CRC(27260361) SHA1(8775cc779eb8b6a0d79fa84d606c970ec2d6ea8d) )
	ROM_LOAD( "rrs1pou0.5d", 0x80000*4, 0x80000,CRC(74d6ec84) SHA1(63f5beee51443c98100330ec04291f71e10716c4) )
	ROM_LOAD( "rrs1pou1.4d", 0x80000*5, 0x80000,CRC(f527caaa) SHA1(f92bdd15323239d593ddac92a11d23a27e6635ed) )

	ROM_REGION( 0x400000, "c352", 0 ) /* sound samples */
	ROM_LOAD( "rrs1wav0.10r", 0x100000*0, 0x100000,CRC(99d11a2d) SHA1(1f3db98a99be0f07c03b0a7817561459a58f310e) )
	ROM_LOAD( "rrs1wav1.10p", 0x100000*2, 0x100000,CRC(ad28444a) SHA1(c31bbf3cae5015e5494fe4988b9b01d822224c69) )
	ROM_LOAD( "rrs1wav2.10n", 0x100000*1, 0x100000,CRC(6f0d4619) SHA1(cd3d57f2ea21497f388ffa29ec7d2665647a01c0) )
	ROM_LOAD( "rrs1wav3.10l", 0x100000*3, 0x100000,CRC(106e761f) SHA1(97f47b857bdcbc79b0aface53dd385e67fcc9108) )

	ROM_REGION( 0x300, "user1", 0 )
	ROM_LOAD( "rr1gam.2d",   0x0000, 0x0100, CRC(b2161bce) SHA1(d2681cc0cf8e68df0d942d392b4eb4458c4bb356) )
	ROM_LOAD( "rr1gam.3d",   0x0100, 0x0100, CRC(b2161bce) SHA1(d2681cc0cf8e68df0d942d392b4eb4458c4bb356) )
	ROM_LOAD( "rr1gam.4d",   0x0200, 0x0100, CRC(b2161bce) SHA1(d2681cc0cf8e68df0d942d392b4eb4458c4bb356) )

	ROM_REGION( 0x2000, "user2", ROMREGION_ERASE00 )

ROM_END

ROM_START( ridgera2ja )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* main program */
	ROM_LOAD32_BYTE( "rrs1prll.4d", 0x00003, 0x80000, CRC(fbf785a2) SHA1(b9333c7623f68f48aa6ae50913a22a527a19576a) )
	ROM_LOAD32_BYTE( "rrs1prlm.2d", 0x00002, 0x80000, CRC(562f747a) SHA1(79d818b87c9a992fc9706fb39e6d560c2b0aa392) )
	ROM_LOAD32_BYTE( "rrs1prum.8d", 0x00001, 0x80000, CRC(93259fb0) SHA1(c29787e873797a003db27adbd20d7b852e26d8c6) )
	ROM_LOAD32_BYTE( "rrs1pruu.6d", 0x00000, 0x80000, CRC(31cdefe8) SHA1(ae836d389bed43dd156eb4cf3e97b6f1ad68181e) )

	ROM_REGION( 0x10000*2, "master", 0 ) /* Master DSP */
	ROM_LOAD16_WORD( "c71.bin", 0,0x1000*2, CRC(47c623ab) SHA1(e363ac50f5556f83308d4cc191b455e9b62bcfc8) )

	ROM_REGION( 0x10000*2, "slave", 0 ) /* Slave DSP */
	ROM_LOAD16_WORD( "c71.bin", 0,0x1000*2, CRC(47c623ab) SHA1(e363ac50f5556f83308d4cc191b455e9b62bcfc8) )

	ROM_REGION( 0x80000, "mcu", 0 ) /* BIOS */
	ROM_LOAD( "c74.bin", 0x0000, 0x4000, CRC(a3dce360) SHA1(8f3248b1890abb2e649927240ae46f73bb171e3b) )

	ROM_REGION( 0x100000, "user4", 0 ) /* sound data and MCU BIOS */
	ROM_LOAD( "rrs1data.6r", 0, 0x080000, CRC(b7063aa8) SHA1(08ff689e8dd529b91eee423c93f084945c6de417) )

	ROM_REGION( 0x200000*8, "textile", 0) /* 16x16x8bpp texture tiles */
	ROM_LOAD( "rrs1cg0.1a", 0x200000*0x4, 0x200000,CRC(714c0091) SHA1(df29512bd6e64827660c40304051366d2c4d7977) )
	ROM_LOAD( "rrs1cg1.2a", 0x200000*0x5, 0x200000,CRC(836545c1) SHA1(05e3346463d8d42b5d33216207e855033a65510d) )
	ROM_LOAD( "rrs1cg2.3a", 0x200000*0x6, 0x200000,CRC(00e9799d) SHA1(280184451138420f64080efe13e5e2795f7b61d4) )
	ROM_LOAD( "rrs1cg3.5a", 0x200000*0x7, 0x200000,CRC(3858983f) SHA1(feda270b71f1310ecf4c17823bc8827ca2951b40) )

	ROM_REGION16_LE( 0x280000, "textilemap", 0 ) /* texture tilemap */
	ROM_LOAD( "rrs1ccrl.5a", 0x000000, 0x200000,CRC(304a8b57) SHA1(f4f3e7c194697d754375f36a0e41d0941fa5d225) )
	ROM_LOAD( "rrs1ccrh.5c", 0x200000, 0x080000,CRC(bd3c86ab) SHA1(cd3a8774843c5864e651fa8989c80e2d975a13e8) )

	ROM_REGION( 0x80000*6, "pointrom", 0 ) /* 3d model data */
	ROM_LOAD( "rrs1pol0.5b", 0x80000*0, 0x80000,CRC(9376c384) SHA1(cde0e36db1beab1523607098a760d81fac2ea90e) )
	ROM_LOAD( "rrs1pol1.4b", 0x80000*1, 0x80000,CRC(094fa832) SHA1(cc59442540b1cdef068c4408b6e048c11042beb8) )
	ROM_LOAD( "rrs1pom0.5c", 0x80000*2, 0x80000,CRC(b47a7f8b) SHA1(0fa0456ad8b4864a7071b5b5ba1a78877c1ac0f0) )
	ROM_LOAD( "rrs1pom1.4c", 0x80000*3, 0x80000,CRC(27260361) SHA1(8775cc779eb8b6a0d79fa84d606c970ec2d6ea8d) )
	ROM_LOAD( "rrs1pou0.5d", 0x80000*4, 0x80000,CRC(74d6ec84) SHA1(63f5beee51443c98100330ec04291f71e10716c4) )
	ROM_LOAD( "rrs1pou1.4d", 0x80000*5, 0x80000,CRC(f527caaa) SHA1(f92bdd15323239d593ddac92a11d23a27e6635ed) )

	ROM_REGION( 0x400000, "c352", 0 ) /* sound samples */
	ROM_LOAD( "rrs1wav0.10r", 0x100000*0, 0x100000,CRC(99d11a2d) SHA1(1f3db98a99be0f07c03b0a7817561459a58f310e) )
	ROM_LOAD( "rrs1wav1.10p", 0x100000*2, 0x100000,CRC(ad28444a) SHA1(c31bbf3cae5015e5494fe4988b9b01d822224c69) )
	ROM_LOAD( "rrs1wav2.10n", 0x100000*1, 0x100000,CRC(6f0d4619) SHA1(cd3d57f2ea21497f388ffa29ec7d2665647a01c0) )
	ROM_LOAD( "rrs1wav3.10l", 0x100000*3, 0x100000,CRC(106e761f) SHA1(97f47b857bdcbc79b0aface53dd385e67fcc9108) )

	ROM_REGION( 0x300, "user1", 0 )
	ROM_LOAD( "rr1gam.2d",   0x0000, 0x0100, CRC(b2161bce) SHA1(d2681cc0cf8e68df0d942d392b4eb4458c4bb356) )
	ROM_LOAD( "rr1gam.3d",   0x0100, 0x0100, CRC(b2161bce) SHA1(d2681cc0cf8e68df0d942d392b4eb4458c4bb356) )
	ROM_LOAD( "rr1gam.4d",   0x0200, 0x0100, CRC(b2161bce) SHA1(d2681cc0cf8e68df0d942d392b4eb4458c4bb356) )

	ROM_REGION( 0x2000, "user2", ROMREGION_ERASE00 )
ROM_END

ROM_START( ridgerac )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* main program */
	ROM_LOAD32_BYTE( "rr3prgll.4d", 0x00003, 0x80000, CRC(856fe5ec) SHA1(72d95b8bd5da551c3d358b8ab266373a89f8aa6a) )
	ROM_LOAD32_BYTE( "rr3prglm.2d", 0x00002, 0x80000, CRC(1e9ef0a9) SHA1(a4577bcdf13673568793d8a324945fca30b10f43) )
	ROM_LOAD32_BYTE( "rr3prgum.8d", 0x00001, 0x80000, CRC(e160f63f) SHA1(9b4b7a13eb4bc19fcb53daedb87e4945c20a1b8e) )
	ROM_LOAD32_BYTE( "rr3prguu.6d", 0x00000, 0x80000, CRC(f07c78c0) SHA1(dbed76d868b761711faf5b6e11f2c9affb91db5d) )

	ROM_REGION( 0x10000*2, "master", 0 ) /* Master DSP */
	ROM_LOAD16_WORD( "c71.bin", 0,0x1000*2, CRC(47c623ab) SHA1(e363ac50f5556f83308d4cc191b455e9b62bcfc8) )

	ROM_REGION( 0x10000*2, "slave", 0 ) /* Slave DSP */
	ROM_LOAD16_WORD( "c71.bin", 0,0x1000*2, CRC(47c623ab) SHA1(e363ac50f5556f83308d4cc191b455e9b62bcfc8) )

	ROM_REGION( 0x080000, "mcu", 0 ) /* BIOS */
	ROM_LOAD( "c74.bin", 0x0000, 0x4000, CRC(a3dce360) SHA1(8f3248b1890abb2e649927240ae46f73bb171e3b) )

	ROM_REGION( 0x100000, "user4", 0 ) /* sound data */
	ROM_LOAD( "rr1data.6r", 0, 0x080000, CRC(18f5f748) SHA1(e0d149a66de36156edd9b55f604c9a9801aaefa8) )

	ROM_REGION( 0x200000*8, "textile", 0) /* 16x16x8bpp texture tiles */
	ROM_LOAD( "rr1cg0.bin", 0x200000*0x4, 0x200000, CRC(b557a795) SHA1(f345486ffbe797246ad80a55d3c4a332ed6e2888) )//,CRC(d1b0eec6) SHA1(f66922c324dfc3ff408db7556c587ef90ca64c3b) )
	ROM_LOAD( "rr1cg1.bin", 0x200000*0x5, 0x200000, CRC(0fa212d9) SHA1(a1311de0a504e2d399044fa8ac32ec6c56ec965f) )//,CRC(bb695d89) SHA1(557bac9d2718519c1f69e374d0ef9a86a43fe86c) )
	ROM_LOAD( "rr1cg2.bin", 0x200000*0x6, 0x200000, CRC(18e2d2bd) SHA1(69c2ea62eeb255f27d3c69373f6716b0a34683cc) )//,CRC(8f374c0a) SHA1(94ff8581de11a03ef86525155f8433bf5858b980) )
	ROM_LOAD( "rr1cg3.bin", 0x200000*0x7, 0x200000, CRC(9564488b) SHA1(6b27d1aea75d6be747c62e165cfa49ecc5d9e767) )//,CRC(072a5c47) SHA1(86b8e973ae6b78197d685fe6d14722d8e2d0dfec) )

	ROM_REGION16_LE( 0x280000, "textilemap", 0 ) /* texture tilemap */
	ROM_LOAD( "rr1ccrl.bin",0x000000, 0x200000, CRC(6092d181) SHA1(52c0e3ac20aa23059a87d1a985d24ae641577310) )//,CRC(c15cb257) SHA1(0cb8f231c62ea37955be5d452a436a6e815af8e8) )
	ROM_LOAD( "rr1ccrh.bin",0x200000, 0x080000, CRC(dd332fd5) SHA1(a7d9c1d6b5a8e3a937b525c1363880e404dcd147) )//,CRC(dd332fd5) SHA1(a7d9c1d6b5a8e3a937b525c1363880e404dcd147) )

	ROM_REGION( 0x80000*6, "pointrom", 0 ) /* 3d model data */
	ROM_LOAD( "rr1potl0.5b", 0x80000*0, 0x80000,CRC(3ac193e3) SHA1(ff213766f15e34dc1b25187b57d94e17930090a3) )
	ROM_LOAD( "rr1potl1.4b", 0x80000*1, 0x80000,CRC(ac3ffba5) SHA1(4eb4dda5faeff237e0d35725b56d309948fba900) )
	ROM_LOAD( "rr1potm0.5c", 0x80000*2, 0x80000,CRC(42a3fa08) SHA1(15db0ae7ccf7f5a77b9dd9a9d82a488b67f8aaff) )
	ROM_LOAD( "rr1potm1.4c", 0x80000*3, 0x80000,CRC(1bc1ea7b) SHA1(52c21eef4989c45acc5fa4deda2d0b63214731c8) )
	ROM_LOAD( "rr1potu0.5d", 0x80000*4, 0x80000,CRC(5e367f72) SHA1(5887f011379dce865fef238b402678a3d2033de9) )
	ROM_LOAD( "rr1potu1.4d", 0x80000*5, 0x80000,CRC(31d92475) SHA1(51d3c0baa223e1bc16ea2950f2e085597528f870) )

	ROM_REGION( 0x800000, "c352", 0 ) /* sound samples */
	ROM_LOAD( "rr1wav0.10r", 0x100000*0, 0x100000,CRC(a8e85bde) SHA1(b56677e9f6c98f7b600043f5dcfef3a482ca7455) )
	ROM_LOAD( "rr1wav1.10p", 0x100000*2, 0x100000,CRC(35f47c8e) SHA1(7c3f9e942f532af8008fbead2a96fee6084bcde6) )
	ROM_LOAD( "rr1wav2.10n", 0x100000*1, 0x100000,CRC(3244cb59) SHA1(b3283b30cfafbfdcbc6d482ecc4ed6a47a527ca4) )
	ROM_LOAD( "rr1wav3.10l", 0x100000*3, 0x100000,CRC(c4cda1a7) SHA1(60bc96880ec79efdff3cc70c09e848692a40bea4) )

	ROM_REGION( 0x300, "user1", 0 )
	ROM_LOAD( "rr1gam.2d",   0x0000, 0x0100, CRC(b2161bce) SHA1(d2681cc0cf8e68df0d942d392b4eb4458c4bb356) )
	ROM_LOAD( "rr1gam.3d",   0x0100, 0x0100, CRC(b2161bce) SHA1(d2681cc0cf8e68df0d942d392b4eb4458c4bb356) )
	ROM_LOAD( "rr1gam.4d",   0x0200, 0x0100, CRC(b2161bce) SHA1(d2681cc0cf8e68df0d942d392b4eb4458c4bb356) )

	ROM_REGION( 0x2000, "user2", ROMREGION_ERASE00 )
ROM_END



ROM_START( ridgeracb )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* main program */
	ROM_LOAD32_BYTE( "rr1prll.4d", 0x00003, 0x80000, CRC(4bb7fc86) SHA1(8291375b8ec4d37e0d9e3bf38da2d5907b0f31bd) )
	ROM_LOAD32_BYTE( "rr1prlm.2d", 0x00002, 0x80000, CRC(68e13830) SHA1(ddc447c7afbb5c4238969d7e78bfe9cf8fac6061) )
	ROM_LOAD32_BYTE( "rr1prum.8d", 0x00001, 0x80000, CRC(705ef78a) SHA1(881903413e66d6fd83d46eb18c4e1230531832ae) )
	ROM_LOAD32_BYTE( "rr2pruu.6d", 0x00000, 0x80000, CRC(a79e456f) SHA1(049c596e01e53e3a401c5c4260517f170688d387) )

	ROM_REGION( 0x10000*2, "master", 0 ) /* Master DSP */
	ROM_LOAD16_WORD( "c71.bin", 0,0x1000*2, CRC(47c623ab) SHA1(e363ac50f5556f83308d4cc191b455e9b62bcfc8) )

	ROM_REGION( 0x10000*2, "slave", 0 ) /* Slave DSP */
	ROM_LOAD16_WORD( "c71.bin", 0,0x1000*2, CRC(47c623ab) SHA1(e363ac50f5556f83308d4cc191b455e9b62bcfc8) )

	ROM_REGION( 0x080000, "mcu", 0 ) /* BIOS */
	ROM_LOAD( "c74.bin", 0x0000, 0x4000, CRC(a3dce360) SHA1(8f3248b1890abb2e649927240ae46f73bb171e3b) )

	ROM_REGION( 0x100000, "user4", 0 ) /* sound data */
	ROM_LOAD( "rr1data.6r", 0, 0x080000, CRC(18f5f748) SHA1(e0d149a66de36156edd9b55f604c9a9801aaefa8) )

	ROM_REGION( 0x200000*8, "textile", 0) /* 16x16x8bpp texture tiles */
	ROM_LOAD( "rr1cg0.bin", 0x200000*0x4, 0x200000, CRC(b557a795) SHA1(f345486ffbe797246ad80a55d3c4a332ed6e2888) )//,CRC(d1b0eec6) SHA1(f66922c324dfc3ff408db7556c587ef90ca64c3b) )
	ROM_LOAD( "rr1cg1.bin", 0x200000*0x5, 0x200000, CRC(0fa212d9) SHA1(a1311de0a504e2d399044fa8ac32ec6c56ec965f) )//,CRC(bb695d89) SHA1(557bac9d2718519c1f69e374d0ef9a86a43fe86c) )
	ROM_LOAD( "rr1cg2.bin", 0x200000*0x6, 0x200000, CRC(18e2d2bd) SHA1(69c2ea62eeb255f27d3c69373f6716b0a34683cc) )//,CRC(8f374c0a) SHA1(94ff8581de11a03ef86525155f8433bf5858b980) )
	ROM_LOAD( "rr1cg3.bin", 0x200000*0x7, 0x200000, CRC(9564488b) SHA1(6b27d1aea75d6be747c62e165cfa49ecc5d9e767) )//,CRC(072a5c47) SHA1(86b8e973ae6b78197d685fe6d14722d8e2d0dfec) )

	ROM_REGION16_LE( 0x280000, "textilemap", 0 ) /* texture tilemap */
	ROM_LOAD( "rr1ccrl.bin",0x000000, 0x200000, CRC(6092d181) SHA1(52c0e3ac20aa23059a87d1a985d24ae641577310) )//,CRC(c15cb257) SHA1(0cb8f231c62ea37955be5d452a436a6e815af8e8) )
	ROM_LOAD( "rr1ccrh.bin",0x200000, 0x080000, CRC(dd332fd5) SHA1(a7d9c1d6b5a8e3a937b525c1363880e404dcd147) )//,CRC(dd332fd5) SHA1(a7d9c1d6b5a8e3a937b525c1363880e404dcd147) )

	ROM_REGION( 0x80000*6, "pointrom", 0 ) /* 3d model data */
	ROM_LOAD( "rr1potl0.5b", 0x80000*0, 0x80000,CRC(3ac193e3) SHA1(ff213766f15e34dc1b25187b57d94e17930090a3) )
	ROM_LOAD( "rr1potl1.4b", 0x80000*1, 0x80000,CRC(ac3ffba5) SHA1(4eb4dda5faeff237e0d35725b56d309948fba900) )
	ROM_LOAD( "rr1potm0.5c", 0x80000*2, 0x80000,CRC(42a3fa08) SHA1(15db0ae7ccf7f5a77b9dd9a9d82a488b67f8aaff) )
	ROM_LOAD( "rr1potm1.4c", 0x80000*3, 0x80000,CRC(1bc1ea7b) SHA1(52c21eef4989c45acc5fa4deda2d0b63214731c8) )
	ROM_LOAD( "rr1potu0.5d", 0x80000*4, 0x80000,CRC(5e367f72) SHA1(5887f011379dce865fef238b402678a3d2033de9) )
	ROM_LOAD( "rr1potu1.4d", 0x80000*5, 0x80000,CRC(31d92475) SHA1(51d3c0baa223e1bc16ea2950f2e085597528f870) )

	ROM_REGION( 0x800000, "c352", 0 ) /* sound samples */
	ROM_LOAD( "rr1wav0.10r", 0x100000*0, 0x100000,CRC(a8e85bde) SHA1(b56677e9f6c98f7b600043f5dcfef3a482ca7455) )
	ROM_LOAD( "rr1wav1.10p", 0x100000*2, 0x100000,CRC(35f47c8e) SHA1(7c3f9e942f532af8008fbead2a96fee6084bcde6) )
	ROM_LOAD( "rr1wav2.10n", 0x100000*1, 0x100000,CRC(3244cb59) SHA1(b3283b30cfafbfdcbc6d482ecc4ed6a47a527ca4) )
	ROM_LOAD( "rr1wav3.10l", 0x100000*3, 0x100000,CRC(c4cda1a7) SHA1(60bc96880ec79efdff3cc70c09e848692a40bea4) )

	ROM_REGION( 0x300, "user1", 0 )
	ROM_LOAD( "rr1gam.2d",   0x0000, 0x0100, CRC(b2161bce) SHA1(d2681cc0cf8e68df0d942d392b4eb4458c4bb356) )
	ROM_LOAD( "rr1gam.3d",   0x0100, 0x0100, CRC(b2161bce) SHA1(d2681cc0cf8e68df0d942d392b4eb4458c4bb356) )
	ROM_LOAD( "rr1gam.4d",   0x0200, 0x0100, CRC(b2161bce) SHA1(d2681cc0cf8e68df0d942d392b4eb4458c4bb356) )

	ROM_REGION( 0x2000, "user2", ROMREGION_ERASE00 )
ROM_END

ROM_START( ridgeracj )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* main program */
	ROM_LOAD32_BYTE( "rr1prll.4d", 0x00003, 0x80000, CRC(4bb7fc86) SHA1(8291375b8ec4d37e0d9e3bf38da2d5907b0f31bd) )
	ROM_LOAD32_BYTE( "rr1prlm.2d", 0x00002, 0x80000, CRC(68e13830) SHA1(ddc447c7afbb5c4238969d7e78bfe9cf8fac6061) )
	ROM_LOAD32_BYTE( "rr1prum.8d", 0x00001, 0x80000, CRC(705ef78a) SHA1(881903413e66d6fd83d46eb18c4e1230531832ae) )
	ROM_LOAD32_BYTE( "rr1pruu.6d", 0x00000, 0x80000, CRC(c1371f96) SHA1(a78e0bf6c147c034487a85efa0a8470f4e8f4bf0) )

	ROM_REGION( 0x10000*2, "master", 0 ) /* Master DSP */
	ROM_LOAD16_WORD( "c71.bin", 0,0x1000*2, CRC(47c623ab) SHA1(e363ac50f5556f83308d4cc191b455e9b62bcfc8) )

	ROM_REGION( 0x10000*2, "slave", 0 ) /* Slave DSP */
	ROM_LOAD16_WORD( "c71.bin", 0,0x1000*2, CRC(47c623ab) SHA1(e363ac50f5556f83308d4cc191b455e9b62bcfc8) )

	ROM_REGION( 0x080000, "mcu", 0 ) /* BIOS */
	ROM_LOAD( "c74.bin", 0x0000, 0x4000, CRC(a3dce360) SHA1(8f3248b1890abb2e649927240ae46f73bb171e3b) )

	ROM_REGION( 0x100000, "user4", 0 ) /* sound data and MCU BIOS */
	ROM_LOAD( "rr1data.6r", 0, 0x080000, CRC(18f5f748) SHA1(e0d149a66de36156edd9b55f604c9a9801aaefa8) )

   ROM_REGION( 0x200000*8, "textile", 0) /* 16x16x8bpp texture tiles */
	ROM_LOAD( "rr1cg0.bin", 0x200000*0x4, 0x200000, CRC(b557a795) SHA1(f345486ffbe797246ad80a55d3c4a332ed6e2888) )//,CRC(d1b0eec6) SHA1(f66922c324dfc3ff408db7556c587ef90ca64c3b) )
	ROM_LOAD( "rr1cg1.bin", 0x200000*0x5, 0x200000, CRC(0fa212d9) SHA1(a1311de0a504e2d399044fa8ac32ec6c56ec965f) )//,CRC(bb695d89) SHA1(557bac9d2718519c1f69e374d0ef9a86a43fe86c) )
	ROM_LOAD( "rr1cg2.bin", 0x200000*0x6, 0x200000, CRC(18e2d2bd) SHA1(69c2ea62eeb255f27d3c69373f6716b0a34683cc) )//,CRC(8f374c0a) SHA1(94ff8581de11a03ef86525155f8433bf5858b980) )
	ROM_LOAD( "rr1cg3.bin", 0x200000*0x7, 0x200000, CRC(9564488b) SHA1(6b27d1aea75d6be747c62e165cfa49ecc5d9e767) )//,CRC(072a5c47) SHA1(86b8e973ae6b78197d685fe6d14722d8e2d0dfec) )

	ROM_REGION16_LE( 0x280000, "textilemap", 0 ) /* texture tilemap */
	ROM_LOAD( "rr1ccrl.bin",0x000000, 0x200000, CRC(6092d181) SHA1(52c0e3ac20aa23059a87d1a985d24ae641577310) )//,CRC(c15cb257) SHA1(0cb8f231c62ea37955be5d452a436a6e815af8e8) )
	ROM_LOAD( "rr1ccrh.bin",0x200000, 0x080000, CRC(dd332fd5) SHA1(a7d9c1d6b5a8e3a937b525c1363880e404dcd147) )//,CRC(dd332fd5) SHA1(a7d9c1d6b5a8e3a937b525c1363880e404dcd147) )

	ROM_REGION( 0x80000*6, "pointrom", 0 ) /* 3d model data */
	ROM_LOAD( "rr1potl0.5b", 0x80000*0, 0x80000,CRC(3ac193e3) SHA1(ff213766f15e34dc1b25187b57d94e17930090a3) )
	ROM_LOAD( "rr1potl1.4b", 0x80000*1, 0x80000,CRC(ac3ffba5) SHA1(4eb4dda5faeff237e0d35725b56d309948fba900) )
	ROM_LOAD( "rr1potm0.5c", 0x80000*2, 0x80000,CRC(42a3fa08) SHA1(15db0ae7ccf7f5a77b9dd9a9d82a488b67f8aaff) )
	ROM_LOAD( "rr1potm1.4c", 0x80000*3, 0x80000,CRC(1bc1ea7b) SHA1(52c21eef4989c45acc5fa4deda2d0b63214731c8) )
	ROM_LOAD( "rr1potu0.5d", 0x80000*4, 0x80000,CRC(5e367f72) SHA1(5887f011379dce865fef238b402678a3d2033de9) )
	ROM_LOAD( "rr1potu1.4d", 0x80000*5, 0x80000,CRC(31d92475) SHA1(51d3c0baa223e1bc16ea2950f2e085597528f870) )

	ROM_REGION( 0x800000, "c352", 0 ) /* sound samples */
	ROM_LOAD( "rr1wav0.10r", 0x100000*0, 0x100000,CRC(a8e85bde) SHA1(b56677e9f6c98f7b600043f5dcfef3a482ca7455) )
	ROM_LOAD( "rr1wav1.10p", 0x100000*2, 0x100000,CRC(35f47c8e) SHA1(7c3f9e942f532af8008fbead2a96fee6084bcde6) )
	ROM_LOAD( "rr1wav2.10n", 0x100000*1, 0x100000,CRC(3244cb59) SHA1(b3283b30cfafbfdcbc6d482ecc4ed6a47a527ca4) )
	ROM_LOAD( "rr1wav3.10l", 0x100000*3, 0x100000,CRC(c4cda1a7) SHA1(60bc96880ec79efdff3cc70c09e848692a40bea4) )

	ROM_REGION( 0x300, "user1", 0 )
	ROM_LOAD( "rr1gam.2d",   0x0000, 0x0100, CRC(b2161bce) SHA1(d2681cc0cf8e68df0d942d392b4eb4458c4bb356) )
	ROM_LOAD( "rr1gam.3d",   0x0100, 0x0100, CRC(b2161bce) SHA1(d2681cc0cf8e68df0d942d392b4eb4458c4bb356) )
	ROM_LOAD( "rr1gam.4d",   0x0200, 0x0100, CRC(b2161bce) SHA1(d2681cc0cf8e68df0d942d392b4eb4458c4bb356) )

	ROM_REGION( 0x2000, "user2", ROMREGION_ERASE00 )
ROM_END

ROM_START( ridgerac3 )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* main program */
        ROM_LOAD32_BYTE( "rr3prgll-3s.4d", 0x000003, 0x080000, CRC(2c3d8cb7) SHA1(46a7b62938fe3edde5c52ce3fdfe447000cd6af0) )
        ROM_LOAD32_BYTE( "rr3prglm-3s.2d", 0x000002, 0x080000, CRC(b15343f2) SHA1(3056eb5a3036a74b2ac641a4c3221986c0be1e27) )
        ROM_LOAD32_BYTE( "rr3prgum-3s.8d", 0x000001, 0x080000, CRC(8fda06ac) SHA1(7e9adba198eb0941100cda64ecedac504f6ac696) )
        ROM_LOAD32_BYTE( "rr3prguu-3s.6d", 0x000000, 0x080000, CRC(868398df) SHA1(422e0f9884904b0df93fcacd1468b8da0458eb8e) )

	ROM_REGION( 0x10000*2, "master", 0 ) /* Master DSP */
	ROM_LOAD16_WORD( "c71.bin", 0,0x1000*2, CRC(47c623ab) SHA1(e363ac50f5556f83308d4cc191b455e9b62bcfc8) )

	ROM_REGION( 0x10000*2, "slave", 0 ) /* Slave DSP */
	ROM_LOAD16_WORD( "c71.bin", 0,0x1000*2, CRC(47c623ab) SHA1(e363ac50f5556f83308d4cc191b455e9b62bcfc8) )

	ROM_REGION( 0x080000, "mcu", 0 ) /* BIOS */
	ROM_LOAD( "c74.bin", 0x0000, 0x4000, CRC(a3dce360) SHA1(8f3248b1890abb2e649927240ae46f73bb171e3b) )

	ROM_REGION( 0x100000, "user4", 0 ) /* sound data */
	ROM_LOAD( "rr1data.6r", 0, 0x080000, CRC(18f5f748) SHA1(e0d149a66de36156edd9b55f604c9a9801aaefa8) )

	ROM_REGION( 0x200000*8, "textile", 0) /* 16x16x8bpp texture tiles */
	ROM_LOAD( "rr1cg0.bin", 0x200000*0x4, 0x200000, CRC(b557a795) SHA1(f345486ffbe797246ad80a55d3c4a332ed6e2888) )//,CRC(d1b0eec6) SHA1(f66922c324dfc3ff408db7556c587ef90ca64c3b) )
	ROM_LOAD( "rr1cg1.bin", 0x200000*0x5, 0x200000, CRC(0fa212d9) SHA1(a1311de0a504e2d399044fa8ac32ec6c56ec965f) )//,CRC(bb695d89) SHA1(557bac9d2718519c1f69e374d0ef9a86a43fe86c) )
	ROM_LOAD( "rr1cg2.bin", 0x200000*0x6, 0x200000, CRC(18e2d2bd) SHA1(69c2ea62eeb255f27d3c69373f6716b0a34683cc) )//,CRC(8f374c0a) SHA1(94ff8581de11a03ef86525155f8433bf5858b980) )
	ROM_LOAD( "rr1cg3.bin", 0x200000*0x7, 0x200000, CRC(9564488b) SHA1(6b27d1aea75d6be747c62e165cfa49ecc5d9e767) )//,CRC(072a5c47) SHA1(86b8e973ae6b78197d685fe6d14722d8e2d0dfec) )

	ROM_REGION16_LE( 0x280000, "textilemap", 0 ) /* texture tilemap */
	ROM_LOAD( "rr1ccrl.bin",0x000000, 0x200000, CRC(6092d181) SHA1(52c0e3ac20aa23059a87d1a985d24ae641577310) )//,CRC(c15cb257) SHA1(0cb8f231c62ea37955be5d452a436a6e815af8e8) )
	ROM_LOAD( "rr1ccrh.bin",0x200000, 0x080000, CRC(dd332fd5) SHA1(a7d9c1d6b5a8e3a937b525c1363880e404dcd147) )//,CRC(dd332fd5) SHA1(a7d9c1d6b5a8e3a937b525c1363880e404dcd147) )

	ROM_REGION( 0x80000*6, "pointrom", 0 ) /* 3d model data */
	ROM_LOAD( "rr1potl0.5b", 0x80000*0, 0x80000,CRC(3ac193e3) SHA1(ff213766f15e34dc1b25187b57d94e17930090a3) )
	ROM_LOAD( "rr1potl1.4b", 0x80000*1, 0x80000,CRC(ac3ffba5) SHA1(4eb4dda5faeff237e0d35725b56d309948fba900) )
	ROM_LOAD( "rr1potm0.5c", 0x80000*2, 0x80000,CRC(42a3fa08) SHA1(15db0ae7ccf7f5a77b9dd9a9d82a488b67f8aaff) )
	ROM_LOAD( "rr1potm1.4c", 0x80000*3, 0x80000,CRC(1bc1ea7b) SHA1(52c21eef4989c45acc5fa4deda2d0b63214731c8) )
	ROM_LOAD( "rr1potu0.5d", 0x80000*4, 0x80000,CRC(5e367f72) SHA1(5887f011379dce865fef238b402678a3d2033de9) )
	ROM_LOAD( "rr1potu1.4d", 0x80000*5, 0x80000,CRC(31d92475) SHA1(51d3c0baa223e1bc16ea2950f2e085597528f870) )

	ROM_REGION( 0x800000, "c352", 0 ) /* sound samples */
	ROM_LOAD( "rr1wav0.10r", 0x100000*0, 0x100000,CRC(a8e85bde) SHA1(b56677e9f6c98f7b600043f5dcfef3a482ca7455) )
	ROM_LOAD( "rr1wav1.10p", 0x100000*2, 0x100000,CRC(35f47c8e) SHA1(7c3f9e942f532af8008fbead2a96fee6084bcde6) )
	ROM_LOAD( "rr1wav2.10n", 0x100000*1, 0x100000,CRC(3244cb59) SHA1(b3283b30cfafbfdcbc6d482ecc4ed6a47a527ca4) )
	ROM_LOAD( "rr1wav3.10l", 0x100000*3, 0x100000,CRC(c4cda1a7) SHA1(60bc96880ec79efdff3cc70c09e848692a40bea4) )

	ROM_REGION( 0x300, "user1", 0 )
	ROM_LOAD( "rr1gam.2d",   0x0000, 0x0100, CRC(b2161bce) SHA1(d2681cc0cf8e68df0d942d392b4eb4458c4bb356) )
	ROM_LOAD( "rr1gam.3d",   0x0100, 0x0100, CRC(b2161bce) SHA1(d2681cc0cf8e68df0d942d392b4eb4458c4bb356) )
	ROM_LOAD( "rr1gam.4d",   0x0200, 0x0100, CRC(b2161bce) SHA1(d2681cc0cf8e68df0d942d392b4eb4458c4bb356) )

	ROM_REGION( 0x2000, "user2", ROMREGION_ERASE00 )
ROM_END


ROM_START( rrf )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* main program */
	ROM_LOAD32_BYTE( "rrf2prgll.4d", 0x00003, 0x80000, CRC(23c6144d) SHA1(99f70e2c60fba7551cafdce12b07da1f8ab8aad6) )
	ROM_LOAD32_BYTE( "rrf2prglm.2d", 0x00002, 0x80000, CRC(1ad638a1) SHA1(505a7f4ba60bbc4e735865fbc5d664311b6045d9) )
	ROM_LOAD32_BYTE( "rrf2prgum.8d", 0x00001, 0x80000, CRC(d7e0aa16) SHA1(cab4578cdd3af84b865114be4105cfdc2e7abf36) )
	ROM_LOAD32_BYTE( "rrf2prguu.6d", 0x00000, 0x80000, CRC(12c808bb) SHA1(64e84686d4ceb8145b9a59b75d0dced830884c9d) )

	ROM_REGION( 0x10000*2, "master", 0 ) /* Master DSP */
	ROM_LOAD16_WORD( "c71.bin", 0,0x1000*2, CRC(47c623ab) SHA1(e363ac50f5556f83308d4cc191b455e9b62bcfc8) )

	ROM_REGION( 0x10000*2, "slave", 0 ) /* Slave DSP */
	ROM_LOAD16_WORD( "c71.bin", 0,0x1000*2, CRC(47c623ab) SHA1(e363ac50f5556f83308d4cc191b455e9b62bcfc8) )

	ROM_REGION( 0x080000, "mcu", 0 ) /* BIOS */
	ROM_LOAD( "c74.bin", 0x0000, 0x4000, CRC(a3dce360) SHA1(8f3248b1890abb2e649927240ae46f73bb171e3b) )

	ROM_REGION( 0x100000, "user4", 0 ) /* sound data */
	ROM_LOAD( "rrf1data.6r", 0, 0x080000, CRC(ce3c6ed6) SHA1(23e033364bc967c10c49fd1d5413dda837670633) )


	ROM_REGION( 0x80000*6, "pointrom", 0 ) /* 3d model data */
	ROM_LOAD( "rrf2potl0.l0", 0x80000*0, 0x80000, CRC(9b762e60) SHA1(50b67ff6678bacba140bad3aedda75c30851fa7a) )
	ROM_LOAD( "rrf2potl1.l1", 0x80000*1, 0x80000, CRC(ab4d66b0) SHA1(59020b3dc2efff99cd528752ca7168b64ae96ac4) )
	ROM_LOAD( "rrf2potm0.m0", 0x80000*2, 0x80000, CRC(02d4daa3) SHA1(9b4e48d3234cb91146b5d31ffcf42ad199ccb903) )
	ROM_LOAD( "rrf2potm1.m1", 0x80000*3, 0x80000, CRC(37e005c2) SHA1(036ae1c44aaa6a6904e6d4938572035ccc6854ed) )
	ROM_LOAD( "rrf2potu0.u0", 0x80000*4, 0x80000, CRC(86b3fe98) SHA1(f242d33f7488e233ccdc0b5d309c64510d7a622d) )
	ROM_LOAD( "rrf2potu1.u1", 0x80000*5, 0x80000, CRC(e0c6ce3d) SHA1(cc559a2237ccb753cb1397fecba64733455a8c43) )

	ROM_REGION( 0x800000, "c352", 0 ) /* sound samples */
	ROM_LOAD( "rrf1wave0.10r", 0x100000*0, 0x100000,CRC(a8e85bde) SHA1(b56677e9f6c98f7b600043f5dcfef3a482ca7455) )
	ROM_LOAD( "rrf1wave1.10p", 0x100000*2, 0x100000,CRC(35f47c8e) SHA1(7c3f9e942f532af8008fbead2a96fee6084bcde6) )
	ROM_LOAD( "rrf1wave2.10n", 0x100000*1, 0x100000,CRC(4ceeae12) SHA1(ae3a6583f8912bc784c7bc63d32448228cf217ba) ) // differs from normal sets
	ROM_LOAD( "rrf1wave3.10l", 0x100000*3, 0x100000,CRC(c4cda1a7) SHA1(60bc96880ec79efdff3cc70c09e848692a40bea4) )

	ROM_REGION( 0x2000, "user2", ROMREGION_ERASE00 )

	/* this stuff was missing from this version, and shouldn't be the same (bad textures if we use these roms) */
	ROM_REGION( 0x300, "user1", 0 )
	ROM_LOAD( "rrf1gam.2d",   0x0000, 0x0100, BAD_DUMP CRC(b2161bce) SHA1(d2681cc0cf8e68df0d942d392b4eb4458c4bb356) )
	ROM_LOAD( "rrf1gam.3d",   0x0100, 0x0100, BAD_DUMP CRC(b2161bce) SHA1(d2681cc0cf8e68df0d942d392b4eb4458c4bb356) )
	ROM_LOAD( "rrf1gam.4d",   0x0200, 0x0100, BAD_DUMP CRC(b2161bce) SHA1(d2681cc0cf8e68df0d942d392b4eb4458c4bb356) )

	ROM_REGION( 0x200000*8, "textile", 0) /* 16x16x8bpp texture tiles */
	ROM_LOAD( "rrf1cg0.bin", 0x200000*0x4, 0x200000, BAD_DUMP CRC(b557a795) SHA1(f345486ffbe797246ad80a55d3c4a332ed6e2888) )
	ROM_LOAD( "rrf1cg1.bin", 0x200000*0x5, 0x200000, BAD_DUMP CRC(0fa212d9) SHA1(a1311de0a504e2d399044fa8ac32ec6c56ec965f) )
	ROM_LOAD( "rrf1cg2.bin", 0x200000*0x6, 0x200000, BAD_DUMP CRC(18e2d2bd) SHA1(69c2ea62eeb255f27d3c69373f6716b0a34683cc) )
	ROM_LOAD( "rrf1cg3.bin", 0x200000*0x7, 0x200000, BAD_DUMP CRC(9564488b) SHA1(6b27d1aea75d6be747c62e165cfa49ecc5d9e767) )

	ROM_REGION16_LE( 0x280000, "textilemap", 0 ) /* texture tilemap */
	ROM_LOAD( "rrf1ccrl.bin",0x000000, 0x200000, BAD_DUMP CRC(6092d181) SHA1(52c0e3ac20aa23059a87d1a985d24ae641577310) )
	ROM_LOAD( "rrf1ccrh.bin",0x200000, 0x080000, BAD_DUMP CRC(dd332fd5) SHA1(a7d9c1d6b5a8e3a937b525c1363880e404dcd147) )
ROM_END

ROM_START( timecris )
	ROM_REGION( 0x400000, "maincpu", 0 ) /* main program */
	ROM_LOAD32_BYTE( "ts2ver-b.1", 0x00003, 0x100000, CRC(29b377f7) SHA1(21864ba964602115c1268fd5edd8006a13a86cfc) )
	ROM_LOAD32_BYTE( "ts2ver-b.2", 0x00002, 0x100000, CRC(79512e25) SHA1(137a215ec192e76e93511456ad504481a566c9c9) )
	ROM_LOAD32_BYTE( "ts2ver-b.3", 0x00001, 0x100000, CRC(9f4ced33) SHA1(32768b5ff263a9e3d11b7b36f6b2d7e951e07419) )
	ROM_LOAD32_BYTE( "ts2ver-b.4", 0x00000, 0x100000, CRC(3e0cfb38) SHA1(3c56342bd73b1617ea579a0d53e19d59bb04fd99) )

	ROM_REGION( 0x10000*2, "master", 0 ) /* Master DSP */
	ROM_LOAD16_WORD( "c71.bin", 0,0x1000*2, CRC(47c623ab) SHA1(e363ac50f5556f83308d4cc191b455e9b62bcfc8) )

	ROM_REGION( 0x10000*2, "slave", 0 ) /* Slave DSP */
	ROM_LOAD16_WORD( "c71.bin", 0,0x1000*2, CRC(47c623ab) SHA1(e363ac50f5556f83308d4cc191b455e9b62bcfc8) )

	ROM_REGION( 0x80000, "mcu", ROMREGION_ERASE00 ) /* BIOS */

	ROM_REGION16_LE( 0x080000, "user4", 0 ) /* MCU BIOS */
	ROM_LOAD( "ts1data.8k", 0, 0x080000, CRC(e68aa973) SHA1(663e80d249be5d5841139d98a9d72e2396851272) )

	ROM_REGION( 0xc00000, "sprite", 0 ) /* 32x32x8bpp sprite tiles */
	ROM_LOAD( "ts1scg0.12f",0x200000*0, 0x200000,CRC(14a3674d) SHA1(c5792a385572452b43bbc7eb8428335b19daa3c0) )
	ROM_LOAD( "ts1scg1.10f",0x200000*1, 0x200000,CRC(11791dbf) SHA1(3d75b468d69a8bf398d45f310cdb8bc88b63f25c) )
	ROM_LOAD( "ts1scg2.8f", 0x200000*2, 0x200000,CRC(d630fff9) SHA1(691394027b858702f06282f965f5b53e6fed496b) )
	ROM_LOAD( "ts1scg3.7f", 0x200000*3, 0x200000,CRC(1a62f015) SHA1(7d09ae480ae7813391616ae0090929ba845a345a) )
	ROM_LOAD( "ts1scg4.5f", 0x200000*4, 0x200000,CRC(511b8dd6) SHA1(936649c0a61d29f024a28e4ab64cce4b55d58f64) )
	ROM_LOAD( "ts1scg5.3f", 0x200000*5, 0x200000,CRC(553bb246) SHA1(94659bee4fd0afe834a8bf3414d8825411cf9e86) )

	ROM_REGION( 0x200000*8, "textile", 0) /* 16x16x8bpp texture tiles */
	ROM_LOAD( "ts1cg0.8d",   0x200000*0x0, 0x200000,CRC(de07b22c) SHA1(f4d07b8840ec8be625eff634bce619e960c334a5) )
	ROM_LOAD( "ts1cg1.10d",  0x200000*0x1, 0x200000,CRC(992d26f6) SHA1(a0b9007312804b413d4c1748527378da4d8d53b3) )
	ROM_LOAD( "ts1cg2.12d",  0x200000*0x2, 0x200000,CRC(6273954f) SHA1(d73a43888b53e4c42fc33e8e1b38e60fd3329413) )
	ROM_LOAD( "ts1cg3.13d",  0x200000*0x3, 0x200000,CRC(38171f24) SHA1(d04caaa5b1b377ced38501b014e4cb7fc831c41d) )
	ROM_LOAD( "ts1cg4.14d",  0x200000*0x4, 0x200000,CRC(51f09856) SHA1(0eef421907ee813d5117e62cf0005bf00eb29c88) )
	ROM_LOAD( "ts1cg5.16d",  0x200000*0x5, 0x200000,CRC(4cd9fd79) SHA1(0d2018ec914683a75bdec8655d678fd562eb6d15) )
	ROM_LOAD( "ts1cg6.18d",  0x200000*0x6, 0x200000,CRC(f17f2ec9) SHA1(ed88ec524626e5bbe2e1ea6838412d3ac85671dd) )

	ROM_REGION16_LE( 0x280000, "textilemap", 0 ) /* texture tilemap */
	ROM_LOAD( "ts1ccrl.3d",	 0x000000, 0x200000,CRC(56cad2df) SHA1(49c0e57d5cf5d5fc4c75da6969bec01d6d443259) )
	ROM_LOAD( "ts1ccrh.1d",	 0x200000, 0x080000,CRC(a1cc3741) SHA1(7fe57924c42e287b134e5d7ad00cffdff1f18084) )

	ROM_REGION( 0x80000*9, "pointrom", 0 ) /* 3d model data */
	ROM_LOAD( "ts1ptrl0.18k", 0x80000*0, 0x80000,CRC(e5f2d275) SHA1(2f5057e65ec8a3ec03f841f15f10769ae1f69139) )
	ROM_LOAD( "ts1ptrl1.16k", 0x80000*1, 0x80000,CRC(2bba3800) SHA1(1d9c944cb06417cb0ac47a58b922dddb83387586) )
	ROM_LOAD( "ts1ptrl2.15k", 0x80000*2, 0x80000,CRC(d4441c08) SHA1(6a6bb9cecbf35cb81b7681e220fc33df9a01d07f) )
	ROM_LOAD( "ts1ptrm0.18j", 0x80000*3, 0x80000,CRC(8aea02ba) SHA1(44ba85ba6d59758448d17ec39dfb628882ddc684) )
	ROM_LOAD( "ts1ptrm1.16j", 0x80000*4, 0x80000,CRC(bccf19bc) SHA1(4a6566948bdd2b0f82b7c30e57d3fe65005c26e3) )
	ROM_LOAD( "ts1ptrm2.15j", 0x80000*5, 0x80000,CRC(7280be31) SHA1(476b7171ae855d8bbd968ccbaa55b5100d274e3b) )
	ROM_LOAD( "ts1ptru0.18f", 0x80000*6, 0x80000,CRC(c30d6332) SHA1(a5c59d0abfe38de975fa0d606ed8500eb02008b7) )
	ROM_LOAD( "ts1ptru1.16f", 0x80000*7, 0x80000,CRC(993cde84) SHA1(c9cdcca1d60bcc41ad881c02dda9895563963ead) )
	ROM_LOAD( "ts1ptru2.15f", 0x80000*8, 0x80000,CRC(7cb25c73) SHA1(616eab3ac238864a584394f7ec8736ece227974a) )

	ROM_REGION( 0xa00000, "c352", 0 ) /* sound samples */
	ROM_LOAD( "ts1wavea.2l", 0x000000, 0x400000, CRC(d1123301) SHA1(4bf1fd746fef4e6befa63c61a761971d729e1573) )
	ROM_LOAD( "ts1waveb.1l", 0x800000, 0x200000, CRC(bf4d7272) SHA1(c7c7b3620e7b3176644b6784ee36e679c9e31cc1) )
ROM_END

ROM_START( timecrisa )
	ROM_REGION( 0x400000, "maincpu", 0 ) /* main program */
	ROM_LOAD32_WORD_SWAP( "ts2ver-a.1", 0x00002, 0x200000, CRC(d57eb74b) SHA1(536dd9305d0ac44110c575776333310cc57b5242) )
	ROM_LOAD32_WORD_SWAP( "ts2ver-a.2", 0x00000, 0x200000, CRC(671588af) SHA1(63f992c6795521fd263a0ebf230f8dc88cbfc443) )

	ROM_REGION( 0x10000*2, "master", 0 ) /* Master DSP */
	ROM_LOAD16_WORD( "c71.bin", 0,0x1000*2, CRC(47c623ab) SHA1(e363ac50f5556f83308d4cc191b455e9b62bcfc8) )

	ROM_REGION( 0x10000*2, "slave", 0 ) /* Slave DSP */
	ROM_LOAD16_WORD( "c71.bin", 0,0x1000*2, CRC(47c623ab) SHA1(e363ac50f5556f83308d4cc191b455e9b62bcfc8) )

	ROM_REGION( 0x80000, "mcu", ROMREGION_ERASE00 ) /* BIOS */

	ROM_REGION16_LE( 0x080000, "user4", 0 ) /* MCU BIOS */
	ROM_LOAD( "ts1data.8k", 0, 0x080000, CRC(e68aa973) SHA1(663e80d249be5d5841139d98a9d72e2396851272) )

    ROM_REGION( 0x200000*6, "sprite", 0 ) /* 32x32x8bpp sprite tiles */
	ROM_LOAD( "ts1scg0.12f",0x200000*0, 0x200000,CRC(14a3674d) SHA1(c5792a385572452b43bbc7eb8428335b19daa3c0) )
	ROM_LOAD( "ts1scg1.10f",0x200000*1, 0x200000,CRC(11791dbf) SHA1(3d75b468d69a8bf398d45f310cdb8bc88b63f25c) )
	ROM_LOAD( "ts1scg2.8f", 0x200000*2, 0x200000,CRC(d630fff9) SHA1(691394027b858702f06282f965f5b53e6fed496b) )
	ROM_LOAD( "ts1scg3.7f", 0x200000*3, 0x200000,CRC(1a62f015) SHA1(7d09ae480ae7813391616ae0090929ba845a345a) )
	ROM_LOAD( "ts1scg4.5f", 0x200000*4, 0x200000,CRC(511b8dd6) SHA1(936649c0a61d29f024a28e4ab64cce4b55d58f64) )
	ROM_LOAD( "ts1scg5.3f", 0x200000*5, 0x200000,CRC(553bb246) SHA1(94659bee4fd0afe834a8bf3414d8825411cf9e86) )

	ROM_REGION( 0x200000*8, "textile", 0) /* 16x16x8bpp texture tiles */
	ROM_LOAD( "ts1cg0.8d",   0x200000*0x0, 0x200000,CRC(de07b22c) SHA1(f4d07b8840ec8be625eff634bce619e960c334a5) )
	ROM_LOAD( "ts1cg1.10d",  0x200000*0x1, 0x200000,CRC(992d26f6) SHA1(a0b9007312804b413d4c1748527378da4d8d53b3) )
	ROM_LOAD( "ts1cg2.12d",  0x200000*0x2, 0x200000,CRC(6273954f) SHA1(d73a43888b53e4c42fc33e8e1b38e60fd3329413) )
	ROM_LOAD( "ts1cg3.13d",  0x200000*0x3, 0x200000,CRC(38171f24) SHA1(d04caaa5b1b377ced38501b014e4cb7fc831c41d) )
	ROM_LOAD( "ts1cg4.14d",  0x200000*0x4, 0x200000,CRC(51f09856) SHA1(0eef421907ee813d5117e62cf0005bf00eb29c88) )
	ROM_LOAD( "ts1cg5.16d",  0x200000*0x5, 0x200000,CRC(4cd9fd79) SHA1(0d2018ec914683a75bdec8655d678fd562eb6d15) )
	ROM_LOAD( "ts1cg6.18d",  0x200000*0x6, 0x200000,CRC(f17f2ec9) SHA1(ed88ec524626e5bbe2e1ea6838412d3ac85671dd) )

	ROM_REGION16_LE( 0x280000, "textilemap", 0 ) /* texture tilemap */
	ROM_LOAD( "ts1ccrl.3d",	 0x000000, 0x200000,CRC(56cad2df) SHA1(49c0e57d5cf5d5fc4c75da6969bec01d6d443259) )
	ROM_LOAD( "ts1ccrh.1d",	 0x200000, 0x080000,CRC(a1cc3741) SHA1(7fe57924c42e287b134e5d7ad00cffdff1f18084) )

	ROM_REGION( 0x80000*9, "pointrom", 0 ) /* 3d model data */
	ROM_LOAD( "ts1ptrl0.18k", 0x80000*0, 0x80000,CRC(e5f2d275) SHA1(2f5057e65ec8a3ec03f841f15f10769ae1f69139) )
	ROM_LOAD( "ts1ptrl1.16k", 0x80000*1, 0x80000,CRC(2bba3800) SHA1(1d9c944cb06417cb0ac47a58b922dddb83387586) )
	ROM_LOAD( "ts1ptrl2.15k", 0x80000*2, 0x80000,CRC(d4441c08) SHA1(6a6bb9cecbf35cb81b7681e220fc33df9a01d07f) )
	ROM_LOAD( "ts1ptrm0.18j", 0x80000*3, 0x80000,CRC(8aea02ba) SHA1(44ba85ba6d59758448d17ec39dfb628882ddc684) )
	ROM_LOAD( "ts1ptrm1.16j", 0x80000*4, 0x80000,CRC(bccf19bc) SHA1(4a6566948bdd2b0f82b7c30e57d3fe65005c26e3) )
	ROM_LOAD( "ts1ptrm2.15j", 0x80000*5, 0x80000,CRC(7280be31) SHA1(476b7171ae855d8bbd968ccbaa55b5100d274e3b) )
	ROM_LOAD( "ts1ptru0.18f", 0x80000*6, 0x80000,CRC(c30d6332) SHA1(a5c59d0abfe38de975fa0d606ed8500eb02008b7) )
	ROM_LOAD( "ts1ptru1.16f", 0x80000*7, 0x80000,CRC(993cde84) SHA1(c9cdcca1d60bcc41ad881c02dda9895563963ead) )
	ROM_LOAD( "ts1ptru2.15f", 0x80000*8, 0x80000,CRC(7cb25c73) SHA1(616eab3ac238864a584394f7ec8736ece227974a) )

	ROM_REGION( 0xa00000, "c352", 0 ) /* sound samples */
	ROM_LOAD( "ts1wavea.2l", 0x000000, 0x400000, CRC(d1123301) SHA1(4bf1fd746fef4e6befa63c61a761971d729e1573) )
	ROM_LOAD( "ts1waveb.1l", 0x800000, 0x200000, CRC(bf4d7272) SHA1(c7c7b3620e7b3176644b6784ee36e679c9e31cc1) )
ROM_END

ROM_START( tokyowar )
	ROM_REGION( 0x400000, "maincpu", 0 ) /* main program */
        ROM_LOAD32_BYTE( "tw2ver-a.1",   0x000003, 0x100000, CRC(2b17ca92) SHA1(7bcb1658059c59fb1a0131a41ede7157855130a8) )
        ROM_LOAD32_BYTE( "tw2ver-a.2",   0x000002, 0x100000, CRC(12da84e3) SHA1(a9406d0b77f60ba930c30e60bf4b3656c8905585) )
        ROM_LOAD32_BYTE( "tw2ver-a.3",   0x000001, 0x100000, CRC(7d42c516) SHA1(28c1596dd55c15207bbb41a8b9a5abc97abc2bc8) )
        ROM_LOAD32_BYTE( "tw2ver-a.4",   0x000000, 0x100000, CRC(b904ed16) SHA1(773e11536e1b3fe4971608a63a8e6eca702f8667) )

	ROM_REGION( 0x10000*2, "master", 0 ) /* Master DSP */
	ROM_LOAD16_WORD( "c71.bin", 0,0x1000*2, CRC(47c623ab) SHA1(e363ac50f5556f83308d4cc191b455e9b62bcfc8) )

	ROM_REGION( 0x10000*2, "slave", 0 ) /* Slave DSP */
	ROM_LOAD16_WORD( "c71.bin", 0,0x1000*2, CRC(47c623ab) SHA1(e363ac50f5556f83308d4cc191b455e9b62bcfc8) )

	ROM_REGION( 0x80000, "mcu", ROMREGION_ERASE00 ) /* BIOS */

	ROM_REGION16_LE( 0x080000, "user4", 0 ) /* MCU BIOS */
        ROM_LOAD( "tw1data.8k",   0x000000, 0x080000, CRC(bd046e4b) SHA1(162bc4ab69959ccab49fd69de291d34d472fb1c8) )

	ROM_REGION( 0x800000, "sprite", 0 ) /* 32x32x8bpp sprite tiles */
        ROM_LOAD( "tw1scg0.12f",  0x000000, 0x200000, CRC(e3ec4daa) SHA1(f3a71ae9820d62075b814ffa2fecf3343ae09ffe) )
        ROM_LOAD( "tw1scg1.10f",  0x200000, 0x200000, CRC(b18a06e9) SHA1(ecf6a1e11603b8ea5119a036a57595dea021d778) )
        ROM_LOAD( "tw1scg2.8f",   0x400000, 0x200000, CRC(36f8c3d8) SHA1(d5c965d5cdd258c77b9db3137ce33404a5a3641c) )
        ROM_LOAD( "tw1scg3.7f",   0x600000, 0x200000, CRC(8e14d013) SHA1(ca63105a5c07bb9653499eef7a757db52612b59b) )

	ROM_REGION( 0x200000*8, "textile", 0) /* 16x16x8bpp texture tiles */
        ROM_LOAD( "tw1cg0.8d",    0x000000, 0x200000, CRC(98b9b070) SHA1(cb920a34700dab330c967cc634717134c04b7e1d) )
        ROM_LOAD( "tw1cg1.10d",   0x200000, 0x200000, CRC(f96a723a) SHA1(5ba14963a4c51c875ac8d3b42049bc334de90038) )
        ROM_LOAD( "tw1cg2.12d",   0x400000, 0x200000, CRC(573e9ded) SHA1(815bda1ac000532c915c2d65ffdb04fee6fa8201) )
        ROM_LOAD( "tw1cg3.13d",   0x600000, 0x200000, CRC(302d5c74) SHA1(5a823f6842cf0f79eb93da47d5bf8c5f51e420db) )
        ROM_LOAD( "tw1cg4.14d",   0x800000, 0x200000, CRC(ab8aa1df) SHA1(355192d999f493e0761fbc822fa9b30c33d8e1c4) )
        ROM_LOAD( "tw1cg5.16d",   0xa00000, 0x200000, CRC(5063f3d0) SHA1(ad8dd2f4184373a3a3ca748b411a5eec1835dc97) )
        ROM_LOAD( "tw1cg6.18d",   0xc00000, 0x200000, CRC(d764027c) SHA1(5cbf93392683885c220628936ba50c09cb40fcfb) )
        ROM_LOAD( "tw1cg7.19d",   0xe00000, 0x200000, CRC(8d89877e) SHA1(7d76d48d64d7ac5411d714a4bb83f37e3e5b8df6) )

	ROM_REGION16_LE( 0x280000, "textilemap", 0 ) /* texture tilemap */
        ROM_LOAD( "tw1ccrl.3d",   0x000000, 0x200000, CRC(d08f5794) SHA1(336a97a2b060505e259e3bcedb9eb8aa4ea8815e) )
        ROM_LOAD( "tw1ccrh.1d",   0x200000, 0x080000, CRC(ad17e693) SHA1(4f06dc82c03159894fb8e10383862920f94563b1) )

	ROM_REGION( 0x600000, "pointrom", 0 ) /* 3d model data */
        ROM_LOAD( "tw1ptrl0.18k", 0x000000, 0x080000, CRC(44ac5e86) SHA1(5e42db58f2e352c0fe5e49588a9283817dd15ab1) )
        ROM_LOAD( "tw1ptrl1.16k", 0x080000, 0x080000, CRC(3c769860) SHA1(19b32c3d262f2a9d07354fbd6ac6be97b05f176e) )
        ROM_LOAD( "tw1ptrl2.15k", 0x100000, 0x080000, CRC(6e94103c) SHA1(ee90b77939a9f5780ce271882133cdf977eb643e) )
        ROM_LOAD( "tw1ptrl3.14k", 0x180000, 0x080000, CRC(e3ce5eb2) SHA1(4e15a6f630be15eb017cd51c5e7901db3138a061) )
        ROM_LOAD( "tw1ptrm0.18j", 0x200000, 0x080000, CRC(e170cea2) SHA1(ca259508d76fdab97a9d0502f871f0e560e6c308) )
        ROM_LOAD( "tw1ptrm1.16j", 0x280000, 0x080000, CRC(36a32237) SHA1(f46851dc5c094810ddc42d56310a9f85908bf715) )
        ROM_LOAD( "tw1ptrm2.15j", 0x300000, 0x080000, CRC(c426c278) SHA1(64232ac3c1649e0d0adb4b03e58b8b5ea4013f83) )
        ROM_LOAD( "tw1ptrm3.14j", 0x380000, 0x080000, CRC(d9b9a651) SHA1(0c49a051526081149d894d629f19cb0f2b66a698) )
        ROM_LOAD( "tw1ptru0.18f", 0x400000, 0x080000, CRC(62a9e9fb) SHA1(24739adba029b0acf2d7078962c9d01098a29a6c) )
        ROM_LOAD( "tw1ptru1.16f", 0x480000, 0x080000, CRC(2fd36177) SHA1(368b915261d914be01ae9daeb52571bead52d14d) )
        ROM_LOAD( "tw1ptru2.15f", 0x500000, 0x080000, CRC(ceacb1c9) SHA1(b86cf576e16bbe26ad0d6d6df8bf28d0071c25e2) )
        ROM_LOAD( "tw1ptru3.14f", 0x580000, 0x080000, CRC(939044c2) SHA1(f4c1c0a1c2f07ca7f784d59ef4162a2a6a8bbc43) )

	ROM_REGION( 0xa00000, "c352", 0 ) /* sound samples */
        ROM_LOAD( "tw1wavea.2l",  0x000000, 0x400000, CRC(ebce6366) SHA1(44ebe90ff3c7af5bebbf1baba3b7a2b1863daebb) )
ROM_END

ROM_START( dirtdash )
	ROM_REGION( 0x400000, "maincpu", 0 ) /* main program */
        ROM_LOAD32_WORD_SWAP( "dt2vera.1",    0x000002, 0x200000, CRC(402a3d73) SHA1(009b57ed0ea228ccedb139d945b9eaf2a36e2502) )
        ROM_LOAD32_WORD_SWAP( "dt2vera.2",    0x000000, 0x200000, CRC(66ed140d) SHA1(a472fdc7b6aaeb4b3643ecdafd32fa665e7c7aa2) )

	ROM_REGION( 0x10000*2, "master", 0 ) /* Master DSP */
	ROM_LOAD16_WORD( "c71.bin", 0,0x1000*2, CRC(47c623ab) SHA1(e363ac50f5556f83308d4cc191b455e9b62bcfc8) )

	ROM_REGION( 0x10000*2, "slave", 0 ) /* Slave DSP */
	ROM_LOAD16_WORD( "c71.bin", 0,0x1000*2, CRC(47c623ab) SHA1(e363ac50f5556f83308d4cc191b455e9b62bcfc8) )

	ROM_REGION( 0x80000, "mcu", ROMREGION_ERASE00 ) /* BIOS */

	ROM_REGION16_LE( 0x080000, "user4", 0 ) /* MCU BIOS */
        ROM_LOAD( "dt1dataa.8k",  0x000000, 0x080000, CRC(9bcdea21) SHA1(26ae025cf746d3a703a82495eb2bb515b828a650) )

	ROM_REGION( 0x800000, "sprite", 0 ) /* 32x32x8bpp sprite tiles */
        ROM_LOAD( "dt1scg0.12f",  0x000000, 0x200000, CRC(a09b5760) SHA1(3dd54ebebf9da1de76874a1adf491ed15849e1b1) )
        ROM_LOAD( "dt1scg1.10f",  0x200000, 0x200000, CRC(f9ac8111) SHA1(814074ae8cc81c6c1201d764a84dd95fe914f19c) )

	ROM_REGION( 0x200000*8, "textile", 0) /* 16x16x8bpp texture tiles */
        ROM_LOAD( "dt1cg0.8d",    0x000000, 0x200000, CRC(10ab95e0) SHA1(ffde1f00ac3e82a36fbcfa060c6b97c92dfcfc8b) )
        ROM_LOAD( "dt1cg1.10d",   0x200000, 0x200000, CRC(d9f1ba53) SHA1(5a1095b726c55001cc1d4c695adc38097e6a0201) )
        ROM_LOAD( "dt1cg2.12d",   0x400000, 0x200000, CRC(bd8b1e0b) SHA1(fcd94e33a0cbd17c9308cb8952e3c618ab56f9fc) )
        ROM_LOAD( "dt1cg3.13d",   0x600000, 0x200000, CRC(ba960663) SHA1(e98149bc4652ea7933ac47d760a6f7e6489f15e2) )
        ROM_LOAD( "dt1cg4.14d",   0x800000, 0x200000, CRC(424b9652) SHA1(fa8865110db03559740c4e633e123d1a009782c4) )
        ROM_LOAD( "dt1cg5.16d",   0xa00000, 0x200000, CRC(29516626) SHA1(1f12c5dc3975b88dc60d87d0409bf311837e9fa4) )
        ROM_LOAD( "dt1cg6.18d",   0xc00000, 0x200000, CRC(e6fa7180) SHA1(85316cde282cff1f913cf9f155cfa36adcc1108e) )
        ROM_LOAD( "dt1cg7.19d",   0xe00000, 0x200000, CRC(2ca19153) SHA1(c82403c8b40bf85daedf610b1bc7bfea9dfc6206) )

	ROM_REGION16_LE( 0x280000, "textilemap", 0 ) /* texture tilemap */
        ROM_LOAD( "dt1ccrl.3d",   0x000000, 0x200000, CRC(e536b313) SHA1(7357da993d2bb3fcc8c1c2feb53689ad368cd80a) )
        ROM_LOAD( "dt1ccrh.1d",   0x200000, 0x080000, CRC(af257064) SHA1(0da561d9f8824618c00209ccef6146e9f3ad72bb) )

	ROM_REGION( 0x600000, "pointrom", 0 ) /* 3d model data */
        ROM_LOAD( "dt1ptrl0.18k", 0x000000, 0x080000, CRC(4e0cac3a) SHA1(c2778e9e93be2de729c6f118caf62ac9f48efbb0) )
        ROM_LOAD( "dt1ptrl1.16k", 0x080000, 0x080000, CRC(59ba9dba) SHA1(a2e9488cf0ff255284c06a1ef653ae86c0d98adc) )
        ROM_LOAD( "dt1ptrl2.15k", 0x100000, 0x080000, CRC(cfe80c67) SHA1(ba3bc48aa39712e63c915070a76974fbd560dee6) )
        ROM_LOAD( "dt1ptrm0.18j", 0x180000, 0x080000, CRC(41f34337) SHA1(7e624e7b6fdefe156168b1c9cc5e919db3b2fbaa) )
        ROM_LOAD( "dt1ptrm1.16j", 0x200000, 0x080000, CRC(d68541fc) SHA1(6b00079bd8f2db26e71968de289cea93458aebe9) )
        ROM_LOAD( "dt1ptrm2.15j", 0x280000, 0x080000, CRC(71e6714d) SHA1(6aad6db3be5020213d7add61c7d927ae9c4fea4e) )
        ROM_LOAD( "dt1ptru0.18f", 0x300000, 0x080000, CRC(4909bd7d) SHA1(0e4ef3987c43ef0438331b82b50dcc97363a45d0) )
        ROM_LOAD( "dt1ptru1.16f", 0x380000, 0x080000, CRC(4a5097df) SHA1(a9c814b0ed4bd92accd0e57be8e3d887114b06a5) )
        ROM_LOAD( "dt1ptru2.15f", 0x400000, 0x080000, CRC(1171eaf5) SHA1(168365ea619386f218585c49025cdd7fd1224082) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* sound samples */
        ROM_LOAD( "dt1wavea.2l",  0x000000, 0x400000, CRC(cbd52e40) SHA1(dc995dd919548c96a90efb0375e5b5f1055e05cb) )
        ROM_LOAD( "dt1waveb.1l",  0x400000, 0x400000, CRC(6b736f94) SHA1(ac3715480aa9a9c2dec099607f89859bb3b73a6a) )
ROM_END

ROM_START( aquajet )
	ROM_REGION( 0x400000, "maincpu", 0 ) /* main program */
	ROM_LOAD32_BYTE( "aj2ver-b.1",   0x000003, 0x100000, CRC(3a67b9f4) SHA1(8cd51f319e082297fdb99634486fe297a0ace654) )
	ROM_LOAD32_BYTE( "aj2ver-b.2",   0x000002, 0x100000, CRC(f5e8fc96) SHA1(e23fcf6f84724d1de15870ff578ff8a6b26e8f31) )
	ROM_LOAD32_BYTE( "aj2ver-b.3",   0x000001, 0x100000, CRC(ef6ebcf7) SHA1(358973b678b9a3065e945fb589af16e8102d437b) )
	ROM_LOAD32_BYTE( "aj2ver-b.4",   0x000000, 0x100000, CRC(7799b909) SHA1(e40005f96f51742b2778605926b8184c9b2c1ad2) )

	ROM_REGION( 0x10000*2, "master", 0 ) /* Master DSP */
	ROM_LOAD16_WORD( "c71.bin", 0,0x1000*2, CRC(47c623ab) SHA1(e363ac50f5556f83308d4cc191b455e9b62bcfc8) )

	ROM_REGION( 0x10000*2, "slave", 0 ) /* Slave DSP */
	ROM_LOAD16_WORD( "c71.bin", 0,0x1000*2, CRC(47c623ab) SHA1(e363ac50f5556f83308d4cc191b455e9b62bcfc8) )

	ROM_REGION( 0x80000, "mcu", ROMREGION_ERASE00 ) /* BIOS */

	ROM_REGION16_LE( 0x080000, "user4", 0 ) /* MCU BIOS */
        ROM_LOAD( "aj1data.8k",   0x000000, 0x080000, CRC(52bcc6d5) SHA1(25319ea6db35cc9bdcb39cc83d597a2a9f1690f3) )

	ROM_REGION( 0x800000, "sprite", 0 ) /* 32x32x8bpp sprite tiles */
        ROM_LOAD( "aj1scg0.12f",  0x000000, 0x200000, CRC(13ea766c) SHA1(a1a259bd8f468b90cbc891f1c2875fe03bba9802) )
        ROM_LOAD( "aj1scg1.10f",  0x200000, 0x200000, CRC(cb3638de) SHA1(0af99aaf00782036d7f479b00b0c3d9d7ad4fc37) )
        ROM_LOAD( "aj1scg2.8f",   0x400000, 0x200000, CRC(1048a09b) SHA1(6859533e24db5ac54e28d480aaac7b411a648dfe) )

	ROM_REGION( 0x200000*8, "textile", 0) /* 16x16x8bpp texture tiles */
        ROM_LOAD( "aj1cg0.8d",    0x000000, 0x200000, CRC(b814e1eb) SHA1(1af29897dcfd0a58743b4fcdd5049d9e6e3d4cbd) )
        ROM_LOAD( "aj1cg1.10d",   0x200000, 0x200000, CRC(dc63d496) SHA1(d3c3eea6b134850004062dd513c0f26096101227) )
        ROM_LOAD( "aj1cg2.12d",   0x400000, 0x200000, CRC(71fbb571) SHA1(40f84ab5a4a7bb4438fb53751762609671d17800) )
        ROM_LOAD( "aj1cg3.13d",   0x600000, 0x200000, CRC(e28052e2) SHA1(3dbc446d7a13312ed4c1d20c2e1209947853f2c4) )
        ROM_LOAD( "aj1cg4.14d",   0x800000, 0x200000, CRC(c77ae1a0) SHA1(e15f2ccbadb8634f6e5cfd2cf48f428d68bd92e1) )
        ROM_LOAD( "aj1cg5.16d",   0xa00000, 0x200000, CRC(15be0080) SHA1(ca14dfd2a66996f0b32fa1155c04f21d300e8f30) )
        ROM_LOAD( "aj1cg6.18d",   0xc00000, 0x200000, CRC(1a4f733a) SHA1(60a991f06e73667fa2c9016189999c4301cba24f) )
        ROM_LOAD( "aj1cg7.19d",   0xe00000, 0x200000, CRC(ea118130) SHA1(24ef22e5c8c6f6e8a01c72466be1e7acbfba63bc) )

	ROM_REGION16_LE( 0x280000, "textilemap", 0 ) /* texture tilemap */
        ROM_LOAD( "aj1ccrl.3d",   0x000000, 0x200000, CRC(3cc7a247) SHA1(336e4dd506d932987e20a5890dd3b0db75c02ccf) )
        ROM_LOAD( "aj1ccrh.1d",   0x200000, 0x080000, CRC(9d936030) SHA1(a383bcca494a9f8d9a08fbe9940c8071d4525d65) )

	ROM_REGION( 0x600000, "pointrom", 0 ) /* 3d model data */
        ROM_LOAD( "aj1ptrl0.18k", 0x000000, 0x080000, CRC(16205d45) SHA1(7e9681af852e2e875f80cbbe350e4982eaf80884) )
        ROM_LOAD( "aj1ptrl1.16k", 0x080000, 0x080000, CRC(6f114da7) SHA1(a1e7349ef8eb0ef043fce06f88bfbb76c2967393) )
        ROM_LOAD( "aj1ptrl2.15k", 0x100000, 0x080000, CRC(719b73f0) SHA1(bf8530a706097ae663769090177f1f259c540aa6) )
        ROM_LOAD( "aj1ptrl3.14k", 0x180000, 0x080000, CRC(9555fe31) SHA1(03bdd0784817191b3600d643b775191181a21706) )
        ROM_LOAD( "aj1ptrm0.18j", 0x200000, 0x080000, CRC(89bee2e0) SHA1(d84b06ef1318814fcf4782fef09a85ae1a87beb8) )
        ROM_LOAD( "aj1ptrm1.16j", 0x280000, 0x080000, CRC(0ecf88c7) SHA1(20b7cb09a6a13599cac024ef19c73612ce2952f6) )
        ROM_LOAD( "aj1ptrm2.15j", 0x300000, 0x080000, CRC(829bc7ba) SHA1(7104d66a027911e38ecf521f01ff098ccb76d5fb) )
        ROM_LOAD( "aj1ptrm3.14j", 0x380000, 0x080000, CRC(7d0a222a) SHA1(e78c405d00429580015c6c7d1bcd35393317a769) )
        ROM_LOAD( "aj1ptru0.18f", 0x400000, 0x080000, CRC(90d4e36a) SHA1(bdc44aac6aef5266d289f03c816aa2abdb263e9b) )
        ROM_LOAD( "aj1ptru1.16f", 0x480000, 0x080000, CRC(bf0cf4bf) SHA1(bbf06c7605c083d2fb8c72528c5c9d3b8b067073) )
        ROM_LOAD( "aj1ptru2.15f", 0x500000, 0x080000, CRC(91ffbb77) SHA1(fbfe7d32ef22037f7190d1b8263b5e8c55f2d892) )
        ROM_LOAD( "aj1ptru3.14f", 0x580000, 0x080000, CRC(d83d8d42) SHA1(e1561ce4538b01db92b7e645ad008cd1a2ddaf8a) )

	ROM_REGION( 0x1000000, "c352", 0 ) /* sound samples */
        ROM_LOAD( "aj1wavea.2l",  0x000000, 0x400000, CRC(8c72ea59) SHA1(3ae8dbd8baae08f1daab2b218932ba9d9451231d) )
        ROM_LOAD( "aj1waveb.1l",  0x800000, 0x400000, CRC(ab5a457f) SHA1(c34531fd574eb0c3e78fc31a9af8658df3446adc) )
ROM_END

/*******************************************************************/

static INPUT_PORTS_START( alpiner )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x01, "DIP4-1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DIP4-2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DIP4-3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIP4-4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DIP4-5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DIP4-6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DIP4-7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DIP4-8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("MCUP5A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) /* DECISION */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) /* L SELECTION */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) /* R SELECTION */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SWING")	/* SWING */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10)

	PORT_START("EDGE")	/* EDGE */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10)

	PORT_START("MCUP5B")
	PORT_DIPNAME( 0x01, 0x01, "DIP5-1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DIP5-2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DIP5-3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIP5-4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DIP5-5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DIP5-6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DIP5-7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DIP5-8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END /* Alpine Racer */


static INPUT_PORTS_START( airco22 )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x01, "DIP1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DIP2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DIP3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIP4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DIP5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DIP6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DIP7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DIP8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("PEDAL")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(100) PORT_KEYDELTA(4)

	PORT_START("STICKX")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(4)

	PORT_START("STICKY")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(4)

	PORT_START("MCUP5A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 )
//  PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) /* Missle */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) /* Gun */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 )
//  PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("MCUP5B")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_BUTTON3 )
INPUT_PORTS_END /* Air Combat22 */

static INPUT_PORTS_START( cybrcycc )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x01, "DIP4-1 (Test Mode)" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DIP4-2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DIP4-3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIP4-4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DIP4-5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DIP4-6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DIP4-7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DIP4-8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	DRIVING_ANALOG_PORTS

	PORT_START("MCUP5A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) /* VIEW */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("MCUP5B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END /* Cyber Cycles */

static INPUT_PORTS_START( tokyowar )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x01, "DIP4-1 (Test Mode)" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DIP4-2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DIP4-3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIP4-4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DIP4-5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DIP4-6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DIP4-7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DIP4-8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	DRIVING_ANALOG_PORTS

	PORT_START("MCUP5A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) /* VIEW */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON4 ) /* RIGHT */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON5 ) /* LEFT */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("MCUP5B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END /* Tokyo Wars */

static INPUT_PORTS_START( aquajet )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x01, "DIP4-1 (Test Mode)" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DIP4-2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DIP4-3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIP4-4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DIP4-5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DIP4-6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DIP4-7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DIP4-8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("GAS")
	PORT_BIT( 0x7f, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10)

	PORT_START("STEER")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10)

	PORT_START("STICKY")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10)

	PORT_START("MCUP5A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("MCUP5B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END /* Aqua Jet */

static INPUT_PORTS_START( propcycl )
	PORT_START("DSW0")	/* DIP4 */
	PORT_DIPNAME( 0x01, 0x01, "DIP1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DIP2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DIP3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIP4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DIP5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DIP6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DIP7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DIP8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("JOY")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )

	PORT_START("STICKX")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10)

	PORT_START("STICKY")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10)

	PORT_START("MCUP5A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("MCUP5B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END /* Prop Cycle */

static INPUT_PORTS_START( cybrcomm )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x0001, 0x0001, "DIP2-1" )
	PORT_DIPSETTING(    0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "DIP2-2" )
	PORT_DIPSETTING(    0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "DIP2-3" )
	PORT_DIPSETTING(    0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "DIP2-4" )
	PORT_DIPSETTING(    0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "DIP2-5" )
	PORT_DIPSETTING(    0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "DIP2-6" )
	PORT_DIPSETTING(    0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "DIP2-7" )
	PORT_DIPSETTING(    0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "DIP2-8" )
	PORT_DIPSETTING(    0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "DIP3-1" )
	PORT_DIPSETTING(    0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, "DIP3-2" )
	PORT_DIPSETTING(    0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, "DIP3-3" )
	PORT_DIPSETTING(    0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, "DIP3-4" )
	PORT_DIPSETTING(    0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, "DIP3-5" )
	PORT_DIPSETTING(    0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, "DIP3-6" )
	PORT_DIPSETTING(    0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "DIP3-7" )
	PORT_DIPSETTING(    0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "DIP3-8" )
	PORT_DIPSETTING(    0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )

	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) /* SHOOT */
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) /* MISSLE */
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) /* VIEW */
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE( 0x0400, IP_ACTIVE_LOW )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	/* Note(s)
       The ranges here are based on the test mode which displays +-224
       The nvram is calibrated using these settings.  If the SUBCPU handling changes then these might
        end up needing to change again too
       Default key arrangement is based on dual-joystick 'Tank' arrangement found in Assault and CyberSled
    */
	PORT_START("STICKY1")		/* VOLUME 0 */
	PORT_BIT( 0xff, 0x7f, IPT_AD_STICK_Y ) PORT_MINMAX(0x47,0xb7) /* range based on test mode */ PORT_CODE_DEC(KEYCODE_I) PORT_CODE_INC(KEYCODE_K) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2) /* right joystick: vertical */
	PORT_START("STICKY2")		/* VOLUME 0 */
	PORT_BIT( 0xff, 0x7f, IPT_AD_STICK_Y ) PORT_MINMAX(0x47,0xb7) /* range based on test mode */ PORT_CODE_DEC(KEYCODE_E) PORT_CODE_INC(KEYCODE_D) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1) /* left joystick: vertical */
	PORT_START("STICKX1")		/* VOLUME 0 */
	PORT_BIT( 0xff, 0x7f, IPT_AD_STICK_X ) PORT_MINMAX(0x47,0xb7) /* range based on test mode */ PORT_CODE_DEC(KEYCODE_J) PORT_CODE_INC(KEYCODE_L) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2) /* right joystick: horizontal */
	PORT_START("STICKX2")		/* VOLUME 0 */
	PORT_BIT( 0xff, 0x7f, IPT_AD_STICK_X ) PORT_MINMAX(0x47,0xb7) /* range based on test mode */ PORT_CODE_DEC(KEYCODE_S) PORT_CODE_INC(KEYCODE_F) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1) /* left joystick: horizontal */
INPUT_PORTS_END /* Cyber Commando */

static INPUT_PORTS_START( timecris )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x01, "DIP4-1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DIP4-2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DIP4-3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIP4-4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DIP4-5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DIP4-6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DIP4-7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START( "LIGHTX" )
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(4)
	PORT_START( "LIGHTY" )
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(4)

	PORT_START("MCUP5A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) /* shot */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) /* pedal */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("MCUP5B")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END /* Time Crisis */

/*****************************************************************************************************/

static CUSTOM_INPUT( acedrvr_shift_read )
{
	int shift = input_port_read(field->port->machine, "SHIFT");

	if (shift > 0 && shift != prev_stick_state)
	{
		prev_stick_state = shift;

		switch (shift)
		{
			case 0x01:
				stick_input = 0x01;
				break;
			case 0x02:
				stick_input = 0x03;
				break;
			case 0x04:
				stick_input = 0x02;
				break;
		}
	}

	return stick_input;
}

static INPUT_PORTS_START( acedrvr )
	PORT_START("DSW0")	/* DIP2 and DIP3 */
	PORT_DIPNAME( 0x0001, 0x0001, "DIP2-1" )
	PORT_DIPSETTING(    0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "DIP2-2" )
	PORT_DIPSETTING(    0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "DIP2-3" )
	PORT_DIPSETTING(    0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "DIP2-4" )
	PORT_DIPSETTING(    0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "DIP2-5" )
	PORT_DIPSETTING(    0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "DIP2-6" )
	PORT_DIPSETTING(    0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "DIP2-7" )
	PORT_DIPSETTING(    0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "DIP2-8" )
	PORT_DIPSETTING(    0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "DIP3-1" )
	PORT_DIPSETTING(    0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, "DIP3-2" )
	PORT_DIPSETTING(    0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, "DIP3-3" )
	PORT_DIPSETTING(    0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, "DIP3-4" )
	PORT_DIPSETTING(    0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, "DIP3-5" )
	PORT_DIPSETTING(    0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, "DIP3-6" )
	PORT_DIPSETTING(    0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "DIP3-7 (TEST MODE?)" )
	PORT_DIPSETTING(    0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "DIP3-8 (TEST MODE?)" )
	PORT_DIPSETTING(    0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )

	PORT_START("INPUTS")
	PORT_BIT( 0x0003, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(acedrvr_shift_read, NULL)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) /* VIEW */
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SHIFT")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Shift Up") PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Shift Center") PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Shift Down") PORT_PLAYER(1)

	DRIVING_ANALOG_PORTS
INPUT_PORTS_END /* Ace Driver */

static INPUT_PORTS_START( victlap )
	PORT_START("DSW0")	/* DIP2 and DIP3 */
	PORT_DIPNAME( 0x0001, 0x0001, "DIP2-1" )
	PORT_DIPSETTING(    0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "DIP2-2" )
	PORT_DIPSETTING(    0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "DIP2-3" )
	PORT_DIPSETTING(    0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "DIP2-4" )
	PORT_DIPSETTING(    0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "DIP2-5" )
	PORT_DIPSETTING(    0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "DIP2-6" )
	PORT_DIPSETTING(    0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "DIP2-7" )
	PORT_DIPSETTING(    0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "DIP2-8" )
	PORT_DIPSETTING(    0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "DIP3-1" )
	PORT_DIPSETTING(    0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, "DIP3-2" )
	PORT_DIPSETTING(    0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, "DIP3-3" )
	PORT_DIPSETTING(    0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, "DIP3-4" )
	PORT_DIPSETTING(    0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, "DIP3-5" )
	PORT_DIPSETTING(    0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, "DIP3-6" )
	PORT_DIPSETTING(    0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "DIP3-7 (TEST MODE?)" )
	PORT_DIPSETTING(    0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "DIP3-8 (TEST MODE?)" )
	PORT_DIPSETTING(    0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )

	PORT_START("INPUTS")
	PORT_BIT( 0x0003, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(acedrvr_shift_read, NULL)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) /* VIEW */
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x8000, 0x8000, "Motion Stop" )
	PORT_DIPSETTING(    0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )

	PORT_START("SHIFT")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Shift Up") PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Shift Center") PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Shift Down") PORT_PLAYER(1)

	DRIVING_ANALOG_PORTS
INPUT_PORTS_END /* Victory Lap */


static CUSTOM_INPUT( ridger_gear_read )
{
	int gear = input_port_read(field->port->machine, "GEARS");

	if (gear > 0 && gear != prev_stick_state)
	{
		prev_stick_state = gear;

		switch (gear)
		{
		case 0x01:
			stick_input = 0x0a;
			break;
		case 0x02:
			stick_input = 0x09;
			break;
		case 0x04:
			stick_input = 0x0e;
			break;
		case 0x08:
			stick_input = 0x0d;
			break;
		case 0x10:
			stick_input = 0x06;
			break;
		case 0x20:
			stick_input = 0x05;
			break;
		}
	}

	return stick_input;
}

static INPUT_PORTS_START( ridgera )
	PORT_START("DSW0")	/* DIP2 and DIP3 */
	PORT_DIPNAME( 0x0001, 0x0001, "DIP2-1 (test mode?)" )
	PORT_DIPSETTING(    0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "DIP2-2" )
	PORT_DIPSETTING(    0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "DIP2-3" )
	PORT_DIPSETTING(    0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "DIP2-4" )
	PORT_DIPSETTING(    0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "DIP2-5" )
	PORT_DIPSETTING(    0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "DIP2-6" )
	PORT_DIPSETTING(    0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "DIP2-7" )
	PORT_DIPSETTING(    0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "DIP2-8" )
	PORT_DIPSETTING(    0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "DIP3-1" )
	PORT_DIPSETTING(    0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, "DIP3-2" )
	PORT_DIPSETTING(    0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, "DIP3-3" )
	PORT_DIPSETTING(    0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, "DIP3-4" )
	PORT_DIPSETTING(    0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, "DIP3-5" )
	PORT_DIPSETTING(    0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, "DIP3-6" )
	PORT_DIPSETTING(    0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "DIP3-7 (TEST MODE)" )
	PORT_DIPSETTING(    0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "DIP3-8" )
	PORT_DIPSETTING(    0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )

	PORT_START("INPUTS")
	PORT_BIT( 0x000f, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(ridger_gear_read, NULL)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) /* CLUTCH */
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* MOTION STOP? */

	PORT_START("GEARS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("1st Gear") PORT_PLAYER(1)	/* 1st gear */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("2nd Gear") PORT_PLAYER(1)	/* 2nd gear */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("3rd Gear") PORT_PLAYER(1)	/* 3rd gear */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("4th Gear") PORT_PLAYER(1)	/* 4th gear */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_NAME("5th Gear") PORT_PLAYER(1)	/* 5th gear */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_NAME("6th Gear") PORT_PLAYER(1)	/* 6th gear */

	DRIVING_ANALOG_PORTS
INPUT_PORTS_END /* Ridge Racer */


static INPUT_PORTS_START( rrf )
	PORT_START("DSW0")	/* DIP2 and DIP3 */
	PORT_DIPNAME( 0x0001, 0x0000, "DIP2-1" )
	PORT_DIPSETTING(    0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0000, "DIP2-2" )
	PORT_DIPSETTING(    0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "DIP2-3" )
	PORT_DIPSETTING(    0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "DIP2-4" )
	PORT_DIPSETTING(    0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "DIP2-5" )
	PORT_DIPSETTING(    0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "DIP2-6" )
	PORT_DIPSETTING(    0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "DIP2-7" )
	PORT_DIPSETTING(    0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "DIP2-8" )
	PORT_DIPSETTING(    0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "DIP3-1" )
	PORT_DIPSETTING(    0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, "DIP3-2" )
	PORT_DIPSETTING(    0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, "DIP3-3" )
	PORT_DIPSETTING(    0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, "DIP3-4" )
	PORT_DIPSETTING(    0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, "DIP3-5" )
	PORT_DIPSETTING(    0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, "DIP3-6" )
	PORT_DIPSETTING(    0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "DIP3-7 (TEST MODE)" )
	PORT_DIPSETTING(    0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "DIP3-8" )
	PORT_DIPSETTING(    0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )

	PORT_START("INPUTS")
	PORT_BIT( 0x000f, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(ridger_gear_read, NULL)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) /* CLUTCH */
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* MOTION STOP? */

	PORT_START("GEARS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("1st Gear") PORT_PLAYER(1)	/* 1st gear */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("2nd Gear") PORT_PLAYER(1)	/* 2nd gear */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("3rd Gear") PORT_PLAYER(1)	/* 3rd gear */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("4th Gear") PORT_PLAYER(1)	/* 4th gear */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_NAME("5th Gear") PORT_PLAYER(1)	/* 5th gear */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_NAME("6th Gear") PORT_PLAYER(1)	/* 6th gear */

	DRIVING_ANALOG_PORTS
INPUT_PORTS_END /* Ridge Racer */


static INPUT_PORTS_START( raveracw )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x0001, 0x0001, "DIP2-1 (test mode)" )
	PORT_DIPSETTING(    0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "DIP2-2" )
	PORT_DIPSETTING(    0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "DIP2-3" )
	PORT_DIPSETTING(    0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "DIP2-4" )
	PORT_DIPSETTING(    0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "DIP2-5" )
	PORT_DIPSETTING(    0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "DIP2-6" )
	PORT_DIPSETTING(    0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "DIP2-7" )
	PORT_DIPSETTING(    0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "DIP2-8" )
	PORT_DIPSETTING(    0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "DIP3-1" )
	PORT_DIPSETTING(    0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, "DIP3-2" )
	PORT_DIPSETTING(    0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, "DIP3-3" )
	PORT_DIPSETTING(    0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, "DIP3-4" )
	PORT_DIPSETTING(    0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, "DIP3-5" )
	PORT_DIPSETTING(    0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, "DIP3-6" )
	PORT_DIPSETTING(    0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "DIP3-7" )
	PORT_DIPSETTING(    0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "DIP3-8" )
	PORT_DIPSETTING(    0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )

	PORT_START("INPUTS")
	PORT_BIT( 0x000f, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(ridger_gear_read, NULL)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) /* CLUTCH */
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON2 ) /* VIEW */
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* MOTION STOP? */

	PORT_START("GEARS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("1st Gear") PORT_PLAYER(1)	/* 1st gear */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("2nd Gear") PORT_PLAYER(1)	/* 2nd gear */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("3rd Gear") PORT_PLAYER(1)	/* 3rd gear */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("4th Gear") PORT_PLAYER(1)	/* 4th gear */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_NAME("5th Gear") PORT_PLAYER(1)	/* 5th gear */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_NAME("6th Gear") PORT_PLAYER(1)	/* 6th gear */

	DRIVING_ANALOG_PORTS
INPUT_PORTS_END /* Rave Racer */

/*****************************************************************************************************/

// MCU speed cheats (every bit helps with these games)
static UINT16 su_82;

// for MCU BIOS v1.41
static READ16_HANDLER( mcu141_speedup_r )
{
	if ((cpu_get_pc(space->cpu) == 0xc12d) && (!(su_82 & 0xff00)))
	{
		cpu_spinuntil_int(space->cpu);
	}

	return su_82;
}

static WRITE16_HANDLER( mcu_speedup_w )
{
	COMBINE_DATA(&su_82);
}

// for MCU BIOS v1.30
static READ16_HANDLER( mcu130_speedup_r )
{
	if ((cpu_get_pc(space->cpu) == 0xc12a) && (!(su_82 & 0xff00)))
	{
		cpu_spinuntil_int(space->cpu);
	}

	return su_82;
}

// for NSTX7702 v1.00 (C74)
static READ16_HANDLER( mcuc74_speedup_r )
{
	if (((cpu_get_pc(space->cpu) == 0xc0df) || (cpu_get_pc(space->cpu) == 0xc101)) && (!(su_82 & 0xff00)))
	{
		cpu_spinuntil_int(space->cpu);
	}

	return su_82;
}

static void install_c74_speedup(running_machine *machine)
{
	memory_install_readwrite16_handler(cputag_get_address_space(machine, "mcu", ADDRESS_SPACE_PROGRAM), 0x80, 0x81, 0, 0, mcuc74_speedup_r, mcu_speedup_w);
}

static void install_130_speedup(running_machine *machine)
{
	// install speedup cheat for 1.30 MCU BIOS
	memory_install_readwrite16_handler(cputag_get_address_space(machine, "mcu", ADDRESS_SPACE_PROGRAM), 0x82, 0x83, 0, 0, mcu130_speedup_r, mcu_speedup_w);
}

static void install_141_speedup(running_machine *machine)
{
	// install speedup cheat for 1.41 MCU BIOS
	memory_install_readwrite16_handler(cputag_get_address_space(machine, "mcu", ADDRESS_SPACE_PROGRAM), 0x82, 0x83, 0, 0, mcu141_speedup_r, mcu_speedup_w);
}

static void namcos22_init( running_machine *machine, enum namcos22_gametype game_type )
{
	namcos22_gametype = game_type;
	mpPointRAM = auto_alloc_array(machine, UINT32, 0x20000);
}

static void namcos22s_init( running_machine *machine, enum namcos22_gametype game_type )
{
	namcos22_init(machine, game_type);
	mSpotRAM.RAM = auto_alloc_array(machine, UINT16, SPOTRAM_SIZE);
}

static DRIVER_INIT( alpiner )
{
	namcos22s_init(machine, NAMCOS22_ALPINE_RACER);

	memory_install_read8_handler(cputag_get_address_space(machine, "mcu", ADDRESS_SPACE_IO), M37710_ADC0_L, M37710_ADC7_H, 0, 0, alpineracer_mcu_adc_r);

	install_130_speedup(machine);
}

static DRIVER_INIT( alpiner2 )
{
	namcos22s_init(machine, NAMCOS22_ALPINE_RACER_2);

	memory_install_read8_handler(cputag_get_address_space(machine, "mcu", ADDRESS_SPACE_IO), M37710_ADC0_L, M37710_ADC7_H, 0, 0, alpineracer_mcu_adc_r);

	install_130_speedup(machine);
}

static DRIVER_INIT( alpinesa )
{
	namcos22s_init(machine, NAMCOS22_ALPINE_SURFER);

	memory_install_read8_handler(cputag_get_address_space(machine, "mcu", ADDRESS_SPACE_IO), M37710_ADC0_L, M37710_ADC7_H, 0, 0, alpineracer_mcu_adc_r);

	install_141_speedup(machine);
}

static DRIVER_INIT( airco22 )
{
	namcos22s_init(machine, NAMCOS22_AIR_COMBAT22);

	memory_install_read8_handler(cputag_get_address_space(machine, "mcu", ADDRESS_SPACE_IO), M37710_ADC0_L, M37710_ADC7_H, 0, 0, airco22_mcu_adc_r);
}

static DRIVER_INIT( propcycl )
{
   UINT32 *pROM = (UINT32 *)memory_region(machine, "maincpu");

	/* patch out strange routine (uninitialized-eprom related?) */
	pROM[0x1992C/4] = 0x4E754E75;

	/**
     * The dipswitch reading routine in Prop Cycle polls the
     * dipswitch value, but promptly overwrites with zero, discarding
     * it.
     *
     * By patching out this behavior, we expose an additional "secret" test.
     *
     * DIP5: real time display of "INST_CUNT, MODE_NUM, MODE_CUNT"
     */
//   pROM[0x22296/4] &= 0xffff0000;
//   pROM[0x22296/4] |= 0x00004e75;

	namcos22s_init(machine, NAMCOS22_PROP_CYCLE);

	memory_install_read8_handler(cputag_get_address_space(machine, "mcu", ADDRESS_SPACE_IO), M37710_ADC0_L, M37710_ADC7_H, 0, 0, propcycle_mcu_adc_r);

	install_141_speedup(machine);
}

static DRIVER_INIT( ridgeraj )
{
	namcos22_init(machine, NAMCOS22_RIDGE_RACER);

	install_c74_speedup(machine);

	old_coin_state = 0;
	credits1 = credits2 = 0;
	stick_input = 0xa;
}

static DRIVER_INIT( ridger2j )
{
	namcos22_init(machine, NAMCOS22_RIDGE_RACER2);

	install_c74_speedup(machine);

	old_coin_state = 0;
	credits1 = credits2 = 0;
	stick_input = 0xa;
}

static DRIVER_INIT( acedrvr )
{
	namcos22_init(machine, NAMCOS22_ACE_DRIVER);

	install_c74_speedup(machine);

	old_coin_state = 0;
	credits1 = credits2 = 0;
	stick_input = 0x3;
}

static DRIVER_INIT( victlap )
{
	namcos22_init(machine, NAMCOS22_VICTORY_LAP);

	install_c74_speedup(machine);

	old_coin_state = 0;
	credits1 = credits2 = 0;
	stick_input = 0x3;
}

static DRIVER_INIT( raveracw )
{
	namcos22_init(machine, NAMCOS22_RAVE_RACER);

	install_c74_speedup(machine);

	old_coin_state = 0;
	credits1 = credits2 = 0;
	stick_input = 0xa;
}

static DRIVER_INIT( cybrcomm )
{
	UINT32 *pROM = (UINT32 *)memory_region(machine, "maincpu");
	pROM[0x18ade8/4] = 0x4e714e71;
	pROM[0x18ae38/4] = 0x4e714e71;
	pROM[0x18ae80/4] = 0x4e714e71;
	pROM[0x18aec8/4] = 0x4e714e71;
	pROM[0x18aefc/4] = 0x4e714e71;

	namcos22_init(machine, NAMCOS22_CYBER_COMMANDO);

	install_c74_speedup(machine);

	old_coin_state = 0;
	credits1 = credits2 = 0;
}

static DRIVER_INIT( cybrcyc )
{
	/* patch DSP RAM test */
	UINT32 *pROM = (UINT32 *)memory_region(machine, "maincpu");
	pROM[0x355C/4] &= 0x0000ffff;
	pROM[0x355C/4] |= 0x4e710000;

	namcos22s_init(machine, NAMCOS22_CYBER_CYCLES);

	memory_install_read8_handler(cputag_get_address_space(machine, "mcu", ADDRESS_SPACE_IO), M37710_ADC0_L, M37710_ADC7_H, 0, 0, cybrcycc_mcu_adc_r);

	install_130_speedup(machine);
}

static DRIVER_INIT( timecris )
{
	namcos22s_init(machine, NAMCOS22_TIME_CRISIS);

	// install_130_speedup(machine); // with speed up the SUBCPU START WAIT test fails
}

static DRIVER_INIT( tokyowar )
{
	namcos22s_init(machine, NAMCOS22_TOKYO_WARS);

	memory_install_read8_handler(cputag_get_address_space(machine, "mcu", ADDRESS_SPACE_IO), M37710_ADC0_L, M37710_ADC7_H, 0, 0, tokyowar_mcu_adc_r);
}

static DRIVER_INIT( aquajet )
{
	namcos22s_init(machine, NAMCOS22_AQUA_JET);

	memory_install_read8_handler(cputag_get_address_space(machine, "mcu", ADDRESS_SPACE_IO), M37710_ADC0_L, M37710_ADC7_H, 0, 0, aquajet_mcu_adc_r);
}

static DRIVER_INIT( dirtdash )
{
	namcos22s_init(machine, NAMCOS22_DIRT_DASH);

	memory_install_read8_handler(cputag_get_address_space(machine, "mcu", ADDRESS_SPACE_IO), M37710_ADC0_L, M37710_ADC7_H, 0, 0, aquajet_mcu_adc_r);
}

/************************************************************************************/

/*     YEAR, NAME,    PARENT,    MACHINE,   INPUT,    INIT,     MNTR,  COMPANY, FULLNAME,                                    FLAGS */
/* System22 games */
GAME( 1995, cybrcomm,  0,        namcos22,  cybrcomm, cybrcomm, ROT0, "Namco", "Cyber Commando (Rev. CY1, Japan)"          , GAME_IMPERFECT_SOUND|GAME_IMPERFECT_GRAPHICS|GAME_NOT_WORKING )
GAME( 1995, raveracw,  0,        namcos22,  raveracw, raveracw, ROT0, "Namco", "Rave Racer (Rev. RV2, World)"              , GAME_IMPERFECT_SOUND|GAME_IMPERFECT_GRAPHICS )
GAME( 1995, raveracj,  raveracw, namcos22,  raveracw, raveracw, ROT0, "Namco", "Rave Racer (Rev. RV1 Ver.B, Japan)"        , GAME_IMPERFECT_SOUND|GAME_IMPERFECT_GRAPHICS )
GAME( 1995, raveracja, raveracw, namcos22,  raveracw, raveracw, ROT0, "Namco", "Rave Racer (Rev. RV1, Japan)"              , GAME_IMPERFECT_SOUND|GAME_IMPERFECT_GRAPHICS )
GAME( 1993, ridgerac,  0,        namcos22,  ridgera,  ridgeraj, ROT0, "Namco", "Ridge Racer (Rev. RR3, World)"             , GAME_IMPERFECT_SOUND|GAME_IMPERFECT_GRAPHICS ) /* 1994-01-17 */
GAME( 1993, ridgeracb, ridgerac, namcos22,  ridgera,  ridgeraj, ROT0, "Namco", "Ridge Racer (Rev. RR2, World)"             , GAME_IMPERFECT_SOUND|GAME_IMPERFECT_GRAPHICS ) /* 1993-10-07 */
GAME( 1993, ridgeracj, ridgerac, namcos22,  ridgera,  ridgeraj, ROT0, "Namco", "Ridge Racer (Rev. RR1, Japan)"             , GAME_IMPERFECT_SOUND|GAME_IMPERFECT_GRAPHICS ) /* 1993-10-07 */
GAME( 1993, ridgerac3, ridgerac, namcos22,  ridgera,  ridgeraj, ROT0, "Namco", "Ridge Racer (3 Screen? Rev. RR2, World)"   , GAME_IMPERFECT_SOUND|GAME_IMPERFECT_GRAPHICS ) /* 1993-10-28 */
GAME( 1993, rrf      , ridgerac, namcos22,  rrf,      ridgeraj, ROT0, "Namco", "Ridge Racer (Full Scale, 1993-12-13, World)"   , GAME_IMPERFECT_SOUND|GAME_IMPERFECT_GRAPHICS|GAME_NOT_WORKING ) // very different version, incomplete dump.
GAME( 1994, ridgera2,  0,        namcos22,  ridgera,  ridger2j, ROT0, "Namco", "Ridge Racer 2 (Rev. RRS2, US)"             , GAME_IMPERFECT_SOUND|GAME_IMPERFECT_GRAPHICS|GAME_NOT_WORKING ) /* POSTs but doesn's start */
GAME( 1994, ridgera2j, ridgera2, namcos22,  ridgera,  ridger2j, ROT0, "Namco", "Ridge Racer 2 (Rev. RRS1, Ver.B, Japan)"   , GAME_IMPERFECT_SOUND|GAME_IMPERFECT_GRAPHICS|GAME_NOT_WORKING )
GAME( 1994, ridgera2ja,ridgera2, namcos22,  ridgera,  ridger2j, ROT0, "Namco", "Ridge Racer 2 (Rev. RRS1, Japan)"          , GAME_IMPERFECT_SOUND|GAME_IMPERFECT_GRAPHICS|GAME_NOT_WORKING )
GAME( 1994, acedrvrw,  0,        namcos22,  acedrvr,  acedrvr,  ROT0, "Namco", "Ace Driver (Rev. AD2, World)"              , GAME_IMPERFECT_SOUND|GAME_IMPERFECT_GRAPHICS )
GAME( 1996, victlapw,  0,        namcos22,  victlap,  victlap,  ROT0, "Namco", "Ace Driver: Victory Lap (Rev. ADV2, World)", GAME_IMPERFECT_SOUND|GAME_IMPERFECT_GRAPHICS )

/* Super System22 games */
GAME( 1995, airco22b, 0,        namcos22s, airco22,  airco22,  ROT0, "Namco", "Air Combat 22 (Rev. ACS1 Ver.B, Japan)"           , GAME_IMPERFECT_SOUND|GAME_IMPERFECT_GRAPHICS|GAME_NOT_WORKING ) /* boots but missing sprite clear DMA? */
GAME( 1995, alpinerd, 0,        namcos22s, alpiner,  alpiner,  ROT0, "Namco", "Alpine Racer (Rev. AR2 Ver.D)"             , GAME_IMPERFECT_SOUND|GAME_IMPERFECT_GRAPHICS )
GAME( 1995, alpinerc, alpinerd, namcos22s, alpiner,  alpiner,  ROT0, "Namco", "Alpine Racer (Rev. AR2 Ver.C)"             , GAME_IMPERFECT_SOUND|GAME_IMPERFECT_GRAPHICS )
GAME( 1995, cybrcycc, 0,        namcos22s, cybrcycc, cybrcyc,  ROT0, "Namco", "Cyber Cycles (Rev. CB2 Ver.C)"             , GAME_IMPERFECT_SOUND|GAME_IMPERFECT_GRAPHICS )
GAME( 1995, dirtdash, 0,        namcos22s, aquajet,  dirtdash, ROT0, "Namco", "Dirt Dash (Rev. DT2)"                      , GAME_IMPERFECT_SOUND|GAME_IMPERFECT_GRAPHICS|GAME_NOT_WORKING )
GAME( 1995, timecris, 0,        namcos22s, timecris, timecris, ROT0, "Namco", "Time Crisis (Rev. TS2 Ver.B)"              , GAME_IMPERFECT_SOUND|GAME_IMPERFECT_GRAPHICS )
GAME( 1995, timecrisa,timecris, namcos22s, timecris, timecris, ROT0, "Namco", "Time Crisis (Rev. TS2 Ver.A)"              , GAME_IMPERFECT_SOUND|GAME_IMPERFECT_GRAPHICS )
GAME( 1996, alpinr2b, 0,        namcos22s, alpiner,  alpiner2, ROT0, "Namco", "Alpine Racer 2 (Rev. ARS2 Ver.B)"          , GAME_IMPERFECT_SOUND|GAME_IMPERFECT_GRAPHICS )
GAME( 1996, alpinr2a, alpinr2b, namcos22s, alpiner,  alpiner2, ROT0, "Namco", "Alpine Racer 2 (Rev. ARS2 Ver.A)"          , GAME_IMPERFECT_SOUND|GAME_IMPERFECT_GRAPHICS|GAME_NOT_WORKING )	/* issues with step lock */
GAME( 1996, alpinesa, 0,        namcos22s, alpiner,  alpinesa, ROT0, "Namco", "Alpine Surfer (Rev. AF2 Ver.A)"            , GAME_IMPERFECT_SOUND|GAME_IMPERFECT_GRAPHICS )
GAME( 1996, propcycl, 0,        namcos22s, propcycl, propcycl, ROT0, "Namco", "Prop Cycle (Rev. PR2 Ver.A)"                , GAME_IMPERFECT_SOUND|GAME_IMPERFECT_GRAPHICS )
GAME( 1996, tokyowar, 0,        namcos22s, tokyowar, tokyowar, ROT0, "Namco", "Tokyo Wars (Rev. TW2 Ver.A)"                , GAME_IMPERFECT_SOUND|GAME_IMPERFECT_GRAPHICS|GAME_NOT_WORKING )	/* missing sprite clear DMA?  or protection? */
GAME( 1996, aquajet,  0,        namcos22s, aquajet,  aquajet,  ROT0, "Namco", "Aqua Jet (Rev. AJ2 Ver.B)"                , GAME_IMPERFECT_SOUND|GAME_IMPERFECT_GRAPHICS )
//GAME( 1997, armdilox, "Armadillo Racing")
