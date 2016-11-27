/***************************************************************************

Taito Triple Screen Games
=========================

Ninja Warriors (c) 1987 Taito
Darius 2       (c) 1989 Taito

David Graves

(this is based on the F2 driver by Bryan McPhail, Brad Oliver, Andrew Prime,
Nicola Salmoria. Thanks to Richard Bush and the Raine team, whose open
source was very helpful in many areas particularly the sprites.)

                *****

The triple screen games operate on hardware with various similarities to
the Taito F2 system, as they share some custom ics e.g. the TC0100SCN.

According to Sixtoe: "The multi-monitor systems had 2 or 3 13" screens;
one in the middle facing the player, and the other 1 or 2 on either side
mounted below and facing directly up reflecting off special semi-reflecting
mirrors, with about 1" of the graphics being overlapped on each screen.
This was the only way to get uninterrupted screens and to be able to see
through both ways. Otherwise you`d have the monitors' edges visible.
You can tell if your arcade has been cheap (like one near me) when you
look at the screens and can see black triangles on the left or right, this
means they bought ordinary mirrors and you can't see through them the
wrong way, as the semi-reflecting mirrors were extremely expensive."

For each screen the games have 3 separate layers of graphics:- one
128x64 tiled scrolling background plane of 8x8 tiles, a similar
foreground plane, and a 128x32 text plane with character definitions
held in ram.

Writing to the first TC0100SCN "writes through" to the two subsidiary
chips so that all three have identical contents. The subsidiary ones are
only addressed individually during initial memory checks, I think. (?)

There is a single sprite plane which covers all 3 screens.
The sprites are 16x16 and are not zoomable.

Twin 68000 processors are used; both have access to sprite ram and
the tilemap areas, and they communicate via 64K of shared ram.

Sound is dealt with by a Z80 controlling a YM2610. Sound commands
are written to the Z80 by the 68000 (the same as in Taito F2 games).


Tilemaps
========

TC0100SCN has tilemaps twice as wide as usual. The two BG tilemaps take
up twice the usual space, $8000 bytes each. The text tilemap takes up
the usual space, because its height is halved.

The triple palette generator (one for each screen) is probably just a
result of the way the hardware works: the colors in each are the same.


Ninja Warriors
Taito, 1987

PCB Layout
----------

J1100136A
K1100313A
SOUND BOARD
|------------------------------|
|  Z80     16MHz        YM2610 |
|M      B31-37.11              |
|  TMM2063           TC0140SYT |
|  TC0060DCA                   |
|  TC0060DCA  YM3016-F         |
|  TL074 TL074 TL074           |
|                     B31-08.19|
|                     B31-09.18|
|  MB3735   MB3735    B31-10.17|
|                     B31-11.16|
|  VOL1     VOL2               |
|                        S     |
|------------------------------|
Notes:
      Z80        - clock 4.000MHz [16/4]
      YM2610     - clock 8.000MHz [16/2]
      MB3735     - Audio power AMP
      TC006DCA   - Taito custom ceramic DAC/filter module (SIL20)
      TC0140SYT  - Taito custom sound control IC (QFP120)
      YM3016F    - Yamaha stereo audio DAC (DOIC16)
      TL074      - Texas Instruments low noise quad J-Fet op-amp (DIP14)
      M          - 25 pin connector
      S          - 30 pin flat cable joining to CPU BOARD
      TMM2063    - 8kx8 SRAM (DIP28)
      B31-37     - TC541000 mask ROM (DIP32)
      Other B31* - 234000 mask ROM (DIP40)


K1100311B
J1100134B
CPU BOARD
|--------------------------------------------------------------|
|                     2018                              2063   |
|                    TC0070RGB   TC0110PCR    B31-02.29 43256  |
|     68000             2018        TC0100SCN B31-01.28 43256  |
|                           26.686MHz                   2063   |
|                B31-16.68  16MHZ                              |
|H                     2018                             2063   |
|  B31-36.97 B31-33.87  TC0070RGB  TC011PCR   B31-02.27 43256  |
|  B31-35.96 B31-32.86  2018       TC0100SCN  B31-01.26 43256  |
|  B31-34.95 B31-31.85  B31-15.62                       2063   |
|J    DIP28     DIP28                                          |
|   43256     43256                                            |
|                                                       2063   |
|                 2018            TC0100SCN  B31-02.24  43256  |
|     B31-17.82   TC0070RGB   TC0110PCR      B31-01.24  43256  |
|                     B31-14-1.54                       2063   |
|                                                              |
|                 2018                                  43256  |
|G                                                      43256  |
|                                43256     43256     B31-13.20 |
|                         3771   B31-45.35 B31-47.32    68000  |
|     PC050CM    TC0040IOC  DSWA B31-29.34 B31-27.31           |
|                           DSWB    DIP40             B31-12.1 |
|--------------------------------------------------------------|
Notes:
      43256 - 32kx8 SRAM (DIP28)
      2063  - 8kx8 SRAM (NDIP28)
      DIP28 - Empty socket(s)
      DIP40 - Socket for mounting 'ROM 5 BOARD'
      2018  - 2kx8 SRAM (NDIP24)
      H     - 12 pin connector for power input
      J     - 15 pin connector
      G     - 22-way edge connector
      B31-12 thru B31-17 - PALs


K9100162A
J9100118A
ROM 5 BOARD
|-----------------|
|B31-38.3 B31-40.6|
|B31-39.2 B31-41.5|
|                 |
| DIP40     74F139|
|-----------------|


K1100312A
J1100135A
OBJECT BOARD
|--------------------------------------------------------------|
|                                                              |
|                    B31-18.IC78                               |
|                                    TMM2064   TMM2064 TMM2064 |
|                    B31-19.IC80                               |
|                                    TMM2064                   |
|                                                              |
|                                                              |
|                             B31-20.IC119                     |
|                             B31-21.IC120                     |
|                             B31-22.IC121                     |
|                                                  B31-04.IC173|
|                                     B31-23.IC143 B31-05.IC174|
|                                     B31-24.IC144 B31-06.IC175|
|                                                  B31-07.IC176|
|                  MB81461 MB81461 MB81461 MB81461             |
|                  MB81461 MB81461 MB81461 MB81461             |
|                  MB81461 MB81461 MB81461 MB81461             |
|H                 MB81461 MB81461 MB81461 MB81461   TC0120SHT |
|                  MB81461 MB81461 MB81461 MB81461             |
|                  MB81461 MB81461 MB81461 MB81461             |
|                  MB81461 MB81461 MB81461 MB81461             |
|                  MB81461 MB81461 MB81461 MB81461             |
|--------------------------------------------------------------|
Notes:
      B31-04 thru 06 - 234000 mask ROM (DIP40)
      B31-18 thru 24 - PALs
      MB81461        - 64kx4 dual-port DRAM (ZIP24)
      TMM2064        - 8kx8 SRAM (DIP28)

Dumpers Notes
-------------

Ninja Warriors (JPN Ver.)
(c)1987 Taito

Sound Board
K1100313A
CPU     :Z80
Sound   :YM2610
OSC     :16000.00KHz
Other   :TC0140SYT,TC0060DCA x2
-----------------------
B31-08.19
B31-09.18
B31-10.17
B31-11.16
B31_37.11
-----------------------
CPU Board
M4300086A
K1100311A
CPU     :TS68000CP8 x2
Sound   :YM2610
OSC     :26686.00KHz,16000.00KHz
Other   :TC0040IOC,TC0070RGB x3,TC0110PCR x3,TC0100SCN x3
-----------------------
B31-01.23
B31-01.26
B31-01.28
B31-02.24
B31-02.27
B31-02.29
B31_27.31
B31_28.32
B31_29.34
B31_30.35
B31_31.85
B31_32.86
B31_33.87
B31_34.95
B31_35.96
B31_36.97
B31_38.3
B31_39.2
B31_40.6
B31_41.5
-----------------------
OBJECT Board
K1100312A
Other   :TC0120SHT
-----------------------
B31-04.173
B31-05.174
B31-06.175
B31-07.176
B31-25.38
B31-26.58


Stephh's notes (based on the game M68000 code and some tests) :

1) 'ninjaw*'

  - Region stored at 0x01fffe.w
  - Sets :
      * 'ninjaw' : region = 0x0003
      * 'ninjawj' : region = 0x0000
  - Coinage relies on the region (code at 0x0013bc) :
      * 0x0000 (Japan), 0x0001 (?) and 0x0002 (US) use TAITO_COINAGE_JAPAN_OLD
      * 0x0003 (World) and 0x0004 (licensed to xxx) use TAITO_COINAGE_WORLD
  - Notice screen only if region = 0x0000
  - According to the manual, DSWB bit 6 determines continue pricing :

    PORT_DIPNAME( 0x40, 0x00, DEF_STR( Continue_Price ) ) PORT_DIPLOCATION("SW2:7")
    PORT_DIPSETTING(    0x40, DEF_STR( 1C_1C ) )
    PORT_DIPSETTING(    0x00, "Same as Start" )

    However, many conditions are required to make it work due to code at 0x001404 :
      * region must not be 0x0000
      * coinage must be the same for both slots
      * coinage must be 2C_1C
    This is why this Dip Switch has NO effect in the sets we have :
      * 'ninjaw' : coinage is always different for the 2 slots
      * 'ninjawj' : region = 0x0000


2) 'darius2'

  - Region stored at 0x03fffe.w
  - Sets :
      * 'darius2' : region = 0x0001
  - Coinage relies on the region (code at 0x00f37a) :
      * 0x0000 (?), 0x0001 (Japan) and 0x0002 (US) use TAITO_COINAGE_JAPAN_OLD
      * 0x0002 (US, licensed to ROMSTAR) uses slighlty different TAITO_COINAGE_US :
        4C_3C instead of 4C_1C, same other settings otherwise
      * 0x0003 (World) uses TAITO_COINAGE_WORLD
  - Texts and game name rely on the region :
      * 0x0001 : some texts in Japanese - game name is "Darius II"
      * other : all texts in English - game name is "Sagaia"
  - Notice screen only if region = 0x0001
  - FBI logo only if region = 0x0002
  - Japan version resets score on continue, other versions don't


TODO
====

Verify 68000 clock rates. Unknown sprite bits.


Ninjaw
------

"Subwoofer" sound filtering isn't perfect.

Some enemies slide relative to the background when they should
be standing still. High cpu interleaving doesn't help much.


Darius 2
--------

"Subwoofer" sound filtering isn't perfect.

(When you lose a life or big enemies appear it's meant to create
rumbling on a subwoofer in the cabinet.)


***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "video/taitoic.h"
#include "machine/taitoio.h"
#include "audio/taitosnd.h"
#include "sound/2610intf.h"
#include "sound/flt_vol.h"
#include "includes/taitoipt.h"
#include "includes/ninjaw.h"

extern const char layout_darius[];

static void parse_control( running_machine *machine )	/* assumes Z80 sandwiched between 68Ks */
{
	/* bit 0 enables cpu B */
	/* however this fails when recovering from a save state
       if cpu B is disabled !! */
	ninjaw_state *state = (ninjaw_state *)machine->driver_data;
	cpu_set_input_line(state->subcpu, INPUT_LINE_RESET, (state->cpua_ctrl & 0x1) ? CLEAR_LINE : ASSERT_LINE);

}

static WRITE16_HANDLER( cpua_ctrl_w )
{
	ninjaw_state *state = (ninjaw_state *)space->machine->driver_data;

	if ((data &0xff00) && ((data &0xff) == 0))
		data = data >> 8;
	state->cpua_ctrl = data;

	parse_control(space->machine);

	logerror("CPU #0 PC %06x: write %04x to cpu control\n", cpu_get_pc(space->cpu), data);
}


/*****************************************
            SOUND
*****************************************/

static void reset_sound_region( running_machine *machine )
{
	ninjaw_state *state = (ninjaw_state *)machine->driver_data;
	memory_set_bank(machine, "bank10", state->banknum);
}

static WRITE8_HANDLER( sound_bankswitch_w )
{
	ninjaw_state *state = (ninjaw_state *)space->machine->driver_data;

	state->banknum = data & 7;
	reset_sound_region(space->machine);
}

static WRITE16_HANDLER( ninjaw_sound_w )
{
	ninjaw_state *state = (ninjaw_state *)space->machine->driver_data;

	if (offset == 0)
		tc0140syt_port_w(state->tc0140syt, 0, data & 0xff);
	else if (offset == 1)
		tc0140syt_comm_w(state->tc0140syt, 0, data & 0xff);

#ifdef MAME_DEBUG
	if (data & 0xff00)
		popmessage("ninjaw_sound_w to high byte: %04x", data);
#endif
}

static READ16_HANDLER( ninjaw_sound_r )
{
	ninjaw_state *state = (ninjaw_state *)space->machine->driver_data;

	if (offset == 1)
		return ((tc0140syt_comm_r(state->tc0140syt, 0) & 0xff));
	else
		return 0;
}


/**** sound pan control ****/

static WRITE8_HANDLER( ninjaw_pancontrol )
{
	ninjaw_state *state = (ninjaw_state *)space->machine->driver_data;
	running_device *flt = NULL;
	offset &= 3;

	switch (offset)
	{
		case 0: flt = state->_2610_1l; break;
		case 1: flt = state->_2610_1r; break;
		case 2: flt = state->_2610_2l; break;
		case 3: flt = state->_2610_2r; break;
	}

	state->pandata[offset] = (float)data * (100.f / 255.0f);
	//popmessage(" pan %02x %02x %02x %02x", state->pandata[0], state->pandata[1], state->pandata[2], state->pandata[3] );
	flt_volume_set_volume(flt, state->pandata[offset] / 100.0);
}


static WRITE16_HANDLER( tc0100scn_triple_screen_w )
{
	ninjaw_state *state = (ninjaw_state *)space->machine->driver_data;

	tc0100scn_word_w(state->tc0100scn_1, offset, data, mem_mask);
	tc0100scn_word_w(state->tc0100scn_2, offset, data, mem_mask);
	tc0100scn_word_w(state->tc0100scn_3, offset, data, mem_mask);
}

/***********************************************************
             MEMORY STRUCTURES
***********************************************************/

static ADDRESS_MAP_START( ninjaw_master_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0bffff) AM_ROM
	AM_RANGE(0x0c0000, 0x0cffff) AM_RAM														/* main ram */
	AM_RANGE(0x200000, 0x200001) AM_DEVREADWRITE8("tc0220ioc", tc0220ioc_portreg_r, tc0220ioc_portreg_w, 0x00ff)
	AM_RANGE(0x200002, 0x200003) AM_DEVREADWRITE8("tc0220ioc", tc0220ioc_port_r, tc0220ioc_port_w, 0x00ff)
	AM_RANGE(0x210000, 0x210001) AM_WRITE(cpua_ctrl_w)
	AM_RANGE(0x220000, 0x220003) AM_READWRITE(ninjaw_sound_r,ninjaw_sound_w)
	AM_RANGE(0x240000, 0x24ffff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x260000, 0x263fff) AM_RAM AM_SHARE("share2") AM_BASE_SIZE_MEMBER(ninjaw_state, spriteram, spriteram_size)
	AM_RANGE(0x280000, 0x293fff) AM_DEVREAD("tc0100scn_1", tc0100scn_word_r) AM_WRITE(tc0100scn_triple_screen_w)	/* tilemaps (1st screen/all screens) */
	AM_RANGE(0x2a0000, 0x2a000f) AM_DEVREADWRITE("tc0100scn_1", tc0100scn_ctrl_word_r, tc0100scn_ctrl_word_w)
	AM_RANGE(0x2c0000, 0x2d3fff) AM_DEVREADWRITE("tc0100scn_2", tc0100scn_word_r, tc0100scn_word_w)		/* tilemaps (2nd screen) */
	AM_RANGE(0x2e0000, 0x2e000f) AM_DEVREADWRITE("tc0100scn_2", tc0100scn_ctrl_word_r, tc0100scn_ctrl_word_w)
	AM_RANGE(0x300000, 0x313fff) AM_DEVREADWRITE("tc0100scn_3", tc0100scn_word_r, tc0100scn_word_w)		/* tilemaps (3rd screen) */
	AM_RANGE(0x320000, 0x32000f) AM_DEVREADWRITE("tc0100scn_3", tc0100scn_ctrl_word_r, tc0100scn_ctrl_word_w)
	AM_RANGE(0x340000, 0x340007) AM_DEVREADWRITE("tc0110pcr_1", tc0110pcr_word_r, tc0110pcr_step1_word_w)		/* palette (1st screen) */
	AM_RANGE(0x350000, 0x350007) AM_DEVREADWRITE("tc0110pcr_2", tc0110pcr_word_r, tc0110pcr_step1_word_w)		/* palette (2nd screen) */
	AM_RANGE(0x360000, 0x360007) AM_DEVREADWRITE("tc0110pcr_3", tc0110pcr_word_r, tc0110pcr_step1_word_w)		/* palette (3rd screen) */
ADDRESS_MAP_END

// NB there could be conflicts between which cpu writes what to the
// palette, as our interleaving won't match the original board.

static ADDRESS_MAP_START( ninjaw_slave_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x05ffff) AM_ROM
	AM_RANGE(0x080000, 0x08ffff) AM_RAM	/* main ram */
	AM_RANGE(0x200000, 0x200001) AM_DEVREADWRITE8("tc0220ioc", tc0220ioc_portreg_r, tc0220ioc_portreg_w, 0x00ff)
	AM_RANGE(0x200002, 0x200003) AM_DEVREADWRITE8("tc0220ioc", tc0220ioc_port_r, tc0220ioc_port_w, 0x00ff)
	AM_RANGE(0x240000, 0x24ffff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x260000, 0x263fff) AM_RAM AM_SHARE("share2")
	AM_RANGE(0x280000, 0x293fff) AM_DEVREAD("tc0100scn_1", tc0100scn_word_r) AM_WRITE(tc0100scn_triple_screen_w)	/* tilemaps (1st screen/all screens) */
	AM_RANGE(0x340000, 0x340007) AM_DEVREADWRITE("tc0110pcr_1", tc0110pcr_word_r, tc0110pcr_step1_word_w)		/* palette (1st screen) */
	AM_RANGE(0x350000, 0x350007) AM_DEVREADWRITE("tc0110pcr_2", tc0110pcr_word_r, tc0110pcr_step1_word_w)		/* palette (2nd screen) */
	AM_RANGE(0x360000, 0x360007) AM_DEVREADWRITE("tc0110pcr_3", tc0110pcr_word_r, tc0110pcr_step1_word_w)		/* palette (3rd screen) */
ADDRESS_MAP_END

static ADDRESS_MAP_START( darius2_master_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0bffff) AM_ROM
	AM_RANGE(0x0c0000, 0x0cffff) AM_RAM							/* main ram */
	AM_RANGE(0x200000, 0x200001) AM_DEVREADWRITE8("tc0220ioc", tc0220ioc_portreg_r, tc0220ioc_portreg_w, 0x00ff)
	AM_RANGE(0x200002, 0x200003) AM_DEVREADWRITE8("tc0220ioc", tc0220ioc_port_r, tc0220ioc_port_w, 0x00ff)
	AM_RANGE(0x210000, 0x210001) AM_WRITE(cpua_ctrl_w)
	AM_RANGE(0x220000, 0x220003) AM_READWRITE(ninjaw_sound_r,ninjaw_sound_w)
	AM_RANGE(0x240000, 0x24ffff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x260000, 0x263fff) AM_RAM AM_SHARE("share2") AM_BASE_SIZE_MEMBER(ninjaw_state, spriteram, spriteram_size)
	AM_RANGE(0x280000, 0x293fff) AM_DEVREAD("tc0100scn_1", tc0100scn_word_r) AM_WRITE(tc0100scn_triple_screen_w)	/* tilemaps (1st screen/all screens) */
	AM_RANGE(0x2a0000, 0x2a000f) AM_DEVREADWRITE("tc0100scn_1", tc0100scn_ctrl_word_r, tc0100scn_ctrl_word_w)
	AM_RANGE(0x2c0000, 0x2d3fff) AM_DEVREADWRITE("tc0100scn_2", tc0100scn_word_r, tc0100scn_word_w)		/* tilemaps (2nd screen) */
	AM_RANGE(0x2e0000, 0x2e000f) AM_DEVREADWRITE("tc0100scn_2", tc0100scn_ctrl_word_r, tc0100scn_ctrl_word_w)
	AM_RANGE(0x300000, 0x313fff) AM_DEVREADWRITE("tc0100scn_3", tc0100scn_word_r, tc0100scn_word_w)		/* tilemaps (3rd screen) */
	AM_RANGE(0x320000, 0x32000f) AM_DEVREADWRITE("tc0100scn_3", tc0100scn_ctrl_word_r, tc0100scn_ctrl_word_w)
	AM_RANGE(0x340000, 0x340007) AM_DEVREADWRITE("tc0110pcr_1", tc0110pcr_word_r, tc0110pcr_step1_word_w)		/* palette (1st screen) */
	AM_RANGE(0x350000, 0x350007) AM_DEVREADWRITE("tc0110pcr_2", tc0110pcr_word_r, tc0110pcr_step1_word_w)		/* palette (2nd screen) */
	AM_RANGE(0x360000, 0x360007) AM_DEVREADWRITE("tc0110pcr_3", tc0110pcr_word_r, tc0110pcr_step1_word_w)		/* palette (3rd screen) */
ADDRESS_MAP_END

static ADDRESS_MAP_START( darius2_slave_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x05ffff) AM_ROM
	AM_RANGE(0x080000, 0x08ffff) AM_RAM														/* main ram */
	AM_RANGE(0x200000, 0x200001) AM_DEVREADWRITE8("tc0220ioc", tc0220ioc_portreg_r, tc0220ioc_portreg_w, 0x00ff)
	AM_RANGE(0x200002, 0x200003) AM_DEVREADWRITE8("tc0220ioc", tc0220ioc_port_r, tc0220ioc_port_w, 0x00ff)
	AM_RANGE(0x240000, 0x24ffff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x260000, 0x263fff) AM_RAM AM_SHARE("share2")
	AM_RANGE(0x280000, 0x293fff) AM_DEVREAD("tc0100scn_1", tc0100scn_word_r) AM_WRITE(tc0100scn_triple_screen_w)	/* tilemaps (1st screen/all screens) */
ADDRESS_MAP_END


/***************************************************************************/

static ADDRESS_MAP_START( ninjaw_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank10")
	AM_RANGE(0xc000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xe003) AM_DEVREADWRITE("ymsnd", ym2610_r,ym2610_w)
	AM_RANGE(0xe200, 0xe200) AM_READNOP AM_DEVWRITE("tc0140syt", tc0140syt_slave_port_w)
	AM_RANGE(0xe201, 0xe201) AM_DEVREADWRITE("tc0140syt", tc0140syt_slave_comm_r,tc0140syt_slave_comm_w)
	AM_RANGE(0xe400, 0xe403) AM_WRITE(ninjaw_pancontrol) /* pan */
	AM_RANGE(0xea00, 0xea00) AM_READNOP
	AM_RANGE(0xee00, 0xee00) AM_WRITENOP /* ? */
	AM_RANGE(0xf000, 0xf000) AM_WRITENOP /* ? */
	AM_RANGE(0xf200, 0xf200) AM_WRITE(sound_bankswitch_w)
ADDRESS_MAP_END


/***********************************************************
             INPUT PORTS, DIPs
***********************************************************/

static INPUT_PORTS_START( ninjaw )
	/* 0x200000 (port 0) -> 0x0c2291.b and 0x24122c (shared RAM) */
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPUNUSED_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW1:2" )
	TAITO_DSWA_BITS_2_TO_3_LOC(SW1)
	TAITO_COINAGE_WORLD_LOC(SW1)

	/* 0x200000 (port 1) -> 0x0c2290.b and 0x24122e (shared RAM) */
	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SW2)
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW2:3" )        /* Manual shows switches 3, 4, 5, 6 & 8 as not used */
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW2:7" )        /* see notes */
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW2:8" )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )                /* Stops working if this is high */
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Freeze") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("IN1")
	TAITO_JOY_DUAL_UDRL( 1, 2 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
INPUT_PORTS_END

/* Can't use PORT_INCLUDE because of PORT_DIPLOCATION */
static INPUT_PORTS_START( ninjawj )
	PORT_INCLUDE(ninjaw)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_JAPAN_OLD_LOC(SW1)
INPUT_PORTS_END

/* Can't use PORT_INCLUDE because of PORT_DIPLOCATION */
static INPUT_PORTS_START( darius2 )
	PORT_INCLUDE(ninjaw)

	/* 0x200000 (port 0) -> 0x0c2002 (-$5ffe,A5) and 0x0c2006 (-$5ffa,A5) */
	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:1")    /* code at 0x00c20e */
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Auto Fire" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Fast" )
	TAITO_DSWA_BITS_2_TO_3_LOC(SW1)
	TAITO_COINAGE_JAPAN_OLD_LOC(SW1)

	/* 0x200000 (port 1) -> 0x0c2004 (-$5ffc,A5) and 0x0c2008 (-$5ff8,A5) */
	PORT_MODIFY("DSWB")
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
	PORT_DIPNAME( 0x40, 0x40, "Invulnerability (Cheat)") PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )
INPUT_PORTS_END


/***********************************************************
                GFX DECODING

    (Thanks to Raine for the obj decoding)
***********************************************************/

static const gfx_layout tilelayout =
{
	16,16,	/* 16*16 sprites */
	RGN_FRAC(1,1),
	4,	/* 4 bits per pixel */
	{ 8, 12, 0, 4 },	/* pixel bits separated, jump 4 to get to next one */
	{ 3, 2, 1, 0, 19, 18, 17, 16,
	  3+ 32*8, 2+ 32*8, 1+ 32*8, 0+ 32*8, 19+ 32*8, 18+ 32*8, 17+ 32*8, 16+ 32*8 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
	  64*8 + 0*32, 64*8 + 1*32, 64*8 + 2*32, 64*8 + 3*32,
	  64*8 + 4*32, 64*8 + 5*32, 64*8 + 6*32, 64*8 + 7*32 },
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

static GFXDECODE_START( ninjaw )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout,  0, 256 )	/* sprites */
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,  0, 256 )	/* scr tiles (screen 1) */
	GFXDECODE_ENTRY( "gfx3", 0, charlayout,  0, 256 )	/* scr tiles (screens 2+) */
GFXDECODE_END


/**************************************************************
                 YM2610 (SOUND)
**************************************************************/

/* handler called by the YM2610 emulator when the internal timers cause an IRQ */
static void irqhandler( running_device *device, int irq )
{
	ninjaw_state *state = (ninjaw_state *)device->machine->driver_data;
	cpu_set_input_line(state->audiocpu, 0, irq ? ASSERT_LINE : CLEAR_LINE);
}

static const ym2610_interface ym2610_config =
{
	irqhandler
};


/**************************************************************
                 SUBWOOFER (SOUND)
**************************************************************/

#if 0
static DEVICE_START( subwoofer )
{
	/* Adjust the lowpass filter of the first three YM2610 channels */

	/* The 150 Hz is a common top frequency played by a generic */
	/* subwoofer, the real Arcade Machine may differs */

	mixer_set_lowpass_frequency(0, 20);
	mixer_set_lowpass_frequency(1, 20);
	mixer_set_lowpass_frequency(2, 20);

	return 0;
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

DECLARE_LEGACY_SOUND_DEVICE(SUBWOOFER, subwoofer);
DEFINE_LEGACY_SOUND_DEVICE(SUBWOOFER, subwoofer);
#endif


/*************************************************************
                 MACHINE DRIVERS

Ninjaw: high interleaving of 100, but doesn't stop enemies
"sliding" when they should be standing still relative
to the scrolling background.

Darius2: arbitrary interleaving of 10 to keep cpus synced.
*************************************************************/

static const tc0100scn_interface darius2_tc0100scn_intf_l =
{
	"lscreen",
	1, 3,		/* gfxnum, txnum */
	22, 0,		/* x_offset, y_offset */
	0, 0,		/* flip_xoff, flip_yoff */
	0, 0,		/* flip_text_xoff, flip_text_yoff */
	0, 0
};

static const tc0100scn_interface darius2_tc0100scn_intf_m =
{
	"mscreen",
	2, 3,		/* gfxnum, txnum */
	22, 0,		/* x_offset, y_offset */
	0, 0,		/* flip_xoff, flip_yoff */
	0, 0,		/* flip_text_xoff, flip_text_yoff */
	2, 1
};

static const tc0100scn_interface darius2_tc0100scn_intf_r =
{
	"rscreen",
	2, 3,		/* gfxnum, txnum */
	22, 0,		/* x_offset, y_offset */
	0, 0,		/* flip_xoff, flip_yoff */
	0, 0,		/* flip_text_xoff, flip_text_yoff */
	4, 1
};

static const tc0110pcr_interface darius2_tc0110pcr_intf_l = {	0	/* pal_offs / 0x1000 */	};
static const tc0110pcr_interface darius2_tc0110pcr_intf_m = {	1	/* pal_offs / 0x1000 */	};
static const tc0110pcr_interface darius2_tc0110pcr_intf_r = {	2	/* pal_offs / 0x1000 */	};

static const tc0220ioc_interface ninjaw_io_intf =
{
	DEVCB_INPUT_PORT("DSWA"), DEVCB_INPUT_PORT("DSWB"),
	DEVCB_INPUT_PORT("IN0"), DEVCB_INPUT_PORT("IN1"), DEVCB_INPUT_PORT("IN2")	/* port read handlers */
};

static const tc0140syt_interface ninjaw_tc0140syt_intf =
{
	"maincpu", "audiocpu"
};


static STATE_POSTLOAD( ninjaw_postload )
{
	parse_control(machine);
	reset_sound_region(machine);
}

static MACHINE_START( ninjaw )
{
	ninjaw_state *state = (ninjaw_state *)machine->driver_data;

	memory_configure_bank(machine, "bank10", 0, 8, memory_region(machine, "audiocpu") + 0xc000, 0x4000);

	state->maincpu = machine->device("maincpu");
	state->audiocpu = machine->device("audiocpu");
	state->subcpu = machine->device("sub");
	state->tc0140syt = machine->device("tc0140syt");
	state->tc0100scn_1 = machine->device("tc0100scn_1");
	state->tc0100scn_2 = machine->device("tc0100scn_2");
	state->tc0100scn_3 = machine->device("tc0100scn_3");

	state->lscreen = machine->device("lscreen");
	state->mscreen = machine->device("mscreen");
	state->rscreen = machine->device("rscreen");

	state->_2610_1l = machine->device("2610.1.l");
	state->_2610_1r = machine->device("2610.1.r");
	state->_2610_2l = machine->device("2610.2.l");
	state->_2610_2r = machine->device("2610.2.r");

	state_save_register_global(machine, state->cpua_ctrl);
	state_save_register_global(machine, state->banknum);
	state_save_register_global_array(machine, state->pandata);
	state_save_register_postload(machine, ninjaw_postload, NULL);
}

static MACHINE_RESET( ninjaw )
{
	ninjaw_state *state = (ninjaw_state *)machine->driver_data;
	state->cpua_ctrl = 0xff;
	state->banknum = 0;
	memset(state->pandata, 0, sizeof(state->pandata));

	/**** mixer control enable ****/
	sound_global_enable(machine, 1);	/* mixer enabled */
}

static MACHINE_DRIVER_START( ninjaw )

	/* driver data */
	MDRV_DRIVER_DATA(ninjaw_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000,16000000/2)	/* 8 MHz ? */
	MDRV_CPU_PROGRAM_MAP(ninjaw_master_map)
	MDRV_CPU_VBLANK_INT("lscreen", irq4_line_hold)

	MDRV_CPU_ADD("audiocpu", Z80,16000000/4)	/* 16/4 MHz ? */
	MDRV_CPU_PROGRAM_MAP(ninjaw_sound_map)

	MDRV_CPU_ADD("sub", M68000,16000000/2)	/* 8 MHz ? */
	MDRV_CPU_PROGRAM_MAP(ninjaw_slave_map)
	MDRV_CPU_VBLANK_INT("lscreen", irq4_line_hold)

	MDRV_QUANTUM_TIME(HZ(6000))	/* CPU slices */

	MDRV_MACHINE_START(ninjaw)
	MDRV_MACHINE_RESET(ninjaw)

	MDRV_TC0220IOC_ADD("tc0220ioc", ninjaw_io_intf)

	/* video hardware */
	MDRV_GFXDECODE(ninjaw)
	MDRV_PALETTE_LENGTH(4096*3)
	MDRV_DEFAULT_LAYOUT(layout_darius)

	MDRV_SCREEN_ADD("lscreen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_SIZE(36*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 36*8-1, 3*8, 31*8-1)

	MDRV_SCREEN_ADD("mscreen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_SIZE(36*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 36*8-1, 3*8, 31*8-1)

	MDRV_SCREEN_ADD("rscreen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_SIZE(36*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 36*8-1, 3*8, 31*8-1)

	MDRV_VIDEO_START(ninjaw)
	MDRV_VIDEO_UPDATE(ninjaw)

	MDRV_TC0100SCN_ADD("tc0100scn_1", darius2_tc0100scn_intf_l)
	MDRV_TC0100SCN_ADD("tc0100scn_2", darius2_tc0100scn_intf_m)
	MDRV_TC0100SCN_ADD("tc0100scn_3", darius2_tc0100scn_intf_r)
	MDRV_TC0110PCR_ADD("tc0110pcr_1", darius2_tc0110pcr_intf_l)
	MDRV_TC0110PCR_ADD("tc0110pcr_2", darius2_tc0110pcr_intf_m)
	MDRV_TC0110PCR_ADD("tc0110pcr_3", darius2_tc0110pcr_intf_r)

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

//  MDRV_SOUND_ADD("subwoofer", SUBWOOFER, 0)

	MDRV_TC0140SYT_ADD("tc0140syt", ninjaw_tc0140syt_intf)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( darius2 )

	/* driver data */
	MDRV_DRIVER_DATA(ninjaw_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000,16000000/2)	/* 8 MHz ? */
	MDRV_CPU_PROGRAM_MAP(darius2_master_map)
	MDRV_CPU_VBLANK_INT("lscreen", irq4_line_hold)

	MDRV_CPU_ADD("audiocpu", Z80,16000000/4)	/* 4 MHz ? */
	MDRV_CPU_PROGRAM_MAP(ninjaw_sound_map)

	MDRV_CPU_ADD("sub", M68000,16000000/2)	/* 8 MHz ? */
	MDRV_CPU_PROGRAM_MAP(darius2_slave_map)
	MDRV_CPU_VBLANK_INT("lscreen", irq4_line_hold)

	MDRV_QUANTUM_TIME(HZ(6000))	/* CPU slices */

	MDRV_MACHINE_START(ninjaw)
	MDRV_MACHINE_RESET(ninjaw)

	MDRV_TC0220IOC_ADD("tc0220ioc", ninjaw_io_intf)

	/* video hardware */
	MDRV_GFXDECODE(ninjaw)
	MDRV_PALETTE_LENGTH(4096*3)
	MDRV_DEFAULT_LAYOUT(layout_darius)

	MDRV_SCREEN_ADD("lscreen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_SIZE(36*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 36*8-1, 3*8, 31*8-1)

	MDRV_SCREEN_ADD("mscreen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_SIZE(36*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 36*8-1, 3*8, 31*8-1)

	MDRV_SCREEN_ADD("rscreen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_SIZE(36*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 36*8-1, 3*8, 31*8-1)

	MDRV_VIDEO_START(ninjaw)
	MDRV_VIDEO_UPDATE(ninjaw)

	MDRV_TC0100SCN_ADD("tc0100scn_1", darius2_tc0100scn_intf_l)
	MDRV_TC0100SCN_ADD("tc0100scn_2", darius2_tc0100scn_intf_m)
	MDRV_TC0100SCN_ADD("tc0100scn_3", darius2_tc0100scn_intf_r)
	MDRV_TC0110PCR_ADD("tc0110pcr_1", darius2_tc0110pcr_intf_l)
	MDRV_TC0110PCR_ADD("tc0110pcr_2", darius2_tc0110pcr_intf_m)
	MDRV_TC0110PCR_ADD("tc0110pcr_3", darius2_tc0110pcr_intf_r)

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

//  MDRV_SOUND_ADD("subwoofer", SUBWOOFER, 0)

	MDRV_TC0140SYT_ADD("tc0140syt", ninjaw_tc0140syt_intf)
MACHINE_DRIVER_END


/***************************************************************************
                    DRIVERS
***************************************************************************/

ROM_START( ninjaw )
	ROM_REGION( 0xc0000, "maincpu", 0 )	/* 256K for 68000 CPUA code */
	ROM_LOAD16_BYTE( "b31-45",    0x00000, 0x10000, CRC(107902c3) SHA1(026f71a918059e3374ae262304a2ee1270f5c5bd) )
	ROM_LOAD16_BYTE( "b31-47",    0x00001, 0x10000, CRC(bd536b1e) SHA1(39c86cbb3a33fc77a0141b5648a1aca862e0a5fd) )
	ROM_LOAD16_BYTE( "b31_29.34", 0x20000, 0x10000, CRC(f2941a37) SHA1(cf1f231d9caddc903116a8b654f49181ca459697) )
	ROM_LOAD16_BYTE( "b31_27.31", 0x20001, 0x10000, CRC(2f3ff642) SHA1(7d6775b51d96b459b163d8fde2385b0e3f5242ca) )

	ROM_LOAD16_BYTE( "b31_41.5", 0x40000, 0x20000, CRC(0daef28a) SHA1(7c7e16b0eebc589ab99f62ddb98b372596ff5ae6) )	/* data roms ? */
	ROM_LOAD16_BYTE( "b31_39.2", 0x40001, 0x20000, CRC(e9197c3c) SHA1(a7f0ef2b3c4258c09edf05284fec45832a8fb147) )
	ROM_LOAD16_BYTE( "b31_40.6", 0x80000, 0x20000, CRC(2ce0f24e) SHA1(39632397ac7e8457607c32c31fccf1c08d4b2621) )
	ROM_LOAD16_BYTE( "b31_38.3", 0x80001, 0x20000, CRC(bc68cd99) SHA1(bb31ea589339c9f9b61e312e1024b5c8410cdb43) )

	ROM_REGION( 0x60000, "sub", 0 )	/* 384K for 68000 CPUB code */
	ROM_LOAD16_BYTE( "b31_33.87", 0x00000, 0x10000, CRC(6ce9af44) SHA1(486e332af238c211c3f64f7ead114282661687c4) )
	ROM_LOAD16_BYTE( "b31_36.97", 0x00001, 0x10000, CRC(ba20b0d4) SHA1(fb3dcb7681a95087afac9aa9393765d786243486) )
	ROM_LOAD16_BYTE( "b31_32.86", 0x20000, 0x10000, CRC(e6025fec) SHA1(071f83a9ddebe67bd6c6c2505318e177895163ee) )
	ROM_LOAD16_BYTE( "b31_35.96", 0x20001, 0x10000, CRC(70d9a89f) SHA1(20f846beb052fd8cddcf00c3e42e3304e102a87b) )
	ROM_LOAD16_BYTE( "b31_31.85", 0x40000, 0x10000, CRC(837f47e2) SHA1(88d596f01566456ba18a01afd0a6a7c121d3ca88) )
	ROM_LOAD16_BYTE( "b31_34.95", 0x40001, 0x10000, CRC(d6b5fb2a) SHA1(e3ae0d7ec62740465a90e4939b10341d3866d860) )

	ROM_REGION( 0x2c000, "audiocpu", 0 )	/* sound cpu */
	ROM_LOAD( "b31_37.11",  0x00000, 0x04000, CRC(0ca5799d) SHA1(6485dde076d15b69b9ee65880dda57ad4f8d129c) )
	ROM_CONTINUE(           0x10000, 0x1c000 )  /* banked stuff */

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "b31-01.23", 0x00000, 0x80000, CRC(8e8237a7) SHA1(3e181a153d9b4b7f6a620614ea9022285583a5b5) )	/* SCR (screen 1) */
	ROM_LOAD( "b31-02.24", 0x80000, 0x80000, CRC(4c3b4e33) SHA1(f99b379be1af085bf102d4d7cf35803e002fe80b) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "b31-07.176", 0x000000, 0x80000, CRC(33568cdb) SHA1(87abf56bbbd3659a1bd3e6ce9e43176be7950b41) )	/* OBJ */
	ROM_LOAD( "b31-06.175", 0x080000, 0x80000, CRC(0d59439e) SHA1(54d844492888e7fe2c3bc61afe64f8d47fdee8dc) )
	ROM_LOAD( "b31-05.174", 0x100000, 0x80000, CRC(0a1fc9fb) SHA1(a5d6975fd4f7e689c8cafd7c9cd3787797955779) )
	ROM_LOAD( "b31-04.173", 0x180000, 0x80000, CRC(2e1e4cb5) SHA1(4733cfc015a68e021108a9e1e8ea807b0e7eac7a) )

	ROM_REGION( 0x100000, "gfx3", 0 )
	ROM_COPY( "gfx1", 0x000000, 0x000000, 0x100000 )	/* SCR (screens 2+) */

/* The actual board duplicates the SCR gfx roms for 2nd/3rd TC0100SCN */
//  ROM_LOAD( "b31-01.26", 0x00000, 0x80000, CRC(8e8237a7) SHA1(3e181a153d9b4b7f6a620614ea9022285583a5b5) ) /* SCR (screen 2) */
//  ROM_LOAD( "b31-02.27", 0x80000, 0x80000, CRC(4c3b4e33) SHA1(f99b379be1af085bf102d4d7cf35803e002fe80b) )
//  ROM_LOAD( "b31-01.28", 0x00000, 0x80000, CRC(8e8237a7) SHA1(3e181a153d9b4b7f6a620614ea9022285583a5b5) ) /* SCR (screen 3) */
//  ROM_LOAD( "b31-02.29", 0x80000, 0x80000, CRC(4c3b4e33) SHA1(f99b379be1af085bf102d4d7cf35803e002fe80b) )

	ROM_REGION( 0x180000, "ymsnd", 0 )	/* ADPCM samples */
	ROM_LOAD( "b31-09.18", 0x000000, 0x80000, CRC(60a73382) SHA1(0ddeb86fcd4d19a58e62bf8564f996d17e36e5c5) )
	ROM_LOAD( "b31-10.17", 0x080000, 0x80000, CRC(c6434aef) SHA1(3348ce87882e3f668aa85bbb517975ec1fc9b6fd) )
	ROM_LOAD( "b31-11.16", 0x100000, 0x80000, CRC(8da531d4) SHA1(525dfab0a0729e9fb6f0e4c8187bf4ce16321b20) )

	ROM_REGION( 0x80000, "ymsnd.deltat", 0 )	/* Delta-T samples */
	ROM_LOAD( "b31-08.19", 0x000000, 0x80000, CRC(a0a1f87d) SHA1(6b0f8094f3a3ef1ced76984e333e22a17c51af29) )

	ROM_REGION( 0x01000, "user1", 0 )	/* unknown roms */
	ROM_LOAD( "b31-25.38", 0x00000, 0x200, CRC(a0b4ba48) SHA1(dc9a46366a0cbf63a609f177c3d3ba9675416662) )
	ROM_LOAD( "b31-26.58", 0x00000, 0x200, CRC(13e5fe15) SHA1(c973c7965954a2a0b427908f099592ed89cf0ff0) )
ROM_END

ROM_START( ninjawj )
	ROM_REGION( 0xc0000, "maincpu", 0 )	/* 256K for 68000 CPUA code */
	ROM_LOAD16_BYTE( "b31_30.35", 0x00000, 0x10000, CRC(056edd9f) SHA1(8922cede80b31ce0f7a00c8cab13d835464c6058) )
	ROM_LOAD16_BYTE( "b31_28.32", 0x00001, 0x10000, CRC(cfa7661c) SHA1(a7a6abb33a514d910e3198d5acbd4c31b2434b6c) )
	ROM_LOAD16_BYTE( "b31_29.34", 0x20000, 0x10000, CRC(f2941a37) SHA1(cf1f231d9caddc903116a8b654f49181ca459697) )
	ROM_LOAD16_BYTE( "b31_27.31", 0x20001, 0x10000, CRC(2f3ff642) SHA1(7d6775b51d96b459b163d8fde2385b0e3f5242ca) )

	ROM_LOAD16_BYTE( "b31_41.5", 0x40000, 0x20000, CRC(0daef28a) SHA1(7c7e16b0eebc589ab99f62ddb98b372596ff5ae6) )	/* data roms ? */
	ROM_LOAD16_BYTE( "b31_39.2", 0x40001, 0x20000, CRC(e9197c3c) SHA1(a7f0ef2b3c4258c09edf05284fec45832a8fb147) )
	ROM_LOAD16_BYTE( "b31_40.6", 0x80000, 0x20000, CRC(2ce0f24e) SHA1(39632397ac7e8457607c32c31fccf1c08d4b2621) )
	ROM_LOAD16_BYTE( "b31_38.3", 0x80001, 0x20000, CRC(bc68cd99) SHA1(bb31ea589339c9f9b61e312e1024b5c8410cdb43) )

	ROM_REGION( 0x60000, "sub", 0 )	/* 384K for 68000 CPUB code */
	ROM_LOAD16_BYTE( "b31_33.87", 0x00000, 0x10000, CRC(6ce9af44) SHA1(486e332af238c211c3f64f7ead114282661687c4) )
	ROM_LOAD16_BYTE( "b31_36.97", 0x00001, 0x10000, CRC(ba20b0d4) SHA1(fb3dcb7681a95087afac9aa9393765d786243486) )
	ROM_LOAD16_BYTE( "b31_32.86", 0x20000, 0x10000, CRC(e6025fec) SHA1(071f83a9ddebe67bd6c6c2505318e177895163ee) )
	ROM_LOAD16_BYTE( "b31_35.96", 0x20001, 0x10000, CRC(70d9a89f) SHA1(20f846beb052fd8cddcf00c3e42e3304e102a87b) )
	ROM_LOAD16_BYTE( "b31_31.85", 0x40000, 0x10000, CRC(837f47e2) SHA1(88d596f01566456ba18a01afd0a6a7c121d3ca88) )
	ROM_LOAD16_BYTE( "b31_34.95", 0x40001, 0x10000, CRC(d6b5fb2a) SHA1(e3ae0d7ec62740465a90e4939b10341d3866d860) )

	ROM_REGION( 0x2c000, "audiocpu", 0 )	/* sound cpu */
	ROM_LOAD( "b31_37.11",  0x00000, 0x04000, CRC(0ca5799d) SHA1(6485dde076d15b69b9ee65880dda57ad4f8d129c) )
	ROM_CONTINUE(           0x10000, 0x1c000 )  /* banked stuff */

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "b31-01.23", 0x00000, 0x80000, CRC(8e8237a7) SHA1(3e181a153d9b4b7f6a620614ea9022285583a5b5) )	/* SCR (screen 1) */
	ROM_LOAD( "b31-02.24", 0x80000, 0x80000, CRC(4c3b4e33) SHA1(f99b379be1af085bf102d4d7cf35803e002fe80b) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "b31-07.176", 0x000000, 0x80000, CRC(33568cdb) SHA1(87abf56bbbd3659a1bd3e6ce9e43176be7950b41) )	/* OBJ */
	ROM_LOAD( "b31-06.175", 0x080000, 0x80000, CRC(0d59439e) SHA1(54d844492888e7fe2c3bc61afe64f8d47fdee8dc) )
	ROM_LOAD( "b31-05.174", 0x100000, 0x80000, CRC(0a1fc9fb) SHA1(a5d6975fd4f7e689c8cafd7c9cd3787797955779) )
	ROM_LOAD( "b31-04.173", 0x180000, 0x80000, CRC(2e1e4cb5) SHA1(4733cfc015a68e021108a9e1e8ea807b0e7eac7a) )

	ROM_REGION( 0x100000, "gfx3", 0 )
	ROM_COPY( "gfx1", 0x000000, 0x000000, 0x100000 )	/* SCR (screens 2+) */

/* The actual board duplicates the SCR gfx roms for 2nd/3rd TC0100SCN */
//  ROM_LOAD( "b31-01.26", 0x00000, 0x80000, CRC(8e8237a7) SHA1(3e181a153d9b4b7f6a620614ea9022285583a5b5) ) /* SCR (screen 2) */
//  ROM_LOAD( "b31-02.27", 0x80000, 0x80000, CRC(4c3b4e33) SHA1(f99b379be1af085bf102d4d7cf35803e002fe80b) )
//  ROM_LOAD( "b31-01.28", 0x00000, 0x80000, CRC(8e8237a7) SHA1(3e181a153d9b4b7f6a620614ea9022285583a5b5) ) /* SCR (screen 3) */
//  ROM_LOAD( "b31-02.29", 0x80000, 0x80000, CRC(4c3b4e33) SHA1(f99b379be1af085bf102d4d7cf35803e002fe80b) )

	ROM_REGION( 0x180000, "ymsnd", 0 )	/* ADPCM samples */
	ROM_LOAD( "b31-09.18", 0x000000, 0x80000, CRC(60a73382) SHA1(0ddeb86fcd4d19a58e62bf8564f996d17e36e5c5) )
	ROM_LOAD( "b31-10.17", 0x080000, 0x80000, CRC(c6434aef) SHA1(3348ce87882e3f668aa85bbb517975ec1fc9b6fd) )
	ROM_LOAD( "b31-11.16", 0x100000, 0x80000, CRC(8da531d4) SHA1(525dfab0a0729e9fb6f0e4c8187bf4ce16321b20) )

	ROM_REGION( 0x80000, "ymsnd.deltat", 0 )	/* Delta-T samples */
	ROM_LOAD( "b31-08.19", 0x000000, 0x80000, CRC(a0a1f87d) SHA1(6b0f8094f3a3ef1ced76984e333e22a17c51af29) )

	ROM_REGION( 0x01000, "user1", 0 )	/* unknown roms */
	ROM_LOAD( "b31-25.38", 0x00000, 0x200, CRC(a0b4ba48) SHA1(dc9a46366a0cbf63a609f177c3d3ba9675416662) )
	ROM_LOAD( "b31-26.58", 0x00000, 0x200, CRC(13e5fe15) SHA1(c973c7965954a2a0b427908f099592ed89cf0ff0) )
ROM_END

ROM_START( darius2 )
	ROM_REGION( 0xc0000, "maincpu", 0 )	/* 256K for 68000 CPUA code */
	ROM_LOAD16_BYTE( "c07-32-1", 0x00000, 0x10000, CRC(216c8f6a) SHA1(493b0779b99a228911f56ef9d2d4a3945683bec0) )
	ROM_LOAD16_BYTE( "c07-29-1", 0x00001, 0x10000, CRC(48de567f) SHA1(cdf50052933cd2603fd4374e8bae8b30a6c690b5) )
	ROM_LOAD16_BYTE( "c07-31-1", 0x20000, 0x10000, CRC(8279d2f8) SHA1(bd3c80a024a58e4b554f4867f56d7f5741eb3031) )
	ROM_LOAD16_BYTE( "c07-30-1", 0x20001, 0x10000, CRC(6122e400) SHA1(2f68a423f9db8d69ab74453f8cef755f703cc94c) )

	ROM_LOAD16_BYTE( "c07-27",   0x40000, 0x20000, CRC(0a6f7b6c) SHA1(0ed915201fbc0bf94fdcbef8dfd021cebe87474f) )	/* data roms ? */
	ROM_LOAD16_BYTE( "c07-25",   0x40001, 0x20000, CRC(059f40ce) SHA1(b05a96580edb66221af2f222df74a020366ce3ea) )
	ROM_LOAD16_BYTE( "c07-26",   0x80000, 0x20000, CRC(1f411242) SHA1(0fca5d864c1925473d0058e4cf81ad926f56cb14) )
	ROM_LOAD16_BYTE( "c07-24",   0x80001, 0x20000, CRC(486c9c20) SHA1(9e98fcc1777f044d69cc93eda674501b3be26097) )

	ROM_REGION( 0x60000, "sub", 0 )	/* 384K for 68000 CPUB code */
	ROM_LOAD16_BYTE( "c07-35-1", 0x00000, 0x10000, CRC(dd8c4723) SHA1(e17159f894ee661a84ccd53e2d00ee78f2b46196) )
	ROM_LOAD16_BYTE( "c07-38-1", 0x00001, 0x10000, CRC(46afb85c) SHA1(a08fb9fd2bf0929a5599ab015680fa663f1d4fe6) )
	ROM_LOAD16_BYTE( "c07-34-1", 0x20000, 0x10000, CRC(296984b8) SHA1(3ba28e293c9d3ce01ee2f8ae2c2aa450fe021d30) )
	ROM_LOAD16_BYTE( "c07-37-1", 0x20001, 0x10000, CRC(8b7d461f) SHA1(c783491ca23223dc58fa7e8f408407b9a10cbce4) )
	ROM_LOAD16_BYTE( "c07-33-1", 0x40000, 0x10000, CRC(2da03a3f) SHA1(f1f2de82e0addc5e19c8935e4f5810896691118f) )
	ROM_LOAD16_BYTE( "c07-36-1", 0x40001, 0x10000, CRC(02cf2b1c) SHA1(c94a64f26f94f182cfe2b6edb37e4ce35a0f681b) )

	ROM_REGION( 0x2c000, "audiocpu", 0 )	/* sound cpu */
	ROM_LOAD( "c07-28",  0x00000, 0x04000, CRC(da304bc5) SHA1(689b4f329d9a640145f82e12dff3dd1fcf8a28c8) )
	ROM_CONTINUE(            0x10000, 0x1c000 )  /* banked stuff */

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "c07-03.12", 0x00000, 0x80000, CRC(189bafce) SHA1(d885e444523489fe24269b90dec58e0d92cfbd6e) )	/* SCR (screen 1) */
	ROM_LOAD( "c07-04.11", 0x80000, 0x80000, CRC(50421e81) SHA1(27ac420602f1dac00dc32903543a518e6f47fb2f) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "c07-01", 0x00000, 0x80000, CRC(3cf0f050) SHA1(f5a1f7e327a2617fb95ce2837e72945fd7447346) )	/* OBJ */
	ROM_LOAD( "c07-02", 0x80000, 0x80000, CRC(75d16d4b) SHA1(795423278b66eca41accce1f8a4425d65af7b629) )

	ROM_REGION( 0x100000, "gfx3", 0 )
	ROM_COPY( "gfx1", 0x000000, 0x000000, 0x100000 )	/* SCR (screens 2+) */

	ROM_REGION( 0x100000, "ymsnd", 0 )	/* ADPCM samples */
	ROM_LOAD( "c07-10.95", 0x00000, 0x80000, CRC(4bbe0ed9) SHA1(081b73c4e4d4fa548445e5548573099bcb1e9213) )
	ROM_LOAD( "c07-11.96", 0x80000, 0x80000, CRC(3c815699) SHA1(0471ff5b0c0da905267f2cee52fd68c8661cccc9) )

	ROM_REGION( 0x80000, "ymsnd.deltat", 0 )	/* Delta-T samples */
	ROM_LOAD( "c07-12.107", 0x00000, 0x80000, CRC(e0b71258) SHA1(0258e308b643d723475824752ebffc4ea29d1ac4) )
ROM_END


/* Working Games */

GAME( 1987, ninjaw,   0,      ninjaw,  ninjaw,   0,  ROT0, "Taito Corporation Japan", "The Ninja Warriors (World)", 0 )
GAME( 1987, ninjawj,  ninjaw, ninjaw,  ninjawj,  0,  ROT0, "Taito Corporation", "The Ninja Warriors (Japan)", 0 )
GAME( 1989, darius2,  0,      darius2, darius2,  0,  ROT0, "Taito Corporation", "Darius II (Japan)", 0 )
