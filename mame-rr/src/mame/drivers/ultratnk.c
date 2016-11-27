/***************************************************************************

Atari Ultra Tank driver

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "includes/ultratnk.h"
#include "audio/sprint4.h"

#define MASTER_CLOCK    12096000

#define HTOTAL 384
#define VTOTAL 262

#define PIXEL_CLOCK    (MASTER_CLOCK / 2)


static int da_latch;



static CUSTOM_INPUT( get_collision )
{
	return ultratnk_collision[(FPTR) param];
}


static CUSTOM_INPUT( get_joystick )
{
	UINT8 joy = input_port_read(field->port->machine, (const char *)param) & 3;

	if (joy == 1)
	{
		return (da_latch < 8) ? 1 : 0;
	}
	if (joy == 2)
	{
		return 0;
	}

	return 1;
}


static TIMER_CALLBACK( nmi_callback	)
{
	int scanline = param + 64;

	if (scanline >= VTOTAL)
		scanline = 32;

	/* NMI and watchdog are disabled during service mode */

	watchdog_enable(machine, input_port_read(machine, "IN0") & 0x40);

	if (input_port_read(machine, "IN0") & 0x40)
		cputag_set_input_line(machine, "maincpu", INPUT_LINE_NMI, PULSE_LINE);

	timer_set(machine, machine->primary_screen->time_until_pos(scanline), NULL, scanline, nmi_callback);
}


static MACHINE_RESET( ultratnk )
{
	timer_set(machine, machine->primary_screen->time_until_pos(32), NULL, 32, nmi_callback);
}


static READ8_HANDLER( ultratnk_wram_r )
{
	return space->machine->generic.videoram.u8[0x380 + offset];
}


static READ8_HANDLER( ultratnk_analog_r )
{
	return (input_port_read(space->machine, "ANALOG") << (~offset & 7)) & 0x80;
}
static READ8_HANDLER( ultratnk_coin_r )
{
	return (input_port_read(space->machine, "COIN") << (~offset & 7)) & 0x80;
}
static READ8_HANDLER( ultratnk_collision_r )
{
	return (input_port_read(space->machine, "COLLISION") << (~offset & 7)) & 0x80;
}


static READ8_HANDLER( ultratnk_options_r )
{
	return (input_port_read(space->machine, "DIP") >> (2 * (offset & 3))) & 3;
}


static WRITE8_HANDLER( ultratnk_wram_w )
{
	space->machine->generic.videoram.u8[0x380 + offset] = data;
}


static WRITE8_HANDLER( ultratnk_collision_reset_w )
{
	ultratnk_collision[(offset >> 1) & 3] = 0;
}


static WRITE8_HANDLER( ultratnk_da_latch_w )
{
	da_latch = data & 15;
}


static WRITE8_HANDLER( ultratnk_led_1_w )
{
	set_led_status(space->machine, 0, offset & 1); /* left player start */
}
static WRITE8_HANDLER( ultratnk_led_2_w )
{
	set_led_status(space->machine, 1, offset & 1); /* right player start */
}


static WRITE8_HANDLER( ultratnk_lockout_w )
{
	coin_lockout_global_w(space->machine, ~offset & 1);
}


static WRITE8_DEVICE_HANDLER( ultratnk_fire_1_w )
{
	discrete_sound_w(device, ULTRATNK_FIRE_EN_1, offset & 1);
}
static WRITE8_DEVICE_HANDLER( ultratnk_fire_2_w )
{
	discrete_sound_w(device, ULTRATNK_FIRE_EN_2, offset & 1);
}
static WRITE8_DEVICE_HANDLER( ultratnk_attract_w )
{
	discrete_sound_w(device, ULTRATNK_ATTRACT_EN, data & 1);
}
static WRITE8_DEVICE_HANDLER( ultratnk_explosion_w )
{
	discrete_sound_w(device, ULTRATNK_EXPLOSION_DATA, data & 15);
}


static ADDRESS_MAP_START( ultratnk_cpu_map, ADDRESS_SPACE_PROGRAM, 8 )

	ADDRESS_MAP_GLOBAL_MASK(0x3fff)

	AM_RANGE(0x0000, 0x007f) AM_MIRROR(0x700) AM_RAM
	AM_RANGE(0x0080, 0x00ff) AM_MIRROR(0x700) AM_READWRITE(ultratnk_wram_r, ultratnk_wram_w)
	AM_RANGE(0x0800, 0x0bff) AM_MIRROR(0x400) AM_RAM_WRITE(ultratnk_video_ram_w) AM_BASE_GENERIC(videoram)

	AM_RANGE(0x1000, 0x17ff) AM_READ_PORT("IN0")
	AM_RANGE(0x1800, 0x1fff) AM_READ_PORT("IN1")

	AM_RANGE(0x2000, 0x2007) AM_MIRROR(0x718) AM_READ(ultratnk_analog_r)
	AM_RANGE(0x2020, 0x2027) AM_MIRROR(0x718) AM_READ(ultratnk_coin_r)
	AM_RANGE(0x2040, 0x2047) AM_MIRROR(0x718) AM_READ(ultratnk_collision_r)
	AM_RANGE(0x2060, 0x2063) AM_MIRROR(0x71c) AM_READ(ultratnk_options_r)

	AM_RANGE(0x2000, 0x2000) AM_MIRROR(0x71f) AM_DEVWRITE("discrete", ultratnk_attract_w)
	AM_RANGE(0x2020, 0x2027) AM_MIRROR(0x718) AM_WRITE(ultratnk_collision_reset_w)
	AM_RANGE(0x2040, 0x2041) AM_MIRROR(0x718) AM_WRITE(ultratnk_da_latch_w)
	AM_RANGE(0x2042, 0x2043) AM_MIRROR(0x718) AM_DEVWRITE("discrete", ultratnk_explosion_w)
	AM_RANGE(0x2044, 0x2045) AM_MIRROR(0x718) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x2066, 0x2067) AM_MIRROR(0x710) AM_WRITE(ultratnk_lockout_w)
	AM_RANGE(0x2068, 0x2069) AM_MIRROR(0x710) AM_WRITE(ultratnk_led_1_w)
	AM_RANGE(0x206a, 0x206b) AM_MIRROR(0x710) AM_WRITE(ultratnk_led_2_w)
	AM_RANGE(0x206c, 0x206d) AM_MIRROR(0x710) AM_DEVWRITE("discrete", ultratnk_fire_2_w)
	AM_RANGE(0x206e, 0x206f) AM_MIRROR(0x710) AM_DEVWRITE("discrete", ultratnk_fire_1_w)

	AM_RANGE(0x2800, 0x2fff) AM_NOP /* diagnostic ROM */
	AM_RANGE(0x3000, 0x3fff) AM_ROM

ADDRESS_MAP_END


static INPUT_PORTS_START( ultratnk )
	PORT_START("IN0")
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_VBLANK )

	PORT_START("IN1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Option 1") PORT_TOGGLE

	PORT_START("COLLISION")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM( get_collision, (void *)0 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM( get_collision, (void *)1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED ) /* VCC */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM( get_collision, (void *)2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_TILT )   /* SLAM */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM( get_collision, (void *)3 )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_NAME("Option 2") PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE3 ) PORT_NAME("Option 3") PORT_TOGGLE

	PORT_START("DIP")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("DIP:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("DIP:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Coinage ) ) PORT_DIPLOCATION("DIP:6,5")
	PORT_DIPSETTING(	0x0c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(	0x08, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(	0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x30, 0x10, "Game Length" ) PORT_DIPLOCATION("DIP:4,3")
	PORT_DIPSETTING(	0x00, "60 Seconds" )
	PORT_DIPSETTING(	0x10, "90 Seconds" )
	PORT_DIPSETTING(	0x20, "120 Seconds" )
	PORT_DIPSETTING(	0x30, "150 Seconds" )
	PORT_DIPNAME( 0xc0, 0x40, "Extended Play" ) PORT_DIPLOCATION("DIP:2,1")
	PORT_DIPSETTING(	0x40, "25 Points" )
	PORT_DIPSETTING(	0x80, "50 Points" )
	PORT_DIPSETTING(	0xc0, "75 Points" )
	PORT_DIPSETTING(	0x00, DEF_STR( None ) )

	PORT_START("ANALOG")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM( get_joystick, "JOY-W" )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM( get_joystick, "JOY-Y" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM( get_joystick, "JOY-X" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM( get_joystick, "JOY-Z" )

	PORT_START("JOY-W")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP ) PORT_PLAYER(1)

	PORT_START("JOY-X")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP ) PORT_PLAYER(1)

	PORT_START("JOY-Y")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP ) PORT_PLAYER(2)

	PORT_START("JOY-Z")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKRIGHT_UP ) PORT_PLAYER(2)

	PORT_START("MOTOR1")
	PORT_ADJUSTER( 35, "Motor 1 RPM" )

	PORT_START("MOTOR2")
	PORT_ADJUSTER( 40, "Motor 2 RPM" )

INPUT_PORTS_END


static const gfx_layout motion_layout =
{
	15, 16,
	RGN_FRAC(1,4),
	1,
	{ 0 },
	{
		7 + RGN_FRAC(0,4), 6 + RGN_FRAC(0,4), 5 + RGN_FRAC(0,4), 4 + RGN_FRAC(0,4),
		7 + RGN_FRAC(1,4), 6 + RGN_FRAC(1,4), 5 + RGN_FRAC(1,4), 4 + RGN_FRAC(1,4),
		7 + RGN_FRAC(2,4), 6 + RGN_FRAC(2,4), 5 + RGN_FRAC(2,4), 4 + RGN_FRAC(2,4),
		7 + RGN_FRAC(3,4), 6 + RGN_FRAC(3,4), 5 + RGN_FRAC(3,4)
	},
	{
		0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38,
		0x40, 0x48, 0x50, 0x58, 0x60, 0x68, 0x70, 0x78
	},
	0x80
};


static GFXDECODE_START( ultratnk )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x1, 0, 5 )
	GFXDECODE_ENTRY( "gfx2", 0, motion_layout, 0, 5 )
GFXDECODE_END


static MACHINE_DRIVER_START( ultratnk )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M6502, PIXEL_CLOCK / 8)
	MDRV_CPU_PROGRAM_MAP(ultratnk_cpu_map)

	MDRV_WATCHDOG_VBLANK_INIT(8)
	MDRV_MACHINE_RESET(ultratnk)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_RAW_PARAMS(PIXEL_CLOCK, HTOTAL, 0, 256, VTOTAL, 0, 224)

	MDRV_GFXDECODE(ultratnk)
	MDRV_PALETTE_LENGTH(10)

	MDRV_PALETTE_INIT(ultratnk)
	MDRV_VIDEO_START(ultratnk)
	MDRV_VIDEO_UPDATE(ultratnk)
	MDRV_VIDEO_EOF(ultratnk)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("discrete", DISCRETE, 0)
	MDRV_SOUND_CONFIG_DISCRETE(ultratnk)
	MDRV_SOUND_ROUTE(0, "mono", 1.0)

MACHINE_DRIVER_END


ROM_START( ultratnk )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD_NIB_LOW ( "030180.n1",	 0x3000, 0x0800, CRC(b6aa6056) SHA1(6de094017b5d87a238053fac88129d20260f8222) ) /* ROM 3 */
	ROM_LOAD_NIB_HIGH( "030181.k1",	 0x3000, 0x0800, CRC(17145c97) SHA1(afe0c9c562c27cd1fba57ea83377b0a4c12496db) ) /* ROM 3 */
	ROM_LOAD_NIB_LOW ( "030182.m1",	 0x3800, 0x0800, CRC(034366a2) SHA1(dc289ce4c79e9937977ca8804ce07b4c8e40e969) ) /* ROM 4 */
	ROM_LOAD_NIB_HIGH( "030183.l1",	 0x3800, 0x0800, CRC(be141602) SHA1(17aad9bab9bf6bd22dc3c2214b049bbd68c87380) ) /* ROM 4 */

	ROM_REGION( 0x0200, "gfx1", 0 ) /* playfield */
	ROM_LOAD_NIB_HIGH( "30172-01.j6", 0x0000, 0x0200, CRC(1d364b23) SHA1(44c5792ed3f33f40cd8632718b0e82152559ecdf) )
	ROM_LOAD_NIB_LOW ( "30173-01.h6", 0x0000, 0x0200, CRC(5c32f331) SHA1(c1d675891490fbc533eaa0da57545398d7325df8) )

	ROM_REGION( 0x1000, "gfx2", 0 ) /* motion */
	ROM_LOAD( "30174-01.n6", 0x0000, 0x0400, CRC(d0e20e73) SHA1(0df1ed4a73255032bb809fb4d0a4bf3f151c749d) )
	ROM_LOAD( "30175-01.m6", 0x0400, 0x0400, CRC(a47459c9) SHA1(4ca92edc172fbac923ba71731a25546c04ffc7b0) )
	ROM_LOAD( "30176-01.l6", 0x0800, 0x0400, CRC(1cc7c2dd) SHA1(7f8aebe8375751183afeae35ea2d241d22ee7a4f) )
	ROM_LOAD( "30177-01.k6", 0x0c00, 0x0400, CRC(3a91b09f) SHA1(1e713cb612eb7d78fc4a003e4e60308f62e0b169) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "30218-01.j10", 0x0000, 0x0020, CRC(d7a2c7b4) SHA1(7453921ecb6268b604dee3743f6e217db19c9871) )

	ROM_REGION( 0x0200, "user1", 0 )
	ROM_LOAD( "30024-01.p8", 0x0000, 0x0200, CRC(e71d2e22) SHA1(434c3a8237468604cce7feb40e6061d2670013b3) ) /* SYNC */
ROM_END


GAME( 1978, ultratnk, 0, ultratnk, ultratnk, 0, 0, "Atari", "Ultra Tank", 0 )
