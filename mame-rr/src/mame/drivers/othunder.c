/***************************************************************************

Operation Thunderbolt  (Taito)
---------------------

David Graves

(this is based on the F2 driver by Bryan McPhail, Brad Oliver, Andrew Prime,
Nicola Salmoria. Thanks to Richard Bush and the Raine team, whose open
source was very helpful in many areas particularly the sprites.)

                *****

Operation Thunderbolt
Taito, 1988

PCB Layout
----------

K1100381A
J1100166A MAIN PCB
|-----------------------------------------------------------------------|
|     VOL     VOL        YM3016   B67-13.40  16MHz  24MHz               |
|                |------|                                           PAL |
|                |TAITO |         Z80                                   |
|                |TC0140|         6264          26.686MHz    6116  6116 |
|                |SYT   |                                    6116  6116 |
|                |------|         YM2610                     6116  6116 |
|    ADC0808     B67-08.67                                   6116  6116 |
|                                B67-07.44         |------|             |
|J               B67-06.66                         |TAITO |    |------| |
|A    |------|                                     |TC0020|    |TAITO | |
|M    |TAITO |                62256  62256         |VAR   |    |TC0050| |
|M    |TC0220|       |------|                      |------|    |VDZ   | |
|A    |IOC   |       |TAITO |                                  |------| |
|     |------|       |TC0100|  6116 6116                                |
|                    |SCN   |                        |------|           |
|    DSWA  DSWB      |------|         B67-05.43      |TAITO |           |
|                                                    |TC0050|           |
|                            |------|                |VDZ   |           |
|     |------|    B67-23.64  |TAITO |   |----|       |------|  B67-04.4 |
|     |TAITO |               |TC0320|   | 6  |                          |
| 555 |TC0110|    B67-20.63  |OBR   |   | 8  |  PAL  |------|  B67-03.3 |
|     |PCR   |               |------|   | 0  |       |TAITO |           |
|     |------|    B67-15.62    62256    | 0  |  PAL  |TC0050|  B67-02.2 |
|                                       | 0  |       |VDZ   |           |
|         6264    B67-14.61    62256    |----|  PAL  |------|  B67-01.1 |
|-----------------------------------------------------------------------|
Notes:
      68000 running at 12.000MHz [24/2]
      Z80 running at 4.000MHz [24/6]
      YM2610 running at 8.000MHz [16/2]
      Taito Custom ICs -
                         TC0220IOC
                         TC0110PCR
                         TC0140SYT
                         TC0100SCN
                         TC0320OBR
                         TC0020VAR
                         TC0050VDZ (x3)


Operation Thunderbolt operates on hardware very similar to the Taito Z
system, in particular the game Spacegun. The lightgun hardware in these
two (as well as the eerom and calibration process) looks identical.

The game has 4 separate layers of graphics - one 64x64 tiled scrolling
background plane of 8x8 tiles, a similar foreground plane, a sprite plane,
and a text plane with character definitions held in ram.

The sprites are 16x8 tiles aggregated through a spritemap rom into 64x64
zoomable sprites.

The main difference is that Operation Thunderbolt uses only a single 68000
CPU, whereas Spacegun has twin 68Ks. (Operation Thunderbolt has a Z80
taking over sound duties, which Spacegun doesn't.)


custom ICs
----------
TC0020VAR     sprites??
TC0050VDZ x3  sprites??
TC0070RGB     video DAC
TC0100SCN     tilemaps
TC0110PCR     palette
TC0140SYT     main/sub CPU interface + sub cpu address decoder and I/O interface
TC0220IOC     I/O interface
TC0310FAM x2  sound volume and panning
TC0320OBR     sprites


memory map
----------
68000:

The address decoding is done by two PALs (IC37 and IC33). Part of the decoding,
and also interrupt control, is done by another PAL (IC36). Luckily this time,
the PALs HAVE been read, so the memory map is accurate :)

Address                  Dir Data             Name      Description
------------------------ --- ---------------- --------- -----------------------
000000xxxxxxxxxxxxxxxxx- R   xxxxxxxxxxxxxxxx TROM0     program ROM
000001xxxxxxxxxxxxxxxxx- R   xxxxxxxxxxxxxxxx TROM1     program ROM
00001000xxxxxxxxxxxxxxxx R/W xxxxxxxxxxxxxxxx TRAMH     work RAM
00001001-----------xxxx- R/W --------xxxxxxxx II/O      TC0220IOC
0001-----------------xxx R/W xxxxxxxxxxxxxxxx CLCS      TC0110PCR
0010--xxxxxxxxxxxxxxxxxx R/W xxxxxxxxxxxxxxxx SCCS      TC0100SCN
0011------------------xx R/W ------------xxxx SSCS      TC0140SYT
0100---------xxxxxxxxxxx R/W xxxxxxxxxxxxxxxx OBCS      TC0320OBR
0101----------------xxx- R/W --------xxxxxxxx A/DOE     ADC0808 A/D converter (for lightgun)
0110----------------000-   W ----------------           IRQ5 acknowledge (automatically enabled on reset)
0110----------------001-   W ---------------- 6PR       IRQ6 acknowledge (automatically enabled on reset)
0110----------------010-   W ---------------- n.c.
0110----------------011-   W ---------------- n.c.
0110----------------100-   W ---------------- n.c.
0110----------------101-   W ---------------- n.c.
0110----------------110-   W ---------------- n.c.
0110----------------111-   W ---------------- n.c.


Z80:

all of the address decoding is done by the TC0140SYT, which uses address bits
A0 and A9-A15, and data bits D0-D3.

Address          Dir Data     Name        Description
---------------- --- -------- ----------- -----------------------
00xxxxxxxxxxxxxx R   xxxxxxxx ROM         program ROM (TC0140SYT ROMCS0 pin)
01xxxxxxxxxxxxxx R   xxxxxxxx ROM         program ROM (banked) (TC0140SYT ROMCS0, ROMA14 and ROMA15 pins)
110xxxxxxxxxxxxx R/W xxxxxxxx SRAM        work RAM (TC0140SYT RAMCS pin)
1110000-------xx R/W xxxxxxxx OP_T        YM2610 (TC0140SYT OPX pin)
1110001--------x R/W ----xxxx             TC0140SYT control
1110010-------00   W ---xxxxx CH1_VOLR_CT TC0310FAM #1 CS2 (TC0140SYT CSA pin)
1110010-------01   W ---xxxxx CH1_VOLF_CT TC0310FAM #1 CS1 (TC0140SYT CSA pin)
1110010-------10   W ---xxxxx CH2_VOLR_CT TC0310FAM #2 CS2 (TC0140SYT CSA pin)
1110010-------11   W ---xxxxx CH2_VOLF_CT TC0310FAM #2 CS1 (TC0140SYT CSA pin)
1110011---------   W                      ??
1110100---------
1110101--------- R   -------x ROTARY 1    TC0140SYT I/OA0 pin
1110101--------- R   ------x- ROTARY 2    TC0140SYT I/OA1 pin
1110101--------- R   -----x-- ROTARY 3    TC0140SYT I/OA2 pin
1110110---------
1110111---------   W                      ??
1111000---------   W                      ??
1111001---------   W ----xxxx             TC0140SYT ROM bankswitch



Notes:
------
- The game checks an external input called ROTARY in the schematics, not
  mentioned in the manual. The ROTARY input controls the separation between
  left and right players, the game adjusts the values written to the external
  volume controller depending on it.
  Possible values are: 111 (max) 011 (high) -01 (med) --0 (low). It's a
  rotary control so only one bit is supposed to be low.

(Changed set led output to proper output port.  --HowardC)
- The outputs I'm turning on are actually the gun solenoid outputs, which
  would rattle the gun while firing.


- BM, 060108 - The original flyer for this game has screenshots which clearly
  show the background is 4 pixels to the left on several game stages (you can
  see the edge of sprites overlapping past the right edge).  Therefore I
  do not believe the TC0100SCN problem mentioned above actually exists.  The
  current emulation appears to be accurate.


Stephh's notes (based on the game M68000 code and some tests) :

1) 'othunder', 'othundrj' and 'othunduo'

  - Region stored at 0x03fffe.w
  - Sets :
      * 'othunder' : region = 0x0003
      * 'othundrj' : region = 0x0001
      * 'othunduo' : region = 0x0002
  - These 3 games are 100% the same, only region differs !
  - Coinage relies on the region (code at 0x000db2) :
      * 0x0001 (Japan) and 0x0002 (US) use TAITO_COINAGE_JAPAN_OLD
      * 0x0003 (World) and 0x0004 (licensed to xxx) use TAITO_COINAGE_WORLD
  - Notice screen only if region = 0x0001
  - According to the manual, DSWB bit 6 determines continue pricing :

    PORT_DIPNAME( 0x40, 0x40, DEF_STR( Continue_Price ) )
    PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
    PORT_DIPSETTING(    0x40, "Same as Start" )

    However, many conditions are required to make it work due to code at 0x00e0c4 :
      * region must not be 0x0001
      * "Allow Continue" Dip Switch must be set to "Yes"
      * coinage must be 2C_1C for both slots
    This is why this Dip Switch has NO effect in the following sets :
      * 'othunder' : coinage can't be 2C_1C for the 2 slots (coin B)
      * 'othundrj' : region = 0x0001
  - DSWB bit 7 ("Language") affects parts of the texts (not the ones in "demo mode")
    but the voices are always in English regardless of the region !


2) 'othundu'

  - Region stored at 0x03fffe.w
  - Sets :
      * 'othundu' : region = 0x0002
  - Comparison with 'othunder' :
      * slightly different code at 0x023c4c
      * additional data from 0x023eee to 0x0240ed (0x0200 bytes)
      * same other notes as for 'othunder'


TODO:
-----

- With the correct clock speed of 12MHz for the 68000, garbage graphics remain
  over the Taito logo on startup. This seems to be a bug in the original which
  would have no effect if our timing was 100% right. The interrupt handling
  should be quite correct, it's derived straight from the schematics and PAL
  dump.
  The current workaround is to make the 68000 run at 13MHz. Lowering below
  12MHz would work as well, and possibly be closer to the real reason (wait
  states slowing the CPU down?)

- The "FIRE!" arrows pointing to padlocks are not in perfect sync with the
  background scrolling. Should they?

- The quality of the zoomed sprites could probably be better. Drawing them as
  made by 16x8 tiles loses precision due to limitations of drawgfxzoom().

- Schematics show a OBPRI output to control sprite priority. This doesn't seem
  to be used however, and isn't hooked up. See othunder_TC0220IOC_w().

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "machine/eeprom.h"
#include "video/taitoic.h"
#include "machine/taitoio.h"
#include "audio/taitosnd.h"
#include "sound/2610intf.h"
#include "sound/flt_vol.h"
#include "includes/othunder.h"
#include "includes/taitoipt.h"


/***********************************************************
                INTERRUPTS
***********************************************************/

static void update_irq( running_machine *machine )
{
	othunder_state *state = (othunder_state *)machine->driver_data;
	cpu_set_input_line(state->maincpu, 6, state->ad_irq ? ASSERT_LINE : CLEAR_LINE);
	cpu_set_input_line(state->maincpu, 5, state->vblank_irq ? ASSERT_LINE : CLEAR_LINE);
}

static WRITE16_HANDLER( irq_ack_w )
{
	othunder_state *state = (othunder_state *)space->machine->driver_data;

	switch (offset)
	{
		case 0:
			state->vblank_irq = 0;
			break;

		case 1:
			state->ad_irq = 0;
			break;
	}

	update_irq(space->machine);
}

static INTERRUPT_GEN( vblank_interrupt )
{
	othunder_state *state = (othunder_state *)device->machine->driver_data;

	state->vblank_irq = 1;
	update_irq(device->machine);
}

static TIMER_CALLBACK( ad_interrupt )
{
	othunder_state *state = (othunder_state *)machine->driver_data;

	state->ad_irq = 1;
	update_irq(machine);
}


/******************************************************************
                    EEPROM

This is an earlier version of the eeprom used in some TaitoB games.
The eeprom unlock command is different, and the write/clock/reset
bits are different.
******************************************************************/

static const eeprom_interface eeprom_intf =
{
	6,				/* address bits */
	16,				/* data bits */
	"0110",			/* read command */
	"0101",			/* write command */
	"0111",			/* erase command */
	"0100000000",	/* lock command */
	"0100111111"	/* unlock command */
};

static WRITE16_HANDLER( othunder_tc0220ioc_w )
{
	othunder_state *state = (othunder_state *)space->machine->driver_data;

	if (ACCESSING_BITS_0_7)
	{
		switch (offset)
		{
			case 0x03:

/*              0000000x    SOL-1 (gun solenoid)
                000000x0    SOL-2 (gun solenoid)
                00000x00    OBPRI (sprite priority)
                0000x000    (unused)
                000x0000    eeprom reset (active low)
                00x00000    eeprom clock
                0x000000    eeprom in data
                x0000000    eeprom out data  */

				/* Recoil Piston Motor Status */
				output_set_value("Player1_Recoil_Piston", data & 0x1 );
				output_set_value("Player2_Recoil_Piston", (data & 0x2) >>1 );

				if (data & 4)
					popmessage("OBPRI SET!");

				eeprom_write_bit(state->eeprom, data & 0x40);
				eeprom_set_clock_line(state->eeprom, (data & 0x20) ? ASSERT_LINE : CLEAR_LINE);
				eeprom_set_cs_line(state->eeprom, (data & 0x10) ? CLEAR_LINE : ASSERT_LINE);
				break;

			default:
				tc0220ioc_w(state->tc0220ioc, offset, data & 0xff);
		}
	}
}


/**********************************************************
            GAME INPUTS
**********************************************************/

static READ16_HANDLER( othunder_tc0220ioc_r )
{
	othunder_state *state = (othunder_state *)space->machine->driver_data;

	switch (offset)
	{
		case 0x03:
			return (eeprom_read_bit(state->eeprom) & 1) << 7;

		default:
			return tc0220ioc_r(state->tc0220ioc, offset);
	}
}

#define P1X_PORT_TAG     "P1X"
#define P1Y_PORT_TAG     "P1Y"
#define P2X_PORT_TAG     "P2X"
#define P2Y_PORT_TAG     "P2Y"
#define ROTARY_PORT_TAG  "ROTARY"

static READ16_HANDLER( othunder_lightgun_r )
{
	static const char *const portname[4] = { P1X_PORT_TAG, P1Y_PORT_TAG, P2X_PORT_TAG, P2Y_PORT_TAG };
	return input_port_read(space->machine, portname[offset]);
}

static WRITE16_HANDLER( othunder_lightgun_w )
{
	/* A write starts the A/D conversion. An interrupt will be triggered when
       the conversion is complete.
       The ADC60808 clock is 512kHz. Conversion takes between 0 and 8 clock
       cycles, so would end in a maximum of 15.625us. We'll use 10. */

	timer_set(space->machine, ATTOTIME_IN_USEC(10), NULL,0, ad_interrupt);
}


/*****************************************
            SOUND
*****************************************/

static void reset_sound_region( running_machine *machine )
{
	othunder_state *state = (othunder_state *)machine->driver_data;
	memory_set_bank(machine, "bank10", state->banknum);
}


static WRITE8_HANDLER( sound_bankswitch_w )
{
	othunder_state *state = (othunder_state *)space->machine->driver_data;
	state->banknum = data & 7;
	reset_sound_region(space->machine);
}

static WRITE16_HANDLER( othunder_sound_w )
{
	othunder_state *state = (othunder_state *)space->machine->driver_data;
	if (offset == 0)
		tc0140syt_port_w(state->tc0140syt, 0, data & 0xff);
	else if (offset == 1)
		tc0140syt_comm_w(state->tc0140syt, 0, data & 0xff);
}

static READ16_HANDLER( othunder_sound_r )
{
	othunder_state *state = (othunder_state *)space->machine->driver_data;
	if (offset == 1)
		return ((tc0140syt_comm_r(state->tc0140syt, 0) & 0xff));
	else
		return 0;
}

static WRITE8_HANDLER( othunder_TC0310FAM_w )
{
	/* there are two TC0310FAM, one for CH1 and one for CH2 from the YM2610. The
       PSG output is routed to both chips. */
	othunder_state *state = (othunder_state *)space->machine->driver_data;
	int voll, volr;

	state->pan[offset] = data & 0x1f;

	/* PSG output (single ANALOG OUT pin on the YM2610, but we have three channels
       because we are using the AY-3-8910 emulation. */
	volr = (state->pan[0] + state->pan[2]) * 100 / (2 * 0x1f);
	voll = (state->pan[1] + state->pan[3]) * 100 / (2 * 0x1f);
	flt_volume_set_volume(state->_2610_0l, voll / 100.0);
	flt_volume_set_volume(state->_2610_0r, volr / 100.0);

	/* CH1 */
	volr = state->pan[0] * 100 / 0x1f;
	voll = state->pan[1] * 100 / 0x1f;
	flt_volume_set_volume(state->_2610_1l, voll / 100.0);
	flt_volume_set_volume(state->_2610_1r, volr / 100.0);

	/* CH2 */
	volr = state->pan[2] * 100 / 0x1f;
	voll = state->pan[3] * 100 / 0x1f;
	flt_volume_set_volume(state->_2610_2l, voll / 100.0);
	flt_volume_set_volume(state->_2610_2r, volr / 100.0);
}


/***********************************************************
             MEMORY STRUCTURES
***********************************************************/

static ADDRESS_MAP_START( othunder_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x080000, 0x08ffff) AM_RAM
	AM_RANGE(0x090000, 0x09000f) AM_READWRITE(othunder_tc0220ioc_r, othunder_tc0220ioc_w)
//  AM_RANGE(0x090006, 0x090007) AM_WRITE(eeprom_w)
//  AM_RANGE(0x09000c, 0x09000d) AM_WRITENOP   /* ?? (keeps writing 0x77) */
	AM_RANGE(0x100000, 0x100007) AM_DEVREADWRITE("tc0110pcr", tc0110pcr_word_r, tc0110pcr_step1_rbswap_word_w)	/* palette */
	AM_RANGE(0x200000, 0x20ffff) AM_DEVREADWRITE("tc0100scn", tc0100scn_word_r, tc0100scn_word_w)	/* tilemaps */
	AM_RANGE(0x220000, 0x22000f) AM_DEVREADWRITE("tc0100scn", tc0100scn_ctrl_word_r, tc0100scn_ctrl_word_w)
	AM_RANGE(0x300000, 0x300003) AM_READWRITE(othunder_sound_r, othunder_sound_w)
	AM_RANGE(0x400000, 0x4005ff) AM_RAM AM_BASE_SIZE_MEMBER(othunder_state, spriteram, spriteram_size)
	AM_RANGE(0x500000, 0x500007) AM_READWRITE(othunder_lightgun_r, othunder_lightgun_w)
	AM_RANGE(0x600000, 0x600003) AM_WRITE(irq_ack_w)
ADDRESS_MAP_END


/***************************************************************************/

static ADDRESS_MAP_START( z80_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank10")
	AM_RANGE(0xc000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xe003) AM_DEVREADWRITE("ymsnd", ym2610_r, ym2610_w)
	AM_RANGE(0xe200, 0xe200) AM_READNOP AM_DEVWRITE("tc0140syt", tc0140syt_slave_port_w)
	AM_RANGE(0xe201, 0xe201) AM_DEVREADWRITE("tc0140syt", tc0140syt_slave_comm_r, tc0140syt_slave_comm_w)
	AM_RANGE(0xe400, 0xe403) AM_WRITE(othunder_TC0310FAM_w) /* pan */
	AM_RANGE(0xe600, 0xe600) AM_WRITENOP /* ? */
	AM_RANGE(0xea00, 0xea00) AM_READ_PORT(ROTARY_PORT_TAG)	/* rotary input */
	AM_RANGE(0xee00, 0xee00) AM_WRITENOP /* ? */
	AM_RANGE(0xf000, 0xf000) AM_WRITENOP /* ? */
	AM_RANGE(0xf200, 0xf200) AM_WRITE(sound_bankswitch_w)
ADDRESS_MAP_END



/***********************************************************
             INPUT PORTS, DIPs
***********************************************************/

static INPUT_PORTS_START( othunder )
	/* 0x090000 -> 0x08a000 */
	PORT_START("DSWA")
	PORT_DIPUNUSED( 0x01, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	TAITO_DSWA_BITS_2_TO_3
	TAITO_COINAGE_WORLD

	/* 0x090002 -> 0x08a002 */
	PORT_START("DSWB")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Magazines/Rockets" )
	PORT_DIPSETTING(    0x0c, "5/3" )
	PORT_DIPSETTING(    0x08, "6/4" )
	PORT_DIPSETTING(    0x04, "7/5" )
	PORT_DIPSETTING(    0x00, "8/6" )
	PORT_DIPNAME( 0x30, 0x30, "Bullets per Magazine" )
	PORT_DIPSETTING(    0x00, "30" )
	PORT_DIPSETTING(    0x10, "35" )
	PORT_DIPSETTING(    0x30, "40" )
	PORT_DIPSETTING(    0x20, "50" )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )                        /* see notes */
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Language ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Japanese ) )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN1")	/* unused */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	/* speed of 13 is compromise between moving aim around screen fast
       enough and being accurate enough not to miss targets. 20 is too
       inaccurate, and 10 is too slow. */

	PORT_START(P1X_PORT_TAG)
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(13) PORT_REVERSE PORT_PLAYER(1)

	PORT_START(P1Y_PORT_TAG)
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_CROSSHAIR(Y, 1.0, -0.057, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(13) PORT_PLAYER(1)

	PORT_START(P2X_PORT_TAG)
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(13) PORT_REVERSE PORT_PLAYER(2)

	PORT_START(P2Y_PORT_TAG)
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_CROSSHAIR(Y, 1.0, -0.057, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(13) PORT_PLAYER(2)

	/* rotary volume control */
	PORT_START(ROTARY_PORT_TAG)
	PORT_CONFNAME( 0x07, 0x07, "Stereo Separation" )
	PORT_CONFSETTING(    0x07, "Maximum" )
	PORT_CONFSETTING(    0x03, DEF_STR( High ) )
	PORT_CONFSETTING(    0x01, DEF_STR( Medium ) )
	PORT_CONFSETTING(    0x00, DEF_STR( Low ) )
INPUT_PORTS_END

static INPUT_PORTS_START( othundrj )
	PORT_INCLUDE( othunder )

	PORT_MODIFY( "DSWA" )
	TAITO_COINAGE_JAPAN_OLD
INPUT_PORTS_END

static INPUT_PORTS_START( othundu )
	PORT_INCLUDE( othundrj )

	PORT_MODIFY( "DSWB" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Continue_Price ) )        /* see notes */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, "Same as Start" )
INPUT_PORTS_END



/***********************************************************
                GFX DECODING
***********************************************************/

static const gfx_layout tile16x8_layout =
{
	16,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 8, 16, 24 },
	{ 32, 33, 34, 35, 36, 37, 38, 39, 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64 },
	64*8
};

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 2*4, 3*4, 0*4, 1*4, 6*4, 7*4, 4*4, 5*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static GFXDECODE_START( othunder )
	GFXDECODE_ENTRY( "gfx2", 0, tile16x8_layout, 0, 256 )	/* sprite parts */
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,      0, 256 )	/* sprites & playfield */
GFXDECODE_END



/**************************************************************
                 YM2610 (SOUND)
**************************************************************/

/* handler called by the YM2610 emulator when the internal timers cause an IRQ */
static void irqhandler( running_device *device, int irq )
{
	othunder_state *state = (othunder_state *)device->machine->driver_data;
	cpu_set_input_line(state->audiocpu, 0, irq ? ASSERT_LINE : CLEAR_LINE);
}

static const ym2610_interface ym2610_config =
{
	irqhandler
};



/***********************************************************
                 MACHINE DRIVERS
***********************************************************/

static const tc0100scn_interface othunder_tc0100scn_intf =
{
	"screen",
	1, 2,		/* gfxnum, txnum */
	4, 0,		/* x_offset, y_offset */
	0, 0,		/* flip_xoff, flip_yoff */
	0, 0,		/* flip_text_xoff, flip_text_yoff */
	0, 0
};

static const tc0110pcr_interface othunder_tc0110pcr_intf =
{
	0
};

static const tc0220ioc_interface othunder_io_intf =
{
	DEVCB_INPUT_PORT("DSWA"), DEVCB_INPUT_PORT("DSWB"),
	DEVCB_INPUT_PORT("IN0"), DEVCB_INPUT_PORT("IN1"), DEVCB_INPUT_PORT("IN2")	/* port read handlers */
};

static const tc0140syt_interface othunder_tc0140syt_intf =
{
	"maincpu", "audiocpu"
};

static STATE_POSTLOAD( othunder_postload )
{
	reset_sound_region(machine);
}

static MACHINE_START( othunder )
{
	othunder_state *state = (othunder_state *)machine->driver_data;

	memory_configure_bank(machine, "bank10", 0, 4, memory_region(machine, "audiocpu") + 0xc000, 0x4000);

	state->maincpu = machine->device("maincpu");
	state->audiocpu = machine->device("audiocpu");
	state->eeprom = machine->device("eeprom");
	state->tc0220ioc = machine->device("tc0220ioc");
	state->tc0100scn = machine->device("tc0100scn");
	state->tc0110pcr = machine->device("tc0110pcr");
	state->tc0140syt = machine->device("tc0140syt");
	state->_2610_0l = machine->device("2610.0l");
	state->_2610_0r = machine->device("2610.0r");
	state->_2610_1l = machine->device("2610.1l");
	state->_2610_1r = machine->device("2610.1r");
	state->_2610_2l = machine->device("2610.2l");
	state->_2610_2r = machine->device("2610.2r");

	state_save_register_global(machine, state->vblank_irq);
	state_save_register_global(machine, state->ad_irq);
	state_save_register_global(machine, state->banknum);
	state_save_register_global_array(machine, state->pan);
	state_save_register_postload(machine, othunder_postload, NULL);
}

static MACHINE_RESET( othunder )
{
	othunder_state *state = (othunder_state *)machine->driver_data;

	state->vblank_irq = 0;
	state->ad_irq = 0;
	state->banknum = 0;
}

static MACHINE_DRIVER_START( othunder )

	/* driver data */
	MDRV_DRIVER_DATA(othunder_state)

	/* basic machine hardware */
//  MDRV_CPU_ADD("maincpu", M68000, 24000000/2 )   /* 12 MHz */
	MDRV_CPU_ADD("maincpu", M68000, 13000000 )	/* fixes garbage graphics on startup */
	MDRV_CPU_PROGRAM_MAP(othunder_map)
	MDRV_CPU_VBLANK_INT("screen", vblank_interrupt)

	MDRV_CPU_ADD("audiocpu", Z80,16000000/4 )	/* 4 MHz */
	MDRV_CPU_PROGRAM_MAP(z80_sound_map)

	MDRV_EEPROM_ADD("eeprom", eeprom_intf)

	MDRV_MACHINE_START(othunder)
	MDRV_MACHINE_RESET(othunder)

	MDRV_TC0220IOC_ADD("tc0220ioc", othunder_io_intf)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(40*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 2*8, 32*8-1)

	MDRV_GFXDECODE(othunder)
	MDRV_PALETTE_LENGTH(4096)

	MDRV_VIDEO_START(othunder)
	MDRV_VIDEO_UPDATE(othunder)

	MDRV_TC0100SCN_ADD("tc0100scn", othunder_tc0100scn_intf)
	MDRV_TC0110PCR_ADD("tc0110pcr", othunder_tc0110pcr_intf)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2610, 16000000/2)
	MDRV_SOUND_CONFIG(ym2610_config)
	MDRV_SOUND_ROUTE(0, "2610.0l", 0.25)
	MDRV_SOUND_ROUTE(0, "2610.0r", 0.25)
	MDRV_SOUND_ROUTE(1, "2610.1l", 1.0)
	MDRV_SOUND_ROUTE(1, "2610.1r", 1.0)
	MDRV_SOUND_ROUTE(2, "2610.2l", 1.0)
	MDRV_SOUND_ROUTE(2, "2610.2r", 1.0)

	MDRV_SOUND_ADD("2610.0l", FILTER_VOLUME, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MDRV_SOUND_ADD("2610.0r", FILTER_VOLUME, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
	MDRV_SOUND_ADD("2610.1l", FILTER_VOLUME, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MDRV_SOUND_ADD("2610.1r", FILTER_VOLUME, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
	MDRV_SOUND_ADD("2610.2l", FILTER_VOLUME, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MDRV_SOUND_ADD("2610.2r", FILTER_VOLUME, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)

	MDRV_TC0140SYT_ADD("tc0140syt", othunder_tc0140syt_intf)
MACHINE_DRIVER_END



/***************************************************************************
                    DRIVERS
***************************************************************************/

ROM_START( othunder )
	ROM_REGION( 0x80000, "maincpu", 0 )	/* 512K for 68000 code */
	ROM_LOAD16_BYTE( "b67-20.63",   0x00000, 0x20000, CRC(21439ea2) SHA1(d5b5a194e9698cf43513c0d56146772e8132ab07) )
	ROM_LOAD16_BYTE( "b67-23.64",   0x00001, 0x20000, CRC(789e9daa) SHA1(15bb0eec68aeea0b9f55889566338c9ce0ac9b5e) )
	ROM_LOAD16_BYTE( "b67-14.61",   0x40000, 0x20000, CRC(7f3dd724) SHA1(2f2eeae0ee31e20082237b9a947c6848771eb73c) )
	ROM_LOAD16_BYTE( "b67-15.62",   0x40001, 0x20000, CRC(e84f62d0) SHA1(3b4a55a14dee7d592467fde9a75bde64deabd27d) )

	ROM_REGION( 0x1c000, "audiocpu", 0 )	/* sound cpu */
	ROM_LOAD( "b67-13.40",   0x00000, 0x04000, CRC(2936b4b1) SHA1(39b41643464dd89e456ab6eb15a0ff0aef30afde) )
	ROM_CONTINUE(            0x10000, 0x0c000 ) /* banked stuff */

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "b67-06.66", 0x00000, 0x80000, CRC(b9a38d64) SHA1(7ae8165b444d9da6ccdbc4a769535bcbb6738aaa) )		/* SCN */

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD32_BYTE( "b67-01", 0x00000, 0x80000, CRC(81ad9acb) SHA1(d9ad3f6332c6ca6b9872da57526a8158a3cf5b2f) )	/* OBJ: each rom has 1 bitplane, forming 16x8 tiles */
	ROM_LOAD32_BYTE( "b67-02", 0x00001, 0x80000, CRC(c20cd2fb) SHA1(b015e1fe167e19826aa451b45cd143d66a6db83c) )
	ROM_LOAD32_BYTE( "b67-03", 0x00002, 0x80000, CRC(bc9019ed) SHA1(7eddc83d71be97ce6637e6b35c226d58e6c39c3f) )
	ROM_LOAD32_BYTE( "b67-04", 0x00003, 0x80000, CRC(2af4c8af) SHA1(b2ae7aad0c59ffc368811f4bd5546dbb6860f9a9) )

	ROM_REGION16_LE( 0x80000, "user1", 0 )
	ROM_LOAD16_WORD( "b67-05.43", 0x00000, 0x80000, CRC(9593e42b) SHA1(54b5538c302a1734ff4b752ab87a8c45d5c6b23d) )	/* index used to create 64x64 sprites on the fly */

	ROM_REGION( 0x80000, "ymsnd", 0 )	/* ADPCM samples */
	ROM_LOAD( "b67-08", 0x00000, 0x80000, CRC(458f41fb) SHA1(acca7c95acd1ae7a1cc51fb7fe644ad6d00ff5ac) )

	ROM_REGION( 0x80000, "ymsnd.deltat", 0 )	/* Delta-T samples */
	ROM_LOAD( "b67-07", 0x00000, 0x80000, CRC(4f834357) SHA1(f34705ce64870a8b24ec2639505079cc031fb719) )

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "plhs18p8b-b67-09.ic15", 0x0000, 0x0149, CRC(62035487) SHA1(5d9538ea9eabff324d274772b1e1fc9a9aec9100) )
	ROM_LOAD( "pal16l8a-b67-11.ic36",  0x0200, 0x0104, CRC(3177fb06) SHA1(c128277fe03342d9ec8da3c6e08a404a3f010547) )
	ROM_LOAD( "pal20l8b-b67-12.ic37",  0x0400, 0x0144, CRC(a47c2798) SHA1(8c963efd416b3f6586cb12afb9417dc95c2bc574) )
	ROM_LOAD( "pal20l8b-b67-10.ic33",  0x0600, 0x0144, CRC(4ced09c7) SHA1(519e6152cc5e4cb3ec24c4dc09101dddf22988aa) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-othunder.bin", 0x0000, 0x0080, CRC(3729b844) SHA1(f6bb41d293d1e47214f8b2d147991404f3278ebf) )
ROM_END

ROM_START( othunderu )
	ROM_REGION( 0x80000, "maincpu", 0 )	/* 512K for 68000 code */
	ROM_LOAD16_BYTE( "b67-20-1.63", 0x00000, 0x20000, CRC(851a453b) SHA1(48b8c379e78cd79463f1e24dc23816a97cf819b8) )
	ROM_LOAD16_BYTE( "b67-22-1.64", 0x00001, 0x20000, CRC(19480dc0) SHA1(8bbc982c89f0878e7639330970df5aa93ecbb083) )
	ROM_LOAD16_BYTE( "b67-14.61",   0x40000, 0x20000, CRC(7f3dd724) SHA1(2f2eeae0ee31e20082237b9a947c6848771eb73c) )
	ROM_LOAD16_BYTE( "b67-15.62",   0x40001, 0x20000, CRC(e84f62d0) SHA1(3b4a55a14dee7d592467fde9a75bde64deabd27d) )

	ROM_REGION( 0x1c000, "audiocpu", 0 )	/* sound cpu */
	ROM_LOAD( "b67-13.40",   0x00000, 0x04000, CRC(2936b4b1) SHA1(39b41643464dd89e456ab6eb15a0ff0aef30afde) )
	ROM_CONTINUE(            0x10000, 0x0c000 ) /* banked stuff */

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "b67-06.66", 0x00000, 0x80000, CRC(b9a38d64) SHA1(7ae8165b444d9da6ccdbc4a769535bcbb6738aaa) )		/* SCN */

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD32_BYTE( "b67-01", 0x00000, 0x80000, CRC(81ad9acb) SHA1(d9ad3f6332c6ca6b9872da57526a8158a3cf5b2f) )	/* OBJ: each rom has 1 bitplane, forming 16x8 tiles */
	ROM_LOAD32_BYTE( "b67-02", 0x00001, 0x80000, CRC(c20cd2fb) SHA1(b015e1fe167e19826aa451b45cd143d66a6db83c) )
	ROM_LOAD32_BYTE( "b67-03", 0x00002, 0x80000, CRC(bc9019ed) SHA1(7eddc83d71be97ce6637e6b35c226d58e6c39c3f) )
	ROM_LOAD32_BYTE( "b67-04", 0x00003, 0x80000, CRC(2af4c8af) SHA1(b2ae7aad0c59ffc368811f4bd5546dbb6860f9a9) )

	ROM_REGION16_LE( 0x80000, "user1", 0 )
	ROM_LOAD16_WORD( "b67-05.43", 0x00000, 0x80000, CRC(9593e42b) SHA1(54b5538c302a1734ff4b752ab87a8c45d5c6b23d) )	/* index used to create 64x64 sprites on the fly */

	ROM_REGION( 0x80000, "ymsnd", 0 )	/* ADPCM samples */
	ROM_LOAD( "b67-08", 0x00000, 0x80000, CRC(458f41fb) SHA1(acca7c95acd1ae7a1cc51fb7fe644ad6d00ff5ac) )

	ROM_REGION( 0x80000, "ymsnd.deltat", 0 )	/* Delta-T samples */
	ROM_LOAD( "b67-07", 0x00000, 0x80000, CRC(4f834357) SHA1(f34705ce64870a8b24ec2639505079cc031fb719) )

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "plhs18p8b-b67-09.ic15", 0x0000, 0x0149, CRC(62035487) SHA1(5d9538ea9eabff324d274772b1e1fc9a9aec9100) )
	ROM_LOAD( "pal16l8a-b67-11.ic36",  0x0200, 0x0104, CRC(3177fb06) SHA1(c128277fe03342d9ec8da3c6e08a404a3f010547) )
	ROM_LOAD( "pal20l8b-b67-12.ic37",  0x0400, 0x0144, CRC(a47c2798) SHA1(8c963efd416b3f6586cb12afb9417dc95c2bc574) )
	ROM_LOAD( "pal20l8b-b67-10.ic33",  0x0600, 0x0144, CRC(4ced09c7) SHA1(519e6152cc5e4cb3ec24c4dc09101dddf22988aa) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-othunder.bin", 0x0000, 0x0080, CRC(3729b844) SHA1(f6bb41d293d1e47214f8b2d147991404f3278ebf) )
ROM_END

ROM_START( othunderuo )
	ROM_REGION( 0x80000, "maincpu", 0 )	/* 512K for 68000 code */
	ROM_LOAD16_BYTE( "b67-20.63",   0x00000, 0x20000, CRC(21439ea2) SHA1(d5b5a194e9698cf43513c0d56146772e8132ab07) )
	ROM_LOAD16_BYTE( "b67-22.64",   0x00001, 0x20000, CRC(0f99ad3c) SHA1(dd6c9e822470ca867ec01e642443a871e879bae5) )
	ROM_LOAD16_BYTE( "b67-14.61",   0x40000, 0x20000, CRC(7f3dd724) SHA1(2f2eeae0ee31e20082237b9a947c6848771eb73c) )
	ROM_LOAD16_BYTE( "b67-15.62",   0x40001, 0x20000, CRC(e84f62d0) SHA1(3b4a55a14dee7d592467fde9a75bde64deabd27d) )

	ROM_REGION( 0x1c000, "audiocpu", 0 )	/* sound cpu */
	ROM_LOAD( "b67-13.40",   0x00000, 0x04000, CRC(2936b4b1) SHA1(39b41643464dd89e456ab6eb15a0ff0aef30afde) )
	ROM_CONTINUE(            0x10000, 0x0c000 ) /* banked stuff */

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "b67-06.66", 0x00000, 0x80000, CRC(b9a38d64) SHA1(7ae8165b444d9da6ccdbc4a769535bcbb6738aaa) )		/* SCN */

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD32_BYTE( "b67-01", 0x00000, 0x80000, CRC(81ad9acb) SHA1(d9ad3f6332c6ca6b9872da57526a8158a3cf5b2f) )	/* OBJ: each rom has 1 bitplane, forming 16x8 tiles */
	ROM_LOAD32_BYTE( "b67-02", 0x00001, 0x80000, CRC(c20cd2fb) SHA1(b015e1fe167e19826aa451b45cd143d66a6db83c) )
	ROM_LOAD32_BYTE( "b67-03", 0x00002, 0x80000, CRC(bc9019ed) SHA1(7eddc83d71be97ce6637e6b35c226d58e6c39c3f) )
	ROM_LOAD32_BYTE( "b67-04", 0x00003, 0x80000, CRC(2af4c8af) SHA1(b2ae7aad0c59ffc368811f4bd5546dbb6860f9a9) )

	ROM_REGION16_LE( 0x80000, "user1", 0 )
	ROM_LOAD16_WORD( "b67-05.43", 0x00000, 0x80000, CRC(9593e42b) SHA1(54b5538c302a1734ff4b752ab87a8c45d5c6b23d) )	/* index used to create 64x64 sprites on the fly */

	ROM_REGION( 0x80000, "ymsnd", 0 )	/* ADPCM samples */
	ROM_LOAD( "b67-08", 0x00000, 0x80000, CRC(458f41fb) SHA1(acca7c95acd1ae7a1cc51fb7fe644ad6d00ff5ac) )

	ROM_REGION( 0x80000, "ymsnd.deltat", 0 )	/* Delta-T samples */
	ROM_LOAD( "b67-07", 0x00000, 0x80000, CRC(4f834357) SHA1(f34705ce64870a8b24ec2639505079cc031fb719) )

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "plhs18p8b-b67-09.ic15", 0x0000, 0x0149, CRC(62035487) SHA1(5d9538ea9eabff324d274772b1e1fc9a9aec9100) )
	ROM_LOAD( "pal16l8a-b67-11.ic36",  0x0200, 0x0104, CRC(3177fb06) SHA1(c128277fe03342d9ec8da3c6e08a404a3f010547) )
	ROM_LOAD( "pal20l8b-b67-12.ic37",  0x0400, 0x0144, CRC(a47c2798) SHA1(8c963efd416b3f6586cb12afb9417dc95c2bc574) )
	ROM_LOAD( "pal20l8b-b67-10.ic33",  0x0600, 0x0144, CRC(4ced09c7) SHA1(519e6152cc5e4cb3ec24c4dc09101dddf22988aa) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-othunder.bin", 0x0000, 0x0080, CRC(3729b844) SHA1(f6bb41d293d1e47214f8b2d147991404f3278ebf) )
ROM_END

ROM_START( othunderj )
	ROM_REGION( 0x80000, "maincpu", 0 )	/* 512K for 68000 code */
	ROM_LOAD16_BYTE( "b67-20.63",   0x00000, 0x20000, CRC(21439ea2) SHA1(d5b5a194e9698cf43513c0d56146772e8132ab07) )
	ROM_LOAD16_BYTE( "b67-21.64",   0x00001, 0x20000, CRC(9690fc86) SHA1(4e695554fc9cc91c5f8cff95dc290333bb56d571) )
	ROM_LOAD16_BYTE( "b67-14.61",   0x40000, 0x20000, CRC(7f3dd724) SHA1(2f2eeae0ee31e20082237b9a947c6848771eb73c) )
	ROM_LOAD16_BYTE( "b67-15.62",   0x40001, 0x20000, CRC(e84f62d0) SHA1(3b4a55a14dee7d592467fde9a75bde64deabd27d) )

	ROM_REGION( 0x1c000, "audiocpu", 0 )	/* sound cpu */
	ROM_LOAD( "b67-13.40",   0x00000, 0x04000, CRC(2936b4b1) SHA1(39b41643464dd89e456ab6eb15a0ff0aef30afde) )
	ROM_CONTINUE(            0x10000, 0x0c000 ) /* banked stuff */

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "b67-06.66", 0x00000, 0x80000, CRC(b9a38d64) SHA1(7ae8165b444d9da6ccdbc4a769535bcbb6738aaa) )		/* SCN */

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD32_BYTE( "b67-01", 0x00000, 0x80000, CRC(81ad9acb) SHA1(d9ad3f6332c6ca6b9872da57526a8158a3cf5b2f) )	/* OBJ: each rom has 1 bitplane, forming 16x8 tiles */
	ROM_LOAD32_BYTE( "b67-02", 0x00001, 0x80000, CRC(c20cd2fb) SHA1(b015e1fe167e19826aa451b45cd143d66a6db83c) )
	ROM_LOAD32_BYTE( "b67-03", 0x00002, 0x80000, CRC(bc9019ed) SHA1(7eddc83d71be97ce6637e6b35c226d58e6c39c3f) )
	ROM_LOAD32_BYTE( "b67-04", 0x00003, 0x80000, CRC(2af4c8af) SHA1(b2ae7aad0c59ffc368811f4bd5546dbb6860f9a9) )

	ROM_REGION16_LE( 0x80000, "user1", 0 )
	ROM_LOAD16_WORD( "b67-05.43", 0x00000, 0x80000, CRC(9593e42b) SHA1(54b5538c302a1734ff4b752ab87a8c45d5c6b23d) )	/* index used to create 64x64 sprites on the fly */

	ROM_REGION( 0x80000, "ymsnd", 0 )	/* ADPCM samples */
	ROM_LOAD( "b67-08", 0x00000, 0x80000, CRC(458f41fb) SHA1(acca7c95acd1ae7a1cc51fb7fe644ad6d00ff5ac) )

	ROM_REGION( 0x80000, "ymsnd.deltat", 0 )	/* Delta-T samples */
	ROM_LOAD( "b67-07", 0x00000, 0x80000, CRC(4f834357) SHA1(f34705ce64870a8b24ec2639505079cc031fb719) )

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "plhs18p8b-b67-09.ic15", 0x0000, 0x0149, CRC(62035487) SHA1(5d9538ea9eabff324d274772b1e1fc9a9aec9100) )
	ROM_LOAD( "pal16l8a-b67-11.ic36",  0x0200, 0x0104, CRC(3177fb06) SHA1(c128277fe03342d9ec8da3c6e08a404a3f010547) )
	ROM_LOAD( "pal20l8b-b67-12.ic37",  0x0400, 0x0144, CRC(a47c2798) SHA1(8c963efd416b3f6586cb12afb9417dc95c2bc574) )
	ROM_LOAD( "pal20l8b-b67-10.ic33",  0x0600, 0x0144, CRC(4ced09c7) SHA1(519e6152cc5e4cb3ec24c4dc09101dddf22988aa) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-othunder.bin", 0x0000, 0x0080, CRC(3729b844) SHA1(f6bb41d293d1e47214f8b2d147991404f3278ebf) )
ROM_END



GAME( 1988, othunder,   0,        othunder, othunder, 0, ORIENTATION_FLIP_X, "Taito Corporation Japan", "Operation Thunderbolt (World)", GAME_SUPPORTS_SAVE )
GAME( 1988, othunderu,  othunder, othunder, othundu,  0, ORIENTATION_FLIP_X, "Taito America Corporation", "Operation Thunderbolt (US)", GAME_SUPPORTS_SAVE )
GAME( 1988, othunderuo, othunder, othunder, othundu,  0, ORIENTATION_FLIP_X, "Taito America Corporation", "Operation Thunderbolt (US, older)", GAME_SUPPORTS_SAVE )
GAME( 1988, othunderj,  othunder, othunder, othundrj, 0, ORIENTATION_FLIP_X, "Taito Corporation", "Operation Thunderbolt (Japan)", GAME_SUPPORTS_SAVE )
