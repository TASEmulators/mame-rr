/***************************************************************************************************

    Cycle Mahbou (c) 1984 Taito Corporation / Seta

    appears to be in the exact middle between the gsword / josvolly HW and the ppking / gladiator HW

    preliminary driver by Angelo Salese

    TODO:
    - protection (two 8741);
    - colors;
    - fix remaining video issues;
    - sound;
    - add flipscreen;

    (wait until it completes the post test, then put 1 to be23)

=====================================================================================================

Cycle Mahbou
(c)1984 Taito/Seta

----------------------------------------
Top
P1-002A
----------------------------------------
P0_20.3D     [53e3a36e]
P0_21.3E     [a7dab6d8]


----------------------------------------
Bottom
P0-001A
CPU  :Z80A x2
OSC  :18.000MHz
Other:AP-001,AP-004,AP-005,AP-006,P7,P8,P9
----------------------------------------
P0_1.1A      [a1588264]
P0_2.1B      [04141837]
P0_3.1C      [a9dd4b22]
P0_4.1E      [456a30df]
P0_5.1F      [a3b9c297]
P0_6.1H      [ec76a0a6]
P0_7.1K      [6507d23f]
P0_10.1N     [a98415db]
P0_11.1R     [626556fe]
P0_12.1S     [1e08902c]
P0_13.1T     [086639c1]
P0_14.1U     [3f5fe2b6]

P0_15.10C    [9cc52c32]
P0_16.10D    [8d03227e]

AP-002.7B    [Not Dump] 8741
AP-003.7C    [Not Dump] /

P1.2E        [6297104c] 82S123
P2.4E        [70a09cc5] /

P0_3.11T     [be89c1f7] 82S129
P0_4.11U     [4886d832] /



--- Team Japump!!! ---
Dumped by Chack'n
27/Nov/2009
28/Nov/2009

****************************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "machine/tait8741.h"

static UINT8 *cyclemb_vram,*cyclemb_cram;
static UINT8 *cyclemb_obj1_ram,*cyclemb_obj2_ram,*cyclemb_obj3_ram;

static PALETTE_INIT( cyclemb )
{
	int i,r,g,b,val;
	int bit0,bit1,bit2;

	for (i = 0; i < 256; i++)
	{
		val = (color_prom[i+0x100]) | (color_prom[i+0x000]<<4);

		bit0 = 0;
		bit1 = (val >> 6) & 0x01;
		bit2 = (val >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (val >> 3) & 0x01;
		bit1 = (val >> 4) & 0x01;
		bit2 = (val >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (val >> 0) & 0x01;
		bit1 = (val >> 1) & 0x01;
		bit2 = (val >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
	}
}

static VIDEO_START( cyclemb )
{

}

static VIDEO_UPDATE( cyclemb )
{
	int x,y,count;
	const gfx_element *gfx = screen->machine->gfx[0];
	UINT8 flip_screen = flip_screen_get(screen->machine);

	count = 0;

	for (y=0;y<32;y++)
	{
		for (x=0;x<64;x++)
		{
			int attr = cyclemb_cram[count];
			int tile = (cyclemb_vram[count]) | ((attr & 3)<<8);
			int color = ((attr & 0xf8) >> 3) ^ 0x1f;
			int odd_line = y & 1 ? 0x40 : 0x00;
//          int sx_offs = flip_screen ? 512 : 0
			int scrollx = ((cyclemb_vram[(y/2)+odd_line]) + (cyclemb_cram[(y/2)+odd_line]<<8) + 48) & 0x1ff;

			if(flip_screen)
			{
				drawgfx_opaque(bitmap,cliprect,gfx,tile,color,1,1,512-(x*8)-scrollx,256-(y*8));
				/* wrap-around */
				drawgfx_opaque(bitmap,cliprect,gfx,tile,color,1,1,512-(x*8)-scrollx+512,256-(y*8));
			}
			else
			{
				drawgfx_opaque(bitmap,cliprect,gfx,tile,color,0,0,(x*8)-scrollx,(y*8));
				/* wrap-around */
				drawgfx_opaque(bitmap,cliprect,gfx,tile,color,0,0,(x*8)-scrollx+512,(y*8));
			}

			count++;
		}
	}

	/*
    bank 1
    xxxx xxxx [0] sprite offset
    ---x xxxx [1] color offset
    bank 2
    xxxx xxxx [0] y offs
    xxxx xxxx [1] x offs
    bank 3
    ---- ---x [1] sprite enable flag?
    */
	{
		UINT8 col,fx,fy,region;
		UINT16 spr_offs,i;
		INT16 x,y;

		/*
        0x3b-0x3c-0x3d tire (0x13 0x00 / 0x17 0x00 )
        0x3b- shirt (0x16 0x00)
        0x20 tire stick (0x16 0x00)
        0x2e go sign (0x11 0x00)
        0x18 trampoline (0x13 0x00)
        0x27 cone (0x13 0x00)
        */

		for(i=0;i<0x40;i+=2)
		{
			y = 0xf1 - cyclemb_obj2_ram[i];
			x = cyclemb_obj2_ram[i+1] - 56;
			spr_offs = (cyclemb_obj1_ram[i+0]);
			col = (cyclemb_obj1_ram[i+1] & 0x3f);
			region = ((cyclemb_obj3_ram[i] & 0x10) >> 4) + 1;
			if(region == 2)
			{
				spr_offs >>= 2;
				spr_offs += ((cyclemb_obj3_ram[i+0] & 3) << 5);
				y-=16;
			}

			if(cyclemb_obj3_ram[i+1] & 1)
				x+=256;
			//if(cyclemb_obj3_ram[i+1] & 2)
//              x-=256;
			fx = (cyclemb_obj3_ram[i+0] & 4) >> 2;
			fy = (cyclemb_obj3_ram[i+0] & 8) >> 3;

			if(flip_screen)
			{
				fx = !fx;
				fy = !fy;
			}
			drawgfx_transpen(bitmap,cliprect,screen->machine->gfx[region],spr_offs,col,fx,fy,x,y,0);
		}
	}

	return 0;
}

static WRITE8_HANDLER( cyclemb_bankswitch_w )
{
	memory_set_bank(space->machine, "bank1", data & 3);
}

#if 0
static WRITE8_HANDLER( sound_cmd_w )
{
	soundlatch_w(space, 0, data & 0xff);
	cputag_set_input_line(space->machine, "audiocpu", 0, HOLD_LINE);
}
#endif

#if 0
static READ8_HANDLER( mcu_status_r )
{
	return 1;
}


static WRITE8_HANDLER( sound_cmd_w ) //actually ciom
{
	soundlatch_w(space, 0, data & 0xff);
	cputag_set_input_line(space->machine, "audiocpu", 0, HOLD_LINE);
}
#endif

static WRITE8_HANDLER( cyclemb_flip_w )
{
	flip_screen_set(space->machine, data & 1);

	// a bunch of other things are setted here
}

static ADDRESS_MAP_START( cyclemb_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x8fff) AM_ROMBANK("bank1")
	AM_RANGE(0x9000, 0x97ff) AM_RAM AM_BASE(&cyclemb_vram)
	AM_RANGE(0x9800, 0x9fff) AM_RAM AM_BASE(&cyclemb_cram)
	AM_RANGE(0xa000, 0xa7ff) AM_RAM AM_BASE(&cyclemb_obj1_ram) //ORAM1 (only a000-a3ff tested)
	AM_RANGE(0xa800, 0xafff) AM_RAM AM_BASE(&cyclemb_obj2_ram) //ORAM2 (only a800-abff tested)
	AM_RANGE(0xb000, 0xb7ff) AM_RAM AM_BASE(&cyclemb_obj3_ram) //ORAM3 (only b000-b3ff tested)
	AM_RANGE(0xb800, 0xbfff) AM_RAM //WRAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( cyclemb_io, ADDRESS_SPACE_IO, 8 )
//  ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xc000, 0xc000) AM_WRITE(cyclemb_bankswitch_w)
	AM_RANGE(0xc09e, 0xc09f) AM_READWRITE(cyclemb_8741_0_r, cyclemb_8741_0_w)
	AM_RANGE(0xc0bf, 0xc0bf) AM_WRITE(cyclemb_flip_w) //flip screen
ADDRESS_MAP_END

static ADDRESS_MAP_START( cyclemb_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x6000, 0x63ff) AM_RAM

ADDRESS_MAP_END

static ADDRESS_MAP_START( cyclemb_sound_io, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVREADWRITE("aysnd", ay8910_r, ay8910_address_data_w)
	AM_RANGE(0x40, 0x40) AM_READ(soundlatch_r) AM_WRITE(soundlatch2_w)
ADDRESS_MAP_END

static MACHINE_RESET( cyclemb )
{
	cyclemb_8741_reset(machine);
}


static INPUT_PORTS_START( cyclemb )
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x01, "IN0" )
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
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "IN1" )
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
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, "IN2" )
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
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN3")
	PORT_DIPNAME( 0x01, 0x01, "IN3" )
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
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN4")
	PORT_DIPNAME( 0x01, 0x01, "IN4" )
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
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "DSW1" )
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
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "DSW2" )
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
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "DSW3" )
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
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ 0,1,2,3,64,65,66,67 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8 },
	16*8
};

static const gfx_layout spritelayout_16x16 =
{
	16,16,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 24*8+0, 24*8+1, 24*8+2, 24*8+3},
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8 },
	64*8
};

static const gfx_layout spritelayout_32x32 =
{
	32,32,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 24*8+0, 24*8+1, 24*8+2, 24*8+3,
			64*8+0, 64*8+1, 64*8+2, 64*8+3, 72*8+0, 72*8+1, 72*8+2, 72*8+3,
			80*8+0, 80*8+1, 80*8+2, 80*8+3, 88*8+0, 88*8+1, 88*8+2, 88*8+3},
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8,
			128*8, 129*8, 130*8, 131*8, 132*8, 133*8, 134*8, 135*8,
			160*8, 161*8, 162*8, 163*8, 164*8, 165*8, 166*8, 167*8 },
	64*8*4    /* every sprite takes (64*8=16x6)*4) bytes */
};

static GFXDECODE_START( cyclemb )
	GFXDECODE_ENTRY( "tilemap_data", 0, charlayout,     0, 0x40 )
	GFXDECODE_ENTRY( "sprite_data_1", 0, spritelayout_16x16,    0x00, 0x40 )
	GFXDECODE_ENTRY( "sprite_data_2", 0, spritelayout_32x32,    0x00, 0x40 )
GFXDECODE_END

static MACHINE_DRIVER_START( cyclemb )
	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu",Z80,18000000/4)
	MDRV_CPU_PROGRAM_MAP(cyclemb_map)
	MDRV_CPU_IO_MAP(cyclemb_io)
	MDRV_CPU_VBLANK_INT("screen",irq0_line_hold)

	MDRV_CPU_ADD("audiocpu",Z80,18000000/4)
	MDRV_CPU_PROGRAM_MAP(cyclemb_sound_map)
	MDRV_CPU_IO_MAP(cyclemb_sound_io)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MDRV_GFXDECODE(cyclemb)
	MDRV_PALETTE_LENGTH(256)
	MDRV_PALETTE_INIT(cyclemb)

	MDRV_VIDEO_START(cyclemb)
	MDRV_VIDEO_UPDATE(cyclemb)

	MDRV_MACHINE_RESET(cyclemb)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("aysnd", AY8910, 18000000/16)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_DRIVER_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( cyclemb )
	ROM_REGION( 0x18000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "p0_1.1a",   0x00000, 0x2000, CRC(a1588264) SHA1(ff17df61207e39443a8ea62be1fce102c163d8e1) )
	ROM_LOAD( "p0_2.1b",   0x02000, 0x2000, CRC(04141837) SHA1(18d2f17fd5334b306ca13a1c26780f4a868a4ac8) )
	ROM_LOAD( "p0_3.1c",   0x04000, 0x2000, CRC(a9dd4b22) SHA1(8d3535ecd43aa0eccf3856b7cbad8702d17dd576) )
	ROM_LOAD( "p0_4.1e",   0x06000, 0x2000, CRC(456a30df) SHA1(75594178e6299ef5a81c134138ac1f1231a36caa) )
	ROM_LOAD( "p0_5.1f",   0x10000, 0x2000, CRC(a3b9c297) SHA1(edbab8639cb73e1376306ef70ef4ae451a75e4a9) )
	ROM_LOAD( "p0_6.1h",   0x12000, 0x2000, CRC(ec76a0a6) SHA1(9d1d3c050c76df42da53896f38ae53c5f79b0c5c) )

	ROM_REGION( 0x4000, "audiocpu", 0 )
	ROM_LOAD( "p0_15.10c",   0x0000, 0x2000, CRC(9cc52c32) SHA1(05d4e7c8ce8fdfc995013c0ed693b4d4778acc25) )
	ROM_LOAD( "p0_16.10d",   0x2000, 0x2000, CRC(8d03227e) SHA1(7e90437cbe5e854025e799348bb2cbca98368bd9) )

	ROM_REGION( 0x4000, "tilemap_data", 0 )
	ROM_LOAD( "p0_21.3e",   0x0000, 0x2000, CRC(a7dab6d8) SHA1(c5802e76abd394a2ce1526815bfbfc12e5e57587) )
	ROM_LOAD( "p0_20.3d",   0x2000, 0x2000, CRC(53e3a36e) SHA1(d95c1dfe216bb8b1f3e14c72a480eb2befa9d1dd) )

	ROM_REGION( 0x2000, "sprite_data_1", ROMREGION_ERASEFF )
	ROM_LOAD( "p0_7.1k",    0x0000, 0x2000, CRC(6507d23f) SHA1(1640b25a6efa0976f13ed7838f31ef53c37c8d2d) )

	ROM_REGION( 0xc000, "sprite_data_2", ROMREGION_ERASEFF )
	ROM_LOAD( "p0_10.1n",   0x4000, 0x2000, CRC(a98415db) SHA1(218a1d3ad27c30263daf87be87b4d5e06d5ac604) )
	ROM_LOAD( "p0_11.1r",   0x0000, 0x2000, CRC(626556fe) SHA1(ebd08a407fe466af14813bdeeb852d6816da932e) )
	ROM_LOAD( "p0_12.1s",   0x6000, 0x2000, CRC(1e08902c) SHA1(3d5f620580dc1fc43cd5f99b2a1e62a6d749f8b9) )
	ROM_LOAD( "p0_13.1t",   0x2000, 0x2000, CRC(086639c1) SHA1(3afbe76bb466d4c5916ef85d4cfc42e0c3f69883) )
	ROM_LOAD( "p0_14.1u",   0x8000, 0x2000, CRC(3f5fe2b6) SHA1(a7d1d0bc449f557ba827936b0fdbcccf7b1ee629) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "p0_3.11t",   0x0000, 0x100, CRC(be89c1f7) SHA1(7fb2d9fccf6c74130c3e0db4ea4269aeb45359e3) )
	ROM_LOAD( "p0_4.11u",   0x0100, 0x100, CRC(4886d832) SHA1(49e77923b7e2a0d5e9d990706dac258ecfd7720e) )

	ROM_REGION( 0x40, "timing_proms", 0 ) //???
	ROM_LOAD( "p1.2e",      0x000, 0x020, CRC(6297104c) SHA1(f2a40811505625a7a7ef4a7e4168c556c263449b) )
	ROM_LOAD( "p2.4e",      0x020, 0x020, CRC(70a09cc5) SHA1(82c0f3122d2c1e8be74b857737380c2e978adeef) )
ROM_END

static DRIVER_INIT( cyclemb )
{
	memory_configure_bank(machine, "bank1", 0, 4, memory_region(machine, "maincpu") + 0x10000, 0x1000);
}

GAME( 1984, cyclemb,  0,   cyclemb,  cyclemb,  cyclemb, ROT0, "Taito Corporation", "Cycle Mahbou (Japan)", GAME_NOT_WORKING )
