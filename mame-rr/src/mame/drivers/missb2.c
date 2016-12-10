/* Miss Bubble 2

A rather odd bootleg of Bubble Bobble with level select, redesigned levels,
redesigned (8bpp!) graphics and different sound hardware... Crazy

The Sound NMI and/or Interrupts aren't likely to be right. The Sound CPU
starts writing to unusual memory ports - either because the NMI/Interrupt
timing is out, or the sheer fact that the Sound CPU code is rather poorly
written, so it may be normal behaviour.

Also, the OKI M6295 seems to be playing the wrong samples, however the current
OKI M6295 sound ROM dump is bad.

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/okim6295.h"
#include "sound/3526intf.h"
#include "includes/bublbobl.h"


/* Video Hardware */

static VIDEO_UPDATE( missb2 )
{
	bublbobl_state *state = (bublbobl_state *)screen->machine->driver_data;
	int offs;
	int sx, sy, xc, yc;
	int gfx_num, gfx_attr, gfx_offs;
	const UINT8 *prom;
	const UINT8 *prom_line;
	UINT16 bg_offs;

	/* Bubble Bobble doesn't have a real video RAM. All graphics (characters */
	/* and sprites) are stored in the same memory region, and information on */
	/* the background character columns is stored in the area dd00-dd3f */

	bitmap_fill(bitmap, cliprect, 255);

	if (!state->video_enable)
		return 0;

	/* background map register */
	//popmessage("%02x",(*state->bgvram) & 0x1f);
	for (bg_offs = ((*state->bgvram) << 4); bg_offs < (((*state->bgvram) << 4) | 0xf); bg_offs++)
	{
		drawgfx_opaque(bitmap,cliprect,screen->machine->gfx[1],
				bg_offs,
				1,
				0,0,
				0,(bg_offs & 0xf) * 0x10);
	}


	sx = 0;

	prom = memory_region(screen->machine, "proms");
	for (offs = 0; offs < state->objectram_size; offs += 4)
	{
		/* skip empty sprites */
		/* this is dword aligned so the UINT32 * cast shouldn't give problems */
		/* on any architecture */
		if (*(UINT32 *)(&state->objectram[offs]) == 0)
			continue;

		gfx_num = state->objectram[offs + 1];
		gfx_attr = state->objectram[offs + 3];
		prom_line = prom + 0x80 + ((gfx_num & 0xe0) >> 1);

		gfx_offs = ((gfx_num & 0x1f) * 0x80);
		if ((gfx_num & 0xa0) == 0xa0)
			gfx_offs |= 0x1000;

		sy = -state->objectram[offs + 0];

		for (yc = 0; yc < 32; yc++)
		{
			if (prom_line[yc / 2] & 0x08)	continue;	/* NEXT */

			if (!(prom_line[yc / 2] & 0x04))	/* next column */
			{
				sx = state->objectram[offs + 2];
				if (gfx_attr & 0x40) sx -= 256;
			}

			for (xc = 0; xc < 2; xc++)
			{
				int goffs, code, /*color,*/ flipx, flipy, x, y;

				goffs = gfx_offs + xc * 0x40 + (yc & 7) * 0x02 +
						(prom_line[yc/2] & 0x03) * 0x10;
				code = state->videoram[goffs] + 256 * (state->videoram[goffs + 1] & 0x03) + 1024 * (gfx_attr & 0x0f);
				//color = (state->videoram[goffs + 1] & 0x3c) >> 2;
				flipx = state->videoram[goffs + 1] & 0x40;
				flipy = state->videoram[goffs + 1] & 0x80;
				x = sx + xc * 8;
				y = (sy + yc * 8) & 0xff;

				if (flip_screen_get(screen->machine))
				{
					x = 248 - x;
					y = 248 - y;
					flipx = !flipx;
					flipy = !flipy;
				}

				drawgfx_transpen(bitmap,cliprect,screen->machine->gfx[0],
						code,
						0,
						flipx,flipy,
						x,y,0xff);
			}
		}

		sx += 16;
	}
	return 0;
}

INLINE void bg_changecolor_RRRRGGGGBBBBxxxx( running_machine *machine, pen_t color, int data )
{
	palette_set_color_rgb(machine, color + 256, pal4bit(data >> 12), pal4bit(data >> 8), pal4bit(data >> 4));
}

static WRITE8_HANDLER( bg_paletteram_RRRRGGGGBBBBxxxx_be_w )
{
	bublbobl_state *state = (bublbobl_state *)space->machine->driver_data;
	state->bg_paletteram[offset] = data;
	bg_changecolor_RRRRGGGGBBBBxxxx(space->machine, offset / 2, state->bg_paletteram[offset | 1] | (state->bg_paletteram[offset & ~1] << 8));
}

static WRITE8_HANDLER( missb2_bg_bank_w )
{
	int bank;

	// I don't know how this is really connected, bit 1 is always high afaik...
	bank = ((data & 2) ? 1 : 0) | ((data & 1) ? 4 : 0);

	memory_set_bank(space->machine, "bank2", bank);
	memory_set_bank(space->machine, "bank3", bank);
}

/* Memory Maps */

static ADDRESS_MAP_START( master_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xdcff) AM_RAM AM_BASE_SIZE_MEMBER(bublbobl_state, videoram, videoram_size)
	AM_RANGE(0xdd00, 0xdfff) AM_RAM AM_BASE_SIZE_MEMBER(bublbobl_state, objectram, objectram_size)
	AM_RANGE(0xe000, 0xf7ff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0xf800, 0xf9ff) AM_RAM_WRITE(paletteram_RRRRGGGGBBBBxxxx_be_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0xfa00, 0xfa00) AM_WRITE(bublbobl_sound_command_w)
	AM_RANGE(0xfa03, 0xfa03) AM_WRITENOP // sound cpu reset
	AM_RANGE(0xfa80, 0xfa80) AM_WRITENOP
	AM_RANGE(0xfb40, 0xfb40) AM_WRITE(bublbobl_bankswitch_w)
	AM_RANGE(0xfc00, 0xfcff) AM_RAM
	AM_RANGE(0xfd00, 0xfdff) AM_RAM			// ???
	AM_RANGE(0xfe00, 0xfe03) AM_RAM			// ???
	AM_RANGE(0xfe80, 0xfe83) AM_RAM			// ???
	AM_RANGE(0xff00, 0xff00) AM_READ_PORT("DSW1")
	AM_RANGE(0xff01, 0xff01) AM_READ_PORT("DSW2")
	AM_RANGE(0xff02, 0xff02) AM_READ_PORT("P1")
	AM_RANGE(0xff03, 0xff03) AM_READ_PORT("P2")
	AM_RANGE(0xff94, 0xff94) AM_WRITENOP	// ???
	AM_RANGE(0xff98, 0xff98) AM_WRITENOP	// ???
ADDRESS_MAP_END

static ADDRESS_MAP_START( slave_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x9000, 0x9fff) AM_ROMBANK("bank2")	// ROM data for the background palette ram
	AM_RANGE(0xa000, 0xafff) AM_ROMBANK("bank3")	// ROM data for the background palette ram
	AM_RANGE(0xb000, 0xb1ff) AM_ROM			// banked ???
	AM_RANGE(0xc000, 0xc1ff) AM_RAM_WRITE(bg_paletteram_RRRRGGGGBBBBxxxx_be_w) AM_BASE_MEMBER(bublbobl_state, bg_paletteram)
	AM_RANGE(0xc800, 0xcfff) AM_RAM			// main ???
	AM_RANGE(0xd000, 0xd000) AM_WRITE(missb2_bg_bank_w)
	AM_RANGE(0xd002, 0xd002) AM_WRITENOP
	AM_RANGE(0xd003, 0xd003) AM_RAM AM_BASE_MEMBER(bublbobl_state, bgvram)
	AM_RANGE(0xe000, 0xf7ff) AM_RAM AM_SHARE("share1")
ADDRESS_MAP_END

// Looks like the original bublbobl code modified to support the OKI M6295.

static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x8fff) AM_RAM
	AM_RANGE(0x9000, 0x9000) AM_DEVREADWRITE("oki", okim6295_r, okim6295_w)
	AM_RANGE(0xa000, 0xa001) AM_DEVREADWRITE("ymsnd", ym3526_r, ym3526_w)
	AM_RANGE(0xb000, 0xb000) AM_READ(soundlatch_r) AM_WRITENOP // message for main cpu
	AM_RANGE(0xb001, 0xb001) AM_READNOP AM_WRITE(bublbobl_sh_nmi_enable_w)	// bit 0: message pending for main cpu, bit 1: message pending for sound cpu
	AM_RANGE(0xb002, 0xb002) AM_WRITE(bublbobl_sh_nmi_disable_w)
	AM_RANGE(0xe000, 0xefff) AM_ROM			// space for diagnostic ROM?
ADDRESS_MAP_END

/* Input Ports */

static INPUT_PORTS_START( missb2 )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Language ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Japanese ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x04, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x08, "20K 80K" )
	PORT_DIPSETTING(    0x0c, "30K 100K" )
	PORT_DIPSETTING(    0x04, "40K 200K" )
	PORT_DIPSETTING(    0x00, "50K 250K" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x10, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPNAME( 0xc0, 0x00, "Monster Speed" )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x80, DEF_STR( High ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Very_High ) )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_TILT ) // ???
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

/* Graphics Layouts */

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,4),
	8,
	{ RGN_FRAC(0,4)+0, RGN_FRAC(0,4)+4, RGN_FRAC(1,4)+0, RGN_FRAC(1,4)+4, RGN_FRAC(2,4)+0, RGN_FRAC(2,4)+4, RGN_FRAC(3,4)+0, RGN_FRAC(3,4)+4 },
	{ 3, 2, 1, 0, 8+3, 8+2, 8+1, 8+0 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static const UINT32 bglayout_xoffset[256] =
{
		0*8,      1*8, 2048*8, 2049*8,    8*8,    9*8, 2056*8, 2057*8,
		4*8,      5*8, 2052*8, 2053*8,   12*8,   13*8, 2060*8, 2061*8,
		256*8 , 257*8, 2304*8, 2305*8,  264*8,  265*8, 2312*8, 2313*8,
		260*8 , 261*8, 2308*8, 2309*8,  268*8,  269*8, 2316*8, 2317*8,
	   1024*8, 1025*8, 3072*8, 3073*8, 1032*8, 1033*8, 3080*8, 3081*8,
	   1028*8, 1029*8, 3076*8, 3077*8, 1036*8, 1037*8, 3084*8, 3085*8,
	   1280*8, 1281*8, 3328*8, 3329*8, 1288*8, 1289*8, 3336*8, 3337*8,
	   1284*8, 1285*8, 3332*8, 3333*8, 1292*8, 1293*8, 3340*8, 3341*8,
		512*8,  513*8, 2560*8, 2561*8,  520*8,  521*8, 2568*8, 2569*8,
		516*8,  517*8, 2564*8, 2565*8,	524*8,  525*8, 2572*8, 2573*8,
		768*8,  769*8, 2816*8, 2817*8,  776*8,  777*8, 2824*8, 2825*8,
		772*8,  773*8, 2820*8, 2821*8,  780*8,  781*8, 2828*8, 2829*8,
	   1536*8, 1537*8, 3584*8, 3585*8, 1544*8, 1545*8, 3592*8, 3593*8,
	   1540*8, 1541*8, 3588*8, 3589*8, 1548*8, 1549*8, 3596*8, 3597*8,
	   1792*8, 1793*8, 3840*8, 3841*8, 1800*8, 1801*8, 3848*8, 3849*8,
	   1796*8, 1797*8, 3844*8, 3845*8, 1804*8, 1805*8, 3852*8, 3853*8,
		  2*8,    3*8, 2050*8, 2051*8,   10*8,   11*8, 2058*8, 2059*8,
		  6*8,    7*8, 2054*8, 2055*8,	 14*8,   15*8, 2062*8, 2063*8,
		258*8,  259*8, 2306*8, 2307*8,  266*8,  267*8, 2314*8, 2315*8,
		262*8,  263*8, 2310*8, 2311*8,	270*8,  271*8, 2318*8, 2319*8,
	   1026*8, 1027*8, 3074*8, 3075*8, 1034*8, 1035*8, 3082*8, 3083*8,
	   1030*8, 1031*8, 3078*8, 3079*8, 1038*8, 1039*8, 3086*8, 3087*8,
	   1282*8, 1283*8, 3330*8, 3331*8, 1290*8, 1291*8, 3338*8, 3339*8,
	   1286*8, 1287*8, 3334*8, 3335*8, 1294*8, 1295*8, 3342*8, 3343*8,
		514*8,  515*8, 2562*8, 2563*8,	522*8,  523*8, 2570*8, 2571*8,
		518*8,  519*8, 2566*8, 2567*8,  526*8,  527*8, 2574*8, 2575*8,
		770*8,  771*8, 2818*8, 2819*8,  778*8,  779*8, 2826*8, 2827*8,
		774*8,  775*8, 2822*8, 2823*8,  782*8,  783*8, 2830*8, 2831*8,
	   1538*8, 1539*8, 3586*8, 3587*8, 1546*8, 1547*8, 3594*8, 3595*8,
	   1542*8, 1543*8, 3590*8, 3591*8, 1550*8, 1551*8, 3598*8, 3599*8,
	   1794*8, 1795*8, 3842*8, 3843*8, 1802*8, 1803*8, 3850*8, 3851*8,
	   1798*8, 1799*8, 3846*8, 3847*8, 1806*8, 1807*8, 3854*8, 3855*8
};

static const gfx_layout bglayout =
{
	256,16,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	EXTENDED_XOFFS,
	{ 0*128, 1*128, 2*128, 3*128, 4*128, 5*128, 6*128, 7*128, 8*128, 9*128, 10*128, 11*128, 12*128, 13*128, 14*128, 15*128 },
	256*128,
	bglayout_xoffset,
	NULL
};

/* Graphics Decode Information */

static GFXDECODE_START( missb2 )
	GFXDECODE_ENTRY( "gfx1", 0x00000, charlayout, 0, 1 )
	GFXDECODE_ENTRY( "gfx2", 0x00000, bglayout,   0, 2 )
GFXDECODE_END


#define MAIN_XTAL 24000000	// not sure about this

/* Sound Interfaces */

// Handler called by the 3526 emulator when the internal timers cause an IRQ
static void irqhandler(running_device *device, int irq)
{
	logerror("YM3526 firing an IRQ\n");
//  cputag_set_input_line(device->machine, "audiocpu", 0, irq ? ASSERT_LINE : CLEAR_LINE);
}

static const ym3526_interface ym3526_config =
{
	irqhandler
};

/* Interrupt Generator */

static INTERRUPT_GEN( missb2_interrupt )
{
	cpu_set_input_line(device, 0, HOLD_LINE);
}

/* Machine Driver */

static MACHINE_START( missb2 )
{
	bublbobl_state *state = (bublbobl_state *)machine->driver_data;

	state->maincpu = machine->device("maincpu");
	state->audiocpu = machine->device("audiocpu");
	state->slave = machine->device("slave");
	state->mcu = NULL;

	state_save_register_global(machine, state->sound_nmi_enable);
	state_save_register_global(machine, state->pending_nmi);
	state_save_register_global(machine, state->sound_status);
	state_save_register_global(machine, state->video_enable);
}

static MACHINE_RESET( missb2 )
{
	bublbobl_state *state = (bublbobl_state *)machine->driver_data;

	state->sound_nmi_enable = 0;
	state->pending_nmi = 0;
	state->sound_status = 0;
}

static MACHINE_DRIVER_START( missb2 )

	/* driver data */
	MDRV_DRIVER_DATA(bublbobl_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, MAIN_XTAL/4)	// 6 MHz
	MDRV_CPU_PROGRAM_MAP(master_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_CPU_ADD("slave", Z80, MAIN_XTAL/4)	// 6 MHz
	MDRV_CPU_PROGRAM_MAP(slave_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_CPU_ADD("audiocpu", Z80, MAIN_XTAL/8)	// 3 MHz
	MDRV_CPU_PROGRAM_MAP(sound_map)
//  MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)
	MDRV_CPU_VBLANK_INT("screen", missb2_interrupt)

	MDRV_QUANTUM_TIME(HZ(6000)) // 100 CPU slices per frame - a high value to ensure proper synchronization of the CPUs

	MDRV_MACHINE_START(missb2)
	MDRV_MACHINE_RESET(missb2)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0, 32*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(missb2)
	MDRV_PALETTE_LENGTH(512)

	MDRV_VIDEO_UPDATE(missb2)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ymsnd", YM3526, MAIN_XTAL/8)
	MDRV_SOUND_CONFIG(ym3526_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)

	MDRV_OKIM6295_ADD("oki", 1056000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.4)
MACHINE_DRIVER_END

/* ROMs */

ROM_START( missb2 )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "msbub2-u.204", 0x00000, 0x10000, CRC(b633bdde) SHA1(29a389c52ff06718f1c4c39f6a854856c237356b) ) /* FIRST AND SECOND HALF IDENTICAL */
	/* ROMs banked at 8000-bfff */
	ROM_LOAD( "msbub2-u.203", 0x10000, 0x10000, CRC(29fd8afe) SHA1(94ead80d20cd3974dd4fb0358915e3bd8b793158) )
	/* 20000-2ffff empty */

	ROM_REGION( 0x10000, "slave", 0 ) /* 64k for the second CPU */
	ROM_LOAD( "msbub2-u.11",  0x0000, 0x10000, CRC(003dc092) SHA1(dff3c2b31d0804a308e5c42cf9705cd3d6144ad7) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k for the third CPU */
	ROM_LOAD( "msbub2-u.211", 0x0000, 0x08000, CRC(08e5d846) SHA1(8509a71df984f0348bdc6ab60eb2ba7ceb9b1246) )

	ROM_REGION( 0x100000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "msbub2-u.14",  0x00000, 0x40000, CRC(b3164b47) SHA1(083a63010515b0aa43b482938ae302b2df985312) )
	ROM_LOAD( "msbub2-u.126", 0x40000, 0x40000, CRC(b0a9a353) SHA1(40d7f4c970d8571de319231c295fa0d2836efcf7) )
	ROM_LOAD( "msbub2-u.124", 0x80000, 0x40000, CRC(4b0d8e5b) SHA1(218da3edcfea228e6df1ac59bc24217713d79410) )
	ROM_LOAD( "msbub2-u.125", 0xc0000, 0x40000, CRC(77b710e2) SHA1(f6f46804a23de6c930bc40a3f45ac70e160f0645) )

	ROM_REGION( 0x200000, "gfx2", 0 ) /* background images */
	ROM_LOAD16_BYTE( "msbub2-u.ic1", 0x100001, 0x80000, CRC(d621cbc3) SHA1(36343d85bdde0e40dfe0f0e4e646546f175903f8) )
	ROM_LOAD16_BYTE( "msbub2-u.ic3", 0x100000, 0x80000, CRC(90e56035) SHA1(8fa18d97a05890178c52b97ff75aed300344a93e) )
	ROM_LOAD16_BYTE( "msbub2-u.ic2", 0x000001, 0x80000, CRC(694c2783) SHA1(401dc8713a02130289f364786c38e70c4c4f9b2e) )
	ROM_LOAD16_BYTE( "msbub2-u.ic4", 0x000000, 0x80000, CRC(be71c9f0) SHA1(1961e931017f644486cea0ce431d50973679c848) )

	ROM_REGION( 0x40000, "oki", 0 ) /* samples */
	ROM_LOAD( "msbub2-u.13", 0x00000, 0x20000, BAD_DUMP CRC(14f07386) SHA1(097897d92226f900e11dbbdd853aff3ac46ff016) )

	/* I doubt this prom is on the board, it's loaded so we can share video emulation with bubble bobble */
	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "a71-25.bin",  0x0000, 0x0100, CRC(2d0f8545) SHA1(089c31e2f614145ef2743164f7b52ae35bc06808) )	/* video timing - taken from bublbobl */
ROM_END

static void configure_banks( running_machine* machine )
{
	UINT8 *ROM = memory_region(machine, "maincpu");
	UINT8 *SLAVE = memory_region(machine, "slave");

	memory_configure_bank(machine, "bank1", 0, 8, &ROM[0x10000], 0x4000);

	/* 2009-11 FP: isn't there a way to configure both at once? */
	memory_configure_bank(machine, "bank2", 0, 7, &SLAVE[0x8000], 0x1000);
	memory_configure_bank(machine, "bank3", 0, 7, &SLAVE[0x9000], 0x1000);
}

static DRIVER_INIT( missb2 )
{
	bublbobl_state *state = (bublbobl_state *)machine->driver_data;

	configure_banks(machine);
	state->video_enable = 0;
}

/* Game Drivers */

GAME( 1996, missb2, 0, missb2, missb2, missb2, ROT0,  "Alpha Co.", "Miss Bubble II", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
