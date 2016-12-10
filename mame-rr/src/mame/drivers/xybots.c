/***************************************************************************

    Atari Xybots hardware

    driver by Aaron Giles

    Games supported:
        * Xybots (1987) [5 sets]

    Known bugs:
        * none at this time

****************************************************************************

    Memory map (TBA)

***************************************************************************/


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/atarigen.h"
#include "audio/atarijsa.h"
#include "includes/xybots.h"



/*************************************
 *
 *  Initialization & interrupts
 *
 *************************************/

static void update_interrupts(running_machine *machine)
{
	xybots_state *state = (xybots_state *)machine->driver_data;
	cputag_set_input_line(machine, "maincpu", 1, state->atarigen.video_int_state ? ASSERT_LINE : CLEAR_LINE);
	cputag_set_input_line(machine, "maincpu", 2, state->atarigen.sound_int_state ? ASSERT_LINE : CLEAR_LINE);
}


static MACHINE_START( xybots )
{
	atarigen_init(machine);
}


static MACHINE_RESET( xybots )
{
	xybots_state *state = (xybots_state *)machine->driver_data;

	atarigen_eeprom_reset(&state->atarigen);
	atarigen_slapstic_reset(&state->atarigen);
	atarigen_interrupt_reset(&state->atarigen, update_interrupts);
	atarijsa_reset();
}



/*************************************
 *
 *  I/O handlers
 *
 *************************************/

static READ16_HANDLER( special_port1_r )
{
	xybots_state *state = (xybots_state *)space->machine->driver_data;
	int result = input_port_read(space->machine, "FFE200");

	if (state->atarigen.cpu_to_sound_ready) result ^= 0x0200;
	result ^= state->h256 ^= 0x0400;
	return result;
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

/* full map verified from schematics */
static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 16 )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x007fff) AM_MIRROR(0x7c0000) AM_ROM
	AM_RANGE(0x008000, 0x00ffff) AM_MIRROR(0x7c0000) AM_ROM	/* slapstic maps here */
	AM_RANGE(0x010000, 0x03ffff) AM_MIRROR(0x7c0000) AM_ROM
	AM_RANGE(0xff8000, 0xff8fff) AM_MIRROR(0x7f8000) AM_RAM_WRITE(atarigen_alpha_w) AM_BASE_MEMBER(xybots_state, atarigen.alpha)
	AM_RANGE(0xff9000, 0xffadff) AM_MIRROR(0x7f8000) AM_RAM
	AM_RANGE(0xffae00, 0xffafff) AM_MIRROR(0x7f8000) AM_RAM_WRITE(atarimo_0_spriteram_w) AM_BASE(&atarimo_0_spriteram)
	AM_RANGE(0xffb000, 0xffbfff) AM_MIRROR(0x7f8000) AM_RAM_WRITE(atarigen_playfield_w) AM_BASE_MEMBER(xybots_state, atarigen.playfield)
	AM_RANGE(0xffc000, 0xffc7ff) AM_MIRROR(0x7f8800) AM_RAM_WRITE(paletteram16_IIIIRRRRGGGGBBBB_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0xffd000, 0xffdfff) AM_MIRROR(0x7f8000) AM_READWRITE(atarigen_eeprom_r, atarigen_eeprom_w) AM_BASE_SIZE_MEMBER(xybots_state, atarigen.eeprom, atarigen.eeprom_size)
	AM_RANGE(0xffe000, 0xffe0ff) AM_MIRROR(0x7f8000) AM_READ(atarigen_sound_r)
	AM_RANGE(0xffe100, 0xffe1ff) AM_MIRROR(0x7f8000) AM_READ_PORT("FFE100")
	AM_RANGE(0xffe200, 0xffe2ff) AM_MIRROR(0x7f8000) AM_READ(special_port1_r)
	AM_RANGE(0xffe800, 0xffe8ff) AM_MIRROR(0x7f8000) AM_WRITE(atarigen_eeprom_enable_w)
	AM_RANGE(0xffe900, 0xffe9ff) AM_MIRROR(0x7f8000) AM_WRITE(atarigen_sound_w)
	AM_RANGE(0xffea00, 0xffeaff) AM_MIRROR(0x7f8000) AM_WRITE(watchdog_reset16_w)
	AM_RANGE(0xffeb00, 0xffebff) AM_MIRROR(0x7f8000) AM_WRITE(atarigen_video_int_ack_w)
	AM_RANGE(0xffee00, 0xffeeff) AM_MIRROR(0x7f8000) AM_WRITE(atarigen_sound_reset_w)
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( xybots )
	PORT_START("FFE100")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P2 Twist Right") PORT_CODE(KEYCODE_W) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P2 Twist Left") PORT_CODE(KEYCODE_Q) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 Twist Right") PORT_CODE(KEYCODE_X) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Twist Left") PORT_CODE(KEYCODE_Z) PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)

	PORT_START("FFE200")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE( 0x0100, IP_ACTIVE_LOW )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNUSED )	/* /AUDBUSY */
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNUSED )	/* 256H */
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_VBLANK )	/* VBLANK */
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_INCLUDE( atarijsa_i )		/* audio port */
	/* Xybots uses a swapped version */
	PORT_MODIFY("JSAI")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout anlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ 0, 1, 2, 3, 8, 9, 10, 11 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	8*16
};


static const gfx_layout pfmolayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0, 4, 8, 12, 16, 20, 24, 28 },
	{ 0*8, 4*8, 8*8, 12*8, 16*8, 20*8, 24*8, 28*8 },
	32*8
};


static GFXDECODE_START( xybots )
	GFXDECODE_ENTRY( "gfx1", 0, pfmolayout,    512, 16 )		/* playfield */
	GFXDECODE_ENTRY( "gfx2", 0, pfmolayout,    256, 48 )		/* sprites */
	GFXDECODE_ENTRY( "gfx3", 0, anlayout,        0, 64 )		/* characters 8x8 */
GFXDECODE_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_DRIVER_START( xybots )
	MDRV_DRIVER_DATA(xybots_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, ATARI_CLOCK_14MHz/2)
	MDRV_CPU_PROGRAM_MAP(main_map)
	MDRV_CPU_VBLANK_INT("screen", atarigen_video_int_gen)

	MDRV_MACHINE_START(xybots)
	MDRV_MACHINE_RESET(xybots)
	MDRV_NVRAM_HANDLER(atarigen)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MDRV_GFXDECODE(xybots)
	MDRV_PALETTE_LENGTH(1024)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	/* note: these parameters are from published specs, not derived */
	/* the board uses a SYNGEN chip to generate video signals */
	MDRV_SCREEN_RAW_PARAMS(ATARI_CLOCK_14MHz/2, 456, 0, 336, 262, 0, 240)

	MDRV_VIDEO_START(xybots)
	MDRV_VIDEO_UPDATE(xybots)

	/* sound hardware */
	MDRV_IMPORT_FROM(jsa_i_stereo_swapped)
MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( xybots )
	ROM_REGION( 0x90000, "maincpu", 0 )	/* 8*64k for 68000 code */
	ROM_LOAD16_BYTE( "136054-2112.17cd", 0x000000, 0x010000, CRC(16d64748) SHA1(3c2ba8ec3185b69c4e1947ac842f2250ee35216e) )
	ROM_LOAD16_BYTE( "136054-2113.19cd", 0x000001, 0x010000, CRC(2677d44a) SHA1(23a3538df13a47f2fd78d4842b9f8b81e38c802e) )
	ROM_LOAD16_BYTE( "136054-2114.17b", 0x020000, 0x008000, CRC(d31890cb) SHA1(b58722a4dcc79e97484c2f5e35b8dbf8c3520bd9) )
	ROM_LOAD16_BYTE( "136054-2115.19b", 0x020001, 0x008000, CRC(750ab1b0) SHA1(0638de738bd804bde4b93cd23190ee0465887cf8) )

	ROM_REGION( 0x14000, "jsa", 0 )	/* 64k for 6502 code */
	ROM_LOAD( "136054-1116.2k",  0x010000, 0x004000, CRC(3b9f155d) SHA1(7080681a7eab282023034379825ca88adc6b300f) )
	ROM_CONTINUE(                0x004000, 0x00c000 )

	ROM_REGION( 0x40000, "gfx1", 0 )
	ROM_LOAD( "136054-2102.12l", 0x000000, 0x008000, CRC(c1309674) SHA1(5a163c894142c8d662557c8322dc04fded637227) )
	ROM_RELOAD(                  0x008000, 0x008000 )
	ROM_LOAD( "136054-2103.11l", 0x010000, 0x010000, CRC(907c024d) SHA1(d41c7471136f4a0632cbae28644ab1650af1467f) )
	ROM_LOAD( "136054-2117.8l",  0x030000, 0x010000, CRC(0cc9b42d) SHA1(a744d97d40afb469ee61c2fc8d4b04ff8cc72755) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "136054-1105.2e",  0x000000, 0x010000, CRC(315a4274) SHA1(9a6cfdd655560e5d0320f95c8b60e733991a0909) )
	ROM_LOAD( "136054-1106.2ef", 0x010000, 0x010000, CRC(3d8c1dd2) SHA1(dd61fc0b96c395e1e65bb7114a60b45d68d08140) )
	ROM_LOAD( "136054-1107.2f",  0x020000, 0x010000, CRC(b7217da5) SHA1(b00ff4a3d0cffb94636f84cd923a78b5a02f9741) )
	ROM_LOAD( "136054-1108.2fj", 0x030000, 0x010000, CRC(77ac65e1) SHA1(85a458adbc1a1c62dbd799f61e8f9f7f8811e06d) )
	ROM_LOAD( "136054-1109.2jk", 0x040000, 0x010000, CRC(1b482c53) SHA1(50f463f00b7fad91c61bfeeb56bf76e120d24129) )
	ROM_LOAD( "136054-1110.2k",  0x050000, 0x010000, CRC(99665ff4) SHA1(e93a85a601ae364d1e773174d488fca74b8d5753) )
	ROM_LOAD( "136054-1111.2l",  0x060000, 0x010000, CRC(416107ee) SHA1(cdfe6c6bd8efaa08506cd5707887c552500c2108) )

	ROM_REGION( 0x02000, "gfx3", 0 )
	ROM_LOAD( "136054-1101.5c",  0x000000, 0x002000, CRC(59c028a2) SHA1(27dcde0da88f949a5e4a7632d4b403b937c8c6e0) )
ROM_END


ROM_START( xybotsg )
	ROM_REGION( 0x90000, "maincpu", 0 )	/* 8*64k for 68000 code */
	ROM_LOAD16_BYTE( "136054-3212.17cd", 0x000000, 0x010000, CRC(4cac5d7c) SHA1(79cdd754fb6055249dace31fe9f8939f13aae8ca) )
	ROM_LOAD16_BYTE( "136054-3213.19cd", 0x000001, 0x010000, CRC(bfcb0b00) SHA1(3e45f72051ea74b544c8578c6fc1284f925caa3d) )
	ROM_LOAD16_BYTE( "136054-3214.17b", 0x020000, 0x008000, CRC(4ad35093) SHA1(6d2d82fb481c68819ec6c87d483eed17d4ae5d1a) )
	ROM_LOAD16_BYTE( "136054-3215.19b", 0x020001, 0x008000, CRC(3a2afbaf) SHA1(61b88d15d95681eb24559d0696203cd4ee63d11f) )

	ROM_REGION( 0x14000, "jsa", 0 )	/* 64k for 6502 code */
	ROM_LOAD( "136054-1116.2k",  0x010000, 0x004000, CRC(3b9f155d) SHA1(7080681a7eab282023034379825ca88adc6b300f) )
	ROM_CONTINUE(                0x004000, 0x00c000 )

	ROM_REGION( 0x40000, "gfx1", 0 )
	ROM_LOAD( "136054-2102.12l", 0x000000, 0x008000, CRC(c1309674) SHA1(5a163c894142c8d662557c8322dc04fded637227) )
	ROM_RELOAD(                  0x008000, 0x008000 )
	ROM_LOAD( "136054-2103.11l", 0x010000, 0x010000, CRC(907c024d) SHA1(d41c7471136f4a0632cbae28644ab1650af1467f) )
	ROM_LOAD( "136054-2117.8l",  0x030000, 0x010000, CRC(0cc9b42d) SHA1(a744d97d40afb469ee61c2fc8d4b04ff8cc72755) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "136054-1105.2e",  0x000000, 0x010000, CRC(315a4274) SHA1(9a6cfdd655560e5d0320f95c8b60e733991a0909) )
	ROM_LOAD( "136054-1106.2ef", 0x010000, 0x010000, CRC(3d8c1dd2) SHA1(dd61fc0b96c395e1e65bb7114a60b45d68d08140) )
	ROM_LOAD( "136054-1107.2f",  0x020000, 0x010000, CRC(b7217da5) SHA1(b00ff4a3d0cffb94636f84cd923a78b5a02f9741) )
	ROM_LOAD( "136054-1108.2fj", 0x030000, 0x010000, CRC(77ac65e1) SHA1(85a458adbc1a1c62dbd799f61e8f9f7f8811e06d) )
	ROM_LOAD( "136054-1109.2jk", 0x040000, 0x010000, CRC(1b482c53) SHA1(50f463f00b7fad91c61bfeeb56bf76e120d24129) )
	ROM_LOAD( "136054-1110.2k",  0x050000, 0x010000, CRC(99665ff4) SHA1(e93a85a601ae364d1e773174d488fca74b8d5753) )
	ROM_LOAD( "136054-1111.2l",  0x060000, 0x010000, CRC(416107ee) SHA1(cdfe6c6bd8efaa08506cd5707887c552500c2108) )

	ROM_REGION( 0x02000, "gfx3", 0 )
	ROM_LOAD( "136054-1101.5c",  0x000000, 0x002000, CRC(59c028a2) SHA1(27dcde0da88f949a5e4a7632d4b403b937c8c6e0) )
ROM_END


ROM_START( xybotsf )
	ROM_REGION( 0x90000, "maincpu", 0 )	/* 8*64k for 68000 code */
	ROM_LOAD16_BYTE( "136054-3612.17cd", 0x000000, 0x010000, CRC(b03a3f3c) SHA1(c88ad0ba5381562095f5b5a13d338d10fa0597f5) )
	ROM_LOAD16_BYTE( "136054-3613.19cd", 0x000001, 0x010000, CRC(ab33eb1f) SHA1(926c32f07c0bcc5832db3a1adf0357e55cae707a) )
	ROM_LOAD16_BYTE( "136054-3614.17b", 0x020000, 0x008000, CRC(7385e0b6) SHA1(98a69901069872b14413c1bfe48783fdb43c1c37) )
	ROM_LOAD16_BYTE( "136054-3615.19b", 0x020001, 0x008000, CRC(8e37b812) SHA1(40f973a49c4b40f3a5d982d332995e792f718dcc) )

	ROM_REGION( 0x14000, "jsa", 0 )	/* 64k for 6502 code */
	ROM_LOAD( "136054-1116.2k",  0x010000, 0x004000, CRC(3b9f155d) SHA1(7080681a7eab282023034379825ca88adc6b300f) )
	ROM_CONTINUE(                0x004000, 0x00c000 )

	ROM_REGION( 0x40000, "gfx1", 0 )
	ROM_LOAD( "136054-2102.12l", 0x000000, 0x008000, CRC(c1309674) SHA1(5a163c894142c8d662557c8322dc04fded637227) )
	ROM_RELOAD(                  0x008000, 0x008000 )
	ROM_LOAD( "136054-2103.11l", 0x010000, 0x010000, CRC(907c024d) SHA1(d41c7471136f4a0632cbae28644ab1650af1467f) )
	ROM_LOAD( "136054-2117.8l",  0x030000, 0x010000, CRC(0cc9b42d) SHA1(a744d97d40afb469ee61c2fc8d4b04ff8cc72755) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "136054-1105.2e",  0x000000, 0x010000, CRC(315a4274) SHA1(9a6cfdd655560e5d0320f95c8b60e733991a0909) )
	ROM_LOAD( "136054-1106.2ef", 0x010000, 0x010000, CRC(3d8c1dd2) SHA1(dd61fc0b96c395e1e65bb7114a60b45d68d08140) )
	ROM_LOAD( "136054-1107.2f",  0x020000, 0x010000, CRC(b7217da5) SHA1(b00ff4a3d0cffb94636f84cd923a78b5a02f9741) )
	ROM_LOAD( "136054-1108.2fj", 0x030000, 0x010000, CRC(77ac65e1) SHA1(85a458adbc1a1c62dbd799f61e8f9f7f8811e06d) )
	ROM_LOAD( "136054-1109.2jk", 0x040000, 0x010000, CRC(1b482c53) SHA1(50f463f00b7fad91c61bfeeb56bf76e120d24129) )
	ROM_LOAD( "136054-1110.2k",  0x050000, 0x010000, CRC(99665ff4) SHA1(e93a85a601ae364d1e773174d488fca74b8d5753) )
	ROM_LOAD( "136054-1111.2l",  0x060000, 0x010000, CRC(416107ee) SHA1(cdfe6c6bd8efaa08506cd5707887c552500c2108) )

	ROM_REGION( 0x02000, "gfx3", 0 )
	ROM_LOAD( "136054-1101.5c",  0x000000, 0x002000, CRC(59c028a2) SHA1(27dcde0da88f949a5e4a7632d4b403b937c8c6e0) )
ROM_END


ROM_START( xybots1 )
	ROM_REGION( 0x90000, "maincpu", 0 )	/* 8*64k for 68000 code */
	ROM_LOAD16_BYTE( "136054-1112.17cd", 0x000000, 0x010000, CRC(2dbab363) SHA1(1473bf1246c6fb3e6b8b1f86a345b532ccf18e8d) )
	ROM_LOAD16_BYTE( "136054-1113.19cd", 0x000001, 0x010000, CRC(847b056e) SHA1(cc4b90f19d7eaee09569ba228c2654f64cec3200) )
	ROM_LOAD16_BYTE( "136054-1114.17b", 0x020000, 0x008000, CRC(7444f88f) SHA1(e2a27754a57a809398ee639fe5d0920b564d4c0b) )
	ROM_LOAD16_BYTE( "136054-1115.19b", 0x020001, 0x008000, CRC(848d072d) SHA1(c4d1181f0227200e60d99a99c1a83897275b055f) )

	ROM_REGION( 0x14000, "jsa", 0 )	/* 64k for 6502 code */
	ROM_LOAD( "136054-1116.2k",  0x010000, 0x004000, CRC(3b9f155d) SHA1(7080681a7eab282023034379825ca88adc6b300f) )
	ROM_CONTINUE(                0x004000, 0x00c000 )

	ROM_REGION( 0x40000, "gfx1", 0 )
	ROM_LOAD( "136054-2102.12l", 0x000000, 0x008000, CRC(c1309674) SHA1(5a163c894142c8d662557c8322dc04fded637227) )
	ROM_RELOAD(               0x008000, 0x008000 )
	ROM_LOAD( "136054-2103.11l", 0x010000, 0x010000, CRC(907c024d) SHA1(d41c7471136f4a0632cbae28644ab1650af1467f) )
	ROM_LOAD( "136054-2117.8l",  0x030000, 0x010000, CRC(0cc9b42d) SHA1(a744d97d40afb469ee61c2fc8d4b04ff8cc72755) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "136054-1105.2e",  0x000000, 0x010000, CRC(315a4274) SHA1(9a6cfdd655560e5d0320f95c8b60e733991a0909) )
	ROM_LOAD( "136054-1106.2ef", 0x010000, 0x010000, CRC(3d8c1dd2) SHA1(dd61fc0b96c395e1e65bb7114a60b45d68d08140) )
	ROM_LOAD( "136054-1107.2f",  0x020000, 0x010000, CRC(b7217da5) SHA1(b00ff4a3d0cffb94636f84cd923a78b5a02f9741) )
	ROM_LOAD( "136054-1108.2fj", 0x030000, 0x010000, CRC(77ac65e1) SHA1(85a458adbc1a1c62dbd799f61e8f9f7f8811e06d) )
	ROM_LOAD( "136054-1109.2jk", 0x040000, 0x010000, CRC(1b482c53) SHA1(50f463f00b7fad91c61bfeeb56bf76e120d24129) )
	ROM_LOAD( "136054-1110.2k",  0x050000, 0x010000, CRC(99665ff4) SHA1(e93a85a601ae364d1e773174d488fca74b8d5753) )
	ROM_LOAD( "136054-1111.2l",  0x060000, 0x010000, CRC(416107ee) SHA1(cdfe6c6bd8efaa08506cd5707887c552500c2108) )

	ROM_REGION( 0x02000, "gfx3", 0 )
	ROM_LOAD( "136054-1101.5c",  0x000000, 0x002000, CRC(59c028a2) SHA1(27dcde0da88f949a5e4a7632d4b403b937c8c6e0) )
ROM_END


ROM_START( xybots0 )
	ROM_REGION( 0x90000, "maincpu", 0 )	/* 8*64k for 68000 code */
	ROM_LOAD16_BYTE( "136054-0112.17cd", 0x000000, 0x010000, CRC(4b830ac4) SHA1(1f6dc0c6648f74c4775b52e3f502e835a8741182) )
	ROM_LOAD16_BYTE( "136054-0113.19cd", 0x000001, 0x010000, CRC(dcfbf8a7) SHA1(0106cd7be55147f4b59e17391e5bb339aaf80535) )
	ROM_LOAD16_BYTE( "136054-0114.17b", 0x020000, 0x008000, CRC(18b875f7) SHA1(aa78553bd3556d0b209513ba80b782cfb0e3bb8b) )
	ROM_LOAD16_BYTE( "136054-0115.19b", 0x020001, 0x008000, CRC(7f116360) SHA1(d12c339ce973bd74be4a4ac9e9d293f6a6e358d6) )

	ROM_REGION( 0x14000, "jsa", 0 )	/* 64k for 6502 code */
	ROM_LOAD( "136054-0116.2k",  0x010000, 0x004000, NO_DUMP CRC(3b9f155d) SHA1(7080681a7eab282023034379825ca88adc6b300f) )
	ROM_CONTINUE(                0x004000, 0x00c000 )

	ROM_REGION( 0x40000, "gfx1", 0 )
	ROM_LOAD( "136054-1102.12l", 0x000000, 0x008000, CRC(0d304e5b) SHA1(203c86c865667b1538f61c0950682fb17ebd9abb) )
	ROM_RELOAD(                  0x008000, 0x008000 )
	ROM_LOAD( "136054-1103.11l", 0x010000, 0x010000, CRC(a514da1d) SHA1(5af3c703e0c8e8d47123241ce39f202c88a8cdb0) )
	ROM_LOAD( "136054-1117.8l",  0x030000, 0x010000, CRC(6b79154d) SHA1(6fd47503c91a23f75046acd1ef8000b63f8e8ba6) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "136054-1105.2e",  0x000000, 0x010000, CRC(315a4274) SHA1(9a6cfdd655560e5d0320f95c8b60e733991a0909) )
	ROM_LOAD( "136054-1106.2ef", 0x010000, 0x010000, CRC(3d8c1dd2) SHA1(dd61fc0b96c395e1e65bb7114a60b45d68d08140) )
	ROM_LOAD( "136054-1107.2f",  0x020000, 0x010000, CRC(b7217da5) SHA1(b00ff4a3d0cffb94636f84cd923a78b5a02f9741) )
	ROM_LOAD( "136054-1108.2fj", 0x030000, 0x010000, CRC(77ac65e1) SHA1(85a458adbc1a1c62dbd799f61e8f9f7f8811e06d) )
	ROM_LOAD( "136054-1109.2jk", 0x040000, 0x010000, CRC(1b482c53) SHA1(50f463f00b7fad91c61bfeeb56bf76e120d24129) )
	ROM_LOAD( "136054-1110.2k",  0x050000, 0x010000, CRC(99665ff4) SHA1(e93a85a601ae364d1e773174d488fca74b8d5753) )
	ROM_LOAD( "136054-1111.2l",  0x060000, 0x010000, CRC(416107ee) SHA1(cdfe6c6bd8efaa08506cd5707887c552500c2108) )

	ROM_REGION( 0x02000, "gfx3", 0 )
	ROM_LOAD( "136054-1101.5c",  0x000000, 0x002000, CRC(59c028a2) SHA1(27dcde0da88f949a5e4a7632d4b403b937c8c6e0) )
ROM_END



/*************************************
 *
 *  Driver initialization
 *
 *************************************/

static DRIVER_INIT( xybots )
{
	xybots_state *state = (xybots_state *)machine->driver_data;
	state->h256 = 0x0400;
	atarigen_slapstic_init(machine->device("maincpu"), 0x008000, 0, 107);
	atarijsa_init(machine, "FFE200", 0x0100);
}



/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1987, xybots,  0,      xybots, xybots, xybots, ROT0, "Atari Games", "Xybots (rev 2)", 0 )
GAME( 1987, xybotsg, xybots, xybots, xybots, xybots, ROT0, "Atari Games", "Xybots (German, rev 3)", 0 )
GAME( 1987, xybotsf, xybots, xybots, xybots, xybots, ROT0, "Atari Games", "Xybots (French, rev 3)", 0 )
GAME( 1987, xybots1, xybots, xybots, xybots, xybots, ROT0, "Atari Games", "Xybots (rev 1)", 0 )
GAME( 1987, xybots0, xybots, xybots, xybots, xybots, ROT0, "Atari Games", "Xybots (rev 0)", 0 )
