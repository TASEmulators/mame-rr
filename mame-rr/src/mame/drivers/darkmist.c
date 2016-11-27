/*
***********************************************************************************
Dark Mist (c)1986  Taito / Seibu

driver by

  David Haywood
  Nicola Salmoria
  Tomasz Slanina

Main CPU : z80 (with encryption, external to z80)
Sound CPU: custom T5182 cpu (like seibu sound system but with internal code)

$e000 - coins (two bytes)
$e2b7 - player 1 energy

TODO:

 - sprite/bg and sprite/sprite priorities (name entry screen, player on raft)
 - cocktail mode
 - unknown bit in sprite attr (there's code used for OR-ing sprite attrib with some
   value (taken from ram) when one of coords is greater than 256-16 )
***********************************************************************************
*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "deprecat.h"
#include "audio/t5182.h"


VIDEO_START(darkmist);
VIDEO_UPDATE(darkmist);
PALETTE_INIT(darkmist);

extern UINT8 *darkmist_scroll;
extern UINT8 *darkmist_spritebank;
static UINT8 * darkmist_workram;

int darkmist_hw;



static WRITE8_HANDLER(darkmist_hw_w)
{
  darkmist_hw=data;
  memory_set_bankptr(space->machine, "bank1",&memory_region(space->machine, "maincpu")[0x010000+((data&0x80)?0x4000:0)]);
}

static READ8_HANDLER(t5182shared_r)
{
	return t5182_sharedram[offset];
}

static WRITE8_HANDLER(t5182shared_w)
{
	t5182_sharedram[offset] = data;
}


static ADDRESS_MAP_START( memmap, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc801, 0xc801) AM_READ_PORT("P1")
	AM_RANGE(0xc802, 0xc802) AM_READ_PORT("P2")
	AM_RANGE(0xc803, 0xc803) AM_READ_PORT("START")
	AM_RANGE(0xc804, 0xc804) AM_WRITE(darkmist_hw_w)
	AM_RANGE(0xc805, 0xc805) AM_WRITEONLY AM_BASE(&darkmist_spritebank)
	AM_RANGE(0xc806, 0xc806) AM_READ_PORT("DSW1")
	AM_RANGE(0xc807, 0xc807) AM_READ_PORT("DSW2")
	AM_RANGE(0xc808, 0xc808) AM_READ_PORT("UNK")
	AM_RANGE(0xd000, 0xd3ff) AM_RAM AM_BASE_GENERIC(paletteram)
	AM_RANGE(0xd400, 0xd41f) AM_RAM AM_BASE(&darkmist_scroll)
	AM_RANGE(0xd600, 0xd67f) AM_READWRITE(t5182shared_r, t5182shared_w)
	AM_RANGE(0xd680, 0xd680) AM_WRITE(t5182_sound_irq_w)
	AM_RANGE(0xd681, 0xd681) AM_READ(t5182_sharedram_semaphore_snd_r)
	AM_RANGE(0xd682, 0xd682) AM_WRITE(t5182_sharedram_semaphore_main_acquire_w)
	AM_RANGE(0xd683, 0xd683) AM_WRITE(t5182_sharedram_semaphore_main_release_w)
	AM_RANGE(0xd800, 0xdfff) AM_RAM AM_BASE_GENERIC(videoram)
	AM_RANGE(0xe000, 0xefff) AM_RAM AM_BASE(&darkmist_workram)
	AM_RANGE(0xf000, 0xffff) AM_RAM AM_BASE_SIZE_GENERIC(spriteram)
ADDRESS_MAP_END


static INPUT_PORTS_START( darkmist )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("START")
	PORT_DIPNAME( 0x01, 0x01, "2-0" )
	PORT_DIPSETTING(    0x01, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, "2-1" )
	PORT_DIPSETTING(    0x02, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x04, "2-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 ) PORT_IMPULSE(1)
	PORT_DIPNAME( 0x20, 0x20, "2-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "2-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "2-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_SERVICE_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW1:6" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:7")	/* Listed as "ALWAYS ON" */
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x06, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x20, "10K / 20K" )
	PORT_DIPSETTING(    0x60, "20K / 40K" )
	PORT_DIPSETTING(    0x40, "30K / 60K" )
	PORT_DIPSETTING(    0x00, "40K / 80K" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )


	PORT_START("UNK")
	PORT_DIPNAME( 0x01, 0x01, "5-0" )
	PORT_DIPSETTING(    0x01, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, "5-1" )
	PORT_DIPSETTING(    0x02, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x04, "5-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, "5-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, "5-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, "5-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "5-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "5-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

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
	RGN_FRAC(1,2),
	4,
	{ 0, 4, RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4 },


	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3,
			16+0, 16+1, 16+2, 16+3, 24+0, 24+1, 24+2, 24+3 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
			8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	32*16
};


static GFXDECODE_START( darkmist )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,  0, 16*4 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout,  0, 16*4 )
	GFXDECODE_ENTRY( "gfx3", 0, tilelayout,  0, 16*4 )
GFXDECODE_END

static INTERRUPT_GEN( darkmist_interrupt )
{
	if(cpu_getiloops(device))
		cpu_set_input_line_and_vector(device, 0, HOLD_LINE, 0x08);
	else
		cpu_set_input_line_and_vector(device, 0, HOLD_LINE, 0x10);
}



static MACHINE_DRIVER_START( darkmist )
	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80,4000000)		 /* ? MHz */
	MDRV_CPU_PROGRAM_MAP(memmap)
	MDRV_CPU_VBLANK_INT_HACK(darkmist_interrupt,2)

	MDRV_CPU_ADD(CPUTAG_T5182,Z80,14318180/4)	/* 3.579545 MHz */
	MDRV_CPU_PROGRAM_MAP(t5182_map)
	MDRV_CPU_IO_MAP(t5182_io)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 256-1, 16, 256-16-1)

	MDRV_GFXDECODE(darkmist)
	MDRV_PALETTE_INIT(darkmist)
	MDRV_PALETTE_LENGTH(0x100*4)
	MDRV_VIDEO_START(darkmist)
	MDRV_VIDEO_UPDATE(darkmist)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ymsnd", YM2151, 14318180/4)	/* 3.579545 MHz */
	MDRV_SOUND_CONFIG(t5182_ym2151_interface)
	MDRV_SOUND_ROUTE(0, "mono", 1.0)
	MDRV_SOUND_ROUTE(1, "mono", 1.0)

MACHINE_DRIVER_END

ROM_START( darkmist )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "dm_15.rom", 0x00000, 0x08000, CRC(21e6503c) SHA1(09174fb424b76f7f2a381297e3420ddd2e76b008) )

	ROM_LOAD( "dm_16.rom", 0x10000, 0x08000, CRC(094579d9) SHA1(2449bc9ba38396912ee9b72dd870ea9fcff95776) )

	ROM_REGION( 0x10000, "t5182", 0 ) /* Toshiba T5182 module */
	ROM_LOAD( "t5182.rom", 0x0000, 0x2000, CRC(d354c8fc) SHA1(a1c9e1ac293f107f69cc5788cf6abc3db1646e33) )
	ROM_LOAD( "dm_17.rom", 0x8000, 0x8000, CRC(7723dcae) SHA1(a0c69e7a7b6fd74f7ed6b9c6419aed94aabcd4b0) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "dm_13.rom", 0x00000, 0x02000, CRC(38bb38d9) SHA1(d751990166dd3d503c5de7667679b96210061cd1) )
	ROM_LOAD( "dm_14.rom", 0x02000, 0x02000, CRC(ac5a31f3) SHA1(79083390671062be2eab93cc875a0f86d709a963) )

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "dm_05.rom", 0x10000, 0x10000, CRC(ca79a738) SHA1(66a76ea0d8ecc44f6cc77102303df74f40bf6118) )
	ROM_LOAD( "dm_01.rom", 0x00000, 0x10000, CRC(652aee6b) SHA1(f4150784f7bd7be83a0041e4c52540aa564062ba) )
	ROM_LOAD( "dm_06.rom", 0x30000, 0x10000, CRC(9629ed2c) SHA1(453f6a0b12efdadd7fcbe03ad37afb0afa6be051) )
	ROM_LOAD( "dm_02.rom", 0x20000, 0x10000, CRC(e2dd15aa) SHA1(1f3a6a1e1afabfe9dc47549ef13ae7696302ae88) )

	ROM_REGION( 0x40000, "gfx3", 0)
	ROM_LOAD( "dm_09.rom", 0x00000, 0x10000, CRC(52154b50) SHA1(5ee1a4bcf0752a057b9993b0069d744c35cf55f4) )
	ROM_LOAD( "dm_11.rom", 0x10000, 0x08000, CRC(3118e2f9) SHA1(dfd946ea1310851f97d31ce58d8280f2d92b0f59) )
	ROM_LOAD( "dm_10.rom", 0x20000, 0x10000, CRC(34fd52b5) SHA1(c4ee464ed79ec91f993b0f894572c0288f0ad1d4) )
	ROM_LOAD( "dm_12.rom", 0x30000, 0x08000, CRC(cc4b9839) SHA1(b7e95513d2e06929fed5005caf3bf8c3fba0b597) )

	ROM_REGION( 0x8000, "user1", 0 )
	/* BG layer map ( 512x64 )*/
	ROM_LOAD( "dm_03.rom", 0x00000, 0x08000, CRC(60b40c2a) SHA1(c046273b15dab95ea4851c26ce941e580fa1b6ec) )

	ROM_REGION( 0x8000, "user2", 0 )
	/* BG layer attr ( 512x64 ) */
	ROM_LOAD( "dm_04.rom", 0x00000, 0x08000, CRC(d47b8cd9) SHA1(86eb7a5d8ea63c0c91f455b1b8322cc7b9c4a968) )

	ROM_REGION( 0x04000, "user3", 0 )
	/* FG layer map ( 64x256 ) */
	ROM_LOAD( "dm_07.rom", 0x00000, 0x04000, CRC(889b1277) SHA1(78405110b9cf1ab988c0cbfdb668498dadb41229) )

	ROM_REGION( 0x04000, "user4", 0 )
	/* FG layer attr ( 64x256 ) */
	ROM_LOAD( "dm_08.rom", 0x00000, 0x04000, CRC(f76f6f46) SHA1(ce1c67dc8976106b24fee8d3a0b9e5deb016a327) )

	ROM_REGION( 0x0600, "proms", 0 )
	/* color lookup tables */
	ROM_LOAD( "63s281n.m7",  0x0000, 0x0100, CRC(897ef49f) SHA1(e40c0fb0a68aa91ceaee86e774a428819a4794bb) )
	ROM_LOAD( "63s281n.d7",  0x0100, 0x0100, CRC(a9975a96) SHA1(3a34569fc68ac15f91e1e90d4e273f844b315091) )
	ROM_LOAD( "63s281n.f11", 0x0200, 0x0100, CRC(8096b206) SHA1(257004aa3501121d058afa6f64b1129303246758) )
	ROM_LOAD( "63s281n.j15", 0x0300, 0x0100, CRC(2ea780a4) SHA1(0f8d6791114705e9982f9035f291d2a305b47f0a) )
	/* unknown */
	ROM_LOAD( "63s281n.l1",  0x0400, 0x0100, CRC(208d17ca) SHA1(a77d56337bcac8d9a7bc3411239dfb3045e069ec) )
	ROM_LOAD( "82s129.d11",  0x0500, 0x0100, CRC(866eab0e) SHA1(398ffe2b82b6e2235746fd987d5f5995d7dc8687) )
ROM_END


static void decrypt_gfx(running_machine *machine)
{
	UINT8 *buf = auto_alloc_array(machine, UINT8, 0x40000);
	UINT8 *rom;
	int size;
	int i;

	rom = memory_region(machine, "gfx1");
	size = memory_region_length(machine, "gfx1");

	/* data lines */
	for (i = 0;i < size/2;i++)
	{
		int w1;

		w1 = (rom[i + 0*size/2] << 8) + rom[i + 1*size/2];

		w1 = BITSWAP16(w1, 9,14,7,2, 6,8,3,15,  10,13,5,12,  0,11,4,1);

		buf[i + 0*size/2] = w1 >> 8;
		buf[i + 1*size/2] = w1 & 0xff;
	}

	/* address lines */
	for (i = 0;i < size;i++)
	{
		rom[i] = buf[BITSWAP24(i,23,22,21,20,19,18,17,16,15,14,13,12, 3,2,1, 11,10,9,8, 0, 7,6,5,4)];
	}


	rom = memory_region(machine, "gfx2");
	size = memory_region_length(machine, "gfx2");

	/* data lines */
	for (i = 0;i < size/2;i++)
	{
		int w1;

		w1 = (rom[i + 0*size/2] << 8) + rom[i + 1*size/2];

		w1 = BITSWAP16(w1, 9,14,7,2, 6,8,3,15,  10,13,5,12,  0,11,4,1);

		buf[i + 0*size/2] = w1 >> 8;
		buf[i + 1*size/2] = w1 & 0xff;
	}

	/* address lines */
	for (i = 0;i < size;i++)
	{
		rom[i] = buf[BITSWAP24(i,23,22,21,20,19,18,17,16,15,14,13, 5,4,3,2, 12,11,10,9,8, 1,0, 7,6)];
	}


	rom = memory_region(machine, "gfx3");
	size = memory_region_length(machine, "gfx3");

	/* data lines */
	for (i = 0;i < size/2;i++)
	{
		int w1;

		w1 = (rom[i + 0*size/2] << 8) + rom[i + 1*size/2];

		w1 = BITSWAP16(w1, 9,14,7,2, 6,8,3,15,  10,13,5,12,  0,11,4,1);

		buf[i + 0*size/2] = w1 >> 8;
		buf[i + 1*size/2] = w1 & 0xff;
	}

	/* address lines */
	for (i = 0;i < size;i++)
	{
		rom[i] = buf[BITSWAP24(i, 23,22,21,20,19,18,17,16,15,14, 12,11,10,9,8, 5,4,3, 13, 7,6, 1,0, 2)];
	}

	auto_free(machine, buf);
}

static void decrypt_snd(running_machine *machine)
{
	int i;
	UINT8 *ROM = memory_region(machine, "t5182");

	for(i=0x8000;i<0x10000;i++)
		ROM[i] = BITSWAP8(ROM[i], 7,1,2,3,4,5,6,0);
}

static DRIVER_INIT(darkmist)
{
	const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);
	int i, len;
	UINT8 *ROM = memory_region(machine, "maincpu");
	UINT8 *buffer = auto_alloc_array(machine, UINT8, 0x10000);
	UINT8 *decrypt = auto_alloc_array(machine, UINT8, 0x8000);

	decrypt_gfx(machine);

	decrypt_snd(machine);

	for(i=0;i<0x8000;i++)
	{
		UINT8 p, d;
		p = d = ROM[i];

		if(((i & 0x20) == 0x00) && ((i & 0x8) != 0))
			p ^= 0x20;

		if(((i & 0x20) == 0x00) && ((i & 0xa) != 0))
			d ^= 0x20;

		if(((i & 0x200) == 0x200) && ((i & 0x408) != 0))
			p ^= 0x10;

		if((i & 0x220) != 0x200)
		{
			p = BITSWAP8(p, 7,6,5,2,3,4,1,0);
			d = BITSWAP8(d, 7,6,5,2,3,4,1,0);
		}

		ROM[i] = d;
		decrypt[i] = p;
	}

	memory_set_decrypted_region(space, 0x0000, 0x7fff, decrypt);
	memory_set_bankptr(space->machine, "bank1",&ROM[0x010000]);

	/* adr line swaps */
	ROM = memory_region(machine, "user1");
	len = memory_region_length(machine, "user1");
	memcpy( buffer, ROM, len );

	for(i=0;i<len;i++)
	{
		ROM[i]=buffer[BITSWAP24(i,23,22,21,20,19,18,17,16,15,6,5,4,3,2,14,13,12,11,8,7,1,0,10,9)];
	}

	ROM = memory_region(machine, "user2");
	len = memory_region_length(machine, "user2");
	memcpy( buffer, ROM, len );
	for(i=0;i<len;i++)
	{
		ROM[i]=buffer[BITSWAP24(i,23,22,21,20,19,18,17,16,15,6,5,4,3,2,14,13,12,11,8,7,1,0,10,9)];
	}

	ROM = memory_region(machine, "user3");
	len = memory_region_length(machine, "user3");
	memcpy( buffer, ROM, len );
	for(i=0;i<len;i++)
	{
		ROM[i]=buffer[BITSWAP24(i,23,22,21,20,19,18,17,16,15,14 ,5,4,3,2,11,10,9,8,13,12,1,0,7,6)];
	}

	ROM = memory_region(machine, "user4");
	len = memory_region_length(machine, "user4");
	memcpy( buffer, ROM, len );
	for(i=0;i<len;i++)
	{
		ROM[i]=buffer[BITSWAP24(i,23,22,21,20,19,18,17,16,15,14 ,5,4,3,2,11,10,9,8,13,12,1,0,7,6)];
	}

	auto_free(machine, buffer);
}

GAME( 1986, darkmist, 0, darkmist, darkmist, darkmist, ROT270, "Taito", "The Lost Castle In Darkmist", GAME_IMPERFECT_GRAPHICS|GAME_NO_COCKTAIL )
