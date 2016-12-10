/************************************************************************

  Italian Gambling games based on Mitsubishi (Renesas) M16C MCU family.

  Written by Roberto Fresca.


  All these games use MCUs with internal ROM for their programs.

  They have 256KB of internal flash ROM that can't be dumped easily,
  and thus we can't emulate them at the moment because there is
  nothing to emulate.

  This driver is just a placeholder for the graphics/sound ROM loading


*************************************************************************

  --- Hardware Notes ---

  The hardware is normally composed by:


  CPU:   1x M30624FGAFP.
           (256KB ROM; 20KB RAM)

  Sound: 1x OKI M6295.
         1x TDA2003 (audio amplifier).
         1x LM358M (audio amplifier).

  PLDs:  2x ispLSI1032E-70JL.

  Clock: 1x Xtal 16.000 MHz.

  ROMs:  1x (up to) 27C4001 or similar (sound).
         4x 27C4001 or similar (graphics).


************************************************************************/

#define MAIN_CLOCK	XTAL_16MHz

#include "emu.h"
#include "cpu/h83002/h8.h"
#include "sound/okim6295.h"


static int test_x, test_y, start_offs;

/*************************
*     Video Hardware     *
*************************/

static VIDEO_START( itgambl3 )
{
	test_x = 256;
	test_y = 256;
	start_offs = 0;
}

/* (dirty) debug code for looking 8bpps blitter-based gfxs */
static VIDEO_UPDATE( itgambl3 )
{
	int x,y,count;
	const UINT8 *blit_ram = memory_region(screen->machine,"gfx1");

	if(input_code_pressed(screen->machine, KEYCODE_Z))
		test_x++;

	if(input_code_pressed(screen->machine, KEYCODE_X))
		test_x--;

	if(input_code_pressed(screen->machine, KEYCODE_A))
		test_y++;

	if(input_code_pressed(screen->machine, KEYCODE_S))
		test_y--;

	if(input_code_pressed(screen->machine, KEYCODE_Q))
		start_offs+=0x200;

	if(input_code_pressed(screen->machine, KEYCODE_W))
		start_offs-=0x200;

	if(input_code_pressed(screen->machine, KEYCODE_E))
		start_offs++;

	if(input_code_pressed(screen->machine, KEYCODE_R))
		start_offs--;

	popmessage("%d %d %04x",test_x,test_y,start_offs);

	bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine));

	count = (start_offs);

	for(y=0;y<test_y;y++)
	{
		for(x=0;x<test_x;x++)
		{
			UINT32 color;

			color = (blit_ram[count] & 0xff)>>0;

			if((x)<screen->visible_area().max_x && ((y)+0)<screen->visible_area().max_y)
				*BITMAP_ADDR32(bitmap, y, x) = screen->machine->pens[color];

			count++;
		}
	}

	return 0;
}


/*************************
* Memory map information *
*************************/

static ADDRESS_MAP_START( itgambl3_map, ADDRESS_SPACE_PROGRAM, 16 )
	ADDRESS_MAP_GLOBAL_MASK(0xffffff)
	AM_RANGE(0x000000, 0xffffff) AM_ROM
ADDRESS_MAP_END


/*************************
*      Input ports       *
*************************/

static INPUT_PORTS_START( itgambl3 )
    PORT_START("IN0")
    PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


/*************************
*    Graphics Layouts    *
*************************/

static const gfx_layout gfxlayout_8x8x8 =
{
/* this is wrong and need to be fixed */

	8,8,
	RGN_FRAC(1,1),
	8,
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64 },
	8*64
};


/******************************
* Graphics Decode Information *
******************************/

static GFXDECODE_START( itgambl3 )
    GFXDECODE_ENTRY( "gfx1", 0, gfxlayout_8x8x8,   0, 16  )
GFXDECODE_END


/**************************
*      Machine Reset      *
**************************/

static MACHINE_RESET( itgambl3 )
{
	/* stop the CPU, we have no code for it anyway */
	cputag_set_input_line(machine, "maincpu", INPUT_LINE_HALT, ASSERT_LINE);
}

/* default 444 palette for debug purpose*/
static PALETTE_INIT( itgambl3 )
{
	int x,r,g,b;

	for(x=0;x<0x100;x++)
	{
		r = (x & 0xf)*0x10;
		g = ((x & 0x3c)>>2)*0x10;
		b = ((x & 0xf0)>>4)*0x10;
		palette_set_color(machine,x,MAKE_RGB(r,g,b));
	}
}


/**************************
*     Machine Drivers     *
**************************/

static MACHINE_DRIVER_START( itgambl3 )

    /* basic machine hardware */
	MDRV_CPU_ADD("maincpu", H83044, MAIN_CLOCK)	/* wrong CPU, but we have not a M16C core ATM */
	MDRV_CPU_PROGRAM_MAP(itgambl3_map)

    /* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(512, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-1)
	MDRV_MACHINE_RESET( itgambl3 )
	MDRV_PALETTE_INIT( itgambl3 )

	MDRV_GFXDECODE(itgambl3)
	MDRV_PALETTE_LENGTH(0x200)
	MDRV_VIDEO_START( itgambl3 )
	MDRV_VIDEO_UPDATE( itgambl3 )

    /* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_OKIM6295_ADD("oki", MAIN_CLOCK/16, OKIM6295_PIN7_HIGH)	/* 1MHz */
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


/*************************
*        Rom Load        *
*************************/

/* Euro Jolly X5

CPU:

1x M30624FGAFP-03001A4 (u1)(main)

This one is a Mitsubishi (Renesas) M16/62A 16bit microcomputer.
It has 256 KB of internal flash ROM + 20 KB of RAM.

1x OKI M6295 (u22)(sound)
1x TDA2003 (u25)(sound)
1x LM358M (u23)(sound)
1x oscillator 16.000MHz (u20)


ROMs:

4x M27C4001 (u21, u15, u16, u17)


PLDs:

1x ST93C46 (u18)
(1K 64 x 16 or 128 x 8 serial microwirw EEPROM)

2x ispLSI1032E-70LJ


Note:

1x JAMMA style edge connector
1x RS232 connector (P1) (along with an ST232C controller (u12)
1x 6 legs jumper (jp1)
1x 8 legs jumper (jp_1)
1x 7 legs jumper (jp2)
1x 4 legs jumper (jp3)
1x red led (d7)
1x battery (bt1)
2x trimmer (r6,r33) (volume)
1x pushbutton (s1)

*/

ROM_START( ejollyx5 )	/* CPU and clock should be changed for this game */
	ROM_REGION( 0x1000000, "maincpu", 0 )	/* all the program code is in here */
	ROM_LOAD( "ejollyx5_m30624fgafp.mcu", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x180000, "gfx1", 0 )	/* different encoded gfx */
	ROM_LOAD( "eurojolly5-ep01.u15", 0x000000, 0x80000, CRC(feb4ef88) SHA1(5a86e92326096e4e0619a8aa6b491553eb46839d) )
	ROM_LOAD( "eurojolly5-ep02.u17", 0x080000, 0x80000, CRC(83b2dab0) SHA1(a65cae227a444fe7474f8f821dbb6a8b506e4ae6) )
	ROM_LOAD( "eurojolly5-ep03.u16", 0x100000, 0x80000, CRC(a0599d3c) SHA1(f52928cd75b4374a45fad37b7a7c1d39ea31b5f2) )

	ROM_REGION( 0x80000, "oki", 0 ) /* M6295 samples, identical halves */
	ROM_LOAD( "eurojolly5-msg0.u21", 0x00000, 0x80000, CRC(edc157bc) SHA1(8400251ca7a74a4a0f2d443ae2c0254f1de955ac) )
ROM_END


/* Grand Prix

CPU:

1x M30624FGAFP-251G108 (u21)(main)

This one is a Mitsubishi (Renesas) M16/62A 16bit microcomputer.
It has 256 KB of internal flash ROM + 20 KB of RAM.

1x OKI M6295 (u2)(sound)
1x TDA2003 (u1)(sound)
1x LM358M (u23)(sound)
1x oscillator 16.000MHz (u9)


ROMs:

1x AM27C040 (u3)
1x MX29F1610 (u22)


PLDs:

2x ispLSI1032E-70LJ-E011S03 (u10, U11)


Note:

1x JAMMA edge connector (jp9)
1x 4 legs connector (jp3)
1x 22x2 legs connector (jp5)
1x 7x2 legs connector (jp17)
1x 9 pins (serial?) connector (jp4)

1x battery
1x trimmer (volume) (rv1)
1x red led (d1)
1x pushbutton (test mode) (RDP2)

PCB N? KGS0243-DF070283/03 made in Italy

*/

ROM_START( grandprx )	/* CPU and clock should be changed for this game */
	ROM_REGION( 0x1000000, "maincpu", 0 )	/* all the program code is in here */
	ROM_LOAD( "grandprx_m30624fgafp.mcu", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x200000, "gfx1", 0 )	/* different encoded gfx */
	ROM_LOAD( "u22.bin", 0x000000, 0x200000, CRC(e8ec804f) SHA1(84e647f693e0273b9b09d7726b814516496121a9) )

	ROM_REGION( 0x80000, "oki", 0 ) /* M6295 samples, identical halves */
	ROM_LOAD( "u3.bin", 0x00000, 0x80000, CRC(c9bb5858) SHA1(c154df7c7dfe394fc1963dc0c73f1d909f5b62ee) )
ROM_END

/*

Super Jolly

CPU:

1x M30624FGAFP-03001A4 (u1)(main)(not dumped)
1x OKI M6295 (u28)(sound)
1x TDA2003 (sound)
1x LM358M (u33)(sound)
1x oscillator 16.000MHz (osc1)

ROMs:

1x MX26C1000A (u29)
3x M27C4001 (u23,u24,u25)
1x ST93C46 (u35)(not dumped)
2x ispLSI1032E-70LJ (PLD)(not dumped)

Note:
1x JAMMA edge connector (JP11)
1x RS232 connector (JP15) (along with an ST232C controller (u34)
1x 4 legs connector (jp18)
1x red led (d3)
1x battery (bt1)
1x trimmer (r9) (volume)
1x trimmer (RVTRIMMER10KHOM)
1x pushbutton (s1)

*/

ROM_START( supjolly )	/* CPU and clock should be changed for this game */
	ROM_REGION( 0x1000000, "maincpu", 0 )	/* all the program code is in here */
	ROM_LOAD( "supjolly_m30624fgafp.mcu", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x180000, "gfx1", 0 )	/* different encoded gfx */
	ROM_LOAD( "1.u23", 0x000000, 0x80000, CRC(feb4ef88) SHA1(5a86e92326096e4e0619a8aa6b491553eb46839d) )
	ROM_LOAD( "2.u24", 0x080000, 0x80000, CRC(83b2dab0) SHA1(a65cae227a444fe7474f8f821dbb6a8b506e4ae6) )
	ROM_LOAD( "3.u25", 0x100000, 0x80000, CRC(3648fcc4) SHA1(c3c4a4f47866783589ca7124efa30c7e902423c1) )

	ROM_REGION( 0x80000, "oki", 0 ) /* M6295 samples, identical halves */
	ROM_LOAD( "saws.u29", 0x00000, 0x20000, CRC(e8612586) SHA1(bf536597a4cf1af5e9f701f2ecd1718320c06edd) )
ROM_END

/*

X Five Jokers

CPUs
1x  M30624FGAFP         u11     16-bit Single-Chip Microcomputer - main (internal ROM not dumped)
1x  MSM6295             u28     4-Channel Mixing ADCPM Voice Synthesis LSI - sound
1x  LM358N              u33     Dual Operational Amplifier - sound
1x  TDA2003             u32     Audio Amplifier - sound
1x  oscillator  16.000MHz   u27
ROMs
4x  M27C4001    1,2,3,S     dumped
RAMs
3x  D431000AGW-70LL     u20,u21,u22
PLDs
2x  ispLSI1032E-70LJ    u12,u13     not dumped
Others

1x 28x2 edge connector
1x 8x2 ISP connector
1x RSR232 connector (JP15)
1x 8 legs connector (JP16)
1x 7 legs connector (JP17)
1x 4 legs connector (JP18)
2x trimmer (volume,spark)
1x pushbutton (TEST)
1x red LED
1x battery 3.6V

*/

ROM_START( x5jokers )	/* CPU and clock should be changed for this game */
	ROM_REGION( 0x1000000, "maincpu", 0 )	/* all the program code is in here */
	ROM_LOAD( "x5jokers_m30624fgafp.mcu", 0x00000, 0x4000, NO_DUMP )

	ROM_REGION( 0x180000, "gfx1", 0 )
	ROM_LOAD( "xfivej3.u23", 0x000000, 0x80000, CRC(c01f1b2d) SHA1(608df59adcc0d7166dfb056bab0e31b8e75d9779) )
	ROM_LOAD( "xfivej2.u24", 0x080000, 0x80000, CRC(d12176f7) SHA1(49c56025e1b2a4cea9711c80e09c786f24b6dce0) )
	ROM_LOAD( "xfivej1.u25", 0x100000, 0x80000, CRC(cdac7a77) SHA1(7487fcb211dc2ff9a5bccefdff0d9d541f1f742b) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "xfivejs.u29", 0x00000, 0x80000, CRC(67d51cb4) SHA1(9182a63473a32a9ad91a7a6a47d5a5d965e3cb03) )
ROM_END



/*************************
*      Game Drivers      *
*************************/

/*    YEAR  NAME      PARENT  MACHINE   INPUT     INIT ROT    COMPANY        FULLNAME        FLAGS  */
GAME( 200?, ejollyx5, 0,      itgambl3, itgambl3, 0,   ROT0, "Solar Games",           "Euro Jolly X5",                  GAME_NOT_WORKING )
GAME( 200?, grandprx, 0,      itgambl3, itgambl3, 0,   ROT0, "4fun",                  "Grand Prix",                     GAME_NOT_WORKING )
GAME( 200?, supjolly, 0,      itgambl3, itgambl3, 0,   ROT0, "<unknown>",             "Super Jolly",                    GAME_NOT_WORKING )
GAME( 200?, x5jokers, 0,      itgambl3, itgambl3, 0,   ROT0, "Electronic Projects",   "X Five Jokers (Version 1.12)",   GAME_NOT_WORKING )
