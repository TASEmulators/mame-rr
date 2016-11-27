/*************************************************************************

    VIDEO POKER - IGT/INTERFLIP
    -----------------------

    Original driver by Grull Osgo.
    Rewrite and additional work by Roberto Fresca.
    Fortune I (IGT) added by Jim Stolis.


    Games running on this hardware:

    * Draw Poker,    1984, IGT - International Game Technology

    * Video Poker,   1984, InterFlip.
    * Black Jack,    1984, InterFlip.
    * Video Dado,    1987, InterFlip.
    * Video Cordoba, 1987, InterFlip.

    * Baby Poker,    1989, Recreativos Franco.
    * Baby Dado,     1989, Recreativos Franco.


***************************************************************************


    History:
    ========

    In 1975, Si Redd founded A1-Supply.  This was renamed to Sircoma in 1978,
    and eventually IGT (International Game Technology) in 1981.

    Along the way, in 1978, Fortune Coin Company was acquired and contained
    the basis to their Fortune I game machines.

    The Fortune I hardware consisted of the following games:
      Regular Draw Poker
      Progressive Draw Poker
      Joker Wild Draw Poker
      Double Up Draw Poker
      Credit Draw Poker
      Lucky 7 Poker (Seven Card Stud Poker)
      Regular 21 (Blackjack)
      Live 21
      Count Down 21
      Two Hand 21
      In Between
      Regular Slot
      Credit Slot

    InterFlip (Spain), is a subsidiary of Recreativos Franco.
    This company was created mainly with the purpose of manufacture
    and distribute export-class games.

    These machines were build in upright wood style cabinets, and compliment
    with class "C" (for casinos) spanish gaming regulations.


***************************************************************************


    Hardware Info
    =============

    This is a two board system: Main & Sound board.


    * Main board:

    1x Intel 8039 CPU               Clocked @ 6/8 MHz.
    2x 2716 EPROM                   Program ROM.
    2x 2716 EPROM                   Character/graphics ROM.
    1x 5101 (256 x 4) SRAM          Data Memory (Battery Backed RAM).
    2x 2114 (1024 x 4) SRAM         8-Bit Char. RAM.
    1x 2114 (1024 x 4) SRAM         4-Bit Color RAM.
    1x 74s287 (256 x 4) TTL PROM    Color PROM.

    1x 6 MHz.(or 8 MHz.) Crystal    CPU clock.
    1x 7.8643 MHz. Crystal          Video System.

    1x 3.6 Ni-Cd Battery            Data Memory.

    TTL type Raster video           Configurable HIGH-LOW resolution through hardware jumper.
                                    Configurable 50Hz-60Hz V-Sync through hardware jumper.

    I/O System                      Buffered, latched & optocoupled.


    * Sound board:

    1x Intel 8039 CPU               Clocked @ 8 MHz.
    2x 2716 EPROM                   Program ROM.
    1x 1408 DAC
    1x 8.0000 MHz. Crystal


********************************************************************************


    Main CPU Memory Map
    ===================

    0x0000 - 0x0FFF         Program ROM.

    Data & Video RAM are mapped through I/O hardware implementations due to
    I8039 memory addressing restrictions.


    Main CPU I/0 Map
    ================

    P1.0          ; Used at bit level, Aux_0 signal.
    P1.1          ; Used at bit level, Aux_1 signal.
    P1.2          ; Used at bit level, Aux_2 signal.
    P1.3          ; Used at bit level, Aux_3 signal.
    P1.4          ; Used at bit level, Aux_4 signal & Sound Latch bit 3
    P1.5          ; Used at bit level, Aux_5 signal & Sound Latch bit 0
    P1.6          ; Expands address bus for video and color RAM access (V.A8)
    P1.7          ; Expands address bus for video and color RAM access (V.A9)

    P2.0 - P2.3:  ; I8039 as address bus expansion (Program memory - High address nibble).

    P2.4:         ; Reads 8 bits from data buffer input port (interface for switch encoder).
                        Bit 0: Lamp_1.
                        Bit 1: Lamp_2.
                        Bit 2: Lamp_3.
                        Bit 3: Lamp_4.
                        Bit 4: Coin Acceptor.
                        Bit 5: Hopper 1 & Sound Latch bit 1.
                        Bit 6: Hopper 2 & Sound Latch bit 2.
                        Bit 7: Diverter.

                    Writes 8 bits to data latch out port (lamps, relays and coils).
                        Bit 0: SW_encoder_0.
                        Bit 1: SW_encoder_1.
                        Bit 2: SW_encoder_2.
                        Bit 3: SW_encoder_3.
                        Bit 4: Coin Out.
                        Bit 5: Undocumented jumper.
                        Bit 6: N/U (pulled up).
                        Bit 7: N/U (pulled up).

    P2.5:         ; Enable access to data RAM.
    P2.6:         ; Enable access to video RAM (write mode) - no read mode.
    P2.7:         ; Enable access to color RAM (write Mode) - no read mode.


********************************************************************************


    Game Info
    =========

    Pay Tables:

    These machines had its pay tables out of screen, in the upper front panel (backlighted).


                        Video Poker (Spanish text only)
    ----------------------------------------------------------------------------
    Fichas Jugadas       1 Ficha    2 Fichas    3 Fichas    4 Fichas    5 Fichas
    ----------------------------------------------------------------------------
    Escalera Maxima         250        500         750        1000        4000
    de color

    Escalera de Color       100        200         300         400         500

    Poker                    50        100         150         200         250

    Full                     11         22          33          44          55

    Color                     7         14          21          28          35

    Escalera                  5         10          15          20          25

    Trio                      3          6           9          12          15

    Doble Pareja              2          4           6           8          10
    ----------------------------------------------------------------------------


                        Black Jack (Spanish text only)
    ----------------------------------------------------------------------------
    Fichas Jugadas       1 Ficha    2 Fichas    3 Fichas    4 Fichas    5 Fichas
    ----------------------------------------------------------------------------
    Empate                   1          2           3           4            5

    La Banca se pasa         2          4           6           8           10

    Jugador tiene mas        2          4           6           8           10
    que la banca

    Jugador tiene menos      2          4           6           8           10
    de 22 con 6 cartas

    Jugador tiene blackjack  2          5           7          10           12
    y la Banca no
    ----------------------------------------------------------------------------


                                Video Cordoba
    ----------------------------------------------------------------------------
    Fichas Jugadas       1 Ficha    2 Fichas    3 Fichas    4 Fichas    5 Fichas
    ----------------------------------------------------------------------------
    TRIPLE BAR             250         250         250         250        2000
    ............................................................................
    3 x Doble Bar        - 100            |    Olive-Olive-Any Bar        - 18
    3 x Single Bar       -  50            |    3 x Orange                 - 14
    3 x Any Bar          -  20            |    Orange-Orange-Any Bar      - 14
    3 x Bell             -  20            |    3 x Cherry                 - 10
    Bell-Bell-Any Bar    -  20            |    2 x Cherry                 -  5
    3 x Olive            -  10            |    1 x Cherry                 -  2
    ............................................................................
    All combinations are valid only from left to rigth
    ----------------------------------------------------------------------------


                     Video Dado
    ---------------------------------------------
    Twelve      (12)                    33 x 1
    Eleven      (11)                    16 x 1
    Crap        (2,3,12)                 8 x 1
    Seven       (7)                      5 x 1
    Field       (2,12)                   3 x 1
    8 or More   (8,9,10,11,12)           2 x 1
    6 or Less   (2,3,4,5,6)              2 x 1

    Winnings less or equal to 25 can be re-played
    ---------------------------------------------


    All payments with less than 400 coins are done through hopper.
    (you need to press "Coin Out" button once per coin due to the lack of hopper emulation)

    Payments over 400 coins are manual.


**************************************************************************


    [2008-10-08]

    - Added Baby Poker Game.
    - Added Baby Dado Game.
    - Mapped "Hand Pay" button for Baby Games.
    - Added decoder to Jackpot mechanical counter.
    - Added sound support to Baby Poker Game.
    - Added tower lamps to Baby Games layouts.
    - Reworked layouts for Baby Games.
    - Reworked the color routines.
    - Added new color routines for Baby Games.
    - Redumped the videocba color PROM.
    - Added color switch. (It changes background color in some games).
    - Added "hopper full" switch support (for diverter function).
    - Added diverter function decoder.
    - Added Button-lamps layout.
    - Added full functional mechanical counters decoding.
    - Added 7 Segment decoder and 7 Digit Counter functions.
    - Added button-lamps layout & mechanical counters simulation on layout.
      Mechanical counters to layout: Coin-In, Coin-Out and Coin to Drop.
    - Added NVRAM support to mechanical counters.

    TO DO
    =====

    * Add Tech. Notes for Baby board, a reworked and improved version on Video Poker hardware.
    * Fix some missing pulses on mechanical counters.
    * Fix the bug on bookeeping mode (videodad & videocba).
    * Figure out the undocumented jumper.
    * Hopper simulation.
    * Switch to resnet system.


**************************************************************************/


#define CPU_CLOCK		(XTAL_6MHz)			/* main cpu clock */
#define CPU_CLOCK_ALT	(XTAL_8MHz)			/* alternative main cpu clock for newer games */
#define SOUND_CLOCK		(XTAL_8MHz)			/* sound cpu clock */
#define VIDEO_CLOCK		(XTAL_7.8643MHz)


#include "emu.h"
#include "cpu/mcs48/mcs48.h"
#include "cpu/mcs51/mcs51.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "videopkr.lh"
#include "blckjack.lh"
#include "videocba.lh"
#include "videodad.lh"
#include "babypkr.lh"
#include "babydad.lh"

#define DATA_NVRAM_SIZE     0x100

static UINT8 data_ram[0x100];
static UINT8 video_ram[0x0400];
static UINT8 color_ram[0x0400];
static UINT16 p1, p2;
static UINT8 t0_latch;
static UINT16 n_offs;

static UINT8 vp_sound_p2;
static UINT8 p24_data;
static UINT8 sound_latch;
static UINT8 baby_latch;
static UINT8 sound_ant;
static UINT8 dc_4020;
static UINT8 dc_40103;
static UINT8 te_40103;
static UINT8 ld_40103;

static UINT8 ant_jckp, jckp, ant_cio, c_io, hp_1, hp_2, bell, aux3, dvrt;
static unsigned long count0, count1, count2, count3, count4;

/* Baby vars */
static UINT8 sbp0, sbp2, sbp3;


/*************************
*     Video Hardware     *
*************************/

static tilemap_t *bg_tilemap;

/* BCD to Seven Segment Decoder */
static UINT8 dec_7seg(int data)
{
	UINT8 segment;
	switch (data)
	{
		case 0: segment = 0x3f; break;
		case 1: segment = 0x06; break;
		case 2: segment = 0x5b; break;
		case 3: segment = 0x4f; break;
		case 4: segment = 0x66; break;
		case 5: segment = 0x6d; break;
		case 6: segment = 0x7d; break;
		case 7: segment = 0x07; break;
		case 8: segment = 0x7f; break;
		case 9: segment = 0x6f; break;
		default: segment = 0x79;
	}

	return segment;
}

/* Display a seven digit counter on layout - Index points to less significant digit*/
static void count_7dig(unsigned long data, UINT8 index)
{
	UINT8 i;
	char strn[8];
	sprintf(strn,"%7lu",data);

	for (i = 0; i < 7; i++)
	{
		output_set_digit_value(index+i, dec_7seg((strn[6 - i] | 0x10) - 0x30));
	}
}

static PALETTE_INIT( videopkr )
{
	int j;

	for (j = 0; j < machine->total_colors(); j++)
	{
		int r, g, b, tr, tg, tb, i;

		i = (color_prom[j] >> 3) & 0x01;

		/* red component */
		tr = 0xf0 - (0xf0 * ((color_prom[j] >> 0) & 0x01));
		r = tr - (i * (tr / 5));

		/* green component */
		tg = 0xf0 - (0xf0 * ((color_prom[j] >> 1) & 0x01));
		g = tg - (i * (tg / 5));

		/* blue component */
		tb = 0xf0 - (0xf0 * ((color_prom[j] >> 2) & 0x01));
		b = tb - (i * (tb / 5));

		palette_set_color(machine, j, MAKE_RGB(r, g, b));
	}
}

static PALETTE_INIT( babypkr )
{
	int j;

	for (j = 0; j < machine->total_colors(); j++)
	{
		int r, g, b, tr, tg, tb, i, top;

		top = 0xff;

		/* intense component */
		i = 0x2f * ((color_prom[j] >> 3) & 0x01);
		top = top - i;

		/* red component */
		tr =  0xdf * ((color_prom[j] >> 0) & 0x01);
		r = top - ((tr * top) / 0x100 );

		/* green component */
		tg =  0xdf * ((color_prom[j] >> 1) & 0x01);
		g = top - ((tg * top) / 0x100 );

		/* blue component */
		tb =  0xdf * ((color_prom[j] >> 2) & 0x01);
		b = top - ((tb * top) / 0x100);

		palette_set_color(machine, j, MAKE_RGB(r, g, b));
	}
}

static PALETTE_INIT( fortune1 )
{
	int j;

	for (j = 0; j < machine->total_colors(); j++)
	{
		int r, g, b, tr, tg, tb, i, c;

		i = (color_prom[j] >> 3) & 0x01;

		/* red component */
		tr = 0xf0 - (0xf0 * ((color_prom[j] >> 0) & 0x01));
		r = tr - (i * (tr / 5));

		/* green component */
		tg = 0xf0 - (0xf0 * ((color_prom[j] >> 1) & 0x01));
		g = tg - (i * (tg / 5));

		/* blue component */
		tb = 0xf0 - (0xf0 * ((color_prom[j] >> 2) & 0x01));
		b = tb - (i * (tb / 5));

		c = j;

		// Swap Position of Inner-most Colors on Each 4 Color Palette
		if ((c % 4) == 1 || (c % 4) == 2)
			c = ((int)(c / 4) * 4) + (3 - (c % 4));

		palette_set_color(machine, c, MAKE_RGB(r, g, b));
	}
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int offs = tile_index;
	int attr = color_ram[offs] + input_port_read(machine, "IN2"); /* Color Switch Action */
	int code = video_ram[offs];
	int color = attr;
	SET_TILE_INFO(0, code, color, 0);
}


static VIDEO_START( videopkr )
{
	bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
}

static VIDEO_START( vidadcba )
{
	bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 16, 8, 32, 32);
}


static VIDEO_UPDATE( videopkr )
{
	tilemap_mark_all_tiles_dirty(bg_tilemap);
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	return 0;
}

/********************
*   NVRAM Handler   *
********************/

static NVRAM_HANDLER( videopkr )
{
	if (read_or_write)
	{
		mame_fwrite(file, data_ram, DATA_NVRAM_SIZE);
		mame_fwrite(file, &count1, 8);
		mame_fwrite(file, &count2, 8);
		mame_fwrite(file, &count3, 8);
		mame_fwrite(file, &count4, 8);

	}
	else
	{
		if (file)
		{
			mame_fread(file, data_ram, DATA_NVRAM_SIZE);
			mame_fread(file, &count1, 8);
			mame_fread(file, &count2, 8);
			mame_fread(file, &count3, 8);
			mame_fread(file, &count4, 8);
		}
		else
		{
			memset(data_ram, 0, DATA_NVRAM_SIZE);
			memset(data_ram, count0, 8);
			memset(data_ram, count0, 8);
			memset(data_ram, count0, 8);
			memset(data_ram, count0, 8);
		}
	}
}


/*************************
*      R/W Handlers      *
*************************/

static READ8_HANDLER( videopkr_io_r )
{
	UINT8 valor = 0, hf, co;

	UINT16 kbdin;

	switch (p2)
	{
		case 0xef:	/* inputs are multiplexed through a diode matrix */
		{
			hf = ((input_port_read(space->machine, "IN1") & 0x10 ) >> 4) & 1;			/* Hopper full detection */
			co = 0x10 * ((input_port_read(space->machine, "IN1") & 0x20 ) >> 5);		/* Coin Out detection */
			kbdin = ((input_port_read(space->machine, "IN1") & 0xaf ) << 8) + input_port_read(space->machine, "IN0");

			switch (kbdin)
			{
				case 0x0000: valor = 0x00; break;
				case 0x0001: valor = 0x01; break;	/* Door */
				case 0x4000: valor = 0x02; break;
				case 0x8000: valor = 0x03; break;	/* Hand Pay */
				case 0x0002: valor = 0x04; break;	/* Books */
				case 0x0004: valor = 0x05; break;	/* Coin In */
				case 0x0008: valor = 0x07; break;	/* Start */
				case 0x0010: valor = 0x08; break;	/* Discard */
				case 0x0020: valor = 0x09; break;	/* Cancel */
				case 0x0040: valor = 0x0a; break;	/* Hold 1 */
				case 0x0080: valor = 0x0b; break;	/* Hold 2 */
				case 0x0100: valor = 0x0c; break;	/* Hold 3 */
				case 0x0200: valor = 0x0d; break;	/* Hold 4 */
				case 0x0400: valor = 0x0e; break;	/* Hold 5 */
				case 0x0800: valor = 0x06; break;	/* Bet */
			}

			if ((valor == 0x00) & hf )
			{
				valor = 0x0f;
			}

			valor += co;
			break;
		}

		case 0xdf:
		{
			n_offs = ((p1 & 0xc0) << 2 ) + offset;
			valor = data_ram[offset];
			break;
		}

		case 0x5f:
		{
			n_offs = ((p1 & 0xc0) << 2 ) + offset;
			valor = data_ram[offset];
			break;
		}

		case 0x7c:
		case 0x7d:
		case 0x7e:
		case 0x7f:
		{
			n_offs = ((p1 & 0xc0) << 2 ) + offset;
			valor = color_ram[n_offs];
			break;
		}

		case 0xbc:
		case 0xbd:
		case 0xbe:
		case 0xbf:
		{
			n_offs = ((p1 & 0xc0) << 2 ) + offset;
			valor = video_ram[n_offs];
			break;
		}
	}

	return valor;
}

static WRITE8_HANDLER( videopkr_io_w )
{
	switch (p2)
	{
		case 0x3c:
		case 0x3d:
		case 0x3e:
		case 0x3f:
		{
			n_offs = ((p1 & 0xc0) << 2 ) + offset;
			color_ram[n_offs] = data & 0x0f;
			video_ram[n_offs] = data;
			tilemap_mark_tile_dirty(bg_tilemap, n_offs);
			break;
		}

		case 0xdf:
		{
			data_ram[offset] = (data & 0x0f) + 0xf0;
			break;
		}

		case 0x7c:
		case 0x7d:
		case 0x7e:
		case 0x7f:
		{
			n_offs = ((p1 & 0xc0) << 2 ) + offset;
			color_ram[n_offs] = data & 0x0f;
			tilemap_mark_tile_dirty(bg_tilemap, n_offs);
			break;
		}

		case 0xbc:
		case 0xbd:
		case 0xbe:
		case 0xbf:
		{
			n_offs = ((p1 & 0xc0) << 2 ) + offset;
			video_ram[n_offs] = data;
			tilemap_mark_tile_dirty(bg_tilemap, n_offs);
			break;
		}

		case 0xef:	/* Port 2.4 */
		{
			output_set_lamp_value(0, (data & 1));			/* L_1 */
			output_set_lamp_value(1, ((data >> 1)& 1));		/* L_2 */
			output_set_lamp_value(2, ((data >> 2) & 1));	/* L_3 */
			output_set_lamp_value(3, ((data >> 3) & 1));	/* L_4 */
			output_set_lamp_value(4, ((data >> 4) & 1));	/* Coin */
			output_set_lamp_value(5, ((data >> 5) & 1));	/* Hopper_1 */
			output_set_lamp_value(6, ((data >> 6) & 1));	/* Hopper_2 */
			output_set_lamp_value(7, ((data >> 7) & 1));	/* Diverter */
			p24_data = data;
			hp_1 = (~p24_data >> 6) & 1;
			hp_2 = (~p24_data >> 5) & 1;
			dvrt = (~p24_data >> 7) & 1;
			break;
		}

		case 0xff:
		{
			t0_latch = t0_latch ^ 0x01;		/* fix the bookkeeping mode */
			break;
		}
	}
}

static READ8_HANDLER( videopkr_p1_data_r )
{
	return p1;
}

static READ8_HANDLER( videopkr_p2_data_r )
{
	return p2;
}

static WRITE8_HANDLER( videopkr_p1_data_w )
{
	p1 = data;

	output_set_lamp_value(8, (data & 1));			/* Aux_0 - Jackpot mech. counter (Baby Games)*/
	output_set_lamp_value(9, ((data >> 1) & 1));	/* Aux_1 - */
	output_set_lamp_value(10, ((data >> 2) & 1));	/* Aux_2 - */
	output_set_lamp_value(11, ((data >> 3) & 1));	/* Aux_3 - */
	output_set_lamp_value(12, ((data >> 4) & 1));	/* Aux_4 - Bell */
	output_set_lamp_value(13, ((data >> 5) & 1));	/* Aux_5 - /CIO */

	jckp = p1 & 1;

	if ((~c_io & 1) & ant_cio & hp_1 & hp_2)
	{
		++count1;	/* Decoded Coin In Mech. Counter*/
	}

	if ((~c_io & 1) & ant_cio & (~hp_1 & 1) & (~hp_2 & 1))
	{
		++count2;	/* Decoded Coind Out Mech. Counter */
	}

	if (~c_io & ant_cio & hp_1 & hp_2 & ~dvrt)
	{
		++count3;	/* Decoded Coin to Drop Mech. Counter */
	}

	if (~jckp & ant_jckp)
	{
		++count4;	/* Decoded Jackpot Mech. Counter */
	}

	count_7dig(count1, 0);
	count_7dig(count2, 7);
	count_7dig(count3, 14);
	count_7dig(count4, 21);

	ant_cio = c_io;
	ant_jckp = jckp;
}

static WRITE8_HANDLER( videopkr_p2_data_w )
{
	p2 = data;
}

static READ8_HANDLER( videopkr_t0_latch )
{
	return t0_latch;
}

static WRITE8_HANDLER( prog_w )
{
	if (!data)
		cputag_set_input_line(space->machine, "maincpu", 0, CLEAR_LINE);	/* clear interrupt FF */
}

/*************************
*     Sound Handlers     *
*************************/
/*

  Sound Data ( Sound Board latch )

    Data Bit     Comes from
    ---------------------------------------------
    bit 0        Coin I/O     Port 1.5
    bit 1        Hopper2      Port 2.4 Data Bit 6
    bit 2        Hopper1      Port 2.4 Data Bit 5
    bit 3        Bell         Port 1.4
    bit 4        Aux_3        Port 1.3
    bit 5        N/U          Pulled Up
    bit 6        N/U          Pulled Up
    bit 7        N/U          Pulled Up


  Sound Codes

    Hex     Bin         Sound                       Game
    --------------------------------------------------------------------------
    0xFF    11111111    No Sound (default state)    All Games.
    0xFE    11111110    Coin In sound               All Games.
    0xEF    11101111    Cards draw sound            Video Poker & Black Jack.
    0xF9    11111001    Hopper run                  Video Poker & Black Jack.
    0xF8    11111000    Coin Out sound              Video Poker & Black Jack.
    0xF6    11110110    Coin Out sound              Video Dado  & Video Cordoba.
    0xFA    11111010    Dice rolling sound          Video Dado.
    0xFA    11111010    Spinning reels sound        Video Cordoba.
    0xFB    11111011    Dice rolling sound          Video Dado.
    0xFB    11111011    Stopping reels sound        Video Cordoba.

*/

static READ8_HANDLER(sound_io_r)
{
	switch (vp_sound_p2)
	{
		case 0xbf:
		{
			c_io = (p1 >> 5) & 1;
			hp_1 = (~p24_data >> 6) & 1;
			hp_2 = (~p24_data >> 5) & 1;
			bell = (p1 >> 4) & 1;
			aux3 = (p1 >> 3) & 1;
			dvrt = (~p24_data >> 7) & 1;
			sound_ant = sound_latch;
			sound_latch = c_io + (hp_1 << 1) + (hp_2 << 2) + (bell << 3) + 0xf0;

			break;
		}
	}

	return sound_latch;
}

static WRITE8_HANDLER(sound_io_w)
{
	if (vp_sound_p2 == 0x5f || vp_sound_p2 == 0xdf)
	{
		dc_40103 = data;
		dc_4020 = 0;
	}
}

static READ8_HANDLER(sound_p2_r)
{
	return vp_sound_p2;
}

static WRITE8_HANDLER(sound_p2_w)
{
	vp_sound_p2 = data;

	switch (data)
	{
		case 0x5f:
		{
			te_40103 = 0;	/* p2.7 LOW */
			ld_40103 = 0;	/* p2.5 LOW */
			break;
		}

		case 0x7f:
		{
			te_40103 = 0;
			ld_40103 = 1;
			break;
		}

		case 0xff:
		{
			te_40103 = 1;
			ld_40103 = 1;
			break;
		}
	}
}


/* Baby Sound Handlers */

static READ8_HANDLER(baby_sound_p0_r)
{
	return sbp0;
}

static WRITE8_HANDLER(baby_sound_p0_w)
{
	sbp0 = data;
}

static READ8_HANDLER(baby_sound_p1_r)
{
	c_io = (p1 >> 5) & 1;
	hp_1 = (~p24_data >> 6) & 1;
	hp_2 = (~p24_data >> 5) & 1;
	bell = (p1 >> 4) & 1;
	aux3 = (p1 >> 3) & 1;
	baby_latch = c_io + (hp_1 << 1) + (hp_2 << 2) + (bell << 3) + (aux3 << 4) + 0xe0;
	return baby_latch;
}

static WRITE8_HANDLER(baby_sound_p1_w)
{
	baby_latch = baby_latch | data;
}

static READ8_HANDLER(baby_sound_p2_r)
{
	return sbp2;
}

static WRITE8_DEVICE_HANDLER(baby_sound_p2_w)
{
	sbp2 = data;
	dac_data_w(device, data);
}

static READ8_DEVICE_HANDLER(baby_sound_p3_r)
{
	return sbp3;
}

static WRITE8_DEVICE_HANDLER(baby_sound_p3_w)
{
	UINT8 lmp_ports, ay_intf;
	sbp3 = data;
	lmp_ports = sbp3 >> 1 & 0x07;

	output_set_value("TOP_1", (lmp_ports >> 0) & 1);
	output_set_value("TOP_2", (lmp_ports >> 1) & 1);
	output_set_value("TOP_3", (lmp_ports >> 2) & 1);

	if (!(sbp3 & 0x10))
	{
		device->reset();
		logerror("AY3-8910: Reset\n");
	}

	ay_intf = (sbp3 >> 5) & 0x07;

	switch (ay_intf)
	{
		case 0x00:	break;
		case 0x01:	break;
		case 0x02:	break;
		case 0x03:	ay8910_data_w(device, 1, sbp0); break;
		case 0x04:	break;
		case 0x05:	sbp0 = ay8910_r(device, sbp0); break;
		case 0x06:	break;
		case 0x07:	ay8910_address_w(device, 0, sbp0); break;
	}
}


static TIMER_DEVICE_CALLBACK(sound_t1_callback)
{
	if (te_40103 == 1)
	{
		dc_40103++;

		if (dc_40103 == 0)
		{
			cputag_set_input_line(timer.machine, "soundcpu", 0, ASSERT_LINE);
		}
	}
}

/*************************
* Memory Map Information *
*************************/

static ADDRESS_MAP_START( i8039_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( i8039_io_port, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x00,            0xff           ) AM_READWRITE(videopkr_io_r, videopkr_io_w)
	AM_RANGE(MCS48_PORT_P1,   MCS48_PORT_P1  ) AM_READWRITE(videopkr_p1_data_r, videopkr_p1_data_w)
	AM_RANGE(MCS48_PORT_P2,   MCS48_PORT_P2  ) AM_READWRITE(videopkr_p2_data_r, videopkr_p2_data_w)
	AM_RANGE(MCS48_PORT_PROG, MCS48_PORT_PROG) AM_WRITE(prog_w)
	AM_RANGE(MCS48_PORT_T0,   MCS48_PORT_T0  ) AM_READ(videopkr_t0_latch)
ADDRESS_MAP_END

static ADDRESS_MAP_START( i8039_sound_mem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( i8039_sound_port, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x00         , 0xff         ) AM_READWRITE(sound_io_r, sound_io_w)
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_DEVWRITE("dac", dac_w)
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_READWRITE(sound_p2_r, sound_p2_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( i8051_sound_mem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( i8051_sound_port, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x0000, 0x1ff) AM_RAM
	/* ports */
	AM_RANGE(MCS51_PORT_P0, MCS51_PORT_P0) AM_READWRITE(baby_sound_p0_r, baby_sound_p0_w)
	AM_RANGE(MCS51_PORT_P1, MCS51_PORT_P1) AM_READWRITE(baby_sound_p1_r, baby_sound_p1_w)
	AM_RANGE(MCS51_PORT_P2, MCS51_PORT_P2) AM_READ(baby_sound_p2_r) AM_DEVWRITE("dac", baby_sound_p2_w)
	AM_RANGE(MCS51_PORT_P3, MCS51_PORT_P3) AM_DEVREADWRITE("aysnd", baby_sound_p3_r, baby_sound_p3_w)
ADDRESS_MAP_END


/************************
*      Input Ports      *
************************/

static INPUT_PORTS_START( videopkr )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_GAMBLE_DOOR )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_GAMBLE_BOOK ) PORT_NAME("Books")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Start")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Discard") PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_POKER_CANCEL )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_POKER_HOLD4 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POKER_HOLD5 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Hopper") PORT_TOGGLE PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN2")
	PORT_DIPNAME( 0x20, 0x00, "Color Sw." )		PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On  ) )

INPUT_PORTS_END

static INPUT_PORTS_START( blckjack )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_GAMBLE_DOOR )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_GAMBLE_BOOK ) PORT_NAME("Books")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Start")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Hit") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Stand") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Double") PORT_CODE(KEYCODE_C)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Hopper") PORT_TOGGLE PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN2")
	PORT_DIPNAME( 0x20, 0x00, "Color Sw." )		PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On  ) )
INPUT_PORTS_END

static INPUT_PORTS_START( videodad )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_GAMBLE_DOOR )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_GAMBLE_BOOK ) PORT_NAME("Books")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Crap") PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("6 or Less") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Seven") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("8 or More") PORT_CODE(KEYCODE_C)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Field") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("Eleven") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_NAME("Twelve") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Hopper") PORT_TOGGLE PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN2")
	PORT_DIPNAME( 0x20, 0x00, "Color Sw." )		PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On  ) )
INPUT_PORTS_END

static INPUT_PORTS_START( videocba )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_GAMBLE_DOOR )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_GAMBLE_BOOK ) PORT_NAME("Books")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Start")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Hopper") PORT_TOGGLE PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT) PORT_NAME("Payout") PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN2")
	PORT_DIPNAME( 0x20, 0x00, "Color Sw." )		PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On  ) )
INPUT_PORTS_END


static INPUT_PORTS_START( babypkr )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_GAMBLE_DOOR )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_GAMBLE_BOOK ) PORT_NAME("Books")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Start")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_GAMBLE_D_UP ) PORT_NAME("Double / Discard") PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_POKER_CANCEL ) PORT_NAME("Cancel / Take")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_POKER_HOLD4 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POKER_HOLD5 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_GAMBLE_BET )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Hopper") PORT_TOGGLE PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED  )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Hand Pay") PORT_CODE(KEYCODE_W)

	PORT_START("IN2")
	PORT_DIPNAME( 0x20, 0x00, "Color Sw." )		PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On  ) )

INPUT_PORTS_END

static INPUT_PORTS_START( babydad )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_GAMBLE_DOOR )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_GAMBLE_BOOK ) PORT_NAME("Books")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Crap") PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("6 or Less") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Seven") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("8 or More") PORT_CODE(KEYCODE_X)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Field")  PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("Eleven") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_NAME("Twelve") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_GAMBLE_BET )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Hopper") PORT_TOGGLE PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED  )
    PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Hand Pay") PORT_CODE(KEYCODE_W)

	PORT_START("IN2")
	PORT_DIPNAME( 0x20, 0x00, "Color Sw." )		PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On  ) )

INPUT_PORTS_END


/*************************
*    Graphics Layouts    *
*************************/

static const gfx_layout tilelayout_16 =
{
	16, 8,
	RGN_FRAC(1,4),
	2,
	{ 0, RGN_FRAC(1,2) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
		RGN_FRAC(1,4), RGN_FRAC(1,4) + 1, RGN_FRAC(1,4) + 2, RGN_FRAC(1,4) + 3,
		RGN_FRAC(1,4) + 4, RGN_FRAC(1,4) + 5, RGN_FRAC(1,4) + 6, RGN_FRAC(1,4) + 7
	},
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout tilelayout_8 =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ 0, RGN_FRAC(1,2) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


/******************************
* Graphics Decode Information *
******************************/

static GFXDECODE_START( videopkr )
	GFXDECODE_ENTRY( "tiles", 0, tilelayout_8, 0, 64 )
GFXDECODE_END


static GFXDECODE_START( videodad )
	GFXDECODE_ENTRY( "tiles", 0, tilelayout_16, 0, 64 )
GFXDECODE_END


/*******************************
*    Machine Start / Reset     *
*******************************/

static MACHINE_START(videopkr)
{
	vp_sound_p2 = 0xff;	/* default P2 latch value */
	sound_latch = 0xff;	/* default sound data latch value */
	p24_data = 0xff;
	p1 = 0xff;
	ant_cio = 0;
	count0 = 0;
}

static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	/* no ports used */
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

/************************
*    Machine Drivers    *
************************/

static MACHINE_DRIVER_START( videopkr )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", I8039, CPU_CLOCK)
	MDRV_CPU_PROGRAM_MAP(i8039_map)
	MDRV_CPU_IO_MAP(i8039_io_port)

	MDRV_CPU_VBLANK_INT("screen", irq0_line_assert)

	MDRV_CPU_ADD("soundcpu", I8039, SOUND_CLOCK)
	MDRV_CPU_PROGRAM_MAP(i8039_sound_mem)
	MDRV_CPU_IO_MAP(i8039_sound_port)
	MDRV_MACHINE_START(videopkr)
	MDRV_NVRAM_HANDLER(videopkr)

	MDRV_TIMER_ADD_PERIODIC("t1_timer", sound_t1_callback, HZ(50))

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)

	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(5*8, 31*8-1, 3*8, 29*8-1)

	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(2080)
	MDRV_GFXDECODE(videopkr)
	MDRV_PALETTE_INIT(videopkr)
	MDRV_PALETTE_LENGTH(256)
	MDRV_VIDEO_START(videopkr)
	MDRV_VIDEO_UPDATE(videopkr)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("dac", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.55)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( blckjack )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(videopkr)

	/* video hardware */
	MDRV_SCREEN_MODIFY("screen")
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(4*8, 31*8-1, 2*8, 30*8-1)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( videodad )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(videopkr)
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_CLOCK(CPU_CLOCK_ALT)

	/* video hardware */
	MDRV_SCREEN_MODIFY("screen")
	MDRV_SCREEN_SIZE(32*16, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(4*16, 31*16-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(videodad)
	MDRV_VIDEO_START(vidadcba)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( babypkr )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(videopkr)
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_CLOCK(CPU_CLOCK_ALT)
	/* most likely romless or eprom */
	MDRV_CPU_REPLACE("soundcpu", I8031, CPU_CLOCK )
	MDRV_CPU_PROGRAM_MAP(i8051_sound_mem)
	MDRV_CPU_IO_MAP(i8051_sound_port)

	/* video hardware */
	MDRV_SCREEN_MODIFY("screen")
	MDRV_SCREEN_SIZE(32*16, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(5*16, 31*16-1, 3*8, 29*8-1)

	MDRV_PALETTE_INIT(babypkr)
	MDRV_GFXDECODE(videodad)
	MDRV_VIDEO_START(vidadcba)

	MDRV_SOUND_ADD("aysnd", AY8910, CPU_CLOCK / 6)
	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( fortune1 )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(videopkr)
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_CLOCK(CPU_CLOCK_ALT)

	MDRV_PALETTE_INIT(fortune1)
MACHINE_DRIVER_END

/*************************
*        Rom Load        *
*************************/

ROM_START( videopkr )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "vpoker.c5",		0x0000, 0x0800,	CRC(200d21e4) SHA1(d991c9f10a36a02491bb0aba32129675fed77a10) )
	ROM_LOAD( "vpoker.c7",		0x0800, 0x0800,	CRC(f72c2a90) SHA1(e9c54d1f895cde0aaca4121a252da40594195a25) )

	ROM_REGION( 0x1000, "soundcpu", 0 )	/* sound cpu program */
	ROM_LOAD( "vpsona3.pbj",	0x0000, 0x0800,	CRC(a4f7bf7f) SHA1(a08287821f3471cb3e1ae0528811da930fd57387) )
	ROM_LOAD( "vpsona2.pbj",	0x0800, 0x0800,	CRC(583a9b95) SHA1(a10e85452e285b2a63f885f4e39b7f76ee8b2407) )

	ROM_REGION( 0x1000, "tiles", 0 )
    ROM_LOAD( "vpbj_b15.org",	0x0000, 0x0800,	CRC(67468e3a) SHA1(761766f0fb92693d32179a914e11da517cc5747d) )
    ROM_LOAD( "vpbj_b12.org",	0x0800, 0x0800,	CRC(4aba166e) SHA1(930cea2216a39b5d72021d1b449db018a121adce) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "vpbjorg.col",	0x0000, 0x0100,	CRC(09abf5f1) SHA1(f2d6b4f2f08b47b93728dafb50576d5ca859255f) )
	ROM_END

ROM_START( blckjack )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "bjc5org.old",	0x0000, 0x0800,	CRC(e266a28a) SHA1(1f90c85a2a817f1927c9ab2cbf79cfa2dd116dc8) )
	ROM_LOAD( "bjc7org.old",	0x0800, 0x0800,	CRC(c60c565f) SHA1(c9ed232301750288bd000ac4e2dcf2253745ff0a) )

	ROM_REGION( 0x1000, "soundcpu", 0 )	/* sound cpu program */
	ROM_LOAD( "vpsona3.pbj",	0x0000, 0x0800,	CRC(a4f7bf7f) SHA1(a08287821f3471cb3e1ae0528811da930fd57387) )
	ROM_LOAD( "vpsona2.pbj",	0x0800, 0x0800,	CRC(583a9b95) SHA1(a10e85452e285b2a63f885f4e39b7f76ee8b2407) )

	ROM_REGION( 0x1000, "tiles", 0 )
	ROM_LOAD( "vpbj_b15.org",	0x0000, 0x0800,	CRC(67468e3a) SHA1(761766f0fb92693d32179a914e11da517cc5747d) )
	ROM_LOAD( "vpbj_b12.org",	0x0800, 0x0800,	CRC(4aba166e) SHA1(930cea2216a39b5d72021d1b449db018a121adce) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "vpbjorg.col",	0x0000, 0x0100,	CRC(09abf5f1) SHA1(f2d6b4f2f08b47b93728dafb50576d5ca859255f) )
ROM_END

ROM_START( videodad )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "dac5org.old",	0x0000, 0x0800,	CRC(b373c8e9) SHA1(7a99d6aa152f8e6adeddbfdfd13278edeaa529bc) )
	ROM_LOAD( "dac7org.old",	0x0800, 0x0800,	CRC(afabae30) SHA1(c4198ba8de6811e3367b0154ff479f6738721bfa) )

	ROM_REGION( 0x1000, "soundcpu", 0 )	/* sound cpu program */
	ROM_LOAD( "vdsona3.dad",	0x0000, 0x0800,	CRC(13f7a462) SHA1(2e2e904637ca7873a2ed67d7ab1524e51b324660) )
	ROM_LOAD( "vdsona2.dad",	0x0800, 0x0800,	CRC(120e4512) SHA1(207748d4f5793180305bb115af877042517d901f) )

	ROM_REGION( 0x2000, "tiles", 0 )
	ROM_LOAD( "vdadob15.bin",	0x0000, 0x0800,	CRC(caa6a4b0) SHA1(af99da30b8ee63d54ac1f1e6737ed707501a5a25) )
	ROM_LOAD( "vdadob14.bin",	0x0800, 0x0800,	CRC(eabfae6b) SHA1(189b38da5e9c99f99c5425cdfefccc6991e3f85e) )
	ROM_LOAD( "vdadob12.bin",	0x1000, 0x0800,	CRC(176f7b31) SHA1(613521ed9caf904db22860686e0424d0c0e0cba6) )
	ROM_LOAD( "vdadob11.bin",	0x1800, 0x0800,	CRC(259492c7) SHA1(003cc40a88f2b9fad0089574963e7e654211bb16) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "vdvcorg.col",	0x0000, 0x0100,	CRC(741b1a22) SHA1(50983ea37f0479793ba38a112a0266c2edc4b5ef) )
ROM_END

ROM_START( videocba )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "vcc5org.old",	0x0000, 0x0800,	CRC(96d72283) SHA1(056197a9e2ad40d1d6610bbe8a1855b81c0a6715) )
	ROM_LOAD( "vcc7org.old",	0x0800, 0x0800,	CRC(fdec55c1) SHA1(19b740f3b7f2acaa0fc09f4c0a2fe69721ebbcaf) )

	ROM_REGION( 0x1000, "soundcpu", 0 )	/* sound cpu program */
	ROM_LOAD( "vcsona3.rod",	0x0000, 0x0800,	CRC(b0948d6c) SHA1(6c45d350288f69b4b2b5ac16ab2b418f14c6eded) )
	ROM_LOAD( "vcsona2.rod",	0x0800, 0x0800,	CRC(44ff9e85) SHA1(5d7988d2d3bca932b77e014dc61f7a2347b01603) )

	ROM_REGION( 0x2000, "tiles", 0 )
	ROM_LOAD( "vcbab15.bin",	0x0000, 0x0800,	CRC(fce8c772) SHA1(f9736b724b620d60a17d77f6b773f39b99b47190) )
	ROM_LOAD( "vcbab14.bin",	0x0800, 0x0800,	CRC(6fd66330) SHA1(0ee3b3329b94ded81f028ebb687e580787c74ded) )
	ROM_LOAD( "vcbab12.bin",	0x1000, 0x0800,	CRC(e534d6c3) SHA1(7a93c6c07b5a28558ee005fed2098dc2933c3252) )
	ROM_LOAD( "vcbab11.bin",	0x1800, 0x0800,	CRC(e2069a6d) SHA1(2d4e71f2838451215e6f9629e2d1a35808510353) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "vdcbaorg.col",	0x0000, 0x0100,	CRC(6cdca5ae) SHA1(f7430af1adfa24fdd68a026ee431ead7d47ba269) )
ROM_END

ROM_START( babypkr )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "pok8039.old",	0x0000, 0x4000,	CRC(c5400ef1) SHA1(1f27c92d2979319070a695f71ed494f6d47fe88f) )

	ROM_REGION( 0x1000, "soundcpu", 0 )
	ROM_LOAD( "dadvpbj.son",	0x0000, 0x1000,	CRC(7b71cd30) SHA1(d782c50689a5aea632b6d274a1a7435a092ad20c) )

	ROM_REGION( 0x20000, "tiles", 0 )
	ROM_LOAD( "vpbjep15.mme",	0x00000, 0x8000,CRC(cad0f7cf) SHA1(0721b8b30dbf2a5da2967b0cfce24b4cd62d3f9d) )
	ROM_LOAD( "vpbjep14.mme",	0x08000, 0x8000,CRC(96f512fa) SHA1(f5344aeb57f53c43156e923fb7f0d8d37c73dbe9) )
	ROM_LOAD( "vpbjep12.mme",	0x10000, 0x8000,CRC(cfdca530) SHA1(609a5ad6f34e6b5c1c35584ddc62d4ff87546415) )
	ROM_LOAD( "vpbjep11.mme",	0x18000, 0x8000,CRC(44e6c489) SHA1(ca211cb3807c476cd8c5ac98b0d18b4b2724df45) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "babypok.col",	0x0000, 0x0100,	CRC(2b98e88a) SHA1(bb22ef090e9e5dddc5c160d41a5f52df0db6feb6) )
ROM_END

ROM_START( babydad )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "da400org.old",	0x0000, 0x4000,	CRC(cbca3a0c) SHA1(5d9428f26edf2c5531398a6ae36b4e9169b2c1c1) )

	ROM_REGION( 0x1000, "soundcpu", 0 )
	ROM_LOAD( "dadvpbj.son",	0x0000, 0x1000,	CRC(7b71cd30) SHA1(d782c50689a5aea632b6d274a1a7435a092ad20c) )

	ROM_REGION( 0x20000, "tiles", 0 )
	ROM_LOAD( "ep15dad.dad",	0x00000, 0x8000,CRC(21bd102d) SHA1(52788d09dbe38fa29b8ff044a1c5249cad3d45b4) )
	ROM_LOAD( "ep14dad.dad",	0x08000, 0x8000,CRC(b6e2c8a2) SHA1(352d88e1d764da5133de2be9987d4875f0c9237f) )
	ROM_LOAD( "ep12dad.dad",	0x10000, 0x8000,CRC(98702beb) SHA1(6d42ea48df7546932570da1e9b0be7a1f01f930c) )
	ROM_LOAD( "ep11dad.dad",	0x18000, 0x8000,CRC(90aac63b) SHA1(8b312f2313334b4b5b0344b786aa1a7a4979ea92) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "babydad.col",	0x0000, 0x0100,	CRC(b3358b3f) SHA1(d499a08fefaa3566de2e6fcddd237d6dfa840d8a) )
ROM_END

ROM_START( fortune1 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "pk485-s-000-7ff.c5",   0x0000, 0x0800, CRC(d74c4860) SHA1(9d151e2be5c1e9fc2e7ce5e533eb08e4b849f2c1) )
	ROM_LOAD( "pk485-s-800-fff.c7",   0x0800, 0x0800, CRC(490da6b0) SHA1(4b7afd058aeda929821d62c58e234769d64339e1) )

	ROM_REGION( 0x1000, "soundcpu", 0 )
	ROM_LOAD( "vpsona3.pbj",	0x0000, 0x0800,	CRC(a4f7bf7f) SHA1(a08287821f3471cb3e1ae0528811da930fd57387) )
	ROM_LOAD( "vpsona2.pbj",	0x0800, 0x0800,	CRC(583a9b95) SHA1(a10e85452e285b2a63f885f4e39b7f76ee8b2407) )

	ROM_REGION( 0x1000, "tiles", 0 )
	ROM_LOAD( "cg073-cg0-a.b12",	 0x0000, 0x0800, CRC(fff2d7aa) SHA1(935b8623fda5b4b25ba1aaea869ebb2baded515c) )
	ROM_LOAD( "cg073-cg1-a.b15",	 0x0800, 0x0800, CRC(a7cb05c4) SHA1(7cd76ade7cf9c50421b054ee525108829c31307c) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "3140-cap8.b8", 0x0000, 0x0100, CRC(09abf5f1) SHA1(f2d6b4f2f08b47b93728dafb50576d5ca859255f) )
ROM_END

/*************************
*      Game Drivers      *
*************************/
/*     YEAR  NAME      PARENT    MACHINE   INPUT     INIT  ROT    COMPANY                                 FULLNAME                              FLAGS  LAYOUT      */
GAMEL( 1984, videopkr, 0,        videopkr, videopkr, 0,    ROT0, "InterFlip",                             "Video Poker",                        0,     layout_videopkr )
GAMEL( 1984, fortune1, videopkr, fortune1, videopkr, 0,    ROT0, "IGT - International Gaming Technology", "Fortune I (PK485-S) Draw Poker",     0,     layout_videopkr )
GAMEL( 1984, blckjack, videopkr, blckjack, blckjack, 0,    ROT0, "InterFlip",                             "Black Jack",                         0,     layout_blckjack )
GAMEL( 1987, videodad, videopkr, videodad, videodad, 0,    ROT0, "InterFlip",                             "Video Dado",                         0,     layout_videodad )
GAMEL( 1987, videocba, videopkr, videodad, videocba, 0,    ROT0, "InterFlip",                             "Video Cordoba",                      0,     layout_videocba )
GAMEL( 1987, babypkr , videopkr, babypkr , babypkr , 0,    ROT0, "Recreativos Franco",                    "Baby Poker",                         0,     layout_babypkr  )
GAMEL( 1987, babydad , videopkr, babypkr , babydad , 0,    ROT0, "Recreativos Franco",                    "Baby Dado",                          0,     layout_babydad  )
