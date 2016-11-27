/***************************************************************************

    Karnov (USA version)                   (c) 1987 Data East USA
    Karnov (Japanese version)              (c) 1987 Data East Corporation
    Wonder Planet (Japanese version)       (c) 1987 Data East Corporation
    Chelnov (World version)                (c) 1987 Data East Corporation
    Chelnov (USA version)                  (c) 1988 Data East USA
    Chelnov (Japanese version)             (c) 1987 Data East Corporation


    Emulation by Bryan McPhail, mish@tendril.co.uk


    NOTE!  Karnov USA & Karnov Japan sets have different gameplay!
      and Chelnov USA & Chelnov Japan sets have different gameplay!

    These games use a 68000 main processor with a 6502, YM2203C and YM3526 for
    sound.  Karnov was a major pain to get going because of the
    'protection' on the main player sprite, probably connected to the Intel
    microcontroller on the board.  The game is very sensitive to the wrong values
    at the input ports...

    There is another Karnov rom set - a bootleg version of the Japanese roms with
    the Data East copyright removed - not supported because the original Japanese
    roms work fine.

    One of the two color PROMs for chelnov and chelnoj is different; one is most
    likely a bad read, but I don't know which one.

    Thanks to Oliver Stabel <stabel@rhein-neckar.netsurf.de> for confirming some
    of the sprite & control information :)

    Cheats:

    Karnov - put 0x30 at 0x60201 to skip a level
    Chelnov - level number at 0x60189 - enter a value at cartoon intro

*******************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/m6502/m6502.h"
#include "sound/2203intf.h"
#include "sound/3526intf.h"
#include "includes/karnov.h"


/*************************************
 *
 *  Microcontroller emulation
 *
 *************************************/

/* Emulation of the protected microcontroller - for coins & general protection */
static void karnov_i8751_w( running_machine *machine, int data )
{
	karnov_state *state = (karnov_state *)machine->driver_data;

	/* Pending coin operations may cause protection commands to be queued */
	if (state->i8751_needs_ack)
	{
		state->i8751_command_queue = data;
		return;
	}

	state->i8751_return = 0;

	if (data == 0x100 && state->microcontroller_id == KARNOVJ)	/* Japan version */
		state->i8751_return = 0x56a;

	if (data == 0x100 && state->microcontroller_id == KARNOV)	/* USA version */
		state->i8751_return = 0x56b;

	if ((data & 0xf00) == 0x300)
		state->i8751_return = (data & 0xff) * 0x12; /* Player sprite mapping */

	/* I'm not sure the ones marked ^ appear in the right order */
	if (data == 0x400) state->i8751_return = 0x4000; /* Get The Map... */
	if (data == 0x402) state->i8751_return = 0x40a6; /* Ancient Ruins */
	if (data == 0x403) state->i8751_return = 0x4054; /* Forest... */
	if (data == 0x404) state->i8751_return = 0x40de; /* ^Rocky hills */
	if (data == 0x405) state->i8751_return = 0x4182; /* Sea */
	if (data == 0x406) state->i8751_return = 0x41ca; /* Town */
	if (data == 0x407) state->i8751_return = 0x421e; /* Desert */
	if (data == 0x401) state->i8751_return = 0x4138; /* ^Whistling wind */
	if (data == 0x408) state->i8751_return = 0x4276; /* ^Heavy Gates */

//  if (!state->i8751_return && data != 0x300) logerror("%s - Unknown Write %02x intel\n", cpuexec_describe_context(machine), data);

	cpu_set_input_line(state->maincpu, 6, HOLD_LINE); /* Signal main cpu task is complete */
	state->i8751_needs_ack = 1;
}

static void wndrplnt_i8751_w( running_machine *machine, int data )
{
	karnov_state *state = (karnov_state *)machine->driver_data;

	/* The last command hasn't been ACK'd (probably a conflict with coin command) */
	if (state->i8751_needs_ack)
	{
		state->i8751_command_queue = data;
		return;
	}

	state->i8751_return=0;

	if (data == 0x100) state->i8751_return = 0x67a;
	if (data == 0x200) state->i8751_return = 0x214;
	if (data == 0x300) state->i8751_return = 0x17; /* Copyright text on title screen */
//  if (data == 0x300) state->i8751_return = 0x1; /* (USA) Copyright text on title screen */

	/* The game writes many values in the 0x600 range, but only a specific mask
    matters for the return value */
	if ((data & 0x600) == 0x600)
	{
		switch (data & 0x18)
		{
			case 0x00:	state->i8751_return = 0x4d53; break;
			case 0x08:	state->i8751_return = 0x4b54; break;
			case 0x10:	state->i8751_return = 0x5453; break;
			case 0x18:	state->i8751_return = 0x5341; break;
		}
	}
//  else logerror("%s - Unknown Write %02x intel\n", cpuexec_describe_context(machine), data);

	/* These are 68k function call addresses - different address for each power-up */
	if (data == 0x400) state->i8751_return = 0x594;
	if (data == 0x401) state->i8751_return = 0x5ea;
	if (data == 0x402) state->i8751_return = 0x628;
	if (data == 0x403) state->i8751_return = 0x66c;
	if (data == 0x404) state->i8751_return = 0x6a4;
	if (data == 0x405) state->i8751_return = 0x6a4;
	if (data == 0x406) state->i8751_return = 0x6a4;

	/* This is 68k program code which is executed every frame */
	if (data == 0x50c) state->i8751_return = 0x13fc;
	if (data == 0x50b) state->i8751_return = 0x00ff;
	if (data == 0x50a) state->i8751_return = 0x0006;
	if (data == 0x509) state->i8751_return = 0x0000;
	if (data == 0x508) state->i8751_return = 0x4a39;
	if (data == 0x507) state->i8751_return = 0x0006;
	if (data == 0x506) state->i8751_return = 0x0000;
	if (data == 0x505) state->i8751_return = 0x66f8;
	if (data == 0x504) state->i8751_return = 0x4a39;
	if (data == 0x503) state->i8751_return = 0x000c;
	if (data == 0x502) state->i8751_return = 0x0003;
	if (data == 0x501) state->i8751_return = 0x6bf8;
	if (data == 0x500) state->i8751_return = 0x4e75;

	cpu_set_input_line(state->maincpu, 6, HOLD_LINE); /* Signal main cpu task is complete */
	state->i8751_needs_ack = 1;
}

static void chelnov_i8751_w( running_machine *machine, int data )
{
	karnov_state *state = (karnov_state *)machine->driver_data;

	/* Pending coin operations may cause protection commands to be queued */
	if (state->i8751_needs_ack)
	{
		state->i8751_command_queue = data;
		return;
	}

	state->i8751_return = 0;

	if (data == 0x200 && state->microcontroller_id == CHELNOVJ)	/* Japan version */
		state->i8751_return = 0x7734;

	if (data == 0x200 && state->microcontroller_id == CHELNOV)	/* USA version */
		state->i8751_return = 0x783e;

	if (data == 0x200 && state->microcontroller_id == CHELNOVW)	/* World version */
		state->i8751_return = 0x7736;

	if (data == 0x100 && state->microcontroller_id == CHELNOVJ)	/* Japan version */
		state->i8751_return = 0x71a;

	if (data == 0x100 && state->microcontroller_id == CHELNOV)	/* USA version */
		state->i8751_return = 0x71b;

	if (data == 0x100 && state->microcontroller_id == CHELNOVW)	/* World version */
		state->i8751_return = 0x71c;

	if (data >= 0x6000 && data < 0x8000)
		state->i8751_return = 1;  /* patched */

	if ((data & 0xf000) == 0x1000) state->i8751_level = 1;	/* Level 1 */
	if ((data & 0xf000) == 0x2000) state->i8751_level++;		/* Level Increment */

	if ((data & 0xf000) == 0x3000)
	{
		/* Sprite table mapping */
		int b = data & 0xff;
		switch (state->i8751_level)
		{
			case 1: /* Level 1, Sprite mapping tables */
				if (state->microcontroller_id == CHELNOV) /* USA */
				{
					if (b < 2) state->i8751_return = 0;
					else if (b < 6) state->i8751_return = 1;
					else if (b < 0xb) state->i8751_return = 2;
					else if (b < 0xf) state->i8751_return = 3;
					else if (b < 0x13) state->i8751_return = 4;
					else state->i8751_return = 5;
				}
				else	/* Japan, World */
				{
					if (b < 3) state->i8751_return = 0;
					else if (b < 8) state->i8751_return = 1;
					else if (b < 0xc) state->i8751_return = 2;
					else if (b < 0x10) state->i8751_return = 3;
					else if (b < 0x19) state->i8751_return = 4;
					else if (b < 0x1b) state->i8751_return = 5;
					else if (b < 0x22) state->i8751_return = 6;
					else if (b < 0x28) state->i8751_return = 7;
					else state->i8751_return = 8;
				}
				break;
			case 2: /* Level 2, Sprite mapping tables, USA & Japan are the same */
				if (b < 3) state->i8751_return = 0;
				else if (b < 9) state->i8751_return = 1;
				else if (b < 0x11) state->i8751_return = 2;
				else if (b < 0x1b) state->i8751_return = 3;
				else if (b < 0x21) state->i8751_return = 4;
				else if (b < 0x28) state->i8751_return = 5;
				else state->i8751_return = 6;
				break;
			case 3: /* Level 3, Sprite mapping tables, USA & Japan are the same */
				if (b < 5) state->i8751_return = 0;
				else if (b < 9) state->i8751_return = 1;
				else if (b < 0xd) state->i8751_return = 2;
				else if (b < 0x11) state->i8751_return = 3;
				else if (b < 0x1b) state->i8751_return = 4;
				else if (b < 0x1c) state->i8751_return = 5;
				else if (b < 0x22) state->i8751_return = 6;
				else if (b < 0x27) state->i8751_return = 7;
				else state->i8751_return = 8;
				break;
			case 4: /* Level 4, Sprite mapping tables, USA & Japan are the same */
				if (b < 4) state->i8751_return = 0;
				else if (b < 0xc) state->i8751_return = 1;
				else if (b < 0xf) state->i8751_return = 2;
				else if (b < 0x19) state->i8751_return = 3;
				else if (b < 0x1c) state->i8751_return = 4;
				else if (b < 0x22) state->i8751_return = 5;
				else if (b < 0x29) state->i8751_return = 6;
				else state->i8751_return = 7;
				break;
			case 5: /* Level 5, Sprite mapping tables */
				if (b < 7) state->i8751_return = 0;
				else if (b < 0xe) state->i8751_return = 1;
				else if (b < 0x14) state->i8751_return = 2;
				else if (b < 0x1a) state->i8751_return = 3;
				else if (b < 0x23) state->i8751_return = 4;
				else if (b < 0x27) state->i8751_return = 5;
				else state->i8751_return = 6;
				break;
			case 6: /* Level 6, Sprite mapping tables */
				if (b < 3) state->i8751_return = 0;
				else if (b < 0xb) state->i8751_return = 1;
				else if (b < 0x11) state->i8751_return = 2;
				else if (b < 0x17) state->i8751_return = 3;
				else if (b < 0x1d) state->i8751_return = 4;
				else if (b < 0x24) state->i8751_return = 5;
				else state->i8751_return = 6;
				break;
			case 7: /* Level 7, Sprite mapping tables */
				if (b < 5) state->i8751_return = 0;
				else if (b < 0xb) state->i8751_return = 1;
				else if (b < 0x11) state->i8751_return = 2;
				else if (b < 0x1a) state->i8751_return = 3;
				else if (b < 0x21) state->i8751_return = 4;
				else if (b < 0x27) state->i8751_return = 5;
				else state->i8751_return = 6;
				break;
		}
	}

	//  logerror("%s - Unknown Write %02x intel\n", cpuexec_describe_context(machine), data);

	cpu_set_input_line(state->maincpu, 6, HOLD_LINE); /* Signal main cpu task is complete */
	state->i8751_needs_ack = 1;
}

/*************************************
 *
 *  Memory handlers
 *
 *************************************/

static WRITE16_HANDLER( karnov_control_w )
{
	karnov_state *state = (karnov_state *)space->machine->driver_data;

	/* Mnemonics filled in from the schematics, brackets are my comments */
	switch (offset << 1)
	{
		case 0: /* SECLR (Interrupt ack for Level 6 i8751 interrupt) */
			cpu_set_input_line(state->maincpu, 6, CLEAR_LINE);

			if (state->i8751_needs_ack)
			{
				/* If a command and coin insert happen at once, then the i8751 will queue the coin command until the previous command is ACK'd */
				if (state->i8751_coin_pending)
				{
					state->i8751_return = state->i8751_coin_pending;
					cpu_set_input_line(state->maincpu, 6, HOLD_LINE);
					state->i8751_coin_pending = 0;
				}
				else if (state->i8751_command_queue)
				{
					/* Pending control command - just write it back as SECREQ */
					state->i8751_needs_ack = 0;
					karnov_control_w(space, 3, state->i8751_command_queue, 0xffff);
					state->i8751_command_queue = 0;
				}
				else
				{
					state->i8751_needs_ack = 0;
				}
			}
			return;

		case 2: /* SONREQ (Sound CPU byte) */
			soundlatch_w(space, 0, data & 0xff);
			cpu_set_input_line(state->audiocpu, INPUT_LINE_NMI, PULSE_LINE);
			break;

		case 4: /* DM (DMA to buffer spriteram) */
			buffer_spriteram16_w(space, 0, 0, 0xffff);
			break;

		case 6: /* SECREQ (Interrupt & Data to i8751) */
			if (state->microcontroller_id == KARNOV || state->microcontroller_id == KARNOVJ)
				karnov_i8751_w(space->machine, data);
			if (state->microcontroller_id == CHELNOV || state->microcontroller_id == CHELNOVJ || state->microcontroller_id == CHELNOVW)
				chelnov_i8751_w(space->machine, data);
			if (state->microcontroller_id == WNDRPLNT)
				wndrplnt_i8751_w(space->machine, data);
			break;

		case 8: /* HSHIFT (9 bits) - Top bit indicates video flip */
			COMBINE_DATA(&state->scroll[0]);
			karnov_flipscreen_w(space->machine, data >> 15);
			break;

		case 0xa: /* VSHIFT */
			COMBINE_DATA(&state->scroll[1]);
			break;

		case 0xc: /* SECR (Reset i8751) */
			logerror("Reset i8751\n");
			state->i8751_needs_ack = 0;
			state->i8751_coin_pending = 0;
			state->i8751_command_queue = 0;
			state->i8751_return = 0;
			break;

		case 0xe: /* INTCLR (Interrupt ack for Level 7 vbl interrupt) */
			cpu_set_input_line(state->maincpu, 7, CLEAR_LINE);
			break;
	}
}

static READ16_HANDLER( karnov_control_r )
{
	karnov_state *state = (karnov_state *)space->machine->driver_data;

	switch (offset << 1)
	{
		case 0:
			return input_port_read(space->machine, "P1_P2");
		case 2: /* Start buttons & VBL */
			return input_port_read(space->machine, "SYSTEM");
		case 4:
			return input_port_read(space->machine, "DSW");
		case 6: /* i8751 return values */
			return state->i8751_return;
	}

	return ~0;
}

/*************************************
 *
 *  Address maps
 *
 *************************************/

static ADDRESS_MAP_START( karnov_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x05ffff) AM_ROM
	AM_RANGE(0x060000, 0x063fff) AM_RAM AM_BASE_MEMBER(karnov_state, ram)
	AM_RANGE(0x080000, 0x080fff) AM_RAM AM_BASE_SIZE_GENERIC(spriteram)
	AM_RANGE(0x0a0000, 0x0a07ff) AM_RAM_WRITE(karnov_videoram_w) AM_BASE_MEMBER(karnov_state, videoram)
	AM_RANGE(0x0a0800, 0x0a0fff) AM_WRITE(karnov_videoram_w) /* Wndrplnt Mirror */
	AM_RANGE(0x0a1000, 0x0a17ff) AM_WRITEONLY AM_BASE_MEMBER(karnov_state, pf_data)
	AM_RANGE(0x0a1800, 0x0a1fff) AM_WRITE(karnov_playfield_swap_w)
	AM_RANGE(0x0c0000, 0x0c0007) AM_READ(karnov_control_r)
	AM_RANGE(0x0c0000, 0x0c000f) AM_WRITE(karnov_control_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( karnov_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x05ff) AM_RAM
	AM_RANGE(0x0800, 0x0800) AM_READ(soundlatch_r)
	AM_RANGE(0x1000, 0x1001) AM_DEVWRITE("ym1", ym2203_w)
	AM_RANGE(0x1800, 0x1801) AM_DEVWRITE("ym2", ym3526_w)
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( common )
	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED ) /* Button 4 on karnov schematics */

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED ) /* Button 4 on karnov schematics */

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED ) /* PL1 Button 5 on karnov schematics */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED ) /* PL2 Button 5 on karnov schematics */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_VBLANK )
INPUT_PORTS_END


static INPUT_PORTS_START( karnov )
	PORT_INCLUDE( common )

	PORT_START("FAKE")	/* Dummy input for i8751 */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_3C ) )
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0000, "SW1:5" )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Cocktail ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x0080, 0x0080, "SW1:8" )
	/* 0x0080 called No Die Mode according to the manual, but it doesn't seem to have any effect */

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0100, "1" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0200, "5" )
	PORT_DIPSETTING(      0x0000, "Infinite (Cheat)")
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0c00, "50 K" )
	PORT_DIPSETTING(      0x0800, "70 K" )
	PORT_DIPSETTING(      0x0400, "90 K" )
	PORT_DIPSETTING(      0x0000, "100 K" )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Timer Speed" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, "Fast" )
INPUT_PORTS_END

static INPUT_PORTS_START( wndrplnt )
	PORT_INCLUDE( common )

	PORT_START("FAKE")	/* Dummy input for i8751 */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_3C ) )
	PORT_DIPUNKNOWN( 0x0010, 0x0000 )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Cocktail ) )

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0100, "1" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0200, "5" )
	PORT_DIPSETTING(      0x0000, "Infinite (Cheat)")
	PORT_DIPUNKNOWN( 0x0400, 0x0400 )
	PORT_DIPUNKNOWN( 0x0800, 0x0800 )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Yes ) )
	PORT_DIPUNKNOWN( 0x2000, 0x2000 )
	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
INPUT_PORTS_END

static INPUT_PORTS_START( chelnov )
	PORT_INCLUDE( common )

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("FAKE")	/* Dummy input for i8751 */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("DSW")
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
	PORT_DIPUNKNOWN( 0x0010, 0x0010 )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Cocktail ) )

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0100, "1" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0200, "5" )
	PORT_DIPSETTING(      0x0000, "Infinite (Cheat)")
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Yes ) )
	PORT_DIPUNKNOWN( 0x2000, 0x2000 )
	PORT_DIPNAME( 0x4000, 0x4000, "Freeze" )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED( 0x8000, 0x8000 )
INPUT_PORTS_END

static INPUT_PORTS_START( chelnovu )
	PORT_INCLUDE( common )

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("FAKE")	/* Dummy input for i8751 */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_3C ) )
	PORT_DIPUNKNOWN( 0x0010, 0x0010 )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Cocktail ) )

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0100, "1" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0200, "5" )
	PORT_DIPSETTING(      0x0000, "Infinite (Cheat)")
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Yes ) )
	PORT_DIPUNKNOWN( 0x2000, 0x2000 )
	PORT_DIPNAME( 0x4000, 0x4000, "Freeze" )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED( 0x8000, 0x8000 )
INPUT_PORTS_END


/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout chars =
{
	8,8,
	1024,
	3,
	{ 0x6000*8,0x4000*8,0x2000*8 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8	/* every sprite takes 8 consecutive bytes */
};

static const gfx_layout sprites =
{
	16,16,
	4096,
	4,
	{ 0x60000*8,0x00000*8,0x20000*8,0x40000*8 },
	{ 16*8, 1+(16*8), 2+(16*8), 3+(16*8), 4+(16*8), 5+(16*8), 6+(16*8), 7+(16*8),
	0,1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 ,8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8},
	16*16
};


/* 16x16 tiles, 4 Planes, each plane is 0x10000 bytes */
static const gfx_layout tiles =
{
	16,16,
	2048,
	4,
	{ 0x30000*8,0x00000*8,0x10000*8,0x20000*8 },
	{ 16*8, 1+(16*8), 2+(16*8), 3+(16*8), 4+(16*8), 5+(16*8), 6+(16*8), 7+(16*8),
	0,1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 ,8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8},
	16*16
};

static GFXDECODE_START( karnov )
	GFXDECODE_ENTRY( "gfx1", 0, chars,     0,  4 )	/* colors 0-31 */
	GFXDECODE_ENTRY( "gfx2", 0, tiles,   512, 16 )	/* colors 512-767 */
	GFXDECODE_ENTRY( "gfx3", 0, sprites, 256, 16 )	/* colors 256-511 */
GFXDECODE_END


/*************************************
 *
 *  Interrupt generator
 *
 *************************************/

static INTERRUPT_GEN( karnov_interrupt )
{
	karnov_state *state = (karnov_state *)device->machine->driver_data;

	/* Coin input to the i8751 generates an interrupt to the main cpu */
	if (input_port_read(device->machine, "FAKE") == state->coin_mask)
		state->latch = 1;

	if (input_port_read(device->machine, "FAKE") != state->coin_mask && state->latch)
	{
		if (state->i8751_needs_ack)
		{
			/* i8751 is busy - queue the command */
			state->i8751_coin_pending = input_port_read(device->machine, "FAKE") | 0x8000;
		}
		else
		{
			state->i8751_return = input_port_read(device->machine, "FAKE") | 0x8000;
			cpu_set_input_line(device, 6, HOLD_LINE);
			state->i8751_needs_ack = 1;
		}

		state->latch = 0;
	}

	cpu_set_input_line(device, 7, HOLD_LINE);	/* VBL */
}

static void sound_irq( running_device *device, int linestate )
{
	karnov_state *state = (karnov_state *)device->machine->driver_data;
	cpu_set_input_line(state->audiocpu, 0, linestate); /* IRQ */
}

static const ym3526_interface ym3526_config =
{
	sound_irq
};

/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_START( karnov )
{
	karnov_state *state = (karnov_state *)machine->driver_data;

	state->maincpu = machine->device("maincpu");
	state->audiocpu = machine->device("audiocpu");

	state_save_register_global(machine, state->flipscreen);
	state_save_register_global_array(machine, state->scroll);

	state_save_register_global(machine, state->i8751_return);
	state_save_register_global(machine, state->i8751_needs_ack);
	state_save_register_global(machine, state->i8751_coin_pending);
	state_save_register_global(machine, state->i8751_command_queue);
	state_save_register_global(machine, state->i8751_level);
	state_save_register_global(machine, state->latch);

}

static MACHINE_RESET( karnov )
{
	karnov_state *state = (karnov_state *)machine->driver_data;

	memset(state->ram, 0, 0x4000 / 2); /* Chelnov likes ram clear on reset.. */

	state->i8751_return = 0;
	state->i8751_needs_ack = 0;
	state->i8751_coin_pending = 0;
	state->i8751_command_queue = 0;
	state->i8751_level = 0;
//  state->latch = 0;

	state->flipscreen = 0;
	state->scroll[0] = 0;
	state->scroll[0] = 0;
}


static MACHINE_DRIVER_START( karnov )

	/* driver data */
	MDRV_DRIVER_DATA(karnov_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 10000000)	/* 10 MHz */
	MDRV_CPU_PROGRAM_MAP(karnov_map)
	MDRV_CPU_VBLANK_INT("screen", karnov_interrupt)

	MDRV_CPU_ADD("audiocpu", M6502, 1500000)	/* Accurate */
	MDRV_CPU_PROGRAM_MAP(karnov_sound_map)

	MDRV_MACHINE_START(karnov)
	MDRV_MACHINE_RESET(karnov)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_BUFFERS_SPRITERAM)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(karnov)
	MDRV_PALETTE_LENGTH(1024)

	MDRV_PALETTE_INIT(karnov)
	MDRV_VIDEO_START(karnov)
	MDRV_VIDEO_UPDATE(karnov)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ym1", YM2203, 1500000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MDRV_SOUND_ADD("ym2", YM3526, 3000000)
	MDRV_SOUND_CONFIG(ym3526_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( wndrplnt )

	/* driver data */
	MDRV_DRIVER_DATA(karnov_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 10000000)	/* 10 MHz */
	MDRV_CPU_PROGRAM_MAP(karnov_map)
	MDRV_CPU_VBLANK_INT("screen", karnov_interrupt)

	MDRV_CPU_ADD("audiocpu", M6502, 1500000)	/* Accurate */
	MDRV_CPU_PROGRAM_MAP(karnov_sound_map)

	MDRV_MACHINE_START(karnov)
	MDRV_MACHINE_RESET(karnov)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_BUFFERS_SPRITERAM)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(karnov)
	MDRV_PALETTE_LENGTH(1024)

	MDRV_PALETTE_INIT(karnov)
	MDRV_VIDEO_START(wndrplnt)
	MDRV_VIDEO_UPDATE(karnov)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ym1", YM2203, 1500000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MDRV_SOUND_ADD("ym2", YM3526, 3000000)
	MDRV_SOUND_CONFIG(ym3526_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( karnov )
	ROM_REGION( 0x60000, "maincpu", 0 )	/* 6*64k for 68000 code */
	ROM_LOAD16_BYTE( "dn08-5",       0x00000, 0x10000, CRC(db92c264) SHA1(bd4bcd984a3455eedd2b78dc2090c9d625025671) )
	ROM_LOAD16_BYTE( "dn11-5",       0x00001, 0x10000, CRC(05669b4b) SHA1(c78d0da5afc66750dd9841a7d4f8f244d878c081) )
	ROM_LOAD16_BYTE( "dn07-",        0x20000, 0x10000, CRC(fc14291b) SHA1(c92207cf70d4c887cd0f53208e8090c7f614c1d3) )
	ROM_LOAD16_BYTE( "dn10-",        0x20001, 0x10000, CRC(a4a34e37) SHA1(f40b680cc7312c844f81d01997f9a47c48d36e88) )
	ROM_LOAD16_BYTE( "dn06-5",       0x40000, 0x10000, CRC(29d64e42) SHA1(c07ff5f29b7ccd5fc97b5086bcae57ab6eb29330) )
	ROM_LOAD16_BYTE( "dn09-5",       0x40001, 0x10000, CRC(072d7c49) SHA1(92195b89274d066a9c1f87dd810683ea66edaff4) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 6502 Sound CPU */
	ROM_LOAD( "dn05-5",       0x8000, 0x8000, CRC(fa1a31a8) SHA1(5007a625be03c546d2a78444d72c28761b10cdb0) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "dn00-",        0x00000, 0x08000, CRC(0ed77c6d) SHA1(4ec86ac56c01c158a580dc13dea3e5cbdf90d0e9) )	/* Characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "dn04-",        0x00000, 0x10000, CRC(a9121653) SHA1(04a67ba6fcf551719734ba2b86ee49c37ee1b842) )	/* Backgrounds */
	ROM_LOAD( "dn01-",        0x10000, 0x10000, CRC(18697c9e) SHA1(b454af7922c4b1a651d303a3d8d89e5cc102f9ca) )
	ROM_LOAD( "dn03-",        0x20000, 0x10000, CRC(90d9dd9c) SHA1(00a3bed276927f099d57e90f28fd77bd41a3c360) )
	ROM_LOAD( "dn02-",        0x30000, 0x10000, CRC(1e04d7b9) SHA1(a2c6fde42569a52cc6d9a86715dea4a8bea80092) )

	ROM_REGION( 0x80000, "gfx3", 0 )
	ROM_LOAD( "dn12-",        0x00000, 0x10000, CRC(9806772c) SHA1(01f17fa033262a3e64e0675cc4e20b3c3f4b254d) )	/* Sprites - 2 sets of 4, interleaved here */
	ROM_LOAD( "dn14-5",       0x10000, 0x08000, CRC(ac9e6732) SHA1(6f61344eb8a13349471145dee252a01aadb8cdf0) )
	ROM_LOAD( "dn13-",        0x20000, 0x10000, CRC(a03308f9) SHA1(1d450725a5c488332c83d8f64a73a750ce7fe4c7) )
	ROM_LOAD( "dn15-5",       0x30000, 0x08000, CRC(8933fcb8) SHA1(0dbda4b032ed3776d7633264f39e6f00ace7a238) )
	ROM_LOAD( "dn16-",        0x40000, 0x10000, CRC(55e63a11) SHA1(3ef0468fa02ac5382007428122216917ad5eaa0e) )
	ROM_LOAD( "dn17-5",       0x50000, 0x08000, CRC(b70ae950) SHA1(1ec833bdad12710ea846ef48dddbe2e1ae6b8ce1) )
	ROM_LOAD( "dn18-",        0x60000, 0x10000, CRC(2ad53213) SHA1(f22696920bf3d74fb0e28e2d7cb31be5e183c6b4) )
	ROM_LOAD( "dn19-5",       0x70000, 0x08000, CRC(8fd4fa40) SHA1(1870fb0c5c64fbc53a10115f0f3c7624cf2465db) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "karnprom.21",  0x0000, 0x0400, CRC(aab0bb93) SHA1(545707fbb1007fca1fe297c5fce61e485e7084fc) )
	ROM_LOAD( "karnprom.20",  0x0400, 0x0400, CRC(02f78ffb) SHA1(cb4dd8b0ce3c404195321b17e10f51352f506958) )
ROM_END

ROM_START( karnovj )
	ROM_REGION( 0x60000, "maincpu", 0 )	/* 6*64k for 68000 code */
	ROM_LOAD16_BYTE( "kar8",         0x00000, 0x10000, CRC(3e17e268) SHA1(3a63928bb0148175519540f9d891b03590094dfb) )
	ROM_LOAD16_BYTE( "kar11",        0x00001, 0x10000, CRC(417c936d) SHA1(d31f9291f18c3d5e3c4430768396e1ac10fd9ea3) )
	ROM_LOAD16_BYTE( "dn07-",        0x20000, 0x10000, CRC(fc14291b) SHA1(c92207cf70d4c887cd0f53208e8090c7f614c1d3) )
	ROM_LOAD16_BYTE( "dn10-",        0x20001, 0x10000, CRC(a4a34e37) SHA1(f40b680cc7312c844f81d01997f9a47c48d36e88) )
	ROM_LOAD16_BYTE( "kar6",         0x40000, 0x10000, CRC(c641e195) SHA1(fa7a2eba70e730f72a8d868160af9c41f9b2e5b0) )
	ROM_LOAD16_BYTE( "kar9",         0x40001, 0x10000, CRC(d420658d) SHA1(4c7e67a80e419b8b94eb015f7f0af0a01f00c28e) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 6502 Sound CPU */
	ROM_LOAD( "kar5",         0x8000, 0x8000, CRC(7c9158f1) SHA1(dfba7b3abd6b8d6991f0207cd252ee652a6050c2) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "dn00-",        0x00000, 0x08000, CRC(0ed77c6d) SHA1(4ec86ac56c01c158a580dc13dea3e5cbdf90d0e9) )	/* Characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "dn04-",        0x00000, 0x10000, CRC(a9121653) SHA1(04a67ba6fcf551719734ba2b86ee49c37ee1b842) )	/* Backgrounds */
	ROM_LOAD( "dn01-",        0x10000, 0x10000, CRC(18697c9e) SHA1(b454af7922c4b1a651d303a3d8d89e5cc102f9ca) )
	ROM_LOAD( "dn03-",        0x20000, 0x10000, CRC(90d9dd9c) SHA1(00a3bed276927f099d57e90f28fd77bd41a3c360) )
	ROM_LOAD( "dn02-",        0x30000, 0x10000, CRC(1e04d7b9) SHA1(a2c6fde42569a52cc6d9a86715dea4a8bea80092) )

	ROM_REGION( 0x80000, "gfx3", 0 )
	ROM_LOAD( "dn12-",        0x00000, 0x10000, CRC(9806772c) SHA1(01f17fa033262a3e64e0675cc4e20b3c3f4b254d) )	/* Sprites - 2 sets of 4, interleaved here */
	ROM_LOAD( "kar14",        0x10000, 0x08000, CRC(c6b39595) SHA1(3bc2d0a613cc1b5d255cccc3b26e21ea1c23e75b) )
	ROM_LOAD( "dn13-",        0x20000, 0x10000, CRC(a03308f9) SHA1(1d450725a5c488332c83d8f64a73a750ce7fe4c7) )
	ROM_LOAD( "kar15",        0x30000, 0x08000, CRC(2f72cac0) SHA1(a71e61eea77ecd3240c5217ae84e7aa3ef21288a) )
	ROM_LOAD( "dn16-",        0x40000, 0x10000, CRC(55e63a11) SHA1(3ef0468fa02ac5382007428122216917ad5eaa0e) )
	ROM_LOAD( "kar17",        0x50000, 0x08000, CRC(7851c70f) SHA1(47b7a64dd8230e95cd7ae7f661c7586c7598c356) )
	ROM_LOAD( "dn18-",        0x60000, 0x10000, CRC(2ad53213) SHA1(f22696920bf3d74fb0e28e2d7cb31be5e183c6b4) )
	ROM_LOAD( "kar19",        0x70000, 0x08000, CRC(7bc174bb) SHA1(d8bc320169fc3a9cdd3f271ea523fb0486abae2c) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "karnprom.21",  0x0000, 0x0400, CRC(aab0bb93) SHA1(545707fbb1007fca1fe297c5fce61e485e7084fc) )
	ROM_LOAD( "karnprom.20",  0x0400, 0x0400, CRC(02f78ffb) SHA1(cb4dd8b0ce3c404195321b17e10f51352f506958) )
ROM_END

ROM_START( wndrplnt )
	ROM_REGION( 0x60000, "maincpu", 0 )	/* 6*64k for 68000 code */
	ROM_LOAD16_BYTE( "ea08.bin",   0x00000, 0x10000, CRC(b0578a14) SHA1(a420d1e8f80405161c86a123610ddf17c7ff07ff) )
	ROM_LOAD16_BYTE( "ea11.bin",   0x00001, 0x10000, CRC(271edc6c) SHA1(6aa411fa4a3613018e7d971c5675f54d5765904d) )
	ROM_LOAD16_BYTE( "ea07.bin",   0x20000, 0x10000, CRC(7095a7d5) SHA1(a7ee88cad03690a72a52b8ea2310416aa53febdd) )
	ROM_LOAD16_BYTE( "ea10.bin",   0x20001, 0x10000, CRC(81a96475) SHA1(2d2e647ed7867b1a7f0dc24544e241e4b1c9fa92) )
	ROM_LOAD16_BYTE( "ea06.bin",   0x40000, 0x10000, CRC(5951add3) SHA1(394552c29a6266becbdb36c3bd65fc1f56701d11) )
	ROM_LOAD16_BYTE( "ea09.bin",   0x40001, 0x10000, CRC(c4b3cb1e) SHA1(006becbcdbbb3e666382e59e8fa5a5ebe06e5724) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* 6502 Sound CPU */
	ROM_LOAD( "ea05.bin",     0x8000, 0x8000, CRC(8dbb6231) SHA1(342faa020448ce916e820b3df18d44191983f7a6) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "ea00.bin",    0x00000, 0x08000, CRC(9f3cac4c) SHA1(af8a275ff531029dbada3c820c9f660fef383100) )	/* Characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "ea04.bin",    0x00000, 0x10000, CRC(7d701344) SHA1(4efaa73a4b2534078ee25111a2f5143c7c7e846f) )	/* Backgrounds */
	ROM_LOAD( "ea01.bin",    0x10000, 0x10000, CRC(18df55fb) SHA1(406ea47365ff8372bb2588c97c438ea02aa17538) )
	ROM_LOAD( "ea03.bin",    0x20000, 0x10000, CRC(922ef050) SHA1(e33aea6df2e1a14bd371ed0a2b172f58edcc0e8e) )
	ROM_LOAD( "ea02.bin",    0x30000, 0x10000, CRC(700fde70) SHA1(9b5b59aaffac091622329dc6ebedb24806b69964) )

	ROM_REGION( 0x80000, "gfx3", 0 )
	ROM_LOAD( "ea12.bin",    0x00000, 0x10000, CRC(a6d4e99d) SHA1(a85dbb23d05d1e386d8a66f505fa9dfcc554327b) )	/* Sprites - 2 sets of 4, interleaved here */
	ROM_LOAD( "ea14.bin",    0x10000, 0x10000, CRC(915ffdc9) SHA1(b65cdc8ee953494f2b69e06cd6c97ee142d83c3e) )
	ROM_LOAD( "ea13.bin",    0x20000, 0x10000, CRC(cd839f3a) SHA1(7eae3a1e080b7db22968d556e80b620cb07976b0) )
	ROM_LOAD( "ea15.bin",    0x30000, 0x10000, CRC(a1f14f16) SHA1(5beb2b8967aa34271f734865704c6bab07d76a8c) )
	ROM_LOAD( "ea16.bin",    0x40000, 0x10000, CRC(7a1d8a9c) SHA1(2b924a7e5a2490a7144b981155f2503d3737875d) )
	ROM_LOAD( "ea17.bin",    0x50000, 0x10000, CRC(21a3223d) SHA1(7754ed9cbe4eed94b49130af6108e919be18e5b3) )
	ROM_LOAD( "ea18.bin",    0x60000, 0x10000, CRC(3fb2cec7) SHA1(7231bb728f1009186d41e177402e84b63f25a44f) )
	ROM_LOAD( "ea19.bin",    0x70000, 0x10000, CRC(87cf03b5) SHA1(29bc25642be1dd7e25f13e96dae90572f7a09d21) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "ea21.prm",      0x0000, 0x0400, CRC(c8beab49) SHA1(970c2bad3cbf2d7fc313997ae0fe11dd04383b40) )
	ROM_LOAD( "ea20.prm",      0x0400, 0x0400, CRC(619f9d1e) SHA1(17fe49b6c9ce17be4a03e3400229e3ef4998a46f) )
ROM_END

ROM_START( chelnovu )
	ROM_REGION( 0x60000, "maincpu", 0 )	/* 6*64k for 68000 code */
	ROM_LOAD16_BYTE( "ee08-a.j15",   0x00000, 0x10000, CRC(2f2fb37b) SHA1(f89b424099097a95cf184d20a15b876c5b639552) )
	ROM_LOAD16_BYTE( "ee11-a.j20",   0x00001, 0x10000, CRC(f306d05f) SHA1(e523ffd17fb0104fe28eac288b6ebf7fc0ea2908) )
	ROM_LOAD16_BYTE( "ee07-a.j14",   0x20000, 0x10000, CRC(9c69ed56) SHA1(23606d2fc7c550eaddf0fd4b0da1a4e2c9263e14) )
	ROM_LOAD16_BYTE( "ee10-a.j18",   0x20001, 0x10000, CRC(d5c5fe4b) SHA1(183b2f5dfa4e0a9067674a29abab2744a887fd19) )
	ROM_LOAD16_BYTE( "ee06-e.j13",   0x40000, 0x10000, CRC(55acafdb) SHA1(9dc0528c888dd73617f8cab76690b9296715680a) )
	ROM_LOAD16_BYTE( "ee09-e.j17",   0x40001, 0x10000, CRC(303e252c) SHA1(d5d2570e42aa1e1b3600d14cc694677248e12750) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* 6502 Sound CPU */
	ROM_LOAD( "ee05-.f3",     0x8000, 0x8000, CRC(6a8936b4) SHA1(2b72cb749e6bddb67c2bd3d27b3a92511f9ef016) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "ee00-e.c5",    0x00000, 0x08000, CRC(e06e5c6b) SHA1(70166257da5be428cb8404d8e1063c59c7722365) )	/* Characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "ee04-.d18",    0x00000, 0x10000, CRC(96884f95) SHA1(9d88d203028288cb26e111880d090bf40ef9385b) )	/* Backgrounds */
	ROM_LOAD( "ee01-.c15",    0x10000, 0x10000, CRC(f4b54057) SHA1(72cd0b098a465232c2148fe6b4224c42dd42e6bc) )
	ROM_LOAD( "ee03-.d15",    0x20000, 0x10000, CRC(7178e182) SHA1(e8f03bda417e8f2f0508df40057d39ce6ee74f16) )
	ROM_LOAD( "ee02-.c18",    0x30000, 0x10000, CRC(9d7c45ae) SHA1(014dfafa6898e5fd0d124391e698b4f76d38fa94) )

	ROM_REGION( 0x80000, "gfx3", 0 )
	ROM_LOAD( "ee12-.f8",     0x00000, 0x10000, CRC(9b1c53a5) SHA1(b0fdc89dc7fd0931fa4bca3bbc20fc88f637ec74) )	/* Sprites */
	ROM_LOAD( "ee13-.f9",     0x20000, 0x10000, CRC(72b8ae3e) SHA1(535dfd70e6d13296342d96917a57d46bdb28a59e) )
	ROM_LOAD( "ee14-.f13",    0x40000, 0x10000, CRC(d8f4bbde) SHA1(1f2d336dd97c9cc39e124c18cae634afb0ef3316) )
	ROM_LOAD( "ee15-.f15",    0x60000, 0x10000, CRC(81e3e68b) SHA1(1059c70b8bfe09c212a19767cfe23efa22afc196) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "ee21.k8",      0x0000, 0x0400, CRC(b1db6586) SHA1(a7ecfcb4cf0f7450900820b3dfad8813efedfbea) )	/* different from the other set; */
															/* might be bad */
	ROM_LOAD( "ee20.l6",      0x0400, 0x0400, CRC(41816132) SHA1(89a1194bd8bf39f13419df685e489440bdb05676) )
ROM_END

ROM_START( chelnov )
	ROM_REGION( 0x60000, "maincpu", 0 )	/* 6*64k for 68000 code */
	ROM_LOAD16_BYTE( "ee08-e.j16",   0x00000, 0x10000, CRC(8275cc3a) SHA1(961166226b68744eef15fed6a306010757b83556) )
	ROM_LOAD16_BYTE( "ee11-e.j19",   0x00001, 0x10000, CRC(889e40a0) SHA1(e927f32d9bc448a331fb7b3478b2d07154f5013b) )
	ROM_LOAD16_BYTE( "a-j14.bin",    0x20000, 0x10000, CRC(51465486) SHA1(e165e754eb756db3abc1f8477171ab817d03a890) )
	ROM_LOAD16_BYTE( "a-j18.bin",    0x20001, 0x10000, CRC(d09dda33) SHA1(1764215606eec61e4fe30c0fc82ea2faf17821dc) )
	ROM_LOAD16_BYTE( "ee06-e.j13",   0x40000, 0x10000, CRC(55acafdb) SHA1(9dc0528c888dd73617f8cab76690b9296715680a) )
	ROM_LOAD16_BYTE( "ee09-e.j17",   0x40001, 0x10000, CRC(303e252c) SHA1(d5d2570e42aa1e1b3600d14cc694677248e12750) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* 6502 Sound CPU */
	ROM_LOAD( "ee05-.f3",     0x8000, 0x8000, CRC(6a8936b4) SHA1(2b72cb749e6bddb67c2bd3d27b3a92511f9ef016) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "ee00-e.c5",    0x00000, 0x08000, CRC(e06e5c6b) SHA1(70166257da5be428cb8404d8e1063c59c7722365) )	/* Characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "ee04-.d18",    0x00000, 0x10000, CRC(96884f95) SHA1(9d88d203028288cb26e111880d090bf40ef9385b) )	/* Backgrounds */
	ROM_LOAD( "ee01-.c15",    0x10000, 0x10000, CRC(f4b54057) SHA1(72cd0b098a465232c2148fe6b4224c42dd42e6bc) )
	ROM_LOAD( "ee03-.d15",    0x20000, 0x10000, CRC(7178e182) SHA1(e8f03bda417e8f2f0508df40057d39ce6ee74f16) )
	ROM_LOAD( "ee02-.c18",    0x30000, 0x10000, CRC(9d7c45ae) SHA1(014dfafa6898e5fd0d124391e698b4f76d38fa94) )

	ROM_REGION( 0x80000, "gfx3", 0 )
	ROM_LOAD( "ee12-.f8",     0x00000, 0x10000, CRC(9b1c53a5) SHA1(b0fdc89dc7fd0931fa4bca3bbc20fc88f637ec74) )	/* Sprites */
	ROM_LOAD( "ee13-.f9",     0x20000, 0x10000, CRC(72b8ae3e) SHA1(535dfd70e6d13296342d96917a57d46bdb28a59e) )
	ROM_LOAD( "ee14-.f13",    0x40000, 0x10000, CRC(d8f4bbde) SHA1(1f2d336dd97c9cc39e124c18cae634afb0ef3316) )
	ROM_LOAD( "ee15-.f15",    0x60000, 0x10000, CRC(81e3e68b) SHA1(1059c70b8bfe09c212a19767cfe23efa22afc196) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "ee21.k8",      0x0000, 0x0400, CRC(b1db6586) SHA1(a7ecfcb4cf0f7450900820b3dfad8813efedfbea) )	/* different from the other set; */
															/* might be bad */
	ROM_LOAD( "ee20.l6",      0x0400, 0x0400, CRC(41816132) SHA1(89a1194bd8bf39f13419df685e489440bdb05676) )
ROM_END

ROM_START( chelnovj )
	ROM_REGION( 0x60000, "maincpu", 0 )	/* 6*64k for 68000 code */
	ROM_LOAD16_BYTE( "a-j15.bin",    0x00000, 0x10000, CRC(1978cb52) SHA1(833b8e80445ec2384e0479afb7430b32d6a14441) )
	ROM_LOAD16_BYTE( "a-j20.bin",    0x00001, 0x10000, CRC(e0ed3d99) SHA1(f47aaec5c72ecc308c32cdcf117ef4965ac5ea61) )
	ROM_LOAD16_BYTE( "a-j14.bin",    0x20000, 0x10000, CRC(51465486) SHA1(e165e754eb756db3abc1f8477171ab817d03a890) )
	ROM_LOAD16_BYTE( "a-j18.bin",    0x20001, 0x10000, CRC(d09dda33) SHA1(1764215606eec61e4fe30c0fc82ea2faf17821dc) )
	ROM_LOAD16_BYTE( "a-j13.bin",    0x40000, 0x10000, CRC(cd991507) SHA1(9da858ea41bfbce78496c086e3b462ea9f3722e8) )
	ROM_LOAD16_BYTE( "a-j17.bin",    0x40001, 0x10000, CRC(977f601c) SHA1(b40a37160b493dcb614922c2a9b4b5f140b62aca) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* 6502 Sound CPU */
	ROM_LOAD( "ee05-.f3",     0x8000, 0x8000, CRC(6a8936b4) SHA1(2b72cb749e6bddb67c2bd3d27b3a92511f9ef016) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "a-c5.bin",     0x00000, 0x08000, CRC(1abf2c6d) SHA1(86d625ae94cd9ea69e4e613895410640efb175b3) )	/* Characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "ee04-.d18",    0x00000, 0x10000, CRC(96884f95) SHA1(9d88d203028288cb26e111880d090bf40ef9385b) )	/* Backgrounds */
	ROM_LOAD( "ee01-.c15",    0x10000, 0x10000, CRC(f4b54057) SHA1(72cd0b098a465232c2148fe6b4224c42dd42e6bc) )
	ROM_LOAD( "ee03-.d15",    0x20000, 0x10000, CRC(7178e182) SHA1(e8f03bda417e8f2f0508df40057d39ce6ee74f16) )
	ROM_LOAD( "ee02-.c18",    0x30000, 0x10000, CRC(9d7c45ae) SHA1(014dfafa6898e5fd0d124391e698b4f76d38fa94) )

	ROM_REGION( 0x80000, "gfx3", 0 )
	ROM_LOAD( "ee12-.f8",     0x00000, 0x10000, CRC(9b1c53a5) SHA1(b0fdc89dc7fd0931fa4bca3bbc20fc88f637ec74) )	/* Sprites */
	ROM_LOAD( "ee13-.f9",     0x20000, 0x10000, CRC(72b8ae3e) SHA1(535dfd70e6d13296342d96917a57d46bdb28a59e) )
	ROM_LOAD( "ee14-.f13",    0x40000, 0x10000, CRC(d8f4bbde) SHA1(1f2d336dd97c9cc39e124c18cae634afb0ef3316) )
	ROM_LOAD( "ee15-.f15",    0x60000, 0x10000, CRC(81e3e68b) SHA1(1059c70b8bfe09c212a19767cfe23efa22afc196) )

	ROM_REGION( 0x0800, "proms", 0 )
	ROM_LOAD( "a-k7.bin",     0x0000, 0x0400, CRC(309c49d8) SHA1(7220002f6ef97514b4e6f61706fc16061120dafa) )	/* different from the other set; */
															/* might be bad */
	ROM_LOAD( "ee20.l6",      0x0400, 0x0400, CRC(41816132) SHA1(89a1194bd8bf39f13419df685e489440bdb05676) )
ROM_END


/*************************************
 *
 *  Driver initialization
 *
 *************************************/

static DRIVER_INIT( karnov )
{
	karnov_state *state = (karnov_state *)machine->driver_data;
	state->microcontroller_id = KARNOV;
	state->coin_mask = 0;
}

static DRIVER_INIT( karnovj )
{
	karnov_state *state = (karnov_state *)machine->driver_data;
	state->microcontroller_id = KARNOVJ;
	state->coin_mask = 0;
}

static DRIVER_INIT( wndrplnt )
{
	karnov_state *state = (karnov_state *)machine->driver_data;
	state->microcontroller_id = WNDRPLNT;
	state->coin_mask = 0;
}

static DRIVER_INIT( chelnov )
{
	karnov_state *state = (karnov_state *)machine->driver_data;
	UINT16 *RAM = (UINT16 *)memory_region(machine, "maincpu");

	state->microcontroller_id = CHELNOV;
	state->coin_mask = 0xe0;
	RAM[0x0a26/2] = 0x4e71;  /* removes a protection lookup table */
	RAM[0x062a/2] = 0x4e71;  /* hangs waiting on i8751 int */
}

static DRIVER_INIT( chelnovw )
{
	karnov_state *state = (karnov_state *)machine->driver_data;
	UINT16 *RAM = (UINT16 *)memory_region(machine, "maincpu");

	state->microcontroller_id = CHELNOVW;
	state->coin_mask = 0xe0;
	RAM[0x0a26/2] = 0x4e71;  /* removes a protection lookup table */
	RAM[0x062a/2] = 0x4e71;  /* hangs waiting on i8751 int */
}

static DRIVER_INIT( chelnovj )
{
	karnov_state *state = (karnov_state *)machine->driver_data;
	UINT16 *RAM = (UINT16 *)memory_region(machine, "maincpu");

	state->microcontroller_id = CHELNOVJ;
	state->coin_mask = 0xe0;
	RAM[0x0a2e/2] = 0x4e71;  /* removes a protection lookup table */
	RAM[0x062a/2] = 0x4e71;  /* hangs waiting on i8751 int */
}


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1987, karnov,   0,       karnov,   karnov,   karnov,   ROT0,   "Data East USA",         "Karnov (US)", GAME_SUPPORTS_SAVE )
GAME( 1987, karnovj,  karnov,  karnov,   karnov,   karnovj,  ROT0,   "Data East Corporation", "Karnov (Japan)", GAME_SUPPORTS_SAVE )
GAME( 1987, wndrplnt, 0,       wndrplnt, wndrplnt, wndrplnt, ROT270, "Data East Corporation", "Wonder Planet (Japan)", GAME_SUPPORTS_SAVE )
GAME( 1988, chelnov,  0,       karnov,   chelnov,  chelnovw, ROT0,   "Data East Corporation", "Chelnov - Atomic Runner (World)", GAME_SUPPORTS_SAVE )
GAME( 1988, chelnovu, chelnov, karnov,   chelnovu, chelnov,  ROT0,   "Data East USA",         "Chelnov - Atomic Runner (US)", GAME_SUPPORTS_SAVE )
GAME( 1988, chelnovj, chelnov, karnov,   chelnovu, chelnovj, ROT0,   "Data East Corporation", "Chelnov - Atomic Runner (Japan)", GAME_SUPPORTS_SAVE )
