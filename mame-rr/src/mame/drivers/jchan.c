/*
    Jackie Chan The Kung-Fu Master
    Jackie Chan in Fists of Fire
    (c) Kaneko 1995

    Driver by David Haywood
     based on work by Sebastien Volpe

 started: May 12 2004

Checks done by main code:
- as part of EEPROM data
jchan  : "95/05/24 Jackie ChanVer 1.20"
jchan2 : "95/11/28 Jackie ChanVer 2.31"
- as (one of) MCU protection cmd
jchan  : "1995/05/24 The kung-Fu Master Jackie Chan   "
jchan2 : "1995/10/24 Fists Of Fire"


 main2sub comunication is done within $400000-$403fff (mainsub_shared_ram):
 - $403C02(W) : ]
 - $403C04(W) : ] main68k sets parameters before calling subcpu routine, when required
 - $403C06(W) : ]
 - $403FFE(W) : main68k writes cmd.w, which triggers IT4 on sub68k
 - $400002(W) : subcpu busy status (0=busy, 0xffff=cmd done) - main68k often clears it first and wait 0xffff
 - $400004(W) : subcpu result, read by maincpu, when required
 - $400000(W) : subcpu writes cmd.w, which triggers IT3 on main68k (*)

(*) this happens @ $5b4(cmd 3),$5f2(cmd 1) (diagnostic routine), IT4(cmd 7), and inside subcpu routines $18,$19

99.99% of main->sub communication is done the following:

        clr.w   $400002.l           ; clear (shared ram) sub-cpu busy status word
        move.w  #cmd, $403ffe.l     ; call subcpu
wait    tst.w   $400002.l           ; read (shared ram) sub-cpu busy status word
        beq     wait                ; active wait-loop

***********************************************************************************************

 probably IT 1,2 triggered at a time for maincpu and IT 1,2,3 for subcpu ?

main 68k interrupts (others are rte)
lev 1 : 0x64 : 0000 1ed8 - counter, dsw, controls, + more to find
lev 2 : 0x68 : 0000 1f26 -
lev 3 : 0x6c : 0000 1f68 - comm. with sub68k, triggered when sub writes cmd @ $400000

sub 68k interrupts (others are rte)
lev 1 : 0x64 : 0000 103a -
lev 2 : 0x68 : 0000 1066 -
lev 3 : 0x6c : 0000 10a4 -
lev 4 : 0x70 : 0000 10d0 - comm. with main68k, triggered when main writes cmd @ $403ffe

***********************************************************************************************
 PCB info
***********************************************************************************************


Jackie Chan Kung Fu Master
Jackie Chan Fist Of Fire
Kaneko, 1995

Both games run on the same PCB, only the EPROMs are swapped.

PCB Layout
----------

JC-BOARD
|--------------------------------------------------------------------|
|  VOL  J2M1X1.U56   JC-300-00.U84                     JC-106-00.U171|
|         YMZ280B    JC-301-00.U85  6264                             |
|     CA741           J2P1X5.U86    6264               JC-107-00.U172|
|        62256        J2P1X6.U87          CG24143                    |
|        62256                                         42S4260       |
|        68000        PLSI2032            6264                       |
|                     28.6364MHz    6116  CG24173      42S4260       |
|                                   6116                             |
|                     BABY004             VIEW2-CHIP   JC-200-00.U177|
|J                                   6264                            |
|A     62256                         6264              JC-100-00.U179|
|M     62256                               J2G1X1.U164               |
|M                                                     JC-101-00.U180|
|A                                         J2G1X2.U165               |
|     62256  J2P1X1.U67             6264               JC-102-00.U181|
|     62256  J2P1X2.U68             6264       42S4260               |
|62256       J2P1X3.U69  16MHz    33.3333MHz           JC-103-00.U182|
|62256       J2P1X4.U70  PLSI2032    6116                            |
|                                    6116   CG24173    JC-104-00.U183|
|  J2D1X1.U13  68000                           42S4240               |
|        93C46                                         JC-105-00.U184|
|   TBS0P01                   6264  6264          6264               |
| DSW1                        6264  6264    CG24143    JC-108-00.U185|
|--------------------------------------------------------------------|

Notes:
    68000 clock - 16.0MHz (both)
  YMZ280B clock - 16.0MHz
  TBS0P01 clock - 16.0MHz (NEC uPD78324 with 32K internal ROM & 1024 byte RAM)
          62256 - 32k x8 SRAM
           6264 - 8k x8 SRAM
           6116 - 2k x8 SRAM
        42S4260 - 256k x16 DRAM
CG24143/CG24173 - Fujitsu custom graphics generators
       pLSI2032 - Lattice CPLD
          CA741 - Intersil High Gain Operational Amplifier
          VSync - 60Hz
          HSync - 15.56kHz

Kung Fu Master - Jackie Chan (C) Kaneko 1995

KANEKO PCB NUMBER: JC01X00047

CPU: TMP68HC000N-16 x 2
SND: YAMAHA YMZ280B
OSC: 16.0000MHZ, 33.3333MHZ, 28.6364MHZ
DIPS:1 DIP LABELLED SW2, 8 POSITION
     Location for SW1 on PCB, but empty.

Eproms

Location    Rom Type    PCB Label
U164        27C2001     SPA-7A
U165        27C2001     SPA-7B
U13         27C1001     27C1001A
U56         27C2001     23C8001E
U67         27C040      27C4001
U68         27C040      27C4001
U69         27C040      27C4001
U70         27C040      27C4001
U86         27C040      27C4001
U87         27C040      27C4001

There are 12 mask roms (42 pin) labelled....

Rom Label           Label on PCB        Location
JC-100-00  9511 D           SPA-0           U179
JC-101-00  9511 D           SPA-1           U180
JC-102-00  9511 D           SPA-2           U181
JC-103-00  9511 D           SPA-3           U182
JC-104-00  T39 9510K7092    SPA-4           U183
JC-105-00  T40 9510K7094    SPA-5           U184
JC-108-00  T65 9517K7012    SPA-6           U185
JC-106-00  T41 9510K7091    SPB-0           U171
JC-107-00  T42 9510K7096    SPB-1           U172
JC-200-00  W10 9510K7055    BG-0            U177
JC-300-00  T43 9510K7098    23C16000        U84
JC-301-00  W11 9510K7059    23C16000        U85

SPB-2, SPB-3 and SPA-7 are labeled but unused on this PCB.

Other chips:
  AMTEL AT93C46 (EEPROM)
  LATTICE pLSI 2032-80LJ (x 2, square, socketed labeled JCOP099 & JCOP100)
  FUJITSU CG24143 4181 9449 Z01 (x 2, square SMD)
  FUJITSU CG24173 6186 9447 Z01 (x 2, square SMD)
  KANEKO VIEW2-CHIP 1633F1208 (square, SMD)
  KANEKO BABY004 9511EX009 VT-171 (square, SMD)
  KANEKO TBS0P01 452 9430HK001 (square, SMD NEC uPD78324 aka "TOYBOX")

Ram I can see...
SONY CXK58257ASP-10L (x 8)
NEC D42101C-3 (x 4)
SHARP LH5497D-20 (x 2)
SANYO LC3564SM-85 (x 12, SMD)
NEC 42S4260-70 (x 4, SMD)

there are 9 PALS on the pcb (not dumped)

*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "deprecat.h"
#include "sound/ymz280b.h"
#include "includes/suprnova.h"
#include "includes/kaneko16.h"

static bitmap_t *sprite_bitmap_1;
static bitmap_t *sprite_bitmap_2;

static UINT32* jchan_sprite_ram32_1;
static UINT32* jchan_sprite_ram32_2;
static UINT32* jchan_sprite_regs32_1;
static UINT32* jchan_sprite_regs32_2;

static int jchan_irq_sub_enable;

static UINT16 *jchan_spriteram_1;
static UINT16* jchan_sprregs_1;
static UINT16 *jchan_spriteram_2;
static UINT16* jchan_sprregs_2;

static UINT16 *mainsub_shared_ram;

/***************************************************************************

                            MCU Code Simulation
                (follows the implementation of kaneko16.c)

Provided we find a working PCB, trojan code will help:
- to get this game working (but there are many #4 sub-commands!)
- to have more patterns and eventualy defeat the MCU 'encryption'
- decapping and manually recovering the interal ROM is being persued

This will benefit galpani3 and other kaneko16 games with TOYBOX MCU.

***************************************************************************/
static UINT16 *mcu_ram, jchan_mcu_com[4];

static void jchan_mcu_run(running_machine *machine)
{
	UINT16 mcu_command = mcu_ram[0x0010/2];		/* command nb */
	UINT16 mcu_offset  = mcu_ram[0x0012/2] / 2;	/* offset in shared RAM where MCU will write */
	UINT16 mcu_subcmd  = mcu_ram[0x0014/2];		/* sub-command parameter, happens only for command #4 */

	logerror("%s : MCU executed command: %04X %04X %04X ",cpuexec_describe_context(machine),mcu_command,mcu_offset*2,mcu_subcmd);

/*
    the only MCU commands found in program code are:
    - 0x04: protection: provide data (see below) and code
    - 0x03: read DSW
    - 0x02: load game settings \ stored in ATMEL AT93C46 chip,
    - 0x42: save game settings / 128 bytes serial EEPROM
*/

	switch (mcu_command >> 8)
	{
		case 0x04: /* Protection: during self-test for mcu_subcmd = 0x3d, 0x3e, 0x3f */
		{
			 toxboy_handle_04_subcommand(machine,mcu_subcmd,mcu_ram);
		}
		break;

		case 0x03:	// DSW
		{
			mcu_ram[mcu_offset] = input_port_read(machine, "DSW");
			logerror("%s : MCU executed command: %04X %04X (read DSW)\n",cpuexec_describe_context(machine),mcu_command,mcu_offset*2);
		}
		break;

		case 0x02: /* load game settings from 93C46 EEPROM ($1090-$10dc) */
		{
			mame_file *f;
			if ((f = nvram_fopen(machine, OPEN_FLAG_READ)) != 0)
			{
				mame_fread(f,&mcu_ram[mcu_offset], 128);
				mame_fclose(f);
			}
			logerror("(load NVRAM settings)\n");
		}
		break;

		case 0x42: /* save game settings to 93C46 EEPROM ($50d4) */
		{
			mame_file *f;
			if ((f = nvram_fopen(machine, OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS)) != 0)
			{
				mame_fwrite(f,&mcu_ram[mcu_offset], 128);
				mame_fclose(f);
			}
			logerror("(save NVRAM settings)\n");
		}
		break;

		default:
			logerror("- UNKNOWN COMMAND!!!\n");
	}
}

#define JCHAN_MCU_COM_W(_n_) \
static WRITE16_HANDLER( jchan_mcu_com##_n_##_w ) \
{ \
	COMBINE_DATA(&jchan_mcu_com[_n_]); \
	if (jchan_mcu_com[0] != 0xFFFF)	return; \
	if (jchan_mcu_com[1] != 0xFFFF)	return; \
	if (jchan_mcu_com[2] != 0xFFFF)	return; \
	if (jchan_mcu_com[3] != 0xFFFF)	return; \
\
	memset(jchan_mcu_com, 0, 4 * sizeof( UINT16 ) ); \
	jchan_mcu_run(space->machine); \
}

JCHAN_MCU_COM_W(0)
JCHAN_MCU_COM_W(1)
JCHAN_MCU_COM_W(2)
JCHAN_MCU_COM_W(3)

static READ16_HANDLER( jchan_mcu_status_r )
{
	logerror("cpu '%s' (PC=%06X): read mcu status\n", space->cpu->tag(), cpu_get_previouspc(space->cpu));
	return 0;
}

/***************************************************************************

 video

***************************************************************************/



// interrupt generation is NOT understood
//  but the order must be something like this,
//  if it is incorrect jchan2 will crash when
//  certain characters win/lose but no finish
//  move was performed
static INTERRUPT_GEN( jchan_vblank )
{
	int i = cpu_getiloops(device);
	switch (i)
	{

		case 0:
			cpu_set_input_line(device, 1, HOLD_LINE);
			break;

		case 100:
			cpu_set_input_line(device, 2, HOLD_LINE);
			break;

	}

	if (jchan_irq_sub_enable)
	{
		switch (i)
		{

			case 0:
				cputag_set_input_line(device->machine, "sub", 1, HOLD_LINE);
				break;

			case 220:
				cputag_set_input_line(device->machine, "sub", 2, HOLD_LINE);
				break;

			case 100:
				cputag_set_input_line(device->machine, "sub", 3, HOLD_LINE);
				break;

		}
	}
}




static VIDEO_START(jchan)
{
	/* so we can use suprnova.c */
	jchan_sprite_ram32_1 = auto_alloc_array(machine, UINT32, 0x4000/4);
	jchan_sprite_ram32_2 = auto_alloc_array(machine, UINT32, 0x4000/4);

	machine->generic.spriteram_size = 0x4000;
	jchan_sprite_regs32_1 = auto_alloc_array(machine, UINT32, 0x40/4);
	jchan_sprite_regs32_2 = auto_alloc_array(machine, UINT32, 0x40/4);

	sprite_bitmap_1 = auto_bitmap_alloc(machine,1024,1024,BITMAP_FORMAT_INDEXED16);
	sprite_bitmap_2 = auto_bitmap_alloc(machine,1024,1024,BITMAP_FORMAT_INDEXED16);

	suprnova_alt_enable_sprites = 1;

	VIDEO_START_CALL( kaneko16_1xVIEW2_tilemaps );
}







static VIDEO_UPDATE(jchan)
{
	int x,y;
	UINT16* src1;
	UINT16* src2;
	UINT16* dst;
	UINT16 pixdata1;
	UINT16 pixdata2;

	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));

	VIDEO_UPDATE_CALL(jchan_view2);

	bitmap_fill(sprite_bitmap_1, cliprect, 0x0000);
	bitmap_fill(sprite_bitmap_2, cliprect, 0x0000);

	skns_draw_sprites(screen->machine, sprite_bitmap_1, cliprect, jchan_sprite_ram32_1, 0x4000, memory_region(screen->machine,"gfx1"), memory_region_length (screen->machine, "gfx1"), jchan_sprite_regs32_1 );
	skns_draw_sprites(screen->machine, sprite_bitmap_2, cliprect, jchan_sprite_ram32_2, 0x4000, memory_region(screen->machine,"gfx2"), memory_region_length (screen->machine, "gfx2"), jchan_sprite_regs32_2 );

	// ignoring priority bits for now - might use alpha too, check 0x8000 of palette writes
	for (y=0;y<240;y++)
	{
		src1 = BITMAP_ADDR16(sprite_bitmap_1, y, 0);
		src2 = BITMAP_ADDR16(sprite_bitmap_2, y, 0);
		dst =  BITMAP_ADDR16(bitmap, y, 0);

		for (x=0;x<320;x++)
		{
			pixdata1 = src1[x];
			pixdata2 = src2[x];

			if (pixdata2 & 0x3fff)
			{
				dst[x] = (pixdata2 & 0x3fff)|0x4000;
			}

			if (pixdata1 & 0x3fff)
			{
				dst[x] = (pixdata1 & 0x3fff)|0x4000;
			}
		}
	}

	return 0;
}

/***************************************************************************

 controls

***************************************************************************/
/*
    controls handling routine is $21e2a, part of IT 1
    player 1/2 controls are read from $f00000/$f00002 resp.
    $f00006 is read and impacts controls 'decoding'
    $f00000 is the only location also written
*/
static UINT16 *jchan_ctrl;

static WRITE16_HANDLER( jchan_ctrl_w )
{
	jchan_irq_sub_enable = data & 0x8000; // hack / guess!
}

static READ16_HANDLER ( jchan_ctrl_r )
{
	switch(offset)
	{
		case 0/2: return input_port_read(space->machine, "P1");
		case 2/2: return input_port_read(space->machine, "P2");
		case 4/2: return input_port_read(space->machine, "SYSTEM");
		case 6/2: return input_port_read(space->machine, "EXTRA");
		default: logerror("jchan_ctrl_r unknown!"); break;
	}
	return jchan_ctrl[offset];
}

/***************************************************************************

 memory maps

***************************************************************************/

/* communications - hacky! */
static WRITE16_HANDLER( main2sub_cmd_w )
{
	COMBINE_DATA(&mainsub_shared_ram[0x03ffe/2]);
	cputag_set_input_line(space->machine, "sub", 4, HOLD_LINE);
}

// is this called?
static WRITE16_HANDLER( sub2main_cmd_w )
{
	COMBINE_DATA(&mainsub_shared_ram[0x0000/2]);
	cputag_set_input_line(space->machine, "maincpu", 3, HOLD_LINE);
}

/* ram convert for suprnova (requires 32-bit stuff) */
static WRITE16_HANDLER( jchan_suprnova_sprite32_1_w )
{
	COMBINE_DATA(&jchan_spriteram_1[offset]);
	offset>>=1;
	jchan_sprite_ram32_1[offset]=(jchan_spriteram_1[offset*2+1]<<16) | (jchan_spriteram_1[offset*2]);
}

static WRITE16_HANDLER( jchan_suprnova_sprite32regs_1_w )
{
	COMBINE_DATA(&jchan_sprregs_1[offset]);
	offset>>=1;
	jchan_sprite_regs32_1[offset]=(jchan_sprregs_1[offset*2+1]<<16) | (jchan_sprregs_1[offset*2]);
}

static WRITE16_HANDLER( jchan_suprnova_sprite32_2_w )
{
	COMBINE_DATA(&jchan_spriteram_2[offset]);
	offset>>=1;
	jchan_sprite_ram32_2[offset]=(jchan_spriteram_2[offset*2+1]<<16) | (jchan_spriteram_2[offset*2]);
}

static WRITE16_HANDLER( jchan_suprnova_sprite32regs_2_w )
{
	COMBINE_DATA(&jchan_sprregs_2[offset]);
	offset>>=1;
	jchan_sprite_regs32_2[offset]=(jchan_sprregs_2[offset*2+1]<<16) | (jchan_sprregs_2[offset*2]);
}


static ADDRESS_MAP_START( jchan_main, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM
	AM_RANGE(0x200000, 0x20ffff) AM_RAM // Work RAM - [A] grid tested, cleared ($9d6-$a54)

	AM_RANGE(0x300000, 0x30ffff) AM_RAM AM_BASE(&mcu_ram)	// MCU [G] grid tested, cleared ($a5a-$ad8)
	AM_RANGE(0x330000, 0x330001) AM_WRITE(jchan_mcu_com0_w)	// _[ these 2 are set to 0xFFFF
	AM_RANGE(0x340000, 0x340001) AM_WRITE(jchan_mcu_com1_w)	//  [ to trigger mcu to run cmd ?
	AM_RANGE(0x350000, 0x350001) AM_WRITE(jchan_mcu_com2_w)	// _[ these 2 are set to 0xFFFF
	AM_RANGE(0x360000, 0x360001) AM_WRITE(jchan_mcu_com3_w)	//  [ for mcu to return its status ?
	AM_RANGE(0x370000, 0x370001) AM_READ(jchan_mcu_status_r)

	AM_RANGE(0x400000, 0x403fff) AM_RAM AM_BASE(&mainsub_shared_ram) AM_SHARE("share1")

	/* 1st sprite layer */
	AM_RANGE(0x500000, 0x503fff) AM_RAM_WRITE(jchan_suprnova_sprite32_1_w) AM_BASE(&jchan_spriteram_1)
	AM_RANGE(0x600000, 0x60003f) AM_RAM_WRITE(jchan_suprnova_sprite32regs_1_w) AM_BASE(&jchan_sprregs_1)

	AM_RANGE(0x700000, 0x70ffff) AM_RAM_WRITE(paletteram16_xGGGGGRRRRRBBBBB_word_w) AM_BASE_GENERIC(paletteram) // palette for sprites?

	AM_RANGE(0xf00000, 0xf00007) AM_READWRITE(jchan_ctrl_r, jchan_ctrl_w) AM_BASE(&jchan_ctrl)

	AM_RANGE(0xf80000, 0xf80001) AM_READWRITE(watchdog_reset16_r, watchdog_reset16_w)	// watchdog
ADDRESS_MAP_END


static ADDRESS_MAP_START( jchan_sub, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x10ffff) AM_RAM // Work RAM - grid tested, cleared ($612-$6dc)

	AM_RANGE(0x400000, 0x403fff) AM_RAM AM_BASE(&mainsub_shared_ram) AM_SHARE("share1")

	/* VIEW2 Tilemap - [D] grid tested, cleared ($1d84), also cleared at startup ($810-$826) */
	AM_RANGE(0x500000, 0x500fff) AM_RAM_WRITE(kaneko16_vram_1_w) AM_BASE(&kaneko16_vram_1)	// Layers 0
	AM_RANGE(0x501000, 0x501fff) AM_RAM_WRITE(kaneko16_vram_0_w) AM_BASE(&kaneko16_vram_0)	//
	AM_RANGE(0x502000, 0x502fff) AM_RAM AM_BASE(&kaneko16_vscroll_1)									//
	AM_RANGE(0x503000, 0x503fff) AM_RAM AM_BASE(&kaneko16_vscroll_0)									//
	AM_RANGE(0x600000, 0x60001f) AM_RAM_WRITE(kaneko16_layers_0_regs_w) AM_BASE(&kaneko16_layers_0_regs)	// Layers 0 Regs

	/* background prites */
	AM_RANGE(0x700000, 0x703fff) AM_RAM_WRITE(jchan_suprnova_sprite32_2_w) AM_BASE(&jchan_spriteram_2)
	AM_RANGE(0x780000, 0x78003f) AM_RAM_WRITE(jchan_suprnova_sprite32regs_2_w) AM_BASE(&jchan_sprregs_2)

	AM_RANGE(0x800000, 0x800003) AM_DEVWRITE8("ymz", ymz280b_w, 0x00ff) // sound

	AM_RANGE(0xa00000, 0xa00001) AM_READWRITE(watchdog_reset16_r, watchdog_reset16_w)	// watchdog
ADDRESS_MAP_END


static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 4, 0, 12, 8, 20, 16, 28, 24, 8*32+4, 8*32+0, 8*32+12, 8*32+8, 8*32+20, 8*32+16, 8*32+28, 8*32+24 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32, 16*32,17*32, 18*32, 19*32, 20*32, 21*32, 22*32, 23*32 },
	32*32
};

// we don't decode the sprites, they are non-tile based and RLE encoded!, see suprnova.c */

static GFXDECODE_START( jchan )
	GFXDECODE_ENTRY( "gfx3", 0, tilelayout,   0, 0x4000/16  )
	GFXDECODE_ENTRY( "gfx3", 0, tilelayout,   0, 0x4000/16  ) // video/kaneko16.c is hardcoded to here for now
GFXDECODE_END


/* input ports */

/* BUTTON1 : weak punch - BUTTON2 : strong punch - BUTTON3 : weak kick - BUTTON 4 : strong kick */
static INPUT_PORTS_START( jchan )
	PORT_START("P1")		/* $f00000.w (-> $2000b1.b) */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP	) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN	) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT	) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT	) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_CONDITION("DSW",0x8000,PORTCOND_EQUALS,0x8000)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )                PORT_CONDITION("DSW",0x8000,PORTCOND_EQUALS,0x0000)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")		/* $f00002.w (-> $2000b5.b) */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP	) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN	) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT	) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT	) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_CONDITION("DSW",0x8000,PORTCOND_EQUALS,0x8000)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )                PORT_CONDITION("DSW",0x8000,PORTCOND_EQUALS,0x0000)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")	/* $f00004.b */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1	)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2	)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_NAME( DEF_STR( Test ) ) PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("EXTRA")		/* $f00006.b */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_CONDITION("DSW",0x8000,PORTCOND_EQUALS,0x8000)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )                PORT_CONDITION("DSW",0x8000,PORTCOND_EQUALS,0x8000)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_CONDITION("DSW",0x8000,PORTCOND_EQUALS,0x8000)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )                PORT_CONDITION("DSW",0x8000,PORTCOND_EQUALS,0x8000)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_CONDITION("DSW",0x8000,PORTCOND_EQUALS,0x0000)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_CONDITION("DSW",0x8000,PORTCOND_EQUALS,0x0000)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_CONDITION("DSW",0x8000,PORTCOND_EQUALS,0x0000)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_CONDITION("DSW",0x8000,PORTCOND_EQUALS,0x0000)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )               /* duplicated Player 1 Button 4 (whatever the layout is) */
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )               /* duplicated Player 2 Button 4 (whatever the layout is) */
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")		/* provided by the MCU - $200098.b <- $300200 */
	PORT_SERVICE( 0x0100, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, "Sound" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Mono ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Stereo ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x0000, "Blood Mode" )
	PORT_DIPSETTING(      0x2000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( High ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Special Prize Available" )    /* impacts $200008.l many times -> see high-score tables - WTF is it ? */
	PORT_DIPSETTING(      0x4000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Buttons Layout" )             /* impacts $200116.l once! -> impacts reading of controls at 0x00021e2a */
	PORT_DIPSETTING(      0x8000, "3+1" )
	PORT_DIPSETTING(      0x0000, "2+2" )
INPUT_PORTS_END

static INPUT_PORTS_START( jchan2 )
	PORT_INCLUDE( jchan )

	PORT_MODIFY("DSW")
	PORT_DIPUNUSED( 0x4000, IP_ACTIVE_LOW )                      /* only read in the "test mode" ("Input Test" screen) */
//  PORT_DIPNAME( 0x8000, 0x8000, "Buttons Layout" )             /* impacts $20011e.l once! -> impacts reading of controls at 0x0002a9b2 */
INPUT_PORTS_END


/* sound stuff */

static const ymz280b_interface ymz280b_intf =
{
	0	// irq ?
};


/* machine driver */

static MACHINE_DRIVER_START( jchan )

	MDRV_CPU_ADD("maincpu", M68000, 16000000)
	MDRV_CPU_PROGRAM_MAP(jchan_main)
	MDRV_CPU_VBLANK_INT_HACK(jchan_vblank, 224)

	MDRV_CPU_ADD("sub", M68000, 16000000)
	MDRV_CPU_PROGRAM_MAP(jchan_sub)

	MDRV_GFXDECODE(jchan)


	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 64*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0*8, 30*8-1)

	MDRV_PALETTE_LENGTH(0x10000)

	MDRV_VIDEO_START(jchan)
	MDRV_VIDEO_UPDATE(jchan)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymz", YMZ280B, 16000000)
	MDRV_SOUND_CONFIG(ymz280b_intf)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END

/* rom loading */

ROM_START( jchan )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "jm01x3.u67", 0x000001, 0x080000, CRC(c0adb141) SHA1(de265e1da06e723492e0c2465cd04e25ce1c237f) )
	ROM_LOAD16_BYTE( "jm00x3.u68", 0x000000, 0x080000, CRC(b1aadc5a) SHA1(0a93693088c0a4b8a79159fb0ebac47d5556d800) )
	ROM_LOAD16_BYTE( "jm11x3.u69", 0x100001, 0x080000, CRC(d2e3f913) SHA1(db2d790fba5351660a9525f545ab1b23dfe319b0) )
	ROM_LOAD16_BYTE( "jm10x3.u70", 0x100000, 0x080000, CRC(ee08fee1) SHA1(5514bd8c625bc7cf8dd5da2f76b760716609b925) )

	ROM_REGION( 0x100000, "sub", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "jsp1x3.u86", 0x000001, 0x080000, CRC(d15d2b8e) SHA1(e253f2d64fee6627f68833b441f41ea6bbb3ab07) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "jsp0x3.u87", 0x000000, 0x080000, CRC(ebec50b1) SHA1(57d7bd728349c2b9d662bcf20a3be92902cb3ffb) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x2000000, "gfx1", 0 ) /* SPA GFX */
	ROM_LOAD( "jc-100-00.179", 0x0000000, 0x0400000, CRC(578d928c) SHA1(1cfe04f9b02c04f95a85d6fe7c4306a535ff969f) ) // SPA0 kaneko logo
	ROM_LOAD( "jc-101-00.180", 0x0400000, 0x0400000, CRC(7f5e1aca) SHA1(66ed3deedfd55d88e7dcd017b9c2ce523ccb421a) ) // SPA1
	ROM_LOAD( "jc-102-00.181", 0x0800000, 0x0400000, CRC(72caaa68) SHA1(f6b98aa949768a306ac9bc5f9c05a1c1a3fb6c3f) ) // SPA2
	ROM_LOAD( "jc-103-00.182", 0x0c00000, 0x0400000, CRC(4e9e9fc9) SHA1(bf799cdee930b7f71aea4d55c3dd6a760f7478bb) ) // SPA3 title logo? + char select
	ROM_LOAD( "jc-104-00.183", 0x1000000, 0x0200000, CRC(6b2a2e93) SHA1(e34010e39043b67493bcb23a04828ab7cda8ba4d) ) // SPA4
	ROM_LOAD( "jc-105-00.184", 0x1200000, 0x0200000, CRC(73cad1f0) SHA1(5dbe4e318948e4f74bfc2d0d59455d43ba030c0d) ) // SPA5 11xxxxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD( "jc-108-00.185", 0x1400000, 0x0200000, CRC(67dd1131) SHA1(96f334378ae0267bdb3dc528635d8d03564bd859) ) // SPA6 text
	ROM_LOAD16_BYTE( "jcs0x3.164", 0x1600000, 0x040000, CRC(9a012cbc) SHA1(b3e7390220c90d55dccfb96397f0af73925e36f9) ) // SPA-7A female portraits
	ROM_LOAD16_BYTE( "jcs1x3.165", 0x1600001, 0x040000, CRC(57ae7c8d) SHA1(4086f638c2aabcee84e838243f0fd15cec5c040d) ) // SPA-7B female portraits

	ROM_REGION( 0x1000000, "gfx2", 0 ) /* SPB GFX (background sprites) */
	ROM_LOAD( "jc-106-00.171", 0x000000, 0x200000, CRC(bc65661b) SHA1(da28b8fcd7c7a0de427a54be2cf41a1d6a295164) ) // SPB0
	ROM_LOAD( "jc-107-00.172", 0x200000, 0x200000, CRC(92a86e8b) SHA1(c37eddbc9d84239deb543504e27b5bdaf2528f79) ) // SPB1

	ROM_REGION( 0x100000, "gfx3", 0 ) /* BG GFX */
	ROM_LOAD( "jc-200.00", 0x000000, 0x100000, CRC(1f30c24e) SHA1(0c413fc67c3ec020e6786e7157d82aa242c8d2ad) )

	ROM_REGION( 0x1000000, "ymz", 0 ) /* Audio */
	ROM_LOAD( "jc-301-00.85", 0x000000, 0x100000, CRC(9c5b3077) SHA1(db9a31e1c65d9f12d0f2fb316ced48a02aae089d) )
	ROM_RELOAD(0x100000,0x100000)
	ROM_LOAD( "jc-300-00.84", 0x200000, 0x200000, CRC(13d5b1eb) SHA1(b047594d0f1a71d89b8f072879ccba480f54a483) )
	ROM_LOAD( "jcw0x0.u56",   0x400000, 0x040000, CRC(bcf25c2a) SHA1(b57a563ab5c05b05d133eed3d099c4de997f37e4) )

	ROM_REGION( 0x020000, "mcudata", 0 ) /* MCU Data */
	ROM_LOAD16_WORD_SWAP( "jcd0x1.u13", 0x000000, 0x020000, CRC(2a41da9c) SHA1(7b1ba0efc0544e276196b9605df1881fde871708) )
ROM_END


ROM_START( jchan2 ) /* Some kind of semi-sequel? MASK ROMs dumped and confirmed to be the same */
	ROM_REGION( 0x200000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "j2p1x1.u67", 0x000001, 0x080000, CRC(5448c4bc) SHA1(447835275d5454f86a51879490a6b22b06a23e81) )
	ROM_LOAD16_BYTE( "j2p1x2.u68", 0x000000, 0x080000, CRC(52104ab9) SHA1(d6647e628662bdb832270540ece18b265b7ce62d) )
	ROM_LOAD16_BYTE( "j2p1x3.u69", 0x100001, 0x080000, CRC(8763ebca) SHA1(daf6af42a34802ef9aa996e340e218779bad695f) )
	ROM_LOAD16_BYTE( "j2p1x4.u70", 0x100000, 0x080000, CRC(0f8e5e69) SHA1(1f71042458f76b7d99382db6412fb6c362cd3ded) )

	ROM_REGION( 0x100000, "sub", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "j2p1x5.u86", 0x000001, 0x080000, CRC(dc897725) SHA1(d3e94bac96497deb2f79996c2d4a349f6da5b1d6) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "j2p1x6.u87", 0x000000, 0x080000, CRC(594224f9) SHA1(bc546a98c5f3c5b08f521c54a4b0e9e2cdf83ced) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x2000000, "gfx1", 0 ) /* SPA GFX */
	ROM_LOAD( "jc-100-00.179", 0x0000000, 0x0400000, CRC(578d928c) SHA1(1cfe04f9b02c04f95a85d6fe7c4306a535ff969f) ) // SPA0 kaneko logo
	ROM_LOAD( "jc-101-00.180", 0x0400000, 0x0400000, CRC(7f5e1aca) SHA1(66ed3deedfd55d88e7dcd017b9c2ce523ccb421a) ) // SPA1
	ROM_LOAD( "jc-102-00.181", 0x0800000, 0x0400000, CRC(72caaa68) SHA1(f6b98aa949768a306ac9bc5f9c05a1c1a3fb6c3f) ) // SPA2
	ROM_LOAD( "jc-103-00.182", 0x0c00000, 0x0400000, CRC(4e9e9fc9) SHA1(bf799cdee930b7f71aea4d55c3dd6a760f7478bb) ) // SPA3 title logo? + char select
	ROM_LOAD( "jc-104-00.183", 0x1000000, 0x0200000, CRC(6b2a2e93) SHA1(e34010e39043b67493bcb23a04828ab7cda8ba4d) ) // SPA4
	ROM_LOAD( "jc-105-00.184", 0x1200000, 0x0200000, CRC(73cad1f0) SHA1(5dbe4e318948e4f74bfc2d0d59455d43ba030c0d) ) // SPA5 11xxxxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD( "jc-108-00.185", 0x1400000, 0x0200000, CRC(67dd1131) SHA1(96f334378ae0267bdb3dc528635d8d03564bd859) ) // SPA6 text
	ROM_LOAD16_BYTE( "j2g1x1.164", 0x1600000, 0x080000, CRC(66a7ea6a) SHA1(605cbc1eb50fb0decbea790f2a11e999d5fde762) ) // SPA-7A female portraits
	ROM_LOAD16_BYTE( "j2g1x2.165", 0x1600001, 0x080000, CRC(660e770c) SHA1(1e385a6ee83559b269d2179e6c247238c0f3c850) ) // SPA-7B female portraits

	ROM_REGION( 0x1000000, "gfx2", 0 ) /* SPB GFX (background sprites) */
	ROM_LOAD( "jc-106-00.171", 0x000000, 0x200000, CRC(bc65661b) SHA1(da28b8fcd7c7a0de427a54be2cf41a1d6a295164) ) // SPB0
	ROM_LOAD( "jc-107-00.172", 0x200000, 0x200000, CRC(92a86e8b) SHA1(c37eddbc9d84239deb543504e27b5bdaf2528f79) ) // SPB1

	ROM_REGION( 0x100000, "gfx3", 0 ) /* BG GFX */
	ROM_LOAD( "jc-200.00", 0x000000, 0x100000, CRC(1f30c24e) SHA1(0c413fc67c3ec020e6786e7157d82aa242c8d2ad) )

	ROM_REGION( 0x1000000, "ymz", 0 ) /* Audio */
	ROM_LOAD( "jc-301-00.85", 0x000000, 0x100000, CRC(9c5b3077) SHA1(db9a31e1c65d9f12d0f2fb316ced48a02aae089d) )
	ROM_RELOAD(0x100000,0x100000)
	ROM_LOAD( "jc-300-00.84", 0x200000, 0x200000, CRC(13d5b1eb) SHA1(b047594d0f1a71d89b8f072879ccba480f54a483) )
	ROM_LOAD( "j2m1x1.u56",   0x400000, 0x040000, CRC(baf6e25e) SHA1(6b02f3eb1eafcd43022a9f60f98573d02277adfe) )

	ROM_REGION( 0x020000, "mcudata", 0 ) /* MCU data */
	ROM_LOAD16_WORD_SWAP( "j2d1x1.u13", 0x000000, 0x020000, CRC(b2b7fc90) SHA1(1b90c13bb41a313c4ed791a15d56073a7c29928b) )
ROM_END

static DRIVER_INIT( jchan )
{
	DRIVER_INIT_CALL( decrypt_toybox_rom );
	// install these here, putting them in the memory map causes issues
	memory_install_write16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x403ffe, 0x403fff, 0, 0, main2sub_cmd_w );
	memory_install_write16_handler(cputag_get_address_space(machine, "sub", ADDRESS_SPACE_PROGRAM), 0x400000, 0x400001, 0, 0, sub2main_cmd_w );


	memset(jchan_mcu_com, 0, 4 * sizeof( UINT16 ) );
}


/* game drivers */
GAME( 1995, jchan,     0,        jchan,    jchan,    jchan,    ROT0, "Kaneko", "Jackie Chan - The Kung-Fu Master", GAME_IMPERFECT_GRAPHICS )
GAME( 1995, jchan2,    0,        jchan,    jchan2,   jchan,    ROT0, "Kaneko", "Jackie Chan in Fists of Fire", GAME_IMPERFECT_GRAPHICS )
