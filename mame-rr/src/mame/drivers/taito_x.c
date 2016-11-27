/***************************************************************************


Taito X-system

driver by Richard Bush, Howie Cohen and Yochizo

25th Nov 2003
video merged with video/seta.c


Supported games:
----------------------------------------------------
 Name                    Company               Year
  Superman                Taito Corp.           1988
  Twin Hawk (World)       Taito Corp. Japan     1988
  Twin Hawk (US)          Taito America Corp.   1988
  Daisenpu (Japan)        Taito Corp.           1988
  Gigandes                East Technology Corp. 1989
  Last Striker            East Technology Corp. 1989
  Balloon Brothers        East Technology Corp. 199?

Please tell me the games worked on this board.


Memory map:
----------------------------------------------------
  0x000000 - 0x07ffff : ROM
  0x300000   ??
  0x400000   ??
  0x500000 - 0x50000f : Dipswitches a & b, 4 bits to each word
  0x600000   ?? 0, 10, 0x4001, 0x4006
  0x700000   ??
  0x800000 - 0x800003 : sound chip
  0x900000 - 0x900fff : c-chip shared RAM space
  0xb00000 - 0xb00fff : palette RAM, words in the format xRRRRRGGGGGBBBBB
  0xc00000   ??
  0xd00000 - 0xd007ff : video attribute RAM
      0000 - 03ff : sprite y coordinate
      0400 - 07ff : tile x & y scroll
  0xe00000 - 0xe00fff : object RAM bank 1
      0000 - 03ff : sprite number (bit mask 0x3fff)
                    sprite y flip (bit mask 0x4000)
                    sprite x flip (bit mask 0x8000)
      0400 - 07ff : sprite x coordinate (bit mask 0x1ff)
                    sprite color (bit mask 0xf800)
      0800 - 0bff : tile number (bit mask 0x3fff)
                    tile y flip (bit mask 0x4000)
                    tile x flip (bit mask 0x8000)
      0c00 - 0fff : tile color (bit mask 0xf800)
  0xe01000 - 0xe01fff : unused(?) portion of object RAM
  0xe02000 - 0xe02fff : object RAM bank 2
  0xe03000 - 0xe03fff : unused(?) portion of object RAM

Interrupt:
----------------------------------------------------
  IRQ level 6 : Superman
  IRQ level 2 : Daisenpu, Balloon Brothers, Gigandes

Screen resolution:
----------------------------------------------------
  384 x 240   : Superman, Balloon Brothers, Gigandes
  384 x 224   : Daisenpu

Sound chip:
----------------------------------------------------
  YM2610 x 1   : Superman, Balloon Brothers, Gigandes
  YM2151 x 1   : Daisenpu


Gigandes
--------

  - Gigandes has background tile glitches on the demo of cave
    stage (the last one before it does demo of level 1 again).
    It seems to be rapidly switching between two different
    background layers. This may be because the game has put
    different tiles in bank 0 and bank 1 (which alternate each
    frame). The other strangeness is that the background is not
    scrolling. So chances are the graphics chip emulation is
    flawed.

    When this cuts to hiscore screen there is flickering white
    tilemap garbage beneath. These leftover tiles have not been
    cleared from $e00800-bff (but they have been from $e02800).
    So alternate 2 frames display the bank with garbage tiles and
    the one without.

    Probably some control register should be keeping the graphics
    chip in the cleared bank, so the garbage is never visible.
    Separate bank control for sprite / tile layers ?
    Maybe this effect is also desired during the cave stage.


TODO
----

  - Fix known problems
  - Any other games that worked on this board?
  - What is correct date for Ballbros? Two different ones
    appear in this driver!


Dumpers Notes
-------------

Details of custom chip numbers on Superman would be welcome.

Daisenpu
--------

(c)1989 Toaplan/Taito
Taito X System

K1100443A MAIN PCB
J1100188A X SYSTEM
P0-051A

CPU  : 68000 (Toshiba TMP68000N-8)
Sound: Z80 (Sharp LH0080A)
       YM2151+YM3012
OSC  : 16.000MHz

B87-01 (Mask ROMs, read as 27C4200)
B87-02
B87-03
B87-04
(These 4 images could be wrong)

B87-05 (M5M27C101K) - Main PRG
B87-06 (M5M27C101K) - Main PRG
B87-07 (27C256) - Sound PRG

This board uses SETA's custom chips.
X1-001A, X1-002A, X1-004, X1-006, X1-007.
Arkanoid II uses X1-001, X1-002, X1-003, X1-004.

Control: 8-way Joystick + 2-buttons


Gigandes
--------

East Technology 1989

P0-057A

           68000-8    1  2  51832  3  4  51832

6264                        16MHz
5
10                                       8
11       TC0140SYT     X1-001A X1-002A   7
                                         9
YM2610                                   6
        Z80A
                     X1-004          X1-006
                                     X1-007


Balloon Brothers
----------------

East Technology 1992

    68000-8    10A   51832           5A 51832

 6264                  2063
 8D                    2063                 SWB  SWA
 EAST-10                    16MHz
 EAST-11    Taito TC0140SYT
 YM2610                                             3
          Z80A         SETA X1-001A SETA X1-002A

                 SETA X1-004                        2
                                                    1
                                                    0


Kyuukyoku no Striker
East Technology/Taito, 1989

This game runs on Seta hardware and also using one Taito custom chip.


PCB Layout
----------

PO-057A
|------------------------------- |
|  YM2610    IC.18D  6264        |
|YM3016                          |
|    Z80  TCO140SYT        68000 |
| X1-004  X1-001A          PE.9A |
|J         X1-002A  6264   62256 |
|A                  6264         |
|M           16MHz         PO.4A |
|M X1-007           DSW1   62256 |
|A X1-006           DSW2         |
|              M-8-3             |
|M-8-5  M-8-4  M-8-2             |
|              M-8-1             |
|              M-8-0             |
|--------------------------------|

Notes:
        All M-8-x ROMs are held on a plug-in sub-board.
        The sub-board has printed on it "East Technology" and has PCB Number PO-046A

         68000 clock: 8.000MHz
           Z80 clock: 4.000MHz
        YM2610 clock: 8.000MHz
               Vsync: 58Hz
               HSync: 15.22kHz


C-Chip notes
------------

Superman seems to be the only game of the four with a c-chip. Daisenpu
appears to use a simple input device with coin counter and lockout in
its place. The East Technology games on this hardware follow Daisenpu.


Stephh's notes (based on the game M68000 code and some tests) :

1) 'superman' and 'supermanj'

  - Region stored at 0x07fffe.w
  - Sets :
      * 'superman' : region = 0x0002
      * 'supermanj' : region = 0x0000
  - These 2 games are 100% the same, only region differs !
  - Coinage relies on the region (code at 0x003b8a) :
      * 0x0000 (Japan) uses TAITO_COINAGE_JAPAN_OLD
      * 0x0001 (US) uses TAITO_COINAGE_US
      * 0x0002 (World) uses TAITO_COINAGE_WORLD
  - Notice screen only if region = 0x0001
  - I can't tell if it's an ingame bug or an emulation bug,
    but boss are far much harder when "Difficulty" Dip Swicth
    is set to "Easy" : put a watch on 0xf00a76.w for level 1 boss
    and you'll notice that MSB is set to 0x01 instead of 0x00


2) 'twinhawk'

  - No region
  - Hard coded coinage table at 0x00b01c and is the same as TAITO_COINAGE_WORLD
  - 2 simultaneous players game (player 1 is blue and player 2 is red)
  - BUTTON3 is only tested in "Test Mode", not when you are playing


3) 'twinhawku'

  - No region
  - Hard coded coinage table at 0x00b02a and is different from TAITO_COINAGE_US :
      * 2 coins slots with their own coinage
      * possible coinage settings : 4C_1C / 3C_1C / 2C_1C / 1C_1C
  - Same other notes as for 'twinhawk'


4) 'daisenpu'

  - No region
  - Hard coded coinage table at 0x00a44a and is the same as TAITO_COINAGE_JAPAN_OLD
  - Differences with 'twinhawk' :
      * notice screen
      * 2 alternate players game (players 1 and 2 are green)
      * additional "Cabinet" Dip Switch
      * different "Bonus Life" settings
  - Same other notes as for 'twinhawk'


5) 'gigandes' and 'gigandesj'

  - No region (not a Taito game anyway)
  - No notice screen
  - Constant bonus life at 50k, 250k then every 200k
  - Bogus "Test Mode" in 'gigandes' (while it is correct for 'gigandesj') :
      * screen is flipped while it shouldn't and it isn't flipped while it should
      * displays cabinet type instead of number of controls
  - DSWA bit 4 effect remains unknown but it MUST remain OFF !
    All I know is that this bit is tested in multiple places in the code
    and that it hangs the game when you are hit (music continues playing though)
  - DSWA bit 7 (called "Free Play" in "Test Mode") gives you infinite lives
    but you will still need to insert coins as a standard game.
    In a 2 players game, when this Dip Switch is ON, player doesn't change when it,
    so player 2 will have to wait that player 1 has ended the game to start.


6) 'kyustrkr'

  - No region (not a Taito game anyway)
  - No notice screen
  - DSWA bit 7 (called "Free Play" in "Test Mode") has the following effects :
      * in a game versus CPU, player(s) will always win regardless of the score
      * in a 1UP versus 2UP game, both players will play 6 matches regardless of the score
  - "Language" Dip Switch also affects game title


7) 'ballbros'

  - No region (not a Taito game anyway)
  - No notice screen
  - Do NOT trust the test mode which has nothing to do with the game !
  - "Difficulty" settings need to be confirmed as I'm not good enough
    at playing this game to see any visible difference :(
    They are based on the ones for other East Technology games though.


***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "includes/taitoipt.h"
#include "audio/taitosnd.h"
#include "includes/seta.h"
#include "sound/2610intf.h"
#include "sound/2151intf.h"
#include "includes/cchip.h"



static READ16_HANDLER( superman_dsw_input_r )
{
	switch (offset)
	{
		case 0x00:
			return  input_port_read(space->machine, "DSWA") & 0x0f;
		case 0x01:
			return (input_port_read(space->machine, "DSWA") & 0xf0) >> 4;
		case 0x02:
			return  input_port_read(space->machine, "DSWB") & 0x0f;
		case 0x03:
			return (input_port_read(space->machine, "DSWB") & 0xf0) >> 4;
		default:
			logerror("taitox unknown dsw read offset: %04x\n", offset);
			return 0x00;
	}
}

static READ16_HANDLER( daisenpu_input_r )
{
	switch (offset)
	{
		case 0x00:
			return input_port_read(space->machine, "IN0");    /* Player 1 controls + START1 */
		case 0x01:
			return input_port_read(space->machine, "IN1");    /* Player 2 controls + START2 */
		case 0x02:
			return input_port_read(space->machine, "IN2");    /* COINn + SERVICE1 + TILT */

		default:
			logerror("taitox unknown input read offset: %04x\n", offset);
			return 0x00;
	}
}

static WRITE16_HANDLER( daisenpu_input_w )
{
	switch (offset)
	{
		case 0x04:	/* coin counters and lockout */
			coin_counter_w(space->machine, 0,data & 0x01);
			coin_counter_w(space->machine, 1,data & 0x02);
			coin_lockout_w(space->machine, 0,~data & 0x04);
			coin_lockout_w(space->machine, 1,~data & 0x08);
//logerror("taitox coin control %04x to offset %04x\n",data,offset);
			break;

		default:
			logerror("taitox unknown input write %04x to offset %04x\n",data,offset);
	}
}


static WRITE16_HANDLER( kyustrkr_input_w )
{
	switch (offset)
	{
		case 0x04:	/* coin counters and lockout */
			coin_counter_w(space->machine, 0,data & 0x01);
			coin_counter_w(space->machine, 1,data & 0x02);
			coin_lockout_w(space->machine, 0,data & 0x04);
			coin_lockout_w(space->machine, 1,data & 0x08);
//logerror("taitox coin control %04x to offset %04x\n",data,offset);
			break;

		default:
			logerror("taitox unknown input write %04x to offset %04x\n",data,offset);
	}
}


/**************************************************************************/

static INT32 banknum;

static void reset_sound_region(running_machine *machine)
{
	memory_set_bankptr(machine,  "bank2", memory_region(machine, "audiocpu") + (banknum * 0x4000) + 0x10000 );
}

static WRITE8_HANDLER( sound_bankswitch_w )
{
	banknum = (data - 1) & 3;
	reset_sound_region(space->machine);
}


/**************************************************************************/

static ADDRESS_MAP_START( superman_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x300000, 0x300001) AM_WRITENOP	/* written each frame at $3a9c, mostly 0x10 */
	AM_RANGE(0x400000, 0x400001) AM_WRITENOP	/* written each frame at $3aa2, mostly 0x10 */
	AM_RANGE(0x500000, 0x500007) AM_READ(superman_dsw_input_r)
	AM_RANGE(0x600000, 0x600001) AM_WRITENOP	/* written each frame at $3ab0, mostly 0x10 */
	AM_RANGE(0x800000, 0x800001) AM_READNOP AM_DEVWRITE8("tc0140syt", tc0140syt_port_w, 0x00ff)
	AM_RANGE(0x800002, 0x800003) AM_DEVREADWRITE8("tc0140syt", tc0140syt_comm_r, tc0140syt_comm_w, 0x00ff)
	AM_RANGE(0x900000, 0x9007ff) AM_READWRITE(cchip1_ram_r, cchip1_ram_w)
	AM_RANGE(0x900802, 0x900803) AM_READWRITE(cchip1_ctrl_r, cchip1_ctrl_w)
	AM_RANGE(0x900c00, 0x900c01) AM_WRITE(cchip1_bank_w)
	AM_RANGE(0xb00000, 0xb00fff) AM_RAM_WRITE(paletteram16_xRRRRRGGGGGBBBBB_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0xd00000, 0xd007ff) AM_RAM AM_BASE_GENERIC(spriteram	)	// Sprites Y
	AM_RANGE(0xe00000, 0xe03fff) AM_RAM AM_BASE_GENERIC(spriteram2	)	// Sprites Code + X + Attr
	AM_RANGE(0xf00000, 0xf03fff) AM_RAM			/* Main RAM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( daisenpu_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
//  AM_RANGE(0x400000, 0x400001) AM_WRITENOP    /* written each frame at $2ac, values change */
	AM_RANGE(0x500000, 0x50000f) AM_READ(superman_dsw_input_r)
//  AM_RANGE(0x600000, 0x600001) AM_WRITENOP    /* written each frame at $2a2, values change */
	AM_RANGE(0x800000, 0x800001) AM_READNOP AM_DEVWRITE8("tc0140syt", tc0140syt_port_w, 0x00ff)
	AM_RANGE(0x800002, 0x800003) AM_DEVREADWRITE8("tc0140syt", tc0140syt_comm_r, tc0140syt_comm_w, 0x00ff)
	AM_RANGE(0x900000, 0x90000f) AM_READWRITE(daisenpu_input_r, daisenpu_input_w)
	AM_RANGE(0xb00000, 0xb00fff) AM_RAM_WRITE(paletteram16_xRRRRRGGGGGBBBBB_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0xd00000, 0xd007ff) AM_RAM AM_BASE_GENERIC(spriteram	)	// Sprites Y
	AM_RANGE(0xe00000, 0xe03fff) AM_RAM AM_BASE_GENERIC(spriteram2	)	// Sprites Code + X + Attr
	AM_RANGE(0xf00000, 0xf03fff) AM_RAM			/* Main RAM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( gigandes_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x400000, 0x400001) AM_WRITENOP	/* 0x1 written each frame at $d42, watchdog? */
	AM_RANGE(0x500000, 0x500007) AM_READ(superman_dsw_input_r)
	AM_RANGE(0x600000, 0x600001) AM_WRITENOP	/* 0x1 written each frame at $d3c, watchdog? */
	AM_RANGE(0x800000, 0x800001) AM_READNOP AM_DEVWRITE8("tc0140syt", tc0140syt_port_w, 0x00ff)
	AM_RANGE(0x800002, 0x800003) AM_DEVREADWRITE8("tc0140syt", tc0140syt_comm_r, tc0140syt_comm_w, 0x00ff)
	AM_RANGE(0x900000, 0x90000f) AM_READWRITE(daisenpu_input_r, daisenpu_input_w)
	AM_RANGE(0xb00000, 0xb00fff) AM_RAM_WRITE(paletteram16_xRRRRRGGGGGBBBBB_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0xd00000, 0xd007ff) AM_RAM AM_BASE_GENERIC(spriteram)	// Sprites Y
	AM_RANGE(0xe00000, 0xe03fff) AM_RAM AM_BASE_GENERIC(spriteram2	)	// Sprites Code + X + Attr
	AM_RANGE(0xf00000, 0xf03fff) AM_RAM			/* Main RAM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( ballbros_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x400000, 0x400001) AM_WRITENOP	/* 0x1 written each frame at $c56, watchdog? */
	AM_RANGE(0x500000, 0x50000f) AM_READ(superman_dsw_input_r)
	AM_RANGE(0x600000, 0x600001) AM_WRITENOP	/* 0x1 written each frame at $c4e, watchdog? */
	AM_RANGE(0x800000, 0x800001) AM_READNOP AM_DEVWRITE8("tc0140syt", tc0140syt_port_w, 0x00ff)
	AM_RANGE(0x800002, 0x800003) AM_DEVREADWRITE8("tc0140syt", tc0140syt_comm_r, tc0140syt_comm_w, 0x00ff)
	AM_RANGE(0x900000, 0x90000f) AM_READWRITE(daisenpu_input_r, daisenpu_input_w)
	AM_RANGE(0xb00000, 0xb00fff) AM_RAM_WRITE(paletteram16_xRRRRRGGGGGBBBBB_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0xd00000, 0xd007ff) AM_RAM AM_BASE_GENERIC(spriteram	)	// Sprites Y
	AM_RANGE(0xe00000, 0xe03fff) AM_RAM AM_BASE_GENERIC(spriteram2	)	// Sprites Code + X + Attr
	AM_RANGE(0xf00000, 0xf03fff) AM_RAM			/* Main RAM */
ADDRESS_MAP_END


/**************************************************************************/

static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank2")
	AM_RANGE(0xc000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xe003) AM_DEVREADWRITE("ymsnd", ym2610_r, ym2610_w)
	AM_RANGE(0xe200, 0xe200) AM_READNOP AM_DEVWRITE("tc0140syt", tc0140syt_slave_port_w)
	AM_RANGE(0xe201, 0xe201) AM_DEVREADWRITE("tc0140syt", tc0140syt_slave_comm_r, tc0140syt_slave_comm_w)
	AM_RANGE(0xe400, 0xe403) AM_WRITENOP /* pan */
	AM_RANGE(0xea00, 0xea00) AM_READNOP
	AM_RANGE(0xee00, 0xee00) AM_WRITENOP /* ? */
	AM_RANGE(0xf000, 0xf000) AM_WRITENOP /* ? */
	AM_RANGE(0xf200, 0xf200) AM_WRITE(sound_bankswitch_w) /* bankswitch ? */
ADDRESS_MAP_END

static ADDRESS_MAP_START( daisenpu_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank2")
	AM_RANGE(0xc000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xe001) AM_DEVREADWRITE("ymsnd", ym2151_r, ym2151_w)
	AM_RANGE(0xe200, 0xe200) AM_READNOP AM_DEVWRITE("tc0140syt", tc0140syt_slave_port_w)
	AM_RANGE(0xe201, 0xe201) AM_DEVREADWRITE("tc0140syt", tc0140syt_slave_comm_r, tc0140syt_slave_comm_w)
	AM_RANGE(0xe400, 0xe403) AM_WRITENOP /* pan */
	AM_RANGE(0xea00, 0xea00) AM_READNOP
	AM_RANGE(0xee00, 0xee00) AM_WRITENOP /* ? */
	AM_RANGE(0xf000, 0xf000) AM_WRITENOP
	AM_RANGE(0xf200, 0xf200) AM_WRITE(sound_bankswitch_w)
ADDRESS_MAP_END


/**************************************************************************/

static INPUT_PORTS_START( taitox_generic )
	/* The Dip Switches will be filled for each game */
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
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

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
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

	PORT_START("IN0")
	TAITO_JOY_UDLR_3_BUTTONS_START( 1 )

	PORT_START("IN1")
	TAITO_JOY_UDLR_3_BUTTONS_START( 2 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( superman )
	PORT_INCLUDE( taitox_generic )

	/* 0x500000 (low) and 0x500002 (high) -> 0xf01c4a ($1c4a,A5) */
	PORT_MODIFY("DSWA")
	TAITO_MACHINE_NO_COCKTAIL_LOC(SWA)
	TAITO_COINAGE_WORLD_LOC(SWA)

	/* 0x500004 (low) and 0x500006 (high) -> 0xf01c4c ($1c4c,A5) */
	PORT_MODIFY("DSWB")
	TAITO_DIFFICULTY_LOC(SWB)
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "SWB:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SWB:4" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) ) PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SWB:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "SWB:8" )

	PORT_MODIFY("IN0")
	TAITO_JOY_UDLR_2_BUTTONS_START( 1 )

	PORT_MODIFY("IN1")
	TAITO_JOY_UDLR_2_BUTTONS_START( 2 )
INPUT_PORTS_END

static INPUT_PORTS_START( supermanj )
	PORT_INCLUDE( superman )

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_JAPAN_OLD_LOC(SWA)
INPUT_PORTS_END

static INPUT_PORTS_START( twinhawk )
	PORT_INCLUDE( taitox_generic )

	/* 0x500000 (low) and 0x500002 (high) -> 0xf001b8 */
	PORT_MODIFY("DSWA")
	TAITO_MACHINE_NO_COCKTAIL_LOC(SWA)
	TAITO_COINAGE_WORLD_LOC(SWA)

	/* 0x500004 (low) and 0x500006 (high) -> 0xf001ba */
	PORT_MODIFY("DSWB")
	TAITO_DIFFICULTY_LOC(SWB)
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x0c, "50k and every 150k" )
	PORT_DIPSETTING(    0x08, "70k and every 200k" )
	PORT_DIPSETTING(    0x04, "50k only" )
	PORT_DIPSETTING(    0x00, "100k only" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) ) PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SWB:7" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_MODIFY("IN0")
	TAITO_JOY_UDLR_2_BUTTONS_START( 1 )

	PORT_MODIFY("IN1")
	TAITO_JOY_UDLR_2_BUTTONS_START( 2 )
INPUT_PORTS_END

static INPUT_PORTS_START( twinhawku )
	PORT_INCLUDE( twinhawk )

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SWA:7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
INPUT_PORTS_END

static INPUT_PORTS_START( daisenpu )
	PORT_INCLUDE( twinhawk )

	/* 0x500000 (low) and 0x500002 (high) -> 0xf001a4 */
	PORT_MODIFY("DSWA")
	TAITO_MACHINE_COCKTAIL_LOC(SWA)
	TAITO_COINAGE_JAPAN_OLD_LOC(SWA)

	/* 0x500004 (low) and 0x500006 (high) -> 0xf001a6 */
	PORT_MODIFY("DSWB")
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x08, "50k and every 150k" )
	PORT_DIPSETTING(    0x0c, "70k and every 200k" )
	PORT_DIPSETTING(    0x04, "100k only" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
INPUT_PORTS_END

static INPUT_PORTS_START( gigandes )
	PORT_INCLUDE( taitox_generic )

	/* 0x500000 (low) and 0x500002 (high) -> 0xf00a3c */
	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )               /* MUST REMAIN OFF ! see notes */
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x80, "Debug Mode" )                     /* see notes */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	/* 0x500004 (low) and 0x500006 (high) -> 0xf00a3e */
	PORT_MODIFY("DSWB")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x18, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Controls ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Dual ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Language ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Japanese ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_MODIFY("IN0")
	TAITO_JOY_UDLR_2_BUTTONS_START( 1 )

	PORT_MODIFY("IN1")
	TAITO_JOY_UDLR_2_BUTTONS_START( 2 )
INPUT_PORTS_END

static INPUT_PORTS_START( kyustrkr )
	PORT_INCLUDE( taitox_generic )

	/* 0x500000 (low) and 0x500002 (high) -> 0xf028fe */
	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x80, "Debug Mode" )                     /* see notes */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	/* 0x500004 (low) and 0x500006 (high) -> 0xf02900 */
	PORT_MODIFY("DSWB")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPUNUSED( 0x08, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Language ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Japanese ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )
INPUT_PORTS_END

static INPUT_PORTS_START( ballbros )
	PORT_INCLUDE( taitox_generic )

	/* 0x500000 (low) and 0x500002 (high) -> 0xf028fe */
	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )            /* to be confirmed */
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )

	/* 0x500004 (low) and 0x500006 (high) -> 0xf02900 */
	PORT_MODIFY("DSWB")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPUNUSED( 0x08, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Language ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Japanese ) )
	PORT_DIPNAME( 0x40, 0x00, "Color Change" )
	PORT_DIPSETTING(    0x00, "Less" )                           /* each pattern */
	PORT_DIPSETTING(    0x40, "More" )                           /* every 3 times */
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_MODIFY("IN0")
	TAITO_JOY_UDLR_1_BUTTON_START( 1 )

	PORT_MODIFY("IN1")
	TAITO_JOY_UDLR_1_BUTTON_START( 2 )
INPUT_PORTS_END


/**************************************************************************/

#define NUM_TILES 16384
static const gfx_layout tilelayout =
{
	16,16,  /* 16*16 sprites */
	NUM_TILES,	/* 16384 of them */
	4,	       /* 4 bits per pixel */
	{ 64*8*NUM_TILES + 8, 64*8*NUM_TILES + 0, 8, 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
		8*16, 8*16+1, 8*16+2, 8*16+3, 8*16+4, 8*16+5, 8*16+6, 8*16+7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
		16*16, 17*16, 18*16, 19*16, 20*16, 21*16, 22*16, 23*16 },

	64*8	/* every sprite takes 64 consecutive bytes */
};
#undef NUM_TILES

static const gfx_layout ballbros_tilelayout =
{
	16,16,  /* 16*16 sprites */
	4096,	/* 4096 of them */
	4,	       /* 4 bits per pixel */
	{ 0x20000*3*8, 0x20000*2*8, 0x20000*1*8, 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
		8*8, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },

	32*8	/* every sprite takes 64 consecutive bytes */
};

static GFXDECODE_START( superman )
	GFXDECODE_ENTRY( "gfx1", 0x000000, tilelayout,    0, 256 )	 /* sprites & playfield */
GFXDECODE_END

static GFXDECODE_START( ballbros )
	GFXDECODE_ENTRY( "gfx1", 0x000000, ballbros_tilelayout,    0, 256 )	 /* sprites & playfield */
GFXDECODE_END


/**************************************************************************/

/* handler called by the YM2610 emulator when the internal timers cause an IRQ */
static void irqhandler(running_device *device, int irq)
{
	cputag_set_input_line(device->machine, "audiocpu", 0, irq ? ASSERT_LINE : CLEAR_LINE);
}

static const ym2610_interface ym2610_config =
{
	irqhandler
};

static const ym2151_interface ym2151_config =
{
	irqhandler
};

static STATE_POSTLOAD( taitox_postload )
{
	reset_sound_region(machine);
}

static MACHINE_START( taitox )
{
	banknum = -1;
	state_save_register_global(machine, banknum);
	state_save_register_postload(machine, taitox_postload, NULL);
}

static const tc0140syt_interface taitox_tc0140syt_intf =
{
	"maincpu", "audiocpu"
};


/**************************************************************************/

static MACHINE_DRIVER_START( superman )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, XTAL_16MHz/2)	/* verified on pcb */
	MDRV_CPU_PROGRAM_MAP(superman_map)
	MDRV_CPU_VBLANK_INT("screen", irq6_line_hold)

	MDRV_CPU_ADD("audiocpu", Z80, XTAL_16MHz/4)	/* verified on pcb */
	MDRV_CPU_PROGRAM_MAP(sound_map)

	MDRV_QUANTUM_TIME(HZ(600))	/* 10 CPU slices per frame - enough for the sound CPU to read all commands */

	MDRV_MACHINE_START(taitox)
	MDRV_MACHINE_RESET(cchip1)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(57.43)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(52*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 48*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(superman)
	MDRV_PALETTE_LENGTH(2048)

	MDRV_VIDEO_START(seta_no_layers)
	MDRV_VIDEO_UPDATE(seta_no_layers)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2610, XTAL_16MHz/2)	/* verified on pcb */
	MDRV_SOUND_CONFIG(ym2610_config)
	MDRV_SOUND_ROUTE(0, "lspeaker",  0.25)
	MDRV_SOUND_ROUTE(0, "rspeaker", 0.25)
	MDRV_SOUND_ROUTE(1, "lspeaker",  1.0)
	MDRV_SOUND_ROUTE(2, "rspeaker", 1.0)

	MDRV_TC0140SYT_ADD("tc0140syt", taitox_tc0140syt_intf)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( daisenpu )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, XTAL_16MHz/2)	/* verified on pcb */
	MDRV_CPU_PROGRAM_MAP(daisenpu_map)
	MDRV_CPU_VBLANK_INT("screen", irq2_line_hold)

	MDRV_CPU_ADD("audiocpu", Z80, XTAL_16MHz/4)	/* verified on pcb */
	MDRV_CPU_PROGRAM_MAP(daisenpu_sound_map)

	MDRV_QUANTUM_TIME(HZ(600))	/* 10 CPU slices per frame - enough for the sound CPU to read all commands */

	MDRV_MACHINE_START(taitox)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(52*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 48*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(superman)
	MDRV_PALETTE_LENGTH(2048)

	MDRV_VIDEO_START(seta_no_layers)
	MDRV_VIDEO_UPDATE(seta_no_layers)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2151, XTAL_16MHz/4)	/* verified on pcb */
	MDRV_SOUND_CONFIG(ym2151_config)
	MDRV_SOUND_ROUTE(0, "lspeaker", 0.45)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.45)

	MDRV_TC0140SYT_ADD("tc0140syt", taitox_tc0140syt_intf)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( gigandes )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 8000000)	/* 8 MHz? */
	MDRV_CPU_PROGRAM_MAP(gigandes_map)
	MDRV_CPU_VBLANK_INT("screen", irq2_line_hold)

	MDRV_CPU_ADD("audiocpu", Z80, 4000000)	/* 4 MHz ??? */
	MDRV_CPU_PROGRAM_MAP(sound_map)

	MDRV_QUANTUM_TIME(HZ(600))	/* 10 CPU slices per frame - enough for the sound CPU to read all commands */

	MDRV_MACHINE_START(taitox)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(52*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 48*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(superman)
	MDRV_PALETTE_LENGTH(2048)

	MDRV_VIDEO_START(seta_no_layers)
	MDRV_VIDEO_UPDATE(seta_no_layers)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2610, 8000000)
	MDRV_SOUND_CONFIG(ym2610_config)
	MDRV_SOUND_ROUTE(0, "lspeaker",  0.25)
	MDRV_SOUND_ROUTE(0, "rspeaker", 0.25)
	MDRV_SOUND_ROUTE(1, "lspeaker",  1.0)
	MDRV_SOUND_ROUTE(2, "rspeaker", 1.0)

	MDRV_TC0140SYT_ADD("tc0140syt", taitox_tc0140syt_intf)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( ballbros )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 8000000)	/* 8 MHz? */
	MDRV_CPU_PROGRAM_MAP(ballbros_map)
	MDRV_CPU_VBLANK_INT("screen", irq2_line_hold)

	MDRV_CPU_ADD("audiocpu", Z80, 4000000)	/* 4 MHz ??? */
	MDRV_CPU_PROGRAM_MAP(sound_map)

	MDRV_QUANTUM_TIME(HZ(600))	/* 10 CPU slices per frame - enough for the sound CPU to read all commands */

	MDRV_MACHINE_START(taitox)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(52*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 48*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(ballbros)
	MDRV_PALETTE_LENGTH(2048)

	MDRV_VIDEO_START(seta_no_layers)
	MDRV_VIDEO_UPDATE(seta_no_layers)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2610, 8000000)
	MDRV_SOUND_CONFIG(ym2610_config)
	MDRV_SOUND_ROUTE(0, "lspeaker",  0.25)
	MDRV_SOUND_ROUTE(0, "rspeaker", 0.25)
	MDRV_SOUND_ROUTE(1, "lspeaker",  1.0)
	MDRV_SOUND_ROUTE(2, "rspeaker", 1.0)

	MDRV_TC0140SYT_ADD("tc0140syt", taitox_tc0140syt_intf)
MACHINE_DRIVER_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

/*
Superman
Taito, 1988

PCB Layout
----------

J1100145A
K1100331A
PO-039A
|---------------------------------------------------|
| VOL                        B50-07.U34  DSWB DSWA  |
|      4558       YM2610 Z80  62256              Z80|
|      4558 YM3014                                  |
|                                                   |
|                                  B06-13           |
|                                   (PAL)           |
|                                                   |
|                                                   |
|                               6264       B50-06.U3|
|J     TESTSW                                       |
|A                                                  |
|M                                                  |
|M                                                  |
|A                                 B06-101          |
|                                    (PAL)          |
|                                                Z80|
|               X1-001A                             |
|                                                   |
|    X1-004                                         |
|               X1-002A       12MHz                 |
|                                                   |
|         B50-01.U46    B50-03.U39                  |
|   X1-006                         6264             |
|X1-007        B50-02.U43   B50-04.U35     B50-05.U1|
|---------------------------------------------------|
Notes:
      All Z80 CPU's running at 6.000MHz (12/2)
      YM2203 running at 3.000Mz (12/4)
      VSync 60Hz
*/

ROM_START( superman )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "a10_09.bin", 0x00000, 0x20000, CRC(640f1d58) SHA1(e768d32eae1dba39c23189996fbd5454c8627809) )
	ROM_LOAD16_BYTE( "a05_07.bin", 0x00001, 0x20000, CRC(fddb9953) SHA1(8b562712810a5a72f4647f1ba1314a1be2e249e7) )
	ROM_LOAD16_BYTE( "a08_08.bin", 0x40000, 0x20000, CRC(79fc028e) SHA1(bf42b3f84dcad8fd9085c702a78dc895cc12d670) )
	ROM_LOAD16_BYTE( "a03_13.bin", 0x40001, 0x20000, CRC(9f446a44) SHA1(16f7cd6438e47fdaac93a368df5c093f6ff0f1f0) )

	ROM_REGION( 0x1c000, "audiocpu", 0 )     /* 64k for Z80 code */
	ROM_LOAD( "d18_10.bin", 0x00000, 0x4000, CRC(6efe79e8) SHA1(7a76efaaeab71473f4b0b23a89141f203488ce1d) )
	ROM_CONTINUE(           0x10000, 0xc000 ) /* banked stuff */

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "f01_14.bin", 0x000000, 0x80000, CRC(89368c3e) SHA1(8d227439ab321fd5d432d860544daea0e78ce588) ) /* Plane 0, 1 */
	ROM_LOAD( "h01_15.bin", 0x080000, 0x80000, CRC(910cc4f9) SHA1(9ecfa84123a8f9d048f0a689647e92f25af73899) )
	ROM_LOAD( "j01_16.bin", 0x100000, 0x80000, CRC(3622ed2f) SHA1(03f4383f6ff8b5f1e26bc6bbef2fb1855d3bb93f) ) /* Plane 2, 3 */
	ROM_LOAD( "k01_17.bin", 0x180000, 0x80000, CRC(c34f27e0) SHA1(07ee02c18ce29f35e8ae87d0c1ed80b726c246a6) )

	ROM_REGION( 0x80000, "ymsnd", 0 )	/* ADPCM samples */
	ROM_LOAD( "e18_01.bin", 0x00000, 0x80000, CRC(3cf99786) SHA1(f6febf9bda87ca04f0a5890d0e8001c26dfa6c81) )
ROM_END

ROM_START( supermanj )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "a10_09.bin", 0x00000, 0x20000, CRC(640f1d58) SHA1(e768d32eae1dba39c23189996fbd5454c8627809) )
	ROM_LOAD16_BYTE( "a05_07.bin", 0x00001, 0x20000, CRC(fddb9953) SHA1(8b562712810a5a72f4647f1ba1314a1be2e249e7) )
	ROM_LOAD16_BYTE( "a08_08.bin", 0x40000, 0x20000, CRC(79fc028e) SHA1(bf42b3f84dcad8fd9085c702a78dc895cc12d670) )
	ROM_LOAD16_BYTE( "b61-06.a3",  0x40001, 0x20000, CRC(714a0b68) SHA1(b0b42c55d2404c7c193eb8cab3bd92e321947845) )

	ROM_REGION( 0x1c000, "audiocpu", 0 )     /* 64k for Z80 code */
	ROM_LOAD( "d18_10.bin", 0x00000, 0x4000, CRC(6efe79e8) SHA1(7a76efaaeab71473f4b0b23a89141f203488ce1d) )
	ROM_CONTINUE(           0x10000, 0xc000 ) /* banked stuff */

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "f01_14.bin", 0x000000, 0x80000, CRC(89368c3e) SHA1(8d227439ab321fd5d432d860544daea0e78ce588) ) /* Plane 0, 1 */
	ROM_LOAD( "h01_15.bin", 0x080000, 0x80000, CRC(910cc4f9) SHA1(9ecfa84123a8f9d048f0a689647e92f25af73899) )
	ROM_LOAD( "j01_16.bin", 0x100000, 0x80000, CRC(3622ed2f) SHA1(03f4383f6ff8b5f1e26bc6bbef2fb1855d3bb93f) ) /* Plane 2, 3 */
	ROM_LOAD( "k01_17.bin", 0x180000, 0x80000, CRC(c34f27e0) SHA1(07ee02c18ce29f35e8ae87d0c1ed80b726c246a6) )

	ROM_REGION( 0x80000, "ymsnd", 0 )	/* ADPCM samples */
	ROM_LOAD( "e18_01.bin", 0x00000, 0x80000, CRC(3cf99786) SHA1(f6febf9bda87ca04f0a5890d0e8001c26dfa6c81) )
ROM_END

/*
Twin Hawk
Taito, 1988

J1100188A X SYSTEM
K1100443A MAIN PCB
PO-051A

*/

ROM_START( twinhawk )
	ROM_REGION( 0x40000, "maincpu", 0 )     /* 256k for 68000 code */
	ROM_LOAD16_BYTE( "b87-11.u7", 0x00000, 0x20000, CRC(fc84a399) SHA1(6e5552b7ee433bee74f8936a8e583b5f81b5f2b2) )
	ROM_LOAD16_BYTE( "b87-10.u5", 0x00001, 0x20000, CRC(17181706) SHA1(b7cab502b68a8f02918412538f23682120cbe1d3) )

	ROM_REGION( 0x14000, "audiocpu", 0 )     /* 32k for Z80 code */
	ROM_LOAD( "b87-07.13e", 0x00000, 0x4000, CRC(e2e0efa0) SHA1(4f1435ba738895996f26a64c2237e8349337df4a) )
	ROM_CONTINUE(           0x10000, 0x4000 ) /* banked stuff */

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "b87-02.3h", 0x000000, 0x80000, CRC(89ad43a0) SHA1(6ff6ee085c1c06a05f4f8743d979d3552b7475a0) ) /* Plane 0, 1 */
	ROM_LOAD( "b87-01.3f", 0x080000, 0x80000, CRC(81e82ae1) SHA1(d4dbdbf9ae0af69bbeccafb3cc2f67dadda72432) )
	ROM_LOAD( "b87-04.3k", 0x100000, 0x80000, CRC(958434b6) SHA1(cf5912c4468cb2079ff180203045a436175c037c) ) /* Plane 2, 3 */
	ROM_LOAD( "b87-03.3j", 0x180000, 0x80000, CRC(ce155ae0) SHA1(7293125fc23f2411c4edd427a2576c145b3f2dd4) )
ROM_END

ROM_START( twinhawku )
	ROM_REGION( 0x40000, "maincpu", 0 )     /* 256k for 68000 code */
	ROM_LOAD16_BYTE( "b87-09.u7", 0x00000, 0x20000, CRC(7e6267c7) SHA1(a623c1b740008675f36e8b63bbc17a573917db30) )
	ROM_LOAD16_BYTE( "b87-08.u5", 0x00001, 0x20000, CRC(31d9916f) SHA1(8ae491a51a4095717c6f65fe81a83902feccd54b) )

	ROM_REGION( 0x14000, "audiocpu", 0 )     /* 32k for Z80 code */
	ROM_LOAD( "b87-07.13e", 0x00000, 0x4000, CRC(e2e0efa0) SHA1(4f1435ba738895996f26a64c2237e8349337df4a) )
	ROM_CONTINUE(           0x10000, 0x4000 ) /* banked stuff */

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "b87-02.3h", 0x000000, 0x80000, CRC(89ad43a0) SHA1(6ff6ee085c1c06a05f4f8743d979d3552b7475a0) ) /* Plane 0, 1 */
	ROM_LOAD( "b87-01.3f", 0x080000, 0x80000, CRC(81e82ae1) SHA1(d4dbdbf9ae0af69bbeccafb3cc2f67dadda72432) )
	ROM_LOAD( "b87-04.3k", 0x100000, 0x80000, CRC(958434b6) SHA1(cf5912c4468cb2079ff180203045a436175c037c) ) /* Plane 2, 3 */
	ROM_LOAD( "b87-03.3j", 0x180000, 0x80000, CRC(ce155ae0) SHA1(7293125fc23f2411c4edd427a2576c145b3f2dd4) )
ROM_END

ROM_START( daisenpu )
	ROM_REGION( 0x40000, "maincpu", 0 )     /* 256k for 68000 code */
	ROM_LOAD16_BYTE( "b87-06.u7", 0x00000, 0x20000, CRC(cf236100) SHA1(7944a20950188f64c0a09edd1a4efe0270264b27) )
	ROM_LOAD16_BYTE( "b87-05.u5", 0x00001, 0x20000, CRC(7f15edc7) SHA1(3deba512f3c97f354ed4155f62058da160047bc5) )

	ROM_REGION( 0x14000, "audiocpu", 0 )     /* 32k for Z80 code */
	ROM_LOAD( "b87-07.13e", 0x00000, 0x4000, CRC(e2e0efa0) SHA1(4f1435ba738895996f26a64c2237e8349337df4a) )
	ROM_CONTINUE(           0x10000, 0x4000 ) /* banked stuff */

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "b87-02.3h", 0x000000, 0x80000, CRC(89ad43a0) SHA1(6ff6ee085c1c06a05f4f8743d979d3552b7475a0) ) /* Plane 0, 1 */
	ROM_LOAD( "b87-01.3f", 0x080000, 0x80000, CRC(81e82ae1) SHA1(d4dbdbf9ae0af69bbeccafb3cc2f67dadda72432) )
	ROM_LOAD( "b87-04.3k", 0x100000, 0x80000, CRC(958434b6) SHA1(cf5912c4468cb2079ff180203045a436175c037c) ) /* Plane 2, 3 */
	ROM_LOAD( "b87-03.3j", 0x180000, 0x80000, CRC(ce155ae0) SHA1(7293125fc23f2411c4edd427a2576c145b3f2dd4) )
ROM_END

ROM_START( gigandes )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "1.10a",      0x00000, 0x20000, CRC(290c50e0) SHA1(ac8008619891a5b54ba2069e4e18836976532c99) )
	ROM_LOAD16_BYTE( "3.5a",       0x00001, 0x20000, CRC(9cef82af) SHA1(6dad850de699d40dfba54bde6baca75bb0059c83) )
	ROM_LOAD16_BYTE( "east_2.8a",  0x40000, 0x20000, CRC(dd94b4d0) SHA1(2efff9fd51b28fd1fb46d16b359f0991af91054e) )
	ROM_LOAD16_BYTE( "east_4.3a",  0x40001, 0x20000, CRC(a647310a) SHA1(49db488a36f6c74729825bdf0214bcd30773eaf4) )

	ROM_REGION( 0x1c000, "audiocpu", 0 )     /* 64k for Z80 code */
	ROM_LOAD( "east_5.17d", 0x00000, 0x4000, CRC(b24ab5f4) SHA1(e4730df984e9686c538df5fc626b795bda1db939) )
	ROM_CONTINUE(           0x10000, 0xc000 ) /* banked stuff */

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "east_8.3f", 0x000000, 0x80000, CRC(75eece28) SHA1(7ce66cd8bca7dd214367beae067727c8735c0f7e) ) /* Plane 0, 1 */
	ROM_LOAD( "east_7.3h", 0x080000, 0x80000, CRC(b179a76a) SHA1(cff2caf1eb0dda8a1b8283b9950b908b102f61de) )
	ROM_LOAD( "east_9.3j", 0x100000, 0x80000, CRC(5c5e6898) SHA1(f348ac752a571902c55f36e21aa3fb9ef97528e3) ) /* Plane 2, 3 */
	ROM_LOAD( "east_6.3k", 0x180000, 0x80000, CRC(52db30e9) SHA1(0b6d73f2c6e6c1ad5fcb2a9edf50069cd0691483) )

	ROM_REGION( 0x80000, "ymsnd.deltat", 0 )      /* Delta-T samples */
	ROM_LOAD( "east-11.16f", 0x00000, 0x80000, CRC(92111f96) SHA1(e781f24761b7a923388f4cda64c7b31388fd64c5) )

	ROM_REGION( 0x80000, "ymsnd", 0 )   /* ADPCM samples */
	ROM_LOAD( "east-10.16e", 0x00000, 0x80000, CRC(ca0ac419) SHA1(b29f30a8ff1286c65b741353b6551918a45bcafe) )
ROM_END

ROM_START( gigandesj )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "east_1.10a", 0x00000, 0x20000, CRC(ae74e4e5) SHA1(1CAC0A0E591B63142D8D249C67F803256FB28C2A) )
	ROM_LOAD16_BYTE( "east_3.5a",  0x00001, 0x20000, CRC(8bcf2116) SHA1(9255D7E0AB568AD7A894421D3260FA80B8A0A5D0) )
	ROM_LOAD16_BYTE( "east_2.8a",  0x40000, 0x20000, CRC(dd94b4d0) SHA1(2efff9fd51b28fd1fb46d16b359f0991af91054e) )
	ROM_LOAD16_BYTE( "east_4.3a",  0x40001, 0x20000, CRC(a647310a) SHA1(49db488a36f6c74729825bdf0214bcd30773eaf4) )

	ROM_REGION( 0x1c000, "audiocpu", 0 )     /* 64k for Z80 code */
	ROM_LOAD( "east_5.17d", 0x00000, 0x4000, CRC(b24ab5f4) SHA1(e4730df984e9686c538df5fc626b795bda1db939) )
	ROM_CONTINUE(           0x10000, 0xc000 ) /* banked stuff */

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "east_8.3f", 0x000000, 0x80000, CRC(75eece28) SHA1(7ce66cd8bca7dd214367beae067727c8735c0f7e) ) /* Plane 0, 1 */
	ROM_LOAD( "east_7.3h", 0x080000, 0x80000, CRC(b179a76a) SHA1(cff2caf1eb0dda8a1b8283b9950b908b102f61de) )
	ROM_LOAD( "east_9.3j", 0x100000, 0x80000, CRC(5c5e6898) SHA1(f348ac752a571902c55f36e21aa3fb9ef97528e3) ) /* Plane 2, 3 */
	ROM_LOAD( "east_6.3k", 0x180000, 0x80000, CRC(52db30e9) SHA1(0b6d73f2c6e6c1ad5fcb2a9edf50069cd0691483) )

	ROM_REGION( 0x80000, "ymsnd.deltat", 0 )   /* Delta-T samples */
	ROM_LOAD( "east-11.16f", 0x00000, 0x80000, CRC(92111f96) SHA1(e781f24761b7a923388f4cda64c7b31388fd64c5) )

	ROM_REGION( 0x80000, "ymsnd", 0 )   /* ADPCM samples */
	ROM_LOAD( "east-10.16e", 0x00000, 0x80000, CRC(ca0ac419) SHA1(b29f30a8ff1286c65b741353b6551918a45bcafe) )
ROM_END

ROM_START( kyustrkr )
	ROM_REGION( 0x40000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "pe.9a", 0x00000, 0x20000, CRC(082b5f96) SHA1(97c08b506b2a07d63f3323359b8564aa3621f483) )
	ROM_LOAD16_BYTE( "po.4a", 0x00001, 0x20000, CRC(0100361e) SHA1(45791f697c86309c459d0d8c3d3e967a3ece3ede) )

	ROM_REGION( 0x1c000, "audiocpu", 0 )     /* 64k for Z80 code */
	ROM_LOAD( "ic.18d", 0x00000, 0x4000, CRC(92cfb788) SHA1(41cd5433584df05652bd0ce8c5a35dc38262d6f2) )
	ROM_CONTINUE(       0x10000, 0xc000 ) /* banked stuff */

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "m-8-3.u3",     0x00000, 0x20000, CRC(1c4084e6) SHA1(addea2ba07bddb41fbe7f0fc859e744990bb9ff5) )
	ROM_LOAD( "m-8-2.u4",     0x20000, 0x20000, CRC(ada21c4d) SHA1(a683c8d798370c50d9bd5e67e91d7ed0f1659c20) )
	ROM_LOAD( "m-8-1.u5",     0x40000, 0x20000, CRC(9d95aad6) SHA1(3391b14196fea12223ab247d909791bc68fc8d56) )
	ROM_LOAD( "m-8-0.u6",     0x60000, 0x20000, CRC(0dfb6ed3) SHA1(0937614c8f97040d0216363bfb2bc21161128a3c) )

	ROM_REGION( 0x80000, "ymsnd.deltat", 0 )   /* Delta-T samples */
	ROM_LOAD( "m-8-5.u2",     0x00000, 0x20000, CRC(d9d90e0a) SHA1(1011548b4fb5f1a194c93ded512e74cda2c06ceb) )

	ROM_REGION( 0x80000, "ymsnd", 0 )   /* ADPCM samples */
	ROM_LOAD( "m-8-4.u1",     0x00000, 0x20000, CRC(d3f6047a) SHA1(0db6d762bbe2d68cddf30e06125b904e1021b96d) )
ROM_END

ROM_START( ballbros )
	ROM_REGION( 0x40000, "maincpu", 0 )     /* 256k for 68000 code */
	ROM_LOAD16_BYTE( "10a", 0x00000, 0x20000, CRC(4af0e858) SHA1(817817169aee075d52411bdbe568514511760386) )
	ROM_LOAD16_BYTE( "5a",  0x00001, 0x20000, CRC(0b983a69) SHA1(7be06761a19e1dc5d1404d1920797b406421e365) )

	ROM_REGION( 0x1c000, "audiocpu", 0 )     /* 64k for Z80 code */
	ROM_LOAD( "8d", 0x00000, 0x4000, CRC(d1c515af) SHA1(00451991b4c793487b156f9be2b2e4688325ff24) )
	ROM_CONTINUE(   0x10000, 0xc000 ) /* banked stuff */

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "3", 0x000000, 0x20000, CRC(ec3e0537) SHA1(51fe5c6ef007c188b2f51ad2225753d2b403e35a) ) /* Plane 0, 1 */
	ROM_LOAD( "2", 0x020000, 0x20000, CRC(bb441717) SHA1(205ae0aa3ded11766ae8f6fe7d08fefff17a9b73) )
	ROM_LOAD( "1", 0x040000, 0x20000, CRC(8196d624) SHA1(c859e3b1d3b481f38cfe47576efc1dcdbe6cde28) )
	ROM_LOAD( "0", 0x060000, 0x20000, CRC(1cc584e5) SHA1(18cf607fa06c095d088b80cea2a1e507d19c7126) )

	ROM_REGION( 0x80000, "ymsnd.deltat", 0 )   /* Delta-T samples */
	ROM_LOAD( "east-11", 0x00000, 0x80000, CRC(92111f96) SHA1(e781f24761b7a923388f4cda64c7b31388fd64c5) )

	ROM_REGION( 0x80000, "ymsnd", 0 )   /* ADPCM samples */
	ROM_LOAD( "east-10", 0x00000, 0x80000, CRC(ca0ac419) SHA1(b29f30a8ff1286c65b741353b6551918a45bcafe) )
ROM_END


static DRIVER_INIT( kyustrkr )
{
	memory_install_write16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x900000, 0x90000f, 0, 0, kyustrkr_input_w);
}


GAME( 1988, superman,  0,        superman, superman,  0,        ROT0,   "Taito Corporation", "Superman", 0 )
GAME( 1988, supermanj, superman, superman, supermanj, 0,        ROT0,   "Taito Corporation", "Superman (Japan)", 0 )
GAME( 1989, twinhawk,  0,        daisenpu, twinhawk,  0,        ROT270, "Taito Corporation Japan", "Twin Hawk (World)", 0 )
GAME( 1989, twinhawku, twinhawk, daisenpu, twinhawku, 0,        ROT270, "Taito America Corporation", "Twin Hawk (US)", 0 )
GAME( 1989, daisenpu,  twinhawk, daisenpu, daisenpu,  0,        ROT270, "Taito Corporation", "Daisenpu (Japan)", 0 )
GAME( 1989, gigandes,  0,        gigandes, gigandes,  0,        ROT0,   "East Technology", "Gigandes", 0 )
GAME( 1989, gigandesj, gigandes, gigandes, gigandes,  0,        ROT0,   "East Technology", "Gigandes (Japan)", 0 )
GAME( 1989, kyustrkr,  0,        ballbros, kyustrkr,  kyustrkr, ROT180, "East Technology", "Last Striker / Kyuukyoku no Striker", 0 )
GAME( 1992, ballbros,  0,        ballbros, ballbros,  0,        ROT0,   "East Technology", "Balloon Brothers", 0 )
