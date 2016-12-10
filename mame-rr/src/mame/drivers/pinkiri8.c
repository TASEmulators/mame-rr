/***************************************************************************

Pinkiri 8 skeleton driver

- current blocker is the video emulation i/o ports, it looks somewhat exotic.

============================================================================
Janshi
(c)1992 Eagle

CPU: HD647180X0P6 (16K EPROM internal rom)
Sound: AY-3-8910, M6295
Others: Battery
OSC: 32MHz, 21MHz & 12.2880MHz

ROMs:
1.1A         [92b140a5]
2.1B         [6de7e086]
3.1D         [4e94d8f2]
4.1F         [a5f6e3ef]
5.1H         [ff2cc769]
6.1K         [8197034d]
11.1L        [a7692ddf]



--- Team Japump!!! ---
Dumped by Chackn
04/May/2007

***************************************************************************/

#include "emu.h"
#include "cpu/z180/z180.h"
#include "sound/okim6295.h"
#include "video/generic.h"

static UINT8* janshi_vram1;
static UINT8* janshi_vram2;
static UINT8* janshi_back_vram;
static UINT8* janshi_crtc_regs;
static UINT8* janshi_unk1;
static UINT8* janshi_widthflags;
static UINT8* janshi_unk2;

static ADDRESS_MAP_START( janshi_vdp_map8, 0, 8 )

	AM_RANGE(0xfc0000, 0xfc1fff) AM_RAM AM_BASE(&janshi_back_vram) // bg tilemap?
	AM_RANGE(0xfc2000, 0xfc2fff) AM_RAM AM_BASE(&janshi_vram1) // xpos, colour, tile number etc.

	AM_RANGE(0xfc3700, 0xfc377f) AM_RAM AM_BASE(&janshi_unk1) // ?? height related?
	AM_RANGE(0xfc3780, 0xfc37bf) AM_RAM AM_BASE(&janshi_widthflags)
	AM_RANGE(0xfc37c0, 0xfc37ff) AM_RAM AM_BASE(&janshi_unk2) // 2x increasing tables 00 10 20 30 etc.

	AM_RANGE(0xfc3800, 0xfc3fff) AM_RAM AM_BASE(&janshi_vram2) // y pos + unknown

	AM_RANGE(0xff0000, 0xff07ff) AM_RAM_WRITE(paletteram_xBBBBBGGGGGRRRRR_split1_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0xff2000, 0xff27ff) AM_RAM_WRITE(paletteram_xBBBBBGGGGGRRRRR_split2_w) AM_BASE_GENERIC(paletteram2)

	AM_RANGE(0xff6000, 0xff601f) AM_RAM AM_BASE(&janshi_crtc_regs)
ADDRESS_MAP_END



/* VDP device to give us our own memory map */

class janshi_vdp_device_config : public device_config,
								 public device_config_memory_interface
{
	friend class janshi_vdp_device;
	janshi_vdp_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
public:
	static device_config *static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock);
	virtual device_t *alloc_device(running_machine &machine) const;
protected:
	virtual void device_config_complete();
	virtual bool device_validity_check(const game_driver &driver) const;
	virtual const address_space_config *memory_space_config(int spacenum = 0) const;
	address_space_config		m_space_config;
};

class janshi_vdp_device : public device_t,
						  public device_memory_interface
{
	friend class janshi_vdp_device_config;
	janshi_vdp_device(running_machine &_machine, const janshi_vdp_device_config &config);
public:
protected:
	virtual void device_start();
	virtual void device_reset();
	const janshi_vdp_device_config &m_config;
};

const device_type JANSHIVDP = janshi_vdp_device_config::static_alloc_device_config;

janshi_vdp_device_config::janshi_vdp_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
	: device_config(mconfig, static_alloc_device_config, "JANSHIVDP", tag, owner, clock),
	  device_config_memory_interface(mconfig, *this),
	  m_space_config("janshi_vdp", ENDIANNESS_LITTLE, 8,24, 0, NULL, *ADDRESS_MAP_NAME(janshi_vdp_map8))
{
}

device_config *janshi_vdp_device_config::static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
{
	return global_alloc(janshi_vdp_device_config(mconfig, tag, owner, clock));
}

device_t *janshi_vdp_device_config::alloc_device(running_machine &machine) const
{
	return auto_alloc(&machine, janshi_vdp_device(machine, *this));
}

void janshi_vdp_device_config::device_config_complete()
{
//  int address_bits = 24;

//  m_space_config = address_space_config("janshi_vdp", ENDIANNESS_BIG, 8,  address_bits, 0, *ADDRESS_MAP_NAME(janshi_vdp_map8));
}

bool janshi_vdp_device_config::device_validity_check(const game_driver &driver) const
{
	bool error = false;
	return error;
}

const address_space_config *janshi_vdp_device_config::memory_space_config(int spacenum) const
{
	return (spacenum == 0) ? &m_space_config : NULL;
}


janshi_vdp_device::janshi_vdp_device(running_machine &_machine, const janshi_vdp_device_config &config)
	: device_t(_machine, config),
	  device_memory_interface(_machine, config, *this),
	  m_config(config)
{
}

void janshi_vdp_device::device_start()
{
}

void janshi_vdp_device::device_reset()
{
}

/* end VDP device to give us our own memory map */




static UINT32 vram_addr;

static VIDEO_START( pinkiri8 )
{

}

/*

ronjan
00, 7d, 00, 40, 00, 42, 00, 44, 00, 46, 00, 48, 00, 4a, 00, 0c, 00, 4e, 00, 10, 00, 12, 00, 14, 00, 16, 00, 18, 00, 1a, 00, 1c,
00, 7d, 00, 40, 00, 42, 00, 44, 00, 46, 00, 48, 00, 4a, 00, 4c, 00, 4e, 00, 50, 00, 52, 00, 54, 00, 56, 00, 58, 00, 5a, 00, 5c,
00, 7d, 00, 40, 00, 42, 00, 44, 00, 46, 00, 48, 00, 4a, 00, 4c, 00, 4e, 00, 50, 00, 52, 00, 54, 00, 56, 00, 58, 00, 5a, 00, 5c,

00, 1e, 00, 20, 00, 22, 00, 24, 00, 26, 00, 68, 00, 6a, 00, 6c, 00, 6e, 00, 70, 00, 72, 00, 74, 00, 76, 00, 38, 00, 3a, 00, c0,
00, 5e, 00, 60, 00, 62, 00, 64, 00, 66, 00, 68, 00, 6a, 00, 6c, 00, 6e, 00, 70, 00, 72, 00, 74, 00, 76, 00, 78, 00, 7a, 00, c0,
00, 5e, 00, 60, 00, 62, 00, 64, 00, 66, 00, 68, 00, 6a, 00, 6c, 00, 6e, 00, 70, 00, 72, 00, 74, 00, 76, 00, 78, 00, 7a, 00, c0,


00, c2, 00, c4, 00, c6, 00, c8, 00, ca, 00, cc, 00, ce, 00, d0, 00, d2, 00, d4, 00, d6, 00, d8, 00, da, 00, dc, 00, de, 00, e0,
00, c2, 00, c4, 00, c6, 00, c8, 00, ca, 00, cc, 00, ce, 00, d0, 00, d2, 00, d4, 00, d6, 00, d8, 00, da, 00, dc, 00, de, 00, e0,
00, c2, 00, c4, 00, c6, 00, c8, 00, ca, 00, cc, 00, ce, 00, d0, 00, d2, 00, d4, 00, d6, 00, d8, 00, da, 00, dc, 00, de, 00, e0,

00, e3, 00, e5, 00, e7, 00, e9, 00, eb, 00, ed, 00, ef, 00, f1, 00, f3, 00, f5, 00, f7, 00, f9, 00, fb, 00, fd, 00, ff, 00, 7e,
00, e3, 00, e5, 00, e7, 00, e9, 00, eb, 00, ed, 00, ef, 00, f1, 00, f3, 00, f5, 00, f7, 00, f9, 00, fb, 00, fd, 00, ff, 00, 7e,
00, e3, 00, e5, 00, e7, 00, e9, 00, eb, 00, ed, 00, ef, 00, f1, 00, f3, 00, f5, 00, f7, 00, f9, 00, fb, 00, fd, 00, ff, 00, 7e,

00, 00, 00, 01, 00, 01, 00, 01, 00, 01, 00, 00, 00, 01, 00, 00, 00, 01, 00, 01, 00, 01, 00, 01, 00, 01, 00, 01, 00, 01, 00, 01,
00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 01, 00, 01, 00, 01, 00, 01, 00, 01, 00, 01,
00, 00, 00, 01, 00, 01, 00, 01, 00, 01, 00, 00, 00, 01, 00, 01, 00, 01, 00, 01, 00, 01, 00, 01, 00, 01, 00, 01, 00, 01, 00, 01,


00, 01, 00, 01, 00, 01, 00, 01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 01, 00, 01, 00, 00, 00, 00,
00, 01, 00, 01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
00, 01, 00, 01, 00, 01, 00, 01, 00, 01, 00, 01, 00, 01, 00, 01, 00, 01, 00, 01, 00, 01, 00, 01, 00, 01, 00, 00, 00, 00, 00, 00,



*/

static VIDEO_UPDATE( pinkiri8 )
{
	static int col_bank;
	const gfx_element *gfx = screen->machine->gfx[0];

	static int game_type_hack = 0;

	if (!strcmp(screen->machine->gamedrv->name,"janshi")) game_type_hack = 1;

	if ( input_code_pressed_once(screen->machine, KEYCODE_W) )
	{
		int i;
		int count2;
		printf("-------------------------------\n");
		count2=0;
		for (i=0x00;i<0x40;i+=2)
		{

			printf("%02x, ", janshi_widthflags[i+1]);

			count2++;

			if (count2==0x10)
			{
				printf("\n");
				count2 = 0;
			}


		}


	}



	//popmessage("%02x",janshi_crtc_regs[0x0a]);
	col_bank = (janshi_crtc_regs[0x0a] & 0x40) >> 6;

	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));

	/* FIXME: color is a bit of a mystery */
	{
		int x,y,col,tile,count,attr;

		count = 0;

		for(y=0;y<64;y++)
		{
			for(x=0;x<32;x++)
			{
				tile = janshi_back_vram[count+1]<<8 | janshi_back_vram[count+0];
				attr = janshi_back_vram[count+2] ^ 0xf0;
				col = (attr >> 4) | 0x10;

				drawgfx_transpen(bitmap,cliprect,gfx,tile,col,0,0,x*16,y*8,0);

				count+=4;
			}
		}
	}

	{
		int x,y,unk2;
		int col;

		int spr_offs,i;

		int width, height;



		for(i=(0x1000/4)-4;i>=0;i--)
		{

		/* vram 1 (video map 0xfc2000)

          tttt tttt | 00tt tttt | cccc c000 | xxxx xxxx |

          vram 2 (video map 0xfc3800)

          yyyy yyyy | ???? ???? |

        there is also some data at 13000 - 137ff
        and a table at 20000..

          */

			spr_offs = ((janshi_vram1[(i*4)+0] & 0xff) | (janshi_vram1[(i*4)+1]<<8)) & 0xffff;
			col = (janshi_vram1[(i*4)+2] & 0xf8) >> 3;
			x =   janshi_vram1[(i*4)+3];

			x &= 0xff;
			x *= 2;

			unk2 = janshi_vram2[(i*2)+1];
			y = (janshi_vram2[(i*2)+0]);

			y = 0x100-y;

			col|= col_bank<<5;

		//  width = 0; height = 0;

			width = 2;
			height = 2;


			// these bits seem to somehow determine the sprite height / widths for the sprite ram region?
			int bit = janshi_widthflags[(i/0x20)*2 + 1];

			if (bit)
			{
				//col = mame_rand(screen->machine);
				width = 2;
			}
			else
			{
				width = 1;
				height = 2;
			}

			// hacks!
			if (game_type_hack==1) // janshi
			{
				if (spr_offs<0x400)
				{
					height = 4;
				}
				else if (spr_offs<0x580)
				{
				//  height = 2;
				}
				else if (spr_offs<0x880)
				{
					height = 4;
				}
				else if (spr_offs<0x1000)
				{
				//  height = 2;
				}
				else if (spr_offs<0x1080)
				{
				//  height = 2;
				}
				else if (spr_offs<0x1700)
				{
					height = 4;
				}
				else if (spr_offs<0x1730)
				{
				//  height = 2;
				}
				else if (spr_offs<0x1930)
				{
					height = 4;
				}
				else if (spr_offs<0x19c0)
				{
					height = 1;
				}
				else
				{
					height = 4;
				}


			}






			if (height==1)
				y+=16;


			// hmm...
			if (height==2)
				y+=16;



			{
				int count = 0;


				for (int yy=0;yy<height;yy++)
				{
					for (int xx=0;xx<width;xx++)
					{
						drawgfx_transpen(bitmap,cliprect,gfx,spr_offs+count,col,0,0,x+xx*16,y+yy*8,0);
						count++;
					}
				}
			}
		}
	}

	return 0;
}

static ADDRESS_MAP_START( pinkiri8_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x00000, 0x0bfff) AM_ROM
	AM_RANGE(0x0c000, 0x0dfff) AM_RAM
	AM_RANGE(0x0e000, 0x0ffff) AM_ROM
	AM_RANGE(0x10000, 0x1ffff) AM_ROM
ADDRESS_MAP_END

static WRITE8_HANDLER( output_regs_w )
{
	if(data & 0x40)
		cputag_set_input_line(space->machine, "maincpu", INPUT_LINE_NMI, CLEAR_LINE);
	//data & 0x80 is probably NMI mask
}

static int prev_writes = 0;

#define LOG_VRAM 0

static WRITE8_HANDLER( pinkiri8_vram_w )
{
	switch(offset)
	{
		case 0:
			vram_addr = (data << 0)  | (vram_addr&0xffff00);
			if (LOG_VRAM) printf("\n prev writes was %04x\n\naddress set to %04x -\n", prev_writes, vram_addr );
			prev_writes = 0;
			break;

		case 1:
			vram_addr = (data << 8)  | (vram_addr & 0xff00ff);
			if (LOG_VRAM)printf("\naddress set to %04x\n", vram_addr);
			break;

		case 2:
			vram_addr = (data << 16) | (vram_addr & 0x00ffff);
			if (LOG_VRAM)printf("\naddress set to %04x\n", vram_addr);
			break;

		case 3:

			const address_space *vdp_space = space->machine->device<janshi_vdp_device>("janshivdp")->space();

			if (LOG_VRAM) printf("%02x ", data);
			prev_writes++;
			vram_addr++;

			memory_write_byte_8le(vdp_space, vram_addr, data);
			break;
	}
}

static UINT8 mux_data;

static WRITE8_HANDLER( mux_w )
{
	mux_data = data;
}

static READ8_HANDLER( mux_p2_r )
{
	switch(mux_data)
	{
		case 0x01: return input_port_read(space->machine, "PL2_01");
		case 0x02: return input_port_read(space->machine, "PL2_02");
		case 0x04: return input_port_read(space->machine, "PL2_03");
		case 0x08: return input_port_read(space->machine, "PL2_04");
		case 0x10: return input_port_read(space->machine, "PL2_05");
	}

	return 0xff;
}

static READ8_HANDLER( mux_p1_r )
{
	switch(mux_data)
	{
		case 0x01: return input_port_read(space->machine, "PL1_01");
		case 0x02: return input_port_read(space->machine, "PL1_02");
		case 0x04: return input_port_read(space->machine, "PL1_03");
		case 0x08: return input_port_read(space->machine, "PL1_04");
		case 0x10: return input_port_read(space->machine, "PL1_05");
	}

	return 0xff;
}

static ADDRESS_MAP_START( pinkiri8_io, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x3f) AM_RAM //Z180 internal I/O
	AM_RANGE(0x60, 0x60) AM_WRITE(output_regs_w)
	AM_RANGE(0x80, 0x83) AM_WRITE(pinkiri8_vram_w)

	AM_RANGE(0xa0, 0xa0) AM_DEVREADWRITE("oki", okim6295_r, okim6295_w ) //correct?
	AM_RANGE(0xb0, 0xb0) AM_WRITE(mux_w) //mux
	AM_RANGE(0xb0, 0xb0) AM_READ(mux_p2_r) // mux inputs
	AM_RANGE(0xb1, 0xb1) AM_READ(mux_p1_r) // mux inputs
	AM_RANGE(0xb2, 0xb2) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xf8, 0xf8) AM_READ_PORT("DSW1")
	AM_RANGE(0xf9, 0xf9) AM_READ_PORT("DSW2")
	AM_RANGE(0xfa, 0xfa) AM_READ_PORT("DSW3")
	AM_RANGE(0xfb, 0xfb) AM_READ_PORT("DSW4")

	/* Wing custom sound chip, same as Lucky Girl Z180 */
	AM_RANGE(0xc3, 0xc3) AM_WRITENOP
	AM_RANGE(0xc7, 0xc7) AM_WRITENOP
	AM_RANGE(0xcb, 0xcb) AM_WRITENOP
	AM_RANGE(0xcf, 0xcf) AM_WRITENOP

	AM_RANGE(0xd3, 0xd3) AM_WRITENOP
	AM_RANGE(0xd7, 0xd7) AM_WRITENOP
	AM_RANGE(0xdb, 0xdb) AM_WRITENOP
	AM_RANGE(0xdf, 0xdf) AM_WRITENOP

	AM_RANGE(0xe3, 0xe3) AM_WRITENOP
	AM_RANGE(0xe7, 0xe7) AM_WRITENOP
	AM_RANGE(0xeb, 0xeb) AM_WRITENOP
	AM_RANGE(0xef, 0xef) AM_WRITENOP

	AM_RANGE(0xf3, 0xf3) AM_WRITENOP
	AM_RANGE(0xf7, 0xf7) AM_WRITENOP

ADDRESS_MAP_END

static INPUT_PORTS_START( base_inputs )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Reset SW")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Books SW")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1) //ron jan needs this
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL1_01")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("PL1_02")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )

	PORT_START("PL1_03")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL1_04")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL1_05")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )

	PORT_START("PL2_01")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("PL2_02")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET ) PORT_PLAYER(2)

	PORT_START("PL2_03")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL2_04")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL2_05")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL ) PORT_PLAYER(2)

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
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

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
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

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

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x01, "DSW4" )
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

static INPUT_PORTS_START( janshi )
	PORT_INCLUDE( base_inputs )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x04, "1 Coin/10 Credits")
	PORT_DIPNAME( 0x08, 0x08, "Round Up Bonus" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, "500" )
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPNAME( 0x30, 0x00, "Base Score" ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x00, "5000 / 8000" )
	PORT_DIPSETTING(    0x10, "4000 / 8000" )
	PORT_DIPSETTING(    0x20, "3000 / 8000" )
	PORT_DIPSETTING(    0x30, "2000 / 8000" )
	PORT_DIPNAME( 0xc0, 0x80, "Win Rate" ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Very_Easy ) )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x03, 0x01, "Play Time" ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x00, "12 Seconds" )
	PORT_DIPSETTING(    0x01, "10 Seconds" )
	PORT_DIPSETTING(    0x02, "8 Seconds" )
	PORT_DIPSETTING(    0x03, "6 Seconds" )
	PORT_DIPNAME( 0x04, 0x04, "Yakuman Bonus" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(	0x00, DEF_STR( No ) )
	PORT_DIPSETTING(	0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(	0x00, DEF_STR( No ) )
	PORT_DIPSETTING(	0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, "BGM" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(	0x00, DEF_STR( No ) )
	PORT_DIPSETTING(	0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, "Voice" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(	0x00, DEF_STR( No ) )
	PORT_DIPSETTING(	0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "Nudity" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(	0x00, DEF_STR( No ) )
	PORT_DIPSETTING(	0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(	0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(	0x80, DEF_STR( Cocktail ) )

	PORT_MODIFY("DSW3")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW3:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW3:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW3:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW3:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW3:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW3:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW3:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW3:8" )

	PORT_MODIFY("DSW4")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW4:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW4:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW4:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW4:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW4:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW4:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW4:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW4:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( ronjan )
	PORT_INCLUDE( base_inputs )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x03, 0x00, "Play Time Limit" ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "16 Seconds" )
	PORT_DIPSETTING(    0x01, "13 Seconds" )
	PORT_DIPSETTING(    0x02, "10 Seconds" )
	PORT_DIPSETTING(    0x03, "7 Seconds" )
	PORT_DIPNAME( 0x04, 0x04, "Coin Payment" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Auto" )
	//FIXME: Bet Min can't be higher than Bet Max, needs a conditional dip here (or maybe, unified dips)
	PORT_DIPNAME( 0x38, 0x38, "Bet Min" ) PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(    0x38, "1" )
	PORT_DIPSETTING(    0x30, "2" )
	PORT_DIPSETTING(    0x28, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x18, "5" )
	PORT_DIPSETTING(    0x10, "6" )
	PORT_DIPSETTING(    0x08, "8" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0xc0, 0x00, "Bet Max" ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0xc0, "5" )
	PORT_DIPSETTING(    0x80, "10" )
	PORT_DIPSETTING(    0x40, "15" )
	PORT_DIPSETTING(    0x00, "20" )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x0f, 0x00, "Rate of Win" ) PORT_DIPLOCATION("SW2:1,2,3,4")
	PORT_DIPSETTING(    0x00, "98%" )
	PORT_DIPSETTING(    0x01, "95%" )
	PORT_DIPSETTING(    0x02, "92%" )
	PORT_DIPSETTING(    0x03, "89%" )
	PORT_DIPSETTING(    0x04, "86%" )
	PORT_DIPSETTING(    0x05, "83%" )
	PORT_DIPSETTING(    0x06, "80%" )
	PORT_DIPSETTING(    0x07, "77%" )
	PORT_DIPSETTING(    0x08, "74%" )
	PORT_DIPSETTING(    0x09, "71%" )
	PORT_DIPSETTING(    0x0a, "68%" )
	PORT_DIPSETTING(    0x0b, "65%" )
	PORT_DIPSETTING(    0x0c, "62%" )
	PORT_DIPSETTING(    0x0d, "59%" )
	PORT_DIPSETTING(    0x0e, "56%" )
	PORT_DIPSETTING(    0x0f, "53%" )
	PORT_DIPNAME( 0x10, 0x10, "Limit Display" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0xe0, 0xe0, "Credit Limit" )  PORT_DIPLOCATION("SW2:6,7,8")
	PORT_DIPSETTING(    0x00, "50000" )
	PORT_DIPSETTING(    0x20, "40000" )
	PORT_DIPSETTING(    0x40, "30000" )
	PORT_DIPSETTING(    0x60, "20000" )
	PORT_DIPSETTING(    0x80, "15000" )
	PORT_DIPSETTING(    0xa0, "10000" )
	PORT_DIPSETTING(    0xc0, "5000" )
	PORT_DIPSETTING(    0xe0, "No Limit" )

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x07, 0x07, "Key In Coinage" ) PORT_DIPLOCATION("SW3:1,2,3")
	PORT_DIPSETTING(    0x00, "1 Coin/500 Credits" )
	PORT_DIPSETTING(    0x01, "1 Coin/200 Credits" )
	PORT_DIPSETTING(    0x02, "1 Coin/100 Credits" )
	PORT_DIPSETTING(    0x03, "1 Coin/50 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin/25 Credits" )
	PORT_DIPSETTING(    0x05, "1 Coin/20 Credits" )
	PORT_DIPSETTING(    0x06, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x78, 0x40, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW3:4,5,6,7")
	PORT_DIPSETTING(    0x78, "10 Coins/1 Credit" )
	PORT_DIPSETTING(    0x70, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x68, "5 Coins/2 Credits" )
	PORT_DIPSETTING(    0x60, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x58, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x48, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x20, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x18, "1 Coin/20 Credits" )
	PORT_DIPSETTING(    0x10, "1 Coin/25 Credits" )
	PORT_DIPSETTING(    0x08, "1 Coin/50 Credits" )
	PORT_DIPSETTING(    0x00, "1 Coin/100 Credits" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW3:8" )

	PORT_MODIFY("DSW4")
	PORT_DIPNAME( 0x01, 0x01, "Odds Type" ) PORT_DIPLOCATION("SW4:1")
	PORT_DIPSETTING(    0x01, "A" )
	PORT_DIPSETTING(    0x00, "B" )
	PORT_DIPNAME( 0x02, 0x02, "Special Bonus Odds" ) PORT_DIPLOCATION("SW4:2")
	PORT_DIPSETTING(    0x02, "A" )
	PORT_DIPSETTING(    0x00, "B" )
	PORT_DIPNAME( 0x04, 0x00, "Kind Mark of Back" ) PORT_DIPLOCATION("SW4:3")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, "Nudity" ) PORT_DIPLOCATION("SW4:4")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, "BGM" ) PORT_DIPLOCATION("SW4:5")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, "Voice" ) PORT_DIPLOCATION("SW4:6")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "Double Up Game" ) PORT_DIPLOCATION("SW4:7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Double Up Limit" ) PORT_DIPLOCATION("SW4:8")
	PORT_DIPSETTING(    0x80, "6" )
	PORT_DIPSETTING(    0x00, "No Limit" )
INPUT_PORTS_END

static INPUT_PORTS_START( pinkiri8 )
	PORT_INCLUDE( base_inputs )

	/* standard mahjong panel converted to a hanafuda one */
	PORT_MODIFY("PL1_01")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_E )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_HANAFUDA_YES )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )

	PORT_MODIFY("PL1_02")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_B )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_HANAFUDA_NO )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )

	PORT_MODIFY("PL1_03")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_C )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_G )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("PL1_04")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_D )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_H )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("PL1_05")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("PL2_01")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_A ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_E ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_HANAFUDA_YES ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )

	PORT_MODIFY("PL2_02")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_B ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_F ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_HANAFUDA_NO ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET ) PORT_PLAYER(2)

	PORT_MODIFY("PL2_03")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_C ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_G ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("PL2_04")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_D ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_H ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("PL2_05")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Game Style" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, "Win / Bet" )
	PORT_DIPSETTING(    0x00, "Out / In" )
	PORT_DIPNAME( 0x02, 0x02, "Premium Hand" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x04, "Coin Payment" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Auto" )
	//FIXME: Bet Min can't be higher than Bet Max, needs a conditional dip here (or maybe, unified dips)
	PORT_DIPNAME( 0x38, 0x38, "Bet Min" ) PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(    0x38, "1" )
	PORT_DIPSETTING(    0x30, "2" )
	PORT_DIPSETTING(    0x28, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x18, "5" )
	PORT_DIPSETTING(    0x10, "6" )
	PORT_DIPSETTING(    0x08, "8" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0xc0, 0x00, "Bet Max" ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0xc0, "5" )
	PORT_DIPSETTING(    0x80, "10" )
	PORT_DIPSETTING(    0x40, "15" )
	PORT_DIPSETTING(    0x00, "20" )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x0f, 0x00, "Rate of Win" ) PORT_DIPLOCATION("SW2:1,2,3,4")
	PORT_DIPSETTING(    0x00, "90%" )
	PORT_DIPSETTING(    0x01, "86%" )
	PORT_DIPSETTING(    0x02, "82%" )
	PORT_DIPSETTING(    0x03, "78%" )
	PORT_DIPSETTING(    0x04, "74%" )
	PORT_DIPSETTING(    0x05, "70%" )
	PORT_DIPSETTING(    0x06, "66%" )
	PORT_DIPSETTING(    0x07, "62%" )
	PORT_DIPSETTING(    0x08, "58%" )
	PORT_DIPSETTING(    0x09, "54%" )
	PORT_DIPSETTING(    0x0a, "50%" )
	PORT_DIPSETTING(    0x0b, "46%" )
	PORT_DIPSETTING(    0x0c, "42%" )
	PORT_DIPSETTING(    0x0d, "38%" )
	PORT_DIPSETTING(    0x0e, "34%" )
	PORT_DIPSETTING(    0x0f, "30%" )
	PORT_DIPNAME( 0x10, 0x10, "Odds Type" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, "A" )
	PORT_DIPSETTING(    0x00, "B" )
	PORT_DIPNAME( 0x20, 0x20, "BGM" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x00, "Oya (Owner)" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, "CPU Only" )
	PORT_DIPSETTING(    0x00, "Winner" )
	PORT_DIPNAME( 0x80, 0x00, "Koi Time Limit" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, "15 Seconds" )
	PORT_DIPSETTING(    0x00, "30 Seconds" )

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x07, 0x07, "Key In Coinage" ) PORT_DIPLOCATION("SW3:1,2,3")
	PORT_DIPSETTING(    0x00, "1 Coin/500 Credits" )
	PORT_DIPSETTING(    0x01, "1 Coin/200 Credits" )
	PORT_DIPSETTING(    0x02, "1 Coin/100 Credits" )
	PORT_DIPSETTING(    0x03, "1 Coin/50 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin/25 Credits" )
	PORT_DIPSETTING(    0x05, "1 Coin/20 Credits" )
	PORT_DIPSETTING(    0x06, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x78, 0x40, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW3:4,5,6,7")
	PORT_DIPSETTING(    0x78, "10 Coins/1 Credit" )
	PORT_DIPSETTING(    0x70, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x68, "5 Coins/2 Credits" )
	PORT_DIPSETTING(    0x60, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x58, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x48, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x20, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x18, "1 Coin/20 Credits" )
	PORT_DIPSETTING(    0x10, "1 Coin/25 Credits" )
	PORT_DIPSETTING(    0x08, "1 Coin/50 Credits" )
	PORT_DIPSETTING(    0x00, "1 Coin/100 Credits" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW3:8" )

	PORT_MODIFY("DSW4")
	PORT_DIPNAME( 0x01, 0x01, "Pinkiri Bonus" ) PORT_DIPLOCATION("SW4:1")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, "Same Month Bonus" ) PORT_DIPLOCATION("SW4:2")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x00, "Play Time Limit" ) PORT_DIPLOCATION("SW4:3")
	PORT_DIPSETTING(    0x04, "7 Seconds" )
	PORT_DIPSETTING(    0x00, "12 Seconds" )
	PORT_DIPNAME( 0x18, 0x00, "Credit Clear" ) PORT_DIPLOCATION("SW4:4,5")
	PORT_DIPSETTING(    0x00, "300 Seconds" )
	PORT_DIPSETTING(    0x08, "180 Seconds" )
	PORT_DIPSETTING(    0x10, "120 Seconds" )
	PORT_DIPSETTING(    0x18, "60 Seconds" )
	PORT_DIPNAME( 0x20, 0x20, "Panel Type" ) PORT_DIPLOCATION("SW4:6") // no real difference?
	PORT_DIPSETTING(    0x20, "Mahjong" )
	PORT_DIPSETTING(    0x00, "Hanafuda" )
	PORT_DIPNAME( 0x40, 0x40, "Flip Flop Button" ) PORT_DIPLOCATION("SW4:7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW4:8" )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	16,8,
	RGN_FRAC(1,5),
	5,
	{ RGN_FRAC(4,5),RGN_FRAC(3,5),RGN_FRAC(2,5),RGN_FRAC(1,5),RGN_FRAC(0,5) },
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*16
};

static GFXDECODE_START( pinkiri8 )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 0x100 )
GFXDECODE_END

static MACHINE_DRIVER_START( pinkiri8 )
	MDRV_CPU_ADD("maincpu",Z180,16000000)
	MDRV_CPU_PROGRAM_MAP(pinkiri8_map)
	MDRV_CPU_IO_MAP(pinkiri8_io)
	MDRV_CPU_VBLANK_INT("screen",nmi_line_assert)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 64*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 64*8-1)
	MDRV_GFXDECODE(pinkiri8)
	MDRV_PALETTE_LENGTH(0x2000)

	MDRV_VIDEO_START(pinkiri8)
	MDRV_VIDEO_UPDATE(pinkiri8)

	MDRV_DEVICE_ADD("janshivdp", JANSHIVDP, 0)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_OKIM6295_ADD("oki", 1056000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_DRIVER_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( pinkiri8 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "pinkiri8-ver.1.02.l1",   0x0000, 0x20000, CRC(f2df5b12) SHA1(e374e184a6a1e932550516011ec09a5accec9b03) )
	ROM_LOAD( "bios.rom", 0x0000, 0x4000, CRC(399df1ee) SHA1(8251f3aa7da4c7899c8e739c10b61260f4471311) ) //overlapped internal ROM

	ROM_REGION( 0x20000*5, "gfx1", 0 )
	ROM_LOAD( "pinkiri8-chr-01.a1",  0x00000, 0x20000, CRC(8ec73662) SHA1(9098348e519ce753dd7f38f0d855181bfc65aa42) )
	ROM_LOAD( "pinkiri8-chr-02.bc1", 0x20000, 0x20000, CRC(8dc20a65) SHA1(4062510fe06e8844a732754b7915a3b67ba2a3c5) )
	ROM_LOAD( "pinkiri8-chr-03.d1",  0x40000, 0x20000, CRC(bd5f269a) SHA1(7dfd039227551f0f0ed4afaafc76ca64a39a9b83) )
	ROM_LOAD( "pinkiri8-chr-04.ef1", 0x60000, 0x20000, CRC(4d0e5005) SHA1(4b90119c359c4de576131fd0e28d2fe1482ce74f) )
	ROM_LOAD( "pinkiri8-chr-05.h1",  0x80000, 0x20000, CRC(036ca165) SHA1(c4a2d6e394bbabcae1413d8a2916a19c90687edf) )

	ROM_REGION( 0x40000, "oki", ROMREGION_ERASE00 )
ROM_END


ROM_START( janshi )
	ROM_REGION( 0x24000, "maincpu", 0 )
	ROM_LOAD( "11.1l",    0x00000, 0x20000, CRC(a7692ddf) SHA1(5e7f43d8337583977baf22a28bbcd9b2182c0cde) )
	ROM_LOAD( "[3] 9009 1992.1 new jansh.bin", 0x0000, 0x4000, CRC(63cd3f12) SHA1(aebac739bffaf043e6acffa978e935f73ee1385f) ) //overlapped internal ROM

	ROM_REGION( 0x140000, "gfx1", 0 )
	ROM_LOAD( "1.1a", 0x000000, 0x40000, CRC(92b140a5) SHA1(f3b38563f74650604ed0faaf84460e0b04b386b7) )
	ROM_LOAD( "2.1b", 0x040000, 0x40000, CRC(6de7e086) SHA1(e87426264f0181c17383ffe0f7ec7ff5fce3d809) )
	ROM_LOAD( "3.1d", 0x080000, 0x40000, CRC(4e94d8f2) SHA1(a25f542943d74915fc82910baafb9ff9db1ffd70) )
	ROM_LOAD( "4.1f", 0x0c0000, 0x40000, CRC(a5f6e3ef) SHA1(f1f3d28b27eea682aa71855a311fb3abdf9af2cd) )
	ROM_LOAD( "5.1h", 0x100000, 0x40000, CRC(ff2cc769) SHA1(ba4cf2923cf3d4d815a9327595f8e1801c3c8a2b) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "6.1k", 0x00000, 0x40000, CRC(8197034d) SHA1(b501dc7a27b1faad1361c309afd726da14b8b5f5) )
ROM_END

ROM_START( ronjan )
	ROM_REGION( 0x24000, "maincpu", 0 )
	ROM_LOAD( "ver201.bin",    0x00000, 0x20000, CRC(caa98c79) SHA1(e18f52fc910e3a77142ad2a3167805cfd664f0f4) )
	ROM_LOAD( "9009 1996.08 ron jan.bin", 0x00000, 0x4000, CRC(4eb74322) SHA1(84f864c0da3fb69948f6eb7ffecf0e722a882efc) ) //overlapped internal ROM

	ROM_REGION( 0x140000, "gfx1", 0 )
	ROM_LOAD( "eagle.1", 0x000000, 0x40000, CRC(11cef2c4) SHA1(fcd46bfa123cd91053f8d49892778e02a275ffdd) )
	ROM_LOAD( "eagle.2", 0x040000, 0x40000, CRC(177c444c) SHA1(5af0f6040ba121c90b3480ce636885cce535d3ea) )
	ROM_LOAD( "eagle.3", 0x080000, 0x40000, CRC(5b15b99f) SHA1(b99e2fa4cde7c8661d1a81ce5045f5df4f1de9f2) )
	ROM_LOAD( "eagle.4", 0x0c0000, 0x40000, CRC(d6797340) SHA1(0394ba570f2008f5a16e7c0a4dc67b1182be8899) )
	ROM_LOAD( "eagle.5", 0x100000, 0x40000, CRC(1aa42eaf) SHA1(edae2d1b58429e09ecfcaa5bcf4a9bfd5fb7cbea) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "eagle.6", 0x00000, 0x40000, CRC(8197034d) SHA1(b501dc7a27b1faad1361c309afd726da14b8b5f5) )
ROM_END

static UINT8 prot_read_index;

static READ8_HANDLER( ronjan_prot_r )
{
	static const char wing_str[6] = { 'W', 'I', 'N', 'G', '8', '9' };

	prot_read_index++;

	if(prot_read_index & 1)
		return 0xff; //value is discarded

	return wing_str[(prot_read_index >> 1)-1];
}

static WRITE8_HANDLER( ronjan_prot_w )
{
	static UINT8 prot_char[6],prot_index;

	if(data == 0)
	{
		prot_index = 0;
	}
	else
	{
		prot_char[prot_index] = data;
		prot_index++;

		if(prot_char[0] == 'E' && prot_char[1] == 'R' && prot_char[2] == 'R' && prot_char[3] == 'O' && prot_char[4] == 'R')
			prot_read_index = 0;
	}
}

static READ8_HANDLER( ronjan_prot_status_r )
{
	return 0; //bit 0 seems a protection status bit
}

static READ8_HANDLER( ronjan_patched_prot_r )
{
	return 0; //value is read then discarded
}

static DRIVER_INIT( ronjan )
{
	memory_install_readwrite8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_IO), 0x90, 0x90, 0, 0, ronjan_prot_r, ronjan_prot_w);
	memory_install_read8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_IO), 0x66, 0x66, 0, 0, ronjan_prot_status_r);
	memory_install_read8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_IO), 0x9f, 0x9f, 0, 0, ronjan_patched_prot_r);
}

GAME( 1992,  janshi,    0,   pinkiri8, janshi,    0,      ROT0, "Eagle",         "Janshi",          GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS | GAME_NOT_WORKING )
GAME( 1994,  ronjan,    0,   pinkiri8, ronjan,    ronjan, ROT0, "Wing Co., Ltd", "Ron Jan (Super)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS | GAME_NOT_WORKING ) // 'SUPER' flashes in the middle of the screen
GAME( 1994,  pinkiri8,  0,   pinkiri8, pinkiri8,  0,      ROT0, "Alta",          "Pinkiri 8",       GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS | GAME_NOT_WORKING )
