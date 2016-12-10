/***************************************************************************

    Parodius (Konami GX955) (c) 1990 Konami

    driver by Nicola Salmoria

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/konami/konami.h" /* for the callback and the firq irq definition */
#include "video/konicdev.h"
#include "sound/2151intf.h"
#include "sound/k053260.h"
#include "includes/konamipt.h"
#include "includes/parodius.h"

/* prototypes */
static KONAMI_SETLINES_CALLBACK( parodius_banking );


static INTERRUPT_GEN( parodius_interrupt )
{
	parodius_state *state = (parodius_state *)device->machine->driver_data;
	if (k052109_is_irq_enabled(state->k052109))
		cpu_set_input_line(device, 0, HOLD_LINE);
}

static READ8_HANDLER( bankedram_r )
{
	parodius_state *state = (parodius_state *)space->machine->driver_data;

	if (state->videobank & 0x01)
	{
		if (state->videobank & 0x04)
			return space->machine->generic.paletteram.u8[offset + 0x0800];
		else
			return space->machine->generic.paletteram.u8[offset];
	}
	else
		return state->ram[offset];
}

static WRITE8_HANDLER( bankedram_w )
{
	parodius_state *state = (parodius_state *)space->machine->driver_data;

	if (state->videobank & 0x01)
	{
		if (state->videobank & 0x04)
			paletteram_xBBBBBGGGGGRRRRR_be_w(space, offset + 0x0800, data);
		else
			paletteram_xBBBBBGGGGGRRRRR_be_w(space, offset, data);
	}
	else
		state->ram[offset] = data;
}

static READ8_HANDLER( parodius_052109_053245_r )
{
	parodius_state *state = (parodius_state *)space->machine->driver_data;

	if (state->videobank & 0x02)
		return k053245_r(state->k053245, offset);
	else
		return k052109_r(state->k052109, offset);
}

static WRITE8_HANDLER( parodius_052109_053245_w )
{
	parodius_state *state = (parodius_state *)space->machine->driver_data;

	if (state->videobank & 0x02)
		k053245_w(state->k053245, offset, data);
	else
		k052109_w(state->k052109, offset, data);
}

static WRITE8_HANDLER( parodius_videobank_w )
{
	parodius_state *state = (parodius_state *)space->machine->driver_data;

	if (state->videobank & 0xf8)
		logerror("%04x: videobank = %02x\n",cpu_get_pc(space->cpu),data);

	/* bit 0 = select palette or work RAM at 0000-07ff */
	/* bit 1 = select 052109 or 053245 at 2000-27ff */
	/* bit 2 = select palette bank 0 or 1 */
	state->videobank = data;
}

static WRITE8_HANDLER( parodius_3fc0_w )
{
	parodius_state *state = (parodius_state *)space->machine->driver_data;

	if ((data & 0xf4) != 0x10)
		logerror("%04x: 3fc0 = %02x\n",cpu_get_pc(space->cpu),data);

	/* bit 0/1 = coin counters */
	coin_counter_w(space->machine, 0, data & 0x01);
	coin_counter_w(space->machine, 1, data & 0x02);

	/* bit 3 = enable char ROM reading through the video RAM */
	k052109_set_rmrd_line(state->k052109, (data & 0x08) ? ASSERT_LINE : CLEAR_LINE);

	/* other bits unknown */
}

static READ8_DEVICE_HANDLER( parodius_sound_r )
{
	return k053260_r(device, 2 + offset);
}

static WRITE8_HANDLER( parodius_sh_irqtrigger_w )
{
	parodius_state *state = (parodius_state *)space->machine->driver_data;
	cpu_set_input_line_and_vector(state->audiocpu, 0, HOLD_LINE, 0xff);
}

#if 0

static void sound_nmi_callback( running_machine *machine, int param )
{
	parodius_state *state = (parodius_state *)machine->driver_data;
	cpu_set_input_line(state->audiocpu, INPUT_LINE_NMI, ( state->nmi_enabled ) ? CLEAR_LINE : ASSERT_LINE );

	nmi_enabled = 0;
}
#endif

static TIMER_CALLBACK( nmi_callback )
{
	parodius_state *state = (parodius_state *)machine->driver_data;
	cpu_set_input_line(state->audiocpu, INPUT_LINE_NMI, ASSERT_LINE);
}

static WRITE8_HANDLER( sound_arm_nmi_w )
{
	parodius_state *state = (parodius_state *)space->machine->driver_data;

	cpu_set_input_line(state->audiocpu, INPUT_LINE_NMI, CLEAR_LINE);
	timer_set(space->machine, ATTOTIME_IN_USEC(50), NULL, 0, nmi_callback);	/* kludge until the K053260 is emulated correctly */
}

/********************************************/

static ADDRESS_MAP_START( parodius_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_READWRITE(bankedram_r, bankedram_w) AM_BASE_MEMBER(parodius_state, ram)
	AM_RANGE(0x0800, 0x1fff) AM_RAM
	AM_RANGE(0x3f8c, 0x3f8c) AM_READ_PORT("P1")
	AM_RANGE(0x3f8d, 0x3f8d) AM_READ_PORT("P2")
	AM_RANGE(0x3f8e, 0x3f8e) AM_READ_PORT("DSW3")
	AM_RANGE(0x3f8f, 0x3f8f) AM_READ_PORT("DSW1")
	AM_RANGE(0x3f90, 0x3f90) AM_READ_PORT("DSW2")
	AM_RANGE(0x3fa0, 0x3faf) AM_DEVREADWRITE("k053245", k053244_r, k053244_w)
	AM_RANGE(0x3fb0, 0x3fbf) AM_DEVWRITE("k053251", k053251_w)
	AM_RANGE(0x3fc0, 0x3fc0) AM_READWRITE(watchdog_reset_r,parodius_3fc0_w)
	AM_RANGE(0x3fc4, 0x3fc4) AM_WRITE(parodius_videobank_w)
	AM_RANGE(0x3fc8, 0x3fc8) AM_WRITE(parodius_sh_irqtrigger_w)
	AM_RANGE(0x3fcc, 0x3fcd) AM_DEVREADWRITE("k053260", parodius_sound_r, k053260_w)	/* K053260 */
	AM_RANGE(0x2000, 0x27ff) AM_READWRITE(parodius_052109_053245_r, parodius_052109_053245_w)
	AM_RANGE(0x2000, 0x5fff) AM_DEVREADWRITE("k052109", k052109_r, k052109_w)
	AM_RANGE(0x6000, 0x9fff) AM_ROMBANK("bank1")			/* banked ROM */
	AM_RANGE(0xa000, 0xffff) AM_ROM					/* ROM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( parodius_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xefff) AM_ROM
	AM_RANGE(0xf000, 0xf7ff) AM_RAM
	AM_RANGE(0xf800, 0xf801) AM_DEVREADWRITE("ymsnd", ym2151_r,ym2151_w)
	AM_RANGE(0xfa00, 0xfa00) AM_WRITE(sound_arm_nmi_w)
	AM_RANGE(0xfc00, 0xfc2f) AM_DEVREADWRITE("k053260", k053260_r,k053260_w)
ADDRESS_MAP_END


/***************************************************************************

    Input Ports

***************************************************************************/

static INPUT_PORTS_START( parodius )
	PORT_START("P1")
	KONAMI8_ALT_B123(1)						// button1 = power-up, button2 = shoot, button3 = missile

	PORT_START("P2")
	KONAMI8_ALT_B123(2)

	PORT_START("DSW1")
	KONAMI_COINAGE(DEF_STR( Free_Play ), "No Coin B")
	/* "No Coin B" = coins produce sound, but no effect on coin counter */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x18, "20000 80000" )
	PORT_DIPSETTING(    0x10, "30000 100000" )
	PORT_DIPSETTING(    0x08, "20000" )
	PORT_DIPSETTING(    0x00, "70000" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Upright Controls" )
	PORT_DIPSETTING(    0x20, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Dual ) )
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END



/***************************************************************************

    Machine Driver

***************************************************************************/

static const k052109_interface parodius_k052109_intf =
{
	"gfx1", 0,
	NORMAL_PLANE_ORDER,
	KONAMI_ROM_DEINTERLEAVE_2,
	parodius_tile_callback
};

static const k05324x_interface parodius_k05324x_intf =
{
	"gfx2", 1,
	NORMAL_PLANE_ORDER,
	0, 0,
	KONAMI_ROM_DEINTERLEAVE_2,
	parodius_sprite_callback
};

static MACHINE_START( parodius )
{
	parodius_state *state = (parodius_state *)machine->driver_data;
	UINT8 *ROM = memory_region(machine, "maincpu");

	memory_configure_bank(machine, "bank1", 0, 14, &ROM[0x10000], 0x4000);
	memory_configure_bank(machine, "bank1", 14, 2, &ROM[0x08000], 0x4000);
	memory_set_bank(machine, "bank1", 0);

	machine->generic.paletteram.u8 = auto_alloc_array_clear(machine, UINT8, 0x1000);

	state->maincpu = machine->device("maincpu");
	state->audiocpu = machine->device("audiocpu");
	state->k053260 = machine->device("k053260");
	state->k053245 = machine->device("k053245");
	state->k053251 = machine->device("k053251");
	state->k052109 = machine->device("k052109");

	state_save_register_global(machine, state->videobank);
	state_save_register_global(machine, state->sprite_colorbase);
	state_save_register_global_array(machine, state->layer_colorbase);
	state_save_register_global_array(machine, state->layerpri);
	state_save_register_global_pointer(machine, machine->generic.paletteram.u8, 0x1000);
}

static MACHINE_RESET( parodius )
{
	parodius_state *state = (parodius_state *)machine->driver_data;
	int i;

	konami_configure_set_lines(machine->device("maincpu"), parodius_banking);

	for (i = 0; i < 3; i++)
	{
		state->layerpri[i] = 0;
		state->layer_colorbase[i] = 0;
	}

	state->sprite_colorbase = 0;
	state->videobank = 0;
}

static MACHINE_DRIVER_START( parodius )

	/* driver data */
	MDRV_DRIVER_DATA(parodius_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", KONAMI, 3000000)		/* 053248 */
	MDRV_CPU_PROGRAM_MAP(parodius_map)
	MDRV_CPU_VBLANK_INT("screen", parodius_interrupt)

	MDRV_CPU_ADD("audiocpu", Z80, 3579545)
	MDRV_CPU_PROGRAM_MAP(parodius_sound_map)
								/* NMIs are triggered by the 053260 */
	MDRV_MACHINE_START(parodius)
	MDRV_MACHINE_RESET(parodius)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_HAS_SHADOWS)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(14*8, (64-14)*8-1, 2*8, 30*8-1 )

	MDRV_PALETTE_LENGTH(2048)

	MDRV_VIDEO_UPDATE(parodius)

	MDRV_K052109_ADD("k052109", parodius_k052109_intf)
	MDRV_K053245_ADD("k053245", parodius_k05324x_intf)
	MDRV_K053251_ADD("k053251")

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2151, 3579545)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)

	MDRV_SOUND_ADD("k053260", K053260, 3579545)
	MDRV_SOUND_ROUTE(0, "lspeaker", 0.70)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.70)
MACHINE_DRIVER_END

/***************************************************************************

  Game ROMs

***************************************************************************/

ROM_START( parodius )
	ROM_REGION( 0x48000, "maincpu", 0 ) /* code + banked roms + palette RAM */
	ROM_LOAD( "955l01.bin", 0x10000, 0x20000, CRC(49a658eb) SHA1(dd53060c4da99b8e1f896ebfec572296ef2b5665) )
	ROM_LOAD( "955l02.bin", 0x30000, 0x18000, CRC(161d7322) SHA1(a752f28c19c58263680221ad1119f2fd57df4723) )
	ROM_CONTINUE(           0x08000, 0x08000 )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "955e03.bin", 0x0000, 0x10000, CRC(940aa356) SHA1(e7466f049be48861fd2d929eed786bd48782b5bb) )

	ROM_REGION( 0x100000, "gfx1", 0 ) /* graphics ( don't dispose as the program can read them, 0 ) */
	ROM_LOAD( "955d07.bin", 0x000000, 0x080000, CRC(89473fec) SHA1(0da18c4b078c3a30233a6f5c2b90032168136f58) ) /* characters */
	ROM_LOAD( "955d08.bin", 0x080000, 0x080000, CRC(43d5cda1) SHA1(2c51bad4857d1d31456c6dc1e7d41326ea35468b) ) /* characters */

	ROM_REGION( 0x100000, "gfx2", 0 ) /* graphics ( don't dispose as the program can read them, 0 ) */
	ROM_LOAD( "955d05.bin", 0x000000, 0x080000, CRC(7a1e55e0) SHA1(7a0e04ebde28d1e7b60aef3de926dc0e78662b1e) )	/* sprites */
	ROM_LOAD( "955d06.bin", 0x080000, 0x080000, CRC(f4252875) SHA1(490f2e19b30cf8724e4b03b8d9f089c470ec13bd) )	/* sprites */

	ROM_REGION( 0x80000, "k053260", 0 ) /* 053260 samples */
	ROM_LOAD( "955d04.bin", 0x00000, 0x80000, CRC(e671491a) SHA1(79e71cb5212eb7d14d3479b0734ea0270473a66d) )
ROM_END

ROM_START( parodiusj )
	ROM_REGION( 0x48000, "maincpu", 0 ) /* code + banked roms + palette RAM */
	ROM_LOAD( "955e01.bin", 0x10000, 0x20000, CRC(49baa334) SHA1(8902fbb2228111b15de6537bd168241933df134d) )
	ROM_LOAD( "955e02.bin", 0x30000, 0x18000, CRC(14010d6f) SHA1(69fe162ea08c3bd4b3e78e9d10d278bd15444af4) )
	ROM_CONTINUE(           0x08000, 0x08000 )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "955e03.bin", 0x0000, 0x10000, CRC(940aa356) SHA1(e7466f049be48861fd2d929eed786bd48782b5bb) )

	ROM_REGION( 0x100000, "gfx1", 0 ) /* graphics ( don't dispose as the program can read them, 0 ) */
	ROM_LOAD( "955d07.bin", 0x000000, 0x080000, CRC(89473fec) SHA1(0da18c4b078c3a30233a6f5c2b90032168136f58) ) /* characters */
	ROM_LOAD( "955d08.bin", 0x080000, 0x080000, CRC(43d5cda1) SHA1(2c51bad4857d1d31456c6dc1e7d41326ea35468b) ) /* characters */

	ROM_REGION( 0x100000, "gfx2", 0 ) /* graphics ( don't dispose as the program can read them, 0 ) */
	ROM_LOAD( "955d05.bin", 0x000000, 0x080000, CRC(7a1e55e0) SHA1(7a0e04ebde28d1e7b60aef3de926dc0e78662b1e) )	/* sprites */
	ROM_LOAD( "955d06.bin", 0x080000, 0x080000, CRC(f4252875) SHA1(490f2e19b30cf8724e4b03b8d9f089c470ec13bd) )	/* sprites */

	ROM_REGION( 0x80000, "k053260", 0 ) /* 053260 samples */
	ROM_LOAD( "955d04.bin", 0x00000, 0x80000, CRC(e671491a) SHA1(79e71cb5212eb7d14d3479b0734ea0270473a66d) )
ROM_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

static KONAMI_SETLINES_CALLBACK( parodius_banking )
{
	if (lines & 0xf0)
		logerror("%04x: setlines %02x\n", cpu_get_pc(device), lines);

	memory_set_bank(device->machine,  "bank1", (lines & 0x0f) ^ 0x0f);
}

GAME( 1990, parodius,  0,        parodius, parodius, 0, ROT0, "Konami", "Parodius DA! (World)", GAME_SUPPORTS_SAVE )
GAME( 1990, parodiusj, parodius, parodius, parodius, 0, ROT0, "Konami", "Parodius DA! (Japan)", GAME_SUPPORTS_SAVE )
