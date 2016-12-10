/*
       Disco Boy

Similar to mitchell.c / egghunt.c .. clearly derived from that hardware

PCB Layout
----------

SOFT ART CO. 1993 NO - 1021
PROGRAM NO-93-01-14-0024
+------------------------------------------+
| YM3014 YM3812 10MHz            5.u94     |
|       6116                    6.u124     |
|       1.u45                    7.u95     |
| M5205 2.u28                   8.u125     |
|J     Z8400B                          6116|
|A          6116          6116 TPC1020AFN  |
|M          6116                       6116|
|M    DSW2                                 |
|A DSW1                          6264      |
|      6264                       u80  u81 |
|12MHz  u2                       u50  u5   |
|      u18                        u78  u79 |
|    Z0840004PSC                 u46  u49  |
+------------------------------------------+

Notes:
  Zilog Z0840004PSC (Z80 cpu, main program CPU)
  Goldstar Z8400B PS (Z80 cpu, sound CPU)
  Yamaha YM3014/YM3812 (rebadged as 83142/5A12)
  OKI M5205
  TI TPC1020AFN-084C
  10.000MHz & 12.000MHz OSCs

*/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "deprecat.h"
#include "sound/msm5205.h"
#include "sound/3812intf.h"



class discoboy_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, discoboy_state(machine)); }

	discoboy_state(running_machine &machine) { }

	/* memory pointers */
	UINT8 *  ram_1;
	UINT8 *  ram_2;
	UINT8 *  ram_3;
	UINT8 *  ram_4;
	UINT8 *  ram_att;

	/* video-related */
	UINT8    ram_bank;
	UINT8    gfxbank;
	UINT8    port_00;
	int      adpcm_data;

	/* devices */
	running_device *audiocpu;
};



static VIDEO_START( discoboy )
{
}

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	discoboy_state *state = (discoboy_state *)machine->driver_data;
	int flipscreen = 0;
	int offs, sx, sy;

	for (offs = 0x1000 - 0x40; offs >= 0; offs -= 0x20)
	{
		int code = state->ram_4[offs];
		int attr = state->ram_4[offs + 1];
		int color = attr & 0x0f;
		sx = state->ram_4[offs + 3] + ((attr & 0x10) << 4);
		sy = ((state->ram_4[offs + 2] + 8) & 0xff) - 8;
		code += (attr & 0xe0) << 3;

		if (code >= 0x400)
		{
			if ((state->gfxbank & 0x30) == 0x00)
			{
				code = 0x400 + (code & 0x3ff);
			}
			else if ((state->gfxbank & 0x30) == 0x10)
			{
				code = 0x400 + (code & 0x3ff) + 0x400;
			}
			else if ((state->gfxbank & 0x30) == 0x20)
			{
				code = 0x400 + (code & 0x3ff) + 0x800;
			}
			else if ((state->gfxbank & 0x30) == 0x30)
			{
				code = 0x400 + (code & 0x3ff) + 0xc00;
			}
			else
			{
				code = mame_rand(machine);
			}
		}

		drawgfx_transpen(bitmap,cliprect,machine->gfx[0],
				 code,
				 color,
				 flipscreen,0,
				 sx,sy,15);
	}
}


static VIDEO_UPDATE( discoboy )
{
	discoboy_state *state = (discoboy_state *)screen->machine->driver_data;
	UINT16 x, y;
	int i;
	int count = 0;

	for (i = 0; i < 0x800; i += 2)
	{
		UINT16 pal;
		int r, g, b;
		pal = state->ram_1[i] | (state->ram_1[i + 1] << 8);

		b = ((pal >> 0) & 0xf) << 4;
		g = ((pal >> 4) & 0xf) << 4;
		r = ((pal >> 8) & 0xf) << 4;

		palette_set_color(screen->machine, i / 2, MAKE_RGB(r, g, b));
	}

	for (i = 0; i < 0x800; i += 2)
	{
		UINT16 pal;
		int r,g,b;
		pal = state->ram_2[i] | (state->ram_2[i + 1] << 8);

		b = ((pal >> 0) & 0xf) << 4;
		g = ((pal >> 4) & 0xf) << 4;
		r = ((pal >> 8) & 0xf) << 4;

		palette_set_color(screen->machine, (i / 2) + 0x400, MAKE_RGB(r, g, b));
	}

	bitmap_fill(bitmap, cliprect, 0x3ff);

	for (y = 0; y < 32; y++)
	{
		for (x = 0; x < 64; x++)
		{
			UINT16 tileno = state->ram_3[count] | (state->ram_3[count + 1] << 8);

			if (tileno > 0x2000)
			{
				if ((state->gfxbank & 0x40) == 0x40)
					tileno = 0x2000 + (tileno & 0x1fff) + 0x2000;
				else
					tileno = 0x2000 + (tileno & 0x1fff) + 0x0000;
			}

			drawgfx_opaque(bitmap, cliprect, screen->machine->gfx[1], tileno, state->ram_att[count / 2], 0, 0, x*8, y*8);
			count += 2;
		}
	}

	draw_sprites(screen->machine, bitmap, cliprect);

	return 0;
}

#ifdef UNUSED_FUNCTION
void discoboy_setrombank( running_machine *machine, UINT8 data )
{
	UINT8 *ROM = memory_region(machine, "maincpu");
	data &= 0x2f;
	memory_set_bankptr(space->machine, "bank1", &ROM[0x6000 + (data * 0x1000)] );
}
#endif

static WRITE8_HANDLER( rambank_select_w )
{
	discoboy_state *state = (discoboy_state *)space->machine->driver_data;
	state->ram_bank = data;
	if (data &= 0x83) logerror("rambank_select_w !!!!!");
}

static WRITE8_HANDLER( discoboy_port_00_w )
{
	discoboy_state *state = (discoboy_state *)space->machine->driver_data;
	if (data & 0xfe) logerror("unk discoboy_port_00_w %02x\n",data);
	state->port_00 = data;
}

static WRITE8_HANDLER( discoboy_port_01_w )
{
	discoboy_state *state = (discoboy_state *)space->machine->driver_data;

	// 00 10 20 30 during gameplay  1,2,3 other times?? title screen bit 0x40 toggle
	//printf("unk discoboy_port_01_w %02x\n",data);
	// discoboy gfxbank
	state->gfxbank = data & 0xf0;

	memory_set_bank(space->machine, "bank1", data & 0x07);
}

static WRITE8_HANDLER( discoboy_port_03_w ) // sfx? (to sound cpu)
{
	discoboy_state *state = (discoboy_state *)space->machine->driver_data;
	//  printf("unk discoboy_port_03_w %02x\n", data);
	//  cpu_set_input_line(state->audiocpu, INPUT_LINE_NMI, HOLD_LINE);
	soundlatch_w(space, 0, data);
	cpu_set_input_line(state->audiocpu, 0, HOLD_LINE);
}

static WRITE8_HANDLER( discoboy_port_06_w )
{
	//printf("unk discoboy_port_06_w %02x\n",data);
	if (data != 0) logerror("port 06!!!! %02x\n",data);
}


static WRITE8_HANDLER( rambank_w )
{
	discoboy_state *state = (discoboy_state *)space->machine->driver_data;

	if (state->ram_bank & 0x20)
		state->ram_2[offset] = data;
	else
		state->ram_1[offset] = data;
}

static READ8_HANDLER( rambank_r )
{
	discoboy_state *state = (discoboy_state *)space->machine->driver_data;

	if (state->ram_bank & 0x20)
		return state->ram_2[offset];
	else
		return state->ram_1[offset];
}

static READ8_HANDLER( rambank2_r )
{
	discoboy_state *state = (discoboy_state *)space->machine->driver_data;

	if (state->port_00 == 0x00)
		return state->ram_3[offset];
	else if (state->port_00 == 0x01)
		return state->ram_4[offset];
	else
		printf("unk rb2_r\n");

	return mame_rand(space->machine);
}

static WRITE8_HANDLER( rambank2_w )
{
	discoboy_state *state = (discoboy_state *)space->machine->driver_data;

	if (state->port_00 == 0x00)
		state->ram_3[offset] = data;
	else if (state->port_00 == 0x01)
		state->ram_4[offset] = data;
	else
		printf("unk rb2_w\n");
}

static READ8_HANDLER( discoboy_ram_att_r )
{
	discoboy_state *state = (discoboy_state *)space->machine->driver_data;
	return state->ram_att[offset];
}

static WRITE8_HANDLER( discoboy_ram_att_w )
{
	discoboy_state *state = (discoboy_state *)space->machine->driver_data;
	state->ram_att[offset] = data;
}

static ADDRESS_MAP_START( discoboy_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xc7ff) AM_READWRITE(rambank_r, rambank_w)
	AM_RANGE(0xc800, 0xcfff) AM_READWRITE(discoboy_ram_att_r, discoboy_ram_att_w)
	AM_RANGE(0xd000, 0xdfff) AM_READWRITE(rambank2_r, rambank2_w)
	AM_RANGE(0xe000, 0xefff) AM_RAM
	AM_RANGE(0xf000, 0xffff) AM_RAM
ADDRESS_MAP_END


static READ8_HANDLER( discoboy_port_06_r )
{
	return 0x00;
}

static ADDRESS_MAP_START( io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("DSWA") AM_WRITE(discoboy_port_00_w)
	AM_RANGE(0x01, 0x01) AM_READ_PORT("SYSTEM") AM_WRITE(discoboy_port_01_w)
	AM_RANGE(0x02, 0x02) AM_READ_PORT("P1")
	AM_RANGE(0x03, 0x03) AM_READ_PORT("P2") AM_WRITE(discoboy_port_03_w)
	AM_RANGE(0x04, 0x04) AM_READ_PORT("DSWB")
	AM_RANGE(0x06, 0x06) AM_READWRITE(discoboy_port_06_r, discoboy_port_06_w) // ???
	AM_RANGE(0x07, 0x07) AM_WRITE(rambank_select_w) // 0x20 is palette bank bit.. others?
ADDRESS_MAP_END

/* Sound */

//static WRITE8_HANDLER( splash_adpcm_data_w ){
//  state->adpcm_data = data;
//}

static void splash_msm5205_int( running_device *device )
{
	discoboy_state *state = (discoboy_state *)device->machine->driver_data;
	msm5205_data_w(device, state->adpcm_data >> 4);
//  state->adpcm_data = (state->adpcm_data << 4) & 0xf0;
}

static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xf000, 0xf7ff) AM_RAM
	AM_RANGE(0xec00, 0xec01) AM_DEVWRITE("ymsnd", ym3812_w)
	AM_RANGE(0xf800, 0xf800) AM_READ(soundlatch_r)
ADDRESS_MAP_END


static INPUT_PORTS_START( discoboy )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SWA:6,7,8")
	PORT_DIPSETTING(	0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(	0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(	0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(	0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(	0x02, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(	0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(	0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(	0x04, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SWA:5")
	PORT_DIPSETTING(	0x08, "Every 150000" )
	PORT_DIPSETTING(	0x00, "Every 300000" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Lives ) ) PORT_DIPLOCATION("SWA:4")
	PORT_DIPSETTING(	0x10, "3" )
	PORT_DIPSETTING(	0x00, "4" )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SWA:2,3")
	PORT_DIPSETTING(	0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(	0x60, DEF_STR( Normal ) )
	PORT_DIPSETTING(	0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(	0x20, DEF_STR( Hardest ) )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_LOW, "SWA:1" )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)

	PORT_START("DSWB")
	PORT_DIPUNUSED_DIPLOC( 0x01, IP_ACTIVE_LOW, "SWB:8" )
	PORT_DIPUNUSED_DIPLOC( 0x02, IP_ACTIVE_LOW, "SWB:7" )
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "SWB:6" )
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SWB:5" )
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_LOW, "SWB:4" )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SWB:3" )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SWB:2" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(	0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout tiles8x8_layout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+4,RGN_FRAC(1,2),  RGN_FRAC(0,2)+4,RGN_FRAC(0,2) },
	{ 0, 1, 2, 3, 8, 9, 10, 11, 256, 257, 258, 259, 264,265,266,267  },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16, 8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	16*32
};

static const gfx_layout tiles8x8_layout2 =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(1,4), RGN_FRAC(0,4),RGN_FRAC(3,4),RGN_FRAC(2,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( discoboy )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0x000, 128 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x8_layout2, 0x000, 128 )
GFXDECODE_END



static const msm5205_interface discoboy_msm5205_interface =
{
	splash_msm5205_int,	/* IRQ handler */
	MSM5205_S48_4B		/* ??? unknown hz */
};


static MACHINE_START( discoboy )
{
	discoboy_state *state = (discoboy_state *)machine->driver_data;

	state->audiocpu = machine->device("audiocpu");

	state_save_register_global(machine, state->ram_bank);
	state_save_register_global(machine, state->port_00);
	state_save_register_global(machine, state->gfxbank);
	state_save_register_global(machine, state->adpcm_data);
}

static MACHINE_RESET( discoboy )
{
	discoboy_state *state = (discoboy_state *)machine->driver_data;

	state->ram_bank = 0;
	state->port_00 = 0;
	state->gfxbank = 0;
	state->adpcm_data = 0x80;
}

static MACHINE_DRIVER_START( discoboy )

	/* driver data */
	MDRV_DRIVER_DATA(discoboy_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80,12000000/2)		 /* 6 MHz? */
	MDRV_CPU_PROGRAM_MAP(discoboy_map)
	MDRV_CPU_IO_MAP(io_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_CPU_ADD("audiocpu", Z80,10000000/2)		 /* 5 MHz? */
	MDRV_CPU_PROGRAM_MAP(sound_map)
//  MDRV_CPU_IO_MAP(sound_io_map)
//  MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)
	MDRV_CPU_VBLANK_INT_HACK(nmi_line_pulse,32)

	MDRV_MACHINE_START( discoboy )
	MDRV_MACHINE_RESET( discoboy )

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_SIZE(512, 256)
	MDRV_SCREEN_VISIBLE_AREA(8*8, 512-1-8*8, 0+8, 256-1-8)

	MDRV_GFXDECODE(discoboy)
	MDRV_PALETTE_LENGTH(0x1000)

	MDRV_VIDEO_START(discoboy)
	MDRV_VIDEO_UPDATE(discoboy)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ymsnd", YM3812, 2500000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.60)


	MDRV_SOUND_ADD("msm", MSM5205, 384000) // ???? unknown
	MDRV_SOUND_CONFIG(discoboy_msm5205_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)
MACHINE_DRIVER_END


ROM_START( discoboy )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "u2",  0x00000, 0x10000, CRC(44a4fefa) SHA1(29b74bb739afffb7baefb5ed4da09cdb1559b011) )
	ROM_LOAD( "u18", 0x10000, 0x20000, CRC(88d1282d) SHA1(1f11dad0f577198c54a1dc182ba7502e398b998f) )

	ROM_REGION( 0x20000, "audiocpu", 0 )
	ROM_LOAD( "2.u28",  0x00000, 0x10000, CRC(7c2ed174) SHA1(ace209dc4cc7a4ffca062842defd84cefc5b10d2))
	ROM_LOAD( "1.u45",  0x10000, 0x10000, CRC(c266c6df) SHA1(f76e38ded43f56a486cf6569c679ddb57a4165fb) )

	ROM_REGION( 0x100000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "5.u94",   0x00000, 0x10000, CRC(dbd20836) SHA1(d97651626b1dc16b93f8aed28bac19fd177e626f) )
	ROM_LOAD( "6.u124",  0x10000, 0x40000, CRC(e20d41f8) SHA1(792294a34840867072bc484d6f3cae3502c8bc28) )
	ROM_LOAD( "7.u95",   0x80000, 0x10000, CRC(1d5617a2) SHA1(6b6bd50c1984748dc8bf6600431d9bb6fe443873) )
	ROM_LOAD( "8.u125",  0x90000, 0x40000, CRC(30be1340) SHA1(e4765b75c8f774c6f7f7b5496a50c33ee3950550) )

	ROM_REGION( 0x100000, "gfx2", ROMREGION_INVERT )
	ROM_LOAD( "u80",   0x00000, 0x10000, CRC(4cc642ae) SHA1(2a59ebc8ab27bf7c3c1aa389ea32fb01d5cfdce8) )
	ROM_LOAD( "u50",   0x10000, 0x20000, CRC(1557ca92) SHA1(5a0afbeede6f0ae1c75bdec446132c673aeb0fe7) )
	ROM_LOAD( "u81",   0x40000, 0x10000, CRC(9e04274e) SHA1(70c28212b242335353e6dd48b7eb176146bec457) )
	ROM_LOAD( "u5",    0x50000, 0x20000, CRC(a07df669) SHA1(7f09b2508b9bffed7a4cd191f707af3c0c2a1de2) )
	ROM_LOAD( "u78",   0x80000, 0x10000, CRC(04571f70) SHA1(afdc7d84f7804c2ced413d13e6985a05f841e79e) )
	ROM_LOAD( "u46",   0x90000, 0x20000, CRC(764ffde4) SHA1(637df403a6ac73456892add3f2403a92afb67f19) )
	ROM_LOAD( "u79",   0xc0000, 0x10000, CRC(646f0f83) SHA1(d5cd050872d4b8c2fc89c3c0f434b1d66e5f1c59) )
	ROM_LOAD( "u49",   0xd0000, 0x20000, CRC(0b6c0d8d) SHA1(820a12c84af4fd5a04e1eca3cbace0002d3024b6) )
ROM_END


static DRIVER_INIT( discoboy )
{
	discoboy_state *state = (discoboy_state *)machine->driver_data;
	UINT8 *ROM = memory_region(machine, "maincpu");

	state->ram_1 = auto_alloc_array(machine, UINT8, 0x800);
	state->ram_2 = auto_alloc_array(machine, UINT8, 0x800);
	state->ram_att = auto_alloc_array(machine, UINT8, 0x800);

	state->ram_3 = auto_alloc_array(machine, UINT8, 0x1000);
	state->ram_4 = auto_alloc_array(machine, UINT8, 0x1000);

	memset(state->ram_1, 0, 0x800);
	memset(state->ram_2, 0, 0x800);
	memset(state->ram_att,0, 0x800);
	memset(state->ram_3, 0, 0x1000);
	memset(state->ram_4, 0, 0x1000);

	state_save_register_global_pointer(machine, state->ram_1, 0x800);
	state_save_register_global_pointer(machine, state->ram_2, 0x800);
	state_save_register_global_pointer(machine, state->ram_att, 0x800);
	state_save_register_global_pointer(machine, state->ram_3, 0x1000);
	state_save_register_global_pointer(machine, state->ram_4, 0x1000);

	memory_configure_bank(machine, "bank1", 0, 8, &ROM[0x10000], 0x4000);
	memory_set_bank(machine, "bank1", 0);
}


GAME( 1993, discoboy,  0,    discoboy, discoboy, discoboy, ROT270, "Soft Art Co.", "Disco Boy", GAME_SUPPORTS_SAVE )
