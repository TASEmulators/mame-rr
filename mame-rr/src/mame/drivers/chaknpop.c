/*
 *  Chack'n Pop
 *  (C) 1983 TAITO Corp.
 *
 * driver by BUT
 */

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "includes/chaknpop.h"


/***************************************************************************

  Memory Handler(s)

***************************************************************************/

static WRITE8_DEVICE_HANDLER ( unknown_port_1_w )
{
	//logerror("%s: write to unknow port 1: 0x%02x\n", cpuexec_describe_context(device->machine), data);
}

static WRITE8_DEVICE_HANDLER ( unknown_port_2_w )
{
	//logerror("%s: write to unknow port 2: 0x%02x\n", cpuexec_describe_context(device->machine), data);
}

static WRITE8_HANDLER ( coinlock_w )
{
	logerror("%04x: coin lock %sable\n", cpu_get_pc(space->cpu), data ? "dis" : "en");
}


/***************************************************************************

  Memory Map(s)

***************************************************************************/

static ADDRESS_MAP_START( chaknpop_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM AM_BASE_MEMBER(chaknpop_state, mcu_ram)
	AM_RANGE(0x8800, 0x8800) AM_READWRITE(chaknpop_mcu_port_a_r, chaknpop_mcu_port_a_w)
	AM_RANGE(0x8801, 0x8801) AM_READWRITE(chaknpop_mcu_port_b_r, chaknpop_mcu_port_b_w)
	AM_RANGE(0x8802, 0x8802) AM_READWRITE(chaknpop_mcu_port_c_r, chaknpop_mcu_port_c_w)
	AM_RANGE(0x8804, 0x8805) AM_DEVREADWRITE("ay1", ay8910_r, ay8910_address_data_w)
	AM_RANGE(0x8806, 0x8807) AM_DEVREADWRITE("ay2", ay8910_r, ay8910_address_data_w)
	AM_RANGE(0x8808, 0x8808) AM_READ_PORT("DSWC")
	AM_RANGE(0x8809, 0x8809) AM_READ_PORT("P1")
	AM_RANGE(0x880a, 0x880a) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x880b, 0x880b) AM_READ_PORT("P2")
	AM_RANGE(0x880c, 0x880c) AM_READWRITE(chaknpop_gfxmode_r, chaknpop_gfxmode_w)
	AM_RANGE(0x880d, 0x880d) AM_WRITE(coinlock_w)												// coin lock out
	AM_RANGE(0x9000, 0x93ff) AM_RAM_WRITE(chaknpop_txram_w) AM_BASE_MEMBER(chaknpop_state, tx_ram)			// TX tilemap
	AM_RANGE(0x9800, 0x983f) AM_RAM_WRITE(chaknpop_attrram_w) AM_BASE_MEMBER(chaknpop_state, attr_ram)		// Color attribute
	AM_RANGE(0x9840, 0x98ff) AM_RAM AM_BASE_SIZE_MEMBER(chaknpop_state, spr_ram, spr_ram_size)	// sprite
	AM_RANGE(0xa000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xffff) AM_RAMBANK("bank1")														// bitmap plane 1-4
ADDRESS_MAP_END

static const ay8910_interface ay8910_interface_1 =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_INPUT_PORT("DSWA"),		// DSW A
	DEVCB_INPUT_PORT("DSWB"),		// DSW B
	DEVCB_NULL,
	DEVCB_NULL
};

static const ay8910_interface ay8910_interface_2 =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_HANDLER(unknown_port_1_w),	// ??
	DEVCB_HANDLER(unknown_port_2_w)	// ??
};


/***************************************************************************

  Input Port(s)

***************************************************************************/

static INPUT_PORTS_START( chaknpop )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )	// LEFT COIN
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )	// RIGHT COIN
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_TILT )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSWC")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Language ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Japanese ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Super Chack'n" )
	PORT_DIPSETTING(    0x04, "pi" )
	PORT_DIPSETTING(    0x00, "1st Chance" )
	PORT_DIPNAME( 0x08, 0x08, "Endless (Cheat)")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Credit Info" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Show Year" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Infinite (Cheat)")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, "1 Way" )
	PORT_DIPSETTING(    0x80, "2 Way" )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "80k and every 100k" )
	PORT_DIPSETTING(    0x01, "60k and every 100k" )
	PORT_DIPSETTING(    0x02, "40k and every 100k" )
	PORT_DIPSETTING(    0x03, "20k and every 100k" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x08, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x18, "1" )
	PORT_DIPNAME( 0x20, 0x00, "Training/Difficulty" )
	PORT_DIPSETTING(    0x20, "Off/Every 10 Min." )
	PORT_DIPSETTING(    0x00, "On/Every 7 Min." )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("DSWA")
	PORT_DIPNAME(0x0f,  0x00, DEF_STR( Coin_A ) )
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
	PORT_DIPNAME(0xf0,  0x00, DEF_STR( Coin_B ) )
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
INPUT_PORTS_END


/***************************************************************************

  Machine Driver(s)

***************************************************************************/

static const gfx_layout spritelayout =
{
	16,16,	/* 16*16 characters */
	256,	/* 256 characters */
	2,	/* 2 bits per pixel */
	{ 0, 0x2000*8 },	/* the two bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7 ,
			8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8	/* every sprite takes 32 consecutive bytes */
};

static const gfx_layout charlayout =
{
	8,8,	/* 8*8 characters */
	1024,	/* 1024 characters */
	2,	/* 2 bits per pixel */
	{ 0, 0x2000*8 },	/* the two bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8	/* every char takes 8 consecutive bytes */
};

static GFXDECODE_START( chaknpop )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout, 0,  8 )
	GFXDECODE_ENTRY( "gfx2", 0, charlayout,   32, 8 )
GFXDECODE_END


static MACHINE_START( chaknpop )
{
	chaknpop_state *state = (chaknpop_state *)machine->driver_data;
	UINT8 *ROM = memory_region(machine, "maincpu");

	memory_configure_bank(machine, "bank1", 0, 2, &ROM[0x10000], 0x4000);

	state_save_register_global(machine, state->gfxmode);
	state_save_register_global(machine, state->flip_x);
	state_save_register_global(machine, state->flip_y);

	state_save_register_global(machine, state->mcu_seed);
	state_save_register_global(machine, state->mcu_result);
	state_save_register_global(machine, state->mcu_select);
}

static MACHINE_RESET( chaknpop )
{
	chaknpop_state *state = (chaknpop_state *)machine->driver_data;

	state->gfxmode = 0;
	state->flip_x = 0;
	state->flip_y = 0;

	state->mcu_seed = MCU_INITIAL_SEED;
	state->mcu_result = 0;
	state->mcu_select = 0;
}

static MACHINE_DRIVER_START( chaknpop )

	/* driver data */
	MDRV_DRIVER_DATA(chaknpop_state)

	/* basic machine hardware */
	/* the real board is 3.072MHz, but it is faster for MAME */
	//MDRV_CPU_ADD("maincpu", Z80, 18432000 / 6)   /* 3.072 MHz */
	MDRV_CPU_ADD("maincpu", Z80, 2350000)
	//MDRV_CPU_ADD("maincpu", Z80, 2760000)
	MDRV_CPU_PROGRAM_MAP(chaknpop_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_MACHINE_START(chaknpop)
	MDRV_MACHINE_RESET(chaknpop)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60.606060)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(chaknpop)
	MDRV_PALETTE_LENGTH(1024)

	MDRV_PALETTE_INIT(chaknpop)
	MDRV_VIDEO_START(chaknpop)
	MDRV_VIDEO_UPDATE(chaknpop)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ay1", AY8910, 18432000 / 12)
	MDRV_SOUND_CONFIG(ay8910_interface_1)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)

	MDRV_SOUND_ADD("ay2", AY8910, 18432000 / 12)
	MDRV_SOUND_CONFIG(ay8910_interface_2)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)
MACHINE_DRIVER_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( chaknpop )
	ROM_REGION( 0x18000, "maincpu", 0 )			/* Main CPU */
	ROM_LOAD( "a04-01.28",    0x00000, 0x2000, CRC(386fe1c8) SHA1(cca24abfb8a7f439251e7936036475c694002561) )
	ROM_LOAD( "a04-02.27",    0x02000, 0x2000, CRC(5562a6a7) SHA1(0c5d81f9aaf858f88007a6bca7f83dc3ef59c5b5) )
	ROM_LOAD( "a04-03.26",    0x04000, 0x2000, CRC(3e2f0a9c) SHA1(f1cf87a4cb07f77104d4a4d369807dac522e052c) )
	ROM_LOAD( "a04-04.25",    0x06000, 0x2000, CRC(5209c7d4) SHA1(dcba785a697df55d84d65735de38365869a1da9d) )
	ROM_LOAD( "a04-05.3",     0x0a000, 0x2000, CRC(8720e024) SHA1(99e445c117d1501a245f9eb8d014abc4712b4963) )

	ROM_REGION( 0x0800, "cpu1", 0 )	/* 2k for the microcontroller */
	/* MCU isn't dumped (its protected) however we simulate it using data
       extracted with a trojan, see machine/chaknpop.c */
	ROM_LOAD( "68705.mcu",   0x0000, 0x0800, NO_DUMP )

	ROM_REGION( 0x4000, "gfx1", 0 )	/* Sprite */
	ROM_LOAD( "a04-08.14",     0x0000, 0x2000, CRC(5575a021) SHA1(c2fad53fe6a12c19cec69d27c13fce6aea2502f2) )
	ROM_LOAD( "a04-07.15",     0x2000, 0x2000, CRC(ae687c18) SHA1(65b25263da88d30cbc0dad94511869596e5c975a) )

	ROM_REGION( 0x4000, "gfx2", 0 )	/* Text */
	ROM_LOAD( "a04-09.98",     0x0000, 0x2000, CRC(757a723a) SHA1(62ab84d2aaa9bc1ea5aa9df8155aa3b5a1e93889) )
	ROM_LOAD( "a04-10.97",     0x2000, 0x2000, CRC(3e3fd608) SHA1(053a8fbdb35bf1c142349f78a63e8cd1adb41ef6) )

	ROM_REGION( 0x0800, "proms", 0 )			/* Palette */
	ROM_LOAD( "a04-11.bin",    0x0000, 0x0400, CRC(9bf0e85f) SHA1(44f0a4712c99a715dec54060afb0b27dc48998b4) )
	ROM_LOAD( "a04-12.bin",    0x0400, 0x0400, CRC(954ce8fc) SHA1(e187f9e2cb754264d149c2896ca949dea3bcf2eb) )
ROM_END


/*  ( YEAR  NAME      PARENT    MACHINE   INPUT     INIT      MONITOR  COMPANY              FULLNAME ) */
GAME( 1983, chaknpop, 0,        chaknpop, chaknpop, 0,        ROT0,    "Taito Corporation", "Chack'n Pop", 0 )
