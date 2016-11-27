/******************************************************************

Mr F Lea
Pacific Novelty 1982

4 way joystick and jump button

I/O Board

 8910  D780C-1
 8910  8910
               SW2
               SW1              8259    8255

               6116  x  x  IO_C IO_A IO_D  x

CPU Board

 8255  D780C-1                  x  x  6116 6116

                 x  CPU_B5  x  CPU_B3  x  CPU_B1
                 x  CPU_D5  x  CPU_D3  x  CPU_D1

Video Board

                        82S19 82S19 82S19

                        82S19

        20MHz
                    93425 6116 6116 93425

                                        clr ram (7489x2)
                                        clr ram (7489x2)
                                        clr ram (7489x2)
           93422 93422
   x  x  VD_J11 VD_J10  x  x  VD_J7 VD_J6     VD_K4 VD_K3 VD_K2 VD_K1
   x  x  VD-L11 VD_L10  x  x  VD_L7 VD_L6     VD_L4 VD_L3 VD_L2 VD_L1

******************************************************************/

#include "emu.h"
#include "deprecat.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "includes/mrflea.h"


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

static WRITE8_HANDLER( mrflea_main_w )
{
	mrflea_state *state = (mrflea_state *)space->machine->driver_data;
	state->status |= 0x01; // pending command to main CPU
	state->main = data;
}

static WRITE8_HANDLER( mrflea_io_w )
{
	mrflea_state *state = (mrflea_state *)space->machine->driver_data;
	state->status |= 0x08; // pending command to IO CPU
	state->io = data;
	cpu_set_input_line(state->subcpu, 0, HOLD_LINE );
}

static READ8_HANDLER( mrflea_main_r )
{
	mrflea_state *state = (mrflea_state *)space->machine->driver_data;
	state->status &= ~0x01; // main CPU command read
	return state->main;
}

static READ8_HANDLER( mrflea_io_r )
{
	mrflea_state *state = (mrflea_state *)space->machine->driver_data;
	state->status &= ~0x08; // IO CPU command read
	return state->io;
}

static READ8_HANDLER( mrflea_main_status_r )
{
	mrflea_state *state = (mrflea_state *)space->machine->driver_data;

	/*  0x01: main CPU command pending
        0x08: io cpu ready */
	return state->status ^ 0x08;
}

static READ8_HANDLER( mrflea_io_status_r )
{
	mrflea_state *state = (mrflea_state *)space->machine->driver_data;

	/*  0x08: IO CPU command pending
        0x01: main cpu ready */
	return state->status ^ 0x01;
}

static INTERRUPT_GEN( mrflea_slave_interrupt )
{
	mrflea_state *state = (mrflea_state *)device->machine->driver_data;
	if (cpu_getiloops(device) == 0 || (state->status & 0x08))
		cpu_set_input_line(device, 0, HOLD_LINE);
}

static READ8_HANDLER( mrflea_interrupt_type_r )
{
/* there are two interrupt types:
    1. triggered (in response to sound command)
    2. heartbeat (for music timing)
*/
	mrflea_state *state = (mrflea_state *)space->machine->driver_data;

	if (state->status & 0x08 )
		return 0x00; /* process command */

	return 0x01; /* music/sound update? */
}

static WRITE8_HANDLER( mrflea_select1_w )
{
	mrflea_state *state = (mrflea_state *)space->machine->driver_data;
	state->select1 = data;
}

static READ8_HANDLER( mrflea_input1_r )
{
	return 0x00;
}

static WRITE8_HANDLER( mrflea_data1_w )
{
}

/*************************************
 *
 *  Address maps
 *
 *************************************/

static ADDRESS_MAP_START( mrflea_master_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xcfff) AM_RAM
	AM_RANGE(0xe000, 0xe7ff) AM_RAM_WRITE(mrflea_videoram_w) AM_BASE_MEMBER(mrflea_state, videoram)
	AM_RANGE(0xe800, 0xe83f) AM_RAM_WRITE(paletteram_xxxxRRRRGGGGBBBB_le_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0xec00, 0xecff) AM_RAM_WRITE(mrflea_spriteram_w) AM_BASE_MEMBER(mrflea_state, spriteram)
ADDRESS_MAP_END

static ADDRESS_MAP_START( mrflea_master_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITENOP /* watchdog? */
	AM_RANGE(0x40, 0x40) AM_WRITE(mrflea_io_w)
	AM_RANGE(0x41, 0x41) AM_READ(mrflea_main_r)
	AM_RANGE(0x42, 0x42) AM_READ(mrflea_main_status_r)
	AM_RANGE(0x43, 0x43) AM_WRITENOP /* 0xa6,0x0d,0x05 */
	AM_RANGE(0x60, 0x60) AM_WRITE(mrflea_gfx_bank_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( mrflea_slave_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x2000, 0x3fff) AM_ROM
	AM_RANGE(0x8000, 0x80ff) AM_RAM
	AM_RANGE(0x9000, 0x905a) AM_RAM /* ? */
ADDRESS_MAP_END

static ADDRESS_MAP_START( mrflea_slave_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITENOP /* watchdog */
	AM_RANGE(0x10, 0x10) AM_READ(mrflea_interrupt_type_r) AM_WRITENOP /* ? / irq ACK */
	AM_RANGE(0x11, 0x11) AM_WRITENOP /* 0x83,0x00,0xfc */
	AM_RANGE(0x20, 0x20) AM_READ(mrflea_io_r)
	AM_RANGE(0x21, 0x21) AM_WRITE(mrflea_main_w)
	AM_RANGE(0x22, 0x22) AM_READ(mrflea_io_status_r)
	AM_RANGE(0x23, 0x23) AM_WRITENOP /* 0xb4,0x09,0x05 */
	AM_RANGE(0x40, 0x40) AM_DEVREAD("ay1", ay8910_r)
	AM_RANGE(0x40, 0x41) AM_DEVWRITE("ay1", ay8910_data_address_w)
	AM_RANGE(0x42, 0x42) AM_READWRITE(mrflea_input1_r, mrflea_data1_w)
	AM_RANGE(0x43, 0x43) AM_WRITE(mrflea_select1_w)
	AM_RANGE(0x44, 0x44) AM_DEVREAD("ay2", ay8910_r)
	AM_RANGE(0x44, 0x45) AM_DEVWRITE("ay2", ay8910_data_address_w)
	AM_RANGE(0x46, 0x46) AM_DEVREAD("ay3", ay8910_r)
	AM_RANGE(0x46, 0x47) AM_DEVWRITE("ay3", ay8910_data_address_w)
ADDRESS_MAP_END

/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( mrflea )
	PORT_START("IN0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
/*
    ------xx
    -----x--
    ----x---
*/
	PORT_DIPNAME( 0x03, 0x03, "Bonus?" )
	PORT_DIPSETTING( 0x03, "A" )
	PORT_DIPSETTING( 0x02, "B" )
	PORT_DIPSETTING( 0x01, "C" )
	PORT_DIPSETTING( 0x00, "D" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING( 0x04, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING( 0x08, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) )
	PORT_DIPSETTING( 0x10, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) )
	PORT_DIPSETTING( 0x20, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )
	PORT_DIPSETTING( 0x40, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )
	PORT_DIPSETTING( 0x80, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_DIPSETTING( 0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING( 0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING( 0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING( 0x01, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )
	PORT_DIPSETTING( 0x0c, "3" )
	PORT_DIPSETTING( 0x08, "4" )
	PORT_DIPSETTING( 0x04, "5" )
	PORT_DIPSETTING( 0x00, "7" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) )
	PORT_DIPSETTING( 0x30, DEF_STR( Easy ) )
	PORT_DIPSETTING( 0x20, DEF_STR( Medium ) )
	PORT_DIPSETTING( 0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING( 0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )
	PORT_DIPSETTING( 0x40, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )
	PORT_DIPSETTING( 0x80, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
INPUT_PORTS_END


/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout tile_layout = {
	8,8,
	0x800, /* number of tiles */
	4,
	{ 0,1,2,3 },
	{ 0*4,1*4,2*4,3*4, 4*4,5*4,6*4,7*4 },
	{ 0*32,1*32,2*32,3*32, 4*32,5*32,6*32,7*32 },
	8*32
};

static const gfx_layout sprite_layout = {
	16,16,
	0x200, /* number of sprites */
	4,
	{ 0*0x4000*8,1*0x4000*8,2*0x4000*8,3*0x4000*8 },
	{ 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 },
	{
		0x00,0x10,0x20,0x30,0x40,0x50,0x60,0x70,
		0x80,0x90,0xa0,0xb0,0xc0,0xd0,0xe0,0xf0
	},
	16*16
};

static GFXDECODE_START( mrflea )
	GFXDECODE_ENTRY( "gfx1", 0, sprite_layout,	0x10, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, tile_layout,	0x00, 1 )
GFXDECODE_END

/*************************************
 *
 *  Sound interfaces
 *
 *************************************/

static const ay8910_interface mrflea_ay8910_interface_0 =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_INPUT_PORT("IN1"),
	DEVCB_INPUT_PORT("IN0")
};

static const ay8910_interface mrflea_ay8910_interface_1 =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_INPUT_PORT("DSW2"),
	DEVCB_INPUT_PORT("DSW1")
};

/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_START( mrflea )
{
	mrflea_state *state = (mrflea_state *)machine->driver_data;

	state->maincpu = machine->device("maincpu");
	state->subcpu = machine->device("sub");

	state_save_register_global(machine, state->gfx_bank);
	state_save_register_global(machine, state->io);
	state_save_register_global(machine, state->main);
	state_save_register_global(machine, state->status);
	state_save_register_global(machine, state->select1);
}

static MACHINE_RESET( mrflea )
{
	mrflea_state *state = (mrflea_state *)machine->driver_data;

	state->gfx_bank = 0;
	state->io = 0;
	state->main = 0;
	state->status = 0;
	state->select1 = 0;
}

static MACHINE_DRIVER_START( mrflea )

	/* driver data */
	MDRV_DRIVER_DATA(mrflea_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, 4000000) /* 4 MHz? */
	MDRV_CPU_PROGRAM_MAP(mrflea_master_map)
	MDRV_CPU_IO_MAP(mrflea_master_io_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold) /* NMI resets the game */

	MDRV_CPU_ADD("sub", Z80, 6000000)
	MDRV_CPU_PROGRAM_MAP(mrflea_slave_map)
	MDRV_CPU_IO_MAP(mrflea_slave_io_map)
	MDRV_CPU_VBLANK_INT_HACK(mrflea_slave_interrupt,2)

	MDRV_QUANTUM_TIME(HZ(6000))

	MDRV_MACHINE_START(mrflea)
	MDRV_MACHINE_RESET(mrflea)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 31*8-1)

	MDRV_GFXDECODE(mrflea)
	MDRV_PALETTE_LENGTH(32)

	MDRV_VIDEO_UPDATE(mrflea)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ay1", AY8910, 2000000)
	MDRV_SOUND_CONFIG(mrflea_ay8910_interface_0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MDRV_SOUND_ADD("ay2", AY8910, 2000000)
	MDRV_SOUND_CONFIG(mrflea_ay8910_interface_1)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MDRV_SOUND_ADD("ay3", AY8910, 2000000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_DRIVER_END

/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( mrflea )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* Z80 code; main CPU */
	ROM_LOAD( "cpu_d1",	0x0000, 0x2000, CRC(d286217c) SHA1(d750d64bb70f735a38b737881abb9a5fbde1c98c) )
	ROM_LOAD( "cpu_d3",	0x2000, 0x2000, CRC(95cf94bc) SHA1(dd0a51d79b0b28952e6177f36af93f296b3cd954) )
	ROM_LOAD( "cpu_d5",	0x4000, 0x2000, CRC(466ca77e) SHA1(513f41a888166a057d28bdc572571a713d77ae5f) )
	ROM_LOAD( "cpu_b1",	0x6000, 0x2000, CRC(721477d6) SHA1(a8a491fcd17a392ca40abfef892dfbc236fd6e0c) )
	ROM_LOAD( "cpu_b3",	0x8000, 0x2000, CRC(f55b01e4) SHA1(93689fa02aab9d1f1acd55b305eafe542ee447b8) )
	ROM_LOAD( "cpu_b5",	0xa000, 0x2000, CRC(79f560aa) SHA1(7326693d7369682f5770bf80df0181d603212900) )

	ROM_REGION( 0x10000, "sub", 0 ) /* Z80 code; IO CPU */
	ROM_LOAD( "io_a11",	0x0000, 0x1000, CRC(7a20c3ee) SHA1(8e0d5770881e6d3d1df17a2ede5a8823ca9d78e3) )
	ROM_LOAD( "io_c11",	0x2000, 0x1000, CRC(8d26e0c8) SHA1(e90e37bd64e991dc47ab80394337073c69b450da) )
	ROM_LOAD( "io_d11",	0x3000, 0x1000, CRC(abd9afc0) SHA1(873314164707ee84739ec76c6119a65a17001620) )

	ROM_REGION( 0x10000, "gfx1", 0 ) /* sprites */
	ROM_LOAD( "vd_l10",	0x0000, 0x2000, CRC(48b2adf9) SHA1(91390cdbd8df610edec87c1681db1576e2f3c58d) )
	ROM_LOAD( "vd_l11",	0x2000, 0x2000, CRC(2ff168c0) SHA1(e24b6a33e9ce50771983db8b8de7e79a1e87929c) )
	ROM_LOAD( "vd_l6",	0x4000, 0x2000, CRC(100158ca) SHA1(83a619e5897a2b379eb7a72fde3e1bc08b7a34c4) )
	ROM_LOAD( "vd_l7",	0x6000, 0x2000, CRC(34501577) SHA1(4b41fbc3d9ebf562aadfb1a96a5b3e177cac34c7) )
	ROM_LOAD( "vd_j10",	0x8000, 0x2000, CRC(3f29b8c3) SHA1(99f306f9c0ec20e690d5a87911cd48ae2b336560) )
	ROM_LOAD( "vd_j11",	0xa000, 0x2000, CRC(39380bea) SHA1(68e4213ef2a1502f74b1dc7af73ef5b355ed5f66) )
	ROM_LOAD( "vd_j6",	0xc000, 0x2000, CRC(2b4b110e) SHA1(37644113b2ce7bd525697ebb2fc8cb295c228a60) )
	ROM_LOAD( "vd_j7",	0xe000, 0x2000, CRC(3a3c8b1e) SHA1(5991d80990212ffe92c546b0e4b4e01c68fdd0cd) )

	ROM_REGION( 0x10000, "gfx2", 0 ) /* characters */
	ROM_LOAD( "vd_k1",	0x0000, 0x2000, CRC(7540e3a7) SHA1(e292e7ec47eaefee8bec1585ec33ea4e6cb64e81) )
	ROM_LOAD( "vd_k2",	0x2000, 0x2000, CRC(6c688219) SHA1(323640b99d9e39b327f500ff2ae6a7f8d0da3ada) )
	ROM_LOAD( "vd_k3",	0x4000, 0x2000, CRC(15e96f3c) SHA1(e57a219666dd440909d3fb75d9a5708cbb904389) )
	ROM_LOAD( "vd_k4",	0x6000, 0x2000, CRC(fe5100df) SHA1(17833f26527f570a3d7365e977492a81ab4e8669) )
	ROM_LOAD( "vd_l1",	0x8000, 0x2000, CRC(d1e3d056) SHA1(5277fdcea9c00f90396bd3120b3221c52f2e3f98) )
	ROM_LOAD( "vd_l2",	0xa000, 0x2000, CRC(4d7fb925) SHA1(dc5224318451a59b020996a513269698a6d19972) )
	ROM_LOAD( "vd_l3",	0xc000, 0x2000, CRC(6d81588a) SHA1(8dbc53d7034a661f9d9afd99f3a3cb5dff3ff137) )
	ROM_LOAD( "vd_l4",	0xe000, 0x2000, CRC(423735a5) SHA1(4ee93f93cd2b08560e148525e08880d64c64fcd2) )
ROM_END

/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1982, mrflea,   0,        mrflea,   mrflea,   0,        ROT270, "Pacific Novelty", "The Amazing Adventures of Mr. F. Lea" , GAME_SUPPORTS_SAVE )
