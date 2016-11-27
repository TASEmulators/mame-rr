/**************************************************************************
Change Lanes
(C) Taito 1983

Jarek Burczynski
Phil Stroffolino
Tomasz Slanina

***************************************************************************/

#include "emu.h"
#include "deprecat.h"
#include "cpu/z80/z80.h"
#include "cpu/m6805/m6805.h"
#include "sound/ay8910.h"
#include "includes/changela.h"

#include "changela.lh"


static READ8_HANDLER( mcu_r )
{
	changela_state *state = (changela_state *)space->machine->driver_data;

	//mame_printf_debug("Z80 MCU  R = %x\n", state->mcu_out);
	return state->mcu_out;
}

/* latch LS374 at U39 */
static WRITE8_HANDLER( mcu_w )
{
	changela_state *state = (changela_state *)space->machine->driver_data;
	state->mcu_in = data;
}


/*********************************
        MCU
*********************************/

static READ8_HANDLER( changela_68705_port_a_r )
{
	changela_state *state = (changela_state *)space->machine->driver_data;
	return (state->port_a_out & state->ddr_a) | (state->port_a_in & ~state->ddr_a);
}

static WRITE8_HANDLER( changela_68705_port_a_w )
{
	changela_state *state = (changela_state *)space->machine->driver_data;
	state->port_a_out = data;
}

static WRITE8_HANDLER( changela_68705_ddr_a_w )
{
	changela_state *state = (changela_state *)space->machine->driver_data;
	state->ddr_a = data;
}

static READ8_HANDLER( changela_68705_port_b_r )
{
	changela_state *state = (changela_state *)space->machine->driver_data;
	return (state->port_b_out & state->ddr_b) | (input_port_read(space->machine, "MCU") & ~state->ddr_b);
}

static WRITE8_HANDLER( changela_68705_port_b_w )
{
	changela_state *state = (changela_state *)space->machine->driver_data;
	state->port_b_out = data;
}

static WRITE8_HANDLER( changela_68705_ddr_b_w )
{
	changela_state *state = (changela_state *)space->machine->driver_data;
	state->ddr_b = data;
}

static READ8_HANDLER( changela_68705_port_c_r )
{
	changela_state *state = (changela_state *)space->machine->driver_data;
	return (state->port_c_out & state->ddr_c) | (state->port_c_in & ~state->ddr_c);
}

static WRITE8_HANDLER( changela_68705_port_c_w )
{
	changela_state *state = (changela_state *)space->machine->driver_data;
	/* PC3 is connected to the CLOCK input of the LS374,
        so we latch the data on positive going edge of the clock */

/* this is strange because if we do this corectly - it just doesn't work */
	if ((data & 8) /*& (!(state->port_c_out & 8))*/ )
		state->mcu_out = state->port_a_out;

	/* PC2 is connected to the /OE input of the LS374 */
	if (!(data & 4))
		state->port_a_in = state->mcu_in;

	state->port_c_out = data;
}

static WRITE8_HANDLER( changela_68705_ddr_c_w )
{
	changela_state *state = (changela_state *)space->machine->driver_data;
	state->ddr_c = data;
}


static ADDRESS_MAP_START( mcu_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0x7ff)
	AM_RANGE(0x0000, 0x0000) AM_READWRITE(changela_68705_port_a_r, changela_68705_port_a_w)
	AM_RANGE(0x0001, 0x0001) AM_READWRITE(changela_68705_port_b_r, changela_68705_port_b_w)
	AM_RANGE(0x0002, 0x0002) AM_READWRITE(changela_68705_port_c_r, changela_68705_port_c_w)

	AM_RANGE(0x0004, 0x0004) AM_WRITE(changela_68705_ddr_a_w)
	AM_RANGE(0x0005, 0x0005) AM_WRITE(changela_68705_ddr_b_w)
	AM_RANGE(0x0006, 0x0006) AM_WRITE(changela_68705_ddr_c_w)

	AM_RANGE(0x0000, 0x007f) AM_RAM
	AM_RANGE(0x0080, 0x07ff) AM_ROM
ADDRESS_MAP_END



/* U30 */
static READ8_HANDLER( changela_24_r )
{
	changela_state *state = (changela_state *)space->machine->driver_data;
	return ((state->port_c_out & 2) << 2) | 7;	/* bits 2,1,0-N/C inputs */
}

static READ8_HANDLER( changela_25_r )
{
	changela_state *state = (changela_state *)space->machine->driver_data;
	//collisions on bits 3,2, bits 1,0-N/C inputs
	return (state->tree1_col << 3) | (state->tree0_col << 2) | 0x03;
}

static READ8_HANDLER( changela_30_r )
{
	return input_port_read(space->machine, "WHEEL") & 0x0f;	//wheel control (clocked input) signal on bits 3,2,1,0
}

static READ8_HANDLER( changela_31_r )
{
	changela_state *state = (changela_state *)space->machine->driver_data;
	/* If the new value is less than the old value, and it did not wrap around,
       or if the new value is greater than the old value, and it did wrap around,
       then we are moving LEFT. */
	UINT8 curr_value = input_port_read(space->machine, "WHEEL");

	if ((curr_value < state->prev_value_31 && (state->prev_value_31 - curr_value) < 0x80)
	||  (curr_value > state->prev_value_31 && (curr_value - state->prev_value_31) > 0x80))
		state->dir_31 = 1;
	if ((state->prev_value_31 < curr_value && (curr_value - state->prev_value_31) < 0x80)
	||  (state->prev_value_31 > curr_value && (state->prev_value_31 - curr_value) > 0x80))
		state->dir_31 = 0;

	state->prev_value_31 = curr_value;

	//wheel UP/DOWN control signal on bit 3, collisions on bits:2,1,0
	return (state->dir_31 << 3) | (state->left_bank_col << 2) | (state->right_bank_col << 1) | state->boat_shore_col;
}

static READ8_HANDLER( changela_2c_r )
{
	int val = input_port_read(space->machine, "IN0");

	val = (val & 0x30) | ((val & 1) << 7) | (((val & 1) ^ 1) << 6);

	return val;
}

static READ8_HANDLER( changela_2d_r )
{
	/* the schems are unreadable - i'm not sure it is V8 (page 74, SOUND I/O BOARD SCHEMATIC 1 OF 2, FIGURE 24 - in the middle on the right side) */
	int v8 = 0;
	int gas;

	if ((space->machine->primary_screen->vpos() & 0xf8) == 0xf8)
		v8 = 1;

	/* Gas pedal is made up of 2 switches, 1 active low, 1 active high */
	switch (input_port_read(space->machine, "IN1") & 0x03)
	{
		case 0x02:
			gas = 0x80;
			break;
		case 0x01:
			gas = 0x00;
			break;
		default:
			gas = 0x40;
			break;
	}

	return (input_port_read(space->machine, "IN1") & 0x20) | gas | (v8 << 4);
}

static WRITE8_HANDLER( mcu_pc_0_w )
{
	changela_state *state = (changela_state *)space->machine->driver_data;
	state->port_c_in = (state->port_c_in & 0xfe) | (data & 1);
}

static WRITE8_HANDLER( changela_collision_reset_0 )
{
	changela_state *state = (changela_state *)space->machine->driver_data;
	state->collision_reset = data & 0x01;
}

static WRITE8_HANDLER( changela_collision_reset_1 )
{
	changela_state *state = (changela_state *)space->machine->driver_data;
	state->tree_collision_reset = data & 0x01;
}

static WRITE8_HANDLER( changela_coin_counter_w )
{
	coin_counter_w(space->machine, offset, data);
}


static ADDRESS_MAP_START( changela_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x83ff) AM_RAM AM_BASE_MEMBER(changela_state, spriteram) /* OBJ0 RAM */
	AM_RANGE(0x9000, 0x97ff) AM_RAM AM_BASE_MEMBER(changela_state, videoram)	/* OBJ1 RAM */
	AM_RANGE(0xa000, 0xa07f) AM_WRITE(changela_colors_w) AM_BASE_MEMBER(changela_state, colorram)	/* Color 93419 RAM 64x9(nine!!!) bits A0-used as the 8-th bit data input (d0-d7->normal, a0->d8) */
	AM_RANGE(0xb000, 0xbfff) AM_ROM

	AM_RANGE(0xc000, 0xc7ff) AM_READWRITE(changela_mem_device_r, changela_mem_device_w)	/* RAM4 (River Bed RAM); RAM5 (Tree RAM) */

	/* LS138 - U16 */
	AM_RANGE(0xc800, 0xc800) AM_WRITENOP				/* not connected */
	AM_RANGE(0xc900, 0xc900) AM_WRITE(changela_mem_device_select_w)	/* selects the memory device to be accessible at 0xc000-0xc7ff */
	AM_RANGE(0xca00, 0xca00) AM_WRITE(changela_slope_rom_addr_hi_w)
	AM_RANGE(0xcb00, 0xcb00) AM_WRITE(changela_slope_rom_addr_lo_w)

	AM_RANGE(0xd000, 0xd001) AM_DEVREADWRITE("ay1", ay8910_r, ay8910_address_data_w)
	AM_RANGE(0xd010, 0xd011) AM_DEVREADWRITE("ay2", ay8910_r, ay8910_address_data_w)

	/* LS259 - U44 */
	AM_RANGE(0xd020, 0xd020) AM_WRITE(changela_collision_reset_0)
	AM_RANGE(0xd021, 0xd022) AM_WRITE(changela_coin_counter_w)
//AM_RANGE(0xd023, 0xd023) AM_WRITENOP

	/* LS139 - U24 */
	AM_RANGE(0xd024, 0xd024) AM_READWRITE(changela_24_r, mcu_pc_0_w)
	AM_RANGE(0xd025, 0xd025) AM_READWRITE(changela_25_r, changela_collision_reset_1)
	AM_RANGE(0xd026, 0xd026) AM_WRITENOP
	AM_RANGE(0xd028, 0xd028) AM_READ(mcu_r)
	AM_RANGE(0xd02c, 0xd02c) AM_READ(changela_2c_r)
	AM_RANGE(0xd02d, 0xd02d) AM_READ(changela_2d_r)

	AM_RANGE(0xd030, 0xd030) AM_READWRITE(changela_30_r, mcu_w)
	AM_RANGE(0xd031, 0xd031) AM_READ(changela_31_r)

	AM_RANGE(0xe000, 0xe000) AM_WRITE(watchdog_reset_w)	/* Watchdog */

	AM_RANGE(0xf000, 0xf7ff) AM_RAM	/* RAM2 (Processor RAM) */
ADDRESS_MAP_END


static INPUT_PORTS_START( changela )
	PORT_START("DSWA")	/* DSWA */
	PORT_DIPNAME( 0x07, 0x01, "Steering Wheel Ratio" )		PORT_DIPLOCATION("SWA:1,2,3")
	//PORT_DIPSETTING(    0x00, "?" ) /* Not documented */
	PORT_DIPSETTING(    0x01, "Recommended Setting" )
	//PORT_DIPSETTING(    0x02, "?" ) /* Not documented */
	//PORT_DIPSETTING(    0x03, "?" ) /* Not documented */
	//PORT_DIPSETTING(    0x04, "?" ) /* Not documented */
	//PORT_DIPSETTING(    0x05, "?" ) /* Not documented */
	//PORT_DIPSETTING(    0x06, "?" ) /* Not documented */
	//PORT_DIPSETTING(    0x07, "?" ) /* Not documented */
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) )		PORT_DIPLOCATION("SWA:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Ignore Memory Failures" )	PORT_DIPLOCATION("SWA:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Controls ) )			PORT_DIPLOCATION("SWA:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Joystick ) )
	PORT_DIPSETTING(    0x00, "Steering Wheel" )
	PORT_DIPNAME( 0x40, 0x40, "Diagnostic" )				PORT_DIPLOCATION("SWA:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Players ) )			PORT_DIPLOCATION("SWA:8")
	PORT_DIPSETTING(    0x80, "1" )
	PORT_DIPSETTING(    0x00, "2" )

	PORT_START("DSWB")	/* DSWB */
	PORT_DIPNAME( 0x03, 0x00, "Max Bonus Fuels" )			PORT_DIPLOCATION("SWB:1,2")
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x00, "99" )
	PORT_DIPNAME( 0x0c, 0x08, "Game Difficulty" )			PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Easy) )
	PORT_DIPSETTING(    0x04, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x30, 0x20, "Traffic Difficulty" )		PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Easy) )
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x40, 0x00, "Land Collisions Enabled" )	PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Car Collision Enabled" )		PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWC")	/* DSWC - coinage */
	PORT_DIPNAME( 0xf0, 0x10, DEF_STR( Coin_A ) )			PORT_DIPLOCATION("SWC:5,6,7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(	0x90, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(	0xa0, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(	0x10, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(	0xb0, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(	0xc0, DEF_STR( 2C_4C ) )
	PORT_DIPSETTING(	0x20, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(	0xd0, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(	0xe0, DEF_STR( 2C_6C ) )
	PORT_DIPSETTING(	0x30, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(	0xf0, DEF_STR( 2C_7C ) )
	PORT_DIPSETTING(	0x40, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(	0x50, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(	0x60, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(	0x70, DEF_STR( 1C_7C ) )
	PORT_DIPNAME( 0x0f, 0x01, DEF_STR( Coin_B ) )			PORT_DIPLOCATION("SWC:1,2,3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(	0x09, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(	0x0a, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(	0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(	0x0b, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(	0x0c, DEF_STR( 2C_4C ) )
	PORT_DIPSETTING(	0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(	0x0d, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(	0x0e, DEF_STR( 2C_6C ) )
	PORT_DIPSETTING(	0x03, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(	0x0f, DEF_STR( 2C_7C ) )
	PORT_DIPSETTING(	0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(	0x05, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(	0x06, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(	0x07, DEF_STR( 1C_7C ) )

	PORT_START("DSWD")	/* DSWD - bonus */
	PORT_DIPNAME( 0x01, 0x01, "Right Slot" )				PORT_DIPLOCATION("SWD:1")
	PORT_DIPSETTING(    0x01, "On Right (Bottom) Counter" )
	PORT_DIPSETTING(    0x00, "On Left (Top) Counter" )
	PORT_DIPNAME( 0x02, 0x02, "Left Slot" )					PORT_DIPLOCATION("SWD:2")
	PORT_DIPSETTING(    0x02, "On Right (Bottom) Counter" )
	PORT_DIPSETTING(    0x00, "On Left (Top) Counter" )
	PORT_DIPNAME( 0x1c, 0x00, "Credits For Bonus" )			PORT_DIPLOCATION("SWD:3,4,5")
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x04, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x14, "5" )
	PORT_DIPSETTING(    0x18, "6" )
	PORT_DIPSETTING(    0x1c, "7" )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SWD:6" )	/* Listed as "Unused" */
	PORT_DIPNAME( 0x40, 0x00, "'King Of The World' Name Length" )PORT_DIPLOCATION("SWD:7")
	PORT_DIPSETTING(    0x40, "3 Letters" )
	PORT_DIPSETTING(    0x00, "Long" )
	PORT_DIPNAME( 0x80, 0x00, "'King Of The World' Name" )	PORT_DIPLOCATION("SWD:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("MCU") /* MCU */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_DIPNAME( 0x30, 0x30, "Self Test Switch" )			PORT_DIPLOCATION("SWT:1,2")
	//PORT_DIPSETTING(    0x00, "?" )                       /* Not possible, 3-state switch */
	PORT_DIPSETTING(    0x20, "Free Game" )					/* "Puts a credit on the game without increasing the coin counter." */
	PORT_DIPSETTING(    0x10, DEF_STR( Test ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Off ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN0") /* 0xDx2C */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Gear Shift") PORT_CODE(KEYCODE_SPACE) PORT_TOGGLE /* Gear shift */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* FWD - negated bit 7 */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* REV - gear position */

	PORT_START("IN1") /* 0xDx2D */
	PORT_BIT( 0x03, 0x00, IPT_PEDAL ) PORT_MINMAX(0x00, 0x02) PORT_SENSITIVITY(10) PORT_KEYDELTA(1) //gas pedal
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_TILT )
	//PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) //gas1
	//PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON2 ) //gas2

	PORT_START("WHEEL") /* 0xDx30 DRIVING_WHEEL */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(50) PORT_KEYDELTA(8)
INPUT_PORTS_END


static const ay8910_interface ay8910_interface_1 =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_INPUT_PORT("DSWA"),
	DEVCB_INPUT_PORT("DSWB"),
	DEVCB_NULL,
	DEVCB_NULL
};

static const ay8910_interface ay8910_interface_2 =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_INPUT_PORT("DSWC"),
	DEVCB_INPUT_PORT("DSWD"),
	DEVCB_NULL,
	DEVCB_NULL
};


static INTERRUPT_GEN( chl_interrupt )
{
	changela_state *state = (changela_state *)device->machine->driver_data;
	int vector = device->machine->primary_screen->vblank() ? 0xdf : 0xcf; /* 4 irqs per frame: 3 times 0xcf, 1 time 0xdf */

//    device->machine->primary_screen->update_partial(device->machine->primary_screen->vpos());

	cpu_set_input_line_and_vector(device, 0, HOLD_LINE, vector);

	/* it seems the V8 == Vblank and it is connected to the INT on the 68705 */
	//so we should cause an INT on the MCU cpu here, as well.
	//but only once per frame !
	if (vector == 0xdf) /* only on vblank */
		generic_pulse_irq_line(state->mcu, 0);

}

static MACHINE_START(changela)
{
	changela_state *state = (changela_state *)machine->driver_data;

	state->mcu = machine->device("mcu");

	/* video */
	state_save_register_global(machine, state->slopeROM_bank);
	state_save_register_global(machine, state->tree_en);
	state_save_register_global(machine, state->horizon);
	state_save_register_global(machine, state->mem_dev_selected);
	state_save_register_global(machine, state->v_count_river);
	state_save_register_global(machine, state->v_count_tree);
	state_save_register_global_array(machine, state->tree_on);

	/* mcu */
	state_save_register_global(machine, state->port_a_in);
	state_save_register_global(machine, state->port_a_out);
	state_save_register_global(machine, state->ddr_a);
	state_save_register_global(machine, state->port_b_out);
	state_save_register_global(machine, state->ddr_b);
	state_save_register_global(machine, state->port_c_in);
	state_save_register_global(machine, state->port_c_out);
	state_save_register_global(machine, state->ddr_c);

	state_save_register_global(machine, state->mcu_out);
	state_save_register_global(machine, state->mcu_in);
	state_save_register_global(machine, state->mcu_pc_1);
	state_save_register_global(machine, state->mcu_pc_0);

	/* misc */
	state_save_register_global(machine, state->tree0_col);
	state_save_register_global(machine, state->tree1_col);
	state_save_register_global(machine, state->left_bank_col);
	state_save_register_global(machine, state->right_bank_col);
	state_save_register_global(machine, state->boat_shore_col);
	state_save_register_global(machine, state->collision_reset);
	state_save_register_global(machine, state->tree_collision_reset);
	state_save_register_global(machine, state->prev_value_31);
	state_save_register_global(machine, state->dir_31);
}

static MACHINE_RESET (changela)
{
	changela_state *state = (changela_state *)machine->driver_data;

	/* video */
	state->slopeROM_bank = 0;
	state->tree_en = 0;
	state->horizon = 0;
	state->mem_dev_selected = 0;
	state->v_count_river = 0;
	state->v_count_tree = 0;
	state->tree_on[0] = 0;
	state->tree_on[1] = 0;

	/* mcu */
	state->mcu_pc_1 = 0;
	state->mcu_pc_0 = 0;

	state->port_a_in = 0;
	state->port_a_out = 0;
	state->ddr_a = 0;
	state->port_b_out = 0;
	state->ddr_b = 0;
	state->port_c_in = 0;
	state->port_c_out = 0;
	state->ddr_c = 0;
	state->mcu_out = 0;
	state->mcu_in = 0;

	/* misc */
	state->tree0_col = 0;
	state->tree1_col = 0;
	state->left_bank_col = 0;
	state->right_bank_col = 0;
	state->boat_shore_col = 0;
	state->collision_reset = 0;
	state->tree_collision_reset = 0;
	state->prev_value_31 = 0;
	state->dir_31 = 0;
}

static MACHINE_DRIVER_START( changela )

	/* driver data */
	MDRV_DRIVER_DATA(changela_state)

	MDRV_CPU_ADD("maincpu", Z80,5000000)
	MDRV_CPU_PROGRAM_MAP(changela_map)
	MDRV_CPU_VBLANK_INT_HACK(chl_interrupt,4)

	MDRV_CPU_ADD("mcu", M68705,2500000)
	MDRV_CPU_PROGRAM_MAP(mcu_map)

	MDRV_MACHINE_START(changela)
	MDRV_MACHINE_RESET(changela)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 262)  /* vert size is a guess */
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 4*8, 32*8-1)

	MDRV_PALETTE_LENGTH(0x40)

	MDRV_VIDEO_START(changela)
	MDRV_VIDEO_UPDATE(changela)

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ay1", AY8910, 1250000)
	MDRV_SOUND_CONFIG(ay8910_interface_1)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD("ay2", AY8910, 1250000)
	MDRV_SOUND_CONFIG(ay8910_interface_2)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END


ROM_START( changela )
	ROM_REGION( 0x10000, "maincpu", 0 )	/* Z80 code */
	ROM_LOAD( "cl25a",	0x0000, 0x2000, CRC(38530a60) SHA1(0b0ef1abe11c5271fcd1671322b77165217553c3) )
	ROM_LOAD( "cl24a",	0x2000, 0x2000, CRC(2fcf4a82) SHA1(c33355e2d4d3fab32c8d713a680ec0fceedab341) )
	ROM_LOAD( "cl23",	0x4000, 0x2000, CRC(08385891) SHA1(d8d66664ec25db067d5a4a6c35ec0ac65b9e0c6a) )
	ROM_LOAD( "cl22",	0x6000, 0x2000, CRC(796e0abd) SHA1(64dd9fc1f9bc44519a253ef0c02e181dd13904bf) )
	ROM_LOAD( "cl27",	0xb000, 0x1000, CRC(3668afb8) SHA1(bcfb788baf806edcb129ea9f9dcb1d4260684773) )

	ROM_REGION( 0x10000, "mcu", 0 )	/* 68705U3 */
	ROM_LOAD( "cl38a",	0x0000, 0x800, CRC(b70156ce) SHA1(c5eab8bbd65c4f587426298da4e22f991ce01dde) )

	ROM_REGION( 0x4000, "gfx1", 0 )	/* tile data */
	ROM_LOAD( "cl111",	0x0000, 0x2000, CRC(41c0149d) SHA1(3ea53a3821b044b3d0451fec1b4ee2c28da393ca) )
	ROM_LOAD( "cl113",	0x2000, 0x2000, CRC(ddf99926) SHA1(e816b88302c5639c7284f4845d450f232d63a10c) )

	ROM_REGION( 0x1000, "gfx2", 0 )	/* obj 1 data */
	ROM_LOAD( "cl46",	0x0000, 0x1000, CRC(9c0a7d28) SHA1(fac9180ea0d9aeea56a84b35cc0958f0dd86a801) )

	ROM_REGION( 0x8000, "user1", 0 )	/* obj 0 data */
	ROM_LOAD( "cl100",	0x0000, 0x2000, CRC(3fa9e4fa) SHA1(9abd7df5fcf143a0c476bd8c8753c5ea294b9f74) )
	ROM_LOAD( "cl99",	0x2000, 0x2000, CRC(67b27b9e) SHA1(7df0f93851959359218c8d2272e30d242a77039d) )
	ROM_LOAD( "cl98",	0x4000, 0x2000, CRC(bffe4149) SHA1(5cf0b98f9d342bd06d575c565ea01bbd79f5e04b) )
	ROM_LOAD( "cl97",	0x6000, 0x2000, CRC(5abab8f9) SHA1(f5156855bbcdf0740fd44520386318ee53ebbf9a) )

	ROM_REGION( 0x1000, "user2", 0 )	/* math tables: SLOPE ROM (river-tree schematic page 1/3) */
	ROM_LOAD( "cl44",	0x0000, 0x1000, CRC(160d2bc7) SHA1(2609208c2bd4618ea340923ee01af69278980c36) ) /* first and 2nd half identical */

	ROM_REGION( 0x3000, "user3", 0 )	/* math tables: TREE ROM (river-tree schematic page 3/3)*/
	ROM_LOAD( "cl7",	0x0000, 0x0800, CRC(01e3efca) SHA1(b26203787f105ba32773e37035c39253050f9c82) ) /* fixed bits: 0xxxxxxx */
	ROM_LOAD( "cl9",	0x1000, 0x2000, CRC(4e53cdd0) SHA1(6255411cfdccbe2c581c83f9127d582623453c3a) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "cl88",	0x0000, 0x0020, CRC(da4d6625) SHA1(2d9a268973518252eb36f479ab650af8c16c885c) ) /* math train state machine */
ROM_END

GAMEL( 1983, changela, 0, changela, changela, 0,   ROT180, "Taito Corporation", "Change Lanes", GAME_SUPPORTS_SAVE, layout_changela )
