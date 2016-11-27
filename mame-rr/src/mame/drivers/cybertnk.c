/*******************************************************************************************

Cyber Tank HW (c) 1987 Coreland Technology

preliminary driver by Angelo Salese

Maybe it has some correlation with WEC Le Mans HW? (supposely that was originally done by Coreland too)

TODO:
- sprite emulation (very likely that it can do zooming);
- road emulation;
- tilemap scrolling /-color banking;
- inputs doesn't work in-game?
- dual screen output.

============================================================================================

(note: the following notes are from "PS")
- Communications
Master slave comms looks like shared RAM.
The slave tests these RAM banks all using the same routine at 0x000006E6
There are also routines for clearing each bank of RAM after POST,
including the shared RAM (0x).
The amount cleared in each bank is different than the amount tested.

IC20 (7D0*2 bytes starting @ 0x080000, code at 0x00000684) \ Tested as 0xFA0 (4000) bytes.
IC21 (7D0*2 bytes starting @ 0x080001, code at 0x00000690) / Cleared at 0x0000042a as 0x1000 bytes.
IC22 (1F4*2 bytes starting @ 0x0c0000, code at 0x0000069E) \ Tested as 0x3e8 (1000) bytes.
IC23 (1F4*2 bytes starting @ 0x0c0001, code at 0x000006AA) / Cleared at 0x00000440 as 0x4000 bytes.
IC24 (1F4*2 bytes starting @ 0x100001, code at 0x000006B8) > Shared RAM

The shared ram is tested and cleared almost the same as the other RAM,
and is mapped into the master at E0000. Only every odd byte is used.

The first 0x10 bytes are used heavily as registers
during the POST, key ones are
    share_0001 hold - slave waits for this to be cleared.
    share_0009 master cmd to slave
    share_0011 slave status
    share_000b bit mask of failed ICs returned to master, defaults to all failed.

There are also block writes carried out by the slave every
second irq 3. The data transfer area appears to start at 0x100021.
Master reads at 0x00005E3C

It is tested as every odd byte from 0x100021 to 0x1003e8,
and cleared as every odd byte from 0x100021 up to 0x100fff)

- Missing ROM data
IC03 and IC04 are missing from the set.
These look like "logical" ICs rather that physical ones.
The main program code is dumped as 2x128k files p1a and p2a.
The checksum routine does 4x64k checks, going by the routine.
IC01 is the first half of p1a, IC02 is the first half of p2a,
IC03 is the second half of p1a and IC04 is the second half of p2a.

IC01 FEFF bytes @ 0x00200 code at 0x000019FE CHKSUM A5 (00200 offset is just after where the checksum is recorded)
IC02 FEFF bytes @ 0x00201 code at 0x00001A24 CHKSUM 87
IC03 FFFF bytes @ 0x20000 code at 0x00001A46 CHKSUM 61
IC04 FFFF bytes @ 0x20001 code at 0x00001A6A CHKSUM E0

Unfortunately the second half of p1a and p2a are empty, all 0xff.
No other ROMs in the entire set match the checksums used in the POST.
I am guessing they are bad dumps.

- Unmapped reads/writes
CPU1 reads at 0x07fff8 and 0x07fffa are the slave reading validation values
to compare to slave program ROM checksums.
The test will never fail, the results of the comparison are ignored by the code,
so there may never have been an implementation.

CPU1 unmapped read at 0x20000 is a checksum overrun by a single loop iteration.
See loop at 0x000006D2, it's a "do while" loop that tests loop after testing ROM.

Unmapped read/write by CPU2 of 0xa005, 0xa006 This looks like loop overrun too,
or maybe caused by the initial base offset which is the same as the loop increments it.
Sub at CPU2:01B7, the block process starts at base 8020h and increments by 20h each time.
It overruns the top of RAM on the last iteration.

============================================================================================
Cyber Tank
Coreland Technology, Inc 1987

---------------------
BA87015

   SS2               -
   SS4               SS3
   -                 -
   -                 SS1

        Y8950             Y8950

    2064
    SS5

    Z80B
    3.5795MHz

---------------------
BA87035

        68000-10      20MHz    68000-10
        SUBH  SUBL             P2A  P1A
        2064  2064             2064 2064


 C04  C03  C02  C01
 C08  C07  C06  C05
 C12  C11  C10  C09
 C16  C15  C14  C13                        2016
                                           2016
                                        W31003
               SW1 SW2 SW3

---------------------
BA87034

  T2   T1                               43256   43256
              IC19  IC20                43256   43256
              IC29  IC30
                                        43256   43256
                                        43256   43256

                                        43256   43256
  T3                                    43256   43256
  T4
                                        43256   43256
                                        43256   43256



                                                  W31004
                                                  W31004
----------------------
BA87033

     22.8MHz    IC2                       2016     W31001
                           IC15           2016     W31004
                                          ROAD_CHL
                                          ROAD_CHH


                        S01
      2064              S02
      2064              S03                    T6          T5
    W31002       2064   S04      W31004

                        S05
                        S06
      2064              S07                    2064        2064
      2064       2064   S08      W31004        2064        2064
    W31002
                        S09
      2064              S10
      2064              S11
    W31002       2064   S12      W31004
                                                VID1CONN  VID2CONN
********************************************************************************************
M68k Master irq table:
lev 1 : 0x64 : 0000 0870 - vblank
lev 2 : 0x68 : 0000 0caa - input device clear?
lev 3 : 0x6c : 0000 0caa - input device clear?
lev 4 : 0x70 : 0000 0caa - input device clear?
lev 5 : 0x74 : 0000 0caa - input device clear?
lev 6 : 0x78 : 0000 0caa - input device clear?
lev 7 : 0x7c : ffff ffff - illegal

M68k Slave irq table:
lev 1 : 0x64 : 0000 07e0 - input device clear?
lev 2 : 0x68 : 0000 07e0 - input device clear?
lev 3 : 0x6c : 0000 0764 - vblank?
lev 4 : 0x70 : 0000 07e0 - input device clear?
lev 5 : 0x74 : 0000 07e0 - input device clear?
lev 6 : 0x78 : 0000 07e0 - input device clear?
lev 7 : 0x7c : 0000 07e0 - input device clear?

*******************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "deprecat.h"
#include "sound/8950intf.h"

static tilemap_t *tx_tilemap;
static UINT16 *tx_vram,*bg_vram,*fg_vram,*spr_ram;
static UINT16 *io_ram;

#define LOG_UNKNOWN_WRITE logerror("unknown io write CPU '%s':%08x  0x%08x 0x%04x & 0x%04x\n", space->cpu->tag(), cpu_get_pc(space->cpu), offset*2, data, mem_mask);

static TILE_GET_INFO( get_tx_tile_info )
{
	int code = tx_vram[tile_index];
	SET_TILE_INFO(
			0,
			code & 0x1fff,
			(code & 0xe000) >> 13,
			0);
}

static VIDEO_START( cybertnk )
{
	tx_tilemap = tilemap_create(machine, get_tx_tile_info,tilemap_scan_rows,8,8,128,32);
	tilemap_set_transparent_pen(tx_tilemap,0);
}

static VIDEO_UPDATE( cybertnk )
{
	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));

	{
		int count,x,y;
		const gfx_element *gfx = screen->machine->gfx[2];

		count = 0;

		for (y=0;y<32;y++)
		{
			for (x=0;x<128;x++)
			{
				UINT16 tile = bg_vram[count] & 0x1fff;
				UINT16 color = (fg_vram[count] & 0xe000) >> 13;

				drawgfx_transpen(bitmap,cliprect,gfx,tile,color+0x194,0,0,x*8,(y*8),0);

				count++;

			}
		}
	}

	{
		int count,x,y;
		const gfx_element *gfx = screen->machine->gfx[1];

		count = 0;

		for (y=0;y<32;y++)
		{
			for (x=0;x<128;x++)
			{
				UINT16 tile = fg_vram[count] & 0x1fff;
				UINT16 color = (fg_vram[count] & 0xe000) >> 13;

				drawgfx_transpen(bitmap,cliprect,gfx,tile,color+0x1c0,0,0,x*8,(y*8),0);

				count++;

			}
		}
	}

	/* non-tile based spriteram (BARE-BONES, looks pretty complex) */
	if(1)
	{
		const UINT8 *blit_ram = memory_region(screen->machine,"spr_gfx");
		int offs,x,y,z,xsize,ysize,yi,xi,col_bank,fx,zoom;
		UINT32 spr_offs,spr_offs_helper;
		int xf,yf,xz,yz;

		for(offs=0;offs<0x1000/2;offs+=8)
		{
			z = (spr_ram[offs+(0x6/2)] & 0xffff);
			if(z == 0xffff || spr_ram[offs+(0x0/2)] == 0x0000) //TODO: check the correct bit
				continue;
			x = (spr_ram[offs+(0xa/2)] & 0x3ff);
			y = (spr_ram[offs+(0x4/2)] & 0xff);
			if(spr_ram[offs+(0x4/2)] & 0x100)
				y = 0x100 - y;
			spr_offs = (((spr_ram[offs+(0x0/2)] & 7) << 16) | (spr_ram[offs+(0x2/2)])) << 2;
			xsize = ((spr_ram[offs+(0xc/2)] & 0x000f)+1) << 3;
			ysize = (spr_ram[offs+(0x8/2)] & 0x00ff)+1;
			fx = (spr_ram[offs+(0xa/2)] & 0x8000) >> 15;
			zoom = (spr_ram[offs+(0xc/2)] & 0xff00) >> 8;

			col_bank = (spr_ram[offs+(0x0/2)] & 0xff00) >> 8;

			xf = 0;
			yf = 0;
			xz = 0;
			yz = 0;

			for(yi = 0;yi < ysize;yi++)
			{
				xf = xz = 0;
				spr_offs_helper = spr_offs;
				for(xi=0;xi < xsize;xi+=8)
				{
					UINT32 color;
					UINT16 dot;
					int shift_pen, x_dec; //helpers

					color = ((blit_ram[spr_offs+0] & 0xff) << 24);
					color|= ((blit_ram[spr_offs+1] & 0xff) << 16);
					color|= ((blit_ram[spr_offs+2] & 0xff) << 8);
					color|= ((blit_ram[spr_offs+3] & 0xff) << 0);

					shift_pen = 28;
					x_dec = 0;

					while(x_dec < 4 && x_dec+xi <= xsize)
					{
						dot = (color >> shift_pen) & 0xf;
						if(dot != 0) // transparent pen
						{
							dot|= col_bank<<4;
							if(fx)
							{
								if(((x+xsize-(xz)) < screen->visible_area().max_x) && ((y+yi) < screen->visible_area().max_y))
									*BITMAP_ADDR16(bitmap, y+yz, x+xsize-(xz)) = screen->machine->pens[dot];
							}
							else
							{
								if(((x+xz) < screen->visible_area().max_x) && ((y+yi) < screen->visible_area().max_y))
									*BITMAP_ADDR16(bitmap, y+yz, x+xz) = screen->machine->pens[dot];
							}
						}
						xf+=zoom;
						if(xf >= 0x100)
						{
							xz++;
							xf-=0x100;
						}
						else
						{
							shift_pen -= 8;
							x_dec++;
							if(xf >= 0x80) { xz++; xf-=0x80; }
						}
					}

					shift_pen = 24;
					x_dec = 4;

					while(x_dec < 8 && x_dec+xi <= xsize)
					{
						dot = (color >> shift_pen) & 0xf;
						if(dot != 0) // transparent pen
						{
							dot|= col_bank<<4;
							if(fx)
							{
								if(((x+xsize-(xz)) < screen->visible_area().max_x) && ((y+yi) < screen->visible_area().max_y))
									*BITMAP_ADDR16(bitmap, y+yz, x+xsize-(xz)) = screen->machine->pens[dot];
							}
							else
							{
								if(((x+x_dec+xi) < screen->visible_area().max_x) && ((y+yi) < screen->visible_area().max_y))
									*BITMAP_ADDR16(bitmap, y+yz, x+xz) = screen->machine->pens[dot];
							}
						}
						xf+=zoom;
						if(xf >= 0x100)
						{
							xz++;
							xf-=0x100;
						}
						else
						{
							shift_pen -= 8;
							x_dec++;
							if(xf >= 0x80) { xz++; xf-=0x80; }
						}
					}

					spr_offs+=4;
				}
				yf+=zoom;
				if(yf >= 0x100)
				{
					yi--;
					yz++;
					spr_offs = spr_offs_helper;
					yf-=0x100;
				}
				if(yf >= 0x80) { yz++; yf-=0x80; }
			}
		}
	}

	tilemap_draw(bitmap,cliprect,tx_tilemap,0,0);


//0x62 0x9a 1c2d0
//0x62 0x9a 1e1e4
//0x20 0x9c 2011c

if(0) //sprite gfx debug viewer
{
	int x,y,count;
	static int test_x, test_y, start_offs,color_pen;
	const UINT8 *blit_ram = memory_region(screen->machine,"spr_gfx");

	if(input_code_pressed(screen->machine, KEYCODE_Z))
		test_x++;

	if(input_code_pressed(screen->machine, KEYCODE_X))
		test_x--;

	if(input_code_pressed(screen->machine, KEYCODE_A))
		test_y++;

	if(input_code_pressed(screen->machine, KEYCODE_S))
		test_y--;

	if(input_code_pressed(screen->machine, KEYCODE_Q))
		start_offs+=0x200;

	if(input_code_pressed(screen->machine, KEYCODE_W))
		start_offs-=0x200;

	if(input_code_pressed_once(screen->machine, KEYCODE_T))
		start_offs+=0x20000;

	if(input_code_pressed_once(screen->machine, KEYCODE_Y))
		start_offs-=0x20000;

	if(input_code_pressed(screen->machine, KEYCODE_E))
		start_offs+=4;

	if(input_code_pressed(screen->machine, KEYCODE_R))
		start_offs-=4;

	if(input_code_pressed(screen->machine, KEYCODE_D))
		color_pen++;

	if(input_code_pressed(screen->machine, KEYCODE_F))
		color_pen--;

	popmessage("%02x %02x %04x %02x",test_x,test_y,start_offs,color_pen);

	bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine));

	count = (start_offs);

	for(y=0;y<test_y;y++)
	{
		for(x=0;x<test_x;x+=8)
		{
			UINT32 color;
			UINT8 dot;

			color = ((blit_ram[count+0] & 0xff) << 24);
			color|= ((blit_ram[count+1] & 0xff) << 16);
			color|= ((blit_ram[count+2] & 0xff) << 8);
			color|= ((blit_ram[count+3] & 0xff) << 0);

			dot = (color & 0xf0000000) >> 28;
			*BITMAP_ADDR16(bitmap, y, x+0) = screen->machine->pens[dot+(color_pen<<4)];

			dot = (color & 0x0f000000) >> 24;
			*BITMAP_ADDR16(bitmap, y, x+4) = screen->machine->pens[dot+(color_pen<<4)];

			dot = (color & 0x00f00000) >> 20;
			*BITMAP_ADDR16(bitmap, y, x+1) = screen->machine->pens[dot+(color_pen<<4)];

			dot = (color & 0x000f0000) >> 16;
			*BITMAP_ADDR16(bitmap, y, x+5) = screen->machine->pens[dot+(color_pen<<4)];

			dot = (color & 0x0000f000) >> 12;
			*BITMAP_ADDR16(bitmap, y, x+2) = screen->machine->pens[dot+(color_pen<<4)];

			dot = (color & 0x00000f00) >> 8;
			*BITMAP_ADDR16(bitmap, y, x+6) = screen->machine->pens[dot+(color_pen<<4)];

			dot = (color & 0x000000f0) >> 4;
			*BITMAP_ADDR16(bitmap, y, x+3) = screen->machine->pens[dot+(color_pen<<4)];

			dot = (color & 0x0000000f) >> 0;
			*BITMAP_ADDR16(bitmap, y, x+7) = screen->machine->pens[dot+(color_pen<<4)];

			count+=4;
		}
	}
}

	return 0;
}

static DRIVER_INIT( cybertnk )
{

}

static WRITE16_HANDLER( tx_vram_w )
{
	COMBINE_DATA(&tx_vram[offset]);
	tilemap_mark_tile_dirty(tx_tilemap,offset);
}

static READ16_HANDLER( io_r )
{
	switch( offset )
	{
		case 2/2:
			return input_port_read(space->machine, "DSW1");

		// 0x00110007 is controller device select
		// 0x001100D5 is controller data
		// 0x00110004 low is controller data ready
		case 4/2:
			switch( (io_ram[6/2]) & 0xff )
			{
				case 0:
					io_ram[0xd4/2] = input_port_read(space->machine, "TRAVERSE");
					break;

				case 0x20:
					io_ram[0xd4/2] = input_port_read(space->machine, "ELEVATE");
					break;

				case 0x40:
					io_ram[0xd4/2] = input_port_read(space->machine, "ACCEL");
					break;

				case 0x42:
					// only once I think, during init at 0x00000410
					// controller return value is stored in $42(a6)
					// but I don't see it referenced again.
					//popmessage("unknown controller device 0x42");
					io_ram[0xd4/2] = 0;
					break;

				case 0x60:
					io_ram[0xd4/2] = input_port_read(space->machine, "HANDLE");
					break;

				//default:
					//popmessage("unknown controller device");
			}
			return 0;

		case 6/2:
			return input_port_read(space->machine, "IN0"); // high half

		case 8/2:
			return input_port_read(space->machine, "IN0"); // low half

		case 0xa/2:
			return input_port_read(space->machine, "DSW2");

		case 0xd4/2:
			return io_ram[offset]; // controller data

		default:
		{
			//popmessage("unknown io read 0x%08x", offset);
			return io_ram[offset];
		}
	}
}

static WRITE16_HANDLER( io_w )
{
	COMBINE_DATA(&io_ram[offset]);

	switch( offset )
	{
		case 0/2:
			// sound data
			if (ACCESSING_BITS_0_7)
				cputag_set_input_line(space->machine, "audiocpu", 0, HOLD_LINE);
			else
				LOG_UNKNOWN_WRITE
			break;

		case 2/2:
			if (ACCESSING_BITS_0_7)
				;//watchdog ? written in similar context to CPU1 @ 0x140002
			else
				LOG_UNKNOWN_WRITE
			break;

		case 6/2:
			if (ACCESSING_BITS_0_7)
				;//select controller device
			else
				;//blank inputs
			break;

		case 8/2:
			if (ACCESSING_BITS_8_15)
				;//blank inputs
			else
				LOG_UNKNOWN_WRITE
			break;

		case 0xc/2:
			//if (ACCESSING_BITS_0_7)
				// This seems to only be written after each irq1 and irq2, irq ack?
				//logerror("irq wrote %04x\n", data);
			//else
			//  LOG_UNKNOWN_WRITE
			break;

		case 0xd4/2:
			if ( ACCESSING_BITS_0_7 )
				;// controller device data
			else
				LOG_UNKNOWN_WRITE
			break;

		// Cabinet pictures show dials and gauges
		// Maybe this is for lamps and stuff, or
		// maybe just debug.
		// They are all written in a block at 0x00000944
		case 0x40/2:
		case 0x42/2:
		case 0x44/2:
		case 0x48/2:
		case 0x4a/2:
		case 0x4c/2:
		case 0x80/2:
		case 0x82/2:
		case 0x84/2:
			popmessage("%02x %02x %02x %02x %02x %02x %02x",io_ram[0x40/2],io_ram[0x42/2],io_ram[0x44/2],io_ram[0x46/2],io_ram[0x48/2],io_ram[0x4a/2],io_ram[0x4c/2]);
			break;

		default:
			LOG_UNKNOWN_WRITE
			break;
	}
}

static READ8_HANDLER( soundport_r )
{
	return io_ram[0] & 0xff;
}

static ADDRESS_MAP_START( master_mem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x080000, 0x087fff) AM_RAM /*Work RAM*/
	AM_RANGE(0x0a0000, 0x0a0fff) AM_RAM AM_BASE(&spr_ram) // non-tile based sprite ram
	AM_RANGE(0x0c0000, 0x0c1fff) AM_RAM_WRITE(tx_vram_w) AM_BASE(&tx_vram)
	AM_RANGE(0x0c4000, 0x0c5fff) AM_RAM AM_BASE(&bg_vram)
	AM_RANGE(0x0c8000, 0x0c9fff) AM_RAM AM_BASE(&fg_vram)
	AM_RANGE(0x0e0000, 0x0e0fff) AM_RAM AM_SHARE("sharedram")
	AM_RANGE(0x100000, 0x107fff) AM_RAM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x110000, 0x1101ff) AM_READWRITE(io_r,io_w) AM_BASE(&io_ram)
ADDRESS_MAP_END

static ADDRESS_MAP_START( slave_mem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x01ffff) AM_ROM
	AM_RANGE(0x080000, 0x083fff) AM_RAM /*Work RAM*/
	AM_RANGE(0x0c0000, 0x0c0fff) AM_RAM
	AM_RANGE(0x100000, 0x100fff) AM_RAM AM_SHARE("sharedram")
	AM_RANGE(0x140000, 0x140003) AM_NOP /*Watchdog? Written during loops and interrupts*/
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_mem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff ) AM_ROM
	AM_RANGE(0x8000, 0x9fff ) AM_RAM
	AM_RANGE(0xa001, 0xa001 ) AM_READ(soundport_r)
	AM_RANGE(0xa005, 0xa006 ) AM_NOP
	AM_RANGE(0xa000, 0xa001 ) AM_DEVREADWRITE("ym1", y8950_r, y8950_w)
	AM_RANGE(0xc000, 0xc001 ) AM_DEVREADWRITE("ym2", y8950_r, y8950_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( cybertnk )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) // MG 1
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) // Cannon 1
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) // MG 2
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) // Cannon 2
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0800, IP_ACTIVE_LOW )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("TRAVERSE")
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_X ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_PLAYER(2)

	PORT_START("ELEVATE")
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_Y ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_PLAYER(2)

	PORT_START("ACCEL")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_PLAYER(1)

	PORT_START("HANDLE")
	PORT_BIT( 0xff, 0x7f, IPT_PADDLE ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(100) PORT_KEYDELTA(4) PORT_PLAYER(1)


	PORT_START("DSW1")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Test ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_BIT(	  0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static const gfx_layout tile_8x8x4 =
{
	8,8,
	RGN_FRAC(1,4),
    4,
    { RGN_FRAC(3,4),RGN_FRAC(1,4),RGN_FRAC(2,4),RGN_FRAC(0,4) },
    { STEP8(0,1) },
    { STEP8(0,8) },
    8*8
};

static GFXDECODE_START( cybertnk )
	GFXDECODE_ENTRY( "gfx1", 0, tile_8x8x4,     0x1400, 16 ) /*Pal offset???*/
	GFXDECODE_ENTRY( "gfx2", 0, tile_8x8x4,     0,      0x400 )
	GFXDECODE_ENTRY( "gfx3", 0, tile_8x8x4,     0,      0x400 )
GFXDECODE_END

#if 0
static INTERRUPT_GEN( master_irq )
{
	switch(cpu_getiloops(device))
	{
		case 0: cpu_set_input_line(device,1,HOLD_LINE); break;
		case 1: cpu_set_input_line(device,3,HOLD_LINE); break;
	}
}

static INTERRUPT_GEN( slave_irq )
{
	switch(cpu_getiloops(device))
	{
		case 0: cpu_set_input_line(device,3,HOLD_LINE); break;
		case 1: cpu_set_input_line(device,1,HOLD_LINE); break;
	}
}
#endif

static const y8950_interface y8950_config = {
	0 /* TODO */
};

static MACHINE_DRIVER_START( cybertnk )
	MDRV_CPU_ADD("maincpu", M68000,20000000/2)
	MDRV_CPU_PROGRAM_MAP(master_mem)
//  MDRV_CPU_VBLANK_INT_HACK(master_irq,2)
	MDRV_CPU_VBLANK_INT("screen", irq1_line_hold)

	MDRV_CPU_ADD("slave", M68000,20000000/2)
	MDRV_CPU_PROGRAM_MAP(slave_mem)
//  MDRV_CPU_VBLANK_INT_HACK(slave_irq,2)
	MDRV_CPU_VBLANK_INT("screen", irq3_line_hold)

	MDRV_CPU_ADD("audiocpu", Z80,3579500)
	MDRV_CPU_PROGRAM_MAP(sound_mem)

	MDRV_QUANTUM_TIME(HZ(6000))//arbitrary value,needed to get the communication to work

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 32*8-1)

	MDRV_GFXDECODE(cybertnk)
	MDRV_PALETTE_LENGTH(0x4000)

	MDRV_VIDEO_START(cybertnk)
	MDRV_VIDEO_UPDATE(cybertnk)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ym1", Y8950, 3579500)
	MDRV_SOUND_CONFIG(y8950_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)

	MDRV_SOUND_ADD("ym2", Y8950, 3579500)
	MDRV_SOUND_CONFIG(y8950_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
MACHINE_DRIVER_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( cybertnk )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "p1a.37",   0x00000, 0x20000, CRC(be1abd16) SHA1(6ad01516301b44899971000c36f7e21070c3d2da) )
	ROM_LOAD16_BYTE( "p2a.36",   0x00001, 0x20000, CRC(5290c89a) SHA1(5a11671505214c20770e2938dab1ee82a030b457) )

	ROM_REGION( 0x20000, "slave", 0 )
	ROM_LOAD16_BYTE( "subl",   0x00000, 0x10000, CRC(3814a2eb) SHA1(252800b21f5cfada34ef5208cda33088daab132b) )
	ROM_LOAD16_BYTE( "subh",   0x00001, 0x10000, CRC(1af7ad58) SHA1(450c65289729d74cd4d17e11be16469246e61b7d) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "ss5.37",    0x0000, 0x8000, CRC(c3ba160b) SHA1(cfbfcad443ff83cd4e707f045a650417aca03d85) )

	ROM_REGION( 0x40000, "ym1", ROMREGION_ERASEFF )
	ROM_LOAD( "ss1.10",    0x00000, 0x20000, CRC(27d1cf94) SHA1(26246f217192bcfa39692df6d388640d385e9ed9) )
	ROM_LOAD( "ss3.11",    0x20000, 0x20000, CRC(a327488e) SHA1(b55357101e392f50f0cf75cf496a3ff4b79b2633) )

	ROM_REGION( 0x80000, "ym2", ROMREGION_ERASEFF )
	ROM_LOAD( "ss2.31",    0x00000, 0x20000, CRC(27d1cf94) SHA1(26246f217192bcfa39692df6d388640d385e9ed9) )
	ROM_LOAD( "ss4.32",    0x20000, 0x20000, CRC(a327488e) SHA1(b55357101e392f50f0cf75cf496a3ff4b79b2633) )

	ROM_REGION( 0x40000, "gfx1", 0 )
	ROM_LOAD( "s09", 0x00000, 0x10000, CRC(69e6470c) SHA1(8e7db6988366cae714fff72449623a7977af1db1) )
	ROM_LOAD( "s10", 0x10000, 0x10000, CRC(77230f44) SHA1(b79fc841fa784d23855e4085310cee435c11348f) )
	ROM_LOAD( "s11", 0x20000, 0x10000, CRC(bfda980d) SHA1(1f975fdd2cfdc345eeb03fbc26fc1be1b2d7737e) )
	ROM_LOAD( "s12", 0x30000, 0x10000, CRC(8a11fcfa) SHA1(a406ac9cf841dd9d829cb83bfe8feb5128a3e77e) )

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "s01", 0x00000, 0x10000, CRC(6513452c) SHA1(95ed2da8f90e16c50716011577606a7dc93ba65e) )
	ROM_LOAD( "s02", 0x10000, 0x10000, CRC(3a270e3b) SHA1(97c8282d4d782c9d2fcfb5e5dabbe1ca88978f5c) )
	ROM_LOAD( "s03", 0x20000, 0x10000, CRC(584eff66) SHA1(308ec058693ce3ce34b058a8dbeedf342134311c) )
	ROM_LOAD( "s04", 0x30000, 0x10000, CRC(51ba5402) SHA1(c4522c4562ce0514bef3257e323bcc255b635544) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "s05", 0x00000, 0x10000, CRC(bddb6008) SHA1(bacb822bac4893eee0648a19ce449e5559d32b5e) )
	ROM_LOAD( "s06", 0x10000, 0x10000, CRC(d65b0fa5) SHA1(ce398a52ad408778fd910c42a9618194b862becf) )
	ROM_LOAD( "s07", 0x20000, 0x10000, CRC(70220567) SHA1(44b48ded8581a6d78b27a3af833f62413ff31c76) )
	ROM_LOAD( "s08", 0x30000, 0x10000, CRC(988c4fcb) SHA1(68d32be70605ad5415f2b6aeabbd92e269f0c9af) )

	/* TODO: fix the rom loading accordingly*/
	ROM_REGION( 0x200000, "spr_gfx", 0 )
	ROM_LOAD32_BYTE( "c01.93" , 0x180001, 0x20000, CRC(b5ee3de2) SHA1(77b9a2818f36826891e510e8550f1025bacfa496) )
	ROM_LOAD32_BYTE( "c02.92" , 0x180000, 0x20000, CRC(1f857d79) SHA1(f410d50970c10814b80baab27cbe69965bf0ccc0) )
	ROM_LOAD32_BYTE( "c03.91" , 0x180003, 0x20000, CRC(d70a93e2) SHA1(e64bb10c58b27def4882f3006784be56de11b812) )
	ROM_LOAD32_BYTE( "c04.90" , 0x180002, 0x20000, CRC(04d6fdc2) SHA1(56f8091c1a010014e951f5f47084e1400006123e) )
	ROM_LOAD32_BYTE( "c05.102", 0x100001, 0x20000, CRC(3f537490) SHA1(12d6545d29dda9f88019040fa33c73a22a2a213b) )
	ROM_LOAD32_BYTE( "c06.101", 0x100000, 0x20000, CRC(ff69c6a4) SHA1(badd20d26ba771780aebf733e1fbd1d37aa66f9b) )
	ROM_LOAD32_BYTE( "c07.100", 0x100003, 0x20000, CRC(5e8eba75) SHA1(6d0c1916517802acf808c8edc8e0b6074bdc90be) )
	ROM_LOAD32_BYTE( "c08.98" , 0x100002, 0x20000, CRC(f0820ddd) SHA1(7fb6c7d66ff96148f14921bc8d0cc0c65ffce4c4) )
	ROM_LOAD32_BYTE( "c09.109", 0x080001, 0x20000, CRC(080f87c3) SHA1(aedebc22ff03d4cc710e71ca14e09c7808f59c72) ) //correct
	ROM_LOAD32_BYTE( "c10.108", 0x080000, 0x20000, CRC(777c6a62) SHA1(4684d1c5d88b37ecb20002b7aa4814bf566e7d4b) )
	ROM_LOAD32_BYTE( "c11.107", 0x080003, 0x20000, CRC(330ca5a1) SHA1(4409da231a5abcec8c7d2d66eefdfd2019a322db) )
	ROM_LOAD32_BYTE( "c12.106", 0x080002, 0x20000, CRC(c1ec8e61) SHA1(09f2f4ddc100e5675c9bd82c200718fb0b69655e) )
	ROM_LOAD32_BYTE( "c13.119", 0x000001, 0x20000, CRC(4e22a7e0) SHA1(69cc7dd528b8af0c28b448285768a3ed079099ba) )
	ROM_LOAD32_BYTE( "c14.118", 0x000000, 0x20000, CRC(bdbd6232) SHA1(94b0741d5eced558723dda32a89aa2b747cdcbbd) )
	ROM_LOAD32_BYTE( "c15.117", 0x000003, 0x20000, CRC(f163d768) SHA1(e54e31a6f956f7de52b59bcdd0cd4ac1662b5664) )
	ROM_LOAD32_BYTE( "c16.116", 0x000002, 0x20000, CRC(5e5017c4) SHA1(586cd729630f00cbaf10d1036edebed1672bc532) )

	ROM_REGION( 0x200000, "road_data", 0 )
	ROM_LOAD16_BYTE( "road_chl" , 0x000000, 0x20000, CRC(862b109c) SHA1(9f81918362218ddc0a6bf0a5317c5150e514b699) )
	ROM_LOAD16_BYTE( "road_chh" , 0x000001, 0x20000, CRC(9dedc988) SHA1(10bae1be0e35320872d4994f7e882cd1de988c90) )

	/*The following ROM regions aren't checked yet*/
	ROM_REGION( 0x30000, "user3", 0 )
	ROM_LOAD( "t1",   0x00000, 0x08000, CRC(24890512) SHA1(2a6c9d39ca0c1c8316e85d9f565f6b3922d596b2) )
	ROM_LOAD( "t2",   0x08000, 0x08000, CRC(5a10480d) SHA1(f17598442091dae14abe3505957d94793f3ed886))
	ROM_LOAD( "t3",   0x10000, 0x08000, CRC(454af4dc) SHA1(e5b18a37715e50db2243432564f5a04fb39dea60) )
	ROM_LOAD( "t4",   0x18000, 0x08000, CRC(0e1ef6a9) SHA1(d230841bbee6d07bab05aa8d37ec2409fc6278bc) )
	/*The following two are identical*/
	ROM_LOAD( "t5",   0x20000, 0x08000, CRC(12eb51bc) SHA1(35708eb456207ebee498c70dd82340b364797c56) )
	ROM_LOAD( "t6",   0x28000, 0x08000, CRC(12eb51bc) SHA1(35708eb456207ebee498c70dd82340b364797c56) )

	ROM_REGION( 0x280, "proms", 0 )
	ROM_LOAD( "ic2",  0x0000, 0x0100, CRC(aad2a447) SHA1(a12923027e3093bd6d358af44d35d2e8e588dd1a) )//road proms related?
	ROM_LOAD( "ic15", 0x0100, 0x0100, CRC(5f8c2c00) SHA1(50162503ac0ee9395377d7e45a84672a9493fb7d) )
	ROM_LOAD( "ic19", 0x0200, 0x0020, CRC(bd15cd71) SHA1(e0946d12eebd5db8707d965be157914d70f7472b) )//T1-T6 proms related?
	ROM_LOAD( "ic20", 0x0220, 0x0020, CRC(2f237563) SHA1(b0081c1cc6e357a6f10ab1ff357bd4e989ec7fb3) )
	ROM_LOAD( "ic29", 0x0240, 0x0020, CRC(95b32c0f) SHA1(5a19f441ced983bacbf3bc1aaee94ca768166447) )
	ROM_LOAD( "ic30", 0x0260, 0x0020, CRC(2bb6033f) SHA1(eb994108734d7d04f8e293eca21bb3051a63cfe9) )
ROM_END

GAME( 1988, cybertnk,  0,       cybertnk,  cybertnk,  cybertnk, ROT0, "Coreland", "Cyber Tank (v1.04)", GAME_NOT_WORKING )
