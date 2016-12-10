/***************************************************************************

    Sega Hang On hardware

****************************************************************************

    Known bugs:
        * none at this time

    To do for each game:
        * verify analog input min/max
        * verify protection

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/mcs51/mcs51.h"
#include "deprecat.h"
#include "includes/segas16.h"
#include "machine/segaic16.h"
#include "machine/fd1089.h"
#include "machine/8255ppi.h"
#include "cpu/m68000/m68000.h"
#include "sound/2203intf.h"
#include "sound/2151intf.h"
#include "sound/segapcm.h"
#include "video/segaic16.h"


#define MASTER_CLOCK_25MHz		(25174800)
#define MASTER_CLOCK_10MHz		(10000000)
#define MASTER_CLOCK_8MHz		(8000000)


/*************************************
 *
 *  Statics
 *
 *************************************/

static UINT16 *workram;

/*************************************
 *
 *  Prototypes
 *
 *************************************/

static WRITE8_DEVICE_HANDLER( sound_latch_w );
static WRITE8_DEVICE_HANDLER( video_lamps_w );
static WRITE8_DEVICE_HANDLER( tilemap_sound_w );
static WRITE8_DEVICE_HANDLER( sub_control_adc_w );

static READ8_DEVICE_HANDLER( adc_status_r );



/*************************************
 *
 *  PPI interfaces
 *
 *************************************/

static const ppi8255_interface hangon_ppi_intf[2] =
{
	{
		DEVCB_NULL,
		DEVCB_NULL,
		DEVCB_NULL,
		DEVCB_HANDLER(sound_latch_w),
		DEVCB_HANDLER(video_lamps_w),
		DEVCB_HANDLER(tilemap_sound_w)
	},
	{
		DEVCB_NULL,
		DEVCB_NULL,
		DEVCB_HANDLER(adc_status_r),
		DEVCB_HANDLER(sub_control_adc_w),
		DEVCB_NULL,
		DEVCB_NULL
	}
};



/*************************************
 *
 *  Configuration
 *
 *************************************/

static void hangon_generic_init( running_machine *machine )
{
	segas1x_state *state = (segas1x_state *)machine->driver_data;

	/* reset the custom handlers and other pointers */
	state->i8751_vblank_hook = NULL;

	state->maincpu = machine->device("maincpu");
	state->soundcpu = machine->device("soundcpu");
	state->subcpu = machine->device("sub");
	state->mcu = machine->device("mcu");
	state->ppi8255_1 = machine->device("ppi8255_1");
	state->ppi8255_2 = machine->device("ppi8255_2");

	state_save_register_global(machine, state->adc_select);
}


static TIMER_CALLBACK( suspend_i8751 )
{
	segas1x_state *state = (segas1x_state *)machine->driver_data;
	cpu_suspend(state->mcu, SUSPEND_REASON_DISABLE, 1);
}



/*************************************
 *
 *  Initialization & interrupts
 *
 *************************************/

static MACHINE_RESET( hangon )
{
	segas1x_state *state = (segas1x_state *)machine->driver_data;

	fd1094_machine_init(machine->device("sub"));

	/* reset misc components */
	segaic16_tilemap_reset(machine, 0);

	/* if we have a fake i8751 handler, disable the actual 8751 */
	if (state->i8751_vblank_hook != NULL)
		timer_call_after_resynch(machine, NULL, 0, suspend_i8751);

	/* reset global state */
	state->adc_select = 0;
}

#if 0
static INTERRUPT_GEN( hangon_irq )
{
	/* according to the schematics, IRQ2 is generated every 16 scanlines */
	if (cpu_getiloops(device) != 0)
		cpu_set_input_line(device, 2, HOLD_LINE);
	else
		cpu_set_input_line(device, 4, HOLD_LINE);
}
#endif


/*************************************
 *
 *  I/O space
 *
 *************************************/

static TIMER_CALLBACK( delayed_ppi8255_w )
{
	segas1x_state *state = (segas1x_state *)machine->driver_data;
	ppi8255_w(state->ppi8255_1, param >> 8, param & 0xff);
}


static READ16_HANDLER( hangon_io_r )
{
	segas1x_state *state = (segas1x_state *)space->machine->driver_data;

	switch (offset & 0x3020/2)
	{
		case 0x0000/2: /* PPI @ 4B */
			return ppi8255_r(state->ppi8255_1, offset & 3);

		case 0x1000/2: /* Input ports and DIP switches */
		{
			static const char *const sysports[] = { "SERVICE", "COINAGE", "DSW", "UNKNOWN" };
			return input_port_read(space->machine, sysports[offset & 3]);
		}

		case 0x3000/2: /* PPI @ 4C */
			return ppi8255_r(state->ppi8255_2, offset & 3);

		case 0x3020/2: /* ADC0804 data output */
		{
			static const char *const adcports[] = { "ADC0", "ADC1", "ADC2", "ADC3" };
			return input_port_read_safe(space->machine, adcports[state->adc_select], 0);
		}
	}

	logerror("%06X:hangon_io_r - unknown read access to address %04X\n", cpu_get_pc(space->cpu), offset * 2);
	return segaic16_open_bus_r(space, 0, mem_mask);
}


static WRITE16_HANDLER( hangon_io_w )
{
	segas1x_state *state = (segas1x_state *)space->machine->driver_data;

	if (ACCESSING_BITS_0_7)
		switch (offset & 0x3020/2)
		{
			case 0x0000/2: /* PPI @ 4B */
				/* the port C handshaking signals control the Z80 NMI, */
				/* so we have to sync whenever we access this PPI */
				timer_call_after_resynch(space->machine, NULL, ((offset & 3) << 8) | (data & 0xff), delayed_ppi8255_w);
				return;

			case 0x3000/2: /* PPI @ 4C */
				ppi8255_w(state->ppi8255_2, offset & 3, data & 0xff);
				return;

			case 0x3020/2: /* ADC0804 */
				return;
		}

	logerror("%06X:hangon_io_w - unknown write access to address %04X = %04X & %04X\n", cpu_get_pc(space->cpu), offset * 2, data, mem_mask);
}


static READ16_HANDLER( sharrier_io_r )
{
	segas1x_state *state = (segas1x_state *)space->machine->driver_data;

	switch (offset & 0x0030/2)
	{
		case 0x0000/2:
			return ppi8255_r(state->ppi8255_1, offset & 3);

		case 0x0010/2: /* Input ports and DIP switches */
		{
			static const char *const sysports[] = { "SERVICE", "UNKNOWN", "COINAGE", "DSW" };
			return input_port_read(space->machine, sysports[offset & 3]);
		}

		case 0x0020/2: /* PPI @ 4C */
			if (offset == 2) return 0;
			return ppi8255_r(state->ppi8255_2, offset & 3);

		case 0x0030/2: /* ADC0804 data output */
		{
			static const char *const adcports[] = { "ADC0", "ADC1", "ADC2", "ADC3" };
			return input_port_read_safe(space->machine, adcports[state->adc_select], 0);
		}
	}

	logerror("%06X:sharrier_io_r - unknown read access to address %04X\n", cpu_get_pc(space->cpu), offset * 2);
	return segaic16_open_bus_r(space, 0, mem_mask);
}


static WRITE16_HANDLER( sharrier_io_w )
{
	segas1x_state *state = (segas1x_state *)space->machine->driver_data;

	if (ACCESSING_BITS_0_7)
		switch (offset & 0x0030/2)
		{
			case 0x0000/2:
				/* the port C handshaking signals control the Z80 NMI, */
				/* so we have to sync whenever we access this PPI */
				timer_call_after_resynch(space->machine, NULL, ((offset & 3) << 8) | (data & 0xff), delayed_ppi8255_w);
				return;

			case 0x0020/2: /* PPI @ 4C */
				ppi8255_w(state->ppi8255_2, offset & 3, data & 0xff);
				return;

			case 0x0030/2: /* ADC0804 */
				return;
		}

	logerror("%06X:sharrier_io_w - unknown write access to address %04X = %04X & %04X\n", cpu_get_pc(space->cpu), offset * 2, data, mem_mask);
}



/*************************************
 *
 *  PPI I/O handlers
 *
 *************************************/

static WRITE8_DEVICE_HANDLER( sound_latch_w )
{
	segas1x_state *state = (segas1x_state *)device->machine->driver_data;
	const address_space *space = cpu_get_address_space(state->maincpu, ADDRESS_SPACE_PROGRAM);
	soundlatch_w(space, offset, data);
}


static WRITE8_DEVICE_HANDLER( video_lamps_w )
{
	/* Port B : Miscellaneous outputs */
	/* D7 : FLIPC (1= flip screen, 0= normal orientation) */
	/* D6 : SHADE0 (1= highlight, 0= shadow) */
	/* D4 : /KILL (1= screen on, 0= screen off) */
	/* D3 : LAMP2 */
	/* D2 : LAMP1 */
	/* D1 : COIN2 */
	/* D0 : COIN1 */
	segaic16_tilemap_set_flip(device->machine, 0, data & 0x80);
	segaic16_sprites_set_flip(device->machine, 0, data & 0x80);
	segaic16_sprites_set_shadow(device->machine, 0, ~data & 0x40);
	segaic16_set_display_enable(device->machine, data & 0x10);
	set_led_status(device->machine, 1, data & 0x08);
	set_led_status(device->machine, 0, data & 0x04);
	coin_counter_w(device->machine, 1, data & 0x02);
	coin_counter_w(device->machine, 0, data & 0x01);
}


static WRITE8_DEVICE_HANDLER( tilemap_sound_w )
{
	segas1x_state *state = (segas1x_state *)device->machine->driver_data;

	/* Port C : Tilemap origin and audio mute */
	/* D7 : Port A handshaking signal /OBF */
	/* D6 : Port A handshaking signal ACK */
	/* D5 : Port A handshaking signal IBF */
	/* D4 : Port A handshaking signal /STB */
	/* D3 : Port A handshaking signal INTR */
	/* D2 : SCONT1 - Tilemap origin bit 1 */
	/* D1 : SCONT0 - Tilemap origin bit 0 */
	/* D0 : MUTE (1= audio on, 0= audio off) */
	cpu_set_input_line(state->soundcpu, INPUT_LINE_NMI, (data & 0x80) ? CLEAR_LINE : ASSERT_LINE);
	segaic16_tilemap_set_colscroll(device->machine, 0, ~data & 0x04);
	segaic16_tilemap_set_rowscroll(device->machine, 0, ~data & 0x02);
	sound_global_enable(device->machine, data & 0x01);
}


static WRITE8_DEVICE_HANDLER( sub_control_adc_w )
{
	segas1x_state *state = (segas1x_state *)device->machine->driver_data;

	/* Port A : S.CPU control and ADC channel select */
	/* D6 : INTR line on second CPU */
	/* D5 : RESET line on second CPU */
	/* D3-D2 : ADC_SELECT */
	cpu_set_input_line(state->subcpu, 4, (data & 0x40) ? CLEAR_LINE : ASSERT_LINE);
	cpu_set_input_line(state->subcpu, INPUT_LINE_RESET, (data & 0x20) ? ASSERT_LINE : CLEAR_LINE);

	/* If the CPU is being Reset we also need to reset the fd1094 state */
	if (data & 0x20)
	{
		fd1094_machine_init(state->subcpu);
	}

	state->adc_select = (data >> 2) & 3;
}


static READ8_DEVICE_HANDLER( adc_status_r )
{
	/* D7 = 0 (left open) */
	/* D6 = /INTR of ADC0804 */
	/* D5 = 0 (left open) */
	/* D4 = 0 (left open) */
	return 0x00;
}



/*************************************
 *
 *  I8751 interrupt generation
 *
 *************************************/

static INTERRUPT_GEN( i8751_main_cpu_vblank )
{
	segas1x_state *state = (segas1x_state *)device->machine->driver_data;

	/* if we have a fake 8751 handler, call it on VBLANK */
	if (state->i8751_vblank_hook != NULL)
		(*state->i8751_vblank_hook)(device->machine);
	irq4_line_hold(device);
}



/*************************************
 *
 *  Per-game I8751 workarounds
 *
 *************************************/

static void sharrier_i8751_sim(running_machine *machine)
{
	workram[0x492/2] = (input_port_read(machine, "ADC0") << 8) | input_port_read(machine, "ADC1");
}



/*************************************
 *
 *  Sound communications
 *
 *************************************/

static void sound_irq(running_device *device, int irq)
{
	segas1x_state *state = (segas1x_state *)device->machine->driver_data;
	cpu_set_input_line(state->soundcpu, 0, irq ? ASSERT_LINE : CLEAR_LINE);
}


static READ8_HANDLER( sound_data_r )
{
	segas1x_state *state = (segas1x_state *)space->machine->driver_data;

	/* assert ACK */
	ppi8255_set_port_c(state->ppi8255_1, 0x00);
	return soundlatch_r(space, offset);
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( hangon_map, ADDRESS_SPACE_PROGRAM, 16 )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x20c000, 0x20ffff) AM_RAM
	AM_RANGE(0x400000, 0x403fff) AM_RAM_WRITE(segaic16_tileram_0_w) AM_BASE(&segaic16_tileram_0)
	AM_RANGE(0x410000, 0x410fff) AM_RAM_WRITE(segaic16_textram_0_w) AM_BASE(&segaic16_textram_0)
	AM_RANGE(0x600000, 0x6007ff) AM_RAM AM_BASE(&segaic16_spriteram_0)
	AM_RANGE(0xa00000, 0xa00fff) AM_RAM_WRITE(segaic16_paletteram_w) AM_BASE(&segaic16_paletteram)
	AM_RANGE(0xc00000, 0xc3ffff) AM_ROM AM_REGION("sub", 0)
	AM_RANGE(0xc68000, 0xc68fff) AM_RAM AM_SHARE("share1") AM_BASE(&segaic16_roadram_0)
	AM_RANGE(0xc7c000, 0xc7ffff) AM_RAM AM_SHARE("share2")
	AM_RANGE(0xe00000, 0xffffff) AM_READWRITE(hangon_io_r, hangon_io_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sharrier_map, ADDRESS_SPACE_PROGRAM, 16 )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x040000, 0x043fff) AM_RAM AM_BASE(&workram)
	AM_RANGE(0x100000, 0x107fff) AM_RAM_WRITE(segaic16_tileram_0_w) AM_BASE(&segaic16_tileram_0)
	AM_RANGE(0x108000, 0x108fff) AM_RAM_WRITE(segaic16_textram_0_w) AM_BASE(&segaic16_textram_0)
	AM_RANGE(0x110000, 0x110fff) AM_RAM_WRITE(segaic16_paletteram_w) AM_BASE(&segaic16_paletteram)
	AM_RANGE(0x124000, 0x127fff) AM_RAM AM_SHARE("share2")
	AM_RANGE(0x130000, 0x130fff) AM_RAM AM_BASE(&segaic16_spriteram_0)
	AM_RANGE(0x140000, 0x14ffff) AM_READWRITE(sharrier_io_r, sharrier_io_w)
	AM_RANGE(0xc68000, 0xc68fff) AM_RAM AM_SHARE("share1") AM_BASE(&segaic16_roadram_0)
ADDRESS_MAP_END



/*************************************
 *
 *  Second CPU memory handlers
 *
 *************************************/

 /* On Super Hang On there is a memory mapper, like the System16 one, todo: emulate it! */
static ADDRESS_MAP_START( sub_map, ADDRESS_SPACE_PROGRAM, 16 )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0x7ffff)
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x068000, 0x068fff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x07c000, 0x07ffff) AM_RAM AM_SHARE("share2")
ADDRESS_MAP_END



/*************************************
 *
 *  Sound CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( sound_map_2203, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_MIRROR(0x0800) AM_RAM
	AM_RANGE(0xd000, 0xd001) AM_MIRROR(0x0ffe) AM_DEVREADWRITE("ymsnd", ym2203_r, ym2203_w)
	AM_RANGE(0xe000, 0xe0ff) AM_MIRROR(0x0f00) AM_DEVREADWRITE("pcm", sega_pcm_r, sega_pcm_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_portmap_2203, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x40, 0x40) AM_MIRROR(0x3f) AM_READ(sound_data_r)
ADDRESS_MAP_END


static ADDRESS_MAP_START( sound_map_2151, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xf000, 0xf0ff) AM_MIRROR(0x700) AM_DEVREADWRITE("pcm", sega_pcm_r, sega_pcm_w)
	AM_RANGE(0xf800, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_portmap_2151, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_MIRROR(0x3e) AM_DEVREADWRITE("ymsnd", ym2151_r, ym2151_w)
	AM_RANGE(0x40, 0x40) AM_MIRROR(0x3f) AM_READ(sound_data_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_portmap_2203x2, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_MIRROR(0x3e) AM_DEVREADWRITE("ym1", ym2203_r, ym2203_w)
	AM_RANGE(0x40, 0x40) AM_MIRROR(0x3f) AM_READ(sound_data_r)
	AM_RANGE(0xc0, 0xc1) AM_MIRROR(0x3e) AM_DEVREADWRITE("ym2", ym2203_r, ym2203_w)
ADDRESS_MAP_END



/*************************************
 *
 *  i8751 MCU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( mcu_io_map, ADDRESS_SPACE_IO, 8 )
ADDRESS_MAP_END



/*************************************
 *
 *  Generic port definitions
 *
 *************************************/

static INPUT_PORTS_START( hangon_generic )
	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("COINAGE")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x07, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x05, "2 Coins/1 Credit 5/3 6/4" )
	PORT_DIPSETTING(    0x04, "2 Coins/1 Credit 4/3" )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, "1 Coin/1 Credit 2/3" )
	PORT_DIPSETTING(    0x02, "1 Coin/1 Credit 4/5" )
	PORT_DIPSETTING(    0x03, "1 Coin/1 Credit 5/6" )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, "Free Play (if Coin B too) or 1/1" )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(    0x70, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x50, "2 Coins/1 Credit 5/3 6/4" )
	PORT_DIPSETTING(    0x40, "2 Coins/1 Credit 4/3" )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, "1 Coin/1 Credit 2/3" )
	PORT_DIPSETTING(    0x20, "1 Coin/1 Credit 4/5" )
	PORT_DIPSETTING(    0x30, "1 Coin/1 Credit 5/6" )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, "Free Play (if Coin A too) or 1/1" )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("UNKNOWN")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( sharrier_generic )
	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("UNKNOWN")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("COINAGE")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x07, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x05, "2 Coins/1 Credit 5/3 6/4" )
	PORT_DIPSETTING(    0x04, "2 Coins/1 Credit 4/3" )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, "1 Coin/1 Credit 2/3" )
	PORT_DIPSETTING(    0x02, "1 Coin/1 Credit 4/5" )
	PORT_DIPSETTING(    0x03, "1 Coin/1 Credit 5/6" )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, "Free Play (if Coin B too) or 1/1" )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(    0x70, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x50, "2 Coins/1 Credit 5/3 6/4" )
	PORT_DIPSETTING(    0x40, "2 Coins/1 Credit 4/3" )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, "1 Coin/1 Credit 2/3" )
	PORT_DIPSETTING(    0x20, "1 Coin/1 Credit 4/5" )
	PORT_DIPSETTING(    0x30, "1 Coin/1 Credit 5/6" )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, "Free Play (if Coin A too) or 1/1" )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END



/*************************************
 *
 *  Game-specific port definitions
 *
 *************************************/

static INPUT_PORTS_START( hangon )
	PORT_INCLUDE( hangon_generic )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x04, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x06, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x18, 0x18, "Time Adj." ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x20, 0x20, "Play Music" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )

	PORT_START("ADC0")	/* steering */
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x20,0xe0) PORT_SENSITIVITY(100) PORT_KEYDELTA(4) PORT_REVERSE

	PORT_START("ADC1")	/* gas pedal */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(20)

	PORT_START("ADC2")	/* brake */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_SENSITIVITY(100) PORT_KEYDELTA(40)
INPUT_PORTS_END


static INPUT_PORTS_START( shangupb )
	PORT_INCLUDE( hangon_generic )

	PORT_MODIFY("SERVICE")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Supercharger") PORT_CODE(KEYCODE_LSHIFT)

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x04, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x06, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x18, 0x18, "Time Adj." ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )

	PORT_START("ADC0")	/* steering */
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x20,0xe0) PORT_SENSITIVITY(100) PORT_KEYDELTA(4) PORT_REVERSE

	PORT_START("ADC1")	/* gas pedal */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(20)

	PORT_START("ADC2")	/* brake */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_SENSITIVITY(100) PORT_KEYDELTA(40)
INPUT_PORTS_END

static INPUT_PORTS_START( shangonro )
	PORT_INCLUDE( shangupb )

	PORT_MODIFY("SERVICE")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED ) // still shows in test mode next to a blank label
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Foot Sw. Right") PORT_CODE(KEYCODE_SPACE) // no effect?
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Foot Sw. Left") PORT_CODE(KEYCODE_LSHIFT) // no effect?
INPUT_PORTS_END


static INPUT_PORTS_START( sharrier )
	PORT_INCLUDE( sharrier_generic )

	PORT_MODIFY("SERVICE")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, "Moving" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, "5000000" )
	PORT_DIPSETTING(    0x00, "7000000" )
	PORT_DIPNAME( 0x20, 0x20, "Trial Time" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x80, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )

	PORT_START("ADC0")	/* X axis */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x20,0xe0) PORT_SENSITIVITY(100) PORT_KEYDELTA(4) PORT_REVERSE

	PORT_START("ADC1")	/* Y axis */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_MINMAX(0x60,0xa0) PORT_SENSITIVITY(100) PORT_KEYDELTA(4) PORT_REVERSE
INPUT_PORTS_END


static INPUT_PORTS_START( enduror )
	PORT_INCLUDE( sharrier_generic )

	PORT_MODIFY("SERVICE")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, "Wheelie" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x04, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x06, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x18, 0x18, "Time Adjust" ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x60, 0x60, "Time Control" ) PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("ADC0")	/* gas pedal */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(20)

	PORT_START("ADC1")	/* brake */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_SENSITIVITY(100) PORT_KEYDELTA(40)

	PORT_START("ADC2")	/* bank up/down */
	PORT_BIT( 0xff, 0x20, IPT_AD_STICK_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(4)

	PORT_START("ADC3")	/* steering */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_MINMAX(0x01,0xff) PORT_SENSITIVITY(100) PORT_KEYDELTA(4) PORT_REVERSE
INPUT_PORTS_END



/*************************************
 *
 *  Sound definitions
 *
 *************************************/

static const ym2203_interface ym2203_config =
{
	{
		AY8910_LEGACY_OUTPUT,
		AY8910_DEFAULT_LOADS,
		DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL
	},
	sound_irq
};


static const ym2151_interface ym2151_config =
{
	sound_irq
};


static const sega_pcm_interface segapcm_interface =
{
	BANK_512
};



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


static GFXDECODE_START( segahang )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,	0, 1024 )
GFXDECODE_END



/*************************************
 *
 *  Generic machine drivers
 *
 *************************************/

static MACHINE_DRIVER_START( hangon_base )

	/* driver data */
	MDRV_DRIVER_DATA(segas1x_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, MASTER_CLOCK_25MHz/4)
	MDRV_CPU_PROGRAM_MAP(hangon_map)
	MDRV_CPU_VBLANK_INT("screen", irq4_line_hold)

	MDRV_CPU_ADD("sub", M68000, MASTER_CLOCK_25MHz/4)
	MDRV_CPU_PROGRAM_MAP(sub_map)

	MDRV_MACHINE_RESET(hangon)
	MDRV_QUANTUM_TIME(HZ(6000))

	MDRV_PPI8255_ADD( "ppi8255_1", hangon_ppi_intf[0] )
	MDRV_PPI8255_ADD( "ppi8255_2", hangon_ppi_intf[1] )

	/* video hardware */
	MDRV_GFXDECODE(segahang)
	MDRV_PALETTE_LENGTH(2048*3)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_RAW_PARAMS(MASTER_CLOCK_25MHz/4, 400, 0, 320, 262, 0, 224)

	MDRV_VIDEO_START(hangon)
	MDRV_VIDEO_UPDATE(hangon)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( sharrier_base )
	MDRV_IMPORT_FROM(hangon_base)

	/* basic machine hardware */
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_CLOCK(MASTER_CLOCK_10MHz)
	MDRV_CPU_PROGRAM_MAP(sharrier_map)
	MDRV_CPU_VBLANK_INT("screen", i8751_main_cpu_vblank)

	MDRV_CPU_MODIFY("sub")
	MDRV_CPU_CLOCK(MASTER_CLOCK_10MHz)

	/* video hardware */
	MDRV_VIDEO_START(sharrier)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( sound_board_2203 )

	/* basic machine hardware */
	MDRV_CPU_ADD("soundcpu", Z80, MASTER_CLOCK_8MHz/2)
	MDRV_CPU_PROGRAM_MAP(sound_map_2203)
	MDRV_CPU_IO_MAP(sound_portmap_2203)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2203, MASTER_CLOCK_8MHz/2)
	MDRV_SOUND_CONFIG(ym2203_config)
	MDRV_SOUND_ROUTE(0, "lspeaker",  0.13)
	MDRV_SOUND_ROUTE(0, "rspeaker", 0.13)
	MDRV_SOUND_ROUTE(1, "lspeaker",  0.13)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.13)
	MDRV_SOUND_ROUTE(2, "lspeaker",  0.13)
	MDRV_SOUND_ROUTE(2, "rspeaker", 0.13)
	MDRV_SOUND_ROUTE(3, "lspeaker",  0.37)
	MDRV_SOUND_ROUTE(3, "rspeaker", 0.37)

	MDRV_SOUND_ADD("pcm", SEGAPCM, MASTER_CLOCK_8MHz)
	MDRV_SOUND_CONFIG(segapcm_interface)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( sound_board_2203x2 )

	/* basic machine hardware */
	MDRV_CPU_ADD("soundcpu", Z80, MASTER_CLOCK_8MHz/2)
	MDRV_CPU_PROGRAM_MAP(sound_map_2151)
	MDRV_CPU_IO_MAP(sound_portmap_2203x2)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ym1", YM2203, MASTER_CLOCK_8MHz/2)
	MDRV_SOUND_CONFIG(ym2203_config)
	MDRV_SOUND_ROUTE(0, "lspeaker",  0.13)
	MDRV_SOUND_ROUTE(0, "rspeaker", 0.13)
	MDRV_SOUND_ROUTE(1, "lspeaker",  0.13)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.13)
	MDRV_SOUND_ROUTE(2, "lspeaker",  0.13)
	MDRV_SOUND_ROUTE(2, "rspeaker", 0.13)
	MDRV_SOUND_ROUTE(3, "lspeaker",  0.37)
	MDRV_SOUND_ROUTE(3, "rspeaker", 0.37)

	MDRV_SOUND_ADD("ym2", YM2203, MASTER_CLOCK_8MHz/2)
	MDRV_SOUND_ROUTE(0, "lspeaker",  0.13)
	MDRV_SOUND_ROUTE(0, "rspeaker", 0.13)
	MDRV_SOUND_ROUTE(1, "lspeaker",  0.13)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.13)
	MDRV_SOUND_ROUTE(2, "lspeaker",  0.13)
	MDRV_SOUND_ROUTE(2, "rspeaker", 0.13)
	MDRV_SOUND_ROUTE(3, "lspeaker",  0.37)
	MDRV_SOUND_ROUTE(3, "rspeaker", 0.37)

	MDRV_SOUND_ADD("pcm", SEGAPCM, MASTER_CLOCK_8MHz/2)
	MDRV_SOUND_CONFIG(segapcm_interface)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( sound_board_2151 )

	/* basic machine hardware */
	MDRV_CPU_ADD("soundcpu", Z80, MASTER_CLOCK_8MHz/2)
	MDRV_CPU_PROGRAM_MAP(sound_map_2151)
	MDRV_CPU_IO_MAP(sound_portmap_2151)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2151, MASTER_CLOCK_8MHz/2)
	MDRV_SOUND_CONFIG(ym2151_config)
	MDRV_SOUND_ROUTE(0, "lspeaker", 0.43)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.43)

	MDRV_SOUND_ADD("pcm", SEGAPCM, MASTER_CLOCK_8MHz/2)
	MDRV_SOUND_CONFIG(segapcm_interface)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END



/*************************************
 *
 *  Specific machine drivers
 *
 *************************************/

static MACHINE_DRIVER_START( hangon )
	MDRV_IMPORT_FROM(hangon_base)
	MDRV_IMPORT_FROM(sound_board_2203)

	MDRV_SEGA16SP_ADD_HANGON("segaspr1")
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( shangupb )
	MDRV_IMPORT_FROM(hangon_base)
	MDRV_IMPORT_FROM(sound_board_2151)

	/* not sure about these speeds, but at 6MHz, the road is not updated fast enough */
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_CLOCK(10000000)
	MDRV_CPU_MODIFY("sub")
	MDRV_CPU_CLOCK(10000000)

	MDRV_SEGA16SP_ADD_HANGON("segaspr1")
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( sharrier )
	MDRV_IMPORT_FROM(sharrier_base)
	MDRV_IMPORT_FROM(sound_board_2203)

	MDRV_CPU_ADD("mcu", I8751, 8000000)
	MDRV_CPU_IO_MAP(mcu_io_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_pulse)

	MDRV_SEGA16SP_ADD_SHARRIER("segaspr1")
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( enduror )
	MDRV_IMPORT_FROM(sharrier_base)
	MDRV_IMPORT_FROM(sound_board_2151)

	MDRV_SEGA16SP_ADD_SHARRIER("segaspr1")
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( enduror1 )
	MDRV_IMPORT_FROM(sharrier_base)
	MDRV_IMPORT_FROM(sound_board_2203)

	MDRV_SEGA16SP_ADD_SHARRIER("segaspr1")
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( endurob2 )
	MDRV_IMPORT_FROM(sharrier_base)
	MDRV_IMPORT_FROM(sound_board_2203x2)

	MDRV_SEGA16SP_ADD_SHARRIER("segaspr1")
MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Hang On (Rev A)
    CPU: 68000 (317-????)

     ASSY CPU BD 834-5704-01
     ASSY CONTROL BD 834-5668
     ASSY ROM BD 834-5669
     ASSY SOUND BD 834-5670

*/
ROM_START( hangon )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-6918a.ic22", 0x000000, 0x8000, CRC(20b1c2b0) SHA1(01b4f5105e2bbeb6ec6dbd18bfb728e3a973e0ca) )
	ROM_LOAD16_BYTE( "epr-6916a.ic8",  0x000001, 0x8000, CRC(7d9db1bf) SHA1(952ee3e7a0d57ec1bb3385e0e6675890b8378d31) )
	ROM_LOAD16_BYTE( "epr-6917a.ic20", 0x010000, 0x8000, CRC(fea12367) SHA1(9a1ce5863c562160b657ad948812b43f42d7d0cc) )
	ROM_LOAD16_BYTE( "epr-6915a.ic6",  0x010001, 0x8000, CRC(ac883240) SHA1(f943341ae13e062f3d12c6221180086ce8bdb8c4) )

	ROM_REGION( 0x40000, "sub", 0 ) /* second 68000 CPU */
	ROM_LOAD16_BYTE( "epr-6920.ic63", 0x0000, 0x8000, CRC(1c95013e) SHA1(8344ac953477279c2c701f984d98292a21dd2f7d) )
	ROM_LOAD16_BYTE( "epr-6919.ic51", 0x0001, 0x8000, CRC(6ca30d69) SHA1(ed933351883ebf6d9ef9428a81d09749b609cd60) )

	ROM_REGION( 0x18000, "gfx1", 0 ) /* tiles */
	ROM_LOAD( "epr-6841.ic38", 0x00000, 0x08000, CRC(54d295dc) SHA1(ad8cdb281032a2f931c2abbeb966998944683dc3) )
	ROM_LOAD( "epr-6842.ic23", 0x08000, 0x08000, CRC(f677b568) SHA1(636ca60bd4be9b5c2be09de8ae49db1063aa6c79) )
	ROM_LOAD( "epr-6843.ic7",  0x10000, 0x08000, CRC(a257f0da) SHA1(9828f8ce4ef245ffb8dbad347f9ca74ed81aa998) )

	ROM_REGION16_BE( 0x80000, "gfx2", 0 ) /* sprites */
	ROM_LOAD16_BYTE( "epr-6819.ic27", 0x000001, 0x8000, CRC(469dad07) SHA1(6d01c0b3506e28832928ad74d518577ff5be323b) )
	ROM_LOAD16_BYTE( "epr-6820.ic34", 0x000000, 0x8000, CRC(87cbc6de) SHA1(b64652e062e1b88c6f6ae8dd2ffe4533bb27ba45) )
	ROM_LOAD16_BYTE( "epr-6821.ic28", 0x010001, 0x8000, CRC(15792969) SHA1(b061dbf24e8b511116446794753c8b0cc49e2149) )
	ROM_LOAD16_BYTE( "epr-6822.ic35", 0x010000, 0x8000, CRC(e9718de5) SHA1(30e3a7d5b33504da03c5780b4a946b977e46098a) )
	ROM_LOAD16_BYTE( "epr-6823.ic29", 0x020001, 0x8000, CRC(49422691) SHA1(caee2a4a3f4587ae27dec330214edaa1229012af) )
	ROM_LOAD16_BYTE( "epr-6824.ic36", 0x020000, 0x8000, CRC(701deaa4) SHA1(053032ef886b85a4cb4753d17b3c27d228695157) )
	ROM_LOAD16_BYTE( "epr-6825.ic30", 0x030001, 0x8000, CRC(6e23c8b4) SHA1(b17fd7d590ed4e6616b7b4d91a47a2820248d8c7) )
	ROM_LOAD16_BYTE( "epr-6826.ic37", 0x030000, 0x8000, CRC(77d0de2c) SHA1(83b126ed1d463504b2702391816e6e20dcd04ffc) )
	ROM_LOAD16_BYTE( "epr-6827.ic31", 0x040001, 0x8000, CRC(7fa1bfb6) SHA1(a27b54c93613372f59050f0b2182d2984a8d2efe) )
	ROM_LOAD16_BYTE( "epr-6828.ic38", 0x040000, 0x8000, CRC(8e880c93) SHA1(8c55deec065daf09a5d1c1c1f3f3f7bc1aeaf563) )
	ROM_LOAD16_BYTE( "epr-6829.ic32", 0x050001, 0x8000, CRC(7ca0952d) SHA1(617d73591158ed3fea5174f7dabf0413d28de9b3) )
	ROM_LOAD16_BYTE( "epr-6830.ic39", 0x050000, 0x8000, CRC(b1a63aef) SHA1(5db0a1cc2d13c6cfc77044f5d7f6f99d198531ed) )
	ROM_LOAD16_BYTE( "epr-6845.ic18", 0x060001, 0x8000, CRC(ba08c9b8) SHA1(65ceaefa18999c468b38576c29101674d1f63e5f) )
	ROM_LOAD16_BYTE( "epr-6846.ic25", 0x060000, 0x8000, CRC(f21e57a3) SHA1(92ce0723e722f446c0cef9e23080a008aa9752e7) )

	ROM_REGION( 0x8000, "gfx3", 0 ) /* road gfx */
	ROM_LOAD( "epr-6840.ic108", 0x0000, 0x8000, CRC(581230e3) SHA1(954eab35059322a12a197bba04bf85f816132f20) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* sound CPU */
	ROM_LOAD( "epr-6833.ic73", 0x00000, 0x4000, CRC(3b942f5f) SHA1(4384b5c090954e69de561dde0ef32104aa11399a) )

	ROM_REGION( 0x10000, "pcm", 0 ) /* Sega PCM sound data */
	ROM_LOAD( "epr-6831.ic5", 0x00000, 0x8000, CRC(cfef5481) SHA1(c04b302fee58f0e59a097b2be2b61e5d03df7c91) )
	ROM_LOAD( "epr-6832.ic6", 0x08000, 0x8000, CRC(4165aea5) SHA1(be05c6d295807af2f396a1ff72d5a3d2a1e6054d) )

	ROM_REGION( 0x2000, "proms", 0 ) /* zoom table */
	ROM_LOAD( "epr-6844.ic123", 0x0000, 0x2000, CRC(e3ec7bd6) SHA1(feec0fe664e16fac0fde61cf64b401b9b0575323) )
ROM_END


/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Hang On
    CPU: 68000 (317-????)

     ASSY CPU BD 834-5704-01
     ASSY CONTROL BD 834-5668
     ASSY ROM BD 834-5669
     ASSY SOUND BD 834-5670

*/
ROM_START( hangon1 )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-6918.ic22", 0x000000, 0x8000, CRC(0bf4f2ac) SHA1(26c5bb6fe805644a8d427ad77814f4b0b1128b1a) )
	ROM_LOAD16_BYTE( "epr-6916.ic8",  0x000001, 0x8000, CRC(06c21c8a) SHA1(f06f21ff272a803c72e5041534053494f055e466) )
	ROM_LOAD16_BYTE( "epr-6917.ic20", 0x010000, 0x8000, CRC(f48a6cbc) SHA1(6437efaeb0e4cb727c03eb83678a9e107d244af1) )
	ROM_LOAD16_BYTE( "epr-6915.ic6",  0x010001, 0x8000, CRC(75d3b5ee) SHA1(00948d0610f52b1b554cadde96227428e510e73e) )

	ROM_REGION( 0x40000, "sub", 0 ) /* second 68000 CPU */
	ROM_LOAD16_BYTE( "epr-6920.ic63", 0x0000, 0x8000, CRC(1c95013e) SHA1(8344ac953477279c2c701f984d98292a21dd2f7d) )
	ROM_LOAD16_BYTE( "epr-6919.ic51", 0x0001, 0x8000, CRC(6ca30d69) SHA1(ed933351883ebf6d9ef9428a81d09749b609cd60) )

	ROM_REGION( 0x18000, "gfx1", 0 ) /* tiles */
	ROM_LOAD( "epr-6841.ic38", 0x00000, 0x08000, CRC(54d295dc) SHA1(ad8cdb281032a2f931c2abbeb966998944683dc3) )
	ROM_LOAD( "epr-6842.ic23", 0x08000, 0x08000, CRC(f677b568) SHA1(636ca60bd4be9b5c2be09de8ae49db1063aa6c79) )
	ROM_LOAD( "epr-6843.ic7",  0x10000, 0x08000, CRC(a257f0da) SHA1(9828f8ce4ef245ffb8dbad347f9ca74ed81aa998) )

	ROM_REGION16_BE( 0x80000, "gfx2", 0 ) /* sprites */
	ROM_LOAD16_BYTE( "epr-6819.ic27", 0x000001, 0x8000, CRC(469dad07) SHA1(6d01c0b3506e28832928ad74d518577ff5be323b) )
	ROM_LOAD16_BYTE( "epr-6820.ic34", 0x000000, 0x8000, CRC(87cbc6de) SHA1(b64652e062e1b88c6f6ae8dd2ffe4533bb27ba45) )
	ROM_LOAD16_BYTE( "epr-6821.ic28", 0x010001, 0x8000, CRC(15792969) SHA1(b061dbf24e8b511116446794753c8b0cc49e2149) )
	ROM_LOAD16_BYTE( "epr-6822.ic35", 0x010000, 0x8000, CRC(e9718de5) SHA1(30e3a7d5b33504da03c5780b4a946b977e46098a) )
	ROM_LOAD16_BYTE( "epr-6823.ic29", 0x020001, 0x8000, CRC(49422691) SHA1(caee2a4a3f4587ae27dec330214edaa1229012af) )
	ROM_LOAD16_BYTE( "epr-6824.ic36", 0x020000, 0x8000, CRC(701deaa4) SHA1(053032ef886b85a4cb4753d17b3c27d228695157) )
	ROM_LOAD16_BYTE( "epr-6825.ic30", 0x030001, 0x8000, CRC(6e23c8b4) SHA1(b17fd7d590ed4e6616b7b4d91a47a2820248d8c7) )
	ROM_LOAD16_BYTE( "epr-6826.ic37", 0x030000, 0x8000, CRC(77d0de2c) SHA1(83b126ed1d463504b2702391816e6e20dcd04ffc) )
	ROM_LOAD16_BYTE( "epr-6827.ic31", 0x040001, 0x8000, CRC(7fa1bfb6) SHA1(a27b54c93613372f59050f0b2182d2984a8d2efe) )
	ROM_LOAD16_BYTE( "epr-6828.ic38", 0x040000, 0x8000, CRC(8e880c93) SHA1(8c55deec065daf09a5d1c1c1f3f3f7bc1aeaf563) )
	ROM_LOAD16_BYTE( "epr-6829.ic32", 0x050001, 0x8000, CRC(7ca0952d) SHA1(617d73591158ed3fea5174f7dabf0413d28de9b3) )
	ROM_LOAD16_BYTE( "epr-6830.ic39", 0x050000, 0x8000, CRC(b1a63aef) SHA1(5db0a1cc2d13c6cfc77044f5d7f6f99d198531ed) )
	ROM_LOAD16_BYTE( "epr-6845.ic18", 0x060001, 0x8000, CRC(ba08c9b8) SHA1(65ceaefa18999c468b38576c29101674d1f63e5f) )
	ROM_LOAD16_BYTE( "epr-6846.ic25", 0x060000, 0x8000, CRC(f21e57a3) SHA1(92ce0723e722f446c0cef9e23080a008aa9752e7) )

	ROM_REGION( 0x8000, "gfx3", 0 ) /* road gfx */
	ROM_LOAD( "epr-6840.ic108", 0x0000, 0x8000, CRC(581230e3) SHA1(954eab35059322a12a197bba04bf85f816132f20) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* sound CPU */
	ROM_LOAD( "epr-6833.ic73", 0x00000, 0x4000, CRC(3b942f5f) SHA1(4384b5c090954e69de561dde0ef32104aa11399a) )

	ROM_REGION( 0x10000, "pcm", 0 ) /* Sega PCM sound data */
	ROM_LOAD( "epr-6831.ic5", 0x00000, 0x8000, CRC(cfef5481) SHA1(c04b302fee58f0e59a097b2be2b61e5d03df7c91) )
	ROM_LOAD( "epr-6832.ic6", 0x08000, 0x8000, CRC(4165aea5) SHA1(be05c6d295807af2f396a1ff72d5a3d2a1e6054d) )

	ROM_REGION( 0x2000, "proms", 0 ) /* zoom table */
	ROM_LOAD( "epr-6844.ic123", 0x0000, 0x2000, CRC(e3ec7bd6) SHA1(feec0fe664e16fac0fde61cf64b401b9b0575323) )
ROM_END

/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************

Super Hang-On (Japan Ver.)
(c)1987 Sega
Ride-on Type

CPU: FD1094 317-0038

Top   : Label 834-6273
Middle: 834-5704 (Label 837-6340)
Bottom: 834-5668 (Label 837-6341)

*/

ROM_START( shangonro )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-10842.22", 0x00000, 0x08000, CRC(24289138) SHA1(700419bb8e4f97e128d85d0077e4aa39a1d2f167) )
	ROM_LOAD16_BYTE( "epr-10839.8",  0x00001, 0x08000, CRC(70f92d5e) SHA1(3ca0e23d6bb44bbe7d21840c8d179c57f8cbfd20) )
	ROM_LOAD16_BYTE( "epr-10841.20", 0x10000, 0x08000, CRC(3bb2186c) SHA1(755dbf5d37809ea1de2e96f9827cf373dc2d3f94) )
	ROM_LOAD16_BYTE( "epr-10838.6",  0x10001, 0x08000, CRC(6aded05a) SHA1(5459157bf4083f677c524d1cb839cb6f4f1a7ee2) )
	ROM_LOAD16_BYTE( "epr-10840.18", 0x20000, 0x08000, CRC(12ee8716) SHA1(8e798d23d22f85cd046641184d104c17b27995b2) )
	ROM_LOAD16_BYTE( "epr-10837.4",  0x20001, 0x08000, CRC(155e0cfd) SHA1(e51734351c887fe3920c881f57abdfbb7d075f57) )

	ROM_REGION( 0x2000, "user1", 0 ) /* FD1094 decryption key */
	ROM_LOAD( "317-0038.key", 0x0000, 0x2000, CRC(85943925) SHA1(76303b0aa79ca9d4a8d10d4e63ee2efe756a0a00) )

	ROM_REGION( 0x40000, "sub", 0 ) /* second 68000 CPU (encrypted FD1094) */
	ROM_LOAD16_BYTE( "epr-10833.31", 0x000001, 0x10000, CRC(13ba98bc) SHA1(83710a7bb9d038f8663e6d42b184d4e4d937a26f) )
	ROM_LOAD16_BYTE( "epr-10831.25", 0x000000, 0x10000, CRC(3a2de9eb) SHA1(20da548cd1fb466942ee45306cfd04766e5a4f50) )
	ROM_LOAD16_BYTE( "epr-10832.30", 0x020001, 0x10000, CRC(543cd7bb) SHA1(124b426adc2d8dc51172ef94cb215bde3b8b42a7) )
	ROM_LOAD16_BYTE( "epr-10830.24", 0x020000, 0x10000, CRC(2ae4e53a) SHA1(b15b5a8b36cbe5fe68b5e18ab3398ebc7214dbee) )

	ROM_REGION( 0x18000, "gfx1", 0 ) /* tiles */
	ROM_LOAD( "epr-10652.38", 0x00000, 0x08000, CRC(260286f9) SHA1(dc7c8d2c6ef924a937328685eed19bda1c8b1819) )
	ROM_LOAD( "epr-10651.23", 0x08000, 0x08000, CRC(c609ee7b) SHA1(c6dacf81cbfe7e5df1f9a967cf571be1dcf1c429) )
	ROM_LOAD( "epr-10650.7",  0x10000, 0x08000, CRC(b236a403) SHA1(af02b8122794c083a66f2ab35d2c73b84b2df0be) )

	ROM_REGION16_BE( 0x00e0000, "gfx2", 0 ) /* sprites */
	ROM_LOAD16_BYTE( "epr-10675.22", 0x000001, 0x010000, CRC(d6ac012b) SHA1(305023b1a0a9d84cfc081ffc2ad7578b53d562f2) )
	ROM_LOAD16_BYTE( "epr-10682.11", 0x000000, 0x010000, CRC(d9d83250) SHA1(f8ca3197edcdf53643a5b335c3c044ddc1310cd4) )
	ROM_LOAD16_BYTE( "epr-10676.21", 0x020001, 0x010000, CRC(25ebf2c5) SHA1(abcf673ae4e280417dd9f46d18c0ec7c0e4802ae) )
	ROM_LOAD16_BYTE( "epr-10683.12", 0x020000, 0x010000, CRC(6365d2e9) SHA1(688e2ba194e859f86cd3486c2575ebae257e975a) )
	ROM_LOAD16_BYTE( "epr-10677.20", 0x040001, 0x010000, CRC(8a57b8d6) SHA1(df1a31559dd2d1e7c2c9d800bf97526bdf3e84e6) )
	ROM_LOAD16_BYTE( "epr-10684.11", 0x040000, 0x010000, CRC(3aff8910) SHA1(4b41a49a7f02363424e814b37edce9a7a44a112e) )
	ROM_LOAD16_BYTE( "epr-10678.19", 0x060001, 0x010000, CRC(af473098) SHA1(a2afaba1cbf672949dc50e407b46d7e9ae183774) )
	ROM_LOAD16_BYTE( "epr-10685.10", 0x060000, 0x010000, CRC(80bafeef) SHA1(f01bcf65485e60f34e533295a896fca0b92e5b14) )
	ROM_LOAD16_BYTE( "epr-10679.18", 0x080001, 0x010000, CRC(03bc4878) SHA1(548fc58bcc620204e30fa12fa4c4f0a3f6a1e4c0) )
	ROM_LOAD16_BYTE( "epr-10686.9",  0x080000, 0x010000, CRC(274b734e) SHA1(906fa528659bc17c9b4744cec52f7096711adce8) )
	ROM_LOAD16_BYTE( "epr-10680.17", 0x0a0001, 0x010000, CRC(9f0677ed) SHA1(5964642b70bfad418da44f2d91476f887b021f74) )
	ROM_LOAD16_BYTE( "epr-10687.8",  0x0a0000, 0x010000, CRC(508a4701) SHA1(d17aea2aadc2e2cd65d81bf91feb3ef6923d5c0b) )
	ROM_LOAD16_BYTE( "epr-10681.16", 0x0c0001, 0x010000, CRC(b176ea72) SHA1(7ec0eb0f13398d014c2e235773ded00351edb3e2) )
	ROM_LOAD16_BYTE( "epr-10688.7",  0x0c0000, 0x010000, CRC(42fcd51d) SHA1(0eacb3527dc21746e5b901fcac83f2764a0f9e2c) )

	ROM_REGION( 0x8000, "gfx3", 0 ) /* road gfx */
	ROM_LOAD( "epr-10866.108", 0x0000, 0x08000, CRC(1bbe4fc8) SHA1(30f7f301e4d10d3b254d12bf3d32e5371661a566) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* sound CPU */
	ROM_LOAD( "epr-10834a.52", 0x0000, 0x08000, CRC(83347dc0) SHA1(079bb750edd6372750a207764e8c84bb6abf2f79) )

	ROM_REGION( 0x20000, "pcm", 0 ) /* Sega PCM sound data */
	ROM_LOAD( "epr-10835.55", 0x00000, 0x10000, CRC(da08ca2b) SHA1(2c94c127efd66f6cf86b25e2653637818a99aed1) )
	ROM_LOAD( "epr-10836.56", 0x10000, 0x10000, CRC(8b10e601) SHA1(75e9bcdd3f096be9bed672d61064b9240690deec) )

	ROM_REGION( 0x2000, "proms", 0 ) /* zoom table */
	ROM_LOAD( "epr-6844.119", 0x0000, 0x2000, CRC(e3ec7bd6) SHA1(feec0fe664e16fac0fde61cf64b401b9b0575323) )
ROM_END

/*
    Super Hang On bootleg (of the above?)
    CPU: 68000 (317-????)
*/

ROM_START( shangonrb )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "s-hangon.30", 0x000000, 0x10000, CRC(d95e82fc) SHA1(bc6cd0b0ac98a9c53f2e22ac086521704ab59e4d) )
	ROM_LOAD16_BYTE( "s-hangon.32", 0x000001, 0x10000, CRC(2ee4b4fb) SHA1(ba4042ab6e533c16c3cde848248d75e484be113f) )
	ROM_LOAD16_BYTE( "s-hangon.29", 0x020000, 0x08000, CRC(12ee8716) SHA1(8e798d23d22f85cd046641184d104c17b27995b2) ) /* Same as EPR-10840 above */
	ROM_LOAD16_BYTE( "s-hangon.31", 0x020001, 0x08000, CRC(155e0cfd) SHA1(e51734351c887fe3920c881f57abdfbb7d075f57) ) /* Same as EPR-10837 above */

	ROM_REGION( 0x40000, "sub", 0 ) /* second 68000 CPU */
	ROM_LOAD16_BYTE( "s-hangon.09", 0x00000, 0x10000, CRC(070c8059) SHA1(a18c5e9473b6634f6e7165300e39029335b41ba3) )
	ROM_LOAD16_BYTE( "s-hangon.05", 0x00001, 0x10000, CRC(9916c54b) SHA1(41a7c5a9bdb1e3feae8fadf1ac5f51fab6376157) )
	ROM_LOAD16_BYTE( "s-hangon.08", 0x20000, 0x10000, CRC(000ad595) SHA1(eb80e798159c09bc5142a7ea8b9b0f895976b0d4) )
	ROM_LOAD16_BYTE( "s-hangon.04", 0x20001, 0x10000, CRC(8f8f4af0) SHA1(1dac21b7df6ec6874d36a07e30de7129b7f7f33a) )

	ROM_REGION( 0x18000, "gfx1", 0 ) /* tiles */
	ROM_LOAD( "epr-10652.38", 0x00000, 0x08000, CRC(260286f9) SHA1(dc7c8d2c6ef924a937328685eed19bda1c8b1819) )
	ROM_LOAD( "epr-10651.23", 0x08000, 0x08000, CRC(c609ee7b) SHA1(c6dacf81cbfe7e5df1f9a967cf571be1dcf1c429) )
	ROM_LOAD( "epr-10650.7",  0x10000, 0x08000, CRC(b236a403) SHA1(af02b8122794c083a66f2ab35d2c73b84b2df0be) )

	ROM_REGION16_BE( 0x00e0000, "gfx2", 0 ) /* sprites */
	ROM_LOAD16_BYTE( "epr-10675.22", 0x000001, 0x010000, CRC(d6ac012b) SHA1(305023b1a0a9d84cfc081ffc2ad7578b53d562f2) )
	ROM_LOAD16_BYTE( "epr-10682.11", 0x000000, 0x010000, CRC(d9d83250) SHA1(f8ca3197edcdf53643a5b335c3c044ddc1310cd4) )
	ROM_LOAD16_BYTE( "s-hangon.20",  0x020001, 0x010000, CRC(eef23b3d) SHA1(2416fa9991afbdddf25d469082e53858289550db) )
	ROM_LOAD16_BYTE( "s-hangon.14",  0x020000, 0x010000, CRC(0f26d131) SHA1(0d8b6eb8b8aae0aa8f0fa0c31dc91ad0e610be3e) )
	ROM_LOAD16_BYTE( "epr-10677.20", 0x040001, 0x010000, CRC(8a57b8d6) SHA1(df1a31559dd2d1e7c2c9d800bf97526bdf3e84e6) )
	ROM_LOAD16_BYTE( "epr-10684.11", 0x040000, 0x010000, CRC(3aff8910) SHA1(4b41a49a7f02363424e814b37edce9a7a44a112e) )
	ROM_LOAD16_BYTE( "epr-10678.19", 0x060001, 0x010000, CRC(af473098) SHA1(a2afaba1cbf672949dc50e407b46d7e9ae183774) )
	ROM_LOAD16_BYTE( "epr-10685.10", 0x060000, 0x010000, CRC(80bafeef) SHA1(f01bcf65485e60f34e533295a896fca0b92e5b14) )
	ROM_LOAD16_BYTE( "epr-10679.18", 0x080001, 0x010000, CRC(03bc4878) SHA1(548fc58bcc620204e30fa12fa4c4f0a3f6a1e4c0) )
	ROM_LOAD16_BYTE( "epr-10686.9",  0x080000, 0x010000, CRC(274b734e) SHA1(906fa528659bc17c9b4744cec52f7096711adce8) )
	ROM_LOAD16_BYTE( "epr-10680.17", 0x0a0001, 0x010000, CRC(9f0677ed) SHA1(5964642b70bfad418da44f2d91476f887b021f74) )
	ROM_LOAD16_BYTE( "epr-10687.8",  0x0a0000, 0x010000, CRC(508a4701) SHA1(d17aea2aadc2e2cd65d81bf91feb3ef6923d5c0b) )
	ROM_LOAD16_BYTE( "epr-10681.16", 0x0c0001, 0x010000, CRC(b176ea72) SHA1(7ec0eb0f13398d014c2e235773ded00351edb3e2) )
	ROM_LOAD16_BYTE( "epr-10688.7",  0x0c0000, 0x010000, CRC(42fcd51d) SHA1(0eacb3527dc21746e5b901fcac83f2764a0f9e2c) )

	ROM_REGION( 0x8000, "gfx3", 0 ) /* road gfx */
	ROM_LOAD( "epr-10866.108", 0x0000, 0x08000, CRC(1bbe4fc8) SHA1(30f7f301e4d10d3b254d12bf3d32e5371661a566) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* sound CPU */
	ROM_LOAD( "epr-10834a.52", 0x0000, 0x08000, CRC(83347dc0) SHA1(079bb750edd6372750a207764e8c84bb6abf2f79) )

	ROM_REGION( 0x20000, "pcm", 0 ) /* Sega PCM sound data */
	ROM_LOAD( "epr-10835.55", 0x00000, 0x10000, CRC(da08ca2b) SHA1(2c94c127efd66f6cf86b25e2653637818a99aed1) )
	ROM_LOAD( "epr-10836.56", 0x10000, 0x10000, CRC(8b10e601) SHA1(75e9bcdd3f096be9bed672d61064b9240690deec) )

	ROM_REGION( 0x2000, "proms", 0 ) /* zoom table */
	ROM_LOAD( "epr-6844.119", 0x0000, 0x2000, CRC(e3ec7bd6) SHA1(feec0fe664e16fac0fde61cf64b401b9b0575323) )
ROM_END

/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Space Harrier
    CPU: 68000 + i8751 (315-5163A)

     ASSY CPU BD 834-5797
     ASSY CONTROL BD 834-5798
     ASSY ROM BD 834-5800
     ASSY SOUND BD 834-5670-01
*/
ROM_START( sharrier )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-7188a.ic97", 0x000000, 0x8000, CRC(45e173c3) SHA1(cbab555c5053f3e4a3f75ff78c41528e2d9d34c7) )
	ROM_LOAD16_BYTE( "epr-7184a.ic84", 0x000001, 0x8000, CRC(e1934a51) SHA1(67817a360b3f1f6c2440986272975bd696a38e70) )
	ROM_LOAD16_BYTE( "epr-7189.ic98",  0x010000, 0x8000, CRC(40b1309f) SHA1(9b050983f043a88f414745d02c912b59bbf1b121) )
	ROM_LOAD16_BYTE( "epr-7185.ic85",  0x010001, 0x8000, CRC(ce78045c) SHA1(ce640f05bed64ff5b47f1064b5fc13e58bc422a4) )
	ROM_LOAD16_BYTE( "epr-7190.ic99",  0x020000, 0x8000, CRC(f6391091) SHA1(3160b342b6447cccf67c932c7c1a42354cdfb058) )
	ROM_LOAD16_BYTE( "epr-7186.ic86",  0x020001, 0x8000, CRC(79b367d7) SHA1(e901036b1b9fac460415d513837c8f852f7750b0) )
	ROM_LOAD16_BYTE( "epr-7191.ic100", 0x030000, 0x8000, CRC(6171e9d3) SHA1(72f8736f421dc93139859fd47f0c8c3c32b6ff0b) )
	ROM_LOAD16_BYTE( "epr-7187.ic87",  0x030001, 0x8000, CRC(70cb72ef) SHA1(d1d89bd133b6905f81c25513d852b7e3a05a7312) )

	ROM_REGION( 0x40000, "sub", 0 ) /* second 68000 CPU */
	ROM_LOAD16_BYTE( "epr-7182.ic54", 0x0000, 0x8000, CRC(d7c535b6) SHA1(c0659a678c0c3776387a4a675016e9a2e9c67ee3) )
	ROM_LOAD16_BYTE( "epr-7183.ic67", 0x0001, 0x8000, CRC(a6153af8) SHA1(b56ba472e4afb474c7a3f7dc11d7428ebbe1a9c7) )

	ROM_REGION( 0x18000, "gfx1", 0 ) /* tiles */
	ROM_LOAD( "epr-7196.ic31", 0x00000, 0x08000, CRC(347fa325) SHA1(9076b16de9598b52a75e5084651ee5a220b0e88b) )
	ROM_LOAD( "epr-7197.ic46", 0x08000, 0x08000, CRC(39d98bd1) SHA1(5aab91bdd08b0f1ea537cd43ccc2e82fd01dd031) )
	ROM_LOAD( "epr-7198.ic60", 0x10000, 0x08000, CRC(3da3ea6b) SHA1(9a6ce304a14e6ef0be41d867284a63b941f960fb) )

	ROM_REGION32_LE( 0x100000, "gfx2", 0 ) /* sprites */
	ROM_LOAD32_BYTE( "epr-7230.ic36", 0x00000, 0x8000, CRC(93e2d264) SHA1(ca56de13756ab77408506d88f291da1da8134435) )
	ROM_LOAD32_BYTE( "epr-7222.ic28", 0x00001, 0x8000, CRC(edbf5fc3) SHA1(a93f8c431075741c181eb422b24c9303487ca16c) )
	ROM_LOAD32_BYTE( "epr-7214.ic18", 0x00002, 0x8000, CRC(e8c537d8) SHA1(c9b3c0f33272c47d32e6aa349d72f7e355468e0e) )
	ROM_LOAD32_BYTE( "epr-7206.ic8",  0x00003, 0x8000, CRC(22844fa4) SHA1(6d0f177082084c8c92085cd53e1d7ddc62d09a4c) )
	ROM_LOAD32_BYTE( "epr-7229.ic35", 0x20000, 0x8000, CRC(cd6e7500) SHA1(6f0e42eb28ad3f5df93d8bb39dc41aba66eee144) )
	ROM_LOAD32_BYTE( "epr-7221.ic27", 0x20001, 0x8000, CRC(41f25a9c) SHA1(bebbd4c4600028205aeff54190b32b664e4710d0) )
	ROM_LOAD32_BYTE( "epr-7213.ic17", 0x20002, 0x8000, CRC(5bb09a67) SHA1(8bedefd2fa29a1a5e36f1d81c4e9067e3c7f28e9) )
	ROM_LOAD32_BYTE( "epr-7205.ic7",  0x20003, 0x8000, CRC(dcaa2ebf) SHA1(9cf77bb966859febc5e5f1447cb719db64aa4db4) )
	ROM_LOAD32_BYTE( "epr-7228.ic34", 0x40000, 0x8000, CRC(d5e15e66) SHA1(2bd81c5c736d725577adf85532de7802bef057f2) )
	ROM_LOAD32_BYTE( "epr-7220.ic26", 0x40001, 0x8000, CRC(ac62ae2e) SHA1(d472dcc1d9b7889d04870920d5c6e5578597b8dc) )
	ROM_LOAD32_BYTE( "epr-7212.ic16", 0x40002, 0x8000, CRC(9c782295) SHA1(c1627f3d849d2fdb02a590502bafbe133212b943) )
	ROM_LOAD32_BYTE( "epr-7204.ic6",  0x40003, 0x8000, CRC(3711105c) SHA1(075197f614786f89bee28ed944371223bc75c9be) )
	ROM_LOAD32_BYTE( "epr-7227.ic33", 0x60000, 0x8000, CRC(60d7c1bb) SHA1(19ddc1d8f269b50343266d508ad04d4c0fff69d1) )
	ROM_LOAD32_BYTE( "epr-7219.ic25", 0x60001, 0x8000, CRC(f6330038) SHA1(805d4ed664972c0773c837d62f094858c1804148) )
	ROM_LOAD32_BYTE( "epr-7211.ic15", 0x60002, 0x8000, CRC(60737b98) SHA1(5e91498bc473f099a1b2887baf980486e922af97) )
	ROM_LOAD32_BYTE( "epr-7203.ic5",  0x60003, 0x8000, CRC(70fb5ebb) SHA1(38755a9be92865d2c5da8112d8d9c0fe8e778cff) )
	ROM_LOAD32_BYTE( "epr-7226.ic32", 0x80000, 0x8000, CRC(6d7b5c97) SHA1(94c27e4ef1e197ee136f9399b07520cae00a366f) )
	ROM_LOAD32_BYTE( "epr-7218.ic24", 0x80001, 0x8000, CRC(cebf797c) SHA1(d3d5aeba1a0e70a322ec86b930814fa8bc744829) )
	ROM_LOAD32_BYTE( "epr-7210.ic14", 0x80002, 0x8000, CRC(24596a8b) SHA1(f580022c4f7dcaefb7209058c310a329479b5fcd) )
	ROM_LOAD32_BYTE( "epr-7202.ic4",  0x80003, 0x8000, CRC(b537d082) SHA1(f87a644d9af8477c9eb94e5d3aeb5f6376264418) )
	ROM_LOAD32_BYTE( "epr-7225.ic31", 0xa0000, 0x8000, CRC(5e784271) SHA1(063bd83b7f42dec556a7bdf736cb51456ba7184b) )
	ROM_LOAD32_BYTE( "epr-7217.ic23", 0xa0001, 0x8000, CRC(510e5e10) SHA1(47b9f1bc8df0690c37d1d045bd361f8599e8a903) )
	ROM_LOAD32_BYTE( "epr-7209.ic13", 0xa0002, 0x8000, CRC(7a2dad15) SHA1(227865447027f8669aa0d06d059f3d61da6d59dd) )
	ROM_LOAD32_BYTE( "epr-7201.ic3",  0xa0003, 0x8000, CRC(f5ba4e08) SHA1(443b07c996cbb213fe57dfdd3879c1d4da27c001) )
	ROM_LOAD32_BYTE( "epr-7224.ic30", 0xc0000, 0x8000, CRC(ec42c9ef) SHA1(313d908f3a964529b18e09825552879817a2e159) )
	ROM_LOAD32_BYTE( "epr-7216.ic22", 0xc0001, 0x8000, CRC(6d4a7d7a) SHA1(997ac38e47d84f0166ca3ece50ac5f2d3435d8d3) )
	ROM_LOAD32_BYTE( "epr-7208.ic12", 0xc0002, 0x8000, CRC(0f732717) SHA1(6a19c88d5d52f4ec4adb32c511fed4caae81c65f) )
	ROM_LOAD32_BYTE( "epr-7200.ic2",  0xc0003, 0x8000, CRC(fc3bf8f3) SHA1(d9168b9bce110bfd531410179a107895c641e105) )
	ROM_LOAD32_BYTE( "epr-7223.ic29", 0xe0000, 0x8000, CRC(ed51fdc4) SHA1(a2696b15a0911ac3b6b330b7d8df58ca51d629de) )
	ROM_LOAD32_BYTE( "epr-7215.ic21", 0xe0001, 0x8000, CRC(dfe75f3d) SHA1(ff908419066494bc28cbd6afe72bd30350b20c4b) )
	ROM_LOAD32_BYTE( "epr-7207.ic11", 0xe0002, 0x8000, CRC(a2c07741) SHA1(747c029ab399c4110dbe360b8913f5c2e57c87cc) )
	ROM_LOAD32_BYTE( "epr-7199.ic1",  0xe0003, 0x8000, CRC(b191e22f) SHA1(406c7f4eed0b8fe93fa0bef370e496894f4d46a4) )

	ROM_REGION( 0x8000, "gfx3", 0 ) /* road gfx */
	ROM_LOAD( "epr-7181.ic2", 0x0000, 0x8000, CRC(b4740419) SHA1(8ece2dc85692e32d0ba0b427c260c3d10ac0b7cc) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* sound CPU */
	ROM_LOAD( "epr-7234.ic73", 0x00000, 0x004000, CRC(d6397933) SHA1(b85bb47efb6c113b3676b10ab86f1798a89d45b4) )
	ROM_LOAD( "epr-7233.ic72", 0x04000, 0x004000, CRC(504e76d9) SHA1(302af9101da01c97ca4be6acd21fb5b8e8f0b7ef) )

	ROM_REGION( 0x10000, "pcm", 0 ) /* Sega PCM sound data */
	ROM_LOAD( "epr-7231.ic5", 0x00000, 0x8000, CRC(871c6b14) SHA1(6d04ddc32fdf1db409cb519890821bd10fc9e58b) )
	ROM_LOAD( "epr-7232.ic6", 0x08000, 0x8000, CRC(4b59340c) SHA1(a01ba8580b65dd17bfd92560265e502d95d3ff16) )

	ROM_REGION( 0x10000, "mcu", 0 )	/* Internal i8751 MCU code */
	ROM_LOAD( "315-5163a.ic32", 0x00000, 0x1000, NO_DUMP )

	ROM_REGION( 0x2000, "proms", 0 ) /* zoom table */
	ROM_LOAD( "epr-6844.ic123", 0x0000, 0x2000, CRC(e3ec7bd6) SHA1(feec0fe664e16fac0fde61cf64b401b9b0575323) )
ROM_END
/**************************************************************************************************************************
    Space Harrier
    CPU: 68000 + i8751 (315-5163)

     ASSY CPU BD 834-5797
     ASSY CONTROL BD 834-5798
     ASSY ROM BD 834-5800
     ASSY SOUND BD 834-5670-01
*/
ROM_START( sharrier1 )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-7188.ic97",  0x000000, 0x8000, CRC(7c30a036) SHA1(d3902342be714b4e181c87ad2bad7102e3eeec20) )
	ROM_LOAD16_BYTE( "epr-7184.ic84",  0x000001, 0x8000, CRC(16deaeb1) SHA1(bdf85b924a914865bf876eda7fc2b20131a4cf2d) )
	ROM_LOAD16_BYTE( "epr-7189.ic98",  0x010000, 0x8000, CRC(40b1309f) SHA1(9b050983f043a88f414745d02c912b59bbf1b121) )
	ROM_LOAD16_BYTE( "epr-7185.ic85",  0x010001, 0x8000, CRC(ce78045c) SHA1(ce640f05bed64ff5b47f1064b5fc13e58bc422a4) )
	ROM_LOAD16_BYTE( "epr-7190.ic99",  0x020000, 0x8000, CRC(f6391091) SHA1(3160b342b6447cccf67c932c7c1a42354cdfb058) )
	ROM_LOAD16_BYTE( "epr-7186.ic86",  0x020001, 0x8000, CRC(79b367d7) SHA1(e901036b1b9fac460415d513837c8f852f7750b0) )
	ROM_LOAD16_BYTE( "epr-7191.ic100", 0x030000, 0x8000, CRC(6171e9d3) SHA1(72f8736f421dc93139859fd47f0c8c3c32b6ff0b) )
	ROM_LOAD16_BYTE( "epr-7187.ic87",  0x030001, 0x8000, CRC(70cb72ef) SHA1(d1d89bd133b6905f81c25513d852b7e3a05a7312) )

	ROM_REGION( 0x40000, "sub", 0 ) /* second 68000 CPU */
	ROM_LOAD16_BYTE( "epr-7182.ic54", 0x0000, 0x8000, CRC(d7c535b6) SHA1(c0659a678c0c3776387a4a675016e9a2e9c67ee3) )
	ROM_LOAD16_BYTE( "epr-7183.ic67", 0x0001, 0x8000, CRC(a6153af8) SHA1(b56ba472e4afb474c7a3f7dc11d7428ebbe1a9c7) )

	ROM_REGION( 0x18000, "gfx1", 0 ) /* tiles */
	ROM_LOAD( "epr-7196.ic31", 0x00000, 0x08000, CRC(347fa325) SHA1(9076b16de9598b52a75e5084651ee5a220b0e88b) )
	ROM_LOAD( "epr-7197.ic46", 0x08000, 0x08000, CRC(39d98bd1) SHA1(5aab91bdd08b0f1ea537cd43ccc2e82fd01dd031) )
	ROM_LOAD( "epr-7198.ic60", 0x10000, 0x08000, CRC(3da3ea6b) SHA1(9a6ce304a14e6ef0be41d867284a63b941f960fb) )

	ROM_REGION32_LE( 0x100000, "gfx2", 0 ) /* sprites */
	ROM_LOAD32_BYTE( "epr-7230.ic36", 0x00000, 0x8000, CRC(93e2d264) SHA1(ca56de13756ab77408506d88f291da1da8134435) )
	ROM_LOAD32_BYTE( "epr-7222.ic28", 0x00001, 0x8000, CRC(edbf5fc3) SHA1(a93f8c431075741c181eb422b24c9303487ca16c) )
	ROM_LOAD32_BYTE( "epr-7214.ic18", 0x00002, 0x8000, CRC(e8c537d8) SHA1(c9b3c0f33272c47d32e6aa349d72f7e355468e0e) )
	ROM_LOAD32_BYTE( "epr-7206.ic8",  0x00003, 0x8000, CRC(22844fa4) SHA1(6d0f177082084c8c92085cd53e1d7ddc62d09a4c) )
	ROM_LOAD32_BYTE( "epr-7229.ic35", 0x20000, 0x8000, CRC(cd6e7500) SHA1(6f0e42eb28ad3f5df93d8bb39dc41aba66eee144) )
	ROM_LOAD32_BYTE( "epr-7221.ic27", 0x20001, 0x8000, CRC(41f25a9c) SHA1(bebbd4c4600028205aeff54190b32b664e4710d0) )
	ROM_LOAD32_BYTE( "epr-7213.ic17", 0x20002, 0x8000, CRC(5bb09a67) SHA1(8bedefd2fa29a1a5e36f1d81c4e9067e3c7f28e9) )
	ROM_LOAD32_BYTE( "epr-7205.ic7",  0x20003, 0x8000, CRC(dcaa2ebf) SHA1(9cf77bb966859febc5e5f1447cb719db64aa4db4) )
	ROM_LOAD32_BYTE( "epr-7228.ic34", 0x40000, 0x8000, CRC(d5e15e66) SHA1(2bd81c5c736d725577adf85532de7802bef057f2) )
	ROM_LOAD32_BYTE( "epr-7220.ic26", 0x40001, 0x8000, CRC(ac62ae2e) SHA1(d472dcc1d9b7889d04870920d5c6e5578597b8dc) )
	ROM_LOAD32_BYTE( "epr-7212.ic16", 0x40002, 0x8000, CRC(9c782295) SHA1(c1627f3d849d2fdb02a590502bafbe133212b943) )
	ROM_LOAD32_BYTE( "epr-7204.ic6",  0x40003, 0x8000, CRC(3711105c) SHA1(075197f614786f89bee28ed944371223bc75c9be) )
	ROM_LOAD32_BYTE( "epr-7227.ic33", 0x60000, 0x8000, CRC(60d7c1bb) SHA1(19ddc1d8f269b50343266d508ad04d4c0fff69d1) )
	ROM_LOAD32_BYTE( "epr-7219.ic25", 0x60001, 0x8000, CRC(f6330038) SHA1(805d4ed664972c0773c837d62f094858c1804148) )
	ROM_LOAD32_BYTE( "epr-7211.ic15", 0x60002, 0x8000, CRC(60737b98) SHA1(5e91498bc473f099a1b2887baf980486e922af97) )
	ROM_LOAD32_BYTE( "epr-7203.ic5",  0x60003, 0x8000, CRC(70fb5ebb) SHA1(38755a9be92865d2c5da8112d8d9c0fe8e778cff) )
	ROM_LOAD32_BYTE( "epr-7226.ic32", 0x80000, 0x8000, CRC(6d7b5c97) SHA1(94c27e4ef1e197ee136f9399b07520cae00a366f) )
	ROM_LOAD32_BYTE( "epr-7218.ic24", 0x80001, 0x8000, CRC(cebf797c) SHA1(d3d5aeba1a0e70a322ec86b930814fa8bc744829) )
	ROM_LOAD32_BYTE( "epr-7210.ic14", 0x80002, 0x8000, CRC(24596a8b) SHA1(f580022c4f7dcaefb7209058c310a329479b5fcd) )
	ROM_LOAD32_BYTE( "epr-7202.ic4",  0x80003, 0x8000, CRC(b537d082) SHA1(f87a644d9af8477c9eb94e5d3aeb5f6376264418) )
	ROM_LOAD32_BYTE( "epr-7225.ic31", 0xa0000, 0x8000, CRC(5e784271) SHA1(063bd83b7f42dec556a7bdf736cb51456ba7184b) )
	ROM_LOAD32_BYTE( "epr-7217.ic23", 0xa0001, 0x8000, CRC(510e5e10) SHA1(47b9f1bc8df0690c37d1d045bd361f8599e8a903) )
	ROM_LOAD32_BYTE( "epr-7209.ic13", 0xa0002, 0x8000, CRC(7a2dad15) SHA1(227865447027f8669aa0d06d059f3d61da6d59dd) )
	ROM_LOAD32_BYTE( "epr-7201.ic3",  0xa0003, 0x8000, CRC(f5ba4e08) SHA1(443b07c996cbb213fe57dfdd3879c1d4da27c001) )
	ROM_LOAD32_BYTE( "epr-7224.ic30", 0xc0000, 0x8000, CRC(ec42c9ef) SHA1(313d908f3a964529b18e09825552879817a2e159) )
	ROM_LOAD32_BYTE( "epr-7216.ic22", 0xc0001, 0x8000, CRC(6d4a7d7a) SHA1(997ac38e47d84f0166ca3ece50ac5f2d3435d8d3) )
	ROM_LOAD32_BYTE( "epr-7208.ic12", 0xc0002, 0x8000, CRC(0f732717) SHA1(6a19c88d5d52f4ec4adb32c511fed4caae81c65f) )
	ROM_LOAD32_BYTE( "epr-7200.ic2",  0xc0003, 0x8000, CRC(fc3bf8f3) SHA1(d9168b9bce110bfd531410179a107895c641e105) )
	ROM_LOAD32_BYTE( "epr-7223.ic29", 0xe0000, 0x8000, CRC(ed51fdc4) SHA1(a2696b15a0911ac3b6b330b7d8df58ca51d629de) )
	ROM_LOAD32_BYTE( "epr-7215.ic21", 0xe0001, 0x8000, CRC(dfe75f3d) SHA1(ff908419066494bc28cbd6afe72bd30350b20c4b) )
	ROM_LOAD32_BYTE( "epr-7207.ic11", 0xe0002, 0x8000, CRC(a2c07741) SHA1(747c029ab399c4110dbe360b8913f5c2e57c87cc) )
	ROM_LOAD32_BYTE( "epr-7199.ic1",  0xe0003, 0x8000, CRC(b191e22f) SHA1(406c7f4eed0b8fe93fa0bef370e496894f4d46a4) )

	ROM_REGION( 0x8000, "gfx3", 0 ) /* road gfx */
	ROM_LOAD( "epr-7181.ic2", 0x0000, 0x8000, CRC(b4740419) SHA1(8ece2dc85692e32d0ba0b427c260c3d10ac0b7cc) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* sound CPU */
	ROM_LOAD( "epr-7234.ic73", 0x00000, 0x004000, CRC(d6397933) SHA1(b85bb47efb6c113b3676b10ab86f1798a89d45b4) )
	ROM_LOAD( "epr-7233.ic72", 0x04000, 0x004000, CRC(504e76d9) SHA1(302af9101da01c97ca4be6acd21fb5b8e8f0b7ef) )

	ROM_REGION( 0x10000, "pcm", 0 ) /* Sega PCM sound data */
	ROM_LOAD( "epr-7231.ic5", 0x00000, 0x8000, CRC(871c6b14) SHA1(6d04ddc32fdf1db409cb519890821bd10fc9e58b) )
	ROM_LOAD( "epr-7232.ic6", 0x08000, 0x8000, CRC(4b59340c) SHA1(a01ba8580b65dd17bfd92560265e502d95d3ff16) )

	ROM_REGION( 0x10000, "mcu", 0 )	/* Internal i8751 MCU code */
	ROM_LOAD( "315-5163.ic32", 0x00000, 0x1000, NO_DUMP )

	ROM_REGION( 0x2000, "proms", 0 ) /* zoom table */
	ROM_LOAD( "epr-6844.ic123", 0x0000, 0x2000, CRC(e3ec7bd6) SHA1(feec0fe664e16fac0fde61cf64b401b9b0575323) )
ROM_END


/**************************************************************************************************************************
 **************************************************************************************************************************
 **************************************************************************************************************************
    Enduro Racer
    CPU: FD1089B (317-0013A)
    YM2151 sound board

     ASSY CPU BD 837-6001-01
     ASSY CONTROL BD 837-6002-01
     ASSY ROM BD 837-6004-01
     ASSY SOUND BD 837-6003-01? (not verified)
*/
ROM_START( enduror )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-7640a.ic97", 0x00000, 0x8000, CRC(1d1dc5d4) SHA1(8e7ae5abd23e949de5d5e1772f90e53d05c866ec) )
	ROM_LOAD16_BYTE( "epr-7636a.ic84", 0x00001, 0x8000, CRC(84131639) SHA1(04981464577d2604eec36c14c5de9c91604ae501) )
	ROM_LOAD16_BYTE( "epr-7641.ic98",  0x10000, 0x8000, CRC(2503ae7c) SHA1(27009d5b47dc207145048edfcc1ac8ffda5f0b78) )
	ROM_LOAD16_BYTE( "epr-7637.ic85",  0x10001, 0x8000, CRC(82a27a8c) SHA1(4b182d8c23454aed7d786c9824932957319b6eff) )
	ROM_LOAD16_BYTE( "epr-7642.ic99",  0x20000, 0x8000, CRC(1c453bea) SHA1(c6e606cdcb1690de05ef5283b48a8a61b2e0ad51) )
	ROM_LOAD16_BYTE( "epr-7638.ic86",  0x20001, 0x8000, CRC(70544779) SHA1(e6403edd7fc0ad5d447c25be5d7f10889aa109ff) )

	ROM_REGION( 0x40000, "sub", 0 ) /* second 68000 CPU */
	ROM_LOAD16_BYTE("epr-7634a.ic54", 0x0000, 0x8000, CRC(aec83731) SHA1(3fe2d0f1a8806b850836741d664c07754a701459) )
	ROM_LOAD16_BYTE("epr-7635a.ic67", 0x0001, 0x8000, CRC(b2fce96f) SHA1(9d6c1a7c2bdbf86430b849a5f6c6fdb5595dc91c) )

	ROM_REGION( 0x18000, "gfx1", 0 ) /* tiles */
	ROM_LOAD( "epr-7644.ic31", 0x00000, 0x08000, CRC(e7a4ff90) SHA1(06d18470019041e32be9a969870cd995de626cd6) )
	ROM_LOAD( "epr-7645.ic46", 0x08000, 0x08000, CRC(4caa0095) SHA1(a24c741cdca0542e462f17ff94f132c62710e198) )
	ROM_LOAD( "epr-7646.ic60", 0x10000, 0x08000, CRC(7e432683) SHA1(c8249b23fce77eb456166161c2d9aa34309efe31) )

	ROM_REGION32_LE( 0x100000, "gfx2", 0 ) /* sprites */
	ROM_LOAD32_BYTE( "epr-7678.ic36", 0x00000, 0x8000, CRC(9fb5e656) SHA1(264b0ad017eb0fc7e0b542e6dd160ba964c100fd) )
	ROM_LOAD32_BYTE( "epr-7670.ic28", 0x00001, 0x8000, CRC(dbbe2f6e) SHA1(310797a61f91d6866e728e0da3b30828e06d1b52) )
	ROM_LOAD32_BYTE( "epr-7662.ic18", 0x00002, 0x8000, CRC(cb0c13c5) SHA1(856d1234fd8f8146e20fe6c65c0a535b7b7512cd) )
	ROM_LOAD32_BYTE( "epr-7654.1c8",  0x00003, 0x8000, CRC(2db6520d) SHA1(d16739e84316b4bd26963b729208169bbf01f499) )
	ROM_LOAD32_BYTE( "epr-7677.ic35", 0x20000, 0x8000, CRC(7764765b) SHA1(62543130816c084d292f229a15b3ce1305c99bb3) )
	ROM_LOAD32_BYTE( "epr-7669.ic27", 0x20001, 0x8000, CRC(f9525faa) SHA1(fbe2f67a9baee069dbca26a669d0a263bcca0d09) )
	ROM_LOAD32_BYTE( "epr-7661.ic17", 0x20002, 0x8000, CRC(fe93a79b) SHA1(591025a371a451c9cddc8c7480c9841a18bb9a7f) )
	ROM_LOAD32_BYTE( "epr-7653.ic7",  0x20003, 0x8000, CRC(46a52114) SHA1(d646ab03c1985953401619457d03072833edc6c7) )
	ROM_LOAD32_BYTE( "epr-7676.ic34", 0x40000, 0x8000, CRC(2e42e0d4) SHA1(508f6f89e681b59272884ba129a5c6ffa1b6ba05) )
	ROM_LOAD32_BYTE( "epr-7668.ic26", 0x40001, 0x8000, CRC(e115ce33) SHA1(1af591bc1567b89d0de399e4a02d896fba938bab) )
	ROM_LOAD32_BYTE( "epr-7660.ic16", 0x40002, 0x8000, CRC(86dfbb68) SHA1(a05ac16fbe3aaf34dd46229d4b71fc1f72a3a556) )
	ROM_LOAD32_BYTE( "epr-7652.ic6",  0x40003, 0x8000, CRC(2880cfdb) SHA1(94b78d78d82c324ca108970d8689f1d6b2ca8a24) )
	ROM_LOAD32_BYTE( "epr-7675.ic33", 0x60000, 0x8000, CRC(05cd2d61) SHA1(51688a5a9bc4da3f88ce162ff30affe8c6d3d0c8) )
	ROM_LOAD32_BYTE( "epr-7667.ic25", 0x60001, 0x8000, CRC(923bde9d) SHA1(7722a7fdbf45f862f1011d1afae8dedd5885bf52) )
	ROM_LOAD32_BYTE( "epr-7659.ic15", 0x60002, 0x8000, CRC(629dc8ce) SHA1(4af2a53678890b02922dee54f7cd3c5550001572) )
	ROM_LOAD32_BYTE( "epr-7651.ic5",  0x60003, 0x8000, CRC(d7902bad) SHA1(f4872d1a42dcf7d5dbdbc1233606a706b39478d7) )
	ROM_LOAD32_BYTE( "epr-7674.ic32", 0x80000, 0x8000, CRC(1a129acf) SHA1(ebaa60ccedc95c58af3ce99105b924b303827f6e) )
	ROM_LOAD32_BYTE( "epr-7666.ic24", 0x80001, 0x8000, CRC(23697257) SHA1(19453b14e8e6789e4c48a80d1b83dbaf37fbdceb) )
	ROM_LOAD32_BYTE( "epr-7658.ic14", 0x80002, 0x8000, CRC(1677f24f) SHA1(4786996cc8a04344e82ec9be7c4e7c8a005914a3) )
	ROM_LOAD32_BYTE( "epr-7650.ic4",  0x80003, 0x8000, CRC(642635ec) SHA1(e42bbae178e9a139325633e8c85a606c91e39e36) )
	ROM_LOAD32_BYTE( "epr-7673.ic31", 0xa0000, 0x8000, CRC(82602394) SHA1(d714f397f33a52429f394fc4c403d39be7911ccf) )
	ROM_LOAD32_BYTE( "epr-7665.ic23", 0xa0001, 0x8000, CRC(12d77607) SHA1(5b5d25646336a8ceae449d5b7a6b70372d81dd8b) )
	ROM_LOAD32_BYTE( "epr-7657.ic13", 0xa0002, 0x8000, CRC(8158839c) SHA1(f22081caf11d6b57488c969b5935cd4686e11197) )
	ROM_LOAD32_BYTE( "epr-7649.ic3",  0xa0003, 0x8000, CRC(4edba14c) SHA1(db0aab94de50f8f9501b7afd2fff70fb0a6b2b36) )
	ROM_LOAD32_BYTE( "epr-7672.ic30", 0xc0000, 0x8000, CRC(d11452f7) SHA1(f68183053005a26c0014050592bad6d63325895e) )
	ROM_LOAD32_BYTE( "epr-7664.ic22", 0xc0001, 0x8000, CRC(0df2cfad) SHA1(d62d12922be921967da37fbc624aaed72c4a2a98) )
	ROM_LOAD32_BYTE( "epr-7656.ic12", 0xc0002, 0x8000, CRC(6c741272) SHA1(ccaedda1436ddc339377e610d51e13726bb6c7eb) )
	ROM_LOAD32_BYTE( "epr-7648.ic2",  0xc0003, 0x8000, CRC(983ea830) SHA1(9629476a300ba711893775ca94dce81a00afd246) )
	ROM_LOAD32_BYTE( "epr-7671.ic29", 0xe0000, 0x8000, CRC(b0c7fdc6) SHA1(c9e0993fed36526e0e46ab2da9413af24b96cae8) )
	ROM_LOAD32_BYTE( "epr-7663.ic21", 0xe0001, 0x8000, CRC(2b0b8f08) SHA1(14aa1e6866f1c23c9ff271e8f216f6ecc21601ab) )
	ROM_LOAD32_BYTE( "epr-7655.ic11", 0xe0002, 0x8000, CRC(3433fe7b) SHA1(636449a0707d6629bf6ea503cfb52ad24af1c017) )
	ROM_LOAD32_BYTE( "epr-7647.ic1",  0xe0003, 0x8000, CRC(2e7fbec0) SHA1(a59ec5fc3341833671fb948cd21b47f3a49db538) )

	ROM_REGION( 0x8000, "gfx3", 0 ) /* road gfx */
	ROM_LOAD( "epr-7633.ic1", 0x0000, 0x8000, CRC(6f146210) SHA1(2f58f0c3563b434ed02700b9ca1545a696a5716e) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* sound CPU */
	ROM_LOAD( "epr-7682.rom", 0x00000, 0x8000, CRC(c4efbf48) SHA1(2bcbc4757d98f291fcaec467abc36158b3f59be3) )

	ROM_REGION( 0x20000, "pcm", 0 ) /* Sega PCM sound data */
	ROM_LOAD( "epr-7681.rom", 0x00000, 0x8000, CRC(bc0c4d12) SHA1(3de71bde4c23e3c31984f20fc4bc7e221354c56f) )
	ROM_LOAD( "epr-7680.rom", 0x10000, 0x8000, CRC(627b3c8c) SHA1(806fe7dce619ad19c09178061be4607d2beba14d) )

	ROM_REGION( 0x2000, "proms", 0 ) /* zoom table */
	ROM_LOAD( "epr-6844.ic123", 0x0000, 0x2000, CRC(e3ec7bd6) SHA1(feec0fe664e16fac0fde61cf64b401b9b0575323) )

	ROM_REGION( 0x2000, "fd1089b", 0 ) /* decryption key */
	ROM_LOAD( "317-0013a.key", 0x0000, 0x2000, CRC(295e6737) SHA1(2eff36f1f24db1154cf970d4c9fd481ae4f9a57c) )
ROM_END

/**************************************************************************************************************************
    Enduro Racer
    CPU: FD1089B (317-0013A)
    YM2203 sound board

     ASSY CPU BD 837-6001-01
     ASSY CONTROL BD 837-6002-01
     ASSY ROM BD 837-6004-01
     ASSY SOUND BD 837-6005-01 (renumbered 834-5670)
*/
ROM_START( enduror1 )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-7630.ic97", 0x00000, 0x8000, CRC(a1bdadab) SHA1(f52d747a6947ad2dbc12765133adfb41eb5a5f2f) )
	ROM_LOAD16_BYTE( "epr-7629.ic84", 0x00001, 0x8000, CRC(f50f4169) SHA1(b4eebb5131bb472db03f0e340743437753a9efe3) )
	ROM_LOAD16_BYTE( "epr-7641.ic98", 0x10000, 0x8000, CRC(2503ae7c) SHA1(27009d5b47dc207145048edfcc1ac8ffda5f0b78) )
	ROM_LOAD16_BYTE( "epr-7637.ic85", 0x10001, 0x8000, CRC(82a27a8c) SHA1(4b182d8c23454aed7d786c9824932957319b6eff) )
	ROM_LOAD16_BYTE( "epr-7642.ic99", 0x20000, 0x8000, CRC(1c453bea) SHA1(c6e606cdcb1690de05ef5283b48a8a61b2e0ad51) )
	ROM_LOAD16_BYTE( "epr-7638.ic86", 0x20001, 0x8000, CRC(70544779) SHA1(e6403edd7fc0ad5d447c25be5d7f10889aa109ff) )

	ROM_REGION( 0x40000, "sub", 0 ) /* second 68000 CPU */
	ROM_LOAD16_BYTE("epr-7634.ic54", 0x0000, 0x8000, CRC(3e07fd32) SHA1(7acb9e9712ecfe928c421c84dece783e75077746) )
	ROM_LOAD16_BYTE("epr-7635.ic67", 0x0001, 0x8000, CRC(22f762ab) SHA1(70fa87da76c714db7213c42128a0b6a27644a1d4) )

	ROM_REGION( 0x18000, "gfx1", 0 ) /* tiles */
	ROM_LOAD( "epr-7644.ic31", 0x00000, 0x08000, CRC(e7a4ff90) SHA1(06d18470019041e32be9a969870cd995de626cd6) )
	ROM_LOAD( "epr-7645.ic46", 0x08000, 0x08000, CRC(4caa0095) SHA1(a24c741cdca0542e462f17ff94f132c62710e198) )
	ROM_LOAD( "epr-7646.ic60", 0x10000, 0x08000, CRC(7e432683) SHA1(c8249b23fce77eb456166161c2d9aa34309efe31) )

	ROM_REGION32_LE( 0x100000, "gfx2", 0 ) /* sprites */
	ROM_LOAD32_BYTE( "epr-7678.ic36", 0x00000, 0x8000, CRC(9fb5e656) SHA1(264b0ad017eb0fc7e0b542e6dd160ba964c100fd) )
	ROM_LOAD32_BYTE( "epr-7670.ic28", 0x00001, 0x8000, CRC(dbbe2f6e) SHA1(310797a61f91d6866e728e0da3b30828e06d1b52) )
	ROM_LOAD32_BYTE( "epr-7662.ic18", 0x00002, 0x8000, CRC(cb0c13c5) SHA1(856d1234fd8f8146e20fe6c65c0a535b7b7512cd) )
	ROM_LOAD32_BYTE( "epr-7654.1c8",  0x00003, 0x8000, CRC(2db6520d) SHA1(d16739e84316b4bd26963b729208169bbf01f499) )
	ROM_LOAD32_BYTE( "epr-7677.ic35", 0x20000, 0x8000, CRC(7764765b) SHA1(62543130816c084d292f229a15b3ce1305c99bb3) )
	ROM_LOAD32_BYTE( "epr-7669.ic27", 0x20001, 0x8000, CRC(f9525faa) SHA1(fbe2f67a9baee069dbca26a669d0a263bcca0d09) )
	ROM_LOAD32_BYTE( "epr-7661.ic17", 0x20002, 0x8000, CRC(fe93a79b) SHA1(591025a371a451c9cddc8c7480c9841a18bb9a7f) )
	ROM_LOAD32_BYTE( "epr-7653.ic7",  0x20003, 0x8000, CRC(46a52114) SHA1(d646ab03c1985953401619457d03072833edc6c7) )
	ROM_LOAD32_BYTE( "epr-7676.ic34", 0x40000, 0x8000, CRC(2e42e0d4) SHA1(508f6f89e681b59272884ba129a5c6ffa1b6ba05) )
	ROM_LOAD32_BYTE( "epr-7668.ic26", 0x40001, 0x8000, CRC(e115ce33) SHA1(1af591bc1567b89d0de399e4a02d896fba938bab) )
	ROM_LOAD32_BYTE( "epr-7660.ic16", 0x40002, 0x8000, CRC(86dfbb68) SHA1(a05ac16fbe3aaf34dd46229d4b71fc1f72a3a556) )
	ROM_LOAD32_BYTE( "epr-7652.ic6",  0x40003, 0x8000, CRC(2880cfdb) SHA1(94b78d78d82c324ca108970d8689f1d6b2ca8a24) )
	ROM_LOAD32_BYTE( "epr-7675.ic33", 0x60000, 0x8000, CRC(05cd2d61) SHA1(51688a5a9bc4da3f88ce162ff30affe8c6d3d0c8) )
	ROM_LOAD32_BYTE( "epr-7667.ic25", 0x60001, 0x8000, CRC(923bde9d) SHA1(7722a7fdbf45f862f1011d1afae8dedd5885bf52) )
	ROM_LOAD32_BYTE( "epr-7659.ic15", 0x60002, 0x8000, CRC(629dc8ce) SHA1(4af2a53678890b02922dee54f7cd3c5550001572) )
	ROM_LOAD32_BYTE( "epr-7651.ic5",  0x60003, 0x8000, CRC(d7902bad) SHA1(f4872d1a42dcf7d5dbdbc1233606a706b39478d7) )
	ROM_LOAD32_BYTE( "epr-7674.ic32", 0x80000, 0x8000, CRC(1a129acf) SHA1(ebaa60ccedc95c58af3ce99105b924b303827f6e) )
	ROM_LOAD32_BYTE( "epr-7666.ic24", 0x80001, 0x8000, CRC(23697257) SHA1(19453b14e8e6789e4c48a80d1b83dbaf37fbdceb) )
	ROM_LOAD32_BYTE( "epr-7658.ic14", 0x80002, 0x8000, CRC(1677f24f) SHA1(4786996cc8a04344e82ec9be7c4e7c8a005914a3) )
	ROM_LOAD32_BYTE( "epr-7650.ic4",  0x80003, 0x8000, CRC(642635ec) SHA1(e42bbae178e9a139325633e8c85a606c91e39e36) )
	ROM_LOAD32_BYTE( "epr-7673.ic31", 0xa0000, 0x8000, CRC(82602394) SHA1(d714f397f33a52429f394fc4c403d39be7911ccf) )
	ROM_LOAD32_BYTE( "epr-7665.ic23", 0xa0001, 0x8000, CRC(12d77607) SHA1(5b5d25646336a8ceae449d5b7a6b70372d81dd8b) )
	ROM_LOAD32_BYTE( "epr-7657.ic13", 0xa0002, 0x8000, CRC(8158839c) SHA1(f22081caf11d6b57488c969b5935cd4686e11197) )
	ROM_LOAD32_BYTE( "epr-7649.ic3",  0xa0003, 0x8000, CRC(4edba14c) SHA1(db0aab94de50f8f9501b7afd2fff70fb0a6b2b36) )
	ROM_LOAD32_BYTE( "epr-7672.ic30", 0xc0000, 0x8000, CRC(d11452f7) SHA1(f68183053005a26c0014050592bad6d63325895e) )
	ROM_LOAD32_BYTE( "epr-7664.ic22", 0xc0001, 0x8000, CRC(0df2cfad) SHA1(d62d12922be921967da37fbc624aaed72c4a2a98) )
	ROM_LOAD32_BYTE( "epr-7656.ic12", 0xc0002, 0x8000, CRC(6c741272) SHA1(ccaedda1436ddc339377e610d51e13726bb6c7eb) )
	ROM_LOAD32_BYTE( "epr-7648.ic2",  0xc0003, 0x8000, CRC(983ea830) SHA1(9629476a300ba711893775ca94dce81a00afd246) )
	ROM_LOAD32_BYTE( "epr-7671.ic29", 0xe0000, 0x8000, CRC(b0c7fdc6) SHA1(c9e0993fed36526e0e46ab2da9413af24b96cae8) )
	ROM_LOAD32_BYTE( "epr-7663.ic21", 0xe0001, 0x8000, CRC(2b0b8f08) SHA1(14aa1e6866f1c23c9ff271e8f216f6ecc21601ab) )
	ROM_LOAD32_BYTE( "epr-7655.ic11", 0xe0002, 0x8000, CRC(3433fe7b) SHA1(636449a0707d6629bf6ea503cfb52ad24af1c017) )
	ROM_LOAD32_BYTE( "epr-7647.ic1",  0xe0003, 0x8000, CRC(2e7fbec0) SHA1(a59ec5fc3341833671fb948cd21b47f3a49db538) )

	ROM_REGION( 0x8000, "gfx3", 0 ) /* road gfx */
	ROM_LOAD( "epr-7633.ic1", 0x0000, 0x8000, CRC(6f146210) SHA1(2f58f0c3563b434ed02700b9ca1545a696a5716e) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* sound CPU */
	ROM_LOAD( "epr-7765.ic73", 0x00000, 0x4000, CRC(81c82fc9) SHA1(99eae7edc62d719993c46a703f9daaf332e236e9) )
	ROM_LOAD( "epr-7764.ic72", 0x04000, 0x4000, CRC(755bfdad) SHA1(2942f3da5a45a3ac7bba6a73142663fd975f4379) )

	ROM_REGION( 0x20000, "pcm", 0 ) /* Sega PCM sound data */
	ROM_LOAD( "epr-7762.ic5", 0x00000, 0x8000, CRC(bc0c4d12) SHA1(3de71bde4c23e3c31984f20fc4bc7e221354c56f) )
	ROM_LOAD( "epr-7763.ic6", 0x08000, 0x8000, CRC(627b3c8c) SHA1(806fe7dce619ad19c09178061be4607d2beba14d) )

	ROM_REGION( 0x2000, "proms", 0 ) /* zoom table */
	ROM_LOAD( "epr-6844.ic123", 0x0000, 0x2000, CRC(e3ec7bd6) SHA1(feec0fe664e16fac0fde61cf64b401b9b0575323) )

	ROM_REGION( 0x2000, "fd1089b", 0 ) /* decryption key */
	ROM_LOAD( "317-0013a.key", 0x0000, 0x2000, CRC(295e6737) SHA1(2eff36f1f24db1154cf970d4c9fd481ae4f9a57c) )
ROM_END

/**************************************************************************************************************************
    Enduro Racer (bootleg)
    CPU: 68000
    YM2203 sound board
*/
ROM_START( endurobl )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "7.13j", 0x030000, 0x08000, CRC(f1d6b4b7) SHA1(32bd966191cbb36d1e60ed1a06d4caa023dd6b88) )
	ROM_CONTINUE(             0x000000, 0x08000 )
	ROM_LOAD16_BYTE( "4.13h", 0x030001, 0x08000, CRC(43bff873) SHA1(04e906c1965a6211fb8e13987db52f1f99cc0203) )	/* rom de-coded */
	ROM_CONTINUE(             0x000001, 0x08000 )		// data de-coded
	ROM_LOAD16_BYTE( "8.14j", 0x010000, 0x08000, CRC(2153154a) SHA1(145d8ed59812d26ca412a01ae77cd7872adaba5a) )
	ROM_LOAD16_BYTE( "5.14h", 0x010001, 0x08000, CRC(0a97992c) SHA1(7a6fc8c575637107ed07a30f6f0f8cb8877cbb43) )
	ROM_LOAD16_BYTE( "9.15j", 0x020000, 0x08000, CRC(db3bff1c) SHA1(343ed27a690800683cdd5128dcdb28c7b45288a3) )	/* one byte difference from */
	ROM_LOAD16_BYTE( "6.15h", 0x020001, 0x08000, CRC(54b1885a) SHA1(f53d906390e5414e73c4cdcbc102d3cb3e719e67) )	/* epr-7638.ic86 / epr-7642.ic99 */

	ROM_REGION( 0x40000, "sub", 0 ) /* second 68000 CPU */
	ROM_LOAD16_BYTE("epr-7634.ic54", 0x0000, 0x8000, CRC(3e07fd32) SHA1(7acb9e9712ecfe928c421c84dece783e75077746) )
	ROM_LOAD16_BYTE("epr-7635.ic67", 0x0001, 0x8000, CRC(22f762ab) SHA1(70fa87da76c714db7213c42128a0b6a27644a1d4) )

	ROM_REGION( 0x18000, "gfx1", 0 ) /* tiles */
	ROM_LOAD( "epr-7644.ic31", 0x00000, 0x08000, CRC(e7a4ff90) SHA1(06d18470019041e32be9a969870cd995de626cd6) )
	ROM_LOAD( "epr-7645.ic46", 0x08000, 0x08000, CRC(4caa0095) SHA1(a24c741cdca0542e462f17ff94f132c62710e198) )
	ROM_LOAD( "epr-7646.ic60", 0x10000, 0x08000, CRC(7e432683) SHA1(c8249b23fce77eb456166161c2d9aa34309efe31) )

	ROM_REGION32_LE( 0x100000, "gfx2", 0 ) /* sprites */
	ROM_LOAD32_BYTE( "epr-7678.ic36", 0x00000, 0x8000, CRC(9fb5e656) SHA1(264b0ad017eb0fc7e0b542e6dd160ba964c100fd) )
	ROM_LOAD32_BYTE( "epr-7670.ic28", 0x00001, 0x8000, CRC(dbbe2f6e) SHA1(310797a61f91d6866e728e0da3b30828e06d1b52) )
	ROM_LOAD32_BYTE( "epr-7662.ic18", 0x00002, 0x8000, CRC(cb0c13c5) SHA1(856d1234fd8f8146e20fe6c65c0a535b7b7512cd) )
	ROM_LOAD32_BYTE( "epr-7654.1c8",  0x00003, 0x8000, CRC(2db6520d) SHA1(d16739e84316b4bd26963b729208169bbf01f499) )
	ROM_LOAD32_BYTE( "epr-7677.ic35", 0x20000, 0x8000, CRC(7764765b) SHA1(62543130816c084d292f229a15b3ce1305c99bb3) )
	ROM_LOAD32_BYTE( "epr-7669.ic27", 0x20001, 0x8000, CRC(f9525faa) SHA1(fbe2f67a9baee069dbca26a669d0a263bcca0d09) )
	ROM_LOAD32_BYTE( "epr-7661.ic17", 0x20002, 0x8000, CRC(fe93a79b) SHA1(591025a371a451c9cddc8c7480c9841a18bb9a7f) )
	ROM_LOAD32_BYTE( "epr-7653.ic7",  0x20003, 0x8000, CRC(46a52114) SHA1(d646ab03c1985953401619457d03072833edc6c7) )
	ROM_LOAD32_BYTE( "epr-7676.ic34", 0x40000, 0x8000, CRC(2e42e0d4) SHA1(508f6f89e681b59272884ba129a5c6ffa1b6ba05) )
	ROM_LOAD32_BYTE( "epr-7668.ic26", 0x40001, 0x8000, CRC(e115ce33) SHA1(1af591bc1567b89d0de399e4a02d896fba938bab) )
	ROM_LOAD32_BYTE( "epr-7660.ic16", 0x40002, 0x8000, CRC(86dfbb68) SHA1(a05ac16fbe3aaf34dd46229d4b71fc1f72a3a556) )
	ROM_LOAD32_BYTE( "epr-7652.ic6",  0x40003, 0x8000, CRC(2880cfdb) SHA1(94b78d78d82c324ca108970d8689f1d6b2ca8a24) )
	ROM_LOAD32_BYTE( "epr-7675.ic33", 0x60000, 0x8000, CRC(05cd2d61) SHA1(51688a5a9bc4da3f88ce162ff30affe8c6d3d0c8) )
	ROM_LOAD32_BYTE( "epr-7667.ic25", 0x60001, 0x8000, CRC(923bde9d) SHA1(7722a7fdbf45f862f1011d1afae8dedd5885bf52) )
	ROM_LOAD32_BYTE( "epr-7659.ic15", 0x60002, 0x8000, CRC(629dc8ce) SHA1(4af2a53678890b02922dee54f7cd3c5550001572) )
	ROM_LOAD32_BYTE( "epr-7651.ic5",  0x60003, 0x8000, CRC(d7902bad) SHA1(f4872d1a42dcf7d5dbdbc1233606a706b39478d7) )
	ROM_LOAD32_BYTE( "epr-7674.ic32", 0x80000, 0x8000, CRC(1a129acf) SHA1(ebaa60ccedc95c58af3ce99105b924b303827f6e) )
	ROM_LOAD32_BYTE( "epr-7666.ic24", 0x80001, 0x8000, CRC(23697257) SHA1(19453b14e8e6789e4c48a80d1b83dbaf37fbdceb) )
	ROM_LOAD32_BYTE( "epr-7658.ic14", 0x80002, 0x8000, CRC(1677f24f) SHA1(4786996cc8a04344e82ec9be7c4e7c8a005914a3) )
	ROM_LOAD32_BYTE( "epr-7650.ic4",  0x80003, 0x8000, CRC(642635ec) SHA1(e42bbae178e9a139325633e8c85a606c91e39e36) )
	ROM_LOAD32_BYTE( "epr-7673.ic31", 0xa0000, 0x8000, CRC(82602394) SHA1(d714f397f33a52429f394fc4c403d39be7911ccf) )
	ROM_LOAD32_BYTE( "epr-7665.ic23", 0xa0001, 0x8000, CRC(12d77607) SHA1(5b5d25646336a8ceae449d5b7a6b70372d81dd8b) )
	ROM_LOAD32_BYTE( "epr-7657.ic13", 0xa0002, 0x8000, CRC(8158839c) SHA1(f22081caf11d6b57488c969b5935cd4686e11197) )
	ROM_LOAD32_BYTE( "epr-7649.ic3",  0xa0003, 0x8000, CRC(4edba14c) SHA1(db0aab94de50f8f9501b7afd2fff70fb0a6b2b36) )
	ROM_LOAD32_BYTE( "epr-7672.ic30", 0xc0000, 0x8000, CRC(d11452f7) SHA1(f68183053005a26c0014050592bad6d63325895e) )
	ROM_LOAD32_BYTE( "epr-7664.ic22", 0xc0001, 0x8000, CRC(0df2cfad) SHA1(d62d12922be921967da37fbc624aaed72c4a2a98) )
	ROM_LOAD32_BYTE( "epr-7656.ic12", 0xc0002, 0x8000, CRC(6c741272) SHA1(ccaedda1436ddc339377e610d51e13726bb6c7eb) )
	ROM_LOAD32_BYTE( "epr-7648.ic2",  0xc0003, 0x8000, CRC(983ea830) SHA1(9629476a300ba711893775ca94dce81a00afd246) )
	ROM_LOAD32_BYTE( "epr-7671.ic29", 0xe0000, 0x8000, CRC(b0c7fdc6) SHA1(c9e0993fed36526e0e46ab2da9413af24b96cae8) )
	ROM_LOAD32_BYTE( "epr-7663.ic21", 0xe0001, 0x8000, CRC(2b0b8f08) SHA1(14aa1e6866f1c23c9ff271e8f216f6ecc21601ab) )
	ROM_LOAD32_BYTE( "epr-7655.ic11", 0xe0002, 0x8000, CRC(3433fe7b) SHA1(636449a0707d6629bf6ea503cfb52ad24af1c017) )
	ROM_LOAD32_BYTE( "epr-7647.ic1",  0xe0003, 0x8000, CRC(2e7fbec0) SHA1(a59ec5fc3341833671fb948cd21b47f3a49db538) )

	ROM_REGION( 0x8000, "gfx3", 0 ) /* road gfx */
	ROM_LOAD( "epr-7633.ic1", 0x0000, 0x8000, CRC(6f146210) SHA1(2f58f0c3563b434ed02700b9ca1545a696a5716e) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* sound CPU */
	ROM_LOAD( "epr-7765.ic73", 0x00000, 0x4000, CRC(81c82fc9) SHA1(99eae7edc62d719993c46a703f9daaf332e236e9) )	/* was "13.16d" */
	ROM_LOAD( "epr-7764.ic72", 0x04000, 0x4000, CRC(755bfdad) SHA1(2942f3da5a45a3ac7bba6a73142663fd975f4379) )	/* was "12.16e" */

	ROM_REGION( 0x20000, "pcm", 0 ) /* Sega PCM sound data */
	ROM_LOAD( "epr-7762.ic5", 0x00000, 0x8000, CRC(bc0c4d12) SHA1(3de71bde4c23e3c31984f20fc4bc7e221354c56f) )
	ROM_LOAD( "epr-7763.ic6", 0x10000, 0x8000, CRC(627b3c8c) SHA1(806fe7dce619ad19c09178061be4607d2beba14d) )

	ROM_REGION( 0x2000, "proms", 0 ) /* zoom table */
	ROM_LOAD( "epr-6844.ic123", 0x0000, 0x2000, CRC(e3ec7bd6) SHA1(feec0fe664e16fac0fde61cf64b401b9b0575323) )
ROM_END

/**************************************************************************************************************************
    Enduro Racer (bootleg)
    CPU: 68000
    2xYM2203 sound board
*/
ROM_START( endurob2 )
	ROM_REGION( 0x040000, "maincpu", 0 ) /* 68000 code */
	/* the program roms should be twice the size */
	ROM_LOAD16_BYTE( "enduro.a07", 0x000000, 0x08000, BAD_DUMP CRC(259069bc) SHA1(42fa47ce4a29294f9eff3eddbba6c305d750aaa5) )
//  ROM_CONTINUE(                  0x030000, 0x08000 )
	ROM_LOAD16_BYTE( "enduro.a04", 0x000001, 0x08000, BAD_DUMP CRC(f584fbd9) SHA1(6c9ddcd1d9cf95c6250b705b27865644da45d197) )
//  ROM_CONTINUE(                  0x030000, 0x08000 )
	ROM_LOAD16_BYTE( "enduro.a08", 0x010000, 0x08000, CRC(d234918c) SHA1(ce2493a4ceff48331551e915fdbe19107865436e) )
	ROM_LOAD16_BYTE( "enduro.a05", 0x010001, 0x08000, CRC(a525dd57) SHA1(587f449ea317dc9eae06e755e7c63a652effbe15) )
	ROM_LOAD16_BYTE( "enduro.a09", 0x020000, 0x08000, CRC(f6391091) SHA1(3160b342b6447cccf67c932c7c1a42354cdfb058) )
	ROM_LOAD16_BYTE( "enduro.a06", 0x020001, 0x08000, CRC(79b367d7) SHA1(e901036b1b9fac460415d513837c8f852f7750b0) )

	ROM_REGION( 0x40000, "sub", 0 ) /* second 68000 CPU */
	ROM_LOAD16_BYTE("epr-7634.ic54", 0x0000, 0x8000, CRC(3e07fd32) SHA1(7acb9e9712ecfe928c421c84dece783e75077746) )
	ROM_LOAD16_BYTE("epr-7635.ic67", 0x0001, 0x8000, CRC(22f762ab) SHA1(70fa87da76c714db7213c42128a0b6a27644a1d4) )

	ROM_REGION( 0x18000, "gfx1", 0 ) /* tiles */
	ROM_LOAD( "epr-7644.ic31", 0x00000, 0x08000, CRC(e7a4ff90) SHA1(06d18470019041e32be9a969870cd995de626cd6) )
	ROM_LOAD( "epr-7645.ic46", 0x08000, 0x08000, CRC(4caa0095) SHA1(a24c741cdca0542e462f17ff94f132c62710e198) )
	ROM_LOAD( "epr-7646.ic60", 0x10000, 0x08000, CRC(7e432683) SHA1(c8249b23fce77eb456166161c2d9aa34309efe31) )

	ROM_REGION32_LE( 0x100000, "gfx2", 0 ) /* sprites */
	ROM_LOAD32_BYTE( "epr-7678.ic36", 0x00000, 0x8000, CRC(9fb5e656) SHA1(264b0ad017eb0fc7e0b542e6dd160ba964c100fd) )
	ROM_LOAD32_BYTE( "epr-7670.ic28", 0x00001, 0x8000, CRC(dbbe2f6e) SHA1(310797a61f91d6866e728e0da3b30828e06d1b52) )
	ROM_LOAD32_BYTE( "epr-7662.ic18", 0x00002, 0x8000, CRC(cb0c13c5) SHA1(856d1234fd8f8146e20fe6c65c0a535b7b7512cd) )
	ROM_LOAD32_BYTE( "epr-7654.1c8",  0x00003, 0x8000, CRC(2db6520d) SHA1(d16739e84316b4bd26963b729208169bbf01f499) )
	ROM_LOAD32_BYTE( "epr-7677.ic35", 0x20000, 0x8000, CRC(7764765b) SHA1(62543130816c084d292f229a15b3ce1305c99bb3) )
	ROM_LOAD32_BYTE( "epr-7669.ic27", 0x20001, 0x8000, CRC(f9525faa) SHA1(fbe2f67a9baee069dbca26a669d0a263bcca0d09) )
	ROM_LOAD32_BYTE( "enduro.a34",    0x20002, 0x8000, CRC(296454d8) SHA1(17e829a08606837d36006849edffe54c76c384d5) )
	ROM_LOAD32_BYTE( "epr-7653.ic7",  0x20003, 0x8000, CRC(46a52114) SHA1(d646ab03c1985953401619457d03072833edc6c7) )
	ROM_LOAD32_BYTE( "epr-7676.ic34", 0x40000, 0x8000, CRC(2e42e0d4) SHA1(508f6f89e681b59272884ba129a5c6ffa1b6ba05) )
	ROM_LOAD32_BYTE( "epr-7668.ic26", 0x40001, 0x8000, CRC(e115ce33) SHA1(1af591bc1567b89d0de399e4a02d896fba938bab) )
	ROM_LOAD32_BYTE( "enduro.a35",    0x40002, 0x8000, CRC(1ebe76df) SHA1(c68257d92b79cd346ca9f5639e6b3dffc6e21a5d) )
	ROM_LOAD32_BYTE( "epr-7652.ic6",  0x40003, 0x8000, CRC(2880cfdb) SHA1(94b78d78d82c324ca108970d8689f1d6b2ca8a24) )
	ROM_LOAD32_BYTE( "enduro.a20",    0x60000, 0x8000, CRC(7c280bc8) SHA1(ad8bb0204a53ea1415f088819748d40c47d96509) )
	ROM_LOAD32_BYTE( "enduro.a28",    0x60001, 0x8000, CRC(321f034b) SHA1(e30f541d0f17a75ac02a49bd5d621c75fdd89298) )
	ROM_LOAD32_BYTE( "enduro.a36",    0x60002, 0x8000, CRC(243e34e5) SHA1(4117435e97841ac2e0233089343f14b4a27dcaed) )
	ROM_LOAD32_BYTE( "enduro.a44",    0x60003, 0x8000, CRC(84bb12a1) SHA1(340de454cee9d78f8b64e12b74450b7a152b8726) )
	ROM_LOAD32_BYTE( "epr-7674.ic32", 0x80000, 0x8000, CRC(1a129acf) SHA1(ebaa60ccedc95c58af3ce99105b924b303827f6e) )
	ROM_LOAD32_BYTE( "epr-7666.ic24", 0x80001, 0x8000, CRC(23697257) SHA1(19453b14e8e6789e4c48a80d1b83dbaf37fbdceb) )
	ROM_LOAD32_BYTE( "epr-7658.ic14", 0x80002, 0x8000, CRC(1677f24f) SHA1(4786996cc8a04344e82ec9be7c4e7c8a005914a3) )
	ROM_LOAD32_BYTE( "epr-7650.ic4",  0x80003, 0x8000, CRC(642635ec) SHA1(e42bbae178e9a139325633e8c85a606c91e39e36) )
	ROM_LOAD32_BYTE( "epr-7673.ic31", 0xa0000, 0x8000, CRC(82602394) SHA1(d714f397f33a52429f394fc4c403d39be7911ccf) )
	ROM_LOAD32_BYTE( "epr-7665.ic23", 0xa0001, 0x8000, CRC(12d77607) SHA1(5b5d25646336a8ceae449d5b7a6b70372d81dd8b) )
	ROM_LOAD32_BYTE( "epr-7657.ic13", 0xa0002, 0x8000, CRC(8158839c) SHA1(f22081caf11d6b57488c969b5935cd4686e11197) )
	ROM_LOAD32_BYTE( "epr-7649.ic3",  0xa0003, 0x8000, CRC(4edba14c) SHA1(db0aab94de50f8f9501b7afd2fff70fb0a6b2b36) )
	ROM_LOAD32_BYTE( "epr-7672.ic30", 0xc0000, 0x8000, CRC(d11452f7) SHA1(f68183053005a26c0014050592bad6d63325895e) )
	ROM_LOAD32_BYTE( "epr-7664.ic22", 0xc0001, 0x8000, CRC(0df2cfad) SHA1(d62d12922be921967da37fbc624aaed72c4a2a98) )
	ROM_LOAD32_BYTE( "enduro.a39",    0xc0002, 0x8000, CRC(1ff3a5e2) SHA1(b4672ed06f6f1ed28538e6dc63efa6eed5c34587) )
	ROM_LOAD32_BYTE( "epr-7648.ic2",  0xc0003, 0x8000, CRC(983ea830) SHA1(9629476a300ba711893775ca94dce81a00afd246) )
	ROM_LOAD32_BYTE( "epr-7671.ic29", 0xe0000, 0x8000, CRC(b0c7fdc6) SHA1(c9e0993fed36526e0e46ab2da9413af24b96cae8) )
	ROM_LOAD32_BYTE( "epr-7663.ic21", 0xe0001, 0x8000, CRC(2b0b8f08) SHA1(14aa1e6866f1c23c9ff271e8f216f6ecc21601ab) )
	ROM_LOAD32_BYTE( "epr-7655.ic11", 0xe0002, 0x8000, CRC(3433fe7b) SHA1(636449a0707d6629bf6ea503cfb52ad24af1c017) )
	ROM_LOAD32_BYTE( "epr-7647.ic1",  0xe0003, 0x8000, CRC(2e7fbec0) SHA1(a59ec5fc3341833671fb948cd21b47f3a49db538) )

	ROM_REGION( 0x8000, "gfx3", 0 ) /* road gfx */
	ROM_LOAD( "epr-7633.ic1", 0x0000, 0x8000, CRC(6f146210) SHA1(2f58f0c3563b434ed02700b9ca1545a696a5716e) )

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* sound CPU */
	ROM_LOAD( "enduro.a16", 0x00000, 0x8000, CRC(d2cb6eb5) SHA1(80c5fab16ec4ddfa67fae94808026b2e6285b7f1) )

	ROM_REGION( 0x20000, "pcm", 0 ) /* Sega PCM sound data */
	ROM_LOAD( "epr-7681.rom", 0x00000, 0x8000, CRC(bc0c4d12) SHA1(3de71bde4c23e3c31984f20fc4bc7e221354c56f) )
	ROM_LOAD( "epr-7680.rom", 0x10000, 0x8000, CRC(627b3c8c) SHA1(806fe7dce619ad19c09178061be4607d2beba14d) )

	ROM_REGION( 0x2000, "proms", 0 ) /* zoom table */
	ROM_LOAD( "epr-6844.ic123", 0x0000, 0x2000, CRC(e3ec7bd6) SHA1(feec0fe664e16fac0fde61cf64b401b9b0575323) )
ROM_END



/*************************************
 *
 *  Driver initialization
 *
 *************************************/

static DRIVER_INIT( hangon )
{
	hangon_generic_init(machine);
}


static DRIVER_INIT( sharrier )
{
	segas1x_state *state = (segas1x_state *)machine->driver_data;

	hangon_generic_init(machine);
	state->i8751_vblank_hook = sharrier_i8751_sim;
}


static DRIVER_INIT( enduror )
{
	hangon_generic_init(machine);
	fd1089b_decrypt(machine);
}


static DRIVER_INIT( endurobl )
{
	const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);
	UINT16 *rom = (UINT16 *)memory_region(machine, "maincpu");
	UINT16 *decrypt = auto_alloc_array(machine, UINT16, 0x40000/2);

	hangon_generic_init(machine);
	memory_set_decrypted_region(space, 0x000000, 0x03ffff, decrypt);

	memcpy(decrypt + 0x00000/2, rom + 0x30000/2, 0x10000);
	memcpy(decrypt + 0x10000/2, rom + 0x10000/2, 0x20000);
}


static DRIVER_INIT( endurob2 )
{
	const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);
	UINT16 *rom = (UINT16 *)memory_region(machine, "maincpu");
	UINT16 *decrypt = auto_alloc_array(machine, UINT16, 0x40000/2);

	hangon_generic_init(machine);
	memory_set_decrypted_region(space, 0x000000, 0x03ffff, decrypt);

	memcpy(decrypt, rom, 0x30000);
	/* missing data ROM */
}

static DRIVER_INIT( shangonro )
{
	hangon_generic_init(machine);

	/* init the FD1094 */
	fd1094_driver_init(machine, "sub", NULL);
}


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1985, hangon,    0,        hangon,   hangon,   hangon,   ROT0, "Sega",    "Hang-On (Rev A)", 0 )
GAME( 1985, hangon1,   hangon,   hangon,   hangon,   hangon,   ROT0, "Sega",    "Hang-On", 0 )
GAME( 1992, shangonro, shangon,  shangupb, shangonro,shangonro,ROT0, "Sega",    "Super Hang-On (Japan, FD1094 317-0038)", 0 )
GAME( 1992, shangonrb, shangon,  shangupb, shangupb, hangon,   ROT0, "bootleg", "Super Hang-On (bootleg)", 0 )
GAME( 1985, sharrier,  0,        sharrier, sharrier, sharrier, ROT0, "Sega",    "Space Harrier (Rev A, 8751 315-5163A)", 0 )
GAME( 1985, sharrier1, sharrier, sharrier, sharrier, sharrier, ROT0, "Sega",    "Space Harrier (8751 315-5163)", 0 )
GAME( 1986, enduror,   0,        enduror,  enduror,  enduror,  ROT0, "Sega",    "Enduro Racer (YM2151, FD1089B 317-0013A)", 0 )
GAME( 1986, enduror1,  enduror,  enduror1, enduror,  enduror,  ROT0, "Sega",    "Enduro Racer (YM2203, FD1089B 317-0013A)", 0 )
GAME( 1986, endurobl,  enduror,  enduror1, enduror,  endurobl, ROT0, "bootleg", "Enduro Racer (bootleg set 1)", 0 )
GAME( 1986, endurob2,  enduror,  endurob2, enduror,  endurob2, ROT0, "bootleg", "Enduro Racer (bootleg set 2)", GAME_NOT_WORKING )

