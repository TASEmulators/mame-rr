/***************************************************************************

    Yie Ar Kung-Fu memory map (preliminary)
    enrique.sanchez@cs.us.es

CPU:    Motorola 6809

Normal 6809 IRQs must be generated each video frame (60 fps).
The 6809 NMI is used for sound timing.


0000        R   VLM5030 status ???
4000         W  control port
                    bit 0 - flip screen
                    bit 1 - NMI enable
                    bit 2 - IRQ enable
                    bit 3 - coin counter A
                    bit 4 - coin counter B
4800         W  sound latch write
4900         W  copy sound latch to SN76496
4a00         W  VLM5030 control
4b00         W  VLM5030 data
4c00        R   DSW #0
4d00        R   DSW #1
4e00        R   IN #0
4e01        R   IN #1
4e02        R   IN #2
4e03        R   DSW #2
4f00         W  watchdog
5000-502f    W  sprite RAM 1 (18 sprites)
                    byte 0 - bit 0 - sprite code MSB
                             bit 6 - flip X
                             bit 7 - flip Y
                    byte 1 - Y position
5030-53ff   RW  RAM
5400-542f    W  sprite RAM 2
                    byte 0 - X position
                    byte 1 - sprite code LSB
5430-57ff   RW  RAM
5800-5fff   RW  video RAM
                    byte 0 - bit 4 - character code MSB
                             bit 6 - flip Y
                             bit 7 - flip X
                    byte 1 - character code LSB
8000-ffff   R   ROM


***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "sound/sn76496.h"
#include "sound/vlm5030.h"
#include "includes/konamipt.h"
#include "includes/trackfld.h"


static READ8_DEVICE_HANDLER( yiear_speech_r )
{
	if (vlm5030_bsy(device))
		return 1;
	else
		return 0;
}

static WRITE8_DEVICE_HANDLER( yiear_VLM5030_control_w )
{
	/* bit 0 is latch direction */
	vlm5030_st(device, (data >> 1) & 1);
	vlm5030_rst(device, (data >> 2) & 1);
}

static INTERRUPT_GEN( yiear_nmi_interrupt )
{
	trackfld_state *state = (trackfld_state *)device->machine->driver_data;

	/* can't use nmi_line_pulse() because interrupt_enable_w() effects it */
	if (state->yiear_nmi_enable)
		cpu_set_input_line(device, INPUT_LINE_NMI, PULSE_LINE);
}


static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0000) AM_DEVREAD("vlm", yiear_speech_r)
	AM_RANGE(0x4000, 0x4000) AM_WRITE(yiear_control_w)
	AM_RANGE(0x4800, 0x4800) AM_WRITE(konami_SN76496_latch_w)
	AM_RANGE(0x4900, 0x4900) AM_DEVWRITE("snsnd", konami_SN76496_w)
	AM_RANGE(0x4a00, 0x4a00) AM_DEVWRITE("vlm", yiear_VLM5030_control_w)
	AM_RANGE(0x4b00, 0x4b00) AM_DEVWRITE("vlm", vlm5030_data_w)
	AM_RANGE(0x4c00, 0x4c00) AM_READ_PORT("DSW1")
	AM_RANGE(0x4d00, 0x4d00) AM_READ_PORT("DSW2")
	AM_RANGE(0x4e00, 0x4e00) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x4e01, 0x4e01) AM_READ_PORT("P1")
	AM_RANGE(0x4e02, 0x4e02) AM_READ_PORT("P2")
	AM_RANGE(0x4e03, 0x4e03) AM_READ_PORT("DSW3")
	AM_RANGE(0x4f00, 0x4f00) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x5000, 0x502f) AM_RAM AM_BASE_SIZE_MEMBER(trackfld_state, spriteram, spriteram_size)
	AM_RANGE(0x5400, 0x542f) AM_RAM AM_BASE_MEMBER(trackfld_state, spriteram2)
	AM_RANGE(0x5800, 0x5fff) AM_WRITE(yiear_videoram_w) AM_BASE_MEMBER(trackfld_state, videoram)
	AM_RANGE(0x5000, 0x5fff) AM_RAM
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END


static INPUT_PORTS_START( yiear )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1")
	KONAMI8_MONO_B12_UNK

	PORT_START("P2")
	KONAMI8_COCKTAIL_B12_UNK

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x08, "30000 80000" )
	PORT_DIPSETTING(    0x00, "40000 90000" )
	PORT_DIPNAME( 0x30, 0x10, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Upright Controls" )
	PORT_DIPSETTING(    0x02, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Dual ) )
	PORT_SERVICE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW3")
	KONAMI_COINAGE(DEF_STR( Free_Play ), "Invalid")
	/* "Invalid" = both coin slots disabled */
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ 4, 0, RGN_FRAC(1,2)+4, RGN_FRAC(1,2)+0 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ 4, 0, RGN_FRAC(1,2)+4, RGN_FRAC(1,2)+0 },
	{ 0*8*8+0, 0*8*8+1, 0*8*8+2, 0*8*8+3, 1*8*8+0, 1*8*8+1, 1*8*8+2, 1*8*8+3,
	  2*8*8+0, 2*8*8+1, 2*8*8+2, 2*8*8+3, 3*8*8+0, 3*8*8+1, 3*8*8+2, 3*8*8+3 },
	{  0*8,  1*8,  2*8,  3*8,  4*8,  5*8,  6*8,  7*8,
	  32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8 },
	64*8
};

static GFXDECODE_START( yiear )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   16, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout,  0, 1 )
GFXDECODE_END



static MACHINE_START( yiear )
{
	trackfld_state *state = (trackfld_state *)machine->driver_data;

	state->audiocpu = NULL;
	state->vlm = machine->device("vlm");

	state_save_register_global(machine, state->yiear_nmi_enable);

	/* sound */
	state_save_register_global(machine, state->SN76496_latch);
	state_save_register_global(machine, state->last_addr);
	state_save_register_global(machine, state->last_irq);
}

static MACHINE_RESET( yiear )
{
	trackfld_state *state = (trackfld_state *)machine->driver_data;

	state->yiear_nmi_enable = 0;
	state->SN76496_latch = 0;
	state->last_addr = 0;
	state->last_irq = 0;
}

static MACHINE_DRIVER_START( yiear )

	/* driver data */
	MDRV_DRIVER_DATA(trackfld_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M6809,XTAL_18_432MHz/12)   /* verified on pcb */
	MDRV_CPU_PROGRAM_MAP(main_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)
	MDRV_CPU_PERIODIC_INT(yiear_nmi_interrupt,480)	/* music tempo (correct frequency unknown) */

	MDRV_MACHINE_START(yiear)
	MDRV_MACHINE_RESET(yiear)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60.58)   /* verified on pcb */
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(yiear)
	MDRV_PALETTE_LENGTH(32)

	MDRV_PALETTE_INIT(yiear)
	MDRV_VIDEO_START(yiear)
	MDRV_VIDEO_UPDATE(yiear)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("snsnd", SN76489A, XTAL_18_432MHz/12)   /* verified on pcb */
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MDRV_SOUND_ADD("vlm", VLM5030, XTAL_3_579545MHz)   /* verified on pcb */
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( yiear )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "i08.10d",      0x08000, 0x4000, CRC(e2d7458b) SHA1(1b192130b5cd879ab686a21aa2b518c90edd89aa) )
	ROM_LOAD( "i07.8d",       0x0c000, 0x4000, CRC(7db7442e) SHA1(d604a995a5505251904447ad697fc9e7f94bf241) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "g16_1.bin",    0x00000, 0x2000, CRC(b68fd91d) SHA1(c267931d69794c292b7ebae5bc35ad842194683a) )
	ROM_LOAD( "g15_2.bin",    0x02000, 0x2000, CRC(d9b167c6) SHA1(a2fd10bddfa4e95e9d49892737ace146209bfa2b) )

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "g04_5.bin",    0x00000, 0x4000, CRC(45109b29) SHA1(0794935b490497b21b99045c90231b7bac151d42) )
	ROM_LOAD( "g03_6.bin",    0x04000, 0x4000, CRC(1d650790) SHA1(5f2a4983b20251c712358547a7c62c0331c6cb6f) )
	ROM_LOAD( "g06_3.bin",    0x08000, 0x4000, CRC(e6aa945b) SHA1(c5757d16c28f5966fd04675c0c640ef9b6b76ca5) )
	ROM_LOAD( "g05_4.bin",    0x0c000, 0x4000, CRC(cc187c22) SHA1(555ba18a9648681e5140b3fd84af16959ee5296d) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "yiear.clr",    0x00000, 0x0020, CRC(c283d71f) SHA1(10cd39f4e951ba6ca5610081c8c1fcd9d68b34d2) )

	ROM_REGION( 0x2000, "vlm", 0 )	/* 8k for the VLM5030 data */
	ROM_LOAD( "a12_9.bin",    0x00000, 0x2000, CRC(f75a1539) SHA1(f139f6cb41351eb81ee47d777db03012aa5fadb1) )
ROM_END

ROM_START( yiear2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "d12_8.bin",    0x08000, 0x4000, CRC(49ecd9dd) SHA1(15692029351e87837cc5a251947ff315fd723aa4) )
	ROM_LOAD( "d14_7.bin",    0x0c000, 0x4000, CRC(bc2e1208) SHA1(a5a0c78ff4e02bd7da3eab3842dfe99956e74155) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "g16_1.bin",    0x00000, 0x2000, CRC(b68fd91d) SHA1(c267931d69794c292b7ebae5bc35ad842194683a) )
	ROM_LOAD( "g15_2.bin",    0x02000, 0x2000, CRC(d9b167c6) SHA1(a2fd10bddfa4e95e9d49892737ace146209bfa2b) )

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "g04_5.bin",    0x00000, 0x4000, CRC(45109b29) SHA1(0794935b490497b21b99045c90231b7bac151d42) )
	ROM_LOAD( "g03_6.bin",    0x04000, 0x4000, CRC(1d650790) SHA1(5f2a4983b20251c712358547a7c62c0331c6cb6f) )
	ROM_LOAD( "g06_3.bin",    0x08000, 0x4000, CRC(e6aa945b) SHA1(c5757d16c28f5966fd04675c0c640ef9b6b76ca5) )
	ROM_LOAD( "g05_4.bin",    0x0c000, 0x4000, CRC(cc187c22) SHA1(555ba18a9648681e5140b3fd84af16959ee5296d) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "yiear.clr",    0x00000, 0x0020, CRC(c283d71f) SHA1(10cd39f4e951ba6ca5610081c8c1fcd9d68b34d2) )

	ROM_REGION( 0x2000, "vlm", 0 )	/* 8k for the VLM5030 data */
	ROM_LOAD( "a12_9.bin",    0x00000, 0x2000, CRC(f75a1539) SHA1(f139f6cb41351eb81ee47d777db03012aa5fadb1) )
ROM_END



GAME( 1985, yiear,  0,     yiear, yiear, 0, ROT0, "Konami", "Yie Ar Kung-Fu (set 1)", GAME_SUPPORTS_SAVE )
GAME( 1985, yiear2, yiear, yiear, yiear, 0, ROT0, "Konami", "Yie Ar Kung-Fu (set 2)", GAME_SUPPORTS_SAVE )
