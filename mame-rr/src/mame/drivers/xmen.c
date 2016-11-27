/***************************************************************************

X-Men

driver by Nicola Salmoria

notes:

the way the double screen works in xmen6p is not fully understood

the board only has one of each gfx chip, the only additional chip not found
on the 2/4p board is 053253.  This chip is also on Run n Gun which should
likewise be a 2 screen game

***************************************************************************/
#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "deprecat.h"
#include "video/konicdev.h"
#include "machine/eeprom.h"
#include "cpu/z80/z80.h"
#include "sound/2151intf.h"
#include "sound/k054539.h"
#include "rendlay.h"
#include "includes/xmen.h"
#include "includes/konamipt.h"

/***************************************************************************

  EEPROM

***************************************************************************/

static const eeprom_interface eeprom_intf =
{
	7,				/* address bits */
	8,				/* data bits */
	"011000",		/*  read command */
	"011100",		/* write command */
	0,				/* erase command */
	"0100000000000",/* lock command */
	"0100110000000" /* unlock command */
};

static WRITE16_HANDLER( eeprom_w )
{
	xmen_state *state = (xmen_state *)space->machine->driver_data;

	logerror("%06x: write %04x to 108000\n",cpu_get_pc(space->cpu),data);
	if (ACCESSING_BITS_0_7)
	{
		/* bit 0 = coin counter */
		coin_counter_w(space->machine, 0, data & 0x01);

		/* bit 2 is data */
		/* bit 3 is clock (active high) */
		/* bit 4 is cs (active low) */
		input_port_write(space->machine, "EEPROMOUT", data, 0xff);
	}
	if (ACCESSING_BITS_8_15)
	{
		/* bit 8 = enable sprite ROM reading */
		k053246_set_objcha_line(state->k053246, (data & 0x0100) ? ASSERT_LINE : CLEAR_LINE);
		/* bit 9 = enable char ROM reading through the video RAM */
		k052109_set_rmrd_line(state->k052109, (data & 0x0200) ? ASSERT_LINE : CLEAR_LINE);
	}
}

static READ16_HANDLER( sound_status_r )
{
	return soundlatch2_r(space, 0);
}

static WRITE16_HANDLER( sound_cmd_w )
{
	if (ACCESSING_BITS_0_7)
	{
		data &= 0xff;
		soundlatch_w(space, 0, data);
	}
}

static WRITE16_HANDLER( sound_irq_w )
{
	xmen_state *state = (xmen_state *)space->machine->driver_data;
	cpu_set_input_line(state->audiocpu, 0, HOLD_LINE);
}

static WRITE16_HANDLER( xmen_18fa00_w )
{
	if(ACCESSING_BITS_0_7)
	{
		/* bit 2 is interrupt enable */
		interrupt_enable_w(space, 0, data & 0x04);
	}
}

static void sound_reset_bank( running_machine *machine )
{
	xmen_state *state = (xmen_state *)machine->driver_data;
	memory_set_bank(machine, "bank4", state->sound_curbank & 0x07);
}

static WRITE8_HANDLER( sound_bankswitch_w )
{
	xmen_state *state = (xmen_state *)space->machine->driver_data;
	state->sound_curbank = data;
	sound_reset_bank(space->machine);
}


static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x080000, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x100fff) AM_DEVREADWRITE("k053246", k053247_word_r, k053247_word_w)
	AM_RANGE(0x101000, 0x101fff) AM_RAM
	AM_RANGE(0x104000, 0x104fff) AM_RAM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x108000, 0x108001) AM_WRITE(eeprom_w)
	AM_RANGE(0x108020, 0x108027) AM_DEVWRITE("k053246", k053246_word_w)
	AM_RANGE(0x10804c, 0x10804d) AM_WRITE(sound_cmd_w)
	AM_RANGE(0x10804e, 0x10804f) AM_WRITE(sound_irq_w)
	AM_RANGE(0x108054, 0x108055) AM_READ(sound_status_r)
	AM_RANGE(0x108060, 0x10807f) AM_DEVWRITE("k053251", k053251_lsb_w)
	AM_RANGE(0x10a000, 0x10a001) AM_READ_PORT("P2_P4") AM_WRITE(watchdog_reset16_w)
	AM_RANGE(0x10a002, 0x10a003) AM_READ_PORT("P1_P3")
	AM_RANGE(0x10a004, 0x10a005) AM_READ_PORT("EEPROM")
	AM_RANGE(0x10a00c, 0x10a00d) AM_DEVREAD("k053246", k053246_word_r)
	AM_RANGE(0x110000, 0x113fff) AM_RAM		/* main RAM */
	AM_RANGE(0x18fa00, 0x18fa01) AM_WRITE(xmen_18fa00_w)
	AM_RANGE(0x18c000, 0x197fff) AM_DEVREADWRITE("k052109", k052109_lsb_r, k052109_lsb_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank4")
	AM_RANGE(0xc000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xe22f) AM_DEVREADWRITE("k054539", k054539_r, k054539_w)
	AM_RANGE(0xe800, 0xe801) AM_MIRROR(0x0400) AM_DEVREADWRITE("ymsnd", ym2151_r, ym2151_w)
	AM_RANGE(0xf000, 0xf000) AM_WRITE(soundlatch2_w)
	AM_RANGE(0xf002, 0xf002) AM_READ(soundlatch_r)
	AM_RANGE(0xf800, 0xf800) AM_WRITE(sound_bankswitch_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( 6p_main_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x080000, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x100fff) AM_RAM AM_BASE_MEMBER(xmen_state, xmen6p_spriteramleft)	/* sprites (screen 1) */
	AM_RANGE(0x101000, 0x101fff) AM_RAM
	AM_RANGE(0x102000, 0x102fff) AM_RAM AM_BASE_MEMBER(xmen_state, xmen6p_spriteramright)	/* sprites (screen 2) */
	AM_RANGE(0x103000, 0x103fff) AM_RAM		/* 6p - a buffer? */
	AM_RANGE(0x104000, 0x104fff) AM_RAM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x108000, 0x108001) AM_WRITE(eeprom_w)
	AM_RANGE(0x108020, 0x108027) AM_DEVWRITE("k053246", k053246_word_w) /* sprites */
	AM_RANGE(0x10804c, 0x10804d) AM_WRITE(sound_cmd_w)
	AM_RANGE(0x10804e, 0x10804f) AM_WRITE(sound_irq_w)
	AM_RANGE(0x108054, 0x108055) AM_READ(sound_status_r)
	AM_RANGE(0x108060, 0x10807f) AM_DEVWRITE("k053251", k053251_lsb_w)
	AM_RANGE(0x10a000, 0x10a001) AM_READ_PORT("P2_P4") AM_WRITE(watchdog_reset16_w)
	AM_RANGE(0x10a002, 0x10a003) AM_READ_PORT("P1_P3")
	AM_RANGE(0x10a004, 0x10a005) AM_READ_PORT("EEPROM")
	AM_RANGE(0x10a006, 0x10a007) AM_READ_PORT("P5_P6")
	AM_RANGE(0x10a00c, 0x10a00d) AM_DEVREAD("k053246", k053246_word_r) /* sprites */
	AM_RANGE(0x110000, 0x113fff) AM_RAM		/* main RAM */
	AM_RANGE(0x18fa00, 0x18fa01) AM_WRITE(xmen_18fa00_w)
/*  AM_RANGE(0x18c000, 0x197fff) AM_DEVWRITE("k052109", k052109_lsb_w) AM_BASE_MEMBER(xmen_state, xmen6p_tilemapleft) */
	AM_RANGE(0x18c000, 0x197fff) AM_RAM AM_BASE_MEMBER(xmen_state, xmen6p_tilemapleft) /* left tilemap (p1,p2,p3 counters) */
/*
    AM_RANGE(0x1ac000, 0x1af7ff) AM_READONLY
    AM_RANGE(0x1ac000, 0x1af7ff) AM_WRITEONLY

    AM_RANGE(0x1b0000, 0x1b37ff) AM_READONLY
    AM_RANGE(0x1b0000, 0x1b37ff) AM_WRITEONLY

    AM_RANGE(0x1b4000, 0x1b77ff) AM_READONLY
    AM_RANGE(0x1b4000, 0x1b77ff) AM_WRITEONLY
*/
	AM_RANGE(0x1ac000, 0x1b7fff) AM_RAM AM_BASE_MEMBER(xmen_state, xmen6p_tilemapright) /* right tilemap */

	/* what are the regions below buffers? (used by hw or software?) */
/*
    AM_RANGE(0x1cc000, 0x1cf7ff) AM_READONLY
    AM_RANGE(0x1cc000, 0x1cf7ff) AM_WRITEONLY

    AM_RANGE(0x1d0000, 0x1d37ff) AM_READONLY
    AM_RANGE(0x1d0000, 0x1d37ff) AM_WRITEONLY
*/
	AM_RANGE(0x1cc000, 0x1d7fff) AM_RAM /* tilemap ? */

	/* whats the stuff below, buffers? */
/*
    AM_RANGE(0x1ec000, 0x1ef7ff) AM_READONLY
    AM_RANGE(0x1ec000, 0x1ef7ff) AM_WRITEONLY
    AM_RANGE(0x1f0000, 0x1f37ff) AM_READONLY
    AM_RANGE(0x1f0000, 0x1f37ff) AM_WRITEONLY
    AM_RANGE(0x1f4000, 0x1f77ff) AM_READONLY
    AM_RANGE(0x1f4000, 0x1f77ff) AM_WRITEONLY
*/
	AM_RANGE(0x1ec000, 0x1f7fff) AM_RAM /* tilemap ? */
ADDRESS_MAP_END


static INPUT_PORTS_START( xmen )
	PORT_START("P1_P3")
	KONAMI16_LSB_UDLR(1, IPT_BUTTON3, IPT_COIN1 )
	KONAMI16_MSB_UDLR(3, IPT_BUTTON3, IPT_COIN3 )

	PORT_START("P2_P4")
	KONAMI16_LSB_UDLR(2, IPT_BUTTON3, IPT_COIN2 )
	KONAMI16_MSB_UDLR(4, IPT_BUTTON3, IPT_COIN4 )

	PORT_START("EEPROM")
	PORT_BIT( 0x003f, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* unused? */
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE("eeprom", eeprom_read_bit)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* EEPROM status - always 1 */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x3000, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* unused? */
	PORT_SERVICE_NO_TOGGLE( 0x4000, IP_ACTIVE_LOW )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* unused? */

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_write_bit)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_set_clock_line)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_set_cs_line)
INPUT_PORTS_END

static INPUT_PORTS_START( xmen2p )
	PORT_START("P1_P3")
	KONAMI16_LSB_UDLR(1, IPT_BUTTON3, IPT_COIN1 )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2_P4")
	KONAMI16_LSB_UDLR(2, IPT_BUTTON3, IPT_COIN2 )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("EEPROM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x003c, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* unused? */
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE("eeprom", eeprom_read_bit)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* EEPROM status - always 1 */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x3000, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* unused? */
	PORT_SERVICE_NO_TOGGLE( 0x4000, IP_ACTIVE_LOW )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* unused? */

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_write_bit)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_set_clock_line)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_set_cs_line)
INPUT_PORTS_END

static CUSTOM_INPUT( xmen_frame_r )
{
	xmen_state *state = (xmen_state *)field->port->machine->driver_data;
	return state->current_frame;
}

static INPUT_PORTS_START( xmen6p )
	PORT_START("P1_P3")
	KONAMI16_LSB_UDLR(1, IPT_BUTTON3, IPT_COIN1 )
	KONAMI16_MSB_UDLR(3, IPT_BUTTON3, IPT_COIN3 )

	PORT_START("P2_P4")
	KONAMI16_LSB_UDLR(2, IPT_BUTTON3, IPT_COIN2 )
	KONAMI16_MSB_UDLR(4, IPT_BUTTON3, IPT_COIN4 )

	PORT_START("P5_P6")
	KONAMI16_LSB_UDLR(5, IPT_BUTTON3, IPT_COIN5 )
	KONAMI16_MSB_UDLR(6, IPT_BUTTON3, IPT_COIN6 )

	PORT_START("EEPROM")
	PORT_BIT( 0x003f, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* unused? */
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE("eeprom", eeprom_read_bit)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* EEPROM status - always 1 */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_START5 ) /* not verified */
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_START6 ) /* not verified */
	PORT_SERVICE_NO_TOGGLE( 0x4000, IP_ACTIVE_LOW )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(xmen_frame_r, NULL)	/* screen indicator? */

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_write_bit)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_set_clock_line)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_set_cs_line)
INPUT_PORTS_END



static INTERRUPT_GEN( xmen_interrupt )
{
	if (cpu_getiloops(device) == 0)
		irq5_line_hold(device);
	else
		irq3_line_hold(device);
}

static STATE_POSTLOAD( xmen_postload )
{
	sound_reset_bank(machine);
}

static MACHINE_START( xmen )
{
	xmen_state *state = (xmen_state *)machine->driver_data;
	UINT8 *ROM = memory_region(machine, "audiocpu");

	memory_configure_bank(machine, "bank4", 0, 8, &ROM[0x10000], 0x4000);
	memory_set_bank(machine, "bank4", 0);

	state->maincpu = machine->device("maincpu");
	state->audiocpu = machine->device("audiocpu");
	state->k053246 = machine->device("k053246");
	state->k053251 = machine->device("k053251");
	state->k052109 = machine->device("k052109");
	state->k054539 = machine->device("k054539");
	state->lscreen = machine->device("lscreen");
	state->rscreen = machine->device("rscreen");

	state_save_register_global(machine, state->sound_curbank);
	state_save_register_global(machine, state->sprite_colorbase);
	state_save_register_global_array(machine, state->layer_colorbase);
	state_save_register_global_array(machine, state->layerpri);
	state_save_register_postload(machine, xmen_postload, NULL);
}

static MACHINE_RESET( xmen )
{
	xmen_state *state = (xmen_state *)machine->driver_data;
	int i;

	for (i = 0; i < 3; i++)
	{
		state->layerpri[i] = 0;
		state->layer_colorbase[i] = 0;
	}

	state->sprite_colorbase = 0;
	state->sound_curbank = 0;
}

static const k052109_interface xmen_k052109_intf =
{
	"gfx1", 0,
	NORMAL_PLANE_ORDER,
	KONAMI_ROM_DEINTERLEAVE_2,
	xmen_tile_callback
};

static const k053247_interface xmen_k053246_intf =
{
	"screen",
	"gfx2", 1,
	NORMAL_PLANE_ORDER,
	53, -2,
	KONAMI_ROM_DEINTERLEAVE_4,
	xmen_sprite_callback
};

static MACHINE_DRIVER_START( xmen )

	/* driver data */
	MDRV_DRIVER_DATA(xmen_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, XTAL_16MHz)	/* verified on pcb */
	MDRV_CPU_PROGRAM_MAP(main_map)
	MDRV_CPU_VBLANK_INT_HACK(xmen_interrupt,2)

	MDRV_CPU_ADD("audiocpu", Z80, XTAL_16MHz/2)	/* verified on pcb */
	MDRV_CPU_PROGRAM_MAP(sound_map)

	MDRV_MACHINE_START(xmen)
	MDRV_MACHINE_RESET(xmen)

	MDRV_EEPROM_ADD("eeprom", eeprom_intf)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_HAS_SHADOWS)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(59.17)   /* verified on pcb */
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(13*8, (64-13)*8-1, 2*8, 30*8-1 )	/* correct, same issue of TMNT2 */
	MDRV_PALETTE_LENGTH(2048)

	MDRV_VIDEO_UPDATE(xmen)

	MDRV_K052109_ADD("k052109", xmen_k052109_intf)
	MDRV_K053246_ADD("k053246", xmen_k053246_intf)
	MDRV_K053251_ADD("k053251")

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2151, XTAL_16MHz/4)	/* verified on pcb */
	MDRV_SOUND_ROUTE(0, "lspeaker", 0.80)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.80)

	MDRV_SOUND_ADD("k054539", K054539, 48000)
	MDRV_SOUND_ROUTE(0, "lspeaker", 0.80)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.80)
MACHINE_DRIVER_END


static MACHINE_START( xmen6p )
{
	xmen_state *state = (xmen_state *)machine->driver_data;

	MACHINE_START_CALL(xmen);

	state_save_register_global(machine, state->current_frame);
}

static MACHINE_RESET( xmen6p )
{
	xmen_state *state = (xmen_state *)machine->driver_data;
	state->current_frame = 0x00;
}

static INTERRUPT_GEN( xmen6p_interrupt )
{
	if (cpu_getiloops(device) == 0)
	{
		irq5_line_hold(device);


	}
	else
	{
//      if (xmen_irqenabled & 0x04)
//      {
			irq3_line_hold(device);
//          state->current_frame = 0x00;

//      }
	}
}

static const k053247_interface xmen6p_k053246_intf =
{
	"lscreen",	/* is this correct? */
	"gfx2", 1,
	NORMAL_PLANE_ORDER,
	53, -2,
	KONAMI_ROM_DEINTERLEAVE_4,
	xmen_sprite_callback
};

static MACHINE_DRIVER_START( xmen6p )

	/* driver data */
	MDRV_DRIVER_DATA(xmen_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 16000000)	/* ? */
	MDRV_CPU_PROGRAM_MAP(6p_main_map)
	MDRV_CPU_VBLANK_INT_HACK(xmen6p_interrupt,2)

	MDRV_CPU_ADD("audiocpu", Z80,8000000)	/* verified with M1, guessed but accurate */
	MDRV_CPU_PROGRAM_MAP(sound_map)

	MDRV_MACHINE_START(xmen6p)
	MDRV_MACHINE_RESET(xmen6p)

	MDRV_EEPROM_ADD("eeprom", eeprom_intf)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_HAS_SHADOWS)
	MDRV_PALETTE_LENGTH(2048)
	MDRV_DEFAULT_LAYOUT(layout_dualhsxs)

	MDRV_SCREEN_ADD("lscreen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(12*8, 48*8-1, 2*8, 30*8-1)

	MDRV_SCREEN_ADD("rscreen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(16*8, 52*8-1, 2*8, 30*8-1)

	MDRV_VIDEO_START(xmen6p)
	MDRV_VIDEO_UPDATE(xmen6p)
	MDRV_VIDEO_EOF(xmen6p)

	MDRV_K052109_ADD("k052109", xmen_k052109_intf)
	MDRV_K053246_ADD("k053246", xmen6p_k053246_intf)
	MDRV_K053251_ADD("k053251")

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2151, 4000000)
	MDRV_SOUND_ROUTE(0, "lspeaker", 0.80)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.80)

	MDRV_SOUND_ADD("k054539", K054539, 48000)
	MDRV_SOUND_ROUTE(0, "lspeaker", 0.80)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.80)
MACHINE_DRIVER_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

/*

    KONAMI - X-MEN 2P/4P - GX065 - PWB353018A

    +--------------------------------------------------------------------------------------+
    |                                                                                      |
    |                                  [  065A06.1f  ]     [  065A12.1h  ][  065A11.1l  ]  |
    |                                                                                      |
    | sound                                                [  065A09.1h  ][  065A10.1l  ]  |
    |  out                                                                                 |
    |                                                                                      |
    |        [  054544  ] [  054539  ]                                                     |
    |                                                                                      |
    |                                                                                      |
    +-+     [    Z80    ] [ YM2151 ] [  065*01.6f  ]                                       |
      |                                                                                    |
    +-+                                                                     [  053246  ]   |
    |                                                                                      |
    |                   [  065A02.9d  ]  [  065A03.9f  ]                                   |
    +-+                                                                                    |
    +-+                 [  065*04.10d ]  [  065*05.10f ]                                   |
    |   J                                                                                  |
    |   A                                                                                  |
    |   M                                                                   [  053247  ]   |
    |   M                                                                                  |
    |   A              [     68000 - 16Mhz     ]  [qz 24Mhz]                               |
    |                                                                                      |
    |                                                                                      |
    |                                                                                      |
    |                    [qz 32Mhz/18.432Mhz]   [  052109  ] [  051962  ]   [  053251  ]   |
    |                                                                                      |
    +-+                                                                                    |
      |                                                                    [  065A08.15l ] |
    +-+                                                                                    |
    |                                                                      [  065A07.16l ] |
    |     [cn7 (4P)]                                                                       |
    |test                                                                                  |
    | sw  [cn6 (3P)]                                                                       |
    |                                                                                      |
    +--------------------------------------------------------------------------------------+

    054544
    054539
    053246
    053247
    052109
    051962
    053251
*/

ROM_START( xmen )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "065-ubb04.10d",  0x00000, 0x20000, CRC(f896c93b) SHA1(0bee89fe4d36a9b2ded864770198eb2df6903580) ) /* US 4 Player version */
	ROM_LOAD16_BYTE( "065-ubb05.10f",  0x00001, 0x20000, CRC(e02e5d64) SHA1(9838c1cf9862db3ca70a23ef5f3c5883729c4e0c) )
	ROM_LOAD16_BYTE( "065-a02.9d",     0x80000, 0x40000, CRC(b31dc44c) SHA1(4bdac05826b4d6d4fe46686ede5190e2f73eefc5) )
	ROM_LOAD16_BYTE( "065-a03.9f",     0x80001, 0x40000, CRC(13842fe6) SHA1(b61f094eb94336edb8708d3437ead9b853b2d6e6) )

	ROM_REGION( 0x30000, "audiocpu", 0 )		/* 64k+128k for sound cpu */
	ROM_LOAD( "065-a01.6f",   0x00000, 0x20000, CRC(147d3a4d) SHA1(a14409fe991e803b9e7812303e3a9ebd857d8b01) )
	ROM_RELOAD(               0x10000, 0x20000 )

	ROM_REGION( 0x200000, "gfx1", 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD( "065-a08.15l",   0x000000, 0x100000, CRC(6b649aca) SHA1(2595f314517738e8614facf578cc951a6c36a180) )	/* tiles */
	ROM_LOAD( "065-a07.16l",   0x100000, 0x100000, CRC(c5dc8fc4) SHA1(9887cb002c8b72be7ce933cb397f00cdc5506c8c) )

	ROM_REGION( 0x400000, "gfx2", 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD( "065-a09.2h",  0x000000, 0x100000, CRC(ea05d52f) SHA1(7f2c14f907355856fb94e3a67b73aa1919776835) )	/* sprites */
	ROM_LOAD( "065-a10.2l",  0x100000, 0x100000, CRC(96b91802) SHA1(641943557b59b91f0edd49ec8a73cef7d9268b32) )
	ROM_LOAD( "065-a12.1h",  0x200000, 0x100000, CRC(321ed07a) SHA1(5b00ed676daeea974bdce6701667cfe573099dad) )
	ROM_LOAD( "065-a11.1l",  0x300000, 0x100000, CRC(46da948e) SHA1(168ac9178ee5bad5931557fb549e1237971d7839) )

	ROM_REGION( 0x200000, "k054539", 0 )	/* samples for the 054539 */
	ROM_LOAD( "065-a06.1f",  0x000000, 0x200000, CRC(5adbcee0) SHA1(435feda697193bc51db80eba46be474cbbc1de4b) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "xmen_ubb.nv", 0x0000, 0x0080, CRC(52f334ba) SHA1(171c22b5ac41bcbbcfc31528cf49c096f6829a72) )
ROM_END

ROM_START( xmenj )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "065-jba04.10d",  0x00000, 0x20000, CRC(d86cf5eb) SHA1(8bf67eb6cdb7187142557c27b058282886984a61) ) /* Japan 4 Player version */
	ROM_LOAD16_BYTE( "065-jba05.10f",  0x00001, 0x20000, CRC(abbc8126) SHA1(482a3c9be45b9d77460bd3df94e3c6cf285e63a2) )
	ROM_LOAD16_BYTE( "065-a02.9d",     0x80000, 0x40000, CRC(b31dc44c) SHA1(4bdac05826b4d6d4fe46686ede5190e2f73eefc5) )
	ROM_LOAD16_BYTE( "065-a03.9f",     0x80001, 0x40000, CRC(13842fe6) SHA1(b61f094eb94336edb8708d3437ead9b853b2d6e6) )

	ROM_REGION( 0x30000, "audiocpu", 0 )		/* 64k+128k for sound cpu */
	ROM_LOAD( "065-a01.6f",   0x00000, 0x20000, CRC(147d3a4d) SHA1(a14409fe991e803b9e7812303e3a9ebd857d8b01) )
	ROM_RELOAD(               0x10000, 0x20000 )

	ROM_REGION( 0x200000, "gfx1", 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD( "065-a08.15l",   0x000000, 0x100000, CRC(6b649aca) SHA1(2595f314517738e8614facf578cc951a6c36a180) )	/* tiles */
	ROM_LOAD( "065-a07.16l",   0x100000, 0x100000, CRC(c5dc8fc4) SHA1(9887cb002c8b72be7ce933cb397f00cdc5506c8c) )

	ROM_REGION( 0x400000, "gfx2", 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD( "065-a09.2h",  0x000000, 0x100000, CRC(ea05d52f) SHA1(7f2c14f907355856fb94e3a67b73aa1919776835) )	/* sprites */
	ROM_LOAD( "065-a10.2l",  0x100000, 0x100000, CRC(96b91802) SHA1(641943557b59b91f0edd49ec8a73cef7d9268b32) )
	ROM_LOAD( "065-a12.1h",  0x200000, 0x100000, CRC(321ed07a) SHA1(5b00ed676daeea974bdce6701667cfe573099dad) )
	ROM_LOAD( "065-a11.1l",  0x300000, 0x100000, CRC(46da948e) SHA1(168ac9178ee5bad5931557fb549e1237971d7839) )

	ROM_REGION( 0x200000, "k054539", 0 )	/* samples for the 054539 */
	ROM_LOAD( "065-a06.1f",  0x000000, 0x200000, CRC(5adbcee0) SHA1(435feda697193bc51db80eba46be474cbbc1de4b) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "xmen_jba.nv", 0x0000, 0x0080, CRC(7439cea7) SHA1(d34b8ed0549b0457362159098e5c86b1356e35d0) )
ROM_END

ROM_START( xmene )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "065-eba04.10d",  0x00000, 0x20000, CRC(3588c5ec) SHA1(7966e7259038468845dafd19e5f7fc576c2901fa) ) /* Europe 4 Player version */
	ROM_LOAD16_BYTE( "065-eba05.10f",  0x00001, 0x20000, CRC(79ce32f8) SHA1(1a21b38d4a82103d78e246aca68ed3e4afaf60f3) )
	ROM_LOAD16_BYTE( "065-a02.9d",     0x80000, 0x40000, CRC(b31dc44c) SHA1(4bdac05826b4d6d4fe46686ede5190e2f73eefc5) )
	ROM_LOAD16_BYTE( "065-a03.9f",     0x80001, 0x40000, CRC(13842fe6) SHA1(b61f094eb94336edb8708d3437ead9b853b2d6e6) )

	ROM_REGION( 0x30000, "audiocpu", 0 )		/* 64k+128k for sound cpu */
	ROM_LOAD( "065-a01.6f",   0x00000, 0x20000, CRC(147d3a4d) SHA1(a14409fe991e803b9e7812303e3a9ebd857d8b01) )
	ROM_RELOAD(               0x10000, 0x20000 )

	ROM_REGION( 0x200000, "gfx1", 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD( "065-a08.15l",   0x000000, 0x100000, CRC(6b649aca) SHA1(2595f314517738e8614facf578cc951a6c36a180) )	/* tiles */
	ROM_LOAD( "065-a07.16l",   0x100000, 0x100000, CRC(c5dc8fc4) SHA1(9887cb002c8b72be7ce933cb397f00cdc5506c8c) )

	ROM_REGION( 0x400000, "gfx2", 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD( "065-a09.2h",  0x000000, 0x100000, CRC(ea05d52f) SHA1(7f2c14f907355856fb94e3a67b73aa1919776835) )	/* sprites */
	ROM_LOAD( "065-a10.2l",  0x100000, 0x100000, CRC(96b91802) SHA1(641943557b59b91f0edd49ec8a73cef7d9268b32) )
	ROM_LOAD( "065-a12.1h",  0x200000, 0x100000, CRC(321ed07a) SHA1(5b00ed676daeea974bdce6701667cfe573099dad) )
	ROM_LOAD( "065-a11.1l",  0x300000, 0x100000, CRC(46da948e) SHA1(168ac9178ee5bad5931557fb549e1237971d7839) )

	ROM_REGION( 0x200000, "k054539", 0 )	/* samples for the 054539 */
	ROM_LOAD( "065-a06.1f",  0x000000, 0x200000, CRC(5adbcee0) SHA1(435feda697193bc51db80eba46be474cbbc1de4b) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "xmen_eba.nv", 0x0000, 0x0080, CRC(37f8e77a) SHA1(0b92caba33486c6fd104806aa96f735743bb2221) )
ROM_END

ROM_START( xmen2pe )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "065-eaa04.10d",  0x00000, 0x20000, CRC(502861e7) SHA1(f96aab2d2006703065de5bd7e341f929d04f5f60) ) /* Europe 2 Player version */
	ROM_LOAD16_BYTE( "065-eaa05.10f",  0x00001, 0x20000, CRC(ca6071bf) SHA1(454ddc3b598389e960e87e577a01a7de71d1f591) )
	ROM_LOAD16_BYTE( "065-a02.9d",     0x80000, 0x40000, CRC(b31dc44c) SHA1(4bdac05826b4d6d4fe46686ede5190e2f73eefc5) )
	ROM_LOAD16_BYTE( "065-a03.9f",     0x80001, 0x40000, CRC(13842fe6) SHA1(b61f094eb94336edb8708d3437ead9b853b2d6e6) )

	ROM_REGION( 0x30000, "audiocpu", 0 )		/* 64k+128k for sound cpu */
	ROM_LOAD( "065-a01.6f",   0x00000, 0x20000, CRC(147d3a4d) SHA1(a14409fe991e803b9e7812303e3a9ebd857d8b01) )
	ROM_RELOAD(               0x10000, 0x20000 )

	ROM_REGION( 0x200000, "gfx1", 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD( "065-a08.15l",   0x000000, 0x100000, CRC(6b649aca) SHA1(2595f314517738e8614facf578cc951a6c36a180) )	/* tiles */
	ROM_LOAD( "065-a07.16l",   0x100000, 0x100000, CRC(c5dc8fc4) SHA1(9887cb002c8b72be7ce933cb397f00cdc5506c8c) )

	ROM_REGION( 0x400000, "gfx2", 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD( "065-a09.2h",  0x000000, 0x100000, CRC(ea05d52f) SHA1(7f2c14f907355856fb94e3a67b73aa1919776835) )	/* sprites */
	ROM_LOAD( "065-a10.2l",  0x100000, 0x100000, CRC(96b91802) SHA1(641943557b59b91f0edd49ec8a73cef7d9268b32) )
	ROM_LOAD( "065-a12.1h",  0x200000, 0x100000, CRC(321ed07a) SHA1(5b00ed676daeea974bdce6701667cfe573099dad) )
	ROM_LOAD( "065-a11.1l",  0x300000, 0x100000, CRC(46da948e) SHA1(168ac9178ee5bad5931557fb549e1237971d7839) )

	ROM_REGION( 0x200000, "k054539", 0 )	/* samples for the 054539 */
	ROM_LOAD( "065-a06.1f",  0x000000, 0x200000, CRC(5adbcee0) SHA1(435feda697193bc51db80eba46be474cbbc1de4b) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "xmen_eaa.nv", 0x0000, 0x0080, CRC(1cbcb653) SHA1(a86b4ad34ccbd868662ff8c61eb21ec07e8bf8b1) )
ROM_END

ROM_START( xmen2pa )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "065-aaa04.10d",  0x00000, 0x20000, CRC(7f8b27c2) SHA1(052db1f47671564a440544a41fc397a19d1aff3a) ) /* Asia 2 Player version */
	ROM_LOAD16_BYTE( "065-aaa05.10f",  0x00001, 0x20000, CRC(841ed636) SHA1(33f96022ce3dae9b49eb51fd4e8f7387a1777002) )
	ROM_LOAD16_BYTE( "065-a02.9d",     0x80000, 0x40000, CRC(b31dc44c) SHA1(4bdac05826b4d6d4fe46686ede5190e2f73eefc5) )
	ROM_LOAD16_BYTE( "065-a03.9f",     0x80001, 0x40000, CRC(13842fe6) SHA1(b61f094eb94336edb8708d3437ead9b853b2d6e6) )

	ROM_REGION( 0x30000, "audiocpu", 0 )		/* 64k+128k for sound cpu */
	ROM_LOAD( "065-a01.6f",   0x00000, 0x20000, CRC(147d3a4d) SHA1(a14409fe991e803b9e7812303e3a9ebd857d8b01) )
	ROM_RELOAD(               0x10000, 0x20000 )

	ROM_REGION( 0x200000, "gfx1", 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD( "065-a08.15l",   0x000000, 0x100000, CRC(6b649aca) SHA1(2595f314517738e8614facf578cc951a6c36a180) )	/* tiles */
	ROM_LOAD( "065-a07.16l",   0x100000, 0x100000, CRC(c5dc8fc4) SHA1(9887cb002c8b72be7ce933cb397f00cdc5506c8c) )

	ROM_REGION( 0x400000, "gfx2", 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD( "065-a09.2h",  0x000000, 0x100000, CRC(ea05d52f) SHA1(7f2c14f907355856fb94e3a67b73aa1919776835) )	/* sprites */
	ROM_LOAD( "065-a10.2l",  0x100000, 0x100000, CRC(96b91802) SHA1(641943557b59b91f0edd49ec8a73cef7d9268b32) )
	ROM_LOAD( "065-a12.1h",  0x200000, 0x100000, CRC(321ed07a) SHA1(5b00ed676daeea974bdce6701667cfe573099dad) )
	ROM_LOAD( "065-a11.1l",  0x300000, 0x100000, CRC(46da948e) SHA1(168ac9178ee5bad5931557fb549e1237971d7839) )

	ROM_REGION( 0x200000, "k054539", 0 )	/* samples for the 054539 */
	ROM_LOAD( "065-a06.1f",  0x000000, 0x200000, CRC(5adbcee0) SHA1(435feda697193bc51db80eba46be474cbbc1de4b) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "xmen_aaa.nv", 0x0000, 0x0080, CRC(750fd447) SHA1(27884c1ceb0b5174f7d06e1e06bbbd6d6c5b47e7) )
ROM_END

ROM_START( xmen2pj )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "065-jaa04.10d",  0x00000, 0x20000, CRC(66746339) SHA1(8cc5f5deb4178b0444ffc5974940a30cb003114e) ) /* Japan 2 Player version */
	ROM_LOAD16_BYTE( "065-jaa05.10f",  0x00001, 0x20000, CRC(1215b706) SHA1(b746dedab9c509b5cd941f0f4ddd3709e8a58cce) )
	ROM_LOAD16_BYTE( "065-a02.9d",     0x80000, 0x40000, CRC(b31dc44c) SHA1(4bdac05826b4d6d4fe46686ede5190e2f73eefc5) )
	ROM_LOAD16_BYTE( "065-a03.9f",     0x80001, 0x40000, CRC(13842fe6) SHA1(b61f094eb94336edb8708d3437ead9b853b2d6e6) )

	ROM_REGION( 0x30000, "audiocpu", 0 )		/* 64k+128k for sound cpu */
	ROM_LOAD( "065-a01.6f",   0x00000, 0x20000, CRC(147d3a4d) SHA1(a14409fe991e803b9e7812303e3a9ebd857d8b01) )
	ROM_RELOAD(               0x10000, 0x20000 )

	ROM_REGION( 0x200000, "gfx1", 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD( "065-a08.15l",   0x000000, 0x100000, CRC(6b649aca) SHA1(2595f314517738e8614facf578cc951a6c36a180) )	/* tiles */
	ROM_LOAD( "065-a07.16l",   0x100000, 0x100000, CRC(c5dc8fc4) SHA1(9887cb002c8b72be7ce933cb397f00cdc5506c8c) )

	ROM_REGION( 0x400000, "gfx2", 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD( "065-a09.2h",  0x000000, 0x100000, CRC(ea05d52f) SHA1(7f2c14f907355856fb94e3a67b73aa1919776835) )	/* sprites */
	ROM_LOAD( "065-a10.2l",  0x100000, 0x100000, CRC(96b91802) SHA1(641943557b59b91f0edd49ec8a73cef7d9268b32) )
	ROM_LOAD( "065-a12.1h",  0x200000, 0x100000, CRC(321ed07a) SHA1(5b00ed676daeea974bdce6701667cfe573099dad) )
	ROM_LOAD( "065-a11.1l",  0x300000, 0x100000, CRC(46da948e) SHA1(168ac9178ee5bad5931557fb549e1237971d7839) )

	ROM_REGION( 0x200000, "k054539", 0 )	/* samples for the 054539 */
	ROM_LOAD( "065-a06.1f",  0x000000, 0x200000, CRC(5adbcee0) SHA1(435feda697193bc51db80eba46be474cbbc1de4b) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "xmen_jaa.nv", 0x0000, 0x0080, CRC(849a9e19) SHA1(bd335a2d33bf4433de4fd57b8108b216eb3a2cf1) )
ROM_END

/*

 KONAMI - X-MEN 2P/4P/6P - GX065 - PWB352532B

    +--------------------------------------------------------------------------------------+
    |                                                                                      |
    |                      [  065A06.1d  ]                 [  065A07.1h  ][  065A08.1l  ]  |
    |                                                                                      |
    |       [  054544  ]                                                                   |
    |                      [  054539  ]     [  053251  ]   [  051962  ] [  052109  ]       |
    |                                                                                      |
    |         [ YM2151 ]                                                                   |
    | sound                     [qz 24Mhz]                                                 |
    |  out                                                                                 |
    +-+  [  065*01.7b  ]                                                                   |
      |                                                                                    |
    +-+                                                                                    |
    |      [    Z80    ]                                                                   |
    |                                                                                      |
    +-+                                                                                    |
    +-+                                                                                    |
    |   J                                                                                  |
    |   A                                                                           [0     |
    |   M                                                                            6     |
    |   M                                                                            5     |
    |   A                                                                            A     |
    |                                                                                0     |
    |                                      [  053253  ]   [  053247  ] [  053246  ]  9.12l]|
    |                                                                                      |
    |                                                                               [0     |
    |                                                                                6     |
    +-+                                                                              5     |
      |                                   [  065A02.17g ]  [  065A03.17j ]           A     |
    +-+                                                                              1     |
    | test                                [  065*04.18g ]  [  065*05.18j ]           0.17l]|
    |  sw                                                                                  |
    | [cn9 (6P)]                                 [     68000 - 16Mhz     ]                 |
    | [cn8 (5P)]                                                                           |
    | [cn7 (4P)]                      [qz 32Mhz/18.432Mhz] [  065A12.22h ][  065A11.22l ]  |
    | [cn6 (3P)]                                                                           |
    |    rgb out                                                                           |
    +--------------------------------------------------------------------------------------+

    054544 *
    054539 *
    053251 *
    051962 *
    052109 *
    053253 - not on other version?
    053247 *
    053246 *

*/

ROM_START( xmen6p )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "065-ecb04.18g",  0x00000, 0x20000, CRC(258eb21f) SHA1(f1a22a880245f28195e5b6519822c0aa3b166541) ) /* Euro 6 Player version */
	ROM_LOAD16_BYTE( "065-ecb05.18j",  0x00001, 0x20000, CRC(25997bcd) SHA1(86fb1c64e133b7ca59ffb3910b62b61ee372c71a) )
	ROM_LOAD16_BYTE( "065-a02.17g",   0x80000, 0x40000, CRC(b31dc44c) SHA1(4bdac05826b4d6d4fe46686ede5190e2f73eefc5) )
	ROM_LOAD16_BYTE( "065-a03.17j",   0x80001, 0x40000, CRC(13842fe6) SHA1(b61f094eb94336edb8708d3437ead9b853b2d6e6) )

	ROM_REGION( 0x30000, "audiocpu", 0 )		/* 64k+128k for sound cpu */
	ROM_LOAD( "065-a01.7b",   0x00000, 0x20000, CRC(147d3a4d) SHA1(a14409fe991e803b9e7812303e3a9ebd857d8b01) )
	ROM_RELOAD(               0x10000, 0x20000 )

	ROM_REGION( 0x200000, "gfx1", 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD( "065-a08.1l",   0x000000, 0x100000, CRC(6b649aca) SHA1(2595f314517738e8614facf578cc951a6c36a180) )	/* tiles */
	ROM_LOAD( "065-a07.1h",   0x100000, 0x100000, CRC(c5dc8fc4) SHA1(9887cb002c8b72be7ce933cb397f00cdc5506c8c) )

	ROM_REGION( 0x400000, "gfx2", 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD( "065-a09.12l",  0x000000, 0x100000, CRC(ea05d52f) SHA1(7f2c14f907355856fb94e3a67b73aa1919776835) )	/* sprites */
	ROM_LOAD( "065-a10.17l",  0x100000, 0x100000, CRC(96b91802) SHA1(641943557b59b91f0edd49ec8a73cef7d9268b32) )
	ROM_LOAD( "065-a12.22h",  0x200000, 0x100000, CRC(321ed07a) SHA1(5b00ed676daeea974bdce6701667cfe573099dad) )
	ROM_LOAD( "065-a11.22l",  0x300000, 0x100000, CRC(46da948e) SHA1(168ac9178ee5bad5931557fb549e1237971d7839) )

	ROM_REGION( 0x200000, "k054539", 0 )	/* samples for the 054539 */
	ROM_LOAD( "065-a06.1d",  0x000000, 0x200000, CRC(5adbcee0) SHA1(435feda697193bc51db80eba46be474cbbc1de4b) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "xmen_ecb.nv", 0x0000, 0x0080, CRC(462c6e1a) SHA1(a57087163d7a760d5922c70842cfae20e6a2f5b5) )
ROM_END


ROM_START( xmen6pu )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "065-ucb04.18g",  0x00000, 0x20000, CRC(0f09b8e0) SHA1(79f4d86d8ec45b39e34ddf45860bea0c74dae183) ) /* US 6 Player version */
	ROM_LOAD16_BYTE( "065-ucb05.18j",  0x00001, 0x20000, CRC(867becbf) SHA1(3f81f4dbd289f98b78d7821a8925598c771f01ef) )
	ROM_LOAD16_BYTE( "065-a02.17g",   0x80000, 0x40000, CRC(b31dc44c) SHA1(4bdac05826b4d6d4fe46686ede5190e2f73eefc5) )
	ROM_LOAD16_BYTE( "065-a03.17j",   0x80001, 0x40000, CRC(13842fe6) SHA1(b61f094eb94336edb8708d3437ead9b853b2d6e6) )

	ROM_REGION( 0x30000, "audiocpu", 0 )		/* 64k+128k for sound cpu */
	ROM_LOAD( "065-a01.7b",   0x00000, 0x20000, CRC(147d3a4d) SHA1(a14409fe991e803b9e7812303e3a9ebd857d8b01) )
	ROM_RELOAD(               0x10000, 0x20000 )

	ROM_REGION( 0x200000, "gfx1", 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD( "065-a08.1l",   0x000000, 0x100000, CRC(6b649aca) SHA1(2595f314517738e8614facf578cc951a6c36a180) )	/* tiles */
	ROM_LOAD( "065-a07.1h",   0x100000, 0x100000, CRC(c5dc8fc4) SHA1(9887cb002c8b72be7ce933cb397f00cdc5506c8c) )

	ROM_REGION( 0x400000, "gfx2", 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD( "065-a09.12l",  0x000000, 0x100000, CRC(ea05d52f) SHA1(7f2c14f907355856fb94e3a67b73aa1919776835) )	/* sprites */
	ROM_LOAD( "065-a10.17l",  0x100000, 0x100000, CRC(96b91802) SHA1(641943557b59b91f0edd49ec8a73cef7d9268b32) )
	ROM_LOAD( "065-a12.22h",  0x200000, 0x100000, CRC(321ed07a) SHA1(5b00ed676daeea974bdce6701667cfe573099dad) )
	ROM_LOAD( "065-a11.22l",  0x300000, 0x100000, CRC(46da948e) SHA1(168ac9178ee5bad5931557fb549e1237971d7839) )

	ROM_REGION( 0x200000, "k054539", 0 )	/* samples for the 054539 */
	ROM_LOAD( "065-a06.1d",  0x000000, 0x200000, CRC(5adbcee0) SHA1(435feda697193bc51db80eba46be474cbbc1de4b) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "xmen_ucb.nv", 0x0000, 0x0080, CRC(f3d0f682) SHA1(b0d4655c651238ae028ffb59a704acba798f93f8) )
ROM_END

GAME( 1992, xmen,    0,    xmen,   xmen,    0,   ROT0, "Konami", "X-Men (4 Players ver UBB)", GAME_SUPPORTS_SAVE ) /* Second "version" letter denotes players, A=2 players, B=4 players, C=6 players ??? */
GAME( 1992, xmenj,   xmen, xmen,   xmen,    0,   ROT0, "Konami", "X-Men (4 Players ver JBA)", GAME_SUPPORTS_SAVE )
GAME( 1992, xmene,   xmen, xmen,   xmen,    0,   ROT0, "Konami", "X-Men (4 Players ver EBA)", GAME_SUPPORTS_SAVE )
GAME( 1992, xmen2pe, xmen, xmen,   xmen2p,  0,   ROT0, "Konami", "X-Men (2 Players ver EAA)", GAME_SUPPORTS_SAVE )
GAME( 1992, xmen2pa, xmen, xmen,   xmen2p,  0,   ROT0, "Konami", "X-Men (2 Players ver AAA)", GAME_SUPPORTS_SAVE )
GAME( 1992, xmen2pj, xmen, xmen,   xmen2p,  0,   ROT0, "Konami", "X-Men (2 Players ver JAA)", GAME_SUPPORTS_SAVE )

GAME( 1992, xmen6p,  xmen, xmen6p, xmen6p,  0,   ROT0, "Konami", "X-Men (6 Players ver ECB)", GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE )
GAME( 1992, xmen6pu, xmen, xmen6p, xmen6p,  0,   ROT0, "Konami", "X-Men (6 Players ver UCB)", GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE )
