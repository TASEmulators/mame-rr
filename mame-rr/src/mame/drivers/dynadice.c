/*
Dynamic Dice (??)

Driver by
    Tomasz Slanina
    Pierpaolo Prazzoli
--
Old, rusty, not working pcb :


Main PCB :

m5l8080ap (8080)

dy_1.bin (1H)
dy_2.bin (1L)
dy_3.bin (1P)

dip 6x  (all off)

xtal 18.432 mhz

--
Sub PCB  DYNA-529-81ST :

AY-3-8910
Z80

dy_4.bin
dy_5.bin
dy_6.bin (near Z80)
*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/i8085/i8085.h"
#include "sound/ay8910.h"


class dynadice_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, dynadice_state(machine)); }

	dynadice_state(running_machine &machine) { }

	/* memory pointers */
	UINT8 *  videoram;
//  UINT8 *  nvram;     // currently this uses generic nvram handling
//  UINT8 *  paletteram;    // currently this uses generic palette handling

	/* video-related */
	tilemap_t  *bg_tilemap,*top_tilemap;

	/* misc */
	int      ay_data;
};


static WRITE8_HANDLER( dynadice_videoram_w )
{
	dynadice_state *state = (dynadice_state *)space->machine->driver_data;
	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
	tilemap_mark_all_tiles_dirty(state->top_tilemap);
}

static WRITE8_HANDLER( sound_data_w )
{
	dynadice_state *state = (dynadice_state *)space->machine->driver_data;
	state->ay_data = data;
}

static WRITE8_DEVICE_HANDLER( sound_control_w )
{
	dynadice_state *state = (dynadice_state *)device->machine->driver_data;
/*
    AY 3-8910 :

    D0 - BC1
    D1 - BC2
    D2 - BDIR
    D3 - /Reset

*/
	if ((data & 7) == 7)
		ay8910_address_w(device, 0, state->ay_data);

	if ((data & 7) == 6)
		ay8910_data_w(device, 0, state->ay_data);
}


static ADDRESS_MAP_START( dynadice_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x23ff) AM_RAM_WRITE(dynadice_videoram_w) AM_BASE_MEMBER(dynadice_state, videoram)
	AM_RANGE(0x4000, 0x40ff) AM_RAM AM_BASE_SIZE_GENERIC(nvram)
ADDRESS_MAP_END

static ADDRESS_MAP_START( dynadice_io_map, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x50, 0x50) AM_READ_PORT("IN0")
	AM_RANGE(0x51, 0x51) AM_READ_PORT("IN1")
	AM_RANGE(0x52, 0x52) AM_READ_PORT("DSW")
	AM_RANGE(0x62, 0x62) AM_WRITENOP
	AM_RANGE(0x63, 0x63) AM_WRITE(soundlatch_w)
	AM_RANGE(0x70, 0x77) AM_WRITENOP
ADDRESS_MAP_END

static ADDRESS_MAP_START( dynadice_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_ROM
	AM_RANGE(0x2000, 0x23ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( dynadice_sound_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ(soundlatch_r)
	AM_RANGE(0x01, 0x01) AM_WRITE(soundlatch_clear_w)
	AM_RANGE(0x02, 0x02) AM_WRITE(sound_data_w)
	AM_RANGE(0x03, 0x03) AM_DEVWRITE("aysnd", sound_control_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( dynadice )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_DIPNAME( 0x02, 0x02, "Initialize NVRAM" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) /* increase number of coins */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) /* decrease number of coins */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )  /* start /stop */

	PORT_START("DSW")
	PORT_DIPNAME( 0x1c, 0x1c, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ))
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ))
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ))
	PORT_DIPSETTING(    0x0c, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x1c, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x14, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_4C ))

	PORT_DIPNAME( 0x01, 0x01, "DSW 1-0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DSW 1-1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DSW 1-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DSW 1-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DSW 1-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout charlayout2 =
{
	8,8,
	RGN_FRAC(1,1),
	3,
	{ 5,6,7 },
	{ 0, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	{ 0*8*8, 1*8*8, 2*8*8, 3*8*8, 4*8*8, 5*8*8, 6*8*8, 7*8*8 },
	8*8*8
};


static GFXDECODE_START( dynadice )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0, 1 ) /* 1bpp */
	GFXDECODE_ENTRY( "gfx2", 0, charlayout2,  0, 1 ) /* 3bpp */
GFXDECODE_END

static TILE_GET_INFO( get_tile_info )
{
	dynadice_state *state = (dynadice_state *)machine->driver_data;
	int code = state->videoram[tile_index];
	SET_TILE_INFO(1, code, 0, 0);
}

static VIDEO_START( dynadice )
{
	dynadice_state *state = (dynadice_state *)machine->driver_data;

	/* pacman - style videoram layout */
	state->bg_tilemap = tilemap_create(machine, get_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	state->top_tilemap = tilemap_create(machine, get_tile_info, tilemap_scan_cols, 8, 8, 2, 32);
	tilemap_set_scrollx(state->bg_tilemap, 0, -16);
}

static VIDEO_UPDATE( dynadice )
{
	dynadice_state *state = (dynadice_state *)screen->machine->driver_data;
	rectangle myclip = *cliprect;
	myclip.max_x = 15;
	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	tilemap_draw(bitmap, &myclip, state->top_tilemap, 0, 0);
	return 0;
}

static PALETTE_INIT( dynadice )
{
	int i;
	for(i = 0; i < 8; i++)
		palette_set_color_rgb(machine, i, pal1bit(i >> 1), pal1bit(i >> 2), pal1bit(i >> 0));
}

static MACHINE_START( dynadice )
{
	dynadice_state *state = (dynadice_state *)machine->driver_data;
	state_save_register_global(machine, state->ay_data);
}

static MACHINE_RESET( dynadice )
{
	dynadice_state *state = (dynadice_state *)machine->driver_data;
	state->ay_data = 0;
}

static MACHINE_DRIVER_START( dynadice )

	/* driver data */
	MDRV_DRIVER_DATA(dynadice_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", I8080,18432000/8)
	MDRV_CPU_PROGRAM_MAP(dynadice_map)
	MDRV_CPU_IO_MAP(dynadice_io_map)

	MDRV_CPU_ADD("audiocpu", Z80,18432000/6)
	MDRV_CPU_PROGRAM_MAP(dynadice_sound_map)
	MDRV_CPU_IO_MAP(dynadice_sound_io_map)

	MDRV_MACHINE_START(dynadice)
	MDRV_MACHINE_RESET(dynadice)

	MDRV_NVRAM_HANDLER(generic_0fill)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256+16, 256)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 34*8-1, 3*8, 28*8-1)

	MDRV_GFXDECODE(dynadice)
	MDRV_PALETTE_LENGTH(8)
	MDRV_PALETTE_INIT(dynadice)

	MDRV_VIDEO_START(dynadice)
	MDRV_VIDEO_UPDATE(dynadice)

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("aysnd", AY8910, 2000000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END

ROM_START( dynadice )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dy_1.bin",     0x0000, 0x1000, CRC(4ad18724) SHA1(78151b02a727f4272eff72765883df9ca09606c3) )
	ROM_LOAD( "dy_2.bin",     0x1000, 0x0800, CRC(82cb1873) SHA1(661f33af4a536b7929d432d755ab44f9280f82db) )
	ROM_LOAD( "dy_3.bin",     0x1800, 0x0800, CRC(a8edad20) SHA1(b812141f216355c986047969326bd1e036be71e6) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "dy_6.bin",     0x0000, 0x0800, CRC(d4e6e6a3) SHA1(84c0fcfd8326a4301accbd192df6e372b98ae537) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "dy_4.bin",     0x0000, 0x0800, CRC(306b851b) SHA1(bf69ed126d32b31e1711ff23c5a75b8a8bd28207) )

	ROM_REGION( 0x0800*8, "gfx2", ROMREGION_ERASE00 )
	/* gfx data will be rearranged here for 8x8 3bpp tiles */

	ROM_REGION( 0x0800, "user1",0 )
	ROM_LOAD( "dy_5.bin",     0x0000, 0x0800, CRC(e4799462) SHA1(5cd0f003572540522d72706bc5a8fa6588553031) )
ROM_END

static DRIVER_INIT( dynadice )
{
	int i, j;
	UINT8 *usr1 = memory_region(machine, "user1");
	UINT8 *cpu2 = memory_region(machine, "audiocpu");
	UINT8 *gfx1 = memory_region(machine, "gfx1");
	UINT8 *gfx2 = memory_region(machine, "gfx2");

	cpu2[0x0b] = 0x23;	/* bug in game code  Dec HL -> Inc HL*/

	/* 1bpp tiles -> 3bpp tiles (dy_5.bin  contains bg/fg color data for each tile line) */
	for (i = 0; i < 0x800; i++)
		for (j = 0; j < 8; j++)
			gfx2[(i << 3) + j] = (gfx1[i] & (0x80 >> j)) ? (usr1[i] & 7) : (usr1[i] >> 4);
}

GAME( 19??, dynadice, 0, dynadice, dynadice, dynadice, ROT90, "<unknown>", "Dynamic Dice", GAME_SUPPORTS_SAVE )
