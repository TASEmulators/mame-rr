/* Diver Boy
 (c)1992 Device Electronics


 ----

 Here's the info about this dump:

 Name:            DiverBoy
 Manufacturer:    Unknown
 Year:            Unknown
 Date Dumped:     17-07-2002 (DD-MM-YYYY)

 CPU:             68000, Z80
 SOUND:           OKIM6295
 GFX:             Unknown

 About the game:

 The worst game i have :) Enjoy it so much as me :D

 ----

 Stephh's notes :

  - COIN3 gives ("Coinage" * 2) coins/credits :

     COIN1/2    COIN3
      4C_1C     2C_1C
      3C_1C    special   (see below)
      2C_1C     1C_1C
      1C_1C     1C_2C
      1C_2C     1C_4C
      1C_3C     1C_6C
      1C_4C     1C_8C
      1C_6C     1C_12C

    when "Coinage" set to 3C_1C, pressing COIN3 has this effect :

      * 1st coin : nothing
      * 2nd coin : adds 1 credit
      * 3rd coin : adds 1 credit
      * 4th coin : nothing
      * 5th coin : adds 1 credit
      * 6th coin : adds 1 credit ...

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"



class diverboy_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, diverboy_state(machine)); }

	diverboy_state(running_machine &machine) { }

	/* memory pointers */
	UINT16 *  spriteram;
//  UINT16 *  paletteram;   // currently this uses generic palette handling
	size_t    spriteram_size;

	/* devices */
	running_device *audiocpu;
};


static VIDEO_START(diverboy)
{
}

static void draw_sprites( running_machine* machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	diverboy_state *state = (diverboy_state *)machine->driver_data;
	UINT16 *source = state->spriteram;
	UINT16 *finish = source + (state->spriteram_size / 2);

	while (source < finish)
	{
		INT16 xpos, ypos, number, colr, bank, flash;

		ypos = source[4];
		xpos = source[0];
		colr = (source[1] & 0x00f0) >> 4;
		number = source[3];
		flash = source[1] & 0x1000;

		colr |= ((source[1] & 0x000c) << 2);

		ypos = 0x100 - ypos;

		bank = (source[1] & 0x0002) >> 1;

		if (!flash || (machine->primary_screen->frame_number() & 1))
		{
			drawgfx_transpen(bitmap,cliprect,machine->gfx[bank],
					number,
					colr,
					0,0,
					xpos,ypos,
					(source[1] & 0x0008) ? -1 : 0);
		}

		source += 8;
	}
}

static VIDEO_UPDATE(diverboy)
{
//  bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine));
	draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}


static WRITE16_HANDLER( soundcmd_w )
{
	diverboy_state *state = (diverboy_state *)space->machine->driver_data;

	if (ACCESSING_BITS_0_7)
	{
		soundlatch_w(space, 0, data & 0xff);
		cpu_set_input_line(state->audiocpu, 0, HOLD_LINE);
	}
}

static WRITE8_DEVICE_HANDLER( okibank_w )
{
	/* bit 2 might be reset */
//  popmessage("%02x",data);

	okim6295_device *oki = downcast<okim6295_device *>(device);
	oki->set_bank_base((data & 3) * 0x40000);
}



static ADDRESS_MAP_START( diverboy_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x040000, 0x04ffff) AM_RAM
	AM_RANGE(0x080000, 0x083fff) AM_RAM AM_BASE_SIZE_MEMBER(diverboy_state, spriteram, spriteram_size)
	AM_RANGE(0x100000, 0x100001) AM_WRITE(soundcmd_w)
	AM_RANGE(0x140000, 0x1407ff) AM_WRITE(paletteram16_xxxxBBBBGGGGRRRR_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x180000, 0x180001) AM_READ_PORT("P1_P2")
	AM_RANGE(0x180002, 0x180003) AM_READ_PORT("DSW")
	AM_RANGE(0x180008, 0x180009) AM_READ_PORT("COINS")
//  AM_RANGE(0x18000a, 0x18000b) AM_READNOP
//  AM_RANGE(0x18000c, 0x18000d) AM_WRITENOP
	AM_RANGE(0x320000, 0x3207ff) AM_WRITEONLY /* ?? */
	AM_RANGE(0x322000, 0x3227ff) AM_WRITEONLY /* ?? */
//  AM_RANGE(0x340000, 0x340001) AM_WRITENOP
//  AM_RANGE(0x340002, 0x340003) AM_WRITENOP
ADDRESS_MAP_END

static ADDRESS_MAP_START( snd_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x9000, 0x9000) AM_DEVWRITE("oki", okibank_w)
	AM_RANGE(0x9800, 0x9800) AM_DEVREADWRITE("oki", okim6295_r, okim6295_w)
	AM_RANGE(0xa000, 0xa000) AM_READ(soundlatch_r)
ADDRESS_MAP_END



static INPUT_PORTS_START( diverboy )
	PORT_START("P1_P2")	// 0x180000.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)	// unused ?
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)	// unused ?
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)		// "Dive"
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)		// unknown effect
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)	// unused ?
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)	// unused ?
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)		// "Dive"
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)		// unknown effect
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("DSW")	// 0x180002.w
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPNAME( 0x10, 0x10, "Display Copyright" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x60, 0x20, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_START("COINS")	// 0x180008.w
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )		// read notes
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )	// must be 00 - check code at 0x001680
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END



static const gfx_layout diverboy_spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{  4, 0,  12, 8,  20, 16, 28, 24,
	  36, 32, 44, 40, 52, 48, 60, 56 },
	{ 0*64, 1*64, 2*64,  3*64,  4*64,  5*64,  6*64,  7*64,
	  8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	16*64
};

static GFXDECODE_START( diverboy )
	GFXDECODE_ENTRY( "gfx1", 0, diverboy_spritelayout, 0, 4*16 )
	GFXDECODE_ENTRY( "gfx2", 0, diverboy_spritelayout, 0, 4*16 )
GFXDECODE_END


static MACHINE_START( diverboy )
{
	diverboy_state *state = (diverboy_state *)machine->driver_data;

	state->audiocpu = machine->device("audiocpu");
}

static MACHINE_DRIVER_START( diverboy )
	MDRV_DRIVER_DATA(diverboy_state)

	MDRV_CPU_ADD("maincpu", M68000, 12000000) /* guess */
	MDRV_CPU_PROGRAM_MAP(diverboy_map)
	MDRV_CPU_VBLANK_INT("screen", irq6_line_hold)

	MDRV_CPU_ADD("audiocpu", Z80, 4000000)
	MDRV_CPU_PROGRAM_MAP(snd_map)

	MDRV_MACHINE_START(diverboy)

	MDRV_GFXDECODE(diverboy)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8+4, 40*8+1, 2*8, 32*8-1)

	MDRV_PALETTE_LENGTH(0x400)

	MDRV_VIDEO_START(diverboy)
	MDRV_VIDEO_UPDATE(diverboy)

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_OKIM6295_ADD("oki", 1320000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END



ROM_START( diverboy )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "db_01.bin", 0x00000, 0x20000, CRC(6aa11366) SHA1(714c8a4a64c18632825a734a76a2d1b031106d76) )
	ROM_LOAD16_BYTE( "db_02.bin", 0x00001, 0x20000, CRC(45f8a673) SHA1(4eea1374cafacb4a2e0b623fcb802deb5fca1b3a) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* z80 */
	ROM_LOAD( "db_05.bin", 0x00000, 0x8000, CRC(ffeb49ec) SHA1(911b13897ff4ace3940bfff4ab88584a93796c24) ) /* this part is empty */
	ROM_CONTINUE( 0x0000, 0x8000 ) /* this part contains the code */

	ROM_REGION( 0x100000, "gfx1", 0 ) /* GFX */
	ROM_LOAD16_BYTE( "db_08.bin", 0x000000, 0x80000, CRC(7bb96220) SHA1(671b3f218106e594b13ae5f2e680cf2e2cfc5501) )
	ROM_LOAD16_BYTE( "db_09.bin", 0x000001, 0x80000, CRC(12b15476) SHA1(400a5b846f70567de137e0b95586dd9cfc27becb) )

	ROM_REGION( 0x180000, "gfx2", 0 ) /* GFX */
	ROM_LOAD16_BYTE( "db_07.bin", 0x000000, 0x20000, CRC(18485741) SHA1(a8edceaf34a98f2aa2bfada9d6e06fb82639a4e0) )
	ROM_LOAD16_BYTE( "db_10.bin", 0x000001, 0x20000, CRC(c381d1cc) SHA1(88b97d8893c500951cfe8e7e7f0b547b36bbe2c0) )
	ROM_LOAD16_BYTE( "db_06.bin", 0x040000, 0x20000, CRC(21b4e352) SHA1(a553de67e5dc751ea81ec4739724e0e46e8c5fab) )
	ROM_LOAD16_BYTE( "db_11.bin", 0x040001, 0x20000, CRC(41d29c81) SHA1(448fd5c1b16159d03436b8bd71ffe871c8daf7fa) )

	ROM_REGION( 0x100000, "oki", 0 ) /* Sound */
	ROM_LOAD( "db_03.bin", 0x00000, 0x20000, CRC(50457505) SHA1(faf1c055ec56d2ed7f5e6993cc04d3317bf1c3cc) )
	ROM_CONTINUE(          0x40000, 0x20000 )
	ROM_CONTINUE(          0x80000, 0x20000 )
	ROM_CONTINUE(          0xc0000, 0x20000 )
	ROM_LOAD( "db_04.bin", 0x20000, 0x20000, CRC(01b81da0) SHA1(914802f3206dc59a720af9d57eb2285bc8ba822b) ) /* same as tumble pop?, is this used? */
	ROM_RELOAD(            0x60000, 0x20000 )
	ROM_RELOAD(            0xa0000, 0x20000 )
	ROM_RELOAD(            0xe0000, 0x20000 )
ROM_END



GAME( 1992, diverboy, 0, diverboy, diverboy, 0, ORIENTATION_FLIP_X, "Electronic Devices Italy", "Diver Boy", GAME_SUPPORTS_SAVE )
