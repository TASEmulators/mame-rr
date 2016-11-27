/***************************************************************************

    Chequered Flag / Checkered Flag (GX717) (c) Konami 1988

    Notes:
    - Position counter doesn't behave correctly because of the K051733
      protection.
    - 007232 volume & panning control is almost certainly wrong.

    2008-07
    Dip locations and recommended settings verified with manual

***************************************************************************/

#include "emu.h"
#include "deprecat.h"
#include "cpu/z80/z80.h"
#include "cpu/konami/konami.h"
#include "video/konicdev.h"
#include "sound/2151intf.h"
#include "sound/k007232.h"
#include "includes/chqflag.h"

#include "chqflag.lh"


static WRITE8_DEVICE_HANDLER( k007232_extvolume_w );

static INTERRUPT_GEN( chqflag_interrupt )
{
	chqflag_state *state = (chqflag_state *)device->machine->driver_data;

	if (cpu_getiloops(device) == 0)
	{
		if (k051960_is_irq_enabled(state->k051960))
			cpu_set_input_line(device, KONAMI_IRQ_LINE, HOLD_LINE);
	}
	else if (cpu_getiloops(device) % 2)
	{
		if (k051960_is_nmi_enabled(state->k051960))
			cpu_set_input_line(device, INPUT_LINE_NMI, PULSE_LINE);
	}
}

static WRITE8_HANDLER( chqflag_bankswitch_w )
{
	chqflag_state *state = (chqflag_state *)space->machine->driver_data;
	int bankaddress;
	UINT8 *RAM = memory_region(space->machine, "maincpu");

	/* bits 0-4 = ROM bank # (0x00-0x11) */
	bankaddress = 0x10000 + (data & 0x1f) * 0x4000;
	memory_set_bankptr(space->machine, "bank4", &RAM[bankaddress]);

	/* bit 5 = memory bank select */
	if (data & 0x20)
	{
		memory_install_read_bank(space, 0x1800, 0x1fff, 0, 0, "bank5");
		memory_install_write8_handler(space, 0x1800, 0x1fff, 0, 0, paletteram_xBBBBBGGGGGRRRRR_be_w);
		memory_set_bankptr(space->machine, "bank5", space->machine->generic.paletteram.v);

		if (state->k051316_readroms)
			memory_install_readwrite8_device_handler(space, state->k051316_1, 0x1000, 0x17ff, 0, 0, k051316_rom_r, k051316_w);	/* 051316 #1 (ROM test) */
		else
			memory_install_readwrite8_device_handler(space, state->k051316_1, 0x1000, 0x17ff, 0, 0, k051316_r, k051316_w);		/* 051316 #1 */
	}
	else
	{
		memory_install_readwrite_bank(space, 0x1000, 0x17ff, 0, 0, "bank1");				/* RAM */
		memory_install_readwrite_bank(space, 0x1800, 0x1fff, 0, 0, "bank2");				/* RAM */
	}

	/* other bits unknown/unused */
}

static WRITE8_HANDLER( chqflag_vreg_w )
{
	chqflag_state *state = (chqflag_state *)space->machine->driver_data;

	/* bits 0 & 1 = coin counters */
	coin_counter_w(space->machine, 1, data & 0x01);
	coin_counter_w(space->machine, 0, data & 0x02);

	/* bit 4 = enable rom reading thru K051316 #1 & #2 */
	state->k051316_readroms = (data & 0x10);

	if (state->k051316_readroms)
		memory_install_read8_device_handler(space, state->k051316_2, 0x2800, 0x2fff, 0, 0, k051316_rom_r);	/* 051316 (ROM test) */
	else
		memory_install_read8_device_handler(space, state->k051316_2, 0x2800, 0x2fff, 0, 0, k051316_r);		/* 051316 */

	/* Bits 3-7 probably control palette dimming in a similar way to TMNT2/Sunset Riders, */
	/* however I don't have enough evidence to determine the exact behaviour. */
	/* Bits 3 and 7 are set in night stages, where the background should get darker and */
	/* the headlight (which have the shadow bit set) become highlights */
	/* Maybe one of the bits inverts the SHAD line while the other darkens the background. */
	if (data & 0x08)
		palette_set_shadow_factor(space->machine, 1 / PALETTE_DEFAULT_SHADOW_FACTOR);
	else
		palette_set_shadow_factor(space->machine, PALETTE_DEFAULT_SHADOW_FACTOR);

	if ((data & 0x80) != state->last_vreg)
	{
		double brt = (data & 0x80) ? PALETTE_DEFAULT_SHADOW_FACTOR : 1.0;
		int i;

		state->last_vreg = data & 0x80;

		/* only affect the background */
		for (i = 512; i < 1024; i++)
			palette_set_pen_contrast(space->machine, i, brt);
	}

//if ((data & 0xf8) && (data & 0xf8) != 0x88)
//  popmessage("chqflag_vreg_w %02x",data);


	/* other bits unknown. bit 5 is used. */
}

static WRITE8_HANDLER( select_analog_ctrl_w )
{
	chqflag_state *state = (chqflag_state *)space->machine->driver_data;
	state->analog_ctrl = data;
}

static READ8_HANDLER( analog_read_r )
{
	chqflag_state *state = (chqflag_state *)space->machine->driver_data;
	switch (state->analog_ctrl & 0x03)
	{
		case 0x00: return (state->accel = input_port_read(space->machine, "IN3"));	/* accelerator */
		case 0x01: return (state->wheel = input_port_read(space->machine, "IN4"));	/* steering */
		case 0x02: return state->accel;						/* accelerator (previous?) */
		case 0x03: return state->wheel;						/* steering (previous?) */
	}

	return 0xff;
}

static WRITE8_HANDLER( chqflag_sh_irqtrigger_w )
{
	chqflag_state *state = (chqflag_state *)space->machine->driver_data;
	soundlatch2_w(space, 0, data);
	cpu_set_input_line(state->audiocpu, 0, HOLD_LINE);
}


/****************************************************************************/

static ADDRESS_MAP_START( chqflag_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_RAM												/* RAM */
	AM_RANGE(0x1000, 0x17ff) AM_RAMBANK("bank1")								/* banked RAM (RAM/051316 (chip 1)) */
	AM_RANGE(0x1800, 0x1fff) AM_RAMBANK("bank2")								/* palette + RAM */
	AM_RANGE(0x2000, 0x2007) AM_DEVREADWRITE("k051960", k051937_r, k051937_w)					/* Sprite control registers */
	AM_RANGE(0x2400, 0x27ff) AM_DEVREADWRITE("k051960", k051960_r, k051960_w)					/* Sprite RAM */
	AM_RANGE(0x2800, 0x2fff) AM_READ_BANK("bank3") AM_DEVWRITE("k051316_2", k051316_w)		/* 051316 zoom/rotation (chip 2) */
	AM_RANGE(0x3000, 0x3000) AM_WRITE(soundlatch_w)								/* sound code # */
	AM_RANGE(0x3001, 0x3001) AM_WRITE(chqflag_sh_irqtrigger_w)					/* cause interrupt on audio CPU */
	AM_RANGE(0x3002, 0x3002) AM_WRITE(chqflag_bankswitch_w)						/* bankswitch control */
	AM_RANGE(0x3003, 0x3003) AM_WRITE(chqflag_vreg_w)							/* enable K051316 ROM reading */
	AM_RANGE(0x3100, 0x3100) AM_READ_PORT("DSW1")								/* DIPSW #1  */
	AM_RANGE(0x3200, 0x3200) AM_READ_PORT("IN1")								/* COINSW, STARTSW, test mode */
	AM_RANGE(0x3201, 0x3201) AM_READ_PORT("IN0")								/* DIPSW #3, SW 4 */
	AM_RANGE(0x3203, 0x3203) AM_READ_PORT("DSW2")								/* DIPSW #2 */
	AM_RANGE(0x3300, 0x3300) AM_WRITE(watchdog_reset_w)							/* watchdog timer */
	AM_RANGE(0x3400, 0x341f) AM_DEVREADWRITE("k051733", k051733_r, k051733_w)					/* 051733 (protection) */
	AM_RANGE(0x3500, 0x350f) AM_DEVWRITE("k051316_1", k051316_ctrl_w)							/* 051316 control registers (chip 1) */
	AM_RANGE(0x3600, 0x360f) AM_DEVWRITE("k051316_2", k051316_ctrl_w)							/* 051316 control registers (chip 2) */
	AM_RANGE(0x3700, 0x3700) AM_WRITE(select_analog_ctrl_w)						/* select accelerator/wheel */
	AM_RANGE(0x3701, 0x3701) AM_READ_PORT("IN2")								/* Brake + Shift + ? */
	AM_RANGE(0x3702, 0x3702) AM_READWRITE(analog_read_r, select_analog_ctrl_w)	/* accelerator/wheel */
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank4")										/* banked ROM */
	AM_RANGE(0x8000, 0xffff) AM_ROM												/* ROM */
ADDRESS_MAP_END

static WRITE8_HANDLER( k007232_bankswitch_w )
{
	chqflag_state *state = (chqflag_state *)space->machine->driver_data;
	int bank_A, bank_B;

	/* banks # for the 007232 (chip 1) */
	bank_A = ((data >> 4) & 0x03);
	bank_B = ((data >> 6) & 0x03);
	k007232_set_bank(state->k007232_1, bank_A, bank_B);

	/* banks # for the 007232 (chip 2) */
	bank_A = ((data >> 0) & 0x03);
	bank_B = ((data >> 2) & 0x03);
	k007232_set_bank(state->k007232_2, bank_A, bank_B);
}

static ADDRESS_MAP_START( chqflag_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM /* ROM */
	AM_RANGE(0x8000, 0x87ff) AM_RAM /* RAM */
	AM_RANGE(0x9000, 0x9000) AM_WRITE(k007232_bankswitch_w)	/* 007232 bankswitch */
	AM_RANGE(0xa000, 0xa00d) AM_DEVREADWRITE("k007232_1", k007232_r, k007232_w)	/* 007232 (chip 1) */
	AM_RANGE(0xa01c, 0xa01c) AM_DEVWRITE("k007232_2", k007232_extvolume_w)	/* extra volume, goes to the 007232 w/ A11 */
	AM_RANGE(0xb000, 0xb00d) AM_DEVREADWRITE("k007232_2", k007232_r, k007232_w)	/* 007232 (chip 2) */
	AM_RANGE(0xc000, 0xc001) AM_DEVREADWRITE("ymsnd", ym2151_r, ym2151_w)	/* YM2151 */
	AM_RANGE(0xd000, 0xd000) AM_READ(soundlatch_r)			/* soundlatch_r */
	AM_RANGE(0xe000, 0xe000) AM_READ(soundlatch2_r)         /* engine sound volume */
	AM_RANGE(0xf000, 0xf000) AM_WRITENOP					/* ??? */
ADDRESS_MAP_END


static INPUT_PORTS_START( chqflag )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, "Invalid" )
	/* Invalid = both coin slots disabled */

	PORT_START("DSW2")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW2:1" )	/* Manual says it's not used */
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW2:2" )	/* Manual says it's not used */
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW2:3" )	/* Manual says it's not used */
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW2:4" )	/* Manual says it's not used */
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5" )	/* Manual says it's not used */
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(	0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(	0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(	0x20, DEF_STR( Difficult ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(	0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )

	PORT_START("IN0")
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW3:4" )	/* Manual says it's not used */

	PORT_START("IN1")
	/* COINSW + STARTSW */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	/* DIPSW #3 */
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW3:1" )	/* Manual says it's not used */
	PORT_DIPNAME( 0x40, 0x40, "Title" ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(	0x40, "Chequered Flag" )
	PORT_DIPSETTING(	0x00, "Checkered Flag" )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW3:3" )

	PORT_START("IN2")	/* Brake, Shift + ??? */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* if this is set, it goes directly to test mode */
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* if bit 7 == 0, the game resets */

	PORT_START("IN3")	/* Accelerator */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(5)

	PORT_START("IN4")	/* Driving wheel */
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x10,0xef) PORT_SENSITIVITY(80) PORT_KEYDELTA(8)
INPUT_PORTS_END



static void chqflag_ym2151_irq_w( running_device *device, int data )
{
	chqflag_state *state = (chqflag_state *)device->machine->driver_data;
	cpu_set_input_line(state->audiocpu, INPUT_LINE_NMI, data ? ASSERT_LINE : CLEAR_LINE);
}


static const ym2151_interface ym2151_config =
{
	chqflag_ym2151_irq_w
};

static void volume_callback0( running_device *device, int v )
{
	k007232_set_volume(device, 0, (v & 0x0f) * 0x11, 0);
	k007232_set_volume(device, 1, 0, (v >> 4) * 0x11);
}

static WRITE8_DEVICE_HANDLER( k007232_extvolume_w )
{
	k007232_set_volume(device, 1, (data & 0x0f) * 0x11/2, (data >> 4) * 0x11/2);
}

static void volume_callback1( running_device *device, int v )
{
	k007232_set_volume(device, 0, (v & 0x0f) * 0x11/2, (v >> 4) * 0x11/2);
}

static const k007232_interface k007232_interface_1 =
{
	volume_callback0
};

static const k007232_interface k007232_interface_2 =
{
	volume_callback1
};

static const k051960_interface chqflag_k051960_intf =
{
	"gfx1", 0,
	NORMAL_PLANE_ORDER,
	KONAMI_ROM_DEINTERLEAVE_2,
	chqflag_sprite_callback
};

static const k051316_interface chqflag_k051316_intf_1 =
{
	"gfx2", 1,
	4, FALSE, 0,
	0, 7, 0,
	chqflag_zoom_callback_0
};

static const k051316_interface chqflag_k051316_intf_2 =
{
	"gfx3", 2,
	8, TRUE, 0xc0,
	1, 0, 0,
	chqflag_zoom_callback_1
};

static MACHINE_START( chqflag )
{
	chqflag_state *state = (chqflag_state *)machine->driver_data;
	UINT8 *ROM = memory_region(machine, "maincpu");

	memory_configure_bank(machine, "bank1", 0, 4, &ROM[0x10000], 0x2000);

	state->maincpu = machine->device("maincpu");
	state->audiocpu = machine->device("audiocpu");
	state->k051316_1 = machine->device("k051316_1");
	state->k051316_2 = machine->device("k051316_2");
	state->k051960 = machine->device("k051960");
	state->k007232_1 = machine->device("k007232_1");
	state->k007232_2 = machine->device("k007232_2");

	state_save_register_global(machine, state->k051316_readroms);
	state_save_register_global(machine, state->last_vreg);
	state_save_register_global(machine, state->analog_ctrl);
	state_save_register_global(machine, state->accel);
	state_save_register_global(machine, state->wheel);
}

static MACHINE_RESET( chqflag )
{
	chqflag_state *state = (chqflag_state *)machine->driver_data;

	state->k051316_readroms = 0;
	state->last_vreg = 0;
	state->analog_ctrl = 0;
	state->accel = 0;
	state->wheel = 0;
}

static MACHINE_DRIVER_START( chqflag )

	/* driver data */
	MDRV_DRIVER_DATA(chqflag_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", KONAMI,XTAL_24MHz/8)	/* 052001 (verified on pcb) */
	MDRV_CPU_PROGRAM_MAP(chqflag_map)
	MDRV_CPU_VBLANK_INT_HACK(chqflag_interrupt,16)	/* ? */

	MDRV_CPU_ADD("audiocpu", Z80, XTAL_3_579545MHz) /* verified on pcb */
	MDRV_CPU_PROGRAM_MAP(chqflag_sound_map)

	MDRV_QUANTUM_TIME(HZ(600))

	MDRV_MACHINE_START(chqflag)
	MDRV_MACHINE_RESET(chqflag)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_HAS_SHADOWS)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(12*8, (64-14)*8-1, 2*8, 30*8-1 )

	MDRV_PALETTE_LENGTH(1024)

	MDRV_VIDEO_START(chqflag)
	MDRV_VIDEO_UPDATE(chqflag)

	MDRV_K051960_ADD("k051960", chqflag_k051960_intf)
	MDRV_K051316_ADD("k051316_1", chqflag_k051316_intf_1)
	MDRV_K051316_ADD("k051316_2", chqflag_k051316_intf_2)
	MDRV_K051733_ADD("k051733")

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2151, XTAL_3_579545MHz) /* verified on pcb */
	MDRV_SOUND_CONFIG(ym2151_config)
	MDRV_SOUND_ROUTE(0, "lspeaker", 0.80)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.80)

	MDRV_SOUND_ADD("k007232_1", K007232, XTAL_3_579545MHz) /* verified on pcb */
	MDRV_SOUND_CONFIG(k007232_interface_1)
	MDRV_SOUND_ROUTE(0, "lspeaker", 0.20)
	MDRV_SOUND_ROUTE(0, "rspeaker", 0.20)
	MDRV_SOUND_ROUTE(1, "lspeaker", 0.20)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.20)

	MDRV_SOUND_ADD("k007232_2", K007232, XTAL_3_579545MHz) /* verified on pcb */
	MDRV_SOUND_CONFIG(k007232_interface_2)
	MDRV_SOUND_ROUTE(0, "lspeaker", 0.20)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.20)
MACHINE_DRIVER_END

ROM_START( chqflag )
	ROM_REGION( 0x58000, "maincpu", 0 )	/* 052001 code */
	ROM_LOAD( "717h02",		0x050000, 0x008000, CRC(f5bd4e78) SHA1(7bab02152d055a6c3a322c88e7ee0b85a39d8ef2) )	/* banked ROM */
	ROM_CONTINUE(			0x008000, 0x008000 )				/* fixed ROM */
	ROM_LOAD( "717e10",		0x010000, 0x040000, CRC(72fc56f6) SHA1(433ea9a33f0230e046c731c70060f6a38db14ac7) )	/* banked ROM */
	/* extra memory for banked RAM */

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* 64k for the SOUND CPU */
	ROM_LOAD( "717e01",		0x000000, 0x008000, CRC(966b8ba8) SHA1(ab7448cb61fa5922b1d8ae5f0d0f42d734ed4f93) )

	ROM_REGION( 0x100000, "gfx1", 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD( "717e04",		0x000000, 0x080000, CRC(1a50a1cc) SHA1(bc16fab84c637ed124e37b115ddc0149560b727d) )	/* sprites */
	ROM_LOAD( "717e05",		0x080000, 0x080000, CRC(46ccb506) SHA1(3ed1f54744fc5cdc0f48e42f250c366267a8199a) )	/* sprites */

	ROM_REGION( 0x020000, "gfx2", 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD( "717e06",		0x000000, 0x020000, CRC(1ec26c7a) SHA1(05b5b522c5ebf5d0a71a7fc39ec9382008ef33c8) )	/* zoom/rotate (N16) */

	ROM_REGION( 0x100000, "gfx3", 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD( "717e07",		0x000000, 0x040000, CRC(b9a565a8) SHA1(a11782f7336e5ad58a4c6ea81f2eeac35d5e7d0a) )	/* zoom/rotate (L20) */
	ROM_LOAD( "717e08",		0x040000, 0x040000, CRC(b68a212e) SHA1(b2bd121a43552c3ade528ac763a0df40c3e648e0) )	/* zoom/rotate (L22) */
	ROM_LOAD( "717e11",		0x080000, 0x040000, CRC(ebb171ec) SHA1(d65d4a6b169ce03e4427b2a397484634f938236b) )	/* zoom/rotate (N20) */
	ROM_LOAD( "717e12",		0x0c0000, 0x040000, CRC(9269335d) SHA1(af298c8cff50d707d6abc806065f8e931f975dc0) )	/* zoom/rotate (N22) */

	ROM_REGION( 0x080000, "k007232_1", 0 )	/* 007232 data (chip 1) */
	ROM_LOAD( "717e03",		0x000000, 0x080000, CRC(ebe73c22) SHA1(fad3334e5e91bf8d11b74ffdbbfd57567e6f6f8c) )

	ROM_REGION( 0x080000, "k007232_2", 0 )	/* 007232 data (chip 2) */
	ROM_LOAD( "717e09",		0x000000, 0x080000, CRC(d74e857d) SHA1(00c851c857650d67fc4caccea4461d99be4acb3c) )
ROM_END

ROM_START( chqflagj )
	ROM_REGION( 0x58000, "maincpu", 0 )	/* 052001 code */
	ROM_LOAD( "717j02.bin",	0x050000, 0x008000, CRC(05355daa) SHA1(130ddbc289c077565e44f33c63a63963e6417e19) )	/* banked ROM */
	ROM_CONTINUE(			0x008000, 0x008000 )				/* fixed ROM */
	ROM_LOAD( "717e10",		0x010000, 0x040000, CRC(72fc56f6) SHA1(433ea9a33f0230e046c731c70060f6a38db14ac7) )	/* banked ROM */
	/* extra memory for banked RAM */

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* 64k for the SOUND CPU */
	ROM_LOAD( "717e01",		0x000000, 0x008000, CRC(966b8ba8) SHA1(ab7448cb61fa5922b1d8ae5f0d0f42d734ed4f93) )

	ROM_REGION( 0x100000, "gfx1", 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD( "717e04",		0x000000, 0x080000, CRC(1a50a1cc) SHA1(bc16fab84c637ed124e37b115ddc0149560b727d) )	/* sprites */
	ROM_LOAD( "717e05",		0x080000, 0x080000, CRC(46ccb506) SHA1(3ed1f54744fc5cdc0f48e42f250c366267a8199a) )	/* sprites */

	ROM_REGION( 0x020000, "gfx2", 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD( "717e06",		0x000000, 0x020000, CRC(1ec26c7a) SHA1(05b5b522c5ebf5d0a71a7fc39ec9382008ef33c8) )	/* zoom/rotate (N16) */

	ROM_REGION( 0x100000, "gfx3", 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD( "717e07",		0x000000, 0x040000, CRC(b9a565a8) SHA1(a11782f7336e5ad58a4c6ea81f2eeac35d5e7d0a) )	/* zoom/rotate (L20) */
	ROM_LOAD( "717e08",		0x040000, 0x040000, CRC(b68a212e) SHA1(b2bd121a43552c3ade528ac763a0df40c3e648e0) )	/* zoom/rotate (L22) */
	ROM_LOAD( "717e11",		0x080000, 0x040000, CRC(ebb171ec) SHA1(d65d4a6b169ce03e4427b2a397484634f938236b) )	/* zoom/rotate (N20) */
	ROM_LOAD( "717e12",		0x0c0000, 0x040000, CRC(9269335d) SHA1(af298c8cff50d707d6abc806065f8e931f975dc0) )	/* zoom/rotate (N22) */

	ROM_REGION( 0x080000, "k007232_1", 0 )	/* 007232 data (chip 1) */
	ROM_LOAD( "717e03",		0x000000, 0x080000, CRC(ebe73c22) SHA1(fad3334e5e91bf8d11b74ffdbbfd57567e6f6f8c) )

	ROM_REGION( 0x080000, "k007232_2", 0 )	/* 007232 data (chip 2) */
	ROM_LOAD( "717e09",		0x000000, 0x080000, CRC(d74e857d) SHA1(00c851c857650d67fc4caccea4461d99be4acb3c) )
ROM_END


GAMEL( 1988, chqflag,  0,       chqflag, chqflag, 0, ROT90, "Konami", "Chequered Flag", GAME_UNEMULATED_PROTECTION | GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE, layout_chqflag )
GAMEL( 1988, chqflagj, chqflag, chqflag, chqflag, 0, ROT90, "Konami", "Chequered Flag (Japan)", GAME_UNEMULATED_PROTECTION | GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE, layout_chqflag )
