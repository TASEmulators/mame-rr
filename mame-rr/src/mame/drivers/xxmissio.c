/*****************************************************************************

XX Mission (c) 1986 UPL

    Driver by Uki

    31/Mar/2001 -

*****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "deprecat.h"
#include "sound/2203intf.h"

VIDEO_START( xxmissio );
VIDEO_UPDATE( xxmissio );

extern UINT8 *xxmissio_bgram;
extern UINT8 *xxmissio_fgram;
extern UINT8 *xxmissio_spriteram;

static UINT8 xxmissio_status;


WRITE8_DEVICE_HANDLER( xxmissio_scroll_x_w );
WRITE8_DEVICE_HANDLER( xxmissio_scroll_y_w );
WRITE8_HANDLER( xxmissio_flipscreen_w );

READ8_HANDLER( xxmissio_bgram_r );
WRITE8_HANDLER( xxmissio_bgram_w );

WRITE8_HANDLER( xxmissio_paletteram_w );

static WRITE8_HANDLER( xxmissio_bank_sel_w )
{
	memory_set_bank(space->machine, "bank1", data & 7);
}

static CUSTOM_INPUT( xxmissio_status_r )
{
	int bit_mask = (FPTR)param;
	return (xxmissio_status & bit_mask) ? 1 : 0;
}

static WRITE8_HANDLER ( xxmissio_status_m_w )
{
	switch (data)
	{
		case 0x00:
			xxmissio_status |= 0x20;
			break;

		case 0x40:
			xxmissio_status &= ~0x08;
			cputag_set_input_line_and_vector(space->machine, "sub", 0, HOLD_LINE, 0x10);
			break;

		case 0x80:
			xxmissio_status |= 0x04;
			break;
	}
}

static WRITE8_HANDLER ( xxmissio_status_s_w )
{
	switch (data)
	{
		case 0x00:
			xxmissio_status |= 0x10;
			break;

		case 0x40:
			xxmissio_status |= 0x08;
			break;

		case 0x80:
			xxmissio_status &= ~0x04;
			cputag_set_input_line_and_vector(space->machine, "maincpu", 0, HOLD_LINE, 0x10);
			break;
	}
}

static INTERRUPT_GEN( xxmissio_interrupt_m )
{
	xxmissio_status &= ~0x20;
	cpu_set_input_line(device, 0, HOLD_LINE);
}

static INTERRUPT_GEN( xxmissio_interrupt_s )
{
	xxmissio_status &= ~0x10;
	cpu_set_input_line(device, 0, HOLD_LINE);
}

static MACHINE_START( xxmissio )
{
	memory_configure_bank(machine, "bank1", 0, 8, memory_region(machine, "user1"), 0x4000);
	memory_set_bank(machine, "bank1", 0);
}

/****************************************************************************/

static ADDRESS_MAP_START( map1, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM

	AM_RANGE(0x8000, 0x8001) AM_DEVREADWRITE("ym1", ym2203_r, ym2203_w)
	AM_RANGE(0x8002, 0x8003) AM_DEVREADWRITE("ym2", ym2203_r, ym2203_w)

	AM_RANGE(0xa000, 0xa000) AM_READ_PORT("P1")
	AM_RANGE(0xa001, 0xa001) AM_READ_PORT("P2")
	AM_RANGE(0xa002, 0xa002) AM_READ_PORT("STATUS")
	AM_RANGE(0xa002, 0xa002) AM_WRITE(xxmissio_status_m_w)
	AM_RANGE(0xa003, 0xa003) AM_WRITE(xxmissio_flipscreen_w)

	AM_RANGE(0xc000, 0xc7ff) AM_SHARE("share1") AM_RAM AM_BASE(&xxmissio_fgram)
	AM_RANGE(0xc800, 0xcfff) AM_SHARE("share2") AM_READWRITE(xxmissio_bgram_r, xxmissio_bgram_w) AM_BASE(&xxmissio_bgram)
	AM_RANGE(0xd000, 0xd7ff) AM_SHARE("share3") AM_RAM AM_BASE(&xxmissio_spriteram)

	AM_RANGE(0xd800, 0xdaff) AM_SHARE("share4") AM_RAM_WRITE(xxmissio_paletteram_w) AM_BASE_GENERIC(paletteram)

	AM_RANGE(0xe000, 0xefff) AM_SHARE("share5") AM_RAM
	AM_RANGE(0xf000, 0xffff) AM_SHARE("share6") AM_RAM
ADDRESS_MAP_END


static ADDRESS_MAP_START( map2, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank1")

	AM_RANGE(0x8000, 0x8001) AM_DEVREADWRITE("ym1", ym2203_r, ym2203_w)
	AM_RANGE(0x8002, 0x8003) AM_DEVREADWRITE("ym2", ym2203_r, ym2203_w)
	AM_RANGE(0x8006, 0x8006) AM_WRITE(xxmissio_bank_sel_w)

	AM_RANGE(0xa000, 0xa000) AM_READ_PORT("P1")
	AM_RANGE(0xa001, 0xa001) AM_READ_PORT("P2")
	AM_RANGE(0xa002, 0xa002) AM_READ_PORT("STATUS")
	AM_RANGE(0xa002, 0xa002) AM_WRITE(xxmissio_status_s_w)
	AM_RANGE(0xa003, 0xa003) AM_WRITE(xxmissio_flipscreen_w)

	AM_RANGE(0xc000, 0xc7ff) AM_SHARE("share1") AM_RAM
	AM_RANGE(0xc800, 0xcfff) AM_SHARE("share2") AM_READWRITE(xxmissio_bgram_r, xxmissio_bgram_w)
	AM_RANGE(0xd000, 0xd7ff) AM_SHARE("share3") AM_RAM

	AM_RANGE(0xd800, 0xdaff) AM_SHARE("share4") AM_RAM_WRITE(xxmissio_paletteram_w)

	AM_RANGE(0xe000, 0xefff) AM_SHARE("share6") AM_RAM
	AM_RANGE(0xf000, 0xffff) AM_SHARE("share5") AM_RAM
ADDRESS_MAP_END


/****************************************************************************/

static INPUT_PORTS_START( xxmissio )
	PORT_START("P1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY

	PORT_START("P2")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x40, 0x40, "Endless Game (Cheat)")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x04, 0x04, "First Bonus" )
	PORT_DIPSETTING(    0x04, "30000" )
	PORT_DIPSETTING(    0x00, "40000" )
	PORT_DIPNAME( 0x18, 0x08, "Bonus Every" )
	PORT_DIPSETTING(    0x18, "50000" )
	PORT_DIPSETTING(    0x08, "70000" )
	PORT_DIPSETTING(    0x10, "90000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 2-6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 2-7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 2-8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("STATUS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(xxmissio_status_r, (void *)0x01)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_VBLANK )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(xxmissio_status_r, (void *)0x04)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(xxmissio_status_r, (void *)0x08)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(xxmissio_status_r, (void *)0x10)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(xxmissio_status_r, (void *)0x20)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(xxmissio_status_r, (void *)0x40)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(xxmissio_status_r, (void *)0x80)
INPUT_PORTS_END

/****************************************************************************/

static const gfx_layout charlayout =
{
	16,8,   /* 16*8 characters */
	2048,   /* 2048 characters */
	4,      /* 4 bits per pixel */
	{0,1,2,3},
	{0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60},
	{64*0, 64*1, 64*2, 64*3, 64*4, 64*5, 64*6, 64*7},
	64*8
};

static const gfx_layout spritelayout =
{
	32,16,    /* 32*16 characters */
	512,	  /* 512 sprites */
	4,        /* 4 bits per pixel */
	{0,1,2,3},
	{0,4,8,12,16,20,24,28,
	 32,36,40,44,48,52,56,60,
	 8*64+0,8*64+4,8*64+8,8*64+12,8*64+16,8*64+20,8*64+24,8*64+28,
	 8*64+32,8*64+36,8*64+40,8*64+44,8*64+48,8*64+52,8*64+56,8*64+60},
	{64*0, 64*1, 64*2, 64*3, 64*4, 64*5, 64*6, 64*7,
	 64*16, 64*17, 64*18, 64*19, 64*20, 64*21, 64*22, 64*23},
	64*8*4
};

static const gfx_layout bglayout =
{
	16,8,   /* 16*8 characters */
	1024,   /* 1024 characters */
	4,      /* 4 bits per pixel */
	{0,1,2,3},
	{0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60},
	{64*0, 64*1, 64*2, 64*3, 64*4, 64*5, 64*6, 64*7},
	64*8
};

static GFXDECODE_START( xxmissio )
	GFXDECODE_ENTRY( "gfx1", 0x0000, charlayout,   256,  8 ) /* FG */
	GFXDECODE_ENTRY( "gfx1", 0x0000, spritelayout,   0,  8 ) /* sprite */
	GFXDECODE_ENTRY( "gfx2", 0x0000, bglayout,     512, 16 ) /* BG */
GFXDECODE_END

/****************************************************************************/

static const ym2203_interface ym2203_interface_1 =
{
	{
		AY8910_LEGACY_OUTPUT,
		AY8910_DEFAULT_LOADS,
		DEVCB_INPUT_PORT("DSW1"),
		DEVCB_INPUT_PORT("DSW2"),
		DEVCB_NULL,
		DEVCB_NULL
	},
	NULL
};

static const ym2203_interface ym2203_interface_2 =
{
	{
		AY8910_LEGACY_OUTPUT,
		AY8910_DEFAULT_LOADS,
		DEVCB_NULL,
		DEVCB_NULL,
		DEVCB_HANDLER(xxmissio_scroll_x_w),
		DEVCB_HANDLER(xxmissio_scroll_y_w)
	},
	NULL
};

static MACHINE_DRIVER_START( xxmissio )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80,12000000/4)	/* 3.0MHz */
	MDRV_CPU_PROGRAM_MAP(map1)
	MDRV_CPU_VBLANK_INT("screen", xxmissio_interrupt_m)

	MDRV_CPU_ADD("sub", Z80,12000000/4)	/* 3.0MHz */
	MDRV_CPU_PROGRAM_MAP(map2)
	MDRV_CPU_VBLANK_INT_HACK(xxmissio_interrupt_s,2)

	MDRV_QUANTUM_TIME(HZ(6000))

	MDRV_MACHINE_START(xxmissio)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 4*8, 28*8-1)

	MDRV_GFXDECODE(xxmissio)
	MDRV_PALETTE_LENGTH(768)

	MDRV_VIDEO_START(xxmissio)
	MDRV_VIDEO_UPDATE(xxmissio)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ym1", YM2203, 12000000/8)
	MDRV_SOUND_CONFIG(ym2203_interface_1)
	MDRV_SOUND_ROUTE(0, "mono", 0.15)
	MDRV_SOUND_ROUTE(1, "mono", 0.15)
	MDRV_SOUND_ROUTE(2, "mono", 0.15)
	MDRV_SOUND_ROUTE(3, "mono", 0.40)

	MDRV_SOUND_ADD("ym2", YM2203, 12000000/8)
	MDRV_SOUND_CONFIG(ym2203_interface_2)
	MDRV_SOUND_ROUTE(0, "mono", 0.15)
	MDRV_SOUND_ROUTE(1, "mono", 0.15)
	MDRV_SOUND_ROUTE(2, "mono", 0.15)
	MDRV_SOUND_ROUTE(3, "mono", 0.40)
MACHINE_DRIVER_END

/****************************************************************************/

ROM_START( xxmissio )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* CPU1 */
	ROM_LOAD( "xx1.4l", 0x0000,  0x8000, CRC(86e07709) SHA1(7bfb7540b6509f07a6388ca2da6b3892f5b1df74) )

	ROM_REGION( 0x10000, "sub", 0 ) /* CPU2 */
	ROM_LOAD( "xx2.4b", 0x0000,  0x4000, CRC(13fa7049) SHA1(e8974d9f271a966611b523496ba8cd910e227a23) )

	ROM_REGION( 0x18000, "user1", 0 ) /* BANK */
	ROM_LOAD( "xx3.6a", 0x00000,  0x8000, CRC(16fdacab) SHA1(2158ca9b14c52bc1cd5ef0f4c0180f0519224403) )
	ROM_LOAD( "xx4.6b", 0x08000,  0x8000, CRC(274bd4d2) SHA1(2ddf9b953584e26f221b1c86181d827bdc3dc81b) )
	ROM_LOAD( "xx5.6d", 0x10000,  0x8000, CRC(c5f35535) SHA1(6812b70beb73fc80cf20d2d51f747952ed106887) )

	ROM_REGION( 0x20000, "gfx1", 0 ) /* FG/sprites */
	ROM_LOAD16_BYTE( "xx6.8j", 0x00001, 0x8000, CRC(dc954d01) SHA1(73ecbbc859da9db9fead91cd03bb90e5779916e2) )
	ROM_LOAD16_BYTE( "xx8.8f", 0x00000, 0x8000, CRC(a9587cc6) SHA1(5fbcb88505f89c4d8a2a228489612ff66fc5d3af) )
	ROM_LOAD16_BYTE( "xx7.8h", 0x10001, 0x8000, CRC(abe9cd68) SHA1(f3ce9b40e3d9cdc9b77a43f9d5d0411338d88833) )
	ROM_LOAD16_BYTE( "xx9.8e", 0x10000, 0x8000, CRC(854e0e5f) SHA1(b01d6a735b175c2f7ac3fc4053702c9da62c6a4e) )

	ROM_REGION( 0x10000, "gfx2", 0 ) /* BG */
	ROM_LOAD16_BYTE( "xx10.4c", 0x0000,  0x8000, CRC(d27d7834) SHA1(60c24dc2ab7e2a33da4002f1f07eaf7898cf387f) )
	ROM_LOAD16_BYTE( "xx11.4b", 0x0001,  0x8000, CRC(d9dd827c) SHA1(aea3a5abd871adf7f75ad4d6cc57eff0833135c7) )
ROM_END

GAME( 1986, xxmissio, 0, xxmissio, xxmissio, 0, ROT90, "UPL", "XX Mission", 0 )
