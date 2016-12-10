/*
   Super Kaneko Nova System
    Original Driver by Sylvain Glaize
    taken to pieces and attempted reconstruction by David Haywood

Mainboard + Cart combo
  Regions are not cross compatible and have their own BIOS
   Regions by color coded cart:

    White      = Japan
    Light Blue = Europe
    Dark Blue  = USA
    Green      = Asia
    Red        = Korean

  Credits (in no particular order):
   Olivier Galibert for all the assistance and information he's provided
   R.Belmont for working on the SH2 timers so sound worked
   Nicola Salmoria for hooking up the Roz and improving the dirty tile handling
   Paul Priest for a lot of things
   Stephh for spotting what was wrong with Puzz Loop's inputs

Puzz Loop is currently the only game dumped for all known regions.  This game is normally found on a ROM 4 BOARD
   so the "U" location is printed on the label as U4 & U6.  However this same game has also been found on the
   ROM-2-BOARD using EPROMs labeled for the ROM 4 BOARD, but inserted in sockets at U8 & U10

ToDo:
   Priorities
   Transparency effects (each pen has a transparent flag, there are seperate r,g,b alpha values to be used for these pens)
   Idle skipping on other games + bios ..
   Fix gaps between zoomed sprites (See Gals Panic 4 continue screen and timer)
   Fix broken games

Alpha pens:
   galpanis uses them to white in/out the bg's during attract? Solid white atm.

Priorities:
   Puzzloop appears to use the priorities, the other games might use alpha effects to disable a layer instead ..
     *they don't appear to use alpha, so how should it work, senknow intro, galpanis attract,  cyvern level 4?

Ram-Based Tiles:
   The front layer in galpanic games
   The kaneko logo in galpanis
   The Bios

Sprite Zoom:
   Used all over the place in the Gals Panic games, check the bit in galpani4 attract where if you hammer the button the ballon expands until it bursts.
   Puzz Loop main logo

Roz:
   Should be used on galpani4 bg's, but for some reason incxx,incxy,incyx,incyy are always 1,0,0,1 resp. Maybe a timer problem, input problem or core problem. Who knows?
      Update, rand() was returning a 15-bit number under mingw. Top bit seemed to determine whether it rotated or not.
   To zoom the skull in the senknow attract mode
   To zoom the girl at the end of a level in galpanis
   To zoom the kaneko logo in galpanis

Linescroll:
   Puzzloop intro, the screen covered in 'puzz loop' of varying sizes, the little ones go left and the big ones right.
   Puzzloop title screen uses it just to scroll the bg up.
   Also used behind the hi-score table on puzzloop...
   Galpani4/galpanis use it for 2P mode on both bg A and bg B.

R,G,B Brightness:
   The Electro Design games have this set darker than 0xff for sprites and bg's anyway.
   Galpanis uses it to dim the bg's in attract on the stats screens for the characters (bg's hidden by white pen atm).
Lots of games use them to fade, haven't seen r,g,b used independently yet but you never know.

etc.etc.

-pjp/dh

gutsn Crash Reason
the routine at 401d8c8 calls 40249e0 with r14 as a parameter, which points where the return pr on the stack is
param is 60ffbc0, pr is at 60ffbcc, but 40249e0 writes more than 12 bytes
Gutsn now works in 0.69, timing/sh2 fixes??

Motherboard Layout
------------------

SUPER-KANEKO-NOVA-SYSTEM
MAIN-BOARD-A
NEP-16


   BATT                                      HM514260
   CR2032                       SKNSA1       HM514260       SH2
            W24257                          LH5168
YMZ280B-F   W24257   BABY004  LATTICE  SW1  M62X42
                              pLSI1016
  HITACHI          28.636MHz 33.3333MHz
  DF1                        21.504MHz
J D49307     ALTAIR                   DENEB
             BY006-224                BY007-32F
A            (QFP208)                 (QFP208)

M                  W24257  W24257           HM514260
            /-     W24257  W24257                                 /-
M VEGA      ||             W24257  SPCII-B  HM514260              ||
  BY005-197 ||             W24257  JH-6186                        ||
A (QFP144)  ||                     (QFP208)                       ||
            ||                               W24257               ||
  DSW1(8)   ||      VIEWIII-A      SPCII-A   W24257 LH540202      ||
            ||      BL-001         JH-4181   W24257               ||
            \-      (QFP240)       (QFP208)  W24257               \-
  OKIM6253


Notes:
      SKNSA1 is BIOS (Asia)
      HM514260, W24257, LH5168, M62X42 all smt SRAM
      LH540202 is DIP SRAM


Cart Layout
-----------

SUPER-KANEKO-NOVA-SYSTEM
ROM-BOARD
NEP-16


 /-                                                    /-
 ||SK300-00  *  PAL PAL D431000 D431000  *       *     ||
 ||                                                    ||
 ||                                                    ||
 ||                        *   SK-200-00               ||
 ||          SK01A  SK01A                              ||
 ||          U8     U10    *      *    SK-101 SK-100-00||
 \-                                                    \-

 Notes:
       *: unpopulated position for surface mounted 16MBit SOP44 MASK ROM
       U8 and U10 are socketed 27C040 EPROM
       All other ROMs are surface mounted SOP44 MASK ROM

Cart Layout
-----------

SUPER KANEKO NOVA SYSTEM
ROM 4 BOARD
NEP-16

Top Side:
 /-                                                    /-
 ||SS300-00 SS210-00     SS102-00 SS103-00   SS201-00  ||
 ||U1       U3           U8       U32        U9        ||
 ||                                                    ||
 ||                                                    ||
 || 082*    SG01A  SG01A  #      #                     ||
 ||          U4     U6    U29    U30                   ||
 \-                                                    \-

Bottom Side:
 /-                                                    /-
 ||SS200-00 SS000-00     SS101-00       ^       ^      ||
 ||U17      U21          U20            U24     U26    ||
 ||                                                    ||
 ||                                                    ||
 ||                                  NEC     NEC       ||
 ||                                  D431000 D43100    ||
 \-                                                    \-

* Kaneko (208 pin PQFP)    # Empty sockets for 27C4001 / 27C040
  ROM0                     ^ Empty sockets for uPD23C32000
  082                        Full NEC ram number: D431000AGW-70LL
  9709PK002


Cart Layout
-----------

SUPER-KANEKO-NOVA-SYSTEM
ROM-2-BOARD
NEP-16


 /-                                                    /-
 ||PZL-300-00 * PAL PAL  *   PZL-200-00 PZL-210-00  *  ||
 ||                                                    ||
 ||                             D431000 D431000        ||
 ||                                                    ||
 ||          PZ01U  PZ01U  #    #                      ||
 ||          U8     U10    U43  U44   *   * PZL-100-00 ||
 \-                                                    \-


*/

#include "emu.h"
#include "deprecat.h"
#include "sound/ymz280b.h"
#include "cpu/sh2/sh2.h"
#include "includes/suprnova.h"

#define BIOS_SKIP 1 // Skip Bios as it takes too long and doesn't complete atm.

UINT32 *skns_tilemapA_ram, *skns_tilemapB_ram, *skns_v3slc_ram;
UINT32 *skns_palette_ram;
UINT32 *skns_pal_regs, *skns_v3_regs, *skns_spc_regs;

static UINT32 *skns_v3t_ram, *skns_main_ram, *skns_cache_ram;

/* hit.c */

static struct {
	UINT16 x1p, y1p, z1p, x1s, y1s, z1s;
	UINT16 x2p, y2p, z2p, x2s, y2s, z2s;
	UINT16 org;

	UINT16 x1_p1, x1_p2, y1_p1, y1_p2, z1_p1, z1_p2;
	UINT16 x2_p1, x2_p2, y2_p1, y2_p2, z2_p1, z2_p2;
	UINT16 x1tox2, y1toy2, z1toz2;
	INT16 x_in, y_in, z_in;
	UINT16 flag;

	UINT8 disconnect;
} hit;

static void hit_calc_orig(UINT16 p, UINT16 s, UINT16 org, UINT16 *l, UINT16 *r)
{
	switch(org & 3) {
	case 0:
		*l = p;
		*r = p+s;
	break;
	case 1:
		*l = p-s/2;
		*r = *l+s;
	break;
	case 2:
		*l = p-s;
		*r = p;
	break;
	case 3:
		*l = p-s;
		*r = p+s;
	break;
	}
}

static void hit_calc_axis(UINT16 x1p, UINT16 x1s, UINT16 x2p, UINT16 x2s, UINT16 org,
			  UINT16 *x1_p1, UINT16 *x1_p2, UINT16 *x2_p1, UINT16 *x2_p2,
			  INT16 *x_in, UINT16 *x1tox2)
{
	UINT16 x1l=0, x1r=0, x2l=0, x2r=0;
	hit_calc_orig(x1p, x1s, org,      &x1l, &x1r);
	hit_calc_orig(x2p, x2s, org >> 8, &x2l, &x2r);

	*x1tox2 = x2p-x1p;
	*x1_p1 = x1p;
	*x2_p1 = x2p;
	*x1_p2 = x1r;
	*x2_p2 = x2l;
	*x_in = x1r-x2l;
}

static void hit_recalc(void)
{
	hit_calc_axis(hit.x1p, hit.x1s, hit.x2p, hit.x2s, hit.org,
		&hit.x1_p1, &hit.x1_p2, &hit.x2_p1, &hit.x2_p2,
		&hit.x_in, &hit.x1tox2);
	hit_calc_axis(hit.y1p, hit.y1s, hit.y2p, hit.y2s, hit.org,
		&hit.y1_p1, &hit.y1_p2, &hit.y2_p1, &hit.y2_p2,
		&hit.y_in, &hit.y1toy2);
	hit_calc_axis(hit.z1p, hit.z1s, hit.z2p, hit.z2s, hit.org,
		&hit.z1_p1, &hit.z1_p2, &hit.z2_p1, &hit.z2_p2,
		&hit.z_in, &hit.z1toz2);

	hit.flag = 0;
	hit.flag |= hit.y2p > hit.y1p ? 0x8000 : hit.y2p == hit.y1p ? 0x4000 : 0x2000;
	hit.flag |= hit.y_in >= 0 ? 0 : 0x1000;
	hit.flag |= hit.x2p > hit.x1p ? 0x0800 : hit.x2p == hit.x1p ? 0x0400 : 0x0200;
	hit.flag |= hit.x_in >= 0 ? 0 : 0x0100;
	hit.flag |= hit.z2p > hit.z1p ? 0x0080 : hit.z2p == hit.z1p ? 0x0040 : 0x0020;
	hit.flag |= hit.z_in >= 0 ? 0 : 0x0010;
	hit.flag |= hit.x_in >= 0 && hit.y_in >= 0 && hit.z_in >= 0 ? 8 : 0;
	hit.flag |= hit.z_in >= 0 && hit.x_in >= 0                  ? 4 : 0;
	hit.flag |= hit.y_in >= 0 && hit.z_in >= 0                  ? 2 : 0;
	hit.flag |= hit.x_in >= 0 && hit.y_in >= 0                  ? 1 : 0;
/*  if(0)
    log_event("HIT", "Recalc, (%d,%d)-(%d,%d)-(%d,%d):(%d,%d)-(%d,%d)-(%d,%d):%04x, (%d,%d,%d), %04x",
          hit.x1p, hit.x1s, hit.y1p, hit.y1s, hit.z1p, hit.z1s,
          hit.x2p, hit.x2s, hit.y2p, hit.y2s, hit.z2p, hit.z2s,
          hit.org,
          hit.x_in, hit.y_in, hit.z_in, hit.flag);
*/
}

static WRITE32_HANDLER ( skns_hit_w )
//void hit_w(UINT32 adr, UINT32 data, int type)
{
	int adr = offset * 4;

	switch(adr) {
	case 0x00:
	case 0x28:
		hit.x1p = data;
	break;
	case 0x08:
	case 0x30:
		hit.y1p = data;
	break;
	case 0x38:
	case 0x50:
		hit.z1p = data;
	break;
	case 0x04:
	case 0x2c:
		hit.x1s = data;
	break;
	case 0x0c:
	case 0x34:
		hit.y1s = data;
	break;
	case 0x3c:
	case 0x54:
		hit.z1s = data;
	break;
	case 0x10:
	case 0x58:
		hit.x2p = data;
	break;
	case 0x18:
	case 0x60:
		hit.y2p = data;
	break;
	case 0x20:
	case 0x68:
		hit.z2p = data;
	break;
	case 0x14:
	case 0x5c:
		hit.x2s = data;
	break;
	case 0x1c:
	case 0x64:
		hit.y2s = data;
	break;
	case 0x24:
	case 0x6c:
		hit.z2s = data;
	break;
	case 0x70:
		hit.org = data;
	break;
	default:
//      log_write("HIT", adr, data, type);
	break;
	}
	hit_recalc();
}

static WRITE32_HANDLER ( skns_hit2_w )
{
//  log_event("HIT", "Set disconnect to %02x", data);
	hit.disconnect = data;
}


static READ32_HANDLER( skns_hit_r )
//UINT32 hit_r(UINT32 adr, int type)
{
	int adr = offset *4;

//  log_read("HIT", adr, type);

	if(hit.disconnect)
		return 0x0000;
	switch(adr) {
	case 0x28:
	case 0x2a:
		return (UINT16)mame_rand(space->machine);
	case 0x00:
	case 0x10:
		return (UINT16)hit.x_in;
	case 0x04:
	case 0x14:
		return (UINT16)hit.y_in;
	case 0x18:
		return (UINT16)hit.z_in;
	case 0x08:
	case 0x1c:
		return hit.flag;
	case 0x40:
		return hit.x1p;
	case 0x48:
		return hit.y1p;
	case 0x50:
		return hit.z1p;
	case 0x44:
		return hit.x1s;
	case 0x4c:
		return hit.y1s;
	case 0x54:
		return hit.z1s;
	case 0x58:
		return hit.x2p;
	case 0x60:
		return hit.y2p;
	case 0x68:
		return hit.z2p;
	case 0x5c:
		return hit.x2s;
	case 0x64:
		return hit.y2s;
	case 0x6c:
		return hit.z2s;
	case 0x70:
		return hit.org;
	case 0x80:
		return hit.x1tox2;
	case 0x84:
		return hit.y1toy2;
	case 0x88:
		return hit.z1toz2;
	case 0x90:
		return hit.x1_p1;
	case 0xa0:
		return hit.y1_p1;
	case 0xb0:
		return hit.z1_p1;
	case 0x98:
		return hit.x1_p2;
	case 0xa8:
		return hit.y1_p2;
	case 0xb8:
		return hit.z1_p2;
	case 0x94:
		return hit.x2_p1;
	case 0xa4:
		return hit.y2_p1;
	case 0xb4:
		return hit.z2_p1;
	case 0x9c:
		return hit.x2_p2;
	case 0xac:
		return hit.y2_p2;
	case 0xbc:
		return hit.z2_p2;
	default:
//      log_read("HIT", adr, type);
	return 0;
	}
}

/* end hit.c */


/* start old driver code */


static TIMER_DEVICE_CALLBACK( interrupt_callback )
{
	cputag_set_input_line(timer.machine, "maincpu", param, HOLD_LINE);
}

static MACHINE_RESET(skns)
{
	memory_set_bankptr(machine, "bank1",memory_region(machine, "user1"));
}


static INTERRUPT_GEN(skns_interrupt)
{
	UINT8 interrupt = 5;
	switch(cpu_getiloops(device))
	{
		case 0:
			interrupt = 5; // VBLANK
			break;
		case 1:
			interrupt = 1; // SPC
			break;
	}
	cpu_set_input_line(device,interrupt,HOLD_LINE);
}

/**********************************************************************************

    Input port definitions

    NOTE: The driver reads data from eight 8bit input ports, even if they
           are unused. So I left them mapped.

**********************************************************************************/

static CUSTOM_INPUT( paddle_r )
{
	const char *tag = (const char *)param;
	return input_port_read(field->port->machine, tag);
}

static INPUT_PORTS_START( skns )		/* 3 buttons, 2 players */
	PORT_START("400000")
	PORT_BIT( 0x000000ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("400004")
	PORT_SERVICE( 0x00000001, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x00000002, 0x00000002, DEF_STR( Flip_Screen ) ) // This port affects 0x000000040191c8 function
	PORT_DIPSETTING(          0x00000002, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000004, 0x00000004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(          0x00000004, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000008, 0x00000008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(          0x00000008, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000010, 0x00000010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(          0x00000010, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000020, 0x00000020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(          0x00000020, DEF_STR( Off ) )
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x00000040, 0x00000040, "Use Backup Ram" )
	PORT_DIPSETTING(          0x00000000, DEF_STR( No ) )
	PORT_DIPSETTING(          0x00000040, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x00000080, 0x00000080, "Freeze" )
	PORT_DIPSETTING(          0x00000000, "Freezes the game")
	PORT_DIPSETTING(          0x00000080, "Right value")
	PORT_BIT( 0x0000ff00, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(paddle_r, "Paddle C")
	PORT_BIT( 0x00ff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(paddle_r, "Paddle B")
	PORT_BIT( 0xff000000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(paddle_r, "Paddle A")

	PORT_START("40000c")
	PORT_BIT( 0x000000ff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(paddle_r, "Paddle D")
	PORT_BIT( 0xffffff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Paddle A")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Paddle B")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Paddle C")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Paddle D")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( skns_1p )		/* 2 buttons, 1 player */
	PORT_INCLUDE( skns )

	PORT_MODIFY("400000")
	/* jjparads and jjparad2 are 1 player only games
       ryouran and teljan have an unemulated feature
       that allows to play them in two player mode
       via a cable-network connection (untestable)
       Service mode test shows only P1 inputs */
	PORT_BIT( 0x00ff0000, IP_ACTIVE_LOW, IPT_UNUSED )
	/* same as above, coin 2 and start 2 are untestable
       in ryouran and teljan. So I left disabled for now */
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_UNUSED )	/* Start 2 */
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_UNUSED )	/* Coin 2 */
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_UNUSED )	/* No Button 3 */

	PORT_MODIFY("400004")
	PORT_DIPNAME( 0x00000010, 0x00000010, "Test Mode" )
	PORT_DIPSETTING(          0x00000010, DEF_STR(Off) )
	PORT_DIPSETTING(          0x00000000, DEF_STR(On) )
INPUT_PORTS_END

static INPUT_PORTS_START( cyvern )		/* 2 buttons, 2 players */
	PORT_INCLUDE( skns )

	PORT_MODIFY("400000")
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_UNUSED )	/* No Button 3 */
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_UNUSED )	/* No Button 3 */
INPUT_PORTS_END

static INPUT_PORTS_START( galpanis )	/* 1 button, 2 players */
	PORT_INCLUDE( skns )

	PORT_MODIFY("400000")
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_UNUSED )	/* No Button 2 */
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_UNUSED )	/* No Button 3 */
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_UNUSED )	/* No Button 2 */
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_UNUSED )	/* No Button 3 */
INPUT_PORTS_END

static INPUT_PORTS_START( puzzloop )	/* 2 buttons, 2 players, paddle */
	PORT_INCLUDE( skns )

	PORT_MODIFY("400000")
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_UNUSED )	/* No Button 3 */
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_UNUSED )	/* No Button 3 */

	PORT_MODIFY("Paddle A")  /* Paddle A */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(15) PORT_REVERSE PORT_PLAYER(1)

	PORT_MODIFY("Paddle B")  /* Paddle B */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(15) PORT_REVERSE PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( vblokbrk )	/* 3 buttons, 2 players, paddle */
	PORT_INCLUDE( skns )

	PORT_MODIFY("Paddle A")  /* Paddle A */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(15) PORT_REVERSE PORT_PLAYER(1)

	PORT_MODIFY("Paddle B")  /* Paddle B */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(15) PORT_REVERSE PORT_PLAYER(2)
INPUT_PORTS_END


static UINT32 timer_0_temp[4];

static WRITE32_HANDLER( skns_msm6242_w )
{
	COMBINE_DATA(&timer_0_temp[offset]);

	if(offset>=4)
	{
		mame_printf_debug("Timer 0 outbound\n");
		return;
	}
}

static WRITE32_HANDLER( skns_io_w )
{
	switch(offset) {
	case 2:
		if(ACCESSING_BITS_24_31)
		{ /* Coin Lock/Count */
//          coin_counter_w(space->machine, 0, data & 0x01000000);
//          coin_counter_w(space->machine, 1, data & 0x02000000);
//          coin_lockout_w(space->machine, 0, ~data & 0x04000000);
//          coin_lockout_w(space->machine, 1, ~data & 0x08000000); // Works in puzzloop, others behave strange.
		}
		if(ACCESSING_BITS_16_23)
		{ /* Analogue Input Select */
		}
		if(ACCESSING_BITS_8_15)
		{ /* Extended Output - Port A, Mahjong inputs, Comms etc. */
		}
		if(ACCESSING_BITS_0_7)
		{ /* Extended Output - Port B */
		}
	break;
	case 3:
		if(ACCESSING_BITS_8_15)
		{ /* Interrupt Clear, do we need these? */
/*          if(data&0x01)
                cputag_set_input_line(space->machine, "maincpu",1,CLEAR_LINE);
            if(data&0x02)
                cputag_set_input_line(space->machine, "maincpu",3,CLEAR_LINE);
            if(data&0x04)
                cputag_set_input_line(space->machine, "maincpu",5,CLEAR_LINE);
            if(data&0x08)
                cputag_set_input_line(space->machine, "maincpu",7,CLEAR_LINE);
            if(data&0x10)
                cputag_set_input_line(space->machine, "maincpu",9,CLEAR_LINE);
            if(data&0x20)
                cputag_set_input_line(space->machine, "maincpu",0xb,CLEAR_LINE);
            if(data&0x40)
                cputag_set_input_line(space->machine, "maincpu",0xd,CLEAR_LINE);
            if(data&0x80)
                cputag_set_input_line(space->machine, "maincpu",0xf,CLEAR_LINE);*/

			/* idle skip for vblokbrk/sarukani, i can't find a better place to put it :-( but i think it works ok unless its making the game too fast */
			if (cpu_get_pc(space->cpu)==0x04013B42)
			{
				if (!strcmp(space->machine->gamedrv->name,"vblokbrk") ||
					!strcmp(space->machine->gamedrv->name,"sarukani"))
					cpu_spinuntil_int(space->cpu);
			}

		}
		else
		{
			logerror("Unk IO Write memmask:%08x offset:%08x data:%08x\n", mem_mask, offset, data);
		}
	break;
	default:
		logerror("Unk IO Write memmask:%08x offset:%08x data:%08x\n", mem_mask, offset, data);
	break;
	}
}


static READ32_HANDLER( skns_msm6242_r )
{
	system_time systime;
	long value;

	space->machine->base_datetime(systime);
	// The clock is not y2k-compatible, wrap back 10 years, screw the leap years
	//  tm->tm_year -= 10;

	switch(offset) {
	case 0:
		value  = (systime.local_time.second % 10)<<24;
		value |= (systime.local_time.second / 10)<<16;
		value |= (systime.local_time.minute % 10)<<8;
		value |= (systime.local_time.minute / 10);
		break;
	case 1:
		value  = (systime.local_time.hour % 10)<<24;
		value |= ((systime.local_time.hour / 10) /*| (tm->tm_hour >= 12 ? 4 : 0)*/)<<16;
		value |= (systime.local_time.mday % 10)<<8;
		value |= (systime.local_time.mday / 10);
		break;
	case 2:
		value  = ((systime.local_time.month + 1) % 10)<<24;
		value |= ((systime.local_time.month + 1) / 10)<<16;
		value |= (systime.local_time.year % 10)<<8;
		value |= ((systime.local_time.year / 10) % 10);
		break;
	case 3:
	default:
		value  = (systime.local_time.weekday)<<24;
		value |= (1)<<16;
		value |= (6)<<8;
		value |= (4);
		break;
	}
	return value;
}

/* end old driver code */

static WRITE32_HANDLER( skns_v3t_w )
{
	UINT8 *btiles = memory_region(space->machine, "gfx3");

	COMBINE_DATA(&skns_v3t_ram[offset]);

	gfx_element_mark_dirty(space->machine->gfx[1], offset/0x40);
	gfx_element_mark_dirty(space->machine->gfx[3], offset/0x20);

	data = skns_v3t_ram[offset];
// i think we need to swap around to decode .. endian issues?
	btiles[offset*4+0] = (data & 0xff000000) >> 24;
	btiles[offset*4+1] = (data & 0x00ff0000) >> 16;
	btiles[offset*4+2] = (data & 0x0000ff00) >> 8;
	btiles[offset*4+3] = (data & 0x000000ff) >> 0;
}

static ADDRESS_MAP_START( skns_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x0007ffff) AM_ROM /* BIOS ROM */
	AM_RANGE(0x00400000, 0x0040000f) AM_WRITE(skns_io_w) /* I/O Write */
	AM_RANGE(0x00400000, 0x00400003) AM_READ_PORT("400000")
	AM_RANGE(0x00400004, 0x00400007) AM_READ_PORT("400004")
	/* In between is write only */
	AM_RANGE(0x0040000c, 0x0040000f) AM_READ_PORT("40000c")
	AM_RANGE(0x00800000, 0x00801fff) AM_RAM AM_BASE_SIZE_GENERIC(nvram) /* 'backup' RAM */
	AM_RANGE(0x00c00000, 0x00c00003) AM_DEVREADWRITE8("ymz", ymz280b_r, ymz280b_w, 0xffff0000) /* ymz280_w (sound) */
	AM_RANGE(0x01000000, 0x0100000f) AM_READWRITE(skns_msm6242_r, skns_msm6242_w)
	AM_RANGE(0x01800000, 0x01800003) AM_WRITE(skns_hit2_w)
	AM_RANGE(0x02000000, 0x02003fff) AM_RAM AM_BASE_SIZE_GENERIC(spriteram) /* sprite ram */
	AM_RANGE(0x02100000, 0x0210003f) AM_RAM AM_BASE(&skns_spc_regs) /* sprite registers */
	AM_RANGE(0x02400000, 0x0240007f) AM_RAM_WRITE(skns_v3_regs_w) AM_BASE(&skns_v3_regs) /* tilemap registers */
	AM_RANGE(0x02500000, 0x02503fff) AM_RAM_WRITE(skns_tilemapA_w) AM_BASE(&skns_tilemapA_ram) /* tilemap A */
	AM_RANGE(0x02504000, 0x02507fff) AM_RAM_WRITE(skns_tilemapB_w) AM_BASE(&skns_tilemapB_ram) /* tilemap B */
	AM_RANGE(0x02600000, 0x02607fff) AM_RAM AM_BASE(&skns_v3slc_ram) /* tilemap linescroll */
	AM_RANGE(0x02a00000, 0x02a0001f) AM_RAM_WRITE(skns_pal_regs_w) AM_BASE(&skns_pal_regs)
	AM_RANGE(0x02a40000, 0x02a5ffff) AM_RAM_WRITE(skns_palette_ram_w) AM_BASE(&skns_palette_ram)
	AM_RANGE(0x02f00000, 0x02f000ff) AM_READWRITE(skns_hit_r, skns_hit_w)
	AM_RANGE(0x04000000, 0x041fffff) AM_ROMBANK("bank1") /* GAME ROM */
	AM_RANGE(0x04800000, 0x0483ffff) AM_RAM_WRITE(skns_v3t_w) AM_BASE(&skns_v3t_ram) /* tilemap b ram based tiles */
	AM_RANGE(0x06000000, 0x060fffff) AM_RAM AM_BASE(&skns_main_ram)
	AM_RANGE(0xc0000000, 0xc0000fff) AM_RAM AM_BASE(&skns_cache_ram) /* 'cache' RAM */
ADDRESS_MAP_END

/***** GFX DECODE *****/

static const gfx_layout skns_tilemap_layout =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
	 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	{ 0*128, 1*128, 2*128, 3*128, 4*128, 5*128, 6*128, 7*128,
	  8*128, 9*128, 10*128, 11*128, 12*128, 13*128, 14*128, 15*128 },
	16*16*8
};

static const gfx_layout skns_4bpptilemap_layout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3  },
	{ 1*4, 0*4, 3*4, 2*4, 5*4, 4*4, 7*4, 6*4,
	 9*4, 8*4, 11*4, 10*4, 13*4, 12*4, 15*4, 14*4 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
	  8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	16*16*4
};

static GFXDECODE_START( skns_bg )
   /* "gfx1" is sprites, RLE encoded */
	GFXDECODE_ENTRY( "gfx2", 0, skns_tilemap_layout, 0x000, 128 )
	GFXDECODE_ENTRY( "gfx3", 0, skns_tilemap_layout, 0x000, 128 )
	GFXDECODE_ENTRY( "gfx2", 0, skns_4bpptilemap_layout, 0x000, 128 )
	GFXDECODE_ENTRY( "gfx3", 0, skns_4bpptilemap_layout, 0x000, 128 )
GFXDECODE_END

/***** MACHINE DRIVER *****/

static const ymz280b_interface ymz280b_intf =
{
	0	// irq ?
};

static MACHINE_DRIVER_START(skns)
	MDRV_CPU_ADD("maincpu", SH2,28638000)
	MDRV_CPU_PROGRAM_MAP(skns_map)
	MDRV_CPU_VBLANK_INT_HACK(skns_interrupt,2)

	MDRV_MACHINE_RESET(skns)
	MDRV_NVRAM_HANDLER(generic_1fill)

	MDRV_TIMER_ADD_PERIODIC("int15_timer", interrupt_callback, MSEC(2))
	MDRV_TIMER_PARAM(15)
	MDRV_TIMER_ADD_PERIODIC("int11_timer", interrupt_callback, MSEC(8))
	MDRV_TIMER_PARAM(11)
	MDRV_TIMER_ADD_PERIODIC("int9_timer", interrupt_callback, HZ(28638000/1824))
	MDRV_TIMER_PARAM(9)

	MDRV_VIDEO_ATTRIBUTES(VIDEO_ALWAYS_UPDATE)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(59.5971) // measured by Guru
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(320,240)
	MDRV_SCREEN_VISIBLE_AREA(0,319,0,239)

	MDRV_PALETTE_LENGTH(32768)
	MDRV_GFXDECODE(skns_bg)

	MDRV_VIDEO_START(skns)
	MDRV_VIDEO_RESET(skns)
	MDRV_VIDEO_EOF(skns)
	MDRV_VIDEO_UPDATE(skns)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymz", YMZ280B, 33333333 / 2)
	MDRV_SOUND_CONFIG(ymz280b_intf)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END

/***** BIOS SKIPPING *****/

static READ32_HANDLER( bios_skip_r )
{
#if BIOS_SKIP
	if ((cpu_get_pc(space->cpu)==0x6f8) || (cpu_get_pc(space->cpu)==0x6fa)) memory_write_byte(space, 0x06000029,1);
#endif
	return skns_main_ram[0x00028/4];
}

/***** IDLE SKIPPING *****/

static READ32_HANDLER( gutsn_speedup_r )
{
/*
    0402206A: MOV.L   @($8C,PC),R5
    0402206C: MOV.L   @($8C,PC),R1
    0402206E: MOV.L   @R1,R2       // R1 == 600C780
    04022070: MOV.L   @R5,R3       // R5 == 6000078
    04022072: CMP/EQ  R2,R3
    04022074: BT      $0402206C
*/
	if (cpu_get_pc(space->cpu)==0x402206e)
	{
		if(skns_main_ram[0x00078/4] == skns_main_ram[0x0c780/4])
			cpu_spinuntil_int(space->cpu);
	}
	return skns_main_ram[0x0c780/4];
}

static READ32_HANDLER( cyvern_speedup_r )
{
	if (cpu_get_pc(space->cpu)==0x402ebd2) cpu_spinuntil_int(space->cpu);
	return skns_main_ram[0x4d3c8/4];
}

static READ32_HANDLER( puzzloopj_speedup_r )
{
	if (cpu_get_pc(space->cpu)==0x401dca0) cpu_spinuntil_int(space->cpu);
	return skns_main_ram[0x86714/4];
}

static READ32_HANDLER( puzzloopa_speedup_r )
{
	if (cpu_get_pc(space->cpu)==0x401d9d4) cpu_spinuntil_int(space->cpu);
	return skns_main_ram[0x85bcc/4];
}

static READ32_HANDLER( puzzloopu_speedup_r )
{
	if (cpu_get_pc(space->cpu)==0x401dab0) cpu_spinuntil_int(space->cpu);
	return skns_main_ram[0x85cec/4];
}

static READ32_HANDLER( puzzloop_speedup_r )
{
/*
    0401DA12: MOV.L   @($80,PC),R1
    0401DA14: MOV.L   @R1,R0 (R1=0x6081d38)
    0401DA16: TST     R0,R0
    0401DA18: BF      $0401DA26
    0401DA26: BRA     $0401DA12
*/
	if (cpu_get_pc(space->cpu)==0x401da14) cpu_spinuntil_int(space->cpu);
	return skns_main_ram[0x81d38/4];
}

static READ32_HANDLER( senknow_speedup_r )
{
	if (cpu_get_pc(space->cpu)==0x4017dce) cpu_spinuntil_int(space->cpu);
	return skns_main_ram[0x0000dc/4];
}

static READ32_HANDLER( teljan_speedup_r )
{
	if (cpu_get_pc(space->cpu)==0x401ba32) cpu_spinuntil_int(space->cpu);
	return skns_main_ram[0x002fb4/4];
}

static READ32_HANDLER( jjparads_speedup_r )
{
	if (cpu_get_pc(space->cpu)==0x4015e84) cpu_spinuntil_int(space->cpu);
	return skns_main_ram[0x000994/4];
}

static READ32_HANDLER( jjparad2_speedup_r )
{
	if (cpu_get_pc(space->cpu)==0x401620a) cpu_spinuntil_int(space->cpu);
	return skns_main_ram[0x000984/4];
}

static READ32_HANDLER( ryouran_speedup_r )
{
	if (cpu_get_pc(space->cpu)==0x40182ce) cpu_spinuntil_int(space->cpu);
	return skns_main_ram[0x000a14/4];
}

static READ32_HANDLER( galpans2_speedup_r )
{
	if (cpu_get_pc(space->cpu)==0x4049ae2) cpu_spinuntil_int(space->cpu);
	return skns_main_ram[0x0fb6bc/4];
}

static READ32_HANDLER( panicstr_speedup_r )
{
	if (cpu_get_pc(space->cpu)==0x404e68a) cpu_spinuntil_int(space->cpu);
	return skns_main_ram[0x0f19e4/4];
}

static READ32_HANDLER( sengekis_speedup_r ) // 60006ee  600308e
{
	if (cpu_get_pc(space->cpu)==0x60006ec) cpu_spinuntil_int(space->cpu);
	return skns_main_ram[0xb74bc/4];
}

static READ32_HANDLER( sengekij_speedup_r ) // 60006ee  600308e
{
	if (cpu_get_pc(space->cpu)==0x60006ec) cpu_spinuntil_int(space->cpu);
	return skns_main_ram[0xb7380/4];
}

static void init_skns(running_machine *machine)
{
	// init DRC to fastest options
	sh2drc_set_options(machine->device("maincpu"), SH2DRC_FASTEST_OPTIONS);

	sh2drc_add_pcflush(machine->device("maincpu"), 0x6f8);
	memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x6000028, 0x600002b, 0, 0, bios_skip_r );
}

static void set_drc_pcflush(running_machine *machine, UINT32 addr)
{
	sh2drc_add_pcflush(machine->device("maincpu"), addr);
}

static DRIVER_INIT( galpani4 ) { skns_sprite_kludge(-5,-1); init_skns(machine);  }
static DRIVER_INIT( galpanis ) { skns_sprite_kludge(-5,-1); init_skns(machine);  }
static DRIVER_INIT( cyvern )   { skns_sprite_kludge(+0,+2); init_skns(machine); memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x604d3c8, 0x604d3cb, 0, 0, cyvern_speedup_r );  set_drc_pcflush(machine, 0x402ebd2);  }
static DRIVER_INIT( galpans2 ) { skns_sprite_kludge(-1,-1); init_skns(machine); memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x60fb6bc, 0x60fb6bf, 0, 0, galpans2_speedup_r ); set_drc_pcflush(machine, 0x4049ae2); }
static DRIVER_INIT( gutsn )    { skns_sprite_kludge(+0,+0); init_skns(machine); memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x600c780, 0x600c783, 0, 0, gutsn_speedup_r ); set_drc_pcflush(machine, 0x402206e); }
static DRIVER_INIT( panicstr ) { skns_sprite_kludge(-1,-1); init_skns(machine); memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x60f19e4, 0x60f19e7, 0, 0, panicstr_speedup_r ); set_drc_pcflush(machine, 0x404e68a);  }
static DRIVER_INIT( senknow )  { skns_sprite_kludge(+1,+1); init_skns(machine); memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x60000dc, 0x60000df, 0, 0, senknow_speedup_r ); set_drc_pcflush(machine, 0x4017dce);  }
static DRIVER_INIT( puzzloop ) { skns_sprite_kludge(-9,-1); init_skns(machine); memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x6081d38, 0x6081d3b, 0, 0, puzzloop_speedup_r ); set_drc_pcflush(machine, 0x401da14); }
static DRIVER_INIT( puzzloopj ) { skns_sprite_kludge(-9,-1); init_skns(machine); memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x6086714, 0x6086717, 0, 0, puzzloopj_speedup_r ); set_drc_pcflush(machine, 0x401dca0); }
static DRIVER_INIT( puzzloopa ) { skns_sprite_kludge(-9,-1); init_skns(machine); memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x6085bcc, 0x6085bcf, 0, 0, puzzloopa_speedup_r ); set_drc_pcflush(machine, 0x401d9d4); }
static DRIVER_INIT( puzzloopu ) { skns_sprite_kludge(-9,-1); init_skns(machine); memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x6085cec, 0x6085cef, 0, 0, puzzloopu_speedup_r ); set_drc_pcflush(machine, 0x401dab0); }
static DRIVER_INIT( jjparads ) { skns_sprite_kludge(+5,+1); init_skns(machine); memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x6000994, 0x6000997, 0, 0, jjparads_speedup_r ); set_drc_pcflush(machine, 0x4015e84); }
static DRIVER_INIT( jjparad2 ) { skns_sprite_kludge(+5,+1); init_skns(machine); memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x6000984, 0x6000987, 0, 0, jjparad2_speedup_r ); set_drc_pcflush(machine, 0x401620a); }
static DRIVER_INIT( ryouran )  { skns_sprite_kludge(+5,+1); init_skns(machine); memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x6000a14, 0x6000a17, 0, 0, ryouran_speedup_r );  set_drc_pcflush(machine, 0x40182ce); }
static DRIVER_INIT( teljan )   { skns_sprite_kludge(+5,+1); init_skns(machine); memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x6002fb4, 0x6002fb7, 0, 0, teljan_speedup_r ); set_drc_pcflush(machine, 0x401ba32); }
static DRIVER_INIT( sengekis ) { skns_sprite_kludge(-192,-272); init_skns(machine); memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x60b74bc, 0x60b74bf, 0, 0, sengekis_speedup_r ); set_drc_pcflush(machine, 0x60006ec); }
static DRIVER_INIT( sengekij ) { skns_sprite_kludge(-192,-272); init_skns(machine); memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x60b7380, 0x60b7383, 0, 0, sengekij_speedup_r ); set_drc_pcflush(machine, 0x60006ec); }
static DRIVER_INIT( sarukani ) { skns_sprite_kludge(-1,-1); init_skns(machine); set_drc_pcflush(machine, 0x4013b42); } // Speedup is in skns_io_w()
static DRIVER_INIT( galpans3 ) { skns_sprite_kludge(-1,-1); init_skns(machine);  }



/***** ROM LOADING *****/

ROM_START( skns )
	ROM_REGION( 0x0100000, "maincpu", 0 ) /* SH-2 Code */
	ROM_LOAD       ( "sknsj1.u10", 0x000000, 0x080000, CRC(7e2b836c) SHA1(92c5a7a2472496028bff0e5980d41dd294f42144) ) /* Japan BIOS */
	ROM_LOAD       ( "sknse1.u10", 0x000000, 0x080000, CRC(e2b9d7d1) SHA1(b530a3bb9dedc8cfafcba9f1f10277590be04a15) ) /* Europ BIOS */
	ROM_LOAD       ( "sknsa1.u10", 0x000000, 0x080000, CRC(745e5212) SHA1(caba649ab2d83b2d7e007eecee0fc582c019df38) ) /* Asia  BIOS */
	ROM_LOAD       ( "sknsu1.u10", 0x000000, 0x080000, CRC(384d21ec) SHA1(a27e8a18099d9cea64fa32db28d01101c2a78815) ) /* USA   BIOS */
	ROM_LOAD       ( "sknsk1.u10", 0x000000, 0x080000, CRC(ff1c9f79) SHA1(a51e598d43e76d37da69b1f094c111273bdfc94a) ) /* Korea BIOS */

	ROM_REGION32_BE( 0x200000, "user1", ROMREGION_ERASE00 ) /* SH-2 Code mapped at 0x04000000 */

	ROM_REGION( 0x800000, "gfx1", ROMREGION_ERASE00 ) /* Sprites */

	ROM_REGION( 0x800000, "gfx2", ROMREGION_ERASE00 ) /* Tiles Plane A */

	ROM_REGION( 0x800000, "gfx3", ROMREGION_ERASE00 ) /* Tiles Plane B */

	ROM_REGION( 0x400000, "ymz", ROMREGION_ERASE00 ) /* Samples */
ROM_END

ROM_START( cyvern )
	ROM_REGION( 0x080000, "maincpu", 0 ) /* SH-2 Code */
	ROM_LOAD       ( "sknsj1.u10",   0x000000, 0x080000, CRC(7e2b836c) SHA1(92c5a7a2472496028bff0e5980d41dd294f42144) ) /* Japan BIOS */

	ROM_REGION32_BE( 0x200000, "user1", 0 ) /* SH-2 Code mapped at 0x04000000 */
	ROM_LOAD16_BYTE( "cvj-even.u10", 0x000000, 0x100000, CRC(802fadb4) SHA1(cbfac3a87a4863466117c61f2ecaf63d506352f6) )
	ROM_LOAD16_BYTE( "cvj-odd.u8",   0x000001, 0x100000, CRC(f8a0fbdd) SHA1(5cc8c12c13b5eb3456083e70100450ba041de76e) )

	ROM_REGION( 0x800000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD( "cv100-00.u24", 0x000000, 0x400000, CRC(cd4ae88a) SHA1(925f4ae01a6ad3633be2a61be69e163f05401cf6) )
	ROM_LOAD( "cv101-00.u20", 0x400000, 0x400000, CRC(a6cb3f0b) SHA1(8d83f44a096ca0a70962ca4c602c4331874c8560) )

	ROM_REGION( 0x800000, "gfx2", 0 ) /* Tiles Plane A */
	ROM_LOAD( "cv200-00.u16", 0x000000, 0x400000, CRC(ddc8c67e) SHA1(9b99e87e69e88011e6d693d19ac5e115b4fa50b0) )
	ROM_LOAD( "cv201-00.u13", 0x400000, 0x400000, CRC(65863321) SHA1(b8b75f50406068ffc3fca3887d2f0a653ca491c9) )

	ROM_REGION( 0x800000, "gfx3", ROMREGION_ERASE00 ) /* Tiles Plane B */
	/* First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles */
	/* 0x040000 - 0x3fffff empty? */
	ROM_LOAD( "cv210-00.u18", 0x400000, 0x400000, CRC(7486bf3a) SHA1(3b4285ca570e9c5ad396c615bfc054372d1b0162) )

	ROM_REGION( 0x400000, "ymz", 0 ) /* Samples */
	ROM_LOAD( "cv300-00.u4", 0x000000, 0x400000, CRC(fbeda465) SHA1(4d5066a22f4589b6b7f85b3e77c348d900ac4bdd) )
ROM_END

ROM_START( galpani4 )
	ROM_REGION( 0x080000, "maincpu", 0 ) /* SH-2 Code */
	ROM_LOAD       ( "sknsj1.u10",   0x000000, 0x080000, CRC(7e2b836c) SHA1(92c5a7a2472496028bff0e5980d41dd294f42144) ) /* Japan BIOS */

	ROM_REGION32_BE( 0x200000, "user1", 0 ) /* SH-2 Code mapped at 0x04000000 */
	ROM_LOAD16_BYTE( "gp4j1.u10", 0x000000, 0x080000, CRC(919a3893) SHA1(83b89a9e628a1f46f8a56ea512fc8ad641d5e239) )
	ROM_LOAD16_BYTE( "gp4j1.u8",  0x000001, 0x080000, CRC(94cb1fb7) SHA1(ac90103dd43cdce6a287ffc13631c1de477a9a71) )

	ROM_REGION( 0x400000, "gfx1", 0 )
	ROM_LOAD( "gp4-100-00.u24", 0x000000, 0x200000, CRC(1df61f01) SHA1(a9e95bbb3013e8f2fd01243b1b392ff07b4f7d02) )
	ROM_LOAD( "gp4-101-00.u20", 0x200000, 0x100000, CRC(8e2c9349) SHA1(a58fa9bcc9684ed4558e3395d592b64a1978a902) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD( "gp4-200-00.u16", 0x000000, 0x200000, CRC(f0781376) SHA1(aeab9553a9af922524e528eb2d019cf36b6e2094) )
	ROM_LOAD( "gp4-201-00.u18", 0x200000, 0x200000, CRC(10c4b183) SHA1(80e05f3932495ad4fc9bf928fa66e6d2931bbb06) )

	ROM_REGION( 0x800000, "gfx3", ROMREGION_ERASE00 ) /* Tiles Plane B */
	/* First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles */
	/* 0x040000 - 0x3fffff empty? */

	ROM_REGION( 0x400000, "ymz", 0 ) /* Samples */
	ROM_LOAD( "gp4-300-00.u4", 0x000000, 0x200000, CRC(8374663a) SHA1(095512564f4de25dc3752d9fbd254b9dabd16d1b) )
ROM_END

ROM_START( galpani4k ) /* ROM-BOARD NEP-16 part number GP04K00372 with extra sound sample rom at U7 */
	ROM_REGION( 0x080000, "maincpu", 0 ) /* SH-2 Code */
	ROM_LOAD       ( "sknsk1.u10",   0x000000, 0x080000, CRC(ff1c9f79) SHA1(a51e598d43e76d37da69b1f094c111273bdfc94a) ) /* Korean BIOS */

	ROM_REGION32_BE( 0x200000, "user1", 0 ) /* SH-2 Code mapped at 0x04000000 */
	ROM_LOAD16_BYTE( "gp4k1.u10", 0x000000, 0x080000, CRC(cbd5c3a0) SHA1(17fc0d6f6050ffd31707cee3fcc263cd5b9d0c4f) )
	ROM_LOAD16_BYTE( "gp4k1.u8",  0x000001, 0x080000, CRC(7a95bfe2) SHA1(82e24fd4674ec25bc6608ced0921e8573fcff2c2) )

	ROM_REGION( 0x400000, "gfx1", 0 )
	ROM_LOAD( "gp4-100-00.u24", 0x000000, 0x200000, CRC(1df61f01) SHA1(a9e95bbb3013e8f2fd01243b1b392ff07b4f7d02) )
	ROM_LOAD( "gp4-101-00.u20", 0x200000, 0x100000, CRC(8e2c9349) SHA1(a58fa9bcc9684ed4558e3395d592b64a1978a902) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD( "gp4-200-00.u16", 0x000000, 0x200000, CRC(f0781376) SHA1(aeab9553a9af922524e528eb2d019cf36b6e2094) )
	ROM_LOAD( "gp4-201-00.u18", 0x200000, 0x200000, CRC(10c4b183) SHA1(80e05f3932495ad4fc9bf928fa66e6d2931bbb06) )

	ROM_REGION( 0x800000, "gfx3", ROMREGION_ERASE00 ) /* Tiles Plane B */
	/* First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles */
	/* 0x040000 - 0x3fffff empty? */

	ROM_REGION( 0x400000, "ymz", 0 ) /* Samples */
	ROM_LOAD( "gp4-300-00.u4", 0x000000, 0x200000, CRC(8374663a) SHA1(095512564f4de25dc3752d9fbd254b9dabd16d1b) ) /* Doesn't seem to use these samples at all */
	ROM_LOAD( "gp4-301-01.u7", 0x200000, 0x200000, CRC(886ef77f) SHA1(047d5fecf2034339c69b2cb605b623a814a18f0d) ) /* Changed some samples when compared to U4 rom  */
ROM_END

ROM_START( galpanis )
	ROM_REGION( 0x080000, "maincpu", 0 ) /* SH-2 Code */
	ROM_LOAD       ( "sknsj1.u10",   0x000000, 0x080000, CRC(7e2b836c) SHA1(92c5a7a2472496028bff0e5980d41dd294f42144) ) /* Japan BIOS */

	ROM_REGION32_BE( 0x200000, "user1", 0 ) /* SH-2 Code mapped at 0x04000000 */
	ROM_LOAD16_BYTE( "gps-e.u10", 0x000000, 0x100000, CRC(c6938c3f) SHA1(05853ee6a44a55702788a75580b04a4be45e9bcb) )
	ROM_LOAD16_BYTE( "gps-o.u8",  0x000001, 0x100000, CRC(e764177a) SHA1(3a1333eb1022ed1a275b9c3d44b5f4ab81618fb6) )

	ROM_REGION( 0x1000000, "gfx1", 0 )
	ROM_LOAD( "gps10000.u24", 0x000000, 0x400000, CRC(a1a7acf2) SHA1(52c86ae907f0c0236808c19f652955b09e90ec5a) )
	ROM_LOAD( "gps10100.u20", 0x400000, 0x400000, CRC(49f764b6) SHA1(9f4289858c3dac625ef623cc381a47b45aa5d8e2) )
	ROM_LOAD( "gps10200.u17", 0x800000, 0x400000, CRC(51980272) SHA1(6c0706d913b33995579aaf0688c4bf26d6d35a78) )

	ROM_REGION( 0x800000, "gfx2", 0 )
	ROM_LOAD( "gps20000.u16", 0x000000, 0x400000, CRC(c146a09e) SHA1(5af5a7b9d9a55ec7aba3fd85a3a0211b92b1b84f) )
	ROM_LOAD( "gps20100.u13", 0x400000, 0x400000, CRC(9dfa2dc6) SHA1(a058c42fd76c23c0e5c8c11f5617fd29e056be7d) )

	ROM_REGION( 0x800000, "gfx3", ROMREGION_ERASE00 ) /* Tiles Plane B */
	/* First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles */
	/* 0x040000 - 0x3fffff empty? */

	ROM_REGION( 0x400000, "ymz", 0 ) /* Samples */
	ROM_LOAD( "gps30000.u4", 0x000000, 0x400000, CRC(9e4da8e3) SHA1(6506d9300a442883357003a05fd2c78d364c35bb) )
ROM_END

ROM_START( galpanisk )
	ROM_REGION( 0x080000, "maincpu", 0 ) /* SH-2 Code */
	ROM_LOAD       ( "sknsk1.u10",   0x000000, 0x080000, CRC(ff1c9f79) SHA1(a51e598d43e76d37da69b1f094c111273bdfc94a) ) /* Korean BIOS */

	ROM_REGION32_BE( 0x200000, "user1", 0 ) /* SH-2 Code mapped at 0x04000000 */
	ROM_LOAD16_BYTE( "gps-000-k1.u10", 0x000000, 0x100000, CRC(c9ff3d8a) SHA1(edfec265654aaa8cb307424e5b2899e708392cd0) )
	ROM_LOAD16_BYTE( "gps-001-k1.u8",  0x000001, 0x100000, CRC(354e601d) SHA1(4d176f2337a3b0b63548b2e542f9fa87d0a1ef7b) )

	ROM_REGION( 0x1000000, "gfx1", 0 )
	ROM_LOAD( "gps10000.u24", 0x000000, 0x400000, CRC(a1a7acf2) SHA1(52c86ae907f0c0236808c19f652955b09e90ec5a) )
	ROM_LOAD( "gps10100.u20", 0x400000, 0x400000, CRC(49f764b6) SHA1(9f4289858c3dac625ef623cc381a47b45aa5d8e2) )
	ROM_LOAD( "gps10200.u17", 0x800000, 0x400000, CRC(51980272) SHA1(6c0706d913b33995579aaf0688c4bf26d6d35a78) )

	ROM_REGION( 0x800000, "gfx2", 0 )
	ROM_LOAD( "gps20000.u16", 0x000000, 0x400000, CRC(c146a09e) SHA1(5af5a7b9d9a55ec7aba3fd85a3a0211b92b1b84f) )
	ROM_LOAD( "gps20100.u13", 0x400000, 0x400000, CRC(9dfa2dc6) SHA1(a058c42fd76c23c0e5c8c11f5617fd29e056be7d) )

	ROM_REGION( 0x800000, "gfx3", ROMREGION_ERASE00 ) /* Tiles Plane B */
	/* First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles */
	/* 0x040000 - 0x3fffff empty? */

	ROM_REGION( 0x400000, "ymz", 0 ) /* Samples */
	ROM_LOAD( "gps30000.u4", 0x000000, 0x400000, CRC(9e4da8e3) SHA1(6506d9300a442883357003a05fd2c78d364c35bb) )
ROM_END

ROM_START( galpans2 )
	ROM_REGION( 0x080000, "maincpu", 0 ) /* SH-2 Code */
	ROM_LOAD       ( "sknsj1.u10",   0x000000, 0x080000, CRC(7e2b836c) SHA1(92c5a7a2472496028bff0e5980d41dd294f42144) ) /* Japan BIOS */

	ROM_REGION32_BE( 0x200000, "user1", 0 ) /* SH-2 Code mapped at 0x04000000 */
	ROM_LOAD16_BYTE( "gps2j.u6", 0x000000, 0x100000, CRC(6e74005b) SHA1(a57e8307062e262c2e7a84e2c58f7dfe03fc0f78) )
	ROM_LOAD16_BYTE( "gps2j.u4", 0x000001, 0x100000, CRC(9b4b2304) SHA1(0b481f4d71d92bf23f38ed22979efd4409004857) )

	ROM_REGION( 0x1000000, "gfx1", 0 )
	ROM_LOAD( "gs210000.u21", 0x000000, 0x400000, CRC(294b2f14) SHA1(90cbd0acdaa2d89d208c28aae33ab57c03624089) )
	ROM_LOAD( "gs210100.u20", 0x400000, 0x400000, CRC(f75c5a9a) SHA1(3919643cee6c88185a1aa3c58c5bc80599bf734e) )
	ROM_LOAD( "gs210200.u8",  0x800000, 0x400000, CRC(25b4f56b) SHA1(f9a33d5ed54a04ecece3035e75508d191bbe74b1) )
	ROM_LOAD( "gs210300.u32", 0xc00000, 0x400000, CRC(db6d4424) SHA1(0a88dafd0ee2490ff2ef39ce8eb1931c41bdda42) )

	ROM_REGION( 0x800000, "gfx2", 0 )
	ROM_LOAD( "gs220000.u17", 0x000000, 0x400000, CRC(5caae1c0) SHA1(8f77e4cf018d7290b2d804cbff9fccf0bf4d2404) )
	ROM_LOAD( "gs220100.u9",  0x400000, 0x400000, CRC(8d51f197) SHA1(19d2afab823ea179918e7bcbf4df2283e77570f0) )

	ROM_REGION( 0x800000, "gfx3", ROMREGION_ERASE00 ) /* Tiles Plane B */
	/* First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles */
	/* 0x040000 - 0x3fffff empty? */
	ROM_LOAD( "gs221000.u3",  0x400000, 0x400000, CRC(58800a18) SHA1(5e6d55ecd12275662d6f59559e137b759f23fff6) )

	ROM_REGION( 0x400000, "ymz", 0 ) /* Samples */
	ROM_LOAD( "gs230000.u1",  0x000000, 0x400000, CRC(0348e8e1) SHA1(8a21c7e5cea0bc08a2595213d689c58c0251fdb5) )
ROM_END

ROM_START( galpans2a )
	ROM_REGION( 0x080000, "maincpu", 0 ) /* SH-2 Code */
	ROM_LOAD       ( "sknsa1.u10", 0x000000, 0x080000, CRC(745e5212) SHA1(caba649ab2d83b2d7e007eecee0fc582c019df38) ) /* Asia BIOS */

	ROM_REGION32_BE( 0x200000, "user1", 0 ) /* SH-2 Code mapped at 0x04000000 */
	ROM_LOAD16_BYTE( "gps2av11.u6", 0x000000, 0x100000, CRC(61c05d5f) SHA1(e47c7951c1f688edb6c677532f750537a64bb7b3) )
	ROM_LOAD16_BYTE( "gps2av11.u4", 0x000001, 0x100000, CRC(2e8c0ac2) SHA1(d066260d6d3c2924b42394e867523e6112a125c5) )

	ROM_REGION( 0x1000000, "gfx1", 0 )
	ROM_LOAD( "gs210000.u21", 0x000000, 0x400000, CRC(294b2f14) SHA1(90cbd0acdaa2d89d208c28aae33ab57c03624089) )
	ROM_LOAD( "gs210100.u20", 0x400000, 0x400000, CRC(f75c5a9a) SHA1(3919643cee6c88185a1aa3c58c5bc80599bf734e) )
	ROM_LOAD( "gs210200.u8",  0x800000, 0x400000, CRC(25b4f56b) SHA1(f9a33d5ed54a04ecece3035e75508d191bbe74b1) )
	ROM_LOAD( "gs210300.u32", 0xc00000, 0x400000, CRC(db6d4424) SHA1(0a88dafd0ee2490ff2ef39ce8eb1931c41bdda42) )

	ROM_REGION( 0x800000, "gfx2", 0 )
	ROM_LOAD( "gs220000.u17", 0x000000, 0x400000, CRC(5caae1c0) SHA1(8f77e4cf018d7290b2d804cbff9fccf0bf4d2404) )
	ROM_LOAD( "gs220100.u9",  0x400000, 0x400000, CRC(8d51f197) SHA1(19d2afab823ea179918e7bcbf4df2283e77570f0) )

	ROM_REGION( 0x800000, "gfx3", ROMREGION_ERASE00 ) /* Tiles Plane B */
	/* First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles */
	/* 0x040000 - 0x3fffff empty? */
	ROM_LOAD( "gs221000.u3",  0x400000, 0x400000, CRC(58800a18) SHA1(5e6d55ecd12275662d6f59559e137b759f23fff6) )

	ROM_REGION( 0x400000, "ymz", 0 ) /* Samples */
	ROM_LOAD( "gs230000.u1",  0x000000, 0x400000, CRC(0348e8e1) SHA1(8a21c7e5cea0bc08a2595213d689c58c0251fdb5) )
ROM_END

/*

Gals Panic SU (Kaneko 1999)
Korean hacked version. Runs on Super Kaneko Nova System mainboard
and original Super Kaneko Nova System ROM board labelled "ROM-BOARD"
EPROMs at U8, U10 and mainboard U10 are new to this version.
The ROM board is wired to accept 16MBit SOP44 maskROMs.
The actual ROMs used are 32M. There are some wire mods to enable the
higher capacity ROMs, basically wiring pin 44 of the SOP44's to
some logic to enable it.
All of the SOP44 ROMs are from Gals Panic 2, but because Gals Panic 2
uses a different ROM board the Gals Panic SU ROMs are at different
locations.
For Gals Panic SU, the 32M ROMs can be taken from the existing
Gals Panic 2 set.

*/

ROM_START( galpansu )
	ROM_REGION( 0x080000, "maincpu", 0 ) /* SH-2 Code */
	ROM_LOAD       ( "bios.u10",   0x000000, 0x080000, CRC(161fb79e) SHA1(dfb5517de3a14a94344e0f58bbe90b69c5cb5b57) ) /* Korea BIOS */

	ROM_REGION32_BE( 0x200000, "user1", 0 ) /* SH-2 Code mapped at 0x04000000 */
	ROM_LOAD16_BYTE( "su.u10", 0x000000, 0x100000, CRC(5ae66218) SHA1(c3f32603e1da945efb984ff99e1a30202e535773) )
	ROM_LOAD16_BYTE( "su.u8",  0x000001, 0x100000, CRC(10977a03) SHA1(2ab95398d6b88d8819f368ee6104d7f8b485778d) )

	/* the rest of the roms match Gals Panic S2, but are in different locations */
	ROM_REGION( 0x1000000, "gfx1", 0 )
	ROM_LOAD( "24", 0x000000, 0x400000, CRC(294b2f14) SHA1(90cbd0acdaa2d89d208c28aae33ab57c03624089) )
	ROM_LOAD( "20", 0x400000, 0x400000, CRC(f75c5a9a) SHA1(3919643cee6c88185a1aa3c58c5bc80599bf734e) )
	ROM_LOAD( "17", 0x800000, 0x400000, CRC(25b4f56b) SHA1(f9a33d5ed54a04ecece3035e75508d191bbe74b1) )
	ROM_LOAD( "32", 0xc00000, 0x400000, CRC(db6d4424) SHA1(0a88dafd0ee2490ff2ef39ce8eb1931c41bdda42) )

	ROM_REGION( 0x800000, "gfx2", 0 )
	ROM_LOAD( "16", 0x000000, 0x400000, CRC(5caae1c0) SHA1(8f77e4cf018d7290b2d804cbff9fccf0bf4d2404) )
	ROM_LOAD( "13",  0x400000, 0x400000, CRC(8d51f197) SHA1(19d2afab823ea179918e7bcbf4df2283e77570f0) )

	ROM_REGION( 0x800000, "gfx3", ROMREGION_ERASE00 ) /* Tiles Plane B */
	/* First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles */
	/* 0x040000 - 0x3fffff empty? */
	ROM_LOAD( "7",  0x400000, 0x400000, CRC(58800a18) SHA1(5e6d55ecd12275662d6f59559e137b759f23fff6) )

	ROM_REGION( 0x400000, "ymz", 0 ) /* Samples */
	ROM_LOAD( "4",  0x000000, 0x400000, CRC(0348e8e1) SHA1(8a21c7e5cea0bc08a2595213d689c58c0251fdb5) )
ROM_END

ROM_START( galpans3 )
	ROM_REGION( 0x080000, "maincpu", 0 ) /* SH-2 Code */
	ROM_LOAD       ( "sknsj1.u10",   0x000000, 0x080000, CRC(7e2b836c) SHA1(92c5a7a2472496028bff0e5980d41dd294f42144) ) /* Japan BIOS */

	ROM_REGION32_BE( 0x200000, "user1", 0 ) /* SH-2 Code mapped at 0x04000000 */
	ROM_LOAD16_BYTE( "gpss3.u10", 0x000000, 0x100000, CRC(c1449a72) SHA1(02db81a0ea349742d6ddf71d59fcfce45f0c5212) )
	ROM_LOAD16_BYTE( "gpss3.u8",  0x000001, 0x100000, CRC(11eb44cf) SHA1(482ef27fa86d6777def46918eac8be019896c0b0) )

	ROM_REGION( 0x1000000, "gfx1", 0 )
	ROM_LOAD( "u24.bin", 0x000000, 0x800000, CRC(70613168) SHA1(637c50e733dbc0226b1e0acc8000faa7e8977cb6) )

	ROM_REGION( 0x800000, "gfx2", 0 )
	ROM_LOAD( "u16.bin", 0x000000, 0x800000, CRC(a96daf2a) SHA1(40f4c32158d320146aeeac34c15ca6816a6876bc) )

	ROM_REGION( 0x800000, "gfx3", ROMREGION_ERASE00 ) /* Tiles Plane B */
	/* First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles */
	/* 0x040000 - 0x3fffff empty? */

	ROM_REGION( 0x400000, "ymz", 0 ) /* Samples */
	ROM_LOAD( "u4.bin", 0x000000, 0x400000, CRC(bf5736c6) SHA1(781292d87e9da1d21c1ac540baefff5e2f84a3f5) )
ROM_END

ROM_START( gutsn )
	ROM_REGION( 0x080000, "maincpu", 0 ) /* SH-2 Code */
	ROM_LOAD       ( "sknsj1.u10",   0x000000, 0x080000, CRC(7e2b836c) SHA1(92c5a7a2472496028bff0e5980d41dd294f42144) ) /* Japan BIOS */

	ROM_REGION32_BE( 0x200000, "user1", 0 ) /* SH-2 Code mapped at 0x04000000 */
	ROM_LOAD16_BYTE( "gts000j0.u6", 0x000000, 0x080000, CRC(8ee91310) SHA1(8dd918189fe445d79c7f028168862b852f70a6f2) )
	ROM_LOAD16_BYTE( "gts001j0.u4", 0x000001, 0x080000, CRC(80b8ee66) SHA1(4faf5f358ceee866f09bd81e63ba3ebd21bde835) )

	ROM_REGION( 0x400000, "gfx1", 0 )
	ROM_LOAD( "gts10000.u24", 0x000000, 0x400000, CRC(1959979e) SHA1(92a68784664dd833ca6fcca1b15cd46b9365d081) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD( "gts20000.u16", 0x000000, 0x400000, CRC(c443aac3) SHA1(b0416a09ead26077e9276bae98d94eeb1cf86877) )

	ROM_REGION( 0x400000, "gfx3", ROMREGION_ERASE00 ) /* Tiles Plane B */
	/* First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles */
	/* 0x040000 - 0x3fffff empty? */

	ROM_REGION( 0x400000, "ymz", 0 ) /* Samples */
	ROM_LOAD( "gts30000.u4", 0x000000, 0x400000, CRC(8c169141) SHA1(41caea6fa644515f7417c84bdac599b13ad07e8c) )
ROM_END

ROM_START( panicstr )
	ROM_REGION( 0x080000, "maincpu", 0 ) /* SH-2 Code */
	ROM_LOAD       ( "sknsj1.u10",   0x000000, 0x080000, CRC(7e2b836c) SHA1(92c5a7a2472496028bff0e5980d41dd294f42144) ) /* Japan BIOS */

	ROM_REGION32_BE( 0x200000, "user1", 0 ) /* SH-2 Code mapped at 0x04000000 */
	ROM_LOAD16_BYTE( "ps1000j0.u10", 0x000000, 0x100000, CRC(59645f89) SHA1(8da205c6e38899d6c637941700dd7eea56011c10) )
	ROM_LOAD16_BYTE( "ps1001j0.u8",  0x000001, 0x100000, CRC(c4722be9) SHA1(7009d320a80cfa7d80efc5fc915081914bc3c827) )

	ROM_REGION( 0x800000, "gfx1", 0 )
	ROM_LOAD( "ps-10000.u24", 0x000000, 0x400000, CRC(294b2f14) SHA1(90cbd0acdaa2d89d208c28aae33ab57c03624089) )
	ROM_LOAD( "ps110100.u20", 0x400000, 0x400000, CRC(e292f393) SHA1(b0914f7f0abf9f821f2592c289ea4e3b3e7f819a) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD( "ps120000.u16", 0x000000, 0x400000, CRC(d772ac15) SHA1(6bf7b9bfccdcb7481b21fa2ab9b683d79033a192) )

	ROM_REGION( 0x400000, "gfx3", ROMREGION_ERASE00 ) /* Tiles Plane B */
	/* First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles */
	/* 0x040000 - 0x3fffff empty? */

	ROM_REGION( 0x400000, "ymz", 0 ) /* Samples */
	ROM_LOAD( "ps-30000.u4",  0x000000, 0x400000, CRC(2262e263) SHA1(73443e5f40f5c5c9bd41c6207fa6376072f0f65e) )
ROM_END

ROM_START( puzzloop )
	ROM_REGION( 0x080000, "maincpu", 0 ) /* SH-2 Code */
	ROM_LOAD       ( "sknse1.u10", 0x000000, 0x080000, CRC(e2b9d7d1) SHA1(b530a3bb9dedc8cfafcba9f1f10277590be04a15) ) /* Europe BIOS */

	ROM_REGION32_BE( 0x200000, "user1", 0 ) /* SH-2 Code mapped at 0x04000000 */
	ROM_LOAD16_BYTE( "pl00e1.u6", 0x000000, 0x080000, CRC(273adc38) SHA1(37ca873342ba9fb9951114048a9cd255f73fe19c) )
	ROM_LOAD16_BYTE( "pl00e1.u4", 0x000001, 0x080000, CRC(14ac2870) SHA1(d1abcfd64d7c0ead67e879c40e1010453fd4da13) )

	ROM_REGION( 0x800000, "gfx1", 0 )
	ROM_LOAD( "pzl10000.u24", 0x000000, 0x400000, CRC(35bf6897) SHA1(8a1f1f5234a61971a62401633de1dec1920fc4da) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD( "pzl20000.u16", 0x000000, 0x400000, CRC(ff558e68) SHA1(69a50c8100edbf2d5d92ce14b3f079f76c544bdd) )

	ROM_REGION( 0x800000, "gfx3", ROMREGION_ERASE00 ) /* Tiles Plane B */
	/* First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles */
	/* 0x040000 - 0x3fffff empty? */
	ROM_LOAD( "pzl21000.u18", 0x400000, 0x400000, CRC(c8b3be64) SHA1(6da9ca8b963ebf10df6bc02bd1bdc66392e2fa60) )

	ROM_REGION( 0x400000, "ymz", 0 ) /* Samples */
	ROM_LOAD( "pzl30000.u4", 0x000000, 0x400000, CRC(38604b8d) SHA1(1191cf48a6a7baa58e51509442b40ea67f5252d2) )
ROM_END

ROM_START( puzzloopj )
	ROM_REGION( 0x080000, "maincpu", 0 ) /* SH-2 Code */
	ROM_LOAD       ( "sknsj1.u10",   0x000000, 0x080000, CRC(7e2b836c) SHA1(92c5a7a2472496028bff0e5980d41dd294f42144) ) /* Japan BIOS */

	ROM_REGION32_BE( 0x200000, "user1", 0 ) /* SH-2 Code mapped at 0x04000000 */
	ROM_LOAD16_BYTE( "pl0j2.u6", 0x000000, 0x080000, CRC(23c3bf97) SHA1(77ea1f32bed5709a6ad5b250370f08cfe8036867) )
	ROM_LOAD16_BYTE( "pl0j2.u4", 0x000001, 0x080000, CRC(55b2a3cb) SHA1(d4cbe143fe2ad622af808cbd9eedffeff3b77e0d) )

	ROM_REGION( 0x800000, "gfx1", 0 )
	ROM_LOAD( "pzl10000.u24", 0x000000, 0x400000, CRC(35bf6897) SHA1(8a1f1f5234a61971a62401633de1dec1920fc4da) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD( "pzl20000.u16", 0x000000, 0x400000, CRC(ff558e68) SHA1(69a50c8100edbf2d5d92ce14b3f079f76c544bdd) )

	ROM_REGION( 0x800000, "gfx3", ROMREGION_ERASE00 ) /* Tiles Plane B */
	/* First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles */
	/* 0x040000 - 0x3fffff empty? */
	ROM_LOAD( "pzl21000.u18", 0x400000, 0x400000, CRC(c8b3be64) SHA1(6da9ca8b963ebf10df6bc02bd1bdc66392e2fa60) )

	ROM_REGION( 0x400000, "ymz", 0 ) /* Samples */
	ROM_LOAD( "pzl30000.u4", 0x000000, 0x400000, CRC(38604b8d) SHA1(1191cf48a6a7baa58e51509442b40ea67f5252d2) )
ROM_END

ROM_START( puzzloopa )
	ROM_REGION( 0x080000, "maincpu", 0 ) /* SH-2 Code */
	ROM_LOAD       ( "sknsa1.u10", 0x000000, 0x080000, CRC(745e5212) SHA1(caba649ab2d83b2d7e007eecee0fc582c019df38) ) /* Asia BIOS */

	ROM_REGION32_BE( 0x200000, "user1", 0 ) /* SH-2 Code mapped at 0x04000000 */
	ROM_LOAD16_BYTE( "pl0a3.u6", 0x000000, 0x080000, CRC(4e8673b8) SHA1(17acfb0550912e6f2519df2bc24fbf629a1f6147) )
	ROM_LOAD16_BYTE( "pl0a3.u4", 0x000001, 0x080000, CRC(e08a1a07) SHA1(aba58a81ae46c7b4e235a3213984026d170fa189) )

	ROM_REGION( 0x800000, "gfx1", 0 )
	ROM_LOAD( "pzl10000.u24", 0x000000, 0x400000, CRC(35bf6897) SHA1(8a1f1f5234a61971a62401633de1dec1920fc4da) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD( "pzl20000.u16", 0x000000, 0x400000, CRC(ff558e68) SHA1(69a50c8100edbf2d5d92ce14b3f079f76c544bdd) )

	ROM_REGION( 0x800000, "gfx3", ROMREGION_ERASE00 ) /* Tiles Plane B */
	/* First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles */
	/* 0x040000 - 0x3fffff empty? */
	ROM_LOAD( "pzl21000.u18", 0x400000, 0x400000, CRC(c8b3be64) SHA1(6da9ca8b963ebf10df6bc02bd1bdc66392e2fa60) )

	ROM_REGION( 0x400000, "ymz", 0 ) /* Samples */
	ROM_LOAD( "pzl30000.u4", 0x000000, 0x400000, CRC(38604b8d) SHA1(1191cf48a6a7baa58e51509442b40ea67f5252d2) )
ROM_END

ROM_START( puzzloopk )
	ROM_REGION( 0x080000, "maincpu", 0 ) /* SH-2 Code */
	ROM_LOAD       ( "sknsk1.u10",   0x000000, 0x080000, CRC(ff1c9f79) SHA1(a51e598d43e76d37da69b1f094c111273bdfc94a) ) /* Korean BIOS */

	ROM_REGION32_BE( 0x200000, "user1", 0 ) /* SH-2 Code mapped at 0x04000000 */
	ROM_LOAD16_BYTE( "pl0k4.u6", 0x000000, 0x080000, CRC(8d81f20c) SHA1(c32a525e8f92a625e3fecb7c43dd04b13e0a75e4) )
	ROM_LOAD16_BYTE( "pl0k4.u4", 0x000001, 0x080000, CRC(17c78e41) SHA1(4a4b612ae00d521d2947ab32554ebb615be72471) )

	ROM_REGION( 0x800000, "gfx1", 0 )
	ROM_LOAD( "pzl10000.u24", 0x000000, 0x400000, CRC(35bf6897) SHA1(8a1f1f5234a61971a62401633de1dec1920fc4da) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD( "pzl20000.u16", 0x000000, 0x400000, CRC(ff558e68) SHA1(69a50c8100edbf2d5d92ce14b3f079f76c544bdd) )

	ROM_REGION( 0x800000, "gfx3", ROMREGION_ERASE00 ) /* Tiles Plane B */
	/* First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles */
	/* 0x040000 - 0x3fffff empty? */
	ROM_LOAD( "pzl21000.u18", 0x400000, 0x400000, CRC(c8b3be64) SHA1(6da9ca8b963ebf10df6bc02bd1bdc66392e2fa60) )

	ROM_REGION( 0x400000, "ymz", 0 ) /* Samples */
	ROM_LOAD( "pzl30000.u4", 0x000000, 0x400000, CRC(38604b8d) SHA1(1191cf48a6a7baa58e51509442b40ea67f5252d2) )
ROM_END

ROM_START( puzzloopu )
	ROM_REGION( 0x080000, "maincpu", 0 ) /* SH-2 Code */
	ROM_LOAD       ( "sknsu1.u10",   0x000000, 0x080000, CRC(384d21ec) SHA1(a27e8a18099d9cea64fa32db28d01101c2a78815) ) /* US BIOS */

	ROM_REGION32_BE( 0x200000, "user1", 0 ) /* SH-2 Code mapped at 0x04000000 */
	ROM_LOAD16_BYTE( "plue5.u6", 0x000000, 0x080000, CRC(e6f3f82f) SHA1(ac61dc22fa3c1b1c2f3a41d3a8fb43938b77ca68) )
	ROM_LOAD16_BYTE( "plue5.u4", 0x000001, 0x080000, CRC(0d081d30) SHA1(ec0cdf120126104b9bb706f68c9ba9c3777dd69c) )

	ROM_REGION( 0x800000, "gfx1", 0 )
	ROM_LOAD( "pzl10000.u24", 0x000000, 0x400000, CRC(35bf6897) SHA1(8a1f1f5234a61971a62401633de1dec1920fc4da) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD( "pzl20000.u16", 0x000000, 0x400000, CRC(ff558e68) SHA1(69a50c8100edbf2d5d92ce14b3f079f76c544bdd) )

	ROM_REGION( 0x800000, "gfx3", ROMREGION_ERASE00 ) /* Tiles Plane B */
	/* First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles */
	/* 0x040000 - 0x3fffff empty? */
	ROM_LOAD( "pzl21000.u18", 0x400000, 0x400000, CRC(c8b3be64) SHA1(6da9ca8b963ebf10df6bc02bd1bdc66392e2fa60) )

	ROM_REGION( 0x400000, "ymz", 0 ) /* Samples */
	ROM_LOAD( "pzl30000.u4", 0x000000, 0x400000, CRC(38604b8d) SHA1(1191cf48a6a7baa58e51509442b40ea67f5252d2) )
ROM_END

/* haven't even tried to run the ones below yet */

ROM_START( jjparads )
	ROM_REGION( 0x080000, "maincpu", 0 ) /* SH-2 Code */
	ROM_LOAD       ( "sknsj1.u10",   0x000000, 0x080000, CRC(7e2b836c) SHA1(92c5a7a2472496028bff0e5980d41dd294f42144) ) /* Japan BIOS */

	ROM_REGION32_BE( 0x200000, "user1", 0 ) /* SH-2 Code mapped at 0x04000000 */
	ROM_LOAD16_BYTE( "jp1j1.u10", 0x000000, 0x080000, CRC(de2fb669) SHA1(229ff1ae0ec5bc77fc17642964e0bb0146594e86) )
	ROM_LOAD16_BYTE( "jp1j1.u8",  0x000001, 0x080000, CRC(7276efb1) SHA1(3edc265b5c02da7d21a2494a6dc2878fbad93f87) )

	ROM_REGION( 0x1000000, "gfx1", 0 )
	ROM_LOAD( "jp100-00.u24", 0x000000, 0x400000, CRC(f31b2e95) SHA1(7e5bb518d4f6423785d3f9f2752a624a66b42469) )
	ROM_LOAD( "jp101-00.u20", 0x400000, 0x400000, CRC(70cc8c24) SHA1(a4805ce19f512b047829548b635e68690d714175) )
	ROM_LOAD( "jp102-00.u17", 0x800000, 0x400000, CRC(35401c1e) SHA1(38fe86a08555bb823b8d64ac043330aaaa6b8892) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "jp200-00.u16", 0x000000, 0x200000, CRC(493d63db) SHA1(4b8fe7ff1ae14a914a675ce4072a4d9e5cfc08b0) )

	ROM_REGION( 0x400000, "gfx3", ROMREGION_ERASE00 ) /* Tiles Plane B */
	/* First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles */
	/* 0x040000 - 0x3fffff empty? */

	ROM_REGION( 0x200000, "ymz", 0 ) /* Samples */
	ROM_LOAD( "jp300-00.u4", 0x000000, 0x200000, CRC(7023fe46) SHA1(24a92133bc664d63b3be67c2ef11cd7b605ee7e8) )
ROM_END

ROM_START( jjparad2 )
	ROM_REGION( 0x080000, "maincpu", 0 ) /* SH-2 Code */
	ROM_LOAD       ( "sknsj1.u10",   0x000000, 0x080000, CRC(7e2b836c) SHA1(92c5a7a2472496028bff0e5980d41dd294f42144) ) /* Japan BIOS */

	ROM_REGION32_BE( 0x200000, "user1", 0 ) /* SH-2 Code mapped at 0x04000000 */
	ROM_LOAD16_BYTE( "jp2000j1.u6", 0x000000, 0x080000, CRC(5d75e765) SHA1(33bcd8f929f6025b00df2ea783b13a391a28a5c3) )
	ROM_LOAD16_BYTE( "jp2001j1.u4", 0x000001, 0x080000, CRC(1771910a) SHA1(7ca9584d379d7b41f303a3ba861f943c570ad97c) )

	ROM_REGION( 0x1000000, "gfx1", 0 )
	ROM_LOAD( "jp210000.u21", 0x000000, 0x400000, CRC(79a7e3d7) SHA1(bd0f8d01971e5895395f97f2520bcd03ab19d229) )
	ROM_LOAD( "jp210100.u20", 0x400000, 0x400000, CRC(42415e0c) SHA1(f7bff86d55fa9002fbd14e4c62f9d3df8faaf7d0) )
	ROM_LOAD( "jp210200.u8",  0x800000, 0x400000, CRC(26731745) SHA1(8939d36b82b10b1010e4b924e6b9fdd4742efe48) )

	ROM_REGION( 0x800000, "gfx2", 0 )
	ROM_LOAD( "jp220000.u17", 0x000000, 0x400000, CRC(d0e71873) SHA1(c6ffba3624e6d4c2d4e12ef7d88a02cbc3867b18) )
	ROM_LOAD( "jp220100.u9",  0x400000, 0x400000, CRC(4c7d964d) SHA1(3352cd866a64466f4f5a990c2c5e3e28e7028a99) )

	ROM_REGION( 0x400000, "gfx3", ROMREGION_ERASE00 ) /* Tiles Plane B */
	/* First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles */
	/* 0x040000 - 0x3fffff empty? */

	ROM_REGION( 0x400000, "ymz", 0 ) /* Samples */
	ROM_LOAD( "jp230000.u1", 0x000000, 0x400000, CRC(73e30d7f) SHA1(af5b16cec722dbbf0e03d73edfa133dbf10ac4f3) )
ROM_END

ROM_START( sengekis )
	ROM_REGION( 0x080000, "maincpu", 0 ) /* SH-2 Code */
	ROM_LOAD       ( "sknsa1.u10", 0x000000, 0x080000, CRC(745e5212) SHA1(caba649ab2d83b2d7e007eecee0fc582c019df38) ) /* Asia BIOS */

	ROM_REGION32_BE( 0x200000, "user1", 0 ) /* SH-2 Code mapped at 0x04000000 */
	ROM_LOAD16_BYTE( "ss01a.u6", 0x000000, 0x080000, CRC(962fe857) SHA1(3df74c5efff11333dea9316a063129dcec0d7bdd) )
	ROM_LOAD16_BYTE( "ss01a.u4", 0x000001, 0x080000, CRC(ee853c23) SHA1(ddbf7f7cf509788ee3daf7b4d8ae1482e6e31a03) )

	ROM_REGION( 0x1000000, "gfx1", 0 )
	ROM_LOAD( "ss100-00.u21", 0x000000, 0x400000, CRC(bc7b3dfa) SHA1(dff10a7aef548abda48470293382057a2ca9557e) )
	ROM_LOAD( "ss101-00.u20", 0x400000, 0x400000, CRC(ab2df280) SHA1(e456c578a36f585b24379d74def1bcab276c2b1b) )
	ROM_LOAD( "ss102-00.u8",  0x800000, 0x400000, CRC(0845eafe) SHA1(663b163bf4e87c7df0030e791f95b1a5827de315) )
	ROM_LOAD( "ss103-00.u32", 0xc00000, 0x400000, CRC(ee451ac9) SHA1(01cc6b6f371c0090a6a7f4c33d05f4b9a6c59fee) )

	ROM_REGION( 0x800000, "gfx2", 0 )
	ROM_LOAD( "ss200-00.u17", 0x000000, 0x400000, CRC(cd773976) SHA1(38b8df5e685be65c3fde09f9e585591f678632d4) )
	ROM_LOAD( "ss201-00.u9",  0x400000, 0x400000, CRC(301fad4c) SHA1(15faf37eeec5cc46afcb4bd236345b5c3dd647ac) )

	ROM_REGION( 0x600000, "gfx3", ROMREGION_ERASE00 ) /* Tiles Plane B */
	/* First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles */
	/* 0x040000 - 0x3fffff empty? */
	ROM_LOAD( "ss210-00.u3",  0x400000, 0x200000, CRC(c3697805) SHA1(bd41064e3527cdc4b9a4ab9c423c916309b3f057) )

	ROM_REGION( 0x400000, "ymz", 0 ) /* Samples */
	ROM_LOAD( "ss300-00.u1", 0x000000, 0x400000, CRC(35b04b18) SHA1(b69f33fc6a50ec20382329317d20b3c1e7f01b87) )
ROM_END

ROM_START( sengekisj )
	ROM_REGION( 0x080000, "maincpu", 0 ) /* SH-2 Code */
	ROM_LOAD       ( "sknsj1.u10",   0x000000, 0x080000, CRC(7e2b836c) SHA1(92c5a7a2472496028bff0e5980d41dd294f42144) ) /* Japan BIOS */

	ROM_REGION32_BE( 0x200000, "user1", 0 ) /* SH-2 Code mapped at 0x04000000 */
	ROM_LOAD16_BYTE( "ss01j.u6", 0x000000, 0x080000, CRC(9efdcd5a) SHA1(66cca04d07999dc8ca0bcf19db925996b34d0390) )
	ROM_LOAD16_BYTE( "ss01j.u4", 0x000001, 0x080000, CRC(92c3f45e) SHA1(60c647e66b0126fb7749874be39938972481b957) )

	ROM_REGION( 0x1000000, "gfx1", 0 )
	ROM_LOAD( "ss100-00.u21", 0x000000, 0x400000, CRC(bc7b3dfa) SHA1(dff10a7aef548abda48470293382057a2ca9557e) )
	ROM_LOAD( "ss101-00.u20", 0x400000, 0x400000, CRC(ab2df280) SHA1(e456c578a36f585b24379d74def1bcab276c2b1b) )
	ROM_LOAD( "ss102-00.u8",  0x800000, 0x400000, CRC(0845eafe) SHA1(663b163bf4e87c7df0030e791f95b1a5827de315) )
	ROM_LOAD( "ss103-00.u32", 0xc00000, 0x400000, CRC(ee451ac9) SHA1(01cc6b6f371c0090a6a7f4c33d05f4b9a6c59fee) )

	ROM_REGION( 0x800000, "gfx2", 0 )
	ROM_LOAD( "ss200-00.u17", 0x000000, 0x400000, CRC(cd773976) SHA1(38b8df5e685be65c3fde09f9e585591f678632d4) )
	ROM_LOAD( "ss201-00.u9",  0x400000, 0x400000, CRC(301fad4c) SHA1(15faf37eeec5cc46afcb4bd236345b5c3dd647ac) )

	ROM_REGION( 0x600000, "gfx3", ROMREGION_ERASE00 ) /* Tiles Plane B */
	/* First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles */
	/* 0x040000 - 0x3fffff empty? */
	ROM_LOAD( "ss210-00.u3",  0x400000, 0x200000, CRC(c3697805) SHA1(bd41064e3527cdc4b9a4ab9c423c916309b3f057) )

	ROM_REGION( 0x400000, "ymz", 0 ) /* Samples */
	ROM_LOAD( "ss300-00.u1", 0x000000, 0x400000, CRC(35b04b18) SHA1(b69f33fc6a50ec20382329317d20b3c1e7f01b87) )
ROM_END

ROM_START( senknow )
	ROM_REGION( 0x080000, "maincpu", 0 ) /* SH-2 Code */
	ROM_LOAD       ( "sknsj1.u10",   0x000000, 0x080000, CRC(7e2b836c) SHA1(92c5a7a2472496028bff0e5980d41dd294f42144) ) /* Japan BIOS */

	ROM_REGION32_BE( 0x200000, "user1", 0 ) /* SH-2 Code mapped at 0x04000000 */
	ROM_LOAD16_BYTE( "snw000j1.u6", 0x000000, 0x080000, CRC(0d6136f6) SHA1(eedd011cfe03577bfaf386723502d03f6e5dbd8c) )
	ROM_LOAD16_BYTE( "snw001j1.u4", 0x000001, 0x080000, CRC(4a10ec3d) SHA1(bbec4fc53bd61d06ffe5a53debada5785b124fdd) )

	ROM_REGION( 0x0800000, "gfx1", 0 )
	ROM_LOAD( "snw10000.u21", 0x000000, 0x400000, CRC(5133c69c) SHA1(d279df3ffd005dbf0930a8e40eaf2467f8653284) )
	ROM_LOAD( "snw10100.u20", 0x400000, 0x400000, CRC(9dafe03f) SHA1(978b4597ff2a54ac5049fd64798e8173b29dd363) )

	ROM_REGION( 0x800000, "gfx2", 0 )
	ROM_LOAD( "snw20000.u17", 0x000000, 0x400000, CRC(d5fe5f8c) SHA1(817d8d0a5fbc0c50dc3c592f938150f82df97cec) )
	ROM_LOAD( "snw20100.u9",  0x400000, 0x400000, CRC(c0037846) SHA1(3267b142ebce47e1717250239d98fdb4af7964f8) )

	ROM_REGION( 0x800000, "gfx3", ROMREGION_ERASE00 ) /* Tiles Plane B */
	/* First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles */
	/* 0x040000 - 0x3fffff empty? */
	ROM_LOAD( "snw21000.u3",  0x400000, 0x400000, CRC(f5c23e79) SHA1(b509680001c3205b289f43d4f44aaaa7f896419b) )

	ROM_REGION( 0x400000, "ymz", 0 ) /* Samples */
	ROM_LOAD( "snw30000.u1",  0x000000, 0x400000, CRC(ec9eef40) SHA1(8f74ec9cb6054a77227c0505094f0ef8bc371429) )
ROM_END

ROM_START( teljan )
	ROM_REGION( 0x080000, "maincpu", 0 ) /* SH-2 Code */
	ROM_LOAD       ( "sknsj1.u10",   0x000000, 0x080000, CRC(7e2b836c) SHA1(92c5a7a2472496028bff0e5980d41dd294f42144) ) /* Japan BIOS */

	ROM_REGION32_BE( 0x200000, "user1", 0 ) /* SH-2 Code mapped at 0x04000000 */
	ROM_LOAD16_BYTE( "tel1j.u10", 0x000000, 0x080000, CRC(09b552fe) SHA1(2f315fd09eb22fa8c81faa1e926038f20daa845f) )
	ROM_LOAD16_BYTE( "tel1j.u8",  0x000001, 0x080000, CRC(070b4345) SHA1(5743f12a351b89593c6adfaeb8a5a2ab7bc8b424) )

	ROM_REGION( 0x1000000, "gfx1", 0 )
	ROM_LOAD( "tj100-00.u24", 0x000000, 0x400000, CRC(810144f1) SHA1(1c90e71e5f34ee05771ab4a673329f78f17791df) )
	ROM_LOAD( "tj101-00.u20", 0x400000, 0x400000, CRC(82f570e1) SHA1(3ba9d1775f897052aca5cff2edbf575399101c5c) )
	ROM_LOAD( "tj102-00.u17", 0x800000, 0x400000, CRC(ace875dc) SHA1(be97c895beeac979c5704986e818d4f3cfa00e49) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD( "tj200-00.u16", 0x000000, 0x400000, CRC(be0f90b2) SHA1(1848a65f244e1e8a3ff7ab38e76f86cabca8b47e) )

	ROM_REGION( 0x400000, "gfx3", ROMREGION_ERASE00 ) /* Tiles Plane B */
	/* First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles */
	/* 0x040000 - 0x3fffff empty? */

	ROM_REGION( 0x400000, "ymz", 0 ) /* Samples */
	ROM_LOAD( "tj300-00.u4", 0x000000, 0x400000, CRC(685495c4) SHA1(3853c0583b84ed3163370ae48e4b3912cbeb986e) )
ROM_END

ROM_START( ryouran )
	ROM_REGION( 0x080000, "maincpu", 0 ) /* SH-2 Code */
	ROM_LOAD       ( "sknsj1.u10",   0x000000, 0x080000, CRC(7e2b836c) SHA1(92c5a7a2472496028bff0e5980d41dd294f42144) ) /* Japan BIOS */

	ROM_REGION32_BE( 0x200000, "user1", 0 ) /* SH-2 Code mapped at 0x04000000 */
	ROM_LOAD16_BYTE( "or000j1.u10",  0x000000, 0x080000, CRC(d93aa491) SHA1(dc01707f1e80d81f28d6b685d08fc6c0d2bf7330) )
	ROM_LOAD16_BYTE( "or001j1.u8",   0x000001, 0x080000, CRC(f466e5e9) SHA1(65d699f6f9e299333e51a6a52cb13a0f1a902fe1) )

	ROM_REGION( 0x1000000, "gfx1", 0 )
	ROM_LOAD( "or100-00.u24", 0x000000, 0x400000, CRC(e9c7695b) SHA1(0a104d4e4e0c933d2eaaf410a8c243db6673786a) )
	ROM_LOAD( "or101-00.u20", 0x400000, 0x400000, CRC(fe06bf12) SHA1(f3a2f88aed65bcc1c16f37fd4c0011e3538128f7) )
	ROM_LOAD( "or102-00.u17", 0x800000, 0x400000, CRC(f2a5237b) SHA1(b8871f9c0f3864c334ec9a8146cf7dd1961ecb94) )

	ROM_REGION( 0x800000, "gfx2", 0 )
	ROM_LOAD( "or200-00.u16", 0x000000, 0x400000, CRC(4c4701a8) SHA1(7b397b553ba86bba2ee82228cabdf2179e878d69) )
	ROM_LOAD( "or201-00.u13", 0x400000, 0x400000, CRC(a94064aa) SHA1(5d736f810ffdbb6ada5c5efcb5fb29eedafc3e2f) )

	ROM_REGION( 0x400000, "gfx3", ROMREGION_ERASE00 ) /* Tiles Plane B */
	/* First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles */
	/* 0x040000 - 0x3fffff empty? */

	ROM_REGION( 0x400000, "ymz", 0 ) /* Samples */
	ROM_LOAD( "or300-00.u4", 0x000000, 0x400000, CRC(a3f64b79) SHA1(6ecb2b4c0d213fe5384b19d6bfdb86871f21fd9f) )
ROM_END

ROM_START( vblokbrk )
	ROM_REGION( 0x080000, "maincpu", 0 ) /* SH-2 Code */
	ROM_LOAD       ( "sknsa1.u10", 0x000000, 0x080000, CRC(745e5212) SHA1(caba649ab2d83b2d7e007eecee0fc582c019df38) ) /* Asia BIOS */

	ROM_REGION32_BE( 0x200000, "user1", 0 ) /* SH-2 Code mapped at 0x04000000 */
	ROM_LOAD16_BYTE( "sk01a.u10", 0x000000, 0x080000, CRC(4d1be53e) SHA1(3d28b73a67530147962b8df6244af8bea2ab080f) )
	ROM_LOAD16_BYTE( "sk01a.u8",  0x000001, 0x080000, CRC(461e0197) SHA1(003573a4abdbecc6dd234a13c61ef07a25d980e2) )

	ROM_REGION( 0x0400000, "gfx1", 0 )
	ROM_LOAD( "sk100-00.u24", 0x000000, 0x200000, CRC(151dd88a) SHA1(87bb1039a9883f721a315760eb2c4abe4a94046f) )
	ROM_LOAD( "sk-101.u20",   0x200000, 0x100000, CRC(779cce23) SHA1(70147b36d982524ba9921823e481ce8fbb5daa26) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "sk200-00.u16", 0x000000, 0x200000, CRC(2e297c61) SHA1(4071b945a1294fbc3d18fab1f144bf09af4349e8) )

	ROM_REGION( 0x400000, "gfx3", ROMREGION_ERASE00 ) /* Tiles Plane B */
	/* First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles */
	/* 0x040000 - 0x3fffff empty? */

	ROM_REGION( 0x200000, "ymz", 0 ) /* Samples */
	ROM_LOAD( "sk300-00.u4", 0x000000, 0x200000, CRC(e6535c05) SHA1(8895b7c326e0261691cb184887ac1ca637302460) )
ROM_END

ROM_START( sarukani )
	ROM_REGION( 0x080000, "maincpu", 0 ) /* SH-2 Code */
	ROM_LOAD       ( "sknsj1.u10",   0x000000, 0x080000, CRC(7e2b836c) SHA1(92c5a7a2472496028bff0e5980d41dd294f42144) ) /* Japan BIOS */

	ROM_REGION32_BE( 0x200000, "user1", 0 ) /* SH-2 Code mapped at 0x04000000 */
	ROM_LOAD16_BYTE( "sk1j1.u10", 0x000000, 0x080000, CRC(fcc131b6) SHA1(5e3e71ee1f736b6098e671e6f57b1fb313c81adb) )
	ROM_LOAD16_BYTE( "sk1j1.u8",  0x000001, 0x080000, CRC(3b6aa343) SHA1(a969b20b1170d82351024cab9e37f2fbfd01ddeb) )

	ROM_REGION( 0x0400000, "gfx1", 0 )
	ROM_LOAD( "sk100-00.u24", 0x000000, 0x200000, CRC(151dd88a) SHA1(87bb1039a9883f721a315760eb2c4abe4a94046f) )
	ROM_LOAD( "sk-101.u20",   0x200000, 0x100000, CRC(779cce23) SHA1(70147b36d982524ba9921823e481ce8fbb5daa26) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "sk200-00.u16", 0x000000, 0x200000, CRC(2e297c61) SHA1(4071b945a1294fbc3d18fab1f144bf09af4349e8) )

	ROM_REGION( 0x400000, "gfx3", ROMREGION_ERASE00 ) /* Tiles Plane B */
	/* First 0x040000 bytes (0x03ff Tiles) are RAM Based Tiles */
	/* 0x040000 - 0x3fffff empty? */

	ROM_REGION( 0x200000, "ymz", 0 ) /* Samples */
	ROM_LOAD( "sk300-00.u4", 0x000000, 0x200000, CRC(e6535c05) SHA1(8895b7c326e0261691cb184887ac1ca637302460) )
ROM_END


/***** GAME DRIVERS *****/

GAME( 1996, skns,      0,        skns, skns,     0,         ROT0,  "Kaneko", "Super Kaneko Nova System BIOS", GAME_IS_BIOS_ROOT )

GAME( 1996, galpani4,  skns,     skns, cyvern,   galpani4,  ROT0,  "Kaneko", "Gals Panic 4 (Japan)", GAME_IMPERFECT_GRAPHICS )
GAME( 1996, galpani4k, galpani4, skns, cyvern,   galpani4,  ROT0,  "Kaneko", "Gals Panic 4 (Korea)", GAME_IMPERFECT_GRAPHICS )
GAME( 1996, jjparads,  skns,     skns, skns_1p,  jjparads,  ROT0,  "Electro Design", "Jan Jan Paradise", GAME_IMPERFECT_GRAPHICS )
GAME( 1997, galpanis,  skns,     skns, galpanis, galpanis,  ROT0,  "Kaneko", "Gals Panic S - Extra Edition (Japan)", GAME_IMPERFECT_GRAPHICS )
GAME( 1997, galpanisk, galpanis, skns, galpanis, galpanis,  ROT0,  "Kaneko", "Gals Panic S - Extra Edition (Korea)", GAME_IMPERFECT_GRAPHICS )
GAME( 1997, jjparad2,  skns,     skns, skns_1p,  jjparad2,  ROT0,  "Electro Design", "Jan Jan Paradise 2", GAME_IMPERFECT_GRAPHICS )
GAME( 1997, sengekis,  skns,     skns, skns,     sengekis,  ROT90, "Kaneko / Warashi", "Sengeki Striker (Asia)", GAME_IMPERFECT_GRAPHICS )
GAME( 1997, sengekisj, sengekis, skns, skns,     sengekij,  ROT90, "Kaneko / Warashi", "Sengeki Striker (Japan)", GAME_IMPERFECT_GRAPHICS )
GAME( 1997, vblokbrk,  skns,     skns, vblokbrk, sarukani,  ROT0,  "Kaneko / Mediaworks", "VS Block Breaker (Asia)", GAME_IMPERFECT_GRAPHICS )
GAME( 1997, sarukani,  vblokbrk, skns, vblokbrk, sarukani,  ROT0,  "Kaneko / Mediaworks", "Saru-Kani-Hamu-Zou (Japan)", GAME_IMPERFECT_GRAPHICS )
GAME( 1998, cyvern,    skns,     skns, cyvern,   cyvern,    ROT90, "Kaneko", "Cyvern (Japan)", GAME_IMPERFECT_GRAPHICS )
GAME( 1998, puzzloop,  skns,     skns, puzzloop, puzzloop,  ROT0,  "Mitchell", "Puzz Loop (Europe)", GAME_IMPERFECT_GRAPHICS )
GAME( 1998, puzzloopj, puzzloop, skns, puzzloop, puzzloopj, ROT0,  "Mitchell", "Puzz Loop (Japan)", GAME_IMPERFECT_GRAPHICS )
GAME( 1998, puzzloopa, puzzloop, skns, puzzloop, puzzloopa, ROT0,  "Mitchell", "Puzz Loop (Asia)", GAME_IMPERFECT_GRAPHICS )
GAME( 1998, puzzloopk, puzzloop, skns, puzzloop, puzzloopu, ROT0,  "Mitchell", "Puzz Loop (Korea)", GAME_IMPERFECT_GRAPHICS ) // Same speed up as US version
GAME( 1998, puzzloopu, puzzloop, skns, puzzloop, puzzloopu, ROT0,  "Mitchell", "Puzz Loop (USA)", GAME_IMPERFECT_GRAPHICS )
GAME( 1998, ryouran ,  skns,     skns, skns_1p,  ryouran,   ROT0,  "Electro Design", "VS Mahjong Otome Ryouran", GAME_IMPERFECT_GRAPHICS )
GAME( 1999, galpans2,  skns,     skns, galpanis, galpans2,  ROT0,  "Kaneko", "Gals Panic S2 (Japan)", GAME_IMPERFECT_GRAPHICS )
GAME( 1999, galpans2a, galpans2, skns, galpanis, galpans2,  ROT0,  "Kaneko", "Gals Panic S2 (Asia)", GAME_IMPERFECT_GRAPHICS )
GAME( 1999, galpansu,  galpans2, skns, galpanis, galpans2,  ROT0,  "Kaneko", "Gals Panic SU (Korea)", GAME_IMPERFECT_GRAPHICS ) // official or hack?
GAME( 1999, panicstr,  skns,     skns, galpanis, panicstr,  ROT0,  "Kaneko", "Panic Street (Japan)", GAME_IMPERFECT_GRAPHICS )
GAME( 1999, senknow ,  skns,     skns, skns,     senknow,   ROT0,  "Kaneko / Kouyousha", "Sen-Know (Japan)", GAME_IMPERFECT_GRAPHICS )
GAME( 1999, teljan  ,  skns,     skns, skns_1p,  teljan,    ROT0,  "Electro Design", "Tel Jan", GAME_IMPERFECT_GRAPHICS )
GAME( 2000, gutsn,     skns,     skns, skns,     gutsn,     ROT0,  "Kaneko / Kouyousha", "Guts'n (Japan)", GAME_IMPERFECT_GRAPHICS ) // quite fragile, started working of it's own accord in 0.69 :)
GAME( 2002, galpans3,  skns,     skns, galpanis, galpans3,  ROT0,  "Kaneko", "Gals Panic S3 (Japan)", GAME_IMPERFECT_GRAPHICS )
