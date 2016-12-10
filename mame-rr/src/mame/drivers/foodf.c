/***************************************************************************

    Atari Food Fight hardware

    driver by Aaron Giles

    Games supported:
        * Food Fight

    Known bugs:
        * none at this time

****************************************************************************

    Memory map

****************************************************************************

    ========================================================================
    MAIN CPU
    ========================================================================
    000000-00FFFF   R     xxxxxxxx xxxxxxxx   Program ROM
    014000-01BFFF   R/W   xxxxxxxx xxxxxxxx   Program RAM
    01C000-01CFFF   R/W   xxxxxxxx xxxxxxxx   Motion object RAM (1024 entries x 2 words)
                    R/W   x------- --------      (0: Horizontal flip)
                    R/W   -x------ --------      (0: Vertical flip)
                    R/W   ---xxxxx --------      (0: Palette select)
                    R/W   -------- xxxxxxxx      (0: Tile index)
                    R/W   xxxxxxxx --------      (1: X position)
                    R/W   -------- xxxxxxxx      (1: Y position)
    800000-8007FF   R/W   xxxxxxxx xxxxxxxx   Playfield RAM (32x32 tiles)
                    R/W   x------- --------      (Tile index MSB)
                    R/W   --xxxxxx --------      (Palette select)
                    R/W   -------- xxxxxxxx      (Tile index LSBs)
    900000-9001FF   R/W   -------- ----xxxx   NVRAM
    940000-940007   R     -------- xxxxxxxx   Analog input read
    944000-944007     W   -------- --------   Analog input select
    948000          R     -------- xxxxxxxx   Digital inputs
                    R     -------- x-------      (Self test)
                    R     -------- -x------      (Player 2 throw)
                    R     -------- --x-----      (Player 1 throw)
                    R     -------- ---x----      (Aux coin)
                    R     -------- ----x---      (2 player start)
                    R     -------- -----x--      (1 player start)
                    R     -------- ------x-      (Right coin)
                    R     -------- -------x      (Left coin)
    948000            W   -------- xxxxxxxx   Digital outputs
                      W   -------- x-------      (Right coin counter)
                      W   -------- -x------      (Left coin counter)
                      W   -------- --x-----      (LED 2)
                      W   -------- ---x----      (LED 1)
                      W   -------- ----x---      (INT2 reset)
                      W   -------- -----x--      (INT1 reset)
                      W   -------- ------x-      (Update)
                      W   -------- -------x      (Playfield flip)
    94C000            W   -------- --------   Unknown
    950000-9501FF     W   -------- xxxxxxxx   Palette RAM (256 entries)
                      W   -------- xx------      (Blue)
                      W   -------- --xxx---      (Green)
                      W   -------- -----xxx      (Red)
    954000            W   -------- --------   NVRAM recall
    958000            W   -------- --------   Watchdog
    A40000-A4001F   R/W   -------- xxxxxxxx   POKEY 2
    A80000-A8001F   R/W   -------- xxxxxxxx   POKEY 1
    AC0000-AC001F   R/W   -------- xxxxxxxx   POKEY 3
    ========================================================================
    Interrupts:
        IRQ1 = 32V
        IRQ2 = VBLANK
    ========================================================================


***************************************************************************/


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/atarigen.h"
#include "sound/pokey.h"
#include "includes/foodf.h"


#define MASTER_CLOCK		12096000


/*************************************
 *
 *  NVRAM
 *
 *************************************/

static READ16_HANDLER( nvram_r )
{
	return space->machine->generic.nvram.u16[offset] | 0xfff0;
}



/*************************************
 *
 *  Interrupts
 *
 *************************************/

static void update_interrupts(running_machine *machine)
{
	foodf_state *state = (foodf_state *)machine->driver_data;
	cputag_set_input_line(machine, "maincpu", 1, state->atarigen.scanline_int_state ? ASSERT_LINE : CLEAR_LINE);
	cputag_set_input_line(machine, "maincpu", 2, state->atarigen.video_int_state ? ASSERT_LINE : CLEAR_LINE);
	cputag_set_input_line(machine, "maincpu", 3, state->atarigen.scanline_int_state && state->atarigen.video_int_state ? ASSERT_LINE : CLEAR_LINE);
}


static TIMER_DEVICE_CALLBACK( scanline_update )
{
	int scanline = param;

	/* WARNING: the timing of this is not perfectly accurate; it should fire on
       32V (i.e., on scanlines 32, 96, 160, and 224). However, due to the interrupt
       structure, it cannot fire at the same time as VBLANK. I have not solved this
       mystery yet */

	/* INT 1 is on 32V */
	atarigen_scanline_int_gen(timer.machine->device("maincpu"));

	/* advance to the next interrupt */
	scanline += 64;
	if (scanline > 192)
		scanline = 0;

	/* set a timer for it */
	timer.adjust(timer.machine->primary_screen->time_until_pos(scanline), scanline);
}


static MACHINE_START( foodf )
{
	foodf_state *state = (foodf_state *)machine->driver_data;
	atarigen_init(machine);
	state_save_register_global(machine, state->whichport);
}


static MACHINE_RESET( foodf )
{
	foodf_state *state = (foodf_state *)machine->driver_data;
	atarigen_interrupt_reset(&state->atarigen, update_interrupts);
	timer_device *scan_timer = machine->device<timer_device>("scan_timer");
	scan_timer->adjust(machine->primary_screen->time_until_pos(0));
}



/*************************************
 *
 *  Digital outputs
 *
 *************************************/

static WRITE16_HANDLER( digital_w )
{
	foodf_state *state = (foodf_state *)space->machine->driver_data;
	if (ACCESSING_BITS_0_7)
	{
		foodf_set_flip(state, data & 0x01);

		/* bit 1 = UPDATE */

		if (!(data & 0x04))
			atarigen_scanline_int_ack_w(space,0,0,0xffff);
		if (!(data & 0x08))
			atarigen_video_int_ack_w(space,0,0,0xffff);

		output_set_led_value(0, (data >> 4) & 1);
		output_set_led_value(1, (data >> 5) & 1);

		coin_counter_w(space->machine, 0, (data >> 6) & 1);
		coin_counter_w(space->machine, 1, (data >> 7) & 1);
	}
}



/*************************************
 *
 *  Analog inputs
 *
 *************************************/

static READ16_HANDLER( analog_r )
{
	static const char *const portnames[] = { "STICK0_X", "STICK1_X", "STICK0_Y", "STICK1_Y" };
	foodf_state *state = (foodf_state *)space->machine->driver_data;

	return input_port_read(space->machine, portnames[state->whichport]);
}


static WRITE16_HANDLER( analog_w )
{
	foodf_state *state = (foodf_state *)space->machine->driver_data;
	state->whichport = offset ^ 3;
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x00ffff) AM_MIRROR(0x3e0000) AM_ROM
	AM_RANGE(0x014000, 0x014fff) AM_MIRROR(0x3e3000) AM_RAM
	AM_RANGE(0x018000, 0x018fff) AM_MIRROR(0x3e3000) AM_RAM
	AM_RANGE(0x01c000, 0x01c0ff) AM_MIRROR(0x3e3f00) AM_RAM AM_BASE_GENERIC(spriteram)
	AM_RANGE(0x800000, 0x8007ff) AM_MIRROR(0x03f800) AM_RAM_WRITE(atarigen_playfield_w) AM_BASE_MEMBER(foodf_state, atarigen.playfield)
	AM_RANGE(0x900000, 0x9001ff) AM_MIRROR(0x03fe00) AM_RAM_READ(nvram_r) AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE(0x940000, 0x940007) AM_MIRROR(0x023ff8) AM_READ(analog_r)
	AM_RANGE(0x944000, 0x944007) AM_MIRROR(0x023ff8) AM_WRITE(analog_w)
	AM_RANGE(0x948000, 0x948001) AM_MIRROR(0x023ffe) AM_READ_PORT("SYSTEM") AM_WRITE(digital_w)
	AM_RANGE(0x950000, 0x9501ff) AM_MIRROR(0x023e00) AM_WRITE(foodf_paletteram_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x954000, 0x954001) AM_MIRROR(0x023ffe) AM_WRITENOP	/* RECALL */
	AM_RANGE(0x958000, 0x958001) AM_MIRROR(0x023ffe) AM_READWRITE(watchdog_reset16_r, watchdog_reset16_w)
	AM_RANGE(0xa40000, 0xa4001f) AM_MIRROR(0x03ffe0) AM_DEVREADWRITE8("pokey2", pokey_r, pokey_w, 0x00ff)
	AM_RANGE(0xa80000, 0xa8001f) AM_MIRROR(0x03ffe0) AM_DEVREADWRITE8("pokey1", pokey_r, pokey_w, 0x00ff)
	AM_RANGE(0xac0000, 0xac001f) AM_MIRROR(0x03ffe0) AM_DEVREADWRITE8("pokey3", pokey_r, pokey_w, 0x00ff)
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( foodf )
	PORT_START("STICK0_X")	/* IN0 */
	PORT_BIT( 0xff, 0x7f, IPT_AD_STICK_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("STICK1_X")	/* IN1 */
	PORT_BIT( 0xff, 0x7f, IPT_AD_STICK_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_REVERSE PORT_COCKTAIL PORT_PLAYER(2)

	PORT_START("STICK0_Y")	/* IN2 */
	PORT_BIT( 0xff, 0x7f, IPT_AD_STICK_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("STICK1_Y")	/* IN3 */
	PORT_BIT( 0xff, 0x7f, IPT_AD_STICK_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_REVERSE PORT_COCKTAIL PORT_PLAYER(2)

	PORT_START("SYSTEM")	/* IN4 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("DSW")	/* SW1 */
	PORT_DIPNAME( 0x07, 0x00, "Bonus Coins" )	PORT_DIPLOCATION("SW1:8,7,6")
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPSETTING(    0x05, "1 for every 2" )
	PORT_DIPSETTING(    0x02, "1 for every 4" )
	PORT_DIPSETTING(    0x01, "1 for every 5" )
	PORT_DIPSETTING(    0x06, "2 for every 4" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Coin_A ))	PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ))
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Coin_B ))	PORT_DIPLOCATION("SW1:4,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_5C ))
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_6C ))
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coinage ))	PORT_DIPLOCATION("SW1:2,1")
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x40, DEF_STR( Free_Play ))
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ STEP4(8*8,1), STEP4(0,1) },
	{ STEP8(0,8) },
	8*16
};


static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(1,2), 0 },
	{ STEP8(8*16,1), STEP8(0,1) },
	{ STEP16(0,8) },
	8*32
};


static GFXDECODE_START( foodf )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0, 64 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 0, 32 )
GFXDECODE_END



/*************************************
 *
 *  Sound definitions
 *
 *************************************/

static READ8_DEVICE_HANDLER( pot_r )
{
	return (input_port_read(device->machine, "DSW") >> offset) << 7;
}

static const pokey_interface pokey_config =
{
	{
		DEVCB_HANDLER(pot_r),
		DEVCB_HANDLER(pot_r),
		DEVCB_HANDLER(pot_r),
		DEVCB_HANDLER(pot_r),
		DEVCB_HANDLER(pot_r),
		DEVCB_HANDLER(pot_r),
		DEVCB_HANDLER(pot_r),
		DEVCB_HANDLER(pot_r)
	}
};



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_DRIVER_START( foodf )
	MDRV_DRIVER_DATA(foodf_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, MASTER_CLOCK/2)
	MDRV_CPU_PROGRAM_MAP(main_map)
	MDRV_CPU_VBLANK_INT("screen", atarigen_video_int_gen)

	MDRV_MACHINE_START(foodf)
	MDRV_MACHINE_RESET(foodf)
	MDRV_NVRAM_HANDLER(generic_1fill)
	MDRV_WATCHDOG_VBLANK_INIT(8)

	MDRV_TIMER_ADD("scan_timer", scanline_update)

	/* video hardware */
	MDRV_GFXDECODE(foodf)
	MDRV_PALETTE_LENGTH(256)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_RAW_PARAMS(MASTER_CLOCK/2, 384, 0, 256, 259, 0, 224)

	MDRV_VIDEO_START(foodf)
	MDRV_VIDEO_UPDATE(foodf)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("pokey1", POKEY, MASTER_CLOCK/2/10)
	MDRV_SOUND_CONFIG(pokey_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.33)

	MDRV_SOUND_ADD("pokey2", POKEY, MASTER_CLOCK/2/10)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.33)

	MDRV_SOUND_ADD("pokey3", POKEY, MASTER_CLOCK/2/10)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.33)
MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( foodf )
	ROM_REGION( 0x10000, "maincpu", 0 )	/* 64k for 68000 code */
	ROM_LOAD16_BYTE( "136020-301.8c",   0x000001, 0x002000, CRC(dfc3d5a8) SHA1(7abe5e9c27098bd8c93cc06f1b9e3db0744019e9) )
	ROM_LOAD16_BYTE( "136020-302.9c",   0x000000, 0x002000, CRC(ef92dc5c) SHA1(eb41291615165f549a68ebc6d4664edef1a04ac5) )
	ROM_LOAD16_BYTE( "136020-303.8d",   0x004001, 0x002000, CRC(64b93076) SHA1(efa4090d96aa0ffd4192a045f174ac5960810bca) )
	ROM_LOAD16_BYTE( "136020-304.9d",   0x004000, 0x002000, CRC(ea596480) SHA1(752aa33a8e8045650dd32ec7c7026e00d7896e0f) )
	ROM_LOAD16_BYTE( "136020-305.8e",   0x008001, 0x002000, CRC(e6cff1b1) SHA1(7c7ad2dcdff60fc092e8a825c5a6de6b506523de) )
	ROM_LOAD16_BYTE( "136020-306.9e",   0x008000, 0x002000, CRC(95159a3e) SHA1(f180126671776f62242ec9fd4a82a581c551ffce) )
	ROM_LOAD16_BYTE( "136020-307.8f",   0x00c001, 0x002000, CRC(17828dbb) SHA1(9d8e29a5e56a8a9c5db8561e4c20ff22f69b46ca) )
	ROM_LOAD16_BYTE( "136020-308.9f",   0x00c000, 0x002000, CRC(608690c9) SHA1(419020c69ce6fded0d9af44ead8ec4727468d58b) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "136020-109.6lm",  0x000000, 0x002000, CRC(c13c90eb) SHA1(ebd2bbbdd7e184851d1ab4b5648481d966c78cc2) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "136020-110.4d",   0x000000, 0x002000, CRC(8870e3d6) SHA1(702007d3d543f872b5bf5d00b49f6e05b46d6600) )
	ROM_LOAD( "136020-111.4e",   0x002000, 0x002000, CRC(84372edf) SHA1(9beef3ff3b28405c45d691adfbc233921073be47) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "136020-112.2p",   0x000000, 0x000100, CRC(0aa962d6) SHA1(efb51e4c95efb1b85206c416c1d6d35c6f4ff35c) )
ROM_END


ROM_START( foodf2 )
	ROM_REGION( 0x10000, "maincpu", 0 )	/* 64k for 68000 code */
	ROM_LOAD16_BYTE( "136020-201.8c",   0x000001, 0x002000, CRC(4ee52d73) SHA1(ff4ab8169a9b260bbd1f49023a30064e2f0b6686) )
	ROM_LOAD16_BYTE( "136020-202.9c",   0x000000, 0x002000, CRC(f8c4b977) SHA1(824d33baa413b2ee898c75157624ea007c92032f) )
	ROM_LOAD16_BYTE( "136020-203.8d",   0x004001, 0x002000, CRC(0e9f99a3) SHA1(37bba66957ee19e7d05fcc3e4583e909809075ed) )
	ROM_LOAD16_BYTE( "136020-204.9d",   0x004000, 0x002000, CRC(f667374c) SHA1(d7be70b56500e2071b7f8c810f7a3e2a6743c6bd) )
	ROM_LOAD16_BYTE( "136020-205.8e",   0x008001, 0x002000, CRC(1edd05b5) SHA1(cc712a11946c103eaa808c86e15676fde8610ad9) )
	ROM_LOAD16_BYTE( "136020-206.9e",   0x008000, 0x002000, CRC(bb8926b3) SHA1(95c6ba8ac6b56d1a67a47758b71712d55a959cd0) )
	ROM_LOAD16_BYTE( "136020-207.8f",   0x00c001, 0x002000, CRC(c7383902) SHA1(f76e2c95fccd0cafff9346a32e0c041c291a6696) )
	ROM_LOAD16_BYTE( "136020-208.9f",   0x00c000, 0x002000, CRC(608690c9) SHA1(419020c69ce6fded0d9af44ead8ec4727468d58b) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "136020-109.6lm",  0x000000, 0x002000, CRC(c13c90eb) SHA1(ebd2bbbdd7e184851d1ab4b5648481d966c78cc2) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "136020-110.4d",   0x000000, 0x002000, CRC(8870e3d6) SHA1(702007d3d543f872b5bf5d00b49f6e05b46d6600) )
	ROM_LOAD( "136020-111.4e",   0x002000, 0x002000, CRC(84372edf) SHA1(9beef3ff3b28405c45d691adfbc233921073be47) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "136020-112.2p",   0x000000, 0x000100, CRC(0aa962d6) SHA1(efb51e4c95efb1b85206c416c1d6d35c6f4ff35c) )
ROM_END


ROM_START( foodfc )
	ROM_REGION( 0x10000, "maincpu", 0 )	/* 64k for 68000 code */
	ROM_LOAD16_BYTE( "136020-113.8c",   0x000001, 0x002000, CRC(193a299f) SHA1(58bbf714eff22d8a47b174e4b121f14a8dcb4ef9) )
	ROM_LOAD16_BYTE( "136020-114.9c",   0x000000, 0x002000, CRC(33ed6bbe) SHA1(5d80fb092d2964b851e6c5982572d4ffc5078c55) )
	ROM_LOAD16_BYTE( "136020-115.8d",   0x004001, 0x002000, CRC(64b93076) SHA1(efa4090d96aa0ffd4192a045f174ac5960810bca) )
	ROM_LOAD16_BYTE( "136020-116.9d",   0x004000, 0x002000, CRC(ea596480) SHA1(752aa33a8e8045650dd32ec7c7026e00d7896e0f) )
	ROM_LOAD16_BYTE( "136020-117.8e",   0x008001, 0x002000, CRC(12a55db6) SHA1(508f02c72074a0e3300ec32c181e4f72cbc4245f) )
	ROM_LOAD16_BYTE( "136020-118.9e",   0x008000, 0x002000, CRC(e6d451d4) SHA1(03bfa932ed419572c08942ad159288b38d24d90f) )
	ROM_LOAD16_BYTE( "136020-119.8f",   0x00c001, 0x002000, CRC(455cc891) SHA1(9f7764c15dea7568326860b910686fec644c42c2) )
	ROM_LOAD16_BYTE( "136020-120.9f",   0x00c000, 0x002000, CRC(34173910) SHA1(19e6032c22d20410386516ffc1a809ae50431c65) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "136020-109.6lm",  0x000000, 0x002000, CRC(c13c90eb) SHA1(ebd2bbbdd7e184851d1ab4b5648481d966c78cc2) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "136020-110.4d",   0x000000, 0x002000, CRC(8870e3d6) SHA1(702007d3d543f872b5bf5d00b49f6e05b46d6600) )
	ROM_LOAD( "136020-111.4e",   0x002000, 0x002000, CRC(84372edf) SHA1(9beef3ff3b28405c45d691adfbc233921073be47) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "136020-112.2p",   0x000000, 0x000100, CRC(0aa962d6) SHA1(efb51e4c95efb1b85206c416c1d6d35c6f4ff35c) )
ROM_END



/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1982, foodf,  0,     foodf, foodf, 0, ROT0, "Atari", "Food Fight (rev 3)", GAME_SUPPORTS_SAVE )
GAME( 1982, foodf2, foodf, foodf, foodf, 0, ROT0, "Atari", "Food Fight (rev 2)", GAME_SUPPORTS_SAVE )
GAME( 1982, foodfc, foodf, foodf, foodf, 0, ROT0, "Atari", "Food Fight (cocktail)", GAME_SUPPORTS_SAVE )
