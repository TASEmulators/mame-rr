/***************************************************************************

    Atari ThunderJaws hardware

    driver by Aaron Giles

    Games supported:
        * ThunderJaws (1990)

    Known bugs:
        * none at this time

****************************************************************************

    Memory map (TBA)

***************************************************************************/


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/atarigen.h"
#include "audio/atarijsa.h"
#include "includes/thunderj.h"



/*************************************
 *
 *  Initialization & interrupts
 *
 *************************************/

static void update_interrupts(running_machine *machine)
{
	thunderj_state *state = (thunderj_state *)machine->driver_data;
	cputag_set_input_line(machine, "maincpu", 4, state->atarigen.scanline_int_state ? ASSERT_LINE : CLEAR_LINE);
	cputag_set_input_line(machine, "extra", 4, state->atarigen.scanline_int_state ? ASSERT_LINE : CLEAR_LINE);
	cputag_set_input_line(machine, "maincpu", 6, state->atarigen.sound_int_state ? ASSERT_LINE : CLEAR_LINE);
}


static MACHINE_START( thunderj )
{
	thunderj_state *state = (thunderj_state *)machine->driver_data;
	atarigen_init(machine);

	state_save_register_global(machine, state->alpha_tile_bank);
}


static MACHINE_RESET( thunderj )
{
	thunderj_state *state = (thunderj_state *)machine->driver_data;

	atarigen_eeprom_reset(&state->atarigen);
	atarigen_interrupt_reset(&state->atarigen, update_interrupts);
	atarivc_reset(*machine->primary_screen, state->atarigen.atarivc_eof_data, 2);
	atarijsa_reset();
}



/*************************************
 *
 *  I/O handling
 *
 *************************************/

static READ16_HANDLER( special_port2_r )
{
	thunderj_state *state = (thunderj_state *)space->machine->driver_data;
	int result = input_port_read(space->machine, "260012");

	if (state->atarigen.sound_to_cpu_ready) result ^= 0x0004;
	if (state->atarigen.cpu_to_sound_ready) result ^= 0x0008;
	result ^= 0x0010;

	return result;
}


static WRITE16_HANDLER( latch_w )
{
	thunderj_state *state = (thunderj_state *)space->machine->driver_data;

	/* reset extra CPU */
	if (ACCESSING_BITS_0_7)
	{
		/* 0 means hold CPU 2's reset low */
		if (data & 1)
			cputag_set_input_line(space->machine, "extra", INPUT_LINE_RESET, CLEAR_LINE);
		else
			cputag_set_input_line(space->machine, "extra", INPUT_LINE_RESET, ASSERT_LINE);

		/* bits 2-5 are the alpha bank */
		if (state->alpha_tile_bank != ((data >> 2) & 7))
		{
			space->machine->primary_screen->update_partial(space->machine->primary_screen->vpos());
			tilemap_mark_all_tiles_dirty(state->atarigen.alpha_tilemap);
			state->alpha_tile_bank = (data >> 2) & 7;
		}
	}
}



/*************************************
 *
 *  Video Controller Hack
 *
 *************************************/

static READ16_HANDLER( thunderj_atarivc_r )
{
	/* Sigh. CPU #1 reads the video controller register twice per frame, once at
       the beginning of interrupt and once near the end. It stores these values in a
       table starting at $163484. CPU #2 periodically looks at this table to make
       sure that it is getting interrupts at the appropriate times, and that the
       VBLANK bit is set appropriately. Unfortunately, due to all the cpu_yield(space->cpu)
       calls we make to synchronize the two CPUs, we occasionally get out of time
       and generate the interrupt outside of the tight tolerances CPU #2 expects.

       So we fake it. Returning scanlines $f5 and $f7 alternately provides the
       correct answer that causes CPU #2 to be happy and not aggressively trash
       memory (which is what it does if this interrupt test fails -- see the code
       at $1E56 to see!) */

	/* Use these lines to detect when things go south: */

#if 0
	if (memory_read_word(space, 0x163482) > 0xfff)
		mame_printf_debug("You're screwed!");
#endif

	return atarivc_r(*space->machine->primary_screen, offset);
}


static WRITE16_HANDLER( thunderj_atarivc_w )
{
	atarivc_w(*space->machine->primary_screen, offset, data, mem_mask);
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x09ffff) AM_ROM
	AM_RANGE(0x0e0000, 0x0e0fff) AM_READWRITE(atarigen_eeprom_r, atarigen_eeprom_w) AM_BASE_SIZE_MEMBER(thunderj_state, atarigen.eeprom, atarigen.eeprom_size)
	AM_RANGE(0x160000, 0x16ffff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x1f0000, 0x1fffff) AM_WRITE(atarigen_eeprom_enable_w)
	AM_RANGE(0x260000, 0x26000f) AM_READ_PORT("260000")
	AM_RANGE(0x260010, 0x260011) AM_READ_PORT("260010")
	AM_RANGE(0x260012, 0x260013) AM_READ(special_port2_r)
	AM_RANGE(0x260030, 0x260031) AM_READ(atarigen_sound_r)
	AM_RANGE(0x2e0000, 0x2e0001) AM_WRITE(watchdog_reset16_w)
	AM_RANGE(0x360010, 0x360011) AM_WRITE(latch_w)
	AM_RANGE(0x360020, 0x360021) AM_WRITE(atarigen_sound_reset_w)
	AM_RANGE(0x360030, 0x360031) AM_WRITE(atarigen_sound_w)
	AM_RANGE(0x3e0000, 0x3e0fff) AM_RAM_WRITE(atarigen_666_paletteram_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x3effc0, 0x3effff) AM_READWRITE(thunderj_atarivc_r, thunderj_atarivc_w) AM_BASE_MEMBER(thunderj_state, atarigen.atarivc_data)
	AM_RANGE(0x3f0000, 0x3f1fff) AM_RAM_WRITE(atarigen_playfield2_latched_msb_w) AM_BASE_MEMBER(thunderj_state, atarigen.playfield2)
	AM_RANGE(0x3f2000, 0x3f3fff) AM_RAM_WRITE(atarigen_playfield_latched_lsb_w) AM_BASE_MEMBER(thunderj_state, atarigen.playfield)
	AM_RANGE(0x3f4000, 0x3f5fff) AM_RAM_WRITE(atarigen_playfield_dual_upper_w) AM_BASE_MEMBER(thunderj_state, atarigen.playfield_upper)
	AM_RANGE(0x3f6000, 0x3f7fff) AM_RAM_WRITE(atarimo_0_spriteram_w) AM_BASE(&atarimo_0_spriteram)
	AM_RANGE(0x3f8000, 0x3f8eff) AM_RAM_WRITE(atarigen_alpha_w) AM_BASE_MEMBER(thunderj_state, atarigen.alpha)
	AM_RANGE(0x3f8f00, 0x3f8f7f) AM_RAM AM_BASE_MEMBER(thunderj_state, atarigen.atarivc_eof_data)
	AM_RANGE(0x3f8f80, 0x3f8fff) AM_RAM_WRITE(atarimo_0_slipram_w) AM_BASE(&atarimo_0_slipram)
	AM_RANGE(0x3f9000, 0x3fffff) AM_RAM
ADDRESS_MAP_END



/*************************************
 *
 *  Extra CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( extra_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x060000, 0x07ffff) AM_ROM
	AM_RANGE(0x160000, 0x16ffff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x260000, 0x26000f) AM_READ_PORT("260000")
	AM_RANGE(0x260010, 0x260011) AM_READ_PORT("260010")
	AM_RANGE(0x260012, 0x260013) AM_READ(special_port2_r)
	AM_RANGE(0x260030, 0x260031) AM_READ(atarigen_sound_r)
	AM_RANGE(0x360000, 0x360001) AM_WRITE(atarigen_video_int_ack_w)
	AM_RANGE(0x360010, 0x360011) AM_WRITE(latch_w)
	AM_RANGE(0x360020, 0x360021) AM_WRITE(atarigen_sound_reset_w)
	AM_RANGE(0x360030, 0x360031) AM_WRITE(atarigen_sound_w)
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( thunderj )
	PORT_START("260000")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("260010")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0c00, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)

	PORT_START("260012")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_VBLANK )
	PORT_SERVICE( 0x0002, IP_ACTIVE_LOW )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNUSED )	/* Input buffer full (@260030) */
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )	/* Output buffer full (@360030) */
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x00e0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0c00, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)

	PORT_INCLUDE( atarijsa_ii )		/* audio board port */
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
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


static GFXDECODE_START( thunderj )
	GFXDECODE_ENTRY( "gfx1", 0, pfmolayout,  512,  96 )	/* sprites & playfield */
	GFXDECODE_ENTRY( "gfx2", 0, pfmolayout,  256, 112 )	/* sprites & playfield */
	GFXDECODE_ENTRY( "gfx3", 0, anlayout,      0, 512 )	/* characters 8x8 */
GFXDECODE_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_DRIVER_START( thunderj )
	MDRV_DRIVER_DATA(thunderj_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, ATARI_CLOCK_14MHz/2)
	MDRV_CPU_PROGRAM_MAP(main_map)

	MDRV_CPU_ADD("extra", M68000, ATARI_CLOCK_14MHz/2)
	MDRV_CPU_PROGRAM_MAP(extra_map)

	MDRV_MACHINE_START(thunderj)
	MDRV_MACHINE_RESET(thunderj)
	MDRV_NVRAM_HANDLER(atarigen)

	/* perfect synchronization due to shared RAM */
	MDRV_QUANTUM_PERFECT_CPU("maincpu")

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MDRV_GFXDECODE(thunderj)
	MDRV_PALETTE_LENGTH(2048)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	/* note: these parameters are from published specs, not derived */
	/* the board uses a VAD chip to generate video signals */
	MDRV_SCREEN_RAW_PARAMS(ATARI_CLOCK_14MHz/2, 456, 0, 336, 262, 0, 240)

	MDRV_VIDEO_START(thunderj)
	MDRV_VIDEO_UPDATE(thunderj)

	/* sound hardware */
	MDRV_IMPORT_FROM(jsa_ii_mono)
MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( thunderj )
	ROM_REGION( 0xa0000, "maincpu", 0 )	/* 10*64k for 68000 code */
	ROM_LOAD16_BYTE( "136076-2001.14e",   0x00000, 0x10000, CRC(f6a71532) SHA1(b1c55968d7da9b64bde737d66aa8f0ddcdcfee27) )
	ROM_LOAD16_BYTE( "136076-2002.14c",   0x00001, 0x10000, CRC(173ec10d) SHA1(e32eca9194336f3d7e289b2a187ed125ed03688c) )
	ROM_LOAD16_BYTE( "136076-2003.15e",   0x20000, 0x10000, CRC(6e155469) SHA1(ba87d0a510304fd8a0f91c81580c4f09fc4d1886) )
	ROM_LOAD16_BYTE( "136076-2004.15c",   0x20001, 0x10000, CRC(e9ff1e42) SHA1(416dd68ec2ae174a5b14fa6fb4fb88bc1afdda66) )
	ROM_LOAD16_BYTE( "136076-2005.16e",   0x40000, 0x10000, CRC(a40242e7) SHA1(ea2311f064885912054ae2e50a60664b216a6253) )
	ROM_LOAD16_BYTE( "136076-2006.16c",   0x40001, 0x10000, CRC(aa18b94c) SHA1(867fd1d5485eacb705ed4ec736ee8c2e78aa9bf4) )
	ROM_LOAD16_BYTE( "136076-1005.15h",   0x60000, 0x10000, CRC(05474ebb) SHA1(74a32dba5ffe2953c81ad9639d99ed01b31b0dba) )
	ROM_LOAD16_BYTE( "136076-1010.16h",   0x60001, 0x10000, CRC(ccff21c8) SHA1(7df8facf563cc1bb8de6cac6f2ddcc58ae0aa8b4) )
	ROM_LOAD16_BYTE( "136076-1007.17e",   0x80000, 0x10000, CRC(9c2a8aba) SHA1(10e4fc04e64bb6a5083a56f630224b5d1af241b2) )
	ROM_LOAD16_BYTE( "136076-1008.17c",   0x80001, 0x10000, CRC(22109d16) SHA1(8725696271c4a617f9f050d9d483fe4141bf1e00) )

	ROM_REGION( 0x80000, "extra", 0 )	/* 8*64k for 68000 code */
	ROM_LOAD16_BYTE( "136076-1011.17l",    0x00000, 0x10000, CRC(bbbbca45) SHA1(977e785e0272a84c8d7e28e25f45064d1b37aad1) )
	ROM_LOAD16_BYTE( "136076-1012.17n",    0x00001, 0x10000, CRC(53e5e638) SHA1(75593e5d328ede105b8db64005dd5d1c5cae11ed) )
	ROM_COPY( "maincpu", 0x60000, 0x60000, 0x20000 )

	ROM_REGION( 0x14000, "jsa", 0 )	/* 64k + 16k for 6502 code */
	ROM_LOAD( "136076-2015.1b", 0x10000, 0x4000, CRC(d8feb7fb) SHA1(684ebf2f0c0df742c98e7f45f74de86a11c8d6e8) )
	ROM_CONTINUE(               0x04000, 0xc000 )

	ROM_REGION( 0x100000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "136076-1021.5s",   0x000000, 0x10000, CRC(d8432766) SHA1(04e7d820974c0890fde1257b4710cf7b520d7d48) )	/* graphics, plane 0 */
	ROM_LOAD( "136076-1025.5r",   0x010000, 0x10000, CRC(839feed5) SHA1(c683ef5b78f8fd63dd557a630544f1e21aebe665) )
	ROM_LOAD( "136076-1029.3p",   0x020000, 0x10000, CRC(fa887662) SHA1(5d19022e8d40be86b85d0bcc28c97207ab9ec403) )
	ROM_LOAD( "136076-1033.6p",   0x030000, 0x10000, CRC(2addda79) SHA1(5a04c718055a5637b7549598ec39ca3cc9883698) )
	ROM_LOAD( "136076-1022.9s",   0x040000, 0x10000, CRC(dcf50371) SHA1(566e71e1dcb8e0266ca870af04b11f7bbee21b18) )	/* graphics, plane 1 */
	ROM_LOAD( "136076-1026.9r",   0x050000, 0x10000, CRC(216e72c8) SHA1(b6155584c8760c4dee3cf2a6320c53ea2161464b) )
	ROM_LOAD( "136076-1030.10s",  0x060000, 0x10000, CRC(dc51f606) SHA1(aa401808d915b2e6cdb17a1d58814a753648c9bb) )
	ROM_LOAD( "136076-1034.10r",  0x070000, 0x10000, CRC(f8e35516) SHA1(dcb23ed69f5a70ac842c6004039ec403bac68d72) )
	ROM_LOAD( "136076-1023.13s",  0x080000, 0x10000, CRC(b6dc3f13) SHA1(c3369b58012e02ad2fd85f1c9643ee5792f4b3de) )	/* graphics, plane 2 */
	ROM_LOAD( "136076-1027.13r",  0x090000, 0x10000, CRC(621cc2ce) SHA1(15db80d61f1c624c09085ed86341f8577bfac168) )
	ROM_LOAD( "136076-1031.14s",  0x0a0000, 0x10000, CRC(4682ceb5) SHA1(609ccd20f654982e01bcc6aea89801c01afe083e) )
	ROM_LOAD( "136076-1035.14r",  0x0b0000, 0x10000, CRC(7a0e1b9e) SHA1(b9a2270ee7e3b3dcf05a47085890d87bf5b3e167) )
	ROM_LOAD( "136076-1024.17s",  0x0c0000, 0x10000, CRC(d84452b5) SHA1(29bc994e37bc08fa40326b811339e7aa3290302c) )	/* graphics, plane 3 */
	ROM_LOAD( "136076-1028.17r",  0x0d0000, 0x10000, CRC(0cc20245) SHA1(ebdcb47909374508abe9d0252fd88d6274a0f729) )
	ROM_LOAD( "136076-1032.14p",  0x0e0000, 0x10000, CRC(f639161a) SHA1(cc2549f7fdd251fa44735a6cd5fdb8ffb97948be) )
	ROM_LOAD( "136076-1036.16p",  0x0f0000, 0x10000, CRC(b342443d) SHA1(fa7865f8a90c0e761e1cc5e155931d0574f2d81c) )

	ROM_REGION( 0x100000, "gfx2", ROMREGION_INVERT )
	ROM_LOAD( "136076-1037.2s",   0x000000, 0x10000, CRC(07addba6) SHA1(4a3286ee570bf4263944854bf959c8ef114cc123) )
	ROM_LOAD( "136076-1041.2r",   0x010000, 0x10000, CRC(1e9c29e4) SHA1(e4afa2c469bfa22504cba5dfd23704c5c2bb33c4) )
	ROM_LOAD( "136076-1045.34s",  0x020000, 0x10000, CRC(e7235876) SHA1(0bb03baec1de3e520dc270a3ed44bec953e08c00) )
	ROM_LOAD( "136076-1049.34r",  0x030000, 0x10000, CRC(a6eb8265) SHA1(f9b5fbe69b973327ebe5ea6e9fa782cb6db010d6) )
	ROM_LOAD( "136076-1038.6s",   0x040000, 0x10000, CRC(2ea543f9) SHA1(e5dafe023b1dc5068f367293da5774ab98ec617c) )
	ROM_LOAD( "136076-1042.6r",   0x050000, 0x10000, CRC(efabdc2b) SHA1(449fa8f229d901328aa14c1d093d10ba1c7e7cd9) )
	ROM_LOAD( "136076-1046.7s",   0x060000, 0x10000, CRC(6692151f) SHA1(294fb66f4a25ce0282b8f3b032df7a7103842540) )
	ROM_LOAD( "136076-1050.7r",   0x070000, 0x10000, CRC(ad7bb5f3) SHA1(1e7083cafd4c06397991cb873eeeb4b4c8a81d9d) )
	ROM_LOAD( "136076-1039.11s",  0x080000, 0x10000, CRC(cb563a40) SHA1(086619ae47c1c0b5fb1913b3e657216e021a3713) )
	ROM_LOAD( "136076-1043.11r",  0x090000, 0x10000, CRC(b7565eee) SHA1(610439f8e08fb88a646e76180bcd72ddfec8c06a) )
	ROM_LOAD( "136076-1047.12s",  0x0a0000, 0x10000, CRC(60877136) SHA1(ac7154b9324782c5c22e18c1d325f58c88938170) )
	ROM_LOAD( "136076-1051.12r",  0x0b0000, 0x10000, CRC(d4715ff0) SHA1(f0c2c7057d84b337ab1f44667e6048885922698e) )
	ROM_LOAD( "136076-1040.15s",  0x0c0000, 0x10000, CRC(6e910fc2) SHA1(29b811e0d8283dd00e2d79c2cbe120faa6834008) )
	ROM_LOAD( "136076-1044.15r",  0x0d0000, 0x10000, CRC(ff67a17a) SHA1(17f3572b526a14bdf3a6da5711c4d96feefc25aa) )
	ROM_LOAD( "136076-1048.16s",  0x0e0000, 0x10000, CRC(200d45b3) SHA1(72f1b3deaebc6266140fb79a97154f80f2cadf33) )
	ROM_LOAD( "136076-1052.16r",  0x0f0000, 0x10000, CRC(74711ef1) SHA1(c1429d6b54dc4352defdd6cf83f1a5734784e703) )

	ROM_REGION( 0x010000, "gfx3", 0 )
	ROM_LOAD( "136076-1020.4m",   0x000000, 0x10000, CRC(65470354) SHA1(9895d26fa9e01c254a3d15e657152cac717c68a3) )	/* alphanumerics */

	ROM_REGION( 0x40000, "adpcm", 0 )	/* 256k for ADPCM */
	ROM_LOAD( "136076-1016.7k",  0x00000, 0x10000, CRC(c10bdf73) SHA1(a0371c6ddef2a95193c68879044b3338d481fc96) )
	ROM_LOAD( "136076-1017.7j",  0x10000, 0x10000, CRC(4e5e25e8) SHA1(373c946abd24ce8dd5221f1a0409af4537610d3d) )
	ROM_LOAD( "136076-1018.7e",  0x20000, 0x10000, CRC(ec81895d) SHA1(56acffb0700d3b70ca705fba9d240a82950fd320) )
	ROM_LOAD( "136076-1019.7d",  0x30000, 0x10000, CRC(a4009037) SHA1(01cd3f4cf510f4956258f39f3ddbb42628bc2b9a) )

	ROM_REGION( 0x0e00, "plds", 0 )
	ROM_LOAD( "gal16v8a-136076-1070.12e", 0x0000, 0x0117, CRC(0e96e51e) SHA1(bf23d0089e5e34ad579cb56ff869353545c29792) )
	ROM_LOAD( "gal16v8a-136076-1071.13j", 0x0200, 0x0117, CRC(ad7beb34) SHA1(d50fa0526fd83b6a9a79adcacef56a64ebafa448) )
	ROM_LOAD( "gal16v8a-136076-1072.5n",  0x0400, 0x0117, CRC(2827620a) SHA1(25c5dfae3d8f4b73936833031b95fafd2ba0ae5d) )
	ROM_LOAD( "gal16v8a-136076-1073.2h",  0x0600, 0x0117, CRC(a2fe4402) SHA1(ef93eee062d51885504dcc58c34cad656e5b1ec4) )
	ROM_LOAD( "gal16v8a-136076-2074.3l",  0x0800, 0x0117, CRC(0e8bc5d7) SHA1(815ecd2cbb3469501f29172a4425bee930852c11) )
	ROM_LOAD( "gal20v8a-136076-1075.2l",  0x0a00, 0x0157, CRC(e75e1e86) SHA1(26d09b614a24bd736fd17a3145f9cc4d61a2d1a9) )
	ROM_LOAD( "gal20v8a-136076-1076.1l",  0x0c00, 0x0157, CRC(f36f873a) SHA1(63e6cc6e50370e89ebe33aa8d4ff9c6e1faeb9a6) )
ROM_END



/*************************************
 *
 *  Driver initialization
 *
 *************************************/

static DRIVER_INIT( thunderj )
{
	atarijsa_init(machine, "260012", 0x0002);
}



/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1990, thunderj, 0, thunderj, thunderj, thunderj, ROT0, "Atari Games", "ThunderJaws", 0 )
