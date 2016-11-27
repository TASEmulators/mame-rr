/***************************************************************************

                            Ginga NinkyouDen
                            (C) 1987 Jaleco

                    driver by Luca Elia (l.elia@tin.it)

CPU   : 68000 68B09
SOUND : YM2149 Y8950(MSX AUDIO)
OSC.  : 6.000MHz 3.579545MHz

* CTC uses MB-8873E (MC-6840)

                    Interesting routines (main cpu)
                    -------------------------------

Interrupts: 1-7]    d17a:   clears 20018 etc.

f4b2    print string:   a1->(char)*,0x25(%) d7.w=color  a0->screen (30000)
f5d6    print 7 digit BCD number: d0.l to (a1)+ color $3000


                    Interesting locations (main cpu)
                    --------------------------------

20014   # of players (1-2)
20018   cleared by interrupts
2001c   credits (max 9)
20020   internal timer?
20024   initial lives
20058   current lives p1
2005c   current lives p2
20070   coins
200a4   time
200a8   energy

60008       values: 0 1 ffff
6000c       bit:    0   flip sceen? <-  70002>>14
                    1   ?           <-

6000e   soundlatch  <- 20038 2003c 20040


                                To Do
                                -----

- The sound section will benefit from proper MC6840 and YM8950 emulation

***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "cpu/m68000/m68000.h"
#include "deprecat.h"
#include "sound/ay8910.h"
#include "sound/8950intf.h"
#include "includes/ginganin.h"


/*
**
**              Main cpu data
**
*/


static ADDRESS_MAP_START( ginganin_map, ADDRESS_SPACE_PROGRAM, 16 )
/* The ROM area: 10000-13fff is written with: 0000 0000 0000 0001, at startup only. Why? */
	AM_RANGE(0x000000, 0x01ffff) AM_ROM
	AM_RANGE(0x020000, 0x023fff) AM_RAM
	AM_RANGE(0x030000, 0x0307ff) AM_RAM_WRITE(ginganin_txtram16_w) AM_BASE_MEMBER(ginganin_state, txtram)
	AM_RANGE(0x040000, 0x0407ff) AM_RAM AM_BASE_SIZE_MEMBER(ginganin_state, spriteram, spriteram_size)
	AM_RANGE(0x050000, 0x0507ff) AM_RAM_WRITE(paletteram16_RRRRGGGGBBBBxxxx_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x060000, 0x06000f) AM_RAM_WRITE(ginganin_vregs16_w) AM_BASE_MEMBER(ginganin_state, vregs)
	AM_RANGE(0x068000, 0x06bfff) AM_RAM_WRITE(ginganin_fgram16_w) AM_BASE_MEMBER(ginganin_state, fgram)
	AM_RANGE(0x070000, 0x070001) AM_READ_PORT("P1_P2")
	AM_RANGE(0x070002, 0x070003) AM_READ_PORT("DSW")
ADDRESS_MAP_END


/*
**
**              Sound cpu data
**
*/

static WRITE8_HANDLER( MC6840_control_port_0_w )
{
	/* MC6840 Emulation by Takahiro Nogi. 1999/09/27
    (This routine hasn't been completed yet.) */

	ginganin_state *state = (ginganin_state *)space->machine->driver_data;
	state->MC6840_index0 = data;

	if (state->MC6840_index0 & 0x80)	/* enable timer output */
	{
		if ((state->MC6840_register0 != state->S_TEMPO) && (state->MC6840_register0 != 0))
		{
			state->S_TEMPO = state->MC6840_register0;
#ifdef MAME_DEBUG
			popmessage("I0:0x%02X R0:0x%02X I1:0x%02X R1:0x%02X", state->MC6840_index0, state->MC6840_register0, state->MC6840_index1, state->MC6840_register1);
#endif
		}
		state->MC6809_FLAG = 1;
	}
	else
		state->MC6809_FLAG = 0;

#ifdef MAME_DEBUG
	logerror("MC6840 Write:(0x%02X)0x%02X\n", state->MC6840_register0,  data);
#endif
}

static WRITE8_HANDLER( MC6840_control_port_1_w )
{
	/* MC6840 Emulation by Takahiro Nogi. 1999/09/27
    (This routine hasn't been completed yet.) */

	ginganin_state *state = (ginganin_state *)space->machine->driver_data;
	state->MC6840_index1 = data;
}

static WRITE8_HANDLER( MC6840_write_port_0_w )
{
	/* MC6840 Emulation by Takahiro Nogi. 1999/09/27
    (This routine hasn't been completed yet.) */

	ginganin_state *state = (ginganin_state *)space->machine->driver_data;
	state->MC6840_register0 = data;
}

static WRITE8_HANDLER( MC6840_write_port_1_w )
{
	/* MC6840 Emulation by Takahiro Nogi. 1999/09/27
    (This routine hasn't been completed yet.) */

	ginganin_state *state = (ginganin_state *)space->machine->driver_data;
	state->MC6840_register1 = data;
}

static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_RAM
	AM_RANGE(0x0800, 0x0800) AM_WRITE(MC6840_control_port_0_w)	/* Takahiro Nogi. 1999/09/27 */
	AM_RANGE(0x0801, 0x0801) AM_WRITE(MC6840_control_port_1_w)	/* Takahiro Nogi. 1999/09/27 */
	AM_RANGE(0x0802, 0x0802) AM_WRITE(MC6840_write_port_0_w)		/* Takahiro Nogi. 1999/09/27 */
	AM_RANGE(0x0803, 0x0803) AM_WRITE(MC6840_write_port_1_w)		/* Takahiro Nogi. 1999/09/27 */
	AM_RANGE(0x1800, 0x1800) AM_READ(soundlatch_r)
	AM_RANGE(0x2000, 0x2001) AM_DEVWRITE("ymsnd", y8950_w)
	AM_RANGE(0x2800, 0x2801) AM_DEVWRITE("aysnd", ay8910_address_data_w)
	AM_RANGE(0x4000, 0xffff) AM_ROM
ADDRESS_MAP_END


/* Input Ports */

static INPUT_PORTS_START( ginganin )
	PORT_START("P1_P2")		/* 70000.w */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("DSW")		/* 70002.w */
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(	  0x0004, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(	  0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(	  0x0006, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(	  0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(	  0x0003, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(	  0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(	  0x0001, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(	  0x0020, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(	  0x0010, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(	  0x0030, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(	  0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(	  0x0018, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(	  0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(	  0x0008, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Infinite Lives")
	PORT_DIPSETTING(	  0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Free Play & Invulnerability")
	PORT_DIPSETTING(	  0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) )
	PORT_DIPSETTING(	  0x0000, "2")
	PORT_DIPSETTING(	  0x0300, "3")
	PORT_DIPSETTING(	  0x0100, "4")
	PORT_DIPSETTING(	  0x0200, "5")
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0400, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0000, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( Upright ) )
	PORT_DIPSETTING(	  0x0800, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )	/* probably unused */
	PORT_DIPSETTING(	  0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )	/* it does something */
	PORT_DIPSETTING(	  0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(	  0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Freeze (Cheat)")
	PORT_DIPSETTING(	  0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
INPUT_PORTS_END



/*
**
**              Gfx data
**
*/


#define layout16x16(_name_,_romsize_) \
static const gfx_layout _name_ =\
{\
	16,16,\
	(_romsize_)*8/(16*16*4),\
	4,\
	{0, 1, 2, 3},\
	{0*4,1*4,2*4,3*4,4*4,5*4,6*4,7*4,\
	 0*4+32*16,1*4+32*16,2*4+32*16,3*4+32*16,4*4+32*16,5*4+32*16,6*4+32*16,7*4+32*16},\
	{0*32,1*32,2*32,3*32,4*32,5*32,6*32,7*32,\
	 8*32,9*32,10*32,11*32,12*32,13*32,14*32,15*32},\
	16*16*4\
};

#define layout8x8(_name_,_romsize_) \
static const gfx_layout _name_ =\
{\
	8,8,\
	(_romsize_)*8/(8*8*4),\
	4,\
	{0, 1, 2, 3},\
	{0*4,1*4,2*4,3*4,4*4,5*4,6*4,7*4}, \
	{0*32,1*32,2*32,3*32,4*32,5*32,6*32,7*32},\
	8*8*4\
};

layout16x16(tilelayout,  0x20000)
layout8x8  (txtlayout,	 0x04000)
layout16x16(spritelayout,0x50000)

static GFXDECODE_START( ginganin )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout,	 256*3, 16 ) /* [0] bg */
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout,	 256*2, 16 ) /* [1] fg */
	GFXDECODE_ENTRY( "gfx3", 0, txtlayout,	 256*0, 16 ) /* [2] txt */
	GFXDECODE_ENTRY( "gfx4", 0, spritelayout, 256*1, 16 ) /* [3] sprites */
GFXDECODE_END




/* Modified by Takahiro Nogi. 1999/09/27 */
static INTERRUPT_GEN( ginganin_sound_interrupt )
{
	/* MC6840 Emulation by Takahiro Nogi. 1999/09/27
    (This routine hasn't been completed yet.) */

	ginganin_state *state = (ginganin_state *)device->machine->driver_data;

	if (state->S_TEMPO_OLD != state->S_TEMPO)
	{
		state->S_TEMPO_OLD = state->S_TEMPO;
		state->MC6809_CTR = 0;
	}

	if (state->MC6809_FLAG != 0)
	{
		if (state->MC6809_CTR > state->S_TEMPO)
		{
			state->MC6809_CTR = 0;
			cpu_set_input_line(device, 0, HOLD_LINE);
		}
		else
			state->MC6809_CTR++;
	}
}





static MACHINE_START( ginganin )
{
	ginganin_state *state = (ginganin_state *)machine->driver_data;

	state->audiocpu = machine->device("audiocpu");

	state_save_register_global(machine, state->layers_ctrl);
	state_save_register_global(machine, state->flipscreen);
	state_save_register_global(machine, state->MC6840_index0);
	state_save_register_global(machine, state->MC6840_register0);
	state_save_register_global(machine, state->MC6840_index1);
	state_save_register_global(machine, state->MC6840_register1);
	state_save_register_global(machine, state->S_TEMPO);
	state_save_register_global(machine, state->S_TEMPO_OLD);
	state_save_register_global(machine, state->MC6809_CTR);
	state_save_register_global(machine, state->MC6809_FLAG);
}

static MACHINE_RESET( ginganin )
{
	ginganin_state *state = (ginganin_state *)machine->driver_data;

	state->layers_ctrl = 0;
	state->flipscreen = 0;
	state->MC6840_index0 = 0;
	state->MC6840_register0 = 0;
	state->MC6840_index1 = 0;
	state->MC6840_register1 = 0;
	state->S_TEMPO = 0;
	state->S_TEMPO_OLD = 0;
	state->MC6809_CTR = 0;
	state->MC6809_FLAG = 0;
}


static MACHINE_DRIVER_START( ginganin )

	/* driver data */
	MDRV_DRIVER_DATA(ginganin_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 6000000)	/* ? */
	MDRV_CPU_PROGRAM_MAP(ginganin_map)
	MDRV_CPU_VBLANK_INT("screen", irq1_line_hold) /* ? (vectors 1-7 cointain the same address) */

	MDRV_CPU_ADD("audiocpu", M6809, 1000000)
	MDRV_CPU_PROGRAM_MAP(sound_map)
	MDRV_CPU_VBLANK_INT_HACK(ginganin_sound_interrupt,60)	/* Takahiro Nogi. 1999/09/27 (1 -> 60) */

	MDRV_MACHINE_START(ginganin)
	MDRV_MACHINE_RESET(ginganin)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 255, 0 + 16 , 255 - 16)

	MDRV_GFXDECODE(ginganin)
	MDRV_PALETTE_LENGTH(1024)

	MDRV_VIDEO_START(ginganin)
	MDRV_VIDEO_UPDATE(ginganin)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("aysnd", AY8910, 3579545 / 2)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)

	MDRV_SOUND_ADD("ymsnd", Y8950, 3579545)	/* The Y8950 is basically a YM3526 with ADPCM built in */
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( ginganin )
	ROM_REGION( 0x20000, "maincpu", 0 )	/* main cpu */
	ROM_LOAD16_BYTE( "gn_02.bin", 0x00000, 0x10000, CRC(4a4e012f) SHA1(7c94a5b6b71e037af355f3aa4623be1f585db8dc) )
	ROM_LOAD16_BYTE( "gn_01.bin", 0x00001, 0x10000, CRC(30256fcb) SHA1(dc15e0da88ae5cabe0150f7290508c3d58c06c11) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* sound cpu */
	ROM_LOAD( "gn_05.bin", 0x00000, 0x10000, CRC(e76e10e7) SHA1(b16f10a1a01b7b04221c9bf1b0d157e936bc5fb5) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "gn_15.bin", 0x000000, 0x10000, CRC(1b8ac9fb) SHA1(1e5ee2a565fa262f1e48c1088d84c6f42d84b4e3) )  /* bg */
	ROM_LOAD( "gn_14.bin", 0x010000, 0x10000, CRC(e73fe668) SHA1(fa39fddd7448d3fc6b539506e33b951db205afa1) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "gn_12.bin", 0x000000, 0x10000, CRC(c134a1e9) SHA1(8bace0f0169e61f1b7254393fa9cad6dca09c335) )  /* fg */
	ROM_LOAD( "gn_13.bin", 0x010000, 0x10000, CRC(1d3bec21) SHA1(305823c78cad9288f918178e1c24cb0459ba2a6e) )

	ROM_REGION( 0x04000, "gfx3", 0 )
	ROM_LOAD( "gn_10.bin", 0x000000, 0x04000, CRC(ae371b2d) SHA1(d5e03b085586ed2bf40713f432bcf12e07318226) )  /* txt */

	ROM_REGION( 0x50000, "gfx4", 0 )
	ROM_LOAD( "gn_06.bin", 0x000000, 0x10000, CRC(bdc65835) SHA1(53222fc3ec15e641289abb754657b0d59b88b66b) )  /* sprites */
	ROM_CONTINUE(		   0x040000, 0x10000 )
	ROM_LOAD( "gn_07.bin", 0x010000, 0x10000, CRC(c2b8eafe) SHA1(a042a200efd4e7361e9ab516085c9fc8067e28b4) )
	ROM_LOAD( "gn_08.bin", 0x020000, 0x10000, CRC(f7c73c18) SHA1(102700e2217bcd1532af56ee6a00ad608c8217db) )
	ROM_LOAD( "gn_09.bin", 0x030000, 0x10000, CRC(a5e07c3b) SHA1(cdda02cd847330575612cb33d1bb38a5d50a3e6d) )

	ROM_REGION( 0x08000, "gfx5", 0 )	/* background tilemaps */
	ROM_LOAD( "gn_11.bin", 0x00000, 0x08000, CRC(f0d0e605) SHA1(0c541e8e036573be1d99ecb71fdb4568ca8cc269) )

	ROM_REGION( 0x20000, "ymsnd", 0 )	/* samples */
	ROM_LOAD( "gn_04.bin", 0x00000, 0x10000, CRC(0ed9133b) SHA1(77f628e8ec28016efac2d906146865ca4ec54bd5) )
	ROM_LOAD( "gn_03.bin", 0x10000, 0x10000, CRC(f1ba222c) SHA1(780c0bd0045bac1e1bb3209576383db90504fbf3) )

ROM_END

ROM_START( ginganina )
	ROM_REGION( 0x20000, "maincpu", 0 )	/* main cpu */
	ROM_LOAD16_BYTE( "2.bin", 0x00000, 0x10000, CRC(6da1d8a3) SHA1(ea81f2934fa7901563e886f3d600edd08ec0ea24) )
	ROM_LOAD16_BYTE( "1.bin", 0x00001, 0x10000, CRC(0bd32d59) SHA1(5ab2c0e4a1d9cafbd3448d981103508debd7ed96) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* sound cpu */
	ROM_LOAD( "gn_05.bin", 0x00000, 0x10000, CRC(e76e10e7) SHA1(b16f10a1a01b7b04221c9bf1b0d157e936bc5fb5) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "gn_15.bin", 0x000000, 0x10000, CRC(1b8ac9fb) SHA1(1e5ee2a565fa262f1e48c1088d84c6f42d84b4e3) )  /* bg */
	ROM_LOAD( "gn_14.bin", 0x010000, 0x10000, CRC(e73fe668) SHA1(fa39fddd7448d3fc6b539506e33b951db205afa1) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "gn_12.bin", 0x000000, 0x10000, CRC(c134a1e9) SHA1(8bace0f0169e61f1b7254393fa9cad6dca09c335) )  /* fg */
	ROM_LOAD( "gn_13.bin", 0x010000, 0x10000, CRC(1d3bec21) SHA1(305823c78cad9288f918178e1c24cb0459ba2a6e) )

	ROM_REGION( 0x04000, "gfx3", 0 )
	ROM_LOAD( "10.bin", 0x000000, 0x04000, CRC(48a20745) SHA1(69855b0402feca4ba9632142e569c652ca05b9fa) )  /* txt */

	ROM_REGION( 0x50000, "gfx4", 0 )
	ROM_LOAD( "gn_06.bin", 0x000000, 0x10000, CRC(bdc65835) SHA1(53222fc3ec15e641289abb754657b0d59b88b66b) )  /* sprites */
	ROM_CONTINUE(		   0x040000, 0x10000 )
	ROM_LOAD( "gn_07.bin", 0x010000, 0x10000, CRC(c2b8eafe) SHA1(a042a200efd4e7361e9ab516085c9fc8067e28b4) )
	ROM_LOAD( "gn_08.bin", 0x020000, 0x10000, CRC(f7c73c18) SHA1(102700e2217bcd1532af56ee6a00ad608c8217db) )
	ROM_LOAD( "gn_09.bin", 0x030000, 0x10000, CRC(a5e07c3b) SHA1(cdda02cd847330575612cb33d1bb38a5d50a3e6d) )

	ROM_REGION( 0x08000, "gfx5", 0 )	/* background tilemaps */
	ROM_LOAD( "gn_11.bin", 0x00000, 0x08000, CRC(f0d0e605) SHA1(0c541e8e036573be1d99ecb71fdb4568ca8cc269) )

	ROM_REGION( 0x20000, "ymsnd", 0 )	/* samples */
	ROM_LOAD( "gn_04.bin", 0x00000, 0x10000, CRC(0ed9133b) SHA1(77f628e8ec28016efac2d906146865ca4ec54bd5) )
	ROM_LOAD( "gn_03.bin", 0x10000, 0x10000, CRC(f1ba222c) SHA1(780c0bd0045bac1e1bb3209576383db90504fbf3) )
ROM_END



static DRIVER_INIT( ginganin )
{
	UINT16 *rom;

	/* main cpu patches */
	rom = (UINT16 *)memory_region(machine, "maincpu");
	/* avoid writes to rom getting to the log */
	rom[0x408 / 2] = 0x6000;
	rom[0x40a / 2] = 0x001c;


	/* sound cpu patches */
	/* let's clear the RAM: ROM starts at 0x4000 */
	memset(memory_region(machine, "audiocpu"), 0, 0x800);
}


GAME( 1987, ginganin,  0,        ginganin, ginganin, ginganin, ROT0, "Jaleco", "Ginga NinkyouDen (set 1)", GAME_SUPPORTS_SAVE )
GAME( 1987, ginganina, ginganin, ginganin, ginganin, ginganin, ROT0, "Jaleco", "Ginga NinkyouDen (set 2)", GAME_SUPPORTS_SAVE )
