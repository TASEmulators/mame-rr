/*

note:
is it working properly?
no idea how the colours work
is there actually a sound chip?

it executes some illegal opcodes??

M6808: illegal opcode: address FDC2, op 53
M6808: illegal opcode: address FDC5, op 41
M6808: illegal opcode: address FDC6, op 49
M6808: illegal opcode: address FDCB, op 46
M6808: illegal opcode: address FDCE, op 53
M6808: illegal opcode: address FDD1, op 46
M6808: illegal opcode: address FDD3, op 4C
M6808: illegal opcode: address FDC2, op 53
M6808: illegal opcode: address FDC5, op 41
M6808: illegal opcode: address FDC6, op 49
M6808: illegal opcode: address FDCB, op 46
M6808: illegal opcode: address FDCE, op 53
M6808: illegal opcode: address FDD1, op 46
M6808: illegal opcode: address FDD3, op 4C
M6808: illegal opcode: address FDC2, op 53
M6808: illegal opcode: address FDC5, op 41
M6808: illegal opcode: address FDC6, op 49
M6808: illegal opcode: address FDCB, op 46
M6808: illegal opcode: address FDCE, op 53
M6808: illegal opcode: address FDD1, op 46
M6808: illegal opcode: address FDD3, op 4C


PCB layout
2005-07-26
f205v

?Poker?
----------
|----------------------|
| Fully boxed = socket |
|----------------------|


| separation = solder


------------------------------------------------------------------------
|                                                                      |
|      E          D          C          B          A                   |
|                                                                      |
| 1 | 74ls08   | 74ls175  | 74ls175  | 74s288   | CD4098BE  | D31B3100 |
|                                                                      |
|                                                                      |
| 2 | 74ls161  | OSCLLTR  | 74ls367  | 74ls05                          |
|                                                                      |
|                         |                                            |
| 3 | 74ls74   | 74ls04   | M                   | 74ls365              |C
|                         | C                                          |O
|                         | 6                                          |N
| 4 | 74ls139  | 74ls32   | 8                   | 74ls365              |N
|                         | 4                                          |E
|                         | 5                                          |C
| 5 | 74ls32   | 74ls32   | P        | 74ls175                         |T
|                         |                                            |O
|                                                                      |R
| 6 | rom A2   | 74ls273  | ram2114  | 74ls157                         |
|                                                                      |2
|                                                                      |2
| 7 | 74ls166  | 74ls245  | ram2114  | 74ls157                         |
|                                                                      |
|   ----------                                                         |
| 8 | rom A11| | 74ls245  | ram2114  | 74ls157                         |
|   ----------                                                         |
|   ----------                                                         |
| 9 | rom A10| | 8017DK-S6802P                                         |
|   ----------                                                         |
|                                                                      |
------------------------------------------------------------------------


*** Notes by Roberto Fresca ***

- Added Muroge Monaco (set 2).
- Added "Las Vegas, Nevada" (hack).
- Added the missing 74s288 dump (32x8 PROM located at 1B) from the new set.
- Confirmed the MC6845 at $4000/$4001.
- Corrected screen size acording to MC6845 registers.
- Corrected visible area acording to MC6845 registers. The last characters line looks odd, but should be visible.
- There are not illegal opcodes. The above mentioned are in fact strings (plain ASCII text).

MC6845 registers:
reg (hex):  00  01  02  03  04  05  06  07  08  09  0A  0B  0C  0D  0E  0F  10  11
val (hex):  27  20  22  04  26  00  20  20  00  07  00  00  80  00  00  00  ns  ns

*/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "video/mc6845.h"

static UINT8 *murogem_videoram;


static ADDRESS_MAP_START( murogem_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x007f) AM_RAM
	AM_RANGE(0x4000, 0x4000) AM_DEVWRITE("crtc", mc6845_address_w)
	AM_RANGE(0x4001, 0x4001) AM_DEVWRITE("crtc", mc6845_register_w)
	AM_RANGE(0x5000, 0x5000) AM_READ_PORT("IN0")
	AM_RANGE(0x5800, 0x5800) AM_READ_PORT("IN1")
	AM_RANGE(0x7000, 0x7000) AM_WRITENOP // sound? payout?
	AM_RANGE(0x8000, 0x87ff) AM_RAM AM_BASE(&murogem_videoram)
	AM_RANGE(0xf000, 0xffff) AM_ROM
ADDRESS_MAP_END


static INPUT_PORTS_START( murogem )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Bet (Replay)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_NAME("Deal")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_CANCEL ) PORT_NAME("Clear Selection")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Select Card 5")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Select Card 4")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Select Card 3")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Select Card 2")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Select Card 1")

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "Reset" )	// reduces credits to 0 and resets game??
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x20, "Scoring" )
	PORT_DIPSETTING(    0x00, "Type 1" )
	PORT_DIPSETTING(    0x20, "Type 2" )
	PORT_DIPSETTING(    0x40, "Type 3" )
	PORT_DIPSETTING(    0x60, "Invalid" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )
INPUT_PORTS_END



static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(0,2),RGN_FRAC(1,2) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( murogem )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0, 16 )
GFXDECODE_END


static PALETTE_INIT(murogem)
{}

static VIDEO_UPDATE(murogem)
{
	int xx,yy,count;
	count = 0x000;

	bitmap_fill(bitmap, cliprect, 0);

	for (yy=0;yy<32;yy++)
	{
		for(xx=0;xx<32;xx++)
		{
			int tileno = murogem_videoram[count]&0x3f;
			int attr = murogem_videoram[count+0x400]&0x0f;

			drawgfx_transpen(bitmap,cliprect,screen->machine->gfx[0],tileno,attr,0,0,xx*8,yy*8,0);

			count++;

		}

	}

	return 0;
}

static const mc6845_interface mc6845_intf =
{
	"screen",	/* screen we are acting on */
	8,			/* number of pixels per video memory address */
	NULL,		/* before pixel update callback */
	NULL,		/* row update callback */
	NULL,		/* after pixel update callback */
	DEVCB_NULL,	/* callback for display state changes */
	DEVCB_NULL,	/* callback for cursor state changes */
	DEVCB_NULL,	/* HSYNC callback */
	DEVCB_NULL,	/* VSYNC callback */
	NULL		/* update address callback */
};


static MACHINE_DRIVER_START( murogem )
	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M6802,8000000)		 /* ? MHz */
	MDRV_CPU_PROGRAM_MAP(murogem_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE((39+1)*8, (38+1)*8)           // Taken from MC6845 init, registers 00 & 04. Normally programmed with (value-1).
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)    // Taken from MC6845 init, registers 01 & 06.

	MDRV_GFXDECODE(murogem)
	MDRV_PALETTE_LENGTH(0x100)

	MDRV_PALETTE_INIT(murogem)
	MDRV_VIDEO_UPDATE(murogem)

	MDRV_MC6845_ADD("crtc", MC6845, 750000, mc6845_intf) /* ? MHz */
MACHINE_DRIVER_END


ROM_START( murogem )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a11-8.8e", 0xf000, 0x0800, CRC(1135345e) SHA1(ae23786a6e2b1b077ce1a183d547af42318ac4d9)  )
	ROM_LOAD( "a10-9.9e", 0xf800, 0x0800, CRC(f96791d9) SHA1(12b85e0f8b20ea9331f8cb2b2cf2a4383bdb8003)  )

	ROM_REGION( 0x0400, "gfx1", 0 )
	ROM_LOAD( "a2.6e", 0x0000, 0x0400, CRC(86e053da) SHA1(b7cdddca273204513c818384860883bf54cf9434)  )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "a3.1b", 0x0000, 0x0020, CRC(abddfb6b) SHA1(ed78b93701b5a3bf2053d2584e9a354fb6cec203) )	/* 74s288 at 1B */

ROM_END

ROM_START( murogema )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "poker.02", 0xf000, 0x0800, CRC(e82a5bed) SHA1(556a041cd90e628b6c26824e58cee679dfd59f50)  )
	ROM_LOAD( "poker.03", 0xf800, 0x0800, CRC(f3fe3f12) SHA1(9c73fbaee20098c734431c6af3168986630dcb67)  )

	ROM_REGION( 0x0400, "gfx1", 0 )
	ROM_LOAD( "poker.01", 0x0000, 0x0400, CRC(164d7443) SHA1(1421a3d32d1296a2544da16b51ade94a58e8ba03)  )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "a3.1b", 0x0000, 0x0020, CRC(abddfb6b) SHA1(ed78b93701b5a3bf2053d2584e9a354fb6cec203) )	/* 74s288 at 1B. Originally named 6336.pkr */

ROM_END

ROM_START( lasvegas )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pk8.8e", 0xf000, 0x0800, CRC(995bd527) SHA1(af96bd0118511b13a755925e3bf5138be61c09d8)  )
	ROM_LOAD( "pk9.9e", 0xf800, 0x0800, CRC(2ab1556e) SHA1(cd7bd377b6a3f6c0f8df61b0da83e55468d599d6)  )

	ROM_REGION( 0x0800, "gfx1", 0 )	// the second half is filled of 0xff.
	ROM_LOAD( "pk6.6e", 0x0000, 0x0800, CRC(78a3593a) SHA1(96ba470f5b0dd6d490eadd09b4b6894e044c66b4)  )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "a3.1b", 0x0000, 0x0020, CRC(abddfb6b) SHA1(ed78b93701b5a3bf2053d2584e9a354fb6cec203) )	/* 74s288 at 1B */

ROM_END

GAME( 198?, murogem,  0,       murogem, murogem, 0, ROT0, "<unknown>", "Muroge Monaco (set 1)", GAME_NO_SOUND|GAME_WRONG_COLORS )
GAME( 198?, murogema, murogem, murogem, murogem, 0, ROT0, "<unknown>", "Muroge Monaco (set 2)", GAME_NO_SOUND|GAME_WRONG_COLORS )
GAME( 198?, lasvegas, murogem, murogem, murogem, 0, ROT0, "hack",      "Las Vegas, Nevada", GAME_NO_SOUND|GAME_WRONG_COLORS )
