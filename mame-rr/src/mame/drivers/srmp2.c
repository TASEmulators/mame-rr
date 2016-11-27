/***************************************************************************

Super Real Mahjong P2
-------------------------------------
driver by Yochizo and Takahiro Nogi

  Yochizo took charge of video and I/O part.
  Takahiro Nogi took charge of sound, I/O and NVRAM part.

  ... and this is based on "seta.c" driver written by Luca Elia.

  Thanks for your reference, Takahiro Nogi and Luca Elia.


Supported games :
==================
 Super Real Mahjong Part1     (C) 1987 Seta
 Super Real Mahjong Part2     (C) 1987 Seta
 Super Real Mahjong Part3     (C) 1988 Seta
 Mahjong Yuugi (set 1)        (C) 1990 Visco
 Mahjong Yuugi (set 2)        (C) 1990 Visco
 Mahjong Pon Chin Kan (set 1) (C) 1991 Visco
 Mahjong Pon Chin Kan (set 2) (C) 1991 Visco


System specs :
===============
   CPU       : 68000 (8MHz)
   Sound     : AY8910 + MSM5205
   Chips     : X1-001, X1-002A, X1-003, X1-004x2, X0-005 x2
           X1-001, X1-002A  : Sprites
           X1-003           : Video output
           X1-004           : ???
           X1-005           : ???


Known issues :
===============
 - I/O port isn't fully analized. Currently avoid I/O error message with hack.
 - AY-3-8910 sound may be wrong.
 - CPU clock of srmp3 does not match the real machine.
 - MSM5205 clock frequency in srmp3 is wrong.


Note:
======
 - In mjyuugi and mjyuugia, DSW3 (Debug switch) is available if you
   turn on the cheat switch.


****************************************************************************/


#include "emu.h"
#include "deprecat.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "sound/ay8910.h"
#include "sound/msm5205.h"
#include "includes/srmp2.h"


/***************************************************************************

  Interrupt(s)

***************************************************************************/

static INTERRUPT_GEN( srmp2_interrupt )
{
	switch (cpu_getiloops(device))
	{
		case 0:		cpu_set_input_line(device, 4, HOLD_LINE);	break;	/* vblank */
		default:	cpu_set_input_line(device, 2, HOLD_LINE);	break;	/* sound */
	}
}


static DRIVER_INIT( srmp2 )
{
	UINT16 *RAM = (UINT16 *) memory_region(machine, "maincpu");

	/* Fix "ERROR BACK UP" and "ERROR IOX" */
	RAM[0x20c80 / 2] = 0x4e75;								// RTS
}

static DRIVER_INIT( srmp3 )
{
	UINT8 *RAM = memory_region(machine, "maincpu");

	/* BANK ROM (0x08000 - 0x1ffff) Check skip [MAIN ROM side] */
	RAM[0x00000 + 0x7b69] = 0x00;							// NOP
	RAM[0x00000 + 0x7b6a] = 0x00;							// NOP

	/* MAIN ROM (0x00000 - 0x07fff) Check skip [BANK ROM side] */
	RAM[0x08000 + 0xc10b] = 0x00;							// NOP
	RAM[0x08000 + 0xc10c] = 0x00;							// NOP
	RAM[0x08000 + 0xc10d] = 0x00;							// NOP
	RAM[0x08000 + 0xc10e] = 0x00;							// NOP
	RAM[0x08000 + 0xc10f] = 0x00;							// NOP
	RAM[0x08000 + 0xc110] = 0x00;							// NOP
	RAM[0x08000 + 0xc111] = 0x00;							// NOP

	/* "ERR IOX" Check skip [MAIN ROM side] */
	RAM[0x00000 + 0x784e] = 0x00;							// NOP
	RAM[0x00000 + 0x784f] = 0x00;							// NOP
	RAM[0x00000 + 0x7850] = 0x00;							// NOP
}

static MACHINE_RESET( srmp2 )
{
	srmp2_state *state = (srmp2_state *)machine->driver_data;

	state->port_select = 0;
}

static MACHINE_RESET( srmp3 )
{
	srmp2_state *state = (srmp2_state *)machine->driver_data;

	state->port_select = 0;
}


/***************************************************************************

  Memory Handler(s)

***************************************************************************/

static WRITE16_HANDLER( srmp2_flags_w )
{
/*
    ---- ---x : Coin Counter
    ---x ---- : Coin Lock Out
    --x- ---- : ADPCM Bank
    x--- ---- : Palette Bank
*/

	srmp2_state *state = (srmp2_state *)space->machine->driver_data;

	coin_counter_w( space->machine, 0, ((data & 0x01) >> 0) );
	coin_lockout_w( space->machine, 0, (((~data) & 0x10) >> 4) );
	state->adpcm_bank = ( (data & 0x20) >> 5 );
	state->color_bank = ( (data & 0x80) >> 7 );
}


static WRITE16_HANDLER( mjyuugi_flags_w )
{
/*
    ---- ---x : Coin Counter
    ---x ---- : Coin Lock Out
*/

	coin_counter_w( space->machine, 0, ((data & 0x01) >> 0) );
	coin_lockout_w( space->machine, 0, (((~data) & 0x10) >> 4) );
}


static WRITE16_HANDLER( mjyuugi_adpcm_bank_w )
{
/*
    ---- xxxx : ADPCM Bank
    --xx ---- : GFX Bank
*/

	srmp2_state *state = (srmp2_state *)space->machine->driver_data;

	state->adpcm_bank = (data & 0x0f);
	state->gfx_bank = ((data >> 4) & 0x03);
}


static WRITE16_DEVICE_HANDLER( srmp2_adpcm_code_w )
{
/*
    - Received data may be playing ADPCM number.
    - 0x000000 - 0x0000ff and 0x010000 - 0x0100ff are offset table.
    - When the hardware receives the ADPCM number, it refers the offset
      table and plays the ADPCM for itself.
*/

	srmp2_state *state = (srmp2_state *)device->machine->driver_data;
	UINT8 *ROM = memory_region(device->machine, "adpcm");

	state->adpcm_sptr = (ROM[((state->adpcm_bank * 0x10000) + (data << 2) + 0)] << 8);
	state->adpcm_eptr = (ROM[((state->adpcm_bank * 0x10000) + (data << 2) + 1)] << 8);
	state->adpcm_eptr  = (state->adpcm_eptr - 1) & 0x0ffff;

	state->adpcm_sptr += (state->adpcm_bank * 0x10000);
	state->adpcm_eptr += (state->adpcm_bank * 0x10000);

	msm5205_reset_w(device, 0);
	state->adpcm_data = -1;
}


static WRITE8_DEVICE_HANDLER( srmp3_adpcm_code_w )
{
/*
    - Received data may be playing ADPCM number.
    - 0x000000 - 0x0000ff and 0x010000 - 0x0100ff are offset table.
    - When the hardware receives the ADPCM number, it refers the offset
      table and plays the ADPCM for itself.
*/

	srmp2_state *state = (srmp2_state *)device->machine->driver_data;
	UINT8 *ROM = memory_region(device->machine, "adpcm");

	state->adpcm_sptr = (ROM[((state->adpcm_bank * 0x10000) + (data << 2) + 0)] << 8);
	state->adpcm_eptr = (ROM[((state->adpcm_bank * 0x10000) + (data << 2) + 1)] << 8);
	state->adpcm_eptr  = (state->adpcm_eptr - 1) & 0x0ffff;

	state->adpcm_sptr += (state->adpcm_bank * 0x10000);
	state->adpcm_eptr += (state->adpcm_bank * 0x10000);

	msm5205_reset_w(device, 0);
	state->adpcm_data = -1;
}


static void srmp2_adpcm_int(running_device *device)
{
	srmp2_state *state = (srmp2_state *)device->machine->driver_data;
	UINT8 *ROM = memory_region(device->machine, "adpcm");

	if (state->adpcm_sptr)
	{
		if (state->adpcm_data == -1)
		{
			state->adpcm_data = ROM[state->adpcm_sptr];

			if (state->adpcm_sptr >= state->adpcm_eptr)
			{
				msm5205_reset_w(device, 1);
				state->adpcm_data = 0;
				state->adpcm_sptr = 0;
			}
			else
			{
				msm5205_data_w(device, ((state->adpcm_data >> 4) & 0x0f));
			}
		}
		else
		{
			msm5205_data_w(device, ((state->adpcm_data >> 0) & 0x0f));
			state->adpcm_sptr++;
			state->adpcm_data = -1;
		}
	}
	else
	{
		msm5205_reset_w(device, 1);
	}
}


static READ16_HANDLER( srmp2_cchip_status_0_r )
{
	return 0x01;
}


static READ16_HANDLER( srmp2_cchip_status_1_r )
{
	return 0x01;
}


static READ16_HANDLER( srmp2_input_1_r )
{
/*
    ---x xxxx : Key code
    --x- ---- : Player 1 and 2 side flag
*/

	srmp2_state *state = (srmp2_state *)space->machine->driver_data;
	static const char *const keynames[] = { "KEY0", "KEY1", "KEY2", "KEY3" };

	if (!ACCESSING_BITS_0_7)
	{
		return 0xffff;
	}

	if (state->port_select != 2)			/* Panel keys */
	{
		int i, j, t;

		for (i = 0x00 ; i < 0x20 ; i += 8)
		{
			j = (i / 0x08);

			for (t = 0 ; t < 8 ; t ++)
			{
				if (!(input_port_read(space->machine, keynames[j]) & ( 1 << t )))
				{
					return (i + t);
				}
			}
		}
	}
	else								/* Analizer and memory reset keys */
	{
		return input_port_read(space->machine, "SERVICE");
	}

	return 0xffff;
}


static READ16_HANDLER( srmp2_input_2_r )
{
	if (!ACCESSING_BITS_0_7)
	{
		return 0x0001;
	}

	/* Always return 1, otherwise freeze. Maybe read I/O status */
	return 0x0001;
}


static WRITE16_HANDLER( srmp2_input_1_w )
{
	srmp2_state *state = (srmp2_state *)space->machine->driver_data;

	state->port_select = (data != 0x0000) ? 1 : 0;
}


static WRITE16_HANDLER( srmp2_input_2_w )
{
	srmp2_state *state = (srmp2_state *)space->machine->driver_data;

	state->port_select = (data == 0x0000) ? 2 : 0;
}


static WRITE8_HANDLER( srmp3_rombank_w )
{
/*
    ---x xxxx : MAIN ROM bank
    xxx- ---- : ADPCM ROM bank
*/

	srmp2_state *state = (srmp2_state *)space->machine->driver_data;
	UINT8 *ROM = memory_region(space->machine, "maincpu");
	int addr;

	state->adpcm_bank = ((data & 0xe0) >> 5);

	if (data & 0x1f) addr = ((0x10000 + (0x2000 * (data & 0x0f))) - 0x8000);
	else addr = 0x10000;

	memory_set_bankptr(space->machine, "bank1", &ROM[addr]);
}

/**************************************************************************

  Memory Map(s)

**************************************************************************/


static ADDRESS_MAP_START( srmp2_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x0c0000, 0x0c3fff) AM_RAM AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE(0x140000, 0x143fff) AM_RAM AM_BASE_MEMBER(srmp2_state,spriteram2.u16)		/* Sprites Code + X + Attr */
	AM_RANGE(0x180000, 0x180609) AM_RAM AM_BASE_MEMBER(srmp2_state,spriteram1.u16)		/* Sprites Y */
	AM_RANGE(0x1c0000, 0x1c0001) AM_WRITENOP						/* ??? */
	AM_RANGE(0x800000, 0x800001) AM_WRITE(srmp2_flags_w)			/* ADPCM bank, Color bank, etc. */
	AM_RANGE(0x900000, 0x900001) AM_READ_PORT("SYSTEM")				/* Coinage */
	AM_RANGE(0x900000, 0x900001) AM_WRITENOP						/* ??? */
	AM_RANGE(0xa00000, 0xa00001) AM_READWRITE(srmp2_input_1_r,srmp2_input_1_w)	/* I/O port 1 */
	AM_RANGE(0xa00002, 0xa00003) AM_READWRITE(srmp2_input_2_r,srmp2_input_2_w)	/* I/O port 2 */
	AM_RANGE(0xb00000, 0xb00001) AM_READ(srmp2_cchip_status_0_r)	/* Custom chip status ??? */
	AM_RANGE(0xb00000, 0xb00001) AM_DEVWRITE("msm", srmp2_adpcm_code_w)	/* ADPCM number */
	AM_RANGE(0xb00002, 0xb00003) AM_READ(srmp2_cchip_status_1_r)	/* Custom chip status ??? */
	AM_RANGE(0xc00000, 0xc00001) AM_WRITENOP						/* ??? */
	AM_RANGE(0xd00000, 0xd00001) AM_WRITENOP						/* ??? */
	AM_RANGE(0xe00000, 0xe00001) AM_WRITENOP						/* ??? */
	AM_RANGE(0xf00000, 0xf00001) AM_DEVREAD8("aysnd", ay8910_r, 0x00ff)
	AM_RANGE(0xf00000, 0xf00003) AM_DEVWRITE8("aysnd", ay8910_address_data_w, 0x00ff)
ADDRESS_MAP_END


static ADDRESS_MAP_START( mjyuugi_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x100000, 0x100001) AM_READ_PORT("SYSTEM")				/* Coinage */
	AM_RANGE(0x100000, 0x100001) AM_WRITE(mjyuugi_flags_w)			/* Coin Counter */
	AM_RANGE(0x100010, 0x100011) AM_READNOP							/* ??? */
	AM_RANGE(0x100010, 0x100011) AM_WRITE(mjyuugi_adpcm_bank_w)		/* ADPCM bank, GFX bank */
	AM_RANGE(0x200000, 0x200001) AM_READNOP							/* ??? */
	AM_RANGE(0x300000, 0x300001) AM_READNOP							/* ??? */
	AM_RANGE(0x500000, 0x500001) AM_READ_PORT("DSW3-1")				/* DSW 3-1 */
	AM_RANGE(0x500010, 0x500011) AM_READ_PORT("DSW3-2")				/* DSW 3-2 */
	AM_RANGE(0x700000, 0x7003ff) AM_RAM_WRITE(paletteram16_xRRRRRGGGGGBBBBB_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x800000, 0x800001) AM_READNOP				/* ??? */
	AM_RANGE(0x900000, 0x900001) AM_READWRITE(srmp2_input_1_r,srmp2_input_1_w)		/* I/O port 1 */
	AM_RANGE(0x900002, 0x900003) AM_READWRITE(srmp2_input_2_r,srmp2_input_2_w)		/* I/O port 2 */
	AM_RANGE(0xa00000, 0xa00001) AM_READ(srmp2_cchip_status_0_r)	/* custom chip status ??? */
	AM_RANGE(0xa00000, 0xa00001) AM_DEVWRITE("msm", srmp2_adpcm_code_w)			/* ADPCM number */
	AM_RANGE(0xa00002, 0xa00003) AM_READ(srmp2_cchip_status_1_r)	/* custom chip status ??? */
	AM_RANGE(0xb00000, 0xb00001) AM_DEVREAD8("aysnd", ay8910_r, 0x00ff)
	AM_RANGE(0xb00000, 0xb00003) AM_DEVWRITE8("aysnd", ay8910_address_data_w, 0x00ff)
	AM_RANGE(0xc00000, 0xc00001) AM_WRITENOP					/* ??? */
	AM_RANGE(0xd00000, 0xd00609) AM_RAM AM_BASE_MEMBER(srmp2_state,spriteram1.u16)	/* Sprites Y */
	AM_RANGE(0xd02000, 0xd023ff) AM_RAM							/* ??? only writes $00fa */
	AM_RANGE(0xe00000, 0xe03fff) AM_RAM AM_BASE_MEMBER(srmp2_state,spriteram2.u16)	/* Sprites Code + X + Attr */
	AM_RANGE(0xffc000, 0xffffff) AM_RAM AM_BASE_SIZE_GENERIC(nvram)
ADDRESS_MAP_END


static READ8_HANDLER( srmp3_cchip_status_0_r )
{
	return 0x01;
}

static READ8_HANDLER( srmp3_cchip_status_1_r )
{
	return 0x01;
}

static WRITE8_HANDLER( srmp3_input_1_w )
{
/*
    ---- --x- : Player 1 side flag ?
    ---- -x-- : Player 2 side flag ?
*/

	srmp2_state *state = (srmp2_state *)space->machine->driver_data;
	logerror("PC:%04X DATA:%02X  srmp3_input_1_w\n", cpu_get_pc(space->cpu), data);

	state->port_select = 0;

	{
		static int qqq01 = 0;
		static int qqq02 = 0;
		static int qqq49 = 0;
		static int qqqzz = 0;

		if (data == 0x01) qqq01++;
		else if (data == 0x02) qqq02++;
		else if (data == 0x49) qqq49++;
		else qqqzz++;

//      popmessage("%04X %04X %04X %04X", qqq01, qqq02, qqq49, qqqzz);
	}
}

static WRITE8_HANDLER( srmp3_input_2_w )
{
	srmp2_state *state = (srmp2_state *)space->machine->driver_data;

	/* Key matrix reading related ? */

	logerror("PC:%04X DATA:%02X  srmp3_input_2_w\n", cpu_get_pc(space->cpu), data);

	state->port_select = 1;

}

static READ8_HANDLER( srmp3_input_r )
{
/*
    ---x xxxx : Key code
    --x- ---- : Player 1 and 2 side flag
*/

	/* Currently I/O port of srmp3 is fully understood. */

	int keydata = 0xff;
	static const char *const keynames[] = { "KEY0", "KEY1", "KEY2", "KEY3" };

	logerror("PC:%04X          srmp3_input_r\n", cpu_get_pc(space->cpu));

	// PC:0x8903    ROM:0xC903
	// PC:0x7805    ROM:0x7805

	if ((cpu_get_pc(space->cpu) == 0x8903) || (cpu_get_pc(space->cpu) == 0x7805))	/* Panel keys */
	{
		int i, j, t;

		for (i = 0x00 ; i < 0x20 ; i += 8)
		{
			j = (i / 0x08);

			for (t = 0 ; t < 8 ; t ++)
			{
				if (!(input_port_read(space->machine, keynames[j]) & ( 1 << t )))
				{
					keydata = (i + t);
				}
			}
		}
	}

	// PC:0x8926    ROM:0xC926
	// PC:0x7822    ROM:0x7822

	if ((cpu_get_pc(space->cpu) == 0x8926) || (cpu_get_pc(space->cpu) == 0x7822))	/* Analizer and memory reset keys */
	{
		keydata = input_port_read(space->machine, "SERVICE");
	}

	return keydata;
}

static WRITE8_HANDLER( srmp3_flags_w )
{
/*
    ---- ---x : Coin Counter
    ---x ---- : Coin Lock Out
    xx-- ---- : GFX Bank
*/

	srmp2_state *state = (srmp2_state *)space->machine->driver_data;

	coin_counter_w( space->machine, 0, ((data & 0x01) >> 0) );
	coin_lockout_w( space->machine, 0, (((~data) & 0x10) >> 4) );
	state->gfx_bank = (data >> 6) & 0x03;
}


static ADDRESS_MAP_START( srmp3_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x9fff) AM_ROMBANK("bank1")							/* rom bank */
	AM_RANGE(0xa000, 0xa7ff) AM_RAM AM_BASE_SIZE_GENERIC(nvram)	/* work ram */
	AM_RANGE(0xa800, 0xa800) AM_WRITENOP							/* flag ? */
	AM_RANGE(0xb000, 0xb303) AM_RAM AM_BASE_MEMBER(srmp2_state,spriteram1.u8)				/* Sprites Y */
	AM_RANGE(0xb800, 0xb800) AM_WRITENOP							/* flag ? */
	AM_RANGE(0xc000, 0xdfff) AM_RAM AM_BASE_MEMBER(srmp2_state,spriteram2.u8)			/* Sprites Code + X + Attr */
	AM_RANGE(0xe000, 0xffff) AM_RAM AM_BASE_MEMBER(srmp2_state,spriteram3.u8)
ADDRESS_MAP_END

static ADDRESS_MAP_START( srmp3_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x20, 0x20) AM_WRITENOP								/* elapsed interrupt signal */
	AM_RANGE(0x40, 0x40) AM_READ_PORT("SYSTEM")	AM_WRITE(srmp3_flags_w)	/* coin, service | GFX bank, counter, lockout */
	AM_RANGE(0x60, 0x60) AM_WRITE(srmp3_rombank_w)						/* ROM bank select */
	AM_RANGE(0xa0, 0xa0) AM_DEVWRITE("msm", srmp3_adpcm_code_w)					/* ADPCM number */
	AM_RANGE(0xa1, 0xa1) AM_READ(srmp3_cchip_status_0_r)				/* custom chip status ??? */
	AM_RANGE(0xc0, 0xc0) AM_READWRITE(srmp3_input_r, srmp3_input_1_w)	/* key matrix | I/O ??? */
	AM_RANGE(0xc1, 0xc1) AM_READWRITE(srmp3_cchip_status_1_r, srmp3_input_2_w)	/* custom chip status ??? | I/O ??? */
	AM_RANGE(0xe0, 0xe1) AM_DEVWRITE("aysnd", ay8910_address_data_w)
	AM_RANGE(0xe2, 0xe2) AM_DEVREAD("aysnd", ay8910_r)
ADDRESS_MAP_END


/***************************************************************************

  Input Port(s)

***************************************************************************/

static INPUT_PORTS_START( seta_mjctrl )
	PORT_START("KEY0")	/* KEY MATRIX INPUT (3) */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY1")	/* KEY MATRIX INPUT (4) */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY2")	/* KEY MATRIX INPUT (5) */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY3")	/* KEY MATRIX INPUT (6) */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( srmp2 )
	PORT_START("SYSTEM")			/* Coinage (0) */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
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
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0xe0, "1 (Easy)" )
	PORT_DIPSETTING(    0xc0, "2" )
	PORT_DIPSETTING(    0xa0, "3" )
	PORT_DIPSETTING(    0x80, "4" )
	PORT_DIPSETTING(    0x60, "5" )
	PORT_DIPSETTING(    0x40, "6" )
	PORT_DIPSETTING(    0x20, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hard)" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x02, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_INCLUDE( seta_mjctrl )

	PORT_START("SERVICE")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_SERVICE3 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( srmp3 )
	PORT_START("SYSTEM")			/* Coinage (0) */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Debug Mode (Cheat)")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Open Reach of CPU" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0xe0, "1 (Easy)" )
	PORT_DIPSETTING(    0xc0, "2" )
	PORT_DIPSETTING(    0xa0, "3" )
	PORT_DIPSETTING(    0x80, "4" )
	PORT_DIPSETTING(    0x60, "5" )
	PORT_DIPSETTING(    0x40, "6" )
	PORT_DIPSETTING(    0x20, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hard)" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x02, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_INCLUDE( seta_mjctrl )

	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE3 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( mjyuugi )
	PORT_START("SYSTEM")			/* Coinage (0) */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x07, "1 (Easy)" )
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x05, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x06, "5" )
	PORT_DIPSETTING(    0x04, "6" )
	PORT_DIPSETTING(    0x08, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hard)" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, "Gal Score" )
	PORT_DIPSETTING(    0x10, "+0" )
	PORT_DIPSETTING(    0x00, "+1000" )
	PORT_DIPNAME( 0x20, 0x20, "Player Score" )
	PORT_DIPSETTING(    0x20, "+0" )
	PORT_DIPSETTING(    0x00, "+1000" )
	PORT_DIPNAME( 0x40, 0x40, "Item price initialize ?" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ) )
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_INCLUDE( seta_mjctrl )

	PORT_START("SERVICE")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_SERVICE3 )
	PORT_DIPNAME( 0x0004, 0x0004, "Debug Mode (Cheat)")
	PORT_DIPSETTING(   0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW3-1")			/* [Debug switch] */
	PORT_DIPNAME( 0x0001, 0x0001, "Debug  0 (Cheat)")
	PORT_DIPSETTING(   0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "Debug  1 (Cheat)")
	PORT_DIPSETTING(   0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Debug  2 (Cheat)")
	PORT_DIPSETTING(   0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Debug  3 (Cheat)")
	PORT_DIPSETTING(   0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x0000, DEF_STR( On ) )
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW3-2")			/* [Debug switch] */
	PORT_DIPNAME( 0x0001, 0x0001, "Debug  4 (Cheat)")
	PORT_DIPSETTING(   0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "Debug  5 (Cheat)")
	PORT_DIPSETTING(   0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Debug  6 (Cheat)")
	PORT_DIPSETTING(   0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Debug  7 (Cheat)")
	PORT_DIPSETTING(   0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x0000, DEF_STR( On ) )
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( ponchin )
	PORT_START("SYSTEM")			/* Coinage (0) */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x06, "1 (Easy)" )
	PORT_DIPSETTING(    0x05, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x07, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hard)" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x30, 0x30, "Player Score" )
	PORT_DIPSETTING(    0x30, "1000" )
	PORT_DIPSETTING(    0x20, "2000" )
//  PORT_DIPSETTING(    0x10, "1000" )
	PORT_DIPSETTING(    0x00, "3000" )
	PORT_DIPNAME( 0x40, 0x40, "Debug Mode (Cheat)")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Auto Tsumo" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_INCLUDE( seta_mjctrl )

	PORT_START("SERVICE")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_SERVICE3 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW3-1")			/* [Debug switch] */
	PORT_DIPNAME( 0x0001, 0x0001, "Debug  0 (Cheat)")
	PORT_DIPSETTING(   0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "Debug  1 (Cheat)")
	PORT_DIPSETTING(   0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Debug  2 (Cheat)")
	PORT_DIPSETTING(   0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Debug  3 (Cheat)")
	PORT_DIPSETTING(   0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x0000, DEF_STR( On ) )
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW3-2")			/* [Debug switch] */
	PORT_DIPNAME( 0x0001, 0x0001, "Debug  4 (Cheat)")
	PORT_DIPSETTING(   0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "Debug  5 (Cheat)")
	PORT_DIPSETTING(   0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Debug  6 (Cheat)")
	PORT_DIPSETTING(   0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Debug  7 (Cheat)")
	PORT_DIPSETTING(   0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x0000, DEF_STR( On ) )
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************

  Machine Driver(s)

***************************************************************************/

static const ay8910_interface srmp2_ay8910_interface =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_INPUT_PORT("DSW2"),
	DEVCB_INPUT_PORT("DSW1"),
	DEVCB_NULL,
	DEVCB_NULL
};


static const msm5205_interface msm5205_config =
{
	srmp2_adpcm_int,			/* IRQ handler */
	MSM5205_S48_4B				/* 8 KHz, 4 Bits  */
};


static const gfx_layout charlayout =
{
	16, 16,
	RGN_FRAC(1, 2),
	4,
	{ RGN_FRAC(1, 2) + 8, RGN_FRAC(1, 2) + 0, 8, 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7, 128, 129, 130, 131, 132, 133, 134, 135 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
	  16*16, 17*16, 18*16, 19*16, 20*16, 21*16, 22*16, 23*16 },
	16*16*2
};

static GFXDECODE_START( srmp2 )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 64 )
GFXDECODE_END

static GFXDECODE_START( srmp3 )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 32 )
GFXDECODE_END


static MACHINE_DRIVER_START( srmp2 )

	MDRV_DRIVER_DATA(srmp2_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000,16000000/2)				/* 8.00 MHz */
	MDRV_CPU_PROGRAM_MAP(srmp2_map)
	MDRV_CPU_VBLANK_INT_HACK(srmp2_interrupt,16)		/* Interrupt times is not understood */

	MDRV_MACHINE_RESET(srmp2)
	MDRV_NVRAM_HANDLER(generic_0fill)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(464, 256-16)
	MDRV_SCREEN_VISIBLE_AREA(16, 464-1, 8, 256-1-24)

	MDRV_GFXDECODE(srmp2)
	MDRV_PALETTE_LENGTH(1024)	/* sprites only */

	MDRV_PALETTE_INIT(srmp2)
	MDRV_VIDEO_UPDATE(srmp2)		/* just draw the sprites */

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("aysnd", AY8910, 20000000/16)
	MDRV_SOUND_CONFIG(srmp2_ay8910_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)

	MDRV_SOUND_ADD("msm", MSM5205, 384000)
	MDRV_SOUND_CONFIG(msm5205_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.45)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( srmp3 )

	MDRV_DRIVER_DATA(srmp2_state)

	/* basic machine hardware */

	MDRV_CPU_ADD("maincpu", Z80, 3500000)		/* 3.50 MHz ? */
	//      4000000,                /* 4.00 MHz ? */
	MDRV_CPU_PROGRAM_MAP(srmp3_map)
	MDRV_CPU_IO_MAP(srmp3_io_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_MACHINE_RESET(srmp3)
	MDRV_NVRAM_HANDLER(generic_0fill)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(400, 256-16)
	MDRV_SCREEN_VISIBLE_AREA(16, 400-1, 8, 256-1-24)

	MDRV_GFXDECODE(srmp3)
	MDRV_PALETTE_LENGTH(512)	/* sprites only */

	MDRV_PALETTE_INIT(srmp3)
	MDRV_VIDEO_UPDATE(srmp3)	/* just draw the sprites */

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("aysnd", AY8910, 16000000/16)
	MDRV_SOUND_CONFIG(srmp2_ay8910_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.20)

	MDRV_SOUND_ADD("msm", MSM5205, 384000)
	MDRV_SOUND_CONFIG(msm5205_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.45)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( mjyuugi )

	MDRV_DRIVER_DATA(srmp2_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000,16000000/2)				/* 8.00 MHz */
	MDRV_CPU_PROGRAM_MAP(mjyuugi_map)
	MDRV_CPU_VBLANK_INT_HACK(srmp2_interrupt,16)		/* Interrupt times is not understood */

	MDRV_MACHINE_RESET(srmp2)
	MDRV_NVRAM_HANDLER(generic_0fill)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(400, 256-16)
	MDRV_SCREEN_VISIBLE_AREA(16, 400-1, 0, 256-1-16)

	MDRV_GFXDECODE(srmp3)
	MDRV_PALETTE_LENGTH(512)			/* sprites only */

	MDRV_VIDEO_UPDATE(mjyuugi)			/* just draw the sprites */

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("aysnd", AY8910, 16000000/16)
	MDRV_SOUND_CONFIG(srmp2_ay8910_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.20)

	MDRV_SOUND_ADD("msm", MSM5205, 384000)
	MDRV_SOUND_CONFIG(msm5205_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.45)
MACHINE_DRIVER_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

/***************************************************************************

Super Real Mahjong PI
(c)1987 Seta
PO-023
M6100242A
860161517

CPU: 68000
Sound: AY-3-8910, M5205
OSC: 8.000MHz(X1) 20.000MHz(X2)
Custom chips: X1-003, X2-003(x3), X1-004(x2), X0-005
plus 2 chips covered with heatsink
Others: Battery, D8243HC

UB0-1.19     [a5960661] 831000
UB0-2.17     [b88dbeba]  |
UB0-3.18     [6302cfa8] /

UB0-4.60     [cb6f7cce]
UB0-5.61     [7b48c540]
UB0-6.62     [6c891ac5]
UB0-7.63     [60a45755]

UB0-8.64     [b58024b9] MB83256
UB0-9.65     [e28c2566]  |
UB0-10.66    [d20a935c]  |
UB0-11.67    [30957ca8] /

UB1.12       [d2ed93c8] MB7124E
UB2.13       [7e7d25f7] /

UB-3 (PAL16L8A - not dumped)

***************************************************************************/

ROM_START( srmp1 )
	ROM_REGION( 0x40000, "maincpu", 0 )					/* 68000 Code */
	ROM_LOAD16_BYTE( "ub0-2.17", 0x000000, 0x20000, CRC(71a00a3d) SHA1(8deb07a4621e0f0f1d6dd503cd7f4f826a63c255) )
	ROM_LOAD16_BYTE( "ub0-3.18", 0x000001, 0x20000, CRC(2950841b) SHA1(1859636602375b4cadbd23457a0d16bc85063ff5) )

	ROM_REGION( 0x200000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD16_BYTE( "ub0-8.64",  0x000000, 0x008000, CRC(b58024b9) SHA1(d38750fa90c1b14288884a3c2713b90f0b941bfb) )
	ROM_LOAD16_BYTE( "ub0-9.65",  0x000001, 0x008000, CRC(e28c2566) SHA1(91ce9d7138e05f9bcd31dfeaaa558d4431ae9515) )
	ROM_LOAD       ( "ubo-5.61",  0x040000, 0x040000, CRC(7b48c540) SHA1(06229caec6846581f95409204ea22f0f76684b08) )
	ROM_LOAD       ( "ubo-4.60",  0x080000, 0x040000, CRC(cb6f7cce) SHA1(27d7c2f4f998023081fac1bbacfd4b0b56edaee2) )
	ROM_LOAD16_BYTE( "ub0-10.66", 0x100000, 0x008000, CRC(d20a935c) SHA1(1d01b0ccfb6c2ea226e16e555796acd10b10e835) )
	ROM_LOAD16_BYTE( "ub0-11.67", 0x100001, 0x008000, CRC(30957ca8) SHA1(02aaa3f2f266e8f4db2d35c177546365d7836004) )
	ROM_LOAD       ( "ubo-7.63",  0x140000, 0x040000, CRC(60a45755) SHA1(22bbf024bbe2186b621389a23697e55d512b501a) )
	ROM_LOAD       ( "ubo-6.62",  0x180000, 0x040000, CRC(6c891ac5) SHA1(eab595bce16e4cdc465a5e2e029c3949a0f28629) )

	ROM_REGION( 0x020000, "adpcm", 0 )				/* Samples */
	ROM_LOAD( "ub0-1.19", 0x000000, 0x20000, CRC(5f21b48c) SHA1(1838632609c176dbaab1d88f9368c03259a5e954) )

	ROM_REGION( 0x800, "proms", 0 )					/* Color PROMs */
	ROM_LOAD( "ub1.12", 0x000, 0x200, CRC(d2ed93c8) SHA1(334d4fe6fa477758b336b138ffc306e2d6334371) )
	ROM_LOAD( "ub2.13", 0x400, 0x200, CRC(7e7d25f7) SHA1(e5bf5071567f95c3bb70347f0b86a9703f9f2e6c) )
ROM_END

ROM_START( srmp2 )
	ROM_REGION( 0x040000, "maincpu", 0 )					/* 68000 Code */
	ROM_LOAD16_BYTE( "uco-2.17", 0x000000, 0x020000, CRC(0d6c131f) SHA1(be85f2578b0ae2a072565605b7dbeb970e5e3851) )
	ROM_LOAD16_BYTE( "uco-3.18", 0x000001, 0x020000, CRC(e9fdf5f8) SHA1(aa1f8cc3f1d0ed942403c0473605775bc1537cbf) )

	ROM_REGION( 0x200000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD       ( "ubo-4.60",  0x000000, 0x040000, CRC(cb6f7cce) SHA1(27d7c2f4f998023081fac1bbacfd4b0b56edaee2) )
	ROM_LOAD       ( "ubo-5.61",  0x040000, 0x040000, CRC(7b48c540) SHA1(06229caec6846581f95409204ea22f0f76684b08) )
	ROM_LOAD16_BYTE( "uco-8.64",  0x080000, 0x040000, CRC(1ca1c7c9) SHA1(05bcca1f88d976d836944a7f5cc74a857fdf6cb9) )
	ROM_LOAD16_BYTE( "uco-9.65",  0x080001, 0x040000, CRC(ef75471b) SHA1(b9843559d36e071bc7d8d81eef44424c4566a10e) )
	ROM_LOAD       ( "ubo-6.62",  0x100000, 0x040000, CRC(6c891ac5) SHA1(eab595bce16e4cdc465a5e2e029c3949a0f28629) )
	ROM_LOAD       ( "ubo-7.63",  0x140000, 0x040000, CRC(60a45755) SHA1(22bbf024bbe2186b621389a23697e55d512b501a) )
	ROM_LOAD16_BYTE( "uco-10.66", 0x180000, 0x040000, CRC(cb6bd857) SHA1(1bd673e10416bc3ca14859cc15cd05caa7d7a625) )
	ROM_LOAD16_BYTE( "uco-11.67", 0x180001, 0x040000, CRC(199f79c0) SHA1(46f437e90ee25c242bf418c0fa1af77d6e4cafc6) )

	ROM_REGION( 0x020000, "adpcm", 0 )				/* Samples */
	ROM_LOAD( "uco-1.19", 0x000000, 0x020000, CRC(f284af8e) SHA1(f0b5ef8ae98101bf8c8885e469a5a36dd5e29129) )

	ROM_REGION( 0x000800, "proms", 0 )					/* Color PROMs */
	ROM_LOAD( "uc-1o.12", 0x000000, 0x000400, CRC(fa59b5cb) SHA1(171c4c36bd1c8e6548b34a9f6e2ff755ecf09b47) )
	ROM_LOAD( "uc-2o.13", 0x000400, 0x000400, CRC(50a33b96) SHA1(cfb6d3cb6b73d1bf484014fb340c28bc1774137d) )
ROM_END

ROM_START( srmp3 )
	ROM_REGION( 0x028000, "maincpu", 0 )					/* 68000 Code */
	ROM_LOAD( "za0-10.bin", 0x000000, 0x008000, CRC(939d126f) SHA1(7a5c7f7fbee8de11a08194d3c8f10a20f8dc2f0a) )
	ROM_CONTINUE(           0x010000, 0x018000 )

	ROM_REGION( 0x400000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD16_BYTE( "za0-02.bin", 0x000000, 0x080000, CRC(85691946) SHA1(8b91210b1b6671ba2c9ec6722e5dc40bdf44e4b5) )
	ROM_LOAD16_BYTE( "za0-04.bin", 0x000001, 0x080000, CRC(c06e7a96) SHA1(a2dfb81004ea72bfa21724374eb8533af606a5df) )
	ROM_LOAD16_BYTE( "za0-01.bin", 0x100000, 0x080000, CRC(95e0d87c) SHA1(34e6c0a95e63cf092092e27c7ba2f649ebf56507) )
	ROM_LOAD16_BYTE( "za0-03.bin", 0x100001, 0x080000, CRC(7c98570e) SHA1(26e28e67bca9954d62d72260370ea872c6058a10) )
	ROM_LOAD16_BYTE( "za0-06.bin", 0x200000, 0x080000, CRC(8b874b0a) SHA1(27fe1ccc2938e1703e484e2925a2f073064cf019) )
	ROM_LOAD16_BYTE( "za0-08.bin", 0x200001, 0x080000, CRC(3de89d88) SHA1(1e6dabe6aeee6a2613feab26b871c235bf491bfa) )
	ROM_LOAD16_BYTE( "za0-05.bin", 0x300000, 0x080000, CRC(80d3b4e6) SHA1(d31d3f904ee8463c1efbb1d106eeb3dc0dc42ab8) )
	ROM_LOAD16_BYTE( "za0-07.bin", 0x300001, 0x080000, CRC(39d15129) SHA1(62b71a82cfc39e6dab3175e03eca5ff92e854f13) )

	ROM_REGION( 0x080000, "adpcm", 0 )				/* Samples */
	ROM_LOAD( "za0-11.bin", 0x000000, 0x080000, CRC(2248c23f) SHA1(35591b51bb23dfd7fa81a05026e9ec0789bb0dde) )

	ROM_REGION( 0x000400, "proms", 0 )					/* Color PROMs */
	ROM_LOAD( "za0-12.prm", 0x000000, 0x000200, CRC(1ac5387c) SHA1(022f204dbe2374478279b586451673a08ee489c8) )
	ROM_LOAD( "za0-13.prm", 0x000200, 0x000200, CRC(4ea3d2fe) SHA1(c7d18b9c1331e08faadf33e52033c658bf2b16fc) )
ROM_END

ROM_START( mjyuugi )
	ROM_REGION( 0x080000, "maincpu", 0 )					/* 68000 Code */
	ROM_LOAD16_BYTE( "um001.001", 0x000000, 0x020000, CRC(28d5340f) SHA1(683d89987b8b794695fdb6104d8e6ff5204afafb) )
	ROM_LOAD16_BYTE( "um001.003", 0x000001, 0x020000, CRC(275197de) SHA1(2f8efa112f23f172eaef9bb732b2a253307dd896) )
	ROM_LOAD16_BYTE( "um001.002", 0x040000, 0x020000, CRC(d5dd4710) SHA1(b70c280f828af507c73ebec3209043eb7ce0ce95) )
	ROM_LOAD16_BYTE( "um001.004", 0x040001, 0x020000, CRC(c5ddb567) SHA1(1a35228439108f3d866547d94d4bafca54a710ec) )

	ROM_REGION( 0x400000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD16_BYTE( "maj-001.10",  0x000000, 0x080000, CRC(3c08942a) SHA1(59165052d7c760ac82157844d54c8dced4125259) )
	ROM_LOAD16_BYTE( "maj-001.08",  0x000001, 0x080000, CRC(e2444311) SHA1(88673d57d54ef0674c8c23a95da5d03cb9c894aa) )
	ROM_LOAD16_BYTE( "maj-001.09",  0x100000, 0x080000, CRC(a1974860) SHA1(f944026cf1aadb9c24ac689cc67d374eea17cb85) )
	ROM_LOAD16_BYTE( "maj-001.07",  0x100001, 0x080000, CRC(b1f1d118) SHA1(37d64ba662b431cf0fdee12983c95b9989eb00af) )
	ROM_LOAD16_BYTE( "maj-001.06",  0x200000, 0x080000, CRC(4c60acdd) SHA1(0ab69cc3ea4bebd9e7b139c89b5ac42a621493e2) )
	ROM_LOAD16_BYTE( "maj-001.04",  0x200001, 0x080000, CRC(0a4b2de1) SHA1(f9cddeffcdceb06053216502eb03d52abf527eb2) )
	ROM_LOAD16_BYTE( "maj-001.05",  0x300000, 0x080000, CRC(6be7047a) SHA1(22ce8c6fead9e16550047dea341983f59c3a6c28) )
	ROM_LOAD16_BYTE( "maj-001.03",  0x300001, 0x080000, CRC(c4fb6ea0) SHA1(b5cd3cf71831fecd096cd7bae6fb813504d1e0d5) )

	ROM_REGION( 0x100000, "adpcm", 0 )				/* Samples */
	ROM_LOAD( "maj-001.01", 0x000000, 0x080000, CRC(029a0b60) SHA1(d02788b8673ae73aca81f1570ff335982ac9ab40) )
	ROM_LOAD( "maj-001.02", 0x080000, 0x080000, CRC(eb28e641) SHA1(67e1d89c9b40e4a83a3783d4343d7a8121668091) )
ROM_END

ROM_START( mjyuugia )
	ROM_REGION( 0x080000, "maincpu", 0 )					/* 68000 Code */
	ROM_LOAD16_BYTE( "um_001.001", 0x000000, 0x020000, CRC(76dc0594) SHA1(4bd81616769cdc59eaf6f7921e404e166500f67f) )
	ROM_LOAD16_BYTE( "um001.003",  0x000001, 0x020000, CRC(275197de) SHA1(2f8efa112f23f172eaef9bb732b2a253307dd896) )
	ROM_LOAD16_BYTE( "um001.002",  0x040000, 0x020000, CRC(d5dd4710) SHA1(b70c280f828af507c73ebec3209043eb7ce0ce95) )
	ROM_LOAD16_BYTE( "um001.004",  0x040001, 0x020000, CRC(c5ddb567) SHA1(1a35228439108f3d866547d94d4bafca54a710ec) )

	ROM_REGION( 0x400000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD16_BYTE( "maj-001.10", 0x000000, 0x080000, CRC(3c08942a) SHA1(59165052d7c760ac82157844d54c8dced4125259) )
	ROM_LOAD16_BYTE( "maj-001.08", 0x000001, 0x080000, CRC(e2444311) SHA1(88673d57d54ef0674c8c23a95da5d03cb9c894aa) )
	ROM_LOAD16_BYTE( "maj-001.09", 0x100000, 0x080000, CRC(a1974860) SHA1(f944026cf1aadb9c24ac689cc67d374eea17cb85) )
	ROM_LOAD16_BYTE( "maj-001.07", 0x100001, 0x080000, CRC(b1f1d118) SHA1(37d64ba662b431cf0fdee12983c95b9989eb00af) )
	ROM_LOAD16_BYTE( "maj-001.06", 0x200000, 0x080000, CRC(4c60acdd) SHA1(0ab69cc3ea4bebd9e7b139c89b5ac42a621493e2) )
	ROM_LOAD16_BYTE( "maj-001.04", 0x200001, 0x080000, CRC(0a4b2de1) SHA1(f9cddeffcdceb06053216502eb03d52abf527eb2) )
	ROM_LOAD16_BYTE( "maj-001.05", 0x300000, 0x080000, CRC(6be7047a) SHA1(22ce8c6fead9e16550047dea341983f59c3a6c28) )
	ROM_LOAD16_BYTE( "maj-001.03", 0x300001, 0x080000, CRC(c4fb6ea0) SHA1(b5cd3cf71831fecd096cd7bae6fb813504d1e0d5) )

	ROM_REGION( 0x100000, "adpcm", 0 )				/* Samples */
	ROM_LOAD( "maj-001.01", 0x000000, 0x080000, CRC(029a0b60) SHA1(d02788b8673ae73aca81f1570ff335982ac9ab40) )
	ROM_LOAD( "maj-001.02", 0x080000, 0x080000, CRC(eb28e641) SHA1(67e1d89c9b40e4a83a3783d4343d7a8121668091) )
ROM_END

ROM_START( ponchin )
	ROM_REGION( 0x080000, "maincpu", 0 )					/* 68000 Code */
	ROM_LOAD16_BYTE( "um2_1_1.u22", 0x000000, 0x020000, CRC(cf88efbb) SHA1(7bd2304d365524fc5bcf3fb30752f5efec73a9f5) )
	ROM_LOAD16_BYTE( "um2_1_3.u42", 0x000001, 0x020000, CRC(e053458f) SHA1(db4a34589a08d0252d700144a6260a0f6c4e8e30) )
	ROM_LOAD16_BYTE( "um2_1_2.u29", 0x040000, 0x020000, CRC(5c2f9bcf) SHA1(e2880123373653c7e5d85fb957474e1c5774640d) )
	ROM_LOAD16_BYTE( "um2_1_4.u44", 0x040001, 0x020000, CRC(2ad4e0c7) SHA1(ca97b825af41f86ebbfc2cf88faafb240c4058d1) )

	ROM_REGION( 0x400000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD16_BYTE( "um2_1_8.u55", 0x000000, 0x080000, CRC(f74a8cb3) SHA1(d1bf712f7ef97a96fc251c7729b39e9f10aab45d) )
	ROM_LOAD16_BYTE( "um2_1_7.u43", 0x000001, 0x080000, CRC(1e87ca84) SHA1(5ddbfd92d6ed1947a3c35f3e93cbcca5059fa1f9) )
	ROM_LOAD16_BYTE( "um2_1_6.u28", 0x200000, 0x080000, CRC(b11e85a7) SHA1(02971b45791d06f88efbae8e0713d28105faf341) )
	ROM_LOAD16_BYTE( "um2_1_5.u20", 0x200001, 0x080000, CRC(a5469d11) SHA1(7e96af23c8434c32f87be1482309999d6a7b33bb) )

	ROM_REGION( 0x100000, "adpcm", 0 )				/* Samples */
	ROM_LOAD( "um2_1_9.u56",  0x000000, 0x080000, CRC(9165c79a) SHA1(854e30fc6121f7b3e5e1e5b6772757a92b63aef8) )
	ROM_LOAD( "um2_1_10.u63", 0x080000, 0x080000, CRC(53e643e9) SHA1(3b221217e8f846ae96a9a47149037cea19d97549) )
ROM_END

ROM_START( ponchina )
	ROM_REGION( 0x080000, "maincpu", 0 )					/* 68000 Code */
	ROM_LOAD16_BYTE( "u22.bin",     0x000000, 0x020000, CRC(9181de20) SHA1(03fdb289d862ff2d87249d35991bd60784e172d9) )
	ROM_LOAD16_BYTE( "um2_1_3.u42", 0x000001, 0x020000, CRC(e053458f) SHA1(db4a34589a08d0252d700144a6260a0f6c4e8e30) )
	ROM_LOAD16_BYTE( "um2_1_2.u29", 0x040000, 0x020000, CRC(5c2f9bcf) SHA1(e2880123373653c7e5d85fb957474e1c5774640d) )
	ROM_LOAD16_BYTE( "um2_1_4.u44", 0x040001, 0x020000, CRC(2ad4e0c7) SHA1(ca97b825af41f86ebbfc2cf88faafb240c4058d1) )

	ROM_REGION( 0x400000, "gfx1", 0 )	/* Sprites */
	ROM_LOAD16_BYTE( "um2_1_8.u55", 0x000000, 0x080000, CRC(f74a8cb3) SHA1(d1bf712f7ef97a96fc251c7729b39e9f10aab45d) )
	ROM_LOAD16_BYTE( "um2_1_7.u43", 0x000001, 0x080000, CRC(1e87ca84) SHA1(5ddbfd92d6ed1947a3c35f3e93cbcca5059fa1f9) )
	ROM_LOAD16_BYTE( "um2_1_6.u28", 0x200000, 0x080000, CRC(b11e85a7) SHA1(02971b45791d06f88efbae8e0713d28105faf341) )
	ROM_LOAD16_BYTE( "um2_1_5.u20", 0x200001, 0x080000, CRC(a5469d11) SHA1(7e96af23c8434c32f87be1482309999d6a7b33bb) )

	ROM_REGION( 0x100000, "adpcm", 0 )				/* Samples */
	ROM_LOAD( "um2_1_9.u56",  0x000000, 0x080000, CRC(9165c79a) SHA1(854e30fc6121f7b3e5e1e5b6772757a92b63aef8) )
	ROM_LOAD( "um2_1_10.u63", 0x080000, 0x080000, CRC(53e643e9) SHA1(3b221217e8f846ae96a9a47149037cea19d97549) )
ROM_END


GAME( 1987, srmp1,     0,        srmp2,    srmp2,    0,       ROT0, "Seta",  "Super Real Mahjong Part 1 (Japan)",  0 )
GAME( 1987, srmp2,     0,        srmp2,    srmp2,    srmp2,   ROT0, "Seta",  "Super Real Mahjong Part 2 (Japan)",  0 )
GAME( 1988, srmp3,     0,        srmp3,    srmp3,    srmp3,   ROT0, "Seta",  "Super Real Mahjong Part 3 (Japan)",  0 )
GAME( 1990, mjyuugi,   0,        mjyuugi,  mjyuugi,  0,       ROT0, "Visco", "Mahjong Yuugi (Japan set 1)",        0 )
GAME( 1990, mjyuugia,  mjyuugi,  mjyuugi,  mjyuugi,  0,       ROT0, "Visco", "Mahjong Yuugi (Japan set 2)",        0 )
GAME( 1991, ponchin,   0,        mjyuugi,  ponchin,  0,       ROT0, "Visco", "Mahjong Pon Chin Kan (Japan set 1)", 0 )
GAME( 1991, ponchina,  ponchin,  mjyuugi,  ponchin,  0,       ROT0, "Visco", "Mahjong Pon Chin Kan (Japan set 2)", 0 )


