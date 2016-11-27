/******************************************************************************


    MAGIC FLY
    ---------

    Driver by Roberto Fresca.


    Games running on this hardware:

    * Magic Fly (P&A Games),    198?
    * 7 e Mezzo (Unknown),      198?


    **** NOTE ****
    This hardware was clearly designed for poker games.
    You can find a complete hardware & software analysis here:
    http://mamedev.emulab.it/robbie


*******************************************************************************


    *** Hardware notes ***


    - CPU:  1x R6502P.

    - CRTC: 1x MC6845P.

    - RAM:  1x MK48Z02B-20.  CMOS 2K*8 zeropower SRAM (NVRAM).
            2x HM6116LP-4.   2K*8 SRAM (Video RAM, only 1st half used).

    - CLK:  1x crystal @ 10.000 MHz.

    - ROMs: 1x AM27128.      (NS3.1)
            2x SEEQ DQ2764.  (1, 2)
            1x SGS M2764.    (NS1)

    - PLDs: 1x PAL16R4A.     (read protected)

    - SOUND: 1x delta-sigma DAC (1-bit/Bitstream).
             1x TDA2002 (8W. car radio audio amplifier)


    Other components:

    - 3x OUAZ SS-112D  (relay 1 pole 12v)
    - 1x 4 DIP switches
    - 1x 30x2 edge connector
    - 1x 10 pins male connector
    - 2x trimmer (audio?)


    PCB Layout:
     _________________________________________________________________
    |                                                                 |
    |                                                                 |
    |                                    _________   _________        |
    |                                   | 74LS08N | | 74LS32  |       |
    |                                   |_________| |_________|       |
    |                                    _________   _________        |
    |                                   | 74LS138 | | 74HC00  |       |
    |                                   |_________| |_________|       |
    |    ______________                 ______________________        |
    |   |              |               |                      |   ____|
    |   | MK48Z02B-20  |               |       R6502P         |  |
    |   |______________|               |______________________|  |
    |  ________________   _________     ______________________   |
    | |                | | 74LS157 |   |                      |  |____
    | |    AM27128     | |_________|   |       MC6845P        |   ____|
    | |________________|  _________    |______________________|   ____|
    |                    | 74LS157 |      ________   _________    ____|
    |                    |_________|     | 74LS14 | | 74LS374 |   ____|
    |  ____________       _________      |________| |_________|   ____|
    | | 74LS245    |     | 74LS157 |                 _________    ____|
    | |____________|     |_________|                | 74HC244 |   ____|
    |  ____________       _________                 |_________|   ____|
    | | 74LS245    |     | 74LS32  |      _______                 ____|  30x2
    | |____________|     |_________|     | | | | |                ____|  connector
    |  ______________                    |4|3|2|1|                ____|
    | | HM6116       |                   |_|_|_|_|                ____|
    | | o MSM2128    |                                            ____|
    | |______________|                   DIP SW x4                ____|
    |  ______________                                             ____|
    | | HM6116       |    ________       _________                ____|
    | | o MSM2128    |   | 74LS08 |     | 74LS174 |               ____|
    | |______________|   |________|     |_________|               ____|
    |  ________________   __________           ______             ____|
    | |                | | PAL16R4A |         | TDA  |-           ____|
    | |     2764       | |__________|         | 2002 |-           ____|
    | |________________|  __________          |______|-           ____|
    |  ________________  | 74LS166  |                             ____|
    | |                | |__________|      _                     |
    | |     2764       |  __________     /   \                   |
    | |________________| | 74LS166  |   | pot |                  |____
    |  ________________  |__________|    \ _ /                      __|
    | |                |  __________       _________         ______|  |
    | |     2764       | | 74LS166  |     | 74LS05  |   _   |ss112d|8 |  10
    | |________________| |__________|     |_________| /   \ |______|8 |  pins
    |  ________  ______   __________       _________ | pot ||ss112d|8 |  male
    | | 74LS04 || osc. | | 74LS193  |     | 74LS86  | \ _ / |______|8 |  connector
    | |________||10 MHz| |__________|     |_________|       |ss112d|8 |
    |           |______|                                    |______|__|
    |_________________________________________________________________|


    Pinouts (from 7mezzo pinout sheet)
    ----------------------------------

    *********** Edge connector ************

    solder side    connector    parts side

    GND               30        GND
    +10v.             29        +10v.
    +10v.             28        +10v.
    unused            27        unused
    unused            26        unused
    GND               25        GND
    +12v.             24        +12v.
    +12v.             23        +12v.
    unused            22        unused
    common C (3)      21        common A (1)
    common D (4)      20        common B (2)
    DEAL              19        DOUBLE
    HOLD 1            18        (unreadable)
    HOLD 2            17        HOLD 5
    HOLD 3            16        HOLD 4
    METER             15        BET
    COUPON            14
                      13        COIN 1
    (unreadable)      12        COIN 2
    TAKE              11        PAY
    SMALL (play1)     10        BIG (play3)
    unused            09        unused
    unused            08        unused
    unused            07        unused
    (unreadable)      06        (unreadable)
    sync              05        (unreadable)
    GND               04        GND
    speaker+          03        speaker+
    speaker- (GND)    02        speaker- (GND)
    +5v.              01        +5v.

    (1) = DOUBLE, DEAL, (unreadable), BET, METER
    (2) = TAKE, SMALL, BIG, PAY
    (3) = HOLD 1, HOLD 2, HOLD 3, HOLD 4, HOLD 5
    (4) = COIN 1, COIN 2, COUPON


    **** Pins connector ****

    pin 01: (soldered to pin 05)
    pin 02:
    pin 03:
    pin 04:
    pin 05: (soldered to pin 01)
    pin 06: (unreadable)
    pin 07: (unreadable)
    pin 08: (unreadable)
    pin 09: (unreadable)
    pin 10: common (GND)


*******************************************************************************


    Memory Map (magicfly)
    ---------------------

    $0000 - $00FF    RAM    ; Zero page (pointers and registers) (NVRAM).

        ($0D)            ; Incremented each time a NMI is triggered.
        ($1D)            ; In case of 0x00, NMI do nothing and return.
        ($11)            ; Store lenghts for text handling.
        ($12)            ; Store values to be written in color RAM.
        ($13 - $14)      ; Pointer to text offset.
        ($15 - $16)      ; Pointer to video RAM.
        ($17 - $18)      ; Pointer to color RAM.
        ($19)            ; Program loops waiting for a value to be written here through NMI (see code at $CA71).
        ($1E - $1F)      ; Decrement. Looks as a sort of counter to temporary freeze graphics.
        ($22 - $25)      ; 4-bytes buffer to compare/operate with other segments.
        ($2E - $31)      ; 4-bytes buffer to compare/operate with other segments.
        ($32 - $35)      ; 4-bytes buffer to compare/operate with other segments.
        ($39)            ; Store bits 4, 5 and 6 of content to be ORed with $94 and written to input selector at $3000.
        ($3A - $3D)      ; Store new read values from $2800 (input port) through NMI.
        ($3F - $42)      ; Store old read values from $2800 (input port) through NMI.
        ($56 - $58)      ; Store credits value (shown in the game under "Secondi").
        ($5A - $5C)      ; Store bonus value (shown in the game under "Credits").
        ($5E - $5F)      ; Store values to be written in video and color ram, respectively.
        ($63 - $63)      ; Store position of marker/cursor (0-4).
        ($66 - $66)      ; Store number of baloon to be risen.
        ($67 - $67)      ; Program compare the content with 0x02, 0x03, 0x04, 0x05 and 0xE1.
                         ; If 0xE1 is found here, the machine hangs showing "I/O ERROR" (see code at $C1A2).
        ($6A - $6B)      ; Balloon #1 secondi (bet).
        ($6C - $6D)      ; Balloon #2 secondi (bet).
        ($6E - $6F)      ; Balloon #3 secondi (bet).
        ($70 - $71)      ; Balloon #4 secondi (bet).
        ($72 - $73)      ; Balloon #5 secondi (bet).
        ($94)            ; Store bit 7 of contents to be ORed with $39 and written to output port ($3000).
        ($96 - $98)      ; Store values from content of $2800 (input port) & 0x80, & 0x40, & 0x10.
        ($9B - $A8)      ; Text scroll buffer.

    $0100 - $01FF    RAM    ; 6502 Stack Pointer (NVRAM).

    $0200 - $07FF    RAM    ; General purpose RAM (NVRAM).

    $0800 - $0801    MC6845 ; MC6845 use $0800 for register addressing and $0801 for register values (see code at $CE86).

                     *** MC6845 init ***

                     Register:   00    01    02    03    04    05    06    07    08    09    10    11    12    13    14    15    16    17
                     Value:     0x27  0x20  0x23  0x03  0x1F  0x04  0x1D  0x1E  0x00  0x07  0x00  0x00  0x00  0x00  0x00  0x00  0x00  0x00.

    $1000 - $13FF    Video RAM  ; Initialized in subroutine starting at $CF83, filled with value stored in $5E.

    $1800 - $1BFF    Color RAM  ; Initialized in subroutine starting at $CF83, filled with value stored in $5F.
                                ; (In 7mezzo is located at $CB13 using $64 and $65 to store video and color ram values.)

    $2800 - $2800    Input port ; Multiplexed input port (code at $CE96).
                                ; NMI routine read from here and store new values in $3A-$3D and copy old ones to $3F-$42.
                                ; Code accept only bits 0-1-2-3-5 as valid inputs. For DIP switches, only bits 4-6-7 are valid.
                                ; If an invalid bit is activated, it will produce an I/O error. (code at $CD0C)

    $3000 - $3000    Output port:

                         Input selector,  ; NMI writes the lower 4 bits to select input.
                         Counters.        ; Bits 4-5-6 are used for Coin1, Coin2, and Payout counters.
                         Sound DAC,       ; Bit 7 is used to transmit DAC data.

    $C000 - $FFFF    ROM space       ; Program ROMs.


*******************************************************************************


    After check the last bit of $1800, code jump into a loop ($DA30)...

    BEHAVIOUR OF BOOT CHECK (magicfly):

    1) Fill the video RAM with spaces (0x20), and color RAM with 0x15.
    2) Check bit 7 of $1800 (video RAM, 1st offset) if activated.
    3) If true, go to $DA30 (incremented fill infinite loop).
    4) If not, fill the video RAM with spaces (0x20), and color RAM with 0x1F.
    5) Check bit 7 of $1800 (video RAM, 1st offset) if activated.
    6) If not, go to $DA30 (incremented fill infinite loop).
    7) If true, returns and continue to NORMAL GAME.

    Since bits 0-2 are for regular colors, seems that bit 3 in color RAM
    (bit 2 for 7mezzo) is mirrored to bit 7 through a kind of device.

    This is the only explanation I found to allow a normal boot, and seems to be
    created as a protection method that don't allow owners to use a ROM-swap on
    his boards, converting one game to another.


*******************************************************************************


    Game Notes
    ----------

    Settings (valid for both games):

    Press F2 to enter the input test (game should be without credits).
    In the input test, you can exit with BET + DEAL, or access the bookkeeping
    mode pressing BET + DEAL again. In the bookkeping mode keep pressed the PAYOUT
    key and press BET to change the percentage (all books will be erased). Press
    again BET + DEAL to exit to a quick RAM & sound test, and then to game mode.


    * Magic Fly

    How to play...

    Insert coins to get credits.
    Choose a balloon to raise, and select it with SELECT button. Bet on other balloons
    using the SELECT/BET key (to bet 1 credit) or BETx10 (to bet 10 credits). Once done,
    press the DEAL/LAST BET key to start the game. All balloons will explode revealing
    numbers. The last one to explode will be the raised one. If your number(s) are higher
    than the one hidden in the raised balloon, you'll win!

    You can repeat the last bet pressing the DEAL/LAST BET key.


    * 7 e Mezzo

    This game is a sort of blackjack, but using a spanish cards set. The objetive is
    to get 7 and half points (as 21 in blackjack). If you got more than 7 and half,
    you're busted.

    All cards have their own value, except for 10, 11 and 12 that are half point.
    Special hands have their own price.

    Sun 7 + sun king = 100 (by credit).
    Any 7 + any king =  16 (by credit).
    Any 7 and half   =   8 (by credit).
    Pass             =   2 (by credit).


    How to play...

    Insert coins to get credits.
    Bet using the BET button. Press DEAL to get the cards.
    You can hit a new card pressing DEAL, or stop pressing STAND. The dealer will hit
    cards to the maximum allowed. If you beat the dealer, you'll be asked to enter the
    double-up game. Press DOUBLE to go on, or TAKE to get the credits.

    In the double-up game, a covered card should be shown. Press BIG or SMALL to get
    your chance...



*******************************************************************************


    Driver updates:


    [2006-07-07]
    - Initial release.

    [2006-07-11]
    - Corrected the total number of chars to decode by rom.
    - Fixed the graphics offset for the text layer.
    - Adjusted the gfx rom region bounds properly.

    [2006-07-21]
    - Rewrote the technical info.
    - Removed fuse maps and unaccurate things.
    - Added new findings, pinouts, and pieces of code.
    - Confirmed and partially mapped one input port.
    - Added a little patch to pass over some checks (for debugging purposes).

    [2006-07-26]
    - Figured out the MC6845 (mapped at $0800-$0801).
    - Fixed the screen size based on MC6845 registers.
    - Fixed the visible area based on MC6845 registers.
    - Corrected the gfx rom region.
    - Solved the NMI/vblank issue. Now attract works.
    - Changed CPU clock to 625 kHz. (text scroll looks so fast with the former value)
    - Added new findings to the technical notes.
    - Added version/revision number to magicfly.
    - Marked magicfly PAL as NO_DUMP (read protected).
    - Added flags GAME_IMPERFECT_GRAPHICS and GAME_WRONG_COLORS.

    [2006-08-06]
    - Figured out how the gfx banks works.
    - Fixed the gfx layers.
    - Fixed the gfx decode.
    - Removed flag GAME_IMPERFECT_GRAPHICS.

    [2007-05-12]
    - Figured out how the protection works.
    - Removed the hacks/patchs that formerly allow boot the games.
    - Figured out how the buffered inputs works.
    - Demuxed all inputs for both games.
    - Unified the memory maps from both games.
    - Added NVRAM support to both games.
    - Crystal documented via #define.
    - CPU clock derived from #defined crystal value.
    - Changed CPU clock to 833 kHz.
    - Mapped DIP switches.
    - Found the maximum bet DIP switch in magicfly.
    - Removed flag GAME_NOT_WORKING for both games.
    - Managed the planes to get the 3bpp GFX colors accurate.
    - Renamed the ROMs acording to pcb pictures and ROM contents.
    - Cleaned up and optimized the driver.
    - Reworked/updated technical notes.

    [2008-03-14]
    - Completed the component list & PCB layout.
    - Added technical references to register $63 (magicfly).
    - Switched crystal to new predefined format.

    [2008-10-25]
    - Added sound support to magicfly and 7mezzo.
    - Hooked coin counters to magicfly and 7mezzo.
    - Inverted the graphics banks to be straight with the hardware accesses.
    - Updated the memory map description and technical notes.
    - Added game notes and documented the test/settings/bookkeeping modes.


    TODO:

    - Correct colors. (where is the palette?)
    - Confirm the CPU clock (there is some lag in MF controls).
    - Split the driver.


*******************************************************************************/


#define MASTER_CLOCK	XTAL_10MHz

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "video/mc6845.h"
#include "sound/dac.h"


/*************************
*     Video Hardware     *
*************************/

static UINT8 *videoram;
static UINT8 *colorram;
static tilemap_t *bg_tilemap;

static WRITE8_HANDLER( magicfly_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

static WRITE8_HANDLER( magicfly_colorram_w )
{
	colorram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

static TILE_GET_INFO( get_magicfly_tile_info )
{
/*  - bits -
    7654 3210
    ---- -xxx   Tiles color.
    ---- x---   Seems to be a kind of attribute (maybe related to color). Not totally understood yet.
    ---x ----   Tiles bank.
    -xx- ----   Aparently not used.
    x--- ----   Mirrored from bit 3. The code check this one to boot the game.

*/
	int attr = colorram[tile_index];
	int code = videoram[tile_index];
	int bank = (attr & 0x10) >> 4;   /* bit 4 switch the gfx banks */
	int color = attr & 0x07;         /* bits 0-2 for color */

    /* Seems that bit 7 is mirrored from bit 3 to have a normal boot */
    /* Boot only check the first color RAM offset */

	colorram[0] = colorram[0] | ((colorram[0] & 0x08) << 4);	/* only for 1st offset */
	//colorram[tile_index] = attr | ((attr & 0x08) << 4);         /* for the whole color RAM */

	SET_TILE_INFO(bank, code, color, 0);
}

static VIDEO_START(magicfly)
{
	bg_tilemap = tilemap_create(machine, get_magicfly_tile_info, tilemap_scan_rows, 8, 8, 32, 29);
}

static TILE_GET_INFO( get_7mezzo_tile_info )
{
/*  - bits -
    7654 3210
    ---- -xxx   Tiles color.
    ---- x---   Seems to be a kind of attribute (maybe related to color). Not totally understood yet.
    ---x ----   Tiles bank.
    -xx- ----   Aparently not used.
    x--- ----   Mirrored from bit 2. The code check this one to boot the game.

*/
	int attr = colorram[tile_index];
	int code = videoram[tile_index];
	int bank = (attr & 0x10) >> 4;    /* bit 4 switch the gfx banks */
	int color = attr & 0x07;          /* bits 0-2 for color */

    /* Seems that bit 7 is mirrored from bit 2 to have a normal boot */
    /* Boot only check the first color RAM offset */

	colorram[0] = colorram[0] | ((colorram[0] & 0x04) << 5);	/* only for 1st offset */
	//colorram[tile_index] = attr | ((attr & 0x04) << 5);         /* for the whole color RAM */

	SET_TILE_INFO(bank, code, color, 0);
}

static VIDEO_START( 7mezzo )
{
	bg_tilemap = tilemap_create(machine, get_7mezzo_tile_info, tilemap_scan_rows, 8, 8, 32, 29);
}

static VIDEO_UPDATE( magicfly )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	return 0;
}

static PALETTE_INIT( magicfly )
{
	int i;

	for (i = 0; i < 0x100; i += 0x20)
	{
		/* 1st gfx bank */
		palette_set_color(machine, i + 0, MAKE_RGB(0x00, 0x00, 0x00));
		palette_set_color(machine, i + 2, MAKE_RGB(0x00, 0x00, 0x00));
		palette_set_color(machine, i + 4, MAKE_RGB(0x00, 0x00, 0x00));
		palette_set_color(machine, i + 6, MAKE_RGB(0x00, 0x00, 0x00));
		palette_set_color(machine, i + 10, MAKE_RGB(0x00, 0x00, 0x00));
		palette_set_color(machine, i + 11, MAKE_RGB(0x00, 0xff, 0x00));
		palette_set_color(machine, i + 12, MAKE_RGB(0x00, 0x00, 0x00));
		palette_set_color(machine, i + 14, MAKE_RGB(0x00, 0x00, 0x00));

		/* 2nd gfx bank */
		palette_set_color(machine, i + 22, MAKE_RGB(0xe0, 0xe0, 0xe0));
		palette_set_color(machine, i + 23, MAKE_RGB(0xff, 0xff, 0xff));
	}
}


/******************************
*         R/W Handlers        *
******************************/

static int input_selector = 0;

static READ8_HANDLER( mux_port_r )
{
	switch( input_selector )
	{
		case 0x01: return input_port_read(space->machine, "IN0-0");
		case 0x02: return input_port_read(space->machine, "IN0-1");
		case 0x04: return input_port_read(space->machine, "IN0-2");
		case 0x08: return input_port_read(space->machine, "IN0-3");
		case 0x00: return input_port_read(space->machine, "DSW0");
	}
	return 0xff;
}

static WRITE8_HANDLER( mux_port_w )
{
/*  - bits -
    7654 3210
    ---- xxxx   Input selector.
    ---x ----   Coin 2.
    --x- ----   Payout.
    -x-- ----   Coin 1.
    x--- ----   Sound DAC.

*/
	input_selector = data & 0x0f;	/* Input Selector */

	dac_data_w(space->machine->device("dac"), data & 0x80);		/* Sound DAC */

	coin_counter_w(space->machine, 0, data & 0x40);	/* Coin1 */
	coin_counter_w(space->machine, 1, data & 0x10);	/* Coin2 */
	coin_counter_w(space->machine, 2, data & 0x20);	/* Payout */
}


/*************************
* Memory map information *
*************************/

static ADDRESS_MAP_START( magicfly_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_BASE_SIZE_GENERIC(nvram)    /* MK48Z02B NVRAM */
	AM_RANGE(0x0800, 0x0800) AM_DEVWRITE("crtc", mc6845_address_w)
	AM_RANGE(0x0801, 0x0801) AM_DEVREADWRITE("crtc", mc6845_register_r, mc6845_register_w)
	AM_RANGE(0x1000, 0x13ff) AM_RAM_WRITE(magicfly_videoram_w) AM_BASE(&videoram)	/* HM6116LP #1 (2K x 8) RAM (only 1st half used) */
	AM_RANGE(0x1800, 0x1bff) AM_RAM_WRITE(magicfly_colorram_w) AM_BASE(&colorram)	/* HM6116LP #2 (2K x 8) RAM (only 1st half used) */
	AM_RANGE(0x2800, 0x2800) AM_READ(mux_port_r)	/* multiplexed input port */
	AM_RANGE(0x3000, 0x3000) AM_WRITE(mux_port_w)	/* output port */
	AM_RANGE(0xc000, 0xffff) AM_ROM					/* ROM space */
ADDRESS_MAP_END


/*************************
*      Input ports       *
*************************/

static INPUT_PORTS_START( magicfly )
/*  Multiplexed 4 x 5 bits.
    Code accept only bits 0, 1, 2, 3 and 5 as valid. */
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* present in the input test */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* present in the input test */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* present in the input test */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* present in the input test */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* present in the input test */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Payout") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* present in the input test */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* present in the input test */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* present in the input test */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* present in the input test */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* present in the input test */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Deal / Last Bet") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Cancel") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Bet10") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Bet1/Select") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW0")    /* Only 4 phisical DIP switches (valid bits = 4, 6, 7) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x10, 0x10, "Maximum Bet" )
	PORT_DIPSETTING(    0x10, "20" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )	/* invalid - don't change */
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( 7mezzo )
/*  Multiplexed 4 x 5 bits.
    Code accept only bits 0, 1, 2, 3 and 5 as valid. */
	PORT_START("IN0-0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* present in the input test */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* present in the input test */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* present in the input test */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Big") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Small") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Payout") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Take") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* present in the input test */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* present in the input test */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* present in the input test */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* present in the input test */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* present in the input test */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* present in the input test */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Deal / Last Bet") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Stand") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Double") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Bet") PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW0")    /* Only 4 phisical DIP switches (valid bits = 4, 6, 7) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )	/* invalid - don't change */
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


/*************************
*    Graphics Layouts    *
*************************/

static const gfx_layout tilelayout =
{
	8, 8,
	256,
	3,
	{ 0, 0x2800*8, 0x4800*8 },	/* bitplanes are separated. */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout charlayout =
{
	8, 8,
	256,
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


/******************************
* Graphics Decode Information *
******************************/

static GFXDECODE_START( magicfly )
	GFXDECODE_ENTRY( "gfx1", 0x1000, tilelayout, 16, 16 )
	GFXDECODE_ENTRY( "gfx1", 0x1800, charlayout, 0, 16 )
GFXDECODE_END


/************************
*    CRTC Interface    *
************************/

static const mc6845_interface mc6845_intf =
{
	"screen",	/* screen we are acting on */
	8,			/* number of pixels per video memory address */
	NULL,		/* before pixel update callback */
	NULL,		/* row update callback */
	NULL,		/* after pixel update callback */
	DEVCB_NULL,	/* callback for display state changes */
	DEVCB_NULL,	/* callback for cursor state changes */
	DEVCB_NULL,	/* HSYNC callback */
	DEVCB_NULL,	/* VSYNC callback */
	NULL		/* update address callback */
};


/*************************
*    Machine Drivers     *
*************************/

static MACHINE_DRIVER_START( magicfly )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M6502, MASTER_CLOCK/12)	/* guess */
	MDRV_CPU_PROGRAM_MAP(magicfly_map)
	MDRV_CPU_VBLANK_INT("screen", nmi_line_pulse)

	MDRV_NVRAM_HANDLER(generic_0fill)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE((39+1)*8, (31+1)*8)				/* Taken from MC6845 init, registers 00 & 04. Normally programmed with (value-1). */
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 29*8-1)	/* Taken from MC6845 init, registers 01 & 06. */

	MDRV_GFXDECODE(magicfly)
	MDRV_PALETTE_LENGTH(256)
	MDRV_PALETTE_INIT(magicfly)

	MDRV_VIDEO_START(magicfly)
	MDRV_VIDEO_UPDATE(magicfly)

	MDRV_MC6845_ADD("crtc", MC6845, MASTER_CLOCK/16, mc6845_intf) /* guess */

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("dac", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( 7mezzo )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(magicfly)

	/* video hardware */
	MDRV_VIDEO_START(7mezzo)

MACHINE_DRIVER_END


/*************************
*        Rom Load        *
*************************/

ROM_START( magicfly )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "magicfly3_3.bin",	0xc000, 0x4000, CRC(c29798d5) SHA1(bf92ac93d650398569b3ab79d01344e74a6d35be) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "magicfly2.bin",	0x0000, 0x2000, CRC(3596a45b) SHA1(7ec32ec767d0883d05606beb588d8f27ba8f10a4) )
	ROM_LOAD( "magicfly1.bin",	0x2000, 0x2000, CRC(724d330c) SHA1(cce3923ce48634b27f0e7d29979cd36e7394ab37) )
	ROM_LOAD( "magicfly0.bin",	0x4000, 0x2000, CRC(44e3c9d6) SHA1(677d25360d261bf2400f399b8015eeb529ad405e) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "pal16r4a-magicfly.bin",	0x0000, 0x0104, NO_DUMP )	/* PAL is read protected */

ROM_END

ROM_START( 7mezzo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ns3_1.bin",	0xc000, 0x4000, CRC(b1867b76) SHA1(eb76cffb81c865352f4767015edade54801f6155) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "ns2.bin",	0x0000, 0x2000, CRC(7983a41c) SHA1(68805ea960c2738d3cd2c7490ffed84f90da029b) )    /* Renamed as ns2.bin regarding pcb location and content */
	ROM_LOAD( "ns1.bin",	0x2000, 0x2000, CRC(a6ada872) SHA1(7f531a76e73d479161e485bdcf816eb8eb9fdc62) )
	ROM_LOAD( "ns0.bin",	0x4000, 0x2000, CRC(e04fb210) SHA1(81e764e296fe387daf8ca67064d5eba2a4fc3c26) )    /* Renamed as ns0.bin regarding pcb location and content */

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "pal16r4a-7mezzo.bin",	0x0000, 0x0104, CRC(61ac7372) SHA1(7560506468a7409075094787182ded24e2d0c0a3) )
ROM_END


/*************************
*      Game Drivers      *
*************************/

/*    YEAR  NAME      PARENT  MACHINE   INPUT     INIT   ROT    COMPANY      FULLNAME    FLAGS... */
GAME( 198?, magicfly, 0,      magicfly, magicfly, 0,     ROT0, "P&A Games", "Magic Fly", GAME_IMPERFECT_COLORS )
GAME( 198?, 7mezzo,   0,      7mezzo,   7mezzo,   0,     ROT0, "<unknown>", "7 e Mezzo", GAME_IMPERFECT_COLORS )
