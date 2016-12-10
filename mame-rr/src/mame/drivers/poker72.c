/*
  Unknown game, dump was marked 'slot 72 - poker'

  GFX roms contain
  'Extrema Systems International Ltd'
  as well as a logo for the company.

  There are also 'Lucky Boy' graphics in various places, which might be the title.


*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"

static UINT8 *poker72_vram,*poker72_pal;
static UINT8 tile_bank;

static VIDEO_START(poker72)
{

}

static VIDEO_UPDATE(poker72)
{
	int x,y,count;

	count = 0;

	for (y=0;y<32;y++)
	{
		for (x=0;x<64;x++)
		{
			int tile = ((poker72_vram[count+1] & 0x0f) << 8 ) | (poker72_vram[count+0] & 0xff); //TODO: tile bank
			int fx = (poker72_vram[count+1] & 0x10);
			int fy = (poker72_vram[count+1] & 0x20);
			int color = (poker72_vram[count+1] & 0xc0) >> 6;

			tile|= tile_bank << 12;

			drawgfx_opaque(bitmap,cliprect,screen->machine->gfx[0],tile,color,fx,fy,x*8,y*8);

			count+=2;
		}
	}

	return 0;
}

static WRITE8_HANDLER( poker72_paletteram_w )
{
	int r,g,b;
	poker72_pal[offset] = data;

	r = poker72_pal[(offset & 0x3ff)+0x000] & 0x3f;
	g = poker72_pal[(offset & 0x3ff)+0x400] & 0x3f;
	b = poker72_pal[(offset & 0x3ff)+0x800] & 0x3f;

	palette_set_color_rgb( space->machine, offset & 0x3ff, pal6bit(r), pal6bit(g), pal6bit(b));
}

static WRITE8_HANDLER( output_w )
{
	UINT8 *ROM = memory_region(space->machine, "maincpu");

	printf("%02x\n",data);

/*  if((data & 0xc) == 0xc)
        memory_set_bankptr(space->machine, "bank1", &ROM[0x10000]);
    else*/
	if(data & 8)
		memory_set_bankptr(space->machine, "bank1", &ROM[0x08000]);
	else
		memory_set_bankptr(space->machine, "bank1", &ROM[0x00000]);
}

static WRITE8_HANDLER( tile_bank_w )
{
	tile_bank = (data & 4) >> 2;
}

static ADDRESS_MAP_START( poker72_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xdfff) AM_RAM //work ram
	AM_RANGE(0xe000, 0xefff) AM_RAM AM_BASE(&poker72_vram)
	AM_RANGE(0xf000, 0xfbff) AM_RAM_WRITE(poker72_paletteram_w) AM_BASE(&poker72_pal)
	AM_RANGE(0xfc00, 0xfdff) AM_RAM //???
	AM_RANGE(0xfe08, 0xfe08) AM_READ_PORT("IN0")
	AM_RANGE(0xfe09, 0xfe09) AM_READ_PORT("IN1")
	AM_RANGE(0xfe0a, 0xfe0a) AM_READ_PORT("IN2")
	AM_RANGE(0xfe0c, 0xfe0c) AM_READ_PORT("IN3")
	AM_RANGE(0xfe0d, 0xfe0d) AM_READ_PORT("IN4")
	AM_RANGE(0xfe0e, 0xfe0e) AM_READ_PORT("IN5")

	AM_RANGE(0xfe17, 0xfe17) AM_READNOP //irq ack
	AM_RANGE(0xfe20, 0xfe20) AM_WRITE(output_w) //output, irq enable?
	AM_RANGE(0xfe22, 0xfe22) AM_WRITE(tile_bank_w)
	AM_RANGE(0xfe40, 0xfe40) AM_DEVREADWRITE("ay", ay8910_r, ay8910_data_w)
	AM_RANGE(0xfe60, 0xfe60) AM_DEVWRITE("ay", ay8910_address_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( poker72 )
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x00, "IN0" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("M. Bet")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Black")


	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Red")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE( 0x0080, IP_ACTIVE_LOW )

	PORT_START("IN3")
	PORT_DIPNAME( 0x01, 0x00, "IN3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
	PORT_START("IN4")
	PORT_DIPNAME( 0x01, 0x00, "IN4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
	PORT_START("IN5")
	PORT_DIPNAME( 0x01, 0x00, "IN5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )


	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x00, "DSW0" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "DSW1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )


INPUT_PORTS_END

static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,4),
	8,
	{ RGN_FRAC(3,4), RGN_FRAC(3,4)+4, RGN_FRAC(2,4), RGN_FRAC(2,4)+4 ,RGN_FRAC(1,4),RGN_FRAC(1,4)+4, RGN_FRAC(0,4),RGN_FRAC(0,4)+4 },
	{ 0,1,2,3,8,9,10,11 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};



static GFXDECODE_START( poker72 )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0, 16 )
GFXDECODE_END

/* default 444 palette for debug purpose */
static PALETTE_INIT( poker72 )
{
	int x,r,g,b;

	for(x=0;x<0x100;x++)
	{
		r = (x & 0xf)*0x10;
		g = ((x & 0x3c)>>2)*0x10;
		b = ((x & 0xf0)>>4)*0x10;
		palette_set_color(machine,x,MAKE_RGB(r,g,b));
	}
}

static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_INPUT_PORT("DSW0"),
	DEVCB_INPUT_PORT("DSW1"),
	DEVCB_NULL,
	DEVCB_NULL
};

static MACHINE_RESET( poker72 )
{
	UINT8 *ROM = memory_region(machine, "maincpu");

	memory_set_bankptr(machine, "bank1", &ROM[0]);
}

static MACHINE_DRIVER_START( poker72 )


	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80,8000000)		 /* ? MHz */
	MDRV_CPU_PROGRAM_MAP(poker72_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_MACHINE_RESET(poker72)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0, 64*8-1, 0, 32*8-1)

	MDRV_GFXDECODE(poker72)
	MDRV_PALETTE_LENGTH(0xe00)
	MDRV_PALETTE_INIT(poker72)

	MDRV_VIDEO_START(poker72)
	MDRV_VIDEO_UPDATE(poker72)

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ay", AY8910, 8000000/8) /* ? Mhz */
	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END



ROM_START( poker72 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "27010.bin", 0x00000, 0x20000, CRC(62447341) SHA1(e442c1f834a5dd2ab6ab3bdd316dfa86f2ca6647) )

	ROM_REGION( 0x1000, "89c51", 0 )
	ROM_LOAD( "89c51.bin", 0x00000, 0x1000, CRC(3fdd2148) SHA1(ea39a52482967268c7387aec77cfab1ae5c427fa) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "270135.bin", 0x00000, 0x20000, CRC(188c96ee) SHA1(7e883454cb080cdc82ce47ac92f51c8d45a55085) )
	ROM_LOAD( "270136.bin", 0x20000, 0x20000, CRC(f84c5068) SHA1(49178fe7b12f547a50879002236105a882767ebb) )
	ROM_LOAD( "270137.bin", 0x40000, 0x20000, CRC(310281d1) SHA1(c28f97bb3613c0b481ab6e16e215549c44b83c47) )
	ROM_LOAD( "270138.bin", 0x60000, 0x20000, CRC(d689313d) SHA1(8b9661b3af0e2ced7fe9fa487641e445ce7835b8) )
ROM_END

static DRIVER_INIT( poker72 )
{
	UINT8 *rom = memory_region(machine, "maincpu");

	rom[0x4a9] = 0x28;
}

GAME( 1995, poker72,  0,    poker72, poker72,  poker72, ROT0, "Extrema Systems International Ltd.", "Poker Monarch (v2.50)", GAME_NOT_WORKING ) // actually unknown, was marked 'slot 72 poker'  Manufacturers logo and 'Lucky Boy' gfx in rom..
