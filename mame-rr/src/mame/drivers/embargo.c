/***************************************************************************

    Cinematronics Embargo driver

***************************************************************************/

#include "emu.h"
#include "cpu/s2650/s2650.h"


class embargo_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, embargo_state(machine)); }

	embargo_state(running_machine &machine) { }

	/* memory pointers */
	UINT8 *  videoram;
	size_t   videoram_size;

	/* misc */
	UINT8    dial_enable_1;
	UINT8    dial_enable_2;
	UINT8    input_select;
};


/*************************************
 *
 *  Video system
 *
 *************************************/

static VIDEO_UPDATE( embargo )
{
	embargo_state *state = (embargo_state *)screen->machine->driver_data;
	offs_t offs;

	for (offs = 0; offs < state->videoram_size; offs++)
	{
		int i;

		UINT8 x = offs << 3;
		UINT8 y = offs >> 5;
		UINT8 data = state->videoram[offs];

		for (i = 0; i < 8; i++)
		{
			pen_t pen = (data & 0x01) ? RGB_WHITE : RGB_BLACK;
			*BITMAP_ADDR32(bitmap, y, x) = pen;

			data = data >> 1;
			x = x + 1;
		}
	}

	return 0;
}



/*************************************
 *
 *  Input handling
 *
 *************************************/

static READ8_HANDLER( input_port_bit_r )
{
	embargo_state *state = (embargo_state *)space->machine->driver_data;
	return (input_port_read(space->machine, "IN1") << (7 - state->input_select)) & 0x80;
}


static READ8_HANDLER( dial_r )
{
	embargo_state *state = (embargo_state *)space->machine->driver_data;

	UINT8 lo = 0;
	UINT8 hi = 0;

	UINT8 mapped_lo = 0;
	UINT8 mapped_hi = 0;

	int i;

	/* game reads 4 bits per dial and maps them onto clock directions */

	static const UINT8 map[] =
	{
		0x00, 0x0b, 0x01, 0x02, 0x04, 0x04, 0x02, 0x03,
		0x09, 0x0a, 0x08, 0x09, 0x08, 0x05, 0x07, 0x06
	};

	if (state->dial_enable_1 && !state->dial_enable_2)
	{
		lo = input_port_read(space->machine, "DIAL0");
		hi = input_port_read(space->machine, "DIAL1");
	}

	if (state->dial_enable_2 && !state->dial_enable_1)
	{
		lo = input_port_read(space->machine, "DIAL2");
		hi = input_port_read(space->machine, "DIAL3");
	}

	lo = 12 * lo / 256;
	hi = 12 * hi / 256;

	for (i = 0; i < 16; i++)
	{
		if (map[i] == lo)
		{
			mapped_lo = i;
		}

		if (map[i] == hi)
		{
			mapped_hi = i;
		}
	}

	return (mapped_hi << 4) | mapped_lo;
}


static WRITE8_HANDLER( port_1_w )
{
	embargo_state *state = (embargo_state *)space->machine->driver_data;
	state->dial_enable_1 = data & 0x01; /* other bits unknown */
}


static WRITE8_HANDLER( port_2_w )
{
	embargo_state *state = (embargo_state *)space->machine->driver_data;
	state->dial_enable_2 = data & 0x01; /* other bits unknown */
}


static WRITE8_HANDLER( input_select_w )
{
	embargo_state *state = (embargo_state *)space->machine->driver_data;
	state->input_select = data & 0x07;
}



/*************************************
 *
 *  Memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x1e00, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0x3fff) AM_RAM AM_BASE_SIZE_MEMBER(embargo_state, videoram, videoram_size)
ADDRESS_MAP_END



/*************************************
 *
 *  Port handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_io_map, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x01, 0x01) AM_READ_PORT("IN0") AM_WRITE(port_1_w)
	AM_RANGE(0x02, 0x02) AM_READWRITE(dial_r, port_2_w)
	AM_RANGE(0x03, 0x03) AM_WRITENOP /* always 0xFE */
	AM_RANGE(S2650_DATA_PORT, S2650_DATA_PORT) AM_READ_PORT("IN2")
	AM_RANGE(S2650_CTRL_PORT, S2650_CTRL_PORT) AM_READWRITE(input_port_bit_r, input_select_w)
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( embargo )

	PORT_START("IN0") /* port 0x01 */
	PORT_DIPNAME( 0x03, 0x00, "Rounds" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )

	PORT_START("IN1") /* S2650_CONTROL_PORT */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)

	PORT_START("IN2") /* S2650_DATA_PORT */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("DIAL0")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(8) PORT_PLAYER(1)

	PORT_START("DIAL1")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(8) PORT_PLAYER(2)

	PORT_START("DIAL2")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(8) PORT_PLAYER(3)

	PORT_START("DIAL3")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(8) PORT_PLAYER(4)

INPUT_PORTS_END



/*************************************
 *
 *  Machine setup
 *
 *************************************/

static MACHINE_START( embargo )
{
	embargo_state *state = (embargo_state *)machine->driver_data;

	/* register for state saving */
	state_save_register_global(machine, state->dial_enable_1);
	state_save_register_global(machine, state->dial_enable_2);
	state_save_register_global(machine, state->input_select);
}


static MACHINE_RESET( embargo )
{
	embargo_state *state = (embargo_state *)machine->driver_data;

	state->dial_enable_1 = 0;
	state->dial_enable_2 = 0;
	state->input_select = 0;
}

/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_DRIVER_START( embargo )

	/* driver data */
	MDRV_DRIVER_DATA(embargo_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", S2650, 625000)
	MDRV_CPU_PROGRAM_MAP(main_map)
	MDRV_CPU_IO_MAP(main_io_map)

	MDRV_MACHINE_START(embargo)
	MDRV_MACHINE_RESET(embargo)

	/* video hardware */
	MDRV_VIDEO_UPDATE(embargo)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 255, 0, 239)
	MDRV_SCREEN_REFRESH_RATE(60)

MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( embargo )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "emb1", 0x0000, 0x0200, CRC(00dcbc24) SHA1(67018a20d7694618123499640f041fb518ea29fa) )
	ROM_LOAD( "emb2", 0x0200, 0x0200, CRC(e7069b11) SHA1(b933095087cd4fe10f12fd244606aaaed1c31bca) )
	ROM_LOAD( "emb3", 0x0400, 0x0200, CRC(1af7a966) SHA1(a8f6d1063927106f44c43f64c26b52c07c5450df) )
	ROM_LOAD( "emb4", 0x0600, 0x0200, CRC(d9c75da0) SHA1(895784ec543f1c73ced5f37751a26cb3305030f3) )
	ROM_LOAD( "emb5", 0x0800, 0x0200, CRC(15960b58) SHA1(2e6c196b240cef92799f83deef2b1c501c01f9c9) )
	ROM_LOAD( "emb6", 0x0a00, 0x0200, CRC(7ba23058) SHA1(ad3736ec7617ecb902ea686055e55203be1ea5fd) )
	ROM_LOAD( "emb7", 0x0c00, 0x0200, CRC(6d46a593) SHA1(5432ae1c167e774c47f06ffd0e8acf801891dee1) )
	ROM_LOAD( "emb8", 0x0e00, 0x0200, CRC(f0b00634) SHA1(317aacc9022596a2af0f3b399fe119fe9c8c1679) )
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1977, embargo, 0, embargo, embargo, 0, ROT0, "Cinematronics", "Embargo", GAME_NO_SOUND | GAME_SUPPORTS_SAVE )
