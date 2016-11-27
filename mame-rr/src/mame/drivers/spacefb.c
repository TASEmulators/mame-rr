/***************************************************************************

    Space Firebird hardware

    Memory Map figured out by Chris Hardy, Paul Johnson and Andy Clark
    MAME driver by Chris Hardy

    Schematics scanned and provided by James Twine
    Thanks to Gary Walton for lending me his REAL Space Firebird

    Known issues/to-do's:
        * Bullet colors are incorrect.  The schematics cannot be right, so
          I am using pure red for now
        * Analog sounds


    0000-3FFF ROM       Code
    8000-83FF RAM       Sprite RAM
    C000-C7FF RAM       Game ram

    IO Ports

    IN:
    Port 0

       bit 0 = Player 1 Right
       bit 1 = Player 1 Left
       bit 2 = unused
       bit 3 = unused
       bit 4 = Player 1 Escape
       bit 5 = unused
       bit 6 = unused
       bit 7 = Player 1 Fire

    Port 1

       bit 0 = Player 2 Right
       bit 1 = Player 2 Left
       bit 2 = unused
       bit 3 = unused
       bit 4 = Player 2 Escape
       bit 5 = unused
       bit 6 = unused
       bit 7 = Player 2 Fire

    Port 2

       bit 0 = unused
       bit 1 = unused
       bit 2 = Start 1 Player game
       bit 3 = Start 2 Players game
       bit 4 = unused
       bit 5 = unused
       bit 6 = Test switch
       bit 7 = Coin and Service switch

    Port 3

       bit 0 = Dipswitch 1
       bit 1 = Dipswitch 2
       bit 2 = Dipswitch 3
       bit 3 = Dipswitch 4
       bit 4 = Dipswitch 5
       bit 5 = Dipswitch 6
       bit 6 = unused (Debug switch - Code jumps to $3800 on reset if on)
       bit 7 = unused

   OUT:
   Port 0 - Video

       bit 0 = Screen flip. (RV)
       bit 1 = unused
       bit 2 = unused
       bit 3 = unused
       bit 4 = unused
       bit 5 = Char/Sprite Bank switch (VREF)
       bit 6 = Turns on Bit 2 of the color PROM. Used to change the bird colors. (CREF)
       bit 7 = unused

   Port 1
       bit 0 = discrete sound (Enemy death)
       bit 1 = INT to 8035
       bit 2 = T1 input to 8035
       bit 3 = PB4 input to 8035
       bit 4 = PB5 input to 8035
       bit 5 = T0 input to 8035
       bit 6 = discrete sound (Ship fire)
       bit 7 = discrete sound (Explosion noise)

   Port 2 - Video control

      These are passed to the sound board and are used to produce a
      red flash effect when you die.

      bit 0 = CONT R       Changes contrast of the red/green/blue part of the stars. This is used to make the starfield flicker
      bit 1 = CONT G
      bit 2 = CONT B
      bit 3 = ALRD         Turns background red on
      bit 4 = ALBU         Turns background blue on
      bit 5 = unused
      bit 6 = unused
      bit 7 = ALBA         Turns off star field (no star field)


****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "includes/spacefb.h"
#include "cpu/mcs48/mcs48.h"
#include "sound/dac.h"



/*************************************
 *
 *  Interrupt generation
 *
 *************************************/

static emu_timer *interrupt_timer;


static TIMER_CALLBACK( interrupt_callback )
{
	int next_vpos;

	/* compute vector and set the interrupt line */
	int vpos = machine->primary_screen->vpos();
	UINT8 vector = 0xc7 | ((vpos & 0x40) >> 2) | ((~vpos & 0x40) >> 3);
	cputag_set_input_line_and_vector(machine, "maincpu", 0, HOLD_LINE, vector);

	/* set up for next interrupt */
	if (vpos == SPACEFB_INT_TRIGGER_COUNT_1)
		next_vpos = SPACEFB_INT_TRIGGER_COUNT_2;
	else
		next_vpos = SPACEFB_INT_TRIGGER_COUNT_1;

	timer_adjust_oneshot(interrupt_timer, machine->primary_screen->time_until_pos(next_vpos), 0);
}


static void create_interrupt_timer(running_machine *machine)
{
	interrupt_timer = timer_alloc(machine, interrupt_callback, NULL);
}


static void start_interrupt_timer(running_machine *machine)
{
	timer_adjust_oneshot(interrupt_timer, machine->primary_screen->time_until_pos(SPACEFB_INT_TRIGGER_COUNT_1), 0);
}



/*************************************
 *
 *  Machine start
 *
 *************************************/

static MACHINE_START( spacefb )
{
	create_interrupt_timer(machine);
}



/*************************************
 *
 *  Machine reset
 *
 *************************************/

static MACHINE_RESET( spacefb )
{
	const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_IO);
	/* the 3 output ports are cleared on reset */
	spacefb_port_0_w(space, 0, 0);
	spacefb_port_1_w(space, 0, 0);
	spacefb_port_2_w(space, 0, 0);

	start_interrupt_timer(machine);
}



/*************************************
 *
 *  Memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( spacefb_main_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x7fff) AM_NOP
	AM_RANGE(0x8000, 0x83ff) AM_MIRROR(0x3c00) AM_RAM AM_BASE(&spacefb_videoram) AM_SIZE(&spacefb_videoram_size)
	AM_RANGE(0xc000, 0xc7ff) AM_MIRROR(0x3000) AM_RAM
	AM_RANGE(0xc800, 0xcfff) AM_MIRROR(0x3000) AM_NOP
ADDRESS_MAP_END


static ADDRESS_MAP_START( spacefb_audio_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0x3ff)
	AM_RANGE(0x0000, 0x03ff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  Port handlers
 *
 *************************************/

static ADDRESS_MAP_START( spacefb_main_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0x7)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("P1")
	AM_RANGE(0x01, 0x01) AM_READ_PORT("P2")
	AM_RANGE(0x02, 0x02) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x03, 0x03) AM_READ_PORT("DSW")
	AM_RANGE(0x04, 0x07) AM_READNOP  /* yes, this is correct (1-of-8 decoder) */

	AM_RANGE(0x00, 0x00) AM_MIRROR(0x04) AM_WRITE(spacefb_port_0_w)
	AM_RANGE(0x01, 0x01) AM_MIRROR(0x04) AM_WRITE(spacefb_port_1_w)
	AM_RANGE(0x02, 0x02) AM_MIRROR(0x04) AM_WRITE(spacefb_port_2_w)
	AM_RANGE(0x03, 0x03) AM_MIRROR(0x04) AM_WRITENOP
ADDRESS_MAP_END


static ADDRESS_MAP_START( spacefb_audio_io_map, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_DEVWRITE("dac", dac_w)
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_READ(spacefb_audio_p2_r)
	AM_RANGE(MCS48_PORT_T0, MCS48_PORT_T0) AM_READ(spacefb_audio_t0_r)
	AM_RANGE(MCS48_PORT_T1, MCS48_PORT_T1) AM_READ(spacefb_audio_t1_r)
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( spacefb )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE ) /* Test ? */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW:1,2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW:3,4")
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW:5")
	PORT_DIPSETTING(    0x00, "5000" )
	PORT_DIPSETTING(    0x10, "8000" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


/* same as Space Firebird, except for the difficulty switch */
static INPUT_PORTS_START( spacedem )
	PORT_INCLUDE( spacefb )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "5000" )
	PORT_DIPSETTING(    0x10, "8000" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_DRIVER_START( spacefb )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, SPACEFB_MAIN_CPU_CLOCK)
	MDRV_CPU_PROGRAM_MAP(spacefb_main_map)
	MDRV_CPU_IO_MAP(spacefb_main_io_map)

	MDRV_CPU_ADD("audiocpu", I8035, SPACEFB_AUDIO_CPU_CLOCK)
	MDRV_CPU_PROGRAM_MAP(spacefb_audio_map)
	MDRV_CPU_IO_MAP(spacefb_audio_io_map)

	MDRV_QUANTUM_TIME(HZ(180))

	MDRV_MACHINE_START(spacefb)
	MDRV_MACHINE_RESET(spacefb)

	/* video hardware */
	MDRV_VIDEO_START(spacefb)
	MDRV_VIDEO_UPDATE(spacefb)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_RAW_PARAMS(SPACEFB_PIXEL_CLOCK, SPACEFB_HTOTAL, SPACEFB_HBEND, SPACEFB_HBSTART, SPACEFB_VTOTAL, SPACEFB_VBEND, SPACEFB_VBSTART)

	/* audio hardware */
	MDRV_IMPORT_FROM(spacefb_audio)

MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( spacefb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "5e.cpu",      0x0000, 0x0800, CRC(2d406678) SHA1(9dff1980fc5267313f99f9f67d2d83eda8aae00e) )
	ROM_LOAD( "5f.cpu",      0x0800, 0x0800, CRC(89f0c34a) SHA1(4d8652fb7c4f22ddbac8c2d7ca7df675eaa2a447) )
	ROM_LOAD( "5h.cpu",      0x1000, 0x0800, CRC(c4bcac3e) SHA1(5364d6fc9d3402b2def163dee7c39fe3fe57eea3) )
	ROM_LOAD( "5i.cpu",      0x1800, 0x0800, CRC(61c00a65) SHA1(afc93e320478c70b3ddca8375fd648c9f2572dab) )
	ROM_LOAD( "5j.cpu",      0x2000, 0x0800, CRC(598420b9) SHA1(92ea695177c7297699d1d18f166e98392ef0e0f9) )
	ROM_LOAD( "5k.cpu",      0x2800, 0x0800, CRC(1713300c) SHA1(9a7b6cc0d79cccadd4988e0e791c1598813b6552) )
	ROM_LOAD( "5m.cpu",      0x3000, 0x0800, CRC(6286f534) SHA1(c47d0df85a52c774a4bc26351fdae18795062b6e) )
	ROM_LOAD( "5n.cpu",      0x3800, 0x0800, CRC(1c9f91ee) SHA1(481a309fe9aa9ce6fd18d7d908c18790f594057d) )

	ROM_REGION( 0x1000, "audiocpu", 0 )
    ROM_LOAD( "ic20.snd",    0x0000, 0x0400, CRC(1c8670b3) SHA1(609124caa11498fc6a6bdf6cdbb8003bbc249dd8) )

	ROM_REGION( 0x1000, "gfx1", 0 )  /* sprites */
	ROM_LOAD( "5k.vid",      0x0000, 0x0800, CRC(236e1ff7) SHA1(575b8ed9ab054a864207e0fde3ae93cdcafbebf2) )
	ROM_LOAD( "6k.vid",      0x0800, 0x0800, CRC(bf901a4e) SHA1(71207ad1ca60aa617dbbc3cd2e4e42520b7c8513) )

	ROM_REGION( 0x0100, "gfx2", 0 )  /* bullets */
	ROM_LOAD( "4i.vid",      0x0000, 0x0100, CRC(528e8533) SHA1(8e41eee1016c98a4f08acbd902daf8e32aa9d9ab) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "mb7051.3n",   0x0000, 0x0020, CRC(465d07af) SHA1(25e246f7674c25d05e5f6e68db88c15aaa10cee1) )
ROM_END

ROM_START( spacefbg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tst-c.5e",    0x0000, 0x0800, CRC(07949110) SHA1(b090e629203c54fc0937d82b0cfe355153a65d6b) )
	ROM_LOAD( "tst-c.5f",    0x0800, 0x0800, CRC(ce591929) SHA1(c9cf7b8a77c108e004e8863b6a08392204e9d434) )
	ROM_LOAD( "tst-c.5h",    0x1000, 0x0800, CRC(55d34ea5) SHA1(d98125e4a33c00285a14cb6cc9880d215b4c29d2) )
	ROM_LOAD( "tst-c.5i",    0x1800, 0x0800, CRC(a11e2881) SHA1(c084a0975b88a981f23a52baa6b8c239dae00e5c) )
	ROM_LOAD( "tst-c.5j",    0x2000, 0x0800, CRC(a6aff352) SHA1(a7fd6b5fe5c76aad726d599142b4cca88109fa10) )
	ROM_LOAD( "tst-c.5k",    0x2800, 0x0800, CRC(f4213603) SHA1(cf39027f2a77cab02d1117025a8eccb868f6a1b0) )
	ROM_LOAD( "5m.cpu",      0x3000, 0x0800, CRC(6286f534) SHA1(c47d0df85a52c774a4bc26351fdae18795062b6e) )
	ROM_LOAD( "5n.cpu",      0x3800, 0x0800, CRC(1c9f91ee) SHA1(481a309fe9aa9ce6fd18d7d908c18790f594057d) )

	ROM_REGION( 0x1000, "audiocpu", 0 )
    ROM_LOAD( "ic20.snd",    0x0000, 0x0400, CRC(1c8670b3) SHA1(609124caa11498fc6a6bdf6cdbb8003bbc249dd8) )

	ROM_REGION( 0x1000, "gfx1", 0 )  /* sprites */
	ROM_LOAD( "tst-v.5k",    0x0000, 0x0800, CRC(bacc780d) SHA1(fe498b477bbf7f03fd256de2f799483383a7e819) )
	ROM_LOAD( "tst-v.6k",    0x0800, 0x0800, CRC(1645ff26) SHA1(34cfa0e6221bf53b1bda8609eb14fbcc5fb5bdcd) )

	ROM_REGION( 0x0100, "gfx2", 0 )  /* bullets */
	ROM_LOAD( "4i.vid",      0x0000, 0x0100, CRC(528e8533) SHA1(8e41eee1016c98a4f08acbd902daf8e32aa9d9ab) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "mb7051.3n",   0x0000, 0x0020, CRC(465d07af) SHA1(25e246f7674c25d05e5f6e68db88c15aaa10cee1) )
ROM_END

ROM_START( spacebrd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sb5e.cpu",    0x0000, 0x0800, CRC(232d66b8) SHA1(d443651819007828a40cea05b6936b762375c48f) )
	ROM_LOAD( "sb5f.cpu",    0x0800, 0x0800, CRC(99504327) SHA1(043182097680b5d6164157055a1a5b95759ca64d) )
	ROM_LOAD( "sb5h.cpu",    0x1000, 0x0800, CRC(49a26fe5) SHA1(851f62df651aa180b6fa236f4c54ed7791d92a21) )
	ROM_LOAD( "sb5i.cpu",    0x1800, 0x0800, CRC(c23025da) SHA1(ccc73ca9754b04e49733661cbd9e788b13163100) )
	ROM_LOAD( "sb5j.cpu",    0x2000, 0x0800, CRC(5e97baf0) SHA1(5e1985b8e3354a0c3454a5e43f80e69f1e1f77c0) )
	ROM_LOAD( "5k.cpu",      0x2800, 0x0800, CRC(1713300c) SHA1(9a7b6cc0d79cccadd4988e0e791c1598813b6552) )
	ROM_LOAD( "sb5m.cpu",    0x3000, 0x0800, CRC(4cbe92fc) SHA1(903b617e42f740e94a6edb6a973dc0d57ac0abee) )
	ROM_LOAD( "sb5n.cpu",    0x3800, 0x0800, CRC(1a798fbf) SHA1(65ff2fe91c2037378314c4a68b2bd21fd167c64a) )

	ROM_REGION( 0x1000, "audiocpu", 0 )
    ROM_LOAD( "ic20.snd",    0x0000, 0x0400, CRC(1c8670b3) SHA1(609124caa11498fc6a6bdf6cdbb8003bbc249dd8) )

	ROM_REGION( 0x1000, "gfx1", 0 )  /* sprites */
	ROM_LOAD( "5k.vid",      0x0000, 0x0800, CRC(236e1ff7) SHA1(575b8ed9ab054a864207e0fde3ae93cdcafbebf2) )
	ROM_LOAD( "6k.vid",      0x0800, 0x0800, CRC(bf901a4e) SHA1(71207ad1ca60aa617dbbc3cd2e4e42520b7c8513) )

	ROM_REGION( 0x0100, "gfx2", 0 )  /* bullets */
	ROM_LOAD( "4i.vid",      0x0000, 0x0100, CRC(528e8533) SHA1(8e41eee1016c98a4f08acbd902daf8e32aa9d9ab) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "spcbird.clr", 0x0000, 0x0020, CRC(25c79518) SHA1(e8f7e8b3d0cf1ed9d723948548f58abf0e2c6d1f) )
ROM_END

/* only a few bytes are different between this and spacebrd above */
ROM_START( spacefbb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fc51",        0x0000, 0x0800, CRC(5657bd2f) SHA1(0e615a7dd5efbbf6f543480bc150f45089c41d32) )
	ROM_LOAD( "fc52",        0x0800, 0x0800, CRC(303b0294) SHA1(a2f5637e201739b440e7ea0868d2d5745fbb4f5b) )
	ROM_LOAD( "sb5h.cpu",    0x1000, 0x0800, CRC(49a26fe5) SHA1(851f62df651aa180b6fa236f4c54ed7791d92a21) )
	ROM_LOAD( "sb5i.cpu",    0x1800, 0x0800, CRC(c23025da) SHA1(ccc73ca9754b04e49733661cbd9e788b13163100) )
	ROM_LOAD( "fc55",        0x2000, 0x0800, CRC(946bee5d) SHA1(6e668cec5986af3d319bf9aa8962a3d9008d0156) )
	ROM_LOAD( "5k.cpu",      0x2800, 0x0800, CRC(1713300c) SHA1(9a7b6cc0d79cccadd4988e0e791c1598813b6552) )
	ROM_LOAD( "sb5m.cpu",    0x3000, 0x0800, CRC(4cbe92fc) SHA1(903b617e42f740e94a6edb6a973dc0d57ac0abee) )
	ROM_LOAD( "sb5n.cpu",    0x3800, 0x0800, CRC(1a798fbf) SHA1(65ff2fe91c2037378314c4a68b2bd21fd167c64a) )

	ROM_REGION( 0x1000, "audiocpu", 0 )
    ROM_LOAD( "fb.snd",      0x0000, 0x0400, CRC(f7a59492) SHA1(22bdc02c72086c38acd9d9675da54ce6ba3f80a3) )

	ROM_REGION( 0x1000, "gfx1", 0 )  /* sprites */
	ROM_LOAD( "fc59",        0x0000, 0x0800, CRC(a00ad16c) SHA1(6130b2250b492b56e3ea94e44f7b2ddf45908d00) )
	ROM_LOAD( "6k.vid",      0x0800, 0x0800, CRC(bf901a4e) SHA1(71207ad1ca60aa617dbbc3cd2e4e42520b7c8513) )

	ROM_REGION( 0x0100, "gfx2", 0 )  /* bullets */
	ROM_LOAD( "4i.vid",      0x0000, 0x0100, CRC(528e8533) SHA1(8e41eee1016c98a4f08acbd902daf8e32aa9d9ab) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "mb7051.3n",   0x0000, 0x0020, CRC(465d07af) SHA1(25e246f7674c25d05e5f6e68db88c15aaa10cee1) )
ROM_END

ROM_START( spacedem )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sdm-c-5e",    0x0000, 0x0800, CRC(be4b9cbb) SHA1(345ea1e56754e0c8300148b53346dbec50b3608e) )
	ROM_LOAD( "sdm-c-5f",    0x0800, 0x0800, CRC(0814f964) SHA1(0186d11ca98f4b2e4c2572db9d440456370275e7) )
	ROM_LOAD( "sdm-c-5h",    0x1000, 0x0800, CRC(ebfff682) SHA1(e060627de302a9ce125d939d9890739d2154a507) )
	ROM_LOAD( "sdm-c-5i",    0x1800, 0x0800, CRC(dd7e1378) SHA1(94a756036e7d03c42ee896b794cb1f8753a67b91) )
	ROM_LOAD( "sdm-c-5j",    0x2000, 0x0800, CRC(98334fda) SHA1(9990bbfb2aa4d953e531bb49eab1c3a999b78b9c) )
	ROM_LOAD( "sdm-c-5k",    0x2800, 0x0800, CRC(ba4933b2) SHA1(9e5003849185ea35b5929c9a8ae188a87bb522cc) )
	ROM_LOAD( "sdm-c-5m",    0x3000, 0x0800, CRC(14d3c656) SHA1(55522df8c2e484b8d5d4a32bf7cfb2b30dcdab4a) )
	ROM_LOAD( "sdm-c-5n",    0x3800, 0x0800, CRC(7e0e41b0) SHA1(e7dd509ab36e0f9be6350b5fa9de4694224477db) )

	ROM_REGION( 0x1000, "audiocpu", 0 )
    ROM_LOAD( "sdm-e-20",    0x0000, 0x0400, CRC(55f40a0b) SHA1(8dff27b636f7f1831f71816505e451cf97fc3f98) )

	ROM_REGION( 0x1000, "gfx1", 0 )  /* sprites */
	ROM_LOAD( "sdm-v-5k",    0x0000, 0x0800, CRC(55758e4d) SHA1(1338b45f76f5a31a5350c953eac36cc543fbe62e) )
	ROM_LOAD( "sdm-v-6k",    0x0800, 0x0800, CRC(3fcbb20c) SHA1(674de509f7b6c5d7c41112881b0c3093b9b176a0) )

	ROM_REGION( 0x0100, "gfx2", 0 )  /* bullets */
	ROM_LOAD( "4i.vid",      0x0000, 0x0100, CRC(528e8533) SHA1(8e41eee1016c98a4f08acbd902daf8e32aa9d9ab) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "sdm-v-3n",    0x0000, 0x0020, CRC(6d8ad169) SHA1(6ccc931774183e14e28bb9b93223d366fd596f30) )
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1980, spacefb,  0,       spacefb, spacefb,  0, ROT270, "Nintendo", "Space Firebird (Nintendo)", GAME_IMPERFECT_COLORS | GAME_IMPERFECT_SOUND )
GAME( 1980, spacefbg, spacefb, spacefb, spacefb,  0, ROT270, "Nintendo (Gremlin license)", "Space Firebird (Gremlin)", GAME_IMPERFECT_COLORS | GAME_IMPERFECT_SOUND )
GAME( 1980, spacebrd, spacefb, spacefb, spacefb,  0, ROT270, "bootleg (Karateco)", "Space Bird (bootleg)", GAME_IMPERFECT_COLORS | GAME_IMPERFECT_SOUND )
GAME( 1980, spacefbb, spacefb, spacefb, spacefb,  0, ROT270, "bootleg", "Space Firebird (bootleg)", GAME_IMPERFECT_COLORS | GAME_IMPERFECT_SOUND )
GAME( 1980, spacedem, spacefb, spacefb, spacedem, 0, ROT270, "Nintendo (Fortrek license)", "Space Demon", GAME_IMPERFECT_COLORS | GAME_IMPERFECT_SOUND )
