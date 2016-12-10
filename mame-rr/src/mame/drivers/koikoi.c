/***************************************************************************

Koi Koi Part 2
---------------
driver by
 Tomasz Slanina
 David Haywood

TODO:
- map missing inputs (temp mapped to z-x-left shift)
- is there (still..) some kind of protection ? timers looks weird (2nd player timer is frozen) (this seems fixed now -AS)
- colors (afaik color(?) prom outputs are connected to one of pals), Missing color prom apparently.


Basic hw is...
z80 (possibly xtal/4)
ay3-8910 (possibly xtal/8)
15.468xtal
1 dsw (8)
2kx8 SRAM (x2)


Few words about inputs (code @ $21bd):
--------------------------------------

There's four reads in a row of input port 3 - 32 possibilities.
But only 14 are valid - two lookup tables are used to decode the inputs.

For example, for one of input keys game expects data: 0,0,8,0
It's encoded (in internal lookup table, as well as in input_tab[]) as 0x68 :
 - bits 0-4 = data to return (valid values are 1,2,4,8,$10 - only one bit set)
 - bits 5-7 = read cycle (1-4) to return above data
All other reads should return 0.

Also, every other call, code at $21bd must return 0 (2 in 2nd read cycle)
to prevent disabling inputs.

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"

#define KOIKOI_CRYSTAL 15468000

static const int input_tab[]= { 0x22, 0x64, 0x44, 0x68, 0x30, 0x50, 0x70, 0x48, 0x28, 0x21, 0x41, 0x82, 0x81, 0x42 };

class koikoi_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, koikoi_state(machine)); }

	koikoi_state(running_machine &machine) { }

	/* memory pointers */
	UINT8 *  videoram;

	/* video-related */
	tilemap_t  *tmap;

	/* misc */
	int inputcnt;
	int inputval;
	int inputlen;
	int ioram[8];
};


/*************************************
 *
 *  Video emulation
 *
 *************************************/

static TILE_GET_INFO( get_tile_info )
{
	koikoi_state *state = (koikoi_state *)machine->driver_data;
	int code  = state->videoram[tile_index] | ((state->videoram[tile_index + 0x400] & 0x40) << 2);
	int color = (state->videoram[tile_index + 0x400] & 0x1f);
	int flip  = (state->videoram[tile_index + 0x400] & 0x80) ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0;

	SET_TILE_INFO( 0, code, color, flip);
}

static PALETTE_INIT( koikoi )
{
	int i;

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 0x10);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x10; i++)
	{
		int bit0, bit1, bit2;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		/* blue component */
		bit0 = 0;
		bit1 = (color_prom[i] >> 6) & 0x01;
		bit2 = (color_prom[i] >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		colortable_palette_set_color(machine->colortable, i, MAKE_RGB(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x20;

	/* characters/sprites */
	for (i = 0; i < 0x100; i++)
	{
		UINT8 ctabentry = color_prom[i] & 0x0f;
		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}
}

static VIDEO_START(koikoi)
{
	koikoi_state *state = (koikoi_state *)machine->driver_data;
	state->tmap = tilemap_create(machine, get_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
}

static VIDEO_UPDATE(koikoi)
{
	koikoi_state *state = (koikoi_state *)screen->machine->driver_data;
	tilemap_draw(bitmap, cliprect, state->tmap, 0, 0);
	return 0;
}

/*************************************
 *
 *  Memory handlers
 *
 *************************************/

static WRITE8_HANDLER( vram_w )
{
	koikoi_state *state = (koikoi_state *)space->machine->driver_data;
	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->tmap, offset & 0x3ff);
}

static READ8_DEVICE_HANDLER( input_r )
{
	koikoi_state *state = (koikoi_state *)device->machine->driver_data;

	if (state->inputcnt < 0)
		return 0;

	if (!state->inputcnt)
	{
		int key = input_port_read(device->machine, "IN1");
		int keyval = 0; //we must return 0 (0x2 in 2nd read) to clear 4 bit at $6600 and allow next read

		if (key)
		{
			while (!(key & 1))
			{
				key >>= 1;
				keyval++;
			}
		}

		state->inputval = input_tab[keyval] & 0x1f;
		state->inputlen = input_tab[keyval] >> 5;
	}

	if (state->inputlen == ++state->inputcnt) //return expected value
	{
		return state->inputval ^ 0xff;
	}

	if (state->inputcnt > 4) //end of cycle
	{
		state->inputcnt = -1;
	}

	return 0xff; //return 0^0xff
}

static WRITE8_DEVICE_HANDLER( unknown_w )
{
	//xor'ed mux select, player 1 = 1,2,4,8, player 2 = 0x10, 0x20, 0x40, 0x80
}

static READ8_HANDLER( io_r )
{
	koikoi_state *state = (koikoi_state *)space->machine->driver_data;
	if (!offset)
		return input_port_read(space->machine, "IN0") ^ state->ioram[4]; //coin

	return 0;
}

static WRITE8_HANDLER( io_w )
{
	koikoi_state *state = (koikoi_state *)space->machine->driver_data;
	if (offset == 7 && data == 0)
		state->inputcnt = 0; //reset read cycle counter

	state->ioram[offset] = data;
}

/*************************************
 *
 *  Address maps
 *
 *************************************/

static ADDRESS_MAP_START( koikoi_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x2fff) AM_ROM
	AM_RANGE(0x6000, 0x67ff) AM_RAM
	AM_RANGE(0x7000, 0x77ff) AM_RAM_WRITE(vram_w) AM_BASE_MEMBER(koikoi_state, videoram)
	AM_RANGE(0x8000, 0x8000) AM_READ_PORT("DSW")
	AM_RANGE(0x9000, 0x9007) AM_READWRITE(io_r, io_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( koikoi_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x02, 0x02) AM_WRITENOP //watchdog
	AM_RANGE(0x03, 0x03) AM_DEVREAD("aysnd", ay8910_r)
	AM_RANGE(0x06, 0x07) AM_DEVWRITE("aysnd", ay8910_data_address_w)
ADDRESS_MAP_END

/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( koikoi )
	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x01, "Timer C" )
	PORT_DIPSETTING(	0x00, "50" )
	PORT_DIPSETTING(	0x01, "70" )
	PORT_DIPSETTING(	0x02, "90" )
	PORT_DIPSETTING(	0x03, "110" )
	PORT_DIPNAME( 0x0c, 0x04, "Timer M" )
	PORT_DIPSETTING(	0x00, "120" )
	PORT_DIPSETTING(	0x04, "150" )
	PORT_DIPSETTING(	0x08, "180" )
	PORT_DIPSETTING(	0x0c, "210" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(	0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(	0x10, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Test Mode" )
	PORT_DIPSETTING(	0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x80, DEF_STR( On ) )

	PORT_START("IN0")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0xbf, IP_ACTIVE_LOW,  IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_HANAFUDA_A )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_HANAFUDA_B )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_HANAFUDA_C )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_HANAFUDA_D )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_HANAFUDA_E )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_HANAFUDA_F )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_HANAFUDA_G )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_HANAFUDA_H )

	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_LSHIFT)

	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_START2 )
INPUT_PORTS_END


/*************************************
 *
 *  Graphic definitions
 *
 *************************************/

static const gfx_layout tilelayout =
{
	8, 8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


static GFXDECODE_START( koikoi )
	GFXDECODE_ENTRY( "gfx1", 0x0000, tilelayout,      0, 32 )
GFXDECODE_END


/*************************************
 *
 *  Sound interface
 *
 *************************************/

static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_NULL,					DEVCB_HANDLER(input_r),
	DEVCB_HANDLER(unknown_w),	DEVCB_NULL
};


/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_START( koikoi )
{
	koikoi_state *state = (koikoi_state *)machine->driver_data;

	state_save_register_global(machine, state->inputcnt);
	state_save_register_global(machine, state->inputval);
	state_save_register_global(machine, state->inputlen);
	state_save_register_global_array(machine, state->ioram);
}

static MACHINE_RESET( koikoi )
{
	koikoi_state *state = (koikoi_state *)machine->driver_data;
	int i;

	state->inputcnt = -1;
	state->inputval = 0;
	state->inputlen = 0;

	for (i = 0; i < 8; i++)
		state->ioram[i] = 0;
}

static MACHINE_DRIVER_START( koikoi )

	/* driver data */
	MDRV_DRIVER_DATA(koikoi_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80,KOIKOI_CRYSTAL/4)	/* ?? */
	MDRV_CPU_PROGRAM_MAP(koikoi_map)
	MDRV_CPU_IO_MAP(koikoi_io_map)
	MDRV_CPU_VBLANK_INT("screen", nmi_line_pulse)

	MDRV_MACHINE_START(koikoi)
	MDRV_MACHINE_RESET(koikoi)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)

	MDRV_GFXDECODE(koikoi)
	MDRV_PALETTE_LENGTH(8*32)
	MDRV_PALETTE_INIT(koikoi)

	MDRV_VIDEO_START(koikoi)
	MDRV_VIDEO_UPDATE(koikoi)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("aysnd", AY8910, KOIKOI_CRYSTAL/8)
	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)
MACHINE_DRIVER_END


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( koikoi )
	ROM_REGION( 0x10000, "maincpu", 0 )	/* code */
	ROM_LOAD( "ic56", 0x0000, 0x1000, CRC(bdc68f9d) SHA1(c45fbc95abb37f750acc1d9f3b35ad0f41af097d) )
	ROM_LOAD( "ic55", 0x1000, 0x1000, CRC(fe09248a) SHA1(c192795678068e387bd406f5cd1c5aba5f5ef66a) )
	ROM_LOAD( "ic54", 0x2000, 0x1000, CRC(925fc57c) SHA1(4c79df92b6617fe84e61359c8e6e3b907b138777) )

	ROM_REGION( 0x3000, "gfx1", 0 )	/* gfx */
	ROM_LOAD( "ic33", 0x0000, 0x1000, CRC(9e4d563b) SHA1(63664dcffc2eb198a161c73131b95a66b2067424) )
	ROM_LOAD( "ic26", 0x1000, 0x1000, CRC(79cb1e93) SHA1(4d08b3d88727b437673f7a51d47396f19bbc3caa) )
	ROM_LOAD( "ic18", 0x2000, 0x1000, CRC(c209362d) SHA1(0620c19fe72e8407db0f487b6413c5d45ac8046c) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "prom.x",    0x000, 0x020,  NO_DUMP )
	ROM_LOAD( "prom.ic23", 0x020, 0x100,  CRC(f1d169a6) SHA1(5ee4b1dfe61e8b97a90cc113ba234298189f1a73) )

	ROM_REGION( 0x0a00, "plds", 0 )
	ROM_LOAD( "pal16r8-10_pink.ic9",   0x0000, 0x0104, CRC(9f8fdb95) SHA1(cdcdb1a6baef18961cf6c75fba0c3aba47f3edbb) )
	ROM_LOAD( "pal16r8-10_green.ic15", 0x0200, 0x0104, CRC(da7b8b95) SHA1(a4eb12f2365ff2b6057e4a2e225e8f879a961d45) )
	ROM_LOAD( "pal16r8a_yellow.ic8",   0x0400, 0x0104, CRC(7d8da540) SHA1(28925d1fb4ef670e9c9d24860b67fdff8791c6a9) )
	ROM_LOAD( "pal16r8a_brown.ic11",   0x0600, 0x0104, CRC(fff46363) SHA1(97f673c862e9d5b12cac283000a779c465c76828) )
	ROM_LOAD( "pal16r8a_red.ic10",     0x0800, 0x0104, CRC(027ad661) SHA1(fa5aafe6deb3a9865498152b92dd3776ea10a51d) )
ROM_END

/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1982, koikoi,   0,      koikoi, koikoi, 0, ROT270, "Kiwako", "Koi Koi Part 2", GAME_WRONG_COLORS | GAME_SUPPORTS_SAVE )
