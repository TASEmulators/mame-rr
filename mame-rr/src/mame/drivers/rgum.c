/*

Royal Gum

Unknown CPU (either Z80 or Z180) - or at least rgum.u47 looks z80 related
rgum.u5 is for a 6502?

Big Black Box in the middle of the PCB (for encryption, or containing roms?)

The ppi at 3000-3003 seems to be a dual port communication thing with the z80.

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m6502/m6502.h"
#include "video/mc6845.h"
#include "machine/8255ppi.h"
#include "sound/ay8910.h"

static UINT8 *vram, *cram;

static VIDEO_START(royalgum)
{

}

static VIDEO_UPDATE(royalgum)
{
	int x,y,count;
	const gfx_element *gfx = screen->machine->gfx[0];

	count = 0;

	for(y=0;y<32;y++)
	{
		for(x=0;x<66;x++)
		{
			int tile = vram[count] | ((cram[count] & 0xf) <<8);

			drawgfx_opaque(bitmap,cliprect,gfx,tile,0,0,0,x*8,y*8);

			count++;
		}
	}

	return 0;
}

static ADDRESS_MAP_START( rgum_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_RAM //not all of it?

	AM_RANGE(0x0800, 0x0800) AM_DEVWRITE("crtc", mc6845_address_w)
	AM_RANGE(0x0801, 0x0801) AM_DEVREADWRITE("crtc", mc6845_register_r, mc6845_register_w)

	AM_RANGE(0x2000, 0x2000) AM_DEVWRITE("aysnd", ay8910_data_w)
	AM_RANGE(0x2002, 0x2002) AM_DEVREADWRITE("aysnd", ay8910_r, ay8910_address_w)

	AM_RANGE(0x2801, 0x2801) AM_READNOP //read but value discarded?
	AM_RANGE(0x2803, 0x2803) AM_READNOP

	AM_RANGE(0x3000, 0x3003) AM_DEVREADWRITE("ppi8255_0", ppi8255_r, ppi8255_w)

	AM_RANGE(0x4000, 0x47ff) AM_RAM AM_BASE(&vram)
	AM_RANGE(0x5000, 0x57ff) AM_RAM AM_BASE(&cram)

	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END


static CUSTOM_INPUT( rgum_heartbeat_r )
{
	static UINT8 hbeat;

	hbeat ^= 1;

	return hbeat;
}


static INPUT_PORTS_START( rgum )
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x01, "IN0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(rgum_heartbeat_r, NULL)
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "IN1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, "IN2" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNKNOWN ) //communication port with the z80?

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "DSW1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "DSW2" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

INPUT_PORTS_END


static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3),RGN_FRAC(1,3),RGN_FRAC(2,3) },
	{ 0, 1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( rgum )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0, 16 )
GFXDECODE_END


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

static const ppi8255_interface ppi8255_intf =
{
	DEVCB_INPUT_PORT("IN0"),		/* Port A read */
	DEVCB_INPUT_PORT("IN1"),		/* Port B read */
	DEVCB_INPUT_PORT("IN2"),		/* Port C read */
	DEVCB_NULL,		/* Port A write */
	DEVCB_NULL,		/* Port B write */
	DEVCB_NULL		/* Port C write */
};

static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

static MACHINE_DRIVER_START( rgum )
	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M65C02,24000000/16)		 /* ? MHz */
	MDRV_CPU_PROGRAM_MAP(rgum_map)
//  MDRV_CPU_VBLANK_INT("screen", nmi_line_pulse)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 256-1, 0, 256-1)

	MDRV_MC6845_ADD("crtc", MC6845, 24000000/16, mc6845_intf)	/* unknown clock & type, hand tuned to get ~50 fps (?) */

	MDRV_PPI8255_ADD( "ppi8255_0", ppi8255_intf )

	MDRV_GFXDECODE(rgum)
	MDRV_PALETTE_LENGTH(0x100)

	MDRV_VIDEO_START(royalgum)
	MDRV_VIDEO_UPDATE(royalgum)

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("aysnd", AY8910, 24000000/16) /* guessed to use the same xtal as the crtc */
	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END




ROM_START( rgum )
	ROM_REGION( 0x20000, "z80cpu", 0 )
	ROM_LOAD( "rgum.u47", 0x00000, 0x20000, CRC(fe410eb9) SHA1(25180ba336269279f251be5483c210a581d27197) ) // encrypted.. 2nd half empty

	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rgum.u5", 0x00000, 0x10000, CRC(9d2d1681) SHA1(1c1da0d970ea2cf58f7961417ab6986cc667da5c) ) // plaintext in here, but firt half is empty

	ROM_REGION( 0x10000, "unk", 0 )
	ROM_LOAD( "rgum.u6", 0x00000, 0x2000, CRC(15a34117) SHA1(c7e0aef4007abfaaa533feb026148ba03230b79f) ) // near the data rom, mostly empty

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "rgum.u16", 0x00000, 0x8000, CRC(2a2c8d78) SHA1(2ce335b900dccbc34ad8ae7ae02ec7c75ffcd559) ) // first half empty
	ROM_CONTINUE(0x00000,0x8000)
	ROM_LOAD( "rgum.u17", 0x08000,  0x8000, CRC(fae4e41a) SHA1(421aac2b567040c3a56e01aa70880c94450eaf76) ) // first half empty
	ROM_CONTINUE(0x08000,0x8000)
	ROM_LOAD( "rgum.u18", 0x10000, 0x8000, CRC(79b17da7) SHA1(31e1845261b0152df56135c212e55c4048b7496f) ) // first half empty
	ROM_CONTINUE(0x10000,0x8000)
ROM_END


GAME( 199?, rgum, 0, rgum, rgum, 0, ROT0, "<unknown>",         "Royal Gum (Italy)", GAME_NOT_WORKING | GAME_NO_SOUND )
