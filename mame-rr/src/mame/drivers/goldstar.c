/***************************************************************************

  Golden Star
  Cherry Master
  Lucky 8 Lines
  Amcoe CGA games

  Golden Star and Cherry Master seem to be almost the same thing, running on
  different hardware.  There are also various bootlegs / hacks, it isn't clear
  exactly what hardware each runs on, some appear to have no OKI for example.

  Driver by Mirko Buffoni.
  Additional Work: David Haywood & Roberto Fresca.

  The vast majority of the sets in here are probably bootlegs and hacks
  hence the slightly different PCBs, rom layouts, slightly hacked program roms
  etc.

****************************************************************************

  Game notes:
  -----------

  * New Lucky 8 Lines:

  Sometimes the game boots with a "Coin Jam" message. Just reset the game to normalize.
  There are 2 sets of controls. Press the BIG key to switch between them.

  Press 9 to enter settings, press START to exit.
  Press 0 to enter stats, press START to exit.
  Keeping pressed 9 + 0 + RESET (F3), will enter the test mode. Press RESET to exit.

  New Lucky 8 Lines has two sets of controls than can be switched through each 'BIG' button.
  Even you can switch controls in middle of the game. When a set of controls are in use,
  the other set is blocked till 'BIG' button is pressed.


  * New Lucky 8 Lines / New Super 8 Lines:

  This set has a regular New Lucky 8 Lines, but allowing bets up to 64.
  It has different settings/parameters, and additional graphics for a game
  called New Super 8 Lines. There are basic reels tiles with a semi-naked woman,
  a sort of Super Mario character from Nintendo, clouds and stars...

  Still can't find how to switch between games.


  * New Lucky 8 Lines / New Super 8 Lines (Witch Bonus):

  Same as above, but allowing bets up to 32 credits.
  It also has different settings/parameters, and a rare feature: the 'Witch Bonus'
  (from Witch Card poker game) is present in the double-up.


  * Cherry Bonus III:

  If a hopper status error appear when the player try to take score,
  pressing Key Out (W) will discharge the credits won.

  Cherry Bonus III has two sets of controls than can be switched through each 'BIG' button.
  Even you can switch controls in middle of the game. When a set of controls are in use,
  the other set is blocked till 'BIG' button is pressed.

  Controls Set2 is using reels stop buttons from Controls Set1.


  * Cherry Master V4 (set 2)

  This set is supposed to be a kind of "stealth".
  The game is hidden into a Tetris game and could be triggered/switched
  in some way. Seems that it was designed for locations/countries where
  gambling games are/were not allowed.

  The game is booting as Cherry Master V4 instead of Tetris ATM...

  Even when the gambling game is working properly, the game is flagged
  as NOT_WORKING till can figure out how can switch between games.


  * Kkoj Noli

  kkuj nol-i / kkoj noli (better romanization).

  kkuj = stab/kill
  nol-i = bees

  The little red box at bottom of the title translates as "South Korea"

  Seems to be a hack of Lucky 8 Lines.

  - Child'ish graphics.
  - For Amusement only... There is no payout/keyout line accessed.
  - No stats or service mode.
  - No NVRAM.
  - Two sets of player's controls, as lucky8.
  - No ay8910, so no extra ports.
  - Only 1 DIP switches bank.

  Nominated for the *WORST* hacked gambling game EVER!


***************************************************************************/


#define MASTER_CLOCK	XTAL_12MHz
#define CPU_CLOCK		MASTER_CLOCK / 4
#define PSG_CLOCK		MASTER_CLOCK / 4
#define AY_CLOCK		MASTER_CLOCK / 8
#define OKI_CLOCK		1056000		/* unverified resonator */

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "sound/okim6295.h"
#include "sound/sn76496.h"
#include "machine/8255ppi.h"
#include "includes/goldstar.h"

#include "lucky8.lh"


static NVRAM_HANDLER( goldstar )
{
	goldstar_state *state = (goldstar_state *)machine->driver_data;

	if (read_or_write)
		mame_fwrite(file,state->nvram,state->nvram_size);
	else
	{
		if (file)
			mame_fread(file,state->nvram,state->nvram_size);
		else
			memset(state->nvram,0xff,state->nvram_size);
	}
}


static WRITE8_HANDLER( protection_w )
{
	goldstar_state *state = (goldstar_state *)space->machine->driver_data;

	if (data == 0x2a)
		state->dataoffset = 0;
}

static READ8_HANDLER( protection_r )
{
	goldstar_state *state = (goldstar_state *)space->machine->driver_data;
	static const int data[4] = { 0x47, 0x4f, 0x4c, 0x44 };

	state->dataoffset %= 4;
	return data[state->dataoffset++];
}

static ADDRESS_MAP_START( goldstar_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xb7ff) AM_ROM
	AM_RANGE(0xb800, 0xbfff) AM_RAM AM_BASE_MEMBER(goldstar_state,nvram) AM_SIZE_MEMBER(goldstar_state,nvram_size)
	AM_RANGE(0xc000, 0xc7ff) AM_ROM
	AM_RANGE(0xc800, 0xcfff) AM_RAM_WRITE( goldstar_fg_vidram_w ) AM_BASE_MEMBER(goldstar_state,fg_vidram)
	AM_RANGE(0xd000, 0xd7ff) AM_RAM_WRITE( goldstar_fg_atrram_w ) AM_BASE_MEMBER(goldstar_state,fg_atrram)
	AM_RANGE(0xd800, 0xd9ff) AM_RAM_WRITE( goldstar_reel1_ram_w ) AM_BASE_MEMBER(goldstar_state,reel1_ram)
	AM_RANGE(0xe000, 0xe1ff) AM_RAM_WRITE( goldstar_reel2_ram_w ) AM_BASE_MEMBER(goldstar_state,reel2_ram)
	AM_RANGE(0xe800, 0xe9ff) AM_RAM_WRITE( goldstar_reel3_ram_w ) AM_BASE_MEMBER(goldstar_state,reel3_ram)
	AM_RANGE(0xf040, 0xf07f) AM_RAM AM_BASE_MEMBER(goldstar_state,reel1_scroll)
	AM_RANGE(0xf080, 0xf0bf) AM_RAM AM_BASE_MEMBER(goldstar_state,reel2_scroll)
	AM_RANGE(0xf0c0, 0xf0ff) AM_RAM AM_BASE_MEMBER(goldstar_state,reel3_scroll)

	AM_RANGE(0xf800, 0xf800) AM_READ_PORT("IN0")
	AM_RANGE(0xf801, 0xf801) AM_READ_PORT("IN1")	/* Test Mode */
	AM_RANGE(0xf802, 0xf802) AM_READ_PORT("DSW1")
//  AM_RANGE(0xf803, 0xf803)
//  AM_RANGE(0xf804, 0xf804)
	AM_RANGE(0xf805, 0xf805) AM_READ_PORT("DSW4")	/* DSW 4 (also appears in 8910 port) */
	AM_RANGE(0xf806, 0xf806) AM_READ_PORT("DSW7")	/* (don't know to which one of the */
	AM_RANGE(0xf810, 0xf810) AM_READ_PORT("UNK1")
	AM_RANGE(0xf811, 0xf811) AM_READ_PORT("UNK2")
	AM_RANGE(0xf820, 0xf820) AM_READ_PORT("DSW2")
	AM_RANGE(0xf830, 0xf830) AM_DEVREADWRITE("aysnd", ay8910_r, ay8910_data_w)
	AM_RANGE(0xf840, 0xf840) AM_DEVWRITE("aysnd", ay8910_address_w)
	AM_RANGE(0xfa00, 0xfa00) AM_WRITE(goldstar_fa00_w)
	AM_RANGE(0xfb00, 0xfb00) AM_DEVREADWRITE("oki", okim6295_r, okim6295_w)
	AM_RANGE(0xfd00, 0xfdff) AM_RAM_WRITE(paletteram_BBGGGRRR_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0xfe00, 0xfe00) AM_READWRITE(protection_r,protection_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( goldstar_readport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x10, 0x10) AM_READ_PORT("DSW6")
ADDRESS_MAP_END

static WRITE8_HANDLER( ncb3_port81_w )
{
//  if (data!=0x00)
//      popmessage("ncb3_port81_w %02x\n",data);
}


static ADDRESS_MAP_START( ncb3_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xb7ff) AM_ROM
	AM_RANGE(0xb800, 0xbfff) AM_RAM AM_BASE_MEMBER(goldstar_state,nvram) AM_SIZE_MEMBER(goldstar_state,nvram_size)
	AM_RANGE(0xc000, 0xc7ff) AM_ROM
	AM_RANGE(0xc800, 0xcfff) AM_RAM_WRITE(goldstar_fg_vidram_w) AM_BASE_MEMBER(goldstar_state,fg_vidram)
	AM_RANGE(0xd000, 0xd7ff) AM_RAM_WRITE(goldstar_fg_atrram_w) AM_BASE_MEMBER(goldstar_state,fg_atrram)
	AM_RANGE(0xd800, 0xd9ff) AM_RAM_WRITE(goldstar_reel1_ram_w) AM_BASE_MEMBER(goldstar_state,reel1_ram)
	AM_RANGE(0xe000, 0xe1ff) AM_RAM_WRITE(goldstar_reel2_ram_w) AM_BASE_MEMBER(goldstar_state,reel2_ram)
	AM_RANGE(0xe800, 0xe9ff) AM_RAM_WRITE(goldstar_reel3_ram_w) AM_BASE_MEMBER(goldstar_state,reel3_ram)
	AM_RANGE(0xf040, 0xf07f) AM_RAM AM_BASE_MEMBER(goldstar_state,reel1_scroll)
	AM_RANGE(0xf080, 0xf0bf) AM_RAM AM_BASE_MEMBER(goldstar_state,reel2_scroll)
	AM_RANGE(0xf100, 0xf17f) AM_RAM AM_BASE_MEMBER(goldstar_state,reel3_scroll) // moved compared to goldstar

	AM_RANGE(0xf800, 0xf803) AM_DEVREADWRITE("ppi8255_0", ppi8255_r, ppi8255_w)	/* Input Ports */
	AM_RANGE(0xf810, 0xf813) AM_DEVREADWRITE("ppi8255_1", ppi8255_r, ppi8255_w)	/* Input Ports */
	AM_RANGE(0xf822, 0xf822) AM_WRITE(goldstar_fa00_w) // hack (connected to ppi output port?, needed for colour banking)
	AM_RANGE(0xf820, 0xf823) AM_DEVREADWRITE("ppi8255_2", ppi8255_r, ppi8255_w)	/* Input/Output Ports */

	AM_RANGE(0xf830, 0xf830) AM_DEVREADWRITE("aysnd", ay8910_r, ay8910_data_w)
	AM_RANGE(0xf840, 0xf840) AM_DEVWRITE("aysnd", ay8910_address_w)
//  AM_RANGE(0xf850, 0xf850) AM_WRITE(ncb3_p1_flip_w)   // need flip?
//  AM_RANGE(0xf860, 0xf860) AM_WRITE(ncb3_p2_flip_w)   // need flip?
	AM_RANGE(0xf870, 0xf870) AM_DEVWRITE("snsnd", sn76496_w)	/* guess... device is initialized, but doesn't seems to be used.*/
ADDRESS_MAP_END

static ADDRESS_MAP_START( ncb3_readwriteport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
//  AM_RANGE(0x00, 0x00) AM_READ(ncb3_unkread_r)    // read from 0x00 when controls set1 is used...
//  AM_RANGE(0x02, 0x02) AM_READ(ncb3_unkread_r)    // read from 0x02 when controls set2 is used...
//  AM_RANGE(0x06, 0x06) AM_READ(ncb3_unkread_r)    // unknown...
//  AM_RANGE(0x08, 0x08) AM_READ(ncb3_unkread_r)    // unknown...
	AM_RANGE(0x10, 0x10) AM_READ_PORT("DSW5")	/* confirmed for ncb3 */
	AM_RANGE(0x81, 0x81) AM_WRITE(ncb3_port81_w) // ---> large writes.

ADDRESS_MAP_END

/* ncb3 findings...

  f800-f803 = 8255_1 (ctrl=9b) ; portA, B & C (input)
  f810-f813 = 8255_2 (ctrl=9b) ; portA, B & C (input)
  f820-f823 = 8255_3 (ctrl=90) ; portA (input); ports B & C (output)
  f830      = AY8910 RW
  f840      = AY8910 ctrl
  f850      = Unknown
  f870      = PSG (init writes)


  I/O

  00 = RW  (chrygld, ncb3 in ctrl set1)
  02 = RW  (ncb3 in ctrl set2)
  06 = RW
  08 = RW
  81 =  W

  00-0f = initial seq. writes

  Controls Set1 = write to f850 (0x1a), read from 0002.
  Controls Set2 = write to f860 (0x1a), read from 0000.

  Controls Set2 is using reels stop buttons from Controls Set1.

*/




static WRITE8_HANDLER( cm_outport1_w )
{
	/* lamps? */
}

static ADDRESS_MAP_START( cm_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xcfff) AM_ROM AM_WRITENOP

	AM_RANGE(0xd000, 0xd7ff) AM_RAM AM_BASE_MEMBER(goldstar_state,nvram) AM_SIZE_MEMBER(goldstar_state,nvram_size)


	AM_RANGE(0xe000, 0xe7ff) AM_RAM_WRITE(goldstar_fg_vidram_w) AM_BASE_MEMBER(goldstar_state,fg_vidram)
	AM_RANGE(0xe800, 0xefff) AM_RAM_WRITE(goldstar_fg_atrram_w) AM_BASE_MEMBER(goldstar_state,fg_atrram)

	AM_RANGE(0xf000, 0xf1ff) AM_RAM_WRITE( goldstar_reel1_ram_w ) AM_BASE_MEMBER(goldstar_state,reel1_ram)
	AM_RANGE(0xf200, 0xf3ff) AM_RAM_WRITE( goldstar_reel2_ram_w ) AM_BASE_MEMBER(goldstar_state,reel2_ram)
	AM_RANGE(0xf400, 0xf5ff) AM_RAM_WRITE( goldstar_reel3_ram_w ) AM_BASE_MEMBER(goldstar_state,reel3_ram)
	AM_RANGE(0xf600, 0xf7ff) AM_RAM

	AM_RANGE(0xf800, 0xf87f) AM_RAM AM_BASE_MEMBER(goldstar_state,reel1_scroll)
	AM_RANGE(0xf880, 0xf9ff) AM_RAM
	AM_RANGE(0xfa00, 0xfa7f) AM_RAM AM_BASE_MEMBER(goldstar_state,reel2_scroll)
	AM_RANGE(0xfa80, 0xfbff) AM_RAM
	AM_RANGE(0xfc00, 0xfc7f) AM_RAM AM_BASE_MEMBER(goldstar_state,reel3_scroll)
	AM_RANGE(0xfc80, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( nfm_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xd7ff) AM_ROM AM_WRITENOP

	AM_RANGE(0xd800, 0xdfff) AM_RAM AM_BASE_MEMBER(goldstar_state,nvram) AM_SIZE_MEMBER(goldstar_state,nvram_size)


	AM_RANGE(0xe000, 0xe7ff) AM_RAM_WRITE(goldstar_fg_vidram_w) AM_BASE_MEMBER(goldstar_state,fg_vidram)
	AM_RANGE(0xe800, 0xefff) AM_RAM_WRITE(goldstar_fg_atrram_w) AM_BASE_MEMBER(goldstar_state,fg_atrram)

	AM_RANGE(0xf000, 0xf1ff) AM_RAM_WRITE( goldstar_reel1_ram_w ) AM_BASE_MEMBER(goldstar_state,reel1_ram)
	AM_RANGE(0xf200, 0xf3ff) AM_RAM_WRITE( goldstar_reel2_ram_w ) AM_BASE_MEMBER(goldstar_state,reel2_ram)
	AM_RANGE(0xf400, 0xf5ff) AM_RAM_WRITE( goldstar_reel3_ram_w ) AM_BASE_MEMBER(goldstar_state,reel3_ram)
	AM_RANGE(0xf600, 0xf7ff) AM_RAM

	AM_RANGE(0xf800, 0xf87f) AM_RAM AM_BASE_MEMBER(goldstar_state,reel1_scroll)
	AM_RANGE(0xf880, 0xf9ff) AM_RAM
	AM_RANGE(0xfa00, 0xfa7f) AM_RAM AM_BASE_MEMBER(goldstar_state,reel2_scroll)
	AM_RANGE(0xfa80, 0xfbff) AM_RAM
	AM_RANGE(0xfc00, 0xfc7f) AM_RAM AM_BASE_MEMBER(goldstar_state,reel3_scroll)
	AM_RANGE(0xfc80, 0xffff) AM_RAM
ADDRESS_MAP_END


static ADDRESS_MAP_START( cm_portmap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x01, 0x01) AM_DEVREAD("aysnd", ay8910_r)
	AM_RANGE(0x02, 0x03) AM_DEVWRITE("aysnd", ay8910_data_address_w)
	AM_RANGE(0x04, 0x07) AM_DEVREADWRITE("ppi8255_0", ppi8255_r, ppi8255_w)	/* Input Ports */
	AM_RANGE(0x08, 0x0b) AM_DEVREADWRITE("ppi8255_1", ppi8255_r, ppi8255_w)	/* DIP switches */
	AM_RANGE(0x10, 0x10) AM_WRITE (cm_outport0_w)	/* output port */
	AM_RANGE(0x11, 0x11) AM_WRITENOP
	AM_RANGE(0x12, 0x12) AM_WRITE (cm_outport1_w)	/* output port */
	AM_RANGE(0x13, 0x13) AM_WRITE(cm_background_col_w)
	AM_RANGE(0x14, 0x14) AM_WRITE(cm_girl_scroll_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( cmast91_portmap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("ppi8255_0", ppi8255_r, ppi8255_w)	/* Input Ports */
	AM_RANGE(0x10, 0x13) AM_DEVREADWRITE("ppi8255_1", ppi8255_r, ppi8255_w)	/* DIP switches */
	AM_RANGE(0x21, 0x21) AM_DEVREAD("aysnd", ay8910_r)
	AM_RANGE(0x22, 0x23) AM_DEVWRITE("aysnd", ay8910_data_address_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( amcoe1_portmap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x01, 0x01) AM_DEVREAD("aysnd", ay8910_r)
	AM_RANGE(0x02, 0x03) AM_DEVWRITE("aysnd", ay8910_data_address_w)
	AM_RANGE(0x04, 0x07) AM_DEVREADWRITE("ppi8255_0", ppi8255_r, ppi8255_w)	/* Input Ports */
	AM_RANGE(0x08, 0x0b) AM_DEVREADWRITE("ppi8255_1", ppi8255_r, ppi8255_w)	/* DIP switches */
	AM_RANGE(0x10, 0x10) AM_WRITE (cm_outport0_w)	/* output port */
	AM_RANGE(0x11, 0x11) AM_WRITENOP
	AM_RANGE(0x12, 0x12) AM_WRITE (cm_outport1_w)	/* output port */
	AM_RANGE(0x13, 0x13) AM_WRITE(cm_background_col_w)
	AM_RANGE(0x20, 0x20) AM_DEVREADWRITE("oki", okim6295_r, okim6295_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( amcoe2_portmap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x01, 0x01) AM_DEVREAD("aysnd", ay8910_r)
	AM_RANGE(0x02, 0x03) AM_DEVWRITE("aysnd", ay8910_data_address_w)
	AM_RANGE(0x04, 0x07) AM_DEVREADWRITE("ppi8255_0", ppi8255_r, ppi8255_w)	/* Input Ports */
	AM_RANGE(0x08, 0x0b) AM_DEVREADWRITE("ppi8255_1", ppi8255_r, ppi8255_w)	/* DIP switches */
	AM_RANGE(0x10, 0x10) AM_WRITE (cm_outport0_w)	/* output port */
	AM_RANGE(0x11, 0x11) AM_WRITENOP
	AM_RANGE(0x12, 0x12) AM_WRITE (cm_outport1_w)	/* output port */
	AM_RANGE(0x13, 0x13) AM_WRITE(cm_background_col_w)
ADDRESS_MAP_END


static WRITE8_HANDLER( lucky8_outport_w )
{
	/* lamps */
	output_set_lamp_value(0, (data >> 1) & 1);	/* D-UP Lamp */
	output_set_lamp_value(1, (data >> 2) & 1);	/* TAKE Lamp */
	output_set_lamp_value(2, (data >> 3) & 1);	/* BET Lamp */
	output_set_lamp_value(3, (data >> 4) & 1);	/* INFO Lamp */
	output_set_lamp_value(4, (data >> 5) & 1);	/* START Lamp */

//  popmessage("lucky8_outb850_w %02x\n", data);

}

static ADDRESS_MAP_START( lucky8_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM AM_BASE_MEMBER(goldstar_state,nvram) AM_SIZE_MEMBER(goldstar_state,nvram_size)
	AM_RANGE(0x8800, 0x8fff) AM_RAM_WRITE(goldstar_fg_vidram_w) AM_BASE_MEMBER(goldstar_state,fg_vidram)
	AM_RANGE(0x9000, 0x97ff) AM_RAM_WRITE(goldstar_fg_atrram_w) AM_BASE_MEMBER(goldstar_state,fg_atrram)
	AM_RANGE(0x9800, 0x99ff) AM_RAM_WRITE(goldstar_reel1_ram_w) AM_BASE_MEMBER(goldstar_state,reel1_ram)
	AM_RANGE(0xa000, 0xa1ff) AM_RAM_WRITE(goldstar_reel2_ram_w) AM_BASE_MEMBER(goldstar_state,reel2_ram)
	AM_RANGE(0xa800, 0xa9ff) AM_RAM_WRITE(goldstar_reel3_ram_w) AM_BASE_MEMBER(goldstar_state,reel3_ram)
	AM_RANGE(0xb040, 0xb07f) AM_RAM AM_BASE_MEMBER(goldstar_state,reel1_scroll)
	AM_RANGE(0xb080, 0xb0bf) AM_RAM AM_BASE_MEMBER(goldstar_state,reel2_scroll)
	AM_RANGE(0xb100, 0xb17f) AM_RAM AM_BASE_MEMBER(goldstar_state,reel3_scroll)

	AM_RANGE(0xb800, 0xb803) AM_DEVREADWRITE("ppi8255_0", ppi8255_r, ppi8255_w)	/* Input Ports */
	AM_RANGE(0xb810, 0xb813) AM_DEVREADWRITE("ppi8255_1", ppi8255_r, ppi8255_w)	/* Input Ports */
	AM_RANGE(0xb820, 0xb823) AM_DEVREADWRITE("ppi8255_2", ppi8255_r, ppi8255_w)	/* Input/Output Ports */
	AM_RANGE(0xb830, 0xb830) AM_DEVREADWRITE("aysnd", ay8910_r, ay8910_data_w)
	AM_RANGE(0xb840, 0xb840) AM_DEVWRITE("aysnd", ay8910_address_w)	/* no sound... only use both ports for DSWs */
	AM_RANGE(0xb850, 0xb850) AM_WRITE(lucky8_outport_w)
	AM_RANGE(0xb870, 0xb870) AM_DEVWRITE("snsnd", sn76496_w)	/* sound */
	AM_RANGE(0xf800, 0xffff) AM_RAM
ADDRESS_MAP_END

static WRITE8_HANDLER( magical_outb850_w )
{
	// guess, could be wrong, this might just be lights

	goldstar_state *state = (goldstar_state *)space->machine->driver_data;

	if (data&0x20)
		state->tile_bank = 1;
	else
		state->tile_bank = 0;

	//popmessage("magical_outb850_w %02x\n", data);

	tilemap_mark_all_tiles_dirty (state->fg_tilemap);

}

static WRITE8_HANDLER( magical_outb860_w )
{
//  popmessage("magical_outb860_w %02x\n", data);
}

static ADDRESS_MAP_START(magical_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	// where does the extra rom data map?? it seems like it should come straight after the existing rom, but it can't if this is a plain z80?
	AM_RANGE(0x8000, 0x87ff) AM_RAM AM_SHARE("share1") AM_BASE_MEMBER(goldstar_state,nvram) AM_SIZE_MEMBER(goldstar_state,nvram_size)
	AM_RANGE(0x8800, 0x8fff) AM_RAM_WRITE(goldstar_fg_vidram_w) AM_BASE_MEMBER(goldstar_state,fg_vidram)
	AM_RANGE(0x9000, 0x97ff) AM_RAM_WRITE(goldstar_fg_atrram_w) AM_BASE_MEMBER(goldstar_state,fg_atrram)
	AM_RANGE(0x9800, 0x99ff) AM_RAM_WRITE(goldstar_reel1_ram_w) AM_BASE_MEMBER(goldstar_state,reel1_ram)
	AM_RANGE(0xa000, 0xa1ff) AM_RAM_WRITE(goldstar_reel2_ram_w) AM_BASE_MEMBER(goldstar_state,reel2_ram)
	AM_RANGE(0xa900, 0xaaff) AM_RAM_WRITE(goldstar_reel3_ram_w) AM_BASE_MEMBER(goldstar_state,reel3_ram) // +0x100 compared to lucky8
	AM_RANGE(0xb040, 0xb07f) AM_RAM AM_BASE_MEMBER(goldstar_state,reel1_scroll)
	AM_RANGE(0xb080, 0xb0bf) AM_RAM AM_BASE_MEMBER(goldstar_state,reel2_scroll)
	AM_RANGE(0xb100, 0xb17f) AM_RAM AM_BASE_MEMBER(goldstar_state,reel3_scroll)

	AM_RANGE(0xb800, 0xb803) AM_DEVREADWRITE("ppi8255_0", ppi8255_r, ppi8255_w)	/* Input Ports */
	AM_RANGE(0xb810, 0xb813) AM_DEVREADWRITE("ppi8255_1", ppi8255_r, ppi8255_w)	/* Input Ports */
	AM_RANGE(0xb820, 0xb823) AM_DEVREADWRITE("ppi8255_2", ppi8255_r, ppi8255_w)	/* Input/Output Ports */
	AM_RANGE(0xb830, 0xb830) AM_DEVREADWRITE("aysnd", ay8910_r, ay8910_data_w)
	AM_RANGE(0xb840, 0xb840) AM_DEVWRITE("aysnd", ay8910_address_w)	/* no sound... only use both ports for DSWs */
	AM_RANGE(0xb850, 0xb850) AM_WRITE(magical_outb850_w) //lamps
	AM_RANGE(0xb860, 0xb860) AM_WRITE(magical_outb860_w) //watchdog
	AM_RANGE(0xb870, 0xb870) AM_DEVWRITE("snsnd", sn76496_w)	/* sound */
	AM_RANGE(0xc000, 0xffff) AM_ROM AM_REGION("maincpu",0xc000)
ADDRESS_MAP_END

static ADDRESS_MAP_START( kkojnoli_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM	/* definitely no NVRAM */
	AM_RANGE(0x8800, 0x8fff) AM_RAM_WRITE(goldstar_fg_vidram_w) AM_BASE_MEMBER(goldstar_state,fg_vidram)
	AM_RANGE(0x9000, 0x97ff) AM_RAM_WRITE(goldstar_fg_atrram_w) AM_BASE_MEMBER(goldstar_state,fg_atrram)
	AM_RANGE(0x9800, 0x99ff) AM_RAM_WRITE(goldstar_reel1_ram_w) AM_BASE_MEMBER(goldstar_state,reel1_ram)
	AM_RANGE(0xa000, 0xa1ff) AM_RAM_WRITE(goldstar_reel2_ram_w) AM_BASE_MEMBER(goldstar_state,reel2_ram)
	AM_RANGE(0xa800, 0xa9ff) AM_RAM_WRITE(goldstar_reel3_ram_w) AM_BASE_MEMBER(goldstar_state,reel3_ram)
	AM_RANGE(0xb040, 0xb07f) AM_RAM AM_BASE_MEMBER(goldstar_state,reel1_scroll)
	AM_RANGE(0xb080, 0xb0bf) AM_RAM AM_BASE_MEMBER(goldstar_state,reel2_scroll)
	AM_RANGE(0xb100, 0xb17f) AM_RAM AM_BASE_MEMBER(goldstar_state,reel3_scroll)

	AM_RANGE(0xb800, 0xb803) AM_DEVREADWRITE("ppi8255_0", ppi8255_r, ppi8255_w)	/* Input Ports */
	AM_RANGE(0xb810, 0xb813) AM_DEVREADWRITE("ppi8255_1", ppi8255_r, ppi8255_w)	/* Input Ports */
	AM_RANGE(0xb820, 0xb823) AM_DEVREADWRITE("ppi8255_2", ppi8255_r, ppi8255_w)	/* Input Port */
	AM_RANGE(0xb830, 0xb830) AM_WRITENOP		/* no ay8910 */
	AM_RANGE(0xb840, 0xb840) AM_WRITENOP		/* no ay8910 */
	AM_RANGE(0xb850, 0xb850) AM_WRITE(lucky8_outport_w)
	AM_RANGE(0xb870, 0xb870) AM_DEVWRITE("snsnd", sn76496_w)	/* sound */
	AM_RANGE(0xf800, 0xffff) AM_RAM
ADDRESS_MAP_END


//static WRITE8_HANDLER( ladylinr_outport_w )
//{
/* LAMPS (b840)...

   .... ...x
   .... ..x.
   .... .x..
   .... x...  BET
   ...x ....  SMALL/INFO
   ..x. ....  START
   .x.. ....
   x... ....
*/
//  popmessage("Output: %02X", data);
//}

static ADDRESS_MAP_START( ladylinr_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM AM_BASE_MEMBER(goldstar_state,nvram) AM_SIZE_MEMBER(goldstar_state,nvram_size)
	AM_RANGE(0x8800, 0x8fff) AM_RAM_WRITE(goldstar_fg_vidram_w) AM_BASE_MEMBER(goldstar_state,fg_vidram)
	AM_RANGE(0x9000, 0x97ff) AM_RAM_WRITE(goldstar_fg_atrram_w) AM_BASE_MEMBER(goldstar_state,fg_atrram)
	AM_RANGE(0x9800, 0x99ff) AM_RAM_WRITE(goldstar_reel1_ram_w) AM_BASE_MEMBER(goldstar_state,reel1_ram)
	AM_RANGE(0xa000, 0xa1ff) AM_RAM_WRITE(goldstar_reel2_ram_w) AM_BASE_MEMBER(goldstar_state,reel2_ram)
	AM_RANGE(0xa800, 0xa9ff) AM_RAM_WRITE(goldstar_reel3_ram_w) AM_BASE_MEMBER(goldstar_state,reel3_ram)
	AM_RANGE(0xb040, 0xb07f) AM_RAM AM_BASE_MEMBER(goldstar_state,reel1_scroll)
	AM_RANGE(0xb080, 0xb0bf) AM_RAM AM_BASE_MEMBER(goldstar_state,reel2_scroll)
	AM_RANGE(0xb100, 0xb17f) AM_RAM AM_BASE_MEMBER(goldstar_state,reel3_scroll)

	AM_RANGE(0xb800, 0xb803) AM_DEVREADWRITE("ppi8255_0", ppi8255_r, ppi8255_w)	/* Input Ports */
	AM_RANGE(0xb810, 0xb813) AM_DEVREADWRITE("ppi8255_1", ppi8255_r, ppi8255_w)	/* DSW bank */
	AM_RANGE(0xb830, 0xb830) AM_DEVREADWRITE("aysnd", ay8910_r, ay8910_data_w)
	AM_RANGE(0xb840, 0xb840) AM_DEVWRITE("aysnd", ay8910_address_w)	/* no sound... only use ports */
	AM_RANGE(0xb850, 0xb850) AM_WRITENOP	/* just turn off the lamps, if exist */
	AM_RANGE(0xb870, 0xb870) AM_DEVWRITE("snsnd", sn76496_w)	/* sound */
	AM_RANGE(0xf800, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( wcat3_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM AM_BASE_MEMBER(goldstar_state,nvram) AM_SIZE_MEMBER(goldstar_state,nvram_size)
	AM_RANGE(0x8800, 0x8fff) AM_RAM_WRITE(goldstar_fg_vidram_w) AM_BASE_MEMBER(goldstar_state,fg_vidram)
	AM_RANGE(0x9000, 0x97ff) AM_RAM_WRITE(goldstar_fg_atrram_w) AM_BASE_MEMBER(goldstar_state,fg_atrram)
	AM_RANGE(0x9800, 0x99ff) AM_RAM_WRITE(goldstar_reel1_ram_w) AM_BASE_MEMBER(goldstar_state,reel1_ram)
	AM_RANGE(0xa000, 0xa1ff) AM_RAM_WRITE(goldstar_reel2_ram_w) AM_BASE_MEMBER(goldstar_state,reel2_ram)
	AM_RANGE(0xa800, 0xa9ff) AM_RAM_WRITE(goldstar_reel3_ram_w) AM_BASE_MEMBER(goldstar_state,reel3_ram)
	AM_RANGE(0xb040, 0xb07f) AM_RAM AM_BASE_MEMBER(goldstar_state,reel1_scroll)
	AM_RANGE(0xb080, 0xb0bf) AM_RAM AM_BASE_MEMBER(goldstar_state,reel2_scroll)
	AM_RANGE(0xb100, 0xb17f) AM_RAM AM_BASE_MEMBER(goldstar_state,reel3_scroll)

	AM_RANGE(0xb800, 0xb803) AM_DEVREADWRITE("ppi8255_0", ppi8255_r, ppi8255_w)	/* Input Ports */
	AM_RANGE(0xb810, 0xb813) AM_DEVREADWRITE("ppi8255_1", ppi8255_r, ppi8255_w)	/* Input Ports */
	AM_RANGE(0xb820, 0xb823) AM_DEVREADWRITE("ppi8255_2", ppi8255_r, ppi8255_w)	/* Input/Output Ports */
	AM_RANGE(0xb830, 0xb830) AM_DEVREADWRITE("aysnd", ay8910_r, ay8910_data_w)
	AM_RANGE(0xb840, 0xb840) AM_DEVWRITE("aysnd", ay8910_address_w)	/* no sound... only use both ports for DSWs */
	AM_RANGE(0xb850, 0xb850) AM_WRITE(lucky8_outport_w)
	AM_RANGE(0xb870, 0xb870) AM_DEVWRITE("snsnd", sn76496_w)	/* sound */
//  AM_RANGE(0xc000, 0xc003) AM_DEVREADWRITE("ppi8255_3", ppi8255_r, ppi8255_w) /* Other PPI initialized? */
	AM_RANGE(0xd000, 0xefff) AM_ROM
	AM_RANGE(0xf000, 0xffff) AM_RAM
ADDRESS_MAP_END


/*
static READ8_HANDLER( unkch_unk_r )
{
    return 0xff;
}
*/


/* newer / more capable hw */
static ADDRESS_MAP_START( unkch_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x9fff) AM_ROM
	AM_RANGE(0xc000, 0xc1ff) AM_RAM_WRITE(paletteram_xBBBBBGGGGGRRRRR_split1_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0xc800, 0xc9ff) AM_RAM_WRITE(paletteram_xBBBBBGGGGGRRRRR_split2_w) AM_BASE_GENERIC(paletteram2)

	AM_RANGE(0xd000, 0xd7ff) AM_RAM AM_BASE_MEMBER(goldstar_state,nvram) AM_SIZE_MEMBER(goldstar_state,nvram_size)

	AM_RANGE(0xd840, 0xd87f) AM_RAM AM_BASE_MEMBER(goldstar_state,reel1_scroll)
	AM_RANGE(0xd880, 0xd8bf) AM_RAM AM_BASE_MEMBER(goldstar_state,reel2_scroll)
	AM_RANGE(0xd900, 0xd93f) AM_RAM AM_BASE_MEMBER(goldstar_state,reel3_scroll)
	AM_RANGE(0xdfc0, 0xdfff) AM_RAM

	AM_RANGE(0xe000, 0xe7ff) AM_RAM_WRITE(goldstar_fg_vidram_w) AM_BASE_MEMBER(goldstar_state,fg_vidram)
	AM_RANGE(0xe800, 0xefff) AM_RAM_WRITE(goldstar_fg_atrram_w) AM_BASE_MEMBER(goldstar_state,fg_atrram)

	AM_RANGE(0xf000, 0xf1ff) AM_RAM_WRITE( goldstar_reel1_ram_w ) AM_BASE_MEMBER(goldstar_state,reel1_ram)
	AM_RANGE(0xf200, 0xf3ff) AM_RAM_WRITE( goldstar_reel2_ram_w ) AM_BASE_MEMBER(goldstar_state,reel2_ram)
	AM_RANGE(0xf400, 0xf5ff) AM_RAM_WRITE( goldstar_reel3_ram_w ) AM_BASE_MEMBER(goldstar_state,reel3_ram)
	AM_RANGE(0xf600, 0xf7ff) AM_RAM
	AM_RANGE(0xf800, 0xf9ff) AM_RAM_WRITE( unkch_reel1_attrram_w ) AM_BASE_MEMBER(goldstar_state,reel1_attrram)
	AM_RANGE(0xfa00, 0xfbff) AM_RAM_WRITE( unkch_reel2_attrram_w ) AM_BASE_MEMBER(goldstar_state,reel2_attrram)
	AM_RANGE(0xfc00, 0xfdff) AM_RAM_WRITE( unkch_reel3_attrram_w ) AM_BASE_MEMBER(goldstar_state,reel3_attrram)
	AM_RANGE(0xfe00, 0xffff) AM_RAM
ADDRESS_MAP_END

static WRITE8_HANDLER( unkcm_0x02_w )
{
	//popmessage("unkcm_0x02_w %02x", data);
}

static WRITE8_HANDLER( unkcm_0x03_w )
{
	//popmessage("unkcm_0x03_w %02x", data);
	goldstar_state *state = (goldstar_state *)space->machine->driver_data;

	state->unkch_vidreg = data;

	// -x-- ----   seems to toggle when a 'normal' tilemap should be displayed instead of the reels?
}


static WRITE8_HANDLER( unkcm_0x11_w )
{
	//popmessage("unkcm_0x11_w %02x", data);
}

static WRITE8_HANDLER( unkcm_0x12_w )
{
//  popmessage("unkcm_0x12_w %02x", data);
}


static ADDRESS_MAP_START( unkch_portmap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
//  AM_RANGE(0x01, 0x01) AM_DEVREAD("aysnd", ay8910_r)
	AM_RANGE(0x02, 0x02) AM_WRITE(unkcm_0x02_w)
	AM_RANGE(0x03, 0x03) AM_WRITE(unkcm_0x03_w)
	AM_RANGE(0x11, 0x11) AM_WRITE(unkcm_0x11_w)
	AM_RANGE(0x12, 0x12) AM_WRITE(unkcm_0x12_w)

//  AM_RANGE(0x04, 0x07) AM_DEVREADWRITE("ppi8255_0", ppi8255_r, ppi8255_w) /* Input Ports */
//  AM_RANGE(0x08, 0x0b) AM_DEVREADWRITE("ppi8255_1", ppi8255_r, ppi8255_w) /* DIP switches */
//  AM_RANGE(0x10, 0x10) AM_WRITE (cm_outport0_w)   /* output port */
//  AM_RANGE(0x11, 0x11) AM_WRITENOP
//  AM_RANGE(0x12, 0x12) AM_WRITE (cm_outport1_w)   /* output port */
//  AM_RANGE(0x13, 0x13) AM_WRITE(cm_background_col_w)
//  AM_RANGE(0x14, 0x14) AM_WRITE(cm_girl_scroll_w)

	AM_RANGE(0x08, 0x08) AM_READ_PORT("IN0")
	AM_RANGE(0x09, 0x09) AM_READ_PORT("IN1")
	AM_RANGE(0x0a, 0x0a) AM_READ_PORT("IN2")
	AM_RANGE(0x0b, 0x0b) AM_READ_PORT("IN3")
	AM_RANGE(0x10, 0x10) AM_READ_PORT("IN4")

ADDRESS_MAP_END



static INPUT_PORTS_START( cmv801 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SLOT_STOP2 ) PORT_NAME("Stop 2 / Big")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SLOT_STOP1 ) PORT_NAME("Stop 1 / D-UP")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SLOT_STOP_ALL ) PORT_NAME("Stop All / Take")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SLOT_STOP3 ) PORT_NAME("Stop 3 / Small / Info")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)	/* Coin B */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_IMPULSE(2)	/* Coin D */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2)	/* Coin C */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)	/* Coin A */

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Out / Attendant")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Settings")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Stats")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:1")	/* OK */
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Hopper Out Switch" )	PORT_DIPLOCATION("DSW1:2")	/* OK */
	PORT_DIPSETTING(    0x02, "Active Low" )
	PORT_DIPSETTING(    0x00, "Active High" )
	PORT_DIPNAME( 0x04, 0x04, "Payout Mode" )		PORT_DIPLOCATION("DSW1:3")	/* OK */
	PORT_DIPSETTING(    0x04, "Payout Switch" )
	PORT_DIPSETTING(    0x00, "Automatic" )
	PORT_DIPNAME( 0x08, 0x00, "W-UP '7'" )			PORT_DIPLOCATION("DSW1:4")	/* not checked */
	PORT_DIPSETTING(    0x08, "Loss" )
	PORT_DIPSETTING(    0x00, "Even" )
	PORT_DIPNAME( 0x10, 0x00, "W-UP Pay Rate" )		PORT_DIPLOCATION("DSW1:5")	/* OK */
	PORT_DIPSETTING(    0x00, "80%" )
	PORT_DIPSETTING(    0x10, "90%" )
	PORT_DIPNAME( 0x20, 0x00, "W-UP Game" )			PORT_DIPLOCATION("DSW1:6")	/* OK */
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Bet Max" )			PORT_DIPLOCATION("DSW1:7,8")	/* OK */
	PORT_DIPSETTING(    0x00, "16" )
	PORT_DIPSETTING(    0x40, "32" )
	PORT_DIPSETTING(    0x80, "64" )
	PORT_DIPSETTING(    0xc0, "96" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x00, "Main Game Pay Rate" )	PORT_DIPLOCATION("DSW2:1,2,3")	/* OK */
	PORT_DIPSETTING(    0x07, "35%" )
	PORT_DIPSETTING(    0x06, "40%" )
	PORT_DIPSETTING(    0x05, "45%" )
	PORT_DIPSETTING(    0x04, "50%" )
	PORT_DIPSETTING(    0x03, "55%" )
	PORT_DIPSETTING(    0x02, "60%" )
	PORT_DIPSETTING(    0x01, "65%" )
	PORT_DIPSETTING(    0x00, "70%" )
	PORT_DIPNAME( 0x18, 0x00, "Hopper Limit" )			PORT_DIPLOCATION("DSW2:4,5")	/* OK */
	PORT_DIPSETTING(    0x18, "300" )
	PORT_DIPSETTING(    0x10, "500" )
	PORT_DIPSETTING(    0x08, "1000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x20, 0x00, "100 Odds Sound" )		PORT_DIPLOCATION("DSW2:6")	/* not checked */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Key-In Type" )			PORT_DIPLOCATION("DSW2:7")	/* OK */
	PORT_DIPSETTING(    0x40, "A-Type" )
	PORT_DIPSETTING(    0x00, "B-Type" )
	PORT_DIPNAME( 0x80, 0x00, "Center Super 7 Bet Limit" )	PORT_DIPLOCATION("DSW2:8")	/* related with DSW 4-6 */
	PORT_DIPSETTING(    0x80, "Unlimited" )
	PORT_DIPSETTING(    0x00, "Limited" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, "Key In Rate" ) PORT_DIPLOCATION("DSW3:1,2")	/* OK */
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40) /* A-Type */
	PORT_DIPSETTING(    0x01, "1 Coin/20 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40)
	PORT_DIPSETTING(    0x02, "1 Coin/50 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40)
	PORT_DIPSETTING(    0x03, "1 Coin/100 Credits" ) PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )     PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00) /* B-Type */
	PORT_DIPSETTING(    0x01, "1 Coin/10 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x02, "1 Coin/25 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x03, "1 Coin/50 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00)
	PORT_DIPNAME( 0x0c, 0x0c, "Coin A Rate" ) PORT_DIPLOCATION("DSW3:3,4")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0c, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x30, 0x30, "Coin D Rate" ) PORT_DIPLOCATION("DSW3:5,6")	/* OK */
	PORT_DIPSETTING(    0x30, DEF_STR( 5C_1C ) )    PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10) /* C-Type */
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )    PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )    PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )    PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )    PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00) /* D-Type */
	PORT_DIPSETTING(    0x10, "1 Coin/10 Credits" ) PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x20, "1 Coin/25 Credits" ) PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x30, "1 Coin/50 Credits" ) PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPNAME( 0xc0, 0xc0, "Coin C Rate" ) PORT_DIPLOCATION("DSW3:7,8")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xc0, "1 Coin/10 Credits" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x07, 0x07, "Credit Limit" )            PORT_DIPLOCATION("DSW4:1,2,3")	/* not checked */
	PORT_DIPSETTING(    0x07, "5,000" )
	PORT_DIPSETTING(    0x06, "10,000" )
	PORT_DIPSETTING(    0x05, "20,000" )
	PORT_DIPSETTING(    0x04, "30,000" )
	PORT_DIPSETTING(    0x03, "40,000" )
	PORT_DIPSETTING(    0x02, "50,000" )
	PORT_DIPSETTING(    0x01, "100,000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x08, 0x08, "Display Of Payout Limit" ) PORT_DIPLOCATION("DSW4:4") /* not working */
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Type Of Coin D" )          PORT_DIPLOCATION("DSW4:5")	/* OK */
	PORT_DIPSETTING(    0x10, "C-Type" )
	PORT_DIPSETTING(    0x00, "D-Type" )
	PORT_DIPNAME( 0x20, 0x20, "Min. Bet For Bonus Play" ) PORT_DIPLOCATION("DSW4:6")	/* OK */
	PORT_DIPSETTING(    0x20, "16 Bet" )
	PORT_DIPSETTING(    0x00, "8 Bet" )
	PORT_DIPNAME( 0x40, 0x40, "Reel Speed" )              PORT_DIPLOCATION("DSW4:7")	/* OK */
	PORT_DIPSETTING(    0x40, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x80, 0x80, "Hopper Out By Coin A" )    PORT_DIPLOCATION("DSW4:8")	/* not checked */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x01, 0x00, "Display Of Doll On Demo" )          PORT_DIPLOCATION("DSW5:1")	/* not working */
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, "Coin In Limit" )                    PORT_DIPLOCATION("DSW5:2,3")	/* not checked */
	PORT_DIPSETTING(    0x06, "1,000" )
	PORT_DIPSETTING(    0x04, "5,000" )
	PORT_DIPSETTING(    0x02, "10,000" )
	PORT_DIPSETTING(    0x00, "20,000" )
	PORT_DIPNAME( 0x18, 0x18, "Condition For 3 Kind Of Bonus" )    PORT_DIPLOCATION("DSW5:4,5")	/* not checked */
	PORT_DIPSETTING(    0x18, "12-7-1" )
	PORT_DIPSETTING(    0x10, "9-5-1" )
	PORT_DIPSETTING(    0x08, "6-3-1" )
	PORT_DIPSETTING(    0x00, "3-2-1" )
	PORT_DIPNAME( 0x20, 0x00, "Display Of Doll At All Fr. Bonus" ) PORT_DIPLOCATION("DSW5:6")	/* not checked */
	PORT_DIPSETTING(    0x20, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )                 PORT_DIPLOCATION("DSW5:7")	/* listed as unused */
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Test Mode For Disp. Of Doll" )      PORT_DIPLOCATION("DSW5:8")	/* not working */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( cmv4 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SLOT_STOP2 ) PORT_NAME("Stop 2 / Big")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SLOT_STOP1 ) PORT_NAME("Stop 1 / D-UP")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SLOT_STOP_ALL ) PORT_NAME("Stop All / Take")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SLOT_STOP3 ) PORT_NAME("Stop 3 / Small / Info")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)	/* Coin B */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_IMPULSE(2)	/* Coin D */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2)	/* Coin C */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)	/* Coin A */

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Out / Attendant")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Settings")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Stats")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:1")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Hopper Out Switch" )	PORT_DIPLOCATION("DSW1:2")	/* OK */
	PORT_DIPSETTING(    0x00, "Active Low" )
	PORT_DIPSETTING(    0x02, "Active High" )
	PORT_DIPNAME( 0x04, 0x00, "Payout Mode" )		PORT_DIPLOCATION("DSW1:3")	/* OK */
	PORT_DIPSETTING(    0x00, "Payout Switch" )
	PORT_DIPSETTING(    0x04, "Automatic" )
	PORT_DIPNAME( 0x08, 0x00, "W-UP '7'" )			PORT_DIPLOCATION("DSW1:4")	/* not checked */
	PORT_DIPSETTING(    0x08, "Loss" )
	PORT_DIPSETTING(    0x00, "Even" )
	PORT_DIPNAME( 0x10, 0x10, "W-UP Pay Rate" )		PORT_DIPLOCATION("DSW1:5")	/* OK */
	PORT_DIPSETTING(    0x00, "40%" )
	PORT_DIPSETTING(    0x10, "60%" )
	PORT_DIPNAME( 0x20, 0x00, "W-UP Game" )			PORT_DIPLOCATION("DSW1:6")	/* OK */
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Bet Max" )			PORT_DIPLOCATION("DSW1:7,8")	/* OK */
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x40, "16" )
	PORT_DIPSETTING(    0x80, "32" )
	PORT_DIPSETTING(    0xc0, "64" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x03, "Main Game Pay Rate" )	PORT_DIPLOCATION("DSW2:1,2,3")	/* OK */
	PORT_DIPSETTING(    0x07, "30%" )
	PORT_DIPSETTING(    0x06, "38%" )
	PORT_DIPSETTING(    0x05, "46%" )
	PORT_DIPSETTING(    0x04, "54%" )
	PORT_DIPSETTING(    0x03, "62%" )
	PORT_DIPSETTING(    0x02, "70%" )
	PORT_DIPSETTING(    0x01, "78%" )
	PORT_DIPSETTING(    0x00, "86%" )
	PORT_DIPNAME( 0x18, 0x00, "Hopper Limit" )			PORT_DIPLOCATION("DSW2:4,5")	/* OK */
	PORT_DIPSETTING(    0x18, "300" )
	PORT_DIPSETTING(    0x10, "500" )
	PORT_DIPSETTING(    0x08, "1000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x20, 0x00, "100 Odds Sound" )		PORT_DIPLOCATION("DSW2:6")	/* not checked */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Key-In Type" )			PORT_DIPLOCATION("DSW2:7")	/* OK */
	PORT_DIPSETTING(    0x40, "A-Type" )
	PORT_DIPSETTING(    0x00, "B-Type" )
	PORT_DIPNAME( 0x80, 0x00, "Center Super 7 Bet Limit" )	PORT_DIPLOCATION("DSW2:8")	/* related with DSW 4-6 */
	PORT_DIPSETTING(    0x80, "Unlimited" )
	PORT_DIPSETTING(    0x00, "Limited" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, "Key In Rate" ) PORT_DIPLOCATION("DSW3:1,2")	/* OK */
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40) /* A-Type */
	PORT_DIPSETTING(    0x01, "1 Coin/20 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40)
	PORT_DIPSETTING(    0x02, "1 Coin/50 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40)
	PORT_DIPSETTING(    0x03, "1 Coin/100 Credits" ) PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )     PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00) /* B-Type */
	PORT_DIPSETTING(    0x01, "1 Coin/10 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x02, "1 Coin/25 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x03, "1 Coin/50 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00)
	PORT_DIPNAME( 0x0c, 0x0c, "Coin A Rate" ) PORT_DIPLOCATION("DSW3:3,4")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0c, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x30, 0x30, "Coin D Rate" ) PORT_DIPLOCATION("DSW3:5,6")	/* OK */
	PORT_DIPSETTING(    0x30, DEF_STR( 5C_1C ) )    PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10) /* C-Type */
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )    PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )    PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )    PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )    PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00) /* D-Type */
	PORT_DIPSETTING(    0x10, "1 Coin/10 Credits" ) PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x20, "1 Coin/25 Credits" ) PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x30, "1 Coin/50 Credits" ) PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPNAME( 0xc0, 0xc0, "Coin C Rate" ) PORT_DIPLOCATION("DSW3:7,8")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xc0, "1 Coin/10 Credits" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x07, 0x06, "Credit Limit" )            PORT_DIPLOCATION("DSW4:1,2,3")	/* OK */
	PORT_DIPSETTING(    0x07, "5,000" )
	PORT_DIPSETTING(    0x06, "10,000" )
	PORT_DIPSETTING(    0x05, "20,000" )
	PORT_DIPSETTING(    0x04, "30,000" )
	PORT_DIPSETTING(    0x03, "40,000" )
	PORT_DIPSETTING(    0x02, "50,000" )
	PORT_DIPSETTING(    0x01, "100,000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x08, 0x00, "Display Of Payout Limit" ) PORT_DIPLOCATION("DSW4:4") /* OK */
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Type Of Coin D" )          PORT_DIPLOCATION("DSW4:5")	/* OK */
	PORT_DIPSETTING(    0x10, "C-Type" )
	PORT_DIPSETTING(    0x00, "D-Type" )
	PORT_DIPNAME( 0x20, 0x20, "Min. Bet For Bonus Play" ) PORT_DIPLOCATION("DSW4:6")	/* OK */
	PORT_DIPSETTING(    0x20, "16 Bet" )
	PORT_DIPSETTING(    0x00, "8 Bet" )
	PORT_DIPNAME( 0x40, 0x40, "Reel Speed" )              PORT_DIPLOCATION("DSW4:7")	/* OK */
	PORT_DIPSETTING(    0x40, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x80, 0x80, "Hopper Out By Coin A" )    PORT_DIPLOCATION("DSW4:8")	/* not checked */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x01, 0x00, "Flash 'Dyna - C.M.V.4' string in attract" ) PORT_DIPLOCATION("DSW5:1")	/* OK */
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, "Coin In Limit" )                    PORT_DIPLOCATION("DSW5:2,3")	/* not checked */
	PORT_DIPSETTING(    0x06, "1,000" )
	PORT_DIPSETTING(    0x04, "5,000" )
	PORT_DIPSETTING(    0x02, "10,000" )
	PORT_DIPSETTING(    0x00, "20,000" )
	PORT_DIPNAME( 0x18, 0x18, "Condition For 3 Kind Of Bonus" )    PORT_DIPLOCATION("DSW5:4,5")	/* not checked */
	PORT_DIPSETTING(    0x18, "12-7-1" )
	PORT_DIPSETTING(    0x10, "9-5-1" )
	PORT_DIPSETTING(    0x08, "6-3-1" )
	PORT_DIPSETTING(    0x00, "3-2-1" )
	PORT_DIPNAME( 0x20, 0x00, "Display Of Doll At All Fr. Bonus" ) PORT_DIPLOCATION("DSW5:6")	/* not checked */
	PORT_DIPSETTING(    0x20, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )                 PORT_DIPLOCATION("DSW5:7")	/* listed as unused */
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Test Mode For Disp. Of Doll" )      PORT_DIPLOCATION("DSW5:8")	/* not working */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( cmaster )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SLOT_STOP3 ) PORT_NAME("Stop 3 / Big")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SLOT_STOP1 ) PORT_NAME("Stop 1 / D-UP")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SLOT_STOP_ALL ) PORT_NAME("Bet / Stop All")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SLOT_STOP2 ) PORT_NAME("Stop 2 / Small / Info")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)	/* Coin B */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_IMPULSE(2)	/* Coin D */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2)	/* Coin C */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)	/* Coin A */

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Out / Attendant")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Settings")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Stats")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:1")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Hopper Out Switch" )	PORT_DIPLOCATION("DSW1:2")	/* OK */
	PORT_DIPSETTING(    0x00, "Active Low" )
	PORT_DIPSETTING(    0x02, "Active High" )
	PORT_DIPNAME( 0x04, 0x00, "Payout Mode" )		PORT_DIPLOCATION("DSW1:3")	/* OK */
	PORT_DIPSETTING(    0x00, "Payout Switch" )
	PORT_DIPSETTING(    0x04, "Automatic" )
	PORT_DIPNAME( 0x08, 0x00, "W-UP '7'" )			PORT_DIPLOCATION("DSW1:4")	/* not checked */
	PORT_DIPSETTING(    0x08, "Loss" )
	PORT_DIPSETTING(    0x00, "Even" )
	PORT_DIPNAME( 0x10, 0x00, "W-UP Pay Rate" )		PORT_DIPLOCATION("DSW1:5")	/* OK */
	PORT_DIPSETTING(    0x00, "60%" )
	PORT_DIPSETTING(    0x10, "70%" )
	PORT_DIPNAME( 0x20, 0x00, "W-UP Game" )			PORT_DIPLOCATION("DSW1:6")	/* OK */
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Bet Max" )			PORT_DIPLOCATION("DSW1:7,8")	/* OK */
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x40, "16" )
	PORT_DIPSETTING(    0x80, "32" )
	PORT_DIPSETTING(    0xc0, "64" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x03, "Main Game Pay Rate" )	PORT_DIPLOCATION("DSW2:1,2,3")	/* OK */
	PORT_DIPSETTING(    0x07, "45%" )
	PORT_DIPSETTING(    0x06, "50%" )
	PORT_DIPSETTING(    0x05, "55%" )
	PORT_DIPSETTING(    0x04, "60%" )
	PORT_DIPSETTING(    0x03, "65%" )
	PORT_DIPSETTING(    0x02, "70%" )
	PORT_DIPSETTING(    0x01, "75%" )
	PORT_DIPSETTING(    0x00, "80%" )
	PORT_DIPNAME( 0x18, 0x00, "Hopper Limit" )			PORT_DIPLOCATION("DSW2:4,5")	/* OK */
	PORT_DIPSETTING(    0x18, "300" )
	PORT_DIPSETTING(    0x10, "500" )
	PORT_DIPSETTING(    0x08, "1000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x20, 0x00, "100 Odds Sound" )		PORT_DIPLOCATION("DSW2:6")	/* not checked */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Key-In Type" )			PORT_DIPLOCATION("DSW2:7")	/* OK */
	PORT_DIPSETTING(    0x40, "A-Type" )
	PORT_DIPSETTING(    0x00, "B-Type" )
	PORT_DIPNAME( 0x80, 0x00, "Center Super 7 Bet Limit" )	PORT_DIPLOCATION("DSW2:8")	/* related with DSW 4-6 */
	PORT_DIPSETTING(    0x80, "Unlimited" )
	PORT_DIPSETTING(    0x00, "Limited" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, "Key In Rate" ) PORT_DIPLOCATION("DSW3:1,2")	/* OK */
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40) /* A-Type */
	PORT_DIPSETTING(    0x01, "1 Coin/20 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40)
	PORT_DIPSETTING(    0x02, "1 Coin/50 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40)
	PORT_DIPSETTING(    0x03, "1 Coin/100 Credits" ) PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )     PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00) /* B-Type */
	PORT_DIPSETTING(    0x01, "1 Coin/10 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x02, "1 Coin/25 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x03, "1 Coin/50 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00)
	PORT_DIPNAME( 0x0c, 0x0c, "Coin A Rate" ) PORT_DIPLOCATION("DSW3:3,4")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0c, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x30, 0x30, "Coin D Rate" ) PORT_DIPLOCATION("DSW3:5,6")	/* OK */
	PORT_DIPSETTING(    0x30, DEF_STR( 5C_1C ) )    PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10) /* C-Type */
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )    PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )    PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )    PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )    PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00) /* D-Type */
	PORT_DIPSETTING(    0x10, "1 Coin/10 Credits" ) PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x20, "1 Coin/25 Credits" ) PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x30, "1 Coin/50 Credits" ) PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPNAME( 0xc0, 0xc0, "Coin C Rate" ) PORT_DIPLOCATION("DSW3:7,8")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xc0, "1 Coin/10 Credits" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x07, 0x06, "Credit Limit" )            PORT_DIPLOCATION("DSW4:1,2,3")	/* OK */
	PORT_DIPSETTING(    0x07, "5,000" )
	PORT_DIPSETTING(    0x06, "10,000" )
	PORT_DIPSETTING(    0x05, "20,000" )
	PORT_DIPSETTING(    0x04, "30,000" )
	PORT_DIPSETTING(    0x03, "40,000" )
	PORT_DIPSETTING(    0x02, "50,000" )
	PORT_DIPSETTING(    0x01, "100,000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x08, 0x00, "Display Of Payout Limit" ) PORT_DIPLOCATION("DSW4:4") /* OK */
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Type Of Coin D" )          PORT_DIPLOCATION("DSW4:5")	/* OK */
	PORT_DIPSETTING(    0x10, "C-Type" )
	PORT_DIPSETTING(    0x00, "D-Type" )
	PORT_DIPNAME( 0x20, 0x20, "Min. Bet For Bonus Play" ) PORT_DIPLOCATION("DSW4:6")	/* OK */
	PORT_DIPSETTING(    0x20, "16 Bet" )
	PORT_DIPSETTING(    0x00, "8 Bet" )
	PORT_DIPNAME( 0x40, 0x40, "Reel Speed" )              PORT_DIPLOCATION("DSW4:7")	/* OK */
	PORT_DIPSETTING(    0x40, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x80, 0x80, "Hopper Out By Coin A" )    PORT_DIPLOCATION("DSW4:8")	/* not checked */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x01, 0x00, "Display Of Doll On Demo" )          PORT_DIPLOCATION("DSW5:1")	/* not working */
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, "Coin In Limit" )                    PORT_DIPLOCATION("DSW5:2,3")	/* not checked */
	PORT_DIPSETTING(    0x06, "1,000" )
	PORT_DIPSETTING(    0x04, "5,000" )
	PORT_DIPSETTING(    0x02, "10,000" )
	PORT_DIPSETTING(    0x00, "20,000" )
	PORT_DIPNAME( 0x18, 0x18, "Condition For 3 Kind Of Bonus" )    PORT_DIPLOCATION("DSW5:4,5")	/* not checked */
	PORT_DIPSETTING(    0x18, "12-7-1" )
	PORT_DIPSETTING(    0x10, "9-5-1" )
	PORT_DIPSETTING(    0x08, "6-3-1" )
	PORT_DIPSETTING(    0x00, "3-2-1" )
	PORT_DIPNAME( 0x20, 0x00, "Display Of Doll At All Fr. Bonus" ) PORT_DIPLOCATION("DSW5:6")	/* not checked */
	PORT_DIPSETTING(    0x20, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )                 PORT_DIPLOCATION("DSW5:7")	/* listed as unused */
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Test Mode For Disp. Of Doll" )      PORT_DIPLOCATION("DSW5:8")	/* not working */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( cmasterb )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Big / Stop All")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP ) PORT_NAME("D-UP")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE ) PORT_NAME("Take Score")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) PORT_NAME("Small / Info")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)	/* Coin B */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_IMPULSE(2)	/* Coin D */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2)	/* Coin C */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)	/* Coin A */

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Out / Attendant")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Settings")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Stats")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:1")	/* OK */
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Hopper Out Switch" )	PORT_DIPLOCATION("DSW1:2")	/* OK */
	PORT_DIPSETTING(    0x02, "Active Low" )
	PORT_DIPSETTING(    0x00, "Active High" )
	PORT_DIPNAME( 0x04, 0x00, "Payout Mode" )		PORT_DIPLOCATION("DSW1:3")	/* OK */
	PORT_DIPSETTING(    0x04, "Payout Switch" )
	PORT_DIPSETTING(    0x00, "Automatic" )
	PORT_DIPNAME( 0x08, 0x00, "W-UP '7'" )			PORT_DIPLOCATION("DSW1:4")	/* not checked */
	PORT_DIPSETTING(    0x08, "Loss" )
	PORT_DIPSETTING(    0x00, "Even" )
	PORT_DIPNAME( 0x10, 0x00, "W-UP Pay Rate" )		PORT_DIPLOCATION("DSW1:5")	/* OK */
	PORT_DIPSETTING(    0x00, "80%" )
	PORT_DIPSETTING(    0x10, "90%" )
	PORT_DIPNAME( 0x20, 0x00, "W-UP Game" )			PORT_DIPLOCATION("DSW1:6")	/* OK */
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Bet Max" )			PORT_DIPLOCATION("DSW1:7,8")	/* OK */
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x40, "16" )
	PORT_DIPSETTING(    0x80, "32" )
	PORT_DIPSETTING(    0xc0, "64" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x03, "Main Game Pay Rate" )	PORT_DIPLOCATION("DSW2:1,2,3")	/* OK */
	PORT_DIPSETTING(    0x07, "55%" )
	PORT_DIPSETTING(    0x06, "60%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x04, "70%" )
	PORT_DIPSETTING(    0x03, "75%" )
	PORT_DIPSETTING(    0x02, "80%" )
	PORT_DIPSETTING(    0x01, "85%" )
	PORT_DIPSETTING(    0x00, "90%" )
	PORT_DIPNAME( 0x18, 0x00, "Hopper Limit" )			PORT_DIPLOCATION("DSW2:4,5")	/* OK */
	PORT_DIPSETTING(    0x18, "300" )
	PORT_DIPSETTING(    0x10, "500" )
	PORT_DIPSETTING(    0x08, "1000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x20, 0x00, "100 Odds Sound" )		PORT_DIPLOCATION("DSW2:6")	/* not checked */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Key-In Type" )			PORT_DIPLOCATION("DSW2:7")	/* OK */
	PORT_DIPSETTING(    0x40, "A-Type" )
	PORT_DIPSETTING(    0x00, "B-Type" )
	PORT_DIPNAME( 0x80, 0x00, "Center Super 7 Bet Limit" )	PORT_DIPLOCATION("DSW2:8")	/* related with DSW 4-6 */
	PORT_DIPSETTING(    0x80, "Unlimited" )
	PORT_DIPSETTING(    0x00, "Limited" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, "Key In Rate" ) PORT_DIPLOCATION("DSW3:1,2")	/* OK */
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40) /* A-Type */
	PORT_DIPSETTING(    0x01, "1 Coin/20 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40)
	PORT_DIPSETTING(    0x02, "1 Coin/50 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40)
	PORT_DIPSETTING(    0x03, "1 Coin/100 Credits" ) PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )     PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00) /* B-Type */
	PORT_DIPSETTING(    0x01, "1 Coin/10 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x02, "1 Coin/25 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x03, "1 Coin/50 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00)
	PORT_DIPNAME( 0x0c, 0x0c, "Coin A Rate" ) PORT_DIPLOCATION("DSW3:3,4")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0c, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x30, 0x30, "Coin D Rate" ) PORT_DIPLOCATION("DSW3:5,6")	/* OK */
	PORT_DIPSETTING(    0x30, DEF_STR( 5C_1C ) )    PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10) /* C-Type */
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )    PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )    PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )    PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )    PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00) /* D-Type */
	PORT_DIPSETTING(    0x10, "1 Coin/10 Credits" ) PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x20, "1 Coin/25 Credits" ) PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x30, "1 Coin/50 Credits" ) PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPNAME( 0xc0, 0xc0, "Coin C Rate" ) PORT_DIPLOCATION("DSW3:7,8")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xc0, "1 Coin/10 Credits" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x07, 0x05, "Credit Limit" )            PORT_DIPLOCATION("DSW4:1,2,3")	/* OK */
	PORT_DIPSETTING(    0x07, "5,000" )
	PORT_DIPSETTING(    0x06, "10,000" )
	PORT_DIPSETTING(    0x05, "20,000" )
	PORT_DIPSETTING(    0x04, "30,000" )
	PORT_DIPSETTING(    0x03, "40,000" )
	PORT_DIPSETTING(    0x02, "50,000" )
	PORT_DIPSETTING(    0x01, "100,000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x08, 0x00, "Display Of Payout Limit" ) PORT_DIPLOCATION("DSW4:4")	/* OK */
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Type Of Coin D" )          PORT_DIPLOCATION("DSW4:5")	/* OK */
	PORT_DIPSETTING(    0x10, "C-Type" )
	PORT_DIPSETTING(    0x00, "D-Type" )
	PORT_DIPNAME( 0x20, 0x20, "Min. Bet For Bonus Play" ) PORT_DIPLOCATION("DSW4:6")	/* OK */
	PORT_DIPSETTING(    0x20, "16 Bet" )
	PORT_DIPSETTING(    0x00, "8 Bet" )
	PORT_DIPNAME( 0x40, 0x40, "Reel Speed" )              PORT_DIPLOCATION("DSW4:7")	/* OK */
	PORT_DIPSETTING(    0x40, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x80, 0x80, "Hopper Out By Coin A" )    PORT_DIPLOCATION("DSW4:8")	/* not checked */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x01, 0x00, "Display Of Doll On Demo" )          PORT_DIPLOCATION("DSW5:1")	/* not checked */
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, "Coin In Limit" )                    PORT_DIPLOCATION("DSW5:2,3")	/* not checked */
	PORT_DIPSETTING(    0x06, "1,000" )
	PORT_DIPSETTING(    0x04, "5,000" )
	PORT_DIPSETTING(    0x02, "10,000" )
	PORT_DIPSETTING(    0x00, "20,000" )
	PORT_DIPNAME( 0x18, 0x18, "Condition For 3 Kind Of Bonus" )    PORT_DIPLOCATION("DSW5:4,5")	/* not checked */
	PORT_DIPSETTING(    0x18, "12-7-1" )
	PORT_DIPSETTING(    0x10, "9-5-1" )
	PORT_DIPSETTING(    0x08, "6-3-1" )
	PORT_DIPSETTING(    0x00, "3-2-1" )
	PORT_DIPNAME( 0x20, 0x00, "Display Of Doll At All Fr. Bonus" ) PORT_DIPLOCATION("DSW5:6")	/* not checked */
	PORT_DIPSETTING(    0x20, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )                 PORT_DIPLOCATION("DSW5:7")	/* listed as unused */
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Test Mode For Disp. Of Doll" )      PORT_DIPLOCATION("DSW5:8")	/* not checked */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( cmasterc )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Big / Stop 1")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP ) PORT_NAME("D-UP / Stop 2")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE ) PORT_NAME("Take Score")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) PORT_NAME("Small / Info / Stop 3")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)	/* Coin B */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_IMPULSE(2)	/* Coin D */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2)	/* Coin C */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)	/* Coin A */

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Out / Attendant")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Settings")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Stats")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:1")	/* OK */
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Hopper Out Switch" )	PORT_DIPLOCATION("DSW1:2")	/* OK */
	PORT_DIPSETTING(    0x02, "Active Low" )
	PORT_DIPSETTING(    0x00, "Active High" )
	PORT_DIPNAME( 0x04, 0x00, "Payout Mode" )		PORT_DIPLOCATION("DSW1:3")	/* OK */
	PORT_DIPSETTING(    0x04, "Payout Switch" )
	PORT_DIPSETTING(    0x00, "Automatic" )
	PORT_DIPNAME( 0x08, 0x00, "W-UP '7'" )			PORT_DIPLOCATION("DSW1:4")	/* not checked */
	PORT_DIPSETTING(    0x08, "Loss" )
	PORT_DIPSETTING(    0x00, "Even" )
	PORT_DIPNAME( 0x10, 0x00, "W-UP Pay Rate" )		PORT_DIPLOCATION("DSW1:5")	/* OK */
	PORT_DIPSETTING(    0x00, "80%" )
	PORT_DIPSETTING(    0x10, "90%" )
	PORT_DIPNAME( 0x20, 0x00, "W-UP Game" )			PORT_DIPLOCATION("DSW1:6")	/* OK */
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Bet Max" )			PORT_DIPLOCATION("DSW1:7,8")	/* OK */
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x40, "16" )
	PORT_DIPSETTING(    0x80, "32" )
	PORT_DIPSETTING(    0xc0, "64" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x03, "Main Game Pay Rate" )	PORT_DIPLOCATION("DSW2:1,2,3")	/* OK */
	PORT_DIPSETTING(    0x07, "55%" )
	PORT_DIPSETTING(    0x06, "60%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x04, "70%" )
	PORT_DIPSETTING(    0x03, "75%" )
	PORT_DIPSETTING(    0x02, "80%" )
	PORT_DIPSETTING(    0x01, "85%" )
	PORT_DIPSETTING(    0x00, "90%" )
	PORT_DIPNAME( 0x18, 0x00, "Hopper Limit" )			PORT_DIPLOCATION("DSW2:4,5")	/* OK */
	PORT_DIPSETTING(    0x18, "300" )
	PORT_DIPSETTING(    0x10, "500" )
	PORT_DIPSETTING(    0x08, "1000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x20, 0x00, "100 Odds Sound" )		PORT_DIPLOCATION("DSW2:6")	/* not checked */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Key-In Type" )			PORT_DIPLOCATION("DSW2:7")	/* OK */
	PORT_DIPSETTING(    0x40, "A-Type" )
	PORT_DIPSETTING(    0x00, "B-Type" )
	PORT_DIPNAME( 0x80, 0x00, "Center Super 7 Bet Limit" )	PORT_DIPLOCATION("DSW2:8")	/* related with DSW 4-6 */
	PORT_DIPSETTING(    0x80, "Unlimited" )
	PORT_DIPSETTING(    0x00, "Limited" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, "Key In Rate" ) PORT_DIPLOCATION("DSW3:1,2")	/* OK */
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40) /* A-Type */
	PORT_DIPSETTING(    0x01, "1 Coin/20 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40)
	PORT_DIPSETTING(    0x02, "1 Coin/50 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40)
	PORT_DIPSETTING(    0x03, "1 Coin/100 Credits" ) PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )     PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00) /* B-Type */
	PORT_DIPSETTING(    0x01, "1 Coin/10 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x02, "1 Coin/25 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x03, "1 Coin/50 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00)
	PORT_DIPNAME( 0x0c, 0x0c, "Coin A Rate" ) PORT_DIPLOCATION("DSW3:3,4")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0c, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x30, 0x30, "Coin D Rate" ) PORT_DIPLOCATION("DSW3:5,6")	/* OK */
	PORT_DIPSETTING(    0x30, DEF_STR( 5C_1C ) )    PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10) /* C-Type */
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )    PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )    PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )    PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )    PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00) /* D-Type */
	PORT_DIPSETTING(    0x10, "1 Coin/10 Credits" ) PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x20, "1 Coin/25 Credits" ) PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x30, "1 Coin/50 Credits" ) PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPNAME( 0xc0, 0xc0, "Coin C Rate" ) PORT_DIPLOCATION("DSW3:7,8")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xc0, "1 Coin/10 Credits" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x07, 0x05, "Credit Limit" )            PORT_DIPLOCATION("DSW4:1,2,3")	/* OK */
	PORT_DIPSETTING(    0x07, "5,000" )
	PORT_DIPSETTING(    0x06, "10,000" )
	PORT_DIPSETTING(    0x05, "20,000" )
	PORT_DIPSETTING(    0x04, "30,000" )
	PORT_DIPSETTING(    0x03, "40,000" )
	PORT_DIPSETTING(    0x02, "50,000" )
	PORT_DIPSETTING(    0x01, "100,000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x08, 0x00, "Display Of Payout Limit" ) PORT_DIPLOCATION("DSW4:4")	/* OK */
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Type Of Coin D" )          PORT_DIPLOCATION("DSW4:5")	/* OK */
	PORT_DIPSETTING(    0x10, "C-Type" )
	PORT_DIPSETTING(    0x00, "D-Type" )
	PORT_DIPNAME( 0x20, 0x20, "Min. Bet For Bonus Play" ) PORT_DIPLOCATION("DSW4:6")	/* OK */
	PORT_DIPSETTING(    0x20, "16 Bet" )
	PORT_DIPSETTING(    0x00, "8 Bet" )
	PORT_DIPNAME( 0x40, 0x40, "Reel Speed" )              PORT_DIPLOCATION("DSW4:7")	/* OK */
	PORT_DIPSETTING(    0x40, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x80, 0x80, "Hopper Out By Coin A" )    PORT_DIPLOCATION("DSW4:8")	/* not checked */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x01, 0x00, "Display Of Doll On Demo" )          PORT_DIPLOCATION("DSW5:1")	/* not checked */
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, "Coin In Limit" )                    PORT_DIPLOCATION("DSW5:2,3")	/* not checked */
	PORT_DIPSETTING(    0x06, "1,000" )
	PORT_DIPSETTING(    0x04, "5,000" )
	PORT_DIPSETTING(    0x02, "10,000" )
	PORT_DIPSETTING(    0x00, "20,000" )
	PORT_DIPNAME( 0x18, 0x18, "Condition For 3 Kind Of Bonus" )    PORT_DIPLOCATION("DSW5:4,5")	/* not checked */
	PORT_DIPSETTING(    0x18, "12-7-1" )
	PORT_DIPSETTING(    0x10, "9-5-1" )
	PORT_DIPSETTING(    0x08, "6-3-1" )
	PORT_DIPSETTING(    0x00, "3-2-1" )
	PORT_DIPNAME( 0x20, 0x00, "Display Of Doll At All Fr. Bonus" ) PORT_DIPLOCATION("DSW5:6")	/* not checked */
	PORT_DIPSETTING(    0x20, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )                 PORT_DIPLOCATION("DSW5:7")	/* listed as unused */
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Test Mode For Disp. Of Doll" )      PORT_DIPLOCATION("DSW5:8")	/* not checked */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( cmast91 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Big")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP ) PORT_NAME("D-UP")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) PORT_NAME("Small / Info")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)	/* Coin B */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_IMPULSE(2)	/* Coin D */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2)	/* Coin C */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)	/* Coin A */

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Out / Attendant")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Settings")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Stats")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:1")	/* OK */
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Hopper Out Switch" )	PORT_DIPLOCATION("DSW1:2")	/* OK */
	PORT_DIPSETTING(    0x02, "Active Low" )
	PORT_DIPSETTING(    0x00, "Active High" )
	PORT_DIPNAME( 0x04, 0x00, "Payout Mode" )		PORT_DIPLOCATION("DSW1:3")	/* OK */
	PORT_DIPSETTING(    0x04, "Payout Switch" )
	PORT_DIPSETTING(    0x00, "Automatic" )
	PORT_DIPNAME( 0x08, 0x00, "W-UP '7'" )			PORT_DIPLOCATION("DSW1:4")	/* not checked */
	PORT_DIPSETTING(    0x08, "Loss" )
	PORT_DIPSETTING(    0x00, "Even" )
	PORT_DIPNAME( 0x10, 0x00, "W-UP Pay Rate" )		PORT_DIPLOCATION("DSW1:5")	/* OK */
	PORT_DIPSETTING(    0x00, "80%" )
	PORT_DIPSETTING(    0x10, "90%" )
	PORT_DIPNAME( 0x20, 0x00, "W-UP Game" )			PORT_DIPLOCATION("DSW1:6")	/* OK */
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Bet Max" )			PORT_DIPLOCATION("DSW1:7,8")	/* OK */
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x40, "16" )
	PORT_DIPSETTING(    0x80, "32" )
	PORT_DIPSETTING(    0xc0, "64" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x03, "Main Game Pay Rate" )	PORT_DIPLOCATION("DSW2:1,2,3")	/* OK */
	PORT_DIPSETTING(    0x07, "45%" )
	PORT_DIPSETTING(    0x06, "50%" )
	PORT_DIPSETTING(    0x05, "55%" )
	PORT_DIPSETTING(    0x04, "60%" )
	PORT_DIPSETTING(    0x03, "65%" )
	PORT_DIPSETTING(    0x02, "70%" )
	PORT_DIPSETTING(    0x01, "75%" )
	PORT_DIPSETTING(    0x00, "80%" )
	PORT_DIPNAME( 0x18, 0x00, "Hopper Limit" )			PORT_DIPLOCATION("DSW2:4,5")	/* OK */
	PORT_DIPSETTING(    0x18, "300" )
	PORT_DIPSETTING(    0x10, "500" )
	PORT_DIPSETTING(    0x08, "1000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x20, 0x00, "100 Odds Sound" )		PORT_DIPLOCATION("DSW2:6")	/* not checked */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Key-In Type" )			PORT_DIPLOCATION("DSW2:7")	/* OK */
	PORT_DIPSETTING(    0x40, "A-Type" )
	PORT_DIPSETTING(    0x00, "B-Type" )
	PORT_DIPNAME( 0x80, 0x00, "Center Super 7 Bet Limit" )	PORT_DIPLOCATION("DSW2:8")	/* related with DSW 4-6 */
	PORT_DIPSETTING(    0x80, "Unlimited" )
	PORT_DIPSETTING(    0x00, "Limited" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, "Key In Rate" ) PORT_DIPLOCATION("DSW3:1,2")	/* OK */
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40) /* A-Type */
	PORT_DIPSETTING(    0x01, "1 Coin/20 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40)
	PORT_DIPSETTING(    0x02, "1 Coin/50 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40)
	PORT_DIPSETTING(    0x03, "1 Coin/100 Credits" ) PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )     PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00) /* B-Type */
	PORT_DIPSETTING(    0x01, "1 Coin/10 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x02, "1 Coin/25 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x03, "1 Coin/50 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00)
	PORT_DIPNAME( 0x0c, 0x0c, "Coin A Rate" ) PORT_DIPLOCATION("DSW3:3,4")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0c, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x30, 0x30, "Coin D Rate" ) PORT_DIPLOCATION("DSW3:5,6")	/* OK */
	PORT_DIPSETTING(    0x30, DEF_STR( 5C_1C ) )    PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10) /* C-Type */
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )    PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )    PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )    PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )    PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00) /* D-Type */
	PORT_DIPSETTING(    0x10, "1 Coin/10 Credits" ) PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x20, "1 Coin/25 Credits" ) PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x30, "1 Coin/50 Credits" ) PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPNAME( 0xc0, 0xc0, "Coin C Rate" ) PORT_DIPLOCATION("DSW3:7,8")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xc0, "1 Coin/10 Credits" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x07, 0x06, "Credit Limit" )            PORT_DIPLOCATION("DSW4:1,2,3")	/* OK */
	PORT_DIPSETTING(    0x07, "5,000" )
	PORT_DIPSETTING(    0x06, "10,000" )
	PORT_DIPSETTING(    0x05, "20,000" )
	PORT_DIPSETTING(    0x04, "30,000" )
	PORT_DIPSETTING(    0x03, "40,000" )
	PORT_DIPSETTING(    0x02, "50,000" )
	PORT_DIPSETTING(    0x01, "100,000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x08, 0x00, "Display Of Payout Limit" ) PORT_DIPLOCATION("DSW4:4")	/* OK */
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Type Of Coin D" )          PORT_DIPLOCATION("DSW4:5")	/* OK */
	PORT_DIPSETTING(    0x10, "C-Type" )
	PORT_DIPSETTING(    0x00, "D-Type" )
	PORT_DIPNAME( 0x20, 0x20, "Min. Bet For Bonus Play" ) PORT_DIPLOCATION("DSW4:6")	/* OK */
	PORT_DIPSETTING(    0x20, "16 Bet" )
	PORT_DIPSETTING(    0x00, "8 Bet" )
	PORT_DIPNAME( 0x40, 0x00, "Reel Speed" )              PORT_DIPLOCATION("DSW4:7")	/* OK */
	PORT_DIPSETTING(    0x40, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x80, 0x80, "Hopper Out By Coin A" )    PORT_DIPLOCATION("DSW4:8")	/* not checked */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )              PORT_DIPLOCATION("DSW5:1")	/* no whores here */
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, "Coin In Limit" )                 PORT_DIPLOCATION("DSW5:2,3")	/* not checked */
	PORT_DIPSETTING(    0x06, "1,000" )
	PORT_DIPSETTING(    0x04, "5,000" )
	PORT_DIPSETTING(    0x02, "10,000" )
	PORT_DIPSETTING(    0x00, "20,000" )
	PORT_DIPNAME( 0x18, 0x18, "Condition For 3 Kind Of Bonus" ) PORT_DIPLOCATION("DSW5:4,5")	/* not checked */
	PORT_DIPSETTING(    0x18, "12-7-1" )
	PORT_DIPSETTING(    0x10, "9-5-1" )
	PORT_DIPSETTING(    0x08, "6-3-1" )
	PORT_DIPSETTING(    0x00, "3-2-1" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )              PORT_DIPLOCATION("DSW5:6")	/* not checked */
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )              PORT_DIPLOCATION("DSW5:7")	/* listed as unused */
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )              PORT_DIPLOCATION("DSW5:8")	/* no whores in demo */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( goldstar )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_V) PORT_NAME("Bet Red / 2")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SLOT_STOP3 ) PORT_NAME("Stop 3 / Small / 1 / Info")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_B) PORT_NAME("Bet Blue / D-UP / 3")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SLOT_STOP1 ) PORT_NAME("Stop 1 / Take")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SLOT_STOP2 ) PORT_NAME("Stop 2 / Big / Ticket")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_N) PORT_NAME("Start / Stop All / 4")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* this is not a coin, not sure what it is */
													/* maybe it's used to buy tickets. Will check soon. */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F1) PORT_NAME("Statistics")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "Game Style" )
	PORT_DIPSETTING(    0x01, "Gettoni" )
	PORT_DIPSETTING(    0x00, "Ticket" )
	PORT_DIPNAME( 0x02, 0x02, "Hopper Out" )
	PORT_DIPSETTING(    0x02, "Active Low" )
	PORT_DIPSETTING(    0x00, "Active High" )
	PORT_DIPNAME( 0x04, 0x04, "Payout Automatic?" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "W-Up '7'" )
	PORT_DIPSETTING(    0x08, "Loss" )
	PORT_DIPSETTING(    0x00, "Even" )
	PORT_DIPNAME( 0x10, 0x10, "W-Up Pay Rate" )
	PORT_DIPSETTING(    0x10, "60%" )
	PORT_DIPSETTING(    0x00, "70%" )
	PORT_DIPNAME( 0x20, 0x20, "W-Up Game" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, "Bet Max" )
	PORT_DIPSETTING(    0xc0, "8 Bet" )
	PORT_DIPSETTING(    0x80, "16 Bet" )
	PORT_DIPSETTING(    0x40, "32 Bet" )
	PORT_DIPSETTING(    0x00, "50 Bet" )

	PORT_START("UNK1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("UNK2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x00, "Main Game Pay Rate" )
	PORT_DIPSETTING(    0x00, "75 %" )
	PORT_DIPSETTING(    0x01, "70 %" )
	PORT_DIPSETTING(    0x02, "65 %" )
	PORT_DIPSETTING(    0x03, "60 %" )
	PORT_DIPSETTING(    0x04, "55 %" )
	PORT_DIPSETTING(    0x05, "50 %" )
	PORT_DIPSETTING(    0x06, "45 %" )
	PORT_DIPSETTING(    0x07, "40 %" )
	PORT_DIPNAME( 0x18, 0x00, "Hopper Limit" )
	PORT_DIPSETTING(    0x18, "300" )
	PORT_DIPSETTING(    0x10, "500" )
	PORT_DIPSETTING(    0x08, "1000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x20, 0x00, "100 Odds Sound" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Key-In Type" )
	PORT_DIPSETTING(    0x40, "B-Type" )
	PORT_DIPSETTING(    0x00, "A-Type" )
	PORT_DIPNAME( 0x80, 0x00, "Center Super 7 Bet Limit" )
	PORT_DIPSETTING(    0x80, "Unlimited" )
	PORT_DIPSETTING(    0x00, "Limited" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin/20 Credits" )
	PORT_DIPSETTING(    0x08, "1 Coin/50 Credits" )
	PORT_DIPSETTING(    0x0c, "1 Coin/100 Credits" )
	PORT_DIPNAME( 0xc0, 0x40, "Coin C" )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xc0, "1 Coin/10 Credits" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x07, 0x06, "Credit Limited" )
	PORT_DIPSETTING(    0x07, "5000" )
	PORT_DIPSETTING(    0x06, "10000" )
	PORT_DIPSETTING(    0x05, "20000" )
	PORT_DIPSETTING(    0x04, "30000" )
	PORT_DIPSETTING(    0x03, "40000" )
	PORT_DIPSETTING(    0x02, "50000" )
	PORT_DIPSETTING(    0x01, "100000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x08, 0x00, "Display Credit Limit" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Type of Coin D" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Bonus Play Min Bet" )
	PORT_DIPSETTING(    0x20, "16 Bet" )
	PORT_DIPSETTING(    0x00, "8 Bet" )
	PORT_DIPNAME( 0x40, 0x00, "Reel Speed" )
	PORT_DIPSETTING(    0x40, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x80, 0x00, "Ticket Payment" )
	PORT_DIPSETTING(    0x80, "1 Ticket/100" )
	PORT_DIPSETTING(    0x00, "Pay All" )

	PORT_START("DSW6")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW7")	/* ??? */
	PORT_BIT( 0xdf, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x20, 0x00, "Show Woman" )
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
INPUT_PORTS_END

static INPUT_PORTS_START( chrygld )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_X) PORT_NAME("Stop 2 / Big / Bonus Game")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_B) PORT_NAME("Blue Bet / D-UP / Card 3")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_Z) PORT_NAME("Stop 1 / Take")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_V) PORT_NAME("Red Bet / Card 2")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_C) PORT_NAME("Stop 3 / Small / Info / Card 1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_CODE(KEYCODE_N) PORT_NAME("Start / Stop All / Card 4")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)	/* Coin B */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_IMPULSE(2)	/* Coin D */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2)	/* Coin C */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)	/* Coin A */

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Out / Attendant")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Settings")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Stats")

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Game Style" )		PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x01, "Tokens" )
	PORT_DIPSETTING(    0x00, "Ticket" )
	PORT_DIPNAME( 0x02, 0x02, "Hopper Out Switch" )	PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x02, "Active Low" )
	PORT_DIPSETTING(    0x00, "Active High" )
	PORT_DIPNAME( 0x04, 0x04, "Payout Mode" )		PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, "Payout Switch" )
	PORT_DIPSETTING(    0x00, "Automatic" )
	PORT_DIPNAME( 0x08, 0x00, "W-UP '7'" )			PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, "Loss" )
	PORT_DIPSETTING(    0x00, "Even" )
	PORT_DIPNAME( 0x10, 0x10, "W-UP Pay Rate" )		PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x10, "80%" )
	PORT_DIPSETTING(    0x00, "90%" )
	PORT_DIPNAME( 0x20, 0x20, "W-UP Game" )			PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, "Bet Max" )			PORT_DIPLOCATION("DSW1:7,8")
	PORT_DIPSETTING(    0xc0, "8" )
	PORT_DIPSETTING(    0x80, "16" )
	PORT_DIPSETTING(    0x40, "32" )
	PORT_DIPSETTING(    0x00, "50" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x00, "Main Game Pay Rate" )	PORT_DIPLOCATION("DSW2:1,2,3")
	PORT_DIPSETTING(    0x00, "55%" )
	PORT_DIPSETTING(    0x01, "60%" )
	PORT_DIPSETTING(    0x02, "65%" )
	PORT_DIPSETTING(    0x03, "70%" )
	PORT_DIPSETTING(    0x04, "75%" )
	PORT_DIPSETTING(    0x05, "80%" )
	PORT_DIPSETTING(    0x06, "85%" )
	PORT_DIPSETTING(    0x07, "90%" )
	PORT_DIPNAME( 0x18, 0x00, "Hopper Limit" )			PORT_DIPLOCATION("DSW2:4,5")
	PORT_DIPSETTING(    0x18, "300" )
	PORT_DIPSETTING(    0x10, "500" )
	PORT_DIPSETTING(    0x08, "1000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x20, 0x00, "100 Odds Sound" )		PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Key-In Type" )			PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(    0x40, "B-Type" )
	PORT_DIPSETTING(    0x00, "A-Type" )
	PORT_DIPNAME( 0x80, 0x00, "Center Super 7 Bet Limit" )	PORT_DIPLOCATION("DSW2:8")	/* related with DSW 4-6 */
	PORT_DIPSETTING(    0x80, "Unlimited" )
	PORT_DIPSETTING(    0x00, "Limited" )

	PORT_START("DSW3")	/* some of these could be wrong */
	PORT_DIPNAME( 0x03, 0x03, "Key In Rate" ) PORT_DIPLOCATION("DSW3:1,2")
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )		PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00) // A-Type
	PORT_DIPSETTING(    0x01, "1 Coin/20 Credits" )		PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x02, "1 Coin/50 Credits" )		PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x03, "1 Coin/100 Credits" )	PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )		PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40) // B-Type
	PORT_DIPSETTING(    0x01, "1 Coin/10 Credits" )		PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40)
	PORT_DIPSETTING(    0x02, "1 Coin/25 Credits" )		PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40)
	PORT_DIPSETTING(    0x03, "1 Coin/50 Credits" )		PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40)
	PORT_DIPNAME( 0x0c, 0x0c, "Coin A Rate" ) PORT_DIPLOCATION("DSW3:3,4")
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin/20 Credits" )
	PORT_DIPSETTING(    0x08, "1 Coin/50 Credits" )
	PORT_DIPSETTING(    0x0c, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x30, 0x30, "Coin D Rate" ) PORT_DIPLOCATION("DSW3:5,6")
	PORT_DIPSETTING(    0x30, DEF_STR( 5C_1C ) )				PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10) // C-Type
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )				PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )				PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )				PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x30, "1 Ticket/Coin / 100 Credits" )	PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00) // D-Type
	PORT_DIPSETTING(    0x20, "1 Ticket/Coin / 100 Credits" )	PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x10, "1 Ticket/Coin / 100 Credits" )	PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x00, "1 Ticket/Coin / 100 Credits" )	PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPNAME( 0xc0, 0xc0, "Coin C Rate" ) PORT_DIPLOCATION("DSW3:7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xc0, "1 Coin/10 Credits" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x07, 0x06, "Credit Limit" )				PORT_DIPLOCATION("DSW4:1,2,3")
	PORT_DIPSETTING(    0x07, "5,000" )
	PORT_DIPSETTING(    0x06, "10,000" )
	PORT_DIPSETTING(    0x05, "20,000" )
	PORT_DIPSETTING(    0x04, "30,000" )
	PORT_DIPSETTING(    0x03, "40,000" )
	PORT_DIPSETTING(    0x02, "50,000" )
	PORT_DIPSETTING(    0x01, "100,000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x08, 0x00, "Display Of Credit Limit" )	PORT_DIPLOCATION("DSW4:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Type Of Coin D" )			PORT_DIPLOCATION("DSW4:5")
	PORT_DIPSETTING(    0x10, "D-Type (Tokens)" )
	PORT_DIPSETTING(    0x00, "C-Type (Ticket)" )
	PORT_DIPNAME( 0x20, 0x00, "Bonus Play Min.Bet" )		PORT_DIPLOCATION("DSW4:6")
	PORT_DIPSETTING(    0x20, "16 Bet" )
	PORT_DIPSETTING(    0x00, "8 Bet" )
	PORT_DIPNAME( 0x40, 0x00, "Reel Speed" )				PORT_DIPLOCATION("DSW4:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x80, 0x80, "Coin A Mode" )				PORT_DIPLOCATION("DSW4:8")
	PORT_DIPSETTING(    0x80, "Unexchange" )
	PORT_DIPSETTING(    0x00, "Exchange" )

	/* DSW5 is not connected yet. Where the hell is connected? */
	PORT_START("DSW5")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unused ) )		PORT_DIPLOCATION("DSW5:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x00, "Coin In Limit" )			PORT_DIPLOCATION("DSW5:2,3")
	PORT_DIPSETTING(    0x06, "1000" )
	PORT_DIPSETTING(    0x04, "5000" )
	PORT_DIPSETTING(    0x02, "10000" )
	PORT_DIPSETTING(    0x00, "20000" )
	PORT_DIPNAME( 0x18, 0x10, "Coin Out Rate" )			PORT_DIPLOCATION("DSW5:4,5")
	PORT_DIPSETTING(    0x00, "100 Credits / 1 Pulse" )
	PORT_DIPSETTING(    0x08, "100 Credits / 5 Pulses" )
	PORT_DIPSETTING(    0x10, "100 Credits / 10 Pulses" )
	PORT_DIPSETTING(    0x18, "100 Credits / 100 Pulses" )
	PORT_DIPNAME( 0x20, 0x00, "W-UP Girl" )				PORT_DIPLOCATION("DSW5:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x40, "Coin In Rate" )			PORT_DIPLOCATION("DSW5:7,8")
	PORT_DIPSETTING(    0xc0, "100 Credits / 1 Pulse" )
	PORT_DIPSETTING(    0x40, "100 Credits / 5 Pulses" )
	PORT_DIPSETTING(    0x80, "100 Credits / 10 Pulses" )
	PORT_DIPSETTING(    0x00, "100 Credits / 100 Pulses" )
INPUT_PORTS_END

static INPUT_PORTS_START( chry10 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_X) PORT_NAME("Stop 2 / Big / Bonus Game")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_B) PORT_NAME("Blue Bet / D-UP / Card 3")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_Z) PORT_NAME("Stop 1 / Take")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_V) PORT_NAME("Red Bet / Card 2")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_C) PORT_NAME("Stop 3 / Small / Info / Card 1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_CODE(KEYCODE_N) PORT_NAME("Start / Stop All / Card 4")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)	/* Coin B */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_IMPULSE(2)	/* Coin D */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2)	/* Coin C */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)	/* Coin A */

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Out / Attendant")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Settings")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Stats")

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "Game Style" )		PORT_DIPLOCATION("DSW1:1")	/* OK */
	PORT_DIPSETTING(    0x01, "Tokens" )
	PORT_DIPSETTING(    0x00, "Ticket" )
	PORT_DIPNAME( 0x02, 0x02, "Hopper Out Switch" )	PORT_DIPLOCATION("DSW1:2")	/* OK */
	PORT_DIPSETTING(    0x02, "Active Low" )
	PORT_DIPSETTING(    0x00, "Active High" )
	PORT_DIPNAME( 0x04, 0x04, "Payout Mode" )		PORT_DIPLOCATION("DSW1:3")	/* OK */
	PORT_DIPSETTING(    0x04, "Payout Switch" )
	PORT_DIPSETTING(    0x00, "Automatic" )
	PORT_DIPNAME( 0x08, 0x00, "W-UP '7'" )			PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, "Loss" )
	PORT_DIPSETTING(    0x00, "Even" )
	PORT_DIPNAME( 0x10, 0x10, "W-UP Pay Rate" )		PORT_DIPLOCATION("DSW1:5")	/* OK */
	PORT_DIPSETTING(    0x10, "80%" )
	PORT_DIPSETTING(    0x00, "90%" )
	PORT_DIPNAME( 0x20, 0x20, "W-UP Game" )			PORT_DIPLOCATION("DSW1:6")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, "Bet Max" )			PORT_DIPLOCATION("DSW1:7,8")	/* OK */
	PORT_DIPSETTING(    0xc0, "8" )
	PORT_DIPSETTING(    0x80, "16" )
	PORT_DIPSETTING(    0x40, "32" )
	PORT_DIPSETTING(    0x00, "64" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x04, "Main Game Pay Rate" )	PORT_DIPLOCATION("DSW2:1,2,3")	/* OK */
	PORT_DIPSETTING(    0x00, "40%" )
	PORT_DIPSETTING(    0x01, "45%" )
	PORT_DIPSETTING(    0x02, "50%" )
	PORT_DIPSETTING(    0x03, "55%" )
	PORT_DIPSETTING(    0x04, "60%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x06, "70%" )
	PORT_DIPSETTING(    0x07, "80%" )
	PORT_DIPNAME( 0x18, 0x00, "Hopper Limit" )			PORT_DIPLOCATION("DSW2:4,5")
	PORT_DIPSETTING(    0x18, "300" )
	PORT_DIPSETTING(    0x10, "500" )
	PORT_DIPSETTING(    0x08, "1000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x20, 0x00, "100 Odds Sound" )		PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Key-In Type" )			PORT_DIPLOCATION("DSW2:7")	/* OK */
	PORT_DIPSETTING(    0x40, "B-Type" )
	PORT_DIPSETTING(    0x00, "A-Type" )
	PORT_DIPNAME( 0x80, 0x00, "Center Super 7 Bet Limit" )	PORT_DIPLOCATION("DSW2:8")	/* related with DSW 4-6 */
	PORT_DIPSETTING(    0x80, "Unlimited" )
	PORT_DIPSETTING(    0x00, "Limited" )

	PORT_START("DSW3")	/* seems ok */
	PORT_DIPNAME( 0x03, 0x03, "Key In Rate" ) PORT_DIPLOCATION("DSW3:1,2")	/* OK */
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )		PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00) // A-Type
	PORT_DIPSETTING(    0x01, "1 Coin/20 Credits" )		PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x02, "1 Coin/50 Credits" )		PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x03, "1 Coin/100 Credits" )	PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )		PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40) // B-Type
	PORT_DIPSETTING(    0x01, "1 Coin/10 Credits" )		PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40)
	PORT_DIPSETTING(    0x02, "1 Coin/25 Credits" )		PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40)
	PORT_DIPSETTING(    0x03, "1 Coin/50 Credits" )		PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40)
	PORT_DIPNAME( 0x0c, 0x0c, "Coin A Rate" ) PORT_DIPLOCATION("DSW3:3,4")	/* OK */
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin/20 Credits" )
	PORT_DIPSETTING(    0x08, "1 Coin/50 Credits" )
	PORT_DIPSETTING(    0x0c, "1 Coin/100 Credits" )
	PORT_DIPNAME( 0x30, 0x30, "Coin D Rate" ) PORT_DIPLOCATION("DSW3:5,6")	/* OK */
	PORT_DIPSETTING(    0x30, "5 Coin/10 Credits" )				PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10) // C-Type
	PORT_DIPSETTING(    0x20, "2 Coin/10 Credits" )				PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x10, "1 Coin/10 Credits" )				PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x00, "1 Coin/20 Credits" )				PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x00, "1 Ticket/Coin / 50 Credits" )	PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x10, "1 Ticket/Coin / 100 Credits" )	PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x20, "1 Ticket/Coin / 200 Credits" )	PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x30, "1 Ticket/Coin / 500 Credits" )	PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00) // D-Type
	PORT_DIPNAME( 0xc0, 0xc0, "Coin C Rate" ) PORT_DIPLOCATION("DSW3:7,8")	/* OK */
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x40, "1 Coin/20 Credits" )
	PORT_DIPSETTING(    0x80, "1 Coin/50 Credits" )
	PORT_DIPSETTING(    0xc0, "1 Coin/100 Credits" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x07, 0x06, "Credit Limit" )				PORT_DIPLOCATION("DSW4:1,2,3")	/* OK */
	PORT_DIPSETTING(    0x07, "5,000" )
	PORT_DIPSETTING(    0x06, "10,000" )
	PORT_DIPSETTING(    0x05, "20,000" )
	PORT_DIPSETTING(    0x04, "30,000" )
	PORT_DIPSETTING(    0x03, "40,000" )
	PORT_DIPSETTING(    0x02, "50,000" )
	PORT_DIPSETTING(    0x01, "100,000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x08, 0x00, "Display Of Credit Limit" )	PORT_DIPLOCATION("DSW4:4")	/* OK */
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Type Of Coin D" )			PORT_DIPLOCATION("DSW4:5")	/* OK */
	PORT_DIPSETTING(    0x10, "D-Type (Tokens)" )
	PORT_DIPSETTING(    0x00, "C-Type (Ticket)" )
	PORT_DIPNAME( 0x20, 0x00, "Bonus Play Min.Bet" )		PORT_DIPLOCATION("DSW4:6")	/* OK */
	PORT_DIPSETTING(    0x20, "16 Bet" )
	PORT_DIPSETTING(    0x00, "8 Bet" )
	PORT_DIPNAME( 0x40, 0x00, "Reel Speed" )				PORT_DIPLOCATION("DSW4:7")	/* OK */
	PORT_DIPSETTING(    0x40, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x80, 0x80, "Coin A Mode" )				PORT_DIPLOCATION("DSW4:8")	/* OK */
	PORT_DIPSETTING(    0x80, "Unexchange" )
	PORT_DIPSETTING(    0x00, "Exchange" )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unused ) )		PORT_DIPLOCATION("DSW5:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x00, "Coin In Limit" )			PORT_DIPLOCATION("DSW5:2,3")
	PORT_DIPSETTING(    0x06, "1000" )
	PORT_DIPSETTING(    0x04, "5000" )
	PORT_DIPSETTING(    0x02, "10000" )
	PORT_DIPSETTING(    0x00, "20000" )
	PORT_DIPNAME( 0x18, 0x10, "Coin Out Rate" )			PORT_DIPLOCATION("DSW5:4,5")
	PORT_DIPSETTING(    0x00, "100 Credits / 1 Pulse" )
	PORT_DIPSETTING(    0x08, "100 Credits / 5 Pulses" )
	PORT_DIPSETTING(    0x10, "100 Credits / 10 Pulses" )
	PORT_DIPSETTING(    0x18, "100 Credits / 100 Pulses" )
	PORT_DIPNAME( 0x20, 0x00, "W-UP Girl" )				PORT_DIPLOCATION("DSW5:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x40, "Coin In Rate" )			PORT_DIPLOCATION("DSW5:7,8")
	PORT_DIPSETTING(    0xc0, "100 Credits / 1 Pulse" )
	PORT_DIPSETTING(    0x40, "100 Credits / 5 Pulses" )
	PORT_DIPSETTING(    0x80, "100 Credits / 10 Pulses" )
	PORT_DIPSETTING(    0x00, "100 Credits / 100 Pulses" )
INPUT_PORTS_END

static INPUT_PORTS_START( ncb3 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_X) PORT_NAME("P1 - Stop 2 / Big / Bonus Game / Switch Controls")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_B) PORT_NAME("P1 - Blue Bet / D-UP / Card 3")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_Z) PORT_NAME("P1 - Stop 1 / Take")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_V) PORT_NAME("P1 - Red Bet / Card 2")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_C) PORT_NAME("P1 - Stop 3 / Small / Info / Card 1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_CODE(KEYCODE_N) PORT_NAME("P1 - Start / Stop All / Card 4")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)	/* Coin B */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_IMPULSE(2)	/* Coin D */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2)	/* Coin C */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)	/* Coin A */

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Out / Attendant")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Settings")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Stats")

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON8 )  PORT_CODE(KEYCODE_S) PORT_NAME("P2 - Stop 2 / Big / Bonus Game / Switch Controls")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_CODE(KEYCODE_G) PORT_NAME("P2 - Blue Bet / D-UP / Card 3")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON7 )  PORT_CODE(KEYCODE_A) PORT_NAME("P2 - Stop 1 / Take")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_CODE(KEYCODE_F) PORT_NAME("P2 - Red Bet / Card 2")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON9 )  PORT_CODE(KEYCODE_D) PORT_NAME("P2 - Stop 3 / Small / Info / Card 1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON12 ) PORT_CODE(KEYCODE_H) PORT_NAME("P2 - Start / Stop All / Card 4")

	/* to check DIP switches... */
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Game Style" )		PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x01, "Tokens" )
	PORT_DIPSETTING(    0x00, "Ticket" )
	PORT_DIPNAME( 0x02, 0x02, "Hopper Out Switch" )	PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x02, "Active Low" )
	PORT_DIPSETTING(    0x00, "Active High" )
	PORT_DIPNAME( 0x04, 0x04, "Payout Mode" )		PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, "Payout Switch" )
	PORT_DIPSETTING(    0x00, "Automatic" )
	PORT_DIPNAME( 0x08, 0x00, "W-UP '7'" )			PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, "Loss" )
	PORT_DIPSETTING(    0x00, "Even" )
	PORT_DIPNAME( 0x10, 0x10, "W-UP Pay Rate" )		PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x10, "80%" )
	PORT_DIPSETTING(    0x00, "90%" )
	PORT_DIPNAME( 0x20, 0x20, "W-UP Game" )			PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, "Bet Max" )			PORT_DIPLOCATION("DSW1:7,8")
	PORT_DIPSETTING(    0xc0, "8" )
	PORT_DIPSETTING(    0x80, "16" )
	PORT_DIPSETTING(    0x40, "32" )
	PORT_DIPSETTING(    0x00, "64" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x00, "Main Game Pay Rate" )	PORT_DIPLOCATION("DSW2:1,2,3")
	PORT_DIPSETTING(    0x00, "55%" )
	PORT_DIPSETTING(    0x01, "60%" )
	PORT_DIPSETTING(    0x02, "65%" )
	PORT_DIPSETTING(    0x03, "70%" )
	PORT_DIPSETTING(    0x04, "75%" )
	PORT_DIPSETTING(    0x05, "80%" )
	PORT_DIPSETTING(    0x06, "85%" )
	PORT_DIPSETTING(    0x07, "90%" )
	PORT_DIPNAME( 0x18, 0x00, "Hopper Limit" )			PORT_DIPLOCATION("DSW2:4,5")
	PORT_DIPSETTING(    0x18, "300" )
	PORT_DIPSETTING(    0x10, "500" )
	PORT_DIPSETTING(    0x08, "1000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x20, 0x00, "100 Odds Sound" )		PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Key-In Type" )			PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(    0x40, "B-Type" )
	PORT_DIPSETTING(    0x00, "A-Type" )
	PORT_DIPNAME( 0x80, 0x00, "Center Super 7 Bet Limit" )	PORT_DIPLOCATION("DSW2:8")	/* related with DSW 4-6 */
	PORT_DIPSETTING(    0x80, "Unlimited" )
	PORT_DIPSETTING(    0x00, "Limited" )

	PORT_START("DSW3")	/* some of these are wrong */
	PORT_DIPNAME( 0x03, 0x03, "Key In Rate" ) PORT_DIPLOCATION("DSW3:1,2")
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )		PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00) // A-Type
	PORT_DIPSETTING(    0x01, "1 Coin/20 Credits" )		PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x02, "1 Coin/50 Credits" )		PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x03, "1 Coin/100 Credits" )	PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )		PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40) // B-Type
	PORT_DIPSETTING(    0x01, "1 Coin/10 Credits" )		PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40)
	PORT_DIPSETTING(    0x02, "1 Coin/25 Credits" )		PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40)
	PORT_DIPSETTING(    0x03, "1 Coin/50 Credits" )		PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40)
	PORT_DIPNAME( 0x0c, 0x0c, "Coin A Rate" ) PORT_DIPLOCATION("DSW3:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0c, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x30, 0x30, "Coin D Rate" ) PORT_DIPLOCATION("DSW3:5,6")
	PORT_DIPSETTING(    0x30, DEF_STR( 5C_1C ) )				PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10) // C-Type
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )				PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )				PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )				PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x30, "1 Ticket/Coin / 100 Credits" )	PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00) // D-Type
	PORT_DIPSETTING(    0x20, "1 Ticket/Coin / 100 Credits" )	PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x10, "1 Ticket/Coin / 100 Credits" )	PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x00, "1 Ticket/Coin / 100 Credits" )	PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPNAME( 0xc0, 0xc0, "Coin C Rate" ) PORT_DIPLOCATION("DSW3:7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xc0, "1 Coin/10 Credits" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x07, 0x06, "Credit Limit" )				PORT_DIPLOCATION("DSW4:1,2,3")
	PORT_DIPSETTING(    0x07, "5,000" )
	PORT_DIPSETTING(    0x06, "10,000" )
	PORT_DIPSETTING(    0x05, "20,000" )
	PORT_DIPSETTING(    0x04, "30,000" )
	PORT_DIPSETTING(    0x03, "40,000" )
	PORT_DIPSETTING(    0x02, "50,000" )
	PORT_DIPSETTING(    0x01, "100,000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x08, 0x00, "Display Of Credit Limit" )	PORT_DIPLOCATION("DSW4:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Type Of Coin D" )			PORT_DIPLOCATION("DSW4:5")
	PORT_DIPSETTING(    0x10, "D-Type (Tokens)" )
	PORT_DIPSETTING(    0x00, "C-Type (Ticket)" )
	PORT_DIPNAME( 0x20, 0x00, "Bonus Play Min.Bet" )		PORT_DIPLOCATION("DSW4:6")
	PORT_DIPSETTING(    0x20, "16 Bet" )
	PORT_DIPSETTING(    0x00, "8 Bet" )
	PORT_DIPNAME( 0x40, 0x00, "Reel Speed" )				PORT_DIPLOCATION("DSW4:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x80, 0x80, "Coin A Mode" )				PORT_DIPLOCATION("DSW4:8")
	PORT_DIPSETTING(    0x80, "Unexchange" )
	PORT_DIPSETTING(    0x00, "Exchange" )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unused ) )		PORT_DIPLOCATION("DSW5:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x00, "Coin In Limit" )			PORT_DIPLOCATION("DSW5:2,3")
	PORT_DIPSETTING(    0x06, "1000" )
	PORT_DIPSETTING(    0x04, "5000" )
	PORT_DIPSETTING(    0x02, "10000" )
	PORT_DIPSETTING(    0x00, "20000" )
	PORT_DIPNAME( 0x18, 0x10, "Coin Out Rate" )			PORT_DIPLOCATION("DSW5:4,5")
	PORT_DIPSETTING(    0x00, "100 Credits / 1 Pulse" )
	PORT_DIPSETTING(    0x08, "100 Credits / 5 Pulses" )
	PORT_DIPSETTING(    0x10, "100 Credits / 10 Pulses" )
	PORT_DIPSETTING(    0x18, "100 Credits / 100 Pulses" )
	PORT_DIPNAME( 0x20, 0x00, "W-UP Girl" )				PORT_DIPLOCATION("DSW5:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x40, "Coin In Rate" )			PORT_DIPLOCATION("DSW5:7,8")
	PORT_DIPSETTING(    0xc0, "100 Credits / 1 Pulse" )
	PORT_DIPSETTING(    0x40, "100 Credits / 5 Pulses" )
	PORT_DIPSETTING(    0x80, "100 Credits / 10 Pulses" )
	PORT_DIPSETTING(    0x00, "100 Credits / 100 Pulses" )
INPUT_PORTS_END

static INPUT_PORTS_START( cb3a )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_X) PORT_NAME("P1 - Stop 2 / Big / Bonus Game / Switch Controls")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_B) PORT_NAME("P1 - Blue Bet / D-UP / Card 3")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_Z) PORT_NAME("P1 - Stop 1 / Take")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_V) PORT_NAME("P1 - Red Bet / Card 2")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_C) PORT_NAME("P1 - Stop 3 / Small / Info / Card 1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_CODE(KEYCODE_N) PORT_NAME("P1 - Start / Stop All / Card 4")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)	/* Coin B */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_IMPULSE(2)	/* Coin D */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2)	/* Coin C */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)	/* Coin A */

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Out / Attendant")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Settings")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Stats")

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON8 )  PORT_CODE(KEYCODE_S) PORT_NAME("P2 - Stop 2 / Big / Bonus Game / Switch Controls")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_CODE(KEYCODE_G) PORT_NAME("P2 - Blue Bet / D-UP / Card 3")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON7 )  PORT_CODE(KEYCODE_A) PORT_NAME("P2 - Stop 1 / Take")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_CODE(KEYCODE_F) PORT_NAME("P2 - Red Bet / Card 2")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON9 )  PORT_CODE(KEYCODE_D) PORT_NAME("P2 - Stop 3 / Small / Info / Card 1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON12 ) PORT_CODE(KEYCODE_H) PORT_NAME("P2 - Start / Stop All / Card 4")

	/* to check DIP switches... */
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Game Style" )		PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x01, "Tokens" )
	PORT_DIPSETTING(    0x00, "Ticket" )
	PORT_DIPNAME( 0x02, 0x02, "Hopper Out Switch" )	PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x02, "Active Low" )
	PORT_DIPSETTING(    0x00, "Active High" )
	PORT_DIPNAME( 0x04, 0x04, "Payout Mode" )		PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, "Payout Switch" )
	PORT_DIPSETTING(    0x00, "Automatic" )
	PORT_DIPNAME( 0x08, 0x00, "W-UP '7'" )			PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, "Loss" )
	PORT_DIPSETTING(    0x00, "Even" )
	PORT_DIPNAME( 0x10, 0x10, "W-UP Pay Rate" )		PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPSETTING(    0x10, "80%" )
	PORT_DIPNAME( 0x20, 0x20, "W-UP Game" )			PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, "Bet Max" )			PORT_DIPLOCATION("DSW1:7,8")
	PORT_DIPSETTING(    0xc0, "8" )
	PORT_DIPSETTING(    0x80, "16" )
	PORT_DIPSETTING(    0x40, "32" )
	PORT_DIPSETTING(    0x00, "64" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x00, "Main Game Pay Rate" )	PORT_DIPLOCATION("DSW2:1,2,3")
	PORT_DIPSETTING(    0x00, "55%" )
	PORT_DIPSETTING(    0x01, "60%" )
	PORT_DIPSETTING(    0x02, "65%" )
	PORT_DIPSETTING(    0x03, "70%" )
	PORT_DIPSETTING(    0x04, "75%" )
	PORT_DIPSETTING(    0x05, "80%" )
	PORT_DIPSETTING(    0x06, "85%" )
	PORT_DIPSETTING(    0x07, "90%" )
	PORT_DIPNAME( 0x18, 0x00, "Hopper Limit" )			PORT_DIPLOCATION("DSW2:4,5")
	PORT_DIPSETTING(    0x18, "300" )
	PORT_DIPSETTING(    0x10, "500" )
	PORT_DIPSETTING(    0x08, "1000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x20, 0x00, "100 Odds Sound" )		PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Key-In Type" )			PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(    0x40, "B-Type" )
	PORT_DIPSETTING(    0x00, "A-Type" )
	PORT_DIPNAME( 0x80, 0x00, "Center Super 7 Bet Limit" )	PORT_DIPLOCATION("DSW2:8")	/* related with DSW 4-6 */
	PORT_DIPSETTING(    0x80, "Unlimited" )
	PORT_DIPSETTING(    0x00, "Limited" )

	PORT_START("DSW3")	/* some of these are wrong */
	PORT_DIPNAME( 0x03, 0x03, "Key In Rate" ) PORT_DIPLOCATION("DSW3:1,2")
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )		PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00) // A-Type
	PORT_DIPSETTING(    0x01, "1 Coin/20 Credits" )		PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x02, "1 Coin/50 Credits" )		PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x03, "1 Coin/100 Credits" )	PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )		PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40) // B-Type
	PORT_DIPSETTING(    0x01, "1 Coin/10 Credits" )		PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40)
	PORT_DIPSETTING(    0x02, "1 Coin/25 Credits" )		PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40)
	PORT_DIPSETTING(    0x03, "1 Coin/50 Credits" )		PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40)
	PORT_DIPNAME( 0x0c, 0x0c, "Coin A Rate" ) PORT_DIPLOCATION("DSW3:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0c, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x30, 0x30, "Coin D Rate" ) PORT_DIPLOCATION("DSW3:5,6")
	PORT_DIPSETTING(    0x30, DEF_STR( 5C_1C ) )				PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10) // C-Type
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )				PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )				PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )				PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x30, "1 Ticket/Coin / 100 Credits" )	PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00) // D-Type
	PORT_DIPSETTING(    0x20, "1 Ticket/Coin / 100 Credits" )	PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x10, "1 Ticket/Coin / 100 Credits" )	PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x00, "1 Ticket/Coin / 100 Credits" )	PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPNAME( 0xc0, 0xc0, "Coin C Rate" ) PORT_DIPLOCATION("DSW3:7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xc0, "1 Coin/10 Credits" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x07, 0x06, "Credit Limit" )				PORT_DIPLOCATION("DSW4:1,2,3")
	PORT_DIPSETTING(    0x07, "5,000" )
	PORT_DIPSETTING(    0x06, "10,000" )
	PORT_DIPSETTING(    0x05, "20,000" )
	PORT_DIPSETTING(    0x04, "30,000" )
	PORT_DIPSETTING(    0x03, "40,000" )
	PORT_DIPSETTING(    0x02, "50,000" )
	PORT_DIPSETTING(    0x01, "100,000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x08, 0x00, "Display Of Credit Limit" )	PORT_DIPLOCATION("DSW4:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Type Of Coin D" )			PORT_DIPLOCATION("DSW4:5")
	PORT_DIPSETTING(    0x10, "D-Type (Tokens)" )
	PORT_DIPSETTING(    0x00, "C-Type (Ticket)" )
	PORT_DIPNAME( 0x20, 0x00, "Bonus Play Min.Bet" )		PORT_DIPLOCATION("DSW4:6")
	PORT_DIPSETTING(    0x20, "16 Bet" )
	PORT_DIPSETTING(    0x00, "8 Bet" )
	PORT_DIPNAME( 0x40, 0x00, "Reel Speed" )				PORT_DIPLOCATION("DSW4:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x80, 0x80, "Coin A Mode" )				PORT_DIPLOCATION("DSW4:8")
	PORT_DIPSETTING(    0x80, "Unexchange" )
	PORT_DIPSETTING(    0x00, "Exchange" )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unused ) )		PORT_DIPLOCATION("DSW5:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x00, "Coin In Limit" )			PORT_DIPLOCATION("DSW5:2,3")
	PORT_DIPSETTING(    0x06, "1000" )
	PORT_DIPSETTING(    0x04, "5000" )
	PORT_DIPSETTING(    0x02, "10000" )
	PORT_DIPSETTING(    0x00, "20000" )
	PORT_DIPNAME( 0x18, 0x10, "Coin Out Rate" )			PORT_DIPLOCATION("DSW5:4,5")
	PORT_DIPSETTING(    0x00, "100 Credits / 1 Pulse" )
	PORT_DIPSETTING(    0x08, "100 Credits / 5 Pulses" )
	PORT_DIPSETTING(    0x10, "100 Credits / 10 Pulses" )
	PORT_DIPSETTING(    0x18, "100 Credits / 100 Pulses" )
	PORT_DIPNAME( 0x20, 0x00, "W-UP Girl" )				PORT_DIPLOCATION("DSW5:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x40, "Coin In Rate" )			PORT_DIPLOCATION("DSW5:7,8")
	PORT_DIPSETTING(    0xc0, "100 Credits / 1 Pulse" )
	PORT_DIPSETTING(    0x40, "100 Credits / 5 Pulses" )
	PORT_DIPSETTING(    0x80, "100 Credits / 10 Pulses" )
	PORT_DIPSETTING(    0x00, "100 Credits / 100 Pulses" )
INPUT_PORTS_END

static INPUT_PORTS_START( lucky8 )
	PORT_START("IN0")	/* d800 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_CODE(KEYCODE_B) PORT_NAME("P1 - Big / Switch Controls")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_C) PORT_NAME("P1 - Double-Up")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_V) PORT_NAME("P1 - Take Score")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_Z) PORT_NAME("P1 - Bet")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_N) PORT_NAME("P1 - Small / Info")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_X) PORT_NAME("P1 - Start")

	PORT_START("IN1")	/* d801 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON12 ) PORT_CODE(KEYCODE_G) PORT_NAME("P2 - Big / Switch Controls")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_CODE(KEYCODE_D) PORT_NAME("P2 - Double-Up")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_CODE(KEYCODE_F) PORT_NAME("P2 - Take Score")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON13 ) PORT_CODE(KEYCODE_A) PORT_NAME("P2 - Bet")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON15 ) PORT_CODE(KEYCODE_H) PORT_NAME("P2 - Small / Info")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON14 ) PORT_CODE(KEYCODE_S) PORT_NAME("P2 - Start")

	PORT_START("IN2")	/* d802 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")	/* d810 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)	// Coin1?
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)	// Coin2?
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2)	// Coin3?
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_IMPULSE(2)	// Coin4?

	PORT_START("IN4")	/* d811 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Out / Attendant")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("Hopper")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Settings")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Stats")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "W-UP Pay Rate" )		PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x10, "80%" )
	PORT_DIPSETTING(    0x00, "90%" )
	PORT_DIPNAME( 0x20, 0x20, "W-UP Game" )			PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x07, "Main Game Pay Rate" )	PORT_DIPLOCATION("DSW2:1,2,3")
	PORT_DIPSETTING(    0x07, "80%" )
	PORT_DIPSETTING(    0x06, "83%" )
	PORT_DIPSETTING(    0x05, "86%" )
	PORT_DIPSETTING(    0x04, "89%" )
	PORT_DIPSETTING(    0x03, "92%" )
	PORT_DIPSETTING(    0x02, "95%" )
	PORT_DIPSETTING(    0x01, "98%" )
	PORT_DIPSETTING(    0x00, "101%" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Paytable Settings" )	PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(    0x40, "Low Pay" )
	PORT_DIPSETTING(    0x00, "High Pay" )
	PORT_DIPNAME( 0x80, 0x80, "W-UP Type" )	PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(    0x80, "Reels (automatic)" )
	PORT_DIPSETTING(    0x00, "Cards (Big/Small)" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW3:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW3:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW3:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x07, 0x07, "Key In" )			PORT_DIPLOCATION("DSW4:1,2,3")
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x01, "10" )
	PORT_DIPSETTING(    0x02, "20" )
	PORT_DIPSETTING(    0x03, "25" )
	PORT_DIPSETTING(    0x04, "40" )
	PORT_DIPSETTING(    0x05, "50" )
	PORT_DIPSETTING(    0x06, "60" )
	PORT_DIPSETTING(    0x07, "100" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW4:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW4:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW4:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW4:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW4:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( lucky8a )
	PORT_START("IN0")	/* d800 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_CODE(KEYCODE_B) PORT_NAME("P1 - Big / Switch Controls")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_C) PORT_NAME("P1 - Double-Up")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_V) PORT_NAME("P1 - Take Score")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_Z) PORT_NAME("P1 - Bet")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_N) PORT_NAME("P1 - Small / Info")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_X) PORT_NAME("P1 - Start")

	PORT_START("IN1")	/* d801 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON12 ) PORT_CODE(KEYCODE_G) PORT_NAME("P2 - Big / Switch Controls")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_CODE(KEYCODE_D) PORT_NAME("P2 - Double-Up")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_CODE(KEYCODE_F) PORT_NAME("P2 - Take Score")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON13 ) PORT_CODE(KEYCODE_A) PORT_NAME("P2 - Bet")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON15 ) PORT_CODE(KEYCODE_H) PORT_NAME("P2 - Small / Info")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON14 ) PORT_CODE(KEYCODE_S) PORT_NAME("P2 - Start")

	PORT_START("IN2")	/* d802 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")	/* d810 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)	// Coin1?
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)	// Coin2?
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2)	// Coin3?
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_IMPULSE(2)	// Coin4?

	PORT_START("IN4")	/* d811 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Out / Attendant")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("Hopper")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Settings")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Stats")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "W-UP Pay Rate" )		PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x10, "80%" )
	PORT_DIPSETTING(    0x00, "90%" )
	PORT_DIPNAME( 0x20, 0x20, "W-UP Game" )			PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x02, "Main Game Pay Rate" )	PORT_DIPLOCATION("DSW2:1,2,3")
	PORT_DIPSETTING(    0x07, "30%" )
	PORT_DIPSETTING(    0x06, "40%" )
	PORT_DIPSETTING(    0x05, "50%" )
	PORT_DIPSETTING(    0x04, "60%" )
	PORT_DIPSETTING(    0x03, "70%" )
	PORT_DIPSETTING(    0x02, "80%" )
	PORT_DIPSETTING(    0x01, "90%" )
	PORT_DIPSETTING(    0x00, "100%" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Paytable Settings" )	PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(    0x40, "Low Pay" )
	PORT_DIPSETTING(    0x00, "High Pay" )
	PORT_DIPNAME( 0x80, 0x80, "W-UP Type" )         PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(    0x80, "Reels (automatic)" )
	PORT_DIPSETTING(    0x00, "Cards (Big/Small)" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW3:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW3:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW3:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x07, 0x07, "Key In" )			PORT_DIPLOCATION("DSW4:1,2,3")
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x01, "10" )
	PORT_DIPSETTING(    0x02, "20" )
	PORT_DIPSETTING(    0x03, "25" )
	PORT_DIPSETTING(    0x04, "40" )
	PORT_DIPSETTING(    0x05, "50" )
	PORT_DIPSETTING(    0x06, "60" )
	PORT_DIPSETTING(    0x07, "100" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW4:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW4:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW4:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW4:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW4:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( ns8lines )
	PORT_START("IN0")	/* d800 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_CODE(KEYCODE_B) PORT_NAME("P1 - Big / Switch Controls")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_C) PORT_NAME("P1 - Double-Up")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_V) PORT_NAME("P1 - Take Score")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_Z) PORT_NAME("P1 - Bet")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_N) PORT_NAME("P1 - Small / Info")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_X) PORT_NAME("P1 - Start")

	PORT_START("IN1")	/* d801 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")	/* d802 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")	/* d810 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)	// Coin1?
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)	// Coin2?
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2)	// Coin3?
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_IMPULSE(2)	// Coin4?

	PORT_START("IN4")	/* d811 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Out / Attendant")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("Hopper")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Settings")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Stats")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "W-UP Pay Rate" )		PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x10, "60%" )
	PORT_DIPSETTING(    0x00, "75%" )
	PORT_DIPNAME( 0x20, 0x20, "W-UP Game" )			PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x07, "Main Game Pay Rate" )	PORT_DIPLOCATION("DSW2:1,2,3")
	PORT_DIPSETTING(    0x07, "58%" )
	PORT_DIPSETTING(    0x06, "62%" )
	PORT_DIPSETTING(    0x05, "66%" )
	PORT_DIPSETTING(    0x04, "70%" )
	PORT_DIPSETTING(    0x03, "74%" )
	PORT_DIPSETTING(    0x02, "78%" )
	PORT_DIPSETTING(    0x01, "82%" )
	PORT_DIPSETTING(    0x00, "86%" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Paytable Settings" )	PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(    0x40, "Low Pay" )
	PORT_DIPSETTING(    0x00, "High Pay" )
	PORT_DIPNAME( 0x80, 0x80, "Double-Up Type" )	PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(    0x80, "Reels (automatic)" )
	PORT_DIPSETTING(    0x00, "Cards (Big/Small)" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW3:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW3:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW3:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x07, 0x07, "Key In" )			PORT_DIPLOCATION("DSW4:1,2,3")
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x01, "10" )
	PORT_DIPSETTING(    0x02, "20" )
	PORT_DIPSETTING(    0x03, "25" )
	PORT_DIPSETTING(    0x04, "40" )
	PORT_DIPSETTING(    0x05, "50" )
	PORT_DIPSETTING(    0x06, "60" )
	PORT_DIPSETTING(    0x07, "100" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW4:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW4:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW4:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW4:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW4:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( ns8linew )
	PORT_START("IN0")	/* b800 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_CODE(KEYCODE_B) PORT_NAME("P1 - Big / Switch Controls")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_C) PORT_NAME("P1 - Double-Up")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_V) PORT_NAME("P1 - Take Score")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_Z) PORT_NAME("P1 - Bet")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_N) PORT_NAME("P1 - Small / Info")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_X) PORT_NAME("P1 - Start")

	PORT_START("IN1")	/* $b801 - P2 Controls... Leftover? Once switched all lamps turn off and no P2 big/small inputs */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON12 ) PORT_CODE(KEYCODE_G) PORT_NAME("P2 - Big / Switch Controls")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_CODE(KEYCODE_D) PORT_NAME("P2 - Double-Up")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_CODE(KEYCODE_F) PORT_NAME("P2 - Take Score")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON13 ) PORT_CODE(KEYCODE_A) PORT_NAME("P2 - Bet")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON15 ) PORT_CODE(KEYCODE_H) PORT_NAME("P2 - Small / Info")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON14 ) PORT_CODE(KEYCODE_S) PORT_NAME("P2 - Start")

	PORT_START("IN2")	/* $b802 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")	/* $b810 - Money in */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)	// Coin1?
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)	// Coin2?
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2)	// Coin3?
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_IMPULSE(2)	// Coin4?

	PORT_START("IN4")	/* $b811 - Service controls */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )  PORT_NAME("Key Out / Attendant")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_CODE(KEYCODE_8_PAD) PORT_NAME("Hopper")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Settings")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )    PORT_NAME("Stats")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "W-UP Pay Rate" )		PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x10, "60%" )
	PORT_DIPSETTING(    0x00, "80%" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, "Main Game Pay Rate" )	PORT_DIPLOCATION("DSW2:1,2")
	PORT_DIPSETTING(    0x03, "60%" )
	PORT_DIPSETTING(    0x02, "70%" )
	PORT_DIPSETTING(    0x01, "80%" )
	PORT_DIPSETTING(    0x00, "90%" )
	PORT_DIPNAME( 0x04, 0x04, "Double-Up" )			PORT_DIPLOCATION("DSW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW3:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW3:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "4th Coin" )			PORT_DIPLOCATION("DSW3:8")
	PORT_DIPSETTING(    0x80, "As Coin 4" )
	PORT_DIPSETTING(    0x00, "As Hopper Line" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x07, 0x07, "Key In" )			PORT_DIPLOCATION("DSW4:1,2,3")
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x01, "10" )
	PORT_DIPSETTING(    0x02, "20" )
	PORT_DIPSETTING(    0x03, "25" )
	PORT_DIPSETTING(    0x04, "40" )
	PORT_DIPSETTING(    0x05, "50" )
	PORT_DIPSETTING(    0x06, "60" )
	PORT_DIPSETTING(    0x07, "100" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW4:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW4:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW4:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW4:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW4:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( ladylinr )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )   PORT_IMPULSE(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3 )   PORT_IMPULSE(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Key In") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )   PORT_IMPULSE(2)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_H) PORT_NAME("Hopper Muenze (Hopper Coin")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_J) PORT_NAME("Hopper Voll (Hopper Fill)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) PORT_NAME("Abschreib (Payout)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Aufsteller (Supervisor)")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Buchhaltung (Bookkeeping)")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SLOT_STOP3 ) PORT_NAME("Hoch (High) / Stop 3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP ) PORT_NAME("Gamble (D-UP)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE ) PORT_NAME("Nehmen (Take)")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Setzen (Bet)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SLOT_STOP1 ) PORT_NAME("Tief (Low) / Stop 1 / Info")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SLOT_STOP2 ) PORT_NAME("Start / Stop 2")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, "Remote Credits" )	PORT_DIPLOCATION("DSW1:1,2")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, "10" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x03, "100" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Coin_A ) )	PORT_DIPLOCATION("DSW1:5")	/* OK */
	PORT_DIPSETTING(    0x00, "20 credits" )
	PORT_DIPSETTING(    0x10, "50 Credits" )
	PORT_DIPNAME( 0x20, 0x20, "Coin B & C" )		PORT_DIPLOCATION("DSW1:6")	/* OK */
	PORT_DIPSETTING(    0x00, "10 credits" )
	PORT_DIPSETTING(    0x20, "20 credits" )
	PORT_DIPNAME( 0x40, 0x40, "Reels Speed" )		PORT_DIPLOCATION("DSW1:7")	/* OK */
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x80, 0x00, "Input Test Mode" )	PORT_DIPLOCATION("DSW1:8")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

/*  there are 2 extra DSW banks...
    both are tied to a daughterboard, maybe hooked to a device.
    they are not related to the original hardware, and are not
    listed in the Input Test Mode.
*/
INPUT_PORTS_END

static INPUT_PORTS_START( kkojnoli )
	PORT_START("IN0")	/* d800 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_CODE(KEYCODE_B) PORT_NAME("P1 - Big / Switch Controls")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_C) PORT_NAME("P1 - Double-Up")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_V) PORT_NAME("P1 - Take Score")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_Z) PORT_NAME("P1 - Bet")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_N) PORT_NAME("P1 - Small / Info")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_X) PORT_NAME("P1 - Start")

	PORT_START("IN1")	/* d801 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON12 ) PORT_CODE(KEYCODE_G) PORT_NAME("P2 - Big / Switch Controls")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_CODE(KEYCODE_D) PORT_NAME("P2 - Double-Up")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_CODE(KEYCODE_F) PORT_NAME("P2 - Take Score")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON13 ) PORT_CODE(KEYCODE_A) PORT_NAME("P2 - Bet")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON15 ) PORT_CODE(KEYCODE_H) PORT_NAME("P2 - Small / Info")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON14 ) PORT_CODE(KEYCODE_S) PORT_NAME("P2 - Start")

	PORT_START("IN2")	/* d802 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")	/* d810 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)	// Coin1?
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)	// Coin2?
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2)	// Coin3?
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_IMPULSE(2)	// Coin4?

	PORT_START("IN4")	/* d811 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "W-UP Type" )			PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x80, "Reels (automatic)" )
	PORT_DIPSETTING(    0x00, "Flowers (Big/Small)" )
INPUT_PORTS_END

/* not verified */
static INPUT_PORTS_START( magical )
	PORT_START("IN0")	/* d800 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_CODE(KEYCODE_X) PORT_NAME("P1 - Big / Switch Controls")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_B) PORT_NAME("P1 - Double-Up")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_Z) PORT_NAME("P1 - Take Score")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_V) PORT_NAME("P1 - Bet")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_C) PORT_NAME("P1 - Small / Info")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_N) PORT_NAME("P1 - Start")

	PORT_START("IN1")	/* d801 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON12 ) PORT_CODE(KEYCODE_S) PORT_NAME("P2 - Big / Switch Controls")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_CODE(KEYCODE_G) PORT_NAME("P2 - Double-Up")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_CODE(KEYCODE_A) PORT_NAME("P2 - Take Score")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON13 ) PORT_CODE(KEYCODE_F) PORT_NAME("P2 - Bet")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON15 ) PORT_CODE(KEYCODE_D) PORT_NAME("P2 - Small / Info")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON14 ) PORT_CODE(KEYCODE_H) PORT_NAME("P2 - Start")

	PORT_START("IN2")	/* d802 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")	/* d810 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2)	// CoinD?
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)	// CoinC?
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)	// CoinA?

	PORT_START("IN4")	/* d811 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Out / Attendant")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("Hopper")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Settings")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Stats")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x00, "Coin A Rate" ) PORT_DIPLOCATION("DSW1:1,2,3")
	PORT_DIPSETTING(    0x05, "1" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x07, "5" )
	PORT_DIPSETTING(    0x04, "10" )
	PORT_DIPSETTING(    0x03, "20" )
	PORT_DIPSETTING(    0x02, "30" )
	PORT_DIPSETTING(    0x01, "40" )
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPNAME( 0x38, 0x00, "Key In" ) PORT_DIPLOCATION("DSW1:4,5,6") // aka 'Coin B'
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x18, "10" )
	PORT_DIPSETTING(    0x20, "20" )
	PORT_DIPSETTING(    0x28, "25" )
	PORT_DIPSETTING(    0x30, "40" )
	PORT_DIPSETTING(    0x38, "50" )
	PORT_DIPSETTING(    0x08, "60" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0xc0, 0x00, "Coin C Rate" ) PORT_DIPLOCATION("DSW1:7,8")
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPSETTING(    0x80, "10" )
	PORT_DIPSETTING(    0xc0, "25" )
	PORT_DIPSETTING(    0x00, "50" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x04, "Main Game Level" )			PORT_DIPLOCATION("DSW2:1,2,3")
	PORT_DIPSETTING(    0x00, "+4" )
	PORT_DIPSETTING(    0x01, "+3" )
	PORT_DIPSETTING(    0x02, "+2" )
	PORT_DIPSETTING(    0x03, "+1" )
	PORT_DIPSETTING(    0x04, "0" )
	PORT_DIPSETTING(    0x05, "-1" )
	PORT_DIPSETTING(    0x06, "-2" )
	PORT_DIPSETTING(    0x07, "-3" )
	PORT_DIPNAME( 0x08, 0x08, "Nudity / Strip" )	PORT_DIPLOCATION("DSW2:4") // verified
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, "Credit Limit" )	PORT_DIPLOCATION("DSW3:1,2")
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPSETTING(    0x01, "2000" )
	PORT_DIPSETTING(    0x02, "3000" )
	PORT_DIPSETTING(    0x03, "Unlimited?" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xf0, 0xf0, "Coin D Rate" )			PORT_DIPLOCATION("DSW3:5,6,7,8")
	PORT_DIPSETTING(    0x00, "10-1" )
	PORT_DIPSETTING(    0x10, "5-1" )
	PORT_DIPSETTING(    0x20, "5-2" )
	PORT_DIPSETTING(    0x30, "4-1" )
	PORT_DIPSETTING(    0x40, "4-3" )
	PORT_DIPSETTING(    0x50, "3-2" )
	PORT_DIPSETTING(    0x60, "3-1" )
	PORT_DIPSETTING(    0x70, "2-1" )
	PORT_DIPSETTING(    0x80, "2-3" )
	PORT_DIPSETTING(    0x90, "2-5" )
	PORT_DIPSETTING(    0xf0, "1-1" )
	PORT_DIPSETTING(    0xe0, "1-2" )
	PORT_DIPSETTING(    0xd0, "1-3" )
	PORT_DIPSETTING(    0xc0, "1-4" )
	PORT_DIPSETTING(    0xb0, "1-5" )
	PORT_DIPSETTING(    0xa0, "1-10" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW4:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW4:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Max Bet" )	PORT_DIPLOCATION("DSW4:3,4")
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x04, "16" )
	PORT_DIPSETTING(    0x08, "32" )
	PORT_DIPSETTING(    0x0c, "64" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW4:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW4:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW4:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW4:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( schery97 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SLOT_STOP_ALL ) PORT_NAME("Stop All / Big")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SLOT_STOP1 ) PORT_NAME("Stop 1 / D-UP")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SLOT_STOP3 ) PORT_NAME("Stop 3 / Take / Select Card")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Play (Bet)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SLOT_STOP2 ) PORT_NAME("Stop 2 / Small / Info")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )   PORT_IMPULSE(2) PORT_NAME("Note In")		/* Note In */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )	/* Key In */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )   PORT_IMPULSE(2)							/* Coin A */

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Out / Attendant")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Settings")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Stats")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x03, "Game Level (Difficulty)" )	PORT_DIPLOCATION("DSW1:1,2,3")	/* OK */
	PORT_DIPSETTING(    0x07, "Level 1" )
	PORT_DIPSETTING(    0x06, "Level 2" )
	PORT_DIPSETTING(    0x05, "Level 3" )
	PORT_DIPSETTING(    0x04, "Level 4" )
	PORT_DIPSETTING(    0x03, "Level 5" )
	PORT_DIPSETTING(    0x02, "Level 6" )
	PORT_DIPSETTING(    0x01, "Level 7" )
	PORT_DIPSETTING(    0x00, "Level 8" )
	PORT_DIPNAME( 0x38, 0x38, "Maximum Play" )				PORT_DIPLOCATION("DSW1:4,5,6")	/* OK */
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x08, "10" )
	PORT_DIPSETTING(    0x10, "24" )
	PORT_DIPSETTING(    0x18, "32" )
	PORT_DIPSETTING(    0x20, "40" )
	PORT_DIPSETTING(    0x28, "48" )
	PORT_DIPSETTING(    0x30, "64" )
	PORT_DIPSETTING(    0x38, "80" )
	PORT_DIPNAME( 0xc0, 0x80, "Minimum Play for Bonus" )	PORT_DIPLOCATION("DSW1:7,8")	/* OK */
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x80, "16" )
	PORT_DIPSETTING(    0xc0, "24" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Double-Up Game" )			PORT_DIPLOCATION("DSW2:1")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, "Skill Spinning" )			PORT_DIPLOCATION("DSW2:2")	/* OK (listed as Non-Stop spinning in the manual) */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x1c, 0x10, "Coin In" )					PORT_DIPLOCATION("DSW2:3,4,5")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x10, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x14, "1 Coin/20 Credits" )
	PORT_DIPSETTING(    0x18, "1 Coin/25 Credits" )
	PORT_DIPSETTING(    0x1c, "1 Coin/100 Credits" )
	PORT_DIPNAME( 0x60, 0x00, "Note In" )					PORT_DIPLOCATION("DSW2:6,7")	/* OK */
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPSETTING(    0x20, "200" )
	PORT_DIPSETTING(    0x40, "500" )
	PORT_DIPSETTING(    0x60, "1000" )
	PORT_DIPNAME( 0x80, 0x00, "WARNING: Always Off" )		PORT_DIPLOCATION("DSW2:8")	/* Listed that way in the manual */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x02, "Minimum Play to Start" )			PORT_DIPLOCATION("DSW3:1,2")	/* OK */
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "8" )
	PORT_DIPSETTING(    0x02, "10" )	/* 16 in the manual */
	PORT_DIPSETTING(    0x03, "16" )	/* 24 in the manual */
	PORT_DIPNAME( 0x0c, 0x08, "Max Coin In & Note In Point" )	PORT_DIPLOCATION("DSW3:3,4")	/* OK */
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPSETTING(    0x04, "5000" )
	PORT_DIPSETTING(    0x08, "10000" )
	PORT_DIPSETTING(    0x0c, "90000" )
	PORT_DIPNAME( 0xf0, 0x00, "Clear / Ticket Unit" )			PORT_DIPLOCATION("DSW3:5,6,7,8")	/* OK */
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x30, "10" )
	PORT_DIPSETTING(    0x40, "15" )
	PORT_DIPSETTING(    0x50, "20" )
	PORT_DIPSETTING(    0x60, "25" )
	PORT_DIPSETTING(    0x70, "30" )
	PORT_DIPSETTING(    0x80, "40" )
	PORT_DIPSETTING(    0x90, "50" )
	PORT_DIPSETTING(    0xa0, "60" )
	PORT_DIPSETTING(    0xb0, "75" )
	PORT_DIPSETTING(    0xc0, "80" )
	PORT_DIPSETTING(    0xd0, "100" )
	PORT_DIPSETTING(    0xe0, "200" )
	PORT_DIPSETTING(    0xf0, "500" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x01, "Check Account" )							PORT_DIPLOCATION("DSW4:1")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x00, "Show In Confirm Screen" )				PORT_DIPLOCATION("DSW4:2")	/* OK */
	PORT_DIPSETTING(    0x00, "Level of Difficulty" )	/* percentage in the manual */
	PORT_DIPSETTING(    0x02, "Percentage" )			/* level of difficulty in the manual */
	PORT_DIPNAME( 0x04, 0x00, "Initial Bonus Settings After Reset" )	PORT_DIPLOCATION("DSW4:3")	/* OK (need a reset after change) */
	PORT_DIPSETTING(    0x00, "Type 1" )
	PORT_DIPSETTING(    0x04, "Type 2" )
	PORT_DIPNAME( 0x08, 0x08, "Bonus Accumulation" )					PORT_DIPLOCATION("DSW4:4")	/* OK (need a reset after change) */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, "Auto Ticket Dispense" )					PORT_DIPLOCATION("DSW4:5")	/* OK (need a reset after change) */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0xe0, 0xe0, "Ticket Dispense Mode" )					PORT_DIPLOCATION("DSW4:6,7,8")	/* OK */
	PORT_DIPSETTING(    0xe0, "Continuous" )
	PORT_DIPSETTING(    0xc0, "Max 1 Ticket Per Game" )
	PORT_DIPSETTING(    0xa0, "Max 2 Ticket Per Game" )
	PORT_DIPSETTING(    0x80, "Max 3 Ticket Per Game" )
	PORT_DIPSETTING(    0x60, "Max 4 Ticket Per Game" )
	PORT_DIPSETTING(    0x40, "Max 5 Ticket Per Game" )
	PORT_DIPSETTING(    0x20, "Max 8 Ticket Per Game" )
	PORT_DIPSETTING(    0x00, "Max 10 Ticket Per Game" )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unused ) )									PORT_DIPLOCATION("DSW5:1")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	/* DIP switches 2 to 5 work only for version 3.51 */
	PORT_DIPNAME( 0x02, 0x00, "Limit Score of Each Game to Max 10x Bet or $5.00" )	PORT_DIPLOCATION("DSW5:2")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )		PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )		PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( Unused ) )	PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x02, DEF_STR( Unused ) )	PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPNAME( 0x04, 0x00, "Play Remaining Score when No Credit" )				PORT_DIPLOCATION("DSW5:3")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )		PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )		PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( Unused ) )	PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x04, DEF_STR( Unused ) )	PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPNAME( 0x08, 0x00, "Reset Remaining Score to Zero" )						PORT_DIPLOCATION("DSW5:4")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )		PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )		PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( Unused ) )	PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x08, DEF_STR( Unused ) )	PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPNAME( 0x10, 0x00, "Ticket Dispense from Score" )						PORT_DIPLOCATION("DSW5:5")	/* OK */
	PORT_DIPSETTING(    0x00, "Use TDDD" )			PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x10, "Use Interface" )		PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( Unused ) )	PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x10, DEF_STR( Unused ) )	PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPNAME( 0x20, 0x20, "Reel Speed (ver 2.3)" )								PORT_DIPLOCATION("DSW5:6")	/* OK (turn the machine off/on after change) */
	PORT_DIPSETTING(    0x20, "Slow" )
	PORT_DIPSETTING(    0x00, "Fast" )
	PORT_DIPNAME( 0x40, 0x00, "Talking (ver 2.1)" )									PORT_DIPLOCATION("DSW5:7")	/* OK (turn the machine off/on after change) */
	PORT_DIPSETTING(    0x40, "Very Little (only sounds)" )
	PORT_DIPSETTING(    0x00, "Full (sounds & speech)" )
	PORT_DIPNAME( 0x80, 0x00, "Count Game to Issue Ticket" )						PORT_DIPLOCATION("DSW5:8")	/* OK (turn the machine off/on after change) */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )
INPUT_PORTS_END

static INPUT_PORTS_START( nfb96 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SLOT_STOP_ALL ) PORT_NAME("Stop All / Big")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SLOT_STOP1 ) PORT_NAME("Stop 1 / D-UP")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SLOT_STOP3 ) PORT_NAME("Stop 3 / Take / Select Card")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Play (Bet)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SLOT_STOP2 ) PORT_NAME("Stop 2 / Small / Info")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )   PORT_IMPULSE(2) PORT_NAME("Note In")		/* Note In */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )	/* Key In */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )   PORT_IMPULSE(2)							/* Coin A */

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Out / Attendant")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Settings")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Stats")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x03, "Game Level (Difficulty)" )	PORT_DIPLOCATION("DSW1:1,2,3")	/* OK */
	PORT_DIPSETTING(    0x07, "Level 1" )
	PORT_DIPSETTING(    0x06, "Level 2" )
	PORT_DIPSETTING(    0x05, "Level 3" )
	PORT_DIPSETTING(    0x04, "Level 4" )
	PORT_DIPSETTING(    0x03, "Level 5" )
	PORT_DIPSETTING(    0x02, "Level 6" )
	PORT_DIPSETTING(    0x01, "Level 7" )
	PORT_DIPSETTING(    0x00, "Level 8" )
	PORT_DIPNAME( 0x38, 0x38, "Maximum Play" )				PORT_DIPLOCATION("DSW1:4,5,6")	/* OK */
	PORT_DIPSETTING(    0x00, "10" )	PORT_CONDITION("DSW5",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x08, "20" )	PORT_CONDITION("DSW5",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x10, "30" )	PORT_CONDITION("DSW5",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x18, "40" )	PORT_CONDITION("DSW5",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x00, "8" )		PORT_CONDITION("DSW5",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x08, "16" )	PORT_CONDITION("DSW5",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x10, "24" )	PORT_CONDITION("DSW5",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x18, "32" )	PORT_CONDITION("DSW5",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x20, "40" )
	PORT_DIPSETTING(    0x28, "48" )
	PORT_DIPSETTING(    0x30, "64" )
	PORT_DIPSETTING(    0x38, "80" )
	PORT_DIPNAME( 0xc0, 0x80, "Minimum Play for Bonus" )	PORT_DIPLOCATION("DSW1:7,8")	/* OK */
	PORT_DIPSETTING(    0x00, "10" )	PORT_CONDITION("DSW5",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x40, "20" )	PORT_CONDITION("DSW5",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x80, "30" )	PORT_CONDITION("DSW5",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0xc0, "40" )	PORT_CONDITION("DSW5",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x00, "8" )		PORT_CONDITION("DSW5",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x40, "16" )	PORT_CONDITION("DSW5",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x80, "24" )	PORT_CONDITION("DSW5",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0xc0, "32" )	PORT_CONDITION("DSW5",0x10,PORTCOND_EQUALS,0x10)

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Double-Up Game" )			PORT_DIPLOCATION("DSW2:1")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, "Skill Spinning" )			PORT_DIPLOCATION("DSW2:2")	/* OK (listed as Non-Stop spinning in the manual) */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x1c, 0x10, "Coin In" )					PORT_DIPLOCATION("DSW2:3,4,5")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x10, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x14, "1 Coin/20 Credits" )
	PORT_DIPSETTING(    0x18, "1 Coin/25 Credits" )
	PORT_DIPSETTING(    0x1c, "1 Coin/100 Credits" )
	PORT_DIPNAME( 0x60, 0x00, "Note In" )					PORT_DIPLOCATION("DSW2:6,7")	/* OK */
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPSETTING(    0x20, "200" )
	PORT_DIPSETTING(    0x40, "500" )
	PORT_DIPSETTING(    0x60, "1000" )
	PORT_DIPNAME( 0x80, 0x00, "WARNING: Always Off" )		PORT_DIPLOCATION("DSW2:8")	/* Listed that way in the manual */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x02, "Minimum Play to Start" )			PORT_DIPLOCATION("DSW3:1,2")	/* OK */
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "10" )	PORT_CONDITION("DSW5",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x02, "20" )	PORT_CONDITION("DSW5",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x03, "30" )	PORT_CONDITION("DSW5",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x01, "8" )		PORT_CONDITION("DSW5",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x02, "16" )	PORT_CONDITION("DSW5",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x03, "24" )	PORT_CONDITION("DSW5",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPNAME( 0x0c, 0x08, "Max Coin In & Note In Point" )	PORT_DIPLOCATION("DSW3:3,4")	/* OK */
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPSETTING(    0x04, "5000" )
	PORT_DIPSETTING(    0x08, "10000" )
	PORT_DIPSETTING(    0x0c, "90000" )
	PORT_DIPNAME( 0xf0, 0x00, "Clear / Ticket Unit" )			PORT_DIPLOCATION("DSW3:5,6,7,8")	/* OK */
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x30, "10" )
	PORT_DIPSETTING(    0x40, "15" )
	PORT_DIPSETTING(    0x50, "20" )
	PORT_DIPSETTING(    0x60, "25" )
	PORT_DIPSETTING(    0x70, "30" )
	PORT_DIPSETTING(    0x80, "40" )
	PORT_DIPSETTING(    0x90, "50" )
	PORT_DIPSETTING(    0xa0, "60" )
	PORT_DIPSETTING(    0xb0, "75" )
	PORT_DIPSETTING(    0xc0, "80" )
	PORT_DIPSETTING(    0xd0, "100" )
	PORT_DIPSETTING(    0xe0, "200" )
	PORT_DIPSETTING(    0xf0, "500" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x01, "Check Account" )							PORT_DIPLOCATION("DSW4:1")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x00, "Show In Confirm Screen" )				PORT_DIPLOCATION("DSW4:2")	/* OK */
	PORT_DIPSETTING(    0x00, "Level of Difficulty" )	/* percentage in the manual */
	PORT_DIPSETTING(    0x02, "Percentage" )			/* level of difficulty in the manual */
	PORT_DIPNAME( 0x04, 0x00, "Initial Bonus Settings After Reset" )	PORT_DIPLOCATION("DSW4:3")	/* OK (need a reset after change) */
	PORT_DIPSETTING(    0x00, "Type 1" )
	PORT_DIPSETTING(    0x04, "Type 2" )
	PORT_DIPNAME( 0x08, 0x08, "Bonus Accumulation" )					PORT_DIPLOCATION("DSW4:4")	/* OK (need a reset after change) */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, "Auto Ticket Dispense" )					PORT_DIPLOCATION("DSW4:5")	/* OK (need a reset after change) */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0xe0, 0xe0, "Ticket Dispense Mode" )					PORT_DIPLOCATION("DSW4:6,7,8")	/* OK */
	PORT_DIPSETTING(    0xe0, "Continuous" )
	PORT_DIPSETTING(    0xc0, "Max 1 Ticket Per Game" )
	PORT_DIPSETTING(    0xa0, "Max 2 Ticket Per Game" )
	PORT_DIPSETTING(    0x80, "Max 3 Ticket Per Game" )
	PORT_DIPSETTING(    0x60, "Max 4 Ticket Per Game" )
	PORT_DIPSETTING(    0x40, "Max 5 Ticket Per Game" )
	PORT_DIPSETTING(    0x20, "Max 8 Ticket Per Game" )
	PORT_DIPSETTING(    0x00, "Max 10 Ticket Per Game" )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unused ) )									PORT_DIPLOCATION("DSW5:1")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Limit Score of Each Game to Max 10x Bet or $5.00" )	PORT_DIPLOCATION("DSW5:2")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )		PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )		PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( Unused ) )	PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x02, DEF_STR( Unused ) )	PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPNAME( 0x04, 0x00, "Use Printer" )										PORT_DIPLOCATION("DSW5:3")	/* OK */
	PORT_DIPSETTING(    0x00, "No (Use TDDD)" )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, "Show Game Name & Odds Table" )						PORT_DIPLOCATION("DSW5:4")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, "Min-Max Bet Type" )									PORT_DIPLOCATION("DSW5:5")	/* OK */
	PORT_DIPSETTING(    0x10, "Base 8" )
	PORT_DIPSETTING(    0x00, "Base 10" )
	PORT_DIPNAME( 0x20, 0x20, "Play Score when no point left" )						PORT_DIPLOCATION("DSW5:6")	/* OK (turn the machine off/on after change) */
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )		PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )		PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x20, DEF_STR( Unused ) )	PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( Unused ) )	PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPNAME( 0x40, 0x00, "Reset Remaining Score when Game Over" )				PORT_DIPLOCATION("DSW5:7")	/* OK (turn the machine off/on after change) */
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )		PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )		PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x40, DEF_STR( Unused ) )	PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( Unused ) )	PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPNAME( 0x80, 0x00, "Advanced Count Game" )								PORT_DIPLOCATION("DSW5:8")	/* OK (turn the machine off/on after change) */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )
INPUT_PORTS_END

static INPUT_PORTS_START( nfb96tx )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SLOT_STOP_ALL ) PORT_NAME("Stop All / Big")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SLOT_STOP1 ) PORT_NAME("Stop 1 / D-UP")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SLOT_STOP3 ) PORT_NAME("Stop 3 / Take / Select Card")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Play (Bet)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SLOT_STOP2 ) PORT_NAME("Stop 2 / Small")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )   PORT_IMPULSE(2) PORT_NAME("Ticket In")		/* Ticket In */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )	/* Key In (not used)*/
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )   PORT_IMPULSE(2)							/* Coin A */

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_W) PORT_NAME("Key Out / Attendant")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_9) PORT_NAME("Settings")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_0) PORT_NAME("Stats")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x03, "Game Level (Difficulty)" )	PORT_DIPLOCATION("DSW1:1,2,3")	/* OK */
	PORT_DIPSETTING(    0x07, "Level 1" )
	PORT_DIPSETTING(    0x06, "Level 2" )
	PORT_DIPSETTING(    0x05, "Level 3" )
	PORT_DIPSETTING(    0x04, "Level 4" )
	PORT_DIPSETTING(    0x03, "Level 5" )
	PORT_DIPSETTING(    0x02, "Level 6" )
	PORT_DIPSETTING(    0x01, "Level 7" )
	PORT_DIPSETTING(    0x00, "Level 8" )
	PORT_DIPNAME( 0x38, 0x38, "Maximum Play" )				PORT_DIPLOCATION("DSW1:4,5,6")	/* OK */
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x08, "16" )
	PORT_DIPSETTING(    0x10, "24" )
	PORT_DIPSETTING(    0x18, "32" )
	PORT_DIPSETTING(    0x20, "40" )
	PORT_DIPSETTING(    0x28, "48" )
	PORT_DIPSETTING(    0x30, "64" )
	PORT_DIPSETTING(    0x38, "80" )
	PORT_DIPNAME( 0xc0, 0x80, "Minimum Play for Bonus" )	PORT_DIPLOCATION("DSW1:7,8")	/* OK */
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x40, "16" )
	PORT_DIPSETTING(    0x80, "24" )
	PORT_DIPSETTING(    0xc0, "32" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Double-Up Game" )			PORT_DIPLOCATION("DSW2:1")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, "Skill Spinning" )			PORT_DIPLOCATION("DSW2:2")	/* OK (listed as Non-Stop spinning in the manual) */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x04, "Coin In" )					PORT_DIPLOCATION("DSW2:3")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x04, "1 Coin/25 Credits" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )			PORT_DIPLOCATION("DSW2:4")	/* unknown */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )			PORT_DIPLOCATION("DSW2:5")	/* unknown */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Ticket In" )					PORT_DIPLOCATION("DSW2:6")	/* OK */
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPSETTING(    0x20, "500" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )			PORT_DIPLOCATION("DSW2:7")	/* unknown */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "WARNING: Always Off" )		PORT_DIPLOCATION("DSW2:8")	/* Listed that way in other manuals */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x02, "Minimum Play to Start" )			PORT_DIPLOCATION("DSW3:1,2")	/* OK */
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "8" )
	PORT_DIPSETTING(    0x02, "16" )
	PORT_DIPSETTING(    0x03, "24" )
	PORT_DIPNAME( 0x0c, 0x08, "Max Coin In & Note In Point" )	PORT_DIPLOCATION("DSW3:3,4")	/* OK */
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPSETTING(    0x04, "5000" )
	PORT_DIPSETTING(    0x08, "10000" )
	PORT_DIPSETTING(    0x0c, "90000" )
	PORT_DIPNAME( 0xf0, 0x00, "Clear / Ticket Unit" )			PORT_DIPLOCATION("DSW3:5,6,7,8")	/* OK */
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x30, "10" )
	PORT_DIPSETTING(    0x40, "15" )
	PORT_DIPSETTING(    0x50, "20" )
	PORT_DIPSETTING(    0x60, "25" )
	PORT_DIPSETTING(    0x70, "30" )
	PORT_DIPSETTING(    0x80, "40" )
	PORT_DIPSETTING(    0x90, "50" )
	PORT_DIPSETTING(    0xa0, "60" )
	PORT_DIPSETTING(    0xb0, "75" )
	PORT_DIPSETTING(    0xc0, "80" )
	PORT_DIPSETTING(    0xd0, "100" )
	PORT_DIPSETTING(    0xe0, "200" )
	PORT_DIPSETTING(    0xf0, "500" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x01, "Check Account" )							PORT_DIPLOCATION("DSW4:1")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x00, "Show In Confirm Screen" )				PORT_DIPLOCATION("DSW4:2")	/* OK */
	PORT_DIPSETTING(    0x00, "Level of Difficulty" )	/* percentage in the manual */
	PORT_DIPSETTING(    0x02, "Percentage" )			/* level of difficulty in the manual */
	PORT_DIPNAME( 0x04, 0x00, "Initial Bonus Settings After Reset" )	PORT_DIPLOCATION("DSW4:3")	/* OK (need a reset after change) */
	PORT_DIPSETTING(    0x00, "Type 1" )
	PORT_DIPSETTING(    0x04, "Type 2" )
	PORT_DIPNAME( 0x08, 0x08, "Bonus Accumulation" )					PORT_DIPLOCATION("DSW4:4")	/* OK (need a reset after change) */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, "Auto Ticket Dispense" )					PORT_DIPLOCATION("DSW4:5")	/* OK (need a reset after change) */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0xe0, 0xe0, "Ticket Dispense Mode" )					PORT_DIPLOCATION("DSW4:6,7,8")	/* OK */
	PORT_DIPSETTING(    0xe0, "Continuous" )
	PORT_DIPSETTING(    0xc0, "Max 1 Ticket Per Game" )
	PORT_DIPSETTING(    0xa0, "Max 2 Ticket Per Game" )
	PORT_DIPSETTING(    0x80, "Max 3 Ticket Per Game" )
	PORT_DIPSETTING(    0x60, "Max 4 Ticket Per Game" )
	PORT_DIPSETTING(    0x40, "Max 5 Ticket Per Game" )
	PORT_DIPSETTING(    0x20, "Max 8 Ticket Per Game" )
	PORT_DIPSETTING(    0x00, "Max 10 Ticket Per Game" )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unused ) )									PORT_DIPLOCATION("DSW5:1")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Limit Score of Each Game to Max 10x Bet or $5.00" )	PORT_DIPLOCATION("DSW5:2")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )		PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )		PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( Unused ) )	PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x02, DEF_STR( Unused ) )	PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPNAME( 0x04, 0x00, "Use Printer" )										PORT_DIPLOCATION("DSW5:3")	/* OK */
	PORT_DIPSETTING(    0x00, "Interfase" )
	PORT_DIPSETTING(    0x04, "Direct Driver" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )									PORT_DIPLOCATION("DSW5:4")	/* OK */
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )									PORT_DIPLOCATION("DSW5:5")	/* OK */
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Play Score when no point left" )						PORT_DIPLOCATION("DSW5:6")	/* OK (turn the machine off/on after change) */
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )		PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )		PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x20, DEF_STR( Unused ) )	PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( Unused ) )	PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPNAME( 0x40, 0x00, "Reset Remaining Score when Game Over" )				PORT_DIPLOCATION("DSW5:7")	/* OK (turn the machine off/on after change) */
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )		PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )		PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x40, DEF_STR( Unused ) )	PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( Unused ) )	PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPNAME( 0x80, 0x00, "Advanced Count Game" )								PORT_DIPLOCATION("DSW5:8")	/* OK (turn the machine off/on after change) */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )
INPUT_PORTS_END

static INPUT_PORTS_START( roypok96 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Big")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP ) PORT_NAME("D-UP")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE ) PORT_NAME("Take / Select Card")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Play (Bet)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) PORT_NAME("Small / Info")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )   PORT_NAME("Note In")		/* Note In */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )   PORT_IMPULSE(2)			/* Coin A */

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Out / Attendant")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Settings")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Stats")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x03, "Game Rate" )					PORT_DIPLOCATION("DSW1:1,2,3")	/* OK */
	PORT_DIPSETTING(    0x07, "90%" )
	PORT_DIPSETTING(    0x06, "85%" )
	PORT_DIPSETTING(    0x05, "80%" )
	PORT_DIPSETTING(    0x04, "75%" )
	PORT_DIPSETTING(    0x03, "70%" )
	PORT_DIPSETTING(    0x02, "65%" )
	PORT_DIPSETTING(    0x01, "60%" )
	PORT_DIPSETTING(    0x00, "55%" )
	PORT_DIPNAME( 0x38, 0x38, "Maximum Play" )				PORT_DIPLOCATION("DSW1:4,5,6")	/* OK */
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x08, "16" )
	PORT_DIPSETTING(    0x10, "24" )
	PORT_DIPSETTING(    0x18, "32" )
	PORT_DIPSETTING(    0x20, "40" )
	PORT_DIPSETTING(    0x28, "48" )
	PORT_DIPSETTING(    0x30, "64" )
	PORT_DIPSETTING(    0x38, "80" )
	PORT_DIPNAME( 0xc0, 0x80, "Minimum Play for Bonus" )	PORT_DIPLOCATION("DSW1:7,8")	/* OK */
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x40, "16" )
	PORT_DIPSETTING(    0x80, "24" )
	PORT_DIPSETTING(    0xc0, "32" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Double-Up Game" )			PORT_DIPLOCATION("DSW2:1")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )			PORT_DIPLOCATION("DSW2:2")	/* unknown */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x1c, 0x10, "Coin In" )					PORT_DIPLOCATION("DSW2:3,4,5")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x10, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x14, "1 Coin/20 Credits" )
	PORT_DIPSETTING(    0x18, "1 Coin/25 Credits" )
	PORT_DIPSETTING(    0x1c, "1 Coin/100 Credits" )
	PORT_DIPNAME( 0x60, 0x00, "Note In" )					PORT_DIPLOCATION("DSW2:6,7")	/* OK */
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPSETTING(    0x20, "200" )
	PORT_DIPSETTING(    0x40, "500" )
	PORT_DIPSETTING(    0x60, "1000" )
	PORT_DIPNAME( 0x80, 0x00, "WARNING: Always Off" )		PORT_DIPLOCATION("DSW2:8")	/* Listed that way in the manual */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x02, "Minimum Play to Start" )			PORT_DIPLOCATION("DSW3:1,2")	/* OK */
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "8" )
	PORT_DIPSETTING(    0x02, "16" )
	PORT_DIPSETTING(    0x03, "24" )
	PORT_DIPNAME( 0x0c, 0x08, "Max Coin In & Note In Point" )	PORT_DIPLOCATION("DSW3:3,4")	/* OK */
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPSETTING(    0x04, "5000" )
	PORT_DIPSETTING(    0x08, "10000" )
	PORT_DIPSETTING(    0x0c, "90000" )
	PORT_DIPNAME( 0xf0, 0x00, "Clear / Ticket Unit" )			PORT_DIPLOCATION("DSW3:5,6,7,8")	/* OK */
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x30, "10" )
	PORT_DIPSETTING(    0x40, "20" )
	PORT_DIPSETTING(    0x50, "25" )
	PORT_DIPSETTING(    0x60, "30" )
	PORT_DIPSETTING(    0x70, "40" )
	PORT_DIPSETTING(    0x80, "50" )
	PORT_DIPSETTING(    0x90, "60" )
	PORT_DIPSETTING(    0xa0, "80" )
	PORT_DIPSETTING(    0xb0, "100" )
	PORT_DIPSETTING(    0xc0, "200" )
	PORT_DIPSETTING(    0xd0, "300" )
	PORT_DIPSETTING(    0xe0, "400" )
	PORT_DIPSETTING(    0xf0, "500" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x01, "Check Account" )							PORT_DIPLOCATION("DSW4:1")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )						PORT_DIPLOCATION("DSW4:2")	/* unknown */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Initial Bonus Settings After Reset" )	PORT_DIPLOCATION("DSW4:3")	/* not checked */
	PORT_DIPSETTING(    0x00, "Type 1" )
	PORT_DIPSETTING(    0x04, "Type 2" )
	PORT_DIPNAME( 0x08, 0x08, "Bonus Accumulation" )					PORT_DIPLOCATION("DSW4:4")	/* not checked */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, "Auto Ticket Dispense" )					PORT_DIPLOCATION("DSW4:5")	/* not checked */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0xe0, 0xe0, "Ticket Dispense Mode" )					PORT_DIPLOCATION("DSW4:6,7,8")	/* OK */
	PORT_DIPSETTING(    0xe0, "Continuous" )
	PORT_DIPSETTING(    0xc0, "Max 1 Ticket Per Game" )
	PORT_DIPSETTING(    0xa0, "Max 2 Ticket Per Game" )
	PORT_DIPSETTING(    0x80, "Max 3 Ticket Per Game" )
	PORT_DIPSETTING(    0x60, "Max 4 Ticket Per Game" )
	PORT_DIPSETTING(    0x40, "Max 5 Ticket Per Game" )
	PORT_DIPSETTING(    0x20, "Max 8 Ticket Per Game" )
	PORT_DIPSETTING(    0x00, "Max 10 Ticket Per Game" )

	PORT_START("DSW5")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( roypok96a )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Big")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP ) PORT_NAME("D-UP")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE ) PORT_NAME("Take / Select Card")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Play (Bet)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) PORT_NAME("Small / Info")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )   PORT_NAME("Note In")		/* Note In */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )   PORT_IMPULSE(2)			/* Coin A */

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Out / Attendant")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Settings")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Stats")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x03, "Game Level (Difficulty)" )	PORT_DIPLOCATION("DSW1:1,2,3")	/* OK */
	PORT_DIPSETTING(    0x07, "Level 1" )
	PORT_DIPSETTING(    0x06, "Level 2" )
	PORT_DIPSETTING(    0x05, "Level 3" )
	PORT_DIPSETTING(    0x04, "Level 4" )
	PORT_DIPSETTING(    0x03, "Level 5" )
	PORT_DIPSETTING(    0x02, "Level 6" )
	PORT_DIPSETTING(    0x01, "Level 7" )
	PORT_DIPSETTING(    0x00, "Level 8" )
	PORT_DIPNAME( 0x38, 0x38, "Maximum Play" )				PORT_DIPLOCATION("DSW1:4,5,6")	/* OK */
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x08, "16" )
	PORT_DIPSETTING(    0x10, "24" )
	PORT_DIPSETTING(    0x18, "32" )
	PORT_DIPSETTING(    0x20, "40" )
	PORT_DIPSETTING(    0x28, "48" )
	PORT_DIPSETTING(    0x30, "64" )
	PORT_DIPSETTING(    0x38, "80" )
	PORT_DIPNAME( 0xc0, 0x80, "Minimum Play for Bonus" )	PORT_DIPLOCATION("DSW1:7,8")	/* OK */
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x40, "16" )
	PORT_DIPSETTING(    0x80, "24" )
	PORT_DIPSETTING(    0xc0, "32" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Double-Up Game" )			PORT_DIPLOCATION("DSW2:1")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )			PORT_DIPLOCATION("DSW2:2")	/* unknown */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x1c, 0x10, "Coin In" )					PORT_DIPLOCATION("DSW2:3,4,5")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x10, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x14, "1 Coin/20 Credits" )
	PORT_DIPSETTING(    0x18, "1 Coin/25 Credits" )
	PORT_DIPSETTING(    0x1c, "1 Coin/100 Credits" )
	PORT_DIPNAME( 0x60, 0x00, "Note In" )					PORT_DIPLOCATION("DSW2:6,7")	/* OK */
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPSETTING(    0x20, "200" )
	PORT_DIPSETTING(    0x40, "500" )
	PORT_DIPSETTING(    0x60, "1000" )
	PORT_DIPNAME( 0x80, 0x00, "WARNING: Always Off" )		PORT_DIPLOCATION("DSW2:8")	/* Listed that way in the manual */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x02, "Minimum Play to Start" )			PORT_DIPLOCATION("DSW3:1,2")	/* OK */
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "8" )
	PORT_DIPSETTING(    0x02, "10" )
	PORT_DIPSETTING(    0x03, "16" )
	PORT_DIPNAME( 0x0c, 0x08, "Max Coin In & Note In Point" )	PORT_DIPLOCATION("DSW3:3,4")	/* OK */
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPSETTING(    0x04, "5000" )
	PORT_DIPSETTING(    0x08, "10000" )
	PORT_DIPSETTING(    0x0c, "90000" )
	PORT_DIPNAME( 0xf0, 0x00, "Clear / Ticket Unit" )			PORT_DIPLOCATION("DSW3:5,6,7,8")	/* OK */
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x30, "10" )
	PORT_DIPSETTING(    0x40, "20" )
	PORT_DIPSETTING(    0x50, "25" )
	PORT_DIPSETTING(    0x60, "30" )
	PORT_DIPSETTING(    0x70, "40" )
	PORT_DIPSETTING(    0x80, "50" )
	PORT_DIPSETTING(    0x90, "60" )
	PORT_DIPSETTING(    0xa0, "80" )
	PORT_DIPSETTING(    0xb0, "100" )
	PORT_DIPSETTING(    0xc0, "200" )
	PORT_DIPSETTING(    0xd0, "300" )
	PORT_DIPSETTING(    0xe0, "400" )
	PORT_DIPSETTING(    0xf0, "500" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x01, "Check Account" )							PORT_DIPLOCATION("DSW4:1")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )						PORT_DIPLOCATION("DSW4:2")	/* unknown */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Initial Bonus Settings After Reset" )	PORT_DIPLOCATION("DSW4:3")	/* not checked */
	PORT_DIPSETTING(    0x00, "Type 1" )
	PORT_DIPSETTING(    0x04, "Type 2" )
	PORT_DIPNAME( 0x08, 0x08, "Bonus Accumulation" )					PORT_DIPLOCATION("DSW4:4")	/* not checked */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, "Auto Ticket Dispense" )					PORT_DIPLOCATION("DSW4:5")	/* not checked */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0xe0, 0xe0, "Ticket Dispense Mode" )					PORT_DIPLOCATION("DSW4:6,7,8")	/* OK */
	PORT_DIPSETTING(    0xe0, "Continuous" )
	PORT_DIPSETTING(    0xc0, "Max 1 Ticket Per Game" )
	PORT_DIPSETTING(    0xa0, "Max 2 Ticket Per Game" )
	PORT_DIPSETTING(    0x80, "Max 3 Ticket Per Game" )
	PORT_DIPSETTING(    0x60, "Max 4 Ticket Per Game" )
	PORT_DIPSETTING(    0x40, "Max 5 Ticket Per Game" )
	PORT_DIPSETTING(    0x20, "Max 8 Ticket Per Game" )
	PORT_DIPSETTING(    0x00, "Max 10 Ticket Per Game" )

	PORT_START("DSW5")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( pokonl97 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_Z) PORT_NAME("Hold 1 / Big / Bet 10")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_X) PORT_NAME("Hold 2 / D-UP")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_V) PORT_NAME("Hold 4 / Take / Select Card")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_B) PORT_NAME("Hold 5 / Play (Bet 1)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_C) PORT_NAME("Hold 3 / Small / End")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_CODE(KEYCODE_N) PORT_NAME("Start / Draw")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )   PORT_NAME("Note In")		/* Note In */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )   PORT_IMPULSE(2)			/* Coin A */

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Out / Attendant")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Settings")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Stats")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x03, "Game Level (Difficulty)" )	PORT_DIPLOCATION("DSW1:1,2,3")	/* OK */
	PORT_DIPSETTING(    0x07, "Level 1" )
	PORT_DIPSETTING(    0x06, "Level 2" )
	PORT_DIPSETTING(    0x05, "Level 3" )
	PORT_DIPSETTING(    0x04, "Level 4" )
	PORT_DIPSETTING(    0x03, "Level 5" )
	PORT_DIPSETTING(    0x02, "Level 6" )
	PORT_DIPSETTING(    0x01, "Level 7" )
	PORT_DIPSETTING(    0x00, "Level 8" )
	PORT_DIPNAME( 0x38, 0x38, "Maximum Play" )				PORT_DIPLOCATION("DSW1:4,5,6")	/* OK */
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPSETTING(    0x08, "20" )
	PORT_DIPSETTING(    0x10, "30" )
	PORT_DIPSETTING(    0x18, "40" )
	PORT_DIPSETTING(    0x20, "50" )
	PORT_DIPSETTING(    0x28, "60" )
	PORT_DIPSETTING(    0x30, "70" )
	PORT_DIPSETTING(    0x38, "80" )
	PORT_DIPNAME( 0xc0, 0x80, "Minimum Play for Bonus" )	PORT_DIPLOCATION("DSW1:7,8")	/* OK */
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x40, "16" )
	PORT_DIPSETTING(    0x80, "24" )
	PORT_DIPSETTING(    0xc0, "32" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Double-Up Game" )			PORT_DIPLOCATION("DSW2:1")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )			PORT_DIPLOCATION("DSW2:2")	/* unknown */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x1c, 0x10, "Coin In" )					PORT_DIPLOCATION("DSW2:3,4,5")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x10, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x14, "1 Coin/20 Credits" )
	PORT_DIPSETTING(    0x18, "1 Coin/25 Credits" )
	PORT_DIPSETTING(    0x1c, "1 Coin/100 Credits" )
	PORT_DIPNAME( 0x60, 0x00, "Note In" )					PORT_DIPLOCATION("DSW2:6,7")	/* OK */
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPSETTING(    0x20, "200" )
	PORT_DIPSETTING(    0x40, "500" )
	PORT_DIPSETTING(    0x60, "1000" )
	PORT_DIPNAME( 0x80, 0x00, "WARNING: Always Off" )		PORT_DIPLOCATION("DSW2:8")	/* Listed that way in the manual */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x02, "Minimum Play to Start" )			PORT_DIPLOCATION("DSW3:1,2")	/* OK */
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x03, "8" )
	PORT_DIPNAME( 0x0c, 0x08, "Max Coin In & Note In Point" )	PORT_DIPLOCATION("DSW3:3,4")	/* OK */
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPSETTING(    0x04, "5000" )
	PORT_DIPSETTING(    0x08, "10000" )
	PORT_DIPSETTING(    0x0c, "90000" )
	PORT_DIPNAME( 0xf0, 0x00, "Clear / Ticket Unit" )			PORT_DIPLOCATION("DSW3:5,6,7,8")	/* OK */
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x30, "10" )
	PORT_DIPSETTING(    0x40, "20" )
	PORT_DIPSETTING(    0x50, "25" )
	PORT_DIPSETTING(    0x60, "30" )
	PORT_DIPSETTING(    0x70, "40" )
	PORT_DIPSETTING(    0x80, "50" )
	PORT_DIPSETTING(    0x90, "60" )
	PORT_DIPSETTING(    0xa0, "80" )
	PORT_DIPSETTING(    0xb0, "100" )
	PORT_DIPSETTING(    0xc0, "200" )
	PORT_DIPSETTING(    0xd0, "300" )
	PORT_DIPSETTING(    0xe0, "400" )
	PORT_DIPSETTING(    0xf0, "500" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x01, "Check Account" )				PORT_DIPLOCATION("DSW4:1")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )			PORT_DIPLOCATION("DSW4:2")	/* unknown */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Type of Game" )				PORT_DIPLOCATION("DSW4:3")	/* OK */
	PORT_DIPSETTING(    0x00, "Discard" )
	PORT_DIPSETTING(    0x04, "Hold" )
	PORT_DIPNAME( 0x08, 0x08, "Odds Table" )				PORT_DIPLOCATION("DSW4:4")	/* OK */
	PORT_DIPSETTING(    0x08, "Table 1" )
	PORT_DIPSETTING(    0x00, "Table 2" )
	PORT_DIPNAME( 0x10, 0x10, "Auto Ticket Dispense" )		PORT_DIPLOCATION("DSW4:5")	/* not checked */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0xe0, 0xe0, "Ticket Dispense Mode" )		PORT_DIPLOCATION("DSW4:6,7,8")	/* OK */
	PORT_DIPSETTING(    0xe0, "Continuous" )
	PORT_DIPSETTING(    0xc0, "Max 1 Ticket Per Game" )
	PORT_DIPSETTING(    0xa0, "Max 2 Ticket Per Game" )
	PORT_DIPSETTING(    0x80, "Max 3 Ticket Per Game" )
	PORT_DIPSETTING(    0x60, "Max 4 Ticket Per Game" )
	PORT_DIPSETTING(    0x40, "Max 5 Ticket Per Game" )
	PORT_DIPSETTING(    0x20, "Max 8 Ticket Per Game" )
	PORT_DIPSETTING(    0x00, "Max 10 Ticket Per Game" )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unused ) )				PORT_DIPLOCATION("DSW5:1")	/* unknown */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unused ) )				PORT_DIPLOCATION("DSW5:2")	/* unknown */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Use Printer" )					PORT_DIPLOCATION("DSW5:3")	/* OK */
	PORT_DIPSETTING(    0x00, "No (Use TDDD)" )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, "Show Game Name & Odds Table" )	PORT_DIPLOCATION("DSW5:4")	/* not checked */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unused ) )				PORT_DIPLOCATION("DSW5:5")	/* unknown */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unused ) )				PORT_DIPLOCATION("DSW5:6")	/* unknown */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unused ) )				PORT_DIPLOCATION("DSW5:7")	/* unknown */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unused ) )				PORT_DIPLOCATION("DSW5:8")	/* unknown */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( match98 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_Z) PORT_NAME("Hit")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_X) PORT_NAME("Unknown1")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_V) PORT_NAME("Unknown3")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_B) PORT_NAME("Play (Bet)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_C) PORT_NAME("Unknown2")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )   PORT_NAME("Note In")		/* Note In */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )   PORT_IMPULSE(2)			/* Coin A */

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Out / Attendant")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Settings")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x03, "Game Level (Difficulty)" )	PORT_DIPLOCATION("DSW1:1,2,3")	/* OK */
	PORT_DIPSETTING(    0x07, "Level 1" )
	PORT_DIPSETTING(    0x06, "Level 2" )
	PORT_DIPSETTING(    0x05, "Level 3" )
	PORT_DIPSETTING(    0x04, "Level 4" )
	PORT_DIPSETTING(    0x03, "Level 5" )
	PORT_DIPSETTING(    0x02, "Level 6" )
	PORT_DIPSETTING(    0x01, "Level 7" )
	PORT_DIPSETTING(    0x00, "Level 8" )
	PORT_DIPNAME( 0x38, 0x38, "Maximum Play" )				PORT_DIPLOCATION("DSW1:4,5,6")	/* OK */
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x08, "10" )
	PORT_DIPSETTING(    0x10, "30" )
	PORT_DIPSETTING(    0x18, "40" )
	PORT_DIPSETTING(    0x20, "50" )
	PORT_DIPSETTING(    0x28, "60" )
	PORT_DIPSETTING(    0x30, "80" )
	PORT_DIPSETTING(    0x38, "100" )
	PORT_DIPNAME( 0xc0, 0x80, "Minimum Play for Bonus" )	PORT_DIPLOCATION("DSW1:7,8")	/* OK */
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x80, "20" )
	PORT_DIPSETTING(    0xc0, "30" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )			PORT_DIPLOCATION("DSW2:1")	/* unknown */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )			PORT_DIPLOCATION("DSW2:2")	/* unknown */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x1c, 0x10, "Coin In" )					PORT_DIPLOCATION("DSW2:3,4,5")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x10, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x14, "1 Coin/20 Credits" )
	PORT_DIPSETTING(    0x18, "1 Coin/25 Credits" )
	PORT_DIPSETTING(    0x1c, "1 Coin/100 Credits" )
	PORT_DIPNAME( 0x60, 0x00, "Note In" )					PORT_DIPLOCATION("DSW2:6,7")	/* OK */
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPSETTING(    0x20, "200" )
	PORT_DIPSETTING(    0x40, "500" )
	PORT_DIPSETTING(    0x60, "1000" )
	PORT_DIPNAME( 0x80, 0x00, "WARNING: Always Off" )		PORT_DIPLOCATION("DSW2:8")	/* Listed that way in the manual */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x02, "Minimum Play to Start" )			PORT_DIPLOCATION("DSW3:1,2")	/* OK */
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "8" )
	PORT_DIPSETTING(    0x02, "10" )
	PORT_DIPSETTING(    0x03, "20" )
	PORT_DIPNAME( 0x0c, 0x08, "Max Coin In & Note In Point" )	PORT_DIPLOCATION("DSW3:3,4")	/* OK */
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPSETTING(    0x04, "5000" )
	PORT_DIPSETTING(    0x08, "10000" )
	PORT_DIPSETTING(    0x0c, "90000" )
	PORT_DIPNAME( 0xf0, 0x00, "Clear / Ticket Unit" )			PORT_DIPLOCATION("DSW3:5,6,7,8")	/* OK */
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x30, "10" )
	PORT_DIPSETTING(    0x40, "20" )
	PORT_DIPSETTING(    0x50, "25" )
	PORT_DIPSETTING(    0x60, "30" )
	PORT_DIPSETTING(    0x70, "40" )
	PORT_DIPSETTING(    0x80, "50" )
	PORT_DIPSETTING(    0x90, "60" )
	PORT_DIPSETTING(    0xa0, "80" )
	PORT_DIPSETTING(    0xb0, "100" )
	PORT_DIPSETTING(    0xc0, "200" )
	PORT_DIPSETTING(    0xd0, "300" )
	PORT_DIPSETTING(    0xe0, "400" )
	PORT_DIPSETTING(    0xf0, "500" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )			PORT_DIPLOCATION("DSW4:1")	/* unknown */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )			PORT_DIPLOCATION("DSW4:2")	/* unknown */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )			PORT_DIPLOCATION("DSW4:3")	/* unknown */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )			PORT_DIPLOCATION("DSW4:4")	/* unknown */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Auto Ticket Dispense" )		PORT_DIPLOCATION("DSW4:5")	/* not checked */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0xe0, 0xe0, "Ticket Dispense Mode" )		PORT_DIPLOCATION("DSW4:6,7,8")	/* OK */
	PORT_DIPSETTING(    0xe0, "Continuous" )
	PORT_DIPSETTING(    0xc0, "Max 1 Ticket Per Game" )
	PORT_DIPSETTING(    0xa0, "Max 2 Ticket Per Game" )
	PORT_DIPSETTING(    0x80, "Max 3 Ticket Per Game" )
	PORT_DIPSETTING(    0x60, "Max 4 Ticket Per Game" )
	PORT_DIPSETTING(    0x40, "Max 5 Ticket Per Game" )
	PORT_DIPSETTING(    0x20, "Max 8 Ticket Per Game" )
	PORT_DIPSETTING(    0x00, "Max 10 Ticket Per Game" )

	PORT_START("DSW5")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( nfb96bl )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SLOT_STOP_ALL ) PORT_NAME("Stop All / Big")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SLOT_STOP1 ) PORT_NAME("Stop 1 / D-UP")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SLOT_STOP3 ) PORT_NAME("Stop 3 / Take / Select Card")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Play (Bet)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SLOT_STOP2 ) PORT_NAME("Stop 2 / Small / Info")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )   PORT_NAME("Note In")		/* Note In */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )   PORT_IMPULSE(2)			/* Coin A */

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Out / Attendant")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Settings")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Stats")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x03, "Game Rate" )					PORT_DIPLOCATION("DSW1:1,2,3")	/* OK */
	PORT_DIPSETTING(    0x07, "90%" )
	PORT_DIPSETTING(    0x06, "85%" )
	PORT_DIPSETTING(    0x05, "80%" )
	PORT_DIPSETTING(    0x04, "75%" )
	PORT_DIPSETTING(    0x03, "70%" )
	PORT_DIPSETTING(    0x02, "65%" )
	PORT_DIPSETTING(    0x01, "60%" )
	PORT_DIPSETTING(    0x00, "55%" )
	PORT_DIPNAME( 0x38, 0x38, "Maximum Play" )				PORT_DIPLOCATION("DSW1:4,5,6")	/* OK */
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x08, "16" )
	PORT_DIPSETTING(    0x10, "24" )
	PORT_DIPSETTING(    0x18, "32" )
	PORT_DIPSETTING(    0x20, "40" )
	PORT_DIPSETTING(    0x28, "48" )
	PORT_DIPSETTING(    0x30, "64" )
	PORT_DIPSETTING(    0x38, "80" )
	PORT_DIPNAME( 0xc0, 0x80, "Minimum Play for Bonus" )	PORT_DIPLOCATION("DSW1:7,8")	/* OK */
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x40, "16" )
	PORT_DIPSETTING(    0x80, "24" )
	PORT_DIPSETTING(    0xc0, "32" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Double-Up Game" )			PORT_DIPLOCATION("DSW2:1")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, "Skill Spinning" )			PORT_DIPLOCATION("DSW2:2")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x1c, 0x10, "Coin In" )					PORT_DIPLOCATION("DSW2:3,4,5")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x10, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x14, "1 Coin/20 Credits" )
	PORT_DIPSETTING(    0x18, "1 Coin/25 Credits" )
	PORT_DIPSETTING(    0x1c, "1 Coin/100 Credits" )
	PORT_DIPNAME( 0x60, 0x00, "Note In" )					PORT_DIPLOCATION("DSW2:6,7")	/* OK */
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPSETTING(    0x20, "200" )
	PORT_DIPSETTING(    0x40, "500" )
	PORT_DIPSETTING(    0x60, "1000" )
	PORT_DIPNAME( 0x80, 0x00, "WARNING: Always Off" )		PORT_DIPLOCATION("DSW2:8")	/* Listed that way in the manual */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x02, "Minimum Play to Start" )			PORT_DIPLOCATION("DSW3:1,2")	/* OK */
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "8" )
	PORT_DIPSETTING(    0x02, "16" )
	PORT_DIPSETTING(    0x03, "24" )
	PORT_DIPNAME( 0x0c, 0x08, "Max Coin In & Note In Point" )	PORT_DIPLOCATION("DSW3:3,4")	/* OK */
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPSETTING(    0x04, "5000" )
	PORT_DIPSETTING(    0x08, "10000" )
	PORT_DIPSETTING(    0x0c, "90000" )
	PORT_DIPNAME( 0xf0, 0x00, "Clear / Ticket Unit" )			PORT_DIPLOCATION("DSW3:5,6,7,8")	/* OK */
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x30, "10" )
	PORT_DIPSETTING(    0x40, "20" )
	PORT_DIPSETTING(    0x50, "25" )
	PORT_DIPSETTING(    0x60, "30" )
	PORT_DIPSETTING(    0x70, "40" )
	PORT_DIPSETTING(    0x80, "50" )
	PORT_DIPSETTING(    0x90, "60" )
	PORT_DIPSETTING(    0xa0, "80" )
	PORT_DIPSETTING(    0xb0, "100" )
	PORT_DIPSETTING(    0xc0, "200" )
	PORT_DIPSETTING(    0xd0, "300" )
	PORT_DIPSETTING(    0xe0, "400" )
	PORT_DIPSETTING(    0xf0, "500" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x01, "Check Account" )							PORT_DIPLOCATION("DSW4:1")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )						PORT_DIPLOCATION("DSW4:2")	/* unknown */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Initial Bonus Settings After Reset" )	PORT_DIPLOCATION("DSW4:3")	/* not checked */
	PORT_DIPSETTING(    0x00, "Type 1" )
	PORT_DIPSETTING(    0x04, "Type 2" )
	PORT_DIPNAME( 0x08, 0x08, "Bonus Accumulation" )					PORT_DIPLOCATION("DSW4:4")	/* not checked */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, "Auto Ticket Dispense" )					PORT_DIPLOCATION("DSW4:5")	/* not checked */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0xe0, 0xe0, "Ticket Dispense Mode" )					PORT_DIPLOCATION("DSW4:6,7,8")	/* OK */
	PORT_DIPSETTING(    0xe0, "Continuous" )
	PORT_DIPSETTING(    0xc0, "Max 1 Ticket Per Game" )
	PORT_DIPSETTING(    0xa0, "Max 2 Ticket Per Game" )
	PORT_DIPSETTING(    0x80, "Max 3 Ticket Per Game" )
	PORT_DIPSETTING(    0x60, "Max 4 Ticket Per Game" )
	PORT_DIPSETTING(    0x40, "Max 5 Ticket Per Game" )
	PORT_DIPSETTING(    0x20, "Max 8 Ticket Per Game" )
	PORT_DIPSETTING(    0x00, "Max 10 Ticket Per Game" )

	PORT_START("DSW5")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( unkch )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_N) PORT_NAME("Bet A / STOP 2")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )   PORT_IMPULSE(2)
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_C) PORT_NAME("Take / STOP1")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_M) PORT_NAME("Bet B / D-UP")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_V) PORT_NAME("Small / STOP 3")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_B) PORT_NAME("Big")

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "IN1" )
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
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start")
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )


	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, "IN2")
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

	PORT_START("IN3")
	PORT_DIPNAME( 0x01, 0x01, "IN3" )
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

	PORT_START("IN4")
	PORT_DIPNAME( 0x01, 0x01, "IN4" )
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
INPUT_PORTS_END

static INPUT_PORTS_START( mtonic )
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x01, "IN0")
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

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "IN1")
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

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, "IN2")
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

	PORT_START("IN3")
	PORT_DIPNAME( 0x01, 0x01, "IN3")
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

	PORT_START("IN4")
	PORT_DIPNAME( 0x01, 0x01, "IN4")
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

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "DSW1")
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

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "DSW2")
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

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "DSW3")
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

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x01, "DSW4")
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
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	4096,    /* 4096 characters */
	3,      /* 3 bits per pixel */
	{ 2, 4, 6 }, /* the bitplanes are packed in one byte */
	{ 0*8+0, 0*8+1, 1*8+0, 1*8+1, 2*8+0, 2*8+1, 3*8+0, 3*8+1 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8   /* every char takes 32 consecutive bytes */
};


static const gfx_layout charlayout_chry10 =
{
	8,8,    /* 8*8 characters */
	RGN_FRAC(1,1),    /* 4096 characters */
	3,      /* 3 bits per pixel */
	{ 2, 4, 6 }, /* the bitplanes are packed in one byte */
	{ 3*8+0, 3*8+1, 2*8+0, 2*8+1, 1*8+0, 1*8+1, 0*8+0, 0*8+1 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8   /* every char takes 32 consecutive bytes */
};

static const gfx_layout tilelayout =
{
	8,32,    /* 8*32 characters */
	256,    /* 256 tiles */
	4,      /* 4 bits per pixel */
	{ 0, 2, 4, 6 },
	{ 0, 1, 1*8+0, 1*8+1, 2*8+0, 2*8+1, 3*8+0, 3*8+1 },
	{ 0*8, 4*8, 8*8, 12*8, 16*8, 20*8, 24*8, 28*8,
			32*8, 36*8, 40*8, 44*8, 48*8, 52*8, 56*8, 60*8,
			64*8, 68*8, 72*8, 76*8, 80*8, 84*8, 88*8, 92*8,
			96*8, 100*8, 104*8, 108*8, 112*8, 116*8, 120*8, 124*8 },
	128*8   /* every char takes 128 consecutive bytes */
};


static const gfx_layout tilelayoutbl =
{
	8,32,    /* 8*32 characters */
	256,    /* 256 tiles */
	4,      /* 4 bits per pixel */
	{ 0, 2, 4, 6 },
	{ 0, 1, 2*8+0, 2*8+1, 1*8+0, 1*8+1, 3*8+0, 3*8+1 },
	{ 0*8, 4*8, 8*8, 12*8, 16*8, 20*8, 24*8, 28*8,
			32*8, 36*8, 40*8, 44*8, 48*8, 52*8, 56*8, 60*8,
			64*8, 68*8, 72*8, 76*8, 80*8, 84*8, 88*8, 92*8,
			96*8, 100*8, 104*8, 108*8, 112*8, 116*8, 120*8, 124*8 },
	128*8   /* every char takes 128 consecutive bytes */
};

static const gfx_layout tilelayout_chry10 =
{
	8,32,    /* 8*32 characters */
	256,    /* 256 tiles */
	4,      /* 4 bits per pixel */
	{ 0, 2, 4, 6 },
	{ 3*8+0, 3*8+1, 2*8+0, 2*8+1, 1*8+0, 1*8+1, 0*8+0, 0*8+1 },
	{ 0*8, 4*8, 8*8, 12*8, 16*8, 20*8, 24*8, 28*8,
			32*8, 36*8, 40*8, 44*8, 48*8, 52*8, 56*8, 60*8,
			64*8, 68*8, 72*8, 76*8, 80*8, 84*8, 88*8, 92*8,
			96*8, 100*8, 104*8, 108*8, 112*8, 116*8, 120*8, 124*8 },
	128*8   /* every char takes 128 consecutive bytes */
};



static const gfx_layout tiles8x8x3_layout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout tiles8x32x4_layout =
{
	8,32,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(0,4), RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8,  2*8,  3*8,  4*8,  5*8,  6*8,  7*8,
	  8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8,
	 16*8,17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8,
	 24*8,25*8, 26*8, 27*8, 28*8, 29*8, 30*8, 31*8
	},
	32*8
};

// cmasterc set
static const gfx_layout tiles8x32x4alt_layout =
{
	8,32,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4,0,4 },
	{ 3,2,1,0,11, 10, 9, 8 },
	{ 0*16, 1*16,  2*16,  3*16,  4*16,  5*16,  6*16,  7*16,
	  8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16,
	 16*16,17*16, 18*16, 19*16, 20*16, 21*16, 22*16, 23*16,
	 24*16,25*16, 26*16, 27*16, 28*16, 29*16, 30*16, 31*16
	},
	16*32
};

static const UINT32 layout_xoffset[128] =
{
	STEP32(0*128,4),STEP32(1*128,4),STEP32(2*128,4),STEP32(3*128,4)
};

static const UINT32 layout_yoffset[128] =
{
	STEP32(0*16384, 512),STEP32(1*16384,512),STEP32(2*16384,512),STEP32(3*16384,512)
};

static const gfx_layout tiles128x128x4_layout =
{
	128,128,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	EXTENDED_XOFFS,
	EXTENDED_YOFFS,
	4 * 16384, /* object takes 8 consecutive bytes */
	layout_xoffset,
	layout_yoffset
};


static const UINT32 layout_xoffset256[256] =
{
	STEP32(0*128,4),STEP32(1*128,4),STEP32(2*128,4),STEP32(3*128,4), STEP32(4*128,4), STEP32(5*128,4), STEP32(6*128,4), STEP32(7*128,4)
};

static const UINT32 layout_yoffset256[256] =
{
	STEP32(0*32768, 1024),STEP32(1*32768,1024),STEP32(2*32768,1024),STEP32(3*32768,1024),STEP32(4*32768,1024), STEP32(5*32768,1024),STEP32(6*32768,1024),STEP32(7*32768,1024)
};


static const gfx_layout tiles256x128x4_layout =
{
	256,256,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	EXTENDED_XOFFS,
	EXTENDED_YOFFS,
	8 * 32768, /* object takes 8 consecutive bytes */
	layout_xoffset256,
	layout_yoffset256
};

#if 0 // decodes an extra plane for cmv4 / cmasterb, not sure if we need to
static const gfx_layout tiles8x32x5_layout =
{
	8,32,
	RGN_FRAC(1,5),
	5,
	{ RGN_FRAC(0,5), RGN_FRAC(1,5), RGN_FRAC(2,5), RGN_FRAC(3,5), RGN_FRAC(4,5) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8,  2*8,  3*8,  4*8,  5*8,  6*8,  7*8,
	  8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8,
	 16*8,17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8,
	 24*8,25*8, 26*8, 27*8, 28*8, 29*8, 30*8, 31*8
	},
	32*8
};
#endif

static const gfx_layout cb3c_tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 4,0,12,8,20,16,28,24 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*32
};


static const gfx_layout cb3c_tiles8x32_layout =
{
	8,32,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 4,0,12,8,20,16,28,24 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32, 8*32,9*32,10*32,11*32,12*32,13*32,14*32,15*32,16*32,17*32,18*32,19*32,20*32,21*32,22*32,23*32,24*32,25*32,26*32,27*32,28*32,29*32,30*32,31*32 },
	32*32
};


static GFXDECODE_START( goldstar )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout, 128,  8 )
GFXDECODE_END

static GFXDECODE_START( bl )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayoutbl, 128,  8 )
GFXDECODE_END

static GFXDECODE_START( ml )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x18000, tilelayout, 128,  8 )
GFXDECODE_END

static GFXDECODE_START( chry10 )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout_chry10,   0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout_chry10, 128,  8 )
GFXDECODE_END

static GFXDECODE_START( cb3c )
	GFXDECODE_ENTRY( "gfx1", 0, cb3c_tiles8x8_layout,   0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, cb3c_tiles8x32_layout, 128,  8 )
GFXDECODE_END

static GFXDECODE_START( ncb3 )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8x3_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x32x4_layout, 128, 4 )
GFXDECODE_END

static GFXDECODE_START( magical )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8x3_layout, 0, 32 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x32x4_layout, 0, 16 )
GFXDECODE_END

static GFXDECODE_START( cm )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8x3_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x32x4_layout, 128+64, 4 ) // or is there a register for the +64?
GFXDECODE_END

static GFXDECODE_START( cmbitmap )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8x3_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x32x4_layout, 128+64, 4 ) // or is there a register for the +64?
	GFXDECODE_ENTRY( "user1", 0, tiles128x128x4_layout, 128, 4 )
GFXDECODE_END

static GFXDECODE_START( cmasterc )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8x3_layout, 0, 16 )
	GFXDECODE_ENTRY( "reels", 0, tiles8x32x4alt_layout, 128+64, 4 )
	GFXDECODE_ENTRY( "user1", 0, tiles128x128x4_layout, 128, 4 )
GFXDECODE_END


static GFXDECODE_START( cmast91 )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8x3_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x32x4_layout, 128+64, 4 ) // or is there a register for the +64?
	GFXDECODE_ENTRY( "user1", 0, tiles256x128x4_layout, 128, 4 ) // wrong... FIXME.
GFXDECODE_END

#if 0 // decodes an extra plane for cmv4 / cmasterb, not sure if we need to
static GFXDECODE_START( cmasterb )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8x3_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x32x5_layout, 0, 4 )
GFXDECODE_END
#endif


static const gfx_layout tiles8x32_4bpp_layout =
{
	8,32,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(0,4),RGN_FRAC(1,4),RGN_FRAC(2,4),RGN_FRAC(3,4) },
	{ 0,1,2,3,4,5,6,7},
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8,16*8,17*8,18*8,19*8,20*8,21*8,22*8,23*8,24*8,25*8,26*8,27*8,28*8,29*8,30*8,31*8 },
	32*8
};

static const gfx_layout tiles8x8_3bpp_layout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3),RGN_FRAC(1,3),RGN_FRAC(2,3) },
	{ 0,1,2,3,4,5,6,7},
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( nfm )
	GFXDECODE_ENTRY( "tilegfx", 0, tiles8x8_3bpp_layout, 0, 16 )
	GFXDECODE_ENTRY( "reelgfx", 0, tiles8x32_4bpp_layout, 0, 16 )
GFXDECODE_END

static const gfx_layout tiles8x8x3_miss1bpp_layout =
{
	8,8,
	RGN_FRAC(1,1),
	3,
	{ 1,2,3 },
	{  8,12,0,4,24,28,16,20 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*32
};

static const gfx_layout tiles8x8x4alt_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1,2,3 },
	{  4,0,12,8,20,16,28,24 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*32
};

static const gfx_layout tiles8x32x4alt2_layout =
{
	8,32,
	RGN_FRAC(1,1),
	4,
	{ 0, 1,2,3 },
	{  4,0,12,8,20,16,28,24 },
	{ STEP32(0,32) },
	32*32
};


static GFXDECODE_START( unkch )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8x4alt_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x32x4alt2_layout, 0, 16 )
GFXDECODE_END

static const gfx_layout tilescherrys_layout =
{
	8,32,
	RGN_FRAC(1,1),
	4,
	{ 3,2,1,0 },
	{  8,12,0,4,24,28,16,20 },
	{ STEP32(0,32) },
	32*32
};

static GFXDECODE_START(cherrys )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8x3_miss1bpp_layout,   0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tilescherrys_layout, 128,  8 )
GFXDECODE_END


static const gfx_layout tiles8x8x4pkr_layout =
{
	8,8,    /* 8*8 characters */
	RGN_FRAC(1,1),    /* 4096 characters */
	4,      /* 3 bits per pixel */
	{ 0, 2, 4, 6 }, /* the bitplanes are packed in one byte */
	{ 0*8+0, 0*8+1, 1*8+0, 1*8+1, 2*8+0, 2*8+1, 3*8+0, 3*8+1 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8   /* every char takes 32 consecutive bytes */
};

static const gfx_layout tiles8x32x4pkr_layout =
{
	8,32,    /* 8*8 characters */
	RGN_FRAC(1,1),    /* 4096 characters */
	4,      /* 3 bits per pixel */
	{ 0, 2, 4, 6 }, /* the bitplanes are packed in one byte */
	{ 0*8+0, 0*8+1, 1*8+0, 1*8+1, 2*8+0, 2*8+1, 3*8+0, 3*8+1 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32, 8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32,
	 16*32,17*32,18*32,19*32,20*32,21*32,22*32,23*32,24*32,25*32, 26*32, 27*32, 28*32, 29*32, 30*32, 31*32},
	32*8*4   /* every char takes 32 consecutive bytes */
};

static GFXDECODE_START( pkrmast )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8x4pkr_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x32x4pkr_layout, 0, 16 )
GFXDECODE_END



/*************************************
*      PPI 8255 (x3) Interfaces      *
*************************************/

static const ppi8255_interface ncb3_ppi8255_intf[3] =
{
	{	/* A, B & C set as input */
		DEVCB_INPUT_PORT("IN0"),	/* Port A read */
		DEVCB_INPUT_PORT("IN3"),	/* Port B read */ //Player2 controls, confirmed.
		DEVCB_NULL,					/* Port C read */
		DEVCB_NULL,					/* Port A write */
		DEVCB_NULL,					/* Port B write */
		DEVCB_NULL					/* Port C write */
	},
	{	/* A, B & C set as input */
		DEVCB_INPUT_PORT("IN1"),	/* Port A read */
		DEVCB_INPUT_PORT("IN2"),	/* Port B read */
		DEVCB_INPUT_PORT("DSW1"),	/* Port C read */
		DEVCB_NULL,					/* Port A write */
		DEVCB_NULL,					/* Port B write */
		DEVCB_NULL					/* Port C write */
	},
	{	/* A set as input */
		DEVCB_INPUT_PORT("DSW2"),	/* Port A read */
		DEVCB_NULL,					/* Port B read */
		DEVCB_NULL,					/* Port C read */
		DEVCB_NULL,					/* Port A write */
		DEVCB_NULL,					/* Port B write */
		DEVCB_NULL					/* Port C write */
	}
};

static const ppi8255_interface cm_ppi8255_intf[2] =
{
	{	/* A, B & C set as input */
		DEVCB_INPUT_PORT("IN0"),	/* Port A read */
		DEVCB_INPUT_PORT("IN1"),	/* Port B read */
		DEVCB_INPUT_PORT("IN2"),	/* Port C read */
		DEVCB_NULL,					/* Port A write */
		DEVCB_NULL,					/* Port B write */
		DEVCB_NULL					/* Port C write */
	},
	{	/* A, B & C set as input */
		DEVCB_INPUT_PORT("DSW1"),	/* Port A read */
		DEVCB_INPUT_PORT("DSW2"),	/* Port B read */
		DEVCB_INPUT_PORT("DSW3"),	/* Port C read */
		DEVCB_NULL,					/* Port A write */
		DEVCB_NULL,					/* Port B write */
		DEVCB_NULL					/* Port C write */
	}
};




static WRITE8_DEVICE_HANDLER( system_outputa_w )
{
	//goldstar_state *state = (goldstar_state *)device->machine->driver_data;
	//popmessage("system_outputa_w %02x",data);
}


static WRITE8_DEVICE_HANDLER( system_outputb_w )
{
	//goldstar_state *state = (goldstar_state *)device->machine->driver_data;
	//popmessage("system_outputb_w %02x",data);
}


static WRITE8_DEVICE_HANDLER( system_outputc_w )
{
	goldstar_state *state = (goldstar_state *)device->machine->driver_data;

	state->lucky8_nmi_enable = data & 8;
	state->unkch_vidreg = data & 2;
	//popmessage("system_outputc_w %02x",data);
}


static const ppi8255_interface lucky8_ppi8255_intf[3] =
{
	{	/* A, B & C set as input */
		DEVCB_INPUT_PORT("IN0"),	/* Port A read */
		DEVCB_INPUT_PORT("IN1"),	/* Port B read */
		DEVCB_INPUT_PORT("IN2"),	/* Port C read */
		DEVCB_NULL,					/* Port A write */
		DEVCB_NULL,					/* Port B write */
		DEVCB_NULL					/* Port C write */
	},
	{	/* A, B & C set as input */
		DEVCB_INPUT_PORT("IN3"),	/* Port A read */
		DEVCB_INPUT_PORT("IN4"),	/* Port B read */
		DEVCB_INPUT_PORT("DSW1"),	/* Port C read */
		DEVCB_NULL,					/* Port A write */
		DEVCB_NULL,					/* Port B write */
		DEVCB_NULL					/* Port C write */
	},
	{	/* A set as input */
		DEVCB_INPUT_PORT("DSW2"),	/* Port A read */
		DEVCB_NULL,					/* Port B read */
		DEVCB_NULL,					/* Port C read */
		DEVCB_HANDLER(system_outputa_w),					/* Port A write */
		DEVCB_HANDLER(system_outputb_w),					/* Port B write */
		DEVCB_HANDLER(system_outputc_w)			/* Port C write */
	}
};

static const ppi8255_interface kkojnoli_ppi8255_intf[3] =
{
	{	/* A, B & C set as input */
		DEVCB_INPUT_PORT("IN0"),	/* Port A read */
		DEVCB_INPUT_PORT("IN1"),	/* Port B read */
		DEVCB_INPUT_PORT("IN2"),	/* Port C read */
		DEVCB_NULL,					/* Port A write */
		DEVCB_NULL,					/* Port B write */
		DEVCB_NULL					/* Port C write */
	},
	{	/* A, B & C set as input */
		DEVCB_INPUT_PORT("IN3"),	/* Port A read */
		DEVCB_INPUT_PORT("IN4"),	/* Port B read */
		DEVCB_NULL,					/* Port C read */
		DEVCB_NULL,					/* Port A write */
		DEVCB_NULL,					/* Port B write */
		DEVCB_NULL					/* Port C write */
	},
	{	/* A set as input */
		DEVCB_INPUT_PORT("DSW1"),	/* Port A read */
		DEVCB_NULL,					/* Port B read */
		DEVCB_NULL,					/* Port C read */
		DEVCB_NULL,					/* Port A write */
		DEVCB_NULL,					/* Port B write */
		DEVCB_NULL					/* Port C write */
	}
};

static const ppi8255_interface ladylinr_ppi8255_intf[2] =
{
	{	/* A, B & C set as input */
		DEVCB_INPUT_PORT("IN0"),	/* Port A read */
		DEVCB_INPUT_PORT("IN1"),	/* Port B read */
		DEVCB_INPUT_PORT("IN2"),	/* Port C read */
		DEVCB_NULL,					/* Port A write */
		DEVCB_NULL,					/* Port B write */
		DEVCB_NULL					/* Port C write */
	},
	{	/* A set as input */
		DEVCB_INPUT_PORT("DSW1"),	/* Port A read */
		DEVCB_NULL,					/* Port B read */
		DEVCB_NULL,					/* Port C read */
		DEVCB_NULL,					/* Port A write */
		DEVCB_NULL,					/* Port B write */
		DEVCB_NULL					/* Port C write */
	}
};

static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_INPUT_PORT("DSW4"),
	DEVCB_INPUT_PORT("DSW3"),
	DEVCB_NULL,
	DEVCB_NULL
};

static const ay8910_interface cm_ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_INPUT_PORT("DSW4"),
	DEVCB_INPUT_PORT("DSW5"),
	DEVCB_NULL,
	DEVCB_NULL
};

static WRITE8_DEVICE_HANDLER( ay8910_outputa_w )
{
	//goldstar_state *state = (goldstar_state *)device->machine->driver_data;
	//popmessage("ay8910_outputa_w %02x",data);
}

static WRITE8_DEVICE_HANDLER( ay8910_outputb_w )
{
	//goldstar_state *state = (goldstar_state *)device->machine->driver_data;
	//popmessage("ay8910_outputb_w %02x",data);
}

static const ay8910_interface lucky8_ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_INPUT_PORT("DSW3"),
	DEVCB_INPUT_PORT("DSW4"),
	DEVCB_HANDLER(ay8910_outputa_w),
	DEVCB_HANDLER(ay8910_outputb_w)
};

static const ay8910_interface ladylinr_ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

static MACHINE_DRIVER_START( goldstar )

	MDRV_DRIVER_DATA(goldstar_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, CPU_CLOCK)
	MDRV_CPU_PROGRAM_MAP(goldstar_map)
	MDRV_CPU_IO_MAP(goldstar_readport)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(goldstar)
	MDRV_PALETTE_LENGTH(256)
	MDRV_NVRAM_HANDLER(goldstar)

	MDRV_VIDEO_START(goldstar)
	MDRV_VIDEO_UPDATE(goldstar)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("aysnd", AY8910, AY_CLOCK)
	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_OKIM6295_ADD("oki", OKI_CLOCK, OKIM6295_PIN7_HIGH)	/* clock frequency & pin 7 not verified */
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

MACHINE_DRIVER_END


static MACHINE_DRIVER_START( goldstbl )

	MDRV_DRIVER_DATA(goldstar_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, CPU_CLOCK)
	MDRV_CPU_PROGRAM_MAP(goldstar_map)
	MDRV_CPU_IO_MAP(goldstar_readport)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
//  MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(bl)
	MDRV_PALETTE_LENGTH(256)
	MDRV_NVRAM_HANDLER(goldstar)

	MDRV_VIDEO_START(goldstar)
	MDRV_VIDEO_UPDATE(goldstar)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("aysnd", AY8910, AY_CLOCK)
	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_OKIM6295_ADD("oki", OKI_CLOCK, OKIM6295_PIN7_HIGH)	/* clock frequency & pin 7 not verified */
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( moonlght )

	MDRV_DRIVER_DATA(goldstar_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, CPU_CLOCK)
	MDRV_CPU_PROGRAM_MAP(goldstar_map)
	MDRV_CPU_IO_MAP(goldstar_readport)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
//  MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(ml)
	MDRV_PALETTE_LENGTH(256)
	MDRV_NVRAM_HANDLER(goldstar)

	MDRV_VIDEO_START(goldstar)
	MDRV_VIDEO_UPDATE(goldstar)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("aysnd", AY8910, AY_CLOCK)
	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_OKIM6295_ADD("oki", OKI_CLOCK, OKIM6295_PIN7_HIGH)	/* clock frequency & pin 7 not verified */
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


static PALETTE_INIT(cm)
{
	/* BBGGGRRR */

	int i;

	for (i = 0; i < 0x100; i++)
	{
		UINT8 data;
		UINT8*proms = memory_region(machine, "proms");

		data = proms[0x000 + i] | (proms[0x100 + i] << 4);

		palette_set_color_rgb(machine, i, pal3bit(data >> 0), pal3bit(data >> 3), pal2bit(data >> 6));
	}
}

static PALETTE_INIT(cmast91)
{
	int i;
	for (i = 0; i < 0x100; i++)
	{
		int r,g,b;

		UINT8*proms = memory_region(machine, "proms");

		b = proms[0x000 + i] << 4;
		g = proms[0x100 + i] << 4;
		r = proms[0x200 + i] << 4;

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
	}
}

static PALETTE_INIT(lucky8)
{
	/* BBGGGRRR */

	int i;
	UINT8 data;
	UINT8 *proms;

	proms = memory_region(machine, "proms");
	for (i = 0; i < 0x100; i++)
	{

		data = proms[0x000 + i] | (proms[0x100 + i] << 4);

		palette_set_color_rgb(machine, i, pal3bit(data >> 0), pal3bit(data >> 3), pal2bit(data >> 6));
	}

	proms = memory_region(machine, "proms2");
	for (i=0; i < 0x20; i++)
	{
		data = proms[i];

		palette_set_color_rgb(machine, i + 0x80, pal3bit(data >> 0), pal3bit(data >> 3), pal2bit(data >> 6));
	}
}


static MACHINE_DRIVER_START( chrygld )

	MDRV_DRIVER_DATA(goldstar_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, CPU_CLOCK)
	MDRV_CPU_PROGRAM_MAP(ncb3_map)
	MDRV_CPU_IO_MAP(ncb3_readwriteport)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	/* 3x 8255 */
	MDRV_PPI8255_ADD( "ppi8255_0", ncb3_ppi8255_intf[0] )
	MDRV_PPI8255_ADD( "ppi8255_1", ncb3_ppi8255_intf[1] )
	MDRV_PPI8255_ADD( "ppi8255_2", ncb3_ppi8255_intf[2] )

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
//  MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(chry10)
	MDRV_PALETTE_LENGTH(256)
	MDRV_PALETTE_INIT(cm)
	MDRV_NVRAM_HANDLER(goldstar)

	MDRV_VIDEO_START(goldstar)
	MDRV_VIDEO_UPDATE(goldstar)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("snsnd", SN76489, PSG_CLOCK)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MDRV_SOUND_ADD("aysnd", AY8910, AY_CLOCK)
	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END



static MACHINE_DRIVER_START( cb3c )

	MDRV_DRIVER_DATA(goldstar_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, CPU_CLOCK)
	MDRV_CPU_PROGRAM_MAP(ncb3_map)
	MDRV_CPU_IO_MAP(ncb3_readwriteport)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	/* 3x 8255 */
	MDRV_PPI8255_ADD( "ppi8255_0", ncb3_ppi8255_intf[0] )
	MDRV_PPI8255_ADD( "ppi8255_1", ncb3_ppi8255_intf[1] )
	MDRV_PPI8255_ADD( "ppi8255_2", ncb3_ppi8255_intf[2] )

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
//  MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(cb3c)
	MDRV_PALETTE_LENGTH(256)
	MDRV_PALETTE_INIT(cm)
	MDRV_NVRAM_HANDLER(goldstar)

	MDRV_VIDEO_START(goldstar)
	MDRV_VIDEO_UPDATE(goldstar)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("snsnd", SN76489, PSG_CLOCK)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MDRV_SOUND_ADD("aysnd", AY8910, AY_CLOCK)
	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( ncb3 )

	MDRV_DRIVER_DATA(goldstar_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, CPU_CLOCK)
	MDRV_CPU_PROGRAM_MAP(ncb3_map)
	MDRV_CPU_IO_MAP(ncb3_readwriteport)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	/* 3x 8255 */
	MDRV_PPI8255_ADD( "ppi8255_0", ncb3_ppi8255_intf[0] )
	MDRV_PPI8255_ADD( "ppi8255_1", ncb3_ppi8255_intf[1] )
	MDRV_PPI8255_ADD( "ppi8255_2", ncb3_ppi8255_intf[2] )

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
//  MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(ncb3)
	MDRV_PALETTE_LENGTH(256)
	MDRV_PALETTE_INIT(cm)

	MDRV_NVRAM_HANDLER(goldstar)

	MDRV_VIDEO_START(goldstar)
	MDRV_VIDEO_UPDATE(goldstar)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("snsnd", SN76489, PSG_CLOCK)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MDRV_SOUND_ADD("aysnd", AY8910, AY_CLOCK)
	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( cm )

	MDRV_DRIVER_DATA(goldstar_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, CPU_CLOCK)
	MDRV_CPU_PROGRAM_MAP(cm_map)
	MDRV_CPU_IO_MAP(cm_portmap)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	/* 2x 8255 */
	MDRV_PPI8255_ADD( "ppi8255_0", cm_ppi8255_intf[0] )
	MDRV_PPI8255_ADD( "ppi8255_1", cm_ppi8255_intf[1] )

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
//  MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(cmbitmap)
	MDRV_PALETTE_LENGTH(256)
	MDRV_PALETTE_INIT(cm)
	MDRV_NVRAM_HANDLER(goldstar)

	MDRV_VIDEO_START(cherrym)
	MDRV_VIDEO_UPDATE(goldstar)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("aysnd", AY8910, AY_CLOCK)
	MDRV_SOUND_CONFIG(cm_ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( cmasterc )

	MDRV_IMPORT_FROM(cm)
	MDRV_GFXDECODE(cmasterc)

MACHINE_DRIVER_END

static MACHINE_DRIVER_START( cmnobmp )

	MDRV_DRIVER_DATA(goldstar_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, CPU_CLOCK)
	MDRV_CPU_PROGRAM_MAP(cm_map)
	MDRV_CPU_IO_MAP(cm_portmap)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	/* 2x 8255 */
	MDRV_PPI8255_ADD( "ppi8255_0", cm_ppi8255_intf[0] )
	MDRV_PPI8255_ADD( "ppi8255_1", cm_ppi8255_intf[1] )

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
//  MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(cm)
	MDRV_PALETTE_LENGTH(256)
	MDRV_PALETTE_INIT(cm)
	MDRV_NVRAM_HANDLER(goldstar)

	MDRV_VIDEO_START(cherrym)
	MDRV_VIDEO_UPDATE(goldstar)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("aysnd", AY8910, AY_CLOCK)
	MDRV_SOUND_CONFIG(cm_ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( cmast91 )

	MDRV_DRIVER_DATA(goldstar_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, CPU_CLOCK)
	MDRV_CPU_PROGRAM_MAP(cm_map)
	MDRV_CPU_IO_MAP(cmast91_portmap)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	/* 2x 8255 */
	MDRV_PPI8255_ADD( "ppi8255_0", cm_ppi8255_intf[0] )
	MDRV_PPI8255_ADD( "ppi8255_1", cm_ppi8255_intf[1] )

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
//  MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(cmast91)
	MDRV_PALETTE_LENGTH(256)
	MDRV_PALETTE_INIT(cmast91)
	MDRV_NVRAM_HANDLER(goldstar)

	MDRV_VIDEO_START(cherrym)
	MDRV_VIDEO_UPDATE(cmast91)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("aysnd", AY8910, AY_CLOCK)
	MDRV_SOUND_CONFIG(cm_ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END

static INTERRUPT_GEN( lucky8_irq )
{
	goldstar_state *state = (goldstar_state *)device->machine->driver_data;

	if(state->lucky8_nmi_enable)
		cpu_set_input_line(device, INPUT_LINE_NMI, PULSE_LINE);
}

static MACHINE_DRIVER_START( lucky8 )

	MDRV_DRIVER_DATA(goldstar_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, CPU_CLOCK)
	MDRV_CPU_PROGRAM_MAP(lucky8_map)
	//MDRV_CPU_IO_MAP(goldstar_readport)
	MDRV_CPU_VBLANK_INT("screen", lucky8_irq)

	/* 3x 8255 */
	MDRV_PPI8255_ADD( "ppi8255_0", lucky8_ppi8255_intf[0] )
	MDRV_PPI8255_ADD( "ppi8255_1", lucky8_ppi8255_intf[1] )
	MDRV_PPI8255_ADD( "ppi8255_2", lucky8_ppi8255_intf[2] )

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
//  MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)
	MDRV_PALETTE_INIT(lucky8)

	MDRV_GFXDECODE(ncb3)
	MDRV_PALETTE_LENGTH(256)
	MDRV_NVRAM_HANDLER(goldstar)

	MDRV_VIDEO_START(goldstar)
	MDRV_VIDEO_UPDATE(goldstar)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("snsnd", SN76489, PSG_CLOCK)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MDRV_SOUND_ADD("aysnd", AY8910, AY_CLOCK)
	MDRV_SOUND_CONFIG(lucky8_ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

MACHINE_DRIVER_END

static PALETTE_INIT(magical)
{
	int i;
	for (i = 0; i < 0x100; i++)
	{
		int r,g,b;

		UINT8*proms = memory_region(machine, "proms");

		b = proms[0x000 + i] << 4;
		g = proms[0x100 + i] << 4;
		r = proms[0x200 + i] << 4;

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
	}
}

static MACHINE_DRIVER_START( magical )

	MDRV_DRIVER_DATA(goldstar_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, CPU_CLOCK)
	MDRV_CPU_PROGRAM_MAP(magical_map)
	//MDRV_CPU_IO_MAP(goldstar_readport)
	MDRV_CPU_VBLANK_INT("screen", lucky8_irq)

	/* 3x 8255 */
	MDRV_PPI8255_ADD( "ppi8255_0", lucky8_ppi8255_intf[0] )
	MDRV_PPI8255_ADD( "ppi8255_1", lucky8_ppi8255_intf[1] )
	MDRV_PPI8255_ADD( "ppi8255_2", lucky8_ppi8255_intf[2] )

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
//  MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)
	MDRV_PALETTE_INIT(magical)

	MDRV_GFXDECODE(magical)
	MDRV_PALETTE_LENGTH(256)
	MDRV_NVRAM_HANDLER(goldstar)

	MDRV_VIDEO_START(magical)
	MDRV_VIDEO_UPDATE(magical)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("snsnd", SN76489, PSG_CLOCK)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.00)  // shut up annoying whine

	MDRV_SOUND_ADD("aysnd", AY8910, AY_CLOCK)
	MDRV_SOUND_CONFIG(lucky8_ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( kkojnoli )

	MDRV_DRIVER_DATA(goldstar_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, CPU_CLOCK)
	MDRV_CPU_PROGRAM_MAP(kkojnoli_map)
	//MDRV_CPU_IO_MAP(goldstar_readport)
	MDRV_CPU_VBLANK_INT("screen", nmi_line_pulse)


	/* 3x 8255 */
	MDRV_PPI8255_ADD( "ppi8255_0", kkojnoli_ppi8255_intf[0] )
	MDRV_PPI8255_ADD( "ppi8255_1", kkojnoli_ppi8255_intf[1] )
	MDRV_PPI8255_ADD( "ppi8255_2", kkojnoli_ppi8255_intf[2] )

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
//  MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)
	MDRV_PALETTE_INIT(lucky8)

	MDRV_GFXDECODE(ncb3)
	MDRV_PALETTE_LENGTH(256)

	MDRV_VIDEO_START(goldstar)
	MDRV_VIDEO_UPDATE(goldstar)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("snsnd", SN76489, PSG_CLOCK)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

MACHINE_DRIVER_END


static MACHINE_DRIVER_START( ladylinr )

	MDRV_DRIVER_DATA(goldstar_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, CPU_CLOCK)
	MDRV_CPU_PROGRAM_MAP(ladylinr_map)
	//MDRV_CPU_IO_MAP(goldstar_readport)
	MDRV_CPU_VBLANK_INT("screen", nmi_line_pulse)

	/* 2x 8255 */
	MDRV_PPI8255_ADD( "ppi8255_0", ladylinr_ppi8255_intf[0] )
	MDRV_PPI8255_ADD( "ppi8255_1", ladylinr_ppi8255_intf[1] )

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
//  MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)
	MDRV_PALETTE_INIT(lucky8)

	MDRV_GFXDECODE(ncb3)
	MDRV_PALETTE_LENGTH(256)
	MDRV_NVRAM_HANDLER(goldstar)

	MDRV_VIDEO_START(goldstar)
	MDRV_VIDEO_UPDATE(goldstar)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")//set up a standard mono speaker called 'mono'

	MDRV_SOUND_ADD("snsnd", SN76489, PSG_CLOCK)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MDRV_SOUND_ADD("aysnd", AY8910, AY_CLOCK)
	MDRV_SOUND_CONFIG(ladylinr_ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( wcat3 )

	MDRV_DRIVER_DATA(goldstar_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, CPU_CLOCK)
	MDRV_CPU_PROGRAM_MAP(wcat3_map)
	//MDRV_CPU_IO_MAP(goldstar_readport)
	MDRV_CPU_VBLANK_INT("screen", nmi_line_pulse)

	/* 3x 8255 */
	MDRV_PPI8255_ADD( "ppi8255_0", lucky8_ppi8255_intf[0] )
	MDRV_PPI8255_ADD( "ppi8255_1", lucky8_ppi8255_intf[1] )
	MDRV_PPI8255_ADD( "ppi8255_2", lucky8_ppi8255_intf[2] )

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
//  MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)
	MDRV_PALETTE_INIT(lucky8)

	MDRV_GFXDECODE(ncb3)
	MDRV_PALETTE_LENGTH(256)
	MDRV_NVRAM_HANDLER(goldstar)

	MDRV_VIDEO_START(goldstar)
	MDRV_VIDEO_UPDATE(goldstar)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("snsnd", SN76489, PSG_CLOCK)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MDRV_SOUND_ADD("aysnd", AY8910, AY_CLOCK)
	MDRV_SOUND_CONFIG(lucky8_ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

MACHINE_DRIVER_END


/* diff with cm machine driver: gfxdecode, OKI & portmap */
static MACHINE_DRIVER_START( amcoe1 )

	MDRV_DRIVER_DATA(goldstar_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, CPU_CLOCK)
	MDRV_CPU_PROGRAM_MAP(cm_map)
	MDRV_CPU_IO_MAP(amcoe1_portmap)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	/* 2x 8255 */
	MDRV_PPI8255_ADD( "ppi8255_0", cm_ppi8255_intf[0] )
	MDRV_PPI8255_ADD( "ppi8255_1", cm_ppi8255_intf[1] )

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
//  MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(cm)
	MDRV_PALETTE_LENGTH(256)
	MDRV_PALETTE_INIT(cm)
	MDRV_NVRAM_HANDLER(goldstar)

	MDRV_VIDEO_START(cherrym)
	MDRV_VIDEO_UPDATE(goldstar)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("aysnd", AY8910, AY_CLOCK)
	MDRV_SOUND_CONFIG(cm_ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_OKIM6295_ADD("oki", OKI_CLOCK, OKIM6295_PIN7_HIGH)	/* clock frequency & pin 7 not verified */
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


/* diff with cm machine driver: gfxdecode, OKI, portmap & tilemaps rect size/position */
static MACHINE_DRIVER_START( amcoe1a )

	MDRV_DRIVER_DATA(goldstar_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, CPU_CLOCK)
	MDRV_CPU_PROGRAM_MAP(cm_map)
	MDRV_CPU_IO_MAP(amcoe1_portmap)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	/* 2x 8255 */
	MDRV_PPI8255_ADD( "ppi8255_0", cm_ppi8255_intf[0] )
	MDRV_PPI8255_ADD( "ppi8255_1", cm_ppi8255_intf[1] )

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
//  MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(cm)
	MDRV_PALETTE_LENGTH(256)
	MDRV_PALETTE_INIT(cm)
	MDRV_NVRAM_HANDLER(goldstar)

	MDRV_VIDEO_START(cherrym)
	MDRV_VIDEO_UPDATE(amcoe1a)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("aysnd", AY8910, AY_CLOCK)
	MDRV_SOUND_CONFIG(cm_ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_OKIM6295_ADD("oki", OKI_CLOCK, OKIM6295_PIN7_HIGH)	/* clock frequency & pin 7 not verified */
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


/* diff with cm machine driver: gfxdecode, AY8910 volume & portmap */
static MACHINE_DRIVER_START( amcoe2 )

	MDRV_DRIVER_DATA(goldstar_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, CPU_CLOCK)
	MDRV_CPU_PROGRAM_MAP(cm_map)
	MDRV_CPU_IO_MAP(amcoe2_portmap)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	/* 2x 8255 */
	MDRV_PPI8255_ADD( "ppi8255_0", cm_ppi8255_intf[0] )
	MDRV_PPI8255_ADD( "ppi8255_1", cm_ppi8255_intf[1] )

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
//  MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(cm)
	MDRV_PALETTE_LENGTH(256)
	MDRV_PALETTE_INIT(cm)
	MDRV_NVRAM_HANDLER(goldstar)

	MDRV_VIDEO_START(cherrym)
	MDRV_VIDEO_UPDATE(goldstar)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("aysnd", AY8910, AY_CLOCK)
	MDRV_SOUND_CONFIG(cm_ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 2.00)	/* analyzed for clips */
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( nfm )

	MDRV_DRIVER_DATA(goldstar_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, CPU_CLOCK)
	MDRV_CPU_PROGRAM_MAP(nfm_map)
	MDRV_CPU_IO_MAP(amcoe2_portmap)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	/* 2x 8255 */
	MDRV_PPI8255_ADD( "ppi8255_0", cm_ppi8255_intf[0] )
	MDRV_PPI8255_ADD( "ppi8255_1", cm_ppi8255_intf[1] )

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
//  MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(nfm)
	MDRV_PALETTE_LENGTH(256)
	MDRV_PALETTE_INIT(cm)
	MDRV_NVRAM_HANDLER(goldstar)

	MDRV_VIDEO_START(cherrym)
	MDRV_VIDEO_UPDATE(goldstar)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("aysnd", AY8910, AY_CLOCK)
	MDRV_SOUND_CONFIG(cm_ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 2.00)	/* analyzed for clips */
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( unkch )

	MDRV_DRIVER_DATA(goldstar_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, CPU_CLOCK)
	MDRV_CPU_PROGRAM_MAP(unkch_map)
	MDRV_CPU_IO_MAP(unkch_portmap)
	MDRV_CPU_VBLANK_INT("screen", nmi_line_pulse)
	//MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
//  MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(unkch)
	MDRV_PALETTE_LENGTH(512)

	//MDRV_NVRAM_HANDLER(goldstar)

	MDRV_VIDEO_START(unkch)
	MDRV_VIDEO_UPDATE(unkch)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

//  MDRV_SOUND_ADD("aysnd", AY8910, AY_CLOCK)
//  MDRV_SOUND_CONFIG(ay8910_config)
//  MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( cherrys )

	MDRV_IMPORT_FROM(ncb3)
	MDRV_GFXDECODE(cherrys)

MACHINE_DRIVER_END

// hw unknown
static MACHINE_DRIVER_START( pkrmast )

	MDRV_DRIVER_DATA(goldstar_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, CPU_CLOCK)
	MDRV_CPU_PROGRAM_MAP(cm_map)
	MDRV_CPU_IO_MAP(cm_portmap)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	/* 2x 8255 */
	MDRV_PPI8255_ADD( "ppi8255_0", cm_ppi8255_intf[0] )
	MDRV_PPI8255_ADD( "ppi8255_1", cm_ppi8255_intf[1] )

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
//  MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(pkrmast)
	MDRV_PALETTE_LENGTH(256)
	MDRV_PALETTE_INIT(cm)
	MDRV_NVRAM_HANDLER(goldstar)

	MDRV_VIDEO_START(cherrym)
	MDRV_VIDEO_UPDATE(goldstar)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("aysnd", AY8910, AY_CLOCK)
	MDRV_SOUND_CONFIG(cm_ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( goldstar )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gs4-cpu.bin",  0x0000, 0x10000, CRC(73e47d4d) SHA1(df2d8233572dc12e8a4b56e5d4f6c566e4ababc9) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "gs2.bin",      0x00000, 0x20000, CRC(a2d5b898) SHA1(84cca22c91628cfefb67013652b151f034a06159) )

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "gs3.bin",      0x00000, 0x08000, CRC(8454ce3c) SHA1(74686ebb91f191db8cbc3d0417a5e8112c5b67b1) )

	ROM_REGION( 0x40000, "oki", 0 )	/* Audio ADPCM */
	ROM_LOAD( "gs1-snd.bin",  0x0000, 0x20000, CRC(9d58960f) SHA1(c68edf95743e146398aabf6b9617d18e1f9bf25b) )
ROM_END


ROM_START( goldstbl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gsb-cpu.bin",  0x0000, 0x10000, CRC(82b238c3) SHA1(1306e700e213f423bdd79b182aa11335796f7f38) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "gs2.bin",      0x00000, 0x20000, CRC(a2d5b898) SHA1(84cca22c91628cfefb67013652b151f034a06159) )

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "gsb-spr.bin",  0x00000, 0x08000, CRC(52ecd4c7) SHA1(7ef013020521a0c19ecd67db1c00047e78a3c736) )

	ROM_REGION( 0x40000, "oki", 0 )	/* Audio ADPCM */
	ROM_LOAD( "gs1-snd.bin",  0x0000, 0x20000, CRC(9d58960f) SHA1(c68edf95743e146398aabf6b9617d18e1f9bf25b) )
ROM_END

/*

Cherry I Gold

Anno    1998
Produttore
N.revisione W4BON (rev.1)


CPU

1x TMPZ84C00AP-6 (u12)(main)
2x D8255AC-2 (u45,u46) (missing)
1x D71055C (u40) (missing)
1x YM2149 (u39)
1x SN76489AN (u38)
1x oscillator 12.0C45

ROMs

1x I27256 (u3)
1x I27C010 (u1)
1x PROM N82S147AN (u2)
1x M27C512 (u20)
1x GAL20V8 (pl1)(read protected)
1x PALCE20V8H (pl2)(read protected)
1x ispLSI1024-60LJ (pl3)(read protected)
3x PALCE16V8H (pl4,pl6,pl7)(read protected)
1x PEEL22CV10 (pl5)(read protected)

Note

1x 36x2 edge connector
1x 10x2 edge connector
2x trimmer (volume)
5x 8x2 switches dip (sw1-5)
1x push lever (TS)


Cherry Gold  (Cherry 10)

Anno    1997
Produttore
N.revisione W4BON (rev.1)

CPU

1x TMPZ84C00AP-6 (u12)(main)
2x D8255AC-2 (u45,u46)
1x D71055C (u40)
1x WF19054 (u39)(equivalent to AY-3-8910)
1x SN76489AN (u38)
1x PIC16F84 (on a small daughterboard)(read protected)
1x oscillator 12.000

ROMs

1x TMS27C256 (u3)
1x TMS27C010 (u1)
1x PROM N82S147AN (u2)
1x M27C512 (u20)
2x PALCE20V8H (pl1,pl2)(read protected)
1x ispLSI1024-60LJ (pl3)(read protected)
3x PALCE16V8H (pl4,pl6,pl7)(read protected)
1x GAL22V10B (pl5)(read protected)

Note

1x 36x2 edge connector
1x 10x2 edge connector
2x trimmer (volume)
5x 8x2 switches dip (sw1-5)
1x push lever (TS)

*/

ROM_START( chry10 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ver.1h2.u20",  0x0000, 0x10000, CRC(85bbde06) SHA1(f44d335feb4697b195e9fc7e5aeaabf099e21ed8) )

	ROM_REGION( 0x10000, "pic", 0 )
	ROM_LOAD( "pic16f84.bad.dump",    0x00000, 0x014f4, BAD_DUMP CRC(876ff1ed) SHA1(fcd6892e2b8371030af15e4d8c9f4a351ce0551c) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "27c010.u1",      0x00000, 0x20000, CRC(05515cf8) SHA1(366dd44ae93bdc4cf456f97f38edac83441cbc89) )

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "1.u3",      0x00000, 0x08000, CRC(32b46e5c) SHA1(49e59589188324e15ec2b8157839423faea9833f) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147.u2",      0x00000, 0x0200, CRC(5c8f2b8f) SHA1(67d2121e75813dd85d83858c5fc5ec6ad9cc2a7d) )

	ROM_REGION( 0x02e5, "palgal", 0 )
	ROM_LOAD( "palce20v8h.pl1.bad.dump",    0x00000, 0x0157, BAD_DUMP CRC(f0c6d78c) SHA1(03ff589711179950209c405192bd41a032c6c6d6) )
	ROM_LOAD( "palce20v8h.pl2.bad.dump",    0x00000, 0x0157, BAD_DUMP CRC(f0c6d78c) SHA1(03ff589711179950209c405192bd41a032c6c6d6) )
	ROM_LOAD( "palce16v8h.pl4.bad.dump",    0x00000, 0x0117, BAD_DUMP CRC(c89d2f52) SHA1(f9d52d9c42ef95b7b85bbf6d09888ebdeac11fd3) )
	ROM_LOAD( "gal22v10b.pl5.bad.dump",     0x00000, 0x02e5, BAD_DUMP CRC(996854bc) SHA1(647d2f49b739f7ca55c0b85290b6a21256834fd8) )
	ROM_LOAD( "palce16v8h.pl6.bad.dump",    0x00000, 0x0117, BAD_DUMP CRC(7e3d99d8) SHA1(983e10eba11e4aeab5103ae644a8e6181d9b27a9) )
	ROM_LOAD( "palce16v8h.pl7.bad.dump",    0x00000, 0x0117, BAD_DUMP CRC(c89d2f52) SHA1(f9d52d9c42ef95b7b85bbf6d09888ebdeac11fd3) )
ROM_END



ROM_START( chrygld )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ol-v9.u20",  0x00000, 0x10000, CRC(b61c0695) SHA1(63c44b20fd7f76bdb33331273d2610e8cfd31add) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "ol-la.u1",      0x00000, 0x20000, CRC(c3c912f1) SHA1(a2131f092ae1971f79a11d6a18b031cd98529320) )

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "1.u3",      0x00000, 0x08000, CRC(32b46e5c) SHA1(49e59589188324e15ec2b8157839423faea9833f) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147.u2",      0x00000, 0x0200, CRC(5c8f2b8f) SHA1(67d2121e75813dd85d83858c5fc5ec6ad9cc2a7d) )

	ROM_REGION( 0x02dd, "palgal", 0 )
	ROM_LOAD( "gal20v8.pl1.bad.dump",    0x00000, 0x0157, BAD_DUMP CRC(bf885908) SHA1(6cac1022172ee0c178fd3b9c187b1ffb4742898f) )
	ROM_LOAD( "palce20v8h.pl2.bad.dump", 0x00000, 0x0157, BAD_DUMP CRC(f0c6d78c) SHA1(03ff589711179950209c405192bd41a032c6c6d6) )
	ROM_LOAD( "palce16v8h.pl4.bad.dump", 0x00000, 0x0117, BAD_DUMP CRC(c89d2f52) SHA1(f9d52d9c42ef95b7b85bbf6d09888ebdeac11fd3) )
	ROM_LOAD( "peel22cv10a.pl5.bad.dump",0x00000, 0x02dd, BAD_DUMP CRC(8e6075d9) SHA1(f2c1b6497a4d9e873d36b89771c135a2cd91d05f) )
	ROM_LOAD( "palce16v8h.pl6.bad.dump", 0x00000, 0x0117, BAD_DUMP CRC(7e3d99d8) SHA1(983e10eba11e4aeab5103ae644a8e6181d9b27a9) )
	ROM_LOAD( "palce16v8h.pl7.bad.dump", 0x00000, 0x0117, BAD_DUMP CRC(c89d2f52) SHA1(f9d52d9c42ef95b7b85bbf6d09888ebdeac11fd3) )
ROM_END



ROM_START( moonlght )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "4.bin",       0x0000, 0x20000, CRC(ecb06cfb) SHA1(e32613cac5583a0fecf04fca98796b91698e530c) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "28.bin",      0x00000, 0x20000, CRC(76915c0f) SHA1(3f6d1c0dd3d9bf29538181a0e930291b822dad8c) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "29.bin",      0x00000, 0x20000, CRC(8a5f274d) SHA1(0f2ad61b00e220fc509c01c11c1a8f4e47b54f2a) )

	ROM_REGION( 0x40000, "oki", 0 )	/* Audio ADPCM */
	ROM_LOAD( "gs1-snd.bin",  0x0000, 0x20000, CRC(9d58960f) SHA1(c68edf95743e146398aabf6b9617d18e1f9bf25b) )
ROM_END



ROM_START( ncb3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "8.512", 0x00000, 0x10000, CRC(1f669cd0) SHA1(fd394119e33c017507fde87a710577e37dcdec07) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "2.256", 0x00000, 0x08000, CRC(83650a94) SHA1(e79420ab559d3f74013708767ca3f238fd333fb7) )
	ROM_LOAD( "3.256", 0x08000, 0x08000, CRC(2f46a3f5) SHA1(3e6022ee8f84039e48f41aea5e68ee28aabdc556) )
	//ROM_LOAD( "4.256", 0x10000, 0x08000, BAD_DUMP CRC(a390f1f2) SHA1(0a04a5af51f91f04773125f703c7cd3397d192f2) ) // FIXED BITS (xxxx1xxx) - use main_7.256 from set below instead?
	ROM_LOAD( "main_7.256", 0x10000, 0x08000, CRC(dcf97517) SHA1(0a29696e0464c8878c499b1786a17080fd088a72) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "7.764", 0x00000, 0x02000, CRC(7cc6d26b) SHA1(de33e8985affce7bd3ead89463117c9aaa93d5e4) )
	ROM_LOAD( "6.764", 0x02000, 0x02000, CRC(e73ea4e3) SHA1(c9fd56461f6986d6bc170403d298fcc408a524e9) )
	ROM_LOAD( "5.764", 0x04000, 0x02000, CRC(91162010) SHA1(3acc21e7074602b247f2f392eb181802092d2f21) )
	ROM_LOAD( "1.764", 0x06000, 0x02000, CRC(cbcc6bfb) SHA1(5bafc934fef1f50d8c182c39d3a7ce795c89d175) )

	ROM_REGION( 0x0200, "proms", 0 )	/* PROM from chrygld. need verification */
	ROM_LOAD( "82s147.u2",      0x00000, 0x0200, BAD_DUMP CRC(5c8f2b8f) SHA1(67d2121e75813dd85d83858c5fc5ec6ad9cc2a7d) )
ROM_END

/*
mame -romident cb3.zip
cpu_u6.512          NO MATCH
main_3.764          = 5.764                 New Cherry Bonus 3
main_4.764          = 1.764                 New Cherry Bonus 3
main_5.256          = 2.256                 New Cherry Bonus 3
main_6.256          = 3.256                 New Cherry Bonus 3
main_7.256          NO MATCH

C:\mame061208>src\mame\mamedriv.c

*/

ROM_START( cb3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cpu_u6.512", 0x00000, 0x10000, CRC(d17c936b) SHA1(bf90edd214118116da675bcfca41247d5891ac90) ) // encrypted??

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "main_5.256", 0x00000, 0x08000, CRC(83650a94) SHA1(e79420ab559d3f74013708767ca3f238fd333fb7) )
	ROM_LOAD( "main_6.256", 0x08000, 0x08000, CRC(2f46a3f5) SHA1(3e6022ee8f84039e48f41aea5e68ee28aabdc556) )
	ROM_LOAD( "main_7.256", 0x10000, 0x08000, CRC(dcf97517) SHA1(0a29696e0464c8878c499b1786a17080fd088a72) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	/* 2 roms missing - the first 2 roms below taken from above set */
	ROM_LOAD( "7.764",      0x00000, 0x02000, CRC(7cc6d26b) SHA1(de33e8985affce7bd3ead89463117c9aaa93d5e4) )
	ROM_LOAD( "6.764",      0x02000, 0x02000, CRC(e73ea4e3) SHA1(c9fd56461f6986d6bc170403d298fcc408a524e9) )
	ROM_LOAD( "main_3.764", 0x04000, 0x02000, CRC(91162010) SHA1(3acc21e7074602b247f2f392eb181802092d2f21) )
	ROM_LOAD( "main_4.764", 0x06000, 0x02000, CRC(cbcc6bfb) SHA1(5bafc934fef1f50d8c182c39d3a7ce795c89d175) )

	ROM_REGION( 0x0200, "proms", 0 )	/* PROM from chrygld. need verification */
	ROM_LOAD( "82s147.u2",      0x00000, 0x0200, BAD_DUMP CRC(5c8f2b8f) SHA1(67d2121e75813dd85d83858c5fc5ec6ad9cc2a7d) )
ROM_END

/*
CB3A
Known differences with ncb3:

- Double-Up rate: 50% and 80% instead of 80% and 90%.

*/

ROM_START( cb3a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cb3a01.bin", 0x00000, 0x10000, CRC(53b099ab) SHA1(612d86d7f011a554903400e60e2c4a0d4f24e095) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "2.256", 0x00000, 0x08000, CRC(83650a94) SHA1(e79420ab559d3f74013708767ca3f238fd333fb7) )
	ROM_LOAD( "3.256", 0x08000, 0x08000, CRC(2f46a3f5) SHA1(3e6022ee8f84039e48f41aea5e68ee28aabdc556) )
	ROM_LOAD( "main_7.256", 0x10000, 0x08000, CRC(dcf97517) SHA1(0a29696e0464c8878c499b1786a17080fd088a72) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "7.764", 0x00000, 0x02000, CRC(7cc6d26b) SHA1(de33e8985affce7bd3ead89463117c9aaa93d5e4) )
	ROM_LOAD( "6.764", 0x02000, 0x02000, CRC(e73ea4e3) SHA1(c9fd56461f6986d6bc170403d298fcc408a524e9) )
	ROM_LOAD( "5.764", 0x04000, 0x02000, CRC(91162010) SHA1(3acc21e7074602b247f2f392eb181802092d2f21) )
	ROM_LOAD( "1.764", 0x06000, 0x02000, CRC(cbcc6bfb) SHA1(5bafc934fef1f50d8c182c39d3a7ce795c89d175) )

	ROM_REGION( 0x0200, "proms", 0 )	/* PROM from chrygld. need verification */
	ROM_LOAD( "82s147.u2",      0x00000, 0x0200, BAD_DUMP CRC(5c8f2b8f) SHA1(67d2121e75813dd85d83858c5fc5ec6ad9cc2a7d) )
ROM_END


ROM_START( cb3b )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "adatabin.bin",   0x00000, 0x10000,  CRC(db583c1b) SHA1(ea733e625922d6064ee4d8ceee4acfa6c1c7337e) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "adatabin_2.bin",   0x00000, 0x10000, CRC(48fd96fb) SHA1(193ed2be51555af80a9f0478139f28963e9d0c5e) )
	ROM_LOAD( "adatabin_3.bin",   0x10000, 0x10000, CRC(010462df) SHA1(53dd3060097f964c516d1cc5be2403a9bd5ee434) )

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "adatabin_4.bin",   0x00000, 0x08000, CRC(3cb4642a) SHA1(8db03c0227836d988e99f6fe4719d871ea3749ca) )

	ROM_REGION( 0x0200, "proms", 0 )	/* PROM from chrygld - wasn't in this set, is it correct?, none of the other proms have the colours? */
	ROM_LOAD( "82s147.u2",      0x00000, 0x0200, BAD_DUMP CRC(5c8f2b8f) SHA1(67d2121e75813dd85d83858c5fc5ec6ad9cc2a7d) )

	ROM_REGION( 0x0200, "proms2", 0 )	/* other roms */
	ROM_LOAD( "adatabin_1.bin",      0x00000, 0x020, CRC(87dbc339) SHA1(e5c67bc29612c8ab93857639e46608a814d471f5) )
	ROM_LOAD( "adatabin_5.bin",      0x00000, 0x180, CRC(ad267b0c) SHA1(a4cfec15ae0cde7d4fb8c278e977995680779058) )
	ROM_LOAD( "adatabin_6.bin",      0x00000, 0x010, CRC(f3d9ed7a) SHA1(594fef6323530f68c7303dcdea77b44c331e5113) )
	ROM_LOAD( "adatabin_0.bin",      0x00000, 0x100, CRC(f566e5e0) SHA1(754f04521b9eb73b34fe3de07e8f3679d1034870) )
ROM_END

ROM_START( cb3c )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "27c512.bin", 0x00000, 0x10000, CRC(c42533cd) SHA1(d55b54b31c910d97418f400fc1ba78460c7183a9) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "cbt1grk.bin", 0x00000, 0x20000, BAD_DUMP CRC(c6fdebc7) SHA1(736bbe5ae7b148e529f7cb80e9ae8903203c7869) )

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "rbt1grb.bin", 0x00000, 0x8000, CRC(ed635dd7) SHA1(350a4b10ccfddcd6f3aaf748c15d585f0b9dc09b) )

	ROM_REGION( 0x0200, "proms", 0 ) // wasn't in this set..
	ROM_LOAD( "82s147.u2",      0x00000, 0x0200, CRC(5c8f2b8f) SHA1(67d2121e75813dd85d83858c5fc5ec6ad9cc2a7d) )
ROM_END


ROM_START( cmv801 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "prg512",   0x0000, 0x10000, CRC(2f6e3fe9) SHA1(c5ffa51478a0dc2d8ff6a0f286cfb461011bb55d) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "m5.256",   0x00000, 0x8000, CRC(19cc1d67) SHA1(47487f9362bfb36a32100ed772960628844462bf) )
	ROM_LOAD( "m6.256",   0x08000, 0x8000, CRC(63b3df4e) SHA1(9bacd23da598805ec18ec5ad15cab95d71eb9262) )
	ROM_LOAD( "m7.256",   0x10000, 0x8000, CRC(e39fff9c) SHA1(22fdc517fa478441622c6245cecb5728c5595757) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "m3.64",     0x0000, 0x2000, CRC(8607ffd9) SHA1(9bc94715554aa2473ae2ed249a47f29c7886b3dc) )
	ROM_LOAD( "m4.64",     0x2000, 0x2000, CRC(c32367be) SHA1(ff217021b9c58e23b2226f8b0a7f5da966225715) )
	ROM_LOAD( "m1.64",     0x4000, 0x2000, CRC(6dfcb188) SHA1(22430429c798954d9d979e62699b58feae7fdbf4) )
	ROM_LOAD( "m2.64",     0x6000, 0x2000, CRC(9678ead2) SHA1(e80aefa98b2363fe9e6b2415762695ace272e4d3) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "27512.u53",  0x0000, 0x10000, BAD_DUMP CRC(e92443d3) SHA1(4b6ca4521841610054165f085ae05510e77af191) ) // taken from other set, was missing in this dump

	ROM_REGION( 0x200, "proms", 0 ) // pal
	ROM_LOAD( "prom2.287", 0x0000, 0x0100, CRC(0489b760) SHA1(78f8632b17a76335183c5c204cdec856988368b0) )
	ROM_LOAD( "prom3.287", 0x0100, 0x0100, CRC(21eb5b19) SHA1(9b8425bdb97f11f4855c998c7792c3291fd07470) )

	ROM_REGION( 0x100, "proms2", 0 ) // something else?
	ROM_LOAD( "prom1.287", 0x0000, 0x0100, CRC(50ec383b) SHA1(ae95b92bd3946b40134bcdc22708d5c6b0f4c23e) )
ROM_END

/*
2764.u10                m4.64                   IDENTICAL
27256.u11               m6.256                  IDENTICAL
2764.u14                m1.64                   IDENTICAL
2764.u15                m3.64                   IDENTICAL
27256.u16               m5.256                  IDENTICAL
27256.u4                m7.256                  IDENTICAL
82s129.u46              prom1.287               IDENTICAL
82s129.u70              prom3.287               IDENTICAL
82s129.u84              prom2.287               IDENTICAL
2764.u9                 m2.64                   IDENTICAL
27512.u53               prg512                  4.640198%
27256.u81                                       NO MATCH

PCB Layout
----------

|-----|  |------|  |---------------------------|
|     |--|      |--|  ROM.U4                   |
|                     ROM.U11  ROM.U10  ROM.U9 |
|_            DSW5(8) ROM.U16  ROM.U15  ROM.U14|
  |  WF19054  DSW4(8)                          |
 _|           DSW3(8) 6116                     |
|             DSW2(8) 6116         6116        |
|             DSW1(8)                          |
|    ?DIP40                        6116        |
|                                              |
|                     PROM.U46            12MHz|
|                                              |
|    8255                       PAL            |
|         ROM.U53                              |
|                                              |
|                                              |
|                                   PAL        |
|BATTERY  PROM.U70        PAL   PAL            |
|_        PROM.U84        6116  ROM.U81    Z80 |
  |--------------------------------------------|
Notes:
      Z80 @ 3.0MHz [12/4]
      WF19054 = AY-3-8910 @ 1.5MHz [12/8]
      ?DIP40 - Maybe another 8255 or HD6845 or something else?
*/

ROM_START( cmv4 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "27256.u81",  0x0000, 0x1000, CRC(e27e98a3) SHA1(1eb03f6c770f25ff5e3c25a1f9b9294c6b3c61d9) )
	ROM_CONTINUE(0x3000,0x1000)
	ROM_CONTINUE(0x7000,0x1000)
	ROM_CONTINUE(0x5000,0x1000)
	ROM_CONTINUE(0x6000,0x1000)
	ROM_CONTINUE(0x2000,0x1000)
	ROM_CONTINUE(0x4000,0x1000)
	ROM_CONTINUE(0x1000,0x1000)

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "27256.u16",  0x00000, 0x8000, CRC(19cc1d67) SHA1(47487f9362bfb36a32100ed772960628844462bf) )
	ROM_LOAD( "27256.u11",  0x08000, 0x8000, CRC(63b3df4e) SHA1(9bacd23da598805ec18ec5ad15cab95d71eb9262) )
	ROM_LOAD( "27256.u4",   0x10000, 0x8000, CRC(e39fff9c) SHA1(22fdc517fa478441622c6245cecb5728c5595757) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "2764.u15",   0x0000, 0x2000, CRC(8607ffd9) SHA1(9bc94715554aa2473ae2ed249a47f29c7886b3dc) )
	ROM_LOAD( "2764.u10",	0x2000, 0x2000, CRC(c32367be) SHA1(ff217021b9c58e23b2226f8b0a7f5da966225715) )
	ROM_LOAD( "2764.u14",   0x4000, 0x2000, CRC(6dfcb188) SHA1(22430429c798954d9d979e62699b58feae7fdbf4) )
	ROM_LOAD( "2764.u9",    0x6000, 0x2000, CRC(9678ead2) SHA1(e80aefa98b2363fe9e6b2415762695ace272e4d3) )

	ROM_REGION( 0x10000, "user1", 0 ) // girl bitmaps
	ROM_LOAD( "27512.u53",  0x0000, 0x10000, CRC(e92443d3) SHA1(4b6ca4521841610054165f085ae05510e77af191) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s129.u84", 0x0000, 0x0100, CRC(0489b760) SHA1(78f8632b17a76335183c5c204cdec856988368b0) )
	ROM_LOAD( "82s129.u70", 0x0100, 0x0100, CRC(21eb5b19) SHA1(9b8425bdb97f11f4855c998c7792c3291fd07470) )

	ROM_REGION( 0x100, "proms2", 0 )
	ROM_LOAD( "82s129.u46", 0x0000, 0x0100, CRC(50ec383b) SHA1(ae95b92bd3946b40134bcdc22708d5c6b0f4c23e) )
ROM_END

/*

cmv4 vs. fruit2

12 and 12 files
82s129.u46                                      FIXED BITS (0000x0xx)
82s129.u46                                      BADADDR                xxxxxxx-
82s129.u70                                      FIXED BITS (0000xxxx)
82s129.u84                                      FIXED BITS (0000xxxx)
                        HOLD8_PR1.U46           FIXED BITS (0000x0xx)
                        HOLD8_PR1.U46           BADADDR                xxxxxxx-
                        HOLD8_PR2.U84           FIXED BITS (0000xxxx)
                        HOLD8_PR3.U79           FIXED BITS (0000xxxx)
                        TETRIS_5.U4             1ST AND 2ND HALF IDENTICAL
                        TETRIS_6.U11            1ST AND 2ND HALF IDENTICAL
                        TETRIS_7.U16            1ST AND 2ND HALF IDENTICAL
2764.u10                HOLDX8_3.U10            IDENTICAL
2764.u14                HOLDX8_2.U14            IDENTICAL
2764.u15                HOLDX8_4.U15            IDENTICAL
82s129.u46              HOLD8_PR1.U46            IDENTICAL
27512.u53               HOLDX8_8.U53            IDENTICAL
82s129.u70              HOLD8_PR3.U79            IDENTICAL
82s129.u84              HOLD8_PR2.U84            IDENTICAL
2764.u9                 HOLDX8_1.U9             IDENTICAL
27256.u81               HOLDX8.U81              25.512695%
27256.u11                                       NO MATCH
27256.u16                                       NO MATCH
27256.u4                                        NO MATCH
                        TETRIS_5.U4             NO MATCH
                        TETRIS_6.U11            NO MATCH
                        TETRIS_7.U16            NO MATCH
*/

ROM_START( cmv4a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "holdx8.u81",	0x0000, 0x2000, CRC(2da4d267) SHA1(3deb6ce4703bb1cca2b58409a6fc5ba7cf601011) )
	ROM_CONTINUE(0x4000,0x2000)
	ROM_CONTINUE(0x2000,0x2000)
	ROM_CONTINUE(0x6000,0x2000)

	ROM_REGION( 0x30000, "gfx1", 0 )	/* double size... tetris gfx inside. */
	ROM_LOAD( "tetris_7.u16",  0x00000, 0x10000, CRC(a9e61fac) SHA1(a4093868b570af52f1cd816d523dbb6a750bc1af) )
	ROM_LOAD( "tetris_6.u11",  0x10000, 0x10000, CRC(f3e15b41) SHA1(6c44bcdcf7b29d201018e2c2cc0b6b0b62cbaf12) )
	ROM_LOAD( "tetris_5.u4",   0x20000, 0x10000, CRC(79a85560) SHA1(b028345ac2d01c643230c6e6c28189c11734aaf5) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "holdx8_4.u15",   0x0000, 0x2000, CRC(8607ffd9) SHA1(9bc94715554aa2473ae2ed249a47f29c7886b3dc) )
	ROM_LOAD( "holdx8_3.u10",	0x2000, 0x2000, CRC(c32367be) SHA1(ff217021b9c58e23b2226f8b0a7f5da966225715) )
	ROM_LOAD( "holdx8_2.u14",   0x4000, 0x2000, CRC(6dfcb188) SHA1(22430429c798954d9d979e62699b58feae7fdbf4) )
	ROM_LOAD( "holdx8_1.u9",    0x6000, 0x2000, CRC(9678ead2) SHA1(e80aefa98b2363fe9e6b2415762695ace272e4d3) )

	ROM_REGION( 0x10000, "user1", 0 )
	// contains a bitmap? and an extra plane for gfx2, should it be used?
	ROM_LOAD( "holdx8_8.u53",  0x0000, 0x10000, CRC(e92443d3) SHA1(4b6ca4521841610054165f085ae05510e77af191) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "hold8_pr2.u84", 0x0000, 0x0100, CRC(0489b760) SHA1(78f8632b17a76335183c5c204cdec856988368b0) )
	ROM_LOAD( "hold8_pr3.u79", 0x0100, 0x0100, CRC(21eb5b19) SHA1(9b8425bdb97f11f4855c998c7792c3291fd07470) )

	ROM_REGION( 0x100, "proms2", 0 )
	ROM_LOAD( "hold8_pr1.u46", 0x0000, 0x0100, CRC(50ec383b) SHA1(ae95b92bd3946b40134bcdc22708d5c6b0f4c23e) )
ROM_END


ROM_START( cmwm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "waterm.rom",  0x0000, 0x1000, CRC(93b6cb9b) SHA1(294e1e5909b304252c79a7d3f50fc175558e713b) )
	ROM_CONTINUE(0x4000,0x1000)
	ROM_CONTINUE(0x3000,0x1000)
	ROM_CONTINUE(0x7000,0x1000)
	ROM_CONTINUE(0x1000,0x1000)
	ROM_CONTINUE(0x6000,0x1000)
	ROM_CONTINUE(0x2000,0x1000)
	ROM_CONTINUE(0x5000,0x1000)

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "27256.u16",  0x00000, 0x8000, CRC(19cc1d67) SHA1(47487f9362bfb36a32100ed772960628844462bf) )
	ROM_LOAD( "27256.u11",  0x08000, 0x8000, CRC(63b3df4e) SHA1(9bacd23da598805ec18ec5ad15cab95d71eb9262) )
	ROM_LOAD( "27256.u4",   0x10000, 0x8000, CRC(e39fff9c) SHA1(22fdc517fa478441622c6245cecb5728c5595757) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "2764.u15",   0x0000, 0x2000, CRC(8607ffd9) SHA1(9bc94715554aa2473ae2ed249a47f29c7886b3dc) )
	ROM_LOAD( "2764.u10",	0x2000, 0x2000, CRC(c32367be) SHA1(ff217021b9c58e23b2226f8b0a7f5da966225715) )
	ROM_LOAD( "2764.u14",   0x4000, 0x2000, CRC(6dfcb188) SHA1(22430429c798954d9d979e62699b58feae7fdbf4) )
	ROM_LOAD( "2764.u9",    0x6000, 0x2000, CRC(9678ead2) SHA1(e80aefa98b2363fe9e6b2415762695ace272e4d3) )

	ROM_REGION( 0x10000, "user1", 0 ) // girl bitmaps
	ROM_LOAD( "27512.u53",  0x0000, 0x10000, CRC(e92443d3) SHA1(4b6ca4521841610054165f085ae05510e77af191) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s129.u84", 0x0000, 0x0100, CRC(0489b760) SHA1(78f8632b17a76335183c5c204cdec856988368b0) )
	ROM_LOAD( "82s129.u70", 0x0100, 0x0100, CRC(21eb5b19) SHA1(9b8425bdb97f11f4855c998c7792c3291fd07470) )

	ROM_REGION( 0x100, "proms2", 0 )
	ROM_LOAD( "82s129.u46", 0x0000, 0x0100, CRC(50ec383b) SHA1(ae95b92bd3946b40134bcdc22708d5c6b0f4c23e) )
ROM_END

ROM_START( cmfun )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cmvfun.rom",  0x0000, 0x1000, CRC(128f373e) SHA1(24d51ab669d568c004e2c94cac22eb8476ce2718) )
	ROM_CONTINUE(0x4000,0x1000)
	ROM_CONTINUE(0x3000,0x1000)
	ROM_CONTINUE(0x7000,0x1000)
	ROM_CONTINUE(0x1000,0x1000)
	ROM_CONTINUE(0x6000,0x1000)
	ROM_CONTINUE(0x2000,0x1000)
	ROM_CONTINUE(0x5000,0x1000)

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "27256.u16",  0x00000, 0x8000, CRC(19cc1d67) SHA1(47487f9362bfb36a32100ed772960628844462bf) )
	ROM_LOAD( "27256.u11",  0x08000, 0x8000, CRC(63b3df4e) SHA1(9bacd23da598805ec18ec5ad15cab95d71eb9262) )
	ROM_LOAD( "27256.u4",   0x10000, 0x8000, CRC(e39fff9c) SHA1(22fdc517fa478441622c6245cecb5728c5595757) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "2764.u15",   0x0000, 0x2000, CRC(8607ffd9) SHA1(9bc94715554aa2473ae2ed249a47f29c7886b3dc) )
	ROM_LOAD( "2764.u10",	0x2000, 0x2000, CRC(c32367be) SHA1(ff217021b9c58e23b2226f8b0a7f5da966225715) )
	ROM_LOAD( "2764.u14",   0x4000, 0x2000, CRC(6dfcb188) SHA1(22430429c798954d9d979e62699b58feae7fdbf4) )
	ROM_LOAD( "2764.u9",    0x6000, 0x2000, CRC(9678ead2) SHA1(e80aefa98b2363fe9e6b2415762695ace272e4d3) )

	ROM_REGION( 0x10000, "user1", 0 ) // girl bitmaps
	ROM_LOAD( "27512.u53",  0x0000, 0x10000, CRC(e92443d3) SHA1(4b6ca4521841610054165f085ae05510e77af191) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s129.u84", 0x0000, 0x0100, CRC(0489b760) SHA1(78f8632b17a76335183c5c204cdec856988368b0) )
	ROM_LOAD( "82s129.u70", 0x0100, 0x0100, CRC(21eb5b19) SHA1(9b8425bdb97f11f4855c998c7792c3291fd07470) )

	ROM_REGION( 0x100, "proms2", 0 )
	ROM_LOAD( "82s129.u46", 0x0000, 0x0100, CRC(50ec383b) SHA1(ae95b92bd3946b40134bcdc22708d5c6b0f4c23e) )
ROM_END

/* looks like a bootleg of cmv4 */
ROM_START( cmaster )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "3.bin",   0x00000, 0x1000, CRC(ccb64229) SHA1(532f4b59952702a3609ff20239acbbacaf71f38f) )
	ROM_CONTINUE(0x4000,0x1000)
	ROM_CONTINUE(0x3000,0x1000)
	ROM_CONTINUE(0x7000,0x1000)
	ROM_CONTINUE(0x1000,0x1000)
	ROM_CONTINUE(0x6000,0x1000)
	ROM_CONTINUE(0x2000,0x1000)
	ROM_CONTINUE(0x5000,0x1000)
	ROM_IGNORE(0x8000) // 2nd half is identical

	ROM_REGION( 0x30000, "gfx1", 0 )
	ROM_LOAD( "6.bin",   0x00000, 0x10000, CRC(a98d610f) SHA1(d14b3bc8bd6dc9fe2d3fb05ec08224b1a9d52bee) )
	ROM_LOAD( "7.bin",   0x10000, 0x10000, CRC(a0ffd2d6) SHA1(e78d20d3ab578ccc880bc19678782cb1f8d3671e) )
	ROM_LOAD( "8.bin",   0x20000, 0x10000, CRC(4f67fca7) SHA1(808e84b9b1f67f137528bb76b0e8aac3dceba20c) )

	// 4-9 all of these contain the same bitmap at 0x0000-0xe000
	// rom 9 matches the rom containing the bitmap in the cmv4 set, but also contains an extra gfx2 plane
	//  should that plane be used??
	ROM_REGION( 0x50000, "graphics", 0 )
	ROM_LOAD( "4.bin",   0x00000, 0x10000, CRC(52240e0f) SHA1(7b8375e1f91fdff2b4ccc2e81fbcf843f7ede292) )
	ROM_LOAD( "5.bin",   0x10000, 0x10000, CRC(763973c1) SHA1(b364f22041f1d678332554edb3c718cf0ad778b4) )
	ROM_LOAD( "1.bin",   0x20000, 0x10000, CRC(634fe2ad) SHA1(2284a09446c8928060270861d372a19c0c9d827a) )
	ROM_LOAD( "2.bin",   0x30000, 0x10000, CRC(a3d59f79) SHA1(588c45550cca673390a35af9617c68c853ff84ba) )
	ROM_LOAD( "9.bin",   0x40000, 0x10000, CRC(e92443d3) SHA1(4b6ca4521841610054165f085ae05510e77af191) )

	// for now we don't copy the extra plane..
	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "graphics", 0x0e000, 0x00000, 0x2000 )
	ROM_COPY( "graphics", 0x1e000, 0x02000, 0x2000 )
	ROM_COPY( "graphics", 0x2e000, 0x04000, 0x2000 )
	ROM_COPY( "graphics", 0x3e000, 0x06000, 0x2000 )

	// copy one of the versions of the bitmap
	ROM_REGION( 0x10000, "user1", 0 )
	ROM_COPY( "graphics", 0x40000, 0x00000, 0xe000 )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s129.u84", 0x0000, 0x0100, CRC(0489b760) SHA1(78f8632b17a76335183c5c204cdec856988368b0) )
	ROM_LOAD( "82s129.u70", 0x0100, 0x0100, CRC(21eb5b19) SHA1(9b8425bdb97f11f4855c998c7792c3291fd07470) )

	ROM_REGION( 0x100, "proms2", 0 )
	ROM_LOAD( "82s129.u46", 0x0000, 0x0100, CRC(50ec383b) SHA1(ae95b92bd3946b40134bcdc22708d5c6b0f4c23e) )
ROM_END


/*  cmasterb & cmasterc

    Cherry Master II ver 8.41

    CPU : LH0080B Z80B CPU

    RAM : MB8128-15 x2

        MB8416-20 x2

        HM6116

    Sound : Winbond WF19054

    Crystal : 12.000 Mhz

    Dip SW :

    4x8 dip + 1 Switch (main test ???)
*/

ROM_START( cmasterb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u81.9",  0x0000,  0x1000, CRC(09e44314) SHA1(dbb7e9afc9a1dc0d4ce7b150324077f3f3579c02) )
	ROM_CONTINUE(0x4000,0x1000)
	ROM_CONTINUE(0x3000,0x1000)
	ROM_CONTINUE(0x7000,0x1000)
	ROM_CONTINUE(0x1000,0x1000)
	ROM_CONTINUE(0x6000,0x1000)
	ROM_CONTINUE(0x2000,0x1000)
	ROM_CONTINUE(0x5000,0x1000)

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "u16.7", 0x00000,  0x8000, CRC(19cc1d67) SHA1(47487f9362bfb36a32100ed772960628844462bf) )
	ROM_LOAD( "u11.6", 0x08000,  0x8000, CRC(63b3df4e) SHA1(9bacd23da598805ec18ec5ad15cab95d71eb9262) )
	ROM_LOAD( "u4.5",  0x10000,  0x8000, CRC(e39fff9c) SHA1(22fdc517fa478441622c6245cecb5728c5595757) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "u15.4",  0x0000,  0x2000, CRC(8607ffd9) SHA1(9bc94715554aa2473ae2ed249a47f29c7886b3dc) )
	ROM_LOAD( "u10.3",  0x2000,  0x2000, CRC(c32367be) SHA1(ff217021b9c58e23b2226f8b0a7f5da966225715) )
	ROM_LOAD( "u14.2",  0x4000,  0x2000, CRC(6dfcb188) SHA1(22430429c798954d9d979e62699b58feae7fdbf4) )
	ROM_LOAD( "u9.1",   0x6000,  0x2000, CRC(9678ead2) SHA1(e80aefa98b2363fe9e6b2415762695ace272e4d3) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "u53.8",  0x0000, 0x10000, CRC(e92443d3) SHA1(4b6ca4521841610054165f085ae05510e77af191) )

	/* proms taken from cmv4, probably wrong  */
	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s129.u84", 0x0000, 0x0100, CRC(0489b760) SHA1(78f8632b17a76335183c5c204cdec856988368b0) )
	ROM_LOAD( "82s129.u70", 0x0100, 0x0100, CRC(21eb5b19) SHA1(9b8425bdb97f11f4855c998c7792c3291fd07470) )

	ROM_REGION( 0x100, "proms2", 0 )
	ROM_LOAD( "82s129.u46", 0x0000, 0x0100, CRC(50ec383b) SHA1(ae95b92bd3946b40134bcdc22708d5c6b0f4c23e) )
ROM_END


ROM_START( cmezspin )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ezspin.rom",  0x0000,  0x1000, CRC(de92b85c) SHA1(36e99b1444980a279293839c6db10f577b9e8657) )
	ROM_CONTINUE(0x4000,0x1000)
	ROM_CONTINUE(0x3000,0x1000)
	ROM_CONTINUE(0x7000,0x1000)
	ROM_CONTINUE(0x1000,0x1000)
	ROM_CONTINUE(0x6000,0x1000)
	ROM_CONTINUE(0x2000,0x1000)
	ROM_CONTINUE(0x5000,0x1000)

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "u16.7", 0x00000,  0x8000, CRC(19cc1d67) SHA1(47487f9362bfb36a32100ed772960628844462bf) )
	ROM_LOAD( "up11.6", 0x08000,  0x8000, CRC(c1466efa) SHA1(d725fc507c77e66bde93d0c33bf469add15f39b9) ) // changed title
	ROM_LOAD( "u4.5",  0x10000,  0x8000, CRC(e39fff9c) SHA1(22fdc517fa478441622c6245cecb5728c5595757) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "u15.4",  0x0000,  0x2000, CRC(8607ffd9) SHA1(9bc94715554aa2473ae2ed249a47f29c7886b3dc) )
	ROM_LOAD( "u10.3",  0x2000,  0x2000, CRC(c32367be) SHA1(ff217021b9c58e23b2226f8b0a7f5da966225715) )
	ROM_LOAD( "u14.2",  0x4000,  0x2000, CRC(6dfcb188) SHA1(22430429c798954d9d979e62699b58feae7fdbf4) )
	ROM_LOAD( "u9.1",   0x6000,  0x2000, CRC(9678ead2) SHA1(e80aefa98b2363fe9e6b2415762695ace272e4d3) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "u53.8",  0x0000, 0x10000, CRC(e92443d3) SHA1(4b6ca4521841610054165f085ae05510e77af191) )

	/* proms taken from cmv4, probably wrong  */
	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s129.u84", 0x0000, 0x0100, CRC(0489b760) SHA1(78f8632b17a76335183c5c204cdec856988368b0) )
	ROM_LOAD( "82s129.u70", 0x0100, 0x0100, CRC(21eb5b19) SHA1(9b8425bdb97f11f4855c998c7792c3291fd07470) )

	ROM_REGION( 0x100, "proms2", 0 )
	ROM_LOAD( "82s129.u46", 0x0000, 0x0100, CRC(50ec383b) SHA1(ae95b92bd3946b40134bcdc22708d5c6b0f4c23e) )
ROM_END

ROM_START( cmasterc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "msii841.u81",  0x3000,  0x1000, CRC(977db602) SHA1(0fd3d6781b654ac6befdc9278f84ca708d5d448c) )
	ROM_CONTINUE(0x2000,0x1000)
	ROM_CONTINUE(0x1000,0x1000)
	ROM_CONTINUE(0x0000,0x1000)
	ROM_CONTINUE(0x4000,0x1000)
	ROM_CONTINUE(0x5000,0x1000)
	ROM_CONTINUE(0x6000,0x1000)
	ROM_CONTINUE(0x7000,0x1000)

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "msii841.u16", 0x00000,  0x8000, CRC(19cc1d67) SHA1(47487f9362bfb36a32100ed772960628844462bf) )
	ROM_LOAD( "msii841.u11", 0x08000,  0x8000, CRC(63b3df4e) SHA1(9bacd23da598805ec18ec5ad15cab95d71eb9262) )
	ROM_LOAD( "msii841.u4",  0x10000,  0x8000, CRC(e39fff9c) SHA1(22fdc517fa478441622c6245cecb5728c5595757) )

	/* these gfx are in a different format to usual */
	ROM_REGION( 0x8000, "reels", 0 )
	ROM_LOAD( "msii841.u1",  0x0000,  0x4000, CRC(cf322ed2) SHA1(84df96229b7bdba0ab498e3bf9c77d7a7661f7b3) )
	ROM_LOAD( "msii841.u2",  0x4000,  0x4000, CRC(58c05653) SHA1(59454c07f4fe5b684d078cf97f2b1ee05b02f4ed) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "msii841.u53",  0x0000, 0x10000, CRC(e92443d3) SHA1(4b6ca4521841610054165f085ae05510e77af191) )

	/* proms taken from cmv4, probably wrong  */
	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s129.u84", 0x0000, 0x0100, CRC(0489b760) SHA1(78f8632b17a76335183c5c204cdec856988368b0) )
	ROM_LOAD( "82s129.u70", 0x0100, 0x0100, CRC(21eb5b19) SHA1(9b8425bdb97f11f4855c998c7792c3291fd07470) )

	ROM_REGION( 0x100, "proms2", 0 )
	ROM_LOAD( "82s129.u46", 0x0000, 0x0100, CRC(50ec383b) SHA1(ae95b92bd3946b40134bcdc22708d5c6b0f4c23e) )
ROM_END

/*

this set is weird, apart from the MGRAISE.BIN it's a cherry master set, I'm assuming that somehow
there is extra hardware which causes it to run a different game contained in that rom?

the extra rom contains

MEGA DOUBLE POKER TM        COPYRIGHT 1991

as well as various

COPYRIGHT (C) 1988-90-92 GERALD DUHAMEL

strings spread across the rom

------------------

Blitz Video Poker
1991

Hardware for a real Video Poker Machine that was supposed to pay out $$$.

Contains:

1 X Z80 CPU
2 X 6505s?
1 GI AY-3-8910
2 x 8255

CH3      BIN        32,768  11-15-98  8:46a CH3.BIN
MAST9    BIN        65,536  11-15-98  8:47a MAST9.BIN
MAST5    BIN        32,768  11-15-98  8:48a MAST5.BIN
MAST6    BIN        32,768  11-15-98  8:50a MAST6.BIN
MAST7    BIN        32,768  11-15-98  8:50a MAST7.BIN
MAST1    BIN         8,192  11-15-98  8:52a MAST1.BIN
MAST2    BIN         8,192  11-15-98  8:53a MAST2.BIN
MAST3    BIN         8,192  11-15-98  8:54a MAST3.BIN
MAST4    BIN         8,192  11-15-98  8:55a MAST4.BIN
MGRAISE  BIN       131,072  11-15-98  8:58a MGRAISE.BIN Sound Amp with 6502

*/

ROM_START( cmasterbv )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ch3.bin",  0x0000, 0x1000, CRC(9af51e47) SHA1(ac002c218502430a3e45259776ca409d32d2d4e5) )
	ROM_CONTINUE(0x4000,0x1000)
	ROM_CONTINUE(0x3000,0x1000)
	ROM_CONTINUE(0x7000,0x1000)
	ROM_CONTINUE(0x1000,0x1000)
	ROM_CONTINUE(0x6000,0x1000)
	ROM_CONTINUE(0x2000,0x1000)
	ROM_CONTINUE(0x5000,0x1000)

	ROM_REGION( 0x20000, "extra", 0 ) // how do we use this?!!
	ROM_LOAD( "mgraise.bin",  0x0000, 0x20000, CRC(019f37d4) SHA1(ab71fe0b41ff4415896a23f28b27a0e64950c68c) )


	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "mast5.bin",  0x00000, 0x8000, CRC(19cc1d67) SHA1(47487f9362bfb36a32100ed772960628844462bf) )
	ROM_LOAD( "mast6.bin",  0x08000, 0x8000, CRC(63b3df4e) SHA1(9bacd23da598805ec18ec5ad15cab95d71eb9262) )
	ROM_LOAD( "mast7.bin",   0x10000, 0x8000, CRC(e39fff9c) SHA1(22fdc517fa478441622c6245cecb5728c5595757) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "mast3.bin",  0x0000, 0x2000, CRC(8607ffd9) SHA1(9bc94715554aa2473ae2ed249a47f29c7886b3dc) )
	ROM_LOAD( "mast4.bin",  0x2000, 0x2000, CRC(c32367be) SHA1(ff217021b9c58e23b2226f8b0a7f5da966225715) )
	ROM_LOAD( "mast1.bin",  0x4000, 0x2000, CRC(6dfcb188) SHA1(22430429c798954d9d979e62699b58feae7fdbf4) )
	ROM_LOAD( "mast2.bin",  0x6000, 0x2000, CRC(9678ead2) SHA1(e80aefa98b2363fe9e6b2415762695ace272e4d3) )

	ROM_REGION( 0x10000, "user1", 0 ) // girl bitmaps
	ROM_LOAD( "mast9.bin",  0x0000, 0x10000, CRC(e92443d3) SHA1(4b6ca4521841610054165f085ae05510e77af191) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s129.u84", 0x0000, 0x0100, CRC(0489b760) SHA1(78f8632b17a76335183c5c204cdec856988368b0) )
	ROM_LOAD( "82s129.u70", 0x0100, 0x0100, CRC(21eb5b19) SHA1(9b8425bdb97f11f4855c998c7792c3291fd07470) )

	ROM_REGION( 0x100, "proms2", 0 )
	ROM_LOAD( "82s129.u46", 0x0000, 0x0100, CRC(50ec383b) SHA1(ae95b92bd3946b40134bcdc22708d5c6b0f4c23e) )
ROM_END

ROM_START( cmasterd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cherryb.rom",  0x0000, 0x1000, CRC(b6ab94f6) SHA1(6e74a2354d15aa1da6b8207e0413158d7cb52a44) )
	ROM_CONTINUE(0x4000,0x1000)
	ROM_CONTINUE(0x3000,0x1000)
	ROM_CONTINUE(0x7000,0x1000)
	ROM_CONTINUE(0x1000,0x1000)
	ROM_CONTINUE(0x6000,0x1000)
	ROM_CONTINUE(0x2000,0x1000)
	ROM_CONTINUE(0x5000,0x1000)

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "u16.7", 0x00000,  0x8000, CRC(19cc1d67) SHA1(47487f9362bfb36a32100ed772960628844462bf) )
	ROM_LOAD( "u11.6", 0x08000,  0x8000, CRC(63b3df4e) SHA1(9bacd23da598805ec18ec5ad15cab95d71eb9262) )
	ROM_LOAD( "u4.5",  0x10000,  0x8000, CRC(e39fff9c) SHA1(22fdc517fa478441622c6245cecb5728c5595757) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "u15.4",  0x0000,  0x2000, CRC(8607ffd9) SHA1(9bc94715554aa2473ae2ed249a47f29c7886b3dc) )
	ROM_LOAD( "u10.3",  0x2000,  0x2000, CRC(c32367be) SHA1(ff217021b9c58e23b2226f8b0a7f5da966225715) )
	ROM_LOAD( "u14.2",  0x4000,  0x2000, CRC(6dfcb188) SHA1(22430429c798954d9d979e62699b58feae7fdbf4) )
	ROM_LOAD( "u9.1",   0x6000,  0x2000, CRC(9678ead2) SHA1(e80aefa98b2363fe9e6b2415762695ace272e4d3) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "u53.8",  0x0000, 0x10000, CRC(e92443d3) SHA1(4b6ca4521841610054165f085ae05510e77af191) )

	/* proms taken from cmv4, probably wrong  */
	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s129.u84", 0x0000, 0x0100, CRC(0489b760) SHA1(78f8632b17a76335183c5c204cdec856988368b0) )
	ROM_LOAD( "82s129.u70", 0x0100, 0x0100, CRC(21eb5b19) SHA1(9b8425bdb97f11f4855c998c7792c3291fd07470) )

	ROM_REGION( 0x100, "proms2", 0 )
	ROM_LOAD( "82s129.u46", 0x0000, 0x0100, CRC(50ec383b) SHA1(ae95b92bd3946b40134bcdc22708d5c6b0f4c23e) )
ROM_END

ROM_START( cmastere )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cm33.rom",  0x0000, 0x1000, CRC(c3c3f7df) SHA1(47eda025859afebe64fd76e17e8390262fb40e0b) )
	ROM_CONTINUE(0x4000,0x1000)
	ROM_CONTINUE(0x3000,0x1000)
	ROM_CONTINUE(0x7000,0x1000)
	ROM_CONTINUE(0x1000,0x1000)
	ROM_CONTINUE(0x6000,0x1000)
	ROM_CONTINUE(0x2000,0x1000)
	ROM_CONTINUE(0x5000,0x1000)

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "u16.7", 0x00000,  0x8000, CRC(19cc1d67) SHA1(47487f9362bfb36a32100ed772960628844462bf) )
	ROM_LOAD( "u11.6", 0x08000,  0x8000, CRC(63b3df4e) SHA1(9bacd23da598805ec18ec5ad15cab95d71eb9262) )
	ROM_LOAD( "u4.5",  0x10000,  0x8000, CRC(e39fff9c) SHA1(22fdc517fa478441622c6245cecb5728c5595757) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "u15.4",  0x0000,  0x2000, CRC(8607ffd9) SHA1(9bc94715554aa2473ae2ed249a47f29c7886b3dc) )
	ROM_LOAD( "u10.3",  0x2000,  0x2000, CRC(c32367be) SHA1(ff217021b9c58e23b2226f8b0a7f5da966225715) )
	ROM_LOAD( "u14.2",  0x4000,  0x2000, CRC(6dfcb188) SHA1(22430429c798954d9d979e62699b58feae7fdbf4) )
	ROM_LOAD( "u9.1",   0x6000,  0x2000, CRC(9678ead2) SHA1(e80aefa98b2363fe9e6b2415762695ace272e4d3) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "u53.8",  0x0000, 0x10000, CRC(e92443d3) SHA1(4b6ca4521841610054165f085ae05510e77af191) )

	/* proms taken from cmv4, probably wrong  */
	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s129.u84", 0x0000, 0x0100, CRC(0489b760) SHA1(78f8632b17a76335183c5c204cdec856988368b0) )
	ROM_LOAD( "82s129.u70", 0x0100, 0x0100, CRC(21eb5b19) SHA1(9b8425bdb97f11f4855c998c7792c3291fd07470) )

	ROM_REGION( 0x100, "proms2", 0 )
	ROM_LOAD( "82s129.u46", 0x0000, 0x0100, CRC(50ec383b) SHA1(ae95b92bd3946b40134bcdc22708d5c6b0f4c23e) )
ROM_END

ROM_START( cmasterf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cmbig55.rom",  0x0000, 0x1000, CRC(2cc4df7b) SHA1(ad5b8108913ff88fb435c8c12b47446575e1360e) )
	ROM_CONTINUE(0x4000,0x1000)
	ROM_CONTINUE(0x3000,0x1000)
	ROM_CONTINUE(0x7000,0x1000)
	ROM_CONTINUE(0x1000,0x1000)
	ROM_CONTINUE(0x6000,0x1000)
	ROM_CONTINUE(0x2000,0x1000)
	ROM_CONTINUE(0x5000,0x1000)

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "u16.7", 0x00000,  0x8000, CRC(19cc1d67) SHA1(47487f9362bfb36a32100ed772960628844462bf) )
	ROM_LOAD( "u11.6", 0x08000,  0x8000, CRC(63b3df4e) SHA1(9bacd23da598805ec18ec5ad15cab95d71eb9262) )
	ROM_LOAD( "u4.5",  0x10000,  0x8000, CRC(e39fff9c) SHA1(22fdc517fa478441622c6245cecb5728c5595757) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "u15.4",  0x0000,  0x2000, CRC(8607ffd9) SHA1(9bc94715554aa2473ae2ed249a47f29c7886b3dc) )
	ROM_LOAD( "u10.3",  0x2000,  0x2000, CRC(c32367be) SHA1(ff217021b9c58e23b2226f8b0a7f5da966225715) )
	ROM_LOAD( "u14.2",  0x4000,  0x2000, CRC(6dfcb188) SHA1(22430429c798954d9d979e62699b58feae7fdbf4) )
	ROM_LOAD( "u9.1",   0x6000,  0x2000, CRC(9678ead2) SHA1(e80aefa98b2363fe9e6b2415762695ace272e4d3) )

	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "u53.8",  0x0000, 0x10000, CRC(e92443d3) SHA1(4b6ca4521841610054165f085ae05510e77af191) )

	/* proms taken from cmv4, probably wrong  */
	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s129.u84", 0x0000, 0x0100, CRC(0489b760) SHA1(78f8632b17a76335183c5c204cdec856988368b0) )
	ROM_LOAD( "82s129.u70", 0x0100, 0x0100, CRC(21eb5b19) SHA1(9b8425bdb97f11f4855c998c7792c3291fd07470) )

	ROM_REGION( 0x100, "proms2", 0 )
	ROM_LOAD( "82s129.u46", 0x0000, 0x0100, CRC(50ec383b) SHA1(ae95b92bd3946b40134bcdc22708d5c6b0f4c23e) )
ROM_END

// the program roms on these seem scrambled somehow, and there are no gfx roms dumped
ROM_START( jkrmast )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jokermaster.rom",  0x0000, 0x10000, CRC(73caf824) SHA1(b7a7bb6190465f7c3b40f2ef97f4f6beeb89ec41) )

	// unknown # of roms / sizes
	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "gfx_roms", 0x00000,  0x8000, NO_DUMP )
	ROM_REGION( 0x20000, "gfx2", ROMREGION_ERASE00 )

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASE00 )
	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "proms", 0x00000,  0x200, NO_DUMP )
	ROM_REGION( 0x100, "proms2", ROMREGION_ERASE00 )
ROM_END

ROM_START( pkrmast )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pokermastera.rom",  0x0000, 0x10000, CRC(467249f7) SHA1(efbab56896dc58d22ec921e7f5fd0befcfaadc52) )

	ROM_REGION( 0x20000, "gfx1", 0 ) // tiles
	ROM_LOAD( "103-2.bin", 0x00000,  0x20000, CRC(ed0dfbfe) SHA1(c3a5b68e821461b161293eaec1515e2b0f26c4f9) )

	ROM_REGION( 0x20000, "gfx2", 0 ) // reels + girl?
	ROM_LOAD( "103-1.bin", 0x00000,  0x20000, CRC(e375cd4b) SHA1(68888126ff9743cd589f3426205231bc3a896588) )

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASE00 )
	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "proms", 0x00000,  0x200, NO_DUMP )
	ROM_REGION( 0x100, "proms2", ROMREGION_ERASE00 )
ROM_END



ROM_START( pkrmasta )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pokermasterb.rom",  0x0000, 0x10000, CRC(f59e0273) SHA1(160426b86dbb8a718cb3b886f90a231baed86a40) )

	ROM_REGION( 0x20000, "gfx1", 0 ) // tiles
	ROM_LOAD( "103-2.bin", 0x00000,  0x20000, CRC(ed0dfbfe) SHA1(c3a5b68e821461b161293eaec1515e2b0f26c4f9) )

	ROM_REGION( 0x20000, "gfx2", 0 ) // reels + girl?
	ROM_LOAD( "103-1.bin", 0x00000,  0x20000, CRC(e375cd4b) SHA1(68888126ff9743cd589f3426205231bc3a896588) )

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASE00 )
	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "proms", 0x00000,  0x200, NO_DUMP )
	ROM_REGION( 0x100, "proms2", ROMREGION_ERASE00 )
ROM_END



/*

Cherry Master '91
-----------------

CPU: z8400
other chips: NEC D71055C, WB5300
Dips 5 x 8 position

OSC: 12.000mhz

all pals are type 16L8
all proms are type s129

*/

ROM_START( cmast91 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "8.bin",   0x00000, 0x01000, CRC(31a16d9f) SHA1(f007148449d66954b780f12a9f910968a4052482) )
	ROM_CONTINUE(0x1000,0x1000)
	ROM_CONTINUE(0x4000,0x1000)
	ROM_CONTINUE(0x5000,0x1000)
	ROM_CONTINUE(0x2000,0x1000)
	ROM_CONTINUE(0x3000,0x1000)
	ROM_CONTINUE(0x6000,0x1000)
	ROM_CONTINUE(0x7000,0x1000)
	ROM_CONTINUE(0x8000,0x1000)
	ROM_CONTINUE(0x9000,0x1000)
	ROM_CONTINUE(0xa000,0x1000)
	ROM_CONTINUE(0xb000,0x1000)
	ROM_CONTINUE(0xc000,0x1000)
	ROM_CONTINUE(0xd000,0x1000)
	ROM_CONTINUE(0xe000,0x1000)
	ROM_CONTINUE(0xf000,0x1000)

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "7.bin",	0x00000, 0x8000, CRC(1edf1f1d) SHA1(558fa01f1efd7f6541047d3930bdce0974bae5b0))
	ROM_LOAD( "6.bin",	0x08000, 0x8000, CRC(13582e74) SHA1(27e318542606b8e8d38250749ba996402d314abd) )
	ROM_LOAD( "5.bin",	0x10000, 0x8000, CRC(28ff88cc) SHA1(46bc0407be857e8348159735b60cfb660f047a56) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "4.bin",	0x00000, 0x8000, CRC(0dbabaa2) SHA1(44235b19dac1c996e2166672b03f6e3888ecbefa) )
	ROM_LOAD( "3.bin",	0x08000, 0x8000, CRC(dc77d04a) SHA1(d8656130cde54d4bb96307899f6d607867e49e6c) )
	ROM_LOAD( "1.bin",	0x10000, 0x8000, CRC(71bdab69) SHA1(d2c594ed88d6368df15b623c48eecc1c219b839e) )
	ROM_LOAD( "2.bin",	0x18000, 0x8000, CRC(fccd48d7) SHA1(af564f5ef9ff5b6363897ce6bdf0b21123911fd4) )

	ROM_REGION( 0x40000, "user1", 0 ) /* unknown, bitmaps, or sound? */
	ROM_LOAD( "9.bin",	0x00000, 0x40000, CRC(92342276) SHA1(f9436752f2ec67cf873fd01c729c7c113dc18be0) ) // ?

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "p1.bin", 0x0000, 0x0100, CRC(ac529f04) SHA1(5bc92e50c85bb23e609172cc15c430ddea7fdcb5) )
	ROM_LOAD( "p2.bin", 0x0100, 0x0100, CRC(3febce95) SHA1(c7c0fec0fb024ebf7d7365a09d28ba3d0037b0b4) )
	ROM_LOAD( "p3.bin", 0x0200, 0x0100, CRC(99dbdf19) SHA1(3680335406f63289f8d9a81b4cd163e4aa0c14d4) )

	ROM_REGION( 0x100, "proms2", 0 ) /* screen layout? */
	ROM_LOAD( "p4.bin", 0x0000, 0x0100, CRC(72212427) SHA1(e87a91f28284313c706ebb8175a3586780636e31) )

	ROM_REGION( 0x800, "plds", 0 ) /* all 16L8 type, protected */
	ROM_LOAD( "pld1.bin", 0x0000, 0x0104, NO_DUMP )
	ROM_LOAD( "pld2.bin", 0x0200, 0x0104, NO_DUMP )
	ROM_LOAD( "pld3.bin", 0x0400, 0x0104, NO_DUMP )
	ROM_LOAD( "pld4.bin", 0x0600, 0x0104, NO_DUMP )
ROM_END



ROM_START( cmast92 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cm9230d.rom",   0x00000, 0x01000, CRC(214a0a2d) SHA1(2d349e0888ac2da3df954517fdeb9214a3b17ae1) )
	// I've not checked the rom loading yet
	ROM_CONTINUE(0x1000,0x1000)
	ROM_CONTINUE(0x4000,0x1000)
	ROM_CONTINUE(0x5000,0x1000)
	ROM_CONTINUE(0x2000,0x1000)
	ROM_CONTINUE(0x3000,0x1000)
	ROM_CONTINUE(0x6000,0x1000)
	ROM_CONTINUE(0x7000,0x1000)
	ROM_CONTINUE(0x8000,0x1000)
	ROM_CONTINUE(0x9000,0x1000)
	ROM_CONTINUE(0xa000,0x1000)
	ROM_CONTINUE(0xb000,0x1000)
	ROM_CONTINUE(0xc000,0x1000)
	ROM_CONTINUE(0xd000,0x1000)
	ROM_CONTINUE(0xe000,0x1000)
	ROM_CONTINUE(0xf000,0x1000)

	// we only have a program rom :-(
	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "cherry master 92 graphics",	0x00000, 0x8000, NO_DUMP )
	ROM_REGION( 0x20000, "gfx2", ROMREGION_ERASEFF )
	ROM_REGION( 0x40000, "user1", ROMREGION_ERASEFF )
	ROM_REGION( 0x300, "proms", ROMREGION_ERASEFF )
	ROM_LOAD( "cherry master 92 proms",	0x00000, 0x100, NO_DUMP )
	ROM_REGION( 0x100, "proms2", ROMREGION_ERASEFF )
ROM_END

/*

        Lucky 8 Line
        Falcon 1989

        G14           6116  9
        G13   D13           8
              D12
        6116                 Z80
        6116                 8255
        7                    8255
        6            SW1     8255
 12MHz  5            SW2     8910
        4  6116      SW4
        3  6116      SW3
        2  6116
        1  6116

    ---

*/

ROM_START( lucky8 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "8",   0x0000, 0x4000, CRC(a187573e) SHA1(864627502025dbc83a0049fc98505655cec7b181) )
	ROM_LOAD( "9",   0x4000, 0x4000, CRC(6f62672e) SHA1(05662ef1a70f93b09e48de497b049a282f070735) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "5",  0x00000, 0x8000, CRC(59026af3) SHA1(3d7f7e78968ca26275635aeaa0e994468a3da575) )
	ROM_LOAD( "6",  0x08000, 0x8000, CRC(67a073c1) SHA1(36194d57d0dc0601fa1fdf2e6806f11b2ea6da36) )
	ROM_LOAD( "7",  0x10000, 0x8000, CRC(c415b9d0) SHA1(fd558fe8a116c33bbd712a639224d041447a45c1) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "1",   0x0000, 0x2000, CRC(29d6f197) SHA1(1542ca457594f6b7fe8f28f7d78023edd7021bc8) )
	ROM_LOAD( "2",   0x2000, 0x2000, CRC(5f812e65) SHA1(70d9ea82f9337936bf21f82b6961768d436f3a6f) )
	ROM_LOAD( "3",   0x4000, 0x2000, CRC(898b9ed5) SHA1(11b7d1cfcf425d00d086c74e0dbcb72068dda9fe) )
	ROM_LOAD( "4",   0x6000, 0x2000, CRC(4f7cfb35) SHA1(0617cf4419be00d9bacc78724089cb8af4104d68) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "d12", 0x0000, 0x0100, CRC(23e81049) SHA1(78071dae70fad870e972d944642fb3a2374be5e4) )
	/* missing prom? - using one from other dump */
	ROM_LOAD( "prom4", 0x0100, 0x0100, CRC(526cf9d3) SHA1(eb779d70f2507d0f26d225ac8f5de8f2243599ca) )

	ROM_REGION( 0x20, "proms2", 0 )
	ROM_LOAD( "d13", 0x0000, 0x0020, CRC(c6b41352) SHA1(d7c3b5aa32e4e456c9432a13bede1db6d62eb270) )

	ROM_REGION( 0x100, "unkprom", 0 )
	ROM_LOAD( "g14", 0x0000, 0x0100, CRC(bd48de71) SHA1(e4fa1e774af1499bc568be5b2deabb859d8c8172) )

	ROM_REGION( 0x20, "unkprom2", 0 )
	ROM_LOAD( "g13", 0x0000, 0x0020, CRC(6df3f972) SHA1(0096a7f7452b70cac6c0752cb62e24b643015b5c) )
ROM_END


/*

Magical Tonic

unknown, 40 pin cpu (plastic box, with "Tonic" sticker on it)
8255 x3
YM2203
12 MHz

4x DSW

*/

/* is this the original magical odds? */
static DRIVER_INIT(mtonic)
{
	int A;
	UINT8 *ROM = memory_region(machine, "maincpu");

	for (A = 0;A < 0x8000;A++)
	{
		if ((A & 4) == 4)
			ROM[A] ^= 0x01;

		ROM[A] = BITSWAP8(ROM[A], 3,6,5,4,7,2,1,0);
	}
}


ROM_START( mtonic )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "8.e6",	0x0000, 0x8000, CRC(01daf2af) SHA1(cb9b12c79dce3c9123510a49dffc9f3cee056cf6) )
	ROM_LOAD( "9.e6",	0x8000, 0x8000, CRC(1770ac79) SHA1(cadfd00ae75b90b1d202d741828e0afbd5ba0bec) )

	ROM_REGION( 0x30000, "gfx1", 0 )
	ROM_LOAD( "5.j10",	0x00000, 0x10000, CRC(b8032ea3) SHA1(15e5335a583d8e7a5422cd4d1d7342874a4962ab) )
	ROM_LOAD( "6.j11",	0x10000, 0x10000, CRC(ff38ff30) SHA1(8fef6e1fe7c307c69c9dcafa69ecf66467b9cb41) )
	ROM_LOAD( "7.j12",	0x20000, 0x10000, CRC(8f1d2db9) SHA1(200de01334905079dca542541e442d4194ecd913) )

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "1.l10",	0x0000, 0x4000, CRC(746588db) SHA1(2a0af552011246d4cc0cd0b670907cf8685ce8ef) )
	ROM_LOAD( "2.l11",	0x4000, 0x4000, CRC(8b7dd248) SHA1(a3ebde9fd0b6b1e42aa9b6d8e30c225abf2f80ce) )
	ROM_LOAD( "3.l12",	0x8000, 0x4000, CRC(de05e678) SHA1(8b9fcb9f912075a20a9ae38100006b57d508e0e7) )
	ROM_LOAD( "4.l13",	0xc000, 0x4000, CRC(8c542eee) SHA1(cb424e2a67c6d39302beca7cd5244bcad4a91189) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "prom1",	0x0000, 0x0100, NO_DUMP )
	ROM_LOAD( "prom2",	0x0100, 0x0100, NO_DUMP )

	ROM_REGION( 0x40, "proms2", 0 )
	ROM_LOAD( "prom3",	0x0000, 0x0020, NO_DUMP )
ROM_END


// is this a bootleg board?
ROM_START( magical )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "8_p6_d12.512", 0x00000, 0x08000, CRC(6978c662) SHA1(cfdbcdcd4085c264e1d0ad4f18160b40d2d4e406) )
	ROM_IGNORE(0x8000) // 1ST AND 2ND HALF IDENTICAL
	ROM_LOAD( "9_p7_d14.512", 0x0c000, 0x04000, CRC(095230ba) SHA1(e60f5497c2cd5f1c0fc33b1e21303dd569654e6d) )
	ROM_IGNORE(0xc000) // BADADDR        --xxxxxxxxxxxxxx

	ROM_REGION( 0x30000, "gfx1", 0 )
	ROM_LOAD( "5_l5_g2.512", 0x20000, 0x10000, CRC(f0ffb199) SHA1(383406e6ab74e8cecaf5574a879bf24aa606ac37) )
	ROM_LOAD( "6_l6_g3.512", 0x10000, 0x10000, CRC(3e44d92b) SHA1(4e00e26a4dbf326c0c919c40382505189e82c85b) )
	ROM_LOAD( "7_l7_g5.512", 0x00000, 0x10000, CRC(654bb754) SHA1(346bfbf85fc38797cf422da47d474e2ef2ef459c) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "1_m1_h2.764", 0x06000, 0x2000, CRC(d7d657e4) SHA1(53e2717edb4cdeac83cd519b08225784600afa61) )
	ROM_LOAD( "2_m2_h3.764", 0x02000, 0x2000, CRC(fb7541d5) SHA1(62a36c186b32bd98dff5c3f0fa5f2ad13992835c) )
	ROM_LOAD( "3_m3_h5.764", 0x04000, 0x2000, CRC(49572d8b) SHA1(ceaaf911f58cceec82f429ca9114bffa1a67ec12) )
	ROM_LOAD( "4_m4_h6.764", 0x00000, 0x2000, CRC(cccfaa5d) SHA1(69d9b8a26c769fd69093610e92918c9a086a2077) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "82s129a.3c", 0x000, 0x100, CRC(8c4c0dc8) SHA1(1b3ddbd253555efd1f48d469b42b272b9d96bacf) )
	ROM_LOAD( "82s129a.1c", 0x100, 0x100, CRC(55e3c65f) SHA1(f51d08e0b2e4d97d2eacb1f6d52777065bbe1ae5) )
	ROM_LOAD( "82s129a.2c", 0x200, 0x100, CRC(4d46f40a) SHA1(c141e94ae5705773605b7a094e65625b4a21db73) )

	ROM_REGION( 0x100, "proms2", 0 )
	ROM_LOAD( "82s129a.6j", 0x000, 0x100, CRC(1200815e) SHA1(8c9a856274246644af25961a2e731dbcb73f12b9) )

	ROM_REGION( 0x100, "proms3", 0 )
	ROM_LOAD( "dm74s288.1a", 0x00, 0x20, CRC(6a13320b) SHA1(6d7c663477f3fbc22fb716e15bfdd9c452eb686a) )
	ROM_LOAD( "dm74s288.1b", 0x20, 0x20, CRC(e04abac8) SHA1(4f2adf9f1482470b6de6d0e547623f62e95eaf24) )
	ROM_LOAD( "dm74s288.12k",0x40, 0x20, CRC(03231e84) SHA1(92abdf6f8ef705b260378e90e6d591da056c2cee) )
ROM_END

// is this a bootleg board?
// program is the same as above set (but without the oversized rom 9), only gfx1 differs
// the proms came from this board
ROM_START( magicala )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "8.12e", 0x00000, 0x08000, CRC(6978c662) SHA1(cfdbcdcd4085c264e1d0ad4f18160b40d2d4e406) )
	ROM_IGNORE(0x8000) // 1ST AND 2ND HALF IDENTICAL
	ROM_LOAD( "9.14e", 0x0c000, 0x04000, CRC(b3661c55) SHA1(f576d434ccec6f342455c18ada156d29634627cb) )

	ROM_REGION( 0x30000, "gfx1", 0 )
	ROM_LOAD( "5.2f", 0x20000, 0x10000, CRC(39e6ad6f) SHA1(0075015b3e794d82fb62984a41be48d17833c9f0) )
	ROM_LOAD( "6.3f", 0x10000, 0x10000, CRC(42e03002) SHA1(5c4f2a1aa91d3c0906665bec7423f8579f434dc1) )
	ROM_LOAD( "7.5f", 0x00000, 0x10000, CRC(5301a9a4) SHA1(f592bfc5f0d835cab6029aed7d6dcdd9d2ee1a2c) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "1_m1_h2.764", 0x06000, 0x2000, CRC(d7d657e4) SHA1(53e2717edb4cdeac83cd519b08225784600afa61) )
	ROM_LOAD( "2_m2_h3.764", 0x02000, 0x2000, CRC(fb7541d5) SHA1(62a36c186b32bd98dff5c3f0fa5f2ad13992835c) )
	ROM_LOAD( "3_m3_h5.764", 0x04000, 0x2000, CRC(49572d8b) SHA1(ceaaf911f58cceec82f429ca9114bffa1a67ec12) )
	ROM_LOAD( "4_m4_h6.764", 0x00000, 0x2000, CRC(cccfaa5d) SHA1(69d9b8a26c769fd69093610e92918c9a086a2077) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "82s129a.3c", 0x000, 0x100, CRC(8c4c0dc8) SHA1(1b3ddbd253555efd1f48d469b42b272b9d96bacf) )
	ROM_LOAD( "82s129a.1c", 0x100, 0x100, CRC(55e3c65f) SHA1(f51d08e0b2e4d97d2eacb1f6d52777065bbe1ae5) )
	ROM_LOAD( "82s129a.2c", 0x200, 0x100, CRC(4d46f40a) SHA1(c141e94ae5705773605b7a094e65625b4a21db73) )

	ROM_REGION( 0x100, "proms2", 0 )
	ROM_LOAD( "82s129a.6j", 0x000, 0x100, CRC(1200815e) SHA1(8c9a856274246644af25961a2e731dbcb73f12b9) )

	ROM_REGION( 0x100, "proms3", 0 )
	ROM_LOAD( "dm74s288.1a", 0x00, 0x20, CRC(6a13320b) SHA1(6d7c663477f3fbc22fb716e15bfdd9c452eb686a) )
	ROM_LOAD( "dm74s288.1b", 0x20, 0x20, CRC(e04abac8) SHA1(4f2adf9f1482470b6de6d0e547623f62e95eaf24) )
	ROM_LOAD( "dm74s288.12k",0x40, 0x20, CRC(03231e84) SHA1(92abdf6f8ef705b260378e90e6d591da056c2cee) )
ROM_END

/*

unknown korean or chinese bootleg of something?

XTAL 12MHz
Z80 @ 3MHz
AY3-8910 @ 1.5MHz
8255 x3
RAM 6116 x5
76489 x1
8-position DSW x4

----

13 and 13 files
g14                                             FIXED BITS (0000xxxx)
g14                                             BADADDR                xxxxx---
g13                                             FIXED BITS (1x1xxxxx11xxxxxx)
d13                                             FIXED BITS (xxxxxx0xxxxxxxxx)
d13                                             1ST AND 2ND HALF IDENTICAL
d12                                             FIXED BITS (0000xxxx)
                        prom1                   FIXED BITS (xxxxxx0xxxxxxxxx)
                        prom1                   1ST AND 2ND HALF IDENTICAL
                        prom2                   FIXED BITS (1x11xxxx11x1xxxx)
                        prom3                   FIXED BITS (0000xxxx)
                        prom4                   FIXED BITS (0000xxxx)
                        prom5                   FIXED BITS (00001xxx)
                        prom5                   BADADDR                xxxxxxx-
d13                     prom1                   IDENTICAL
d12                     prom3                   IDENTICAL
6                       7                       IDENTICAL
5                       6                       IDENTICAL
4                       5                       IDENTICAL
3                       4                       IDENTICAL
2                       3                       IDENTICAL
1                       2                       IDENTICAL
7                       8                       99.990845%
g13                     prom2                   90.625000%
g14                     prom4                   61.718750%
9                                               NO MATCH
8                                               NO MATCH
                        1                       NO MATCH
                        prom5                   NO MATCH

There is a loop at 0x0010 that decrement (HL) when is pointing to ROM space.
This should be worked out or patched to allow boot the game.
Seems to be related to timing since once patched the game is very fast.

*/
ROM_START( lucky8a )
	ROM_REGION( 0x8000, "maincpu", 0 )
	// we have to patch this, it might be bad
	ROM_LOAD( "1",  0x0000, 0x8000, BAD_DUMP CRC(554cddff) SHA1(8a0678993c7010f70adc9e9443b51cf5929bf110) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "6",  0x00000, 0x8000, CRC(59026af3) SHA1(3d7f7e78968ca26275635aeaa0e994468a3da575) )
	ROM_LOAD( "7",  0x08000, 0x8000, CRC(67a073c1) SHA1(36194d57d0dc0601fa1fdf2e6806f11b2ea6da36) )
	ROM_LOAD( "8",  0x10000, 0x8000, CRC(80b35f06) SHA1(561d257d7bc8976cfa08f36d84961f1263509b5b) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "2",   0x0000, 0x2000, CRC(29d6f197) SHA1(1542ca457594f6b7fe8f28f7d78023edd7021bc8) )
	ROM_LOAD( "3",   0x2000, 0x2000, CRC(5f812e65) SHA1(70d9ea82f9337936bf21f82b6961768d436f3a6f) )
	ROM_LOAD( "4",   0x4000, 0x2000, CRC(898b9ed5) SHA1(11b7d1cfcf425d00d086c74e0dbcb72068dda9fe) )
	ROM_LOAD( "5",   0x6000, 0x2000, CRC(4f7cfb35) SHA1(0617cf4419be00d9bacc78724089cb8af4104d68) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "prom3", 0x0000, 0x0100, CRC(23e81049) SHA1(78071dae70fad870e972d944642fb3a2374be5e4) )
	ROM_LOAD( "prom4", 0x0100, 0x0100, CRC(526cf9d3) SHA1(eb779d70f2507d0f26d225ac8f5de8f2243599ca) )

	ROM_REGION( 0x40, "proms2", 0 )
	ROM_LOAD( "prom1", 0x0000, 0x0020, CRC(c6b41352) SHA1(d7c3b5aa32e4e456c9432a13bede1db6d62eb270) )

	ROM_REGION( 0x100, "unkprom", 0 )
	ROM_LOAD( "prom5", 0x000, 0x0100, CRC(1d668d4a) SHA1(459117f78323ea264d3a29f1da2889bbabe9e4be) )

	ROM_REGION( 0x40, "unkprom2", 0 )
	ROM_LOAD( "prom2", 0x0000, 0x0020, CRC(7b1a769f) SHA1(788b3573df17d398c74662fec4fd7693fc27e2ef) )
ROM_END

/*
   New Lucky 8 Lines (set 3, extended gfx)

  This set has the New Lucky 8 Lines / New Super 8 Lines program.
  Same extended tileset for reels, but lacks of the New Super 8 Lines title tiles.
  Maybe is a hidden feature, maybe just graphics for another hack.

*/

ROM_START( lucky8b )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "8.bin",	0x0000, 0x8000, CRC(ab7c58f2) SHA1(74782772bcc91178fa381074ddca99e0515f7693) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "5.bin",	0x00000, 0x8000, CRC(9bbe5437) SHA1(ef3789cf34d83827bd6ad4755fd443c3d9bdf661) )
	ROM_LOAD( "6.bin",	0x08000, 0x8000, CRC(bc17a96b) SHA1(6ae6a99c72153d68b01feacc45d94f8f88ac8733) )
	ROM_LOAD( "7.bin",	0x10000, 0x8000, CRC(06a98714) SHA1(e58efdcbdc021976d5a1253c03bea0bfad4d92db) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "1.bin",	0x0000, 0x2000, CRC(b45f41e2) SHA1(890c94c802f5ada97bc73f5a7a09e69c3207966c) )
	ROM_LOAD( "2.bin",	0x2000, 0x2000, CRC(0463413a) SHA1(061b8335fdd44767e8c1832f5b5101276ad0f689) )
	ROM_LOAD( "3.bin",	0x4000, 0x2000, CRC(b4e58020) SHA1(5c0fcc4b5d484ca7de5f2bd568a391a45967a9cc) )
	ROM_LOAD( "4.bin",	0x6000, 0x2000, CRC(0a25964b) SHA1(d41eda201bb01229fb6e2ff437196dd65eebe577) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "u4.bin", 0x0000, 0x0100, CRC(23e81049) SHA1(78071dae70fad870e972d944642fb3a2374be5e4) )
	ROM_LOAD( "u5.bin", 0x0100, 0x0100, CRC(526cf9d3) SHA1(eb779d70f2507d0f26d225ac8f5de8f2243599ca) )

	ROM_REGION( 0x40, "proms2", 0 )
	ROM_LOAD( "u2.bin", 0x0000, 0x0020, CRC(c6b41352) SHA1(d7c3b5aa32e4e456c9432a13bede1db6d62eb270) )

	ROM_REGION( 0x100, "unkprom", 0 )
	ROM_LOAD( "u3.bin", 0x0000, 0x0100, CRC(1d668d4a) SHA1(459117f78323ea264d3a29f1da2889bbabe9e4be) )

	ROM_REGION( 0x20, "unkprom2", 0 )
	ROM_LOAD( "u1.bin", 0x0000, 0x0020, CRC(6df3f972) SHA1(0096a7f7452b70cac6c0752cb62e24b643015b5c) )
ROM_END

/*
  New Lucky 8 Lines / New Super 8 Lines.

  This set has a regular New Lucky 8 Lines, but allowing bets up to 64.
  It has different settings/parameters, and additional graphics for a game
  called New Super 8 Lines. There are basic reels tiles with a semi-naked woman,
  a sort of Super Mario character from Nintendo, clouds and stars...


 PCB Layout:
+----------------------------------------------------------------------------------------------------------------------------+
|      J       I       H         G            F         E         D           C          B              A                    |
|                                                                        +----------+                                        |
|   +-----+ +-----+ +-------+ +-----+                +-----+             |ASB IN USA| +--------------+ +-----+            +--+
|14 |  U  | |  U  | |   F   | |  T  |                |  M  |             |4FJ       | |      8       | |  R  |            |
|   +-----+ +-----+ +-------+ +-----+                +-----+             |HM6116LP-2| |   D27256     | +-----+            |
|                                                                        +----------+ |              |                    +--+
|   +-----+           +-----+ +-----+      +-----+   +-------+ +-----+   +-------+    +--------------+ +-----+             --|
|13 |  H  |           |  V  | |  T  |      |  H  |   |   F   | |  S  |   |   A   |                     |  M  |             --|
|   +-----+           +-----+ +-----+      +-----+   +-------+ +-----+   +-------+                     +-----+             --|
|                +----------+                                                                                              --|
|                |Toshiba   | +-----+                +-----+   +-----+   +-------+                                         --|
|12              |TMM2016BP-| |  O  |                |  B  |   |  S  |   |   A   |                                         --|
|                |12        | +-----+                +-----+   +-----+   +-------+                                         --|
|                +----------+                                                                                              --|
|                +----------+                                            +------------------+                              --|
|                |          | +-----+                +-----+   +-----+   |      ZILOG       |                              --|
|11              |HM6116L-90| |  O  |                |  B  |   |  L  |   |   Z0840004PSC    |                              --|
|                |          | +-----+                +-----+   +-----+   |   Z80 CPU        |                              --|
|                +----------+                                            +------------------+                              --|
|   +-----+  +--------------+ +-----+      +-----+   +-----+   +-----+   +------------------+                              --|
|10 |  J  |  |      [7]     | |  O  |      |  W  |   |  B  |   |  X  |   |    NEC JAPAN     |                              --|
|   +-----+  |    D27256    | +-----+      +-----+   +-----+   +-----+   |    D8255AC-2     |                              --|
|            |              |                                            |    9014XD010     |                              --|
|   +-----+  +--------------+ +-----+      +-----+   +-----+   +-----+   +------------------+                            36--|
|9  |  J  |  +--------------+ |  O  |      |  E  |   |  C  |   |  L  |                                                 Pinout|
|   +-----+  |      [6]     | +-----+      +-----+   +-----+   +-----+   +------------------+                              --|
|   +-----+  |    D27256    | +-------+    +-----+   +-----+ +--------+  |    NEC JAPAN     |                              --|
|8  |  J  |  |              | |   I   |    |  E  |   |  D  | |  DIP1  |  |    D8255AC-2     |                              --|
|   +-----+  +--------------+ +-------+    +-----+   +-----+ +--------+  |    9014XD010     |                              --|
|   +-----+  +--------------+ +-------+              +-----+ +--------+  +------------------+ +---+ +-----+                --|
|7  |  G  |  |      [5]     | |   I   |              |  E  | |  DIP2  |  +------------------+ | Q | |  P  |                --|
|   +-----+  |   D27256     | +-------+              +-----+ +--------+  |    NEC JAPAN     | +---+ +-----+                --|
|            |              |                                            |                  |                              --|
|    XTAL    +--------------+ +-------+              +-----+ +--------+  |                  |       +-----+                --|
|6  .----.   +--------------+ |   I   |              |  L  | |  DIP3  |  +------------------+       |  P  |                --|
|            |      [4]     | +-------+              +-----+ +--------+  +------------------+       +-----+                --|
|            |   D2764D     |                                            |     Winbond      |                              --|
|   +-----+  |              | +-------+    +-----+   +-----+ +--------+  |     WF19054      |                              --|
|5  |  J  |  +--------------+ |   I   |    |  K  |   |  B  | |  DIP4  |  |  4150C14090830   |                              --|
|   +-----+  +--------------+ +-------+    +-----+   +-----+ +--------+  +------------------+                              --|
|            |      [3]     | +----------+                                                                                 --|
|   +-----+  |  HN482764G   | |Toshiba Tm| +-----+   +-----+ +---------+  +------+ +------+                                --|
|4  |  J  |  |              | |m2016BP-12| |  K  |   |  B  | |    I    |  |    V | |   X  |                                --|
|   +-----+  +--------------+ +----------+ +-----+   +-----+ +---------+  +------+ +------+                               +--+
|   +-----+  +--------------+ +----------+ +-----+   +-----+ +-------+    +------+ +------+                               |
|3  |  J  |  |     [2]      | |USC 6516-A| |  T  |   |  B  | |   H   |    |    V | |   X  |                               |
|   +-----+  |    D2764     | |9252E GYU1| +-----+   +-----+ +-------+    +------+ +------+                               +--+
|            |              | +----------+                                                                                   |
|   +-----+  +--------------+ +----------+ +-----+   +-----+  +------+    +------+                                           |
|2  |  J  |  +--------------+ |Toshiba Tm| |  O  |   |  M  |  |   Y  |    |   Z  |                                           |
|   +-----+  |     [1]      | |m2016BP-12| +-----+   +-----+  +------+    +------+                                           |
|   +-----+  |  MBM2764-25  | +----------+ +-----+   +-----+  +------+                                                       |
|1  |  J  |  |              | +----------+ |  O  |   |  N  |  |   W  |                                                       |
|   +-----+  +--------------+ |HM6116L-90| +-----+   +-----+  +------+                                  A                    |
|                             |  9140A   |                                           +-----+    10 Pins   +-----+            |
|      J       I       H      +----------+    F         E         D           C     B|     ||||||||||||||||     |            |
+------------------------------------------------------------------------------------+     +--------------+     +------------+


DIP1:                     DIP2:                     DIP3:                     DIP4:
+-------------------+     +-------------------+     +-------------------+     +-------------------+
| ON                |     | ON                |     | ON                |     | ON                |
| +---------------+ |     | +---------------+ |     | +---------------+ |     | +---------------+ |
| |_|_|_|#|_|_|_|_| |     | |#|#|#|_|_|_|_|#| |     | |_|_|#|#|#|#|#|_| |     | |_|_|_|_|#|_|_|_| |
| |#|#|#| |#|#|#|#| |     | | | | |#|#|#|#| | |     | |#|#| | | | | |#| |     | |#|#|#|#| |#|#|#| |
| +---------------+ |     | +---------------+ |     | +---------------+ |     | +---------------+ |
|  1 2 3 4 5 6 7 8  |     |  1 2 3 4 5 6 7 8  |     |  1 2 3 4 5 6 7 8  |     |  1 2 3 4 5 6 7 8  |
+-------------------+     +-------------------+     +-------------------+     +-------------------+


1x XTAL = 12 Mhz


A = SN74LS244N / XXAC9307
B = GD74LS161A / A9417
C = MB74LS10 / 8507 M12
D = 74LSMPC / SLB1254
E = GD74LS74A / 9430
F = HD74LS273P
G = HD74LS368AP
H = 74LS32
I = 74LS245  / W994K9318 / Malaysia
J = GS 9429 / GD74LS166     ?????
K = SN74LS283N / KKFQ9149
L = GS 9427 / GD74LS138
M = GS 9424 / GD74LS04
N = Malaysia 9022AS / SN74LS139AN
O = GS 9425 / GD74LS157 ????
P = HD74LS04P
Q = 5560 / JRC / 3151A
R = HD74HC00P
S = DM74S288
T = DM74S287
U = LS02
V = LS174
W = LS08
X = LS367
Y = LS60 ??
Z = sn76489an

*/

ROM_START( ns8lines )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "8.bin",	0x0000, 0x8000, CRC(ab7c58f2) SHA1(74782772bcc91178fa381074ddca99e0515f7693) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "5.bin",	0x00000, 0x8000, CRC(994a9894) SHA1(4063c2c5e111f24a85df1665fd3f9fbb20fda4da) )
	ROM_LOAD( "6.bin",	0x08000, 0x8000, CRC(80888d64) SHA1(91ec96709df77c534d381e391839984a88aeb1e0) )
	ROM_LOAD( "7.bin",	0x10000, 0x8000, CRC(255d5860) SHA1(f171fde3d542594132b38b44300f750d45fb67a2) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "1.bin",	0x0000, 0x2000, CRC(b45f41e2) SHA1(890c94c802f5ada97bc73f5a7a09e69c3207966c) )
	ROM_LOAD( "2.bin",	0x2000, 0x2000, CRC(0463413a) SHA1(061b8335fdd44767e8c1832f5b5101276ad0f689) )
	ROM_LOAD( "3.bin",	0x4000, 0x2000, CRC(6be213c8) SHA1(bf5a002961b0827581cbab4249321ae5b51316f0) )
	ROM_LOAD( "4.bin",	0x6000, 0x2000, CRC(0a25964b) SHA1(d41eda201bb01229fb6e2ff437196dd65eebe577) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "u4.bin", 0x0000, 0x0100, CRC(23e81049) SHA1(78071dae70fad870e972d944642fb3a2374be5e4) )
	ROM_LOAD( "u5.bin", 0x0100, 0x0100, CRC(526cf9d3) SHA1(eb779d70f2507d0f26d225ac8f5de8f2243599ca) )

	ROM_REGION( 0x40, "proms2", 0 )
	ROM_LOAD( "u2.bin", 0x0000, 0x0020, CRC(c6b41352) SHA1(d7c3b5aa32e4e456c9432a13bede1db6d62eb270) )

	ROM_REGION( 0x100, "unkprom", 0 )
	ROM_LOAD( "u3.bin", 0x0000, 0x0100, CRC(1d668d4a) SHA1(459117f78323ea264d3a29f1da2889bbabe9e4be) )

	ROM_REGION( 0x20, "unkprom2", 0 )
	ROM_LOAD( "u1.bin", 0x0000, 0x0020, CRC(6df3f972) SHA1(0096a7f7452b70cac6c0752cb62e24b643015b5c) )
ROM_END

/*

  New Lucky 8 Lines / New Super 8 Lines (Witch Bonus)

  This set has the 'Witch Bonus' present in Witch Card games.

  1.1h         [1/2]      1    [1/2]      IDENTICAL
  2.3h         [1/2]      2    [1/2]      IDENTICAL
  3.4h         [1/2]      3    [1/2]      IDENTICAL
  4.5h         [1/2]      4    [1/2]      IDENTICAL
  5.7h         [1/4]      5    [1/4]      IDENTICAL
  6.8h         [1/4]      6    [1/4]      IDENTICAL
  7.10h        [1/4]      7    [1/4]      IDENTICAL
  7.10h        [3/4]      7    [3/4]      IDENTICAL
  7.10h        [2/4]      7    [2/4]      99.426270%
  6.8h         [2/4]      6    [2/4]      99.255371%
  5.7h         [2/4]      5    [2/4]      99.230957%
  5.7h         [3/4]      5    [3/4]      94.152832%
  6.8h         [3/4]      6    [3/4]      94.152832%
  7.10h        [4/4]      7    [4/4]      82.739258%
  7.10h        [4/4]      6    [4/4]      82.739258%
  7.10h        [4/4]      5    [4/4]      82.739258%
  4.5h         [2/2]      4    [2/2]      81.372070%
  6.8h         [4/4]      9    [2/2]      75.952148%
  3.4h         [2/2]      3    [2/2]      71.752930%
  2.3h         [2/2]      2    [2/2]      70.434570%
  1.1h         [2/2]      1    [2/2]      68.115234%
  f5-8.14b     [4/4]      8    [1/2]      8.276367%
  5.7h         [4/4]      9    [1/2]      7.104492%
  f5-8.14b     [3/4]      8    [2/2]      1.904297%

*/

ROM_START( ns8linew )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "f5-8.14b",   0x0000, 0x8000, CRC(63dd3005) SHA1(62d71dbfa0a00c6b050db067ad55e80225e1589d) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "5.7h",  0x00000, 0x8000, CRC(19713d36) SHA1(ae04b8b72b0c9a279f24d7c4d619bac4629d9a4f) )
	ROM_LOAD( "6.8h",  0x08000, 0x8000, CRC(576197b9) SHA1(22273365cfe181f95efb895a28825f388b901a49) )
	ROM_LOAD( "7.10h", 0x10000, 0x8000, CRC(790c349c) SHA1(32bec8463233b2eb5a889c91d35f53b9d117f279) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "1.1h",  0x0000, 0x2000, CRC(b45f41e2) SHA1(890c94c802f5ada97bc73f5a7a09e69c3207966c) )
	ROM_LOAD( "2.3h",  0x2000, 0x2000, CRC(0463413a) SHA1(061b8335fdd44767e8c1832f5b5101276ad0f689) )
	ROM_LOAD( "3.4h",  0x4000, 0x2000, CRC(b4e58020) SHA1(5c0fcc4b5d484ca7de5f2bd568a391a45967a9cc) )
	ROM_LOAD( "4.5h",  0x6000, 0x2000, CRC(0a25964b) SHA1(d41eda201bb01229fb6e2ff437196dd65eebe577) )

	ROM_REGION( 0x200, "proms", 0 )	/* proper dumps */
	ROM_LOAD( "dm74s287.g13", 0x0000, 0x0100, CRC(23e81049) SHA1(78071dae70fad870e972d944642fb3a2374be5e4) )
	ROM_LOAD( "dm74s287.g14", 0x0100, 0x0100, CRC(526cf9d3) SHA1(eb779d70f2507d0f26d225ac8f5de8f2243599ca) )

	ROM_REGION( 0x20, "proms2", 0 )
	ROM_LOAD( "dm74s288.d13", 0x0000, 0x0020, CRC(c6b41352) SHA1(d7c3b5aa32e4e456c9432a13bede1db6d62eb270) )

	ROM_REGION( 0x100, "unkprom", 0 )
	ROM_LOAD( "dm74s287.f3",  0x0000, 0x0100, CRC(1d668d4a) SHA1(459117f78323ea264d3a29f1da2889bbabe9e4be) )

	ROM_REGION( 0x20, "unkprom2", 0 )
	ROM_LOAD( "dm74s288.d12", 0x0000, 0x0020, CRC(6df3f972) SHA1(0096a7f7452b70cac6c0752cb62e24b643015b5c) )
ROM_END

/*
    LADY LINER - TAB Austria

    Hardware Notes:
    ---------------

    CPU:   1x Z80.
    Sound: 1x AY8930.
    I/O:   2x P8255A.

    Clock: 1x Xtal @ 12.0000MHz.

    ROMs:  1x NM27C256Q (ladybrd)
           7x M27C64A (1,2,3,4,71,72,73)
           2x PROM AM27S19PC (39,73)
           3x PROM AM27S21PC (37,38,96)

    RAM:   4x HY6116ALP-10 (near graphics ROMs)
           1x HY6116ALP-10 (near program ROM)

    1x 8 DIP Switches.
    1x trimmer (volume).

    Connectors:  1x 18x2 edge connector.
                 1x 22x2 edge connector.

    Both connectors are wired to a strange small PCB with:
    1x 6x2 edge connector + 1x 60x2 edge connector (with smaller spacing) + 2x 8 DIP Switches.

    Silkscreened on PCB:
    "TAB AUSTRIA"

    Sticker on PCB:
    "TAB Austria" & "LL 2690"

*/

ROM_START( ladylinr )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "ladybrd.bin",	0x0000, 0x8000, CRC(44d2aed0) SHA1(1afe6178d1bf4ad0b623f33be879ed5180ad2db1) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "ll73.bin",	0x00000, 0x8000, CRC(afa4a705) SHA1(779340713df7029553cfc1c57997dfdd96a0f0cc) )
	ROM_LOAD( "ll72.bin",	0x08000, 0x8000, CRC(bd1d8a39) SHA1(01e37704c753352024e79b0b83b040f8288b9aed) )
	ROM_LOAD( "ll71.bin",	0x10000, 0x8000, CRC(1c417efa) SHA1(491579a76d80c4f488ef94393d12a190571ae285) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "1.bin",	0x0000, 0x2000, CRC(29d6f197) SHA1(1542ca457594f6b7fe8f28f7d78023edd7021bc8) )
	ROM_LOAD( "2.bin",	0x2000, 0x2000, CRC(5f812e65) SHA1(70d9ea82f9337936bf21f82b6961768d436f3a6f) )
	ROM_LOAD( "3.bin",	0x4000, 0x2000, CRC(898b9ed5) SHA1(11b7d1cfcf425d00d086c74e0dbcb72068dda9fe) )
	ROM_LOAD( "4.bin",	0x6000, 0x2000, CRC(4f7cfb35) SHA1(0617cf4419be00d9bacc78724089cb8af4104d68) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "am27s21pc.38",	0x0000, 0x0100, CRC(06a0ee6f) SHA1(e793fbb9e14e4e6c6d6783a36edee74f28e7e214) )
	ROM_LOAD( "am27s21pc.37",	0x0100, 0x0100, CRC(8589d23c) SHA1(9629c0d8af3cce47ef376898a4be84c0752a265b) )

	ROM_REGION( 0x40, "proms2", 0 )
	ROM_LOAD( "am27s19pc.39",	0x0000, 0x0020, CRC(c6b41352) SHA1(d7c3b5aa32e4e456c9432a13bede1db6d62eb270) )

	ROM_REGION( 0x100, "unkprom", 0 )
	ROM_LOAD( "am27s21pc.96",	0x0000, 0x0100, CRC(1d668d4a) SHA1(459117f78323ea264d3a29f1da2889bbabe9e4be) )

	ROM_REGION( 0x40, "unkprom2", 0 )
	ROM_LOAD( "am27s19pc.73",	0x0000, 0x0020, CRC(b48d0b41) SHA1(01d2d0fd5e79c17043e97146001150b4b32ac86c) )
ROM_END

/*

  Board had a sticker that said MODEL 9006

  .u22  2764
  .u21  2764
  .u20  2764
  .u19  2764
  .u18  27256
  .u17  27256
  .u16  27256
  .u66  27256

  HM6116P
  D4016C-2 x3
  D4016C-5
  12Mhz Crystal
  D780C
  UM82C55A-PC
  M5L8255AP-5
  D8255AC-2

*/
ROM_START( kkojnoli )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "9006.u66",	0x0000, 0x8000, CRC(5807a005) SHA1(9c7156656cd651c7785c42ce25e96aadd8e3d9ff) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "9006.u18",	0x00000, 0x8000, CRC(b83e304f) SHA1(1022518928d8cb094340927a65b8fb72b0f3c640) )
	ROM_LOAD( "9006.u17",	0x08000, 0x8000, CRC(13a3258b) SHA1(605555ae48012ca4b1829d1c835b33ddeec17da6) )
	ROM_LOAD( "9006.u16",	0x10000, 0x8000, CRC(84d09be4) SHA1(09a9e491a9a5fc7882c88d46ae2a6e7e99d082f4) )

	ROM_REGION( 0x8000, "gfx2", 0 ) /* redumped */
	ROM_LOAD( "9006.u22",	0x0000, 0x2000, CRC(4e93130d) SHA1(faaaf51844da8d3bdb908fb8ce0f2442e26b5f62) )
	ROM_LOAD( "9006.u20",	0x2000, 0x2000, CRC(717fe736) SHA1(04e578c1992bbdb312bb6bc12137bd96522a50e6) )
	ROM_LOAD( "9006.u21",	0x4000, 0x2000, CRC(f5314f3f) SHA1(0423dc545fce0322377f1934894a999427709b33) )
	ROM_LOAD( "9006.u19",	0x6000, 0x2000, CRC(c321d50b) SHA1(8c132d8fcc812bcec5966c8a3960dfbe5d9f8c36) )

	/* proper proms recent dumped */
	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "9006.u24", 0x0000, 0x0100, CRC(23e81049) SHA1(78071dae70fad870e972d944642fb3a2374be5e4) )
	ROM_LOAD( "9006.u23", 0x0100, 0x0100, CRC(526cf9d3) SHA1(eb779d70f2507d0f26d225ac8f5de8f2243599ca) )

	/* this one seems bitrotten (bits 3 and 7), except for this issue,
       the prom have concordance with prom1 from lucky8 */
	ROM_REGION( 0x40, "proms2", 0 )
	ROM_LOAD( "prom1", 0x0000, 0x0020, CRC(c6b41352) SHA1(d7c3b5aa32e4e456c9432a13bede1db6d62eb270) )
//  ROM_LOAD( "9006.u57", 0x0000, 0x0020, CRC(8a37416a) SHA1(696b46db2ff2bb9ef471ff925977e8a186b17de8) )

	ROM_REGION( 0x100, "unkprom", 0 )
	ROM_LOAD( "9006.u41", 0x0000, 0x0100, CRC(1d668d4a) SHA1(459117f78323ea264d3a29f1da2889bbabe9e4be) )

	ROM_REGION( 0x20, "unkprom2", 0 )
	ROM_LOAD( "9006.u58", 0x0000, 0x0020, CRC(6df3f972) SHA1(0096a7f7452b70cac6c0752cb62e24b643015b5c) )
ROM_END

/*

Wild cat 3 by E.A.I.

Cherry master type game

.h1  2764   handwritten 1
.h3  2764   handwritten 2
.h4  2764   handwritten 3
.h5  2764   handwritten 4
.h7  27256  handwritten 5
.h8  27256  handwritten 6
.h10 27256  stickered 7 E.A.I.
.g13 82s129 stickered G13
.g14 82s129 stickered G14
.d12 82s123 handwritten 2
.d13 82s123 stickered D13
.f3  82s129 handwritten 3

open 24 pin socket @ B13

Daughter board
.u5  27512

Z80 on daughter board
SN76489AN
6116 x4
12.000 MHz crystal
8255 x3
Winbound WF19054 40 pin dip

*/

ROM_START( wcat3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wcat3.u5",	0x0000, 0x10000, CRC(bf21cde5) SHA1(b501ba8ea815e3b19b26196f6fd48243892278eb) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "wcat3.h7",	0x10000, 0x8000, CRC(065cb575) SHA1(4dd49773c4caeaa489342e61f26c8eaaae876edc) )
	ROM_LOAD( "wcat3.h8",	0x08000, 0x8000, CRC(60463213) SHA1(b0937b4a55f74831ce9a06f3df0af504845f908d) )
	ROM_LOAD( "wcat3.h10",	0x00000, 0x8000, CRC(dda38c26) SHA1(4b9292911133dd6067a1c61a44845e824e88a52d) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "wcat3.h1",	0x6000, 0x2000, CRC(0509d556) SHA1(c2f46d279f45b544c67b0c966659cc6d5d53c22f) )
	ROM_LOAD( "wcat3.h2",	0x4000, 0x2000, CRC(d50f3d62) SHA1(8500c7f3a2f51ea0ed7e142ecdc4e669ba3e7065) )
	ROM_LOAD( "wcat3.h4",	0x2000, 0x2000, CRC(373d9949) SHA1(ff483505fb9e86411acad7059bf5434dde290946) )
	ROM_LOAD( "wcat3.h5",	0x0000, 0x2000, CRC(50febe3b) SHA1(0479bcee53b174aa0413951e283e446b09a6f156) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "wcat3.g13",	0x0000, 0x0100, CRC(c29a36f2) SHA1(936b07a195f6e7f6a884bd35f442003cf67aa447) )
	ROM_LOAD( "wcat3.g14",	0x0100, 0x0100, CRC(dcd53d2c) SHA1(bbcb4266117c3cd1c8ef0e5046d3558c8293313a) )

	ROM_REGION( 0x40, "proms2", 0 )
	ROM_LOAD( "wcat3.d12",	0x0000, 0x0020, CRC(6df3f972) SHA1(0096a7f7452b70cac6c0752cb62e24b643015b5c) )

	ROM_REGION( 0x100, "unkprom", 0 )
	ROM_LOAD( "wcat3.f3",	0x0000, 0x0100, CRC(1d668d4a) SHA1(459117f78323ea264d3a29f1da2889bbabe9e4be) )

	ROM_REGION( 0x40, "unkprom2", 0 )
	ROM_LOAD( "wcat3.d13",	0x0000, 0x0020, CRC(eab832ed) SHA1(0fbc8914ba1805cfc6698fe7f137a934e63a4f89) )
ROM_END


/* these 'Amcoe' games look like bootlegs of cherry master
  the z80 roms are encrypted */
ROM_START( skill98 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sk98133.bin", 0x0000, 0x1000, CRC(77a5dd54) SHA1(e693f477b42b83f1f5e45fb7c56486119bf91856) )
	ROM_CONTINUE(0x4000,0x1000)
	ROM_CONTINUE(0x3000,0x1000)
	ROM_CONTINUE(0x7000,0x1000)
	ROM_CONTINUE(0x1000,0x1000)
	ROM_CONTINUE(0x6000,0x1000)
	ROM_CONTINUE(0x2000,0x1000)
	ROM_CONTINUE(0x5000,0x1000)
	ROM_CONTINUE(0x8000,0x1000)
	ROM_CONTINUE(0x9000,0x1000)
	ROM_CONTINUE(0xa000,0x1000)
	ROM_CONTINUE(0xb000,0x1000)
	ROM_CONTINUE(0xc000,0x1000)
	ROM_CONTINUE(0xd000,0x1000)
	ROM_CONTINUE(0xe000,0x1000)
	ROM_CONTINUE(0xf000,0x1000)

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )

	ROM_REGION( 0x20000, "graphics", 0 )
	ROM_LOAD( "sk98h.bin",  0x00000, 0x10000, CRC(0574357b) SHA1(96a846f6d49dd67ad078ad9240e632f79ae1b437) )
	ROM_LOAD( "sk98l.bin",  0x10000, 0x10000, CRC(ebe802a4) SHA1(178542c204fd1027874e6d2c099edaa7878c993f) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_COPY( "graphics", 0x18000, 0x00000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x08000, 0x08000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x04000, 0x10000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x1c000, 0x04000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x0c000, 0x0c000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x14000, 0x14000, 0x4000 ) // 2

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "graphics", 0x02000, 0x00000, 0x2000 )
	ROM_COPY( "graphics", 0x12000, 0x02000, 0x2000 )
	ROM_COPY( "graphics", 0x00000, 0x04000, 0x2000 )
	ROM_COPY( "graphics", 0x10000, 0x06000, 0x2000 )

	ROM_REGION( 0x200, "proms", 0 ) // palette
	ROM_LOAD( "sk98u19.bin", 0x0000, 0x0100, CRC(796c7960) SHA1(0f64a8119fd4809a5ac79585b415b34b2a83e9dc) )
	ROM_LOAD( "sk98u20.bin", 0x0100, 0x0100, CRC(a0862663) SHA1(e27c3bba5f87b51a19ea8068f2ce7b82a6f0eedb) )

	ROM_REGION( 0x100, "sku1920.bin", 0 ) // colours again?
	ROM_LOAD( "sku1920.bin", 0x0000, 0x0100, CRC(a8c86d5e) SHA1(d19cd5e57ac8fdd685540c1bb2e1474d1326362b) )

	ROM_REGION( 0x80000, "oki", 0 ) // samples
	ROM_LOAD( "sk98t.bin", 0x00000, 0x20000, CRC(8598b059) SHA1(9e031e30e58a9c1b3d029004ee0f1616711fa2ae) )
ROM_END



ROM_START( schery97 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "sc352.bin", 0x00000, 0x1000, CRC(d3857d85) SHA1(e97b2634f0993631023c08f6baf800461abfad12) )
	ROM_CONTINUE(0x4000,0x1000)
	ROM_CONTINUE(0x3000,0x1000)
	ROM_CONTINUE(0x7000,0x1000)
	ROM_CONTINUE(0x1000,0x1000)
	ROM_CONTINUE(0x6000,0x1000)
	ROM_CONTINUE(0x2000,0x1000)
	ROM_CONTINUE(0x5000,0x1000)
	ROM_CONTINUE(0x8000,0x1000)
	ROM_CONTINUE(0x9000,0x1000)
	ROM_CONTINUE(0xa000,0x1000)
	ROM_CONTINUE(0xb000,0x1000)
	ROM_CONTINUE(0xc000,0x1000)
	ROM_CONTINUE(0xd000,0x1000)
	ROM_CONTINUE(0xe000,0x1000)
	ROM_CONTINUE(0xf000,0x1000)

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )

	ROM_REGION( 0x20000, "graphics", 0 )
	ROM_LOAD( "sc97h.bin",  0x00000, 0x10000, CRC(def39ee2) SHA1(5e6817bd947ebf16d0313285a00876b796b71cab) )
	ROM_LOAD( "sc97l.bin",  0x10000, 0x10000, CRC(6f4d6aea) SHA1(6809c26e6975cac97b0f8c01a508d4e022859b1a) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_COPY( "graphics", 0x18000, 0x00000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x08000, 0x08000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x04000, 0x10000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x1c000, 0x04000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x0c000, 0x0c000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x14000, 0x14000, 0x4000 ) // 2

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "graphics", 0x02000, 0x00000, 0x2000 )
	ROM_COPY( "graphics", 0x12000, 0x02000, 0x2000 )
	ROM_COPY( "graphics", 0x00000, 0x04000, 0x2000 )
	ROM_COPY( "graphics", 0x10000, 0x06000, 0x2000 )

	ROM_REGION( 0x200, "proms", 0 ) // palette
	ROM_LOAD( "sc97u19.bin", 0x0000, 0x0100, CRC(6a01caca) SHA1(9b3e9eebb9fcc8770f7e92f0f1c0434516ee613d) )
	ROM_LOAD( "sc97u20.bin", 0x0100, 0x0100, CRC(5899c1d5) SHA1(c335b99bb58da3a11005a8952a15d9f43bdff157) )

	ROM_REGION( 0x100, "proms2", 0 ) // colours again?
	ROM_LOAD( "scu1920.bin", 0x0000, 0x0100, CRC(3aa291dd) SHA1(f35c916b5463ff9ec6e57048af29a746148a13af) )

	ROM_REGION( 0x80000, "oki", 0 ) // samples
	ROM_LOAD( "sc97t.bin", 0x00000, 0x20000, CRC(8598b059) SHA1(9e031e30e58a9c1b3d029004ee0f1616711fa2ae) )
ROM_END

ROM_START( schery97a )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "sc352c4.bin", 0x00000, 0x1000, CRC(44f55f6e) SHA1(8b6e8618281de480979de37c7b36a0e68a524f47) ) // ?? alt program?
	ROM_CONTINUE(0x4000,0x1000)
	ROM_CONTINUE(0x3000,0x1000)
	ROM_CONTINUE(0x7000,0x1000)
	ROM_CONTINUE(0x1000,0x1000)
	ROM_CONTINUE(0x6000,0x1000)
	ROM_CONTINUE(0x2000,0x1000)
	ROM_CONTINUE(0x5000,0x1000)
	ROM_CONTINUE(0x8000,0x1000)
	ROM_CONTINUE(0x9000,0x1000)
	ROM_CONTINUE(0xa000,0x1000)
	ROM_CONTINUE(0xb000,0x1000)
	ROM_CONTINUE(0xc000,0x1000)
	ROM_CONTINUE(0xd000,0x1000)
	ROM_CONTINUE(0xe000,0x1000)
	ROM_CONTINUE(0xf000,0x1000)

	ROM_REGION( 0x20000, "graphics", 0 )
	ROM_LOAD( "sc97h.bin",  0x00000, 0x10000, CRC(def39ee2) SHA1(5e6817bd947ebf16d0313285a00876b796b71cab) )
	ROM_LOAD( "sc97l.bin",  0x10000, 0x10000, CRC(6f4d6aea) SHA1(6809c26e6975cac97b0f8c01a508d4e022859b1a) )

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_COPY( "graphics", 0x18000, 0x00000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x08000, 0x08000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x04000, 0x10000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x1c000, 0x04000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x0c000, 0x0c000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x14000, 0x14000, 0x4000 ) // 2

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "graphics", 0x02000, 0x00000, 0x2000 )
	ROM_COPY( "graphics", 0x12000, 0x02000, 0x2000 )
	ROM_COPY( "graphics", 0x00000, 0x04000, 0x2000 )
	ROM_COPY( "graphics", 0x10000, 0x06000, 0x2000 )

	ROM_REGION( 0x200, "proms", 0 ) // palette
	ROM_LOAD( "sc97u19.bin", 0x0000, 0x0100, CRC(6a01caca) SHA1(9b3e9eebb9fcc8770f7e92f0f1c0434516ee613d) )
	ROM_LOAD( "sc97u20.bin", 0x0100, 0x0100, CRC(5899c1d5) SHA1(c335b99bb58da3a11005a8952a15d9f43bdff157) )

	ROM_REGION( 0x100, "sku1920.bin", 0 ) // colours again?
	ROM_LOAD( "scu1920.bin", 0x0000, 0x0100, CRC(3aa291dd) SHA1(f35c916b5463ff9ec6e57048af29a746148a13af) )

	ROM_REGION( 0x80000, "oki", 0 ) // samples
	ROM_LOAD( "sc97t.bin", 0x00000, 0x20000, CRC(8598b059) SHA1(9e031e30e58a9c1b3d029004ee0f1616711fa2ae) )
ROM_END

ROM_START( roypok96 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rp35.bin", 0x0000, 0x1000, CRC(e1509440) SHA1(30d931b02d4eb74f9a16c57eb12e834cf24f87a9) )
	ROM_CONTINUE(0x4000,0x1000)
	ROM_CONTINUE(0x3000,0x1000)
	ROM_CONTINUE(0x7000,0x1000)
	ROM_CONTINUE(0x1000,0x1000)
	ROM_CONTINUE(0x6000,0x1000)
	ROM_CONTINUE(0x2000,0x1000)
	ROM_CONTINUE(0x5000,0x1000)
	ROM_CONTINUE(0x8000,0x8000)

	ROM_REGION( 0x20000, "graphics", 0 )
	ROM_LOAD( "rp35h.bin",  0x00000, 0x10000, CRC(664649ea) SHA1(7915ab31afd2a1bbb8f817f961e0e522d76f5c05) )
	ROM_LOAD( "rp35l.bin",  0x10000, 0x10000, CRC(ef416c4e) SHA1(5aac157ba15c66f79a7a68935095bef9a2636f7b) )

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_COPY( "graphics", 0x18000, 0x00000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x08000, 0x08000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x04000, 0x10000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x1c000, 0x04000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x0c000, 0x0c000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x14000, 0x14000, 0x4000 ) // 2

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "graphics", 0x02000, 0x00000, 0x2000 )
	ROM_COPY( "graphics", 0x12000, 0x02000, 0x2000 )
	ROM_COPY( "graphics", 0x00000, 0x04000, 0x2000 )
	ROM_COPY( "graphics", 0x10000, 0x06000, 0x2000 )

	ROM_REGION( 0x200, "proms", 0 ) // palette
	ROM_LOAD( "rpu19.bin", 0x0000, 0x0100, CRC(deb9ae3c) SHA1(056ce4947244ade1ff70f167a998140745b5cffa) )
	ROM_LOAD( "rpu20.bin", 0x0100, 0x0100, CRC(b3e0a328) SHA1(f8990fcd1e90d3e9205ee81f1d7dd105dbdcfcd6) )

	ROM_REGION( 0x100, "proms2", 0 ) // colours again?
	ROM_LOAD( "rpu1920.bin", 0x0000, 0x0100, CRC(e204e8f3) SHA1(9005fe9c72055af690701cd239f4b3665b2fae21) )
ROM_END

ROM_START( roypok96a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rp36.bin", 0x0000, 0x1000, CRC(7fffff21) SHA1(85533e6aa0c6810cdaed9a6d1f1313f7bc871cbd) )
	ROM_CONTINUE(0x4000,0x1000)
	ROM_CONTINUE(0x3000,0x1000)
	ROM_CONTINUE(0x7000,0x1000)
	ROM_CONTINUE(0x1000,0x1000)
	ROM_CONTINUE(0x6000,0x1000)
	ROM_CONTINUE(0x2000,0x1000)
	ROM_CONTINUE(0x5000,0x1000)
	ROM_CONTINUE(0x8000,0x8000)


	ROM_REGION( 0x20000, "graphics", 0 )
	ROM_LOAD( "rp35h.bin",  0x00000, 0x10000, CRC(664649ea) SHA1(7915ab31afd2a1bbb8f817f961e0e522d76f5c05) )
	ROM_LOAD( "rp35l.bin",  0x10000, 0x10000, CRC(ef416c4e) SHA1(5aac157ba15c66f79a7a68935095bef9a2636f7b) )

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_COPY( "graphics", 0x18000, 0x00000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x08000, 0x08000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x04000, 0x10000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x1c000, 0x04000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x0c000, 0x0c000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x14000, 0x14000, 0x4000 ) // 2

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "graphics", 0x02000, 0x00000, 0x2000 )
	ROM_COPY( "graphics", 0x12000, 0x02000, 0x2000 )
	ROM_COPY( "graphics", 0x00000, 0x04000, 0x2000 )
	ROM_COPY( "graphics", 0x10000, 0x06000, 0x2000 )

	ROM_REGION( 0x200, "proms", 0 ) // palette
	ROM_LOAD( "rpu19.bin", 0x0000, 0x0100, CRC(deb9ae3c) SHA1(056ce4947244ade1ff70f167a998140745b5cffa) )
	ROM_LOAD( "rpu20.bin", 0x0100, 0x0100, CRC(b3e0a328) SHA1(f8990fcd1e90d3e9205ee81f1d7dd105dbdcfcd6) )

	ROM_REGION( 0x100, "proms2", 0 ) // colours again?
	ROM_LOAD( "rpu1920.bin", 0x0000, 0x0100, CRC(e204e8f3) SHA1(9005fe9c72055af690701cd239f4b3665b2fae21) )
ROM_END


ROM_START( roypok96b )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rp36c3.bin", 0x0000, 0x1000, CRC(c7317ed7) SHA1(ca88d02c5ea5c03dd9407d71ab88e81c21791fe8) )
	ROM_CONTINUE(0x4000,0x1000)
	ROM_CONTINUE(0x3000,0x1000)
	ROM_CONTINUE(0x7000,0x1000)
	ROM_CONTINUE(0x1000,0x1000)
	ROM_CONTINUE(0x6000,0x1000)
	ROM_CONTINUE(0x2000,0x1000)
	ROM_CONTINUE(0x5000,0x1000)
	ROM_CONTINUE(0x8000,0x8000)


	ROM_REGION( 0x20000, "graphics", 0 )
	ROM_LOAD( "rp35h.bin",  0x00000, 0x10000, CRC(664649ea) SHA1(7915ab31afd2a1bbb8f817f961e0e522d76f5c05) )
	ROM_LOAD( "rp35l.bin",  0x10000, 0x10000, CRC(ef416c4e) SHA1(5aac157ba15c66f79a7a68935095bef9a2636f7b) )

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_COPY( "graphics", 0x18000, 0x00000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x08000, 0x08000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x04000, 0x10000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x1c000, 0x04000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x0c000, 0x0c000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x14000, 0x14000, 0x4000 ) // 2

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "graphics", 0x02000, 0x00000, 0x2000 )
	ROM_COPY( "graphics", 0x12000, 0x02000, 0x2000 )
	ROM_COPY( "graphics", 0x00000, 0x04000, 0x2000 )
	ROM_COPY( "graphics", 0x10000, 0x06000, 0x2000 )

	ROM_REGION( 0x200, "proms", 0 ) // palette
	ROM_LOAD( "rpu19.bin", 0x0000, 0x0100, CRC(deb9ae3c) SHA1(056ce4947244ade1ff70f167a998140745b5cffa) )
	ROM_LOAD( "rpu20.bin", 0x0100, 0x0100, CRC(b3e0a328) SHA1(f8990fcd1e90d3e9205ee81f1d7dd105dbdcfcd6) )

	ROM_REGION( 0x100, "proms2", 0 ) // colours again?
	ROM_LOAD( "rpu1920.bin", 0x0000, 0x0100, CRC(e204e8f3) SHA1(9005fe9c72055af690701cd239f4b3665b2fae21) )
ROM_END




ROM_START( pokonl97 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "po33.bin", 0x00000, 0x1000, CRC(55bdd5cf) SHA1(7fd9e5c63ab2439db33710d7684f5df5e7324325) )
	ROM_CONTINUE(0x4000,0x1000)
	ROM_CONTINUE(0x3000,0x1000)
	ROM_CONTINUE(0x7000,0x1000)
	ROM_CONTINUE(0x1000,0x1000)
	ROM_CONTINUE(0x6000,0x1000)
	ROM_CONTINUE(0x2000,0x1000)
	ROM_CONTINUE(0x5000,0x1000)
	ROM_CONTINUE(0x8000,0x8000)

	ROM_REGION( 0x20000, "graphics", 0 )
	ROM_LOAD( "po97h.bin",  0x00000, 0x10000, CRC(fe845426) SHA1(80a1ffa28f92ad381ccf01b387afddd3ee849a58) )
	ROM_LOAD( "po97l.bin",  0x10000, 0x10000, CRC(d389d5be) SHA1(a88db3bf411dd1bdf8dc42c8c440d71b24ef95ee) )

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_COPY( "graphics", 0x18000, 0x00000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x08000, 0x08000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x04000, 0x10000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x1c000, 0x04000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x0c000, 0x0c000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x14000, 0x14000, 0x4000 ) // 2

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "graphics", 0x02000, 0x00000, 0x2000 )
	ROM_COPY( "graphics", 0x12000, 0x02000, 0x2000 )
	ROM_COPY( "graphics", 0x00000, 0x04000, 0x2000 )
	ROM_COPY( "graphics", 0x10000, 0x06000, 0x2000 )


	ROM_REGION( 0x200, "proms", 0 ) // palette
	ROM_LOAD( "po97u19.bin", 0x0000, 0x0100, CRC(889dd4b3) SHA1(dc4b2506bf61f1bc4d491c3a9c410be11d93b76f) )
	ROM_LOAD( "po97u20.bin", 0x0100, 0x0100, CRC(e44d1b48) SHA1(0a21b79c03f33d31303ba6cabc4b5a23d7c9cfe3) )

	ROM_REGION( 0x100, "proms2", 0 ) // colours again?
	ROM_LOAD( "pou1920.bin", 0x0000, 0x0100, CRC(ceac07bb) SHA1(b6fca4ef937c0a75d6371db405faf15d69462fc4) )

	ROM_REGION( 0x80000, "oki", 0 ) // samples
	ROM_LOAD( "po97t.bin", 0x00000, 0x20000, CRC(dab7cbeb) SHA1(40cf5717485f31d5b5267a9f79ead0d21509d68c) )
ROM_END


ROM_START( nfb96 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "fb362c1.bin", 0x00000, 0x1000, CRC(5054418a) SHA1(a4bfe05b0eb3476651c06cb9ff78051e55c943c5) )
	ROM_CONTINUE(0x4000, 0x1000)
	ROM_CONTINUE(0x3000, 0x1000)
	ROM_CONTINUE(0x7000, 0x1000)
	ROM_CONTINUE(0x1000, 0x1000)
	ROM_CONTINUE(0x6000, 0x1000)
	ROM_CONTINUE(0x2000, 0x1000)
	ROM_CONTINUE(0x5000, 0x1000)
	ROM_CONTINUE(0x8000, 0x1000)
	ROM_CONTINUE(0x9000, 0x1000)
	ROM_CONTINUE(0xa000, 0x1000)
	ROM_CONTINUE(0xb000, 0x1000)
	ROM_CONTINUE(0xc000, 0x1000)
	ROM_CONTINUE(0xd000, 0x1000)
	ROM_CONTINUE(0xe000, 0x1000)
	ROM_CONTINUE(0xf000, 0x1000)

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )

	ROM_REGION( 0x20000, "graphics", 0 )
	ROM_LOAD( "fbseh.bin",  0x00000, 0x10000, CRC(2fc10ce7) SHA1(a2418cfbe7ed217848ace8ea06587bcaa6b2c8f2) )
	ROM_LOAD( "fbsel.bin",  0x10000, 0x10000, CRC(fb9d679a) SHA1(a4f6246bdbbf2e25f702006b30a62bc7873137de) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_COPY( "graphics", 0x18000, 0x00000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x08000, 0x08000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x04000, 0x10000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x1c000, 0x04000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x0c000, 0x0c000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x14000, 0x14000, 0x4000 ) // 2

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "graphics", 0x02000, 0x00000, 0x2000 )
	ROM_COPY( "graphics", 0x12000, 0x02000, 0x2000 )
	ROM_COPY( "graphics", 0x00000, 0x04000, 0x2000 )
	ROM_COPY( "graphics", 0x10000, 0x06000, 0x2000 )

	ROM_REGION( 0x200, "proms", 0 ) // palette
	ROM_LOAD( "chu19.bin", 0x0000, 0x0100, CRC(fafc43ad) SHA1(e94592b83f19e5f9b6205473c1e06b36405ebfc2) )
	ROM_LOAD( "chu20.bin", 0x0100, 0x0100, CRC(05224f73) SHA1(051c3ee9c63f5436e4f6c355fc308f37910a88ef) )

	ROM_REGION( 0x100, "proms2", 0 ) // colours again?
	ROM_LOAD( "chu1920.bin", 0x0000, 0x0100, CRC(71b0e11d) SHA1(1d2a2a31d8571f580c0cb7f4833823841072b31f) )

	ROM_REGION( 0x80000, "oki", ROMREGION_ERASEFF ) // samples
	// none?
ROM_END

ROM_START( nfb96a )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "fb363c1.bin", 0x00000, 0x1000, CRC(ddc22974) SHA1(86f5d4d32f69d054ee1b444e2c4d470603e0391c) ) // ?? alt program?
	ROM_CONTINUE(0x4000, 0x1000)
	ROM_CONTINUE(0x3000, 0x1000)
	ROM_CONTINUE(0x7000, 0x1000)
	ROM_CONTINUE(0x1000, 0x1000)
	ROM_CONTINUE(0x6000, 0x1000)
	ROM_CONTINUE(0x2000, 0x1000)
	ROM_CONTINUE(0x5000, 0x1000)
	ROM_CONTINUE(0x8000, 0x1000)
	ROM_CONTINUE(0x9000, 0x1000)
	ROM_CONTINUE(0xa000, 0x1000)
	ROM_CONTINUE(0xb000, 0x1000)
	ROM_CONTINUE(0xc000, 0x1000)
	ROM_CONTINUE(0xd000, 0x1000)
	ROM_CONTINUE(0xe000, 0x1000)
	ROM_CONTINUE(0xf000, 0x1000)

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )


	ROM_REGION( 0x20000, "graphics", 0 )
	ROM_LOAD( "fbseh.bin",  0x00000, 0x10000, CRC(2fc10ce7) SHA1(a2418cfbe7ed217848ace8ea06587bcaa6b2c8f2) )
	ROM_LOAD( "fbsel.bin",  0x10000, 0x10000, CRC(fb9d679a) SHA1(a4f6246bdbbf2e25f702006b30a62bc7873137de) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_COPY( "graphics", 0x18000, 0x00000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x08000, 0x08000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x04000, 0x10000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x1c000, 0x04000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x0c000, 0x0c000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x14000, 0x14000, 0x4000 ) // 2

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "graphics", 0x02000, 0x00000, 0x2000 )
	ROM_COPY( "graphics", 0x12000, 0x02000, 0x2000 )
	ROM_COPY( "graphics", 0x00000, 0x04000, 0x2000 )
	ROM_COPY( "graphics", 0x10000, 0x06000, 0x2000 )

	ROM_REGION( 0x200, "proms", 0 ) // palette
	ROM_LOAD( "chu19.bin", 0x0000, 0x0100, CRC(fafc43ad) SHA1(e94592b83f19e5f9b6205473c1e06b36405ebfc2) )
	ROM_LOAD( "chu20.bin", 0x0100, 0x0100, CRC(05224f73) SHA1(051c3ee9c63f5436e4f6c355fc308f37910a88ef) )

	ROM_REGION( 0x100, "sku1920.bin", 0 ) // colours again?
	ROM_LOAD( "chu1920.bin", 0x0000, 0x0100, CRC(71b0e11d) SHA1(1d2a2a31d8571f580c0cb7f4833823841072b31f) )

	ROM_REGION( 0x80000, "oki", ROMREGION_ERASEFF ) // samples
	// none?
ROM_END

ROM_START( nfb96b )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "fbse354.bin", 0x00000, 0x1000, CRC(89dd3290) SHA1(040aa1daae58a38db66a046a4379975e014a2598) ) // ?? alt program?
	ROM_CONTINUE(0x4000, 0x1000)
	ROM_CONTINUE(0x3000, 0x1000)
	ROM_CONTINUE(0x7000, 0x1000)
	ROM_CONTINUE(0x1000, 0x1000)
	ROM_CONTINUE(0x6000, 0x1000)
	ROM_CONTINUE(0x2000, 0x1000)
	ROM_CONTINUE(0x5000, 0x1000)
	ROM_CONTINUE(0x8000, 0x1000)
	ROM_CONTINUE(0x9000, 0x1000)
	ROM_CONTINUE(0xa000, 0x1000)
	ROM_CONTINUE(0xb000, 0x1000)
	ROM_CONTINUE(0xc000, 0x1000)
	ROM_CONTINUE(0xd000, 0x1000)
	ROM_CONTINUE(0xe000, 0x1000)
	ROM_CONTINUE(0xf000, 0x1000)

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )

	/* these graphic roms display a different copyright */
	ROM_REGION( 0x20000, "graphics", 0 )
	ROM_LOAD( "fb96seh.bin",  0x00000, 0x10000, CRC(12042a0a) SHA1(4bc5f87f4b92f303fef100bf16e3d7b27670b793) )
	ROM_LOAD( "fb96sel.bin",  0x10000, 0x10000, CRC(d611f10b) SHA1(425cad584e85f21de214bf978555a7811b13aa35) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_COPY( "graphics", 0x18000, 0x00000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x08000, 0x08000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x04000, 0x10000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x1c000, 0x04000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x0c000, 0x0c000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x14000, 0x14000, 0x4000 ) // 2

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "graphics", 0x02000, 0x00000, 0x2000 )
	ROM_COPY( "graphics", 0x12000, 0x02000, 0x2000 )
	ROM_COPY( "graphics", 0x00000, 0x04000, 0x2000 )
	ROM_COPY( "graphics", 0x10000, 0x06000, 0x2000 )

	ROM_REGION( 0x200, "proms", 0 ) // palette
	ROM_LOAD( "chu19.bin", 0x0000, 0x0100, CRC(fafc43ad) SHA1(e94592b83f19e5f9b6205473c1e06b36405ebfc2) )
	ROM_LOAD( "chu20.bin", 0x0100, 0x0100, CRC(05224f73) SHA1(051c3ee9c63f5436e4f6c355fc308f37910a88ef) )

	ROM_REGION( 0x100, "sku1920.bin", 0 ) // colours again?
	ROM_LOAD( "chu1920.bin", 0x0000, 0x0100, CRC(71b0e11d) SHA1(1d2a2a31d8571f580c0cb7f4833823841072b31f) )

	ROM_REGION( 0x80000, "oki", ROMREGION_ERASEFF ) // samples
	// none?
ROM_END

ROM_START( nfb96c )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "fbse362.bin", 0x00000, 0x1000, CRC(aa2576f2) SHA1(b6f0d6078dee01b90e08c09008f299439768c266) ) // ?? alt program?
	ROM_CONTINUE(0x4000, 0x1000)
	ROM_CONTINUE(0x3000, 0x1000)
	ROM_CONTINUE(0x7000, 0x1000)
	ROM_CONTINUE(0x1000, 0x1000)
	ROM_CONTINUE(0x6000, 0x1000)
	ROM_CONTINUE(0x2000, 0x1000)
	ROM_CONTINUE(0x5000, 0x1000)
	ROM_CONTINUE(0x8000, 0x8000)

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )

	/* these graphic roms display a different copyright */
	ROM_REGION( 0x20000, "graphics", 0 )
	ROM_LOAD( "fb96seh.bin",  0x00000, 0x10000, CRC(12042a0a) SHA1(4bc5f87f4b92f303fef100bf16e3d7b27670b793) )
	ROM_LOAD( "fb96sel.bin",  0x10000, 0x10000, CRC(d611f10b) SHA1(425cad584e85f21de214bf978555a7811b13aa35) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_COPY( "graphics", 0x18000, 0x00000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x08000, 0x08000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x04000, 0x10000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x1c000, 0x04000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x0c000, 0x0c000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x14000, 0x14000, 0x4000 ) // 2

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "graphics", 0x02000, 0x00000, 0x2000 )
	ROM_COPY( "graphics", 0x12000, 0x02000, 0x2000 )
	ROM_COPY( "graphics", 0x00000, 0x04000, 0x2000 )
	ROM_COPY( "graphics", 0x10000, 0x06000, 0x2000 )

	ROM_REGION( 0x200, "proms", 0 ) // palette
	ROM_LOAD( "chu19.bin", 0x0000, 0x0100, CRC(fafc43ad) SHA1(e94592b83f19e5f9b6205473c1e06b36405ebfc2) )
	ROM_LOAD( "chu20.bin", 0x0100, 0x0100, CRC(05224f73) SHA1(051c3ee9c63f5436e4f6c355fc308f37910a88ef) )

	ROM_REGION( 0x100, "sku1920.bin", 0 ) // colours again?
	ROM_LOAD( "chu1920.bin", 0x0000, 0x0100, CRC(71b0e11d) SHA1(1d2a2a31d8571f580c0cb7f4833823841072b31f) )

	ROM_REGION( 0x80000, "oki", ROMREGION_ERASEFF ) // samples
	// none?
ROM_END


ROM_START( nfb96txt )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "tf122axt.bin", 0x00000, 0x1000, CRC(50b5b105) SHA1(01348c463985d9967b2494b649fa02edbd61f698) )
	ROM_CONTINUE(0x4000, 0x1000)
	ROM_CONTINUE(0x3000, 0x1000)
	ROM_CONTINUE(0x7000, 0x1000)
	ROM_CONTINUE(0x1000, 0x1000)
	ROM_CONTINUE(0x6000, 0x1000)
	ROM_CONTINUE(0x2000, 0x1000)
	ROM_CONTINUE(0x5000, 0x1000)
	ROM_CONTINUE(0x8000, 0x8000)

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )

	/* these graphic roms display a different copyright */
	ROM_REGION( 0x20000, "graphics", 0 )
	ROM_LOAD( "txfbh.bin",  0x00000, 0x10000, CRC(9e9ba897) SHA1(901bb2596ff67c0290977fd508247aa1da0a09b9) )
	ROM_LOAD( "txfbl.bin",  0x10000, 0x10000, CRC(d1b8920c) SHA1(c6dc065134724baafed4c1cfa4aaf3c23dfb7a32) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_COPY( "graphics", 0x18000, 0x00000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x08000, 0x08000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x04000, 0x10000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x1c000, 0x04000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x0c000, 0x0c000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x14000, 0x14000, 0x4000 ) // 2

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "graphics", 0x02000, 0x00000, 0x2000 )
	ROM_COPY( "graphics", 0x12000, 0x02000, 0x2000 )
	ROM_COPY( "graphics", 0x00000, 0x04000, 0x2000 )
	ROM_COPY( "graphics", 0x10000, 0x06000, 0x2000 )

	ROM_REGION( 0x200, "proms", 0 ) // palette
	ROM_LOAD( "chu19.bin", 0x0000, 0x0100, CRC(fafc43ad) SHA1(e94592b83f19e5f9b6205473c1e06b36405ebfc2) )
	ROM_LOAD( "chu20.bin", 0x0100, 0x0100, CRC(05224f73) SHA1(051c3ee9c63f5436e4f6c355fc308f37910a88ef) )

	ROM_REGION( 0x100, "sku1920.bin", 0 ) // colours again?
	ROM_LOAD( "chu1920.bin", 0x0000, 0x0100, CRC(71b0e11d) SHA1(1d2a2a31d8571f580c0cb7f4833823841072b31f) )

	ROM_REGION( 0x80000, "oki", ROMREGION_ERASEFF ) // samples
	// none?
ROM_END


ROM_START( nc96 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "ch362c1.bin", 0x00000, 0x1000, CRC(10fa984b) SHA1(92ed9838db28deca038c4a5c080a723075ae0c53) )
	ROM_CONTINUE(0x4000, 0x1000)
	ROM_CONTINUE(0x3000, 0x1000)
	ROM_CONTINUE(0x7000, 0x1000)
	ROM_CONTINUE(0x1000, 0x1000)
	ROM_CONTINUE(0x6000, 0x1000)
	ROM_CONTINUE(0x2000, 0x1000)
	ROM_CONTINUE(0x5000, 0x1000)
	ROM_CONTINUE(0x8000, 0x8000)

	ROM_REGION( 0x20000, "graphics", 0 )
	ROM_LOAD( "chseh.bin",  0x00000, 0x10000, CRC(fb90df1d) SHA1(84ec1f40a014a0043b3c3c999428dd274caba1b8) )
	ROM_LOAD( "chsel.bin",  0x10000, 0x10000, CRC(e0166f3e) SHA1(27e180fe6e03f48771b540e34415eee54951788f) )

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_COPY( "graphics", 0x18000, 0x00000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x08000, 0x08000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x04000, 0x10000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x1c000, 0x04000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x0c000, 0x0c000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x14000, 0x14000, 0x4000 ) // 2

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "graphics", 0x02000, 0x00000, 0x2000 )
	ROM_COPY( "graphics", 0x12000, 0x02000, 0x2000 )
	ROM_COPY( "graphics", 0x00000, 0x04000, 0x2000 )
	ROM_COPY( "graphics", 0x10000, 0x06000, 0x2000 )

	// no proms present, using nfb96 ones
	ROM_REGION( 0x200, "proms", 0 ) // palette
	ROM_LOAD( "chu19.bin", 0x0000, 0x0100, CRC(fafc43ad) SHA1(e94592b83f19e5f9b6205473c1e06b36405ebfc2) )
	ROM_LOAD( "chu20.bin", 0x0100, 0x0100, CRC(05224f73) SHA1(051c3ee9c63f5436e4f6c355fc308f37910a88ef) )

	ROM_REGION( 0x100, "sku1920.bin", 0 ) // colours again?
	ROM_LOAD( "chu1920.bin", 0x0000, 0x0100, CRC(71b0e11d) SHA1(1d2a2a31d8571f580c0cb7f4833823841072b31f) )

	ROM_REGION( 0x80000, "oki", ROMREGION_ERASEFF ) // samples
	// none?
ROM_END


ROM_START( nc96a )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "ch363c1.bin", 0x00000, 0x1000, CRC(e566cea3) SHA1(be81b21267ae2ff64c4dbc58e99a9ff4bf1d21aa) ) // ?? alt program?
	ROM_CONTINUE(0x4000, 0x1000)
	ROM_CONTINUE(0x3000, 0x1000)
	ROM_CONTINUE(0x7000, 0x1000)
	ROM_CONTINUE(0x1000, 0x1000)
	ROM_CONTINUE(0x6000, 0x1000)
	ROM_CONTINUE(0x2000, 0x1000)
	ROM_CONTINUE(0x5000, 0x1000)
	ROM_CONTINUE(0x8000, 0x8000)

	ROM_REGION( 0x20000, "graphics", 0 )
	ROM_LOAD( "chseh.bin",  0x00000, 0x10000, CRC(fb90df1d) SHA1(84ec1f40a014a0043b3c3c999428dd274caba1b8) )
	ROM_LOAD( "chsel.bin",  0x10000, 0x10000, CRC(e0166f3e) SHA1(27e180fe6e03f48771b540e34415eee54951788f) )

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_COPY( "graphics", 0x18000, 0x00000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x08000, 0x08000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x04000, 0x10000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x1c000, 0x04000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x0c000, 0x0c000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x14000, 0x14000, 0x4000 ) // 2

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "graphics", 0x02000, 0x00000, 0x2000 )
	ROM_COPY( "graphics", 0x12000, 0x02000, 0x2000 )
	ROM_COPY( "graphics", 0x00000, 0x04000, 0x2000 )
	ROM_COPY( "graphics", 0x10000, 0x06000, 0x2000 )

	// no proms present, using nfb96 ones
	ROM_REGION( 0x200, "proms", 0 ) // palette
	ROM_LOAD( "chu19.bin", 0x0000, 0x0100, CRC(fafc43ad) SHA1(e94592b83f19e5f9b6205473c1e06b36405ebfc2) )
	ROM_LOAD( "chu20.bin", 0x0100, 0x0100, CRC(05224f73) SHA1(051c3ee9c63f5436e4f6c355fc308f37910a88ef) )

	ROM_REGION( 0x100, "sku1920.bin", 0 ) // colours again?
	ROM_LOAD( "chu1920.bin", 0x0000, 0x0100, CRC(71b0e11d) SHA1(1d2a2a31d8571f580c0cb7f4833823841072b31f) )

	ROM_REGION( 0x80000, "oki", ROMREGION_ERASEFF ) // samples
	// none?
ROM_END



ROM_START( nc96b )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "chse354.bin", 0x00000, 0x1000, CRC(160f7b78) SHA1(537a91317e613676b748d4e4ec7015183872814b) ) // ?? alt program?
	ROM_CONTINUE(0x4000, 0x1000)
	ROM_CONTINUE(0x3000, 0x1000)
	ROM_CONTINUE(0x7000, 0x1000)
	ROM_CONTINUE(0x1000, 0x1000)
	ROM_CONTINUE(0x6000, 0x1000)
	ROM_CONTINUE(0x2000, 0x1000)
	ROM_CONTINUE(0x5000, 0x1000)
	ROM_CONTINUE(0x8000, 0x8000)

	ROM_REGION( 0x20000, "graphics", 0 )
	ROM_LOAD( "ch96seh.bin",  0x00000, 0x10000, CRC(65dee6ba) SHA1(77f5769ed0b745a4735576e9f0ce90dcdd9b5410) )
	ROM_LOAD( "ch96sel.bin",  0x10000, 0x10000, CRC(c21cc114) SHA1(f7b6ff5ac34dc1a7332e8c1b9cc40f3b65deac05) )


	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_COPY( "graphics", 0x18000, 0x00000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x08000, 0x08000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x04000, 0x10000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x1c000, 0x04000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x0c000, 0x0c000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x14000, 0x14000, 0x4000 ) // 2

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "graphics", 0x02000, 0x00000, 0x2000 )
	ROM_COPY( "graphics", 0x12000, 0x02000, 0x2000 )
	ROM_COPY( "graphics", 0x00000, 0x04000, 0x2000 )
	ROM_COPY( "graphics", 0x10000, 0x06000, 0x2000 )

	// no proms present, using nfb96 ones
	ROM_REGION( 0x200, "proms", 0 ) // palette
	ROM_LOAD( "chu19.bin", 0x0000, 0x0100, CRC(fafc43ad) SHA1(e94592b83f19e5f9b6205473c1e06b36405ebfc2) )
	ROM_LOAD( "chu20.bin", 0x0100, 0x0100, CRC(05224f73) SHA1(051c3ee9c63f5436e4f6c355fc308f37910a88ef) )

	ROM_REGION( 0x100, "sku1920.bin", 0 ) // colours again?
	ROM_LOAD( "chu1920.bin", 0x0000, 0x0100, CRC(71b0e11d) SHA1(1d2a2a31d8571f580c0cb7f4833823841072b31f) )

	ROM_REGION( 0x80000, "oki", ROMREGION_ERASEFF ) // samples
	// none?
ROM_END


ROM_START( nc96c )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "chse362.bin", 0x00000, 0x1000, CRC(66df35d3) SHA1(da33f6413b8cf7b472023abc3e5cfe6d52ed1418) ) // ?? alt program?
	ROM_CONTINUE(0x4000, 0x1000)
	ROM_CONTINUE(0x3000, 0x1000)
	ROM_CONTINUE(0x7000, 0x1000)
	ROM_CONTINUE(0x1000, 0x1000)
	ROM_CONTINUE(0x6000, 0x1000)
	ROM_CONTINUE(0x2000, 0x1000)
	ROM_CONTINUE(0x5000, 0x1000)
	ROM_CONTINUE(0x8000, 0x8000)

	ROM_REGION( 0x20000, "graphics", 0 )
	ROM_LOAD( "ch96seh.bin",  0x00000, 0x10000, CRC(65dee6ba) SHA1(77f5769ed0b745a4735576e9f0ce90dcdd9b5410) )
	ROM_LOAD( "ch96sel.bin",  0x10000, 0x10000, CRC(c21cc114) SHA1(f7b6ff5ac34dc1a7332e8c1b9cc40f3b65deac05) )

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_COPY( "graphics", 0x18000, 0x00000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x08000, 0x08000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x04000, 0x10000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x1c000, 0x04000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x0c000, 0x0c000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x14000, 0x14000, 0x4000 ) // 2

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "graphics", 0x02000, 0x00000, 0x2000 )
	ROM_COPY( "graphics", 0x12000, 0x02000, 0x2000 )
	ROM_COPY( "graphics", 0x00000, 0x04000, 0x2000 )
	ROM_COPY( "graphics", 0x10000, 0x06000, 0x2000 )

	// no proms present, using nfb96 ones
	ROM_REGION( 0x200, "proms", 0 ) // palette
	ROM_LOAD( "chu19.bin", 0x0000, 0x0100, CRC(fafc43ad) SHA1(e94592b83f19e5f9b6205473c1e06b36405ebfc2) )
	ROM_LOAD( "chu20.bin", 0x0100, 0x0100, CRC(05224f73) SHA1(051c3ee9c63f5436e4f6c355fc308f37910a88ef) )

	ROM_REGION( 0x100, "sku1920.bin", 0 ) // colours again?
	ROM_LOAD( "chu1920.bin", 0x0000, 0x0100, CRC(71b0e11d) SHA1(1d2a2a31d8571f580c0cb7f4833823841072b31f) )

	ROM_REGION( 0x80000, "oki", ROMREGION_ERASEFF ) // samples
	// none?
ROM_END

ROM_START( nc96txt )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "tc132axt.bin", 0x00000, 0x1000, CRC(a77dc042) SHA1(1076a6687da1871b666bd214a755b68f5e1aeb8c) )
	ROM_CONTINUE(0x4000, 0x1000)
	ROM_CONTINUE(0x3000, 0x1000)
	ROM_CONTINUE(0x7000, 0x1000)
	ROM_CONTINUE(0x1000, 0x1000)
	ROM_CONTINUE(0x6000, 0x1000)
	ROM_CONTINUE(0x2000, 0x1000)
	ROM_CONTINUE(0x5000, 0x1000)
	ROM_CONTINUE(0x8000, 0x8000)

	ROM_REGION( 0x20000, "graphics", 0 )
	ROM_LOAD( "txchh.bin",  0x00000, 0x10000, CRC(07621bde) SHA1(65fd52545a399694394e623a7249d180d1a4fa9f) )
	ROM_LOAD( "txchl.bin",  0x10000, 0x10000, CRC(3b00b7dc) SHA1(99d513d23eea28fedf59ad272a3280abd0d3c2ab) )

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_COPY( "graphics", 0x18000, 0x00000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x08000, 0x08000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x04000, 0x10000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x1c000, 0x04000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x0c000, 0x0c000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x14000, 0x14000, 0x4000 ) // 2

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "graphics", 0x02000, 0x00000, 0x2000 )
	ROM_COPY( "graphics", 0x12000, 0x02000, 0x2000 )
	ROM_COPY( "graphics", 0x00000, 0x04000, 0x2000 )
	ROM_COPY( "graphics", 0x10000, 0x06000, 0x2000 )

	// no proms present, using nfb96 ones
	ROM_REGION( 0x200, "proms", 0 ) // palette
	ROM_LOAD( "chu19.bin", 0x0000, 0x0100, CRC(fafc43ad) SHA1(e94592b83f19e5f9b6205473c1e06b36405ebfc2) )
	ROM_LOAD( "chu20.bin", 0x0100, 0x0100, CRC(05224f73) SHA1(051c3ee9c63f5436e4f6c355fc308f37910a88ef) )

	ROM_REGION( 0x100, "sku1920.bin", 0 ) // colours again?
	ROM_LOAD( "chu1920.bin", 0x0000, 0x0100, CRC(71b0e11d) SHA1(1d2a2a31d8571f580c0cb7f4833823841072b31f) )

	ROM_REGION( 0x80000, "oki", ROMREGION_ERASEFF ) // samples
	// none?
ROM_END

ROM_START( match98 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "match133.bin", 0x00000, 0x1000, CRC(ddd82435) SHA1(4d7310f77e1f87e2b5c820a311aaefd82307b388) )
	ROM_CONTINUE(0x4000, 0x1000)
	ROM_CONTINUE(0x3000, 0x1000)
	ROM_CONTINUE(0x7000, 0x1000)
	ROM_CONTINUE(0x1000, 0x1000)
	ROM_CONTINUE(0x6000, 0x1000)
	ROM_CONTINUE(0x2000, 0x1000)
	ROM_CONTINUE(0x5000, 0x1000)
	ROM_CONTINUE(0x8000, 0x8000)

	ROM_REGION( 0x20000, "graphics", 0 )
	ROM_LOAD( "match98h.bin",  0x00000, 0x10000, CRC(94899f26) SHA1(1b6f953b6251496d7d06fb0a2d0b861e04ebc3df) )
	ROM_LOAD( "match98l.bin",  0x10000, 0x10000, CRC(6db4b962) SHA1(7d476e244d70a86dacf85dd9c790f63aef4b7cd9) )

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_COPY( "graphics", 0x18000, 0x00000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x08000, 0x08000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x04000, 0x10000, 0x4000 ) // 1
	ROM_COPY( "graphics", 0x1c000, 0x04000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x0c000, 0x0c000, 0x4000 ) // 2
	ROM_COPY( "graphics", 0x14000, 0x14000, 0x4000 ) // 2

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "graphics", 0x02000, 0x00000, 0x2000 )
	ROM_COPY( "graphics", 0x12000, 0x02000, 0x2000 )
	ROM_COPY( "graphics", 0x00000, 0x04000, 0x2000 )
	ROM_COPY( "graphics", 0x10000, 0x06000, 0x2000 )

	ROM_REGION( 0x200, "proms", 0 ) // palette
	ROM_LOAD( "matchu19.bin", 0x0000, 0x0100, CRC(59cd3566) SHA1(e9726aad631d86e2c122e3d75f87abc22ea7ef97) )
	ROM_LOAD( "matchu20.bin", 0x0100, 0x0100, CRC(f421503c) SHA1(591c80d6ff63021fab31b3bfcde1b47cd75fd7bb) )

	ROM_REGION( 0x100, "proms2", 0 ) // colours again?
	ROM_LOAD( "matu1920.bin", 0x0000, 0x0100, CRC(c249576f) SHA1(54d51a54f4b2503c706c1c06050e33be7f479dfc) )

	ROM_REGION( 0x100, "proms3", 0 ) // ? none of the other sets have this
	ROM_LOAD( "matchu8.bin", 0x0000, 0x0100, CRC(dba4579d) SHA1(fba0a5adad13728c805fbe9666a8e02484cfa821) )

	ROM_REGION( 0x80000, "oki", ROMREGION_ERASEFF ) // samples
	ROM_LOAD( "match98t.bin", 0x00000, 0x40000, CRC(830f4e01) SHA1(fbc41e9100a69663b0f799aee447edd5fabd2af7) )
ROM_END




/* descrambled by looking at CALLs

0000 -> 0000

46e7 -> 16e7
4027 -> 1027

35f3 -> 25f3
3327 -> 2327

7f6a -> 3f6a

1095 -> 4095
1d2f -> 4d2f
1e8b -> 4e8b

6246 -> 5246
628f -> 528f

2bed -> 6bed
2db7 -> 6db7

5838 -> 7838
58a2 -> 78a2

810f -> 810f

9762 -> 9762

a??? -> a???

b84a -> b84a

c??? -> c???

*/

ROM_START( nfb96se )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dogdptb.prg",0x00000, 0x1000, CRC(0690f915) SHA1(ed2477ba260a421013603017cfd1e1ba5ecd7f4e) ) // alt program?
	ROM_CONTINUE(0x4000, 0x1000)
	ROM_CONTINUE(0x3000, 0x1000)
	ROM_CONTINUE(0x7000, 0x1000)
	ROM_CONTINUE(0x1000, 0x1000)
	ROM_CONTINUE(0x6000, 0x1000)
	ROM_CONTINUE(0x2000, 0x1000)
	ROM_CONTINUE(0x5000, 0x1000)
	ROM_CONTINUE(0x8000, 0x1000)
	ROM_CONTINUE(0x9000, 0x1000)
	ROM_CONTINUE(0xa000, 0x1000)
	ROM_CONTINUE(0xb000, 0x1000)
	ROM_CONTINUE(0xc000, 0x1000)
	ROM_CONTINUE(0xd000, 0x1000)
	ROM_CONTINUE(0xe000, 0x1000)
	ROM_CONTINUE(0xf000, 0x1000)

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "dogd5.rom",   0x10000, 0x08000, CRC(c03b5632) SHA1(4f603ec5218adcbfce09ec6d3643ffb5006056dd) )
	ROM_LOAD( "dogd6.rom",   0x08000, 0x08000, CRC(c48e5b5c) SHA1(9d79631b54d9915cd161b5028c1be7879254d9be) )
	ROM_LOAD( "dogd7.rom",   0x00000, 0x08000, CRC(2f03f1e2) SHA1(b221ad7177fcf4d6d65b8ee9c0f5e4289688c707) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "dogd1.rom",	0x6000, 0x2000, CRC(0f9f77b1) SHA1(03719f79a39f93f38e4170143a5654bd74596206) )
	ROM_LOAD( "dogd2.rom",	0x4000, 0x2000, CRC(6ab19916) SHA1(f125365b3c5546d72662cf439311811ae761f225) )
	ROM_LOAD( "dogd3.rom",	0x2000, 0x2000, CRC(5d4810a5) SHA1(8e9e50c6c7c13010ecb726041a1ac8eccead96ce) )
	ROM_LOAD( "dogd4.rom",	0x0000, 0x2000, CRC(be31f6fa) SHA1(b522ff520b3fbb34c55c7bb1fe7dfeecd593d6be) )

	// taken from new fruit bonus '96, might be wrong
	ROM_REGION( 0x200, "proms", 0 ) // palette
	ROM_LOAD( "chu19.bin", 0x0000, 0x0100, CRC(fafc43ad) SHA1(e94592b83f19e5f9b6205473c1e06b36405ebfc2) )
	ROM_LOAD( "chu20.bin", 0x0100, 0x0100, CRC(05224f73) SHA1(051c3ee9c63f5436e4f6c355fc308f37910a88ef) )

	ROM_REGION( 0x100, "proms2", 0 ) // colours again?
	ROM_LOAD( "chu1920.bin", 0x0000, 0x0100, CRC(71b0e11d) SHA1(1d2a2a31d8571f580c0cb7f4833823841072b31f) )
ROM_END


// this set has an encrypted program rom
ROM_START( nfb96sea )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dog_08.rom",   0x00000, 0x1000, CRC(357f13e8) SHA1(ca0872c9f7dc44a4c1c342f7f53c490f6342f1d2) )
	ROM_CONTINUE(0x4000, 0x1000)
	ROM_CONTINUE(0x3000, 0x1000)
	ROM_CONTINUE(0x7000, 0x1000)
	ROM_CONTINUE(0x1000, 0x1000)
	ROM_CONTINUE(0x6000, 0x1000)
	ROM_CONTINUE(0x2000, 0x1000)
	ROM_CONTINUE(0x5000, 0x1000)
	ROM_CONTINUE(0x8000, 0x1000)
	ROM_CONTINUE(0x9000, 0x1000)
	ROM_CONTINUE(0xa000, 0x1000)
	ROM_CONTINUE(0xb000, 0x1000)
	ROM_CONTINUE(0xc000, 0x1000)
	ROM_CONTINUE(0xd000, 0x1000)
	ROM_CONTINUE(0xe000, 0x1000)
	ROM_CONTINUE(0xf000, 0x1000)

	/* the graphic roms on this set are a mess, the planes don't match up properly */
	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "dog_05.rom",   0x10000, 0x08000, CRC(c03b5632) SHA1(4f603ec5218adcbfce09ec6d3643ffb5006056dd) )
	ROM_LOAD( "dog_06.rom",   0x08000, 0x08000, CRC(c48e5b5c) SHA1(9d79631b54d9915cd161b5028c1be7879254d9be) )
	ROM_LOAD( "dog_07.rom",   0x00000, 0x08000, CRC(97033c70) SHA1(4d5746f43f8f4d374ba2f31d21defd21921d39bf) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "dog_01.rom",	0x0000, 0x2000, CRC(26eb35ce) SHA1(564c775eda7a026841095b210257aea59288f042) )
	ROM_LOAD( "dog_02.rom",	0x2000, 0x2000, CRC(0e220d8a) SHA1(f84145250785dae78ea5af6388d91ad24b42ff9c) )
	ROM_LOAD( "dog_03.rom",	0x4000, 0x2000, CRC(01a7ff6f) SHA1(bfb4ad07d99807eadbb0cb85c5a6cf60a5875f2d) )
	ROM_LOAD( "dog_04.rom",	0x6000, 0x2000, CRC(be31f6fa) SHA1(b522ff520b3fbb34c55c7bb1fe7dfeecd593d6be) )

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )

	// taken from new fruit bonus '96, might be wrong
	ROM_REGION( 0x200, "proms", 0 ) // palette
	ROM_LOAD( "chu19.bin", 0x0000, 0x0100, CRC(fafc43ad) SHA1(e94592b83f19e5f9b6205473c1e06b36405ebfc2) )
	ROM_LOAD( "chu20.bin", 0x0100, 0x0100, CRC(05224f73) SHA1(051c3ee9c63f5436e4f6c355fc308f37910a88ef) )

	ROM_REGION( 0x100, "proms2", 0 ) // colours again?
	ROM_LOAD( "chu1920.bin", 0x0000, 0x0100, CRC(71b0e11d) SHA1(1d2a2a31d8571f580c0cb7f4833823841072b31f) )
ROM_END

ROM_START( nfb96seb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dogdptb.prg",0x00000, 0x1000, CRC(0690f915) SHA1(ed2477ba260a421013603017cfd1e1ba5ecd7f4e) ) // alt program?
	ROM_CONTINUE(0x4000, 0x1000)
	ROM_CONTINUE(0x3000, 0x1000)
	ROM_CONTINUE(0x7000, 0x1000)
	ROM_CONTINUE(0x1000, 0x1000)
	ROM_CONTINUE(0x6000, 0x1000)
	ROM_CONTINUE(0x2000, 0x1000)
	ROM_CONTINUE(0x5000, 0x1000)
	ROM_CONTINUE(0x8000, 0x8000)


	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "dogmx5",   0x10000, 0x08000, CRC(c03b5632) SHA1(4f603ec5218adcbfce09ec6d3643ffb5006056dd) )
	ROM_LOAD( "dogmx6",   0x08000, 0x08000, CRC(c48e5b5c) SHA1(9d79631b54d9915cd161b5028c1be7879254d9be) )
	ROM_LOAD( "dogmx7",   0x00000, 0x08000, CRC(97033c70) SHA1(4d5746f43f8f4d374ba2f31d21defd21921d39bf) ) // wagner video junk on this layer

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "dogmx1",	0x6000, 0x2000, CRC(b72d2c2c) SHA1(faf60ca0f522868e6dbf7c3ace5c84d8fd001df3) )
	ROM_LOAD( "dogmx2",	0x4000, 0x2000, CRC(a85f5516) SHA1(1564e6c490883c96bffc561d9115eb53450945ce) )
	ROM_LOAD( "dogmx3",	0x2000, 0x2000, CRC(f1a8aea8) SHA1(c20b779a73856d94e862d87ad337c9501da86691) )
	ROM_LOAD( "dogmx4",	0x0000, 0x2000, CRC(be31f6fa) SHA1(b522ff520b3fbb34c55c7bb1fe7dfeecd593d6be) )

	// taken from new fruit bonus '96, might be wrong
	ROM_REGION( 0x200, "proms", 0 ) // palette
	ROM_LOAD( "chu19.bin", 0x0000, 0x0100, CRC(fafc43ad) SHA1(e94592b83f19e5f9b6205473c1e06b36405ebfc2) )
	ROM_LOAD( "chu20.bin", 0x0100, 0x0100, CRC(05224f73) SHA1(051c3ee9c63f5436e4f6c355fc308f37910a88ef) )

	ROM_REGION( 0x100, "proms2", 0 ) // colours again?
	ROM_LOAD( "chu1920.bin", 0x0000, 0x0100, CRC(71b0e11d) SHA1(1d2a2a31d8571f580c0cb7f4833823841072b31f) )
ROM_END

// this contains elephants etc. instead of the usual symbols, maybe
// it's meant to work with the above program roms?
ROM_START( carb2002 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dogd.prg",   0x00000, 0x1000, CRC(000102e0) SHA1(a1824576845b67fbc1a9a16d5aafa6cd000ea4fb) )
	ROM_CONTINUE(0x4000, 0x1000)
	ROM_CONTINUE(0x3000, 0x1000)
	ROM_CONTINUE(0x7000, 0x1000)
	ROM_CONTINUE(0x1000, 0x1000)
	ROM_CONTINUE(0x6000, 0x1000)
	ROM_CONTINUE(0x2000, 0x1000)
	ROM_CONTINUE(0x5000, 0x1000)
	ROM_CONTINUE(0x8000, 0x1000)
	ROM_CONTINUE(0x9000, 0x1000)
	ROM_CONTINUE(0xa000, 0x1000)
	ROM_CONTINUE(0xb000, 0x1000)
	ROM_CONTINUE(0xc000, 0x1000)
	ROM_CONTINUE(0xd000, 0x1000)
	ROM_CONTINUE(0xe000, 0x1000)
	ROM_CONTINUE(0xf000, 0x1000)

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "dogtai5",   0x10000, 0x08000, CRC(0c8a4afb) SHA1(994295eea7964d60b6a1db911679739a10be9bbe) )
	ROM_LOAD( "dogtai6",   0x08000, 0x08000, CRC(e9f5dc36) SHA1(6a4060c901f10202fe935701f1f1087c8477da56) )
	ROM_LOAD( "dogtai7",   0x00000, 0x08000, CRC(73c4c1aa) SHA1(31a70861dc54b442a1e50adf3f013dbc38fbbbb1) )
	// alt. replacements for roms 5+6?, erases the word 'slot' on the title screen?
	ROM_LOAD( "dogdif5",   0x10000, 0x08000, CRC(a1986e44) SHA1(3178de9c6063c9f33878b6070db95b2eeb12ffea) )
	ROM_LOAD( "dogdif6",   0x08000, 0x08000, CRC(a5d389fc) SHA1(3db570c938a387708974f24a110cf25b9b52ac22) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "dogtai1",	0x6000, 0x2000, CRC(68ed1c26) SHA1(495a8b57c368b0b7c2a618d4f1e980d5187f411a) )
	ROM_LOAD( "dogtai2",	0x4000, 0x2000, CRC(b5e25d9b) SHA1(9374f7662f92c10ca6d1af570eaa4d161173283f) )
	ROM_LOAD( "dogtai3",	0x2000, 0x2000, CRC(df13aeb2) SHA1(942f742a722bab44dd3de270001b60d888c44111) )
	ROM_LOAD( "dogtai4",	0x0000, 0x2000, CRC(170f07ce) SHA1(4b48841f9c5bdf7bfbc05113148666a5bcdd3d35) )

	// taken from new fruit bonus '96, definitely wrong
	ROM_REGION( 0x200, "proms", 0 ) // palette
	ROM_LOAD( "chu19.bin", 0x0000, 0x0100, BAD_DUMP CRC(fafc43ad) SHA1(e94592b83f19e5f9b6205473c1e06b36405ebfc2) )
	ROM_LOAD( "chu20.bin", 0x0100, 0x0100, BAD_DUMP CRC(05224f73) SHA1(051c3ee9c63f5436e4f6c355fc308f37910a88ef) )

	ROM_REGION( 0x100, "proms2", 0 ) // colours again?
	ROM_LOAD( "chu1920.bin", 0x0000, 0x0100, BAD_DUMP CRC(71b0e11d) SHA1(1d2a2a31d8571f580c0cb7f4833823841072b31f) )
ROM_END

// same program as dogh set.. different gfx
ROM_START( carb2003 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dogd.prg",   0x00000, 0x1000, CRC(000102e0) SHA1(a1824576845b67fbc1a9a16d5aafa6cd000ea4fb) )
	ROM_CONTINUE(0x4000, 0x1000)
	ROM_CONTINUE(0x3000, 0x1000)
	ROM_CONTINUE(0x7000, 0x1000)
	ROM_CONTINUE(0x1000, 0x1000)
	ROM_CONTINUE(0x6000, 0x1000)
	ROM_CONTINUE(0x2000, 0x1000)
	ROM_CONTINUE(0x5000, 0x1000)
	ROM_CONTINUE(0x8000, 0x1000)
	ROM_CONTINUE(0x9000, 0x1000)
	ROM_CONTINUE(0xa000, 0x1000)
	ROM_CONTINUE(0xb000, 0x1000)
	ROM_CONTINUE(0xc000, 0x1000)
	ROM_CONTINUE(0xd000, 0x1000)
	ROM_CONTINUE(0xe000, 0x1000)
	ROM_CONTINUE(0xf000, 0x1000)

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "timedog5.rom",   0x10000, 0x08000, CRC(cf05b66d) SHA1(a711a86f2a82dd685a379fda0cf7240b2ca2696e) )
	ROM_LOAD( "timedog6.rom",   0x08000, 0x08000, CRC(2d81bdbe) SHA1(56eaa9347014340b902d8f0bc38b719acf56c314) )
	ROM_LOAD( "timedog7.rom",   0x00000, 0x08000, CRC(f8e410e5) SHA1(1edc863902cfb1605aca08f6970f9bd24147ca0b) )

	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )

	// these seem to contain mixed planes of different gfx sets.. not correct
	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "timedog1.rom",	0x6000, 0x2000, CRC(d29e0217) SHA1(df978143ed313b33f848e7337097fe29c1fa4506) )
	ROM_LOAD( "timedog2.rom",	0x4000, 0x2000, CRC(fafb6a51) SHA1(57e79e5efd525cdf5a4475eedfee2c8fc1417b76) )
	ROM_LOAD( "timedog3.rom",	0x2000, 0x2000, CRC(6f305ac7) SHA1(55f0d73b783854584195658ab4e74770bc13ba0b) )
	ROM_LOAD( "timedog4.rom",	0x0000, 0x2000, CRC(807a16fc) SHA1(111e7d171f9278abea666d6ad41b02f2c8bf98d8) )
	ROM_IGNORE(0x2000)

	// taken from new fruit bonus '96, definitely wrong
	ROM_REGION( 0x200, "proms", 0 ) // palette
	ROM_LOAD( "chu19.bin", 0x0000, 0x0100, BAD_DUMP CRC(fafc43ad) SHA1(e94592b83f19e5f9b6205473c1e06b36405ebfc2) )
	ROM_LOAD( "chu20.bin", 0x0100, 0x0100, BAD_DUMP CRC(05224f73) SHA1(051c3ee9c63f5436e4f6c355fc308f37910a88ef) )

	ROM_REGION( 0x100, "proms2", 0 ) // colours again?
	ROM_LOAD( "chu1920.bin", 0x0000, 0x0100, BAD_DUMP CRC(71b0e11d) SHA1(1d2a2a31d8571f580c0cb7f4833823841072b31f) )
ROM_END



ROM_START( nfm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fuitprg", 0x00000, 0x01000, CRC(6f6c98cf) SHA1(4641cb2b90d4d21edc65e504584f3ec92fe741c4) )
	ROM_CONTINUE(0x5000,0x1000)
	ROM_CONTINUE(0x7000,0x1000)
	ROM_CONTINUE(0xa000,0x1000)
	ROM_CONTINUE(0x2000,0x1000) //?
	ROM_CONTINUE(0xb000,0x1000) //?
	ROM_CONTINUE(0x9000,0x1000)
	ROM_CONTINUE(0xc000,0x1000)
	ROM_CONTINUE(0x1000,0x1000)
	ROM_CONTINUE(0x3000,0x1000)
	ROM_CONTINUE(0x6000,0x1000)
	ROM_CONTINUE(0x4000,0x1000)
	ROM_CONTINUE(0x8000,0x1000)
	ROM_CONTINUE(0xd000,0x1000) // ?
	ROM_CONTINUE(0xe000,0x1000)
	ROM_CONTINUE(0xf000,0x1000)

	ROM_REGION( 0x10000, "reelgfx", 0 )
	ROM_LOAD( "fruit1", 0x00000, 0x04000, CRC(dd096dae) SHA1(ab34942cfa4fe7d46892819372c42f566c249f8c) )
	ROM_CONTINUE(0x0000, 0x4000)
	ROM_LOAD( "fruit2", 0x04000, 0x04000, CRC(6a37a16f) SHA1(7adb08d3e4de9768a8e41760a044bf52509da211) )
	ROM_CONTINUE(0x0000, 0x4000)
	ROM_LOAD( "fruit3", 0x08000, 0x04000, CRC(f3361ba7) SHA1(1a7b9c4f685656447bd6ce5f361e6e4af63012e3) )
	ROM_CONTINUE(0x8000, 0x4000)
	ROM_LOAD( "fruit4", 0x0c000, 0x04000, CRC(99ac5ddf) SHA1(65b6abb98f3156f4c0c55478d09c612eed5ae555) )

	// do these graphics really belong with this set? a lot of the tiles seem wrong for it
	ROM_REGION( 0x18000, "tilegfx", 0 )
	ROM_LOAD( "fruit5", 0x00000, 0x08000, CRC(a7a8f08d) SHA1(76c93194133ba85c0dde1f364260e16d5b647134) )
	ROM_LOAD( "fruit6", 0x08000, 0x08000, CRC(39d5b89a) SHA1(4cf52fa557ffc792d3e13f7dbb5d45fd617bac85) )
	ROM_LOAD( "fruit7", 0x10000, 0x08000, CRC(3ade6709) SHA1(9cdf2814e50c5433c582fc40265c5df2a16e99e7) )

	ROM_REGION( 0x18000, "proms", 0 ) // colours?
	ROM_LOAD( "fruiprg2", 0x00000, 0x08000, CRC(13925ff5) SHA1(236415a244ef6092834f8080cf0d2e04bbfa2650) )
ROM_END



ROM_START( unkch1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u6.bin",  0x0000, 0x10000, CRC(30309996) SHA1(290f35f587fdf78dcb4f09403c510deec533c9c2) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "u29.bin", 0x00000, 0x20000, CRC(6db245a1) SHA1(e9f85ba29b0af483eae6f999f49f1e431d9d2e27) )

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "u41.bin", 0x00000, 0x40000, CRC(b2bca15d) SHA1(57747c9c05e5ab54e40cbded2e420dfbfc929ce5) )
ROM_END

ROM_START( unkch2 ) // only gfx2 differs from unkch1
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u6.bin",  0x0000, 0x10000, CRC(30309996) SHA1(290f35f587fdf78dcb4f09403c510deec533c9c2) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "u29.bin", 0x00000, 0x20000, CRC(6db245a1) SHA1(e9f85ba29b0af483eae6f999f49f1e431d9d2e27) )

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "u41.1", 0x00000, 0x40000, CRC(725b48c7) SHA1(2f21c33fb7d23ad9411e926130a65b75029b9112) )
ROM_END

ROM_START( unkch3 )  // gfx2 is the same as unkch1, others differ
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u6.3",  0x0000, 0x10000, CRC(902f9e42) SHA1(ac5843089748d457f70ea52d15285a0ccda705ad) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "u29.3", 0x00000, 0x20000, CRC(546929e6) SHA1(f97fe5687f8776f0abe68962a0246c9bbeb6acd1) )

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "u41.bin", 0x00000, 0x40000, CRC(b2bca15d) SHA1(57747c9c05e5ab54e40cbded2e420dfbfc929ce5) )
ROM_END

ROM_START( unkch4 )  // all roms unique
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u6.4",  0x0000, 0x10000, CRC(eb191efa) SHA1(3004f26f9af7633df572f609647716cc4ac75990) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "u29.4", 0x00000, 0x20000, CRC(eaec0034) SHA1(6b2d3922873979eafcd4c71c52017263482b82ab) )

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "u41.4", 0x00000, 0x40000, CRC(ef586512) SHA1(a720e40903dd04b2c498efad40d583618596e048) )
ROM_END


static DRIVER_INIT(goldstar)
{
	int A;
	UINT8 *ROM = memory_region(machine, "maincpu");

	for (A = 0;A < 0x10000;A++)
	{
		if ((A & 0x30) == 0)
			ROM[A] ^= 0x82;
		else
			ROM[A] ^= 0xcc;
	}
}

// this block swapping is the same for chry10, chrygld and cb3
//  the underlying bitswaps / xors are different however
static void do_blockswaps(running_machine *machine, UINT8* ROM)
{
	int A;
	UINT8 *buffer;

	static const UINT16 cherry_swaptables[32] = {
		/* to align with goldstar */
		0x0800, 0x4000, 0x2800, 0x5800,
		0x1800, 0x3000, 0x6800, 0x7000,
		0x0000, 0x4800, 0x2000, 0x5000,
		0x1000, 0x7800, 0x6000, 0x3800,
		/* bit below, I'm not sure, no exact match, but only the first ones matter,
           as the is just garbage */
		0xc000, 0xc800, 0xd000, 0xd800,
		0xe000, 0xe800, 0xf000, 0xf800,
		0x8000, 0x8800, 0x9000, 0x9800,
		0xa000, 0xa800, 0xb000, 0xb800,
	};

	buffer = auto_alloc_array(machine, UINT8, 0x10000);
	memcpy(buffer,ROM,0x10000);

	// swap some 0x800 blocks around..
	for (A =0;A<32; A++)
	{
		memcpy(ROM+A*0x800,buffer+cherry_swaptables[A],0x800);
	}

	auto_free(machine, buffer);
}

static void dump_to_file(running_machine* machine, UINT8* ROM)
{
	#if 0
	{
		FILE *fp;
		char filename[256];
		sprintf(filename,"decrypted_%s", machine->gamedrv->name);
		fp=fopen(filename, "w+b");
		if (fp)
		{
			fwrite(ROM, 0x10000, 1, fp);
			fclose(fp);
		}
	}
	#endif
}

static UINT8 decrypt(UINT8 cipherText, UINT16 address)
{
	int idx;
	UINT8 output;
	int rotation[8] = {1, 0, 0, 1, 0, 1, 1, 1};
	int sbox[8] = {0x08, 0x08, 0x28, 0x00, 0x20, 0x20, 0x88, 0x88};

	idx = BIT(cipherText, 1) | (BIT(address,0) << 1) | (BIT(address, 4) << 2);

	if (rotation[idx] == 0)
		output = BITSWAP8(cipherText, 5, 6, 3, 4, 7, 2, 1, 0);   // rotates bit #3, #5 and #7 in one direction...
	else
		output = BITSWAP8(cipherText, 3, 6, 7, 4, 5, 2, 1, 0);   // ... or in the other

	return output ^ sbox[idx];
}

static UINT8 chry10_decrypt(UINT8 cipherText)
{
	return cipherText ^ (BIT(cipherText, 4) << 3) ^ (BIT(cipherText, 1) << 5) ^ (BIT(cipherText, 6) << 7);
}

static DRIVER_INIT( chry10 )
{
	UINT8 *ROM = memory_region(machine, "maincpu");
	int size = memory_region_length(machine, "maincpu");
	int start = 0;

	int i;

	for (i = start; i < size; i++)
	{
		ROM[i] = chry10_decrypt(ROM[i]);
	}

	do_blockswaps(machine, ROM);

	/* The game has a PIC for protection.
       If the code enter to this sub, just
       keeps looping eternally...
    */
	ROM[0xA5DC] = 0xc9;

	dump_to_file(machine, ROM);
}

static DRIVER_INIT( cb3 )
{
	UINT8 *ROM = memory_region(machine, "maincpu");
	int size = memory_region_length(machine, "maincpu");
	int start = 0;

	int i;

	for (i = start; i < size; i++)
	{
		ROM[i] = decrypt(ROM[i], i);
	}

	do_blockswaps(machine, ROM);
	dump_to_file(machine, ROM);
}


static DRIVER_INIT( chrygld )
{
	int A;
	UINT8 *ROM = memory_region(machine, "maincpu");
	do_blockswaps(machine, ROM);

	// a data bitswap
	for (A = 0;A < 0x10000;A++)
	{
		UINT8 dat = ROM[A];
		dat =  BITSWAP8(dat,5,6,3,4,7,2,1,0);
		ROM[A] = dat;
	}

	dump_to_file(machine, ROM);
}

static DRIVER_INIT(cm)
{
	UINT8 *ROM = memory_region(machine, "maincpu");

/*  forcing PPI mode 0 for all, and A, B & C as input.
    the mixed modes 2-0 are not working properly.
*/
	ROM[0x0021] = 0x9b;
	ROM[0x0025] = 0x9b;
}

static DRIVER_INIT(cmv4)
{
	UINT8 *ROM = memory_region(machine, "maincpu");

/*  forcing PPI mode 0 for all, and A, B & C as input.
    the mixed modes 2-0 are not working properly.
*/
	ROM[0x0209] = 0x9b;
	ROM[0x020d] = 0x9b;
}

static DRIVER_INIT(cmast91)
{
	UINT8 *ROM = memory_region(machine, "maincpu");

/*  forcing PPI mode 0 for all, and A, B & C as input.
    the mixed modes 2-0 are not working properly.
*/
	ROM[0x0070] = 0x9b;
	ROM[0x0a92] = 0x9b;
}

static DRIVER_INIT(lucky8a)
{
	UINT8 *ROM = memory_region(machine, "maincpu");

	ROM[0x0010] = 0x21;
}

static DRIVER_INIT( nfb96sea )
{
	int i;
	UINT8 *ROM = memory_region(machine, "maincpu");

	for (i = 0;i < 0x10000;i++)
	{
		UINT8 x = ROM[i];
		switch(i & 7)
		{
			case 0: x = BITSWAP8(x^0x80, 1,6,7,4,5,2,3,0); break;
			case 1: x = BITSWAP8(x^0xa0, 5,6,3,4,1,2,7,0); break;
			case 2: x = BITSWAP8(x^0x02, 5,6,3,4,1,2,7,0); break;
			case 3: x = BITSWAP8(x^0xa0, 3,6,1,4,7,2,5,0); break;
			case 4: x = BITSWAP8(x^0x82, 3,6,1,4,7,2,5,0); break;
			case 5: x = BITSWAP8(x^0x02, 1,6,7,4,5,2,3,0); break;
			case 6: x = BITSWAP8(x^0x08, 3,6,1,4,7,2,5,0); break;
			case 7: x = BITSWAP8(x^0x80, 5,6,3,4,1,2,7,0); break;
		}

		ROM[i] = x;
	}
}


static READ8_HANDLER( fixedvalb4_r )
{
	return 0xb4;
}
static READ8_HANDLER( fixedvala8_r )
{
	return 0xa8;
}
static DRIVER_INIT( schery97 )
{
	int i;
	UINT8 *ROM = memory_region(machine, "maincpu");
	for (i = 0;i < 0x10000;i++)
	{
		UINT8 x = ROM[i];
		switch(i & 0x12)
		{
			case 0x00: x = BITSWAP8(x^0x3e, 1,0,7,6,5,4,3,2); break;
			case 0x02: x = BITSWAP8(x^0x4d, 0,7,6,5,4,3,2,1); break;
			case 0x10: x = BITSWAP8(x^0x24, 2,1,0,7,6,5,4,3); break;
			case 0x12: x = BITSWAP8(x^0xbb, 4,3,2,1,0,7,6,5); break;
		}

		ROM[i] = x;
	}

	memory_install_read8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_IO), 0x1d, 0x1d, 0, 0, fixedvala8_r);
	memory_install_read8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_IO), 0x2a, 0x2a, 0, 0, fixedvalb4_r);
	/* Oki 6295 at 0x20 */
}

static READ8_HANDLER( fixedval38_r )
{
	return 0x38;
}

static DRIVER_INIT( schery97a )
{
	int i;
	UINT8 *ROM = memory_region(machine, "maincpu");
	for (i = 0;i < 0x10000;i++)
	{
		UINT8 x = ROM[i];
		switch(i & 6)
		{
			case 0: x = BITSWAP8(x^0xb9, 4,0,6,7,3,1,5,2); break;
			case 2: x = BITSWAP8(x^0x8f, 6,7,4,0,3,2,1,5); break;
			case 4: x = BITSWAP8(x^0xd2, 3,4,0,2,5,6,1,7); break;
			case 6: x = BITSWAP8(x^0xd1, 6,0,2,1,4,5,3,7); break;
		}

		ROM[i] = x;
	}



	memory_install_read8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_IO), 0x16, 0x16, 0, 0, fixedval38_r);
	/* Oki 6295 at 0x20 */
}

static READ8_HANDLER( fixedvalea_r )
{
	return 0xea;
}

static DRIVER_INIT( skill98 )
{
	int i;
	UINT8 *ROM = memory_region(machine, "maincpu");
	for (i = 0;i < 0x10000;i++)
	{
		UINT8 x = ROM[i];
		switch(i & 0x12)
		{
			case 0x00: x = BITSWAP8(x^0x21, 2,1,0,7,6,5,4,3); break;
			case 0x02: x = BITSWAP8(x^0x45, 2,1,0,7,6,5,4,3); break;
			case 0x10: x = BITSWAP8(x^0x23, 4,3,2,1,0,7,6,5); break;
			case 0x12: x = BITSWAP8(x^0x5b, 4,3,2,1,0,7,6,5); break;
		}

		ROM[i] = x;
	}

	memory_install_read8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_IO), 0x1e, 0x1e, 0, 0, fixedvalea_r);
	/* Oki 6295 at 0x20 */
}

static READ8_HANDLER( fixedval68_r )
{
	return 0x68;
}

static DRIVER_INIT( fb36xc1 )
{
	int i;
	UINT8 *ROM = memory_region(machine, "maincpu");
	for (i = 0;i < 0x10000;i++)
	{
		UINT8 x = ROM[i];

		switch(i & 0x12)
		{
			case 0x00: x = BITSWAP8(x^0xf5, 6,4,3,7,0,1,5,2); break;
			case 0x02: x = BITSWAP8(x^0xe6, 4,6,3,0,7,2,1,5); break;
			case 0x10: x = BITSWAP8(x^0x34, 0,3,5,2,4,6,1,7); break;
			case 0x12: x = BITSWAP8(x^0xc6, 2,0,4,1,6,5,3,7); break;
		}
		ROM[i] = x;
	}

	memory_install_read8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_IO), 0x31, 0x31, 0, 0, fixedval68_r);

}

static READ8_HANDLER( fixedval80_r )
{
	return 0x80;
}

static READ8_HANDLER( fixedval96_r )
{
	return 0x96;
}

static READ8_HANDLER( fixedvalaa_r )
{
	return 0xaa;
}

static DRIVER_INIT( fbse354 )
{
	int i;
	UINT8 *ROM = memory_region(machine, "maincpu");
	for (i = 0;i < 0x10000;i++)
	{
		UINT8 x = ROM[i];

		switch(i & 5)
		{
			case 0: x = BITSWAP8(x^0x6a, 2,1,0,7,6,5,4,3); break;
			case 1: x = BITSWAP8(x^0xcc, 0,7,6,5,4,3,2,1); break;
			case 4: x = BITSWAP8(x^0x8f, 3,2,1,0,7,6,5,4); break;
			case 5: x = BITSWAP8(x^0x93, 4,3,2,1,0,7,6,5); break;
		}
		ROM[i] = x;
	}
	// nfb96b needs both of these
	memory_install_read8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_IO), 0x23, 0x23, 0, 0, fixedval80_r);
	memory_install_read8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_IO), 0x5a, 0x5a, 0, 0, fixedvalaa_r);

	// csel96b
	memory_install_read8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_IO), 0x6e, 0x6e, 0, 0, fixedval96_r);

}

static READ8_HANDLER( fixedvalbe_r )
{
	return 0xbe;
}


static DRIVER_INIT( fbse362 )
{
	int i;
	UINT8 *ROM = memory_region(machine, "maincpu");
	for (i = 0;i < 0x10000;i++)
	{
		UINT8 x = ROM[i];

		switch(i & 5)
		{
			case 0: x = BITSWAP8(x^0xce, 1,0,7,6,5,4,3,2); break;
			case 1: x = BITSWAP8(x^0x9e, 3,2,1,0,7,6,5,4); break;
			case 4: x = BITSWAP8(x^0xc3, 0,7,6,5,4,3,2,1); break;
			case 5: x = BITSWAP8(x^0xdb, 4,3,2,1,0,7,6,5); break;
		}
		ROM[i] = x;
	}

	memory_install_read8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_IO), 0x2e, 0x2e, 0, 0, fixedvalbe_r);

}

static READ8_HANDLER( fixedval90_r )
{
	return 0x90;
}

static READ8_HANDLER( fixedval84_r )
{
	return 0x84;
}

static DRIVER_INIT( rp35 )
{
	int i;
	UINT8 *ROM = memory_region(machine, "maincpu");
	for (i = 0;i < 0x10000;i++)
	{
		UINT8 x = ROM[i];

		switch(i & 3)
		{
			case 0: x = BITSWAP8(x^0x2a, 0,7,6,5,4,3,2,1); break;
			case 1: x = BITSWAP8(x^0x1c, 4,3,2,1,0,7,6,5); break;
			case 2: x = BITSWAP8(x^0x4f, 3,2,1,0,7,6,5,4); break;
			case 3: x = BITSWAP8(x^0x23, 1,0,7,6,5,4,3,2); break;
		}
		ROM[i] = x;
	}

	memory_install_read8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_IO), 0x5e, 0x5e, 0, 0, fixedval84_r);
	memory_install_read8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_IO), 0x36, 0x36, 0, 0, fixedval90_r);
}

static READ8_HANDLER( fixedvalb2_r )
{
	return 0xb2;
}

static DRIVER_INIT( rp36 )
{
	int i;
	UINT8 *ROM = memory_region(machine, "maincpu");
	for (i = 0;i < 0x10000;i++)
	{
		UINT8 x = ROM[i];

		switch(i & 5)
		{
			case 0: x = BITSWAP8(x^0xee, 2,1,0,7,6,5,4,3); break;
			case 1: x = BITSWAP8(x^0x9f, 3,2,1,0,7,6,5,4); break;
			case 4: x = BITSWAP8(x^0xc7, 3,2,1,0,7,6,5,4); break;
			case 5: x = BITSWAP8(x^0xc3, 3,2,1,0,7,6,5,4); break;
		}

		ROM[i] = x;
	}

	memory_install_read8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_IO), 0x34, 0x34, 0, 0, fixedvalb2_r);
}

static READ8_HANDLER( fixedval48_r )
{
	return 0x48;
}

static DRIVER_INIT( rp36c3 )
{
	int i;
	UINT8 *ROM = memory_region(machine, "maincpu");
	for (i = 0;i < 0x10000;i++)
	{
		UINT8 x = ROM[i];

		switch(i & 0xa)
		{
			case 0x0: x = BITSWAP8(x^0xfd, 6,4,0,7,3,1,5,2); break;
			case 0x2: x = BITSWAP8(x^0xee, 4,6,7,0,3,2,1,5); break;
			case 0x8: x = BITSWAP8(x^0x2c, 0,3,4,2,5,6,1,7); break;
			case 0xa: x = BITSWAP8(x^0xd6, 2,0,6,1,4,5,3,7); break;
		}

		ROM[i] = x;
	}

	memory_install_read8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_IO), 0x17, 0x17, 0, 0, fixedval48_r);
}

static READ8_HANDLER( fixedval09_r )
{
	return 0x09;
}

static READ8_HANDLER( fixedval74_r )
{
	return 0x74;
}


static DRIVER_INIT( po33 )
{
	int i;
	UINT8 *ROM = memory_region(machine, "maincpu");
	for (i = 0;i < 0x10000;i++)
	{
		UINT8 x = ROM[i];

		switch(i & 0x14)
		{
			case 0x00: x = BITSWAP8(x^0xde, 2,1,0,7,6,5,4,3); break;
			case 0x04: x = BITSWAP8(x^0x3c, 0,7,6,5,4,3,2,1); break;
			case 0x10: x = BITSWAP8(x^0x2f, 3,2,1,0,7,6,5,4); break;
			case 0x14: x = BITSWAP8(x^0x5b, 4,3,2,1,0,7,6,5); break;
		}

		ROM[i] = x;
	}
	memory_install_read8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_IO), 0x32, 0x32, 0, 0, fixedval74_r);
	memory_install_read8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_IO), 0x12, 0x12, 0, 0, fixedval09_r);
	/* oki6295 at 0x20 */
}

static READ8_HANDLER( fixedval58_r )
{
	return 0x58;
}

static DRIVER_INIT( tc132axt )
{
	int i;
	UINT8 *ROM = memory_region(machine, "maincpu");
	for (i = 0;i < 0x10000;i++)
	{
		UINT8 x = ROM[i];

		switch(i & 0x22)
		{
			case 0x00: x = BITSWAP8(x^0x5f, 6,4,3,7,0,5,2,1); break;
			case 0x02: x = BITSWAP8(x^0xe7, 4,6,3,0,7,5,1,2); break;
			case 0x20: x = BITSWAP8(x^0x18, 0,3,5,2,4,7,1,6); break;
			case 0x22: x = BITSWAP8(x^0x74, 2,0,4,1,6,7,3,5); break;
		}

		ROM[i] = x;
	}

	memory_install_read8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_IO), 0x21, 0x21, 0, 0, fixedval58_r);
}

static READ8_HANDLER( fixedvale4_r )
{
	return 0xe4;
}

static READ8_HANDLER( fixedvalc7_r )
{
	return 0xc7;
}

static DRIVER_INIT( match133 )
{
	int i;
	UINT8 *ROM = memory_region(machine, "maincpu");
	for (i = 0;i < 0x10000;i++)
	{
		UINT8 x = ROM[i];

		switch(i & 0x12)
		{
			case 0x00: x = BITSWAP8(x^0xde, 3,2,1,0,7,6,5,4); break;
			case 0x02: x = BITSWAP8(x^0x3d, 1,0,7,6,5,4,3,2); break;
			case 0x10: x = BITSWAP8(x^0x2f, 4,3,2,1,0,7,6,5); break;
			case 0x12: x = BITSWAP8(x^0x5c, 4,3,2,1,0,7,6,5); break;
		}

		ROM[i] = x;
	}

	memory_install_read8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_IO), 0x16, 0x16, 0, 0, fixedvalc7_r);
	memory_install_read8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_IO), 0x1a, 0x1a, 0, 0, fixedvale4_r);
}

static DRIVER_INIT(cherrys)
{
	int i;
	UINT8 *ROM = memory_region(machine, "maincpu");

	unsigned char rawData[256] = {
		0xCC, 0xCD, 0xCE, 0xCF, 0xC8, 0xC9, 0xCA, 0xCB, 0xC4, 0xC5, 0xC6, 0xC7,
		0xC0, 0xC1, 0xC2, 0xC3, 0xDC, 0xDD, 0xDE, 0xDF, 0xD8, 0xD9, 0xDA, 0xDB,
		0xD4, 0xD5, 0xD6, 0xD7, 0xD0, 0xD1, 0xD2, 0xD3, 0xEC, 0xED, 0xEE, 0xEF,
		0xE8, 0xE9, 0xEA, 0xEB, 0xE4, 0xE5, 0xE6, 0xE7, 0xE0, 0xE1, 0xE2, 0xE3,
		0xFC, 0xFD, 0xFE, 0xFF, 0xF8, 0xF9, 0xFA, 0xFB, 0xF4, 0xF5, 0xF6, 0xF7,
		0xF0, 0xF1, 0xF2, 0xF3, 0x8C, 0x8D, 0x8E, 0x8F, 0x88, 0x89, 0x8A, 0x8B,
		0x84, 0x85, 0x86, 0x87, 0x80, 0x81, 0x82, 0x83, 0x9C, 0x9D, 0x9E, 0x9F,
		0x98, 0x99, 0x9A, 0x9B, 0x94, 0x95, 0x96, 0x97, 0x90, 0x91, 0x92, 0x93,
		0xAC, 0xAD, 0xAE, 0xAF, 0xA8, 0xA9, 0xAA, 0xAB, 0xA4, 0xA5, 0xA6, 0xA7,
		0xA0, 0xA1, 0xA2, 0xA3, 0xBC, 0xBD, 0xBE, 0xBF, 0xB8, 0xB9, 0xBA, 0xBB,
		0xB4, 0xB5, 0xB6, 0xB7, 0xB0, 0xB1, 0xB2, 0xB3, 0x4C, 0x4D, 0x4E, 0x4F,
		0x48, 0x49, 0x4A, 0x4B, 0x44, 0x45, 0x46, 0x47, 0x40, 0x41, 0x42, 0x43,
		0x5C, 0x5D, 0x5E, 0x5F, 0x58, 0x59, 0x5A, 0x5B, 0x54, 0x55, 0x56, 0x57,
		0x50, 0x51, 0x52, 0x53, 0x6C, 0x6D, 0x6E, 0x6F, 0x68, 0x69, 0x6A, 0x6B,
		0x64, 0x65, 0x66, 0x67, 0x60, 0x61, 0x62, 0x63, 0x7C, 0x7D, 0x7E, 0x7F,
		0x78, 0x79, 0x7A, 0x7B, 0x74, 0x75, 0x76, 0x77, 0x70, 0x71, 0x72, 0x73,
		0x0C, 0x0D, 0x0E, 0x0F, 0x08, 0x09, 0x0A, 0x0B, 0x04, 0x05, 0x06, 0x07,
		0x00, 0x01, 0x02, 0x03, 0x1C, 0x1D, 0x1E, 0x1F, 0x18, 0x19, 0x1A, 0x1B,
		0x14, 0x15, 0x16, 0x17, 0x10, 0x11, 0x12, 0x13, 0x2C, 0x2D, 0x2E, 0x2F,
		0x28, 0x29, 0x2A, 0x2B, 0x24, 0x25, 0x26, 0x27, 0x20, 0x21, 0x22, 0x23,
		0x3C, 0x3D, 0x3E, 0x3F, 0x38, 0x39, 0x3A, 0x3B, 0x34, 0x35, 0x36, 0x37,
		0x30, 0x31, 0x32, 0x33
	};

	for (i = 0;i < 0x10000;i++)
	{
		ROM[i] = ROM[i] ^ rawData[i&0xff];
	}

}

/* todo: remove these patches! */
static DRIVER_INIT( unkch1 )
{
	UINT8 *ROM = memory_region(machine, "maincpu");
	ROM[0x9d52] = 0x00;
	ROM[0x9d53] = 0x00;
}

static DRIVER_INIT( unkch3 )
{
	UINT8 *ROM = memory_region(machine, "maincpu");
	ROM[0x9b86] = 0x00;
	ROM[0x9b87] = 0x00;
}

static DRIVER_INIT( unkch4 )
{
	UINT8 *ROM = memory_region(machine, "maincpu");
	ROM[0x9a6e] = 0x00;
	ROM[0x9a6f] = 0x00;
}



/*********************************************
*                Game Drivers                *
**********************************************

       YEAR  NAME       PARENT    MACHINE   INPUT     INIT       ROT    COMPANY              FULLNAME                                      FLAGS              LAYOUT */
GAME(  199?, goldstar,  0,        goldstar, goldstar, goldstar,  ROT0, "IGS",               "Golden Star",                                 0 )
GAME(  199?, goldstbl,  goldstar, goldstbl, goldstar, 0,         ROT0, "IGS",               "Golden Star (Blue version)",                  0 )
GAME(  199?, moonlght,  goldstar, moonlght, goldstar, 0,         ROT0, "bootleg",           "Moon Light (bootleg of Golden Star)",         0 )
GAME(  199?, chrygld,   0,        chrygld,  chrygld,  chrygld,   ROT0, "bootleg",           "Cherry Gold I",                               0 )
GAME(  199?, chry10,    0,        chrygld,  chry10,   chry10,    ROT0, "bootleg",           "Cherry 10 (bootleg with PIC16F84)",           0 )

// are these really dyna, or bootlegs?
GAME(  199?, ncb3,      0,        ncb3,     ncb3,     0,         ROT0, "Dyna",              "Cherry Bonus III (ver.1.40, set 1)",          0 )
GAME(  199?, cb3a,      ncb3,     ncb3,     cb3a,     0,         ROT0, "Dyna",              "Cherry Bonus III (ver.1.40, set 2)",          0 )
GAME(  199?, cb3,       ncb3,     ncb3,     ncb3,     cb3,       ROT0, "Dyna",              "Cherry Bonus III (ver.1.40, encrypted)",      0 )
GAME(  199?, cb3b,      ncb3,     cherrys,  ncb3,     cherrys,   ROT0, "Dyna",              "Cherry Bonus III (alt)",           0 )
GAME(  199?, cb3c,      ncb3,     cb3c,     chrygld,  cb3,       ROT0, "bootleg",           "Cherry Bonus III (alt, set 2)", GAME_NOT_WORKING)

// cherry master hardware has a rather different mem map, but is basically the same
GAME(  198?, cmv801,    0,        cm,       cmv801,   cm,        ROT0, "Corsica",           "Cherry Master (Corsica, ver.8.01)",           0 ) /* says ED-96 where the manufacturer is on some games.. */

// most of these are almost certainly bootlegs, with added features, hacked payouts etc. identifying which are
// the original, unmodified dyna versions is almost impossible due to lack of documentation from back in the day,
// even original boards almost always run modified sets
GAME(  1992, cmv4,      0,        cm,       cmv4,     cmv4,      ROT0, "Dyna",              "Cherry Master (ver.4, set 1)",                0 )
GAME(  1992, cmv4a,     cmv4,     cm,       cmv4,     cmv4,      ROT0, "Dyna",              "Cherry Master (ver.4, set 2)",                GAME_NOT_WORKING )	/* stealth game? */
GAME(  199?, cmwm,      cmv4,     cm,       cmv4,     cmv4,      ROT0, "Dyna",              "Cherry Master (Watermelon bootleg / hack)",   0 )
GAME(  1995, cmfun,     cmv4,     cm,       cmv4,     cmv4,      ROT0, "Dyna",              "Cherry Master (Fun USA v2.5 bootleg / hack)", 0 )
GAME(  1991, cmaster,   0,        cm,       cmaster,  0,         ROT0, "Dyna",              "Cherry Master I (ver.1.01, set 1)",           0 )
GAME(  1991, cmasterb,  cmaster,  cm,       cmasterb, cmv4,      ROT0, "Dyna",              "Cherry Master I (ver.1.01, set 2)",           0 )
GAME(  1991, cmezspin,  cmaster,  cm,       cmasterb, cmv4,      ROT0, "Dyna",              "Cherry Master I (E-Z Spin bootleg / hack)",   0 )
GAME(  1991, cmasterc,  cmaster,  cmasterc, cmasterc, cmv4,      ROT0, "Dyna",              "Cherry Master I (ver.1.01, set 3)",           0 )
GAME(  1991, cmasterbv, cmaster,  cm,       cmasterb, cmv4,      ROT0, "Dyna",              "Cherry Master I (ver.1.01, set 4, with Blitz Poker ROM?)", GAME_NOT_WORKING ) // Cherry Master works, but no idea how to use the Blitz ROM
GAME(  1991, cmasterd,  cmaster,  cm,       cmasterb, cmv4,      ROT0, "Dyna",              "Cherry Master I (ver.1.01, set 5)",           0 )
GAME(  1991, cmastere,  cmaster,  cm,       cmasterb, cmv4,      ROT0, "Dyna",              "Cherry Master I (ver.1.01, set 6)",           0 )
GAME(  1991, cmasterf,  cmaster,  cm,       cmasterb, cmv4,      ROT0, "Dyna",              "Cherry Master I (ver.1.01, set 7)",           0 )

GAME(  199?, jkrmast,   0,        pkrmast,  cmasterb, cmv4,      ROT0, "<unknown>",         "Joker Master",                                GAME_NOT_WORKING ) // incomplete dump + encrypted?
GAME(  199?, pkrmast,   jkrmast,  pkrmast,  cmasterb, cmv4,      ROT0, "<unknown>",         "Poker Master (set 1)",                        GAME_NOT_WORKING ) // incomplete dump + encrypted?
GAME(  199?, pkrmasta,  jkrmast,  pkrmast,  cmasterb, cmv4,      ROT0, "<unknown>",         "Poker Master (set 2)",                        GAME_NOT_WORKING ) // incomplete dump + encrypted?


GAME(  1991, cmast91,   0,        cmast91,  cmast91,  cmast91,   ROT0, "Dyna",              "Cherry Master '91 (ver.1.30)",                0 )
GAME(  1992, cmast92,   0,        cmast91,  cmast91,  cmast91,   ROT0, "Dyna",              "Cherry Master '92",                           GAME_NOT_WORKING ) // no gfx roms are dumped


GAMEL( 1989, lucky8,    0,        lucky8,   lucky8,   0,         ROT0, "Wing Co., Ltd. / GEI", "New Lucky 8 Lines (set 1, W-4)",                           0,                     layout_lucky8 )
GAMEL( 1989, lucky8a,   lucky8,   lucky8,   lucky8a,  lucky8a,   ROT0, "Wing Co., Ltd. / GEI", "New Lucky 8 Lines (set 2, W-4)",                           0,                     layout_lucky8 )
GAMEL( 1989, lucky8b,   lucky8,   lucky8,   ns8lines, 0,         ROT0, "Wing Co., Ltd. / GEI", "New Lucky 8 Lines (set 3, W-4, extended gfx)",             0,                     layout_lucky8 )
GAMEL( 198?, ns8lines,  0,        lucky8,   ns8lines, 0,         ROT0, "<unknown>",            "New Lucky 8 Lines / New Super 8 Lines (W-4)",              0,                     layout_lucky8 )
GAMEL( 198?, ns8linew,  0,        lucky8,   ns8linew, 0,         ROT0, "<unknown>",            "New Lucky 8 Lines / New Super 8 Lines (F-5, Witch Bonus)", 0,                     layout_lucky8 )
GAMEL( 198?, kkojnoli,  0,        kkojnoli, kkojnoli, 0,         ROT0, "hack",                 "Kkoj Noli (Kill the Bees)",                                GAME_IMPERFECT_COLORS, layout_lucky8 )
GAME(  198?, ladylinr,  0,        ladylinr, ladylinr, 0,         ROT0, "TAB Austria",          "Lady Liner",                                               0 )
GAME(  198?, wcat3,     0,        wcat3,    lucky8,   0,         ROT0, "E.A.I.",               "Wild Cat 3",                                               GAME_NOT_WORKING )

GAME(  1992, magical,   0,        magical,  magical,  0,         ROT0, "Pal Company / Micro Manufacturing Inc.", "Magical Odds (set 1)",               GAME_WRONG_COLORS | GAME_IMPERFECT_GRAPHICS )
GAME(  1992, magicala,  magical,  magical,  magical,  0,         ROT0, "Pal Company / Micro Manufacturing Inc.", "Magical Odds (set 2)",               GAME_WRONG_COLORS | GAME_IMPERFECT_GRAPHICS )
GAME(  1991, mtonic,    magical,  magical,  mtonic,   mtonic,    ROT0, "Pal Company",                            "Magical Odds (set 3, alt hardware)", GAME_WRONG_COLORS | GAME_NOT_WORKING |GAME_NO_SOUND) // Magical Odds logo / company in GFX


/* --- Amcoe games --- */

/*    YEAR  NAME       PARENT    MACHINE   INPUT      INIT       ROT    COMPANY    FULLNAME                                                FLAGS  */

GAME( 1997, schery97,  0,        amcoe1,   schery97,  schery97,  ROT0, "Amcoe",   "Skill Cherry '97 (ver. sc3.52)",                        0 )	/* running in CB hardware */
GAME( 1997, schery97a, schery97, amcoe1,   schery97,  schery97a, ROT0, "Amcoe",   "Skill Cherry '97 (ver. sc3.52c4)",                      0 )	/* running in C4 hardware */
GAME( 1998, skill98,   0,        amcoe1,   schery97,  skill98,   ROT0, "Amcoe",   "Skill '98 (ver. s98-1.33)",                             0 )
GAME( 1997, pokonl97,  0,        amcoe1,   pokonl97,  po33,      ROT0, "Amcoe",   "Poker Only '97 (ver. 3.3)",                                        0 )	/* ver. 3.3 */
GAME( 1998, match98,   0,        amcoe1a,  match98,   match133,  ROT0, "Amcoe",   "Match '98 (ver. 1.33)",                                 0 )

GAME( 1996, nfb96,     0,        amcoe2,   nfb96,     fb36xc1,   ROT0, "Amcoe",   "New Fruit Bonus '96 Special Edition (set 1)",           0 )	/* ver. 00-3.62 */
GAME( 1996, nfb96a,    nfb96,    amcoe2,   nfb96,     fb36xc1,   ROT0, "Amcoe",   "New Fruit Bonus '96 Special Edition (set 2)",           0 )	/* ver. 02-3.63 */
GAME( 1996, nfb96b,    nfb96,    amcoe2,   nfb96,     fbse354,   ROT0, "Amcoe",   "New Fruit Bonus '96 Special Edition (set 3)",           0 )	/* ver. 00-3.54 */
GAME( 1996, nfb96c,    nfb96,    amcoe2,   nfb96,     fbse362,   ROT0, "Amcoe",   "New Fruit Bonus '96 Special Edition (set 4)",           0 )	/* ver. 00-3.62 ??? */
GAME( 1996, nfb96txt,  nfb96,    amcoe2,   nfb96tx,   tc132axt,  ROT0, "Amcoe",   "New Fruit Bonus '96 Special Edition (set 5, Texas XT)", 0 )	/* ver. tf1.22axt */

GAME( 1996, roypok96,  0,        amcoe2,   roypok96,  rp35,      ROT0, "Amcoe",   "Royal Poker '96 (set 1)",                               0 )	/* ver. 97-3.5 */
GAME( 1996, roypok96a, roypok96, amcoe2,   roypok96a, rp36,      ROT0, "Amcoe",   "Royal Poker '96 (set 2)",                               0 )	/* ver. 98-3.6 */
GAME( 1996, roypok96b, roypok96, amcoe2,   roypok96a, rp36c3,    ROT0, "Amcoe",   "Royal Poker '96 (set 3)",                               0 )	/* ver. 98-3.6 ??? */

GAME( 1996, nc96,      nfb96,    amcoe2,   nfb96,     fb36xc1,   ROT0, "Amcoe",   "New Cherry '96 Special Edition (set 1)",                0 )
GAME( 1996, nc96a,     nfb96,    amcoe2,   nfb96,     fb36xc1,   ROT0, "Amcoe",   "New Cherry '96 Special Edition (set 2)",                0 )
GAME( 1996, nc96b,     nfb96,    amcoe2,   nfb96,     fbse354,   ROT0, "Amcoe",   "New Cherry '96 Special Edition (set 3)",                0 )
GAME( 1996, nc96c,     nfb96,    amcoe2,   nfb96,     fbse362,   ROT0, "Amcoe",   "New Cherry '96 Special Edition (set 4)",                0 )
GAME( 2000, nc96txt,   nfb96,    amcoe2,   nfb96tx,   tc132axt,  ROT0, "Amcoe",   "New Cherry '96 Special Edition (set 5, Texas XT)",      0 )	/* ver. tc1.32axt */


/* these all appear to be graphic hacks of 'New Fruit Bonus '96', they can run with the same program rom
   some sets are messy and appear to have mismatched graphic roms, they needed to be sorted out properly
*/
/*    YEAR  NAME       PARENT    MACHINE   INPUT      INIT       ROT    COMPANY                 FULLNAME                                                FLAGS  */
GAME( 1996, nfb96se,   nfb96,    amcoe2,   nfb96bl,   0,         ROT0, "bootleg",              "New Fruit Bonus '96 Special Edition (bootleg, set 1)",  0 )
GAME( 1996, nfb96sea,  nfb96,    amcoe2,   nfb96bl,   nfb96sea,  ROT0, "bootleg",              "New Fruit Bonus '96 Special Edition (bootleg, set 2)",  GAME_WRONG_COLORS ) // encrypted program
GAME( 1996, nfb96seb,  nfb96,    amcoe2,   nfb96bl,   0,         ROT0, "bootleg",              "New Fruit Bonus '96 Special Edition (bootleg, set 3)",  GAME_WRONG_COLORS )
GAME( 2002, carb2002,  nfb96,    amcoe2,   nfb96bl,   0,         ROT0, "bootleg",              "Carriage Bonus 2002 (bootleg)",                         GAME_WRONG_COLORS )
GAME( 2003, carb2003,  nfb96,    amcoe2,   nfb96bl,   0,         ROT0, "bootleg",              "Carriage Bonus 2003 (bootleg)",                         GAME_WRONG_COLORS )

GAME( 2003, nfm,       0,        nfm,      nfb96bl,   0,         ROT0, "Ming-Yang Electronic", "New Fruit Machine (Ming-Yang Electronic)",              GAME_NOT_WORKING )

// these have 'cherry 1994' in the program roms, but also "Super Cherry / New Cherry Gold '99" probably hacks of a 1994 version of Cherry Bonus / Cherry Master (Super Cherry Master?)
GAME( 1999, unkch1,   0,         unkch,    unkch,  unkch1,    ROT0, "bootleg",              "New Cherry Gold '99 (bootleg of Super Cherry Master) (set 1)", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 1999, unkch2,   unkch1,    unkch,    unkch,  unkch1,    ROT0, "bootleg",              "Super Cherry Gold (bootleg of Super Cherry Master)",           GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 1999, unkch3,   unkch1,    unkch,    unkch,  unkch3,    ROT0, "bootleg",              "New Cherry Gold '99 (bootleg of Super Cherry Master) (set 2)", GAME_NOT_WORKING|GAME_NO_SOUND ) // cards have been hacked to look like barrels, girl removed?
GAME( 1999, unkch4,   unkch1,    unkch,    unkch,  unkch4,    ROT0, "bootleg",              "Grand Cherry Master (bootleg of Super Cherry Master)",         GAME_NOT_WORKING|GAME_NO_SOUND ) // by 'toy system' hungary



/* possible stealth sets:

 - cmv4a    ---> see the 1fxx zone. put a bp in 1f9f to see the loop.

*/
