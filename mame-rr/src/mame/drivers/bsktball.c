/***************************************************************************

    Atari Basketball hardware

    driver by Mike Balfour

    Games supported:
        * Basketball

    Known issues:
        * none at this time

****************************************************************************

    Note:  The original hardware uses the Player 1 and Player 2 Start buttons
    as the Jump/Shoot buttons.

    If you have any questions about how this driver works, don't hesitate to
    ask.  - Mike Balfour (mab22@po.cwru.edu)

    2008-07
    Dip locations verified with manual

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "deprecat.h"
#include "includes/bsktball.h"
#include "sound/discrete.h"


/*************************************
 *
 *  Palette generation
 *
 *************************************/

static PALETTE_INIT( bsktball )
{
	int i;

	machine->colortable = colortable_alloc(machine, 4);

	colortable_palette_set_color(machine->colortable,0,MAKE_RGB(0x00,0x00,0x00)); /* BLACK */
	colortable_palette_set_color(machine->colortable,1,MAKE_RGB(0x80,0x80,0x80)); /* LIGHT GREY */
	colortable_palette_set_color(machine->colortable,2,MAKE_RGB(0x50,0x50,0x50)); /* DARK GREY */
	colortable_palette_set_color(machine->colortable,3,MAKE_RGB(0xff,0xff,0xff)); /* WHITE */

	/* playfield */
	for (i = 0; i < 2; i++)
	{
		colortable_entry_set_value(machine->colortable, i*4 + 0, 1);
		colortable_entry_set_value(machine->colortable, i*4 + 1, 3 * i);
		colortable_entry_set_value(machine->colortable, i*4 + 2, 3 * i);
		colortable_entry_set_value(machine->colortable, i*4 + 3, 3 * i);
	}

	/* motion */
	for (i = 0; i < 4*4*4; i++)
	{
		colortable_entry_set_value(machine->colortable, 2*4 + i*4 + 0, 1);
		colortable_entry_set_value(machine->colortable, 2*4 + i*4 + 1, (i >> 2) & 3);
		colortable_entry_set_value(machine->colortable, 2*4 + i*4 + 2, (i >> 0) & 3);
		colortable_entry_set_value(machine->colortable, 2*4 + i*4 + 3, (i >> 4) & 3);
	}
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0x3fff)
	AM_RANGE(0x0000, 0x01ff) AM_RAM /* Zero Page RAM */
	AM_RANGE(0x0800, 0x0800) AM_READ(bsktball_in0_r)
	AM_RANGE(0x0802, 0x0802) AM_READ_PORT("IN1")
	AM_RANGE(0x0803, 0x0803) AM_READ_PORT("DSW")
	AM_RANGE(0x1000, 0x1000) AM_WRITENOP /* Timer Reset */
	AM_RANGE(0x1010, 0x1010) AM_DEVWRITE("discrete", bsktball_bounce_w) /* Crowd Amp / Bounce */
	AM_RANGE(0x1022, 0x1023) AM_WRITENOP /* Coin Counter */
	AM_RANGE(0x1024, 0x1025) AM_WRITE(bsktball_led1_w) /* LED 1 */
	AM_RANGE(0x1026, 0x1027) AM_WRITE(bsktball_led2_w) /* LED 2 */
	AM_RANGE(0x1028, 0x1029) AM_WRITE(bsktball_ld1_w) /* LD 1 */
	AM_RANGE(0x102a, 0x102b) AM_WRITE(bsktball_ld2_w) /* LD 2 */
	AM_RANGE(0x102c, 0x102d) AM_DEVWRITE("discrete", bsktball_noise_reset_w) /* Noise Reset */
	AM_RANGE(0x102e, 0x102f) AM_WRITE(bsktball_nmion_w) /* NMI On */
	AM_RANGE(0x1030, 0x1030) AM_DEVWRITE("discrete", bsktball_note_w) /* Music Ckt Note Dvsr */
	AM_RANGE(0x1800, 0x1bbf) AM_RAM_WRITE(bsktball_videoram_w) AM_BASE_MEMBER(bsktball_state, videoram) /* DISPLAY */
	AM_RANGE(0x1bc0, 0x1bff) AM_RAM AM_BASE_MEMBER(bsktball_state, motion)
	AM_RANGE(0x1c00, 0x1cff) AM_RAM
	AM_RANGE(0x2000, 0x3fff) AM_ROM /* PROGRAM */
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( bsktball )
	PORT_START("TRACK0_X")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) /* Sensitivity, clip, min, max */

	PORT_START("TRACK0_Y")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10)

	PORT_START("TRACK1_X")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2) /* Sensitivity, clip, min, max */

	PORT_START("TRACK1_Y")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_UNUSED )
	/* 0x04 - SPARE */
	/* 0x08 - SPARE */
	/* 0x10 - DR0 = PL2 H DIR */
	/* 0x20 - DR1 = PL2 V DIR */
	/* 0x40 - DR2 = PL1 H DIR */
	/* 0x80 - DR3 = PL1 V DIR */

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_VBLANK )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* SPARE */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* TEST STEP */
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* COIN 0 */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 ) /* COIN 1 */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) /* COIN 2 */

	PORT_START("DSW")
	PORT_DIPNAME( 0x07, 0x00, "Play Time per Credit" ) PORT_DIPLOCATION("SW:1,2,3")
	PORT_DIPSETTING(	0x07, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(	0x06, "2:30" )
	PORT_DIPSETTING(	0x05, "2:00" )
	PORT_DIPSETTING(	0x04, "1:30" )
	PORT_DIPSETTING(	0x03, "1:15" )
	PORT_DIPSETTING(	0x02, "0:45" )
	PORT_DIPSETTING(	0x01, "0:30" )
	PORT_DIPSETTING(	0x00, "1:00" )
	PORT_DIPNAME( 0x18, 0x00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW:4,5")
	PORT_DIPSETTING(	0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(	0x10, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(	0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(	0x18, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x20, 0x00, "Cost" ) PORT_DIPLOCATION("SW:6")
	PORT_DIPSETTING(	0x20, "Two Coin Minimum" )
	PORT_DIPSETTING(	0x00, "One Coin Minimum" )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Language ) ) PORT_DIPLOCATION("SW:7,8")
	PORT_DIPSETTING(	0xc0, DEF_STR( German ) )
	PORT_DIPSETTING(	0x80, DEF_STR( French ) )
	PORT_DIPSETTING(	0x40, DEF_STR( Spanish ) )
	PORT_DIPSETTING(	0x00, DEF_STR( English ) )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics layouts
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,
	64,
	2,
	{ 0, 8*0x800 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


static const gfx_layout motionlayout =
{
	8,32,
	64,
	2,
	{ 0, 8*0x800 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{	0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8,
		16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8,
		24*8, 25*8, 26*8, 27*8, 28*8, 29*8, 30*8, 31*8 },
	32*8
};


static GFXDECODE_START( bsktball )
	GFXDECODE_ENTRY( "gfx1", 0x0600, charlayout,   0x00, 0x02 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, motionlayout, 0x08, 0x40 )
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_START( bsktball )
{
	bsktball_state *state = (bsktball_state *)machine->driver_data;

	state_save_register_global(machine, state->nmi_on);
	state_save_register_global(machine, state->i256v);
	state_save_register_global(machine, state->ld1);
	state_save_register_global(machine, state->ld2);
	state_save_register_global(machine, state->dir0);
	state_save_register_global(machine, state->dir1);
	state_save_register_global(machine, state->dir2);
	state_save_register_global(machine, state->dir3);
	state_save_register_global(machine, state->last_p1_horiz);
	state_save_register_global(machine, state->last_p1_vert);
	state_save_register_global(machine, state->last_p2_horiz);
	state_save_register_global(machine, state->last_p2_vert);
}

static MACHINE_RESET( bsktball )
{
	bsktball_state *state = (bsktball_state *)machine->driver_data;

	state->nmi_on = 0;
	state->i256v = 0;
	state->ld1 = 0;
	state->ld2 = 0;
	state->dir0 = 0;
	state->dir1 = 0;
	state->dir2 = 0;
	state->dir3 = 0;
	state->last_p1_horiz = 0;
	state->last_p1_vert = 0;
	state->last_p2_horiz = 0;
	state->last_p2_vert = 0;
}


static MACHINE_DRIVER_START( bsktball )

	/* driver data */
	MDRV_DRIVER_DATA(bsktball_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M6502,750000)
	MDRV_CPU_PROGRAM_MAP(main_map)
	MDRV_CPU_VBLANK_INT_HACK(bsktball_interrupt,8)

	MDRV_MACHINE_START(bsktball)
	MDRV_MACHINE_RESET(bsktball)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 28*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 28*8-1)

	MDRV_GFXDECODE(bsktball)
	MDRV_PALETTE_LENGTH(2*4 + 4*4*4*4)

	MDRV_PALETTE_INIT(bsktball)
	MDRV_VIDEO_START(bsktball)
	MDRV_VIDEO_UPDATE(bsktball)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("discrete", DISCRETE, 0)
	MDRV_SOUND_CONFIG_DISCRETE(bsktball)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( bsktball )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "034765.d1",    0x2000, 0x0800, CRC(798cea39) SHA1(b1b709a74258b01b21d7c2038a3b6abe879944c5) )
	ROM_LOAD( "034764.c1",    0x2800, 0x0800, CRC(a087109e) SHA1(f5d6dcccc4a54db35be3d8997bc51e73892747fb) )
	ROM_LOAD( "034766.f1",    0x3000, 0x0800, CRC(a82e9a9f) SHA1(9aca236c5145c04a8aaebb316179482bbdc9ddfc) )
	ROM_LOAD( "034763.b1",    0x3800, 0x0800, CRC(1fc69359) SHA1(a215ba3bb18ea2c57c443dfc4c4a0a3846bbedfe) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "034757.a6",    0x0000, 0x0800, CRC(010e8ad3) SHA1(43ce2c2089ec3011e2d28e8257a35efeed0e71c5) )
	ROM_LOAD( "034758.b6",    0x0800, 0x0800, CRC(f7bea344) SHA1(df544bff67bb0334f77cef11792199d9c3f5fdf4) )
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1979, bsktball, 0, bsktball, bsktball, 0, ROT0, "Atari", "Basketball", GAME_SUPPORTS_SAVE )
