/*
Panic Road
----------

TODO:
- there are 3 more bitplanes of tile graphics, but colors are correct as they are...
  what are they, priority info? Should they be mapped at 8000-bfff in memory?
- problems with bg tilemaps reading (USER3 region)

--

Panic Road (JPN Ver.)
(c)1986 Taito / Seibu
SEI-8611M (M6100219A)

OSC  : 14.31818MHz,12.0000MHz,16.0000MHz
CPU  : V20 (Sony CXQ70116D-8) @ 8.000MHz [16/2]
       Toshiba T5182 @ 3.579545 (14.31818/4]
Sound: YM2151 @ 3.579545 [14.31818/4]
    VSync 60Hz
    HSync 15.32kHz
Other:
    SEI0010BU(TC17G005AN-0025) x3
    SEI0021BU(TC17G008AN-0022)
    Toshiba(TC17G008AN-0024)
    SEI0030BU(TC17G005AN-0026)
    SEI0050BU(MA640 00)
    SEI0040BU(TC15G008AN-0048) @ 6.00MHz [12/2]


13F.BIN      [4e6b3c04]
15F.BIN      [d735b572]

22D.BIN      [eb1a46e1]

5D.BIN       [f3466906]
7D.BIN       [8032c1e9]

2A.BIN       [3ac0e5b4]
2B.BIN       [567d327b]
2C.BIN       [cd77ec79]
2D.BIN       [218d2c3e]

2J.BIN       [80f05923]
2K.BIN       [35f07bca]

1.19N        [674131b9]
2.19M        [3d48b0b5]

A.15C        [c75772bc] 82s129
B.14C        [145d1e0d]  |
C.13C        [11c11bbd]  |
D.9B         [f99cac4b] /

8A.BPR       [908684a6] 63s281
10J.BPR      [1dd80ee1]  |
10L.BPR      [f3f29695]  |
12D.BPR      [0df8aa3c] /

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/nec/nec.h"
#include "deprecat.h"
#include "audio/t5182.h"

#define MASTER_CLOCK	XTAL_16MHz
#define SOUND_CLOCK		XTAL_14_31818MHz
#define TC15_CLOCK		XTAL_12MHz

static tilemap_t *bgtilemap, *txttilemap;
static UINT8 *scrollram;
static UINT8 *mainram;

static PALETTE_INIT( panicr )
{
	int i;

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 0x100);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x100; i++)
	{
		int r = pal4bit(color_prom[i + 0x000]);
		int g = pal4bit(color_prom[i + 0x100]);
		int b = pal4bit(color_prom[i + 0x200]);

		colortable_palette_set_color(machine->colortable, i, MAKE_RGB(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x300;

	// txt lookup table
	for (i = 0; i < 0x100; i++)
	{
		UINT8 ctabentry;

		if (color_prom[i] & 0x40)
			ctabentry = 0;
		else
			ctabentry = (color_prom[i] & 0x3f) | 0x80;

		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}

	// tile lookup table
	for (i = 0x100; i < 0x200; i++)
	{
		UINT8 ctabentry = (color_prom[i] & 0x3f) | 0x00;

		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}

	// sprite lookup table
	for (i = 0x200; i < 0x300; i++)
	{
		UINT8 ctabentry;

		if (color_prom[i] & 0x40)
			ctabentry = 0;
		else
			ctabentry = (color_prom[i] & 0x3f) | 0x40;

		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}
}

static TILE_GET_INFO( get_bgtile_info )
{
	int code,attr;

	code=memory_region(machine, "user1")[tile_index];
	attr=memory_region(machine, "user2")[tile_index];
	code+=((attr&7)<<8);
	SET_TILE_INFO(
		1,
        code,
		(attr & 0xf0) >> 4,
        0);
}

static TILE_GET_INFO( get_txttile_info )
{
	int code=machine->generic.videoram.u8[tile_index*4];
	int attr=machine->generic.videoram.u8[tile_index*4+2];
	int color = attr & 0x07;

	tileinfo->group = color;

	SET_TILE_INFO(
		0,
		code + ((attr & 8) << 5),
		color,
		0);
}

static READ8_HANDLER(t5182shared_r)
{
	if ((offset & 1) == 0)
		return t5182_sharedram[offset/2];
	else
		return 0;
}

static WRITE8_HANDLER(t5182shared_w)
{
	if ((offset & 1) == 0)
		t5182_sharedram[offset/2] = data;
}


static ADDRESS_MAP_START( panicr_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x00000, 0x01fff) AM_RAM AM_BASE(&mainram)
	AM_RANGE(0x02000, 0x02fff) AM_RAM AM_BASE_GENERIC(spriteram)
	AM_RANGE(0x03000, 0x03fff) AM_RAM
	AM_RANGE(0x08000, 0x0bfff) AM_RAM AM_REGION("user3", 0) //attribue map ?
	AM_RANGE(0x0c000, 0x0cfff) AM_RAM AM_BASE_GENERIC(videoram)
	AM_RANGE(0x0d000, 0x0d000) AM_WRITE(t5182_sound_irq_w)
	AM_RANGE(0x0d002, 0x0d002) AM_READ(t5182_sharedram_semaphore_snd_r)
	AM_RANGE(0x0d004, 0x0d004) AM_WRITE(t5182_sharedram_semaphore_main_acquire_w)
	AM_RANGE(0x0d006, 0x0d006) AM_WRITE(t5182_sharedram_semaphore_main_release_w)
	AM_RANGE(0x0d200, 0x0d2ff) AM_READWRITE(t5182shared_r, t5182shared_w)
	AM_RANGE(0x0d400, 0x0d400) AM_READ_PORT("P1")
	AM_RANGE(0x0d402, 0x0d402) AM_READ_PORT("P2")
	AM_RANGE(0x0d404, 0x0d404) AM_READ_PORT("START")
	AM_RANGE(0x0d406, 0x0d406) AM_READ_PORT("DSW1")
	AM_RANGE(0x0d407, 0x0d407) AM_READ_PORT("DSW2")
	AM_RANGE(0x0d800, 0x0d81f) AM_RAM AM_BASE (&scrollram)
	AM_RANGE(0xf0000, 0xfffff) AM_ROM
ADDRESS_MAP_END

static VIDEO_START( panicr )
{
	bgtilemap = tilemap_create( machine, get_bgtile_info,tilemap_scan_rows,16,16,1024,16 );

	txttilemap = tilemap_create( machine, get_txttile_info,tilemap_scan_rows,8,8,32,32 );
	colortable_configure_tilemap_groups(machine->colortable, txttilemap, machine->gfx[0], 0);
}

static void draw_sprites(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect )
{
	UINT8 *spriteram = machine->generic.spriteram.u8;
	int offs,fx,fy,x,y,color,sprite;

	for (offs = 0; offs<0x1000; offs+=16)
	{

		fx = 0;
		fy = spriteram[offs+1] & 0x80;
		y = spriteram[offs+2];
		x = spriteram[offs+3];

		color = spriteram[offs+1] & 0x0f;

		sprite = spriteram[offs+0]+(scrollram[0x0c]<<8);

		drawgfx_transmask(bitmap,cliprect,machine->gfx[2],
				sprite,
				color,fx,fy,x,y,
				colortable_get_transpen_mask(machine->colortable, machine->gfx[2], color, 0));
	}
}

static VIDEO_UPDATE( panicr)
{
	bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine));
	tilemap_mark_all_tiles_dirty( txttilemap );
	tilemap_set_scrollx( bgtilemap,0, ((scrollram[0x02]&0x0f)<<12)+((scrollram[0x02]&0xf0)<<4)+((scrollram[0x04]&0x7f)<<1)+((scrollram[0x04]&0x80)>>7) );
	tilemap_draw(bitmap,cliprect,bgtilemap,0,0);
	draw_sprites(screen->machine,bitmap,cliprect);
	tilemap_draw(bitmap,cliprect,txttilemap,0,0);

	return 0;
}

static INTERRUPT_GEN( panicr_interrupt )
{
	if (cpu_getiloops(device))
		cpu_set_input_line_and_vector(device, 0, HOLD_LINE, 0xc8/4);
	else
		cpu_set_input_line_and_vector(device, 0, HOLD_LINE, 0xc4/4);
}

static INPUT_PORTS_START( panicr )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) //left
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) //right
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) //shake 1
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) //shake 2
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) //left
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) //right
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) //shake 1
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) //shake 2
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("START")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xe7, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )	PORT_DIPLOCATION("SW1:8,7,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Coin_B ) )	PORT_DIPLOCATION("SW1:5,4")
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_SERVICE_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )	PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x04, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:7,6")
	PORT_DIPSETTING(    0x06, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x18, 0x18, "Bonus" )		PORT_DIPLOCATION("SW2:5,4")
	PORT_DIPSETTING(    0x18, "50k & every 1OOk" )
	PORT_DIPSETTING(    0x10, "1Ok 20k" )
	PORT_DIPSETTING(    0x08, "20k 40k" )
	PORT_DIPSETTING(    0x00, "50k 100k" )
	PORT_DIPNAME( 0x60, 0x40, "Balls" )		PORT_DIPLOCATION("SW2:3,2")
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x40, "3" )
	PORT_DIPSETTING(    0x60, "1" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )	PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_START(T5182COINPORT)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(2)
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ 0, 4, RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,4),
//  8,
//  { 0, 4, RGN_FRAC(1,4)+0, RGN_FRAC(1,4)+4, RGN_FRAC(2,4)+0, RGN_FRAC(2,4)+4, RGN_FRAC(3,4)+0, RGN_FRAC(3,4)+4 },
	4,
	{ RGN_FRAC(2,4)+0, RGN_FRAC(2,4)+4, RGN_FRAC(3,4)+0, RGN_FRAC(3,4)+4 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3,
			16+0, 16+1, 16+2, 16+3, 24+0, 24+1, 24+2, 24+3 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
			8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	32*16
};


static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ 0, 4, RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4 },
	{ 0, 1, 2, 3, 4*8+0, 4*8+1, 4*8+2,  4*8+3,
		8*8+0, 8*8+1, 8*8+2, 8*8+3, 12*8+0, 12*8+1, 12*8+2, 12*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 16*8, 17*8, 18*8, 19*8,
		32*8, 33*8, 34*8, 35*8, 48*8, 49*8, 50*8, 51*8 },
	32*16
};

static GFXDECODE_START( panicr )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0x000,  8 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout,   0x100, 16 )
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout, 0x200, 16 )
GFXDECODE_END

static MACHINE_DRIVER_START( panicr )
	MDRV_CPU_ADD("maincpu", V20,MASTER_CLOCK/2) /* Sony 8623h9 CXQ70116D-8 (V20 compatible) */
	MDRV_CPU_PROGRAM_MAP(panicr_map)
	MDRV_CPU_VBLANK_INT_HACK(panicr_interrupt,2)

	MDRV_CPU_ADD(CPUTAG_T5182,Z80,SOUND_CLOCK/4)	/* 3.579545 MHz */
	MDRV_CPU_PROGRAM_MAP(t5182_map)
	MDRV_CPU_IO_MAP(t5182_io)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(panicr)
	MDRV_PALETTE_LENGTH(256*3)
	MDRV_PALETTE_INIT(panicr)

	MDRV_VIDEO_START(panicr)
	MDRV_VIDEO_UPDATE(panicr)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ymsnd", YM2151, SOUND_CLOCK/4)	/* 3.579545 MHz */
	MDRV_SOUND_CONFIG(t5182_ym2151_interface)
	MDRV_SOUND_ROUTE(0, "mono", 1.0)
	MDRV_SOUND_ROUTE(1, "mono", 1.0)

MACHINE_DRIVER_END

ROM_START( panicr )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* v20 main cpu */
	ROM_LOAD16_BYTE("2.19m",   0x0f0000, 0x08000, CRC(3d48b0b5) SHA1(a6e8b38971a8964af463c16f32bb7dbd301dd314) )
	ROM_LOAD16_BYTE("1.19n",   0x0f0001, 0x08000, CRC(674131b9) SHA1(63499cd5ad39e79e70f3ba7060680f0aa133f095) )

	ROM_REGION( 0x10000, "t5182", 0 ) /* Toshiba T5182 module */
	ROM_LOAD( "t5182.rom", 0x0000, 0x2000, CRC(d354c8fc) SHA1(a1c9e1ac293f107f69cc5788cf6abc3db1646e33) )
	ROM_LOAD( "22d.bin",   0x8000, 0x8000, CRC(eb1a46e1) SHA1(278859ae4bca9f421247e646d789fa1206dcd8fc) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "13f.bin", 0x000000, 0x2000, CRC(4e6b3c04) SHA1(f388969d5d822df0eaa4d8300cbf9cee47468360) )
	ROM_LOAD( "15f.bin", 0x002000, 0x2000, CRC(d735b572) SHA1(edcdb6daec97ac01a73c5010727b1694f512be71) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "2a.bin", 0x000000, 0x20000, CRC(3ac0e5b4) SHA1(96b8bdf02002ec8ce87fd47fd21f7797a79d79c9) )
	ROM_LOAD( "2b.bin", 0x020000, 0x20000, CRC(567d327b) SHA1(762b18ef1627d71074ba02b0eb270bd9a01ac0d8) )
	ROM_LOAD( "2c.bin", 0x040000, 0x20000, CRC(cd77ec79) SHA1(94b61b7d77c016ae274eddbb1e66e755f312e11d) )
	ROM_LOAD( "2d.bin", 0x060000, 0x20000, CRC(218d2c3e) SHA1(9503b3b67e71dc63448aed7815845b844e240afe) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "2j.bin", 0x000000, 0x20000, CRC(80f05923) SHA1(5c886446fd77d3c39cb4fa43ea4beb8c89d20636) )
	ROM_LOAD( "2k.bin", 0x020000, 0x20000, CRC(35f07bca) SHA1(54e6f82c2e6e1373c3ac1c6138ef738e5a0be6d0) )

	ROM_REGION( 0x04000, "user1", 0 )
	ROM_LOAD( "5d.bin", 0x00000, 0x4000, CRC(f3466906) SHA1(42b512ba93ba7ac958402d1871c5ae015def3501) ) //tilemaps
	ROM_REGION( 0x04000, "user2", 0 )
	ROM_LOAD( "7d.bin", 0x00000, 0x4000, CRC(8032c1e9) SHA1(fcc8579c0117ebe9271cff31e14a30f61a9cf031) ) //attribute maps

	ROM_REGION( 0x04000, "user3", 0 )
	ROM_COPY( "user2", 0x0000, 0x0000, 0x4000 )

	ROM_REGION( 0x0800,  "proms", 0 )
	ROM_LOAD( "b.14c",   0x00000, 0x100, CRC(145d1e0d) SHA1(8073fd176a1805552a5ac00ca0d9189e6e8936b1) )	// red
	ROM_LOAD( "a.15c",   0x00100, 0x100, CRC(c75772bc) SHA1(ec84052aedc1d53f9caba3232ffff17de69561b2) )	// green
	ROM_LOAD( "c.13c",   0x00200, 0x100, CRC(11c11bbd) SHA1(73663b2cf7269a62011ee067a026269ce0c15a7c) )	// blue
	ROM_LOAD( "12d.bpr", 0x00300, 0x100, CRC(0df8aa3c) SHA1(5149265d788ea4885793b0786f765524b4745f04) )	// txt lookup table
	ROM_LOAD( "8a.bpr",  0x00400, 0x100, CRC(908684a6) SHA1(82d9cb8aed576d1132615b5341c36ef51856b3a6) )	// tile lookup table
	ROM_LOAD( "10j.bpr", 0x00500, 0x100, CRC(1dd80ee1) SHA1(2d634e75666b919446e76fd35a06af27a1a89707) )	// sprite lookup table
	ROM_LOAD( "d.9b",    0x00600, 0x100, CRC(f99cac4b) SHA1(b4e6d0e0186fe186e747a9f6857b97591948c682) )	// unknown
	ROM_LOAD( "10l.bpr", 0x00700, 0x100, CRC(f3f29695) SHA1(2607e96564a5e6e9a542377a01f399ea86a36c48) )	// unknown
ROM_END


static DRIVER_INIT( panicr )
{
	UINT8 *buf = auto_alloc_array(machine, UINT8, 0x80000);
	UINT8 *rom;
	int size;
	int i;

	rom = memory_region(machine, "gfx1");
	size = memory_region_length(machine, "gfx1");

	// text data lines
	for (i = 0;i < size/2;i++)
	{
		int w1;

		w1 = (rom[i + 0*size/2] << 8) + rom[i + 1*size/2];

		w1 = BITSWAP16(w1,  9,12,7,3,  8,13,6,2, 11,14,1,5, 10,15,4,0);

		buf[i + 0*size/2] = w1 >> 8;
		buf[i + 1*size/2] = w1 & 0xff;
	}

	// text address lines
	for (i = 0;i < size;i++)
	{
		rom[i] = buf[BITSWAP24(i,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6, 2,3,1,0,5,4)];
	}


	rom = memory_region(machine, "gfx2");
	size = memory_region_length(machine, "gfx2");

	// tiles data lines
	for (i = 0;i < size/4;i++)
	{
		int w1,w2;

		w1 = (rom[i + 0*size/4] << 8) + rom[i + 3*size/4];
		w2 = (rom[i + 1*size/4] << 8) + rom[i + 2*size/4];

		w1 = BITSWAP16(w1, 14,12,11,9,   3,2,1,0, 10,15,13,8,   7,6,5,4);
		w2 = BITSWAP16(w2,  3,13,15,4, 12,2,5,11, 14,6,1,10,    8,7,9,0);

		buf[i + 0*size/4] = w1 >> 8;
		buf[i + 1*size/4] = w1 & 0xff;
		buf[i + 2*size/4] = w2 >> 8;
		buf[i + 3*size/4] = w2 & 0xff;
	}

	// tiles address lines
	for (i = 0;i < size;i++)
	{
		rom[i] = buf[BITSWAP24(i,23,22,21,20,19,18,17,16,15,14,13,12, 5,4,3,2, 11,10,9,8,7,6, 0,1)];
	}


	rom = memory_region(machine, "gfx3");
	size = memory_region_length(machine, "gfx3");

	// sprites data lines
	for (i = 0;i < size/2;i++)
	{
		int w1;

		w1 = (rom[i + 0*size/2] << 8) + rom[i + 1*size/2];


		w1 = BITSWAP16(w1, 11,5,7,12, 4,10,13,3, 6,14,9,2, 0,15,1,8);


		buf[i + 0*size/2] = w1 >> 8;
		buf[i + 1*size/2] = w1 & 0xff;
	}

	// sprites address lines
	for (i = 0;i < size;i++)
	{
		rom[i] = buf[i];
	}

	//rearrange  bg tilemaps a bit....

	rom = memory_region(machine, "user1");
	size = memory_region_length(machine, "user1");
	memcpy(buf,rom, size);

	{
		int j;
		for(j=0;j<16;j++)
			for (i = 0;i < size/16;i+=8)
			{
				memcpy(&rom[i+(size/16)*j],&buf[i*16+8*j],8);
			}
	}

	rom = memory_region(machine, "user2");
	size = memory_region_length(machine, "user2");

	memcpy(buf,rom, size);
	{
		int j;
		for(j=0;j<16;j++)
			for (i = 0;i < size/16;i+=8)
			{
				memcpy(&rom[i+(size/16)*j],&buf[i*16+8*j],8);
			}
	}

	auto_free(machine, buf);
}


GAME( 1986, panicr,  0,       panicr,  panicr,  panicr, ROT270, "Taito", "Panic Road", GAME_NOT_WORKING )
