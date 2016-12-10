/***************************************************************************

    Tekunon Kougyou Beam Invader hardware

    driver by Zsolt Vasvari

    Games supported:
        * Beam Invader (2 sets)

    Known issues:
        * Port 0 might be an analog port select


Stephh's notes (based on the games Z80 code and some tests) :

  - The min/max values for the controllers might not be accurate, but I have no infos at all.
    So I put the min/max values from what I see in the Z80 code (see below).
  - Is the visible area correct ? The invaders and the ship don't reach the left part of the screen !

1) 'beaminv'

  - Routine to handle the analog inputs at 0x0521.
    Contents from 0x3400 (IN2) is compared with contents from 0x1d25 (value in RAM).
    Contents from 0x3400 is not limited but contents from 0x1d25 range is the following :
      . player 1 : min = 0x1c - max = 0xd1
      . player 2 : min = 0x2d - max = 0xe2
    This is why sometimes the ship moves even if you don't do anything !
  - Screen flipping is internally handled (no specific write to memory or out to a port).
  - I can't tell if controller select is handled with a out to port 0 but I haven't found
    any other write to memory or out to another port.
  - Player's turn is handled by multiple reads from 0x1839 in RAM :
      . 1 player  game : [0x1839] = 0x00
      . 2 players game : [0x1839] = 0xaa (player 1) or 0x55 (player 2)
  - Credits are stored at address 0x1837 (BCD coded, range 0x00-0x99)

2) 'beaminva'

  - Routine to handle the analog inputs at 0x04bd.
    Contents from 0x3400 (IN2) is compared with contents from 0x1d05 (value in RAM).
    Contents from 0x3400 is limited to range 0x35-0x95 but contents from 0x1d05 range is the following :
      . player 1 : min = 0x1c - max = 0xd1
      . player 2 : min = 0x2d - max = 0xe2
    This is why sometimes the ship moves even if you don't do anything !
  - Screen flipping is internally handled (no specific write to memory or out to a port).
  - I can't tell if controller select is handled with a out to port 0 but I haven't found
    any other write to memory or out to another port.
  - Player's turn is handled by multiple reads from 0x1838 in RAM :
      . 1 player  game : [0x1838] = 0x00
      . 2 players game : [0x1838] = 0xaa (player 1) or 0x55 (player 2)
  - Credits are stored at address 0x1836 (BCD coded, range 0x00-0x99)

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"


class beaminv_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, beaminv_state(machine)); }

	beaminv_state(running_machine &machine) { }

	/* memory pointers */
	UINT8 *    videoram;
	size_t     videoram_size;

	/* misc */
	emu_timer  *interrupt_timer;

	/* input-related */
	UINT8      controller_select;

	/* devices */
	running_device *maincpu;
};



/*************************************
 *
 *  Interrupt generation
 *
 *************************************/

/* the interrupt scanlines are a guess */

#define INTERRUPTS_PER_FRAME	(2)

static const int interrupt_lines[INTERRUPTS_PER_FRAME] = { 0x00, 0x80 };


static TIMER_CALLBACK( interrupt_callback )
{
	beaminv_state *state = (beaminv_state *)machine->driver_data;
	int interrupt_number = param;
	int next_interrupt_number;
	int next_vpos;

	cpu_set_input_line(state->maincpu, 0, HOLD_LINE);

	/* set up for next interrupt */
	next_interrupt_number = (interrupt_number + 1) % INTERRUPTS_PER_FRAME;
	next_vpos = interrupt_lines[next_interrupt_number];

	timer_adjust_oneshot(state->interrupt_timer, machine->primary_screen->time_until_pos(next_vpos), next_interrupt_number);
}


static void create_interrupt_timer( running_machine *machine )
{
	beaminv_state *state = (beaminv_state *)machine->driver_data;
	state->interrupt_timer = timer_alloc(machine, interrupt_callback, NULL);
}


static void start_interrupt_timer( running_machine *machine )
{
	beaminv_state *state = (beaminv_state *)machine->driver_data;
	int vpos = interrupt_lines[0];
	timer_adjust_oneshot(state->interrupt_timer, machine->primary_screen->time_until_pos(vpos), 0);
}



/*************************************
 *
 *  Machine setup
 *
 *************************************/

static MACHINE_START( beaminv )
{
	beaminv_state *state = (beaminv_state *)machine->driver_data;
	create_interrupt_timer(machine);

	state->maincpu = machine->device("maincpu");

	/* setup for save states */
	state_save_register_global(machine, state->controller_select);
}



/*************************************
 *
 *  Machine reset
 *
 *************************************/

static MACHINE_RESET( beaminv )
{
	beaminv_state *state = (beaminv_state *)machine->driver_data;
	start_interrupt_timer(machine);

	state->controller_select = 0;
}



/*************************************
 *
 *  Video system
 *
 *************************************/

static VIDEO_UPDATE( beaminv )
{
	beaminv_state *state = (beaminv_state *)screen->machine->driver_data;
	offs_t offs;

	for (offs = 0; offs < state->videoram_size; offs++)
	{
		int i;

		UINT8 y = offs;
		UINT8 x = offs >> 8 << 3;
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


static READ8_HANDLER( v128_r )
{
	return (space->machine->primary_screen->vpos() >> 7) & 0x01;
}



/*************************************
 *
 *  Controller
 *
 *************************************/

#define P1_CONTROL_PORT_TAG	("CONTP1")
#define P2_CONTROL_PORT_TAG	("CONTP2")


static WRITE8_HANDLER( controller_select_w )
{
	beaminv_state *state = (beaminv_state *)space->machine->driver_data;
	/* 0x01 (player 1) or 0x02 (player 2) */
	state->controller_select = data;
}


static READ8_HANDLER( controller_r )
{
	beaminv_state *state = (beaminv_state *)space->machine->driver_data;
	return input_port_read(space->machine, (state->controller_select == 1) ? P1_CONTROL_PORT_TAG : P2_CONTROL_PORT_TAG);
}



/*************************************
 *
 *  Memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x17ff) AM_ROM
	AM_RANGE(0x1800, 0x1fff) AM_RAM
	AM_RANGE(0x2400, 0x2400) AM_MIRROR(0x03ff) AM_READ_PORT("DSW")
	AM_RANGE(0x2800, 0x2800) AM_MIRROR(0x03ff) AM_READ_PORT("INPUTS")
	AM_RANGE(0x3400, 0x3400) AM_MIRROR(0x03ff) AM_READ(controller_r)
	AM_RANGE(0x3800, 0x3800) AM_MIRROR(0x03ff) AM_READ(v128_r)
	AM_RANGE(0x4000, 0x5fff) AM_RAM AM_BASE_SIZE_MEMBER(beaminv_state, videoram, videoram_size)
ADDRESS_MAP_END



/*************************************
 *
 *  Port handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(controller_select_w) /* to be confirmed */
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( beaminv )
	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPSETTING(    0x04, "2000" )
	PORT_DIPSETTING(    0x08, "3000" )
	PORT_DIPSETTING(    0x0c, "4000" )
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_HIGH )
	PORT_DIPNAME( 0x60, 0x00, "Faster Bombs At" )
	PORT_DIPSETTING(    0x00, "49 Enemies" )
	PORT_DIPSETTING(    0x20, "39 Enemies" )
	PORT_DIPSETTING(    0x40, "29 Enemies" )
	PORT_DIPSETTING(    0x60, "Never" )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_HIGH )

	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START(P1_CONTROL_PORT_TAG)
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(20) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_PLAYER(1)

	PORT_START(P2_CONTROL_PORT_TAG)
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(20) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( beaminva )
	PORT_INCLUDE( beaminv )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "1500" )
	PORT_DIPSETTING(    0x04, "2000" )
	PORT_DIPSETTING(    0x08, "2500" )
	PORT_DIPSETTING(    0x0c, "3000" )
	PORT_DIPNAME( 0x60, 0x00, "Faster Bombs At" )
	PORT_DIPSETTING(    0x00, "44 Enemies" )
	PORT_DIPSETTING(    0x20, "39 Enemies" )
	PORT_DIPSETTING(    0x40, "34 Enemies" )
	PORT_DIPSETTING(    0x40, "29 Enemies" )

	PORT_MODIFY(P1_CONTROL_PORT_TAG)
	PORT_BIT( 0xff, 0x65, IPT_PADDLE ) PORT_MINMAX(0x35,0x95) PORT_SENSITIVITY(20) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_PLAYER(1)

	PORT_MODIFY(P2_CONTROL_PORT_TAG)
	PORT_BIT( 0xff, 0x65, IPT_PADDLE ) PORT_MINMAX(0x35,0x95) PORT_SENSITIVITY(20) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_PLAYER(2)
INPUT_PORTS_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_DRIVER_START( beaminv )

	/* driver data */
	MDRV_DRIVER_DATA(beaminv_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, 2000000)	/* 2 MHz ? */
	MDRV_CPU_PROGRAM_MAP(main_map)
	MDRV_CPU_IO_MAP(main_io_map)

	MDRV_MACHINE_START(beaminv)
	MDRV_MACHINE_RESET(beaminv)

	/* video hardware */
	MDRV_VIDEO_UPDATE(beaminv)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 247, 16, 231)
	MDRV_SCREEN_REFRESH_RATE(60)

MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( beaminv )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "0a", 0x0000, 0x0400, CRC(17503086) SHA1(18c789216e5c4330dba3eeb24919dae636bf803d) )
	ROM_LOAD( "1a", 0x0400, 0x0400, CRC(aa9e1666) SHA1(050e2bd169f1502f49b7e6f5f2df9dac0d8107aa) )
	ROM_LOAD( "2a", 0x0800, 0x0400, CRC(ebaa2fc8) SHA1(b4ff1e1bdfe9efdc08873bba2f0a30d24678f9d8) )
	ROM_LOAD( "3a", 0x0c00, 0x0400, CRC(4f62c2e6) SHA1(4bd7d5e4f18d250003c7d771f1cdab08d699a765) )
	ROM_LOAD( "4a", 0x1000, 0x0400, CRC(3eebf757) SHA1(990eebda80ec52b7e3a36912c6e9230cd97f9f25) )
	ROM_LOAD( "5a", 0x1400, 0x0400, CRC(ec08bc1f) SHA1(e1df6704298e470a77158740c275fdca105e8f69) )
ROM_END


ROM_START( beaminva )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rom_0", 0x0000, 0x0400, CRC(67e100dd) SHA1(5f58e2ed3da14c48f7c382ee6091a59caf8e0609) )
	ROM_LOAD( "rom_1", 0x0400, 0x0400, CRC(442bbe98) SHA1(0e0382d4f6491629449759747019bd453a458b66) )
	ROM_LOAD( "rom_2", 0x0800, 0x0400, CRC(5d5d2f68) SHA1(e363f9445bbba1492188efe1830cae96f6078878) )
	ROM_LOAD( "rom_3", 0x0c00, 0x0400, CRC(527906b8) SHA1(9bda7da653db64246597ca386adab4cbab319189) )
	ROM_LOAD( "rom_4", 0x1000, 0x0400, CRC(920bb3f0) SHA1(3b9897d31c551e0b9193f775a6be65376b4a8c34) )
	ROM_LOAD( "rom_5", 0x1400, 0x0400, CRC(3f6980e4) SHA1(cb73cbc474677e6e302cb3842f32923ef2cdc98d) )
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1979, beaminv,  0,       beaminv, beaminv,  0, ROT270, "Tekunon Kougyou", "Beam Invader (set 1)", GAME_NO_SOUND | GAME_SUPPORTS_SAVE )
GAME( 1979, beaminva, beaminv, beaminv, beaminva, 0, ROT270, "Tekunon Kougyou", "Beam Invader (set 2)", GAME_NO_SOUND | GAME_SUPPORTS_SAVE ) // what's the real title ?
