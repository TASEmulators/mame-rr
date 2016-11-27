/*
Gunpey
Banpresto, 2000

The hardware looks very Raizing/8ing -ish, especially the ROM stickers and PCB silk-screening
which are identical to those on Brave Blade and Battle Bakraid ;-)

PCB Layout
----------

VG-10
|-------------------------------------|
|        M6295  ROM5                  |
|        YMZ280B-F      ROM4   ROM3   |
|  YAC516      16.93MHz               |
|                       61256  ROM1   |
|                       61256  ROM2   |
|J                                    |
|A             PAL       XILINX       |
|M                       XC95108      |
|M             57.2424MHz             |
|A                            V30     |
|                                     |
|              |-------|              |
|              |AXELL  |              |
|       DSW1   |AG-1   |              |
|       DSW2   |AX51101|  T2316162    |
|              |-------|  T2316162    |
|-------------------------------------|

Notes:
      V30 clock: 14.3106MHz (= 57.2424 / 4)
      YMZ280B clock: 16.93MHz
      OKI M6295 clock: 2.11625MHz (= 16.93 / 8), sample rate = clock / 165
      VSync: 60Hz
      HSync: 15.79kHz

      ROMs:
           GP_ROM1.021 \
           GP_ROM2.022 / 27C040, Main program

           GP_ROM3.025 \
           GP_ROM4.525 / SOP44 32M MASK, Graphics

           GP_ROM5.622   SOP44 32M MASK, OKI samples
*/

#include "emu.h"
#include "cpu/nec/nec.h"
#include "sound/okim6295.h"
#include "sound/ymz280b.h"

static UINT16 *blit_buffer;

static VIDEO_START( gunpey )
{
	blit_buffer = auto_alloc_array(machine, UINT16, 512*512);
}

static VIDEO_UPDATE( gunpey )
{
	int x,y;
	int count;

	count = 0;

	for(y=0;y<512;y++)
	{
		for(x=0;x<512;x++)
		{
			UINT32 color;
			int r,g,b;
			color = (blit_buffer[count] & 0xffff);

			b = (color & 0x001f) << 3;
			g = (color & 0x03e0) >> 2;
			r = (color & 0x7c00) >> 7;

			if(x<screen->visible_area().max_x && y<screen->visible_area().max_y)
				*BITMAP_ADDR32(bitmap, y, x) = b | (g<<8) | (r<<16);


			count++;
		}
	}

	return 0;
}

static WRITE8_HANDLER( gunpey_status_w )
{
}

static READ8_HANDLER( gunpey_status_r )
{
	if(offset == 1)
		return 0x54;

	return 0x00;
}

static READ8_HANDLER( gunpey_inputs_r )
{
	switch(offset+0x7f40)
	{
		case 0x7f40: return input_port_read(space->machine, "DSW1");
		case 0x7f41: return input_port_read(space->machine, "DSW2");
		case 0x7f42: return input_port_read(space->machine, "P1");
		case 0x7f43: return input_port_read(space->machine, "P2");
		case 0x7f44: return input_port_read(space->machine, "SYSTEM");
	}

	return 0xff;
}

static WRITE8_HANDLER( gunpey_blitter_w )
{
	static UINT16 blit_ram[0x10];
	UINT8 *blit_rom = memory_region(space->machine, "blit_data");
	int x,y;

	blit_ram[offset] = data;

	if(offset == 0 && data == 2) // blitter trigger
	{
		int srcx = blit_ram[0x04]+(blit_ram[0x05]<<8);
		int srcy = blit_ram[0x06]+(blit_ram[0x07]<<8);
		int dstx = blit_ram[0x08]+((blit_ram[0x09] & 0x01) <<8);
		int dsty = blit_ram[0x0a]+((blit_ram[0x0b] & 0x01) <<8);
		int xsize = blit_ram[0x0c]+1;
		int ysize = blit_ram[0x0e]+1;
		int color,color_offs;

		if(blit_ram[0x01] == 8) //1bpp?
		{
			// ...
		}
		else //4bpp
		{
			for(y=0;y<ysize;y++)
			{
				for(x=0;x<xsize;x+=2)
				{
					UINT32 src_index = ((srcy+y)*2048+(srcx+x)) & 0x3fffff;
					UINT32 dst_index = ((dsty+y)*512+(dstx+x)) & 0x3ffff;

					color_offs = ((blit_rom[src_index] & 0xf0)>>4) + 0x10;
					color = (blit_rom[color_offs*2+0x3B1DFD]) | (blit_rom[color_offs*2+0x3B1DFD+1]<<8);
					blit_buffer[dst_index+1] = color;

					color_offs = ((blit_rom[src_index] & 0xf)>>0) + 0x10;
					color = (blit_rom[color_offs*2+0x3B1DFD]) | (blit_rom[color_offs*2+0x3B1DFD+1]<<8);
					blit_buffer[dst_index] = color;
				}
			}
		}


		printf("%02x %02x %02x %02x|%02x %02x %02x %02x|%02x %02x %02x %02x|%02x %02x %02x %02x\n"
		,blit_ram[0],blit_ram[1],blit_ram[2],blit_ram[3]
		,blit_ram[4],blit_ram[5],blit_ram[6],blit_ram[7]
		,blit_ram[8],blit_ram[9],blit_ram[0xa],blit_ram[0xb]
		,blit_ram[0xc],blit_ram[0xd],blit_ram[0xe],blit_ram[0xf]);
	}
}

/***************************************************************************************/

static ADDRESS_MAP_START( mem_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x00000, 0x0ffff) AM_RAM
//  AM_RANGE(0x50000, 0x500ff) AM_RAM
//  AM_RANGE(0x50100, 0x502ff) AM_NOP
	AM_RANGE(0x80000, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( io_map, ADDRESS_SPACE_IO, 16 )
	AM_RANGE(0x7f40, 0x7f45) AM_READ8(gunpey_inputs_r,0xffff)

//  AM_RANGE(0x7f48, 0x7f48) AM_WRITE(output_w)
	AM_RANGE(0x7f80, 0x7f81) AM_DEVREADWRITE8("ymz", ymz280b_r, ymz280b_w, 0xffff)

	AM_RANGE(0x7f88, 0x7f89) AM_DEVREADWRITE8("oki", okim6295_r, okim6295_w, 0xff00)

	AM_RANGE(0x7fc8, 0x7fc9) AM_READWRITE8( gunpey_status_r,  gunpey_status_w, 0xffff )
	AM_RANGE(0x7fd0, 0x7fdf) AM_WRITE8( gunpey_blitter_w, 0xffff )
ADDRESS_MAP_END


/***************************************************************************************/


static void sound_irq_gen(running_device *device, int state)
{
	logerror("sound irq\n");
}

static const ymz280b_interface ymz280b_intf =
{
	sound_irq_gen
};


/***************************************************************************************/

static INPUT_PORTS_START( gunpey )
	PORT_START("DSW1")	// IN0 - 7f40
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x00, "Difficulty (vs. mode)" )
	PORT_DIPSETTING(    0x04, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x30, 0x00, "Matches (vs. mode)?" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPSETTING(    0x30, "5" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")	// IN1 - 7f41
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x38, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("P1")	// IN2 - 7f42
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1        ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2        ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3        ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P2")	// IN3 - 7f43
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1        ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2        ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3        ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SYSTEM")	// IN4 - 7f44
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_IMPULSE(1)	// TEST!!
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

/***************************************************************************************/

/* test hack */
static PALETTE_INIT( gunpey )
{
	int i,r,g,b,val;
	UINT8 *blit_rom = memory_region(machine, "blit_data");

	for (i = 0; i < 512; i+=2)
	{
		val = (blit_rom[i+0x3B1DFD]) | (blit_rom[i+0x3B1DFD+1]<<8);

		b = (val & 0x001f) >> 0;
		b<<=3;
		g = (val & 0x03e0) >> 5;
		g<<=3;
		r = (val & 0x7c00) >> 10;
		r<<=3;

		palette_set_color(machine, i/2, MAKE_RGB(r, g, b));
	}

}

static INTERRUPT_GEN( gunpey_interrupt )
{
	cpu_set_input_line_and_vector(device,0,HOLD_LINE,0x200/4);
}

/***************************************************************************************/
static MACHINE_DRIVER_START( gunpey )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", V30, 57242400 / 4)
	MDRV_CPU_PROGRAM_MAP(mem_map)
	MDRV_CPU_IO_MAP(io_map)
	MDRV_CPU_VBLANK_INT("screen", gunpey_interrupt)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(512, 512)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 512-1, 0*8, 512-1)

	MDRV_PALETTE_LENGTH(0x800)
	MDRV_PALETTE_INIT(gunpey)

	MDRV_VIDEO_START(gunpey)
	MDRV_VIDEO_UPDATE(gunpey)

	MDRV_SPEAKER_STANDARD_STEREO("lspeaker","rspeaker")

	MDRV_OKIM6295_ADD("oki", XTAL_16_9344MHz / 8 / 165, OKIM6295_PIN7_HIGH)
	MDRV_SOUND_ROUTE(0, "lspeaker", 0.25)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.25)

	MDRV_SOUND_ADD("ymz", YMZ280B, XTAL_16_9344MHz)
	MDRV_SOUND_CONFIG(ymz280b_intf)
	MDRV_SOUND_ROUTE(0, "lspeaker", 0.25)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.25)
MACHINE_DRIVER_END


/***************************************************************************************/

ROM_START( gunpey )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* V30 code */
	ROM_LOAD16_BYTE( "gp_rom1.021",  0x00000, 0x80000, CRC(07a589a7) SHA1(06c4140ffd5f74b3d3ddfc424f43fcd08d903490) )
	ROM_LOAD16_BYTE( "gp_rom2.022",  0x00001, 0x80000, CRC(f66bc4cf) SHA1(54931d878d228c535b9e2bf22a0a3e41756f0fe5) )

	ROM_REGION( 0x400000, "blit_data", 0 )
	ROM_LOAD( "gp_rom3.025",  0x00000, 0x400000,  CRC(f2d1f9f0) SHA1(0d20301fd33892074508b9d127456eae80cc3a1c) )

	ROM_REGION( 0x400000, "ymz", 0 )
	ROM_LOAD( "gp_rom4.525",  0x000000, 0x400000, CRC(78dd1521) SHA1(91d2046c60e3db348f29f776def02e3ef889f2c1) ) // 11xxxxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x400000, "oki", 0 )
	ROM_LOAD( "gp_rom5.622",  0x000000, 0x400000,  CRC(f79903e0) SHA1(4fd50b4138e64a48ec1504eb8cd172a229e0e965)) // 1xxxxxxxxxxxxxxxxxxxxx = 0xFF
ROM_END

static DRIVER_INIT( gunpey )
{
	UINT8 *rom = memory_region(machine, "maincpu");

	/* patch SLOOOOW cycle checks ... */
	rom[0x848b5] = 0x7e;
//  rom[0x848b6] = 0x03;
	rom[0x89657] = 0x75;
	rom[0x8e628] = 0x75;

}

GAME( 2000, gunpey, 0, gunpey, gunpey, gunpey,	ROT0, "Banpresto", "Gunpey",GAME_NOT_WORKING)
