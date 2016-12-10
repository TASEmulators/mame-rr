/*************************************************************************************

Submarine (c) 1985 Sigma

driver by David Haywood & Angelo Salese

TODO:
- finish dip-switches;
- a bunch of unemulated writes at 0xe*** (I believe that there are individual
  flip screen x & y)
- flip screen support;

======================================================================================

 2 PCBs

 PCB1 - Bottom? (video?) board

|------------------------------------------------------------------- --|
|           B                                                          |
|     A           C     D     E     F     -     H     -    J           |
|                                                                      |
|                                                                      |
|1                                             (rom)                   |
|                                               OBJ1             T     |
|2                           Sony                                e    --
|                            CX23001           (rom)             x    --
|3                                              OBJ2             t    --
|                                                                     --C
|4                           (prom)            (rom)                  --N
|                              E4               OBJ3                  --3
|5                           (prom)                                   --
|                              E5                                     --
|6                           Sony                                     --
|                            CX23001                                  --
|7              (prom)                                                 |
|                 C7                                                   |
|8              (prom)                                                 |
|                 C8                                                   |
|9  (prom)                                                            --
|    A9    R                                                          --
|10 (prom) e                                                          --
|    A10   s                         HM6116P-3                        --C
|11 (prom)                                                            --N
|    A11                             (rom)                            --4
|12                                  VRAM1                            --
|                                                                     --
|13                                  (rom)                            --
|                                    VRAM2                            --
|14                                                                    |
|                                    (rom)                             |
|15                                  VRAM3                             |
|                                                                      |
|                                                                      |
|----------------------------------------------------------------------|

 Text = sigma enterprises, inc.
                 JAPAN F-021BCRT
         (rotated 90 degress left)

  Res = a bunch of resistors (colour weighting?)


PCB2  (Top board, CPU board)

|----------------------------------------------------------------------|
|                                                                      |
|         A       B       C       D      E       F       G      H      |
--|                                                        16000  18.432
  |                                                        OSC1    OSC2|
  |1           - - - - - - - - - - -                                   |
--|           |                                                    T   |
--|2                 NECD780        |                              e  --
--|           |      (z80 CPU)                                     x  --
--|  3                              |                              t  --
--|            - - - - - - - - - - -                                  --C
--|  4                                                                --N
--|                                                                   --3
--|C 5          T         T        T                                  --
  |N            E(rom)    E(rom)   E(rom)                             --
  |2 6          M         M        M                                  --
--|             P         P        P                                  --
|    7          1         2        3                                   |
--|C                                                                   |
  |N 8                                                                 |
  |1                                                                   |
--|  9                                                                --
--|                                                                   --
--| 10                                                                --
--|                                                                   --C
--| 11                                                                --N
--|                                                                   --4
--| 12                             DSW1 DSW2                          --
--|                                                                   --
--| 13                                                                --
  |                                                                   --
  | 14                                                         L       |
--|                                AY8910     R(rom)           H(z80A) |
|          15                                 O                0       |
|                                  AY8910     M                0       |
|          16                                 M                8       |
|                                                              0A      |
|----------------------------------------------------------------------|

 Text = F-020   CPU1 JAPAN   sigma enterprises, inc.

*************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"

#define MASTER_CLOCK			XTAL_18_432MHz

class sub_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, sub_state(machine)); }

	sub_state(running_machine &machine) { }

	UINT8* vid;
	UINT8* attr;
	UINT8* scrolly;
	UINT8* spriteram;
	UINT8* spriteram2;
	UINT8 nmi_en;
};

static VIDEO_START(sub)
{
}

static VIDEO_UPDATE(sub)
{
	sub_state *state = (sub_state *)screen->machine->driver_data;
	const gfx_element *gfx = screen->machine->gfx[0];
	const gfx_element *gfx_1 = screen->machine->gfx[1];
	int y,x;
	int count = 0;

	for (y=0;y<32;y++)
	{
		for (x=0;x<32;x++)
		{
			UINT16 tile = state->vid[count];
			UINT8 col;
			UINT8 y_offs = state->scrolly[x];

			tile += (state->attr[count]&0xe0)<<3;
			col = (state->attr[count]&0x1f);

			drawgfx_opaque(bitmap,cliprect,gfx,tile,col+0x40,0,0,x*8,(y*8)-y_offs);
			drawgfx_opaque(bitmap,cliprect,gfx,tile,col+0x40,0,0,x*8,(y*8)-y_offs+256);

			count++;
		}
	}


	/*
    sprite bank 1
    0 xxxx xxxx X offset
    1 tttt tttt tile offset
    sprite bank 2
    0 yyyy yyyy Y offset
    1 f--- ---- flips the X offset
    1 -f-- ---- flip y, inverted
    1 --cc cccc color
    */
	{
		UINT8 *spriteram = state->spriteram;
		UINT8 *spriteram_2 = state->spriteram2;
		UINT8 x,y,spr_offs,i,col,fx,fy;

		for(i=0;i<0x40;i+=2)
		{
			spr_offs = spriteram[i+1];
			x = spriteram[i+0];
			y = 0xe0 - spriteram_2[i+1];
			col = (spriteram_2[i+0])&0x3f;
			fx = (spriteram_2[i+0] & 0x80) ? 0 : 1;
			if(fx) { x = 0xe0 - x; }
			fy = (spriteram_2[i+0] & 0x40) ? 0 : 1;

			drawgfx_transpen(bitmap,cliprect,gfx_1,spr_offs,col,0,fy,x,y,0);
		}
	}

	count = 0;

	/* re-draw score display above the sprites (window effect) */
	for (y=0;y<32;y++)
	{
		for (x=0;x<32;x++)
		{
			UINT16 tile = state->vid[count];
			UINT8 col;
			UINT8 y_offs = state->scrolly[x];

			tile += (state->attr[count]&0xe0)<<3;
			col = (state->attr[count]&0x1f);

			if(x >= 28)
			{
				drawgfx_opaque(bitmap,cliprect,gfx,tile,col+0x40,0,0,x*8,(y*8)-y_offs);
				drawgfx_opaque(bitmap,cliprect,gfx,tile,col+0x40,0,0,x*8,(y*8)-y_offs+256);
			}

			count++;
		}
	}

	return 0;
}

static ADDRESS_MAP_START( subm_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xafff) AM_ROM
	AM_RANGE(0xb000, 0xbfff) AM_RAM
	AM_RANGE(0xc000, 0xc3ff) AM_RAM AM_BASE_MEMBER(sub_state,attr)
	AM_RANGE(0xc400, 0xc7ff) AM_RAM AM_BASE_MEMBER(sub_state,vid)
	AM_RANGE(0xd000, 0xd03f) AM_RAM AM_BASE_MEMBER(sub_state,spriteram)
	AM_RANGE(0xd800, 0xd83f) AM_RAM AM_BASE_MEMBER(sub_state,spriteram2)
	AM_RANGE(0xd840, 0xd85f) AM_RAM AM_BASE_MEMBER(sub_state,scrolly)

	AM_RANGE(0xe000, 0xe000) AM_NOP
	AM_RANGE(0xe800, 0xe800) AM_NOP
	AM_RANGE(0xe801, 0xe801) AM_NOP
	AM_RANGE(0xe802, 0xe802) AM_NOP
	AM_RANGE(0xe803, 0xe803) AM_NOP
	AM_RANGE(0xe805, 0xe805) AM_NOP

	AM_RANGE(0xf000, 0xf000) AM_READ_PORT("DSW0") // DSW0?
	AM_RANGE(0xf020, 0xf020) AM_READ_PORT("DSW1") // DSW1?
	AM_RANGE(0xf040, 0xf040) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xf060, 0xf060) AM_READ_PORT("IN0")
ADDRESS_MAP_END

static WRITE8_HANDLER( subm_to_sound_w )
{
	soundlatch_w(space, 0, data & 0xff);
	cputag_set_input_line(space->machine, "soundcpu", 0, HOLD_LINE);
}

static WRITE8_HANDLER( nmi_mask_w )
{
	sub_state *state = (sub_state *)space->machine->driver_data;

	state->nmi_en = data & 1;
}

static ADDRESS_MAP_START( subm_io, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READWRITE(soundlatch2_r, subm_to_sound_w) // to/from sound CPU
ADDRESS_MAP_END

static ADDRESS_MAP_START( subm_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x47ff) AM_RAM
	AM_RANGE(0x6000, 0x6000) AM_WRITE(nmi_mask_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( subm_sound_io, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READWRITE(soundlatch_r, soundlatch2_w) // to/from main CPU
	AM_RANGE(0x40, 0x41) AM_DEVREADWRITE("ay1", ay8910_r, ay8910_address_data_w)
	AM_RANGE(0x80, 0x81) AM_DEVREADWRITE("ay2", ay8910_r, ay8910_address_data_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( sub )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START1 )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x01, "DSWC" )
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
	PORT_DIPNAME( 0xf0, 0x10, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
// Duplicates
//  PORT_DIPSETTING(    0xa0, DEF_STR( 1C_3C ) )
//  PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
//  PORT_DIPSETTING(    0xc0, DEF_STR( 1C_6C ) )
//  PORT_DIPSETTING(    0xd0, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Upright ) )	/* Seperate controls for each player */
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) ) /* Controls via player 1 for both, but need to get x/y screen flip working to fully test */
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END



static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 },
	8*8
};

static const gfx_layout tiles16x32_layout = {
	16,32,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ 64+0, 64+1, 64+2, 64+3, 64+4, 64+5, 64+6, 64+7, 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 55*8, 54*8, 53*8, 52*8, 51*8, 50*8, 49*8, 48*8,
	  39*8, 38*8, 37*8, 36*8, 35*8, 34*8, 33*8, 32*8,
	  23*8, 22*8, 21*8, 20*8, 19*8, 18*8, 17*8, 16*8,
	   7*8,  6*8,  5*8,  4*8,  3*8,  2*8,  1*8,  0*8
	},
	64*8
};

static GFXDECODE_START( sub )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0, 0x80 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles16x32_layout, 0, 0x80 )
GFXDECODE_END

static PALETTE_INIT( sub )
{
	int i;
	UINT8* lookup = memory_region(machine,"proms2");

	/* allocate the colortable */
	machine->colortable = colortable_alloc(machine, 0x100);

	for (i = 0;i < 0x100;i++)
	{
		int r,g,b;
		r = (color_prom[0x000] >> 0);
		g = (color_prom[0x100] >> 0);
		b = (color_prom[0x200] >> 0);

		//colortable_palette_set_color(machine->colortable, i, MAKE_RGB(r, g, b));
		colortable_palette_set_color(machine->colortable, i, MAKE_RGB(pal4bit(r), pal4bit(g), pal4bit(b)));

		color_prom++;
	}


	for (i = 0;i < 0x400;i++)
	{
		UINT8 ctabentry = lookup[i+0x400] | (lookup[i+0x000] << 4);
		colortable_entry_set_value(machine->colortable, i, ctabentry);
	}

}


static INTERRUPT_GEN( subm_sound_irq )
{
	sub_state *state = (sub_state *)device->machine->driver_data;

	if(state->nmi_en)
		cputag_set_input_line(device->machine, "soundcpu", INPUT_LINE_NMI, PULSE_LINE);
}

static MACHINE_DRIVER_START( sub )

	MDRV_DRIVER_DATA( sub_state )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80,MASTER_CLOCK/6)		 /* ? MHz */
	MDRV_CPU_PROGRAM_MAP(subm_map)
	MDRV_CPU_IO_MAP(subm_io)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_CPU_ADD("soundcpu", Z80,MASTER_CLOCK/6)		 /* ? MHz */
	MDRV_CPU_PROGRAM_MAP(subm_sound_map)
	MDRV_CPU_IO_MAP(subm_sound_io)
	MDRV_CPU_PERIODIC_INT(subm_sound_irq, 120) //???


	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 256-1, 16, 256-16-1)

	MDRV_GFXDECODE(sub)
	MDRV_PALETTE_LENGTH(0x400)
	MDRV_PALETTE_INIT(sub)

	MDRV_VIDEO_START(sub)
	MDRV_VIDEO_UPDATE(sub)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ay1", AY8910, MASTER_CLOCK/6/2) /* ? Mhz */
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.23)

	MDRV_SOUND_ADD("ay2", AY8910, MASTER_CLOCK/6/2) /* ? Mhz */
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.23)
MACHINE_DRIVER_END


ROM_START( sub )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "temp 1 pos b6 27128.bin",	  0x0000, 0x4000, CRC(6875b31d) SHA1(e7607e53687f1331cc97de939de144a7954ca3c3) )
	ROM_LOAD( "temp 2 pos c6 27128.bin",	  0x4000, 0x4000, CRC(bc7f8f43) SHA1(088156a66acb2214c638d9d1ad18e9836b27eff0) )
	ROM_LOAD( "temp 3 pos d6 2764.bin",	  0x8000, 0x2000, CRC(3546c226) SHA1(35e53c0db75c89e8e222d2139b841e77f5cc282c) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "m sound pos f14 2764.bin",	  0x0000, 0x2000, CRC(61536a97) SHA1(84effc2251bf7c91e0bb670a651117503de8940d) )
	ROM_RELOAD( 0x2000, 0x2000 )

	ROM_REGION( 0xc000, "gfx1", 0)
	ROM_LOAD( "vram 1 pos f12 27128  version3.bin",	  0x0000, 0x4000, CRC(8d176ba0) SHA1(b0bf4af97e991545d6b38e8159eb909376e6df35) )
	ROM_LOAD( "vram 2 pos f14 27128  version3.bin",	  0x4000, 0x4000, CRC(0677cf3a) SHA1(072e9391f6a230b78124e820da0f0d27ffa45dc3) )
	ROM_LOAD( "vram 3 pos f15 27128  version3.bin",	  0x8000, 0x4000, CRC(9a4cd1a0) SHA1(a321b88386424d73d7d73a7f321317b0f21d2eb6) )

	ROM_REGION( 0xc000, "gfx2", 0 )
	ROM_LOAD( "obj 1 pos h1 27128  version3.bin",	  0x0000, 0x4000, CRC(63173e65) SHA1(2be3776c0e08d2c876cfce842e02345389e1fba0) )
	ROM_LOAD( "obj 2 pos h3 27128  version3.bin",	  0x4000, 0x4000, CRC(3898d1a8) SHA1(acd3d7695a0fe9faa5e4315032c65e131d24a3ce) )
	ROM_LOAD( "obj 3 pos h4 27128  version3.bin",	  0x8000, 0x4000, CRC(304e2145) SHA1(d4eb49b5502872718d64e53f02acd2150f6bf713) )

	ROM_REGION( 0x300, "proms", 0 ) // color proms
	ROM_LOAD( "prom pos a9 n82s129",	  0x0200, 0x100, CRC(8df9cefe) SHA1(86320eb8135932d79c4478929b9fd90ffba55712) )
	ROM_LOAD( "prom pos a10 n82s129",	  0x0100, 0x100, CRC(3c834094) SHA1(4d681431376a8ed071566d74d4accc737bf965dd) )
	ROM_LOAD( "prom pos a11 n82s129",	  0x0000, 0x100, CRC(339afa95) SHA1(ff4ff712960f41c26419a681e8dcceaeef75d2e3) )

	ROM_REGION( 0x800, "proms2", 0 ) // look-up tables
	ROM_LOAD( "prom pos e5 n82s131",	  0x0000, 0x200, CRC(0024b5dd) SHA1(7d623f8e8964336d643820850cef0fb641e52e22) )
	ROM_LOAD( "prom pos c7 n82s129",	  0x0200, 0x100, CRC(9072d259) SHA1(9679fa01372d14a866836c9193204ff6e33cf67c) )
	ROM_LOAD( "prom pos e4 n82s131",	  0x0400, 0x200, CRC(307aa2cf) SHA1(839eccf1d34adaf9a5006bfb30e3524bc19a9b41) )
	ROM_LOAD( "prom pos c8 n82s129",	  0x0600, 0x100, CRC(351e1ef8) SHA1(530c9012ff5abda1c4ba9787ca999ca1ae1a893d) )
ROM_END

GAME( 1985, sub,  0,    sub, sub,  0, ROT270, "Sigma Enterprises Inc.", "Submarine", GAME_NO_COCKTAIL )
