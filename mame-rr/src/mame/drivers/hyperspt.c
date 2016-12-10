/***************************************************************************

Based on drivers from Juno First emulator by Chris Hardy (chrish@kcbbs.gen.nz)

- DIPs verified and defaults (when avaialble) are set from manuals

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m6800/m6800.h"
#include "cpu/m6809/m6809.h"
#include "sound/dac.h"
#include "sound/sn76496.h"
#include "sound/vlm5030.h"
#include "machine/konami1.h"
#include "includes/konamipt.h"
#include "includes/trackfld.h"


static WRITE8_HANDLER( hyperspt_coin_counter_w )
{
	coin_counter_w(space->machine, offset, data);
}


static ADDRESS_MAP_START( hyperspt_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x1000, 0x10bf) AM_RAM AM_BASE_SIZE_MEMBER(trackfld_state, spriteram, spriteram_size)
	AM_RANGE(0x10c0, 0x10ff) AM_RAM AM_BASE_MEMBER(trackfld_state, scroll)	/* Scroll amount */
	AM_RANGE(0x1400, 0x1400) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x1480, 0x1480) AM_WRITE(hyperspt_flipscreen_w)
	AM_RANGE(0x1481, 0x1481) AM_WRITE(konami_sh_irqtrigger_w)  /* cause interrupt on audio CPU */
	AM_RANGE(0x1483, 0x1484) AM_WRITE(hyperspt_coin_counter_w)
	AM_RANGE(0x1487, 0x1487) AM_WRITE(interrupt_enable_w)  /* Interrupt enable */
	AM_RANGE(0x1500, 0x1500) AM_WRITE(soundlatch_w)
	AM_RANGE(0x1600, 0x1600) AM_READ_PORT("DSW2")
	AM_RANGE(0x1680, 0x1680) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x1681, 0x1681) AM_READ_PORT("P1_P2")
	AM_RANGE(0x1682, 0x1682) AM_READ_PORT("P3_P4")
	AM_RANGE(0x1683, 0x1683) AM_READ_PORT("DSW1")
	AM_RANGE(0x2000, 0x27ff) AM_RAM_WRITE(hyperspt_videoram_w) AM_BASE_MEMBER(trackfld_state, videoram)
	AM_RANGE(0x2800, 0x2fff) AM_RAM_WRITE(hyperspt_colorram_w) AM_BASE_MEMBER(trackfld_state, colorram)
	AM_RANGE(0x3000, 0x37ff) AM_RAM
	AM_RANGE(0x3800, 0x3fff) AM_RAM AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE(0x4000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( roadf_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x1000, 0x10bf) AM_RAM AM_BASE_SIZE_MEMBER(trackfld_state, spriteram, spriteram_size)
	AM_RANGE(0x10c0, 0x10ff) AM_RAM AM_BASE_MEMBER(trackfld_state, scroll)	/* Scroll amount */
	AM_RANGE(0x1400, 0x1400) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x1480, 0x1480) AM_WRITE(hyperspt_flipscreen_w)
	AM_RANGE(0x1481, 0x1481) AM_WRITE(konami_sh_irqtrigger_w)  /* cause interrupt on audio CPU */
	AM_RANGE(0x1483, 0x1484) AM_WRITE(hyperspt_coin_counter_w)
	AM_RANGE(0x1487, 0x1487) AM_WRITE(interrupt_enable_w)  /* Interrupt enable */
	AM_RANGE(0x1500, 0x1500) AM_WRITE(soundlatch_w)
	AM_RANGE(0x1600, 0x1600) AM_READ_PORT("DSW2")
	AM_RANGE(0x1680, 0x1680) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x1681, 0x1681) AM_READ_PORT("P1")
	AM_RANGE(0x1682, 0x1682) AM_READ_PORT("P2")
	AM_RANGE(0x1683, 0x1683) AM_READ_PORT("DSW1")
	AM_RANGE(0x2000, 0x27ff) AM_RAM_WRITE(hyperspt_videoram_w) AM_BASE_MEMBER(trackfld_state, videoram)
	AM_RANGE(0x2800, 0x2fff) AM_RAM_WRITE(hyperspt_colorram_w) AM_BASE_MEMBER(trackfld_state, colorram)
	AM_RANGE(0x3000, 0x37ff) AM_RAM
	AM_RANGE(0x3800, 0x3fff) AM_RAM AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE(0x4000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x4fff) AM_RAM
	AM_RANGE(0x6000, 0x6000) AM_READ(soundlatch_r)
	AM_RANGE(0x8000, 0x8000) AM_READ(hyperspt_sh_timer_r)
	AM_RANGE(0xa000, 0xa000) AM_DEVWRITE("vlm", vlm5030_data_w) /* speech data */
	AM_RANGE(0xc000, 0xdfff) AM_DEVWRITE("vlm", hyperspt_sound_w)	  /* speech and output control */
	AM_RANGE(0xe000, 0xe000) AM_DEVWRITE("dac", dac_w)
	AM_RANGE(0xe001, 0xe001) AM_WRITE(konami_SN76496_latch_w)  /* Loads the snd command into the snd latch */
	AM_RANGE(0xe002, 0xe002) AM_DEVWRITE("snsnd", konami_SN76496_w) 	 /* This address triggers the SN chip to read the data port. */
ADDRESS_MAP_END

static ADDRESS_MAP_START( soundb_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x4fff) AM_RAM
	AM_RANGE(0x6000, 0x6000) AM_READ(soundlatch_r)
	AM_RANGE(0x8000, 0x8000) AM_READ(hyperspt_sh_timer_r)
	AM_RANGE(0xa000, 0xa000) AM_NOP
	AM_RANGE(0xc000, 0xdfff) AM_WRITE(hyprolyb_adpcm_w)	  /* speech and output control */
	AM_RANGE(0xe000, 0xe000) AM_DEVWRITE("dac", dac_w)
	AM_RANGE(0xe001, 0xe001) AM_WRITE(konami_SN76496_latch_w)  /* Loads the snd command into the snd latch */
	AM_RANGE(0xe002, 0xe002) AM_DEVWRITE("snsnd", konami_SN76496_w) 	 /* This address triggers the SN chip to read the data port. */
ADDRESS_MAP_END

static INPUT_PORTS_START( hyperspt )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1_P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P3_P4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3  ) PORT_PLAYER(3) //PORT_COCKTAIL  These were commented out
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2  ) PORT_PLAYER(3) //PORT_COCKTAIL  before the system changed.
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1  ) PORT_PLAYER(3) //PORT_COCKTAIL  Why?
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3  ) PORT_PLAYER(4) //PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2  ) PORT_PLAYER(4) //PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1  ) PORT_PLAYER(4) //PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "No Coin B", SW1)
	/* "No Coin B" = coins produce sound, but no effect on coin counter */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "After Last Event" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(	0x01, "Game Over" )
	PORT_DIPSETTING(	0x00, "Game Continues" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(	0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(	0x02, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(	0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "World Records" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(	0x08, "Don't Erase" )
	PORT_DIPSETTING(	0x00, "Erase on Reset" )
	PORT_DIPNAME( 0xf0, 0x40, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:5,6,7,8")
	PORT_DIPSETTING(	0xf0, "Easy 1" )
	PORT_DIPSETTING(	0xe0, "Easy 2" )
	PORT_DIPSETTING(	0xd0, "Easy 3" )
	PORT_DIPSETTING(	0xc0, "Easy 4" )
	PORT_DIPSETTING(	0xb0, "Normal 1" )
	PORT_DIPSETTING(	0xa0, "Normal 2" )
	PORT_DIPSETTING(	0x90, "Normal 3" )
	PORT_DIPSETTING(	0x80, "Normal 4" )
	PORT_DIPSETTING(	0x70, "Normal 5" )
	PORT_DIPSETTING(	0x60, "Normal 6" )
	PORT_DIPSETTING(	0x50, "Normal 7" )
	PORT_DIPSETTING(	0x40, "Normal 8" )
	PORT_DIPSETTING(	0x30, "Difficult 1" )
	PORT_DIPSETTING(	0x20, "Difficult 2" )
	PORT_DIPSETTING(	0x10, "Difficult 3" )
	PORT_DIPSETTING(	0x00, "Difficult 4" )
INPUT_PORTS_END


static INPUT_PORTS_START( roadf )
	PORT_START("SYSTEM")
	KONAMI8_SYSTEM_UNK

	PORT_START("P1")
	KONAMI8_MONO_B12_UNK

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* the game doesn't boot if this is 1 */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "No Coin B", SW1)
	/* "No Coin B" = coins produce sound, but no effect on coin counter */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(	0x01, DEF_STR( No ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x06, 0x04, "Number of Opponents" ) PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(	0x06, "Few" )
	PORT_DIPSETTING(	0x04, DEF_STR( Normal ) )
	PORT_DIPSETTING(	0x02, "Many" )
	PORT_DIPSETTING(	0x00, "Great Many" )
	PORT_DIPNAME( 0x08, 0x08, "Speed of Opponents" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(	0x08, "Fast" )
	PORT_DIPSETTING(	0x00, "Slow" )
	PORT_DIPNAME( 0x30, 0x20, "Fuel Consumption" ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(	0x30, "Slow" )
	PORT_DIPSETTING(	0x20, DEF_STR( Normal ) )
	PORT_DIPSETTING(	0x10, "Fast" )
	PORT_DIPSETTING(	0x00, "Very Fast" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(	0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(	0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(	0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout hyperspt_charlayout =
{
	8,8,	/* 8*8 sprites */
	1024,	/* 1024 characters */
	4,	/* 4 bits per pixel */
	{ 0x4000*8+4, 0x4000*8+0, 4, 0	},
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8	/* every sprite takes 64 consecutive bytes */
};

static const gfx_layout hyperspt_spritelayout =
{
	16,16,	/* 16*16 sprites */
	512,	/* 512 sprites */
	4,	/* 4 bits per pixel */
	{ 0x8000*8+4, 0x8000*8+0, 4, 0 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 24*8+0, 24*8+1, 24*8+2, 24*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 ,
		32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8 },
	64*8	/* every sprite takes 64 consecutive bytes */
};

static GFXDECODE_START( hyperspt )
	GFXDECODE_ENTRY( "gfx1", 0, hyperspt_spritelayout,     0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, hyperspt_charlayout,	16*16, 16 )
GFXDECODE_END


static const gfx_layout roadf_charlayout =
{
	8,8,	/* 8*8 sprites */
	1536,	/* 1536 characters */
	4,	/* 4 bits per pixel */
	{ 0x6000*8+4, 0x6000*8+0, 4, 0	},
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8	/* every sprite takes 64 consecutive bytes */
};

static const gfx_layout roadf_spritelayout =
{
	16,16,	/* 16*16 sprites */
	256,	/* 256 sprites */
	4,	/* 4 bits per pixel */
	{ 0x4000*8+4, 0x4000*8+0, 4, 0 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 24*8+0, 24*8+1, 24*8+2, 24*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 ,
		32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8 },
	64*8	/* every sprite takes 64 consecutive bytes */
};

static GFXDECODE_START( roadf )
	GFXDECODE_ENTRY( "gfx1", 0, roadf_spritelayout,     0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, roadf_charlayout,	 16*16, 16 )
GFXDECODE_END



static MACHINE_START( hyperspt )
{
	trackfld_state *state = (trackfld_state *)machine->driver_data;

	state->audiocpu = machine->device<cpu_device>("audiocpu");
	state->vlm = machine->device("vlm");

	/* sound */
	state_save_register_global(machine, state->SN76496_latch);
	state_save_register_global(machine, state->last_addr);
	state_save_register_global(machine, state->last_irq);
}

static MACHINE_START( hypersptb )
{
	trackfld_state *state = (trackfld_state *)machine->driver_data;

	MACHINE_START_CALL(hyperspt);

	state_save_register_global(machine, state->hyprolyb_adpcm_ready);	// only bootlegs
	state_save_register_global(machine, state->hyprolyb_adpcm_busy);
	state_save_register_global(machine, state->hyprolyb_vck_ready);
}

static MACHINE_RESET( hyperspt )
{
	trackfld_state *state = (trackfld_state *)machine->driver_data;

	state->SN76496_latch = 0;
	state->last_addr = 0;
	state->last_irq = 0;
}

static MACHINE_RESET( hypersptb )
{
	trackfld_state *state = (trackfld_state *)machine->driver_data;

	MACHINE_RESET_CALL(hyperspt);

	state->hyprolyb_adpcm_ready = 0;
	state->hyprolyb_adpcm_busy = 0;
	state->hyprolyb_vck_ready = 0;
}

static MACHINE_DRIVER_START( hyperspt )

	/* driver data */
	MDRV_DRIVER_DATA(trackfld_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M6809, XTAL_18_432MHz/12)	/* verified on pcb */
	MDRV_CPU_PROGRAM_MAP(hyperspt_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_CPU_ADD("audiocpu", Z80,XTAL_14_31818MHz/4) /* verified on pcb */
	MDRV_CPU_PROGRAM_MAP(sound_map)

	MDRV_MACHINE_START(hyperspt)
	MDRV_MACHINE_RESET(hyperspt)
	MDRV_NVRAM_HANDLER(generic_0fill)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(hyperspt)
	MDRV_PALETTE_LENGTH(16*16+16*16)

	MDRV_PALETTE_INIT(hyperspt)
	MDRV_VIDEO_START(hyperspt)
	MDRV_VIDEO_UPDATE(hyperspt)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("dac", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MDRV_SOUND_ADD("snsnd", SN76496, XTAL_14_31818MHz/8) /* verified on pcb */
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MDRV_SOUND_ADD("vlm", VLM5030, XTAL_3_579545MHz) /* verified on pcb */
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( hypersptb )
	MDRV_IMPORT_FROM(hyperspt)
	MDRV_DEVICE_REMOVE("vlm")

	MDRV_CPU_MODIFY("audiocpu")
	MDRV_CPU_PROGRAM_MAP(soundb_map)

	MDRV_CPU_ADD("adpcm", M6802, XTAL_14_31818MHz/8)	/* unknown clock */
	MDRV_CPU_PROGRAM_MAP(hyprolyb_adpcm_map)

	MDRV_MACHINE_START(hypersptb)
	MDRV_MACHINE_RESET(hypersptb)

	MDRV_SOUND_ADD("msm", MSM5205, 384000)
	MDRV_SOUND_CONFIG(hyprolyb_msm5205_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( roadf )
	MDRV_IMPORT_FROM(hyperspt)

	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(roadf_map)
	MDRV_GFXDECODE(roadf)
	MDRV_VIDEO_START(roadf)
MACHINE_DRIVER_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( hyperspt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "c01",          0x4000, 0x2000, CRC(0c720eeb) SHA1(cc0719db7e59c72e603ab2ca42565303bc41d281) )
	ROM_LOAD( "c02",          0x6000, 0x2000, CRC(560258e0) SHA1(788d0d3cbbd97fb54eceb3281ccf84a31e5e3e98) )
	ROM_LOAD( "c03",          0x8000, 0x2000, CRC(9b01c7e6) SHA1(0106f94b38ad62e7514e56aab35581968074bbe0) )
	ROM_LOAD( "c04",          0xa000, 0x2000, CRC(10d7e9a2) SHA1(ebf1dd7ba10179c41b42358c45e49424ce8495cd) )
	ROM_LOAD( "c05",          0xc000, 0x2000, CRC(b105a8cd) SHA1(7d77ab4d75c0bff7ac7372a5ff5fe55839b57d19) )
	ROM_LOAD( "c06",          0xe000, 0x2000, CRC(1a34a849) SHA1(daa42a959ea162ca7f098010c85a7453a8805df8) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "c10",          0x0000, 0x2000, CRC(3dc1a6ff) SHA1(1e67cac46b6c8a9a0bb1560e135983435520f1fc) )
	ROM_LOAD( "c09",          0x2000, 0x2000, CRC(9b525c3e) SHA1(d8775ec3b4f12117431a2b7c7eaa038c1255241b) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "c14",          0x00000, 0x2000, CRC(c72d63be) SHA1(0677b4f7196551ebc1bbbecd0e15d79f8e32857d) )
	ROM_LOAD( "c13",          0x02000, 0x2000, CRC(76565608) SHA1(418fb9a81c0583d0214afb27fea28794563b8460) )
	ROM_LOAD( "c12",          0x04000, 0x2000, CRC(74d2cc69) SHA1(684b65455217f243b3690822d445efdcb18211bb) )
	ROM_LOAD( "c11",          0x06000, 0x2000, CRC(66cbcb4d) SHA1(c4ea51a6f30d2cd0cd6e22fdadb83d889f2cc471) )
	ROM_LOAD( "c18",          0x08000, 0x2000, CRC(ed25e669) SHA1(2e306db101cd4443b0a81cecf817e5ebbdaf1bba) )
	ROM_LOAD( "c17",          0x0a000, 0x2000, CRC(b145b39f) SHA1(e696e1f9b44aa44360ea9962c4ee9b61db8e53f5) )
	ROM_LOAD( "c16",          0x0c000, 0x2000, CRC(d7ff9f2b) SHA1(b0e6a056db96027ba0c10d3ee3bfdef145a236e2) )
	ROM_LOAD( "c15",          0x0e000, 0x2000, CRC(f3d454e6) SHA1(9d04dcd1b0354e01773923295bba2602e00467f9) )

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "c26",          0x00000, 0x2000, CRC(a6897eac) SHA1(a1dd950c29885f7bb4784fed46810ae47bff87dd) )
	ROM_LOAD( "c24",          0x02000, 0x2000, CRC(5fb230c0) SHA1(8caebf3788c1fb71c1ba72b0045503d45936d4ce) )
	ROM_LOAD( "c22",          0x04000, 0x2000, CRC(ed9271a0) SHA1(a458ad79922383f45f6522775e19cf693e226883) )
	ROM_LOAD( "c20",          0x06000, 0x2000, CRC(183f4324) SHA1(f6bcd03c25dea300876ace950f118a971557168f) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "c03_c27.bin",  0x0000, 0x0020, CRC(bc8a5956) SHA1(90746145d9f380c29919edea3ef7a8434c48c9d9) )
	ROM_LOAD( "j12_c28.bin",  0x0020, 0x0100, CRC(2c891d59) SHA1(79050fbe058c24349927edc7937ec68a77f450f1) )
	ROM_LOAD( "a09_c29.bin",  0x0120, 0x0100, CRC(811a3f3f) SHA1(474f03345847cd9791ff6b7161286bbfef3f990a) )

	ROM_REGION( 0x10000, "vlm", 0 )	/*  64k for speech rom    */
	ROM_LOAD( "c08",          0x0000, 0x2000, CRC(e8f8ea78) SHA1(8d37818e5a2740c96696f37996f2a3f870386690) )
ROM_END



ROM_START( hypersptb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.5g",          0x4000, 0x2000, CRC(0cfc68a7) SHA1(48ddd444f4366a39afb589e1c5df10d815b4b64f) )
	ROM_LOAD( "2.7g",          0x6000, 0x2000, CRC(560258e0) SHA1(788d0d3cbbd97fb54eceb3281ccf84a31e5e3e98) )
	ROM_LOAD( "3.8g",          0x8000, 0x2000, CRC(9b01c7e6) SHA1(0106f94b38ad62e7514e56aab35581968074bbe0) )
	ROM_LOAD( "4.11g",          0xa000, 0x2000, CRC(4ed32240) SHA1(b093763655ebfd00b08542b49eab4606ea1ef8c6) )
	ROM_LOAD( "5.13g",          0xc000, 0x2000, CRC(b105a8cd) SHA1(7d77ab4d75c0bff7ac7372a5ff5fe55839b57d19) )
	ROM_LOAD( "6.15g",          0xe000, 0x2000, CRC(1a34a849) SHA1(daa42a959ea162ca7f098010c85a7453a8805df8) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "12.17a",          0x0000, 0x2000, CRC(3dc1a6ff) SHA1(1e67cac46b6c8a9a0bb1560e135983435520f1fc) )
	ROM_LOAD( "11.15a",          0x2000, 0x2000, CRC(9b525c3e) SHA1(d8775ec3b4f12117431a2b7c7eaa038c1255241b) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "20.15g",          0x00000, 0x4000, CRC(551d222f) SHA1(cd3d544ca5009cad3f9748804301bfed57329775) ) // 27256 ?! - it doesn't need to be!
	ROM_CONTINUE(0xc000,0x4000) // causes data we want to be at 0xe000 - 0xffff
	ROM_LOAD( "16.19j",          0x00000, 0x2000, CRC(c72d63be) SHA1(0677b4f7196551ebc1bbbecd0e15d79f8e32857d) )
	ROM_LOAD( "15.18j",          0x02000, 0x2000, CRC(76565608) SHA1(418fb9a81c0583d0214afb27fea28794563b8460) )
	ROM_LOAD( "14.17j",          0x04000, 0x2000, CRC(74d2cc69) SHA1(684b65455217f243b3690822d445efdcb18211bb) )
	ROM_LOAD( "13.15j",          0x06000, 0x2000, CRC(66cbcb4d) SHA1(c4ea51a6f30d2cd0cd6e22fdadb83d889f2cc471) )
	ROM_LOAD( "17.19g",          0x08000, 0x2000, CRC(ed25e669) SHA1(2e306db101cd4443b0a81cecf817e5ebbdaf1bba) )
	ROM_LOAD( "18.18g",          0x0a000, 0x2000, CRC(b145b39f) SHA1(e696e1f9b44aa44360ea9962c4ee9b61db8e53f5) )
	ROM_LOAD( "19.17g",          0x0c000, 0x2000, CRC(d7ff9f2b) SHA1(b0e6a056db96027ba0c10d3ee3bfdef145a236e2) )


	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "b.14a",          0x00000, 0x4000, CRC(8fd90bd2) SHA1(4faa270002f859a27719d08004b012cc297405f5) )  // 27256 ?! - it doesn't need to be!
	ROM_CONTINUE(0x0000,0x4000)  // causes data we want to be at 0x0000 - 0x1fff
	ROM_LOAD( "a.12a",          0x02000, 0x2000, CRC(a3d422c6) SHA1(f0e26a7190e64acd9b682aab5a66223de4055de0) )
	ROM_LOAD( "d.14c",          0x04000, 0x2000, CRC(ed9271a0) SHA1(a458ad79922383f45f6522775e19cf693e226883) )
	ROM_LOAD( "c.12c",          0x06000, 0x2000, CRC(0c8ed053) SHA1(2bf5e4fd94cbf7d459495a144b86b677eb1f89da) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "mmi6331-1.3c",  0x0000, 0x0020, CRC(bc8a5956) SHA1(90746145d9f380c29919edea3ef7a8434c48c9d9) )
	// no proms matched the ones below, but they're colours, so I assume them to be the same
	ROM_LOAD( "j12_c28.bin",  0x0020, 0x0100, CRC(2c891d59) SHA1(79050fbe058c24349927edc7937ec68a77f450f1) )
	ROM_LOAD( "a09_c29.bin",  0x0120, 0x0100, CRC(811a3f3f) SHA1(474f03345847cd9791ff6b7161286bbfef3f990a) )

    /* These ROM's are located on the Sound Board */
	ROM_REGION( 0x10000, "adpcm", 0 )	/*  64k for the 6802 which plays ADPCM samples */
	ROM_LOAD( "10.20c",       0x8000, 0x2000, CRC(a4cddeb8) SHA1(057981ad3b04239662bb19342e9ec14b0dab2351) )
	ROM_LOAD( "9.20cd",       0xa000, 0x2000, CRC(e9919365) SHA1(bd11d6e3ee2c6e698159c2768e315389d666107f) )
	ROM_LOAD( "8.20d",        0xc000, 0x2000, CRC(49a06454) SHA1(159a293125d7ac92a81120e290497ee7ed6d8acf) )
	ROM_LOAD( "7.20b",        0xe000, 0x2000, CRC(607a36df) SHA1(17e553e5070771133ed094f05b26dd6cd63cfc23) )

ROM_END

ROM_START( hpolym84 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "c01",          0x4000, 0x2000, CRC(0c720eeb) SHA1(cc0719db7e59c72e603ab2ca42565303bc41d281) )
	ROM_LOAD( "c02",          0x6000, 0x2000, CRC(560258e0) SHA1(788d0d3cbbd97fb54eceb3281ccf84a31e5e3e98) )
	ROM_LOAD( "c03",          0x8000, 0x2000, CRC(9b01c7e6) SHA1(0106f94b38ad62e7514e56aab35581968074bbe0) )
	ROM_LOAD( "330e04.bin",   0xa000, 0x2000, CRC(9c5e2934) SHA1(7d25e53ca54f6b382785888838acff27bc2c1d43) )
	ROM_LOAD( "c05",          0xc000, 0x2000, CRC(b105a8cd) SHA1(7d77ab4d75c0bff7ac7372a5ff5fe55839b57d19) )
	ROM_LOAD( "c06",          0xe000, 0x2000, CRC(1a34a849) SHA1(daa42a959ea162ca7f098010c85a7453a8805df8) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "c10",          0x0000, 0x2000, CRC(3dc1a6ff) SHA1(1e67cac46b6c8a9a0bb1560e135983435520f1fc) )
	ROM_LOAD( "c09",          0x2000, 0x2000, CRC(9b525c3e) SHA1(d8775ec3b4f12117431a2b7c7eaa038c1255241b) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "c14",          0x00000, 0x2000, CRC(c72d63be) SHA1(0677b4f7196551ebc1bbbecd0e15d79f8e32857d) )
	ROM_LOAD( "c13",          0x02000, 0x2000, CRC(76565608) SHA1(418fb9a81c0583d0214afb27fea28794563b8460) )
	ROM_LOAD( "c12",          0x04000, 0x2000, CRC(74d2cc69) SHA1(684b65455217f243b3690822d445efdcb18211bb) )
	ROM_LOAD( "c11",          0x06000, 0x2000, CRC(66cbcb4d) SHA1(c4ea51a6f30d2cd0cd6e22fdadb83d889f2cc471) )
	ROM_LOAD( "c18",          0x08000, 0x2000, CRC(ed25e669) SHA1(2e306db101cd4443b0a81cecf817e5ebbdaf1bba) )
	ROM_LOAD( "c17",          0x0a000, 0x2000, CRC(b145b39f) SHA1(e696e1f9b44aa44360ea9962c4ee9b61db8e53f5) )
	ROM_LOAD( "c16",          0x0c000, 0x2000, CRC(d7ff9f2b) SHA1(b0e6a056db96027ba0c10d3ee3bfdef145a236e2) )
	ROM_LOAD( "c15",          0x0e000, 0x2000, CRC(f3d454e6) SHA1(9d04dcd1b0354e01773923295bba2602e00467f9) )

	ROM_REGION( 0x08000, "gfx2", 0 )
	ROM_LOAD( "c26",          0x00000, 0x2000, CRC(a6897eac) SHA1(a1dd950c29885f7bb4784fed46810ae47bff87dd) )
	ROM_LOAD( "330e24.bin",   0x02000, 0x2000, CRC(f9bbfe1d) SHA1(f24a0c3e10e727e3e9fd123cda8bb557af1fea12) )
	ROM_LOAD( "c22",          0x04000, 0x2000, CRC(ed9271a0) SHA1(a458ad79922383f45f6522775e19cf693e226883) )
	ROM_LOAD( "330e20.bin",   0x06000, 0x2000, CRC(29969b92) SHA1(baf394c56b8a2855f32b9e6d7346faf50e75bcf2) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "c03_c27.bin",  0x0000, 0x0020, CRC(bc8a5956) SHA1(90746145d9f380c29919edea3ef7a8434c48c9d9) )
	ROM_LOAD( "j12_c28.bin",  0x0020, 0x0100, CRC(2c891d59) SHA1(79050fbe058c24349927edc7937ec68a77f450f1) )
	ROM_LOAD( "a09_c29.bin",  0x0120, 0x0100, CRC(811a3f3f) SHA1(474f03345847cd9791ff6b7161286bbfef3f990a) )

	ROM_REGION( 0x10000, "vlm", 0 )	/*  64k for speech rom    */
	ROM_LOAD( "c08",          0x0000, 0x2000, CRC(e8f8ea78) SHA1(8d37818e5a2740c96696f37996f2a3f870386690) )
ROM_END

ROM_START( roadf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "g05_g01.bin",  0x4000, 0x2000, CRC(e2492a06) SHA1(e03895b83f1529dd7bb20e1380cb60c7606db3e4) )
	ROM_LOAD( "g07_f02.bin",  0x6000, 0x2000, CRC(0bf75165) SHA1(d3d16d63ca15c8f6b05c37b4e37e41785334ffff) )
	ROM_LOAD( "g09_g03.bin",  0x8000, 0x2000, CRC(dde401f8) SHA1(aa1810290c14d15d14e2f82a6780fc82d06d437b) )
	ROM_LOAD( "g11_f04.bin",  0xA000, 0x2000, CRC(b1283c77) SHA1(3fdd8d97cdd8a0b7c12db6797ed17f730425f337) )
	ROM_LOAD( "g13_f05.bin",  0xC000, 0x2000, CRC(0ad4d796) SHA1(44335c769341b3e10bb92556c0718884fd4b5d20) )
	ROM_LOAD( "g15_f06.bin",  0xE000, 0x2000, CRC(fa42e0ed) SHA1(408d365183fd95e54695a17abbba87d729546d7c) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a17_d10.bin",  0x0000, 0x2000, CRC(c33c927e) SHA1(f1a8522e3bfc3a07bb42408d2937a4129e4c3fee) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "j19_e14.bin",  0x00000, 0x4000, CRC(16d2bcff) SHA1(37c63faaaca43909bfb1e2ccb370efe4b276d8a9) )
	ROM_LOAD( "g19_e18.bin",  0x04000, 0x4000, CRC(490685ff) SHA1(5ca0aa3771d60688671aae196f10f9feecb15106) )

	ROM_REGION( 0x0c000, "gfx2", 0 )
	ROM_LOAD( "a14_e26.bin",  0x00000, 0x4000, CRC(f5c738e2) SHA1(9f10be775791dee9801b1167f838a9110084842d) )
	ROM_LOAD( "a12_d24.bin",  0x04000, 0x2000, CRC(2d82c930) SHA1(fea26c00ad3acb1f44a5fdc79a7dd8ddce17d317) )
	ROM_LOAD( "c14_e22.bin",  0x06000, 0x4000, CRC(fbcfbeb9) SHA1(e5a938fc2fe2378d836dfe8ba516994cd5cf0bb5) )
	ROM_LOAD( "c12_d20.bin",  0x0a000, 0x2000, CRC(5e0cf994) SHA1(c81274d809c685ccf24108f56a4fa54146d4f493) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "c03_c27.bin",  0x0000, 0x0020, CRC(45d5e352) SHA1(0f4d358aaffcb68193247090e82f093752730518) )
	ROM_LOAD( "j12_c28.bin",  0x0020, 0x0100, CRC(2955e01f) SHA1(b0652d177a45571edc5978143d4023e7b173b383) )
	ROM_LOAD( "a09_c29.bin",  0x0120, 0x0100, CRC(5b3b5f2a) SHA1(e83556fba6d50ad20dff6e19bd300ba0c30cc6e2) )
ROM_END

ROM_START( roadf2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "5g",           0x4000, 0x2000, CRC(d8070d30) SHA1(334e4586686c29d33c3281cc446c13d2d96301dd) )
	ROM_LOAD( "6g",           0x6000, 0x2000, CRC(8b661672) SHA1(bdc983d1ad88372ea1fc8263d4c254d26079ece7) )
	ROM_LOAD( "8g",           0x8000, 0x2000, CRC(714929e8) SHA1(0176e4199a091485af30e00777678e51664dee23) )
	ROM_LOAD( "11g",          0xA000, 0x2000, CRC(0f2c6b94) SHA1(a18fe9021e464374de524454403eccc0aaf3eeb7) )
	ROM_LOAD( "g13_f05.bin",  0xC000, 0x2000, CRC(0ad4d796) SHA1(44335c769341b3e10bb92556c0718884fd4b5d20) )
	ROM_LOAD( "g15_f06.bin",  0xE000, 0x2000, CRC(fa42e0ed) SHA1(408d365183fd95e54695a17abbba87d729546d7c) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a17_d10.bin",  0x0000, 0x2000, CRC(c33c927e) SHA1(f1a8522e3bfc3a07bb42408d2937a4129e4c3fee) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "j19_e14.bin",  0x00000, 0x4000, CRC(16d2bcff) SHA1(37c63faaaca43909bfb1e2ccb370efe4b276d8a9) )
	ROM_LOAD( "g19_e18.bin",  0x04000, 0x4000, CRC(490685ff) SHA1(5ca0aa3771d60688671aae196f10f9feecb15106) )

	ROM_REGION( 0x0c000, "gfx2", 0 )
	ROM_LOAD( "a14_e26.bin",  0x00000, 0x4000, CRC(f5c738e2) SHA1(9f10be775791dee9801b1167f838a9110084842d) )
	ROM_LOAD( "a12_d24.bin",  0x04000, 0x2000, CRC(2d82c930) SHA1(fea26c00ad3acb1f44a5fdc79a7dd8ddce17d317) )
	ROM_LOAD( "c14_e22.bin",  0x06000, 0x4000, CRC(fbcfbeb9) SHA1(e5a938fc2fe2378d836dfe8ba516994cd5cf0bb5) )
	ROM_LOAD( "c12_d20.bin",  0x0a000, 0x2000, CRC(5e0cf994) SHA1(c81274d809c685ccf24108f56a4fa54146d4f493) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "c03_c27.bin",  0x0000, 0x0020, CRC(45d5e352) SHA1(0f4d358aaffcb68193247090e82f093752730518) )
	ROM_LOAD( "j12_c28.bin",  0x0020, 0x0100, CRC(2955e01f) SHA1(b0652d177a45571edc5978143d4023e7b173b383) )
	ROM_LOAD( "a09_c29.bin",  0x0120, 0x0100, CRC(5b3b5f2a) SHA1(e83556fba6d50ad20dff6e19bd300ba0c30cc6e2) )
ROM_END


static DRIVER_INIT( hyperspt )
{
	konami1_decode(machine, "maincpu");
}


GAME( 1984, hyperspt,  0,        hyperspt,  hyperspt, hyperspt, ROT0,  "Konami (Centuri license)", "Hyper Sports", GAME_SUPPORTS_SAVE )
GAME( 1984, hypersptb, hyperspt, hypersptb, hyperspt, hyperspt, ROT0,  "bootleg", "Hyper Sports (bootleg)", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE ) // has ADPCM vis MSM5205 instead of VLM
GAME( 1984, hpolym84,  hyperspt, hyperspt,  hyperspt, hyperspt, ROT0,  "Konami",  "Hyper Olympic '84", GAME_SUPPORTS_SAVE )
GAME( 1984, roadf,     0,        roadf,     roadf,    hyperspt, ROT90, "Konami",  "Road Fighter (set 1)", GAME_SUPPORTS_SAVE )
GAME( 1984, roadf2,    roadf,    roadf,     roadf,    hyperspt, ROT90, "Konami",  "Road Fighter (set 2)", GAME_SUPPORTS_SAVE )
