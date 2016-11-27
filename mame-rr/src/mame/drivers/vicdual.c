/***************************************************************************

    VIC Dual Game board

    Games supported:
        * Depth Charge
        * Sub Hunter
        * Safari
        * Frogs
        * Space Attack
        * Space Attack / Head On
        * Head On
        * Head On 2
        * Invinco / Head On 2
        * N-Sub
        * Samurai
        * Invinco
        * Invinco / Deep Scan
        * Tranquillizer Gun
        * Space Trek
        * Carnival
        * Borderline
        * Digger
        * Pulsar
        * Heiankyo Alien
        * Alpha Fighter / Head On

        * and a few clones and bootlegs

    Notes:
        * Head On and Space Attack had both color and monochrome
          versions.  There is a game configuration option to
          switch between them
        * There existed a vertical version of Head On as well

    Known issues/to-do's:
        * Analog sound missing in many games
        * Missing color PROM for "Alpha Fighter / Head On"
        * A few of the games have an extra 18K pull-up resistor on the
          blue color gun, Carnival, for example.
          Colors inaccurate?  Blue background?
        * Does Digger run too fast compared to the real machine?
          The timing is implemented according to the schematics, but
          who knows...
        * DIP switches need verifying in most of the games
        * DIP switch locations need to be added to all the games

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/i8085/i8085.h"
#include "includes/vicdual.h"

#include "depthch.lh"



/*************************************
 *
 *  Coin handling
 *
 *************************************/

/* the main CPU is reset when a coin is inserted */

#define COIN_PORT_TAG		"COIN"

static UINT32 coin_status;


static TIMER_CALLBACK( clear_coin_status )
{
	coin_status = 0;
}


static void assert_coin_status(void)
{
	coin_status = 1;
}


static CUSTOM_INPUT( vicdual_read_coin_status )
{
	return coin_status;
}


static INPUT_CHANGED( coin_changed )
{
	if (newval && !oldval)
	{
		/* increment the coin counter */
		coin_counter_w(field->port->machine, 0, 1);
		coin_counter_w(field->port->machine, 0, 0);

		cputag_set_input_line(field->port->machine, "maincpu", INPUT_LINE_RESET, PULSE_LINE);

		/* simulate the coin switch being closed for a while */
		timer_set(field->port->machine, double_to_attotime(4 * attotime_to_double(field->port->machine->primary_screen->frame_period())), NULL, 0, clear_coin_status);
	}
}


#define PORT_COIN									\
	PORT_START(COIN_PORT_TAG)					\
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED(coin_changed, NULL) \
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )



/*************************************
 *
 *  Timing
 *
 *  Various games use various timing
 *  sources -- the only way to tell for
 *  sure which is which is from the
 *  schematics.
 *
 *************************************/

static int get_vcounter(running_machine *machine)
{
	int vcounter = machine->primary_screen->vpos();

	/* the vertical synch counter gets incremented at the end of HSYNC,
       compensate for this */
	if (machine->primary_screen->hpos() >= VICDUAL_HSEND)
		vcounter = (vcounter + 1) % VICDUAL_VTOTAL;

	return vcounter;
}


static CUSTOM_INPUT( vicdual_get_64v )
{
	return (get_vcounter(field->port->machine) >> 6) & 0x01;
}


static CUSTOM_INPUT( vicdual_get_vblank_comp )
{
	return (get_vcounter(field->port->machine) < VICDUAL_VBSTART);
}


static CUSTOM_INPUT( vicdual_get_composite_blank_comp )
{
	return (vicdual_get_vblank_comp(field, 0) && !field->port->machine->primary_screen->hblank());
}


static CUSTOM_INPUT( vicdual_get_timer_value )
{
	/* return the state of the timer (old code claims "4MHz square wave", but it was toggled once every 2msec, or 500Hz) */
	return attotime_to_ticks(timer_get_time(field->port->machine), 500) & 1;
}


/*************************************
 *
 *  Color vs. B&W configuration
 *
 *************************************/

#define COLOR_BW_PORT_TAG		"COLOR_BW"


int vicdual_is_cabinet_color(running_machine *machine)
{
	return (input_port_read(machine, COLOR_BW_PORT_TAG) == 0);
}


#define PORT_CABINET_COLOR_OR_BW						\
	PORT_START(COLOR_BW_PORT_TAG)					\
	PORT_CONFNAME( 0x01, 0x00, DEF_STR( Cabinet ) )		\
	PORT_CONFSETTING(    0x00, "Color" )				\
	PORT_CONFSETTING(    0x01, "Black and White" )		\
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static UINT8 *vicdual_videoram;
static UINT8 *vicdual_characterram;


static WRITE8_HANDLER( vicdual_videoram_w )
{
	space->machine->primary_screen->update_now();
	vicdual_videoram[offset] = data;
}


UINT8 vicdual_videoram_r(offs_t offset)
{
	return vicdual_videoram[offset];
}


static WRITE8_HANDLER( vicdual_characterram_w )
{
	space->machine->primary_screen->update_now();
	vicdual_characterram[offset] = data;
}


UINT8 vicdual_characterram_r(offs_t offset)
{
	return vicdual_characterram[offset];
}



/*************************************
 *
 *  Root driver structure
 *
 *************************************/

static MACHINE_DRIVER_START( vicdual_root )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, VICDUAL_MAIN_CPU_CLOCK)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_RAW_PARAMS(VICDUAL_PIXEL_CLOCK, VICDUAL_HTOTAL, VICDUAL_HBEND, VICDUAL_HBSTART, VICDUAL_VTOTAL, VICDUAL_VBEND, VICDUAL_VBSTART)

MACHINE_DRIVER_END



/*************************************
 *
 *  Depthcharge
 *
 *************************************/

static READ8_HANDLER( depthch_io_r )
{
	UINT8 ret = 0;

	if (offset & 0x01)  ret = input_port_read(space->machine, "IN0");
	if (offset & 0x08)  ret = input_port_read(space->machine, "IN1");

	return ret;
}


static WRITE8_HANDLER( depthch_io_w )
{
	if (offset & 0x01)  assert_coin_status();
	if (offset & 0x04)  depthch_audio_w(space, 0, data);
}


static ADDRESS_MAP_START( depthch_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_MIRROR(0x4000) AM_ROM
	AM_RANGE(0x8000, 0x83ff) AM_MIRROR(0x7000) AM_RAM_WRITE(vicdual_videoram_w) AM_BASE(&vicdual_videoram)
	AM_RANGE(0x8400, 0x87ff) AM_MIRROR(0x7000) AM_RAM
	AM_RANGE(0x8800, 0x8fff) AM_MIRROR(0x7000) AM_RAM_WRITE(vicdual_characterram_w) AM_BASE(&vicdual_characterram)
ADDRESS_MAP_END


static ADDRESS_MAP_START( depthch_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xf)

	/* no decoder, just logic gates, so in theory the
       game can read/write from multiple locations at once */
	AM_RANGE(0x00, 0x0f) AM_READWRITE(depthch_io_r, depthch_io_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( depthch )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_get_64v, NULL)
	PORT_BIT( 0x7e, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_read_coin_status, NULL)

	PORT_COIN
INPUT_PORTS_END


static MACHINE_DRIVER_START( depthch )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(vicdual_root)
	MDRV_CPU_REPLACE("maincpu", I8080, VICDUAL_MAIN_CPU_CLOCK)
	MDRV_CPU_PROGRAM_MAP(depthch_map)
	MDRV_CPU_IO_MAP(depthch_io_map)

	/* video hardware */
	MDRV_VIDEO_UPDATE(vicdual_bw)

	/* audio hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_IMPORT_FROM(depthch_audio)

MACHINE_DRIVER_END



/*************************************
 *
 *  Safari
 *
 *************************************/

static READ8_HANDLER( safari_io_r )
{
	UINT8 ret = 0;

	if (offset & 0x01)  ret = input_port_read(space->machine, "IN0");
	if (offset & 0x08)  ret = input_port_read(space->machine, "IN1");

	return ret;
}


static WRITE8_HANDLER( safari_io_w )
{
	if (offset & 0x01)  assert_coin_status();
	if (offset & 0x02) { /* safari_audio_w(0, data) */ }
}


static ADDRESS_MAP_START( safari_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
    AM_RANGE(0x4000, 0x7fff) AM_NOP	/* unused */
	AM_RANGE(0x8000, 0x8fff) AM_MIRROR(0x3000) AM_RAM
	AM_RANGE(0xc000, 0xc3ff) AM_MIRROR(0x3000) AM_RAM_WRITE(vicdual_videoram_w) AM_BASE(&vicdual_videoram)
	AM_RANGE(0xc400, 0xc7ff) AM_MIRROR(0x3000) AM_RAM
	AM_RANGE(0xc800, 0xcfff) AM_MIRROR(0x3000) AM_RAM_WRITE(vicdual_characterram_w) AM_BASE(&vicdual_characterram)
ADDRESS_MAP_END


static ADDRESS_MAP_START( safari_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xf)

	/* no decoder, just logic gates, so in theory the
       game can read/write from multiple locations at once */
	AM_RANGE(0x00, 0x0f) AM_READWRITE(safari_io_r, safari_io_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( safari )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Aim Up") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Aim Down") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_get_64v, NULL)
	PORT_BIT( 0x0e, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_read_coin_status, NULL)

	PORT_COIN
INPUT_PORTS_END


static MACHINE_DRIVER_START( safari )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(vicdual_root)
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(safari_map)
	MDRV_CPU_IO_MAP(safari_io_map)

	/* video hardware */
	MDRV_VIDEO_UPDATE(vicdual_bw)

MACHINE_DRIVER_END



/*************************************
 *
 *  Frogs
 *
 *************************************/

static READ8_HANDLER( frogs_io_r )
{
	UINT8 ret = 0;

	if (offset & 0x01)  ret = input_port_read(space->machine, "IN0");
	if (offset & 0x08)  ret = input_port_read(space->machine, "IN1");

	return ret;
}


static WRITE8_HANDLER( frogs_io_w )
{
	if (offset & 0x01)  assert_coin_status();
	if (offset & 0x02)  frogs_audio_w(space, 0, data);
}


static ADDRESS_MAP_START( frogs_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_MIRROR(0x4000) AM_ROM
	AM_RANGE(0x8000, 0x83ff) AM_MIRROR(0x7000) AM_RAM_WRITE(vicdual_videoram_w) AM_BASE(&vicdual_videoram)
	AM_RANGE(0x8400, 0x87ff) AM_MIRROR(0x7000) AM_RAM
	AM_RANGE(0x8800, 0x8fff) AM_MIRROR(0x7000) AM_RAM_WRITE(vicdual_characterram_w) AM_BASE(&vicdual_characterram)
ADDRESS_MAP_END


static ADDRESS_MAP_START( frogs_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xf)

	/* no decoder, just logic gates, so in theory the
       game can read/write from multiple locations at once */
	AM_RANGE(0x00, 0x0f) AM_READWRITE(frogs_io_r, frogs_io_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( frogs )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY	/* The original joystick was a 3-way */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY	/* stick, of which Mame's 4-way does */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY	/* a fine simulation */
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Allow Free Game" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, "Time" )
	PORT_DIPSETTING(    0x00, "60" )
	PORT_DIPSETTING(    0x20, "90" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_1C ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_get_64v, NULL)
	PORT_BIT( 0x7e, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_read_coin_status, NULL)

	PORT_COIN

//  PORT_START("IN2")
//  PORT_ADJUSTER( 25, "Boing Volume" )

//  PORT_START("IN3")
//  PORT_ADJUSTER( 25, "Buzzz Volume" )

//  PORT_START("IN4")
//  PORT_ADJUSTER( 25, "Croak Volume" )

//  PORT_START("IN5")
//  PORT_ADJUSTER( 25, "Hop Volume" )

//  PORT_START("IN6")
//  PORT_ADJUSTER( 50, "Splash Volume" )

	PORT_START("R93")
	PORT_ADJUSTER( 50, "Zip Volume" )
INPUT_PORTS_END


static MACHINE_DRIVER_START( frogs )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(vicdual_root)
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(frogs_map)
	MDRV_CPU_IO_MAP(frogs_io_map)
	MDRV_MACHINE_START(frogs_audio)

	/* video hardware */
	MDRV_VIDEO_UPDATE(vicdual_bw)

	/* audio hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_IMPORT_FROM(frogs_audio)

MACHINE_DRIVER_END



/*************************************
 *
 *  Head On
 *  Space Attack
 *
 *************************************/

static READ8_HANDLER( headon_io_r )
{
	UINT8 ret = 0;

	if (offset & 0x01)  ret = input_port_read(space->machine, "IN0");
	if (offset & 0x08)  ret = input_port_read(space->machine, "IN1");

	return ret;
}


static READ8_HANDLER( sspaceat_io_r )
{
	UINT8 ret = 0;

	if (offset & 0x01)  ret = input_port_read(space->machine, "IN0");
	if (offset & 0x04)  ret = input_port_read(space->machine, "IN1");
	if (offset & 0x08)  ret = input_port_read(space->machine, "IN2");

	return ret;
}


static WRITE8_HANDLER( headon_io_w )
{
	if (offset & 0x01)  assert_coin_status();
	if (offset & 0x02)  headon_audio_w(space, 0, data);
	if (offset & 0x04) { /* vicdual_palette_bank_w(0, data)  */ }	 /* not written to */
}


static ADDRESS_MAP_START( headon_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_MIRROR(0x6000) AM_ROM
    AM_RANGE(0x8000, 0xbfff) AM_NOP	/* unused */
	AM_RANGE(0xc000, 0xc3ff) AM_MIRROR(0x3000) AM_RAM_WRITE(vicdual_videoram_w) AM_BASE(&vicdual_videoram)
	AM_RANGE(0xc400, 0xc7ff) AM_MIRROR(0x3000) AM_RAM
	AM_RANGE(0xc800, 0xcfff) AM_MIRROR(0x3000) AM_RAM_WRITE(vicdual_characterram_w) AM_BASE(&vicdual_characterram)
ADDRESS_MAP_END


static ADDRESS_MAP_START( headon_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xf)

	/* no decoder, just logic gates, so in theory the
       game can read/write from multiple locations at once */
	AM_RANGE(0x00, 0x0f) AM_READWRITE(headon_io_r, headon_io_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( sspaceat_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xf)

	/* no decoder, just logic gates, so in theory the
       game can read/write from multiple locations at once */
	AM_RANGE(0x00, 0x0f) AM_READWRITE(sspaceat_io_r, headon_io_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( headon )
	PORT_START("IN0")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_get_64v, NULL)
	PORT_BIT( 0x7e, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_read_coin_status, NULL)

	PORT_CABINET_COLOR_OR_BW

	PORT_COIN
INPUT_PORTS_END




static INPUT_PORTS_START( supcrash )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
    PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
    PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
    PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
    PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_get_timer_value, NULL)
    PORT_DIPNAME( 0x04, 0x04, "Rom Test" )
    PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )
    PORT_BIT( 0x7a, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_read_coin_status, NULL)

	PORT_CABINET_COLOR_OR_BW

	PORT_COIN
INPUT_PORTS_END


static INPUT_PORTS_START( sspaceat )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x00, "Bonus Life For Final UFO" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0e, 0x0e, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x0e, "3" )
	PORT_DIPSETTING(    0x0c, "4" )
	PORT_DIPSETTING(    0x0a, "5" )
	PORT_DIPSETTING(    0x06, "6" )
/* the following are duplicates
    PORT_DIPSETTING(    0x00, "4" )
    PORT_DIPSETTING(    0x04, "4" )
    PORT_DIPSETTING(    0x08, "4" )
    PORT_DIPSETTING(    0x02, "5" ) */
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x10, "15000" )
	PORT_BIT( 0x60, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x80, 0x00, "Credits Display" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_get_timer_value, NULL)
	PORT_BIT( 0x7e, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_read_coin_status, NULL)

	PORT_CABINET_COLOR_OR_BW

	PORT_COIN
INPUT_PORTS_END


static MACHINE_DRIVER_START( headon )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(vicdual_root)
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(headon_map)
	MDRV_CPU_IO_MAP(headon_io_map)

	/* video hardware */
	MDRV_VIDEO_UPDATE(vicdual_bw_or_color)

	/* audio hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_IMPORT_FROM(headon_audio)

MACHINE_DRIVER_END

static MACHINE_DRIVER_START( headons )
	MDRV_IMPORT_FROM(headon)

	/* video hardware */
	MDRV_VIDEO_UPDATE(vicdual_bw) // no colour prom on PCB, must be bw?
MACHINE_DRIVER_END



static MACHINE_DRIVER_START( sspaceat )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(vicdual_root)
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(headon_map)
	MDRV_CPU_IO_MAP(sspaceat_io_map)

	/* video hardware */
	MDRV_VIDEO_UPDATE(vicdual_bw_or_color)

MACHINE_DRIVER_END



/*************************************
 *
 *  Head On 2
 *  Digger
 *
 *************************************/

static READ8_HANDLER( headon2_io_r )
{
	UINT8 ret = 0;

	if (offset & 0x01)  ret = input_port_read(space->machine, "IN0");
	if (offset & 0x02)  /* schematics show this as in input port, but never read from */
	if (offset & 0x04)  ret = input_port_read(space->machine, "IN1");
	if (offset & 0x08)  ret = input_port_read(space->machine, "IN2");
	if (offset & 0x12)  logerror("********* Read from port %x\n", offset);

	return ret;
}


static WRITE8_HANDLER( headon2_io_w )
{
	if (offset & 0x01)  assert_coin_status();
	if (offset & 0x02)  headon_audio_w(space, 0, data);
	if (offset & 0x04)  vicdual_palette_bank_w(space, 0, data);
    if (offset & 0x08) { /* schematics show this as going into a shifer circuit, but never written to */ }
    if (offset & 0x10) { /* schematics show this as going to an edge connector, but never written to */ }
	if (offset & 0x18)  logerror("********* Write to port %x\n", offset);
}


static WRITE8_HANDLER( digger_io_w )
{
	if (offset & 0x01)  assert_coin_status();
	if (offset & 0x02) { /* digger_audio_1_w(0, data) */ }
	if (offset & 0x04)
	{
		vicdual_palette_bank_w(space, 0, data & 0x03);
		/* digger_audio_2_w(0, data & 0xfc); */
	}

    if (offset & 0x08) { /* schematics show this as going into a shifer circuit, but never written to */ }
    if (offset & 0x10) { /* schematics show this as going to an edge connector, but never written to */ }
	if (offset & 0x18)  logerror("********* Write to port %x\n", offset);
}


static ADDRESS_MAP_START( headon2_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_MIRROR(0x6000) AM_ROM
 /* AM_RANGE(0x8000, 0x80ff) AM_MIRROR(0x3f00) */  /* schematics show this as battery backed RAM, but doesn't appear to be used */
	AM_RANGE(0xc000, 0xc3ff) AM_MIRROR(0x3000) AM_RAM_WRITE(vicdual_videoram_w) AM_BASE(&vicdual_videoram)
	AM_RANGE(0xc400, 0xc7ff) AM_MIRROR(0x3000) AM_RAM
	AM_RANGE(0xc800, 0xcfff) AM_MIRROR(0x3000) AM_RAM_WRITE(vicdual_characterram_w) AM_BASE(&vicdual_characterram)
ADDRESS_MAP_END


static ADDRESS_MAP_START( headon2_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0x1f)

	/* no decoder, just logic gates, so in theory the
       game can read/write from multiple locations at once */
	AM_RANGE(0x00, 0x1f) AM_READWRITE(headon2_io_r, headon2_io_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( digger_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0x1f)

	/* no decoder, just logic gates, so in theory the
       game can read/write from multiple locations at once */
	AM_RANGE(0x00, 0x1f) AM_READWRITE(headon2_io_r, digger_io_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( headon2 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY

	PORT_START("IN1")
	PORT_BIT( 0x07, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x18, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x00, "6" )
  /*PORT_DIPSETTING(    0x08, "5" )*/
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_get_timer_value, NULL)
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_BIT( 0x7c, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_read_coin_status, NULL)

	PORT_COIN
INPUT_PORTS_END

/* this actually seems to ignore the dipswitches and is hardcoded to 2 coins 1 credit, and 2 lives */
static INPUT_PORTS_START( car2 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	/* controls are active_high around on this bootleg */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY

	PORT_START("IN1")
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* probably unused */
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x18, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x00, "6" )
  /*PORT_DIPSETTING(    0x08, "5" )*/
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* probably unused */

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_get_timer_value, NULL)
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_BIT( 0x7c, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_read_coin_status, NULL)

	PORT_COIN
INPUT_PORTS_END

static INPUT_PORTS_START( digger )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY

	PORT_START("IN1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x60, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_get_timer_value, NULL)
	PORT_BIT( 0x7e, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_read_coin_status, NULL)

	PORT_COIN
INPUT_PORTS_END


static MACHINE_DRIVER_START( headon2 )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(vicdual_root)
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(headon2_map)
	MDRV_CPU_IO_MAP(headon2_io_map)

	/* video hardware */
	MDRV_VIDEO_UPDATE(vicdual_color)

	/* audio hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_IMPORT_FROM(headon_audio)

MACHINE_DRIVER_END

static MACHINE_DRIVER_START( headon2bw )
	/* basic machine hardware */
	MDRV_IMPORT_FROM(headon2)
	/* video hardware */
	MDRV_VIDEO_UPDATE(vicdual_bw)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( digger )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(vicdual_root)
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(headon2_map)
	MDRV_CPU_IO_MAP(digger_io_map)

	/* video hardware */
	MDRV_VIDEO_UPDATE(vicdual_color)

MACHINE_DRIVER_END



/*************************************
 *
 *  Invinco / Head On 2
 *  Invinco / Deap Scan
 *  Space Attack / Head On
 *  Tranquillizer Gun
 *  Space Trek
 *  Carnival
 *  Borderline
 *  Pulsar
 *  Heiankyo Alien
 *  Alpha Fighter / Head On
 *
 *************************************/

static WRITE8_HANDLER( invho2_io_w )
{
	if (offset & 0x01)  invho2_audio_w(space, 0, data);
	if (offset & 0x02)  invinco_audio_w(space, 0, data);
	if (offset & 0x08)  assert_coin_status();
	if (offset & 0x40)  vicdual_palette_bank_w(space, 0, data);
}


static WRITE8_HANDLER( invds_io_w )
{
	if (offset & 0x01)  invinco_audio_w(space, 0, data);
	if (offset & 0x02) { /* deepscan_audio_w(0, data) */ }
	if (offset & 0x08)  assert_coin_status();
	if (offset & 0x40)  vicdual_palette_bank_w(space, 0, data);
}


static WRITE8_HANDLER( sspacaho_io_w )
{
	if (offset & 0x01)  invho2_audio_w(space, 0, data);
	if (offset & 0x02) { /* sspaceatt_audio_w(space, 0, data) */ }
	if (offset & 0x08)  assert_coin_status();
	if (offset & 0x40)  vicdual_palette_bank_w(space, 0, data);
}


static WRITE8_HANDLER( tranqgun_io_w )
{
	if (offset & 0x01) { /* tranqgun_audio_w(space, 0, data) */ }
	if (offset & 0x02)  vicdual_palette_bank_w(space, 0, data);
	if (offset & 0x08)  assert_coin_status();
}


static WRITE8_HANDLER( spacetrk_io_w )
{
	if (offset & 0x01) { /* spacetrk_audio_w(space, 0, data) */ }
	if (offset & 0x02) { /* spacetrk_audio_w(space, 0, data) */ }
	if (offset & 0x08)  assert_coin_status();
	if (offset & 0x40)  vicdual_palette_bank_w(space, 0, data);
}


static WRITE8_HANDLER( carnival_io_w )
{
	if (offset & 0x01)  carnival_audio_1_w(space, 0, data);
	if (offset & 0x02)  carnival_audio_2_w(space, 0, data);
	if (offset & 0x08)  assert_coin_status();
	if (offset & 0x40)  vicdual_palette_bank_w(space, 0, data);
}


static WRITE8_HANDLER( brdrline_io_w )
{
	if (offset & 0x01) { /* brdrline_audio_w(space, 0, data) */ }
	if (offset & 0x02)  vicdual_palette_bank_w(space, 0, data);
	if (offset & 0x08)  assert_coin_status();
}


static WRITE8_HANDLER( pulsar_io_w )
{
	if (offset & 0x01)  pulsar_audio_1_w(space, 0, data);
	if (offset & 0x02)  pulsar_audio_2_w(space, 0, data);
	if (offset & 0x08)  assert_coin_status();
	if (offset & 0x40)  vicdual_palette_bank_w(space, 0, data);
}


static WRITE8_HANDLER( heiankyo_io_w )
{
	if (offset & 0x01) { /* heiankyo_audio_1_w(0, data) */ }

	if (offset & 0x02)
	{
		vicdual_palette_bank_w(space, 0, data & 0x03);
		/* heiankyo_audio_2_w(0, data & 0xfc); */
	}

	if (offset & 0x08)  assert_coin_status();
}


static WRITE8_HANDLER( alphaho_io_w )
{
	if (offset & 0x01) { /* headon_audio_w(0, data) */ }
	if (offset & 0x02) { /* alphaf_audio_w(0, data) */ }
	if (offset & 0x08)  assert_coin_status();
	if (offset & 0x40)  vicdual_palette_bank_w(space, 0, data);
}


static ADDRESS_MAP_START( vicdual_dualgame_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_MIRROR(0x4000) AM_ROM
	AM_RANGE(0x8000, 0x83ff) AM_MIRROR(0x7000) AM_RAM_WRITE(vicdual_videoram_w) AM_BASE(&vicdual_videoram)
	AM_RANGE(0x8400, 0x87ff) AM_MIRROR(0x7000) AM_RAM
	AM_RANGE(0x8800, 0x8fff) AM_MIRROR(0x7000) AM_RAM_WRITE(vicdual_characterram_w) AM_BASE(&vicdual_characterram)
ADDRESS_MAP_END


static ADDRESS_MAP_START( invho2_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0x7f)

	AM_RANGE(0x00, 0x00) AM_MIRROR(0x7c) AM_READ_PORT("IN0")
	AM_RANGE(0x01, 0x01) AM_MIRROR(0x7c) AM_READ_PORT("IN1")
	AM_RANGE(0x02, 0x02) AM_MIRROR(0x7c) AM_READ_PORT("IN2")
	AM_RANGE(0x03, 0x03) AM_MIRROR(0x7c) AM_READ_PORT("IN3")

	/* no decoder, just logic gates, so in theory the
       game can write to multiple locations at once */
	AM_RANGE(0x00, 0x7f) AM_WRITE(invho2_io_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( invds_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0x7f)

	AM_RANGE(0x00, 0x00) AM_MIRROR(0x7c) AM_READ_PORT("IN0")
	AM_RANGE(0x01, 0x01) AM_MIRROR(0x7c) AM_READ_PORT("IN1")
	AM_RANGE(0x02, 0x02) AM_MIRROR(0x7c) AM_READ_PORT("IN2")
	AM_RANGE(0x03, 0x03) AM_MIRROR(0x7c) AM_READ_PORT("IN3")

	/* no decoder, just logic gates, so in theory the
       game can write to multiple locations at once */
	AM_RANGE(0x00, 0x7f) AM_WRITE(invds_io_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( sspacaho_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0x7f)

	AM_RANGE(0x00, 0x00) AM_MIRROR(0x7c) AM_READ_PORT("IN0")
	AM_RANGE(0x01, 0x01) AM_MIRROR(0x7c) AM_READ_PORT("IN1")
	AM_RANGE(0x02, 0x02) AM_MIRROR(0x7c) AM_READ_PORT("IN2")
	AM_RANGE(0x03, 0x03) AM_MIRROR(0x7c) AM_READ_PORT("IN3")

	/* no decoder, just logic gates, so in theory the
       game can write to multiple locations at once */
	AM_RANGE(0x00, 0x7f) AM_WRITE(sspacaho_io_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( tranqgun_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xf)

	AM_RANGE(0x00, 0x00) AM_MIRROR(0x0c) AM_READ_PORT("IN0")
	AM_RANGE(0x01, 0x01) AM_MIRROR(0x0c) AM_READ_PORT("IN1")
	AM_RANGE(0x02, 0x02) AM_MIRROR(0x0c) AM_READ_PORT("IN2")
	AM_RANGE(0x03, 0x03) AM_MIRROR(0x0c) AM_READ_PORT("IN3")

	/* no decoder, just logic gates, so in theory the
       game can write to multiple locations at once */
	AM_RANGE(0x00, 0x0f) AM_WRITE(tranqgun_io_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( spacetrk_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0x7f)

	AM_RANGE(0x00, 0x00) AM_MIRROR(0x7c) AM_READ_PORT("IN0")
	AM_RANGE(0x01, 0x01) AM_MIRROR(0x7c) AM_READ_PORT("IN1")
	AM_RANGE(0x02, 0x02) AM_MIRROR(0x7c) AM_READ_PORT("IN2")
	AM_RANGE(0x03, 0x03) AM_MIRROR(0x7c) AM_READ_PORT("IN3")

	/* no decoder, just logic gates, so in theory the
       game can write to multiple locations at once */
	AM_RANGE(0x00, 0x7f) AM_WRITE(spacetrk_io_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( carnival_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0x7f)

	AM_RANGE(0x00, 0x00) AM_MIRROR(0x7c) AM_READ_PORT("IN0")
	AM_RANGE(0x01, 0x01) AM_MIRROR(0x7c) AM_READ_PORT("IN1")
	AM_RANGE(0x02, 0x02) AM_MIRROR(0x7c) AM_READ_PORT("IN2")
	AM_RANGE(0x03, 0x03) AM_MIRROR(0x7c) AM_READ_PORT("IN3")

	/* no decoder, just logic gates, so in theory the
       game can write to multiple locations at once */
	AM_RANGE(0x00, 0x7f) AM_WRITE(carnival_io_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( brdrline_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xf)

	AM_RANGE(0x00, 0x00) AM_MIRROR(0x0c) AM_READ_PORT("IN0")
	AM_RANGE(0x01, 0x01) AM_MIRROR(0x0c) AM_READ_PORT("IN1")
	AM_RANGE(0x02, 0x02) AM_MIRROR(0x0c) AM_READ_PORT("IN2")
	AM_RANGE(0x03, 0x03) AM_MIRROR(0x0c) AM_READ_PORT("IN3")

	/* no decoder, just logic gates, so in theory the
       game can write to multiple locations at once */
	AM_RANGE(0x00, 0x0f) AM_WRITE(brdrline_io_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( pulsar_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0x7f)

	AM_RANGE(0x00, 0x00) AM_MIRROR(0x7c) AM_READ_PORT("IN0")
	AM_RANGE(0x01, 0x01) AM_MIRROR(0x7c) AM_READ_PORT("IN1")
	AM_RANGE(0x02, 0x02) AM_MIRROR(0x7c) AM_READ_PORT("IN2")
	AM_RANGE(0x03, 0x03) AM_MIRROR(0x7c) AM_READ_PORT("IN3")

	/* no decoder, just logic gates, so in theory the
       game can write to multiple locations at once */
	AM_RANGE(0x00, 0x7f) AM_WRITE(pulsar_io_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( heiankyo_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xf)

	AM_RANGE(0x00, 0x00) AM_MIRROR(0x0c) AM_READ_PORT("IN0")
	AM_RANGE(0x01, 0x01) AM_MIRROR(0x0c) AM_READ_PORT("IN1")
	AM_RANGE(0x02, 0x02) AM_MIRROR(0x0c) AM_READ_PORT("IN2")
	AM_RANGE(0x03, 0x03) AM_MIRROR(0x0c) AM_READ_PORT("IN3")

	/* no decoder, just logic gates, so in theory the
       game can write to multiple locations at once */
	AM_RANGE(0x00, 0x0f) AM_WRITE(heiankyo_io_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( alphaho_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0x7f)

	AM_RANGE(0x00, 0x00) AM_MIRROR(0x7c) AM_READ_PORT("IN0")
	AM_RANGE(0x01, 0x01) AM_MIRROR(0x7c) AM_READ_PORT("IN1")
	AM_RANGE(0x02, 0x02) AM_MIRROR(0x7c) AM_READ_PORT("IN2")
	AM_RANGE(0x03, 0x03) AM_MIRROR(0x7c) AM_READ_PORT("IN3")

	/* no decoder, just logic gates, so in theory the
       game can write to multiple locations at once */
	AM_RANGE(0x00, 0x7f) AM_WRITE(alphaho_io_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( invho2 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_DIPNAME( 0x04, 0x04, "Head On Lives (1/2)" )
	PORT_DIPSETTING(    0x04, "+0" )
	PORT_DIPSETTING(    0x00, "+1" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_DIPNAME( 0x04, 0x00, "Head On Lives (2/2)" )
	PORT_DIPSETTING(    0x04, "+0" )
	PORT_DIPSETTING(    0x00, "+1" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_get_composite_blank_comp, NULL)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_DIPNAME( 0x04, 0x00, "Invinco Lives" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x04, "6" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_get_timer_value, NULL)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )	/* probably unused */

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	/* There's probably a bug in the code: this would likely be the second */
	/* bit of the Invinco Lives setting, but the game reads bit 3 instead */
	/* of bit 2. */
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_read_coin_status, NULL)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Game Select") PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_COIN
INPUT_PORTS_END


static INPUT_PORTS_START( invds )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_DIPNAME( 0x04, 0x00, "Invinco Lives (1/2)" )
	PORT_DIPSETTING(    0x00, "+0" )
	PORT_DIPSETTING(    0x04, "+1" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_DIPNAME( 0x04, 0x00, "Invinco Lives (2/2)" )
	PORT_DIPSETTING(    0x00, "+0" )
	PORT_DIPSETTING(    0x04, "+2" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_get_composite_blank_comp, NULL)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_DIPNAME( 0x04, 0x00, "Deep Scan Lives (1/2)" )
	PORT_DIPSETTING(    0x00, "+0" )
	PORT_DIPSETTING(    0x04, "+1" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_get_timer_value, NULL)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	/* +1 and +2 gives 2 lives instead of 6 */
	PORT_DIPNAME( 0x04, 0x00, "Deep Scan Lives (2/2)" )
	PORT_DIPSETTING(    0x04, "+0" )
	PORT_DIPSETTING(    0x00, "+2" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_read_coin_status, NULL)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Game Select") PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_COIN
INPUT_PORTS_END


static INPUT_PORTS_START( sspacaho )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_DIPNAME( 0x04, 0x04, "S.A. Lives (1/2)" )
	PORT_DIPSETTING(    0x00, "+0" )
	PORT_DIPSETTING(    0x04, "+1" )
	PORT_DIPNAME( 0x08, 0x00, "H.O. Lives" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x04, 0x00, "S.A. Lives (2/2)" )
	PORT_DIPSETTING(    0x00, "+0" )
	PORT_DIPSETTING(    0x04, "+2" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_get_composite_blank_comp, NULL)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x04, 0x00, "S.A. Bonus Life" )
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x04, "15000" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_get_timer_value, NULL)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x04, 0x00, "S.A. Bonus Life For Final UFO" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_read_coin_status, NULL)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Game Select") PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_COIN
INPUT_PORTS_END


static INPUT_PORTS_START( tranqgun )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_get_vblank_comp, NULL)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_get_timer_value, NULL)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_read_coin_status, NULL)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_COIN
INPUT_PORTS_END


static INPUT_PORTS_START( spacetrk )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) ) /* unknown, but used */
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_get_composite_blank_comp, NULL)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* must be high for bonus life to work */
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_get_timer_value, NULL)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* must be high for bonus life to work */
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_read_coin_status, NULL)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_COIN
INPUT_PORTS_END


static INPUT_PORTS_START( sptrekct )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) ) /* unknown, but used */
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_get_composite_blank_comp, NULL)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* must be high for bonus life to work */
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_get_timer_value, NULL)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* must be high for bonus life to work */
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_read_coin_status, NULL)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_COIN
INPUT_PORTS_END


static INPUT_PORTS_START( carnival )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_get_composite_blank_comp, NULL)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_get_timer_value, NULL)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_read_coin_status, NULL)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_COIN
INPUT_PORTS_END

/* not verified */
static INPUT_PORTS_START( carnivalh )
	PORT_START("IN0")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_get_64v, NULL)
	PORT_BIT( 0x7e, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_read_coin_status, NULL)

	PORT_CABINET_COLOR_OR_BW

	PORT_COIN
INPUT_PORTS_END


static INPUT_PORTS_START( carnvckt )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_get_composite_blank_comp, NULL)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_get_timer_value, NULL)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_read_coin_status, NULL)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_COIN
INPUT_PORTS_END


static INPUT_PORTS_START( brdrline )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_DIPNAME( 0x04, 0x04, "Infinite Lives" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP	 ) PORT_4WAY
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_get_vblank_comp, NULL)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_get_64v, NULL)	/* yes, this is different */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_read_coin_status, NULL)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_COIN
INPUT_PORTS_END


static INPUT_PORTS_START( pulsar )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_DIPNAME( 0x04, 0x04, "Lives (1/2)" )
	PORT_DIPSETTING(    0x04, "+0" )
	PORT_DIPSETTING(    0x00, "+2" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_DIPNAME( 0x04, 0x04, "Lives (2/2)" )
	PORT_DIPSETTING(    0x04, "+0" )
	PORT_DIPSETTING(    0x00, "+1" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_get_composite_blank_comp, NULL)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_get_timer_value, NULL)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_read_coin_status, NULL)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_COIN
INPUT_PORTS_END


static INPUT_PORTS_START( heiankyo )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) /* bonus life? */
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "2 Players Mode" )
	PORT_DIPSETTING(    0x08, "Alternating" )
	PORT_DIPSETTING(    0x00, "Simultaneous" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) /* bonus life? */
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_get_composite_blank_comp, NULL)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* has to be 0, protection? */
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) /* bonus life? */
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_get_timer_value, NULL)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* has to be 0, protection? */
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )	/* probably unused */

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_read_coin_status, NULL)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_COIN
INPUT_PORTS_END


static INPUT_PORTS_START( alphaho )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_DIPNAME( 0x04, 0x00, "Alpha Fighter Lives (1/2)" )
	PORT_DIPSETTING(    0x00, "+0" )
	PORT_DIPSETTING(    0x04, "+1" )
	PORT_DIPNAME( 0x08, 0x00, "Head On Lives" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_DIPNAME( 0x04, 0x00, "Alpha Fighter Lives (2/2)" )
	PORT_DIPSETTING(    0x00, "+0" )
	PORT_DIPSETTING(    0x04, "+2" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_get_composite_blank_comp, NULL)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_get_timer_value, NULL)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_DIPNAME( 0x04, 0x00, "Alpha Fighter Unknown" )	// related to soccer frequency (code at 0x4950)
	PORT_DIPSETTING(    0x00, DEF_STR ( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR ( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_read_coin_status, NULL)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Game Select") PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */

	PORT_COIN
INPUT_PORTS_END


static MACHINE_DRIVER_START( vicdual_dualgame_root )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(vicdual_root)
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(vicdual_dualgame_map)

	/* video hardware */
	MDRV_VIDEO_UPDATE(vicdual_color)

MACHINE_DRIVER_END


static MACHINE_DRIVER_START( invho2 )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(vicdual_dualgame_root)
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_IO_MAP(invho2_io_map)

	/* audio hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_IMPORT_FROM(invinco_audio)
	MDRV_IMPORT_FROM(headon_audio)

MACHINE_DRIVER_END


static MACHINE_DRIVER_START( invds )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(vicdual_dualgame_root)
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_IO_MAP(invds_io_map)

	/* audio hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_IMPORT_FROM(invinco_audio)

MACHINE_DRIVER_END


static MACHINE_DRIVER_START( sspacaho )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(vicdual_dualgame_root)
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_IO_MAP(sspacaho_io_map)

	/* audio hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_IMPORT_FROM(headon_audio)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( spacetrk )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(vicdual_dualgame_root)
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_IO_MAP(spacetrk_io_map)

MACHINE_DRIVER_END


static MACHINE_DRIVER_START( carnival )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(vicdual_dualgame_root)
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_IO_MAP(carnival_io_map)

	/* audio hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_IMPORT_FROM(carnival_audio)

MACHINE_DRIVER_END

static MACHINE_DRIVER_START( carnivalh )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(vicdual_dualgame_root)
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_IO_MAP(headon_io_map)

	/* audio hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_IMPORT_FROM(carnival_audio)

MACHINE_DRIVER_END



static MACHINE_DRIVER_START( tranqgun )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(vicdual_dualgame_root)
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_IO_MAP(tranqgun_io_map)

MACHINE_DRIVER_END


static MACHINE_DRIVER_START( brdrline )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(vicdual_dualgame_root)
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_IO_MAP(brdrline_io_map)

MACHINE_DRIVER_END


static MACHINE_DRIVER_START( pulsar )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(vicdual_dualgame_root)
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_IO_MAP(pulsar_io_map)

	/* audio hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_IMPORT_FROM(pulsar_audio)

MACHINE_DRIVER_END


static MACHINE_DRIVER_START( heiankyo )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(vicdual_dualgame_root)
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_IO_MAP(heiankyo_io_map)

MACHINE_DRIVER_END


static MACHINE_DRIVER_START( alphaho )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(vicdual_dualgame_root)
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_IO_MAP(alphaho_io_map)

MACHINE_DRIVER_END



/*************************************
 *
 *  Samurai
 *
 *************************************/

static UINT8 samurai_protection_data;


static WRITE8_HANDLER( samurai_protection_w )
{
	samurai_protection_data = data;
}


static CUSTOM_INPUT( samurai_protection_r )
{
	int offset = (FPTR)param;
	UINT32 answer = 0;

	if (samurai_protection_data == 0xab)
		answer = 0x02;
	else if (samurai_protection_data == 0x1d)
		answer = 0x0c;

	return (answer >> offset) & 0x01;
}


static WRITE8_HANDLER( samurai_io_w )
{
	if (offset & 0x02) { /* samurai_audio_w(0, data) */ }
	if (offset & 0x08)  assert_coin_status();
	if (offset & 0x40)  vicdual_palette_bank_w(space, 0, data);
}


/* dual game hardware */
static ADDRESS_MAP_START( samurai_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_MIRROR(0x4000) AM_ROM AM_WRITE(samurai_protection_w)
	AM_RANGE(0x8000, 0x83ff) AM_MIRROR(0x7000) AM_RAM_WRITE(vicdual_videoram_w) AM_BASE(&vicdual_videoram)
	AM_RANGE(0x8400, 0x87ff) AM_MIRROR(0x7000) AM_RAM
	AM_RANGE(0x8800, 0x8fff) AM_MIRROR(0x7000) AM_RAM_WRITE(vicdual_characterram_w) AM_BASE(&vicdual_characterram)
ADDRESS_MAP_END


static ADDRESS_MAP_START( samurai_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0x7f)

	AM_RANGE(0x00, 0x00) AM_MIRROR(0x7c) AM_READ_PORT("IN0")
	AM_RANGE(0x01, 0x01) AM_MIRROR(0x7c) AM_READ_PORT("IN1")
	AM_RANGE(0x02, 0x02) AM_MIRROR(0x7c) AM_READ_PORT("IN2")
	AM_RANGE(0x03, 0x03) AM_MIRROR(0x7c) AM_READ_PORT("IN3")

	/* no decoder, just logic gates, so in theory the
       game can write to multiple locations at once */
	AM_RANGE(0x00, 0x7f) AM_WRITE(samurai_io_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( samurai )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x08, 0x08, "Infinite Lives" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(samurai_protection_r, (void *)1)
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) ) /* unknown, but used */
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_get_composite_blank_comp, NULL)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(samurai_protection_r, (void *)2)
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_get_timer_value, NULL)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(samurai_protection_r, (void *)3)
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_read_coin_status, NULL)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_COIN
INPUT_PORTS_END


static MACHINE_DRIVER_START( samurai )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(vicdual_root)
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(samurai_map)
	MDRV_CPU_IO_MAP(samurai_io_map)

	/* video hardware */
	MDRV_VIDEO_UPDATE(vicdual_color)

MACHINE_DRIVER_END



/*************************************
 *
 *  N-Sub
 *
 *  the colors are wrong because it has
 *  a different resistor network than the
 *  other games.
 *
 *************************************/

static READ8_HANDLER( nsub_io_r )
{
	UINT8 ret = 0;

	if (offset & 0x01)  ret = input_port_read(space->machine, "IN0");
	if (offset & 0x08)  ret = input_port_read(space->machine, "IN1");

	return ret;
}


static WRITE8_HANDLER( nsub_io_w )
{
	if (offset & 0x01)  assert_coin_status();
	if (offset & 0x02) { /* nsub_audio_w(0, data) */ }
	if (offset & 0x04)  vicdual_palette_bank_w(space, 0, data);
}


static ADDRESS_MAP_START( nsub_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_MIRROR(0x4000) AM_ROM
    AM_RANGE(0x8000, 0xbfff) AM_NOP	/* unused */
	AM_RANGE(0xc000, 0xc3ff) AM_MIRROR(0x3000) AM_RAM_WRITE(vicdual_videoram_w) AM_BASE(&vicdual_videoram)
	AM_RANGE(0xc400, 0xc7ff) AM_MIRROR(0x3000) AM_RAM
	AM_RANGE(0xc800, 0xcfff) AM_MIRROR(0x3000) AM_RAM_WRITE(vicdual_characterram_w) AM_BASE(&vicdual_characterram)
ADDRESS_MAP_END


static ADDRESS_MAP_START( nsub_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xf)

	/* no decoder, just logic gates, so in theory the
       game can read/write from multiple locations at once */
	AM_RANGE(0x00, 0x1f) AM_READWRITE(nsub_io_r, nsub_io_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( nsub )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_get_composite_blank_comp, NULL)
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME(0x08,  0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME(0x10,  0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME(0x20,  0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME(0x40,  0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_read_coin_status, NULL)

	PORT_COIN
INPUT_PORTS_END


static MACHINE_DRIVER_START( nsub )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(vicdual_root)
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(nsub_map)
	MDRV_CPU_IO_MAP(nsub_io_map)

	/* video hardware */
	MDRV_VIDEO_UPDATE(vicdual_color)

MACHINE_DRIVER_END



/*************************************
 *
 *  Invinco
 *
 *************************************/

static READ8_HANDLER( invinco_io_r )
{
	UINT8 ret = 0;

	if (offset & 0x01)  ret = input_port_read(space->machine, "IN0");
	if (offset & 0x02)  ret = input_port_read(space->machine, "IN1");
	if (offset & 0x08)  ret = input_port_read(space->machine, "IN2");

	return ret;
}


static WRITE8_HANDLER( invinco_io_w )
{
	if (offset & 0x01)  assert_coin_status();
	if (offset & 0x02)  invinco_audio_w(space, 0, data);
	if (offset & 0x04)  vicdual_palette_bank_w(space, 0, data);
}


static ADDRESS_MAP_START( invinco_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_MIRROR(0x4000) AM_ROM
    AM_RANGE(0x8000, 0xbfff) AM_NOP	/* unused */
	AM_RANGE(0xc000, 0xc3ff) AM_MIRROR(0x3000) AM_RAM_WRITE(vicdual_videoram_w) AM_BASE(&vicdual_videoram)
	AM_RANGE(0xc400, 0xc7ff) AM_MIRROR(0x3000) AM_RAM
	AM_RANGE(0xc800, 0xcfff) AM_MIRROR(0x3000) AM_RAM_WRITE(vicdual_characterram_w) AM_BASE(&vicdual_characterram)
ADDRESS_MAP_END


static ADDRESS_MAP_START( invinco_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xf)

	/* no decoder, just logic gates, so in theory the
       game can read/write from multiple locations at once */
	AM_RANGE(0x00, 0x1f) AM_READWRITE(invinco_io_r, invinco_io_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( invinco )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* probably unused */

	PORT_START("IN1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x60, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_get_composite_blank_comp, NULL)
	PORT_BIT( 0x7e, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(vicdual_read_coin_status, NULL)

	PORT_COIN
INPUT_PORTS_END


static MACHINE_DRIVER_START( invinco )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(vicdual_root)
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(invinco_map)
	MDRV_CPU_IO_MAP(invinco_io_map)

	/* video hardware */
	MDRV_VIDEO_UPDATE(vicdual_color)

	/* audio hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_IMPORT_FROM(invinco_audio)

MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( depthch )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "50a", 0x0000, 0x0400, CRC(56c5ffed) SHA1(f1e6cc322da93615d59850b3225a50f06fe58259) )
	ROM_LOAD( "51a", 0x0400, 0x0400, CRC(695eb81f) SHA1(f2491b8b9ce2dbb6d2606dcfaeb8658671f25400) )
	ROM_LOAD( "52",  0x0800, 0x0400, CRC(aed0ba1b) SHA1(cb7473e6b3c192953ae1832ab444545ddd85babb) )
	ROM_LOAD( "53",  0x0c00, 0x0400, CRC(2ccbd2d0) SHA1(76d8459bbad709666ce0c0be51f1d09e091983a2) )
	ROM_LOAD( "54a", 0x1000, 0x0400, CRC(1b7f6a43) SHA1(08d7864378b012a735eac4968f4dd86e36dc9d8d) )
	ROM_LOAD( "55a", 0x1400, 0x0400, CRC(9fc2eb41) SHA1(95a1684da346709908cd66bec06acfaeead596cf) )

	ROM_REGION( 0x0040, "user1", 0 )	/* timing PROMs */
	ROM_LOAD( "316-0043.u87", 0x0000, 0x0020, CRC(e60a7960) SHA1(b8b8716e859c57c35310efc4594262afedb84823) )	/* control PROM */
	ROM_LOAD( "316-0042.u88", 0x0020, 0x0020, CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) )	/* sequence PROM */
ROM_END

ROM_START( depthcho )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD_NIB_LOW ( "316-0025.u63", 0x0000, 0x0400, CRC(bec75b9c) SHA1(8abe8b63be892e6abb7a886222b9eab40c5fcda0) )
	ROM_LOAD_NIB_HIGH( "316-0022.u51", 0x0000, 0x0400, CRC(977b7889) SHA1(dc1e874c2fd44709117474c5b210d67130ac361f) )
	ROM_LOAD_NIB_LOW ( "316-0030.u89", 0x0400, 0x0400, CRC(9e2bbb45) SHA1(be60d7330a160e15a8a822aa791aed3060b3b1db) )
	ROM_LOAD_NIB_HIGH( "316-0028.u77", 0x0400, 0x0400, CRC(597ae441) SHA1(8d3af5e64e838a57057d46f97a7b1c1037c1a0cf) )
	ROM_LOAD_NIB_LOW ( "316-0026.u64", 0x0800, 0x0400, CRC(61cc0802) SHA1(7173920f38188a1d1637cb2cb48e31cdd03a194e) )
	ROM_LOAD_NIB_HIGH( "316-0023.u52", 0x0800, 0x0400, CRC(9244b613) SHA1(6587035ec22d90194cdc3efaed3571a1ab975e1c) )
	ROM_LOAD_NIB_LOW ( "316-0031.u90", 0x0c00, 0x0400, CRC(861ffed1) SHA1(14e0a6a13726052000477c3586d99486167b8812) )
	ROM_LOAD_NIB_HIGH( "316-0029.u78", 0x0c00, 0x0400, CRC(53178634) SHA1(d8c4b70c3ab5144938ca0989300ad68e48391490) )
	ROM_LOAD_NIB_LOW ( "316-0027.u65", 0x1000, 0x0400, CRC(4eecfc70) SHA1(1a1f6cc5da6df91e9e9016def65184201c3d2672) )
	ROM_LOAD_NIB_HIGH( "316-0024.u53", 0x1000, 0x0400, CRC(a9f55883) SHA1(78bfbc76f84657d32eb1b8072186b403729ea614) )
	ROM_LOAD_NIB_LOW ( "316-0049.u91", 0x1400, 0x0400, CRC(dc7eff35) SHA1(1915e92c09cba5868bd2e73ad395e19ddf47a3de) )
	ROM_LOAD_NIB_HIGH( "316-0048.u79", 0x1400, 0x0400, CRC(6e700621) SHA1(2b8db1cbbaf7808d4bf446435bbbbfc4d7761db8) )

	ROM_REGION( 0x0040, "user1", 0 )	/* timing PROMs */
	ROM_LOAD( "316-0013.u27", 0x0000, 0x0020, CRC(690ef530) SHA1(6c0de3fa87a341cd378fefb8e06bf7918db9a074) )	/* control PROM */
	ROM_LOAD( "316-0014.u28", 0x0020, 0x0020, CRC(7b7a8492) SHA1(6ba8d891cc6eb0dd80051377b6b832e8894655e7) )	/* sequence PROM */
ROM_END

ROM_START( subhunt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD_NIB_LOW ( "dp04",         0x0000, 0x0400, CRC(0ace1aef) SHA1(071256dd63e2e449093a65a4c9b006be5e17b786) )
	ROM_LOAD_NIB_HIGH( "dp01",         0x0000, 0x0400, CRC(da9e835b) SHA1(505c969b479aeab11bb6a21ef06837280846d90a) )
	ROM_LOAD_NIB_LOW ( "dp10",         0x0400, 0x0400, CRC(de752f20) SHA1(513a92554d14a09d6b80ba8017d161c7cda9ed8c) )
	ROM_LOAD_NIB_HIGH( "316-0028.u77", 0x0400, 0x0400, CRC(597ae441) SHA1(8d3af5e64e838a57057d46f97a7b1c1037c1a0cf) ) // dp07
	ROM_LOAD_NIB_LOW ( "dp05",         0x0800, 0x0400, CRC(1c0530cf) SHA1(b1f2b1038ee063533669341f1a71755eecc2e1a9) )
	ROM_LOAD_NIB_HIGH( "316-0023.u52", 0x0800, 0x0400, CRC(9244b613) SHA1(6587035ec22d90194cdc3efaed3571a1ab975e1c) ) // dp02
	ROM_LOAD_NIB_LOW ( "dp11",         0x0c00, 0x0400, CRC(0007044a) SHA1(c8d7c693e3059ff020563336fe712c234e94b8f9) )
	ROM_LOAD_NIB_HIGH( "dp08",         0x0c00, 0x0400, CRC(4d4e3ec8) SHA1(a0d5392fe5795cc6bf7373f194186506283c947c) )
	ROM_LOAD_NIB_LOW ( "dp06",         0x1000, 0x0400, CRC(63e1184b) SHA1(91934cb041365dabdc58a831312577fdb0dc923b) )
	ROM_LOAD_NIB_HIGH( "dp03",         0x1000, 0x0400, CRC(d70dbfd8) SHA1(0183a6b1ffd87a9e28588a7a9aa18aeb003560f0) )
	ROM_LOAD_NIB_LOW ( "dp12",         0x1400, 0x0400, CRC(170d7718) SHA1(4348e4e2dbb1edd9a4228fd3ccef58c50f1ae129) )
	ROM_LOAD_NIB_HIGH( "dp09",         0x1400, 0x0400, CRC(97466803) SHA1(f04ba4a1a960836974a85832596fc3a92a711094) )

	ROM_REGION( 0x0040, "user1", 0 )
	ROM_LOAD( "316-0013.u27", 0x0000, 0x0020, CRC(690ef530) SHA1(6c0de3fa87a341cd378fefb8e06bf7918db9a074) )	/* control PROM */
	ROM_LOAD( "316-0014.u28", 0x0020, 0x0020, CRC(7b7a8492) SHA1(6ba8d891cc6eb0dd80051377b6b832e8894655e7) )	/* sequence PROM */
ROM_END


ROM_START( safari )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "316-0066.u48", 0x0000, 0x0400, CRC(2a26b098) SHA1(a16b04110fb142cec01c10460b14ec0c4e8d99af) )
	ROM_LOAD( "316-0065.u47", 0x0400, 0x0400, CRC(b776f7db) SHA1(7332d1b18e1b199d87367182f185abafd9ad0bb1) )
	ROM_LOAD( "316-0064.u46", 0x0800, 0x0400, CRC(19d8c196) SHA1(219dca308a4f917617cfe291580eb23fc2cb4687) )
	ROM_LOAD( "316-0063.u45", 0x0c00, 0x0400, CRC(028bad25) SHA1(94120f197c15705d9447d4615b82e31b61672f89) )
	ROM_LOAD( "316-0062.u44", 0x1000, 0x0400, CRC(504e0575) SHA1(069390941a0d79d623dce816fefef4d52b6e929f) )
	ROM_LOAD( "316-0061.u43", 0x1400, 0x0400, CRC(d4c528e0) SHA1(8b28b70f4cdb12189bb7526d70e4df849a4b9c42) )
	ROM_LOAD( "316-0060.u42", 0x1800, 0x0400, CRC(48c7b0cc) SHA1(26f757927212a01b2682ab520dd3b26a5524bdc3) )
	ROM_LOAD( "316-0059.u41", 0x1c00, 0x0400, CRC(3f7baaff) SHA1(5f935cb2212718226cca10f4bcb28a5fdde109c7) )
	ROM_LOAD( "316-0058.u40", 0x2000, 0x0400, CRC(0d5058f1) SHA1(00fd39a058e206b1bc5669438ab9670fa4db1921) )
	ROM_LOAD( "316-0057.u39", 0x2400, 0x0400, CRC(298e8c41) SHA1(b9b6bc84d2531c85e4529c910d6e97ea83650ce3) )

	ROM_REGION( 0x0040, "user1", 0 )	/* timing PROMs */
	ROM_LOAD( "316-0043.u87", 0x0000, 0x0020, CRC(e60a7960) SHA1(b8b8716e859c57c35310efc4594262afedb84823) )	/* control PROM */
	ROM_LOAD( "316-0042.u88", 0x0020, 0x0020, CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) )	/* sequence PROM */
ROM_END

ROM_START( safaria ) // bootleg board, but possibly a legit alt revision
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hu1.22c",      0x0000, 0x0400, CRC(f27d5961) SHA1(9780e9659746c959206b8700598cfb3925ae7938) )
	ROM_LOAD( "hu2.20c",      0x0400, 0x0400, CRC(11a9cb59) SHA1(5132151a97146d973292b15284e4d58de9ee7cc6) )
	ROM_LOAD( "hu3.19c",      0x0800, 0x0400, CRC(4fe746cb) SHA1(b4b6eac78dd9a76c102994990c219ae832442cc1) )
	ROM_LOAD( "hu4.17c",      0x0c00, 0x0400, CRC(f0bad948) SHA1(5dda4513e96cfb0b6535184c611ac5832afbbfde) )
	ROM_LOAD( "hu5.16c",      0x1000, 0x0400, CRC(d994f98a) SHA1(a51db8b8975f1fa7ca3bae56d4d929b8d3d7bfb7) )
	ROM_LOAD( "hu6.15c",      0x1400, 0x0400, CRC(174b5964) SHA1(dfa88aaa572d4d46ffe3c8f247dbc370e624f5c4) )
	ROM_LOAD( "hu7.13c",      0x1800, 0x0400, CRC(3e94caa1) SHA1(520ae5924b970126c07368bba900b7603997c5cc) )
	ROM_LOAD( "hu8.12c",      0x1c00, 0x0400, CRC(a8a5dca0) SHA1(2424d9e3b4ed2c73b14ec0c26d63453fb4f7f6c2) )
	ROM_LOAD( "hu9.11c",      0x2000, 0x0400, CRC(0ace0939) SHA1(34704b836445628341fb6a77b1ebd47a76c5640d) )
	ROM_LOAD( "hu10.9c",      0x2400, 0x0400, CRC(9dae33ca) SHA1(91472e3b60ff055724ae574b182a450d2a00081c) )

	ROM_REGION( 0x0040, "user1", 0 )	/* timing PROMs */
	ROM_LOAD( "32.21e", 0x0000, 0x0020, CRC(e60a7960) SHA1(b8b8716e859c57c35310efc4594262afedb84823) )	/* control PROM */
	ROM_LOAD( "31.22e", 0x0020, 0x0020, CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) )	/* sequence PROM */
ROM_END


ROM_START( frogs )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "316-119a.u48",     0x0000, 0x0400, CRC(b1d1fce4) SHA1(572015bede39b14526e93919b63b6d01ae38a09a) )
	ROM_LOAD( "316-118a.u47",     0x0400, 0x0400, CRC(12fdcc05) SHA1(06c6d17edec9fb03f46514c1f6c5d8c420ef4d05) )
	ROM_LOAD( "316-117a.u46",     0x0800, 0x0400, CRC(8a5be424) SHA1(ed8a09b4318929b83118f87e2da601028349f2bd) )
	ROM_LOAD( "316-116b.u45",     0x0c00, 0x0400, CRC(09b82619) SHA1(1063e268138b4ff6a8037d8d1a0816c34bbac690) )
	ROM_LOAD( "316-115a.u44",     0x1000, 0x0400, CRC(3d4e4fa8) SHA1(4655c4922328837af410cb298e0c296ae0099591) )
	ROM_LOAD( "316-114a.u43",     0x1400, 0x0400, CRC(04a21853) SHA1(1e84eb84d5770f54925055b748ab9ca2aa72c1cc) )
	ROM_LOAD( "316-113a.u42",     0x1800, 0x0400, CRC(02786692) SHA1(8a8937fd92beecf1119fe3f6b41a700725412aa1) )
	ROM_LOAD( "316-112a.u41",     0x1c00, 0x0400, CRC(0be2a058) SHA1(271f3b60cba422fff7e782fda198c3897c275b46) )

	ROM_REGION( 0x0040, "user1", 0 )	/* timing PROMs */
	ROM_LOAD( "316-0043.u87", 0x0000, 0x0020, CRC(e60a7960) SHA1(b8b8716e859c57c35310efc4594262afedb84823) )	/* control PROM */
	ROM_LOAD( "316-0042.u88", 0x0020, 0x0020, CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) )	/* sequence PROM */
ROM_END

/*

N-Sub by SEGA 1979

97399-P-16 N-SUB UPRIGHT

Label says : U41(U20) ~ U48(U27)
        EPR   EPR
        268   275

and also : PR69

Despite what the label says, here is correct name and position from a real pcb !

Epr-268.u48
Epr-269.u47
Epr-270.u46
Epr-271.u45
Epr-272.u44
Epr-273.u43
Epr-274.u42
Epr-275.u41
Pr-69.u11

This game use a separate "daughter" board for input ??? ref: 97269-P-B
with a prom on it : PR-02 type MMI 6336-1j which is soldered.

*/

ROM_START( nsub )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-268.u48", 0x0000, 0x0800, CRC(485b4704) SHA1(d3989cfe5f8d723bc1a6be185614d138666912d2) )
	ROM_LOAD( "epr-269.u47", 0x0800, 0x0800, CRC(32774ac9) SHA1(0a2c209f627a8d703c02e75c361c363272d1f435) )
	ROM_LOAD( "epr-270.u46", 0x1000, 0x0800, CRC(af7ca40a) SHA1(c0f5732079a51979758f3a159084b84be8b2ad3b) )
	ROM_LOAD( "epr-271.u45", 0x1800, 0x0800, CRC(3f9c180b) SHA1(7438454a348b36d1f5ea59f179f715b827244142) )
	ROM_LOAD( "epr-272.u44", 0x2000, 0x0800, CRC(d818aa51) SHA1(1d3ca550f597c4924b9a805fa955a4a8ff557769) )
	ROM_LOAD( "epr-273.u43", 0x2800, 0x0800, CRC(03a6f12a) SHA1(1eefd4607a718c291b29f1b0a6adf0367840b242) )
	ROM_LOAD( "epr-274.u42", 0x3000, 0x0800, CRC(d69eb098) SHA1(fd3e67d18b5891aa65aab5967d49810c5d88dcee) )
	ROM_LOAD( "epr-275.u41", 0x3800, 0x0800, CRC(1c7d90cc) SHA1(8483825d9811c925407328836ae10f98b011c3dd) )

	ROM_REGION( 0x0020, "proms", ROMREGION_INVERT )
	ROM_LOAD( "pr-69.u11", 0x0000, 0x0020, CRC(c94dd091) SHA1(f88cfb033ff83adb7375652be1fa32ba489d8418) )

	ROM_REGION( 0x0040, "user1", 0 )	/* timing PROMs */
	ROM_LOAD( "pr33.u82", 0x0000, 0x0020, CRC(e60a7960) SHA1(b8b8716e859c57c35310efc4594262afedb84823) )	/* control PROM */
	ROM_LOAD( "pr34.u83", 0x0020, 0x0020, CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) )	/* sequence PROM */
ROM_END

ROM_START( sspaceat )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "155.u27",      0x0000, 0x0400, CRC(ba7bb86f) SHA1(030e6f69d3ae00456fc02d1dc0fb915a81689df4) )
	ROM_LOAD( "156.u26",      0x0400, 0x0400, CRC(0b3a491c) SHA1(19cef304bb91f745797f27adbb9d334876d4fb78) )
	ROM_LOAD( "157.u25",      0x0800, 0x0400, CRC(3d3fac3b) SHA1(b22c2517af7c7077032d1b83e4628173d168e3ca) )
	ROM_LOAD( "158.u24",      0x0c00, 0x0400, CRC(843b80f6) SHA1(b61466d3546f1e0759ec84e841664cbe4d2a0a4d) )
	ROM_LOAD( "159.u23",      0x1000, 0x0400, CRC(1eacf60d) SHA1(52d5bfad4357619a9bdc7435e66ed5accadc6401) )
	ROM_LOAD( "160.u22",      0x1400, 0x0400, CRC(e61d482f) SHA1(d41550af1cb244adddff5c151fe2c140591c58ff) )
	ROM_LOAD( "161.u21",      0x1800, 0x0400, CRC(eb5e0993) SHA1(745135f50e50a8516a529a3caff27ee2580227f1) )
	ROM_LOAD( "162.u20",      0x1c00, 0x0400, CRC(5f84d550) SHA1(4fa2c48f843ad49b55598b2757e0e4e1e117aacb) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "316-0138.u44", 0x0000, 0x0020, CRC(67104ea9) SHA1(26b6bd2a1973b83bb9af4e3385d8cb14cb3f62f2) )

	ROM_REGION( 0x0040, "user1", 0 )	/* timing PROMs */
	ROM_LOAD( "316-0043.u65", 0x0000, 0x0020, CRC(e60a7960) SHA1(b8b8716e859c57c35310efc4594262afedb84823) )	/* control PROM */
	ROM_LOAD( "316-0042.u66", 0x0020, 0x0020, CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) )	/* sequence PROM */
ROM_END

ROM_START( sspaceat2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "81.u48",       0x0000, 0x0400, CRC(3e4b29f6) SHA1(ec99b7e156bad1f9f900fdebb289f0c9abf08647) )
	ROM_LOAD( "58.u47",       0x0400, 0x0400, CRC(176adb80) SHA1(9798d3b2d59fe4b7d26927b444746f135f0f0d8e) )
	ROM_LOAD( "59.u46",       0x0800, 0x0400, CRC(b2400d05) SHA1(12011fd91bbdfc94b02f9089be54d7cbb8dedece) )
	ROM_LOAD( "150.u45",      0x0c00, 0x0400, CRC(cf9bfa65) SHA1(3521bd2608705a83bd8d3daa0239708d2a8755e3) )
	ROM_LOAD( "151.u44",      0x1000, 0x0400, CRC(064530f1) SHA1(8278b271ae7d67e0b5433aefb150fd743ce6558a) )
	ROM_LOAD( "152.u43",      0x1400, 0x0400, CRC(c65c30fe) SHA1(849a0d46575ad0c72aceef28daa27911ec35181a) )
	ROM_LOAD( "153.u42",      0x1800, 0x0400, CRC(ea70c7f6) SHA1(656d113636224ec9c30982daa6a43877bc6ee58f) )
	ROM_LOAD( "156a.u41",     0x1c00, 0x0400, CRC(9029d2ce) SHA1(57d21650bfe9d76d874661443768213321acc56b) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "316-0138.u44", 0x0000, 0x0020, CRC(67104ea9) SHA1(26b6bd2a1973b83bb9af4e3385d8cb14cb3f62f2) )

	ROM_REGION( 0x0040, "user1", 0 )	/* timing PROMs */
	ROM_LOAD( "316-0043.u65", 0x0000, 0x0020, CRC(e60a7960) SHA1(b8b8716e859c57c35310efc4594262afedb84823) )	/* control PROM */
	ROM_LOAD( "316-0042.u66", 0x0020, 0x0020, CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) )	/* sequence PROM */
ROM_END

ROM_START( sspaceat3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-115.u48",   0x0000, 0x0400, CRC(9bc36d80) SHA1(519b3f810b133ac82f066851626b73460956a807) )
	ROM_LOAD( "epr-116.u47",   0x0400, 0x0400, CRC(2c2750b3) SHA1(eab297678e6ee45d6f723d8ff7e6a29086ad4c78) )
	ROM_LOAD( "epr-117.u46",   0x0800, 0x0400, CRC(fa7c2cc0) SHA1(26e4f2c8599d16f1c7ec4bfb0a5a3dc709901045) )
	ROM_LOAD( "epr-118.u45",   0x0c00, 0x0400, CRC(273884ae) SHA1(9efae4acb9ba9bdef0fb58c2a16e0092c6c1a2ba) )
	ROM_LOAD( "epr-119.u44",   0x1000, 0x0400, CRC(1b53c6de) SHA1(7d8f3f5026e7d1a3b78a54c9c1acbb50a4f02c94) )
	ROM_LOAD( "epr-120.u43",   0x1400, 0x0400, CRC(60add585) SHA1(01d78d5cbad680b8ad7eb39f53eefad148d48ee2) )
	ROM_LOAD( "epr-121.u42",   0x1800, 0x0400, CRC(0979f72b) SHA1(244e80552b905df5484bb52100b2e46859fd2cf6) )
	ROM_LOAD( "epr-122.u41",   0x1c00, 0x0400, CRC(45cb3486) SHA1(0e9d5e8dd43643588989354847483283487b9a75) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "316-0138.u44", 0x0000, 0x0020, CRC(67104ea9) SHA1(26b6bd2a1973b83bb9af4e3385d8cb14cb3f62f2) )

	ROM_REGION( 0x0040, "user1", 0 )	/* timing PROMs */
	ROM_LOAD( "316-0043.u65", 0x0000, 0x0020, CRC(e60a7960) SHA1(b8b8716e859c57c35310efc4594262afedb84823) )	/* control PROM */
	ROM_LOAD( "316-0042.u66", 0x0020, 0x0020, CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) )	/* sequence PROM */
ROM_END

ROM_START( sspaceatc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "139.u27",      0x0000, 0x0400, CRC(9f2112fc) SHA1(89c129ef1a95c5934a7c775994aafc91911b0051) )
	ROM_LOAD( "140.u26",      0x0400, 0x0400, CRC(ddbeed35) SHA1(48b33d7b35457675b545ca42c8afd79b86ce6035) )
	ROM_LOAD( "141.u25",      0x0800, 0x0400, CRC(b159924d) SHA1(320bbe156493f30a573ff548398f8f469e261e21) )
	ROM_LOAD( "142.u24",      0x0c00, 0x0400, CRC(f2ebfce9) SHA1(4792bca4a331bc41fd850760e6260e933063398f) )
	ROM_LOAD( "143.u23",      0x1000, 0x0400, CRC(bff34a66) SHA1(8a7490a13b9526c45f3afee1eee59d2b0096105f) )
	ROM_LOAD( "144.u22",      0x1400, 0x0400, CRC(fa062d58) SHA1(76e15e1d29b0e22ab310381d3d9faddf8912b205) )
	ROM_LOAD( "145.u21",      0x1800, 0x0400, CRC(7e950614) SHA1(0fe4de728ca550f0ef904b2d7d84fb2d56648401) )
	ROM_LOAD( "146.u20",      0x1c00, 0x0400, CRC(8ba94fbc) SHA1(371702c2d489fa0f1959734ebd35af45006712fa) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "316-0138.u44", 0x0000, 0x0020, CRC(67104ea9) SHA1(26b6bd2a1973b83bb9af4e3385d8cb14cb3f62f2) )

	ROM_REGION( 0x0040, "user1", 0 )	/* timing PROMs */
	ROM_LOAD( "316-0043.u65", 0x0000, 0x0020, CRC(e60a7960) SHA1(b8b8716e859c57c35310efc4594262afedb84823) )	/* control PROM */
	ROM_LOAD( "316-0042.u66", 0x0020, 0x0020, CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) )	/* sequence PROM */
ROM_END


/*
Head On
Irem, 1979? / 1980?

PCB Layout
----------

    M-15L
   |---------------------------------------------------------------------------------|
   |                                                                                 |
   | DSW(8)  74175   74175   7400  74LS08   74121   M53214     |-------|      E1.9A  |
   |                                                           | 6502  |             |
   |          7432   74175   7404    7427    7442  74LS241     |-------|             |
   |                                                                          E2.9B  |
   |                                                                                 |
   |        74LS74    7432  74161   74161    7442  74LS241  74LS367  74LS367         |
 |-|                                                                          E3.9C  |
 |          M53214 74LS367   7442    7486    8216     2114  74LS157  74LS367         |
 |                                                                                   |
 |4         M53214 74LS367  74161    7486    8216     2114  74LS157     2111  E4.9D  |
 |4                                                                                  |
 |W         M53214 74LS367  74161    7486    8216     2114  74LS157     2111         |
 |A                                                                           E5.9F  |
 |Y         M53214 74LS367  74161    7486    8216    74166     2114  74LS157         |
 |                         11.73MHz                                                  |
 |            7400    7432  7404    74161    8216    74166     2114  74LS157  E6.9G  |
 |-|                                                                                 |
   |   VR3 VR2 VR1    7432  7404     7400  *74173     7400  74LS139  74LS157         |
   |                                       *74S04                                    |
   |                                                                                 |
   |---------------------------------------------------------------------------------|
Notes:
      All IC's are listed
      All ROMs type 2708 (1K x8)

      6502 clock: 733.125kHz (11.73 /16)
               *: These 2 IC's piggybacked. 74S04 on top
         VR1/2/3: 5K potentiometers, controls RGB saturation levels
            2114: 1K x4 SRAM
            2111: 256bytes x4 SRAM
            8216: 256bytes x1 SRAM

Sound PCB
---------

M-15S
|---------------------------|
|                           |
|  NE555  NE555             |
|                           |
|  NE555  NE555             |
|               LM3900   VR1|
|                           |
|  C1815x9               VR2|
|                           |
|               LM3900   VR3|
|                           |
|                        VR4|
|                           |
|                        VR5|
|    TA7222                 |
|---------------------------|
Notes:
      PCB contains lots of resistors, capacitors, transistors etc.

      VR1/2/3/4/5: Potentiometers for volume of each sound
                   VR1 - Car rumble
                   VR2 - Collision bang
                   VR3 - Tire screech (when changing lanes)
                   VR4 - Score counter (numbers incrementing etc)
                   VR5 - Master volume
            C1815: Transistor (x9)
           TA7222: Power Amp
*/

ROM_START( headon )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "316-163a.u27", 0x0000, 0x0400, CRC(4bb51259) SHA1(43411ffda3fe03b1d694f70791b0bab5786759c0) )
	ROM_LOAD( "316-164a.u26", 0x0400, 0x0400, CRC(aeac8c5f) SHA1(ef9ad63d13076a559ba12c6421ad61de21dd4c90) )
	ROM_LOAD( "316-165a.u25", 0x0800, 0x0400, CRC(f1a0cb72) SHA1(540b30225ef176c416ea5b142fe7dbb67b7a78fb) )
	ROM_LOAD( "316-166c.u24", 0x0c00, 0x0400, CRC(65d12951) SHA1(25fb0da2ea62a2b1ec214ce5c599a183e121b98a) )
	ROM_LOAD( "316-167c.u23", 0x1000, 0x0400, CRC(2280831e) SHA1(128e7f7444440f113b3395dcb333281e0e8bef93) )
	ROM_LOAD( "316-192a.u22", 0x1400, 0x0400, CRC(ed4666f2) SHA1(a12c22bfbb027eab3181627804b69129e89bd22c) )
	ROM_LOAD( "316-193a.u21", 0x1800, 0x0400, CRC(37a1df4c) SHA1(45e1670351f0ef92ef4d9100b0e60ae598df4275) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "316-0138.u44", 0x0000, 0x0020, CRC(67104ea9) SHA1(26b6bd2a1973b83bb9af4e3385d8cb14cb3f62f2) )

	ROM_REGION( 0x0040, "user1", 0 )	/* timing PROMs */
	ROM_LOAD( "316-0043.u87", 0x0000, 0x0020, CRC(e60a7960) SHA1(b8b8716e859c57c35310efc4594262afedb84823) )	/* control PROM */
	ROM_LOAD( "316-0042.u88", 0x0020, 0x0020, CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) )	/* sequence PROM */
ROM_END

ROM_START( headonb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "316-163a.u27", 0x0000, 0x0400, CRC(4bb51259) SHA1(43411ffda3fe03b1d694f70791b0bab5786759c0) )
	ROM_LOAD( "316-164a.u26", 0x0400, 0x0400, CRC(aeac8c5f) SHA1(ef9ad63d13076a559ba12c6421ad61de21dd4c90) )
	ROM_LOAD( "316-165a.u25", 0x0800, 0x0400, CRC(f1a0cb72) SHA1(540b30225ef176c416ea5b142fe7dbb67b7a78fb) )
	ROM_LOAD( "316-166b.u24", 0x0c00, 0x0400, CRC(1c59008a) SHA1(430ecc3c2422d61af35eab77b96a480254572cc6) )
	ROM_LOAD( "316-167a.u23", 0x1000, 0x0400, CRC(069e839e) SHA1(e1ed68573c13c0c88a2bb7b2096860523de952c0) )
	ROM_LOAD( "316-192a.u22", 0x1400, 0x0400, CRC(ed4666f2) SHA1(a12c22bfbb027eab3181627804b69129e89bd22c) )
	ROM_LOAD( "316193a1.u21", 0x1800, 0x0400, CRC(d3782c1d) SHA1(340782374b7015a0aaf98aeb6503b759e199a58a) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "316-0138.u44", 0x0000, 0x0020, CRC(67104ea9) SHA1(26b6bd2a1973b83bb9af4e3385d8cb14cb3f62f2) )

	ROM_REGION( 0x0040, "user1", 0 )	/* timing PROMs */
	ROM_LOAD( "316-0043.u87", 0x0000, 0x0020, CRC(e60a7960) SHA1(b8b8716e859c57c35310efc4594262afedb84823) )	/* control PROM */
	ROM_LOAD( "316-0042.u88", 0x0020, 0x0020, CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) )	/* sequence PROM */
ROM_END


/*
Head On (Sidam) Notes

Board made by Sidam, but no Sidam copyright notice

---CPU:

1x Z80CPU (main)
1x oscillator 15.468MHz

---ROMs:

7x F2708
2x N82S123N

---Note:

1x 22x2 edge connector
1x trimmer (volume)
*/

ROM_START( headons )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "0.1a",         0x0000, 0x0400, CRC(4bb51259) SHA1(43411ffda3fe03b1d694f70791b0bab5786759c0) )
	ROM_LOAD( "1.3a",         0x0400, 0x0400, CRC(aeac8c5f) SHA1(ef9ad63d13076a559ba12c6421ad61de21dd4c90) )
	ROM_LOAD( "2.4a",         0x0800, 0x0400, CRC(f1a0cb72) SHA1(540b30225ef176c416ea5b142fe7dbb67b7a78fb) )
	ROM_LOAD( "3.6a",         0x0c00, 0x0400, CRC(461c2658) SHA1(561ef24a20fb2cc3c05d836c06026069400be085) )
	ROM_LOAD( "4.8a",         0x1000, 0x0400, CRC(79fc7f31) SHA1(835fbaa2bac8b955bc8fe5e932705c67e10308ac) )
	ROM_LOAD( "5.9a",         0x1400, 0x0400, CRC(ed4666f2) SHA1(a12c22bfbb027eab3181627804b69129e89bd22c) )
	ROM_LOAD( "6.11a",        0x1800, 0x0400, CRC(7a709d68) SHA1(c1f0178c7a8cb39948e52e91a841401cfd932271) )

	ROM_REGION( 0x0040, "user1", 0 )	/* timing PROMs */
	ROM_LOAD( "316-0043.u87", 0x0000, 0x0020, CRC(e60a7960) SHA1(b8b8716e859c57c35310efc4594262afedb84823) )	/* control PROM */
	ROM_LOAD( "316-0042.u88", 0x0020, 0x0020, CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) )	/* sequence PROM */
ROM_END

/* this one is the same PCB but does show the Sidam copyright */
ROM_START( headonsa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "10305.0.9a",         0x0000, 0x0400, CRC(9a37407b) SHA1(3cd3dbd13c76d01b7541307de92f69d6779046f5) )
	ROM_LOAD( "10305.1.8a",         0x0400, 0x0400, CRC(aeac8c5f) SHA1(ef9ad63d13076a559ba12c6421ad61de21dd4c90) )
	ROM_LOAD( "10305.2.7a",         0x0800, 0x0400, CRC(f1a0cb72) SHA1(540b30225ef176c416ea5b142fe7dbb67b7a78fb) )
	ROM_LOAD( "10305.3.6a",         0x0c00, 0x0400, CRC(ae33fcc4) SHA1(7e0a27f1f502c5293f294875b49186e800a2c749) )
	ROM_LOAD( "10305.4.5a",         0x1000, 0x0400, CRC(e87f6fd8) SHA1(7fc1ade66c6783861ab310790f023b02a8db7e08) )
	ROM_LOAD( "10305.5.4a",         0x1400, 0x0400, CRC(387e2eba) SHA1(9feca874e795710884d17ca5122280c30c6b6af0) )
	ROM_LOAD( "10305.6b.3a",        0x1800, 0x0400, CRC(18749071) SHA1(6badb5cf6f6017d884492e9ef16195f1112d23b5) )

	ROM_REGION( 0x0040, "user1", 0 )	/* timing PROMs */
	ROM_LOAD( "10303.3e", 0x0000, 0x0020, CRC(e60a7960) SHA1(b8b8716e859c57c35310efc4594262afedb84823) )	/* control PROM */
	ROM_LOAD( "10302.2e", 0x0020, 0x0020, CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) )	/* sequence PROM */
ROM_END


ROM_START( headon2s )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "10304.0.9a",      0x0000, 0x0400, CRC(256a1fc8) SHA1(2ff621d7160e1420fd2cd9ad62d134e22b1650b3) )
	ROM_LOAD( "10304.1.8a",      0x0400, 0x0400, CRC(61c47b15) SHA1(47619bd51fcaf47dd72e940c474f310c9287f2f4) )
	ROM_LOAD( "10304.2.7a",      0x0800, 0x0400, CRC(a6c268d4) SHA1(3b6c27f700ea4474f5354bbdcce82883c2e7e6e9) )
	ROM_LOAD( "10304.3.6a",      0x0c00, 0x0400, CRC(17a09f24) SHA1(0cb40ec185f2ee3a26e943d84e8e2834d5f9d3ed) )
	ROM_LOAD( "10304.4.5a",      0x1000, 0x0400, CRC(9af8a2e0) SHA1(92f45bc593fabf7a30615820b4b91677071bc67e) )
	ROM_LOAD( "10304.5.4a",      0x1400, 0x0400, CRC(6975286c) SHA1(bcb5af18a991b9898fe28e69575c89c7b02d762d) )
	ROM_LOAD( "10304.6.3a",      0x1800, 0x0400, CRC(06fbcdce) SHA1(821b501dbf59c45d5e03afa3c786fca727da9cd6) )
	ROM_LOAD( "10304.7b.2a",     0x1c00, 0x0400, CRC(3588fc8f) SHA1(4529b79a1b654591ee2e879922a5377edc1faee5) )

	ROM_REGION( 0x0040, "user1", 0 )	/* timing PROMs */
	ROM_LOAD( "10303.3e", 0x0000, 0x0020, CRC(e60a7960) SHA1(b8b8716e859c57c35310efc4594262afedb84823) )	/* control PROM */
	ROM_LOAD( "10302.2e", 0x0020, 0x0020, CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) )	/* sequence PROM */
ROM_END


/*
Super Crash notes

---CPU:

1x MK3880N-Z80CPU
1x oscillator 14MHz

---ROMs:

3x TMS2716JL
1x FA2708
2x MMI6331

---Note:

1x 22x2 edge connector
1x 4 switches dip
1x trimmer (volume)
*/

ROM_START( supcrash )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1-2-scrash.bin", 0x0000, 0x0800, CRC(789a8b73) SHA1(ce0b844729fc4d46ddc82635c8d5a49aa88a3797) )
	ROM_LOAD( "3-4-scrash.bin", 0x0800, 0x0800, CRC(7a310527) SHA1(384c7ddc8da4282b705ad387ae3946a30f0fd05b) )
	ROM_LOAD( "5-6-scrash.bin", 0x1000, 0x0800, CRC(62d33c09) SHA1(ade49f417380f64212491f6be16de39c0c00a364) )
	ROM_LOAD( "7-8-scrash.bin", 0x1800, 0x0400, CRC(0f8ea335) SHA1(cf2d6cd54dbf689bc0f23aa908bffb0766e8bbd3) )

	ROM_REGION( 0x0040, "user1", 0 )	/* timing PROMs */
	ROM_LOAD( "316-0043.u87",   0x0000, 0x0020, CRC(e60a7960) SHA1(b8b8716e859c57c35310efc4594262afedb84823) )	/* control PROM */
	ROM_LOAD( "316-0042.u88",   0x0020, 0x0020, CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) )	/* sequence PROM */
ROM_END


ROM_START( headon2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u27.bin",      0x0000, 0x0400, CRC(fa47d2fb) SHA1(b3208f5bce228c453bdafbc9c1f2c8e1bd920d32) )
	ROM_LOAD( "u26.bin",      0x0400, 0x0400, CRC(61c47b15) SHA1(47619bd51fcaf47dd72e940c474f310c9287f2f4) )
	ROM_LOAD( "u25.bin",      0x0800, 0x0400, CRC(bb16db92) SHA1(f57dfbe52b0e545c7c889ac846dc7281d28f2698) )
	ROM_LOAD( "u24.bin",      0x0c00, 0x0400, CRC(17a09f24) SHA1(0cb40ec185f2ee3a26e943d84e8e2834d5f9d3ed) )
	ROM_LOAD( "u23.bin",      0x1000, 0x0400, CRC(0024895e) SHA1(60f81c383f1541555c26f7cf111a12a34f7f4f3e) )
	ROM_LOAD( "u22.bin",      0x1400, 0x0400, CRC(f798304d) SHA1(55526c6daead9b74a88e8bc0311155aa41a93210) )
	ROM_LOAD( "u21.bin",      0x1800, 0x0400, CRC(4c19dd40) SHA1(0bdfed47594c7aa5ff655b507350fc6a912b6855) )
	ROM_LOAD( "u20.bin",      0x1c00, 0x0400, CRC(25887ff2) SHA1(67cfb5ac93902b4c603f02c876c021ff453e5f0e) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "316-0138.u44", 0x0000, 0x0020, CRC(67104ea9) SHA1(26b6bd2a1973b83bb9af4e3385d8cb14cb3f62f2) )

	ROM_REGION( 0x0020, "user1", 0 )	/* timing PROM */
	ROM_LOAD( "316-0206.u65", 0x0000, 0x0020, CRC(9617d796) SHA1(7cff2741866095ff42eadd8022bea349ec8d2f39) )	/* control PROM */
ROM_END


/*
    Car 2 (Headon 2)

    1x Z80
    1x TDA2002 (sound)
    2x NE556A (sound)
    1x oscillator 15468.48
*/

ROM_START( car2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "car2.0",      0x0000, 0x0400, CRC(37e031f9) SHA1(b8aee0db50507410e5be70d3a8574b7e9dd4a959) )
	ROM_LOAD( "car2.1",      0x0400, 0x0400, CRC(61c47b15) SHA1(47619bd51fcaf47dd72e940c474f310c9287f2f4) )
	ROM_LOAD( "car2.2",      0x0800, 0x0400, CRC(a6c268d4) SHA1(3b6c27f700ea4474f5354bbdcce82883c2e7e6e9) )
	ROM_LOAD( "car2.3",      0x0c00, 0x0400, CRC(17a09f24) SHA1(0cb40ec185f2ee3a26e943d84e8e2834d5f9d3ed) )
	ROM_LOAD( "car2.4",      0x1000, 0x0400, CRC(9af8a2e0) SHA1(92f45bc593fabf7a30615820b4b91677071bc67e) )
	ROM_LOAD( "car2.5",      0x1400, 0x0400, CRC(6975286c) SHA1(bcb5af18a991b9898fe28e69575c89c7b02d762d) )
	ROM_LOAD( "car2.6",      0x1800, 0x0400, CRC(4c19dd40) SHA1(0bdfed47594c7aa5ff655b507350fc6a912b6855) )
	ROM_LOAD( "car2.7",      0x1c00, 0x0400, CRC(41a93920) SHA1(e63df556f998b5e5d99d69a9fd200aaf0403f3f7) )

	ROM_REGION( 0x0020, "user1", 0 )	/* timing PROM */
	ROM_LOAD( "316-0206.u65", 0x0000, 0x0020, CRC(9617d796) SHA1(7cff2741866095ff42eadd8022bea349ec8d2f39) )	/* control PROM */
ROM_END

ROM_START( invho2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "271b.u33",     0x0000, 0x0400, CRC(44356a73) SHA1(6ff1050d84b6b7a006762c35e0b3d2befb0f90d6) )
	ROM_LOAD( "272b.u32",     0x0400, 0x0400, CRC(bd251265) SHA1(134f081c62173fab80b46918e4a073cf5f72df77) )
	ROM_LOAD( "273b.u31",     0x0800, 0x0400, CRC(2fc80cd9) SHA1(2790fb45233c8d6e74a56fcdfde1c468926d44d2) )
	ROM_LOAD( "274b.u30",     0x0c00, 0x0400, CRC(4fac4210) SHA1(9bb2fe888edab7e52a180d7f4d7fdab17392a736) )
	ROM_LOAD( "275b.u29",     0x1000, 0x0400, CRC(85af508e) SHA1(4b71c9583fe3b16a24f05d685d1edbd53ff81f81) )
	ROM_LOAD( "276b.u28",     0x1400, 0x0400, CRC(e305843a) SHA1(bb5113d3e0a4ca81e055da9c03755d0e6270d927) )
	ROM_LOAD( "277b.u27",     0x1800, 0x0400, CRC(b6b4221e) SHA1(8cfeff5ca7d29d973409df7f422428411462eab6) )
	ROM_LOAD( "278b.u26",     0x1c00, 0x0400, CRC(74d42250) SHA1(023227d314ec91c9b508b7fd60f163414165c25b) )
	ROM_LOAD( "279b.u8",      0x2000, 0x0400, CRC(8d30a3e0) SHA1(ac8cfca1b334d95e209bcfceeeeca31c03faecc8) )
	ROM_LOAD( "280b.u7",      0x2400, 0x0400, CRC(b5ee60ec) SHA1(dbc682b5770755fed8c04ef0f0311b2850228236) )
	ROM_LOAD( "281b.u6",      0x2800, 0x0400, CRC(21a6d4f2) SHA1(e8c8b263ffe53d5af50a561d038bfa96136767ad) )
	ROM_LOAD( "282b.u5",      0x2c00, 0x0400, CRC(07d54f8a) SHA1(f99af42ec24938cdd19c0ff7ac2b9e9882dc3655) )
	ROM_LOAD( "283b.u4",      0x3000, 0x0400, CRC(bdbe7ec1) SHA1(45e08c533acc538d88a0580535325b9ff1a60f2f) )
	ROM_LOAD( "284b.u3",      0x3400, 0x0400, CRC(ae9e9f16) SHA1(7f708ce49f34582d53abad0d811265dd28af899f) )
	ROM_LOAD( "285b.u2",      0x3800, 0x0400, CRC(8dc3ec34) SHA1(1f70027ad3b781244b672ee77225e32589c61e46) )
	ROM_LOAD( "286b.u1",      0x3c00, 0x0400, CRC(4bab9ba2) SHA1(a74881acf222466deb9c9e35dff532af6b10a7fc) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "316-0287.u49", 0x0000, 0x0020, CRC(d4374b01) SHA1(85ea0915f23571358e2e0c2b66b968e7b93f4bd6) )

	ROM_REGION( 0x0020, "user1", 0 )	/* timing PROM */
	ROM_LOAD( "316-0206.u14", 0x0000, 0x0020, CRC(9617d796) SHA1(7cff2741866095ff42eadd8022bea349ec8d2f39) )	/* control PROM */
ROM_END

ROM_START( sspacaho )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-0001.bin", 0x0000, 0x0800, CRC(ba62f57a) SHA1(7cfc079c6afe317b6c389c06802fdf1f83858510) )
	ROM_LOAD( "epr-0002.bin", 0x0800, 0x0800, CRC(94b3c59c) SHA1(e6ee1c25fb45d03d514421c231d794f9da05f47f) )
	ROM_LOAD( "epr-0003.bin", 0x1000, 0x0800, CRC(df13aef2) SHA1(61d210eeb59fe132e14fdd7eb6a39ebc55168097) )
	ROM_LOAD( "epr-0004.bin", 0x1800, 0x0800, CRC(8431e15e) SHA1(b028e718ee90f37c848e5f83494be61cb90338e2) )
	ROM_LOAD( "epr-0005.bin", 0x2000, 0x0800, CRC(eec2b6e7) SHA1(4ed830755b4d1da6111afc6c16c7c633521ccb9c) )
	ROM_LOAD( "epr-0006.bin", 0x2800, 0x0800, CRC(780e47ed) SHA1(533879b8abf0e69644fd8b784dbe9bf10cde6d9f) )
	ROM_LOAD( "epr-0007.bin", 0x3000, 0x0800, CRC(8189a2fa) SHA1(3c13a394f48adb8f7c6b8203bd0749921461ea06) )
	ROM_LOAD( "epr-0008.bin", 0x3800, 0x0800, CRC(34a64a80) SHA1(a588fae0ecaa80677887d8c95ef8896a4bdd77ee) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "316-0138.u44", 0x0000, 0x0020, CRC(67104ea9) SHA1(26b6bd2a1973b83bb9af4e3385d8cb14cb3f62f2) )

	ROM_REGION( 0x0020, "user1", 0 )	/* timing PROM */
	ROM_LOAD( "316-0206.u14", 0x0000, 0x0020, CRC(9617d796) SHA1(7cff2741866095ff42eadd8022bea349ec8d2f39) )	/* control PROM */
ROM_END

ROM_START( samurai )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-289.u33",   0x0000, 0x0400, CRC(a1a9cb03) SHA1(1875a86ad5938295dd5db6bb045be46eba8638ba) )
	ROM_LOAD( "epr-290.u32",   0x0400, 0x0400, CRC(49fede51) SHA1(58ab1779d555281ec436ae90dcdf4ada42625892) )
	ROM_LOAD( "epr-291.u31",   0x0800, 0x0400, CRC(6503dd72) SHA1(e0a3f42418a13f38314ec6e7951cd45c686fecbc) )
	ROM_LOAD( "epr-292.u30",   0x0c00, 0x0400, CRC(179c224f) SHA1(0a202718a9c2f5f4f1553d1dccd99bebb511363f) )
	ROM_LOAD( "epr-366.u29",   0x1000, 0x0400, CRC(3df2abec) SHA1(f52182c93026de0f5e7a3c36fe4a35e386c95d0c) )
	ROM_LOAD( "epr-355.u28",   0x1400, 0x0400, CRC(b24517a4) SHA1(51d613ac21e33c70705f1731b905af72dc561dbf) )
	ROM_LOAD( "epr-367.u27",   0x1800, 0x0400, CRC(992a6e5a) SHA1(45ae4bf297d7ce52d879d3af5e26960c0dd5034c) )
	ROM_LOAD( "epr-368.u26",   0x1c00, 0x0400, CRC(403c72ce) SHA1(eaa7a56a28b393db97fd6f1a62b7592cd47060f0) )
	ROM_LOAD( "epr-369.u8",    0x2000, 0x0400, CRC(3cfd115b) SHA1(58d1d34605b75f0078eb7e61eca8d5897a8b8294) )
	ROM_LOAD( "epr-370.u7",    0x2400, 0x0400, CRC(2c30db12) SHA1(9dcf8aa5cfdda5b350fc6b70524a15f970d98d91) )
	ROM_LOAD( "epr-299.u6",    0x2800, 0x0400, CRC(87c71139) SHA1(10c273c2f6a58bba8b5891dfd851e18898b45fd1) )
	ROM_LOAD( "epr-371.u5",    0x2c00, 0x0400, CRC(761f56cf) SHA1(0d63e8b0ac7bfe9f3cd08cea68eb941b1a5a536c) )
	ROM_LOAD( "epr-301.u4",    0x3000, 0x0400, CRC(23de1ff7) SHA1(51bf437a62ee770918524c8d0e8b0e007a800021) )
	ROM_LOAD( "epr-372.u3",    0x3400, 0x0400, CRC(292cfd89) SHA1(a8131f03e8e9f5009508813445bbea559bc27726) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "pr55.clr",     0x0000, 0x0020, CRC(975f5fb0) SHA1(d5917d68ad5549fe5cc997521c3b0a5a279d2231) )

	ROM_REGION( 0x0040, "user1", 0 )	/* misc PROMs */
	ROM_LOAD( "316-0043.u87", 0x0000, 0x0020, CRC(e60a7960) SHA1(b8b8716e859c57c35310efc4594262afedb84823) )	/* control PROM */
	ROM_LOAD( "316-0042.u88", 0x0020, 0x0020, CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) )	/* sequence PROM */
ROM_END

ROM_START( invinco )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "310a.u27",     0x0000, 0x0400, CRC(e3931365) SHA1(e34083004515ad45ddbf9ab89c34473b6c5d46fb) )
	ROM_LOAD( "311a.u26",     0x0400, 0x0400, CRC(de1a6c4a) SHA1(ca7ab7b4c77319f7923d56ad8b60d16211af19bc) )
	ROM_LOAD( "312a.u25",     0x0800, 0x0400, CRC(e3c08f39) SHA1(13c177980722559ec885565d3f889b830322f25e) )
	ROM_LOAD( "313a.u24",     0x0c00, 0x0400, CRC(b680b306) SHA1(b03a5621bdf6a0bdc79af9361a2c0cc339b50b4b) )
	ROM_LOAD( "314a.u23",     0x1000, 0x0400, CRC(790f07d9) SHA1(222e1c392c92fc547a67d96410f9a4277f3bb6cb) )
	ROM_LOAD( "315a.u22",     0x1400, 0x0400, CRC(0d13bed2) SHA1(c23d937acad2171aae8f76671843a841f62d147f) )
	ROM_LOAD( "316a.u21",     0x1800, 0x0400, CRC(88d7eab8) SHA1(272b099414b68a428f9538f0219f92569525b87c) )
	ROM_LOAD( "317a.u20",     0x1c00, 0x0400, CRC(75389463) SHA1(277ba127f322caf2486735b82bf7b96bb9e34a56) )
	ROM_LOAD( "318a.uxx",     0x2000, 0x0400, CRC(0780721d) SHA1(580aefb382702babdf8248e36330c4b22e8579c8) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "316-246.u44",  0x0000, 0x0020, CRC(fe4406cb) SHA1(92e2459420a7f7412f02cfaf68604fc233b0a245) )
ROM_END

ROM_START( invds )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "367.u33",      0x0000, 0x0400, CRC(e6a33eae) SHA1(16de70e8fcd093964a448a86bc89b1c607152ead) )
	ROM_LOAD( "368.u32",      0x0400, 0x0400, CRC(421554a8) SHA1(efb6759998e36322258c172aa7e8ba6416b3235f) )
	ROM_LOAD( "369.u31",      0x0800, 0x0400, CRC(531e917a) SHA1(f1d48e18f51de36d01bafab5bfa68d16a8c0192b) )
	ROM_LOAD( "370.u30",      0x0c00, 0x0400, CRC(2ad68f8c) SHA1(0ceee2d04e239856ecf18f93b5ee75d72e031f92) )
	ROM_LOAD( "371.u29",      0x1000, 0x0400, CRC(1b98dc5c) SHA1(ac8a1226405cab9d6049304163d658c5ae611c1a) )
	ROM_LOAD( "372.u28",      0x1400, 0x0400, CRC(3a72190a) SHA1(f2852b3ca5fce274ed5452d28d06a3a27fe8a94f) )
	ROM_LOAD( "373.u27",      0x1800, 0x0400, CRC(3d361520) SHA1(bf8441cd5050f643535229b4761310f7dc8a2997) )
	ROM_LOAD( "374.u26",      0x1c00, 0x0400, CRC(e606e7d9) SHA1(66efa2beda9fcc2f3cd8fc77c99c196de30d1a30) )
	ROM_LOAD( "375.u8",       0x2000, 0x0400, CRC(adbe8d32) SHA1(5c883f0053c4f8124b81c49847b00301edb9f654) )
	ROM_LOAD( "376.u7",       0x2400, 0x0400, CRC(79409a46) SHA1(12772cf3e06d5127adb16396c4589cd9a06cbce5) )
	ROM_LOAD( "377.u6",       0x2800, 0x0400, CRC(3f021a71) SHA1(ba4a3833c6f7d28c3033de77882ff7ee0b96b190) )
	ROM_LOAD( "378.u5",       0x2c00, 0x0400, CRC(49a542b0) SHA1(5aa90df413ac0498cadaff9ca805896066e0f8a8) )
	ROM_LOAD( "379.u4",       0x3000, 0x0400, CRC(ee140e49) SHA1(5c29e34afdf91b68554e678699179802616c1f94) )
	ROM_LOAD( "380.u3",       0x3400, 0x0400, CRC(688ba831) SHA1(24f977c99714a08bf14c7753d42e2744aa86c396) )
	ROM_LOAD( "381.u2",       0x3800, 0x0400, CRC(798ba0c7) SHA1(7867d107f95077c539263619155c7f3ce4ef0968) )
	ROM_LOAD( "382.u1",       0x3c00, 0x0400, CRC(8d195c24) SHA1(5c314947ba13112b4154d3be069892cca4f5da42) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "316-246",      0x0000, 0x0020, CRC(fe4406cb) SHA1(92e2459420a7f7412f02cfaf68604fc233b0a245) )

	ROM_REGION( 0x0020, "user1", 0 )	/* misc PROM */
	ROM_LOAD( "316-0206.u14", 0x0000, 0x0020, CRC(9617d796) SHA1(7cff2741866095ff42eadd8022bea349ec8d2f39) )	/* control PROM */
ROM_END

ROM_START( tranqgun )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u33.bin",      0x0000, 0x0400, CRC(6d50e902) SHA1(1d14c0b28cb3650bb57b9ef61265fe94c453d648) )
	ROM_LOAD( "u32.bin",      0x0400, 0x0400, CRC(f0ba0e60) SHA1(fcdd4355ccc1893a8a0450403f466bee916793dc) )
	ROM_LOAD( "u31.bin",      0x0800, 0x0400, CRC(9fe440d3) SHA1(a0be6caf3ea821c84be83351a0b80c240485218f) )
	ROM_LOAD( "u30.bin",      0x0c00, 0x0400, CRC(1041608e) SHA1(9f9667a4ad98f43c606b534157488fb7dd7b18ba) )
	ROM_LOAD( "u29.bin",      0x1000, 0x0400, CRC(fb5de95f) SHA1(7ff57c2e7b074e74936fde808f455a118dc495c2) )
	ROM_LOAD( "u28.bin",      0x1400, 0x0400, CRC(03fd8727) SHA1(d334c50adcaea02f9b55b4ec1b4c7dda3a0298ca) )
	ROM_LOAD( "u27.bin",      0x1800, 0x0400, CRC(3d93239b) SHA1(211748992f5735cf4b8a69f035aa89b40c814d75) )
	ROM_LOAD( "u26.bin",      0x1c00, 0x0400, CRC(20f64a7f) SHA1(e61e7dd0f418c17954b72d443a1de206ef8cf4d0) )
	ROM_LOAD( "u8.bin",       0x2000, 0x0400, CRC(5121c695) SHA1(6b8ae30f2f17ff886e6c61783613a60b5c3d9b82) )
	ROM_LOAD( "u7.bin",       0x2400, 0x0400, CRC(b13d21f7) SHA1(f624e96c1dada3222b6b5a83f0bdf52b821077d6) )
	ROM_LOAD( "u6.bin",       0x2800, 0x0400, CRC(603cee59) SHA1(7c1f55cfe36cf52687f1c60d9277b46bd12054d4) )
	ROM_LOAD( "u5.bin",       0x2c00, 0x0400, CRC(7f25475f) SHA1(f519d59872d9429300040cb0ce940e8103087763) )
	ROM_LOAD( "u4.bin",       0x3000, 0x0400, CRC(57dc3123) SHA1(db57cb77d840bc97d0fa9de28cc4eba90b0319d0) )
	ROM_LOAD( "u3.bin",       0x3400, 0x0400, CRC(7aa7829b) SHA1(ef9835d83d4468a28aa2a837b3c23760d9e5480e) )
	ROM_LOAD( "u2.bin",       0x3800, 0x0400, CRC(a9b10df5) SHA1(f74e83c5f51ecb53e3190c1ae83fa59b60806f62) )
	ROM_LOAD( "u1.bin",       0x3c00, 0x0400, CRC(431a7449) SHA1(67a142c084078ebebe9ed91b577fffe1ab0f39a2) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "u49.bin",      0x0000, 0x0020, CRC(6481445b) SHA1(2136408f25a95ed73882deaa5a174d4a1a7ba438) )

	ROM_REGION( 0x0040, "user1", 0 )	/* misc PROMs */
	ROM_LOAD( "316-0043.u87", 0x0000, 0x0020, CRC(e60a7960) SHA1(b8b8716e859c57c35310efc4594262afedb84823) )	/* control PROM */
	ROM_LOAD( "316-0042.u88", 0x0020, 0x0020, CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) )	/* sequence PROM */
ROM_END

ROM_START( spacetrk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u33.bin",      0x0000, 0x0400, CRC(9033fe50) SHA1(0a9b86af03956575403d8b494963f55887fc4dc3) )
	ROM_LOAD( "u32.bin",      0x0400, 0x0400, CRC(08f61f0d) SHA1(f206b18959e2cb6d4f6962415695eca0412da739) )
	ROM_LOAD( "u31.bin",      0x0800, 0x0400, CRC(1088a8c4) SHA1(ca40098137f0d90548d62a4daead3c6bc488fde0) )
	ROM_LOAD( "u30.bin",      0x0c00, 0x0400, CRC(55560cc8) SHA1(751585e261088cc740aa293a1e71f13b928c37f9) )
	ROM_LOAD( "u29.bin",      0x1000, 0x0400, CRC(71713958) SHA1(85399d3cbe5ed265b59b4441c17247b09c4c34d0) )
	ROM_LOAD( "u28.bin",      0x1400, 0x0400, CRC(7bcf5ca3) SHA1(05a6d31c0229e840081cad7fb17bdebc0fd6484f) )
	ROM_LOAD( "u27.bin",      0x1800, 0x0400, CRC(ad7a2065) SHA1(04a98b4578c7770f69525c614c402ccf2f3c99ed) )
	ROM_LOAD( "u26.bin",      0x1c00, 0x0400, CRC(6060fe77) SHA1(d7606daabdb0212daef6724b0ba6abab13731080) )
	ROM_LOAD( "u8.bin",       0x2000, 0x0400, CRC(75a90624) SHA1(61f10dd66ba1daa1b37c407d22d2ea3da2c8fec8) )
	ROM_LOAD( "u7.bin",       0x2400, 0x0400, CRC(7b31a2ab) SHA1(58ebf8ccb2e807aacc24c0017cb8c094124c1776) )
	ROM_LOAD( "u6.bin",       0x2800, 0x0400, CRC(94135b33) SHA1(deb836034ab1de016a26d45e7176f292aa34038c) )
	ROM_LOAD( "u5.bin",       0x2c00, 0x0400, CRC(cfbf2538) SHA1(d200ae1a866a6e4587226e449a3d09d330a79cf4) )
	ROM_LOAD( "u4.bin",       0x3000, 0x0400, CRC(b4b95129) SHA1(04c4ced1b8d0bf2cbb2533d9870574395a5f0793) )
	ROM_LOAD( "u3.bin",       0x3400, 0x0400, CRC(03ca1d70) SHA1(66ed7664ee1ddbadd6993d44c3684e6cb47912b3) )
	ROM_LOAD( "u2.bin",       0x3800, 0x0400, CRC(a968584b) SHA1(4ea4b83e116627daec4d226591924f84740e7786) )
	ROM_LOAD( "u1.bin",       0x3c00, 0x0400, CRC(e6e300e8) SHA1(4e198915c4f2b904dcc9ad81e5106bd711546e4e) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "u49.bin",      0x0000, 0x0020, CRC(aabae4cd) SHA1(6748d20318aed1c9949a3373166ebdca13eae965) )

	ROM_REGION( 0x0040, "user1", 0 )	/* misc PROMs */
	ROM_LOAD( "316-0043.u87", 0x0000, 0x0020, CRC(e60a7960) SHA1(b8b8716e859c57c35310efc4594262afedb84823) )	/* control PROM */
	ROM_LOAD( "316-0042.u88", 0x0020, 0x0020, CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) )	/* sequence PROM */
ROM_END

ROM_START( spacetrkc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u33c.bin",     0x0000, 0x0400, CRC(b056b928) SHA1(1bbf5c30b226c5ca3c09fcff36a1b21132a524b6) )
	ROM_LOAD( "u32c.bin",     0x0400, 0x0400, CRC(dffb11d9) SHA1(5c95b7e493ac9e8714d91d19b6f01967559ce55c) )
	ROM_LOAD( "u31c.bin",     0x0800, 0x0400, CRC(9b25d46f) SHA1(29362fe4587f6425cc55c6f5592f853caf8bc1e2) )
	ROM_LOAD( "u30c.bin",     0x0c00, 0x0400, CRC(3a612bfe) SHA1(914a9a1192bfb255ee8b72671d33b5ae17b3a1b7) )
	ROM_LOAD( "u29c.bin",     0x1000, 0x0400, CRC(d8bb6e0c) SHA1(6bf57dde384a447350deb7125e287af147d51878) )
	ROM_LOAD( "u28c.bin",     0x1400, 0x0400, CRC(0e367740) SHA1(5140244d4cd64727ddfe052b7b4880e9599998f9) )
	ROM_LOAD( "u27c.bin",     0x1800, 0x0400, CRC(d59fec86) SHA1(8f0163aa61fb35a9977e24c36f7c01c3fb0bc156) )
	ROM_LOAD( "u26c.bin",     0x1c00, 0x0400, CRC(9deefa0f) SHA1(d109743e737931227bb504c47ba3cc959ae24756) )
	ROM_LOAD( "u8c.bin",      0x2000, 0x0400, CRC(613116c5) SHA1(23ad643a3df0d3823b776422454cba2e590a49c6) )
	ROM_LOAD( "u7c.bin",      0x2400, 0x0400, CRC(3bdf2464) SHA1(a19a863da8fb33650483ba5b4ea133f1c6393620) )
	ROM_LOAD( "u6c.bin",      0x2800, 0x0400, CRC(039d73fa) SHA1(08498e76b4f29f22078dfbe7ec52f47528f6db42) )
	ROM_LOAD( "u5c.bin",      0x2c00, 0x0400, CRC(1638344f) SHA1(0f8488763f3f3776ab09eb5cf1d4db8ebd384345) )
	ROM_LOAD( "u4c.bin",      0x3000, 0x0400, CRC(e34443cd) SHA1(8f2302f48aaceae70d8e576f318f44e96f92d4f3) )
	ROM_LOAD( "u3c.bin",      0x3400, 0x0400, CRC(6f16cbd7) SHA1(ef32558731d374a1417bb58ba24c1aa79d71ecd8) )
	ROM_LOAD( "u2c.bin",      0x3800, 0x0400, CRC(94da3cdc) SHA1(082f96c6fad0ccb314e80efd3a2d7e156a55c48e) )
	ROM_LOAD( "u1c.bin",      0x3c00, 0x0400, CRC(2a228bf4) SHA1(7c39700c7a63ca3202134acefa8e6f04f7145dfd) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "u49.bin",      0x0000, 0x0020, CRC(aabae4cd) SHA1(6748d20318aed1c9949a3373166ebdca13eae965) )

	ROM_REGION( 0x0040, "user1", 0 )	/* misc PROMs */
	ROM_LOAD( "316-0043.u87", 0x0000, 0x0020, CRC(e60a7960) SHA1(b8b8716e859c57c35310efc4594262afedb84823) )	/* control PROM */
	ROM_LOAD( "316-0042.u88", 0x0020, 0x0020, CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) )	/* sequence PROM */
ROM_END

ROM_START( carnival )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-651.u33",   0x0000, 0x0400, CRC(9f2736e6) SHA1(c3fb9197b5e83dc7d5335de2268e0acb30cf8328) )
	ROM_LOAD( "epr-652.u32",   0x0400, 0x0400, CRC(a1f58beb) SHA1(e027beca7bf3ef5ef67e2195f909332fd194b5dc) )
	ROM_LOAD( "epr-653.u31",   0x0800, 0x0400, CRC(67b17922) SHA1(46cdfd0371dec61a5440c2111660729c0f0ecdb8) )
	ROM_LOAD( "epr-654.u30",   0x0c00, 0x0400, CRC(befb09a5) SHA1(da44b6a869b5eb0705e01fee4478696f6bef9de8) )
	ROM_LOAD( "epr-655.u29",   0x1000, 0x0400, CRC(623fcdad) SHA1(35890964f5cf799c141002916641089ccec0fcc9) )
	ROM_LOAD( "epr-656.u28",   0x1400, 0x0400, CRC(53040332) SHA1(ff7a06d93cb890abf0616770774668396d128ba3) )
	ROM_LOAD( "epr-657.u27",   0x1800, 0x0400, CRC(f2537467) SHA1(262b859098f4f7e5e9bf2f83bda833044824226e) )
	ROM_LOAD( "epr-658.u26",   0x1c00, 0x0400, CRC(fcc3854e) SHA1(7adbd6ca6f636dec75fa6eccdf3381686e074bc6) )
	ROM_LOAD( "epr-659.u8",    0x2000, 0x0400, CRC(28be8d69) SHA1(2d9ac9a53f00fe2282e4317585e6bddadb676c0f) )
	ROM_LOAD( "epr-660.u7",    0x2400, 0x0400, CRC(3873ccdb) SHA1(56be81fdee8947758ba966915c0739e5560a7f94) )
	ROM_LOAD( "epr-661.u6",    0x2800, 0x0400, CRC(d9a96dff) SHA1(0366acf3418901bfeeda59d4cd51fe8ceaad4577) )
	ROM_LOAD( "epr-662.u5",    0x2c00, 0x0400, CRC(d893ca72) SHA1(564176ab7f3757d51db8eef9fbc4228fa2ce328f) )
	ROM_LOAD( "epr-663.u4",    0x3000, 0x0400, CRC(df8c63c5) SHA1(e8d0632b5cb5bd7f698485531f3edeb13efdc685) )
	ROM_LOAD( "epr-664.u3",    0x3400, 0x0400, CRC(689a73e8) SHA1(b4134e8d892df7ba3352e4d3f581923decae6e54) )
	ROM_LOAD( "epr-665.u2",    0x3800, 0x0400, CRC(28e7b2b6) SHA1(57eb5dd0f11da8ff8001e76036264246d6bc27d2) )
	ROM_LOAD( "epr-666.u1",    0x3c00, 0x0400, CRC(4eec7fae) SHA1(cdc858165136c55b01511805c9d4dc6bc598fe1f) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "316-633",      0x0000, 0x0020, CRC(f0084d80) SHA1(95ec912ac2c64cd58a50c68afc0993746841a531) )

	ROM_REGION( 0x0800, "audiocpu", 0 )	/* sound ROM */
	ROM_LOAD( "epr-412",     0x0000, 0x0400, CRC(0dbaa2b0) SHA1(eae7fc362a0ff8f908c42e093c7dbb603659373c) )

	ROM_REGION( 0x0020, "user1", 0 )	/* timing PROM */
	ROM_LOAD( "316-0206.u14", 0x0000, 0x0020, CRC(9617d796) SHA1(7cff2741866095ff42eadd8022bea349ec8d2f39) )	/* control PROM */
ROM_END

ROM_START( carnivalc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-501.u33", 0x0000, 0x0400, CRC(688503d2) SHA1(a1fe03c23276d458ba74f7473524918eb9b7c7e5) )
	ROM_LOAD( "epr-652.u32", 0x0400, 0x0400, CRC(a1f58beb) SHA1(e027beca7bf3ef5ef67e2195f909332fd194b5dc) )
	ROM_LOAD( "epr-653.u31", 0x0800, 0x0400, CRC(67b17922) SHA1(46cdfd0371dec61a5440c2111660729c0f0ecdb8) )
	ROM_LOAD( "epr-654.u30", 0x0c00, 0x0400, CRC(befb09a5) SHA1(da44b6a869b5eb0705e01fee4478696f6bef9de8) )
	ROM_LOAD( "epr-655.u29", 0x1000, 0x0400, CRC(623fcdad) SHA1(35890964f5cf799c141002916641089ccec0fcc9) )
	ROM_LOAD( "epr-506.u28", 0x1400, 0x0400, CRC(ba916e97) SHA1(87e1ac8bd21bafb815b702048c6be24f410a8998) )
	ROM_LOAD( "epr-507.u27", 0x1800, 0x0400, CRC(d0bda4a5) SHA1(faf3f3c2a8f962c6c2901e5a4a31b452b506ee22) )
	ROM_LOAD( "epr-508.u26", 0x1c00, 0x0400, CRC(f0258cad) SHA1(065c90835dadeda7085422295cde09491c94b6e0) )
	ROM_LOAD( "epr-509.u8",  0x2000, 0x0400, CRC(dcc8a530) SHA1(2d1cac3b40f5afab5423d45ecc3f5565266f9c03) )
	ROM_LOAD( "epr-510.u7",  0x2400, 0x0400, CRC(92c2ba51) SHA1(c0c0c836d2aa6943bd808acc12161adf23cd0d21) )
	ROM_LOAD( "epr-511.u6",  0x2800, 0x0400, CRC(3af899a0) SHA1(540cbdc6a912cb325bbea33935bfa06e13f0021a) )
	ROM_LOAD( "epr-512.u5",  0x2c00, 0x0400, CRC(09f7b3e6) SHA1(18a58680500148e7a031e91230d73b2ce4dc712d) )
	ROM_LOAD( "epr-513.u4",  0x3000, 0x0400, CRC(8f41974c) SHA1(65e9c2378ad99f804590de36ffba4c60bfa1bfe3) )
	ROM_LOAD( "epr-514.u3",  0x3400, 0x0400, CRC(2788d140) SHA1(7341c44fb1f7eb8c4d25d3e1e75bcf3bfdb9a89c) )
	ROM_LOAD( "epr-515.u2",  0x3800, 0x0400, CRC(10decaa9) SHA1(4c980164dde275e1488ae74a7ad61e6acc75e608) )
	ROM_LOAD( "epr-516.u1",  0x3c00, 0x0400, CRC(7c32b352) SHA1(8cb472a7f71a301417c6a8e4a26a9bdcd43b6062) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "316-633",      0x0000, 0x0020, CRC(f0084d80) SHA1(95ec912ac2c64cd58a50c68afc0993746841a531) )

	ROM_REGION( 0x0800, "audiocpu", 0 )	/* sound ROM */
	ROM_LOAD( "epr-412",     0x0000, 0x0400, CRC(0dbaa2b0) SHA1(eae7fc362a0ff8f908c42e093c7dbb603659373c) )

	ROM_REGION( 0x0020, "user1", 0 )	/* timing PROM */
	ROM_LOAD( "316-0206.u14", 0x0000, 0x0020, CRC(9617d796) SHA1(7cff2741866095ff42eadd8022bea349ec8d2f39) )	/* control PROM */
ROM_END

ROM_START( carnivalh )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-155.u48",   0x0000, 0x0800, CRC(0a5f1f65) SHA1(7830157ad378f92a8069debf78d50b9deafb4d40) )
	ROM_LOAD( "epr-156.u47",   0x0800, 0x0800, CRC(422221ff) SHA1(57180236dd1cf5838131b35256c3d46cb8015804) )
	ROM_LOAD( "epr-157.u46",   0x1000, 0x0800, CRC(1551dffb) SHA1(f91563a14c5febee64b45579acf49518cd54bec3) )
	ROM_LOAD( "epr-158.u45",   0x1800, 0x0800, CRC(9238b5c0) SHA1(673f4fedd2e7beef6bc5e6371b2ed7722f3c141c) )
	ROM_LOAD( "epr-159.u44",   0x2000, 0x0800, CRC(5c2b9a33) SHA1(2cc57ceebd2938e589c07892b6587b8571678fcc) )
	ROM_LOAD( "epr-160.u43",   0x2800, 0x0800, CRC(dd70471f) SHA1(2d987e946873955998c428930592ea7734970b16) )
	ROM_LOAD( "epr-161.u42",   0x3000, 0x0800, CRC(42714a0d) SHA1(96f8904912d85b2e7037d129ff2443e90693fe22) )
	ROM_LOAD( "epr-162.u41",   0x3800, 0x0800, CRC(56e1c120) SHA1(24816b6a9bc238571ab8ea79bb876cf249ed4d60) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "pr-62.u44",      0x0000, 0x0020, CRC(f0084d80) SHA1(95ec912ac2c64cd58a50c68afc0993746841a531) ) /* Same as 316-633 */

	ROM_REGION( 0x0800, "audiocpu", 0 )	/* sound ROM */
	ROM_LOAD( "epr-412.u5",     0x0000, 0x0400, CRC(0dbaa2b0) SHA1(eae7fc362a0ff8f908c42e093c7dbb603659373c) )

	ROM_REGION( 0x0040, "user1", 0 )	/* misc PROMs (type n82s123) */
	ROM_LOAD( "316-043.u65", 0x0000, 0x0020, CRC(e60a7960) SHA1(b8b8716e859c57c35310efc4594262afedb84823) )	/* control PROM */
	ROM_LOAD( "316-042.u66", 0x0020, 0x0020, CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) )	/* sequence PROM */
ROM_END

ROM_START( carnivalha )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-155.u48",   0x0000, 0x0800, CRC(0a5f1f65) SHA1(7830157ad378f92a8069debf78d50b9deafb4d40) )
	ROM_LOAD( "epr-156.u47",   0x0800, 0x0800, CRC(422221ff) SHA1(57180236dd1cf5838131b35256c3d46cb8015804) )
	ROM_LOAD( "epr-157.u46",   0x1000, 0x0800, CRC(1551dffb) SHA1(f91563a14c5febee64b45579acf49518cd54bec3) )
	ROM_LOAD( "epr-158.u45",   0x1800, 0x0800, CRC(9238b5c0) SHA1(673f4fedd2e7beef6bc5e6371b2ed7722f3c141c) )
	ROM_LOAD( "epr-159.u44",   0x2000, 0x0800, CRC(5c2b9a33) SHA1(2cc57ceebd2938e589c07892b6587b8571678fcc) )
	ROM_LOAD( "epr-160.u43",   0x2800, 0x0800, CRC(dd70471f) SHA1(2d987e946873955998c428930592ea7734970b16) )
	ROM_LOAD( "epr-161x.u42",  0x3000, 0x0800, CRC(8133ba08) SHA1(fc314e847810140d99ad071667c10a1a57c9f892) ) // rom marked with 161X, probably modified in some way
	ROM_LOAD( "epr-162.u41",   0x3800, 0x0800, CRC(56e1c120) SHA1(24816b6a9bc238571ab8ea79bb876cf249ed4d60) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "pr-62.u44",      0x0000, 0x0020, CRC(f0084d80) SHA1(95ec912ac2c64cd58a50c68afc0993746841a531) ) /* Same as 316-633 */

	ROM_REGION( 0x0800, "audiocpu", 0 )	/* sound ROM */
	ROM_LOAD( "epr-412.u5",     0x0000, 0x0400, CRC(0dbaa2b0) SHA1(eae7fc362a0ff8f908c42e093c7dbb603659373c) )

	ROM_REGION( 0x0040, "user1", 0 )	/* misc PROMs (type n82s123) */
	ROM_LOAD( "316-043.u65", 0x0000, 0x0020, CRC(e60a7960) SHA1(b8b8716e859c57c35310efc4594262afedb84823) )	/* control PROM */
	ROM_LOAD( "316-042.u66", 0x0020, 0x0020, CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) )	/* sequence PROM */
ROM_END

ROM_START( brdrline )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b1.bin",       0x0000, 0x0400, CRC(df182769) SHA1(2b1b70c6282b32e0a4ed80ab4e6b20f90630e910) )
	ROM_LOAD( "b2.bin",       0x0400, 0x0400, CRC(e1d1c4ce) SHA1(86320c836577549af6fe6c311f8475a51de52627) )
	ROM_LOAD( "b3.bin",       0x0800, 0x0400, CRC(4ec4afa2) SHA1(dd5b97f1a37cd655064b773e4a755b87de4c6a3f) )
	ROM_LOAD( "b4.bin",       0x0c00, 0x0400, CRC(88de95f6) SHA1(fe3388346785ad15dead89913e4ff36120a83599) )
	ROM_LOAD( "b5.bin",       0x1000, 0x0400, CRC(2e4e13b9) SHA1(bdf31c11733127b8b77fa72933d3b9dc6834d5d8) )
	ROM_LOAD( "b6.bin",       0x1400, 0x0400, CRC(c181e87a) SHA1(426e1ce15477039e4a19b536500f387518026efc) )
	ROM_LOAD( "b7.bin",       0x1800, 0x0400, CRC(21180015) SHA1(b23f876db1a9a986f1087ead07a01e836d5ee842) )
	ROM_LOAD( "b8.bin",       0x1c00, 0x0400, CRC(56a7fee0) SHA1(495efa91773fd3cf36da4e538893db08e64e5bab) )
	ROM_LOAD( "b9.bin",       0x2000, 0x0400, CRC(bb532e63) SHA1(da511e0be58b13781897e6efb5a59a3558016b12) )
	ROM_LOAD( "b10.bin",      0x2400, 0x0400, CRC(64793709) SHA1(fabfb783f1d93a3d9454fc345a64498e4b5b9138) )
	ROM_LOAD( "b11.bin",      0x2800, 0x0400, CRC(2ae2f928) SHA1(afd99c800801d38ee59008344bd9a3901f72ff50) )
	ROM_LOAD( "b12.bin",      0x2c00, 0x0400, CRC(e14cfaf5) SHA1(d159e93f703aae3c04da08102ff718d5a4ca7a91) )
	ROM_LOAD( "b13.bin",      0x3000, 0x0400, CRC(605e0d27) SHA1(771de6d31ee7896a2441f1df4565027793d99989) )
	ROM_LOAD( "b14.bin",      0x3400, 0x0400, CRC(93f5714f) SHA1(50a043be1e1cf8b1aeb846571a12fe70cbb3477e) )
	ROM_LOAD( "b15.bin",      0x3800, 0x0400, CRC(2f8a9b1c) SHA1(853d5ca017b133c1f13f703cceb20f04199d4752) )
	ROM_LOAD( "b16.bin",      0x3c00, 0x0400, CRC(cc138bed) SHA1(7d3eebdeaff19783d5ef20a7ececec00773434fc) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "borderc.49",   0x0000, 0x0020, CRC(bc6be94e) SHA1(34e113ec25e19212b74907d35be5cb8714a8249c) )

	ROM_REGION( 0x0800, "cpu1", 0 )	/* sound ROM */
	ROM_LOAD( "au.bin",       0x0000, 0x0400, CRC(a23e1d9f) SHA1(ce209571f6341aa6f036a015e666673098bc98ea) )

	ROM_REGION( 0x0100, "user1", 0 )	/* misc PROM */
	ROM_LOAD( "border.32",   0x0000, 0x0020, CRC(c128d0ba) SHA1(0ce9febbb7e2f5388ed999a479e3d385dba0b342) )
	ROM_LOAD( "bordera.15",  0x0000, 0x0020, CRC(6449e678) SHA1(421c45c8fba3c2bc2a7ebbea2c837c8fa1a5a2f3) )
	ROM_LOAD( "borderb.14",  0x0000, 0x0020, CRC(55dcdef1) SHA1(6fbd041edc258b7e1b99bbe9526612cfb1b541f8) )
	/* following 2 from sound board */
	ROM_LOAD( "prom93427.1", 0x0000, 0x0100, CRC(64b98dc7) SHA1(f0bb7d0b4b56cc2936ce4cbec165394f3026ed6d) )
	ROM_LOAD( "prom93427.2", 0x0000, 0x0100, CRC(bda82367) SHA1(1c96453c2ae372892c39b5657cf2b252a90a10a9) )
ROM_END


/*
Notes on Sidam set

This set isn't really very interesting, it just has the code to draw the status bar text patched out.
Only one ROM is changed.  Supported because Sidam were a well known Italian bootlegger so this has
some historical importance.  'Sidam' isn't displayed anywhere ingame, only on the PCB?.

---CPU

Lower board (label "BLC200681"):
1x MK3880-Z80CPU (main)
1x oscillator 10.000MHz

---ROMs:

Lower board (label "BLC200681"):
16x F2708
3x PROM 5610

Upper board (label "BLC300681 MADE IN ITALY"):
1x F2708
1x PROM 82S123
2x PROM 93427

---Note:

Lower board (label "BLC200681"):
1x 22x2 edge connector
1x 8 switches dip

Upper board (label "BLC300681 MADE IN ITALY"):
7x trimmer
*/

ROM_START( brdrlins )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.33",       0x0000, 0x0400, CRC(df182769) SHA1(2b1b70c6282b32e0a4ed80ab4e6b20f90630e910) )
	ROM_LOAD( "2.32",       0x0400, 0x0400, CRC(98b26e2a) SHA1(ae1c3726827571ee1c2716e1044d6aae5608c0af) ) // 3 bytes changed in this rom
	ROM_LOAD( "3.31",       0x0800, 0x0400, CRC(4ec4afa2) SHA1(dd5b97f1a37cd655064b773e4a755b87de4c6a3f) )
	ROM_LOAD( "4.30",       0x0c00, 0x0400, CRC(88de95f6) SHA1(fe3388346785ad15dead89913e4ff36120a83599) )
	ROM_LOAD( "5.29",       0x1000, 0x0400, CRC(2e4e13b9) SHA1(bdf31c11733127b8b77fa72933d3b9dc6834d5d8) )
	ROM_LOAD( "6.28",       0x1400, 0x0400, CRC(c181e87a) SHA1(426e1ce15477039e4a19b536500f387518026efc) )
	ROM_LOAD( "7.27",       0x1800, 0x0400, CRC(21180015) SHA1(b23f876db1a9a986f1087ead07a01e836d5ee842) )
	ROM_LOAD( "8.26",       0x1c00, 0x0400, CRC(56a7fee0) SHA1(495efa91773fd3cf36da4e538893db08e64e5bab) )
	ROM_LOAD( "9.8",        0x2000, 0x0400, CRC(bb532e63) SHA1(da511e0be58b13781897e6efb5a59a3558016b12) )
	ROM_LOAD( "10.7",       0x2400, 0x0400, CRC(64793709) SHA1(fabfb783f1d93a3d9454fc345a64498e4b5b9138) )
	ROM_LOAD( "11.6",       0x2800, 0x0400, CRC(2ae2f928) SHA1(afd99c800801d38ee59008344bd9a3901f72ff50) )
	ROM_LOAD( "12.5",       0x2c00, 0x0400, CRC(e14cfaf5) SHA1(d159e93f703aae3c04da08102ff718d5a4ca7a91) )
	ROM_LOAD( "13.4",       0x3000, 0x0400, CRC(605e0d27) SHA1(771de6d31ee7896a2441f1df4565027793d99989) )
	ROM_LOAD( "14.3",       0x3400, 0x0400, CRC(93f5714f) SHA1(50a043be1e1cf8b1aeb846571a12fe70cbb3477e) )
	ROM_LOAD( "15.2",       0x3800, 0x0400, CRC(2f8a9b1c) SHA1(853d5ca017b133c1f13f703cceb20f04199d4752) )
	ROM_LOAD( "16.1",       0x3c00, 0x0400, CRC(cc138bed) SHA1(7d3eebdeaff19783d5ef20a7ececec00773434fc) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "5610.49",   0x0000, 0x0020, CRC(bc6be94e) SHA1(34e113ec25e19212b74907d35be5cb8714a8249c) )

	ROM_REGION( 0x0800, "cpu1", 0 )	/* sound ROM */
	ROM_LOAD( "au.bin",       0x0000, 0x0400, CRC(a23e1d9f) SHA1(ce209571f6341aa6f036a015e666673098bc98ea) )

	ROM_REGION( 0x0100, "user1", 0 )	/* misc PROM */
	ROM_LOAD( "82s123.bin",   0x0000, 0x0020, CRC(c128d0ba) SHA1(0ce9febbb7e2f5388ed999a479e3d385dba0b342) )
	ROM_LOAD( "5610.15",  0x0000, 0x0020, CRC(6449e678) SHA1(421c45c8fba3c2bc2a7ebbea2c837c8fa1a5a2f3) )
	ROM_LOAD( "5610.14",  0x0000, 0x0020, CRC(55dcdef1) SHA1(6fbd041edc258b7e1b99bbe9526612cfb1b541f8) )
	/* following 2 from sound board */
	ROM_LOAD( "93427.1", 0x0000, 0x0100, CRC(64b98dc7) SHA1(f0bb7d0b4b56cc2936ce4cbec165394f3026ed6d) )
	ROM_LOAD( "93427.2", 0x0000, 0x0100, CRC(bda82367) SHA1(1c96453c2ae372892c39b5657cf2b252a90a10a9) )
ROM_END

ROM_START( brdrlinb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "border1.33",   0x0000, 0x0800, CRC(48387706) SHA1(b4db2f05e722812370b0b24cd15061d6fc578560) ) // karateco
	ROM_LOAD( "border2.30",   0x0800, 0x0800, CRC(1d669b60) SHA1(47ef5141591e177419fa352968be26ecc6fafd89) )
	ROM_LOAD( "border3.29",   0x1000, 0x0800, CRC(6e4d6fb3) SHA1(4747359c4f09e93c563b93ee1189743894332b47) )
	ROM_LOAD( "border4.27",   0x1800, 0x0800, CRC(718446d8) SHA1(cc15cb37ebb51970fcdebf74ab4308a25b40af2a) )
	ROM_LOAD( "border5.08",   0x2000, 0x0800, CRC(a0584337) SHA1(1fd9c60dc42a178c88d4e4e4b4d5de495ea906c6) )
	ROM_LOAD( "border6.06",   0x2800, 0x0800, CRC(cb30fb98) SHA1(96ea83111e8ccf409fe40f19ce200a50ea9ea288) )
	ROM_LOAD( "border7.04",   0x3000, 0x0800, CRC(200c5321) SHA1(f9faa6125dd2adc69c4ba9d962c0998f815ccd1c) )
	ROM_LOAD( "border8.02",   0x3800, 0x0800, CRC(735e140d) SHA1(1c0b6cf2d8c88601084dfcb8d7490b85ef1a86d5) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "borderc.49",   0x0000, 0x0020, CRC(bc6be94e) SHA1(34e113ec25e19212b74907d35be5cb8714a8249c) )

	ROM_REGION( 0x0800, "cpu1", 0 )	/* sound ROM */
	ROM_LOAD( "bords.bin",    0x0000, 0x0400, CRC(a23e1d9f) SHA1(ce209571f6341aa6f036a015e666673098bc98ea) )

	ROM_REGION( 0x0020, "user1", 0 )	/* misc PROM */
	ROM_LOAD( "border.32",   0x0000, 0x0020, CRC(c128d0ba) SHA1(0ce9febbb7e2f5388ed999a479e3d385dba0b342) )
	ROM_LOAD( "bordera.15",  0x0000, 0x0020, CRC(6449e678) SHA1(421c45c8fba3c2bc2a7ebbea2c837c8fa1a5a2f3) )
	ROM_LOAD( "borderb.14",  0x0000, 0x0020, CRC(55dcdef1) SHA1(6fbd041edc258b7e1b99bbe9526612cfb1b541f8) )
ROM_END

ROM_START( digger )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "684.u27",      0x0000, 0x0400, CRC(bba0d7c2) SHA1(1e55dd95b07b562dcc1e52ecf9460d302b14ee60) )
	ROM_LOAD( "685.u26",      0x0400, 0x0400, CRC(85210d8b) SHA1(8260ca809a3a20a52b146d357253aa958d08887e) )
	ROM_LOAD( "686.u25",      0x0800, 0x0400, CRC(2d87238c) SHA1(de8dbe56c4c71b5d75e77c39cfbd771b91c0db0f) )
	ROM_LOAD( "687.u24",      0x0c00, 0x0400, CRC(0dd0604e) SHA1(d357e195a6e61615b0e8cfb027628c2ce5f2b1c5) )
	ROM_LOAD( "688.u23",      0x1000, 0x0400, CRC(2f649667) SHA1(f4c08c7c8f1e9cee84bf844810f006d27bd35025) )
	ROM_LOAD( "689.u22",      0x1400, 0x0400, CRC(89fd63d9) SHA1(413f5df7eedb67848119c675e4edd1ce211ded24) )
	ROM_LOAD( "690.u21",      0x1800, 0x0400, CRC(a86622a6) SHA1(44c0712cc1e11255dc0e27f4754984885c9fa8ad) )
	ROM_LOAD( "691.u20",      0x1c00, 0x0400, CRC(8aca72d8) SHA1(59306dbc350dc64a17ad2f9909ef00e9860c56e7) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "316-507",      0x0000, 0x0020, CRC(fdb22e8f) SHA1(b09241b532cfe7679e837e9f3e5956cfc588a0be) )

	ROM_REGION( 0x0020, "user1", 0 )	/* timing PROM */
	ROM_LOAD( "316-0206.u14", 0x0000, 0x0020, CRC(9617d796) SHA1(7cff2741866095ff42eadd8022bea349ec8d2f39) )	/* control PROM */
ROM_END

ROM_START( pulsar )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "790.u33",      0x0000, 0x0400, CRC(5e3816da) SHA1(83f019fa3598e383310b4c21441e4f8ef0c9d4fb) )
	ROM_LOAD( "791.u32",      0x0400, 0x0400, CRC(ce0aee83) SHA1(f3755592a9aaa2d493d017c8da19354fd5598860) )
	ROM_LOAD( "792.u31",      0x0800, 0x0400, CRC(72d78cf1) SHA1(d292d80826081073d279d163a107926fa80f02d0) )
	ROM_LOAD( "793.u30",      0x0c00, 0x0400, CRC(42155dd4) SHA1(d3032b0509eb3aa9c49b92ae69971ec019310b16) )
	ROM_LOAD( "794.u29",      0x1000, 0x0400, CRC(11c7213a) SHA1(417a608913e84c456f3181e94e496b05599dfb14) )
	ROM_LOAD( "795.u28",      0x1400, 0x0400, CRC(d2f02e29) SHA1(2b4333ecb89abd80634686b9ef667213423bc4a8) )
	ROM_LOAD( "796.u27",      0x1800, 0x0400, CRC(67737a2e) SHA1(6e8616bb0fba603fec9d97a1852986390db67b0a) )
	ROM_LOAD( "797.u26",      0x1c00, 0x0400, CRC(ec250b24) SHA1(b8bbdc888008d072672024fced55fcf3da668532) )
	ROM_LOAD( "798.u8",       0x2000, 0x0400, CRC(1d34912d) SHA1(32b10bc9617fa14b9e98bd572c86bba495f64d18) )
	ROM_LOAD( "799.u7",       0x2400, 0x0400, CRC(f5695e4c) SHA1(77251f917802d01d76a9a6cf8bb6c12919803388) )
	ROM_LOAD( "800.u6",       0x2800, 0x0400, CRC(bf91ad92) SHA1(ebadcb9a6d2ee5e4b432f132095fac8d72e05a34) )
	ROM_LOAD( "801.u5",       0x2c00, 0x0400, CRC(1e9721dc) SHA1(ddb94275c8fd26f7323b89d234b34883737f8738) )
	ROM_LOAD( "802.u4",       0x3000, 0x0400, CRC(d32d2192) SHA1(334c10dd418fe320fc9a689c798e90787aaae7bf) )
	ROM_LOAD( "803.u3",       0x3400, 0x0400, CRC(3ede44d5) SHA1(3967fc6dc25b5872ae1ad389a31257b39879e13d) )
	ROM_LOAD( "804.u2",       0x3800, 0x0400, CRC(62847b01) SHA1(79763623df6eb7d189a0b0583ecc12c88602a44d) )
	ROM_LOAD( "805.u1",       0x3c00, 0x0400, CRC(ab418e86) SHA1(ad0dfd982b3bd3943aaf670d48485218f1cc4998) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "316-0789.u49", 0x0000, 0x0020, CRC(7fc1861f) SHA1(e005a8afd6b9e6b7d4ddf362c204e472b80582c6) )

	ROM_REGION( 0x0020, "user1", 0 )	/* misc PROM */
	ROM_LOAD( "316-0206.u14", 0x0000, 0x0020, CRC(9617d796) SHA1(7cff2741866095ff42eadd8022bea349ec8d2f39) )	/* control PROM */
ROM_END

ROM_START( heiankyo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ha16.u33",     0x0000, 0x0400, CRC(1eec8b36) SHA1(55644cfeb7a9d64e52f11611c91c6186038772a3) )
	ROM_LOAD( "ha15.u32",     0x0400, 0x0400, CRC(c1b9a1a5) SHA1(068ad2da4852a50c948c4f9b3e1b1aa5c5bf5ca5) )
	ROM_LOAD( "ha14.u31",     0x0800, 0x0400, CRC(5b7b582e) SHA1(078b8b7d1836cc31cee244a58fb6a6a50135f833) )
	ROM_LOAD( "ha13.u30",     0x0c00, 0x0400, CRC(4aa67e01) SHA1(5539a028cb1935bb4d6ab747c92792f5462add1f) )
	ROM_LOAD( "ha12.u29",     0x1000, 0x0400, CRC(75889ca6) SHA1(552bcf976f31d7b634b79175c0470978b6b82433) )
	ROM_LOAD( "ha11.u28",     0x1400, 0x0400, CRC(d469226a) SHA1(dfca01d956e12162fab261a017c727a756b67206) )
	ROM_LOAD( "ha10.u27",     0x1800, 0x0400, CRC(4e203074) SHA1(1a80c396ceb9a2b1c737e1af791dbab2bee10ce5) )
	ROM_LOAD( "ha9.u26",      0x1c00, 0x0400, CRC(9c3a3dd2) SHA1(afcd85ec0174bdcab31135b4e271cec1eb75fd02) )
	ROM_LOAD( "ha8.u8",       0x2000, 0x0400, CRC(6cc64878) SHA1(4d03ff925d80835c27512b3bd04ea57f91b4491f) )
	ROM_LOAD( "ha7.u7",       0x2400, 0x0400, CRC(6d2f9527) SHA1(4e51c5404d0302547c1ae85b27ffe4de11d68224) )
	ROM_LOAD( "ha6.u6",       0x2800, 0x0400, CRC(e467c353) SHA1(a76b4f6d9702f760f287b5285f76ea4206c6934a) )
	ROM_LOAD( "ha3.u3",       0x2c00, 0x0400, CRC(6a55eda8) SHA1(f526ebf18a33271b798e53cfcadb27e4c3a03466) )
	/* 3000-37ff empty */
	ROM_LOAD( "ha2.u2",       0x3800, 0x0400, CRC(056b3b8b) SHA1(3cce6c928599604ffdcdb767caa7b32d8ec1e03d) )
	ROM_LOAD( "ha1.u1",       0x3c00, 0x0400, CRC(b8da2b5e) SHA1(70d97b89cb3162bd479203c53148319179a9873f) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "316-138.u49",  0x0000, 0x0020, CRC(67104ea9) SHA1(26b6bd2a1973b83bb9af4e3385d8cb14cb3f62f2) )

	ROM_REGION( 0x0040, "user1", 0 )	/* misc PROMs */
	ROM_LOAD( "316-0043.u87", 0x0000, 0x0020, CRC(e60a7960) SHA1(b8b8716e859c57c35310efc4594262afedb84823) )	/* control PROM */
	ROM_LOAD( "316-0042.u88", 0x0020, 0x0020, CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) )	/* sequence PROM */
ROM_END

ROM_START( alphaho )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "c0.bin",       0x0000, 0x0400, CRC(db774c23) SHA1(c5042872110ae8d0c5c7629892a16b87e8f19d96) )
	ROM_LOAD( "c1.bin",       0x0400, 0x0400, CRC(b63f4695) SHA1(95b3ca96ca48f2c525eaf2b49956248e46686688) )
	ROM_LOAD( "c2.bin",       0x0800, 0x0400, CRC(4ebf0ba4) SHA1(b6651f7424fd5e62422b45ccce118db64dab50bf) )
	ROM_LOAD( "c3.bin",       0x0c00, 0x0400, CRC(126f17ec) SHA1(a78b9f62d76305903ad56ebf995ba0745774e6f2) )
	ROM_LOAD( "c4.bin",       0x1000, 0x0400, CRC(52798c61) SHA1(c01c11b99f56ca7f5d8824a3b9f795b2075d21a0) )
	ROM_LOAD( "c5.bin",       0x1400, 0x0400, CRC(4827cb36) SHA1(a5f56aa73147503fb6c5a3f38489754d8d27f257) )
	ROM_LOAD( "c6.bin",       0x1800, 0x0400, CRC(8b2ff47e) SHA1(43c84fe40180c42ade3c77790aeadeeb28e5c5cd) )
	ROM_LOAD( "c7.bin",       0x1c00, 0x0400, CRC(44921df4) SHA1(3ece06f20330aebe2770bf74142cd5e7bc5297ec) )
	ROM_LOAD( "c8.bin",       0x2000, 0x0400, CRC(9fb12fca) SHA1(a5b49ddd86be44b9220cc4ceb84b32c55e5d432b) )
	ROM_LOAD( "c9.bin",       0x2400, 0x0400, CRC(e5f622f7) SHA1(57858b6abbf34fc4ab2b19a469cbd945a0e14a0e) )
	ROM_LOAD( "ca.bin",       0x2800, 0x0400, CRC(82b28e77) SHA1(c4f425daa02acbc19d4ecee8bd38d8bb338e085f) )
	ROM_LOAD( "cb.bin",       0x2c00, 0x0400, CRC(94fba0ad) SHA1(c623368c710ddf8ff05e09c963eeb863c9951df3) )
	ROM_LOAD( "cc.bin",       0x3000, 0x0400, CRC(de338b6d) SHA1(a465aaac041e4aa017afb9821dd049ac4950ba92) )
	ROM_LOAD( "cd.bin",       0x3400, 0x0400, CRC(be76baac) SHA1(4b6a46c9484cfc90fa405f8568df44cfc96b1d7a) )
	ROM_LOAD( "ce.bin",       0x3800, 0x0400, CRC(3c409d57) SHA1(fda00b4eaaa187f489a8957952005e08b2b7ba40) )
	ROM_LOAD( "cf.bin",       0x3c00, 0x0400, CRC(d03c5a09) SHA1(eebc1d2302bd2bae28c25187bb099ae618c5cd05) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "alphaho.col",  0x0000, 0x0020, NO_DUMP )
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAMEL(1977, depthch,  0,        depthch,  depthch,  0, ROT0,   "Gremlin", "Depthcharge", GAME_IMPERFECT_SOUND, layout_depthch )
GAMEL(1977, depthcho, depthch,  depthch,  depthch,  0, ROT0,   "Gremlin", "Depthcharge (older)", GAME_IMPERFECT_SOUND, layout_depthch )
GAME( 1977, subhunt,  depthch,  depthch,  depthch,  0, ROT0,   "Gremlin (Taito license)", "Sub Hunter", GAME_IMPERFECT_SOUND )
GAME( 1977, safari,   0,        safari,   safari,   0, ROT0,   "Gremlin", "Safari (set 1)", GAME_NO_SOUND )
GAME( 1977, safaria,  safari,   safari,   safari,   0, ROT0,   "Gremlin", "Safari (set 2, bootleg?)", GAME_NO_SOUND ) // on a bootleg board, but seems a different code revision too
GAME( 1978, frogs,    0,        frogs,    frogs,    0, ROT0,   "Gremlin", "Frogs", GAME_IMPERFECT_SOUND )
GAME( 1979, sspaceat, 0,        sspaceat, sspaceat, 0, ROT270, "Sega", "Space Attack (upright set 1)", GAME_NO_SOUND )
GAME( 1979, sspaceat2,sspaceat, sspaceat, sspaceat, 0, ROT270, "Sega", "Space Attack (upright set 2)", GAME_NO_SOUND )
GAME( 1979, sspaceat3,sspaceat, sspaceat, sspaceat, 0, ROT270, "Sega", "Space Attack (upright set 3)", GAME_NO_SOUND )
GAME( 1979, sspaceatc,sspaceat, sspaceat, sspaceat, 0, ROT270, "Sega", "Space Attack (cocktail)", GAME_NO_SOUND )
GAME( 1979, sspacaho, 0,        sspacaho, sspacaho, 0, ROT270, "Sega", "Space Attack / Head On", GAME_NO_SOUND )
GAME( 1979, headon,   0,        headon,   headon,   0, ROT0,   "Gremlin", "Head On (2 players)",  GAME_IMPERFECT_SOUND )
GAME( 1979, headonb,  headon,   headon,   headon,   0, ROT0,   "Gremlin", "Head On (1 player)",  GAME_IMPERFECT_SOUND )
GAME( 1979, headons,  headon,   headons,  headon,   0, ROT0,   "bootleg (Sidam)", "Head On (Sidam bootleg, set 1)",  GAME_IMPERFECT_SOUND )
GAME( 1979, headonsa, headon,   headons,  headon,   0, ROT0,   "bootleg (Sidam)", "Head On (Sidam bootleg, set 2)",  GAME_NOT_WORKING ) // won't coin up?
GAME( 1979, supcrash, headon,   headons,  supcrash, 0, ROT0,   "bootleg", "Super Crash (bootleg of Head On)", GAME_NO_SOUND )
GAME( 1979, headon2,  0,        headon2,  headon2,  0, ROT0,   "Sega", "Head On 2",  GAME_IMPERFECT_SOUND )
GAME( 1979, headon2s, headon2,  headon2bw,car2,     0, ROT0,   "bootleg (Sidam)", "Head On 2 (Sidam bootleg)",  GAME_NOT_WORKING ) // won't coin up?
GAME( 1979, car2,     headon2,  headon2bw,car2,     0, ROT0,   "bootleg (RZ Bologna)", "Car 2 (bootleg of Head On 2)",  GAME_IMPERFECT_SOUND ) // title still says 'HeadOn 2'
GAME( 1979, invho2,   0,        invho2,   invho2,   0, ROT270, "Sega", "Invinco / Head On 2", GAME_IMPERFECT_SOUND )
GAME( 1980, nsub,     0,        nsub,     nsub,     0, ROT270, "Sega", "N-Sub (upright)", GAME_IMPERFECT_GRAPHICS | GAME_NO_SOUND )
GAME( 1980, samurai,  0,        samurai,  samurai,  0, ROT270, "Sega", "Samurai", GAME_NO_SOUND )
GAME( 1979, invinco,  0,        invinco,  invinco,  0, ROT270, "Sega", "Invinco", GAME_IMPERFECT_SOUND )
GAME( 1979, invds,    0,        invds,    invds,    0, ROT270, "Sega", "Invinco / Deep Scan", GAME_IMPERFECT_SOUND )
GAME( 1980, tranqgun, 0,        tranqgun, tranqgun, 0, ROT270, "Sega", "Tranquillizer Gun", GAME_NO_SOUND )
GAME( 1980, spacetrk, 0,        spacetrk, spacetrk, 0, ROT270, "Sega", "Space Trek (upright)", GAME_NO_SOUND )
GAME( 1980, spacetrkc,spacetrk, spacetrk, sptrekct, 0, ROT270, "Sega", "Space Trek (cocktail)", GAME_NO_SOUND )
GAME( 1980, carnival, 0,        carnival, carnival, 0, ROT270, "Sega", "Carnival (upright)", GAME_IMPERFECT_SOUND )
GAME( 1980, carnivalc,carnival, carnival, carnvckt, 0, ROT270, "Sega", "Carnival (cocktail)",  GAME_IMPERFECT_SOUND )
GAME( 1980, carnivalh,carnival, carnivalh,carnivalh,0, ROT270, "Sega", "Carnival (Head On hardware, set 1)",  GAME_IMPERFECT_SOUND )
GAME( 1980, carnivalha,carnival,carnivalh,carnivalh,0, ROT270, "Sega", "Carnival (Head On hardware, set 2)",  GAME_IMPERFECT_SOUND )
GAME( 1981, brdrline, 0,        brdrline, brdrline, 0, ROT270, "Sega", "Borderline", GAME_NO_SOUND )
GAME( 1981, brdrlins, brdrline, brdrline, brdrline, 0, ROT270, "bootleg (Sidam)", "Borderline (Sidam bootleg)", GAME_NO_SOUND )
GAME( 1981, brdrlinb, brdrline, brdrline, brdrline, 0, ROT270, "bootleg (Karateco)", "Borderline (Karateco bootleg)", GAME_NO_SOUND )
GAME( 1980, digger,   0,        digger,   digger,   0, ROT270, "Sega", "Digger", GAME_NO_SOUND )
GAME( 1981, pulsar,   0,        pulsar,   pulsar,   0, ROT270, "Sega", "Pulsar", GAME_IMPERFECT_SOUND )
GAME( 1979, heiankyo, 0,        heiankyo, heiankyo, 0, ROT270, "Denki Onkyo", "Heiankyo Alien", GAME_NO_SOUND )
GAME( 19??, alphaho,  0,        alphaho,  alphaho,  0, ROT270, "Data East Corporation", "Alpha Fighter / Head On", GAME_WRONG_COLORS | GAME_NO_SOUND )
