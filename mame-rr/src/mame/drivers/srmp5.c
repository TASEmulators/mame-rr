/*

Super Real Mahjong P5
(c)1994 SETA

preliminary driver by Tomasz Slanina


--

CPU   : R3560 (IDT MIPS R3000 derivative)
SOUND : TC6210AF (ST-0016)
OSC.  : 50.0000MHz 42.9545MHz

SX008-01.BIN ; CHR ROM
SX008-02.BIN ;  |
SX008-03.BIN ;  |
SX008-04.BIN ;  |
SX008-05.BIN ;  |
SX008-06.BIN ;  |
SX008-07.BIN ; /
SX008-08.BIN ; SOUND DATA
SX008-09.BIN ; /
SX008-11.BIN ; MAIN PRG
SX008-12.BIN ;  |
SX008-13.BIN ;  |
SX008-14.BIN ; /


Note:

attract sound ON/OFF of DIPSW doesn't work.
This is not a bug (real machine behaves the same).
*/


#include "emu.h"
#include "deprecat.h"
#include "cpu/z80/z80.h"
#include "cpu/mips/r3000.h"
#include "sound/st0016.h"
#include "includes/st0016.h"

#define DEBUG_CHAR

#define SPRITE_GLOBAL_X 0
#define SPRITE_GLOBAL_Y 1
#define SUBLIST_OFFSET	2
#define SUBLIST_LENGTH	3

#define SUBLIST_OFFSET_SHIFT 3
#define SPRITE_LIST_END_MARKER 0x8000

#define SPRITE_TILE    0
#define SPRITE_PALETTE 1
#define SPRITE_LOCAL_X 2
#define SPRITE_LOCAL_Y 3
#define SPRITE_SIZE    4

#define SPRITE_SUBLIST_ENTRY_LENGTH 8
#define SPRITE_LIST_ENTRY_LENGTH    4

#define SPRITE_DATA_GRANULARITY 0x80

class srmp5_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, srmp5_state(machine)); }

	srmp5_state(running_machine &machine) { }

	UINT32 databank;
	UINT16 *tileram;
	UINT16 *palram;
	UINT16 *sprram;

	UINT8 input_select;

	UINT8 cmd1;
	UINT8 cmd2;
	UINT8 cmd_stat;

	UINT32 vidregs[0x120 / 4];
#ifdef DEBUG_CHAR
	UINT8 tileduty[0x2000];
#endif
};


static VIDEO_UPDATE( srmp5 )
{
	srmp5_state *state = (srmp5_state *)screen->machine->driver_data;
	int x,y,address,xs,xs2,ys,ys2,height,width,xw,yw,xb,yb,sizex,sizey;
	UINT16 *sprite_list=state->sprram;
	UINT16 *sprite_list_end=&state->sprram[0x4000]; //guess
	UINT8 *pixels=(UINT8 *)state->tileram;
	const rectangle &visarea = screen->visible_area();

//Table surface seems to be tiles, but display corrupts when switching the scene if always ON.
//Currently the tiles are OFF.
#ifdef BG_ENABLE
	UINT8 tile_width  = (state->vidregs[2] >> 0) & 0xFF;
	UINT8 tile_height = (state->vidregs[2] >> 8) & 0xFF;
	if(tile_width && tile_height)
	{
		// 16x16 tile
		UINT16 *map = &sprram[0x2000];
		for(yw = 0; yw < tile_height; yw++)
		{
			for(xw = 0; xw < tile_width; xw++)
			{
				UINT16 tile = map[yw * 128 + xw * 2];
				if(tile >= 0x2000) continue;

				address = tile * SPRITE_DATA_GRANULARITY;
				for(y = 0; y < 16; y++)
				{
					for(x = 0; x < 16; x++)
					{
						UINT8 pen = pixels[address];
						if(pen)
						{
							UINT16 pixdata=state->palram[pen];
							*BITMAP_ADDR32(bitmap, yw * 16 + y, xw * 16 + x) = ((pixdata&0x7c00)>>7) | ((pixdata&0x3e0)<<6) | ((pixdata&0x1f)<<19);
						}
						address++;
					}
				}
			}
		}
	}
	else
#endif
		bitmap_fill(bitmap,cliprect,0);

	while((sprite_list[SUBLIST_OFFSET]&SPRITE_LIST_END_MARKER)==0 && sprite_list<sprite_list_end)
	{
		UINT16 *sprite_sublist=&state->sprram[sprite_list[SUBLIST_OFFSET]<<SUBLIST_OFFSET_SHIFT];
		UINT16 sublist_length=sprite_list[SUBLIST_LENGTH];
		INT16 global_x,global_y;

		if(0!=sprite_list[SUBLIST_OFFSET])
		{
			global_x=(INT16)sprite_list[SPRITE_GLOBAL_X];
			global_y=(INT16)sprite_list[SPRITE_GLOBAL_Y];
			while(sublist_length)
			{
				x=(INT16)sprite_sublist[SPRITE_LOCAL_X]+global_x;
				y=(INT16)sprite_sublist[SPRITE_LOCAL_Y]+global_y;
				width =(sprite_sublist[SPRITE_SIZE]>> 4)&0xf;
				height=(sprite_sublist[SPRITE_SIZE]>>12)&0xf;

				sizex=(sprite_sublist[SPRITE_SIZE]>>0)&0xf;
				sizey=(sprite_sublist[SPRITE_SIZE]>>8)&0xf;

				address=(sprite_sublist[SPRITE_TILE] & ~(sprite_sublist[SPRITE_SIZE] >> 11 & 7))*SPRITE_DATA_GRANULARITY;
				y -= (height + 1) * (sizey + 1)-1;
				for(xw=0;xw<=width;xw++)
				{
					xb = (sprite_sublist[SPRITE_PALETTE] & 0x8000) ? (width-xw)*(sizex+1)+x: xw*(sizex+1)+x;
					for(yw=0;yw<=height;yw++)
					{
						yb = yw*(sizey+1)+y;
						for(ys=0;ys<=sizey;ys++)
						{
							ys2 = (sprite_sublist[SPRITE_PALETTE] & 0x4000) ? ys : (sizey - ys);
							for(xs=0;xs<=sizex;xs++)
							{
								UINT8 pen=pixels[address&(0x100000-1)];
								xs2 = (sprite_sublist[SPRITE_PALETTE] & 0x8000) ? (sizex - xs) : xs;
								if(pen)
								{
									if(xb+xs2<=visarea.max_x && xb+xs2>=visarea.min_x && yb+ys2<=visarea.max_y && yb+ys2>=visarea.min_y )
									{
										UINT16 pixdata=state->palram[pen+((sprite_sublist[SPRITE_PALETTE]&0xff)<<8)];
										*BITMAP_ADDR32(bitmap, yb+ys2, xb+xs2) = ((pixdata&0x7c00)>>7) | ((pixdata&0x3e0)<<6) | ((pixdata&0x1f)<<19);
									}
								}
								++address;
							}
						}
					}
				}
				sprite_sublist+=SPRITE_SUBLIST_ENTRY_LENGTH;
				--sublist_length;
			}
		}
		sprite_list+=SPRITE_LIST_ENTRY_LENGTH;
	}

#ifdef DEBUG_CHAR
	{
		int i;
		for(i = 0; i < 0x2000; i++)
		{
			if (state->tileduty[i] == 1)
			{
				decodechar(screen->machine->gfx[0], i, (UINT8 *)state->tileram);
				state->tileduty[i] = 0;
			}
		}
	}
#endif
	return 0;
}

static READ32_HANDLER(srmp5_palette_r)
{
	srmp5_state *state = (srmp5_state *)space->machine->driver_data;

	return state->palram[offset];
}

static WRITE32_HANDLER(srmp5_palette_w)
{
	srmp5_state *state = (srmp5_state *)space->machine->driver_data;

	COMBINE_DATA(&state->palram[offset]);
	palette_set_color(space->machine, offset, MAKE_RGB(data << 3 & 0xFF, data >> 2 & 0xFF, data >> 7 & 0xFF));
}
static WRITE32_HANDLER(bank_w)
{
	srmp5_state *state = (srmp5_state *)space->machine->driver_data;

	COMBINE_DATA(&state->databank);
}

static READ32_HANDLER(tileram_r)
{
	srmp5_state *state = (srmp5_state *)space->machine->driver_data;

	return state->tileram[offset];
}

static WRITE32_HANDLER(tileram_w)
{
	srmp5_state *state = (srmp5_state *)space->machine->driver_data;

	state->tileram[offset] = data & 0xFFFF; //lower 16bit only
#ifdef DEBUG_CHAR
	state->tileduty[offset >> 6] = 1;
#endif
}

static READ32_HANDLER(spr_r)
{
	srmp5_state *state = (srmp5_state *)space->machine->driver_data;

	return state->sprram[offset];
}

static WRITE32_HANDLER(spr_w)
{
	srmp5_state *state = (srmp5_state *)space->machine->driver_data;

	state->sprram[offset] = data & 0xFFFF; //lower 16bit only
}

static READ32_HANDLER(data_r)
{
	srmp5_state *state = (srmp5_state *)space->machine->driver_data;
	UINT32 data;
	const UINT8 *usr = memory_region(space->machine, "user2");

	data=((state->databank>>4)&0xf)*0x100000; //guess
	data=usr[data+offset*2]+usr[data+offset*2+1]*256;
	return data|(data<<16);
}

static WRITE32_HANDLER(input_select_w)
{
	srmp5_state *state = (srmp5_state *)space->machine->driver_data;

	state->input_select = data & 0x0F;
}

static READ32_HANDLER(srmp5_inputs_r)
{
	srmp5_state *state = (srmp5_state *)space->machine->driver_data;
	UINT32 ret = 0;

	switch (state->input_select)
	{
	case 0x01:
		ret = input_port_read(space->machine, "IN0");
		break;
	case 0x02:
		ret = input_port_read(space->machine, "IN1");
		break;
	case 0x04:
		ret = input_port_read(space->machine, "IN2");
		break;
	case 0x00:
	case 0x08:
		ret = input_port_read(space->machine, "IN3");
		break;
	}
	return ret;
}

//almost all cmds are sound related
static WRITE32_HANDLER(cmd1_w)
{
	srmp5_state *state = (srmp5_state *)space->machine->driver_data;

	state->cmd1 = data & 0xFF;
	logerror("cmd1_w %08X\n", data);
}

static WRITE32_HANDLER(cmd2_w)
{
	srmp5_state *state = (srmp5_state *)space->machine->driver_data;

	state->cmd2 = data & 0xFF;
	state->cmd_stat = 5;
	logerror("cmd2_w %08X\n", data);
}

static READ32_HANDLER(cmd_stat32_r)
{
	srmp5_state *state = (srmp5_state *)space->machine->driver_data;

	return state->cmd_stat;
}

static READ32_HANDLER(srmp5_vidregs_r)
{
	srmp5_state *state = (srmp5_state *)space->machine->driver_data;

	logerror("vidregs read  %08X %08X\n", offset << 2, state->vidregs[offset]);
	return state->vidregs[offset];
}

static WRITE32_HANDLER(srmp5_vidregs_w)
{
	srmp5_state *state = (srmp5_state *)space->machine->driver_data;

	COMBINE_DATA(&state->vidregs[offset]);
	if(offset != 0x10C / 4)
		logerror("vidregs write %08X %08X\n", offset << 2, state->vidregs[offset]);
}

static READ32_HANDLER(irq_ack_clear)
{
	cputag_set_input_line(space->machine, "sub", R3000_IRQ4, CLEAR_LINE);
	return 0;
}

static ADDRESS_MAP_START( srmp5_mem, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x000fffff) AM_RAM //maybe 0 - 2fffff ?
	AM_RANGE(0x002f0000, 0x002f7fff) AM_RAM
	AM_RANGE(0x01000000, 0x01000003) AM_WRITEONLY  // 0xaa .. watchdog ?
	AM_RANGE(0x01800000, 0x01800003) AM_RAM //?1
	AM_RANGE(0x01800004, 0x01800007) AM_READ_PORT("DSW1")
	AM_RANGE(0x01800008, 0x0180000b) AM_READ_PORT("DSW2")
	AM_RANGE(0x0180000c, 0x0180000f) AM_WRITE(bank_w)
	AM_RANGE(0x01800010, 0x01800013) AM_READ(srmp5_inputs_r) //multiplexed controls (selected by writes to 1c)
	AM_RANGE(0x01800014, 0x01800017) AM_READ_PORT("TEST")
	AM_RANGE(0x0180001c, 0x0180001f) AM_WRITE(input_select_w)//c1 c2 c4 c8 => mahjong inputs (at $10) - bits 0-3
	AM_RANGE(0x01800200, 0x01800203) AM_RAM  //sound related ? only few writes after boot
	AM_RANGE(0x01802000, 0x01802003) AM_WRITE(cmd1_w)
	AM_RANGE(0x01802004, 0x01802007) AM_WRITE(cmd2_w)
	AM_RANGE(0x01802008, 0x0180200b) AM_READ(cmd_stat32_r)
	AM_RANGE(0x01a00000, 0x01bfffff) AM_READ(data_r)
	AM_RANGE(0x01c00000, 0x01c00003) AM_READNOP // debug? 'Toru'

	AM_RANGE(0x0a000000, 0x0a0fffff) AM_READWRITE(spr_r, spr_w)
	AM_RANGE(0x0a100000, 0x0a17ffff) AM_READWRITE(srmp5_palette_r, srmp5_palette_w)
	//0?N???A?????????i??????????
	AM_RANGE(0x0a180000, 0x0a180003) AM_READNOP // write 0x00000400
	AM_RANGE(0x0a180000, 0x0a18011f) AM_READWRITE(srmp5_vidregs_r, srmp5_vidregs_w)
	AM_RANGE(0x0a200000, 0x0a3fffff) AM_READWRITE(tileram_r, tileram_w)

	AM_RANGE(0x1eff0000, 0x1eff001f) AM_WRITEONLY
	AM_RANGE(0x1eff003c, 0x1eff003f) AM_READ(irq_ack_clear)
	AM_RANGE(0x1fc00000, 0x1fdfffff) AM_ROM AM_REGION("user1", 0)
	AM_RANGE(0x2fc00000, 0x2fdfffff) AM_ROM AM_REGION("user1", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( st0016_mem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xe900, 0xe9ff) AM_DEVREADWRITE("stsnd", st0016_snd_r, st0016_snd_w)
	AM_RANGE(0xec00, 0xec1f) AM_READ(st0016_character_ram_r) AM_WRITE(st0016_character_ram_w)
	AM_RANGE(0xf000, 0xffff) AM_RAM
ADDRESS_MAP_END

READ8_HANDLER(st0016_dma_r);

static READ8_HANDLER(cmd1_r)
{
	srmp5_state *state = (srmp5_state *)space->machine->driver_data;

	state->cmd_stat = 0;
	return state->cmd1;
}

static READ8_HANDLER(cmd2_r)
{
	srmp5_state *state = (srmp5_state *)space->machine->driver_data;

	return state->cmd2;
}

static READ8_HANDLER(cmd_stat8_r)
{
	srmp5_state *state = (srmp5_state *)space->machine->driver_data;

	return state->cmd_stat;
}

static ADDRESS_MAP_START( st0016_io, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0xbf) AM_READ(st0016_vregs_r) AM_WRITE(st0016_vregs_w)
	AM_RANGE(0xc0, 0xc0) AM_READ(cmd1_r)
	AM_RANGE(0xc1, 0xc1) AM_READ(cmd2_r)
	AM_RANGE(0xc2, 0xc2) AM_READ(cmd_stat8_r)
	AM_RANGE(0xe1, 0xe1) AM_WRITE(st0016_rom_bank_w)
	AM_RANGE(0xe7, 0xe7) AM_WRITE(st0016_rom_bank_w)
	AM_RANGE(0xf0, 0xf0) AM_READ(st0016_dma_r)
ADDRESS_MAP_END


static INPUT_PORTS_START( srmp5 )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0040, 0x0040, "PUT" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )
	PORT_BIT( 0xffffff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0000, "8" )
	PORT_DIPSETTING(      0x0001, "7" )
	PORT_DIPSETTING(      0x0002, "6" )
	PORT_DIPSETTING(      0x0003, "5" )
	PORT_DIPSETTING(      0x0007, "4" )
	PORT_DIPSETTING(      0x0006, "1" )
	PORT_DIPSETTING(      0x0005, "2" )
	PORT_DIPSETTING(      0x0004, "3" )
	PORT_DIPNAME( 0x0008, 0x0008, "Kuitan" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Test ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT ( 0xffffff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0")
	PORT_BIT ( 0xfffffff0, IP_ACTIVE_LOW, IPT_UNUSED ) // explicitely discarded
	PORT_BIT ( 0x00000001, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT ( 0x00000002, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT ( 0x00000004, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT ( 0x00000008, IP_ACTIVE_LOW, IPT_MAHJONG_PON )

	PORT_START("IN1")
	PORT_BIT ( 0xffffffc0, IP_ACTIVE_LOW, IPT_UNUSED ) // explicitely discarded
	PORT_BIT ( 0x00000001, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT ( 0x00000002, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT ( 0x00000004, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT ( 0x00000008, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT ( 0x00000010, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT ( 0x00000020, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN2")
	PORT_BIT ( 0xffffffe0, IP_ACTIVE_LOW, IPT_UNUSED ) // explicitely discarded
	PORT_BIT ( 0x00000001, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT ( 0x00000002, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT ( 0x00000004, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT ( 0x00000008, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT ( 0x00000010, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )

	PORT_START("IN3")
	PORT_BIT ( 0xffffff60, IP_ACTIVE_LOW, IPT_UNUSED ) // explicitely discarded
	PORT_BIT ( 0x00000001, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT ( 0x00000002, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT ( 0x00000004, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT ( 0x00000008, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT ( 0x00000010, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT ( 0x00000080, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)

	PORT_START("TEST")
	PORT_BIT ( 0x00000080, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F2)
	PORT_BIT ( 0xffffff7f, IP_ACTIVE_LOW, IPT_UNUSED ) // explicitely discarded

INPUT_PORTS_END

static const st0016_interface st0016_config =
{
	&st0016_charram
};

static const r3000_cpu_core config =
{
	1,	/* 1 if we have an FPU, 0 otherwise */
	4096,	/* code cache size */
	4096	/* data cache size */
};

static const gfx_layout tile_16x8x8_layout =
{
	16,8,
	RGN_FRAC(1,1),
	8,
	{ STEP8(0, 1) },
	{ STEP16(0, 8) },
	{ STEP8(0, 8*16) },
	16*8*8
};

#if 0
static const gfx_layout tile_16x16x8_layout =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ STEP8(0, 1) },
	{ STEP16(0, 8) },
	{ STEP16(0, 8*16) },
	16*16*8
};
#endif

static GFXDECODE_START( srmp5 )
	GFXDECODE_ENTRY( "gfx1", 0, tile_16x8x8_layout,  0x0, 0x800  )
	//GFXDECODE_ENTRY( "gfx1", 0, tile_16x16x8_layout, 0x0, 0x800  )
GFXDECODE_END

static MACHINE_DRIVER_START( srmp5 )

	MDRV_DRIVER_DATA( srmp5_state )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu",Z80,8000000)
	MDRV_CPU_PROGRAM_MAP(st0016_mem)
	MDRV_CPU_IO_MAP(st0016_io)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_CPU_ADD("sub", R3000LE, 25000000)
	MDRV_CPU_CONFIG(config)
	MDRV_CPU_PROGRAM_MAP(srmp5_mem)
	MDRV_CPU_VBLANK_INT("screen", irq4_line_assert)

	MDRV_QUANTUM_TIME(HZ(6000))

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(96*8, 64*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 42*8-1, 2*8, 32*8-1)

	MDRV_PALETTE_LENGTH(0x1800)
#ifdef DEBUG_CHAR
	MDRV_GFXDECODE( srmp5 )
#endif
	MDRV_VIDEO_START(st0016)
	MDRV_VIDEO_UPDATE(srmp5)

	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("stsnd", ST0016, 0)
	MDRV_SOUND_CONFIG(st0016_config)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END

ROM_START( srmp5 )
	ROM_REGION( 0x410000, "maincpu", 0 )
	ROM_LOAD( "sx008-08.bin",   0x010000, 0x200000,   CRC(d4ac54f4) SHA1(c3dc76cd71485796a0b6a960294ea96eae8c946e) )
	ROM_LOAD( "sx008-09.bin",   0x210000, 0x200000,   CRC(5a3e6560) SHA1(92ea398f3c5e3035869f0ca5dfe7b05c90095318) )
	ROM_COPY( "maincpu",  0x10000, 0x00000, 0x08000 )

	ROM_REGION32_BE( 0x200000, "user1", 0 )
	ROM_LOAD32_BYTE( "sx008-14.bin",   0x00000, 0x80000,   CRC(b5c55120) SHA1(0a41351c9563b2c6a00709189a917757bd6e0a24) )
	ROM_LOAD32_BYTE( "sx008-13.bin",   0x00001, 0x80000,   CRC(0af475e8) SHA1(24cddffa0f8c81832ae8870823d772e3b7493194) )
	ROM_LOAD32_BYTE( "sx008-12.bin",   0x00002, 0x80000,   CRC(43e9bb98) SHA1(e46dd98d2e1babfa12ddf2fa9b31377e8691d3a1) )
	ROM_LOAD32_BYTE( "sx008-11.bin",   0x00003, 0x80000,   CRC(ca15ff45) SHA1(5ee610e0bb835568c36898210a6f8394902d5b54) )

	ROM_REGION( 0xf00000, "user2",0) /* gfx ? */
	ROM_LOAD( "sx008-01.bin",   0x000000, 0x200000,   CRC(82dabf48) SHA1(c53e9ed0056c431eab13ab362936c25d3cc5abba) )
	ROM_LOAD( "sx008-02.bin",   0x200000, 0x200000,   CRC(cfd2be0f) SHA1(a21f2928e08047c97443123aceba7ff4e95c6d3d) )
	ROM_LOAD( "sx008-03.bin",   0x400000, 0x200000,   CRC(d7323b10) SHA1(94ecc17b6b8b071cf2c61bbef4aec2c6c7693c62) )
	ROM_LOAD( "sx008-04.bin",   0x600000, 0x200000,   CRC(b10d3067) SHA1(21c36307780d4f38ec54d87cd222d65e4f8c00a5) )
	ROM_LOAD( "sx008-05.bin",   0x800000, 0x200000,   CRC(0ff5e6f5) SHA1(ab7d021757f341d28db6d7d009c20ec9d7bd83c1) )
	ROM_LOAD( "sx008-06.bin",   0xa00000, 0x200000,   CRC(ba6fd7c4) SHA1(f086195c5c647e07e77ce2a23e94d28e6ad9ff4f) )
	ROM_LOAD( "sx008-07.bin",   0xc00000, 0x200000,   CRC(3564485d) SHA1(12464de4e2b6c4df1595183996d1987f0ecffb01) )
#ifdef DEBUG_CHAR
	ROM_REGION( 0x100000, "gfx1", 0)
	ROM_FILL( 0, 0x100000, 0x00)
#endif
ROM_END

static DRIVER_INIT(srmp5)
{
	srmp5_state *state = (srmp5_state *)machine->driver_data;
	st0016_game = 9;

	state->tileram = auto_alloc_array(machine, UINT16, 0x100000/2);
	state->sprram  = auto_alloc_array(machine, UINT16, 0x080000/2);
	state->palram  = auto_alloc_array(machine, UINT16, 0x040000/2);
#ifdef DEBUG_CHAR
	memset(state->tileduty, 1, 0x2000);
#endif
}

GAME( 1994, srmp5,	0,	  srmp5,    srmp5,    srmp5,    ROT0, "Seta",  "Super Real Mahjong P5", GAME_IMPERFECT_GRAPHICS)
