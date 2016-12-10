/***************************************************************************

    Grand Champion
    (C) 1981 Taito

    MAME driver by Ernesto Corvi and Phil Stroffolino

****************************************************************************

    Known Issues:

    -   pc3259 handling (for collision detection) is not accurate.
        Collision detection does work, but there are bits which define
        the type of collision (and determine whether a pit stop is
        required) that I don't know how to handle.

    -   sound: missing engine noise

    -   "radar" is probably wrong

    -   LED and tachometer display are missing/faked
        Note that a dipswitch setting allows score to be displayed
        onscreen, but there's no equivalent for tachometer.

    Notes:

    -   The object of the game is to avoid the opposing cars.

    -   The player has to drive through Dark Tunnels, Rain,
        Lightning, Sleet, Snow, and a Track that suddenly
        divides to get to the Finish Line first.

    -   "GRAND CHAMPION" has a Radar Feature which enables
        the Player to see his position relative to the other
        cars.

    -   A Rank Feature is also provided which shows the
        Player's numerical rank all through the race.

    -   The Speech Feature enhances the game play.

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "includes/grchamp.h"

#include "grchamp.lh"


/*************************************
 *
 *  Constants
 *
 *************************************/

#define MASTER_CLOCK		(18432000)
#define SOUND_CLOCK			(6000000)

#define PIXEL_CLOCK			(MASTER_CLOCK/3)

#define HTOTAL				(384)
#define HBEND				(0)
#define HBSTART				(256)

#define VTOTAL				(264)
#define VBEND				(16)
#define VBSTART				(240)



/*************************************
 *
 *  Machine setup
 *
 *************************************/

static MACHINE_RESET( grchamp )
{
	/* if the coin system is 1 way, lock Coin B (Page 40) */
	coin_lockout_w(machine, 1, (input_port_read(machine, "DSWB") & 0x10) ? 1 : 0);
}



/*************************************
 *
 *  Interrupt generation
 *
 *************************************/

static INTERRUPT_GEN( grchamp_cpu0_interrupt )
{
	grchamp_state *state = (grchamp_state *)device->machine->driver_data;

	if (state->cpu0_out[0] & 0x01)
		cpu_set_input_line(device, 0, ASSERT_LINE);
}


static INTERRUPT_GEN( grchamp_cpu1_interrupt )
{
	grchamp_state *state = (grchamp_state *)device->machine->driver_data;

	if (state->cpu1_out[4] & 0x01)
		cpu_set_input_line(device, 0, ASSERT_LINE);
}



/*************************************
 *
 *  Main CPU outputs
 *
 *************************************/

static WRITE8_HANDLER( cpu0_outputs_w )
{
	grchamp_state *state = (grchamp_state *)space->machine->driver_data;
	UINT8 diff = data ^ state->cpu0_out[offset];
	state->cpu0_out[offset] = data;

	switch (offset)
	{
		case 0x00:	/* OUT0 */

			/* bit 0: low = clear IRQ on main CPU */
			/* bit 1: /HTCLR = clear collision detect */
			/* bit 4: HEAD LAMP */
			/* bit 5: CHANGE */
			/* bit 6: FOG OUT */
			/* bit 7: RADARON */
			if ((diff & 0x01) && !(data & 0x01))
				cputag_set_input_line(space->machine, "maincpu", 0, CLEAR_LINE);
			if ((diff & 0x02) && !(data & 0x02))
				state->collide = state->collmode = 0;
			break;

		case 0x01:	/* OUT1 */
			/* connects to pc3259, pin 23 (read collision data?) */
			state->collmode++;
			break;

		case 0x02:	/* OUT2 */
			/* bit 0-7: MYDH (car X position) */
			break;

		case 0x03:	/* OUT3 */
			/* bit 0-7: MYDV (car Y position) */
			break;

		case 0x04:	/* OUT4 */
			/* bit 0-3: player car tile select */
			/* bit 4-7: rain tile select */
			break;

		case 0x07:	/* OUT7 */
			/* bit 0-7: rain Y position */
			break;

		case 0x08:	/* OUT8 */
			/* bit 0-7: rain X position */
			break;

		case 0x09:	/* OUT9 */
			/* bit 0-3: n/c */
			/* bit 4:   coin lockout */
			/* bit 5:   Game Over lamp */
			/* bit 6-7: n/c */
			coin_lockout_global_w(space->machine, (data >> 4) & 1);
			output_set_value("led0", (~data >> 5) & 1);
			break;

		case 0x0a:	/* OUT10 */
			/* bit 0: n/c */
			/* bit 1: G-12 */
			/* bit 2: G-S */
			/* bit 3: G-16 */
			/* bit 4: G-Y */
			/* bit 5: n/c */
			/* bit 6: G-Z */
			if (diff)
				mame_printf_debug("OUT10=%02X\n", data);
			break;

		case 0x0d:	/* OUT13 */
			watchdog_reset(space->machine);
			break;

		case 0x0e:	/* OUT14 */
			/* O-21 connector */
			soundlatch_w(space, 0, data);
			cputag_set_input_line(space->machine, "audiocpu", INPUT_LINE_NMI, PULSE_LINE);
			break;
	}
}


static WRITE8_HANDLER( led_board_w )
{
	static const UINT8 ls247_map[16] =
		{ 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x58,0x4c,0x62,0x69,0x78,0x00 };
	grchamp_state *state = (grchamp_state *)space->machine->driver_data;

	switch (offset)
	{
		case 0x00:
			state->ledlatch = (state->ledlatch & 0xff00) | (data << 0);
			break;

		case 0x04:
			state->ledlatch = (state->ledlatch & 0x00ff) | (data << 8);
			break;

		case 0x08:
			state->ledaddr = data & 0x0f;
			break;

		case 0x0c:
			state->ledram[state->ledaddr & 0x07] = state->ledlatch;
			output_set_digit_value(state->ledaddr & 0x07, ls247_map[state->ledram[state->ledaddr & 0x07] & 0x0f]);
			/*
                ledram[0] & 0x0f = score LSD
                ledram[1] & 0x0f = score
                ledram[2] & 0x0f = score
                ledram[3] & 0x0f = score
                ledram[4] & 0x0f = score MSD
                ledram[5] & 0x0f = time LSD
                ledram[6] & 0x0f = time MSD
                ledram[7] & 0x0f = ???
            */
			break;
	}
}



/*************************************
 *
 *  Sub CPU outputs
 *
 *************************************/

static WRITE8_HANDLER( cpu1_outputs_w )
{
	running_device *discrete = space->machine->device("discrete");
	grchamp_state *state = (grchamp_state *)space->machine->driver_data;
	UINT8 diff = data ^ state->cpu1_out[offset];
	state->cpu1_out[offset] = data;

	switch (offset)
	{
		case 0x00:	/* OUT0 */
			/* bit 0-7: left/right synk bus xscroll LSBs */
			break;

		case 0x01:	/* OUT1 */
			/* bit 0: left/right synk bus xscroll MSB */
			break;

		case 0x02:	/* OUT2 */
			/* bit 0-7: left synk bus yscroll */
			break;

		case 0x03:	/* OUT3 */
			/* bit 0-3: analog tachometer output */
			/* bit 4:   palette MSB */
			/* bit 5:   disable the 256H line in the center tilemap */
			break;

		case 0x04:	/* OUT4 */
			/* bit 0:   interrupt enable for CPU 1 */
			if ((diff & 0x01) && !(data & 0x01))
				cputag_set_input_line(space->machine, "sub", 0, CLEAR_LINE);
			break;

		case 0x05:	/* OUT5 - unused */
			break;

		case 0x06:	/* OUT6 - unused */
			break;

		case 0x07:	/* OUT7 */
			/* bit 0-7: right synk bus yscroll */
			break;

		case 0x08:	/* OUT8 */
			/* bit 0-7: latches data to main CPU input port 2 */
			state->comm_latch = data;
			break;

		case 0x09:	/* OUT9 */
			/* bit 0-7: center synk bus xscroll LSBs */
			break;

		case 0x0a:	/* OUTA */
			/* bit 0: center synk bus xscroll MSB */
			break;

		case 0x0b:	/* OUTB */
			/* bit 0-7: center synk bus yscroll */
			break;

		case 0x0c: /* OUTC */
			/* bit 0:   FOG */
			/* bit 1:   IDLING */
			/* bit 2-4: ATTACK UP 1-3 */
			/* bit 5-6: SIFT 1-2 */
			/* bit 7:   ENGINE CS */
			discrete_sound_w(discrete, GRCHAMP_ENGINE_CS_EN, data & 0x80);
			discrete_sound_w(discrete, GRCHAMP_SIFT_DATA, (data >> 5) & 0x03);
			discrete_sound_w(discrete, GRCHAMP_ATTACK_UP_DATA, (data >> 2) & 0x07);
			discrete_sound_w(discrete, GRCHAMP_IDLING_EN, data & 0x02);
			discrete_sound_w(discrete, GRCHAMP_FOG_EN, data & 0x01);
			break;

		case 0x0d: /* OUTD */
			/* bit 0-3: ATTACK SPEED 1-4 */
			/* bit 4-7: PLAYER SPEED 1-4 */
			discrete_sound_w(discrete, GRCHAMP_PLAYER_SPEED_DATA, (data >> 4) & 0x0f);
			discrete_sound_w(discrete, GRCHAMP_ATTACK_SPEED_DATA,  data & 0x0f);
			break;

		default:
			/* OUTE - Page 48: goes to connector Q-27 */
			/* OUTF - unused */
			break;
	}
}



/*************************************
 *
 *  CPU 0 Inputs
 *
 *************************************/

INLINE UINT8 get_pc3259_bits(running_machine *machine, grchamp_state *state, int offs)
{
	int bits;

	/* force a partial update to the current position */
	machine->primary_screen->update_partial(machine->primary_screen->vpos());

	/* get the relevant 4 bits */
	bits = (state->collide >> (offs*4)) & 0x0f;

	/* replicate to both nibbles */
	return bits | (bits << 4);
}


static READ8_HANDLER( pc3259_0_r )
{
	grchamp_state *state = (grchamp_state *)space->machine->driver_data;
	return get_pc3259_bits(space->machine, state, 0);
}


static READ8_HANDLER( pc3259_1_r )
{
	grchamp_state *state = (grchamp_state *)space->machine->driver_data;
	return get_pc3259_bits(space->machine, state, 1);
}


static READ8_HANDLER( pc3259_2_r )
{
	grchamp_state *state = (grchamp_state *)space->machine->driver_data;
	return get_pc3259_bits(space->machine, state, 2);
}


static READ8_HANDLER( pc3259_3_r )
{
	grchamp_state *state = (grchamp_state *)space->machine->driver_data;
	return get_pc3259_bits(space->machine, state, 3);
}



/*************************************
 *
 *  Inter-CPU communication
 *
 *************************************/

static READ8_HANDLER( sub_to_main_comm_r )
{
	grchamp_state *state = (grchamp_state *)space->machine->driver_data;
	return state->comm_latch;
}


static TIMER_CALLBACK( main_to_sub_comm_sync_w )
{
	grchamp_state *state = (grchamp_state *)machine->driver_data;
	int offset = param >> 8;
	state->comm_latch2[offset & 3] = param;
}


static WRITE8_HANDLER( main_to_sub_comm_w )
{
	timer_call_after_resynch(space->machine, NULL, data | (offset << 8), main_to_sub_comm_sync_w);
}


static READ8_HANDLER( main_to_sub_comm_r )
{
	grchamp_state *state = (grchamp_state *)space->machine->driver_data;
	return state->comm_latch2[offset];
}



/*************************************
 *
 *  Sound port handlers
 *
 *************************************/

static WRITE8_DEVICE_HANDLER( grchamp_portA_0_w )
{
	discrete_sound_w(device, GRCHAMP_A_DATA, data);
}

static WRITE8_DEVICE_HANDLER( grchamp_portB_0_w )
{
	discrete_sound_w(device, GRCHAMP_B_DATA, 255-data);
}

static WRITE8_DEVICE_HANDLER( grchamp_portA_2_w )
{
	/* A0/A1 modify the output of AY8910 #2 */
	/* A7 contributes to the discrete logic hanging off of AY8910 #0 */
}
static WRITE8_DEVICE_HANDLER( grchamp_portB_2_w )
{
	/* B0 connects elsewhere */
}



/*************************************
 *
 *  Sound interfaces
 *
 *************************************/

static const ay8910_interface ay8910_interface_1 =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DEVICE_HANDLER("discrete", grchamp_portA_0_w),
	DEVCB_DEVICE_HANDLER("discrete", grchamp_portB_0_w)
};

static const ay8910_interface ay8910_interface_3 =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_HANDLER(grchamp_portA_2_w),
	DEVCB_HANDLER(grchamp_portB_2_w)
};



/*************************************
 *
 *  Graphics Layouts
 *
 *************************************/

static const gfx_layout sprite_layout =
{
	16,16,
	RGN_FRAC(1,2),
	2,
	{ 0,RGN_FRAC(1,2) },
	{ STEP8(0,1),STEP8(64,1) },
	{ STEP8(0,8),STEP8(128,8) },
	16*16
};

static const gfx_layout tile_layout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ 0,4,RGN_FRAC(1,2),RGN_FRAC(1,2)+4 },
	{ STEP4(0,1),STEP4(8*8+0,1) },
	{ STEP8(0,8) },
	16*8
};

static GFXDECODE_START( grchamp )
	GFXDECODE_ENTRY( "gfx1", 0x0000, gfx_8x8x2_planar,	0, 8 )
	GFXDECODE_ENTRY( "gfx2", 0x0000, tile_layout,		0, 16 )
	GFXDECODE_ENTRY( "gfx3", 0x0000, tile_layout,		0, 16 )
	GFXDECODE_ENTRY( "gfx4", 0x0000, tile_layout,		0, 16 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, sprite_layout,		0, 32 )
GFXDECODE_END



/*************************************
 *
 *  Address maps
 *
 *************************************/

/* complete memory map derived from schematics */
static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x43ff) AM_MIRROR(0x0400) AM_RAM
	AM_RANGE(0x4800, 0x4bff) AM_MIRROR(0x0400) AM_RAM AM_BASE_MEMBER(grchamp_state, radarram)
	AM_RANGE(0x5000, 0x53ff) AM_MIRROR(0x0400) AM_RAM AM_BASE_MEMBER(grchamp_state, videoram)
	AM_RANGE(0x5800, 0x58ff) AM_MIRROR(0x0700) AM_RAM AM_BASE_MEMBER(grchamp_state, spriteram)
ADDRESS_MAP_END


static ADDRESS_MAP_START( main_portmap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_MIRROR(0x78) AM_READ_PORT("ACCEL")
	AM_RANGE(0x02, 0x02) AM_MIRROR(0x78) AM_READ(sub_to_main_comm_r)
	AM_RANGE(0x03, 0x03) AM_MIRROR(0x78) AM_READ_PORT("WHEEL")
	AM_RANGE(0x04, 0x04) AM_MIRROR(0x78) AM_READ_PORT("DSWA")
	AM_RANGE(0x05, 0x05) AM_MIRROR(0x78) AM_READ_PORT("DSWB")
	AM_RANGE(0x06, 0x06) AM_MIRROR(0x78) AM_READ_PORT("TILT")
	AM_RANGE(0x01, 0x01) AM_MIRROR(0x60) AM_READ(pc3259_0_r)
	AM_RANGE(0x09, 0x09) AM_MIRROR(0x60) AM_READ(pc3259_1_r)
	AM_RANGE(0x11, 0x11) AM_MIRROR(0x60) AM_READ(pc3259_2_r)
	AM_RANGE(0x19, 0x19) AM_MIRROR(0x60) AM_READ(pc3259_3_r)
	AM_RANGE(0x00, 0x0f) AM_MIRROR(0x40) AM_WRITE(cpu0_outputs_w)
	AM_RANGE(0x10, 0x13) AM_MIRROR(0x40) AM_WRITE(main_to_sub_comm_w)
	AM_RANGE(0x20, 0x2f) AM_MIRROR(0x53) AM_WRITE(led_board_w)
ADDRESS_MAP_END


/* complete memory map derived from schematics */
static ADDRESS_MAP_START( sub_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x27ff) AM_RAM_WRITE(grchamp_left_w) AM_BASE_MEMBER(grchamp_state, leftram)
	AM_RANGE(0x2800, 0x2fff) AM_RAM_WRITE(grchamp_right_w) AM_BASE_MEMBER(grchamp_state, rightram)
	AM_RANGE(0x3000, 0x37ff) AM_RAM_WRITE(grchamp_center_w) AM_BASE_MEMBER(grchamp_state, centerram)
	AM_RANGE(0x4000, 0x43ff) AM_MIRROR(0x0400) AM_RAM
	AM_RANGE(0x5000, 0x6fff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( sub_portmap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_READ(main_to_sub_comm_r)
	AM_RANGE(0x00, 0x0f) AM_MIRROR(0x70) AM_WRITE(cpu1_outputs_w)
ADDRESS_MAP_END


/* complete memory map derived from schematics */
static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x4000, 0x43ff) AM_RAM
	AM_RANGE(0x4800, 0x4801) AM_MIRROR(0x07f8) AM_DEVWRITE("ay1", ay8910_address_data_w)
	AM_RANGE(0x4801, 0x4801) AM_MIRROR(0x07f8) AM_DEVREAD("ay1", ay8910_r)
	AM_RANGE(0x4802, 0x4803) AM_MIRROR(0x07f8) AM_DEVWRITE("ay2", ay8910_address_data_w)
	AM_RANGE(0x4803, 0x4803) AM_MIRROR(0x07f8) AM_DEVREAD("ay2", ay8910_r)
	AM_RANGE(0x4804, 0x4805) AM_MIRROR(0x07fa) AM_DEVWRITE("ay3", ay8910_address_data_w)
	AM_RANGE(0x4805, 0x4805) AM_MIRROR(0x07fa) AM_DEVREAD("ay3", ay8910_r)
	AM_RANGE(0x5000, 0x5000) AM_READ(soundlatch_r)
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( grchamp )

	PORT_START("TILT")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE3 )	/* High Score reset switch */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_TOGGLE	/* High Gear */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_TILT )		/* Tilt */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )	/* Service */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )		/* Coin A */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )		/* Coin B */

	PORT_START("ACCEL")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(16) PORT_CODE_INC(KEYCODE_LCONTROL)
	//mask,default,type,sensitivity,delta,min,max

	PORT_START("WHEEL")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(5) PORT_REVERSE
	//mask,default,type,sensitivity,delta,min,max

	PORT_START("DSWA")
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Coin_A ) )		PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x0f, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_8C ) )
	PORT_DIPNAME( 0xf0, 0x00, DEF_STR( Coin_B ) )		PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(    0xf0, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_8C ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x03, 0x02, "Extra Race" )		PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x00, "4th" )
	PORT_DIPSETTING(    0x02, "5th" )
	PORT_DIPSETTING(    0x01, "6th" )
	PORT_DIPSETTING(    0x03, "7th" )
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_HIGH, "SW2:3" )	/* Listed as "Unused and should remain in the OFF position" */
	PORT_DIPNAME( 0x08, 0x00, "RAM Test" )			PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Coin System" )		PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, "1 Way" )
	PORT_DIPSETTING(    0x00, "2 Way" )
	PORT_DIPNAME( 0x20, 0x00, "Display '1981'" )		PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "Display Score" )		PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, "LEDs" )
	PORT_DIPSETTING(    0x40, "On Screen" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )		PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static MACHINE_DRIVER_START( grchamp )
	MDRV_DRIVER_DATA(grchamp_state)

	/* basic machine hardware */
	/* CPU BOARD */
	MDRV_CPU_ADD("maincpu", Z80, PIXEL_CLOCK/2)
	MDRV_CPU_PROGRAM_MAP(main_map)
	MDRV_CPU_IO_MAP(main_portmap)
	MDRV_CPU_VBLANK_INT("screen", grchamp_cpu0_interrupt)

	/* GAME BOARD */
	MDRV_CPU_ADD("sub", Z80, PIXEL_CLOCK/2)
	MDRV_CPU_PROGRAM_MAP(sub_map)
	MDRV_CPU_IO_MAP(sub_portmap)
	MDRV_CPU_VBLANK_INT("screen", grchamp_cpu1_interrupt)

	/* SOUND BOARD */
	MDRV_CPU_ADD("audiocpu", Z80, SOUND_CLOCK/2)
	MDRV_CPU_PROGRAM_MAP(sound_map)
	MDRV_CPU_PERIODIC_INT(irq0_line_hold, (double)SOUND_CLOCK/4/16/16/10/16)

	MDRV_MACHINE_RESET(grchamp)
	MDRV_WATCHDOG_VBLANK_INIT(8)
	MDRV_QUANTUM_TIME(HZ(6000))

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_ALWAYS_UPDATE)
	MDRV_GFXDECODE(grchamp)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_RAW_PARAMS(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART)

	MDRV_PALETTE_INIT(grchamp)
	MDRV_VIDEO_START(grchamp)
	MDRV_VIDEO_UPDATE(grchamp)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ay1", AY8910, SOUND_CLOCK/4)	/* 3B */
	MDRV_SOUND_CONFIG(ay8910_interface_1)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.2)

	MDRV_SOUND_ADD("ay2", AY8910, SOUND_CLOCK/4)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.2)

	MDRV_SOUND_ADD("ay3", AY8910, SOUND_CLOCK/4)	/* 1B */
	MDRV_SOUND_CONFIG(ay8910_interface_3)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.2)

	MDRV_SOUND_ADD("discrete", DISCRETE, 0)
	MDRV_SOUND_CONFIG_DISCRETE(grchamp)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( grchamp )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "gm03",   0x0000, 0x1000, CRC(47fda76e) SHA1(fd5f1a651481669d64e5e0799369c22472265535) )
	ROM_LOAD( "gm04",   0x1000, 0x1000, CRC(07a623dc) SHA1(bb8a6531d95e996148c06fd336db4054eb1d28dd) )
	ROM_LOAD( "gm05",	0x2000, 0x1000, CRC(716e1fba) SHA1(fe596873c932513227b982cd23af440d31612de9) )
	ROM_LOAD( "gm06",	0x3000, 0x1000, CRC(157db30b) SHA1(a74314d3aef4659ea96ed659e5db2883e7ae1cb1) )

	ROM_REGION( 0x8000, "sub", 0 )
	ROM_LOAD( "gm09",	0x0000, 0x1000, CRC(d57bd109) SHA1(d1cb5ba783eaceda45893f6404fe9dbac740a2de) )
	ROM_LOAD( "gm10",	0x1000, 0x1000, CRC(41ba07f1) SHA1(103eeacdd36b4347fc62debb6b5f4163083313f4) )
	ROM_LOAD( "gr16",	0x5000, 0x1000, CRC(885d708e) SHA1(d5d2978a0eeca167ec1fb9f6f981388de46fbf81) )
	ROM_LOAD( "gr15",	0x6000, 0x1000, CRC(a822430b) SHA1(4d29612489362d2dc3f3a9eab609902a50c34aff) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "gm07",	0x0000, 0x1000, CRC(65dcc572) SHA1(c9b19af365fa7ade2698be0bb892591ba281ecb0) )
	ROM_LOAD( "gm08",	0x1000, 0x1000, CRC(224d880c) SHA1(68aaaa0213d09cf34ba50c91d8c031d041f8a76f) )

	ROM_REGION( 0x2000, "gfx1", 0 ) /* characters/sprites */
	ROM_LOAD( "gm01",	0x0000, 0x1000, CRC(846f8e89) SHA1(346bfd69268606fde27643b4d135b481536b73b1) )
	ROM_LOAD( "gm02",	0x1000, 0x1000, CRC(5911948d) SHA1(6f3a9a7f8d6a04b8e6d83756764c9c4185983d9b) )

	ROM_REGION( 0x2000, "gfx2", 0 ) /* left tiles */
	ROM_LOAD( "gr20",	0x0000, 0x1000, CRC(88ba2c03) SHA1(4dfd136f122663223043c6cd79566f8eeec72681) )
	ROM_LOAD( "gr19",	0x1000, 0x1000, CRC(ff34b444) SHA1(51c67a1691da3a2d8ddcff5fd8fa816b1f9c60c0) )

	ROM_REGION( 0x2000, "gfx3", 0 ) /* right tiles */
	ROM_LOAD( "gr21",	0x0000, 0x1000, CRC(2f77a9f3) SHA1(9e20a776c5e8c7577c3e8467d4f8ac7ac909901f) )
	ROM_LOAD( "gr22",	0x1000, 0x1000, CRC(31bb5fc7) SHA1(9f638e632e7c72461bedecb710ac9b30f015eebf) )

	ROM_REGION( 0x2000, "gfx4", 0 ) /* center tiles */
	ROM_LOAD( "gr13",	0x0000, 0x1000, CRC(d5e19ebd) SHA1(d0ca553eec87619ec489f7ba6238f1fdde7c480b) )
	ROM_LOAD( "gr14",	0x1000, 0x1000, CRC(d129b8e4) SHA1(db25bfde2a48e14d38a43133d88d479c3cc1397a) )

	ROM_REGION( 0x0800, "gfx5", 0 ) /* rain */
	ROM_LOAD( "gr10",	0x0000, 0x0800, CRC(b1f0a873) SHA1(f7ef1a16556ae3e7d70209bcb38ea3ae94208789) )

	ROM_REGION( 0x0800, "gfx6", 0 ) /* headlights */
	ROM_LOAD( "gr12",	0x0000, 0x0800, CRC(f3bc599e) SHA1(3ec19584896a0bf10b9c5750f3c78ad3e722cc49) )

	ROM_REGION( 0x1000, "gfx7", 0 ) /* player */
	ROM_LOAD( "gr11",	0x0000, 0x1000, CRC(54eb3ec9) SHA1(22739240f53c708d8e53094d96916778e12beeed) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "gr23.bpr", 0x00, 0x20, CRC(41c6c48d) SHA1(8bd14b5f02f9da0a68e3125955be18462b57401d) ) /* background colors */
	ROM_LOAD( "gr09.bpr", 0x20, 0x20, CRC(260fb2b9) SHA1(db0bf49f12a944613d113317d7dfea25bd7469fc) ) /* sprite/text colors */
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAMEL( 1981, grchamp, 0, grchamp, grchamp, 0, ROT270, "Taito", "Grand Champion", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS, layout_grchamp )
