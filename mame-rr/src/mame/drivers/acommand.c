/*******************************************************************************************

Alien Command (c) 1993 Jaleco

driver by Angelo Salese

Actually same HW as the Cisco Heat ones.

TODO:
-Understand what "devices" area needs to make this working.It's likely that the upper switches
controls the UFO's and the lower switches the astronauts.
-Back tilemap paging is likely to be incorrect.
-3D Artworks for the UFO's,Astronauts etc.
-Merge to the Cisco Heat driver.

Notes:
-The real HW is a redemption machine with two guns, similar to the "Cosmo Gang the Video"
(Namco) bonus games.

m68k irq table vectors
lev 1 : 0x64 : 0000 04f0 - rte
lev 2 : 0x68 : 0000 044a - vblank
lev 3 : 0x6c : 0000 0484 - "dynamic color change" (?)
lev 4 : 0x70 : 0000 04f0 - rte
lev 5 : 0x74 : 0000 04f0 - rte
lev 6 : 0x78 : 0000 04f0 - rte
lev 7 : 0x7c : 0000 04f0 - rte

===========================================================================================

Jaleco Alien Command
Redemption Video Game with Guns

2/7/99

Hardware Specs: 68000 at 12Mhz and OKI6295

JALMR17  BIN       524,288  02-07-99  1:17a JALMR17.BIN
JALCF2   BIN     1,048,576  02-07-99  1:10a JALCF2.BIN
JALCF3   BIN       131,072  02-07-99  1:12a JALCF3.BIN
JALCF4   BIN       131,072  02-07-99  1:13a JALCF4.BIN
JALCF5   BIN       524,288  02-07-99  1:15a JALCF5.BIN
JALCF6   BIN       131,072  02-07-99  1:14a JALCF6.BIN
JALGP1   BIN       524,288  02-07-99  1:21a JALGP1.BIN
JALGP2   BIN       524,288  02-07-99  1:24a JALGP2.BIN
JALGP3   BIN       524,288  02-07-99  1:20a JALGP3.BIN
JALGP4   BIN       524,288  02-07-99  1:23a JALGP4.BIN
JALGP5   BIN       524,288  02-07-99  1:19a JALGP5.BIN
JALGP6   BIN       524,288  02-07-99  1:23a JALGP6.BIN
JALGP7   BIN       524,288  02-07-99  1:19a JALGP7.BIN
JALGP8   BIN       524,288  02-07-99  1:22a JALGP8.BIN
JALMR14  BIN       524,288  02-07-99  1:17a JALMR14.BIN
JALCF1   BIN     1,048,576  02-07-99  1:11a JALCF1.BIN


*******************************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "deprecat.h"
#include "sound/okim6295.h"

static tilemap_t *tx_tilemap,*bg_tilemap;
static UINT16 *ac_txvram,*ac_bgvram;
static UINT16 *ac_vregs;

static TILEMAP_MAPPER( bg_scan )
{
	/* logical (col,row) -> memory offset */
	return (row & 0x0f) + ((col & 0xff) << 4) + ((row & 0x70) << 8);
}

static TILE_GET_INFO( ac_get_bg_tile_info )
{
	int code = ac_bgvram[tile_index];
	SET_TILE_INFO(
			1,
			code & 0xfff,
			(code & 0xf000) >> 12,
			0);
}

static TILE_GET_INFO( ac_get_tx_tile_info )
{
	int code = ac_txvram[tile_index];
	SET_TILE_INFO(
			0,
			code & 0xfff,
			(code & 0xf000) >> 12,
			0);
}

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int priority, int pri_mask)
{
	UINT16 *spriteram16 = machine->generic.spriteram.u16;
	int offs;

	for (offs = 0;offs < machine->generic.spriteram_size/2;offs += 8)
	{
		if (!(spriteram16[offs+0] & 0x1000))
		{
			int sx = (spriteram16[offs+3] & 0x0ff);
			int code = spriteram16[offs+6];
			int color = spriteram16[offs+7];
			int w = (spriteram16[offs+0] & 0x0f);
			int h = ((spriteram16[offs+0] & 0xf0) >> 4);
			int sy = (spriteram16[offs+4] & 0x0ff) - ((h+1)*0x10);
/**/		int pri = spriteram16[offs+5];
/**/		int flipy = ((spriteram16[offs+1] & 0x0200) >> 9);
/**/		int flipx = ((spriteram16[offs+1] & 0x0100) >> 8);

			int xx,yy,x;
			int delta = 16;

			flipx ^= flip_screen_get(machine);
			flipy ^= flip_screen_get(machine);

			if ((pri&pri_mask)!=priority) continue;

			if (flip_screen_get(machine))
			{
				sx = 368 - sx;
				sy = 240 - sy;
				delta = -16;
			}

			yy = h;
			do
			{
				x = sx;
				xx = w;
				do
				{
					drawgfx_transpen(bitmap,cliprect,machine->gfx[2],
							code,
							color,
							flipx, flipy,
							((x + 16) & 0x1ff) - 16,sy & 0x1ff,15);

					code++;
					x += delta;
				} while (--xx >= 0);

				sy += delta;
			} while (--yy >= 0);
		}
	}
}


static VIDEO_START( acommand )
{
	tx_tilemap = tilemap_create(machine, ac_get_tx_tile_info,tilemap_scan_cols,8,8,512,32);
	bg_tilemap = tilemap_create(machine, ac_get_bg_tile_info,bg_scan,16,16,256,16);

	ac_vregs = auto_alloc_array(machine, UINT16, 0x80/2);

	tilemap_set_transparent_pen(tx_tilemap,15);
}


#define LED_ON		0x01c00
#define LED_OFF		0x00000
/*
     a
    ---
f   | | b
    -g-
e   | | c
    ---
     d
a & 1
b & 2
c & 4
d & 8
e & 10
f & 20
g & 40
7f
*/
/*                                    0    1    2    3    4    5    6    7    8    9    a    b    c    d    e    f*/
static const UINT8 led_fill[0x10] = { 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x00,0x00,0x00,0x00,0x00,0x00};

static void draw_led(bitmap_t *bitmap, int x, int y,UINT8 value)
{
	plot_box(bitmap, x, y, 6, 10, 0x00000000);

	/*a*/
	*BITMAP_ADDR16(bitmap, y+0, x+1) = ((led_fill[value] & 0x0001) ? LED_ON : LED_OFF);
	*BITMAP_ADDR16(bitmap, y+0, x+2) = ((led_fill[value] & 0x0001) ? LED_ON : LED_OFF);
	*BITMAP_ADDR16(bitmap, y+0, x+3) = ((led_fill[value] & 0x0001) ? LED_ON : LED_OFF);
	/*b*/
	*BITMAP_ADDR16(bitmap, y+1, x+4) = ((led_fill[value] & 0x0002) ? LED_ON : LED_OFF);
	*BITMAP_ADDR16(bitmap, y+2, x+4) = ((led_fill[value] & 0x0002) ? LED_ON : LED_OFF);
	*BITMAP_ADDR16(bitmap, y+3, x+4) = ((led_fill[value] & 0x0002) ? LED_ON : LED_OFF);
	/*c*/
	*BITMAP_ADDR16(bitmap, y+5, x+4) = ((led_fill[value] & 0x0004) ? LED_ON : LED_OFF);
	*BITMAP_ADDR16(bitmap, y+6, x+4) = ((led_fill[value] & 0x0004) ? LED_ON : LED_OFF);
	*BITMAP_ADDR16(bitmap, y+7, x+4) = ((led_fill[value] & 0x0004) ? LED_ON : LED_OFF);
	/*d*/
	*BITMAP_ADDR16(bitmap, y+8, x+1) = ((led_fill[value] & 0x0008) ? LED_ON : LED_OFF);
	*BITMAP_ADDR16(bitmap, y+8, x+2) = ((led_fill[value] & 0x0008) ? LED_ON : LED_OFF);
	*BITMAP_ADDR16(bitmap, y+8, x+3) = ((led_fill[value] & 0x0008) ? LED_ON : LED_OFF);
	/*e*/
	*BITMAP_ADDR16(bitmap, y+5, x+0) = ((led_fill[value] & 0x0010) ? LED_ON : LED_OFF);
	*BITMAP_ADDR16(bitmap, y+6, x+0) = ((led_fill[value] & 0x0010) ? LED_ON : LED_OFF);
	*BITMAP_ADDR16(bitmap, y+7, x+0) = ((led_fill[value] & 0x0010) ? LED_ON : LED_OFF);
	/*f*/
	*BITMAP_ADDR16(bitmap, y+1, x+0) = ((led_fill[value] & 0x0020) ? LED_ON : LED_OFF);
	*BITMAP_ADDR16(bitmap, y+2, x+0) = ((led_fill[value] & 0x0020) ? LED_ON : LED_OFF);
	*BITMAP_ADDR16(bitmap, y+3, x+0) = ((led_fill[value] & 0x0020) ? LED_ON : LED_OFF);
	/*g*/
	*BITMAP_ADDR16(bitmap, y+4, x+1) = ((led_fill[value] & 0x0040) ? LED_ON : LED_OFF);
	*BITMAP_ADDR16(bitmap, y+4, x+2) = ((led_fill[value] & 0x0040) ? LED_ON : LED_OFF);
	*BITMAP_ADDR16(bitmap, y+4, x+3) = ((led_fill[value] & 0x0040) ? LED_ON : LED_OFF);
}

static UINT16 led0,led1;

static VIDEO_UPDATE( acommand )
{
	tilemap_draw(bitmap,cliprect,bg_tilemap,0,0);
	draw_sprites(screen->machine,bitmap,cliprect,0,0);
	tilemap_draw(bitmap,cliprect,tx_tilemap,0,0);

	/*Order might be wrong,but these for sure are the led numbers tested*/
	draw_led(bitmap,  0, 20, (led0 & 0x0f00) >> 8);
	draw_led(bitmap,  6, 20, (led0 & 0x00f0) >> 4);
	draw_led(bitmap, 12, 20, (led0 & 0x000f));

	draw_led(bitmap, 256-18,20,(led0 & 0xf000) >> 12);
	draw_led(bitmap, 256-12,20,(led1 & 0xf0) >> 4);
	draw_led(bitmap, 256-6,20, (led1 & 0xf));
	return 0;
}


static WRITE16_HANDLER( ac_bgvram_w )
{
	COMBINE_DATA(&ac_bgvram[offset]);
	tilemap_mark_tile_dirty(bg_tilemap,offset);
}

static WRITE16_HANDLER( ac_txvram_w )
{
	COMBINE_DATA(&ac_txvram[offset]);
	tilemap_mark_tile_dirty(tx_tilemap,offset);
}

static WRITE16_HANDLER(ac_bgscroll_w)
{
	switch(offset)
	{
		case 0: tilemap_set_scrollx(bg_tilemap,0,data); break;
		case 1: tilemap_set_scrolly(bg_tilemap,0,data); break;
		case 2: /*BG_TILEMAP priority?*/ break;
	}
}

static WRITE16_HANDLER(ac_txscroll_w)
{
	switch(offset)
	{
		case 0: tilemap_set_scrollx(tx_tilemap,0,data); break;
		case 1: tilemap_set_scrolly(tx_tilemap,0,data); break;
		case 2: /*TX_TILEMAP priority?*/ break;
	}
}

/******************************************************************************************/

static UINT16 *ac_devram;

static READ16_HANDLER(ac_devices_r)
{
	logerror("(PC=%06x) read at %04x\n",cpu_get_pc(space->cpu),offset*2);

	switch(offset)
	{
		case 0x0008/2:
			/*
                --x- ---- ---- ---- Ticket Dispenser - 2
                ---x ---- ---- ---- Ticket Dispenser - 1
                ---- -x-- ---- ---- Right Gun HIT
                ---- ---x ---- ---- Left Gun HIT
                ---- ---- --x- ---- Service Mode (Toggle)
                ---- ---- ---x ---- Service Coin
                ---- ---- ---- x--- COIN2
                ---- ---- ---- -x-- COIN1
                ---- ---- ---- --x- (Activate Test)
                ---- ---- ---- ---x (Advance Thru Tests)
            */
			return input_port_read(space->machine, "IN0");
		case 0x0014/2:
		case 0x0016/2:
			return okim6295_r(space->machine->device("oki1"),0);
		case 0x0018/2:
		case 0x001a/2:
			return okim6295_r(space->machine->device("oki2"),0);
		case 0x0040/2:
			/*
                "Upper switch / Under Switch"
                xx-x ---- xx-x xx-x
                -x-- ---- ---- ---- Catch Switch - 3
                --x- ---- ---- ---- Lower Switch - 3
                ---x ---- ---- ---- Upper Switch - 3
                ---- -x-- ---- ---- Catch Switch - 2
                ---- --x- ---- ---- Lower Switch - 2
                ---- ---x ---- ---- Upper Switch - 2
                ---- ---- -x-- ---- Catch Switch - 1
                ---- ---- --x- ---- Lower Switch - 1 (active high)
                ---- ---- ---x ---- Upper Switch - 1 (active low)
                ---- ---- ---- --xx Boss Door - Motor
            */
        //22dc8
		{
			static UINT16 ufo_sw1;
			ufo_sw1 = ac_devram[offset] & 3;
			if(ac_devram[offset] & 0x10)
				ufo_sw1|= 0x10;
			if(ac_devram[offset] & 0x40)
				ufo_sw1|= 0x20;
			if(ac_devram[offset] & 0x100)
				ufo_sw1|=0x100;
			if(ac_devram[offset] & 0x400)
				ufo_sw1|=0x200;
			if(ac_devram[offset] & 0x1000)
				ufo_sw1|=0x1000;
			if(ac_devram[offset] & 0x4000)
				ufo_sw1|=0x2000;
//          if(ac_devram[0x0048/2] & 0x0001)
//              ufo_sw1|=0x0040;
//          if(ac_devram[0x0048/2] & 0x0004)
//              ufo_sw1|=0x0400;
//          if(ac_devram[0x0048/2] & 0x0100)
//              ufo_sw1|=0x4000;
			return ufo_sw1;
		}
		case 0x0044/2:
			/*
                ---- ---- --x- ---- Lower Switch - 5
                ---- ---- ---x ---- Upper Switch - 5
                ---- ---- ---- --x- Lower Switch - 4 (active high)
                ---- ---- ---- ---x Upper Switch - 4 (active low)
            */
		{
			static UINT16 ufo_sw2;
			ufo_sw2 = 0;
			if(ac_devram[offset] & 0x01)
				ufo_sw2|= 1;
			if(ac_devram[offset] & 0x04)
				ufo_sw2|= 2;
			if(ac_devram[offset] & 0x10)
				ufo_sw2|=0x10;
			if(ac_devram[offset] & 0x40)
				ufo_sw2|=0x20;
			return ufo_sw2;
		}
		case 0x0048/2:
			return ac_devram[offset];
		case 0x005c/2:
			/*
                xxxx xxxx ---- ---- DIPSW4
                ---- ---- xxxx xxxx DIPSW3
            */
			return input_port_read(space->machine, "IN1");
	}
	return ac_devram[offset];
}

static WRITE16_HANDLER(ac_devices_w)
{
	COMBINE_DATA(&ac_devram[offset]);
	switch(offset)
	{
		case 0x00/2:
			if (ACCESSING_BITS_0_7)
			{
				okim6295_device *oki1 = space->machine->device<okim6295_device>("oki1");
				okim6295_device *oki2 = space->machine->device<okim6295_device>("oki2");
				oki1->set_bank_base(0x40000 * (data & 0x3));
				oki2->set_bank_base(0x40000 * (data & 0x30) >> 4);
			}
			break;
		case 0x14/2:
		case 0x16/2:
			if(ACCESSING_BITS_0_7)
				okim6295_w(space->machine->device("oki1"),0,data);
			break;
		case 0x18/2:
		case 0x1a/2:
			if(ACCESSING_BITS_0_7)
				okim6295_w(space->machine->device("oki2"),0,data);
			break;
		case 0x1c/2:
			/*IRQ mask?*/
			break;
		case 0x40/2:
			break;
		case 0x44/2:
			break;
		case 0x48/2:
			break;
		case 0x50/2:
			led0 = ac_devram[offset];
			//popmessage("%04x",led0);
			break;
		case 0x54/2:
			led1 = ac_devram[offset];
			//popmessage("%04x",led0);
			break;
	}
}

/*This is always zero ATM*/
static WRITE16_HANDLER(ac_unk2_w)
{
	if(data)
		popmessage("UNK-2 enabled %04x",data);
}

static ADDRESS_MAP_START( acommand_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x082000, 0x082005) AM_WRITE(ac_bgscroll_w)
	AM_RANGE(0x082100, 0x082105) AM_WRITE(ac_txscroll_w)
	AM_RANGE(0x082208, 0x082209) AM_WRITE(ac_unk2_w)
	AM_RANGE(0x0a0000, 0x0a3fff) AM_RAM_WRITE(ac_bgvram_w) AM_BASE(&ac_bgvram)
	AM_RANGE(0x0b0000, 0x0b3fff) AM_RAM_WRITE(ac_txvram_w) AM_BASE(&ac_txvram)
	AM_RANGE(0x0b8000, 0x0bffff) AM_RAM_WRITE(paletteram16_RRRRGGGGBBBBRGBx_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x0f0000, 0x0f7fff) AM_RAM
	AM_RANGE(0x0f8000, 0x0f8fff) AM_RAM AM_BASE_SIZE_GENERIC(spriteram)
	AM_RANGE(0x0f9000, 0x0fffff) AM_RAM
	AM_RANGE(0x100000, 0x1000ff) AM_READ(ac_devices_r) AM_WRITE(ac_devices_w) AM_BASE(&ac_devram)
ADDRESS_MAP_END

static INPUT_PORTS_START( acommand )
	PORT_START("IN0")
    PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
    PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2 )
    PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN1 )
    PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN2 )
    PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0020, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0040, 0x0040, "IN0" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, "Ticket Dispenser - 1" )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, "Ticket Dispenser - 2")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x0001, 0x0001, "IN2" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4,
			16*32+0*4, 16*32+1*4, 16*32+2*4, 16*32+3*4, 16*32+4*4, 16*32+5*4, 16*32+6*4, 16*32+7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
			8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	32*32
};

static GFXDECODE_START( acommand )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0x2700, 16 ) /*???*/
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout, 0x0f00, 256 )
	GFXDECODE_ENTRY( "gfx3", 0, tilelayout, 0x1800, 256 )
GFXDECODE_END

static INTERRUPT_GEN( acommand_irq )
{
	switch ( cpu_getiloops(device) )
	{
		case 0:		cpu_set_input_line(device, 3, HOLD_LINE);
		case 1:		cpu_set_input_line(device, 2, HOLD_LINE);
	}
}

static MACHINE_DRIVER_START( acommand )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu",M68000,12000000)
	MDRV_CPU_PROGRAM_MAP(acommand_map)
	MDRV_CPU_VBLANK_INT_HACK(acommand_irq,2)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MDRV_GFXDECODE(acommand)
	MDRV_PALETTE_LENGTH(0x4000)

	MDRV_VIDEO_START(acommand)
	MDRV_VIDEO_UPDATE(acommand)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_OKIM6295_ADD("oki1", 2112000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)

	MDRV_OKIM6295_ADD("oki2", 2112000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
MACHINE_DRIVER_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( acommand )
	ROM_REGION( 0x040000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "jalcf3.bin",   0x000000, 0x020000, CRC(f031abf7) SHA1(e381742fd6a6df4ddae42ddb3a074a55dc550b3c) )
	ROM_LOAD16_BYTE( "jalcf4.bin",   0x000001, 0x020000, CRC(dd0c0540) SHA1(3e788fcb30ae725bd0ec9b57424e3946db1e946f) )

	ROM_REGION( 0x20000, "gfx1", 0 ) /* BG0 */
	ROM_LOAD( "jalcf6.bin",   0x000000, 0x020000, CRC(442173d6) SHA1(56c02bc2761967040127977ecabe844fc45e2218) )

	ROM_REGION( 0x080000, "gfx2", 0 ) /* BG1 */
	ROM_LOAD( "jalcf5.bin",   0x000000, 0x080000, CRC(ff0be97f) SHA1(5ccab778318dec30849d7b7f25091d4aab8bde32) )

	ROM_REGION( 0x400000, "gfx3", 0 ) /* SPR */
	ROM_LOAD16_BYTE( "jalgp1.bin",   0x000000, 0x080000, CRC(c4aeeae2) SHA1(ee0d3dd93a604f8e1a96b55c4a1cd001d49f1157) )
	ROM_LOAD16_BYTE( "jalgp2.bin",   0x000001, 0x080000, CRC(f0e4e80e) SHA1(08252ef8b5e309cce2d4654410142f4ae9e3ef22) )
	ROM_LOAD16_BYTE( "jalgp3.bin",   0x100000, 0x080000, CRC(7acebd83) SHA1(64be95186d62003b637fcdf45a9c0b7aab182116) )
	ROM_LOAD16_BYTE( "jalgp4.bin",   0x100001, 0x080000, CRC(6a6b72f3) SHA1(3ba359b1a89eb3f6664ed83d93f79d7f895d4222) )
	ROM_LOAD16_BYTE( "jalgp5.bin",   0x200000, 0x080000, CRC(65ab751d) SHA1(f2cb8701eb8c3567a1d03248e6918c5a7b5df939) )
	ROM_LOAD16_BYTE( "jalgp6.bin",   0x200001, 0x080000, CRC(24e3ab23) SHA1(d1431688e1518ba52935f6ab44b815975bec4c27) )
	ROM_LOAD16_BYTE( "jalgp7.bin",   0x300000, 0x080000, CRC(44b71098) SHA1(a6ec2573f9a266d4f8f315f6e99b12525011f512) )
	ROM_LOAD16_BYTE( "jalgp8.bin",   0x300001, 0x080000, CRC(ce0b7838) SHA1(46e34971cb62565a3948d8c0a18086648c32e13b) )

	ROM_REGION( 0x100000, "oki1", 0 )	/* M6295 samples */
	ROM_LOAD( "jalcf2.bin",   0x000000, 0x100000, CRC(b982fd97) SHA1(35ee5b1b9be762ccfefda24d73e329ceea876deb) )

	ROM_REGION( 0x100000, "oki2", 0 )	/* M6295 samples */
	ROM_LOAD( "jalcf1.bin",   0x000000, 0x100000, CRC(24af21d3) SHA1(f68ab81a6c833b57ae9eef916a1c8578f3d893dd) )

	ROM_REGION( 0x100000, "user1", 0 ) /* ? these two below are identical*/
	ROM_LOAD( "jalmr14.bin",   0x000000, 0x080000, CRC(9d428fb7) SHA1(02f72938d73db932bd217620a175a05215f6016a) )
	ROM_LOAD( "jalmr17.bin",   0x080000, 0x080000, CRC(9d428fb7) SHA1(02f72938d73db932bd217620a175a05215f6016a) )
ROM_END

GAME( 1994, acommand,  0,       acommand,  acommand,  0, ROT0, "Jaleco", "Alien Command" , GAME_NOT_WORKING )
