/****************************************************************************

    Forty-Love (c) Taito 1984

    driver by Jaroslaw Burczynski

****************************************************************************/

/*
    TO DO:
    - sprites graphics decoding could be changed to only use
      color codes 8-15 (now it decodes all 64 colors). Perhaps the same
      applies to character graphics (colors 0-7 only),

    - sprite memory needs to be buffered ?

    - controls may be wrong (BUTTON 3 - not used ?)

    - palette has 1024 colors, but only first 256 are used at the moment

    - pixel layer needs priority ?
*/

/*
Game                     : 40-0  (Forty-Love Tennis game)
Manufacturer             : Taito
Game number              : A30
Vintage                  : 1984
Game serial/model number : M4300006B ?

I dont have the wiring harness for this board, so dont know if it works.
One GFX ROM is bad though.
See A30-26.u23\A30-26.txt for details about the bad ROM.

This is a four board system - Main, Video, ROM, and Sound boards.

Details:
  Main board:    J1100004A / K1100010A <--label with K1100026A covers these
    1 x NEC D780C-1 (Z80) CPU
    1 x Motorola M68705P5S MCU
    1 x Mitsubishi M5517P SRAM chip        1 x (8bit x 2048) used
    1 x Fujitsu MB14241 ??? chip           ( DAC ????? )
    4 x Fujitsu MB81416-10 DRAM chips      4 x (4bit x 16384) 1/2 used
    1 x TD62003P (lamps/LEDs driver)
    1 x 8MHz xtal
    3 x 8way DSW

  Sound board:   J1100005A / K1100011A
    1 x NEC D780C-1 (Z80) CPU
    1 x Mitsubishi M5517P SRAM chip        1 x (8bit x 2048) used
    1 x Yamaha YM2149
    1 x OKI M5232
    1 x Fujitsu MB3731 Audio amp
    1 x 8MHz xtal

  Video board:   J1100008A / K1100025A
    4 x AMD AM93422 RAM chips              4 x (4bit x 256)     1/2 used
    2 x Mitsubishi M5517P SRAM chips       2 x (8bit x 2048)    1/2 used
    6 x Mitsubishi M53357P (=LS157)
    1 x 18.432MHz xtal

  ROM board:     J9100005A / K9100008A


ROMS:       Programmer   Device           Legend:
             Checksum                ___
A30-08.u08     AD5C       2764          \
A30-09.u09     C1E4       2764           \
A30-10.u10     C6B1       2764            \ Sound Board ROMs
A30-11.u37     ACD8       2764            /
A30-12.u38     B7C4       2764           /
A30-13.u39     6B43       2764       ___/
A30-14.u41                M68705P5S  ___> Main board MCU
A30-15.u03     2F6E       6353          \
A30-16.u01     2E97       6353           \ Video board BPROMs
A30-17.u02     2CD3       6353           / All read as 82S137
A30-18.u13     15E5       7643       ___/
A30-19.ic1     C88C       2764          \
A30-20.ic2     7B40       2764           \  ROM board ROMs.
A30-21.ic3     E2B4       2764            \ These are Program ROMs for
A30-22.ic4     8937       2764            / the main board
A30-23.ic5     A6F6       2764           /
A30-24.ic6     75DC       2764       ___/
A30-25.u22     1903       2764          \
A30-26.u23      ??        2764  BAD !    \
A30-27.u24     005B       2764            \
A30-28.u25     279F       2764             \ Video board GFX ROMs
A30-29.u62     BAA4       2764             /
A30-30.u63     0BB6       2764            /
A30-31.u64     461C       2764           /
A30-32.u65     E764       2764       ___/

Notes,
Programmer used: HiLo All-08A programmer.

Q and P connectors, provide connection between the main and video board,
via ribbon cables.
The following are board layouts, and component locations.

     Main-Board    J1100004A / K1100010A <--label with K1100026A covers these
 +--------------------------------------------------+
 |                           DSW1 DSW2 DSW3         |
 |  5517       Z80                                  |
 =                                                  |
 =  ROMskt                                          |
 P                                                  ==
 =  ROMskt     68705P5 (A30-14)                     ==
 =                                                  ==  Wiring harness
 |  ROMskt     MB14241            TD62003           ==  connector
 |                                                  ==
 =                                                  ==
 =                                                  ==
 Q       81416                 Ribbon    =          |
 =       81416                 cable     =          ===  Wiring harness
 =       81416                 to sound  S          ===  connector
 |       81416                 board --> =          ===
 | 8MHz                                  =          |
 +--------------------------------------------------+

     ROM board     J9100005A / K9100008A

 +------------------------+
 |                        |
 | ROMskt  A30-23  A30-24 |
 |                        |
 | ROMskt  A30-21  A30-22 |
 |                        |
 | ROMskt  A30-19  A30-20 |
 +------------------------+


     Video Board   J1100008A / K1100025A
 +--------------------------------------------------+
 |          A30-32                         A30-15   |
 |                           A30-28 A30-18 A30-17   |
 =          A30-31                         A30-16   |
 =                           A30-27                 |
 Q          A30-30                                  |
 =                           A30-26                 ==
 =18.432MHz A30-29                                  ==
 |     53357      53357      A30-25                 ==  Wiring harness
 |                                                  ==  connector
 =     53357  5517                                  ==
 =     53357                           53357        ==
 P     53357                     93422              |
 =                               93422              |
 =                               93422              |
 |            5517               93422              |
 |                                                  |
 +--------------------------------------------------+

     Sound Board   J1100005A / K1100011A
     (The following representation is rotated 90 degrees anti-clockwise)
                               _______  Ribbon cable
 +------------------------+   \|/       to main board
 | A30-13  A30-14  A30-15 +----------+
 |                          |||S|||  |
 |                  Z80              |
 | YM2149                     MB3731 |
 |                  A30-08           |
 |                  A30-09           |
 | M5232            A30-10           |
 |                  5517             |
 |                                   |
 |                                   |
 | 8MHz                              |
 +-----------------------------------+

     Side view

       --Sound board-------------------
    -----ROM board---
 --------Main board--------------------------------
 --------Video board-------------------------------

Details by Quench

*************************************************************

FieldDay by Taito

Same board as 40-Love

M4300048A

18.432 mhz crystal
2x m5m5517
4x am93422 (2101)

A17-15->18 6353 1024x4 prom

M4300049A (relabeled J1100004A/K1100010A)

8 mhz crystal
4x 4416
1x m5m5517
z80c

a17_14 protection processor. 28 Pin Motorolla 15-00011-01 DA68235 (Labeled 8909)
Next to MB14241

Rom Daughterboard

K9100013A (relabeled J9100005A/K9100008A)

Sound Board

K1100024A (relabeled J1100005A/K1100011A)

8 Mhz crystal
DZ80C
YM2149
Oki M5232 (6532?)

Notes - Has jumper setting for 122HZ or 61HZ)

*/

#include "emu.h"
#include "deprecat.h"
#include "cpu/z80/z80.h"
#include "cpu/m6805/m6805.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "sound/msm5232.h"
#include "includes/buggychl.h"

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



#if 0
static WRITE8_HANDLER( fortyl_coin_counter_w )
{
	coin_counter_w(space->machine, offset,data);
}
#endif


static READ8_HANDLER( fortyl_mcu_r )
{
	return buggychl_mcu_r(space, offset);
}

static READ8_HANDLER( fortyl_mcu_status_r )
{
	return buggychl_mcu_status_r(space, offset);
}

static WRITE8_HANDLER( fortyl_mcu_w )
{
	buggychl_mcu_w(space, offset, data);
}

static WRITE8_HANDLER( bank_select_w )
{

	if ((data != 0x02) && (data != 0xfd))
	{
//      logerror("WRONG BANK SELECT = %x !!!!\n",data);
//      popmessage("WRONG BANK SELECT = %x !!!!\n",data);
	}

	memory_set_bank(space->machine, "bank1", data & 1);
}

static WRITE8_HANDLER( pix1_w )
{
	buggychl_state *state = (buggychl_state *)space->machine->driver_data;
//  if (data > 7)
//      logerror("pix1 = %2x\n", data);

	state->pix1 = data;
}
static WRITE8_HANDLER( pix2_w )
{
	buggychl_state *state = (buggychl_state *)space->machine->driver_data;
//  if ((data!=0x00) && (data != 0xff))
//      logerror("pix2 = %2x\n", data);

	state->pix2[0] = state->pix2[1];
	state->pix2[1] = data;
}

#if 0
static READ8_HANDLER( pix1_r )
{
	buggychl_state *state = (buggychl_state *)space->machine->driver_data;
	return state->pix1;
}
#endif

static READ8_HANDLER( pix2_r )
{
	buggychl_state *state = (buggychl_state *)space->machine->driver_data;
	int res;
	int d1 = state->pix1 & 7;

	res = (((state->pix2[1] << (d1 + 8)) | (state->pix2[0] << d1)) & 0xff00) >> 8;

	return res;
}


/****************************************************************************
                        fake MCU (undoukai and field day)
****************************************************************************/

static const UINT8 mcu_data0[0x80] =
{
	0x0a,0x08,0x0f,0x07,0x06,0x05,0x04,0x00,
	0x0a,0x0b,0x15,0x02,0x03,0x15,0x0a,0x08,
	0x0f,0x07,0x06,0x05,0x04,0x0c,0x01,0x0c,
	0x01,0x15,0x0b,0x0a,0x00,0x0d,0x0e,0x15,
	0x0a,0x08,0x0f,0x07,0x06,0x05,0x04,0x0b,
	0x09,0x0c,0x01,0x0c,0x01,0x15,0x0a,0x08,
	0x0f,0x07,0x06,0x05,0x04,0x0c,0x0c,0x01,
	0x0c,0x0c,0xff,0xe1,0x23,0xc5,0xc9,0x15,

	0x3e,0x4a,0xcd,0x2b,0x00,0x4e,0x79,0xf6,
	0xa0,0xe6,0xa2,0xcb,0xcf,0xf5,0x0e,0x62,
	0xcd,0x35,0x00,0xcb,0xc9,0x79,0xe6,0x02,
	0x5f,0xe1,0x4b,0x69,0x3e,0x00,0x2e,0x38,
	0xb5,0x6f,0x06,0x23,0x11,0xfe,0xdf,0x1a,
	0x27,0x77,0xe6,0x01,0x5f,0x16,0x00,0xcb,
	0xfe,0x19,0x05,0x78,0x20,0xee,0xc1,0xe1,
	0x23,0xc5,0xc9,0xee,0x37,0x28,0xc0,0xc5
};

static const UINT8 mcu_data1[0x80] =
{
	0x00,0x78,0xef,0x66,0xdc,0x50,0xc2,0x33,
	0xa1,0x0c,0x74,0xd9,0x3a,0x96,0xef,0x42,
	0x90,0xd9,0x1c,0x58,0x8f,0xbe,0xe6,0x07,
	0x20,0x31,0x39,0x39,0x2f,0x1c,0x00,0xda,
	0xa9,0x6d,0x27,0xd6,0x79,0x11,0x9c,0x1b,
	0x8e,0xf3,0x4c,0x97,0xd5,0x05,0x27,0x3a,
	0x3f,0x35,0x1b,0xf3,0xbb,0x73,0x1c,0xb4,
	0x3c,0xb3,0x1a,0x6f,0xb4,0xe7,0x09,0x19,
	0x17,0x04,0xde,0xa6,0x5c,0xff,0x90,0x0e,
	0x78,0xd0,0x15,0x47,0x65,0x70,0x68,0x4c,
	0x1c,0xd9,0x82,0x18,0x99,0x07,0x60,0xa6,
	0xd8,0xf6,0x00,0x30,0x08,0x31,0x02,0xb7,
	0x81,0xfa,0x30,0x02,0x01,0x01,0x14,0x30,
	0x07,0xd6,0x7e,0x30,0x1d,0x15,0x25,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff
};

static const UINT8 mcu_data2[0x80] =
{
	0x00,0x04,0x08,0x00,0x11,0x16,0x1a,0x1f,
	0x23,0x28,0x2c,0x30,0x35,0x39,0x3d,0x42,
	0x46,0x4a,0x4f,0x53,0x57,0x5b,0x5f,0x64,
	0x68,0x6c,0x70,0x74,0x78,0x7c,0x80,0x83,
	0x87,0x8b,0x8f,0x92,0x96,0x9a,0x90,0xa1,
	0xa4,0xa7,0xa8,0xae,0xb1,0xb5,0xb8,0xbb,
	0xbe,0xc1,0xc4,0xc6,0xc9,0xcc,0xcf,0xd1,
	0xd4,0xd6,0xd9,0xdb,0xdd,0xdf,0xe2,0xe4,
	0xe6,0xe8,0xe9,0xeb,0xed,0xee,0xf0,0xf2,
	0xf3,0xf4,0xf6,0xf7,0xf8,0xf9,0xfa,0xfb,
	0xfc,0xfc,0xfd,0xfe,0xfe,0xff,0xff,0xff,
	0xff,0xff,0x00,0x3d,0x26,0xbe,0xd6,0x6c,
	0x30,0xa6,0xb7,0x81,0x01,0x04,0x14,0xb6,
	0xa4,0x97,0x02,0xb7,0x81,0x1b,0x13,0x23,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff
};


static WRITE8_HANDLER( undoukai_mcu_w )
{
	buggychl_state *state = (buggychl_state *)space->machine->driver_data;
	int ram_adr = state->mcu_ram[0x1b5] * 0x100 + state->mcu_ram[0x1b4];

	int d, i;

	//  logerror("mcu_w %02x\n", data);


	if (state->mcu_cmd != -1)
	{
		state->mcu_in[(state->mcu_cmd & 0x10) >> 4][state->mcu_cmd & 0x0f] = data;
		state->mcu_cmd = -1;
	}
	else
	{
		switch (data)
		{
			case 0xc0:
			case 0xc1:
			case 0xc2:
			case 0xc3:
			case 0xc4:
			case 0xc5:
			case 0xc6:
			case 0xc7:
				state->mcu_cmd = (data & 0x0f) | 0x10;
				break;

			case 0xb0:
			case 0xb1:
			case 0xb2:
			case 0xb3:
				state->mcu_cmd = data & 0x0f;
				break;

			case 0x30:
			case 0x31:
			case 0x32:
			case 0x33:
			case 0x34:
			case 0x35:
			case 0x36:
			case 0x37:
			case 0x38:
			case 0x39:
				state->from_mcu = state->mcu_out[0][data & 0x0f];
				break;

			case 0x40:
			case 0x41:
			case 0x42:
				state->from_mcu = state->mcu_out[1][data & 0x0f];
				break;


			case 0x01:
				state->mcu_out[0][0] = (state->mcu_in[0][0] ^ (state->mcu_in[0][0] >> 4)) & 0x0f;
				break;

			case 0x02:
				if (state->mcu_in[0][3] != 0x00)
				{
					state->mcu_out[0][1] = 0x0c;
					state->mcu_out[0][2] = 0x00;
				}
				else
				{
					state->mcu_out[0][2] = 0xa2;
					switch (state->mcu_in[0][0] & 0x03)
					{
						case 0: state->mcu_out[0][1] = 0x55; break;
						case 1: state->mcu_out[0][1] = 0x3d; break;
						case 2: state->mcu_out[0][1] = 0x45; break;
						case 3: state->mcu_out[0][1] = 0x4d; break;
					}
				}
				break;

			case 0x03:
				state->mcu_out[0][1] = (((state->mcu_in[0][0] * 8) & 0x38) -1) & 0xff ;

				if (state->mcu_in[0][1] | state->mcu_in[0][2])
					d = 0x40;
				else
					d = 0x00;

				for (i = 0; i < 8; i++)
					state->mcu_out[0][i + 2] = mcu_data0[((state->mcu_out[0][1] + i) & 0x3f) + d];
				break;

			case 0x04:
				state->mcu_out[0][0] = ((state->mcu_in[0][0] & 0x0f) << 4) + (state->mcu_in[0][1] & 0x0f);
				state->mcu_out[0][1] = ((state->mcu_in[0][2] & 0x0f) << 4) + (state->mcu_in[0][3] & 0x0f);
				break;

			case 0x05:
//              state->mcu_out[0][0] = 255 * cos(PI * state->mcu_in[0][0] / 180);
//              state->mcu_out[0][1] = 255 * sin(PI * state->mcu_in[0][0] / 180);

				d = state->mcu_in[0][0] & 0x7f;
				state->mcu_out[0][0] = mcu_data1[d];
				state->mcu_out[0][1] = mcu_data2[d];
				break;

			case 0x06:
				if (state->mcu_in[0][0] != 0x00)
					state->mcu_out[0][0] = 0xfa;
				else
					switch (state->mcu_in[0][1])
					{
						case 0x00: state->mcu_out[0][0] = 0x02; break;
						case 0x01: state->mcu_out[0][0] = 0x01; break;
						case 0x02: state->mcu_out[0][0] = 0x01; break;
						case 0x03: state->mcu_out[0][0] = 0x04; break;
						case 0x04: state->mcu_out[0][0] = 0x01; break;
						case 0x05: state->mcu_out[0][0] = 0x14; break;
						case 0x06: state->mcu_out[0][0] = 0x14; break;
						case 0x07: state->mcu_out[0][0] = 0xb6; break;
						default:
						//  popmessage("cmd06: %02x %02x", state->mcu_in[0][0], state->mcu_in[0][1]);
							logerror("cmd06: %02x %02x\n", state->mcu_in[0][0], state->mcu_in[0][1]);
					}
				break;

			case 0x07:
				switch (state->mcu_in[0][0] & 7)
				{
					case 0: state->mcu_out[0][0] = 0x1d; break;
					case 1: state->mcu_out[0][0] = 0x1b; break;
					case 2: state->mcu_out[0][0] = 0x15; break;
					case 3: state->mcu_out[0][0] = 0x13; break;
					case 4: state->mcu_out[0][0] = 0x25; break;
					case 5: state->mcu_out[0][0] = 0x23; break;
					case 6: state->mcu_out[0][0] = 0xff; break;
					case 7: state->mcu_out[0][0] = 0xff; break;
				}
				break;

			case 0x0e:

				if(ram_adr >= 0xa000 && ram_adr < 0xa800)
				{
					ram_adr = ram_adr - 0xa000;
					state->mcu_out[1][0] = state->mcu_ram[ram_adr];
					state->mcu_out[1][1] = state->mcu_ram[ram_adr + 1];
					state->mcu_out[1][2] = state->mcu_ram[ram_adr + 2] & 0x0f;
				}
				break;

			default:
				state->from_mcu = 0x5d;

//              popmessage("unknown cmd%02x: %02x %02x %02x %02x", data, state->mcu_in[0][0], state->mcu_in[0][1], state->mcu_in[0][2], state->mcu_in[0][3]);
//              logerror("unknown cmd%02x: %02x %02x %02x %02x\n", data, state->mcu_in[0][0], state->mcu_in[0][1], state->mcu_in[0][2], state->mcu_in[0][3]);
		}
	}
}

static READ8_HANDLER( undoukai_mcu_r )
{
	buggychl_state *state = (buggychl_state *)space->machine->driver_data;

	//  logerror("mcu_r %02x\n", state->from_mcu);

	return state->from_mcu;
}

static READ8_HANDLER( undoukai_mcu_status_r )
{
	int res = 3;

	return res;
}

/***************************************************************************/

static DRIVER_INIT( undoukai )
{
	buggychl_state *state = (buggychl_state *)machine->driver_data;
	UINT8 *ROM = memory_region(machine, "maincpu");
	memory_configure_bank(machine, "bank1", 0, 2, &ROM[0x10000], 0x2000);

	state->pix_color[0] = 0x000;
	state->pix_color[1] = 0x1e3;
	state->pix_color[2] = 0x16c;
	state->pix_color[3] = 0x1ec;
}

static DRIVER_INIT( 40love )
{
	buggychl_state *state = (buggychl_state *)machine->driver_data;
	UINT8 *ROM = memory_region(machine, "maincpu");
	memory_configure_bank(machine, "bank1", 0, 2, &ROM[0x10000], 0x2000);

	#if 0
		/* character ROM hack
            to show a white line on the opponent side */

		UINT8 *ROM = memory_region(machine, "gfx2");
		int adr = 0x10 * 0x022b;
		ROM[adr + 0x000a] = 0x00;
		ROM[adr + 0x000b] = 0x00;
		ROM[adr + 0x400a] = 0x00;
		ROM[adr + 0x400b] = 0x00;
	#endif

	state->pix_color[0] = 0x000;
	state->pix_color[1] = 0x1e3;
	state->pix_color[2] = 0x16c;
	state->pix_color[3] = 0x1ec;
}

/***************************************************************************/

static READ8_HANDLER( from_snd_r )
{
	buggychl_state *state = (buggychl_state *)space->machine->driver_data;
	state->snd_flag = 0;
	return state->snd_data;
}

static READ8_HANDLER( snd_flag_r )
{
	buggychl_state *state = (buggychl_state *)space->machine->driver_data;
	return state->snd_flag | 0xfd;
}

static WRITE8_HANDLER( to_main_w )
{
	buggychl_state *state = (buggychl_state *)space->machine->driver_data;
	state->snd_data = data;
	state->snd_flag = 2;
}

/***************************************************************************/

static ADDRESS_MAP_START( 40love_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM	/* M5517P on main board */
	AM_RANGE(0x8800, 0x8800) AM_READWRITE(fortyl_mcu_r, fortyl_mcu_w)
	AM_RANGE(0x8801, 0x8801) AM_READWRITE(fortyl_mcu_status_r, pix1_w)		//pixel layer related
	AM_RANGE(0x8802, 0x8802) AM_WRITE(bank_select_w)
	AM_RANGE(0x8803, 0x8803) AM_READWRITE(pix2_r, pix2_w)		//pixel layer related
	AM_RANGE(0x8804, 0x8804) AM_READWRITE(from_snd_r, sound_command_w)
	AM_RANGE(0x8805, 0x8805) AM_READ(snd_flag_r) AM_WRITENOP /*sound_reset*/ //????
	AM_RANGE(0x8807, 0x8807) AM_READNOP /* unknown */
	AM_RANGE(0x8808, 0x8808) AM_READ_PORT("DSW3")
	AM_RANGE(0x8809, 0x8809) AM_READ_PORT("P1")
	AM_RANGE(0x880a, 0x880a) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x880b, 0x880b) AM_READ_PORT("P2")
	AM_RANGE(0x880c, 0x880c) AM_READ_PORT("DSW1") AM_WRITE(fortyl_pixram_sel_w) /* pixram bank select */
	AM_RANGE(0x880d, 0x880d) AM_READ_PORT("DSW2") AM_WRITENOP /* unknown */
	AM_RANGE(0x9000, 0x97ff) AM_READWRITE(fortyl_bg_videoram_r, fortyl_bg_videoram_w) AM_BASE_MEMBER(buggychl_state, videoram)		/* #1 M5517P on video board */
	AM_RANGE(0x9800, 0x983f) AM_RAM AM_BASE_MEMBER(buggychl_state, video_ctrl)			/* video control area */
	AM_RANGE(0x9840, 0x987f) AM_RAM AM_BASE_SIZE_MEMBER(buggychl_state, spriteram, spriteram_size)	/* sprites part 1 */
	AM_RANGE(0x9880, 0x98bf) AM_READWRITE(fortyl_bg_colorram_r, fortyl_bg_colorram_w) AM_BASE_MEMBER(buggychl_state, colorram)		/* background attributes (2 bytes per line) */
	AM_RANGE(0x98c0, 0x98ff) AM_RAM AM_BASE_SIZE_MEMBER(buggychl_state, spriteram2, spriteram2_size)/* sprites part 2 */
	AM_RANGE(0xa000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xffff) AM_READWRITE(fortyl_pixram_r, fortyl_pixram_w) /* banked pixel layer */
ADDRESS_MAP_END

static ADDRESS_MAP_START( undoukai_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x9fff) AM_ROMBANK("bank1")
	AM_RANGE(0xa000, 0xa7ff) AM_RAM AM_BASE_MEMBER(buggychl_state, mcu_ram) /* M5517P on main board */
	AM_RANGE(0xa800, 0xa800) AM_READWRITE(undoukai_mcu_r, undoukai_mcu_w)
	AM_RANGE(0xa801, 0xa801) AM_READWRITE(undoukai_mcu_status_r, pix1_w)		//pixel layer related
	AM_RANGE(0xa802, 0xa802) AM_WRITE(bank_select_w)
	AM_RANGE(0xa803, 0xa803) AM_READWRITE(pix2_r, pix2_w)		//pixel layer related
	AM_RANGE(0xa804, 0xa804) AM_READWRITE(from_snd_r, sound_command_w)
	AM_RANGE(0xa805, 0xa805) AM_READ(snd_flag_r) AM_WRITENOP /*sound_reset*/	//????
	AM_RANGE(0xa807, 0xa807) AM_READNOP AM_WRITENOP /* unknown */
	AM_RANGE(0xa808, 0xa808) AM_READ_PORT("DSW3")
	AM_RANGE(0xa809, 0xa809) AM_READ_PORT("P1")
	AM_RANGE(0xa80a, 0xa80a) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xa80b, 0xa80b) AM_READ_PORT("P2")
	AM_RANGE(0xa80c, 0xa80c) AM_READ_PORT("DSW1") AM_WRITE(fortyl_pixram_sel_w) /* pixram bank select */
	AM_RANGE(0xa80d, 0xa80d) AM_READ_PORT("DSW2") AM_WRITENOP /* unknown */
	AM_RANGE(0xb000, 0xb7ff) AM_READWRITE(fortyl_bg_videoram_r, fortyl_bg_videoram_w) AM_BASE_MEMBER(buggychl_state, videoram)		/* #1 M5517P on video board */
	AM_RANGE(0xb800, 0xb83f) AM_RAM AM_BASE_MEMBER(buggychl_state, video_ctrl)			/* video control area */
	AM_RANGE(0xb840, 0xb87f) AM_RAM AM_BASE_SIZE_MEMBER(buggychl_state, spriteram, spriteram_size)	/* sprites part 1 */
	AM_RANGE(0xb880, 0xb8bf) AM_READWRITE(fortyl_bg_colorram_r, fortyl_bg_colorram_w) AM_BASE_MEMBER(buggychl_state, colorram)		/* background attributes (2 bytes per line) */
	AM_RANGE(0xb8e0, 0xb8ff) AM_RAM AM_BASE_SIZE_MEMBER(buggychl_state, spriteram2, spriteram2_size) /* sprites part 2 */
	AM_RANGE(0xc000, 0xffff) AM_READWRITE(fortyl_pixram_r, fortyl_pixram_w)
ADDRESS_MAP_END

static MACHINE_RESET( ta7630 )
{
	buggychl_state *state = (buggychl_state *)machine->driver_data;
	int i;

	double db			= 0.0;
	double db_step		= 1.50;	/* 1.50 dB step (at least, maybe more) */
	double db_step_inc	= 0.125;
	for (i = 0; i < 16; i++)
	{
		double max = 100.0 / pow(10.0, db/20.0);
		state->vol_ctrl[15 - i] = max;
		/*logerror("vol_ctrl[%x] = %i (%f dB)\n", 15 - i, state->vol_ctrl[15 - i], db);*/
		db += db_step;
		db_step += db_step_inc;
	}

	/* for (i = 0; i < 8; i++)
        logerror("SOUND Chan#%i name=%s\n", i, mixer_get_name(i) ); */
/*
  channels 0-2 AY#0
  channels 3,4 MSM5232 group1,group2
*/
}

static WRITE8_DEVICE_HANDLER( sound_control_0_w )
{
	buggychl_state *state = (buggychl_state *)device->machine->driver_data;
	state->snd_ctrl0 = data & 0xff;
//  popmessage("SND0 0=%02x 1=%02x 2=%02x 3=%02x", state->snd_ctrl0, state->snd_ctrl1, state->snd_ctrl2, state->snd_ctrl3);

	/* this definitely controls main melody voice on 2'-1 and 4'-1 outputs */
	sound_set_output_gain(device, 0, state->vol_ctrl[(state->snd_ctrl0 >> 4) & 15] / 100.0);	/* group1 from msm5232 */
	sound_set_output_gain(device, 1, state->vol_ctrl[(state->snd_ctrl0 >> 4) & 15] / 100.0);	/* group1 from msm5232 */
	sound_set_output_gain(device, 2, state->vol_ctrl[(state->snd_ctrl0 >> 4) & 15] / 100.0);	/* group1 from msm5232 */
	sound_set_output_gain(device, 3, state->vol_ctrl[(state->snd_ctrl0 >> 4) & 15] / 100.0);	/* group1 from msm5232 */

}
static WRITE8_DEVICE_HANDLER( sound_control_1_w )
{
	buggychl_state *state = (buggychl_state *)device->machine->driver_data;
	state->snd_ctrl1 = data & 0xff;
//  popmessage("SND1 0=%02x 1=%02x 2=%02x 3=%02x", state->snd_ctrl0, state->snd_ctrl1, state->snd_ctrl2, state->snd_ctrl3);
	sound_set_output_gain(device, 4, state->vol_ctrl[(state->snd_ctrl1 >> 4) & 15] / 100.0);	/* group2 from msm5232 */
	sound_set_output_gain(device, 5, state->vol_ctrl[(state->snd_ctrl1 >> 4) & 15] / 100.0);	/* group2 from msm5232 */
	sound_set_output_gain(device, 6, state->vol_ctrl[(state->snd_ctrl1 >> 4) & 15] / 100.0);	/* group2 from msm5232 */
	sound_set_output_gain(device, 7, state->vol_ctrl[(state->snd_ctrl1 >> 4) & 15] / 100.0);	/* group2 from msm5232 */
}

static WRITE8_DEVICE_HANDLER( sound_control_2_w )
{
	buggychl_state *state = (buggychl_state *)device->machine->driver_data;
	int i;
	state->snd_ctrl2 = data & 0xff;
//  popmessage("SND2 0=%02x 1=%02x 2=%02x 3=%02x", state->snd_ctrl0, state->snd_ctrl1, state->snd_ctrl2, state->snd_ctrl3);

	for (i = 0; i < 3; i++)
		sound_set_output_gain(device, i, state->vol_ctrl[(state->snd_ctrl2 >> 4) & 15] / 100.0);	/* ym2149f all */
}

static WRITE8_DEVICE_HANDLER( sound_control_3_w ) /* unknown */
{
	buggychl_state *state = (buggychl_state *)device->machine->driver_data;
	state->snd_ctrl3 = data & 0xff;
//  popmessage("SND3 0=%02x 1=%02x 2=%02x 3=%02x", state->snd_ctrl0, state->snd_ctrl1, state->snd_ctrl2, state->snd_ctrl3);
}

static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
	AM_RANGE(0xc800, 0xc801) AM_DEVWRITE("aysnd", ay8910_address_data_w)
	AM_RANGE(0xca00, 0xca0d) AM_DEVWRITE("msm", msm5232_w)
	AM_RANGE(0xcc00, 0xcc00) AM_DEVWRITE("msm", sound_control_0_w)
	AM_RANGE(0xce00, 0xce00) AM_DEVWRITE("msm", sound_control_1_w)
	AM_RANGE(0xd800, 0xd800) AM_READWRITE(soundlatch_r, to_main_w)
	AM_RANGE(0xda00, 0xda00) AM_READNOP AM_WRITE(nmi_enable_w) /* unknown read */
	AM_RANGE(0xdc00, 0xdc00) AM_WRITE(nmi_disable_w)
	AM_RANGE(0xde00, 0xde00) AM_READNOP AM_DEVWRITE("dac", dac_signed_w)		/* signed 8-bit DAC - unknown read */
	AM_RANGE(0xe000, 0xefff) AM_ROM /* space for diagnostics ROM */
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


static INPUT_PORTS_START( 40love )
	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW1:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW1:2" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )	PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x10, DEF_STR( Lives ) )		PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x18, "4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW1:6" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Flip_Screen ) )	PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )		PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("DSW2") /* All OK */
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Coin_A ) )		PORT_DIPLOCATION("SW2:1,2,3,4")
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
	PORT_DIPNAME( 0xf0, 0x00, DEF_STR( Coin_B ) )		PORT_DIPLOCATION("SW2:5,6,7,8")
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
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW3:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW3:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW3:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW3:4" )
	PORT_DIPNAME( 0x10, 0x10, "Display Credit Settings" )	PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Year Display" )				PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Score points to: (Cheat)")	PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x40, "Winner" )
	PORT_DIPSETTING(    0x00, "Human" )
	PORT_DIPNAME( 0x80, 0x00, "Coin Door Type" )			PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x00, "Single Slot" )
	PORT_DIPSETTING(    0x80, "Double Slot" )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )	//??
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )	//??
	PORT_BIT( 0x04, IP_ACTIVE_HIGH,IPT_COIN1 )	//OK
	PORT_BIT( 0x08, IP_ACTIVE_HIGH,IPT_COIN2 )	//OK
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )	//OK
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )	//OK
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )	//OK
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_TILT )	//OK

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_COCKTAIL
INPUT_PORTS_END

static INPUT_PORTS_START( undoukai )
	PORT_INCLUDE( 40love )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )	PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "4 (Hard)" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "1 (Easy)" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Lives ) )		PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Bonus_Life ) )	PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( None ) )
	PORT_DIPSETTING(    0x00, "100000 200000" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Players ) )		PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, "1 Or 2" )
	PORT_DIPSETTING(    0x00, "1 To 4" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )	PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_MODIFY("DSW3") /* & START */
	PORT_BIT(           0x01, IP_ACTIVE_LOW, IPT_START2 )	PORT_DIPLOCATION("SW3:1")
	PORT_BIT(           0x02, IP_ACTIVE_LOW, IPT_START1 )	PORT_DIPLOCATION("SW3:2")
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Demo_Sounds ) )		PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Freeze" )					PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "No Qualify (Cheat)")			PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("P1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START4 )

	PORT_MODIFY("P2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static const gfx_layout char_layout =
{
	8,8,
	0x400,
	4,
	{ 2*0x2000*8+0, 2*0x2000*8+4, 0,4 },
	{ 3,2,1,0, 11,10,9,8 },
	{ 0*8,2*8,4*8,6*8,8*8,10*8,12*8,14*8 },
	16*8
};

static const gfx_layout sprite_layout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4, 0,4 },

	{ 3,2,1,0, 11,10,9,8,
		16*8+3, 16*8+2, 16*8+1, 16*8+0, 16*8+11, 16*8+10, 16*8+9, 16*8+8 },

	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			16*16, 17*16, 18*16, 19*16, 20*16, 21*16, 22*16, 23*16 },
	64*8
};


static GFXDECODE_START( 40love )
	GFXDECODE_ENTRY( "gfx2", 0, char_layout, 0, 64 )
	GFXDECODE_ENTRY( "gfx1", 0, sprite_layout, 0, 64 )
GFXDECODE_END

static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DEVICE_HANDLER("aysnd", sound_control_2_w),
	DEVCB_HANDLER(sound_control_3_w)
};

static const msm5232_interface msm5232_config =
{
	{ 1.0e-6, 1.0e-6, 1.0e-6, 1.0e-6, 1.0e-6, 1.0e-6, 1.0e-6, 1.0e-6 }	/* 1.0 uF capacitors (verified on real PCB) */
};

/*******************************************************************************/

static MACHINE_START( common )
{
	buggychl_state *state = (buggychl_state *)machine->driver_data;

	state->audiocpu = machine->device("audiocpu");
	state->mcu = machine->device("mcu");

	/* video */
	state_save_register_global(machine, state->pix1);
	state_save_register_global_array(machine, state->pix2);
	/* sound */
	state_save_register_global(machine, state->sound_nmi_enable);
	state_save_register_global(machine, state->pending_nmi);
	state_save_register_global(machine, state->snd_data);
	state_save_register_global(machine, state->snd_flag);
	state_save_register_global_array(machine, state->vol_ctrl);
	state_save_register_global(machine, state->snd_ctrl0);
	state_save_register_global(machine, state->snd_ctrl1);
	state_save_register_global(machine, state->snd_ctrl2);
	state_save_register_global(machine, state->snd_ctrl3);
}

static MACHINE_START( 40love )
{
	buggychl_state *state = (buggychl_state *)machine->driver_data;

	MACHINE_START_CALL(common);

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

static MACHINE_START( undoukai )
{
	buggychl_state *state = (buggychl_state *)machine->driver_data;

	MACHINE_START_CALL(common);

	/* fake mcu */
	state_save_register_global(machine, state->from_mcu);
	state_save_register_global(machine, state->mcu_cmd);
	state_save_register_global_array(machine, state->mcu_in[0]);
	state_save_register_global_array(machine, state->mcu_in[1]);
	state_save_register_global_array(machine, state->mcu_out[0]);
	state_save_register_global_array(machine, state->mcu_out[1]);
}

static MACHINE_RESET( common )
{
	buggychl_state *state = (buggychl_state *)machine->driver_data;

	MACHINE_RESET_CALL(ta7630);

	/* video */
	state->pix1 = 0;
	state->pix2[0] = 0;
	state->pix2[1] = 0;
	/* sound */
	state->sound_nmi_enable = 0;
	state->pending_nmi = 0;
	state->snd_data = 0;
	state->snd_flag = 0;
	state->snd_ctrl0 = 0;
	state->snd_ctrl1 = 0;
	state->snd_ctrl2 = 0;
	state->snd_ctrl3 = 0;
}

static MACHINE_RESET( 40love )
{
	buggychl_state *state = (buggychl_state *)machine->driver_data;

	cputag_set_input_line(machine, "mcu", 0, CLEAR_LINE);

	MACHINE_RESET_CALL(common);

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

static MACHINE_RESET( undoukai )
{
	buggychl_state *state = (buggychl_state *)machine->driver_data;
	int i;

	MACHINE_RESET_CALL(common);

	/* fake mcu */
	state->from_mcu = 0xff;
	state->mcu_cmd = -1;

	for (i = 0; i < 16; i++)
	{
		state->mcu_in[0][i] = 0;
		state->mcu_in[1][i] = 0;
		state->mcu_out[0][i] = 0;
		state->mcu_out[1][i] = 0;
	}
}

static MACHINE_DRIVER_START( 40love )

	/* driver data */
	MDRV_DRIVER_DATA(buggychl_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu",Z80,8000000/2) /* OK */
	MDRV_CPU_PROGRAM_MAP(40love_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_CPU_ADD("audiocpu",Z80,8000000/2) /* OK */
	MDRV_CPU_PROGRAM_MAP(sound_map)
	MDRV_CPU_VBLANK_INT_HACK(irq0_line_hold,2)	/* source/number of IRQs is unknown */

	MDRV_CPU_ADD("mcu",M68705,18432000/6) /* OK */
	MDRV_CPU_PROGRAM_MAP(mcu_map)

	MDRV_QUANTUM_TIME(HZ(6000))	/* high interleave to ensure proper synchronization of CPUs */
	MDRV_MACHINE_START(40love)
	MDRV_MACHINE_RESET(40love)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(128,128+255, 2*8, 30*8-1)
	MDRV_GFXDECODE(40love)
	MDRV_PALETTE_LENGTH(1024)

	MDRV_PALETTE_INIT(fortyl)
	MDRV_VIDEO_START(fortyl)
	MDRV_VIDEO_UPDATE(fortyl)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("aysnd", AY8910, 2000000)
	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)

	MDRV_SOUND_ADD("msm", MSM5232, 8000000/4)
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

	MDRV_SOUND_ADD("dac", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.20)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( undoukai )

	/* driver data */
	MDRV_DRIVER_DATA(buggychl_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu",Z80,8000000/2)
	MDRV_CPU_PROGRAM_MAP(undoukai_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_CPU_ADD("audiocpu",Z80,8000000/2)
	MDRV_CPU_PROGRAM_MAP(sound_map)
	MDRV_CPU_VBLANK_INT_HACK(irq0_line_hold,2)	/* source/number of IRQs is unknown */

//  MDRV_CPU_ADD("mcu",M68705,18432000/6)
//  MDRV_CPU_PROGRAM_MAP(mcu_map)

	MDRV_MACHINE_START(undoukai)
	MDRV_MACHINE_RESET(undoukai)	/* init machine */

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(128,128+255, 2*8, 30*8-1)
	MDRV_GFXDECODE(40love)
	MDRV_PALETTE_LENGTH(1024)

	MDRV_PALETTE_INIT(fortyl)
	MDRV_VIDEO_START(fortyl)
	MDRV_VIDEO_UPDATE(fortyl)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("aysnd", AY8910, 2000000)
	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)

	MDRV_SOUND_ADD("msm", MSM5232, 8000000/4)
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

	MDRV_SOUND_ADD("dac", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.20)
MACHINE_DRIVER_END

/*******************************************************************************/

ROM_START( 40love )
	ROM_REGION( 0x14000, "maincpu", 0 ) /* Z80 main CPU */
	ROM_LOAD( "a30-19.ic1", 0x00000, 0x2000, CRC(7baca598) SHA1(b1767f5af9b3f484afb4423afe1f9c15db92c2ac) )
	ROM_LOAD( "a30-20.ic2", 0x02000, 0x2000, CRC(a7b4f2cc) SHA1(67f570874fa0feb21f2a9a0712fadf78ebaad91c) )
	ROM_LOAD( "a30-21.ic3", 0x04000, 0x2000, CRC(49a372e8) SHA1(7c15fac65369d2e90b432c0f5c8e1d7295c379d1) )
	ROM_LOAD( "a30-22.ic4", 0x06000, 0x2000, CRC(0c06d2b3) SHA1(e5b0c8e57b0a6d131496e168023e12bacc17e93e) )
	ROM_LOAD( "a30-23.ic5", 0x10000, 0x2000, CRC(6dcd186e) SHA1(c8d88a2f35ba77ea822bdd8133033c8eb0bb5f72) )	/* banked at 0xa000 */
	ROM_LOAD( "a30-24.ic6", 0x12000, 0x2000, CRC(590c20c8) SHA1(93689d6a299dfbe33ffec42d13378091d8589b34) )	/* banked at 0xa000 */

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 sound CPU */
	ROM_LOAD( "a30-08.u08", 0x0000, 0x2000, CRC(2fc42ee1) SHA1(b56e5f9acbcdc476252e188f41ad7249dba6f8e1) )
	ROM_LOAD( "a30-09.u09", 0x2000, 0x2000, CRC(3a75abce) SHA1(ad2df26789d38196c0677c22ab8f176e99604b18) )
	ROM_LOAD( "a30-10.u10", 0x4000, 0x2000, CRC(393c4b5b) SHA1(a8e1dd5c33e929bc832cccc13b85ecd13fff1eb2) )
	ROM_LOAD( "a30-11.u37", 0x6000, 0x2000, CRC(11b2c6d2) SHA1(d55690512a37c4df2386a845e0cfb14f8052295b) )
	ROM_LOAD( "a30-12.u38", 0x8000, 0x2000, CRC(f7afd475) SHA1(dd09d5ca7fec5e0454f9efb8ebc722561010f124) )
	ROM_LOAD( "a30-13.u39", 0xa000, 0x2000, CRC(e806630f) SHA1(09022aae88ea0171a0aacf3260fa3a95e8faeb21) )

	ROM_REGION( 0x0800, "mcu", 0 )	/* 2k for the microcontroller */
	ROM_LOAD( "a30-14"    , 0x0000, 0x0800, CRC(c4690279) SHA1(60bc77e03b9be434bb97a374a2fedeb8d049a660) )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "a30-25.u22", 0x0000, 0x2000, CRC(15e594cf) SHA1(d2d506a55f6ac2c191e5d5b3127021cde366c71c) )
	ROM_LOAD( "415f.26", 0x2000, 0x2000, BAD_DUMP CRC(3a45a205) SHA1(0939ecaabbb9be2a0719ef252e3f244299734ba6)  )	/* this actually seems good, but we need to find another one to verify */
	ROM_LOAD( "a30-27.u24", 0x4000, 0x2000, CRC(57c67f6f) SHA1(293e5bfa7c859886abd70f78fe2e4b13a3fce3f5) )
	ROM_LOAD( "a30-28.u25", 0x6000, 0x2000, CRC(d581d067) SHA1(ce132cf2503917f0846b838c6ce4ad4183181bf9) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "a30-29.u62", 0x0000, 0x2000, CRC(02deaf40) SHA1(fb424a40bd9d959664a6d1ddf477fc16e694b9fa) )
	ROM_LOAD( "a30-30.u63", 0x2000, 0x2000, CRC(439f3731) SHA1(4661149baa8472989cc8ac85c51e55df69957d99) )
	ROM_LOAD( "a30-31.u64", 0x4000, 0x2000, CRC(7ed70e81) SHA1(f90a3ce701ebe746803cf01ea1f6725c552007de) )
	ROM_LOAD( "a30-32.u65", 0x6000, 0x2000, CRC(0434655b) SHA1(261c5e60e830967564c053dc1d40fbf1e7194fc8) )

	ROM_REGION( 0x1000, "proms", 0 )
	ROM_LOAD( "a30-15.u03", 0x0000, 0x0400, CRC(55e38cc7) SHA1(823a6d7f29eadf5d12702d782d4297b0d4c65a0e) )	/* red */
	ROM_LOAD( "a30-16.u01", 0x0400, 0x0400, CRC(13997e20) SHA1(9fae1cf633409a88263dc66a17b1c2eeccd05f4f) )	/* green */
	ROM_LOAD( "a30-17.u02", 0x0800, 0x0400, CRC(5031f2f3) SHA1(1836d82fdc9f39cb318a791af2a935c27baabfd7) )	/* blue */
	ROM_LOAD( "a30-18.u13", 0x0c00, 0x0400, CRC(78697c0f) SHA1(31382ed4c0d44024f7f57a9de6407527f4d5b0d1) )	/* ??? */

ROM_END

ROM_START( fieldday )
	ROM_REGION( 0x14000, "maincpu", 0 ) /* Z80 main CPU  */
	ROM_LOAD( "a17_44.bin", 0x00000, 0x2000, CRC(d59812e1) SHA1(f3e7e2f09fba5964c92813cd652aa093fe3e4415) )
	ROM_LOAD( "a17_45.bin", 0x02000, 0x2000, CRC(828bfb9a) SHA1(0be24ec076b715d65e9c8e01e3be76628e4f60ed) )
	ROM_LOAD( "a23_05.bin", 0x04000, 0x2000, CRC(2670cad3) SHA1(8ba3a6b788fa4e997f9153226f6f13b32fc33124) )
	ROM_LOAD( "a23_06.bin", 0x06000, 0x2000, CRC(737ce7de) SHA1(52a46fe14978e217de81dcd529d16d62fb5a4e46) )
	ROM_LOAD( "a23_07.bin", 0x10000, 0x2000, CRC(ee2fb306) SHA1(f2b0a6af279b459fe61d56ba4d36d519318376fb) )
	ROM_LOAD( "a23_08.bin", 0x12000, 0x2000, CRC(1ed2f1ad) SHA1(e3cf954dd2c34759147d0c85da7a716a8eb0e820) )
	ROM_COPY( "maincpu" , 0x10000, 0x8000, 0x2000 ) /* to avoid 'bank bug' */

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 sound CPU */
	ROM_LOAD( "a17_24.bin", 0x0000, 0x2000, CRC(6bac6b7f) SHA1(eb95192204a868737d609b789312ac37c31d3071) )
	ROM_LOAD( "a17_25.bin", 0x2000, 0x2000, CRC(570b90b1) SHA1(2a8c3bebd15655ffbfeaf40c2db90292afbb11ef) )
	ROM_LOAD( "a17_26.bin", 0x4000, 0x2000, CRC(7a8ea7f4) SHA1(1d9d2b54645266f95aa89cdbec6f82d4ac20d6e4) )
	ROM_LOAD( "a17_27.bin", 0x6000, 0x2000, CRC(e10594d9) SHA1(3df15b8b0c7af9fed93eca27237e15e6a7a8f835) )
	ROM_LOAD( "a17_28.bin", 0x8000, 0x2000, CRC(1a4d1dae) SHA1(fbc3c55ad9f15ead432c136eec648fe22e523ea7) )
	ROM_LOAD( "a17_29.bin", 0xa000, 0x2000, CRC(3c540007) SHA1(549e7ff260214c538913ff548dcb088987845911) )

	ROM_REGION( 0x0800, "cpu2", 0 )	/* 2k for the microcontroller */
	ROM_LOAD( "a17_14.bin", 0x0000, 0x0800, NO_DUMP )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "a17_36.bin", 0x0000, 0x2000, CRC(e3dd51f7) SHA1(95a97ea925c5bc7bdc00887e6d17d817b36befc4) )
	ROM_LOAD( "a17_37.bin", 0x2000, 0x2000, CRC(1623f71f) SHA1(f5df7498b9a08e82ea11cb1b1fcdabca48cbf33a) )
	ROM_LOAD( "a17_38.bin", 0x4000, 0x2000, CRC(ca9f74db) SHA1(a002f1dfa9497793bfb18292e7a71ae12d70fb88) )
	ROM_LOAD( "a17_39.bin", 0x6000, 0x2000, CRC(fb6c667c) SHA1(da56be8d997db199588ee22fae30cc6d87e80704) )


	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "a23_09.bin", 0x0000, 0x2000, CRC(1e430be5) SHA1(9296e1a0d820bb218578d55b739b4fc5fdafb125) )
	ROM_LOAD( "a23_10.bin", 0x2000, 0x2000, CRC(ee2e54f0) SHA1(0a92fa39696a8005f9441131b6d98205b7c26e7b) )
	ROM_LOAD( "a23_11.bin", 0x4000, 0x2000, CRC(6d37f15c) SHA1(3eb9a2e230d88f2871e6972a01d8e7cc7db1b123) )
	ROM_LOAD( "a23_12.bin", 0x6000, 0x2000, CRC(86da42d2) SHA1(aa79cd954c96217ca2daf37addac168f8cca24f9) )

	ROM_REGION( 0x1000, "proms", 0 )
	ROM_LOAD( "a17-15.10v", 0x0000, 0x0400, CRC(9df472b7) SHA1(0cd9dd735238daf8e8228ba9481df57fb8925328) )	/* red */
	ROM_LOAD( "a17-16.8v",  0x0400, 0x0400, CRC(3bf1ff5f) SHA1(a0453851aefa9acdba4a86aaca8c442cb8550987) )	/* green */
	ROM_LOAD( "a17-17.9v",  0x0800, 0x0400, CRC(c42ae956) SHA1(057ce3783305c98622f7dfc0ee7d4882137a2ef8) )	/* blue */
	ROM_LOAD( "a17-18.23v", 0x0c00, 0x0400, CRC(3023a1da) SHA1(08ce4c6e99d04b358d66f0588852311d07183619) )	/* ??? */
ROM_END

ROM_START( undoukai )
	ROM_REGION( 0x14000, "maincpu", 0 ) /* Z80 main CPU  */
	ROM_LOAD( "a17-01.70c", 0x00000, 0x4000, CRC(6ce324d9) SHA1(9c5207ac897eaae5a6aa1a05a918c9cb58544664) )
	ROM_LOAD( "a17-02.71c", 0x04000, 0x4000, CRC(055c7ef1) SHA1(f974bd441b8e3621ac5f8d36104791c97051a97a) )
	ROM_LOAD( "a17-03.72c", 0x10000, 0x4000, CRC(9034a5c5) SHA1(bc3dae0dee08b6989275ac220fc76bfe61367154) ) /* banked at 0x8000 */
	ROM_COPY( "maincpu" , 0x10000, 0x8000, 0x2000 ) /* to avoid 'bank bug' */

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 sound CPU */
	ROM_LOAD( "a17-08.8s",  0x0000, 0x2000, CRC(2545aa0e) SHA1(190ef99890251e1e49b14ffd28f2badb4d0d8fbe) )
	ROM_LOAD( "a17-09.9s",  0x2000, 0x2000, CRC(57e2cdbb) SHA1(ae6187d62fb36a37be06040e0fd85e0252cdf750) )
	ROM_LOAD( "a17-10.10s", 0x4000, 0x2000, CRC(38a288fe) SHA1(af4979cae59ca2569a3663132451b9b554552a79) )
	ROM_LOAD( "a17-11.37s", 0x6000, 0x2000, CRC(036d6969) SHA1(20e03dab14d44bf3c7c6aace3b28b2826581d1c7) )
	ROM_LOAD( "a17-12.38s", 0x8000, 0x2000, CRC(cb7e6dcd) SHA1(5286c6d340c1d465caebae5dd7e3d4ff8b7f8f5e) )
	ROM_LOAD( "a17-13.39s", 0xa000, 0x2000, CRC(0a40930e) SHA1(8c4b9fa0aed67a3e269c2136ef81791fc8acd1da) )

	ROM_REGION( 0x0800, "cpu2", 0 )	/* 2k for the microcontroller */
	ROM_LOAD( "a17-14.41c", 0x0000, 0x0800, NO_DUMP )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "a17-04.18v", 0x0000, 0x4000, CRC(84dabee2) SHA1(698f12ee4201665988248853dafbf4b16dfc6517) )
	ROM_LOAD( "a17-05.19v", 0x4000, 0x4000, CRC(10bf3451) SHA1(23ebb1409c90d225ff5a13ad23d4dff1acaf904a) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "a17-06.59v", 0x0000, 0x4000, CRC(50f28ad9) SHA1(001555ad123ac85000999b1aa39c1b2568e26f46) )
	ROM_LOAD( "a17-07.60v", 0x4000, 0x4000, CRC(7a4b4238) SHA1(8e58803645e61a7144a659d403f318a8899d36e2) )

	ROM_REGION( 0x1000, "proms", 0 )
	ROM_LOAD( "a17-15.10v", 0x0000, 0x0400, CRC(9df472b7) SHA1(0cd9dd735238daf8e8228ba9481df57fb8925328) )	/* red */
	ROM_LOAD( "a17-16.8v",  0x0400, 0x0400, CRC(3bf1ff5f) SHA1(a0453851aefa9acdba4a86aaca8c442cb8550987) )	/* green */
	ROM_LOAD( "a17-17.9v",  0x0800, 0x0400, CRC(c42ae956) SHA1(057ce3783305c98622f7dfc0ee7d4882137a2ef8) )	/* blue */
	ROM_LOAD( "a17-18.23v", 0x0c00, 0x0400, CRC(3023a1da) SHA1(08ce4c6e99d04b358d66f0588852311d07183619) )	/* ??? */
ROM_END

GAME( 1984, 40love,   0,        40love,   40love,   40love,   ROT0, "Taito Corporation", "Forty-Love", GAME_SUPPORTS_SAVE | GAME_IMPERFECT_GRAPHICS )
GAME( 1984, fieldday, 0,        undoukai, undoukai, undoukai, ROT0, "Taito Corporation", "Field Day", GAME_SUPPORTS_SAVE )
GAME( 1984, undoukai, fieldday, undoukai, undoukai, undoukai, ROT0, "Taito Corporation", "The Undoukai (Japan)", GAME_SUPPORTS_SAVE )
