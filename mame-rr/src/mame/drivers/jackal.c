/***************************************************************************

  jackal.c

  Written by Kenneth Lin (kenneth_lin@ai.vancouver.bc.ca)

Notes:
- This game uses two 005885 gfx chip in parallel. The unique thing about it is
  that the two 4bpp tilemaps from the two chips are merged to form a single
  8bpp tilemap.
- topgunbl is derived from a completely different version, which supports gun
  turret rotation. The copyright year is also different, but this doesn't
  necessarily mean anything.

TODO:
- Coin counters don't work correctly, because the register is overwritten by
  other routines and the coin counter bits rapidly toggle between 0 and 1.
- running the sound CPU at the nominal clock rate, music stops working at the
  beginning of the game. This is kludged by overclocking the sound CPU. This
  looks like a CPU communication timing issue however fiddling with the
  interleave factor has no effect.


Memory Map
----------

MAIN CPU:

Address range 00xxxxxxxxxx---- is handled by the 007343 custom so layout is
inferred by program behaviour. Note that address lines A8 and A9 are ORed
together and go to the single A8.9 input of the 007343.

Address          Dir Data     Description
---------------- --- -------- -----------------------
00000000000000xx R/W xxxxxxxx 005885 registers
0000000000000100 R/W xxxxxxxx 005885 registers
0000000000010000 R   xxxxxxxx DIPSW1
0000000000010001 R   xxxxxxxx P1 inputs + DIPSW3.4
0000000000010010 R   xxxxxxxx P2 inputs
0000000000010011 R   xxxxxxxx Coin inputs + DIPSW3.1-3
00000000000101-0 R   xxxxxxxx P1 extra inputs (only used by the bootleg for the rotary control)
00000000000101-1 R   xxxxxxxx P2 extra inputs (only used by the bootleg for the rotary control)
00000000000110-0 R   xxxxxxxx DIPSW2
00000000000111-0   W -------x Coin Counter 1 (to 005924 OUT1 input)
                   W ------x- Coin Counter 2 (to 005924 OUT2 input)
                   W -----x-- unknown ("END", to connector SVCN4P pin 4)
                   W ----x--- sprite RAM bank (to 007343 OBJB input)
                   W ---x---- 005885 select (to 007343 GATEB input)
                   W --x----- ROM bank
00000000000111-1 R/W -------- Watchdog reset (to 005924 AFR input)
00000000001xxxxx R/W xxxxxxxx scroll RAM (005885)
00000000010xxxxx R/W xxxxxxxx Z RAM (005885)
000xxxxxxxxxxxxx R/W xxxxxxxx RAM (shared with sound CPU--note that addresses 0000-005F are handled above so they are excluded)
0010xxxxxxxxxxxx R/W xxxxxxxx video RAM (005885)
0011xxxxxxxxxxxx R/W xxxxxxxx sprite RAM (005885)
01xxxxxxxxxxxxxx R   xxxxxxxx ROM (banked)
10xxxxxxxxxxxxxx R   xxxxxxxx ROM (banked)
11xxxxxxxxxxxxxx R   xxxxxxxx ROM


SOUND CPU:

Address          Dir Data     Description
---------------- --- -------- -----------------------
000-------------              n.c.
001------------x R/W xxxxxxxx YM2151
010-xxxxxxxxxxxx R/W xxxxxxxx 007327 (palette)
011xxxxxxxxxxxxx R/W xxxxxxxx RAM (shared with main CPU)
1xxxxxxxxxxxxxxx R   xxxxxxxx ROM

***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "sound/2151intf.h"
#include "includes/jackal.h"
#include "includes/konamipt.h"


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

static READ8_HANDLER( topgunbl_rotary_r )
{
	return (1 << input_port_read_safe(space->machine, offset ? "DIAL1" : "DIAL0", 0x00)) ^ 0xff;
}

static WRITE8_HANDLER( jackal_flipscreen_w )
{
	jackal_state *state = (jackal_state *)space->machine->driver_data;
	state->irq_enable = data & 0x02;
	flip_screen_set(space->machine, data & 0x08);
}

static READ8_HANDLER( jackal_zram_r )
{
	jackal_state *state = (jackal_state *)space->machine->driver_data;
	return state->rambank[0x0020 + offset];
}


static READ8_HANDLER( jackal_voram_r )
{
	jackal_state *state = (jackal_state *)space->machine->driver_data;
	return state->rambank[0x2000 + offset];
}


static READ8_HANDLER( jackal_spriteram_r )
{
	jackal_state *state = (jackal_state *)space->machine->driver_data;
	return state->spritebank[0x3000 + offset];
}


static WRITE8_HANDLER( jackal_rambank_w )
{
	jackal_state *state = (jackal_state *)space->machine->driver_data;
	UINT8 *rgn = memory_region(space->machine, "master");

	if (data & 0x04)
		popmessage("jackal_rambank_w %02x", data);

	coin_counter_w(space->machine, 0, data & 0x01);
	coin_counter_w(space->machine, 1, data & 0x02);

	state->spritebank = &rgn[((data & 0x08) << 13)];
	state->rambank = &rgn[((data & 0x10) << 12)];
	memory_set_bank(space->machine, "bank1", (data & 0x20) ? 1 : 0);
}


static WRITE8_HANDLER( jackal_zram_w )
{
	jackal_state *state = (jackal_state *)space->machine->driver_data;
	state->rambank[0x0020 + offset] = data;
}


static WRITE8_HANDLER( jackal_voram_w )
{
	jackal_state *state = (jackal_state *)space->machine->driver_data;

	if ((offset & 0xf800) == 0)
		jackal_mark_tile_dirty(space->machine, offset & 0x3ff);

	state->rambank[0x2000 + offset] = data;
}


static WRITE8_HANDLER( jackal_spriteram_w )
{
	jackal_state *state = (jackal_state *)space->machine->driver_data;
	state->spritebank[0x3000 + offset] = data;
}

/*************************************
 *
 *  Address maps
 *
 *************************************/

static ADDRESS_MAP_START( master_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0003) AM_RAM AM_BASE_MEMBER(jackal_state, videoctrl)	// scroll + other things
	AM_RANGE(0x0004, 0x0004) AM_WRITE(jackal_flipscreen_w)
	AM_RANGE(0x0010, 0x0010) AM_READ_PORT("DSW1")
	AM_RANGE(0x0011, 0x0011) AM_READ_PORT("IN1")
	AM_RANGE(0x0012, 0x0012) AM_READ_PORT("IN2")
	AM_RANGE(0x0013, 0x0013) AM_READ_PORT("IN0")
	AM_RANGE(0x0014, 0x0015) AM_READ(topgunbl_rotary_r)
	AM_RANGE(0x0018, 0x0018) AM_READ_PORT("DSW2")
	AM_RANGE(0x0019, 0x0019) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x001c, 0x001c) AM_WRITE(jackal_rambank_w)
	AM_RANGE(0x0020, 0x005f) AM_READWRITE(jackal_zram_r, jackal_zram_w)				// MAIN   Z RAM,SUB    Z RAM
	AM_RANGE(0x0060, 0x1fff) AM_RAM AM_SHARE("share1")							// M COMMON RAM,S COMMON RAM
	AM_RANGE(0x2000, 0x2fff) AM_READWRITE(jackal_voram_r, jackal_voram_w)			// MAIN V O RAM,SUB  V O RAM
	AM_RANGE(0x3000, 0x3fff) AM_READWRITE(jackal_spriteram_r, jackal_spriteram_w)	// MAIN V O RAM,SUB  V O RAM
	AM_RANGE(0x4000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( slave_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x2000, 0x2001) AM_DEVREADWRITE("ymsnd", ym2151_r, ym2151_w)
	AM_RANGE(0x4000, 0x43ff) AM_RAM AM_BASE_MEMBER(jackal_state, paletteram)	// self test only checks 0x4000-0x423f, 007327 should actually go up to 4fff
	AM_RANGE(0x6000, 0x605f) AM_RAM						// SOUND RAM (Self test check 0x6000-605f, 0x7c00-0x7fff)
	AM_RANGE(0x6060, 0x7fff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( jackal )
	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "No Coin B", SW1)
	/* "No Coin B" = coins produce sound, but no effect on coin counter */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) ) PORT_DIPLOCATION( "SW2:1,2" )
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) ) PORT_DIPLOCATION( "SW2:3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION( "SW2:4,5" )
	PORT_DIPSETTING(    0x18, "30K 150K" )
	PORT_DIPSETTING(    0x10, "50K 200K" )
	PORT_DIPSETTING(    0x08, "30K" )
	PORT_DIPSETTING(    0x00, "50K" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) ) PORT_DIPLOCATION( "SW2:6,7" )
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION( "SW2:8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN0")
	KONAMI8_SYSTEM_10
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION( "SW3:1" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Sound Adjustment" ) PORT_DIPLOCATION( "SW3:2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x00, "Sound Mode" ) PORT_DIPLOCATION( "SW3:3" )
	PORT_DIPSETTING(    0x80, DEF_STR( Mono ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Stereo ) )

	PORT_START("IN1")
	KONAMI8_B12(1)
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) ) PORT_DIPLOCATION( "SW3:4" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	KONAMI8_B12_UNK(2)
INPUT_PORTS_END

static INPUT_PORTS_START( topgunbl )
	PORT_INCLUDE(jackal)

	PORT_MODIFY("IN0")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DIAL0")	// player 1 8-way rotary control - converted in topgunbl_rotary_r()
	PORT_BIT( 0xff, 0x00, IPT_POSITIONAL ) PORT_POSITIONS(8) PORT_WRAPS PORT_SENSITIVITY(15) PORT_KEYDELTA(1) PORT_CODE_DEC(KEYCODE_Z) PORT_CODE_INC(KEYCODE_X) PORT_FULL_TURN_COUNT(8)

	PORT_START("DIAL1")	// player 2 8-way rotary control - converted in topgunbl_rotary_r()
	PORT_BIT( 0xff, 0x00, IPT_POSITIONAL ) PORT_POSITIONS(8) PORT_WRAPS PORT_SENSITIVITY(15) PORT_KEYDELTA(1) PORT_CODE_DEC(KEYCODE_N) PORT_CODE_INC(KEYCODE_M) PORT_PLAYER(2) PORT_FULL_TURN_COUNT(8)
INPUT_PORTS_END


/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout charlayout =
{
	8, 8,
	RGN_FRAC(1,4),
	8,	/* 8 bits per pixel (!) */
	{ 0, 1, 2, 3, RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+1, RGN_FRAC(1,2)+2, RGN_FRAC(1,2)+3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static const gfx_layout spritelayout =
{
	16, 16,
	RGN_FRAC(1,4),
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4,
			32*8+0*4, 32*8+1*4, 32*8+2*4, 32*8+3*4, 32*8+4*4, 32*8+5*4, 32*8+6*4, 32*8+7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
			16*32, 17*32, 18*32, 19*32, 20*32, 21*32, 22*32, 23*32 },
	32*32
};

static const gfx_layout spritelayout8 =
{
	8, 8,
	RGN_FRAC(1,4),
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static GFXDECODE_START( jackal )
	GFXDECODE_ENTRY( "gfx1", 0x00000, charlayout,        0,  1 )	// colors 256-511 without lookup
	GFXDECODE_ENTRY( "gfx1", 0x20000, spritelayout,  0x100, 16 )	// colors   0- 15 with lookup
	GFXDECODE_ENTRY( "gfx1", 0x20000, spritelayout8, 0x100, 16 )	// to handle 8x8 sprites
	GFXDECODE_ENTRY( "gfx1", 0x60000, spritelayout,  0x200, 16 )	// colors  16- 31 with lookup
	GFXDECODE_ENTRY( "gfx1", 0x60000, spritelayout8, 0x200, 16 )	// to handle 8x8 sprites
GFXDECODE_END

/*************************************
 *
 *  Interrupt generator
 *
 *************************************/

static INTERRUPT_GEN( jackal_interrupt )
{
	jackal_state *state = (jackal_state *)device->machine->driver_data;

	if (state->irq_enable)
	{
		cpu_set_input_line(device, 0, HOLD_LINE);
		cpu_set_input_line(state->slavecpu, INPUT_LINE_NMI, PULSE_LINE);
	}
}


/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_START( jackal )
{
	jackal_state *state = (jackal_state *)machine->driver_data;
	UINT8 *ROM = memory_region(machine, "master");

	memory_configure_bank(machine, "bank1", 0, 1, &ROM[0x04000], 0x8000);
	memory_configure_bank(machine, "bank1", 1, 1, &ROM[0x14000], 0x8000);
	memory_set_bank(machine, "bank1", 0);

	state->mastercpu = machine->device("master");
	state->slavecpu = machine->device("slave");

	state_save_register_global(machine, state->irq_enable);
}

static MACHINE_RESET( jackal )
{
	jackal_state *state = (jackal_state *)machine->driver_data;
	UINT8 *rgn = memory_region(machine, "master");

	// HACK: running at the nominal clock rate, music stops working
	// at the beginning of the game. This fixes it.
	machine->device("slave")->set_clock_scale(1.2f);

	state->rambank = rgn;
	state->spritebank = rgn;

	state->irq_enable = 0;
}

static MACHINE_DRIVER_START( jackal )

	/* driver data */
	MDRV_DRIVER_DATA(jackal_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("master", M6809, MASTER_CLOCK/12) // verified on pcb
	MDRV_CPU_PROGRAM_MAP(master_map)
	MDRV_CPU_VBLANK_INT("screen", jackal_interrupt)

	MDRV_CPU_ADD("slave", M6809, MASTER_CLOCK/12) // verified on pcb
	MDRV_CPU_PROGRAM_MAP(slave_map)

	MDRV_QUANTUM_TIME(HZ(6000))

	MDRV_MACHINE_START(jackal)
	MDRV_MACHINE_RESET(jackal)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(1*8, 31*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(jackal)
	MDRV_PALETTE_LENGTH(0x300)

	MDRV_PALETTE_INIT(jackal)
	MDRV_VIDEO_START(jackal)
	MDRV_VIDEO_UPDATE(jackal)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2151, SOUND_CLOCK) // verified on pcb
	MDRV_SOUND_ROUTE(0, "lspeaker", 0.50)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.50)
MACHINE_DRIVER_END

/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( jackal )
	ROM_REGION( 0x20000, "master", 0 )	/* Banked 64k for 1st CPU */
	ROM_LOAD( "j-v02.rom",    0x04000, 0x8000, CRC(0b7e0584) SHA1(e4019463345a4c020d5a004c9a400aca4bdae07b) )
	ROM_CONTINUE(             0x14000, 0x8000 )
	ROM_LOAD( "j-v03.rom",    0x0c000, 0x4000, CRC(3e0dfb83) SHA1(5ba7073751eee33180e51143b348256597909516) )

	ROM_REGION( 0x10000, "slave", 0 )     /* 64k for 2nd cpu (Graphics & Sound)*/
	ROM_LOAD( "631t01.bin",   0x8000, 0x8000, CRC(b189af6a) SHA1(f7df996c394fdd6f2ce128a8df38d7838f7ec6d6) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "631t04.bin",   0x00000, 0x20000, CRC(457f42f0) SHA1(08413a13d128875dddcf4f6ad302363096bf1d41) )
	ROM_LOAD16_BYTE( "631t05.bin",   0x00001, 0x20000, CRC(732b3fc1) SHA1(7e89650b9e5e2b7ae82f8c55ac9995740f6fdfe1) )
	ROM_LOAD16_BYTE( "631t06.bin",   0x40000, 0x20000, CRC(2d10e56e) SHA1(447b464ea725fb9ef87da067a41bcf463b427cce) )
	ROM_LOAD16_BYTE( "631t07.bin",   0x40001, 0x20000, CRC(4961c397) SHA1(b430df58fc3bb722d6fb23bed7d04afdb7e5d9c1) )

	ROM_REGION( 0x0200, "proms", 0 )	/* color lookup tables */
	ROM_LOAD( "631r08.bpr",   0x0000, 0x0100, CRC(7553a172) SHA1(eadf1b4157f62c3af4602da764268df954aa0018) )
	ROM_LOAD( "631r09.bpr",   0x0100, 0x0100, CRC(a74dd86c) SHA1(571f606f8fc0fd3d98d26761de79ccb4cc9ab044) )
ROM_END

ROM_START( topgunr )
	ROM_REGION( 0x20000, "master", 0 )	/* Banked 64k for 1st CPU */
	ROM_LOAD( "tgnr15d.bin",  0x04000, 0x8000, CRC(f7e28426) SHA1(db2d5f252a574b8aa4d8406a8e93b423fd2a7fef) )
	ROM_CONTINUE(             0x14000, 0x8000 )
	ROM_LOAD( "tgnr16d.bin",  0x0c000, 0x4000, CRC(c086844e) SHA1(4d6f27ac3aabb4b2d673aa619e407e417ad89337) )

	ROM_REGION( 0x10000, "slave", 0 )     /* 64k for 2nd cpu (Graphics & Sound)*/
	ROM_LOAD( "631t01.bin",   0x8000, 0x8000, CRC(b189af6a) SHA1(f7df996c394fdd6f2ce128a8df38d7838f7ec6d6) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "tgnr7h.bin",   0x00000, 0x20000, CRC(50122a12) SHA1(c9e0132a3a40d9d28685c867c70231947d8a9cb7) )
	ROM_LOAD16_BYTE( "tgnr8h.bin",   0x00001, 0x20000, CRC(6943b1a4) SHA1(40de2b434600ea4c8fb42e6b21be2c3705a55d67) )
	ROM_LOAD16_BYTE( "tgnr12h.bin",  0x40000, 0x20000, CRC(37dbbdb0) SHA1(f94db780d69e7dd40231a75629af79469d957378) )
	ROM_LOAD16_BYTE( "tgnr13h.bin",  0x40001, 0x20000, CRC(22effcc8) SHA1(4d174b0ce64def32050f87343c4b1424e0fef6f7) )

	ROM_REGION( 0x0200, "proms", 0 )	/* color lookup tables */
	ROM_LOAD( "631r08.bpr",   0x0000, 0x0100, CRC(7553a172) SHA1(eadf1b4157f62c3af4602da764268df954aa0018) )
	ROM_LOAD( "631r09.bpr",   0x0100, 0x0100, CRC(a74dd86c) SHA1(571f606f8fc0fd3d98d26761de79ccb4cc9ab044) )
ROM_END

ROM_START( jackalj )
	ROM_REGION( 0x20000, "master", 0 )	/* Banked 64k for 1st CPU */
	ROM_LOAD( "631t02.bin",   0x04000, 0x8000, CRC(14db6b1a) SHA1(b469ea50aa94a2bda3bd0442300aa1272e5f30c4) )
	ROM_CONTINUE(             0x14000, 0x8000 )
	ROM_LOAD( "631t03.bin",   0x0c000, 0x4000, CRC(fd5f9624) SHA1(2520c1ff54410ef498ecbf52877f011900baed4c) )

	ROM_REGION( 0x10000, "slave", 0 )     /* 64k for 2nd cpu (Graphics & Sound)*/
	ROM_LOAD( "631t01.bin",   0x8000, 0x8000, CRC(b189af6a) SHA1(f7df996c394fdd6f2ce128a8df38d7838f7ec6d6) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "631t04.bin",   0x00000, 0x20000, CRC(457f42f0) SHA1(08413a13d128875dddcf4f6ad302363096bf1d41) )
	ROM_LOAD16_BYTE( "631t05.bin",   0x00001, 0x20000, CRC(732b3fc1) SHA1(7e89650b9e5e2b7ae82f8c55ac9995740f6fdfe1) )
	ROM_LOAD16_BYTE( "631t06.bin",   0x40000, 0x20000, CRC(2d10e56e) SHA1(447b464ea725fb9ef87da067a41bcf463b427cce) )
	ROM_LOAD16_BYTE( "631t07.bin",   0x40001, 0x20000, CRC(4961c397) SHA1(b430df58fc3bb722d6fb23bed7d04afdb7e5d9c1) )

	ROM_REGION( 0x0200, "proms", 0 )	/* color lookup tables */
	ROM_LOAD( "631r08.bpr",   0x0000, 0x0100, CRC(7553a172) SHA1(eadf1b4157f62c3af4602da764268df954aa0018) )
	ROM_LOAD( "631r09.bpr",   0x0100, 0x0100, CRC(a74dd86c) SHA1(571f606f8fc0fd3d98d26761de79ccb4cc9ab044) )
ROM_END

ROM_START( topgunbl )
	ROM_REGION( 0x20000, "master", 0 )	/* Banked 64k for 1st CPU */
	ROM_LOAD( "t-3.c5",       0x04000, 0x8000, CRC(7826ad38) SHA1(875e87867924905b9b83bc203eb7ffe81cf72233) )
	ROM_LOAD( "t-4.c4",       0x14000, 0x8000, CRC(976c8431) SHA1(c199f57c25380d741aec85b0e0bfb6acf383e6a6) )
	ROM_LOAD( "t-2.c6",       0x0c000, 0x4000, CRC(d53172e5) SHA1(44b7f180c17f9a121a2f06f2d3471920a8989e21) )

	ROM_REGION( 0x10000, "slave", 0 )     /* 64k for 2nd cpu (Graphics & Sound)*/
	ROM_LOAD( "t-1.c14",      0x8000, 0x8000, CRC(54aa2d29) SHA1(ebc6b3a5db5120cc33d62e3213d0e881f658282d) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "tgnr7h.bin",   0x00000, 0x20000, CRC(50122a12) SHA1(c9e0132a3a40d9d28685c867c70231947d8a9cb7) )
	ROM_LOAD16_BYTE( "tgnr8h.bin",   0x00001, 0x20000, CRC(6943b1a4) SHA1(40de2b434600ea4c8fb42e6b21be2c3705a55d67) )
	ROM_LOAD16_BYTE( "tgnr12h.bin",  0x40000, 0x20000, CRC(37dbbdb0) SHA1(f94db780d69e7dd40231a75629af79469d957378) )
	ROM_LOAD16_BYTE( "tgnr13h.bin",  0x40001, 0x20000, CRC(22effcc8) SHA1(4d174b0ce64def32050f87343c4b1424e0fef6f7) )

#if 0
	/* same data, different layout (and one bad ROM) */
	ROM_LOAD16_WORD_SWAP( "t-17.n12",     0x00000, 0x08000, CRC(e8875110) )
	ROM_LOAD16_WORD_SWAP( "t-18.n13",     0x08000, 0x08000, CRC(cf14471d) )
	ROM_LOAD16_WORD_SWAP( "t-19.n14",     0x10000, 0x08000, CRC(46ee5dd2) )
	ROM_LOAD16_WORD_SWAP( "t-20.n15",     0x18000, 0x08000, CRC(3f472344) )
	ROM_LOAD16_WORD_SWAP( "t-6.n1",       0x20000, 0x08000, CRC(539cc48c) )
	ROM_LOAD16_WORD_SWAP( "t-5.m1",       0x28000, 0x08000, BAD_DUMP CRC(2dd9a5e9) )
	ROM_LOAD16_WORD_SWAP( "t-7.n2",       0x30000, 0x08000, CRC(0ecd31b1) )
	ROM_LOAD16_WORD_SWAP( "t-8.n3",       0x38000, 0x08000, CRC(f946ada7) )
	ROM_LOAD16_WORD_SWAP( "t-13.n8",      0x40000, 0x08000, CRC(5d669abb) )
	ROM_LOAD16_WORD_SWAP( "t-14.n9",      0x48000, 0x08000, CRC(f349369b) )
	ROM_LOAD16_WORD_SWAP( "t-15.n10",     0x50000, 0x08000, CRC(7c5a91dd) )
	ROM_LOAD16_WORD_SWAP( "t-16.n11",     0x58000, 0x08000, CRC(5ec46d8e) )
	ROM_LOAD16_WORD_SWAP( "t-9.n4",       0x60000, 0x08000, CRC(8269caca) )
	ROM_LOAD16_WORD_SWAP( "t-10.n5",      0x68000, 0x08000, CRC(25393e4f) )
	ROM_LOAD16_WORD_SWAP( "t-11.n6",      0x70000, 0x08000, CRC(7895c22d) )
	ROM_LOAD16_WORD_SWAP( "t-12.n7",      0x78000, 0x08000, CRC(15606dfc) )
#endif

	ROM_REGION( 0x0200, "proms", 0 )	/* color lookup tables */
	ROM_LOAD( "631r08.bpr",   0x0000, 0x0100, CRC(7553a172) SHA1(eadf1b4157f62c3af4602da764268df954aa0018) )
	ROM_LOAD( "631r09.bpr",   0x0100, 0x0100, CRC(a74dd86c) SHA1(571f606f8fc0fd3d98d26761de79ccb4cc9ab044) )
ROM_END


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1986, jackal,   0,      jackal, jackal,   0, ROT90, "Konami", "Jackal (World)", 0 )
GAME( 1986, topgunr,  jackal, jackal, jackal,   0, ROT90, "Konami", "Top Gunner (US)", 0 )
GAME( 1986, jackalj,  jackal, jackal, jackal,   0, ROT90, "Konami", "Tokushu Butai Jackal (Japan)", 0 )
GAME( 1986, topgunbl, jackal, jackal, topgunbl, 0, ROT90, "bootleg", "Top Gunner (bootleg)", 0 )
