/***************************************************************************
Big Event Golf (c) Taito 1986

driver by Jaroslaw Burczynski
          Tomasz Slanina


****************************************************************************


Taito 1986  M4300056B

K1100156A Sound
J1100068A
                                YM2149     2016
                                MSM5232    A67-16
                                Z80        A67-17
                                           A67-18
                                8MHz
-----------------------------------------------------
K1100215A CPU
J1100066A

2064
A67-21
A67-20      A67-03
Z80         A67-04
            A67-05
            A67-06                          2016
            A67-07
            A67-08
            A67-09
            A67-10-2          2016
                              A67-11
            10MHz             Z80
                                   A67-19-1 (68705P5)
----------------------------------------------------
K1100215A VIDEO
J1100072A

                                 41464            2148
  93422                          41464            2148
  93422                          41464            2148
  93422                          41464
  93422                          41464
                                 41464
                                 41464
                                 41464


       A67-12-1                2016
       A67-13-1
       A67-14-1     2016
       A67-15-1                      18.432MHz

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "deprecat.h"
#include "sound/ay8910.h"
#include "sound/msm5232.h"
#include "cpu/m6805/m6805.h"
#include "includes/bigevglf.h"


static WRITE8_HANDLER( beg_banking_w )
{
	bigevglf_state *state = (bigevglf_state *)space->machine->driver_data;
	state->beg_bank = data;

/* d0-d3 connect to A11-A14 of the ROMs (via ls273 latch)
   d4-d7 select one of ROMs (via ls273(above) and then ls154)
*/
	memory_set_bank(space->machine, "bank1", state->beg_bank & 0xff); /* empty sockets for IC37-IC44 ROMS */
}

static TIMER_CALLBACK( from_sound_latch_callback )
{
	bigevglf_state *state = (bigevglf_state *)machine->driver_data;
	state->from_sound = param & 0xff;
	state->sound_state |= 2;
}
static WRITE8_HANDLER( beg_fromsound_w )	/* write to D800 sets bit 1 in status */
{
	timer_call_after_resynch(space->machine, NULL, (cpu_get_pc(space->cpu) << 16) | data, from_sound_latch_callback);
}

static READ8_HANDLER( beg_fromsound_r )
{
	bigevglf_state *state = (bigevglf_state *)space->machine->driver_data;
	/* set a timer to force synchronization after the read */
	timer_call_after_resynch(space->machine, NULL, 0, NULL);
	return state->from_sound;
}

static READ8_HANDLER( beg_soundstate_r )
{
	bigevglf_state *state = (bigevglf_state *)space->machine->driver_data;
	UINT8 ret = state->sound_state;
	/* set a timer to force synchronization after the read */
	timer_call_after_resynch(space->machine, NULL, 0, NULL);
	state->sound_state &= ~2; /* read from port 21 clears bit 1 in status */
	return ret;
}

static READ8_HANDLER( soundstate_r )
{
	bigevglf_state *state = (bigevglf_state *)space->machine->driver_data;
	/* set a timer to force synchronization after the read */
	timer_call_after_resynch(space->machine, NULL, 0, NULL);
	return state->sound_state;
}

static TIMER_CALLBACK( nmi_callback )
{
	bigevglf_state *state = (bigevglf_state *)machine->driver_data;

	if (state->sound_nmi_enable)
		cpu_set_input_line(state->audiocpu, INPUT_LINE_NMI, PULSE_LINE);
	else
		state->pending_nmi = 1;
	state->sound_state &= ~1;
}

static WRITE8_HANDLER( sound_command_w )	/* write to port 20 clears bit 0 in status */
{
	bigevglf_state *state = (bigevglf_state *)space->machine->driver_data;
	state->for_sound = data;
	timer_call_after_resynch(space->machine, NULL, data, nmi_callback);
}

static READ8_HANDLER( sound_command_r )	/* read from D800 sets bit 0 in status */
{
	bigevglf_state *state = (bigevglf_state *)space->machine->driver_data;
	state->sound_state |= 1;
	return state->for_sound;
}

static WRITE8_HANDLER( nmi_disable_w )
{
	bigevglf_state *state = (bigevglf_state *)space->machine->driver_data;
	state->sound_nmi_enable = 0;
}

static WRITE8_HANDLER( nmi_enable_w )
{
	bigevglf_state *state = (bigevglf_state *)space->machine->driver_data;
	state->sound_nmi_enable = 1;
	if (state->pending_nmi)
	{
		cpu_set_input_line(state->audiocpu, INPUT_LINE_NMI, PULSE_LINE);
		state->pending_nmi = 0;
	}
}

static TIMER_CALLBACK( deferred_ls74_w )
{
	bigevglf_state *state = (bigevglf_state *)machine->driver_data;
	int offs = (param >> 8) & 255;
	int data = param & 255;
	state->beg13_ls74[offs] = data;
}

/* do this on a timer to let the CPUs synchronize */
static WRITE8_HANDLER( beg13_a_clr_w )
{
	timer_call_after_resynch(space->machine, NULL, (0 << 8) | 0, deferred_ls74_w);
}

static WRITE8_HANDLER( beg13_b_clr_w )
{
	timer_call_after_resynch(space->machine, NULL, (1 << 8) | 0, deferred_ls74_w);
}

static WRITE8_HANDLER( beg13_a_set_w )
{
	timer_call_after_resynch(space->machine, NULL, (0 << 8) | 1, deferred_ls74_w);
}

static WRITE8_HANDLER( beg13_b_set_w )
{
	timer_call_after_resynch(space->machine, NULL, (1 << 8) | 1, deferred_ls74_w);
}

static READ8_HANDLER( beg_status_r )
{
	bigevglf_state *state = (bigevglf_state *)space->machine->driver_data;

/* d0 = Q of 74ls74 IC13(partA)
   d1 = Q of 74ls74 IC13(partB)
   d2 =
   d3 =
   d4 =
   d5 =
   d6 = d7 = 10MHz/2

*/
	/* set a timer to force synchronization after the read */
	timer_call_after_resynch(space->machine, NULL, 0, NULL);
	return (state->beg13_ls74[0] << 0) | (state->beg13_ls74[1] << 1);
}


static READ8_HANDLER( beg_trackball_x_r )
{
	bigevglf_state *state = (bigevglf_state *)space->machine->driver_data;
	static const char *const portx_name[2] = { "P1X", "P2X" };

	return input_port_read(space->machine, portx_name[state->port_select]);
}

static READ8_HANDLER( beg_trackball_y_r )
{
	bigevglf_state *state = (bigevglf_state *)space->machine->driver_data;
	static const char *const porty_name[2] = { "P1Y", "P2Y" };

	return input_port_read(space->machine, porty_name[state->port_select]);
}

static WRITE8_HANDLER( beg_port08_w )
{
	bigevglf_state *state = (bigevglf_state *)space->machine->driver_data;
	state->port_select = (data & 0x04) >> 2;
}


static INPUT_PORTS_START( bigevglf )
	PORT_START("PORT00")		/* port 00 on sub cpu */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("PORT04")		/* port 04 on sub cpu - bit 0 and bit 1 are coin inputs */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )

	PORT_START("DSW1")			/* port 05 on sub cpu */
	PORT_DIPNAME( 0x01,   0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02,   0x02, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x04, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPNAME( 0x08,   0x08, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0xf0,   0xf0, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(      0x50, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0xa0, DEF_STR( 1C_2C ) )

	PORT_START("DSW2")			/* port 06 on sub cpu */
	PORT_DIPNAME( 0x03,   0x03, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x01, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c,   0x0c, "Holes" ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x04, "1" )
	PORT_DIPSETTING(      0x08, "2" )
	PORT_DIPSETTING(      0x0c, "3" )
	PORT_DIPSETTING(      0x00, "4" )
	PORT_DIPNAME( 0x10,   0x00, DEF_STR( Language ) ) PORT_DIPLOCATION("SW2:5") /* Also changes the copyright on the title screen */
	PORT_DIPSETTING(      0x00, DEF_STR( English ) )                            /* (c) 1986 Taito America Corp. */
	PORT_DIPSETTING(      0x10, DEF_STR( Japanese ) )                           /* (c) Taito Corporation 1986 */
	PORT_DIPNAME( 0xe0,   0xa0, "Full game price (credits)" ) PORT_DIPLOCATION("SW2:6,7,8")
	PORT_DIPSETTING(      0xe0, "3" )
	PORT_DIPSETTING(      0xc0, "4" )
	PORT_DIPSETTING(      0xa0, "5" )
	PORT_DIPSETTING(      0x80, "6" )
	PORT_DIPSETTING(      0x60, "7" )
	PORT_DIPSETTING(      0x40, "8" )
	PORT_DIPSETTING(      0x20, "9" )
	PORT_DIPSETTING(      0x00, "10" )

	PORT_START("P1X")	/* port 02 on sub cpu - muxed port 0 */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(30) PORT_KEYDELTA(10)

	PORT_START("P1Y")	/* port 03 on sub cpu - muxed port 0 */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_REVERSE

	PORT_START("P2X")	/* port 02 on sub cpu - muxed port 1 */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_COCKTAIL

	PORT_START("P2Y")	/* port 03 on sub cpu - muxed port 1 */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_REVERSE PORT_COCKTAIL
INPUT_PORTS_END

static INPUT_PORTS_START( bigevglfj )
	PORT_INCLUDE(bigevglf)

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x10,   0x10, DEF_STR( Language ) ) PORT_DIPLOCATION("SW2:5") /* Doesn't change the title screen copyright like the US set */
	PORT_DIPSETTING(      0x00, DEF_STR( English ) )
	PORT_DIPSETTING(      0x10, DEF_STR( Japanese ) )
INPUT_PORTS_END


/*****************************************************************************/
/* Main CPU */

static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xcfff) AM_RAM
	AM_RANGE(0xd000, 0xd7ff) AM_ROMBANK("bank1")
	AM_RANGE(0xd800, 0xdbff) AM_RAM AM_SHARE("share1") /* only half of the RAM is accessible, line a10 of IC73 (6116) is GNDed */
	AM_RANGE(0xe000, 0xe7ff) AM_WRITE(bigevglf_palette_w) AM_BASE_MEMBER(bigevglf_state, paletteram)
	AM_RANGE(0xe800, 0xefff) AM_WRITEONLY AM_BASE_MEMBER(bigevglf_state, spriteram1) /* sprite 'templates' */
	AM_RANGE(0xf000, 0xf0ff) AM_READWRITE(bigevglf_vidram_r, bigevglf_vidram_w) /* 41464 (64kB * 8 chips), addressed using ports 1 and 5 */
	AM_RANGE(0xf840, 0xf8ff) AM_RAM AM_BASE_MEMBER(bigevglf_state, spriteram2)  /* spriteram (x,y,offset in spriteram1,palette) */
ADDRESS_MAP_END

static ADDRESS_MAP_START( bigevglf_portmap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITENOP	/* video ram enable ???*/
	AM_RANGE(0x01, 0x01) AM_WRITE(bigevglf_gfxcontrol_w)  /* plane select */
	AM_RANGE(0x02, 0x02) AM_WRITE(beg_banking_w)
	AM_RANGE(0x03, 0x03) AM_WRITE(beg13_a_set_w)
	AM_RANGE(0x04, 0x04) AM_WRITE(beg13_b_clr_w)
	AM_RANGE(0x05, 0x05) AM_WRITE(bigevglf_vidram_addr_w)	/* video banking (256 banks) for f000-f0ff area */
	AM_RANGE(0x06, 0x06) AM_READ(beg_status_r)
ADDRESS_MAP_END


/*********************************************************************************/
/* Sub CPU */

static ADDRESS_MAP_START( sub_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x47ff) AM_RAM
	AM_RANGE(0x8000, 0x83ff) AM_RAM AM_SHARE("share1") /* shared with main CPU */
ADDRESS_MAP_END


static READ8_HANDLER( sub_cpu_mcu_coin_port_r )
{
	bigevglf_state *state = (bigevglf_state *)space->machine->driver_data;
	/*
            bit 0 and bit 1 = coin inputs
            bit 3 and bit 4 = MCU status
            bit 5           = must toggle, vblank ?

    */
	state->mcu_coin_bit5 ^= 0x20;
	return bigevglf_mcu_status_r(space, 0) | (input_port_read(space->machine, "PORT04") & 3) | state->mcu_coin_bit5;	/* bit 0 and bit 1 - coin inputs */
}

static ADDRESS_MAP_START( bigevglf_sub_portmap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("PORT00")
	AM_RANGE(0x01, 0x01) AM_READNOP
	AM_RANGE(0x02, 0x02) AM_READ(beg_trackball_x_r)
	AM_RANGE(0x03, 0x03) AM_READ(beg_trackball_y_r)
	AM_RANGE(0x04, 0x04) AM_READ(sub_cpu_mcu_coin_port_r)
	AM_RANGE(0x05, 0x05) AM_READ_PORT("DSW1")
	AM_RANGE(0x06, 0x06) AM_READ_PORT("DSW2")
	AM_RANGE(0x07, 0x07) AM_READNOP
	AM_RANGE(0x08, 0x08) AM_WRITE(beg_port08_w) /* muxed port select + other unknown stuff */
	AM_RANGE(0x0b, 0x0b) AM_READ(bigevglf_mcu_r)
	AM_RANGE(0x0c, 0x0c) AM_WRITE(bigevglf_mcu_w)
	AM_RANGE(0x0e, 0x0e) AM_WRITENOP /* 0-enable MCU, 1-keep reset line ASSERTED; D0 goes to the input of ls74 and the /Q of this ls74 goes to reset line on 68705 */
	AM_RANGE(0x10, 0x17) AM_WRITE(beg13_a_clr_w)
	AM_RANGE(0x18, 0x1f) AM_WRITE(beg13_b_set_w)
	AM_RANGE(0x20, 0x20) AM_READWRITE(beg_fromsound_r, sound_command_w)
	AM_RANGE(0x21, 0x21) AM_READ(beg_soundstate_r)
ADDRESS_MAP_END




/*********************************************************************************/
/* Sound CPU */

static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
	AM_RANGE(0xc800, 0xc801) AM_DEVWRITE("aysnd", ay8910_address_data_w)
	AM_RANGE(0xca00, 0xca0d) AM_DEVWRITE("msm", msm5232_w)
	AM_RANGE(0xcc00, 0xcc00) AM_WRITENOP
	AM_RANGE(0xce00, 0xce00) AM_WRITENOP
	AM_RANGE(0xd800, 0xd800) AM_READWRITE(sound_command_r, beg_fromsound_w)	/* write to D800 sets bit 1 in status */
	AM_RANGE(0xda00, 0xda00) AM_READWRITE(soundstate_r, nmi_enable_w)
	AM_RANGE(0xdc00, 0xdc00) AM_WRITE(nmi_disable_w)
	AM_RANGE(0xde00, 0xde00) AM_WRITENOP
	AM_RANGE(0xe000, 0xefff) AM_READNOP		/* space for diagnostics ROM */
ADDRESS_MAP_END


/*********************************************************************************/
/* MCU */

static ADDRESS_MAP_START( m68705_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0x7ff)
	AM_RANGE(0x0000, 0x0000) AM_READWRITE(bigevglf_68705_port_a_r, bigevglf_68705_port_a_w)
	AM_RANGE(0x0001, 0x0001) AM_READWRITE(bigevglf_68705_port_b_r, bigevglf_68705_port_b_w)
	AM_RANGE(0x0002, 0x0002) AM_READWRITE(bigevglf_68705_port_c_r, bigevglf_68705_port_c_w)
	AM_RANGE(0x0004, 0x0004) AM_WRITE(bigevglf_68705_ddr_a_w)
	AM_RANGE(0x0005, 0x0005) AM_WRITE(bigevglf_68705_ddr_b_w)
	AM_RANGE(0x0006, 0x0006) AM_WRITE(bigevglf_68705_ddr_c_w)
	AM_RANGE(0x0010, 0x007f) AM_RAM
	AM_RANGE(0x0080, 0x07ff) AM_ROM
ADDRESS_MAP_END


static const gfx_layout gfxlayout =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{ 0, RGN_FRAC(1,4),RGN_FRAC(2,4),RGN_FRAC(3,4)},
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8 },
	8*8
};

static GFXDECODE_START( bigevglf )
	GFXDECODE_ENTRY( "gfx1", 0, gfxlayout,   0x20*16, 16 )
GFXDECODE_END

static const msm5232_interface msm5232_config =
{
	{ 0.65e-6, 0.65e-6, 0.65e-6, 0.65e-6, 0.65e-6, 0.65e-6, 0.65e-6, 0.65e-6 }	/* 0.65 (???) uF capacitors */
};

static MACHINE_START( bigevglf )
{
	bigevglf_state *state = (bigevglf_state *)machine->driver_data;

	state->audiocpu = machine->device("audiocpu");
	state->mcu = machine->device("mcu");

	state_save_register_global(machine, state->vidram_bank);
	state_save_register_global(machine, state->plane_selected);
	state_save_register_global(machine, state->plane_visible);

	state_save_register_global_array(machine, state->beg13_ls74);
	state_save_register_global(machine, state->beg_bank);
	state_save_register_global(machine, state->port_select);

	state_save_register_global(machine, state->sound_nmi_enable);
	state_save_register_global(machine, state->pending_nmi);
	state_save_register_global(machine, state->for_sound);
	state_save_register_global(machine, state->from_sound);
	state_save_register_global(machine, state->sound_state);

	state_save_register_global(machine, state->main_sent);
	state_save_register_global(machine, state->mcu_sent);
	state_save_register_global(machine, state->mcu_coin_bit5);

	state_save_register_global(machine, state->port_a_in);
	state_save_register_global(machine, state->port_a_out);
	state_save_register_global(machine, state->ddr_a);
	state_save_register_global(machine, state->port_b_in);
	state_save_register_global(machine, state->port_b_out);
	state_save_register_global(machine, state->ddr_b);
	state_save_register_global(machine, state->port_c_in);
	state_save_register_global(machine, state->port_c_out);
	state_save_register_global(machine, state->ddr_c);
	state_save_register_global(machine, state->from_mcu);
}

static MACHINE_RESET( bigevglf )
{
	bigevglf_state *state = (bigevglf_state *)machine->driver_data;

	state->vidram_bank = 0;
	state->plane_selected = 0;
	state->plane_visible = 0;

	state->beg13_ls74[0] = 0;
	state->beg13_ls74[1] = 0;
	state->beg_bank = 0;
	state->port_select = 0;

	state->sound_nmi_enable = 0;
	state->pending_nmi = 0;
	state->for_sound = 0;
	state->from_sound = 0;
	state->sound_state = 0;

	state->main_sent = 0;
	state->mcu_sent = 0;
	state->mcu_coin_bit5 = 0;

	state->port_a_in = 0;
	state->port_a_out = 0;
	state->ddr_a = 0;
	state->port_b_in = 0;
	state->port_b_out = 0;
	state->ddr_b = 0;
	state->port_c_in = 0;
	state->port_c_out = 0;
	state->ddr_c = 0;
	state->from_mcu = 0;
}


static MACHINE_DRIVER_START( bigevglf )

	/* driver data */
	MDRV_DRIVER_DATA(bigevglf_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80,10000000/2)		/* 5 MHz ? */
	MDRV_CPU_PROGRAM_MAP(main_map)
	MDRV_CPU_IO_MAP(bigevglf_portmap)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)	/* vblank */

	MDRV_CPU_ADD("sub", Z80,10000000/2)		/* 5 MHz ? */
	MDRV_CPU_PROGRAM_MAP(sub_map)
	MDRV_CPU_IO_MAP(bigevglf_sub_portmap)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)	/* vblank */

	MDRV_CPU_ADD("audiocpu", Z80,8000000/2)	/* 4 MHz ? */
	MDRV_CPU_PROGRAM_MAP(sound_map)
	MDRV_CPU_VBLANK_INT_HACK(irq0_line_hold,2)	/* IRQ generated by ???;
        2 irqs/frame give good music tempo but also SOUND ERROR in test mode,
        4 irqs/frame give SOUND OK in test mode but music seems to be running too fast */

	MDRV_CPU_ADD("mcu", M68705,2000000)	/* ??? */
	MDRV_CPU_PROGRAM_MAP(m68705_map)

	MDRV_QUANTUM_TIME(HZ(600))	/* 10 CPU slices per frame - interleaving is forced on the fly */

	MDRV_MACHINE_START(bigevglf)
	MDRV_MACHINE_RESET(bigevglf)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MDRV_GFXDECODE(bigevglf)
	MDRV_PALETTE_LENGTH(0x800)
	MDRV_VIDEO_START(bigevglf)
	MDRV_VIDEO_UPDATE(bigevglf)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("aysnd", AY8910, 8000000/4)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15) /* YM2149 really */

	MDRV_SOUND_ADD("msm", MSM5232, 8000000/4)
	MDRV_SOUND_CONFIG(msm5232_config)
	MDRV_SOUND_ROUTE(0, "mono", 1.0)	// pin 28  2'-1
	MDRV_SOUND_ROUTE(1, "mono", 1.0)	// pin 29  4'-1
	MDRV_SOUND_ROUTE(2, "mono", 1.0)	// pin 30  8'-1
	MDRV_SOUND_ROUTE(3, "mono", 1.0)	// pin 31 16'-1
	MDRV_SOUND_ROUTE(4, "mono", 1.0)	// pin 36  2'-2
	MDRV_SOUND_ROUTE(5, "mono", 1.0)	// pin 35  4'-2
	MDRV_SOUND_ROUTE(6, "mono", 1.0)	// pin 34  8'-2
	MDRV_SOUND_ROUTE(7, "mono", 1.0)	// pin 33 16'-2
	// pin 1 SOLO  8'       not mapped
	// pin 2 SOLO 16'       not mapped
	// pin 22 Noise Output  not mapped
MACHINE_DRIVER_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( bigevglf )
	ROM_REGION( 0x90000, "maincpu", 0 )
	ROM_LOAD( "a67-21",   0x00000, 0x8000, CRC(2a62923d) SHA1(7b025180e203f268ae4d11baa18096e0a5704f77))
	ROM_LOAD( "a67-20",   0x08000, 0x4000, CRC(841561b1) SHA1(5d91449e135ef22508194a9543343c29e1c496cf))
	ROM_LOAD( "a67-03",   0x10000, 0x8000, CRC(695b3396) SHA1(2a3738e6bc492a4c68d2e15a2e474f7654aeb94d))
	ROM_LOAD( "a67-04",   0x18000, 0x8000, CRC(b8941902) SHA1(a03e432cbd8ea1df7223ea99ff1db220a57fc698))
	ROM_LOAD( "a67-05",   0x20000, 0x8000, CRC(681f5f4f) SHA1(2a5d8eeaf6ac697d5d4ee15164b6c4b1b81d7a29))
	ROM_LOAD( "a67-06",   0x28000, 0x8000, CRC(026f6fe5) SHA1(923b7d8363e587ef20b6518bee968d378166c76b))
	ROM_LOAD( "a67-07",   0x30000, 0x8000, CRC(27706bed) SHA1(da702c5a098eb106332996ec5d0e2c014782031e))
	ROM_LOAD( "a67-08",   0x38000, 0x8000, CRC(e922023a) SHA1(ea4c4b5e2f82ab20afb15f115e8cbc66d8471927))
	ROM_LOAD( "a67-09",   0x40000, 0x8000, CRC(a9d4263e) SHA1(8c3f2d541583e8e4b22e0beabcd04c5765508535))
	ROM_LOAD( "a67-10",   0x48000, 0x8000, CRC(7c35f4a3) SHA1(de60dc991c67fb7d48314bc8b2c1a27ad040bf1e))

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "a67-11",   0x00000, 0x4000, CRC(a2660d20) SHA1(3d8b670c071862d677d4e30655dcb6b8d830648a))

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a67-16",   0x0000, 0x4000, CRC(5fb6d22e) SHA1(1701aa94b7f524187fd7213a94535bed3e8b6cc9))
	ROM_LOAD( "a67-17",   0x4000, 0x4000, CRC(9f57deae) SHA1(dbdb3d77c3de0113ef6671aec854e4e44ee162ef))
	ROM_LOAD( "a67-18",   0x8000, 0x4000, CRC(40d54fed) SHA1(bfa0922809bffafec15d3ef59ac8b8ad653860a1))

	ROM_REGION( 0x0800, "mcu", 0 )
	ROM_LOAD( "a67_19-1", 0x0000, 0x0800, CRC(25691658) SHA1(aabf47abac43abe2ffed18ead1cb94e587149e6e))

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "a67-12",   0x00000, 0x8000, CRC(980cc3c5) SHA1(c35c20cfff0b5cfd5b95742333ae20a2c93371b5))
	ROM_LOAD( "a67-13",   0x08000, 0x8000, CRC(ad6e04af) SHA1(4680d789cf53c4808105ad4f3c70aedb6d8bcf36))
	ROM_LOAD( "a67-14",   0x10000, 0x8000, CRC(d6708cce) SHA1(5b48f9dff2a3e28242dc2004469dc2ac2b5d0321))
	ROM_LOAD( "a67-15",   0x18000, 0x8000, CRC(1d261428) SHA1(0f3e6d83a8a462436fa414de4e1e4306db869d3e))
ROM_END

ROM_START( bigevglfj )
	ROM_REGION( 0x90000, "maincpu", 0 )
	ROM_LOAD( "a67-02-2.10", 0x00000, 0x8000, CRC(b3edbb78) SHA1(7873b1a94cca830f1d1c143376cb49f6e48dbf0b))
	ROM_LOAD( "a67-01-2.9",  0x08000, 0x4000, CRC(7788b5d0) SHA1(f331138352c4d7b4566d342047785ed97e7b5990))
	ROM_LOAD( "a67-03",      0x10000, 0x8000, CRC(695b3396) SHA1(2a3738e6bc492a4c68d2e15a2e474f7654aeb94d))
	ROM_LOAD( "a67-04",      0x18000, 0x8000, CRC(b8941902) SHA1(a03e432cbd8ea1df7223ea99ff1db220a57fc698))
	ROM_LOAD( "a67-05",      0x20000, 0x8000, CRC(681f5f4f) SHA1(2a5d8eeaf6ac697d5d4ee15164b6c4b1b81d7a29))
	ROM_LOAD( "a67-06",      0x28000, 0x8000, CRC(026f6fe5) SHA1(923b7d8363e587ef20b6518bee968d378166c76b))
	ROM_LOAD( "a67-07",      0x30000, 0x8000, CRC(27706bed) SHA1(da702c5a098eb106332996ec5d0e2c014782031e))
	ROM_LOAD( "a67-08",      0x38000, 0x8000, CRC(e922023a) SHA1(ea4c4b5e2f82ab20afb15f115e8cbc66d8471927))
	ROM_LOAD( "a67-09",      0x40000, 0x8000, CRC(a9d4263e) SHA1(8c3f2d541583e8e4b22e0beabcd04c5765508535))
	ROM_LOAD( "a67-10",      0x48000, 0x8000, CRC(7c35f4a3) SHA1(de60dc991c67fb7d48314bc8b2c1a27ad040bf1e))

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "a67-11",   0x00000, 0x4000, CRC(a2660d20) SHA1(3d8b670c071862d677d4e30655dcb6b8d830648a))

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a67-16",   0x0000, 0x4000, CRC(5fb6d22e) SHA1(1701aa94b7f524187fd7213a94535bed3e8b6cc9))
	ROM_LOAD( "a67-17",   0x4000, 0x4000, CRC(9f57deae) SHA1(dbdb3d77c3de0113ef6671aec854e4e44ee162ef))
	ROM_LOAD( "a67-18",   0x8000, 0x4000, CRC(40d54fed) SHA1(bfa0922809bffafec15d3ef59ac8b8ad653860a1))

	ROM_REGION( 0x0800, "mcu", 0 )
	ROM_LOAD( "a67_19-1", 0x0000, 0x0800, CRC(25691658) SHA1(aabf47abac43abe2ffed18ead1cb94e587149e6e))

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "a67-12",   0x00000, 0x8000, CRC(980cc3c5) SHA1(c35c20cfff0b5cfd5b95742333ae20a2c93371b5))
	ROM_LOAD( "a67-13",   0x08000, 0x8000, CRC(ad6e04af) SHA1(4680d789cf53c4808105ad4f3c70aedb6d8bcf36))
	ROM_LOAD( "a67-14",   0x10000, 0x8000, CRC(d6708cce) SHA1(5b48f9dff2a3e28242dc2004469dc2ac2b5d0321))
	ROM_LOAD( "a67-15",   0x18000, 0x8000, CRC(1d261428) SHA1(0f3e6d83a8a462436fa414de4e1e4306db869d3e))
ROM_END

static DRIVER_INIT( bigevglf )
{
	UINT8 *ROM = memory_region(machine, "maincpu");
	memory_configure_bank(machine, "bank1", 0, 0xff, &ROM[0x10000], 0x800);
}

GAME( 1986, bigevglf,  0,        bigevglf, bigevglf, bigevglf, ROT270, "Taito America Corporation", "Big Event Golf (US)", GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
GAME( 1986, bigevglfj, bigevglf, bigevglf, bigevglfj,bigevglf, ROT270, "Taito Corporation", "Big Event Golf (Japan)", GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
