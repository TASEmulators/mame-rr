/***************************************************************************

                              -= Metal Clash =-

    driver by Luca Elia, based on brkthru.c by Phil Stroffolino


CPUs    :   2 x 6809
Sound   :   YM2203  +  YM3526
Video   :   TC15G008AP + TC15G032CY (TOSHIBA)

---------------------------------------------------------------------------
Year + Game         Boards
---------------------------------------------------------------------------
85  Metal Clash     DE-0212-1 & DE-0213-1
---------------------------------------------------------------------------

Notes:

- Similar hardware to that in brkthru.c
- Screenshots here: www.ne.jp/asahi/cc-sakura/akkun/bekkan/metal.html

To Do:

metlclsh:
- Clocks are all unknown
- Text on the title screen has wrong colors the first time around
 (unitialized foreground palette 1, will be initialized shortly)
- The background tilemap ram is bankswitched with other (not understood) ram
- There are a few unknown writes

***************************************************************************/

#include "emu.h"
#include "deprecat.h"
#include "cpu/m6809/m6809.h"
#include "sound/2203intf.h"
#include "sound/3526intf.h"
#include "includes/metlclsh.h"


/***************************************************************************

                            Memory Maps - CPU #1

***************************************************************************/

static WRITE8_HANDLER( metlclsh_cause_irq )
{
	metlclsh_state *state = (metlclsh_state *)space->machine->driver_data;
	cpu_set_input_line(state->subcpu, M6809_IRQ_LINE, ASSERT_LINE);
}

static WRITE8_HANDLER( metlclsh_ack_nmi )
{
	metlclsh_state *state = (metlclsh_state *)space->machine->driver_data;
	cpu_set_input_line(state->maincpu, INPUT_LINE_NMI, CLEAR_LINE);
}

static ADDRESS_MAP_START( metlclsh_master_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x9fff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0xa000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc000) AM_READ_PORT("IN0")
	AM_RANGE(0xc001, 0xc001) AM_READ_PORT("IN1")
	AM_RANGE(0xc002, 0xc002) AM_READ_PORT("IN2")
	AM_RANGE(0xc003, 0xc003) AM_READ_PORT("DSW")
	AM_RANGE(0xc080, 0xc080) AM_WRITENOP							// ? 0
	AM_RANGE(0xc0c2, 0xc0c2) AM_WRITE(metlclsh_cause_irq)			// cause irq on cpu #2
	AM_RANGE(0xc0c3, 0xc0c3) AM_WRITE(metlclsh_ack_nmi)				// nmi ack
/**/AM_RANGE(0xc800, 0xc82f) AM_RAM_WRITE(paletteram_xxxxBBBBGGGGRRRR_split1_w) AM_BASE_GENERIC(paletteram)
/**/AM_RANGE(0xcc00, 0xcc2f) AM_RAM_WRITE(paletteram_xxxxBBBBGGGGRRRR_split2_w) AM_BASE_GENERIC(paletteram2)
	AM_RANGE(0xd000, 0xd001) AM_DEVREADWRITE("ym1", ym2203_r,ym2203_w)
/**/AM_RANGE(0xd800, 0xdfff) AM_RAM_WRITE(metlclsh_fgram_w) AM_BASE_MEMBER(metlclsh_state, fgram)
	AM_RANGE(0xe000, 0xe001) AM_DEVWRITE("ym2", ym3526_w	)
	AM_RANGE(0xe800, 0xe9ff) AM_RAM AM_BASE_SIZE_MEMBER(metlclsh_state, spriteram, spriteram_size)
	AM_RANGE(0xfff0, 0xffff) AM_ROM									// Reset/IRQ vectors
ADDRESS_MAP_END


/***************************************************************************

                            Memory Maps - CPU #2

***************************************************************************/

static WRITE8_HANDLER( metlclsh_cause_nmi2 )
{
	metlclsh_state *state = (metlclsh_state *)space->machine->driver_data;
	cpu_set_input_line(state->maincpu, INPUT_LINE_NMI, ASSERT_LINE);
}

static WRITE8_HANDLER( metlclsh_ack_irq2 )
{
	metlclsh_state *state = (metlclsh_state *)space->machine->driver_data;
	cpu_set_input_line(state->subcpu, M6809_IRQ_LINE, CLEAR_LINE);
}

static WRITE8_HANDLER( metlclsh_ack_nmi2 )
{
	metlclsh_state *state = (metlclsh_state *)space->machine->driver_data;
	cpu_set_input_line(state->subcpu, INPUT_LINE_NMI, CLEAR_LINE);
}

static WRITE8_HANDLER( metlclsh_flipscreen_w )
{
	flip_screen_set(space->machine, data & 1);
}

static ADDRESS_MAP_START( metlclsh_slave_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x9fff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0xc000, 0xc000) AM_READ_PORT("IN0") AM_WRITE(metlclsh_gfxbank_w)	// bg tiles bank
	AM_RANGE(0xc001, 0xc001) AM_READ_PORT("IN1")
	AM_RANGE(0xc002, 0xc002) AM_READ_PORT("IN2")
	AM_RANGE(0xc003, 0xc003) AM_READ_PORT("DSW")
	AM_RANGE(0xc0c0, 0xc0c0) AM_WRITE(metlclsh_cause_nmi2)			// cause nmi on cpu #1
	AM_RANGE(0xc0c1, 0xc0c1) AM_WRITE(metlclsh_ack_irq2)			// irq ack
	AM_RANGE(0xd000, 0xd7ff) AM_ROMBANK("bank1") AM_WRITE(metlclsh_bgram_w) AM_BASE_MEMBER(metlclsh_state, bgram) // this is banked
	AM_RANGE(0xe301, 0xe301) AM_WRITE(metlclsh_flipscreen_w)		// 0/1
	AM_RANGE(0xe401, 0xe401) AM_WRITE(metlclsh_rambank_w)
	AM_RANGE(0xe402, 0xe403) AM_WRITEONLY AM_BASE_MEMBER(metlclsh_state, scrollx)
//  AM_RANGE(0xe404, 0xe404) AM_WRITENOP                            // ? 0
//  AM_RANGE(0xe410, 0xe410) AM_WRITENOP                            // ? 0 on startup only
	AM_RANGE(0xe417, 0xe417) AM_WRITE(metlclsh_ack_nmi2)			// nmi ack
	AM_RANGE(0xfff0, 0xffff) AM_ROM									// Reset/IRQ vectors
ADDRESS_MAP_END


/***************************************************************************

                                Input Ports

***************************************************************************/

static INPUT_PORTS_START( metlclsh )
	PORT_START("IN0")		/* c000 */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Infinite Energy (Cheat)")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Infinite Lives (Cheat)")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")		/* c001 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN2")		/* c002 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(1)

	PORT_START("DSW")		/* c003 */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPNAME( 0x02, 0x02, "Enemies Speed" )
	PORT_DIPSETTING(    0x02, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x04, 0x04, "Enemies Energy" )
	PORT_DIPSETTING(    0x04, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x08, 0x08, "Time" )
	PORT_DIPSETTING(    0x00, "75" )
	PORT_DIPSETTING(    0x08, "90" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )	// cpu2 will clr c040 on startup forever
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_VBLANK )
INPUT_PORTS_END


/***************************************************************************

                            Graphics Layouts

***************************************************************************/

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ STEP8(8*8*2,1), STEP8(8*8*0,1) },
	{ STEP8(8*8*0,8), STEP8(8*8*1,8) },
	16*16
};

static const gfx_layout tilelayout16 =
{
	16,16,
	RGN_FRAC(1,4),
	3,
	{ RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{ STEP8(8*8*0+7,-1), STEP8(8*8*2+7,-1) },
	{ STEP8(8*8*0,8), STEP8(8*8*1,8) },
	16*16
};

static const gfx_layout tilelayout8 =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ 0, 4 },
	{ STEP4(RGN_FRAC(1,2),1), STEP4(0,1) },
	{ STEP8(0,8) },
	8*8
};

static GFXDECODE_START( metlclsh )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout, 0x00, 2 ) // [0] Sprites
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout16, 0x10, 1 ) // [1] Background
	GFXDECODE_ENTRY( "gfx3", 0, tilelayout8,  0x20, 4 ) // [2] Foreground
GFXDECODE_END


/***************************************************************************

                                Machine Drivers

***************************************************************************/

static void metlclsh_irqhandler(running_device *device, int linestate)
{
	metlclsh_state *state = (metlclsh_state *)device->machine->driver_data;
	cpu_set_input_line(state->maincpu, M6809_IRQ_LINE, linestate);
}

static const ym3526_interface ym3526_config =
{
	metlclsh_irqhandler
};

static INTERRUPT_GEN( metlclsh_interrupt2 )
{
	if (cpu_getiloops(device) == 0)
		return;
	/* generate NMI on coin insertion */
	if ((~input_port_read(device->machine, "IN2") & 0xc0) || (~input_port_read(device->machine, "DSW") & 0x40))
		cpu_set_input_line(device, INPUT_LINE_NMI, ASSERT_LINE);
}

static MACHINE_START( metlclsh )
{
	metlclsh_state *state = (metlclsh_state *)machine->driver_data;

	state->maincpu = machine->device("maincpu");
	state->subcpu = machine->device("sub");

	state_save_register_global(machine, state->write_mask);
	state_save_register_global(machine, state->gfxbank);
}

static MACHINE_RESET( metlclsh )
{
	metlclsh_state *state = (metlclsh_state *)machine->driver_data;

	flip_screen_set(machine, 0);

	state->write_mask = 0;
	state->gfxbank = 0;
}

static MACHINE_DRIVER_START( metlclsh )

	/* driver data */
	MDRV_DRIVER_DATA(metlclsh_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M6809, 1500000)        // ?
	MDRV_CPU_PROGRAM_MAP(metlclsh_master_map)
	// IRQ by YM3526, NMI by cpu #2

	MDRV_CPU_ADD("sub", M6809, 1500000)        // ?
	MDRV_CPU_PROGRAM_MAP(metlclsh_slave_map)
	MDRV_CPU_VBLANK_INT_HACK(metlclsh_interrupt2,2)
	// IRQ by cpu #1, NMI by coins insertion

	MDRV_MACHINE_START(metlclsh)
	MDRV_MACHINE_RESET(metlclsh)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(58)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)	// we're using IPT_VBLANK
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 1*8, 30*8-1)

	MDRV_GFXDECODE(metlclsh)
	MDRV_PALETTE_LENGTH(3 * 16)

	MDRV_VIDEO_START(metlclsh)
	MDRV_VIDEO_UPDATE(metlclsh)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ym1", YM2203, 1500000)
	MDRV_SOUND_ROUTE(0, "mono", 0.10)
	MDRV_SOUND_ROUTE(1, "mono", 0.10)
	MDRV_SOUND_ROUTE(2, "mono", 0.10)
	MDRV_SOUND_ROUTE(3, "mono", 0.50)

	MDRV_SOUND_ADD("ym2", YM3526, 3000000)
	MDRV_SOUND_CONFIG(ym3526_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END


/***************************************************************************

                                ROMs Loading

***************************************************************************/

/***************************************************************************

METAL CLASH[DATA EAST] JAPAN (c)1985
ROM Type:2764,27256

Name            Size    Location
--------------------------------
CS00.BIN    2764    C11 cpu
CS01.BIN    27256   C12 cpu
CS02.BIN    27256   C14 cpu
CS03.BIN    27256   C15 cpu
CS04.BIN    27256   C17 cpu
CS05.BIN    27256   H7  sound

CS06.BIN    27256   D9  Video
CS07.BIN    27256   D10 Video
CS08.BIN    27256   D12 Video

TTL-PROM 82S123(Color Table,8bit x 32Byte).
0000:3A 78 79 71 75 74 76 32
0008:3A 3D 29 21 25 14 16 12
0010:00 00 00 00 00 00 00 00
0018:00 00 00 00 00 00 00 00

This ROM work at DE-0212-1 & DE-0213-1

cpu   :6809(MAIN),6809(SOUND)
sound :YM2203,YM3526
custom:TC15G008AP,TC15G032CY(TOSHIBA)
color :82S123

DIP-SW
SW1
1 Coin CHARGE SELECT 1
2 Coin CHARGE SELECT 1
3 Coin CHARGE SELECT 2
4 Coin CHARGE SELECT 2
5 DON't CHANGE(for SERVICE ??)
6 ATTRACT SOUND
7 My ROBOT Infinity
8 My ROBOT LEFT not Decriment

SW2
1 My ROBOT LEFT 2/3
2 EMENY SPEED  EASY/DIFFICULT
3 EMENY ENERGY EASY/DIFFICULT
4 TIME LONG/DIFFICULT
5 SCREEN CHANGE NORMAL/FLIP
6 none ??
7 none ??
8 DON't CHANGE(for SERVICE ??)

"DARWIN 4078" use TC15G032CY too.

***************************************************************************/

ROM_START( metlclsh )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cs04.bin",    0x00000, 0x8000, CRC(c2cc79a6) SHA1(0f586d4145afabbb45ea4865ed7a6590b14a2ab0) )
	ROM_LOAD( "cs00.bin",    0x0a000, 0x2000, CRC(af0f2998) SHA1(09dd2516406168660d5cd3a36be1e5f0adbcdb8a) )
	ROM_COPY( "maincpu", 0x7ff0, 0xfff0, 0x10 )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "cs03.bin",    0x00000, 0x8000, CRC(51c4720c) SHA1(7fd93bdcf029e7d2509b73b32f61fddf85f3453f) )
	ROM_COPY( "sub", 0x7ff0, 0xfff0, 0x10 )

	ROM_REGION( 0x18000, "gfx1", 0 )	// Sprites
	ROM_LOAD( "cs06.bin",    0x00000, 0x8000, CRC(9f61403f) SHA1(0ebb1cb9d4983746b6b32ec948e7b9efd90783d1) )
	ROM_LOAD( "cs07.bin",    0x08000, 0x8000, CRC(d0610ea5) SHA1(3dfa16cbe93a4c08993111f78a8dd22c874fdd28) )
	ROM_LOAD( "cs08.bin",    0x10000, 0x8000, CRC(a8b02125) SHA1(145a22b2910b2fbfb28925f58968ee2bdeae1dda) )

	ROM_REGION( 0x10000, "gfx2", 0 )	// Background
	ROM_LOAD( "cs01.bin",    0x00000, 0x1000, CRC(9c72343d) SHA1(c5618be7874ab6c930b0e68935c93f1958a1916d) )
	ROM_CONTINUE(            0x04000, 0x1000 )
	ROM_CONTINUE(            0x08000, 0x1000 )
	ROM_CONTINUE(            0x0c000, 0x1000 )
	ROM_CONTINUE(            0x01000, 0x1000 )
	ROM_CONTINUE(            0x05000, 0x1000 )
	ROM_CONTINUE(            0x09000, 0x1000 )
	ROM_CONTINUE(            0x0d000, 0x1000 )
	ROM_LOAD( "cs02.bin",    0x02000, 0x1000, CRC(3674673e) SHA1(8ba8864cefcb79afe5fe6821005a9d19742756e9) )
	ROM_CONTINUE(            0x06000, 0x1000 )
	ROM_CONTINUE(            0x0a000, 0x1000 )
	ROM_CONTINUE(            0x0e000, 0x1000 )
	ROM_CONTINUE(            0x03000, 0x1000 )
	ROM_CONTINUE(            0x07000, 0x1000 )
	ROM_CONTINUE(            0x0b000, 0x1000 )
	ROM_CONTINUE(            0x0f000, 0x1000 )

	ROM_REGION( 0x04000, "gfx3", 0 )	// Foreground
	ROM_LOAD( "cs05.bin",    0x00000, 0x4000, CRC(f90c9c6b) SHA1(ca8e497e9c388078343dd1303beef6ee38748d6a) )
	ROM_CONTINUE(            0x00000, 0x4000 )	// first half is empty

	ROM_REGION( 0x020, "proms", 0 )	// ?
	ROM_LOAD( "82s123.prm",   0x0000, 0x20, CRC(6844cc88) SHA1(89d23367aa6ff541205416e82781fe938dfeeb52) )
ROM_END

GAME( 1985, metlclsh, 0, metlclsh, metlclsh, 0, ROT0, "Data East", "Metal Clash (Japan)", GAME_SUPPORTS_SAVE )
