/***************************************************************************

Pandora's Palace(GX328) (c) 1984 Konami/Interlogic

Driver by Manuel Abadia <manu@teleline.es>

Notes:
- Press 1P and 2P together to enter test mode.

TODO:
- CPU B continuously reads from 1e00. It seems to be important, could be a
  scanline counter or something like that.

2009-03:
Added dsw locations and verified factory setting based on Guru's notes
(DSW3 not mentioned)

Boards:
- CPU/Video board labeled PWB(A)2000109B
- Sound board labeled PWB(B)3000154A

***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "cpu/z80/z80.h"
#include "cpu/mcs48/mcs48.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "includes/konamipt.h"
#include "includes/pandoras.h"


#define MASTER_CLOCK		XTAL_18_432MHz
#define SOUND_CLOCK			XTAL_14_31818MHz


static INTERRUPT_GEN( pandoras_master_interrupt )
{
	pandoras_state *state = (pandoras_state *)device->machine->driver_data;

	if (state->irq_enable_a)
		cpu_set_input_line(device, M6809_IRQ_LINE, HOLD_LINE);
}

static INTERRUPT_GEN( pandoras_slave_interrupt )
{
	pandoras_state *state = (pandoras_state *)device->machine->driver_data;

	if (state->irq_enable_b)
		cpu_set_input_line(device, M6809_IRQ_LINE, HOLD_LINE);
}

static WRITE8_HANDLER( pandoras_int_control_w )
{
	/*  byte 0: irq enable (CPU A)
        byte 2: coin counter 1
        byte 3: coin counter 2
        byte 5: flip screen
        byte 6: irq enable (CPU B)
        byte 7: NMI to CPU B

        other bytes unknown */

	pandoras_state *state = (pandoras_state *)space->machine->driver_data;

	switch (offset)
	{
		case 0x00:	if (!data)
					cpu_set_input_line(state->maincpu, M6809_IRQ_LINE, CLEAR_LINE);
				state->irq_enable_a = data;
				break;
		case 0x02:	coin_counter_w(space->machine, 0,data & 0x01);
				break;
		case 0x03:	coin_counter_w(space->machine, 1,data & 0x01);
				break;
		case 0x05:	pandoras_flipscreen_w(space, 0, data);
				break;
		case 0x06:	if (!data)
					cpu_set_input_line(state->subcpu, M6809_IRQ_LINE, CLEAR_LINE);
				state->irq_enable_b = data;
				break;
		case 0x07:	cpu_set_input_line(state->subcpu, INPUT_LINE_NMI, PULSE_LINE);
				break;

		default:	logerror("%04x: (irq_ctrl) write %02x to %02x\n",cpu_get_pc(space->cpu), data, offset);
				break;
	}
}

static WRITE8_HANDLER( pandoras_cpua_irqtrigger_w )
{
	pandoras_state *state = (pandoras_state *)space->machine->driver_data;

	if (!state->firq_old_data_a && data)
		cpu_set_input_line(state->maincpu, M6809_FIRQ_LINE, HOLD_LINE);

	state->firq_old_data_a = data;
}

static WRITE8_HANDLER( pandoras_cpub_irqtrigger_w )
{
	pandoras_state *state = (pandoras_state *)space->machine->driver_data;

	if (!state->firq_old_data_b && data)
		cpu_set_input_line(state->subcpu, M6809_FIRQ_LINE, HOLD_LINE);

	state->firq_old_data_b = data;
}

static WRITE8_HANDLER( pandoras_i8039_irqtrigger_w )
{
	pandoras_state *state = (pandoras_state *)space->machine->driver_data;
	cpu_set_input_line(state->mcu, 0, ASSERT_LINE);
}

static WRITE8_HANDLER( i8039_irqen_and_status_w )
{
	pandoras_state *state = (pandoras_state *)space->machine->driver_data;

	/* bit 7 enables IRQ */
	if ((data & 0x80) == 0)
		cpu_set_input_line(state->mcu, 0, CLEAR_LINE);

	/* bit 5 goes to 8910 port A */
	state->i8039_status = (data & 0x20) >> 5;
}

static WRITE8_HANDLER( pandoras_z80_irqtrigger_w )
{
	pandoras_state *state = (pandoras_state *)space->machine->driver_data;
	cpu_set_input_line_and_vector(state->audiocpu, 0, HOLD_LINE, 0xff);
}



static ADDRESS_MAP_START( pandoras_master_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_RAM AM_SHARE("share1") AM_BASE_MEMBER(pandoras_state, spriteram)				/* Work RAM (Shared with CPU B) */
	AM_RANGE(0x1000, 0x13ff) AM_RAM_WRITE(pandoras_cram_w) AM_SHARE("share2") AM_BASE_MEMBER(pandoras_state, colorram)	/* Color RAM (shared with CPU B) */
	AM_RANGE(0x1400, 0x17ff) AM_RAM_WRITE(pandoras_vram_w) AM_SHARE("share3") AM_BASE_MEMBER(pandoras_state, videoram)	/* Video RAM (shared with CPU B) */
	AM_RANGE(0x1800, 0x1807) AM_WRITE(pandoras_int_control_w)								/* INT control */
	AM_RANGE(0x1a00, 0x1a00) AM_WRITE(pandoras_scrolly_w)									/* bg scroll */
	AM_RANGE(0x1c00, 0x1c00) AM_WRITE(pandoras_z80_irqtrigger_w)							/* cause INT on the Z80 */
	AM_RANGE(0x1e00, 0x1e00) AM_WRITE(soundlatch_w)											/* sound command to the Z80 */
	AM_RANGE(0x2000, 0x2000) AM_WRITE(pandoras_cpub_irqtrigger_w)							/* cause FIRQ on CPU B */
	AM_RANGE(0x2001, 0x2001) AM_WRITE(watchdog_reset_w)										/* watchdog reset */
	AM_RANGE(0x4000, 0x5fff) AM_ROM															/* space for diagnostic ROM */
	AM_RANGE(0x6000, 0x67ff) AM_RAM AM_SHARE("share4")											/* Shared RAM with CPU B */
	AM_RANGE(0x8000, 0xffff) AM_ROM															/* ROM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( pandoras_slave_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_RAM AM_SHARE("share1")										/* Work RAM (Shared with CPU A) */
	AM_RANGE(0x1000, 0x13ff) AM_RAM_WRITE(pandoras_cram_w) AM_SHARE("share2")						/* Color RAM (shared with CPU A) */
	AM_RANGE(0x1400, 0x17ff) AM_RAM_WRITE(pandoras_vram_w) AM_SHARE("share3")						/* Video RAM (shared with CPU A) */
	AM_RANGE(0x1800, 0x1800) AM_READ_PORT("DSW1")
	AM_RANGE(0x1800, 0x1807) AM_WRITE(pandoras_int_control_w)								/* INT control */
	AM_RANGE(0x1a00, 0x1a00) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x1a01, 0x1a01) AM_READ_PORT("P1")
	AM_RANGE(0x1a02, 0x1a02) AM_READ_PORT("P2")
	AM_RANGE(0x1a03, 0x1a03) AM_READ_PORT("DSW3")
	AM_RANGE(0x1c00, 0x1c00) AM_READ_PORT("DSW2")
//  AM_RANGE(0x1e00, 0x1e00) AM_READNOP                                                     /* ??? seems to be important */
	AM_RANGE(0x8000, 0x8000) AM_WRITE(watchdog_reset_w)										/* watchdog reset */
	AM_RANGE(0xa000, 0xa000) AM_WRITE(pandoras_cpua_irqtrigger_w)							/* cause FIRQ on CPU A */
	AM_RANGE(0xc000, 0xc7ff) AM_RAM AM_SHARE("share4")												/* Shared RAM with the CPU A */
	AM_RANGE(0xe000, 0xffff) AM_ROM															/* ROM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( pandoras_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_ROM															/* ROM */
	AM_RANGE(0x2000, 0x23ff) AM_RAM															/* RAM */
	AM_RANGE(0x4000, 0x4000) AM_READ(soundlatch_r)											/* soundlatch_r */
	AM_RANGE(0x6000, 0x6000) AM_DEVWRITE("aysnd", ay8910_address_w)							/* AY-8910 */
	AM_RANGE(0x6001, 0x6001) AM_DEVREAD("aysnd", ay8910_r)										/* AY-8910 */
	AM_RANGE(0x6002, 0x6002) AM_DEVWRITE("aysnd", ay8910_data_w)								/* AY-8910 */
	AM_RANGE(0x8000, 0x8000) AM_WRITE(pandoras_i8039_irqtrigger_w)							/* cause INT on the 8039 */
	AM_RANGE(0xa000, 0xa000) AM_WRITE(soundlatch2_w)										/* sound command to the 8039 */
ADDRESS_MAP_END

static ADDRESS_MAP_START( pandoras_i8039_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( pandoras_i8039_io_map, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x00, 0xff) AM_READ(soundlatch2_r)
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_DEVWRITE("dac", dac_w)
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_WRITE(i8039_irqen_and_status_w)
ADDRESS_MAP_END


/***************************************************************************

    Input Ports

***************************************************************************/

static INPUT_PORTS_START( pandoras )
	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "No Coin B", SW1)
	/* "No Coin B" = coins produce sound, but no effect on coin counter */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )		PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )		PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Bonus_Life ) )	PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "20k and every 60k" )
	PORT_DIPSETTING(    0x10, "30k and every 70k" )
	PORT_DIPSETTING(    0x08, "20k" )
	PORT_DIPSETTING(    0x00, "30k" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )	PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )	PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "Freeze" )
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

	PORT_START("SYSTEM")
	KONAMI8_SYSTEM_10
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1")
	KONAMI8_MONO_B1_UNK

	PORT_START("P2")
	KONAMI8_COCKTAIL_B1_UNK
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 15*4, 14*4, 13*4, 12*4, 11*4, 10*4, 9*4, 8*4,
			7*4, 6*4, 5*4, 4*4, 3*4, 2*4, 1*4, 0*4 },
	{ 15*4*16, 14*4*16, 13*4*16, 12*4*16, 11*4*16, 10*4*16, 9*4*16, 8*4*16,
			7*4*16, 6*4*16, 5*4*16, 4*4*16, 3*4*16, 2*4*16, 1*4*16, 0*4*16 },
	32*4*8
};

static GFXDECODE_START( pandoras )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout,     0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, charlayout,   16*16, 16 )
GFXDECODE_END

/***************************************************************************

    Machine Driver

***************************************************************************/

static MACHINE_START( pandoras )
{
	pandoras_state *state = (pandoras_state *)machine->driver_data;

	state->maincpu = machine->device<cpu_device>("maincpu");
	state->subcpu = machine->device<cpu_device>("sub");
	state->audiocpu = machine->device<cpu_device>("audiocpu");
	state->mcu = machine->device<cpu_device>("mcu");

	state_save_register_global(machine, state->firq_old_data_a);
	state_save_register_global(machine, state->firq_old_data_b);
	state_save_register_global(machine, state->irq_enable_a);
	state_save_register_global(machine, state->irq_enable_b);
	state_save_register_global(machine, state->i8039_status);
}

static MACHINE_RESET( pandoras )
{
	pandoras_state *state = (pandoras_state *)machine->driver_data;

	state->firq_old_data_a = 0;
	state->firq_old_data_b = 0;
	state->irq_enable_a = 0;
	state->irq_enable_b = 0;
	state->i8039_status = 0;

	state->flipscreen = 0;
}

static READ8_DEVICE_HANDLER( pandoras_portA_r )
{
	pandoras_state *state = (pandoras_state *)device->machine->driver_data;
	return state->i8039_status;
}

static READ8_DEVICE_HANDLER( pandoras_portB_r )
{
	pandoras_state *state = (pandoras_state *)device->machine->driver_data;
	return (state->audiocpu->total_cycles() / 512) & 0x0f;
}

static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_HANDLER(pandoras_portA_r),	// not used
	DEVCB_HANDLER(pandoras_portB_r),
	DEVCB_NULL,
	DEVCB_NULL
};

static MACHINE_DRIVER_START( pandoras )

	/* driver data */
	MDRV_DRIVER_DATA(pandoras_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M6809, MASTER_CLOCK/6)	/* CPU A */
	MDRV_CPU_PROGRAM_MAP(pandoras_master_map)
	MDRV_CPU_VBLANK_INT("screen", pandoras_master_interrupt)

	MDRV_CPU_ADD("sub", M6809, MASTER_CLOCK/6)		/* CPU B */
	MDRV_CPU_PROGRAM_MAP(pandoras_slave_map)
	MDRV_CPU_VBLANK_INT("screen", pandoras_slave_interrupt)

	MDRV_CPU_ADD("audiocpu", Z80, SOUND_CLOCK/8)
	MDRV_CPU_PROGRAM_MAP(pandoras_sound_map)

	MDRV_CPU_ADD("mcu", I8039, SOUND_CLOCK/2)
	MDRV_CPU_PROGRAM_MAP(pandoras_i8039_map)
	MDRV_CPU_IO_MAP(pandoras_i8039_io_map)

	MDRV_QUANTUM_TIME(HZ(3000))	/* slices per frame */

	MDRV_MACHINE_START(pandoras)
	MDRV_MACHINE_RESET(pandoras)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(pandoras)
	MDRV_PALETTE_LENGTH(16*16+16*16)

	MDRV_PALETTE_INIT(pandoras)
	MDRV_VIDEO_START(pandoras)
	MDRV_VIDEO_UPDATE(pandoras)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("aysnd", AY8910, SOUND_CLOCK/8)
	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)

	MDRV_SOUND_ADD("dac", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_DRIVER_END


/***************************************************************************

  Game ROMs

***************************************************************************/

ROM_START( pandoras )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* 64K for the CPU A */
	ROM_LOAD( "pand_j13.cpu",	0x08000, 0x02000, CRC(7a0fe9c5) SHA1(e68c8d76d1abb69ac72b0e2cd8c1dfc540064ee3) )
	ROM_LOAD( "pand_j12.cpu",	0x0a000, 0x02000, CRC(7dc4bfe1) SHA1(359c3051e5d7a34d0e49578e4c168fd19c73e202) )
	ROM_LOAD( "pand_j10.cpu",	0x0c000, 0x02000, CRC(be3af3b7) SHA1(91321b53e17e58b674104cb95b1c35ee8fecae22) )
	ROM_LOAD( "pand_j9.cpu",	0x0e000, 0x02000, CRC(e674a17a) SHA1(a4b096dc455425dd60298acf2203659ef6f8d857) )

	ROM_REGION( 0x10000, "sub", 0 ) /* 64K for the CPU B */
	ROM_LOAD( "pand_j5.cpu",	0x0e000, 0x02000, CRC(4aab190b) SHA1(d2204953d6b6b34cea851bfc9c2b31426e75f90b) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64K for the Sound CPU */
	ROM_LOAD( "pand_6c.snd",	0x00000, 0x02000, CRC(0c1f109d) SHA1(4e6cdee99261764bd2fea5abbd49d800baba0dc5) )

	ROM_REGION( 0x2000, "mcu", 0 ) /* 4K for the Sound CPU 2 (Data is mirrored to fit into an 8K rom) */
	ROM_LOAD( "pand_7e.snd",	0x00000, 0x02000, CRC(1071c1ba) SHA1(3693be69f4b32fb3031bcdee8cac0d46ec8c2804) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "pand_j18.cpu",	0x00000, 0x02000, CRC(99a696c5) SHA1(35a27cd5ecc51a9a1acf01eb8078a1028f03be32) )	/* sprites */
	ROM_LOAD( "pand_j17.cpu",	0x02000, 0x02000, CRC(38a03c21) SHA1(b0c8f642787bab3cd1d76657e56f07f4f6f9073c) )
	ROM_LOAD( "pand_j16.cpu",	0x04000, 0x02000, CRC(e0708a78) SHA1(9dbd08b6ca8a66a61e128d1806888696273de848) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "pand_a18.cpu",	0x00000, 0x02000, CRC(23706d4a) SHA1(cca92e6ff90e3006a79a214f1211fd659771de53) )	/* tiles */
	ROM_LOAD( "pand_a19.cpu",	0x02000, 0x02000, CRC(a463b3f9) SHA1(549b7ee6e47325b80186441da11879fb8b1b47be) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "pandora.2a",		0x0000, 0x020, CRC(4d56f939) SHA1(a8dac604bfdaf4b153b75dbf165de113152b6daa) ) /* palette */
	ROM_LOAD( "pandora.17g",	0x0020, 0x100, CRC(c1a90cfc) SHA1(c6581f2d543e38f1de399774183cf0698e61dab5) ) /* sprite lookup table */
	ROM_LOAD( "pandora.16b",	0x0120, 0x100, CRC(c89af0c3) SHA1(4072c8d61521b34ce4dbce1d48f546402e9539cd) ) /* character lookup table */
ROM_END


GAME( 1984, pandoras, 0, pandoras, pandoras, 0, ROT90, "Konami / Interlogic", "Pandora's Palace", GAME_SUPPORTS_SAVE )
