/***************************************************************************

    Block Out

    driver by Nicola Salmoria

    DIP locations verified for:
    - blockout (manual)

****************************************************************************

   Agress PCB Info
   Palco System Corp., 1991

   This game runs on an original unmodified Technos 'Block Out' PCB.
   All of the Technos identifications are hidden under 'Palco' or 'Agress' stickers.

   PCB Layout (Applies to both Agress and Block Out)
   ----------

   PS-05307 (sticker)
   TA-0029-P1-02 (printed on Block Out PCB under the sticker)

   |--------------------------------------------------------|
   | M51516            YM2151                    3.579545MHz|
   |           YM3012     6116                              |
   |   MB3615  1.056MHz   PALCO3.73            20MHz        |
   |           M6295                                        |
   |   MB3615                            82S129PR.25  28MHz |
   |           PALCO4.78                                    |
   |                      Z80            |-------|          |
   |                                     |TECHNOS|          |
   |J         2018     6264              |TJ-001 |          |
   |A                                    |(QFP80)|          |
   |M         2018                       |-------|          |
   |M                                                       |
   |A                  6264                                 |
   |                               MB81461-12               |
   |                               MB81461-12               |
   |                               MB81461-12               |
   |                               MB81461-12               |
   |                               MB81461-12               |
   |  DSW1(8)                      MB81461-12               |
   |                               MB81461-12               |
   |  DSW2(8)                      MB81461-12               |
   |         PALCO2.91                                      |
   |                 PALCO1.81               68000          |
   |--------------------------------------------------------|

   Notes:
      68000 clock : 10.000MHz
      Z80 clock   : 3.579545MHz
      M6295 clock : 1.056MHz, sample rate = 1056000 / 132
      YM2151 clock: 3.579545MHz
      VSync       : 58Hz

      PROM is used for video timing etc, without it, no graphics are displayed,
      only 'Insert Coin' and the manufacturer text/year on the title screen.

      palco1.81 \ Main program   27C010
      palco2.91 /                  "
      palco3.73   OKI samples    27C256
      palco4.78   Z80 program    27C010

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "deprecat.h"
#include "cpu/z80/z80.h"
#include "sound/2151intf.h"
#include "sound/okim6295.h"
#include "includes/blockout.h"


static INTERRUPT_GEN( blockout_interrupt )
{
	/* interrupt 6 is vblank */
	/* interrupt 5 reads coin inputs - might have to be triggered only */
	/* when a coin is inserted */
	cpu_set_input_line(device, 6 - cpu_getiloops(device), HOLD_LINE);
}

static WRITE16_HANDLER( blockout_sound_command_w )
{
	blockout_state *state = (blockout_state *)space->machine->driver_data;

	if (ACCESSING_BITS_0_7)
	{
		soundlatch_w(space, offset, data & 0xff);
		cpu_set_input_line(state->audiocpu, INPUT_LINE_NMI, PULSE_LINE);
	}
}


/*************************************
 *
 *  Address maps
 *
 *************************************/

static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x100000, 0x100001) AM_READ_PORT("P1")
	AM_RANGE(0x100002, 0x100003) AM_READ_PORT("P2")
	AM_RANGE(0x100004, 0x100005) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x100006, 0x100007) AM_READ_PORT("DSW1")
	AM_RANGE(0x100008, 0x100009) AM_READ_PORT("DSW2")
	AM_RANGE(0x100014, 0x100015) AM_WRITE(blockout_sound_command_w)
	AM_RANGE(0x100016, 0x100017) AM_WRITENOP	/* don't know, maybe reset sound CPU */
	AM_RANGE(0x180000, 0x1bffff) AM_RAM_WRITE(blockout_videoram_w) AM_BASE_MEMBER(blockout_state, videoram)
	AM_RANGE(0x1d4000, 0x1dffff) AM_RAM	/* work RAM */
	AM_RANGE(0x1f4000, 0x1fffff) AM_RAM	/* work RAM */
	AM_RANGE(0x200000, 0x207fff) AM_RAM AM_BASE_MEMBER(blockout_state, frontvideoram)
	AM_RANGE(0x208000, 0x21ffff) AM_RAM	/* ??? */
	AM_RANGE(0x280002, 0x280003) AM_WRITE(blockout_frontcolor_w)
	AM_RANGE(0x280200, 0x2805ff) AM_RAM_WRITE(blockout_paletteram_w) AM_BASE_MEMBER(blockout_state, paletteram)
ADDRESS_MAP_END

static ADDRESS_MAP_START( audio_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x8800, 0x8801) AM_DEVREADWRITE("ymsnd", ym2151_r, ym2151_w)
	AM_RANGE(0x9800, 0x9800) AM_DEVREADWRITE("oki", okim6295_r, okim6295_w)
	AM_RANGE(0xa000, 0xa000) AM_READ(soundlatch_r)
ADDRESS_MAP_END


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( blockout )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN3 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )		PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	/* the following two are supposed to control Coin 2, but they don't work. */
	/* This happens on the original board too. */
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW1:3" )		/* According to the manual SW1:3,4 should be the same settings as SW1:1,2 */
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW1:4" )
	PORT_DIPNAME( 0x10, 0x10, "1 Coin to Continue" )	PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )			/* Same price as Coinage setting */
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )			/* Always 1 Coin */
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )	PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW1:7" )		/* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW1:8" )		/* Listed as "Unused" */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )	PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x04, 0x04, "Rotate Buttons" )		PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW2:4" )		/* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5" )		/* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6" )		/* Listed as "Unused" */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 )		PORT_DIPLOCATION("SW2:7") /* Listed as "Unused" */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_DIPLOCATION("SW2:8") /* Listed as "Unused" */
INPUT_PORTS_END

static INPUT_PORTS_START( blockoutj )
	PORT_INCLUDE( blockout )

	PORT_MODIFY("P1")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_MODIFY("P2")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_MODIFY("DSW2")
	/* The following switch is 2/3 rotate buttons on the other sets, option doesn't exist in the Japan version */
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW2:3" )
	/* these can still be used on the difficutly select even if they can't be used for rotating pieces in this version */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON4 )		PORT_DIPLOCATION("SW2:7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_DIPLOCATION("SW2:8")
INPUT_PORTS_END

static INPUT_PORTS_START( agress )
	PORT_INCLUDE( blockout )

	/* factory shipment setting is all dips OFF */
	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x04, 0x04, "Opening Cut" )			PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW1:5" )

	PORT_MODIFY("DSW2")
	/* Engrish here. The manual says "Number of Prayers". Maybe related to lives? */
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Players ) )		PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, "1" )
	PORT_DIPSETTING(    0x00, "2" )
INPUT_PORTS_END


/*************************************
 *
 *  Sound interface
 *
 *************************************/

/* handler called by the 2151 emulator when the internal timers cause an IRQ */
static void blockout_irq_handler(running_device *device, int irq)
{
	blockout_state *state = (blockout_state *)device->machine->driver_data;
	cpu_set_input_line_and_vector(state->audiocpu, 0, irq ? ASSERT_LINE : CLEAR_LINE, 0xff);
}

static const ym2151_interface ym2151_config =
{
	blockout_irq_handler
};


/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_START( blockout )
{
	blockout_state *state = (blockout_state *)machine->driver_data;

	state->audiocpu = machine->device("audiocpu");

	state_save_register_global(machine, state->color);
}

static MACHINE_RESET( blockout )
{
	blockout_state *state = (blockout_state *)machine->driver_data;

	state->color = 0;
}

static MACHINE_DRIVER_START( blockout )

	/* driver data */
	MDRV_DRIVER_DATA(blockout_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 10000000)       /* MRH - 8.76 makes gfx/adpcm samples sync better -- but 10 is correct speed*/
	MDRV_CPU_PROGRAM_MAP(main_map)
	MDRV_CPU_VBLANK_INT_HACK(blockout_interrupt,2)

	MDRV_CPU_ADD("audiocpu", Z80, 3579545)	/* 3.579545 MHz */
	MDRV_CPU_PROGRAM_MAP(audio_map)

	MDRV_MACHINE_START(blockout)
	MDRV_MACHINE_RESET(blockout)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(58)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(320, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 319, 8, 247)

	MDRV_PALETTE_LENGTH(513)

	MDRV_VIDEO_START(blockout)
	MDRV_VIDEO_UPDATE(blockout)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2151, 3579545)
	MDRV_SOUND_CONFIG(ym2151_config)
	MDRV_SOUND_ROUTE(0, "lspeaker", 0.60)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.60)

	MDRV_OKIM6295_ADD("oki", 1056000, OKIM6295_PIN7_HIGH)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)
MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( blockout )
	ROM_REGION( 0x40000, "maincpu", 0 )	/* 2*128k for 68000 code */
	ROM_LOAD16_BYTE( "bo29a0-2.bin", 0x00000, 0x20000, CRC(b0103427) SHA1(53cac2adc04783abbde21e9f3c0e655f22f68f69) )
	ROM_LOAD16_BYTE( "bo29a1-2.bin", 0x00001, 0x20000, CRC(5984d5a2) SHA1(4b350856d0313d40eaa3d8a8d9e310f74bc20398) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "bo29e3-0.bin", 0x0000, 0x8000, CRC(3ea01f78) SHA1(5fc4ad4d9f03d7c26d2afc3e7ede75589e40b0d8) )

	ROM_REGION( 0x40000, "oki", 0 )	/* 128k for ADPCM samples - sound chip is OKIM6295 */
	ROM_LOAD( "bo29e2-0.bin", 0x0000, 0x20000, CRC(15c5a99d) SHA1(89091eda454a028fd1f17501584bd589baf6d523) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "mb7114h.25",   0x0000, 0x0100, CRC(b25bbda7) SHA1(840f1470886bd0019db3cd29e3d1d80205a65f48) )	/* unknown */
ROM_END

ROM_START( blockout2 )
	ROM_REGION( 0x40000, "maincpu", 0 )	/* 2*128k for 68000 code */
	ROM_LOAD16_BYTE( "29a0",         0x00000, 0x20000, CRC(605f931e) SHA1(65fa7227dafde1fc8564e09fa949fe575b394d8a) )
	ROM_LOAD16_BYTE( "29a1",         0x00001, 0x20000, CRC(38f07000) SHA1(e4070e3067d77cc1b0d8d0c63786f2729c5c703a) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "bo29e3-0.bin", 0x0000, 0x8000, CRC(3ea01f78) SHA1(5fc4ad4d9f03d7c26d2afc3e7ede75589e40b0d8) )

	ROM_REGION( 0x40000, "oki", 0 )	/* 128k for ADPCM samples - sound chip is OKIM6295 */
	ROM_LOAD( "bo29e2-0.bin", 0x0000, 0x20000, CRC(15c5a99d) SHA1(89091eda454a028fd1f17501584bd589baf6d523) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "mb7114h.25",   0x0000, 0x0100, CRC(b25bbda7) SHA1(840f1470886bd0019db3cd29e3d1d80205a65f48) )	/* unknown */
ROM_END

ROM_START( blockoutj )
	ROM_REGION( 0x40000, "maincpu", 0 )	/* 2*128k for 68000 code */
	ROM_LOAD16_BYTE( "2.bin",         0x00000, 0x20000, CRC(e16cf065) SHA1(541b30b054cf08f10d6ca4746423759f4326c005) )
	ROM_LOAD16_BYTE( "1.bin",         0x00001, 0x20000, CRC(950b28a3) SHA1(7d1635ac2a3fc1efdd2f78cd6038bd7b4c907b1b) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "bo29e3-0.bin", 0x0000, 0x8000, CRC(3ea01f78) SHA1(5fc4ad4d9f03d7c26d2afc3e7ede75589e40b0d8) )

	ROM_REGION( 0x40000, "oki", 0 )	/* 128k for ADPCM samples - sound chip is OKIM6295 */
	ROM_LOAD( "bo29e2-0.bin", 0x0000, 0x20000, CRC(15c5a99d) SHA1(89091eda454a028fd1f17501584bd589baf6d523) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "mb7114h.25",   0x0000, 0x0100, CRC(b25bbda7) SHA1(840f1470886bd0019db3cd29e3d1d80205a65f48) )	/* unknown */
ROM_END

ROM_START( agress )
	ROM_REGION( 0x40000, "maincpu", 0 )	/* 2*128k for 68000 code */
	ROM_LOAD16_BYTE( "palco1.81",         0x00000, 0x20000, CRC(3acc917a) SHA1(14960588673458d862daf14a8d7474af6c95c2ad) )
	ROM_LOAD16_BYTE( "palco2.91",         0x00001, 0x20000, CRC(abfd5bcc) SHA1(bf0ea8ba00750ea2ddf2b8afc96393bf8a730068) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "palco3.73", 0x0000, 0x8000, CRC(2a21c97d) SHA1(7f71bf18db3e6ff9c69c589268450e66c6585cdd) )

	ROM_REGION( 0x40000, "oki", 0 )	/* 128k for ADPCM samples - sound chip is OKIM6295 */
	ROM_LOAD( "palco4.78", 0x0000, 0x20000, CRC(9dfd9cfe) SHA1(5ea8f98bc0cd117cde81c04f02aa33199afe8231) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129pr.25",   0x0000, 0x0100, CRC(b25bbda7) SHA1(840f1470886bd0019db3cd29e3d1d80205a65f48) )	/* unknown */
ROM_END

// this is probably an original English version with copyright year hacked
ROM_START( agressb )
	ROM_REGION( 0x40000, "maincpu", 0 )	/* 2*128k for 68000 code */
	ROM_LOAD16_BYTE( "palco1.ic81",  0x00000, 0x20000, CRC(a1875175) SHA1(6c9946bcd4fe7987d4f817ea25bfc76432188883) )
	ROM_LOAD16_BYTE( "palco2.ic91",  0x00001, 0x20000, CRC(ab3182c3) SHA1(788a3e7cf6ef889262f3d72af8be9ec951eb397b) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "palco3.ic73",  0x000000, 0x08000, CRC(2a21c97d) SHA1(7f71bf18db3e6ff9c69c589268450e66c6585cdd) )

	ROM_REGION( 0x40000, "oki", 0 )	/* 128k for ADPCM samples - sound chip is OKIM6295 */
	ROM_LOAD( "palco4.ic78",  0x000000, 0x20000, CRC(9dfd9cfe) SHA1(5ea8f98bc0cd117cde81c04f02aa33199afe8231) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "prom29-mb7114h.ic25", 0x000000, 0x0100, CRC(b25bbda7) SHA1(840f1470886bd0019db3cd29e3d1d80205a65f48) )
ROM_END


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1989, blockout, 0,        blockout, blockout, 0, ROT0, "Technos Japan / California Dreams", "Block Out (set 1)", GAME_SUPPORTS_SAVE )
GAME( 1989, blockout2,blockout, blockout, blockout, 0, ROT0, "Technos Japan / California Dreams", "Block Out (set 2)", GAME_SUPPORTS_SAVE )
GAME( 1989, blockoutj,blockout, blockout, blockoutj,0, ROT0, "Technos Japan / California Dreams", "Block Out (Japan)", GAME_SUPPORTS_SAVE )
GAME( 1991, agress,   0,        blockout, agress,   0, ROT0, "Palco", "Agress", GAME_SUPPORTS_SAVE )
GAME( 2003, agressb,  agress,   blockout, agress,   0, ROT0, "bootleg", "Agress (English bootleg)", GAME_SUPPORTS_SAVE )
