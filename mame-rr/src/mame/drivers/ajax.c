/***************************************************************************

    "AJAX/Typhoon"  (Konami GX770)

    Driver by:
        Manuel Abadia <manu@teleline.es>

    TO DO:
    - Find the CPU core bug, that makes the 052001 to read from 0x0000

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m6809/m6809.h"
#include "cpu/konami/konami.h"
#include "video/konicdev.h"
#include "sound/2151intf.h"
#include "sound/k007232.h"
#include "includes/ajax.h"
#include "includes/konamipt.h"

static WRITE8_DEVICE_HANDLER( k007232_extvol_w );
static WRITE8_HANDLER( sound_bank_w );

/****************************************************************************/

static ADDRESS_MAP_START( ajax_main_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x01c0) AM_READWRITE(ajax_ls138_f10_r, ajax_ls138_f10_w)	/* bankswitch + sound command + FIRQ command */
	AM_RANGE(0x0800, 0x0807) AM_DEVREADWRITE("k051960", k051937_r, k051937_w)					/* sprite control registers */
	AM_RANGE(0x0c00, 0x0fff) AM_DEVREADWRITE("k051960", k051960_r, k051960_w)					/* sprite RAM 2128SL at J7 */
	AM_RANGE(0x1000, 0x1fff) AM_RAM_WRITE(paletteram_xBBBBBGGGGGRRRRR_be_w) AM_BASE_GENERIC(paletteram)/* palette */
	AM_RANGE(0x2000, 0x3fff) AM_RAM AM_SHARE("share1")									/* shared RAM with the 6809 */
	AM_RANGE(0x4000, 0x5fff) AM_RAM												/* RAM 6264L at K10 */
	AM_RANGE(0x6000, 0x7fff) AM_ROMBANK("bank2")										/* banked ROM */
	AM_RANGE(0x8000, 0xffff) AM_ROM												/* ROM N11 */
ADDRESS_MAP_END

static ADDRESS_MAP_START( ajax_sub_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_DEVREADWRITE("k051316", k051316_r, k051316_w)	/* 051316 zoom/rotation layer */
	AM_RANGE(0x0800, 0x080f) AM_DEVWRITE("k051316", k051316_ctrl_w)				/* 051316 control registers */
	AM_RANGE(0x1000, 0x17ff) AM_DEVREAD("k051316", k051316_rom_r)				/* 051316 (ROM test) */
	AM_RANGE(0x1800, 0x1800) AM_WRITE(ajax_bankswitch_2_w)			/* bankswitch control */
	AM_RANGE(0x2000, 0x3fff) AM_RAM AM_SHARE("share1")						/* shared RAM with the 052001 */
	AM_RANGE(0x4000, 0x7fff) AM_DEVREADWRITE("k052109", k052109_r, k052109_w)		/* video RAM + color RAM + video registers */
	AM_RANGE(0x8000, 0x9fff) AM_ROMBANK("bank1")							/* banked ROM */
	AM_RANGE(0xa000, 0xffff) AM_ROM									/* ROM I16 */
ADDRESS_MAP_END

static ADDRESS_MAP_START( ajax_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM								/* ROM F6 */
	AM_RANGE(0x8000, 0x87ff) AM_RAM								/* RAM 2128SL at D16 */
	AM_RANGE(0x9000, 0x9000) AM_WRITE(sound_bank_w)				/* 007232 bankswitch */
	AM_RANGE(0xa000, 0xa00d) AM_DEVREADWRITE("k007232_1", k007232_r, k007232_w)		/* 007232 registers (chip 1) */
	AM_RANGE(0xb000, 0xb00d) AM_DEVREADWRITE("k007232_2", k007232_r, k007232_w)		/* 007232 registers (chip 2) */
	AM_RANGE(0xb80c, 0xb80c) AM_DEVWRITE("k007232_2", k007232_extvol_w)			/* extra volume, goes to the 007232 w/ A11 */
																/* selecting a different latch for the external port */
	AM_RANGE(0xc000, 0xc001) AM_DEVREADWRITE("ymsnd", ym2151_r, ym2151_w)		/* YM2151 */
	AM_RANGE(0xe000, 0xe000) AM_READ(soundlatch_r)				/* soundlatch_r */
ADDRESS_MAP_END


static INPUT_PORTS_START( ajax )
	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "No Coin B", SW1)
	/* "No Coin B" = coins produce sound, but no effect on coin counter */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )		PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(	0x03, "2" )
	PORT_DIPSETTING(	0x02, "3" )
	PORT_DIPSETTING(	0x01, "5" )
	PORT_DIPSETTING(	0x00, "7" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )		PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(	0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(	0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x18, 0x10, DEF_STR( Bonus_Life ) )	PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(	0x18, "30000 150000" )
	PORT_DIPSETTING(	0x10, "50000 200000" )
	PORT_DIPSETTING(	0x08, "30000" )
	PORT_DIPSETTING(	0x00, "50000" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )	PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(	0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(	0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(	0x20, DEF_STR( Difficult ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )	PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(	0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )	PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(	0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Upright Controls" )		PORT_DIPLOCATION("SW3:2")	// Listed as "unused" and forced to be off in the manual.
	PORT_DIPSETTING(    0x02, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Dual ) )
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW3:3" )
	PORT_DIPNAME( 0x08, 0x08, "Control in 3D Stages" )	PORT_DIPLOCATION("SW3:4")	// The manual make reference to "general control"
	PORT_DIPSETTING(	0x08, DEF_STR( Normal ) )
	PORT_DIPSETTING(	0x00, "Inverted" )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")	/* COINSW & START */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )	/* service */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1")
	KONAMI8_B123_UNK(1)

	PORT_START("P2")
	KONAMI8_B123_UNK(2)
INPUT_PORTS_END



/*  sound_bank_w:
    Handled by the LS273 Octal +ve edge trigger D-type Flip-flop with Reset at B11:

    Bit Description
    --- -----------
    7   CONT1 (???) \
    6   CONT2 (???) / One or both bits are set to 1 when you kill a enemy
    5   \
    3   / 4MBANKH
    4   \
    2   / 4MBANKL
    1   \
    0   / 2MBANK
*/

static WRITE8_HANDLER( sound_bank_w )
{
	ajax_state *state = (ajax_state *)space->machine->driver_data;
	int bank_A, bank_B;

	/* banks # for the 007232 (chip 1) */
	bank_A = BIT(data, 1);
	bank_B = BIT(data, 0);
	k007232_set_bank(state->k007232_1, bank_A, bank_B);

	/* banks # for the 007232 (chip 2) */
	bank_A = ((data >> 4) & 0x03);
	bank_B = ((data >> 2) & 0x03);
	k007232_set_bank(state->k007232_2, bank_A, bank_B);
}

static void volume_callback0(running_device *device, int v)
{
	k007232_set_volume(device, 0, (v >> 4) * 0x11, 0);
	k007232_set_volume(device, 1, 0, (v & 0x0f) * 0x11);
}

static WRITE8_DEVICE_HANDLER( k007232_extvol_w )
{
	/* channel A volume (mono) */
	k007232_set_volume(device, 0, (data & 0x0f) * 0x11/2, (data & 0x0f) * 0x11/2);
}

static void volume_callback1(running_device *device, int v)
{
	/* channel B volume/pan */
	k007232_set_volume(device, 1, (v & 0x0f) * 0x11/2, (v >> 4) * 0x11/2);
}

static const k007232_interface k007232_interface_1 =
{
	volume_callback0
};

static const k007232_interface k007232_interface_2 =
{
	volume_callback1
};



static const k052109_interface ajax_k052109_intf =
{
	"gfx1", 0,
	NORMAL_PLANE_ORDER,
	KONAMI_ROM_DEINTERLEAVE_2,
	ajax_tile_callback
};

static const k051960_interface ajax_k051960_intf =
{
	"gfx2", 1,
	NORMAL_PLANE_ORDER,
	KONAMI_ROM_DEINTERLEAVE_2,
	ajax_sprite_callback
};

static const k051316_interface ajax_k051316_intf =
{
	"gfx3", 2,
	7, FALSE, 0,
	0, 0, 0,
	ajax_zoom_callback
};

static MACHINE_DRIVER_START( ajax )

	/* driver data */
	MDRV_DRIVER_DATA(ajax_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", KONAMI, 3000000)	/* 12/4 MHz*/
	MDRV_CPU_PROGRAM_MAP(ajax_main_map)
	MDRV_CPU_VBLANK_INT("screen", ajax_interrupt)	/* IRQs triggered by the 051960 */

	MDRV_CPU_ADD("sub", M6809, 3000000)	/* ? */
	MDRV_CPU_PROGRAM_MAP(ajax_sub_map)

	MDRV_CPU_ADD("audiocpu", Z80, 3579545)	/* 3.58 MHz */
	MDRV_CPU_PROGRAM_MAP(ajax_sound_map)

	MDRV_QUANTUM_TIME(HZ(600))

	MDRV_MACHINE_RESET(ajax)
	MDRV_MACHINE_START(ajax)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_HAS_SHADOWS)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(14*8, (64-14)*8-1, 2*8, 30*8-1 )
	MDRV_PALETTE_LENGTH(2048)

	MDRV_VIDEO_START(ajax)
	MDRV_VIDEO_UPDATE(ajax)

	MDRV_K052109_ADD("k052109", ajax_k052109_intf)
	MDRV_K051960_ADD("k051960", ajax_k051960_intf)
	MDRV_K051316_ADD("k051316", ajax_k051316_intf)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2151, 3579545)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)

	MDRV_SOUND_ADD("k007232_1", K007232, 3579545)
	MDRV_SOUND_CONFIG(k007232_interface_1)
	MDRV_SOUND_ROUTE(0, "lspeaker", 0.20)
	MDRV_SOUND_ROUTE(0, "rspeaker", 0.20)
	MDRV_SOUND_ROUTE(1, "lspeaker", 0.20)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.20)

	MDRV_SOUND_ADD("k007232_2", K007232, 3579545)
	MDRV_SOUND_CONFIG(k007232_interface_2)
	MDRV_SOUND_ROUTE(0, "lspeaker", 0.50)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.50)
MACHINE_DRIVER_END


/*

 This set is using 27512 Roms on a sub-board instead of Mask roms
 -- info from Phil Morris

 These are normally on the main board in the form of large mask ROMs, but at one stage
 the mask ROMs were unavailable so Konami had to provide a separate ROM board with
 36 x 27C512s instead.

*/

ROM_START( ajax )
	ROM_REGION( 0x28000, "maincpu", 0 )	/* 052001 code */
	ROM_LOAD( "770_m01.n11",	0x10000, 0x08000, CRC(4a64e53a) SHA1(acd249bfcb5f248c41b3e40c7c1bce1b8c645d3a) )	/* banked ROM */
	ROM_CONTINUE(				0x08000, 0x08000 )				/* fixed ROM */
	ROM_LOAD( "770_l02.n12",	0x18000, 0x10000, CRC(ad7d592b) SHA1(c75d9696b16de231c479379dd02d33fe54021d88) )	/* banked ROM */

	ROM_REGION( 0x22000, "sub", 0 )	/* 64k + 72k for banked ROMs */
	ROM_LOAD( "770_l05.i16",	0x20000, 0x02000, CRC(ed64fbb2) SHA1(429046edaf1299afa7fb9c385b4ef0c244ec2409) )	/* banked ROM */
	ROM_CONTINUE(				0x0a000, 0x06000 )				/* fixed ROM */
	ROM_LOAD( "770_f04.g16",	0x10000, 0x10000, CRC(e0e4ec9c) SHA1(15ae09c3ad67ec626d8178ec1417f0c57ca4eca4) )	/* banked ROM */

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* 64k for the SOUND CPU */
	ROM_LOAD( "770_h03.f16",	0x00000, 0x08000, CRC(2ffd2afc) SHA1(ca2ef684f87bcf9b70b3ec66ec80685edaf04b9b) )

	ROM_REGION( 0x080000, "gfx1", 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD16_BYTE( "770c13-a.f3",		0x000000, 0x010000, CRC(4ef6fff2) SHA1(0a2953f6907738b795d96184329431539386a463) )
	ROM_LOAD16_BYTE( "770c13-c.f4",		0x000001, 0x010000, CRC(97ffbab6) SHA1(97d9a39600eed918e12908a9abed0d4161c20ef6) )
	ROM_LOAD16_BYTE( "770c13-b.e3",		0x020000, 0x010000, CRC(86fdd706) SHA1(334c2720fc35aa556c6c5850d32f9bc9a6800fba) )
	ROM_LOAD16_BYTE( "770c13-d.e4",		0x020001, 0x010000, CRC(7d7acb2d) SHA1(3797743edf99201de928246e22e65ad17afe62f8) )
	ROM_LOAD16_BYTE( "770c12-a.f5",		0x040000, 0x010000, CRC(6c0ade68) SHA1(35e4548a37e19210c767ef2ed4c514dbde6806c2) )
	ROM_LOAD16_BYTE( "770c12-c.f6",		0x040001, 0x010000, CRC(61fc39cc) SHA1(34d0342ec0878590c289a66b39bde121cfadf00f) )
	ROM_LOAD16_BYTE( "770c12-b.e5",		0x060000, 0x010000, CRC(5f221cc6) SHA1(9a7a9c7853a3b582c4034b773cef08aee5391d6e) )
	ROM_LOAD16_BYTE( "770c12-d.e6",		0x060001, 0x010000, CRC(f1edb2f4) SHA1(3e66cc711e25cbf6e6a747d43a9efec0710d5b7a) )

	ROM_REGION( 0x100000, "gfx2", 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD16_BYTE( "770c09-a.f8",		0x000000, 0x010000, CRC(76690fb8) SHA1(afe267a37b65d63d3765dc3b88d8a8262446f786) )
	ROM_LOAD16_BYTE( "770c09-e.f9",		0x000001, 0x010000, CRC(17b482c9) SHA1(3535197956f5bf5b564fec1ddbb3e3ea3bf1f7bd) )
	ROM_LOAD16_BYTE( "770c09-b.e8",		0x020000, 0x010000, CRC(cd1709d1) SHA1(5a835639eb2d75adcfd0103b0800dd74b2bf9503) )
	ROM_LOAD16_BYTE( "770c09-f.e9",		0x020001, 0x010000, CRC(cba4b47e) SHA1(6ecb6283de4aa5ef8441db62b19200397f7734b3) )
	ROM_LOAD16_BYTE( "770c09-c.d8",		0x040000, 0x010000, CRC(bfd080b8) SHA1(83e186e08f442167e66575305930fa93f838faa6) )
	ROM_LOAD16_BYTE( "770c09-g.d9",		0x040001, 0x010000, CRC(77d58ea0) SHA1(8647c6920032e010b71ba4bc966ef6e1fd0a58a8) )
	ROM_LOAD16_BYTE( "770c09-d.c8",		0x060000, 0x010000, CRC(6f955600) SHA1(6f85adb633a670c8540b1e86d4bb6640829e74da) )
	ROM_LOAD16_BYTE( "770c09-h.c9",		0x060001, 0x010000, CRC(494a9090) SHA1(decd4442c206d1cd8f7741f2499aa3264b247d06) )
	ROM_LOAD16_BYTE( "770c08-a.f10",	0x080000, 0x010000, CRC(efd29a56) SHA1(2a9f138d1242a35162a3f092b0343dff899e3b83) )
	ROM_LOAD16_BYTE( "770c08-e.f11",	0x080001, 0x010000, CRC(6d43afde) SHA1(03d16125e7d082df08cd5e52a6694a1ddb765e4f) )
	ROM_LOAD16_BYTE( "770c08-b.e10",	0x0a0000, 0x010000, CRC(f3374014) SHA1(613c91e02fbf577668ea558c1893b845962368dd) )
	ROM_LOAD16_BYTE( "770c08-f.e11",	0x0a0001, 0x010000, CRC(f5ba59aa) SHA1(b65ea2ec20c2e9fa2e0dfe4c38d3d4f0b7160a97) )
	ROM_LOAD16_BYTE( "770c08-c.d10",	0x0c0000, 0x010000, CRC(28e7088f) SHA1(45c53a58bc6d2e70d5d20d5e6d58ec3e5bea3eeb) )
	ROM_LOAD16_BYTE( "770c08-g.d11",	0x0c0001, 0x010000, CRC(17da8f6d) SHA1(ba1d33d44cd50ff5d5a15b23d1a6153bc7b09579) )
	ROM_LOAD16_BYTE( "770c08-d.c10",	0x0e0000, 0x010000, CRC(91591777) SHA1(53f416a51f7075f070168bced7b6f925f54c7b84) )
	ROM_LOAD16_BYTE( "770c08-h.c11",	0x0e0001, 0x010000, CRC(d97d4b15) SHA1(e3d7d7adeec8c8c808acb9f84641fd3a6bf249be) )

	ROM_REGION( 0x080000, "gfx3", 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD( "770c06",		0x000000, 0x040000, CRC(d0c592ee) SHA1(c1be73dd259f2779d715659b177e47513776a0d4) )	/* zoom/rotate (F4) */
	ROM_LOAD( "770c07",		0x040000, 0x040000, CRC(0b399fb1) SHA1(fbe26f9aa9a655d08bebcdd79719d35134ca4dd5) )	/* zoom/rotate (H4) */

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "63s241.j11",	0x0000, 0x0200, CRC(9bdd719f) SHA1(de98e562080a97714047a8ad17abc6662c188897) )	/* priority encoder (not used) */

	ROM_REGION( 0x040000, "k007232_1", 0 )	/* 007232 data (chip 1) */
	ROM_LOAD( "770c10-a.a7",		0x000000, 0x010000, CRC(e45ec094) SHA1(540c56e1d778e6082db23aa3da64f6179b1f3635) )
	ROM_LOAD( "770c10-b.a6",		0x010000, 0x010000, CRC(349db7d3) SHA1(210da067038abeb021a77b3bf2664c9a49b3410a) )
	ROM_LOAD( "770c10-c.a5",		0x020000, 0x010000, CRC(71cb1f05) SHA1(57399806746b659f52114fb7bd4e11a7992a2c5d) )
	ROM_LOAD( "770c10-d.a4",		0x030000, 0x010000, CRC(e8ab1844) SHA1(dc22c4d11d6396a051398ba9ec6380aa3f856e71) )

	ROM_REGION( 0x080000, "k007232_2", 0 )	/* 007232 data (chip 2) */
	ROM_LOAD( "770c11-a.c6",		0x000000, 0x010000, CRC(8cccd9e0) SHA1(73e50a896ed212462046b7bfa04aad5e266425ca) )
	ROM_LOAD( "770c11-b.c5",		0x010000, 0x010000, CRC(0af2fedd) SHA1(038189210a73f668a0d913ff2dfc4ffa2e6bd5f4) )
	ROM_LOAD( "770c11-c.c4",		0x020000, 0x010000, CRC(7471f24a) SHA1(04d7a69ddc01017a773485fa891711d94c8ad47c) )
	ROM_LOAD( "770c11-d.c3",		0x030000, 0x010000, CRC(a58be323) SHA1(0401ede130cf9a529469bfb3dbcc8aee68e53243) )
	ROM_LOAD( "770c11-e.b7",		0x040000, 0x010000, CRC(dd553541) SHA1(96f36cb7b696f465005c7e7f1e4373b98a337864) )
	ROM_LOAD( "770c11-f.b6",		0x050000, 0x010000, CRC(3f78bd0f) SHA1(1d445c2b6460d6aac6f2acf0d5a5d73c31ba52e0) )
	ROM_LOAD( "770c11-g.b5",		0x060000, 0x010000, CRC(078c51b2) SHA1(6ad7ae8cda62023a286f5b4ac393ea0d02d20aeb) )
	ROM_LOAD( "770c11-h.b4",		0x070000, 0x010000, CRC(7300c2e1) SHA1(f9d23074701fb2127aed45d7cff91cc1cf8ce717) )
ROM_END

ROM_START( typhoon )
	ROM_REGION( 0x28000, "maincpu", 0 )	/* 052001 code */
	ROM_LOAD( "770_k01.n11",	0x10000, 0x08000, CRC(5ba74a22) SHA1(897d3309f2efb3bfa56e86581ee4a492e656788c) )	/* banked ROM */
	ROM_CONTINUE(				0x08000, 0x08000 )				/* fixed ROM */
	ROM_LOAD( "770_k02.n12",	0x18000, 0x10000, CRC(3bcf782a) SHA1(4b6127bced0b2519f8ad30587f32588a16368071) )	/* banked ROM */

	ROM_REGION( 0x22000, "sub", 0 )	/* 64k + 72k for banked ROMs */
	ROM_LOAD( "770_k05.i16",	0x20000, 0x02000, CRC(0f1bebbb) SHA1(012a8867ee0febaaadd7bcbc91e462bda5d3a411) )	/* banked ROM */
	ROM_CONTINUE(				0x0a000, 0x06000 )				/* fixed ROM */
	ROM_LOAD( "770_f04.g16",	0x10000, 0x10000, CRC(e0e4ec9c) SHA1(15ae09c3ad67ec626d8178ec1417f0c57ca4eca4) )	/* banked ROM */

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* 64k for the SOUND CPU */
	ROM_LOAD( "770_h03.f16",	0x00000, 0x08000, CRC(2ffd2afc) SHA1(ca2ef684f87bcf9b70b3ec66ec80685edaf04b9b) )

	ROM_REGION( 0x080000, "gfx1", 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD( "770c13",		0x000000, 0x040000, CRC(b859ca4e) SHA1(f58678d503683f78cca0d5ed2d79f6f68ab3495a) )	/* characters (N22) */
	ROM_LOAD( "770c12",		0x040000, 0x040000, CRC(50d14b72) SHA1(e3ff4a5aeefa6c10b5f7fec18297948b7c5acfdf) )	/* characters (K22) */

	ROM_REGION( 0x100000, "gfx2", 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD( "770c09",		0x000000, 0x080000, CRC(1ab4a7ff) SHA1(fa007b41027f95d29d2a9f931a2fe235844db637) )	/* sprites (N4) */
	ROM_LOAD( "770c08",		0x080000, 0x080000, CRC(a8e80586) SHA1(0401f59baa691905287cef94427f39e0c3f0adc6) )	/* sprites (K4) */

	ROM_REGION( 0x080000, "gfx3", 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD( "770c06",		0x000000, 0x040000, CRC(d0c592ee) SHA1(c1be73dd259f2779d715659b177e47513776a0d4) )	/* zoom/rotate (F4) */
	ROM_LOAD( "770c07",		0x040000, 0x040000, CRC(0b399fb1) SHA1(fbe26f9aa9a655d08bebcdd79719d35134ca4dd5) )	/* zoom/rotate (H4) */

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "63s241.j11",	0x0000, 0x0200, CRC(9bdd719f) SHA1(de98e562080a97714047a8ad17abc6662c188897) )	/* priority encoder (not used) */

	ROM_REGION( 0x040000, "k007232_1", 0 )	/* 007232 data (chip 1) */
	ROM_LOAD( "770c10",		0x000000, 0x040000, CRC(7fac825f) SHA1(581522d7a02dad16d2803ff344b4db133f767e6b) )

	ROM_REGION( 0x080000, "k007232_2", 0 )	/* 007232 data (chip 2) */
	ROM_LOAD( "770c11",		0x000000, 0x080000, CRC(299a615a) SHA1(29cdcc21998c72f4cf311792b904b79bde236bab) )
ROM_END

ROM_START( ajaxj )
	ROM_REGION( 0x28000, "maincpu", 0 )	/* 052001 code */
	ROM_LOAD( "770_l01.n11",	0x10000, 0x08000, CRC(7cea5274) SHA1(8e3b2b11a8189e3a1703b3b4b453fbb386f5537f) )	/* banked ROM */
	ROM_CONTINUE(				0x08000, 0x08000 )				/* fixed ROM */
	ROM_LOAD( "770_l02.n12",	0x18000, 0x10000, CRC(ad7d592b) SHA1(c75d9696b16de231c479379dd02d33fe54021d88) )	/* banked ROM */

	ROM_REGION( 0x22000, "sub", 0 )	/* 64k + 72k for banked ROMs */
	ROM_LOAD( "770_l05.i16",	0x20000, 0x02000, CRC(ed64fbb2) SHA1(429046edaf1299afa7fb9c385b4ef0c244ec2409) )	/* banked ROM */
	ROM_CONTINUE(				0x0a000, 0x06000 )				/* fixed ROM */
	ROM_LOAD( "770_f04.g16",	0x10000, 0x10000, CRC(e0e4ec9c) SHA1(15ae09c3ad67ec626d8178ec1417f0c57ca4eca4) )	/* banked ROM */

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* 64k for the SOUND CPU */
	ROM_LOAD( "770_f03.f16",	0x00000, 0x08000, CRC(3fe914fd) SHA1(c691920402bd859e2bf765084704a8bfad302cfa) )

	ROM_REGION( 0x080000, "gfx1", 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD( "770c13",		0x000000, 0x040000, CRC(b859ca4e) SHA1(f58678d503683f78cca0d5ed2d79f6f68ab3495a) )	/* characters (N22) */
	ROM_LOAD( "770c12",		0x040000, 0x040000, CRC(50d14b72) SHA1(e3ff4a5aeefa6c10b5f7fec18297948b7c5acfdf) )	/* characters (K22) */

	ROM_REGION( 0x100000, "gfx2", 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD( "770c09",		0x000000, 0x080000, CRC(1ab4a7ff) SHA1(fa007b41027f95d29d2a9f931a2fe235844db637) )	/* sprites (N4) */
	ROM_LOAD( "770c08",		0x080000, 0x080000, CRC(a8e80586) SHA1(0401f59baa691905287cef94427f39e0c3f0adc6) )	/* sprites (K4) */

	ROM_REGION( 0x080000, "gfx3", 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD( "770c06",		0x000000, 0x040000, CRC(d0c592ee) SHA1(c1be73dd259f2779d715659b177e47513776a0d4) )	/* zoom/rotate (F4) */
	ROM_LOAD( "770c07",		0x040000, 0x040000, CRC(0b399fb1) SHA1(fbe26f9aa9a655d08bebcdd79719d35134ca4dd5) )	/* zoom/rotate (H4) */

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "63s241.j11",	0x0000, 0x0200, CRC(9bdd719f) SHA1(de98e562080a97714047a8ad17abc6662c188897) )	/* priority encoder (not used) */

	ROM_REGION( 0x040000, "k007232_1", 0 )	/* 007232 data (chip 1) */
	ROM_LOAD( "770c10",		0x000000, 0x040000, CRC(7fac825f) SHA1(581522d7a02dad16d2803ff344b4db133f767e6b) )

	ROM_REGION( 0x080000, "k007232_2", 0 )	/* 007232 data (chip 2) */
	ROM_LOAD( "770c11",		0x000000, 0x080000, CRC(299a615a) SHA1(29cdcc21998c72f4cf311792b904b79bde236bab) )
ROM_END


GAME( 1987, ajax,    0,    ajax, ajax, 0, ROT90, "Konami", "Ajax", GAME_SUPPORTS_SAVE )
GAME( 1987, typhoon, ajax, ajax, ajax, 0, ROT90, "Konami", "Typhoon", GAME_SUPPORTS_SAVE )
GAME( 1987, ajaxj,   ajax, ajax, ajax, 0, ROT90, "Konami", "Ajax (Japan)", GAME_SUPPORTS_SAVE )
