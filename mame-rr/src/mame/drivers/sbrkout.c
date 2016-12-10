/***************************************************************************

    Atari Super Breakout hardware

    driver by Mike Balfour

    Games supported:
        * Super Breakout

    Known issues:
        * none at this time

****************************************************************************

    To do:

    * Merge with Sprint 1

****************************************************************************

    Stephh's notes (based on the games M6502 code and some tests) :

      - Each time the game is reset, it is set to "Cavity".
        I can't remember if it's the correct behaviour or not,
        but the VBLANK interruption is not called in "demo mode".
      - You can only select the game after 1st coin is inserted
        and before you press BUTTON1 to launch the first ball,
        then the VBLANK interruption is no more called.
        This means that player 2 plays the same game as player 1.

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "sound/dac.h"

#include "sbrkout.lh"



/*************************************
 *
 *  Constants
 *
 *************************************/

#define MAIN_CLOCK		XTAL_12_096MHz
#define TIME_4V 		ATTOTIME_IN_HZ(MAIN_CLOCK/2/256/2/4)



/*************************************
 *
 *  Globals
 *
 *************************************/

static emu_timer *scanline_timer;
static emu_timer *pot_timer;

static tilemap_t *bg_tilemap;

static UINT8 sync2_value;
static UINT8 pot_mask[2];
static UINT8 pot_trigger[2];



/*************************************
 *
 *  Prototypes
 *
 *************************************/

static TIMER_CALLBACK( scanline_callback );
static TIMER_CALLBACK( pot_trigger_callback );



/*************************************
 *
 *  Machine setup
 *
 *************************************/

static MACHINE_START( sbrkout )
{
	memory_set_bankptr(machine, "bank1", &machine->generic.videoram.u8[0x380]);
	scanline_timer = timer_alloc(machine, scanline_callback, NULL);
	pot_timer = timer_alloc(machine, pot_trigger_callback, NULL);

	state_save_register_global(machine, sync2_value);
	state_save_register_global_array(machine, pot_mask);
	state_save_register_global_array(machine, pot_trigger);
}


static MACHINE_RESET( sbrkout )
{
	timer_adjust_oneshot(scanline_timer, machine->primary_screen->time_until_pos(0), 0);
}



/*************************************
 *
 *  Interrupts
 *
 *************************************/

static TIMER_CALLBACK( scanline_callback )
{
	int scanline = param;

	/* force a partial update before anything happens */
	machine->primary_screen->update_partial(scanline);

	/* if this is a rising edge of 16V, assert the CPU interrupt */
	if (scanline % 32 == 16)
		cputag_set_input_line(machine, "maincpu", 0, ASSERT_LINE);

	/* update the DAC state */
	dac_data_w(machine->device("dac"), (machine->generic.videoram.u8[0x380 + 0x11] & (scanline >> 2)) ? 255 : 0);

	/* on the VBLANK, read the pot and schedule an interrupt time for it */
	if (scanline == machine->primary_screen->visible_area().max_y + 1)
	{
		UINT8 potvalue = input_port_read(machine, "PADDLE");
		timer_adjust_oneshot(pot_timer, machine->primary_screen->time_until_pos(56 + (potvalue / 2), (potvalue % 2) * 128), 0);
	}

	/* call us back in 4 scanlines */
	scanline += 4;
	if (scanline >= machine->primary_screen->height())
		scanline = 0;
	timer_adjust_oneshot(scanline_timer, machine->primary_screen->time_until_pos(scanline), scanline);
}


static WRITE8_HANDLER( irq_ack_w )
{
	cputag_set_input_line(space->machine, "maincpu", 0, CLEAR_LINE);
}



/*************************************
 *
 *  Inputs
 *
 *************************************/

static READ8_HANDLER( switches_r )
{
	UINT8 result = 0xff;

	/* DIP switches are selected by ADR0+ADR1 if ADR3 == 0 */
	if ((offset & 0x0b) == 0x00)
		result &= (input_port_read(space->machine, "DIPS") << 6) | 0x3f;
	if ((offset & 0x0b) == 0x01)
		result &= (input_port_read(space->machine, "DIPS") << 4) | 0x3f;
	if ((offset & 0x0b) == 0x02)
		result &= (input_port_read(space->machine, "DIPS") << 0) | 0x3f;
	if ((offset & 0x0b) == 0x03)
		result &= (input_port_read(space->machine, "DIPS") << 2) | 0x3f;

	/* other switches are selected by ADR0+ADR1+ADR2 if ADR4 == 0 */
	if ((offset & 0x17) == 0x00)
		result &= (input_port_read(space->machine, "SELECT") << 7) | 0x7f;
	if ((offset & 0x17) == 0x04)
		result &= ((pot_trigger[0] & ~pot_mask[0]) << 7) | 0x7f;
	if ((offset & 0x17) == 0x05)
		result &= ((pot_trigger[1] & ~pot_mask[1]) << 7) | 0x7f;
	if ((offset & 0x17) == 0x06)
		result &= input_port_read(space->machine, "SERVE");
	if ((offset & 0x17) == 0x07)
		result &= (input_port_read(space->machine, "SELECT") << 6) | 0x7f;

	return result;
}


static void update_nmi_state(running_machine *machine)
{
	if ((pot_trigger[0] & ~pot_mask[0]) | (pot_trigger[1] & ~pot_mask[1]))
		cputag_set_input_line(machine, "maincpu", INPUT_LINE_NMI, ASSERT_LINE);
	else
		cputag_set_input_line(machine, "maincpu", INPUT_LINE_NMI, CLEAR_LINE);
}


static TIMER_CALLBACK( pot_trigger_callback )
{
	pot_trigger[param] = 1;
	update_nmi_state(machine);
}


static WRITE8_HANDLER( pot_mask1_w )
{
	pot_mask[0] = ~offset & 1;
	pot_trigger[0] = 0;
	update_nmi_state(space->machine);
}


static WRITE8_HANDLER( pot_mask2_w )
{
	pot_mask[1] = ~offset & 1;
	pot_trigger[1] = 0;
	update_nmi_state(space->machine);
}



/*************************************
 *
 *  Lamps and other outputs
 *
 *************************************/

/*
    The LEDs are turned on and off by two consecutive memory addresses.  The
    first address turns them off, the second address turns them on.  This is
    reversed for the Serve LED, which has a NOT on the signal.
*/

static WRITE8_HANDLER( start_1_led_w )
{
	output_set_led_value(0, offset & 1);
}


static WRITE8_HANDLER( start_2_led_w )
{
	output_set_led_value(1, offset & 1);
}


static WRITE8_HANDLER( serve_led_w )
{
	output_set_led_value(0, ~offset & 1);
}


static WRITE8_HANDLER( coincount_w )
{
	coin_counter_w(space->machine, 0, offset & 1);
}



/*************************************
 *
 *  Video timing
 *
 *************************************/

static READ8_HANDLER( sync_r )
{
	int hpos = space->machine->primary_screen->hpos();
	sync2_value = (hpos >= 128 && hpos <= space->machine->primary_screen->visible_area().max_x);
	return space->machine->primary_screen->vpos();
}


static READ8_HANDLER( sync2_r )
{
	return (sync2_value << 7) | 0x7f;
}



/*************************************
 *
 *  Background tilemap
 *
 *************************************/

static TILE_GET_INFO( get_bg_tile_info )
{
	int code = (machine->generic.videoram.u8[tile_index] & 0x80) ? machine->generic.videoram.u8[tile_index] : 0;
	SET_TILE_INFO(0, code, 0, 0);
}


static VIDEO_START( sbrkout )
{
	bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
}


static WRITE8_HANDLER( sbrkout_videoram_w )
{
	space->machine->generic.videoram.u8[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}



/*************************************
 *
 *  Video update
 *
 *************************************/

static VIDEO_UPDATE( sbrkout )
{
	int ball;

	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);

	for (ball = 2; ball >= 0; ball--)
	{
		int code = ((screen->machine->generic.videoram.u8[0x380 + 0x18 + ball * 2 + 1] & 0x80) >> 7);
		int sx = 31 * 8 - screen->machine->generic.videoram.u8[0x380 + 0x10 + ball * 2];
		int sy = 30 * 8 - screen->machine->generic.videoram.u8[0x380 + 0x18 + ball * 2];

		drawgfx_transpen(bitmap, cliprect, screen->machine->gfx[1], code, 0, 0, 0, sx, sy, 0);
	}
	return 0;
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

/* full memory map derived from schematics */
static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0x3fff)
	AM_RANGE(0x0000, 0x007f) AM_MIRROR(0x380) AM_RAMBANK("bank1")
	AM_RANGE(0x0400, 0x07ff) AM_RAM_WRITE(sbrkout_videoram_w) AM_BASE_GENERIC(videoram)
	AM_RANGE(0x0800, 0x083f) AM_READ(switches_r)
	AM_RANGE(0x0840, 0x0840) AM_MIRROR(0x003f) AM_READ_PORT("COIN")
	AM_RANGE(0x0880, 0x0880) AM_MIRROR(0x003f) AM_READ_PORT("START")
	AM_RANGE(0x08c0, 0x08c0) AM_MIRROR(0x003f) AM_READ_PORT("SERVICE")
	AM_RANGE(0x0c00, 0x0c00) AM_MIRROR(0x03ff) AM_READ(sync_r)
	AM_RANGE(0x0c10, 0x0c11) AM_MIRROR(0x000e) AM_WRITE(serve_led_w)
	AM_RANGE(0x0c30, 0x0c31) AM_MIRROR(0x000e) AM_WRITE(start_1_led_w)
	AM_RANGE(0x0c40, 0x0c41) AM_MIRROR(0x000e) AM_WRITE(start_2_led_w)
	AM_RANGE(0x0c50, 0x0c51) AM_MIRROR(0x000e) AM_WRITE(pot_mask1_w)
	AM_RANGE(0x0c60, 0x0c61) AM_MIRROR(0x000e) AM_WRITE(pot_mask2_w)
	AM_RANGE(0x0c70, 0x0c71) AM_MIRROR(0x000e) AM_WRITE(coincount_w)
	AM_RANGE(0x0c80, 0x0c80) AM_MIRROR(0x007f) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x0e00, 0x0e00) AM_MIRROR(0x007f) AM_WRITE(irq_ack_w)
	AM_RANGE(0x1000, 0x1000) AM_MIRROR(0x03ff) AM_READ(sync2_r)
	AM_RANGE(0x2800, 0x3fff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( sbrkout )
	PORT_START("DIPS")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Language ) )
	PORT_DIPSETTING(	0x00, DEF_STR( English ) )
	PORT_DIPSETTING(	0x01, DEF_STR( German ) )
	PORT_DIPSETTING(	0x02, DEF_STR( French ) )
	PORT_DIPSETTING(	0x03, DEF_STR( Spanish ) )
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Coinage ) )
	PORT_DIPSETTING(	0x0c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(	0x08, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(	0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x70, 0x00, "Extended Play" )
	/* Progressive */
	PORT_DIPSETTING(	0x10, "200" )	PORT_CONDITION("SELECT",0x03,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(	0x20, "400" )	PORT_CONDITION("SELECT",0x03,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(	0x30, "600" )	PORT_CONDITION("SELECT",0x03,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(	0x40, "900" )	PORT_CONDITION("SELECT",0x03,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(	0x50, "1200" )	PORT_CONDITION("SELECT",0x03,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(	0x60, "1600" )	PORT_CONDITION("SELECT",0x03,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(	0x70, "2000" )	PORT_CONDITION("SELECT",0x03,PORTCOND_EQUALS,0x00)
	/* Double */
	PORT_DIPSETTING(	0x10, "200" )	PORT_CONDITION("SELECT",0x03,PORTCOND_EQUALS,0x02)
	PORT_DIPSETTING(	0x20, "400" )	PORT_CONDITION("SELECT",0x03,PORTCOND_EQUALS,0x02)
	PORT_DIPSETTING(	0x30, "600" )	PORT_CONDITION("SELECT",0x03,PORTCOND_EQUALS,0x02)
	PORT_DIPSETTING(	0x40, "800" )	PORT_CONDITION("SELECT",0x03,PORTCOND_EQUALS,0x02)
	PORT_DIPSETTING(	0x50, "1000" )	PORT_CONDITION("SELECT",0x03,PORTCOND_EQUALS,0x02)
	PORT_DIPSETTING(	0x60, "1200" )	PORT_CONDITION("SELECT",0x03,PORTCOND_EQUALS,0x02)
	PORT_DIPSETTING(	0x70, "1500" )	PORT_CONDITION("SELECT",0x03,PORTCOND_EQUALS,0x02)
	/* Cavity */
	PORT_DIPSETTING(	0x10, "200" )	PORT_CONDITION("SELECT",0x03,PORTCOND_EQUALS,0x01)
	PORT_DIPSETTING(	0x20, "300" )	PORT_CONDITION("SELECT",0x03,PORTCOND_EQUALS,0x01)
	PORT_DIPSETTING(	0x30, "400" )	PORT_CONDITION("SELECT",0x03,PORTCOND_EQUALS,0x01)
	PORT_DIPSETTING(	0x40, "700" )	PORT_CONDITION("SELECT",0x03,PORTCOND_EQUALS,0x01)
	PORT_DIPSETTING(	0x50, "900" )	PORT_CONDITION("SELECT",0x03,PORTCOND_EQUALS,0x01)
	PORT_DIPSETTING(	0x60, "1100" )	PORT_CONDITION("SELECT",0x03,PORTCOND_EQUALS,0x01)
	PORT_DIPSETTING(	0x70, "1400" )	PORT_CONDITION("SELECT",0x03,PORTCOND_EQUALS,0x01)
	PORT_DIPSETTING(	0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Lives ) )
	PORT_DIPSETTING(	0x80, "3" )
	PORT_DIPSETTING(	0x00, "5" )

	PORT_START("COIN")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN2 )

	PORT_START("START")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SERVICE")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_TILT )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("SERVE")
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 )

	PORT_START("PADDLE")
	PORT_BIT( 0xff, 0x00, IPT_PADDLE ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_CENTERDELTA(0)

	PORT_START("SELECT")		/* IN6 - fake port, used to set the game select dial */
	PORT_CONFNAME( 0x03, 0x00, "Game Select" )
	PORT_CONFSETTING(    0x00, "Progressive" )
	PORT_CONFSETTING(    0x02, "Double" )
	PORT_CONFSETTING(    0x01, "Cavity" )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,
	64,
	1,
	{ 0 },
	{ 4, 5, 6, 7, 0x200*8 + 4, 0x200*8 + 5, 0x200*8 + 6, 0x200*8 + 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


static const gfx_layout balllayout =
{
	3,3,
	2,
	1,
	{ 0 },
	{ 0, 1, 2 },
	{ 0*8, 1*8, 2*8 },
	3*8
};


static GFXDECODE_START( sbrkout )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, balllayout, 0, 1 )
GFXDECODE_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_DRIVER_START( sbrkout )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M6502,MAIN_CLOCK/16)	   /* 375 KHz? Should be 750KHz? */
	MDRV_CPU_PROGRAM_MAP(main_map)

	MDRV_MACHINE_START(sbrkout)
	MDRV_MACHINE_RESET(sbrkout)
	MDRV_WATCHDOG_VBLANK_INIT(8)

	/* video hardware */
	MDRV_GFXDECODE(sbrkout)
	MDRV_PALETTE_LENGTH(2)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_RAW_PARAMS(MAIN_CLOCK/2, 384, 0, 256, 262, 0, 224)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)

	MDRV_PALETTE_INIT(black_and_white)
	MDRV_VIDEO_START(sbrkout)
	MDRV_VIDEO_UPDATE(sbrkout)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("dac", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

// see page 3-21 of schematics which explain the 2 versions.
// the rev 01 and rev 02 are not mentioned in the tm-118 manual despite it being a first printing
// hence they are probably prototypes. neither is dumped.

ROM_START( sbrkout ) // rev 04
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "033453.c1",    0x2800, 0x0800, CRC(a35d00e3) SHA1(53617ed1d362e82d6f45abd66056bffe23300e3b) )
	ROM_LOAD( "033454.d1",    0x3000, 0x0800, CRC(d42ea79a) SHA1(66c9b29226cde36d1ac6d1e81f34ebb5c79eded4) )
	ROM_LOAD( "033455.e1",    0x3800, 0x0800, CRC(e0a6871c) SHA1(1bdfa73d7b8d91e1c68b7847fc310cac314ee02d) )

	ROM_REGION( 0x0400, "gfx1", 0 )
	ROM_LOAD( "033280.p4",    0x0000, 0x0200, CRC(5a69ce85) SHA1(ad9078d12495c350738bdb0b1e1b6120d9e01f60) )
	ROM_LOAD( "033281.r4",    0x0200, 0x0200, CRC(066bd624) SHA1(cfb86c7013a70b8375126b23a4e66df5f3b9186b) )

	ROM_REGION( 0x0020, "gfx2", 0 )
	ROM_LOAD( "033282.k6",    0x0000, 0x0020, CRC(6228736b) SHA1(bc176261dba11521df19d545ce604f8cc294287a) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "006400.m2",    0x0000, 0x0100, CRC(b8094b4c) SHA1(82dc6799a19984f3b204ee3aeeb007e55afc8be3) )	/* sync (not used) */
	ROM_LOAD( "006401.e2",    0x0100, 0x0020, CRC(857df8db) SHA1(06313d5bde03220b2bc313d18e50e4bb1d0cfbbb) )	/* memory mapper */
ROM_END


ROM_START( sbrkout3 ) // rev 03; main cpu roms are on 1024x4bit (82s137 or equiv) proms, otherwise seems identical to rev 04
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD_NIB_HIGH( "33442-01.kl0",    0x2C00, 0x0400, CRC(FB5CB68A) SHA1(301E8E47F6A82D6C2290A890BCD5C53D61D58FF7) )
	ROM_LOAD_NIB_LOW( "33448-01.kl1",    0x2C00, 0x0400, CRC(B1D2B269) SHA1(46DE71F1F9695F03465FD9B2289B5C5FFE19B3A2) )
	ROM_LOAD_NIB_HIGH( "33443-01.l0",    0x3000, 0x0400, CRC(1E7D059F) SHA1(E1831FEBFD26CF2560351D45F37763A7498C029E) )
	ROM_LOAD_NIB_LOW( "33449-01.l1",    0x3000, 0x0400, CRC(F936918D) SHA1(9D62FE75D39F95085A4380059C4980F3AFFE1BBF) )
	ROM_LOAD_NIB_HIGH( "33444-01.m0",    0x3400, 0x0400, CRC(5B7E0E3B) SHA1(4DBD62B23249FBB05E1FFFE50B89A5E280A2DDE9) )
	ROM_LOAD_NIB_LOW( "33450-01.m1",    0x3400, 0x0400, CRC(430CF9E8) SHA1(8E6075F12DBE0B973500D4E38E0090E40EE47260) )
	ROM_LOAD_NIB_HIGH( "33445-01.n0",    0x3800, 0x0400, CRC(CDF19919) SHA1(13623BDE69E7F352BEAEF33524F69D74C540E1CC) )
	ROM_LOAD_NIB_LOW( "33451-01.n1",    0x3800, 0x0400, CRC(19F7C50D) SHA1(91BA9EF7AB4B200A55AE7B7979F4A01E617DD9AD) )
	ROM_LOAD_NIB_HIGH( "33446-01.p0",    0x3C00, 0x0400, CRC(9553663C) SHA1(6C28B3A11B7FF0AA224BF262C664A62166DC9CDF) )
	ROM_LOAD_NIB_LOW( "33452-01.p1",    0x3C00, 0x0400, CRC(6DC0439A) SHA1(9CC0B735935A610519EB1B53ED303223E69AF0B7) )

	ROM_REGION( 0x0400, "gfx1", 0 )
	ROM_LOAD( "033280.p4",    0x0000, 0x0200, CRC(5a69ce85) SHA1(ad9078d12495c350738bdb0b1e1b6120d9e01f60) )
	ROM_LOAD( "033281.r4",    0x0200, 0x0200, CRC(066bd624) SHA1(cfb86c7013a70b8375126b23a4e66df5f3b9186b) )

	ROM_REGION( 0x0020, "gfx2", 0 )
	ROM_LOAD( "033282.k6",    0x0000, 0x0020, CRC(6228736b) SHA1(bc176261dba11521df19d545ce604f8cc294287a) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "006400.m2",    0x0000, 0x0100, CRC(b8094b4c) SHA1(82dc6799a19984f3b204ee3aeeb007e55afc8be3) )	/* sync (not used) */
	ROM_LOAD( "006401.e2",    0x0100, 0x0020, CRC(857df8db) SHA1(06313d5bde03220b2bc313d18e50e4bb1d0cfbbb) )	/* memory mapper */
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAMEL( 1978, sbrkout, 0, sbrkout, sbrkout, 0, ROT270, "Atari", "Super Breakout (rev 04)", GAME_SUPPORTS_SAVE, layout_sbrkout )
GAMEL( 1978, sbrkout3, sbrkout, sbrkout, sbrkout, 0, ROT270, "Atari", "Super Breakout (rev 03)", GAME_SUPPORTS_SAVE, layout_sbrkout )
