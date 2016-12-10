/***************************************************************************

Real Mahjong Haihai                (c)1985 Alba
Real Mahjong Haihai Jinji Idou Hen (c)1986 Alba
Real Mahjong Haihai Seichouhen     (c)1986 Visco

CPU:    Z80
Sound:  AY-3-8910
        M5205
OSC:    20.000MHz

driver by Nicola Salmoria

TODO:
- input handling is not well understood... it might well be handled by a
  protection device. I think it is, because rmhaijin and rmhaisei do additional
  checks which are obfuscated in a way that would make sense only for
  protection (in rmhaisei the failure is more explicit, in rmhaijin it's
  deviously delayed to a later part of the game).
  In themj the checks are patched out, maybe it's a bootleg?

- there probably is an area of NVRAM, because credits don't go away after
  a reset.

- some unknown reads and writes.

- visible area uncertain.

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "sound/msm5205.h"

static int gfxbank;
static UINT8 *videoram;
static UINT8 *colorram;
static tilemap_t *bg_tilemap;

static WRITE8_HANDLER( rmhaihai_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

static WRITE8_HANDLER( rmhaihai_colorram_w )
{
	colorram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

static TILE_GET_INFO( get_bg_tile_info )
{
	int attr = colorram[tile_index];
	int code = videoram[tile_index] + (gfxbank << 12) + ((attr & 0x07) << 8) + ((attr & 0x80) << 4);
	int color = (gfxbank << 5) + (attr >> 3);

	SET_TILE_INFO(0, code, color, 0);
}

static VIDEO_START( rmhaihai )
{
	bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows,
		8, 8, 64, 32);
}

static VIDEO_UPDATE( rmhaihai )
{
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);
	return 0;
}



static int keyboard_cmd;

static READ8_HANDLER( keyboard_r )
{
	static const char *const keynames[] = { "KEY0", "KEY1" };

	logerror("%04x: keyboard_r\n",cpu_get_pc(space->cpu));
	switch(cpu_get_pc(space->cpu))
	{
		/* read keyboard */
		case 0x0aba:	// rmhaihai, rmhaisei
		case 0x0b2a:	// rmhaihib
		case 0x0ab4:	// rmhaijin
		case 0x0aea:	// themj
		{
			int i;

			for (i = 0; i < 31; i++)
			{
				if (input_port_read(space->machine, keynames[i/16]) & (1 << (i & 15))) return i+1;
			}
			if (input_port_read(space->machine, "KEY1") & 0x8000) return 0x80;	// coin
			return 0;
		}
		case 0x5c7b:	// rmhaihai, rmhaisei, rmhaijin
		case 0x5950:	// rmhaihib
		case 0x5bf3:	// themj, but the test is NOPed out!
			return 0xcc;	/* keyboard_cmd = 0xcb */


		case 0x13a:	// additional checks done by rmhaijin
			if (keyboard_cmd == 0x3b) return 0xdd;
			if (keyboard_cmd == 0x85) return 0xdc;
			if (keyboard_cmd == 0xf2) return 0xd6;
			if (keyboard_cmd == 0xc1) return 0x8f;
			if (keyboard_cmd == 0xd0) return 0x08;
			return 0;

		case 0x140:	// additional checks done by rmhaisei
		case 0x155:	// additional checks done by themj, but they are patched out!
			if (keyboard_cmd == 0x11) return 0x57;
			if (keyboard_cmd == 0x3e) return 0xda;
			if (keyboard_cmd == 0x48) return 0x74;
			if (keyboard_cmd == 0x5d) return 0x46;
			if (keyboard_cmd == 0xd0) return 0x08;
			return 0;
	}

	/* there are many more reads whose function is unknown, returning 0 seems fine */
	return 0;
}

static WRITE8_HANDLER( keyboard_w )
{
logerror("%04x: keyboard_w %02x\n",cpu_get_pc(space->cpu),data);
	keyboard_cmd = data;
}

static READ8_HANDLER( samples_r )
{
	return memory_region(space->machine, "adpcm")[offset];
}

static WRITE8_DEVICE_HANDLER( adpcm_w )
{
	msm5205_data_w(device,data);         /* bit0..3  */
	msm5205_reset_w(device,(data>>5)&1); /* bit 5    */
	msm5205_vclk_w (device,(data>>4)&1); /* bit4     */
}

static WRITE8_HANDLER( ctrl_w )
{
	flip_screen_set(space->machine, data & 0x01);

	// (data & 0x02) is switched on and off in service mode

	coin_lockout_w(space->machine, 0, ~data & 0x04);
	coin_counter_w(space->machine, 0, data & 0x08);

	// (data & 0x10) is medal in service mode

	gfxbank = (data & 0x40) >> 6;	/* rmhaisei only */
}

static WRITE8_HANDLER( themj_rombank_w )
{
	UINT8 *rom = memory_region(space->machine, "maincpu") + 0x10000;
	int bank = data & 0x03;
logerror("banksw %d\n",bank);
	memory_set_bankptr(space->machine, "bank1", rom + bank*0x4000);
	memory_set_bankptr(space->machine, "bank2", rom + bank*0x4000 + 0x2000);
}

static MACHINE_RESET( themj )
{
	themj_rombank_w(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_IO), 0, 0);
}



static ADDRESS_MAP_START( rmhaihai_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x9fff) AM_ROM
	AM_RANGE(0xa000, 0xa7ff) AM_RAM
	AM_RANGE(0xa800, 0xafff) AM_RAM_WRITE(rmhaihai_colorram_w) AM_BASE(&colorram)
	AM_RANGE(0xb000, 0xb7ff) AM_RAM_WRITE(rmhaihai_videoram_w) AM_BASE(&videoram)
	AM_RANGE(0xb83c, 0xb83c) AM_WRITENOP	// ??
	AM_RANGE(0xbc00, 0xbc00) AM_WRITENOP	// ??
	AM_RANGE(0xc000, 0xdfff) AM_ROM
	AM_RANGE(0xe000, 0xffff) AM_ROM			/* rmhaisei only */
ADDRESS_MAP_END

static ADDRESS_MAP_START( rmhaihai_io_map, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_READ(samples_r)
	AM_RANGE(0x8000, 0x8000) AM_READ(keyboard_r) AM_WRITENOP	// ??
	AM_RANGE(0x8001, 0x8001) AM_READNOP AM_WRITE(keyboard_w)	// ??
	AM_RANGE(0x8020, 0x8020) AM_DEVREAD("aysnd", ay8910_r)
	AM_RANGE(0x8020, 0x8021) AM_DEVWRITE("aysnd", ay8910_address_data_w)
	AM_RANGE(0x8040, 0x8040) AM_DEVWRITE("msm", adpcm_w)
	AM_RANGE(0x8060, 0x8060) AM_WRITE(ctrl_w)
	AM_RANGE(0x8080, 0x8080) AM_WRITENOP	// ??
	AM_RANGE(0xbc04, 0xbc04) AM_WRITENOP	// ??
	AM_RANGE(0xbc0c, 0xbc0c) AM_WRITENOP	// ??
ADDRESS_MAP_END

static ADDRESS_MAP_START( themj_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x9fff) AM_ROMBANK("bank1")
	AM_RANGE(0xa000, 0xa7ff) AM_RAM
	AM_RANGE(0xa800, 0xafff) AM_RAM_WRITE(rmhaihai_colorram_w) AM_BASE(&colorram)
	AM_RANGE(0xb000, 0xb7ff) AM_RAM_WRITE(rmhaihai_videoram_w) AM_BASE(&videoram)
	AM_RANGE(0xc000, 0xdfff) AM_ROMBANK("bank2")
	AM_RANGE(0xe000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( themj_io_map, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_READ(samples_r)
	AM_RANGE(0x8000, 0x8000) AM_READ(keyboard_r) AM_WRITENOP	// ??
	AM_RANGE(0x8001, 0x8001) AM_READNOP AM_WRITE(keyboard_w)	// ??
	AM_RANGE(0x8020, 0x8020) AM_DEVREAD("aysnd", ay8910_r)
	AM_RANGE(0x8020, 0x8021) AM_DEVWRITE("aysnd", ay8910_address_data_w)
	AM_RANGE(0x8040, 0x8040) AM_DEVWRITE("msm", adpcm_w)
	AM_RANGE(0x8060, 0x8060) AM_WRITE(ctrl_w)
	AM_RANGE(0x8080, 0x8080) AM_WRITENOP	// ??
	AM_RANGE(0x80a0, 0x80a0) AM_WRITE(themj_rombank_w)
	AM_RANGE(0xbc04, 0xbc04) AM_WRITENOP	// ??
	AM_RANGE(0xbc0c, 0xbc0c) AM_WRITENOP	// ??
ADDRESS_MAP_END


static INPUT_PORTS_START( mjctrl )
	PORT_START("KEY0")		// fake, handled by keyboard_r()
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_MAHJONG_SMALL )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_MAHJONG_BIG )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_MAHJONG_SCORE )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_MAHJONG_K )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_MAHJONG_RON )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_MAHJONG_G )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_MAHJONG_CHI )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_MAHJONG_C )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_MAHJONG_L )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_MAHJONG_H )

	PORT_START("KEY1")	// fake, handled by keyboard_r()
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_MAHJONG_PON )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_MAHJONG_D )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_MAHJONG_I )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_MAHJONG_KAN )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_MAHJONG_E )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_MAHJONG_M )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_MAHJONG_A )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_MAHJONG_BET )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_MAHJONG_J )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_MAHJONG_REACH )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_MAHJONG_F )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_MAHJONG_N )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_MAHJONG_B )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1)

	PORT_START("KEY2")	// fake, handled by keyboard_r()
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_MAHJONG_SMALL ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_MAHJONG_DOUBLE_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_MAHJONG_BIG ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_MAHJONG_SCORE ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_MAHJONG_LAST_CHANCE ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_MAHJONG_K ) PORT_PLAYER(2)
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_MAHJONG_RON ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_MAHJONG_G ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_MAHJONG_CHI ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_MAHJONG_C ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_MAHJONG_L ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_MAHJONG_H ) PORT_PLAYER(2)


	PORT_START("KEY3")	// fake, handled by keyboard_r()
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_MAHJONG_PON ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_MAHJONG_D ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_MAHJONG_I ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_MAHJONG_KAN ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_MAHJONG_E ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_MAHJONG_M ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_MAHJONG_A ) PORT_PLAYER(2)
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_MAHJONG_BET ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_MAHJONG_J ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_MAHJONG_REACH ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_MAHJONG_F ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_MAHJONG_N ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_MAHJONG_B ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(1)
INPUT_PORTS_END

static INPUT_PORTS_START( rmhaihai )
	PORT_START("DSW2")  /* dsw2 */
	PORT_DIPNAME( 0x01, 0x01, "Unknown 2-1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xfe, 0xfe, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0xfe, "1 (Easy)" )
	PORT_DIPSETTING(    0x7e, "2" )
	PORT_DIPSETTING(    0xbe, "3" )
	PORT_DIPSETTING(    0xde, "4" )
	PORT_DIPSETTING(    0xee, "5" )
	PORT_DIPSETTING(    0xf6, "6" )
	PORT_DIPSETTING(    0xfa, "7" )
	PORT_DIPSETTING(    0xfc, "8 (Difficult)" )

    PORT_START("DSW1")  /* dsw1 */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x02, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, "A 2/1 B 1/2" )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, "A 1/2 B 2/1" )
	PORT_DIPSETTING(    0x08, "A 1/3 B 3/1" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, "Medal" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_INCLUDE( mjctrl )
INPUT_PORTS_END

static INPUT_PORTS_START( rmhaihib )
	PORT_START("DSW2")  /* dsw2 */
	PORT_DIPNAME( 0x01, 0x01, "Unknown 2-1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 2-2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Gal Bonus Bet" )
	PORT_DIPSETTING(    0x04, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x18, 0x18, "Gal Bonus" )
	PORT_DIPSETTING(    0x18, "8" )
	PORT_DIPSETTING(    0x10, "16" )
	PORT_DIPSETTING(    0x08, "24" )
	PORT_DIPSETTING(    0x00, "32" )
	PORT_DIPNAME( 0xe0, 0xe0, "Pay Setting" )
	PORT_DIPSETTING(    0xe0, "90%" )
	PORT_DIPSETTING(    0xc0, "80%" )
	PORT_DIPSETTING(    0xa0, "70%" )
	PORT_DIPSETTING(    0x80, "60%" )
	PORT_DIPSETTING(    0x60, "50%" )
	PORT_DIPSETTING(    0x40, "40%" )
	PORT_DIPSETTING(    0x20, "30%" )
	PORT_DIPSETTING(    0x00, "20%" )

	PORT_START("DSW1")  /* dsw1 */
	PORT_DIPNAME( 0x03, 0x03, "Bet Max" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "10" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x04, "A 1/1 B 1/10" )
	PORT_DIPSETTING(    0x08, "A 1/1 B 1/5" )
	PORT_DIPSETTING(    0x00, "A 1/1 B 5/1" )
	PORT_DIPSETTING(    0x0c, "A 1/1 B 10/1" )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 1-5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 1-8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_INCLUDE( mjctrl )

//  PORT_START("EXTRA") // 11
//  PORT_BIT(    0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Pay Out") PORT_CODE(KEYCODE_3)
//  PORT_BIT(     0x02, IP_ACTIVE_LOW, IPT_SERVICE4 ) /* RAM clear */
//  PORT_SERVICE( 0x04, IP_ACTIVE_LOW )
//  PORT_BIT(     0x08, IP_ACTIVE_LOW, IPT_SERVICE2 ) /* Analyzer */
//  PORT_BIT(     0xF0, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	3,
	{ RGN_FRAC(1,2)+4, 0, 4 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8
};

static GFXDECODE_START( rmhaihai )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 32 )
GFXDECODE_END

static GFXDECODE_START( themj )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 64 )
GFXDECODE_END



static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_INPUT_PORT("DSW2"),
	DEVCB_INPUT_PORT("DSW1")
};

static const msm5205_interface msm5205_config =
{
	0,				/* interrupt function */
	MSM5205_SEX_4B	/* vclk input mode    */
};



static MACHINE_DRIVER_START( rmhaihai )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu",Z80,20000000/4)	/* 5 MHz ??? */
	MDRV_CPU_PROGRAM_MAP(rmhaihai_map)
	MDRV_CPU_IO_MAP(rmhaihai_io_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(4*8, 60*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(rmhaihai)
	MDRV_PALETTE_LENGTH(0x100)

	MDRV_PALETTE_INIT(RRRR_GGGG_BBBB)
	MDRV_VIDEO_START(rmhaihai)
	MDRV_VIDEO_UPDATE(rmhaihai)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("aysnd", AY8910, 20000000/16)
	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	MDRV_SOUND_ADD("msm", MSM5205, 500000)
	MDRV_SOUND_CONFIG(msm5205_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( rmhaisei )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(rmhaihai)

	/* video hardware */
	MDRV_GFXDECODE(themj)
	MDRV_PALETTE_LENGTH(0x200)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( themj )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(rmhaihai)

	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(themj_map)
	MDRV_CPU_IO_MAP(themj_io_map)

	MDRV_MACHINE_RESET(themj)

	/* video hardware */
	MDRV_GFXDECODE(themj)
	MDRV_PALETTE_LENGTH(0x200)
MACHINE_DRIVER_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( rmhaihai )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "s3-6.11g",     0x00000, 0x2000, CRC(e7af7ba2) SHA1(1b0f87a16006a96e5b59e055966addac3e2ca926) )
	ROM_CONTINUE(             0x06000, 0x2000 )
	ROM_LOAD( "s3-4.8g",      0x04000, 0x2000, CRC(f849e75c) SHA1(4636bcaa7cddb9bc012212098a25f3c57cfc6b51) )
	ROM_CONTINUE(             0x02000, 0x2000 )
	ROM_LOAD( "s3-2.6g",      0x08000, 0x2000, CRC(d614532b) SHA1(99911c679ff6f990ae493bfc0b71a2fff0ef1796) )
	ROM_CONTINUE(             0x0c000, 0x2000 )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "s0-10.8a",     0x00000, 0x4000, CRC(797c63d1) SHA1(2ff9c3c61b28c34de97c0117b7eadb409d79df46) )
	ROM_LOAD( "s0-9.7a",      0x04000, 0x4000, CRC(b2526747) SHA1(73d0a19a5bb83e8977e94a47abbb65f9c7788c78) )
	ROM_LOAD( "s0-8.6a",      0x08000, 0x4000, CRC(146eaa31) SHA1(0e38aab52ff9bf0d42fea24caeee6ca90d63ace2) )
	ROM_LOAD( "s1-7.5a",      0x0c000, 0x4000, CRC(be59e742) SHA1(19d253f72f760f6350f76b313cf8aca7e3f90e8d) )
	ROM_LOAD( "s0-12.11a",    0x10000, 0x4000, CRC(e4229389) SHA1(b14d7855b66fe03c1485cb735cb20f59f19f248f) )
	ROM_LOAD( "s1-11.10a",    0x14000, 0x4000, CRC(029ef909) SHA1(fd867b8e1ccd5b88f18409ff17939ec8420c6131) )
	/* 0x18000-0x1ffff empty space filled by the init function */

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "s2.13b",       0x0000, 0x0100, CRC(911d32a5) SHA1(36f2b62009918862c13f3eda05a21403b4d9607f) )
	ROM_LOAD( "s1.13a",       0x0100, 0x0100, CRC(e9be978a) SHA1(50c7ca7a7496cb6fe5e8ce0db693ccb82dbbb8c6) )
	ROM_LOAD( "s3.13c",       0x0200, 0x0100, CRC(609775a6) SHA1(70a787aec0852e106216a4ca9891d36aef60b189) )

	ROM_REGION( 0x8000, "adpcm", 0 )	/* ADPCM samples, read directly by the main CPU */
	ROM_LOAD( "s0-1.5g",      0x00000, 0x8000, CRC(65e55b7e) SHA1(3852fb3b37eccdcddff05d8ef4a742fcb8b63473) )
ROM_END

ROM_START( rmhaihib )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "s-30-6.11g",   0x00000,  0x2000, CRC(f3e13cc8) SHA1(7eb9b17ea9efb5b2891ec40a9ff9744e84c0511c) )
	ROM_CONTINUE(             0x06000,  0x2000 )
	ROM_LOAD( "s-30-4.8g",    0x04000,  0x2000, CRC(f6642584) SHA1(5160baf267fd5dd8385ea5a9ff82e9c220fee342) )
	ROM_CONTINUE(             0x02000,  0x2000 )
	ROM_LOAD( "s-30-2.6g",    0x08000,  0x2000, CRC(e5959703) SHA1(15552d90296d0b6790642f554d08e79e827a16ee) )
	ROM_CONTINUE(             0x0c000,  0x2000 )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "s0-10.8a",     0x00000, 0x4000, CRC(797c63d1) SHA1(2ff9c3c61b28c34de97c0117b7eadb409d79df46) )
	ROM_LOAD( "s0-9.7a",      0x04000, 0x4000, CRC(b2526747) SHA1(73d0a19a5bb83e8977e94a47abbb65f9c7788c78) )
	ROM_LOAD( "s0-8.6a",      0x08000, 0x4000, CRC(146eaa31) SHA1(0e38aab52ff9bf0d42fea24caeee6ca90d63ace2) )
	ROM_LOAD( "s1-7.5a",      0x0c000, 0x4000, CRC(be59e742) SHA1(19d253f72f760f6350f76b313cf8aca7e3f90e8d) )
	ROM_LOAD( "s0-12.11a",    0x10000, 0x4000, CRC(e4229389) SHA1(b14d7855b66fe03c1485cb735cb20f59f19f248f) )
	ROM_LOAD( "s1-11.10a",    0x14000, 0x4000, CRC(029ef909) SHA1(fd867b8e1ccd5b88f18409ff17939ec8420c6131) )
	/* 0x18000-0x1ffff empty space filled by the init function */

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "s2.13b",       0x0000, 0x0100, CRC(911d32a5) SHA1(36f2b62009918862c13f3eda05a21403b4d9607f) )
	ROM_LOAD( "s1.13a",       0x0100, 0x0100, CRC(e9be978a) SHA1(50c7ca7a7496cb6fe5e8ce0db693ccb82dbbb8c6) )
	ROM_LOAD( "s3.13c",       0x0200, 0x0100, CRC(609775a6) SHA1(70a787aec0852e106216a4ca9891d36aef60b189) )

	ROM_REGION( 0x8000, "adpcm", 0 )	/* ADPCM samples, read directly by the main CPU */
	ROM_LOAD( "s0-1.5g",      0x00000, 0x8000, CRC(65e55b7e) SHA1(3852fb3b37eccdcddff05d8ef4a742fcb8b63473) )
ROM_END

ROM_START( rmhaijin )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "s-4-6.11g",    0x00000, 0x2000, CRC(474c9ace) SHA1(9161a5c64054f079d57676f3d7f61ca149018f61) )
	ROM_CONTINUE(             0x06000, 0x2000 )
	ROM_LOAD( "s-4-4.8g",     0x04000, 0x2000, CRC(c76ab584) SHA1(7d76fa6166108d6a511d5311c0d34b55364afec1) )
	ROM_CONTINUE(             0x02000, 0x2000 )
	ROM_LOAD( "s-4-2.6g",     0x08000, 0x2000, CRC(77b16f5b) SHA1(5e91b6b34ab8196a246c428b98f47a5b167dca76) )
	ROM_CONTINUE(             0x0c000, 0x2000 )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "s-1-10.8a",    0x00000, 0x4000, CRC(797c63d1) SHA1(2ff9c3c61b28c34de97c0117b7eadb409d79df46) )
	ROM_LOAD( "s-1-9.7a",     0x04000, 0x4000, CRC(5d3793d4) SHA1(43665d44ab2db42a28243c269ca451c90fe60abc) )
	ROM_LOAD( "s-1-8.6a",     0x08000, 0x4000, CRC(6fcd990b) SHA1(c7e35c6d9d75cd743d23a78de5dab63e034e33a8) )
	ROM_LOAD( "s-2-7.5a",     0x0c000, 0x4000, CRC(e92658bd) SHA1(db4b55bb10c38357729bb0f59a9ff66f4b81a220) )
	ROM_LOAD( "s-1-12.11a",   0x10000, 0x4000, CRC(7502a191) SHA1(e3543a2cf78d4046a580d972f68a4f10aa066144) )
	ROM_LOAD( "s-2-11.10a",   0x14000, 0x4000, CRC(9ebbc607) SHA1(8ab707f2a197772bae94e9129eb3f40d408c88bf) )
	/* 0x18000-0x1ffff empty space filled by the init function */

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "s5.13b",       0x0000, 0x0100, CRC(153aa7bf) SHA1(945db334e27be431a34670b2d94de639f67038d1) )
	ROM_LOAD( "s4.13a",       0x0100, 0x0100, CRC(5d643e6e) SHA1(df34be9d4cb0129069c2ed40c916c84674b62bb3) )
	ROM_LOAD( "s6.13c",       0x0200, 0x0100, CRC(fd6ff344) SHA1(cd00985f8bbff1ab5a149a00320d861ac8655bf8) )

	ROM_REGION( 0x8000, "adpcm", 0 )	/* ADPCM samples, read directly by the main CPU */
	ROM_LOAD( "s-0-1.5g",     0x00000, 0x8000, CRC(65e55b7e) SHA1(3852fb3b37eccdcddff05d8ef4a742fcb8b63473) )
ROM_END

ROM_START( rmhaisei )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sei-11.h11",   0x00000, 0x2000, CRC(7c35692b) SHA1(8890ca90ae84c63bfd2b4857bbdd02bd9a2f29a9) )
	ROM_CONTINUE(             0x06000, 0x2000 )
	ROM_LOAD( "sei-10.h8",    0x04000, 0x2000, CRC(cbd58124) SHA1(562eb13c2dc441294b1b7dafe37ac27a9b7bba2b) )
	ROM_CONTINUE(             0x02000, 0x2000 )
	ROM_LOAD( "sei-8.h6",     0x08000, 0x2000, CRC(8c8dc2fd) SHA1(7744ff7d4ad6888256c43a33dfd7f5c0d5be5815) )
	ROM_CONTINUE(             0x0c000, 0x2000 )
	ROM_LOAD( "sei-9.h7",     0x0e000, 0x2000, CRC(9132368d) SHA1(ca0924399cdd1554fc0407719c74d492743db156) )

	ROM_REGION( 0x40000, "gfx1", 0 )
	ROM_LOAD( "sei-4.a8",     0x00000, 0x8000, CRC(6a0234bf) SHA1(ad6642aa6fca84a22625265a7c82f50e307ba2f9) )
	ROM_LOAD( "sei-3.a7",     0x08000, 0x8000, CRC(c48bc39f) SHA1(de5aca9f72b437b7e7559bbd4b22c1b3ab70e450) )
	ROM_LOAD( "sei-2.a6",     0x10000, 0x8000, CRC(e479ba47) SHA1(b2bda054cd70181e223fe33d63924b029d196676) )
	ROM_LOAD( "sei-1.a5",     0x18000, 0x8000, CRC(fe6555f8) SHA1(b3201f465f9e897ec5805512e3ff488ef77f2f25) )
	ROM_LOAD( "sei-6.a11",    0x20000, 0x8000, CRC(86f1b462) SHA1(ccabbdca44840de5f9b8f6af24117e545b8f1ef7) )
	ROM_LOAD( "sei-5.a9",     0x28000, 0x8000, CRC(8bf780bc) SHA1(5ef72ee3f45f1cdde06131797faf26a9776f6a13) )
	/* 0x30000-0x3ffff empty space filled by the init function */

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "2.bpr",        0x0000, 0x0200, CRC(9ad2afcd) SHA1(6cd4cd5f693ee882a98598e8f86ee2baf3b105bf) )
	ROM_LOAD( "1.bpr",        0x0200, 0x0200, CRC(9b036f82) SHA1(4b14084e5a6674e69bd4bbc3a483c277bfc73808) )
	ROM_LOAD( "3.bpr",        0x0400, 0x0200, CRC(0fa1a50a) SHA1(9e8a2c9554a61bfdacb434f8c22c1085b1c93aa1) )

	ROM_REGION( 0x8000, "adpcm", 0 )	/* ADPCM samples, read directly by the main CPU */
	ROM_LOAD( "sei-7.h5",     0x00000, 0x8000, CRC(3e412c1a) SHA1(bc5e324ea26b8dd1e37c4e8b0d7ba712c1222bc7) )
ROM_END

ROM_START( themj )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* CPU */
	ROM_LOAD( "t7.bin",       0x00000,  0x02000, CRC(a58563c3) SHA1(53faeb66606214eb97ef8ff9affe68705e18a0b3) )
	ROM_CONTINUE(             0x06000,  0x02000 )
	ROM_LOAD( "t8.bin",       0x04000,  0x02000, CRC(bdf29475) SHA1(6296561da9c3a299d69bba8a98362c40b677ea9a) )
	ROM_CONTINUE(             0x02000,  0x02000 )
	ROM_LOAD( "t9.bin",       0x0e000,  0x02000, CRC(d5537d03) SHA1(ba27e83fcc9b6962373e2f723fc681481ec76864) )
	ROM_LOAD( "no1.bin",      0x10000,  0x10000, CRC(a67dd977) SHA1(835648c5df51053c883d90d7309e53232b945ceb) ) /* banked */

	ROM_REGION( 0x40000, "gfx1", 0 ) /* gfx */
	ROM_LOAD( "t3.bin",       0x00000,  0x8000, CRC(f0735c62) SHA1(5ff0da7fc72512797ec59ee57467fa81abcfdb8b) )
	ROM_LOAD( "t4.bin",       0x08000,  0x8000, CRC(952227fa) SHA1(7c2b5fe18bbaa482d93ab99a8f886838b596df8d) )
	ROM_LOAD( "t5.bin",       0x10000,  0x8000, CRC(3deea9b4) SHA1(e445b545a8d293f6a5724e6c484cb1062c631bcc) )
	ROM_LOAD( "t6.bin",       0x18000,  0x8000, CRC(47717958) SHA1(b25a9bd72bf5aa024ce2631440bb2ad762544e54) )
	ROM_LOAD( "t1.bin",       0x20000,  0x8000, CRC(9b9a458e) SHA1(91146bd3ed7ed016c90ae5c3e3510d0d8d216ba5) )
	ROM_LOAD( "t2.bin",       0x28000,  0x8000, CRC(4702375f) SHA1(9e824007e3e26ad6fb2ccbbcf35aa7cfdf5c469e) )
	/* 0x30000-0x3ffff empty space filled by the init function */

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "5.bin",        0x0000,  0x0200, CRC(062fb055) SHA1(20a6d236e3ab1df8c471cccca31ec05442595c82) )
	ROM_LOAD( "4.bin",        0x0200,  0x0200, CRC(9f81a6d7) SHA1(2735815c0c922d0c81559d792fcaa39bd9615536) )
	ROM_LOAD( "6.bin",        0x0400,  0x0200, CRC(61373ec7) SHA1(73861914aae29e3996f9991f324c358a29c46969) )

	ROM_REGION( 0x8000, "adpcm", 0 )	/* ADPCM samples, read directly by the main CPU */
	ROM_LOAD( "t0.bin",       0x00000,  0x8000, CRC(3e412c1a) SHA1(bc5e324ea26b8dd1e37c4e8b0d7ba712c1222bc7) )
ROM_END


static DRIVER_INIT( rmhaihai )
{
	UINT8 *rom = memory_region(machine, "gfx1");
	int size = memory_region_length(machine, "gfx1");
	int a,b;

	size /= 2;
	rom += size;

	/* unpack the high bit of gfx */
	for (b = size - 0x4000;b >= 0;b -= 0x4000)
	{
		if (b) memcpy(rom + b,rom + b/2,0x2000);

		for (a = 0;a < 0x2000;a++)
		{
			rom[a + b + 0x2000] = rom[a + b] >> 4;
		}
	}
}


GAME( 1985, rmhaihai, 0,        rmhaihai, rmhaihai, rmhaihai, ROT0, "Alba",  "Real Mahjong Haihai (Japan)", 0 )
GAME( 1985, rmhaihib, rmhaihai, rmhaihai, rmhaihib, rmhaihai, ROT0, "Alba",  "Real Mahjong Haihai [BET] (Japan)", 0 )
GAME( 1986, rmhaijin, 0,        rmhaihai, rmhaihai, rmhaihai, ROT0, "Alba",  "Real Mahjong Haihai Jinji Idou Hen (Japan)", 0 )
GAME( 1986, rmhaisei, 0,        rmhaisei, rmhaihai, rmhaihai, ROT0, "Visco", "Real Mahjong Haihai Seichouhen (Japan)", 0 )
GAME( 1987, themj,    0,        themj,    rmhaihai, rmhaihai, ROT0, "Visco", "The Mah-jong (Japan)", 0 )
