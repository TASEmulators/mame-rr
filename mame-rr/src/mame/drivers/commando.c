/***************************************************************************

Commando memory map (preliminary)

driver by Nicola Salmoria


MAIN CPU
0000-bfff ROM
d000-d3ff Video RAM
d400-d7ff Color RAM
d800-dbff background video RAM
dc00-dfff background color RAM
e000-ffff RAM
fe00-ff7f Sprites

read:
c000      IN0
c001      IN1
c002      IN2
c003      DSW1
c004      DSW2

write:
c808-c809 background scroll x position
c80a-c80b background scroll y position

SOUND CPU
0000-3fff ROM
4000-47ff RAM

write:
8000      YM2203 #1 control
8001      YM2203 #1 write
8002      YM2203 #2 control
8003      YM2203 #2 write

****************************************************************************

Note : there is an ingame typo bug that doesn't display the bonus life values
       correctly on the title screen in 'commando', 'commandoj' and 'spaceinv'.

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "deprecat.h"
#include "sound/2203intf.h"
#include "includes/commando.h"


/* Memory Maps */

static ADDRESS_MAP_START( commando_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc000) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xc001, 0xc001) AM_READ_PORT("P1")
	AM_RANGE(0xc002, 0xc002) AM_READ_PORT("P2")
	AM_RANGE(0xc003, 0xc003) AM_READ_PORT("DSW1")
	AM_RANGE(0xc004, 0xc004) AM_READ_PORT("DSW2")
	AM_RANGE(0xc800, 0xc800) AM_WRITE(soundlatch_w)
	AM_RANGE(0xc804, 0xc804) AM_WRITE(commando_c804_w)
	AM_RANGE(0xc808, 0xc809) AM_WRITE(commando_scrollx_w)
	AM_RANGE(0xc80a, 0xc80b) AM_WRITE(commando_scrolly_w)
	AM_RANGE(0xd000, 0xd3ff) AM_RAM_WRITE(commando_videoram2_w) AM_BASE_MEMBER(commando_state, videoram2)
	AM_RANGE(0xd400, 0xd7ff) AM_RAM_WRITE(commando_colorram2_w) AM_BASE_MEMBER(commando_state, colorram2)
	AM_RANGE(0xd800, 0xdbff) AM_RAM_WRITE(commando_videoram_w) AM_BASE_MEMBER(commando_state, videoram)
	AM_RANGE(0xdc00, 0xdfff) AM_RAM_WRITE(commando_colorram_w) AM_BASE_MEMBER(commando_state, colorram)
	AM_RANGE(0xe000, 0xfdff) AM_RAM
	AM_RANGE(0xfe00, 0xff7f) AM_RAM AM_BASE_SIZE_GENERIC(spriteram)
	AM_RANGE(0xff80, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x47ff) AM_RAM
	AM_RANGE(0x6000, 0x6000) AM_READ(soundlatch_r)
	AM_RANGE(0x8000, 0x8001) AM_DEVWRITE("ym1", ym2203_w)
	AM_RANGE(0x8002, 0x8003) AM_DEVWRITE("ym2", ym2203_w)
ADDRESS_MAP_END

/* Input Ports */

static INPUT_PORTS_START( commando )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, "Starting Area" )	PORT_DIPLOCATION("SW1:8,7")
	PORT_DIPSETTING(    0x03, "0 (Forest 1)" )
	PORT_DIPSETTING(    0x01, "2 (Desert 1)" )
	PORT_DIPSETTING(    0x02, "4 (Forest 2)" )
	PORT_DIPSETTING(    0x00, "6 (Desert 2)" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:6,5")
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:4,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_3C ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:8,7,6")
	PORT_DIPSETTING(    0x07, "10K 50K+" )
	PORT_DIPSETTING(    0x03, "10K 60K+" )
	PORT_DIPSETTING(    0x05, "20K 60K+" )
	PORT_DIPSETTING(    0x01, "20K 70K+" )
	PORT_DIPSETTING(    0x06, "30K 70K+" )
	PORT_DIPSETTING(    0x02, "30K 80K+" )
	PORT_DIPSETTING(    0x04, "40K 100K+" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Difficult ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:2,1")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, "Upright Two Players" )
	PORT_DIPSETTING(    0xc0, DEF_STR( Cocktail ) )
INPUT_PORTS_END

static INPUT_PORTS_START( commandou )
	PORT_INCLUDE(commando)

	PORT_MODIFY("DSW2")
	PORT_SERVICE_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW2:5" )
INPUT_PORTS_END

/* Graphics Layouts */

static const gfx_layout charlayout =
{
	8, 8,
	RGN_FRAC(1,1),
	2,
	{ 4, 0 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static const gfx_layout tilelayout =
{
	16, 16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8
};

static const gfx_layout spritelayout =
{
	16, 16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+4, RGN_FRAC(1,2)+0, 4, 0 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3,
			32*8+0, 32*8+1, 32*8+2, 32*8+3, 33*8+0, 33*8+1, 33*8+2, 33*8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	64*8
};

/* Graphics Decode Information */

static GFXDECODE_START( commando )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   192, 16 )	// colors 192-255
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout,     0, 16 )	// colors   0-127
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout, 128,  4 )	// colors 128-191
GFXDECODE_END

/* Sound Interface */

#define XTAL		12000000
#define PHI_B		XTAL/2/2
#define PHI_MAIN	4000000	// ??? too complicated to trace from schematics

/* Interrupt Generator */

static INTERRUPT_GEN( commando_interrupt )
{
	cpu_set_input_line_and_vector(device, 0, HOLD_LINE, 0xd7);	// RST 10h - VBLANK
}

/* Machine Driver */

static MACHINE_START( commando )
{
	commando_state *state = (commando_state *)machine->driver_data;

	state->audiocpu = machine->device("audiocpu");

	state_save_register_global_array(machine, state->scroll_x);
	state_save_register_global_array(machine, state->scroll_y);
}

static MACHINE_RESET( commando )
{
	commando_state *state = (commando_state *)machine->driver_data;

	state->scroll_x[0] = 0;
	state->scroll_x[1] = 0;
	state->scroll_y[0] = 0;
	state->scroll_y[1] = 0;
}


static MACHINE_DRIVER_START( commando )

	/* driver data */
	MDRV_DRIVER_DATA(commando_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, PHI_MAIN)	// ???
	MDRV_CPU_PROGRAM_MAP(commando_map)
	MDRV_CPU_VBLANK_INT("screen", commando_interrupt)

	MDRV_CPU_ADD("audiocpu", Z80, PHI_B)	// 3 MHz
	MDRV_CPU_PROGRAM_MAP(sound_map)
	MDRV_CPU_VBLANK_INT_HACK(irq0_line_hold, 4)

	MDRV_MACHINE_START(commando)
	MDRV_MACHINE_RESET(commando)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_BUFFERS_SPRITERAM)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(commando)
	MDRV_PALETTE_LENGTH(256)

	MDRV_PALETTE_INIT(RRRR_GGGG_BBBB)
	MDRV_VIDEO_START(commando)
	MDRV_VIDEO_UPDATE(commando)
	MDRV_VIDEO_EOF(commando)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ym1", YM2203, PHI_B/2)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)

	MDRV_SOUND_ADD("ym2", YM2203, PHI_B/2)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)
MACHINE_DRIVER_END


/* ROMs */

ROM_START( commando )
	ROM_REGION( 2*0x10000, "maincpu", 0 )	/* 64k for code + 64k for decrypted opcodes */
	ROM_LOAD( "cm04.9m",  0x0000, 0x8000, CRC(8438b694) SHA1(e154478d8f1b635355bd777370acabe49cb9d309) )
	ROM_LOAD( "cm03.8m",  0x8000, 0x4000, CRC(35486542) SHA1(531a85c9e03970ce037be84f2240c2df6f6e3ec1) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "cm02.9f",  0x0000, 0x4000, CRC(f9cc4a74) SHA1(ee8dd73919c6f47f62cc6d999de9510db9f79b8f) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "vt01.5d",  0x0000, 0x4000, CRC(505726e0) SHA1(2435c87c9c9d78a6e703cf0e1f6a0288207fcd4c) )	// characters

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "vt11.5a",  0x00000, 0x4000, CRC(7b2e1b48) SHA1(5d49e1d8146e4ef744445b68f35677302e875a85) )	// SCR X (tiles)
	ROM_LOAD( "vt12.6a",  0x04000, 0x4000, CRC(81b417d3) SHA1(5ec7e3f0c8069384a5f6eb39232c228b9d7b8c0c) )	// SCR X
	ROM_LOAD( "vt13.7a",  0x08000, 0x4000, CRC(5612dbd2) SHA1(9e4e1a22b6cbf60607b9a81dae34482ae55f7c47) )	// SCR Y
	ROM_LOAD( "vt14.8a",  0x0c000, 0x4000, CRC(2b2dee36) SHA1(8792278464fa3da47176582025f6673a15a581e2) )	// SCR Y
	ROM_LOAD( "vt15.9a",  0x10000, 0x4000, CRC(de70babf) SHA1(6717e23baf55f84d3143fb432140a7c3e102ac26) )	// SCR Z
	ROM_LOAD( "vt16.10a", 0x14000, 0x4000, CRC(14178237) SHA1(f896e71c7004349c9a46155edfd9f0aaa186065d) )	// SCR Z

	ROM_REGION( 0x18000, "gfx3", 0 )
	ROM_LOAD( "vt05.7e",  0x00000, 0x4000, CRC(79f16e3d) SHA1(04e1f03a4d6b4cc2b81bce3a290bbb95de900d35) )	// sprites
	ROM_LOAD( "vt06.8e",  0x04000, 0x4000, CRC(26fee521) SHA1(2fbfc73ee860f72a20229a01d4da9f5cc2e858d3) )
	ROM_LOAD( "vt07.9e",  0x08000, 0x4000, CRC(ca88bdfd) SHA1(548b05460bc7983cc81f15c70e87f47d10db2812) )
	ROM_LOAD( "vt08.7h",  0x0c000, 0x4000, CRC(2019c883) SHA1(883c0156ceab99f4849fe36972c4162b4ac8c216) )
	ROM_LOAD( "vt09.8h",  0x10000, 0x4000, CRC(98703982) SHA1(ba9a9b0dcadd4f52502828408c4a19b0bd518351) )
	ROM_LOAD( "vt10.9h",  0x14000, 0x4000, CRC(f069d2f8) SHA1(2c92300a9407470b34965021de882f1f7a84730c) )

	ROM_REGION( 0x600, "proms", 0 )
	ROM_LOAD( "vtb1.1d",  0x0000, 0x0100, CRC(3aba15a1) SHA1(8b057f6e26155dd9e48bde182e680fce4519f600) )	/* red */
	ROM_LOAD( "vtb2.2d",  0x0100, 0x0100, CRC(88865754) SHA1(ca6dddca98baf00a65b2fb70b69cf4704ef8c831) )	/* green */
	ROM_LOAD( "vtb3.3d",  0x0200, 0x0100, CRC(4c14c3f6) SHA1(644ac17c7413f094ec9a15cba87bbd421b26321f) )	/* blue */
	ROM_LOAD( "vtb4.1h",  0x0300, 0x0100, CRC(b388c246) SHA1(038f9851699331ad887b6281a9df053dca3db8fd) )	/* palette selector (not used) */
	ROM_LOAD( "vtb5.6l",  0x0400, 0x0100, CRC(712ac508) SHA1(5349d722ab6733afdda65f6e0a98322f0d515e86) )	/* interrupt timing (not used) */
	ROM_LOAD( "vtb6.6e",  0x0500, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )	/* video timing (not used) */
ROM_END

ROM_START( commandou )
	ROM_REGION( 2*0x10000, "maincpu", 0 )	/* 64k for code + 64k for decrypted opcodes */
	ROM_LOAD( "u4-f.9m",  0x0000, 0x8000, CRC(a6118935) SHA1(d5811968b23d61e344e151747bcc3c0ed2b9497b) )
	ROM_LOAD( "u3-f.8m",  0x8000, 0x4000, CRC(24f49684) SHA1(d38a7bd9f3b506747a03f6b94c3f8a2d9fc59166) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "cm02.9f",  0x0000, 0x4000, CRC(f9cc4a74) SHA1(ee8dd73919c6f47f62cc6d999de9510db9f79b8f) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "vt01.5d",  0x0000, 0x4000, CRC(505726e0) SHA1(2435c87c9c9d78a6e703cf0e1f6a0288207fcd4c) )	// characters

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "vt11.5a",  0x00000, 0x4000, CRC(7b2e1b48) SHA1(5d49e1d8146e4ef744445b68f35677302e875a85) )	// SCR X (tiles)
	ROM_LOAD( "vt12.6a",  0x04000, 0x4000, CRC(81b417d3) SHA1(5ec7e3f0c8069384a5f6eb39232c228b9d7b8c0c) )	// SCR X
	ROM_LOAD( "vt13.7a",  0x08000, 0x4000, CRC(5612dbd2) SHA1(9e4e1a22b6cbf60607b9a81dae34482ae55f7c47) )	// SCR Y
	ROM_LOAD( "vt14.8a",  0x0c000, 0x4000, CRC(2b2dee36) SHA1(8792278464fa3da47176582025f6673a15a581e2) )	// SCR Y
	ROM_LOAD( "vt15.9a",  0x10000, 0x4000, CRC(de70babf) SHA1(6717e23baf55f84d3143fb432140a7c3e102ac26) )	// SCR Z
	ROM_LOAD( "vt16.10a", 0x14000, 0x4000, CRC(14178237) SHA1(f896e71c7004349c9a46155edfd9f0aaa186065d) )	// SCR Z

	ROM_REGION( 0x18000, "gfx3", 0 )
	ROM_LOAD( "vt05.7e",  0x00000, 0x4000, CRC(79f16e3d) SHA1(04e1f03a4d6b4cc2b81bce3a290bbb95de900d35) )	// sprites
	ROM_LOAD( "vt06.8e",  0x04000, 0x4000, CRC(26fee521) SHA1(2fbfc73ee860f72a20229a01d4da9f5cc2e858d3) )
	ROM_LOAD( "vt07.9e",  0x08000, 0x4000, CRC(ca88bdfd) SHA1(548b05460bc7983cc81f15c70e87f47d10db2812) )
	ROM_LOAD( "vt08.7h",  0x0c000, 0x4000, CRC(2019c883) SHA1(883c0156ceab99f4849fe36972c4162b4ac8c216) )
	ROM_LOAD( "vt09.8h",  0x10000, 0x4000, CRC(98703982) SHA1(ba9a9b0dcadd4f52502828408c4a19b0bd518351) )
	ROM_LOAD( "vt10.9h",  0x14000, 0x4000, CRC(f069d2f8) SHA1(2c92300a9407470b34965021de882f1f7a84730c) )

	ROM_REGION( 0x600, "proms", 0 )
	ROM_LOAD( "vtb1.1d",  0x0000, 0x0100, CRC(3aba15a1) SHA1(8b057f6e26155dd9e48bde182e680fce4519f600) )	/* red */
	ROM_LOAD( "vtb2.2d",  0x0100, 0x0100, CRC(88865754) SHA1(ca6dddca98baf00a65b2fb70b69cf4704ef8c831) )	/* green */
	ROM_LOAD( "vtb3.3d",  0x0200, 0x0100, CRC(4c14c3f6) SHA1(644ac17c7413f094ec9a15cba87bbd421b26321f) )	/* blue */
	ROM_LOAD( "vtb4.1h",  0x0300, 0x0100, CRC(b388c246) SHA1(038f9851699331ad887b6281a9df053dca3db8fd) )	/* palette selector (not used) */
	ROM_LOAD( "vtb5.6l",  0x0400, 0x0100, CRC(712ac508) SHA1(5349d722ab6733afdda65f6e0a98322f0d515e86) )	/* interrupt timing (not used) */
	ROM_LOAD( "vtb6.6e",  0x0500, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )	/* video timing (not used) */
ROM_END

ROM_START( commandoj )
	ROM_REGION( 2*0x10000, "maincpu", 0 )	/* 64k for code + 64k for decrypted opcodes */
	ROM_LOAD( "so04.9m", 0x0000, 0x8000, CRC(d3f2bfb3) SHA1(738a5673ac6a907cb04cfb125e8aab3f7437b9d2) )
	ROM_LOAD( "so03.8m", 0x8000, 0x4000, CRC(ed01f472) SHA1(fa181293ae8f0fee78d412259eb81f6de1e1307a) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "so02.9f", 0x0000, 0x4000, CRC(ca20aca5) SHA1(206a8fd4a8985e7ceed7de8349ba02627e881503) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "vt01.5d",  0x0000, 0x4000, CRC(505726e0) SHA1(2435c87c9c9d78a6e703cf0e1f6a0288207fcd4c) )	// characters

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "vt11.5a",  0x00000, 0x4000, CRC(7b2e1b48) SHA1(5d49e1d8146e4ef744445b68f35677302e875a85) )	// SCR X (tiles)
	ROM_LOAD( "vt12.6a",  0x04000, 0x4000, CRC(81b417d3) SHA1(5ec7e3f0c8069384a5f6eb39232c228b9d7b8c0c) )	// SCR X
	ROM_LOAD( "vt13.7a",  0x08000, 0x4000, CRC(5612dbd2) SHA1(9e4e1a22b6cbf60607b9a81dae34482ae55f7c47) )	// SCR Y
	ROM_LOAD( "vt14.8a",  0x0c000, 0x4000, CRC(2b2dee36) SHA1(8792278464fa3da47176582025f6673a15a581e2) )	// SCR Y
	ROM_LOAD( "vt15.9a",  0x10000, 0x4000, CRC(de70babf) SHA1(6717e23baf55f84d3143fb432140a7c3e102ac26) )	// SCR Z
	ROM_LOAD( "vt16.10a", 0x14000, 0x4000, CRC(14178237) SHA1(f896e71c7004349c9a46155edfd9f0aaa186065d) )	// SCR Z

	ROM_REGION( 0x18000, "gfx3", 0 )
	ROM_LOAD( "vt05.7e",  0x00000, 0x4000, CRC(79f16e3d) SHA1(04e1f03a4d6b4cc2b81bce3a290bbb95de900d35) )	// sprites
	ROM_LOAD( "vt06.8e",  0x04000, 0x4000, CRC(26fee521) SHA1(2fbfc73ee860f72a20229a01d4da9f5cc2e858d3) )
	ROM_LOAD( "vt07.9e",  0x08000, 0x4000, CRC(ca88bdfd) SHA1(548b05460bc7983cc81f15c70e87f47d10db2812) )
	ROM_LOAD( "vt08.7h",  0x0c000, 0x4000, CRC(2019c883) SHA1(883c0156ceab99f4849fe36972c4162b4ac8c216) )
	ROM_LOAD( "vt09.8h",  0x10000, 0x4000, CRC(98703982) SHA1(ba9a9b0dcadd4f52502828408c4a19b0bd518351) )
	ROM_LOAD( "vt10.9h",  0x14000, 0x4000, CRC(f069d2f8) SHA1(2c92300a9407470b34965021de882f1f7a84730c) )

	ROM_REGION( 0x600, "proms", 0 )
	ROM_LOAD( "vtb1.1d",  0x0000, 0x0100, CRC(3aba15a1) SHA1(8b057f6e26155dd9e48bde182e680fce4519f600) )	/* red */
	ROM_LOAD( "vtb2.2d",  0x0100, 0x0100, CRC(88865754) SHA1(ca6dddca98baf00a65b2fb70b69cf4704ef8c831) )	/* green */
	ROM_LOAD( "vtb3.3d",  0x0200, 0x0100, CRC(4c14c3f6) SHA1(644ac17c7413f094ec9a15cba87bbd421b26321f) )	/* blue */
	ROM_LOAD( "vtb4.1h",  0x0300, 0x0100, CRC(b388c246) SHA1(038f9851699331ad887b6281a9df053dca3db8fd) )	/* palette selector (not used) */
	ROM_LOAD( "vtb5.6l",  0x0400, 0x0100, CRC(712ac508) SHA1(5349d722ab6733afdda65f6e0a98322f0d515e86) )	/* interrupt timing (not used) */
	ROM_LOAD( "vtb6.6e",  0x0500, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )	/* video timing (not used) */
ROM_END

ROM_START( commandob )
	ROM_REGION( 2*0x10000, "maincpu", 0 )	/* 64k for code + 64k for decrypted opcodes */
	ROM_LOAD( "commandob_04_9m_27256.bin",  0x0000, 0x8000, CRC(348a7654) SHA1(f3668c47c154a9c7d7afeabb0259c9bc56e847ac) )
	ROM_LOAD( "cm03.8m",  0x8000, 0x4000, CRC(35486542) SHA1(531a85c9e03970ce037be84f2240c2df6f6e3ec1) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "cm02.9f",  0x0000, 0x4000, CRC(f9cc4a74) SHA1(ee8dd73919c6f47f62cc6d999de9510db9f79b8f) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "vt01.5d",  0x0000, 0x4000, CRC(505726e0) SHA1(2435c87c9c9d78a6e703cf0e1f6a0288207fcd4c) )	// characters

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "vt11.5a",  0x00000, 0x4000, CRC(7b2e1b48) SHA1(5d49e1d8146e4ef744445b68f35677302e875a85) )	// SCR X (tiles)
	ROM_LOAD( "vt12.6a",  0x04000, 0x4000, CRC(81b417d3) SHA1(5ec7e3f0c8069384a5f6eb39232c228b9d7b8c0c) )	// SCR X
	ROM_LOAD( "vt13.7a",  0x08000, 0x4000, CRC(5612dbd2) SHA1(9e4e1a22b6cbf60607b9a81dae34482ae55f7c47) )	// SCR Y
	ROM_LOAD( "vt14.8a",  0x0c000, 0x4000, CRC(2b2dee36) SHA1(8792278464fa3da47176582025f6673a15a581e2) )	// SCR Y
	ROM_LOAD( "vt15.9a",  0x10000, 0x4000, CRC(de70babf) SHA1(6717e23baf55f84d3143fb432140a7c3e102ac26) )	// SCR Z
	ROM_LOAD( "vt16.10a", 0x14000, 0x4000, CRC(14178237) SHA1(f896e71c7004349c9a46155edfd9f0aaa186065d) )	// SCR Z

	ROM_REGION( 0x18000, "gfx3", 0 )
	ROM_LOAD( "vt05.7e",  0x00000, 0x4000, CRC(79f16e3d) SHA1(04e1f03a4d6b4cc2b81bce3a290bbb95de900d35) )	// sprites
	ROM_LOAD( "vt06.8e",  0x04000, 0x4000, CRC(26fee521) SHA1(2fbfc73ee860f72a20229a01d4da9f5cc2e858d3) )
	ROM_LOAD( "vt07.9e",  0x08000, 0x4000, CRC(ca88bdfd) SHA1(548b05460bc7983cc81f15c70e87f47d10db2812) )
	ROM_LOAD( "vt08.7h",  0x0c000, 0x4000, CRC(2019c883) SHA1(883c0156ceab99f4849fe36972c4162b4ac8c216) )
	ROM_LOAD( "vt09.8h",  0x10000, 0x4000, CRC(98703982) SHA1(ba9a9b0dcadd4f52502828408c4a19b0bd518351) )
	ROM_LOAD( "vt10.9h",  0x14000, 0x4000, CRC(f069d2f8) SHA1(2c92300a9407470b34965021de882f1f7a84730c) )

        /* I did not dumped the PROMs of the bootleg board, I'm just adding the parent ones, it has the same
           number of PROMs on the same board locations as the original board. */
	ROM_REGION( 0x600, "proms", 0 )
	ROM_LOAD( "vtb1.1d",  0x0000, 0x0100, CRC(3aba15a1) SHA1(8b057f6e26155dd9e48bde182e680fce4519f600) )	/* red */
	ROM_LOAD( "vtb2.2d",  0x0100, 0x0100, CRC(88865754) SHA1(ca6dddca98baf00a65b2fb70b69cf4704ef8c831) )	/* green */
	ROM_LOAD( "vtb3.3d",  0x0200, 0x0100, CRC(4c14c3f6) SHA1(644ac17c7413f094ec9a15cba87bbd421b26321f) )	/* blue */
	ROM_LOAD( "vtb4.1h",  0x0300, 0x0100, CRC(b388c246) SHA1(038f9851699331ad887b6281a9df053dca3db8fd) )	/* palette selector (not used) */
	ROM_LOAD( "vtb5.6l",  0x0400, 0x0100, CRC(712ac508) SHA1(5349d722ab6733afdda65f6e0a98322f0d515e86) )	/* interrupt timing (not used) */
	ROM_LOAD( "vtb6.6e",  0x0500, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )	/* video timing (not used) */

        /* There's a 16L8A PAL (with a 74LS244P and a 74LS367AP) on a tiny sub-board between the CPU1 ROMs
           and the CPU1 (a Z80 compatible NEC D780C-1). This sub-board is plugged on what seems to be
           a ROM socket. */
	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "commandob_pal16l8a.bin", 0x0000, 0x0104, NO_DUMP ) /* I Didn't try to dump it... */
ROM_END

ROM_START( sinvasn )
	ROM_REGION( 2*0x10000, "maincpu", 0 )	/* 64k for code + 64k for decrypted opcodes */
	ROM_LOAD( "sp04.9m",  0x0000, 0x8000, CRC(33f9601e) SHA1(71182227b77fccbbc1d89b5828aa86dcc64ca05e) )
	ROM_LOAD( "sp03.8m",  0x8000, 0x4000, CRC(c7fb43b3) SHA1(36d0dffdacc36a6b6a77101d942c0821846f3275) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "u2.9f",    0x0000, 0x4000, CRC(cbf8c40e) SHA1(0c8dce034d96d075e012cbb8f68c2817b860d969) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "u1.5d",    0x0000, 0x4000, CRC(f477e13a) SHA1(ec5b80f5d508501e72cba028dc45b2c307ac452b) )	/* characters */

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "vt11.5a",  0x00000, 0x4000, CRC(7b2e1b48) SHA1(5d49e1d8146e4ef744445b68f35677302e875a85) )	// SCR X (tiles)
	ROM_LOAD( "vt12.6a",  0x04000, 0x4000, CRC(81b417d3) SHA1(5ec7e3f0c8069384a5f6eb39232c228b9d7b8c0c) )	// SCR X
	ROM_LOAD( "vt13.7a",  0x08000, 0x4000, CRC(5612dbd2) SHA1(9e4e1a22b6cbf60607b9a81dae34482ae55f7c47) )	// SCR Y
	ROM_LOAD( "vt14.8a",  0x0c000, 0x4000, CRC(2b2dee36) SHA1(8792278464fa3da47176582025f6673a15a581e2) )	// SCR Y
	ROM_LOAD( "vt15.9a",  0x10000, 0x4000, CRC(de70babf) SHA1(6717e23baf55f84d3143fb432140a7c3e102ac26) )	// SCR Z
	ROM_LOAD( "vt16.10a", 0x14000, 0x4000, CRC(14178237) SHA1(f896e71c7004349c9a46155edfd9f0aaa186065d) )	// SCR Z

	ROM_REGION( 0x18000, "gfx3", 0 )
	ROM_LOAD( "u5.e7",    0x00000, 0x4000, CRC(2a97c933) SHA1(bfddb0c0f930a7caffad7e52d394d72c09ffb45f) )	/* sprites */
	ROM_LOAD( "sp06.e8",  0x04000, 0x4000, CRC(d7887212) SHA1(43ad98263d6314d40abf33087127c23a3ad72335) )
	ROM_LOAD( "sp07.e9",  0x08000, 0x4000, CRC(9abe7a20) SHA1(5f1b851bd66a3ab818b893286d3ebf2194f425c4) )
	ROM_LOAD( "u8.h7",    0x0c000, 0x4000, CRC(d6b4aa2e) SHA1(5bbf536f73010182b9150dd4fb1e2a42b5b380b0) )
	ROM_LOAD( "sp09.h8",  0x10000, 0x4000, CRC(3985b318) SHA1(ac4c67c3af42121869c1b9470377404bc88793c2) )
	ROM_LOAD( "sp10.h9",  0x14000, 0x4000, CRC(3c131b0f) SHA1(dd3e63199120502c03eedd024a2eed3b5d3e2a1c) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "vtb1.1d",  0x0000, 0x0100, CRC(3aba15a1) SHA1(8b057f6e26155dd9e48bde182e680fce4519f600) )	/* red */
	ROM_LOAD( "vtb2.2d",  0x0100, 0x0100, CRC(88865754) SHA1(ca6dddca98baf00a65b2fb70b69cf4704ef8c831) )	/* green */
	ROM_LOAD( "vtb3.3d",  0x0200, 0x0100, CRC(4c14c3f6) SHA1(644ac17c7413f094ec9a15cba87bbd421b26321f) )	/* blue */
	ROM_LOAD( "vtb4.1h",  0x0300, 0x0100, CRC(b388c246) SHA1(038f9851699331ad887b6281a9df053dca3db8fd) )	/* palette selector (not used) */
	ROM_LOAD( "vtb5.6l",  0x0400, 0x0100, CRC(712ac508) SHA1(5349d722ab6733afdda65f6e0a98322f0d515e86) )	/* interrupt timing (not used) */
	ROM_LOAD( "vtb6.6e",  0x0500, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )	/* video timing (not used) */
ROM_END

ROM_START( sinvasnb )
	ROM_REGION( 2*0x10000, "maincpu", 0 )	/* 64k for code + 64k for decrypted opcodes */
	ROM_LOAD( "u4",       0x0000, 0x8000, CRC(834ba0de) SHA1(85f40559e6a436f3f752b6e862a419a5b9481fa8) )
	ROM_LOAD( "u3",       0x8000, 0x4000, CRC(07e4ee3a) SHA1(6d7665b3072f075893ef37e55147b10271d069ef) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "u2",       0x0000, 0x4000, CRC(cbf8c40e) SHA1(0c8dce034d96d075e012cbb8f68c2817b860d969) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "u1",       0x0000, 0x4000, CRC(f477e13a) SHA1(ec5b80f5d508501e72cba028dc45b2c307ac452b) )	/* characters */

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "vt11.5a",  0x00000, 0x4000, CRC(7b2e1b48) SHA1(5d49e1d8146e4ef744445b68f35677302e875a85) )	// SCR X (tiles)
	ROM_LOAD( "vt12.6a",  0x04000, 0x4000, CRC(81b417d3) SHA1(5ec7e3f0c8069384a5f6eb39232c228b9d7b8c0c) )	// SCR X
	ROM_LOAD( "vt13.7a",  0x08000, 0x4000, CRC(5612dbd2) SHA1(9e4e1a22b6cbf60607b9a81dae34482ae55f7c47) )	// SCR Y
	ROM_LOAD( "vt14.8a",  0x0c000, 0x4000, CRC(2b2dee36) SHA1(8792278464fa3da47176582025f6673a15a581e2) )	// SCR Y
	ROM_LOAD( "vt15.9a",  0x10000, 0x4000, CRC(de70babf) SHA1(6717e23baf55f84d3143fb432140a7c3e102ac26) )	// SCR Z
	ROM_LOAD( "vt16.10a", 0x14000, 0x4000, CRC(14178237) SHA1(f896e71c7004349c9a46155edfd9f0aaa186065d) )	// SCR Z

	ROM_REGION( 0x18000, "gfx3", 0 )
	ROM_LOAD( "u5",       0x00000, 0x4000, CRC(2a97c933) SHA1(bfddb0c0f930a7caffad7e52d394d72c09ffb45f) )	/* sprites */
	ROM_LOAD( "vt06.e8",  0x04000, 0x4000, CRC(26fee521) SHA1(2fbfc73ee860f72a20229a01d4da9f5cc2e858d3) )
	ROM_LOAD( "vt07.e9",  0x08000, 0x4000, CRC(ca88bdfd) SHA1(548b05460bc7983cc81f15c70e87f47d10db2812) )
	ROM_LOAD( "u8",       0x0c000, 0x4000, CRC(d6b4aa2e) SHA1(5bbf536f73010182b9150dd4fb1e2a42b5b380b0) )
	ROM_LOAD( "vt09.h8",  0x10000, 0x4000, CRC(98703982) SHA1(ba9a9b0dcadd4f52502828408c4a19b0bd518351) )
	ROM_LOAD( "vt10.h9",  0x14000, 0x4000, CRC(f069d2f8) SHA1(2c92300a9407470b34965021de882f1f7a84730c) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "vtb1.1d",  0x0000, 0x0100, CRC(3aba15a1) SHA1(8b057f6e26155dd9e48bde182e680fce4519f600) )	/* red */
	ROM_LOAD( "vtb2.2d",  0x0100, 0x0100, CRC(88865754) SHA1(ca6dddca98baf00a65b2fb70b69cf4704ef8c831) )	/* green */
	ROM_LOAD( "vtb3.3d",  0x0200, 0x0100, CRC(4c14c3f6) SHA1(644ac17c7413f094ec9a15cba87bbd421b26321f) )	/* blue */
	ROM_LOAD( "vtb4.1h",  0x0300, 0x0100, CRC(b388c246) SHA1(038f9851699331ad887b6281a9df053dca3db8fd) )	/* palette selector (not used) */
	ROM_LOAD( "vtb5.6l",  0x0400, 0x0100, CRC(712ac508) SHA1(5349d722ab6733afdda65f6e0a98322f0d515e86) )	/* interrupt timing (not used) */
	ROM_LOAD( "vtb6.6e",  0x0500, 0x0100, CRC(0eaf5158) SHA1(bafd4108708f66cd7b280e47152b108f3e254fc9) )	/* video timing (not used) */
ROM_END

/* Driver Initialization */

static DRIVER_INIT( commando )
{
	const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);
	UINT8 *rom = memory_region(machine, "maincpu");
	UINT8 *decrypt = auto_alloc_array(machine, UINT8, 0xc000);
	int A;

	memory_set_decrypted_region(space, 0x0000, 0xbfff, decrypt);

	// the first opcode is *not* encrypted
	decrypt[0] = rom[0];
	for (A = 1; A < 0xc000; A++)
	{
		int src;

		src = rom[A];
		decrypt[A] = (src & 0x11) | ((src & 0xe0) >> 4) | ((src & 0x0e) << 4);
	}
}

static DRIVER_INIT( spaceinv )
{
	const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);
	UINT8 *rom = memory_region(machine, "maincpu");
	UINT8 *decrypt = auto_alloc_array(machine, UINT8, 0xc000);
	int A;

	memory_set_decrypted_region(space, 0x0000, 0xbfff, decrypt);

	// the first opcode *is* encrypted
	for (A = 0; A < 0xc000; A++)
	{
		int src;

		src = rom[A];
		decrypt[A] = (src & 0x11) | ((src & 0xe0) >> 4) | ((src & 0x0e) << 4);
	}
}

/* Game Drivers */

GAME( 1985, commando,  0,        commando, commando, commando, ROT270, "Capcom", "Commando (World)", GAME_SUPPORTS_SAVE )
GAME( 1985, commandou, commando, commando, commandou,commando, ROT270, "Capcom (Data East USA license)", "Commando (US)", GAME_SUPPORTS_SAVE )
GAME( 1985, commandoj, commando, commando, commando, commando, ROT270, "Capcom", "Senjou no Ookami", GAME_SUPPORTS_SAVE )
GAME( 1985, commandob, commando, commando, commando, spaceinv, ROT270, "bootleg", "Commando (bootleg)", GAME_SUPPORTS_SAVE )
GAME( 1985, sinvasn,   commando, commando, commando, commando, ROT270, "Capcom", "Space Invasion (Europe)", GAME_SUPPORTS_SAVE )
GAME( 1985, sinvasnb,  commando, commando, commando, spaceinv, ROT270, "bootleg", "Space Invasion (bootleg)", GAME_SUPPORTS_SAVE )
