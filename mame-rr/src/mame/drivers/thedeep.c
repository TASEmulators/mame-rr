/***************************************************************************

                            -= Run Deep / The Deep =-

                    driver by   Luca Elia (l.elia@tin.it)

Main CPU    :   Z80 (LH0080B @ 6MHz) + i8751 (Intel C8751H-88, protection)

Sound CPU   :   65C02 (R65C02P2 @ 2MHz)

Sound Chips :   YM2203C

Video Chips :   L7B0073 DATA EAST MXC 06 8746
                L7A0072 DATA EAST BAC 06 VAE8713

Board       :   DE-0298-1

Notes:

- The MCU handles coins and the bank switching of the roms for the main cpu.
  It additionally provides some z80 code that is copied to ram.

- One ROM (FI-1) is not used.

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m6502/m6502.h"
#include "deprecat.h"
#include "includes/thedeep.h"
#include "sound/2203intf.h"

/***************************************************************************

                            Main CPU + MCU simulation

***************************************************************************/

static int nmi_enable;

static WRITE8_HANDLER( thedeep_nmi_w )
{
	nmi_enable = data;
}

static WRITE8_HANDLER( thedeep_sound_w )
{
	soundlatch_w(space, 0, data);
	cputag_set_input_line(space->machine, "audiocpu", INPUT_LINE_NMI, PULSE_LINE);
}

static UINT8 protection_command, protection_data;
static int protection_index, protection_irq;
static int rombank;

static MACHINE_RESET( thedeep )
{
	memory_set_bankptr(machine, "bank1", memory_region(machine, "maincpu") + 0x10000 + 0 * 0x4000);
	thedeep_scroll[0] = 0;
	thedeep_scroll[1] = 0;
	thedeep_scroll[2] = 0;
	thedeep_scroll[3] = 0;
	protection_command = 0;
	protection_index = -1;
	protection_irq = 0;
	rombank = -1;
}

static WRITE8_HANDLER( thedeep_protection_w )
{
	protection_command = data;
	switch (protection_command)
	{
		case 0x11:
			flip_screen_set(space->machine, 1);
		break;

		case 0x20:
			flip_screen_set(space->machine, 0);
		break;

		case 0x30:
		case 0x31:
		case 0x32:
		case 0x33:
		{
			UINT8 *rom;
			int new_rombank = protection_command & 3;
			if (rombank == new_rombank)	break;
			rombank = new_rombank;
			rom = memory_region(space->machine, "maincpu");
			memory_set_bankptr(space->machine, "bank1", rom + 0x10000 + rombank * 0x4000);
			/* there's code which falls through from the fixed ROM to bank #1, I have to */
			/* copy it there otherwise the CPU bank switching support will not catch it. */
			memcpy(rom + 0x08000, rom + 0x10000 + rombank * 0x4000, 0x4000);
		}
		break;

		case 0x59:
		{
			if (protection_index < 0)
				protection_index = 0;

			if ( protection_index < 0x19b )
// d000-d00c:   hl += a * b
// d00d-d029:   input a (e.g. $39) output hl (e.g. h=$03 l=$09).
//              Replace trainling 0's with space ($10). 00 -> '  '
// d02a-d039:   input a (e.g. $39) output hl (e.g. h=$03 l=$09).
//              Replace trainling 0's with space ($10). 00 -> ' 0'
// d03a-d046:   input a (e.g. $39) output hl (e.g. h=$03 l=$09). 00 -> '00'
// d047-d086:   a /= e (e can be 0!)
// d087-d0a4:   print ASCII string from HL to IX (sub $30 to every char)
// d0a4-d0be:   print any string from HL to IX
// d0bf-d109:   print ASCII string from HL to IX. Color is in c. (e.g. "game over")
// d10a-d11f:   print 2 digit decimal number in hl to ix, color c. change ix
// d120-d157:   update score: add 3 BCD bytes at ix to those at iy, then clear those at ix
// d158-d165:   print digit: (IX+0) <- H; (IX+1) <-L. ix+=40
// d166-d174:   hl = (hl + 2*a)
// d175-d181:   hl *= e (e must be non zero)
// d182-d19a:   hl /= de
				protection_data = memory_region(space->machine, "cpu3")[0x185+protection_index++];
			else
				protection_data = 0xc9;

			protection_irq  = 1;
		}
		break;

		default:
			logerror( "pc %04x: protection_command %02x\n", cpu_get_pc(space->cpu),protection_command);
	}
}

static READ8_HANDLER( thedeep_e004_r )
{
	return protection_irq ? 1 : 0;
}

static READ8_HANDLER( thedeep_protection_r )
{
	protection_irq = 0;
	return protection_data;
}

static WRITE8_HANDLER( thedeep_e100_w )
{
	if (data != 1)
		logerror("pc %04x: e100 = %02x\n", cpu_get_pc(space->cpu),data);
}

static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")	// ROM (banked)
	AM_RANGE(0xc000, 0xcfff) AM_RAM
	AM_RANGE(0xd000, 0xdfff) AM_RAM				            	// RAM (MCU data copied here)
	AM_RANGE(0xe000, 0xe000) AM_READWRITE(thedeep_protection_r, thedeep_protection_w	)	// To MCU
	AM_RANGE(0xe004, 0xe004) AM_READWRITE(thedeep_e004_r, thedeep_nmi_w			)	//
	AM_RANGE(0xe008, 0xe008) AM_READ_PORT("e008")			// P1 (Inputs)
	AM_RANGE(0xe009, 0xe009) AM_READ_PORT("e009")			// P2
	AM_RANGE(0xe00a, 0xe00a) AM_READ_PORT("e00a")			// DSW1
	AM_RANGE(0xe00b, 0xe00b) AM_READ_PORT("e00b")			// DSW2
	AM_RANGE(0xe00c, 0xe00c) AM_WRITE(thedeep_sound_w		)	// To Sound CPU
	AM_RANGE(0xe100, 0xe100) AM_WRITE(thedeep_e100_w		)	// ?
	AM_RANGE(0xe210, 0xe213) AM_WRITEONLY AM_BASE(&thedeep_scroll				)	// Scroll
	AM_RANGE(0xe400, 0xe7ff) AM_RAM AM_BASE_SIZE_GENERIC(spriteram)	// Sprites
	AM_RANGE(0xe800, 0xefff) AM_RAM_WRITE(thedeep_vram_1_w) AM_BASE(&thedeep_vram_1		)	// Text Layer
	AM_RANGE(0xf000, 0xf7ff) AM_RAM_WRITE(thedeep_vram_0_w) AM_BASE(&thedeep_vram_0		)	// Background Layer
	AM_RANGE(0xf800, 0xf83f) AM_RAM AM_BASE(&thedeep_scroll2				)	// Column Scroll
	AM_RANGE(0xf840, 0xffff) AM_RAM
ADDRESS_MAP_END


/***************************************************************************

                                    Sound CPU

***************************************************************************/

static ADDRESS_MAP_START( audio_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_RAM
	AM_RANGE(0x0800, 0x0801) AM_DEVWRITE("ymsnd", ym2203_w	)	//
	AM_RANGE(0x3000, 0x3000) AM_READ(soundlatch_r				)	// From Main CPU
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END


/***************************************************************************

                                Input Ports

***************************************************************************/

static INPUT_PORTS_START( thedeep )
	PORT_START("e008")
	PORT_BIT(  0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    )	// Up / down shown in service mode
	PORT_BIT(  0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  )
	PORT_BIT(  0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  )
	PORT_BIT(  0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT(  0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT(  0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT(  0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x80, IP_ACTIVE_LOW, IPT_START1  )

	PORT_START("e009")
	PORT_BIT(  0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT(  0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT(  0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT(  0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT(  0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT(  0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT(  0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x80, IP_ACTIVE_LOW, IPT_START2  )

	PORT_START("e00a")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x10, 0x10, "Invulnerability (Cheat)")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Start Stage" )
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("e00b")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Harder ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "50k" )
	PORT_DIPSETTING(    0x30, "50k 70k" )
	PORT_DIPSETTING(    0x20, "60k 80k" )
	PORT_DIPSETTING(    0x10, "80k 100k" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) ) /* Listed as "Unused" in the manual */
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("MCU")	// Read by the mcu
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_IMPULSE(1)
INPUT_PORTS_END


/***************************************************************************

                                Graphics Layouts

***************************************************************************/

static const gfx_layout layout_8x8x2 =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ 0, 4 },
	{ STEP4(RGN_FRAC(1,2),1), STEP4(RGN_FRAC(0,2),1) },
	{ STEP8(0,8) },
	8*8
};

static const gfx_layout layout_16x16x4 =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(0,4),RGN_FRAC(1,4),RGN_FRAC(2,4),RGN_FRAC(3,4) },
	{ STEP8(8*8*2,1), STEP8(0,1) },
	{ STEP16(0,8) },
	16*16
};

static GFXDECODE_START( thedeep )
	GFXDECODE_ENTRY( "sprites", 0, layout_16x16x4,	0x080,  8 ) // [0] Sprites
	GFXDECODE_ENTRY( "bg_gfx", 0, layout_16x16x4,	0x100, 16 ) // [1] Background Layer
	GFXDECODE_ENTRY( "text", 0, layout_8x8x2,	0x000, 16 ) // [2] Text Layer
GFXDECODE_END



/***************************************************************************

                                Machine Drivers

***************************************************************************/

static void irqhandler(running_device *device, int irq)
{
	cputag_set_input_line(device->machine, "audiocpu", 0, irq ? ASSERT_LINE : CLEAR_LINE);
}

static const ym2203_interface thedeep_ym2203_intf =
{
	{
		AY8910_LEGACY_OUTPUT,
		AY8910_DEFAULT_LOADS,
		DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL
	},
	irqhandler
};

static INTERRUPT_GEN( thedeep_interrupt )
{
	if (cpu_getiloops(device))
	{
		if (protection_command != 0x59)
		{
			int coins = input_port_read(device->machine, "MCU");
			if		(coins & 1)	protection_data = 1;
			else if	(coins & 2)	protection_data = 2;
			else if	(coins & 4)	protection_data = 3;
			else				protection_data = 0;

			if (protection_data)
				protection_irq = 1;
		}
		if (protection_irq)
			cpu_set_input_line(device, 0, HOLD_LINE);
	}
	else
	{
		if (nmi_enable)
		{
			cpu_set_input_line(device, INPUT_LINE_NMI, ASSERT_LINE);
			cpu_set_input_line(device, INPUT_LINE_NMI, CLEAR_LINE);
		}
	}
}

static MACHINE_DRIVER_START( thedeep )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, XTAL_12MHz/2)		/* verified on pcb */
	MDRV_CPU_PROGRAM_MAP(main_map)
	MDRV_CPU_VBLANK_INT_HACK(thedeep_interrupt,2)	/* IRQ by MCU, NMI by vblank (maskable) */

	MDRV_CPU_ADD("audiocpu", M65C02, XTAL_12MHz/8)		/* verified on pcb */
	MDRV_CPU_PROGRAM_MAP(audio_map)
	/* IRQ by YM2203, NMI by when sound latch written by main cpu */

	/* CPU3 is a i8751 running at 8Mhz (8mhz xtal)*/

	MDRV_MACHINE_RESET(thedeep)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(0x100, 0xf8)
	MDRV_SCREEN_VISIBLE_AREA(0, 0x100-1, 0, 0xf8-1)

	MDRV_GFXDECODE(thedeep)
	MDRV_PALETTE_LENGTH(512)

	MDRV_PALETTE_INIT(thedeep)
	MDRV_VIDEO_START(thedeep)
	MDRV_VIDEO_UPDATE(thedeep)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ymsnd", YM2203, XTAL_12MHz/4)  /* verified on pcb */
	MDRV_SOUND_CONFIG(thedeep_ym2203_intf)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END



/***************************************************************************

                                ROMs Loading

***************************************************************************/

/***************************************************************************

Here are the proms for The Deep!
NOTE: This game is Vertical.
I couldn't test this board so I don't know the manufakturer, sorry.
1 Z80
1 R6502
1 YM 2203
1 OSC 12 Mhz
1 OSC 8 Mhz
1 MPU 8751 (which is read-protected)

If you need more info or if this package doesn't
Work, mail me.

..............CaBBe!...................................

***************************************************************************/

ROM_START( thedeep )
	ROM_REGION( 0x20000, "maincpu", 0 )		/* Z80 Code */
	ROM_LOAD( "dp-10.rom", 0x00000, 0x08000, CRC(7480b7a5) SHA1(ac6f121873a70c8077576322c201b7089c7b8a91) )
	ROM_LOAD( "dp-09.rom", 0x10000, 0x10000, CRC(c630aece) SHA1(809916a1ba1c8e0af4c228b0f26ac638e2abf81e) )

	ROM_REGION( 0x10000, "audiocpu", 0 )		/* 65C02 Code */
	ROM_LOAD( "dp-12.rom", 0x8000, 0x8000, CRC(c4e848c4) SHA1(d2dec5c8d7d59703f5485cab9124bf4f835fe728) )

	ROM_REGION( 0x1000, "cpu3", 0 )		/* i8751 Code */
	ROM_LOAD( "dp-14", 0x0000, 0x1000, CRC(0b886dad) SHA1(487192764342f8b0a320d20a378bf94f84592da9) )	// 1xxxxxxxxxxx = 0xFF

	ROM_REGION( 0x40000, "sprites", 0 )	/* Sprites */
	ROM_LOAD( "dp-08.rom", 0x00000, 0x10000, CRC(c5624f6b) SHA1(a3c0b13cddae760f30c7344d718cd69cad990054) )
	ROM_LOAD( "dp-07.rom", 0x10000, 0x10000, CRC(c76768c1) SHA1(e41ace1cb06ebe7f676b3b179b7dd01d00cf4d6a) )
	ROM_LOAD( "dp-06.rom", 0x20000, 0x10000, CRC(98adea78) SHA1(6a1af70de995a0a5e42fd395dd9454b7e2d9cb82) )
	ROM_LOAD( "dp-05.rom", 0x30000, 0x10000, CRC(76ea7dd1) SHA1(c29abb44a1182b47da749eeeb2db025ae3f28ea7) )

	ROM_REGION( 0x40000, "bg_gfx", 0 )	/* 16x16x4 Background Layer */
	ROM_LOAD( "dp-03.rom", 0x00000, 0x10000, CRC(6bf5d819) SHA1(74079632d7c88ec22010c1a5bece0e36847fdab9) )
	ROM_LOAD( "dp-01.rom", 0x10000, 0x10000, CRC(e56be2fe) SHA1(25acc0f6d9cb5a727c9bac3e80aeb85a4727ddb0) )
	ROM_LOAD( "dp-04.rom", 0x20000, 0x10000, CRC(4db02c3c) SHA1(6284541372dec1113570cef31ca3c1a202fb4add) )
	ROM_LOAD( "dp-02.rom", 0x30000, 0x10000, CRC(1add423b) SHA1(b565340d719044ba2c428aab74f43f5a7cf7e2a3) )

	ROM_REGION( 0x4000, "text", 0 )	/* 8x8x2 Text Layer */
	ROM_LOAD( "dp-11.rom", 0x0000, 0x4000, CRC(196e23d1) SHA1(ed14e63fccb3e5dce462d9b8155e78749eaf9b3b) )

	ROM_REGION( 0x600, "proms", 0 )	/* Colors */
	ROM_LOAD( "fi-1", 0x000, 0x200, CRC(f31efe09) SHA1(808c90fe02ed7b4000967c331b8773c4168b8a97) )	// FIXED BITS (xxxxxx0xxxxxx0x0)
	ROM_LOAD( "fi-2", 0x200, 0x200, CRC(f305c8d5) SHA1(f82c709dc75a3c681d6f0ebf2702cb90110b1f0c) )	// FIXED BITS (0000xxxx)
	ROM_LOAD( "fi-3", 0x400, 0x200, CRC(f61a9686) SHA1(24082f60b72268d240ceca6999bdf18872625cd2) )
ROM_END

ROM_START( rundeep )
	ROM_REGION( 0x20000, "maincpu", 0 )		/* Z80 Code */
	ROM_LOAD( "3", 0x00000, 0x08000, CRC(c9c9e194) SHA1(e9552c3321585f0902f29b55a7de8e2316885713) )
	ROM_LOAD( "9", 0x10000, 0x10000, CRC(931f4e67) SHA1(f4942c5f0fdbcd6cdb96ddbbf2015f938b56b466) )

	ROM_REGION( 0x10000, "audiocpu", 0 )		/* 65C02 Code */
	ROM_LOAD( "dp-12.rom", 0x8000, 0x8000, CRC(c4e848c4) SHA1(d2dec5c8d7d59703f5485cab9124bf4f835fe728) )

	ROM_REGION( 0x1000, "cpu3", 0 )		/* i8751 Code */
	ROM_LOAD( "dp-14", 0x0000, 0x1000, CRC(0b886dad) SHA1(487192764342f8b0a320d20a378bf94f84592da9) )	// 1xxxxxxxxxxx = 0xFF

	ROM_REGION( 0x40000, "sprites", 0 )	/* Sprites */
	ROM_LOAD( "dp-08.rom", 0x00000, 0x10000, CRC(c5624f6b) SHA1(a3c0b13cddae760f30c7344d718cd69cad990054) )
	ROM_LOAD( "dp-07.rom", 0x10000, 0x10000, CRC(c76768c1) SHA1(e41ace1cb06ebe7f676b3b179b7dd01d00cf4d6a) )
	ROM_LOAD( "dp-06.rom", 0x20000, 0x10000, CRC(98adea78) SHA1(6a1af70de995a0a5e42fd395dd9454b7e2d9cb82) )
	ROM_LOAD( "dp-05.rom", 0x30000, 0x10000, CRC(76ea7dd1) SHA1(c29abb44a1182b47da749eeeb2db025ae3f28ea7) )

	ROM_REGION( 0x40000, "bg_gfx", 0 )	/* 16x16x4 Background Layer */
	ROM_LOAD( "dp-03.rom", 0x00000, 0x10000, CRC(6bf5d819) SHA1(74079632d7c88ec22010c1a5bece0e36847fdab9) )
	ROM_LOAD( "dp-01.rom", 0x10000, 0x10000, CRC(e56be2fe) SHA1(25acc0f6d9cb5a727c9bac3e80aeb85a4727ddb0) )
	ROM_LOAD( "dp-04.rom", 0x20000, 0x10000, CRC(4db02c3c) SHA1(6284541372dec1113570cef31ca3c1a202fb4add) )
	ROM_LOAD( "dp-02.rom", 0x30000, 0x10000, CRC(1add423b) SHA1(b565340d719044ba2c428aab74f43f5a7cf7e2a3) )

	ROM_REGION( 0x4000, "text", 0 )	/* 8x8x2 Text Layer */
	ROM_LOAD( "11", 0x0000, 0x4000, CRC(5d29e4b9) SHA1(608345291062e9ce329ebe9a8c1e65d52e358785) )

	ROM_REGION( 0x600, "proms", 0 )	/* Colors */
	ROM_LOAD( "fi-1", 0x000, 0x200, CRC(f31efe09) SHA1(808c90fe02ed7b4000967c331b8773c4168b8a97) )	// FIXED BITS (xxxxxx0xxxxxx0x0)
	ROM_LOAD( "fi-2", 0x200, 0x200, CRC(f305c8d5) SHA1(f82c709dc75a3c681d6f0ebf2702cb90110b1f0c) )	// FIXED BITS (0000xxxx)
	ROM_LOAD( "fi-3", 0x400, 0x200, CRC(f61a9686) SHA1(24082f60b72268d240ceca6999bdf18872625cd2) )
ROM_END

GAME( 1987, thedeep, 0,      thedeep, thedeep, 0, ROT270, "Wood Place Inc.", "The Deep (Japan)", 0 )
GAME( 1988, rundeep, thedeep,thedeep, thedeep, 0, ROT270, "bootleg (Cream)", "Run Deep", 0 )
