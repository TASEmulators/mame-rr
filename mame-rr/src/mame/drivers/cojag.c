/***************************************************************************

    Atari CoJag hardware

    driver by Aaron Giles

    Games supported:
        * Area 51 (3 Sets)
        * Maximum Force (2 Sets)
        * Area 51/Maximum Force Duo (2 Sets)
        * Vicious Circle
        * Fishin' Frenzy
        * Freeze

    To do:
        * map out unused RAM per-game via memory_nop_read/write

    Note: There is believed to be a 68020 version of Maximum Force
            (not confirmed or dumped)

****************************************************************************

    Area51/Maximum Force (c)1997 Atari Games
    Maximum Force
    A055451

    Components:
    sdt79r3041-20j
    Atari Jaguar CPU V1.0 6sc880hf106
    Atari Jaguar DSP V1.0 sc414201ft (has Motorolla logo)
    Altera epm7128elc84-15 marked A-21652
    VIA vt83c461 IDE controller
    Actel a1010b marked A-22096 near IDE and gun inputs
    Dallas ds1232s watchdog
    52MHz osc near Altera PLCC
    40MHz osc near 79R3041
    14.318180MHz osc near Jag DSP
    12x hm514260cj7 RAM (near Jaguar CPU/DSP)
    4x  sdt71256 RAM (near Boot ROMs's)
    Atmel atf16v8b marked a-21647 (near Jag CPU)
    Altera ep22lc-10 marked A-21648 (near Jag DSP)
    ICT 22cv10aj marked A-21649 (near Jag CPU)
    ICT 22cv10aj marked A-21650 (near Jag CPU)
    ICT 22cv10aj marked A-21651 (near Jag CPU)
    tea6320t
    AKM ak4310vm
    tda1554q amplifier
    Microchip 28c16a-15 BRAM

    ROM's:
    27c4001
    R3K MAX/A51 KIT
    LL
    (c)1997 Atari
    V 1.0

    27c4001
    R3K MAX/A51 KIT
    LH
    (c)1997 Atari
    V 1.0

    27c4001
    R3K MAX/A51 KIT
    HL
    (c)1997 Atari
    V 1.0

    27c4001
    R3K MAX/A51 KIT
    HH
    (c)1997 Atari
    V 1.0

    Jumpers:
    jsp1 (1/2 connected= w/sub 2/3 connected= normal speaker)
    jsp2 (1/2 connected= w/sub 2/3 connected= normal speaker)
    jamaud (1/2 connected=stereo 2/3 connected=mono)
    jimpr (1/2 connected=hi video R impedance 2/3 connected=lo)
    jimpg (1/2 connected=hi video G impedance 2/3 connected=lo)
    jimpb (1/2 connected=hi video B impedance 2/3 connected=lo)

    Connectors:
    idea  standard IDE connector
    ideb laptop size IDE connector
    jgun1 8 pin gun input
    jgun2 8 pin gun input
    xtracoin 1 6 pin (coin3/4 bills?)
    jvupdn 3 pin (?)
    jsync 3 pin (?)
    JAMMA
    jspkr left/right/subwoofer output
    hdpower 4 pin PC power connector for HD

****************************************************************************

Area 51 (Time Warner Interactive License)
Atari/Mesa Logic, 1995

This game runs on Atari Cojag 68k hardware.

PCB Layouts
-----------

Main Board:

COJAG A053538
ATARI GAMES (C) 1994
(Atari comment near AMP - 'MMM, DONUTS.')
|--------------------------------------------------|
|TDA1554Q  TEA6320T   AK4310            JXCLKTRM   |
|JSPKR     ST91D314                        XC7336  |
|         JSP1 JWELLSR  PAL4  PAL5                 |
|   JSP4  JSP2 PAL1            45160 45160  *   *  |
| HDPOWER JSP3 JWELLSB                        JXBUS|
| SELFTEST     PAL2     PAL6   45160 45160  *   *  |
|     LED      JWELLSG                             |
|     LED              |------|45160 45160  *   *  |
|J    LED              |JAGUAR|                    |
|A                     | CPU  |45160 45160  *   *  |
|M                     |      |                    |
|M                     |------|                    |
|A                                                 |
|   14.31818MHz        |------|                    |
|   LM613              |JAGUAR|                    |
|   52MHz  PAL3        | DSP  |                    |
|                      |      |                    |
|  JSYNC               |------|                    |
|  JVUPDN           DIP8                           |
|              A1010B                              |
|JPLY3   JVCR            VT83C461                  |
|                                                  |
|                               LED                |
|  JPLY4    JGUN2  JGUN1  JIDEB                    |
|--------------------------------------------------|
Notes:
      JAGUAR CPU - Atari Jaguar CPU (QFP208)
      JAGUAR DSP - Atari Jaguar DSP (QFP160)
      45160      - TMS45160DZ-70 512K x16 DRAM (SOJ40)
                   Note: This RAM is in banks. There are 4 banks, each bank is 2MBytes x 64bit
                   Only banks 0 and 1 are populated.
      *          - Unpopulated DRAM positions (banks 2 and 3)
      TDA1554Q   - Philips TDA1554Q power AMP
      TEA6320T   - Philips TEA6320T op AMP (SOP32)
      AK4310     - AKM AK4310VM (SOIC24)
      ST91D314   - STS Microelectronics ST91D314 3403 op AMP (SOIC14)
      PAL1       - ATMEL ATF16V8B PAL (labelled '136105-0006', PLCC20)
      PAL2       - ATMEL ATF16V8B PAL (labelled '136105-0006', PLCC20)
      PAL3       - ATMEL ATF16V8B PAL (labelled '136105-0007', PLCC20)
      PAL4, PAL5 - Philips PL22V10-10A PAL (labelled 'MYF 136105-0008', PLCC28)
      PAL6       - ATMEL ATF16V8B (labelled '136105-0005', PLCC20)
      A1010B     - ACTEL A1010B Complex Programmable Logic Device (labeled 'MYF 136105-0010', PLCC44)
      XC7336     - Xilinx XC7336 Complex PRogrammable Logic Device (labelled 'MYF 136105-0009', PLCC44)
      VT83C461   - VIA VT83C461 IDE Hard Drive Controller (QFP100)
      LM613      - (DIP16)
      SELFTEST   - Test Switch
      HDPOWER    - Standard (PC-type) 4 pin hard drive power connector
      JSP1       - 3 pin jumper block, set to 2-3
      JSP2       - 3 pin jumper block, set to 2-3
      JSP3       - 3 pin jumper block, jumper not installed
      JSP4       - 3 pin jumper block, set to 2-3
      JWELLSR    - 3 pin jumper block, set to 2-3
      JWELLSB    - 3 pin jumper block, set to 2-3
      JWELLSG    - 3 pin jumper block, set to 2-3
      JSYNC      - 3 pin jumper block, jumper not installed
      JVUPDN     - 3 pin jumper block, jumper not installed
      JVCR       - 3 pin jumper block, jumper not installed
      JXCLKTRM   - 2 pin header, not shorted
      JPLY3      - Connector for player 3 controls
      JPLY4      - Connector for player 4 controls
      JGUN1      - Connector for player 1 gun
      JGUN2      - Connector for player 2 gun
      JIDEB      - Connector for 40 pin IDE cable connected to Quantum Fireball 1080AT IDE hard drive (C/H/S = 2112/16/63)
      JSPKR      - Connector for left/right speaker for stereo sound output
      DIP8       - Unpopulated DIP8 socket
      JXBUS      - 96 pin connector to top board


Top Board:

EC20X32
A053448
ATARI GAMES (C) 1994
|------------------------------|
|               136105-0002C.3P|
| 71256                        |
|               136105-0001C.3M|
| 71256                        |
|               136105-0000C.3K|
| 71256                        |
|               136105-0003C.3H|
| 71256                        |
|              LED             |
|                              |
| AT28C16  DS1232   XC7354     |
|                              |
|                              |
|      MC68EC020               |
|                              |
| 50MHz                        |
|------------------------------|
Notes:
      MC68EC020       - Motorola 68EC020FG25 CPU clocked at 25MHz (QFP100)
      AT28C16         - ATMEL 2k x8 EEPROM (DIP24)
      XC7354          - Xilinx XC7354 Complex Programmable Logic Device (labelled 'MYF 136105-0004', PLCC68, socketed)
      DS1232          - Dallas DS1232 System Reset IC (DIP8)
      71256           - 32K x8 SRAM (SOJ28)
      136105-0002C.3P - 27C040 EPROM (labelled 'AREA 51 136105-0002C (C)1995 ATARI GMS CS 55FE', DIP32)
      136105-0001C.3M - 27C040 EPROM (labelled 'AREA 51 136105-0001C (C)1995 ATARI GMS CS 3DFD', DIP32)
      136105-0000C.3K - 27C040 EPROM (labelled 'AREA 51 136105-0000C (C)1995 ATARI GMS CS 63FC', DIP32)
      136105-0003C.3H - 27C040 EPROM (labelled 'AREA 51 136105-0003C (C)1995 ATARI GMS CS 45FF', DIP32)

****************************************************************************

Maximum Force
Atari, 1997

PCB Layout
----------

MAXIMUM FORCE A055451 ATARI GAMES (C) 1996
|-----------------------------------------------------------------------------------|
|HM514260CJ7 HM514260CJ7 HM514260CJ7 HM514260CJ7            AT28C16                 |
|                                                                                   |
|HM514260CJ7 HM514260CJ7 HM514260CJ7 HM514260CJ7            PROGLL.17Y   PROGLH.21Y |
|                                                                                   |
|HM514260CJ7 HM514260CJ7 HM514260CJ7 HM514260CJ7            PROGHL.21V   PROGHH.21V |
|                                                                                   |
|                                                                                   |
|             |------|      |------|                                                |
|             |JAGUAR|      |JAGUAR|                                                |
|GAL     GAL  |GPU   |      |DSP   |                     |----------|               |
|             |------|      |------|                     |          |               |
|                                                        |IDT       |               |
|GAL     GAL                                     40MHz   |79R3041-20|               |
|                                          GAL           |          |               |
|                                    14.31818MHz         |----------|           IDE |
|AK4310VM                                                                      CONN |
|                                                       |-----------|               |
|LM78L05                                     52MHz      |ALTERA MAX |               |
|                                                       |EPM7128ELC84               |
|        TEA6320                                        |           |               |
|78L09                                    LM613         |           |      VIA      |
|            MONO/STEREO                                |-----------|      VT83C461 |
|TDA1554           SELFTEST                                                         |
|                                                  DS1232      ACTEL_A1010B         |
|           HDD_PWR                                                                 |
| JSPKR |--|           JAMMA            |--| JSYNC  JVUPDN   XTRACOIN1  JGUN1  JGUN2|
|-------|  |----------------------------|  |----------------------------------------|
Notes:
      JGUN1 / JGUN2 - Connector for Guns
      VIA VT83C461 - IDE Controller
      JSPKR - Speaker Output Connector (for use with Stereo jumper)

****************************************************************************

    Memory map (TBA)

    ========================================================================
    MAIN CPU
    ========================================================================

    ------------------------------------------------------------
    000000-3FFFFF   R/W   xxxxxxxx xxxxxxxx   DRAM 0
    400000-7FFFFF   R/W   xxxxxxxx xxxxxxxx   DRAM 1
    800000-BFFFFF   R     xxxxxxxx xxxxxxxx   Graphic ROM bank
    C00000-DFFFFF   R     xxxxxxxx xxxxxxxx   Sound ROM bank
    F00000-F000FF   R/W   xxxxxxxx xxxxxxxx   Tom Internal Registers
    F00400-F005FF   R/W   xxxxxxxx xxxxxxxx   CLUT - color lookup table A
    F00600-F007FF   R/W   xxxxxxxx xxxxxxxx   CLUT - color lookup table B
    F00800-F00D9F   R/W   xxxxxxxx xxxxxxxx   LBUF - line buffer A
    F01000-F0159F   R/W   xxxxxxxx xxxxxxxx   LBUF - line buffer B
    F01800-F01D9F   R/W   xxxxxxxx xxxxxxxx   LBUF - line buffer currently selected
    F02000-F021FF   R/W   xxxxxxxx xxxxxxxx   GPU control registers
    F02200-F022FF   R/W   xxxxxxxx xxxxxxxx   Blitter registers
    F03000-F03FFF   R/W   xxxxxxxx xxxxxxxx   Local GPU RAM
    F08800-F08D9F   R/W   xxxxxxxx xxxxxxxx   LBUF - 32-bit access to line buffer A
    F09000-F0959F   R/W   xxxxxxxx xxxxxxxx   LBUF - 32-bit access to line buffer B
    F09800-F09D9F   R/W   xxxxxxxx xxxxxxxx   LBUF - 32-bit access to line buffer currently selected
    F0B000-F0BFFF   R/W   xxxxxxxx xxxxxxxx   32-bit access to local GPU RAM
    F10000-F13FFF   R/W   xxxxxxxx xxxxxxxx   Jerry
    F14000-F17FFF   R/W   xxxxxxxx xxxxxxxx   Joysticks and GPIO0-5
    F18000-F1AFFF   R/W   xxxxxxxx xxxxxxxx   Jerry DSP
    F1B000-F1CFFF   R/W   xxxxxxxx xxxxxxxx   Local DSP RAM
    F1D000-F1DFFF   R     xxxxxxxx xxxxxxxx   Wavetable ROM
    ------------------------------------------------------------

***************************************************************************/


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/mips/r3000.h"
#include "cpu/jaguar/jaguar.h"
#include "machine/idectrl.h"
#include "sound/dac.h"
#include "includes/jaguar.h"


#define JAGUAR_CLOCK		XTAL_52MHz
#define R3000_CLOCK			XTAL_40MHz
#define M68K_CLOCK			XTAL_50MHz



/*************************************
 *
 *  Global variables
 *
 *************************************/

UINT32 *jaguar_shared_ram;
UINT32 *jaguar_gpu_ram;
UINT32 *jaguar_gpu_clut;
UINT32 *jaguar_dsp_ram;
UINT32 *jaguar_wave_rom;
UINT8 cojag_is_r3000;



/*************************************
 *
 *  Local variables
 *
 *************************************/

static cpu_device *main_cpu;

static UINT32 misc_control_data;
static UINT8 eeprom_enable;

static UINT32 *rom_base;



/*************************************
 *
 *  Machine init
 *
 *************************************/

static MACHINE_RESET( cojag )
{
	UINT8 *rom = memory_region(machine, "user2");

	/* 68020 only: copy the interrupt vectors into RAM */
	if (!cojag_is_r3000)
		memcpy(jaguar_shared_ram, rom_base, 0x10);

	/* configure banks for gfx/sound ROMs */
	if (rom)
	{
		/* graphics banks */
		if (cojag_is_r3000)
		{
			memory_configure_bank(machine, "bank1", 0, 2, rom + 0x800000, 0x400000);
			memory_set_bank(machine, "bank1", 0);
		}
		memory_configure_bank(machine, "bank8", 0, 2, rom + 0x800000, 0x400000);
		memory_set_bank(machine, "bank8", 0);

		/* sound banks */
		memory_configure_bank(machine, "bank2", 0, 8, rom + 0x000000, 0x200000);
		memory_configure_bank(machine, "bank9", 0, 8, rom + 0x000000, 0x200000);
		memory_set_bank(machine, "bank2", 0);
		memory_set_bank(machine, "bank9", 0);
	}

	/* clear any spinuntil stuff */
	jaguar_gpu_resume(machine);
	jaguar_dsp_resume(machine);

	/* halt the CPUs */
	jaguargpu_ctrl_w(machine->device("gpu"), G_CTRL, 0, 0xffffffff);
	jaguardsp_ctrl_w(machine->device("audiocpu"), D_CTRL, 0, 0xffffffff);

	/* set blitter idle flag */
	blitter_status = 1;
}



/*************************************
 *
 *  Misc. control bits
 *
 *************************************/

static READ32_HANDLER( misc_control_r )
{
	/*  D7    = board reset (low)
        D6    = audio must & reset (high)
        D5    = volume control data (invert on write)
        D4    = volume control clock
        D3-D1 = audio bank 2-0
        D0    = shared memory select (0=XBUS) */

	return misc_control_data ^ 0x20;
}


static WRITE32_HANDLER( misc_control_w )
{
	logerror("%08X:misc_control_w(%02X)\n", cpu_get_previouspc(space->cpu), data);

	/*  D7    = board reset (low)
        D6    = audio must & reset (high)
        D5    = volume control data (invert on write)
        D4    = volume control clock
        D3-D1 = audio bank 2-0
        D0    = shared memory select (0=XBUS) */

	/* handle resetting the DSPs */
	if (!(data & 0x80))
	{
		/* clear any spinuntil stuff */
		jaguar_gpu_resume(space->machine);
		jaguar_dsp_resume(space->machine);

		/* halt the CPUs */
		jaguargpu_ctrl_w(space->machine->device("gpu"), G_CTRL, 0, 0xffffffff);
		jaguardsp_ctrl_w(space->machine->device("audiocpu"), D_CTRL, 0, 0xffffffff);
	}

	/* adjust banking */
	if (memory_region(space->machine, "user2"))
	{
		memory_set_bank(space->machine, "bank2", (data >> 1) & 7);
		memory_set_bank(space->machine, "bank9", (data >> 1) & 7);
	}

	COMBINE_DATA(&misc_control_data);
}



/*************************************
 *
 *  32-bit access to the GPU
 *
 *************************************/

static READ32_HANDLER( gpuctrl_r )
{
	return jaguargpu_ctrl_r(space->machine->device("gpu"), offset);
}


static WRITE32_HANDLER( gpuctrl_w )
{
	jaguargpu_ctrl_w(space->machine->device("gpu"), offset, data, mem_mask);
}



/*************************************
 *
 *  32-bit access to the DSP
 *
 *************************************/

static READ32_HANDLER( dspctrl_r )
{
	return jaguardsp_ctrl_r(space->machine->device("audiocpu"), offset);
}


static WRITE32_HANDLER( dspctrl_w )
{
	jaguardsp_ctrl_w(space->machine->device("audiocpu"), offset, data, mem_mask);
}


static READ32_HANDLER( jaguar_wave_rom_r )
{
	return jaguar_wave_rom[offset];
}



/*************************************
 *
 *  Output ports
 *
 *************************************/

static WRITE32_HANDLER( latch_w )
{
	logerror("%08X:latch_w(%X)\n", cpu_get_previouspc(space->cpu), data);

	/* adjust banking */
	if (memory_region(space->machine, "user2"))
	{
		if (cojag_is_r3000)
			memory_set_bank(space->machine, "bank1", data & 1);
		memory_set_bank(space->machine, "bank8", data & 1);
	}
}



/*************************************
 *
 *  EEPROM access
 *
 *************************************/

static READ32_HANDLER( eeprom_data_r )
{
	if (cojag_is_r3000)
		return space->machine->generic.nvram.u32[offset] | 0xffffff00;
	else
		return space->machine->generic.nvram.u32[offset] | 0x00ffffff;
}


static WRITE32_HANDLER( eeprom_enable_w )
{
	eeprom_enable = 1;
}


static WRITE32_HANDLER( eeprom_data_w )
{
//  if (eeprom_enable)
	{
		if (cojag_is_r3000)
			space->machine->generic.nvram.u32[offset] = data & 0x000000ff;
		else
			space->machine->generic.nvram.u32[offset] = data & 0xff000000;
	}
//  else
//      logerror("%08X:error writing to disabled EEPROM\n", cpu_get_previouspc(space->cpu));
	eeprom_enable = 0;
}



/*************************************
 *
 *  GPU synchronization & speedup
 *
 *************************************/

/*
    Explanation:

    The GPU generally sits in a tight loop waiting for the main CPU to store
    a jump address into a specific memory location. This speedup is designed
    to catch that loop, which looks like this:

        load    (r28),r21
        jump    (r21)
        nop

    When nothing is pending, the GPU keeps the address of the load instruction
    at (r28) so that it loops back on itself. When the main CPU wants to execute
    a command, it stores an alternate address to (r28).

    Even if we don't optimize this case, we do need to detect when a command
    is written to the GPU in order to improve synchronization until the GPU
    has finished. To do this, we start a temporary high frequency timer and
    run it until we get back to the spin loop.
*/

static UINT32 *gpu_jump_address;
static UINT8 gpu_command_pending;
static UINT32 gpu_spin_pc;

static TIMER_CALLBACK( gpu_sync_timer )
{
	/* if a command is still pending, and we haven't maxed out our timer, set a new one */
	if (gpu_command_pending && param < 1000)
		timer_set(machine, ATTOTIME_IN_USEC(50), NULL, ++param, gpu_sync_timer);
}


static WRITE32_HANDLER( gpu_jump_w )
{
	/* update the data in memory */
	COMBINE_DATA(gpu_jump_address);
	logerror("%08X:GPU jump address = %08X\n", cpu_get_previouspc(space->cpu), *gpu_jump_address);

	/* if the GPU is suspended, release it now */
	jaguar_gpu_resume(space->machine);

	/* start the sync timer going, and note that there is a command pending */
	timer_call_after_resynch(space->machine, NULL, 0, gpu_sync_timer);
	gpu_command_pending = 1;
}


static READ32_HANDLER( gpu_jump_r )
{
	/* if the current GPU command is just pointing back to the spin loop, and */
	/* we're reading it from the spin loop, we can optimize */
	if (*gpu_jump_address == gpu_spin_pc && cpu_get_previouspc(space->cpu) == gpu_spin_pc)
	{
#if ENABLE_SPEEDUP_HACKS
		/* spin if we're allowed */
		jaguar_gpu_suspend(space->machine);
#endif

		/* no command is pending */
		gpu_command_pending = 0;
	}

	/* return the current value */
	return *gpu_jump_address;
}



/*************************************
 *
 *  Main CPU speedup (R3000 games)
 *
 *************************************/

/*
    Explanation:

    Instead of sitting in a tight loop, the CPU will run the random number
    generator over and over while waiting for an interrupt. In order to catch
    that, we snoop the memory location it is polling, and see if it is read
    at least 5 times in a row, each time less than 200 cycles apart. If so,
    we assume it is spinning. Also, by waiting for 5 iterations, we let it
    crank through some random numbers, just not several thousand every frame.
*/

#if ENABLE_SPEEDUP_HACKS

static UINT32 *main_speedup;
static int main_speedup_hits;
static UINT64 main_speedup_last_cycles;
static UINT64 main_speedup_max_cycles;

static READ32_HANDLER( cojagr3k_main_speedup_r )
{
	UINT64 curcycles = main_cpu->total_cycles();

	/* if it's been less than main_speedup_max_cycles cycles since the last time */
	if (curcycles - main_speedup_last_cycles < main_speedup_max_cycles)
	{
		/* increment the count; if we hit 5, we can spin until an interrupt comes */
		if (main_speedup_hits++ > 5)
		{
			cpu_spinuntil_int(space->cpu);
			main_speedup_hits = 0;
		}
	}

	/* if it's been more than main_speedup_max_cycles cycles, reset our count */
	else
		main_speedup_hits = 0;

	/* remember the last cycle count */
	main_speedup_last_cycles = curcycles;

	/* return the real value */
	return *main_speedup;
}

#endif



/*************************************
 *
 *  Additional main CPU speedup
 *  (Freeze only)
 *
 *************************************/

/*
    Explanation:

    The main CPU hands data off to the GPU to process. But rather than running
    in parallel, the main CPU just sits and waits for the result. This speedup
    makes sure we don't waste time emulating that spin loop.
*/

#if ENABLE_SPEEDUP_HACKS

static UINT32 *main_gpu_wait;

static READ32_HANDLER( main_gpu_wait_r )
{
	if (gpu_command_pending)
		cpu_spinuntil_int(space->cpu);
	return *main_gpu_wait;
}

#endif



/*************************************
 *
 *  Main CPU speedup (Area 51)
 *
 *************************************/

/*
    Explanation:

    Very similar to the R3000 code, except we need to verify that the value in
    *main_speedup is actually 0.
*/

#if ENABLE_SPEEDUP_HACKS

static WRITE32_HANDLER( area51_main_speedup_w )
{
	UINT64 curcycles = main_cpu->total_cycles();

	/* store the data */
	COMBINE_DATA(main_speedup);

	/* if it's been less than 400 cycles since the last time */
	if (*main_speedup == 0 && curcycles - main_speedup_last_cycles < 400)
	{
		/* increment the count; if we hit 5, we can spin until an interrupt comes */
		if (main_speedup_hits++ > 5)
		{
			cpu_spinuntil_int(space->cpu);
			main_speedup_hits = 0;
		}
	}

	/* if it's been more than 400 cycles, reset our count */
	else
		main_speedup_hits = 0;

	/* remember the last cycle count */
	main_speedup_last_cycles = curcycles;
}


/*
    Explanation:

    The Area 51/Maximum Force duo writes to a non-aligned address, so our check
    against 0 must handle that explicitly.
*/

static WRITE32_HANDLER( area51mx_main_speedup_w )
{
	UINT64 curcycles = main_cpu->total_cycles();

	/* store the data */
	COMBINE_DATA(&main_speedup[offset]);

	/* if it's been less than 450 cycles since the last time */
	if (((main_speedup[0] << 16) | (main_speedup[1] >> 16)) == 0 && curcycles - main_speedup_last_cycles < 450)
	{
		/* increment the count; if we hit 5, we can spin until an interrupt comes */
		if (main_speedup_hits++ > 10)
		{
			cpu_spinuntil_int(space->cpu);
			main_speedup_hits = 0;
		}
	}

	/* if it's been more than 450 cycles, reset our count */
	else
		main_speedup_hits = 0;

	/* remember the last cycle count */
	main_speedup_last_cycles = curcycles;
}

#endif



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( r3000_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x04000000, 0x047fffff) AM_RAM AM_BASE(&jaguar_shared_ram) AM_SHARE("share1")
	AM_RANGE(0x04800000, 0x04bfffff) AM_ROMBANK("bank1")
	AM_RANGE(0x04c00000, 0x04dfffff) AM_ROMBANK("bank2")
	AM_RANGE(0x04e00000, 0x04e003ff) AM_DEVREADWRITE("ide", ide_controller32_r, ide_controller32_w)
	AM_RANGE(0x04f00000, 0x04f003ff) AM_READWRITE(jaguar_tom_regs32_r, jaguar_tom_regs32_w)
	AM_RANGE(0x04f00400, 0x04f007ff) AM_RAM AM_BASE(&jaguar_gpu_clut) AM_SHARE("share2")
	AM_RANGE(0x04f02100, 0x04f021ff) AM_READWRITE(gpuctrl_r, gpuctrl_w)
	AM_RANGE(0x04f02200, 0x04f022ff) AM_READWRITE(jaguar_blitter_r, jaguar_blitter_w)
	AM_RANGE(0x04f03000, 0x04f03fff) AM_MIRROR(0x00008000) AM_RAM AM_BASE(&jaguar_gpu_ram) AM_SHARE("share3")
	AM_RANGE(0x04f10000, 0x04f103ff) AM_READWRITE(jaguar_jerry_regs32_r, jaguar_jerry_regs32_w)
	AM_RANGE(0x04f16000, 0x04f1600b) AM_READ(cojag_gun_input_r)	// GPI02
	AM_RANGE(0x04f17000, 0x04f17003) AM_READ_PORT("SYSTEM")		// GPI03
	AM_RANGE(0x04f17800, 0x04f17803) AM_WRITE(latch_w)			// GPI04
	AM_RANGE(0x04f17c00, 0x04f17c03) AM_READ_PORT("P1_P2")		// GPI05
	AM_RANGE(0x04f1a100, 0x04f1a13f) AM_READWRITE(dspctrl_r, dspctrl_w)
	AM_RANGE(0x04f1a140, 0x04f1a17f) AM_READWRITE(jaguar_serial_r, jaguar_serial_w)
	AM_RANGE(0x04f1b000, 0x04f1cfff) AM_RAM AM_BASE(&jaguar_dsp_ram) AM_SHARE("share4")

	AM_RANGE(0x06000000, 0x06000003) AM_READWRITE(misc_control_r, misc_control_w)
	AM_RANGE(0x10000000, 0x1007ffff) AM_RAM
	AM_RANGE(0x12000000, 0x120fffff) AM_RAM		// tested in self-test only?
	AM_RANGE(0x14000004, 0x14000007) AM_WRITE(watchdog_reset32_w)
	AM_RANGE(0x16000000, 0x16000003) AM_WRITE(eeprom_enable_w)
	AM_RANGE(0x18000000, 0x18001fff) AM_READWRITE(eeprom_data_r, eeprom_data_w) AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE(0x1fc00000, 0x1fdfffff) AM_ROM AM_REGION("user1", 0) AM_BASE(&rom_base)
ADDRESS_MAP_END


static ADDRESS_MAP_START( m68020_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x000000, 0x7fffff) AM_RAM AM_BASE(&jaguar_shared_ram) AM_SHARE("share1")
	AM_RANGE(0x800000, 0x9fffff) AM_ROM AM_REGION("user1", 0) AM_BASE(&rom_base)
	AM_RANGE(0xa00000, 0xa1ffff) AM_RAM
	AM_RANGE(0xa20000, 0xa21fff) AM_READWRITE(eeprom_data_r, eeprom_data_w) AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE(0xa30000, 0xa30003) AM_WRITE(watchdog_reset32_w)
	AM_RANGE(0xa40000, 0xa40003) AM_WRITE(eeprom_enable_w)
	AM_RANGE(0xb70000, 0xb70003) AM_READWRITE(misc_control_r, misc_control_w)
	AM_RANGE(0xc00000, 0xdfffff) AM_ROMBANK("bank2")
	AM_RANGE(0xe00000, 0xe003ff) AM_DEVREADWRITE("ide",  ide_controller32_r, ide_controller32_w)
	AM_RANGE(0xf00000, 0xf003ff) AM_READWRITE(jaguar_tom_regs32_r, jaguar_tom_regs32_w)
	AM_RANGE(0xf00400, 0xf007ff) AM_RAM AM_BASE(&jaguar_gpu_clut) AM_SHARE("share2")
	AM_RANGE(0xf02100, 0xf021ff) AM_READWRITE(gpuctrl_r, gpuctrl_w)
	AM_RANGE(0xf02200, 0xf022ff) AM_READWRITE(jaguar_blitter_r, jaguar_blitter_w)
	AM_RANGE(0xf03000, 0xf03fff) AM_MIRROR(0x008000) AM_RAM AM_BASE(&jaguar_gpu_ram) AM_SHARE("share3")
	AM_RANGE(0xf10000, 0xf103ff) AM_READWRITE(jaguar_jerry_regs32_r, jaguar_jerry_regs32_w)
	AM_RANGE(0xf16000, 0xf1600b) AM_READ(cojag_gun_input_r)	// GPI02
	AM_RANGE(0xf17000, 0xf17003) AM_READ_PORT("SYSTEM")		// GPI03
//  AM_RANGE(0xf17800, 0xf17803) AM_WRITE(latch_w)          // GPI04
	AM_RANGE(0xf17c00, 0xf17c03) AM_READ_PORT("P1_P2")		// GPI05
	AM_RANGE(0xf1a100, 0xf1a13f) AM_READWRITE(dspctrl_r, dspctrl_w)
	AM_RANGE(0xf1a140, 0xf1a17f) AM_READWRITE(jaguar_serial_r, jaguar_serial_w)
	AM_RANGE(0xf1b000, 0xf1cfff) AM_RAM AM_BASE(&jaguar_dsp_ram) AM_SHARE("share4")
ADDRESS_MAP_END



/*************************************
 *
 *  GPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( gpu_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x000000, 0x7fffff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x800000, 0xbfffff) AM_ROMBANK("bank8")
	AM_RANGE(0xc00000, 0xdfffff) AM_ROMBANK("bank9")
	AM_RANGE(0xe00000, 0xe003ff) AM_DEVREADWRITE("ide", ide_controller32_r, ide_controller32_w)
	AM_RANGE(0xf00000, 0xf003ff) AM_READWRITE(jaguar_tom_regs32_r, jaguar_tom_regs32_w)
	AM_RANGE(0xf00400, 0xf007ff) AM_RAM AM_SHARE("share2")
	AM_RANGE(0xf02100, 0xf021ff) AM_READWRITE(gpuctrl_r, gpuctrl_w)
	AM_RANGE(0xf02200, 0xf022ff) AM_READWRITE(jaguar_blitter_r, jaguar_blitter_w)
	AM_RANGE(0xf03000, 0xf03fff) AM_RAM AM_SHARE("share3")
	AM_RANGE(0xf10000, 0xf103ff) AM_READWRITE(jaguar_jerry_regs32_r, jaguar_jerry_regs32_w)
ADDRESS_MAP_END



/*************************************
 *
 *  DSP memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( dsp_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x000000, 0x7fffff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x800000, 0xbfffff) AM_ROMBANK("bank8")
	AM_RANGE(0xc00000, 0xdfffff) AM_ROMBANK("bank9")
	AM_RANGE(0xf10000, 0xf103ff) AM_READWRITE(jaguar_jerry_regs32_r, jaguar_jerry_regs32_w)
	AM_RANGE(0xf1a100, 0xf1a13f) AM_READWRITE(dspctrl_r, dspctrl_w)
	AM_RANGE(0xf1a140, 0xf1a17f) AM_READWRITE(jaguar_serial_r, jaguar_serial_w)
	AM_RANGE(0xf1b000, 0xf1cfff) AM_RAM AM_SHARE("share4")
	AM_RANGE(0xf1d000, 0xf1dfff) AM_READ(jaguar_wave_rom_r) AM_BASE(&jaguar_wave_rom)
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

/* "FAKE0" is read at 0x04f17000
    D23-20 = /SER-4-1
    D19-16 = COINR4-1
    D7     = /VSYNCNEQ
    D6     = /S-TEST
    D5     = /VOLUMEUP
    D4     = /VOLUMEDOWN
    D3-D0  = ACTC4-1
*/
static INPUT_PORTS_START( area51 )
	PORT_START("P1_P2")
	PORT_BIT( 0x000000ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0000fe00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_BIT( 0x00ff0000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0xfe000000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(custom_port_read, "FAKE0")
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(custom_port_read, "FAKE0")

	PORT_START("FAKE0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_SERVICE( 0x0040, IP_ACTIVE_LOW )			// s-test
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_SPECIAL )	// vsyncneq
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("FAKE1_X")				/* fake analog X */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 320.0/(320.0 - 7 -7), 0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10)

	PORT_START("FAKE1_Y")				/* fake analog Y */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, (240.0 - 1)/240, 0.0, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(10)

	PORT_START("FAKE2_X")				/* fake analog X */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 320.0/(320.0 - 7 -7), 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("FAKE2_Y")				/* fake analog Y */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, (240.0 - 1)/240, 0.0, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("IN3")			/* gun triggers */
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_SPECIAL )	// gun data valid
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_SPECIAL )	// gun data valid
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0xfff00000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( freezeat )
	PORT_START("P1_P2")
	PORT_BIT( 0x000000ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)

	PORT_BIT( 0x00ff0000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)

	PORT_START("SYSTEM")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(custom_port_read, "FAKE0")
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(custom_port_read, "FAKE0")

	PORT_START("FAKE0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SPECIAL )	// volume down
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SPECIAL )	// volume up
	PORT_SERVICE( 0x0040, IP_ACTIVE_LOW )			// s-test
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_SPECIAL )	// vsyncneq
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x000f0000, IP_ACTIVE_HIGH, IPT_SPECIAL )	// coin returns
	PORT_BIT( 0x00f00000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xff000000, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( fishfren )
	PORT_START("P1_P2")
	PORT_BIT( 0x000000ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)

	PORT_BIT( 0x00ff0000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)

	PORT_START("SYSTEM")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(custom_port_read, "FAKE0")
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(custom_port_read, "FAKE0")

	PORT_START("FAKE0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SPECIAL )	// volume down
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SPECIAL )	// volume up
	PORT_SERVICE( 0x0040, IP_ACTIVE_LOW )			// s-test
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_SPECIAL )	// vsyncneq
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x000f0000, IP_ACTIVE_HIGH, IPT_SPECIAL )	// coin returns
	PORT_BIT( 0x00f00000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xff000000, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( vcircle )
	PORT_START("P1_P2")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0x000000f8, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)

	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x00f80000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)

	PORT_START("SYSTEM")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(custom_port_read, "FAKE0")
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(custom_port_read, "FAKE0")

	PORT_START("FAKE0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_SERVICE( 0x0040, IP_ACTIVE_LOW )			// s-test
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_SPECIAL )	// vsyncneq
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x000f0000, IP_ACTIVE_HIGH, IPT_SPECIAL )	// coin returns
	PORT_BIT( 0x00f00000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xff000000, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static const r3000_cpu_core config =
{
	0,		/* 1 if we have an FPU, 0 otherwise */
	4096,	/* code cache size */
	4096	/* data cache size */
};


static const jaguar_cpu_config gpu_config =
{
	jaguar_gpu_cpu_int
};


static const jaguar_cpu_config dsp_config =
{
	jaguar_dsp_cpu_int
};


static MACHINE_DRIVER_START( cojagr3k )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", R3041BE, R3000_CLOCK)
	MDRV_CPU_CONFIG(config)
	MDRV_CPU_PROGRAM_MAP(r3000_map)

	MDRV_CPU_ADD("gpu", JAGUARGPU, JAGUAR_CLOCK/2)
	MDRV_CPU_CONFIG(gpu_config)
	MDRV_CPU_PROGRAM_MAP(gpu_map)

	MDRV_CPU_ADD("audiocpu", JAGUARDSP, JAGUAR_CLOCK/2)
	MDRV_CPU_CONFIG(dsp_config)
	MDRV_CPU_PROGRAM_MAP(dsp_map)

	MDRV_MACHINE_RESET(cojag)
	MDRV_NVRAM_HANDLER(generic_1fill)

	MDRV_IDE_CONTROLLER_ADD("ide", jaguar_external_int)

	MDRV_TIMER_ADD("serial_timer", jaguar_serial_callback)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_RAW_PARAMS(COJAG_PIXEL_CLOCK/2, 456, 42, 402, 262, 17, 257)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)

	MDRV_VIDEO_START(cojag)
	MDRV_VIDEO_UPDATE(cojag)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("dac1", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)

	MDRV_SOUND_ADD("dac2", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( cojag68k )
	MDRV_IMPORT_FROM(cojagr3k)

	/* basic machine hardware */
	MDRV_CPU_REPLACE("maincpu", M68EC020, M68K_CLOCK/2)
	MDRV_CPU_PROGRAM_MAP(m68020_map)
MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definition(s)
 *
 *  Date Information comes from either
 *   ROM labels or from the Self-Test
 *   as "Main"
 *
 *************************************/

/****************************************

       ROM & Hard Disk based games

****************************************/

ROM_START( area51t ) /* 68020 based, Area51 Time Warner License  Date: Nov 15, 1995 */
	ROM_REGION32_BE( 0x200000, "user1", 0 )	/* 2MB for 68020 code */
	ROM_LOAD32_BYTE( "136105-0003c.3h", 0x00000, 0x80000, CRC(e70a97c4) SHA1(39dabf6bf3dc6f717a587f362d040bfb332be9e1) )
	ROM_LOAD32_BYTE( "136105-0002c.3p", 0x00001, 0x80000, CRC(e9c9f4bd) SHA1(7c6c50372d45dca8929767241b092339f3bab4d2) )
	ROM_LOAD32_BYTE( "136105-0001c.3m", 0x00002, 0x80000, CRC(6f135a81) SHA1(2d9660f240b14481e8c46bc98713e9dc12035063) )
	ROM_LOAD32_BYTE( "136105-0000c.3k", 0x00003, 0x80000, CRC(94f50c14) SHA1(a54552e3ac5c4f481ba4f2fc7d724534576fe76c) )

	DISK_REGION( "ide" )
	DISK_IMAGE( "area51t", 0, SHA1(d2865cc7b1bb08a4393a72013a90e18d8a8f9860) )
ROM_END

ROM_START( area51a ) /* 68020 based, Area51 Atari Games License  Date: Oct 25, 1995 */
	ROM_REGION32_BE( 0x200000, "user1", 0 )	/* 2MB for 68020 code */
	ROM_LOAD32_BYTE( "3h", 0x00000, 0x80000, CRC(116d37e6) SHA1(5d36cae792dd349faa77cd2d8018722a28ee55c1) )
	ROM_LOAD32_BYTE( "3p", 0x00001, 0x80000, CRC(eb10f539) SHA1(dadc4be5a442dd4bd17385033056555e528ed994) )
	ROM_LOAD32_BYTE( "3m", 0x00002, 0x80000, CRC(c6d8322b) SHA1(90cf848a4195c51b505653cc2c74a3b9e3c851b8) )
	ROM_LOAD32_BYTE( "3k", 0x00003, 0x80000, CRC(729eb1b7) SHA1(21864b4281b1ad17b2903e3aa294e4be74161e80) )

	DISK_REGION( "ide" )
	DISK_IMAGE( "area51", 0, SHA1(3b303bc37e206a6d7339352c869f050d04186f11) )
ROM_END

ROM_START( area51 ) /* R3000 based, labeled as "Area51 2-C"  Date: Nov 11 1996 */
	ROM_REGION32_BE( 0x200000, "user1", 0 )	/* 2MB for IDT 79R3041 code */
	ROM_LOAD32_BYTE( "a51_2-c.hh", 0x00000, 0x80000, CRC(13af6a1e) SHA1(69da54ed6886e825156bbcc256e8d7abd4dc1ff8) )
	ROM_LOAD32_BYTE( "a51_2-c.hl", 0x00001, 0x80000, CRC(8ab6649b) SHA1(9b4945bc04f8a73161638a2c5fa2fd84c6fd31b4) )
	ROM_LOAD32_BYTE( "a51_2-c.lh", 0x00002, 0x80000, CRC(a6524f73) SHA1(ae377a6803a4f7d1bbcc111725af121a3e82317d) )
	ROM_LOAD32_BYTE( "a51_2-c.ll", 0x00003, 0x80000, CRC(471b15d2) SHA1(4b5f45ee140b03a6be61475cae1c2dbef0f07457) )

	DISK_REGION( "ide" )
	DISK_IMAGE( "area51", 0, SHA1(3b303bc37e206a6d7339352c869f050d04186f11) )
ROM_END

ROM_START( maxforce ) /* R3000 based, labeled as "Maximum Force 5-23-97 v1.05" */
	ROM_REGION32_BE( 0x200000, "user1", 0 )	/* 2MB for IDT 79R3041 code */
	ROM_LOAD32_BYTE( "maxf_105.hh", 0x00000, 0x80000, CRC(ec7f8167) SHA1(0cf057bfb1f30c2c9621d3ed25021e7ba7bdd46e) )
	ROM_LOAD32_BYTE( "maxf_105.hl", 0x00001, 0x80000, CRC(3172611c) SHA1(00f14f871b737c66c20f95743740d964d0be3f24) )
	ROM_LOAD32_BYTE( "maxf_105.lh", 0x00002, 0x80000, CRC(84d49423) SHA1(88d9a6724f1118f2bbef5dfa27accc2b65c5ba1d) )
	ROM_LOAD32_BYTE( "maxf_105.ll", 0x00003, 0x80000, CRC(16d0768d) SHA1(665a6d7602a7f2f5b1f332b0220b1533143d56b1) )

	DISK_REGION( "ide" )
	DISK_IMAGE( "maxforce", 0, SHA1(d54e7a8f3866bb2a1d28ae637e7c92ffa4dbe558) )
ROM_END


ROM_START( maxf_102 ) /* R3000 based, labeled as "Maximum Force 2-27-97 v1.02" */
	ROM_REGION32_BE( 0x200000, "user1", 0 )	/* 2MB for IDT 79R3041 code */
	ROM_LOAD32_BYTE( "maxf_102.hh", 0x00000, 0x80000, CRC(8ff7009d) SHA1(da22eae298a6e0e36f503fa091ac3913423dcd0f) )
	ROM_LOAD32_BYTE( "maxf_102.hl", 0x00001, 0x80000, CRC(96c2cc1d) SHA1(b332b8c042b92c736131c478cefac1c3c2d2673b) )
	ROM_LOAD32_BYTE( "maxf_102.lh", 0x00002, 0x80000, CRC(459ffba5) SHA1(adb40db6904e84c17f32ac6518fd2e994da7883f) )
	ROM_LOAD32_BYTE( "maxf_102.ll", 0x00003, 0x80000, CRC(e491be7f) SHA1(cbe281c099a4aa87067752d68cf2bb0ab3900531) )

	DISK_REGION( "ide" )
	DISK_IMAGE( "maxforce", 0, SHA1(d54e7a8f3866bb2a1d28ae637e7c92ffa4dbe558) )
ROM_END


ROM_START( maxf_ng ) /* R3000 based, stickers say 'NO GORE' */
	ROM_REGION32_BE( 0x200000, "user1", 0 )	/* 2MB for IDT 79R3041 code */
	ROM_LOAD32_BYTE( "mf_ng_hh.21v", 0x00000, 0x80000, CRC(08791c02) SHA1(9befbff3201c7d345109b26c296fd8548dbfc95b) )
	ROM_LOAD32_BYTE( "mf_ng_hl.17v", 0x00001, 0x80000, CRC(52cf482c) SHA1(ff98b3f04987acef82a97a2ad35a9085fa84e6d5) )
	ROM_LOAD32_BYTE( "mf_ng_lh.21y", 0x00002, 0x80000, CRC(ab4ee992) SHA1(69f0fe111d3f5f31151d2922579e5073e484b1e1) )
	ROM_LOAD32_BYTE( "mf_ng_ll.17y", 0x00003, 0x80000, CRC(674aab43) SHA1(f79d790538756d1100b7e4ffed192a62a031a2cb) )

	ROM_REGION( 0x800, "user2", 0 ) /* 28C16 style eeprom, currently loaded but not used */
	ROM_LOAD( "28c16.17z", 0x000, 0x800, CRC(1cdd9088) SHA1(4f01f02ff95f31ced87a3cdd7f171afd92551266) )

	DISK_REGION( "ide" )
	DISK_IMAGE( "maxforce", 0, SHA1(d54e7a8f3866bb2a1d28ae637e7c92ffa4dbe558) )
ROM_END


ROM_START( area51mx )	/* 68020 based, Labeled as "68020 MAX/A51 KIT 2.0" Date: Apr 22, 1998 */
	ROM_REGION32_BE( 0x200000, "user1", 0 ) /* 2MB for 68020 code */
	ROM_LOAD32_BYTE( "area51mx.3h", 0x00000, 0x80000, CRC(47cbf30b) SHA1(23377bcc65c0fc330d5bc7e76e233bae043ac364) )
	ROM_LOAD32_BYTE( "area51mx.3p", 0x00001, 0x80000, CRC(a3c93684) SHA1(f6b3357bb69900a176fd6bc6b819b2f57b7d0f59) )
	ROM_LOAD32_BYTE( "area51mx.3m", 0x00002, 0x80000, CRC(d800ac17) SHA1(3d515c8608d8101ee9227116175b3c3f1fe22e0c) )
	ROM_LOAD32_BYTE( "area51mx.3k", 0x00003, 0x80000, CRC(0e78f308) SHA1(adc4c8e441eb8fe525d0a6220eb3a2a8791a7289) )

	DISK_REGION( "ide" )
	DISK_IMAGE( "area51mx", 0, SHA1(5ff10f4e87094d4449eabf3de7549564ca568c7e) )
ROM_END


ROM_START( a51mxr3k ) /* R3000 based, Labeled as "R3K Max/A51 Kit Ver 1.0" */
	ROM_REGION32_BE( 0x200000, "user1", 0 )	/* 2MB for IDT 79R3041 code */
	ROM_LOAD32_BYTE( "a51mxr3k.hh", 0x00000, 0x80000, CRC(a984dab2) SHA1(debb3bc11ff49e87a52e89a69533a1bab7db700e) )
	ROM_LOAD32_BYTE( "a51mxr3k.hl", 0x00001, 0x80000, CRC(0af49d74) SHA1(c19f26056a823fd32293e9a7b3ea868640eabf49) )
	ROM_LOAD32_BYTE( "a51mxr3k.lh", 0x00002, 0x80000, CRC(d7d94dac) SHA1(2060a74715f36a0d7f5dd0855eda48ad1f20f095) )
	ROM_LOAD32_BYTE( "a51mxr3k.ll", 0x00003, 0x80000, CRC(ece9e5ae) SHA1(7e44402726f5afa6d1670b27aa43ad13d21c4ad9) )

	DISK_REGION( "ide" )
	DISK_IMAGE( "area51mx", 0, SHA1(5ff10f4e87094d4449eabf3de7549564ca568c7e) )
ROM_END


ROM_START( vcircle )
	ROM_REGION32_BE( 0x200000, "user1", 0 )	/* 2MB for R3000 code */
	ROM_LOAD32_BYTE( "hh", 0x00000, 0x80000, CRC(7276f5f5) SHA1(716287e370a4f300b1743103f8031afc82de38ca) )
	ROM_LOAD32_BYTE( "hl", 0x00001, 0x80000, CRC(146060a1) SHA1(f291989f1f0ef228757f1990fb14da5ff8f3cf8d) )
	ROM_LOAD32_BYTE( "lh", 0x00002, 0x80000, CRC(be4b2ef6) SHA1(4332b3036e9cb12685e914d085d9a63aa856f0be) )
	ROM_LOAD32_BYTE( "ll", 0x00003, 0x80000, CRC(ba8753eb) SHA1(0322e0e37d814a38d08ba191b1a97fb1a55fe461) )

	DISK_REGION( "ide" )
	DISK_IMAGE( "vcircle", 0, SHA1(bfa79c4cacdc9c2cd6362f62a23056b3e35a2034) )
ROM_END



/****************************************

       ROM based games

****************************************/


ROM_START( fishfren )
	ROM_REGION32_BE( 0x200000, "user1", 0 )	/* 2MB for R3000 code */
	ROM_LOAD32_BYTE( "hh", 0x00000, 0x80000, CRC(2ef79767) SHA1(abcea584f2cbd71b05f9d7e61f40ca9da6799215) )
	ROM_LOAD32_BYTE( "hl", 0x00001, 0x80000, CRC(7eefd4a2) SHA1(181be04836704098082fd78cacc68ffa70e77892) )
	ROM_LOAD32_BYTE( "lh", 0x00002, 0x80000, CRC(bbe9ed15) SHA1(889af29afe6d984b39105aa238400392a5dfb2c5) )
	ROM_LOAD32_BYTE( "ll", 0x00003, 0x80000, CRC(d70d0f2c) SHA1(2689cbe56ae3d491348b241528b0fe345fa8484c) )

	ROM_REGION32_BE( 0x1000000, "user2", 0 )	/* 16MB for 64-bit ROM data */
	ROMX_LOAD( "l63-56", 0x000000, 0x100000, CRC(42764ea5) SHA1(805245f01006bd974fbac56f688cfcf137ddc914), ROM_SKIP(7) )
	ROMX_LOAD( "l55-48", 0x000001, 0x100000, CRC(0c7592bb) SHA1(d5bd6b872abad58947842205f9eac46fd065e88f), ROM_SKIP(7) )
	ROMX_LOAD( "l47-40", 0x000002, 0x100000, CRC(6d7dcdb1) SHA1(914dae3b9df5c861f794b683571c5fb0c2c3c3fd), ROM_SKIP(7) )
	ROMX_LOAD( "l39-32", 0x000003, 0x100000, CRC(ef3b8d98) SHA1(858c3342e9693bfe887b91dde1116a1656a1a105), ROM_SKIP(7) )
	ROMX_LOAD( "l31-24", 0x000004, 0x100000, CRC(132d628e) SHA1(3ff9fa86092eb01f21ca3ccf1ee1e3a583cbdecb), ROM_SKIP(7) )
	ROMX_LOAD( "l23-16", 0x000005, 0x100000, CRC(b841f039) SHA1(79f661aee009aef2f5ad4122ae3e0ac94097a427), ROM_SKIP(7) )
	ROMX_LOAD( "l15-08", 0x000006, 0x100000, CRC(0800214e) SHA1(5372f2c3470619a4967958c76055486f76b5f150), ROM_SKIP(7) )
	ROMX_LOAD( "l07-00", 0x000007, 0x100000, CRC(f83b2e78) SHA1(83ee9d2bfba83e04fb794270926bd3e558c9aaa4), ROM_SKIP(7) )
	ROMX_LOAD( "h63-56", 0x800000, 0x080000, CRC(67740765) SHA1(8b22413d25e0dbfe2227d1a8a023961a4c13cb76), ROM_SKIP(7) )
	ROMX_LOAD( "h55-48", 0x800001, 0x080000, CRC(ffed0091) SHA1(6c8104acd7e6d95a111f9c7a4d3b6984293d72c4), ROM_SKIP(7) )
	ROMX_LOAD( "h47-40", 0x800002, 0x080000, CRC(6f448f72) SHA1(3a298b9851e4ba7aa611aa6c2b0dcf06f4301463), ROM_SKIP(7) )
	ROMX_LOAD( "h39-32", 0x800003, 0x080000, CRC(25a5bd67) SHA1(79f29bd36afb4574b9c923eee293964284713540), ROM_SKIP(7) )
	ROMX_LOAD( "h31-24", 0x800004, 0x080000, CRC(e7088cc0) SHA1(4cb184de748c5633e669a4675e6db9920d34811e), ROM_SKIP(7) )
	ROMX_LOAD( "h23-16", 0x800005, 0x080000, CRC(ab477a76) SHA1(ae9aa97dbc758cd741710fe08c6ea94a0a318451), ROM_SKIP(7) )
	ROMX_LOAD( "h15-08", 0x800006, 0x080000, CRC(25a423f1) SHA1(7530cf2e28e0755bfcbd70789ef5cbbfb3d94f9f), ROM_SKIP(7) )
	ROMX_LOAD( "h07-00", 0x800007, 0x080000, CRC(0f5f4cc6) SHA1(caa2b514fb1f2a815e63f7b8c6b79ce2dfa308c4), ROM_SKIP(7) )
	ROM_COPY( "user2", 0x800000, 0xc00000, 0x400000 )
ROM_END

ROM_START( freezeat )
	ROM_REGION32_BE( 0x200000, "user1", 0 )	/* 2MB for R3000 code */
	ROM_LOAD32_BYTE( "prog_eng.hh",          0x000000, 0x040000, CRC(f7cffafd) SHA1(62369de4cf0a5abab86f6bcf9621028b9e171ec3) )
	ROM_LOAD32_BYTE( "prog_eng.hl",          0x000001, 0x040000, CRC(17150705) SHA1(c5a32d334bffb58a816920cc1251a21acc5a6f92) )
	ROM_LOAD32_BYTE( "prog_eng.lh",          0x000002, 0x040000, CRC(12a903bf) SHA1(41f5949d7ed2081917af8411f92666b754564b37) )
	ROM_LOAD32_BYTE( "prog_eng.ll",          0x000003, 0x040000, CRC(cf69f971) SHA1(132b06f5fb49801fff7e5deb7aa71b44d5b1c6ca) )

	ROM_REGION32_BE( 0x1000000, "user2", 0 )	/* 16MB for 64-bit ROM data */
	ROMX_LOAD( "fish_gr0.63-56", 0x000000, 0x100000, CRC(b61061c5) SHA1(aeb409aa5073232d80ed81b27946e753290234f4), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr0.55-48", 0x000001, 0x100000, CRC(c85acf42) SHA1(c3365caeb126a83a7e7afcda25f05849ceb5c98b), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr0.47-40", 0x000002, 0x100000, CRC(67f78f59) SHA1(40b256a8939fad365c7e896cff4a959fcc70a477), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr0.39-32", 0x000003, 0x100000, CRC(6be0508a) SHA1(20f617278ce1666348822d80686cecd8d9b1bc78), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr0.31-24", 0x000004, 0x100000, CRC(905606e0) SHA1(866cd98ea2399fed96f76b16dce751e2c7cfdc98), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr0.23-16", 0x000005, 0x100000, CRC(cdeef6fa) SHA1(1b4d58951b662040540e7d51f88c1b6f282562ee), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr0.15-08", 0x000006, 0x100000, CRC(ad81f204) SHA1(58584a6c8c6cfb6366eaa10aba8a226e419f5ce9), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr0.07-00", 0x000007, 0x100000, CRC(10ce7254) SHA1(2a88d45dbe78ea8358ecd8522b38d775a2fdb34a), ROM_SKIP(7) )
	ROMX_LOAD( "fish_eng.63-56", 0x800000, 0x100000, CRC(4a03f971) SHA1(1ae5ad9a6cd2d612c6519193134dcd5a3f6a5049), ROM_SKIP(7) )
	ROMX_LOAD( "fish_eng.55-48", 0x800001, 0x100000, CRC(6bc00de0) SHA1(b1b180c33906826703452875ce250b28352e2797), ROM_SKIP(7) )
	ROMX_LOAD( "fish_eng.47-40", 0x800002, 0x100000, CRC(41ccc677) SHA1(76ee042632cfdcc99a9bfb75f2a4ef04e08f101b), ROM_SKIP(7) )
	ROMX_LOAD( "fish_eng.39-32", 0x800003, 0x100000, CRC(59a8fa03) SHA1(19e91a4791e0d2dbd8578cee0fa07c491204b0dc), ROM_SKIP(7) )
	ROMX_LOAD( "fish_eng.31-24", 0x800004, 0x100000, CRC(c3bb50a1) SHA1(b868ac0812d1c13feae82d293bb323a93a72e1d3), ROM_SKIP(7) )
	ROMX_LOAD( "fish_eng.23-16", 0x800005, 0x100000, CRC(237cfc93) SHA1(15f61dc621c5328cc7752c76b2b1dae265a5e886), ROM_SKIP(7) )
	ROMX_LOAD( "fish_eng.15-08", 0x800006, 0x100000, CRC(65bec279) SHA1(5e99972279ee9ad32e67866fc63799579a10f2dd), ROM_SKIP(7) )
	ROMX_LOAD( "fish_eng.07-00", 0x800007, 0x100000, CRC(13fa20ad) SHA1(0a04fdea025109c0e604ef2a6d58cfb3adce9bd1), ROM_SKIP(7) )
ROM_END

ROM_START( freezeatjp )
	ROM_REGION32_BE( 0x200000, "user1", 0 )	/* 2MB for R3000 code */
	ROM_LOAD32_BYTE( "prog_jpn.hh",          0x000000, 0x040000, CRC(989302bf) SHA1(232927ec0a52b8bb587a3c206af8e1c6cde67860) )
	ROM_LOAD32_BYTE( "prog_jpn.hl",          0x000001, 0x040000, CRC(6262b760) SHA1(12ca749f5cdc6db7d19f88a21f5f955b80206784) )
	ROM_LOAD32_BYTE( "prog_jpn.lh",          0x000002, 0x040000, CRC(c6a12b0c) SHA1(971242b5b09e15164e7c335e684b5043510c6462) )
	ROM_LOAD32_BYTE( "prog_jpn.ll",          0x000003, 0x040000, CRC(241ea755) SHA1(0db3cfbe577fc78387528390ebb14dbb7a09c97d) )

	ROM_REGION32_BE( 0x1000000, "user2", 0 )	/* 16MB for 64-bit ROM data */
	ROMX_LOAD( "fish_gr0.63-56", 0x000000, 0x100000, CRC(b61061c5) SHA1(aeb409aa5073232d80ed81b27946e753290234f4), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr0.55-48", 0x000001, 0x100000, CRC(c85acf42) SHA1(c3365caeb126a83a7e7afcda25f05849ceb5c98b), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr0.47-40", 0x000002, 0x100000, CRC(67f78f59) SHA1(40b256a8939fad365c7e896cff4a959fcc70a477), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr0.39-32", 0x000003, 0x100000, CRC(6be0508a) SHA1(20f617278ce1666348822d80686cecd8d9b1bc78), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr0.31-24", 0x000004, 0x100000, CRC(905606e0) SHA1(866cd98ea2399fed96f76b16dce751e2c7cfdc98), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr0.23-16", 0x000005, 0x100000, CRC(cdeef6fa) SHA1(1b4d58951b662040540e7d51f88c1b6f282562ee), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr0.15-08", 0x000006, 0x100000, CRC(ad81f204) SHA1(58584a6c8c6cfb6366eaa10aba8a226e419f5ce9), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr0.07-00", 0x000007, 0x100000, CRC(10ce7254) SHA1(2a88d45dbe78ea8358ecd8522b38d775a2fdb34a), ROM_SKIP(7) )
	ROMX_LOAD( "fish_jpn.63-56", 0x800000, 0x100000, CRC(78c65a1f) SHA1(1a97737c222809930bde9df7a55e1ff1581a3202), ROM_SKIP(7) )
	ROMX_LOAD( "fish_jpn.55-48", 0x800001, 0x100000, CRC(9ffac0f1) SHA1(3ba5f8de32a5febb5d3d22f59ccb477834d33934), ROM_SKIP(7) )
	ROMX_LOAD( "fish_jpn.47-40", 0x800002, 0x100000, CRC(18543fb7) SHA1(4ab9969de9a66d6b7b70cfa5290c1cf7bce54838), ROM_SKIP(7) )
	ROMX_LOAD( "fish_jpn.39-32", 0x800003, 0x100000, CRC(22578f15) SHA1(4e314b22456ed4e4282406e990207e3b9bdf6203), ROM_SKIP(7) )
	ROMX_LOAD( "fish_jpn.31-24", 0x800004, 0x100000, CRC(5c41b91b) SHA1(ce354c8a4a3872b009e8af9f75e8f4f0892c7a7e), ROM_SKIP(7) )
	ROMX_LOAD( "fish_jpn.23-16", 0x800005, 0x100000, CRC(c2462646) SHA1(207d51a2aae076bc78548cf96325e670ea41609c), ROM_SKIP(7) )
	ROMX_LOAD( "fish_jpn.15-08", 0x800006, 0x100000, CRC(f8d998ec) SHA1(ffce4a16fbb2fff3dc0a29c0cede4dfe6316e97b), ROM_SKIP(7) )
	ROMX_LOAD( "fish_jpn.07-00", 0x800007, 0x100000, CRC(e7e0daa5) SHA1(108da84cc6b4df7ae88cfdacd27c1728e59cdb81), ROM_SKIP(7) )
ROM_END

ROM_START( freezeat2 )
	ROM_REGION32_BE( 0x200000, "user1", 0 )	/* 2MB for R3000 code */
	ROM_LOAD32_BYTE( "prog.hh",              0x000000, 0x040000, CRC(a8aefa52) SHA1(ba95da93035520de4b15245f68217c59dfb69dbd) )
	ROM_LOAD32_BYTE( "prog.hl",              0x000001, 0x040000, CRC(152dd641) SHA1(52fa260baf1979ed8f15f8abcbbeebd8e595d0e4) )
	ROM_LOAD32_BYTE( "prog.lh",              0x000002, 0x040000, CRC(416d26ed) SHA1(11cf3b88415a8a5d0bb8e1df08603a85202186ef) )
	ROM_LOAD32_BYTE( "prog.ll",              0x000003, 0x040000, CRC(d6a5dbc8) SHA1(0e2176c35cbc59b2a5283366210409d0e930bac7) )

	ROM_REGION32_BE( 0x1000000, "user2", 0 )	/* 16MB for 64-bit ROM data */
	ROMX_LOAD( "fish_gr0.63-56", 0x000000, 0x100000, CRC(99d0dc75) SHA1(b32126eea70c7584d1c34a6ca33282fbaf4b03aa), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr0.55-48", 0x000001, 0x100000, CRC(2dfdfe62) SHA1(e0554d36ef5cf4b6ce171857ea4f2737f11286a5), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr0.47-40", 0x000002, 0x100000, CRC(722aee2a) SHA1(bc79433131bed5b08453d1b80324a28a552783de), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr0.39-32", 0x000003, 0x100000, CRC(919e31b4) SHA1(3807d4629d8277c780dba888c23d17ba47803f27), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr0.31-24", 0x000004, 0x100000, CRC(a957ac95) SHA1(ddfaca994c06976bee8b123857904e64f40b7f31), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr0.23-16", 0x000005, 0x100000, CRC(a147ec66) SHA1(6291008158d581b81e025ed34ff0950983c12c67), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr0.15-08", 0x000006, 0x100000, CRC(206d2f38) SHA1(6aca89df26d3602ff1da3c23f19e0782439623ff), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr0.07-00", 0x000007, 0x100000, CRC(06559831) SHA1(b2c022457425d7900337cfa2fd1622336c0c0bc5), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr1.63-56", 0x800000, 0x100000, CRC(30c624d2) SHA1(4ced77d1663169d0cb37d6728ec52e67f05064c5), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr1.55-48", 0x800001, 0x100000, CRC(049cd60f) SHA1(8a7615a76b57a4e6ef5d95a5ee6c56086671dbb6), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr1.47-40", 0x800002, 0x100000, CRC(d6aaf3bf) SHA1(1c597bdc0e61fd0941cff5a8a93f24f108bd0daa), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr1.39-32", 0x800003, 0x100000, CRC(7d6ebc69) SHA1(668769297f75f9c367bc5cde26419ed092fc9dd8), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr1.31-24", 0x800004, 0x100000, CRC(6e5fee1f) SHA1(1eca79c8d395f881d0a05f10073998fcae70c3b1), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr1.23-16", 0x800005, 0x100000, CRC(a8b1e9b4) SHA1(066285928e574e656510b90bc212a8d86660bd07), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr1.15-08", 0x800006, 0x100000, CRC(c90080e6) SHA1(a764bdd6b4e9e727f7468a53424a9211ec5fd5a8), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr1.07-00", 0x800007, 0x100000, CRC(1f20c020) SHA1(71b32386dc0444264f2f1e2a81899e0e9260994c), ROM_SKIP(7) )
ROM_END

ROM_START( freezeat3 )
	ROM_REGION32_BE( 0x200000, "user1", 0 )	/* 2MB for R3000 code */
	ROM_LOAD32_BYTE( "prog.hh",              0x000000, 0x040000, CRC(863942e6) SHA1(c7429c8a5c86ff93c64950e201cffca83dd7b7b0) )
	ROM_LOAD32_BYTE( "prog.hl",              0x000001, 0x040000, CRC(2acc18ef) SHA1(ead02566f7641b1d1066bd2e257b695e5c7e8437) )
	ROM_LOAD32_BYTE( "prog.lh",              0x000002, 0x040000, CRC(948cf20c) SHA1(86c757aa3c849ef5ba94ed4d5dbf10e833dab6bd) )
	ROM_LOAD32_BYTE( "prog.ll",              0x000003, 0x040000, CRC(5f44969e) SHA1(32345d7c56a3a890e71f8c71f25414d442b60af8) )

	ROM_REGION32_BE( 0x1000000, "user2", 0 )	/* 16MB for 64-bit ROM data */
	ROMX_LOAD( "fish_gr0.63-56", 0x000000, 0x100000, CRC(36799449) SHA1(bb706fe7fdc68f840702a127eed7d4519dd45869), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr0.55-48", 0x000001, 0x100000, CRC(23959947) SHA1(a35a6e62c7b2be57d41b1b64be93713cbf897f0a), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr0.47-40", 0x000002, 0x100000, CRC(4657e4e0) SHA1(b6c07182babcb0a106bf4a8f2e3f524371dd882d), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr0.39-32", 0x000003, 0x100000, CRC(b6ea4b64) SHA1(176f94f14307c40b9c611d6f6bc9118e498cdfad), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr0.31-24", 0x000004, 0x100000, CRC(7d4ce71f) SHA1(a1cf5aa9df8dd29c777c10cfdce0925981584261), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr0.23-16", 0x000005, 0x100000, CRC(02db4fd1) SHA1(fce6f31802bf36d6b006f0b212f553bdf21f9374), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr0.15-08", 0x000006, 0x100000, CRC(7d496d6c) SHA1(f82db0621729a00acf4077482e9dfab040ac829b), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr0.07-00", 0x000007, 0x100000, CRC(3aa389d8) SHA1(52502f2f3c91d7c29261f60fe8f489a352399c96), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr1.63-56", 0x800000, 0x100000, CRC(ead678c9) SHA1(f83d467f6685965b6176b10adbd4e35ef808baf3), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr1.55-48", 0x800001, 0x100000, CRC(3591e752) SHA1(df242d2f724edfd78f7191f0ba7a8cde2c09b25f), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr1.47-40", 0x800002, 0x100000, CRC(e29a7a6c) SHA1(0bfb26076b390492eed81d4c4f0852c64fdccfce), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr1.39-32", 0x800003, 0x100000, CRC(e980f957) SHA1(78e8ef07f443ce7991a46005627d5802d36d731c), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr1.31-24", 0x800004, 0x100000, CRC(d90c5221) SHA1(7a330f39f3751d58157f872d92c3c2b91fe60d14), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr1.23-16", 0x800005, 0x100000, CRC(9be0d4de) SHA1(9bb67a1f1db77483e896fed7096c1e23c153ede4), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr1.15-08", 0x800006, 0x100000, CRC(122248af) SHA1(80dd5486106d475bd9f6d78919ebeb176e7becff), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr1.07-00", 0x800007, 0x100000, CRC(5ae08327) SHA1(822d8292793509ebfbfce27e92a74c78c4328bda), ROM_SKIP(7) )
ROM_END

ROM_START( freezeat4 )
	ROM_REGION32_BE( 0x200000, "user1", 0 )	/* 2MB for R3000 code */
	ROM_LOAD32_BYTE( "prog.hh",              0x000000, 0x040000, CRC(80336f5e) SHA1(9946e8eebec2cd68db059f40f535ea212f41913d) )
	ROM_LOAD32_BYTE( "prog.hl",              0x000001, 0x040000, CRC(55125520) SHA1(13be4fbf32bcd94a2ea97fd690bd1dfdff146d33) )
	ROM_LOAD32_BYTE( "prog.lh",              0x000002, 0x040000, CRC(9d99c794) SHA1(f443f05a5979db66d61ef4174f0369a1cf4b7793) )
	ROM_LOAD32_BYTE( "prog.ll",              0x000003, 0x040000, CRC(e03700e0) SHA1(24d41750f02ee7e8fb379e517751b661400aa521) )

	ROM_REGION32_BE( 0x1000000, "user2", 0 )	/* 16MB for 64-bit ROM data */
	ROMX_LOAD( "fish_gr0.63-56", 0x000000, 0x100000, CRC(36799449) SHA1(bb706fe7fdc68f840702a127eed7d4519dd45869), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr0.55-48", 0x000001, 0x100000, CRC(23959947) SHA1(a35a6e62c7b2be57d41b1b64be93713cbf897f0a), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr0.47-40", 0x000002, 0x100000, CRC(4657e4e0) SHA1(b6c07182babcb0a106bf4a8f2e3f524371dd882d), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr0.39-32", 0x000003, 0x100000, CRC(b6ea4b64) SHA1(176f94f14307c40b9c611d6f6bc9118e498cdfad), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr0.31-24", 0x000004, 0x100000, CRC(7d4ce71f) SHA1(a1cf5aa9df8dd29c777c10cfdce0925981584261), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr0.23-16", 0x000005, 0x100000, CRC(02db4fd1) SHA1(fce6f31802bf36d6b006f0b212f553bdf21f9374), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr0.15-08", 0x000006, 0x100000, CRC(7d496d6c) SHA1(f82db0621729a00acf4077482e9dfab040ac829b), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr0.07-00", 0x000007, 0x100000, CRC(3aa389d8) SHA1(52502f2f3c91d7c29261f60fe8f489a352399c96), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr1.63-56", 0x800000, 0x100000, CRC(c91b6ee4) SHA1(58d2d6b1b9847150b8b3e358842c4a097ef91475), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr1.55-48", 0x800001, 0x100000, CRC(65528e55) SHA1(18020cababed379f77149b7e89e80b294766df31), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr1.47-40", 0x800002, 0x100000, CRC(8fe4187f) SHA1(c9ceec40688617e1251142465d0e608f80a83e40), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr1.39-32", 0x800003, 0x100000, CRC(fdf05a42) SHA1(849e224b68be2fb396ee4cb4729517470af7c282), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr1.31-24", 0x800004, 0x100000, CRC(bb2cd741) SHA1(ac55a54c702d222cb1b9bb480b0f7a71bc315878), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr1.23-16", 0x800005, 0x100000, CRC(ea8c5984) SHA1(eca1619c17dfac154a2024ec49b4b4f9f06a50c9), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr1.15-08", 0x800006, 0x100000, CRC(0b00c816) SHA1(879b0e9d92fe737d740c348dc1cc376c8abfbdb8), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr1.07-00", 0x800007, 0x100000, CRC(a84335c3) SHA1(340f5ddb9bff1ecd469eab8be36cc0ede84f1f5e), ROM_SKIP(7) )
ROM_END

ROM_START( freezeat5 )
	ROM_REGION32_BE( 0x200000, "user1", 0 )	/* 2MB for R3000 code */
	ROM_LOAD32_BYTE( "prog.hh",              0x000000, 0x040000, CRC(95c4fc64) SHA1(cd00efe7f760ef1e4cdc4bc8a3b368427cb15d8a) )
	ROM_LOAD32_BYTE( "prog.hl",              0x000001, 0x040000, CRC(ffb9cb71) SHA1(35d6a5440d63bc5b94c4447645365039169da368) )
	ROM_LOAD32_BYTE( "prog.lh",              0x000002, 0x040000, CRC(3ddacd80) SHA1(79f9650531847eefd83908b6ea1e8362688b377c) )
	ROM_LOAD32_BYTE( "prog.ll",              0x000003, 0x040000, CRC(95ebefb0) SHA1(b88b12adabd7b0902c3a78919bcec8d9a2b04168) )

	ROM_REGION32_BE( 0x1000000, "user2", 0 )	/* 16MB for 64-bit ROM data */
	ROMX_LOAD( "fish_gr0.63-56", 0x000000, 0x100000, CRC(404a10c3) SHA1(8e353ac7608bd54f0fea610c85166ad14f2faadb), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr0.55-48", 0x000001, 0x100000, CRC(0b262f2f) SHA1(2a963cb5c3344091406d090edfdda498709c6aa6), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr0.47-40", 0x000002, 0x100000, CRC(43f86d26) SHA1(b31d36b11052514b5bcd5bf8e400457ca572c306), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr0.39-32", 0x000003, 0x100000, CRC(5cf0228f) SHA1(7a8c59cf9a7744e9f332db5f661f507323375968), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr0.31-24", 0x000004, 0x100000, CRC(7a24ff98) SHA1(db9e0e8bb417f187267a6e4fc1e66ff060ee4096), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr0.23-16", 0x000005, 0x100000, CRC(ea163c93) SHA1(d07ed26191d36497c56b15774625a49ecb958386), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr0.15-08", 0x000006, 0x100000, CRC(d364534f) SHA1(153908bb8929a898945f768f8bc3d853c6aeaceb), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr0.07-00", 0x000007, 0x100000, CRC(7ba4cb0d) SHA1(16bd487123f499b7080596dc76253081179a0f66), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr1.63-56", 0x800000, 0x100000, CRC(0e1fc4a9) SHA1(a200bb0af5f1e2c3f8d221ae4e9ba55b9dfb8550), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr1.55-48", 0x800001, 0x100000, CRC(b696b875) SHA1(16dc4d5cee3f08360cf19926584419c21d781f45), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr1.47-40", 0x800002, 0x100000, CRC(e78d9302) SHA1(f8b5ed992c433d63677edbeafd3e465b1d42b455), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr1.39-32", 0x800003, 0x100000, CRC(9b50374c) SHA1(d8af3c9d8e0459e24b974cdf2e75c7c39582912f), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr1.31-24", 0x800004, 0x100000, CRC(b6a19b7e) SHA1(5668b27db4dade8efb1524b8ecd1fe78498e8460), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr1.23-16", 0x800005, 0x100000, CRC(ff835b67) SHA1(19da2de1d067069871c33c8b25fd2eac2d03f627), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr1.15-08", 0x800006, 0x100000, CRC(8daf6995) SHA1(2f44031378b5fb1ba1f80a966dbe902316dc6fe8), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr1.07-00", 0x800007, 0x100000, CRC(3676ac70) SHA1(640c4d4f53ca2bcae2009e402fd6ad70e40defa4), ROM_SKIP(7) )
ROM_END

ROM_START( freezeat6 )
	ROM_REGION32_BE( 0x200000, "user1", 0 )	/* 2MB for R3000 code */
	ROM_LOAD32_BYTE( "prog.hh",              0x000000, 0x040000, CRC(120711fe) SHA1(387e3cc8a1a9ea7d65c528387891d09ed9889fe3) )
	ROM_LOAD32_BYTE( "prog.hl",              0x000001, 0x040000, CRC(18dd292a) SHA1(00e79851140716985f43594142c97e510a06b24a) )
	ROM_LOAD32_BYTE( "prog.lh",              0x000002, 0x040000, CRC(ce387e72) SHA1(021a274da0b828550a47c3778e1059d4e759693a) )
	ROM_LOAD32_BYTE( "prog.ll",              0x000003, 0x040000, CRC(9b307b7c) SHA1(71b696802fe7c867525d2626351dcfacedabd696) )

	ROM_REGION32_BE( 0x1000000, "user2", 0 )	/* 16MB for 64-bit ROM data */
	ROMX_LOAD( "fish_gr0.63-56", 0x000000, 0x100000, CRC(293a3308) SHA1(e4c88759c3b8f8a359db83817dbd0428350b4f7e), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr0.55-48", 0x000001, 0x100000, CRC(18bb4bdf) SHA1(1f6c49b3b5946390fa7582b531f8d9af3baa2567), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr0.47-40", 0x000002, 0x100000, CRC(1faedcc6) SHA1(1e4ecbe4553fb3ebfbd03bd7e16066ccb531d00b), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr0.39-32", 0x000003, 0x100000, CRC(536bc349) SHA1(06d7ac38b2c8cdc85e2cb531bba9c836e50c8247), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr0.31-24", 0x000004, 0x100000, CRC(813d4a31) SHA1(e024f9da2f15a482d8142870baf487297b995ed9), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr0.23-16", 0x000005, 0x100000, CRC(f881514b) SHA1(a694f90621e2c1569a6a5ed8920838ba5506f72e), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr0.15-08", 0x000006, 0x100000, CRC(d7634655) SHA1(d7ac83c0fa5d0ec57d096d4d704fe99ee8160e09), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr0.07-00", 0x000007, 0x100000, CRC(3fca32a3) SHA1(22753a9678e04d9355238e013e58d9f45315579d), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr1.63-56", 0x800000, 0x100000, CRC(a2b89d3a) SHA1(9cfcd0b88dea192ba39efcdccc78d1a0fd8f3388), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr1.55-48", 0x800001, 0x100000, CRC(766822a8) SHA1(2c9b14542a5467c1a3451559ea296da09c2cfdb9), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr1.47-40", 0x800002, 0x100000, CRC(112b519c) SHA1(f0e1ed1b8ad271fa9708f513b11d5cca6e550668), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr1.39-32", 0x800003, 0x100000, CRC(435b5d37) SHA1(ecb6e7271d993f8e315b85e69166838e66dd41a8), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr1.31-24", 0x800004, 0x100000, CRC(2637ae7f) SHA1(5e0bd0e08d8c1eaae725b4d55030c2698abd46e7), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr1.23-16", 0x800005, 0x100000, CRC(e732f1bf) SHA1(a228aee0cc36a0089716f20bfa75d87750692adb), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr1.15-08", 0x800006, 0x100000, CRC(7d4e2d9e) SHA1(4cb9b754b7585df4cae6bdd7085a57729d53e643), ROM_SKIP(7) )
	ROMX_LOAD( "fish_gr1.07-00", 0x800007, 0x100000, CRC(8ea036af) SHA1(1f9baec6712e0ba0e8a744529e41799217760194), ROM_SKIP(7) )
ROM_END


/*************************************
 *
 *  Driver initialization
 *
 *************************************/

static void cojag_common_init(running_machine *machine, UINT16 gpu_jump_offs, UINT16 spin_pc)
{
	/* copy over the ROM */
	main_cpu = machine->device<cpu_device>("maincpu");
	cojag_is_r3000 = (main_cpu->type() == R3041BE);

	/* install synchronization hooks for GPU */
	if (cojag_is_r3000)
		memory_install_write32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x04f0b000 + gpu_jump_offs, 0x04f0b003 + gpu_jump_offs, 0, 0, gpu_jump_w);
	else
		memory_install_write32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xf0b000 + gpu_jump_offs, 0xf0b003 + gpu_jump_offs, 0, 0, gpu_jump_w);
	memory_install_read32_handler(cputag_get_address_space(machine, "gpu", ADDRESS_SPACE_PROGRAM), 0xf03000 + gpu_jump_offs, 0xf03003 + gpu_jump_offs, 0, 0, gpu_jump_r);
	gpu_jump_address = &jaguar_gpu_ram[gpu_jump_offs/4];
	gpu_spin_pc = 0xf03000 + spin_pc;

	/* init the sound system and install DSP speedups */
	cojag_sound_init(machine);
}


static DRIVER_INIT( area51a )
{
	cojag_common_init(machine, 0x5c4, 0x5a0);

#if ENABLE_SPEEDUP_HACKS
	/* install speedup for main CPU */
	main_speedup = memory_install_write32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xa02030, 0xa02033, 0, 0, area51_main_speedup_w);
#endif
}


static DRIVER_INIT( area51 )
{
	cojag_common_init(machine, 0x0c0, 0x09e);

#if ENABLE_SPEEDUP_HACKS
	/* install speedup for main CPU */
	main_speedup_max_cycles = 120;
	main_speedup = memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x100062e8, 0x100062eb, 0, 0, cojagr3k_main_speedup_r);
#endif
}

static DRIVER_INIT( maxforce )
{
	cojag_common_init(machine, 0x0c0, 0x09e);

	/* patch the protection */
	rom_base[0x220/4] = 0x03e00008;

#if ENABLE_SPEEDUP_HACKS
	/* install speedup for main CPU */
	main_speedup_max_cycles = 120;
	main_speedup = memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x1000865c, 0x1000865f, 0, 0, cojagr3k_main_speedup_r);
#endif
}


static DRIVER_INIT( area51mx )
{
	cojag_common_init(machine, 0x0c0, 0x09e);

	/* patch the protection */
	rom_base[0x418/4] = 0x4e754e75;

#if ENABLE_SPEEDUP_HACKS
	/* install speedup for main CPU */
	main_speedup = memory_install_write32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xa19550, 0xa19557, 0, 0, area51mx_main_speedup_w);
#endif
}


static DRIVER_INIT( a51mxr3k )
{
	cojag_common_init(machine, 0x0c0, 0x09e);

	/* patch the protection */
	rom_base[0x220/4] = 0x03e00008;

#if ENABLE_SPEEDUP_HACKS
	/* install speedup for main CPU */
	main_speedup_max_cycles = 120;
	main_speedup = memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x10006f0c, 0x10006f0f, 0, 0, cojagr3k_main_speedup_r);
#endif
}


static DRIVER_INIT( fishfren )
{
	cojag_common_init(machine, 0x578, 0x554);

#if ENABLE_SPEEDUP_HACKS
	/* install speedup for main CPU */
	main_speedup_max_cycles = 200;
	main_speedup = memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x10021b60, 0x10021b63, 0, 0, cojagr3k_main_speedup_r);
#endif
}


static void init_freeze_common(running_machine *machine, offs_t main_speedup_addr)
{
	cojag_common_init(machine, 0x0bc, 0x09c);

#if ENABLE_SPEEDUP_HACKS
	/* install speedup for main CPU */
	main_speedup_max_cycles = 200;
	if (main_speedup_addr != 0)
		main_speedup = memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), main_speedup_addr, main_speedup_addr + 3, 0, 0, cojagr3k_main_speedup_r);
	main_gpu_wait = memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x0400d900, 0x0400d900 + 3, 0, 0, main_gpu_wait_r);
#endif
}

static DRIVER_INIT( freezeat ) { init_freeze_common(machine, 0x1001a9f4); }
static DRIVER_INIT( freezeat2 ) { init_freeze_common(machine, 0x1001a8c4); }
static DRIVER_INIT( freezeat3 ) { init_freeze_common(machine, 0x1001a134); }
static DRIVER_INIT( freezeat4 ) { init_freeze_common(machine, 0x1001a134); }
static DRIVER_INIT( freezeat5 ) { init_freeze_common(machine, 0x10019b34); }
static DRIVER_INIT( freezeat6 ) { init_freeze_common(machine, 0x10019684); }


static DRIVER_INIT( vcircle )
{
	cojag_common_init(machine, 0x5c0, 0x5a0);

#if ENABLE_SPEEDUP_HACKS
	/* install speedup for main CPU */
	main_speedup_max_cycles = 50;
	main_speedup = memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x12005b34, 0x12005b37, 0, 0, cojagr3k_main_speedup_r);
#endif
}



/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1996, area51,    0,        cojagr3k,  area51,   area51,   ROT0, "Atari Games", "Area 51 (R3000)", 0 )
GAME( 1995, area51t,   area51,   cojag68k,  area51,   area51a,  ROT0, "Atari Games (Time Warner license)", "Area 51 (Time Warner license)", 0 )
GAME( 1995, area51a,   area51,   cojag68k,  area51,   area51a,  ROT0, "Atari Games", "Area 51 (Atari Games license)", 0 )
GAME( 1995, fishfren,  0,        cojagr3k,  fishfren, fishfren, ROT0, "Time Warner Interactive", "Fishin' Frenzy (prototype)", 0 )
GAME( 1996, freezeat,  0,        cojagr3k,  freezeat, freezeat, ROT0, "Atari Games", "Freeze (Atari) (prototype, English voice, 96/10/25)", 0 )
GAME( 1996, freezeatjp,freezeat, cojagr3k,  freezeat, freezeat, ROT0, "Atari Games", "Freeze (Atari) (prototype, Japanese voice, 96/10/25)", 0 )
GAME( 1996, freezeat2, freezeat, cojagr3k,  freezeat, freezeat2,ROT0, "Atari Games", "Freeze (Atari) (prototype, 96/10/18)", 0 )
GAME( 1996, freezeat3, freezeat, cojagr3k,  freezeat, freezeat3,ROT0, "Atari Games", "Freeze (Atari) (prototype, 96/10/07)", 0 )
GAME( 1996, freezeat4, freezeat, cojagr3k,  freezeat, freezeat4,ROT0, "Atari Games", "Freeze (Atari) (prototype, 96/10/03)", 0 )
GAME( 1996, freezeat5, freezeat, cojagr3k,  freezeat, freezeat5,ROT0, "Atari Games", "Freeze (Atari) (prototype, 96/09/20, AMOA-96)", 0 )
GAME( 1996, freezeat6, freezeat, cojagr3k,  freezeat, freezeat6,ROT0, "Atari Games", "Freeze (Atari) (prototype, 96/09/07, Jamma-96)", 0 )
GAME( 1996, maxforce,  0,        cojagr3k,  area51,   maxforce, ROT0, "Atari Games", "Maximum Force v1.05", 0 )
GAME( 1996, maxf_102,  maxforce, cojagr3k,  area51,   maxforce, ROT0, "Atari Games", "Maximum Force v1.02", 0 )
GAME( 1996, maxf_ng,   maxforce, cojagr3k,  area51,   maxforce, ROT0, "Atari Games", "Maximum Force (No Gore version)", 0 )
GAME( 1998, area51mx,  0,        cojag68k,  area51,   area51mx, ROT0, "Atari Games", "Area 51 / Maximum Force Duo v2.0", 0 )
GAME( 1998, a51mxr3k,  area51mx, cojagr3k,  area51,   a51mxr3k, ROT0, "Atari Games", "Area 51 / Maximum Force Duo (R3000)", 0 )
GAME( 1996, vcircle,   0,        cojagr3k,  vcircle,  vcircle,  ROT0, "Atari Games", "Vicious Circle (prototype)", 0 )
