/***************************************************************************

Taito Dual Screen Games
=======================

Darius 2      (c) 1989 Taito
Warrior Blade (c) 1991 Taito

David Graves

(this is based on the F2 driver by Bryan McPhail, Brad Oliver, Andrew Prime,
Nicola Salmoria. Thanks to Richard Bush and the Raine team, whose open
source was very helpful in many areas particularly the sprites.)

                *****

The dual screen games operate on hardware with various similarities to
the Taito F2 system, as they share some custom ics e.g. the TC0100SCN.

For each screen the games have 3 separate layers of graphics: - one
128x64 tiled scrolling background plane of 8x8 tiles, a similar
foreground plane, and a 128x32 text plane with character definitions
held in ram. As well as this, there is a single sprite plane which
covers both screens. The sprites are 16x16 and are not zoomable.

Writing to the first TC0100SCN "writes through" to the subsidiary one
so both have identical contents. The only time the second TC0100SCN is
addressed on its own is during initial memory checks, I think. (?)

Warrior Blade has a slightly different gfx set for the 2nd screen
because the programmers ran out of scr gfx space (only 0xffff tiles
can be addressed by the TC0100SCN). In-game while tiles are
scrolling from one screen to the other it is necessary to have
identical gfx tiles for both screens. But for static screens (e.g. cut
scenes between levels) the gfx tiles needn't be the same. By
exploiting this they squeezed some extra graphics into the game.

There is a single 68000 processor which takes care of everything
except sound. That is done by a Z80 controlling a YM2610. Sound
commands are written to the Z80 by the 68000.


Tilemaps
========

TC0100SCN has tilemaps twice as wide as usual. The two BG tilemaps take
up twice the usual space, $8000 bytes each. The text tilemap takes up
the usual space, as its height is halved.

The double palette generator(one for each screen) is probably just a
result of the way the hardware works: they both have the same colors.


Dumper's Info
-------------

Darius II (Dual Screen Old & New JPN Ver.)
(c)1989 Taito
J1100204A
K1100483A

CPU     :MC68000P12F(16MHz),Z80A
Sound   :YM2610
OSC     :26.686MHz,16.000MHz
Other   :
TC0140SYT
TC0220IOC
TC0110PCR x2
TC0100SCN x2
TC0390LHC-1
TC0130LNB x8

Warrior Blade (JPN Ver.)
(c)1991 Taito
J1100295A
K1100710A (Label K11J0710A)

CPU     :MC68000P12F(16MHz),Z80A
Sound   :YM2610B
OSC     :26.686MHz,16.000MHz
Other   :
TC0140SYT
TC0510NIO
TC0110PCR x2
TC0100SCN x2
TC0390LHC-1
TC0130LNB x8

[The 390LHC/130LNB functions are unknown].


Stephh's notes (based on the game M68000 code and some tests) :

1) 'darius2d' and 'drius2do'

  - Region stored at 0x07fffe.w
  - Sets :
      * 'darius2d' : region = 0x0001
      * 'drius2do' : region = 0x0001
  - Comparison with 'darius2' :
      * coinage routine at 0x013e3a ('darius2d') or 0x013da4 ('drius2do')
      * same specific US coinage (4C_3C instead of 4C_1C for coinage)
      * what used to be an "Invulnerability" Dip Switch in 'darius' is now unused
        because the code to test it has been removed (not "noped")
      * different joystick mapping : 2 UDLR joys instead of 2 UDRL joys
      * same other notes as for 'darius2' (ninjaw.c driver)


2) 'warriorb'

  - Region stored at 0x0ffffe.w
  - Sets :
      * 'warriorb' : region = 0x0001
  - Coinage relies on the region (code at 0x002614) :
      * 0x0001 (Japan) uses TAITO_COINAGE_JAPAN_NEW
      * 0x0002 (US) uses TAITO_COINAGE_US
      * 0x0003 (World) uses TAITO_COINAGE_WORLD
  - Additional Japanese text when region = 0x0001
  - Notice screen only if region = 0x0001 or region = 0x0002
  - FBI logo only if region = 0x0002
  - US version doesn't reset score on continue, other versions do


TODO
====

Unknown sprite bits.


Darius 2
--------

The unpleasant sounds when some big enemies appear are wrong: they
are meant to create rumbling on a subwoofer in the cabinet, a sort of
vibration device. They still affect the other channels despite
filtering above 20Hz.


Warriorb
--------

Colscroll effects?

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "rendlay.h"
#include "cpu/m68000/m68000.h"
#include "video/taitoic.h"
#include "machine/taitoio.h"
#include "audio/taitosnd.h"
#include "sound/2610intf.h"
#include "sound/flt_vol.h"
#include "includes/warriorb.h"
#include "includes/taitoipt.h"

static MACHINE_START( warriorb );
static MACHINE_RESET( taito_dualscreen );



/***********************************************************
                          SOUND
***********************************************************/

static void reset_sound_region( running_machine *machine )
{
	warriorb_state *state = (warriorb_state *)machine->driver_data;
	memory_set_bank(machine, "bank10", state->banknum);
}

static WRITE8_HANDLER( sound_bankswitch_w )
{
	warriorb_state *state = (warriorb_state *)space->machine->driver_data;

	state->banknum = data & 7;
	reset_sound_region(space->machine);
}

static WRITE16_HANDLER( warriorb_sound_w )
{
	warriorb_state *state = (warriorb_state *)space->machine->driver_data;

	if (offset == 0)
		tc0140syt_port_w(state->tc0140syt, 0, data & 0xff);
	else if (offset == 1)
		tc0140syt_comm_w(state->tc0140syt, 0, data & 0xff);
}

static READ16_HANDLER( warriorb_sound_r )
{
	warriorb_state *state = (warriorb_state *)space->machine->driver_data;

	if (offset == 1)
		return ((tc0140syt_comm_r(state->tc0140syt, 0) & 0xff));
	else
		return 0;
}


static WRITE8_HANDLER( warriorb_pancontrol )
{
	warriorb_state *state = (warriorb_state *)space->machine->driver_data;
	running_device *flt = NULL;
	offset &= 3;

	switch (offset)
	{
		case 0: flt = state->_2610_1l; break;
		case 1: flt = state->_2610_1r; break;
		case 2: flt = state->_2610_2l; break;
		case 3: flt = state->_2610_2r; break;
	}

	state->pandata[offset] = (data << 1) + data;   /* original volume*3 */
	//popmessage(" pan %02x %02x %02x %02x", state->pandata[0], state->pandata[1], state->pandata[2], state->pandata[3] );
	flt_volume_set_volume(flt, state->pandata[offset] / 100.0);
}


static WRITE16_HANDLER( tc0100scn_dual_screen_w )
{
	warriorb_state *state = (warriorb_state *)space->machine->driver_data;

	tc0100scn_word_w(state->tc0100scn_1, offset, data, mem_mask);
	tc0100scn_word_w(state->tc0100scn_2, offset, data, mem_mask);
}

/***********************************************************
                      MEMORY STRUCTURES
***********************************************************/

static ADDRESS_MAP_START( darius2d_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x10ffff) AM_RAM		/* main ram */
	AM_RANGE(0x200000, 0x213fff) AM_DEVREAD("tc0100scn_1", tc0100scn_word_r) AM_WRITE(tc0100scn_dual_screen_w)	/* tilemaps (all screens) */
	AM_RANGE(0x214000, 0x2141ff) AM_WRITENOP											/* error in screen clearing code ? */
	AM_RANGE(0x220000, 0x22000f) AM_DEVREADWRITE("tc0100scn_1", tc0100scn_ctrl_word_r, tc0100scn_ctrl_word_w)
	AM_RANGE(0x240000, 0x253fff) AM_DEVREADWRITE("tc0100scn_2", tc0100scn_word_r, tc0100scn_word_w)		/* tilemaps (2nd screen) */
	AM_RANGE(0x260000, 0x26000f) AM_DEVREADWRITE("tc0100scn_2", tc0100scn_ctrl_word_r, tc0100scn_ctrl_word_w)
	AM_RANGE(0x400000, 0x400007) AM_DEVREADWRITE("tc0110pcr_1", tc0110pcr_word_r, tc0110pcr_step1_word_w)	/* palette (1st screen) */
	AM_RANGE(0x420000, 0x420007) AM_DEVREADWRITE("tc0110pcr_2", tc0110pcr_word_r, tc0110pcr_step1_word_w)	/* palette (2nd screen) */
	AM_RANGE(0x600000, 0x6013ff) AM_RAM AM_BASE_SIZE_MEMBER(warriorb_state, spriteram, spriteram_size)
	AM_RANGE(0x800000, 0x80000f) AM_DEVREADWRITE8("tc0220ioc", tc0220ioc_r, tc0220ioc_w, 0x00ff)
//  AM_RANGE(0x820000, 0x820001) AM_WRITENOP    // ???
	AM_RANGE(0x830000, 0x830003) AM_READWRITE(warriorb_sound_r, warriorb_sound_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( warriorb_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM
	AM_RANGE(0x200000, 0x213fff) AM_RAM
	AM_RANGE(0x300000, 0x313fff) AM_DEVREAD("tc0100scn_1", tc0100scn_word_r) AM_WRITE(tc0100scn_dual_screen_w)	/* tilemaps (all screens) */
	AM_RANGE(0x320000, 0x32000f) AM_DEVREADWRITE("tc0100scn_1", tc0100scn_ctrl_word_r, tc0100scn_ctrl_word_w)
	AM_RANGE(0x340000, 0x353fff) AM_DEVREADWRITE("tc0100scn_2", tc0100scn_word_r, tc0100scn_word_w)		/* tilemaps (2nd screen) */
	AM_RANGE(0x360000, 0x36000f) AM_DEVREADWRITE("tc0100scn_2", tc0100scn_ctrl_word_r, tc0100scn_ctrl_word_w)
	AM_RANGE(0x400000, 0x400007) AM_DEVREADWRITE("tc0110pcr_1", tc0110pcr_word_r, tc0110pcr_step1_word_w)	/* palette (1st screen) */
	AM_RANGE(0x420000, 0x420007) AM_DEVREADWRITE("tc0110pcr_2", tc0110pcr_word_r, tc0110pcr_step1_word_w)	/* palette (2nd screen) */
	AM_RANGE(0x600000, 0x6013ff) AM_RAM AM_BASE_SIZE_MEMBER(warriorb_state, spriteram, spriteram_size)
	AM_RANGE(0x800000, 0x80000f) AM_DEVREADWRITE("tc0510nio", tc0510nio_halfword_r, tc0510nio_halfword_w)
//  AM_RANGE(0x820000, 0x820001) AM_WRITENOP    // ? uses bits 0,2,3
	AM_RANGE(0x830000, 0x830003) AM_READWRITE(warriorb_sound_r, warriorb_sound_w)
ADDRESS_MAP_END

/***************************************************************************/

static ADDRESS_MAP_START( z80_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank10")
	AM_RANGE(0xc000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xe003) AM_DEVREADWRITE("ymsnd", ym2610_r, ym2610_w)
	AM_RANGE(0xe200, 0xe200) AM_READNOP AM_DEVWRITE("tc0140syt", tc0140syt_slave_port_w)
	AM_RANGE(0xe201, 0xe201) AM_DEVREADWRITE("tc0140syt", tc0140syt_slave_comm_r, tc0140syt_slave_comm_w)
	AM_RANGE(0xe400, 0xe403) AM_WRITE(warriorb_pancontrol) /* pan */
	AM_RANGE(0xea00, 0xea00) AM_READNOP
	AM_RANGE(0xee00, 0xee00) AM_WRITENOP /* ? */
	AM_RANGE(0xf000, 0xf000) AM_WRITENOP /* ? */
	AM_RANGE(0xf200, 0xf200) AM_WRITE(sound_bankswitch_w)
ADDRESS_MAP_END


/***********************************************************
                     INPUT PORTS, DIPs
***********************************************************/

static INPUT_PORTS_START( darius2d )
	/* 0x800000 -> 0x109e16 ($1e16,A5) and 0x109e1a ($1e1a,A5) */
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:1")    /* code at 0x0170f2 ('darius2d') or 0x01705c ('drius2do') */
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Auto Fire" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Fast" )
	TAITO_DSWA_BITS_2_TO_3_LOC(SW1)
	TAITO_COINAGE_JAPAN_OLD_LOC(SW1)

	/* 0x800002 -> 0x109e18 ($1e18,A5) and 0x109e1c ($1e1c,A5)  */
	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, "every 700k" )
	PORT_DIPSETTING(    0x08, "every 800k" )
	PORT_DIPSETTING(    0x04, "every 900k" )
	PORT_DIPSETTING(    0x00, "every 1000k" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW2:7" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	TAITO_JOY_DUAL_UDLR( 1, 2 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Freeze") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( warriorb )
	PORT_INCLUDE(darius2d)

	/* 0x800000 -> 0x202912.b (-$56ee,A5) */
	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x03, 0x03, "Vitality Recovery" ) PORT_DIPLOCATION("SW1:1,2") /* table at 0x00d508 - 4 * 4 words */
	PORT_DIPSETTING(    0x02, "Less" )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, "More" )
	PORT_DIPSETTING(    0x00, "Most" )
	TAITO_DSWA_BITS_2_TO_3_LOC(SW1)
	TAITO_COINAGE_JAPAN_NEW_LOC(SW1)

	/* 0x800002 -> 0x202913.b (-$56ed,A5) */
	PORT_MODIFY("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPNAME( 0x04, 0x04, "Gold Sheep at" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, "50k only" )
	PORT_DIPSETTING(    0x00, "50k then every 70k" )
	PORT_DIPNAME( 0x08, 0x08, "Magic Energy Loss" ) PORT_DIPLOCATION("SW2:4")   /* code at 0x0587de - when BUTTON3 pressed */
	PORT_DIPSETTING(    0x08, "Always Player" )
	PORT_DIPSETTING(    0x00, "Player or Magician" )
	PORT_DIPNAME( 0x10, 0x10, "Player Starting Strength" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Full" )
	PORT_DIPNAME( 0x20, 0x20, "Magician appears" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, "When you get a Crystal" )
	PORT_DIPSETTING(    0x00, "Always" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Rounds" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, "Normal (10-14, depends on skill)" )
	PORT_DIPSETTING(    0x00, "Long (14)" )

	PORT_MODIFY("IN0")
	/* Japanese version actually doesn't have the third button */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)	PORT_NAME("P1 Button 3 (Cheat)")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)	PORT_NAME("P2 Button 3 (Cheat)")
INPUT_PORTS_END


/***********************************************************
                        GFX DECODING
***********************************************************/

static const gfx_layout tilelayout =
{
	16,16,	/* 16*16 sprites */
	RGN_FRAC(1,1),
	4,	/* 4 bits per pixel */
	{ 0, 1, 2, 3 },
	{ 3*4, 2*4, 11*4, 10*4, 1*4, 0*4, 9*4, 8*4, 7*4, 6*4, 15*4, 14*4, 5*4, 4*4, 13*4, 12*4 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64, 8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	128*8	/* every sprite takes 128 consecutive bytes */
};

static const gfx_layout charlayout =
{
	8,8,	/* 8*8 characters */
	RGN_FRAC(1,1),
	4,	/* 4 bits per pixel */
	{ 0, 1, 2, 3 },
	{ 2*4, 3*4, 0*4, 1*4, 6*4, 7*4, 4*4, 5*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8	/* every sprite takes 32 consecutive bytes */
};

static GFXDECODE_START( warriorb )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout,  0, 256 )	/* sprites */
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,  0, 256 )	/* scr tiles (screen 1) */
	GFXDECODE_ENTRY( "gfx3", 0, charlayout,  0, 256 )	/* scr tiles (screen 2) */
GFXDECODE_END


/**************************************************************
                           YM2610 (SOUND)
**************************************************************/

/* handler called by the YM2610 emulator when the internal timers cause an IRQ */
static void irqhandler( running_device *device, int irq )
{
	warriorb_state *state = (warriorb_state *)device->machine->driver_data;
	cpu_set_input_line(state->audiocpu, 0, irq ? ASSERT_LINE : CLEAR_LINE);
}

static const ym2610_interface ym2610_config =
{
	irqhandler
};


/**************************************************************
                         SUBWOOFEr(SOUND)
**************************************************************/

#if 0
static DEVICE_START( subwoofer )
{
	/* Adjust the lowpass filter of the first three YM2610 channels */

	mixer_set_lowpass_frequency(0, 20);
	mixer_set_lowpass_frequency(1, 20);
	mixer_set_lowpass_frequency(2, 20);
}

static DEVICE_GET_INFO( subwoofer )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(subwoofer);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Subwoofer");					break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
	}
}
#endif


/***********************************************************
                       MACHINE DRIVERS
***********************************************************/

static const tc0100scn_interface darius2d_tc0100scn_intf_l =
{
	"lscreen",
	1, 3,		/* gfxnum, txnum */
	4, 0,		/* x_offset, y_offset */
	0, 0,		/* flip_xoff, flip_yoff */
	0, 0,		/* flip_text_xoff, flip_text_yoff */
	0, 0
};

static const tc0100scn_interface darius2d_tc0100scn_intf_r =
{
	"rscreen",
	2, 3,		/* gfxnum, txnum */
	4, 0,		/* x_offset, y_offset */
	0, 0,		/* flip_xoff, flip_yoff */
	0, 0,		/* flip_text_xoff, flip_text_yoff */
	0, 1
};

static const tc0100scn_interface warriorb_tc0100scn_intf_l =
{
	"lscreen",
	1, 3,		/* gfxnum, txnum */
	4, 0,		/* x_offset, y_offset */
	0, 0,		/* flip_xoff, flip_yoff */
	0, 0,		/* flip_text_xoff, flip_text_yoff */
	0, 0
};

static const tc0100scn_interface warriorb_tc0100scn_intf_r =
{
	"rscreen",
	2, 3,		/* gfxnum, txnum */
	4, 0,		/* x_offset, y_offset */
	0, 0,		/* flip_xoff, flip_yoff */
	0, 0,		/* flip_text_xoff, flip_text_yoff */
	1, 1
};

static const tc0110pcr_interface darius2d_tc0110pcr_intf_l =
{
	0
};

static const tc0110pcr_interface darius2d_tc0110pcr_intf_r =
{
	1
};


static const tc0220ioc_interface darius2d_io_intf =
{
	DEVCB_INPUT_PORT("DSWA"), DEVCB_INPUT_PORT("DSWB"),
	DEVCB_INPUT_PORT("IN0"), DEVCB_INPUT_PORT("IN1"), DEVCB_INPUT_PORT("IN2")	/* port read handlers */
};

static const tc0510nio_interface warriorb_io_intf =
{
	DEVCB_INPUT_PORT("DSWA"), DEVCB_INPUT_PORT("DSWB"),
	DEVCB_INPUT_PORT("IN0"), DEVCB_INPUT_PORT("IN1"), DEVCB_INPUT_PORT("IN2")	/* port read handlers */
};

static const tc0140syt_interface warriorb_tc0140syt_intf =
{
	"maincpu", "audiocpu"
};


static STATE_POSTLOAD( warriorb_postload )
{
	reset_sound_region(machine);
}

static MACHINE_START( warriorb )
{
	warriorb_state *state = (warriorb_state *)machine->driver_data;

	memory_configure_bank(machine, "bank10", 0, 8, memory_region(machine, "audiocpu") + 0xc000, 0x4000);

	state->maincpu = machine->device("maincpu");
	state->audiocpu = machine->device("audiocpu");
	state->tc0140syt = machine->device("tc0140syt");
	state->tc0100scn_1 = machine->device("tc0100scn_1");
	state->tc0100scn_2 = machine->device("tc0100scn_2");

	state->lscreen = machine->device("lscreen");
	state->rscreen = machine->device("rscreen");

	state->_2610_1l = machine->device("2610.1.l");
	state->_2610_1r = machine->device("2610.1.r");
	state->_2610_2l = machine->device("2610.2.l");
	state->_2610_2r = machine->device("2610.2.r");

	state_save_register_global(machine, state->banknum);
	state_save_register_global_array(machine, state->pandata);
	state_save_register_postload(machine, warriorb_postload, NULL);
}

static MACHINE_RESET( taito_dualscreen )
{
	warriorb_state *state = (warriorb_state *)machine->driver_data;

	state->banknum = 0;

	/**** mixer control enable ****/
	sound_global_enable(machine, 1);	/* mixer enabled */
}

static MACHINE_DRIVER_START( darius2d )

	/* driver data */
	MDRV_DRIVER_DATA(warriorb_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 12000000)	/* 12 MHz ??? (Might well be 16!) */
	MDRV_CPU_PROGRAM_MAP(darius2d_map)
	MDRV_CPU_VBLANK_INT("lscreen", irq4_line_hold)

	MDRV_CPU_ADD("audiocpu", Z80,16000000/4)	/* 4 MHz ? */
	MDRV_CPU_PROGRAM_MAP(z80_sound_map)

	MDRV_MACHINE_START( warriorb )
	MDRV_MACHINE_RESET( taito_dualscreen )

	MDRV_TC0220IOC_ADD("tc0220ioc", darius2d_io_intf)

	/* video hardware */
	MDRV_GFXDECODE(warriorb)
	MDRV_PALETTE_LENGTH(4096*2)
	MDRV_DEFAULT_LAYOUT(layout_dualhsxs)

	MDRV_SCREEN_ADD("lscreen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_SIZE(40*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 3*8, 32*8-1)

	MDRV_SCREEN_ADD("rscreen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_SIZE(40*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 3*8, 32*8-1)

	MDRV_VIDEO_START(warriorb)
	MDRV_VIDEO_UPDATE(warriorb)

	MDRV_TC0100SCN_ADD("tc0100scn_1", darius2d_tc0100scn_intf_l)
	MDRV_TC0100SCN_ADD("tc0100scn_2", darius2d_tc0100scn_intf_r)
	MDRV_TC0110PCR_ADD("tc0110pcr_1", darius2d_tc0110pcr_intf_l)
	MDRV_TC0110PCR_ADD("tc0110pcr_2", darius2d_tc0110pcr_intf_r)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2610, 16000000/2)
	MDRV_SOUND_CONFIG(ym2610_config)
	MDRV_SOUND_ROUTE(0, "lspeaker",  0.25)
	MDRV_SOUND_ROUTE(0, "rspeaker", 0.25)
	MDRV_SOUND_ROUTE(1, "2610.1.l", 1.0)
	MDRV_SOUND_ROUTE(1, "2610.1.r", 1.0)
	MDRV_SOUND_ROUTE(2, "2610.2.l", 1.0)
	MDRV_SOUND_ROUTE(2, "2610.2.r", 1.0)

	MDRV_SOUND_ADD("2610.1.l", FILTER_VOLUME, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MDRV_SOUND_ADD("2610.1.r", FILTER_VOLUME, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
	MDRV_SOUND_ADD("2610.2.l", FILTER_VOLUME, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MDRV_SOUND_ADD("2610.2.r", FILTER_VOLUME, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)

	MDRV_TC0140SYT_ADD("tc0140syt", warriorb_tc0140syt_intf)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( warriorb )

	/* driver data */
	MDRV_DRIVER_DATA(warriorb_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 16000000)	/* 16 MHz ? */
	MDRV_CPU_PROGRAM_MAP(warriorb_map)
	MDRV_CPU_VBLANK_INT("lscreen", irq4_line_hold)

	MDRV_CPU_ADD("audiocpu", Z80,16000000/4)	/* 4 MHz ? */
	MDRV_CPU_PROGRAM_MAP(z80_sound_map)

	MDRV_MACHINE_START( warriorb )
	MDRV_MACHINE_RESET( taito_dualscreen )

	MDRV_TC0510NIO_ADD("tc0510nio", warriorb_io_intf)

	/* video hardware */
	MDRV_GFXDECODE(warriorb)
	MDRV_PALETTE_LENGTH(4096*2)
	MDRV_DEFAULT_LAYOUT(layout_dualhsxs)

	MDRV_SCREEN_ADD("lscreen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_SIZE(40*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 2*8, 32*8-1)

	MDRV_SCREEN_ADD("rscreen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_SIZE(40*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 2*8, 32*8-1)

	MDRV_VIDEO_START(warriorb)
	MDRV_VIDEO_UPDATE(warriorb)

	MDRV_TC0100SCN_ADD("tc0100scn_1", warriorb_tc0100scn_intf_l)
	MDRV_TC0100SCN_ADD("tc0100scn_2", warriorb_tc0100scn_intf_r)
	MDRV_TC0110PCR_ADD("tc0110pcr_1", darius2d_tc0110pcr_intf_l)
	MDRV_TC0110PCR_ADD("tc0110pcr_2", darius2d_tc0110pcr_intf_r)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2610, 16000000/2)
	MDRV_SOUND_CONFIG(ym2610_config)
	MDRV_SOUND_ROUTE(0, "lspeaker",  0.25)
	MDRV_SOUND_ROUTE(0, "rspeaker", 0.25)
	MDRV_SOUND_ROUTE(1, "2610.1.l", 1.0)
	MDRV_SOUND_ROUTE(1, "2610.1.r", 1.0)
	MDRV_SOUND_ROUTE(2, "2610.2.l", 1.0)
	MDRV_SOUND_ROUTE(2, "2610.2.r", 1.0)

	MDRV_SOUND_ADD("2610.1.l", FILTER_VOLUME, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MDRV_SOUND_ADD("2610.1.r", FILTER_VOLUME, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
	MDRV_SOUND_ADD("2610.2.l", FILTER_VOLUME, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MDRV_SOUND_ADD("2610.2.r", FILTER_VOLUME, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)

	MDRV_TC0140SYT_ADD("tc0140syt", warriorb_tc0140syt_intf)
MACHINE_DRIVER_END


/***************************************************************************
                                 DRIVERS
***************************************************************************/

ROM_START( darius2d )
	ROM_REGION( 0x100000, "maincpu", 0 )	/* 512K for 68000 code */
	ROM_LOAD16_BYTE( "c07_20-2.74", 0x00000, 0x20000, CRC(a0f345b8) SHA1(1ce46e9707ec9ad51b26acf613eedc0536d227ae) )
	ROM_LOAD16_BYTE( "c07_19-2.73", 0x00001, 0x20000, CRC(925412c6) SHA1(7f1f62b7b2261c440dccd512ebd3faea141b7c83) )
	ROM_LOAD16_BYTE( "c07_21-2.76", 0x40000, 0x20000, CRC(bdd60e37) SHA1(777d3f67deba7df0da9d2605b2e2198f4bf47ebc) )
	ROM_LOAD16_BYTE( "c07_18-2.71", 0x40001, 0x20000, CRC(23fcd89b) SHA1(8aaf4ac836773d9b064ded68a6f092fe9eec7ac2) )

	ROM_LOAD16_WORD_SWAP( "c07-09.75",   0x80000, 0x80000, CRC(cc69c2ce) SHA1(47883b9e14d8b6dd74db221bff396477231938f2) )	/* data rom */

	ROM_REGION( 0x2c000, "audiocpu", 0 )	/* sound cpu */
	ROM_LOAD( "c07-17.69", 0x00000, 0x04000, CRC(ae16c905) SHA1(70ba5aacd8a8e00b94719e3955abad8827c67aa8) )
	ROM_CONTINUE(          0x10000, 0x1c000 ) /* banked stuff */

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "c07-03.12", 0x00000, 0x80000, CRC(189bafce) SHA1(d885e444523489fe24269b90dec58e0d92cfbd6e) )	/* SCr(screen 1) */
	ROM_LOAD( "c07-04.11", 0x80000, 0x80000, CRC(50421e81) SHA1(27ac420602f1dac00dc32903543a518e6f47fb2f) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD32_BYTE( "c07-06.27", 0x00000, 0x80000, CRC(5eebbcd6) SHA1(d4d860bf6b099956c45c7273ad77b1d35deba4c1) )	/* OBJ */
	ROM_LOAD32_BYTE( "c07-05.24", 0x00001, 0x80000, CRC(fb6d0550) SHA1(2d570ff5ef262cb4cb52e8584a7f167263194d37) )
	ROM_LOAD32_BYTE( "c07-08.25", 0x00002, 0x80000, CRC(a07dc846) SHA1(7199a604fcd693215ddb7670bfb2daf150145fd7) )
	ROM_LOAD32_BYTE( "c07-07.26", 0x00003, 0x80000, CRC(fd9f9e74) SHA1(e89beb5cac844fe16662465b0c76337692591aae) )

	ROM_REGION( 0x100000, "gfx3", 0 )
	ROM_COPY( "gfx1", 0x000000, 0x000000, 0x100000 )	/* SCr(screen 2) */

/* The actual board duplicates the SCR gfx roms for the 2nd TC0100SCN */
//  ROM_LOAD( "c07-03.47", 0x00000, 0x80000, CRC(189bafce) SHA1(d885e444523489fe24269b90dec58e0d92cfbd6e) )
//  ROM_LOAD( "c07-04.48", 0x80000, 0x80000, CRC(50421e81) SHA1(27ac420602f1dac00dc32903543a518e6f47fb2f) )

	ROM_REGION( 0x100000, "ymsnd", 0 )	/* ADPCM samples */
	ROM_LOAD( "c07-10.95", 0x00000, 0x80000, CRC(4bbe0ed9) SHA1(081b73c4e4d4fa548445e5548573099bcb1e9213) )
	ROM_LOAD( "c07-11.96", 0x80000, 0x80000, CRC(3c815699) SHA1(0471ff5b0c0da905267f2cee52fd68c8661cccc9) )

	ROM_REGION( 0x080000, "ymsnd.deltat", 0 )	/* Delta-T samples */
	ROM_LOAD( "c07-12.107", 0x00000, 0x80000, CRC(e0b71258) SHA1(0258e308b643d723475824752ebffc4ea29d1ac4) )

	ROM_REGION( 0x001000, "user1", 0 )	/* unknown roms */
	ROM_LOAD( "c07-13.37", 0x00000, 0x00400, CRC(3ca18eb3) SHA1(54560f02c2be67993940831222130e90cd171991) )
	ROM_LOAD( "c07-14.38", 0x00000, 0x00400, CRC(baf2a193) SHA1(b7f103b5f5aab0702dd21fd7e3a82261ae1760e9) )

// Pals, not dumped
//  ROM_LOAD( "C07-15.78", 0x00000, 0x00?00, NO_DUMP )
//  ROM_LOAD( "C07-16.79", 0x00000, 0x00?00, NO_DUMP )
ROM_END

ROM_START( darius2do )
	ROM_REGION( 0x100000, "maincpu", 0 )	/* 512K for 68000 code */
	ROM_LOAD16_BYTE( "c07_20-1.74", 0x00000, 0x20000, CRC(48b0804a) SHA1(932fb2cd55e6bfef84cf3cfaf3e75b4297a92b34) )
	ROM_LOAD16_BYTE( "c07_19-1.73", 0x00001, 0x20000, CRC(1f9a4f83) SHA1(d02caef350bdcac0ff771b5c92bb4e7435e0c9fa) )
	ROM_LOAD16_BYTE( "c07_21-1.76", 0x40000, 0x20000, CRC(b491b0ca) SHA1(dd7aa196c6002abc8e2f885f3f997f2279e59769) )
	ROM_LOAD16_BYTE( "c07_18-1.71", 0x40001, 0x20000, CRC(c552e42f) SHA1(dc952002a9a738cb1789f7c51acb71693ae03549) )

	ROM_LOAD16_WORD_SWAP( "c07-09.75",   0x80000, 0x80000, CRC(cc69c2ce) SHA1(47883b9e14d8b6dd74db221bff396477231938f2) )	/* data rom */

	ROM_REGION( 0x2c000, "audiocpu", 0 )	/* sound cpu */
	ROM_LOAD( "c07-17.69", 0x00000, 0x04000, CRC(ae16c905) SHA1(70ba5aacd8a8e00b94719e3955abad8827c67aa8) )
	ROM_CONTINUE(          0x10000, 0x1c000 ) /* banked stuff */

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "c07-03.12", 0x00000, 0x80000, CRC(189bafce) SHA1(d885e444523489fe24269b90dec58e0d92cfbd6e) )	/* SCr(screen 1) */
	ROM_LOAD( "c07-04.11", 0x80000, 0x80000, CRC(50421e81) SHA1(27ac420602f1dac00dc32903543a518e6f47fb2f) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD32_BYTE( "c07-06.27", 0x00000, 0x80000, CRC(5eebbcd6) SHA1(d4d860bf6b099956c45c7273ad77b1d35deba4c1) )	/* OBJ */
	ROM_LOAD32_BYTE( "c07-05.24", 0x00001, 0x80000, CRC(fb6d0550) SHA1(2d570ff5ef262cb4cb52e8584a7f167263194d37) )
	ROM_LOAD32_BYTE( "c07-08.25", 0x00002, 0x80000, CRC(a07dc846) SHA1(7199a604fcd693215ddb7670bfb2daf150145fd7) )
	ROM_LOAD32_BYTE( "c07-07.26", 0x00003, 0x80000, CRC(fd9f9e74) SHA1(e89beb5cac844fe16662465b0c76337692591aae) )

	ROM_REGION( 0x100000, "gfx3", 0 )
	ROM_COPY( "gfx1", 0x000000, 0x000000, 0x100000 )	/* SCr(screen 2) */

/* The actual board duplicates the SCR gfx roms for the 2nd TC0100SCN */
//  ROM_LOAD( "c07-03.47", 0x00000, 0x80000, CRC(189bafce) SHA1(d885e444523489fe24269b90dec58e0d92cfbd6e) )
//  ROM_LOAD( "c07-04.48", 0x80000, 0x80000, CRC(50421e81) SHA1(27ac420602f1dac00dc32903543a518e6f47fb2f) )

	ROM_REGION( 0x100000, "ymsnd", 0 )	/* ADPCM samples */
	ROM_LOAD( "c07-10.95", 0x00000, 0x80000, CRC(4bbe0ed9) SHA1(081b73c4e4d4fa548445e5548573099bcb1e9213) )
	ROM_LOAD( "c07-11.96", 0x80000, 0x80000, CRC(3c815699) SHA1(0471ff5b0c0da905267f2cee52fd68c8661cccc9) )

	ROM_REGION( 0x080000, "ymsnd.deltat", 0 )	/* Delta-T samples */
	ROM_LOAD( "c07-12.107", 0x00000, 0x80000, CRC(e0b71258) SHA1(0258e308b643d723475824752ebffc4ea29d1ac4) )

	ROM_REGION( 0x001000, "user1", 0 )	/* unknown roms */
	ROM_LOAD( "c07-13.37", 0x00000, 0x00400, CRC(3ca18eb3) SHA1(54560f02c2be67993940831222130e90cd171991) )
	ROM_LOAD( "c07-14.38", 0x00000, 0x00400, CRC(baf2a193) SHA1(b7f103b5f5aab0702dd21fd7e3a82261ae1760e9) )
ROM_END

ROM_START( warriorb )
	ROM_REGION( 0x200000, "maincpu", 0 )	/* 1024K for 68000 code */
	ROM_LOAD16_BYTE( "d24_20-1.74", 0x000000, 0x40000, CRC(4452dc25) SHA1(bbb4fbc25a3f263ce2716698cacaca201cb9591b) )
	ROM_LOAD16_BYTE( "d24_19-1.73", 0x000001, 0x40000, CRC(15c16016) SHA1(5b28834d8d5296c562c90a861c6ccdd46cc3c204) )
	ROM_LOAD16_BYTE( "d24_21-1.76", 0x080000, 0x40000, CRC(783ef8e1) SHA1(28a43d5231031b2ff3e437c3b6b8604f0d2b521b) )
	ROM_LOAD16_BYTE( "d24_18-1.71", 0x080001, 0x40000, CRC(4502db60) SHA1(b29c441ab79f753378ea47e7c22924db0cd5eb89) )

	ROM_LOAD16_WORD_SWAP( "d24-09.75",   0x100000, 0x100000, CRC(ece5cc59) SHA1(337db41d5a74fa4202b1be1a672a068ec3b205a8) )	/* data rom */
	/* Note: Raine wrongly doubles up d24-09 as delta-t samples */

	ROM_REGION( 0x2c000, "audiocpu", 0 )	/* sound cpu */
	ROM_LOAD( "d24_17.69",  0x00000, 0x04000, CRC(e41e4aae) SHA1(9bf40b6e8aa5c6ec62c5d21edbb2214f6550c94f) )
	ROM_CONTINUE(           0x10000, 0x1c000 ) /* banked stuff */

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "d24-02.12", 0x000000, 0x100000, CRC(9f50c271) SHA1(1a1b2ae7cb7785e7f66aa26258a6cd2921a29545) )	/* SCR A, screen 1 */
	ROM_LOAD( "d24-01.11", 0x100000, 0x100000, CRC(326dcca9) SHA1(1993776d71bca7d6dfc6f84dd9262d0dcae87f69) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD32_BYTE( "d24-06.27", 0x000000, 0x100000, CRC(918486fe) SHA1(cc9e287221ef33dba77a22975e23b250ba50b758) )	/* OBJ */
	ROM_LOAD32_BYTE( "d24-03.24", 0x000001, 0x100000, CRC(46db9fd7) SHA1(f08f3c9833d80ce161b06f4ae484c5c79539639c) )
	ROM_LOAD32_BYTE( "d24-04.25", 0x000002, 0x100000, CRC(148e0493) SHA1(f1cb819830e5bd544b11762784e228b5cb62b7e4) )
	ROM_LOAD32_BYTE( "d24-05.26", 0x000003, 0x100000, CRC(9f414317) SHA1(204cf47404e5e1085c1108abacd2b79a6cd0f74a) )

	ROM_REGION( 0x200000, "gfx3", 0 )
	ROM_LOAD( "d24-07.47", 0x000000, 0x100000, CRC(9f50c271) SHA1(1a1b2ae7cb7785e7f66aa26258a6cd2921a29545) )	/* SCR B, screen 2 */
	ROM_LOAD( "d24-08.48", 0x100000, 0x100000, CRC(1e6d1528) SHA1(d6843aa67befd7db44f468be16ba2f0efb85d40f) )

	ROM_REGION( 0x300000, "ymsnd", 0 )	/* ADPCM samples */
	ROM_LOAD( "d24-12.107", 0x000000, 0x100000, CRC(279203a1) SHA1(ed75e811a1f0863c134034457ce2e97372726bdb) )
	ROM_LOAD( "d24-10.95",  0x100000, 0x100000, CRC(0e0c716d) SHA1(5e2f334dd484678766c5a71196d9bad0ba0fe8d9) )
	ROM_LOAD( "d24-11.118", 0x200000, 0x100000, CRC(15362573) SHA1(8602c9f24134cac6fe1375fb189b152f0c68aeb7) )

	/* No Delta-T samples */

	ROM_REGION( 0x01000, "user1", 0 )	/* unknown roms */
	ROM_LOAD( "d24-13.37", 0x00000, 0x400, CRC(3ca18eb3) SHA1(54560f02c2be67993940831222130e90cd171991) )
	ROM_LOAD( "d24-14.38", 0x00000, 0x400, CRC(baf2a193) SHA1(b7f103b5f5aab0702dd21fd7e3a82261ae1760e9) )
//  ROM_LOAD( "d24-15.78", 0x00000, 0xa??, NO_DUMP )    /* Pals */
//  ROM_LOAD( "d24-16.79", 0x00000, 0xa??, NO_DUMP )
ROM_END


/* Working Games */

GAME( 1989, darius2d, darius2,  darius2d, darius2d, 0, ROT0, "Taito Corporation", "Darius II (dual screen) (Japan)", 0 )
GAME( 1989, darius2do,darius2,  darius2d, darius2d, 0, ROT0, "Taito Corporation", "Darius II (dual screen) (Japan old version)", 0 )
GAME( 1991, warriorb, 0,        warriorb, warriorb, 0, ROT0, "Taito Corporation", "Warrior Blade - Rastan Saga Episode III (Japan)", 0 )
