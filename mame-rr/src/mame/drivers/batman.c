/***************************************************************************

    Atari Batman hardware

    driver by Aaron Giles

    Games supported:
        * Batman (1991)

    Known bugs:
        * none at this time

****************************************************************************

    Memory map (TBA)

***************************************************************************/


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "audio/atarijsa.h"
#include "includes/batman.h"



/*************************************
 *
 *  Initialization
 *
 *************************************/

static void update_interrupts(running_machine *machine)
{
	batman_state *state = (batman_state *)machine->driver_data;
	cputag_set_input_line(machine, "maincpu", 4, state->atarigen.scanline_int_state ? ASSERT_LINE : CLEAR_LINE);
	cputag_set_input_line(machine, "maincpu", 6, state->atarigen.sound_int_state ? ASSERT_LINE : CLEAR_LINE);
}


static MACHINE_START( batman )
{
	batman_state *state = (batman_state *)machine->driver_data;
	atarigen_init(machine);

	state_save_register_global(machine, state->latch_data);
	state_save_register_global(machine, state->alpha_tile_bank);
}


static MACHINE_RESET( batman )
{
	batman_state *state = (batman_state *)machine->driver_data;

	atarigen_eeprom_reset(&state->atarigen);
	atarigen_interrupt_reset(&state->atarigen, update_interrupts);
	atarivc_reset(*machine->primary_screen, state->atarigen.atarivc_eof_data, 2);
	atarigen_scanline_timer_reset(*machine->primary_screen, batman_scanline_update, 8);
	atarijsa_reset();
}



/*************************************
 *
 *  Video controller access
 *
 *************************************/

static READ16_HANDLER( batman_atarivc_r )
{
	return atarivc_r(*space->machine->primary_screen, offset);
}


static WRITE16_HANDLER( batman_atarivc_w )
{
	atarivc_w(*space->machine->primary_screen, offset, data, mem_mask);
}



/*************************************
 *
 *  I/O handling
 *
 *************************************/

static READ16_HANDLER( special_port2_r )
{
	batman_state *state = (batman_state *)space->machine->driver_data;
	int result = input_port_read(space->machine, "260010");
	if (state->atarigen.sound_to_cpu_ready) result ^= 0x0010;
	if (state->atarigen.cpu_to_sound_ready) result ^= 0x0020;
	return result;
}


static WRITE16_HANDLER( latch_w )
{
	batman_state *state = (batman_state *)space->machine->driver_data;
	int oldword = state->latch_data;
	COMBINE_DATA(&state->latch_data);

	/* bit 4 is connected to the /RESET pin on the 6502 */
	if (state->latch_data & 0x0010)
		cputag_set_input_line(space->machine, "jsa", INPUT_LINE_RESET, CLEAR_LINE);
	else
		cputag_set_input_line(space->machine, "jsa", INPUT_LINE_RESET, ASSERT_LINE);

	/* alpha bank is selected by the upper 4 bits */
	if ((oldword ^ state->latch_data) & 0x7000)
	{
		space->machine->primary_screen->update_partial(space->machine->primary_screen->vpos());
		tilemap_mark_all_tiles_dirty(state->atarigen.alpha_tilemap);
		state->alpha_tile_bank = (state->latch_data >> 12) & 7;
	}
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

/* full map verified from schematics and GALs */
/* addresses in the 1xxxxx region map to /WAIT */
/* addresses in the 2xxxxx region map to /WAIT2 */
static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 16 )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0x3fffff)
	AM_RANGE(0x000000, 0x0bffff) AM_ROM
	AM_RANGE(0x100000, 0x10ffff) AM_MIRROR(0x010000) AM_RAM
	AM_RANGE(0x120000, 0x120fff) AM_MIRROR(0x01f000) AM_READWRITE(atarigen_eeprom_r, atarigen_eeprom_w) AM_BASE_SIZE_MEMBER(batman_state, atarigen.eeprom, atarigen.eeprom_size)
	AM_RANGE(0x260000, 0x260001) AM_MIRROR(0x11ff8c) AM_READ_PORT("260000")
	AM_RANGE(0x260002, 0x260003) AM_MIRROR(0x11ff8c) AM_READ_PORT("260002")
	AM_RANGE(0x260010, 0x260011) AM_MIRROR(0x11ff8e) AM_READ(special_port2_r)
	AM_RANGE(0x260030, 0x260031) AM_MIRROR(0x11ff8e) AM_READ(atarigen_sound_r)
	AM_RANGE(0x260040, 0x260041) AM_MIRROR(0x11ff8e) AM_WRITE(atarigen_sound_w)
	AM_RANGE(0x260050, 0x260051) AM_MIRROR(0x11ff8e) AM_WRITE(latch_w)
	AM_RANGE(0x260060, 0x260061) AM_MIRROR(0x11ff8e) AM_WRITE(atarigen_eeprom_enable_w)
	AM_RANGE(0x2a0000, 0x2a0001) AM_MIRROR(0x11fffe) AM_WRITE(watchdog_reset16_w)
	AM_RANGE(0x3e0000, 0x3e0fff) AM_MIRROR(0x100000) AM_RAM_WRITE(atarigen_666_paletteram_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x3effc0, 0x3effff) AM_MIRROR(0x100000) AM_READWRITE(batman_atarivc_r, batman_atarivc_w) AM_BASE_MEMBER(batman_state, atarigen.atarivc_data)
	AM_RANGE(0x3f0000, 0x3f1fff) AM_MIRROR(0x100000) AM_WRITE(atarigen_playfield2_latched_msb_w) AM_BASE_MEMBER(batman_state, atarigen.playfield2)
	AM_RANGE(0x3f2000, 0x3f3fff) AM_MIRROR(0x100000) AM_WRITE(atarigen_playfield_latched_lsb_w) AM_BASE_MEMBER(batman_state, atarigen.playfield)
	AM_RANGE(0x3f4000, 0x3f5fff) AM_MIRROR(0x100000) AM_WRITE(atarigen_playfield_dual_upper_w) AM_BASE_MEMBER(batman_state, atarigen.playfield_upper)
	AM_RANGE(0x3f6000, 0x3f7fff) AM_MIRROR(0x100000) AM_WRITE(atarimo_0_spriteram_w) AM_BASE(&atarimo_0_spriteram)
	AM_RANGE(0x3f8000, 0x3f8fef) AM_MIRROR(0x100000) AM_WRITE(atarigen_alpha_w) AM_BASE_MEMBER(batman_state, atarigen.alpha)
	AM_RANGE(0x3f8f00, 0x3f8f7f) AM_MIRROR(0x100000) AM_BASE_MEMBER(batman_state, atarigen.atarivc_eof_data)
	AM_RANGE(0x3f8f80, 0x3f8fff) AM_MIRROR(0x100000) AM_WRITE(atarimo_0_slipram_w) AM_BASE(&atarimo_0_slipram)
	AM_RANGE(0x3f0000, 0x3fffff) AM_MIRROR(0x100000) AM_RAM
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( batman )
	PORT_START("260000")		/* 260000 */
	PORT_BIT( 0x01ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)

	PORT_START("260002")		/* 260002 */
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("260010")		/* 260010 */
	PORT_BIT( 0x000f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNUSED )	/* Input buffer full (@260030) */
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )	/* Output buffer full (@260040) */
	PORT_SERVICE( 0x0040, IP_ACTIVE_LOW )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_VBLANK )

	PORT_INCLUDE( atarijsa_iii )		/* audio board port */
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
	{ RGN_FRAC(0,4), RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


static GFXDECODE_START( batman )
	GFXDECODE_ENTRY( "gfx3", 0, pfmolayout,  512, 64 )		/* sprites & playfield */
	GFXDECODE_ENTRY( "gfx2", 0, pfmolayout,  256, 64 )		/* sprites & playfield */
	GFXDECODE_ENTRY( "gfx1", 0, anlayout,      0, 64 )		/* characters 8x8 */
GFXDECODE_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_DRIVER_START( batman )
	MDRV_DRIVER_DATA(batman_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, ATARI_CLOCK_14MHz)
	MDRV_CPU_PROGRAM_MAP(main_map)

	MDRV_MACHINE_START(batman)
	MDRV_MACHINE_RESET(batman)
	MDRV_NVRAM_HANDLER(atarigen)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MDRV_GFXDECODE(batman)
	MDRV_PALETTE_LENGTH(2048)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	/* note: these parameters are from published specs, not derived */
	/* the board uses a VAD chip to generate video signals */
	MDRV_SCREEN_RAW_PARAMS(ATARI_CLOCK_14MHz/2, 456, 0, 336, 262, 0, 240)

	MDRV_VIDEO_START(batman)
	MDRV_VIDEO_UPDATE(batman)

	/* sound hardware */
	MDRV_IMPORT_FROM(jsa_iii_mono)
MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( batman )
	ROM_REGION( 0xc0000, "maincpu", 0 )	/* 6*128k for 68000 code */
	ROM_LOAD16_BYTE( "136085-2030.10r",  0x00000, 0x20000, CRC(7cf4e5bf) SHA1(d6068a65fb524d839a34e596a272fac1ce90981c) )
	ROM_LOAD16_BYTE( "136085-2031.7r",   0x00001, 0x20000, CRC(7d7f3fc4) SHA1(8ee3e9ad3464006a26c36155b6099433110e2a6e) )
	ROM_LOAD16_BYTE( "136085-2032.91r",  0x40000, 0x20000, CRC(d80afb20) SHA1(5696627f6fa713dba4d12443c945f3e1cb6452a3) )
	ROM_LOAD16_BYTE( "136085-2033.6r",   0x40001, 0x20000, CRC(97efa2b8) SHA1(782e263ca22356c1747c50aed158d8c459364ad8) )
	ROM_LOAD16_BYTE( "136085-2034.9r",   0x80000, 0x20000, CRC(05388c62) SHA1(de037203d94e72e2922c89256da080ae023ca0e7) )
	ROM_LOAD16_BYTE( "136085-2035.5r",   0x80001, 0x20000, CRC(e77c92dd) SHA1(6d475092f7628114960d26b8ec1c5eae5e61ce25) )

	ROM_REGION( 0x14000, "jsa", 0 )	/* 64k + 16k for 6502 code */
	ROM_LOAD( "136085-1040.12c",  0x10000, 0x4000, CRC(080db83c) SHA1(ec084b7c1dc5878acd6d081e2e8b8d1e8b3d8a45) )
	ROM_CONTINUE(                 0x04000, 0xc000 )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "136085-2009.10m",  0x00000, 0x20000, CRC(a82d4923) SHA1(38e03eebd95347a383f3d7357462252961bd3c7f) )	/* alphanumerics */

	ROM_REGION( 0x100000, "gfx2", ROMREGION_INVERT )
	ROM_LOAD( "136085-1010.13r",  0x000000, 0x20000, CRC(466e1365) SHA1(318530e8a97c1b6ee3e671e677fc7684df5cc3a8) )	/* graphics, plane 0 */
	ROM_LOAD( "136085-1014.14r",  0x020000, 0x20000, CRC(ef53475a) SHA1(9bfc66bb8dd02757e4a79a75928b260f4518a94b) )
	ROM_LOAD( "136085-1011.13m",  0x040000, 0x20000, CRC(8cda5efc) SHA1(b0410f9bf1f38f5f1e9add15079b03d7f19b4c8f) )	/* graphics, plane 1 */
	ROM_LOAD( "136085-1015.14m",  0x060000, 0x20000, CRC(043e7f8b) SHA1(a3b2c539c1fa9b1e8d3dc8163b9a7c6e22408122) )
	ROM_LOAD( "136085-1012.13f",  0x080000, 0x20000, CRC(b017f2c3) SHA1(12846f0ae33e808dfb0795ea4138115b0eb36c4e) )	/* graphics, plane 2 */
	ROM_LOAD( "136085-1016.14f",  0x0a0000, 0x20000, CRC(70aa2360) SHA1(5b944b533be40b859b7fae64559286034409ac6c) )
	ROM_LOAD( "136085-1013.13c",  0x0c0000, 0x20000, CRC(68b64975) SHA1(f3fb45dd74fc21dd2eece87e739c734963962f93) )	/* graphics, plane 3 */
	ROM_LOAD( "136085-1017.14c",  0x0e0000, 0x20000, CRC(e4af157b) SHA1(ddf9eff84c882a096f7e435a6227b32d31029f9e) )

	ROM_REGION( 0x100000, "gfx3", ROMREGION_INVERT )
	ROM_LOAD( "136085-1018.15r",  0x000000, 0x20000, CRC(4c14f1e5) SHA1(2a65d0aafd944886d9e801c9de0f857f2e9db773) )
	ROM_LOAD( "136085-1022.16r",  0x020000, 0x20000, CRC(7476a15d) SHA1(779f9aec114aa71a268a7a646d998c1593f55d08) )
	ROM_LOAD( "136085-1019.15m",  0x040000, 0x20000, CRC(2046d9ec) SHA1(3b14b18545eac2e6cb1d3157ec1af251287b3e45) )
	ROM_LOAD( "136085-1023.16m",  0x060000, 0x20000, CRC(75cac686) SHA1(f3b1f485e51cc4af5809ec3fa2e7353fe9acb18f) )
	ROM_LOAD( "136085-1020.15f",  0x080000, 0x20000, CRC(cc4f4b94) SHA1(b8a11dbe436496898c8b6d64163a3a92eb843086) )
	ROM_LOAD( "136085-1024.16f",  0x0a0000, 0x20000, CRC(d60d35e0) SHA1(d74752090aec9fe0b7a67f62ae3024da74a004e3) )
	ROM_LOAD( "136085-1021.15c",  0x0c0000, 0x20000, CRC(9c8ef9ba) SHA1(c2540adfc227a654a3f91e2cfdcd98b3a04ae4fb) )
	ROM_LOAD( "136085-1025.16c",  0x0e0000, 0x20000, CRC(5d30bcd1) SHA1(817e225511ab98e7575ee512d659c51fcb7716dc) )

	ROM_REGION( 0x80000, "adpcm", 0 )	/* 1MB for ADPCM */
	ROM_LOAD( "136085-1041.19e",  0x00000, 0x20000, CRC(d97d5dbb) SHA1(7609841c773e3d1ae5a21da81e3387260fd8da41) )
	ROM_LOAD( "136085-1042.17e",  0x20000, 0x20000, CRC(8c496986) SHA1(07c84c68885e2ab3e81ee92942d6a0f29e4dffa8) )
	ROM_LOAD( "136085-1043.15e",  0x40000, 0x20000, CRC(51812d3b) SHA1(6748fecef753179a9257c0da5a7b7c9648437208) )
	ROM_LOAD( "136085-1044.12e",  0x60000, 0x20000, CRC(5e2d7f31) SHA1(737c7204d91f5dd5c9ed0321fc6c0d6194a18f8a) )

	ROM_REGION( 0x1000, "plds", 0 )
	ROM_LOAD( "gal16v8a-136085-1001.m9",  0x0000, 0x0117, CRC(45dfc0cf) SHA1(39cbb27e504e09d97caea144bfdec2247a39caf9) )
	ROM_LOAD( "gal16v8a-136085-1002.l9",  0x0200, 0x0117, CRC(35c221ae) SHA1(6f3241fcd8b7e241036a0f553f118d8aec413732) )
	ROM_LOAD( "gal20v8a-136085-1003.c9",  0x0400, 0x0157, CRC(cbfb2b4f) SHA1(1566b2cf0a3e1cc982e0d9262ad7be3f3e452787) )
	ROM_LOAD( "gal20v8a-136085-1004.b9",  0x0600, 0x0157, CRC(227cd23a) SHA1(5592c08b0635f7aef233b7836cfe6b419d6123f2) )
	ROM_LOAD( "gal16v8a-136085-1005.d11", 0x0800, 0x0117, CRC(a2fe4402) SHA1(ef93eee062d51885504dcc58c34cad656e5b1ec4) )
	ROM_LOAD( "gal16v8a-136085-1006.d9",  0x0a00, 0x0117, CRC(87d1c2dc) SHA1(bfb0c45b63cd100a757d777a43202d4555036ef4) )
	ROM_LOAD( "gal16v8a-136085-1038.c17", 0x0c00, 0x0117, CRC(0a591138) SHA1(12137f30428fcb952bb6feb37a54451ad683d3b0) )
	ROM_LOAD( "gal16v8a-136085-1039.c19", 0x0e00, 0x0117, CRC(47565e09) SHA1(7aeac8de6f9fda23da50655352b845f692ab0747) )
ROM_END



/*************************************
 *
 *  Driver initialization
 *
 *************************************/

static DRIVER_INIT( batman )
{
	static const UINT16 default_eeprom[] =
	{
		0x0001,0x01F1,0x0154,0x01C5,0x0100,0x0113,0x0300,0x0173,
		0x0700,0x0154,0x0200,0x0107,0x0100,0x0120,0x0300,0x0165,
		0x0125,0x0100,0x0149,0x019D,0x016C,0x018B,0x01F1,0x0154,
		0x01C5,0x0100,0x0113,0x0300,0x0173,0x0700,0x0154,0x0200,
		0x0107,0x0100,0x0120,0x0300,0x0165,0x0125,0x0100,0x0149,
		0x019D,0x016C,0x018B,0x6800,0x0134,0x0113,0x0148,0x0100,
		0x019A,0x0105,0x01DC,0x01A2,0x013A,0x0139,0x0100,0x0105,
		0x01AB,0x016A,0x0149,0x0100,0x01ED,0x0105,0x0185,0x01B2,
		0x0134,0x0100,0x0105,0x0160,0x01AA,0x0149,0x0100,0x0105,
		0x012A,0x0152,0x0110,0x0100,0x0168,0x0105,0x0113,0x012E,
		0x0150,0x0218,0x01D0,0x0100,0x01D0,0x0300,0x01D0,0x0600,
		0x01D0,0x02C8,0x0000
	};
	batman_state *state = (batman_state *)machine->driver_data;
	state->atarigen.eeprom_default = default_eeprom;
	atarijsa_init(machine, "260010", 0x0040);
}



/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1991, batman, 0, batman, batman, batman, ROT0, "Atari Games", "Batman", GAME_SUPPORTS_SAVE )
