/***************************************************************************

Bagman memory map

driver by Nicola Salmoria
protection and speech emulation by Jarek Burczynski
protection info Andrew Deschenes

memory map:

0000-5fff ROM
6000-67ff RAM
9000-93ff Video RAM
9800-9bff Color RAM
9800-981f Sprites (hidden portion of color RAM)
9c00-9fff ? (filled with 3f, not used otherwise)
c000-ffff ROM (Super Bagman only)

memory mapped ports:

read:
a000      PAL16r6 output. (RD4 line)
a800      ? (read only in one place, not used) (RD5 line)
b000      DSW (RD6 line)
b800      watchdog reset (RD7 line)

write:
a000      interrupt enable
a001      horizontal flip
a002      vertical flip
a003      video enable?? (seems to be unused in the schems)
a004      coin counter
a007      ? /SCS line in the schems connected to AY8910 pin A4 or AA (schems are unreadable)

a800-a805 these lines control the state machine driving TMS5110 (only bit 0 matters)
          a800,a801,a802 - speech roms BIT select (000 bit 7, 001 bit 4, 010 bit 2)
          a803 - 0 keeps the state machine in reset state; 1 starts speech
          a804 - connected to speech rom 11 (QS) chip enable
          a805 - connected to speech rom 12 (QT) chip enable
b000      ?
b800      ?


PAL16r6 This chip is custom logic used for guards controlling.
        Inputs are connected to buffered address(!!!) lines AB0,AB1,AB2,AB3,AB4,AB5,AB6
        We simulate this writing a800 to a805 there (which is wrong but works)


I/O ports:

I/O 8  ;AY-3-8910 Control Reg.
I/O 9  ;AY-3-8910 Data Write Reg.
I/O C  ;AY-3-8910 Data Read Reg.
        Port A of the 8910 is connected to IN0
        Port B of the 8910 is connected to IN1

DIP locations verified for:
    - bagman (manual)
    - squaitsa (manual)

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "sound/tms5110.h"
#include "includes/bagman.h"

//static int speech_rom_address = 0;

static UINT8 ls259_buf[8] = {0,0,0,0,0,0,0,0};


static WRITE8_DEVICE_HANDLER( bagman_ls259_w )
{
	const address_space *space = cputag_get_address_space(device->machine, "maincpu", ADDRESS_SPACE_PROGRAM);
	bagman_pal16r6_w(space, offset,data); /*this is just a simulation*/

	if (ls259_buf[offset] != (data&1) )
	{
		ls259_buf[offset] = data&1;

		switch (offset)
		{
		case 0:
		case 1:
		case 2:
			tmsprom_bit_w(device, 0, 7 - ((ls259_buf[0]<<2) | (ls259_buf[1]<<1) | (ls259_buf[2]<<0)));
			break;
		case 3:
			tmsprom_enable_w(device, ls259_buf[offset]);
			break;
		case 4:
			tmsprom_rom_csq_w(device, 0, ls259_buf[offset]);
			break;
		case 5:
			tmsprom_rom_csq_w(device, 1, ls259_buf[offset]);
			break;
		}
	}
}

static WRITE8_HANDLER( bagman_coin_counter_w )
{
	coin_counter_w(space->machine, offset,data);
}

static WRITE8_DEVICE_HANDLER( bagman_interrupt_w )
{
	data &= 1;
	if (!data)
		cpu_set_input_line(device, 0, CLEAR_LINE);
	cpu_interrupt_enable(device, data);
}

static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0x67ff) AM_RAM
	AM_RANGE(0x9000, 0x93ff) AM_RAM_WRITE(bagman_videoram_w) AM_BASE(&bagman_videoram)
	AM_RANGE(0x9800, 0x9bff) AM_RAM_WRITE(bagman_colorram_w) AM_BASE(&bagman_colorram)
	AM_RANGE(0x9c00, 0x9fff) AM_WRITENOP	/* written to, but unused */
	AM_RANGE(0xa000, 0xa000) AM_READ(bagman_pal16r6_r)
	//AM_RANGE(0xa800, 0xa805) AM_READ(bagman_ls259_r) /*just for debugging purposes*/
	AM_RANGE(0xa000, 0xa000) AM_DEVWRITE("maincpu", bagman_interrupt_w)
	AM_RANGE(0xa001, 0xa002) AM_WRITE(bagman_flipscreen_w)
	AM_RANGE(0xa003, 0xa003) AM_WRITEONLY AM_BASE(&bagman_video_enable)
	AM_RANGE(0xc000, 0xffff) AM_ROM /* Super Bagman only */
	AM_RANGE(0x9800, 0x981f) AM_WRITEONLY AM_BASE_SIZE_GENERIC(spriteram)	/* hidden portion of color RAM */
									/* here only to initialize the pointer, */
									/* writes are handled by bagman_colorram_w */
	AM_RANGE(0xa800, 0xa805) AM_DEVWRITE("tmsprom", bagman_ls259_w) /* TMS5110 driving state machine */
	AM_RANGE(0xa004, 0xa004) AM_WRITE(bagman_coin_counter_w)
	AM_RANGE(0xb000, 0xb000) AM_READ_PORT("DSW")
	AM_RANGE(0xb800, 0xb800) AM_READNOP								/* looks like watchdog from schematics */

#if 0
	AM_RANGE(0xa007, 0xa007) AM_WRITENOP	/* ???? */
	AM_RANGE(0xb000, 0xb000) AM_WRITENOP	/* ???? */
	AM_RANGE(0xb800, 0xb800) AM_WRITENOP	/* ???? */
#endif
ADDRESS_MAP_END



static ADDRESS_MAP_START( pickin_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x7000, 0x77ff) AM_RAM
	AM_RANGE(0x8800, 0x8bff) AM_RAM_WRITE(bagman_videoram_w) AM_BASE(&bagman_videoram)
	AM_RANGE(0x9800, 0x9bff) AM_RAM_WRITE(bagman_colorram_w) AM_BASE(&bagman_colorram)
	AM_RANGE(0x9800, 0x981f) AM_WRITEONLY AM_BASE_SIZE_GENERIC(spriteram)	/* hidden portion of color RAM */
									/* here only to initialize the pointer, */
									/* writes are handled by bagman_colorram_w */
	AM_RANGE(0x9c00, 0x9fff) AM_WRITENOP	/* written to, but unused */
	AM_RANGE(0xa000, 0xa000) AM_WRITE(interrupt_enable_w)
	AM_RANGE(0xa001, 0xa002) AM_WRITE(bagman_flipscreen_w)
	AM_RANGE(0xa003, 0xa003) AM_WRITEONLY AM_BASE(&bagman_video_enable)
	AM_RANGE(0xa004, 0xa004) AM_WRITE(bagman_coin_counter_w)
	AM_RANGE(0xa800, 0xa800) AM_READ_PORT("DSW")


	AM_RANGE(0xa005, 0xa005) AM_WRITENOP	/* ???? */
	AM_RANGE(0xa006, 0xa006) AM_WRITENOP	/* ???? */
	AM_RANGE(0xa007, 0xa007) AM_WRITENOP	/* ???? */

	/* guess */
	AM_RANGE(0xb000, 0xb000) AM_DEVWRITE("ay2", ay8910_address_w)
	AM_RANGE(0xb800, 0xb800) AM_DEVREADWRITE("ay2", ay8910_r, ay8910_data_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( main_portmap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x08, 0x09) AM_DEVWRITE("aysnd", ay8910_address_data_w)
	AM_RANGE(0x0c, 0x0c) AM_DEVREAD("aysnd", ay8910_r)
	//AM_RANGE(0x56, 0x56) AM_WRITENOP
ADDRESS_MAP_END



static INPUT_PORTS_START( bagman )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )			PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(	0x03, "2" )
	PORT_DIPSETTING(	0x02, "3" )
	PORT_DIPSETTING(	0x01, "4" )
	PORT_DIPSETTING(	0x00, "5" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Coinage ) )			PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(	0x00, "2C/1C 1C/1C 1C/3C 1C/7C" )
	PORT_DIPSETTING(	0x04, "1C/1C 1C/2C 1C/6C 1C/14C" )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) )		PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(	0x18, DEF_STR( Easy ) )
	PORT_DIPSETTING(	0x10, DEF_STR( Medium ) )
	PORT_DIPSETTING(	0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Language ) )			PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(	0x20, DEF_STR( English ) )
	PORT_DIPSETTING(	0x00, DEF_STR( French ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Bonus_Life ) )		PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(	0x40, "30000" )
	PORT_DIPSETTING(	0x00, "40000" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )			PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(	0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Cocktail ) )
INPUT_PORTS_END

static INPUT_PORTS_START( bagmans )
	PORT_INCLUDE( bagman )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x20, 0x20, DEF_STR ( Demo_Sounds ) )
	PORT_DIPSETTING(	0x00, DEF_STR ( Off ) )
	PORT_DIPSETTING(	0x20, DEF_STR ( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( sbagman )
	PORT_INCLUDE( bagman )

	PORT_MODIFY("P1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) /* double-function button, start and shoot */

	PORT_MODIFY("P2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL /* double-function button, start and shoot */
INPUT_PORTS_END

static INPUT_PORTS_START( pickin )
	PORT_INCLUDE( bagman )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Coinage ) )			PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(	0x00, "2C/1C 1C/1C 1C/3C 1C/7C" )
	PORT_DIPSETTING(	0x01, "1C/1C 1C/2C 1C/6C 1C/14C" )
	PORT_DIPNAME( 0x06, 0x04, DEF_STR( Lives ) )			PORT_DIPLOCATION("SW1:2,3")
	PORT_DIPSETTING(	0x06, "2" )
	PORT_DIPSETTING(	0x04, "3" )
	PORT_DIPSETTING(	0x02, "4" )
	PORT_DIPSETTING(	0x00, "5" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Free_Play ) )		PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(	0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW1:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW1:6" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Language ) )			PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(	0x40, DEF_STR( English ) )
	PORT_DIPSETTING(	0x00, DEF_STR( French ) )
INPUT_PORTS_END

static INPUT_PORTS_START( botanic )
	PORT_INCLUDE( bagman )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x08, 0x08, "Invulnerability Fruits" )	PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(	0x08, "3" )
	PORT_DIPSETTING(	0x00, DEF_STR( None ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW1:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW1:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW1:7" )
INPUT_PORTS_END


static INPUT_PORTS_START( squaitsa )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SPECIAL ) // special handling for the p1 dial
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SPECIAL ) // ^
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SPECIAL ) // special handling for the p2 dial
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SPECIAL ) // ^
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_START("DSW")
    PORT_DIPNAME(    0x01, 0x01, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW:1")
    PORT_DIPSETTING( 0x00, DEF_STR( 2C_1C ) )
    PORT_DIPSETTING( 0x01, DEF_STR( 1C_1C ) )
    PORT_DIPNAME(    0x06, 0x06, "Max Points" ) PORT_DIPLOCATION("SW:2,3")
    PORT_DIPSETTING( 0x06, "7" )
    PORT_DIPSETTING( 0x04, "11" )
    PORT_DIPSETTING( 0x02, "15" )
    PORT_DIPSETTING( 0x00, "21" )
    PORT_DIPNAME(    0x18, 0x18, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW:4,5")
    PORT_DIPSETTING( 0x00, "Level 1" )
    PORT_DIPSETTING( 0x08, "Level 2" )
    PORT_DIPSETTING( 0x10, "Level 3" )
    PORT_DIPSETTING( 0x18, "Level 4" )
    PORT_DIPNAME(    0x20, 0x20, DEF_STR( Language ) ) PORT_DIPLOCATION("SW:6")
    PORT_DIPSETTING( 0x20, DEF_STR( Spanish ) )
    PORT_DIPSETTING( 0x00, DEF_STR( English ) )
    PORT_DIPNAME(    0x40, 0x40, "Body Fault" ) PORT_DIPLOCATION("SW:7")
    PORT_DIPSETTING( 0x40, DEF_STR( Off ) )
    PORT_DIPSETTING( 0x00, DEF_STR( On ) )
    PORT_DIPNAME(    0x80, 0x00, "Protection?" )	/* Left empty in the dips scan */
    PORT_DIPSETTING( 0x80, DEF_STR( Off ) )
    PORT_DIPSETTING( 0x00, DEF_STR( On ) )

	PORT_START("DIAL_P1")
    PORT_BIT( 0xff, 0, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(5)

	PORT_START("DIAL_P2")
    PORT_BIT( 0xff, 0, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(5) PORT_COCKTAIL
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,	/* 8*8 characters */
	512,	/* 512 characters */
	2,	/* 2 bits per pixel */
	{ 0, 512*8*8 }, /* the two bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7 }, /* pretty straightforward layout */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};
static const gfx_layout spritelayout =
{
	16,16,	/* 16*16 sprites */
	128,	/* 128 sprites */
	2,	/* 2 bits per pixel */
	{ 0, 128*16*16 },	/* the two bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7,	/* pretty straightforward layout */
			8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8	/* every sprite takes 32 consecutive bytes */
};



static GFXDECODE_START( bagman )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,		0, 16 )	/* char set #1 */
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout,	0, 16 )	/* sprites */
	GFXDECODE_ENTRY( "gfx2", 0, charlayout,		0, 16 )	/* char set #2 */
GFXDECODE_END

static GFXDECODE_START( pickin )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,		0, 16 )	/* char set #1 */
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout,	0, 16 )	/* sprites */
	/* no gfx2 */
GFXDECODE_END



static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_INPUT_PORT("P1"),
	DEVCB_INPUT_PORT("P2"),
	DEVCB_NULL,
	DEVCB_NULL
};

/* squaitsa doesn't map the dial directly, instead it polls the results of the dial thru an external circuitry.
   I don't know if the following is correct, there can possbily be multiple solutions for the same problem. */
static READ8_DEVICE_HANDLER( dial_input_p1_r )
{
	static UINT8 res,dial_val,old_val;

	dial_val = input_port_read(device->machine, "DIAL_P1");

	if(res != 0x60)
		res = 0x60;
	else if(dial_val > old_val)
		res = 0x40;
	else if(dial_val < old_val)
		res = 0x20;
	else
		res = 0x60;

	old_val = dial_val;

	return (input_port_read(device->machine, "P1") & 0x9f) | (res);
}

static READ8_DEVICE_HANDLER( dial_input_p2_r )
{
	static UINT8 res,dial_val,old_val;

	dial_val = input_port_read(device->machine, "DIAL_P2");

	if(res != 0x60)
		res = 0x60;
	else if(dial_val > old_val)
		res = 0x40;
	else if(dial_val < old_val)
		res = 0x20;
	else
		res = 0x60;

	old_val = dial_val;

	return (input_port_read(device->machine, "P2") & 0x9f) | (res);
}

static const ay8910_interface ay8910_dial_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_HANDLER(dial_input_p1_r),
	DEVCB_HANDLER(dial_input_p2_r),
	DEVCB_NULL,
	DEVCB_NULL
};

static const ay8910_interface ay8910_interface_2 =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

static const tmsprom_interface prom_intf =
{
	"5110ctrl",						/* prom memory region - sound region is automatically assigned */
	0x1000,							/* individual rom_size */
	1,								/* bit # of pdc line */
	/* virtual bit 8: constant 0, virtual bit 9:constant 1 */
	8,								/* bit # of ctl1 line */
	2,								/* bit # of ctl2 line */
	8,								/* bit # of ctl4 line */
	2,								/* bit # of ctl8 line */
	6,								/* bit # of rom reset */
	7,								/* bit # of stop */
	DEVCB_DEVICE_LINE("tms", tms5110_pdc_w),		/* tms pdc func */
	DEVCB_DEVICE_HANDLER("tms", tms5110_ctl_w)		/* tms ctl func */
};

static const tms5110_interface bagman_tms5110_interface =
{
	/* legacy interface */
	NULL,											/* function to be called when chip requests another bit */
	NULL,											/* speech ROM load address callback */
	/* new rom controller interface */
	DEVCB_DEVICE_LINE("tmsprom", tmsprom_m0_w),		/* the M0 line */
	DEVCB_NULL,										/* the M1 line */
	DEVCB_NULL,										/* Write to ADD1,2,4,8 - 4 address bits */
	DEVCB_DEVICE_LINE("tmsprom", tmsprom_data_r),	/* Read one bit from ADD8/Data - voice data */
	DEVCB_NULL										/* rom clock - Only used to drive the data lines */
};

static MACHINE_DRIVER_START( bagman )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, BAGMAN_H0)
	MDRV_CPU_PROGRAM_MAP(main_map)
	MDRV_CPU_IO_MAP(main_portmap)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_assert)

	MDRV_MACHINE_RESET(bagman)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_RAW_PARAMS(BAGMAN_HCLK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART)
	MDRV_GFXDECODE(bagman)
	MDRV_PALETTE_LENGTH(64)

	MDRV_PALETTE_INIT(bagman)
	MDRV_VIDEO_START(bagman)
	MDRV_VIDEO_UPDATE(bagman)

	MDRV_DEVICE_ADD("tmsprom", TMSPROM, 640000 / 2)  /* rom clock */
	MDRV_DEVICE_CONFIG(prom_intf)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("aysnd", AY8910, BAGMAN_H0 / 2)
	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)

	MDRV_SOUND_ADD("tms", TMS5110A, 640000)
	MDRV_SOUND_CONFIG(bagman_tms5110_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( pickin )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, BAGMAN_H0)
	MDRV_CPU_PROGRAM_MAP(pickin_map)
	MDRV_CPU_IO_MAP(main_portmap)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_MACHINE_RESET(bagman)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_RAW_PARAMS(BAGMAN_HCLK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_GFXDECODE(pickin)
	MDRV_PALETTE_LENGTH(64)

	MDRV_PALETTE_INIT(bagman)
	MDRV_VIDEO_START(bagman)
	MDRV_VIDEO_UPDATE(bagman)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("aysnd", AY8910, 1500000)
	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)

	/* maybe */
	MDRV_SOUND_ADD("ay2", AY8910, 1500000)
	MDRV_SOUND_CONFIG(ay8910_interface_2)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)
MACHINE_DRIVER_END

/*

Botanic
Valadon Automation 1983

z80
6116 - work ram
2x 2114 - screen ram
2x 2114
6x 27ls00 - sprite buffer ram

2x ay8910

18.432mhz crystal

*/


static MACHINE_DRIVER_START( botanic )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, BAGMAN_H0)
	MDRV_CPU_PROGRAM_MAP(pickin_map)
	MDRV_CPU_IO_MAP(main_portmap)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_MACHINE_RESET(bagman)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_RAW_PARAMS(BAGMAN_HCLK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART)

	MDRV_GFXDECODE(bagman)
	MDRV_PALETTE_LENGTH(64)

	MDRV_PALETTE_INIT(bagman)
	MDRV_VIDEO_START(bagman)
	MDRV_VIDEO_UPDATE(bagman)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("aysnd", AY8910, 1500000)
	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)

	MDRV_SOUND_ADD("ay2", AY8910, 1500000)
	MDRV_SOUND_CONFIG(ay8910_interface_2)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( squaitsa )
	MDRV_IMPORT_FROM( botanic )
	MDRV_SOUND_MODIFY("aysnd")
	MDRV_SOUND_CONFIG(ay8910_dial_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)
MACHINE_DRIVER_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( bagman )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "e9_b05.bin",   0x0000, 0x1000, CRC(e0156191) SHA1(bb5f16d49fbe48f3bac118acd1fea51ec4bc5355) )
	ROM_LOAD( "f9_b06.bin",   0x1000, 0x1000, CRC(7b758982) SHA1(c8460023b43fed4aca9c6b987faea334832c5e30) )
	ROM_LOAD( "f9_b07.bin",   0x2000, 0x1000, CRC(302a077b) SHA1(916c4a6ea1e631cc72bdb91ff9d263dcbaf08bb2) )
	ROM_LOAD( "k9_b08.bin",   0x3000, 0x1000, CRC(f04293cb) SHA1(ce6b0ae4088ce28c75d414f506fad2cf2b6920c2) )
	ROM_LOAD( "m9_b09s.bin",  0x4000, 0x1000, CRC(68e83e4f) SHA1(9454564885a1003cee7107db18bedb387b85e9ab) )
	ROM_LOAD( "n9_b10.bin",   0x5000, 0x1000, CRC(1d6579f7) SHA1(3ab54329f516156b1c9f68efbe59c95d5240bc8c) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "e1_b02.bin",   0x0000, 0x1000, CRC(4a0a6b55) SHA1(955f8bd4bd9b0fc3c6c359c25ba543ba26c04cbd) )
	ROM_LOAD( "j1_b04.bin",   0x1000, 0x1000, CRC(c680ef04) SHA1(79406bc786374abfcd9f548268c445b5c8d8858d) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "c1_b01.bin",   0x0000, 0x1000, CRC(705193b2) SHA1(ca9cfd05f9195c2a38e8854012de51b6ee6bb403) )
	ROM_LOAD( "f1_b03s.bin",  0x1000, 0x1000, CRC(dba1eda7) SHA1(26d877028b3a31dd671f9e667316c8a14780ca73) )

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "p3.bin",       0x0000, 0x0020, CRC(2a855523) SHA1(91e032233fee397c90b7c1662934aca9e0671482) )
	ROM_LOAD( "r3.bin",       0x0020, 0x0020, CRC(ae6f1019) SHA1(fd711882b670380cb4bd909c840ba06277b8fbe3) )

	ROM_REGION( 0x0060, "5110ctrl", 0)
	ROM_LOAD( "r6.bin",       0x0000, 0x0020, CRC(c58a4f6a) SHA1(35ef244b3e94032df2610aa594ea5670b91e1449) ) /*state machine driving TMS5110*/

	ROM_REGION( 0x2000, "tmsprom", 0 ) /* data for the TMS5110 speech chip */
	ROM_LOAD( "r9_b11.bin",   0x0000, 0x1000, CRC(2e0057ff) SHA1(33e3ffa6418f86864eb81e5e9bda4bf540c143a6) )
	ROM_LOAD( "t9_b12.bin",   0x1000, 0x1000, CRC(b2120edd) SHA1(52b89dbcc749b084331fa82b13d0876e911fce52) )
ROM_END

ROM_START( bagnard )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "e9_b05.bin",   0x0000, 0x1000, CRC(e0156191) SHA1(bb5f16d49fbe48f3bac118acd1fea51ec4bc5355) )
	ROM_LOAD( "f9_b06.bin",   0x1000, 0x1000, CRC(7b758982) SHA1(c8460023b43fed4aca9c6b987faea334832c5e30) )
	ROM_LOAD( "f9_b07.bin",   0x2000, 0x1000, CRC(302a077b) SHA1(916c4a6ea1e631cc72bdb91ff9d263dcbaf08bb2) )
	ROM_LOAD( "k9_b08.bin",   0x3000, 0x1000, CRC(f04293cb) SHA1(ce6b0ae4088ce28c75d414f506fad2cf2b6920c2) )
	ROM_LOAD( "bagnard.009",  0x4000, 0x1000, CRC(4f0088ab) SHA1(a8009f5b8517ba4d84fbc483b199f2514f24eae8) )
	ROM_LOAD( "bagnard.010",  0x5000, 0x1000, CRC(cd2cac01) SHA1(76749161feb9af2b3e928408a21b93d143915b57) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "e1_b02.bin",   0x0000, 0x1000, CRC(4a0a6b55) SHA1(955f8bd4bd9b0fc3c6c359c25ba543ba26c04cbd) )
	ROM_LOAD( "j1_b04.bin",   0x1000, 0x1000, CRC(c680ef04) SHA1(79406bc786374abfcd9f548268c445b5c8d8858d) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "bagnard.001",  0x0000, 0x1000, CRC(060b044c) SHA1(3121f07adb661663a2303085eea1b662968f8f98) )
	ROM_LOAD( "bagnard.003",  0x1000, 0x1000, CRC(8043bc1a) SHA1(bd2f3dfe26cf8d987d9ecaa41eac4bdc4e16a692) )

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "p3.bin",       0x0000, 0x0020, CRC(2a855523) SHA1(91e032233fee397c90b7c1662934aca9e0671482) )
	ROM_LOAD( "r3.bin",       0x0020, 0x0020, CRC(ae6f1019) SHA1(fd711882b670380cb4bd909c840ba06277b8fbe3) )

	ROM_REGION( 0x0060, "5110ctrl", 0)
	ROM_LOAD( "r6.bin",       0x0000, 0x0020, CRC(c58a4f6a) SHA1(35ef244b3e94032df2610aa594ea5670b91e1449) ) /*state machine driving TMS5110*/

	ROM_REGION( 0x2000, "tmsprom", 0 ) /* data for the TMS5110 speech chip */
	ROM_LOAD( "r9_b11.bin",   0x0000, 0x1000, CRC(2e0057ff) SHA1(33e3ffa6418f86864eb81e5e9bda4bf540c143a6) )
	ROM_LOAD( "t9_b12.bin",   0x1000, 0x1000, CRC(b2120edd) SHA1(52b89dbcc749b084331fa82b13d0876e911fce52) )
ROM_END

ROM_START( bagnarda )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bagman.005",   0x0000, 0x1000, CRC(98fca49c) SHA1(60bf15d700cf4174ac531c11febf21d69ec02db5) )
	ROM_LOAD( "bagman.006",   0x1000, 0x1000, CRC(8f447432) SHA1(71fee4feb92cdd35dcd3ad9e95ea9f186cb25e25) )
	ROM_LOAD( "bagman.007",   0x2000, 0x1000, CRC(236203a6) SHA1(3d661c135a5036adeaf5fed2be38c97bbc72cd0a) )
	ROM_LOAD( "bagman.008",   0x3000, 0x1000, CRC(8bd8c6cb) SHA1(3d34333b20d8ef189425334985285e0634c5ee23) )
	ROM_LOAD( "bagman.009",   0x4000, 0x1000, CRC(6211ba82) SHA1(6d43e16cc99159b188f93bed7f9afef81c1b7fb3) )
	ROM_LOAD( "bagman.010",   0x5000, 0x1000, CRC(08ed1247) SHA1(172fb0d1b919fb80f5603ebb52779664122f8e94) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "bagman.002",   0x0000, 0x1000, CRC(7dc57abc) SHA1(73ae325ac1077936741833d33095ad6375353c31) )
	ROM_LOAD( "bagman.004",   0x1000, 0x1000, CRC(1e21577e) SHA1(fc849c2fbaf7353a44a9f2743ccf6ac1adb8dc62) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "bagman.001",   0x0000, 0x1000, CRC(1eb56acd) SHA1(f75f6709006e78417999d423d2078ed80eae73a2) )
	ROM_LOAD( "bagman.003",   0x1000, 0x1000, CRC(0ad82a39) SHA1(30ac0ff5bc63934c3eb572c7c13df324757e5e44) )

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "p3.bin",       0x0000, 0x0020, CRC(2a855523) SHA1(91e032233fee397c90b7c1662934aca9e0671482) )
	ROM_LOAD( "r3.bin",       0x0020, 0x0020, CRC(ae6f1019) SHA1(fd711882b670380cb4bd909c840ba06277b8fbe3) )

	ROM_REGION( 0x0060, "5110ctrl", 0)
	ROM_LOAD( "r6.bin",       0x0000, 0x0020, CRC(c58a4f6a) SHA1(35ef244b3e94032df2610aa594ea5670b91e1449) ) /*state machine driving TMS5110*/

	ROM_REGION( 0x2000, "tmsprom", 0 ) /* data for the TMS5110 speech chip */
	ROM_LOAD( "r9_b11.bin",   0x0000, 0x1000, CRC(2e0057ff) SHA1(33e3ffa6418f86864eb81e5e9bda4bf540c143a6) )
	ROM_LOAD( "t9_b12.bin",   0x1000, 0x1000, CRC(b2120edd) SHA1(52b89dbcc749b084331fa82b13d0876e911fce52) )
ROM_END

ROM_START( bagmans )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a4_9e.bin",    0x0000, 0x1000, CRC(5fb0a1a3) SHA1(849cd60b58de9585a78a1c4c1747f666a4a4fcc3) )
	ROM_LOAD( "a5-9f",        0x1000, 0x1000, CRC(2ddf6bb9) SHA1(151068dddc55163bb6f925f68e5d04e347ded6a5) )
	ROM_LOAD( "a4_9j.bin",    0x2000, 0x1000, CRC(b2da8b77) SHA1(ea36cd6be42c5548a9a91054aeebb4b985ba15c9) )
	ROM_LOAD( "a5-9k",        0x3000, 0x1000, CRC(f91d617b) SHA1(a3323b51277e08747701cc4e2d3a9c466e96d4c1) )
	ROM_LOAD( "a4_9m.bin",    0x4000, 0x1000, CRC(b8e75eb6) SHA1(433fd736512f10bc0879b15821eb55cc41d58d33) )
	ROM_LOAD( "a5-9n",        0x5000, 0x1000, CRC(68e4b64d) SHA1(55950d7c07c621cafa001d5d3bfec6bbc02712e2) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "a2_1e.bin",    0x0000, 0x1000, CRC(f217ac09) SHA1(a9716674401dff27344a01df8121b6b648688680) )
	ROM_LOAD( "j1_b04.bin",   0x1000, 0x1000, CRC(c680ef04) SHA1(79406bc786374abfcd9f548268c445b5c8d8858d) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "a2_1c.bin",    0x0000, 0x1000, CRC(f3e11bd7) SHA1(43ee00ff777008c89f619eb183e7c5e63f6c7694) )
	ROM_LOAD( "a2_1f.bin",    0x1000, 0x1000, CRC(d0f7105b) SHA1(fb382703850a4ded567706e02ebb7f3e22531b7c) )

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "p3.bin",       0x0000, 0x0020, CRC(2a855523) SHA1(91e032233fee397c90b7c1662934aca9e0671482) )
	ROM_LOAD( "r3.bin",       0x0020, 0x0020, CRC(ae6f1019) SHA1(fd711882b670380cb4bd909c840ba06277b8fbe3) )

	ROM_REGION( 0x0060, "5110ctrl", 0)
	ROM_LOAD( "r6.bin",       0x0000, 0x0020, CRC(c58a4f6a) SHA1(35ef244b3e94032df2610aa594ea5670b91e1449) ) /*state machine driving TMS5110*/

	ROM_REGION( 0x2000, "tmsprom", 0 ) /* data for the TMS5110 speech chip */
	ROM_LOAD( "r9_b11.bin",   0x0000, 0x1000, CRC(2e0057ff) SHA1(33e3ffa6418f86864eb81e5e9bda4bf540c143a6) )
	ROM_LOAD( "t9_b12.bin",   0x1000, 0x1000, CRC(b2120edd) SHA1(52b89dbcc749b084331fa82b13d0876e911fce52) )
ROM_END

ROM_START( bagmans2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a4_9e.bin",    0x0000, 0x1000, CRC(5fb0a1a3) SHA1(849cd60b58de9585a78a1c4c1747f666a4a4fcc3) )
	ROM_LOAD( "a4_9f.bin",    0x1000, 0x1000, CRC(7871206e) SHA1(14d9b7a0779d59a870e0d4b911797dff5435a16c) )
	ROM_LOAD( "a4_9j.bin",    0x2000, 0x1000, CRC(b2da8b77) SHA1(ea36cd6be42c5548a9a91054aeebb4b985ba15c9) )
	ROM_LOAD( "a4_9k.bin",    0x3000, 0x1000, CRC(36b6a944) SHA1(270dd2566b36129366adcbdd5a8db396bec7631f) )
	ROM_LOAD( "a4_9m.bin",    0x4000, 0x1000, CRC(b8e75eb6) SHA1(433fd736512f10bc0879b15821eb55cc41d58d33) )
	ROM_LOAD( "a4_9n.bin",    0x5000, 0x1000, CRC(83fccb1c) SHA1(7225d738b64a2cdaaec8860017de4229f2852ed2) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "a2_1e.bin",    0x0000, 0x1000, CRC(f217ac09) SHA1(a9716674401dff27344a01df8121b6b648688680) )
	ROM_LOAD( "j1_b04.bin",   0x1000, 0x1000, CRC(c680ef04) SHA1(79406bc786374abfcd9f548268c445b5c8d8858d) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "a2_1c.bin",    0x0000, 0x1000, CRC(f3e11bd7) SHA1(43ee00ff777008c89f619eb183e7c5e63f6c7694) )
	ROM_LOAD( "a2_1f.bin",    0x1000, 0x1000, CRC(d0f7105b) SHA1(fb382703850a4ded567706e02ebb7f3e22531b7c) )

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "p3.bin",       0x0000, 0x0020, CRC(2a855523) SHA1(91e032233fee397c90b7c1662934aca9e0671482) )
	ROM_LOAD( "r3.bin",       0x0020, 0x0020, CRC(ae6f1019) SHA1(fd711882b670380cb4bd909c840ba06277b8fbe3) )

	ROM_REGION( 0x0060, "5110ctrl", 0)
	ROM_LOAD( "r6.bin",       0x0000, 0x0020, CRC(c58a4f6a) SHA1(35ef244b3e94032df2610aa594ea5670b91e1449) ) /*state machine driving TMS5110*/

	ROM_REGION( 0x2000, "tmsprom", 0 ) /* data for the TMS5110 speech chip */
	ROM_LOAD( "r9_b11.bin",   0x0000, 0x1000, CRC(2e0057ff) SHA1(33e3ffa6418f86864eb81e5e9bda4bf540c143a6) )
	ROM_LOAD( "t9_b12.bin",   0x1000, 0x1000, CRC(b2120edd) SHA1(52b89dbcc749b084331fa82b13d0876e911fce52) )
ROM_END



ROM_START( sbagman )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "5.9e",         0x0000, 0x1000, CRC(1b1d6b0a) SHA1(549161f6adc88fa16339815e05af33ca57815660) )
	ROM_LOAD( "6.9f",         0x1000, 0x1000, CRC(ac49cb82) SHA1(5affa0c03bedf2c9d5368c7f075818e1760c12ae) )
	ROM_LOAD( "7.9j",         0x2000, 0x1000, CRC(9a1c778d) SHA1(a655e25dc9efdf60cc5b34e42c93c4acaa4a7922) )
	ROM_LOAD( "8.9k",         0x3000, 0x1000, CRC(b94fbb73) SHA1(5d676c5d1d864d70d98f0137c4072062a781b3a0) )
	ROM_LOAD( "9.9m",         0x4000, 0x1000, CRC(601f34ba) SHA1(1b7ee61a341b9a87abe4fe10b0c647a9b0b97d38) )
	ROM_LOAD( "10.9n",        0x5000, 0x1000, CRC(5f750918) SHA1(3dc44f259e88999dbb95b4d4376281cc81c1ab87) )
	ROM_LOAD( "13.8d",        0xc000, 0x0e00, CRC(944a4453) SHA1(cd64d9267d2c5cea39464ba9308752c690e7fd24) )
	ROM_CONTINUE(			  0xfe00, 0x0200 )
	ROM_LOAD( "14.8f",        0xd000, 0x0400, CRC(83b10139) SHA1(8a1880c6ab8a345676fe30465351d69cc1b416b2) )
	ROM_CONTINUE(			  0xe400, 0x0200 )
	ROM_CONTINUE(			  0xd600, 0x0a00 )
	ROM_LOAD( "15.8j",        0xe000, 0x0400, CRC(fe924879) SHA1(b80cbf9cba91e553f7685aef348854c02f0619c7) )
	ROM_CONTINUE(			  0xd400, 0x0200 )
	ROM_CONTINUE(			  0xe600, 0x0a00 )
	ROM_LOAD( "16.8k",        0xf000, 0x0e00, CRC(b77eb1f5) SHA1(ef94c1b449e3fa230491052fc3bd4db3f1239263) )
	ROM_CONTINUE(			  0xce00, 0x0200 )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "2.1e",         0x0000, 0x1000, CRC(f4d3d4e6) SHA1(167ad0259578966fe86384df844e69cf2cc77443) )
	ROM_LOAD( "4.1j",         0x1000, 0x1000, CRC(2c6a510d) SHA1(304064f11e80f4ec471174823b8aaf59844061ac) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "1.1c",         0x0000, 0x1000, CRC(a046ff44) SHA1(af319cfb74e5efe435c26e971de13bd390f4b378) )
	ROM_LOAD( "3.1f",         0x1000, 0x1000, CRC(a4422da4) SHA1(3aa55ca8c99566c1c9eb097b6d645c4216e09dfb) )

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "p3.bin",       0x0000, 0x0020, CRC(2a855523) SHA1(91e032233fee397c90b7c1662934aca9e0671482) )
	ROM_LOAD( "r3.bin",       0x0020, 0x0020, CRC(ae6f1019) SHA1(fd711882b670380cb4bd909c840ba06277b8fbe3) )

	ROM_REGION( 0x0060, "5110ctrl", 0)
	ROM_LOAD( "r6.bin",       0x0000, 0x0020, CRC(c58a4f6a) SHA1(35ef244b3e94032df2610aa594ea5670b91e1449) ) /*state machine driving TMS5110*/

	ROM_REGION( 0x2000, "tmsprom", 0 ) /* data for the TMS5110 speech chip */
	ROM_LOAD( "11.9r",        0x0000, 0x1000, CRC(2e0057ff) SHA1(33e3ffa6418f86864eb81e5e9bda4bf540c143a6) )
	ROM_LOAD( "12.9t",        0x1000, 0x1000, CRC(b2120edd) SHA1(52b89dbcc749b084331fa82b13d0876e911fce52) )
ROM_END

ROM_START( sbagmans )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sbag_9e.bin",  0x0000, 0x1000, CRC(c19696f2) SHA1(3a40202a97201a123033358f7afcb06f8ac15063) )
	ROM_LOAD( "6.9f",         0x1000, 0x1000, CRC(ac49cb82) SHA1(5affa0c03bedf2c9d5368c7f075818e1760c12ae) )
	ROM_LOAD( "7.9j",         0x2000, 0x1000, CRC(9a1c778d) SHA1(a655e25dc9efdf60cc5b34e42c93c4acaa4a7922) )
	ROM_LOAD( "8.9k",         0x3000, 0x1000, CRC(b94fbb73) SHA1(5d676c5d1d864d70d98f0137c4072062a781b3a0) )
	ROM_LOAD( "sbag_9m.bin",  0x4000, 0x1000, CRC(b21e246e) SHA1(39d2e93ac5240bb45e76c30c535d12e302690dde) )
	ROM_LOAD( "10.9n",        0x5000, 0x1000, CRC(5f750918) SHA1(3dc44f259e88999dbb95b4d4376281cc81c1ab87) )
	ROM_LOAD( "13.8d",        0xc000, 0x0e00, CRC(944a4453) SHA1(cd64d9267d2c5cea39464ba9308752c690e7fd24) )
	ROM_CONTINUE(			  0xfe00, 0x0200 )
	ROM_LOAD( "sbag_f8.bin",  0xd000, 0x0400, CRC(0f3e6de4) SHA1(a7e50d210630b500e534d626d76110dee4aeb18d) )
	ROM_CONTINUE(			  0xe400, 0x0200 )
	ROM_CONTINUE(			  0xd600, 0x0a00 )
	ROM_LOAD( "15.8j",        0xe000, 0x0400, CRC(fe924879) SHA1(b80cbf9cba91e553f7685aef348854c02f0619c7) )
	ROM_CONTINUE(			  0xd400, 0x0200 )
	ROM_CONTINUE(			  0xe600, 0x0a00 )
	ROM_LOAD( "16.8k",        0xf000, 0x0e00, CRC(b77eb1f5) SHA1(ef94c1b449e3fa230491052fc3bd4db3f1239263) )
	ROM_CONTINUE(			  0xce00, 0x0200 )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "2.1e",         0x0000, 0x1000, CRC(f4d3d4e6) SHA1(167ad0259578966fe86384df844e69cf2cc77443) )
	ROM_LOAD( "4.1j",         0x1000, 0x1000, CRC(2c6a510d) SHA1(304064f11e80f4ec471174823b8aaf59844061ac) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "sbag_1c.bin",  0x0000, 0x1000, CRC(262f870a) SHA1(90877b869a7e927cfa4f9729ec3d6eac3a95dc8f) )
	ROM_LOAD( "sbag_1f.bin",  0x1000, 0x1000, CRC(350ed0fb) SHA1(c7804e9618ebc88a1e3684a92a98d9a181441a1f) )

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "p3.bin",       0x0000, 0x0020, CRC(2a855523) SHA1(91e032233fee397c90b7c1662934aca9e0671482) )
	ROM_LOAD( "r3.bin",       0x0020, 0x0020, CRC(ae6f1019) SHA1(fd711882b670380cb4bd909c840ba06277b8fbe3) )

	ROM_REGION( 0x0060, "5110ctrl", 0)
	ROM_LOAD( "r6.bin",       0x0000, 0x0020, CRC(c58a4f6a) SHA1(35ef244b3e94032df2610aa594ea5670b91e1449) ) /*state machine driving TMS5110*/

	ROM_REGION( 0x2000, "tmsprom", 0 ) /* data for the TMS5110 speech chip */
	ROM_LOAD( "11.9r",        0x0000, 0x1000, CRC(2e0057ff) SHA1(33e3ffa6418f86864eb81e5e9bda4bf540c143a6) )
	ROM_LOAD( "12.9t",        0x1000, 0x1000, CRC(b2120edd) SHA1(52b89dbcc749b084331fa82b13d0876e911fce52) )
ROM_END

ROM_START( pickin )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "9e",           0x0000, 0x1000, CRC(efd0bd43) SHA1(b70a471a809c08286a82934046357fb46556f641) )
	ROM_LOAD( "9f",           0x1000, 0x1000, CRC(b5785a23) SHA1(9eddda5695981cb0470dfea68d5e2e8e220382b1) )
	ROM_LOAD( "9j",           0x2000, 0x1000, CRC(65ee9fd4) SHA1(2efa40c19a7b0644ef4f4b2ce6a025b2b880239d) )
	ROM_LOAD( "9k",           0x3000, 0x1000, CRC(7b23350e) SHA1(dff19602a0e46ca0bcdbdf2a1d61fd2c80ac70e7) )
	ROM_LOAD( "9m",           0x4000, 0x1000, CRC(935a7248) SHA1(d9af4405d51ce1ff6c4b84709dc85c0db88b1d54) )
	ROM_LOAD( "9n",           0x5000, 0x1000, CRC(52485d1d) SHA1(c309eec506f978388463f20d56d958e6639c31e8) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "1f",           0x0000, 0x1000, CRC(c5e96ac6) SHA1(b2d740b6d07c765e8eb2dce31fe285a15a9fe597) )
	ROM_LOAD( "1j",           0x1000, 0x1000, CRC(41c4ac1c) SHA1(aac58a9d675a9b70140d82341231bcf6c77c7b41) )

	/* no gfx2 */

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "6331-1.3p",    0x0000, 0x0020, CRC(fac81668) SHA1(5fa369a5c0ad3a2fc068305336e24772b8e84b62) )
	ROM_LOAD( "6331-1.3r",    0x0020, 0x0020, CRC(14ee1603) SHA1(f3c071399606b727ae7dd0bfc21e1c6ca2d43c7c) )
ROM_END

ROM_START( botanic )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bota_05.9e",    0x0000, 0x1000, CRC(cc66e6f8) SHA1(251481b16f8925a11f02f49e5a79f6524460aa6c) )
	ROM_LOAD( "bota_06.9f",    0x1000, 0x1000, CRC(59892f41) SHA1(eb01601a9163679560b878366aaf7cc0fb54a3e9) )
	ROM_LOAD( "bota_07.9j",    0x2000, 0x1000, CRC(b7c544ef) SHA1(75b5224c313e97c2c02ca7e9bc3f682278cb7a5c) )
	ROM_LOAD( "bota_08.9k",    0x3000, 0x1000, CRC(0afea479) SHA1(d69b2263b4ed09d8f4e40f379aa4a64187a75a52) )
	ROM_LOAD( "bota_09.9m",    0x4000, 0x1000, CRC(2da36120) SHA1(359d7747d8b7c7b4ce876fed722f19dc20e58b89) )
	ROM_LOAD( "bota_10.9n",    0x5000, 0x1000, CRC(7ce9fbc8) SHA1(cd2ba01470964640fad9ccf6ff23cbd76c0c2aeb) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "bota_02.1e",   0x0000, 0x1000, CRC(bea449a6) SHA1(fe06208996d15a4d50753fb62a3020063a0a6620) )
	ROM_LOAD( "bota_04.1j",   0x1000, 0x1000, CRC(a5deb8ed) SHA1(b6b38daffdda263a366656168a6d094ad2b1458f) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "bota_01.1c",    0x0000, 0x1000, CRC(a1148d89) SHA1(b1424693cebc410749216457d07bae54b903bc07) )
	ROM_LOAD( "bota_03.1f",    0x1000, 0x1000, CRC(70be5565) SHA1(a7eab667a82d3e7321f393073f29c6e5e865ec6b) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "bota_3p.3p",    0x0000, 0x0020, CRC(a8a2ddd2) SHA1(fc2da863d13e92f7682f393a08bc9357841ae7ea) )
	ROM_LOAD( "bota_3a.3a",    0x0020, 0x0020, CRC(edf88f34) SHA1(b9c342d51303d552f87df2543a34e38c30acd07c) )
ROM_END

/*

Squash (Itisa)

Anno    1984
Produttore  Itisa-Valadon-gecas

CPU

1x SGS Z8400AB1-Z80ACPU (main)
2x AY-3-8910 (sound)
1x LM380 (sound)
1x oscillator 18432
ROMs

7x 2732
2x MMI6331
Note

1x 22x2 edge connector
1x trimmer (volume)
1x 8 switches dip

This is a strange thing: the PCB is marked "Valadon Automation (C) 1983" and "Fabrique
sous license par GECAS/MILANO" (manufactured under license from GECAS/MILANO)

But if you look in rom 7 with an hex editor you can see the following: "(C) 1984 ITISA"
and "UN BONJOUR A JACQUES DE PEPE PETIT ET HENK" (a good morning to Jacques from Pepe
Petit and Henk). These are the programmers in ITISA, Henk Spits, Josep M. Petit, Josep
Morillas, the very same 3 persons working on BOTANIC (1984)(ITISA).

Game writings in the eprom are in English and Spanish.

So we have an English/Spanish game with a French easter egg on a French PCB manufactured
under license from an Italian company! Let's call it melting pot!

*/

ROM_START( squaitsa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sq5.3.9e",    0x0000, 0x1000,CRC(04128d92) SHA1(ca7b7c4be5f40bcefc92b231ce3bba859c9967ee) )
	ROM_LOAD( "sq6.4.9f",    0x1000, 0x1000,CRC(4ff7dd56) SHA1(1955675a9ee3ad7b9185cd027bc42284e15c7451) )
	ROM_LOAD( "sq7.5.9j",    0x2000, 0x1000,CRC(e46ecda6) SHA1(25cd94b6c9602cc00fe3459b524639fd3beb72be) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "sq2.1.1e",   0x0000, 0x1000,CRC(0eb6ecad) SHA1(da2facbfa5f2fe233ea09777e9880b4f1d3c1079) )
	ROM_LOAD( "sq4.2.1j",   0x1000, 0x1000,CRC(8d875b0e) SHA1(f949da71167aa81c1cfaefc6f3d88b57792b6191) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "sq1.1c",    0x0000, 0x1000,CRC(b6d563e5) SHA1(90a89fd8e892a612c74bd2c7e38acb08c22c6046) )
	ROM_LOAD( "sq3.1f",    0x1000, 0x1000,CRC(0d9d87e6) SHA1(881039d3b8805bb1a546e28abda3273e79714033) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "mmi6331.3p",    0x0000, 0x0020,CRC(06eab7ce) SHA1(d0bafedb340bf12d81446cc672307bb01e5d3026) )
	ROM_LOAD( "mmi6331.3r",    0x0020, 0x0020,CRC(86c1e7db) SHA1(5c974b51d770a555ddab5c23f03a666c6f286cbf) )
ROM_END

static DRIVER_INIT( bagnarda )
{
	/* initialize video enable because it's not done in the code */
	*bagman_video_enable = 1;
}

GAME( 1982, bagman,	   0,  bagman,  bagman,  0,        ROT270, "Valadon Automation", "Bagman", 0 )
GAME( 1982, bagnard,  bagman,  bagman,  bagman,  0,        ROT270, "Valadon Automation", "Le Bagnard (set 1)", 0 )
GAME( 1982, bagnarda, bagman,  bagman,  bagman,  bagnarda, ROT270, "Valadon Automation", "Le Bagnard (set 2)", 0 )
GAME( 1982, bagmans,  bagman,  bagman,  bagmans, 0,        ROT270, "Valadon Automation (Stern Electronics license)", "Bagman (Stern Electronics, set 1)", 0 )
GAME( 1982, bagmans2, bagman,  bagman,  bagman,  0,        ROT270, "Valadon Automation (Stern Electronics license)", "Bagman (Stern Electronics, set 2)", 0 )
GAME( 1984, sbagman,       0,  bagman,  sbagman, 0,        ROT270, "Valadon Automation", "Super Bagman", 0 )
GAME( 1984, sbagmans, sbagman, bagman,  sbagman, 0,        ROT270, "Valadon Automation (Stern Electronics license)", "Super Bagman (Stern Electronics)", 0 )
GAME( 1983, pickin,	   0,  pickin,  pickin,  0,        ROT270, "Valadon Automation", "Pickin'", 0 )
GAME( 1984, botanic,       0,  botanic, botanic, 0,        ROT270, "Valadon Automation (Itisa license)", "Botanic", 0 )
GAME( 1984, squaitsa,      0,  squaitsa,squaitsa,0,        ROT0,   "Itisa",              "Squash (Itisa)", 0 )

