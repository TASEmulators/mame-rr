/***************************************************************************

    Kitco Crowns Golf hardware

    driver by Aaron Giles

    Games supported:
        * Crowns Golf (4 sets)
        * Crowns Golf in Hawaii

    Known bugs:
        * not sure if the analog inputs are handled correctly

    Text Strings in sound CPU ROM read:
    ARIES ELECA
    1984JAN15 V-0

    Text Strings in the bootleg sound CPU ROM read:
    WHO AM I?      (In place of "ARIES ELECA")
    1984JULY1 V-1  (In place of "1984JAN15 V-0")
    1984 COPYRIGHT BY WHO

    2008-08
    Dip locations and factory settings verified with manual

****************************************************************************

    Memory map (TBA)

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "includes/crgolf.h"
#include "sound/ay8910.h"
#include "sound/msm5205.h"


/*************************************
 *
 *  ROM banking
 *
 *************************************/

static WRITE8_HANDLER( rom_bank_select_w )
{
	memory_set_bank(space->machine, "bank1", data & 15);
}


static MACHINE_START( crgolf )
{
	crgolf_state *state = (crgolf_state *)machine->driver_data;

	state->maincpu = machine->device("maincpu");
	state->audiocpu = machine->device("audiocpu");

	/* configure the banking */
	memory_configure_bank(machine, "bank1", 0, 16, memory_region(machine, "maincpu") + 0x10000, 0x2000);
	memory_set_bank(machine, "bank1", 0);

	/* register for save states */
	state_save_register_global(machine, state->port_select);
	state_save_register_global(machine, state->main_to_sound_data);
	state_save_register_global(machine, state->sound_to_main_data);
	state_save_register_global(machine, state->sample_offset);
	state_save_register_global(machine, state->sample_count);
}


static MACHINE_RESET( crgolf )
{
	crgolf_state *state = (crgolf_state *)machine->driver_data;

	state->port_select = 0;
	state->main_to_sound_data = 0;
	state->sound_to_main_data = 0;
	state->sample_offset = 0;
	state->sample_count = 0;
}


/*************************************
 *
 *  Input ports
 *
 *************************************/

static READ8_HANDLER( switch_input_r )
{
	static const char *const portnames[] = { "IN0", "IN1", "P1", "P2", "DSW", "UNUSED0", "UNUSED1" };
	crgolf_state *state = (crgolf_state *)space->machine->driver_data;

	return input_port_read(space->machine, portnames[state->port_select]);
}


static READ8_HANDLER( analog_input_r )
{
	return ((input_port_read(space->machine, "STICK0") >> 4) | (input_port_read(space->machine, "STICK1") & 0xf0)) ^ 0x88;
}


static WRITE8_HANDLER( switch_input_select_w )
{
	crgolf_state *state = (crgolf_state *)space->machine->driver_data;

	if (!(data & 0x40)) state->port_select = 6;
	if (!(data & 0x20)) state->port_select = 5;
	if (!(data & 0x10)) state->port_select = 4;
	if (!(data & 0x08)) state->port_select = 3;
	if (!(data & 0x04)) state->port_select = 2;
	if (!(data & 0x02)) state->port_select = 1;
	if (!(data & 0x01)) state->port_select = 0;
}


static WRITE8_HANDLER( unknown_w )
{
	logerror("%04X:unknown_w = %02X\n", cpu_get_pc(space->cpu), data);
}



/*************************************
 *
 *  Main->Sound CPU communications
 *
 *************************************/

static TIMER_CALLBACK( main_to_sound_callback )
{
	crgolf_state *state = (crgolf_state *)machine->driver_data;

	cpu_set_input_line(state->audiocpu, INPUT_LINE_NMI, ASSERT_LINE);
	state->main_to_sound_data = param;
}


static WRITE8_HANDLER( main_to_sound_w )
{
	timer_call_after_resynch(space->machine, NULL, data, main_to_sound_callback);
}


static READ8_HANDLER( main_to_sound_r )
{
	crgolf_state *state = (crgolf_state *)space->machine->driver_data;

	cpu_set_input_line(state->audiocpu, INPUT_LINE_NMI, CLEAR_LINE);
	return state->main_to_sound_data;
}



/*************************************
 *
 *  Sound->Main CPU communications
 *
 *************************************/

static TIMER_CALLBACK( sound_to_main_callback )
{
	crgolf_state *state = (crgolf_state *)machine->driver_data;

	cpu_set_input_line(state->maincpu, INPUT_LINE_NMI, ASSERT_LINE);
	state->sound_to_main_data = param;
}


static WRITE8_HANDLER( sound_to_main_w )
{
	timer_call_after_resynch(space->machine, NULL, data, sound_to_main_callback);
}


static READ8_HANDLER( sound_to_main_r )
{
	crgolf_state *state = (crgolf_state *)space->machine->driver_data;

	cpu_set_input_line(state->maincpu, INPUT_LINE_NMI, CLEAR_LINE);
	return state->sound_to_main_data;
}



/*************************************
 *
 *  Hawaii auto-sample player
 *
 *************************************/

static void vck_callback( running_device *device )
{
	crgolf_state *state = (crgolf_state *)device->machine->driver_data;

	/* only play back if we have data remaining */
	if (state->sample_count != 0xff)
	{
		UINT8 data = memory_region(device->machine, "adpcm")[state->sample_offset >> 1];

		/* write the next nibble and advance */
		msm5205_data_w(device, (data >> (4 * (~state->sample_offset & 1))) & 0x0f);
		state->sample_offset++;

		/* every 256 clocks, we decrement the length */
		if (!(state->sample_offset & 0xff))
		{
			state->sample_count--;

			/* if we hit 0xff, automatically turn off playback */
			if (state->sample_count == 0xff)
				msm5205_reset_w(device, 1);
		}
	}
}


static WRITE8_DEVICE_HANDLER( crgolfhi_sample_w )
{
	crgolf_state *state = (crgolf_state *)device->machine->driver_data;

	switch (offset)
	{
		/* offset 0 holds the MSM5205 in reset */
		case 0:
			msm5205_reset_w(device, 1);
			break;

		/* offset 1 is the length/256 nibbles */
		case 1:
			state->sample_count = data;
			break;

		/* offset 2 is the offset/256 nibbles */
		case 2:
			state->sample_offset = data << 8;
			break;

		/* offset 3 turns on playback */
		case 3:
			msm5205_reset_w(device, 0);
			break;
	}
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x5fff) AM_RAM
	AM_RANGE(0x6000, 0x7fff) AM_ROMBANK("bank1")
	AM_RANGE(0x8003, 0x8003) AM_WRITEONLY AM_BASE_MEMBER(crgolf_state, color_select)
	AM_RANGE(0x8004, 0x8004) AM_WRITEONLY AM_BASE_MEMBER(crgolf_state, screen_flip)
	AM_RANGE(0x8005, 0x8005) AM_WRITEONLY AM_BASE_MEMBER(crgolf_state, screen_select)
	AM_RANGE(0x8006, 0x8006) AM_WRITEONLY AM_BASE_MEMBER(crgolf_state, screenb_enable)
	AM_RANGE(0x8007, 0x8007) AM_WRITEONLY AM_BASE_MEMBER(crgolf_state, screena_enable)
	AM_RANGE(0x8800, 0x8800) AM_READWRITE(sound_to_main_r, main_to_sound_w)
	AM_RANGE(0x9000, 0x9000) AM_WRITE(rom_bank_select_w)
	AM_RANGE(0xa000, 0xffff) AM_READWRITE(crgolf_videoram_r, crgolf_videoram_w)
ADDRESS_MAP_END



/*************************************
 *
 *  Sound CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0xc000, 0xc001) AM_DEVWRITE("aysnd", ay8910_address_data_w)
	AM_RANGE(0xc002, 0xc002) AM_WRITENOP
	AM_RANGE(0xe000, 0xe000) AM_READWRITE(switch_input_r, switch_input_select_w)
	AM_RANGE(0xe001, 0xe001) AM_READWRITE(analog_input_r, unknown_w)
	AM_RANGE(0xe003, 0xe003) AM_READWRITE(main_to_sound_r, sound_to_main_w)
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( crgolf )
	PORT_START("IN0")	/* CREDIT */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN1")	/* SELECT */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START3 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START4 )
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(1)			/* club select */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)			/* backward address */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)			/* forward address */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(1)			/* open stance */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(1)			/* closed stance */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)	/* direction left */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)	/* direction right */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)			/* shot switch */

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_COCKTAIL		/* club select */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL		/* backward address */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_COCKTAIL		/* forward address */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_COCKTAIL		/* open stance */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_COCKTAIL		/* closed stance */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL	/* direction left */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL	/* direction right */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL		/* shot switch */

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x06, 0x04, "Half-Round Play" ) PORT_DIPLOCATION("SW:1,4")
	PORT_DIPSETTING(    0x00, "4 Coins" )
	PORT_DIPSETTING(    0x02, "5 Coins" )
	PORT_DIPSETTING(    0x04, "6 Coins" )
	PORT_DIPSETTING(    0x06, "10 Coins" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW:3")
	PORT_DIPSETTING(    0x08, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x10, 0x00, "Clear High Scores" ) PORT_DIPLOCATION("SW:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW:5")
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x00, "SW:8" )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_HIGH, "SW:7" )

	PORT_START("UNUSED0")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("UNUSED1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STICK0")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(70) PORT_KEYDELTA(16) PORT_REVERSE

	PORT_START("STICK1")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(70) PORT_KEYDELTA(16) PORT_REVERSE PORT_COCKTAIL
INPUT_PORTS_END



/*************************************
 *
 *  Sound definitions
 *
 *************************************/

static const msm5205_interface msm5205_intf =
{
	vck_callback,
	MSM5205_S64_4B
};



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_DRIVER_START( crgolf )

	/* driver data */
	MDRV_DRIVER_DATA(crgolf_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80,MASTER_CLOCK/3/2)
	MDRV_CPU_PROGRAM_MAP(main_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_CPU_ADD("audiocpu", Z80,MASTER_CLOCK/3/2)
	MDRV_CPU_PROGRAM_MAP(sound_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_MACHINE_START(crgolf)
	MDRV_MACHINE_RESET(crgolf)
	MDRV_QUANTUM_TIME(HZ(6000))

	/* video hardware */
	MDRV_IMPORT_FROM(crgolf_video)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("aysnd", AY8910, MASTER_CLOCK/3/2/2)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( crgolfhi )
	MDRV_IMPORT_FROM(crgolf)

	MDRV_SOUND_ADD("msm", MSM5205, 384000)
	MDRV_SOUND_CONFIG(msm5205_intf)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( crgolf ) // 834-5419-04
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "crwnc1.1c",  0x00000, 0x2000, CRC(3246e405) SHA1(f6018029317ac96df5866ca6a2bb2135edbd7e77) )
	ROM_LOAD( "crwna1.1a",  0x02000, 0x2000, CRC(b9a936e2) SHA1(cebf67d9c42627fbb39648674012a6cf8cb287b5) )
	ROM_LOAD( "epr5880.6b", 0x10000, 0x2000, CRC(4d6d8dad) SHA1(1530f81ad0097eadc75884ff8690b60b85ae451b) )
	ROM_LOAD( "epr5885.5e", 0x1e000, 0x2000, CRC(fac6d56c) SHA1(67dc1918d5ab2443e967359e51d49dd134cdf25d) )
	ROM_LOAD( "epr5881.6f", 0x20000, 0x2000, CRC(dd48dc1f) SHA1(d4560a88d872bd5f401344e3adb25f8486caca11) )
	ROM_LOAD( "epr5886.5f", 0x22000, 0x2000, CRC(a09b27b8) SHA1(8b2d8322b633f6c7174bdb1fff0f6cef2d5a86de) )
	ROM_LOAD( "epr5882.6h", 0x24000, 0x2000, CRC(fb86a168) SHA1(a679c9f50ac952da6c65f6593dce805023b8fc45) )
	ROM_LOAD( "epr5887.5h", 0x26000, 0x2000, CRC(981f03ef) SHA1(42f686b970902bc42ac0f81bd2fc93dbdf766b1a) )
	ROM_LOAD( "epr5883.6j", 0x28000, 0x2000, CRC(e64125ff) SHA1(ae2014d1039f4ed02c55053519bdeddd2f60a77a) )
	ROM_LOAD( "epr5888.5j", 0x2a000, 0x2000, CRC(efc0e15a) SHA1(ba5772830f921004a2d9c90f557c04c799c755b9) )
	ROM_LOAD( "epr5884.6k", 0x2c000, 0x2000, CRC(eb455966) SHA1(14278b598ac1d4007d5357cb40899c92a052417f) )
	ROM_LOAD( "epr5889.5k", 0x2e000, 0x2000, CRC(88357391) SHA1(afdb5ed6555adf60bd64808413fc72fa5c67b6ec) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "epr6198.1f",  0x0000, 0x1000, CRC(388c33d6) SHA1(42fd19c4b4ec7538d6c437552efb258bf2dcebc0) )
	ROM_LOAD( "epr5892.1e",  0x2000, 0x2000, CRC(608dc2e2) SHA1(d906537cffd3e055f52f37a0490b3bb63107b2f9) )
	ROM_LOAD( "epr5891a.1d", 0x4000, 0x2000, CRC(f353b585) SHA1(f09dcd0240131f872ceef5ddc9c89ab2fc92d117) )
	ROM_LOAD( "epr5890a.1c", 0x6000, 0x2000, CRC(b737c2e8) SHA1(8596abbdff74300230b5ec5bf8acfe222eb3414f) )

	ROM_REGION( 0x0020,  "proms", 0 )
	ROM_LOAD( "pr5877.1s", 0x0000, 0x0020, CRC(f880b95d) SHA1(5ad0ee39e2b9befaf3895ec635d5865b7b1e562b) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "pal16l8.3e",  0x0000, 0x0104, NO_DUMP ) /* PAL is read protected */
ROM_END

ROM_START( crgolfa ) // 834-5419-03
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "epr6143.1c", 0x00000, 0x2000, CRC(4b301360) SHA1(2a7dd4876f4448b4b59b6dd02e55eb2d0126b777) )
	ROM_LOAD( "epr6142.1a", 0x02000, 0x2000, CRC(8fc5e67f) SHA1(6563db94c55cfc7d2270daccaab57fc7b422b9f9) )
	ROM_LOAD( "epr5880.6b", 0x10000, 0x2000, CRC(4d6d8dad) SHA1(1530f81ad0097eadc75884ff8690b60b85ae451b) )
	ROM_LOAD( "epr5885.5e", 0x1e000, 0x2000, CRC(fac6d56c) SHA1(67dc1918d5ab2443e967359e51d49dd134cdf25d) )
	ROM_LOAD( "epr5881.6f", 0x20000, 0x2000, CRC(dd48dc1f) SHA1(d4560a88d872bd5f401344e3adb25f8486caca11) )
	ROM_LOAD( "epr5886.5f", 0x22000, 0x2000, CRC(a09b27b8) SHA1(8b2d8322b633f6c7174bdb1fff0f6cef2d5a86de) )
	ROM_LOAD( "epr5882.6h", 0x24000, 0x2000, CRC(fb86a168) SHA1(a679c9f50ac952da6c65f6593dce805023b8fc45) )
	ROM_LOAD( "epr5887.5h", 0x26000, 0x2000, CRC(981f03ef) SHA1(42f686b970902bc42ac0f81bd2fc93dbdf766b1a) )
	ROM_LOAD( "epr5883.6j", 0x28000, 0x2000, CRC(e64125ff) SHA1(ae2014d1039f4ed02c55053519bdeddd2f60a77a) )
	ROM_LOAD( "epr5888.5j", 0x2a000, 0x2000, CRC(efc0e15a) SHA1(ba5772830f921004a2d9c90f557c04c799c755b9) )
	ROM_LOAD( "epr5884.6k", 0x2c000, 0x2000, CRC(eb455966) SHA1(14278b598ac1d4007d5357cb40899c92a052417f) )
	ROM_LOAD( "epr5889.5k", 0x2e000, 0x2000, CRC(88357391) SHA1(afdb5ed6555adf60bd64808413fc72fa5c67b6ec) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "epr6144.1f",  0x0000, 0x1000, CRC(3fdc8cd6) SHA1(01d118d56a0e363af66a36ba583c4cbce86ee1d1) )
	ROM_LOAD( "epr5892.1e",  0x2000, 0x2000, CRC(608dc2e2) SHA1(d906537cffd3e055f52f37a0490b3bb63107b2f9) )
	ROM_LOAD( "epr5891a.1d", 0x4000, 0x2000, CRC(f353b585) SHA1(f09dcd0240131f872ceef5ddc9c89ab2fc92d117) )
	ROM_LOAD( "epr5890a.1c", 0x6000, 0x2000, CRC(b737c2e8) SHA1(8596abbdff74300230b5ec5bf8acfe222eb3414f) )

	ROM_REGION( 0x0020,  "proms", 0 )
	ROM_LOAD( "pr5877.1s", 0x0000, 0x0020, CRC(f880b95d) SHA1(5ad0ee39e2b9befaf3895ec635d5865b7b1e562b) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "pal16l8.3e",  0x0000, 0x0104, NO_DUMP ) /* PAL is read protected */
ROM_END


ROM_START( crgolfb )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "epr5879b.1c", 0x00000, 0x2000, CRC(927be359) SHA1(d534f7e3ef4ced8eea882ae2b8425df4c5842833) )
	ROM_LOAD( "epr5878.1a",  0x02000, 0x2000, CRC(65fd0fa0) SHA1(de95ff95c9f981cd9eadf8b028ee5373bc69007b) )
	ROM_LOAD( "epr5880.6b",  0x10000, 0x2000, CRC(4d6d8dad) SHA1(1530f81ad0097eadc75884ff8690b60b85ae451b) )
	ROM_LOAD( "epr5885.5e",  0x1e000, 0x2000, CRC(fac6d56c) SHA1(67dc1918d5ab2443e967359e51d49dd134cdf25d) )
	ROM_LOAD( "epr5881.6f",  0x20000, 0x2000, CRC(dd48dc1f) SHA1(d4560a88d872bd5f401344e3adb25f8486caca11) )
	ROM_LOAD( "epr5886.5f",  0x22000, 0x2000, CRC(a09b27b8) SHA1(8b2d8322b633f6c7174bdb1fff0f6cef2d5a86de) )
	ROM_LOAD( "epr5882.6h",  0x24000, 0x2000, CRC(fb86a168) SHA1(a679c9f50ac952da6c65f6593dce805023b8fc45) )
	ROM_LOAD( "epr5887.5h",  0x26000, 0x2000, CRC(981f03ef) SHA1(42f686b970902bc42ac0f81bd2fc93dbdf766b1a) )
	ROM_LOAD( "epr5883.6j",  0x28000, 0x2000, CRC(e64125ff) SHA1(ae2014d1039f4ed02c55053519bdeddd2f60a77a) )
	ROM_LOAD( "epr5888.5j",  0x2a000, 0x2000, CRC(efc0e15a) SHA1(ba5772830f921004a2d9c90f557c04c799c755b9) )
	ROM_LOAD( "epr5884.6k",  0x2c000, 0x2000, CRC(eb455966) SHA1(14278b598ac1d4007d5357cb40899c92a052417f) )
	ROM_LOAD( "epr5889.5k",  0x2e000, 0x2000, CRC(88357391) SHA1(afdb5ed6555adf60bd64808413fc72fa5c67b6ec) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "epr5893c.1f", 0x0000, 0x1000, CRC(5011646d) SHA1(1bbf83107396d69c17580d4b1b38d93f741a608f) )
	ROM_LOAD( "epr5892.1e",  0x2000, 0x2000, CRC(608dc2e2) SHA1(d906537cffd3e055f52f37a0490b3bb63107b2f9) )
	ROM_LOAD( "epr5891a.1d", 0x4000, 0x2000, CRC(f353b585) SHA1(f09dcd0240131f872ceef5ddc9c89ab2fc92d117) )
	ROM_LOAD( "epr5890a.1c", 0x6000, 0x2000, CRC(b737c2e8) SHA1(8596abbdff74300230b5ec5bf8acfe222eb3414f) )

	ROM_REGION( 0x0020,  "proms", 0 )
	ROM_LOAD( "pr5877.1s",   0x0000, 0x0020, CRC(f880b95d) SHA1(5ad0ee39e2b9befaf3895ec635d5865b7b1e562b) )
ROM_END


ROM_START( crgolfc )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "15.1a",      0x00000, 0x2000, CRC(e6194356) SHA1(78eec53a0658b552e6a8af109d9c9754e4ddadcb) )
	ROM_LOAD( "16.1c",      0x02000, 0x2000, CRC(f50412e2) SHA1(5a50fb1edfc26072e921447bd157fe996f707e05) )
	ROM_LOAD( "cg.1",       0x10000, 0x2000, CRC(ad7d537a) SHA1(deff74074a8b16ea91a0fa72d97ec36336c87b97) ) //  1.6a
	ROM_LOAD( "epr5885.5e", 0x1e000, 0x2000, CRC(fac6d56c) SHA1(67dc1918d5ab2443e967359e51d49dd134cdf25d) ) //  6.5a
	ROM_LOAD( "epr5881.6f", 0x20000, 0x2000, CRC(dd48dc1f) SHA1(d4560a88d872bd5f401344e3adb25f8486caca11) ) //  2.6b
	ROM_LOAD( "epr5886.5f", 0x22000, 0x2000, CRC(a09b27b8) SHA1(8b2d8322b633f6c7174bdb1fff0f6cef2d5a86de) ) //  7.5b
	ROM_LOAD( "3.6c",       0x24000, 0x2000, CRC(b7fcee1a) SHA1(47e9a2cee945c5f59490b73c475ec2512ea0f559) )
	ROM_LOAD( "epr5887.5h", 0x26000, 0x2000, CRC(981f03ef) SHA1(42f686b970902bc42ac0f81bd2fc93dbdf766b1a) ) //  8.5c
	ROM_LOAD( "epr5883.6j", 0x28000, 0x2000, CRC(e64125ff) SHA1(ae2014d1039f4ed02c55053519bdeddd2f60a77a) ) //  4.6d
	ROM_LOAD( "epr5888.5j", 0x2a000, 0x2000, CRC(efc0e15a) SHA1(ba5772830f921004a2d9c90f557c04c799c755b9) ) //  9.5d
	ROM_LOAD( "epr5884.6k", 0x2c000, 0x2000, CRC(eb455966) SHA1(14278b598ac1d4007d5357cb40899c92a052417f) ) //  5.6e
	ROM_LOAD( "epr5889.5k", 0x2e000, 0x2000, CRC(88357391) SHA1(afdb5ed6555adf60bd64808413fc72fa5c67b6ec) ) // 10.5e

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "11.1e",       0x0000, 0x1000, CRC(53295a1a) SHA1(ec6c4df9f32e4b3ffe48e823d90a9e6a671e6027) )
	ROM_LOAD( "epr5892.1e",  0x2000, 0x2000, CRC(608dc2e2) SHA1(d906537cffd3e055f52f37a0490b3bb63107b2f9) ) // 12.1d
	ROM_LOAD( "epr5891a.1d", 0x4000, 0x2000, CRC(f353b585) SHA1(f09dcd0240131f872ceef5ddc9c89ab2fc92d117) ) // 13.1c
	ROM_LOAD( "epr5890a.1c", 0x6000, 0x2000, CRC(b737c2e8) SHA1(8596abbdff74300230b5ec5bf8acfe222eb3414f) ) // 14.1b

	ROM_REGION( 0x0020,  "proms", 0 )
	ROM_LOAD( "pr5877.1s", 0x0000, 0x0020, CRC(f880b95d) SHA1(5ad0ee39e2b9befaf3895ec635d5865b7b1e562b) )
ROM_END


ROM_START( crgolfbt )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "epr5879b.1c", 0x00000, 0x2000, CRC(927be359) SHA1(d534f7e3ef4ced8eea882ae2b8425df4c5842833) )
	ROM_LOAD( "epr5878.1a",  0x02000, 0x2000, CRC(65fd0fa0) SHA1(de95ff95c9f981cd9eadf8b028ee5373bc69007b) )
	ROM_LOAD( "cg.1",        0x10000, 0x2000, CRC(ad7d537a) SHA1(deff74074a8b16ea91a0fa72d97ec36336c87b97) )
	ROM_LOAD( "epr5885.5e",  0x1e000, 0x2000, CRC(fac6d56c) SHA1(67dc1918d5ab2443e967359e51d49dd134cdf25d) ) // cg.6
	ROM_LOAD( "epr5881.6f",  0x20000, 0x2000, CRC(dd48dc1f) SHA1(d4560a88d872bd5f401344e3adb25f8486caca11) ) // cg.2
	ROM_LOAD( "epr5886.5f",  0x22000, 0x2000, CRC(a09b27b8) SHA1(8b2d8322b633f6c7174bdb1fff0f6cef2d5a86de) ) // cg.7
	ROM_LOAD( "epr5882.6h",  0x24000, 0x2000, CRC(fb86a168) SHA1(a679c9f50ac952da6c65f6593dce805023b8fc45) ) // cg.3
	ROM_LOAD( "epr5887.5h",  0x26000, 0x2000, CRC(981f03ef) SHA1(42f686b970902bc42ac0f81bd2fc93dbdf766b1a) ) // cg.8
	ROM_LOAD( "epr5883.6j",  0x28000, 0x2000, CRC(e64125ff) SHA1(ae2014d1039f4ed02c55053519bdeddd2f60a77a) ) // cg.4
	ROM_LOAD( "epr5888.5j",  0x2a000, 0x2000, CRC(efc0e15a) SHA1(ba5772830f921004a2d9c90f557c04c799c755b9) ) // cg.9
	ROM_LOAD( "epr5884.6k",  0x2c000, 0x2000, CRC(eb455966) SHA1(14278b598ac1d4007d5357cb40899c92a052417f) ) // cg.5
	ROM_LOAD( "epr5889.5k",  0x2e000, 0x2000, CRC(88357391) SHA1(afdb5ed6555adf60bd64808413fc72fa5c67b6ec) ) // cg.10

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "cg.14",       0x0000, 0x1000, CRC(07156cd9) SHA1(8907cf9228d6de117b24969d4e039cee330f9b1e) )
	ROM_LOAD( "epr5892.1e",  0x2000, 0x2000, CRC(608dc2e2) SHA1(d906537cffd3e055f52f37a0490b3bb63107b2f9) ) // cg.13
	ROM_LOAD( "epr5891a.1d", 0x4000, 0x2000, CRC(f353b585) SHA1(f09dcd0240131f872ceef5ddc9c89ab2fc92d117) ) // cg.12
	ROM_LOAD( "epr5890a.1c", 0x6000, 0x2000, CRC(b737c2e8) SHA1(8596abbdff74300230b5ec5bf8acfe222eb3414f) ) // cg.11

	ROM_REGION( 0x0020,  "proms", 0 )
	ROM_LOAD( "pr5877.1s",   0x0000, 0x0020, CRC(f880b95d) SHA1(5ad0ee39e2b9befaf3895ec635d5865b7b1e562b) )
ROM_END


ROM_START( crgolfhi )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "cpu.c1",  0x00000, 0x2000, CRC(8b101085) SHA1(a59c369be3e7e645d8b20032998a778a2056b7d7) )
	ROM_LOAD( "cpu.a1",  0x02000, 0x2000, CRC(f48a8ee8) SHA1(cc07c7258caf251e9cb52f12be779cb02fca0b0a) )
	ROM_LOAD( "main.b6", 0x10000, 0x2000, CRC(5b0336c6) SHA1(86e2c197f23a2f2f7666448b74611150ca15a2af) )
	ROM_LOAD( "main.b5", 0x12000, 0x2000, CRC(7b80149a) SHA1(c802a79b1430b15d166f5fca11d2ed4e65bc65a9) )
	ROM_LOAD( "main.c6", 0x14000, 0x2000, CRC(7804cb1c) SHA1(487f979f47a0f40fa35331c71a66dc8428387a26) )
	ROM_LOAD( "main.c5", 0x16000, 0x2000, CRC(7721efc5) SHA1(9f3fb6845e5815ada1535da7800e175769fd46b1) )
	ROM_LOAD( "main.d6", 0x18000, 0x2000, CRC(f3ccdfaa) SHA1(c266737caf7222a971d0297b944c5710d3ec12be) )
	ROM_LOAD( "main.d5", 0x1a000, 0x2000, CRC(bef85c95) SHA1(516615975207209a4c649df7ffd451167fc40c45) )
	ROM_LOAD( "main.e6", 0x1c000, 0x2000, CRC(aa75e849) SHA1(226e7712e65f86422a1caebf3b95abcf39af2277) )
	ROM_LOAD( "main.e5", 0x1e000, 0x2000, CRC(e8eefbc4) SHA1(02393d3c0a1234ec51348d755725562cc7861285) )
	ROM_LOAD( "main.f6", 0x20000, 0x2000, CRC(e1130eec) SHA1(26a68f8af543983fcae73db59d075b11ee101ca8) )
	ROM_LOAD( "main.f5", 0x22000, 0x2000, CRC(090c21e3) SHA1(e5e0fc1e4ffd2a9c344cfc70a9e8e7cebb0821cc) )
	ROM_LOAD( "main.h6", 0x24000, 0x2000, CRC(33b8ada4) SHA1(73192108daa0724c30c1deea7d52538a49bfdf8f) )
	ROM_LOAD( "main.h5", 0x26000, 0x2000, CRC(16e5a26c) SHA1(7bb6e5d852f352331953058c17e753fee04d1cf9) )
	ROM_LOAD( "main.j6", 0x28000, 0x2000, CRC(22db8cce) SHA1(cd646830129bfdd2f5f10c8f6732e76f8a15b74f) )
	ROM_LOAD( "main.j5", 0x2a000, 0x2000, CRC(f757de30) SHA1(38330f10051735683f41ed425900b9f0f9ee01be) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "main.f1",  0x0000, 0x2000, CRC(e7c471de) SHA1(b953807bc714496363ca33ad0fc11a2d30aa7b7e) )

	ROM_REGION( 0x8000, "adpcm", 0 )
	ROM_LOAD( "sub.r1", 0x0000, 0x2000, CRC(9be85e38) SHA1(a108fe812d0518e7bef32fd76998c0c70b70723e) )
	ROM_LOAD( "sub.r2", 0x2000, 0x2000, CRC(d65b8e3a) SHA1(de6acffbe2d7078f0598857a6a3b2179e5c82a34) )
	ROM_LOAD( "sub.r3", 0x4000, 0x2000, CRC(65967250) SHA1(7620560ea57b8e5d259ea8881fb8d8ca46228014) )
	ROM_LOAD( "sub.r4", 0x6000, 0x2000, CRC(d3716776) SHA1(7e38437d255c5f28aac24f0943c10fc1ce998b60) )

	ROM_REGION( 0x0020,  "proms", 0 )
	ROM_LOAD( "prom.s1", 0x0000, 0x0020, CRC(014427df) SHA1(85a5e660f9667e032b80152bbde351007e5c88df) )
ROM_END



/*************************************
 *
 *  Game-specific init
 *
 *************************************/

static DRIVER_INIT( crgolfhi )
{
	running_device *msm = machine->device("msm");
	memory_install_write8_device_handler(cputag_get_address_space(machine, "audiocpu", ADDRESS_SPACE_PROGRAM), msm, 0xa000, 0xa003, 0, 0, crgolfhi_sample_w);
}



/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1984, crgolf,   0,      crgolf,   crgolf,  0,        ROT0, "Nasco Japan", "Crowns Golf (834-5419-04)", GAME_SUPPORTS_SAVE )
GAME( 1984, crgolfa,  crgolf, crgolf,   crgolf,  0,        ROT0, "Nasco Japan", "Crowns Golf (834-5419-03)", GAME_SUPPORTS_SAVE )
GAME( 1984, crgolfb,  crgolf, crgolf,   crgolf,  0,        ROT0, "Nasco Japan", "Crowns Golf (set 3)", GAME_SUPPORTS_SAVE )
GAME( 1984, crgolfc,  crgolf, crgolf,   crgolf,  0,        ROT0, "Nasco Japan", "Champion Golf", GAME_SUPPORTS_SAVE )
GAME( 1984, crgolfbt, crgolf, crgolf,   crgolf,  0,        ROT0, "bootleg", "Champion Golf (bootleg)", GAME_SUPPORTS_SAVE )
GAME( 1985, crgolfhi, 0,      crgolfhi, crgolf,  crgolfhi, ROT0, "Nasco Japan", "Crowns Golf in Hawaii" , GAME_SUPPORTS_SAVE )
