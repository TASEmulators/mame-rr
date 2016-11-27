/***************************************************************************

  Funky Jet                               (c) 1992 Data East / Mitchell Corporation
  Sotsugyo Shousho                        (c) 1995 Mitchell Corporation

  But actually a Data East pcb...  Hardware is pretty close to Super Burger
  Time but with a different graphics chip.

  Emulation by Bryan McPhail, mish@tendril.co.uk


Stephh's notes :

0) 'sotsugyo'

  - COIN2 doesn't work due to code at 0x0001f0 :

        0001F0: 1228 004A                move.b  ($4a,A0), D1

    It should be (as code in 0x00019e) :

        0001F0: 3228 004A                move.w  ($4a,A0), D1

  - SERVICE1 is very strange : instead of adding 1 credit, EACH time it is
    pressed, n credits are added depending on the "Coinage A" Dip Switch :

        Coin_A    n

         3C_1C    1
         2C_1C    1
         1C_1C    1
         1C_2C    2
         1C_3C    3
         1C_4C    4
         1C_5C    5
         1C_6C    6

  - When the "Unused" Dip Switch is ON, the palette is modified.


Funky Jet
Data East, 1992

PCB Layout
----------

DE-0372-0
|-------------------------------------------------------------|
|                                               32.200MHz     |
|   3014   2151     JH02.15F            |-----|               |
|                                       | 45  |  PAL          |
|VOL                6264                |-----|               |
|          JH03.14J             |-----|                       |
|                               | 59  |                       |
| M6295    6264     JH01-2.13F  |-----|    PAL                |
|                                          PAL   63           |
|          6264     JH00-2.11F             PAL                |
|                                     6264               PAL  |
|                                                             |
|    6116                             6264                    |
|    6116                                     28MHz |-------| |
|                     |-------|                     |       | |
|J                    |       |               6116  |  52   | |
|A       DSW2(8)      |  74   |               6116  |       | |
|M                    |       |         6116        |-------| |
|M       DSW1(8)      |-------|         6116                  |
|A      |-----|                                     MAT-01.4A |
|       | 146 |                                               |
|       |-----|     MAT-02.2F                       MAT-00.2A |
|        PAL                                                  |
|-------------------------------------------------------------|
Notes:
      68000 clock     - 14.000MHz [28/2]
      YM2151 clock    - 3.58MHz [32.220/9]
      HuC6280 clock   - 8.050MHz [32.200/4]
      Oki M6295 clock - 1.000MHz [28/28], Sample Rate = 1000000/132
      VSync           - 58Hz
      HSync           - 15.68kHz

      Custom ICs -
                   74 (QFP160)
                   52 (QFP128)
                   45 (QFP80) - HuC6280
                   59 (QFP64) - 68000
                   146(QFP100)
                   63 (SOP28)

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/h6280/h6280.h"
#include "includes/decocrpt.h"
#include "includes/decoprot.h"
#include "includes/funkyjet.h"
#include "sound/2151intf.h"
#include "sound/okim6295.h"
#include "video/deco16ic.h"


/******************************************************************************/

static ADDRESS_MAP_START( funkyjet_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x120000, 0x1207ff) AM_RAM_WRITE(paletteram16_xxxxBBBBGGGGRRRR_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x140000, 0x143fff) AM_RAM
	AM_RANGE(0x160000, 0x1607ff) AM_RAM AM_BASE_SIZE_MEMBER(funkyjet_state, spriteram, spriteram_size)
	AM_RANGE(0x180000, 0x1807ff) AM_READWRITE(deco16_146_funkyjet_prot_r, deco16_146_funkyjet_prot_w) AM_BASE(&deco16_prot_ram)
	AM_RANGE(0x184000, 0x184001) AM_WRITENOP
	AM_RANGE(0x188000, 0x188001) AM_WRITENOP
	AM_RANGE(0x300000, 0x30000f) AM_DEVWRITE("deco_custom", deco16ic_pf12_control_w)
	AM_RANGE(0x320000, 0x321fff) AM_DEVREADWRITE("deco_custom", deco16ic_pf1_data_r, deco16ic_pf1_data_w)
	AM_RANGE(0x322000, 0x323fff) AM_DEVREADWRITE("deco_custom", deco16ic_pf2_data_r, deco16ic_pf2_data_w)
	AM_RANGE(0x340000, 0x340bff) AM_RAM AM_BASE_MEMBER(funkyjet_state, pf1_rowscroll)
	AM_RANGE(0x342000, 0x342bff) AM_RAM AM_BASE_MEMBER(funkyjet_state, pf2_rowscroll)
ADDRESS_MAP_END

/******************************************************************************/

/* Physical memory map (21 bits) */
static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x000000, 0x00ffff) AM_ROM
	AM_RANGE(0x100000, 0x100001) AM_NOP /* YM2203 - this board doesn't have one */
	AM_RANGE(0x110000, 0x110001) AM_DEVREADWRITE("ymsnd", ym2151_r, ym2151_w)
	AM_RANGE(0x120000, 0x120001) AM_DEVREADWRITE("oki", okim6295_r, okim6295_w)
	AM_RANGE(0x130000, 0x130001) AM_NOP /* This board only has 1 oki chip */
	AM_RANGE(0x140000, 0x140001) AM_READ(soundlatch_r)
	AM_RANGE(0x1f0000, 0x1f1fff) AM_RAMBANK("bank8")
	AM_RANGE(0x1fec00, 0x1fec01) AM_WRITE(h6280_timer_w)
	AM_RANGE(0x1ff400, 0x1ff403) AM_WRITE(h6280_irq_status_w)
ADDRESS_MAP_END

/******************************************************************************/

static INPUT_PORTS_START( funkyjet )
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)	/* Button 3 only in "test mode" */
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)	/* Button 3 only in "test mode" */
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_VBLANK )

	/* Dips seem inverted with respect to other Deco games */
	PORT_START("DSW")
	PORT_DIPNAME( 0x00e0, 0x00e0, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x00e0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x00a0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x001c, 0x001c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x001c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0014, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE( 0x0001, IP_ACTIVE_LOW )

	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, "Freeze" )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x8000, "1" )
	PORT_DIPSETTING(      0xc000, "2" )
	PORT_DIPSETTING(      0x4000, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
INPUT_PORTS_END

static INPUT_PORTS_START( funkyjetj )
	PORT_INCLUDE(funkyjet)

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( sotsugyo )
	PORT_INCLUDE(funkyjet)

	PORT_MODIFY("DSW")
	PORT_DIPUNUSED( 0x0100, IP_ACTIVE_LOW )                   // See notes
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0400, "1" )
	PORT_DIPSETTING(      0x0800, "2" )
	PORT_DIPSETTING(      0x0c00, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x3000, 0x2000, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Freeze" )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

/******************************************************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8, RGN_FRAC(1,2)+0, 8, 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static const gfx_layout tile_layout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8, RGN_FRAC(1,2)+0, 8, 0 },
	{ 32*8+0, 32*8+1, 32*8+2, 32*8+3, 32*8+4, 32*8+5, 32*8+6, 32*8+7,
			0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	64*8
};

static GFXDECODE_START( funkyjet )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,  256, 32 )	/* Characters 8x8 */
	GFXDECODE_ENTRY( "gfx1", 0, tile_layout, 256, 32 )	/* Tiles 16x16 */
	GFXDECODE_ENTRY( "gfx2", 0, tile_layout,   0, 16 )	/* Sprites 16x16 */
GFXDECODE_END

/******************************************************************************/

static void sound_irq( running_device *device, int state )
{
	funkyjet_state *driver_state = (funkyjet_state *)device->machine->driver_data;
	cpu_set_input_line(driver_state->audiocpu, 1, state); /* IRQ 2 */
}

static const ym2151_interface ym2151_config =
{
	sound_irq
};

static const deco16ic_interface funkyjet_deco16ic_intf =
{
	"screen",
	1, 0, 1,
	0x0f, 0x0f, 0x0f, 0x0f,	/* trans masks (default values) */
	0, 16, 0, 16, /* color base (default values) */
	0x0f, 0x0f, 0x0f, 0x0f,	/* color masks (default values) */
	NULL, NULL, NULL, NULL
};

static MACHINE_START( funkyjet )
{
	funkyjet_state *state = (funkyjet_state *)machine->driver_data;

	state->maincpu = machine->device("maincpu");
	state->audiocpu = machine->device("audiocpu");
	state->deco16ic = machine->device("deco_custom");
}

static MACHINE_DRIVER_START( funkyjet )

	/* driver data */
	MDRV_DRIVER_DATA(funkyjet_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 14000000) /* 28 MHz crystal */
	MDRV_CPU_PROGRAM_MAP(funkyjet_map)
	MDRV_CPU_VBLANK_INT("screen", irq6_line_hold)

	MDRV_CPU_ADD("audiocpu", H6280,32220000/4)	/* Custom chip 45, Audio section crystal is 32.220 MHz */
	MDRV_CPU_PROGRAM_MAP(sound_map)

	MDRV_MACHINE_START(funkyjet)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(58)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(529))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(40*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(funkyjet)
	MDRV_PALETTE_LENGTH(1024)

	MDRV_VIDEO_UPDATE(funkyjet)

	MDRV_DECO16IC_ADD("deco_custom", funkyjet_deco16ic_intf)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2151, 32220000/9)
	MDRV_SOUND_CONFIG(ym2151_config)
	MDRV_SOUND_ROUTE(0, "lspeaker", 0.45)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.45)

	MDRV_OKIM6295_ADD("oki", 1000000, OKIM6295_PIN7_HIGH)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)
MACHINE_DRIVER_END

/******************************************************************************/

ROM_START( funkyjet )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "jk00.12f", 0x00000, 0x40000, CRC(712089c1) SHA1(84167c90303a228107f55596e2ff8b9f111d1bc2) )
	ROM_LOAD16_BYTE( "jk01.13f", 0x00001, 0x40000, CRC(be3920d7) SHA1(6627956d148681bc49991c544a09b07271ea4c7f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* Sound CPU */
	ROM_LOAD( "jk02.16f",    0x00000, 0x10000, CRC(748c0bd8) SHA1(35910e6a4c4f198fb76bde0f5b053e2c66cfa0ff) )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "mat02", 0x000000, 0x80000, CRC(e4b94c7e) SHA1(7b6ddd0bd388c8d32277fce4b3abb102724bc7d1) ) /* Encrypted chars */

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "mat01", 0x000000, 0x80000, CRC(24093a8d) SHA1(71f76ddd8a4b6e05ceb2fff4e20b6edb5e011e79) ) /* sprites */
	ROM_LOAD( "mat00", 0x080000, 0x80000, CRC(fbda0228) SHA1(815d49898d02e699393e370209181f2ca8301949) )

	ROM_REGION( 0x40000, "oki", 0 )	/* ADPCM samples */
	ROM_LOAD( "jk03.15h",    0x00000, 0x20000, CRC(69a0eaf7) SHA1(05038e82ee03106625f05082fe9912e16be181ee) )
ROM_END

ROM_START( funkyjetj )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "jh00-2.11f", 0x00000, 0x40000, CRC(5b98b700) SHA1(604bd04f4031b0a3b53db2fab4a0e160dff6936d) )
	ROM_LOAD16_BYTE( "jh01-2.13f", 0x00001, 0x40000, CRC(21280220) SHA1(b365b6c8aa778e21a14b2813e93b9c9d02e14995) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* Sound CPU */
	ROM_LOAD( "jk02.16f",    0x00000, 0x10000, CRC(748c0bd8) SHA1(35910e6a4c4f198fb76bde0f5b053e2c66cfa0ff) )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "mat02", 0x000000, 0x80000, CRC(e4b94c7e) SHA1(7b6ddd0bd388c8d32277fce4b3abb102724bc7d1) ) /* Encrypted chars */

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "mat01", 0x000000, 0x80000, CRC(24093a8d) SHA1(71f76ddd8a4b6e05ceb2fff4e20b6edb5e011e79) ) /* sprites */
	ROM_LOAD( "mat00", 0x080000, 0x80000, CRC(fbda0228) SHA1(815d49898d02e699393e370209181f2ca8301949) )

	ROM_REGION( 0x40000, "oki", 0 )	/* ADPCM samples */
	ROM_LOAD( "jk03.15h",    0x00000, 0x20000, CRC(69a0eaf7) SHA1(05038e82ee03106625f05082fe9912e16be181ee) )
ROM_END

ROM_START( sotsugyo )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "03.12f", 0x00000, 0x40000, CRC(d175dfd1) SHA1(61c91d5e20b0492e6ac3b19fe9639eb4f169ae77) )
	ROM_LOAD16_BYTE( "04.13f", 0x00001, 0x40000, CRC(2072477c) SHA1(23820a519e4503854e63ab3ad7eec58178c8d822) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* Sound CPU */
	ROM_LOAD( "sb020.16f",    0x00000, 0x10000, CRC(baf5ec93) SHA1(82b22a0b565e51cd40733f21fa876dd7064eb604) )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "02.2f", 0x000000, 0x80000, CRC(337b1451) SHA1(ab3a4526e683c23b7634ac3304fb073f6ce98e82) ) /* chars */

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "01.4a", 0x000000, 0x80000, CRC(fa10dd54) SHA1(5dfe66df0bbab5eb151bf65f7e767a2325a50b36) ) /* sprites */
	ROM_LOAD( "00.2a", 0x080000, 0x80000, CRC(d35a14ef) SHA1(b8d27766db7e183aee208c690364e4383f3c6882) )

	ROM_REGION( 0x40000, "oki", 0 )	/* ADPCM samples */
	ROM_LOAD( "sb030.15h",    0x00000, 0x20000, CRC(1ea43f48) SHA1(74cc8c740f1c7fa94c2cb460ea4ee7aa0c490ed7) )
ROM_END

static DRIVER_INIT( funkyjet )
{
	deco74_decrypt_gfx(machine, "gfx1");
}

/******************************************************************************/

GAME( 1992, funkyjet, 0,        funkyjet, funkyjet, funkyjet, ROT0, "Data East (Mitchell license)", "Funky Jet (World)", GAME_SUPPORTS_SAVE )
GAME( 1992, funkyjetj,funkyjet, funkyjet, funkyjetj,funkyjet, ROT0, "Data East Corporation", "Funky Jet (Japan)", GAME_SUPPORTS_SAVE )
GAME( 1995, sotsugyo, 0,        funkyjet, sotsugyo, funkyjet, ROT0, "Mitchell (Atlus license)", "Sotsugyo Shousho", GAME_SUPPORTS_SAVE )
