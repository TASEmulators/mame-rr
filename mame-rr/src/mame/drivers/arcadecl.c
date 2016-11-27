/***************************************************************************

    Atari Arcade Classics hardware (prototypes)

    driver by Aaron Giles

    Games supported:
        * Arcade Classics (1992)
        * Sparkz (1982)

    Known bugs:
        * none at this time

****************************************************************************

    Memory map

****************************************************************************

    ========================================================================
    MAIN CPU
    ========================================================================
    000000-0FFFFF   R     xxxxxxxx xxxxxxxx   Program ROM
    200000-21FFFF   R/W   xxxxxxxx xxxxxxxx   Playfield RAM (512x256 pixels)
                    R/W   xxxxxxxx --------      (Left pixel)
                    R/W   -------- xxxxxxxx      (Right pixel)
    3C0000-3C01FF   R/W   xxxxxxxx xxxxxxxx   Playfield palette RAM (256 entries)
                    R/W   x------- --------      (RGB 1 LSB)
                    R/W   -xxxxx-- --------      (Red 5 MSB)
                    R/W   ------xx xxx-----      (Green 5 MSB)
                    R/W   -------- ---xxxxx      (Blue 5 MSB)
    3C0200-3C03FF   R/W   xxxxxxxx xxxxxxxx   Motion object palette RAM (256 entries)
    3C0400-3C07FF   R/W   xxxxxxxx xxxxxxxx   Extra palette RAM (512 entries)
    3E0000-3E07FF   R/W   xxxxxxxx xxxxxxxx   Motion object RAM (256 entries x 4 words)
                    R/W   -------- xxxxxxxx      (0: Link to next object)
                    R/W   x------- --------      (1: Horizontal flip)
                    R/W   -xxxxxxx xxxxxxxx      (1: Tile index)
                    R/W   xxxxxxxx x-------      (2: X position)
                    R/W   -------- ----xxxx      (2: Palette select)
                    R/W   xxxxxxxx x-------      (3: Y position)
                    R/W   -------- -xxx----      (3: Number of X tiles - 1)
                    R/W   -------- -----xxx      (3: Number of Y tiles - 1)
    3E0800-3EFFFF   R/W   xxxxxxxx xxxxxxxx   Extra sprite RAM
    640000          R     xxxxxxxx --------   Input port 1
    640002          R     xxxxxxxx --------   Input port 2
    640010          R     -------- xx------   Status port
                    R     -------- x-------      (VBLANK)
                    R     -------- -x------      (Self test)
    640012          R     -------- --xx--xx   Coin inputs
                    R     -------- --xx----      (Service coins)
                    R     -------- ------xx      (Coin switches)
    640020-640027   R     -------- xxxxxxxx   Analog inputs
    640040            W   -------- x--xxxxx   Sound control
                      W   -------- x-------      (ADPCM bank select)
                      W   -------- ---xxxxx      (Volume)
    640060            W   -------- --------   EEPROM enable
    641000-641FFF   R/W   -------- xxxxxxxx   EEPROM
    642000-642001   R/W   xxxxxxxx --------   MSM6295 communications
    646000            W   -------- --------   32V IRQ acknowledge
    647000            W   -------- --------   Watchdog reset
    ========================================================================
    Interrupts:
        IRQ4 = 32V
    ========================================================================

****************************************************************************/


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "includes/arcadecl.h"
#include "sound/okim6295.h"


#define MASTER_CLOCK		XTAL_14_31818MHz


/*************************************
 *
 *  Interrupt handling
 *
 *************************************/

static void update_interrupts(running_machine *machine)
{
	rampart_state *state = (rampart_state *)machine->driver_data;
	cputag_set_input_line(machine, "maincpu", 4, state->atarigen.scanline_int_state ? ASSERT_LINE : CLEAR_LINE);
}


static void scanline_update(screen_device &screen, int scanline)
{
	/* generate 32V signals */
	if ((scanline & 32) == 0)
		atarigen_scanline_int_gen(screen.machine->device("maincpu"));
}



/*************************************
 *
 *  Initialization
 *
 *************************************/

static MACHINE_START( arcadecl )
{
	atarigen_init(machine);
}


static MACHINE_RESET( arcadecl )
{
	rampart_state *state = (rampart_state *)machine->driver_data;

	atarigen_eeprom_reset(&state->atarigen);
	atarigen_interrupt_reset(&state->atarigen, update_interrupts);
	atarigen_scanline_timer_reset(*machine->primary_screen, scanline_update, 32);
}



/*************************************
 *
 *  Latch write
 *
 *************************************/

static WRITE16_HANDLER( latch_w )
{
	/* bit layout in this register:

        0x0080 == ADPCM bank
        0x001F == volume
    */

	/* lower byte being modified? */
	if (ACCESSING_BITS_0_7)
	{
		okim6295_device *oki = space->machine->device<okim6295_device>("oki");
		oki->set_bank_base((data & 0x80) ? 0x40000 : 0x00000);
		atarigen_set_oki6295_vol(space->machine, (data & 0x001f) * 100 / 0x1f);
	}
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x200000, 0x21ffff) AM_RAM AM_BASE_MEMBER(rampart_state, bitmap)
	AM_RANGE(0x3c0000, 0x3c07ff) AM_RAM_WRITE(atarigen_expanded_666_paletteram_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x3e0000, 0x3e07ff) AM_RAM_WRITE(atarimo_0_spriteram_w) AM_BASE(&atarimo_0_spriteram)
	AM_RANGE(0x3e0800, 0x3effbf) AM_RAM
	AM_RANGE(0x3effc0, 0x3effff) AM_RAM_WRITE(atarimo_0_slipram_w) AM_BASE(&atarimo_0_slipram)
	AM_RANGE(0x640000, 0x640001) AM_READ_PORT("PLAYER1")
	AM_RANGE(0x640002, 0x640003) AM_READ_PORT("PLAYER2")
	AM_RANGE(0x640010, 0x640011) AM_READ_PORT("STATUS")
	AM_RANGE(0x640012, 0x640013) AM_READ_PORT("COIN")
	AM_RANGE(0x640020, 0x640021) AM_READ_PORT("TRACKX2")
	AM_RANGE(0x640022, 0x640023) AM_READ_PORT("TRACKY2")
	AM_RANGE(0x640024, 0x640025) AM_READ_PORT("TRACKX1")
	AM_RANGE(0x640026, 0x640027) AM_READ_PORT("TRACKY1")
	AM_RANGE(0x640040, 0x64004f) AM_WRITE(latch_w)
	AM_RANGE(0x640060, 0x64006f) AM_WRITE(atarigen_eeprom_enable_w)
	AM_RANGE(0x641000, 0x641fff) AM_READWRITE(atarigen_eeprom_r, atarigen_eeprom_w) AM_BASE_SIZE_MEMBER(rampart_state, atarigen.eeprom, atarigen.eeprom_size)
	AM_RANGE(0x642000, 0x642001) AM_DEVREADWRITE8("oki", okim6295_r, okim6295_w, 0xff00)
	AM_RANGE(0x646000, 0x646fff) AM_WRITE(atarigen_scanline_int_ack_w)
	AM_RANGE(0x647000, 0x647fff) AM_WRITE(watchdog_reset16_w)
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( arcadecl )
	PORT_START("PLAYER1")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PLAYER2")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("STATUS")
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(  0x0004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(  0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE( 0x0040, IP_ACTIVE_LOW )
	PORT_BIT(  0x0080, IP_ACTIVE_HIGH, IPT_VBLANK )
	PORT_BIT(  0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("COIN")
	PORT_BIT(  0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT(  0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT(  0x000c, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT(  0x0010, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT(  0x0020, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT(  0xffc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("TRACKX2")
    PORT_BIT( 0x00ff, 0, IPT_TRACKBALL_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(32) PORT_REVERSE PORT_PLAYER(2)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("TRACKY2")
	PORT_BIT( 0x00ff, 0, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(32) PORT_PLAYER(2)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("TRACKX1")
    PORT_BIT( 0x00ff, 0, IPT_TRACKBALL_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(32) PORT_REVERSE PORT_PLAYER(1)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("TRACKY1")
    PORT_BIT( 0x00ff, 0, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(32) PORT_PLAYER(1)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( sparkz )
	PORT_START("PLAYER1")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)

	PORT_START("PLAYER2")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)

	PORT_START("STATUS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE( 0x0040, IP_ACTIVE_LOW )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_VBLANK )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("COIN")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x000c, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE2 )         /* not in "test mode" */
	PORT_BIT( 0xffc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("TRACKX2")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("TRACKY2")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("TRACKX1")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("TRACKY1")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout molayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0, 4, 8, 12, 16, 20, 24, 28 },
	{ 0*8, 4*8, 8*8, 12*8, 16*8, 20*8, 24*8, 28*8 },
	32*8
};


static GFXDECODE_START( arcadecl )
	GFXDECODE_ENTRY( "gfx1", 0, molayout,  256, 16 )
GFXDECODE_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_DRIVER_START( arcadecl )
	MDRV_DRIVER_DATA(rampart_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, MASTER_CLOCK)
	MDRV_CPU_PROGRAM_MAP(main_map)
	MDRV_CPU_VBLANK_INT("screen", atarigen_video_int_gen)

	MDRV_MACHINE_START(arcadecl)
	MDRV_MACHINE_RESET(arcadecl)
	MDRV_NVRAM_HANDLER(atarigen)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MDRV_GFXDECODE(arcadecl)
	MDRV_PALETTE_LENGTH(512)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	/* note: these parameters are from published specs, not derived */
	/* the board uses an SOS-2 chip to generate video signals */
	MDRV_SCREEN_RAW_PARAMS(MASTER_CLOCK/2, 456, 0+12, 336+12, 262, 0, 240)

	MDRV_VIDEO_START(arcadecl)
	MDRV_VIDEO_UPDATE(arcadecl)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_OKIM6295_ADD("oki", MASTER_CLOCK/4/3, OKIM6295_PIN7_LOW)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( arcadecl )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "pgm0",  0x00000, 0x80000, CRC(b5b93623) SHA1(a2e96c0c6eceb3d8f205e28d6b8197055aeb8cc4) )
	ROM_LOAD16_BYTE( "prog1", 0x00001, 0x80000, CRC(e7efef85) SHA1(05f2119d8ecc27f6efea85f5174ea7da404d7e9b) )

	ROM_REGION( 0x80000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "atcl_mob",   0x00000, 0x80000, CRC(0e9b3930) SHA1(51a449b79235d6ca5189e671399acff72a2d3dc8) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "adpcm",      0x00000, 0x80000, CRC(03ca7f03) SHA1(87ff53599b6f0cdfa5a1779773e09cc5cfe3c2a8) )
ROM_END


ROM_START( sparkz )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sparkzpg.0", 0x00000, 0x80000, CRC(a75c331c) SHA1(855ed44bd23c1dd0ca64926cacc8be62aca82fe2) )
	ROM_LOAD16_BYTE( "sparkzpg.1", 0x00001, 0x80000, CRC(1af1fc04) SHA1(6d92edb1a881ba6b63e0144c9c3e631b654bf8ae) )

	ROM_REGION( 0x20, "gfx1", ROMREGION_ERASEFF )
	/* empty */

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "sparkzsn",      0x00000, 0x80000, CRC(87097ce2) SHA1(dc4d199b5af692d111c087af3edc01e2ac0287a8) )
ROM_END



/*************************************
 *
 *  Driver initialization
 *
 *************************************/

static DRIVER_INIT( sparkz )
{
	memset(memory_region(machine, "gfx1"), 0, memory_region_length(machine, "gfx1"));
}



/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1992, arcadecl, 0, arcadecl, arcadecl, 0,      ROT0, "Atari Games", "Arcade Classics (prototype)", GAME_SUPPORTS_SAVE )
GAME( 1992, sparkz,   0, arcadecl, sparkz,   sparkz, ROT0, "Atari Games", "Sparkz (prototype)", GAME_SUPPORTS_SAVE )
