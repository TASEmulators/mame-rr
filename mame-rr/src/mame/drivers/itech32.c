/***************************************************************************

    Incredible Technologies/Strata system
    (32-bit blitter variant)

    Driver by Aaron Giles
    Golden Tee variants & World Class Bowling Deluxe additions by Brian A. Troha

    Games supported:
        * Time Killers (2 sets)
        * Bloodstorm (4 sets)
        * Hard Yardage (2 sets)
        * Pairs (3 sets)
        * Driver's Edge (1 set)
        * World Class Bowling (9 sets)
        * Street Fighter: The Movie (4 sets)
        * Shuffleshot (3 sets)
        * Golden Tee 3D Golf (11 sets)
        * Golden Tee Golf '97 (7 sets)
        * Golden Tee Golf '98 (4 sets)
        * Golden Tee Golf '99 (4 Sets)
        * Golden Tee Golf 2K (5 Sets)
        * Golden Tee Classic (3 Sets)

    Known issues:
        * volume controls do not work in the Golden Tee games
        * Driver's Edge accesses many uninitialized RAM locations;
            requires hack to make steering in attract mode work

****************************************************************************

    There are at least 4 types of Golden Tee PCBs
        * 3 Tier long ROM board
            - Versions end in "L"
            - No cocktail mode
            - 2 trackball connectors
            - GT3D
        * 3 Tier short ROM board
            - Versions end in "S"
            - No cocktail mode
            - 2 trackball connectors
            - GT3D
        * 3 Tier short ROM board
            - Versions end in "S"
            - No cocktail mode
            - 1 trackball connector
            - GT97 through Classic
        * Large single layer PCB
            - GT3D for this board ends in "N" for "Non-Tournament"
            - Tournament versions end in "T" (requires optional chips, see below)
            - 2 trackball connectors
            - Cocktail mode support with most recent version of each chipset, except GT3D
            - GT3D through GT Classic

    There are at least 3 different revisions of the sound board for the 3-tier Golden Tee boards
        P/N 1066 Rev 2: Ensoniq sample 2M 1350901601, sound CPU code is V1.0 to V1.2
        P/N 1078 Rev 1: Ensoniq sample 2MX16U 1350901801, sound CPU code is V2.0 to V2.2, this
            is the same sample rom IT used on the 3-tier World Class Bowling for SROM 0.
        P/N 1078 Rev 1: This revision dropped the Ensoniq samples and converted to the samples
            currently used on the single PCB format.  Roms are identified by the use of "NR"
            on the label and the sound code is labeled "GTG3_NR(U23)"

    GT Classic was the last chipset for this platform and was sold mainly as an upgrade for GT99/GT2K

    Later versions of games (Golden Tee Golf, Shuffle Shot & World Class
    Bowling) converged to a single platform, the large single PCB
    (P/N 1083 Rev 2).  With the correct jumpers setting (surface mounted)
    and chipset this board can run any of the 3 listed programs.

    Starting with GT Fore!, I.T. moved to a redesigned hardware platform known as the Eagle platform.
        It has a main board with a 3Dfx video card and is hard drive based. This series started with
        GT Fore! in 2000 and continued through 2006 ending with Golden Tee Complete. This final
        version incuded all the courses from all the previous years in the Fore! series.
        The Eagle platform also supports I.T.'s hunting series "Big Buck Hunter" and the light game
        gun called Carnival King.

        Some time in 2004 I.T. introduced a new bowling game called Silver Strike Bowling on a full
        blown PC system known as "Nighthawk System Box" (AKA The Nighthawk Chassis) to replace it's
        aging World Class Bowling game.  Other known games on this platform include Golden Tee Live!
        and Target Toss Pro: Bags

    Trivia: For the Golden Tee series, the second generation was called GT2.  The third gen was known
            as GT3 but also included Golden Tee '97 through 2K and Classic.  The fourth gen on the
            Eagle platform was GT4... using the pun or play on words became the GT Fore! series ;-)

****************************************************************************

    Memory map TBD

****************************************************************************

    Memory sizes:
        Time Killers:
            2 * MS6264L    = 2 *   8k =   16k (main RAM)
            6 * V52C8126K  = 6 * 128k =  768k (video RAM)
            3 * NMS64X8AM  = 3 *  32k =   96k (palette RAM)

        Hard Yardage:
            2 * MS62256    = 2 *  32k =   64k (main RAM)
            4 * V52C8128K  = 4 * 128k =  512k (video RAM)
            3 * MB84256    = 3 *  32k =   96k (palette RAM)

        Bloodstorm:
            2 * MS62256    = 2 *  32k =   64k (main RAM)
            6 * V52C8128K  = 6 * 128k =  768k (video RAM)
            3 * MB84256    = 3 *  32k =   96k (palette RAM)

        Driver's Edge:
            4 * CXK58258   = 4 *  32k =  128k (main RAM)
            4 * CY7C199    = 4 *  32k =  128k (main RAM)
            8 * V52C8128K  = 8 * 128k = 1024k (video RAM)
            3 * CXK58257AM = 3 *  32k =   96k (palette RAM)
            8 * CY7C141    = 8 *   1k =    8k (dual ported shared RAM btw 68k and TMS)
            8 * CXK581000AM= 8 * 128k = 1024k (TMS1 RAM)
            4 * CY7C185    = 4 *   8k =   32k (TMS1 RAM)
            8 * CY7C199    = 8 *  32k =  256k (TMS2 RAM)

****************************************************************************

Golden Tee 3D / World Class Bowling / Shuffleshot hardware (aka P/N 1083 PCB)
Incredible Technologies, 1996-2002

This covers all the Incredible Technologies games on the single large PCB

PCB Layout
----------

P/N 1083 REV 2
|----------------------------------------------------------------------|
|SROM2 SROM1 SROM0 U88 LH5164                 XC7336(2)          GRM2_0|
|                                  HM538123       8MHz       GRM1_0    |
|SROM4 SROM3                       HM538123              GRM0_0        |
|       68B09           CY7C199    HM538123   |------|           GRM2_1|
|            PAL1 PAL2             HM538123   | IT42 |       GRM1_1    |
|                       CY7C199    HM538123   |      |   GRM0_1        |
|           16MHz                  HM538123   |------|           GRM2_3|
|   ES5506              CY7C199                              GRM1_2    |
|                       PROM1 PROM0   CY7C185            GRM0_2        |
|   TDA1543 LED                       CY7C185    68EC020         GRM2_3|
|   X9312WS X9312WS                   CY7C185          25MHz GRM1_3    |
|    3403             TC551001        CY7C185            GRM0_3        |
|TICKETS     BATTERY    PROM2 PROM3              XC7336(1)      PLAYER1|
|                   LTC691                           DSW(4)            |
|VOLUME                LT723                                    PLAYER2|
|  TDA7240A             DSW(4)                 4MHz       PAL3         |
|                                              PIC16C54 LED            |
|------------------------------JAMMA-----------------------------------|
Notes:
      68EC020   - clock 25.000MHz
      ES5506    - clock 16.000MHz
      68B09     - clock 2.000MHz [16/8]
      CY7C185   - 8k x8 SRAM
      CY7C199   - 32k x8 SRAM
      HM528123  - Hitachi HM538123 128k x8 VRAM
      TC551001  - Toshiba TC551001 128k x8 SRAM
      LH5164    - Sharp LH5164 8k x8 SRAM
      XC7336(1) - XILINX XC7336 CPLD labelled 'ITMP3-15'
      XC7336(2) - XILINX XC7336 CPLD labelled 'ITVR-1'
      IT42      - Custom Incredible Technologies Graphics Generator (QFP208)

      Measurements
      ------------
      X1    - 8.00006MHz
      X2    - 25.00002MHz
      X3    - 4.00000MHz
      X5    - 15.99984MHz
      VSync - 59.7612Hz
      HSync - 15.6570kHz

****************************************************************************

Hot Memory (German V1.2)
Tuning/Strata/Incredible Technologies, 1994

This game is a clone of Pairs and runs on Incredible Technologies 32bit hardware.

PCB Layout
----------

Top (2 separate PCBs plugged into the main board)
---

P/N 1060 REV 0
|---------------------------------|    |---------------------------------|
|6522          SND.U17    SROM0   |    |  GROM15            GROM10       |
|   6809     6264   ENSONIC       |    |                                 |
|                                 |    |  GROM16            GROM11       |
|  LED1                           |    |                                 |
|        PAL1                     |    | *GROM17           *GROM12       |
|                                 |    |                                 |
|                  *SROM2 *SROM1  |    | *GROM18           *GROM13       |
|    ES5506  16MHz                |    |                                 |
|               MM5437            |    |                                 |
|                                 |    |                                 |
|            PAL2                 |    |  GROM5             GROM0        |
|                                 |    |                                 |
|                           VOL   |    |  GROM6             GROM1        |
|                    TDA1543      |    |                                 |
|                                 |    | *GROM7            *GROM2        |
|    555         3403    3403     |    |                                 |
|                                 |    | *GROM8            *GROM3        |
|                                 |    |                                 |
|        JP3                      |    | *GROM9            *GROM4        |
|---------------------------------|    |---------------------------------|
Notes:
      *      - These locations not populated
      ES5506 - Ensonic ES5506 OTTOR2, clock 16.000MHz
      6809   - STMicroelectronics EF68B09, clock 2.000MHz
      ENSONIC- DIP42 chip labelled 'ENSONIC (C)1992 2M 1350901601 9320 1.00' at location SROM0
               -This is actually a 16MBit DIP42 MaskROM
      MM5437 - National Semiconductor MM5437 pseudo-random noise generator chip (DIP8)
      LED1   - Sound Status Yellow LED, blinks when active
      PAL1   - Labelled 'ITBP-1'
      PAL2   - Labelled 'ITSS-1'
      JP3    - 4 pin connector for right & left speaker output

Main Board
----------

P/N 1059 REV3
|------------------------------------------------------------------------|
|MC3423            84256                   V52C8128K70                   |
|                                                                        |
|                                                                        |
|                  84256                                                 |
|                                                                        |
|                                          V52C8128K70                   |
|A                 84256                            V52C8128K70          |
|M                                                           V52C8128K70 |
|M             PAL3      LED2                                            |
|A                                       PAL4   PAL5   PAL6   PAL7   PAL8|
|J      |-----|                                                          |
|       |     |               HOTMEM1.U88                                |
|       |  6  |           62256                                          |
|       |  8  |                                         |----------|     |
|       |  0  |12MHz                          8MHz      |          |     |
|       |  0  |               HOTMEM0.U83               |   IT42   |     |
|       |  0  | BATTERY   62256                         | (QFP208) |     |
|       |     |                                         |          |     |
|DSW(4) |-----| ADM690                     25MHz        |----------|     |
|  J1         J2                                                         |
|------------------------------------------------------------------------|
Notes:
      AMMAJ - Note JAMMA connector is backwards!
      68000 - clock 12.000MHz
      HSync - 15.68kHz
      VSync - 60Hz
      J1/J2 - 15 Pin Connector For Player 3 & 4 Controls
      PAL3  - GAL22V10
      PAL4  - Labelled 'ITVS-16A'
      PAL5  - Labelled 'ITVS-15'
      PAL6  - Labelled 'ITVS-14'
      PAL7  - Labelled 'ITVS-13'
      PAL8  - Labelled 'ITVS-12'
      LED2  - CPU Status Green LED, blinks when active
      IT42  - Custom Incredible Technologies Graphics Generator (QFP208)
      ADM690- Analog Devices ADM690 4.65V Reset, Battery Switchover, Watchdog Timer, Power Fail Input IC (DIP8)
      MC3423- Motorola MC3423 Overvoltage Sensing Circuit (SOIC8)
      84256 - Fujitsu MB84256A-10L 32k x8 SRAM (SOP28)
      62256 - MOSEL MS62256L-10PC 32k x8 SRAM (DIP28)
      V52C8128K70 - Vitelic V52C8128K70 ?? possibly 128k x8 DRAM (SOJ40)

****************************************************************************

Hard Yardage
Strata/Incredible Technologies, 1993

PCB Layout
----------

Top (2 separate PCBs plugged into the main board)
---

P/N 1060 REV 0                         P/N 1061 REV 1
|---------------------------------|    |---------------------------------|
|6522          SND.U17    SROM0   |    |  ITFB3             ITFB2        |
|   6809     6264   ENSONIC       |    |                                 |
|                                 |    |  ITFB7             ITFB6        |
|  LED1                           |    |                                 |
|        PAL1                     |    |  ITFB11            ITFB10       |
|                                 |    |                                 |
|                  *SROM2  SROM1  |    | *GROM18           *GROM13       |
|    ES5506  16MHz                |    |                                 |
|               MM5437            |    | *GROM19           *GROM14       |
|                                 |    |                                 |
|            PAL2                 |    |  ITFB1             ITFB0        |
|                                 |    |                                 |
|                           VOL   |    |  ITFB5             ITFB4        |
|                    TDA1543      |    |                                 |
|                                 |    |  ITFB9             ITFB8        |
|    555         3403    3403     |    |                                 |
|                                 |    | *GROM8            *GROM3        |
|                                 |    |                                 |
|        JP3                      |    | *GROM9            *GROM4        |
|---------------------------------|    |---------------------------------|

Notes:
      Same main board (P/N 1059 REV3) & sound PCB(P/N 1060 REV 0) as shown above.
      *      - These locations not populated

****************************************************************************

World Class Bowling
Incredible Technologies, 1995

PCB Layout
----------

Top (2 separate PCBs plugged into the main board)
---

P/N 1060 REV 0                         P/N 1079 REV 1
|---------------------------------|    |---------------------------------|
|6522          SND.U17    SROM0   |    |                                 |
|   6809     6264   ENSONIC       |    |  GRM0_3            GRM0_2       |
|                                 |    |                                 |
|  LED1                           |    |  GRM1_3            GRM1_2       |
|        PAL1                     |    |                                 |
|                                 |    | *GRM2_3           *GRM2_2       |
|                  *SROM2  SROM1  |    |                                 |
|    ES5506  16MHz                |    |                                 |
|               MM5437            |    |                                 |
|                                 |    |  GRM0_1            GRM0_0       |
|            PAL2                 |    |                                 |
|                                 |    |  GRM1_1            GRM1_0       |
|                           VOL   |    |                                 |
|                    TDA1543      |    | *GRM2_1           *GRM2_0       |
|                                 |    |                                 |
|    555         3403    3403     |    |                                 |
|                                 |    | ITBWL-1                      JP7|
|                                 |    | 4MHz                            |
|        JP3                      |    | LED                          JP8|
|---------------------------------|    |---------------------------------|

Notes:
      Same main board (P/N 1059 REV3) & sound PCB(P/N 1060 REV 0) as shown above.
      *       - These locations not populated
      ENSONIC - DIP42 chip labelled 'ENSONIC (C)1993 2MX16U 1350901801 9312 1.00' at location SROM0
              - This is actually a 16MBit DIP42 MaskROM
      ITBWL-1 PIC 16C54 used for protection
      JP7 6 Pin trackball connector (Player 1)
      JP8 6 Pin trackball connector (Player 2)


****************************************************************************/

#include "emu.h"
#include "cpu/tms32031/tms32031.h"
#include "cpu/m6800/m6800.h"
#include "deprecat.h"
#include "cpu/m6809/m6809.h"
#include "cpu/m68000/m68000.h"
#include "machine/6522via.h"
#include "machine/ticket.h"
#include "includes/itech32.h"
#include "sound/es5506.h"
#include "machine/timekpr.h"
#include "devconv.h"


#define FULL_LOGGING				0
#define LOG_DRIVEDGE_UNINIT_RAM		0



/*************************************
 *
 *  Static data
 *
 *************************************/

static UINT8 vint_state;
static UINT8 xint_state;
static UINT8 qint_state;

static UINT8 sound_data;
static UINT8 sound_return;
static UINT8 sound_int_state;

static UINT16 *main_rom;
static UINT16 *main_ram;
static size_t main_ram_size;
static UINT32 *nvram;
static size_t nvram_size;

static offs_t itech020_prot_address;

static UINT8 is_drivedge;

static UINT32 *tms1_ram, *tms2_ram;
static UINT32 *tms1_boot;
static UINT8 tms_spinning[2];

#define START_TMS_SPINNING(n)			do { cpu_spinuntil_trigger(space->cpu, 7351 + n); tms_spinning[n] = 1; } while (0)
#define STOP_TMS_SPINNING(machine, n)	do { cpuexec_trigger(machine, 7351 + n); tms_spinning[n] = 0; } while (0)



/*************************************
 *
 *  Interrupt handling
 *
 *************************************/

INLINE int determine_irq_state(int vint, int xint, int qint)
{
	int level = 0;


	/* determine which level is active */
	if (vint_state) level = 1;
	if (xint_state) level = 2;
	if (qint_state) level = 3;

	/* Driver's Edge shifts the interrupts a bit */
	if (is_drivedge && level) level += 2;

	return level;
}


void itech32_update_interrupts(running_machine *machine, int vint, int xint, int qint)
{
	/* update the states */
	if (vint != -1) vint_state = vint;
	if (xint != -1) xint_state = xint;
	if (qint != -1) qint_state = qint;

	if (is_drivedge)
	{
		cputag_set_input_line(machine, "maincpu", 3, vint_state ? ASSERT_LINE : CLEAR_LINE);
		cputag_set_input_line(machine, "maincpu", 4, xint_state ? ASSERT_LINE : CLEAR_LINE);
		cputag_set_input_line(machine, "maincpu", 5, qint_state ? ASSERT_LINE : CLEAR_LINE);
	}
	else
	{
		cputag_set_input_line(machine, "maincpu", 1, vint_state ? ASSERT_LINE : CLEAR_LINE);
		cputag_set_input_line(machine, "maincpu", 2, xint_state ? ASSERT_LINE : CLEAR_LINE);
		cputag_set_input_line(machine, "maincpu", 3, qint_state ? ASSERT_LINE : CLEAR_LINE);
	}
}


static INTERRUPT_GEN( generate_int1 )
{
	/* signal the NMI */
	itech32_update_interrupts(device->machine, 1, -1, -1);
	if (FULL_LOGGING) logerror("------------ VBLANK (%d) --------------\n", device->machine->primary_screen->vpos());
}


static WRITE16_HANDLER( int1_ack_w )
{
	itech32_update_interrupts(space->machine, 0, -1, -1);
}



/*************************************
 *
 *  Machine initialization
 *
 *************************************/

static MACHINE_RESET( itech32 )
{
	vint_state = xint_state = qint_state = 0;
	sound_data = 0;
	sound_return = 0;
	sound_int_state = 0;
}


static MACHINE_RESET( drivedge )
{
	MACHINE_RESET_CALL(itech32);

	cputag_set_input_line(machine, "dsp1", INPUT_LINE_RESET, ASSERT_LINE);
	cputag_set_input_line(machine, "dsp2", INPUT_LINE_RESET, ASSERT_LINE);
	STOP_TMS_SPINNING(machine, 0);
	STOP_TMS_SPINNING(machine, 1);
}



/*************************************
 *
 *  Input handlers
 *
 *************************************/

static int special_result = 0;

static CUSTOM_INPUT( special_port_r )
{
	if (sound_int_state)
		special_result ^= 1;

	return special_result;
}

static READ16_HANDLER( trackball_r )
{
	int lower = input_port_read(space->machine, "TRACKX1");
	int upper = input_port_read(space->machine, "TRACKY1");

	return (lower & 15) | ((upper & 15) << 4);
}


static READ32_HANDLER( trackball32_8bit_r )
{
	int lower = input_port_read(space->machine, "TRACKX1");
	int upper = input_port_read(space->machine, "TRACKY1");

	return (lower & 255) | ((upper & 255) << 8);
}


static READ32_HANDLER( trackball32_4bit_r )
{
	static int effx, effy;
	static int lastresult;
	static attotime lasttime;
	attotime curtime = timer_get_time(space->machine);

	if (attotime_compare(attotime_sub(curtime, lasttime), space->machine->primary_screen->scan_period()) > 0)
	{
		int upper, lower;
		int dx, dy;

		int curx = input_port_read(space->machine, "TRACKX1");
		int cury = input_port_read(space->machine, "TRACKY1");

		dx = curx - effx;
		if (dx < -0x80) dx += 0x100;
		else if (dx > 0x80) dx -= 0x100;
		if (dx > 7) dx = 7;
		else if (dx < -7) dx = -7;
		effx = (effx + dx) & 0xff;
		lower = effx & 15;

		dy = cury - effy;
		if (dy < -0x80) dy += 0x100;
		else if (dy > 0x80) dy -= 0x100;
		if (dy > 7) dy = 7;
		else if (dy < -7) dy = -7;
		effy = (effy + dy) & 0xff;
		upper = effy & 15;

		lastresult = lower | (upper << 4);
	}

	lasttime = curtime;
	return lastresult | (lastresult << 16);
}


static READ32_HANDLER( trackball32_4bit_p2_r )
{
	static int effx, effy;
	static int lastresult;
	static attotime lasttime;
	attotime curtime = timer_get_time(space->machine);

	if (attotime_compare(attotime_sub(curtime, lasttime), space->machine->primary_screen->scan_period()) > 0)
	{
		int upper, lower;
		int dx, dy;

		int curx = input_port_read(space->machine, "TRACKX2");
		int cury = input_port_read(space->machine, "TRACKY2");

		dx = curx - effx;
		if (dx < -0x80) dx += 0x100;
		else if (dx > 0x80) dx -= 0x100;
		if (dx > 7) dx = 7;
		else if (dx < -7) dx = -7;
		effx = (effx + dx) & 0xff;
		lower = effx & 15;

		dy = cury - effy;
		if (dy < -0x80) dy += 0x100;
		else if (dy > 0x80) dy -= 0x100;
		if (dy > 7) dy = 7;
		else if (dy < -7) dy = -7;
		effy = (effy + dy) & 0xff;
		upper = effy & 15;

		lastresult = lower | (upper << 4);
	}

	lasttime = curtime;
	return lastresult | (lastresult << 16);
}


static READ32_HANDLER( trackball32_4bit_combined_r )
{
	return trackball32_4bit_r(space, offset, mem_mask) |
			(trackball32_4bit_p2_r(space, offset, mem_mask) << 8);
}


static READ32_HANDLER( drivedge_steering_r )
{
	int val = input_port_read(space->machine, "STEER") * 2 - 0x100;
	if (val < 0) val = 0x100 | (-val);
	return val << 16;
}


static READ32_HANDLER( drivedge_gas_r )
{
	int val = input_port_read(space->machine, "GAS");
	return val << 16;
}


/*************************************
 *
 *  Protection is hangled through a PIC 16C54 MCU
 *
 *************************************/

static READ16_HANDLER( wcbowl_prot_result_r )
{
	return main_ram[0x111d/2];
}


static READ32_HANDLER( itech020_prot_result_r )
{
	UINT32 result = ((UINT32 *)main_ram)[itech020_prot_address >> 2];
	result >>= (~itech020_prot_address & 3) * 8;
	return (result & 0xff) << 8;
}


static READ32_HANDLER( gt2kp_prot_result_r )
{
	return 0x00010000;	/* 32 bit value at 680000 to 680003 will return the needed value of 0x01 */
}


static READ32_HANDLER( gtclass_prot_result_r )
{
	return 0x00008000;	/* 32 bit value at 680000 to 680003 will return the needed value of 0x80 */
}



/*************************************
 *
 *  Sound banking
 *
 *************************************/

static WRITE8_HANDLER( sound_bank_w )
{
	memory_set_bankptr(space->machine, "bank1", &memory_region(space->machine, "soundcpu")[0x10000 + data * 0x4000]);
}



/*************************************
 *
 *  Sound communication
 *
 *************************************/

static TIMER_CALLBACK( delayed_sound_data_w )
{
	sound_data = param;
	sound_int_state = 1;
	cputag_set_input_line(machine, "soundcpu", M6809_IRQ_LINE, ASSERT_LINE);
}


static WRITE16_HANDLER( sound_data_w )
{
	if (ACCESSING_BITS_0_7)
		timer_call_after_resynch(space->machine, NULL, data & 0xff, delayed_sound_data_w);
}


static READ32_HANDLER( sound_data32_r )
{
	return sound_return << 16;
}


static WRITE32_HANDLER( sound_data32_w )
{
	if (ACCESSING_BITS_16_23)
		timer_call_after_resynch(space->machine, NULL, (data >> 16) & 0xff, delayed_sound_data_w);
}


static READ8_HANDLER( sound_data_r )
{
	cputag_set_input_line(space->machine, "soundcpu", M6809_IRQ_LINE, CLEAR_LINE);
	sound_int_state = 0;
	return sound_data;
}


static WRITE8_HANDLER( sound_return_w )
{
	sound_return = data;
}


static READ8_HANDLER( sound_data_buffer_r )
{
	return 0;
}



/*************************************
 *
 *  Sound I/O port handling
 *
 *************************************/

static WRITE8_DEVICE_HANDLER( drivedge_portb_out )
{
	const address_space *space = cputag_get_address_space(device->machine, "maincpu", ADDRESS_SPACE_PROGRAM);
//  logerror("PIA port B write = %02x\n", data);

	/* bit 0 controls the fan light */
	/* bit 1 controls the tow light */
	/* bit 2 controls the horn light */
	/* bit 4 controls the ticket dispenser */
	/* bit 5 controls the coin counter */
	/* bit 6 controls the diagnostic sound LED */
	set_led_status(space->machine, 1, data & 0x01);
	set_led_status(space->machine, 2, data & 0x02);
	set_led_status(space->machine, 3, data & 0x04);
	ticket_dispenser_w(device->machine->device("ticket"), 0, (data & 0x10) << 3);
	coin_counter_w(space->machine, 0, (data & 0x20) >> 5);
}


static WRITE8_DEVICE_HANDLER( drivedge_turbo_light )
{
	set_led_status(device->machine, 0, data);
}


static WRITE8_DEVICE_HANDLER( pia_portb_out )
{
	const address_space *space = cputag_get_address_space(device->machine, "maincpu", ADDRESS_SPACE_PROGRAM);
//  logerror("PIA port B write = %02x\n", data);

	/* bit 4 controls the ticket dispenser */
	/* bit 5 controls the coin counter */
	/* bit 6 controls the diagnostic sound LED */
	ticket_dispenser_w(device->machine->device("ticket"), 0, (data & 0x10) << 3);
	coin_counter_w(space->machine, 0, (data & 0x20) >> 5);
}



/*************************************
 *
 *  Sound 6522 VIA handling
 *
 *************************************/

static const via6522_interface via_interface =
{
	/*inputs : A/B         */ DEVCB_NULL, DEVCB_NULL,
	/*inputs : CA/B1,CA/B2 */ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL,
	/*outputs: A/B         */ DEVCB_NULL, DEVCB_HANDLER(pia_portb_out),
	/*outputs: CA/B1,CA/B2 */ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL,
	/*irq                  */ DEVCB_CPU_INPUT_LINE("soundcpu", M6809_FIRQ_LINE)
};


static const via6522_interface drivedge_via_interface =
{
	/*inputs : A/B         */ DEVCB_NULL, DEVCB_NULL,
	/*inputs : CA/B1,CA/B2 */ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL,
	/*outputs: A/B         */ DEVCB_NULL, DEVCB_HANDLER(drivedge_portb_out),
	/*outputs: CA/B1,CA/B2 */ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_HANDLER(drivedge_turbo_light),
	/*irq                  */ DEVCB_CPU_INPUT_LINE("soundcpu", M6809_FIRQ_LINE)
};



/*************************************
 *
 *  Additional sound code
 *
 *************************************/

static WRITE8_HANDLER( firq_clear_w )
{
	cputag_set_input_line(space->machine, "soundcpu", M6809_FIRQ_LINE, CLEAR_LINE);
}



/*************************************
 *
 *  Driver's Edge stuff
 *
 *************************************/

static WRITE32_HANDLER( tms_reset_assert_w )
{
	cputag_set_input_line(space->machine, "dsp1", INPUT_LINE_RESET, ASSERT_LINE);
	cputag_set_input_line(space->machine, "dsp2", INPUT_LINE_RESET, ASSERT_LINE);
}


static WRITE32_HANDLER( tms_reset_clear_w )
{
	/* kludge to prevent crash on first boot */
	if ((tms1_ram[0] & 0xff000000) == 0)
	{
		cputag_set_input_line(space->machine, "dsp1", INPUT_LINE_RESET, CLEAR_LINE);
		STOP_TMS_SPINNING(space->machine, 0);
	}
	if ((tms2_ram[0] & 0xff000000) == 0)
	{
		cputag_set_input_line(space->machine, "dsp2", INPUT_LINE_RESET, CLEAR_LINE);
		STOP_TMS_SPINNING(space->machine, 1);
	}
}


static WRITE32_HANDLER( tms1_68k_ram_w )
{
	COMBINE_DATA(&tms1_ram[offset]);
	if (offset == 0) COMBINE_DATA(tms1_boot);
	if (offset == 0x382 && tms_spinning[0]) STOP_TMS_SPINNING(space->machine, 0);
	if (!tms_spinning[0])
		cpuexec_boost_interleave(space->machine, ATTOTIME_IN_HZ(CPU020_CLOCK/256), ATTOTIME_IN_USEC(20));
}


static WRITE32_HANDLER( tms2_68k_ram_w )
{
	COMBINE_DATA(&tms2_ram[offset]);
	if (offset == 0x382 && tms_spinning[1]) STOP_TMS_SPINNING(space->machine, 1);
	if (!tms_spinning[1])
		cpuexec_boost_interleave(space->machine, ATTOTIME_IN_HZ(CPU020_CLOCK/256), ATTOTIME_IN_USEC(20));
}


static WRITE32_HANDLER( tms1_trigger_w )
{
	COMBINE_DATA(&tms1_ram[offset]);
	cpuexec_boost_interleave(space->machine, ATTOTIME_IN_HZ(CPU020_CLOCK/256), ATTOTIME_IN_USEC(20));
}


static WRITE32_HANDLER( tms2_trigger_w )
{
	COMBINE_DATA(&tms2_ram[offset]);
	cpuexec_boost_interleave(space->machine, ATTOTIME_IN_HZ(CPU020_CLOCK/256), ATTOTIME_IN_USEC(20));
}


static READ32_HANDLER( drivedge_tms1_speedup_r )
{
	if (tms1_ram[0x382] == 0 && cpu_get_pc(space->cpu) == 0xee) START_TMS_SPINNING(0);
	return tms1_ram[0x382];
}


static READ32_HANDLER( drivedge_tms2_speedup_r )
{
	if (tms2_ram[0x382] == 0 && cpu_get_pc(space->cpu) == 0x809808) START_TMS_SPINNING(1);
	return tms2_ram[0x382];
}



/*************************************
 *
 *  32-bit shunts
 *
 *************************************/

static WRITE32_HANDLER( int1_ack32_w )
{
	int1_ack_w(space, offset, data, mem_mask);
}

static WRITE32_DEVICE_HANDLER( timekeeper_32be_w )
{
	write32be_with_write8_device_handler( timekeeper_w, device, offset, data, mem_mask );
}

static READ32_DEVICE_HANDLER( timekeeper_32be_r )
{
	return read32be_with_read8_device_handler( timekeeper_r, device, offset, mem_mask );
}



/*************************************
 *
 *  NVRAM read/write
 *
 *************************************/

static NVRAM_HANDLER( itech32 )
{
	int i;

	if (read_or_write)
		mame_fwrite(file, main_ram, main_ram_size);
	else if (file)
		mame_fread(file, main_ram, main_ram_size);
	else
	{
		for (i = 0x80; i < main_ram_size; i++)
			((UINT8 *)main_ram)[i] = mame_rand(machine);

		/* due to accessing uninitialized RAM, we need this hack */
		if (is_drivedge)
			((UINT32 *)main_ram)[0x2ce4/4] = 0x0000001e;
	}
}


static NVRAM_HANDLER( itech020 )
{
	int i;

	if (read_or_write)
		mame_fwrite(file, nvram, nvram_size);
	else if (file)
		mame_fread(file, nvram, nvram_size);
	else
	{
		for (i = 0; i < nvram_size; i++)
			((UINT8 *)nvram)[i] = mame_rand(machine);
	}
}


/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

/*------ Time Killers memory layout ------*/
static ADDRESS_MAP_START( timekill_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x003fff) AM_RAM AM_BASE(&main_ram) AM_SIZE(&main_ram_size)
	AM_RANGE(0x040000, 0x040001) AM_READ_PORT("P1")
	AM_RANGE(0x048000, 0x048001) AM_READ_PORT("P2")
	AM_RANGE(0x050000, 0x050001) AM_READ_PORT("SYSTEM") AM_WRITE(timekill_intensity_w)
	AM_RANGE(0x058000, 0x058001) AM_READ_PORT("DIPS") AM_WRITE(watchdog_reset16_w)
	AM_RANGE(0x060000, 0x060001) AM_WRITE(timekill_colora_w)
	AM_RANGE(0x068000, 0x068001) AM_WRITE(timekill_colorbc_w)
	AM_RANGE(0x070000, 0x070001) AM_WRITENOP	/* noisy */
	AM_RANGE(0x078000, 0x078001) AM_WRITE(sound_data_w)
	AM_RANGE(0x080000, 0x08007f) AM_READWRITE(itech32_video_r, itech32_video_w) AM_BASE(&itech32_video)
	AM_RANGE(0x0a0000, 0x0a0001) AM_WRITE(int1_ack_w)
	AM_RANGE(0x0c0000, 0x0c7fff) AM_RAM_WRITE(timekill_paletteram_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x100000, 0x17ffff) AM_ROM AM_REGION("user1", 0) AM_BASE(&main_rom)
ADDRESS_MAP_END


/*------ BloodStorm and later games memory layout ------*/
static ADDRESS_MAP_START( bloodstm_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x00ffff) AM_RAM AM_BASE(&main_ram) AM_SIZE(&main_ram_size)
	AM_RANGE(0x080000, 0x080001) AM_READ_PORT("P1") AM_WRITE(int1_ack_w)
	AM_RANGE(0x100000, 0x100001) AM_READ_PORT("P2")
	AM_RANGE(0x180000, 0x180001) AM_READ_PORT("P3")
	AM_RANGE(0x200000, 0x200001) AM_READ_PORT("P4") AM_WRITE(watchdog_reset16_w)
	AM_RANGE(0x280000, 0x280001) AM_READ_PORT("DIPS")
	AM_RANGE(0x300000, 0x300001) AM_WRITE(bloodstm_color1_w)
	AM_RANGE(0x380000, 0x380001) AM_WRITE(bloodstm_color2_w)
	AM_RANGE(0x400000, 0x400001) AM_WRITE(watchdog_reset16_w)
	AM_RANGE(0x480000, 0x480001) AM_WRITE(sound_data_w)
	AM_RANGE(0x500000, 0x5000ff) AM_READWRITE(bloodstm_video_r, bloodstm_video_w) AM_BASE(&itech32_video)
	AM_RANGE(0x580000, 0x59ffff) AM_RAM_WRITE(bloodstm_paletteram_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x700000, 0x700001) AM_WRITE(bloodstm_plane_w)
	AM_RANGE(0x780000, 0x780001) AM_READ_PORT("EXTRA")
	AM_RANGE(0x800000, 0x87ffff) AM_MIRROR(0x780000) AM_ROM AM_REGION("user1", 0) AM_BASE(&main_rom)
ADDRESS_MAP_END


/*------ Driver's Edge memory layouts ------*/

#if LOG_DRIVEDGE_UNINIT_RAM
static UINT8 written[0x8000];

static READ32_HANDLER( test1_r )
{
	if (ACCESSING_BITS_24_31 && !written[0x100 + offset*4+0]) logerror("%06X:read from uninitialized memory %04X\n", cpu_get_pc(space->cpu), 0x100 + offset*4+0);
	if (ACCESSING_BITS_16_23 && !written[0x100 + offset*4+1]) logerror("%06X:read from uninitialized memory %04X\n", cpu_get_pc(space->cpu), 0x100 + offset*4+1);
	if (ACCESSING_BITS_8_15 && !written[0x100 + offset*4+2]) logerror("%06X:read from uninitialized memory %04X\n", cpu_get_pc(space->cpu), 0x100 + offset*4+2);
	if (ACCESSING_BITS_0_7 && !written[0x100 + offset*4+3]) logerror("%06X:read from uninitialized memory %04X\n", cpu_get_pc(space->cpu), 0x100 + offset*4+3);
	return ((UINT32 *)main_ram)[0x100/4 + offset];
}

static WRITE32_HANDLER( test1_w )
{
	if (ACCESSING_BITS_24_31) written[0x100 + offset*4+0] = 1;
	if (ACCESSING_BITS_16_23) written[0x100 + offset*4+1] = 1;
	if (ACCESSING_BITS_8_15) written[0x100 + offset*4+2] = 1;
	if (ACCESSING_BITS_0_7) written[0x100 + offset*4+3] = 1;
	COMBINE_DATA(&((UINT32 *)main_ram)[0x100/4 + offset]);
}

static READ32_HANDLER( test2_r )
{
	if (ACCESSING_BITS_24_31 && !written[0xc00 + offset*4+0]) logerror("%06X:read from uninitialized memory %04X\n", cpu_get_pc(space->cpu), 0xc00 + offset*4+0);
	if (ACCESSING_BITS_16_23 && !written[0xc00 + offset*4+1]) logerror("%06X:read from uninitialized memory %04X\n", cpu_get_pc(space->cpu), 0xc00 + offset*4+1);
	if (ACCESSING_BITS_8_15 && !written[0xc00 + offset*4+2]) logerror("%06X:read from uninitialized memory %04X\n", cpu_get_pc(space->cpu), 0xc00 + offset*4+2);
	if (ACCESSING_BITS_0_7 && !written[0xc00 + offset*4+3]) logerror("%06X:read from uninitialized memory %04X\n", cpu_get_pc(space->cpu), 0xc00 + offset*4+3);
	return ((UINT32 *)main_ram)[0xc00/4 + offset];
}

static WRITE32_HANDLER( test2_w )
{
	if (ACCESSING_BITS_24_31) written[0xc00 + offset*4+0] = 1;
	if (ACCESSING_BITS_16_23) written[0xc00 + offset*4+1] = 1;
	if (ACCESSING_BITS_8_15) written[0xc00 + offset*4+2] = 1;
	if (ACCESSING_BITS_0_7) written[0xc00 + offset*4+3] = 1;
	COMBINE_DATA(&((UINT32 *)main_ram)[0xc00/4 + offset]);
}
#endif

static ADDRESS_MAP_START( drivedge_map, ADDRESS_SPACE_PROGRAM, 32 )
#if LOG_DRIVEDGE_UNINIT_RAM
AM_RANGE(0x000100, 0x0003ff) AM_MIRROR(0x40000) AM_READWRITE(test1_r, test1_w)
AM_RANGE(0x000c00, 0x007fff) AM_MIRROR(0x40000) AM_READWRITE(test2_r, test2_w)
#endif
	AM_RANGE(0x000000, 0x03ffff) AM_MIRROR(0x40000) AM_RAM AM_BASE((UINT32 **)&main_ram) AM_SIZE(&main_ram_size)
	AM_RANGE(0x080000, 0x080003) AM_READ_PORT("80000")
	AM_RANGE(0x082000, 0x082003) AM_READ_PORT("82000")
	AM_RANGE(0x084000, 0x084003) AM_READWRITE(sound_data32_r, sound_data32_w)
//  AM_RANGE(0x086000, 0x08623f) AM_RAM -- networking -- first 0x40 bytes = our data, next 0x40*8 bytes = their data, r/w on IRQ2
	AM_RANGE(0x088000, 0x088003) AM_READ(drivedge_steering_r)
	AM_RANGE(0x08a000, 0x08a003) AM_READ(drivedge_gas_r) AM_WRITENOP
	AM_RANGE(0x08c000, 0x08c003) AM_READ_PORT("8c000")
	AM_RANGE(0x08e000, 0x08e003) AM_READ_PORT("8e000") AM_WRITENOP
	AM_RANGE(0x100000, 0x10000f) AM_WRITE(drivedge_zbuf_control_w) AM_BASE(&drivedge_zbuf_control)
	AM_RANGE(0x180000, 0x180003) AM_WRITE(drivedge_color0_w)
	AM_RANGE(0x1a0000, 0x1bffff) AM_RAM_WRITE(drivedge_paletteram_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x1c0000, 0x1c0003) AM_WRITENOP
	AM_RANGE(0x1e0000, 0x1e0113) AM_READWRITE(itech020_video_r, itech020_video_w) AM_BASE((UINT32 **)&itech32_video)
	AM_RANGE(0x1e4000, 0x1e4003) AM_WRITE(tms_reset_assert_w)
	AM_RANGE(0x1ec000, 0x1ec003) AM_WRITE(tms_reset_clear_w)
	AM_RANGE(0x200000, 0x200003) AM_READ_PORT("200000")
	AM_RANGE(0x280000, 0x280fff) AM_RAM_WRITE(tms1_68k_ram_w) AM_SHARE("share1")
	AM_RANGE(0x300000, 0x300fff) AM_RAM_WRITE(tms2_68k_ram_w) AM_SHARE("share2")
	AM_RANGE(0x380000, 0x380003) AM_WRITENOP // AM_WRITE(watchdog_reset16_w)
	AM_RANGE(0x600000, 0x607fff) AM_ROM AM_REGION("user1", 0) AM_BASE((UINT32 **)&main_rom)
ADDRESS_MAP_END

static ADDRESS_MAP_START( drivedge_tms1_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x000000, 0x001fff) AM_RAM AM_BASE(&tms1_boot)
	AM_RANGE(0x008000, 0x0083ff) AM_MIRROR(0x400) AM_RAM_WRITE(tms1_trigger_w) AM_SHARE("share1") AM_BASE(&tms1_ram)
	AM_RANGE(0x080000, 0x0bffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( drivedge_tms2_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x008000, 0x0083ff) AM_MIRROR(0x8400) AM_RAM_WRITE(tms2_trigger_w) AM_SHARE("share2") AM_BASE(&tms2_ram)
	AM_RANGE(0x080000, 0x08ffff) AM_RAM
ADDRESS_MAP_END


/*------ 68EC020-based memory layout ------*/
static ADDRESS_MAP_START( itech020_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x000000, 0x007fff) AM_RAM AM_BASE((UINT32 **)&main_ram) AM_SIZE(&main_ram_size)
	AM_RANGE(0x080000, 0x080003) AM_READ_PORT("P1") AM_WRITE(int1_ack32_w)
	AM_RANGE(0x100000, 0x100003) AM_READ_PORT("P2")
	AM_RANGE(0x180000, 0x180003) AM_READ_PORT("P3")
	AM_RANGE(0x200000, 0x200003) AM_READ_PORT("P4")
	AM_RANGE(0x280000, 0x280003) AM_READ_PORT("DIPS")
	AM_RANGE(0x300000, 0x300003) AM_WRITE(itech020_color1_w)
	AM_RANGE(0x380000, 0x380003) AM_WRITE(itech020_color2_w)
	AM_RANGE(0x400000, 0x400003) AM_WRITE(watchdog_reset32_w)
	AM_RANGE(0x480000, 0x480003) AM_WRITE(sound_data32_w)
	AM_RANGE(0x500000, 0x5000ff) AM_READWRITE(itech020_video_r, itech020_video_w) AM_BASE((UINT32 **)&itech32_video)
	AM_RANGE(0x578000, 0x57ffff) AM_READNOP				/* touched by protection */
	AM_RANGE(0x580000, 0x59ffff) AM_RAM_WRITE(itech020_paletteram_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x600000, 0x603fff) AM_RAM AM_BASE(&nvram) AM_SIZE(&nvram_size)
/* ? */	AM_RANGE(0x61ff00, 0x61ffff) AM_WRITENOP			/* Unknown Writes */
	AM_RANGE(0x680000, 0x680003) AM_READ(itech020_prot_result_r) AM_WRITENOP
/* ! */	AM_RANGE(0x680800, 0x68083f) AM_READONLY AM_WRITENOP /* Serial DUART Channel A/B & Top LED sign - To Do! */
	AM_RANGE(0x700000, 0x700003) AM_WRITE(itech020_plane_w)
	AM_RANGE(0x800000, 0xbfffff) AM_ROM AM_REGION("user1", 0) AM_BASE((UINT32 **)&main_rom)
ADDRESS_MAP_END



/*************************************
 *
 *  Sound CPU memory handlers
 *
 *************************************/

/*------ Rev 1 sound board memory layout ------*/
static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0000) AM_WRITE(sound_return_w)
	AM_RANGE(0x0400, 0x0400) AM_READ(sound_data_r)
	AM_RANGE(0x0800, 0x083f) AM_MIRROR(0x80) AM_DEVREADWRITE("ensoniq", es5506_r, es5506_w)
	AM_RANGE(0x0c00, 0x0c00) AM_WRITE(sound_bank_w)
	AM_RANGE(0x1000, 0x1000) AM_WRITENOP	/* noisy */
	AM_RANGE(0x1400, 0x140f) AM_DEVREADWRITE("via6522_0", via_r, via_w)
	AM_RANGE(0x2000, 0x3fff) AM_RAM
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank1")
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END


/*------ Rev 2 sound board memory layout ------*/
static ADDRESS_MAP_START( sound_020_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0000) AM_MIRROR(0x400) AM_READ(sound_data_r)
	AM_RANGE(0x0800, 0x083f) AM_MIRROR(0x80) AM_DEVREADWRITE("ensoniq", es5506_r, es5506_w)
	AM_RANGE(0x0c00, 0x0c00) AM_WRITE(sound_bank_w)
	AM_RANGE(0x1400, 0x1400) AM_WRITE(firq_clear_w)
	AM_RANGE(0x1800, 0x1800) AM_READ(sound_data_buffer_r) AM_WRITENOP
	AM_RANGE(0x2000, 0x3fff) AM_RAM
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank1")
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( timekill )
	PORT_START("P1")		/* 40000 */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)

	PORT_START("P2")		/* 48000 */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)

	PORT_START("SYSTEM")	/* 50000 */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DIPS")		/* 58000 */
	PORT_SERVICE_NO_TOGGLE( 0x0001, IP_ACTIVE_LOW )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_VBLANK )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(special_port_r, NULL)
	PORT_DIPNAME( 0x0010, 0x0000, "Video Sync" )	 PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0000, "-" )
	PORT_DIPSETTING(      0x0010, "+" )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, "Violence" )	 PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x0080, IP_ACTIVE_HIGH, "SW1:4" )
INPUT_PORTS_END


static INPUT_PORTS_START( itech32_base_16bit )
	PORT_START("P1")	/* 080000 */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)

	PORT_START("P2")	/* 100000 */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)

	PORT_START("P3")	/* 180000 */
	PORT_BIT( 0x00ff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P4")	/* 200000 */
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DIPS")	/* 280000 */
	PORT_SERVICE_NO_TOGGLE( 0x0001, IP_ACTIVE_LOW )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_VBLANK )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(special_port_r, NULL)
	PORT_DIPNAME( 0x0010, 0x0000, "Video Sync" )	 PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0000, "-" )
	PORT_DIPSETTING(      0x0010, "+" )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x0080, IP_ACTIVE_HIGH, "SW1:1" )

	PORT_START("EXTRA")	/* 78000 */
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( bloodstm )
	PORT_INCLUDE( itech32_base_16bit )

	PORT_MODIFY("P1")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)

	PORT_MODIFY("P2")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_MODIFY("DIPS")
	PORT_DIPNAME( 0x0040, 0x0000, "Violence" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_MODIFY("EXTRA")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x00c0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( hardyard )
	PORT_INCLUDE( itech32_base_16bit )

	PORT_MODIFY("P3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(3)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(3)

	PORT_MODIFY("P4")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(4)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(4)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(4)

	PORT_MODIFY("DIPS")	/* 280000 */
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Upright ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Players ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPSETTING(      0x0040, "2" )
INPUT_PORTS_END


static INPUT_PORTS_START( pairs )
	PORT_INCLUDE( itech32_base_16bit )

	PORT_MODIFY("P1")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("P2")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("DIPS")
	PORT_DIPNAME( 0x0040, 0x0000, "Modesty" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( drivedge )
	PORT_START("8c000")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Gear 1") PORT_CODE(KEYCODE_Z) PORT_PLAYER(1)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("Gear 2") PORT_CODE(KEYCODE_X) PORT_PLAYER(1)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("Gear 3") PORT_CODE(KEYCODE_C) PORT_PLAYER(1)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("Gear 4") PORT_CODE(KEYCODE_V) PORT_PLAYER(1)
	PORT_SERVICE_NO_TOGGLE( 0x00400000, IP_ACTIVE_LOW )
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("8e000")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Fan") PORT_CODE(KEYCODE_F) PORT_PLAYER(1)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Tow Truck") PORT_CODE(KEYCODE_T) PORT_PLAYER(1)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Horn") PORT_CODE(KEYCODE_SPACE) PORT_PLAYER(1)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Turbo Boost") PORT_CODE(KEYCODE_B) PORT_PLAYER(1)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Network On") PORT_CODE(KEYCODE_N) PORT_PLAYER(1)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Key")
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("200000")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_VBLANK )
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_SPECIAL )
	PORT_DIPNAME( 0x70000000, 0x00000000, "Network Number" ) PORT_DIPLOCATION("SW1:4,3,2")
	PORT_DIPSETTING(          0x00000000, "1" )
	PORT_DIPSETTING(          0x10000000, "2" )
	PORT_DIPSETTING(          0x20000000, "3" )
	PORT_DIPSETTING(          0x30000000, "4" )
	PORT_DIPSETTING(          0x40000000, "5" )
	PORT_DIPSETTING(          0x50000000, "6" )
	PORT_DIPSETTING(          0x60000000, "7" )
	PORT_DIPSETTING(          0x70000000, "8" )
	PORT_SERVICE_DIPLOC( 0x80000000, IP_ACTIVE_HIGH, "SW1:1" )

	PORT_START("80000")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x00ff0000, 0x00000000, IPT_PEDAL3 ) PORT_MINMAX(0x00000000,0x00060000) PORT_SENSITIVITY(2) PORT_KEYDELTA(100)

	PORT_START("82000")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x00ff0000, 0x00000000, IPT_PEDAL2 ) PORT_MINMAX(0x00000000,0x00060000) PORT_SENSITIVITY(2) PORT_KEYDELTA(100)	/* Brake */

	PORT_START("STEER")		/* 88000 */
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(25) PORT_KEYDELTA(5)

	PORT_START("GAS")		/* 8A000 */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_MINMAX(0x00,0x0c) PORT_SENSITIVITY(1) PORT_KEYDELTA(20) PORT_PLAYER(1)
INPUT_PORTS_END


static INPUT_PORTS_START( wcbowl )
	PORT_INCLUDE( itech32_base_16bit )

	PORT_MODIFY("P1")
	PORT_BIT( 0x00f0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("P2")
	PORT_BIT( 0x00f0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("TRACKX1")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(25) PORT_KEYDELTA(32) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("TRACKY1")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(25) PORT_KEYDELTA(32) PORT_PLAYER(1)
INPUT_PORTS_END


static INPUT_PORTS_START( itech32_base_32bit )
	PORT_START("P1")	/* 080000 */
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)

	PORT_START("P2")	/* 100000 */
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)

	PORT_START("P3")	/* 180000 */
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x00ff0000, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P4")	/* 200000 */
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x00ff0000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DIPS")	/* 280000 */
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x00010000, IP_ACTIVE_LOW )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_VBLANK )
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM(special_port_r, NULL)
	PORT_DIPNAME( 0x00100000, 0x00000000, "Video Sync" )	 PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(          0x00000000, "-" )
	PORT_DIPSETTING(          0x00100000, "+" )
	PORT_DIPNAME( 0x00200000, 0x00000000, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00200000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00400000, 0x00000000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(          0x00400000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x00800000, IP_ACTIVE_HIGH, "SW1:1" )
INPUT_PORTS_END


static INPUT_PORTS_START( wcbowln ) /* WCB version 1.66 supports cocktail mode */
	PORT_INCLUDE( itech32_base_32bit )

	PORT_MODIFY("P1")
	PORT_BIT( 0x00f00000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("P2")
	PORT_BIT( 0x00f00000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("P3")
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x00fb0000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("DIPS")
	PORT_DIPNAME( 0x00100000, 0x00000000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00100000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00200000, 0x00000000, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:3") /* v1.66 Rom sets support Cocktail mode (verified) */
	PORT_DIPSETTING(          0x00000000, DEF_STR( Upright ) )
	PORT_DIPSETTING(          0x00200000, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x00400000, 0x00000000, "Freeze Screen" )	 PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00400000, DEF_STR( On ) )

	PORT_START("TRACKX1")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(25) PORT_KEYDELTA(32) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("TRACKY1")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(25) PORT_KEYDELTA(32) PORT_PLAYER(1)

	PORT_START("TRACKX2")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(25) PORT_KEYDELTA(32) PORT_REVERSE PORT_COCKTAIL PORT_PLAYER(2)

	PORT_START("TRACKY2")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(25) PORT_KEYDELTA(32) PORT_COCKTAIL PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( wcbowldx )
	PORT_INCLUDE( wcbowln )

	PORT_MODIFY("DIPS")	/* 280000 */
	PORT_DIPNAME( 0x00200000, 0x00000000, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:3")	/* Verified */
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00200000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00400000, 0x00000000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00400000, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( sftm )
	PORT_INCLUDE( itech32_base_32bit )

	PORT_MODIFY("P3")
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)

	PORT_MODIFY("DIPS")
	PORT_DIPNAME( 0x00400000, 0x00000000, "Freeze Screen" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00400000, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( shufshot ) /* ShuffleShot v1.39 & v1.40 support cocktail mode */
	PORT_INCLUDE( itech32_base_32bit )

	PORT_MODIFY("P1")
	PORT_BIT( 0x00f00000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("P2")
	PORT_BIT( 0x00f00000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("P3")
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x00fb0000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("DIPS")
	PORT_DIPNAME( 0x00200000, 0x00000000, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:3")	/* Verified */
	PORT_DIPSETTING(          0x00000000, DEF_STR( Upright ) )
	PORT_DIPSETTING(          0x00200000, DEF_STR( Cocktail ) )

	PORT_START("TRACKX1")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(25) PORT_KEYDELTA(32) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("TRACKY1")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(25) PORT_KEYDELTA(32) PORT_PLAYER(1)

	PORT_START("TRACKX2")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(25) PORT_KEYDELTA(32) PORT_REVERSE PORT_COCKTAIL PORT_PLAYER(2)

	PORT_START("TRACKY2")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(25) PORT_KEYDELTA(32) PORT_COCKTAIL PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( shufbowl )
	/*
    Earlier versions of Shuffleshot & World Class Bowling share the same input
    port set up. IE: "Freeze Screen" and no support for a cocktail mode
    */
	PORT_INCLUDE( itech32_base_32bit )

	PORT_MODIFY("P1")
	PORT_BIT( 0x00f00000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("P2")
	PORT_BIT( 0x00f00000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("P3")
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x00fb0000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("DIPS")
	PORT_DIPNAME( 0x00400000, 0x00000000, "Freeze Screen" )	 PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00400000, DEF_STR( On ) )

	PORT_START("TRACKX1")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(25) PORT_KEYDELTA(32) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("TRACKY1")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(25) PORT_KEYDELTA(32) PORT_PLAYER(1)

	PORT_START("TRACKX2")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(25) PORT_KEYDELTA(32) PORT_REVERSE PORT_COCKTAIL PORT_PLAYER(2)

	PORT_START("TRACKY2")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(25) PORT_KEYDELTA(32) PORT_COCKTAIL PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( gt3d )
	PORT_INCLUDE( itech32_base_32bit )

	PORT_MODIFY("P1")
	PORT_BIT( 0x00f00000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("P2")
	PORT_BIT( 0x00f00000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("P3")
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x00f80000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("DIPS")
	PORT_DIPNAME( 0x00100000, 0x00000000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00100000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00200000, 0x00000000, "Trackball Orientation" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(          0x00000000, "Normal Mount" )
	PORT_DIPSETTING(          0x00200000, "45 Degree Angle" )
	PORT_DIPNAME( 0x00400000, 0x00000000, DEF_STR( Controls ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(          0x00000000, "One Trackball" )
	PORT_DIPSETTING(          0x00400000, "Two Trackballs" )

	PORT_START("TRACKX1")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(25) PORT_KEYDELTA(32) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("TRACKY1")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(25) PORT_KEYDELTA(32) PORT_PLAYER(1)

	PORT_START("TRACKX2")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(25) PORT_KEYDELTA(32) PORT_REVERSE PORT_COCKTAIL PORT_PLAYER(2)

	PORT_START("TRACKY2")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(25) PORT_KEYDELTA(32) PORT_COCKTAIL PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( gt97 ) /* v1.30 is the first known version of GT97 to support Cocktail mode! */
	PORT_INCLUDE(gt3d)

	PORT_MODIFY("DIPS")
	PORT_DIPNAME( 0x00100000, 0x00000000, "Freeze Screen" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00100000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00200000, 0x00000000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:3")	/* Seem to have no effect on the game */
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00200000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00400000, 0x00000000, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(          0x00000000, DEF_STR( Upright ) )
	PORT_DIPSETTING(          0x00400000, DEF_STR( Cocktail ) )
INPUT_PORTS_END


static INPUT_PORTS_START( gt97o ) /* Older versions of GT97 do NOT support a cocktail mode */
	PORT_INCLUDE(gt3d)

	PORT_MODIFY("DIPS")
	PORT_DIPNAME( 0x00100000, 0x00000000, "Freeze Screen" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00100000, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( gt97s )
	PORT_INCLUDE(gt3d)

	PORT_MODIFY("DIPS")
	PORT_DIPNAME( 0x00100000, 0x00000000, "Freeze Screen" )	 PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00100000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00400000, 0x00000000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:2")	/* Single controller version -  has no effect */
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00400000, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( gt98 )
	PORT_INCLUDE(gt3d)

	PORT_MODIFY("DIPS")
	PORT_DIPNAME( 0x00200000, 0x00000000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:3")	/* Seem to have no effect on the game */
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00200000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00400000, 0x00000000, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(          0x00000000, DEF_STR( Upright ) )
	PORT_DIPSETTING(          0x00400000, DEF_STR( Cocktail ) )
INPUT_PORTS_END


static INPUT_PORTS_START( gt98s )
	PORT_INCLUDE(gt3d)

	PORT_MODIFY("DIPS")
	PORT_DIPNAME( 0x00200000, 0x00000000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:3")	/* Seem to have no effect on the game */
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00200000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00400000, 0x00000000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:2")	/* Single controller version -  has no effect */
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00400000, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( s_ver )
	PORT_INCLUDE(gt3d)

	PORT_MODIFY("DIPS")
	PORT_DIPNAME( 0x00100000, 0x00000000, "Trackball Orientation" ) PORT_DIPLOCATION("SW1:4")	/* Determined by actual use / trial & error */
	PORT_DIPSETTING(          0x00000000, "Normal Mount" )						/* The manual says "Always on (defualt)" and "Off -- UNUSED --" */
	PORT_DIPSETTING(          0x00100000, "45 Degree Angle" )
	PORT_DIPNAME( 0x00200000, 0x00000000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:3")	/* Single controller version -  has no effect */
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00200000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00400000, 0x00000000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:2")	/* Single controller version -  has no effect */
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00400000, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( aama )
	PORT_INCLUDE(gt3d)

	PORT_MODIFY("DIPS")
	PORT_DIPNAME( 0x00100000, 0x00000000, "Trackball Orientation" ) PORT_DIPLOCATION("SW1:4")	/* Determined by actual use / trial & error */
	PORT_DIPSETTING(          0x00000000, "Normal Mount" )						/* The manual says "Always on (defualt)" and "Off -- UNUSED --" */
	PORT_DIPSETTING(          0x00100000, "45 Degree Angle" )
	PORT_DIPNAME( 0x00200000, 0x00000000, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(          0x00000000, DEF_STR( Upright ) )
	PORT_DIPSETTING(          0x00200000, DEF_STR( Cocktail ) )					/* Cocktail mode REQUIRES "Controls" to be set to "Two Trackballs" */
	PORT_DIPNAME( 0x00400000, 0x00000000, DEF_STR( Controls ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(          0x00000000, "One Trackball" )
	PORT_DIPSETTING(          0x00400000, "Two Trackballs" )					/* Two Trackballs will work for Upright for "side by side" controls */
INPUT_PORTS_END



/*************************************
 *
 *  Sound definitions
 *
 *************************************/

static const es5506_interface es5506_config =
{
	"ensoniq.0",
	"ensoniq.1",
	"ensoniq.2",
	"ensoniq.3"
};



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_DRIVER_START( timekill )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, CPU_CLOCK)
	MDRV_CPU_PROGRAM_MAP(timekill_map)
	MDRV_CPU_VBLANK_INT("screen", generate_int1)

	MDRV_CPU_ADD("soundcpu", M6809, SOUND_CLOCK/8)
	MDRV_CPU_PROGRAM_MAP(sound_map)

	MDRV_MACHINE_RESET(itech32)
	MDRV_NVRAM_HANDLER(itech32)

	MDRV_TICKET_DISPENSER_ADD("ticket", 200, TICKET_MOTOR_ACTIVE_HIGH, TICKET_STATUS_ACTIVE_HIGH)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MDRV_PALETTE_LENGTH(8192)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_RAW_PARAMS(VIDEO_CLOCK, 508, 0, 384, 262, 0, 256)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)

	MDRV_VIDEO_START(itech32)
	MDRV_VIDEO_UPDATE(itech32)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ensoniq", ES5506, SOUND_CLOCK)
	MDRV_SOUND_CONFIG(es5506_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	/* via */
	MDRV_VIA6522_ADD("via6522_0", SOUND_CLOCK/8, via_interface)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( bloodstm )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(timekill)

	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(bloodstm_map)

	/* video hardware */
	MDRV_PALETTE_LENGTH(32768)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( drivedge )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(bloodstm)

	MDRV_CPU_REPLACE("maincpu", M68EC020, CPU020_CLOCK)
	MDRV_CPU_PROGRAM_MAP(drivedge_map)

	MDRV_CPU_ADD("dsp1", TMS32031, TMS_CLOCK)
	MDRV_CPU_PROGRAM_MAP(drivedge_tms1_map)

	MDRV_CPU_ADD("dsp2", TMS32031, TMS_CLOCK)
	MDRV_CPU_PROGRAM_MAP(drivedge_tms2_map)

//  MDRV_CPU_ADD("comm", M6803, 8000000/4) -- network CPU

	MDRV_MACHINE_RESET(drivedge)
	MDRV_QUANTUM_TIME(HZ(6000))
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( sftm )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(bloodstm)

	MDRV_CPU_REPLACE("maincpu", M68EC020, CPU020_CLOCK)
	MDRV_CPU_PROGRAM_MAP(itech020_map)
	MDRV_CPU_VBLANK_INT("screen", generate_int1)

	MDRV_CPU_MODIFY("soundcpu")
	MDRV_CPU_PROGRAM_MAP(sound_020_map)
	MDRV_CPU_VBLANK_INT_HACK(irq1_line_assert,4)

	MDRV_NVRAM_HANDLER(itech020)

	/* via */
	MDRV_DEVICE_REMOVE("via6522_0")
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( tourny )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(sftm)

	MDRV_M48T02_ADD( "m48t02"  )
MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

/* Maximum sftm code size */
#undef  CODE_SIZE
#define CODE_SIZE   0x0400000

ROM_START( timekill )
	ROM_REGION16_BE( 0x80000, "user1", 0 )
	ROM_LOAD16_BYTE( "tk00v132.u54", 0x00000, 0x40000, CRC(68c74b81) SHA1(acdf677f82d7428acc6cf01076d43dd6330e9cb3) )
	ROM_LOAD16_BYTE( "tk01v132.u53", 0x00001, 0x40000, CRC(2158d8ef) SHA1(14aa66e020a9fa890fadbaf0936dfdc4e272f543) )

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "tksndv41.u17", 0x10000, 0x18000, CRC(c699af7b) SHA1(55863513a1c27dcb257dbc20e522cfafa9b92c9d) )
	ROM_CONTINUE(             0x08000, 0x08000 )

	ROM_REGION( 0x880000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "tk0_rom.1", 0x000000, 0x200000, CRC(94cbf6f8) SHA1(dac5c4d9c8e42336c236ecc3c72b3b1f8282dc2f) )
	ROM_LOAD32_BYTE( "tk1_rom.2", 0x000001, 0x200000, CRC(c07dea98) SHA1(dd8b88beb9781579eb0a17231ad2a274b70ae1bc) )
	ROM_LOAD32_BYTE( "tk2_rom.3", 0x000002, 0x200000, CRC(183eed2a) SHA1(3905268fe45ecc47cd4d349666b4d33efda2140b) )
	ROM_LOAD32_BYTE( "tk3_rom.4", 0x000003, 0x200000, CRC(b1da1058) SHA1(a1d483701c661d69cecc9d073b23683b119f5ef1) )
	ROM_LOAD32_BYTE( "tkgrom.01", 0x800000, 0x020000, CRC(b030c3d9) SHA1(f5c21285ec8ff4f74205e0cf18da67e733e31183) )
	ROM_LOAD32_BYTE( "tkgrom.02", 0x800001, 0x020000, CRC(e98492a4) SHA1(fe8fb4bd3900109f3872f2930e8ddc9d19f599fd) )
	ROM_LOAD32_BYTE( "tkgrom.03", 0x800002, 0x020000, CRC(6088fa64) SHA1(a3eee10bdef48fefec3836f551172dbe0819acf6) )
	ROM_LOAD32_BYTE( "tkgrom.04", 0x800003, 0x020000, CRC(95be2318) SHA1(60580c87d63a114df44e2580e138128388ff447b) )

	ROM_REGION16_BE( 0x400000, "ensoniq.2", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "tksrom00.u18", 0x000000, 0x80000, CRC(79d8b83a) SHA1(78934b4d0ccca8fefcf8277e4296eb1d59cd575b) )
	ROM_LOAD16_BYTE( "tksrom01.u20", 0x100000, 0x80000, CRC(ec01648c) SHA1(b83c66cf22db5d89b9ed79b79861b79429d8380c) )
	ROM_LOAD16_BYTE( "tksrom02.u26", 0x200000, 0x80000, CRC(051ced3e) SHA1(6b63c4837e709806ffea9a37d93933635d356a6e) )
ROM_END


ROM_START( timekill131 )
	ROM_REGION16_BE( 0x80000, "user1", 0 )
	ROM_LOAD16_BYTE( "tk00v131.u54", 0x00000, 0x40000, CRC(e09ae32b) SHA1(b090a38600d0499f7b4cb80a2715f27216d408b0) )
	ROM_LOAD16_BYTE( "tk01v131.u53", 0x00001, 0x40000, CRC(c29137ec) SHA1(4dcfba13b6f865a256bcb0406b6c83c309b17313) )

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "tksnd.u17", 0x10000, 0x18000, CRC(ab1684c3) SHA1(cc7e591fd160b259f8aecddb2c5a3c36e4e37b2f) )
	ROM_CONTINUE(          0x08000, 0x08000 )

	ROM_REGION( 0x880000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "tk0_rom.1", 0x000000, 0x200000, CRC(94cbf6f8) SHA1(dac5c4d9c8e42336c236ecc3c72b3b1f8282dc2f) )
	ROM_LOAD32_BYTE( "tk1_rom.2", 0x000001, 0x200000, CRC(c07dea98) SHA1(dd8b88beb9781579eb0a17231ad2a274b70ae1bc) )
	ROM_LOAD32_BYTE( "tk2_rom.3", 0x000002, 0x200000, CRC(183eed2a) SHA1(3905268fe45ecc47cd4d349666b4d33efda2140b) )
	ROM_LOAD32_BYTE( "tk3_rom.4", 0x000003, 0x200000, CRC(b1da1058) SHA1(a1d483701c661d69cecc9d073b23683b119f5ef1) )
	ROM_LOAD32_BYTE( "tkgrom.01", 0x800000, 0x020000, CRC(b030c3d9) SHA1(f5c21285ec8ff4f74205e0cf18da67e733e31183) )
	ROM_LOAD32_BYTE( "tkgrom.02", 0x800001, 0x020000, CRC(e98492a4) SHA1(fe8fb4bd3900109f3872f2930e8ddc9d19f599fd) )
	ROM_LOAD32_BYTE( "tkgrom.03", 0x800002, 0x020000, CRC(6088fa64) SHA1(a3eee10bdef48fefec3836f551172dbe0819acf6) )
	ROM_LOAD32_BYTE( "tkgrom.04", 0x800003, 0x020000, CRC(95be2318) SHA1(60580c87d63a114df44e2580e138128388ff447b) )

	ROM_REGION16_BE( 0x400000, "ensoniq.2", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "tksrom00.u18", 0x000000, 0x80000, CRC(79d8b83a) SHA1(78934b4d0ccca8fefcf8277e4296eb1d59cd575b) )
	ROM_LOAD16_BYTE( "tksrom01.u20", 0x100000, 0x80000, CRC(ec01648c) SHA1(b83c66cf22db5d89b9ed79b79861b79429d8380c) )
	ROM_LOAD16_BYTE( "tksrom02.u26", 0x200000, 0x80000, CRC(051ced3e) SHA1(6b63c4837e709806ffea9a37d93933635d356a6e) )
ROM_END


ROM_START( bloodstm )
	ROM_REGION16_BE( 0x80000, "user1", 0 )
	ROM_LOAD16_BYTE( "bld02.bin", 0x00000, 0x40000, CRC(95f36db6) SHA1(72ec5ca93aed8fb12d5e5b7ff3d07c5cf1dab4bb) )
	ROM_LOAD16_BYTE( "bld01.bin", 0x00001, 0x40000, CRC(fcc04b93) SHA1(7029d68f20196b6c2c30339500c7da54f2b5b054) )

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "bldsnd.u17", 0x10000, 0x18000, CRC(dddeedbb) SHA1(f8ea786836630fc44bba968845fd2cb42cd81592) )
	ROM_CONTINUE(           0x08000, 0x08000 )

	ROM_REGION( 0x880000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "bsgrom0.bin",  0x000000, 0x080000, CRC(4e10b8c1) SHA1(b80f7dbf32faa97829c735da6dc0ee424dc9ecec) )
	ROM_LOAD32_BYTE( "bsgrom5.bin",  0x000001, 0x080000, CRC(6333b6ce) SHA1(53b884e09d198f8f53449bd011ba743c28b2c934) )
	ROM_LOAD32_BYTE( "bsgrom10.bin", 0x000002, 0x080000, CRC(a972a65c) SHA1(7772c2e0aaa0b3183c6287125b95fd1cd0c3775a) )
	ROM_LOAD32_BYTE( "bsgrom15.bin", 0x000003, 0x080000, CRC(9a8f54aa) SHA1(3a2f99c28324ba1fcfb652bdc596d94c90637b72) )
	ROM_LOAD32_BYTE( "bsgrom1.bin",  0x200000, 0x080000, CRC(10abf660) SHA1(e5b28ee12972abbcd883d702f496feae62d19e07) )
	ROM_LOAD32_BYTE( "bsgrom6.bin",  0x200001, 0x080000, CRC(06a260d5) SHA1(f506b799bbb5d4465a4ab564cfd276877b8a52e9) )
	ROM_LOAD32_BYTE( "bsgrom11.bin", 0x200002, 0x080000, CRC(f2cab3c7) SHA1(eecb1f2f8a3f3b1663a51c343e28e1b2078da5f4) )
	ROM_LOAD32_BYTE( "bsgrom16.bin", 0x200003, 0x080000, CRC(403aef7b) SHA1(0a2a61480cddd020bfbad01c6c328dfa6760bddd) )
	ROM_LOAD32_BYTE( "bsgrom2.bin",  0x400000, 0x080000, CRC(488200b1) SHA1(ae25bb5e9a836ae362d88de4e392ff46ff93c636) )
	ROM_LOAD32_BYTE( "bsgrom7.bin",  0x400001, 0x080000, CRC(5bb19727) SHA1(fd8aa73ff0606ac86f054794f6abc42f99b4f856) )
	ROM_LOAD32_BYTE( "bsgrom12.bin", 0x400002, 0x080000, CRC(b10d674f) SHA1(f5c713480deac44b86173ca9ac3aeea8a4d2ac85) )
	ROM_LOAD32_BYTE( "bsgrom17.bin", 0x400003, 0x080000, CRC(7119df7e) SHA1(67acb0525f21d5cd174528fb0fa72c7101deb5eb) )
	ROM_LOAD32_BYTE( "bsgrom3.bin",  0x600000, 0x080000, CRC(2378792e) SHA1(8cf1040a86ce7b6be3f56d144d4a17b7a888a648) )
	ROM_LOAD32_BYTE( "bsgrom8.bin",  0x600001, 0x080000, CRC(3640ca2e) SHA1(1fbc8306c7310ab23d40887c16f80e2dc3d730f8) )
	ROM_LOAD32_BYTE( "bsgrom13.bin", 0x600002, 0x080000, CRC(bd4a071d) SHA1(f25483591659d7424a4c62d0093fa56e5d337bf3) )
	ROM_LOAD32_BYTE( "bsgrom18.bin", 0x600003, 0x080000, CRC(12959bb8) SHA1(f74d407004ccbf461f749672e4e57a5f0b6b549f) )
	ROM_FILL(                        0x800000, 0x080000, 0xff )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "ensoniq.2m", 0x000000, 0x200000, CRC(9fdc4825) SHA1(71e5255c66d9010be7e6f27916b605441fc53839) ) /* Ensoniq 2m 1350901601 */

	ROM_REGION16_BE( 0x400000, "ensoniq.2", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "bssrom0.bin",  0x000000, 0x80000, CRC(ee4570c8) SHA1(73dd292224bf182770b3cc2d90eb52b7d7b24378) )
	ROM_LOAD16_BYTE( "bssrom1.bin",  0x100000, 0x80000, CRC(b0f32ec5) SHA1(666f904b31cdef12cbf1dbb43a7d3ff7c2903260) )
	ROM_LOAD16_BYTE( "bssrom2.bin",  0x300000, 0x40000, CRC(8aee1e77) SHA1(f949fa89ee7d59f457ce89c72d461cecd0cface3) )
ROM_END


ROM_START( bloodstm22 )
	ROM_REGION16_BE( 0x80000, "user1", 0 )
	ROM_LOAD16_BYTE( "bld00v22.u83", 0x00000, 0x40000, CRC(904e9208) SHA1(12e96027724b905140250db969130d90b1afec83) )
	ROM_LOAD16_BYTE( "bld01v22.u88", 0x00001, 0x40000, CRC(78336a7b) SHA1(76002ce4a2d83ceae10d9c9c123013832a081150) )

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "bldsnd.u17", 0x10000, 0x18000, CRC(dddeedbb) SHA1(f8ea786836630fc44bba968845fd2cb42cd81592) )
	ROM_CONTINUE(           0x08000, 0x08000 )

	ROM_REGION( 0x880000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "bsgrom0.bin",  0x000000, 0x080000, CRC(4e10b8c1) SHA1(b80f7dbf32faa97829c735da6dc0ee424dc9ecec) )
	ROM_LOAD32_BYTE( "bsgrom5.bin",  0x000001, 0x080000, CRC(6333b6ce) SHA1(53b884e09d198f8f53449bd011ba743c28b2c934) )
	ROM_LOAD32_BYTE( "bsgrom10.bin", 0x000002, 0x080000, CRC(a972a65c) SHA1(7772c2e0aaa0b3183c6287125b95fd1cd0c3775a) )
	ROM_LOAD32_BYTE( "bsgrom15.bin", 0x000003, 0x080000, CRC(9a8f54aa) SHA1(3a2f99c28324ba1fcfb652bdc596d94c90637b72) )
	ROM_LOAD32_BYTE( "bsgrom1.bin",  0x200000, 0x080000, CRC(10abf660) SHA1(e5b28ee12972abbcd883d702f496feae62d19e07) )
	ROM_LOAD32_BYTE( "bsgrom6.bin",  0x200001, 0x080000, CRC(06a260d5) SHA1(f506b799bbb5d4465a4ab564cfd276877b8a52e9) )
	ROM_LOAD32_BYTE( "bsgrom11.bin", 0x200002, 0x080000, CRC(f2cab3c7) SHA1(eecb1f2f8a3f3b1663a51c343e28e1b2078da5f4) )
	ROM_LOAD32_BYTE( "bsgrom16.bin", 0x200003, 0x080000, CRC(403aef7b) SHA1(0a2a61480cddd020bfbad01c6c328dfa6760bddd) )
	ROM_LOAD32_BYTE( "bsgrom2.bin",  0x400000, 0x080000, CRC(488200b1) SHA1(ae25bb5e9a836ae362d88de4e392ff46ff93c636) )
	ROM_LOAD32_BYTE( "bsgrom7.bin",  0x400001, 0x080000, CRC(5bb19727) SHA1(fd8aa73ff0606ac86f054794f6abc42f99b4f856) )
	ROM_LOAD32_BYTE( "bsgrom12.bin", 0x400002, 0x080000, CRC(b10d674f) SHA1(f5c713480deac44b86173ca9ac3aeea8a4d2ac85) )
	ROM_LOAD32_BYTE( "bsgrom17.bin", 0x400003, 0x080000, CRC(7119df7e) SHA1(67acb0525f21d5cd174528fb0fa72c7101deb5eb) )
	ROM_LOAD32_BYTE( "bsgrom3.bin",  0x600000, 0x080000, CRC(2378792e) SHA1(8cf1040a86ce7b6be3f56d144d4a17b7a888a648) )
	ROM_LOAD32_BYTE( "bsgrom8.bin",  0x600001, 0x080000, CRC(3640ca2e) SHA1(1fbc8306c7310ab23d40887c16f80e2dc3d730f8) )
	ROM_LOAD32_BYTE( "bsgrom13.bin", 0x600002, 0x080000, CRC(bd4a071d) SHA1(f25483591659d7424a4c62d0093fa56e5d337bf3) )
	ROM_LOAD32_BYTE( "bsgrom18.bin", 0x600003, 0x080000, CRC(12959bb8) SHA1(f74d407004ccbf461f749672e4e57a5f0b6b549f) )
	ROM_FILL(                        0x800000, 0x080000, 0xff )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "ensoniq.2m", 0x000000, 0x200000, CRC(9fdc4825) SHA1(71e5255c66d9010be7e6f27916b605441fc53839) ) /* Ensoniq 2m 1350901601 */

	ROM_REGION16_BE( 0x400000, "ensoniq.2", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "bssrom0.bin",  0x000000, 0x80000, CRC(ee4570c8) SHA1(73dd292224bf182770b3cc2d90eb52b7d7b24378) )
	ROM_LOAD16_BYTE( "bssrom1.bin",  0x100000, 0x80000, CRC(b0f32ec5) SHA1(666f904b31cdef12cbf1dbb43a7d3ff7c2903260) )
	ROM_LOAD16_BYTE( "bssrom2.bin",  0x300000, 0x40000, CRC(8aee1e77) SHA1(f949fa89ee7d59f457ce89c72d461cecd0cface3) )
ROM_END


ROM_START( bloodstm21 )
	ROM_REGION16_BE( 0x80000, "user1", 0 )
	ROM_LOAD16_BYTE( "bld00v21.u83", 0x00000, 0x40000, CRC(71215c8e) SHA1(ee0f94c3a2619d7e3cc1ba5e1888a97b0c75a3ae) )
	ROM_LOAD16_BYTE( "bld01v21.u88", 0x00001, 0x40000, CRC(da403da6) SHA1(0f09f38ae932acb4ddbb6323bce58be7284cb24b) )

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "bldsnd.u17", 0x10000, 0x18000, CRC(dddeedbb) SHA1(f8ea786836630fc44bba968845fd2cb42cd81592) )
	ROM_CONTINUE(           0x08000, 0x08000 )

	ROM_REGION( 0x880000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "bsgrom0.bin",  0x000000, 0x080000, CRC(4e10b8c1) SHA1(b80f7dbf32faa97829c735da6dc0ee424dc9ecec) )
	ROM_LOAD32_BYTE( "bsgrom5.bin",  0x000001, 0x080000, CRC(6333b6ce) SHA1(53b884e09d198f8f53449bd011ba743c28b2c934) )
	ROM_LOAD32_BYTE( "bsgrom10.bin", 0x000002, 0x080000, CRC(a972a65c) SHA1(7772c2e0aaa0b3183c6287125b95fd1cd0c3775a) )
	ROM_LOAD32_BYTE( "bsgrom15.bin", 0x000003, 0x080000, CRC(9a8f54aa) SHA1(3a2f99c28324ba1fcfb652bdc596d94c90637b72) )
	ROM_LOAD32_BYTE( "bsgrom1.bin",  0x200000, 0x080000, CRC(10abf660) SHA1(e5b28ee12972abbcd883d702f496feae62d19e07) )
	ROM_LOAD32_BYTE( "bsgrom6.bin",  0x200001, 0x080000, CRC(06a260d5) SHA1(f506b799bbb5d4465a4ab564cfd276877b8a52e9) )
	ROM_LOAD32_BYTE( "bsgrom11.bin", 0x200002, 0x080000, CRC(f2cab3c7) SHA1(eecb1f2f8a3f3b1663a51c343e28e1b2078da5f4) )
	ROM_LOAD32_BYTE( "bsgrom16.bin", 0x200003, 0x080000, CRC(403aef7b) SHA1(0a2a61480cddd020bfbad01c6c328dfa6760bddd) )
	ROM_LOAD32_BYTE( "bsgrom2.bin",  0x400000, 0x080000, CRC(488200b1) SHA1(ae25bb5e9a836ae362d88de4e392ff46ff93c636) )
	ROM_LOAD32_BYTE( "bsgrom7.bin",  0x400001, 0x080000, CRC(5bb19727) SHA1(fd8aa73ff0606ac86f054794f6abc42f99b4f856) )
	ROM_LOAD32_BYTE( "bsgrom12.bin", 0x400002, 0x080000, CRC(b10d674f) SHA1(f5c713480deac44b86173ca9ac3aeea8a4d2ac85) )
	ROM_LOAD32_BYTE( "bsgrom17.bin", 0x400003, 0x080000, CRC(7119df7e) SHA1(67acb0525f21d5cd174528fb0fa72c7101deb5eb) )
	ROM_LOAD32_BYTE( "bsgrom3.bin",  0x600000, 0x080000, CRC(2378792e) SHA1(8cf1040a86ce7b6be3f56d144d4a17b7a888a648) )
	ROM_LOAD32_BYTE( "bsgrom8.bin",  0x600001, 0x080000, CRC(3640ca2e) SHA1(1fbc8306c7310ab23d40887c16f80e2dc3d730f8) )
	ROM_LOAD32_BYTE( "bsgrom13.bin", 0x600002, 0x080000, CRC(bd4a071d) SHA1(f25483591659d7424a4c62d0093fa56e5d337bf3) )
	ROM_LOAD32_BYTE( "bsgrom18.bin", 0x600003, 0x080000, CRC(12959bb8) SHA1(f74d407004ccbf461f749672e4e57a5f0b6b549f) )
	ROM_FILL(                        0x800000, 0x080000, 0xff )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "ensoniq.2m", 0x000000, 0x200000, CRC(9fdc4825) SHA1(71e5255c66d9010be7e6f27916b605441fc53839) ) /* Ensoniq 2m 1350901601 */

	ROM_REGION16_BE( 0x400000, "ensoniq.2", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "bssrom0.bin",  0x000000, 0x80000, CRC(ee4570c8) SHA1(73dd292224bf182770b3cc2d90eb52b7d7b24378) )
	ROM_LOAD16_BYTE( "bssrom1.bin",  0x100000, 0x80000, CRC(b0f32ec5) SHA1(666f904b31cdef12cbf1dbb43a7d3ff7c2903260) )
	ROM_LOAD16_BYTE( "bssrom2.bin",  0x300000, 0x40000, CRC(8aee1e77) SHA1(f949fa89ee7d59f457ce89c72d461cecd0cface3) )
ROM_END


ROM_START( bloodstm11 )
	ROM_REGION16_BE( 0x80000, "user1", 0 )
	ROM_LOAD16_BYTE( "bld00-11.u83", 0x00000, 0x40000, CRC(4fff8f9b) SHA1(90f450497935322b0082a70e10abf758fc441dd0) )
	ROM_LOAD16_BYTE( "bld01-11.u88", 0x00001, 0x40000, CRC(59ce23ea) SHA1(6aa02fff07f5ec6dff4f6db9ea7878a722079f81) )

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "bldsnd.u17", 0x10000, 0x18000, CRC(dddeedbb) SHA1(f8ea786836630fc44bba968845fd2cb42cd81592) )
	ROM_CONTINUE(           0x08000, 0x08000 )

	ROM_REGION( 0x880000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "bsgrom0.bin",  0x000000, 0x080000, CRC(4e10b8c1) SHA1(b80f7dbf32faa97829c735da6dc0ee424dc9ecec) )
	ROM_LOAD32_BYTE( "bsgrom5.bin",  0x000001, 0x080000, CRC(6333b6ce) SHA1(53b884e09d198f8f53449bd011ba743c28b2c934) )
	ROM_LOAD32_BYTE( "bsgrom10.bin", 0x000002, 0x080000, CRC(a972a65c) SHA1(7772c2e0aaa0b3183c6287125b95fd1cd0c3775a) )
	ROM_LOAD32_BYTE( "bsgrom15.bin", 0x000003, 0x080000, CRC(9a8f54aa) SHA1(3a2f99c28324ba1fcfb652bdc596d94c90637b72) )
	ROM_LOAD32_BYTE( "bsgrom1.bin",  0x200000, 0x080000, CRC(10abf660) SHA1(e5b28ee12972abbcd883d702f496feae62d19e07) )
	ROM_LOAD32_BYTE( "bsgrom6.bin",  0x200001, 0x080000, CRC(06a260d5) SHA1(f506b799bbb5d4465a4ab564cfd276877b8a52e9) )
	ROM_LOAD32_BYTE( "bsgrom11.bin", 0x200002, 0x080000, CRC(f2cab3c7) SHA1(eecb1f2f8a3f3b1663a51c343e28e1b2078da5f4) )
	ROM_LOAD32_BYTE( "bsgrom16.bin", 0x200003, 0x080000, CRC(403aef7b) SHA1(0a2a61480cddd020bfbad01c6c328dfa6760bddd) )
	ROM_LOAD32_BYTE( "bsgrom2.bin",  0x400000, 0x080000, CRC(488200b1) SHA1(ae25bb5e9a836ae362d88de4e392ff46ff93c636) )
	ROM_LOAD32_BYTE( "bsgrom7.bin",  0x400001, 0x080000, CRC(5bb19727) SHA1(fd8aa73ff0606ac86f054794f6abc42f99b4f856) )
	ROM_LOAD32_BYTE( "bsgrom12.bin", 0x400002, 0x080000, CRC(b10d674f) SHA1(f5c713480deac44b86173ca9ac3aeea8a4d2ac85) )
	ROM_LOAD32_BYTE( "bsgrom17.bin", 0x400003, 0x080000, CRC(7119df7e) SHA1(67acb0525f21d5cd174528fb0fa72c7101deb5eb) )
	ROM_LOAD32_BYTE( "bsgrom3.bin",  0x600000, 0x080000, CRC(2378792e) SHA1(8cf1040a86ce7b6be3f56d144d4a17b7a888a648) )
	ROM_LOAD32_BYTE( "bsgrom8.bin",  0x600001, 0x080000, CRC(3640ca2e) SHA1(1fbc8306c7310ab23d40887c16f80e2dc3d730f8) )
	ROM_LOAD32_BYTE( "bsgrom13.bin", 0x600002, 0x080000, CRC(bd4a071d) SHA1(f25483591659d7424a4c62d0093fa56e5d337bf3) )
	ROM_LOAD32_BYTE( "bsgrom18.bin", 0x600003, 0x080000, CRC(12959bb8) SHA1(f74d407004ccbf461f749672e4e57a5f0b6b549f) )
	ROM_FILL(                        0x800000, 0x080000, 0xff )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "ensoniq.2m", 0x000000, 0x200000, CRC(9fdc4825) SHA1(71e5255c66d9010be7e6f27916b605441fc53839) ) /* Ensoniq 2m 1350901601 */

	ROM_REGION16_BE( 0x400000, "ensoniq.2", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "bssrom0.bin",  0x000000, 0x80000, CRC(ee4570c8) SHA1(73dd292224bf182770b3cc2d90eb52b7d7b24378) )
	ROM_LOAD16_BYTE( "bssrom1.bin",  0x100000, 0x80000, CRC(b0f32ec5) SHA1(666f904b31cdef12cbf1dbb43a7d3ff7c2903260) )
	ROM_LOAD16_BYTE( "bssrom2.bin",  0x300000, 0x40000, CRC(8aee1e77) SHA1(f949fa89ee7d59f457ce89c72d461cecd0cface3) )
ROM_END


ROM_START( hardyard )	/* Version 1.2 (3-tier board set: P/N 1059 Rev 3,  P/N 1061 Rev 1 &  P/N 1060 Rev 0) */
	ROM_REGION16_BE( 0x80000, "user1", 0 )
	ROM_LOAD16_BYTE( "fb00v12.u83", 0x00000, 0x40000, CRC(c7497692) SHA1(6c11535cf011e15dd7ffb5eba8e8da557c38277e) )
	ROM_LOAD16_BYTE( "fb01v12.u88", 0x00001, 0x40000, CRC(3320c79a) SHA1(d1d32048c541782e60c525d9789fe12607a6df3a) )

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "fbsndv11.u17", 0x10000, 0x18000, CRC(d221b121) SHA1(06f351274a9dcb522f67f58499c9dc2ef5f06c07) )
	ROM_CONTINUE(             0x08000, 0x08000 )

	ROM_REGION( 0x880000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "itfb0.bin",  0x000000, 0x100000, CRC(0b7781af) SHA1(0e6b617a5d9a2d0d50a3839231177f2934177b87) )
	ROM_LOAD32_BYTE( "itfb1.bin",  0x000001, 0x100000, CRC(178d0f9b) SHA1(2c13be9063742c24a4b8409fe1d16f6c989f20e0) )
	ROM_LOAD32_BYTE( "itfb2.bin",  0x000002, 0x100000, CRC(0a17231e) SHA1(1499783ac32c3c6956d4084d623a432aecfd7769) )
	ROM_LOAD32_BYTE( "itfb3.bin",  0x000003, 0x100000, CRC(104456af) SHA1(6b6adca80f663cdc8fcbdf58c033d32193e91b4b) )
	ROM_LOAD32_BYTE( "itfb4.bin",  0x400000, 0x100000, CRC(2cb6f454) SHA1(e3af2809d43ddb4f17342a0b63848bf9a579b1eb) )
	ROM_LOAD32_BYTE( "itfb5.bin",  0x400001, 0x100000, CRC(9b19b873) SHA1(4393dce2fd6e1f3c2b855759a985e1e068959e0a) )
	ROM_LOAD32_BYTE( "itfb6.bin",  0x400002, 0x100000, CRC(58694394) SHA1(9b0742d136de9870f50a1f47347071a21283067b) )
	ROM_LOAD32_BYTE( "itfb7.bin",  0x400003, 0x100000, CRC(9b7a2d1a) SHA1(e4aa8d5f76b26d16cabaf88dfa1bfba8052fe99d) )
	ROM_LOAD32_BYTE( "itfb8.bin",  0x800000, 0x020000, CRC(a1656bf8) SHA1(4df05a1cdf5d636956d1c3d1f4f1988b254608d5) )
	ROM_LOAD32_BYTE( "itfb9.bin",  0x800001, 0x020000, CRC(2afa9e10) SHA1(d422447fd2fc2f9350af472eb1f1223383a1a259) )
	ROM_LOAD32_BYTE( "itfb10.bin", 0x800002, 0x020000, CRC(d5d15b38) SHA1(f414c19d8d88f916fbfa24fc3e16cea2e0acce08) )
	ROM_LOAD32_BYTE( "itfb11.bin", 0x800003, 0x020000, CRC(cd4f0df0) SHA1(632eb0cf27d7bf3df09d03f373a3195dd5a702b8) )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "ensoniq.2m", 0x000000, 0x200000, CRC(9fdc4825) SHA1(71e5255c66d9010be7e6f27916b605441fc53839) ) /* Ensoniq 2m 1350901601 */

	ROM_REGION16_BE( 0x400000, "ensoniq.2", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "fbsrom00.bin", 0x000000, 0x080000, CRC(b0a76ad2) SHA1(d1125cf96f6b9613840b8d22afa59748fb32ab90) )
	ROM_LOAD16_BYTE( "fbsrom01.bin", 0x100000, 0x080000, CRC(9fbf6a02) SHA1(90c86a94767a383895183a25b15084ed62891518) )
ROM_END


ROM_START( hardyard10 )	/* Version 1.0 (3-tier board set: P/N 1059 Rev 3, P/N 1061 Rev 1 &  P/N 1060 Rev 0) */
	ROM_REGION16_BE( 0x80000, "user1", 0 )
	ROM_LOAD16_BYTE( "fb00v10.u83", 0x00000, 0x40000, CRC(f839393c) SHA1(ba06172bc4781f7738ce43019031715fee4b344c) )
	ROM_LOAD16_BYTE( "fb01v10.u88", 0x00001, 0x40000, CRC(ca444702) SHA1(49bcc0994da9cd2c31c0cd78b822aceeaffd035f) )

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "fbsndv10.u17", 0x10000, 0x18000, CRC(6c6db5b8) SHA1(925e7c7cc7c3d290f4a334f24eef574aaac3150c) )
	ROM_CONTINUE(             0x08000, 0x08000 )

	ROM_REGION( 0x880000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "itfb0.bin",  0x000000, 0x100000, CRC(0b7781af) SHA1(0e6b617a5d9a2d0d50a3839231177f2934177b87) )
	ROM_LOAD32_BYTE( "itfb1.bin",  0x000001, 0x100000, CRC(178d0f9b) SHA1(2c13be9063742c24a4b8409fe1d16f6c989f20e0) )
	ROM_LOAD32_BYTE( "itfb2.bin",  0x000002, 0x100000, CRC(0a17231e) SHA1(1499783ac32c3c6956d4084d623a432aecfd7769) )
	ROM_LOAD32_BYTE( "itfb3.bin",  0x000003, 0x100000, CRC(104456af) SHA1(6b6adca80f663cdc8fcbdf58c033d32193e91b4b) )
	ROM_LOAD32_BYTE( "itfb4.bin",  0x400000, 0x100000, CRC(2cb6f454) SHA1(e3af2809d43ddb4f17342a0b63848bf9a579b1eb) )
	ROM_LOAD32_BYTE( "itfb5.bin",  0x400001, 0x100000, CRC(9b19b873) SHA1(4393dce2fd6e1f3c2b855759a985e1e068959e0a) )
	ROM_LOAD32_BYTE( "itfb6.bin",  0x400002, 0x100000, CRC(58694394) SHA1(9b0742d136de9870f50a1f47347071a21283067b) )
	ROM_LOAD32_BYTE( "itfb7.bin",  0x400003, 0x100000, CRC(9b7a2d1a) SHA1(e4aa8d5f76b26d16cabaf88dfa1bfba8052fe99d) )
	ROM_LOAD32_BYTE( "itfb8.bin",  0x800000, 0x020000, CRC(a1656bf8) SHA1(4df05a1cdf5d636956d1c3d1f4f1988b254608d5) )
	ROM_LOAD32_BYTE( "itfb9.bin",  0x800001, 0x020000, CRC(2afa9e10) SHA1(d422447fd2fc2f9350af472eb1f1223383a1a259) )
	ROM_LOAD32_BYTE( "itfb10.bin", 0x800002, 0x020000, CRC(d5d15b38) SHA1(f414c19d8d88f916fbfa24fc3e16cea2e0acce08) )
	ROM_LOAD32_BYTE( "itfb11.bin", 0x800003, 0x020000, CRC(cd4f0df0) SHA1(632eb0cf27d7bf3df09d03f373a3195dd5a702b8) )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "ensoniq.2m", 0x000000, 0x200000, CRC(9fdc4825) SHA1(71e5255c66d9010be7e6f27916b605441fc53839) ) /* Ensoniq 2m 1350901601 */

	ROM_REGION16_BE( 0x400000, "ensoniq.2", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "fbsrom00.bin", 0x000000, 0x080000, CRC(b0a76ad2) SHA1(d1125cf96f6b9613840b8d22afa59748fb32ab90) )
	ROM_LOAD16_BYTE( "fbsrom01.bin", 0x100000, 0x080000, CRC(9fbf6a02) SHA1(90c86a94767a383895183a25b15084ed62891518) )
ROM_END


ROM_START( pairs )	/* Version 1.2 (3-tier board set: P/N 1059 Rev 3, P/N ???? Rev ? &  P/N 1060 Rev 0) */
	ROM_REGION16_BE( 0x80000, "user1", 0 )
	ROM_LOAD16_BYTE( "pair0v12.u83", 0x00000, 0x20000, CRC(a9c761d8) SHA1(2618c9c3f336cf30f760fd88f12c09985cfd4ee7) )
	ROM_LOAD16_BYTE( "pair1v12.u88", 0x00001, 0x20000, CRC(5141eb86) SHA1(3bb10d588e6334a33e5c2c468651699e84f46cdc) )

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "snd.u17", 0x10000, 0x18000, CRC(7a514cfd) SHA1(ef5bc74c9560d2c058298051070fa748e58f07e1) )
	ROM_CONTINUE(        0x08000, 0x08000 )

	ROM_REGION( 0x880000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "grom0",  0x000000, 0x80000, CRC(baf1c2dd) SHA1(4de50001bce294ea5eea581cee9ca5a966701176) )
	ROM_LOAD32_BYTE( "grom5",  0x000001, 0x80000, CRC(30e993f3) SHA1(fe32aabacbe9d6d9419410faafe048c01988ac78) )
	ROM_LOAD32_BYTE( "grom10", 0x000002, 0x80000, CRC(3f52f50d) SHA1(abb7ec8fa1af0876dacfe04d76fbc8fc18a2b610) )
	ROM_LOAD32_BYTE( "grom15", 0x000003, 0x80000, CRC(fd38aa36) SHA1(7c65b2a42bb45b81b841792c475ea391c03a4eb2) )
	ROM_LOAD32_BYTE( "grom1",  0x200000, 0x40000, CRC(b0bd7008) SHA1(29cb334e9af73f7aef4bf55eae79d8cc05412164) )
	ROM_LOAD32_BYTE( "grom6",  0x200001, 0x40000, CRC(f7b20a47) SHA1(5a68add24b0f9cfb56b3e7aedc28382c8ead81a1) )
	ROM_LOAD32_BYTE( "grom11", 0x200002, 0x40000, CRC(1e5f2523) SHA1(c6c362bc0bb303e271176ce8c2a49990be1834cd) )
	ROM_LOAD32_BYTE( "grom16", 0x200003, 0x40000, CRC(b2975259) SHA1(aa82f8e855f2ebf1d7178a46f2b515d7c9a26299) )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "ensoniq.2m", 0x000000, 0x200000, CRC(9fdc4825) SHA1(71e5255c66d9010be7e6f27916b605441fc53839) ) /* Ensoniq 2m 1350901601 */

	ROM_REGION16_BE( 0x400000, "ensoniq.2", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "srom0.bin", 0x000000, 0x80000, CRC(19a857f9) SHA1(2515b4c127191d52d3b5a72477384847d8cabad3) )
ROM_END


ROM_START( pairsa )	/* Version ?? (3-tier board set: P/N 1059 Rev 3, P/N ???? Rev ? &  P/N 1060 Rev 0) */
	ROM_REGION16_BE( 0x80000, "user1", 0 )
	ROM_LOAD16_BYTE( "pair0.u83", 0x00000, 0x20000, CRC(774995a3) SHA1(93df91378b56802d14c105f7f48ed8a4f7bafffd) )
	ROM_LOAD16_BYTE( "pair1.u88", 0x00001, 0x20000, CRC(85d0b73a) SHA1(48a6ac6de94be13e407da13e3e2440d858714b4b) )

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "snd.u17", 0x10000, 0x18000, CRC(7a514cfd) SHA1(ef5bc74c9560d2c058298051070fa748e58f07e1) )
	ROM_CONTINUE(        0x08000, 0x08000 )

	ROM_REGION( 0x880000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "grom0",  0x000000, 0x80000, CRC(baf1c2dd) SHA1(4de50001bce294ea5eea581cee9ca5a966701176) )
	ROM_LOAD32_BYTE( "grom5",  0x000001, 0x80000, CRC(30e993f3) SHA1(fe32aabacbe9d6d9419410faafe048c01988ac78) )
	ROM_LOAD32_BYTE( "grom10", 0x000002, 0x80000, CRC(3f52f50d) SHA1(abb7ec8fa1af0876dacfe04d76fbc8fc18a2b610) )
	ROM_LOAD32_BYTE( "grom15", 0x000003, 0x80000, CRC(fd38aa36) SHA1(7c65b2a42bb45b81b841792c475ea391c03a4eb2) )
	ROM_LOAD32_BYTE( "grom1",  0x200000, 0x40000, CRC(b0bd7008) SHA1(29cb334e9af73f7aef4bf55eae79d8cc05412164) )
	ROM_LOAD32_BYTE( "grom6",  0x200001, 0x40000, CRC(f7b20a47) SHA1(5a68add24b0f9cfb56b3e7aedc28382c8ead81a1) )
	ROM_LOAD32_BYTE( "grom11", 0x200002, 0x40000, CRC(1e5f2523) SHA1(c6c362bc0bb303e271176ce8c2a49990be1834cd) )
	ROM_LOAD32_BYTE( "grom16", 0x200003, 0x40000, CRC(b2975259) SHA1(aa82f8e855f2ebf1d7178a46f2b515d7c9a26299) )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "ensoniq.2m", 0x000000, 0x200000, CRC(9fdc4825) SHA1(71e5255c66d9010be7e6f27916b605441fc53839) ) /* Ensoniq 2m 1350901601 */

	ROM_REGION16_BE( 0x400000, "ensoniq.2", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "srom0", 0x000000, 0x80000, CRC(1d96c581) SHA1(3b7c84b7db3b098ec28c7058c16f97e9cf0e4733) )
ROM_END

ROM_START( hotmemry )	/* Version 1.2 (3-tier board set: P/N 1059 Rev 3, P/N ???? Rev ? &  P/N 1060 Rev 0) */
	ROM_REGION16_BE( 0x80000, "user1", 0 )
	ROM_LOAD16_BYTE( "hotmem0.u83", 0x00000, 0x40000, CRC(5b9d87a2) SHA1(5a1ca7b622832fcb641e081d0c2a49c38ca795cd) )
	ROM_LOAD16_BYTE( "hotmem1.u88", 0x00001, 0x40000, CRC(aeea087c) SHA1(3a8bdc04bc4051691823d0c5a1a3429475692100) )

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "hotmem_snd.u17", 0x10000, 0x18000, CRC(805941c7) SHA1(4a6832d93ff2b986cb54052658af62584782cb59) )
	ROM_CONTINUE(               0x08000, 0x08000 )

	ROM_REGION( 0x880000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "hotmem.grom0",  0x000000, 0x80000, CRC(68f279ef) SHA1(66098e68474e692676662b03835d1b74f581b0f4) )
	ROM_LOAD32_BYTE( "hotmem.grom5",  0x000001, 0x80000, CRC(295bb43d) SHA1(ccecdbc9dc9ef925fe59a53eeff89135d2ae748d) )
	ROM_LOAD32_BYTE( "hotmem.grom10", 0x000002, 0x80000, CRC(f8cc939b) SHA1(cbd35346f057f1e615705acb2595ba550f6d8772) )
	ROM_LOAD32_BYTE( "hotmem.grom15", 0x000003, 0x80000, CRC(a03d9bcd) SHA1(e75be35fd5cfec1e7ab3e0db468bff4d76a9cb27) )
	ROM_LOAD32_BYTE( "hotmem.grom1",  0x200000, 0x40000, CRC(b446105e) SHA1(25b5067c09490086095fcf10085f0eeffd53b27f) )
	ROM_LOAD32_BYTE( "hotmem.grom6",  0x200001, 0x40000, CRC(3a7ba9eb) SHA1(dd7548afb8ee92af369732f4159b0ad0d1b58259) )
	ROM_LOAD32_BYTE( "hotmem.grom11", 0x200002, 0x40000, CRC(9ec4ea41) SHA1(5bc4489b881c9736cb39891b6ab5e75f5c45907c) )
	ROM_LOAD32_BYTE( "hotmem.grom16", 0x200003, 0x40000, CRC(4507a895) SHA1(3d6cd6cd81b62545f7be5991f67803cf11c96ee6) )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "ensoniq.2m", 0x000000, 0x200000, CRC(9fdc4825) SHA1(71e5255c66d9010be7e6f27916b605441fc53839) ) /* Ensoniq 2m 1350901601 */

	ROM_REGION16_BE( 0x400000, "ensoniq.2", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "hotmem.srom0", 0x000000, 0x80000, CRC(c1103224) SHA1(52cf341ff9092ecb8cb94f66a96ee0c726bf1412) )
ROM_END

ROM_START( wcbowldx )	/* Deluxe version 2.00 (PCB P/N 1083 Rev 2), This version is derived from the Tournament version, but tournament features have be removed/disabled */
	ROM_REGION32_BE( CODE_SIZE, "user1", 0 )
	ROM_LOAD32_BYTE( "wcbdprm0.200", 0x00000, 0x20000, CRC(280df7f0) SHA1(dacffe8fc21263093b0f4a4fbf444abd49afbff1) )
	ROM_LOAD32_BYTE( "wcbdprm1.200", 0x00001, 0x20000, CRC(526eded0) SHA1(106d5503ed4db2411e1f3446d613eac525a8a9cc) )
	ROM_LOAD32_BYTE( "wcbdprm2.200", 0x00002, 0x20000, CRC(cb263173) SHA1(66acafaa9aba375124921774efc152e2a298a464) )
	ROM_LOAD32_BYTE( "wcbdprm3.200", 0x00003, 0x20000, CRC(43ecad0b) SHA1(890a843c162c052a790e432db10f968875be835c) )

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "wcb_snd.u88", 0x10000, 0x18000, CRC(e97a6d28) SHA1(96d7b7856918abcc460083f2a46582ba2a689288) ) /* Actual version is 4.01 but may be labeled V4.0 or V4.01 */
	ROM_CONTINUE(            0x08000, 0x08000 )

	ROM_REGION( 0x880000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "wcb_grom.0_0", 0x000000, 0x080000, CRC(6fcb4246) SHA1(91fb5d18ea9494b08251d1e611c80414df3aad66) )
	ROM_LOAD32_BYTE( "wcb_grom.0_1", 0x000001, 0x080000, CRC(2ae31f45) SHA1(85218aa9a7ca7c6870427ffbd08b78255813ff90) )
	ROM_LOAD32_BYTE( "wcb_grom.0_2", 0x000002, 0x080000, CRC(bccc0f35) SHA1(2389662e881e86f8cdb36eb2a082923d976676c8) )
	ROM_LOAD32_BYTE( "wcb_grom.0_3", 0x000003, 0x080000, CRC(ab1da462) SHA1(3f3a4a083483d2d95d5ef540eea077cad799fcb7) )
	ROM_LOAD32_BYTE( "wcb_grom.1_0", 0x200000, 0x080000, CRC(bdfafd1f) SHA1(bc0e6fe83d3f8e88c2e55ba3a436875d5470de5b) )
	ROM_LOAD32_BYTE( "wcb_grom.1_1", 0x200001, 0x080000, CRC(7d6baa2e) SHA1(c47854b064aa96d2581c23afe13cd05a36f9dae3) )
	ROM_LOAD32_BYTE( "wcb_grom.1_2", 0x200002, 0x080000, CRC(7513d3de) SHA1(b17650ed5210860c3bde53647a30f8fce67aaa38) )
	ROM_LOAD32_BYTE( "wcb_grom.1_3", 0x200003, 0x080000, CRC(e46877e6) SHA1(f50c904ec5b2b8cbc92f2b28641433c91ee17af5) )
	ROM_LOAD32_BYTE( "wcbdgrom.2_0", 0x400000, 0x040000, CRC(3b4c5a5c) SHA1(152cbc5d1aa84012f49fa13d91fd89a10f874508) )
	ROM_LOAD32_BYTE( "wcbdgrom.2_1", 0x400001, 0x040000, CRC(ed0b9b26) SHA1(5173e3936cae419255b3034e2b8a69f6bd136b93) )
	ROM_LOAD32_BYTE( "wcbdgrom.2_2", 0x400002, 0x040000, CRC(4b9e345e) SHA1(6842a8a6690db8478bdbb8e8119d3eef5e77439b) )
	ROM_LOAD32_BYTE( "wcbdgrom.2_3", 0x400003, 0x040000, CRC(4e13ee7a) SHA1(245ce87cd6839119a9a976b9a90cd1028aa3e291) )
	ROM_FILL(                      0x500000, 0x380000, 0xff )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "wcb_srom.0",  0x000000, 0x080000, CRC(d42dd283) SHA1(8ef75c398d1c51d2d7d299ac309a2352179864d9) ) /* Newer sound sample roms */
	ROM_LOAD16_BYTE( "wcb_srom.1",  0x200000, 0x080000, CRC(7a69ab54) SHA1(d1f9194446e235af69c6ff28af0dccc44ab9b5d3) ) /* Newer sound sample roms */

	ROM_REGION16_BE( 0x400000, "ensoniq.1", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "wcb_srom.2",  0x000000, 0x080000, CRC(346530a2) SHA1(9951971ad6d368fb718027e18331d12f0a72970c) ) /* Newer sound sample roms */
	ROM_LOAD16_BYTE( "wcb_srom.3",  0x200000, 0x040000, CRC(1dfe3a31) SHA1(94947f495692288fbf14fc7796a84c5548a2e8a8) ) /* Newer sound sample roms, rom is a 27C020 in this set */
ROM_END


ROM_START( wcbowl140 )	/* Version 1.40 Tournament (PCB P/N 1083 Rev 2) */
	ROM_REGION32_BE( CODE_SIZE, "user1", 0 )
	ROM_LOAD32_BYTE( "wcbfprm0.140", 0x00000, 0x20000, CRC(9d31ceb1) SHA1(d147976a763ba1e18d861351b12c5d275b94a562) ) /* Labeled "WCBF PROM0 1.40" ect */
	ROM_LOAD32_BYTE( "wcbfprm1.140", 0x00001, 0x20000, CRC(c6669452) SHA1(ba58da7bee5120682e2306454da287c969014899) )
	ROM_LOAD32_BYTE( "wcbfprm2.140", 0x00002, 0x20000, CRC(d2fc4d09) SHA1(17983759ad6137a2e67b8414ea58880865311534) )
	ROM_LOAD32_BYTE( "wcbfprm3.140", 0x00003, 0x20000, CRC(c41258a4) SHA1(182e8a25bdb126a4de8a44a1c26fd8b66f06d66e) )

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "wcb_snd.u88", 0x10000, 0x18000, CRC(e97a6d28) SHA1(96d7b7856918abcc460083f2a46582ba2a689288) ) /* Actual version is 4.01 but may be labeled V4.0 or V4.01 */
	ROM_CONTINUE(            0x08000, 0x08000 )

	ROM_REGION( 0x880000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "wcb_grom.0_0", 0x000000, 0x080000, CRC(6fcb4246) SHA1(91fb5d18ea9494b08251d1e611c80414df3aad66) )
	ROM_LOAD32_BYTE( "wcb_grom.0_1", 0x000001, 0x080000, CRC(2ae31f45) SHA1(85218aa9a7ca7c6870427ffbd08b78255813ff90) )
	ROM_LOAD32_BYTE( "wcb_grom.0_2", 0x000002, 0x080000, CRC(bccc0f35) SHA1(2389662e881e86f8cdb36eb2a082923d976676c8) )
	ROM_LOAD32_BYTE( "wcb_grom.0_3", 0x000003, 0x080000, CRC(ab1da462) SHA1(3f3a4a083483d2d95d5ef540eea077cad799fcb7) )
	ROM_LOAD32_BYTE( "wcb_grom.1_0", 0x200000, 0x080000, CRC(bdfafd1f) SHA1(bc0e6fe83d3f8e88c2e55ba3a436875d5470de5b) )
	ROM_LOAD32_BYTE( "wcb_grom.1_1", 0x200001, 0x080000, CRC(7d6baa2e) SHA1(c47854b064aa96d2581c23afe13cd05a36f9dae3) )
	ROM_LOAD32_BYTE( "wcb_grom.1_2", 0x200002, 0x080000, CRC(7513d3de) SHA1(b17650ed5210860c3bde53647a30f8fce67aaa38) )
	ROM_LOAD32_BYTE( "wcb_grom.1_3", 0x200003, 0x080000, CRC(e46877e6) SHA1(f50c904ec5b2b8cbc92f2b28641433c91ee17af5) )
	ROM_LOAD32_BYTE( "wcbdgrom.2_0", 0x400000, 0x040000, CRC(3b4c5a5c) SHA1(152cbc5d1aa84012f49fa13d91fd89a10f874508) )
	ROM_LOAD32_BYTE( "wcbdgrom.2_1", 0x400001, 0x040000, CRC(ed0b9b26) SHA1(5173e3936cae419255b3034e2b8a69f6bd136b93) )
	ROM_LOAD32_BYTE( "wcbdgrom.2_2", 0x400002, 0x040000, CRC(4b9e345e) SHA1(6842a8a6690db8478bdbb8e8119d3eef5e77439b) )
	ROM_LOAD32_BYTE( "wcbdgrom.2_3", 0x400003, 0x040000, CRC(4e13ee7a) SHA1(245ce87cd6839119a9a976b9a90cd1028aa3e291) )
	ROM_FILL(                        0x500000, 0x380000, 0xff )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "wcb_srom.0",  0x000000, 0x080000, CRC(d42dd283) SHA1(8ef75c398d1c51d2d7d299ac309a2352179864d9) ) /* Newer sound sample roms */
	ROM_LOAD16_BYTE( "wcb_srom.1",  0x200000, 0x080000, CRC(7a69ab54) SHA1(d1f9194446e235af69c6ff28af0dccc44ab9b5d3) ) /* Newer sound sample roms */

	ROM_REGION16_BE( 0x400000, "ensoniq.1", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "wcb_srom.2",  0x000000, 0x080000, CRC(346530a2) SHA1(9951971ad6d368fb718027e18331d12f0a72970c) ) /* Newer sound sample roms */
	ROM_LOAD16_BYTE( "wcb_srom.3",  0x200000, 0x040000, CRC(1dfe3a31) SHA1(94947f495692288fbf14fc7796a84c5548a2e8a8) ) /* Newer sound sample roms, rom is a 27C020 in this set */
ROM_END


ROM_START( wcbowl )	/* Version 1.66 (PCB P/N 1083 Rev 2) */
	ROM_REGION32_BE( CODE_SIZE, "user1", 0 )
	ROM_LOAD32_BYTE( "wcb_prm0.166", 0x00000, 0x20000, CRC(f6774112) SHA1(cb09bb3e40490b3cdc3a5f7d18168384b5b29d85) )
	ROM_LOAD32_BYTE( "wcb_prm1.166", 0x00001, 0x20000, CRC(931821ae) SHA1(328cd78ba70fe3cb0bdbc53833fe6fb153aceaea) )
	ROM_LOAD32_BYTE( "wcb_prm2.166", 0x00002, 0x20000, CRC(c54f5e40) SHA1(2cd92ba1db74b24e908d10f733757801db180dd0) )
	ROM_LOAD32_BYTE( "wcb_prm3.166", 0x00003, 0x20000, CRC(dd72c796) SHA1(4c1542c51848a88a663e56ae0b47bf9d2d9f7d54) )

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "wcb_snd.u88", 0x10000, 0x18000, CRC(e97a6d28) SHA1(96d7b7856918abcc460083f2a46582ba2a689288) ) /* Actual version is 4.01 but may be labeled V4.0 or V4.01 */
	ROM_CONTINUE(            0x08000, 0x08000 )

	ROM_REGION( 0x880000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "wcb_grom.0_0", 0x000000, 0x080000, CRC(6fcb4246) SHA1(91fb5d18ea9494b08251d1e611c80414df3aad66) )
	ROM_LOAD32_BYTE( "wcb_grom.0_1", 0x000001, 0x080000, CRC(2ae31f45) SHA1(85218aa9a7ca7c6870427ffbd08b78255813ff90) )
	ROM_LOAD32_BYTE( "wcb_grom.0_2", 0x000002, 0x080000, CRC(bccc0f35) SHA1(2389662e881e86f8cdb36eb2a082923d976676c8) )
	ROM_LOAD32_BYTE( "wcb_grom.0_3", 0x000003, 0x080000, CRC(ab1da462) SHA1(3f3a4a083483d2d95d5ef540eea077cad799fcb7) )
	ROM_LOAD32_BYTE( "wcb_grom.1_0", 0x200000, 0x080000, CRC(bdfafd1f) SHA1(bc0e6fe83d3f8e88c2e55ba3a436875d5470de5b) )
	ROM_LOAD32_BYTE( "wcb_grom.1_1", 0x200001, 0x080000, CRC(7d6baa2e) SHA1(c47854b064aa96d2581c23afe13cd05a36f9dae3) )
	ROM_LOAD32_BYTE( "wcb_grom.1_2", 0x200002, 0x080000, CRC(7513d3de) SHA1(b17650ed5210860c3bde53647a30f8fce67aaa38) )
	ROM_LOAD32_BYTE( "wcb_grom.1_3", 0x200003, 0x080000, CRC(e46877e6) SHA1(f50c904ec5b2b8cbc92f2b28641433c91ee17af5) )
	ROM_FILL(                        0x400000, 0x480000, 0xff )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "wcb_srom.0",  0x000000, 0x080000, CRC(d42dd283) SHA1(8ef75c398d1c51d2d7d299ac309a2352179864d9) ) /* Newer sound sample roms */
	ROM_LOAD16_BYTE( "wcb_srom.1",  0x200000, 0x080000, CRC(7a69ab54) SHA1(d1f9194446e235af69c6ff28af0dccc44ab9b5d3) ) /* Newer sound sample roms */

	ROM_REGION16_BE( 0x400000, "ensoniq.1", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "wcb_srom.2",  0x000000, 0x080000, CRC(346530a2) SHA1(9951971ad6d368fb718027e18331d12f0a72970c) ) /* Newer sound sample roms */
	ROM_LOAD16_BYTE( "wcb_srom.3",  0x200000, 0x040000, CRC(1dfe3a31) SHA1(94947f495692288fbf14fc7796a84c5548a2e8a8) ) /* Newer sound sample roms, rom is a 27C020 in this set */
ROM_END


ROM_START( wcbowl165 )	/* Version 1.65 (PCB P/N 1083 Rev 2) */
	ROM_REGION32_BE( CODE_SIZE, "user1", 0 )
	ROM_LOAD32_BYTE( "wcb_prm0.165", 0x00000, 0x20000, CRC(cf0f6c25) SHA1(90685288994dce73d5b1070a55fca3f1713c5bb6) )
	ROM_LOAD32_BYTE( "wcb_prm1.165", 0x00001, 0x20000, CRC(076ab158) SHA1(e6d8a6726e27ba6916d4711dff88f26f1dc162e1) )
	ROM_LOAD32_BYTE( "wcb_prm2.165", 0x00002, 0x20000, CRC(47259009) SHA1(78a6e70e747030a5ed43d49384061e53f4a77675) )
	ROM_LOAD32_BYTE( "wcb_prm3.165", 0x00003, 0x20000, CRC(4c6b4e4f) SHA1(77f5f4b632dd1919ae210bbdc75042bdbebf6660) )

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "wcb_snd.u88", 0x10000, 0x18000, CRC(e97a6d28) SHA1(96d7b7856918abcc460083f2a46582ba2a689288) ) /* Actual version is 4.01 but may be labeled V4.0 or V4.01 */
	ROM_CONTINUE(            0x08000, 0x08000 )

	ROM_REGION( 0x880000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "wcb_grom.0_0", 0x000000, 0x080000, CRC(6fcb4246) SHA1(91fb5d18ea9494b08251d1e611c80414df3aad66) )
	ROM_LOAD32_BYTE( "wcb_grom.0_1", 0x000001, 0x080000, CRC(2ae31f45) SHA1(85218aa9a7ca7c6870427ffbd08b78255813ff90) )
	ROM_LOAD32_BYTE( "wcb_grom.0_2", 0x000002, 0x080000, CRC(bccc0f35) SHA1(2389662e881e86f8cdb36eb2a082923d976676c8) )
	ROM_LOAD32_BYTE( "wcb_grom.0_3", 0x000003, 0x080000, CRC(ab1da462) SHA1(3f3a4a083483d2d95d5ef540eea077cad799fcb7) )
	ROM_LOAD32_BYTE( "wcb_grom.1_0", 0x200000, 0x080000, CRC(bdfafd1f) SHA1(bc0e6fe83d3f8e88c2e55ba3a436875d5470de5b) )
	ROM_LOAD32_BYTE( "wcb_grom.1_1", 0x200001, 0x080000, CRC(7d6baa2e) SHA1(c47854b064aa96d2581c23afe13cd05a36f9dae3) )
	ROM_LOAD32_BYTE( "wcb_grom.1_2", 0x200002, 0x080000, CRC(7513d3de) SHA1(b17650ed5210860c3bde53647a30f8fce67aaa38) )
	ROM_LOAD32_BYTE( "wcb_grom.1_3", 0x200003, 0x080000, CRC(e46877e6) SHA1(f50c904ec5b2b8cbc92f2b28641433c91ee17af5) )
	ROM_FILL(                        0x400000, 0x480000, 0xff )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "wcb_srom.0",  0x000000, 0x080000, CRC(d42dd283) SHA1(8ef75c398d1c51d2d7d299ac309a2352179864d9) ) /* Newer sound sample roms */
	ROM_LOAD16_BYTE( "wcb_srom.1",  0x200000, 0x080000, CRC(7a69ab54) SHA1(d1f9194446e235af69c6ff28af0dccc44ab9b5d3) ) /* Newer sound sample roms */

	ROM_REGION16_BE( 0x400000, "ensoniq.1", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "wcb_srom.2",  0x000000, 0x080000, CRC(346530a2) SHA1(9951971ad6d368fb718027e18331d12f0a72970c) ) /* Newer sound sample roms */
	ROM_LOAD16_BYTE( "wcb_srom.3",  0x200000, 0x040000, CRC(1dfe3a31) SHA1(94947f495692288fbf14fc7796a84c5548a2e8a8) ) /* Newer sound sample roms, rom is a 27C020 in this set */
ROM_END


ROM_START( wcbowl161 )	/* Version 1.61 (PCB P/N 1083 Rev 2) */
	ROM_REGION32_BE( CODE_SIZE, "user1", 0 )
	ROM_LOAD32_BYTE( "wcb_prm0.161", 0x00000, 0x20000, CRC(b879d4a7) SHA1(8b5af3f4d3522bdb8e1d6092b2e311fbfaec2bd0) )
	ROM_LOAD32_BYTE( "wcb_prm1.161", 0x00001, 0x20000, CRC(49f3ed6a) SHA1(6c6857bd3fbfe0cfeaf0e512bbbd795376a21472) )
	ROM_LOAD32_BYTE( "wcb_prm2.161", 0x00002, 0x20000, CRC(47259009) SHA1(78a6e70e747030a5ed43d49384061e53f4a77675) )
	ROM_LOAD32_BYTE( "wcb_prm3.161", 0x00003, 0x20000, CRC(e5081f85) SHA1(a5513b8dd917a35f1c8b7f833c2d5622353d39f0) )

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "wcb_sndv4.u88", 0x10000, 0x18000, CRC(194a51d7) SHA1(c67b042008ff2a2713562d3789e5bc3a312fae17) ) /* Version 4.0 */
	ROM_CONTINUE(             0x08000, 0x08000 )

	ROM_REGION( 0x880000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "wcb_grom.0_0", 0x000000, 0x080000, CRC(6fcb4246) SHA1(91fb5d18ea9494b08251d1e611c80414df3aad66) )
	ROM_LOAD32_BYTE( "wcb_grom.0_1", 0x000001, 0x080000, CRC(2ae31f45) SHA1(85218aa9a7ca7c6870427ffbd08b78255813ff90) )
	ROM_LOAD32_BYTE( "wcb_grom.0_2", 0x000002, 0x080000, CRC(bccc0f35) SHA1(2389662e881e86f8cdb36eb2a082923d976676c8) )
	ROM_LOAD32_BYTE( "wcb_grom.0_3", 0x000003, 0x080000, CRC(ab1da462) SHA1(3f3a4a083483d2d95d5ef540eea077cad799fcb7) )
	ROM_LOAD32_BYTE( "wcb_grom.1_0", 0x200000, 0x080000, CRC(bdfafd1f) SHA1(bc0e6fe83d3f8e88c2e55ba3a436875d5470de5b) )
	ROM_LOAD32_BYTE( "wcb_grom.1_1", 0x200001, 0x080000, CRC(7d6baa2e) SHA1(c47854b064aa96d2581c23afe13cd05a36f9dae3) )
	ROM_LOAD32_BYTE( "wcb_grom.1_2", 0x200002, 0x080000, CRC(7513d3de) SHA1(b17650ed5210860c3bde53647a30f8fce67aaa38) )
	ROM_LOAD32_BYTE( "wcb_grom.1_3", 0x200003, 0x080000, CRC(e46877e6) SHA1(f50c904ec5b2b8cbc92f2b28641433c91ee17af5) )
	ROM_FILL(                        0x400000, 0x480000, 0xff )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "wcbosrom.0",  0x000000, 0x080000, CRC(c3821cb5) SHA1(3c2c27d1e577201cbd0d28cc48fc80ae7747faa1) ) /* Older sound sample roms */
	ROM_LOAD16_BYTE( "wcbosrom.1",  0x200000, 0x080000, CRC(37bfa3c7) SHA1(98c98296acaa55e6fd12a62305ff387c863fc5ed) ) /* Older sound sample roms */

	ROM_REGION16_BE( 0x400000, "ensoniq.1", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "wcbosrom.2",  0x000000, 0x080000, CRC(f82c08fd) SHA1(8f6f47f5a4b68a31df4c2eb330dc95a9963e55c1) ) /* Older sound sample roms */
	ROM_LOAD16_BYTE( "wcbosrom.3",  0x200000, 0x020000, CRC(1c2efdee) SHA1(d306c9e7f9c4c2662561401170439a10a9ee89ed) ) /* Older sound sample roms, rom is a 27C010 in this set */
ROM_END


ROM_START( wcbowl16 )	/* Version 1.6 (PCB P/N 1083 Rev 2), This is the first set to move to the single board platform */
	ROM_REGION32_BE( CODE_SIZE, "user1", 0 )
	ROM_LOAD32_BYTE( "wcb_prm0.16n", 0x00000, 0x20000, CRC(332c558f) SHA1(179b763e2189c11cf6751eb6c419fe4417b288a8) ) /* Program roms labeled as "WCB PROM0 V1.6N" ect */
	ROM_LOAD32_BYTE( "wcb_prm1.16n", 0x00001, 0x20000, CRC(c5750857) SHA1(8121b56d304ab405b06d4aa4c7d2db2f912f0bf2) )
	ROM_LOAD32_BYTE( "wcb_prm2.16n", 0x00002, 0x20000, CRC(28f4ee8a) SHA1(a6a2b9cca622df0b9d181c35f4d01e8ab00392a0) )
	ROM_LOAD32_BYTE( "wcb_prm3.16n", 0x00003, 0x20000, CRC(f0a58979) SHA1(c837b2a5b0e5ae993f1e453b59e3fbd9ed5de2a7) )

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "wcb_sndv3.u88", 0x10000, 0x18000, CRC(45c4f659) SHA1(cfd140b9947654f677409a0fb4fa0c7b65992f95) ) /* Version 3.0N */
	ROM_CONTINUE(              0x08000, 0x08000 )

	ROM_REGION( 0x880000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "wcb_grom.0_0", 0x000000, 0x080000, CRC(6fcb4246) SHA1(91fb5d18ea9494b08251d1e611c80414df3aad66) )
	ROM_LOAD32_BYTE( "wcb_grom.0_1", 0x000001, 0x080000, CRC(2ae31f45) SHA1(85218aa9a7ca7c6870427ffbd08b78255813ff90) )
	ROM_LOAD32_BYTE( "wcb_grom.0_2", 0x000002, 0x080000, CRC(bccc0f35) SHA1(2389662e881e86f8cdb36eb2a082923d976676c8) )
	ROM_LOAD32_BYTE( "wcb_grom.0_3", 0x000003, 0x080000, CRC(ab1da462) SHA1(3f3a4a083483d2d95d5ef540eea077cad799fcb7) )
	ROM_LOAD32_BYTE( "wcb_grom.1_0", 0x200000, 0x080000, CRC(bdfafd1f) SHA1(bc0e6fe83d3f8e88c2e55ba3a436875d5470de5b) )
	ROM_LOAD32_BYTE( "wcb_grom.1_1", 0x200001, 0x080000, CRC(7d6baa2e) SHA1(c47854b064aa96d2581c23afe13cd05a36f9dae3) )
	ROM_LOAD32_BYTE( "wcb_grom.1_2", 0x200002, 0x080000, CRC(7513d3de) SHA1(b17650ed5210860c3bde53647a30f8fce67aaa38) )
	ROM_LOAD32_BYTE( "wcb_grom.1_3", 0x200003, 0x080000, CRC(e46877e6) SHA1(f50c904ec5b2b8cbc92f2b28641433c91ee17af5) )
	ROM_FILL(                        0x400000, 0x480000, 0xff )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "wcbosrom.0",  0x000000, 0x080000, CRC(c3821cb5) SHA1(3c2c27d1e577201cbd0d28cc48fc80ae7747faa1) ) /* Older sound sample roms */
	ROM_LOAD16_BYTE( "wcbosrom.1",  0x200000, 0x080000, CRC(37bfa3c7) SHA1(98c98296acaa55e6fd12a62305ff387c863fc5ed) ) /* Older sound sample roms */

	ROM_REGION16_BE( 0x400000, "ensoniq.1", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "wcbosrom.2",  0x000000, 0x080000, CRC(f82c08fd) SHA1(8f6f47f5a4b68a31df4c2eb330dc95a9963e55c1) ) /* Older sound sample roms */
	ROM_LOAD16_BYTE( "wcbosrom.3",  0x200000, 0x020000, CRC(1c2efdee) SHA1(d306c9e7f9c4c2662561401170439a10a9ee89ed) ) /* Older sound sample roms, rom is a 27C010 in this set */
ROM_END


ROM_START( wcbowl15 )	/* Version 1.5 (3-tier board set: P/N 1059 Rev 3, P/N 1079 Rev 1 & P/N 1060 Rev 0) */
	/* v1.0 for this platform has been confirmed, but not dumped */
	ROM_REGION16_BE( 0x80000, "user1", 0 )
	ROM_LOAD16_BYTE( "wcb_v15.u83", 0x00000, 0x20000, CRC(3ca9ab85) SHA1(364946dceb3f7279b7d67d9d685a98ba7f4901aa) ) /* Labeled as "WCB V1.5 (U83)" */
	ROM_LOAD16_BYTE( "wcb_v15.u88", 0x00001, 0x20000, CRC(d43e6fad) SHA1(fd72f6945e7f5ef86dc28503749d18086dd29906) ) /* Labeled as "WCB V1.5 (U88)" */

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "wcb_snd2.u17", 0x10000, 0x18000, CRC(c14907ba) SHA1(e52fb87c1f9b5847efc0ef15eb7e6c04dcf38110) ) /* Labeled as "WCB SND V2.0" */
	ROM_CONTINUE(             0x08000, 0x08000 )

	ROM_REGION( 0x880000, "gfx1", 0 )
	/* No known set specificly checks for this, however the GROM data may be in the form of four 8 Meg roms:
    ROM_LOAD32_BYTE( "wcb8grom.0_0", 0x000000, 0x100000, CRC(40837737) SHA1(f073943ec6f84285a8559553fb292ec1f8a629d0) ) Labeled as "WCB GROM0_0 *" ect
    ROM_LOAD32_BYTE( "wcb8grom.0_1", 0x000001, 0x100000, CRC(1615aee8) SHA1(6184919371a894b1d6f2e06a2b328cb55abed4a9) )
    ROM_LOAD32_BYTE( "wcb8grom.0_2", 0x000002, 0x100000, CRC(d8e0b06e) SHA1(4981c0cf16df68a1b4da7ebf65ca587c21292478) )
    ROM_LOAD32_BYTE( "wcb8grom.0_3", 0x000003, 0x100000, CRC(0348a7f0) SHA1(462f77514c0e9a28da63732a4f31e9483d4c483e) )
    */
	ROM_LOAD32_BYTE( "wcb_grm0.0", 0x000000, 0x080000, CRC(5d79aaae) SHA1(e1bf5c46843f69b8bac41dde73d89ba59b4c8b7f) )
	ROM_LOAD32_BYTE( "wcb_grm0.1", 0x000001, 0x080000, CRC(e26dcedb) SHA1(15441b97dd3d50d28007062fe28841fa3f762ec9) )
	ROM_LOAD32_BYTE( "wcb_grm0.2", 0x000002, 0x080000, CRC(32735875) SHA1(4017a8577d8efa8c5b95bd30723ebbf6ecaeba2b) )
	ROM_LOAD32_BYTE( "wcb_grm0.3", 0x000003, 0x080000, CRC(019d0ab8) SHA1(3281eada296ad746da80ef6e5909c50b03b90d08) )
	ROM_LOAD32_BYTE( "wcb_grm1.0", 0x200000, 0x080000, CRC(8bd31762) SHA1(a7274c8173b4fb04a6aed0b6a622b52a811a8c83) )
	ROM_LOAD32_BYTE( "wcb_grm1.1", 0x200001, 0x080000, CRC(b3f761fc) SHA1(5880ca1423cea9a7ca3d0875c8db33787f4056d4) )
	ROM_LOAD32_BYTE( "wcb_grm1.2", 0x200002, 0x080000, CRC(c22f44ad) SHA1(b25b11346ee1812b2be79105faf64faa0302c105) )
	ROM_LOAD32_BYTE( "wcb_grm1.3", 0x200003, 0x080000, CRC(036084c4) SHA1(6d2e402d2f4565e037a2676ba676e4b1da2b5dfe) )
	ROM_FILL(                      0x400000, 0x480000, 0xff )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "ensoniq.2mx", 0x000000, 0x200000, CRC(0814ab80) SHA1(e92525f7cf58cf480510c278fea705f67225e58f) ) /* Ensoniq 2MX16U 1350901801 */

	ROM_REGION16_BE( 0x400000, "ensoniq.2", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "wcb_srm.0",  0x000000, 0x080000, CRC(115bcd1f) SHA1(c321bf3145c11de1351c8f9cd554ab3d6600e854) )
	ROM_LOAD16_BYTE( "wcb_srm.1",  0x100000, 0x080000, CRC(87a4a4d8) SHA1(60db2f466686481857eb39b90ac7a19d0a96adac) )
ROM_END


ROM_START( wcbowl13 )	/* Version 1.3 (3-tier board set: P/N 1059 Rev 3, P/N 1079 Rev 1 & P/N 1060 Rev 0) */
	ROM_REGION16_BE( 0x80000, "user1", 0 )
	ROM_LOAD16_BYTE( "wcb_v13.u83", 0x00000, 0x20000, CRC(2b6d284e) SHA1(339951661509d07b69c670b7249f30a616872bdf) ) /* Labeled as "WCB V1.3 (U83)" */
	ROM_LOAD16_BYTE( "wcb_v13.u88", 0x00001, 0x20000, CRC(039af877) SHA1(2ac9a57e358ab1ccf9a4d18f7992b59f172e31cf) ) /* Labeled as "WCB V1.3 (U88)" */

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "wcb_snd2.u17", 0x10000, 0x18000, CRC(c14907ba) SHA1(e52fb87c1f9b5847efc0ef15eb7e6c04dcf38110) ) /* Labeled as "WCB SND V2.0" */
	ROM_CONTINUE(             0x08000, 0x08000 )

	ROM_REGION( 0x880000, "gfx1", 0 )
	/* No known set specificly checks for this, however the GROM data may be in the form of four 8 Meg roms:
    ROM_LOAD32_BYTE( "wcb8grom.0_0", 0x000000, 0x100000, CRC(40837737) SHA1(f073943ec6f84285a8559553fb292ec1f8a629d0) ) Labeled as "WCB GROM0_0 *" ect
    ROM_LOAD32_BYTE( "wcb8grom.0_1", 0x000001, 0x100000, CRC(1615aee8) SHA1(6184919371a894b1d6f2e06a2b328cb55abed4a9) )
    ROM_LOAD32_BYTE( "wcb8grom.0_2", 0x000002, 0x100000, CRC(d8e0b06e) SHA1(4981c0cf16df68a1b4da7ebf65ca587c21292478) )
    ROM_LOAD32_BYTE( "wcb8grom.0_3", 0x000003, 0x100000, CRC(0348a7f0) SHA1(462f77514c0e9a28da63732a4f31e9483d4c483e) )
    */
	ROM_LOAD32_BYTE( "wcb_grm0.0", 0x000000, 0x080000, CRC(5d79aaae) SHA1(e1bf5c46843f69b8bac41dde73d89ba59b4c8b7f) )
	ROM_LOAD32_BYTE( "wcb_grm0.1", 0x000001, 0x080000, CRC(e26dcedb) SHA1(15441b97dd3d50d28007062fe28841fa3f762ec9) )
	ROM_LOAD32_BYTE( "wcb_grm0.2", 0x000002, 0x080000, CRC(32735875) SHA1(4017a8577d8efa8c5b95bd30723ebbf6ecaeba2b) )
	ROM_LOAD32_BYTE( "wcb_grm0.3", 0x000003, 0x080000, CRC(019d0ab8) SHA1(3281eada296ad746da80ef6e5909c50b03b90d08) )
	ROM_LOAD32_BYTE( "wcb_grm1.0", 0x200000, 0x080000, CRC(8bd31762) SHA1(a7274c8173b4fb04a6aed0b6a622b52a811a8c83) )
	ROM_LOAD32_BYTE( "wcb_grm1.1", 0x200001, 0x080000, CRC(b3f761fc) SHA1(5880ca1423cea9a7ca3d0875c8db33787f4056d4) )
	ROM_LOAD32_BYTE( "wcb_grm1.2", 0x200002, 0x080000, CRC(c22f44ad) SHA1(b25b11346ee1812b2be79105faf64faa0302c105) )
	ROM_LOAD32_BYTE( "wcb_grm1.3", 0x200003, 0x080000, CRC(036084c4) SHA1(6d2e402d2f4565e037a2676ba676e4b1da2b5dfe) )
	ROM_FILL(                      0x400000, 0x480000, 0xff )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "ensoniq.2mx", 0x000000, 0x200000, CRC(0814ab80) SHA1(e92525f7cf58cf480510c278fea705f67225e58f) ) /* Ensoniq 2MX16U 1350901801 */

	ROM_REGION16_BE( 0x400000, "ensoniq.2", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "wcb_srm.0",  0x000000, 0x080000, CRC(115bcd1f) SHA1(c321bf3145c11de1351c8f9cd554ab3d6600e854) )
	ROM_LOAD16_BYTE( "wcb_srm.1",  0x100000, 0x080000, CRC(87a4a4d8) SHA1(60db2f466686481857eb39b90ac7a19d0a96adac) )
ROM_END


ROM_START( wcbowl12 )	/* Version 1.2 (3-tier board set: P/N 1059 Rev 3, P/N 1079 Rev 1 & P/N 1060 Rev 0) */
	ROM_REGION16_BE( 0x80000, "user1", 0 )
	ROM_LOAD16_BYTE( "wcb_v12.u83", 0x00000, 0x20000, CRC(0602c5ce) SHA1(4339f77301f9c607c6f1dc81270d03681e874e69) ) /* Labeled as "WCB V1.2 (U83)" */
	ROM_LOAD16_BYTE( "wcb_v12.u88", 0x00001, 0x20000, CRC(49573493) SHA1(42813573f4ab951cd830193c0ffe2ce7d79c354b) ) /* Labeled as "WCB V1.2 (U88)" */

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "wcb_snd2.u17", 0x10000, 0x18000, CRC(c14907ba) SHA1(e52fb87c1f9b5847efc0ef15eb7e6c04dcf38110) ) /* Labeled as "WCB SND V2.0" */
	ROM_CONTINUE(             0x08000, 0x08000 )

	ROM_REGION( 0x880000, "gfx1", 0 )
	/* No known set specificly checks for this, however the GROM data may be in the form of four 8 Meg roms:
    ROM_LOAD32_BYTE( "wcb8grom.0_0", 0x000000, 0x100000, CRC(40837737) SHA1(f073943ec6f84285a8559553fb292ec1f8a629d0) ) Labeled as "WCB GROM0_0 *" ect
    ROM_LOAD32_BYTE( "wcb8grom.0_1", 0x000001, 0x100000, CRC(1615aee8) SHA1(6184919371a894b1d6f2e06a2b328cb55abed4a9) )
    ROM_LOAD32_BYTE( "wcb8grom.0_2", 0x000002, 0x100000, CRC(d8e0b06e) SHA1(4981c0cf16df68a1b4da7ebf65ca587c21292478) )
    ROM_LOAD32_BYTE( "wcb8grom.0_3", 0x000003, 0x100000, CRC(0348a7f0) SHA1(462f77514c0e9a28da63732a4f31e9483d4c483e) )
    */
	ROM_LOAD32_BYTE( "wcb_grm0.0", 0x000000, 0x080000, CRC(5d79aaae) SHA1(e1bf5c46843f69b8bac41dde73d89ba59b4c8b7f) )
	ROM_LOAD32_BYTE( "wcb_grm0.1", 0x000001, 0x080000, CRC(e26dcedb) SHA1(15441b97dd3d50d28007062fe28841fa3f762ec9) )
	ROM_LOAD32_BYTE( "wcb_grm0.2", 0x000002, 0x080000, CRC(32735875) SHA1(4017a8577d8efa8c5b95bd30723ebbf6ecaeba2b) )
	ROM_LOAD32_BYTE( "wcb_grm0.3", 0x000003, 0x080000, CRC(019d0ab8) SHA1(3281eada296ad746da80ef6e5909c50b03b90d08) )
	ROM_LOAD32_BYTE( "wcb_grm1.0", 0x200000, 0x080000, CRC(8bd31762) SHA1(a7274c8173b4fb04a6aed0b6a622b52a811a8c83) )
	ROM_LOAD32_BYTE( "wcb_grm1.1", 0x200001, 0x080000, CRC(b3f761fc) SHA1(5880ca1423cea9a7ca3d0875c8db33787f4056d4) )
	ROM_LOAD32_BYTE( "wcb_grm1.2", 0x200002, 0x080000, CRC(c22f44ad) SHA1(b25b11346ee1812b2be79105faf64faa0302c105) )
	ROM_LOAD32_BYTE( "wcb_grm1.3", 0x200003, 0x080000, CRC(036084c4) SHA1(6d2e402d2f4565e037a2676ba676e4b1da2b5dfe) )
	ROM_FILL(                      0x400000, 0x480000, 0xff )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "ensoniq.2mx", 0x000000, 0x200000, CRC(0814ab80) SHA1(e92525f7cf58cf480510c278fea705f67225e58f) ) /* Ensoniq 2MX16U 1350901801 */

	ROM_REGION16_BE( 0x400000, "ensoniq.2", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "wcb_srm.0",  0x000000, 0x080000, CRC(115bcd1f) SHA1(c321bf3145c11de1351c8f9cd554ab3d6600e854) )
	ROM_LOAD16_BYTE( "wcb_srm.1",  0x100000, 0x080000, CRC(87a4a4d8) SHA1(60db2f466686481857eb39b90ac7a19d0a96adac) )
ROM_END


ROM_START( drivedge )
	ROM_REGION32_BE( 0x8000, "user1", 0 )
	ROM_LOAD( "de-u130.bin", 0x00000, 0x8000, CRC(873579b0) SHA1(ce7fcbea780aee376c2f4c659a75fcf6b7abbdb4) )

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "desndu17.bin", 0x10000, 0x18000, CRC(6e8ca8bc) SHA1(98ad36877b40123b0396943754234df8de183687) )
	ROM_CONTINUE(             0x08000, 0x08000 )

	ROM_REGION( 0x10000, "cpu4", 0 )
	ROM_LOAD( "de-u7net.bin", 0x08000, 0x08000, CRC(dea8b9de) SHA1(46ba3a549522d7e6a32792814a04fd34839c7e55) )

	ROM_REGION( 0xa00000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "de-grom0.bin", 0x000000, 0x80000, CRC(7fe5b01b) SHA1(b31e48971253d77e2277434b1b1590cbbea2209f) )
	ROM_LOAD32_BYTE( "de-grom5.bin", 0x000001, 0x80000, CRC(5ea6dbc2) SHA1(c2de55ec6a527d0555504070a7ecb43b8aa797ea) )
	ROM_LOAD32_BYTE( "de-grm10.bin", 0x000002, 0x80000, CRC(76be06cd) SHA1(b533a07853b531e318c5a85139a74ca3edb9089f) )
	ROM_LOAD32_BYTE( "de-grm15.bin", 0x000003, 0x80000, CRC(119bf46b) SHA1(67f5434581d5f0042c7acd36d2c64ffe69efaa76) )
	ROM_LOAD32_BYTE( "de-grom1.bin", 0x200000, 0x80000, CRC(5b88e8b7) SHA1(04f05d9e811697c28a5671df6a9530594978decc) )
	ROM_LOAD32_BYTE( "de-grom6.bin", 0x200001, 0x80000, CRC(2cb76b9a) SHA1(0db32cb572121c8a751dcce55b94adc48f9be738) )
	ROM_LOAD32_BYTE( "de-grm11.bin", 0x200002, 0x80000, CRC(5d29018c) SHA1(11f346afedfac4f7b0d5d4995dd38ec2d7fc7777) )
	ROM_LOAD32_BYTE( "de-grm16.bin", 0x200003, 0x80000, CRC(476940fb) SHA1(00dab9aeb0d5cc23e4f78f15cb976ddcafa63b42) )
	ROM_LOAD32_BYTE( "de-grom2.bin", 0x400000, 0x80000, CRC(5ccc4c62) SHA1(fc49bba2208a1157fe0948fcadac79c597f382c4) )
	ROM_LOAD32_BYTE( "de-grom7.bin", 0x400001, 0x80000, CRC(45367070) SHA1(c7cf074f95cf287c4caf70d2286608c50ad01044) )
	ROM_LOAD32_BYTE( "de-grm12.bin", 0x400002, 0x80000, CRC(b978ef5a) SHA1(d1fce9c7966b8324ce1a4a99d92cd69ec32f5c47) )
	ROM_LOAD32_BYTE( "de-grm17.bin", 0x400003, 0x80000, CRC(eff8abac) SHA1(83da116368fae05a0c3c12ea72656681912a1175) )
	ROM_LOAD32_BYTE( "de-grom3.bin", 0x600000, 0x20000, CRC(9cd252c9) SHA1(7db6bdeeb2967154cd104ac2e20761cb99046d70) )
	ROM_LOAD32_BYTE( "de-grom8.bin", 0x600001, 0x20000, CRC(43f10ca4) SHA1(9eb0e2fd1adc25b334f86582be8e5960de0caba7) )
	ROM_LOAD32_BYTE( "de-grm13.bin", 0x600002, 0x20000, CRC(431d131e) SHA1(efe5a4aa65fde1f094adc6e701db8be94a4b625c) )
	ROM_LOAD32_BYTE( "de-grm18.bin", 0x600003, 0x20000, CRC(b09e0d9c) SHA1(b14ff39b028c0070ccca601c21542896168bd0b7) )
	ROM_LOAD32_BYTE( "de-grom4.bin", 0x800000, 0x20000, CRC(c499cdfa) SHA1(acec47fb606f999f9d88fdce1b5860d5afcd5106) )
	ROM_LOAD32_BYTE( "de-grom9.bin", 0x800001, 0x20000, CRC(e5f21566) SHA1(ce41c7e808799eea217e14e9aabe6ce617f87287) )
	ROM_LOAD32_BYTE( "de-grm14.bin", 0x800002, 0x20000, CRC(09dbe382) SHA1(a85ecba433eb9bb75b4060d1b6391f66f4c8146c) )
	ROM_LOAD32_BYTE( "de-grm19.bin", 0x800003, 0x20000, CRC(4ced78e1) SHA1(7995c8684ca28cbdf620d10297850463fa473fe8) )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "ensoniq.2m", 0x000000, 0x200000, CRC(9fdc4825) SHA1(71e5255c66d9010be7e6f27916b605441fc53839) ) /* Ensoniq 2m 1350901601 */

	ROM_REGION16_BE( 0x400000, "ensoniq.2", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "de-srom0.bin", 0x000000, 0x80000, CRC(649c685f) SHA1(95d8f257cac621c8bd4abaa88ea5f7b3b8adea4c) )
	ROM_LOAD16_BYTE( "de-srom1.bin", 0x100000, 0x80000, CRC(df4fff97) SHA1(3c43623bfc176639417e86a037b92026e84a5dce) )
ROM_END


ROM_START( sftm )	/* Version 1.12 */
	ROM_REGION32_BE( CODE_SIZE, "user1", 0 )
	ROM_LOAD32_BYTE( "sftmrom0.112", 0x00000, 0x40000, CRC(9d09355c) SHA1(ca8c31d580e4b18b630c38e4ac1c353cf27ab4a2) )
	ROM_LOAD32_BYTE( "sftmrom1.112", 0x00001, 0x40000, CRC(a58ac6a9) SHA1(a481a789c397151efcbec7ad9983daa30f289d4e) )
	ROM_LOAD32_BYTE( "sftmrom2.112", 0x00002, 0x40000, CRC(2f21a4f6) SHA1(66b158c40375a0f729d44fd4c888cf6a5bbe2bf1) )
	ROM_LOAD32_BYTE( "sftmrom3.112", 0x00003, 0x40000, CRC(d26648d9) SHA1(9e3e1fa104da680c4a704d10d6518eea6382f039) )

	ROM_REGION( 0x48000, "soundcpu", 0 )
	ROM_LOAD( "sfmsndv1.u23", 0x10000, 0x38000, CRC(10d85366) SHA1(10d539c3ba37e277642c0c5888cb1886fb0f55fc) )
	ROM_CONTINUE(             0x08000, 0x08000 )

	ROM_REGION( 0x2080000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "rm0-0.bin",  0x0000000, 0x400000, CRC(09ef29cb) SHA1(430da5b00554582391478849d5b1547fe12eedbe) )
	ROM_LOAD32_BYTE( "rm0-1.bin",  0x0000001, 0x400000, CRC(6f5910fa) SHA1(1979d19dd36a9118dfaf021e05302982be5dbe69) )
	ROM_LOAD32_BYTE( "rm0-2.bin",  0x0000002, 0x400000, CRC(b8a2add5) SHA1(62e5bef936f014ac836c0cd5322eaba7018496b4) )
	ROM_LOAD32_BYTE( "rm0-3.bin",  0x0000003, 0x400000, CRC(6b6ff867) SHA1(72bc95ef361f9238602f0e03aed0adac8b59d227) )
	ROM_LOAD32_BYTE( "rm1-0.bin",  0x1000000, 0x400000, CRC(d5d65f77) SHA1(0bbb83bb42a442ef157472f3243ab44efa0c0aa0) )
	ROM_LOAD32_BYTE( "rm1-1.bin",  0x1000001, 0x400000, CRC(90467e27) SHA1(217561664871c60b0193337e34020ddd336b8f15) )
	ROM_LOAD32_BYTE( "rm1-2.bin",  0x1000002, 0x400000, CRC(903e56c2) SHA1(843ed9855ffdf37b100b3c5614139d552fd9cd6d) )
	ROM_LOAD32_BYTE( "rm1-3.bin",  0x1000003, 0x400000, CRC(fac35686) SHA1(ba99ab265620575c14c46806dc543d1f9fd24462) )
	ROM_LOAD32_BYTE( "sfmgrm.3_0", 0x2000000, 0x020000, CRC(3e1f76f7) SHA1(8aefe376e7248a583a6af02e5f9b2a4b48cc91d7) )
	ROM_LOAD32_BYTE( "sfmgrm.3_1", 0x2000001, 0x020000, CRC(578054b6) SHA1(99201959de28dbfd7692cedea4485751d3d4788f) )
	ROM_LOAD32_BYTE( "sfmgrm.3_2", 0x2000002, 0x020000, CRC(9af2f698) SHA1(e679728d8eba9f09379e503fa380202cd9adfde1) )
	ROM_LOAD32_BYTE( "sfmgrm.3_3", 0x2000003, 0x020000, CRC(cd38d1d6) SHA1(0cea60d6897b34eeb13997030f6ee7e1dfb3c833) )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "sfm_srom.0", 0x000000, 0x200000, CRC(6ca1d3fc) SHA1(904f4c55a1bc83531a6d87ff706afd8cdfaee83b) )

	ROM_REGION16_BE( 0x400000, "ensoniq.3", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "sfm_srom.3",  0x000000, 0x080000, CRC(4f181534) SHA1(e858a33b22558665427146ec79dfba48edc20c2c) )
ROM_END


ROM_START( sftm111 )	/* Version 1.11 */
	ROM_REGION32_BE( CODE_SIZE, "user1", 0 )
	ROM_LOAD32_BYTE( "sftmrom0.111", 0x00000, 0x40000, CRC(28187ddc) SHA1(7e4fa285be9389c913fca849098a7c0d9404df7a) )
	ROM_LOAD32_BYTE( "sftmrom1.111", 0x00001, 0x40000, CRC(ec2ce6fa) SHA1(b79aebb73ba77c2ebe081142853e81473743ac46) )
	ROM_LOAD32_BYTE( "sftmrom2.111", 0x00002, 0x40000, CRC(be20510e) SHA1(52e154fe4b77e461961fa23593383ef9b6dfb92f) )
	ROM_LOAD32_BYTE( "sftmrom3.111", 0x00003, 0x40000, CRC(eead342f) SHA1(b6df89527b527543df5535ef00945e64ff321e09) )

	ROM_REGION( 0x48000, "soundcpu", 0 )
	ROM_LOAD( "sfmsndv1.u23", 0x10000, 0x38000, CRC(10d85366) SHA1(10d539c3ba37e277642c0c5888cb1886fb0f55fc) )
	ROM_CONTINUE(             0x08000, 0x08000 )

	ROM_REGION( 0x2080000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "rm0-0.bin",  0x0000000, 0x400000, CRC(09ef29cb) SHA1(430da5b00554582391478849d5b1547fe12eedbe) )
	ROM_LOAD32_BYTE( "rm0-1.bin",  0x0000001, 0x400000, CRC(6f5910fa) SHA1(1979d19dd36a9118dfaf021e05302982be5dbe69) )
	ROM_LOAD32_BYTE( "rm0-2.bin",  0x0000002, 0x400000, CRC(b8a2add5) SHA1(62e5bef936f014ac836c0cd5322eaba7018496b4) )
	ROM_LOAD32_BYTE( "rm0-3.bin",  0x0000003, 0x400000, CRC(6b6ff867) SHA1(72bc95ef361f9238602f0e03aed0adac8b59d227) )
	ROM_LOAD32_BYTE( "rm1-0.bin",  0x1000000, 0x400000, CRC(d5d65f77) SHA1(0bbb83bb42a442ef157472f3243ab44efa0c0aa0) )
	ROM_LOAD32_BYTE( "rm1-1.bin",  0x1000001, 0x400000, CRC(90467e27) SHA1(217561664871c60b0193337e34020ddd336b8f15) )
	ROM_LOAD32_BYTE( "rm1-2.bin",  0x1000002, 0x400000, CRC(903e56c2) SHA1(843ed9855ffdf37b100b3c5614139d552fd9cd6d) )
	ROM_LOAD32_BYTE( "rm1-3.bin",  0x1000003, 0x400000, CRC(fac35686) SHA1(ba99ab265620575c14c46806dc543d1f9fd24462) )
	ROM_LOAD32_BYTE( "sfmgrm.3_0", 0x2000000, 0x020000, CRC(3e1f76f7) SHA1(8aefe376e7248a583a6af02e5f9b2a4b48cc91d7) )
	ROM_LOAD32_BYTE( "sfmgrm.3_1", 0x2000001, 0x020000, CRC(578054b6) SHA1(99201959de28dbfd7692cedea4485751d3d4788f) )
	ROM_LOAD32_BYTE( "sfmgrm.3_2", 0x2000002, 0x020000, CRC(9af2f698) SHA1(e679728d8eba9f09379e503fa380202cd9adfde1) )
	ROM_LOAD32_BYTE( "sfmgrm.3_3", 0x2000003, 0x020000, CRC(cd38d1d6) SHA1(0cea60d6897b34eeb13997030f6ee7e1dfb3c833) )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "sfm_srom.0", 0x000000, 0x200000, CRC(6ca1d3fc) SHA1(904f4c55a1bc83531a6d87ff706afd8cdfaee83b) )

	ROM_REGION16_BE( 0x400000, "ensoniq.3", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "sfm_srom.3",  0x000000, 0x080000, CRC(4f181534) SHA1(e858a33b22558665427146ec79dfba48edc20c2c) )
ROM_END


ROM_START( sftm110 )	/* Version 1.10 */
	ROM_REGION32_BE( CODE_SIZE, "user1", 0 )
	ROM_LOAD32_BYTE( "sftmrom0.110", 0x00000, 0x40000, CRC(00c0c63c) SHA1(39f614cca51fe7843c2158b6d9abdc52dc1b0bef) )
	ROM_LOAD32_BYTE( "sftmrom1.110", 0x00001, 0x40000, CRC(d4d2a67e) SHA1(88069caf171bb9c5602bc493f1f1dafa26d2fc78) )
	ROM_LOAD32_BYTE( "sftmrom2.110", 0x00002, 0x40000, CRC(d7b36c92) SHA1(fbdb6f3636b84b76cf42351392492b791429a0e4) )
	ROM_LOAD32_BYTE( "sftmrom3.110", 0x00003, 0x40000, CRC(be3efdbd) SHA1(169aff265d1520031988e51083d1f208cf2529b4) )

	ROM_REGION( 0x48000, "soundcpu", 0 )
	ROM_LOAD( "sfmsndv1.u23", 0x10000, 0x38000, CRC(10d85366) SHA1(10d539c3ba37e277642c0c5888cb1886fb0f55fc) )
	ROM_CONTINUE(             0x08000, 0x08000 )

	ROM_REGION( 0x2080000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "rm0-0.bin",  0x0000000, 0x400000, CRC(09ef29cb) SHA1(430da5b00554582391478849d5b1547fe12eedbe) )
	ROM_LOAD32_BYTE( "rm0-1.bin",  0x0000001, 0x400000, CRC(6f5910fa) SHA1(1979d19dd36a9118dfaf021e05302982be5dbe69) )
	ROM_LOAD32_BYTE( "rm0-2.bin",  0x0000002, 0x400000, CRC(b8a2add5) SHA1(62e5bef936f014ac836c0cd5322eaba7018496b4) )
	ROM_LOAD32_BYTE( "rm0-3.bin",  0x0000003, 0x400000, CRC(6b6ff867) SHA1(72bc95ef361f9238602f0e03aed0adac8b59d227) )
	ROM_LOAD32_BYTE( "rm1-0.bin",  0x1000000, 0x400000, CRC(d5d65f77) SHA1(0bbb83bb42a442ef157472f3243ab44efa0c0aa0) )
	ROM_LOAD32_BYTE( "rm1-1.bin",  0x1000001, 0x400000, CRC(90467e27) SHA1(217561664871c60b0193337e34020ddd336b8f15) )
	ROM_LOAD32_BYTE( "rm1-2.bin",  0x1000002, 0x400000, CRC(903e56c2) SHA1(843ed9855ffdf37b100b3c5614139d552fd9cd6d) )
	ROM_LOAD32_BYTE( "rm1-3.bin",  0x1000003, 0x400000, CRC(fac35686) SHA1(ba99ab265620575c14c46806dc543d1f9fd24462) )
	ROM_LOAD32_BYTE( "sfmgrm.3_0", 0x2000000, 0x020000, CRC(3e1f76f7) SHA1(8aefe376e7248a583a6af02e5f9b2a4b48cc91d7) )
	ROM_LOAD32_BYTE( "sfmgrm.3_1", 0x2000001, 0x020000, CRC(578054b6) SHA1(99201959de28dbfd7692cedea4485751d3d4788f) )
	ROM_LOAD32_BYTE( "sfmgrm.3_2", 0x2000002, 0x020000, CRC(9af2f698) SHA1(e679728d8eba9f09379e503fa380202cd9adfde1) )
	ROM_LOAD32_BYTE( "sfmgrm.3_3", 0x2000003, 0x020000, CRC(cd38d1d6) SHA1(0cea60d6897b34eeb13997030f6ee7e1dfb3c833) )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "sfm_srom.0", 0x000000, 0x200000, CRC(6ca1d3fc) SHA1(904f4c55a1bc83531a6d87ff706afd8cdfaee83b) )

	ROM_REGION16_BE( 0x400000, "ensoniq.3", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "sfm_srom.3",  0x000000, 0x080000, CRC(4f181534) SHA1(e858a33b22558665427146ec79dfba48edc20c2c) )
ROM_END


ROM_START( sftmj )	/* Version 1.12N (Japan) */
	ROM_REGION32_BE( CODE_SIZE, "user1", 0 )
	ROM_LOAD32_BYTE( "sfmprom0.12n", 0x00000, 0x40000, CRC(640a04a8) SHA1(adc7f5880962cbcc5f9f28e72a84070da6e2ec36) )
	ROM_LOAD32_BYTE( "sfmprom1.12n", 0x00001, 0x40000, CRC(2a27b690) SHA1(f63c3665ec030ecc2d7a10ead182941ade1c79d0) )
	ROM_LOAD32_BYTE( "sfmprom2.12n", 0x00002, 0x40000, CRC(cec1dd7b) SHA1(4c4cf14bc17ddef216d16a7fbcef2e4694b45eb4) )
	ROM_LOAD32_BYTE( "sfmprom3.12n", 0x00003, 0x40000, CRC(48fa60f4) SHA1(2d8bd4b5e3279af188feb3fb5e52a3d234bedd0a) )

	ROM_REGION( 0x48000, "soundcpu", 0 )
	ROM_LOAD( "snd_v111.u23", 0x10000, 0x38000, CRC(004854ed) SHA1(7ecb74dc3f45b038cc9904fea5c89d3e74fcbcf3) )
	ROM_CONTINUE(             0x08000, 0x08000 )

	ROM_REGION( 0x2080000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "rm0-0.bin",  0x0000000, 0x400000, CRC(09ef29cb) SHA1(430da5b00554582391478849d5b1547fe12eedbe) )
	ROM_LOAD32_BYTE( "rm0-1.bin",  0x0000001, 0x400000, CRC(6f5910fa) SHA1(1979d19dd36a9118dfaf021e05302982be5dbe69) )
	ROM_LOAD32_BYTE( "rm0-2.bin",  0x0000002, 0x400000, CRC(b8a2add5) SHA1(62e5bef936f014ac836c0cd5322eaba7018496b4) )
	ROM_LOAD32_BYTE( "rm0-3.bin",  0x0000003, 0x400000, CRC(6b6ff867) SHA1(72bc95ef361f9238602f0e03aed0adac8b59d227) )
	ROM_LOAD32_BYTE( "rm1-0.bin",  0x1000000, 0x400000, CRC(d5d65f77) SHA1(0bbb83bb42a442ef157472f3243ab44efa0c0aa0) )
	ROM_LOAD32_BYTE( "rm1-1.bin",  0x1000001, 0x400000, CRC(90467e27) SHA1(217561664871c60b0193337e34020ddd336b8f15) )
	ROM_LOAD32_BYTE( "rm1-2.bin",  0x1000002, 0x400000, CRC(903e56c2) SHA1(843ed9855ffdf37b100b3c5614139d552fd9cd6d) )
	ROM_LOAD32_BYTE( "rm1-3.bin",  0x1000003, 0x400000, CRC(fac35686) SHA1(ba99ab265620575c14c46806dc543d1f9fd24462) )
	ROM_LOAD32_BYTE( "sfmgrm.3_0", 0x2000000, 0x020000, CRC(3e1f76f7) SHA1(8aefe376e7248a583a6af02e5f9b2a4b48cc91d7) )
	ROM_LOAD32_BYTE( "sfmgrm.3_1", 0x2000001, 0x020000, CRC(578054b6) SHA1(99201959de28dbfd7692cedea4485751d3d4788f) )
	ROM_LOAD32_BYTE( "sfmgrm.3_2", 0x2000002, 0x020000, CRC(9af2f698) SHA1(e679728d8eba9f09379e503fa380202cd9adfde1) )
	ROM_LOAD32_BYTE( "sfmgrm.3_3", 0x2000003, 0x020000, CRC(cd38d1d6) SHA1(0cea60d6897b34eeb13997030f6ee7e1dfb3c833) )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "sfm_srom.0", 0x000000, 0x200000, CRC(6ca1d3fc) SHA1(904f4c55a1bc83531a6d87ff706afd8cdfaee83b) )

	ROM_REGION16_BE( 0x400000, "ensoniq.3", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "sfm_srom.3",  0x000000, 0x080000, CRC(4f181534) SHA1(e858a33b22558665427146ec79dfba48edc20c2c) )
ROM_END


ROM_START( shufshot )	/* Version 1.40 (PCB P/N 1083 Rev 2) */
	ROM_REGION32_BE( CODE_SIZE, "user1", 0 )
	ROM_LOAD32_BYTE( "shotprm0.140", 0x00000, 0x20000, CRC(33c0c98b) SHA1(9960a1e8131e5dde33450560665f315e5a97dc05) )
	ROM_LOAD32_BYTE( "shotprm1.140", 0x00001, 0x20000, CRC(d30a8831) SHA1(3a7937b542f703dfc2ae74b6fdb2ac6a8e22bdbd) )
	ROM_LOAD32_BYTE( "shotprm2.140", 0x00002, 0x20000, CRC(ea10ada8) SHA1(e8167def9929876f6d2b4771b265114d9b04136e) )
	ROM_LOAD32_BYTE( "shotprm3.140", 0x00003, 0x20000, CRC(4b28f28b) SHA1(602e230cc69ae872e40d72c85ec66f111826c15e) )

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "shotsnd.u88", 0x10000, 0x18000, CRC(e37d599d) SHA1(105f91e968ecf553d910a97726ddc536289bbb2b) )
	ROM_CONTINUE(            0x08000, 0x08000 )

	ROM_REGION( 0x800000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "shf_grom.0_0", 0x000000, 0x80000, CRC(832a3d6a) SHA1(443328fa61b79c93ec6c9d24893b2ec38358a905) )
	ROM_LOAD32_BYTE( "shf_grom.0_1", 0x000001, 0x80000, CRC(155e48a2) SHA1(187d65423ff9a3d6b6c34c885a1b2397fa5371cf) )
	ROM_LOAD32_BYTE( "shf_grom.0_2", 0x000002, 0x80000, CRC(9f2b470d) SHA1(012e917856042cbe00d476e3220a7f9c841bd199) )
	ROM_LOAD32_BYTE( "shf_grom.0_3", 0x000003, 0x80000, CRC(3855a16a) SHA1(f8c03efab87ddcb6940f657ad1f0138ceaa2118e) )
	ROM_LOAD32_BYTE( "shf_grom.1_0", 0x200000, 0x80000, CRC(ed140389) SHA1(f438a887b44a277f81e954bef73ac478eaff58c8) )
	ROM_LOAD32_BYTE( "shf_grom.1_1", 0x200001, 0x80000, CRC(bd2ffbca) SHA1(667692ce61a4896ceecf2a2bb37f742f175a6152) )
	ROM_LOAD32_BYTE( "shf_grom.1_2", 0x200002, 0x80000, CRC(c6de4187) SHA1(4854604330bb14f862abe22d755e08b54e0b1a04) )
	ROM_LOAD32_BYTE( "shf_grom.1_3", 0x200003, 0x80000, CRC(0c707aa2) SHA1(1da83523e04eeae4dbc8748a31a074331bf975d1) )
	ROM_LOAD32_BYTE( "shf_grom.2_0", 0x400000, 0x80000, CRC(529b4259) SHA1(4f98f28c83c3f8f822ea45d31be526af4a504cbc) )
	ROM_LOAD32_BYTE( "shf_grom.2_1", 0x400001, 0x80000, CRC(4b52ab1a) SHA1(5c438df7f2edea8f4d8734408fd94acf9d340755) )
	ROM_LOAD32_BYTE( "shf_grom.2_2", 0x400002, 0x80000, CRC(f45fad03) SHA1(3ff062928ef5bcdce8748ddd972c5da67207227a) )
	ROM_LOAD32_BYTE( "shf_grom.2_3", 0x400003, 0x80000, CRC(1bcb26c8) SHA1(49e730c56c4a3171a2962fa65f3b16481590c636) )
	ROM_LOAD32_BYTE( "shf_grom.3_0", 0x600000, 0x80000, CRC(a29763db) SHA1(e2154fb3e400aba300f1a23d53504588426dfbfe) )
	ROM_LOAD32_BYTE( "shf_grom.3_1", 0x600001, 0x80000, CRC(c757084c) SHA1(6f78ee13c68afd635aa3871cddf7207a19d4039b) )
	ROM_LOAD32_BYTE( "shf_grom.3_2", 0x600002, 0x80000, CRC(2971cb25) SHA1(5f0508ebff1bf66ee0f78addfa09a43f8fff9a36) )
	ROM_LOAD32_BYTE( "shf_grom.3_3", 0x600003, 0x80000, CRC(4fcbee51) SHA1(4e2ec4475986c3916c4566b2bc007f41a8c13609) )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "shf_srom.0", 0x000000, 0x80000, CRC(9a3cb6c9) SHA1(2af3ce3b1cd7688199195a66cf01bb83775d42fa) )
	ROM_LOAD16_BYTE( "shf_srom.1", 0x200000, 0x80000, CRC(8c89948a) SHA1(1054eca5de352c17f34f31ef16297ba6177a37ba) )
ROM_END


ROM_START( shufshot139 )	/* Version 1.39 (PCB P/N 1083 Rev 2) */
	ROM_REGION32_BE( CODE_SIZE, "user1", 0 )
	ROM_LOAD32_BYTE( "shotprm0.139", 0x00000, 0x20000, CRC(e811fc4a) SHA1(9e1d8f64ac89ac865929f6a23f66d95eeeda3ac9) )
	ROM_LOAD32_BYTE( "shotprm1.139", 0x00001, 0x20000, CRC(f9d120c5) SHA1(f94216f1fb6d810ddee98479e83f0719b30b768f) )
	ROM_LOAD32_BYTE( "shotprm2.139", 0x00002, 0x20000, CRC(9f12414d) SHA1(c1120079173f7ed6118f7105443afd7d38d8af94) )
	ROM_LOAD32_BYTE( "shotprm3.139", 0x00003, 0x20000, CRC(108a69be) SHA1(1b2ebe4767be084707522a90f009d3a70e03d578) )

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "shotsnd.u88", 0x10000, 0x18000, CRC(e37d599d) SHA1(105f91e968ecf553d910a97726ddc536289bbb2b) )
	ROM_CONTINUE(            0x08000, 0x08000 )

	ROM_REGION( 0x800000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "shf_grom.0_0", 0x000000, 0x80000, CRC(832a3d6a) SHA1(443328fa61b79c93ec6c9d24893b2ec38358a905) )
	ROM_LOAD32_BYTE( "shf_grom.0_1", 0x000001, 0x80000, CRC(155e48a2) SHA1(187d65423ff9a3d6b6c34c885a1b2397fa5371cf) )
	ROM_LOAD32_BYTE( "shf_grom.0_2", 0x000002, 0x80000, CRC(9f2b470d) SHA1(012e917856042cbe00d476e3220a7f9c841bd199) )
	ROM_LOAD32_BYTE( "shf_grom.0_3", 0x000003, 0x80000, CRC(3855a16a) SHA1(f8c03efab87ddcb6940f657ad1f0138ceaa2118e) )
	ROM_LOAD32_BYTE( "shf_grom.1_0", 0x200000, 0x80000, CRC(ed140389) SHA1(f438a887b44a277f81e954bef73ac478eaff58c8) )
	ROM_LOAD32_BYTE( "shf_grom.1_1", 0x200001, 0x80000, CRC(bd2ffbca) SHA1(667692ce61a4896ceecf2a2bb37f742f175a6152) )
	ROM_LOAD32_BYTE( "shf_grom.1_2", 0x200002, 0x80000, CRC(c6de4187) SHA1(4854604330bb14f862abe22d755e08b54e0b1a04) )
	ROM_LOAD32_BYTE( "shf_grom.1_3", 0x200003, 0x80000, CRC(0c707aa2) SHA1(1da83523e04eeae4dbc8748a31a074331bf975d1) )
	ROM_LOAD32_BYTE( "shf_grom.2_0", 0x400000, 0x80000, CRC(529b4259) SHA1(4f98f28c83c3f8f822ea45d31be526af4a504cbc) )
	ROM_LOAD32_BYTE( "shf_grom.2_1", 0x400001, 0x80000, CRC(4b52ab1a) SHA1(5c438df7f2edea8f4d8734408fd94acf9d340755) )
	ROM_LOAD32_BYTE( "shf_grom.2_2", 0x400002, 0x80000, CRC(f45fad03) SHA1(3ff062928ef5bcdce8748ddd972c5da67207227a) )
	ROM_LOAD32_BYTE( "shf_grom.2_3", 0x400003, 0x80000, CRC(1bcb26c8) SHA1(49e730c56c4a3171a2962fa65f3b16481590c636) )
	ROM_LOAD32_BYTE( "shf_grom.3_0", 0x600000, 0x80000, CRC(a29763db) SHA1(e2154fb3e400aba300f1a23d53504588426dfbfe) )
	ROM_LOAD32_BYTE( "shf_grom.3_1", 0x600001, 0x80000, CRC(c757084c) SHA1(6f78ee13c68afd635aa3871cddf7207a19d4039b) )
	ROM_LOAD32_BYTE( "shf_grom.3_2", 0x600002, 0x80000, CRC(2971cb25) SHA1(5f0508ebff1bf66ee0f78addfa09a43f8fff9a36) )
	ROM_LOAD32_BYTE( "shf_grom.3_3", 0x600003, 0x80000, CRC(4fcbee51) SHA1(4e2ec4475986c3916c4566b2bc007f41a8c13609) )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "shf_srom.0", 0x000000, 0x80000, CRC(9a3cb6c9) SHA1(2af3ce3b1cd7688199195a66cf01bb83775d42fa) )
	ROM_LOAD16_BYTE( "shf_srom.1", 0x200000, 0x80000, CRC(8c89948a) SHA1(1054eca5de352c17f34f31ef16297ba6177a37ba) )
ROM_END


ROM_START( shufshot137 )	/* Version 1.37 (PCB P/N 1083 Rev 2) */
	ROM_REGION32_BE( CODE_SIZE, "user1", 0 )
	ROM_LOAD32_BYTE( "shotprm0.137", 0x00000, 0x20000, CRC(6499c76f) SHA1(60fdaefb09088ac609addd40569bd7fab12593bc) )
	ROM_LOAD32_BYTE( "shotprm1.137", 0x00001, 0x20000, CRC(64fb47a4) SHA1(32ce9d91b16b8aaf545c0a22842ad8d806727a17) )
	ROM_LOAD32_BYTE( "shotprm2.137", 0x00002, 0x20000, CRC(e0df3025) SHA1(edff5c5c4486981ac0783f337a0845854d0217f0) )
	ROM_LOAD32_BYTE( "shotprm3.137", 0x00003, 0x20000, CRC(efa66ad8) SHA1(d8dc754529284e6c06b912e226c8a4520aab49fc) )

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "shotsnd.u88", 0x10000, 0x18000, CRC(e37d599d) SHA1(105f91e968ecf553d910a97726ddc536289bbb2b) )
	ROM_CONTINUE(            0x08000, 0x08000 )

	ROM_REGION( 0x800000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "shf_grom.0_0", 0x000000, 0x80000, CRC(832a3d6a) SHA1(443328fa61b79c93ec6c9d24893b2ec38358a905) )
	ROM_LOAD32_BYTE( "shf_grom.0_1", 0x000001, 0x80000, CRC(155e48a2) SHA1(187d65423ff9a3d6b6c34c885a1b2397fa5371cf) )
	ROM_LOAD32_BYTE( "shf_grom.0_2", 0x000002, 0x80000, CRC(9f2b470d) SHA1(012e917856042cbe00d476e3220a7f9c841bd199) )
	ROM_LOAD32_BYTE( "shf_grom.0_3", 0x000003, 0x80000, CRC(3855a16a) SHA1(f8c03efab87ddcb6940f657ad1f0138ceaa2118e) )
	ROM_LOAD32_BYTE( "shf_grom.1_0", 0x200000, 0x80000, CRC(ed140389) SHA1(f438a887b44a277f81e954bef73ac478eaff58c8) )
	ROM_LOAD32_BYTE( "shf_grom.1_1", 0x200001, 0x80000, CRC(bd2ffbca) SHA1(667692ce61a4896ceecf2a2bb37f742f175a6152) )
	ROM_LOAD32_BYTE( "shf_grom.1_2", 0x200002, 0x80000, CRC(c6de4187) SHA1(4854604330bb14f862abe22d755e08b54e0b1a04) )
	ROM_LOAD32_BYTE( "shf_grom.1_3", 0x200003, 0x80000, CRC(0c707aa2) SHA1(1da83523e04eeae4dbc8748a31a074331bf975d1) )
	ROM_LOAD32_BYTE( "shf_grom.2_0", 0x400000, 0x80000, CRC(529b4259) SHA1(4f98f28c83c3f8f822ea45d31be526af4a504cbc) )
	ROM_LOAD32_BYTE( "shf_grom.2_1", 0x400001, 0x80000, CRC(4b52ab1a) SHA1(5c438df7f2edea8f4d8734408fd94acf9d340755) )
	ROM_LOAD32_BYTE( "shf_grom.2_2", 0x400002, 0x80000, CRC(f45fad03) SHA1(3ff062928ef5bcdce8748ddd972c5da67207227a) )
	ROM_LOAD32_BYTE( "shf_grom.2_3", 0x400003, 0x80000, CRC(1bcb26c8) SHA1(49e730c56c4a3171a2962fa65f3b16481590c636) )
	ROM_LOAD32_BYTE( "shf_grom.3_0", 0x600000, 0x80000, CRC(a29763db) SHA1(e2154fb3e400aba300f1a23d53504588426dfbfe) )
	ROM_LOAD32_BYTE( "shf_grom.3_1", 0x600001, 0x80000, CRC(c757084c) SHA1(6f78ee13c68afd635aa3871cddf7207a19d4039b) )
	ROM_LOAD32_BYTE( "shf_grom.3_2", 0x600002, 0x80000, CRC(2971cb25) SHA1(5f0508ebff1bf66ee0f78addfa09a43f8fff9a36) )
	ROM_LOAD32_BYTE( "shf_grom.3_3", 0x600003, 0x80000, CRC(4fcbee51) SHA1(4e2ec4475986c3916c4566b2bc007f41a8c13609) )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "shf_srom.0", 0x000000, 0x80000, CRC(9a3cb6c9) SHA1(2af3ce3b1cd7688199195a66cf01bb83775d42fa) )
	ROM_LOAD16_BYTE( "shf_srom.1", 0x200000, 0x80000, CRC(8c89948a) SHA1(1054eca5de352c17f34f31ef16297ba6177a37ba) )
ROM_END


ROM_START( gt3d )	/* Version 1.93N for the single large type PCB P/N 1083 Rev 2 */
	ROM_REGION32_BE( CODE_SIZE, "user1", 0 )
	ROM_LOAD32_BYTE( "gtg3prm0.93n", 0x00000, 0x80000, CRC(cacacb44) SHA1(747f48a52e140ab3e321b8f6a96f06bc70dc7cfa) )
	ROM_LOAD32_BYTE( "gtg3prm1.93n", 0x00001, 0x80000, CRC(4c172d7f) SHA1(d4217d5d4d561e46e0213e6f8dc8d9a874f86877) )
	ROM_LOAD32_BYTE( "gtg3prm2.93n", 0x00002, 0x80000, CRC(b53fe6f0) SHA1(4fbaa2f2a877c051b06ffa570e40156142d8e6bf) )
	ROM_LOAD32_BYTE( "gtg3prm3.93n", 0x00003, 0x80000, CRC(78468761) SHA1(f3a785dffa5269b5dbd3aee63ed97fe8b8cdcc0e) )

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "gt_nr.u88", 0x10000, 0x18000, CRC(2cee9e98) SHA1(02edac7abab2335c1cd824d1d9b26aa32238a2de) )
	ROM_CONTINUE(          0x08000, 0x08000 )

	ROM_REGION( 0x600000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "gtg3_grm.0_0", 0x000000, 0x80000, CRC(1b10379d) SHA1(b6d61771e2bc3909ea4229777867b217a3e9e580) )
	ROM_LOAD32_BYTE( "gtg3_grm.0_1", 0x000001, 0x80000, CRC(3b852e1a) SHA1(4b3653d55c52fc2eb5438d1604247a8e68d569a5) )
	ROM_LOAD32_BYTE( "gtg3_grm.0_2", 0x000002, 0x80000, CRC(d43ffb35) SHA1(748be07e03bbbb40cd7a725c708cacc46f33d9ca) )
	ROM_LOAD32_BYTE( "gtg3_grm.0_3", 0x000003, 0x80000, CRC(2d24e93e) SHA1(505272c1a509d013c2ce9fb8e8e0ac88870d74e7) )
	ROM_LOAD32_BYTE( "gtg3_grm.1_0", 0x200000, 0x80000, CRC(4476b239) SHA1(71b8258ca94859eb4bdf83b855a87aff79d3df2b) )
	ROM_LOAD32_BYTE( "gtg3_grm.1_1", 0x200001, 0x80000, CRC(0aadfad2) SHA1(56a283b30a13b77ec53b7ae2b4129d44b7fa25d4) )
	ROM_LOAD32_BYTE( "gtg3_grm.1_2", 0x200002, 0x80000, CRC(27871980) SHA1(fe473d12cb4805e25dcac8f9ae187891936b961b) )
	ROM_LOAD32_BYTE( "gtg3_grm.1_3", 0x200003, 0x80000, CRC(7dbc242b) SHA1(9aa5074cad633446e0110a1a3d9c6e1f2158070a) )
	ROM_LOAD32_BYTE( "gtg3tgrm.2_0", 0x400000, 0x80000, CRC(80ae7148) SHA1(e19d3390a2a0dad260d770fdbbb64d1f8e43d53f) )
	ROM_LOAD32_BYTE( "gtg3tgrm.2_1", 0x400001, 0x80000, CRC(0f85a618) SHA1(d9ced21c20f9ed6b7f19e7645d75b239ea709b79) )
	ROM_LOAD32_BYTE( "gtg3tgrm.2_2", 0x400002, 0x80000, CRC(09ca5fbf) SHA1(6a6ed4d5d76035d8acc33c6494fba6012194362e) )
	ROM_LOAD32_BYTE( "gtg3tgrm.2_3", 0x400003, 0x80000, CRC(d136853a) SHA1(0777d6bfab9e3d57c2a61d058fd185fc1f547698) )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "gt_srom0.nr", 0x000000, 0x100000, CRC(44983bd7) SHA1(a6ac966ec113b079434d7f871e4ce7266206d234) )
	ROM_LOAD16_BYTE( "gt_srom1.nr", 0x200000, 0x080000, CRC(1b3f18b6) SHA1(3b65de6a90c5ede183b5f8ca1875736bc1425772) )
ROM_END


ROM_START( gt3ds192 ) /* Version 1.92 for the 3 tier type PCB with short ROM board P/N 1069 REV 2 */
	ROM_REGION32_BE( CODE_SIZE, "user1", 0 )
	ROM_LOAD32_BYTE( "gtg3prm0.92s", 0x00000, 0x80000, CRC(eee38005) SHA1(3a879dce1abf449e847ba8f45ff5e1d70d13c966) )
	ROM_LOAD32_BYTE( "gtg3prm1.92s", 0x00001, 0x80000, CRC(818ba70e) SHA1(abeadd0efa8ea0d7924ff0951aafe8ee4a5f45d9) )
	ROM_LOAD32_BYTE( "gtg3prm2.92s", 0x00002, 0x80000, CRC(7ab661a1) SHA1(9db0a64c91ba15dad6ca071639d8e2d366dc7756) )
	ROM_LOAD32_BYTE( "gtg3prm3.92s", 0x00003, 0x80000, CRC(f9f96c01) SHA1(037c8d2d0d81c08745c044121c62f05a814db576) )

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "gt312snd.u23", 0x10000, 0x18000, CRC(cbbe41f9) SHA1(6a602addff87d32bb6df3ffb0563e8b2d3c4adcc) )
	ROM_CONTINUE(             0x08000, 0x08000 )

	ROM_REGION( 0x600000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "gtg3_grm.0_0", 0x000000, 0x80000, CRC(1b10379d) SHA1(b6d61771e2bc3909ea4229777867b217a3e9e580) )
	ROM_LOAD32_BYTE( "gtg3_grm.0_1", 0x000001, 0x80000, CRC(3b852e1a) SHA1(4b3653d55c52fc2eb5438d1604247a8e68d569a5) )
	ROM_LOAD32_BYTE( "gtg3_grm.0_2", 0x000002, 0x80000, CRC(d43ffb35) SHA1(748be07e03bbbb40cd7a725c708cacc46f33d9ca) )
	ROM_LOAD32_BYTE( "gtg3_grm.0_3", 0x000003, 0x80000, CRC(2d24e93e) SHA1(505272c1a509d013c2ce9fb8e8e0ac88870d74e7) )
	ROM_LOAD32_BYTE( "gtg3_grm.1_0", 0x200000, 0x80000, CRC(4476b239) SHA1(71b8258ca94859eb4bdf83b855a87aff79d3df2b) )
	ROM_LOAD32_BYTE( "gtg3_grm.1_1", 0x200001, 0x80000, CRC(0aadfad2) SHA1(56a283b30a13b77ec53b7ae2b4129d44b7fa25d4) )
	ROM_LOAD32_BYTE( "gtg3_grm.1_2", 0x200002, 0x80000, CRC(27871980) SHA1(fe473d12cb4805e25dcac8f9ae187891936b961b) )
	ROM_LOAD32_BYTE( "gtg3_grm.1_3", 0x200003, 0x80000, CRC(7dbc242b) SHA1(9aa5074cad633446e0110a1a3d9c6e1f2158070a) )
	ROM_LOAD32_BYTE( "gtg3_grm.2_0", 0x400000, 0x80000, CRC(15cb22bc) SHA1(571898d34099468c48aaf84a0fcc9b6eb30a8e38) )
	ROM_LOAD32_BYTE( "gtg3_grm.2_1", 0x400001, 0x80000, CRC(52f3ad44) SHA1(454a29cc82b05ee24834ef617db0c96270f9400c) )
	ROM_LOAD32_BYTE( "gtg3_grm.2_2", 0x400002, 0x80000, CRC(c7712125) SHA1(d3933b850617324b0c877686ec62abb52285b824) )
	ROM_LOAD32_BYTE( "gtg3_grm.2_3", 0x400003, 0x80000, CRC(77869bcf) SHA1(dde3b64578b79a94c7e346561a691229bbcfadac) )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "ensoniq.2m", 0x000000, 0x200000, CRC(9fdc4825) SHA1(71e5255c66d9010be7e6f27916b605441fc53839) ) /* Ensoniq 2m 1350901601 */

	ROM_REGION16_BE( 0x400000, "ensoniq.2", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "gtg3_srm.1", 0x000000, 0x80000, CRC(ac669434) SHA1(6941884c553515f5bc06af772897835a44aa8e4c) )
	ROM_LOAD16_BYTE( "gtg3_srm.2", 0x200000, 0x80000, CRC(6853578e) SHA1(5033a5614da4a2fe93766bb019e1479205298f75) )

	ROM_REGION16_BE( 0x400000, "ensoniq.3", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "gtg3_srm.3", 0x000000, 0x20000, CRC(d2462965) SHA1(45d2c9aaac681315d2e5cb39be151467f278e395) )
ROM_END


ROM_START( gt3dl192 ) /* Version 1.92 for the 3 tier type PCB with long ROM board P/N 1080 REV 5 */
	ROM_REGION32_BE( CODE_SIZE, "user1", 0 )
	ROM_LOAD32_BYTE( "gtg3prm0.92l", 0x00000, 0x80000, CRC(b449b939) SHA1(d5845fa4ed2702c2f05bfd22fe436c3b85ace0f8) )
	ROM_LOAD32_BYTE( "gtg3prm1.92l", 0x00001, 0x80000, CRC(ff986e67) SHA1(1d03aa7bf6a301eedb2beef93d54b58abe4e63a6) )
	ROM_LOAD32_BYTE( "gtg3prm2.92l", 0x00002, 0x80000, CRC(eb959447) SHA1(62a6382c5dc7d97b19c2e21ed47bb87d530cb43d) )
	ROM_LOAD32_BYTE( "gtg3prm3.92l", 0x00003, 0x80000, CRC(0265b798) SHA1(44c73f66f6b29a3cc6208cd7fde00605b61f8f15) )

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "gt322snd.u23", 0x10000, 0x18000, CRC(26fe2e92) SHA1(437ca0ea94dc0fa215f5375daa41d3dfe9bb17e0) )
	ROM_CONTINUE(             0x08000, 0x08000 )

	ROM_REGION( 0x600000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "gtg3_grm.0_0", 0x000000, 0x80000, CRC(1b10379d) SHA1(b6d61771e2bc3909ea4229777867b217a3e9e580) )
	ROM_LOAD32_BYTE( "gtg3_grm.0_1", 0x000001, 0x80000, CRC(3b852e1a) SHA1(4b3653d55c52fc2eb5438d1604247a8e68d569a5) )
	ROM_LOAD32_BYTE( "gtg3_grm.0_2", 0x000002, 0x80000, CRC(d43ffb35) SHA1(748be07e03bbbb40cd7a725c708cacc46f33d9ca) )
	ROM_LOAD32_BYTE( "gtg3_grm.0_3", 0x000003, 0x80000, CRC(2d24e93e) SHA1(505272c1a509d013c2ce9fb8e8e0ac88870d74e7) )
	ROM_LOAD32_BYTE( "gtg3_grm.1_0", 0x200000, 0x80000, CRC(4476b239) SHA1(71b8258ca94859eb4bdf83b855a87aff79d3df2b) )
	ROM_LOAD32_BYTE( "gtg3_grm.1_1", 0x200001, 0x80000, CRC(0aadfad2) SHA1(56a283b30a13b77ec53b7ae2b4129d44b7fa25d4) )
	ROM_LOAD32_BYTE( "gtg3_grm.1_2", 0x200002, 0x80000, CRC(27871980) SHA1(fe473d12cb4805e25dcac8f9ae187891936b961b) )
	ROM_LOAD32_BYTE( "gtg3_grm.1_3", 0x200003, 0x80000, CRC(7dbc242b) SHA1(9aa5074cad633446e0110a1a3d9c6e1f2158070a) )
	ROM_LOAD32_BYTE( "gtg3_grm.2_0", 0x400000, 0x80000, CRC(15cb22bc) SHA1(571898d34099468c48aaf84a0fcc9b6eb30a8e38) )
	ROM_LOAD32_BYTE( "gtg3_grm.2_1", 0x400001, 0x80000, CRC(52f3ad44) SHA1(454a29cc82b05ee24834ef617db0c96270f9400c) )
	ROM_LOAD32_BYTE( "gtg3_grm.2_2", 0x400002, 0x80000, CRC(c7712125) SHA1(d3933b850617324b0c877686ec62abb52285b824) )
	ROM_LOAD32_BYTE( "gtg3_grm.2_3", 0x400003, 0x80000, CRC(77869bcf) SHA1(dde3b64578b79a94c7e346561a691229bbcfadac) )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "ensoniq.2mx", 0x000000, 0x200000, CRC(0814ab80) SHA1(e92525f7cf58cf480510c278fea705f67225e58f) ) /* Ensoniq 2MX16U 1350901801 */

	ROM_REGION16_BE( 0x400000, "ensoniq.2", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "gtg3_srm.1", 0x000000, 0x80000, CRC(ac669434) SHA1(6941884c553515f5bc06af772897835a44aa8e4c) )
	ROM_LOAD16_BYTE( "gtg3_srm.2", 0x200000, 0x80000, CRC(6853578e) SHA1(5033a5614da4a2fe93766bb019e1479205298f75) )

	ROM_REGION16_BE( 0x400000, "ensoniq.3", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "gtg3_srm.3", 0x000000, 0x20000, CRC(d2462965) SHA1(45d2c9aaac681315d2e5cb39be151467f278e395) )
ROM_END


ROM_START( gt3dl191 ) /* Version 1.91 for the 3 tier type PCB with long ROM board P/N 1080 REV 5 */
	ROM_REGION32_BE( CODE_SIZE, "user1", 0 )
	ROM_LOAD32_BYTE( "gtg3prm0.91l", 0x00000, 0x80000, CRC(a3ea30d8) SHA1(675ca44b3a4fb542dfe4e9ce8463d2fc91491405) )
	ROM_LOAD32_BYTE( "gtg3prm1.91l", 0x00001, 0x80000, CRC(3aa87e56) SHA1(67c8bb5a869e1ff816e3d1da74fe4de4b1b3203c) )
	ROM_LOAD32_BYTE( "gtg3prm2.91l", 0x00002, 0x80000, CRC(41720e87) SHA1(c699b9ac892649004f1437bd3fe68a23b5d7ba27) )
	ROM_LOAD32_BYTE( "gtg3prm3.91l", 0x00003, 0x80000, CRC(30946139) SHA1(94ba341e13ffa27b56c12242b156fdf0698ad171) )

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "gt321snd.u23", 0x10000, 0x18000, CRC(6ae2646d) SHA1(0c62cc5f2911913167c5391648325409e7a3d892) )
	ROM_CONTINUE(             0x08000, 0x08000 )

	ROM_REGION( 0x600000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "gtg3_grm.0_0", 0x000000, 0x80000, CRC(1b10379d) SHA1(b6d61771e2bc3909ea4229777867b217a3e9e580) )
	ROM_LOAD32_BYTE( "gtg3_grm.0_1", 0x000001, 0x80000, CRC(3b852e1a) SHA1(4b3653d55c52fc2eb5438d1604247a8e68d569a5) )
	ROM_LOAD32_BYTE( "gtg3_grm.0_2", 0x000002, 0x80000, CRC(d43ffb35) SHA1(748be07e03bbbb40cd7a725c708cacc46f33d9ca) )
	ROM_LOAD32_BYTE( "gtg3_grm.0_3", 0x000003, 0x80000, CRC(2d24e93e) SHA1(505272c1a509d013c2ce9fb8e8e0ac88870d74e7) )
	ROM_LOAD32_BYTE( "gtg3_grm.1_0", 0x200000, 0x80000, CRC(4476b239) SHA1(71b8258ca94859eb4bdf83b855a87aff79d3df2b) )
	ROM_LOAD32_BYTE( "gtg3_grm.1_1", 0x200001, 0x80000, CRC(0aadfad2) SHA1(56a283b30a13b77ec53b7ae2b4129d44b7fa25d4) )
	ROM_LOAD32_BYTE( "gtg3_grm.1_2", 0x200002, 0x80000, CRC(27871980) SHA1(fe473d12cb4805e25dcac8f9ae187891936b961b) )
	ROM_LOAD32_BYTE( "gtg3_grm.1_3", 0x200003, 0x80000, CRC(7dbc242b) SHA1(9aa5074cad633446e0110a1a3d9c6e1f2158070a) )
	ROM_LOAD32_BYTE( "gtg3_grm.2_0", 0x400000, 0x80000, CRC(15cb22bc) SHA1(571898d34099468c48aaf84a0fcc9b6eb30a8e38) )
	ROM_LOAD32_BYTE( "gtg3_grm.2_1", 0x400001, 0x80000, CRC(52f3ad44) SHA1(454a29cc82b05ee24834ef617db0c96270f9400c) )
	ROM_LOAD32_BYTE( "gtg3_grm.2_2", 0x400002, 0x80000, CRC(c7712125) SHA1(d3933b850617324b0c877686ec62abb52285b824) )
	ROM_LOAD32_BYTE( "gtg3_grm.2_3", 0x400003, 0x80000, CRC(77869bcf) SHA1(dde3b64578b79a94c7e346561a691229bbcfadac) )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "ensoniq.2mx", 0x000000, 0x200000, CRC(0814ab80) SHA1(e92525f7cf58cf480510c278fea705f67225e58f) ) /* Ensoniq 2MX16U 1350901801 */

	ROM_REGION16_BE( 0x400000, "ensoniq.2", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "gtg3_srm.1", 0x000000, 0x80000, CRC(ac669434) SHA1(6941884c553515f5bc06af772897835a44aa8e4c) )
	ROM_LOAD16_BYTE( "gtg3_srm.2", 0x200000, 0x80000, CRC(6853578e) SHA1(5033a5614da4a2fe93766bb019e1479205298f75) )

	ROM_REGION16_BE( 0x400000, "ensoniq.3", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "gtg3_srm.3", 0x000000, 0x20000, CRC(d2462965) SHA1(45d2c9aaac681315d2e5cb39be151467f278e395) )
ROM_END


ROM_START( gt3dv18 ) /* Version 1.8 for the 3 tier type PCB with short ROM board P/N 1069 REV 2 */
	ROM_REGION32_BE( CODE_SIZE, "user1", 0 )
	ROM_LOAD32_BYTE( "gtg3prm0.18", 0x00000, 0x80000, CRC(0fa53c40) SHA1(45e339dede03d749c00f9459eebb26de4965c31c) )
	ROM_LOAD32_BYTE( "gtg3prm1.18", 0x00001, 0x80000, CRC(bef2cbe3) SHA1(ffed58db023d05817bd7a32958699115f4fbf37d) )
	ROM_LOAD32_BYTE( "gtg3prm2.18", 0x00002, 0x80000, CRC(1d5fb128) SHA1(bef5ff44836a5d32431c4ef998c9041b5c769281) )
	ROM_LOAD32_BYTE( "gtg3prm3.18", 0x00003, 0x80000, CRC(5542c335) SHA1(8e906512dc9c14f99c52faaa4315a7839dbd648c) )

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "gt311snd.u23", 0x10000, 0x18000, CRC(2f4cde9f) SHA1(571597e992e334e87307830e4a6a439c9d15fa76) )
	ROM_CONTINUE(             0x08000, 0x08000 )

	ROM_REGION( 0x600000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "gtg3_grm.0_0", 0x000000, 0x80000, CRC(1b10379d) SHA1(b6d61771e2bc3909ea4229777867b217a3e9e580) )
	ROM_LOAD32_BYTE( "gtg3_grm.0_1", 0x000001, 0x80000, CRC(3b852e1a) SHA1(4b3653d55c52fc2eb5438d1604247a8e68d569a5) )
	ROM_LOAD32_BYTE( "gtg3_grm.0_2", 0x000002, 0x80000, CRC(d43ffb35) SHA1(748be07e03bbbb40cd7a725c708cacc46f33d9ca) )
	ROM_LOAD32_BYTE( "gtg3_grm.0_3", 0x000003, 0x80000, CRC(2d24e93e) SHA1(505272c1a509d013c2ce9fb8e8e0ac88870d74e7) )
	ROM_LOAD32_BYTE( "gtg3_grm.1_0", 0x200000, 0x80000, CRC(4476b239) SHA1(71b8258ca94859eb4bdf83b855a87aff79d3df2b) )
	ROM_LOAD32_BYTE( "gtg3_grm.1_1", 0x200001, 0x80000, CRC(0aadfad2) SHA1(56a283b30a13b77ec53b7ae2b4129d44b7fa25d4) )
	ROM_LOAD32_BYTE( "gtg3_grm.1_2", 0x200002, 0x80000, CRC(27871980) SHA1(fe473d12cb4805e25dcac8f9ae187891936b961b) )
	ROM_LOAD32_BYTE( "gtg3_grm.1_3", 0x200003, 0x80000, CRC(7dbc242b) SHA1(9aa5074cad633446e0110a1a3d9c6e1f2158070a) )
	ROM_LOAD32_BYTE( "gtg3_grm.2_0", 0x400000, 0x80000, CRC(15cb22bc) SHA1(571898d34099468c48aaf84a0fcc9b6eb30a8e38) )
	ROM_LOAD32_BYTE( "gtg3_grm.2_1", 0x400001, 0x80000, CRC(52f3ad44) SHA1(454a29cc82b05ee24834ef617db0c96270f9400c) )
	ROM_LOAD32_BYTE( "gtg3_grm.2_2", 0x400002, 0x80000, CRC(c7712125) SHA1(d3933b850617324b0c877686ec62abb52285b824) )
	ROM_LOAD32_BYTE( "gtg3_grm.2_3", 0x400003, 0x80000, CRC(77869bcf) SHA1(dde3b64578b79a94c7e346561a691229bbcfadac) )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "ensoniq.2m", 0x000000, 0x200000, CRC(9fdc4825) SHA1(71e5255c66d9010be7e6f27916b605441fc53839) ) /* Ensoniq 2m 1350901601 */

	ROM_REGION16_BE( 0x400000, "ensoniq.2", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "gtg3_srm.1", 0x000000, 0x80000, CRC(ac669434) SHA1(6941884c553515f5bc06af772897835a44aa8e4c) )
	ROM_LOAD16_BYTE( "gtg3_srm.2", 0x200000, 0x80000, CRC(6853578e) SHA1(5033a5614da4a2fe93766bb019e1479205298f75) )

	ROM_REGION16_BE( 0x400000, "ensoniq.3", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "gtg3_srm.3", 0x000000, 0x20000, CRC(d2462965) SHA1(45d2c9aaac681315d2e5cb39be151467f278e395) )
ROM_END


ROM_START( gt3dv17 ) /* Version 1.7 for the 3 tier type PCB with short ROM board P/N 1069 REV 2 */
	ROM_REGION32_BE( CODE_SIZE, "user1", 0 )
	ROM_LOAD32_BYTE( "gtg3prm0.17", 0x00000, 0x80000, CRC(9a6fc839) SHA1(bd9a98f4a29786a3f28abb4e7eca63f62bb0dcf7) )
	ROM_LOAD32_BYTE( "gtg3prm1.17", 0x00001, 0x80000, CRC(26606578) SHA1(791782d4d99c2239e2967a8cf1467bad831b7500) )
	ROM_LOAD32_BYTE( "gtg3prm2.17", 0x00002, 0x80000, CRC(9c4d348b) SHA1(04167b9c4f3c99c5a1e0396c094cb9185c8d17c1) )
	ROM_LOAD32_BYTE( "gtg3prm3.17", 0x00003, 0x80000, CRC(53b1d6e7) SHA1(f1662275d45316a892d7722cd2fa1c2259acbcfa) )

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "gt311snd.u23", 0x10000, 0x18000, CRC(2f4cde9f) SHA1(571597e992e334e87307830e4a6a439c9d15fa76) )
	ROM_CONTINUE(             0x08000, 0x08000 )

	ROM_REGION( 0x600000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "gtg3_grm.0_0", 0x000000, 0x80000, CRC(1b10379d) SHA1(b6d61771e2bc3909ea4229777867b217a3e9e580) )
	ROM_LOAD32_BYTE( "gtg3_grm.0_1", 0x000001, 0x80000, CRC(3b852e1a) SHA1(4b3653d55c52fc2eb5438d1604247a8e68d569a5) )
	ROM_LOAD32_BYTE( "gtg3_grm.0_2", 0x000002, 0x80000, CRC(d43ffb35) SHA1(748be07e03bbbb40cd7a725c708cacc46f33d9ca) )
	ROM_LOAD32_BYTE( "gtg3_grm.0_3", 0x000003, 0x80000, CRC(2d24e93e) SHA1(505272c1a509d013c2ce9fb8e8e0ac88870d74e7) )
	ROM_LOAD32_BYTE( "gtg3_grm.1_0", 0x200000, 0x80000, CRC(4476b239) SHA1(71b8258ca94859eb4bdf83b855a87aff79d3df2b) )
	ROM_LOAD32_BYTE( "gtg3_grm.1_1", 0x200001, 0x80000, CRC(0aadfad2) SHA1(56a283b30a13b77ec53b7ae2b4129d44b7fa25d4) )
	ROM_LOAD32_BYTE( "gtg3_grm.1_2", 0x200002, 0x80000, CRC(27871980) SHA1(fe473d12cb4805e25dcac8f9ae187891936b961b) )
	ROM_LOAD32_BYTE( "gtg3_grm.1_3", 0x200003, 0x80000, CRC(7dbc242b) SHA1(9aa5074cad633446e0110a1a3d9c6e1f2158070a) )
	ROM_LOAD32_BYTE( "gtg3_grm.2_0", 0x400000, 0x80000, CRC(15cb22bc) SHA1(571898d34099468c48aaf84a0fcc9b6eb30a8e38) )
	ROM_LOAD32_BYTE( "gtg3_grm.2_1", 0x400001, 0x80000, CRC(52f3ad44) SHA1(454a29cc82b05ee24834ef617db0c96270f9400c) )
	ROM_LOAD32_BYTE( "gtg3_grm.2_2", 0x400002, 0x80000, CRC(c7712125) SHA1(d3933b850617324b0c877686ec62abb52285b824) )
	ROM_LOAD32_BYTE( "gtg3_grm.2_3", 0x400003, 0x80000, CRC(77869bcf) SHA1(dde3b64578b79a94c7e346561a691229bbcfadac) )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "ensoniq.2m", 0x000000, 0x200000, CRC(9fdc4825) SHA1(71e5255c66d9010be7e6f27916b605441fc53839) ) /* Ensoniq 2m 1350901601 */

	ROM_REGION16_BE( 0x400000, "ensoniq.2", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "gtg3_srm.1", 0x000000, 0x80000, CRC(ac669434) SHA1(6941884c553515f5bc06af772897835a44aa8e4c) )
	ROM_LOAD16_BYTE( "gtg3_srm.2", 0x200000, 0x80000, CRC(6853578e) SHA1(5033a5614da4a2fe93766bb019e1479205298f75) )

	ROM_REGION16_BE( 0x400000, "ensoniq.3", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "gtg3_srm.3", 0x000000, 0x20000, CRC(d2462965) SHA1(45d2c9aaac681315d2e5cb39be151467f278e395) )
ROM_END


ROM_START( gt3dv16 ) /* Version 1.6 for the 3 tier type PCB with short ROM board P/N 1069 REV 2 */
	ROM_REGION32_BE( CODE_SIZE, "user1", 0 )
	ROM_LOAD32_BYTE( "gtg3prm0.16", 0x00000, 0x80000, CRC(99d9a7e7) SHA1(62621fba9e9b1e1d11038238ebad8755db58ff21) )
	ROM_LOAD32_BYTE( "gtg3prm1.16", 0x00001, 0x80000, CRC(0ec4b307) SHA1(4e425462a32c7b85b02a694d6c65e69a0eb29ce8) )
	ROM_LOAD32_BYTE( "gtg3prm2.16", 0x00002, 0x80000, CRC(02ce6085) SHA1(8148a0d67646ec1ecb440e087ac20d3e64bf525d) )
	ROM_LOAD32_BYTE( "gtg3prm3.16", 0x00003, 0x80000, CRC(e77fa8a2) SHA1(34c6a5e24c115fdbb33d605aad07c7861ee7f3c8) )

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "gt311snd.u23", 0x10000, 0x18000, CRC(2f4cde9f) SHA1(571597e992e334e87307830e4a6a439c9d15fa76) )
	ROM_CONTINUE(             0x08000, 0x08000 )

	ROM_REGION( 0x600000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "gtg3_grm.0_0", 0x000000, 0x80000, CRC(1b10379d) SHA1(b6d61771e2bc3909ea4229777867b217a3e9e580) )
	ROM_LOAD32_BYTE( "gtg3_grm.0_1", 0x000001, 0x80000, CRC(3b852e1a) SHA1(4b3653d55c52fc2eb5438d1604247a8e68d569a5) )
	ROM_LOAD32_BYTE( "gtg3_grm.0_2", 0x000002, 0x80000, CRC(d43ffb35) SHA1(748be07e03bbbb40cd7a725c708cacc46f33d9ca) )
	ROM_LOAD32_BYTE( "gtg3_grm.0_3", 0x000003, 0x80000, CRC(2d24e93e) SHA1(505272c1a509d013c2ce9fb8e8e0ac88870d74e7) )
	ROM_LOAD32_BYTE( "gtg3_grm.1_0", 0x200000, 0x80000, CRC(4476b239) SHA1(71b8258ca94859eb4bdf83b855a87aff79d3df2b) )
	ROM_LOAD32_BYTE( "gtg3_grm.1_1", 0x200001, 0x80000, CRC(0aadfad2) SHA1(56a283b30a13b77ec53b7ae2b4129d44b7fa25d4) )
	ROM_LOAD32_BYTE( "gtg3_grm.1_2", 0x200002, 0x80000, CRC(27871980) SHA1(fe473d12cb4805e25dcac8f9ae187891936b961b) )
	ROM_LOAD32_BYTE( "gtg3_grm.1_3", 0x200003, 0x80000, CRC(7dbc242b) SHA1(9aa5074cad633446e0110a1a3d9c6e1f2158070a) )
	ROM_LOAD32_BYTE( "gtg3_grm.2_0", 0x400000, 0x80000, CRC(15cb22bc) SHA1(571898d34099468c48aaf84a0fcc9b6eb30a8e38) )
	ROM_LOAD32_BYTE( "gtg3_grm.2_1", 0x400001, 0x80000, CRC(52f3ad44) SHA1(454a29cc82b05ee24834ef617db0c96270f9400c) )
	ROM_LOAD32_BYTE( "gtg3_grm.2_2", 0x400002, 0x80000, CRC(c7712125) SHA1(d3933b850617324b0c877686ec62abb52285b824) )
	ROM_LOAD32_BYTE( "gtg3_grm.2_3", 0x400003, 0x80000, CRC(77869bcf) SHA1(dde3b64578b79a94c7e346561a691229bbcfadac) )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "ensoniq.2m", 0x000000, 0x200000, CRC(9fdc4825) SHA1(71e5255c66d9010be7e6f27916b605441fc53839) ) /* Ensoniq 2m 1350901601 */

	ROM_REGION16_BE( 0x400000, "ensoniq.2", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "gtg3_srm.1", 0x000000, 0x80000, CRC(ac669434) SHA1(6941884c553515f5bc06af772897835a44aa8e4c) )
	ROM_LOAD16_BYTE( "gtg3_srm.2", 0x200000, 0x80000, CRC(6853578e) SHA1(5033a5614da4a2fe93766bb019e1479205298f75) )

	ROM_REGION16_BE( 0x400000, "ensoniq.3", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "gtg3_srm.3", 0x000000, 0x20000, CRC(d2462965) SHA1(45d2c9aaac681315d2e5cb39be151467f278e395) )
ROM_END


ROM_START( gt3dv15 ) /* Version 1.5 for the 3 tier type PCB with short ROM board P/N 1069 REV 2 */
	ROM_REGION32_BE( CODE_SIZE, "user1", 0 )
	ROM_LOAD32_BYTE( "gtg3prm0.15", 0x00000, 0x80000, CRC(51a5e811) SHA1(ee5399e6f8ab2955c875adcb8ec7a859e42daa01) )
	ROM_LOAD32_BYTE( "gtg3prm1.15", 0x00001, 0x80000, CRC(1e8744ad) SHA1(2c38857f88f16b881b93db91ef9d15b710f2d40c) )
	ROM_LOAD32_BYTE( "gtg3prm2.15", 0x00002, 0x80000, CRC(e465c813) SHA1(b836cc01c6fb86980dda2d4418cd1ecb9899cb17) )
	ROM_LOAD32_BYTE( "gtg3prm3.15", 0x00003, 0x80000, CRC(3b25e198) SHA1(5b8cd9771126739e8834c11d0d6ae08b2990e3b0) )

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "gt311snd.u23", 0x10000, 0x18000, CRC(2f4cde9f) SHA1(571597e992e334e87307830e4a6a439c9d15fa76) )
	ROM_CONTINUE(             0x08000, 0x08000 )

	ROM_REGION( 0x600000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "gtg3_grm.0_0", 0x000000, 0x80000, CRC(1b10379d) SHA1(b6d61771e2bc3909ea4229777867b217a3e9e580) )
	ROM_LOAD32_BYTE( "gtg3_grm.0_1", 0x000001, 0x80000, CRC(3b852e1a) SHA1(4b3653d55c52fc2eb5438d1604247a8e68d569a5) )
	ROM_LOAD32_BYTE( "gtg3_grm.0_2", 0x000002, 0x80000, CRC(d43ffb35) SHA1(748be07e03bbbb40cd7a725c708cacc46f33d9ca) )
	ROM_LOAD32_BYTE( "gtg3_grm.0_3", 0x000003, 0x80000, CRC(2d24e93e) SHA1(505272c1a509d013c2ce9fb8e8e0ac88870d74e7) )
	ROM_LOAD32_BYTE( "gtg3_grm.1_0", 0x200000, 0x80000, CRC(4476b239) SHA1(71b8258ca94859eb4bdf83b855a87aff79d3df2b) )
	ROM_LOAD32_BYTE( "gtg3_grm.1_1", 0x200001, 0x80000, CRC(0aadfad2) SHA1(56a283b30a13b77ec53b7ae2b4129d44b7fa25d4) )
	ROM_LOAD32_BYTE( "gtg3_grm.1_2", 0x200002, 0x80000, CRC(27871980) SHA1(fe473d12cb4805e25dcac8f9ae187891936b961b) )
	ROM_LOAD32_BYTE( "gtg3_grm.1_3", 0x200003, 0x80000, CRC(7dbc242b) SHA1(9aa5074cad633446e0110a1a3d9c6e1f2158070a) )
	ROM_LOAD32_BYTE( "gtg3_grm.2_0", 0x400000, 0x80000, CRC(15cb22bc) SHA1(571898d34099468c48aaf84a0fcc9b6eb30a8e38) )
	ROM_LOAD32_BYTE( "gtg3_grm.2_1", 0x400001, 0x80000, CRC(52f3ad44) SHA1(454a29cc82b05ee24834ef617db0c96270f9400c) )
	ROM_LOAD32_BYTE( "gtg3_grm.2_2", 0x400002, 0x80000, CRC(c7712125) SHA1(d3933b850617324b0c877686ec62abb52285b824) )
	ROM_LOAD32_BYTE( "gtg3_grm.2_3", 0x400003, 0x80000, CRC(77869bcf) SHA1(dde3b64578b79a94c7e346561a691229bbcfadac) )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "ensoniq.2m", 0x000000, 0x200000, CRC(9fdc4825) SHA1(71e5255c66d9010be7e6f27916b605441fc53839) ) /* Ensoniq 2m 1350901601 */

	ROM_REGION16_BE( 0x400000, "ensoniq.2", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "gtg3_srm.1", 0x000000, 0x80000, CRC(ac669434) SHA1(6941884c553515f5bc06af772897835a44aa8e4c) )
	ROM_LOAD16_BYTE( "gtg3_srm.2", 0x200000, 0x80000, CRC(6853578e) SHA1(5033a5614da4a2fe93766bb019e1479205298f75) )

	ROM_REGION16_BE( 0x400000, "ensoniq.3", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "gtg3_srm.3", 0x000000, 0x20000, CRC(d2462965) SHA1(45d2c9aaac681315d2e5cb39be151467f278e395) )
ROM_END


ROM_START( gt3dv14 ) /* Version 1.4 for the 3 tier type PCB with short ROM board P/N 1069 REV 2 */
	ROM_REGION32_BE( CODE_SIZE, "user1", 0 )
	ROM_LOAD32_BYTE( "gtg3prm0.14", 0x00000, 0x80000, CRC(396934a7) SHA1(061941e7472f6386be7fa0675ba109dfccb6d35f) )
	ROM_LOAD32_BYTE( "gtg3prm1.14", 0x00001, 0x80000, CRC(5ba19b8d) SHA1(31d6c9c89ad231fb99d2ff762660fa390c53e129) )
	ROM_LOAD32_BYTE( "gtg3prm2.14", 0x00002, 0x80000, CRC(23991fcf) SHA1(5ba460ed88cebe1656501cadff27f2c0c90b721c) )
	ROM_LOAD32_BYTE( "gtg3prm3.14", 0x00003, 0x80000, CRC(2f7b5a26) SHA1(d0e676fc03ff7592f2d6bae806bee242e42c4452) )

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "gt310snd.u23", 0x10000, 0x18000, CRC(4f106cd1) SHA1(a3c3e6c649084fe6472e0a1f95d538c67d29098c) )
	ROM_CONTINUE(             0x08000, 0x08000 )

	ROM_REGION( 0x600000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "gtg3_grm.0_0", 0x000000, 0x80000, CRC(1b10379d) SHA1(b6d61771e2bc3909ea4229777867b217a3e9e580) )
	ROM_LOAD32_BYTE( "gtg3_grm.0_1", 0x000001, 0x80000, CRC(3b852e1a) SHA1(4b3653d55c52fc2eb5438d1604247a8e68d569a5) )
	ROM_LOAD32_BYTE( "gtg3_grm.0_2", 0x000002, 0x80000, CRC(d43ffb35) SHA1(748be07e03bbbb40cd7a725c708cacc46f33d9ca) )
	ROM_LOAD32_BYTE( "gtg3_grm.0_3", 0x000003, 0x80000, CRC(2d24e93e) SHA1(505272c1a509d013c2ce9fb8e8e0ac88870d74e7) )
	ROM_LOAD32_BYTE( "gtg3_grm.1_0", 0x200000, 0x80000, CRC(4476b239) SHA1(71b8258ca94859eb4bdf83b855a87aff79d3df2b) )
	ROM_LOAD32_BYTE( "gtg3_grm.1_1", 0x200001, 0x80000, CRC(0aadfad2) SHA1(56a283b30a13b77ec53b7ae2b4129d44b7fa25d4) )
	ROM_LOAD32_BYTE( "gtg3_grm.1_2", 0x200002, 0x80000, CRC(27871980) SHA1(fe473d12cb4805e25dcac8f9ae187891936b961b) )
	ROM_LOAD32_BYTE( "gtg3_grm.1_3", 0x200003, 0x80000, CRC(7dbc242b) SHA1(9aa5074cad633446e0110a1a3d9c6e1f2158070a) )
	ROM_LOAD32_BYTE( "gtg3_grm.2_0", 0x400000, 0x80000, CRC(15cb22bc) SHA1(571898d34099468c48aaf84a0fcc9b6eb30a8e38) )
	ROM_LOAD32_BYTE( "gtg3_grm.2_1", 0x400001, 0x80000, CRC(52f3ad44) SHA1(454a29cc82b05ee24834ef617db0c96270f9400c) )
	ROM_LOAD32_BYTE( "gtg3_grm.2_2", 0x400002, 0x80000, CRC(c7712125) SHA1(d3933b850617324b0c877686ec62abb52285b824) )
	ROM_LOAD32_BYTE( "gtg3_grm.2_3", 0x400003, 0x80000, CRC(77869bcf) SHA1(dde3b64578b79a94c7e346561a691229bbcfadac) )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "ensoniq.2m", 0x000000, 0x200000, CRC(9fdc4825) SHA1(71e5255c66d9010be7e6f27916b605441fc53839) ) /* Ensoniq 2m 1350901601 */

	ROM_REGION16_BE( 0x400000, "ensoniq.2", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "gtg3_srm.1", 0x000000, 0x80000, CRC(ac669434) SHA1(6941884c553515f5bc06af772897835a44aa8e4c) )
	ROM_LOAD16_BYTE( "gtg3_srm.2", 0x200000, 0x80000, CRC(6853578e) SHA1(5033a5614da4a2fe93766bb019e1479205298f75) )

	ROM_REGION16_BE( 0x400000, "ensoniq.3", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "gtg3_srm.3", 0x000000, 0x20000, CRC(d2462965) SHA1(45d2c9aaac681315d2e5cb39be151467f278e395) )
ROM_END


ROM_START( gt3dt231 ) /* Version 2.31 Tournament Edition (PCB P/N 1083 Rev 2) */
	ROM_REGION32_BE( CODE_SIZE, "user1", 0 )
	ROM_LOAD32_BYTE( "gtg3prm0.231", 0x00000, 0x100000, CRC(92a5c3e9) SHA1(a20c9ffb9b08eff1d59b77d08e6411275c58d932) )
	ROM_LOAD32_BYTE( "gtg3prm1.231", 0x00001, 0x100000, CRC(a3b60226) SHA1(2b78fb2917ad66883d2353d82f48b5aeb599d444) )
	ROM_LOAD32_BYTE( "gtg3prm2.231", 0x00002, 0x100000, CRC(d1659616) SHA1(7035ce46bde63024237e6c4777ddd10b58caeb98) )
	ROM_LOAD32_BYTE( "gtg3prm3.231", 0x00003, 0x100000, CRC(1d231ea2) SHA1(3001f25a337055d6fba522d10f5098c0acf6ff7a) )

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "gt_nr.u88", 0x10000, 0x18000, CRC(2cee9e98) SHA1(02edac7abab2335c1cd824d1d9b26aa32238a2de) )
	ROM_CONTINUE(          0x08000, 0x08000 )

	ROM_REGION( 0x600000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "gtg3_grm.0_0", 0x000000, 0x80000, CRC(1b10379d) SHA1(b6d61771e2bc3909ea4229777867b217a3e9e580) )
	ROM_LOAD32_BYTE( "gtg3_grm.0_1", 0x000001, 0x80000, CRC(3b852e1a) SHA1(4b3653d55c52fc2eb5438d1604247a8e68d569a5) )
	ROM_LOAD32_BYTE( "gtg3_grm.0_2", 0x000002, 0x80000, CRC(d43ffb35) SHA1(748be07e03bbbb40cd7a725c708cacc46f33d9ca) )
	ROM_LOAD32_BYTE( "gtg3_grm.0_3", 0x000003, 0x80000, CRC(2d24e93e) SHA1(505272c1a509d013c2ce9fb8e8e0ac88870d74e7) )
	ROM_LOAD32_BYTE( "gtg3_grm.1_0", 0x200000, 0x80000, CRC(4476b239) SHA1(71b8258ca94859eb4bdf83b855a87aff79d3df2b) )
	ROM_LOAD32_BYTE( "gtg3_grm.1_1", 0x200001, 0x80000, CRC(0aadfad2) SHA1(56a283b30a13b77ec53b7ae2b4129d44b7fa25d4) )
	ROM_LOAD32_BYTE( "gtg3_grm.1_2", 0x200002, 0x80000, CRC(27871980) SHA1(fe473d12cb4805e25dcac8f9ae187891936b961b) )
	ROM_LOAD32_BYTE( "gtg3_grm.1_3", 0x200003, 0x80000, CRC(7dbc242b) SHA1(9aa5074cad633446e0110a1a3d9c6e1f2158070a) )
	ROM_LOAD32_BYTE( "gtg3tgrm.2_0", 0x400000, 0x80000, CRC(80ae7148) SHA1(e19d3390a2a0dad260d770fdbbb64d1f8e43d53f) )
	ROM_LOAD32_BYTE( "gtg3tgrm.2_1", 0x400001, 0x80000, CRC(0f85a618) SHA1(d9ced21c20f9ed6b7f19e7645d75b239ea709b79) )
	ROM_LOAD32_BYTE( "gtg3tgrm.2_2", 0x400002, 0x80000, CRC(09ca5fbf) SHA1(6a6ed4d5d76035d8acc33c6494fba6012194362e) )
	ROM_LOAD32_BYTE( "gtg3tgrm.2_3", 0x400003, 0x80000, CRC(d136853a) SHA1(0777d6bfab9e3d57c2a61d058fd185fc1f547698) )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "gt_srom0.nr", 0x000000, 0x100000, CRC(44983bd7) SHA1(a6ac966ec113b079434d7f871e4ce7266206d234) )
	ROM_LOAD16_BYTE( "gt_srom1.nr", 0x200000, 0x080000, CRC(1b3f18b6) SHA1(3b65de6a90c5ede183b5f8ca1875736bc1425772) )
ROM_END


ROM_START( gt3dt211 ) /* Version 2.11 Tournament Edition (PCB P/N 1083 Rev 2) */
	ROM_REGION32_BE( CODE_SIZE, "user1", 0 )
	ROM_LOAD32_BYTE( "gtg3prm0.211", 0x00000, 0x100000, CRC(54360fdf) SHA1(05687f7f41f719458ee72b4517c7be3e33ee80cf) )
	ROM_LOAD32_BYTE( "gtg3prm1.211", 0x00001, 0x100000, CRC(9142ebb7) SHA1(ae3c9bf1a954c2f790cdb7953a6fea3d1d277aa9) )
	ROM_LOAD32_BYTE( "gtg3prm2.211", 0x00002, 0x100000, CRC(058b906a) SHA1(ff2cb3b955f7ce8041c967ed08b92e355d9abe2d) )
	ROM_LOAD32_BYTE( "gtg3prm3.211", 0x00003, 0x100000, CRC(8dbeee1b) SHA1(76f003619f3b4e7c55949f8ebf85364b0c9e7115) )

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "gt_nr.u88", 0x10000, 0x18000, CRC(2cee9e98) SHA1(02edac7abab2335c1cd824d1d9b26aa32238a2de) )
	ROM_CONTINUE(          0x08000, 0x08000 )

	ROM_REGION( 0x600000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "gtg38grm.0_0", 0x000000, 0x100000, CRC(22c481b7) SHA1(399bffec5e3b27aec2ebd8d7d49689b92f453e39) ) /* actually labeled "GTG3 GROM 0_0**" ect */
	ROM_LOAD32_BYTE( "gtg38grm.0_1", 0x000001, 0x100000, CRC(40e4032b) SHA1(e8a39ce415cf2326464efc5af23e9b7921621932) )
	ROM_LOAD32_BYTE( "gtg38grm.0_2", 0x000002, 0x100000, CRC(67a02ef9) SHA1(4f9e5217eeaf68fc72af1dc9e3a16d876de8d11d) )
	ROM_LOAD32_BYTE( "gtg38grm.0_3", 0x000003, 0x100000, CRC(1173a710) SHA1(1f612c1efbf38796707f5b5fecf9d4044691f031) )
	/*
    The above 4 roms have the same exact data as the other sets, but in 8 meg roms instead of 4 meg roms.
    This is the only set that specificaly checks for these roms in this format
    */
	ROM_LOAD32_BYTE( "gtg3tgrm.2_0", 0x400000, 0x080000, CRC(80ae7148) SHA1(e19d3390a2a0dad260d770fdbbb64d1f8e43d53f) ) /* actually labeled "GTG3 GROM 1_0*" ect */
	ROM_LOAD32_BYTE( "gtg3tgrm.2_1", 0x400001, 0x080000, CRC(0f85a618) SHA1(d9ced21c20f9ed6b7f19e7645d75b239ea709b79) )
	ROM_LOAD32_BYTE( "gtg3tgrm.2_2", 0x400002, 0x080000, CRC(09ca5fbf) SHA1(6a6ed4d5d76035d8acc33c6494fba6012194362e) )
	ROM_LOAD32_BYTE( "gtg3tgrm.2_3", 0x400003, 0x080000, CRC(d136853a) SHA1(0777d6bfab9e3d57c2a61d058fd185fc1f547698) )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "gt_srom0.nr", 0x000000, 0x100000, CRC(44983bd7) SHA1(a6ac966ec113b079434d7f871e4ce7266206d234) )
	ROM_LOAD16_BYTE( "gt_srom1.nr", 0x200000, 0x080000, CRC(1b3f18b6) SHA1(3b65de6a90c5ede183b5f8ca1875736bc1425772) )
ROM_END


ROM_START( gt97 ) /* Version 1.30 */
	ROM_REGION32_BE( CODE_SIZE, "user1", 0 )
	ROM_LOAD32_BYTE( "gt97prm0.130", 0x00000, 0x80000, CRC(7490ba4e) SHA1(b833d4175617727b3dc80e242457996a2efb844c) )
	ROM_LOAD32_BYTE( "gt97prm1.130", 0x00001, 0x80000, CRC(71f9c5f3) SHA1(c472aa1bcc217656f409614b73f0b7662215c202) )
	ROM_LOAD32_BYTE( "gt97prm2.130", 0x00002, 0x80000, CRC(8292b51a) SHA1(f8167b0aef87fb286006a17043de041c71afe41d) )
	ROM_LOAD32_BYTE( "gt97prm3.130", 0x00003, 0x80000, CRC(64539f72) SHA1(58fccee17987cb010d9b7f3b8f060a1b1040b21f) )

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "gt_nr.u88", 0x10000, 0x18000, CRC(2cee9e98) SHA1(02edac7abab2335c1cd824d1d9b26aa32238a2de) )
	ROM_CONTINUE(          0x08000, 0x08000 )

	ROM_REGION( 0x600000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "gt97_grm.0_0", 0x000000, 0x80000, CRC(81784aaf) SHA1(9544ed2087ca5f71c747e3b782513614937a51ed) )
	ROM_LOAD32_BYTE( "gt97_grm.0_1", 0x000001, 0x80000, CRC(345bda44) SHA1(b6d66c99bd68396e1e94ece76989a6190571ae14) )
	ROM_LOAD32_BYTE( "gt97_grm.0_2", 0x000002, 0x80000, CRC(b2beb40d) SHA1(2b955919e7e5f9093eb3a856f93c39684437b371) )
	ROM_LOAD32_BYTE( "gt97_grm.0_3", 0x000003, 0x80000, CRC(7cef32ff) SHA1(614c6aa26398d054a087dfe5e75cef78e6096f30) )
	ROM_LOAD32_BYTE( "gt97_grm.1_0", 0x200000, 0x80000, CRC(1cc4c309) SHA1(ef15f663ad1ea14c25f08fb6c1d5a58a38834ae6) )
	ROM_LOAD32_BYTE( "gt97_grm.1_1", 0x200001, 0x80000, CRC(512cea45) SHA1(03b229317c8f357fa2d2ead93bf519251edc6605) )
	ROM_LOAD32_BYTE( "gt97_grm.1_2", 0x200002, 0x80000, CRC(0599b505) SHA1(13078eae4051b3a4c1955cc246e58ef51ee2e53a) )
	ROM_LOAD32_BYTE( "gt97_grm.1_3", 0x200003, 0x80000, CRC(257eacc9) SHA1(2d03c38965930c4c1da4aa86583bb19c5f135100) )
	ROM_LOAD32_BYTE( "gt97_grm.2_0", 0x400000, 0x80000, CRC(95b6e7ee) SHA1(c27f69f69927e75ec8cba0df5c12e10418a53bbc) )
	ROM_LOAD32_BYTE( "gt97_grm.2_1", 0x400001, 0x80000, CRC(afd558b4) SHA1(0571bfc2b0131cdd62f92641dd1acf7b10ae09d8) )
	ROM_LOAD32_BYTE( "gt97_grm.2_2", 0x400002, 0x80000, CRC(5b7a8733) SHA1(e0a947ebabc8e6ab23b375afef49cd4be7d6d570) )
	ROM_LOAD32_BYTE( "gt97_grm.2_3", 0x400003, 0x80000, CRC(72e8ae60) SHA1(410da9970c50021f1724c8c51678691322eece92) )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "gt_srom0.nr", 0x000000, 0x100000, CRC(44983bd7) SHA1(a6ac966ec113b079434d7f871e4ce7266206d234) )
	ROM_LOAD16_BYTE( "gt_srom1.nr", 0x200000, 0x080000, CRC(1b3f18b6) SHA1(3b65de6a90c5ede183b5f8ca1875736bc1425772) )
ROM_END


ROM_START( gt97v122 ) /* Version 1.22 */
	ROM_REGION32_BE( CODE_SIZE, "user1", 0 )
	ROM_LOAD32_BYTE( "gt97prm0.122", 0x00000, 0x80000, CRC(4a543c99) SHA1(733b904a964c02a118186eaa0e4ff7670fb8ffef) )
	ROM_LOAD32_BYTE( "gt97prm1.122", 0x00001, 0x80000, CRC(27668628) SHA1(f0816f4589536de0dc30aef3eb2b99b2a22ddba4) )
	ROM_LOAD32_BYTE( "gt97prm2.122", 0x00002, 0x80000, CRC(d73a769f) SHA1(41c2416424efbd9d341ab5eea1451402dca1c340) )
	ROM_LOAD32_BYTE( "gt97prm3.122", 0x00003, 0x80000, CRC(03962957) SHA1(9b2f6afb3a5ba9c93468baf595527df15fb0fcde) )

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "gt_nr.u88", 0x10000, 0x18000, CRC(2cee9e98) SHA1(02edac7abab2335c1cd824d1d9b26aa32238a2de) )
	ROM_CONTINUE(          0x08000, 0x08000 )

	ROM_REGION( 0x600000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "gt97_grm.0_0", 0x000000, 0x80000, CRC(81784aaf) SHA1(9544ed2087ca5f71c747e3b782513614937a51ed) )
	ROM_LOAD32_BYTE( "gt97_grm.0_1", 0x000001, 0x80000, CRC(345bda44) SHA1(b6d66c99bd68396e1e94ece76989a6190571ae14) )
	ROM_LOAD32_BYTE( "gt97_grm.0_2", 0x000002, 0x80000, CRC(b2beb40d) SHA1(2b955919e7e5f9093eb3a856f93c39684437b371) )
	ROM_LOAD32_BYTE( "gt97_grm.0_3", 0x000003, 0x80000, CRC(7cef32ff) SHA1(614c6aa26398d054a087dfe5e75cef78e6096f30) )
	ROM_LOAD32_BYTE( "gt97_grm.1_0", 0x200000, 0x80000, CRC(1cc4c309) SHA1(ef15f663ad1ea14c25f08fb6c1d5a58a38834ae6) )
	ROM_LOAD32_BYTE( "gt97_grm.1_1", 0x200001, 0x80000, CRC(512cea45) SHA1(03b229317c8f357fa2d2ead93bf519251edc6605) )
	ROM_LOAD32_BYTE( "gt97_grm.1_2", 0x200002, 0x80000, CRC(0599b505) SHA1(13078eae4051b3a4c1955cc246e58ef51ee2e53a) )
	ROM_LOAD32_BYTE( "gt97_grm.1_3", 0x200003, 0x80000, CRC(257eacc9) SHA1(2d03c38965930c4c1da4aa86583bb19c5f135100) )
	ROM_LOAD32_BYTE( "gt97_grm.2_0", 0x400000, 0x80000, CRC(95b6e7ee) SHA1(c27f69f69927e75ec8cba0df5c12e10418a53bbc) )
	ROM_LOAD32_BYTE( "gt97_grm.2_1", 0x400001, 0x80000, CRC(afd558b4) SHA1(0571bfc2b0131cdd62f92641dd1acf7b10ae09d8) )
	ROM_LOAD32_BYTE( "gt97_grm.2_2", 0x400002, 0x80000, CRC(5b7a8733) SHA1(e0a947ebabc8e6ab23b375afef49cd4be7d6d570) )
	ROM_LOAD32_BYTE( "gt97_grm.2_3", 0x400003, 0x80000, CRC(72e8ae60) SHA1(410da9970c50021f1724c8c51678691322eece92) )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "gt_srom0.nr", 0x000000, 0x100000, CRC(44983bd7) SHA1(a6ac966ec113b079434d7f871e4ce7266206d234) )
	ROM_LOAD16_BYTE( "gt_srom1.nr", 0x200000, 0x080000, CRC(1b3f18b6) SHA1(3b65de6a90c5ede183b5f8ca1875736bc1425772) )
ROM_END


ROM_START( gt97v121 ) /* Version 1.21 */
	ROM_REGION32_BE( CODE_SIZE, "user1", 0 )
	ROM_LOAD32_BYTE( "gt97prm0.121", 0x00000, 0x80000, CRC(a210a2c6) SHA1(40475387d731ab87957e6b46725195a057d42067) )
	ROM_LOAD32_BYTE( "gt97prm1.121", 0x00001, 0x80000, CRC(a60806f8) SHA1(8695a24d7f186462bcfdfaa391896c7518717c57) )
	ROM_LOAD32_BYTE( "gt97prm2.121", 0x00002, 0x80000, CRC(a97ce668) SHA1(b392a3aab1f1887adc31fb802e7d7bedeb36c3c9) )
	ROM_LOAD32_BYTE( "gt97prm3.121", 0x00003, 0x80000, CRC(7a6b1ad8) SHA1(da652e005d32494bc2363ef806c3facd03213437) )

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "gt_nr.u88", 0x10000, 0x18000, CRC(2cee9e98) SHA1(02edac7abab2335c1cd824d1d9b26aa32238a2de) )
	ROM_CONTINUE(          0x08000, 0x08000 )

	ROM_REGION( 0x600000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "gt97_grm.0_0", 0x000000, 0x80000, CRC(81784aaf) SHA1(9544ed2087ca5f71c747e3b782513614937a51ed) )
	ROM_LOAD32_BYTE( "gt97_grm.0_1", 0x000001, 0x80000, CRC(345bda44) SHA1(b6d66c99bd68396e1e94ece76989a6190571ae14) )
	ROM_LOAD32_BYTE( "gt97_grm.0_2", 0x000002, 0x80000, CRC(b2beb40d) SHA1(2b955919e7e5f9093eb3a856f93c39684437b371) )
	ROM_LOAD32_BYTE( "gt97_grm.0_3", 0x000003, 0x80000, CRC(7cef32ff) SHA1(614c6aa26398d054a087dfe5e75cef78e6096f30) )
	ROM_LOAD32_BYTE( "gt97_grm.1_0", 0x200000, 0x80000, CRC(1cc4c309) SHA1(ef15f663ad1ea14c25f08fb6c1d5a58a38834ae6) )
	ROM_LOAD32_BYTE( "gt97_grm.1_1", 0x200001, 0x80000, CRC(512cea45) SHA1(03b229317c8f357fa2d2ead93bf519251edc6605) )
	ROM_LOAD32_BYTE( "gt97_grm.1_2", 0x200002, 0x80000, CRC(0599b505) SHA1(13078eae4051b3a4c1955cc246e58ef51ee2e53a) )
	ROM_LOAD32_BYTE( "gt97_grm.1_3", 0x200003, 0x80000, CRC(257eacc9) SHA1(2d03c38965930c4c1da4aa86583bb19c5f135100) )
	ROM_LOAD32_BYTE( "gt97_grm.2_0", 0x400000, 0x80000, CRC(95b6e7ee) SHA1(c27f69f69927e75ec8cba0df5c12e10418a53bbc) )
	ROM_LOAD32_BYTE( "gt97_grm.2_1", 0x400001, 0x80000, CRC(afd558b4) SHA1(0571bfc2b0131cdd62f92641dd1acf7b10ae09d8) )
	ROM_LOAD32_BYTE( "gt97_grm.2_2", 0x400002, 0x80000, CRC(5b7a8733) SHA1(e0a947ebabc8e6ab23b375afef49cd4be7d6d570) )
	ROM_LOAD32_BYTE( "gt97_grm.2_3", 0x400003, 0x80000, CRC(72e8ae60) SHA1(410da9970c50021f1724c8c51678691322eece92) )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "gt_srom0.nr", 0x000000, 0x100000, CRC(44983bd7) SHA1(a6ac966ec113b079434d7f871e4ce7266206d234) )
	ROM_LOAD16_BYTE( "gt_srom1.nr", 0x200000, 0x080000, CRC(1b3f18b6) SHA1(3b65de6a90c5ede183b5f8ca1875736bc1425772) )
ROM_END


ROM_START( gt97s121 ) /* Version 1.21S for the 3 tier type PCB with short ROM board P/N 1088 Rev 0 */
	ROM_REGION32_BE( CODE_SIZE, "user1", 0 )
	ROM_LOAD32_BYTE( "gt97prm0.21s", 0x00000, 0x80000, CRC(1143f45b) SHA1(df1f30d8054133be140f66219d69cd6bff74713a) )
	ROM_LOAD32_BYTE( "gt97prm1.21s", 0x00001, 0x80000, CRC(e7cfb1ea) SHA1(8f6cf3cae920e07e14009a933821abe5b0ec6eb1) )
	ROM_LOAD32_BYTE( "gt97prm2.21s", 0x00002, 0x80000, CRC(0cc24291) SHA1(941cada0f6c34ce8f6a23d5bf3ba052bb0edd9f1) )
	ROM_LOAD32_BYTE( "gt97prm3.21s", 0x00003, 0x80000, CRC(922727c2) SHA1(57f8b772841fdf2fb575301698ed60b119392ec9) )

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "gt322_nr.u23", 0x10000, 0x18000, CRC(04effd73) SHA1(4277031655f8de851eba0e4134ba619a12f5dd4a) )
	ROM_CONTINUE(             0x08000, 0x08000 )

	ROM_REGION( 0x600000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "gt97_grm.0_0", 0x000000, 0x80000, CRC(81784aaf) SHA1(9544ed2087ca5f71c747e3b782513614937a51ed) )
	ROM_LOAD32_BYTE( "gt97_grm.0_1", 0x000001, 0x80000, CRC(345bda44) SHA1(b6d66c99bd68396e1e94ece76989a6190571ae14) )
	ROM_LOAD32_BYTE( "gt97_grm.0_2", 0x000002, 0x80000, CRC(b2beb40d) SHA1(2b955919e7e5f9093eb3a856f93c39684437b371) )
	ROM_LOAD32_BYTE( "gt97_grm.0_3", 0x000003, 0x80000, CRC(7cef32ff) SHA1(614c6aa26398d054a087dfe5e75cef78e6096f30) )
	ROM_LOAD32_BYTE( "gt97_grm.1_0", 0x200000, 0x80000, CRC(1cc4c309) SHA1(ef15f663ad1ea14c25f08fb6c1d5a58a38834ae6) )
	ROM_LOAD32_BYTE( "gt97_grm.1_1", 0x200001, 0x80000, CRC(512cea45) SHA1(03b229317c8f357fa2d2ead93bf519251edc6605) )
	ROM_LOAD32_BYTE( "gt97_grm.1_2", 0x200002, 0x80000, CRC(0599b505) SHA1(13078eae4051b3a4c1955cc246e58ef51ee2e53a) )
	ROM_LOAD32_BYTE( "gt97_grm.1_3", 0x200003, 0x80000, CRC(257eacc9) SHA1(2d03c38965930c4c1da4aa86583bb19c5f135100) )
	ROM_LOAD32_BYTE( "gt97_grm.2_0", 0x400000, 0x80000, CRC(95b6e7ee) SHA1(c27f69f69927e75ec8cba0df5c12e10418a53bbc) )
	ROM_LOAD32_BYTE( "gt97_grm.2_1", 0x400001, 0x80000, CRC(afd558b4) SHA1(0571bfc2b0131cdd62f92641dd1acf7b10ae09d8) )
	ROM_LOAD32_BYTE( "gt97_grm.2_2", 0x400002, 0x80000, CRC(5b7a8733) SHA1(e0a947ebabc8e6ab23b375afef49cd4be7d6d570) )
	ROM_LOAD32_BYTE( "gt97_grm.2_3", 0x400003, 0x80000, CRC(72e8ae60) SHA1(410da9970c50021f1724c8c51678691322eece92) )

	ROM_REGION16_BE( 0x400000, "ensoniq.2", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "gt_srom0.nr", 0x000000, 0x100000, CRC(44983bd7) SHA1(a6ac966ec113b079434d7f871e4ce7266206d234) ) /* actually labeled "GTG3 SROM1 NR**" */
	ROM_LOAD16_BYTE( "gt_srom1.nr", 0x200000, 0x080000, CRC(1b3f18b6) SHA1(3b65de6a90c5ede183b5f8ca1875736bc1425772) ) /* actually labeled "GTG3 SROM2 NR*"  */
ROM_END


ROM_START( gt97v120 ) /* Version 1.20 */
	ROM_REGION32_BE( CODE_SIZE, "user1", 0 )
	ROM_LOAD32_BYTE( "gt97prm0.120", 0x00000, 0x80000, CRC(cdc4226f) SHA1(35ae8514f543f1cf45b303952f34a0a395733268) )
	ROM_LOAD32_BYTE( "gt97prm1.120", 0x00001, 0x80000, CRC(b36fc43f) SHA1(43e5acbb751b216e8da0249eb3e596d8e453b0dc) )
	ROM_LOAD32_BYTE( "gt97prm2.120", 0x00002, 0x80000, CRC(30b0d97e) SHA1(423808f0957cd259a33f5fad43222edbacb445ea) )
	ROM_LOAD32_BYTE( "gt97prm3.120", 0x00003, 0x80000, CRC(77281d3a) SHA1(f400c8be5dc1b91008c84ddbd6c945c4cb96b0bd) )

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "gt_nr.u88", 0x10000, 0x18000, CRC(2cee9e98) SHA1(02edac7abab2335c1cd824d1d9b26aa32238a2de) )
	ROM_CONTINUE(          0x08000, 0x08000 )

	ROM_REGION( 0x600000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "gt97_grm.0_0", 0x000000, 0x80000, CRC(81784aaf) SHA1(9544ed2087ca5f71c747e3b782513614937a51ed) )
	ROM_LOAD32_BYTE( "gt97_grm.0_1", 0x000001, 0x80000, CRC(345bda44) SHA1(b6d66c99bd68396e1e94ece76989a6190571ae14) )
	ROM_LOAD32_BYTE( "gt97_grm.0_2", 0x000002, 0x80000, CRC(b2beb40d) SHA1(2b955919e7e5f9093eb3a856f93c39684437b371) )
	ROM_LOAD32_BYTE( "gt97_grm.0_3", 0x000003, 0x80000, CRC(7cef32ff) SHA1(614c6aa26398d054a087dfe5e75cef78e6096f30) )
	ROM_LOAD32_BYTE( "gt97_grm.1_0", 0x200000, 0x80000, CRC(1cc4c309) SHA1(ef15f663ad1ea14c25f08fb6c1d5a58a38834ae6) )
	ROM_LOAD32_BYTE( "gt97_grm.1_1", 0x200001, 0x80000, CRC(512cea45) SHA1(03b229317c8f357fa2d2ead93bf519251edc6605) )
	ROM_LOAD32_BYTE( "gt97_grm.1_2", 0x200002, 0x80000, CRC(0599b505) SHA1(13078eae4051b3a4c1955cc246e58ef51ee2e53a) )
	ROM_LOAD32_BYTE( "gt97_grm.1_3", 0x200003, 0x80000, CRC(257eacc9) SHA1(2d03c38965930c4c1da4aa86583bb19c5f135100) )
	ROM_LOAD32_BYTE( "gt97_grm.2_0", 0x400000, 0x80000, CRC(95b6e7ee) SHA1(c27f69f69927e75ec8cba0df5c12e10418a53bbc) )
	ROM_LOAD32_BYTE( "gt97_grm.2_1", 0x400001, 0x80000, CRC(afd558b4) SHA1(0571bfc2b0131cdd62f92641dd1acf7b10ae09d8) )
	ROM_LOAD32_BYTE( "gt97_grm.2_2", 0x400002, 0x80000, CRC(5b7a8733) SHA1(e0a947ebabc8e6ab23b375afef49cd4be7d6d570) )
	ROM_LOAD32_BYTE( "gt97_grm.2_3", 0x400003, 0x80000, CRC(72e8ae60) SHA1(410da9970c50021f1724c8c51678691322eece92) )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "gt_srom0.nr", 0x000000, 0x100000, CRC(44983bd7) SHA1(a6ac966ec113b079434d7f871e4ce7266206d234) )
	ROM_LOAD16_BYTE( "gt_srom1.nr", 0x200000, 0x080000, CRC(1b3f18b6) SHA1(3b65de6a90c5ede183b5f8ca1875736bc1425772) )
ROM_END


ROM_START( gt97t243 ) /* Version 2.43 Tournament Edition (PCB P/N 1083 Rev 2) */
	ROM_REGION32_BE( CODE_SIZE, "user1", 0 )
	ROM_LOAD32_BYTE( "gt97prm0.243", 0x00000, 0x100000, CRC(b8de60f1) SHA1(06b1f8b9d0b878d5a19e6756957e2df19e013ad6) )
	ROM_LOAD32_BYTE( "gt97prm1.243", 0x00001, 0x100000, CRC(8152e5d3) SHA1(2a4f8acc6a4e33864c97d5974e2230b1cf3632ea) )
	ROM_LOAD32_BYTE( "gt97prm2.243", 0x00002, 0x100000, CRC(b80061be) SHA1(9a6a6281690b3bd2eabb081467bfda074639fa6a) )
	ROM_LOAD32_BYTE( "gt97prm3.243", 0x00003, 0x100000, CRC(d184968d) SHA1(eecd4e9d060e4b8feabca715d3d7f9738641cfcc) )

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "gt_nr.u88", 0x10000, 0x18000, CRC(2cee9e98) SHA1(02edac7abab2335c1cd824d1d9b26aa32238a2de) )
	ROM_CONTINUE(          0x08000, 0x08000 )

	ROM_REGION( 0x600000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "gt97_grm.0_0", 0x000000, 0x80000, CRC(81784aaf) SHA1(9544ed2087ca5f71c747e3b782513614937a51ed) )
	ROM_LOAD32_BYTE( "gt97_grm.0_1", 0x000001, 0x80000, CRC(345bda44) SHA1(b6d66c99bd68396e1e94ece76989a6190571ae14) )
	ROM_LOAD32_BYTE( "gt97_grm.0_2", 0x000002, 0x80000, CRC(b2beb40d) SHA1(2b955919e7e5f9093eb3a856f93c39684437b371) )
	ROM_LOAD32_BYTE( "gt97_grm.0_3", 0x000003, 0x80000, CRC(7cef32ff) SHA1(614c6aa26398d054a087dfe5e75cef78e6096f30) )
	ROM_LOAD32_BYTE( "gt97_grm.1_0", 0x200000, 0x80000, CRC(1cc4c309) SHA1(ef15f663ad1ea14c25f08fb6c1d5a58a38834ae6) )
	ROM_LOAD32_BYTE( "gt97_grm.1_1", 0x200001, 0x80000, CRC(512cea45) SHA1(03b229317c8f357fa2d2ead93bf519251edc6605) )
	ROM_LOAD32_BYTE( "gt97_grm.1_2", 0x200002, 0x80000, CRC(0599b505) SHA1(13078eae4051b3a4c1955cc246e58ef51ee2e53a) )
	ROM_LOAD32_BYTE( "gt97_grm.1_3", 0x200003, 0x80000, CRC(257eacc9) SHA1(2d03c38965930c4c1da4aa86583bb19c5f135100) )
	ROM_LOAD32_BYTE( "gt97_grm.2_0", 0x400000, 0x80000, CRC(95b6e7ee) SHA1(c27f69f69927e75ec8cba0df5c12e10418a53bbc) )
	ROM_LOAD32_BYTE( "gt97_grm.2_1", 0x400001, 0x80000, CRC(afd558b4) SHA1(0571bfc2b0131cdd62f92641dd1acf7b10ae09d8) )
	ROM_LOAD32_BYTE( "gt97_grm.2_2", 0x400002, 0x80000, CRC(5b7a8733) SHA1(e0a947ebabc8e6ab23b375afef49cd4be7d6d570) )
	ROM_LOAD32_BYTE( "gt97_grm.2_3", 0x400003, 0x80000, CRC(72e8ae60) SHA1(410da9970c50021f1724c8c51678691322eece92) )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "gt_srom0.nr", 0x000000, 0x100000, CRC(44983bd7) SHA1(a6ac966ec113b079434d7f871e4ce7266206d234) )
	ROM_LOAD16_BYTE( "gt_srom1.nr", 0x200000, 0x080000, CRC(1b3f18b6) SHA1(3b65de6a90c5ede183b5f8ca1875736bc1425772) )
ROM_END


ROM_START( gt97t240 ) /* Version 2.40 Tournament Edition (PCB P/N 1083 Rev 2) */
	ROM_REGION32_BE( CODE_SIZE, "user1", 0 )
	ROM_LOAD32_BYTE( "gt97prm0.240", 0x00000, 0x100000, CRC(88a386d0) SHA1(003dbf784125b1a442f85e18f8161695dcacc3a8) )
	ROM_LOAD32_BYTE( "gt97prm1.240", 0x00001, 0x100000, CRC(b0d751aa) SHA1(7e6ab9c2bb0bd4f50360655c59f48c44f6135f4f) )
	ROM_LOAD32_BYTE( "gt97prm2.240", 0x00002, 0x100000, CRC(451be534) SHA1(2f78cdba607c4b936b5cbdb520757d038d9aa7a3) )
	ROM_LOAD32_BYTE( "gt97prm3.240", 0x00003, 0x100000, CRC(70da8ca5) SHA1(baf56d04c5d75165fc9f3269650847e46bbbe2d3) )

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "gt_nr.u88", 0x10000, 0x18000, CRC(2cee9e98) SHA1(02edac7abab2335c1cd824d1d9b26aa32238a2de) )
	ROM_CONTINUE(          0x08000, 0x08000 )

	ROM_REGION( 0x600000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "gt97_grm.0_0", 0x000000, 0x80000, CRC(81784aaf) SHA1(9544ed2087ca5f71c747e3b782513614937a51ed) )
	ROM_LOAD32_BYTE( "gt97_grm.0_1", 0x000001, 0x80000, CRC(345bda44) SHA1(b6d66c99bd68396e1e94ece76989a6190571ae14) )
	ROM_LOAD32_BYTE( "gt97_grm.0_2", 0x000002, 0x80000, CRC(b2beb40d) SHA1(2b955919e7e5f9093eb3a856f93c39684437b371) )
	ROM_LOAD32_BYTE( "gt97_grm.0_3", 0x000003, 0x80000, CRC(7cef32ff) SHA1(614c6aa26398d054a087dfe5e75cef78e6096f30) )
	ROM_LOAD32_BYTE( "gt97_grm.1_0", 0x200000, 0x80000, CRC(1cc4c309) SHA1(ef15f663ad1ea14c25f08fb6c1d5a58a38834ae6) )
	ROM_LOAD32_BYTE( "gt97_grm.1_1", 0x200001, 0x80000, CRC(512cea45) SHA1(03b229317c8f357fa2d2ead93bf519251edc6605) )
	ROM_LOAD32_BYTE( "gt97_grm.1_2", 0x200002, 0x80000, CRC(0599b505) SHA1(13078eae4051b3a4c1955cc246e58ef51ee2e53a) )
	ROM_LOAD32_BYTE( "gt97_grm.1_3", 0x200003, 0x80000, CRC(257eacc9) SHA1(2d03c38965930c4c1da4aa86583bb19c5f135100) )
	ROM_LOAD32_BYTE( "gt97_grm.2_0", 0x400000, 0x80000, CRC(95b6e7ee) SHA1(c27f69f69927e75ec8cba0df5c12e10418a53bbc) )
	ROM_LOAD32_BYTE( "gt97_grm.2_1", 0x400001, 0x80000, CRC(afd558b4) SHA1(0571bfc2b0131cdd62f92641dd1acf7b10ae09d8) )
	ROM_LOAD32_BYTE( "gt97_grm.2_2", 0x400002, 0x80000, CRC(5b7a8733) SHA1(e0a947ebabc8e6ab23b375afef49cd4be7d6d570) )
	ROM_LOAD32_BYTE( "gt97_grm.2_3", 0x400003, 0x80000, CRC(72e8ae60) SHA1(410da9970c50021f1724c8c51678691322eece92) )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "gt_srom0.nr", 0x000000, 0x100000, CRC(44983bd7) SHA1(a6ac966ec113b079434d7f871e4ce7266206d234) )
	ROM_LOAD16_BYTE( "gt_srom1.nr", 0x200000, 0x080000, CRC(1b3f18b6) SHA1(3b65de6a90c5ede183b5f8ca1875736bc1425772) )
ROM_END


ROM_START( gt98 )	/* Version 1.10 */
	ROM_REGION32_BE( CODE_SIZE, "user1", 0 )
	ROM_LOAD32_BYTE( "gt98prm0.110", 0x00000, 0x80000, CRC(dd93ab2a) SHA1(b7eb6331f781422d6d46babcc24a85ae36b25914) )
	ROM_LOAD32_BYTE( "gt98prm1.110", 0x00001, 0x80000, CRC(6ea92960) SHA1(05d22ad6c6027afe7ebb3bc7c70f58d840ed3d4e) )
	ROM_LOAD32_BYTE( "gt98prm2.110", 0x00002, 0x80000, CRC(27a8a15f) SHA1(f1eb7b24f9cb77877ceaa033abfde124e159cb2b) )
	ROM_LOAD32_BYTE( "gt98prm3.110", 0x00003, 0x80000, CRC(d61f2bb2) SHA1(6eb9f779b78ef396eff6dbc25fd6dba128c77124) )

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "gt_nr.u88", 0x10000, 0x18000, CRC(2cee9e98) SHA1(02edac7abab2335c1cd824d1d9b26aa32238a2de) )
	ROM_CONTINUE(          0x08000, 0x08000 )

	ROM_REGION( 0x600000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "gt98_grm.0_0", 0x000000, 0x80000, CRC(2d79492b) SHA1(16d66d937c34ddf616f31cba0d285326a31cad85) )
	ROM_LOAD32_BYTE( "gt98_grm.0_1", 0x000001, 0x80000, CRC(79afda1a) SHA1(77a9883f14b58ceece9c76ce88bb900bc4accf25) )
	ROM_LOAD32_BYTE( "gt98_grm.0_2", 0x000002, 0x80000, CRC(8c381f56) SHA1(41a5b70f9e524a1cade031f864350ec75c08c956) )
	ROM_LOAD32_BYTE( "gt98_grm.0_3", 0x000003, 0x80000, CRC(46c35ba6) SHA1(a1976dd8710442cdb92c47f778acacba4380731b) )
	ROM_LOAD32_BYTE( "gt98_grm.1_0", 0x200000, 0x80000, CRC(b07bc634) SHA1(48a9aeafaf844374d129d209884ec3a23abe249f) )
	ROM_LOAD32_BYTE( "gt98_grm.1_1", 0x200001, 0x80000, CRC(b23d59a7) SHA1(be68da263691e297b266e81485f5f28a5a5ad2f2) )
	ROM_LOAD32_BYTE( "gt98_grm.1_2", 0x200002, 0x80000, CRC(9c113abc) SHA1(8cb23da237dce73bbd283662c6344876d1c352f3) )
	ROM_LOAD32_BYTE( "gt98_grm.1_3", 0x200003, 0x80000, CRC(231bbe58) SHA1(b662a2ffd881a22ec0503810dca8bd61a4994463) )
	ROM_LOAD32_BYTE( "gt98_grm.2_0", 0x400000, 0x80000, CRC(db5cec87) SHA1(831cebd0c90c118d007b737b2eb5fb374a86cf4b) )
	ROM_LOAD32_BYTE( "gt98_grm.2_1", 0x400001, 0x80000, CRC(c74fc7d3) SHA1(38581876d4557f79acbc2c639bd4188a49d3b7cc) )
	ROM_LOAD32_BYTE( "gt98_grm.2_2", 0x400002, 0x80000, CRC(1227609d) SHA1(5a586d2383c9090ff3847abd2c645354dacd400f) )
	ROM_LOAD32_BYTE( "gt98_grm.2_3", 0x400003, 0x80000, CRC(78745131) SHA1(c430be4cb650f1f6265406ca8fcad8df809282f5) )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "gt_srom0.nr", 0x000000, 0x100000, CRC(44983bd7) SHA1(a6ac966ec113b079434d7f871e4ce7266206d234) )
	ROM_LOAD16_BYTE( "gt_srom1.nr", 0x200000, 0x080000, CRC(1b3f18b6) SHA1(3b65de6a90c5ede183b5f8ca1875736bc1425772) )
ROM_END


ROM_START( gt98v100 )	/* Version 1.00 */
	ROM_REGION32_BE( CODE_SIZE, "user1", 0 )
	ROM_LOAD32_BYTE( "gt98prm0.100", 0x00000, 0x80000, CRC(f2dc0a6c) SHA1(59f7f8c7feb30c6416cb4ac2299f2c620d4c4e5f) )
	ROM_LOAD32_BYTE( "gt98prm1.100", 0x00001, 0x80000, CRC(b0ca22f3) SHA1(f849b42d449e07a12e0bc20b98693125506c7ed6) )
	ROM_LOAD32_BYTE( "gt98prm2.100", 0x00002, 0x80000, CRC(f940acdc) SHA1(3b5f5f299dbd7d4e0bef4aac8787f955f31754c2) )
	ROM_LOAD32_BYTE( "gt98prm3.100", 0x00003, 0x80000, CRC(22d7e8dc) SHA1(7d22963f7948029cf74994abe0be4a5181112c32) )

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "gt_nr.u88", 0x10000, 0x18000, CRC(2cee9e98) SHA1(02edac7abab2335c1cd824d1d9b26aa32238a2de) )
	ROM_CONTINUE(          0x08000, 0x08000 )

	ROM_REGION( 0x600000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "gt98_grm.0_0", 0x000000, 0x80000, CRC(2d79492b) SHA1(16d66d937c34ddf616f31cba0d285326a31cad85) )
	ROM_LOAD32_BYTE( "gt98_grm.0_1", 0x000001, 0x80000, CRC(79afda1a) SHA1(77a9883f14b58ceece9c76ce88bb900bc4accf25) )
	ROM_LOAD32_BYTE( "gt98_grm.0_2", 0x000002, 0x80000, CRC(8c381f56) SHA1(41a5b70f9e524a1cade031f864350ec75c08c956) )
	ROM_LOAD32_BYTE( "gt98_grm.0_3", 0x000003, 0x80000, CRC(46c35ba6) SHA1(a1976dd8710442cdb92c47f778acacba4380731b) )
	ROM_LOAD32_BYTE( "gt98_grm.1_0", 0x200000, 0x80000, CRC(b07bc634) SHA1(48a9aeafaf844374d129d209884ec3a23abe249f) )
	ROM_LOAD32_BYTE( "gt98_grm.1_1", 0x200001, 0x80000, CRC(b23d59a7) SHA1(be68da263691e297b266e81485f5f28a5a5ad2f2) )
	ROM_LOAD32_BYTE( "gt98_grm.1_2", 0x200002, 0x80000, CRC(9c113abc) SHA1(8cb23da237dce73bbd283662c6344876d1c352f3) )
	ROM_LOAD32_BYTE( "gt98_grm.1_3", 0x200003, 0x80000, CRC(231bbe58) SHA1(b662a2ffd881a22ec0503810dca8bd61a4994463) )
	ROM_LOAD32_BYTE( "gt98_grm.2_0", 0x400000, 0x80000, CRC(db5cec87) SHA1(831cebd0c90c118d007b737b2eb5fb374a86cf4b) )
	ROM_LOAD32_BYTE( "gt98_grm.2_1", 0x400001, 0x80000, CRC(c74fc7d3) SHA1(38581876d4557f79acbc2c639bd4188a49d3b7cc) )
	ROM_LOAD32_BYTE( "gt98_grm.2_2", 0x400002, 0x80000, CRC(1227609d) SHA1(5a586d2383c9090ff3847abd2c645354dacd400f) )
	ROM_LOAD32_BYTE( "gt98_grm.2_3", 0x400003, 0x80000, CRC(78745131) SHA1(c430be4cb650f1f6265406ca8fcad8df809282f5) )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "gt_srom0.nr", 0x000000, 0x100000, CRC(44983bd7) SHA1(a6ac966ec113b079434d7f871e4ce7266206d234) )
	ROM_LOAD16_BYTE( "gt_srom1.nr", 0x200000, 0x080000, CRC(1b3f18b6) SHA1(3b65de6a90c5ede183b5f8ca1875736bc1425772) )
ROM_END


ROM_START( gt98s100 ) /* Version 1.00S for the 3 tier type PCB with short ROM board P/N 1088 Rev 0 */
	ROM_REGION32_BE( CODE_SIZE, "user1", 0 )
	ROM_LOAD32_BYTE( "gt98prm0.10s", 0x00000, 0x80000, CRC(962ff444) SHA1(f9abefaee82f811ef1d3df45782edd5bcb1da23a) )
	ROM_LOAD32_BYTE( "gt98prm1.10s", 0x00001, 0x80000, CRC(be0ac375) SHA1(40b99004a7698866eeb0e2defda52e61be455f36) )
	ROM_LOAD32_BYTE( "gt98prm2.10s", 0x00002, 0x80000, CRC(304e881c) SHA1(582ff2c1c03853eec9830663d5263b499e68f285) )
	ROM_LOAD32_BYTE( "gt98prm3.10s", 0x00003, 0x80000, CRC(ac04ea81) SHA1(f8a7c896cb0af747bab49b3438f52aaa1b0dfc73) )

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "gt322_nr.u23", 0x10000, 0x18000, CRC(04effd73) SHA1(4277031655f8de851eba0e4134ba619a12f5dd4a) )
	ROM_CONTINUE(             0x08000, 0x08000 )

	ROM_REGION( 0x600000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "gt98_grm.0_0", 0x000000, 0x80000, CRC(2d79492b) SHA1(16d66d937c34ddf616f31cba0d285326a31cad85) )
	ROM_LOAD32_BYTE( "gt98_grm.0_1", 0x000001, 0x80000, CRC(79afda1a) SHA1(77a9883f14b58ceece9c76ce88bb900bc4accf25) )
	ROM_LOAD32_BYTE( "gt98_grm.0_2", 0x000002, 0x80000, CRC(8c381f56) SHA1(41a5b70f9e524a1cade031f864350ec75c08c956) )
	ROM_LOAD32_BYTE( "gt98_grm.0_3", 0x000003, 0x80000, CRC(46c35ba6) SHA1(a1976dd8710442cdb92c47f778acacba4380731b) )
	ROM_LOAD32_BYTE( "gt98_grm.1_0", 0x200000, 0x80000, CRC(b07bc634) SHA1(48a9aeafaf844374d129d209884ec3a23abe249f) )
	ROM_LOAD32_BYTE( "gt98_grm.1_1", 0x200001, 0x80000, CRC(b23d59a7) SHA1(be68da263691e297b266e81485f5f28a5a5ad2f2) )
	ROM_LOAD32_BYTE( "gt98_grm.1_2", 0x200002, 0x80000, CRC(9c113abc) SHA1(8cb23da237dce73bbd283662c6344876d1c352f3) )
	ROM_LOAD32_BYTE( "gt98_grm.1_3", 0x200003, 0x80000, CRC(231bbe58) SHA1(b662a2ffd881a22ec0503810dca8bd61a4994463) )
	ROM_LOAD32_BYTE( "gt98_grm.2_0", 0x400000, 0x80000, CRC(db5cec87) SHA1(831cebd0c90c118d007b737b2eb5fb374a86cf4b) )
	ROM_LOAD32_BYTE( "gt98_grm.2_1", 0x400001, 0x80000, CRC(c74fc7d3) SHA1(38581876d4557f79acbc2c639bd4188a49d3b7cc) )
	ROM_LOAD32_BYTE( "gt98_grm.2_2", 0x400002, 0x80000, CRC(1227609d) SHA1(5a586d2383c9090ff3847abd2c645354dacd400f) )
	ROM_LOAD32_BYTE( "gt98_grm.2_3", 0x400003, 0x80000, CRC(78745131) SHA1(c430be4cb650f1f6265406ca8fcad8df809282f5) )

	ROM_REGION16_BE( 0x400000, "ensoniq.2", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "gt_srom0.nr", 0x000000, 0x100000, CRC(44983bd7) SHA1(a6ac966ec113b079434d7f871e4ce7266206d234) ) /* actually labeled "GTG3 SROM1 NR**" */
	ROM_LOAD16_BYTE( "gt_srom1.nr", 0x200000, 0x080000, CRC(1b3f18b6) SHA1(3b65de6a90c5ede183b5f8ca1875736bc1425772) ) /* actually labeled "GTG3 SROM2 NR*"  */
ROM_END


ROM_START( gt98t303 )	/* Version 3.03 Tournament Edition (PCB P/N 1083 Rev 2) */
	ROM_REGION32_BE( CODE_SIZE, "user1", 0 )
	ROM_LOAD32_BYTE( "gt98prm0.303", 0x00000, 0x100000, CRC(e3879c30) SHA1(fa9dd2df8969a98a3c87c6a96594e1f49ca7ec91) )
	ROM_LOAD32_BYTE( "gt98prm1.303", 0x00001, 0x100000, CRC(6a42ab1e) SHA1(9d8c5a48f0b91dcc8898913eec3d09ddded0f43d) )
	ROM_LOAD32_BYTE( "gt98prm2.303", 0x00002, 0x100000, CRC(a695c1bc) SHA1(e10ce3a97c28ba439c06b2cbd3ebe0cb456687a9) )
	ROM_LOAD32_BYTE( "gt98prm3.303", 0x00003, 0x100000, CRC(bd7f5c7a) SHA1(969833ff8c5fa7277abd2d03940fcd02853bb12e) )

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "gt_nr.u88", 0x10000, 0x18000, CRC(2cee9e98) SHA1(02edac7abab2335c1cd824d1d9b26aa32238a2de) )
	ROM_CONTINUE(          0x08000, 0x08000 )

	ROM_REGION( 0x600000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "gt98_grm.0_0", 0x000000, 0x80000, CRC(2d79492b) SHA1(16d66d937c34ddf616f31cba0d285326a31cad85) )
	ROM_LOAD32_BYTE( "gt98_grm.0_1", 0x000001, 0x80000, CRC(79afda1a) SHA1(77a9883f14b58ceece9c76ce88bb900bc4accf25) )
	ROM_LOAD32_BYTE( "gt98_grm.0_2", 0x000002, 0x80000, CRC(8c381f56) SHA1(41a5b70f9e524a1cade031f864350ec75c08c956) )
	ROM_LOAD32_BYTE( "gt98_grm.0_3", 0x000003, 0x80000, CRC(46c35ba6) SHA1(a1976dd8710442cdb92c47f778acacba4380731b) )
	ROM_LOAD32_BYTE( "gt98_grm.1_0", 0x200000, 0x80000, CRC(b07bc634) SHA1(48a9aeafaf844374d129d209884ec3a23abe249f) )
	ROM_LOAD32_BYTE( "gt98_grm.1_1", 0x200001, 0x80000, CRC(b23d59a7) SHA1(be68da263691e297b266e81485f5f28a5a5ad2f2) )
	ROM_LOAD32_BYTE( "gt98_grm.1_2", 0x200002, 0x80000, CRC(9c113abc) SHA1(8cb23da237dce73bbd283662c6344876d1c352f3) )
	ROM_LOAD32_BYTE( "gt98_grm.1_3", 0x200003, 0x80000, CRC(231bbe58) SHA1(b662a2ffd881a22ec0503810dca8bd61a4994463) )
	ROM_LOAD32_BYTE( "gt98_grm.2_0", 0x400000, 0x80000, CRC(db5cec87) SHA1(831cebd0c90c118d007b737b2eb5fb374a86cf4b) )
	ROM_LOAD32_BYTE( "gt98_grm.2_1", 0x400001, 0x80000, CRC(c74fc7d3) SHA1(38581876d4557f79acbc2c639bd4188a49d3b7cc) )
	ROM_LOAD32_BYTE( "gt98_grm.2_2", 0x400002, 0x80000, CRC(1227609d) SHA1(5a586d2383c9090ff3847abd2c645354dacd400f) )
	ROM_LOAD32_BYTE( "gt98_grm.2_3", 0x400003, 0x80000, CRC(78745131) SHA1(c430be4cb650f1f6265406ca8fcad8df809282f5) )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "gt_srom0.nr", 0x000000, 0x100000, CRC(44983bd7) SHA1(a6ac966ec113b079434d7f871e4ce7266206d234) )
	ROM_LOAD16_BYTE( "gt_srom1.nr", 0x200000, 0x080000, CRC(1b3f18b6) SHA1(3b65de6a90c5ede183b5f8ca1875736bc1425772) )
ROM_END


ROM_START( gt99 )	/* Version 1.00 */
	ROM_REGION32_BE( CODE_SIZE, "user1", 0 )
	ROM_LOAD32_BYTE( "gt99prm0.100", 0x00000, 0x80000, CRC(1ca05267) SHA1(431788db68122df5b6c0642ffc84954fb3043295) )
	ROM_LOAD32_BYTE( "gt99prm1.100", 0x00001, 0x80000, CRC(4fb757fa) SHA1(9efa6f933b20e5a6de9a5da3c0197cf29c8f1df2) )
	ROM_LOAD32_BYTE( "gt99prm2.100", 0x00002, 0x80000, CRC(3eb2b13a) SHA1(6b6b79c7f07cc345f392d12625548c8fae6a1d42) )
	ROM_LOAD32_BYTE( "gt99prm3.100", 0x00003, 0x80000, CRC(03454e7d) SHA1(be885433830976b6c684e944f1d3a96d261b27f2) )

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "gt_nr.u88", 0x10000, 0x18000, CRC(2cee9e98) SHA1(02edac7abab2335c1cd824d1d9b26aa32238a2de) )
	ROM_CONTINUE(          0x08000, 0x08000 )

	ROM_REGION( 0x600000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "gt99_grm.0_0", 0x000000, 0x80000, CRC(c22b50f9) SHA1(9e8acfce6cc30adc150b602d026c00fa1fb7747f) )
	ROM_LOAD32_BYTE( "gt99_grm.0_1", 0x000001, 0x80000, CRC(d6d6be57) SHA1(fda20185e842dd4aa1a1601f95e5cc787644f4c3) )
	ROM_LOAD32_BYTE( "gt99_grm.0_2", 0x000002, 0x80000, CRC(005d4791) SHA1(b03d5835465ccc4fe73f4adb1342ef2b38aad90c) )
	ROM_LOAD32_BYTE( "gt99_grm.0_3", 0x000003, 0x80000, CRC(0c998eb7) SHA1(2950a56192dd794e3f34459c52edf7ea484d6901) )
	ROM_LOAD32_BYTE( "gt99_grm.1_0", 0x200000, 0x80000, CRC(8b79d6e2) SHA1(b4aeac78a470bd8b9f557ded39775cb56f525cce) )
	ROM_LOAD32_BYTE( "gt99_grm.1_1", 0x200001, 0x80000, CRC(84ef1803) SHA1(c4e2f0451a35874603cc767b4dbb566b2507ae39) )
	ROM_LOAD32_BYTE( "gt99_grm.1_2", 0x200002, 0x80000, CRC(d73d8afc) SHA1(2fced1ee7dd11c6db07750f617356c6608ac0291) )
	ROM_LOAD32_BYTE( "gt99_grm.1_3", 0x200003, 0x80000, CRC(59f48688) SHA1(37b2c84e487f4f3a9145bef34c573a3716b4a6a7) )

	/* GT99, GT2K & GT Classic all share the above listed 8 graphics roms and may be labeled GT99, GT2K or GTClassic */

	ROM_LOAD32_BYTE( "gt99_grm.2_0", 0x400000, 0x80000, CRC(693d9d68) SHA1(c8f0a5ca72b239aed8150a79f330b109bd6c3d95) )
	ROM_LOAD32_BYTE( "gt99_grm.2_1", 0x400001, 0x80000, CRC(2c0b8b8c) SHA1(f36c40b29ad9a4849f12eaa79c6b26aa85ca6ee9) )
	ROM_LOAD32_BYTE( "gt99_grm.2_2", 0x400002, 0x80000, CRC(ba1b5961) SHA1(d9078653edafb1aaabf2fe040b77a194b3a34863) )
	ROM_LOAD32_BYTE( "gt99_grm.2_3", 0x400003, 0x80000, CRC(cfccd5c2) SHA1(6d87675e9cdaebc801a6f52688e0a142f578d36d) )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "gt_srom0.nr", 0x000000, 0x100000, CRC(44983bd7) SHA1(a6ac966ec113b079434d7f871e4ce7266206d234) )
	ROM_LOAD16_BYTE( "gt_srom1.nr", 0x200000, 0x080000, CRC(1b3f18b6) SHA1(3b65de6a90c5ede183b5f8ca1875736bc1425772) )
ROM_END


ROM_START( gt99s100 )	/* Version 1.00S for the 3 tier type PCB with short ROM board P/N 1088 Rev 0 */
	ROM_REGION32_BE( CODE_SIZE, "user1", 0 )
	ROM_LOAD32_BYTE( "gt99prm0.10s", 0x00000, 0x80000, CRC(58e7c4e1) SHA1(2d5e2d841ffb626338f4dd9ffb558d05ed476078) )
	ROM_LOAD32_BYTE( "gt99prm1.10s", 0x00001, 0x80000, CRC(09f8bdf4) SHA1(b933b48e19ca31ead93027ea328c2d9e581cbd31) )
	ROM_LOAD32_BYTE( "gt99prm2.10s", 0x00002, 0x80000, CRC(fd084b68) SHA1(bd39ea7d201892f5583db21685010c75354ac3b9) )
	ROM_LOAD32_BYTE( "gt99prm3.10s", 0x00003, 0x80000, CRC(3ff88ff7) SHA1(ca59f5888932be78434ae9a1b5cb06b01156c8ba) )

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "gt322_nr.u23", 0x10000, 0x18000, CRC(04effd73) SHA1(4277031655f8de851eba0e4134ba619a12f5dd4a) )
	ROM_CONTINUE(             0x08000, 0x08000 )

	ROM_REGION( 0x600000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "gt99_grm.0_0", 0x000000, 0x80000, CRC(c22b50f9) SHA1(9e8acfce6cc30adc150b602d026c00fa1fb7747f) )
	ROM_LOAD32_BYTE( "gt99_grm.0_1", 0x000001, 0x80000, CRC(d6d6be57) SHA1(fda20185e842dd4aa1a1601f95e5cc787644f4c3) )
	ROM_LOAD32_BYTE( "gt99_grm.0_2", 0x000002, 0x80000, CRC(005d4791) SHA1(b03d5835465ccc4fe73f4adb1342ef2b38aad90c) )
	ROM_LOAD32_BYTE( "gt99_grm.0_3", 0x000003, 0x80000, CRC(0c998eb7) SHA1(2950a56192dd794e3f34459c52edf7ea484d6901) )
	ROM_LOAD32_BYTE( "gt99_grm.1_0", 0x200000, 0x80000, CRC(8b79d6e2) SHA1(b4aeac78a470bd8b9f557ded39775cb56f525cce) )
	ROM_LOAD32_BYTE( "gt99_grm.1_1", 0x200001, 0x80000, CRC(84ef1803) SHA1(c4e2f0451a35874603cc767b4dbb566b2507ae39) )
	ROM_LOAD32_BYTE( "gt99_grm.1_2", 0x200002, 0x80000, CRC(d73d8afc) SHA1(2fced1ee7dd11c6db07750f617356c6608ac0291) )
	ROM_LOAD32_BYTE( "gt99_grm.1_3", 0x200003, 0x80000, CRC(59f48688) SHA1(37b2c84e487f4f3a9145bef34c573a3716b4a6a7) )

	/* GT99, GT2K & GT Classic all share the above listed 8 graphics roms and may be labeled GT99, GT2K or GTClassic */

	ROM_LOAD32_BYTE( "gt99_grm.2_0", 0x400000, 0x80000, CRC(693d9d68) SHA1(c8f0a5ca72b239aed8150a79f330b109bd6c3d95) )
	ROM_LOAD32_BYTE( "gt99_grm.2_1", 0x400001, 0x80000, CRC(2c0b8b8c) SHA1(f36c40b29ad9a4849f12eaa79c6b26aa85ca6ee9) )
	ROM_LOAD32_BYTE( "gt99_grm.2_2", 0x400002, 0x80000, CRC(ba1b5961) SHA1(d9078653edafb1aaabf2fe040b77a194b3a34863) )
	ROM_LOAD32_BYTE( "gt99_grm.2_3", 0x400003, 0x80000, CRC(cfccd5c2) SHA1(6d87675e9cdaebc801a6f52688e0a142f578d36d) )

	ROM_REGION16_BE( 0x400000, "ensoniq.2", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "gt_srom0.nr", 0x000000, 0x100000, CRC(44983bd7) SHA1(a6ac966ec113b079434d7f871e4ce7266206d234) ) /* actually labeled "GTG3 SROM1 NR**" */
	ROM_LOAD16_BYTE( "gt_srom1.nr", 0x200000, 0x080000, CRC(1b3f18b6) SHA1(3b65de6a90c5ede183b5f8ca1875736bc1425772) ) /* actually labeled "GTG3 SROM2 NR*"  */
ROM_END


ROM_START( gt99t400 )	/* Version 4.00 Tournament Edition (PCB P/N 1083 Rev 2) */
	ROM_REGION32_BE( CODE_SIZE, "user1", 0 )
	ROM_LOAD32_BYTE( "gt99prm0.400", 0x00000, 0x100000, CRC(bc58e0a2) SHA1(9e207acb860d532238f6105fd5b0d283056e016f) )
	ROM_LOAD32_BYTE( "gt99prm1.400", 0x00001, 0x100000, CRC(89d8cc6b) SHA1(ff2a5452c1c3a14c22abe380cb1ce263c23cc071) )
	ROM_LOAD32_BYTE( "gt99prm2.400", 0x00002, 0x100000, CRC(891e26c1) SHA1(11eeb60160924fbf58e409465541ef1cb15ff933) )
	ROM_LOAD32_BYTE( "gt99prm3.400", 0x00003, 0x100000, CRC(127f7aa7) SHA1(d83b4366b526b5f745d0556e5ec39faa02cb8570) )

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "gt_nr.u88", 0x10000, 0x18000, CRC(2cee9e98) SHA1(02edac7abab2335c1cd824d1d9b26aa32238a2de) )
	ROM_CONTINUE(          0x08000, 0x08000 )

	ROM_REGION( 0x600000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "gt99_grm.0_0", 0x000000, 0x80000, CRC(c22b50f9) SHA1(9e8acfce6cc30adc150b602d026c00fa1fb7747f) )
	ROM_LOAD32_BYTE( "gt99_grm.0_1", 0x000001, 0x80000, CRC(d6d6be57) SHA1(fda20185e842dd4aa1a1601f95e5cc787644f4c3) )
	ROM_LOAD32_BYTE( "gt99_grm.0_2", 0x000002, 0x80000, CRC(005d4791) SHA1(b03d5835465ccc4fe73f4adb1342ef2b38aad90c) )
	ROM_LOAD32_BYTE( "gt99_grm.0_3", 0x000003, 0x80000, CRC(0c998eb7) SHA1(2950a56192dd794e3f34459c52edf7ea484d6901) )
	ROM_LOAD32_BYTE( "gt99_grm.1_0", 0x200000, 0x80000, CRC(8b79d6e2) SHA1(b4aeac78a470bd8b9f557ded39775cb56f525cce) )
	ROM_LOAD32_BYTE( "gt99_grm.1_1", 0x200001, 0x80000, CRC(84ef1803) SHA1(c4e2f0451a35874603cc767b4dbb566b2507ae39) )
	ROM_LOAD32_BYTE( "gt99_grm.1_2", 0x200002, 0x80000, CRC(d73d8afc) SHA1(2fced1ee7dd11c6db07750f617356c6608ac0291) )
	ROM_LOAD32_BYTE( "gt99_grm.1_3", 0x200003, 0x80000, CRC(59f48688) SHA1(37b2c84e487f4f3a9145bef34c573a3716b4a6a7) )

	/* GT99, GT2K & GT Classic all share the above listed 8 graphics roms and may be labeled GT99, GT2K or GTClassic */

	ROM_LOAD32_BYTE( "gt99_grm.2_0", 0x400000, 0x80000, CRC(693d9d68) SHA1(c8f0a5ca72b239aed8150a79f330b109bd6c3d95) )
	ROM_LOAD32_BYTE( "gt99_grm.2_1", 0x400001, 0x80000, CRC(2c0b8b8c) SHA1(f36c40b29ad9a4849f12eaa79c6b26aa85ca6ee9) )
	ROM_LOAD32_BYTE( "gt99_grm.2_2", 0x400002, 0x80000, CRC(ba1b5961) SHA1(d9078653edafb1aaabf2fe040b77a194b3a34863) )
	ROM_LOAD32_BYTE( "gt99_grm.2_3", 0x400003, 0x80000, CRC(cfccd5c2) SHA1(6d87675e9cdaebc801a6f52688e0a142f578d36d) )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "gt_srom0.nr", 0x000000, 0x100000, CRC(44983bd7) SHA1(a6ac966ec113b079434d7f871e4ce7266206d234) )
	ROM_LOAD16_BYTE( "gt_srom1.nr", 0x200000, 0x080000, CRC(1b3f18b6) SHA1(3b65de6a90c5ede183b5f8ca1875736bc1425772) )
ROM_END


ROM_START( gtroyal )	/* Version 4.02T EDM (Tournament Edition, PCB P/N 1083 Rev 2) */
	ROM_REGION32_BE( CODE_SIZE, "user1", 0 )
	ROM_LOAD32_BYTE( "gtr_prm0.402", 0x00000, 0x100000, CRC(ae499ea3) SHA1(0de651900fd92b2de3fcbc092c1292d546f12819) )
	ROM_LOAD32_BYTE( "gtr_prm1.402", 0x00001, 0x100000, CRC(87ee04b5) SHA1(2c4c4a80073bfd28066bef371fbd1008149cc56c) )
	ROM_LOAD32_BYTE( "gtr_prm2.402", 0x00002, 0x100000, CRC(a925d392) SHA1(07e398279a0abd058a4bda700a7f3fe90737cb04) )
	ROM_LOAD32_BYTE( "gtr_prm3.402", 0x00003, 0x100000, CRC(1c442664) SHA1(abf8043719f0bd855df17c22b2b7fb13ae160b4d) )

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "gt_nr.u88", 0x10000, 0x18000, CRC(2cee9e98) SHA1(02edac7abab2335c1cd824d1d9b26aa32238a2de) )
	ROM_CONTINUE(          0x08000, 0x08000 )

	ROM_REGION( 0x600000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "gt99_grm.0_0", 0x000000, 0x80000, CRC(c22b50f9) SHA1(9e8acfce6cc30adc150b602d026c00fa1fb7747f) )
	ROM_LOAD32_BYTE( "gt99_grm.0_1", 0x000001, 0x80000, CRC(d6d6be57) SHA1(fda20185e842dd4aa1a1601f95e5cc787644f4c3) )
	ROM_LOAD32_BYTE( "gt99_grm.0_2", 0x000002, 0x80000, CRC(005d4791) SHA1(b03d5835465ccc4fe73f4adb1342ef2b38aad90c) )
	ROM_LOAD32_BYTE( "gt99_grm.0_3", 0x000003, 0x80000, CRC(0c998eb7) SHA1(2950a56192dd794e3f34459c52edf7ea484d6901) )
	ROM_LOAD32_BYTE( "gt99_grm.1_0", 0x200000, 0x80000, CRC(8b79d6e2) SHA1(b4aeac78a470bd8b9f557ded39775cb56f525cce) )
	ROM_LOAD32_BYTE( "gt99_grm.1_1", 0x200001, 0x80000, CRC(84ef1803) SHA1(c4e2f0451a35874603cc767b4dbb566b2507ae39) )
	ROM_LOAD32_BYTE( "gt99_grm.1_2", 0x200002, 0x80000, CRC(d73d8afc) SHA1(2fced1ee7dd11c6db07750f617356c6608ac0291) )
	ROM_LOAD32_BYTE( "gt99_grm.1_3", 0x200003, 0x80000, CRC(59f48688) SHA1(37b2c84e487f4f3a9145bef34c573a3716b4a6a7) )

	/* GT Royal uses the same 12 graphics roms as a standard US GT99 version */

	ROM_LOAD32_BYTE( "gt99_grm.2_0", 0x400000, 0x80000, CRC(693d9d68) SHA1(c8f0a5ca72b239aed8150a79f330b109bd6c3d95) )
	ROM_LOAD32_BYTE( "gt99_grm.2_1", 0x400001, 0x80000, CRC(2c0b8b8c) SHA1(f36c40b29ad9a4849f12eaa79c6b26aa85ca6ee9) )
	ROM_LOAD32_BYTE( "gt99_grm.2_2", 0x400002, 0x80000, CRC(ba1b5961) SHA1(d9078653edafb1aaabf2fe040b77a194b3a34863) )
	ROM_LOAD32_BYTE( "gt99_grm.2_3", 0x400003, 0x80000, CRC(cfccd5c2) SHA1(6d87675e9cdaebc801a6f52688e0a142f578d36d) )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "gt_srom0.nr", 0x000000, 0x100000, CRC(44983bd7) SHA1(a6ac966ec113b079434d7f871e4ce7266206d234) )
	ROM_LOAD16_BYTE( "gt_srom1.nr", 0x200000, 0x080000, CRC(1b3f18b6) SHA1(3b65de6a90c5ede183b5f8ca1875736bc1425772) )
ROM_END


ROM_START( gt2k ) /* Version 1.00 */
	ROM_REGION32_BE( CODE_SIZE, "user1", 0 )
	ROM_LOAD32_BYTE( "gt2kprm0.100", 0x00000, 0x80000, CRC(b83d7b67) SHA1(9e3c4f5d09ae63d75f4c9499b0a09acea7b022b2) )
	ROM_LOAD32_BYTE( "gt2kprm1.100", 0x00001, 0x80000, CRC(89bd952d) SHA1(8b49610b9947dbc4cb3ab28f6aed31d8b848a2bf) )
	ROM_LOAD32_BYTE( "gt2kprm2.100", 0x00002, 0x80000, CRC(b603d283) SHA1(dc02b4969f96a089766b07eb45a2eb6be6ae0aad) )
	ROM_LOAD32_BYTE( "gt2kprm3.100", 0x00003, 0x80000, CRC(85ba9e2d) SHA1(f026d6dcf91848a40bcc9f9ba5e9262de33c30d1) )

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "gt_nr.u88", 0x10000, 0x18000, CRC(2cee9e98) SHA1(02edac7abab2335c1cd824d1d9b26aa32238a2de) )
	ROM_CONTINUE(          0x08000, 0x08000 )

	ROM_REGION( 0x600000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "gt2k_grm.0_0", 0x000000, 0x80000, CRC(c22b50f9) SHA1(9e8acfce6cc30adc150b602d026c00fa1fb7747f) )
	ROM_LOAD32_BYTE( "gt2k_grm.0_1", 0x000001, 0x80000, CRC(d6d6be57) SHA1(fda20185e842dd4aa1a1601f95e5cc787644f4c3) )
	ROM_LOAD32_BYTE( "gt2k_grm.0_2", 0x000002, 0x80000, CRC(005d4791) SHA1(b03d5835465ccc4fe73f4adb1342ef2b38aad90c) )
	ROM_LOAD32_BYTE( "gt2k_grm.0_3", 0x000003, 0x80000, CRC(0c998eb7) SHA1(2950a56192dd794e3f34459c52edf7ea484d6901) )
	ROM_LOAD32_BYTE( "gt2k_grm.1_0", 0x200000, 0x80000, CRC(8b79d6e2) SHA1(b4aeac78a470bd8b9f557ded39775cb56f525cce) )
	ROM_LOAD32_BYTE( "gt2k_grm.1_1", 0x200001, 0x80000, CRC(84ef1803) SHA1(c4e2f0451a35874603cc767b4dbb566b2507ae39) )
	ROM_LOAD32_BYTE( "gt2k_grm.1_2", 0x200002, 0x80000, CRC(d73d8afc) SHA1(2fced1ee7dd11c6db07750f617356c6608ac0291) )
	ROM_LOAD32_BYTE( "gt2k_grm.1_3", 0x200003, 0x80000, CRC(59f48688) SHA1(37b2c84e487f4f3a9145bef34c573a3716b4a6a7) )

	/* GT99, GT2K & GT Classic all share the above listed 8 graphics roms and may be labeled GT99, GT2K or GTClassic */

	ROM_LOAD32_BYTE( "gt2k_grm.2_0", 0x400000, 0x80000, CRC(cc11b93f) SHA1(f281448c8fa23595dd2664cc8c168565b60d4fc1) )
	ROM_LOAD32_BYTE( "gt2k_grm.2_1", 0x400001, 0x80000, CRC(1c3a0126) SHA1(3f9de1239dd64b9f50220842ffcf3e0f8928d2dc) )
	ROM_LOAD32_BYTE( "gt2k_grm.2_2", 0x400002, 0x80000, CRC(97814df5) SHA1(fcbb9ca08cbcef20231e602c99cd14e7905b2110) )
	ROM_LOAD32_BYTE( "gt2k_grm.2_3", 0x400003, 0x80000, CRC(f0f7373f) SHA1(6d71f338992598feffd9b4ac26bd0cf2f9edb53e) )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "gt_srom0.nr", 0x000000, 0x100000, CRC(44983bd7) SHA1(a6ac966ec113b079434d7f871e4ce7266206d234) )
	ROM_LOAD16_BYTE( "gt_srom1.nr", 0x200000, 0x080000, CRC(1b3f18b6) SHA1(3b65de6a90c5ede183b5f8ca1875736bc1425772) )
ROM_END


ROM_START( gt2kp100 ) /* Version 1.00 Infinite Loop Protection */
	ROM_REGION32_BE( CODE_SIZE, "user1", 0 )
	ROM_LOAD32_BYTE( "gt2kprm0.10p", 0x00000, 0x80000, CRC(16e8502d) SHA1(8bdf90793ae6d38dc0638c74a1c804c395ac5868) )
	ROM_LOAD32_BYTE( "gt2kprm1.10p", 0x00001, 0x80000, CRC(bf47cd95) SHA1(f63ac0dbe2668dfc2ba675862b0b0c754f89a9a8) )
	ROM_LOAD32_BYTE( "gt2kprm2.10p", 0x00002, 0x80000, CRC(204ddf15) SHA1(395659f4852021ef19967af5f3966c14cef6f327) )
	ROM_LOAD32_BYTE( "gt2kprm3.10p", 0x00003, 0x80000, CRC(45b9dd56) SHA1(819f5aa04cacebfb7f000e35df2b4a5ec4d8895e) )

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "gt_nr.u88", 0x10000, 0x18000, CRC(2cee9e98) SHA1(02edac7abab2335c1cd824d1d9b26aa32238a2de) )
	ROM_CONTINUE(          0x08000, 0x08000 )

	ROM_REGION( 0x600000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "gt2k_grm.0_0", 0x000000, 0x80000, CRC(c22b50f9) SHA1(9e8acfce6cc30adc150b602d026c00fa1fb7747f) )
	ROM_LOAD32_BYTE( "gt2k_grm.0_1", 0x000001, 0x80000, CRC(d6d6be57) SHA1(fda20185e842dd4aa1a1601f95e5cc787644f4c3) )
	ROM_LOAD32_BYTE( "gt2k_grm.0_2", 0x000002, 0x80000, CRC(005d4791) SHA1(b03d5835465ccc4fe73f4adb1342ef2b38aad90c) )
	ROM_LOAD32_BYTE( "gt2k_grm.0_3", 0x000003, 0x80000, CRC(0c998eb7) SHA1(2950a56192dd794e3f34459c52edf7ea484d6901) )
	ROM_LOAD32_BYTE( "gt2k_grm.1_0", 0x200000, 0x80000, CRC(8b79d6e2) SHA1(b4aeac78a470bd8b9f557ded39775cb56f525cce) )
	ROM_LOAD32_BYTE( "gt2k_grm.1_1", 0x200001, 0x80000, CRC(84ef1803) SHA1(c4e2f0451a35874603cc767b4dbb566b2507ae39) )
	ROM_LOAD32_BYTE( "gt2k_grm.1_2", 0x200002, 0x80000, CRC(d73d8afc) SHA1(2fced1ee7dd11c6db07750f617356c6608ac0291) )
	ROM_LOAD32_BYTE( "gt2k_grm.1_3", 0x200003, 0x80000, CRC(59f48688) SHA1(37b2c84e487f4f3a9145bef34c573a3716b4a6a7) )

	/* GT99, GT2K & GT Classic all share the above listed 8 graphics roms and may be labeled GT99, GT2K or GTClassic */

	ROM_LOAD32_BYTE( "gt2k_grm.2_0", 0x400000, 0x80000, CRC(cc11b93f) SHA1(f281448c8fa23595dd2664cc8c168565b60d4fc1) )
	ROM_LOAD32_BYTE( "gt2k_grm.2_1", 0x400001, 0x80000, CRC(1c3a0126) SHA1(3f9de1239dd64b9f50220842ffcf3e0f8928d2dc) )
	ROM_LOAD32_BYTE( "gt2k_grm.2_2", 0x400002, 0x80000, CRC(97814df5) SHA1(fcbb9ca08cbcef20231e602c99cd14e7905b2110) )
	ROM_LOAD32_BYTE( "gt2k_grm.2_3", 0x400003, 0x80000, CRC(f0f7373f) SHA1(6d71f338992598feffd9b4ac26bd0cf2f9edb53e) )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "gt_srom0.nr", 0x000000, 0x100000, CRC(44983bd7) SHA1(a6ac966ec113b079434d7f871e4ce7266206d234) )
	ROM_LOAD16_BYTE( "gt_srom1.nr", 0x200000, 0x080000, CRC(1b3f18b6) SHA1(3b65de6a90c5ede183b5f8ca1875736bc1425772) )
ROM_END


ROM_START( gt2ks100 )	/* Version 1.00S for the 3 tier type PCB with short ROM board P/N 1088 Rev 0 */
	ROM_REGION32_BE( CODE_SIZE, "user1", 0 )
	ROM_LOAD32_BYTE( "gt2kprm0.10s", 0x00000, 0x80000, CRC(3aab67c8) SHA1(c08dcad9e7c2440058ee4d683b2257c6ae42ad4d) ) /* chips labeled as "GT2K KIT PROM0 v1.00M" ect */
	ROM_LOAD32_BYTE( "gt2kprm1.10s", 0x00001, 0x80000, CRC(47d4a74d) SHA1(b4f80de1ffea11bf716c891519990e0fe2dfbc23) )
	ROM_LOAD32_BYTE( "gt2kprm2.10s", 0x00002, 0x80000, CRC(77a222cc) SHA1(bab3732c49c388f43db27c3c36c6eba02ec92708) )
	ROM_LOAD32_BYTE( "gt2kprm3.10s", 0x00003, 0x80000, CRC(c3e77ad5) SHA1(3861779c1a60778a686c3cae5b77ea408de1bb1f) )

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "gt322_nr.u23", 0x10000, 0x18000, CRC(04effd73) SHA1(4277031655f8de851eba0e4134ba619a12f5dd4a) )
	ROM_CONTINUE(             0x08000, 0x08000 )

	ROM_REGION( 0x600000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "gt2k_grm.0_0", 0x000000, 0x80000, CRC(c22b50f9) SHA1(9e8acfce6cc30adc150b602d026c00fa1fb7747f) )
	ROM_LOAD32_BYTE( "gt2k_grm.0_1", 0x000001, 0x80000, CRC(d6d6be57) SHA1(fda20185e842dd4aa1a1601f95e5cc787644f4c3) )
	ROM_LOAD32_BYTE( "gt2k_grm.0_2", 0x000002, 0x80000, CRC(005d4791) SHA1(b03d5835465ccc4fe73f4adb1342ef2b38aad90c) )
	ROM_LOAD32_BYTE( "gt2k_grm.0_3", 0x000003, 0x80000, CRC(0c998eb7) SHA1(2950a56192dd794e3f34459c52edf7ea484d6901) )
	ROM_LOAD32_BYTE( "gt2k_grm.1_0", 0x200000, 0x80000, CRC(8b79d6e2) SHA1(b4aeac78a470bd8b9f557ded39775cb56f525cce) )
	ROM_LOAD32_BYTE( "gt2k_grm.1_1", 0x200001, 0x80000, CRC(84ef1803) SHA1(c4e2f0451a35874603cc767b4dbb566b2507ae39) )
	ROM_LOAD32_BYTE( "gt2k_grm.1_2", 0x200002, 0x80000, CRC(d73d8afc) SHA1(2fced1ee7dd11c6db07750f617356c6608ac0291) )
	ROM_LOAD32_BYTE( "gt2k_grm.1_3", 0x200003, 0x80000, CRC(59f48688) SHA1(37b2c84e487f4f3a9145bef34c573a3716b4a6a7) )

	/* GT99, GT2K & GT Classic all share the above listed 8 graphics roms and may be labeled GT99, GT2K or GTClassic */

	ROM_LOAD32_BYTE( "gt2k_grm.2_0", 0x400000, 0x80000, CRC(cc11b93f) SHA1(f281448c8fa23595dd2664cc8c168565b60d4fc1) )
	ROM_LOAD32_BYTE( "gt2k_grm.2_1", 0x400001, 0x80000, CRC(1c3a0126) SHA1(3f9de1239dd64b9f50220842ffcf3e0f8928d2dc) )
	ROM_LOAD32_BYTE( "gt2k_grm.2_2", 0x400002, 0x80000, CRC(97814df5) SHA1(fcbb9ca08cbcef20231e602c99cd14e7905b2110) )
	ROM_LOAD32_BYTE( "gt2k_grm.2_3", 0x400003, 0x80000, CRC(f0f7373f) SHA1(6d71f338992598feffd9b4ac26bd0cf2f9edb53e) )

	ROM_REGION16_BE( 0x400000, "ensoniq.2", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "gt_srom0.nr", 0x000000, 0x100000, CRC(44983bd7) SHA1(a6ac966ec113b079434d7f871e4ce7266206d234) ) /* actually labeled "GTG3 SROM1 NR**" */
	ROM_LOAD16_BYTE( "gt_srom1.nr", 0x200000, 0x080000, CRC(1b3f18b6) SHA1(3b65de6a90c5ede183b5f8ca1875736bc1425772) ) /* actually labeled "GTG3 SROM2 NR*"  */
ROM_END


ROM_START( gt2kt500 ) /* Version 5.00 Tournament Edition (PCB P/N 1083 Rev 2) */
	ROM_REGION32_BE( CODE_SIZE, "user1", 0 )
	ROM_LOAD32_BYTE( "gt2kprm0.500", 0x00000, 0x100000, CRC(8f20f9eb) SHA1(e7b19c34fff39040b8849483146303d4eb394da6) )
	ROM_LOAD32_BYTE( "gt2kprm1.500", 0x00001, 0x100000, CRC(bdecc1f5) SHA1(f97edb54cffdba68d46dfb86d884192ffaa0d204) )
	ROM_LOAD32_BYTE( "gt2kprm2.500", 0x00002, 0x100000, CRC(46666c15) SHA1(7a90a8131a68b0b7bc394bdf5ef702be8164c1ee) )
	ROM_LOAD32_BYTE( "gt2kprm3.500", 0x00003, 0x100000, CRC(89544fbc) SHA1(6ce05ddccbca167cccd70cc73dbb7161e48c3e40) )

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "gt_nr.u88", 0x10000, 0x18000, CRC(2cee9e98) SHA1(02edac7abab2335c1cd824d1d9b26aa32238a2de) )
	ROM_CONTINUE(          0x08000, 0x08000 )

	ROM_REGION( 0x600000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "gt2k_grm.0_0", 0x000000, 0x80000, CRC(c22b50f9) SHA1(9e8acfce6cc30adc150b602d026c00fa1fb7747f) )
	ROM_LOAD32_BYTE( "gt2k_grm.0_1", 0x000001, 0x80000, CRC(d6d6be57) SHA1(fda20185e842dd4aa1a1601f95e5cc787644f4c3) )
	ROM_LOAD32_BYTE( "gt2k_grm.0_2", 0x000002, 0x80000, CRC(005d4791) SHA1(b03d5835465ccc4fe73f4adb1342ef2b38aad90c) )
	ROM_LOAD32_BYTE( "gt2k_grm.0_3", 0x000003, 0x80000, CRC(0c998eb7) SHA1(2950a56192dd794e3f34459c52edf7ea484d6901) )
	ROM_LOAD32_BYTE( "gt2k_grm.1_0", 0x200000, 0x80000, CRC(8b79d6e2) SHA1(b4aeac78a470bd8b9f557ded39775cb56f525cce) )
	ROM_LOAD32_BYTE( "gt2k_grm.1_1", 0x200001, 0x80000, CRC(84ef1803) SHA1(c4e2f0451a35874603cc767b4dbb566b2507ae39) )
	ROM_LOAD32_BYTE( "gt2k_grm.1_2", 0x200002, 0x80000, CRC(d73d8afc) SHA1(2fced1ee7dd11c6db07750f617356c6608ac0291) )
	ROM_LOAD32_BYTE( "gt2k_grm.1_3", 0x200003, 0x80000, CRC(59f48688) SHA1(37b2c84e487f4f3a9145bef34c573a3716b4a6a7) )

	/* GT99, GT2K & GT Classic all share the above listed 8 graphics roms and may be labeled GT99, GT2K or GTClassic */

	ROM_LOAD32_BYTE( "gt2k_grm.2_0", 0x400000, 0x80000, CRC(cc11b93f) SHA1(f281448c8fa23595dd2664cc8c168565b60d4fc1) )
	ROM_LOAD32_BYTE( "gt2k_grm.2_1", 0x400001, 0x80000, CRC(1c3a0126) SHA1(3f9de1239dd64b9f50220842ffcf3e0f8928d2dc) )
	ROM_LOAD32_BYTE( "gt2k_grm.2_2", 0x400002, 0x80000, CRC(97814df5) SHA1(fcbb9ca08cbcef20231e602c99cd14e7905b2110) )
	ROM_LOAD32_BYTE( "gt2k_grm.2_3", 0x400003, 0x80000, CRC(f0f7373f) SHA1(6d71f338992598feffd9b4ac26bd0cf2f9edb53e) )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "gt_srom0.nr", 0x000000, 0x100000, CRC(44983bd7) SHA1(a6ac966ec113b079434d7f871e4ce7266206d234) )
	ROM_LOAD16_BYTE( "gt_srom1.nr", 0x200000, 0x080000, CRC(1b3f18b6) SHA1(3b65de6a90c5ede183b5f8ca1875736bc1425772) )
ROM_END


ROM_START( gtsuprem ) /* Version 5.10T ELC S (Tournament Edition, PCB P/N 1083 Rev 2) */
	ROM_REGION32_BE( CODE_SIZE, "user1", 0 )
	ROM_LOAD32_BYTE( "gts_prm0.510", 0x00000, 0x100000, CRC(a14f7e2b) SHA1(fa913452701e580a9242994020a42e0be7d41a60) ) /* chips labeled as "GTG SUP ELC PROM0 5.10T" ect */
	ROM_LOAD32_BYTE( "gts_prm1.510", 0x00001, 0x100000, CRC(772f4dc9) SHA1(c44c1893f28386b6457bffd0a85b361c1033a805) )
	ROM_LOAD32_BYTE( "gts_prm2.510", 0x00002, 0x100000, CRC(fbaae916) SHA1(dd436f71a89acf3a6de3feffebf35f071b73ae77) )
	ROM_LOAD32_BYTE( "gts_prm3.510", 0x00003, 0x100000, CRC(69b13204) SHA1(88705d1acb44420ffbd3655e983455904f15a955) )

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "gt_nr.u88", 0x10000, 0x18000, CRC(2cee9e98) SHA1(02edac7abab2335c1cd824d1d9b26aa32238a2de) )
	ROM_CONTINUE(          0x08000, 0x08000 )

	ROM_REGION( 0x600000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "gt2k_grm.0_0", 0x000000, 0x80000, CRC(c22b50f9) SHA1(9e8acfce6cc30adc150b602d026c00fa1fb7747f) )
	ROM_LOAD32_BYTE( "gt2k_grm.0_1", 0x000001, 0x80000, CRC(d6d6be57) SHA1(fda20185e842dd4aa1a1601f95e5cc787644f4c3) )
	ROM_LOAD32_BYTE( "gt2k_grm.0_2", 0x000002, 0x80000, CRC(005d4791) SHA1(b03d5835465ccc4fe73f4adb1342ef2b38aad90c) )
	ROM_LOAD32_BYTE( "gt2k_grm.0_3", 0x000003, 0x80000, CRC(0c998eb7) SHA1(2950a56192dd794e3f34459c52edf7ea484d6901) )
	ROM_LOAD32_BYTE( "gt2k_grm.1_0", 0x200000, 0x80000, CRC(8b79d6e2) SHA1(b4aeac78a470bd8b9f557ded39775cb56f525cce) )
	ROM_LOAD32_BYTE( "gt2k_grm.1_1", 0x200001, 0x80000, CRC(84ef1803) SHA1(c4e2f0451a35874603cc767b4dbb566b2507ae39) )
	ROM_LOAD32_BYTE( "gt2k_grm.1_2", 0x200002, 0x80000, CRC(d73d8afc) SHA1(2fced1ee7dd11c6db07750f617356c6608ac0291) )
	ROM_LOAD32_BYTE( "gt2k_grm.1_3", 0x200003, 0x80000, CRC(59f48688) SHA1(37b2c84e487f4f3a9145bef34c573a3716b4a6a7) )

	/* GT99, GT2K & GT Classic all share the above listed 8 graphics roms and may be labeled GT99, GT2K or GTClassic */
	/* The Euro version has different GROM2_x compared to the standard US versions.   All GROMs labeled "GT SUPREME" */

	ROM_LOAD32_BYTE( "gts_grom.2_0", 0x400000, 0x80000, CRC(33998a3e) SHA1(53832e37c42155eb9c774eb33b8b36fe387fa162) )
	ROM_LOAD32_BYTE( "gts_grom.2_1", 0x400001, 0x80000, CRC(afa937ef) SHA1(3a6cb5a6b40ad8c77f1eceeda65afd007c8388d7) )
	ROM_LOAD32_BYTE( "gts_grom.2_2", 0x400002, 0x80000, CRC(8f39c061) SHA1(36fd4660ba13595eab367535a5ad9137fd753d22) )
	ROM_LOAD32_BYTE( "gts_grom.2_3", 0x400003, 0x80000, CRC(c3c2337a) SHA1(02e6bb31dad5c2563cd6da6c419f327796b06e80) )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "gt_srom0.nr", 0x000000, 0x100000, CRC(44983bd7) SHA1(a6ac966ec113b079434d7f871e4ce7266206d234) )
	ROM_LOAD16_BYTE( "gt_srom1.nr", 0x200000, 0x080000, CRC(1b3f18b6) SHA1(3b65de6a90c5ede183b5f8ca1875736bc1425772) )
ROM_END


ROM_START( gtclassc ) /* Version 1.00 */
	ROM_REGION32_BE( CODE_SIZE, "user1", 0 )
	ROM_LOAD32_BYTE( "gtc_prm0.100", 0x00000, 0x80000, CRC(a57e6ef0) SHA1(9a67b8d9314a774654f89343df2e6a6fd3cfef01) )
	ROM_LOAD32_BYTE( "gtc_prm1.100", 0x00001, 0x80000, CRC(15f8a831) SHA1(982675b26f5f19aaf7d8adc73474e05dd82c56a3) )
	ROM_LOAD32_BYTE( "gtc_prm2.100", 0x00002, 0x80000, CRC(2f260a93) SHA1(b953e003a588e6c1d7d7c065afd6cdfefb526642) )
	ROM_LOAD32_BYTE( "gtc_prm3.100", 0x00003, 0x80000, CRC(03a1fcdd) SHA1(6cf96de58231a3734adc272c2631d09b93eca8ad) )

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "gt_nr.u88", 0x10000, 0x18000, CRC(2cee9e98) SHA1(02edac7abab2335c1cd824d1d9b26aa32238a2de) )
	ROM_CONTINUE(          0x08000, 0x08000 )

	ROM_REGION( 0x600000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "gtc_grom.0_0", 0x000000, 0x80000, CRC(c22b50f9) SHA1(9e8acfce6cc30adc150b602d026c00fa1fb7747f) )
	ROM_LOAD32_BYTE( "gtc_grom.0_1", 0x000001, 0x80000, CRC(d6d6be57) SHA1(fda20185e842dd4aa1a1601f95e5cc787644f4c3) )
	ROM_LOAD32_BYTE( "gtc_grom.0_2", 0x000002, 0x80000, CRC(005d4791) SHA1(b03d5835465ccc4fe73f4adb1342ef2b38aad90c) )
	ROM_LOAD32_BYTE( "gtc_grom.0_3", 0x000003, 0x80000, CRC(0c998eb7) SHA1(2950a56192dd794e3f34459c52edf7ea484d6901) )
	ROM_LOAD32_BYTE( "gtc_grom.1_0", 0x200000, 0x80000, CRC(8b79d6e2) SHA1(b4aeac78a470bd8b9f557ded39775cb56f525cce) )
	ROM_LOAD32_BYTE( "gtc_grom.1_1", 0x200001, 0x80000, CRC(84ef1803) SHA1(c4e2f0451a35874603cc767b4dbb566b2507ae39) )
	ROM_LOAD32_BYTE( "gtc_grom.1_2", 0x200002, 0x80000, CRC(d73d8afc) SHA1(2fced1ee7dd11c6db07750f617356c6608ac0291) )
	ROM_LOAD32_BYTE( "gtc_grom.1_3", 0x200003, 0x80000, CRC(59f48688) SHA1(37b2c84e487f4f3a9145bef34c573a3716b4a6a7) )

	/* GT99, GT2K & GT Classic all share the above listed 8 graphics roms and may be labeled GT99, GT2K or GTClassic */

	ROM_LOAD32_BYTE( "gtc_grom.2_0", 0x400000, 0x80000, CRC(c4f54398) SHA1(08e57ef5cb56c793edc677d53b2e036acf558564) )
	ROM_LOAD32_BYTE( "gtc_grom.2_1", 0x400001, 0x80000, CRC(2c1f83cf) SHA1(a5a8724c59177fbf2a676ece0e93a08a3a1b0d68) )
	ROM_LOAD32_BYTE( "gtc_grom.2_2", 0x400002, 0x80000, CRC(607657a6) SHA1(705e8c6878e9b2f4da510c198f7ac8987869fce3) )
	ROM_LOAD32_BYTE( "gtc_grom.2_3", 0x400003, 0x80000, CRC(7ad615c1) SHA1(b5360885f775ba5e5e13fa624091cca6e3e6948a) )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "gt_srom0.nr", 0x000000, 0x100000, CRC(44983bd7) SHA1(a6ac966ec113b079434d7f871e4ce7266206d234) )
	ROM_LOAD16_BYTE( "gt_srom1.nr", 0x200000, 0x080000, CRC(1b3f18b6) SHA1(3b65de6a90c5ede183b5f8ca1875736bc1425772) )
ROM_END


ROM_START( gtclasscp ) /* Version 1.00 Infinite Loop Protection */
	ROM_REGION32_BE( CODE_SIZE, "user1", 0 )
	ROM_LOAD32_BYTE( "gtcpprm0.100", 0x00000, 0x80000, CRC(21f0e0ea) SHA1(734f5ec7f28451d46656c8a0c2cc96c09c55cf9b) )
	ROM_LOAD32_BYTE( "gtcpprm1.100", 0x00001, 0x80000, CRC(d2a69fbc) SHA1(a34d87bbfe1d9273d16cb73fe20fbf8ccd04e2b1) )
	ROM_LOAD32_BYTE( "gtcpprm2.100", 0x00002, 0x80000, CRC(a8dea029) SHA1(b2541879aab7e468da846e464f7f642262db03b3) )
	ROM_LOAD32_BYTE( "gtcpprm3.100", 0x00003, 0x80000, CRC(6016299e) SHA1(ee92768615bddb74f63c4e2ccb46550fa8ece86c) )

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "gt_nr.u88", 0x10000, 0x18000, CRC(2cee9e98) SHA1(02edac7abab2335c1cd824d1d9b26aa32238a2de) )
	ROM_CONTINUE(          0x08000, 0x08000 )

	ROM_REGION( 0x600000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "gtc_grom.0_0", 0x000000, 0x80000, CRC(c22b50f9) SHA1(9e8acfce6cc30adc150b602d026c00fa1fb7747f) )
	ROM_LOAD32_BYTE( "gtc_grom.0_1", 0x000001, 0x80000, CRC(d6d6be57) SHA1(fda20185e842dd4aa1a1601f95e5cc787644f4c3) )
	ROM_LOAD32_BYTE( "gtc_grom.0_2", 0x000002, 0x80000, CRC(005d4791) SHA1(b03d5835465ccc4fe73f4adb1342ef2b38aad90c) )
	ROM_LOAD32_BYTE( "gtc_grom.0_3", 0x000003, 0x80000, CRC(0c998eb7) SHA1(2950a56192dd794e3f34459c52edf7ea484d6901) )
	ROM_LOAD32_BYTE( "gtc_grom.1_0", 0x200000, 0x80000, CRC(8b79d6e2) SHA1(b4aeac78a470bd8b9f557ded39775cb56f525cce) )
	ROM_LOAD32_BYTE( "gtc_grom.1_1", 0x200001, 0x80000, CRC(84ef1803) SHA1(c4e2f0451a35874603cc767b4dbb566b2507ae39) )
	ROM_LOAD32_BYTE( "gtc_grom.1_2", 0x200002, 0x80000, CRC(d73d8afc) SHA1(2fced1ee7dd11c6db07750f617356c6608ac0291) )
	ROM_LOAD32_BYTE( "gtc_grom.1_3", 0x200003, 0x80000, CRC(59f48688) SHA1(37b2c84e487f4f3a9145bef34c573a3716b4a6a7) )

	/* GT99, GT2K & GT Classic all share the above listed 8 graphics roms and may be labeled GT99, GT2K or GTClassic */

	ROM_LOAD32_BYTE( "gtc_grom.2_0", 0x400000, 0x80000, CRC(c4f54398) SHA1(08e57ef5cb56c793edc677d53b2e036acf558564) )
	ROM_LOAD32_BYTE( "gtc_grom.2_1", 0x400001, 0x80000, CRC(2c1f83cf) SHA1(a5a8724c59177fbf2a676ece0e93a08a3a1b0d68) )
	ROM_LOAD32_BYTE( "gtc_grom.2_2", 0x400002, 0x80000, CRC(607657a6) SHA1(705e8c6878e9b2f4da510c198f7ac8987869fce3) )
	ROM_LOAD32_BYTE( "gtc_grom.2_3", 0x400003, 0x80000, CRC(7ad615c1) SHA1(b5360885f775ba5e5e13fa624091cca6e3e6948a) )

	ROM_REGION16_BE( 0x400000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "gt_srom0.nr", 0x000000, 0x100000, CRC(44983bd7) SHA1(a6ac966ec113b079434d7f871e4ce7266206d234) )
	ROM_LOAD16_BYTE( "gt_srom1.nr", 0x200000, 0x080000, CRC(1b3f18b6) SHA1(3b65de6a90c5ede183b5f8ca1875736bc1425772) )
ROM_END


ROM_START( gtclasscs )  /* Version 1.00S for the 3 tier type PCB with short ROM board P/N 1088 Rev 0 */
	ROM_REGION32_BE( CODE_SIZE, "user1", 0 )
	ROM_LOAD32_BYTE( "gtc_prm0.10s", 0x00000, 0x80000, CRC(1e41884f) SHA1(354baf00ad7cba4cdcd55c3a26dd0171dc39448a) ) /* chips labeled as "GTClassic PROM0 v1.00M" ect */
	ROM_LOAD32_BYTE( "gtc_prm1.10s", 0x00001, 0x80000, CRC(31c18b2c) SHA1(0ce5ff917b135786354d87aae88f64fdd17b1a47) )
	ROM_LOAD32_BYTE( "gtc_prm2.10s", 0x00002, 0x80000, CRC(8896efcb) SHA1(59cb9793c610a9e9b3119d8d570a15253a821ede) )
	ROM_LOAD32_BYTE( "gtc_prm3.10s", 0x00003, 0x80000, CRC(567a9490) SHA1(45379c9a26b82d0a8dbb5da26a9f23dbf1e87fc1) )

	ROM_REGION( 0x28000, "soundcpu", 0 )
	ROM_LOAD( "gt322_nr.u23", 0x10000, 0x18000, CRC(04effd73) SHA1(4277031655f8de851eba0e4134ba619a12f5dd4a) )
	ROM_CONTINUE(             0x08000, 0x08000 )

	ROM_REGION( 0x600000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "gtc_grom.0_0", 0x000000, 0x80000, CRC(c22b50f9) SHA1(9e8acfce6cc30adc150b602d026c00fa1fb7747f) )
	ROM_LOAD32_BYTE( "gtc_grom.0_1", 0x000001, 0x80000, CRC(d6d6be57) SHA1(fda20185e842dd4aa1a1601f95e5cc787644f4c3) )
	ROM_LOAD32_BYTE( "gtc_grom.0_2", 0x000002, 0x80000, CRC(005d4791) SHA1(b03d5835465ccc4fe73f4adb1342ef2b38aad90c) )
	ROM_LOAD32_BYTE( "gtc_grom.0_3", 0x000003, 0x80000, CRC(0c998eb7) SHA1(2950a56192dd794e3f34459c52edf7ea484d6901) )
	ROM_LOAD32_BYTE( "gtc_grom.1_0", 0x200000, 0x80000, CRC(8b79d6e2) SHA1(b4aeac78a470bd8b9f557ded39775cb56f525cce) )
	ROM_LOAD32_BYTE( "gtc_grom.1_1", 0x200001, 0x80000, CRC(84ef1803) SHA1(c4e2f0451a35874603cc767b4dbb566b2507ae39) )
	ROM_LOAD32_BYTE( "gtc_grom.1_2", 0x200002, 0x80000, CRC(d73d8afc) SHA1(2fced1ee7dd11c6db07750f617356c6608ac0291) )
	ROM_LOAD32_BYTE( "gtc_grom.1_3", 0x200003, 0x80000, CRC(59f48688) SHA1(37b2c84e487f4f3a9145bef34c573a3716b4a6a7) )

	/* GT99, GT2K & GT Classic all share the above listed 8 graphics roms and may be labeled GT99, GT2K or GTClassic */

	ROM_LOAD32_BYTE( "gtc_grom.2_0", 0x400000, 0x80000, CRC(c4f54398) SHA1(08e57ef5cb56c793edc677d53b2e036acf558564) )
	ROM_LOAD32_BYTE( "gtc_grom.2_1", 0x400001, 0x80000, CRC(2c1f83cf) SHA1(a5a8724c59177fbf2a676ece0e93a08a3a1b0d68) )
	ROM_LOAD32_BYTE( "gtc_grom.2_2", 0x400002, 0x80000, CRC(607657a6) SHA1(705e8c6878e9b2f4da510c198f7ac8987869fce3) )
	ROM_LOAD32_BYTE( "gtc_grom.2_3", 0x400003, 0x80000, CRC(7ad615c1) SHA1(b5360885f775ba5e5e13fa624091cca6e3e6948a) )


	ROM_REGION16_BE( 0x400000, "ensoniq.2", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "gt_srom0.nr", 0x000000, 0x100000, CRC(44983bd7) SHA1(a6ac966ec113b079434d7f871e4ce7266206d234) ) /* actually labeled "GTG3 SROM1 NR**" */
	ROM_LOAD16_BYTE( "gt_srom1.nr", 0x200000, 0x080000, CRC(1b3f18b6) SHA1(3b65de6a90c5ede183b5f8ca1875736bc1425772) ) /* actually labeled "GTG3 SROM2 NR*"  */
ROM_END



/*************************************
 *
 *  Driver-specific init
 *
 *************************************/

static void init_program_rom(running_machine *machine)
{
	memcpy(main_ram, main_rom, 0x80);
}


static DRIVER_INIT( timekill )
{
	init_program_rom(machine);
	itech32_vram_height = 512;
	itech32_planes = 2;
	is_drivedge = 0;
}


static DRIVER_INIT( hardyard )
{
	init_program_rom(machine);
	itech32_vram_height = 1024;
	itech32_planes = 1;
	is_drivedge = 0;
}


static DRIVER_INIT( bloodstm )
{
	init_program_rom(machine);
	itech32_vram_height = 1024;
	itech32_planes = 1;
	is_drivedge = 0;
}


static DRIVER_INIT( drivedge )
{
	init_program_rom(machine);
	itech32_vram_height = 1024;
	itech32_planes = 1;
	is_drivedge = 1;

	memory_install_read32_handler(cputag_get_address_space(machine, "dsp1", ADDRESS_SPACE_PROGRAM), 0x8382, 0x8382, 0, 0, drivedge_tms1_speedup_r);
	memory_install_read32_handler(cputag_get_address_space(machine, "dsp2", ADDRESS_SPACE_PROGRAM), 0x8382, 0x8382, 0, 0, drivedge_tms2_speedup_r);
}


static DRIVER_INIT( wcbowl )
{
	/*
        This is the 3 tier PCB set:
          Main  P/N 1059 Rev 3 (see Hot Memory PCB layout above)
          ROM   P/N 1079 Rev 1 (contains graphic roms, 4MHz OSC + ITBWL-1 security PIC chip)
          Sound P/N 1060 Rev 0 (see Hot Memory PCB layout above)
    */
	init_program_rom(machine);
	itech32_vram_height = 1024;
	itech32_planes = 1;

	memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x680000, 0x680001, 0, 0, trackball_r);

	memory_nop_read(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x578000, 0x57ffff, 0, 0);
	memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x680080, 0x680081, 0, 0, wcbowl_prot_result_r);
	memory_nop_write(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x680080, 0x680081, 0, 0);
}


static void init_sftm_common(running_machine *machine, int prot_addr)
{
	init_program_rom(machine);
	itech32_vram_height = 1024;
	itech32_planes = 1;
	is_drivedge = 0;

	itech020_prot_address = prot_addr;

	memory_install_write32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x300000, 0x300003, 0, 0, itech020_color2_w);
	memory_install_write32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x380000, 0x380003, 0, 0, itech020_color1_w);
}


static DRIVER_INIT( sftm )
{
	init_sftm_common(machine, 0x7a6a);
}


static DRIVER_INIT( sftm110 )
{
	init_sftm_common(machine, 0x7a66);
}


static void init_shuffle_bowl_common(running_machine *machine, int prot_addr)
{
	/*
        The newest versions of World Class Bowling are on the same exact
        platform as Shuffle Shot. So We'll use the same general INIT
        routine for these two programs.  IE: PCB P/N 1083 Rev 2
    */
	init_program_rom(machine);
	itech32_vram_height = 1024;
	itech32_planes = 1;
	is_drivedge = 0;

	itech020_prot_address = prot_addr;

	memory_install_write32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x300000, 0x300003, 0, 0, itech020_color2_w);
	memory_install_write32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x380000, 0x380003, 0, 0, itech020_color1_w);
	memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x180800, 0x180803, 0, 0, trackball32_4bit_r);
	memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x181000, 0x181003, 0, 0, trackball32_4bit_p2_r);
}


static DRIVER_INIT( shufshot )	/* PIC 16C54 labeled as ITSHF-1 (ITBWL-4 for WCB Deluxe) */
{
	init_shuffle_bowl_common(machine, 0x111a);
}


static DRIVER_INIT( wcbowln )	/* PIC 16C54 labeled as ITBWL-3 */
{
	/* The security PROM is NOT interchangable between the Deluxe and "normal" versions. */
	init_shuffle_bowl_common(machine, 0x1116);
}

static void install_timekeeper(running_machine *machine)
{
	running_device *device = machine->device("m48t02");
	memory_install_readwrite32_device_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), device, 0x681000, 0x6817ff, 0, 0, timekeeper_32be_r, timekeeper_32be_w);
}

static DRIVER_INIT( wcbowlt )	/* PIC 16C54 labeled as ITBWL-3 */
{
	/* Tournament Version, Same protection memory address as WCB Deluxe, but uses the standard WCB pic ITBWL-3 */
	init_shuffle_bowl_common(machine, 0x111a);

	install_timekeeper(machine);
}

static void init_gt_common(running_machine *machine)
{
	init_program_rom(machine);
	itech32_vram_height = 1024;
	itech32_planes = 2;
	is_drivedge = 0;

	itech020_prot_address = 0x112f;
}


static DRIVER_INIT( gt3d )
{
	/*
        This is the 3 tier PCB with the short ROM board:
        Known (dumped) GT versions on this board:  GT3D v1.4 through v1.92S

        Hacked versions of this PCB have been found with GT97
        through GTClassic. This is _NOT_ a factory modification
    */
	memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x200000, 0x200003, 0, 0, trackball32_8bit_r);
	init_gt_common(machine);
}


static DRIVER_INIT( aama )
{
	/*
        This is the single PCB style board commonly referred to as:
        "AAMA Serial Numbers 676266 and Up." All versions of GT on this
        board share the same sound CPU code and sample ROMs.
        This board has all versions of GT for it, GT3D through GTClassic
    */
	memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x180800, 0x180803, 0, 0, trackball32_4bit_r);
	memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x181000, 0x181003, 0, 0, trackball32_4bit_p2_r);
	init_gt_common(machine);
}


static DRIVER_INIT( aamat )
{
	/*
        Tournament Version - So install needed handler for the TimeKeeper ram
    */
	DRIVER_INIT_CALL(aama);

	install_timekeeper(machine);
}


static DRIVER_INIT( s_ver )
{
	/*
        This is a special 3 tier PCB with a short ROM board and 1 trackball
        connector, P/N 1088 Rev 0. Known versions of Golden Tee for this
        board: GT97 v1.21S, GT98, GT99, GT2K & GT Classic Versions 1.00S
        Trackball info is read through 200202 (actually 200203).
    */
	memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x200200, 0x200203, 0, 0, trackball32_4bit_r);
	init_gt_common(machine);
}


static DRIVER_INIT( gt3dl )
{
	/*
        This is the 3 tier PCB with the long ROM board:
        Known GT versions on this board:  GT3D v1.92L & v1.91L

        Player 1 trackball read through 200003
        Player 2 trackball read through 200002
    */
	memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x200000, 0x200003, 0, 0, trackball32_4bit_combined_r);
	init_gt_common(machine);
}


static DRIVER_INIT( gt2kp )
{
	/* a little extra protection */
	memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x680000, 0x680003, 0, 0, gt2kp_prot_result_r);
	DRIVER_INIT_CALL(aama);

	/* The protection code is:

        addq #1,    1132.w
        clr.l       d0
        clr.l       d1
        move.b      680002,d0       ; Read protection result
        move.b      112f.w,d1
        andi.b      #$01,d0
Label1  bne.s       Label1          ; Infinite loop if result isn't 0x01
        nop                         ; Otherwise just return to the game :-)

    */
}


static DRIVER_INIT( gtclasscp )
{
	/* a little extra protection */
	memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x680000, 0x680003, 0, 0, gtclass_prot_result_r);
	DRIVER_INIT_CALL(aama);

	/* The protection code is:

        addq #1,    1132.w
        clr.l       d0
        clr.l       d1
        move.b      680002,d0       ; Read protection result
        andi.b      #$C0,d0
        cmpi.b      #$80,d0
Label1  bne.s       Label1          ; Infinite loop if result isn't 0x80
        rts                         ; Otherwise just return to the game :-)

    */
}



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1992, timekill,    0,        timekill, timekill, timekill, ROT0, "Strata/Incredible Technologies",   "Time Killers (v1.32)", 0 )
GAME( 1992, timekill131, timekill, timekill, timekill, timekill, ROT0, "Strata/Incredible Technologies",   "Time Killers (v1.31)", 0 )
GAME( 1993, hardyard,    0,        bloodstm, hardyard, hardyard, ROT0, "Strata/Incredible Technologies",   "Hard Yardage (v1.20)", 0 )
GAME( 1993, hardyard10,  hardyard, bloodstm, hardyard, hardyard, ROT0, "Strata/Incredible Technologies",   "Hard Yardage (v1.00)", 0 )
GAME( 1994, bloodstm,    0,        bloodstm, bloodstm, bloodstm, ROT0, "Strata/Incredible Technologies",   "Blood Storm (v2.22)", 0 )
GAME( 1994, bloodstm22,  bloodstm, bloodstm, bloodstm, bloodstm, ROT0, "Strata/Incredible Technologies",   "Blood Storm (v2.20)", 0 )
GAME( 1994, bloodstm21,  bloodstm, bloodstm, bloodstm, bloodstm, ROT0, "Strata/Incredible Technologies",   "Blood Storm (v2.10)", 0 )
GAME( 1994, bloodstm11,  bloodstm, bloodstm, bloodstm, bloodstm, ROT0, "Strata/Incredible Technologies",   "Blood Storm (v1.10)", 0 )
GAME( 1994, pairs,       0,        bloodstm, pairs,    bloodstm, ROT0, "Strata/Incredible Technologies",   "Pairs (V1.2, 09/30/94)", 0 )
GAME( 1994, pairsa,      pairs,    bloodstm, pairs,    bloodstm, ROT0, "Strata/Incredible Technologies",   "Pairs (09/07/94)", 0 )
GAME( 1994, hotmemry,    pairs,    bloodstm, pairs,    bloodstm, ROT0, "Tuning/Incredible Technologies",   "Hot Memory (V1.2, Germany)", 0 )
GAME( 1994, drivedge,    0,        drivedge, drivedge, drivedge, ROT0, "Strata/Incredible Technologies",   "Driver's Edge", GAME_IMPERFECT_GRAPHICS )
GAME( 1995, wcbowl,      0,        sftm,     wcbowln,  wcbowln,  ROT0, "Incredible Technologies",          "World Class Bowling (v1.66)" , 0) /* PIC 16C54 labeled as ITBWL-3 */
GAME( 1995, wcbowl165,   wcbowl,   sftm,     shufbowl, wcbowln,  ROT0, "Incredible Technologies",          "World Class Bowling (v1.65)" , 0) /* PIC 16C54 labeled as ITBWL-3 */
GAME( 1995, wcbowl161,   wcbowl,   sftm,     shufbowl, wcbowln,  ROT0, "Incredible Technologies",          "World Class Bowling (v1.61)" , 0) /* PIC 16C54 labeled as ITBWL-3 */
GAME( 1995, wcbowl16,    wcbowl,   sftm,     shufbowl, wcbowln,  ROT0, "Incredible Technologies",          "World Class Bowling (v1.6)" , 0) /* PIC 16C54 labeled as ITBWL-3 */
GAME( 1995, wcbowl15,    wcbowl,   bloodstm, wcbowl,   wcbowl,   ROT0, "Incredible Technologies",          "World Class Bowling (v1.5)" , 0) /* PIC 16C54 labeled as ITBWL-1 */
GAME( 1995, wcbowl13,    wcbowl,   bloodstm, wcbowl,   wcbowl,   ROT0, "Incredible Technologies",          "World Class Bowling (v1.3)" , 0) /* PIC 16C54 labeled as ITBWL-1 */
GAME( 1995, wcbowl12,    wcbowl,   bloodstm, wcbowl,   wcbowl,   ROT0, "Incredible Technologies",          "World Class Bowling (v1.2)" , 0) /* PIC 16C54 labeled as ITBWL-1 */
GAME( 1995, sftm,        0,        sftm,     sftm,     sftm,     ROT0, "Capcom / Incredible Technologies", "Street Fighter: The Movie (v1.12)" , 0) /* PIC 16C54 labeled as ITSF-1 */
GAME( 1995, sftm111,     sftm,     sftm,     sftm,     sftm110,  ROT0, "Capcom / Incredible Technologies", "Street Fighter: The Movie (v1.11)" , 0) /* PIC 16C54 labeled as ITSF-1 */
GAME( 1995, sftm110,     sftm,     sftm,     sftm,     sftm110,  ROT0, "Capcom / Incredible Technologies", "Street Fighter: The Movie (v1.10)" , 0) /* PIC 16C54 labeled as ITSF-1 */
GAME( 1995, sftmj,       sftm,     sftm,     sftm,     sftm,     ROT0, "Capcom / Incredible Technologies", "Street Fighter: The Movie (v1.12N, Japan)" , 0) /* PIC 16C54 labeled as ITSF-1 */
GAME( 1997, shufshot,    0,        sftm,     shufshot, shufshot, ROT0, "Strata/Incredible Technologies",   "Shuffleshot (v1.40)" , 0) /* PIC 16C54 labeled as ITSHF-1 */
GAME( 1997, shufshot139, shufshot, sftm,     shufshot, shufshot, ROT0, "Strata/Incredible Technologies",   "Shuffleshot (v1.39)" , 0) /* PIC 16C54 labeled as ITSHF-1 */
GAME( 1997, shufshot137, shufshot, sftm,     shufbowl, shufshot, ROT0, "Strata/Incredible Technologies",   "Shuffleshot (v1.37)" , 0) /* PIC 16C54 labeled as ITSHF-1 */
GAME( 1997, wcbowl140,   wcbowldx, tourny,   wcbowldx, wcbowlt,  ROT0, "Incredible Technologies",          "World Class Bowling Tournament (v1.40)" , 0) /* PIC 16C54 labeled as ITBWL-3 */
GAME( 1999, wcbowldx,    0,        sftm,     wcbowldx, shufshot, ROT0, "Incredible Technologies",          "World Class Bowling Deluxe (v2.00)" , 0) /* PIC 16C54 labeled as ITBWL-4 */

/*
    The following naming conventions are used:

    gt3d s 192
      ^^ ^ ^^^
    Game | Version
         |
    v = Version (AAMA 676266+ PCB P/N 1083 Rev 2)
    l = Long ROM board:
        Main  P/N 1064 Rev 1
        ROM   P/N 1080 Rev 5
        Sound P/N 1078 Rev 1
    s = Short ROM board:
        Main  P/N 1064 Rev 1
        ROM   P/N 1069 Rev 2 or P/N 1088 Rev 0 (1 Trackball Connector)
        Sound P/N 1078 Rev 1 or P/N 1066 Rev 2
    n = Non-tournament (AAMA 676266+) PCB P/N 1083 Rev 2 - Applies only for GT3D
    t = Tournament PCB P/N 1083 Rev 2, with necessary chips on board:
        Philips SCN68681 (DUART) & ST Timekeeper (M48T02-120PC1) with an additional 3.6864MHz OSC

    NOTE: Due to various different upgrade packages from IT, the 3 tier boards can end up with any combination
        of rom boards and sound boards.  For historical reasons, GTG3 3 tier S versions will use the oldest
        sound file set up. Version L will use the newer Ensoniq sample rom and v2.X sound code, while gt97
        through gtclassic will use the lastest "NR" versions.

  GT Royal Edition Tournament is a Euro GT99
  GT Supreme Edition Tournament is a Euro GT2K with slightly different GROM2_0 through GROM2_3
     GT Supreme (on a real PCB with actual tournament data) comes up with a different title screen and is (c) 2002
     showing the title as GT Supreme Plus! and the Hole-In-One board shows an aditional course, Coconut Cove from GT99
     Current emulation does not reproduce this extra title screen due to missing tournament data.
  GT Diamond Edition Tournament is a Euro GT Classic (Not currently dumped)
     This would be the only known tournament version of Classic

NOTE: There is an "8 Meg board" version of the P/N 1083 Rev 2 PCB, so GROM0_0 through GROM0_3 are 8 meg roms and labeled "GTxx GROM0_0 **" ect
      while GROM1_0 through GROM1_3 are 4 meg roms matching "4 Meg board" GROM2_0 through GROM2_3 and are labeled "GTxx GROM1_0 *" ect
      It is possible to find these rom combinations on any given GT board set.  There is only 1 known GT set which specificly checks for 8 meg
      roms under the GROM Checksum test.

    Parent set will always be gt(year) with the most recent version.  IE: gt97 is Golden Tee '97 v1.30

*/
GAME( 1995, gt3d,     0,        sftm,    gt3d,  aama,     ROT0, "Incredible Technologies", "Golden Tee 3D Golf (v1.93N)" , 0) /* PIC 16C54 labeled as ITGF-2 */
GAME( 1995, gt3dl192, gt3d,     sftm,    gt3d,  gt3dl,    ROT0, "Incredible Technologies", "Golden Tee 3D Golf (v1.92L)" , 0) /* PIC 16C54 labeled as ITGF-2 */
GAME( 1995, gt3dl191, gt3d,     sftm,    gt3d,  gt3dl,    ROT0, "Incredible Technologies", "Golden Tee 3D Golf (v1.91L)" , 0) /* PIC 16C54 labeled as ITGF-2 */
GAME( 1995, gt3ds192, gt3d,     sftm,    gt3d,  gt3d,     ROT0, "Incredible Technologies", "Golden Tee 3D Golf (v1.92S)" , 0) /* PIC 16C54 labeled as ITGF-1 */
GAME( 1995, gt3dv18,  gt3d,     sftm,    gt3d,  gt3d,     ROT0, "Incredible Technologies", "Golden Tee 3D Golf (v1.8)" , 0) /* PIC 16C54 labeled as ITGF-1 */
GAME( 1995, gt3dv17,  gt3d,     sftm,    gt3d,  gt3d,     ROT0, "Incredible Technologies", "Golden Tee 3D Golf (v1.7)" , 0) /* PIC 16C54 labeled as ITGF-1 */
GAME( 1995, gt3dv16,  gt3d,     sftm,    gt3d,  gt3d,     ROT0, "Incredible Technologies", "Golden Tee 3D Golf (v1.6)" , 0) /* PIC 16C54 labeled as ITGF-1 */
GAME( 1995, gt3dv15,  gt3d,     sftm,    gt3d,  gt3d,     ROT0, "Incredible Technologies", "Golden Tee 3D Golf (v1.5)" , 0) /* PIC 16C54 labeled as ITGF-1 */
GAME( 1995, gt3dv14,  gt3d,     sftm,    gt3d,  gt3d,     ROT0, "Incredible Technologies", "Golden Tee 3D Golf (v1.4)" , 0) /* PIC 16C54 labeled as ITGF-1 */
GAME( 1995, gt3dt231, gt3d,     tourny,  gt3d,  aamat,    ROT0, "Incredible Technologies", "Golden Tee 3D Golf Tournament (v2.31)" , 0) /* PIC 16C54 labeled as ITGF-2 */
GAME( 1995, gt3dt211, gt3d,     tourny,  gt3d,  aamat,    ROT0, "Incredible Technologies", "Golden Tee 3D Golf Tournament (v2.11)" , 0) /* PIC 16C54 labeled as ITGF-2 */

GAME( 1997, gt97,     0,        sftm,    gt97,  aama,     ROT0, "Incredible Technologies", "Golden Tee '97 (v1.30)" , 0) /* PIC 16C54 labeled as ITGFS-3 */
GAME( 1997, gt97v122, gt97,     sftm,    gt97o, aama,     ROT0, "Incredible Technologies", "Golden Tee '97 (v1.22)" , 0) /* PIC 16C54 labeled as ITGFS-3 */
GAME( 1997, gt97v121, gt97,     sftm,    gt97o, aama,     ROT0, "Incredible Technologies", "Golden Tee '97 (v1.21)" , 0) /* PIC 16C54 labeled as ITGFS-3 */
GAME( 1997, gt97s121, gt97,     sftm,    gt97s, s_ver,    ROT0, "Incredible Technologies", "Golden Tee '97 (v1.21S)" , 0) /* PIC 16C54 labeled as ITGFM-3 */
GAME( 1997, gt97v120, gt97,     sftm,    gt97o, aama,     ROT0, "Incredible Technologies", "Golden Tee '97 (v1.20)" , 0) /* PIC 16C54 labeled as ITGFS-3 */
GAME( 1997, gt97t243, gt97,     tourny,  gt97o, aamat,    ROT0, "Incredible Technologies", "Golden Tee '97 Tournament (v2.43)" , 0) /* PIC 16C54 labeled as ITGFS-3 */
GAME( 1997, gt97t240, gt97,     tourny,  gt97o, aamat,    ROT0, "Incredible Technologies", "Golden Tee '97 Tournament (v2.40)" , 0) /* PIC 16C54 labeled as ITGFS-3 */

GAME( 1998, gt98,     0,        sftm,    aama,  aama,     ROT0, "Incredible Technologies", "Golden Tee '98 (v1.10)" , 0) /* PIC 16C54 labeled as ITGF98 */
GAME( 1998, gt98v100, gt98,     sftm,    gt98,  aama,     ROT0, "Incredible Technologies", "Golden Tee '98 (v1.00)" , 0) /* PIC 16C54 labeled as ITGF98 */
GAME( 1998, gt98s100, gt98,     sftm,    gt98s, s_ver,    ROT0, "Incredible Technologies", "Golden Tee '98 (v1.00S)" , 0) /* PIC 16C54 labeled as ITGF98-M */
GAME( 1998, gt98t303, gt98,     tourny,  gt98s, aamat,    ROT0, "Incredible Technologies", "Golden Tee '98 Tournament (v3.03)" , 0) /* PIC 16C54 labeled as ITGF98 */

GAME( 1999, gt99,     0,        sftm,    aama,  aama,     ROT0, "Incredible Technologies", "Golden Tee '99 (v1.00)" , 0) /* PIC 16C54 labeled as ITGF99 */
GAME( 1999, gt99s100, gt99,     sftm,    s_ver, s_ver,    ROT0, "Incredible Technologies", "Golden Tee '99 (v1.00S)" , 0) /* PIC 16C54 labeled as ITGF99-M */
GAME( 1999, gt99t400, gt99,     tourny,  gt98s, aamat,    ROT0, "Incredible Technologies", "Golden Tee '99 Tournament (v4.00)" , 0) /* PIC 16C54 labeled as ITGF99 */
GAME( 1999, gtroyal,  gt99,     tourny,  gt98s, aamat,    ROT0, "Incredible Technologies", "Golden Tee Royal Edition Tournament (v4.02)" , 0) /* PIC 16C54 labeled as ITGF99I */

GAME( 2000, gt2k,     0,        sftm,    aama,  aama,     ROT0, "Incredible Technologies", "Golden Tee 2K (v1.00)" , 0) /* PIC 16C54 labeled as ITGF2K */
GAME( 2000, gt2kp100, gt2k,     sftm,    aama,  gt2kp,    ROT0, "Incredible Technologies", "Golden Tee 2K (v1.00) (alt protection)" , 0) /* PIC 16C54 labeled as ???? */
GAME( 2000, gt2ks100, gt2k,     sftm,    s_ver, s_ver,    ROT0, "Incredible Technologies", "Golden Tee 2K (v1.00S)" , 0) /* PIC 16C54 labeled as ITGF2K-M */
GAME( 2000, gt2kt500, gt2k,     tourny,  gt98s, aamat,    ROT0, "Incredible Technologies", "Golden Tee 2K Tournament (v5.00)" , 0) /* PIC 16C54 labeled as ITGF2K */
GAME( 2002, gtsuprem, gt2k,     tourny,  gt98s, aamat,    ROT0, "Incredible Technologies", "Golden Tee Supreme Edition Tournament (v5.10)" , 0) /* PIC 16C54 labeled as ITGF2K-I */

GAME( 2001, gtclassc,  0,        sftm,    aama,  aama,      ROT0, "Incredible Technologies", "Golden Tee Classic (v1.00)" , 0) /* PIC 16C54 labeled as ITGFCL */
GAME( 2001, gtclasscp, gtclassc, sftm,    aama,  gtclasscp, ROT0, "Incredible Technologies", "Golden Tee Classic (v1.00) (alt protection)" , 0) /* PIC 16C54 labeled as ITGFCL */
GAME( 2001, gtclasscs, gtclassc, sftm,    s_ver, s_ver,     ROT0, "Incredible Technologies", "Golden Tee Classic (v1.00S)" , 0) /* PIC 16C54 labeled as ITGFCL-M */
