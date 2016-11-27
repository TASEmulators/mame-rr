/***************************************************************************

Buggy Challenge - (c) 1984 Taito Corporation

driver by Ernesto Corvi and Nicola Salmoria

TODO:
- I'm almost sure that I'm not handling the zoom x ROM table correctly. Gives
  reasonable results, though. I'm confident that the zoom y table handling is
  correct.
- Tilemap and sprite placement might not be accurate, there aren't many
  references.
- The gradient sky is completely wrong - it's more of a placeholder to show
  that it's supposed to be there. It is supposed to skew along with the
  background, and the gradient can move around (the latter doesn't seem to
  be used except for making it cover the whole screen on the title screen,
  and start at the middle during gameplay)
- Video driver is largely unoptimized
- Support for the 7630's controlling the sound chip outputs (bass/treble,
  volume) is completely missing.
- The sound Z80 seems to write answers for the main Z80, but the latter doesn't
  seem to read them.

Notes:
- There is also a 4-channel version of the sound board for the cockpit
  cabinet (ROMs not dumped)


Memory Map
----------
0000 - 3fff = ROM A22-04 (23)
4000 - 7fff = ROM A22-05 (22)
8000 - 87ff = RAM (36)
8800 - 8fff = RAM (35)

c800 - cbff = videoram
cc00 - cfff = videoram

d100 = /ANYOUT
    bit7 = lamp
    bit6 = lockout
    bit4 = OJMODE (sprite palette bank)
    bit3 = SKY OFF
    bit2 = /SN3OFF
    bit1 = flip screen X
    bit0 = flip screen Y
d200 = bank switch
    bit2 = Bank Select bit 1
    bit1 = Bank Select bit 0
    bit0 = EA13 (high/low part of banked ROM)
d300 = /TRESET (Watchdog reset?)
d301 = No name?
    bit6 = FLPF2 (W-6)
    bit5 = FLPE2 (W-5)
    bit4 = FLPD2 (W-4)
    bot2 = FLPF1 (W-3)
    bit1 = FLPE1 (W-2)
    bit0 = FLPD1 (W-1)
d302 - bit 0 = /RESET line on the 68705

d304 - d307 = SCCON1 to SCCON4

d613 = /SoundCS = /RESET line on all audio CPUs

d700 - d7ff = ( /VCRRQ - palette ram )

d800 - d8ff /ScrollRQ (S37)
da00 - daff /ScrollRQ (S37)
db00 - dbff /ScrollRQ (S37)

dcxx = /SPOSI (S36)

2008-07
Dip locations and factory settings verified from dip listing

***************************************************************************/

#include "emu.h"
#include "deprecat.h"
#include "cpu/z80/z80.h"
#include "cpu/m6805/m6805.h"
#include "sound/ay8910.h"
#include "sound/msm5232.h"
#include "includes/buggychl.h"

#include "buggychl.lh"


static WRITE8_HANDLER( bankswitch_w )
{
	memory_set_bank(space->machine, "bank1", data & 0x07);	// shall we check if data&7 < # banks?
}

static TIMER_CALLBACK( nmi_callback )
{
	buggychl_state *state = (buggychl_state *)machine->driver_data;

	if (state->sound_nmi_enable)
		cpu_set_input_line(state->audiocpu, INPUT_LINE_NMI, PULSE_LINE);
	else
		state->pending_nmi = 1;
}

static WRITE8_HANDLER( sound_command_w )
{
	soundlatch_w(space, 0, data);
	timer_call_after_resynch(space->machine, NULL, data, nmi_callback);
}

static WRITE8_HANDLER( nmi_disable_w )
{
	buggychl_state *state = (buggychl_state *)space->machine->driver_data;
	state->sound_nmi_enable = 0;
}

static WRITE8_HANDLER( nmi_enable_w )
{
	buggychl_state *state = (buggychl_state *)space->machine->driver_data;
	state->sound_nmi_enable = 1;
	if (state->pending_nmi)
	{
		cpu_set_input_line(state->audiocpu, INPUT_LINE_NMI, PULSE_LINE);
		state->pending_nmi = 0;
	}
}

static WRITE8_HANDLER( sound_enable_w )
{
	sound_global_enable(space->machine, data & 1);
}



static ADDRESS_MAP_START( buggychl_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM /* A22-04 (23) */
	AM_RANGE(0x4000, 0x7fff) AM_ROM /* A22-05 (22) */
	AM_RANGE(0x8000, 0x87ff) AM_RAM /* 6116 SRAM (36) */
	AM_RANGE(0x8800, 0x8fff) AM_RAM /* 6116 SRAM (35) */
	AM_RANGE(0x9000, 0x9fff) AM_WRITE(buggychl_sprite_lookup_w)
	AM_RANGE(0xa000, 0xbfff) AM_ROMBANK("bank1") AM_WRITE(buggychl_chargen_w) AM_BASE_MEMBER(buggychl_state, charram)
	AM_RANGE(0xc800, 0xcfff) AM_RAM AM_BASE_SIZE_MEMBER(buggychl_state, videoram, videoram_size)
	AM_RANGE(0xd100, 0xd100) AM_WRITE(buggychl_ctrl_w)
	AM_RANGE(0xd200, 0xd200) AM_WRITE(bankswitch_w)
	AM_RANGE(0xd300, 0xd300) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0xd303, 0xd303) AM_WRITE(buggychl_sprite_lookup_bank_w)
	AM_RANGE(0xd400, 0xd400) AM_READWRITE(buggychl_mcu_r, buggychl_mcu_w)
	AM_RANGE(0xd401, 0xd401) AM_READ(buggychl_mcu_status_r)
	AM_RANGE(0xd500, 0xd57f) AM_WRITEONLY AM_BASE_SIZE_MEMBER(buggychl_state, spriteram, spriteram_size)
	AM_RANGE(0xd600, 0xd600) AM_READ_PORT("DSW1")
	AM_RANGE(0xd601, 0xd601) AM_READ_PORT("DSW2")
	AM_RANGE(0xd602, 0xd602) AM_READ_PORT("DSW3")
	AM_RANGE(0xd603, 0xd603) AM_READ_PORT("IN0")	/* player inputs */
	AM_RANGE(0xd608, 0xd608) AM_READ_PORT("WHEEL")
	AM_RANGE(0xd609, 0xd609) AM_READ_PORT("IN1")	/* coin + accelerator */
//  AM_RANGE(0xd60a, 0xd60a) // other inputs, not used?
//  AM_RANGE(0xd60b, 0xd60b) // other inputs, not used?
	AM_RANGE(0xd610, 0xd610) AM_WRITE(sound_command_w)
	AM_RANGE(0xd618, 0xd618) AM_WRITENOP	/* accelerator clear */
	AM_RANGE(0xd700, 0xd7ff) AM_WRITE(paletteram_xxxxRRRRGGGGBBBB_be_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0xd840, 0xd85f) AM_WRITEONLY AM_BASE_MEMBER(buggychl_state, scrollv)
	AM_RANGE(0xdb00, 0xdbff) AM_WRITEONLY AM_BASE_MEMBER(buggychl_state, scrollh)
	AM_RANGE(0xdc04, 0xdc04) AM_WRITEONLY /* should be fg scroll */
	AM_RANGE(0xdc06, 0xdc06) AM_WRITE(buggychl_bg_scrollx_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x47ff) AM_RAM
	AM_RANGE(0x4800, 0x4801) AM_DEVWRITE("ay1", ay8910_address_data_w)
	AM_RANGE(0x4802, 0x4803) AM_DEVWRITE("ay2", ay8910_address_data_w)
	AM_RANGE(0x4810, 0x481d) AM_DEVWRITE("msm", msm5232_w)
	AM_RANGE(0x4820, 0x4820) AM_RAM	/* VOL/BAL   for the 7630 on the MSM5232 output */
	AM_RANGE(0x4830, 0x4830) AM_RAM	/* TRBL/BASS for the 7630 on the MSM5232 output  */
	AM_RANGE(0x5000, 0x5000) AM_READ(soundlatch_r)
//  AM_RANGE(0x5001, 0x5001) AM_READNOP /* is command pending? */
	AM_RANGE(0x5001, 0x5001) AM_WRITE(nmi_enable_w)
	AM_RANGE(0x5002, 0x5002) AM_WRITE(nmi_disable_w)
	AM_RANGE(0x5003, 0x5003) AM_WRITE(sound_enable_w)
	AM_RANGE(0xe000, 0xefff) AM_ROM	/* space for diagnostics ROM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( mcu_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0x7ff)
	AM_RANGE(0x0000, 0x0000) AM_READWRITE(buggychl_68705_port_a_r, buggychl_68705_port_a_w)
	AM_RANGE(0x0001, 0x0001) AM_READWRITE(buggychl_68705_port_b_r, buggychl_68705_port_b_w)
	AM_RANGE(0x0002, 0x0002) AM_READWRITE(buggychl_68705_port_c_r, buggychl_68705_port_c_w)
	AM_RANGE(0x0004, 0x0004) AM_WRITE(buggychl_68705_ddr_a_w)
	AM_RANGE(0x0005, 0x0005) AM_WRITE(buggychl_68705_ddr_b_w)
	AM_RANGE(0x0006, 0x0006) AM_WRITE(buggychl_68705_ddr_c_w)
	AM_RANGE(0x0010, 0x007f) AM_RAM
	AM_RANGE(0x0080, 0x07ff) AM_ROM
ADDRESS_MAP_END

/******************************************************************************/

static INPUT_PORTS_START( buggychl )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, "Game Over Bonus" ) PORT_DIPLOCATION("SW1:1,2")	/* Arks/Flags/Fuel */
	PORT_DIPSETTING(    0x03, "2000/1000/50" )
	PORT_DIPSETTING(    0x02, "1000/500/30" )
	PORT_DIPSETTING(    0x01, "500/200/10" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x18, DEF_STR( Easy ) )			/* 1300 units of fuel */
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )		/* 1200 units of fuel */
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )			/* 1100 units of fuel */
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )		/* 1000 units of fuel */
	PORT_SERVICE_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW1:6" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )		/* Only listed as OFF in the manual */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW2:1,2,3,4")
	PORT_DIPSETTING(    0x0f, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_8C ) )
	PORT_DIPNAME( 0xf0, 0x00, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW2:5,6,7,8")
	PORT_DIPSETTING(    0xf0, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_8C ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "Start button needed" ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )		/* Only listed as OFF in the manual */
	PORT_DIPNAME( 0x04, 0x04, "Fuel loss (Cheat)") PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Crash only" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )		/* Only listed as OFF in the manual */
	PORT_DIPNAME( 0x10, 0x10, "Coinage Display" ) PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, "Year Display" ) PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "Invulnerability (Cheat)") PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Coin Slots" ) PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x80, "2" )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 )	/* shift */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Test Button") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_BUTTON1 )	/* accelerator */

	PORT_START("WHEEL") /* wheel */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(30) PORT_KEYDELTA(15) PORT_REVERSE
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,
	256,
	4,
	{ 3*0x800*8, 2*0x800*8, 0x800*8, 0 },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout spritelayout =
{
	16,1,
	RGN_FRAC(1,8),
	4,
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{ RGN_FRAC(1,8)+7, RGN_FRAC(1,8)+6, RGN_FRAC(1,8)+5, RGN_FRAC(1,8)+4, RGN_FRAC(1,8)+3, RGN_FRAC(1,8)+2, RGN_FRAC(1,8)+1, RGN_FRAC(1,8)+0,
			7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0 },
	8
};

static GFXDECODE_START( buggychl )
	GFXDECODE_ENTRY( NULL,           0, charlayout,   0, 8 ) /* decoded at runtime */
	/* sprites are drawn pixel by pixel by draw_sprites() */
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout, 0, 8 )
GFXDECODE_END



static WRITE8_DEVICE_HANDLER( port_a_0_w )
{
	/* VOL/BAL   for the 7630 on this 8910 output */
}
static WRITE8_DEVICE_HANDLER( port_b_0_w )
{
	/* TRBL/BASS for the 7630 on this 8910 output */
}
static WRITE8_DEVICE_HANDLER( port_a_1_w )
{
	/* VOL/BAL   for the 7630 on this 8910 output */
}
static WRITE8_DEVICE_HANDLER( port_b_1_w )
{
	/* TRBL/BASS for the 7630 on this 8910 output */
}


static const ay8910_interface ay8910_interface_1 =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_HANDLER(port_a_0_w),
	DEVCB_HANDLER(port_b_0_w)
};

static const ay8910_interface ay8910_interface_2 =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_HANDLER(port_a_1_w),
	DEVCB_HANDLER(port_b_1_w)
};

static const msm5232_interface msm5232_config =
{
	{ 0.39e-6, 0.39e-6, 0.39e-6, 0.39e-6, 0.39e-6, 0.39e-6, 0.39e-6, 0.39e-6 }	/* default 0.39 uF capacitors (not verified) */
};


static MACHINE_START( buggychl )
{
	buggychl_state *state = (buggychl_state *)machine->driver_data;
	UINT8 *ROM = memory_region(machine, "maincpu");

	memory_configure_bank(machine, "bank1", 0, 6, &ROM[0x10000], 0x2000);

	state->audiocpu = machine->device("audiocpu");
	state->mcu = machine->device("mcu");

	state_save_register_global(machine, state->sound_nmi_enable);
	state_save_register_global(machine, state->pending_nmi);
	state_save_register_global_array(machine, state->sprite_lookup);
	state_save_register_global(machine, state->sl_bank);
	state_save_register_global(machine, state->bg_on);
	state_save_register_global(machine, state->sky_on);
	state_save_register_global(machine, state->sprite_color_base);
	state_save_register_global(machine, state->bg_scrollx);
	/* mcu */
	state_save_register_global(machine, state->from_main);
	state_save_register_global(machine, state->from_mcu);
	state_save_register_global(machine, state->mcu_sent);
	state_save_register_global(machine, state->main_sent);
	state_save_register_global(machine, state->port_a_in);
	state_save_register_global(machine, state->port_a_out);
	state_save_register_global(machine, state->ddr_a);
	state_save_register_global(machine, state->port_b_in);
	state_save_register_global(machine, state->port_b_out);
	state_save_register_global(machine, state->ddr_b);
	state_save_register_global(machine, state->port_c_in);
	state_save_register_global(machine, state->port_c_out);
	state_save_register_global(machine, state->ddr_c);
}

static MACHINE_RESET( buggychl )
{
	buggychl_state *state = (buggychl_state *)machine->driver_data;

	cputag_set_input_line(machine, "mcu", 0, CLEAR_LINE);

	state->sound_nmi_enable = 0;
	state->pending_nmi = 0;
	state->sl_bank = 0;
	state->bg_on = 0;
	state->sky_on = 0;
	state->sprite_color_base = 0;
	state->bg_scrollx = 0;
	/* mcu */
	state->mcu_sent = 0;
	state->main_sent = 0;
	state->from_main = 0;
	state->from_mcu = 0;
	state->port_a_in = 0;
	state->port_a_out = 0;
	state->ddr_a = 0;
	state->port_b_in = 0;
	state->port_b_out = 0;
	state->ddr_b = 0;
	state->port_c_in = 0;
	state->port_c_out = 0;
	state->ddr_c = 0;
}

static MACHINE_DRIVER_START( buggychl )

	/* driver data */
	MDRV_DRIVER_DATA(buggychl_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, 4000000) /* 4 MHz??? */
	MDRV_CPU_PROGRAM_MAP(buggychl_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_CPU_ADD("audiocpu", Z80, 4000000) /* 4 MHz??? */
	MDRV_CPU_PROGRAM_MAP(sound_map)
	MDRV_CPU_VBLANK_INT_HACK(irq0_line_hold,60)	/* irq is timed, tied to the cpu clock and not to vblank */
							/* nmi is caused by the main cpu */

	MDRV_CPU_ADD("mcu", M68705,8000000/2)  /* 4 MHz */
	MDRV_CPU_PROGRAM_MAP(mcu_map)

	MDRV_MACHINE_START(buggychl)
	MDRV_MACHINE_RESET(buggychl)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(buggychl)
	MDRV_PALETTE_LENGTH(128+128)

	MDRV_PALETTE_INIT(buggychl)
	MDRV_VIDEO_START(buggychl)
	MDRV_VIDEO_UPDATE(buggychl)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ay1", AY8910, 8000000/4)
	MDRV_SOUND_CONFIG(ay8910_interface_1)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD("ay2", AY8910, 8000000/4)
	MDRV_SOUND_CONFIG(ay8910_interface_2)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD("msm", MSM5232, 2000000)
	MDRV_SOUND_CONFIG(msm5232_config)
	MDRV_SOUND_ROUTE(0, "mono", 1.0)	// pin 28  2'-1
	MDRV_SOUND_ROUTE(1, "mono", 1.0)	// pin 29  4'-1
	MDRV_SOUND_ROUTE(2, "mono", 1.0)	// pin 30  8'-1
	MDRV_SOUND_ROUTE(3, "mono", 1.0)	// pin 31 16'-1
	MDRV_SOUND_ROUTE(4, "mono", 1.0)	// pin 36  2'-2
	MDRV_SOUND_ROUTE(5, "mono", 1.0)	// pin 35  4'-2
	MDRV_SOUND_ROUTE(6, "mono", 1.0)	// pin 34  8'-2
	MDRV_SOUND_ROUTE(7, "mono", 1.0)	// pin 33 16'-2
	// pin 1 SOLO  8'       not mapped
	// pin 2 SOLO 16'       not mapped
	// pin 22 Noise Output  not mapped
MACHINE_DRIVER_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( buggychl )
	ROM_REGION( 0x1c000, "maincpu", 0 )
	ROM_LOAD( "a22-04-2.23", 0x00000, 0x4000, CRC(16445a6a) SHA1(5ce7b0b1aeb3b6cd400965467f913558f39c251f) )
	ROM_LOAD( "a22-05-2.22", 0x04000, 0x4000, CRC(d57430b2) SHA1(3e5b8c21a342d8e26c12a78535748073bc5b8742) )
	ROM_LOAD( "a22-01.3",    0x10000, 0x4000, CRC(af3b7554) SHA1(fd4f5a6cf9253f64c7e86d566802a02baae3b379) ) /* banked */
	ROM_LOAD( "a22-02.2",    0x14000, 0x4000, CRC(b8a645fb) SHA1(614a0656dee0cfa1d7e16ec1e0138a423ecaf18b) ) /* banked */
	ROM_LOAD( "a22-03.1",    0x18000, 0x4000, CRC(5f45d469) SHA1(3a1b9ab2d57c06bfffb1271583944c90d3f6b5a2) ) /* banked */

	ROM_REGION( 0x10000, "audiocpu", 0 )  /* sound Z80 */
	ROM_LOAD( "a22-24.28",   0x00000, 0x4000, CRC(1e7f841f) SHA1(2dc0787b08d32acb78291b689c02dbb83d04d08c) )

	ROM_REGION( 0x0800, "mcu", 0 )	/* 8k for the microcontroller */
	ROM_LOAD( "a22-19.31",   0x00000, 0x0800, CRC(06a71df0) SHA1(28183e6769e1471e7f28dc2a9f5b54e14b7ef339) )

	ROM_REGION( 0x20000, "gfx1", 0 )	/* sprites */
	ROM_LOAD( "a22-06.111",  0x00000, 0x4000, CRC(1df91b17) SHA1(440d33bf984042fb4eac8f17bb385992ccdc6113) )
	ROM_LOAD( "a22-07.110",  0x04000, 0x4000, CRC(2f0ab9b7) SHA1(07b98e23d12da834d522e29fe7891503dc258b05) )
	ROM_LOAD( "a22-08.109",  0x08000, 0x4000, CRC(49cb2134) SHA1(f9998617c097b90be7257ba6fc1e46ff9e1f8916) )
	ROM_LOAD( "a22-09.108",  0x0c000, 0x4000, CRC(e682e200) SHA1(3e2b5dd97e4f522f208d331f6903c69d49555b61) )
	ROM_LOAD( "a22-10.107",  0x10000, 0x4000, CRC(653b7e25) SHA1(70c69288438caf6725c6d96ff75cdc82af005b2b) )
	ROM_LOAD( "a22-11.106",  0x14000, 0x4000, CRC(8057b55c) SHA1(9eeb4980cb14fb1c9b6f3aeff4e0aef1338fc71c) )
	ROM_LOAD( "a22-12.105",  0x18000, 0x4000, CRC(8b365b24) SHA1(a306c1f6fe1f5563602ab424f1b4f6ac17d1e47d) )
	ROM_LOAD( "a22-13.104",  0x1c000, 0x4000, CRC(2c6d68fe) SHA1(9e1a0e44ae2b9986d0ebff49a0fd4df3e8a7f4e7) )

	ROM_REGION( 0x4000, "gfx2", 0 )	/* sprite zoom tables */
	ROM_LOAD( "a22-14.59",   0x0000, 0x2000, CRC(a450b3ef) SHA1(42646bfaed19ea01ffe06996bb6c2fd6c70076d6) )	/* vertical */
	ROM_LOAD( "a22-15.115",  0x2000, 0x1000, CRC(337a0c14) SHA1(2aa6814f74497c5c55bf7098d7f6f5508845e36c) )	/* horizontal */
	ROM_LOAD( "a22-16.116",  0x3000, 0x1000, CRC(337a0c14) SHA1(2aa6814f74497c5c55bf7098d7f6f5508845e36c) )	/* horizontal */
ROM_END

ROM_START( buggychlt )
	ROM_REGION( 0x1c000, "maincpu", 0 )
	ROM_LOAD( "bu04.bin",    0x00000, 0x4000, CRC(f90ab854) SHA1(d4536c98be35de3d888548e2de15f8435ca4f08c) )
	ROM_LOAD( "bu05.bin",    0x04000, 0x4000, CRC(543d0949) SHA1(b7b0b0319f5376e7cfcfd0e8a4fa6fea566e0206) )
	ROM_LOAD( "a22-01.3",    0x10000, 0x4000, CRC(af3b7554) SHA1(fd4f5a6cf9253f64c7e86d566802a02baae3b379) ) /* banked */
	ROM_LOAD( "a22-02.2",    0x14000, 0x4000, CRC(b8a645fb) SHA1(614a0656dee0cfa1d7e16ec1e0138a423ecaf18b) ) /* banked */
	ROM_LOAD( "a22-03.1",    0x18000, 0x4000, CRC(5f45d469) SHA1(3a1b9ab2d57c06bfffb1271583944c90d3f6b5a2) ) /* banked */

	ROM_REGION( 0x10000, "audiocpu", 0 )  /* sound Z80 */
	ROM_LOAD( "a22-24.28",   0x00000, 0x4000, CRC(1e7f841f) SHA1(2dc0787b08d32acb78291b689c02dbb83d04d08c) )

	ROM_REGION( 0x0800, "mcu", 0 )	/* 8k for the microcontroller */
	ROM_LOAD( "a22-19.31",   0x00000, 0x0800, CRC(06a71df0) SHA1(28183e6769e1471e7f28dc2a9f5b54e14b7ef339) )

	ROM_REGION( 0x20000, "gfx1", 0 )	/* sprites */
	ROM_LOAD( "a22-06.111",  0x00000, 0x4000, CRC(1df91b17) SHA1(440d33bf984042fb4eac8f17bb385992ccdc6113) )
	ROM_LOAD( "a22-07.110",  0x04000, 0x4000, CRC(2f0ab9b7) SHA1(07b98e23d12da834d522e29fe7891503dc258b05) )
	ROM_LOAD( "a22-08.109",  0x08000, 0x4000, CRC(49cb2134) SHA1(f9998617c097b90be7257ba6fc1e46ff9e1f8916) )
	ROM_LOAD( "a22-09.108",  0x0c000, 0x4000, CRC(e682e200) SHA1(3e2b5dd97e4f522f208d331f6903c69d49555b61) )
	ROM_LOAD( "a22-10.107",  0x10000, 0x4000, CRC(653b7e25) SHA1(70c69288438caf6725c6d96ff75cdc82af005b2b) )
	ROM_LOAD( "a22-11.106",  0x14000, 0x4000, CRC(8057b55c) SHA1(9eeb4980cb14fb1c9b6f3aeff4e0aef1338fc71c) )
	ROM_LOAD( "a22-12.105",  0x18000, 0x4000, CRC(8b365b24) SHA1(a306c1f6fe1f5563602ab424f1b4f6ac17d1e47d) )
	ROM_LOAD( "a22-13.104",  0x1c000, 0x4000, CRC(2c6d68fe) SHA1(9e1a0e44ae2b9986d0ebff49a0fd4df3e8a7f4e7) )

	ROM_REGION( 0x4000, "gfx2", 0 )	/* sprite zoom tables */
	ROM_LOAD( "a22-14.59",   0x0000, 0x2000, CRC(a450b3ef) SHA1(42646bfaed19ea01ffe06996bb6c2fd6c70076d6) )	/* vertical */
	ROM_LOAD( "a22-15.115",  0x2000, 0x1000, CRC(337a0c14) SHA1(2aa6814f74497c5c55bf7098d7f6f5508845e36c) )	/* horizontal */
	ROM_LOAD( "a22-16.116",  0x3000, 0x1000, CRC(337a0c14) SHA1(2aa6814f74497c5c55bf7098d7f6f5508845e36c) )	/* horizontal */
ROM_END


GAMEL( 1984, buggychl, 0,        buggychl, buggychl, 0, ROT270, "Taito Corporation", "Buggy Challenge", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS, layout_buggychl )
GAMEL( 1984, buggychlt,buggychl, buggychl, buggychl, 0, ROT270, "Taito Corporation (Tecfri license)", "Buggy Challenge (Tecfri)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS, layout_buggychl )
