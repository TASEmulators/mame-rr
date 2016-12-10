/***************************************************************************

"Combat School" (also known as "Boot Camp") - (Konami GX611)

TODO:
- in combatsc (and more generally the 007121) the number of sprites can be
  increased from 0x40 to 0x80. There is a hack in konamiic.c to handle that,
  but it is wrong. If you don't pass the Iron Man stage, a few sprites are
  left dangling on the screen.(*not a bug, 64 sprites are the maximum)
- it seems that to get correct target colors in firing range III we have to
  use the WRONG lookup table (the one for tiles instead of the one for
  sprites).
- in combatscb, wrong sprite/char priority (see cpu head at beginning of arm
  wrestling, and heads in intermission after firing range III)
- hook up sound in bootleg (the current sound is a hack, making use of the
  Konami ROMset)
- understand how the trackball really works
- YM2203 pitch is wrong. Fixing it screws up the tempo.

  Update: 3MHz(24MHz/8) is the more appropriate clock speed for the 2203.
  It gives the correct pitch(ear subjective) compared to the official
  soundtrack albeit the music plays slow by about 10%.

  Execution timing of the Z80 is important because it maintains music tempo
  by polling the 2203's second timer. Even when working alone with no
  context-switch the chip shouldn't be running at 1.5MHz otherwise it won't
  keep the right pace. Similar Konmai games from the same period(mainevt,
  battlnts, flkatck...etc.) all have a 3.579545MHz Z80 for sound.

  In spite of adjusting clock speed polling is deemed inaccurate when
  interleaving is taken into account. A high resolution timer around the
  poll loop is probably the best bet. The driver sets its timer manually
  because strange enough, interleaving doesn't occur immediately when
  cpuexec_boost_interleave() is called. Speculations are TIME_NOWs could have
  been used as the timer durations to force instant triggering.


Credits:

    Hardware Info:
        Jose Tejada Gomez
        Manuel Abadia
        Cesareo Gutierrez

    MAME Driver:
        Phil Stroffolino
        Manuel Abadia

Memory Maps (preliminary):

***************************
* Combat School (bootleg) *
***************************

MAIN CPU:
---------
00c0-00c3   Objects control
0500        bankswitch control
0600-06ff   palette
0800-1fff   RAM
2000-2fff   Video RAM (banked)
3000-3fff   Object RAM (banked)
4000-7fff   Banked Area + IO + Video Registers
8000-ffff   ROM

SOUND CPU:
----------
0000-8000   ROM
8000-87ef   RAM
87f0-87ff   ???
9000-9001   YM2203
9008        ???
9800        OKIM5205?
a000        soundlatch?
a800        OKIM5205?
fffc-ffff   ???


        Notes about the sound systsem of the bootleg:
        ---------------------------------------------
        The positions 0x87f0-0x87ff are very important, it
        does work similar to a semaphore (same as a lot of
        vblank bits). For example in the init code, it writes
        zero to 0x87fa, then it waits to it 'll be different
        to zero, but it isn't written by this cpu. (shareram?)
        I have tried put here a K007232 chip, but it didn't
        work.

        Sound chips: OKI M5205 & YM2203

        We are using the other sound hardware for now.

****************************
* Combat School (Original) *
****************************

0000-005f   Video Registers (banked)
0400-0407   input ports
0408        coin counters
0410        bankswitch control
0600-06ff   palette
0800-1fff   RAM
2000-2fff   Video RAM (banked)
3000-3fff   Object RAM (banked)
4000-7fff   Banked Area + IO + Video Registers
8000-ffff   ROM

SOUND CPU:
----------
0000-8000   ROM
8000-87ff   RAM
9000        uPD7759
b000        uPD7759
c000        uPD7759
d000        soundlatch_r
e000-e001   YM2203


2008-08:
Dip location and recommended settings verified with the US manual

***************************************************************************/

#include "emu.h"
#include "cpu/hd6309/hd6309.h"
#include "cpu/z80/z80.h"
#include "sound/2203intf.h"
#include "sound/upd7759.h"
#include "video/konicdev.h"
#include "includes/combatsc.h"


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

static WRITE8_HANDLER( combatsc_vreg_w )
{
	combatsc_state *state = (combatsc_state *)space->machine->driver_data;
	if (data != state->vreg)
	{
		tilemap_mark_all_tiles_dirty(state->textlayer);
		if ((data & 0x0f) != (state->vreg & 0x0f))
			tilemap_mark_all_tiles_dirty(state->bg_tilemap[0]);
		if ((data >> 4) != (state->vreg >> 4))
			tilemap_mark_all_tiles_dirty(state->bg_tilemap[1]);
		state->vreg = data;
	}
}

static WRITE8_HANDLER( combatscb_sh_irqtrigger_w )
{
	combatsc_state *state = (combatsc_state *)space->machine->driver_data;
	soundlatch_w(space, offset, data);
	cpu_set_input_line_and_vector(state->audiocpu, 0, HOLD_LINE, 0xff);
}

static READ8_HANDLER( combatscb_io_r )
{
	static const char *const portnames[] = { "IN0", "IN1", "DSW1", "DSW2" };

	return input_port_read(space->machine, portnames[offset]);
}

static WRITE8_HANDLER( combatscb_priority_w )
{
	combatsc_state *state = (combatsc_state *)space->machine->driver_data;
	state->priority = data & 0x20;
}

static WRITE8_HANDLER( combatsc_bankselect_w )
{
	combatsc_state *state = (combatsc_state *)space->machine->driver_data;

	if (data & 0x40)
	{
		state->video_circuit = 1;
		state->videoram = state->page[1];
		state->scrollram = state->scrollram1;
	}
	else
	{
		state->video_circuit = 0;
		state->videoram = state->page[0];
		state->scrollram = state->scrollram0;
	}

	state->priority = data & 0x20;

	if (data & 0x10)
		memory_set_bank(space->machine, "bank1", (data & 0x0e) >> 1);
	else
		memory_set_bank(space->machine, "bank1", 8 + (data & 1));
}

static WRITE8_HANDLER( combatscb_io_w )
{
	combatsc_state *state = (combatsc_state *)space->machine->driver_data;

	switch (offset)
	{
		case 0x400: combatscb_priority_w(space, 0, data); break;
		case 0x800: combatscb_sh_irqtrigger_w(space, 0, data); break;
		case 0xc00:	combatsc_vreg_w(space, 0, data); break;
		default: state->io_ram[offset] = data; break;
	}
}

static WRITE8_HANDLER( combatscb_bankselect_w )
{
	combatsc_state *state = (combatsc_state *)space->machine->driver_data;

	if (data & 0x40)
	{
		state->video_circuit = 1;
		state->videoram = state->page[1];
	}
	else
	{
		state->video_circuit = 0;
		state->videoram = state->page[0];
	}

	data = data & 0x1f;

	if (data != state->bank_select)
	{
		state->bank_select = data;

		if (data & 0x10)
			memory_set_bank(space->machine, "bank1", (data & 0x0e) >> 1);
		else
			memory_set_bank(space->machine, "bank1", 8 + (data & 1));

		if (data == 0x1f)
		{
			memory_set_bank(space->machine, "bank1", 8 + (data & 1));
			memory_install_write8_handler(space, 0x4000, 0x7fff, 0, 0, combatscb_io_w);
			memory_install_read8_handler(space, 0x4400, 0x4403, 0, 0, combatscb_io_r);/* IO RAM & Video Registers */
		}
		else
		{
			memory_install_read_bank(space, 0x4000, 0x7fff, 0, 0, "bank1");	/* banked ROM */
			memory_unmap_write(space, 0x4000, 0x7fff, 0, 0);	/* banked ROM */
		}
	}
}

/****************************************************************************/

static WRITE8_HANDLER( combatsc_coin_counter_w )
{
	/* b7-b3: unused? */
	/* b1: coin counter 2 */
	/* b0: coin counter 1 */

	coin_counter_w(space->machine, 0, data & 0x01);
	coin_counter_w(space->machine, 1, data & 0x02);
}

static READ8_HANDLER( trackball_r )
{
	combatsc_state *state = (combatsc_state *)space->machine->driver_data;

	if (offset == 0)
	{
		int i, dir[4];
		static const char *const tracknames[] = { "TRACK0_Y", "TRACK0_X", "TRACK1_Y", "TRACK1_X" };

		for (i = 0; i < 4; i++)
		{
			UINT8 curr;

			curr = input_port_read_safe(space->machine, tracknames[i], 0xff);

			dir[i] = curr - state->pos[i];
			state->sign[i] = dir[i] & 0x80;
			state->pos[i] = curr;
		}

		/* fix sign for orthogonal movements */
		if (dir[0] || dir[1])
		{
			if (!dir[0]) state->sign[0] = state->sign[1] ^ 0x80;
			if (!dir[1]) state->sign[1] = state->sign[0];
		}
		if (dir[2] || dir[3])
		{
			if (!dir[2]) state->sign[2] = state->sign[3] ^ 0x80;
			if (!dir[3]) state->sign[3] = state->sign[2];
		}
	}

	return state->sign[offset] | (state->pos[offset] & 0x7f);
}


/* the protection is a simple multiply */
static WRITE8_HANDLER( protection_w )
{
	combatsc_state *state = (combatsc_state *)space->machine->driver_data;
	state->prot[offset] = data;
}
static READ8_HANDLER( protection_r )
{
	combatsc_state *state = (combatsc_state *)space->machine->driver_data;
	return ((state->prot[0] * state->prot[1]) >> (offset * 8)) & 0xff;
}
static WRITE8_HANDLER( protection_clock_w )
{
	/* 0x3f is written here every time before accessing the other registers */
}


/****************************************************************************/

static WRITE8_HANDLER( combatsc_sh_irqtrigger_w )
{
	combatsc_state *state = (combatsc_state *)space->machine->driver_data;
	cpu_set_input_line_and_vector(state->audiocpu, 0, HOLD_LINE, 0xff);
}

static READ8_DEVICE_HANDLER( combatsc_busy_r )
{
	return upd7759_busy_r(device) ? 1 : 0;
}

static WRITE8_DEVICE_HANDLER( combatsc_play_w )
{
	upd7759_start_w(device, data & 2);
}

static WRITE8_DEVICE_HANDLER( combatsc_voice_reset_w )
{
	upd7759_reset_w(device,data & 1);
}

static WRITE8_DEVICE_HANDLER( combatsc_portA_w )
{
	/* unknown. always write 0 */
}

static READ8_DEVICE_HANDLER ( combatsc_ym2203_r )
{
	combatsc_state *state = (combatsc_state *)device->machine->driver_data;
	int status = ym2203_r(device,offset);

	if (cpu_get_pc(state->audiocpu) == 0x334)
	{
		if (state->boost)
		{
			state->boost = 0;
			timer_adjust_periodic(state->interleave_timer, attotime_zero, 0, state->audiocpu->cycles_to_attotime(80));
		}
		else if (status & 2)
		{
			state->boost = 1;
			timer_adjust_oneshot(state->interleave_timer, attotime_zero, 0);
		}
	}

	return status;
}

/*************************************
 *
 *  Address maps
 *
 *************************************/

static ADDRESS_MAP_START( combatsc_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0007) AM_WRITE(combatsc_pf_control_w)
	AM_RANGE(0x0020, 0x005f) AM_READWRITE(combatsc_scrollram_r, combatsc_scrollram_w)
//  AM_RANGE(0x0060, 0x00ff) AM_WRITEONLY                 /* RAM */

	AM_RANGE(0x0200, 0x0201) AM_READWRITE(protection_r, protection_w)
	AM_RANGE(0x0206, 0x0206) AM_WRITE(protection_clock_w)

	AM_RANGE(0x0400, 0x0400) AM_READ_PORT("IN0")
	AM_RANGE(0x0401, 0x0401) AM_READ_PORT("DSW3")			/* DSW #3 */
	AM_RANGE(0x0402, 0x0402) AM_READ_PORT("DSW1")			/* DSW #1 */
	AM_RANGE(0x0403, 0x0403) AM_READ_PORT("DSW2")			/* DSW #2 */
	AM_RANGE(0x0404, 0x0407) AM_READ(trackball_r)			/* 1P & 2P controls / trackball */
	AM_RANGE(0x0408, 0x0408) AM_WRITE(combatsc_coin_counter_w)	/* coin counters */
	AM_RANGE(0x040c, 0x040c) AM_WRITE(combatsc_vreg_w)
	AM_RANGE(0x0410, 0x0410) AM_WRITE(combatsc_bankselect_w)
	AM_RANGE(0x0414, 0x0414) AM_WRITE(soundlatch_w)
	AM_RANGE(0x0418, 0x0418) AM_WRITE(combatsc_sh_irqtrigger_w)
	AM_RANGE(0x041c, 0x041c) AM_WRITE(watchdog_reset_w)			/* watchdog reset? */

	AM_RANGE(0x0600, 0x06ff) AM_RAM AM_BASE_MEMBER(combatsc_state, paletteram)		/* palette */
	AM_RANGE(0x0800, 0x1fff) AM_RAM								/* RAM */
	AM_RANGE(0x2000, 0x3fff) AM_READWRITE(combatsc_video_r, combatsc_video_w)
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank1")						/* banked ROM area */
	AM_RANGE(0x8000, 0xffff) AM_ROM								/* ROM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( combatscb_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x04ff) AM_RAM
	AM_RANGE(0x0500, 0x0500) AM_WRITE(combatscb_bankselect_w)
	AM_RANGE(0x0600, 0x06ff) AM_RAM AM_BASE_MEMBER(combatsc_state, paletteram)		/* palette */
	AM_RANGE(0x0800, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0x3fff) AM_READWRITE(combatsc_video_r, combatsc_video_w)
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank1")						/* banked ROM/RAM area */
	AM_RANGE(0x8000, 0xffff) AM_ROM								/* ROM */
ADDRESS_MAP_END

#if 0
static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM										/* ROM */
	AM_RANGE(0x8000, 0x87ef) AM_RAM										/* RAM */
	AM_RANGE(0x87f0, 0x87ff) AM_RAM										/* ??? */
	AM_RANGE(0x9000, 0x9001) AM_DEVREADWRITE("ymsnd", ym2203_r, ym2203_w)	/* YM 2203 */
	AM_RANGE(0x9008, 0x9009) AM_DEVREAD("ymsnd", ym2203_r)					/* ??? */
	AM_RANGE(0xa000, 0xa000) AM_READ(soundlatch_r)						/* soundlatch_r? */
	AM_RANGE(0x8800, 0xfffb) AM_ROM										/* ROM? */
	AM_RANGE(0xfffc, 0xffff) AM_RAM										/* ??? */
ADDRESS_MAP_END
#endif

static ADDRESS_MAP_START( combatsc_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM												/* ROM */
	AM_RANGE(0x8000, 0x87ff) AM_RAM												/* RAM */

	AM_RANGE(0x9000, 0x9000) AM_DEVWRITE("upd", combatsc_play_w)					/* upd7759 play voice */
	AM_RANGE(0xa000, 0xa000) AM_DEVWRITE("upd", upd7759_port_w)					/* upd7759 voice select */
	AM_RANGE(0xb000, 0xb000) AM_DEVREAD("upd", combatsc_busy_r)					/* upd7759 busy? */
	AM_RANGE(0xc000, 0xc000) AM_DEVWRITE("upd", combatsc_voice_reset_w)			/* upd7759 reset? */

	AM_RANGE(0xd000, 0xd000) AM_READ(soundlatch_r)								/* soundlatch_r? */
	AM_RANGE(0xe000, 0xe001) AM_DEVREADWRITE("ymsnd", combatsc_ym2203_r, ym2203_w)	/* YM 2203 intercepted */
ADDRESS_MAP_END


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( common_inputs )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(	0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On )  )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x00, "SW3:2" )	/* Not Used according to the manual */
	PORT_SERVICE_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW3:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x00, "SW3:4" )	/* Not Used according to the manual */
INPUT_PORTS_END

static INPUT_PORTS_START( dips )
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
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	/* None = coin slot B disabled */

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW2:1" )	/* Not Used according to the manual */
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW2:2" )	/* Not Used according to the manual */
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(	0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(	0x04, DEF_STR( Cocktail ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW2:4" )	/* Not Used according to the manual */
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW2:5" )	/* Not Used according to the manual */
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING( 0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING( 0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING( 0x20, DEF_STR( Difficult ) )
	PORT_DIPSETTING( 0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING( 0x80, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( combatsc )
	PORT_INCLUDE( dips )

	PORT_INCLUDE( common_inputs )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
INPUT_PORTS_END

static INPUT_PORTS_START( combatsct )
	PORT_INCLUDE( dips )

	PORT_INCLUDE( common_inputs )

	/* trackball 1P */
	PORT_START("TRACK0_Y")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(10) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("TRACK0_X")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(10) PORT_KEYDELTA(10) PORT_PLAYER(1)

	/* trackball 2P (not implemented yet) */
	PORT_START("TRACK1_Y")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(10) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(2)

	PORT_START("TRACK1_X")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(10) PORT_KEYDELTA(10) PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( combatscb )
	PORT_INCLUDE( dips )

	PORT_MODIFY("DSW2")
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x00, "SW2:3" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(	0x10, DEF_STR( No ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Yes ) )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout gfxlayout =
{
	8,8,
	0x4000,
	4,
	{ 0,1,2,3 },
	{ 0, 4, 8, 12, 16, 20, 24, 28},
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static const gfx_layout tile_layout =
{
	8,8,
	0x2000, /* number of tiles */
	4,		/* bitplanes */
	{ 0*0x10000*8, 1*0x10000*8, 2*0x10000*8, 3*0x10000*8 }, /* plane offsets */
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8 },
	8*8
};

static const gfx_layout sprite_layout =
{
	16,16,
	0x800,	/* number of sprites */
	4,		/* bitplanes */
	{ 3*0x10000*8, 2*0x10000*8, 1*0x10000*8, 0*0x10000*8 }, /* plane offsets */
	{
		0,1,2,3,4,5,6,7,
		16*8+0,16*8+1,16*8+2,16*8+3,16*8+4,16*8+5,16*8+6,16*8+7
	},
	{
		0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8,
		8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8
	},
	8*8*4
};

static GFXDECODE_START( combatsc )
	GFXDECODE_ENTRY( "gfx1", 0x00000, gfxlayout, 0, 8*16 )
	GFXDECODE_ENTRY( "gfx2", 0x00000, gfxlayout, 0, 8*16 )
GFXDECODE_END

static GFXDECODE_START( combatscb )
	GFXDECODE_ENTRY( "gfx1", 0x00000, tile_layout,   0, 8*16 )
	GFXDECODE_ENTRY( "gfx1", 0x40000, tile_layout,   0, 8*16 )
	GFXDECODE_ENTRY( "gfx2", 0x00000, sprite_layout, 0, 8*16 )
	GFXDECODE_ENTRY( "gfx2", 0x40000, sprite_layout, 0, 8*16 )
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

static const ym2203_interface ym2203_config =
{
	{
		AY8910_LEGACY_OUTPUT,
		AY8910_DEFAULT_LOADS,
		DEVCB_NULL,
		DEVCB_NULL,
		DEVCB_HANDLER(combatsc_portA_w),
		DEVCB_NULL
	},
	NULL
};


static MACHINE_START( combatsc )
{
	combatsc_state *state = (combatsc_state *)machine->driver_data;
	UINT8 *MEM = memory_region(machine, "maincpu") + 0x38000;

	state->io_ram  = MEM + 0x0000;
	state->page[0] = MEM + 0x4000;
	state->page[1] = MEM + 0x6000;

	state->interleave_timer = timer_alloc(machine, NULL, NULL);

	state->audiocpu = machine->device<cpu_device>("audiocpu");
	state->k007121_1 = machine->device("k007121_1");
	state->k007121_2 = machine->device("k007121_2");

	memory_configure_bank(machine, "bank1", 0, 10, memory_region(machine, "maincpu") + 0x10000, 0x4000);

	state_save_register_global(machine, state->priority);
	state_save_register_global(machine, state->vreg);
	state_save_register_global(machine, state->bank_select);
	state_save_register_global(machine, state->video_circuit);
	state_save_register_global(machine, state->boost);
	state_save_register_global_array(machine, state->prot);
	state_save_register_global_array(machine, state->pos);
	state_save_register_global_array(machine, state->sign);
}


static MACHINE_RESET( combatsc )
{
	combatsc_state *state = (combatsc_state *)machine->driver_data;
	const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);
	int i;

	memset(state->io_ram,  0x00, 0x4000);
	memset(state->page[0], 0x00, 0x2000);
	memset(state->page[1], 0x00, 0x2000);

	state->vreg = -1;
	state->boost = 1;
	state->bank_select = -1;
	state->prot[0] = 0;
	state->prot[1] = 0;

	for (i = 0; i < 4; i++)
	{
		state->pos[i] = 0;
		state->sign[i] = 0;
	}

	combatsc_bankselect_w(space, 0, 0);
}

/* combat school (original) */
static MACHINE_DRIVER_START( combatsc )

	/* driver data */
	MDRV_DRIVER_DATA(combatsc_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", HD6309, 3000000*4)	/* 3 MHz? */
	MDRV_CPU_PROGRAM_MAP(combatsc_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_CPU_ADD("audiocpu", Z80,3579545)	/* 3.579545 MHz */
	MDRV_CPU_PROGRAM_MAP(combatsc_sound_map)

	MDRV_QUANTUM_TIME(HZ(1200))

	MDRV_MACHINE_START(combatsc)
	MDRV_MACHINE_RESET(combatsc)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(combatsc)
	MDRV_PALETTE_LENGTH(8*16*16)

	MDRV_PALETTE_INIT(combatsc)
	MDRV_VIDEO_START(combatsc)
	MDRV_VIDEO_UPDATE(combatsc)

	MDRV_K007121_ADD("k007121_1")
	MDRV_K007121_ADD("k007121_2")

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ymsnd", YM2203, 3000000)
	MDRV_SOUND_CONFIG(ym2203_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.20)

	MDRV_SOUND_ADD("upd", UPD7759, UPD7759_STANDARD_CLOCK)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.70)
MACHINE_DRIVER_END

/* combat school (bootleg on different hardware) */
static MACHINE_DRIVER_START( combatscb )

	/* driver data */
	MDRV_DRIVER_DATA(combatsc_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", HD6309, 3000000*4)	/* 3 MHz? */
	MDRV_CPU_PROGRAM_MAP(combatscb_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_CPU_ADD("audiocpu", Z80,3579545)	/* 3.579545 MHz */
	MDRV_CPU_PROGRAM_MAP(combatsc_sound_map) /* FAKE */

	MDRV_QUANTUM_TIME(HZ(1200))

	MDRV_MACHINE_START(combatsc)
	MDRV_MACHINE_RESET(combatsc)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(combatscb)
	MDRV_PALETTE_LENGTH(8*16*16)

	MDRV_PALETTE_INIT(combatscb)
	MDRV_VIDEO_START(combatscb)
	MDRV_VIDEO_UPDATE(combatscb)

	/* We are using the original sound subsystem */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ymsnd", YM2203, 3000000)
	MDRV_SOUND_CONFIG(ym2203_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.20)

	MDRV_SOUND_ADD("upd", UPD7759, UPD7759_STANDARD_CLOCK)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.70)
MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( combatsc )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 6309 code */
	ROM_LOAD( "611g01.rom", 0x30000, 0x08000, CRC(857ffffe) SHA1(de7566d58314df4b7fdc07eb31a3f9bdd12d1a73) )
	ROM_CONTINUE(           0x08000, 0x08000 )
	ROM_LOAD( "611g02.rom", 0x10000, 0x20000, CRC(9ba05327) SHA1(ea03845fb49d18ac4fca97cfffce81db66b9967b) )
	/* extra 0x8000 for banked RAM */

	ROM_REGION( 0x10000 , "audiocpu", 0 ) /* sound CPU */
	ROM_LOAD( "611g03.rom", 0x00000, 0x08000, CRC(2a544db5) SHA1(94a97c3c54bf13ccc665aa5057ac6b1d700fae2d) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "611g07.rom",    0x00000, 0x40000, CRC(73b38720) SHA1(e109eb78aea464127d813284ca040e8d719599e3) )
	ROM_LOAD16_BYTE( "611g08.rom",    0x00001, 0x40000, CRC(46e7d28c) SHA1(1ece7fac954204ac35d00f3d573964fcf82dcf77) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "611g11.rom",    0x00000, 0x40000, CRC(69687538) SHA1(4349a1c052a759acdf7259f8bf8c5c9489b788f2) )
	ROM_LOAD16_BYTE( "611g12.rom",    0x00001, 0x40000, CRC(9c6bf898) SHA1(eafc227b4e7df0c652ec7d78784c039c35965fdc) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "611g06.h14",  0x0000, 0x0100, CRC(f916129a) SHA1(d5e4a8a3baab8fcdac86ef5182858cede1abf040) ) /* sprites lookup table */
	ROM_LOAD( "611g05.h15",  0x0100, 0x0100, CRC(207a7b07) SHA1(f4e638e7f182e5228a062b243406d0ceaaa5bfdc) ) /* chars lookup table */
	ROM_LOAD( "611g10.h6",   0x0200, 0x0100, CRC(f916129a) SHA1(d5e4a8a3baab8fcdac86ef5182858cede1abf040) ) /* sprites lookup table */
	ROM_LOAD( "611g09.h7",   0x0300, 0x0100, CRC(207a7b07) SHA1(f4e638e7f182e5228a062b243406d0ceaaa5bfdc) ) /* chars lookup table */

	ROM_REGION( 0x20000, "upd", 0 )	/* uPD7759 data */
	ROM_LOAD( "611g04.rom",  0x00000, 0x20000, CRC(2987e158) SHA1(87c5129161d3be29a339083349807e60b625c3f7) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "ampal16l8.e7", 0x0000, 0x0104, CRC(300a9936) SHA1(a4a87e93f41392fc7d7d8601d7187d87b9f9ab01) )
	ROM_LOAD( "pal16r6.16d",  0x0200, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal20l8.8h",   0x0400, 0x0144, NO_DUMP ) /* PAL is read protected */
ROM_END

ROM_START( combatsct )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 6309 code */
	ROM_LOAD( "g01.rom",     0x30000, 0x08000, CRC(489c132f) SHA1(c717195f89add4be4a21ecc1ddd58361b0ab4a74) )
	ROM_CONTINUE(            0x08000, 0x08000 )
	ROM_LOAD( "611g02.rom",  0x10000, 0x20000, CRC(9ba05327) SHA1(ea03845fb49d18ac4fca97cfffce81db66b9967b) )
	/* extra 0x8000 for banked RAM */

	ROM_REGION( 0x10000 , "audiocpu", 0 ) /* sound CPU */
	ROM_LOAD( "611g03.rom", 0x00000, 0x08000, CRC(2a544db5) SHA1(94a97c3c54bf13ccc665aa5057ac6b1d700fae2d) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "611g07.rom",    0x00000, 0x40000, CRC(73b38720) SHA1(e109eb78aea464127d813284ca040e8d719599e3) )
	ROM_LOAD16_BYTE( "611g08.rom",    0x00001, 0x40000, CRC(46e7d28c) SHA1(1ece7fac954204ac35d00f3d573964fcf82dcf77) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "611g11.rom",    0x00000, 0x40000, CRC(69687538) SHA1(4349a1c052a759acdf7259f8bf8c5c9489b788f2) )
	ROM_LOAD16_BYTE( "611g12.rom",    0x00001, 0x40000, CRC(9c6bf898) SHA1(eafc227b4e7df0c652ec7d78784c039c35965fdc) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "611g06.h14",  0x0000, 0x0100, CRC(f916129a) SHA1(d5e4a8a3baab8fcdac86ef5182858cede1abf040) ) /* sprites lookup table */
	ROM_LOAD( "611g05.h15",  0x0100, 0x0100, CRC(207a7b07) SHA1(f4e638e7f182e5228a062b243406d0ceaaa5bfdc) ) /* chars lookup table */
	ROM_LOAD( "611g10.h6",   0x0200, 0x0100, CRC(f916129a) SHA1(d5e4a8a3baab8fcdac86ef5182858cede1abf040) ) /* sprites lookup table */
	ROM_LOAD( "611g09.h7",   0x0300, 0x0100, CRC(207a7b07) SHA1(f4e638e7f182e5228a062b243406d0ceaaa5bfdc) ) /* chars lookup table */

	ROM_REGION( 0x20000, "upd", 0 )	/* uPD7759 data */
	ROM_LOAD( "611g04.rom",  0x00000, 0x20000, CRC(2987e158) SHA1(87c5129161d3be29a339083349807e60b625c3f7) )
ROM_END

ROM_START( combatscj )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 6309 code */
	ROM_LOAD( "611p01.a14",  0x30000, 0x08000, CRC(d748268e) SHA1(91588b6a0d3af47065204b980a56544a9f29b6d9) )
	ROM_CONTINUE(            0x08000, 0x08000 )
	ROM_LOAD( "611g02.rom",  0x10000, 0x20000, CRC(9ba05327) SHA1(ea03845fb49d18ac4fca97cfffce81db66b9967b) )
	/* extra 0x8000 for banked RAM */

	ROM_REGION( 0x10000 , "audiocpu", 0 ) /* sound CPU */
	ROM_LOAD( "611g03.rom", 0x00000, 0x08000, CRC(2a544db5) SHA1(94a97c3c54bf13ccc665aa5057ac6b1d700fae2d) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "611g07.rom",    0x00000, 0x40000, CRC(73b38720) SHA1(e109eb78aea464127d813284ca040e8d719599e3) )
	ROM_LOAD16_BYTE( "611g08.rom",    0x00001, 0x40000, CRC(46e7d28c) SHA1(1ece7fac954204ac35d00f3d573964fcf82dcf77) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "611g11.rom",    0x00000, 0x40000, CRC(69687538) SHA1(4349a1c052a759acdf7259f8bf8c5c9489b788f2) )
	ROM_LOAD16_BYTE( "611g12.rom",    0x00001, 0x40000, CRC(9c6bf898) SHA1(eafc227b4e7df0c652ec7d78784c039c35965fdc) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "611g06.h14",  0x0000, 0x0100, CRC(f916129a) SHA1(d5e4a8a3baab8fcdac86ef5182858cede1abf040) ) /* sprites lookup table */
	ROM_LOAD( "611g05.h15",  0x0100, 0x0100, CRC(207a7b07) SHA1(f4e638e7f182e5228a062b243406d0ceaaa5bfdc) ) /* chars lookup table */
	ROM_LOAD( "611g10.h6",   0x0200, 0x0100, CRC(f916129a) SHA1(d5e4a8a3baab8fcdac86ef5182858cede1abf040) ) /* sprites lookup table */
	ROM_LOAD( "611g09.h7",   0x0300, 0x0100, CRC(207a7b07) SHA1(f4e638e7f182e5228a062b243406d0ceaaa5bfdc) ) /* chars lookup table */

	ROM_REGION( 0x20000, "upd", 0 )	/* uPD7759 data */
	ROM_LOAD( "611g04.rom",  0x00000, 0x20000, CRC(2987e158) SHA1(87c5129161d3be29a339083349807e60b625c3f7) )
ROM_END

ROM_START( bootcamp )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 6309 code */
	ROM_LOAD( "xxx-v01.12a", 0x30000, 0x08000, CRC(c10dca64) SHA1(f34de26e998b1501e430d46e96cdc58ebc68481e) )
	ROM_CONTINUE(            0x08000, 0x08000 )
	ROM_LOAD( "611g02.rom",  0x10000, 0x20000, CRC(9ba05327) SHA1(ea03845fb49d18ac4fca97cfffce81db66b9967b) )
	/* extra 0x8000 for banked RAM */

	ROM_REGION( 0x10000 , "audiocpu", 0 ) /* sound CPU */
	ROM_LOAD( "611g03.rom", 0x00000, 0x08000, CRC(2a544db5) SHA1(94a97c3c54bf13ccc665aa5057ac6b1d700fae2d) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "611g07.rom",    0x00000, 0x40000, CRC(73b38720) SHA1(e109eb78aea464127d813284ca040e8d719599e3) )
	ROM_LOAD16_BYTE( "611g08.rom",    0x00001, 0x40000, CRC(46e7d28c) SHA1(1ece7fac954204ac35d00f3d573964fcf82dcf77) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "611g11.rom",    0x00000, 0x40000, CRC(69687538) SHA1(4349a1c052a759acdf7259f8bf8c5c9489b788f2) )
	ROM_LOAD16_BYTE( "611g12.rom",    0x00001, 0x40000, CRC(9c6bf898) SHA1(eafc227b4e7df0c652ec7d78784c039c35965fdc) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "611g06.h14",  0x0000, 0x0100, CRC(f916129a) SHA1(d5e4a8a3baab8fcdac86ef5182858cede1abf040) ) /* sprites lookup table */
	ROM_LOAD( "611g05.h15",  0x0100, 0x0100, CRC(207a7b07) SHA1(f4e638e7f182e5228a062b243406d0ceaaa5bfdc) ) /* chars lookup table */
	ROM_LOAD( "611g10.h6",   0x0200, 0x0100, CRC(f916129a) SHA1(d5e4a8a3baab8fcdac86ef5182858cede1abf040) ) /* sprites lookup table */
	ROM_LOAD( "611g09.h7",   0x0300, 0x0100, CRC(207a7b07) SHA1(f4e638e7f182e5228a062b243406d0ceaaa5bfdc) ) /* chars lookup table */

    ROM_REGION( 0x20000, "upd", 0 )	/* uPD7759 data */
	ROM_LOAD( "611g04.rom",  0x00000, 0x20000, CRC(2987e158) SHA1(87c5129161d3be29a339083349807e60b625c3f7) )
ROM_END

ROM_START( combatscb )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 6809 code */
	ROM_LOAD( "combat.002",	 0x30000, 0x08000, CRC(0996755d) SHA1(bb6bbbf7ab3b5fab5e1c6cebc7b3f0d720493c3b) )
	ROM_CONTINUE(            0x08000, 0x08000 )
	ROM_LOAD( "combat.003",	 0x10000, 0x10000, CRC(229c93b2) SHA1(ac3fd3df1bb5f6a461d0d1423c50568348ef69df) )
	ROM_LOAD( "combat.004",	 0x20000, 0x10000, CRC(a069cb84) SHA1(f49f70afb17df46b16f5801ef42edb0706730723) )
	/* extra 0x8000 for banked RAM */

	ROM_REGION( 0x10000 , "audiocpu", 0 ) /* sound CPU */
	ROM_LOAD( "combat.001",  0x00000, 0x10000, CRC(61456b3b) SHA1(320db628283dd1bec465e95020d1a1158e6d6ae4) )
	ROM_LOAD( "611g03.rom",  0x00000, 0x08000, CRC(2a544db5) SHA1(94a97c3c54bf13ccc665aa5057ac6b1d700fae2d) ) /* FAKE - from Konami set! */

	ROM_REGION( 0x80000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "combat.006",  0x00000, 0x10000, CRC(8dc29a1f) SHA1(564dd7c6acff34db93b8e300dda563f5f38ba159) ) /* tiles, bank 0 */
	ROM_LOAD( "combat.008",  0x10000, 0x10000, CRC(61599f46) SHA1(cfd79a88bb496773daf207552c67f595ee696bc4) )
	ROM_LOAD( "combat.010",  0x20000, 0x10000, CRC(d5cda7cd) SHA1(140db6270c3f358aa27013db3bb819a48ceb5142) )
	ROM_LOAD( "combat.012",  0x30000, 0x10000, CRC(ca0a9f57) SHA1(d6b3daf7c34345bb2f64068d480bd51d7bb36e4d) )
	ROM_LOAD( "combat.005",  0x40000, 0x10000, CRC(0803a223) SHA1(67d4162385dd56d5396e181070bfa6760521eb45) ) /* tiles, bank 1 */
	ROM_LOAD( "combat.007",  0x50000, 0x10000, CRC(23caad0c) SHA1(0544cde479c6d4192da5bb4b6f0e2e75d09663c3) )
	ROM_LOAD( "combat.009",  0x60000, 0x10000, CRC(5ac80383) SHA1(1e89c371a92afc000d593daebda4156952a15244) )
	ROM_LOAD( "combat.011",  0x70000, 0x10000, CRC(cda83114) SHA1(12d2a9f694287edb3bb0ee7a8ba0e0724dad8e1f) )

	ROM_REGION( 0x80000, "gfx2", ROMREGION_INVERT )
	ROM_LOAD( "combat.013",  0x00000, 0x10000, CRC(4bed2293) SHA1(3369de47d4ba041d9f17a18dcca2af7ac9f8bc0c) ) /* sprites, bank 0 */
	ROM_LOAD( "combat.015",  0x10000, 0x10000, CRC(26c41f31) SHA1(f8eb7d0729a21a0dd92ce99c9cda0cde9526b861) )
	ROM_LOAD( "combat.017",  0x20000, 0x10000, CRC(6071e6da) SHA1(ba5f8e83b07faaffc564d3568630e17efdb5a09f) )
	ROM_LOAD( "combat.019",  0x30000, 0x10000, CRC(3b1cf1b8) SHA1(ff4de37c051bcb374c44d1b99006ff6ff5e1f927) )
	ROM_LOAD( "combat.014",  0x40000, 0x10000, CRC(82ea9555) SHA1(59bf7836938ce9e3242d1cca754de8dbe85bbfb7) ) /* sprites, bank 1 */
	ROM_LOAD( "combat.016",  0x50000, 0x10000, CRC(2e39bb70) SHA1(a6c4acd93cc803e987de6e18fbdc5ce4634b14a8) )
	ROM_LOAD( "combat.018",  0x60000, 0x10000, CRC(575db729) SHA1(6b1676da4f24fc90c77262789b6cc116184ab912) )
	ROM_LOAD( "combat.020",  0x70000, 0x10000, CRC(8d748a1a) SHA1(4386e14e19b91e053033dde2a13019bc6d8e1d5a) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "prom.d10",    0x0000, 0x0100, CRC(265f4c97) SHA1(76f1b75a593d3d77ef6173a1948f842d5b27d418) ) /* sprites lookup table */
	ROM_LOAD( "prom.c11",    0x0100, 0x0100, CRC(a7a5c0b4) SHA1(48bfc3af40b869599a988ebb3ed758141bcfd4fc) ) /* priority? */

	ROM_REGION( 0x20000, "upd", 0 )	/* uPD7759 data */
	ROM_LOAD( "611g04.rom",  0x00000, 0x20000, CRC(2987e158) SHA1(87c5129161d3be29a339083349807e60b625c3f7) )	/* FAKE - from Konami set! */
ROM_END


/*************************************
 *
 *  Driver initialization
 *
 *************************************/

static DRIVER_INIT( combatsc )
{
	/* joystick instead of trackball */
	memory_install_read_port(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x0404, 0x0404, 0, 0, "IN1");
}


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1988, combatsc,  0,        combatsc,  combatsc,  combatsc,  ROT0, "Konami",  "Combat School (joystick)", 0 )
GAME( 1987, combatsct, combatsc, combatsc,  combatsct, 0,         ROT0, "Konami",  "Combat School (trackball)", 0 )
GAME( 1987, combatscj, combatsc, combatsc,  combatsct, 0,         ROT0, "Konami",  "Combat School (Japan trackball)", 0 )
GAME( 1987, bootcamp,  combatsc, combatsc,  combatsct, 0,         ROT0, "Konami",  "Boot Camp", 0 )
GAME( 1988, combatscb, combatsc, combatscb, combatscb, 0,         ROT0, "bootleg", "Combat School (bootleg)", GAME_IMPERFECT_COLORS )
