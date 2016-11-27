/***************************************************************************

    Sega pre-System 16 & System 16A hardware

****************************************************************************

    Known bugs:
        * none at this time

    DIP locations verified from manual for:
        * aceattaa
        * aliensyn
        * aliensynj
        * passsht16a
        * quartet
        * quart2
        * shinobi
        * tetris
        * timescan
        * wb3

***************************************************************************

System16A Hardware Overview
---------------------------

The games on this system include... (there may be more??)
Action Fighter (C) Sega 1985
Alex Kidd      (C) Sega 1986
Fantasy Zone   (C) Sega 1986
SDI            (C) Sega 1987
Shinobi        (C) Sega 1987
Tetris         (C) Sega 1988
Passing Shot   (C) Sega 1988

PCB Layout
----------

Top PCB

171-5306 (number under PCB, no numbers on top)
         |----------|     |-----------|     |-----------|
  |------|----------|-----|-----------|-----|-----------|------|
|-|      16MHz    25.1478MHz                                   |
| |                                  315-5149                  |
|-|    YM3012 YM2151  ROM.IC24 ROM.IC41                        |
  | VOL                                                        |
  |                   ROM.IC25 ROM.IC42  MB3771                |
|-|         D8255                           315-5155           |
|                     ROM.IC26 ROM.IC43     315-5155  ROM.IC93 |
|S                                                             |
|E          Z80A      TC5565   TC5565       315-5155  ROM.IC94 |
|G                315-5141                  315-5155           |
|A         ROM.IC12                                   ROM.IC95 |
|5                                          315-5155           |
|6          2016                            315-5155           |
|                                                     2016     |
|-|                                       8751                 |
  |        DSW2        |-------------|                2016     |
|-|                    |    68000    | 315-5244                |
|                      |-------------|       315-5142          |
|          DSW1                                                |
|                        10MHz                                 |
|--------------------------------------------------------------|
Notes:
      68000    - running at 10.000MHz. Is replaced with a Hitachi FD1094 in some games.
      Z80      - running at 4.000MHz [16/4]
      YM2151   - running at 4.000MHz [16/4]
      2016     - Fujitsu MB8128 2K x8 SRAM (DIP24)
      TC5565   - Toshiba TC5565 8K x8 SRAM (DIP28)
      8751     - Intel 8751 Microcontroller. It appears to be not used, and instead, games use a small plug-in board
                 containing only one 74HC04 TTL IC. The daughterboard has Sega part number '837-0068' & '171-5468' stamped onto it.
      315-5141 - Signetics CK2605 stamped '315-5141' (DIP20)
      315-5149 - 82S153 Field Programmable Logic Array, sticker '315-5149'(DIP20)
      315-5244 - 82S153 Field Programmable Logic Array, sticker '315-5244'(DIP20)
      315-5142 - Signetics CK2605 stamped '315-5142' (DIP20)
      315-5155 - Custom Sega IC (DIP20)

                         Sound     |---------------------- Main Program --------------------|  |---------- Tiles ---------|
                         Program
Game           CPU       IC12      IC24      IC25      IC26      IC41      IC42      IC43      IC93      IC94      IC95
---------------------------------------------------------------------------------------------------------------------------
Action Fighter 317-0018  EPR10284  EPR10353  EPR10351  EPR10349  EPR10352  EPR10350  EPR10348  EPR10283  EPR10282  EPR10281
Alex Kid       317-0021  EPR10434  -         EPR10428  EPR10427  -         EPR10429  EPR10430  EPR10433  EPR10432  EPR10431
Alex Kid (Alt) 317-0021  EPR10434  -         EPR10446  EPR10445  -         EPR10448  EPR10447  EPR10433  EPR10432  EPR10431
Fantasy Zone   68000     EPR7535   EPR7384   EPR7383   EPR7382   EPR7387   EPR7386   EPR7385   EPR7390   EPR7389   EPR7388
SDI            317-0027  EPR10759  EPR10752  EPR10969  EPR10968  EPR10755  EPR10971  EPR10970  EPR10758  EPR10757  EPR10756
Shinobi        317-0050  EPR11267  -         EPR11261  EPR11260  -         EPR11262  EPR11263  EPR11266  EPR11265  EPR11264
Tetris         317-0093  EPR12205  -         -         EPR12200  -         -         EPR12201  EPR12204  EPR12203  EPR12202


Bottom PCB

171-5307 (number under PCB, no numbers on top)
         |----------|     |-----------|     |-----------|
|--------|----------|-----|-----------|-----|-----------|------|
|                                           315-5144           |-|
|                                                              | |
|                                                              |-|
|        2148 2148 2148                                        |
|                              ROM.IC24    ROM.IC11            |
|        2148 2148 2148  ROM.IC30   ROM.IC18                   |
|                                                   D7751      |
|                                                        6MHz  |
|                              ROM.IC23    ROM.IC10     D8243C |
|            315-5049    ROM.IC29   ROM.IC17                   |
|                                                              |
|                  315-5106    315-5108                        |
|                        315-5107     2018  2018               |
|                                                              |
|            315-5049                                          |
|                                              ROM.IC5 ROM.IC2 |
|TC5565 TC5565                  315-5011                       |
|                                                              |
|               2016  315-5143       315-5012  ROM.IC4 ROM.IC1 |
|TC5565 TC5565  2016                                           |
|--------------------------------------------------------------|
Notes:
      D7751    - NEC uPD7751C Microcontroller, running at 6.000MHz. This is a clone of an 8048 MCU
      D8243C   - NEC D8243C (DIP24)
      2016     - Fujitsu MB8128 2K x8 SRAM (DIP24)
      2018     - Sony CXD5813 2K x8 SRAM
      TC5565   - Toshiba TC5565 8K x8 SRAM (DIP28)
      2148     - Fujitsu MBM2148 1K x4 SRAM (DIP18)
      315-5144 - Signetics CK2605 stamped '315-5144' (DIP20)
      315-5143 - Signetics CK2605 stamped '315-5143' (DIP20)
      315-5106 - PAL16R6 stamped '315-5106' (DIP20)
      315-5107 - PAL16R6 stamped '315-5107' (DIP20)
      315-5108 - PAL16R6 stamped '315-5108' (DIP20)
      315-5011 - Custom Sega IC (DIP40)
      315-5012 - Custom Sega IC (DIP48)
      315-5049 - Custom Sega IC (SDIP64)

               |---------- 7751 Sound Data ---------|  |--------------------------------- Sprites ----------------------------------|

Game           IC1       IC2       IC4       IC5       IC10      IC11      IC17      IC18      IC23      IC24      IC29      IC30
-------------------------------------------------------------------------------------------------------------------------------------
Action Fighter -         -         -         -         EPR10285  EPR10289  EPR10286  EPR10290  EPR10287  EPR10291  EPR10288  EPR10292
Alex Kid       EPR10435  EPR10436  -         -         EPR10437  EPR10441  EPR10438  EPR10442  EPR10439  EPR10443  EPR10440  EPR10444
Fantasy Zone   -         -         -         -         EPR7392   EPR7396   EPR7393   EPR7397   EPR7394   EPR7398   -         -
SDI            -         -         -         -         EPR10760  EPR10763  EPR10761  EPR10764  EPR10762  EPR10765  -         -
Shinobi        EPR11268  -         -         -         EPR11290  EPR11294  EPR11291  EPR11295  EPR11292  EPR11296  EPR11293  EPR11297
Tetris         -         -         -         -         EPR12169  EPR12170  -         -         -         -         -         -
*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "cpu/mcs51/mcs51.h"
#include "includes/segas16.h"
#include "machine/8255ppi.h"
#include "machine/segacrp2.h"
#include "machine/fd1089.h"
#include "machine/i8243.h"
#include "cpu/mcs48/mcs48.h"
#include "sound/dac.h"
#include "sound/2151intf.h"
#include "video/segaic16.h"


/*************************************
 *
 *  Statics
 *
 *************************************/

static UINT16 *workram;


/*************************************
 *
 *  Prototypes
 *
 *************************************/

static READ16_HANDLER( misc_io_r );
static WRITE16_HANDLER( misc_io_w );

static WRITE8_DEVICE_HANDLER( video_control_w );
static WRITE8_DEVICE_HANDLER( tilemap_sound_w );



/*************************************
 *
 *  PPI interfaces
 *
 *************************************/

static const ppi8255_interface single_ppi_intf =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_MEMORY_HANDLER("maincpu", PROGRAM, soundlatch_w),
	DEVCB_HANDLER(video_control_w),
	DEVCB_HANDLER(tilemap_sound_w)
};



/*************************************
 *
 *  Configuration
 *
 *************************************/

static void system16a_generic_init(running_machine *machine)
{
	segas1x_state *state = (segas1x_state *)machine->driver_data;

	/* init the FD1094 */
	fd1094_driver_init(machine, "maincpu", NULL);

	/* reset the custom handlers and other pointers */
	state->custom_io_r = NULL;
	state->custom_io_w = NULL;
	state->lamp_changed_w = NULL;
	state->i8751_vblank_hook = NULL;

	state->maincpu = machine->device("maincpu");
	state->soundcpu = machine->device("soundcpu");
	state->mcu = machine->device("mcu");
	state->ymsnd = machine->device("ymsnd");
	state->ppi8255 = machine->device("ppi8255");
	state->n7751 = machine->device("n7751");
}


static TIMER_CALLBACK( suspend_i8751 )
{
	segas1x_state *state = (segas1x_state *)machine->driver_data;
	cpu_suspend(state->mcu, SUSPEND_REASON_DISABLE, 1);
}



/*************************************
 *
 *  Initialization & interrupts
 *
 *************************************/

static MACHINE_START( system16a )
{
	segas1x_state *state = (segas1x_state *)machine->driver_data;

	state_save_register_global(machine, state->video_control);
	state_save_register_global(machine, state->mcu_control);
	state_save_register_global(machine, state->n7751_command);
	state_save_register_global(machine, state->n7751_rom_address);
	state_save_register_global(machine, state->mj_input_num);
	state_save_register_global(machine, state->last_buttons1);
	state_save_register_global(machine, state->last_buttons2);
	state_save_register_global(machine, state->read_port);
}


static MACHINE_RESET( system16a )
{
	segas1x_state *state = (segas1x_state *)machine->driver_data;

	fd1094_machine_init(machine->device("maincpu"));

	/* if we have a fake i8751 handler, disable the actual 8751 */
	if (state->i8751_vblank_hook != NULL)
		timer_call_after_resynch(machine, NULL, 0, suspend_i8751);

	state->mcu_control = 0x00;
}



/*************************************
 *
 *  I/O space
 *
 *************************************/

static TIMER_CALLBACK( delayed_ppi8255_w )
{
	segas1x_state *state = (segas1x_state *)machine->driver_data;
	ppi8255_w(state->ppi8255, param >> 8, param & 0xff);
}


static READ16_HANDLER( standard_io_r )
{
	segas1x_state *state = (segas1x_state *)space->machine->driver_data;
	offset &= 0x3fff/2;
	switch (offset & (0x3000/2))
	{
		case 0x0000/2:
			return ppi8255_r(state->ppi8255, offset & 3);

		case 0x1000/2:
		{
			static const char *const sysports[] = { "SERVICE", "P1", "UNUSED", "P2" };
			return input_port_read(space->machine, sysports[offset & 3]);
		}

		case 0x2000/2:
			return input_port_read(space->machine, (offset & 1) ? "DSW2" : "DSW1");
	}
	logerror("%06X:standard_io_r - unknown read access to address %04X\n", cpu_get_pc(space->cpu), offset * 2);
	return 0xffff;
}


static WRITE16_HANDLER( standard_io_w )
{
	offset &= 0x3fff/2;
	switch (offset & (0x3000/2))
	{
		case 0x0000/2:
			/* the port C handshaking signals control the Z80 NMI, */
			/* so we have to sync whenever we access this PPI */
			if (ACCESSING_BITS_0_7)
				timer_call_after_resynch(space->machine, NULL, ((offset & 3) << 8) | (data & 0xff), delayed_ppi8255_w);
			return;
	}
	logerror("%06X:standard_io_w - unknown write access to address %04X = %04X & %04X\n", cpu_get_pc(space->cpu), offset * 2, data, mem_mask);
}


static READ16_HANDLER( misc_io_r )
{
	segas1x_state *state = (segas1x_state *)space->machine->driver_data;

	if (state->custom_io_r)
		return (*state->custom_io_r)(space, offset, mem_mask);
	else
		return standard_io_r(space, offset, mem_mask);
}


static WRITE16_HANDLER( misc_io_w )
{
	segas1x_state *state = (segas1x_state *)space->machine->driver_data;

	if (state->custom_io_w)
		(*state->custom_io_w)(space, offset, data, mem_mask);
	else
		standard_io_w(space, offset, data, mem_mask);
}



/*************************************
 *
 *  Video control
 *
 *************************************/

static WRITE8_DEVICE_HANDLER( video_control_w )
{
	/*
        PPI port B

        D7 : Screen flip (1= flip, 0= normal orientation)
        D6 : To 8751 pin 13 (/INT1)
        D5 : To 315-5149 pin 17.
        D4 : Screen enable (1= display, 0= blank)
        D3 : Lamp #2 (1= on, 0= off)
        D2 : Lamp #1 (1= on, 0= off)
        D1 : Coin meter #2
        D0 : Coin meter #1
    */

	segas1x_state *state = (segas1x_state *)device->machine->driver_data;

	if (((state->video_control ^ data) & 0x0c) && state->lamp_changed_w)
		(*state->lamp_changed_w)(device->machine, state->video_control ^ data, data);
	state->video_control = data;

	segaic16_tilemap_set_flip(device->machine, 0, data & 0x80);
	segaic16_sprites_set_flip(device->machine, 0, data & 0x80);

	if (state->mcu != NULL)
		cpu_set_input_line(state->mcu, MCS51_INT1_LINE, (data & 0x40) ? CLEAR_LINE : ASSERT_LINE);

	segaic16_set_display_enable(device->machine, data & 0x10);
	set_led_status(device->machine, 1, data & 0x08);
	set_led_status(device->machine, 0, data & 0x04);
	coin_counter_w(device->machine, 1, data & 0x02);
	coin_counter_w(device->machine, 0, data & 0x01);
}



/*************************************
 *
 *  Sound control
 *
 *************************************/

static WRITE8_DEVICE_HANDLER( tilemap_sound_w )
{
	/*
        PPI port C

        D7 : Port A handshaking signal /OBF
        D6 : Port A handshaking signal ACK
        D5 : Port A handshaking signal IBF
        D4 : Port A handshaking signal /STB
        D3 : Port A handshaking signal INTR
        D2 : To PAL 315-5107 pin 9 (SCONT1)
        D1 : To PAL 315-5108 pin 19 (SCONT0)
        D0 : To MUTE input on MB3733 amplifier.
             0= Sound is disabled
             1= sound is enabled
    */
	segas1x_state *state = (segas1x_state *)device->machine->driver_data;

	cpu_set_input_line(state->soundcpu, INPUT_LINE_NMI, (data & 0x80) ? CLEAR_LINE : ASSERT_LINE);
	segaic16_tilemap_set_colscroll(device->machine, 0, ~data & 0x04);
	segaic16_tilemap_set_rowscroll(device->machine, 0, ~data & 0x02);
}



/*************************************
 *
 *  Sound interaction
 *
 *************************************/

static READ8_HANDLER( sound_data_r )
{
	segas1x_state *state = (segas1x_state *)space->machine->driver_data;

	/* assert ACK */
	ppi8255_set_port_c(state->ppi8255, 0x00);
	return soundlatch_r(space, offset);
}


static WRITE8_HANDLER( n7751_command_w )
{
	/*
        Z80 7751 control port

        D7-D5 = connected to 7751 port C
        D4    = /CS for ROM 3
        D3    = /CS for ROM 2
        D2    = /CS for ROM 1
        D1    = /CS for ROM 0
        D0    = A14 line to ROMs
    */
	segas1x_state *state = (segas1x_state *)space->machine->driver_data;

	int numroms = memory_region_length(space->machine, "n7751data") / 0x8000;
	state->n7751_rom_address &= 0x3fff;
	state->n7751_rom_address |= (data & 0x01) << 14;
	if (!(data & 0x02) && numroms >= 1) state->n7751_rom_address |= 0x00000;
	if (!(data & 0x04) && numroms >= 2) state->n7751_rom_address |= 0x08000;
	if (!(data & 0x08) && numroms >= 3) state->n7751_rom_address |= 0x10000;
	if (!(data & 0x10) && numroms >= 4) state->n7751_rom_address |= 0x18000;
	state->n7751_command = data >> 5;
}


static WRITE8_DEVICE_HANDLER( n7751_control_w )
{
	/*
        YM2151 output port

        D1 = /RESET line on 7751
        D0 = /IRQ line on 7751
    */
	segas1x_state *state = (segas1x_state *)device->machine->driver_data;

	cpu_set_input_line(state->n7751, INPUT_LINE_RESET, (data & 0x01) ? CLEAR_LINE : ASSERT_LINE);
	cpu_set_input_line(state->n7751, 0, (data & 0x02) ? CLEAR_LINE : ASSERT_LINE);
	cpuexec_boost_interleave(device->machine, attotime_zero, ATTOTIME_IN_USEC(100));
}


static WRITE8_DEVICE_HANDLER( n7751_rom_offset_w )
{
	/* P4 - address lines 0-3 */
	/* P5 - address lines 4-7 */
	/* P6 - address lines 8-11 */
	/* P7 - address lines 12-13 */
	segas1x_state *state = (segas1x_state *)device->machine->driver_data;

	int mask = (0xf << (4 * offset)) & 0x3fff;
	int newdata = (data << (4 * offset)) & mask;
	state->n7751_rom_address = (state->n7751_rom_address & ~mask) | newdata;
}


static READ8_HANDLER( n7751_rom_r )
{
	/* read from BUS */
	segas1x_state *state = (segas1x_state *)space->machine->driver_data;
	return memory_region(space->machine, "n7751data")[state->n7751_rom_address];
}


static READ8_DEVICE_HANDLER( n7751_p2_r )
{
	/* read from P2 - 8255's PC0-2 connects to 7751's S0-2 (P24-P26 on an 8048) */
	/* bit 0x80 is an alternate way to control the sample on/off; doesn't appear to be used */
	segas1x_state *state = (segas1x_state *)device->machine->driver_data;
	return 0x80 | ((state->n7751_command & 0x07) << 4) | (i8243_p2_r(device, offset) & 0x0f);
}


static WRITE8_DEVICE_HANDLER( n7751_p2_w )
{
	/* write to P2; low 4 bits go to 8243 */
	i8243_p2_w(device, offset, data & 0x0f);

	/* output of bit $80 indicates we are ready (1) or busy (0) */
	/* no other outputs are used */
}


static READ8_HANDLER( n7751_t1_r )
{
	/* T1 - labelled as "TEST", connected to ground */
	return 0;
}



/*************************************
 *
 *  I8751 interrupt generation
 *
 *************************************/

static INTERRUPT_GEN( i8751_main_cpu_vblank )
{
	/* if we have a fake 8751 handler, call it on VBLANK */
	segas1x_state *state = (segas1x_state *)device->machine->driver_data;

	if (state->i8751_vblank_hook != NULL)
		(*state->i8751_vblank_hook)(device->machine);
}



/*************************************
 *
 *  Per-game I8751 workarounds
 *
 *************************************/

static void dumpmtmt_i8751_sim(running_machine *machine)
{
	segas1x_state *state = (segas1x_state *)machine->driver_data;
	UINT8 flag = workram[0x200/2] >> 8;
	UINT8 tick = workram[0x200/2] & 0xff;
	UINT8 sec = workram[0x202/2] >> 8;
	UINT8 min = workram[0x202/2] & 0xff;

	/* signal a VBLANK to the main CPU */
	cpu_set_input_line(state->maincpu, 4, HOLD_LINE);

	/* out of time? set the flag */
	if (tick == 0 && sec == 0 && min == 0)
		flag = 1;
	else
	{
		if (tick != 0)
			tick--;
		else
		{
			/* the game counts 64 ticks per second */
			tick = 0x40;

			/* seconds are counted in BCD */
			if (sec != 0)
				sec = (sec & 0xf) ? sec - 1 : (sec - 0x10) + 9;
			else
			{
				sec = 0x59;

				/* minutes are counted normally */
				if (min != 0)
					min--;
				else
				{
					flag = 1;
					tick = sec = min = 0;
				}
			}
		}
	}
	workram[0x200/2] = (flag << 8) + tick;
	workram[0x202/2] = (sec << 8) + min;
}


static void quartet_i8751_sim(running_machine *machine)
{
	segas1x_state *state = (segas1x_state *)machine->driver_data;
	const address_space *space = cpu_get_address_space(state->maincpu, ADDRESS_SPACE_PROGRAM);

	/* signal a VBLANK to the main CPU */
	cpu_set_input_line(state->maincpu, 4, HOLD_LINE);

	/* X scroll values */
	segaic16_textram_0_w(space, 0xff8/2, workram[0x0d14/2], 0xffff);
	segaic16_textram_0_w(space, 0xffa/2, workram[0x0d18/2], 0xffff);

	/* page values */
	segaic16_textram_0_w(space, 0xe9e/2, workram[0x0d1c/2], 0xffff);
	segaic16_textram_0_w(space, 0xe9c/2, workram[0x0d1e/2], 0xffff);
}



/*************************************
 *
 *  Ace Attacker custom I/O
 *
 *************************************/

static READ16_HANDLER( aceattaa_custom_io_r )
{
	segas1x_state *state = (segas1x_state *)space->machine->driver_data;

	switch (offset & (0x3000/2))
	{
		case 0x1000/2:
			switch (offset & 3)
			{
				case 0x01:
				{
					switch (state->video_control & 0xf)
					{
						case 0x00: return input_port_read(space->machine, "P1");
						case 0x04: return input_port_read(space->machine, "ANALOGX1");
						case 0x08: return input_port_read(space->machine, "ANALOGY1");
						case 0x0c: return input_port_read(space->machine, "UNUSED");
					}
					break;
				}

				case 0x02:
					return input_port_read(space->machine, "DIAL1") | (input_port_read(space->machine, "DIAL2") << 4);

				case 0x03:
				{
					switch (state->video_control & 0xf)
					{
						case 0x00: return input_port_read(space->machine, "P2");
						case 0x04: return input_port_read(space->machine, "ANALOGX2");
						case 0x08: return input_port_read(space->machine, "ANALOGY2");
						case 0x0c: return input_port_read(space->machine, "POW2");
					}
					break;
				}
			}
			break;
	}

	return standard_io_r(space, offset, mem_mask);
}



/*************************************
 *
 *  Major League custom I/O
 *
 *************************************/

static READ16_HANDLER( mjleague_custom_io_r )
{
	segas1x_state *state = (segas1x_state *)space->machine->driver_data;

	switch (offset & (0x3000/2))
	{
		case 0x1000/2:
			switch (offset & 3)
			{
				/* offset 0 contains the regular switches; the two upper bits map to the */
				/* upper bit of the trackball controls */
				case 0:
				{
					UINT8 buttons = input_port_read(space->machine, "SERVICE");
					UINT8 analog1 = input_port_read(space->machine, (state->video_control & 4) ? "ANALOGY1" : "ANALOGX1");
					UINT8 analog2 = input_port_read(space->machine, (state->video_control & 4) ? "ANALOGY2" : "ANALOGX2");
					buttons |= (analog1 & 0x80) >> 1;
					buttons |= (analog2 & 0x80);
					return buttons;
				}

				/* offset 1 contains the low 7 bits of player 1's trackballs, plus the */
				/* player 1 select switch mapped to bit 7 */
				case 1:
				{
					UINT8 buttons = input_port_read(space->machine, "BUTTONS1");
					UINT8 analog = input_port_read(space->machine, (state->video_control & 4) ? "ANALOGY1" : "ANALOGX1");
					return (buttons & 0x80) | (analog & 0x7f);
				}

				/* offset 2 contains either the batting control or the "stance" button state */
				case 2:
				{
					if (state->video_control & 4)
						return (input_port_read(space->machine, "ANALOGZ1") >> 4) | (input_port_read(space->machine, "ANALOGZ2") & 0xf0);
					else
					{
						UINT8 buttons1 = input_port_read(space->machine, "BUTTONS1");
						UINT8 buttons2 = input_port_read(space->machine, "BUTTONS2");

						if (!(buttons1 & 0x01))
							state->last_buttons1 = 0;
						else if (!(buttons1 & 0x02))
							state->last_buttons1 = 1;
						else if (!(buttons1 & 0x04))
							state->last_buttons1 = 2;
						else if (!(buttons1 & 0x08))
							state->last_buttons1 = 3;

						if (!(buttons2 & 0x01))
							state->last_buttons2 = 0;
						else if (!(buttons2 & 0x02))
							state->last_buttons2 = 1;
						else if (!(buttons2 & 0x04))
							state->last_buttons2 = 2;
						else if (!(buttons2 & 0x08))
							state->last_buttons2 = 3;

						return state->last_buttons1 | (state->last_buttons2 << 4);
					}
				}

				/* offset 2 contains the low 7 bits of player 2's trackballs, plus the */
				/* player 2 select switch mapped to bit 7 */
				case 3:
				{
					UINT8 buttons = input_port_read(space->machine, "BUTTONS2");
					UINT8 analog = input_port_read(space->machine, (state->video_control & 4) ? "ANALOGY2" : "ANALOGX2");
					return (buttons & 0x80) | (analog & 0x7f);
				}
			}
			break;
	}
	return standard_io_r(space, offset, mem_mask);
}



/*************************************
 *
 *  Passing Shot custom I/O
 *
 *************************************/

static READ16_HANDLER( passsht16a_custom_io_r )
{
	segas1x_state *state = (segas1x_state *)space->machine->driver_data;

	switch (offset & (0x3000/2))
	{
		case 0x1000/2:
			switch (offset & 3)
			{
				case 0:
					state->read_port = 0;
					break;

				case 1:
					switch ((state->read_port++) & 3)
					{
						case 0: return input_port_read(space->machine, "P1");
						case 1: return input_port_read(space->machine, "P2");
						case 2: return input_port_read(space->machine, "P3");
						case 3: return input_port_read(space->machine, "P4");
					}

					break;
			}
			break;
	}
	return standard_io_r(space, offset, mem_mask);
}



/*************************************
 *
 *  SDI custom I/O
 *
 *************************************/

static READ16_HANDLER( sdi_custom_io_r )
{
	segas1x_state *state = (segas1x_state *)space->machine->driver_data;

	switch (offset & (0x3000/2))
	{
		case 0x1000/2:
			switch (offset & 3)
			{
				case 1:	return input_port_read(space->machine, (state->video_control & 4) ? "ANALOGY1" : "ANALOGX1");
				case 3:	return input_port_read(space->machine, (state->video_control & 4) ? "ANALOGY2" : "ANALOGX2");
			}
			break;
	}
	return standard_io_r(space, offset, mem_mask);
}



/*************************************
 *
 *  Sukeban Jansi Ryuko custom I/O
 *
 *************************************/

static READ16_HANDLER( sjryuko_custom_io_r )
{
	segas1x_state *state = (segas1x_state *)space->machine->driver_data;

	static const char *const portname[] = { "MJ0", "MJ1", "MJ2", "MJ3", "MJ4", "MJ5" };
	switch (offset & (0x3000/2))
	{
		case 0x1000/2:
			switch (offset & 3)
			{
				case 1:
					if (input_port_read_safe(space->machine, portname[state->mj_input_num], 0xff) != 0xff)
						return 0xff & ~(1 << state->mj_input_num);
					return 0xff;

				case 2:
					return input_port_read_safe(space->machine, portname[state->mj_input_num], 0xff);
			}
			break;
	}
	return standard_io_r(space, offset, mem_mask);
}


static void sjryuko_lamp_changed_w(running_machine *machine, UINT8 changed, UINT8 newval)
{
	segas1x_state *state = (segas1x_state *)machine->driver_data;

	if ((changed & 4) && (newval & 4))
		state->mj_input_num = (state->mj_input_num + 1) % 6;
}



/*************************************
 *
 *  MCU I/O
 *
 *************************************/

INLINE UINT8 maincpu_byte_r(running_machine *machine, offs_t offset)
{
	segas1x_state *state = (segas1x_state *)machine->driver_data;
	return memory_read_byte(cpu_get_address_space(state->maincpu, ADDRESS_SPACE_PROGRAM), offset);
}


INLINE void maincpu_byte_w(running_machine *machine, offs_t offset, UINT8 data)
{
	segas1x_state *state = (segas1x_state *)machine->driver_data;
	memory_write_byte(cpu_get_address_space(state->maincpu, ADDRESS_SPACE_PROGRAM), offset, data);
}


static WRITE8_HANDLER( mcu_control_w )
{
	segas1x_state *state = (segas1x_state *)space->machine->driver_data;
	int irqline;

	/* if we have a fake i8751 handler, ignore writes by the actual 8751 */
	if (state->i8751_vblank_hook != NULL)
		return;

	cpu_set_input_line(state->maincpu, INPUT_LINE_RESET, (data & 0x40) ? ASSERT_LINE : CLEAR_LINE);
	for (irqline = 1; irqline <= 7; irqline++)
		cpu_set_input_line(state->maincpu, irqline, ((~data & 7) == irqline) ? ASSERT_LINE : CLEAR_LINE);

	if (data & 0x40)
		segaic16_set_display_enable(space->machine, 1);

	if ((state->mcu_control ^ data) & 0x40)
		cpuexec_boost_interleave(space->machine, attotime_zero, ATTOTIME_IN_USEC(10));

	state->mcu_control = data;
}


static WRITE8_HANDLER( mcu_io_w )
{
	/*
        1.00 0... = work RAM (accessed @ $4000+x) or I/O (accessed @ $8000+x)
        1.00 1... = text RAM (accessed @ $8000+x)
        1.01 1... = palette RAM
        1.10 1... = checksum #0
        1.11 0... = checksum #1
        1.11 1... = checksum #2
    */
	segas1x_state *state = (segas1x_state *)space->machine->driver_data;

	switch ((state->mcu_control >> 3) & 7)
	{
		case 0:
			if (offset >= 0x4000 && offset < 0x8000)
				maincpu_byte_w(space->machine, 0xc70001 ^ (offset & 0x3fff), data);
			else if (offset >= 0x8000 && offset < 0xc000)
				maincpu_byte_w(space->machine, 0xc40001 ^ (offset & 0x3fff), data);
			else
				logerror("%03X: MCU movx write mode %02X offset %04X = %02X\n",
						 cpu_get_pc(space->cpu), state->mcu_control, offset, data);
			break;

		case 1:
			if (offset >= 0x8000 && offset < 0x9000)
				maincpu_byte_w(space->machine, 0x410001 ^ (offset & 0xfff), data);
			else
				logerror("%03X: MCU movx write mode %02X offset %04X = %02X\n",
						 cpu_get_pc(space->cpu), state->mcu_control, offset, data);
			break;

		case 3:
			maincpu_byte_w(space->machine, 0x840001 ^ offset, data);
			break;

		case 5:
		case 6:
		case 7:
			/* ROM */

		default:
			logerror("%03X: MCU movx write mode %02X offset %04X = %02X\n",
					 cpu_get_pc(space->cpu), state->mcu_control, offset, data);
			break;
	}
}


static READ8_HANDLER( mcu_io_r )
{
	segas1x_state *state = (segas1x_state *)space->machine->driver_data;

	switch ((state->mcu_control >> 3) & 7)
	{
		case 0:
			if (offset >= 0x0000 && offset < 0x3fff)
				return watchdog_reset_r(space, 0);		/* unsure about this one */
			else if (offset >= 0x4000 && offset < 0x8000)
				return maincpu_byte_r(space->machine, 0xc70001 ^ (offset & 0x3fff));
			else if (offset >= 0x8000 && offset < 0xc000)
				return maincpu_byte_r(space->machine, 0xc40001 ^ (offset & 0x3fff));
			logerror("%03X: MCU movx read mode %02X offset %04X\n",
					 cpu_get_pc(space->cpu), state->mcu_control, offset);
			return 0xff;

		case 1:
			if (offset >= 0x8000 && offset < 0x9000)
				return maincpu_byte_r(space->machine, 0x410001 ^ (offset & 0xfff));
			logerror("%03X: MCU movx read mode %02X offset %04X\n",
					 cpu_get_pc(space->cpu), state->mcu_control, offset);
			return 0xff;

		case 3:
			return maincpu_byte_r(space->machine, 0x840001 ^ offset);

		case 5:
			return memory_region(space->machine, "maincpu")[0x00000 + offset];
		case 6:
			return memory_region(space->machine, "maincpu")[0x10000 + offset];
		case 7:
			return memory_region(space->machine, "maincpu")[0x20000 + offset];

		default:
			logerror("%03X: MCU movx read mode %02X offset %04X\n",
					 cpu_get_pc(space->cpu), state->mcu_control, offset);
			return 0xff;
	}
}


static INTERRUPT_GEN( mcu_irq_assert )
{
	/* toggle the INT0 line on the MCU */
	cpu_set_input_line(device, MCS51_INT0_LINE, ASSERT_LINE);
	cpu_set_input_line(device, MCS51_INT0_LINE, CLEAR_LINE);

	/* boost interleave to ensure that the MCU can break the M68000 out of a STOP */
	cpuexec_boost_interleave(device->machine, attotime_zero, ATTOTIME_IN_USEC(100));
}



/*************************************
 *
 *  Capacitor-backed RAM
 *
 *************************************/

static NVRAM_HANDLER( system16a )
{
	if (read_or_write)
		mame_fwrite(file, workram, 0x4000);
	else if (file)
		mame_fread(file, workram, 0x4000);
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( system16a_map, ADDRESS_SPACE_PROGRAM, 16 )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x03ffff) AM_MIRROR(0x380000) AM_ROM
	AM_RANGE(0x400000, 0x407fff) AM_MIRROR(0xb88000) AM_RAM_WRITE(segaic16_tileram_0_w) AM_BASE(&segaic16_tileram_0)
	AM_RANGE(0x410000, 0x410fff) AM_MIRROR(0xb8f000) AM_RAM_WRITE(segaic16_textram_0_w) AM_BASE(&segaic16_textram_0)
	AM_RANGE(0x440000, 0x4407ff) AM_MIRROR(0x3bf800) AM_RAM AM_BASE(&segaic16_spriteram_0)
	AM_RANGE(0x840000, 0x840fff) AM_MIRROR(0x3bf000) AM_RAM_WRITE(segaic16_paletteram_w) AM_BASE(&segaic16_paletteram)
	AM_RANGE(0xc40000, 0xc43fff) AM_MIRROR(0x39c000) AM_READWRITE(misc_io_r, misc_io_w)
	AM_RANGE(0xc60000, 0xc6ffff) AM_READ(watchdog_reset16_r)
	AM_RANGE(0xc70000, 0xc73fff) AM_MIRROR(0x38c000) AM_RAM AM_BASE(&workram)
ADDRESS_MAP_END



/*************************************
 *
 *  Sound CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xe800, 0xe800) AM_READ(sound_data_r)
	AM_RANGE(0xf800, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_portmap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_MIRROR(0x3e) AM_DEVREADWRITE("ymsnd", ym2151_r, ym2151_w)
	AM_RANGE(0x80, 0x80) AM_MIRROR(0x3f) AM_WRITE(n7751_command_w)
	AM_RANGE(0xc0, 0xc0) AM_MIRROR(0x3f) AM_READ(sound_data_r)
ADDRESS_MAP_END



/*************************************
 *
 *  N7751 handlers
 *
 *************************************/

static ADDRESS_MAP_START( n7751_portmap, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(MCS48_PORT_BUS,  MCS48_PORT_BUS)  AM_READ(n7751_rom_r)
	AM_RANGE(MCS48_PORT_T1,   MCS48_PORT_T1)   AM_READ(n7751_t1_r)
	AM_RANGE(MCS48_PORT_P1,   MCS48_PORT_P1)   AM_DEVWRITE("dac", dac_w)
	AM_RANGE(MCS48_PORT_P2,   MCS48_PORT_P2)   AM_DEVREADWRITE("n7751_8243", n7751_p2_r, n7751_p2_w)
	AM_RANGE(MCS48_PORT_PROG, MCS48_PORT_PROG) AM_DEVWRITE("n7751_8243", i8243_prog_w)
ADDRESS_MAP_END



/*************************************
 *
 *  i8751 MCU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( mcu_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(mcu_io_r, mcu_io_w)
	AM_RANGE(MCS51_PORT_P1, MCS51_PORT_P1) AM_READNOP AM_WRITE(mcu_control_w)
	AM_RANGE(MCS51_PORT_P3, MCS51_PORT_P3) AM_READNOP	/* read during jb int0 */
ADDRESS_MAP_END



/*************************************
 *
 *  Generic port definitions
 *
 *************************************/

static INPUT_PORTS_START( system16a_generic )
	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY

	PORT_START("UNUSED")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x07, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x05, "2 Coins/1 Credit 5/3 6/4" )
	PORT_DIPSETTING(    0x04, "2 Coins/1 Credit 4/3" )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, "1 Coin/1 Credit 2/3" )
	PORT_DIPSETTING(    0x02, "1 Coin/1 Credit 4/5" )
	PORT_DIPSETTING(    0x03, "1 Coin/1 Credit 5/6" )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, "Free Play (if Coin B too) or 1/1" )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(    0x70, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x50, "2 Coins/1 Credit 5/3 6/4" )
	PORT_DIPSETTING(    0x40, "2 Coins/1 Credit 4/3" )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, "1 Coin/1 Credit 2/3" )
	PORT_DIPSETTING(    0x20, "1 Coin/1 Credit 4/5" )
	PORT_DIPSETTING(    0x30, "1 Coin/1 Credit 5/6" )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, "Free Play (if Coin A too) or 1/1" )

	PORT_START("DSW2")
	PORT_DIPUNUSED_DIPLOC( 0x0001, 0x0001, "SW2:1" )
	PORT_DIPUNUSED_DIPLOC( 0x0002, 0x0002, "SW2:2" )
	PORT_DIPUNUSED_DIPLOC( 0x0004, 0x0004, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x0008, 0x0008, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0010, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0020, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0080, "SW2:8" )
INPUT_PORTS_END


/*************************************
 *
 *  Game-specific port definitions
 *
 *************************************/

static INPUT_PORTS_START( aceattaa )
	PORT_INCLUDE( system16a_generic )

	PORT_MODIFY("SERVICE")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 )			/* Block Switch */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)	/* Block Switch */

	/* This is the direction control of the "hand" device for player 1 */
	/* I'm sure there is a better way to do this but I don't fully understand how the device works yet */
	PORT_MODIFY("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON5 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON6 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON7 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON8 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON9 )

	/* This is the power control of the "hand" device for player 1 */
	PORT_MODIFY("UNUSED")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON10 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON11 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON12 )

	/* This is the direction control of the "hand" device for player 2 */
	/* I'm sure there is a better way to do this but I don't fully understand how the device works yet */
	PORT_MODIFY("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_PLAYER(2)

	/* This is the power control of the "hand" device for player 2 */
	PORT_START("POW2")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON10 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON11 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON12 ) PORT_PLAYER(2)

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0e, 0x0e, "Starting Points" )  PORT_DIPLOCATION("SW2:2,3,4")
	PORT_DIPSETTING(    0x06, "2000" )
	PORT_DIPSETTING(    0x0a, "3000" )
	PORT_DIPSETTING(    0x0c, "4000" )
	PORT_DIPSETTING(    0x0e, "5000" )
	PORT_DIPSETTING(    0x08, "6000" )
	PORT_DIPSETTING(    0x04, "7000" )
	PORT_DIPSETTING(    0x02, "8000" )
	PORT_DIPSETTING(    0x00, "9000" )
	PORT_DIPNAME( 0x30, 0x30, "Point Table" ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )

	PORT_START("ANALOGX1")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(75) PORT_KEYDELTA(5) PORT_REVERSE

	PORT_START("ANALOGY1")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(75) PORT_KEYDELTA(5)

	PORT_START("DIAL1")
	PORT_BIT( 0x0f, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(15)

	PORT_START("ANALOGX2")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(75) PORT_KEYDELTA(5) PORT_REVERSE

	PORT_START("ANALOGY2")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(75) PORT_KEYDELTA(5)

	PORT_START("DIAL2")
	PORT_BIT( 0x0f, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(15)
INPUT_PORTS_END

static INPUT_PORTS_START( afighter )
	PORT_INCLUDE( system16a_generic )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Infinite ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, "10000 - 20000" )
	PORT_DIPSETTING(    0x20, "20000 - 40000" )
	PORT_DIPSETTING(    0x10, "30000 - 60000" )
	PORT_DIPSETTING(    0x00, "40000 - 80000" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )
INPUT_PORTS_END


static INPUT_PORTS_START( alexkidd )
	PORT_INCLUDE( system16a_generic )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Continues ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "Only before level 5" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x00, "240 (Cheat)")
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x20, "10000" )
	PORT_DIPSETTING(    0x30, "20000" )
	PORT_DIPSETTING(    0x10, "40000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Time Adjust" ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x80, "70" )
	PORT_DIPSETTING(    0xc0, "60" )
	PORT_DIPSETTING(    0x40, "50" )
	PORT_DIPSETTING(    0x00, "40" )
INPUT_PORTS_END


static INPUT_PORTS_START( aliensyn )
	PORT_INCLUDE( system16a_generic )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x30, 0x30, "Timer" ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x00, "120" )
	PORT_DIPSETTING(    0x10, "130" )
	PORT_DIPSETTING(    0x20, "140" )
	PORT_DIPSETTING(    0x30, "150" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x80, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
INPUT_PORTS_END


static INPUT_PORTS_START( aliensynj )
	PORT_INCLUDE( system16a_generic )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x30, 0x30, "Timer" ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x00, "150" )
	PORT_DIPSETTING(    0x10, "160" )
	PORT_DIPSETTING(    0x20, "170" )
	PORT_DIPSETTING(    0x30, "180" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x80, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
INPUT_PORTS_END


static INPUT_PORTS_START( bodyslam )
	PORT_INCLUDE( system16a_generic )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Timer Speed" ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, "Slowest" )
	PORT_DIPSETTING(    0x08, "Slow" )
	PORT_DIPSETTING(    0x04, "Fast" )
	PORT_DIPSETTING(    0x00, "Fastest" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x80, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
INPUT_PORTS_END


static INPUT_PORTS_START( fantzone )
	PORT_INCLUDE( system16a_generic )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x00, "240 (Cheat)")
	PORT_DIPNAME( 0x30, 0x30, "Extra Ship Cost" ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, "5000" )
	PORT_DIPSETTING(    0x20, "10000" )
	PORT_DIPSETTING(    0x10, "15000" )
	PORT_DIPSETTING(    0x00, "20000" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x80, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
INPUT_PORTS_END


static INPUT_PORTS_START( mjleague )
	PORT_INCLUDE( system16a_generic )

	PORT_MODIFY("SERVICE")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL )	/* upper bit of trackball */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL )	/* upper bit of trackball */

	PORT_MODIFY("P1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_SPECIAL )

	PORT_MODIFY("P2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_SPECIAL )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x00, "Starting Points" ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, "2000" )
	PORT_DIPSETTING(    0x08, "3000" )
	PORT_DIPSETTING(    0x04, "5000" )
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPNAME( 0x10, 0x10, "Team Select" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:6") //??? something to do with cocktail mode?
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("ANALOGX1")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(75) PORT_KEYDELTA(5)

	PORT_START("ANALOGY1")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(75) PORT_KEYDELTA(5)

	PORT_START("ANALOGX2")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(75) PORT_KEYDELTA(5) PORT_PLAYER(2)

	PORT_START("ANALOGY2")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(75) PORT_KEYDELTA(5) PORT_PLAYER(2)

	PORT_START("ANALOGZ1")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Z ) PORT_SENSITIVITY(75) PORT_KEYDELTA(15)

	PORT_START("ANALOGZ2")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Z ) PORT_SENSITIVITY(75) PORT_KEYDELTA(15) PORT_PLAYER(2)

	PORT_START("BUTTONS1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON5 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("BUTTONS2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( passsht16a )
	PORT_INCLUDE( system16a_generic )

	PORT_MODIFY("SERVICE")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START4 )

	PORT_MODIFY("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 )

	PORT_MODIFY("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)

	PORT_START("P3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(3)

	PORT_START("P4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(4)

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0e, 0x0e, "Initial Point" ) PORT_DIPLOCATION("SW2:2,3,4")
	PORT_DIPSETTING(    0x06, "2000" )
	PORT_DIPSETTING(    0x0a, "3000" )
	PORT_DIPSETTING(    0x0c, "4000" )
	PORT_DIPSETTING(    0x0e, "5000" )
	PORT_DIPSETTING(    0x08, "6000" )
	PORT_DIPSETTING(    0x04, "7000" )
	PORT_DIPSETTING(    0x02, "8000" )
	PORT_DIPSETTING(    0x00, "9000" )
	PORT_DIPNAME( 0x30, 0x30, "Point Table" ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x80, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
INPUT_PORTS_END


static INPUT_PORTS_START( quartet )
	PORT_INCLUDE( system16a_generic )

	PORT_MODIFY("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP  ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_MODIFY("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE2 )

	PORT_MODIFY("UNUSED")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE3 )

	PORT_MODIFY("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE4 )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, "Credit Power" ) PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x04, "500" )
	PORT_DIPSETTING(    0x06, "1000" )
	PORT_DIPSETTING(    0x02, "2000" )
	PORT_DIPSETTING(    0x00, "9000" )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x20, 0x20, "Coin During Game" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, "Power" )
	PORT_DIPSETTING(    0x00, "Credit" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(0x80, IP_ACTIVE_LOW, "SW2:8" )
INPUT_PORTS_END


static INPUT_PORTS_START( quart2 )
	PORT_INCLUDE( system16a_generic )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, "Credit Power" ) PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x04, "500" )
	PORT_DIPSETTING(    0x06, "1000" )
	PORT_DIPSETTING(    0x02, "2000" )
	PORT_DIPSETTING(    0x00, "9000" )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
INPUT_PORTS_END


static INPUT_PORTS_START( sdi )
	PORT_INCLUDE( system16a_generic )

	PORT_MODIFY("SERVICE")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_MODIFY("P1")
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )

	PORT_MODIFY("UNUSED")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN )  PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP )    PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT )  PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN )  PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP )    PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT )  PORT_8WAY PORT_PLAYER(2)

	PORT_MODIFY("P2")
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x00, "Free")
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x80, "Every 50000" )
	PORT_DIPSETTING(    0xc0, "50000" )
	PORT_DIPSETTING(    0x40, "100000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )

	PORT_START("ANALOGX1")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(75) PORT_KEYDELTA(5)

	PORT_START("ANALOGY1")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(75) PORT_KEYDELTA(5) PORT_REVERSE

	PORT_START("ANALOGX2")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(75) PORT_KEYDELTA(5) PORT_PLAYER(2)

	PORT_START("ANALOGY2")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(75) PORT_KEYDELTA(5) PORT_PLAYER(2) PORT_REVERSE
INPUT_PORTS_END


static INPUT_PORTS_START( shinobi )
	PORT_INCLUDE( system16a_generic )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x40, 0x40, "Enemy's Bullet Speed" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, "Slow" )
	PORT_DIPSETTING(    0x00, "Fast" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Language ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Japanese ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
INPUT_PORTS_END


static INPUT_PORTS_START( sjryuko )
	PORT_INCLUDE( system16a_generic )

	PORT_MODIFY("SERVICE")
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("P1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("P2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("MJ0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("MJ1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("MJ2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("MJ3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("MJ4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("MJ5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( tetris )
	PORT_INCLUDE( system16a_generic )

	PORT_MODIFY("P1")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("P2")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("DSW2")
	/* SW2:1,3,4,7,8 Unused according to manual.
    From the code SW2:3,4 looks like some kind of difficulty level,
    but all 4 levels points to the same place so it doesn't actually change anything!! */
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
INPUT_PORTS_END


static INPUT_PORTS_START( timescan )
	PORT_INCLUDE( system16a_generic )

	PORT_MODIFY("P1")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("UNUSED")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SWE:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Out Lane Pin" ) PORT_DIPLOCATION("SWE:2")
	PORT_DIPSETTING(    0x02, "Near" )
	PORT_DIPSETTING(    0x00, "Far" )
	PORT_DIPNAME( 0x0c, 0x0c, "Special" ) PORT_DIPLOCATION("SWE:3,4")
	PORT_DIPSETTING(    0x08, "7 Credits" )
	PORT_DIPSETTING(    0x0c, "3 Credits" )
	PORT_DIPSETTING(    0x04, "1 Credit" )
	PORT_DIPSETTING(    0x00, "2000000 Points" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Allow_Continue ) )  PORT_DIPLOCATION("SWE:5")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0020, "SWE:6" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "SWE:7" ) /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0080, "SWE:8" ) /* Listed as "Unused" */

	PORT_MODIFY("P2")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x1e, 0x1e, "Extended Score" ) PORT_DIPLOCATION("SW2:2,3,4,5")
	PORT_DIPSETTING(    0x16, "Credit 1000000/2000000" )
	PORT_DIPSETTING(    0x14, "Credit 1200000/2500000" )
	PORT_DIPSETTING(    0x12, "Credit 1500000/3000000" )
	PORT_DIPSETTING(    0x10, "Credit 2000000/4000000" )
	PORT_DIPSETTING(    0x1c, "Credit 1000000" )
	PORT_DIPSETTING(    0x1e, "Credit 1200000" )
	PORT_DIPSETTING(    0x1a, "Credit 1500000" )
	PORT_DIPSETTING(    0x18, "Credit 1800000" )
	PORT_DIPSETTING(    0x0e, "Ball 100000" )
	PORT_DIPSETTING(    0x0c, "Ball 200000" )
	PORT_DIPSETTING(    0x0a, "Ball 300000" )
	PORT_DIPSETTING(    0x08, "Ball 400000" )
	PORT_DIPSETTING(    0x06, "Ball 500000" )
	PORT_DIPSETTING(    0x04, "Ball 600000" )
	PORT_DIPSETTING(    0x02, "Ball 700000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x20, 0x20, "Number Match" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "Pin Rebound" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, "Well" )
	PORT_DIPSETTING(    0x00, "A Little" )
	/*
        Pin Rebound = The Setting of "Well" or "A Little" signifies the
        rebound strength and the resulting difficulty or ease in which the
        ball goes out of play.
    */
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, "3" )
	PORT_DIPSETTING(    0x00, "5" )
INPUT_PORTS_END


static INPUT_PORTS_START( wb3 )
	PORT_INCLUDE( system16a_generic )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, "50k/100k/180k/300k" )
	PORT_DIPSETTING(    0x00, "50k/150k/300k" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x40, 0x40, "Test Mode" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )	/* Normal game */
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )	/* Levels are selectable / Player is Invincible */
	/* Swtches 1 & 8 are listed as "Always off" */
INPUT_PORTS_END



/*************************************
 *
 *  Sound definitions
 *
 *************************************/

static const ym2151_interface ym2151_config =
{
	NULL,
	n7751_control_w
};



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


static GFXDECODE_START( segas16a )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,	0, 1024 )
GFXDECODE_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_DRIVER_START( system16a )

	/* driver data */
	MDRV_DRIVER_DATA(segas1x_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 10000000)
	MDRV_CPU_PROGRAM_MAP(system16a_map)
	MDRV_CPU_VBLANK_INT("screen", irq4_line_hold)

	MDRV_CPU_ADD("soundcpu", Z80, 4000000)
	MDRV_CPU_PROGRAM_MAP(sound_map)
	MDRV_CPU_IO_MAP(sound_portmap)

	MDRV_CPU_ADD("n7751", N7751, 6000000)
	MDRV_CPU_IO_MAP(n7751_portmap)

	MDRV_I8243_ADD("n7751_8243", NULL, n7751_rom_offset_w)

	MDRV_MACHINE_START(system16a)
	MDRV_MACHINE_RESET(system16a)
	MDRV_NVRAM_HANDLER(system16a)

	MDRV_PPI8255_ADD( "ppi8255", single_ppi_intf )

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(342,262)	/* to be verified */
	MDRV_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0*8, 28*8-1)

	MDRV_SEGA16SP_ADD_16A("segaspr1")

	MDRV_GFXDECODE(segas16a)
	MDRV_PALETTE_LENGTH(2048*3)

	MDRV_VIDEO_START(system16a)
	MDRV_VIDEO_UPDATE(system16a)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ymsnd", YM2151, 4000000)
	MDRV_SOUND_CONFIG(ym2151_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.43)

	MDRV_SOUND_ADD("dac", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( system16a_no7751 )
	MDRV_IMPORT_FROM(system16a)
	MDRV_DEVICE_REMOVE("n7751")
	MDRV_DEVICE_REMOVE("dac")

	MDRV_SOUND_REPLACE("ymsnd", YM2151, 4000000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( system16a_8751 )
	MDRV_IMPORT_FROM(system16a)
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_VBLANK_INT("screen", i8751_main_cpu_vblank)

	MDRV_CPU_ADD("mcu", I8751, 8000000)
	MDRV_CPU_IO_MAP(mcu_io_map)
	MDRV_CPU_VBLANK_INT("screen", mcu_irq_assert)
MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Ace Attacker, Sega System 16A
    CPU: FD1094 317-0060
*/
ROM_START( aceattaca )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-11573.43", 0x00000, 0x10000, CRC(ca116367) SHA1(ca09b01568dafec48354072267accb73a97a670f) )
	ROM_LOAD16_BYTE( "epr-11571.26", 0x00001, 0x10000, CRC(5631d1d6) SHA1(08a2d375e3e4caf392cf2da2dc5497fd1142ab15) )
	ROM_LOAD16_BYTE( "epr-11574.42", 0x20000, 0x10000, CRC(8d3ed7bd) SHA1(0cb947a796071b0b787638a85fce39135ad8d3cb) )
	ROM_LOAD16_BYTE( "epr-11572.25", 0x20001, 0x10000, CRC(35c27c25) SHA1(ac983db30edb4c4b71f04359cd22c663940435f5) )

	ROM_REGION( 0x2000, "user1", 0 ) /* decryption key */
	ROM_LOAD( "317-0060.key", 0x0000, 0x2000, CRC(f4ee940f) SHA1(13cca3f19bd9761d484185a3476bec3c8c18efcf) )

	ROM_REGION( 0x30000, "gfx1", 0 ) /* tiles */
	ROM_LOAD( "epr-11575.95", 0x00000, 0x10000, CRC(bab9fbf3) SHA1(53d78dfc003e04730c7679c830cc086cbf3e3548) )
	ROM_LOAD( "epr-11576.94", 0x10000, 0x10000, CRC(067ed682) SHA1(13e2e1f812e5a3994a0f1d35eb210881645e74cd) )
	ROM_LOAD( "epr-11577.93", 0x20000, 0x10000, CRC(f67cf331) SHA1(457b9e618dfa678ae374cdeb25488af3de2e8949) )

	ROM_REGION16_BE( 0x080000, "gfx2", 0 ) /* sprites */
	ROM_LOAD16_BYTE( "epr-11501.10", 0x00001, 0x08000, CRC(09179ead) SHA1(3e6bf04e1e9ea867d087a47ff04ad0a064a8e299) )
	ROM_CONTINUE(                    0x40001, 0x08000 )
	ROM_LOAD16_BYTE( "epr-11505.11", 0x00000, 0x08000, CRC(b67f1ecf) SHA1(3a26cdf91e5a1a11c1a8857e713a9e00cc1bfce0) )
	ROM_CONTINUE(                    0x40000, 0x08000 )
	ROM_LOAD16_BYTE( "epr-11502.17", 0x10001, 0x08000, CRC(7464bae4) SHA1(c6b6ca240cee72e7fbd17bad5ca2d5cfe8e835c6) )
	ROM_CONTINUE(                    0x50001, 0x08000 )
	ROM_LOAD16_BYTE( "epr-11506.18", 0x10000, 0x08000, CRC(b0104def) SHA1(c81a66ec3a600c1d4c5d058caef15936c59b2574) )
	ROM_CONTINUE(                    0x50000, 0x08000 )
	ROM_LOAD16_BYTE( "epr-11503.23", 0x20001, 0x08000, CRC(344c0692) SHA1(3125701f6bb91d8f64515e214b571e169c30a444) )
	ROM_CONTINUE(                    0x60001, 0x08000 )
	ROM_LOAD16_BYTE( "epr-11507.24", 0x20000, 0x08000, CRC(a2af710a) SHA1(1c8b75b72797146c2eb788511f8cb1b367fc3e0d) )
	ROM_CONTINUE(                    0x60000, 0x08000 )
	ROM_LOAD16_BYTE( "epr-11504.29", 0x30001, 0x08000, CRC(42b4a5f8) SHA1(efb7beee7b45023861711f56b7cc907400cb266e) )
	ROM_CONTINUE(                    0x70001, 0x08000 )
	ROM_LOAD16_BYTE( "epr-11508.30", 0x30000, 0x08000, CRC(5cbb833c) SHA1(dc7041b6a4fa75d050bfc2176d0f9e242b55a0b8) )
	ROM_CONTINUE(                    0x70000, 0x08000 )

	ROM_REGION( 0x20000, "soundcpu", 0 ) /* sound CPU */
	ROM_LOAD( "epr-11578.12", 0x0000, 0x8000, CRC(3d58e39a) SHA1(3e3591ac96903376698cf95d1b1a5f5db590db3c) )

	ROM_REGION( 0x1000, "n7751", 0 )      /* 4k for 7751 onboard ROM */
	ROM_LOAD( "7751.bin",     0x0000, 0x0400, CRC(6a9534fc) SHA1(67ad94674db5c2aab75785668f610f6f4eccd158) ) /* 7751 - U34 */

	ROM_REGION( 0x20000, "n7751data", 0 ) /* 7751 sound data */
	ROM_LOAD( "epr-11579.1", 0x00000, 0x8000, CRC(1a994135) SHA1(f42444521a878d32b189876f8854f6363e1b353b) )
	ROM_LOAD( "epr-11580.2", 0x08000, 0x8000, CRC(961646ed) SHA1(4c8f87a10ffd7035145dedba86deb373368b0e49) )
	ROM_LOAD( "epr-11581.4", 0x10000, 0x8000, CRC(d271a6e5) SHA1(de303f70abfa28e599e5eb0c2b314f43faa1b484) )
	ROM_LOAD( "epr-11582.5", 0x18000, 0x8000, CRC(bcbe3d8a) SHA1(2e4bb0ebffdd594b192c5b854ea425029d92fdb1) )
ROM_END

/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Action Fighter, Sega System 16A
    CPU: FD1089A 317-0018?@(dedicated cabinet version uses 317-0014)
 */
ROM_START( afighter )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-10350", 0x00001, 0x08000, CRC(f2cd6b3f) SHA1(380f75b8c1696b388179641866cd1d23f78664e7) )
	ROM_LOAD16_BYTE( "epr-10353", 0x00000, 0x08000, CRC(5a757dc9) SHA1(b0540844c8a09195f5d12312f8e27c334641d7b8) )
	ROM_LOAD16_BYTE( "epr-10349", 0x10001, 0x08000, CRC(4b434c37) SHA1(5f3afbdb9cdb0762e56b702a195274f30193b472) )
	ROM_LOAD16_BYTE( "epr-10352", 0x10000, 0x08000, CRC(f8abb143) SHA1(97e78291c15bdf95fd35adca6b9e002480137b12) )
	ROM_LOAD16_BYTE( "epr-10348", 0x20001, 0x08000, CRC(e51e3012) SHA1(bb5522aacb55b5f04aa4cb7a642e202f0ddd7c84) )
	ROM_LOAD16_BYTE( "epr-10351", 0x20000, 0x08000, CRC(ede21d8d) SHA1(b3e3944d706c606fd01e00d9511f020ce9aec9f0) )

	ROM_REGION( 0x30000, "gfx1", 0 ) /* tiles */
	ROM_LOAD( "epr-10281.95", 0x00000, 0x10000, CRC(30e92cda) SHA1(36293a2a8a22dca5350571f19f3d5d04e1b27458) )
	ROM_LOAD( "epr-10282.94", 0x10000, 0x10000, CRC(b67b8910) SHA1(f3f029a3e6547114cec28e5cf8fda65ef434c353) )
	ROM_LOAD( "epr-10283.93", 0x20000, 0x10000, CRC(e7dbfd2d) SHA1(91bae3fbc4a3c612dc507eecfa8de1c2e1e7afee) )

	ROM_REGION16_BE( 0x40000, "gfx2", 0 ) /* sprites */
	ROM_LOAD16_BYTE( "epr-10285", 0x00001, 0x08000, CRC(98aa3d04) SHA1(1d26d17a72e55281e3444fee9c5af69ffb9e3c69) )
	ROM_LOAD16_BYTE( "epr-10286", 0x10001, 0x08000, CRC(8da050cf) SHA1(c28e8968dbd9c110672581f4486f70d5f45df7f5) )
	ROM_LOAD16_BYTE( "epr-10287", 0x20001, 0x08000, CRC(7989b74a) SHA1(a87acafe82b37a11d8f8b1f2ee4c9b2e1bb8161c) )
	ROM_LOAD16_BYTE( "epr-10288", 0x30001, 0x08000, CRC(d3ce551a) SHA1(0ff2170d9ef89058273025dd8d5e1021094adef1) )
	ROM_LOAD16_BYTE( "epr-10289", 0x00000, 0x08000, CRC(c59d1b98) SHA1(e232f2519234981c0e4ffecdd25c48083d9f93a8) )
	ROM_LOAD16_BYTE( "epr-10290", 0x10000, 0x08000, CRC(39354223) SHA1(d8a73d3f7fc2d83d23bb7434f43bc8804f35cc16) )
	ROM_LOAD16_BYTE( "epr-10291", 0x20000, 0x08000, CRC(6e4b245c) SHA1(1f8cecf7ea2d2dfa5ce18d7ee34b0da2cc40221e) )
	ROM_LOAD16_BYTE( "epr-10292", 0x30000, 0x08000, CRC(cef289a3) SHA1(7ab817b6348c168f79be325fb3cc2cca14ee0f8e) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* sound CPU */
	ROM_LOAD( "epr-10284.12", 0x00000, 0x8000, CRC(8ff09116) SHA1(8b99b6d2499897cfbd037a7e7cf5bc53bce8a63a) )

	ROM_REGION( 0x2000, "fd1089a", 0 ) /* decryption key */
	ROM_LOAD( "317-0018.key", 0x0000, 0x2000, CRC(fee04be8) SHA1(c58d78299ef4cede517be823a8a8a90e46c6ba0d) )
ROM_END


/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Alex Kidd, Sega System 16A
    CPU: 68000
 */
ROM_START( alexkidd )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-10447.43", 0x000000, 0x10000, CRC(29e87f71) SHA1(af980e55c02b3de1121c144fee23af74d24042ac) )
	ROM_LOAD16_BYTE( "epr-10445.26", 0x000001, 0x10000, CRC(25ce5b6f) SHA1(dfec64df7e8d145d30740808bc94bdbbe667c4e8) )
	ROM_LOAD16_BYTE( "epr-10448.42", 0x020000, 0x10000, CRC(05baedb5) SHA1(fc15989bf3d850170e4e018d74f18553f0268576) )
	ROM_LOAD16_BYTE( "epr-10446.25", 0x020001, 0x10000, CRC(cd61d23c) SHA1(c235c4fef28556e9f2d07e815ad213c308e85598) )

	ROM_REGION( 0x18000, "gfx1", 0 ) /* tiles */
	ROM_LOAD( "epr-10431.95", 0x00000, 0x08000, CRC(a7962c39) SHA1(c816fc5d9f21b2ba32b9841e64b634bce7ea78c8) )
	ROM_LOAD( "epr-10432.94", 0x08000, 0x08000, CRC(db8cd24e) SHA1(656d98844ad9ccaa68e3f501137dddd0a27d999d) )
	ROM_LOAD( "epr-10433.93", 0x10000, 0x08000, CRC(e163c8c2) SHA1(ac54c5ecedca5b1a2c550de32687ca57c4d3a411) )

	ROM_REGION16_BE( 0x040000, "gfx2", 0 ) /* sprites */
	ROM_LOAD16_BYTE( "epr-10437.10", 0x00001, 0x8000, CRC(522f7618) SHA1(9a6bc857dfef1dd1b7bffa034523c1c4cd8b3f4c) )
	ROM_LOAD16_BYTE( "epr-10441.11", 0x00000, 0x8000, CRC(74e3a35c) SHA1(26b980a0a3aee94ac38e0e0c7d305bb35a60d1c4) )
	ROM_LOAD16_BYTE( "epr-10438.17", 0x10001, 0x8000, CRC(738a6362) SHA1(a3c5f10c263cb216d275875f6333484a1cca281b) )
	ROM_LOAD16_BYTE( "epr-10442.18", 0x10000, 0x8000, CRC(86cb9c14) SHA1(42bd0ed985de61ff183eed0192257966caa01594) )
	ROM_LOAD16_BYTE( "epr-10439.23", 0x20001, 0x8000, CRC(b391aca7) SHA1(ca9d80b67e5365f709f90a5342b5e3aa7c7126e1) )
	ROM_LOAD16_BYTE( "epr-10443.24", 0x20000, 0x8000, CRC(95d32635) SHA1(788af2af1ae783128bcdc8cd44d17cd2f1542231) )
	ROM_LOAD16_BYTE( "epr-10440.29", 0x30001, 0x8000, CRC(23939508) SHA1(68450a18fc7e35f5b0155632aa68cffd251be38c) )
	ROM_LOAD16_BYTE( "epr-10444.30", 0x30000, 0x8000, CRC(82115823) SHA1(e4103003cda949bebe57815115a5028f4fe8e7d7) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* sound CPU */
	ROM_LOAD( "epr-10434.12", 0x0000, 0x8000, CRC(77141cce) SHA1(6c5e83527f7e11a5ff5cc4fa75d55618a55e1a58) )

	ROM_REGION( 0x1000, "n7751", 0 )      /* 4k for 7751 onboard ROM */
	ROM_LOAD( "7751.bin",     0x0000, 0x0400, CRC(6a9534fc) SHA1(67ad94674db5c2aab75785668f610f6f4eccd158) ) /* 7751 - U34 */

	ROM_REGION( 0x10000, "n7751data", 0 ) /* 7751 sound data (not used yet) */
	ROM_LOAD( "epr-10435.1",  0x0000, 0x8000, CRC(ad89f6e3) SHA1(812a132142065b0fe13b5f0ac534b6d8830ba102) )
	ROM_LOAD( "epr-10436.2",  0x8000, 0x8000, CRC(96c76613) SHA1(fe3e4e649fd2cb2453eec0c92015bd54b3b9a1b5) )
ROM_END

/**************************************************************************************************************************
    Alex Kidd, Sega System 16A
    CPU: FD1089A 317-0021
 */
ROM_START( alexkidd1 )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-10429.43", 0x000000, 0x10000, CRC(bdf49eca) SHA1(899bc2d346544e4a33de51b60e02ebf7ee82cea8) )
	ROM_LOAD16_BYTE( "epr-10427.26", 0x000001, 0x10000, CRC(f6e3dd29) SHA1(bb94ebc062bb7c6c13b68579053b9cbe8b92417c) )
	ROM_LOAD16_BYTE( "epr-10430.42", 0x020000, 0x10000, CRC(89e3439f) SHA1(7c751bb477584842d93fda6686b03e289140bd62) )
	ROM_LOAD16_BYTE( "epr-10428.25", 0x020001, 0x10000, CRC(dbed3210) SHA1(1e2d22935a633641ff88967d67ec673ee25cbf55) )

	ROM_REGION( 0x18000, "gfx1", 0 ) /* tiles */
	ROM_LOAD( "epr-10431.95", 0x00000, 0x08000, CRC(a7962c39) SHA1(c816fc5d9f21b2ba32b9841e64b634bce7ea78c8) )
	ROM_LOAD( "epr-10432.94", 0x08000, 0x08000, CRC(db8cd24e) SHA1(656d98844ad9ccaa68e3f501137dddd0a27d999d) )
	ROM_LOAD( "epr-10433.93", 0x10000, 0x08000, CRC(e163c8c2) SHA1(ac54c5ecedca5b1a2c550de32687ca57c4d3a411) )

	ROM_REGION16_BE( 0x040000, "gfx2", 0 ) /* sprites */
	ROM_LOAD16_BYTE( "epr-10437.10", 0x00001, 0x8000, CRC(522f7618) SHA1(9a6bc857dfef1dd1b7bffa034523c1c4cd8b3f4c) )
	ROM_LOAD16_BYTE( "epr-10441.11", 0x00000, 0x8000, CRC(74e3a35c) SHA1(26b980a0a3aee94ac38e0e0c7d305bb35a60d1c4) )
	ROM_LOAD16_BYTE( "epr-10438.17", 0x10001, 0x8000, CRC(738a6362) SHA1(a3c5f10c263cb216d275875f6333484a1cca281b) )
	ROM_LOAD16_BYTE( "epr-10442.18", 0x10000, 0x8000, CRC(86cb9c14) SHA1(42bd0ed985de61ff183eed0192257966caa01594) )
	ROM_LOAD16_BYTE( "epr-10439.23", 0x20001, 0x8000, CRC(b391aca7) SHA1(ca9d80b67e5365f709f90a5342b5e3aa7c7126e1) )
	ROM_LOAD16_BYTE( "epr-10443.24", 0x20000, 0x8000, CRC(95d32635) SHA1(788af2af1ae783128bcdc8cd44d17cd2f1542231) )
	ROM_LOAD16_BYTE( "epr-10440.29", 0x30001, 0x8000, CRC(23939508) SHA1(68450a18fc7e35f5b0155632aa68cffd251be38c) )
	ROM_LOAD16_BYTE( "epr-10444.30", 0x30000, 0x8000, CRC(82115823) SHA1(e4103003cda949bebe57815115a5028f4fe8e7d7) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* sound CPU */
	ROM_LOAD( "epr-10434.12", 0x0000, 0x8000, CRC(77141cce) SHA1(6c5e83527f7e11a5ff5cc4fa75d55618a55e1a58) )

	ROM_REGION( 0x1000, "n7751", 0 )      /* 4k for 7751 onboard ROM */
	ROM_LOAD( "7751.bin",     0x0000, 0x0400, CRC(6a9534fc) SHA1(67ad94674db5c2aab75785668f610f6f4eccd158) ) /* 7751 - U34 */

	ROM_REGION( 0x10000, "n7751data", 0 ) /* 7751 sound data (not used yet) */
	ROM_LOAD( "epr-10435.1",  0x0000, 0x8000, CRC(ad89f6e3) SHA1(812a132142065b0fe13b5f0ac534b6d8830ba102) )
	ROM_LOAD( "epr-10436.2",  0x8000, 0x8000, CRC(96c76613) SHA1(fe3e4e649fd2cb2453eec0c92015bd54b3b9a1b5) )

	ROM_REGION( 0x2000, "fd1089a", 0 ) /* decryption key */
	ROM_LOAD( "317-0021.key", 0x0000, 0x2000, BAD_DUMP CRC(85be8eac) SHA1(3857bf43b3b0ab60c04223e2393d99504a730d73) )
ROM_END

/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Alien Syndrome, System 16A
    CPU: FD1089B 317-0037
 */

ROM_START( aliensyn5 )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-10804.43", 0x00000, 0x8000, CRC(23f78b83) SHA1(76538663e8f7ffffeea4928e3ebe091dac954dc7) )
	ROM_LOAD16_BYTE( "epr-10802.26", 0x00001, 0x8000, CRC(996768bd) SHA1(8c972f0f4120600ecd10ceb3456243d9614924b7) )
	ROM_LOAD16_BYTE( "epr-10805.42", 0x10000, 0x8000, CRC(53d7fe50) SHA1(9a4e18f4d11c77ec04a817dca7b618297658113e) )
	ROM_LOAD16_BYTE( "epr-10803.25", 0x10001, 0x8000, CRC(0536dd33) SHA1(f094ed25d81dadf15a47c15711e47c3cb5f33227) )
	ROM_LOAD16_BYTE( "epr-10732.41", 0x20000, 0x8000, CRC(c5712bfc) SHA1(2f11570784c1fc83cb16dbd1d43e21cf104568ac) )
	ROM_LOAD16_BYTE( "epr-10729.24", 0x20001, 0x8000, CRC(3e520e30) SHA1(8eac2640ff8c4944d6cd82421b565f506c480017) )

	ROM_REGION( 0x30000, "gfx1", 0 ) /* tiles */
	ROM_LOAD( "epr-10739.95", 0x00000, 0x10000, CRC(a29ec207) SHA1(c469d2689a7bdc2a59dfff56ce13d34e9fbac263) )
	ROM_LOAD( "epr-10740.94", 0x10000, 0x10000, CRC(47f93015) SHA1(68247a6bffd1d4d1c450148dd46214d01ce1c668) )
	ROM_LOAD( "epr-10741.93", 0x20000, 0x10000, CRC(4970739c) SHA1(5bdf4222209ec46e0015bfc0f90578dd9b30bdd1) )

	ROM_REGION16_BE( 0x080000, "gfx2", 0 ) /* sprites */
	ROM_LOAD16_BYTE( "epr-10709.10", 0x00001, 0x08000, CRC(addf0a90) SHA1(a92c9531f1817763773471ce63f566b9e88360a0) )
	ROM_CONTINUE(                    0x40001, 0x08000 )
	ROM_LOAD16_BYTE( "epr-10713.11", 0x00000, 0x08000, CRC(ececde3a) SHA1(9c12d4665179bf433c42f5ddc8a043ad592aa90e) )
	ROM_CONTINUE(                    0x40000, 0x08000 )
	ROM_LOAD16_BYTE( "epr-10710.17", 0x10001, 0x08000, CRC(992369eb) SHA1(c6796acf6807e9ba4c3d241903653f91adf4764e) )
	ROM_CONTINUE(                    0x50001, 0x08000 )
	ROM_LOAD16_BYTE( "epr-10714.18", 0x10000, 0x08000, CRC(91bf42fb) SHA1(4b9d3e97768323dee01e92378adafecb26bcc094) )
	ROM_CONTINUE(                    0x50000, 0x08000 )
	ROM_LOAD16_BYTE( "epr-10711.23", 0x20001, 0x08000, CRC(29166ef6) SHA1(99a7cfd7d811537c821412a320beadb5a9c09af3) )
	ROM_CONTINUE(                    0x60001, 0x08000 )
	ROM_LOAD16_BYTE( "epr-10715.24", 0x20000, 0x08000, CRC(a7c57384) SHA1(46f8efa691d7bbb0a18119c0ff12cff7c0d129e1) )
	ROM_CONTINUE(                    0x60000, 0x08000 )
	ROM_LOAD16_BYTE( "epr-10712.29", 0x30001, 0x08000, CRC(876ad019) SHA1(39973ddb5a5746e0e094c759447bff1130c72c84) )
	ROM_CONTINUE(                    0x70001, 0x08000 )
	ROM_LOAD16_BYTE( "epr-10716.30", 0x30000, 0x08000, CRC(40ba1d48) SHA1(e2d4d2689bb9b9bdc85e7f72a6665e5fd4c583aa) )
	ROM_CONTINUE(                    0x70000, 0x08000 )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* sound CPU */
	ROM_LOAD( "epr-10705.12", 0x00000, 0x8000, CRC(777b749e) SHA1(086b03100064a98228f95db7962b2671121c46ea) )

	ROM_REGION( 0x1000, "n7751", 0 )  /* 4k for 7751 onboard ROM */
	ROM_LOAD( "7751.bin",     0x0000, 0x0400, CRC(6a9534fc) SHA1(67ad94674db5c2aab75785668f610f6f4eccd158) ) /* 7751 - U34 */

	ROM_REGION( 0x18000, "n7751data", 0 ) /* 7751 sound data */
	ROM_LOAD( "epr-10706.1",  0x00000, 0x8000, CRC(aa114acc) SHA1(81a2b3586ae90bc7fc55b82478ffe182ac49983e) )
	ROM_LOAD( "epr-10707.2",  0x08000, 0x8000, CRC(800c1d82) SHA1(aac4123bd35f87da09264649f4cf8326b2ba3cb8) )
	ROM_LOAD( "epr-10708.4",  0x10000, 0x8000, CRC(5921ef52) SHA1(eff9978361692e6e60a9c6caf5740dd6182cfe4a) )

	ROM_REGION( 0x2000, "fd1089b", 0 ) /* decryption key */
	ROM_LOAD( "317-0037.key", 0x0000, 0x2000, CRC(49e882e5) SHA1(29d87af8fc775b22a9a546c112f8f5e7f700ac1a) )
ROM_END


/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Alien Syndrome, pre-System 16
    CPU: FD1089A 317-0033
 */
ROM_START( aliensyn2 )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-10808", 0x00000, 0x8000, CRC(e669929f) SHA1(b5ab41d6f31f0369f8c5f5eb6fc08e8c23312b96) )
	ROM_LOAD16_BYTE( "epr-10806", 0x00001, 0x8000, CRC(9f7f8fdd) SHA1(819e9c491b7d23deaef646d37319c38e75827d68) )
	ROM_LOAD16_BYTE( "epr-10809", 0x10000, 0x8000, CRC(9a424919) SHA1(a7be5d9bed329099df10ff5a0104cb832485bd0a) )
	ROM_LOAD16_BYTE( "epr-10807", 0x10001, 0x8000, CRC(3d2c3530) SHA1(567ed45c84b1d3d92371c4ad33fdb28f68cf29a3) )
	ROM_LOAD16_BYTE( "epr-10701", 0x20000, 0x8000, CRC(92171751) SHA1(69a282c01db7224f32386a6db25309e09e29a112) )
	ROM_LOAD16_BYTE( "epr-10698", 0x20001, 0x8000, CRC(c1e4fdc0) SHA1(65817a9336f7887d2bf14485bdff8352c960d2ab) )

	ROM_REGION( 0x30000, "gfx1", 0 ) /* tiles */
	ROM_LOAD( "10739", 0x00000, 0x10000, CRC(a29ec207) SHA1(c469d2689a7bdc2a59dfff56ce13d34e9fbac263) )
	ROM_LOAD( "10740", 0x10000, 0x10000, CRC(47f93015) SHA1(68247a6bffd1d4d1c450148dd46214d01ce1c668) )
	ROM_LOAD( "10741", 0x20000, 0x10000, CRC(4970739c) SHA1(5bdf4222209ec46e0015bfc0f90578dd9b30bdd1) )

	ROM_REGION16_BE( 0x080000, "gfx2", 0 ) /* sprites */
	ROM_LOAD16_BYTE( "10709.b1", 0x00001, 0x08000, CRC(addf0a90) SHA1(a92c9531f1817763773471ce63f566b9e88360a0) )
	ROM_CONTINUE(                0x40001, 0x08000 )
	ROM_LOAD16_BYTE( "10713.b5", 0x00000, 0x08000, CRC(ececde3a) SHA1(9c12d4665179bf433c42f5ddc8a043ad592aa90e) )
	ROM_CONTINUE(                0x40000, 0x08000 )
	ROM_LOAD16_BYTE( "10710.b2", 0x10001, 0x08000, CRC(992369eb) SHA1(c6796acf6807e9ba4c3d241903653f91adf4764e) )
	ROM_CONTINUE(                0x50001, 0x08000 )
	ROM_LOAD16_BYTE( "10714.b6", 0x10000, 0x08000, CRC(91bf42fb) SHA1(4b9d3e97768323dee01e92378adafecb26bcc094) )
	ROM_CONTINUE(                0x50000, 0x08000 )
	ROM_LOAD16_BYTE( "10711.b3", 0x20001, 0x08000, CRC(29166ef6) SHA1(99a7cfd7d811537c821412a320beadb5a9c09af3) )
	ROM_CONTINUE(                0x60001, 0x08000 )
	ROM_LOAD16_BYTE( "10715.b7", 0x20000, 0x08000, CRC(a7c57384) SHA1(46f8efa691d7bbb0a18119c0ff12cff7c0d129e1) )
	ROM_CONTINUE(                0x60000, 0x08000 )
	ROM_LOAD16_BYTE( "10712.b4", 0x30001, 0x08000, CRC(876ad019) SHA1(39973ddb5a5746e0e094c759447bff1130c72c84) )
	ROM_CONTINUE(                0x70001, 0x08000 )
	ROM_LOAD16_BYTE( "10716.b8", 0x30000, 0x08000, CRC(40ba1d48) SHA1(e2d4d2689bb9b9bdc85e7f72a6665e5fd4c583aa) )
	ROM_CONTINUE(                0x70000, 0x08000 )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* sound CPU */
	ROM_LOAD( "10705", 0x00000, 0x8000, CRC(777b749e) SHA1(086b03100064a98228f95db7962b2671121c46ea) )

	ROM_REGION( 0x1000, "n7751", 0 )  /* 4k for 7751 onboard ROM */
	ROM_LOAD( "7751.bin",     0x0000, 0x0400, CRC(6a9534fc) SHA1(67ad94674db5c2aab75785668f610f6f4eccd158) ) /* 7751 - U34 */

	ROM_REGION( 0x18000, "n7751data", 0 ) /* 7751 sound data */
	ROM_LOAD( "10706", 0x00000, 0x8000, CRC(aa114acc) SHA1(81a2b3586ae90bc7fc55b82478ffe182ac49983e) )
	ROM_LOAD( "10707", 0x08000, 0x8000, CRC(800c1d82) SHA1(aac4123bd35f87da09264649f4cf8326b2ba3cb8) )
	ROM_LOAD( "10708", 0x10000, 0x8000, CRC(5921ef52) SHA1(eff9978361692e6e60a9c6caf5740dd6182cfe4a) )

	ROM_REGION( 0x2000, "fd1089a", 0 ) /* decryption key */
	ROM_LOAD( "317-0033.key", 0x0000, 0x2000, CRC(49e882e5) SHA1(29d87af8fc775b22a9a546c112f8f5e7f700ac1a) )
ROM_END

/**************************************************************************************************************************
    Alien Syndrome (Japan, old), System 16
    CPU: FD1089A 317-0033
 */
ROM_START( aliensynjo )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* Custom 68000 code */
	ROM_LOAD16_BYTE( "epr-10699.43", 0x00000, 0x8000, CRC(3fd38d17) SHA1(538c1246121051a1af9ba2a4259eb1fe7e4952e1) )
	ROM_LOAD16_BYTE( "epr-10696.26", 0x00001, 0x8000, CRC(d734f19f) SHA1(4a08c35084f7a9364ba0f058b9a9ffc30c8b5a78) )
	ROM_LOAD16_BYTE( "epr-10700.42", 0x10000, 0x8000, CRC(3b04b252) SHA1(0e40e89e8feb7c98ee1da1c3fb3fe1d317c66842) )
	ROM_LOAD16_BYTE( "epr-10697.25", 0x10001, 0x8000, CRC(f2bc123d) SHA1(7848529342495289e2d4f865767f3649cd85993b) )
	ROM_LOAD16_BYTE( "epr-10701.41", 0x20000, 0x8000, CRC(92171751) SHA1(69a282c01db7224f32386a6db25309e09e29a112) )
	ROM_LOAD16_BYTE( "epr-10698.24", 0x20001, 0x8000, CRC(c1e4fdc0) SHA1(65817a9336f7887d2bf14485bdff8352c960d2ab) )

	ROM_REGION( 0x30000, "gfx1", 0 ) /* tiles */
	ROM_LOAD( "epr-10739.95", 0x00000, 0x10000, CRC(a29ec207) SHA1(c469d2689a7bdc2a59dfff56ce13d34e9fbac263) )
	ROM_LOAD( "epr-10740.94", 0x10000, 0x10000, CRC(47f93015) SHA1(68247a6bffd1d4d1c450148dd46214d01ce1c668) )
	ROM_LOAD( "epr-10741.93", 0x20000, 0x10000, CRC(4970739c) SHA1(5bdf4222209ec46e0015bfc0f90578dd9b30bdd1) )

	ROM_REGION16_BE( 0x080000, "gfx2", 0 ) /* sprites */
	ROM_LOAD16_BYTE( "epr-10709.10", 0x00001, 0x08000, CRC(addf0a90) SHA1(a92c9531f1817763773471ce63f566b9e88360a0) )
	ROM_CONTINUE(                    0x40001, 0x08000 )
	ROM_LOAD16_BYTE( "epr-10713.11", 0x00000, 0x08000, CRC(ececde3a) SHA1(9c12d4665179bf433c42f5ddc8a043ad592aa90e) )
	ROM_CONTINUE(                    0x40000, 0x08000 )
	ROM_LOAD16_BYTE( "epr-10710.17", 0x10001, 0x08000, CRC(992369eb) SHA1(c6796acf6807e9ba4c3d241903653f91adf4764e) )
	ROM_CONTINUE(                    0x50001, 0x08000 )
	ROM_LOAD16_BYTE( "epr-10714.18", 0x10000, 0x08000, CRC(91bf42fb) SHA1(4b9d3e97768323dee01e92378adafecb26bcc094) )
	ROM_CONTINUE(                    0x50000, 0x08000 )
	ROM_LOAD16_BYTE( "epr-10711.23", 0x20001, 0x08000, CRC(29166ef6) SHA1(99a7cfd7d811537c821412a320beadb5a9c09af3) )
	ROM_CONTINUE(                    0x60001, 0x08000 )
	ROM_LOAD16_BYTE( "epr-10715.24", 0x20000, 0x08000, CRC(a7c57384) SHA1(46f8efa691d7bbb0a18119c0ff12cff7c0d129e1) )
	ROM_CONTINUE(                    0x60000, 0x08000 )
	ROM_LOAD16_BYTE( "epr-10712.29", 0x30001, 0x08000, CRC(876ad019) SHA1(39973ddb5a5746e0e094c759447bff1130c72c84) )
	ROM_CONTINUE(                    0x70001, 0x08000 )
	ROM_LOAD16_BYTE( "epr-10716.30", 0x30000, 0x08000, CRC(40ba1d48) SHA1(e2d4d2689bb9b9bdc85e7f72a6665e5fd4c583aa) )
	ROM_CONTINUE(                    0x70000, 0x08000 )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* sound CPU */
	ROM_LOAD( "epr-10705.12", 0x00000, 0x8000, CRC(777b749e) SHA1(086b03100064a98228f95db7962b2671121c46ea) )

	ROM_REGION( 0x1000, "n7751", 0 )  /* 4k for 7751 onboard ROM */
	ROM_LOAD( "7751.bin",     0x0000, 0x0400, CRC(6a9534fc) SHA1(67ad94674db5c2aab75785668f610f6f4eccd158) ) /* 7751 - U34 */

	ROM_REGION( 0x18000, "n7751data", 0 ) /* 7751 sound data */
	ROM_LOAD( "epr-10706.1",  0x00000, 0x8000, CRC(aa114acc) SHA1(81a2b3586ae90bc7fc55b82478ffe182ac49983e) )
	ROM_LOAD( "epr-10707.2",  0x08000, 0x8000, CRC(800c1d82) SHA1(aac4123bd35f87da09264649f4cf8326b2ba3cb8) )
	ROM_LOAD( "epr-10708.4",  0x10000, 0x8000, CRC(5921ef52) SHA1(eff9978361692e6e60a9c6caf5740dd6182cfe4a) )

	ROM_REGION( 0x2000, "fd1089a", 0 ) /* decryption key */
	ROM_LOAD( "317-0033.key", 0x0000, 0x2000, CRC(49e882e5) SHA1(29d87af8fc775b22a9a546c112f8f5e7f700ac1a) )
ROM_END


/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Body Slam, pre-System 16
    CPU: 68000
    i8751 317-0015
 */
ROM_START( bodyslam )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-10066.b9",  0x000000, 0x8000, CRC(6cd53290) SHA1(68ef83ad99a26a507d9bc4cd715462169f4ac41f) )
	ROM_LOAD16_BYTE( "epr-10063.b6",  0x000001, 0x8000, CRC(dd849a16) SHA1(b8cb9f2685a739698a3ed18f76617fd4ac9cb424) )
	ROM_LOAD16_BYTE( "epr-10067.b10", 0x010000, 0x8000, CRC(db22a5ce) SHA1(95c37d4913fa31d5edf02661681bc83deec731d9) )
	ROM_LOAD16_BYTE( "epr-10064.b7",  0x010001, 0x8000, CRC(53d6b7e0) SHA1(00bfa1487479629f60e1cc1b98ced47e4cb07964) )
	ROM_LOAD16_BYTE( "epr-10068.b11", 0x020000, 0x8000, CRC(15ccc665) SHA1(b088a9bcb1499854794b2dbf4c689f3ae3ce2808) )
	ROM_LOAD16_BYTE( "epr-10065.b8",  0x020001, 0x8000, CRC(0e5fa314) SHA1(44e36fde102ba6aef2c3b4374ddc21690f2fe527) )

	ROM_REGION( 0x18000, "gfx1", 0 ) /* tiles */
	ROM_LOAD( "epr-10321.c9",  0x00000, 0x8000, CRC(cd3e7cba) SHA1(4d3cfc7346c6e63e2221193601f949162d0e4f90) )
	ROM_LOAD( "epr-10322.c10", 0x08000, 0x8000, CRC(b53d3217) SHA1(baebf20925e9f8ab6660f041a24721716d5b7d92) )
	ROM_LOAD( "epr-10323.c11", 0x10000, 0x8000, CRC(915a3e61) SHA1(6504a8b26b7b4880971cd69ac2c8aae30dcfa18c) )

	ROM_REGION16_BE( 0x40000, "gfx2", 0 ) /* sprites */
	ROM_LOAD16_BYTE( "epr-10012.c5",  0x00001, 0x08000, CRC(990824e8) SHA1(bd45f75d07cb4e17583c2d76050e5f819f4b7efe) )
	ROM_LOAD16_BYTE( "epr-10016.b2",  0x00000, 0x08000, CRC(af5dc72f) SHA1(97bbb76940c702e642d8222dda71447b8f60b616) )
	ROM_LOAD16_BYTE( "epr-10013.c6",  0x10001, 0x08000, CRC(9a0919c5) SHA1(e39e60c1e834b3b46bf2ef1c5952841bebe66ade) )
	ROM_LOAD16_BYTE( "epr-10017.b3",  0x10000, 0x08000, CRC(62aafd95) SHA1(e1e3a95fd11cabf81f44ac2dd3f951d3094725e6) )
	ROM_LOAD16_BYTE( "epr-10027.c7",  0x20001, 0x08000, CRC(3f1c57c7) SHA1(1336da8dc167a323f09534a2f62ae6f9c62290e4) )
	ROM_LOAD16_BYTE( "epr-10028.b4",  0x20000, 0x08000, CRC(80d4946d) SHA1(d4c96a18ef6c2ac6bd9d153d8862a3af894642e8) )
	ROM_LOAD16_BYTE( "epr-10015.c8",  0x30001, 0x08000, CRC(582d3b6a) SHA1(4f1d0060682e3fc1147082286e00e6a296a95da2) )
	ROM_LOAD16_BYTE( "epr-10019.b5",  0x30000, 0x08000, CRC(e020c38b) SHA1(d13d38a64f2afa7df3cbccef2fe505a4421b73ad) )

	ROM_REGION( 0x30000, "soundcpu", 0 ) /* sound CPU */
	ROM_LOAD( "epr-10026.b1", 0x00000, 0x8000, CRC(123b69b8) SHA1(c0614a8c822991e257f7218908247df278056de8) )

	ROM_REGION( 0x1000, "n7751", 0 )  /* 4k for 7751 onboard ROM */
	ROM_LOAD( "7751.bin",     0x0000, 0x0400, CRC(6a9534fc) SHA1(67ad94674db5c2aab75785668f610f6f4eccd158) ) /* 7751 - U34 */

	ROM_REGION( 0x20000, "n7751data", 0 ) /* 7751 sound data */
	ROM_LOAD( "epr-10029.c1", 0x00000, 0x8000, CRC(7e4aca83) SHA1(703486b96d493941ee87267e8363220a851f008e) )
	ROM_LOAD( "epr-10030.c2", 0x08000, 0x8000, CRC(dcc1df0b) SHA1(a82a557fa48f4b3e1ab38f61b84d749cd417e80f) )
	ROM_LOAD( "epr-10031.c3", 0x10000, 0x8000, CRC(ea3c4472) SHA1(ad8eac2d3d14fd6aba713f4d624861c17aabf757) )
	ROM_LOAD( "epr-10032.c4", 0x18000, 0x8000, CRC(0aabebce) SHA1(fab12df8f4eab270be491c6c025d832c338e1e83) )

	ROM_REGION( 0x1000, "mcu", 0 )	/* Intel i8751 protection MCU */
	ROM_LOAD( "317-0015.bin", 0x0000, 0x1000, CRC(833869e2) SHA1(2675fda669351e958be28ca28de276abb2bbc99a) )
ROM_END

/**************************************************************************************************************************
    Dump Matsumoto, pre-System 16
    CPU: 68000
    i8751 317-0011A
 */
ROM_START( dumpmtmt )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-7704a.b9",  0x000000, 0x8000, CRC(96de6c7b) SHA1(f23edf86c5044c151a8502957af7ca0de420d55e) )
	ROM_LOAD16_BYTE( "epr-7701a.b6",  0x000001, 0x8000, CRC(786d1009) SHA1(c56ebd169c2792cde610a7130cffdc0363fca871) )
	ROM_LOAD16_BYTE( "epr-7705a.b10", 0x010000, 0x8000, CRC(fc584391) SHA1(27238408fba2dda67f29094a6700b634b6fdaa58) )
	ROM_LOAD16_BYTE( "epr-7702a.b7",  0x010001, 0x8000, CRC(2241a8fd) SHA1(d968ab57aa228dbb7ae6f17d7bf22991291e75ae) )
	ROM_LOAD16_BYTE( "epr-7706a.b11", 0x020000, 0x8000, CRC(6bbcc9d0) SHA1(e8e0b85867f11eec6b280f3ad9e2746d3d97ab28) )
	ROM_LOAD16_BYTE( "epr-7703a.b8",  0x020001, 0x8000, CRC(fcb0cd40) SHA1(999e107fe08fcb52729ddebc7714a85c47e748b1) )

	ROM_REGION( 0x18000, "gfx1", 0 ) /* tiles */
	ROM_LOAD( "epr-7707a.c9",  0x00000, 0x8000, CRC(45318738) SHA1(6885347321aec8c4829a71e4518d1742f939ea9c) )
	ROM_LOAD( "epr-7708a.c10", 0x08000, 0x8000, CRC(411be9a4) SHA1(808a9c941d353f34c3491ca2cde984e73cc7a87d) )
	ROM_LOAD( "epr-7709a.c11", 0x10000, 0x8000, CRC(74ceb5a8) SHA1(93ed6bb4a3c820f3a7ee5e9b2c2ce35d2bed8529) )

	ROM_REGION16_BE( 0x40000, "gfx2", 0 ) /* sprites */
	ROM_LOAD16_BYTE( "epr-7715.c5",  0x00001, 0x08000, CRC(bf47e040) SHA1(5aa1b9adaa2095844c10993402a0597bb5768efb) )
	ROM_LOAD16_BYTE( "epr-7719.b2",  0x00000, 0x08000, CRC(fa5c5d6c) SHA1(6cac5d3fd705d1365348d57a18bbeb1eb9e412b8) )
	ROM_LOAD16_BYTE( "epr-10013.c6", 0x10001, 0x08000, CRC(9a0919c5) SHA1(e39e60c1e834b3b46bf2ef1c5952841bebe66ade) )	/* 7716 */
	ROM_LOAD16_BYTE( "epr-10017.b3", 0x10000, 0x08000, CRC(62aafd95) SHA1(e1e3a95fd11cabf81f44ac2dd3f951d3094725e6) )	/* 7720 */
	ROM_LOAD16_BYTE( "epr-7717.c7",  0x20001, 0x08000, CRC(fa64c86d) SHA1(ada722dd6efbf466a719ee1fe34a36ce1ea20184) )
	ROM_LOAD16_BYTE( "epr-7721.b4",  0x20000, 0x08000, CRC(62a9143e) SHA1(28f0dc0329163f0a6505dd34a24a843b35118c5e) )
	ROM_LOAD16_BYTE( "epr-10015.c8", 0x30001, 0x08000, CRC(582d3b6a) SHA1(4f1d0060682e3fc1147082286e00e6a296a95da2) )	/* 7718 */
	ROM_LOAD16_BYTE( "epr-10019.b5", 0x30000, 0x08000, CRC(e020c38b) SHA1(d13d38a64f2afa7df3cbccef2fe505a4421b73ad) )	/* 7722 */

	ROM_REGION( 0x30000, "soundcpu", 0 ) /* sound CPU */
	ROM_LOAD( "epr-7710a.b1", 0x00000, 0x8000, CRC(a19b8ba8) SHA1(21b628d4ecbe38a6d96a39ca4252ff1cb728343f) )

	ROM_REGION( 0x1000, "n7751", 0 )      /* 4k for 7751 onboard ROM */
	ROM_LOAD( "7751.bin",     0x0000, 0x0400, CRC(6a9534fc) SHA1(67ad94674db5c2aab75785668f610f6f4eccd158) ) /* 7751 - U34 */

	ROM_REGION( 0x20000, "n7751data", 0 ) /* 7751 sound data */
	ROM_LOAD( "epr-7711.c1", 0x00000, 0x8000, CRC(efa9aabd) SHA1(b0928313b98159b95f3a6784c6279924774b9253) )
	ROM_LOAD( "epr-7712.c2", 0x08000, 0x8000, CRC(7bcd85cf) SHA1(9acba6998327e1074d7311a9b6d06da9baf69aa0) )
	ROM_LOAD( "epr-7713.c3", 0x10000, 0x8000, CRC(33f292e7) SHA1(4358cd3922a0dcbf109d2d697c7b8c4e090c3d52) )
	ROM_LOAD( "epr-7714.c4", 0x18000, 0x8000, CRC(8fd48c47) SHA1(1cba63a9e7e0b477683b7758d124f4949558ba7a) )

	ROM_REGION( 0x1000, "mcu", 0 )	/* protection MCU */
	ROM_LOAD( "317-0011a.mcu", 0x00000, 0x1000, NO_DUMP )
ROM_END


/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Fantasy Zone, Sega System 16A
    CPU: 68000
 */
ROM_START( fantzone )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-7385a.43", 0x000000, 0x8000, CRC(4091af42) SHA1(1d4fdd32db9f75e5ccaab5766a50249ad71a60af) )
	ROM_LOAD16_BYTE( "epr-7382a.26", 0x000001, 0x8000, CRC(77d67bfd) SHA1(886ce4c2d779cedd81f85737ef55fce3c94baa18) )
	ROM_LOAD16_BYTE( "epr-7386a.42", 0x010000, 0x8000, CRC(b0a67cd0) SHA1(2e2bf2b7306fc567f7d13f89977543b368c19027) )
	ROM_LOAD16_BYTE( "epr-7383a.25", 0x010001, 0x8000, CRC(5f79b2a9) SHA1(de3125bbd0a126fc5a67ba3134cd3f4608ebdfce) )
	ROM_LOAD16_BYTE( "epr-7387.41",  0x020000, 0x8000, CRC(0acd335d) SHA1(f39566a2069eefa7682c57c6521ea7a328738d06) )
	ROM_LOAD16_BYTE( "epr-7384.24",  0x020001, 0x8000, CRC(fd909341) SHA1(2f1e01eb7d7b330c9c0dd98e5f8ed4973f0e93fb) )

	ROM_REGION( 0x18000, "gfx1", 0 ) /* tiles */
	ROM_LOAD( "epr-7388.95",  0x00000, 0x08000, CRC(8eb02f6b) SHA1(80511b944b57541669010bd5a0ca52bc98eabd62) )
	ROM_LOAD( "epr-7389.94",  0x08000, 0x08000, CRC(2f4f71b8) SHA1(ceb39e95cd43904b8e4f89c7227491e139fb3ca6) )
	ROM_LOAD( "epr-7390.93",  0x10000, 0x08000, CRC(d90609c6) SHA1(4232f6ecb21f242c0c8d81e06b88bc742668609f) )

	ROM_REGION16_BE( 0x30000, "gfx2", 0 ) /* sprites */
	ROM_LOAD16_BYTE( "epr-7392.10",  0x00001, 0x8000, CRC(5bb7c8b6) SHA1(eaa0ed63ac4f66ee285757e842bdd7b005292600) )
	ROM_LOAD16_BYTE( "epr-7396.11",  0x00000, 0x8000, CRC(74ae4b57) SHA1(1f24b1faea765994b85f0e7ac8e944c8da22103f) )
	ROM_LOAD16_BYTE( "epr-7393.17",  0x10001, 0x8000, CRC(14fc7e82) SHA1(ca7caca989a3577dd30ad4f66b0fcce712a454ef) )
	ROM_LOAD16_BYTE( "epr-7397.18",  0x10000, 0x8000, CRC(e05a1e25) SHA1(9691d9f0763b7483ee6912437902f22ab4b78a05) )
	ROM_LOAD16_BYTE( "epr-7394.23",  0x20001, 0x8000, CRC(531ca13f) SHA1(19e68bc515f6021e1145cff4f3f0e083839ee8f3) )
	ROM_LOAD16_BYTE( "epr-7398.24",  0x20000, 0x8000, CRC(68807b49) SHA1(0a189da8cdd2090e76d6d06c55b478abce60542d) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* sound CPU */
	ROM_LOAD( "epr-7535a.12", 0x0000, 0x8000, CRC(bc1374fa) SHA1(ed2c87ae024dc251e175239f1bccc728fc096548) )
ROM_END

/**************************************************************************************************************************
    Fantasy Zone, Sega System 16A
    CPU: 68000
 */
ROM_START( fantzone1 )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-7385.43",  0x000000, 0x8000, CRC(5cb64450) SHA1(5831405359975dd7d8c6614b20fd9b18a5d6410d) )
	ROM_LOAD16_BYTE( "epr-7382.26",  0x000001, 0x8000, CRC(3fda7416) SHA1(91f34cc8afb4ad8bc783c31d25781a1359c44cfe) )
	ROM_LOAD16_BYTE( "epr-7386.42",  0x010000, 0x8000, CRC(15810ace) SHA1(e61a258ab6601d359f6ad1f37a2b2801bf777d26) )
	ROM_LOAD16_BYTE( "epr-7383.25",  0x010001, 0x8000, CRC(a001e10a) SHA1(04ebb012b10817db36997d0ee877104d512decf8) )
	ROM_LOAD16_BYTE( "epr-7387.41",  0x020000, 0x8000, CRC(0acd335d) SHA1(f39566a2069eefa7682c57c6521ea7a328738d06) )
	ROM_LOAD16_BYTE( "epr-7384.24",  0x020001, 0x8000, CRC(fd909341) SHA1(2f1e01eb7d7b330c9c0dd98e5f8ed4973f0e93fb) )

	ROM_REGION( 0x18000, "gfx1", 0 ) /* tiles */
	ROM_LOAD( "epr-7388.95",  0x00000, 0x08000, CRC(8eb02f6b) SHA1(80511b944b57541669010bd5a0ca52bc98eabd62) )
	ROM_LOAD( "epr-7389.94",  0x08000, 0x08000, CRC(2f4f71b8) SHA1(ceb39e95cd43904b8e4f89c7227491e139fb3ca6) )
	ROM_LOAD( "epr-7390.93",  0x10000, 0x08000, CRC(d90609c6) SHA1(4232f6ecb21f242c0c8d81e06b88bc742668609f) )

	ROM_REGION16_BE( 0x30000, "gfx2", 0 ) /* sprites */
	ROM_LOAD16_BYTE( "epr-7392.10",  0x00001, 0x8000, CRC(5bb7c8b6) SHA1(eaa0ed63ac4f66ee285757e842bdd7b005292600) )
	ROM_LOAD16_BYTE( "epr-7396.11",  0x00000, 0x8000, CRC(74ae4b57) SHA1(1f24b1faea765994b85f0e7ac8e944c8da22103f) )
	ROM_LOAD16_BYTE( "epr-7393.17",  0x10001, 0x8000, CRC(14fc7e82) SHA1(ca7caca989a3577dd30ad4f66b0fcce712a454ef) )
	ROM_LOAD16_BYTE( "epr-7397.18",  0x10000, 0x8000, CRC(e05a1e25) SHA1(9691d9f0763b7483ee6912437902f22ab4b78a05) )
	ROM_LOAD16_BYTE( "epr-7394.23",  0x20001, 0x8000, CRC(531ca13f) SHA1(19e68bc515f6021e1145cff4f3f0e083839ee8f3) )
	ROM_LOAD16_BYTE( "epr-7398.24",  0x20000, 0x8000, CRC(68807b49) SHA1(0a189da8cdd2090e76d6d06c55b478abce60542d) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* sound CPU */
	ROM_LOAD( "epr-7535.12",  0x0000, 0x8000, CRC(0cb2126a) SHA1(42b18a81bed58ef59eaad929007eef89ad273dbb) )
ROM_END

/**************************************************************************************************************************
    Fantasy Zone, Sega System 16A
    CPU: 68000 + NEC 0317-5000 encrypted Z80
 */
ROM_START( fantzonep )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-7385.43",  0x000000, 0x8000, CRC(5cb64450) SHA1(5831405359975dd7d8c6614b20fd9b18a5d6410d) )
	ROM_LOAD16_BYTE( "epr-7382.26",  0x000001, 0x8000, CRC(3fda7416) SHA1(91f34cc8afb4ad8bc783c31d25781a1359c44cfe) )
	ROM_LOAD16_BYTE( "epr-7386.42",  0x010000, 0x8000, CRC(15810ace) SHA1(e61a258ab6601d359f6ad1f37a2b2801bf777d26) )
	ROM_LOAD16_BYTE( "epr-7383.25",  0x010001, 0x8000, CRC(a001e10a) SHA1(04ebb012b10817db36997d0ee877104d512decf8) )
	ROM_LOAD16_BYTE( "epr-7387.41",  0x020000, 0x8000, CRC(0acd335d) SHA1(f39566a2069eefa7682c57c6521ea7a328738d06) )
	ROM_LOAD16_BYTE( "epr-7384.24",  0x020001, 0x8000, CRC(fd909341) SHA1(2f1e01eb7d7b330c9c0dd98e5f8ed4973f0e93fb) )

	ROM_REGION( 0x18000, "gfx1", 0 ) /* tiles */
	ROM_LOAD( "epr-7388.95",  0x00000, 0x08000, CRC(8eb02f6b) SHA1(80511b944b57541669010bd5a0ca52bc98eabd62) )
	ROM_LOAD( "epr-7389.94",  0x08000, 0x08000, CRC(2f4f71b8) SHA1(ceb39e95cd43904b8e4f89c7227491e139fb3ca6) )
	ROM_LOAD( "epr-7390.93",  0x10000, 0x08000, CRC(d90609c6) SHA1(4232f6ecb21f242c0c8d81e06b88bc742668609f) )

	ROM_REGION16_BE( 0x30000, "gfx2", 0 ) /* sprites */
	ROM_LOAD16_BYTE( "epr-7392.10",  0x00001, 0x8000, CRC(5bb7c8b6) SHA1(eaa0ed63ac4f66ee285757e842bdd7b005292600) )
	ROM_LOAD16_BYTE( "epr-7396.11",  0x00000, 0x8000, CRC(74ae4b57) SHA1(1f24b1faea765994b85f0e7ac8e944c8da22103f) )
	ROM_LOAD16_BYTE( "epr-7393.17",  0x10001, 0x8000, CRC(14fc7e82) SHA1(ca7caca989a3577dd30ad4f66b0fcce712a454ef) )
	ROM_LOAD16_BYTE( "epr-7397.18",  0x10000, 0x8000, CRC(e05a1e25) SHA1(9691d9f0763b7483ee6912437902f22ab4b78a05) )
	ROM_LOAD16_BYTE( "epr-7394.23",  0x20001, 0x8000, CRC(531ca13f) SHA1(19e68bc515f6021e1145cff4f3f0e083839ee8f3) )
	ROM_LOAD16_BYTE( "epr-7398.24",  0x20000, 0x8000, CRC(68807b49) SHA1(0a189da8cdd2090e76d6d06c55b478abce60542d) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* sound CPU */
	ROM_LOAD( "epr-7391.12",  0x0000, 0x8000, CRC(c03e155e) SHA1(6bbdf308c47b96ba622329520b70fa0ea47248a4) ) /* Original encrypted sound code */
ROM_END


/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Major League, pre-System 16
    CPU: 68000 (version uses i8751(315-5158C) known to be exist)
 */
ROM_START( mjleague )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-7404.9b",   0x000000, 0x8000, CRC(ec1655b5) SHA1(5c1df364fa9733daa4478c5f88298089e4963c33) )
	ROM_LOAD16_BYTE( "epr-7401.6b",   0x000001, 0x8000, CRC(2befa5e0) SHA1(0a1681a4c7d62a5754ba6f3845436b4d08324246) )
	ROM_LOAD16_BYTE( "epr-7405.10b",  0x010000, 0x8000, CRC(7a4f4e38) SHA1(65a22097dd933e83f326bd64b3863915897780a6) )
	ROM_LOAD16_BYTE( "epr-7402.7b",   0x010001, 0x8000, CRC(b7bef762) SHA1(214450e0b094f99ef38dec2a3e5cbdb0b30e917d) )
	ROM_LOAD16_BYTE( "epr-7406a.11b", 0x020000, 0x8000, CRC(bb743639) SHA1(5d99638a79f02ce14374d3b1f3d9fbfc5c13c6e1) )
	ROM_LOAD16_BYTE( "epr-7403a.8b",  0x020001, 0x8000, CRC(0a39a4d0) SHA1(ff6c579a69fe20c4ebe8672f1a57a6efa9982fb6) )

	ROM_REGION( 0x18000, "gfx1", 0 ) /* tiles */
	ROM_LOAD( "epr-7051.9a",  0x00000, 0x08000, CRC(10ca255a) SHA1(ccf58ffcac2f7fbdbfbdf32601a1b97f359cbd91) )
	ROM_LOAD( "epr-7052.10a", 0x08000, 0x08000, CRC(2550db0e) SHA1(28f8d68f43d26f12793fe295c205cc86adc4e96a) )
	ROM_LOAD( "epr-7053.11a", 0x10000, 0x08000, CRC(5bfea038) SHA1(01dc6e14cc7bba9f7930e68573c441fa2841f49a) )

	ROM_REGION16_BE( 0x40000, "gfx2", 0 ) /* sprites */
	ROM_LOAD16_BYTE( "epr-7055.5a", 0x00001, 0x8000, CRC(1fb860bd) SHA1(4a4155d0352dfae9e402a2b2f1558ef17b1303b4) )
	ROM_LOAD16_BYTE( "epr-7059.2b", 0x00000, 0x8000, CRC(3d14091d) SHA1(36208415b2012b6e948fefa15b0f7041748066be) )
	ROM_LOAD16_BYTE( "epr-7056.6a", 0x10001, 0x8000, CRC(b35dd968) SHA1(e306b5e38acf583d7b2089302622ad25ae5564b0) )
	ROM_LOAD16_BYTE( "epr-7060.3b", 0x10000, 0x8000, CRC(61bb3757) SHA1(5c87cf23be22b84e3dae746527ca057d870d6397) )
	ROM_LOAD16_BYTE( "epr-7057.7a", 0x20001, 0x8000, CRC(3e5a2b6f) SHA1(d3dbafb4acb916e02c978a156008bd75ba122fb7) )
	ROM_LOAD16_BYTE( "epr-7061.4b", 0x20000, 0x8000, CRC(c808dad5) SHA1(9b65acc8dc23b16e56327298188d1a6ab48b2b5d) )
	ROM_LOAD16_BYTE( "epr-7058.8a", 0x30001, 0x8000, CRC(b543675f) SHA1(35ffc9295a8849a18fabe156fdbc9801ea2179cd) )
	ROM_LOAD16_BYTE( "epr-7062.5b", 0x30000, 0x8000, CRC(9168eb47) SHA1(daaa7836e627a0679e65373d8f20a9383ba4c905) )

	ROM_REGION( 0x20000, "soundcpu", 0 ) /* sound CPU */
	ROM_LOAD( "epr-7054c.1b", 0x00000, 0x8000, CRC(4443b744) SHA1(73359a6e9d62b382dee47fea31b9e17eb26a0321) )

	ROM_REGION( 0x1000, "n7751", 0 )      /* 4k for 7751 onboard ROM */
	ROM_LOAD( "7751.bin",     0x0000, 0x0400, CRC(6a9534fc) SHA1(67ad94674db5c2aab75785668f610f6f4eccd158) ) /* 7751 - U34 */

	ROM_REGION( 0x20000, "n7751data", 0 ) /* 7751 sound data */
	ROM_LOAD( "epr-7063.1a", 0x00000, 0x8000, CRC(45d8908a) SHA1(e61f81f953c1a744ded36fed3b55774e4747af29) )
	ROM_LOAD( "epr-7065.2a", 0x08000, 0x8000, CRC(8c8f8cff) SHA1(fca5a916a8b25800ee5e8771e2ced0ed9bd737f4) )
	ROM_LOAD( "epr-7064.3a", 0x10000, 0x8000, CRC(159f6636) SHA1(66fa3f3e95a6ef3d3ff4ded09c05ab1131d9fbbb) )
	ROM_LOAD( "epr-7066.4a", 0x18000, 0x8000, CRC(f5cfa91f) SHA1(c85d68cbcd03fe1436bed12235c033610acc11ee) )
ROM_END


/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Passing Shot (Japan, 4 Players), Sega System 16A
    CPU: FD1094 (317-0071)
 */

ROM_START( passsht16a )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-11833.43", 0x000000, 0x10000, CRC(5eb1405c) SHA1(0a68d3fcc074475d38f999c93082d4a9dff0f19a) )
	ROM_LOAD16_BYTE( "epr-11832.26", 0x000001, 0x10000, CRC(718a3fe4) SHA1(bd6844c53ce3b64b113795360175df92d095b467) )

	ROM_REGION( 0x2000, "user1", 0 )	/* decryption key */
	ROM_LOAD( "317-0071.key", 0x0000, 0x2000, CRC(c69949ec) SHA1(1c63f42404ee1d8333e734e892b1c4cac0cb440e) )

	ROM_REGION( 0x30000, "gfx1", 0 ) /* tiles */
	ROM_LOAD( "epr-11834.95", 0x00000, 0x10000, CRC(df4e18ab) SHA1(ac9b424eded18f128e6cc743bbddf9afd869c8c4) )
	ROM_LOAD( "epr-11835.94", 0x10000, 0x10000, CRC(6a07acc0) SHA1(218071612ee6fa89b16a47a77325a962ba38926d) )
	ROM_LOAD( "epr-11836.93", 0x20000, 0x10000, CRC(93c74928) SHA1(43ea7855d5d4dcc4921b3a7b814acc75c5cfde15) )

	ROM_REGION16_BE( 0x080000, "gfx2", 0 ) /* sprites */
	ROM_LOAD16_BYTE( "epr-11842.10", 0x00001, 0x08000, CRC(b6e94727) SHA1(0838e034f1f10d9cd1312c8c94b5c57387c0c271) )
	ROM_CONTINUE(                    0x40001, 0x08000 )
	ROM_LOAD16_BYTE( "epr-11845.11", 0x00000, 0x08000, CRC(17e8d5d5) SHA1(ac1074b0a705be13c6e3391441e6cfec1d2b3f8a) )
	ROM_CONTINUE(                    0x40000, 0x08000 )
	ROM_LOAD16_BYTE( "epr-11843.17", 0x10001, 0x08000, CRC(3e670098) SHA1(2cfc83f4294be30cd868738886ac546bd8489962) )
	ROM_CONTINUE(                    0x50001, 0x08000 )
	ROM_LOAD16_BYTE( "epr-11846.18", 0x10000, 0x08000, CRC(50eb71cc) SHA1(463b4917ca19c7f4ad2c2845caa104d5e4a2dda3) )
	ROM_CONTINUE(                    0x50000, 0x08000 )
	ROM_LOAD16_BYTE( "epr-11844.23", 0x20001, 0x08000, CRC(05733ca8) SHA1(1dbc7c99450ebe6a9fd8c0244fd3cb38b74984ef) )
	ROM_CONTINUE(                    0x60001, 0x08000 )
	ROM_LOAD16_BYTE( "epr-11847.24", 0x20000, 0x08000, CRC(81e49697) SHA1(a70fa409e3555ad6c8f28930a7026fdf2deb8c65) )
	ROM_CONTINUE(                    0x60000, 0x08000 )

	ROM_REGION( 0x20000, "soundcpu", 0 ) /* sound CPU */
	ROM_LOAD( "epr-11837.12", 0x0000, 0x8000, CRC(74d11552) SHA1(5a0f0c3fb858ed2bad8002fce4e29d730f102bcd) )

	ROM_REGION( 0x1000, "n7751", 0 )      /* 4k for 7751 onboard ROM */
	ROM_LOAD( "7751.bin",     0x0000, 0x0400, CRC(6a9534fc) SHA1(67ad94674db5c2aab75785668f610f6f4eccd158) ) /* 7751 - U34 */

	ROM_REGION( 0x20000, "n7751data", 0 ) /* 7751 sound data */
	ROM_LOAD( "epr-11838.1",  0x00000, 0x8000, CRC(a465cd69) SHA1(25da3809824fb3df1f93bbfa88355a7c50e44211) )
	ROM_LOAD( "epr-11839.2",  0x08000, 0x8000, CRC(99de6197) SHA1(f7de6a34fa185754c12276a94b1513234d352f3f) )
	ROM_LOAD( "epr-11840.4",  0x10000, 0x8000, CRC(9854e8b3) SHA1(bc9d8a17ff96cf03f9a955223c11d9f1fb0309c5) )
	ROM_LOAD( "epr-11841.5",  0x18000, 0x8000, CRC(1e89877e) SHA1(bbceb4f3cafae40d1e256f4336d3ee0bcdf8d077) )
ROM_END


/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Quartet, pre-System 16
    CPU: 68000
    i8751 315-5194
 */
ROM_START( quartet )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-7458a.9b",  0x000000, 0x8000, CRC(42e7b23e) SHA1(9df3b1b915723f9a927ef03d80ae7983a8c91a21) )
	ROM_LOAD16_BYTE( "epr-7455a.6b",  0x000001, 0x8000, CRC(01631ab2) SHA1(2d613d23fe79072f850ccc9020830dea54312b23) )
	ROM_LOAD16_BYTE( "epr-7459a.10b", 0x010000, 0x8000, CRC(6b540637) SHA1(4b2e9ba06b80f8fb502310ab770805f8c6a47567) )
	ROM_LOAD16_BYTE( "epr-7456a.7b",  0x010001, 0x8000, CRC(31ca583e) SHA1(8ade8f7e42ae3e171b138410374e4c090fdc4ecb) )
	ROM_LOAD16_BYTE( "epr-7460.11b",  0x020000, 0x8000, CRC(a444ea13) SHA1(884ed22d606e3bd30d8401fe1750687e54674e82) )
	ROM_LOAD16_BYTE( "epr-7457.8b",   0x020001, 0x8000, CRC(3b282c23) SHA1(95de41a97f50f6169887c6d9724d5c42a41bb264) )

	ROM_REGION( 0x18000, "gfx1", 0 ) /* tiles */
	ROM_LOAD( "epr-7461.9c",  0x00000, 0x08000, CRC(f6af07f2) SHA1(546fabbda936d61a90d2395d033fd4d6bb0bc38a) )
	ROM_LOAD( "epr-7462.10c", 0x08000, 0x08000, CRC(7914af28) SHA1(4bf59fe4a0b0aa5d4cc0b6f9375ffab3c96e8a2b) )
	ROM_LOAD( "epr-7463.11c", 0x10000, 0x08000, CRC(827c5603) SHA1(8db3bd6eae5aeeb229e017471049ef5347974df5) )

	ROM_REGION16_BE( 0x40000, "gfx2", 0 ) /* sprites  - the same as quartet 2 */
	ROM_LOAD16_BYTE( "epr-7465.5c",  0x00001, 0x8000, CRC(8a1ab7d7) SHA1(a2f317538c70a1603b65d795223407cbaaf88524) )
	ROM_LOAD16_BYTE( "epr-7469.2b",  0x00000, 0x8000, CRC(cb65ae4f) SHA1(3ee7b3b4cce113a6f394e8dfd317cdb6ffae64f7) )
	ROM_LOAD16_BYTE( "epr-7466.6c",  0x10001, 0x8000, CRC(b2d3f4f3) SHA1(65e654fde10bee4cb5eee8234d0babb78fe41cfb) )
	ROM_LOAD16_BYTE( "epr-7470.3b",  0x10000, 0x8000, CRC(16fc67b1) SHA1(788fe2878c5c9faea43c2f166f32c22ee51c7d09) )
	ROM_LOAD16_BYTE( "epr-7467.7c",  0x20001, 0x8000, CRC(0af68de2) SHA1(81163baf3f0e45bac950a6d9c24b3a886db1509c) )
	ROM_LOAD16_BYTE( "epr-7471.4b",  0x20000, 0x8000, CRC(13fad5ac) SHA1(75b480083fbb14cbef969126989bf9b2235fd31e) )
	ROM_LOAD16_BYTE( "epr-7468.8c",  0x30001, 0x8000, CRC(ddfd40c0) SHA1(6c12ad668cd0c82e7d7d46bfbdcee8b9d46ebd09) )
	ROM_LOAD16_BYTE( "epr-7472.5b",  0x30000, 0x8000, CRC(8e2762ec) SHA1(872e19a6aab81d7a2472367d0e31dc1295da7182) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* sound CPU */
	ROM_LOAD( "epr-7464.1b",  0x0000, 0x8000, CRC(9f291306) SHA1(96a09542a863ccf2ded43e2df6f913722b3f97b1) )

	ROM_REGION( 0x1000, "n7751", 0 )      /* 4k for 7751 onboard ROM */
	ROM_LOAD( "7751.bin",     0x0000, 0x0400, CRC(6a9534fc) SHA1(67ad94674db5c2aab75785668f610f6f4eccd158) ) /* 7751 - U34 */

	ROM_REGION( 0x20000, "n7751data", 0 ) /* 7751 sound data */
	ROM_LOAD( "epr-7473.1c",  0x00000, 0x8000, CRC(06ec75fa) SHA1(5f14bc887449122700c46ad22c0379a1682e0bdb) )
	ROM_LOAD( "epr-7475.2c",  0x08000, 0x8000, CRC(7abd1206) SHA1(54d52dc0b9c245cd2df647e714310a71b803cbcf) )
	ROM_LOAD( "epr-7474.3c",  0x10000, 0x8000, CRC(dbf853b8) SHA1(e82f497e1144f23f3233b5c45ef182bfc7923715) )
	ROM_LOAD( "epr-7476.4c",  0x18000, 0x8000, CRC(5eba655a) SHA1(6713ef12037cba3139d0f469c82bd90b44bae8ce) )

	ROM_REGION( 0x1000, "mcu", 0 )	/* Intel i8751 protection MCU */
	ROM_LOAD( "315-5194.mcu", 0x00000, 0x1000, NO_DUMP )

	ROM_REGION( 0x0500, "plds", 0 )
	ROM_LOAD( "pal16r6a.22g", 0x0000, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16r6a.23g", 0x0200, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pls153.8j",    0x0400, 0x00eb, CRC(0fe1eefd) SHA1(38ba6f57006af8c0d4d7f74aa4778ac1ee2a21fc) )
ROM_END

/**************************************************************************************************************************
    Quartet, pre-System 16
    CPU: 68000
    i8751 315-5194
 */
ROM_START( quarteta )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-7458.9b",  0x000000, 0x8000, CRC(0096499f) SHA1(dcf8e33513ce7c6660ea546c8e1c574fde629a22) )
	ROM_LOAD16_BYTE( "epr-7455.6b",  0x000001, 0x8000, CRC(da934390) SHA1(d40eb65b6a36a4c1ebeadb76e47a61bd8b2e4b89) )
	ROM_LOAD16_BYTE( "epr-7459.10b", 0x010000, 0x8000, CRC(d130cf61) SHA1(3a065f5c296b10b97c78d49aa285ae7afb16e881) )
	ROM_LOAD16_BYTE( "epr-7456.7b",  0x010001, 0x8000, CRC(7847149f) SHA1(fc8ad669f2bc426cb7af78d92ea147cbd1e181af) )
	ROM_LOAD16_BYTE( "epr-7460.11b", 0x020000, 0x8000, CRC(a444ea13) SHA1(884ed22d606e3bd30d8401fe1750687e54674e82) )
	ROM_LOAD16_BYTE( "epr-7457.8b",  0x020001, 0x8000, CRC(3b282c23) SHA1(95de41a97f50f6169887c6d9724d5c42a41bb264) )

	ROM_REGION( 0x18000, "gfx1", 0 ) /* tiles */
	ROM_LOAD( "epr-7461.9c",  0x00000, 0x08000, CRC(f6af07f2) SHA1(546fabbda936d61a90d2395d033fd4d6bb0bc38a) )
	ROM_LOAD( "epr-7462.10c", 0x08000, 0x08000, CRC(7914af28) SHA1(4bf59fe4a0b0aa5d4cc0b6f9375ffab3c96e8a2b) )
	ROM_LOAD( "epr-7463.11c", 0x10000, 0x08000, CRC(827c5603) SHA1(8db3bd6eae5aeeb229e017471049ef5347974df5) )

	ROM_REGION16_BE( 0x40000, "gfx2", 0 ) /* sprites  - the same as quartet 2 */
	ROM_LOAD16_BYTE( "epr-7465.5c",  0x00001, 0x8000, CRC(8a1ab7d7) SHA1(a2f317538c70a1603b65d795223407cbaaf88524) )
	ROM_LOAD16_BYTE( "epr-7469.2b",  0x00000, 0x8000, CRC(cb65ae4f) SHA1(3ee7b3b4cce113a6f394e8dfd317cdb6ffae64f7) )
	ROM_LOAD16_BYTE( "epr-7466.6c",  0x10001, 0x8000, CRC(b2d3f4f3) SHA1(65e654fde10bee4cb5eee8234d0babb78fe41cfb) )
	ROM_LOAD16_BYTE( "epr-7470.3b",  0x10000, 0x8000, CRC(16fc67b1) SHA1(788fe2878c5c9faea43c2f166f32c22ee51c7d09) )
	ROM_LOAD16_BYTE( "epr-7467.7c",  0x20001, 0x8000, CRC(0af68de2) SHA1(81163baf3f0e45bac950a6d9c24b3a886db1509c) )
	ROM_LOAD16_BYTE( "epr-7471.4b",  0x20000, 0x8000, CRC(13fad5ac) SHA1(75b480083fbb14cbef969126989bf9b2235fd31e) )
	ROM_LOAD16_BYTE( "epr-7468.8c",  0x30001, 0x8000, CRC(ddfd40c0) SHA1(6c12ad668cd0c82e7d7d46bfbdcee8b9d46ebd09) )
	ROM_LOAD16_BYTE( "epr-7472.5b",  0x30000, 0x8000, CRC(8e2762ec) SHA1(872e19a6aab81d7a2472367d0e31dc1295da7182) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* sound CPU */
	ROM_LOAD( "epr-7464.1b",  0x0000, 0x8000, CRC(9f291306) SHA1(96a09542a863ccf2ded43e2df6f913722b3f97b1) )

	ROM_REGION( 0x1000, "n7751", 0 )      /* 4k for 7751 onboard ROM */
	ROM_LOAD( "7751.bin",     0x0000, 0x0400, CRC(6a9534fc) SHA1(67ad94674db5c2aab75785668f610f6f4eccd158) ) /* 7751 - U34 */

	ROM_REGION( 0x20000, "n7751data", 0 ) /* 7751 sound data */
	ROM_LOAD( "epr-7473.1c",  0x00000, 0x8000, CRC(06ec75fa) SHA1(5f14bc887449122700c46ad22c0379a1682e0bdb) )
	ROM_LOAD( "epr-7475.2c",  0x08000, 0x8000, CRC(7abd1206) SHA1(54d52dc0b9c245cd2df647e714310a71b803cbcf) )
	ROM_LOAD( "epr-7474.3c",  0x10000, 0x8000, CRC(dbf853b8) SHA1(e82f497e1144f23f3233b5c45ef182bfc7923715) )
	ROM_LOAD( "epr-7476.4c",  0x18000, 0x8000, CRC(5eba655a) SHA1(6713ef12037cba3139d0f469c82bd90b44bae8ce) )

	ROM_REGION( 0x1000, "mcu", 0 )	/* Intel i8751 protection MCU */
	ROM_LOAD( "315-5194.mcu", 0x00000, 0x1000, NO_DUMP )

	ROM_REGION( 0x0500, "plds", 0 )
	ROM_LOAD( "pal16r6a.22g", 0x0000, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16r6a.23g", 0x0200, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pls153.8j",    0x0400, 0x00eb, CRC(0fe1eefd) SHA1(38ba6f57006af8c0d4d7f74aa4778ac1ee2a21fc) )
ROM_END


/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Quartet 2, pre-System 16
    CPU: 68000
    i8751 317-0010

    CPU/Video/Sound Board: 171-5335
    ROM Board:             171-5336
 */
ROM_START( quartet2 )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-7695.b9",  0x000000, 0x8000, CRC(67177cd8) SHA1(c4ea001dfbeeb29a09d597fb50d71f54e4e9572a) )
	ROM_LOAD16_BYTE( "epr-7692.b6",  0x000001, 0x8000, CRC(50f50b08) SHA1(646c0d545150b95e5d8d47bf63360f7326add08f) )
	ROM_LOAD16_BYTE( "epr-7696.b10", 0x010000, 0x8000, CRC(4273c3b7) SHA1(4cae221678a6d2b7806487becd4ba09b520f9fa0) )
	ROM_LOAD16_BYTE( "epr-7693.b7",  0x010001, 0x8000, CRC(0aa337bb) SHA1(f31f8f294fccd866eadebfafee067bfae44b3184) )
	ROM_LOAD16_BYTE( "epr-7697.b11", 0x020000, 0x8000, CRC(3a6a375d) SHA1(8ebea6b7f1208438b47e887b46cb569725c4042a) )
	ROM_LOAD16_BYTE( "epr-7694.b8",  0x020001, 0x8000, CRC(d87b2ca2) SHA1(58adf0900e41036b1b78a931ab94b30ce601909d) )

	ROM_REGION( 0x18000, "gfx1", 0 ) /* tiles */
	ROM_LOAD( "epr-7698.c9",  0x00000, 0x08000, CRC(547a6058) SHA1(5248e974c8d12183c996b1fc8fda09e8a4bf0d2d) )
	ROM_LOAD( "epr-7699.c10", 0x08000, 0x08000, CRC(77ec901d) SHA1(b5961895473c16a8f4a111185cce48b05ab66885) )
	ROM_LOAD( "epr-7700.c11", 0x10000, 0x08000, CRC(7e348cce) SHA1(82bba65280faaf3280208c85caef48ec8baeade8) )

	ROM_REGION16_BE( 0x040000, "gfx2", 0 ) /* sprites */
	ROM_LOAD16_BYTE( "epr-7465.5c",  0x00001, 0x8000, CRC(8a1ab7d7) SHA1(a2f317538c70a1603b65d795223407cbaaf88524) )
	ROM_LOAD16_BYTE( "epr-7469.2b",  0x00000, 0x8000, CRC(cb65ae4f) SHA1(3ee7b3b4cce113a6f394e8dfd317cdb6ffae64f7) )
	ROM_LOAD16_BYTE( "epr-7466.6c",  0x10001, 0x8000, CRC(b2d3f4f3) SHA1(65e654fde10bee4cb5eee8234d0babb78fe41cfb) )
	ROM_LOAD16_BYTE( "epr-7470.3b",  0x10000, 0x8000, CRC(16fc67b1) SHA1(788fe2878c5c9faea43c2f166f32c22ee51c7d09) )
	ROM_LOAD16_BYTE( "epr-7467.7c",  0x20001, 0x8000, CRC(0af68de2) SHA1(81163baf3f0e45bac950a6d9c24b3a886db1509c) )
	ROM_LOAD16_BYTE( "epr-7471.4b",  0x20000, 0x8000, CRC(13fad5ac) SHA1(75b480083fbb14cbef969126989bf9b2235fd31e) )
	ROM_LOAD16_BYTE( "epr-7468.8c",  0x30001, 0x8000, CRC(ddfd40c0) SHA1(6c12ad668cd0c82e7d7d46bfbdcee8b9d46ebd09) )
	ROM_LOAD16_BYTE( "epr-7472.5b",  0x30000, 0x8000, CRC(8e2762ec) SHA1(872e19a6aab81d7a2472367d0e31dc1295da7182) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* sound CPU */
	ROM_LOAD( "epr-7464.1b",  0x0000, 0x8000, CRC(9f291306) SHA1(96a09542a863ccf2ded43e2df6f913722b3f97b1) )

	ROM_REGION( 0x1000, "n7751", 0 )      /* 4k for 7751 onboard ROM */
	ROM_LOAD( "7751.bin",     0x0000, 0x0400, CRC(6a9534fc) SHA1(67ad94674db5c2aab75785668f610f6f4eccd158) ) /* 7751 - U34 */

	ROM_REGION( 0x20000, "n7751data", 0 ) /* 7751 sound data */
	ROM_LOAD( "epr-7473.1c",  0x00000, 0x8000, CRC(06ec75fa) SHA1(5f14bc887449122700c46ad22c0379a1682e0bdb) )
	ROM_LOAD( "epr-7475.2c",  0x08000, 0x8000, CRC(7abd1206) SHA1(54d52dc0b9c245cd2df647e714310a71b803cbcf) )
	ROM_LOAD( "epr-7474.3c",  0x10000, 0x8000, CRC(dbf853b8) SHA1(e82f497e1144f23f3233b5c45ef182bfc7923715) )
	ROM_LOAD( "epr-7476.4c",  0x18000, 0x8000, CRC(5eba655a) SHA1(6713ef12037cba3139d0f469c82bd90b44bae8ce) )

	ROM_REGION( 0x1000, "mcu", 0 )	/* Intel i8751 protection MCU */
	ROM_LOAD( "317-0010.bin", 0x00000, 0x1000, CRC(8c2033ea) SHA1(4a60d141517a5d5d065f40f71be4d2ee3be18384) )
ROM_END

/**************************************************************************************************************************
    Quartet 2, pre-System 16
    CPU: 68000
 */
ROM_START( quartet2a )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-7728.b9",  0x000000, 0x8000, CRC(56a8c88e) SHA1(33eaca5272f3588058952ca0b1fa298b89418e81) )
	ROM_LOAD16_BYTE( "epr-7725.b6",  0x000001, 0x8000, CRC(ee15fcc9) SHA1(70d9755145245537f6aeb0d39abeda7811749b8c) )
	ROM_LOAD16_BYTE( "epr-7729.b10", 0x010000, 0x8000, CRC(bc242123) SHA1(8e58dd89b70ba06d12437010a7375464647262f5) )
	ROM_LOAD16_BYTE( "epr-7726.b7",  0x010001, 0x8000, CRC(9d1c48e7) SHA1(e11a358895c7809cdf7241ff9317c2b162e4040e) )
	ROM_LOAD16_BYTE( "epr-7697.b11", 0x020000, 0x8000, CRC(3a6a375d) SHA1(8ebea6b7f1208438b47e887b46cb569725c4042a) )
	ROM_LOAD16_BYTE( "epr-7694.b8",  0x020001, 0x8000, CRC(d87b2ca2) SHA1(58adf0900e41036b1b78a931ab94b30ce601909d) )

	ROM_REGION( 0x18000, "gfx1", 0 ) /* tiles */
	ROM_LOAD( "epr-7698.c9",  0x00000, 0x08000, CRC(547a6058) SHA1(5248e974c8d12183c996b1fc8fda09e8a4bf0d2d) )
	ROM_LOAD( "epr-7699.c10", 0x08000, 0x08000, CRC(77ec901d) SHA1(b5961895473c16a8f4a111185cce48b05ab66885) )
	ROM_LOAD( "epr-7700.c11", 0x10000, 0x08000, CRC(7e348cce) SHA1(82bba65280faaf3280208c85caef48ec8baeade8) )

	ROM_REGION16_BE( 0x040000, "gfx2", 0 ) /* sprites */
	ROM_LOAD16_BYTE( "epr-7465.5c",  0x00001, 0x8000, CRC(8a1ab7d7) SHA1(a2f317538c70a1603b65d795223407cbaaf88524) )
	ROM_LOAD16_BYTE( "epr-7469.2b",  0x00000, 0x8000, CRC(cb65ae4f) SHA1(3ee7b3b4cce113a6f394e8dfd317cdb6ffae64f7) )
	ROM_LOAD16_BYTE( "epr-7466.6c",  0x10001, 0x8000, CRC(b2d3f4f3) SHA1(65e654fde10bee4cb5eee8234d0babb78fe41cfb) )
	ROM_LOAD16_BYTE( "epr-7470.3b",  0x10000, 0x8000, CRC(16fc67b1) SHA1(788fe2878c5c9faea43c2f166f32c22ee51c7d09) )
	ROM_LOAD16_BYTE( "epr-7467.7c",  0x20001, 0x8000, CRC(0af68de2) SHA1(81163baf3f0e45bac950a6d9c24b3a886db1509c) )
	ROM_LOAD16_BYTE( "epr-7471.4b",  0x20000, 0x8000, CRC(13fad5ac) SHA1(75b480083fbb14cbef969126989bf9b2235fd31e) )
	ROM_LOAD16_BYTE( "epr-7468.8c",  0x30001, 0x8000, CRC(ddfd40c0) SHA1(6c12ad668cd0c82e7d7d46bfbdcee8b9d46ebd09) )
	ROM_LOAD16_BYTE( "epr-7472.5b",  0x30000, 0x8000, CRC(8e2762ec) SHA1(872e19a6aab81d7a2472367d0e31dc1295da7182) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* sound CPU */
	ROM_LOAD( "epr-7464.1b",  0x0000, 0x8000, CRC(9f291306) SHA1(96a09542a863ccf2ded43e2df6f913722b3f97b1) )

	ROM_REGION( 0x1000, "n7751", 0 )      /* 4k for 7751 onboard ROM */
	ROM_LOAD( "7751.bin",     0x0000, 0x0400, CRC(6a9534fc) SHA1(67ad94674db5c2aab75785668f610f6f4eccd158) ) /* 7751 - U34 */

	ROM_REGION( 0x20000, "n7751data", 0 ) /* 7751 sound data */
	ROM_LOAD( "epr-7473.1c",  0x00000, 0x8000, CRC(06ec75fa) SHA1(5f14bc887449122700c46ad22c0379a1682e0bdb) )
	ROM_LOAD( "epr-7475.2c",  0x08000, 0x8000, CRC(7abd1206) SHA1(54d52dc0b9c245cd2df647e714310a71b803cbcf) )
	ROM_LOAD( "epr-7474.3c",  0x10000, 0x8000, CRC(dbf853b8) SHA1(e82f497e1144f23f3233b5c45ef182bfc7923715) )
	ROM_LOAD( "epr-7476.4c",  0x18000, 0x8000, CRC(5eba655a) SHA1(6713ef12037cba3139d0f469c82bd90b44bae8ce) )
ROM_END

/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    SDI, Sega System 16A
    CPU: FD1089B (317-0027)
 */
ROM_START( sdi )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-10970.43", 0x000000, 0x8000, CRC(b8fa4a2c) SHA1(06b448bbee0a2b2809d9af7a2a22c5847343c079) )
	ROM_LOAD16_BYTE( "epr-10968.26", 0x000001, 0x8000, CRC(a3f97793) SHA1(0f924fae0d13b3387a0e5171482f6d413432ddb3) )
	ROM_LOAD16_BYTE( "epr-10971.42", 0x010000, 0x8000, CRC(c44a0328) SHA1(3736bb83e728bb0e15ea58bc2a6c2fe66a1a4885) )
	ROM_LOAD16_BYTE( "epr-10969.25", 0x010001, 0x8000, CRC(455d15bd) SHA1(be679ecb1687b0675614ad27973c20808ad53797) )
	ROM_LOAD16_BYTE( "epr-10755.41", 0x020000, 0x8000, CRC(405e3969) SHA1(6d8c3bd06d35c971f7db005dffa2e83cae1378f8) )
	ROM_LOAD16_BYTE( "epr-10752.24", 0x020001, 0x8000, CRC(77453740) SHA1(9032463e5e14c3c610c31e2eb6e2c962df9adf46) )

	ROM_REGION( 0x30000, "gfx1", 0 ) /* tiles */
	ROM_LOAD( "epr-10756.95", 0x00000, 0x10000, CRC(44d8a506) SHA1(363d49dcb65ac0093f3ed3b259b1bc45f0291e9d) )
	ROM_LOAD( "epr-10757.94", 0x10000, 0x10000, CRC(497e1740) SHA1(95b166a9db46a27087e417c1b2cbb76bee2e64a7) )
	ROM_LOAD( "epr-10758.93", 0x20000, 0x10000, CRC(61d61486) SHA1(d48ff87216947b78903cd98a10436babdf8b75a0) )

	ROM_REGION16_BE( 0x70000, "gfx2", 0 ) /* sprites */
	ROM_LOAD16_BYTE( "epr-10760.10", 0x00001, 0x08000, CRC(30e2c50a) SHA1(1fb9e69d4cb97fdcb0f98c2a7ede246aaa4ac382) )
	ROM_CONTINUE(                    0x40001, 0x08000 )
	ROM_LOAD16_BYTE( "epr-10763.11", 0x00000, 0x08000, CRC(794e3e8b) SHA1(91ca1cb9aabf99adc8426feed4494a992afb8c4a) )
	ROM_CONTINUE(                    0x40000, 0x08000 )
	ROM_LOAD16_BYTE( "epr-10761.17", 0x10001, 0x08000, CRC(6a8b3fd0) SHA1(a122d3cb0b3263714f026e57d85b0dbf6cb110d7) )
	ROM_CONTINUE(                    0x50001, 0x08000 )
	ROM_LOAD16_BYTE( "epr-10764.18", 0x10000, 0x08000, CRC(602da5d5) SHA1(d32cdde7d86c4561e7bfa547d7d7995ce9a43c24) )
	ROM_CONTINUE(                    0x50000, 0x08000 )
	ROM_LOAD16_BYTE( "epr-10762.23", 0x20001, 0x08000, CRC(b9de3aeb) SHA1(2f7a55a8377e831338a884f8962d6ab2757e8c9b) )
	ROM_CONTINUE(                    0x60001, 0x08000 )
	ROM_LOAD16_BYTE( "epr-10765.24", 0x20000, 0x08000, CRC(0a73a057) SHA1(7f31124c67541a245e069e5b6aac59935d99a9a9) )
	ROM_CONTINUE(                    0x60000, 0x08000 )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* sound CPU */
	ROM_LOAD( "epr-10759.12", 0x0000, 0x8000, CRC(d7f9649f) SHA1(ce4abe7dd7e33da048569d7817063345fab75ea7) )

	ROM_REGION( 0x2000, "fd1089b", 0 ) /* decryption key */
	ROM_LOAD( "317-0027.key", 0x0000, 0x2000, BAD_DUMP CRC(9a5307b2) SHA1(2fcc576ed95b96ff6ea71252c3fab33b8b3fc1f5) )
ROM_END


/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Shinobi, Sega System 16A
    CPU: 68000 (unprotected)
 */
ROM_START( shinobi )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-12010.43", 0x000000, 0x10000, CRC(7df7f4a2) SHA1(86ac00a3a8ecc1a7fcb00533ea12a6cb6d59089b) )
	ROM_LOAD16_BYTE( "epr-12008.26", 0x000001, 0x10000, CRC(f5ae64cd) SHA1(33c9f25fcaff80b03d074d9d44d94976162411bf) )
	ROM_LOAD16_BYTE( "epr-12011.42", 0x020000, 0x10000, CRC(9d46e707) SHA1(37ab25b3b37365c9f45837bfb6ec80652691dd4c) ) // == epr-11283
	ROM_LOAD16_BYTE( "epr-12009.25", 0x020001, 0x10000, CRC(7961d07e) SHA1(38cbdab35f901532c0ad99ad0083513abd2ff182) ) // == epr-11281

	ROM_REGION( 0x30000, "gfx1", 0 ) /* tiles */
	ROM_LOAD( "epr-11264.95", 0x00000, 0x10000, CRC(46627e7d) SHA1(66bb5b22a2100e7b9df303007a837bc2d52cf7ba) )
	ROM_LOAD( "epr-11265.94", 0x10000, 0x10000, CRC(87d0f321) SHA1(885b38eaff2dcaeab4eeaa20cc8a2885d520abd6) )
	ROM_LOAD( "epr-11266.93", 0x20000, 0x10000, CRC(efb4af87) SHA1(0b8a905023e1bc808fd2b1c3cfa3778cde79e659) )

	ROM_REGION16_BE( 0x080000, "gfx2", 0 ) /* sprites */
	ROM_LOAD16_BYTE( "epr-11290.10", 0x00001, 0x08000, CRC(611f413a) SHA1(180f83216e2dfbfd77b0fb3be83c3042954d12df) )
	ROM_CONTINUE(                    0x40001, 0x08000 )
	ROM_LOAD16_BYTE( "epr-11294.11", 0x00000, 0x08000, CRC(5eb00fc1) SHA1(97e02eee74f61fabcad2a9e24f1868cafaac1d51) )
	ROM_CONTINUE(                    0x40000, 0x08000 )
	ROM_LOAD16_BYTE( "epr-11291.17", 0x10001, 0x08000, CRC(3c0797c0) SHA1(df18c7987281bd9379026c6cf7f96f6ae49fd7f9) )
	ROM_CONTINUE(                    0x50001, 0x08000 )
	ROM_LOAD16_BYTE( "epr-11295.18", 0x10000, 0x08000, CRC(25307ef8) SHA1(91ffbe436f80d583524ee113a8b7c0cf5d8ab286) )
	ROM_CONTINUE(                    0x50000, 0x08000 )
	ROM_LOAD16_BYTE( "epr-11292.23", 0x20001, 0x08000, CRC(c29ac34e) SHA1(b5e9b8c3233a7d6797f91531a0d9123febcf1660) )
	ROM_CONTINUE(                    0x60001, 0x08000 )
	ROM_LOAD16_BYTE( "epr-11296.24", 0x20000, 0x08000, CRC(04a437f8) SHA1(ea5fed64443236e3404fab243761e60e2e48c84c) )
	ROM_CONTINUE(                    0x60000, 0x08000 )
	ROM_LOAD16_BYTE( "epr-11293.29", 0x30001, 0x08000, CRC(41f41063) SHA1(5cc461e9738dddf9eea06831fce3702d94674163) )
	ROM_CONTINUE(                    0x70001, 0x08000 )
	ROM_LOAD16_BYTE( "epr-11297.30", 0x30000, 0x08000, CRC(b6e1fd72) SHA1(eb86e4bf880bd1a1d9bcab3f2f2e917bcaa06172) )
	ROM_CONTINUE(                    0x70000, 0x08000 )

	ROM_REGION( 0x20000, "soundcpu", 0 ) /* sound CPU */
	ROM_LOAD( "epr-11267.12", 0x0000, 0x8000, CRC(dd50b745) SHA1(52e1977569d3713ad864d607170c9a61cd059a65) )

	ROM_REGION( 0x1000, "n7751", 0 )      /* 4k for 7751 onboard ROM */
	ROM_LOAD( "7751.bin",     0x0000, 0x0400, CRC(6a9534fc) SHA1(67ad94674db5c2aab75785668f610f6f4eccd158) ) /* 7751 - U34 */

	ROM_REGION( 0x08000, "n7751data", 0 ) /* 7751 sound data */
	ROM_LOAD( "epr-11268.1",  0x0000, 0x8000, CRC(6d7966da) SHA1(90f55a99f784c21d7c135e630f4e8b1d4d043d66) )
ROM_END

/* Shinobi bootleg by 'Star' (decrypted version on original HW?, has the 7751 samples etc.) */
ROM_START( shinobls )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "b3", 0x000000, 0x10000, CRC(38e59646) SHA1(6a13015a93260ab99811b95950bb122eade01c27) )
	ROM_LOAD16_BYTE( "b1", 0x000001, 0x10000, CRC(8529d192) SHA1(202b912d20a2d82abe055b4a5e8c509ab7d69ff8) )
	ROM_LOAD16_BYTE( "b4", 0x020000, 0x10000, CRC(a2a620bd) SHA1(f8b135ce14d6c5eac5e40ddfd5ad2f1e6f2bc7a6) )
	ROM_LOAD16_BYTE( "b2", 0x020001, 0x10000, CRC(a3ceda52) SHA1(97a1c52a162fb1d43b3f8f16613b70ce582a8d26) )

	ROM_REGION( 0x30000, "gfx1", 0 ) /* tiles */
	ROM_LOAD( "b5", 0x00000, 0x10000, CRC(46627e7d) SHA1(66bb5b22a2100e7b9df303007a837bc2d52cf7ba) )
	ROM_LOAD( "b6", 0x10000, 0x10000, CRC(87d0f321) SHA1(885b38eaff2dcaeab4eeaa20cc8a2885d520abd6) )
	ROM_LOAD( "b7", 0x20000, 0x10000, CRC(efb4af87) SHA1(0b8a905023e1bc808fd2b1c3cfa3778cde79e659) )

	ROM_REGION16_BE( 0x080000, "gfx2", 0 ) /* sprites */
	ROM_LOAD16_BYTE( "b10", 0x00001, 0x08000, CRC(611f413a) SHA1(180f83216e2dfbfd77b0fb3be83c3042954d12df) )
	ROM_CONTINUE(           0x40001, 0x08000 )
	ROM_LOAD16_BYTE( "b14", 0x00000, 0x08000, CRC(5eb00fc1) SHA1(97e02eee74f61fabcad2a9e24f1868cafaac1d51) )
	ROM_CONTINUE(           0x40000, 0x08000 )
	ROM_LOAD16_BYTE( "b11", 0x10001, 0x08000, CRC(3c0797c0) SHA1(df18c7987281bd9379026c6cf7f96f6ae49fd7f9) )
	ROM_CONTINUE(           0x50001, 0x08000 )
	ROM_LOAD16_BYTE( "b15", 0x10000, 0x08000, CRC(25307ef8) SHA1(91ffbe436f80d583524ee113a8b7c0cf5d8ab286) )
	ROM_CONTINUE(           0x50000, 0x08000 )
	ROM_LOAD16_BYTE( "b12", 0x20001, 0x08000, CRC(c29ac34e) SHA1(b5e9b8c3233a7d6797f91531a0d9123febcf1660) )
	ROM_CONTINUE(           0x60001, 0x08000 )
	ROM_LOAD16_BYTE( "b16", 0x20000, 0x08000, CRC(04a437f8) SHA1(ea5fed64443236e3404fab243761e60e2e48c84c) )
	ROM_CONTINUE(           0x60000, 0x08000 )
	/* It's possible that the modifications to these roms are meant to stop the Sega logo from appearing,
       however, with the current system 16a emulation this doesn't happen, maybe it isn't actually running
       on a genuine Sega board? */
	ROM_LOAD16_BYTE( "b13", 0x30001, 0x08000, CRC(7e98bd36) SHA1(069c51478af7567e704fc9e25c9e327f02db171d) )
	ROM_CONTINUE(           0x70001, 0x08000 )
	ROM_LOAD16_BYTE( "b17", 0x30000, 0x08000, CRC(0315cf42) SHA1(2d129171aece883cb9c2805f894b3867ec98332b) )
	ROM_CONTINUE(           0x70000, 0x08000 )

	ROM_REGION( 0x20000, "soundcpu", 0 ) /* sound CPU */
	ROM_LOAD( "b8", 0x0000, 0x8000, CRC(dd50b745) SHA1(52e1977569d3713ad864d607170c9a61cd059a65) )

	ROM_REGION( 0x1000, "n7751", 0 )      /* 4k for 7751 onboard ROM */
	ROM_LOAD( "7751.bin",     0x0000, 0x0400, CRC(6a9534fc) SHA1(67ad94674db5c2aab75785668f610f6f4eccd158) ) /* 7751 - U34 */

	ROM_REGION( 0x08000, "n7751data", 0 ) /* 7751 sound data */
	ROM_LOAD( "b9", 0x0000, 0x8000, CRC(6d7966da) SHA1(90f55a99f784c21d7c135e630f4e8b1d4d043d66) )
ROM_END

/* Shinobi bootleg by 'Beta' (7751 replaced by what? Sample rom is different, but no extra sound CPU rom present, missing?) */
/* otherwise it seems to run fine on System 16A */
ROM_START( shinoblb )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "4.3k", 0x000000, 0x10000, CRC(c178a39c) SHA1(05ff1679cdfc3618df8b3fabdeab64b1f2299aa3) )
	ROM_LOAD16_BYTE( "2.3n", 0x000001, 0x10000, CRC(5ad8ebf2) SHA1(b22e0c8d4b27c553abface17c625e207d19417ab) )
	ROM_LOAD16_BYTE( "5.2k", 0x020000, 0x10000, CRC(a2a620bd) SHA1(f8b135ce14d6c5eac5e40ddfd5ad2f1e6f2bc7a6) )
	ROM_LOAD16_BYTE( "3.2n", 0x020001, 0x10000, CRC(a3ceda52) SHA1(97a1c52a162fb1d43b3f8f16613b70ce582a8d26) )

	ROM_REGION( 0x30000, "gfx1", 0 ) /* tiles */
	ROM_LOAD( "8.3b", 0x00000, 0x10000, CRC(46627e7d) SHA1(66bb5b22a2100e7b9df303007a837bc2d52cf7ba) )
	ROM_LOAD( "7.4b", 0x10000, 0x10000, CRC(87d0f321) SHA1(885b38eaff2dcaeab4eeaa20cc8a2885d520abd6) )
	ROM_LOAD( "6.5b", 0x20000, 0x10000, CRC(efb4af87) SHA1(0b8a905023e1bc808fd2b1c3cfa3778cde79e659) )

	ROM_REGION16_BE( 0x080000, "gfx2", 0 ) /* sprites */
	ROM_LOAD16_BYTE( "9.6r",  0x00001, 0x08000, CRC(611f413a) SHA1(180f83216e2dfbfd77b0fb3be83c3042954d12df) )
	ROM_CONTINUE(             0x40001, 0x08000 )
	ROM_LOAD16_BYTE( "13.8r", 0x00000, 0x08000, CRC(5eb00fc1) SHA1(97e02eee74f61fabcad2a9e24f1868cafaac1d51) )
	ROM_CONTINUE(             0x40000, 0x08000 )
	ROM_LOAD16_BYTE( "10.6q", 0x10001, 0x08000, CRC(3c0797c0) SHA1(df18c7987281bd9379026c6cf7f96f6ae49fd7f9) )
	ROM_CONTINUE(             0x50001, 0x08000 )
	ROM_LOAD16_BYTE( "14.8q", 0x10000, 0x08000, CRC(25307ef8) SHA1(91ffbe436f80d583524ee113a8b7c0cf5d8ab286) )
	ROM_CONTINUE(             0x50000, 0x08000 )
	ROM_LOAD16_BYTE( "11.6p", 0x20001, 0x08000, CRC(c29ac34e) SHA1(b5e9b8c3233a7d6797f91531a0d9123febcf1660) )
	ROM_CONTINUE(             0x60001, 0x08000 )
	ROM_LOAD16_BYTE( "15.8p", 0x20000, 0x08000, CRC(04a437f8) SHA1(ea5fed64443236e3404fab243761e60e2e48c84c) )
	ROM_CONTINUE(             0x60000, 0x08000 )
	ROM_LOAD16_BYTE( "12.6n", 0x30001, 0x08000, CRC(41f41063) SHA1(5cc461e9738dddf9eea06831fce3702d94674163) )
	ROM_CONTINUE(             0x70001, 0x08000 )
	ROM_LOAD16_BYTE( "16.8n", 0x30000, 0x08000, CRC(b6e1fd72) SHA1(eb86e4bf880bd1a1d9bcab3f2f2e917bcaa06172) )
	ROM_CONTINUE(             0x70000, 0x08000 )

	ROM_REGION( 0x20000, "soundcpu", 0 ) /* sound CPU */
	ROM_LOAD( "1.5s", 0x0000, 0x8000, CRC(dd50b745) SHA1(52e1977569d3713ad864d607170c9a61cd059a65) )

	/* these 2 n7751 roms weren't present in this set, it's possible it didn't have them */
	ROM_REGION( 0x1000, "n7751", 0 )      /* 4k for 7751 onboard ROM */
	ROM_LOAD( "7751.bin",     0x0000, 0x0400, CRC(6a9534fc) SHA1(67ad94674db5c2aab75785668f610f6f4eccd158) ) /* 7751 - U34 */

	ROM_REGION( 0x08000, "n7751data", 0 ) /* 7751 sound data */
	ROM_LOAD( "b9", 0x0000, 0x8000, CRC(6d7966da) SHA1(90f55a99f784c21d7c135e630f4e8b1d4d043d66) )

	ROM_REGION( 0x08000, "samples", 0 )
	/* sound samples (played by what?, not the same as the original) */
	/* marked as 'bad dump' pending investigation, we might actually be missing a cpu rom to play them */
	ROM_LOAD( "17.6u", 0x0000, 0x8000, BAD_DUMP CRC(b7a6890c) SHA1(6431df82c7dbe454cabc6084c1a677ebb42ae4b3) )
ROM_END

/**************************************************************************************************************************
    Shinobi, Sega System 16A
    CPU: FD1094 (317-0050)
 */
ROM_START( shinobi1 )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-11262.42", 0x000000, 0x10000, CRC(d4b8df12) SHA1(64bfa2dd8a3d99728d9eeb114887272d9590d0b8) )
	ROM_LOAD16_BYTE( "epr-11260.27", 0x000001, 0x10000, CRC(2835c95d) SHA1(b5b42af265d3a16183e02d58b053ec2894072679) )
	ROM_LOAD16_BYTE( "epr-11263.43", 0x020000, 0x10000, CRC(a2a620bd) SHA1(f8b135ce14d6c5eac5e40ddfd5ad2f1e6f2bc7a6) )
	ROM_LOAD16_BYTE( "epr-11261.25", 0x020001, 0x10000, CRC(a3ceda52) SHA1(97a1c52a162fb1d43b3f8f16613b70ce582a8d26) )

	ROM_REGION( 0x2000, "user1", 0 )	/* decryption key */
	ROM_LOAD( "317-0050.key", 0x0000, 0x2000, CRC(82c39ced) SHA1(5490237ff7f20f9ebfa3e46eedd5afd4f1c28548) )

	ROM_REGION( 0x30000, "gfx1", 0 ) /* tiles */
	ROM_LOAD( "epr-11264.95", 0x00000, 0x10000, CRC(46627e7d) SHA1(66bb5b22a2100e7b9df303007a837bc2d52cf7ba) )
	ROM_LOAD( "epr-11265.94", 0x10000, 0x10000, CRC(87d0f321) SHA1(885b38eaff2dcaeab4eeaa20cc8a2885d520abd6) )
	ROM_LOAD( "epr-11266.93", 0x20000, 0x10000, CRC(efb4af87) SHA1(0b8a905023e1bc808fd2b1c3cfa3778cde79e659) )

	ROM_REGION16_BE( 0x080000, "gfx2", 0 ) /* sprites */
	ROM_LOAD16_BYTE( "epr-11290.10", 0x00001, 0x08000, CRC(611f413a) SHA1(180f83216e2dfbfd77b0fb3be83c3042954d12df) )
	ROM_CONTINUE(                    0x40001, 0x08000 )
	ROM_LOAD16_BYTE( "epr-11294.11", 0x00000, 0x08000, CRC(5eb00fc1) SHA1(97e02eee74f61fabcad2a9e24f1868cafaac1d51) )
	ROM_CONTINUE(                    0x40000, 0x08000 )
	ROM_LOAD16_BYTE( "epr-11291.17", 0x10001, 0x08000, CRC(3c0797c0) SHA1(df18c7987281bd9379026c6cf7f96f6ae49fd7f9) )
	ROM_CONTINUE(                    0x50001, 0x08000 )
	ROM_LOAD16_BYTE( "epr-11295.18", 0x10000, 0x08000, CRC(25307ef8) SHA1(91ffbe436f80d583524ee113a8b7c0cf5d8ab286) )
	ROM_CONTINUE(                    0x50000, 0x08000 )
	ROM_LOAD16_BYTE( "epr-11292.23", 0x20001, 0x08000, CRC(c29ac34e) SHA1(b5e9b8c3233a7d6797f91531a0d9123febcf1660) )
	ROM_CONTINUE(                    0x60001, 0x08000 )
	ROM_LOAD16_BYTE( "epr-11296.24", 0x20000, 0x08000, CRC(04a437f8) SHA1(ea5fed64443236e3404fab243761e60e2e48c84c) )
	ROM_CONTINUE(                    0x60000, 0x08000 )
	ROM_LOAD16_BYTE( "epr-11293.29", 0x30001, 0x08000, CRC(41f41063) SHA1(5cc461e9738dddf9eea06831fce3702d94674163) )
	ROM_CONTINUE(                    0x70001, 0x08000 )
	ROM_LOAD16_BYTE( "epr-11297.30", 0x30000, 0x08000, CRC(b6e1fd72) SHA1(eb86e4bf880bd1a1d9bcab3f2f2e917bcaa06172) )
	ROM_CONTINUE(                    0x70000, 0x08000 )

	ROM_REGION( 0x20000, "soundcpu", 0 ) /* sound CPU */
	ROM_LOAD( "epr-11267.12", 0x0000, 0x8000, CRC(dd50b745) SHA1(52e1977569d3713ad864d607170c9a61cd059a65) )

	ROM_REGION( 0x1000, "n7751", 0 )      /* 4k for 7751 onboard ROM */
	ROM_LOAD( "7751.bin",     0x0000, 0x0400, CRC(6a9534fc) SHA1(67ad94674db5c2aab75785668f610f6f4eccd158) ) /* 7751 - U34 */

	ROM_REGION( 0x08000, "n7751data", 0 ) /* 7751 sound data */
	ROM_LOAD( "epr-11268.1",  0x0000, 0x8000, CRC(6d7966da) SHA1(90f55a99f784c21d7c135e630f4e8b1d4d043d66) )
ROM_END


/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Sukeban Jansi Ryuko, Sega System 16A
    CPU: FD1089B (317-5021, also 317-5018 known to be exist)

     (JPN Ver.)
    (c)1988 White Board

    Sega System 16A/16B

    IC61:   839-0068 (16A)
    IC69:   315-5150 (16A)

    CPU:    317-5021 (16A/16B)
*/
ROM_START( sjryuko1 )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-12251.43",  0x000000, 0x08000, CRC(1af3cd0b) SHA1(a14907bf8da8010bacaf35893037310f1bb8d375) )
	ROM_LOAD16_BYTE( "epr-12249.26",  0x000001, 0x08000, CRC(743d467d) SHA1(0eaccd3fd5c64513a86d23928a1469557c972f57) )
	ROM_LOAD16_BYTE( "epr-12252.42",  0x010000, 0x08000, CRC(7ae309d6) SHA1(399c2a4d8b64df03e02b95cc635ee041254b7683) )
	ROM_LOAD16_BYTE( "epr-12250.25",  0x010001, 0x08000, CRC(52c40f19) SHA1(0606943248b2433b70a7e4ad3408d4d3957756c9) )

	ROM_REGION( 0x18000, "gfx1", 0 ) /* tiles */
	ROM_LOAD( "12224-95.b9",  0x00000, 0x08000, CRC(eac17ba1) SHA1(6dfea3383b7c9c47bc0943a8d86fc89efcb85ae2) )
	ROM_LOAD( "12225-94.b10", 0x08000, 0x08000, CRC(2310fc98) SHA1(c40ca62edbe5cfa2f84811426233412cd5bd398c) )
	ROM_LOAD( "12226-93.b11", 0x10000, 0x08000, CRC(210e6999) SHA1(5707cc613060b0070a822850b9afab8293f64dd7) )
	/*(epr- xxxxx - S16a location . S16b location */

	ROM_REGION16_BE( 0x80000, "gfx2", ROMREGION_ERASE00 ) /* sprites */
	ROM_LOAD16_BYTE( "12232-10.b1", 0x00001, 0x08000, CRC(0adec62b) SHA1(cd798a7994cea73bffe78feac4e692d755074b1d) )
	ROM_CONTINUE(                   0x40001, 0x08000 )
	ROM_LOAD16_BYTE( "12236-11.b5", 0x00000, 0x08000, CRC(286b9af8) SHA1(085251b8ce8b7fadf15b8ebd5872f0337adf142b) )
	ROM_CONTINUE(                   0x40000, 0x08000 )
	ROM_LOAD16_BYTE( "12233-17.b2", 0x10001, 0x08000, CRC(3e45969c) SHA1(804f3714c97877c6f0caf458f8af38e8d8179d73) )
	ROM_CONTINUE(                   0x50001, 0x08000 )
	ROM_LOAD16_BYTE( "12237-18.b6", 0x10000, 0x08000, CRC(e5058e96) SHA1(4a1f663c7c87fe7177a52017da3f2f55568bd863) )
	ROM_CONTINUE(                   0x50000, 0x08000 )
	ROM_LOAD16_BYTE( "12234-23.b3", 0x20001, 0x08000, CRC(8c8d54ef) SHA1(a8adee4f6ad8079af88cf471af42ace8ac8d093e) )
	ROM_CONTINUE(                   0x60001, 0x08000 )
	ROM_LOAD16_BYTE( "12238-24.b7", 0x20000, 0x08000, CRC(7ada3304) SHA1(e402442e73d93a1b174e3fcab6a97fb2d450994c) )
	ROM_CONTINUE(                   0x60000, 0x08000 )
	ROM_LOAD16_BYTE( "12235-29.b4", 0x30001, 0x08000, CRC(fa45d511) SHA1(41e343b039e8633b2469a5eaf5e4196b682f0d01) )
	ROM_CONTINUE(                   0x70001, 0x08000 )
	ROM_LOAD16_BYTE( "12239-30.b8", 0x30000, 0x08000, CRC(91f70c8b) SHA1(c3ac9cf248540d948f7845eb17ec95e1be8d00bb) )
	ROM_CONTINUE(                   0x70000, 0x08000 )
	/*(epr- xxxxx - S16a location . S16b location */

	ROM_REGION( 0x20000, "soundcpu", 0 ) /* sound CPU */
	ROM_LOAD( "epr-12227.12", 0x0000, 0x8000, CRC(5b12409d) SHA1(b25d6fa004461426f6358ab70fd071239c78e949) )

	ROM_REGION( 0x1000, "n7751", 0 )      /* 4k for 7751 onboard ROM */
	ROM_LOAD( "7751.bin",     0x0000, 0x0400, CRC(6a9534fc) SHA1(67ad94674db5c2aab75785668f610f6f4eccd158) ) /* 7751 - U34 */

	ROM_REGION( 0x20000, "n7751data", 0 ) /* 7751 sound data */
	ROM_LOAD( "epr-12228.1", 0x00000, 0x8000, CRC(6b2e6aef) SHA1(64ae6ec327c32cdb877a493ebfe11af15e2388ac) )
	ROM_LOAD( "epr-12229.2", 0x08000, 0x8000, CRC(b7aa015c) SHA1(0ef023f73722e27180c271b207a5097220f40b5e) )
	ROM_LOAD( "epr-12230.4", 0x10000, 0x8000, CRC(d0f61fd4) SHA1(e6f29459d7395122f26957f56e38926aebd9004c) )
	ROM_LOAD( "epr-12231.5", 0x18000, 0x8000, CRC(780bdc57) SHA1(8c859043bba389292604385b88c743728180f9a9) )

	ROM_REGION( 0x2000, "fd1089b", 0 ) /* decryption key */
	ROM_LOAD( "317-5021.key", 0x0000, 0x2000, BAD_DUMP CRC(4a3422e4) SHA1(69a32a6987ff2481f6d6cbbe399269a2461b8bad) )
ROM_END


/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Tetris, Sega System 16A
    CPU: FD1094 (317-0093)

    Top board

    Pos.   Label       Part        Notes

    D4     EPR-12205   27C256      Z80 program
    D8     EPR-12200   27C256      68000 program
    C8     Unused                  68000 program
    B8     Unused                  68000 program
    D11    EPR-12201   27C256      68000 program
    C11    Unused                  68000 program
    B11    Unused                  68000 program
    C18    EPR-12204   27C512      Tile data
    D18    EPR-12203   27C512      Tile data
    E18    EPR-12202   27C512      Tile data

    Bottom board

    Pos.   Label       Part        Notes

    D3     EPR-12169   27C256      Sprite data
    D4     Unused                  Sprite data
    D5     Unused                  Sprite data
    D6     Unused                  Sprite data
    F3     EPR-12170   27C256      Sprite data
    F4     Unused                  Sprite data
    F5     Unused                  Sprite data
    F6     Unused                  Sprite data
 */
ROM_START( tetris )
	ROM_REGION( 0x40000, "maincpu", ROMREGION_ERASEFF ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-12201.rom", 0x000000, 0x8000, CRC(338e9b51) SHA1(f56a1124c963d4ad72a806b26f9aa906aaa37d2b) )
	ROM_LOAD16_BYTE( "epr-12200.rom", 0x000001, 0x8000, CRC(fb058779) SHA1(0045985ea943ebc7e44bd95127c5e5212c2821e8) )

	ROM_REGION( 0x2000, "user1", 0 )	/* decryption key */
	ROM_LOAD( "317-0093.key",  0x0000, 0x2000, CRC(e0064442) SHA1(cc70b1a2c66729c4540dabd6a24a5f5615beedcd) )

	ROM_REGION( 0x30000, "gfx1", 0 ) /* tiles */
	ROM_LOAD( "epr-12202.rom", 0x00000, 0x10000, CRC(2f7da741) SHA1(51a685673b4a57a13818eca65d122230f20bd9a0) )
	ROM_LOAD( "epr-12203.rom", 0x10000, 0x10000, CRC(a6e58ec5) SHA1(5a6c43c989768270e0ab61cfaa5ef86d4607fe20) )
	ROM_LOAD( "epr-12204.rom", 0x20000, 0x10000, CRC(0ae98e23) SHA1(f067b81b85f9e03a6373c7c53ff52d5395b8a985) )

	ROM_REGION16_BE( 0x10000, "gfx2", 0 ) /* sprites */
	ROM_LOAD16_BYTE( "epr-12169.b1",  0x00001, 0x8000, CRC(dacc6165) SHA1(87b1a7643e3630ff73b2b117752496e1ea5da23d) )
	ROM_LOAD16_BYTE( "epr-12170.b5",  0x00000, 0x8000, CRC(87354e42) SHA1(e7fd55aee59b51d82cb9b619fbb815ad6839560c) )

	ROM_REGION( 0x40000, "soundcpu", 0 ) /* sound CPU */
	ROM_LOAD( "epr-12205.rom", 0x0000, 0x8000, CRC(6695dc99) SHA1(08123aa24c302bc9243329384bd9c2545a4d50c3) )
ROM_END

/**************************************************************************************************************************
    Tetris, Sega System 16A
    CPU: FD1094 (317-0093a)
*/
ROM_START( tetris3 )
	ROM_REGION( 0x40000, "maincpu", ROMREGION_ERASEFF ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-12201a.43", 0x000000, 0x8000, CRC(9250e5cf) SHA1(e848a8279ce35f516754eec33b3b443d2e819eaa) )
	ROM_LOAD16_BYTE( "epr-12200a.26", 0x000001, 0x8000, CRC(85d4b0ff) SHA1(f9d8e1ebb0c02a6c3c0b0acc78a6bea081ffc6f7) )

	ROM_REGION( 0x2000, "user1", 0 ) /* decryption key */
	ROM_LOAD( "317-0093a.key", 0x0000, 0x2000, CRC(7ca4a8ee) SHA1(c85763b7c5d606ee72181d9baba7de5e2c457fd8) )

	ROM_REGION( 0x30000, "gfx1", 0 ) /* tiles */
	ROM_LOAD( "epr-12202.rom", 0x00000, 0x10000, CRC(2f7da741) SHA1(51a685673b4a57a13818eca65d122230f20bd9a0) )
	ROM_LOAD( "epr-12203.rom", 0x10000, 0x10000, CRC(a6e58ec5) SHA1(5a6c43c989768270e0ab61cfaa5ef86d4607fe20) )
	ROM_LOAD( "epr-12204.rom", 0x20000, 0x10000, CRC(0ae98e23) SHA1(f067b81b85f9e03a6373c7c53ff52d5395b8a985) )

	ROM_REGION16_BE( 0x10000, "gfx2", 0 ) /* sprites */
	ROM_LOAD16_BYTE( "epr-12169.b1",  0x00001, 0x8000, CRC(dacc6165) SHA1(87b1a7643e3630ff73b2b117752496e1ea5da23d) )
	ROM_LOAD16_BYTE( "epr-12170.b5",  0x00000, 0x8000, CRC(87354e42) SHA1(e7fd55aee59b51d82cb9b619fbb815ad6839560c) )

	ROM_REGION( 0x40000, "soundcpu", 0 ) /* sound CPU */
	ROM_LOAD( "epr-12205.rom", 0x0000, 0x8000, CRC(6695dc99) SHA1(08123aa24c302bc9243329384bd9c2545a4d50c3) )
ROM_END


/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Time Scanner, Sega System 16A
    CPU: FD1089B (317-0024)

    GAME NUMBER: TOP 837-5941-01, BOTTOM 837-5942-01
    CPU: FD1089B 6J2 317-0024

    BOARD: SYSTEM 16B

    GAME NUMBER: ???-????-??
    CPU: MC68000

    IC POSITIONS    EPROMS      EPROMS
    S16A    S16B    NUMBERS     FUNCTIONS

    26  -   EPR10537A   PROGRAM 317-0024
    25  -   EPR10538    "
    24  -   EPR10539    "
    43  -   EPR10540A   "
    42  -   EPR10541    "
    41  -   EPR10542    "

    95  B9  EPR10543    SCREEN
    94  B10 EPR10544    "
    93  B11 EPR10545    "

    12  -   EPR10546    SOUND PROGRAM
    1   -   EPR10547    SPEECH

    10  B1  EPR10548    OBJECT
    17  B2  EPR10549    "
    23  B3  EPR10550    "
    29  B4  EPR10551    "
    11  B5  EPR10552    "
    18  B6  EPR10553    "
    24  B7  EPR10554    "
    30  B8  EPR10555    "

    -   A7  EPR10562    SOUND PROGRAM
    -   A8  EPR10563    SPEECH

    -   A1  EPR10850    PROGRAM MC68000
    -   A2  EPR10851    "
    -   A3  EPR10852    "
    -   A4  EPR10853    "
    -   A5  EPR10854    "
    -   A6  EPR10855    "
*/
ROM_START( timescan1 )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-10540a.43", 0x00000, 0x8000, CRC(76848b0b) SHA1(a7755898c2b3212d80034c47760440db6dcde83c) )
	ROM_LOAD16_BYTE( "epr-10537a.26", 0x00001, 0x8000, CRC(4ddc434a) SHA1(54908654f1445f2d3a3b1496015f3347ad603225) )
	ROM_LOAD16_BYTE( "epr-10541.42",  0x10000, 0x8000, CRC(cc6d945e) SHA1(0ace2a8fddc27da4c8c3efb16f245f6325f02ed5) )
	ROM_LOAD16_BYTE( "epr-10538.25",  0x10001, 0x8000, CRC(68379473) SHA1(7f6e0b3fb29ef5dd1023625ef7a7270fc230d40f) )
	ROM_LOAD16_BYTE( "epr-10542.41",  0x20000, 0x8000, CRC(10217dfa) SHA1(845ea0483dca0aae042da52fbd7bc07e7e2f026d) )
	ROM_LOAD16_BYTE( "epr-10539.24",  0x20001, 0x8000, CRC(10943b2e) SHA1(a297ed455062a3d39b9eecfe2b92474d47ce758f) )

	ROM_REGION( 0x18000, "gfx1", 0 ) /* tiles */
	ROM_LOAD( "epr-10543.95", 0x00000, 0x8000, CRC(07dccc37) SHA1(544cc6a3b3ef64727ecf5098b84ade2dd5330614) )
	ROM_LOAD( "epr-10544.94", 0x08000, 0x8000, CRC(84fb9a3a) SHA1(efde54cc9582f68e58cae05f717a4fc8f620c0fc) )
	ROM_LOAD( "epr-10545.93", 0x10000, 0x8000, CRC(c8694bc0) SHA1(e48fc349ef454ded86141937f70b006e64da6b6b) )

	ROM_REGION16_BE( 0x40000, "gfx2", 0 ) /* sprites */
	ROM_LOAD16_BYTE( "epr-10548.10",  0x00001, 0x8000, CRC(aa150735) SHA1(b6e6ff9229c641e196fc7a0a2cf7aa362f554676) )
	ROM_LOAD16_BYTE( "epr-10552.11",  0x00000, 0x8000, CRC(6fcbb9f7) SHA1(0a0fab930477d8b79e500263bbc80d3bf73778f8) )
	ROM_LOAD16_BYTE( "epr-10549.17",  0x10001, 0x8000, CRC(2f59f067) SHA1(1fb64cce2f98ddcb5ecb662e63ea636a8da08bcd) )
	ROM_LOAD16_BYTE( "epr-10553.18",  0x10000, 0x8000, CRC(8a220a9f) SHA1(c17547d85721fa19e5f445b5be30b3fbf5e8cc6e) )
	ROM_LOAD16_BYTE( "epr-10550.23",  0x20001, 0x8000, CRC(f05069ff) SHA1(bd95761036c2fad8ddf4e169d899b173822ee4b0) )
	ROM_LOAD16_BYTE( "epr-10554.24",  0x20000, 0x8000, CRC(dc64f809) SHA1(ea85eefa98ec55e9e872940821a959ff4eb1bd1c) )
	ROM_LOAD16_BYTE( "epr-10551.29",  0x30001, 0x8000, CRC(435d811f) SHA1(b28eb09620113cd7578387c4d96029f2acb8ec06) )
	ROM_LOAD16_BYTE( "epr-10555.30",  0x30000, 0x8000, CRC(2143c471) SHA1(d413aa216349ddf773a39d2826c3a940b4149229) )

	ROM_REGION( 0x20000, "soundcpu", 0 ) /* sound CPU */
	ROM_LOAD( "epr-10546.12", 0x0000, 0x8000, CRC(1ebee5cc) SHA1(5e24ee25e770068a1292e657307cf53f6a8ae1c9) )

	ROM_REGION( 0x1000, "n7751", 0 )      /* 4k for 7751 onboard ROM */
	ROM_LOAD( "7751.bin",     0x0000, 0x0400, CRC(6a9534fc) SHA1(67ad94674db5c2aab75785668f610f6f4eccd158) ) /* 7751 - U34 */

	ROM_REGION( 0x08000, "n7751data", 0 ) /* 7751 sound data */
	ROM_LOAD( "epr-10547.1",  0x0000, 0x8000, CRC(d24ffc4b) SHA1(3b250e1f026664f7a37f65d1c1a07381e88f11e8) )

	ROM_REGION( 0x2000, "fd1089b", 0 ) /* decryption key */
	ROM_LOAD( "317-0024.key", 0x0000, 0x2000, BAD_DUMP CRC(ee42ec18) SHA1(cb65dd681f38ce20440ddcb01a935c2c8eecc77f) )
ROM_END


/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Wonder Boy III, Sega System 16A
    CPU: FD1094 (317-0084)
 */
ROM_START( wb31 )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-12084.bin", 0x000000, 0x10000, CRC(b6deb654) SHA1(37066cc63902233bb8b56d3171c42bf8a8f82e58) )
	ROM_LOAD16_BYTE( "epr-12082.bin", 0x000001, 0x10000, CRC(38dc5b15) SHA1(b25bf60d269a87f9d8dbc1a3787c8ff9a6e7482c) )
	ROM_LOAD16_BYTE( "epr-12085.bin", 0x020000, 0x10000, CRC(0962098b) SHA1(150fc439dd5e773bef706f058abdb4d2ec44e355) )
	ROM_LOAD16_BYTE( "epr-12083.bin", 0x020001, 0x10000, CRC(3d631a8e) SHA1(4940ff6cf380fb914876ade39ea37f42b79bf11d) )

	ROM_REGION( 0x2000, "user1", 0 )	/* decryption key */
	ROM_LOAD( "317-0084.key",  0x0000, 0x2000, CRC(2c58dafa) SHA1(24d06970eda896fdd5e3486132bd19834f7d3659) )

	ROM_REGION( 0x30000, "gfx1", 0 ) /* tiles */
	ROM_LOAD( "epr-12086.bin", 0x00000, 0x10000, CRC(45b949df) SHA1(84390d16da00b775988e5f6c20950cb2304b1a74) )
	ROM_LOAD( "epr-12087.bin", 0x10000, 0x10000, CRC(6f0396b7) SHA1(0a340f2b58e5ecfe504197a8fd2111181e868a3e) )
	ROM_LOAD( "epr-12088.bin", 0x20000, 0x10000, CRC(ba8c0749) SHA1(7d996c7a1ad249c06ef7ec9c87a83710c98005d3) )

	ROM_REGION16_BE( 0x80000, "gfx2", 0 ) /* sprites */
	ROM_LOAD16_BYTE( "epr-12090.b1", 0x00001, 0x008000, CRC(aeeecfca) SHA1(496124b170a725ad863c741d4e021ab947511e4c) )
	ROM_CONTINUE(                    0x40001, 0x008000 )
	ROM_LOAD16_BYTE( "epr-12094.b5", 0x00000, 0x008000, CRC(615e4927) SHA1(d23f164973afa770714e284a77ddf10f18cc596b) )
	ROM_CONTINUE(                    0x40000, 0x008000 )
	ROM_LOAD16_BYTE( "epr-12091.b2", 0x10001, 0x008000, CRC(8409a243) SHA1(bcbb9510a6499d8147543d6befa5a49f4ac055d9) )
	ROM_CONTINUE(                    0x50001, 0x008000 )
	ROM_LOAD16_BYTE( "epr-12095.b6", 0x10000, 0x008000, CRC(e774ec2c) SHA1(a4aa15ec7be5539a740ad02ff720458018dbc536) )
	ROM_CONTINUE(                    0x50000, 0x008000 )
	ROM_LOAD16_BYTE( "epr-12092.b3", 0x20001, 0x008000, CRC(5c2f0d90) SHA1(e0fbc0f841e4607ad232931368b16e81440a75c4) )
	ROM_CONTINUE(                    0x60001, 0x008000 )
	ROM_LOAD16_BYTE( "epr-12096.b7", 0x20000, 0x008000, CRC(0cd59d6e) SHA1(caf754a461feffafcfe7bfc6e89da76c4db257c5) )
	ROM_CONTINUE(                    0x60000, 0x008000 )
	ROM_LOAD16_BYTE( "epr-12093.b4", 0x30001, 0x008000, CRC(4891e7bb) SHA1(1be04fcabe9bfa8cf746263a5bcca67902a021a0) )
	ROM_CONTINUE(                    0x70001, 0x008000 )
	ROM_LOAD16_BYTE( "epr-12097.b8", 0x30000, 0x008000, CRC(e645902c) SHA1(497cfcf6c25cc2e042e16dbcb1963d2223def15a) )
	ROM_CONTINUE(                    0x70000, 0x008000 )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* sound CPU */
	ROM_LOAD( "epr-12089.bin", 0x0000, 0x8000, CRC(8321eb0b) SHA1(61cf95833c0aa38e35fc18db39d4ec74e4aaf01e) )
ROM_END

/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Wonder Boy III, Sega System 16A
    CPU: FD1089A (317-0086)
 */
ROM_START( wb35 )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-12120.43", 0x000000, 0x10000, CRC(cf41adf1) SHA1(aab2146d990d6698d73edf8d8699e98a1f01d534) )
	ROM_LOAD16_BYTE( "epr-12118.26", 0x000001, 0x10000, CRC(050ad6d0) SHA1(4d5e5b9a6b9b33ec767a18c30a8252063341eefb) )
	ROM_LOAD16_BYTE( "epr-12121.42", 0x020000, 0x10000, CRC(5e44c0a9) SHA1(4d4c9575a6cd571a7cf635164e92afd92d4b63fe) )
	ROM_LOAD16_BYTE( "epr-12119.25", 0x020001, 0x10000, CRC(b67b97a3) SHA1(0a5ab21c7b4ca6d6a31852dd8dfb94bb55c0f15d) )

	ROM_REGION( 0x30000, "gfx1", 0 ) /* tiles */
	ROM_LOAD( "epr-12086.95", 0x00000, 0x10000, CRC(45b949df) SHA1(84390d16da00b775988e5f6c20950cb2304b1a74) )
	ROM_LOAD( "epr-12087.94", 0x10000, 0x10000, CRC(5fb761aa) SHA1(dcf88e68732a8ec122d0603d87f6ea1f1614adef) )
	ROM_LOAD( "epr-12088.83", 0x20000, 0x10000, CRC(00579c39) SHA1(12acdea75e3d040d60a9f32d05fd9e0191d38f21) )

	ROM_REGION16_BE( 0x80000, "gfx2", 0 ) /* sprites */
	ROM_LOAD16_BYTE( "epr-12090.10", 0x00001, 0x008000, CRC(c85904c1) SHA1(e5af26ce870813bd5a88b430fc3881c7fcb63aef) )
	ROM_CONTINUE(                    0x40001, 0x008000 )
	ROM_LOAD16_BYTE( "epr-12094.11", 0x00000, 0x008000, CRC(615e4927) SHA1(d23f164973afa770714e284a77ddf10f18cc596b) )
	ROM_CONTINUE(                    0x40000, 0x008000 )
	ROM_LOAD16_BYTE( "epr-12091.17", 0x10001, 0x008000, CRC(8409a243) SHA1(bcbb9510a6499d8147543d6befa5a49f4ac055d9) )
	ROM_CONTINUE(                    0x50001, 0x008000 )
	ROM_LOAD16_BYTE( "epr-12095.18", 0x10000, 0x008000, CRC(e774ec2c) SHA1(a4aa15ec7be5539a740ad02ff720458018dbc536) )
	ROM_CONTINUE(                    0x50000, 0x008000 )
	ROM_LOAD16_BYTE( "epr-12092.23", 0x20001, 0x008000, CRC(5c2f0d90) SHA1(e0fbc0f841e4607ad232931368b16e81440a75c4) )
	ROM_CONTINUE(                    0x60001, 0x008000 )
	ROM_LOAD16_BYTE( "epr-12096.24", 0x20000, 0x008000, CRC(d55ca9c9) SHA1(cff95bd799cf9625f95261620d2df7f0891e34da) )
	ROM_CONTINUE(                    0x60000, 0x008000 )
	ROM_LOAD16_BYTE( "epr-12093.29", 0x30001, 0x008000, CRC(9e6d9086) SHA1(1cfe7175f538db9a47ef576b33be49057db0927d) )
	ROM_CONTINUE(                    0x70001, 0x008000 )
	ROM_LOAD16_BYTE( "epr-12097.30", 0x30000, 0x008000, CRC(e645902c) SHA1(497cfcf6c25cc2e042e16dbcb1963d2223def15a) )
	ROM_CONTINUE(                    0x70000, 0x008000 )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* sound CPU */
	ROM_LOAD( "epr-12089.12", 0x0000, 0x8000, CRC(8321eb0b) SHA1(61cf95833c0aa38e35fc18db39d4ec74e4aaf01e) )

	ROM_REGION( 0x2000, "fd1089a", 0 ) /* decryption key */
	ROM_LOAD( "317-wb35.key", 0x0000, 0x2000, BAD_DUMP CRC(8a2e0575) SHA1(e43a2c8ca102ec38871067685a860da53d748765) )
ROM_END

ROM_START( wb35a )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-12120.43_alt", 0x000000, 0x10000, CRC(cbd8c99b) SHA1(2401aab80293bbe3b603a0b6af33ebc8834915e9) )
	ROM_LOAD16_BYTE( "epr-12118.26_alt", 0x000001, 0x10000, CRC(e9a3280c) SHA1(b7725171286915f8e1039d8063ba9e9f6ab40b56) )
	ROM_LOAD16_BYTE( "epr-12121.42",     0x020000, 0x10000, CRC(5e44c0a9) SHA1(4d4c9575a6cd571a7cf635164e92afd92d4b63fe) )
	ROM_LOAD16_BYTE( "epr-12119.25_alt", 0x020001, 0x10000, CRC(01ed3ef9) SHA1(29491a81a735198393f1cc2f06940cf40168ff59) )

	ROM_REGION( 0x30000, "gfx1", 0 ) /* tiles */
	ROM_LOAD( "epr-12086.95", 0x00000, 0x10000, CRC(45b949df) SHA1(84390d16da00b775988e5f6c20950cb2304b1a74) )
	ROM_LOAD( "epr-12087.94", 0x10000, 0x10000, CRC(5fb761aa) SHA1(dcf88e68732a8ec122d0603d87f6ea1f1614adef) )
	ROM_LOAD( "epr-12088.83", 0x20000, 0x10000, CRC(00579c39) SHA1(12acdea75e3d040d60a9f32d05fd9e0191d38f21) )

	ROM_REGION16_BE( 0x80000, "gfx2", 0 ) /* sprites */
	ROM_LOAD16_BYTE( "epr-12090.b1", 0x00001, 0x008000, CRC(aeeecfca) SHA1(496124b170a725ad863c741d4e021ab947511e4c) )
	ROM_CONTINUE(                    0x40001, 0x008000 )
	ROM_LOAD16_BYTE( "epr-12094.b5", 0x00000, 0x008000, CRC(615e4927) SHA1(d23f164973afa770714e284a77ddf10f18cc596b) )
	ROM_CONTINUE(                    0x40000, 0x008000 )
	ROM_LOAD16_BYTE( "epr-12091.b2", 0x10001, 0x008000, CRC(8409a243) SHA1(bcbb9510a6499d8147543d6befa5a49f4ac055d9) )
	ROM_CONTINUE(                    0x50001, 0x008000 )
	ROM_LOAD16_BYTE( "epr-12095.b6", 0x10000, 0x008000, CRC(e774ec2c) SHA1(a4aa15ec7be5539a740ad02ff720458018dbc536) )
	ROM_CONTINUE(                    0x50000, 0x008000 )
	ROM_LOAD16_BYTE( "epr-12092.b3", 0x20001, 0x008000, CRC(5c2f0d90) SHA1(e0fbc0f841e4607ad232931368b16e81440a75c4) )
	ROM_CONTINUE(                    0x60001, 0x008000 )
	ROM_LOAD16_BYTE( "epr-12096.b7", 0x20000, 0x008000, CRC(0cd59d6e) SHA1(caf754a461feffafcfe7bfc6e89da76c4db257c5) )
	ROM_CONTINUE(                    0x60000, 0x008000 )
	ROM_LOAD16_BYTE( "epr-12093.b4", 0x30001, 0x008000, CRC(4891e7bb) SHA1(1be04fcabe9bfa8cf746263a5bcca67902a021a0) )
	ROM_CONTINUE(                    0x70001, 0x008000 )
	ROM_LOAD16_BYTE( "epr-12097.b8", 0x30000, 0x008000, CRC(e645902c) SHA1(497cfcf6c25cc2e042e16dbcb1963d2223def15a) )
	ROM_CONTINUE(                    0x70000, 0x008000 )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* sound CPU */
	ROM_LOAD( "epr-12089.12", 0x0000, 0x8000, CRC(8321eb0b) SHA1(61cf95833c0aa38e35fc18db39d4ec74e4aaf01e) )

	ROM_REGION( 0x2000, "fd1089a", 0 ) /* decryption key */
	ROM_LOAD( "317-wb35.key", 0x0000, 0x2000, BAD_DUMP CRC(8a2e0575) SHA1(e43a2c8ca102ec38871067685a860da53d748765) )
ROM_END


/*************************************
 *
 *  Generic driver initialization
 *
 *************************************/

static DRIVER_INIT( generic_16a )
{
	system16a_generic_init(machine);
}


static DRIVER_INIT( aceattaa )
{
	segas1x_state *state = (segas1x_state *)machine->driver_data;

	system16a_generic_init(machine);
	state->custom_io_r = aceattaa_custom_io_r;
}


static DRIVER_INIT( fd1089a_16a )
{
	system16a_generic_init(machine);
	fd1089a_decrypt(machine);
}


static DRIVER_INIT( fd1089b_16a )
{
	system16a_generic_init(machine);
	fd1089b_decrypt(machine);
}


static DRIVER_INIT( dumpmtmt )
{
	segas1x_state *state = (segas1x_state *)machine->driver_data;

	system16a_generic_init(machine);
	state->i8751_vblank_hook = dumpmtmt_i8751_sim;
}


static DRIVER_INIT( mjleague )
{
	segas1x_state *state = (segas1x_state *)machine->driver_data;

	system16a_generic_init(machine);
	state->custom_io_r = mjleague_custom_io_r;
}

static DRIVER_INIT( passsht16a )
{
	segas1x_state *state = (segas1x_state *)machine->driver_data;

	system16a_generic_init(machine);
	state->custom_io_r = passsht16a_custom_io_r;
}

static DRIVER_INIT( quartet )
{
	segas1x_state *state = (segas1x_state *)machine->driver_data;

	system16a_generic_init(machine);
	state->i8751_vblank_hook = quartet_i8751_sim;
}


static DRIVER_INIT( fantzonep )
{
	system16a_generic_init(machine);

	sega_315_5177_decode(machine, "soundcpu");
}


static DRIVER_INIT( sdi )
{
	segas1x_state *state = (segas1x_state *)machine->driver_data;

	system16a_generic_init(machine);
	fd1089b_decrypt(machine);
	state->custom_io_r = sdi_custom_io_r;
}


static DRIVER_INIT( sjryukoa )
{
	segas1x_state *state = (segas1x_state *)machine->driver_data;

	system16a_generic_init(machine);
	fd1089b_decrypt(machine);
	state->custom_io_r = sjryuko_custom_io_r;
	state->lamp_changed_w = sjryuko_lamp_changed_w;
}



/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

/* "Pre-System 16" */
GAME( 1986, bodyslam,   0,        system16a_8751,   bodyslam,   generic_16a, ROT0,   "Sega",          "Body Slam (8751 317-0015)", GAME_SUPPORTS_SAVE )
GAME( 1986, dumpmtmt,   bodyslam, system16a_8751,   bodyslam,   dumpmtmt,    ROT0,   "Sega",          "Dump Matsumoto (Japan, 8751 317-0011a)", GAME_UNEMULATED_PROTECTION | GAME_SUPPORTS_SAVE )
GAME( 1985, mjleague,   0,        system16a,        mjleague,   mjleague,    ROT270, "Sega",          "Major League", GAME_SUPPORTS_SAVE )
GAME( 1986, quartet,    0,        system16a_8751,   quartet,    quartet,     ROT0,   "Sega",          "Quartet (Rev A, 8751 315-5194)", GAME_UNEMULATED_PROTECTION | GAME_SUPPORTS_SAVE )
GAME( 1986, quarteta,   quartet,  system16a_8751,   quartet,    quartet,     ROT0,   "Sega",          "Quartet (8751 315-5194)", GAME_UNEMULATED_PROTECTION | GAME_SUPPORTS_SAVE )
GAME( 1986, quartet2,   quartet,  system16a_8751,   quart2,     generic_16a, ROT0,   "Sega",          "Quartet 2 (8751 317-0010)", GAME_SUPPORTS_SAVE )
GAME( 1986, quartet2a,  quartet,  system16a,        quart2,     generic_16a, ROT0,   "Sega",          "Quartet 2 (unprotected)", GAME_SUPPORTS_SAVE )

/* System 16A */
GAME( 1987, aliensyn5,  aliensyn, system16a,        aliensyn,   fd1089b_16a, ROT0,   "Sega",           "Alien Syndrome (set 5, System 16A, FD1089B 317-0037)", GAME_SUPPORTS_SAVE )
GAME( 1987, aliensyn2,  aliensyn, system16a,        aliensyn,   fd1089a_16a, ROT0,   "Sega",           "Alien Syndrome (set 2, System 16A, FD1089A 317-0033)", GAME_SUPPORTS_SAVE )
GAME( 1987, aliensynjo, aliensyn, system16a,        aliensynj,  fd1089a_16a, ROT0,   "Sega",           "Alien Syndrome (set 1, Japan, old, System 16A, FD1089A 317-0033)", GAME_SUPPORTS_SAVE )
GAME( 1988, aceattaca,  aceattac, system16a       , aceattaa,   aceattaa,    ROT270, "Sega",           "Ace Attacker (Japan, System 16A, FD1094 317-0060)", GAME_SUPPORTS_SAVE )
GAME( 1986, afighter,   0,        system16a_no7751, afighter,   fd1089a_16a, ROT270, "Sega",           "Action Fighter (FD1089A 317-0018)", GAME_SUPPORTS_SAVE )
GAME( 1986, alexkidd,   0,        system16a,        alexkidd,   generic_16a, ROT0,   "Sega",           "Alex Kidd: The Lost Stars (set 2, unprotected)", GAME_SUPPORTS_SAVE )
GAME( 1986, alexkidd1,  alexkidd, system16a,        alexkidd,   fd1089a_16a, ROT0,   "Sega",           "Alex Kidd: The Lost Stars (set 1, FD1089A 317-0021)", GAME_SUPPORTS_SAVE )
GAME( 1986, fantzone,   0,        system16a_no7751, fantzone,   generic_16a, ROT0,   "Sega",           "Fantasy Zone (Rev A, unprotected)", GAME_SUPPORTS_SAVE )
GAME( 1986, fantzone1,  fantzone, system16a_no7751, fantzone,   generic_16a, ROT0,   "Sega",           "Fantasy Zone (unprotected)", GAME_SUPPORTS_SAVE )
GAME( 1986, fantzonep,  fantzone, system16a_no7751, fantzone,   fantzonep,   ROT0,   "Sega",           "Fantasy Zone (317-5000)", GAME_SUPPORTS_SAVE )
GAME( 1988, passsht16a, passsht,  system16a,        passsht16a, passsht16a,  ROT270, "Sega",           "Passing Shot (Japan, 4 Players, System 16A, FD1094 317-0071)", GAME_SUPPORTS_SAVE )
GAME( 1987, sdi,        0,        system16a_no7751, sdi,        sdi,         ROT0,   "Sega",           "SDI - Strategic Defense Initiative (Japan, old, System 16A, FD1089B 317-0027)", GAME_SUPPORTS_SAVE )
GAME( 1987, shinobi,    0,        system16a,        shinobi,    generic_16a, ROT0,   "Sega",           "Shinobi (set 6, System 16A, unprotected)", GAME_SUPPORTS_SAVE )
GAME( 1987, shinobi1,   shinobi,  system16a,        shinobi,    generic_16a, ROT0,   "Sega",           "Shinobi (set 1, System 16A, FD1094 317-0050)", GAME_SUPPORTS_SAVE )
GAME( 1987, shinobls,   shinobi,  system16a,        shinobi,    generic_16a, ROT0,   "bootleg (Star)", "Shinobi (Star bootleg, System 16A)", GAME_SUPPORTS_SAVE )
GAME( 1987, shinoblb,   shinobi,  system16a,        shinobi,    generic_16a, ROT0,   "bootleg (Beta)", "Shinobi (Beta bootleg)", GAME_SUPPORTS_SAVE ) // should have different sound hw? using original ATM
GAME( 1987, sjryuko1,   sjryuko,  system16a,        sjryuko,    sjryukoa,    ROT0,   "White Board",    "Sukeban Jansi Ryuko (set 1, System 16A, FD1089B 317-5021)", GAME_SUPPORTS_SAVE )
GAME( 1988, tetris,     0,        system16a_no7751, tetris,     generic_16a, ROT0,   "Sega",           "Tetris (set 4, Japan, System 16A, FD1094 317-0093)", GAME_SUPPORTS_SAVE )
GAME( 1988, tetris3,    tetris,   system16a_no7751, tetris,     generic_16a, ROT0,   "Sega",           "Tetris (set 3, Japan, System 16A, FD1094 317-0093a)", GAME_SUPPORTS_SAVE )
GAME( 1987, timescan1,  timescan, system16a,        timescan,   fd1089b_16a, ROT270, "Sega",           "Time Scanner (set 1, System 16A, FD1089B 317-0024)", GAME_SUPPORTS_SAVE )
GAME( 1988, wb31,       wb3,      system16a_no7751, wb3,        generic_16a, ROT0,   "Sega / Westone", "Wonder Boy III - Monster Lair (set 1, System 16A, FD1094 317-0084)", GAME_SUPPORTS_SAVE )
GAME( 1988, wb35,       wb3,      system16a_no7751, wb3,        fd1089a_16a, ROT0,   "Sega / Westone", "Wonder Boy III - Monster Lair (set 5, System 16A, FD1089A 317-xxxx, bad dump?)", GAME_NOT_WORKING | GAME_SUPPORTS_SAVE )
GAME( 1988, wb35a,      wb3,      system16a_no7751, wb3,        fd1089a_16a, ROT0,   "Sega / Westone", "Wonder Boy III - Monster Lair (set 6, System 16A, FD1089A 317-xxxx)", GAME_NOT_WORKING | GAME_SUPPORTS_SAVE )
