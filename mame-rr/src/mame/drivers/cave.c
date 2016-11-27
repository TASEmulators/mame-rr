/***************************************************************************

                              -= Cave Hardware =-

                    driver by   Luca Elia (l.elia@tin.it)


Main  CPU    :  MC68000

Sound CPU    :  Z80 [Optional]

Sound Chips  :  YMZ280B or
                OKIM6295 x (1|2) + YM2203 / YM2151 [Optional]

Other        :  93C46 EEPROM


-----------------------------------------------------------------------------------------
Year + Game           License       PCB         Tilemaps        Sprites         Other
-----------------------------------------------------------------------------------------
94 Mazinger Z         Banpresto     ?           038 9335EX706   013 9341E7009   Z80
94 PowerInstinct 2    Atlus         ATG02?      038 9429WX709   013             Z80 NMK 112
95 P.I. Legends       Atlus         AT047G2-B   038 9429WX709   013 9341E7009   Z80 NMK 112
95 Metamoqester       Banpresto     BP947A      038 9437WX711   013 9346E7002   Z80
95 Sailor Moon        Banpresto     BP945A      038 9437WX711   013 9346E7002   Z80
95 Donpachi           Atlus         AT-C01DP-2  038 9429WX727   013 8647-01     NMK 112
96 Air Gallet         Banpresto     BP962A      038 9437WX711   013 9346E7002   Z80
96 Hotdog Storm       Marble        ASTC9501    038 9341EX702                   Z80
97 Dodonpachi         Atlus         ATC03D2     ?
98 Dangun Feveron     Nihon System  CV01        038 9808WX003   013 9807EX004
98 ESP Ra.De.         Atlus         ATC04       ?
98 Uo Poko            Jaleco        CV02        038 9749WX001   013 9749EX004
99 Guwange            Atlus         ATC05       ?
99 Gaia Crusaders     Noise Factory ?           038 9838WX003   013 9918EX008
99 Koro Koro Quest    Takumi        TUG-01B     038 9838WX004   013 9838EX004
99 Crusher Makochan   Takumi        TUG-01B     ""              ""
99 Tobikose! Jumpman  Namco         TJ0476      038 9919WX007   013 9934WX002
01 Thunder Heroes     Primetek      ?           038 9838WX003   013 9918EX008
-----------------------------------------------------------------------------------------

To Do:

- Sprite lag in some games (e.g. metmqstr). The sprites chip probably
  generates interrupts (unknown_irq)


Stephh's notes (based on the games M68000 code and some tests) :

1) 'gaia'

  - Difficulty Dip Switch also affects "Bonus Life" Dip Switch


2) 'theroes'

  - This is a English/Chinese version, but from the manual, there might exist a English/Japanese one
  - Difficulty Dip Switch also affects "Bonus Life" Dip Switch
  - There are less degrees of difficulty in this version
  - DSW2 bit 5 effect remains unknown :
      * it is checked at address 0x008d16 at the begining of each sub-level
      * it is checked at address 0x00c382 when you quickly push the joystick left or right twice
    Any info is welcome !

Versions known to exist but not dumped:
  Pretty Soldier Sailor Moon (95/03/21)
  Dodonpachi Campaign Version
     Reportedly only 3 ever made, one was given out as a prize to a high score contest winner.  The other two
     PCBs were shown running (and could be played) at a Cave fan show known as Cave Festival 2006. There are
     videos of the game being played floating around the internet and on YouTube. AKA DDP-CV or DDP BLUE ROM

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/eeprom.h"
#include "machine/nmk112.h"
#include "cpu/z80/z80.h"
#include "includes/cave.h"
#include "sound/2203intf.h"
#include "sound/2151intf.h"
#include "sound/okim6295.h"
#include "sound/ymz280b.h"

/***************************************************************************


                        Interrupt Handling Routines


***************************************************************************/


/* Update the IRQ state based on all possible causes */
static void update_irq_state( running_machine *machine )
{
	cave_state *state = (cave_state *)machine->driver_data;
	if (state->vblank_irq || state->sound_irq || state->unknown_irq)
		cpu_set_input_line(state->maincpu, state->irq_level, ASSERT_LINE);
	else
		cpu_set_input_line(state->maincpu, state->irq_level, CLEAR_LINE);
}

static TIMER_CALLBACK( cave_vblank_start )
{
	cave_state *state = (cave_state *)machine->driver_data;
	state->vblank_irq = 1;
	update_irq_state(machine);
	cave_get_sprite_info(machine);
	state->agallet_vblank_irq = 1;
}

static TIMER_CALLBACK( cave_vblank_end )
{
	cave_state *state = (cave_state *)machine->driver_data;
	if (state->kludge == 3)	/* mazinger metmqstr */
	{
		state->unknown_irq = 1;
		update_irq_state(machine);
	}
	state->agallet_vblank_irq = 0;
}

/* Called once/frame to generate the VBLANK interrupt */
static INTERRUPT_GEN( cave_interrupt )
{
	cave_state *state = (cave_state *)device->machine->driver_data;
	timer_set(device->machine, ATTOTIME_IN_USEC(17376 - state->time_vblank_irq), NULL, 0, cave_vblank_start);
	timer_set(device->machine, ATTOTIME_IN_USEC(17376 - state->time_vblank_irq + 2000), NULL, 0, cave_vblank_end);
}

/* Called by the YMZ280B to set the IRQ state */
static void sound_irq_gen( running_device *device, int state )
{
	cave_state *cave = (cave_state *)device->machine->driver_data;
	cave->sound_irq = (state != 0);
	update_irq_state(device->machine);
}


/*  Level 1 irq routines:

    Game        |first read | bit==0->routine + |
                |offset:    | read this offset  |

    ddonpach    4,0         0 -> vblank + 4     1 -> rte    2 -> like 0     read sound
    dfeveron    0           0 -> vblank + 4     1 -> + 6    -               read sound
    uopoko      0           0 -> vblank + 4     1 -> + 6    -               read sound
    esprade     0           0 -> vblank + 4     1 -> rte    2 must be 0     read sound
    guwange     0           0 -> vblank + 6,4   1 -> + 6,4  2 must be 0     read sound
    mazinger    0           0 -> vblank + 4     rest -> scroll + 6
*/


/* Reads the cause of the interrupt and clears the state */

static READ16_HANDLER( cave_irq_cause_r )
{
	cave_state *state = (cave_state *)space->machine->driver_data;
	int result = 0x0003;

	if (state->vblank_irq)
		result ^= 0x01;
	if (state->unknown_irq)
		result ^= 0x02;

	if (offset == 4/2)
		state->vblank_irq = 0;
	if (offset == 6/2)
		state->unknown_irq = 0;

	update_irq_state(space->machine);

/*
    sailormn and agallet wait for bit 2 of $b80001 to go 1 -> 0.
    It must happen once per frame as agallet uses this to show
    the copyright notice screen for ~8.5s
*/
	if (offset == 0)
	{
		result &= ~4;
		result |= (state->agallet_vblank_irq ? 0 : 4);
	}

	return result;
}


/***************************************************************************


                            Sound Handling Routines


***************************************************************************/

/*  We need a FIFO buffer for sailormn, where the inter-CPUs
    communication is *really* tight */

static READ8_HANDLER( soundflags_r )
{
	// bit 2 is low: can read command (lo)
	// bit 3 is low: can read command (hi)
//  cave_state *state = (cave_state *)space->machine->driver_data;
//  return  (state->sound_flag1 ? 0 : 4) |
//          (state->sound_flag2 ? 0 : 8) ;
return 0;
}

static READ16_HANDLER( soundflags_ack_r )
{
	// bit 0 is low: can write command
	// bit 1 is low: can read answer
	cave_state *state = (cave_state *)space->machine->driver_data;
//  return  ((state->sound_flag1 | state->sound_flag2) ? 1 : 0) |
//          ((state->soundbuf_len > 0) ? 0 : 2) ;

	return ((state->soundbuf_len > 0) ? 0 : 2) ;
}

/* Main CPU: write a 16 bit sound latch and generate a NMI on the sound CPU */
static WRITE16_HANDLER( sound_cmd_w )
{
	cave_state *state = (cave_state *)space->machine->driver_data;
//  state->sound_flag1 = 1;
//  state->sound_flag2 = 1;
	soundlatch_word_w(space, offset, data, mem_mask);
	cpu_set_input_line(state->audiocpu, INPUT_LINE_NMI, PULSE_LINE);
	cpu_spinuntil_time(space->cpu, ATTOTIME_IN_USEC(50));	// Allow the other cpu to reply
}

/* Sound CPU: read the low 8 bits of the 16 bit sound latch */
static READ8_HANDLER( soundlatch_lo_r )
{
//  cave_state *state = (cave_state *)space->machine->driver_data;
//  state->sound_flag1 = 0;
	return soundlatch_word_r(space, offset, 0x00ff) & 0xff;
}

/* Sound CPU: read the high 8 bits of the 16 bit sound latch */
static READ8_HANDLER( soundlatch_hi_r )
{
//  cave_state *state = (cave_state *)space->machine->driver_data;
//  state->sound_flag2 = 0;
	return soundlatch_word_r(space, offset, 0xff00) >> 8;
}

/* Main CPU: read the latch written by the sound CPU (acknowledge) */
static READ16_HANDLER( soundlatch_ack_r )
{
	cave_state *state = (cave_state *)space->machine->driver_data;
	if (state->soundbuf_len > 0)
	{
		UINT8 data = state->soundbuf_data[0];
		memmove(state->soundbuf_data, state->soundbuf_data + 1, (32 - 1) * sizeof(state->soundbuf_data[0]));
		state->soundbuf_len--;
		return data;
	}
	else
	{
		logerror("CPU #1 - PC %04X: Sound Buffer 2 Underflow Error\n", cpu_get_pc(space->cpu));
		return 0xff;
	}
}


/* Sound CPU: write latch for the main CPU (acknowledge) */
static WRITE8_HANDLER( soundlatch_ack_w )
{
	cave_state *state = (cave_state *)space->machine->driver_data;
	state->soundbuf_data[state->soundbuf_len] = data;
	if (state->soundbuf_len < 32)
		state->soundbuf_len++;
	else
		logerror("CPU #1 - PC %04X: Sound Buffer 2 Overflow Error\n", cpu_get_pc(space->cpu));
}



/***************************************************************************


                                    EEPROM


***************************************************************************/

static WRITE16_DEVICE_HANDLER( cave_eeprom_msb_w )
{
	if (data & ~0xfe00)
		logerror("%s: Unknown EEPROM bit written %04X\n", cpuexec_describe_context(device->machine), data);

	if (ACCESSING_BITS_8_15)  // even address
	{
		coin_lockout_w(device->machine, 1,~data & 0x8000);
		coin_lockout_w(device->machine, 0,~data & 0x4000);
		coin_counter_w(device->machine, 1, data & 0x2000);
		coin_counter_w(device->machine, 0, data & 0x1000);

		// latch the bit
		eeprom_write_bit(device, data & 0x0800);

		// reset line asserted: reset.
		eeprom_set_cs_line(device, (data & 0x0200) ? CLEAR_LINE : ASSERT_LINE);

		// clock line asserted: write latch or select next bit to read
		eeprom_set_clock_line(device, (data & 0x0400) ? ASSERT_LINE : CLEAR_LINE);
	}
}

static WRITE16_DEVICE_HANDLER( sailormn_eeprom_msb_w )
{
	sailormn_tilebank_w(device->machine, data & 0x0100);
	cave_eeprom_msb_w(device, offset, data & ~0x0100, mem_mask);
}

static WRITE16_DEVICE_HANDLER( hotdogst_eeprom_msb_w )
{
	if (ACCESSING_BITS_8_15)  // even address
	{
		// latch the bit
		eeprom_write_bit(device, data & 0x0800);

		// reset line asserted: reset.
		eeprom_set_cs_line(device, (data & 0x0200) ? CLEAR_LINE : ASSERT_LINE);

		// clock line asserted: write latch or select next bit to read
		eeprom_set_clock_line(device, (data & 0x0400) ? CLEAR_LINE: ASSERT_LINE);
	}
}

static WRITE16_DEVICE_HANDLER( cave_eeprom_lsb_w )
{
	if (data & ~0x00ef)
		logerror("%s: Unknown EEPROM bit written %04X\n",cpuexec_describe_context(device->machine),data);

	if (ACCESSING_BITS_0_7)  // odd address
	{
		coin_lockout_w(device->machine, 1, ~data & 0x0008);
		coin_lockout_w(device->machine, 0, ~data & 0x0004);
		coin_counter_w(device->machine, 1,  data & 0x0002);
		coin_counter_w(device->machine, 0,  data & 0x0001);

		// latch the bit
		eeprom_write_bit(device, data & 0x80);

		// reset line asserted: reset.
		eeprom_set_cs_line(device, (data & 0x20) ? CLEAR_LINE : ASSERT_LINE);

		// clock line asserted: write latch or select next bit to read
		eeprom_set_clock_line(device, (data & 0x40) ? ASSERT_LINE : CLEAR_LINE);
	}
}

/*  - No eeprom or lockouts */
static WRITE16_HANDLER( gaia_coin_lsb_w )
{
	if (ACCESSING_BITS_0_7)  // odd address
	{
		coin_counter_w(space->machine, 1, data & 0x0002);
		coin_counter_w(space->machine, 0, data & 0x0001);
	}
}

/*  - No coin lockouts
    - Writing 0xcf00 shouldn't send a 1 bit to the eeprom   */
static WRITE16_DEVICE_HANDLER( metmqstr_eeprom_msb_w )
{
	if (data & ~0xff00)
		logerror("%s: Unknown EEPROM bit written %04X\n", cpuexec_describe_context(device->machine), data);

	if (ACCESSING_BITS_8_15)  // even address
	{
		coin_counter_w(device->machine, 1, data & 0x2000);
		coin_counter_w(device->machine, 0, data & 0x1000);

		if (~data & 0x0100)
		{
			// latch the bit
			eeprom_write_bit(device, data & 0x0800);

			// reset line asserted: reset.
			eeprom_set_cs_line(device, (data & 0x0200) ? CLEAR_LINE : ASSERT_LINE);

			// clock line asserted: write latch or select next bit to read
			eeprom_set_clock_line(device, (data & 0x0400) ? ASSERT_LINE : CLEAR_LINE);
		}
	}
}

static const eeprom_interface eeprom_interface_93C46_8bit =
{
	7,				// address bits 7
	8,				// data bits    8
	"*110",			// read         1 10 aaaaaa
	"*101",			// write        1 01 aaaaaa dddddddddddddddd
	"*111",			// erase        1 11 aaaaaa
	"*10000xxxx",	// lock         1 00 00xxxx
	"*10011xxxx",	// unlock       1 00 11xxxx
	1,
//  "*10001xxxx"    // write all    1 00 01xxxx dddddddddddddddd
//  "*10010xxxx"    // erase all    1 00 10xxxx
};



/***************************************************************************


                            Memory Maps - Main CPU


***************************************************************************/

/*  Lines starting with an empty comment in the following MemoryReadAddress
     arrays are there for debug (e.g. the game does not read from those ranges
    AFAIK)  */

/***************************************************************************
                                Dangun Feveron
***************************************************************************/

static ADDRESS_MAP_START( dfeveron_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM																	// ROM
	AM_RANGE(0x100000, 0x10ffff) AM_RAM																	// RAM
	AM_RANGE(0x300000, 0x300003) AM_DEVREADWRITE8("ymz", ymz280b_r, ymz280b_w, 0x00ff)					// YMZ280
/**/AM_RANGE(0x400000, 0x407fff) AM_RAM AM_BASE_SIZE_MEMBER(cave_state, spriteram, spriteram_size)		// Sprites
/**/AM_RANGE(0x408000, 0x40ffff) AM_RAM AM_BASE_MEMBER(cave_state, spriteram_2)							// Sprites?
/**/AM_RANGE(0x500000, 0x507fff) AM_RAM_WRITE(cave_vram_0_w) AM_BASE_MEMBER(cave_state, vram_0)			// Layer 0
/**/AM_RANGE(0x600000, 0x607fff) AM_RAM_WRITE(cave_vram_1_w) AM_BASE_MEMBER(cave_state, vram_1)			// Layer 1
/**/AM_RANGE(0x708000, 0x708fff) AM_RAM AM_BASE_SIZE_MEMBER(cave_state, paletteram, paletteram_size)	// Palette
/**/AM_RANGE(0x710000, 0x710bff) AM_READONLY															// ?
	AM_RANGE(0x710c00, 0x710fff) AM_RAM																	// ?
	AM_RANGE(0x800000, 0x800007) AM_READ(cave_irq_cause_r)												// IRQ Cause
	AM_RANGE(0x800000, 0x80007f) AM_WRITEONLY AM_BASE_MEMBER(cave_state, videoregs)						// Video Regs
/**/AM_RANGE(0x900000, 0x900005) AM_RAM AM_BASE_MEMBER(cave_state, vctrl_0)								// Layer 0 Control
/**/AM_RANGE(0xa00000, 0xa00005) AM_RAM AM_BASE_MEMBER(cave_state, vctrl_1)								// Layer 1 Control
	AM_RANGE(0xb00000, 0xb00001) AM_READ_PORT("IN0")													// Inputs
	AM_RANGE(0xb00002, 0xb00003) AM_READ_PORT("IN1")													// Inputs + EEPROM
	AM_RANGE(0xc00000, 0xc00001) AM_DEVWRITE("eeprom", cave_eeprom_msb_w)								// EEPROM
ADDRESS_MAP_END


/***************************************************************************
                                Dodonpachi
***************************************************************************/

static ADDRESS_MAP_START( ddonpach_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM																	// ROM
	AM_RANGE(0x100000, 0x10ffff) AM_RAM																	// RAM
	AM_RANGE(0x300000, 0x300003) AM_DEVREADWRITE8("ymz", ymz280b_r, ymz280b_w, 0x00ff)					// YMZ280
/**/AM_RANGE(0x400000, 0x407fff) AM_RAM AM_BASE_SIZE_MEMBER(cave_state, spriteram, spriteram_size)		// Sprites
/**/AM_RANGE(0x408000, 0x40ffff) AM_RAM AM_BASE_MEMBER(cave_state, spriteram_2)							// Sprites?
/**/AM_RANGE(0x500000, 0x507fff) AM_RAM_WRITE(cave_vram_0_w) AM_BASE_MEMBER(cave_state, vram_0)			// Layer 0
/**/AM_RANGE(0x600000, 0x607fff) AM_RAM_WRITE(cave_vram_1_w) AM_BASE_MEMBER(cave_state, vram_1)			// Layer 1
/**/AM_RANGE(0x700000, 0x70ffff) AM_RAM_WRITE(cave_vram_2_8x8_w) AM_BASE_MEMBER(cave_state, vram_2)		// Layer 2
	AM_RANGE(0x800000, 0x800007) AM_READ(cave_irq_cause_r)												// IRQ Cause
	AM_RANGE(0x800000, 0x80007f) AM_WRITEONLY AM_BASE_MEMBER(cave_state, videoregs)						// Video Regs
/**/AM_RANGE(0x900000, 0x900005) AM_RAM AM_BASE_MEMBER(cave_state, vctrl_0)								// Layer 0 Control
/**/AM_RANGE(0xa00000, 0xa00005) AM_RAM AM_BASE_MEMBER(cave_state, vctrl_1)								// Layer 1 Control
/**/AM_RANGE(0xb00000, 0xb00005) AM_RAM AM_BASE_MEMBER(cave_state, vctrl_2)								// Layer 2 Control
/**/AM_RANGE(0xc00000, 0xc0ffff) AM_RAM AM_BASE_SIZE_MEMBER(cave_state, paletteram, paletteram_size)	// Palette
	AM_RANGE(0xd00000, 0xd00001) AM_READ_PORT("IN0")													// Inputs
	AM_RANGE(0xd00002, 0xd00003) AM_READ_PORT("IN1")													// Inputs + EEPROM
	AM_RANGE(0xe00000, 0xe00001) AM_DEVWRITE("eeprom", cave_eeprom_msb_w)								// EEPROM
ADDRESS_MAP_END


/***************************************************************************
                                    Donpachi
***************************************************************************/

static READ16_HANDLER( donpachi_videoregs_r )
{
	switch (offset)
	{
		case 0:
		case 1:
		case 2:
		case 3:	return cave_irq_cause_r(space, offset, 0xffff);

		default:	return 0x0000;
	}
}

#if 0
WRITE16_HANDLER( donpachi_videoregs_w )
{
	cave_state *state = (cave_state *)space->machine->driver_data;
	COMBINE_DATA(&state->videoregs[offset]);

	switch (offset)
	{
//      case 0x78/2:    watchdog_reset16_w(0, 0);    break;
	}
}
#endif

static ADDRESS_MAP_START( donpachi_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM																		// ROM
	AM_RANGE(0x100000, 0x10ffff) AM_RAM																		// RAM
	AM_RANGE(0x200000, 0x207fff) AM_RAM_WRITE(cave_vram_1_w) AM_BASE_MEMBER(cave_state, vram_1)				// Layer 1
	AM_RANGE(0x300000, 0x307fff) AM_RAM_WRITE(cave_vram_0_w) AM_BASE_MEMBER(cave_state, vram_0)				// Layer 0
	AM_RANGE(0x400000, 0x407fff) AM_RAM_WRITE(cave_vram_2_8x8_w) AM_BASE_MEMBER(cave_state, vram_2)			// Layer 2
	AM_RANGE(0x500000, 0x507fff) AM_RAM AM_BASE_SIZE_MEMBER(cave_state, spriteram, spriteram_size)			// Sprites
	AM_RANGE(0x508000, 0x50ffff) AM_RAM AM_BASE_MEMBER(cave_state, spriteram_2)								// Sprites?
/**/AM_RANGE(0x600000, 0x600005) AM_RAM AM_BASE_MEMBER(cave_state, vctrl_1)									// Layer 1 Control
/**/AM_RANGE(0x700000, 0x700005) AM_RAM AM_BASE_MEMBER(cave_state, vctrl_0)									// Layer 0 Control
/**/AM_RANGE(0x800000, 0x800005) AM_RAM AM_BASE_MEMBER(cave_state, vctrl_2)									// Layer 2 Control
	AM_RANGE(0x900000, 0x90007f) AM_RAM_READ(donpachi_videoregs_r) AM_BASE_MEMBER(cave_state, videoregs)	// Video Regs
/**/AM_RANGE(0xa08000, 0xa08fff) AM_RAM AM_BASE_SIZE_MEMBER(cave_state, paletteram, paletteram_size)		// Palette
	AM_RANGE(0xb00000, 0xb00003) AM_DEVREADWRITE8("oki1", okim6295_r, okim6295_w, 0x00ff)					// M6295
	AM_RANGE(0xb00010, 0xb00013) AM_DEVREADWRITE8("oki2", okim6295_r, okim6295_w, 0x00ff)					//
	AM_RANGE(0xb00020, 0xb0002f) AM_DEVWRITE("nmk112", nmk112_okibank_lsb_w)								//
	AM_RANGE(0xc00000, 0xc00001) AM_READ_PORT("IN0")														// Inputs
	AM_RANGE(0xc00002, 0xc00003) AM_READ_PORT("IN1")														// Inputs + EEPROM
	AM_RANGE(0xd00000, 0xd00001) AM_DEVWRITE("eeprom", cave_eeprom_msb_w)									// EEPROM
ADDRESS_MAP_END


/***************************************************************************
                                    Esprade
***************************************************************************/

static ADDRESS_MAP_START( esprade_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM																	// ROM
	AM_RANGE(0x100000, 0x10ffff) AM_RAM																	// RAM
	AM_RANGE(0x300000, 0x300003) AM_DEVREADWRITE8("ymz", ymz280b_r, ymz280b_w, 0x00ff)					// YMZ280
/**/AM_RANGE(0x400000, 0x407fff) AM_RAM AM_BASE_SIZE_MEMBER(cave_state, spriteram, spriteram_size)		// Sprites
/**/AM_RANGE(0x408000, 0x40ffff) AM_RAM AM_BASE_MEMBER(cave_state, spriteram_2)							// Sprites?
/**/AM_RANGE(0x500000, 0x507fff) AM_RAM_WRITE(cave_vram_0_w) AM_BASE_MEMBER(cave_state, vram_0)			// Layer 0
/**/AM_RANGE(0x600000, 0x607fff) AM_RAM_WRITE(cave_vram_1_w) AM_BASE_MEMBER(cave_state, vram_1)			// Layer 1
/**/AM_RANGE(0x700000, 0x707fff) AM_RAM_WRITE(cave_vram_2_w) AM_BASE_MEMBER(cave_state, vram_2)			// Layer 2
	AM_RANGE(0x800000, 0x800007) AM_READ(cave_irq_cause_r)												// IRQ Cause
	AM_RANGE(0x800000, 0x80007f) AM_WRITEONLY AM_BASE_MEMBER(cave_state, videoregs)						// Video Regs
/**/AM_RANGE(0x900000, 0x900005) AM_RAM AM_BASE_MEMBER(cave_state, vctrl_0)								// Layer 0 Control
/**/AM_RANGE(0xa00000, 0xa00005) AM_RAM AM_BASE_MEMBER(cave_state, vctrl_1)								// Layer 1 Control
/**/AM_RANGE(0xb00000, 0xb00005) AM_RAM AM_BASE_MEMBER(cave_state, vctrl_2)								// Layer 2 Control
/**/AM_RANGE(0xc00000, 0xc0ffff) AM_RAM AM_BASE_SIZE_MEMBER(cave_state, paletteram, paletteram_size)	// Palette
	AM_RANGE(0xd00000, 0xd00001) AM_READ_PORT("IN0" )													// Inputs
	AM_RANGE(0xd00002, 0xd00003) AM_READ_PORT("IN1" )													// Inputs + EEPROM
	AM_RANGE(0xe00000, 0xe00001) AM_DEVWRITE("eeprom", cave_eeprom_msb_w)								// EEPROM
ADDRESS_MAP_END


/***************************************************************************
                                    Gaia Crusaders
***************************************************************************/

static ADDRESS_MAP_START( gaia_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM																	// ROM
	AM_RANGE(0x100000, 0x10ffff) AM_RAM																	// RAM
	AM_RANGE(0x300000, 0x300003) AM_DEVREADWRITE8("ymz", ymz280b_r, ymz280b_w, 0x00ff)					// YMZ280
	AM_RANGE(0x400000, 0x407fff) AM_RAM AM_BASE_SIZE_MEMBER(cave_state, spriteram, spriteram_size)		// Sprite bank 1
	AM_RANGE(0x408000, 0x40ffff) AM_RAM AM_BASE_MEMBER(cave_state, spriteram_2)							// Sprite bank 2
	AM_RANGE(0x500000, 0x507fff) AM_RAM_WRITE(cave_vram_0_w) AM_BASE_MEMBER(cave_state, vram_0)			// Layer 0
	AM_RANGE(0x508000, 0x50ffff) AM_RAM																	// More Layer 0, Tested but not used?
	AM_RANGE(0x600000, 0x607fff) AM_RAM_WRITE(cave_vram_1_w) AM_BASE_MEMBER(cave_state, vram_1)			// Layer 1
	AM_RANGE(0x608000, 0x60ffff) AM_RAM																	// More Layer 1, Tested but not used?
	AM_RANGE(0x700000, 0x707fff) AM_RAM_WRITE(cave_vram_2_w) AM_BASE_MEMBER(cave_state, vram_2)			// Layer 2
	AM_RANGE(0x708000, 0x70ffff) AM_RAM																	// More Layer 2, Tested but not used?
	AM_RANGE(0x800000, 0x800007) AM_READ(cave_irq_cause_r)												// IRQ Cause
	AM_RANGE(0x800000, 0x80007f) AM_WRITEONLY AM_BASE_MEMBER(cave_state, videoregs)						// Video Regs
/**/AM_RANGE(0x900000, 0x900005) AM_RAM AM_BASE_MEMBER(cave_state, vctrl_0)								// Layer 0 Control
/**/AM_RANGE(0xa00000, 0xa00005) AM_RAM AM_BASE_MEMBER(cave_state, vctrl_1)								// Layer 1 Control
/**/AM_RANGE(0xb00000, 0xb00005) AM_RAM AM_BASE_MEMBER(cave_state, vctrl_2)								// Layer 2 Control
	AM_RANGE(0xc00000, 0xc0ffff) AM_RAM AM_BASE_SIZE_MEMBER(cave_state, paletteram, paletteram_size)	// Palette
	AM_RANGE(0xd00010, 0xd00011) AM_READ_PORT("IN0")													// Inputs
	AM_RANGE(0xd00010, 0xd00011) AM_WRITE(gaia_coin_lsb_w)												// Coin counter only
	AM_RANGE(0xd00012, 0xd00013) AM_READ_PORT("IN1")													// Inputs
	AM_RANGE(0xd00014, 0xd00015) AM_READ_PORT("DSW")													// Dips
	AM_RANGE(0xd00014, 0xd00015) AM_WRITE(watchdog_reset16_w)											// Watchdog?
ADDRESS_MAP_END


/***************************************************************************
                                    Guwange
***************************************************************************/

static ADDRESS_MAP_START( guwange_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM																	// ROM
	AM_RANGE(0x200000, 0x20ffff) AM_RAM																	// RAM
	AM_RANGE(0x300000, 0x300007) AM_READ(cave_irq_cause_r)												// IRQ Cause
	AM_RANGE(0x300000, 0x30007f) AM_WRITEONLY AM_BASE_MEMBER(cave_state, videoregs)						// Video Regs
/**/AM_RANGE(0x400000, 0x407fff) AM_RAM AM_BASE_SIZE_MEMBER(cave_state, spriteram, spriteram_size)		// Sprites
/**/AM_RANGE(0x408000, 0x40ffff) AM_RAM AM_BASE_MEMBER(cave_state, spriteram_2)							// Sprites?
/**/AM_RANGE(0x500000, 0x507fff) AM_RAM_WRITE(cave_vram_0_w) AM_BASE_MEMBER(cave_state, vram_0)			// Layer 0
/**/AM_RANGE(0x600000, 0x607fff) AM_RAM_WRITE(cave_vram_1_w) AM_BASE_MEMBER(cave_state, vram_1)			// Layer 1
/**/AM_RANGE(0x700000, 0x707fff) AM_RAM_WRITE(cave_vram_2_w) AM_BASE_MEMBER(cave_state, vram_2)			// Layer 2
	AM_RANGE(0x800000, 0x800003) AM_DEVREADWRITE8("ymz", ymz280b_r, ymz280b_w, 0x00ff)					// YMZ280
/**/AM_RANGE(0x900000, 0x900005) AM_RAM AM_BASE_MEMBER(cave_state, vctrl_0)								// Layer 0 Control
/**/AM_RANGE(0xa00000, 0xa00005) AM_RAM AM_BASE_MEMBER(cave_state, vctrl_1)								// Layer 1 Control
/**/AM_RANGE(0xb00000, 0xb00005) AM_RAM AM_BASE_MEMBER(cave_state, vctrl_2)								// Layer 2 Control
/**/AM_RANGE(0xc00000, 0xc0ffff) AM_RAM AM_BASE_SIZE_MEMBER(cave_state, paletteram, paletteram_size)	// Palette
	AM_RANGE(0xd00010, 0xd00011) AM_READ_PORT("IN0")													// Inputs
	AM_RANGE(0xd00010, 0xd00011) AM_DEVWRITE("eeprom", cave_eeprom_lsb_w)								// EEPROM
	AM_RANGE(0xd00012, 0xd00013) AM_READ_PORT("IN1")													// Inputs + EEPROM
//  AM_RANGE(0xd00012, 0xd00013) AM_WRITENOP                                                            // ?
//  AM_RANGE(0xd00014, 0xd00015) AM_WRITENOP                                                            // ? $800068 in dfeveron ? probably Watchdog
ADDRESS_MAP_END


/***************************************************************************
                                Hotdog Storm
***************************************************************************/

static ADDRESS_MAP_START( hotdogst_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM																	// ROM
	AM_RANGE(0x300000, 0x30ffff) AM_RAM																	// RAM
/**/AM_RANGE(0x408000, 0x408fff) AM_RAM AM_BASE_SIZE_MEMBER(cave_state, paletteram, paletteram_size)	// Palette
/**/AM_RANGE(0x880000, 0x887fff) AM_RAM_WRITE(cave_vram_0_w) AM_BASE_MEMBER(cave_state, vram_0)			// Layer 0
/**/AM_RANGE(0x900000, 0x907fff) AM_RAM_WRITE(cave_vram_1_w) AM_BASE_MEMBER(cave_state, vram_1)			// Layer 1
/**/AM_RANGE(0x980000, 0x987fff) AM_RAM_WRITE(cave_vram_2_w) AM_BASE_MEMBER(cave_state, vram_2)			// Layer 2
	AM_RANGE(0xa80000, 0xa80007) AM_READ(cave_irq_cause_r)												// IRQ Cause
//  AM_RANGE(0xa8006e, 0xa8006f) AM_READ(soundlatch_ack_r)                                              // From Sound CPU
	AM_RANGE(0xa8006e, 0xa8006f) AM_WRITE(sound_cmd_w)													// To Sound CPU
	AM_RANGE(0xa80000, 0xa8007f) AM_WRITEONLY AM_BASE_MEMBER(cave_state, videoregs)						// Video Regs
/**/AM_RANGE(0xb00000, 0xb00005) AM_RAM AM_BASE_MEMBER(cave_state, vctrl_0)								// Layer 0 Control
/**/AM_RANGE(0xb80000, 0xb80005) AM_RAM AM_BASE_MEMBER(cave_state, vctrl_1)								// Layer 1 Control
/**/AM_RANGE(0xc00000, 0xc00005) AM_RAM AM_BASE_MEMBER(cave_state, vctrl_2)								// Layer 2 Control
	AM_RANGE(0xc80000, 0xc80001) AM_READ_PORT("IN0")													// Inputs
	AM_RANGE(0xc80002, 0xc80003) AM_READ_PORT("IN1")													// Inputs + EEPROM
	AM_RANGE(0xd00000, 0xd00001) AM_DEVWRITE("eeprom", hotdogst_eeprom_msb_w)							// EEPROM
	AM_RANGE(0xd00002, 0xd00003) AM_WRITENOP															// ???
/**/AM_RANGE(0xf00000, 0xf07fff) AM_RAM AM_BASE_SIZE_MEMBER(cave_state, spriteram, spriteram_size)		// Sprites
/**/AM_RANGE(0xf08000, 0xf0ffff) AM_RAM AM_BASE_MEMBER(cave_state, spriteram_2)							// Sprites?
ADDRESS_MAP_END


/***************************************************************************
                               Koro Koro Quest
***************************************************************************/

static void show_leds(running_machine *machine)
{
#ifdef MAME_DEBUG
//  cave_state *state = (cave_state *)machine->driver_data;
//  popmessage("led %04X eep %02X", state->leds[0], (state->leds[1] >> 8) & ~0x70);
#endif
}

static WRITE16_HANDLER( korokoro_leds_w )
{
	cave_state *state = (cave_state *)space->machine->driver_data;
	COMBINE_DATA(&state->leds[0]);

	set_led_status(space->machine, 0, data & 0x8000);
	set_led_status(space->machine, 1, data & 0x4000);
	set_led_status(space->machine, 2, data & 0x1000);	// square button
	set_led_status(space->machine, 3, data & 0x0800);	// round  button
//  coin_lockout_w(space->machine, 1, ~data & 0x0200);   // coin lockouts?
//  coin_lockout_w(space->machine, 0, ~data & 0x0100);

//  coin_counter_w(space->machine, 2, data & 0x0080);
//  coin_counter_w(space->machine, 1, data & 0x0020);
	coin_counter_w(space->machine, 0, data & 0x0010);

	set_led_status(space->machine, 5, data & 0x0008);
	set_led_status(space->machine, 6, data & 0x0004);
	set_led_status(space->machine, 7, data & 0x0002);
	set_led_status(space->machine, 8, data & 0x0001);

	show_leds(space->machine);
}


static WRITE16_DEVICE_HANDLER( korokoro_eeprom_msb_w )
{
	cave_state *state = (cave_state *)device->machine->driver_data;
	if (data & ~0x7000)
	{
		logerror("%s: Unknown EEPROM bit written %04X\n",cpuexec_describe_context(device->machine),data);
		COMBINE_DATA(&state->leds[1]);
		show_leds(device->machine);
	}

	if (ACCESSING_BITS_8_15)  // even address
	{
		state->hopper = data & 0x0100;	// ???

		// latch the bit
		eeprom_write_bit(device, data & 0x4000);

		// reset line asserted: reset.
		eeprom_set_cs_line(device, (data & 0x1000) ? CLEAR_LINE : ASSERT_LINE);

		// clock line asserted: write latch or select next bit to read
		eeprom_set_clock_line(device, (data & 0x2000) ? ASSERT_LINE : CLEAR_LINE);
	}
}

static CUSTOM_INPUT( korokoro_hopper_r )
{
	cave_state *state = (cave_state *)field->port->machine->driver_data;
	return state->hopper ? 1 : 0;
}


static ADDRESS_MAP_START( korokoro_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM																		// ROM
	AM_RANGE(0x100000, 0x107fff) AM_WRITE(cave_vram_0_w) AM_BASE_MEMBER(cave_state, vram_0)					// Layer 0
	AM_RANGE(0x140000, 0x140005) AM_WRITEONLY AM_BASE_MEMBER(cave_state, vctrl_0)							// Layer 0 Control
	AM_RANGE(0x180000, 0x187fff) AM_WRITEONLY AM_BASE_SIZE_MEMBER(cave_state, spriteram, spriteram_size)	// Sprites
	AM_RANGE(0x1c0000, 0x1c0007) AM_READ(cave_irq_cause_r)													// IRQ Cause
	AM_RANGE(0x1c0000, 0x1c007f) AM_WRITEONLY AM_BASE_MEMBER(cave_state, videoregs)							// Video Regs
	AM_RANGE(0x200000, 0x207fff) AM_WRITEONLY AM_BASE_SIZE_MEMBER(cave_state, paletteram, paletteram_size)	// Palette
//  AM_RANGE(0x240000, 0x240003) AM_DEVREAD8( "ymz", ymz280b_r, 0x00ff)                                     // YMZ280
	AM_RANGE(0x240000, 0x240003) AM_DEVWRITE8( "ymz", ymz280b_w, 0x00ff)									// YMZ280
	AM_RANGE(0x280000, 0x280001) AM_READ_PORT("IN0")														// Inputs + ???
	AM_RANGE(0x280002, 0x280003) AM_READ_PORT("IN1")														// Inputs + EEPROM
	AM_RANGE(0x280008, 0x280009) AM_WRITE(korokoro_leds_w)													// Leds
	AM_RANGE(0x28000a, 0x28000b) AM_DEVWRITE("eeprom", korokoro_eeprom_msb_w)								// EEPROM
	AM_RANGE(0x28000c, 0x28000d) AM_WRITENOP																// 0 (watchdog?)
	AM_RANGE(0x300000, 0x30ffff) AM_RAM																		// RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( crusherm_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM																		// ROM
	AM_RANGE(0x100000, 0x107fff) AM_WRITE(cave_vram_0_w) AM_BASE_MEMBER(cave_state, vram_0)					// Layer 0
	AM_RANGE(0x140000, 0x140005) AM_WRITEONLY AM_BASE_MEMBER(cave_state, vctrl_0)							// Layer 0 Control
	AM_RANGE(0x180000, 0x187fff) AM_WRITEONLY AM_BASE_SIZE_MEMBER(cave_state, spriteram, spriteram_size)	// Sprites
	AM_RANGE(0x200000, 0x207fff) AM_WRITEONLY AM_BASE_SIZE_MEMBER(cave_state, paletteram, paletteram_size)	// Palette
	AM_RANGE(0x240000, 0x240003) AM_DEVWRITE8( "ymz", ymz280b_w, 0x00ff)									// YMZ280
	AM_RANGE(0x280000, 0x280001) AM_READ_PORT("IN0")														// Inputs + ???
	AM_RANGE(0x280002, 0x280003) AM_READ_PORT("IN1")														// Inputs + EEPROM
	AM_RANGE(0x280008, 0x280009) AM_WRITE(korokoro_leds_w)													// Leds
	AM_RANGE(0x28000a, 0x28000b) AM_DEVWRITE("eeprom", korokoro_eeprom_msb_w)								// EEPROM
	AM_RANGE(0x28000c, 0x28000d) AM_WRITENOP																// 0 (watchdog?)
	AM_RANGE(0x300000, 0x300007) AM_READ(cave_irq_cause_r)													// IRQ Cause
	AM_RANGE(0x300000, 0x30007f) AM_WRITEONLY AM_BASE_MEMBER(cave_state, videoregs)							// Video Regs
	AM_RANGE(0x340000, 0x34ffff) AM_RAM																		// RAM
ADDRESS_MAP_END

/***************************************************************************
                                Mazinger Z
***************************************************************************/

static ADDRESS_MAP_START( mazinger_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM																	// ROM
	AM_RANGE(0x100000, 0x10ffff) AM_RAM																	// RAM
/**/AM_RANGE(0x200000, 0x207fff) AM_RAM AM_BASE_SIZE_MEMBER(cave_state, spriteram, spriteram_size)		// Sprites
/**/AM_RANGE(0x208000, 0x20ffff) AM_RAM AM_BASE_MEMBER(cave_state, spriteram_2)							// Sprites?
	AM_RANGE(0x300000, 0x300007) AM_READ(cave_irq_cause_r)												// IRQ Cause
	AM_RANGE(0x300068, 0x300069) AM_WRITE(watchdog_reset16_w)											// Watchdog
	AM_RANGE(0x30006e, 0x30006f) AM_READWRITE(soundlatch_ack_r, sound_cmd_w)							// From Sound CPU
	AM_RANGE(0x300000, 0x30007f) AM_WRITEONLY AM_BASE_MEMBER(cave_state, videoregs)						// Video Regs
	AM_RANGE(0x400000, 0x407fff) AM_RAM_WRITE(cave_vram_1_8x8_w) AM_BASE_MEMBER(cave_state, vram_1)		// Layer 1
/**/AM_RANGE(0x500000, 0x507fff) AM_RAM_WRITE(cave_vram_0_8x8_w) AM_BASE_MEMBER(cave_state, vram_0)		// Layer 0
/**/AM_RANGE(0x600000, 0x600005) AM_RAM AM_BASE_MEMBER(cave_state, vctrl_1)								// Layer 1 Control
/**/AM_RANGE(0x700000, 0x700005) AM_RAM AM_BASE_MEMBER(cave_state, vctrl_0)								// Layer 0 Control
	AM_RANGE(0x800000, 0x800001) AM_READ_PORT("IN0")													// Inputs
	AM_RANGE(0x800002, 0x800003) AM_READ_PORT("IN1")													// Inputs + EEPROM
	AM_RANGE(0x900000, 0x900001) AM_DEVWRITE("eeprom", cave_eeprom_msb_w)								// EEPROM
/**/AM_RANGE(0xc08000, 0xc0ffff) AM_RAM AM_BASE_SIZE_MEMBER(cave_state, paletteram, paletteram_size)	// Palette
	AM_RANGE(0xd00000, 0xd7ffff) AM_ROMBANK("bank1")													// ROM
ADDRESS_MAP_END


/***************************************************************************
                                Metamoqester
***************************************************************************/

static ADDRESS_MAP_START( metmqstr_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM																	// ROM
	AM_RANGE(0x100000, 0x17ffff) AM_ROM																	// ROM
	AM_RANGE(0x200000, 0x27ffff) AM_ROM																	// ROM
	AM_RANGE(0x408000, 0x408fff) AM_RAM AM_BASE_SIZE_MEMBER(cave_state, paletteram, paletteram_size)	// Palette
	AM_RANGE(0x600000, 0x600001) AM_READ(watchdog_reset16_r)											// Watchdog?
	AM_RANGE(0x880000, 0x887fff) AM_RAM_WRITE(cave_vram_2_w) AM_BASE_MEMBER(cave_state, vram_2)			// Layer 2
	AM_RANGE(0x888000, 0x88ffff) AM_RAM																	//
	AM_RANGE(0x900000, 0x907fff) AM_RAM_WRITE(cave_vram_1_w) AM_BASE_MEMBER(cave_state, vram_1)			// Layer 1
	AM_RANGE(0x908000, 0x90ffff) AM_RAM																	//
	AM_RANGE(0x980000, 0x987fff) AM_RAM_WRITE(cave_vram_0_w) AM_BASE_MEMBER(cave_state, vram_0)			// Layer 0
	AM_RANGE(0x988000, 0x98ffff) AM_RAM																	//
	AM_RANGE(0xa80000, 0xa80007) AM_READ(cave_irq_cause_r)												// IRQ Cause
	AM_RANGE(0xa80068, 0xa80069) AM_WRITE(watchdog_reset16_w)											// Watchdog?
	AM_RANGE(0xa8006c, 0xa8006d) AM_READ(soundflags_ack_r) AM_WRITENOP									// Communication
	AM_RANGE(0xa8006e, 0xa8006f) AM_READWRITE(soundlatch_ack_r, sound_cmd_w)							// From Sound CPU
	AM_RANGE(0xa80000, 0xa8007f) AM_WRITEONLY AM_BASE_MEMBER(cave_state, videoregs)						// Video Regs
/**/AM_RANGE(0xb00000, 0xb00005) AM_RAM AM_BASE_MEMBER(cave_state, vctrl_2)								// Layer 2 Control
/**/AM_RANGE(0xb80000, 0xb80005) AM_RAM AM_BASE_MEMBER(cave_state, vctrl_1)								// Layer 1 Control
/**/AM_RANGE(0xc00000, 0xc00005) AM_RAM AM_BASE_MEMBER(cave_state, vctrl_0)								// Layer 0 Control
	AM_RANGE(0xc80000, 0xc80001) AM_READ_PORT("IN0")													// Inputs
	AM_RANGE(0xc80002, 0xc80003) AM_READ_PORT("IN1")													// Inputs + EEPROM
	AM_RANGE(0xd00000, 0xd00001) AM_DEVWRITE("eeprom", metmqstr_eeprom_msb_w)							// EEPROM
	AM_RANGE(0xf00000, 0xf07fff) AM_RAM AM_BASE_SIZE_MEMBER(cave_state, spriteram, spriteram_size)		// Sprites
	AM_RANGE(0xf08000, 0xf0ffff) AM_RAM AM_BASE_MEMBER(cave_state, spriteram_2)							// RAM
ADDRESS_MAP_END


/***************************************************************************
                                Power Instinct 2
***************************************************************************/

static READ16_DEVICE_HANDLER( pwrinst2_eeprom_r )
{
	return ~8 + ((eeprom_read_bit(device) & 1) ? 8 : 0);
}

INLINE void vctrl_w(UINT16 *VCTRL, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT16 data, ATTR_UNUSED UINT16 mem_mask)
{
	if (offset == 4 / 2)
	{
		switch (data & 0x000f)
		{
			case 1:	data = (data & ~0x000f) | 0;	break;
			case 2:	data = (data & ~0x000f) | 1;	break;
			case 4:	data = (data & ~0x000f) | 2;	break;
			default:
			case 8:	data = (data & ~0x000f) | 3;	break;
		}
	}
	COMBINE_DATA(&VCTRL[offset]);
}
static WRITE16_HANDLER( pwrinst2_vctrl_0_w )	{ cave_state *state = (cave_state *)space->machine->driver_data; vctrl_w(state->vctrl_0, offset, data, mem_mask); }
static WRITE16_HANDLER( pwrinst2_vctrl_1_w )	{ cave_state *state = (cave_state *)space->machine->driver_data; vctrl_w(state->vctrl_1, offset, data, mem_mask); }
static WRITE16_HANDLER( pwrinst2_vctrl_2_w )	{ cave_state *state = (cave_state *)space->machine->driver_data; vctrl_w(state->vctrl_2, offset, data, mem_mask); }
static WRITE16_HANDLER( pwrinst2_vctrl_3_w )	{ cave_state *state = (cave_state *)space->machine->driver_data; vctrl_w(state->vctrl_3, offset, data, mem_mask); }

static ADDRESS_MAP_START( pwrinst2_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM																		// ROM
	AM_RANGE(0x400000, 0x40ffff) AM_RAM																		// RAM
	AM_RANGE(0x500000, 0x500001) AM_READ_PORT("IN0")														// Inputs
	AM_RANGE(0x500002, 0x500003) AM_READ_PORT("IN1")														//
	AM_RANGE(0x600000, 0x6fffff) AM_ROM AM_REGION("user1", 0)												// extra data ROM space
	AM_RANGE(0x700000, 0x700001) AM_DEVWRITE("eeprom", cave_eeprom_msb_w)									// EEPROM
	AM_RANGE(0x800000, 0x807fff) AM_RAM_WRITE(cave_vram_2_w) AM_BASE_MEMBER(cave_state, vram_2)				// Layer 2
	AM_RANGE(0x880000, 0x887fff) AM_RAM_WRITE(cave_vram_0_w) AM_BASE_MEMBER(cave_state, vram_0)				// Layer 0
	AM_RANGE(0x900000, 0x907fff) AM_RAM_WRITE(cave_vram_1_w) AM_BASE_MEMBER(cave_state, vram_1)				// Layer 1
	AM_RANGE(0x980000, 0x987fff) AM_RAM_WRITE(cave_vram_3_8x8_w) AM_BASE_MEMBER(cave_state, vram_3)			// Layer 3
	AM_RANGE(0xa00000, 0xa07fff) AM_RAM AM_BASE_SIZE_MEMBER(cave_state, spriteram, spriteram_size)			// Sprites
	AM_RANGE(0xa08000, 0xa0ffff) AM_RAM AM_BASE_MEMBER(cave_state, spriteram_2)								// Sprites?
	AM_RANGE(0xa10000, 0xa1ffff) AM_RAM																		// Sprites?
	AM_RANGE(0xa80000, 0xa8007f) AM_RAM_READ(donpachi_videoregs_r) AM_BASE_MEMBER(cave_state, videoregs)	// Video Regs
/**/AM_RANGE(0xb00000, 0xb00005) AM_RAM_WRITE(pwrinst2_vctrl_2_w) AM_BASE_MEMBER(cave_state, vctrl_2)		// Layer 2 Control
/**/AM_RANGE(0xb80000, 0xb80005) AM_RAM_WRITE(pwrinst2_vctrl_0_w) AM_BASE_MEMBER(cave_state, vctrl_0)		// Layer 0 Control
/**/AM_RANGE(0xc00000, 0xc00005) AM_RAM_WRITE(pwrinst2_vctrl_1_w) AM_BASE_MEMBER(cave_state, vctrl_1)		// Layer 1 Control
/**/AM_RANGE(0xc80000, 0xc80005) AM_RAM_WRITE(pwrinst2_vctrl_3_w) AM_BASE_MEMBER(cave_state, vctrl_3)		// Layer 3 Control
	AM_RANGE(0xd80000, 0xd80001) AM_READ(soundlatch_ack_r)													// ? From Sound CPU
	AM_RANGE(0xe00000, 0xe00001) AM_WRITE(sound_cmd_w)														// To Sound CPU
	AM_RANGE(0xe80000, 0xe80001) AM_DEVREAD("eeprom", pwrinst2_eeprom_r)									// EEPROM
	AM_RANGE(0xf00000, 0xf04fff) AM_RAM AM_BASE_SIZE_MEMBER(cave_state, paletteram, paletteram_size)		// Palette
ADDRESS_MAP_END


/***************************************************************************
                                Sailor Moon
***************************************************************************/

static READ16_HANDLER( sailormn_input0_r )
{
//  watchdog_reset16_r(0, 0);    // written too rarely for mame.
	return input_port_read(space->machine, "IN0");
}

static ADDRESS_MAP_START( sailormn_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM																	// ROM
	AM_RANGE(0x100000, 0x10ffff) AM_RAM																	// RAM
	AM_RANGE(0x110000, 0x110001) AM_RAM																	// (agallet)
	AM_RANGE(0x200000, 0x3fffff) AM_ROM																	// ROM
	AM_RANGE(0x400000, 0x407fff) AM_RAM																	// (agallet)
	AM_RANGE(0x408000, 0x40bfff) AM_RAM AM_BASE_SIZE_MEMBER(cave_state, paletteram, paletteram_size)	// Palette
	AM_RANGE(0x40c000, 0x40ffff) AM_RAM																	// (agallet)
	AM_RANGE(0x410000, 0x410001) AM_RAM																	// (agallet)
	AM_RANGE(0x500000, 0x507fff) AM_RAM AM_BASE_SIZE_MEMBER(cave_state, spriteram, spriteram_size)		// Sprites
	AM_RANGE(0x508000, 0x50ffff) AM_RAM AM_BASE_MEMBER(cave_state, spriteram_2)							// Sprites?
	AM_RANGE(0x510000, 0x510001) AM_RAM																	// (agallet)
	AM_RANGE(0x600000, 0x600001) AM_READ(sailormn_input0_r)												// Inputs + Watchdog!
	AM_RANGE(0x600002, 0x600003) AM_READ_PORT("IN1")													// Inputs + EEPROM
	AM_RANGE(0x700000, 0x700001) AM_DEVWRITE("eeprom", sailormn_eeprom_msb_w)							// EEPROM
	AM_RANGE(0x800000, 0x807fff) AM_RAM_WRITE(cave_vram_0_w) AM_BASE_MEMBER(cave_state, vram_0)			// Layer 0
	AM_RANGE(0x880000, 0x887fff) AM_RAM_WRITE(cave_vram_1_w) AM_BASE_MEMBER(cave_state, vram_1)			// Layer 1
	AM_RANGE(0x900000, 0x907fff) AM_RAM_WRITE(cave_vram_2_w) AM_BASE_MEMBER(cave_state, vram_2)			// Layer 2
	AM_RANGE(0x908000, 0x908001) AM_RAM																	// (agallet)
/**/AM_RANGE(0xa00000, 0xa00005) AM_RAM AM_BASE_MEMBER(cave_state, vctrl_0)								// Layer 0 Control
/**/AM_RANGE(0xa80000, 0xa80005) AM_RAM AM_BASE_MEMBER(cave_state, vctrl_1)								// Layer 1 Control
/**/AM_RANGE(0xb00000, 0xb00005) AM_RAM AM_BASE_MEMBER(cave_state, vctrl_2)								// Layer 2 Control
	AM_RANGE(0xb80000, 0xb80007) AM_READ(cave_irq_cause_r)												// IRQ Cause (bit 2 tested!)
	AM_RANGE(0xb8006c, 0xb8006d) AM_READ(soundflags_ack_r)												// Communication
	AM_RANGE(0xb8006e, 0xb8006f) AM_READ(soundlatch_ack_r)												// From Sound CPU
	AM_RANGE(0xb8006e, 0xb8006f) AM_WRITE(sound_cmd_w)													// To Sound CPU
	AM_RANGE(0xb80000, 0xb8007f) AM_WRITEONLY AM_BASE_MEMBER(cave_state, videoregs)						// Video Regs
ADDRESS_MAP_END


/***************************************************************************
                            Tobikose! Jumpman
***************************************************************************/

static WRITE16_DEVICE_HANDLER( tjumpman_eeprom_lsb_w )
{
	if (data & ~0x0038)
		logerror("%s: Unknown EEPROM bit written %04X\n",cpuexec_describe_context(device->machine),data);

	if (ACCESSING_BITS_0_7)  // odd address
	{
		// latch the bit
		eeprom_write_bit(device, data & 0x0020);

		// reset line asserted: reset.
		eeprom_set_cs_line(device, (data & 0x0008) ? CLEAR_LINE : ASSERT_LINE);

		// clock line asserted: write latch or select next bit to read
		eeprom_set_clock_line(device, (data & 0x0010) ? ASSERT_LINE : CLEAR_LINE);
	}
}

static WRITE16_HANDLER( tjumpman_leds_w )
{
	cave_state *state = (cave_state *)space->machine->driver_data;
	if (ACCESSING_BITS_0_7)
	{
		set_led_status(space->machine, 0,	data & 0x0001);	// suru
		set_led_status(space->machine, 1,	data & 0x0002);	// shinai
		set_led_status(space->machine, 2,	data & 0x0004);	// payout
		set_led_status(space->machine, 3,	data & 0x0008);	// go
		set_led_status(space->machine, 4,	data & 0x0010);	// 1 bet
		set_led_status(space->machine, 5,	data & 0x0020);	// medal
		state->hopper	=					data & 0x0040;	// hopper
		set_led_status(space->machine, 6,	data & 0x0080);	// 3 bet
	}

//  popmessage("led %04X", data);
}

static CUSTOM_INPUT( tjumpman_hopper_r )
{
	cave_state *state = (cave_state *)field->port->machine->driver_data;
	return (state->hopper && !(field->port->machine->primary_screen->frame_number() % 10)) ? 0 : 1;
}

static ADDRESS_MAP_START( tjumpman_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM																	// ROM
	AM_RANGE(0x100000, 0x10ffff) AM_RAM AM_BASE_SIZE_GENERIC( nvram )									// RAM
	AM_RANGE(0x200000, 0x207fff) AM_RAM AM_BASE_SIZE_MEMBER(cave_state, spriteram, spriteram_size)		// Sprites
	AM_RANGE(0x208000, 0x20ffff) AM_RAM AM_BASE_MEMBER(cave_state, spriteram_2)							// Sprite bank 2
	AM_RANGE(0x304000, 0x307fff) AM_WRITE(cave_vram_0_w)												// Layer 0 - 16x16 tiles mapped here
	AM_RANGE(0x300000, 0x307fff) AM_RAM_WRITE(cave_vram_0_w) AM_BASE_MEMBER(cave_state, vram_0)			// Layer 0
	AM_RANGE(0x400000, 0x400005) AM_WRITEONLY AM_BASE_MEMBER(cave_state, vctrl_0)						// Layer 0 Control
	AM_RANGE(0x500000, 0x50ffff) AM_RAM AM_BASE_SIZE_MEMBER(cave_state, paletteram, paletteram_size)	// Palette
	AM_RANGE(0x600000, 0x600001) AM_READ_PORT("IN0")													// Inputs + EEPROM + Hopper
	AM_RANGE(0x600002, 0x600003) AM_READ_PORT("IN1")													// Inputs
	AM_RANGE(0x700000, 0x700007) AM_READ(cave_irq_cause_r)												// IRQ Cause
	AM_RANGE(0x700068, 0x700069) AM_WRITE(watchdog_reset16_w)											// Watchdog
	AM_RANGE(0x700000, 0x70007f) AM_WRITEONLY AM_BASE_MEMBER(cave_state, videoregs)						// Video Regs
	AM_RANGE(0x800000, 0x800001) AM_DEVREADWRITE8("oki1", okim6295_r, okim6295_w, 0x00ff)				// M6295
	AM_RANGE(0xc00000, 0xc00001) AM_WRITE(tjumpman_leds_w)												// Leds + Hopper
	AM_RANGE(0xe00000, 0xe00001) AM_DEVWRITE("eeprom", tjumpman_eeprom_lsb_w)							// EEPROM
ADDRESS_MAP_END


/***************************************************************************
                                    Uo Poko
***************************************************************************/

static ADDRESS_MAP_START( uopoko_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM																	// ROM
	AM_RANGE(0x100000, 0x10ffff) AM_RAM																	// RAM
	AM_RANGE(0x300000, 0x300003) AM_DEVREADWRITE8("ymz", ymz280b_r, ymz280b_w, 0x00ff)					// YMZ280
/**/AM_RANGE(0x400000, 0x407fff) AM_RAM AM_BASE_SIZE_MEMBER(cave_state, spriteram, spriteram_size)		// Sprites
/**/AM_RANGE(0x408000, 0x40ffff) AM_RAM AM_BASE_MEMBER(cave_state, spriteram_2)							// Sprites?
/**/AM_RANGE(0x500000, 0x507fff) AM_RAM_WRITE(cave_vram_0_w) AM_BASE_MEMBER(cave_state, vram_0)			// Layer 0
	AM_RANGE(0x600000, 0x600007) AM_READ(cave_irq_cause_r)												// IRQ Cause
	AM_RANGE(0x600000, 0x60007f) AM_WRITEONLY AM_BASE_MEMBER(cave_state, videoregs)						// Video Regs
/**/AM_RANGE(0x700000, 0x700005) AM_RAM AM_BASE_MEMBER(cave_state, vctrl_0)								// Layer 0 Control
/**/AM_RANGE(0x800000, 0x80ffff) AM_RAM AM_BASE_SIZE_MEMBER(cave_state, paletteram, paletteram_size)	// Palette
	AM_RANGE(0x900000, 0x900001) AM_READ_PORT("IN0")													// Inputs
	AM_RANGE(0x900002, 0x900003) AM_READ_PORT("IN1")													// Inputs + EEPROM
	AM_RANGE(0xa00000, 0xa00001) AM_DEVWRITE("eeprom", cave_eeprom_msb_w)								// EEPROM
ADDRESS_MAP_END



/***************************************************************************


                        Memory Maps - Sound CPU (Optional)


***************************************************************************/

/***************************************************************************
                                Hotdog Storm
***************************************************************************/

static WRITE8_HANDLER( hotdogst_rombank_w )
{
	if (data & ~0x0f)
		logerror("CPU #1 - PC %04X: Bank %02X\n", cpu_get_pc(space->cpu), data);

	memory_set_bank(space->machine, "bank2", data & 0x0f);
}

static WRITE8_HANDLER( hotdogst_okibank_w )
{
	UINT8 *RAM = memory_region(space->machine, "oki");
	int bank1 = (data >> 0) & 0x3;
	int bank2 = (data >> 4) & 0x3;
	memcpy(RAM + 0x20000 * 0, RAM + 0x40000 + 0x20000 * bank1, 0x20000);
	memcpy(RAM + 0x20000 * 1, RAM + 0x40000 + 0x20000 * bank2, 0x20000);
}

static ADDRESS_MAP_START( hotdogst_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM					// ROM
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank2")	// ROM (Banked)
	AM_RANGE(0xe000, 0xffff) AM_RAM					// RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( hotdogst_sound_portmap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(hotdogst_rombank_w)					// ROM bank
	AM_RANGE(0x30, 0x30) AM_READ(soundlatch_lo_r)						// From Main CPU
	AM_RANGE(0x40, 0x40) AM_READ(soundlatch_hi_r)						//
	AM_RANGE(0x50, 0x51) AM_DEVREADWRITE("ymsnd", ym2203_r, ym2203_w)	//
	AM_RANGE(0x60, 0x60) AM_DEVREADWRITE("oki", okim6295_r, okim6295_w)	// M6295
	AM_RANGE(0x70, 0x70) AM_WRITE(hotdogst_okibank_w)					// Samples bank
ADDRESS_MAP_END


/***************************************************************************
                                Mazinger Z
***************************************************************************/

static WRITE8_HANDLER( mazinger_rombank_w )
{
	if (data & ~0x07)
		logerror("CPU #1 - PC %04X: Bank %02X\n", cpu_get_pc(space->cpu), data);

	memory_set_bank(space->machine, "bank2", data & 0x07);
}

static ADDRESS_MAP_START( mazinger_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM					// ROM
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank2")	// ROM (Banked)
	AM_RANGE(0xc000, 0xc7ff) AM_RAM					// RAM
	AM_RANGE(0xf800, 0xffff) AM_RAM					// RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( mazinger_sound_portmap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(mazinger_rombank_w)	// ROM bank
	AM_RANGE(0x10, 0x10) AM_WRITE(soundlatch_ack_w)		// To Main CPU
	AM_RANGE(0x30, 0x30) AM_READ(soundlatch_lo_r)		// From Main CPU
	AM_RANGE(0x50, 0x51) AM_DEVWRITE("ymsnd", ym2203_w)	// YM2203
	AM_RANGE(0x52, 0x53) AM_DEVREAD("ymsnd", ym2203_r)	// YM2203
	AM_RANGE(0x70, 0x70) AM_DEVWRITE("oki", okim6295_w)	// M6295
	AM_RANGE(0x74, 0x74) AM_WRITE(hotdogst_okibank_w)	// Samples bank
ADDRESS_MAP_END


/***************************************************************************
                                Metamoqester
***************************************************************************/

static WRITE8_HANDLER( metmqstr_rombank_w )
{
	if (data & ~0x0f)
		logerror("CPU #1 - PC %04X: Bank %02X\n", cpu_get_pc(space->cpu), data);

	memory_set_bank(space->machine, "bank1", data & 0x0f);
}

static WRITE8_HANDLER( metmqstr_okibank0_w )
{
	UINT8 *ROM = memory_region(space->machine, "oki1");
	int bank1 = (data >> 0) & 0x7;
	int bank2 = (data >> 4) & 0x7;
	memcpy(ROM + 0x20000 * 0, ROM + 0x40000 + 0x20000 * bank1, 0x20000);
	memcpy(ROM + 0x20000 * 1, ROM + 0x40000 + 0x20000 * bank2, 0x20000);
}

static WRITE8_HANDLER( metmqstr_okibank1_w )
{
	UINT8 *ROM = memory_region(space->machine, "oki2");
	int bank1 = (data >> 0) & 0x7;
	int bank2 = (data >> 4) & 0x7;
	memcpy(ROM + 0x20000 * 0, ROM + 0x40000 + 0x20000 * bank1, 0x20000);
	memcpy(ROM + 0x20000 * 1, ROM + 0x40000 + 0x20000 * bank2, 0x20000);
}

static ADDRESS_MAP_START( metmqstr_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM					// ROM
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank1")	// ROM (Banked)
	AM_RANGE(0xe000, 0xffff) AM_RAM					// RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( metmqstr_sound_portmap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(metmqstr_rombank_w)					// Rom Bank
	AM_RANGE(0x20, 0x20) AM_READ(soundflags_r)							// Communication
	AM_RANGE(0x30, 0x30) AM_READ(soundlatch_lo_r)						// From Main CPU
	AM_RANGE(0x40, 0x40) AM_READ(soundlatch_hi_r)						//
	AM_RANGE(0x50, 0x51) AM_DEVREADWRITE("ymsnd", ym2151_r, ym2151_w)	// YM2151
	AM_RANGE(0x60, 0x60) AM_DEVWRITE("oki1", okim6295_w)				// M6295 #0
	AM_RANGE(0x70, 0x70) AM_WRITE(metmqstr_okibank0_w)					// Samples Bank #0
	AM_RANGE(0x80, 0x80) AM_DEVWRITE("oki2", okim6295_w)				// M6295 #1
	AM_RANGE(0x90, 0x90) AM_WRITE(metmqstr_okibank1_w)					// Samples Bank #1
ADDRESS_MAP_END


/***************************************************************************
                                Power Instinct 2
***************************************************************************/

static WRITE8_HANDLER( pwrinst2_rombank_w )
{
	if (data & ~0x07)
		logerror("CPU #1 - PC %04X: Bank %02X\n", cpu_get_pc(space->cpu), data);

	memory_set_bank(space->machine, "bank1", data & 0x07);
}

static ADDRESS_MAP_START( pwrinst2_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM					// ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")	// ROM (Banked)
	AM_RANGE(0xe000, 0xffff) AM_RAM					// RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( pwrinst2_sound_portmap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_DEVREADWRITE("oki1", okim6295_r, okim6295_w)	// M6295
	AM_RANGE(0x08, 0x08) AM_DEVREADWRITE("oki2", okim6295_r, okim6295_w)	//
	AM_RANGE(0x10, 0x17) AM_DEVWRITE("nmk112", nmk112_okibank_w)			// Samples bank
	AM_RANGE(0x40, 0x41) AM_DEVREADWRITE("ymsnd", ym2203_r, ym2203_w)		//
	AM_RANGE(0x50, 0x50) AM_WRITE(soundlatch_ack_w)							// To Main CPU
//  AM_RANGE(0x51, 0x51) AM_WRITENOP                                         // ?? volume
	AM_RANGE(0x80, 0x80) AM_WRITE(pwrinst2_rombank_w)						// ROM bank
	AM_RANGE(0x60, 0x60) AM_READ(soundlatch_hi_r)							// From Main CPU
	AM_RANGE(0x70, 0x70) AM_READ(soundlatch_lo_r)							//
ADDRESS_MAP_END


/***************************************************************************
                                Sailor Moon
***************************************************************************/

static READ8_HANDLER( mirror_ram_r )
{
	cave_state *state = (cave_state *)space->machine->driver_data;
	return state->mirror_ram[offset];
}

static WRITE8_HANDLER( mirror_ram_w )
{
	cave_state *state = (cave_state *)space->machine->driver_data;
	state->mirror_ram[offset] = data;
}

static WRITE8_HANDLER( sailormn_rombank_w )
{
	if (data & ~0x1f)
		logerror("CPU #1 - PC %04X: Bank %02X\n", cpu_get_pc(space->cpu), data);

	memory_set_bank(space->machine, "bank1", data & 0x1f);
}

static WRITE8_HANDLER( sailormn_okibank0_w )
{
	UINT8 *RAM = memory_region(space->machine, "oki1");
	int bank1 = (data >> 0) & 0xf;
	int bank2 = (data >> 4) & 0xf;
	memcpy(RAM + 0x20000 * 0, RAM + 0x40000 + 0x20000 * bank1, 0x20000);
	memcpy(RAM + 0x20000 * 1, RAM + 0x40000 + 0x20000 * bank2, 0x20000);
}

static WRITE8_HANDLER( sailormn_okibank1_w )
{
	UINT8 *RAM = memory_region(space->machine, "oki2");
	int bank1 = (data >> 0) & 0xf;
	int bank2 = (data >> 4) & 0xf;
	memcpy(RAM + 0x20000 * 0, RAM + 0x40000 + 0x20000 * bank1, 0x20000);
	memcpy(RAM + 0x20000 * 1, RAM + 0x40000 + 0x20000 * bank2, 0x20000);
}

static ADDRESS_MAP_START( sailormn_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM										// ROM
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank1")						// ROM (Banked)
	AM_RANGE(0xc000, 0xdfff) AM_READWRITE(mirror_ram_r, mirror_ram_w) AM_BASE_MEMBER(cave_state, mirror_ram)	// RAM
	AM_RANGE(0xe000, 0xffff) AM_READWRITE(mirror_ram_r, mirror_ram_w)	// Mirrored RAM (agallet)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sailormn_sound_portmap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(sailormn_rombank_w)						// Rom Bank
	AM_RANGE(0x10, 0x10) AM_WRITE(soundlatch_ack_w)							// To Main CPU
	AM_RANGE(0x20, 0x20) AM_READ(soundflags_r)								// Communication
	AM_RANGE(0x30, 0x30) AM_READ(soundlatch_lo_r)							// From Main CPU
	AM_RANGE(0x40, 0x40) AM_READ(soundlatch_hi_r)							//
	AM_RANGE(0x50, 0x51) AM_DEVREADWRITE("ymsnd", ym2151_r, ym2151_w)		// YM2151
	AM_RANGE(0x60, 0x60) AM_DEVREADWRITE("oki1", okim6295_r, okim6295_w)	// M6295 #0
	AM_RANGE(0x70, 0x70) AM_WRITE(sailormn_okibank0_w)						// Samples Bank #0
	AM_RANGE(0x80, 0x80) AM_DEVREADWRITE("oki2", okim6295_r, okim6295_w)	// M6295 #1
	AM_RANGE(0xc0, 0xc0) AM_WRITE(sailormn_okibank1_w)						// Samples Bank #1
ADDRESS_MAP_END



/***************************************************************************


                                Input Ports


***************************************************************************/

/*
    dfeveron config menu:
    101624.w -> 8,a6    preferences
    101626.w -> c,a6    (1:coin<<4|credit) <<8 | (2:coin<<4|credit)
*/

/* Most games use this */
static INPUT_PORTS_START( cave )
	PORT_START("IN0")	// Player 1
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(6)
	PORT_SERVICE_NO_TOGGLE( 0x0200, IP_ACTIVE_LOW )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )	// sw? exit service mode
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )	// sw? enter & exit service mode
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")	// Player 2
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(6)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE("eeprom", eeprom_read_bit)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

/* Gaia Crusaders, no EEPROM. Has DIPS */
static INPUT_PORTS_START( gaia )
	PORT_INCLUDE( cave )

	PORT_MODIFY("IN0")	// Player 1 + 2
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)

	PORT_MODIFY("IN1")	// Coins
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(6)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(6)
	PORT_SERVICE_NO_TOGGLE( 0x0004, IP_ACTIVE_LOW )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0000, DEF_STR( Language ) )			PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( English ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Japanese ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coinage ) )			PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(      0x0008, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0000, "2 Coins/1 Credit (1 to continue)" )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Free_Play ) )		PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Allow_Continue ) )	PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) )			PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0100, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0200, "4" )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Bonus_Life ) )		PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0400, "150k/300k" ) PORT_CONDITION("DSW", 0xe000, PORTCOND_EQUALS, 0xc000)
	PORT_DIPSETTING(      0x0400, "150k/350k" ) PORT_CONDITION("DSW", 0xe000, PORTCOND_EQUALS, 0xa000)
	PORT_DIPSETTING(      0x0400, "150k/350k" ) PORT_CONDITION("DSW", 0xe000, PORTCOND_EQUALS, 0xe000)
	PORT_DIPSETTING(      0x0400, "150k/400k" ) PORT_CONDITION("DSW", 0xe000, PORTCOND_EQUALS, 0x6000)
	PORT_DIPSETTING(      0x0400, "150k/400k" ) PORT_CONDITION("DSW", 0xe000, PORTCOND_EQUALS, 0x8000)
	PORT_DIPSETTING(      0x0400, "150k/400k" ) PORT_CONDITION("DSW", 0xe000, PORTCOND_EQUALS, 0x2000)
	PORT_DIPSETTING(      0x0400, "200k/500k" ) PORT_CONDITION("DSW", 0xe000, PORTCOND_EQUALS, 0x4000)
	PORT_DIPSETTING(      0x0400, "200k/500k" ) PORT_CONDITION("DSW", 0xe000, PORTCOND_EQUALS, 0x0000)
	PORT_DIPNAME( 0x1800, 0x1800, "Damage" )					PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(      0x1800, "+0" )
	PORT_DIPSETTING(      0x1000, "+1" )
	PORT_DIPSETTING(      0x0800, "+2" )
	PORT_DIPSETTING(      0x0000, "+3" )
	PORT_DIPNAME( 0xe000, 0xe000, DEF_STR( Difficulty ) )		PORT_DIPLOCATION("SW2:6,7,8")
	PORT_DIPSETTING(      0xc000, DEF_STR( Very_Easy ) )
	PORT_DIPSETTING(      0xa000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0xe000, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( Medium_Hard ) )
	PORT_DIPSETTING(      0x8000, "Hard 1" )
	PORT_DIPSETTING(      0x2000, "Hard 2" )
	PORT_DIPSETTING(      0x4000, DEF_STR( Very_Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
INPUT_PORTS_END

static INPUT_PORTS_START( theroes )
	PORT_INCLUDE( gaia )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x0004, 0x0000, DEF_STR( Language ) )			PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( English ) )
	PORT_DIPSETTING(      0x0004, "Chinese" )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Bonus_Life ) )		PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0400, "150k/300k" ) PORT_CONDITION("DSW", 0xc000, PORTCOND_EQUALS, 0x8000)
	PORT_DIPSETTING(      0x0400, "150k/350k" ) PORT_CONDITION("DSW", 0xc000, PORTCOND_EQUALS, 0xc000)
	PORT_DIPSETTING(      0x0400, "150k/400k" ) PORT_CONDITION("DSW", 0xc000, PORTCOND_EQUALS, 0x4000)
	PORT_DIPSETTING(      0x0400, "200k/500k" ) PORT_CONDITION("DSW", 0xc000, PORTCOND_EQUALS, 0x0000)
	PORT_DIPUNKNOWN_DIPLOC( 0x2000, 0x2000, "SW2:6" )
	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Difficulty ) )		PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Very_Easy ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Medium_Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
INPUT_PORTS_END


/* Normal layout but with 4 buttons */
static INPUT_PORTS_START( metmqstr )
	PORT_INCLUDE( cave )

	PORT_MODIFY("IN0")	// Player 1
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)

	PORT_MODIFY("IN1")	// Player 2
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
INPUT_PORTS_END

/* Different layout */
static INPUT_PORTS_START( guwange )
	PORT_START("IN0")	// Player 1 & 2
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_START("IN1")	// Coins
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(6)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(6)
	PORT_SERVICE_NO_TOGGLE( 0x0004, IP_ACTIVE_LOW )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE("eeprom", eeprom_read_bit)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( korokoro )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(10)	// bit 0x0010 of leds (coin)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(10)	// bit 0x0020 of leds (does coin sound)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(10)	// bit 0x0080 of leds
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON1 )	// round  button (choose)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 )	// square button (select in service mode / medal out in game)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_SERVICE2 )	// service medal out?
	PORT_SERVICE( 0x2000, IP_ACTIVE_LOW )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_SERVICE1 )	// service coin
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_SPECIAL )	PORT_CUSTOM(korokoro_hopper_r, NULL) // motor / hopper status ???

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE("eeprom", eeprom_read_bit)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( tjumpman )
	PORT_START("IN0")
	PORT_SERVICE_NO_TOGGLE( 0x01, IP_ACTIVE_LOW )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE("eeprom", eeprom_read_bit)
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_OTHER ) PORT_NAME( DEF_STR( Yes ) ) PORT_CODE(KEYCODE_Y)	// suru ("do")
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_NAME( "1 Bet" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_SPECIAL ) PORT_CUSTOM(tjumpman_hopper_r, NULL)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_CONFNAME( 0x08, 0x08, "Self Test" )
	PORT_CONFSETTING(    0x08, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2  ) PORT_NAME( DEF_STR( No ) ) PORT_CODE(KEYCODE_N)	// shinai ("not")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME( "Go" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1   )													// medal
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME( "3 Bet" )
INPUT_PORTS_END


/***************************************************************************


                            Graphics Layouts


***************************************************************************/

/* 8x8x4 tiles */
static const gfx_layout layout_8x8x4 =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{STEP4(0,1)},
	{STEP8(0,4)},
	{STEP8(0,4*8)},
	8*8*4
};

/* 8x8x6 tiles (in a 8x8x8 layout) */
static const gfx_layout layout_8x8x6 =
{
	8,8,
	RGN_FRAC(1,1),
	6,
	{8,9, 0,1,2,3},
	{0*4,1*4,4*4,5*4,8*4,9*4,12*4,13*4},
	{0*64,1*64,2*64,3*64,4*64,5*64,6*64,7*64},
	8*8*8
};

/* 8x8x6 tiles (4 bits in one rom, 2 bits in the other,
   unpacked in 2 pages of 4 bits) */
static const gfx_layout layout_8x8x6_2 =
{
	8,8,
	RGN_FRAC(1,2),
	6,
	{RGN_FRAC(1,2)+2,RGN_FRAC(1,2)+3, STEP4(0,1)},
	{STEP8(0,4)},
	{STEP8(0,4*8)},
	8*8*4
};

/* 8x8x8 tiles */
static const gfx_layout layout_8x8x8 =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{8,9,10,11, 0,1,2,3},
	{0*4,1*4,4*4,5*4,8*4,9*4,12*4,13*4},
	{0*64,1*64,2*64,3*64,4*64,5*64,6*64,7*64},
	8*8*8
};

/* 8x8x8 tiles, split in two roms (low / high nibble) */
static const gfx_layout layout_8x8x8_split =
{
	8,8,
	RGN_FRAC(1,2),
	8,
	{STEP4(RGN_FRAC(1,2),1), STEP4(0,1)},
	{STEP8(0,4)},
	{STEP8(0,4*8)},
	8*8*4
};

#if 0
/* 16x16x8 Zooming Sprites - No need to decode them */
static const gfx_layout layout_sprites =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{STEP8(0,1)},
	{STEP16(0,8)},
	{STEP16(0,16*8)},
	16*16*8
};
#endif

/***************************************************************************
                                Dangun Feveron
***************************************************************************/

static GFXDECODE_START( dfeveron )
	/* There are only $800 colors here, the first half for sprites
       the second half for tiles. We use $8000 virtual colors instead
       for consistency with games having $8000 real colors.
       A PALETTE_INIT function is thus needed for sprites */

//  "sprites"
	GFXDECODE_ENTRY( "layer0", 0, layout_8x8x4,	0x4400, 0x40 ) // [0] Layer 0
	GFXDECODE_ENTRY( "layer1", 0, layout_8x8x4,	0x4400, 0x40 ) // [1] Layer 1
GFXDECODE_END

/***************************************************************************
                                Dodonpachi
***************************************************************************/

static GFXDECODE_START( ddonpach )
	/* Layers 01 are 4 bit deep and use the first 16 of every 256
       colors for any given color code (a PALETTE_INIT function
       is provided for these layers, filling the 8000-83ff entries
       in the color table). Layer 2 uses the whole 256 for any given
       color code and the 4000-7fff range in the color table.   */

//  "sprites"
	GFXDECODE_ENTRY( "layer0", 0, layout_8x8x4,	0x8000, 0x40 ) // [0] Layer 0
	GFXDECODE_ENTRY( "layer1", 0, layout_8x8x4,	0x8000, 0x40 ) // [1] Layer 1
	GFXDECODE_ENTRY( "layer2", 0, layout_8x8x8,	0x4000, 0x40 ) // [2] Layer 2
GFXDECODE_END

/***************************************************************************
                                Donpachi
***************************************************************************/

static GFXDECODE_START( donpachi )
	/* There are only $800 colors here, the first half for sprites
       the second half for tiles. We use $8000 virtual colors instead
       for consistency with games having $8000 real colors.
       A PALETTE_INIT function is thus needed for sprites */

//  "sprites"
	GFXDECODE_ENTRY( "layer0", 0, layout_8x8x4,	0x4400, 0x40 ) // [0] Layer 0
	GFXDECODE_ENTRY( "layer1", 0, layout_8x8x4,	0x4400, 0x40 ) // [1] Layer 1
	GFXDECODE_ENTRY( "layer2", 0, layout_8x8x4,	0x4400, 0x40 ) // [2] Layer 2
GFXDECODE_END

/***************************************************************************
                                Esprade
***************************************************************************/

static GFXDECODE_START( esprade )
//  "sprites"
	GFXDECODE_ENTRY( "layer0", 0, layout_8x8x8,	0x4000, 0x40 ) // [0] Layer 0
	GFXDECODE_ENTRY( "layer1", 0, layout_8x8x8,	0x4000, 0x40 ) // [1] Layer 1
	GFXDECODE_ENTRY( "layer2", 0, layout_8x8x8,	0x4000, 0x40 ) // [2] Layer 2
GFXDECODE_END

/***************************************************************************
                                Hotdog Storm
***************************************************************************/

static GFXDECODE_START( hotdogst )
	/* There are only $800 colors here, the first half for sprites
       the second half for tiles. We use $8000 virtual colors instead
       for consistency with games having $8000 real colors.
       A PALETTE_INIT function is needed for sprites */

//  "sprites"
	GFXDECODE_ENTRY( "layer0", 0, layout_8x8x4,	0x4000, 0x40 ) // [0] Layer 0
	GFXDECODE_ENTRY( "layer1", 0, layout_8x8x4,	0x4000, 0x40 ) // [1] Layer 1
	GFXDECODE_ENTRY( "layer2", 0, layout_8x8x4,	0x4000, 0x40 ) // [2] Layer 2
GFXDECODE_END

/***************************************************************************
                                Koro Koro Quest
***************************************************************************/

static GFXDECODE_START( korokoro )
//  "sprites"
	GFXDECODE_ENTRY( "layer0", 0, layout_8x8x4,	0x4400, 0x40 ) // [0] Layer 0
GFXDECODE_END

/***************************************************************************
                                Mazinger Z
***************************************************************************/

static GFXDECODE_START( mazinger )
	/*  Sprites are 4 bit deep.
        Layer 0 is 4 bit deep.
        Layer 1 uses 64 color palettes, but the game only fills the
        first 16 colors of each palette, Indeed, the gfx data in ROM
        is empty in the top 4 bits. Additionally even if there are
        $40 color codes, only $400 colors are addressable.
        A PALETTE_INIT function is thus needed for sprites and layer 0.   */

//  "sprites"
	GFXDECODE_ENTRY( "layer0", 0, layout_8x8x4,	0x4000, 0x40 ) // [0] Layer 0
	GFXDECODE_ENTRY( "layer1", 0, layout_8x8x6,	0x4400, 0x40 ) // [1] Layer 1
GFXDECODE_END


/***************************************************************************
                                Power Instinct 2
***************************************************************************/

static GFXDECODE_START( pwrinst2 )
//  "sprites"
	GFXDECODE_ENTRY( "layer0", 0, layout_8x8x4,	0x0800+0x8000, 0x40 ) // [0] Layer 0
	GFXDECODE_ENTRY( "layer1", 0, layout_8x8x4,	0x1000+0x8000, 0x40 ) // [1] Layer 1
	GFXDECODE_ENTRY( "layer2", 0, layout_8x8x4,	0x1800+0x8000, 0x40 ) // [2] Layer 2
	GFXDECODE_ENTRY( "layer3", 0, layout_8x8x4,	0x2000+0x8000, 0x40 ) // [3] Layer 3
GFXDECODE_END


/***************************************************************************
                                Sailor Moon
***************************************************************************/

static GFXDECODE_START( sailormn )
	/* 4 bit sprites ? */
//  "sprites"
	GFXDECODE_ENTRY( "layer0", 0, layout_8x8x4,	0x4400, 0x40 ) // [0] Layer 0
	GFXDECODE_ENTRY( "layer1", 0, layout_8x8x4,	0x4800, 0x40 ) // [1] Layer 1
	GFXDECODE_ENTRY( "layer2", 0, layout_8x8x6_2,	0x4c00, 0x40 ) // [2] Layer 2
GFXDECODE_END


/***************************************************************************
                            Tobikose! Jumpman
***************************************************************************/

static GFXDECODE_START( tjumpman )
//  "sprites"
	GFXDECODE_ENTRY( "layer0", 0, layout_8x8x8_split,	0x4000, 0x40 ) // [0] Layer 0
GFXDECODE_END


/***************************************************************************
                                Uo Poko
***************************************************************************/

static GFXDECODE_START( uopoko )
//  "sprites"
	GFXDECODE_ENTRY( "layer0", 0, layout_8x8x8,	0x4000, 0x40 ) // [0] Layer 0
GFXDECODE_END


/***************************************************************************


                                Machine Drivers


***************************************************************************/

static MACHINE_START( cave )
{
	cave_state *state = (cave_state *)machine->driver_data;

	state->maincpu = machine->device("maincpu");
	state->audiocpu = machine->device("audiocpu");

	state_save_register_global(machine, state->soundbuf_len);
	state_save_register_global_array(machine, state->soundbuf_data);

	state_save_register_global(machine, state->vblank_irq);
	state_save_register_global(machine, state->sound_irq);
	state_save_register_global(machine, state->unknown_irq);
	state_save_register_global(machine, state->agallet_vblank_irq);
}

static MACHINE_RESET( cave )
{
	cave_state *state = (cave_state *)machine->driver_data;

	memset(state->soundbuf_data, 0, 32);
	state->soundbuf_len = 0;

	state->vblank_irq = 0;
	state->sound_irq = 0;
	state->unknown_irq = 0;
	state->agallet_vblank_irq = 0;
}

static const ymz280b_interface ymz280b_intf =
{
	sound_irq_gen
};

static void irqhandler(running_device *device, int irq)
{
	cputag_set_input_line(device->machine, "audiocpu", 0, irq ? ASSERT_LINE : CLEAR_LINE);
}

static const ym2151_interface ym2151_config =
{
	irqhandler
};

static const ym2203_interface ym2203_config =
{
	{
		AY8910_LEGACY_OUTPUT,
		AY8910_DEFAULT_LOADS,
		DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL
	},
	irqhandler
};

/***************************************************************************
                                Dangun Feveron
***************************************************************************/

static MACHINE_DRIVER_START( dfeveron )

	/* driver data */
	MDRV_DRIVER_DATA(cave_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, XTAL_16MHz)
	MDRV_CPU_PROGRAM_MAP(dfeveron_map)
	MDRV_CPU_VBLANK_INT("screen", cave_interrupt)

	MDRV_MACHINE_START(cave)
	MDRV_MACHINE_RESET(cave)
	MDRV_EEPROM_93C46_ADD("eeprom")

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(15625/271.5)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(320, 240)
	MDRV_SCREEN_VISIBLE_AREA(0, 320-1, 0, 240-1)

	MDRV_GFXDECODE(dfeveron)
	MDRV_PALETTE_LENGTH(0x8000)	/* $8000 palette entries for consistency with the other games */
	MDRV_PALETTE_INIT(dfeveron)

	MDRV_VIDEO_START(cave_2_layers)
	MDRV_VIDEO_UPDATE(cave)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymz", YMZ280B, XTAL_16_9344MHz)
	MDRV_SOUND_CONFIG(ymz280b_intf)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END


/***************************************************************************
                                Dodonpachi
***************************************************************************/


static MACHINE_DRIVER_START( ddonpach )

	/* driver data */
	MDRV_DRIVER_DATA(cave_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, XTAL_16MHz)
	MDRV_CPU_PROGRAM_MAP(ddonpach_map)
	MDRV_CPU_VBLANK_INT("screen", cave_interrupt)

	MDRV_MACHINE_START(cave)
	MDRV_MACHINE_RESET(cave)
	MDRV_EEPROM_93C46_ADD("eeprom")

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(15625/271.5)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(320, 240)
	MDRV_SCREEN_VISIBLE_AREA(0, 320-1, 0, 240-1)

	MDRV_GFXDECODE(ddonpach)
	MDRV_PALETTE_LENGTH(0x8000 + 0x40*16)	// $400 extra entries for layers 1&2
	MDRV_PALETTE_INIT(ddonpach)

	MDRV_VIDEO_START(cave_3_layers)
	MDRV_VIDEO_UPDATE(cave)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymz", YMZ280B, XTAL_16_9344MHz)
	MDRV_SOUND_CONFIG(ymz280b_intf)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END


/***************************************************************************
                                    Donpachi
***************************************************************************/

static const nmk112_interface donpachi_nmk112_intf =
{
	"oki1", "oki2", 1 << 0	// chip #0 (music) is not paged
};

static MACHINE_DRIVER_START( donpachi )

	/* driver data */
	MDRV_DRIVER_DATA(cave_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, XTAL_16MHz)
	MDRV_CPU_PROGRAM_MAP(donpachi_map)
	MDRV_CPU_VBLANK_INT("screen", cave_interrupt)

	MDRV_MACHINE_START(cave)
	MDRV_MACHINE_RESET(cave)
	MDRV_EEPROM_93C46_ADD("eeprom")

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(15625/271.5)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(320, 240)
	MDRV_SCREEN_VISIBLE_AREA(0, 320-1, 0, 240-1)

	MDRV_GFXDECODE(donpachi)
	MDRV_PALETTE_LENGTH(0x8000)	/* $8000 palette entries for consistency with the other games */
	MDRV_PALETTE_INIT(dfeveron)

	MDRV_VIDEO_START(cave_3_layers)
	MDRV_VIDEO_UPDATE(cave)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_OKIM6295_ADD("oki1", XTAL_1_056MHz, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.60)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.60)

	MDRV_OKIM6295_ADD("oki2", 2112000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)

	MDRV_NMK112_ADD("nmk112", donpachi_nmk112_intf)
MACHINE_DRIVER_END


/***************************************************************************
                                Esprade
***************************************************************************/

static MACHINE_DRIVER_START( esprade )

	/* driver data */
	MDRV_DRIVER_DATA(cave_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, XTAL_16MHz)
	MDRV_CPU_PROGRAM_MAP(esprade_map)
	MDRV_CPU_VBLANK_INT("screen", cave_interrupt)

	MDRV_MACHINE_START(cave)
	MDRV_MACHINE_RESET(cave)
	MDRV_EEPROM_93C46_ADD("eeprom")

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(15625/271.5)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(320, 240)
	MDRV_SCREEN_VISIBLE_AREA(0, 320-1, 0, 240-1)

	MDRV_GFXDECODE(esprade)
	MDRV_PALETTE_LENGTH(0x8000)
	MDRV_PALETTE_INIT(cave)

	MDRV_VIDEO_START(cave_3_layers)
	MDRV_VIDEO_UPDATE(cave)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymz", YMZ280B, XTAL_16_9344MHz)
	MDRV_SOUND_CONFIG(ymz280b_intf)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END


/***************************************************************************
                                    Gaia Crusaders
***************************************************************************/

static MACHINE_DRIVER_START( gaia )

	/* driver data */
	MDRV_DRIVER_DATA(cave_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, XTAL_16MHz)
	MDRV_CPU_PROGRAM_MAP(gaia_map)
	MDRV_CPU_VBLANK_INT("screen", cave_interrupt)

	MDRV_MACHINE_START(cave)
	MDRV_MACHINE_RESET(cave)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(15625/271.5)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(320, 240)
	MDRV_SCREEN_VISIBLE_AREA(0, 320-1, 0, 224-1)

	MDRV_GFXDECODE(esprade)
	MDRV_PALETTE_LENGTH(0x8000)
	MDRV_PALETTE_INIT(cave)

	MDRV_VIDEO_START(cave_3_layers)
	MDRV_VIDEO_UPDATE(cave)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymz", YMZ280B, XTAL_16_9344MHz)
	MDRV_SOUND_CONFIG(ymz280b_intf)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END


/***************************************************************************
                                    Guwange
***************************************************************************/

static MACHINE_DRIVER_START( guwange )

	/* driver data */
	MDRV_DRIVER_DATA(cave_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, XTAL_16MHz)
	MDRV_CPU_PROGRAM_MAP(guwange_map)
	MDRV_CPU_VBLANK_INT("screen", cave_interrupt)

	MDRV_MACHINE_START(cave)
	MDRV_MACHINE_RESET(cave)
	MDRV_EEPROM_93C46_ADD("eeprom")

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(15625/271.5)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(320, 240)
	MDRV_SCREEN_VISIBLE_AREA(0, 320-1, 0, 240-1)

	MDRV_GFXDECODE(esprade)
	MDRV_PALETTE_LENGTH(0x8000)
	MDRV_PALETTE_INIT(cave)

	MDRV_VIDEO_START(cave_3_layers)
	MDRV_VIDEO_UPDATE(cave)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymz", YMZ280B, XTAL_16_9344MHz)
	MDRV_SOUND_CONFIG(ymz280b_intf)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END

/***************************************************************************
                                Hotdog Storm
***************************************************************************/

static MACHINE_DRIVER_START( hotdogst )

	/* driver data */
	MDRV_DRIVER_DATA(cave_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, XTAL_16MHz)
	MDRV_CPU_PROGRAM_MAP(hotdogst_map)
	MDRV_CPU_VBLANK_INT("screen", cave_interrupt)

	MDRV_CPU_ADD("audiocpu", Z80, XTAL_4MHz)
	MDRV_CPU_PROGRAM_MAP(hotdogst_sound_map)
	MDRV_CPU_IO_MAP(hotdogst_sound_portmap)

	MDRV_MACHINE_START(cave)
	MDRV_MACHINE_RESET(cave)
	MDRV_EEPROM_93C46_ADD("eeprom")

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(15625/271.5)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(384, 240)
	MDRV_SCREEN_VISIBLE_AREA(0, 384-1, 0, 240-1)

	MDRV_GFXDECODE(hotdogst)
	MDRV_PALETTE_LENGTH(0x8000)	/* $8000 palette entries for consistency with the other games */
	MDRV_PALETTE_INIT(dfeveron)

	MDRV_VIDEO_START(cave_3_layers)
	MDRV_VIDEO_UPDATE(cave)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2203, XTAL_4MHz)
	MDRV_SOUND_CONFIG(ym2203_config)
	MDRV_SOUND_ROUTE(0, "lspeaker",  0.20)
	MDRV_SOUND_ROUTE(0, "rspeaker", 0.20)
	MDRV_SOUND_ROUTE(1, "lspeaker",  0.20)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.20)
	MDRV_SOUND_ROUTE(2, "lspeaker",  0.20)
	MDRV_SOUND_ROUTE(2, "rspeaker", 0.20)
	MDRV_SOUND_ROUTE(3, "lspeaker",  0.80)
	MDRV_SOUND_ROUTE(3, "rspeaker", 0.80)

	MDRV_OKIM6295_ADD("oki", XTAL_1_056MHz, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
MACHINE_DRIVER_END


/***************************************************************************
                               Koro Koro Quest
***************************************************************************/

static MACHINE_DRIVER_START( korokoro )

	/* driver data */
	MDRV_DRIVER_DATA(cave_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, XTAL_16MHz)
	MDRV_CPU_PROGRAM_MAP(korokoro_map)
	MDRV_CPU_VBLANK_INT("screen", cave_interrupt)

	MDRV_MACHINE_START(cave)
	MDRV_MACHINE_RESET(cave)
	MDRV_EEPROM_ADD("eeprom", eeprom_interface_93C46_8bit)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(15625/271.5)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(320, 240)
	MDRV_SCREEN_VISIBLE_AREA(0, 320-1-2, 0, 240-1-1)

	MDRV_GFXDECODE(korokoro)
	MDRV_PALETTE_LENGTH(0x8000)	/* $8000 palette entries for consistency with the other games */
	MDRV_PALETTE_INIT(korokoro)

	MDRV_VIDEO_START(cave_1_layer)
	MDRV_VIDEO_UPDATE(cave)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymz", YMZ280B, XTAL_16_9344MHz)
	MDRV_SOUND_CONFIG(ymz280b_intf)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( crusherm )

	/* driver data */
	MDRV_IMPORT_FROM( korokoro )

	/* basic machine hardware */
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(crusherm_map)
MACHINE_DRIVER_END


/***************************************************************************
                                Mazinger Z
***************************************************************************/

static MACHINE_DRIVER_START( mazinger )

	/* driver data */
	MDRV_DRIVER_DATA(cave_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, XTAL_16MHz)
	MDRV_CPU_PROGRAM_MAP(mazinger_map)
	MDRV_CPU_VBLANK_INT("screen", cave_interrupt)

	MDRV_CPU_ADD("audiocpu", Z80, XTAL_4MHz) // Bidirectional communication
	MDRV_CPU_PROGRAM_MAP(mazinger_sound_map)
	MDRV_CPU_IO_MAP(mazinger_sound_portmap)

	MDRV_WATCHDOG_TIME_INIT(SEC(3))	/* a guess, and certainly wrong */

	MDRV_MACHINE_START(cave)
	MDRV_MACHINE_RESET(cave)
	MDRV_EEPROM_93C46_ADD("eeprom")

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(15625/271.5)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(384, 240)
	MDRV_SCREEN_VISIBLE_AREA(0, 384-1, 0, 240-1)

	MDRV_GFXDECODE(mazinger)
	MDRV_PALETTE_LENGTH(0x8000)	/* $8000 palette entries for consistency with the other games */
	MDRV_PALETTE_INIT(mazinger)

	MDRV_VIDEO_START(cave_2_layers)
	MDRV_VIDEO_UPDATE(cave)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2203, XTAL_4MHz)
	MDRV_SOUND_CONFIG(ym2203_config)
	MDRV_SOUND_ROUTE(0, "lspeaker",  0.20)
	MDRV_SOUND_ROUTE(0, "rspeaker", 0.20)
	MDRV_SOUND_ROUTE(1, "lspeaker",  0.20)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.20)
	MDRV_SOUND_ROUTE(2, "lspeaker",  0.20)
	MDRV_SOUND_ROUTE(2, "rspeaker", 0.20)
	MDRV_SOUND_ROUTE(3, "lspeaker",  0.60)
	MDRV_SOUND_ROUTE(3, "rspeaker", 0.60)

	MDRV_OKIM6295_ADD("oki", XTAL_1_056MHz, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 2.0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 2.0)
MACHINE_DRIVER_END


/***************************************************************************
                                Metamoqester
***************************************************************************/

static MACHINE_DRIVER_START( metmqstr )

	/* driver data */
	MDRV_DRIVER_DATA(cave_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, XTAL_32MHz / 2)
	MDRV_CPU_PROGRAM_MAP(metmqstr_map)
	MDRV_CPU_VBLANK_INT("screen", cave_interrupt)

	MDRV_CPU_ADD("audiocpu", Z80, XTAL_32MHz / 4)
	MDRV_CPU_PROGRAM_MAP(metmqstr_sound_map)
	MDRV_CPU_IO_MAP(metmqstr_sound_portmap)

	MDRV_WATCHDOG_TIME_INIT(SEC(3))	/* a guess, and certainly wrong */

	MDRV_MACHINE_START(cave)
	MDRV_MACHINE_RESET(cave)	/* start with the watchdog armed */
	MDRV_EEPROM_93C46_ADD("eeprom")

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(15625/271.5)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(0x200, 240)
	MDRV_SCREEN_VISIBLE_AREA(0x7d, 0x7d + 0x180-1, 0, 240-1)

	MDRV_GFXDECODE(donpachi)
	MDRV_PALETTE_LENGTH(0x8000)	/* $8000 palette entries for consistency with the other games */
	MDRV_PALETTE_INIT(dfeveron)

	MDRV_VIDEO_START(cave_3_layers)
	MDRV_VIDEO_UPDATE(cave)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2151, XTAL_16MHz / 4)
	MDRV_SOUND_CONFIG(ym2151_config)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.20)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.20)

	MDRV_OKIM6295_ADD("oki1", XTAL_32MHz / 16 , OKIM6295_PIN7_HIGH)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)

	MDRV_OKIM6295_ADD("oki2", XTAL_32MHz / 16 , OKIM6295_PIN7_HIGH)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
MACHINE_DRIVER_END


/***************************************************************************
                                Power Instinct 2
***************************************************************************/

/*  X1 = 12 MHz, X2 = 28 MHz, X3 = 16 MHz. OKI: / 165 mode A ; / 132 mode B */

static const nmk112_interface pwrinst2_nmk112_intf =
{
	"oki1", "oki2", 0
};

static MACHINE_DRIVER_START( pwrinst2 )

	/* driver data */
	MDRV_DRIVER_DATA(cave_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, XTAL_16MHz)	/* 16 MHz */
	MDRV_CPU_PROGRAM_MAP(pwrinst2_map)
	MDRV_CPU_VBLANK_INT("screen", cave_interrupt)

	MDRV_CPU_ADD("audiocpu", Z80,XTAL_16MHz / 2)	/* 8 MHz */
	MDRV_CPU_PROGRAM_MAP(pwrinst2_sound_map)
	MDRV_CPU_IO_MAP(pwrinst2_sound_portmap)

	MDRV_MACHINE_START(cave)
	MDRV_MACHINE_RESET(cave)
	MDRV_EEPROM_93C46_ADD("eeprom")

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(15625/271.5)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(0x200, 240)
	MDRV_SCREEN_VISIBLE_AREA(0x70, 0x70 + 0x140-1, 0, 240-1)

	MDRV_GFXDECODE(pwrinst2)
	MDRV_PALETTE_LENGTH(0x8000+0x2800)
	MDRV_PALETTE_INIT(pwrinst2)

	MDRV_VIDEO_START(cave_4_layers)
	MDRV_VIDEO_UPDATE(cave)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2203, XTAL_16MHz / 4)
	MDRV_SOUND_CONFIG(ym2203_config)
	MDRV_SOUND_ROUTE(0, "lspeaker",  0.40)
	MDRV_SOUND_ROUTE(0, "rspeaker", 0.40)
	MDRV_SOUND_ROUTE(1, "lspeaker",  0.40)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.40)
	MDRV_SOUND_ROUTE(2, "lspeaker",  0.40)
	MDRV_SOUND_ROUTE(2, "rspeaker", 0.40)
	MDRV_SOUND_ROUTE(3, "lspeaker",  0.80)
	MDRV_SOUND_ROUTE(3, "rspeaker", 0.80)

	MDRV_OKIM6295_ADD("oki1", XTAL_3MHz , OKIM6295_PIN7_LOW)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.80)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.80)

	MDRV_OKIM6295_ADD("oki2", XTAL_3MHz , OKIM6295_PIN7_LOW)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.00)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.00)

	MDRV_NMK112_ADD("nmk112", pwrinst2_nmk112_intf)
MACHINE_DRIVER_END


/***************************************************************************
                        Sailor Moon / Air Gallet
***************************************************************************/

static MACHINE_DRIVER_START( sailormn )

	/* driver data */
	MDRV_DRIVER_DATA(cave_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, XTAL_16MHz)
	MDRV_CPU_PROGRAM_MAP(sailormn_map)
	MDRV_CPU_VBLANK_INT("screen", cave_interrupt)

	MDRV_CPU_ADD("audiocpu", Z80, XTAL_8MHz) // Bidirectional Communication
	MDRV_CPU_PROGRAM_MAP(sailormn_sound_map)
	MDRV_CPU_IO_MAP(sailormn_sound_portmap)

//  MDRV_QUANTUM_TIME(HZ(600))

	MDRV_MACHINE_START(cave)
	MDRV_MACHINE_RESET(cave)
	MDRV_EEPROM_93C46_ADD("eeprom")

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(15625/271.5)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(320+1, 240)
	MDRV_SCREEN_VISIBLE_AREA(0+1, 320+1-1, 0, 240-1)

	MDRV_GFXDECODE(sailormn)
	MDRV_PALETTE_LENGTH(0x8000)	/* $8000 palette entries for consistency with the other games */
	MDRV_PALETTE_INIT(sailormn)	// 4 bit sprites, 6 bit tiles

	MDRV_VIDEO_START(sailormn_3_layers)	/* Layer 2 has 1 banked ROM */
	MDRV_VIDEO_UPDATE(cave)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MDRV_SOUND_ADD("ymsnd", YM2151, XTAL_16MHz/4)
	MDRV_SOUND_CONFIG(ym2151_config)
	MDRV_SOUND_ROUTE(0, "lspeaker", 0.30)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.30)

	MDRV_OKIM6295_ADD("oki1", 2112000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)

	MDRV_OKIM6295_ADD("oki2", 2112000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
MACHINE_DRIVER_END


/***************************************************************************
                            Tobikose! Jumpman
***************************************************************************/

static MACHINE_DRIVER_START( tjumpman )

	/* driver data */
	MDRV_DRIVER_DATA(cave_state)

	MDRV_NVRAM_HANDLER(generic_0fill)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, XTAL_28MHz / 2)
	MDRV_CPU_PROGRAM_MAP(tjumpman_map)
	MDRV_CPU_VBLANK_INT("screen", cave_interrupt)

	MDRV_WATCHDOG_TIME_INIT(SEC(3))	/* a guess, and certainly wrong */

	MDRV_MACHINE_START(cave)
	MDRV_MACHINE_RESET(cave)
	MDRV_EEPROM_93C46_ADD("eeprom")

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(15625/271.5)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(0x200, 240)
	MDRV_SCREEN_VISIBLE_AREA(0x80, 0x80 + 0x140-1, 0, 240-1)

	MDRV_GFXDECODE(tjumpman)
	MDRV_PALETTE_LENGTH(0x8000)
	MDRV_PALETTE_INIT(cave)

	MDRV_VIDEO_START(cave_1_layer)
	MDRV_VIDEO_UPDATE(cave)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_OKIM6295_ADD("oki1", XTAL_28MHz / 28, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)

	// oki2 spot is unpopulated
MACHINE_DRIVER_END


/***************************************************************************
                                Uo Poko
***************************************************************************/

static MACHINE_DRIVER_START( uopoko )

	/* driver data */
	MDRV_DRIVER_DATA(cave_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, XTAL_16MHz)
	MDRV_CPU_PROGRAM_MAP(uopoko_map)
	MDRV_CPU_VBLANK_INT("screen", cave_interrupt)

	MDRV_MACHINE_START(cave)
	MDRV_EEPROM_93C46_ADD("eeprom")

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(15625/271.5)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(320, 240)
	MDRV_SCREEN_VISIBLE_AREA(0, 320-1, 0, 240-1)

	MDRV_GFXDECODE(uopoko)
	MDRV_PALETTE_LENGTH(0x8000)

	MDRV_PALETTE_INIT(cave)
	MDRV_VIDEO_START(cave_1_layer)
	MDRV_VIDEO_UPDATE(cave)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymz", YMZ280B, XTAL_16_9344MHz)
	MDRV_SOUND_CONFIG(ymz280b_intf)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END


/***************************************************************************


                                ROMs Loading


***************************************************************************/

/* 4 bits -> 8 bits. Even and odd pixels are swapped */
static void unpack_sprites(running_machine *machine)
{
	const UINT32 len	=	memory_region_length(machine, "sprites");
	UINT8 *rgn			=	memory_region       (machine, "sprites");
	UINT8 *src			=	rgn + len / 2 - 1;
	UINT8 *dst			=	rgn + len - 1;

	while(dst > src)
	{
		UINT8 data = *src--;
		/* swap even and odd pixels */
		*dst-- = data >> 4;		*dst-- = data & 0xF;
	}
}


/* 4 bits -> 8 bits. Even and odd pixels and even and odd words, are swapped */
static void ddonpach_unpack_sprites(running_machine *machine)
{
	const UINT32 len	=	memory_region_length(machine, "sprites");
	UINT8 *rgn			=	memory_region       (machine, "sprites");
	UINT8 *src			=	rgn + len / 2 - 1;
	UINT8 *dst			=	rgn + len - 1;

	while(dst > src)
	{
		UINT8 data1 = *src--;
		UINT8 data2 = *src--;
		UINT8 data3 = *src--;
		UINT8 data4 = *src--;

		/* swap even and odd pixels, and even and odd words */
		*dst-- = data2 & 0xF;		*dst-- = data2 >> 4;
		*dst-- = data1 & 0xF;		*dst-- = data1 >> 4;
		*dst-- = data4 & 0xF;		*dst-- = data4 >> 4;
		*dst-- = data3 & 0xF;		*dst-- = data3 >> 4;
	}
}


/* 2 pages of 4 bits -> 8 bits */
static void esprade_unpack_sprites(running_machine *machine)
{
	UINT8 *src		=	memory_region(machine, "sprites");
	UINT8 *dst		=	src + memory_region_length(machine, "sprites");

	while(src < dst)
	{
		UINT8 data1 = src[0];
		UINT8 data2 = src[1];

		src[0] = ((data1 & 0x0f)<<4) + (data2 & 0x0f);
		src[1] = (data1 & 0xf0) + ((data2 & 0xf0)>>4);

		src += 2;
	}
}

/***************************************************************************

                                Air Gallet

Banpresto
Runs on identical board to Sailor Moon (several sockets unpopulated)

PCB: BP945A (overstamped with BP962A)
CPU: TMP68HC000P16 (68000, 64 pin DIP)
SND: Z84C0008PEC (Z80, 40 pin DIP), OKI M6295 x 2, YM2151, YM3012
OSC: 28.000MHz, 16.000MHz
RAM: 62256 x 8, NEC 424260 x 2, 6264 x 5

Other Chips:
SGS Thomson ST93C46CB1 (EEPROM)
PALS (same as Sailor Moon, not dumped):
      18CV8 label SMBG
      18CV8 label SMZ80
      18CV8 label SMCPU
      GAL16V8 (located near BP962A.U47)

GFX:  038 9437WX711 (176 pin PQFP)
      038 9437WX711 (176 pin PQFP)
      038 9437WX711 (176 pin PQFP)
      013 9346E7002 (240 pin PQFP)

On PCB near JAMMA connector is a small push button to access test mode.

ROMS:
BP962A.U9   27C040      Sound Program
BP962A.U45  27C240      Main Program
BP962A.U47  23C16000    Sound
BP962A.U48  23C16000    Sound
BP962A.U53  23C16000    GFX
BP962A.U54  23C16000    GFX
BP962A.U57  23C16000    GFX
BP962A.U65  23C16000    GFX
BP962A.U76  23C16000    GFX
BP962A.U77  23C16000    GFX

***************************************************************************/



#define ROMS_AGALLET \
	ROM_REGION( 0x400000, "maincpu", 0 ) \
	ROM_LOAD16_WORD_SWAP( "bp962a.u45", 0x000000, 0x080000, CRC(24815046) SHA1(f5eeae60b923ae850b335e7898a2760407631d8b) ) \
	\
	ROM_REGION( 0x88000, "audiocpu", 0 ) \
	ROM_LOAD( "bp962a.u9",  0x00000, 0x08000, CRC(06caddbe) SHA1(6a3cc50558ba19a31b21b7f3ec6c6e2846244ff1) ) \
	ROM_CONTINUE(           0x10000, 0x78000             ) \
	\
	ROM_REGION( 0x400000 * 2, "sprites", 0 ) \
	ROM_LOAD( "bp962a.u76", 0x000000, 0x200000, CRC(858da439) SHA1(33a3d2a3ec3fa3364b00e1e43b405e5030a5b2a3) ) \
	ROM_LOAD( "bp962a.u77", 0x200000, 0x200000, CRC(ea2ba35e) SHA1(72487f21d44fe7be9a98068ce7f57a43c132945f) ) \
	\
	ROM_REGION( 0x100000, "layer0", 0 ) \
	ROM_LOAD( "bp962a.u53", 0x000000, 0x100000, CRC(fcd9a107) SHA1(169b94db8389e7d47d4d77f36907a62c30fea727) ) \
	ROM_CONTINUE(           0x000000, 0x100000             ) \
	\
	ROM_REGION( 0x200000, "layer1", 0 )	\
	ROM_LOAD( "bp962a.u54", 0x000000, 0x200000, CRC(0cfa3409) SHA1(17107e26762ef7e3b902fb29a6d7bc534a4d09aa) ) \
	\
	ROM_REGION( (1*0x200000)*2, "layer2", 0 )	\
	\
	ROM_LOAD( "bp962a.u57", 0x000000, 0x200000, CRC(6d608957) SHA1(15f6e8346f5f95eb229505b1b4666dabeb810ee8) ) \
	\
	ROM_LOAD( "bp962a.u65", 0x200000, 0x100000, CRC(135fcf9a) SHA1(2e8c89c2627bbdef160d96724d07883fb2fa1a57) ) \
	ROM_CONTINUE(           0x200000, 0x100000             ) \
	\
	ROM_REGION( 0x240000, "oki1", 0 ) \
	ROM_LOAD( "bp962a.u48", 0x040000, 0x200000, CRC(ae00a1ce) SHA1(5e8c74df0ac77efb3080406870856f958be14f79) ) \
	\
	ROM_REGION( 0x240000, "oki2", 0 )	\
	ROM_LOAD( "bp962a.u47", 0x040000, 0x200000, CRC(6d4e9737) SHA1(81c7ecdfc2d38d0b35e26745866f6672f566f936) ) \

/* the regions differ only in the EEPROM, hence the macro above - all EEPROMs are Factory Defaulted */
ROM_START( agallet )
	ROMS_AGALLET

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "agallet_europe.nv", 0x0000, 0x0080, CRC(ec38bf65) SHA1(cb8d9eacc0cf55a0c6b187e6673e3354554314b5) )
ROM_END

ROM_START( agalletu )
	ROMS_AGALLET

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "agallet_usa.nv", 0x0000, 0x0080, CRC(72e65056) SHA1(abf1a86df01064d9d5d8c418e8367817319ec335) )
ROM_END

ROM_START( agalletj )
	ROMS_AGALLET

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "agallet_japan.nv", 0x0000, 0x0080, CRC(0753f547) SHA1(aabb987470406b8729894108bc4d050f7200917d) )
ROM_END

ROM_START( agalletk )
	ROMS_AGALLET

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "agallet_korea.nv", 0x0000, 0x0080, CRC(7f41c253) SHA1(50793d4da0ad6eb590941d26a729a1cf4b3c25c2) )
ROM_END

ROM_START( agallett ) // the dumped board was this region
	ROMS_AGALLET

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "agallet_taiwan.nv", 0x0000, 0x0080, CRC(0af46742) SHA1(37b704c4c573b2aabd6f016e9e8dd458f95148f7) )
ROM_END

ROM_START( agalleth )
	ROMS_AGALLET

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "agallet_hongkong.nv", 0x0000, 0x0080, CRC(998d1a74) SHA1(13e7e27a18417949d49e97d521781fc0feeef792) )
ROM_END

/***************************************************************************

              Fever SOS (International) / Dangun Feveron (Japan)

Board:  CV01
OSC:    28.0, 16.0, 16.9 MHz

***************************************************************************/

ROM_START( dfeveron )
	ROM_REGION( 0x100000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "cv01-u34.bin", 0x000000, 0x080000, CRC(be87f19d) SHA1(595239245df3835cdf5a99a6c62480465558d8d3) )
	ROM_LOAD16_BYTE( "cv01-u33.bin", 0x000001, 0x080000, CRC(e53a7db3) SHA1(ddced29f78dc3cc89038757b6577ba2ba0d8b041) )

	ROM_REGION( 0x800000 * 2, "sprites", 0 )		/* Sprites: * 2 */
	ROM_LOAD( "cv01-u25.bin", 0x000000, 0x400000, CRC(a6f6a95d) SHA1(e1eb45cb5d0e6163edfd9d830633b913fb53c6ca) )
	ROM_LOAD( "cv01-u26.bin", 0x400000, 0x400000, CRC(32edb62a) SHA1(3def74e1316b80cc25a8c3ac162cd7bcb8cc807c) )

	ROM_REGION( 0x200000, "layer0", 0 )	/* Layer 0 */
	ROM_LOAD( "cv01-u50.bin", 0x000000, 0x200000, CRC(7a344417) SHA1(828bd8f95d2fcc34407e17629ccafc904a4ea12d) )

	ROM_REGION( 0x200000, "layer1", 0 )	/* Layer 1 */
	ROM_LOAD( "cv01-u49.bin", 0x000000, 0x200000, CRC(d21cdda7) SHA1(cace4650de580c3c4a037f1f5c32bfc1846b383c) )

	ROM_REGION( 0x400000, "ymz", 0 )	/* Samples */
	ROM_LOAD( "cv01-u19.bin", 0x000000, 0x400000, CRC(5f5514da) SHA1(53f27364aee544572a82649c9ff29bacc642b732) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-dfeveron.bin", 0x0000, 0x0080, CRC(c3174959) SHA1(29b5c94722756481e4f84bfd75dee15fdb5c8cf7) )
ROM_END

/*

Fever SOS

  The program code checks for 0x05 & 0x19 at the 17th & 18th byte in the EEPROM.  Therefore
  you cannot convert a Dangun Feveron over to a Fever SOS by changing the 2 program roms

Jumper JP1:
INT Version - 2 & 3
JAP Version - 1 & 2

However there are more differences:

U4:
INT Version - 013 9838EX003
JAP Version - 013 9807EX004 (The second set of numbers are manufacture day codes)

UA2 & UB2:
INT Version - 038 9838WX001
JAP Version - 038 9808WX003 (The second set of numbers are manufacture day codes)

TA8030S (Beside SW1)
INT Version  - NOT MOUNTED
JAP Version - TA8030S (WatchDog Timer, might be controlled by JP1)

U47 & U48 - Differ
U38 & U37 - Differ - These chips are Static RAM

It actually looks like the international version is older than
the Japanese version PCB wise, but the software date is 98/09/25
and mine is 98/09/17!

The famous full extent of the JAM is inside the image but so is
"full extent" of the LAW. There are also other version strings
inside the same image look here...

          NOTICE
  THIS GAME IS FOR USE IN
                KOREA ONLY
            HONG KONG ONLY
               TAIWAN ONLY
       SOUTHEAST ASIA ONLY
               EUROPE ONLY
                U.S.A ONLY
                JAPAN ONLY
SALES, EXPORT OR OPERATION
OUTSIDE THIS COUNTRY MAY BE
CONSTRUED AS COPYRIGHT AND
TRADEMARK INFRINGEMENT AND
IS STRICTLY PROHIBITED.
VIOLATOR AND SUBJECT TO
SEVERE PENALTIES AND WILL
BE PROSECUTED TO THE FULL
EXTENT OF THE JAM.
              98/09/10 VER.

Look at the version date!

          NOTICE
THIS GAME MAY NOT BE SOLD,
EXPORTED OR OPERATED
WITHOUTPROOF OF LEGAL CONSENT
BY CAVE CO.,LTD.
VIOLATION OF THESE TERMS WILL
RESULT IN COPYRIGHT AND
TRADEMARK INFRINGEMENT,AND IS
STRICTLY PROHIBITED.
VIOLATORS ARE SUBJECT TO
SEVERE PENALTIES AND WILL BE
PROSECUTED TO THE FULL EXTENT
OF THE LAW GOVERNED BY THE
COUNTRY OF ORIGIN.
                 98/09/25 VER

This is from Fever SOS image! Both version strings are present!

The PCB is also different, UD's PCB does not have the Cave logo and
the CV01 marker in the lower left corner of the PCB.

There is some "engrish" story inside the UD image but this is NOT
present in the japanese images...

*/

ROM_START( feversos )
	ROM_REGION( 0x100000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "cv01-u34.sos", 0x000000, 0x080000, CRC(24ef3ce6) SHA1(42799eebbb2686a837b8972aec684143deadca59) )
	ROM_LOAD16_BYTE( "cv01-u33.sos", 0x000001, 0x080000, CRC(64ff73fd) SHA1(7fc3a8469cec2361d373a4dac4a547c13ca5f709) )

	ROM_REGION( 0x800000 * 2, "sprites", 0 )		/* Sprites: * 2 */
	ROM_LOAD( "cv01-u25.bin", 0x000000, 0x400000, CRC(a6f6a95d) SHA1(e1eb45cb5d0e6163edfd9d830633b913fb53c6ca) )
	ROM_LOAD( "cv01-u26.bin", 0x400000, 0x400000, CRC(32edb62a) SHA1(3def74e1316b80cc25a8c3ac162cd7bcb8cc807c) )

	ROM_REGION( 0x200000, "layer0", 0 )	/* Layer 0 */
	ROM_LOAD( "cv01-u50.bin", 0x000000, 0x200000, CRC(7a344417) SHA1(828bd8f95d2fcc34407e17629ccafc904a4ea12d) )

	ROM_REGION( 0x200000, "layer1", 0 )	/* Layer 1 */
	ROM_LOAD( "cv01-u49.bin", 0x000000, 0x200000, CRC(d21cdda7) SHA1(cace4650de580c3c4a037f1f5c32bfc1846b383c) )

	ROM_REGION( 0x400000, "ymz", 0 )	/* Samples */
	ROM_LOAD( "cv01-u19.bin", 0x000000, 0x400000, CRC(5f5514da) SHA1(53f27364aee544572a82649c9ff29bacc642b732) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-feversos.bin", 0x0000, 0x0080, CRC(d80303aa) SHA1(8580f7c2223c72614516d800a98465e362c333ef) )
ROM_END

/***************************************************************************

                                Dodonpachi (Japan)

PCB:    AT-C03 D2
CPU:    MC68000-16
Sound:  YMZ280B
OSC:    28.0000MHz
        16.0000MHz
        16.9MHz (16.9344MHz?)

***************************************************************************/

ROM_START( ddonpach )
	ROM_REGION( 0x100000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "b1.u27", 0x000000, 0x080000, CRC(b5cdc8d3) SHA1(58757b50e21a27e500a82c03f62cf02a85389926) )
	ROM_LOAD16_BYTE( "b2.u26", 0x000001, 0x080000, CRC(6bbb063a) SHA1(e5de64b9c3efc0a38a2e0e16b78ee393bff63558) )

	ROM_REGION( 0x800000 * 2, "sprites", 0 )		/* Sprites: * 2 */
	ROM_LOAD( "u50.bin", 0x000000, 0x200000, CRC(14b260ec) SHA1(33bda210302428d5500115d0c7a839cdfcb67d17) )
	ROM_LOAD( "u51.bin", 0x200000, 0x200000, CRC(e7ba8cce) SHA1(ad74a6b7d53760b19587c4a6dbea937daa7e87ce) )
	ROM_LOAD( "u52.bin", 0x400000, 0x200000, CRC(02492ee0) SHA1(64d9cc64a4ad189a8b03cf6a749ddb732b4a0014) )
	ROM_LOAD( "u53.bin", 0x600000, 0x200000, CRC(cb4c10f0) SHA1(a622e8bd0c938b5d38b392b247400b744d8be288) )

	ROM_REGION( 0x200000, "layer0", 0 )	/* Layer 0 */
	ROM_LOAD( "u60.bin", 0x000000, 0x200000, CRC(903096a7) SHA1(a243e903fef7c4a7b71383263e82e42acd869261) )

	ROM_REGION( 0x200000, "layer1", 0 )	/* Layer 1 */
	ROM_LOAD( "u61.bin", 0x000000, 0x200000, CRC(d89b7631) SHA1(a66bb4955ca58fab8973ca37a0f971e9a67ce017) )

	ROM_REGION( 0x200000, "layer2", 0 )	/* Layer 2 */
	ROM_LOAD( "u62.bin", 0x000000, 0x200000, CRC(292bfb6b) SHA1(11b385991ee990eb5ef36e136b988802b5f90fa4) )

	ROM_REGION( 0x400000, "ymz", 0 )	/* Samples */
	ROM_LOAD( "u6.bin", 0x000000, 0x200000, CRC(9dfdafaf) SHA1(f5cb450cdc78a20c3a74c6dac05c9ac3cba08327) )
	ROM_LOAD( "u7.bin", 0x200000, 0x200000, CRC(795b17d5) SHA1(cbfc29f1df9600c82e0fdae00edd00da5b73e14c) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-ddonpach.bin", 0x0000, 0x0080, CRC(315fb546) SHA1(7f597107d1610fc286413e0e93c794c80c0c554f) )
ROM_END


ROM_START( ddonpachj )
	ROM_REGION( 0x100000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "u27.bin", 0x000000, 0x080000, CRC(2432ff9b) SHA1(fbc826c30553f6553ead40b312b73c049e8f4bf6) )
	ROM_LOAD16_BYTE( "u26.bin", 0x000001, 0x080000, CRC(4f3a914a) SHA1(ae98eba049f1462aa1145f6959b9f9a32c97278f) )

	ROM_REGION( 0x800000 * 2, "sprites", 0 )		/* Sprites: * 2 */
	ROM_LOAD( "u50.bin", 0x000000, 0x200000, CRC(14b260ec) SHA1(33bda210302428d5500115d0c7a839cdfcb67d17) )
	ROM_LOAD( "u51.bin", 0x200000, 0x200000, CRC(e7ba8cce) SHA1(ad74a6b7d53760b19587c4a6dbea937daa7e87ce) )
	ROM_LOAD( "u52.bin", 0x400000, 0x200000, CRC(02492ee0) SHA1(64d9cc64a4ad189a8b03cf6a749ddb732b4a0014) )
	ROM_LOAD( "u53.bin", 0x600000, 0x200000, CRC(cb4c10f0) SHA1(a622e8bd0c938b5d38b392b247400b744d8be288) )

	ROM_REGION( 0x200000, "layer0", 0 )	/* Layer 0 */
	ROM_LOAD( "u60.bin", 0x000000, 0x200000, CRC(903096a7) SHA1(a243e903fef7c4a7b71383263e82e42acd869261) )

	ROM_REGION( 0x200000, "layer1", 0 )	/* Layer 1 */
	ROM_LOAD( "u61.bin", 0x000000, 0x200000, CRC(d89b7631) SHA1(a66bb4955ca58fab8973ca37a0f971e9a67ce017) )

	ROM_REGION( 0x200000, "layer2", 0 )	/* Layer 2 */
	ROM_LOAD( "u62.bin", 0x000000, 0x200000, CRC(292bfb6b) SHA1(11b385991ee990eb5ef36e136b988802b5f90fa4) )

	ROM_REGION( 0x400000, "ymz", 0 )	/* Samples */
	ROM_LOAD( "u6.bin", 0x000000, 0x200000, CRC(9dfdafaf) SHA1(f5cb450cdc78a20c3a74c6dac05c9ac3cba08327) )
	ROM_LOAD( "u7.bin", 0x200000, 0x200000, CRC(795b17d5) SHA1(cbfc29f1df9600c82e0fdae00edd00da5b73e14c) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-ddonpach.bin", 0x0000, 0x0080, CRC(315fb546) SHA1(7f597107d1610fc286413e0e93c794c80c0c554f) )
ROM_END


/***************************************************************************

                                Donpachi

Known versions:

USA        Version 1.12 1995/05/2x
Korea      Version 1.12 1995/05/2x
Hong Kong  Version 1.10 1995/05/17
Japan      Version 1.01 1995/05/11

BOARD #:      AT-C01DP-2
CPU:          TMP68HC000-16
VOICE:        M6295 x2
OSC:          28.000/16.000/4.220MHz
EEPROM:       ATMEL 93C46
CUSTOM:       ATLUS 8647-01 013
              038 9429WX727 x3
              NMK 112 (M6295 sample ROM banking)

---------------------------------------------------
 filenames          devices       kind
---------------------------------------------------
 PRG.U29            27C4096       68000 main prg.
 U58.BIN            27C020        gfx   data
 ATDP.U32           57C8200       M6295 data
 ATDP.U33           57C16200      M6295 data
 ATDP.U44           57C16200      gfx   data
 ATDP.U45           57C16200      gfx   data
 ATDP.U54           57C8200       gfx   data
 ATDP.U57           57C8200       gfx   data

 USA Version
----------------------------------------------------
 prgu.U29           27C4002       68000 Main Program
 text.u58           27C2001       Labeled as "TEXT"

***************************************************************************/

ROM_START( donpachi )
	ROM_REGION( 0x080000, "maincpu", 0 )		/* 68000 code */
	ROM_LOAD16_WORD_SWAP( "prgu.u29",     0x00000, 0x80000, CRC(89c36802) SHA1(7857c726cecca5a4fce282e0d2b873774d2c1b1d) )

	ROM_REGION( 0x400000 * 2, "sprites", 0 )		/* Sprites */
	ROM_LOAD( "atdp.u44", 0x000000, 0x200000, CRC(7189e953) SHA1(53adbe6ea5e01ecb48575e9db82cc3d0dc8a3726) )
	ROM_LOAD( "atdp.u45", 0x200000, 0x200000, CRC(6984173f) SHA1(625dd6674adeb206815855b8b6a1fba79ed5c4cd) )

	ROM_REGION( 0x100000, "layer0", 0 )	/* Layer 0 */
	ROM_LOAD( "atdp.u54", 0x000000, 0x100000, CRC(6bda6b66) SHA1(6472e6706505bac17484fb8bf4e8922ced4adf63) )

	ROM_REGION( 0x100000, "layer1", 0 )	/* Layer 1 */
	ROM_LOAD( "atdp.u57", 0x000000, 0x100000, CRC(0a0e72b9) SHA1(997e8253777e7acca5a1c0c4026e78eecc122d5d) )

	ROM_REGION( 0x040000, "layer2", 0 )	/* Text / Character Layer */
	ROM_LOAD( "text.u58", 0x000000, 0x040000, CRC(5dba06e7) SHA1(f9dab7f6c732a683fddb4cae090a875b3962332b) )

	ROM_REGION( 0x240000, "oki1", 0 )	/* OKIM6295 #1 Samples */
	/* Leave the 0x40000 bytes addressable by the chip empty */
	ROM_LOAD( "atdp.u33", 0x040000, 0x200000, CRC(d749de00) SHA1(64a0acc23eb2515e7d0459f0289919e083c63afc) )

	ROM_REGION( 0x340000, "oki2", 0 )	/* OKIM6295 #2 Samples */
	/* Leave the 0x40000 bytes addressable by the chip empty */
	ROM_LOAD( "atdp.u32", 0x040000, 0x100000, CRC(0d89fcca) SHA1(e16ed15fa5e72537822f7b37e83ccfed0fa87338) )
	ROM_LOAD( "atdp.u33", 0x140000, 0x200000, CRC(d749de00) SHA1(64a0acc23eb2515e7d0459f0289919e083c63afc) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-donpachi.bin", 0x0000, 0x0080, CRC(315fb546) SHA1(7f597107d1610fc286413e0e93c794c80c0c554f) )
ROM_END

ROM_START( donpachij )
	ROM_REGION( 0x080000, "maincpu", 0 )		/* 68000 code */
	ROM_LOAD16_WORD_SWAP( "prg.u29",     0x00000, 0x80000, CRC(6be14af6) SHA1(5b1158071f160efeded816ae4c4edca1d00d6e05) )

	ROM_REGION( 0x400000 * 2, "sprites", 0 )		/* Sprites */
	ROM_LOAD( "atdp.u44", 0x000000, 0x200000, CRC(7189e953) SHA1(53adbe6ea5e01ecb48575e9db82cc3d0dc8a3726) )
	ROM_LOAD( "atdp.u45", 0x200000, 0x200000, CRC(6984173f) SHA1(625dd6674adeb206815855b8b6a1fba79ed5c4cd) )

	ROM_REGION( 0x100000, "layer0", 0 )	/* Layer 0 */
	ROM_LOAD( "atdp.u54", 0x000000, 0x100000, CRC(6bda6b66) SHA1(6472e6706505bac17484fb8bf4e8922ced4adf63) )

	ROM_REGION( 0x100000, "layer1", 0 )	/* Layer 1 */
	ROM_LOAD( "atdp.u57", 0x000000, 0x100000, CRC(0a0e72b9) SHA1(997e8253777e7acca5a1c0c4026e78eecc122d5d) )

	ROM_REGION( 0x040000, "layer2", 0 )	/* Text / Character Layer */
	ROM_LOAD( "u58.bin", 0x000000, 0x040000, CRC(285379ff) SHA1(b9552edcec29ddf4b552800b145c398b94117ab0) )

	ROM_REGION( 0x240000, "oki1", 0 )	/* OKIM6295 #1 Samples */
	/* Leave the 0x40000 bytes addressable by the chip empty */
	ROM_LOAD( "atdp.u33", 0x040000, 0x200000, CRC(d749de00) SHA1(64a0acc23eb2515e7d0459f0289919e083c63afc) )

	ROM_REGION( 0x340000, "oki2", 0 )	/* OKIM6295 #2 Samples */
	/* Leave the 0x40000 bytes addressable by the chip empty */
	ROM_LOAD( "atdp.u32", 0x040000, 0x100000, CRC(0d89fcca) SHA1(e16ed15fa5e72537822f7b37e83ccfed0fa87338) )
	ROM_LOAD( "atdp.u33", 0x140000, 0x200000, CRC(d749de00) SHA1(64a0acc23eb2515e7d0459f0289919e083c63afc) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-donpachi.bin", 0x0000, 0x0080, CRC(315fb546) SHA1(7f597107d1610fc286413e0e93c794c80c0c554f) )
ROM_END

ROM_START( donpachikr )
	ROM_REGION( 0x080000, "maincpu", 0 )		/* 68000 code */
	ROM_LOAD16_WORD_SWAP( "prgk.u26",    0x00000, 0x80000, CRC(bbaf4c8b) SHA1(0f9d42c8c4c5b69e3d39bf768bc4b663f66b4f36) )

	ROM_REGION( 0x400000 * 2, "sprites", 0 )		/* Sprites */
	ROM_LOAD( "atdp.u44", 0x000000, 0x200000, CRC(7189e953) SHA1(53adbe6ea5e01ecb48575e9db82cc3d0dc8a3726) )
	ROM_LOAD( "atdp.u45", 0x200000, 0x200000, CRC(6984173f) SHA1(625dd6674adeb206815855b8b6a1fba79ed5c4cd) )

	ROM_REGION( 0x100000, "layer0", 0 )	/* Layer 0 */
	ROM_LOAD( "atdp.u54", 0x000000, 0x100000, CRC(6bda6b66) SHA1(6472e6706505bac17484fb8bf4e8922ced4adf63) )

	ROM_REGION( 0x100000, "layer1", 0 )	/* Layer 1 */
	ROM_LOAD( "atdp.u57", 0x000000, 0x100000, CRC(0a0e72b9) SHA1(997e8253777e7acca5a1c0c4026e78eecc122d5d) )

	ROM_REGION( 0x040000, "layer2", 0 )	/* Text / Character Layer */
	ROM_LOAD( "u58.bin", 0x000000, 0x040000, CRC(285379ff) SHA1(b9552edcec29ddf4b552800b145c398b94117ab0) )

	ROM_REGION( 0x240000, "oki1", 0 )	/* OKIM6295 #1 Samples */
	/* Leave the 0x40000 bytes addressable by the chip empty */
	ROM_LOAD( "atdp.u33", 0x040000, 0x200000, CRC(d749de00) SHA1(64a0acc23eb2515e7d0459f0289919e083c63afc) )

	ROM_REGION( 0x340000, "oki2", 0 )	/* OKIM6295 #2 Samples */
	/* Leave the 0x40000 bytes addressable by the chip empty */
	ROM_LOAD( "atdp.u32", 0x040000, 0x100000, CRC(0d89fcca) SHA1(e16ed15fa5e72537822f7b37e83ccfed0fa87338) )
	ROM_LOAD( "atdp.u33", 0x140000, 0x200000, CRC(d749de00) SHA1(64a0acc23eb2515e7d0459f0289919e083c63afc) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-donpachi.bin", 0x0000, 0x0080, CRC(315fb546) SHA1(7f597107d1610fc286413e0e93c794c80c0c554f) )
ROM_END

ROM_START( donpachihk )
	ROM_REGION( 0x080000, "maincpu", 0 )		/* 68000 code */
	ROM_LOAD16_WORD_SWAP( "37.u29",    0x00000, 0x80000, CRC(71f39f30) SHA1(08a028208f21c073d450a29061604f27775786a8) )

	ROM_REGION( 0x400000 * 2, "sprites", 0 )		/* Sprites */
	ROM_LOAD( "atdp.u44", 0x000000, 0x200000, CRC(7189e953) SHA1(53adbe6ea5e01ecb48575e9db82cc3d0dc8a3726) )
	ROM_LOAD( "atdp.u45", 0x200000, 0x200000, CRC(6984173f) SHA1(625dd6674adeb206815855b8b6a1fba79ed5c4cd) )

	ROM_REGION( 0x100000, "layer0", 0 )	/* Layer 0 */
	ROM_LOAD( "atdp.u54", 0x000000, 0x100000, CRC(6bda6b66) SHA1(6472e6706505bac17484fb8bf4e8922ced4adf63) )

	ROM_REGION( 0x100000, "layer1", 0 )	/* Layer 1 */
	ROM_LOAD( "atdp.u57", 0x000000, 0x100000, CRC(0a0e72b9) SHA1(997e8253777e7acca5a1c0c4026e78eecc122d5d) )

	ROM_REGION( 0x040000, "layer2", 0 )	/* Text / Character Layer */
	ROM_LOAD( "u58.bin", 0x000000, 0x040000, CRC(285379ff) SHA1(b9552edcec29ddf4b552800b145c398b94117ab0) )

	ROM_REGION( 0x240000, "oki1", 0 )	/* OKIM6295 #1 Samples */
	/* Leave the 0x40000 bytes addressable by the chip empty */
	ROM_LOAD( "atdp.u33", 0x040000, 0x200000, CRC(d749de00) SHA1(64a0acc23eb2515e7d0459f0289919e083c63afc) )

	ROM_REGION( 0x340000, "oki2", 0 )	/* OKIM6295 #2 Samples */
	/* Leave the 0x40000 bytes addressable by the chip empty */
	ROM_LOAD( "atdp.u32", 0x040000, 0x100000, CRC(0d89fcca) SHA1(e16ed15fa5e72537822f7b37e83ccfed0fa87338) )
	ROM_LOAD( "atdp.u33", 0x140000, 0x200000, CRC(d749de00) SHA1(64a0acc23eb2515e7d0459f0289919e083c63afc) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-donpachi.bin", 0x0000, 0x0080, CRC(315fb546) SHA1(7f597107d1610fc286413e0e93c794c80c0c554f) )
ROM_END


/***************************************************************************

                                ESP Ra.De.

ATC04
OSC:    28.0, 16.0, 16.9 MHz

***************************************************************************/

ROM_START( esprade )
	ROM_REGION( 0x100000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "u42.int", 0x000000, 0x080000, CRC(3b510a73) SHA1(ab1666eb826cb4a71588d86831dd18a2ef1c2a33) )
	ROM_LOAD16_BYTE( "u41.int", 0x000001, 0x080000, CRC(97c1b649) SHA1(37a56b7b9662219a356aee3f4b5cbb774ac4950e) )

	ROM_REGION( 0x1000000, "sprites", 0 )		/* Sprites */
	ROM_LOAD16_BYTE( "u63.bin", 0x000000, 0x400000, CRC(2f2fe92c) SHA1(9519e365248bcec8419786eabb16fe4aae299af5) )
	ROM_LOAD16_BYTE( "u64.bin", 0x000001, 0x400000, CRC(491a3da4) SHA1(53549a2bd3edc7b5e73fb46e1421b156bb0c190f) )
	ROM_LOAD16_BYTE( "u65.bin", 0x800000, 0x400000, CRC(06563efe) SHA1(94e72da1f542b4e0525b4b43994242816b43dbdc) )
	ROM_LOAD16_BYTE( "u66.bin", 0x800001, 0x400000, CRC(7bbe4cfc) SHA1(e77d0ed7a11b5abca1df8a0eb20ac9360cf79e76) )

	ROM_REGION( 0x800000, "layer0", 0 )	/* Layer 0 */
	ROM_LOAD( "u54.bin", 0x000000, 0x400000, CRC(e7ca6936) SHA1(b7f5ab67071a1d9dd3d2c1cd2304d9cdad68850c) )
	ROM_LOAD( "u55.bin", 0x400000, 0x400000, CRC(f53bd94f) SHA1(d0a74fb3d36fe522ef075e5ae44a9980da8abe2f) )

	ROM_REGION( 0x800000, "layer1", 0 )	/* Layer 1 */
	ROM_LOAD( "u52.bin", 0x000000, 0x400000, CRC(e7abe7b4) SHA1(e98da45497e1aaf0d6ab352ec3e43c7438ed792a) )
	ROM_LOAD( "u53.bin", 0x400000, 0x400000, CRC(51a0f391) SHA1(8b7355cbad119f4e1add14e5cd5e343ec6706104) )

	ROM_REGION( 0x400000, "layer2", 0 )	/* Layer 2 */
	ROM_LOAD( "u51.bin", 0x000000, 0x400000, CRC(0b9b875c) SHA1(ef05447cd8565ae24bb71db42342724622ad1e3e) )

	ROM_REGION( 0x400000, "ymz", 0 )	/* Samples */
	ROM_LOAD( "u19.bin", 0x000000, 0x400000, CRC(f54b1cab) SHA1(34d70bb5798de85d892c062001d9ac1d6604fd9f) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-esprade.bin", 0x0000, 0x0080, CRC(315fb546) SHA1(7f597107d1610fc286413e0e93c794c80c0c554f) )
ROM_END

ROM_START( espradej )
	ROM_REGION( 0x100000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "u42.bin", 0x000000, 0x080000, CRC(75d03c42) SHA1(1c176185b6f1531752b633a97f705ffa0cfeb5ad) )
	ROM_LOAD16_BYTE( "u41.bin", 0x000001, 0x080000, CRC(734b3ef0) SHA1(f584227b85c347d62d5f179445011ce0f607bcfd) )

	ROM_REGION( 0x1000000, "sprites", 0 )		/* Sprites */
	ROM_LOAD16_BYTE( "u63.bin", 0x000000, 0x400000, CRC(2f2fe92c) SHA1(9519e365248bcec8419786eabb16fe4aae299af5) )
	ROM_LOAD16_BYTE( "u64.bin", 0x000001, 0x400000, CRC(491a3da4) SHA1(53549a2bd3edc7b5e73fb46e1421b156bb0c190f) )
	ROM_LOAD16_BYTE( "u65.bin", 0x800000, 0x400000, CRC(06563efe) SHA1(94e72da1f542b4e0525b4b43994242816b43dbdc) )
	ROM_LOAD16_BYTE( "u66.bin", 0x800001, 0x400000, CRC(7bbe4cfc) SHA1(e77d0ed7a11b5abca1df8a0eb20ac9360cf79e76) )

	ROM_REGION( 0x800000, "layer0", 0 )	/* Layer 0 */
	ROM_LOAD( "u54.bin", 0x000000, 0x400000, CRC(e7ca6936) SHA1(b7f5ab67071a1d9dd3d2c1cd2304d9cdad68850c) )
	ROM_LOAD( "u55.bin", 0x400000, 0x400000, CRC(f53bd94f) SHA1(d0a74fb3d36fe522ef075e5ae44a9980da8abe2f) )

	ROM_REGION( 0x800000, "layer1", 0 )	/* Layer 1 */
	ROM_LOAD( "u52.bin", 0x000000, 0x400000, CRC(e7abe7b4) SHA1(e98da45497e1aaf0d6ab352ec3e43c7438ed792a) )
	ROM_LOAD( "u53.bin", 0x400000, 0x400000, CRC(51a0f391) SHA1(8b7355cbad119f4e1add14e5cd5e343ec6706104) )

	ROM_REGION( 0x400000, "layer2", 0 )	/* Layer 2 */
	ROM_LOAD( "u51.bin", 0x000000, 0x400000, CRC(0b9b875c) SHA1(ef05447cd8565ae24bb71db42342724622ad1e3e) )

	ROM_REGION( 0x400000, "ymz", 0 )	/* Samples */
	ROM_LOAD( "u19.bin", 0x000000, 0x400000, CRC(f54b1cab) SHA1(34d70bb5798de85d892c062001d9ac1d6604fd9f) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-esprade.bin", 0x0000, 0x0080, CRC(315fb546) SHA1(7f597107d1610fc286413e0e93c794c80c0c554f) )
ROM_END

ROM_START( espradejo )
	ROM_REGION( 0x100000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "u42.old", 0x000000, 0x080000, CRC(0718c7e5) SHA1(c7d1f30bd2ef363cad15b6918f9980312a15809a) )
	ROM_LOAD16_BYTE( "u41.old", 0x000001, 0x080000, CRC(def30539) SHA1(957ad0b06f06689ae71393572592f6b8f818603a) )

	ROM_REGION( 0x1000000, "sprites", 0 )		/* Sprites */
	ROM_LOAD16_BYTE( "u63.bin", 0x000000, 0x400000, CRC(2f2fe92c) SHA1(9519e365248bcec8419786eabb16fe4aae299af5) )
	ROM_LOAD16_BYTE( "u64.bin", 0x000001, 0x400000, CRC(491a3da4) SHA1(53549a2bd3edc7b5e73fb46e1421b156bb0c190f) )
	ROM_LOAD16_BYTE( "u65.bin", 0x800000, 0x400000, CRC(06563efe) SHA1(94e72da1f542b4e0525b4b43994242816b43dbdc) )
	ROM_LOAD16_BYTE( "u66.bin", 0x800001, 0x400000, CRC(7bbe4cfc) SHA1(e77d0ed7a11b5abca1df8a0eb20ac9360cf79e76) )

	ROM_REGION( 0x800000, "layer0", 0 )	/* Layer 0 */
	ROM_LOAD( "u54.bin", 0x000000, 0x400000, CRC(e7ca6936) SHA1(b7f5ab67071a1d9dd3d2c1cd2304d9cdad68850c) )
	ROM_LOAD( "u55.bin", 0x400000, 0x400000, CRC(f53bd94f) SHA1(d0a74fb3d36fe522ef075e5ae44a9980da8abe2f) )

	ROM_REGION( 0x800000, "layer1", 0 )	/* Layer 1 */
	ROM_LOAD( "u52.bin", 0x000000, 0x400000, CRC(e7abe7b4) SHA1(e98da45497e1aaf0d6ab352ec3e43c7438ed792a) )
	ROM_LOAD( "u53.bin", 0x400000, 0x400000, CRC(51a0f391) SHA1(8b7355cbad119f4e1add14e5cd5e343ec6706104) )

	ROM_REGION( 0x400000, "layer2", 0 )	/* Layer 2 */
	ROM_LOAD( "u51.bin", 0x000000, 0x400000, CRC(0b9b875c) SHA1(ef05447cd8565ae24bb71db42342724622ad1e3e) )

	ROM_REGION( 0x400000, "ymz", 0 )	/* Samples */
	ROM_LOAD( "u19.bin", 0x000000, 0x400000, CRC(f54b1cab) SHA1(34d70bb5798de85d892c062001d9ac1d6604fd9f) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-esprade.bin", 0x0000, 0x0080, CRC(315fb546) SHA1(7f597107d1610fc286413e0e93c794c80c0c554f) )
ROM_END


/***************************************************************************

                                Gaia Crusaders

Noise Factory, 1999

PCB Layout
----------

|------------------------------------------------|
|   YAC516    YMZ280B      XC9536      68000     |
|          16MHz                       PRG2   PAL|
|                          TC51832     PRG1      |
|     SND3     SND2        TC51832   28.322MHz   |
|              SND1        62256     16MHz       |
|                          62256                 |
|J 62256 62256 62256 62256 62256 62256           |
|A                                 KM416C256     |
|M                                      KM416C256|
|M     -------------------  ---------------      |
|A     |     |     |     |  |             | 62256|
|      |     |     |     |  |             |      |
| DSW1 |     |     |     |  |013 9918EX008| 62256|
|      |038 9838WX003(x3)|  |             |      |
|      -------------------  ---------------      |
| DSW2                                           |
|                    XC9536          OBJ2        |
|                                                |
|       BG2     BG3    BG1           OBJ1        |
|                                                |
|------------------------------------------------|

Notes:
      68000 clock  : 16.000MHz
      YMZ280B clock: 16.000MHz
      VSync        : 58Hz
      HSync        : 15.40kHz

***************************************************************************/

ROM_START( gaia )
	ROM_REGION( 0x100000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "prg1.127", 0x000000, 0x080000, CRC(47b904b2) SHA1(58b9b55f59cf00f70b690a0371096e86f4d723c2) )
	ROM_LOAD16_BYTE( "prg2.128", 0x000001, 0x080000, CRC(469b7794) SHA1(502f855c51005a866900b19c3a0a170d9ea02392) )

	ROM_REGION( 0x1000000, "sprites", 0 )  /* Sprites */
	ROM_LOAD( "obj1.736", 0x000000, 0x400000, CRC(f4f84e5d) SHA1(8f445dd7a5c8a996939c211e5aec5742121a6e7e) )
	ROM_LOAD( "obj2.738", 0x400000, 0x400000, CRC(15c2a9ce) SHA1(631eb2968395be86ef2403733e7d4ec769a013b9) )

	ROM_REGION( 0x400000, "layer0", 0 )	/* Layer 0 */
	ROM_LOAD( "bg1.989", 0x000000, 0x400000, CRC(013a693d) SHA1(2cc5be6f47c13febed942e1c3167946efedc5f9b) )

	ROM_REGION( 0x400000, "layer1", 0 )	/* Layer 1 */
	ROM_LOAD( "bg2.995", 0x000000, 0x400000, CRC(783cc62f) SHA1(8b6e4212688b53be5ecc29ff2d41fd43e7d0a420) )

	ROM_REGION( 0x400000, "layer2", 0 )	/* Layer 2 */
	ROM_LOAD( "bg3.998", 0x000000, 0x400000, CRC(bcd61d1c) SHA1(660a3b02a8c39e1117b00d0ad06f73221fef4ce8) )

	ROM_REGION( 0xc00000, "ymz", 0 )	/* Samples */
	ROM_LOAD( "snd1.447", 0x000000, 0x400000, CRC(92770a52) SHA1(81f6835e1b45eb0f367e4586fdda92466f02edb9) )
	ROM_LOAD( "snd2.454", 0x400000, 0x400000, CRC(329ae1cf) SHA1(0c5e5074a5d8f4fb85ab4893bc953f192dcb301a) )
	ROM_LOAD( "snd3.455", 0x800000, 0x400000, CRC(4048d64e) SHA1(5e4ec6d37e70484e2fcd04188385e79ef0b53026) )
ROM_END

/*
Thunder Heroes
Primetek Investments Ltd. , 2001

A quasi-clone, remake or continuation of Gaia Crusaders but is clearly a different game.

PCB Layout
----------

|------------------------------------------------------|
| 4558  YAC516   YMZ280B     XILINX                    |
| 4558   16MHz               XC9536    68000      PAL  |
|                SND2                                  |
|   VOL  SND3                TC51832   EPM0            |
|                SND1                                  |
|                            TC51832   EPM1            |
|                    58257           28.322MHz         |
|J                   58257                16MHz        |
|A   58257  58257  58257  58257  58257                 |
|M  DSW1                      58257   M514260  M514260 |
|M                                                     |
|A  DSW2 |------|  |------|  |------|  |--------|58257 |
|        | 038  |  | 038  |  | 038  |  |  013   |      |
|        |      |  |      |  |      |  |        |58257 |
|        |------|  |------|  |------|  |        |      |
|                                      |--------|      |
|                         XILINX            OBJ2       |
|                         XC9536                       |
|       BG2       BG3       BG1             OBJ1       |
|------------------------------------------------------|
Notes:
      68000 clock 16.00MHz
      YMZ280B clock 16.000MHz
      HSync 15.4kHz
      VSync 58Hz
      038/013 = Same video chips used on some Banpresto games
*/

ROM_START( theroes )
	ROM_REGION( 0x100000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "t-hero-epm1.u0127", 0x000000, 0x080000, CRC(09db7195) SHA1(6aa5aa80e3b74e405ed8f1b9b801ce4367756986) )
	ROM_LOAD16_BYTE( "t-hero-epm0.u0129", 0x000001, 0x080000, CRC(2d4e3310) SHA1(7c3284a2adc7943db50933a209d037422f87f80b) )

	ROM_REGION( 0x1000000, "sprites", 0 )  /* Sprites */
	ROM_LOAD( "t-hero-obj1.u0736", 0x000000, 0x400000, CRC(35090f7c) SHA1(035e6c12a87d9c7241eea34fc7e2170bec842acc) )
	ROM_LOAD( "t-hero-obj2.u0738", 0x400000, 0x400000, CRC(71605108) SHA1(6070c26d8f22fafc81d97cacfef96ae652e355d0) )

	ROM_REGION( 0x400000, "layer0", 0 )	/* Layer 0 */
	ROM_LOAD( "t-hero-bg1.u0999", 0x000000, 0x400000, CRC(47b0fb40) SHA1(a7217b3d805b4255c589821cdadd9b190cada525) )

	ROM_REGION( 0x400000, "layer1", 0 )	/* Layer 1 */
	ROM_LOAD( "t-hero-bg2.u0995", 0x000000, 0x400000, CRC(b16237a1) SHA1(66aed2c5036492a17d20de90333e172a6f117851) )

	ROM_REGION( 0x400000, "layer2", 0 )	/* Layer 2 */
	ROM_LOAD( "t-hero-bg3.u0998", 0x000000, 0x400000, CRC(08eb5604) SHA1(3d32966708c73198272c40e6ddc680bf4c7919eb) )

	ROM_REGION( 0xc00000, "ymz", 0 )	/* Samples */
	ROM_LOAD( "crvsaders-snd1.u0447", 0x000000, 0x400000, CRC(92770a52) SHA1(81f6835e1b45eb0f367e4586fdda92466f02edb9) )
	ROM_LOAD( "crvsaders-snd2.u0454", 0x400000, 0x400000, CRC(329ae1cf) SHA1(0c5e5074a5d8f4fb85ab4893bc953f192dcb301a) )
	ROM_LOAD( "t-hero-snd3.u0455",    0x800000, 0x400000, CRC(52b0b2c0) SHA1(6e96698905391c21a4fedd60e2768734b58add4e) )
ROM_END


/***************************************************************************

                                Guwange (Japan)

PCB:    ATC05
CPU:    MC68000-16
Sound:  YMZ280B
OSC:    28.0000MHz
        16.0000MHz
        16.9MHz

***************************************************************************/

ROM_START( guwange )
	ROM_REGION( 0x100000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "gu-u0127.bin", 0x000000, 0x080000, CRC(f86b5293) SHA1(f8b1cd77cc25328d5010889850e4b86c27d9e396) )
	ROM_LOAD16_BYTE( "gu-u0129.bin", 0x000001, 0x080000, CRC(6c0e3b93) SHA1(aaad6569b9a7b6f9a315062f9fedfc95851c1bc6) )

	ROM_REGION( 0x2000000, "sprites", 0 )		/* Sprites */
	ROM_LOAD16_BYTE( "u083.bin", 0x0000000, 0x800000, CRC(adc4b9c4) SHA1(3f9fb004e19187bbfa87ddfe8cfc69740656a1bd) )
	ROM_LOAD16_BYTE( "u082.bin", 0x0000001, 0x800000, CRC(3d75876c) SHA1(705b8c2dbdc31e9516f429969f87988beec796d7) )
	ROM_LOAD16_BYTE( "u086.bin", 0x1000000, 0x400000, CRC(188e4f81) SHA1(626074d81782a6de0b52406331b4b8561d3e36f5) )
	ROM_RELOAD(                  0x1800000, 0x400000 )
	ROM_LOAD16_BYTE( "u085.bin", 0x1000001, 0x400000, CRC(a7d5659e) SHA1(10abac022ebe106a3ca7186ff18ca2757f903033) )
	ROM_RELOAD(                  0x1800001, 0x400000 )
//sprite bug fix?
//  ROM_FILL(                    0x1800000, 0x800000, 0xff )

	ROM_REGION( 0x800000, "layer0", 0 )	/* Layer 0 */
	ROM_LOAD( "u101.bin", 0x000000, 0x800000, CRC(0369491f) SHA1(ca6b1345506f13a17c9bace01637d1f61a278644) )

	ROM_REGION( 0x400000, "layer1", 0 )	/* Layer 1 */
	ROM_LOAD( "u10102.bin", 0x000000, 0x400000, CRC(e28d6855) SHA1(7001a6e298c6a1fcceb79586bf5f4bf0f30027f6) )

	ROM_REGION( 0x400000, "layer2", 0 )	/* Layer 2 */
	ROM_LOAD( "u10103.bin", 0x000000, 0x400000, CRC(0fe91b8e) SHA1(8b71ebeef5e4d2b00fdaaab97776d74e1c96dc59) )

	ROM_REGION( 0x400000, "ymz", 0 )	/* Samples */
	ROM_LOAD( "u0462.bin", 0x000000, 0x400000, CRC(b3d75691) SHA1(71d8dae92be1542a3cff50efeec0bf3c14ab59f5) )

    ROM_REGION( 0x0004, "plds", 0 )
    ROM_LOAD( "atc05-1.bin", 0x0000, 0x0001, NO_DUMP ) /* GAL16V8D-15LP located at U159 */
    ROM_LOAD( "u0259.bin",   0x0000, 0x0001, NO_DUMP ) /* XC9536-15PC44C Located at U0249. (Chip label different then label silk screened onto the board.) */
    ROM_LOAD( "u108.bin",    0x0000, 0x0001, NO_DUMP ) /* XC9536-15PC44C Located at U108. */
    ROM_LOAD( "u084.bin",    0x0000, 0x0001, NO_DUMP ) /* XC9536-15PC44C Located at U084. */

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-guwange.bin", 0x0000, 0x0080, CRC(c3174959) SHA1(29b5c94722756481e4f84bfd75dee15fdb5c8cf7) )
ROM_END


/***************************************************************************

                                Hot Dog Storm
Marble 1996

6264 6264 MP7 6264 6264 MP6 6264 6264 MP5  32MHz
                                                6264
                                                6264
                                                  MP4
                                                  MP3
                                                       93C46
                                      68257
                                      68257 68000-12
                                             YM2203
                                          Z80
  MP8 MP9
  68257
  68257                         U19    MP1      6296

***************************************************************************/

ROM_START( hotdogst )
	ROM_REGION( 0x100000, "maincpu", 0 )	/* 68000 code */
	ROM_LOAD16_BYTE( "mp3u29", 0x00000, 0x80000, CRC(1f4e5479) SHA1(5c3d7b36b1eda4c87c53e4f7cf89951cc5bcc871) )
	ROM_LOAD16_BYTE( "mp4u28", 0x00001, 0x80000, CRC(6f1c3c4b) SHA1(ab4e4d9b2ef74a2eefda718e120bef05fd0346ff) )

	ROM_REGION( 0x48000, "audiocpu", 0 )	/* Z80 code */
	ROM_LOAD( "mp2u19", 0x00000, 0x08000, CRC(ff979ebe) SHA1(4cb80086cfdc69a321c7f75455cef89e20488b76) )	// FIRST AND SECOND HALF IDENTICAL
	ROM_CONTINUE(       0x10000, 0x38000             )

	ROM_REGION( 0x400000 * 2, "sprites", 0 )		/* Sprites: * 2 */
	ROM_LOAD( "mp9u55", 0x000000, 0x200000, CRC(258d49ec) SHA1(f39e30c82d8f680f248e1eb59d7c5acb479fa277) )
	ROM_LOAD( "mp8u54", 0x200000, 0x200000, CRC(bdb4d7b8) SHA1(0dd490988aa84b0e9a21ade5fd606b03eca13f6c) )

	ROM_REGION( 0x80000, "layer0", 0 )	/* Layer 0 */
	ROM_LOAD( "mp7u56", 0x00000, 0x80000, CRC(87c21c50) SHA1(fc0eea79abdd96edb4fa2c7047aaa728ef838234) )

	ROM_REGION( 0x80000, "layer1", 0 )	/* Layer 1 */
	ROM_LOAD( "mp6u61", 0x00000, 0x80000, CRC(4dafb288) SHA1(4756259adfe49ba42cde25e7902655b0f0731a6c) )

	ROM_REGION( 0x80000, "layer2", 0 )	/* Layer 2 */
	ROM_LOAD( "mp5u64", 0x00000, 0x80000, CRC(9b26458c) SHA1(acef62422fa3f92e6ca1eba0ee6fb914cd1ee190) )

	ROM_REGION( 0xc0000, "oki", 0 )	/* Samples */
	/* Leave the 0x40000 bytes addressable by the chip empty */
	ROM_LOAD( "mp1u65", 0x40000, 0x80000, CRC(4868be1b) SHA1(32b8234b19fdbe07fa5057fa7965e36807e35e77) )	// 1xxxxxxxxxxxxxxxxxx = 0xFF, 4 x 0x20000

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-hotdogst.bin", 0x0000, 0x0080, CRC(12b4f934) SHA1(5b28d8fbd78869db78ce49e541a9d65558841966) )
ROM_END


/***************************************************************************

Koro Koro Quest
Takumi, 1999

Hardware is kind of Banpresto-ish

TUG-01B
MP-001
|--------------------------------------------------|
|        NJM4560   ROM.U0130                       |
|                                     68000        |
|                                                  |
|     YAC516                       16MHz       PAL |
|                                  PAL             |
|                PAL    PAL                        |
|M                 62256                   3V_BATT |
|A    93C46        62256                           |
|H                                                 |
|J                 28MHz  |---------|              |
|O        |------|        |  013    |              |
|N        | 038  |        |  9838E  |     M5M44260 |
|G        | 9838W| 62256  |  X004   |     M5M44260 |
|5        | X004 | 62256  |         |              |
|6        |------|        |---------|              |
|  YMZ280B                                 62256   |
|  16.9344MHz                              62256   |
|                                                  |
|                     PAL                          |
|                                                  |
|ROM.U1186                                 X       |
|                                                  |
|   X       ROM.U1060   ROM.U1051      ROM.U1066   |
|--------------------------------------------------|

 PCB Number - TUG-01B MP001-00175
 68000-16 + 16MHZ OSC
 YMZ280B + YAC516-M + Xtal 16.9344MHz
 93C46 EEPROM
 Custom - 013 9838EX004 (QFP240), 038 9838WX004 (QFP144) + OSC 28MHz
 RAM - 62256 (x8), M5M44260 (x2)
 3volt battery
 GAL16V8H (x5)

***************************************************************************/

ROM_START( korokoro )
	ROM_REGION( 0x80000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "mp-001_ver07.u0130", 0x000000, 0x080000, CRC(86c7241f) SHA1(c9f0ab63c4fe36df1300445e9bb0d5c6a1bb733f) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x180000 * 2, "sprites", 0 )		/* Sprites: * 2 */
	ROM_LOAD( "mp-001_ver01.u1066", 0x000000, 0x100000, CRC(c5c6af7e) SHA1(13ac26fd703672a01d629be4e5efe9fb8720a4fb) )
	ROM_LOAD( "mp-001_ver01.u1051", 0x100000, 0x080000, CRC(fe5e28e8) SHA1(44da1a7d813b149f9bae351bbcbd0bc2d4c70e10) )	// 1xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x100000, "layer0", 0 )	/* Layer 0 */
	ROM_LOAD( "mp-001_ver01.u1060", 0x000000, 0x100000, CRC(ec9cf9d8) SHA1(32fa7120e30c14e484de3b3a9c93efe3654d43c8) )

	ROM_REGION( 0x100000, "ymz", 0 )	/* Samples */
	ROM_LOAD( "mp-001_ver01.u1186", 0x000000, 0x100000, CRC(d16e7c5d) SHA1(1f825ace3ed2e23c8d3212320c4645d3d52214c7) )
ROM_END

ROM_START( crusherm )
	ROM_REGION( 0x80000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "mp-003ver01.u0130", 0x000000, 0x080000, CRC(a4f56e6b) SHA1(1d3af7602c48a6b6c76c376dbc8ad3823b56868a) )

	ROM_REGION( 0x200000 * 2, "sprites", 0 )		/* Sprites: * 2 */
	ROM_LOAD( "mp-003ver01.u1067", 0x000000, 0x100000, CRC(268a4921) SHA1(8bb818466616051af01680b381af53b8b6a18428) )
	ROM_LOAD( "mp-003ver01.u1066", 0x100000, 0x100000, CRC(79e77a6e) SHA1(9d03dd083769851d628ba6b3d77cfde9603e74f4) )

	ROM_REGION( 0x100000, "layer0", 0 )	/* Layer 0 */
	ROM_LOAD( "mp-003ver01.u1060", 0x000000, 0x100000, CRC(7661893e) SHA1(d51645c96247b039214393ba5eae7357144dfd65) )

	ROM_REGION( 0x200000, "ymz", 0 )	/* Samples */
	ROM_LOAD( "mp-003ver01.u1186", 0x000000, 0x100000, CRC(c3aeb745) SHA1(1bb8ab0512a9a9b0d3ce15f90b49cda431fb14eb) )
	ROM_LOAD( "mp-003ver01.u1187", 0x100000, 0x100000, CRC(d9312497) SHA1(a349cfdbcad96701a74f06394e87f0e0614e115d) )
ROM_END

/***************************************************************************

                                Mazinger Z

Banpresto 1994

U63               038               62256
                  9335EX706         62256
3664                            62256  62256
3664                                U924      32MHz
                                    U24
U60               038             68000
                  9335EX706
3664                                U21   YM2203  92E422
3664                                Z80
                                    3664
                  013
                  9341E7009
U56
U55

62256 62256      514260  514260     U64         M6295

***************************************************************************/



#define ROMS_MAZINGER \
	ROM_REGION( 0x80000, "maincpu", 0 ) \
	ROM_LOAD16_WORD_SWAP( "mzp-0.u24", 0x00000, 0x80000, CRC(43a4279f) SHA1(2c17eb31040bb7f1554bc1c9a968eec5e72af097) ) \
	\
	ROM_REGION16_BE( 0x80000, "user1", 0 ) \
	ROM_LOAD16_WORD_SWAP( "mzp-1.924", 0x00000, 0x80000, CRC(db40acba) SHA1(797a3046b6ab33773c5c4d6bb6d045ea60c1eb45) ) \
	\
	ROM_REGION( 0x28000, "audiocpu", 0 ) \
	ROM_LOAD( "mzs.u21", 0x00000, 0x08000, CRC(c5b4f7ed) SHA1(01f3cd1dd4045029260544e0e1c15dd08817012e) ) \
	ROM_CONTINUE(        0x10000, 0x18000             ) \
	\
	ROM_REGION( 0x400000 * 2, "sprites", ROMREGION_ERASEFF ) \
	ROM_LOAD( "bp943a-2.u56", 0x000000, 0x200000, CRC(97e13959) SHA1(c30b1093aacebafefcae701af767dd36fc55fac7) ) \
	ROM_LOAD( "bp943a-3.u55", 0x200000, 0x080000, CRC(9c4957dd) SHA1(e775605a01b6cadc318855ac046dad03c4fc5bb4) ) \
	\
	ROM_REGION( 0x200000, "layer0", 0 ) \
	ROM_LOAD( "bp943a-1.u60", 0x000000, 0x200000, CRC(46327415) SHA1(679d26caefa975569198fac550105c370e2be00d) ) \
	\
	ROM_REGION( 0x200000, "layer1", 0 ) \
	ROM_LOAD( "bp943a-0.u63", 0x000000, 0x200000, CRC(c1fed98a) SHA1(c276505f80a49b129862966a19db507f97153e45) ) \
	\
	ROM_REGION( 0x0c0000, "oki", 0 ) \
	ROM_LOAD( "bp943a-4.u64", 0x040000, 0x080000, CRC(3fc7f29a) SHA1(feb21b918243c0a03dfa4a80cc80b86be4f62680) ) \

/* the regions differ only in the EEPROM, hence the macro above - all EEPROMs are Factory Defaulted */
ROM_START( mazinger )
	ROMS_MAZINGER

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "mazinger_world.nv", 0x0000, 0x0080, CRC(4f6225c6) SHA1(ed8e1c3ca9b961778cd317deb0dd8a0143eaab4f) )
ROM_END

ROM_START( mazingerj )
	ROMS_MAZINGER

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "mazinger_japan.nv", 0x0000, 0x0080, CRC(f84a2a45) SHA1(2e8ad10994bba9a3952889ed0ec3bcaca9b41d03) )
ROM_END

/***************************************************************************

             Metamoqester (World) / Oni - The Ninja Master (Japan)

(C) 1995 Banpresto

PCB: BP947A
CPU: MC68HC000P16 (68000, 64 pin DIP)
SND: Z0840008PSC (Z80, 40 pin DIP), AD-65 x 2 (= OKI M6295), YM2151, CY5002 (= YM3012)
OSC: 32.000 MHz
RAM: LGS GM76C88ALFW-15 x 9 (28 pin SOP), LGS GM71C4260AJ70 x 2 (40 pin SOJ)
     Hitachi HM62256LFP-12T x 2 (40 pin SOJ)

Other Chips:
AT93C46 (EEPROM)
PAL (not dumped, located near 68000): ATF16V8 x 1

GFX:  (Same GFX chips as "Sailor Moon")

      038 9437WX711 (176 pin PQFP)
      038 9437WX711 (176 pin PQFP)
      038 9437WX711 (176 pin PQFP)
      013 9346E7002 (240 pin PQFP)

On PCB near JAMMA connector is a small push button labelled SW1 to access test mode.

ROMS:
BP947A.U37  16M Mask    \ Oki Samples
BP947A.U42  16M Mask    /

BP947A.U46  16M Mask    \
BP947A.U47  16M Mask    |
BP947A.U48  16M Mask    |
BP947A.U49  16M Mask    | GFX
BP947A.U50  16M Mask    |
BP947A.U51  16M Mask    |
BP947A.U52  16M Mask    /

BP947A.U20  27C020        Sound PRG

BP947A.U25  27C240      \
BP947A.U28  27C240      | Main PRG
BP947A.U29  27C240      /

***************************************************************************/

ROM_START( metmqstr )
	ROM_REGION( 0x280000, "maincpu", 0 )		/* 68000 code */
	ROM_LOAD16_WORD_SWAP( "bp947a.u25", 0x000000, 0x80000, CRC(0a5c3442) SHA1(684b79912dedc103f45c42fdebf9983e091b1308) )
	ROM_LOAD16_WORD_SWAP( "bp947a.u28", 0x100000, 0x80000, CRC(8c55decf) SHA1(76c6ce4c8e621273258d31ceb9ec4442fcf1a393) )
	ROM_LOAD16_WORD_SWAP( "bp947a.u29", 0x200000, 0x80000, CRC(cf0f3f3b) SHA1(49a3c0e7536edd53bbf09353e43e9166d736b3f4) )

	ROM_REGION( 0x48000, "audiocpu", 0 )		/* Z80 code */
	ROM_LOAD( "bp947a.u20",  0x00000, 0x08000, CRC(a4a36170) SHA1(ae55094518bd968ea0d04613a133c1421e412012) )
	ROM_CONTINUE(            0x10000, 0x38000             )

	ROM_REGION( 0x800000 * 2, "sprites", 0 )		/* Sprites */
	ROM_LOAD( "bp947a.u49", 0x000000, 0x200000, CRC(09749531) SHA1(6deeed2712241611ec3202c49a66beed28698af8) )
	ROM_LOAD( "bp947a.u50", 0x200000, 0x200000, CRC(19cea8b2) SHA1(87fb29458074f0e4852237e0184b8b3b44b0eb29) )
	ROM_LOAD( "bp947a.u51", 0x400000, 0x200000, CRC(c19bed67) SHA1(ac664a15512c0e8c8b701833aede95f53cd46a45) )
	ROM_LOAD( "bp947a.u52", 0x600000, 0x200000, CRC(70c64875) SHA1(1c20ab100ccfdf42c97a25e4deb9041b83f5ca8d) )

	ROM_REGION( 0x100000, "layer0", 0 )	/* Layer 0 */
	ROM_LOAD( "bp947a.u48", 0x000000, 0x100000, CRC(04ff6a3d) SHA1(7187db436f7a2ab59a3f5c6ab297b3d740e20f1d) )	// FIRST AND SECOND HALF IDENTICAL
	ROM_CONTINUE(           0x000000, 0x100000             )

	ROM_REGION( 0x100000, "layer1", 0 )	/* Layer 1 */
	ROM_LOAD( "bp947a.u47", 0x000000, 0x100000, CRC(0de42827) SHA1(05d452ca11a31f941cb8a9b0cbb0b59c6b0cbdcb) )	// FIRST AND SECOND HALF IDENTICAL
	ROM_CONTINUE(           0x000000, 0x100000             )

	ROM_REGION( 0x100000, "layer2", 0 )	/* Layer 2 */
	ROM_LOAD( "bp947a.u46", 0x000000, 0x100000, CRC(0f9c906e) SHA1(03872e8be28637df66373bddb04ed91de4f9db75) )	// FIRST AND SECOND HALF IDENTICAL
	ROM_CONTINUE(           0x000000, 0x100000             )

	ROM_REGION( 0x140000, "oki1", 0 )	/* OKIM6295 #1 Samples */
	/* Leave the 0x40000 bytes addressable by the chip empty */
	ROM_LOAD( "bp947a.u42", 0x040000, 0x100000, CRC(2ce8ff2a) SHA1(8ef8c5b7d4a0e60c980c2962e75f7977faafa311) )	// FIRST AND SECOND HALF IDENTICAL
	ROM_CONTINUE(           0x040000, 0x100000             )

	ROM_REGION( 0x140000, "oki2", 0 )	/* OKIM6295 #2 Samples */
	/* Leave the 0x40000 bytes addressable by the chip empty */
	ROM_LOAD( "bp947a.u37", 0x040000, 0x100000, CRC(c3077c8f) SHA1(0a76316a81b7de78279b859549eb5161a721ac71) )	// FIRST AND SECOND HALF IDENTICAL
	ROM_CONTINUE(           0x040000, 0x100000             )
ROM_END

ROM_START( nmaster )
	ROM_REGION( 0x280000, "maincpu", 0 )		/* 68000 code */
	ROM_LOAD16_WORD_SWAP( "bp947a_n.u25",0x000000, 0x80000, CRC(748cc514) SHA1(11d882e77a539407c314f087386e50d691a6bc0b) )
	ROM_LOAD16_WORD_SWAP( "bp947a.u28" , 0x100000, 0x80000, CRC(8c55decf) SHA1(76c6ce4c8e621273258d31ceb9ec4442fcf1a393) )
	ROM_LOAD16_WORD_SWAP( "bp947a.u29",  0x200000, 0x80000, CRC(cf0f3f3b) SHA1(49a3c0e7536edd53bbf09353e43e9166d736b3f4) )

	ROM_REGION( 0x48000, "audiocpu", 0 )		/* Z80 code */
	ROM_LOAD( "bp947a.u20",  0x00000, 0x08000, CRC(a4a36170) SHA1(ae55094518bd968ea0d04613a133c1421e412012) )
	ROM_CONTINUE(            0x10000, 0x38000             )

	ROM_REGION( 0x800000 * 2, "sprites", 0 )		/* Sprites */
	ROM_LOAD( "bp947a.u49", 0x000000, 0x200000, CRC(09749531) SHA1(6deeed2712241611ec3202c49a66beed28698af8) )
	ROM_LOAD( "bp947a.u50", 0x200000, 0x200000, CRC(19cea8b2) SHA1(87fb29458074f0e4852237e0184b8b3b44b0eb29) )
	ROM_LOAD( "bp947a.u51", 0x400000, 0x200000, CRC(c19bed67) SHA1(ac664a15512c0e8c8b701833aede95f53cd46a45) )
	ROM_LOAD( "bp947a.u52", 0x600000, 0x200000, CRC(70c64875) SHA1(1c20ab100ccfdf42c97a25e4deb9041b83f5ca8d) )

	ROM_REGION( 0x100000, "layer0", 0 )	/* Layer 0 */
	ROM_LOAD( "bp947a.u48", 0x000000, 0x100000, CRC(04ff6a3d) SHA1(7187db436f7a2ab59a3f5c6ab297b3d740e20f1d) )	// FIRST AND SECOND HALF IDENTICAL
	ROM_CONTINUE(           0x000000, 0x100000             )

	ROM_REGION( 0x100000, "layer1", 0 )	/* Layer 1 */
	ROM_LOAD( "bp947a.u47", 0x000000, 0x100000, CRC(0de42827) SHA1(05d452ca11a31f941cb8a9b0cbb0b59c6b0cbdcb) )	// FIRST AND SECOND HALF IDENTICAL
	ROM_CONTINUE(           0x000000, 0x100000             )

	ROM_REGION( 0x100000, "layer2", 0 )	/* Layer 2 */
	ROM_LOAD( "bp947a.u46", 0x000000, 0x100000, CRC(0f9c906e) SHA1(03872e8be28637df66373bddb04ed91de4f9db75) )	// FIRST AND SECOND HALF IDENTICAL
	ROM_CONTINUE(           0x000000, 0x100000             )

	ROM_REGION( 0x140000, "oki1", 0 )	/* OKIM6295 #1 Samples */
	/* Leave the 0x40000 bytes addressable by the chip empty */
	ROM_LOAD( "bp947a.u42", 0x040000, 0x100000, CRC(2ce8ff2a) SHA1(8ef8c5b7d4a0e60c980c2962e75f7977faafa311) )	// FIRST AND SECOND HALF IDENTICAL
	ROM_CONTINUE(           0x040000, 0x100000             )

	ROM_REGION( 0x140000, "oki2", 0 )	/* OKIM6295 #2 Samples */
	/* Leave the 0x40000 bytes addressable by the chip empty */
	ROM_LOAD( "bp947a.u37", 0x040000, 0x100000, CRC(c3077c8f) SHA1(0a76316a81b7de78279b859549eb5161a721ac71) )	// FIRST AND SECOND HALF IDENTICAL
	ROM_CONTINUE(           0x040000, 0x100000             )
ROM_END


/***************************************************************************

           Power Instinct 2 (USA) / Gouketsuji Ichizoku 2 (Japan)

(c)1994 Atlus
CPU: 68000, Z80
Sound: YM2203, AR17961 (x2)
Custom: NMK 112 (M6295 sample ROM banking), Atlus 8647-01  013, 038 (x4)
X1 = 12 MHz
X2 = 28 MHz
X3 = 16 MHz

***************************************************************************/

ROM_START( pwrinst2 )
	ROM_REGION( 0x200000, "maincpu", 0 )		/* 68000 code */
	ROM_LOAD16_BYTE( "g02.u45", 0x000000, 0x80000, CRC(7b33bc43) SHA1(a68eb94e679f03c354932b8c5cd1bb2922fec0aa) )
	ROM_LOAD16_BYTE( "g02.u44", 0x000001, 0x80000, CRC(8f6f6637) SHA1(024b12c0fe40e27c79e38bd7601a9183a62d75fd) )
	ROM_LOAD16_BYTE( "g02.u43", 0x100000, 0x80000, CRC(178e3d24) SHA1(926234f4196a5d5e3bd1438abbf73355f2c65b06) )
	ROM_LOAD16_BYTE( "g02.u42", 0x100001, 0x80000, CRC(a0b4ee99) SHA1(c6df4aa2543b04d8bda7683f503e5eb763e506af) )

	ROM_REGION16_BE( 0x100000, "user1", ROMREGION_ERASE00 )	/* 68000 extra data roms */
	/* not used */

	ROM_REGION( 0x24000, "audiocpu", 0 )		/* Z80 code */
	ROM_LOAD( "g02.u3a", 0x00000, 0x0c000, CRC(ebea5e1e) SHA1(4d3af9e5f29d0c1b26563f51250039c9e8bd3735) )
	ROM_CONTINUE(        0x10000, 0x14000             )

	ROM_REGION( 0xe00000 * 2, "sprites", 0 )		/* Sprites */
	ROM_LOAD( "g02.u61", 0x000000, 0x200000, CRC(91e30398) SHA1(2b59a5e40bed2a988382054fe30d92808dad3348) )
	ROM_LOAD( "g02.u62", 0x200000, 0x200000, CRC(d9455dd7) SHA1(afa69fe9a540cd78b8cfecf09cffa1401c01141a) )
	ROM_LOAD( "g02.u63", 0x400000, 0x200000, CRC(4d20560b) SHA1(ceaee8cf0b69cc366b95ddcb689a5594d79e5114) )
	ROM_LOAD( "g02.u64", 0x600000, 0x200000, CRC(b17b9b6e) SHA1(fc6213d8322cda4c7f653e2d7d6d314ce84c97b7) )
	ROM_LOAD( "g02.u65", 0x800000, 0x200000, CRC(08541878) SHA1(138cf077a49a26440a3da1bdc2c399a208359e57) )
	ROM_LOAD( "g02.u66", 0xa00000, 0x200000, CRC(becf2a36) SHA1(f8b386d0292b1dc745b7253a3df51d1aa8d5e9db) )
	ROM_LOAD( "g02.u67", 0xc00000, 0x200000, CRC(52fe2b8b) SHA1(dd50aa62f7db995e28f47de9b3fb749aeeaaa5b0) )

	ROM_REGION( 0x200000, "layer0", 0 )	/* Layer 0 */
	ROM_LOAD( "g02.u78", 0x000000, 0x200000, CRC(1eca63d2) SHA1(538942b43301f950e3d5139461331c54dc90129d) )

	ROM_REGION( 0x100000, "layer1", 0 )	/* Layer 1 */
	ROM_LOAD( "g02.u81", 0x000000, 0x100000, CRC(8a3ff685) SHA1(4a59ec50ec4470453374fe10f76d3e894494b49f) )

	ROM_REGION( 0x100000, "layer2", 0 )	/* Layer 2 */
	ROM_LOAD( "g02.u89", 0x000000, 0x100000, CRC(373e1f73) SHA1(ec1ae9fab37eee41be8e1bc6dad03809b62fdbce) )

	ROM_REGION( 0x080000, "layer3", 0 )	/* Layer 3 */
	ROM_LOAD( "g02.82a", 0x000000, 0x080000, CRC(4b3567d6) SHA1(d3e14783b312d2bea9722a8e3c22bcec81e26166) )

	ROM_REGION( 0x440000, "oki1", 0 )	/* OKIM6295 #1 Samples */
	/* Leave the 0x40000 bytes addressable by the chip empty */
	ROM_LOAD( "g02.u53", 0x040000, 0x200000, CRC(c4bdd9e0) SHA1(a938a831e789ddf6f3cc5f3e5f3877ec7bd62d4e) )
	ROM_LOAD( "g02.u54", 0x240000, 0x200000, CRC(1357d50e) SHA1(433766177ce9d6933f90de85ba91bfc6d8d5d664) )

	ROM_REGION( 0x440000, "oki2", 0 )	/* OKIM6295 #2 Samples */
	/* Leave the 0x40000 bytes addressable by the chip empty */
	ROM_LOAD( "g02.u55", 0x040000, 0x200000, CRC(2d102898) SHA1(bd81f4cd2ba100707db0c5bb1419f0b23c998574) )
	ROM_LOAD( "g02.u56", 0x240000, 0x200000, CRC(9ff50dda) SHA1(1121685e387c20e228032f2b0f5cbb606376fc15) )
ROM_END

ROM_START( pwrinst2j )
	ROM_REGION( 0x200000, "maincpu", 0 )		/* 68000 code */
	ROM_LOAD16_BYTE( "g02j.u45", 0x000000, 0x80000, CRC(42d0abd7) SHA1(c58861d43c4539ccc8b2f93eabc56aab37d3aa34))
	ROM_LOAD16_BYTE( "g02j.u44", 0x000001, 0x80000, CRC(362b7af3) SHA1(2d15611530cef76f0f9c82ee0411966079ae19c3))
	ROM_LOAD16_BYTE( "g02j.u43", 0x100000, 0x80000, CRC(c94c596b) SHA1(ee755a344f769e3ed05d8ca57f517b9e8c02f22e) )
	ROM_LOAD16_BYTE( "g02j.u42", 0x100001, 0x80000, CRC(4f4c8270) SHA1(1fa964f5646bd1d078e3661c21e191b0789c05c9) )

	ROM_REGION16_BE( 0x100000, "user1", ROMREGION_ERASE00 )	/* 68000 extra data roms */
	/* not used */

	ROM_REGION( 0x24000, "audiocpu", 0 )		/* Z80 code */
	ROM_LOAD( "g02j.u3a", 0x00000, 0x0c000, CRC(eead01f1) SHA1(0ced6755e471e0303fe397b3d54a5c799762ebd8) )
	ROM_CONTINUE(        0x10000, 0x14000             )

	ROM_REGION( 0xe00000 * 2, "sprites", 0 )		/* Sprites */
	ROM_LOAD( "g02.u61", 0x000000, 0x200000, CRC(91e30398) SHA1(2b59a5e40bed2a988382054fe30d92808dad3348) )
	ROM_LOAD( "g02.u62", 0x200000, 0x200000, CRC(d9455dd7) SHA1(afa69fe9a540cd78b8cfecf09cffa1401c01141a) )
	ROM_LOAD( "g02.u63", 0x400000, 0x200000, CRC(4d20560b) SHA1(ceaee8cf0b69cc366b95ddcb689a5594d79e5114) )
	ROM_LOAD( "g02.u64", 0x600000, 0x200000, CRC(b17b9b6e) SHA1(fc6213d8322cda4c7f653e2d7d6d314ce84c97b7) )
	ROM_LOAD( "g02.u65", 0x800000, 0x200000, CRC(08541878) SHA1(138cf077a49a26440a3da1bdc2c399a208359e57) )
	ROM_LOAD( "g02.u66", 0xa00000, 0x200000, CRC(becf2a36) SHA1(f8b386d0292b1dc745b7253a3df51d1aa8d5e9db) )
	ROM_LOAD( "g02.u67", 0xc00000, 0x200000, CRC(52fe2b8b) SHA1(dd50aa62f7db995e28f47de9b3fb749aeeaaa5b0) )

	ROM_REGION( 0x200000, "layer0", 0 )	/* Layer 0 */
	ROM_LOAD( "g02.u78", 0x000000, 0x200000, CRC(1eca63d2) SHA1(538942b43301f950e3d5139461331c54dc90129d) )

	ROM_REGION( 0x100000, "layer1", 0 )	/* Layer 1 */
	ROM_LOAD( "g02.u81", 0x000000, 0x100000, CRC(8a3ff685) SHA1(4a59ec50ec4470453374fe10f76d3e894494b49f) )

	ROM_REGION( 0x100000, "layer2", 0 )	/* Layer 2 */
	ROM_LOAD( "g02.u89", 0x000000, 0x100000, CRC(373e1f73) SHA1(ec1ae9fab37eee41be8e1bc6dad03809b62fdbce) )

	ROM_REGION( 0x080000, "layer3", 0 )	/* Layer 3 */
	ROM_LOAD( "g02j.82a", 0x000000, 0x080000, CRC(3be86fe1) SHA1(313bfe5fb8dc5fee4462db259738e079759f9390) )

	ROM_REGION( 0x440000, "oki1", 0 )	/* OKIM6295 #1 Samples */
	/* Leave the 0x40000 bytes addressable by the chip empty */
	ROM_LOAD( "g02.u53", 0x040000, 0x200000, CRC(c4bdd9e0) SHA1(a938a831e789ddf6f3cc5f3e5f3877ec7bd62d4e) )
	ROM_LOAD( "g02.u54", 0x240000, 0x200000, CRC(1357d50e) SHA1(433766177ce9d6933f90de85ba91bfc6d8d5d664) )

	ROM_REGION( 0x440000, "oki2", 0 )	/* OKIM6295 #2 Samples */
	/* Leave the 0x40000 bytes addressable by the chip empty */
	ROM_LOAD( "g02.u55", 0x040000, 0x200000, CRC(2d102898) SHA1(bd81f4cd2ba100707db0c5bb1419f0b23c998574) )
	ROM_LOAD( "g02.u56", 0x240000, 0x200000, CRC(9ff50dda) SHA1(1121685e387c20e228032f2b0f5cbb606376fc15) )
ROM_END

/*

Power Instinct Legends (US) / Gouketsuji Ichizoku Saikyou Densetsu (Japan)
Atlus, 1995

PCB Layout
----------

AT047G2-B ATLUS
|---------------------------------------------------------------|
|LM324 M6295  G02_U53          Z80  16MHz 28MHz 12MHz  TA8030S  |
|VOL          G02_U54 |------| SOUND_U3                TEXT_U82 |
|      M6295  G02_U55 |NMK112|   6264         6264              |
|uPC2505      G02_U56 |      |                        |------|  |
|      4558           |------|                6264    |038   |  |
|     Y3014   YM2203    PAL                           |9429WX|  |
|                                            ATGS_U89 |------|  |
|J       TEST_SW  62256                                         |
|A    93C46       62256                       6264    |------|  |
|M            |----SUB-BOARD-----|                    |038   |  |
|M    |---|   |*P  P *P  P *P *P |      PAL   6264    |9429WX|  |
|A    |   |   | R  R  R  R  R  R |                    |------|  |
|     | 6 |   | 1  O  1  O  1  1 |           ATGS_U81           |
|     | 8 |   | 2  G  2  G  2  2 |                    |------|  |
|     | 0 |   | U  U  U  U  U  U |            6264    |038   |  |
|     | 0 |   | 2  4  4  4  3  5 |                    |9429WX|  |
|     | 0 |   |    5     4       |62256       6264    |------|  |
|     |   |   |------------------|62256                         |
|     |---|     PAL            |-------|     ATGS_U78 |------|  |
|--------|                     |8647-01|              |038   |  |
|*ATGS_U1|                     |013    |    KM416C256 |9429WX|  |
|        |                     |9341E70|              |------|  |
|        |G02_U66    G02_U63   |-------|                6264    |
|        |  G02_U65    G02_U62   62256      KM416C256           |
|*ATGS_U2|    G02_U64    G02_U61 62256                  6264    |
|--------|------------------------------------------------------|
Notes:
      ROMs marked with * are located on a plug-in sub board
      68000 clock - 16.000MHz
      Z80 clock   - 8.000MHz [16/2]
      6295 clocks - 3.000MHz [12/4], sample rate = 3000000 / 165
      YM2203 clock- 4.000MHz [16/4]
      VSync       - 57.5Hz
      HSync       - 15.23kHz

      ROMs -
            U3       : 27C1001 EPROM
            U82      : 27C040 EPROM
            PR12*    : 27C040 EPROMs
            PROG*    : 27C040 EPROMs
            ALL other ROMs are soldered-in 16M 42 pin MASKROM (read as 27C160)
*/

ROM_START( plegends )
	ROM_REGION( 0x200000, "maincpu", 0 )		/* 68000 code */
	ROM_LOAD16_BYTE( "d12.u45", 0x000000, 0x80000, CRC(ed8a2e3d) SHA1(0a09c58cd8a726189cd7679d06343e0b8c3de945) )
	ROM_LOAD16_BYTE( "d13.u44", 0x000001, 0x80000, CRC(25821731) SHA1(7c6ece92b36dc7eb489879d9ae3e8af9380b9f62) )
	ROM_LOAD16_BYTE( "d14.u2",  0x100000, 0x80000, CRC(c2cb1402) SHA1(78e70915ca32b97c22605a304dc8611e1fe01ae9) ) /* Contains text strings */
	ROM_LOAD16_BYTE( "d16.u3",  0x100001, 0x80000, CRC(50a1c63e) SHA1(5a8431a81aa61034e67141944b9e7cf97842773a) ) /* Contains text strings */

	ROM_REGION16_BE( 0x100000, "user1", 0 )	/* 68000 extra data roms */
	ROM_LOAD16_BYTE( "d15.u4",  0x000000, 0x80000, CRC(6352cec0) SHA1(a54d55b8d642e438158268d0d41880b6589e48e2) )
	ROM_LOAD16_BYTE( "d17.u5",  0x000001, 0x80000, CRC(7af810d8) SHA1(5e24f78a228809a001f3f3372c1b32ea05070e17) )

	ROM_REGION( 0x44000, "audiocpu", 0 )		/* Z80 code */
	ROM_LOAD( "d19.u3", 0x00000, 0x0c000, CRC(47598459) SHA1(4e9dcfebfbd160230768965e8c6e5ed446c1aa7b) ) /* Same as sound.u3 below, but twice the size? */
	ROM_CONTINUE(        0x10000, 0x34000             )

	ROM_REGION( 0x1000000 * 2, "sprites", 0 )		/* Sprites */
	ROM_LOAD( "g02.u61", 0x000000, 0x200000, CRC(91e30398) SHA1(2b59a5e40bed2a988382054fe30d92808dad3348) )
	ROM_LOAD( "g02.u62", 0x200000, 0x200000, CRC(d9455dd7) SHA1(afa69fe9a540cd78b8cfecf09cffa1401c01141a) )
	ROM_LOAD( "g02.u63", 0x400000, 0x200000, CRC(4d20560b) SHA1(ceaee8cf0b69cc366b95ddcb689a5594d79e5114) )
	ROM_LOAD( "g02.u64", 0x600000, 0x200000, CRC(b17b9b6e) SHA1(fc6213d8322cda4c7f653e2d7d6d314ce84c97b7) )
	ROM_LOAD( "g02.u65", 0x800000, 0x200000, CRC(08541878) SHA1(138cf077a49a26440a3da1bdc2c399a208359e57) )
	ROM_LOAD( "g02.u66", 0xa00000, 0x200000, CRC(becf2a36) SHA1(f8b386d0292b1dc745b7253a3df51d1aa8d5e9db) )
	ROM_LOAD( "atgs.u1", 0xc00000, 0x200000, CRC(aa6f34a9) SHA1(00de85de1b413bd2c46931c13365f8556b50b634) ) /* US version's rom labeled "sp6_u67-1" */
	ROM_LOAD( "atgs.u2", 0xe00000, 0x200000, CRC(553eda27) SHA1(5b9126f966f0c64b3ac7c06526064d71e4df60c5) ) /* US version's rom labeled "sp6_u67-2" */

	ROM_REGION( 0x200000, "layer0", 0 )	/* Layer 0 */
	ROM_LOAD( "atgs.u78", 0x000000, 0x200000, CRC(16710ecb) SHA1(6277f7f6095457df649932550b04242e5853ec5e) ) /* US version's rom labeled "bg0_u78" */

	ROM_REGION( 0x200000, "layer1", 0 )	/* Layer 1 */
	ROM_LOAD( "atgs.u81", 0x000000, 0x200000, CRC(cb2aca91) SHA1(869f0f2db35c45ec90b74d33d521cbb598e60a3f) ) /* US version's rom labeled "bg1_u81" */

	ROM_REGION( 0x200000, "layer2", 0 )	/* Layer 2 */
	ROM_LOAD( "atgs.u89", 0x000000, 0x200000, CRC(65f45a0f) SHA1(b7f4b56308dcdc144100d0a92d91255459a320a4) ) /* US version's rom labeled "bg2_u89" */

	ROM_REGION( 0x080000, "layer3", 0 )	/* Layer 3 */
	ROM_LOAD( "text.u82", 0x000000, 0x080000, CRC(f57333ea) SHA1(409d8005ffcf91943e4a743b2434ce425f5bdc36) ) /* US version's rom labeled "d20" */

	ROM_REGION( 0x440000, "oki1", 0 )	/* OKIM6295 #1 Samples */
	/* Leave the 0x40000 bytes addressable by the chip empty */
	ROM_LOAD( "g02.u53", 0x040000, 0x200000, CRC(c4bdd9e0) SHA1(a938a831e789ddf6f3cc5f3e5f3877ec7bd62d4e) )
	ROM_LOAD( "g02.u54", 0x240000, 0x200000, CRC(1357d50e) SHA1(433766177ce9d6933f90de85ba91bfc6d8d5d664) )

	ROM_REGION( 0x440000, "oki2", 0 )	/* OKIM6295 #2 Samples */
	/* Leave the 0x40000 bytes addressable by the chip empty */
	ROM_LOAD( "g02.u55", 0x040000, 0x200000, CRC(2d102898) SHA1(bd81f4cd2ba100707db0c5bb1419f0b23c998574) )
	ROM_LOAD( "g02.u56", 0x240000, 0x200000, CRC(9ff50dda) SHA1(1121685e387c20e228032f2b0f5cbb606376fc15) )
ROM_END

ROM_START( plegendsj )
	ROM_REGION( 0x200000, "maincpu", 0 )		/* 68000 code */
	ROM_LOAD16_BYTE( "prog.u45", 0x000000, 0x80000, CRC(94f53db2) SHA1(34c671f160cfcb7d46cc964731ff2b77dc0be928) )
	ROM_LOAD16_BYTE( "prog.u44", 0x000001, 0x80000, CRC(db0ad756) SHA1(9c1510491cdc9442062ee3bd8a1bb93f00d33d97) )
	ROM_LOAD16_BYTE( "pr12.u2",  0x100000, 0x80000, CRC(0e202559) SHA1(217a8e47d5c679aff02ca43de1641230e4f78b01) ) /* Contains text in Japanese */
	ROM_LOAD16_BYTE( "pr12.u3",  0x100001, 0x80000, CRC(54742f21) SHA1(fae7bb7381478eb077f0409acd521f77417aa968) ) /* Contains text in Japanese */

	ROM_REGION16_BE( 0x100000, "user1", 0 )	/* 68000 extra data roms */
	ROM_LOAD16_BYTE( "d15.u4",  0x000000, 0x80000, CRC(6352cec0) SHA1(a54d55b8d642e438158268d0d41880b6589e48e2) )
	ROM_LOAD16_BYTE( "d17.u5",  0x000001, 0x80000, CRC(7af810d8) SHA1(5e24f78a228809a001f3f3372c1b32ea05070e17) )

	ROM_REGION( 0x24000, "audiocpu", 0 )		/* Z80 code */
	ROM_LOAD( "sound.u3", 0x00000, 0x0c000, CRC(36f71520) SHA1(11d0a059ddba3e1aa4c54ccdde7b3f5c7bde482f) )
	ROM_CONTINUE(        0x10000, 0x14000             )

	ROM_REGION( 0x1000000 * 2, "sprites", 0 )		/* Sprites */
	ROM_LOAD( "g02.u61", 0x000000, 0x200000, CRC(91e30398) SHA1(2b59a5e40bed2a988382054fe30d92808dad3348) )
	ROM_LOAD( "g02.u62", 0x200000, 0x200000, CRC(d9455dd7) SHA1(afa69fe9a540cd78b8cfecf09cffa1401c01141a) )
	ROM_LOAD( "g02.u63", 0x400000, 0x200000, CRC(4d20560b) SHA1(ceaee8cf0b69cc366b95ddcb689a5594d79e5114) )
	ROM_LOAD( "g02.u64", 0x600000, 0x200000, CRC(b17b9b6e) SHA1(fc6213d8322cda4c7f653e2d7d6d314ce84c97b7) )
	ROM_LOAD( "g02.u65", 0x800000, 0x200000, CRC(08541878) SHA1(138cf077a49a26440a3da1bdc2c399a208359e57) )
	ROM_LOAD( "g02.u66", 0xa00000, 0x200000, CRC(becf2a36) SHA1(f8b386d0292b1dc745b7253a3df51d1aa8d5e9db) )
	ROM_LOAD( "atgs.u1", 0xc00000, 0x200000, CRC(aa6f34a9) SHA1(00de85de1b413bd2c46931c13365f8556b50b634) ) /* US version's rom labeled "sp6_u67-1" */
	ROM_LOAD( "atgs.u2", 0xe00000, 0x200000, CRC(553eda27) SHA1(5b9126f966f0c64b3ac7c06526064d71e4df60c5) ) /* US version's rom labeled "sp6_u67-2" */

	ROM_REGION( 0x200000, "layer0", 0 )	/* Layer 0 */
	ROM_LOAD( "atgs.u78", 0x000000, 0x200000, CRC(16710ecb) SHA1(6277f7f6095457df649932550b04242e5853ec5e) ) /* US version's rom labeled "bg0_u78" */

	ROM_REGION( 0x200000, "layer1", 0 )	/* Layer 1 */
	ROM_LOAD( "atgs.u81", 0x000000, 0x200000, CRC(cb2aca91) SHA1(869f0f2db35c45ec90b74d33d521cbb598e60a3f) ) /* US version's rom labeled "bg1_u81" */

	ROM_REGION( 0x200000, "layer2", 0 )	/* Layer 2 */
	ROM_LOAD( "atgs.u89", 0x000000, 0x200000, CRC(65f45a0f) SHA1(b7f4b56308dcdc144100d0a92d91255459a320a4) ) /* US version's rom labeled "bg2_u89" */

	ROM_REGION( 0x080000, "layer3", 0 )	/* Layer 3 */
	ROM_LOAD( "text.u82", 0x000000, 0x080000, CRC(f57333ea) SHA1(409d8005ffcf91943e4a743b2434ce425f5bdc36) ) /* US version's rom labeled "d20" */

	ROM_REGION( 0x440000, "oki1", 0 )	/* OKIM6295 #1 Samples */
	/* Leave the 0x40000 bytes addressable by the chip empty */
	ROM_LOAD( "g02.u53", 0x040000, 0x200000, CRC(c4bdd9e0) SHA1(a938a831e789ddf6f3cc5f3e5f3877ec7bd62d4e) )
	ROM_LOAD( "g02.u54", 0x240000, 0x200000, CRC(1357d50e) SHA1(433766177ce9d6933f90de85ba91bfc6d8d5d664) )

	ROM_REGION( 0x440000, "oki2", 0 )	/* OKIM6295 #2 Samples */
	/* Leave the 0x40000 bytes addressable by the chip empty */
	ROM_LOAD( "g02.u55", 0x040000, 0x200000, CRC(2d102898) SHA1(bd81f4cd2ba100707db0c5bb1419f0b23c998574) )
	ROM_LOAD( "g02.u56", 0x240000, 0x200000, CRC(9ff50dda) SHA1(1121685e387c20e228032f2b0f5cbb606376fc15) )
ROM_END


/***************************************************************************

                         Pretty Soldier Sailor Moon

(C) 1995 Banpresto
PCB: BP945A
CPU: TMP68HC000P16 (68000, 64 pin DIP)
SND: Z84C0008PEC (Z80, 40 pin DIP), OKI M6295 x 2, YM2151, YM3012
OSC: 28.000MHz, 16.000MHz
RAM: NEC 43256 x 8, NEC 424260 x 2, Sanyo LC3664 x 5

Other Chips:
SGS Thomson ST93C46CB1 (EEPROM?)
PALS (not dumped):
      18CV8 label SMBG
      18CV8 label SMZ80
      18CV8 label SMCPU
      GAL16V8 (located near BPSM-U47)

GFX:  038 9437WX711 (176 pin PQFP)
      038 9437WX711 (176 pin PQFP)
      038 9437WX711 (176 pin PQFP)
      013 9346E7002 (240 pin PQFP)

On PCB near JAMMA connector is a small push button to access test mode.

ROMS:
BP945A.U9   27C040      Sound Program
BP945A.U45  27C240      Main Program
BPSM.U46    23C16000    Main Program?
BPSM.U47    23C4000     Sound?
BPSM.U48    23C16000    Sound?
BPSM.U53    23C16000    GFX
BPSM.U54    23C16000    GFX
BPSM.U57    23C16000    GFX
BPSM.U58    23C16000    GFX
BPSM.U59    23C16000    GFX
BPSM.U60    23C16000    GFX
BPSM.U61    23C16000    GFX
BPSM.U62    23C16000    GFX
BPSM.U63    23C16000    GFX
BPSM.U64    23C16000    GFX
BPSM.U65    23C16000    GFX
BPSM.U76    23C16000    GFX
BPSM.U77    23C16000    GFX

***************************************************************************/



#define ROMS_SAILORMN \
	ROM_REGION( 0x400000, "maincpu", 0 ) \
	ROM_LOAD16_WORD_SWAP( "bpsm945a.u45", 0x000000, 0x080000, CRC(898c9515) SHA1(0fe8d7f13f5cfe2f6e79a0a21b2e8e7e70e65c4b) ) \
	ROM_LOAD16_WORD_SWAP( "bpsm.u46",     0x200000, 0x200000, CRC(32084e80) SHA1(0ac503190d95009620b5ad7e7e0e63324f6fa4eb) ) \
	\
	ROM_REGION( 0x88000, "audiocpu", 0 ) \
	ROM_LOAD( "bpsm945a.u9",  0x00000, 0x08000, CRC(438de548) SHA1(81a0ca1cd662e2017aa980da162d39cfd0a19f14) ) \
	ROM_CONTINUE(             0x10000, 0x78000             ) \
	\
	ROM_REGION( 0x400000 * 2, "sprites", 0 ) \
	ROM_LOAD( "bpsm.u76", 0x000000, 0x200000, CRC(a243a5ba) SHA1(3a32d685e53e0b75977f7acb187cf414a50c7f8b) ) \
	ROM_LOAD( "bpsm.u77", 0x200000, 0x200000, CRC(5179a4ac) SHA1(ceb8d3d889aae885debb2c9cf2263f60be3f1212) ) \
	\
	ROM_REGION( 0x200000, "layer0", 0 ) \
	ROM_LOAD( "bpsm.u53", 0x000000, 0x200000, CRC(b9b15f83) SHA1(8c574c97d38fb9e2889648c8d677b171e80a4229) ) \
	\
	ROM_REGION( 0x200000, "layer1", 0 ) \
	ROM_LOAD( "bpsm.u54", 0x000000, 0x200000, CRC(8f00679d) SHA1(4ea412f8ecdb9fd46f2d1378809919d1a62fcc2b) ) \
	\
	ROM_REGION( (5*0x200000)*2, "layer2", 0 ) \
	\
	ROM_LOAD( "bpsm.u57", 0x000000, 0x200000, CRC(86be7b63) SHA1(6b7d3d41fb1e4045c765b3cc98304464d91e6e3d) ) \
	ROM_LOAD( "bpsm.u58", 0x200000, 0x200000, CRC(e0bba83b) SHA1(9e1434814efd9321b2e5210b995d2fe66cca37dd) ) \
	ROM_LOAD( "bpsm.u62", 0x400000, 0x200000, CRC(a1e3bfac) SHA1(4528887d57e519df8dd60b2392db4c175c57b239) ) \
	ROM_LOAD( "bpsm.u61", 0x600000, 0x200000, CRC(6a014b52) SHA1(107c687479b59c455fc514cd61d290853c95ad9a) ) \
	ROM_LOAD( "bpsm.u60", 0x800000, 0x200000, CRC(992468c0) SHA1(3c66cc08313a9a326badc44f53a98cdfe0643da4) ) \
	\
	ROM_LOAD( "bpsm.u65", 0xa00000, 0x200000, CRC(f60fb7b5) SHA1(72cb8908cd687a330e14657664cd35037a52c39e) ) \
	ROM_LOAD( "bpsm.u64", 0xc00000, 0x200000, CRC(6559d31c) SHA1(bf688123a4beff625652cc1844bf0dc192f5c90f) ) \
	ROM_LOAD( "bpsm.u63", 0xe00000, 0x100000, CRC(d57a56b4) SHA1(e039b336887b66eba4e0630a3cb04cbd8fe14073) ) \
	ROM_CONTINUE(         0xe00000, 0x100000             ) \
	\
	ROM_REGION( 0x240000, "oki1", 0 ) \
	ROM_LOAD( "bpsm.u48", 0x040000, 0x200000, CRC(498e4ed1) SHA1(28d45a41702d9e5af4e214c1800b2e513ec84d51) ) \
	\
	ROM_REGION( 0x240000, "oki2", 0 ) \
	ROM_LOAD( "bpsm.u47", 0x040000, 0x080000, CRC(0f2901b9) SHA1(ebd3e9e39e8d2bc91688dac19b99548a28b4733c) ) \
	ROM_RELOAD(           0x0c0000, 0x080000             ) \
	ROM_RELOAD(           0x140000, 0x080000             ) \
	ROM_RELOAD(           0x1c0000, 0x080000             ) \

/* the regions differ only in the EEPROM, hence the macro above - all EEPROMs are Factory Defaulted */
ROM_START( sailormn )
	ROMS_SAILORMN

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "sailormn_europe.nv", 0x0000, 0x0080, CRC(59a7dc50) SHA1(6b116bdfbde42192b01678cb0b9bab0f2e56fd28) )
ROM_END

ROM_START( sailormnu )
	ROMS_SAILORMN

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "sailormn_usa.nv", 0x0000, 0x0080, CRC(3915abe3) SHA1(1b8d3b8c65cf2298939c27607ec52630c017c7ea) )
ROM_END

ROM_START( sailormnj )
	ROMS_SAILORMN

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "sailormn_japan.nv", 0x0000, 0x0080, CRC(ea03c30a) SHA1(2afc71f932674e34fc4491db0e2027e0371569fc) )
ROM_END

ROM_START( sailormnk )
	ROMS_SAILORMN

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "sailormn_korea.nv", 0x0000, 0x0080, CRC(0e7de398) SHA1(b495bf43d8596a0dc9843c74fc04fd21499bd115) )
ROM_END

ROM_START( sailormnt )
	ROMS_SAILORMN

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "sailormn_taiwan.nv", 0x0000, 0x0080, CRC(6c7e8c2a) SHA1(68ef4e6593e4c12e6488a20dcc6dda920b01de67) )
ROM_END

ROM_START( sailormnh )
	ROMS_SAILORMN

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "sailormn_hongkong.nv", 0x0000, 0x0080, CRC(4d24c874) SHA1(93b87ef18cff98e4f6a60234692e7a9a9c8e7552) )
ROM_END


#define ROMS_SAILORMNO \
	ROM_REGION( 0x400000, "maincpu", 0 ) \
	ROM_LOAD16_WORD_SWAP( "smprg.u45",    0x000000, 0x080000, CRC(234f1152) SHA1(8fc6d4a8995d550862d328011d3357c09334f0fa) ) \
	ROM_LOAD16_WORD_SWAP( "bpsm.u46",     0x200000, 0x200000, CRC(32084e80) SHA1(0ac503190d95009620b5ad7e7e0e63324f6fa4eb) ) \
	\
	ROM_REGION( 0x88000, "audiocpu", 0 ) \
	ROM_LOAD( "bpsm945a.u9",  0x00000, 0x08000, CRC(438de548) SHA1(81a0ca1cd662e2017aa980da162d39cfd0a19f14) ) \
	ROM_CONTINUE(             0x10000, 0x78000             ) \
	\
	ROM_REGION( 0x400000 * 2, "sprites", 0 ) \
	ROM_LOAD( "bpsm.u76", 0x000000, 0x200000, CRC(a243a5ba) SHA1(3a32d685e53e0b75977f7acb187cf414a50c7f8b) ) \
	ROM_LOAD( "bpsm.u77", 0x200000, 0x200000, CRC(5179a4ac) SHA1(ceb8d3d889aae885debb2c9cf2263f60be3f1212) ) \
	\
	ROM_REGION( 0x200000, "layer0", 0 ) \
	ROM_LOAD( "bpsm.u53", 0x000000, 0x200000, CRC(b9b15f83) SHA1(8c574c97d38fb9e2889648c8d677b171e80a4229) ) \
	\
	ROM_REGION( 0x200000, "layer1", 0 ) \
	ROM_LOAD( "bpsm.u54", 0x000000, 0x200000, CRC(8f00679d) SHA1(4ea412f8ecdb9fd46f2d1378809919d1a62fcc2b) ) \
	\
	ROM_REGION( (5*0x200000)*2, "layer2", 0 ) \
	\
	ROM_LOAD( "bpsm.u57", 0x000000, 0x200000, CRC(86be7b63) SHA1(6b7d3d41fb1e4045c765b3cc98304464d91e6e3d) ) \
	ROM_LOAD( "bpsm.u58", 0x200000, 0x200000, CRC(e0bba83b) SHA1(9e1434814efd9321b2e5210b995d2fe66cca37dd) ) \
	ROM_LOAD( "bpsm.u62", 0x400000, 0x200000, CRC(a1e3bfac) SHA1(4528887d57e519df8dd60b2392db4c175c57b239) ) \
	ROM_LOAD( "bpsm.u61", 0x600000, 0x200000, CRC(6a014b52) SHA1(107c687479b59c455fc514cd61d290853c95ad9a) ) \
	ROM_LOAD( "bpsm.u60", 0x800000, 0x200000, CRC(992468c0) SHA1(3c66cc08313a9a326badc44f53a98cdfe0643da4) ) \
	\
	ROM_LOAD( "bpsm.u65", 0xa00000, 0x200000, CRC(f60fb7b5) SHA1(72cb8908cd687a330e14657664cd35037a52c39e) ) \
	ROM_LOAD( "bpsm.u64", 0xc00000, 0x200000, CRC(6559d31c) SHA1(bf688123a4beff625652cc1844bf0dc192f5c90f) ) \
	ROM_LOAD( "bpsm.u63", 0xe00000, 0x100000, CRC(d57a56b4) SHA1(e039b336887b66eba4e0630a3cb04cbd8fe14073) ) \
	ROM_CONTINUE(         0xe00000, 0x100000             ) \
	\
	ROM_REGION( 0x240000, "oki1", 0 ) \
	ROM_LOAD( "bpsm.u48", 0x040000, 0x200000, CRC(498e4ed1) SHA1(28d45a41702d9e5af4e214c1800b2e513ec84d51) ) \
	\
	ROM_REGION( 0x240000, "oki2", 0 ) \
	ROM_LOAD( "bpsm.u47", 0x040000, 0x080000, CRC(0f2901b9) SHA1(ebd3e9e39e8d2bc91688dac19b99548a28b4733c) ) \
	ROM_RELOAD(           0x0c0000, 0x080000             ) \
	ROM_RELOAD(           0x140000, 0x080000             ) \
	ROM_RELOAD(           0x1c0000, 0x080000             ) \

/* the regions differ only in the EEPROM, hence the macro above - all EEPROMs are Factory Defaulted */
ROM_START( sailormno )
	ROMS_SAILORMNO

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "sailormn_europe.nv", 0x0000, 0x0080, CRC(59a7dc50) SHA1(6b116bdfbde42192b01678cb0b9bab0f2e56fd28) )
ROM_END

ROM_START( sailormnou )
	ROMS_SAILORMNO

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "sailormn_usa.nv", 0x0000, 0x0080, CRC(3915abe3) SHA1(1b8d3b8c65cf2298939c27607ec52630c017c7ea) )
ROM_END

ROM_START( sailormnoj )
	ROMS_SAILORMNO

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "sailormn_japan.nv", 0x0000, 0x0080, CRC(ea03c30a) SHA1(2afc71f932674e34fc4491db0e2027e0371569fc) )
ROM_END

ROM_START( sailormnok )
	ROMS_SAILORMNO

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "sailormn_korea.nv", 0x0000, 0x0080, CRC(0e7de398) SHA1(b495bf43d8596a0dc9843c74fc04fd21499bd115) )
ROM_END

ROM_START( sailormnot )
	ROMS_SAILORMNO

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "sailormn_taiwan.nv", 0x0000, 0x0080, CRC(6c7e8c2a) SHA1(68ef4e6593e4c12e6488a20dcc6dda920b01de67) )
ROM_END

ROM_START( sailormnoh )
	ROMS_SAILORMNO

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "sailormn_hongkong.nv", 0x0000, 0x0080, CRC(4d24c874) SHA1(93b87ef18cff98e4f6a60234692e7a9a9c8e7552) )
ROM_END


/***************************************************************************

  Tobikose! Jumpman by Namco, 1999
  Namco EMG4 platform, PCB TJ0476

  TMP 68HC000P-16

  013 9934WX002
  038 9919WX007

  OKI M6295

  Battery
  EEPROM?

  28MHz XTAL

***************************************************************************/

ROM_START( tjumpman )
	ROM_REGION( 0x080000, "maincpu", 0 )		/* 68000 code */
	ROM_LOAD16_WORD_SWAP( "tj1_mpr-0c.u41", 0x00000, 0x80000, CRC(de3030b8) SHA1(5f2165ea039c34cab605ebddc0b61eadc47b1532) )

	ROM_REGION( 0x100000 * 2, "sprites", 0 )		/* Sprites */
	ROM_LOAD16_BYTE( "tj1_obj-0a.u52", 0x00000, 0x80000, CRC(b42cf8e8) SHA1(9ed7fb3574ed163a81f34a0d8cfa7a4661439932) )
	ROM_LOAD16_BYTE( "tj1_obj-1a.u53", 0x00001, 0x80000, CRC(5f0124d7) SHA1(4d9cfa464159998c176a178c668273d128dedff8) )

	ROM_REGION( 0x80000, "layer0", 0 )	/* Layer 0 */
	ROM_LOAD( "tj1_cha-0a.u60", 0x00000, 0x40000, CRC(8aa08a38) SHA1(92b4df72fb8a833bb686ea478811243c5b868470) )
	ROM_LOAD( "tj1_cha-1a.u61", 0x40000, 0x40000, CRC(50072c82) SHA1(f8823e5a865afb8824cafd3b6483e2b6250ee77f) )

	ROM_REGION( 0x40000, "oki1", 0 )	/* OKIM6295 #1 Samples */
	ROM_LOAD( "tj1_voi-0a.u27", 0x00000, 0x40000, CRC(b5693aae) SHA1(8887ae98030cb5d184e3d57d2b4e48bf1e76a232) )

	ROM_REGION( 0x0002, "plds", 0 )
	ROM_LOAD( "tj1_gal.uxx", 0x0000, 0x0001, NO_DUMP )	// GAL16V8D-15LP
	ROM_LOAD( "tj1_gal.uyy", 0x0000, 0x0001, NO_DUMP )	// GAL16V8D-15LP
ROM_END


/***************************************************************************

                             Puzzle Uo Poko
Board: CV02
OSC:    28.0, 16.0, 16.9 MHz

***************************************************************************/

ROM_START( uopoko )
	ROM_REGION( 0x100000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "u26.int", 0x000000, 0x080000, CRC(b445c9ac) SHA1(4dda1c6e19de629ea4d9061560c32a9f0deabd53) )
	ROM_LOAD16_BYTE( "u25.int", 0x000001, 0x080000, CRC(a1258482) SHA1(7f4adc4a6d069032aaf3d93eb60fde16b59483f8) )

	ROM_REGION( 0x400000 * 2, "sprites", 0 )		/* Sprites: * 2 */
	ROM_LOAD( "u33.bin", 0x000000, 0x400000, CRC(5d142ad2) SHA1(f26abcf7a625a322b83df44fbd6e852bfb03663c) )

	ROM_REGION( 0x400000, "layer0", 0 )	/* Layer 0 */
	ROM_LOAD( "u49.bin", 0x000000, 0x400000, CRC(12fb11bb) SHA1(953df1b16b5c9a6c3eb2fdebec4669a879270e73) )

	ROM_REGION( 0x200000, "ymz", 0 )	/* Samples */
	ROM_LOAD( "u4.bin", 0x000000, 0x200000, CRC(a2d0d755) SHA1(f8493ef7f367f3dc2a229ba785ac67bc5c2c54c0) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-uopoko.bin", 0x0000, 0x0080, CRC(f4a24b95) SHA1(4043f0ffed24e38b4f7dbe1a5a4a9e79bdde7dfd) )
ROM_END

ROM_START( uopokoj )
	ROM_REGION( 0x100000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "u26.bin", 0x000000, 0x080000, CRC(e7eec050) SHA1(cf3a77741029f96dbbec5ca7217a1723e4233cff) )
	ROM_LOAD16_BYTE( "u25.bin", 0x000001, 0x080000, CRC(68cb6211) SHA1(a6db0bc2e3e54b6992a44b7d52395917e66db49b) )

	ROM_REGION( 0x400000 * 2, "sprites", 0 )		/* Sprites: * 2 */
	ROM_LOAD( "u33.bin", 0x000000, 0x400000, CRC(5d142ad2) SHA1(f26abcf7a625a322b83df44fbd6e852bfb03663c) )

	ROM_REGION( 0x400000, "layer0", 0 )	/* Layer 0 */
	ROM_LOAD( "u49.bin", 0x000000, 0x400000, CRC(12fb11bb) SHA1(953df1b16b5c9a6c3eb2fdebec4669a879270e73) )

	ROM_REGION( 0x200000, "ymz", 0 )	/* Samples */
	ROM_LOAD( "u4.bin", 0x000000, 0x200000, CRC(a2d0d755) SHA1(f8493ef7f367f3dc2a229ba785ac67bc5c2c54c0) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-uopoko.bin", 0x0000, 0x0080, CRC(f4a24b95) SHA1(4043f0ffed24e38b4f7dbe1a5a4a9e79bdde7dfd) )
ROM_END






/***************************************************************************


    Drivers Init Routines - Rom decryption/unpacking, global vars etc.


***************************************************************************/

/* Tiles are 6 bit, 4 bits stored in one rom, 2 bits in the other.
   Expand the 2 bit part into a 4 bit layout, so we can decode it */
static void sailormn_unpack_tiles( running_machine *machine, const char *region )
{
	const UINT32 len	=	memory_region_length(machine, region);
	UINT8 *rgn		=	memory_region(machine, region);
	UINT8 *src		=	rgn + (len/4)*3 - 1;
	UINT8 *dst		=	rgn + (len/4)*4 - 2;

	while(src <= dst)
	{
		UINT8 data = src[0];

		dst[0] = ((data & 0x03) << 4) + ((data & 0x0c) >> 2);
		dst[1] = ((data & 0x30) >> 0) + ((data & 0xc0) >> 6);

		src -= 1;
		dst -= 2;
	}
}

static void init_cave(running_machine *machine)
{
	cave_state *state = (cave_state *)machine->driver_data;

	state->spritetype[0] = 0;	// Normal sprites
	state->kludge = 0;
	state->time_vblank_irq = 100;

	state->irq_level = 1;
}


static DRIVER_INIT( agallet )
{
	UINT8 *ROM = memory_region(machine, "audiocpu");
	init_cave(machine);

	memory_configure_bank(machine, "bank1", 0, 0x02, &ROM[0x00000], 0x4000);
	memory_configure_bank(machine, "bank1", 2, 0x1e, &ROM[0x10000], 0x4000);

	sailormn_unpack_tiles(machine, "layer2");

	unpack_sprites(machine);
}

static DRIVER_INIT( dfeveron )
{
	cave_state *state = (cave_state *)machine->driver_data;
	init_cave(machine);

	unpack_sprites(machine);
	state->kludge = 2;
}

static DRIVER_INIT( feversos )
{
	cave_state *state = (cave_state *)machine->driver_data;
	init_cave(machine);

	unpack_sprites(machine);
	state->kludge = 2;
}

static DRIVER_INIT( ddonpach )
{
	cave_state *state = (cave_state *)machine->driver_data;
	init_cave(machine);

	ddonpach_unpack_sprites(machine);
	state->spritetype[0] = 1;	// "different" sprites (no zooming?)
	state->time_vblank_irq = 90;
}

static DRIVER_INIT( donpachi )
{
	cave_state *state = (cave_state *)machine->driver_data;
	init_cave(machine);

	ddonpach_unpack_sprites(machine);
	state->spritetype[0] = 1;	// "different" sprites (no zooming?)
	state->time_vblank_irq = 90;
}


static DRIVER_INIT( esprade )
{
	cave_state *state = (cave_state *)machine->driver_data;
	init_cave(machine);

	esprade_unpack_sprites(machine);
	state->time_vblank_irq = 2000;	/**/

#if 0		//ROM PATCH
	{
		UINT16 *rom = (UINT16 *)memory_region(machine, "maincpu");
		rom[0x118A/2] = 0x4e71;			//palette fix   118A: 5548              SUBQ.W  #2,A0       --> NOP
	}
#endif
}

static DRIVER_INIT( gaia )
{
	cave_state *state = (cave_state *)machine->driver_data;
	init_cave(machine);

	/* No EEPROM */

	unpack_sprites(machine);
	state->spritetype[0] = 2;	// Normal sprites with different position handling
	state->time_vblank_irq = 2000;	/**/
}

static DRIVER_INIT( guwange )
{
	cave_state *state = (cave_state *)machine->driver_data;
	init_cave(machine);

	esprade_unpack_sprites(machine);
	state->time_vblank_irq = 2000;	/**/
}

static DRIVER_INIT( hotdogst )
{
	cave_state *state = (cave_state *)machine->driver_data;
	UINT8 *ROM = memory_region(machine, "audiocpu");

	init_cave(machine);

	memory_configure_bank(machine, "bank2", 0, 0x2, &ROM[0x00000], 0x4000);
	memory_configure_bank(machine, "bank2", 2, 0xe, &ROM[0x10000], 0x4000);

	unpack_sprites(machine);
	state->spritetype[0] = 2;	// Normal sprites with different position handling
	state->time_vblank_irq = 2000;	/**/
}

static DRIVER_INIT( mazinger )
{
	cave_state *state = (cave_state *)machine->driver_data;
	UINT8 *ROM = memory_region(machine, "audiocpu");
	UINT8 *buffer;
	UINT8 *src = memory_region(machine, "sprites");
	int len = memory_region_length(machine, "sprites");

	init_cave(machine);

	memory_configure_bank(machine, "bank2", 0, 2, &ROM[0x00000], 0x4000);
	memory_configure_bank(machine, "bank2", 2, 6, &ROM[0x10000], 0x4000);

	/* decrypt sprites */
	buffer = auto_alloc_array(machine, UINT8, len);
	{
		int i;
		for (i = 0; i < len; i++)
			buffer[i ^ 0xdf88] = src[BITSWAP24(i,23,22,21,20,19,9,7,3,15,4,17,14,18,2,16,5,11,8,6,13,1,10,12,0)];
		memcpy(src, buffer, len);
		auto_free(machine, buffer);
	}

	unpack_sprites(machine);
	state->spritetype[0] = 2;	// Normal sprites with different position handling
	state->kludge = 3;
	state->time_vblank_irq = 2100;

	/* setup extra ROM */
	memory_set_bankptr(machine, "bank1",memory_region(machine, "user1"));
}


static DRIVER_INIT( metmqstr )
{
	cave_state *state = (cave_state *)machine->driver_data;
	UINT8 *ROM = memory_region(machine, "audiocpu");

	init_cave(machine);

	memory_configure_bank(machine, "bank1", 0, 0x2, &ROM[0x00000], 0x4000);
	memory_configure_bank(machine, "bank1", 2, 0xe, &ROM[0x10000], 0x4000);

	unpack_sprites(machine);
	state->spritetype[0] = 2;	// Normal sprites with different position handling
	state->kludge = 3;
	state->time_vblank_irq = 17376;
}


static DRIVER_INIT( pwrinst2j )
{
	cave_state *state = (cave_state *)machine->driver_data;
	UINT8 *ROM = memory_region(machine, "audiocpu");
	UINT8 *buffer;
	UINT8 *src = memory_region(machine, "sprites");
	int len = memory_region_length(machine, "sprites");
	int i, j;

	init_cave(machine);

	memory_configure_bank(machine, "bank1", 0, 3, &ROM[0x00000], 0x4000);
	memory_configure_bank(machine, "bank1", 3, 5, &ROM[0x10000], 0x4000);

	buffer = auto_alloc_array(machine, UINT8, len);
	{
		for(i = 0; i < len/2; i++)
		{
			j = BITSWAP24(i,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7, 2,4,6,1,5,3, 0);
			if(((j & 6) == 0) || ((j & 6) == 6))
				j ^= 6;
			buffer[j ^ 7] = (src[i] >> 4) | (src[i] << 4);
		}

		memcpy(src,buffer,len);
		auto_free(machine, buffer);
	}

	unpack_sprites(machine);
	state->spritetype[0] = 3;
	state->kludge = 4;
	state->time_vblank_irq = 2000;	/**/
}

static DRIVER_INIT( pwrinst2 )
{
	/* this patch fixes on of the moves, why is it needed? is the rom bad or is there another
       problem? does the Japan set need it or not? */
	DRIVER_INIT_CALL(pwrinst2j);

#if 1		//ROM PATCH
	{
		UINT16 *rom = (UINT16 *)memory_region(machine, "maincpu");
		rom[0xd46c / 2] = 0xd482;			// kurara dash fix  0xd400 -> 0xd482
	}
#endif

}

static DRIVER_INIT( sailormn )
{
	cave_state *state = (cave_state *)machine->driver_data;
	UINT8 *ROM = memory_region(machine, "audiocpu");
	UINT8 *buffer;
	UINT8 *src = memory_region(machine, "sprites");
	int len = memory_region_length(machine, "sprites");

	init_cave(machine);

	memory_configure_bank(machine, "bank1", 0, 0x02, &ROM[0x00000], 0x4000);
	memory_configure_bank(machine, "bank1", 2, 0x1e, &ROM[0x10000], 0x4000);

	/* decrypt sprites */
	buffer = auto_alloc_array(machine, UINT8, len);
	{
		int i;
		for (i = 0; i < len; i++)
			buffer[i ^ 0x950c4] = src[BITSWAP24(i,23,22,21,20,15,10,12,6,11,1,13,3,16,17,2,5,14,7,18,8,4,19,9,0)];
		memcpy(src, buffer, len);
		auto_free(machine, buffer);
	}

	sailormn_unpack_tiles( machine, "layer2" );

	unpack_sprites(machine);
	state->spritetype[0] = 2;	// Normal sprites with different position handling
	state->kludge = 1;
	state->time_vblank_irq = 2000;

	state->sailormn_tilebank = 0;
	state_save_register_global(machine, state->sailormn_tilebank);
}

static DRIVER_INIT( tjumpman )
{
	cave_state *state = (cave_state *)machine->driver_data;
	init_cave(machine);

	unpack_sprites(machine);
	state->spritetype[0] = 2;	// Normal sprites with different position handling
	state->kludge = 3;
	state->time_vblank_irq = 17376;

	state->hopper = 0;
	state_save_register_global(machine, state->hopper);
}

static DRIVER_INIT( uopoko )
{
	cave_state *state = (cave_state *)machine->driver_data;
	init_cave(machine);

	unpack_sprites(machine);
	state->kludge = 2;
	state->time_vblank_irq = 2000;	/**/
}

static DRIVER_INIT( korokoro )
{
	cave_state *state = (cave_state *)machine->driver_data;
	init_cave(machine);

	state->irq_level = 2;

	unpack_sprites(machine);
	state->time_vblank_irq = 2000;	/**/

	state->leds[0] = 0;
	state->leds[1] = 0;
	state->hopper = 0;
	state_save_register_global_array(machine, state->leds);
	state_save_register_global(machine, state->hopper);
}

/***************************************************************************


                                Game Drivers


***************************************************************************/

GAME( 1994, pwrinst2,   0,        pwrinst2, metmqstr, pwrinst2, ROT0,   "Atlus",                                  "Power Instinct 2 (US, Ver. 94/04/08)",                   0 )
GAME( 1994, pwrinst2j,  pwrinst2, pwrinst2, metmqstr, pwrinst2j,ROT0,   "Atlus",                                  "Gouketsuji Ichizoku 2 (Japan, Ver. 94/04/08)",           0 )

// The EEPROM determines the region, program roms are the same between sets
GAME( 1994, mazinger,   0,        mazinger, cave,     mazinger, ROT90,  "Banpresto / Dynamic Pl. Toei Animation", "Mazinger Z (World)",                                     0 )
GAME( 1994, mazingerj,  mazinger, mazinger, cave,     mazinger, ROT90,  "Banpresto / Dynamic Pl. Toei Animation", "Mazinger Z (Japan)",                                     0 )

GAME( 1995, donpachi,   0,        donpachi, cave,     donpachi, ROT270, "Cave (Atlus license)",                   "DonPachi (US)",                                          0 )
GAME( 1995, donpachij,  donpachi, donpachi, cave,     donpachi, ROT270, "Cave (Atlus license)",                   "DonPachi (Japan)",                                       0 )
GAME( 1995, donpachikr, donpachi, donpachi, cave,     donpachi, ROT270, "Cave (Atlus license)",                   "DonPachi (Korea)",                                       0 )
GAME( 1995, donpachihk, donpachi, donpachi, cave,     donpachi, ROT270, "Cave (Atlus license)",                   "DonPachi (Hong Kong)",                                   0 )

GAME( 1995, metmqstr,   0,        metmqstr, metmqstr, metmqstr, ROT0,   "Banpresto / Pandorabox",                 "Metamoqester (International)",                           0 )
GAME( 1995, nmaster,    metmqstr, metmqstr, metmqstr, metmqstr, ROT0,   "Banpresto / Pandorabox",                 "Oni - The Ninja Master (Japan)",                         0 )

GAME( 1995, plegends,   0,        pwrinst2, metmqstr, pwrinst2j,ROT0,   "Atlus",                                  "Power Instinct Legends (US, Ver. 95/06/20)",             0 )
GAME( 1995, plegendsj,  plegends, pwrinst2, metmqstr, pwrinst2j,ROT0,   "Atlus",                                  "Gouketsuji Ichizoku Saikyou Densetsu (Japan, Ver. 95/06/20)", 0 )

// The EEPROM determines the region, program roms are the same between sets
GAME( 1995, sailormn,   0,        sailormn, cave,     sailormn, ROT0,   "Banpresto",                              "Pretty Soldier Sailor Moon (Ver. 95/03/22B, Europe)",    0 )
GAME( 1995, sailormnu,  sailormn, sailormn, cave,     sailormn, ROT0,   "Banpresto",                              "Pretty Soldier Sailor Moon (Ver. 95/03/22B, USA)",       0 )
GAME( 1995, sailormnj,  sailormn, sailormn, cave,     sailormn, ROT0,   "Banpresto",                              "Pretty Soldier Sailor Moon (Ver. 95/03/22B, Japan)",     0 )
GAME( 1995, sailormnk,  sailormn, sailormn, cave,     sailormn, ROT0,   "Banpresto",                              "Pretty Soldier Sailor Moon (Ver. 95/03/22B, Korea)",     0 )
GAME( 1995, sailormnt,  sailormn, sailormn, cave,     sailormn, ROT0,   "Banpresto",                              "Pretty Soldier Sailor Moon (Ver. 95/03/22B, Taiwan)",    0 )
GAME( 1995, sailormnh,  sailormn, sailormn, cave,     sailormn, ROT0,   "Banpresto",                              "Pretty Soldier Sailor Moon (Ver. 95/03/22B, Hong Kong)", 0 )
GAME( 1995, sailormno,  sailormn, sailormn, cave,     sailormn, ROT0,   "Banpresto",                              "Pretty Soldier Sailor Moon (Ver. 95/03/22, Europe)",     0 )
GAME( 1995, sailormnou, sailormn, sailormn, cave,     sailormn, ROT0,   "Banpresto",                              "Pretty Soldier Sailor Moon (Ver. 95/03/22, USA)",        0 )
GAME( 1995, sailormnoj, sailormn, sailormn, cave,     sailormn, ROT0,   "Banpresto",                              "Pretty Soldier Sailor Moon (Ver. 95/03/22, Japan)",      0 )
GAME( 1995, sailormnok, sailormn, sailormn, cave,     sailormn, ROT0,   "Banpresto",                              "Pretty Soldier Sailor Moon (Ver. 95/03/22, Korea)",      0 )
GAME( 1995, sailormnot, sailormn, sailormn, cave,     sailormn, ROT0,   "Banpresto",                              "Pretty Soldier Sailor Moon (Ver. 95/03/22, Taiwan)",     0 )
GAME( 1995, sailormnoh, sailormn, sailormn, cave,     sailormn, ROT0,   "Banpresto",                              "Pretty Soldier Sailor Moon (Ver. 95/03/22, Hong Kong)",	0 )

// The EEPROM determines the region, program roms are the same between sets
GAME( 1996, agallet,    0,        sailormn, cave,     agallet,  ROT270, "Banpresto / Gazelle",                    "Air Gallet (Europe)", 0 )
GAME( 1996, agalletu,   agallet,  sailormn, cave,     agallet,  ROT270, "Banpresto / Gazelle",                    "Air Gallet (USA)", 0 )
GAME( 1996, agalletj,   agallet,  sailormn, cave,     agallet,  ROT270, "Banpresto / Gazelle",                    "Air Gallet (Japan)", 0 )
GAME( 1996, agalletk,   agallet,  sailormn, cave,     agallet,  ROT270, "Banpresto / Gazelle",                    "Air Gallet (Korea)", 0 )
GAME( 1996, agallett,   agallet,  sailormn, cave,     agallet,  ROT270, "Banpresto / Gazelle",                    "Air Gallet (Taiwan)", 0 )
GAME( 1996, agalleth,   agallet,  sailormn, cave,     agallet,  ROT270, "Banpresto / Gazelle",                    "Air Gallet (Hong Kong)",                               0 )

GAME( 1996, hotdogst,   0,        hotdogst, cave,     hotdogst, ROT90,  "Marble",                                 "Hotdog Storm (International)",                         0 )

GAME( 1997, ddonpach,   0,        ddonpach, cave,     ddonpach, ROT270, "Cave (Atlus license)",                   "DoDonPachi (International, Master Ver. 97/02/05)",     0 )
GAME( 1997, ddonpachj,  ddonpach, ddonpach, cave,     ddonpach, ROT270, "Cave (Atlus license)",                   "DoDonPachi (Japan, Master Ver. 97/02/05)",             0 )

GAME( 1998, dfeveron,   feversos, dfeveron, cave,     dfeveron, ROT270, "Cave (Nihon System license)",            "Dangun Feveron (Japan, Ver. 98/09/17)",                0 )
GAME( 1998, feversos,   0,        dfeveron, cave,     feversos, ROT270, "Cave (Nihon System license)",            "Fever SOS (International, Ver. 98/09/25)",             0 )

GAME( 1998, esprade,    0,        esprade,  cave,     esprade,  ROT270, "Cave (Atlus license)",                   "ESP Ra.De. (International, Ver. 98/04/22)",            0 )
GAME( 1998, espradej,   esprade,  esprade,  cave,     esprade,  ROT270, "Cave (Atlus license)",                   "ESP Ra.De. (Japan, Ver. 98/04/21)",                    0 )
GAME( 1998, espradejo,  esprade,  esprade,  cave,     esprade,  ROT270, "Cave (Atlus license)",                   "ESP Ra.De. (Japan, Ver. 98/04/14)",                    0 )

GAME( 1998, uopoko,     0,        uopoko,   cave,     uopoko,   ROT0,   "Cave (Jaleco license)",                  "Puzzle Uo Poko (International)",                       0 )
GAME( 1998, uopokoj,    uopoko,   uopoko,   cave,     uopoko,   ROT0,   "Cave (Jaleco license)",                  "Puzzle Uo Poko (Japan)",                               0 )

GAME( 1999, guwange,    0,        guwange,  guwange,  guwange,  ROT270, "Cave (Atlus license)",                   "Guwange (Japan, Master Ver. 99/06/24)",                0 )

GAME( 1999, gaia,       0,        gaia,     gaia,     gaia,     ROT0,   "Noise Factory",                          "Gaia Crusaders",		                                  GAME_IMPERFECT_SOUND ) // cuts out occasionally

GAME( 2001, theroes,    0,        gaia,     theroes,  gaia,     ROT0,   "Primetek Investments",                   "Thunder Heroes",		                                  GAME_IMPERFECT_SOUND ) // cuts out occasionally

GAME( 1999, korokoro,   0,        korokoro, korokoro, korokoro, ROT0,   "Takumi",                                 "Koro Koro Quest (Japan)",                              0 )

GAME( 1999, crusherm,   0,        crusherm, korokoro, korokoro, ROT0,   "Takumi",                                 "Crusher Makochan (Japan)",                             0 )

GAME( 1999, tjumpman,   0,        tjumpman, tjumpman, tjumpman, ROT0,   "Namco",                                  "Tobikose! Jumpman",                                    0 )
