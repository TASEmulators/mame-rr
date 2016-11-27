/***************************************************************************

Juno First :  memory map same as tutankham with some address changes
Chris Hardy (chrish@kcbbs.gen.nz)

Thanks to Rob Jarret for the original Tutankham memory map on which both the
Juno First emu and the mame driver is based on.

        Juno First memory map by Chris Hardy

Read/Write memory

$0000-$7FFF = Screen RAM (only written to)
$8000-$800f = Palette RAM. BBGGGRRR (D7->D0)
$8100-$8FFF = Work RAM

Write memory

$8030   - interrupt control register D0 = interupts on or off
$8031   - unknown
$8032   - unknown
$8033   - unknown
$8034   - flip screen x
$8035   - flip screen y

$8040   - Sound CPU req/ack data
$8050   - Sound CPU command data
$8060   - Banked memory page select.
$8070/1 - Blitter source data word
$8072/3 - Blitter destination word. Write to $8073 triggers a blit

Read memory

$8010   - Dipswitch 2
$801c   - Watchdog
$8020   - Start/Credit IO
                D2 = Credit 1
                D3 = Start 1
                D4 = Start 2
$8024   - P1 IO
                D0 = left
                D1 = right
                D2 = up
                D3 = down
                D4 = fire 2
                D5 = fire 1

$8028   - P2 IO - same as P1 IO
$802c   - Dipswitch 1



$9000->$9FFF Banked Memory - see below
$A000->$BFFF "juno\\JFA_B9.BIN",
$C000->$DFFF "juno\\JFB_B10.BIN",
$E000->$FFFF "juno\\JFC_A10.BIN",

Banked memory - Paged into $9000->$9FFF..

NOTE - In Tutankhm this only contains graphics, in Juno First it also contains code. (which
        generally sets up the blitter)

    "juno\\JFC1_A4.BIN",    $0000->$1FFF
    "juno\\JFC2_A5.BIN",    $2000->$3FFF
    "juno\\JFC3_A6.BIN",    $4000->$5FFF
    "juno\\JFC4_A7.BIN",    $6000->$7FFF
    "juno\\JFC5_A8.BIN",    $8000->$9FFF
    "juno\\JFC6_A9.BIN",    $A000->$bFFF

Blitter source graphics

    "juno\\JFS3_C7.BIN",    $C000->$DFFF
    "juno\\JFS4_D7.BIN",    $E000->$FFFF
    "juno\\JFS5_E7.BIN",    $10000->$11FFF


***************************************************************************/


#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "cpu/mcs48/mcs48.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "sound/flt_rc.h"
#include "machine/konami1.h"
#include "includes/timeplt.h"
#include "includes/konamipt.h"


static WRITE8_HANDLER( junofrst_bankselect_w )
{
	memory_set_bank(space->machine, "bank1", data & 0x0f);
}


static READ8_DEVICE_HANDLER( junofrst_portA_r )
{
	timeplt_state *state = (timeplt_state *)device->machine->driver_data;
	int timer;

	/* main xtal 14.318MHz, divided by 8 to get the CPU clock, further */
	/* divided by 1024 to get this timer */
	/* (divide by (1024/2), and not 1024, because the CPU cycle counter is */
	/* incremented every other state change of the clock) */
	timer = (state->soundcpu->total_cycles() / (1024 / 2)) & 0x0f;

	/* low three bits come from the 8039 */

	return (timer << 4) | state->i8039_status;
}


static WRITE8_DEVICE_HANDLER( junofrst_portB_w )
{
	timeplt_state *state = (timeplt_state *)device->machine->driver_data;
	running_device *filter[3] = { state->filter_0_0, state->filter_0_1, state->filter_0_2 };
	int i;

	for (i = 0; i < 3; i++)
	{
		int C = 0;

		if (data & 1)
			C += 47000;	/* 47000pF = 0.047uF */
		if (data & 2)
			C += 220000;	/* 220000pF = 0.22uF */

		data >>= 2;
		filter_rc_set_RC(filter[i], FLT_RC_LOWPASS, 1000, 2200, 200, CAP_P(C));
	}
}


static WRITE8_HANDLER( junofrst_sh_irqtrigger_w )
{
	timeplt_state *state = (timeplt_state *)space->machine->driver_data;

	if (state->last_irq == 0 && data == 1)
	{
		/* setting bit 0 low then high triggers IRQ on the sound CPU */
		cpu_set_input_line_and_vector(state->soundcpu, 0, HOLD_LINE, 0xff);
	}

	state->last_irq = data;
}


static WRITE8_HANDLER( junofrst_i8039_irq_w )
{
	timeplt_state *state = (timeplt_state *)space->machine->driver_data;
	cpu_set_input_line(state->i8039, 0, ASSERT_LINE);
}


static WRITE8_HANDLER( i8039_irqen_and_status_w )
{
	timeplt_state *state = (timeplt_state *)space->machine->driver_data;

	if ((data & 0x80) == 0)
		cpu_set_input_line(state->i8039, 0, CLEAR_LINE);
	state->i8039_status = (data & 0x70) >> 4;
}


static WRITE8_HANDLER( flip_screen_w )
{
	tutankhm_flip_screen_x_w(space, 0, data);
	tutankhm_flip_screen_y_w(space, 0, data);
}


static WRITE8_HANDLER( junofrst_coin_counter_w )
{
	coin_counter_w(space->machine, offset, data);
}



static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_RAM AM_BASE_SIZE_MEMBER(timeplt_state, videoram, videoram_size)
	AM_RANGE(0x8000, 0x800f) AM_RAM AM_BASE_MEMBER(timeplt_state, paletteram)
	AM_RANGE(0x8010, 0x8010) AM_READ_PORT("DSW2")
	AM_RANGE(0x801c, 0x801c) AM_READ(watchdog_reset_r)
	AM_RANGE(0x8020, 0x8020) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x8024, 0x8024) AM_READ_PORT("P1")
	AM_RANGE(0x8028, 0x8028) AM_READ_PORT("P2")
	AM_RANGE(0x802c, 0x802c) AM_READ_PORT("DSW1")
	AM_RANGE(0x8030, 0x8030) AM_WRITE(interrupt_enable_w)
	AM_RANGE(0x8031, 0x8032) AM_WRITE(junofrst_coin_counter_w)
	AM_RANGE(0x8033, 0x8033) AM_WRITEONLY AM_BASE_MEMBER(timeplt_state, scroll)  /* not used in Juno */
	AM_RANGE(0x8034, 0x8035) AM_WRITE(flip_screen_w)
	AM_RANGE(0x8040, 0x8040) AM_WRITE(junofrst_sh_irqtrigger_w)
	AM_RANGE(0x8050, 0x8050) AM_WRITE(soundlatch_w)
	AM_RANGE(0x8060, 0x8060) AM_WRITE(junofrst_bankselect_w)
	AM_RANGE(0x8070, 0x8073) AM_WRITE(junofrst_blitter_w)
	AM_RANGE(0x8100, 0x8fff) AM_RAM
	AM_RANGE(0x9000, 0x9fff) AM_ROMBANK("bank1")
	AM_RANGE(0xa000, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( audio_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x2000, 0x23ff) AM_RAM
	AM_RANGE(0x3000, 0x3000) AM_READ(soundlatch_r)
	AM_RANGE(0x4000, 0x4000) AM_DEVWRITE("aysnd", ay8910_address_w)
	AM_RANGE(0x4001, 0x4001) AM_DEVREAD("aysnd", ay8910_r)
	AM_RANGE(0x4002, 0x4002) AM_DEVWRITE("aysnd", ay8910_data_w)
	AM_RANGE(0x5000, 0x5000) AM_WRITE(soundlatch2_w)
	AM_RANGE(0x6000, 0x6000) AM_WRITE(junofrst_i8039_irq_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( mcu_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( mcu_io_map, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x00, 0xff) AM_READ(soundlatch2_r)
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_DEVWRITE("dac", dac_w)
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_WRITE(i8039_irqen_and_status_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( junofrst )
	PORT_START("SYSTEM")
	KONAMI8_SYSTEM_UNK

	PORT_START("P1")
	KONAMI8_MONO_B213_UNK

	PORT_START("P2")
	KONAMI8_COCKTAIL_B213_UNK

	PORT_START("DSW1")
	KONAMI_COINAGE(DEF_STR( Free_Play ), "No Coin B")
	/* "No Coin B" = coins produce sound, but no effect on coin counter */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "256 (Cheat)")
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x70, 0x70, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x70, "1 (Easiest)" )
	PORT_DIPSETTING(    0x60, "2" )
	PORT_DIPSETTING(    0x50, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0x30, "5" )
	PORT_DIPSETTING(    0x20, "6" )
	PORT_DIPSETTING(    0x10, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hardest)" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END



static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_HANDLER(junofrst_portA_r),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_HANDLER(junofrst_portB_w)
};


static MACHINE_START( junofrst )
{
	timeplt_state *state = (timeplt_state *)machine->driver_data;

	state->maincpu = machine->device<cpu_device>("maincpu");
	state->i8039 = machine->device("mcu");
	state->soundcpu = machine->device<cpu_device>("audiocpu");
	state->filter_0_0 = machine->device("filter.0.0");
	state->filter_0_1 = machine->device("filter.0.1");
	state->filter_0_2 = machine->device("filter.0.2");

	state_save_register_global(machine, state->i8039_status);
	state_save_register_global(machine, state->last_irq);
	state_save_register_global(machine, state->flip_x);
	state_save_register_global(machine, state->flip_y);
	state_save_register_global_array(machine, state->blitterdata);
}

static MACHINE_RESET( junofrst )
{
	timeplt_state *state = (timeplt_state *)machine->driver_data;

	state->i8039_status = 0;
	state->last_irq = 0;
	state->flip_x = 0;
	state->flip_y = 0;
	state->blitterdata[0] = 0;
	state->blitterdata[1] = 0;
	state->blitterdata[2] = 0;
	state->blitterdata[3] = 0;
}

static MACHINE_DRIVER_START( junofrst )

	/* driver data */
	MDRV_DRIVER_DATA(timeplt_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M6809, 1500000)			/* 1.5 MHz ??? */
	MDRV_CPU_PROGRAM_MAP(main_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_CPU_ADD("audiocpu", Z80,14318000/8)	/* 1.78975 MHz */
	MDRV_CPU_PROGRAM_MAP(audio_map)

	MDRV_CPU_ADD("mcu", I8039,8000000)	/* 8MHz crystal */
	MDRV_CPU_PROGRAM_MAP(mcu_map)
	MDRV_CPU_IO_MAP(mcu_io_map)

	MDRV_MACHINE_START(junofrst)
	MDRV_MACHINE_RESET(junofrst)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(30)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)	/* not sure about the visible area */

	MDRV_VIDEO_UPDATE(tutankhm)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("aysnd", AY8910, 14318000/8)
	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(0, "filter.0.0", 0.30)
	MDRV_SOUND_ROUTE(1, "filter.0.1", 0.30)
	MDRV_SOUND_ROUTE(2, "filter.0.2", 0.30)

	MDRV_SOUND_ADD("dac", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD("filter.0.0", FILTER_RC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
	MDRV_SOUND_ADD("filter.0.1", FILTER_RC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
	MDRV_SOUND_ADD("filter.0.2", FILTER_RC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


ROM_START( junofrst )
	ROM_REGION( 0x1c000, "maincpu", 0 )	/* code + space for decrypted opcodes */
	ROM_LOAD( "jfa_b9.bin",   0x0a000, 0x2000, CRC(f5a7ab9d) SHA1(9603e797839290f8e1f93ccff9cc820604cc49ab) ) /* program ROMs */
	ROM_LOAD( "jfb_b10.bin",  0x0c000, 0x2000, CRC(f20626e0) SHA1(46f58bdc1a613124e2c148b61f774fcc6c232868) )
	ROM_LOAD( "jfc_a10.bin",  0x0e000, 0x2000, CRC(1e7744a7) SHA1(bee69833af886436016560295cddf0c8b4c5e771) )

	ROM_LOAD( "jfc1_a4.bin",  0x10000, 0x2000, CRC(03ccbf1d) SHA1(02b45fe3c51bdc940919aac68136a121ed9bee18) ) /* graphic and code ROMs (banked) */
	ROM_LOAD( "jfc2_a5.bin",  0x12000, 0x2000, CRC(cb372372) SHA1(a48e7de08647cbece7787c287217eac7e7a7510b) )
	ROM_LOAD( "jfc3_a6.bin",  0x14000, 0x2000, CRC(879d194b) SHA1(3c7af8767c9ce908fa1761180c6e585823216d8a) )
	ROM_LOAD( "jfc4_a7.bin",  0x16000, 0x2000, CRC(f28af80b) SHA1(4d0e247e729365476dd3996c7d1f2a19fc83d773) )
	ROM_LOAD( "jfc5_a8.bin",  0x18000, 0x2000, CRC(0539f328) SHA1(c532aaed7f9e6f564e3df0dc6d8fdbee6ed721a2) )
	ROM_LOAD( "jfc6_a9.bin",  0x1a000, 0x2000, CRC(1da2ad6e) SHA1(de997d1b2ff6671088b57192bc9f1279359fad5d) )

	ROM_REGION(  0x10000 , "audiocpu", 0 ) /* 64k for Z80 sound CPU code */
	ROM_LOAD( "jfs1_j3.bin",  0x0000, 0x1000, CRC(235a2893) SHA1(b90251c4971f7ba12e407f86c32723d513d6b4a0) )

	ROM_REGION( 0x1000, "mcu", 0 )	/* 8039 */
	ROM_LOAD( "jfs2_p4.bin",  0x0000, 0x1000, CRC(d0fa5d5f) SHA1(9d0730d1d037bf96b0c933a32355602bf2d735dd) )

	ROM_REGION( 0x6000, "gfx1", 0 )	/* BLTROM, used at runtime */
	ROM_LOAD( "jfs3_c7.bin",  0x00000, 0x2000, CRC(aeacf6db) SHA1(f99ef9f9153d7a83e1881d9181faac99cb8c8a57) )
	ROM_LOAD( "jfs4_d7.bin",  0x02000, 0x2000, CRC(206d954c) SHA1(65494766676f18d8b5ae9a54cee00790e7b1e67e) )
	ROM_LOAD( "jfs5_e7.bin",  0x04000, 0x2000, CRC(1eb87a6e) SHA1(f5471b9f6f1fa6d6e5d76300d89f71da3129516a) )
ROM_END

ROM_START( junofrstg )
	ROM_REGION( 0x1c000, "maincpu", 0 )	/* code + space for decrypted opcodes */
	ROM_LOAD( "jfg_a.9b",     0x0a000, 0x2000, CRC(8f77d1c5) SHA1(d47fcdbc47673c228661a3528fff0c691c76df9e) ) /* program ROMs */
	ROM_LOAD( "jfg_b.10b",    0x0c000, 0x2000, CRC(cd645673) SHA1(25994210a8a424bdf2eca3efa19e7eeffc097cec) )
	ROM_LOAD( "jfg_c.10a",    0x0e000, 0x2000, CRC(47852761) SHA1(eeef814b6ad681d4c2274f0a69d1ed9c5c1b9118) )

	ROM_LOAD( "jfgc1.4a",     0x10000, 0x2000, CRC(90a05ae6) SHA1(0aa835e1d33ab0433189b329b791c952e69103c1) ) /* graphic and code ROMs (banked) */
	ROM_LOAD( "jfc2_a5.bin",  0x12000, 0x2000, CRC(cb372372) SHA1(a48e7de08647cbece7787c287217eac7e7a7510b) )
	ROM_LOAD( "jfc3_a6.bin",  0x14000, 0x2000, CRC(879d194b) SHA1(3c7af8767c9ce908fa1761180c6e585823216d8a) )
	ROM_LOAD( "jfgc4.7a",     0x16000, 0x2000, CRC(e8864a43) SHA1(52b04e69036622abeb6ec99ac3daeda6a2572994) )
	ROM_LOAD( "jfc5_a8.bin",  0x18000, 0x2000, CRC(0539f328) SHA1(c532aaed7f9e6f564e3df0dc6d8fdbee6ed721a2) )
	ROM_LOAD( "jfc6_a9.bin",  0x1a000, 0x2000, CRC(1da2ad6e) SHA1(de997d1b2ff6671088b57192bc9f1279359fad5d) )

	ROM_REGION(  0x10000 , "audiocpu", 0 ) /* 64k for Z80 sound CPU code */
	ROM_LOAD( "jfs1_j3.bin",  0x0000, 0x1000, CRC(235a2893) SHA1(b90251c4971f7ba12e407f86c32723d513d6b4a0) )

	ROM_REGION( 0x1000, "mcu", 0 )	/* 8039 */
	ROM_LOAD( "jfs2_p4.bin",  0x0000, 0x1000, CRC(d0fa5d5f) SHA1(9d0730d1d037bf96b0c933a32355602bf2d735dd) )

	ROM_REGION( 0x6000, "gfx1", 0 )	/* BLTROM, used at runtime */
	ROM_LOAD( "jfs3_c7.bin",  0x00000, 0x2000, CRC(aeacf6db) SHA1(f99ef9f9153d7a83e1881d9181faac99cb8c8a57) )
	ROM_LOAD( "jfs4_d7.bin",  0x02000, 0x2000, CRC(206d954c) SHA1(65494766676f18d8b5ae9a54cee00790e7b1e67e) )
	ROM_LOAD( "jfs5_e7.bin",  0x04000, 0x2000, CRC(1eb87a6e) SHA1(f5471b9f6f1fa6d6e5d76300d89f71da3129516a) )
ROM_END



static DRIVER_INIT( junofrst )
{
	UINT8 *decrypted = konami1_decode(machine, "maincpu");

	memory_configure_bank(machine, "bank1", 0, 16, memory_region(machine, "maincpu") + 0x10000, 0x1000);
	memory_configure_bank_decrypted(machine, "bank1", 0, 16, decrypted + 0x10000, 0x1000);
}


GAME( 1983, junofrst, 0,        junofrst, junofrst, junofrst, ROT90, "Konami", "Juno First", 0 )
GAME( 1983, junofrstg,junofrst, junofrst, junofrst, junofrst, ROT90, "Konami (Gottlieb license)", "Juno First (Gottlieb)", 0 )
