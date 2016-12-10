/******************************************************************************************

Seibu Mahjong games (distributed by Tecmo)

CPU: V30 D70116C-10
Sound: Z80 YM3812 M6295
OSC: 12.000MHz 16.000MHz 7.15909MHz


ToDo:
 Inputs are imperfect (missing dips)
 Some sprite flickers on attract mode
 totmejan: Are the "dots" behind the girls in attract mode correct?

PCB Layout

|---------------------------------------------------------------|
|LA4460  YM3812  M6295  E-JAN.U0911 6116        Z80A 7.15909MHz |
|                                                               |
|                                    5.U1016                    |
|                SEI0100                        PAL             |
|                                                               |
|                           E-JAN.U064          4.U061          |
|                                                      SEI0160  |
|           SEI0181                             3.U063          |
|                                                               |
|                                                               |
|           PAL             SEI0200           6264        PAL   |
|                           TC110G21AF                          |
|           82S135.U083                 PAL   6264              |
|                                                               |
|  DSW     DSW                                                  |
|                                              62256     62256  |
|  E-JAN.U078                PAL                                |
|                                                               |
| PAL                        PAL               1.U022    2.U023 |
|            SEI0210                                            |
| PAL                        PAL                                |
|                                                               |
| 12MHz                16MHz        V30                         |
|---------------------------------------------------------------|
Notes:
      V30 clock - 8.000MHz [16/2]
      Z80 clock - 3.579545MHz [7.15909/2]
      YM3812 clock - 3.579545MHz [7.15909/2]
      M6295 clock - 1.000MHz [16/16], Pin 7 HIGH
      VSync - 60Hz
      HSync - 15.38kHz


*******************************************************************************************/

#include "emu.h"
#include "cpu/nec/nec.h"
#include "audio/seibu.h"
#include "sound/3812intf.h"
#include "includes/sei_crtc.h"

#define GOODEJAN_MHZ1 7159090
#define GOODEJAN_MHZ2 16000000
#define GOODEJAN_MHZ3 12000000

extern UINT16 *seibucrtc_sc0vram,*seibucrtc_sc1vram,*seibucrtc_sc2vram,*seibucrtc_sc3vram;
extern UINT16 *seibucrtc_vregs;
extern UINT16 seibucrtc_sc0bank;
static UINT16 goodejan_mux_data;

static WRITE16_HANDLER( goodejan_gfxbank_w )
{
	seibucrtc_sc0bank_w((data & 0x100)>>8);// = (data & 0x100)>>8;
}

/* Multiplexer device for the mahjong panel */
static READ16_HANDLER( mahjong_panel_r )
{
	UINT16 ret;
	ret = 0xffff;

	switch(goodejan_mux_data)
	{
		case 1:    ret = input_port_read(space->machine, "KEY0"); break;
		case 2:    ret = input_port_read(space->machine, "KEY1"); break;
		case 4:    ret = input_port_read(space->machine, "KEY2"); break;
		case 8:    ret = input_port_read(space->machine, "KEY3"); break;
		case 0x10: ret = input_port_read(space->machine, "KEY4"); break;
	}

	return ret;
}

static WRITE16_HANDLER( mahjong_panel_w )
{
	goodejan_mux_data = data;
}

static ADDRESS_MAP_START( goodejan_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x00000, 0x0afff) AM_RAM
	AM_RANGE(0x0c000, 0x0c7ff) AM_RAM_WRITE(seibucrtc_sc0vram_w) AM_BASE(&seibucrtc_sc0vram)
	AM_RANGE(0x0c800, 0x0cfff) AM_RAM_WRITE(seibucrtc_sc3vram_w) AM_BASE(&seibucrtc_sc3vram)
	AM_RANGE(0x0d000, 0x0dfff) AM_RAM_WRITE(paletteram16_xxxxBBBBGGGGRRRR_word_w) AM_BASE_GENERIC(paletteram)
	/*Guess: these two aren't used/initialized at all.*/
	AM_RANGE(0x0e000, 0x0e7ff) AM_RAM_WRITE(seibucrtc_sc1vram_w) AM_BASE(&seibucrtc_sc1vram)
	AM_RANGE(0x0e800, 0x0efff) AM_RAM_WRITE(seibucrtc_sc2vram_w) AM_BASE(&seibucrtc_sc2vram)
	AM_RANGE(0x0f800, 0x0ffff) AM_RAM AM_BASE_GENERIC(spriteram)
	AM_RANGE(0xc0000, 0xfffff) AM_ROM
ADDRESS_MAP_END

/*totmejan CRT is at 8000-804f,goodejan is at 8040-807f(808f but not tested)*/
static ADDRESS_MAP_START( common_io_map, ADDRESS_SPACE_IO, 16 )
	AM_RANGE(0x9000, 0x9001) AM_WRITE(goodejan_gfxbank_w)
	AM_RANGE(0xb000, 0xb003) AM_WRITENOP
	AM_RANGE(0xb004, 0xb005) AM_WRITE(mahjong_panel_w)

	AM_RANGE(0xc000, 0xc001) AM_READ_PORT("DSW1")
	AM_RANGE(0xc002, 0xc003) AM_READ(mahjong_panel_r)
	AM_RANGE(0xc004, 0xc005) AM_READ_PORT("DSW2") // switches
	AM_RANGE(0xd000, 0xd00f) AM_READWRITE(seibu_main_word_r, seibu_main_word_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( totmejan_io_map, ADDRESS_SPACE_IO, 16 )
	AM_RANGE(0x8000, 0x804f) AM_RAM_WRITE(seibucrtc_vregs_w) AM_BASE(&seibucrtc_vregs)
	AM_IMPORT_FROM(common_io_map)
ADDRESS_MAP_END

static ADDRESS_MAP_START( goodejan_io_map, ADDRESS_SPACE_IO, 16 )
	AM_RANGE(0x8040, 0x807f) AM_RAM_WRITE(seibucrtc_vregs_w) AM_BASE(&seibucrtc_vregs)
	AM_IMPORT_FROM(common_io_map)
ADDRESS_MAP_END

static INPUT_PORTS_START( goodejan )
	SEIBU_COIN_INPUTS	/* coin inputs read through sound cpu */

	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0001, 0x0001, "0" )
	PORT_DIPSETTING(	  0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	  0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	  0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	  0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	  0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	  0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Cross Hatch Test" )
	PORT_DIPSETTING(	  0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	  0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	  0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	  0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	  0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	  0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	  0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	  0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	  0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	  0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_START("DSW2")
	PORT_DIPNAME( 0x0001, 0x0001, "1" )
	PORT_DIPSETTING(	  0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	  0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	  0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	  0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	  0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	  0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	  0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	  0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	  0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	  0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	  0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	  0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	  0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	  0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	  0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	  0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout tilelayout =
{
	16,16,	/* 16*16 sprites  */
	RGN_FRAC(1,1),
	4,	/* 4 bits per pixel */
	{ 12, 8, 4, 0 },
	{
	   0,  1,  2,  3,
	   16, 17, 18, 19,
	   512,513,514,515,
	   528,529,530,531
	},
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32, 8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	32*32,
};

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,	/* 4 bits per pixel */
	{ 12, 8, 4, 0 },
	{
		0,  1,  2,  3,
	    16, 17, 18, 19
	},
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32  },
	8*32
};

static GFXDECODE_START( goodejan )
	GFXDECODE_ENTRY( "spr_gfx", 0,tilelayout, 0x200, 0x40 ) /* Sprites */
	GFXDECODE_ENTRY( "bg_gfx", 0, tilelayout, 0x000, 0x30 ) /* Tiles */
	GFXDECODE_ENTRY( "md_gfx", 0, tilelayout, 0x300, 0x10 ) /* Text */
	GFXDECODE_ENTRY( "fg_gfx", 0, tilelayout, 0x600, 0x10 ) /* Tiles */
	GFXDECODE_ENTRY( "tx_gfx", 0, charlayout, 0x100, 0x10 ) /* Text */
GFXDECODE_END

static INTERRUPT_GEN( goodejan_irq )
{
	cpu_set_input_line_and_vector(device,0,HOLD_LINE,0x208/4);
/* vector 0x00c is just a reti */
}

static MACHINE_DRIVER_START( goodejan )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", V30, GOODEJAN_MHZ2/2)
	MDRV_CPU_PROGRAM_MAP(goodejan_map)
	MDRV_CPU_IO_MAP(goodejan_io_map)
	MDRV_CPU_VBLANK_INT("screen",goodejan_irq)

	SEIBU_SOUND_SYSTEM_CPU(GOODEJAN_MHZ1/2)

	MDRV_MACHINE_RESET(seibu_sound)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1) //TODO: dynamic resolution

	MDRV_GFXDECODE(goodejan)
	MDRV_PALETTE_LENGTH(0x1000)

	MDRV_VIDEO_START(seibu_crtc)
	MDRV_VIDEO_UPDATE(seibu_crtc)

	/* sound hardware */
	SEIBU_SOUND_SYSTEM_YM3812_INTERFACE(GOODEJAN_MHZ1/2,GOODEJAN_MHZ2/16)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( totmejan )
	MDRV_IMPORT_FROM( goodejan )
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_IO_MAP(totmejan_io_map)
MACHINE_DRIVER_END

ROM_START( totmejan )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* V30 code */
	ROM_LOAD16_BYTE( "1.022",  0xc0000, 0x20000, CRC(63c3c54f) SHA1(3116b73b848a1f7391a47b994951ba1af92ba298) )
	ROM_LOAD16_BYTE( "2.023",  0xc0001, 0x20000, CRC(c0b9892f) SHA1(127f439a9e625d5a0f5e88102fed6500433cd9cc) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "5.1016", 0x000000, 0x08000,  CRC(8bfdb304) SHA1(454fd84eb7d9338f0b5f8de0ffae541d17b958d5) )
	ROM_CONTINUE(             0x010000, 0x08000 )
	ROM_COPY( "audiocpu", 0, 0x018000, 0x08000 )

	ROM_REGION( 0x080000, "spr_gfx", 0 )
	ROM_LOAD( "e-jan.078", 0x000000, 0x080000, CRC(ff9ee9d8) SHA1(5e49e9a666630ca9867ee96b9d2b8d6f503b25df) )

	ROM_REGION( 0x100000, "bg_gfx", 0 )
	ROM_LOAD( "e-jan.064", 0x000000, 0x100000, CRC(5f6185ee) SHA1(599e4a574672cd1571032e879b3032d06b70e4e2) )

	ROM_REGION( 0x20000, "md_gfx", ROMREGION_ERASEFF )
	/*Empty*/
	ROM_REGION( 0x20000, "fg_gfx", ROMREGION_ERASEFF )
	/*Empty*/

	ROM_REGION( 0x20000, "tx_gfx", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "3.063",  0x00000, 0x10000, CRC(61b5ae88) SHA1(16105a4e97765454079deda8eaa456d60d44e906) )
	ROM_LOAD16_BYTE( "4.061",  0x00001, 0x10000, CRC(29fb6ad2) SHA1(8a9c4625472daefca7fb73a9ef3717e86c3d632f) )

	ROM_REGION( 0x80000, "oki", 0 )	 /* ADPCM samples */
	ROM_LOAD( "e-jan.0911", 0x00000, 0x80000, CRC(a7fb93c2) SHA1(c2e1300f142032c087c96e1a785af28a6d678947) )

	ROM_REGION( 0x200, "user1", 0 ) /* not used */
	ROM_LOAD( "fmj08.083", 0x000, 0x100, CRC(9657b7ad) SHA1(e9b469c2b3534593f7fe0ea19cbbf93b55957e42) )
ROM_END

ROM_START( goodejan )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* V30 code */
	ROM_LOAD16_BYTE( "1.022",  0xc0000, 0x20000, CRC(8555122f) SHA1(92e1ec02fb81ae972eb7492b5d226b40ca65c70d) )
	ROM_LOAD16_BYTE( "2.023",  0xc0001, 0x20000, CRC(32704d74) SHA1(9722b7f1e506a17e0fa5234e05f79333cd99a364) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "5.1016", 0x000000, 0x08000,  CRC(732e9eae) SHA1(d306610f08630708bbbb97d71e9ed4d7e027579a) )
	ROM_CONTINUE(             0x010000, 0x08000 )
	ROM_COPY( "audiocpu", 0, 0x018000, 0x08000 )

	ROM_REGION( 0x080000, "spr_gfx", 0 )
	ROM_LOAD( "e_jan2obj.078", 0x000000, 0x080000, CRC(0f892ef2) SHA1(188ae43db1c48fb6870aa45c64718e901831499b) )

	ROM_REGION( 0x100000, "bg_gfx", 0 )
	ROM_LOAD( "e_jan2scr.064", 0x000000, 0x100000, CRC(71654822) SHA1(fe2a128413999085e321e455aeebda0360d38cb8) )

	ROM_REGION( 0x20000, "md_gfx", ROMREGION_ERASEFF )
	/*Empty*/
	ROM_REGION( 0x20000, "fg_gfx", ROMREGION_ERASEFF )
	/*Empty*/

	ROM_REGION( 0x20000, "tx_gfx", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "3.063",  0x00000, 0x10000, CRC(f355564a) SHA1(35140cc86504d6fdaba00b520d226724bac9f546) )
	ROM_LOAD16_BYTE( "4.061",  0x00001, 0x10000, CRC(5bdf7225) SHA1(a8eded9dc5be1db20cddbed1ae8c22de1674de2a) )

	ROM_REGION( 0x80000, "oki", 0 )	 /* ADPCM samples */
	ROM_LOAD( "e-jan.911", 0x00000, 0x80000, CRC(6d2cbc35) SHA1(61f47e2a94b8877906224f46d8301a26a0b9e55f) )

	ROM_REGION( 0x200, "user1", 0 ) /* not used */
	ROM_LOAD( "fmj08.083", 0x000, 0x100, CRC(9657b7ad) SHA1(e9b469c2b3534593f7fe0ea19cbbf93b55957e42) )
ROM_END

ROM_START( goodejana )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* V30 code */
	ROM_LOAD16_BYTE( "1.u022",  0xc0000, 0x20000, CRC(d496cdd1) SHA1(144a9d8850b3b62520b71efd2ed1459bd673ac92) )
	ROM_LOAD16_BYTE( "2.u023",  0xc0001, 0x20000, CRC(5eda77bb) SHA1(ac54125988f9c929207becf0dcbab72eff4f054a) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "5.1016", 0x000000, 0x08000,  CRC(732e9eae) SHA1(d306610f08630708bbbb97d71e9ed4d7e027579a) )
	ROM_CONTINUE(             0x010000, 0x08000 )
	ROM_COPY( "audiocpu", 0, 0x018000, 0x08000 )

	ROM_REGION( 0x080000, "spr_gfx", 0 )
	ROM_LOAD( "e_jan2obj.078", 0x000000, 0x080000, CRC(0f892ef2) SHA1(188ae43db1c48fb6870aa45c64718e901831499b) )

	ROM_REGION( 0x100000, "bg_gfx", 0 )
	ROM_LOAD( "e_jan2scr.064", 0x000000, 0x100000, CRC(71654822) SHA1(fe2a128413999085e321e455aeebda0360d38cb8) )

	ROM_REGION( 0x20000, "md_gfx", ROMREGION_ERASEFF )
	/*Empty*/
	ROM_REGION( 0x20000, "fg_gfx", ROMREGION_ERASEFF )
	/*Empty*/

	ROM_REGION( 0x20000, "tx_gfx", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "3.063",  0x00000, 0x10000, CRC(f355564a) SHA1(35140cc86504d6fdaba00b520d226724bac9f546) )
	ROM_LOAD16_BYTE( "4.061",  0x00001, 0x10000, CRC(5bdf7225) SHA1(a8eded9dc5be1db20cddbed1ae8c22de1674de2a) )

	ROM_REGION( 0x80000, "oki", 0 )	 /* ADPCM samples */
	ROM_LOAD( "e-jan.911", 0x00000, 0x80000, CRC(6d2cbc35) SHA1(61f47e2a94b8877906224f46d8301a26a0b9e55f) )

	ROM_REGION( 0x200, "user1", 0 ) /* not used */
	ROM_LOAD( "fmj08.083", 0x000, 0x100, CRC(9657b7ad) SHA1(e9b469c2b3534593f7fe0ea19cbbf93b55957e42) )
ROM_END

GAME( 1991, totmejan, 0,        totmejan, goodejan, 0, ROT0, "Seibu Kaihatsu (Tecmo license)", "Tottemo E Jong", GAME_IMPERFECT_GRAPHICS )
GAME( 1991, goodejan, 0,        goodejan, goodejan, 0, ROT0, "Seibu Kaihatsu (Tecmo license)", "Good E Jong -Kachinuki Mahjong Syoukin Oh!!- (set 1)", GAME_IMPERFECT_GRAPHICS )
GAME( 1991, goodejana,goodejan, goodejan, goodejan, 0, ROT0, "Seibu Kaihatsu (Tecmo license)", "Good E Jong -Kachinuki Mahjong Syoukin Oh!!- (set 2)", GAME_IMPERFECT_GRAPHICS )
