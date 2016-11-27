/* Hanaroku - Alba ZC HW */

/*
TODO:
- colour decoding might not be perfect
- Background color should be green, but current handling might be wrong.
- some unknown sprite attributes
- don't know what to do when the jackpot is displayed (missing controls ?)
- according to the board pic, there should be one more 4-switches dip
  switch bank, and probably some NVRAM because there's a battery.
*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"

class albazc_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, albazc_state(machine)); }

	albazc_state(running_machine &machine) { }

	/* video-related */
	UINT8 *  spriteram1;
	UINT8 *  spriteram2;
	UINT8 *  spriteram3;
	UINT8 flip_bit;
};



/* video */

static PALETTE_INIT( hanaroku )
{
	int i;
	int r, g, b;

	for (i = 0; i < 0x200; i++)
	{
		b = (color_prom[i * 2 + 1] & 0x1f);
		g = ((color_prom[i * 2 + 1] & 0xe0) | ((color_prom[i * 2 + 0]& 0x03) <<8)) >> 5;
		r = (color_prom[i * 2 + 0] & 0x7c) >> 2;

		palette_set_color_rgb(machine, i, pal5bit(r), pal5bit(g), pal5bit(b));
	}
}


static VIDEO_START( hanaroku )
{
}

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	albazc_state *state = (albazc_state *)machine->driver_data;
	int i;

	for (i = 511; i >= 0; i--)
	{
		int code = state->spriteram1[i] | (state->spriteram2[i] << 8);
		int color = (state->spriteram2[i + 0x200] & 0xf8) >> 3;
		int flipx = 0;
		int flipy = 0;
		int sx = state->spriteram1[i + 0x200] | ((state->spriteram2[i + 0x200] & 0x07) << 8);
		int sy = 242 - state->spriteram3[i];

		if (state->flip_bit)
		{
			sy = 242 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap, cliprect, machine->gfx[0], code, color, flipx, flipy,
			sx, sy, 0);
	}
}

static VIDEO_UPDATE(hanaroku)
{
	bitmap_fill(bitmap, cliprect, 0x1f0);	// ???
	draw_sprites(screen->machine, bitmap, cliprect);
	return 0;
}

static WRITE8_HANDLER( hanaroku_out_0_w )
{
	/*
        bit     description

         0      meter1 (coin1)
         1      meter2 (coin2)
         2      meter3 (1/2 d-up)
         3      meter4
         4      call out (meter)
         5      lockout (key)
         6      hopper2 (play)
         7      meter5 (start)
    */

	coin_counter_w(space->machine, 0, data & 0x01);
	coin_counter_w(space->machine, 1, data & 0x02);
	coin_counter_w(space->machine, 2, data & 0x04);
	coin_counter_w(space->machine, 3, data & 0x08);
	coin_counter_w(space->machine, 4, data & 0x80);
}

static WRITE8_HANDLER( hanaroku_out_1_w )
{
	/*
        bit     description

         0      hopper1 (data clear)
         1      dis dat
         2      dis clk
         3      pay out
         4      ext in 1
         5      ext in 2
         6      ?
         7      ?
    */
}

static WRITE8_HANDLER( hanaroku_out_2_w )
{
	// unused
}

static WRITE8_HANDLER( albazc_vregs_w )
{
	albazc_state *state = (albazc_state *)space->machine->driver_data;

	#ifdef UNUSED_FUNCTION
	{
		static UINT8 x[5];
		x[offset] = data;
		popmessage("%02x %02x %02x %02x %02x",x[0],x[1],x[2],x[3],x[4]);
	}
	#endif

	if(offset == 0)
	{
		/* core bug with this? */
		//flip_screen_set(space->machine, (data & 0x40) >> 6);
		state->flip_bit = (data & 0x40) >> 6;
	}
}

/* main cpu */

static ADDRESS_MAP_START( hanaroku_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM AM_BASE_MEMBER(albazc_state, spriteram1)
	AM_RANGE(0x9000, 0x97ff) AM_RAM AM_BASE_MEMBER(albazc_state, spriteram2)
	AM_RANGE(0xa000, 0xa1ff) AM_RAM AM_BASE_MEMBER(albazc_state, spriteram3)
	AM_RANGE(0xa200, 0xa2ff) AM_WRITENOP	// ??? written once during P.O.S.T.
	AM_RANGE(0xa300, 0xa304) AM_WRITE(albazc_vregs_w)	// ???
	AM_RANGE(0xb000, 0xb000) AM_WRITENOP	// ??? always 0x40
	AM_RANGE(0xc000, 0xc3ff) AM_RAM			// main ram
	AM_RANGE(0xc400, 0xc4ff) AM_RAM			// ???
	AM_RANGE(0xd000, 0xd000) AM_DEVREAD("aysnd", ay8910_r)
	AM_RANGE(0xd000, 0xd001) AM_DEVWRITE("aysnd", ay8910_address_data_w)
	AM_RANGE(0xe000, 0xe000) AM_READ_PORT("IN0") AM_WRITE(hanaroku_out_0_w)
	AM_RANGE(0xe001, 0xe001) AM_READ_PORT("IN1")
	AM_RANGE(0xe002, 0xe002) AM_READ_PORT("IN2") AM_WRITE(hanaroku_out_1_w)
	AM_RANGE(0xe004, 0xe004) AM_READ_PORT("DSW3") AM_WRITE(hanaroku_out_2_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( hanaroku )
	PORT_START("IN0")	/* 0xe000 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )		// adds n credits depending on "Coinage" Dip Switch
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )		// adds 5 credits
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("1/2 D-Up") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Reset") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Meter") PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Key") PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 ) PORT_NAME("Play")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start")

	PORT_START("IN1")	/* 0xe001 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_B )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_HANAFUDA_C )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_HANAFUDA_D )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_HANAFUDA_E )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_HANAFUDA_F )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_HANAFUDA_YES )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_HANAFUDA_NO )

	PORT_START("IN2")	/* 0xe002 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_NAME("Data Clear")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("Medal In") PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("Pay Out") PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Ext In 1")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Ext In 2")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")	/* 0xd000 - Port A */
	PORT_BIT(  0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW2")	/* 0xd000 - Port B */
	PORT_BIT(  0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW3")	/* 0xe004 */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )		// Stored at 0xc028
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Flip_Screen ) )	// Stored at 0xc03a
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )		// Stored at 0xc078
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x20, "Game Mode" )				// Stored at 0xc02e
	PORT_DIPSETTING(    0x30, "Mode 0" )				// Collect OFF
	PORT_DIPSETTING(    0x20, "Mode 1" )				// Collect ON (code at 0x36ea)
	PORT_DIPSETTING(    0x10, "Mode 2" )				// Collect ON (code at 0x3728)
	PORT_DIPSETTING(    0x00, "Mode 3" )				// No credit counter
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )
INPUT_PORTS_END


static const gfx_layout hanaroku_charlayout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4),RGN_FRAC(2,4),RGN_FRAC(1,4),RGN_FRAC(0,4) },
	{ 0,1,2,3,4,5,6,7,
		64,65,66,67,68,69,70,71},
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		128+0*8,128+1*8,128+2*8,128+3*8,128+4*8,128+5*8,128+6*8,128+7*8 },
	16*16
};



static GFXDECODE_START( hanaroku )
	GFXDECODE_ENTRY( "gfx1", 0, hanaroku_charlayout,   0, 32  )
GFXDECODE_END


static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_INPUT_PORT("DSW1"),
	DEVCB_INPUT_PORT("DSW2"),
	DEVCB_NULL,
	DEVCB_NULL
};


static MACHINE_DRIVER_START( hanaroku )

	/* driver data */
	MDRV_DRIVER_DATA(albazc_state)

	MDRV_CPU_ADD("maincpu", Z80,6000000)		 /* ? MHz */
	MDRV_CPU_PROGRAM_MAP(hanaroku_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 64*8)
	MDRV_SCREEN_VISIBLE_AREA(0, 48*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(hanaroku)
	MDRV_PALETTE_LENGTH(0x200)

	MDRV_PALETTE_INIT(hanaroku)
	MDRV_VIDEO_START(hanaroku)
	MDRV_VIDEO_UPDATE(hanaroku)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("aysnd", AY8910, 1500000) /* ? MHz */
	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END


ROM_START( hanaroku )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* z80 code */
	ROM_LOAD( "zc5_1a.u02",  0x00000, 0x08000, CRC(9e3b62ce) SHA1(81aee570b67950c21ab3c8f9235dd383529b34d5) )

	ROM_REGION( 0x20000, "gfx1", 0 ) /* tiles */
	ROM_LOAD( "zc0_002.u14",  0x00000, 0x08000, CRC(76adab7f) SHA1(6efbe52ae4a1d15fe93bd05058546bf146a64154) )
	ROM_LOAD( "zc0_003.u15",  0x08000, 0x08000, CRC(c208e64b) SHA1(0bc226c39331bb2e1d4d8f756199ceec85c28f28) )
	ROM_LOAD( "zc0_004.u16",  0x10000, 0x08000, CRC(e8a46ee4) SHA1(09cac230c1c49cb282f540b1608ad33b1cc1a943) )
	ROM_LOAD( "zc0_005.u17",  0x18000, 0x08000, CRC(7ad160a5) SHA1(c897fbe4a7c2a2f352333131dfd1a76e176f0ed8) )

	ROM_REGION( 0x0400, "proms", 0 ) /* colour */
	ROM_LOAD16_BYTE( "zc0_006.u21",  0x0000, 0x0200, CRC(8e8fbc30) SHA1(7075521bbd790c46c58d9e408b0d7d6a42ed00bc) )
	ROM_LOAD16_BYTE( "zc0_007.u22",  0x0001, 0x0200, CRC(67225de1) SHA1(98322e71d93d247a67fb4e52edad6c6c32a603d8) )
ROM_END


GAME( 1988, hanaroku, 0,        hanaroku, hanaroku, 0, ROT0, "Alba", "Hanaroku", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_COLORS | GAME_SUPPORTS_SAVE )
