/********************************************************************

Enigma 2 (c) Zilec Electronics

driver by Pierpaolo Prazzoli and Tomasz Slanina


enigma2 (1981)
 2xZ80 + AY8910
 Original dedicated board

enigma2a (1984?)
 Conversion applied to a Taito Space Invaders Part II board set with 1984 copyrighy. hack / Bootleg ?

enigma2b (1981)
 Conversion like enigma2a, but boots with 1981 copyright and Phantoms II title

TODO:
 - enigma2  - Star blinking frequency
 - enigma2a + enigma2b - bad sound ROM?

*********************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/i8085/i8085.h"
#include "deprecat.h"
#include "sound/ay8910.h"

#define LOG_PROT	(0)

/* these values provide a fairly low refresh rate of around 53Hz, but
   they were derived from the schemtics.  The horizontal synch chain
   counts from 0x0c0-0x1ff and the vertical one from 0x0d8-0x1ff.  */

#define MASTER_CLOCK		(10000000)
#define CPU_CLOCK			(MASTER_CLOCK / 4)
#define PIXEL_CLOCK			(MASTER_CLOCK / 2)
#define AY8910_CLOCK		(MASTER_CLOCK / 8)
#define HCOUNTER_START		(0x0c0)
#define HCOUNTER_END		(0x1ff)
#define HTOTAL				(HCOUNTER_END + 1 - HCOUNTER_START)
#define HBEND				(0x000)
#define HBSTART				(0x100)
#define VCOUNTER_START		(0x0d8)
#define VCOUNTER_END		(0x1ff)
#define VTOTAL				(VCOUNTER_END + 1 - VCOUNTER_START)
#define VBEND				(0x048)
#define VBSTART				(VTOTAL)

/* the IRQ line is cleared (active LO) at these vertical sync counter
   values and raised one scan line later */
#define INT_TRIGGER_COUNT_1	(0x10f)
#define INT_TRIGGER_COUNT_2	(0x18f)


#define NUM_PENS	(8)


class enigma2_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, enigma2_state(machine)); }

	enigma2_state(running_machine &machine) { }

	/* memory pointers */
	UINT8 *  videoram;

	/* misc */
	int blink_count;
	UINT8 sound_latch;
	UINT8 last_sound_data;
	UINT8 protection_data;
	UINT8 flip_screen;

	emu_timer *interrupt_clear_timer;
	emu_timer *interrupt_assert_timer;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
};


/*************************************
 *
 *  Interrupt generation
 *
 *************************************/


INLINE UINT16 vpos_to_vysnc_chain_counter( int vpos )
{
	return vpos + VCOUNTER_START;
}


INLINE int vysnc_chain_counter_to_vpos( UINT16 counter )
{
	return counter - VCOUNTER_START;
}


static TIMER_CALLBACK( interrupt_clear_callback )
{
	enigma2_state *state = (enigma2_state *)machine->driver_data;
	cpu_set_input_line(state->maincpu, 0, CLEAR_LINE);
}


static TIMER_CALLBACK( interrupt_assert_callback )
{
	enigma2_state *state = (enigma2_state *)machine->driver_data;
	UINT16 next_counter;
	int next_vpos;

	/* compute vector and set the interrupt line */
	int vpos = machine->primary_screen->vpos();
	UINT16 counter = vpos_to_vysnc_chain_counter(vpos);
	UINT8 vector = 0xc7 | ((counter & 0x80) >> 3) | ((~counter & 0x80) >> 4);
	cpu_set_input_line_and_vector(state->maincpu, 0, ASSERT_LINE, vector);

	/* set up for next interrupt */
	if (counter == INT_TRIGGER_COUNT_1)
		next_counter = INT_TRIGGER_COUNT_2;
	else
		next_counter = INT_TRIGGER_COUNT_1;

	next_vpos = vysnc_chain_counter_to_vpos(next_counter);
	timer_adjust_oneshot(state->interrupt_assert_timer, machine->primary_screen->time_until_pos(next_vpos), 0);
	timer_adjust_oneshot(state->interrupt_clear_timer, machine->primary_screen->time_until_pos(vpos + 1), 0);
}


static void create_interrupt_timers( running_machine *machine )
{
	enigma2_state *state = (enigma2_state *)machine->driver_data;
	state->interrupt_clear_timer = timer_alloc(machine, interrupt_clear_callback, NULL);
	state->interrupt_assert_timer = timer_alloc(machine, interrupt_assert_callback, NULL);
}


static void start_interrupt_timers( running_machine *machine )
{
	enigma2_state *state = (enigma2_state *)machine->driver_data;
	int vpos = vysnc_chain_counter_to_vpos(INT_TRIGGER_COUNT_1);
	timer_adjust_oneshot(state->interrupt_assert_timer, machine->primary_screen->time_until_pos(vpos), 0);
}



static MACHINE_START( enigma2 )
{
	enigma2_state *state = (enigma2_state *)machine->driver_data;
	create_interrupt_timers(machine);

	state->maincpu = machine->device("maincpu");
	state->audiocpu = machine->device("audiocpu");

	state_save_register_global(machine, state->blink_count);
	state_save_register_global(machine, state->sound_latch);
	state_save_register_global(machine, state->last_sound_data);
	state_save_register_global(machine, state->protection_data);
	state_save_register_global(machine, state->flip_screen);
}


static MACHINE_RESET( enigma2 )
{
	enigma2_state *state = (enigma2_state *)machine->driver_data;
	cputag_set_input_line(machine, "audiocpu", INPUT_LINE_NMI, CLEAR_LINE);

	state->last_sound_data = 0;
	state->protection_data = 0;
	state->flip_screen = 0;
	state->sound_latch = 0;
	state->blink_count = 0;

	start_interrupt_timers(machine);
}


/*************************************
 *
 *  Video system
 *
 *************************************/

static void get_pens(pen_t *pens)
{
	offs_t i;

	for (i = 0; i < NUM_PENS; i++)
	{
		/* this color gun arrengement is supported by the flyer screenshot */
		pens[i] = MAKE_RGB(pal1bit(i >> 2), pal1bit(i >> 1), pal1bit(i >> 0));
	}
}


static VIDEO_UPDATE( enigma2 )
{
	enigma2_state *state = (enigma2_state *)screen->machine->driver_data;
	pen_t pens[NUM_PENS];

	const rectangle &visarea = screen->visible_area();
	UINT8 *prom = memory_region(screen->machine, "proms");
	UINT8 *color_map_base = state->flip_screen ? &prom[0x0400] : &prom[0x0000];
	UINT8 *star_map_base = (state->blink_count & 0x08) ? &prom[0x0c00] : &prom[0x0800];

	UINT8 x = 0;
	UINT16 bitmap_y = visarea.min_y;
	UINT8 y = (UINT8)vpos_to_vysnc_chain_counter(bitmap_y);
	UINT8 video_data = 0;
	UINT8 fore_color = 0;
	UINT8 star_color = 0;

	get_pens(pens);

	while (1)
	{
		UINT8 bit;
		UINT8 color;

		/* read the video RAM */
		if ((x & 0x07) == 0x00)
		{
			offs_t color_map_address = (y >> 3 << 5) | (x >> 3);
			/* the schematics shows it like this, but it doesn't work as this would
               produce no stars, due to the contents of the PROM -- maybe there is
               a star disabled bit somewhere that's connected here instead of flip_screen_get(screen->machine) */
			/* star_map_address = (y >> 4 << 6) | (engima2_flip_screen_get() << 5) | (x >> 3); */
			offs_t star_map_address = (y >> 4 << 6) | 0x20 | (x >> 3);

			offs_t videoram_address = (y << 5) | (x >> 3);

			/* when the screen is flipped, all the video address bits are inverted,
               and the adder at 16A is activated */
			if (state->flip_screen)  videoram_address = (~videoram_address + 0x0400) & 0x1fff;

			video_data = state->videoram[videoram_address];

			fore_color = color_map_base[color_map_address] & 0x07;
			star_color = star_map_base[star_map_address] & 0x07;
		}

		/* plot the current pixel */
		if (state->flip_screen)
		{
			bit = video_data & 0x80;
			video_data = video_data << 1;
		}
		else
		{
			bit = video_data & 0x01;
			video_data = video_data >> 1;
		}

		if (bit)
			color = fore_color;
		else
			/* stars only appear at certain positions */
			color = ((x & y & 0x0f) == 0x0f) ? star_color : 0;

		*BITMAP_ADDR32(bitmap, bitmap_y, x) = pens[color];

		/* next pixel */
		x = x + 1;

		/* end of line? */
		if (x == 0)
		{
			/* end of screen? */
			if (bitmap_y == visarea.max_y)
				break;

			/* next row */
			y = y + 1;
			bitmap_y = bitmap_y + 1;
		}
	}

	state->blink_count++;

	return 0;
}


static VIDEO_UPDATE( enigma2a )
{
	enigma2_state *state = (enigma2_state *)screen->machine->driver_data;
	UINT8 x = 0;
	const rectangle &visarea = screen->visible_area();
	UINT16 bitmap_y = visarea.min_y;
	UINT8 y = (UINT8)vpos_to_vysnc_chain_counter(bitmap_y);
	UINT8 video_data = 0;

	while (1)
	{
		UINT8 bit;
		pen_t pen;

		/* read the video RAM */
		if ((x & 0x07) == 0x00)
		{
			offs_t videoram_address = (y << 5) | (x >> 3);

			/* when the screen is flipped, all the video address bits are inverted,
               and the adder at 16A is activated */
			if (state->flip_screen)  videoram_address = (~videoram_address + 0x0400) & 0x1fff;

			video_data = state->videoram[videoram_address];
		}

		/* plot the current pixel */
		if (state->flip_screen)
		{
			bit = video_data & 0x80;
			video_data = video_data << 1;
		}
		else
		{
			bit = video_data & 0x01;
			video_data = video_data >> 1;
		}

		pen = bit ? RGB_WHITE : RGB_BLACK;
		*BITMAP_ADDR32(bitmap, bitmap_y, x) = pen;

		/* next pixel */
		x = x + 1;

		/* end of line? */
		if (x == 0)
		{
			/* end of screen? */
			if (bitmap_y == visarea.max_y)
				break;

			/* next row */
			y = y + 1;
			bitmap_y = bitmap_y + 1;
		}
	}

	return 0;
}



static READ8_HANDLER( dip_switch_r )
{
	enigma2_state *state = (enigma2_state *)space->machine->driver_data;
	UINT8 ret = 0x00;

	if (LOG_PROT) logerror("DIP SW Read: %x at %x (prot data %x)\n", offset, cpu_get_pc(space->cpu), state->protection_data);
	switch (offset)
	{
	case 0x01:
		if (state->protection_data != 0xff)
			ret = state->protection_data ^ 0x88;
		else
			ret = input_port_read(space->machine, "DSW");
		break;

	case 0x02:
		if (cpu_get_pc(space->cpu) == 0x07e5)
			ret = 0xaa;
		else
			ret = 0xf4;
		break;

	case 0x35:	ret = 0x38; break;
	case 0x51:	ret = 0xaa; break;
	case 0x79:	ret = 0x38; break;
	}

	return ret;
}


static WRITE8_HANDLER( sound_data_w )
{
	enigma2_state *state = (enigma2_state *)space->machine->driver_data;
	/* clock sound latch shift register on rising edge of D2 */
	if (!(data & 0x04) && (state->last_sound_data & 0x04))
		state->sound_latch = (state->sound_latch << 1) | (~data & 0x01);

	cpu_set_input_line(state->audiocpu, INPUT_LINE_NMI, (data & 0x02) ? ASSERT_LINE : CLEAR_LINE);

	state->last_sound_data = data;
}


static READ8_DEVICE_HANDLER( sound_latch_r )
{
	enigma2_state *state = (enigma2_state *)device->machine->driver_data;
	return BITSWAP8(state->sound_latch,0,1,2,3,4,5,6,7);
}


static WRITE8_DEVICE_HANDLER( protection_data_w )
{
	enigma2_state *state = (enigma2_state *)device->machine->driver_data;
	if (LOG_PROT) logerror("%s: Protection Data Write: %x\n", cpuexec_describe_context(device->machine), data);
	state->protection_data = data;
}


static WRITE8_HANDLER( enigma2_flip_screen_w )
{
	enigma2_state *state = (enigma2_state *)space->machine->driver_data;
	state->flip_screen = ((data >> 5) & 0x01) && ((input_port_read(space->machine, "DSW") & 0x20) == 0x20);
}


static CUSTOM_INPUT( p1_controls_r )
{
	return input_port_read(field->port->machine, "P1CONTROLS");
}


static CUSTOM_INPUT( p2_controls_r )
{
	enigma2_state *state = (enigma2_state *)field->port->machine->driver_data;
	if (state->flip_screen)
		return input_port_read(field->port->machine, "P2CONTROLS");
	else
		return input_port_read(field->port->machine, "P1CONTROLS");
}



static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_HANDLER(sound_latch_r),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_HANDLER(protection_data_w)
};



static ADDRESS_MAP_START( engima2_main_cpu_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x1fff) AM_ROM AM_WRITENOP
	AM_RANGE(0x2000, 0x3fff) AM_MIRROR(0x4000) AM_RAM AM_BASE_MEMBER(enigma2_state, videoram)
	AM_RANGE(0x4000, 0x4fff) AM_ROM AM_WRITENOP
	AM_RANGE(0x5000, 0x57ff) AM_READ(dip_switch_r) AM_WRITENOP
	AM_RANGE(0x5800, 0x5800) AM_MIRROR(0x07f8) AM_NOP
	AM_RANGE(0x5801, 0x5801) AM_MIRROR(0x07f8) AM_READ_PORT("IN0") AM_WRITENOP
	AM_RANGE(0x5802, 0x5802) AM_MIRROR(0x07f8) AM_READ_PORT("IN1") AM_WRITENOP
	AM_RANGE(0x5803, 0x5803) AM_MIRROR(0x07f8) AM_READNOP AM_WRITE(sound_data_w)
	AM_RANGE(0x5804, 0x5804) AM_MIRROR(0x07f8) AM_NOP
	AM_RANGE(0x5805, 0x5805) AM_MIRROR(0x07f8) AM_READNOP AM_WRITE(enigma2_flip_screen_w)
	AM_RANGE(0x5806, 0x5807) AM_MIRROR(0x07f8) AM_NOP
ADDRESS_MAP_END


static ADDRESS_MAP_START( engima2a_main_cpu_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_ROM AM_WRITENOP
	AM_RANGE(0x2000, 0x3fff) AM_MIRROR(0x4000) AM_RAM AM_BASE_MEMBER(enigma2_state, videoram)
	AM_RANGE(0x4000, 0x4fff) AM_ROM AM_WRITENOP
	AM_RANGE(0x5000, 0x57ff) AM_READ(dip_switch_r) AM_WRITENOP
	AM_RANGE(0x5800, 0x5fff) AM_NOP
ADDRESS_MAP_END


static ADDRESS_MAP_START( engima2a_main_cpu_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0x7)
	AM_RANGE(0x00, 0x00) AM_NOP
	AM_RANGE(0x01, 0x01) AM_READ_PORT("IN0") AM_WRITENOP
	AM_RANGE(0x02, 0x02) AM_READ_PORT("IN1") AM_WRITENOP
	AM_RANGE(0x03, 0x03) AM_READNOP AM_WRITE(sound_data_w)
	AM_RANGE(0x04, 0x04) AM_NOP
	AM_RANGE(0x05, 0x05) AM_READNOP AM_WRITE(enigma2_flip_screen_w)
	AM_RANGE(0x06, 0x07) AM_NOP
ADDRESS_MAP_END


static ADDRESS_MAP_START( engima2_audio_cpu_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_MIRROR(0x1000) AM_ROM AM_WRITENOP
	AM_RANGE(0x2000, 0x7fff) AM_NOP
	AM_RANGE(0x8000, 0x83ff) AM_MIRROR(0x1c00) AM_RAM
	AM_RANGE(0xa000, 0xa001) AM_MIRROR(0x1ffc) AM_DEVWRITE("aysnd", ay8910_address_data_w)
	AM_RANGE(0xa002, 0xa002) AM_MIRROR(0x1ffc) AM_DEVREAD("aysnd", ay8910_r)
	AM_RANGE(0xa003, 0xa003) AM_MIRROR(0x1ffc) AM_NOP
	AM_RANGE(0xc000, 0xffff) AM_NOP
ADDRESS_MAP_END



static INPUT_PORTS_START( enigma2 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x78, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(p1_controls_r, NULL)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x78, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(p2_controls_r, NULL)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x1c, 0x00, "Skill level" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x14, "6" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x40, 0x40, "Number of invaders" )
	PORT_DIPSETTING(    0x40, "16" )
	PORT_DIPSETTING(    0x00, "32" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )

	PORT_START("P1CONTROLS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2CONTROLS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( enigma2a )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x70, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(p1_controls_r, NULL)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x70, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(p2_controls_r, NULL)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x1c, 0x00, "Skill level" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x14, "6" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )

	PORT_START("P1CONTROLS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0xf8, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2CONTROLS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0xf8, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static MACHINE_DRIVER_START( enigma2 )

	/* driver data */
	MDRV_DRIVER_DATA(enigma2_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, CPU_CLOCK)
	MDRV_CPU_PROGRAM_MAP(engima2_main_cpu_map)

	MDRV_CPU_ADD("audiocpu", Z80, 2500000)
	MDRV_CPU_VBLANK_INT_HACK(irq0_line_hold,8)
	MDRV_CPU_PROGRAM_MAP(engima2_audio_cpu_map)

	MDRV_MACHINE_START(enigma2)
	MDRV_MACHINE_RESET(enigma2)

	/* video hardware */
	MDRV_VIDEO_UPDATE(enigma2)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_RAW_PARAMS(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART)

	/* audio hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("aysnd", AY8910, AY8910_CLOCK)
	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( enigma2a )

	/* driver data */
	MDRV_DRIVER_DATA(enigma2_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", I8080, CPU_CLOCK)
	MDRV_CPU_PROGRAM_MAP(engima2a_main_cpu_map)
	MDRV_CPU_IO_MAP(engima2a_main_cpu_io_map)

	MDRV_CPU_ADD("audiocpu", Z80, 2500000)
	MDRV_CPU_VBLANK_INT_HACK(irq0_line_hold,8)
	MDRV_CPU_PROGRAM_MAP(engima2_audio_cpu_map)

	MDRV_MACHINE_START(enigma2)
	MDRV_MACHINE_RESET(enigma2)

	/* video hardware */
	MDRV_VIDEO_UPDATE(enigma2a)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_RAW_PARAMS(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART)

	/* audio hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("aysnd", AY8910, AY8910_CLOCK)
	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END



ROM_START( enigma2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.5d",         0x0000, 0x0800, CRC(499749de) SHA1(401928ff41d3b4cbb68e6ad3bf3be4a10ae1781f) )
	ROM_LOAD( "2.7d",         0x0800, 0x0800, CRC(173c1329) SHA1(3f1ad46d0e58ab236e4ff2b385d09fbf113627da) )
	ROM_LOAD( "3.8d",         0x1000, 0x0800, CRC(c7d3e6b1) SHA1(43f7c3a02b46747998260d5469248f21714fe12b) )
	ROM_LOAD( "4.10d",        0x1800, 0x0800, CRC(c6a7428c) SHA1(3503f09856655c5973fb89f60d1045fe41012aa9) )
	ROM_LOAD( "5.11d",  	  0x4000, 0x0800, CRC(098ac15b) SHA1(cce28a2540a9eabb473391fff92895129ae41751) )
	ROM_LOAD( "6.13d",  	  0x4800, 0x0800, CRC(240a9d4b) SHA1(ca1c69fafec0471141ce1254ddfaef54fecfcbf0) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "enigma2.s",    0x0000, 0x1000, CRC(68fd8c54) SHA1(69996d5dfd996f0aacb26e397bef314204a2a88a) )

	ROM_REGION( 0x1000, "proms", 0 )	/* color map/star map */
	ROM_LOAD( "7.11f",        0x0000, 0x0800, CRC(409b5aad) SHA1(1b774a70f725637458ed68df9ed42476291b0e43) )
	ROM_LOAD( "8.13f",        0x0800, 0x0800, CRC(e9cb116d) SHA1(41da4f46c5614ec3345c233467ebad022c6b0bf5) )
ROM_END


ROM_START( enigma2a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "36_en1.bin",   0x0000, 0x0800, CRC(15f44806) SHA1(4a2f7bc91d4edf7a069e0865d964371c97af0a0a) )
	ROM_LOAD( "35_en2.bin",   0x0800, 0x0800, CRC(e841072f) SHA1(6ab02fd9fdeac5ab887cd25eee3d6b70ab01f849) )
	ROM_LOAD( "34_en3.bin",   0x1000, 0x0800, CRC(43d06cf4) SHA1(495af05d54c0325efb67347f691e64d194645d85) )
	ROM_LOAD( "33_en4.bin",   0x1800, 0x0800, CRC(8879a430) SHA1(c97f44bef3741eef74e137d2459e79f1b3a90457) )
	ROM_LOAD( "5.11d",        0x4000, 0x0800, CRC(098ac15b) SHA1(cce28a2540a9eabb473391fff92895129ae41751) )
	ROM_LOAD( "6.13d",  	  0x4800, 0x0800, CRC(240a9d4b) SHA1(ca1c69fafec0471141ce1254ddfaef54fecfcbf0) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sound.bin",    0x0000, 0x0800, BAD_DUMP CRC(5f092d3c) SHA1(17c70f6af1b5560a45e6b1bdb330a98b27570fe9) )
ROM_END

ROM_START( enigma2b )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ic36.bin",   0x0000, 0x0800, CRC(71dc9ecc) SHA1(a41260259cf0a36b01b5e8ad35cf968e920d22d9) )
	ROM_LOAD( "ic35.bin",   0x0800, 0x0800, CRC(e841072f) SHA1(6ab02fd9fdeac5ab887cd25eee3d6b70ab01f849) )
	ROM_LOAD( "ic34.bin",   0x1000, 0x0800, CRC(1384073d) SHA1(7a3a910c0431e680cc952a10a040b02f3df0532a) )
	ROM_LOAD( "ic33.bin",   0x1800, 0x0800, CRC(ac6c2410) SHA1(d35565a5ffe795d0c36970bd9c2f948bf79e0ed8) )
	ROM_LOAD( "ic32.bin",   0x4000, 0x0800, CRC(098ac15b) SHA1(cce28a2540a9eabb473391fff92895129ae41751) )
	ROM_LOAD( "ic42.bin",   0x4800, 0x0800, CRC(240a9d4b) SHA1(ca1c69fafec0471141ce1254ddfaef54fecfcbf0) )

	/* this rom was completely broken on this pcb.. */
	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sound.bin",    0x0000, 0x0800, BAD_DUMP CRC(5f092d3c) SHA1(17c70f6af1b5560a45e6b1bdb330a98b27570fe9) )
ROM_END


static DRIVER_INIT(enigma2)
{
	offs_t i;
	UINT8 *rom = memory_region(machine, "audiocpu");

	for(i = 0; i < 0x2000; i++)
	{
		rom[i] = BITSWAP8(rom[i],4,5,6,0,7,1,3,2);
	}
}



GAME( 1981, enigma2,  0,	   enigma2,  enigma2,  enigma2, ROT270, "GamePlan (Zilec Electronics license)", "Enigma II", GAME_SUPPORTS_SAVE )
GAME( 1984, enigma2a, enigma2, enigma2a, enigma2a, enigma2, ROT270, "Zilec Electronics", "Enigma II (Space Invaders hardware)", GAME_SUPPORTS_SAVE )
GAME( 1981, enigma2b, enigma2, enigma2a, enigma2a, enigma2, ROT270, "Zilec Electronics", "Phantoms II (Space Invaders hardware)", GAME_SUPPORTS_SAVE )
