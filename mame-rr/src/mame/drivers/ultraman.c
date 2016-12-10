/***************************************************************************

    Ultraman (c) 1991  Banpresto / Bandai

    Driver by Manuel Abadia <manu@teleline.es>

    2009-03:
    Added dsw locations and verified factory setting based on Guru's notes

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "video/konicdev.h"
#include "sound/2151intf.h"
#include "sound/okim6295.h"
#include "includes/ultraman.h"

static WRITE16_HANDLER( sound_cmd_w )
{
	if (ACCESSING_BITS_0_7)
		soundlatch_w(space, 0, data & 0xff);
}

static WRITE16_HANDLER( sound_irq_trigger_w )
{
	ultraman_state *state = (ultraman_state *)space->machine->driver_data;

	if (ACCESSING_BITS_0_7)
		cpu_set_input_line(state->audiocpu, INPUT_LINE_NMI, PULSE_LINE);
}


static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x080000, 0x08ffff) AM_RAM
	AM_RANGE(0x180000, 0x183fff) AM_RAM_WRITE(paletteram16_xRRRRRGGGGGBBBBB_word_w) AM_BASE_GENERIC(paletteram)/* Palette */
	AM_RANGE(0x1c0000, 0x1c0001) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x1c0002, 0x1c0003) AM_READ_PORT("P1")
	AM_RANGE(0x1c0004, 0x1c0005) AM_READ_PORT("P2")
	AM_RANGE(0x1c0006, 0x1c0007) AM_READ_PORT("DSW1")
	AM_RANGE(0x1c0008, 0x1c0009) AM_READ_PORT("DSW2")
	AM_RANGE(0x1c0018, 0x1c0019) AM_WRITE(ultraman_gfxctrl_w)	/* counters + gfx ctrl */
	AM_RANGE(0x1c0020, 0x1c0021) AM_WRITE(sound_cmd_w)
	AM_RANGE(0x1c0028, 0x1c0029) AM_WRITE(sound_irq_trigger_w)
	AM_RANGE(0x1c0030, 0x1c0031) AM_WRITE(watchdog_reset16_w)
	AM_RANGE(0x204000, 0x204fff) AM_DEVREADWRITE8("k051316_1", k051316_r, k051316_w, 0x00ff)	/* K051316 #0 RAM */
	AM_RANGE(0x205000, 0x205fff) AM_DEVREADWRITE8("k051316_2", k051316_r, k051316_w, 0x00ff)	/* K051316 #1 RAM */
	AM_RANGE(0x206000, 0x206fff) AM_DEVREADWRITE8("k051316_3", k051316_r, k051316_w, 0x00ff)	/* K051316 #2 RAM */
	AM_RANGE(0x207f80, 0x207f9f) AM_DEVWRITE8("k051316_1", k051316_ctrl_w, 0x00ff)	/* K051316 #0 registers */
	AM_RANGE(0x207fa0, 0x207fbf) AM_DEVWRITE8("k051316_2", k051316_ctrl_w, 0x00ff)	/* K051316 #1 registers */
	AM_RANGE(0x207fc0, 0x207fdf) AM_DEVWRITE8("k051316_3", k051316_ctrl_w, 0x00ff)	/* K051316 #2 registers */
	AM_RANGE(0x304000, 0x30400f) AM_DEVREADWRITE8("k051960", k051937_r, k051937_w, 0x00ff)		/* Sprite control */
	AM_RANGE(0x304800, 0x304fff) AM_DEVREADWRITE8("k051960", k051960_r, k051960_w, 0x00ff)		/* Sprite RAM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_RAM
	AM_RANGE(0xc000, 0xc000) AM_READ(soundlatch_r)	/* Sound latch read */
//  AM_RANGE(0xd000, 0xd000) AM_WRITENOP      /* ??? */
	AM_RANGE(0xe000, 0xe000) AM_DEVREADWRITE("oki", okim6295_r, okim6295_w)		/* M6295 */
	AM_RANGE(0xf000, 0xf001) AM_DEVREADWRITE("ymsnd", ym2151_r, ym2151_w)	/* YM2151 */
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
//  AM_RANGE(0x00, 0x00) AM_WRITENOP                     /* ??? */
ADDRESS_MAP_END


static INPUT_PORTS_START( ultraman )
	PORT_START("SYSTEM")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE(0x10, IP_ACTIVE_LOW)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )		PORT_DIPLOCATION("SW1:8,7,6,5")
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, "No Coin A" )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )		PORT_DIPLOCATION("SW1:4,3,2,1")
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, "No Coin B" )		/* 5C_3C according to manual, but it's not true */
	/* No Coin X = coin slot X open (coins produce sound), but no effect on coin counter */

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW2:8" ) /* Manual states it's "Unused" */
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW2:7" ) /* Manual states it's "Unused" */
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Allow_Continue ) )	PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) )		PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) )		PORT_DIPLOCATION("SW2:4,3")
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x40, 0x40, "Upright Controls" )			PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Dual ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )			PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )
INPUT_PORTS_END


static const k051960_interface ultraman_k051960_intf =
{
	"gfx1", 0,
	NORMAL_PLANE_ORDER,
	KONAMI_ROM_DEINTERLEAVE_2,
	ultraman_sprite_callback
};

static const k051316_interface ultraman_k051316_intf_0 =
{
	"gfx2", 1,
	4, FALSE, 0,
	0, 8, 0,
	ultraman_zoom_callback_0
};

static const k051316_interface ultraman_k051316_intf_1 =
{
	"gfx3", 2,
	4, FALSE, 0,
	0, 8, 0,
	ultraman_zoom_callback_1
};

static const k051316_interface ultraman_k051316_intf_2 =
{
	"gfx4", 3,
	4, TRUE, 0,
	0, 8, 0,
	ultraman_zoom_callback_2
};

static MACHINE_START( ultraman )
{
	ultraman_state *state = (ultraman_state *)machine->driver_data;

	state->maincpu = machine->device("maincpu");
	state->audiocpu = machine->device("audiocpu");
	state->k051960 = machine->device("k051960");
	state->k051316_1 = machine->device("k051316_1");
	state->k051316_2 = machine->device("k051316_2");
	state->k051316_3 = machine->device("k051316_3");

	state_save_register_global(machine, state->bank0);
	state_save_register_global(machine, state->bank1);
	state_save_register_global(machine, state->bank2);
}

static MACHINE_RESET( ultraman )
{
	ultraman_state *state = (ultraman_state *)machine->driver_data;

	state->bank0 = -1;
	state->bank1 = -1;
	state->bank2 = -1;
}

static MACHINE_DRIVER_START( ultraman )

	/* driver data */
	MDRV_DRIVER_DATA(ultraman_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000,24000000/2)		/* 12 MHz? */
	MDRV_CPU_PROGRAM_MAP(main_map)
	MDRV_CPU_VBLANK_INT("screen", irq4_line_hold)

	MDRV_CPU_ADD("audiocpu", Z80,24000000/6)	/* 4 MHz? */
	MDRV_CPU_PROGRAM_MAP(sound_map)
	MDRV_CPU_IO_MAP(sound_io_map)

	MDRV_QUANTUM_TIME(HZ(600))

	MDRV_MACHINE_START(ultraman)
	MDRV_MACHINE_RESET(ultraman)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_HAS_SHADOWS)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(14*8, (64-14)*8-1, 2*8, 30*8-1 )

	MDRV_PALETTE_LENGTH(8192)

	MDRV_VIDEO_START(ultraman)
	MDRV_VIDEO_UPDATE(ultraman)

	MDRV_K051960_ADD("k051960", ultraman_k051960_intf)
	MDRV_K051316_ADD("k051316_1", ultraman_k051316_intf_0)
	MDRV_K051316_ADD("k051316_2", ultraman_k051316_intf_1)
	MDRV_K051316_ADD("k051316_3", ultraman_k051316_intf_2)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2151, 24000000/6)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)

	MDRV_OKIM6295_ADD("oki", 1056000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)
MACHINE_DRIVER_END



ROM_START( ultraman )
	ROM_REGION( 0x040000, "maincpu", 0 )	/* 68000 code */
	ROM_LOAD16_BYTE(	"910-b01.c11",	0x000000, 0x020000, CRC(3d9e4323) SHA1(54ee218c9be1ac029836624839d0845b39e6e30f) )
	ROM_LOAD16_BYTE(	"910-b02.d11",	0x000001, 0x020000, CRC(d24c82e9) SHA1(e792e2601e235939546fe98d52bfafe5a95b3491) )

	ROM_REGION( 0x010000, "audiocpu", 0 )	/* Z80 code */
	ROM_LOAD( "910-a05.d05",	0x00000, 0x08000, CRC(ebaef189) SHA1(73e6163466d55ae782f55839ba9c98f06c30876b) )

	ROM_REGION( 0x100000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD( "910-a19.l04",	0x000000, 0x080000, CRC(2dc9ffdc) SHA1(aa34247c82d48c8d13f5209be292127938a4a682) )
	ROM_LOAD( "910-a20.l01",	0x080000, 0x080000, CRC(a4298dce) SHA1(62faf8f0c0490a9562b75ce27909fbee6e84b22a) )

	ROM_REGION( 0x080000, "gfx2", 0 )	/* BG 1  */
	ROM_LOAD( "910-a07.j15",	0x000000, 0x020000, CRC(8b43a64e) SHA1(e373d0fd88b59fb01782dfaeccb1e13673a35766) )
	ROM_LOAD( "910-a08.j16",	0x020000, 0x020000, CRC(c3829826) SHA1(0d383a7afac2a3b5da692375a2b2cd675848861a) )
	ROM_LOAD( "910-a09.j18",	0x040000, 0x020000, CRC(ee10b519) SHA1(a34bd7d89bb8a19af7252ed96ffce212788c586b) )
	ROM_LOAD( "910-a10.j19",	0x060000, 0x020000, CRC(cffbb0c3) SHA1(e9ebe350289f0436de10a6289b04eed3b6a9f98e) )

	ROM_REGION( 0x080000, "gfx3", 0 ) /* BG 2 */
	ROM_LOAD( "910-a11.l15",	0x000000, 0x020000, CRC(17a5581d) SHA1(aca5d465a0e181a266a165aeb0112a4696b0cd18) )
	ROM_LOAD( "910-a12.l16",	0x020000, 0x020000, CRC(39763fb5) SHA1(0e1795af4bae545a0a2be265398837fb2d623232) )
	ROM_LOAD( "910-a13.l18",	0x040000, 0x020000, CRC(66b25a4f) SHA1(954552b005582c90d570ae32c715108ec4b088f1) )
	ROM_LOAD( "910-a14.l19",	0x060000, 0x020000, CRC(09fbd412) SHA1(d11587db7b03f3a75ad8964523bb34f4453bbaca) )

	ROM_REGION( 0x080000, "gfx4", 0 ) /* BG 3 */
	ROM_LOAD( "910-a15.m15",	0x000000, 0x020000, CRC(6d5bfbb7) SHA1(e98c594446b506cb32cc5cc958d2f0de22ebed5e) )
	ROM_LOAD( "910-a16.m16",	0x020000, 0x020000, CRC(5f6f8c3d) SHA1(e365836d2263f36aa4602f0618bf7ce693d2e106) )
	ROM_LOAD( "910-a17.m18",	0x040000, 0x020000, CRC(1f3ec4ff) SHA1(875f53516f47decc4ce31154cf4694c8429ee4ea) )
	ROM_LOAD( "910-a18.m19",	0x060000, 0x020000, CRC(fdc42929) SHA1(079827c1b1a3c32f8547dd91bba8ae37034c16be) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "910-a21.f14",	0x000000, 0x000100, CRC(64460fbc) SHA1(b5295e1d3303d5d816ad44da7b011bbfa613f9e4) )	/* priority encoder (not used) */

	ROM_REGION( 0x040000, "oki", 0 )	/* M6295 data */
	ROM_LOAD( "910-a06.c06",	0x000000, 0x040000, CRC(28fa99c9) SHA1(54663d79ee105ac18d6ba01333a52e3732f2e5fe) )
ROM_END


GAME( 1991, ultraman, 0, ultraman, ultraman, 0, ROT0, "Banpresto / Bandai", "Ultraman (Japan)", GAME_SUPPORTS_SAVE )
