/***************************************************************************************************************************

Cherry Chance (c) 1987 Taito Corporation?

driver by David Haywood & Angelo Salese

A cherry-type game that uses the tnzs video chip,might be a modified board as well.

TODO:
-imperfect hopper emulation makes this game to not coin up and gives "hopper time out errors" when you win (put dsw 1
(0x01) and 8 (0x80) to enable hopper full-mode);
-Wrong colors,caused by missing color proms;

============================================================================================================================

Cherry Chance Readme

This one should be a simple project, it uses a Z80 and a Seta graphics chipset and a YM2149 for sound. It is a slot machine.

There are 3 banks of dipswitches, 4,8,8. Battery backup of a 4364 cpu ram. 2 6264 video rams. All 5 eproms are 27512

Chip    checksum
cpu $ba0d
cha0    $2ed7
cha1    $dc81
cha2    $cca8
cha3    $10d8

2 color proms for the output. will get those dumped as well.


*****************************************************************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "includes/tnzs.h"
#include "sound/ay8910.h"

static WRITE8_HANDLER( output_0_w )
{

	//---- --x- divider?
	coin_lockout_w(space->machine, 0, ~data & 1);

//  coin_counter_w(space->machine, 0, ~data & 1);
}


static READ8_HANDLER( input_1_r )
{
	tnzs_state *state = (tnzs_state *)space->machine->driver_data;
	return (state->hop_io) | (state->bell_io) | (input_port_read(space->machine, "SP") & 0xff);
}

static WRITE8_HANDLER( output_1_w )
{
	tnzs_state *state = (tnzs_state *)space->machine->driver_data;

	state->hop_io = (data & 0x40)>>4;
	state->bell_io = (data & 0x80)>>4;
}

static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM

	AM_RANGE(0xa000, 0xbfff) AM_RAM AM_BASE_MEMBER(tnzs_state, objram)

	AM_RANGE(0xc000, 0xdfff) AM_RAM

	AM_RANGE(0xe000, 0xe1ff) AM_RAM AM_BASE_MEMBER(tnzs_state, vdcram)
	AM_RANGE(0xe200, 0xe2ff) AM_RAM AM_BASE_MEMBER(tnzs_state, scrollram) /* scrolling info */
	AM_RANGE(0xe300, 0xe303) AM_RAM AM_MIRROR(0xfc) AM_BASE_MEMBER(tnzs_state, objctrl) /* control registers (0x80 mirror used by Arkanoid 2) */
	AM_RANGE(0xe800, 0xe800) AM_WRITEONLY AM_BASE_MEMBER(tnzs_state, bg_flag)	/* enable / disable background transparency */

	AM_RANGE(0xf000, 0xf000) AM_READNOP AM_WRITENOP //???
	AM_RANGE(0xf001, 0xf001) AM_READ(input_1_r) AM_WRITE(output_0_w)
	AM_RANGE(0xf002, 0xf002) AM_READ_PORT("IN0") AM_WRITE(output_1_w)
	AM_RANGE(0xf800, 0xf801) AM_DEVWRITE("aysnd", ay8910_address_data_w)
	AM_RANGE(0xf801, 0xf801) AM_DEVREAD("aysnd", ay8910_r)
ADDRESS_MAP_END


static INPUT_PORTS_START( cchance )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Bet SW") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Analyzer")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Payout SW")  PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("SP")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Opt 1") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Opt 2") PORT_CODE(KEYCODE_S)
//  PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Payout") PORT_CODE(KEYCODE_D)
//  PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Hop Over") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Slottle") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Drop SW") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Reset Key") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Last Key") PORT_CODE(KEYCODE_K)
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "DSW1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_START("DSW2") //likely to be unused
	PORT_DIPNAME( 0x01, 0x01, "DSW2" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout cchance_layout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8
};



static GFXDECODE_START( cchance )
	GFXDECODE_ENTRY( "gfx1", 0, cchance_layout,   0x0, 32  )
GFXDECODE_END

static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_INPUT_PORT("DSW1"),
	DEVCB_INPUT_PORT("DSW2"),
	DEVCB_NULL,
	DEVCB_NULL
};

static MACHINE_START( cchance )
{
	tnzs_state *state = (tnzs_state *)machine->driver_data;
	state->mcu = NULL;

	state_save_register_global(machine, state->screenflip);
	state_save_register_global(machine, state->hop_io);
	state_save_register_global(machine, state->bell_io);
}

static MACHINE_RESET( cchance )
{
	tnzs_state *state = (tnzs_state *)machine->driver_data;

	state->screenflip = 0;
	state->mcu_type = -1;
	state->hop_io = 0;
	state->bell_io = 0;

}

static MACHINE_DRIVER_START( cchance )

	/* driver data */
	MDRV_DRIVER_DATA(tnzs_state)

	MDRV_CPU_ADD("maincpu", Z80,4000000)		 /* ? MHz */
	MDRV_CPU_PROGRAM_MAP(main_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_MACHINE_START(cchance)
	MDRV_MACHINE_RESET(cchance)

	MDRV_GFXDECODE(cchance)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(57.5)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)

	MDRV_PALETTE_LENGTH(512)

	MDRV_PALETTE_INIT(arknoid2)
	MDRV_VIDEO_UPDATE(tnzs)
	MDRV_VIDEO_EOF(tnzs)

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("aysnd", AY8910, 1500000/2)
	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_DRIVER_END

ROM_START( cchance )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("chance-cccpu.bin", 0x00000, 0x10000, CRC(77531028) SHA1(6f647dea3f1c5884c32a35e04ab6c8a61688171a) )

	ROM_REGION( 0x40000, "gfx1", 0 )
	ROM_LOAD("chance-cccha0.bin", 0x30000, 0x10000, CRC(df8403cf) SHA1(997a6e07079fcbcae2fb82bbd7af0db9b90a03e0))
	ROM_LOAD("chance-cccha1.bin", 0x20000, 0x10000, CRC(26fddc7d) SHA1(d89757c28f14dccdc7d898e19fea59f41f4fa903) )
	ROM_LOAD("chance-cccha2.bin", 0x10000, 0x10000, CRC(fa5ccf5b) SHA1(21957a6a7b88c315d1fbb82e98a924a637a28397) )
	ROM_LOAD("chance-cccha3.bin", 0x00000, 0x10000, CRC(2a2979c9) SHA1(5036313e219ec561fa6753f0db6bb28c6fc97963) )

	ROM_REGION( 0x0400, "proms", 0 )		/* color proms */
	ROM_LOAD( "prom1", 0x0000, 0x0200, NO_DUMP )
	ROM_LOAD( "prom2", 0x0200, 0x0200, NO_DUMP )
ROM_END

GAME( 1987?, cchance,  0,    cchance, cchance,  0, ROT0, "<unknown>", "Cherry Chance", GAME_NOT_WORKING | GAME_WRONG_COLORS | GAME_SUPPORTS_SAVE )
