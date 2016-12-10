/*
Dorachan (Dora-Chan ?) (c) 1980 Craul Denshi
Driver by Tomasz Slanina

Similar to Beam Invader
Todo:
- discrete sound
- dips (if any) - bits 5,6,7 of input port 0 ?
*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "deprecat.h"


#define NUM_PENS	(8)


class dorachan_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, dorachan_state(machine)); }

	dorachan_state(running_machine &machine) { }

	/* memory pointers */
	UINT8 *  videoram;
	size_t   videoram_size;

	/* video-related */
	UINT8    flip_screen;

	/* devices */
	running_device *main_cpu;
};


/*************************************
 *
 *  Protection handling
 *
 *************************************/

static CUSTOM_INPUT( dorachan_protection_r )
{
	dorachan_state *state = (dorachan_state *)field->port->machine->driver_data;
	UINT8 ret = 0;

	switch (cpu_get_previouspc(state->main_cpu))
	{
	case 0x70ce: ret = 0xf2; break;
	case 0x72a2: ret = 0xd5; break;
	case 0x72b5: ret = 0xcb; break;

	default:
		mame_printf_debug("unhandled $2400 read @ %x\n", cpu_get_previouspc(state->main_cpu));
		break;
	}

	return ret;
}



/*************************************
 *
 *  Video system
 *
 *************************************/

static void get_pens(pen_t *pens)
{
	offs_t i;

	for (i = 0; i < NUM_PENS; i++)
	{
		pens[i] = MAKE_RGB(pal1bit(i >> 2), pal1bit(i >> 1), pal1bit(i >> 0));
	}
}


static VIDEO_UPDATE( dorachan )
{
	dorachan_state *state = (dorachan_state *)screen->machine->driver_data;
	pen_t pens[NUM_PENS];
	offs_t offs;
	const UINT8 *color_map_base;

	get_pens(pens);

	color_map_base = memory_region(screen->machine, "proms");

	for (offs = 0; offs < state->videoram_size; offs++)
	{
		int i;
		UINT8 fore_color;

		UINT8 x = offs >> 8 << 3;
		UINT8 y = offs & 0xff;

		/* the need for +1 is extremely unusual, but definetely correct */
		offs_t color_address = ((((offs << 2) & 0x03e0) | (offs >> 8)) + 1) & 0x03ff;

		UINT8 data = state->videoram[offs];

		if (state->flip_screen)
			fore_color = (color_map_base[color_address] >> 3) & 0x07;
		else
			fore_color = (color_map_base[color_address] >> 0) & 0x07;

		for (i = 0; i < 8; i++)
		{
			UINT8 color = (data & 0x01) ? fore_color : RGB_BLACK;
			*BITMAP_ADDR32(bitmap, y, x) = pens[color];

			data = data >> 1;
			x = x + 1;
		}
	}

	return 0;
}


static WRITE8_HANDLER(dorachan_ctrl_w)
{
	dorachan_state *state = (dorachan_state *)space->machine->driver_data;
	state->flip_screen = (data >> 6) & 0x01;
}


static CUSTOM_INPUT( dorachan_v128_r )
{
	dorachan_state *state = (dorachan_state *)field->port->machine->driver_data;

	/* to avoid resetting (when player 2 starts) bit 0 need to be inverted when screen is flipped */
	return ((field->port->machine->primary_screen->vpos() >> 7) & 0x01) ^ state->flip_screen;
}



/*************************************
 *
 *  Memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( dorachan_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x17ff) AM_ROM
	AM_RANGE(0x1800, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0x23ff) AM_ROM
	AM_RANGE(0x2400, 0x2400) AM_MIRROR(0x03ff) AM_READ_PORT("PROT")
	AM_RANGE(0x2800, 0x2800) AM_MIRROR(0x03ff) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x2c00, 0x2c00) AM_MIRROR(0x03ff) AM_READ_PORT("JOY")
	AM_RANGE(0x3800, 0x3800) AM_MIRROR(0x03ff) AM_READ_PORT("V128")
	AM_RANGE(0x4000, 0x5fff) AM_RAM AM_BASE_SIZE_MEMBER(dorachan_state, videoram, videoram_size)
	AM_RANGE(0x6000, 0x77ff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  Port handlers
 *
 *************************************/

static ADDRESS_MAP_START( dorachan_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x01, 0x01) AM_WRITENOP
	AM_RANGE(0x02, 0x02) AM_WRITENOP
	AM_RANGE(0x03, 0x03) AM_WRITE(dorachan_ctrl_w)
ADDRESS_MAP_END



/*************************************
 *
 *  Port definition
 *
 *************************************/

static INPUT_PORTS_START( dorachan )
	PORT_START("PROT")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(dorachan_protection_r, NULL)

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("JOY")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )

	PORT_START("V128")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(dorachan_v128_r, NULL)
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_START( dorachan )
{
	dorachan_state *state = (dorachan_state *)machine->driver_data;

	state->main_cpu = machine->device("maincpu");

	state_save_register_global(machine, state->flip_screen);
}

static MACHINE_RESET( dorachan )
{
	dorachan_state *state = (dorachan_state *)machine->driver_data;

	state->flip_screen = 0;
}

static MACHINE_DRIVER_START( dorachan )

	/* driver data */
	MDRV_DRIVER_DATA(dorachan_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, 2000000)
	MDRV_CPU_PROGRAM_MAP(dorachan_map)
	MDRV_CPU_IO_MAP(dorachan_io_map)
	MDRV_CPU_VBLANK_INT_HACK(irq0_line_hold,2)

	MDRV_MACHINE_START(dorachan)
	MDRV_MACHINE_RESET(dorachan)

	/* video hardware */
	MDRV_VIDEO_UPDATE(dorachan)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(1*8, 31*8-1, 1*8, 31*8-1)
	MDRV_SCREEN_REFRESH_RATE(60)

MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definition
 *
 *************************************/

ROM_START( dorachan )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "c1.e1",      0x0000, 0x0400, CRC(29d66a96) SHA1(a0297d87574af65c6ded99aeb377ac407f6f163f) )
	ROM_LOAD( "d2.e2",      0x0400, 0x0400, CRC(144b6cd1) SHA1(195ce86e912a4b395097008c6d812fd75a1a2482) )
	ROM_LOAD( "d3.e3",      0x0800, 0x0400, CRC(a9a1bed7) SHA1(98af6f851c4477f770b6bd67e5465b5a271311ee) )
	ROM_LOAD( "d4.e5",      0x0c00, 0x0400, CRC(099ddf4b) SHA1(e4dd2b17a4320615204c66c24f60e58db13a5319) )
	ROM_LOAD( "c5.e6",      0x1000, 0x0400, CRC(49449dab) SHA1(3627c16cc17fae9de2294a37602b726e107d0a13) )
	ROM_LOAD( "d6.e7",      0x1400, 0x0400, CRC(5e409680) SHA1(f5e4d820c0f0493d724cd0d3da1113bccc09c2c3) )
	ROM_LOAD( "c7.e8",      0x2000, 0x0400, CRC(b331a5ff) SHA1(1053953c76dddff450b9c9037e7797d50f9c7046) )
	ROM_LOAD( "d8.rom",     0x6000, 0x0400, CRC(5fe1e731) SHA1(8e5dcb5f8d1d6f8c06808dd808f8bce7b07014ee) )
	ROM_LOAD( "d9.rom",     0x6400, 0x0400, CRC(338881a8) SHA1(cd725b42c3f96826e94345698738f6b5a532d3d5) )
	ROM_LOAD( "d10.rom",    0x6800, 0x0400, CRC(f8c59517) SHA1(655a976b1221e5aff69e0c0cc58d02c0b7bb6197) )
	ROM_LOAD( "d11.rom",    0x6c00, 0x0400, CRC(c2e0f066) SHA1(be6b780a8957d945e5634ac9689b440a41e9a2a4) )
	ROM_LOAD( "d12.rom",    0x7000, 0x0400, CRC(275e5dc1) SHA1(ac07db4b428daa49a52c679de95ddedbea0076b9) )
	ROM_LOAD( "d13.rom",    0x7400, 0x0400, CRC(24ccfcf9) SHA1(85e5052ee657f518b0509eb64e494bc3a74e651e) )

	ROM_REGION( 0x0400, "proms", 0 )  /* color map */
	ROM_LOAD( "d14.rom",    0x0000, 0x0400, CRC(c0d3ee84) SHA1(f2207c685ce8d5144a373c28f11d2cebf9518b65) )
ROM_END



/*************************************
 *
 *  Game driver
 *
 *************************************/

GAME( 1980, dorachan, 0, dorachan, dorachan, 0, ROT270, "Craul Denshi", "Dorachan", GAME_NO_SOUND | GAME_SUPPORTS_SAVE )
