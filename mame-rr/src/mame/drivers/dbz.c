/*
  Dragonball Z                  (c) 1993 Banpresto
  Dragonball Z 2 - Super Battle (c) 1994 Banpresto

  Driver by David Haywood, R. Belmont and Pierpaolo Prazzoli

  MC68000 + Konami Xexex-era video hardware and system controller ICs
  Z80 + YM2151 + OKIM6295 for sound

  Note: game has an extremely complete test mode, it's beautiful for emulation.
        flip the DIP and check it out!

  TODO:
    - Self Test Fails
    - Banpresto logo in DBZ has bad colors after 1 run of the attract mode because
      it's associated to the wrong logical tilemap and the same happens in DBZ2
      test mode. It should be a bug in K056832 emulation.

PCB Layout:

BP924-1  PWB250248D (note PCB is identical to DBZ2 also)
|-------------------------------------------------------|
| YM3014  Z80    32MHz  053252       222A05   222A07    |
|   YM2151 5168                      222A04   222A06    |
| 1.056kHz 5168                                         |
|  M6295   222A10                           5864        |
|   222A03     68000                 2018   5864 053246A|
|J          222A11  222A12           2018               |
|A 5864        *       *                                |
|M 5864     62256   62256                               |
|M                                               053247A|
|A                                                      |
|                                053936 053936     2018 |
|    053251  053251                                2018 |
|                                        CY7C128        |
|       054157  054156  5864     CY7C128 CY7C128 CY7C128|
|       222A01  222A02  5864     CY7C128                |
| DSW2  DSW1            5864     CY7C128 222A08  222A09 |
|                                           *       *   |
|-------------------------------------------------------|

Notes:
      68k clock: 16.000MHz
      Z80 clock: 4.000MHz
   YM2151 clock: 4.000MHz
    M6295 clock: 1.056MHz (sample rate = /132)
          Vsync: 55Hz
          Hsync: 15.36kHz
              *: unpopulated ROM positions on DBZ

*/

#include "emu.h"
#include "deprecat.h"

#include "video/konicdev.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "sound/2151intf.h"
#include "sound/okim6295.h"
#include "includes/dbz.h"


static INTERRUPT_GEN( dbz_interrupt )
{
	dbz_state *state = (dbz_state *)device->machine->driver_data;

	switch (cpu_getiloops(device))
	{
		case 0:
			cpu_set_input_line(device, M68K_IRQ_2, HOLD_LINE);
			break;

		case 1:
			if (k053246_is_irq_enabled(state->k053246))
				cpu_set_input_line(device, M68K_IRQ_4, HOLD_LINE);
			break;
	}
}

#if 0
static READ16_HANDLER( dbzcontrol_r )
{
	dbz_state *state = (dbz_state *)space->machine->driver_data;
	return state->control;
}
#endif

static WRITE16_HANDLER( dbzcontrol_w )
{
	dbz_state *state = (dbz_state *)space->machine->driver_data;
	/* bit 10 = enable '246 readback */

	COMBINE_DATA(&state->control);

	if (data & 0x400)
		k053246_set_objcha_line(state->k053246, ASSERT_LINE);
	else
		k053246_set_objcha_line(state->k053246, CLEAR_LINE);

	coin_counter_w(space->machine, 0, data & 1);
	coin_counter_w(space->machine, 1, data & 2);
}

static WRITE16_HANDLER( dbz_sound_command_w )
{
	soundlatch_w(space, 0, data >> 8);
}

static WRITE16_HANDLER( dbz_sound_cause_nmi )
{
	dbz_state *state = (dbz_state *)space->machine->driver_data;
	cpu_set_input_line(state->audiocpu, INPUT_LINE_NMI, PULSE_LINE);
}

static void dbz_sound_irq( running_device *device, int irq )
{
	dbz_state *state = (dbz_state *)device->machine->driver_data;

	if (irq)
		cpu_set_input_line(state->audiocpu, 0, ASSERT_LINE);
	else
		cpu_set_input_line(state->audiocpu, 0, CLEAR_LINE);
}

static ADDRESS_MAP_START( dbz_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x480000, 0x48ffff) AM_RAM
	AM_RANGE(0x490000, 0x491fff) AM_DEVREADWRITE("k056832", k056832_ram_word_r, k056832_ram_word_w)	// '157 RAM is mirrored twice
	AM_RANGE(0x492000, 0x493fff) AM_DEVREADWRITE("k056832", k056832_ram_word_r, k056832_ram_word_w)
	AM_RANGE(0x498000, 0x49ffff) AM_DEVREAD("k056832", k056832_rom_word_8000_r)	// code near a60 in dbz2, subroutine at 730 in dbz
	AM_RANGE(0x4a0000, 0x4a0fff) AM_DEVREADWRITE("k053246", k053247_word_r, k053247_word_w)
	AM_RANGE(0x4a1000, 0x4a3fff) AM_RAM
	AM_RANGE(0x4a8000, 0x4abfff) AM_RAM_WRITE(paletteram16_xRRRRRGGGGGBBBBB_word_w) AM_BASE_GENERIC(paletteram) // palette
	AM_RANGE(0x4c0000, 0x4c0001) AM_DEVREAD("k053246", k053246_word_r)
	AM_RANGE(0x4c0000, 0x4c0007) AM_DEVWRITE("k053246", k053246_word_w)
	AM_RANGE(0x4c4000, 0x4c4007) AM_DEVWRITE("k053246", k053246_word_w)
	AM_RANGE(0x4c8000, 0x4c8007) AM_DEVWRITE("k056832", k056832_b_word_w)
	AM_RANGE(0x4cc000, 0x4cc03f) AM_DEVWRITE("k056832", k056832_word_w)
	AM_RANGE(0x4d0000, 0x4d001f) AM_DEVWRITE("k053936_1", k053936_ctrl_w)
	AM_RANGE(0x4d4000, 0x4d401f) AM_DEVWRITE("k053936_2", k053936_ctrl_w)
	AM_RANGE(0x4e0000, 0x4e0001) AM_READ_PORT("P1_P2")
	AM_RANGE(0x4e0002, 0x4e0003) AM_READ_PORT("SYSTEM_DSW1")
	AM_RANGE(0x4e4000, 0x4e4001) AM_READ_PORT("DSW2")
	AM_RANGE(0x4e8000, 0x4e8001) AM_WRITENOP
	AM_RANGE(0x4ec000, 0x4ec001) AM_WRITE(dbzcontrol_w)
	AM_RANGE(0x4f0000, 0x4f0001) AM_WRITE(dbz_sound_command_w)
	AM_RANGE(0x4f4000, 0x4f4001) AM_WRITE(dbz_sound_cause_nmi)
	AM_RANGE(0x4f8000, 0x4f801f) AM_WRITENOP		// 251 #1
	AM_RANGE(0x4fc000, 0x4fc01f) AM_DEVWRITE("k053251", k053251_lsb_w)	// 251 #2

	AM_RANGE(0x500000, 0x501fff) AM_RAM_WRITE(dbz_bg2_videoram_w) AM_BASE_MEMBER(dbz_state, bg2_videoram)
	AM_RANGE(0x508000, 0x509fff) AM_RAM_WRITE(dbz_bg1_videoram_w) AM_BASE_MEMBER(dbz_state, bg1_videoram)
	AM_RANGE(0x510000, 0x513fff) AM_DEVREADWRITE("k053936_1", k053936_linectrl_r, k053936_linectrl_w) // ?? guess, it might not be
	AM_RANGE(0x518000, 0x51bfff) AM_DEVREADWRITE("k053936_2", k053936_linectrl_r, k053936_linectrl_w) // ?? guess, it might not be
	AM_RANGE(0x600000, 0x6fffff) AM_READNOP 			// PSAC 1 ROM readback window
	AM_RANGE(0x700000, 0x7fffff) AM_READNOP 			// PSAC 2 ROM readback window
ADDRESS_MAP_END

/* dbz sound */
/* IRQ: from YM2151.  NMI: from 68000.  Port 0: write to ack NMI */

static ADDRESS_MAP_START( dbz_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_RAM
	AM_RANGE(0xc000, 0xc001) AM_DEVREADWRITE("ymsnd", ym2151_r, ym2151_w)
	AM_RANGE(0xd000, 0xd002) AM_DEVREADWRITE("oki", okim6295_r, okim6295_w)
	AM_RANGE(0xe000, 0xe001) AM_READ(soundlatch_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( dbz_sound_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITENOP
ADDRESS_MAP_END

/**********************************************************************************/


static INPUT_PORTS_START( dbz )
	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM_DSW1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0008, IP_ACTIVE_LOW )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) )	// I think this is right, but can't stomach the game long enough to check
	PORT_DIPSETTING(      0x0100, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPUNKNOWN( 0x0400, 0x0400 )						// seems unused
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Flip_Screen ) )	// Definitely correct
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNKNOWN( 0x1000, 0x1000 )
	PORT_SERVICE( 0x2000, 0x2000 )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Language ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( English ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Japanese ) )
	PORT_DIPNAME( 0x8000, 0x0000, "Mask ROM Test" )			//NOP'd
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_BIT( 0x00ff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(custom_port_read, "FAKE")
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(custom_port_read, "FAKE")

	PORT_START("FAKE")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )
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
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )
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
	PORT_DIPSETTING(    0x00, "No Coin B" )
	/* "No Coin B" = coins produce sound, but no effect on coin counter */
INPUT_PORTS_END

static INPUT_PORTS_START( dbz2 )
	PORT_INCLUDE( dbz )

	PORT_MODIFY("SYSTEM_DSW1")
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Level_Select ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

/**********************************************************************************/

static const ym2151_interface ym2151_config =
{
	dbz_sound_irq
};

/**********************************************************************************/

static const gfx_layout bglayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4, 8*4,
	  9*4, 10*4, 11*4, 12*4, 13*4, 14*4, 15*4 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
			8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	128*8
};

static GFXDECODE_START( dbz )
	GFXDECODE_ENTRY( "gfx3", 0, bglayout, 0, 512 )
	GFXDECODE_ENTRY( "gfx4", 0, bglayout, 0, 512 )
GFXDECODE_END

/**********************************************************************************/

static const k056832_interface dbz_k056832_intf =
{
	"gfx1", 2,
	K056832_BPP_4,
	1, 1,
	KONAMI_ROM_DEINTERLEAVE_2,
	dbz_tile_callback, "none"
};

static const k053247_interface dbz_k053246_intf =
{
	"screen",
	"gfx2", 3,
	NORMAL_PLANE_ORDER,
	-52, 16,
	KONAMI_ROM_DEINTERLEAVE_NONE,
	dbz_sprite_callback
};

/* both k053936 use the same wrap/offs */
static const k053936_interface dbz_k053936_intf =
{
	1, -46, -16
};


static MACHINE_START( dbz )
{
	dbz_state *state = (dbz_state *)machine->driver_data;

	state->maincpu = machine->device("maincpu");
	state->audiocpu = machine->device("audiocpu");
	state->k053936_1 = machine->device("k053936_1");
	state->k053936_2 = machine->device("k053936_2");
	state->k056832 = machine->device("k056832");
	state->k053246 = machine->device("k053246");
	state->k053251 = machine->device("k053251");

	state_save_register_global(machine, state->control);
	state_save_register_global(machine, state->sprite_colorbase);
	state_save_register_global_array(machine, state->layerpri);
	state_save_register_global_array(machine, state->layer_colorbase);
}

static MACHINE_RESET( dbz )
{
	dbz_state *state = (dbz_state *)machine->driver_data;
	int i;

	for (i = 0; i < 5; i++)
		state->layerpri[i] = 0;

	for (i = 0; i < 6; i++)
		state->layer_colorbase[i] = 0;

	state->sprite_colorbase = 0;
	state->control = 0;
}

static MACHINE_DRIVER_START( dbz )

	/* driver data */
	MDRV_DRIVER_DATA(dbz_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 16000000)
	MDRV_CPU_PROGRAM_MAP(dbz_map)
	MDRV_CPU_VBLANK_INT_HACK(dbz_interrupt,2)

	MDRV_CPU_ADD("audiocpu", Z80, 4000000)
	MDRV_CPU_PROGRAM_MAP(dbz_sound_map)
	MDRV_CPU_IO_MAP(dbz_sound_io_map)

	MDRV_MACHINE_START(dbz)
	MDRV_MACHINE_RESET(dbz)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_HAS_SHADOWS)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(55)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0, 48*8-1, 0, 32*8-1)

	MDRV_GFXDECODE(dbz)

	MDRV_VIDEO_START(dbz)
	MDRV_VIDEO_UPDATE(dbz)
	MDRV_PALETTE_LENGTH(0x4000/2)

	MDRV_K056832_ADD("k056832", dbz_k056832_intf)
	MDRV_K053246_ADD("k053246", dbz_k053246_intf)
	MDRV_K053251_ADD("k053251")
	MDRV_K053936_ADD("k053936_1", dbz_k053936_intf)
	MDRV_K053936_ADD("k053936_2", dbz_k053936_intf)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2151, 4000000)
	MDRV_SOUND_CONFIG(ym2151_config)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)

	MDRV_OKIM6295_ADD("oki", 1056000, OKIM6295_PIN7_HIGH)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
MACHINE_DRIVER_END

/**********************************************************************************/

ROM_START( dbz )
	/* main program */
	ROM_REGION( 0x400000, "maincpu", 0)
	ROM_LOAD16_BYTE( "222a11.9e", 0x000000, 0x80000, CRC(60c7d9b2) SHA1(718ef89e89b3943845e91bedfc5c1d26229f9fe5) )
	ROM_LOAD16_BYTE( "222a12.9f", 0x000001, 0x80000, CRC(6ebc6853) SHA1(e9b2068246228968cc6b8554215563cacaa5ba9f) )

	ROM_REGION( 0x400000, "user1", 0)
	ROM_LOAD16_BYTE( "222a11.9e", 0x000000, 0x80000, CRC(60c7d9b2) SHA1(718ef89e89b3943845e91bedfc5c1d26229f9fe5) )
	ROM_LOAD16_BYTE( "222a12.9f", 0x000001, 0x80000, CRC(6ebc6853) SHA1(e9b2068246228968cc6b8554215563cacaa5ba9f) )

	/* sound program */
	ROM_REGION( 0x010000, "audiocpu", 0 )
	ROM_LOAD("222a10.5e", 0x000000, 0x08000, CRC(1c93e30a) SHA1(8545a0ac5126b3c855e1901b186f57820699895d) )

	/* tiles */
	ROM_REGION( 0x400000, "gfx1", 0)
	ROM_LOAD( "222a01.27c", 0x000000, 0x200000, CRC(9fce4ed4) SHA1(81e19375b351ee247f066434dd595149333d73c5) )
	ROM_LOAD( "222a02.27e", 0x200000, 0x200000, CRC(651acaa5) SHA1(33942a90fb294b5da6a48e5bfb741b31babca188) )

	/* sprites */
	ROM_REGION( 0x800000, "gfx2", 0)
	ROM_LOAD64_WORD( "222a04.3j", 0x000000, 0x200000, CRC(2533b95a) SHA1(35910836b6030130d742eae6c4bf1cdf1ff43fa4) )
	ROM_LOAD64_WORD( "222a05.1j", 0x000002, 0x200000, CRC(731b7f93) SHA1(b676fff2ede5aa72c49fe12736cd60766462fe0b) )
	ROM_LOAD64_WORD( "222a06.3l", 0x000004, 0x200000, CRC(97b767d3) SHA1(3d879c431586da2f88c632ab1a531b4a5ec96939) )
	ROM_LOAD64_WORD( "222a07.1l", 0x000006, 0x200000, CRC(430bc873) SHA1(ea483195bb7f20ef3df7cfba153e5f6f8d53e5f9) )

	/* K053536 PSAC-2 #1 */
	ROM_REGION( 0x200000, "gfx3", 0)
	ROM_LOAD( "222a08.25k", 0x000000, 0x200000, CRC(6410ee1b) SHA1(2296aafd3ba25f63a12130f7b58de53e88f14e92) )

	/* K053536 PSAC-2 #2 */
	ROM_REGION( 0x200000, "gfx4", 0)
	ROM_LOAD( "222a09.25l", 0x000000, 0x200000, CRC(f7b3f070) SHA1(50ebd8cfcda292a3df5664de50f9212108d58923) )

	/* sound data */
	ROM_REGION( 0x40000, "oki", 0)
	ROM_LOAD( "222a03.7c", 0x000000, 0x40000, CRC(1924467b) SHA1(57922090509bcc63b4783e8f2c5e95afd2090b87) )
ROM_END

ROM_START( dbz2 )
	/* main program */
	ROM_REGION( 0x400000, "maincpu", 0)
	ROM_LOAD16_BYTE( "a9e.9e", 0x000000, 0x80000, CRC(e6a142c9) SHA1(7951c8f7036a67a0cd3260f434654820bf3e603f) )
	ROM_LOAD16_BYTE( "a9f.9f", 0x000001, 0x80000, CRC(76cac399) SHA1(af6daa1f8b87c861dc62adef5ca029190c3cb9ae) )

	/* sound program */
	ROM_REGION( 0x010000, "audiocpu", 0 )
	ROM_LOAD("s-001.5e", 0x000000, 0x08000, CRC(154e6d03) SHA1(db15c20982692271f40a733dfc3f2486221cd604) )

	/* tiles */
	ROM_REGION( 0x400000, "gfx1", 0)
	ROM_LOAD( "ds-b01.27c", 0x000000, 0x200000, CRC(8dc39972) SHA1(c6e3d4e0ff069e08bdb68e2b0ad24cc7314e4e93) )
	ROM_LOAD( "ds-b02.27e", 0x200000, 0x200000, CRC(7552f8cd) SHA1(1f3beffe9733b1a18d44b5e8880ff1cc97e7a8ab) )

	/* sprites */
	ROM_REGION( 0x800000, "gfx2", 0)
	ROM_LOAD64_WORD( "ds-o01.3j", 0x000000, 0x200000, CRC(d018531f) SHA1(d4082fe28e9f1f3f35aa75b4be650cadf1cef192) )
	ROM_LOAD64_WORD( "ds-o02.1j", 0x000002, 0x200000, CRC(5a0f1ebe) SHA1(3bb9e1389299dc046a24740ef1a1c543e44b5c37) )
	ROM_LOAD64_WORD( "ds-o03.3l", 0x000004, 0x200000, CRC(ddc3bef1) SHA1(69638ef53f627a238a12b6c206d57faadf894893) )
	ROM_LOAD64_WORD( "ds-o04.1l", 0x000006, 0x200000, CRC(b5df6676) SHA1(194cfce460ccd29e2cceec577aae4ec936ae88e5) )

	/* K053536 PSAC-2 #1 */
	ROM_REGION( 0x400000, "gfx3", 0)
	ROM_LOAD( "ds-p01.25k", 0x000000, 0x200000, CRC(1c7aad68) SHA1(a5296cf12cec262eede55397ea929965576fea81) )
	ROM_LOAD( "ds-p02.27k", 0x200000, 0x200000, CRC(e4c3a43b) SHA1(f327f75fe82f8aafd2cfe6bdd3a426418615974b) )

	/* K053536 PSAC-2 #2 */
	ROM_REGION( 0x400000, "gfx4", 0)
	ROM_LOAD( "ds-p03.25l", 0x000000, 0x200000, CRC(1eaa671b) SHA1(1875eefc6f2c3fc8feada56bfa6701144e8ef64b) )
	ROM_LOAD( "ds-p04.27l", 0x200000, 0x200000, CRC(5845ff98) SHA1(73b4c3f439321ce9c462119fe933e7cbda8cd498) )

	/* sound data */
	ROM_REGION( 0x40000, "oki", 0)
	ROM_LOAD( "pcm.7c", 0x000000, 0x40000, CRC(b58c884a) SHA1(0e2a7267e9dff29c9af25558081ec9d56629bc43) )
ROM_END

static DRIVER_INIT( dbz )
{
	UINT16 *ROM;

	ROM = (UINT16 *)memory_region(machine, "maincpu");

	// nop out dbz1's mask rom test
	// tile ROM test
	ROM[0x790/2] = 0x4e71;
	ROM[0x792/2] = 0x4e71;
	// PSAC2 ROM test
	ROM[0x982/2] = 0x4e71;
	ROM[0x984/2] = 0x4e71;
	ROM[0x986/2] = 0x4e71;
	ROM[0x988/2] = 0x4e71;
	ROM[0x98a/2] = 0x4e71;
	ROM[0x98c/2] = 0x4e71;
	ROM[0x98e/2] = 0x4e71;
	ROM[0x990/2] = 0x4e71;
}

GAME( 1993, dbz,  0, dbz, dbz,  dbz,  ROT0, "Banpresto", "Dragonball Z", GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE )
GAME( 1994, dbz2, 0, dbz, dbz2, 0,    ROT0, "Banpresto", "Dragonball Z 2 - Super Battle", GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE )
