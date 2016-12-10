/***************************************************************************

Atari Poolshark Driver

***************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "includes/poolshrk.h"
#include "sound/discrete.h"


static int poolshrk_da_latch;


static DRIVER_INIT( poolshrk )
{
	UINT8* pSprite = memory_region(machine, "gfx1");
	UINT8* pOffset = memory_region(machine, "proms");

	int i;
	int j;

	/* re-arrange sprite data using the PROM */

	for (i = 0; i < 16; i++)
	{
		for (j = 0; j < 16; j++)
		{
			UINT16 v =
				(pSprite[0] << 0xC) |
				(pSprite[1] << 0x8) |
				(pSprite[2] << 0x4) |
				(pSprite[3] << 0x0);

			v >>= pOffset[j];

			pSprite[0] = (v >> 0xC) & 15;
			pSprite[1] = (v >> 0x8) & 15;
			pSprite[2] = (v >> 0x4) & 15;
			pSprite[3] = (v >> 0x0) & 15;

			pSprite += 4;
		}
	}
}


static WRITE8_HANDLER( poolshrk_da_latch_w )
{
	poolshrk_da_latch = data & 15;
}


static WRITE8_HANDLER( poolshrk_led_w )
{
	if (offset & 2)
		set_led_status(space->machine, 0, offset & 1);
	if (offset & 4)
		set_led_status(space->machine, 1, offset & 1);
}


static WRITE8_HANDLER( poolshrk_watchdog_w )
{
	if ((offset & 3) == 3)
	{
		watchdog_reset_w(space, 0, 0);
	}
}


static READ8_HANDLER( poolshrk_input_r )
{
	static const char *const portnames[] = { "IN0", "IN1", "IN2", "IN3" };
	UINT8 val = input_port_read(space->machine, portnames[offset & 3]);

	int x = input_port_read(space->machine, (offset & 1) ? "AN1" : "AN0");
	int y = input_port_read(space->machine, (offset & 1) ? "AN3" : "AN2");

	if (x >= poolshrk_da_latch) val |= 8;
	if (y >= poolshrk_da_latch) val |= 4;

	if ((offset & 3) == 3)
	{
		watchdog_reset_r(space, 0);
	}

	return val;
}


static READ8_HANDLER( poolshrk_irq_reset_r )
{
	cputag_set_input_line(space->machine, "maincpu", 0, CLEAR_LINE);

	return 0;
}


static ADDRESS_MAP_START( poolshrk_cpu_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x00ff) AM_MIRROR(0x2300) AM_RAM
	AM_RANGE(0x0400, 0x07ff) AM_MIRROR(0x2000) AM_WRITEONLY AM_BASE(&poolshrk_playfield_ram)
	AM_RANGE(0x0800, 0x080f) AM_MIRROR(0x23f0) AM_WRITEONLY AM_BASE(&poolshrk_hpos_ram)
	AM_RANGE(0x0c00, 0x0c0f) AM_MIRROR(0x23f0) AM_WRITEONLY AM_BASE(&poolshrk_vpos_ram)
	AM_RANGE(0x1000, 0x13ff) AM_MIRROR(0x2000) AM_READWRITE(poolshrk_input_r, poolshrk_watchdog_w)
	AM_RANGE(0x1400, 0x17ff) AM_MIRROR(0x2000) AM_DEVWRITE("discrete", poolshrk_scratch_sound_w)
	AM_RANGE(0x1800, 0x1bff) AM_MIRROR(0x2000) AM_DEVWRITE("discrete", poolshrk_score_sound_w)
	AM_RANGE(0x1c00, 0x1fff) AM_MIRROR(0x2000) AM_DEVWRITE("discrete", poolshrk_click_sound_w)
	AM_RANGE(0x4000, 0x4000) AM_NOP /* diagnostic ROM location */
	AM_RANGE(0x6000, 0x63ff) AM_WRITE(poolshrk_da_latch_w)
	AM_RANGE(0x6400, 0x67ff) AM_DEVWRITE("discrete", poolshrk_bump_sound_w)
	AM_RANGE(0x6800, 0x6bff) AM_READ(poolshrk_irq_reset_r)
	AM_RANGE(0x6c00, 0x6fff) AM_WRITE(poolshrk_led_w)
	AM_RANGE(0x7000, 0x7fff) AM_ROM
ADDRESS_MAP_END


static INPUT_PORTS_START( poolshrk )
	PORT_START("IN0")
	PORT_BIT( 0x0C, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x0C, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0x80, 0x00, "Extended Play" )
	PORT_DIPSETTING( 0x80, DEF_STR( Off ))
	PORT_DIPSETTING( 0x00, DEF_STR( On ))

	PORT_START("IN2")
	PORT_DIPNAME( 0x03, 0x02, "Racks Per Game" )
	PORT_DIPSETTING( 0x03, "2" )
	PORT_DIPSETTING( 0x02, "3" )
	PORT_DIPSETTING( 0x01, "4" )
	PORT_DIPSETTING( 0x00, "5" )
	PORT_BIT( 0x0C, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN3")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ))
	PORT_DIPSETTING( 0x00, DEF_STR( 2C_1C ))
	PORT_DIPSETTING( 0x03, DEF_STR( 1C_1C ))
	PORT_DIPSETTING( 0x02, DEF_STR( 1C_2C ))
	PORT_DIPSETTING( 0x01, DEF_STR( 1C_4C ))
	PORT_BIT( 0x0C, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("AN0")
	PORT_BIT( 15, 8, IPT_AD_STICK_X ) PORT_MINMAX(0,15) PORT_SENSITIVITY(25) PORT_KEYDELTA(1) PORT_PLAYER(1)

	PORT_START("AN1")
	PORT_BIT( 15, 8, IPT_AD_STICK_X ) PORT_MINMAX(0,15) PORT_SENSITIVITY(25) PORT_KEYDELTA(1) PORT_PLAYER(2)

	PORT_START("AN2")
	PORT_BIT( 15, 8, IPT_AD_STICK_Y ) PORT_MINMAX(0,15) PORT_SENSITIVITY(25) PORT_KEYDELTA(1) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("AN3")
	PORT_BIT( 15, 8, IPT_AD_STICK_Y ) PORT_MINMAX(0,15) PORT_SENSITIVITY(25) PORT_KEYDELTA(1) PORT_REVERSE PORT_PLAYER(2)

INPUT_PORTS_END


static const gfx_layout poolshrk_sprite_layout =
{
	16, 16,   /* width, height */
	16,       /* total         */
	1,        /* planes        */
	{ 0 },    /* plane offsets */
	{
		0x04, 0x05, 0x06, 0x07, 0x0C, 0x0D, 0x0E, 0x0F,
		0x14, 0x15, 0x16, 0x17, 0x1C, 0x1D, 0x1E, 0x1F
	},
	{
		0x000, 0x020, 0x040, 0x060, 0x080, 0x0A0, 0x0C0, 0x0E0,
		0x100, 0x120, 0x140, 0x160, 0x180, 0x1A0, 0x1C0, 0x1E0
	},
	0x200     /* increment */
};


static const gfx_layout poolshrk_tile_layout =
{
	8, 8,     /* width, height */
	64,       /* total         */
	1,        /* planes        */
	{ 0 },    /* plane offsets */
	{
		7, 6, 5, 4, 3, 2, 1, 0
	},
	{
		0x000, 0x200, 0x400, 0x600, 0x800, 0xA00, 0xC00, 0xE00
	},
	0x8       /* increment */
};


static GFXDECODE_START( poolshrk )
	GFXDECODE_ENTRY( "gfx1", 0, poolshrk_sprite_layout, 0, 2 )
	GFXDECODE_ENTRY( "gfx2", 0, poolshrk_tile_layout, 0, 1 )
GFXDECODE_END


static PALETTE_INIT( poolshrk )
{
	palette_set_color(machine,0,MAKE_RGB(0x7F, 0x7F, 0x7F));
	palette_set_color(machine,1,MAKE_RGB(0xFF, 0xFF, 0xFF));
	palette_set_color(machine,2,MAKE_RGB(0x7F, 0x7F, 0x7F));
	palette_set_color(machine,3,MAKE_RGB(0x00, 0x00, 0x00));
}


static MACHINE_DRIVER_START( poolshrk )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M6800, 11055000 / 8) /* ? */
	MDRV_CPU_PROGRAM_MAP(poolshrk_cpu_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_assert)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(1, 255, 24, 255)

	MDRV_GFXDECODE(poolshrk)
	MDRV_PALETTE_LENGTH(4)
	MDRV_PALETTE_INIT(poolshrk)
	MDRV_VIDEO_START(poolshrk)
	MDRV_VIDEO_UPDATE(poolshrk)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("discrete", DISCRETE, 0)
	MDRV_SOUND_CONFIG_DISCRETE(poolshrk)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


ROM_START( poolshrk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "7329.k1", 0x7000, 0x800, CRC(88152245) SHA1(c7c5e43ea488a197e92a1dc2231578f8ed86c98d) )
	ROM_LOAD( "7330.l1", 0x7800, 0x800, CRC(fb41d3e9) SHA1(c17994179362da13acfcd36a28f45e328428c031) )

	ROM_REGION( 0x400, "gfx1", 0 )   /* sprites */
	ROM_LOAD( "7325.j5", 0x0000, 0x200, CRC(fae87eed) SHA1(8891d0ea60f72f826d71dc6b064a2ba81b298914) )
	ROM_LOAD( "7326.h5", 0x0200, 0x200, CRC(05ec9762) SHA1(6119c4529334c98a0a42ca13a98a8661fc594d80) )

	ROM_REGION( 0x200, "gfx2", 0 )   /* tiles */
	ROM_LOAD( "7328.n6", 0x0000, 0x200, CRC(64bcbf3a) SHA1(a4e3ce6b4734234359e3ef784a771e40580c2a2a) )

	ROM_REGION( 0x20, "proms", 0 )                   /* line offsets */
	ROM_LOAD( "7327.k6", 0x0000, 0x020, CRC(f74cef5b) SHA1(f470bf5b193dae4b44e89bc4c4476cf8d98e7cfd) )
ROM_END


GAME( 1977, poolshrk, 0, poolshrk, poolshrk, poolshrk, 0, "Atari", "Poolshark", 0 )
