/***************************************************************************

    Capcom Baseball


    Somewhat similar to the "Mitchell hardware", but different enough to
    deserve its own driver.

    TODO:
    - understand what bit 6 of input port 0x12 is
    - unknown bit 5 of bankswitch register

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "includes/cps1.h"	// needed for decoding functions only
#include "includes/cbasebal.h"
#include "machine/eeprom.h"
#include "sound/okim6295.h"
#include "sound/2413intf.h"


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

static WRITE8_HANDLER( cbasebal_bankswitch_w )
{
	cbasebal_state *state = (cbasebal_state *)space->machine->driver_data;

	/* bits 0-4 select ROM bank */
	//logerror("%04x: bankswitch %02x\n", cpu_get_pc(space->cpu), data);
	memory_set_bank(space->machine, "bank1", data & 0x1f);

	/* bit 5 used but unknown */

	/* bits 6-7 select RAM bank */
	state->rambank = (data & 0xc0) >> 6;
}


static READ8_HANDLER( bankedram_r )
{
	cbasebal_state *state = (cbasebal_state *)space->machine->driver_data;

	switch (state->rambank)
	{
	case 2:
		return cbasebal_textram_r(space, offset);	/* VRAM */
	case 1:
		if (offset < 0x800)
			return space->machine->generic.paletteram.u8[offset];
		else
			return 0;
		break;
	default:
		return cbasebal_scrollram_r(space, offset);	/* SCROLL */
	}
}

static WRITE8_HANDLER( bankedram_w )
{
	cbasebal_state *state = (cbasebal_state *)space->machine->driver_data;

	switch (state->rambank)
	{
	case 2:
		cbasebal_textram_w(space, offset, data);
		break;
	case 1:
		if (offset < 0x800)
			paletteram_xxxxBBBBRRRRGGGG_le_w(space, offset, data);
		break;
	default:
		cbasebal_scrollram_w(space, offset, data);
		break;
	}
}

static WRITE8_HANDLER( cbasebal_coinctrl_w )
{
	coin_lockout_w(space->machine, 0, ~data & 0x04);
	coin_lockout_w(space->machine, 1, ~data & 0x08);
	coin_counter_w(space->machine, 0, data & 0x01);
	coin_counter_w(space->machine, 1, data & 0x02);
}


/*************************************
 *
 *  EEPROM
 *
 *************************************/

static const eeprom_interface cbasebal_eeprom_intf =
{
	6,		/* address bits */
	16,		/* data bits */
	"0110",	/*  read command */
	"0101",	/* write command */
	"0111"	/* erase command */
};


/*************************************
 *
 *  Address maps
 *
 *************************************/

static ADDRESS_MAP_START( cbasebal_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xcfff) AM_READWRITE(bankedram_r, bankedram_w) AM_BASE_GENERIC(paletteram)	/* palette + vram + scrollram */
	AM_RANGE(0xe000, 0xfdff) AM_RAM		/* work RAM */
	AM_RANGE(0xfe00, 0xffff) AM_RAM AM_BASE_SIZE_MEMBER(cbasebal_state, spriteram, spriteram_size)
ADDRESS_MAP_END

static ADDRESS_MAP_START( cbasebal_portmap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(cbasebal_bankswitch_w)
	AM_RANGE(0x01, 0x01) AM_WRITE_PORT("IO_01")
	AM_RANGE(0x02, 0x02) AM_WRITE_PORT("IO_02")
	AM_RANGE(0x03, 0x03) AM_WRITE_PORT("IO_03")
	AM_RANGE(0x05, 0x05) AM_DEVWRITE("oki", okim6295_w)
	AM_RANGE(0x06, 0x07) AM_DEVWRITE("ymsnd", ym2413_w)
	AM_RANGE(0x08, 0x09) AM_WRITE(cbasebal_scrollx_w)
	AM_RANGE(0x0a, 0x0b) AM_WRITE(cbasebal_scrolly_w)
	AM_RANGE(0x10, 0x10) AM_READ_PORT("P1")
	AM_RANGE(0x11, 0x11) AM_READ_PORT("P2")
	AM_RANGE(0x12, 0x12) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x13, 0x13) AM_WRITE(cbasebal_gfxctrl_w)
	AM_RANGE(0x14, 0x14) AM_WRITE(cbasebal_coinctrl_w)
ADDRESS_MAP_END


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( cbasebal )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_VBLANK )		/* ? */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE("eeprom", eeprom_read_bit)

	PORT_START( "IO_01" )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_set_cs_line)

	PORT_START( "IO_02" )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_set_clock_line)

	PORT_START( "IO_03" )
	PORT_BIT( 0x00000040, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_write_bit)
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout cbasebal_textlayout =
{
	8,8,	/* 8*8 characters */
	4096,	/* 4096 characters */
	2,		/* 2 bits per pixel */
	{ 0, 4 },
	{ 8+3, 8+2, 8+1, 8+0, 3, 2, 1, 0 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8    /* every char takes 16 consecutive bytes */
};

static const gfx_layout cbasebal_tilelayout =
{
	16,16,	/* 16*16 tiles */
	4096,	/* 4096 tiles */
	4,		/* 4 bits per pixel */
	{ 4096*64*8+4, 4096*64*8+0,4, 0 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3,
			16*16+0, 16*16+1, 16*16+2, 16*16+3, 16*16+8+0, 16*16+8+1, 16*16+8+2, 16*16+8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	64*8    /* every tile takes 64 consecutive bytes */
};

static const gfx_layout cbasebal_spritelayout =
{
	16,16,  /* 16*16 sprites */
	4096,   /* 2048 sprites */
	4,      /* 4 bits per pixel */
	{ 4096*64*8+4, 4096*64*8+0, 4, 0 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3,
			32*8+0, 32*8+1, 32*8+2, 32*8+3, 33*8+0, 33*8+1, 33*8+2, 33*8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	64*8    /* every sprite takes 64 consecutive bytes */
};

static GFXDECODE_START( cbasebal )
	GFXDECODE_ENTRY( "gfx1", 0, cbasebal_textlayout,   256,  8 ) /* colors 256- 287 */
	GFXDECODE_ENTRY( "gfx2", 0, cbasebal_tilelayout,   768, 16 ) /* colors 768-1023 */
	GFXDECODE_ENTRY( "gfx3", 0, cbasebal_spritelayout, 512,  8 ) /* colors 512- 639 */
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_START( cbasebal )
{
	cbasebal_state *state = (cbasebal_state *)machine->driver_data;

	memory_configure_bank(machine, "bank1", 0, 32, memory_region(machine, "maincpu") + 0x10000, 0x4000);

	state_save_register_global(machine, state->rambank);
	state_save_register_global(machine, state->tilebank);
	state_save_register_global(machine, state->spritebank);
	state_save_register_global(machine, state->text_on);
	state_save_register_global(machine, state->bg_on);
	state_save_register_global(machine, state->obj_on);
	state_save_register_global(machine, state->flipscreen);
	state_save_register_global_array(machine, state->scroll_x);
	state_save_register_global_array(machine, state->scroll_y);
}

static MACHINE_RESET( cbasebal )
{
	cbasebal_state *state = (cbasebal_state *)machine->driver_data;

	state->rambank = 0;
	state->tilebank = 0;
	state->spritebank = 0;
	state->text_on = 0;
	state->bg_on = 0;
	state->obj_on = 0;
	state->flipscreen = 0;
	state->scroll_x[0] = 0;
	state->scroll_x[1] = 0;
	state->scroll_y[0] = 0;
	state->scroll_y[1] = 0;
}

static MACHINE_DRIVER_START( cbasebal )

	/* driver data */
	MDRV_DRIVER_DATA(cbasebal_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, 6000000)	/* ??? */
	MDRV_CPU_PROGRAM_MAP(cbasebal_map)
	MDRV_CPU_IO_MAP(cbasebal_portmap)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)	/* ??? */

	MDRV_MACHINE_START(cbasebal)
	MDRV_MACHINE_RESET(cbasebal)

	MDRV_EEPROM_ADD("eeprom", cbasebal_eeprom_intf)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(8*8, (64-8)*8-1, 2*8, 30*8-1 )

	MDRV_GFXDECODE(cbasebal)
	MDRV_PALETTE_LENGTH(1024)

	MDRV_VIDEO_START(cbasebal)
	MDRV_VIDEO_UPDATE(cbasebal)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_OKIM6295_ADD("oki", 1056000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD("ymsnd", YM2413, 3579545)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( cbasebal )
	ROM_REGION( 0x90000, "maincpu", 0 )
	ROM_LOAD( "cbj10.11j",    0x00000, 0x08000, CRC(bbff0acc) SHA1(db9e2c89e030255851789caaf85f24dc73609d9b) )
	ROM_LOAD( "cbj07.16f",    0x10000, 0x20000, CRC(8111d13f) SHA1(264e21e824c87f55da326440c6ed71e1c287a63e) )
	ROM_LOAD( "cbj06.14f",    0x30000, 0x20000, CRC(9aaa0e37) SHA1(1a7b96b44c66b58f06707aafb1806520747b8c76) )
	ROM_LOAD( "cbj05.13f",    0x50000, 0x20000, CRC(d0089f37) SHA1(32354c3f4693a65e297791c4d8faac3aa9cff5a1) )
	/* 0x70000-0x8ffff empty (space for 04) */

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "cbj13.16m",    0x00000, 0x10000, CRC(2359fa0a) SHA1(3a37532ea43dd4b150c53a240d35a57a9b76d23d) )	/* text */

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "cbj02.1f",     0x00000, 0x20000, CRC(d6740535) SHA1(2ece885525718fd5fe52b8fa4c07930695b89659) )	/* tiles */
	ROM_LOAD( "cbj03.2f",     0x20000, 0x20000, CRC(88098dcd) SHA1(caddebeea581129d6a62fc9f7f354d61eef175c7) )
	ROM_LOAD( "cbj08.1j",     0x40000, 0x20000, CRC(5f3344bf) SHA1(1d3193078108e86e31bbfce15a8d2443cfbf2ff6) )
	ROM_LOAD( "cbj09.2j",     0x60000, 0x20000, CRC(aafffdae) SHA1(26e76b55fff49811df8e5b1f165be20ec8dd196a) )

	ROM_REGION( 0x80000, "gfx3", 0 )
	ROM_LOAD( "cbj11.1m",     0x00000, 0x20000, CRC(bdc1507d) SHA1(efeaf3066acfb7186d73ad8e5b291d6e61965de2) )	/* sprites */
	ROM_LOAD( "cbj12.2m",     0x20000, 0x20000, CRC(973f3efe) SHA1(d776499d5ac4bc23eb5d1f28b88447cc07d8ac99) )
	ROM_LOAD( "cbj14.1n",     0x40000, 0x20000, CRC(765dabaa) SHA1(742d1c50b65f649f23eac7976fe26c2d7400e4e1) )
	ROM_LOAD( "cbj15.2n",     0x60000, 0x20000, CRC(74756de5) SHA1(791d6620cdb563f0b3a717432aa4647981b0a10e) )

	ROM_REGION( 0x80000, "oki", 0 )	/* OKIM */
	ROM_LOAD( "cbj01.1e",     0x00000, 0x20000, CRC(1d8968bd) SHA1(813e475d1d0c343e7dad516f1fe564d00c9c27fb) )
ROM_END


/*************************************
 *
 *  Driver initialization
 *
 *************************************/

static DRIVER_INIT( cbasebal )
{
	pang_decode(machine);
}


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1989, cbasebal, 0, cbasebal, cbasebal, cbasebal, ROT0, "Capcom", "Capcom Baseball (Japan)", GAME_SUPPORTS_SAVE )
