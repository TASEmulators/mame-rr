/***************************************************************************

    Star Fire/Fire One system

    driver by Daniel Boris, Olivier Galibert, Aaron Giles

****************************************************************************

    Memory map

****************************************************************************

    ========================================================================
    MAIN CPU
    ========================================================================
    0000-7FFF   R     xxxxxxxx   Program ROM
    8000-9FFF   R/W   xxxxxxxx   Scratch RAM, actually mapped into low VRAM
    9000          W   xxxxxxxx   VRAM write control register
                  W   xxx-----      (VRAM shift amount 1)
                  W   ---x----      (VRAM write mirror 1)
                  W   ----xxx-      (VRAM shift amount 2)
                  W   -------x      (VRAM write mirror 2)
    9001          W   xxxxxxxx   Video control register
                  W   x-------      (Color RAM source select)
                  W   -x------      (Palette RAM write enable)
                  W   --x-----      (Video RAM write enable)
                  W   ---x----      (Right side mask select)
                  W   ----xxxx      (Video RAM ALU operation)
    9800-9807   R     xxxxxxxx   Input ports
    A000-BFFF   R/W   xxxxxxxx   Color RAM
    C000-DFFF   R/W   xxxxxxxx   Video RAM, using shift/mirror 1 and color
    E000-FFFF   R/W   xxxxxxxx   Video RAM, using shift/mirror 2
    ========================================================================
    Interrupts:
       NMI generated once/frame
    ========================================================================

***************************************************************************

Notes:

starfira has one less rom in total than starfire but everything passes as
 ok in the rom test so its probably just an earlier revision or something

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "includes/starfire.h"



static UINT8 fireone_select;
static read8_space_func input_read;



/*************************************
 *
 *  Scratch RAM, mapped into video RAM
 *
 *************************************/

static WRITE8_HANDLER( starfire_scratch_w )
{
	/* A12 and A3 select video control registers */
	if ((offset & 0x1008) == 0x1000)
	{
		switch (offset & 7)
		{
			case 0:	starfire_vidctrl_w(space, 0, data); break;
			case 1: starfire_vidctrl1_w(space, 0, data); break;
			case 2:
				/* Sounds */
				fireone_select = (data & 0x8) ? 0 : 1;
				break;
		}
	}

	/* convert to a videoram offset */
	offset = (offset & 0x31f) | ((offset & 0xe0) << 5);
    starfire_videoram[offset] = data;
}


static READ8_HANDLER( starfire_scratch_r )
{
	/* A11 selects input ports */
	if (offset & 0x800)
		return (*input_read)(space, offset);

	/* convert to a videoram offset */
	offset = (offset & 0x31f) | ((offset & 0xe0) << 5);
    return starfire_videoram[offset];
}



/*************************************
 *
 *  Game-specific input handlers
 *
 *************************************/

static READ8_HANDLER( starfire_input_r )
{
	switch (offset & 15)
	{
		case 0:	return input_port_read(space->machine, "DSW");
		case 1:	return input_port_read(space->machine, "SYSTEM");	/* Note: need to loopback sounds lengths on that one */
		case 5: return input_port_read(space->machine, "STICKZ");
		case 6:	return input_port_read(space->machine, "STICKX");
		case 7:	return input_port_read(space->machine, "STICKY");
		default: return 0xff;
	}
}


static READ8_HANDLER( fireone_input_r )
{
	static const UINT8 fireone_paddle_map[64] =
	{
		0x00,0x01,0x03,0x02,0x06,0x07,0x05,0x04,
		0x0c,0x0d,0x0f,0x0e,0x0a,0x0b,0x09,0x08,
		0x18,0x19,0x1b,0x1a,0x1e,0x1f,0x1d,0x1c,
		0x14,0x15,0x17,0x16,0x12,0x13,0x11,0x10,
		0x30,0x31,0x33,0x32,0x36,0x37,0x35,0x34,
		0x3c,0x3d,0x3f,0x3e,0x3a,0x3b,0x39,0x38,
		0x28,0x29,0x2b,0x2a,0x2e,0x2f,0x2d,0x2c,
		0x24,0x25,0x27,0x26,0x22,0x23,0x21,0x20
	};
	int temp;

	switch (offset & 15)
	{
		case 0:	return input_port_read(space->machine, "DSW");
		case 1:	return input_port_read(space->machine, "SYSTEM");
		case 2:
			temp = fireone_select ? input_port_read(space->machine, "P1") : input_port_read(space->machine, "P2");
			temp = (temp & 0xc0) | fireone_paddle_map[temp & 0x3f];
			return temp;
		default: return 0xff;
	}
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x9fff) AM_READWRITE(starfire_scratch_r, starfire_scratch_w)
	AM_RANGE(0xa000, 0xbfff) AM_RAM_WRITE(starfire_colorram_w) AM_BASE(&starfire_colorram)
	AM_RANGE(0xc000, 0xffff) AM_READWRITE(starfire_videoram_r, starfire_videoram_w) AM_BASE(&starfire_videoram)
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( starfire )
	PORT_START("DSW")	/* DSW0 */
	PORT_DIPNAME( 0x03, 0x00, "Time" )
	PORT_DIPSETTING(    0x00, "90 Sec" )
	PORT_DIPSETTING(    0x01, "80 Sec" )
	PORT_DIPSETTING(    0x02, "70 Sec" )
	PORT_DIPSETTING(    0x03, "60 Sec" )
	PORT_DIPNAME( 0x04, 0x00, "Coin(s) to Start" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPNAME( 0x08, 0x00, "Fuel per Coin" )
	PORT_DIPSETTING(    0x00, "300" )
	PORT_DIPSETTING(    0x08, "600" )
	PORT_DIPNAME( 0x30, 0x00, "Bonus" )
	PORT_DIPSETTING(    0x00, "300 points" )
	PORT_DIPSETTING(    0x10, "500 points" )
	PORT_DIPSETTING(    0x20, "700 points" )
	PORT_DIPSETTING(    0x30, DEF_STR( None ) )
	PORT_DIPNAME( 0x40, 0x00, "Score Table Hold" )
	PORT_DIPSETTING(    0x00, "fixed length" )
	PORT_DIPSETTING(    0x40, "fixed length+fire" )
	PORT_SERVICE( 0x80, IP_ACTIVE_HIGH )

	PORT_START("SYSTEM")	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("STICKX")	/* IN2 */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10)

	PORT_START("STICKY")	/* IN3 */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_REVERSE

	PORT_START("STICKZ")	/* IN4 */ /* throttle */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Z ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_REVERSE
INPUT_PORTS_END


static INPUT_PORTS_START( fireone )
	PORT_START("DSW")	/* DSW0 */
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, "2 Coins/1 Player" )
	PORT_DIPSETTING(    0x02, "2 Coins/1 or 2 Players" )
	PORT_DIPSETTING(    0x00, "1 Coin/1 Player" )
	PORT_DIPSETTING(    0x01, "1 Coin/1 or 2 Players" )
	PORT_DIPNAME( 0x0c, 0x0c, "Time" )
	PORT_DIPSETTING(    0x00, "75 Sec" )
	PORT_DIPSETTING(    0x04, "90 Sec" )
	PORT_DIPSETTING(    0x08, "105 Sec" )
	PORT_DIPSETTING(    0x0c, "120 Sec" )
	PORT_DIPNAME( 0x30, 0x00, "Bonus difficulty" )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_HIGH )

	PORT_START("SYSTEM")	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P1")	/* IN2 */
	PORT_BIT( 0x3f, 0x20, IPT_PADDLE ) PORT_MINMAX(0,63) PORT_SENSITIVITY(50) PORT_KEYDELTA(1) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)

	PORT_START("P2")	/* IN3 */
	PORT_BIT( 0x3f, 0x20, IPT_PADDLE ) PORT_MINMAX(0,63) PORT_SENSITIVITY(50) PORT_KEYDELTA(1) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
INPUT_PORTS_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_DRIVER_START( starfire )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, STARFIRE_CPU_CLOCK)
	MDRV_CPU_PROGRAM_MAP(main_map)
	MDRV_CPU_VBLANK_INT("screen", nmi_line_pulse)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_RAW_PARAMS(STARFIRE_PIXEL_CLOCK, STARFIRE_HTOTAL, STARFIRE_HBEND, STARFIRE_HBSTART, STARFIRE_VTOTAL, STARFIRE_VBEND, STARFIRE_VBSTART)

	MDRV_VIDEO_START(starfire)
	MDRV_VIDEO_UPDATE(starfire)

	/* audio hardware */
MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( starfire )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sfire.1a",     0x0000, 0x0800, CRC(9990af64) SHA1(05eccf1084ace55be9d6cf0fccddcaa18fa5487a) )
	ROM_LOAD( "sfire.2a",     0x0800, 0x0800, CRC(6e17ba33) SHA1(59433696f56018a7b253491b1db3ff45546dcd46) )
	ROM_LOAD( "sfire.1b",     0x1000, 0x0800, CRC(946175d0) SHA1(6a55d9f6031b96e9e05d61d59a23d4fc6df724bf) )
	ROM_LOAD( "sfire.2b",     0x1800, 0x0800, CRC(67be4275) SHA1(dd6232e034030e0c2b4d866fda36cbe22d8518f7) )
	ROM_LOAD( "sfire.1c",     0x2000, 0x0800, CRC(c56b4e07) SHA1(e55ae84c484a78372180783df37750cdad8b04a2) )
	ROM_LOAD( "sfire.2c",     0x2800, 0x0800, CRC(b4b9d3a7) SHA1(8f3e0d67d1e94f6b1c41a78e59ac81f021aa827a) )
	ROM_LOAD( "sfire.1d",     0x3000, 0x0800, CRC(fd52ffb5) SHA1(c1ba2ffb7de0301a962cca2e693bfbbd9838b852) )
	ROM_LOAD( "sfire.2d",     0x3800, 0x0800, CRC(51c69fe3) SHA1(33159cb3ea5029d395fc20916899aa05139c2d51) )
	ROM_LOAD( "sfire.1e",     0x4000, 0x0800, CRC(01994ec8) SHA1(db694f922a98bb0fc585cad83bee8a88d72fca8f) )
	ROM_LOAD( "sfire.2e",     0x4800, 0x0800, CRC(ef3d1b71) SHA1(ca427209194f519b1ac5b94d29c2789445303dc1) )
	ROM_LOAD( "sfire.1f",     0x5000, 0x0800, CRC(af31dc39) SHA1(0dfeff6973fd03e85b08e70c77d212f0bb60121d) )

	ROM_REGION( 0x0040, "proms", 0 ) /* DRAM addressing */
	ROM_LOAD( "prom-1.7a",    0x0000, 0x0020, CRC(ae1f4acd) SHA1(1d502b61db73cf6a4ac3d235455a5c464f12652a) ) /* BPROM type is N82S123 */
	ROM_LOAD( "prom-2.8a",    0x0020, 0x0020, CRC(9b713924) SHA1(943ad55d232f7bb99886a9a273dd14a1e1533491) ) /* BPROM type is N82S123 */
ROM_END

ROM_START( starfirea )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "starfire.1a",  0x0000, 0x0800, CRC(6adcd7e7) SHA1(a931fb80e48db3050ce3bc39f455961c0c7c56ce) )
	ROM_LOAD( "starfire.2a",  0x0800, 0x0800, CRC(835c70ea) SHA1(36828735aa48de5e3e973ca1f42ef08537e1c6ce) )
	ROM_LOAD( "starfire.1b",  0x1000, 0x0800, CRC(377afbef) SHA1(97cb5a20aeb8c70670d6db8f41b2abcb181755c6) )
	ROM_LOAD( "starfire.2b",  0x1800, 0x0800, CRC(f3a833cb) SHA1(d2e01806ead71b0946347fd9668fd3f24524734e) )
	ROM_LOAD( "starfire.1c",  0x2000, 0x0800, CRC(db625c1d) SHA1(5d0307258a73b4b82fbe7b10634076412f4ab3c7) )
	ROM_LOAD( "starfire.2c",  0x2800, 0x0800, CRC(68fa2ce6) SHA1(2b32df960bc4ec38f50f0d23ab96becb68bc4034) )
	ROM_LOAD( "starfire.1d",  0x3000, 0x0800, CRC(c6b5f1d1) SHA1(85a3f7ce7a51597609c762c9a809b84922f8a6e5) )
	ROM_LOAD( "starfire.2d",  0x3800, 0x0800, CRC(ab2a36a5) SHA1(debd9503246b4d27c8136bfb60cdffd9107ad95e) )
	ROM_LOAD( "starfire.1e",  0x4000, 0x0800, CRC(1ac8ba8c) SHA1(90c5a8a943edad74141b15e1f145598abce8cb75) )
	ROM_LOAD( "starfire.2e",  0x4800, 0x0800, CRC(ba8434c5) SHA1(1831b291dfe3e4b081e66caa909b8c727bfffa7b) )

	ROM_REGION( 0x0040, "proms", 0 ) /* DRAM addressing */
	ROM_LOAD( "prom-1.7a",    0x0000, 0x0020, CRC(ae1f4acd) SHA1(1d502b61db73cf6a4ac3d235455a5c464f12652a) ) /* BPROM type is N82S123 */
	ROM_LOAD( "prom-2.8a",    0x0020, 0x0020, CRC(9b713924) SHA1(943ad55d232f7bb99886a9a273dd14a1e1533491) ) /* BPROM type is N82S123 */
ROM_END

ROM_START( fireone )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fo-ic13.7b",   0x0000, 0x0800, CRC(f927f086) SHA1(509db84d781dd2d5aaefd561539738f0db7c4ca5) )
	ROM_LOAD( "fo-ic24.7c",   0x0800, 0x0800, CRC(0d2d8723) SHA1(e9bb2092ce7786016f15e42916ad48ef12735e9c) )
	ROM_LOAD( "fo-ic12.6b",   0x1000, 0x0800, CRC(ac7783d9) SHA1(8bcfcc5d3126382f4ec8904e0435de0931abc41e) )
	ROM_LOAD( "fo-ic23.6c",   0x1800, 0x0800, CRC(15c74ee7) SHA1(0adb87c2471ecbbd18d10579043765ce877dbde7) )
	ROM_LOAD( "fo-ic11.5b",   0x2000, 0x0800, CRC(721930a1) SHA1(826245ffbd399056a74ccd14cd2bd4acd2fb2d24) )
	ROM_LOAD( "fo-ic22.5c",   0x2800, 0x0800, CRC(f0c965b4) SHA1(ffe96e636720325d9a40b729128730446b74435b) )
	ROM_LOAD( "fo-ic10.4b",   0x3000, 0x0800, CRC(27a7b2c0) SHA1(7a8c70e565bdcb6e085e4d283f41c92758640055) )
	ROM_LOAD( "fo-ic21.4c",   0x3800, 0x0800, CRC(b142c857) SHA1(609fbd0c0b5833807fd606284c26ad7cb7e4d742) )
	ROM_LOAD( "fo-ic09.3b",   0x4000, 0x0800, CRC(1c076b1b) SHA1(874c09c81e90e1be869902057b7359e71f77db52) )
	ROM_LOAD( "fo-ic20.3c",   0x4800, 0x0800, CRC(b4ac6e71) SHA1(4731dd6865929b8c9c33cbe4cf1dde23046d6914) )
	ROM_LOAD( "fo-ic08.2b",   0x5000, 0x0800, CRC(5839e2ff) SHA1(9d8a17c5b64cdf5bf222f4dbca48f0210b18e403) )
	ROM_LOAD( "fo-ic19.2c",   0x5800, 0x0800, CRC(9fd85e11) SHA1(f8264357a63f757bc58f3703e60e219d67d0d081) )
	ROM_LOAD( "fo-ic07.1b",   0x6000, 0x0800, CRC(b90baae1) SHA1(c7dedf38e5a1977234f1f745a7aa443f6bf7db52) )
	ROM_LOAD( "fo-ic18.1c",   0x6800, 0x0800, CRC(771ee5ba) SHA1(6577e219386de594dbde8a54d5f5f9657419061a) )

	ROM_REGION( 0x0040, "proms", 0 ) /* DRAM addressing */
	ROM_LOAD( "prom-1.7a",    0x0000, 0x0020, CRC(ae1f4acd) SHA1(1d502b61db73cf6a4ac3d235455a5c464f12652a) ) /* BPROM type is N82S123 */
	ROM_LOAD( "prom-2.8a",    0x0020, 0x0020, CRC(9b713924) SHA1(943ad55d232f7bb99886a9a273dd14a1e1533491) ) /* BPROM type is N82S123 */
ROM_END

ROM_START( starfir2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sfire2.01",    0x0000, 0x0800, CRC(f75be2f4) SHA1(b15511c345363f45eee0c019aa336a9aa16e63ea) )
	ROM_LOAD( "sfire2.02",    0x0800, 0x0800, CRC(ccf98c6a) SHA1(3e7792aa47750ee19baf1e74016038fe80c92381) )
	ROM_LOAD( "sfire2.03",    0x1000, 0x0800, CRC(604b2d50) SHA1(39d402135aaaa44c1ad05e1665eb6668280fae28) )
	ROM_LOAD( "sfire2.04",    0x1800, 0x0800, CRC(f8a9658f) SHA1(aea97387001183a797375971c7325b4a838ea1d5) )
	ROM_LOAD( "sfire2.05",    0x2000, 0x0800, CRC(acbaf827) SHA1(a546340f8533557a86b589f5011e5af0439e0d4d) )
	ROM_LOAD( "sfire2.06",    0x2800, 0x0800, CRC(3525bb22) SHA1(1a1ca8b5ef1a5584d28644bdc751635aac3fad02) )
	ROM_LOAD( "sfire2.07",    0x3000, 0x0800, CRC(7fce0e54) SHA1(17355fe98cf1511c32e90434960ced7b3f3ecac7) )
	ROM_LOAD( "sfire2.08",    0x3800, 0x0800, CRC(98054c14) SHA1(4a561a9d87be9c5d4283ee78c4cf05c10c979a2f) )
	ROM_LOAD( "sfire2.09",    0x4000, 0x0800, CRC(abaa4144) SHA1(045ebcd38d6a3f75c6d819a42aa1fb92ac84755c) )
	ROM_LOAD( "sfire2.10",    0x4800, 0x0800, CRC(a0b3dadb) SHA1(d86683b528b5fbafad0cdd054940bc04b056b850) )
	ROM_LOAD( "sfire2.11",    0x5000, 0x0800, CRC(a61ebbd2) SHA1(9fdf6558306aebbf5e9e106e4f4f6f7a3e703696) )
	ROM_LOAD( "sfire2.12",    0x5800, 0x0800, CRC(a35ba06d) SHA1(122f1dbc235977367fdd06b7517c356a3147dfd1) )

	ROM_REGION( 0x0040, "proms", 0 ) /* DRAM addressing */
	ROM_LOAD( "prom-1.7a",    0x0000, 0x0020, CRC(ae1f4acd) SHA1(1d502b61db73cf6a4ac3d235455a5c464f12652a) ) /* BPROM type is N82S123 */
	ROM_LOAD( "prom-2.8a",    0x0020, 0x0020, CRC(9b713924) SHA1(943ad55d232f7bb99886a9a273dd14a1e1533491) ) /* BPROM type is N82S123 */
ROM_END



/*************************************
 *
 *  Driver init
 *
 *************************************/

static DRIVER_INIT( starfire )
{
	input_read = starfire_input_r;
}

static DRIVER_INIT( fireone )
{
	input_read = fireone_input_r;

	/* register for state saving */
	state_save_register_global(machine, fireone_select);
}



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1979, starfire, 0,        starfire, starfire, starfire, ROT0, "Exidy", "Star Fire (set 1)", GAME_NO_SOUND | GAME_SUPPORTS_SAVE )
GAME( 1979, starfirea,starfire, starfire, starfire, starfire, ROT0, "Exidy", "Star Fire (set 2)", GAME_NO_SOUND | GAME_SUPPORTS_SAVE )
GAME( 1979, fireone,  0,        starfire, fireone,  fireone,  ROT0, "Exidy", "Fire One", GAME_NO_SOUND | GAME_SUPPORTS_SAVE )
GAME( 1979, starfir2, 0,        starfire, starfire, starfire, ROT0, "Exidy", "Star Fire 2", GAME_NO_SOUND | GAME_SUPPORTS_SAVE )
