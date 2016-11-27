/***************************************************************************

    Art & Magic hardware

    driver by Aaron Giles and Nicola Salmoria

    Games supported:
        * Cheese Chase
        * Ultimate Tennis
        * Stone Ball

    Known bugs:
        * measured against a real PCB, the games run slightly too fast
          in spite of accurately measured VBLANK timings

    DIP locations verified for:
        * ultennis (manual+test mode)
        * cheesech (test mode)
        * stonebal (test mode)
        * stoneba2 (test mode)

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/tms34010/tms34010.h"
#include "video/tlc34076.h"
#include "includes/artmagic.h"
#include "sound/okim6295.h"


#define MASTER_CLOCK_40MHz		(XTAL_40MHz)
#define MASTER_CLOCK_25MHz		(XTAL_25MHz)


static UINT16 *control;

static UINT8 tms_irq, hack_irq;

static UINT8 prot_input[16];
static UINT8 prot_input_index;
static UINT8 prot_output[16];
static UINT8 prot_output_index;
static UINT8 prot_output_bit;
static UINT8 prot_bit_index;
static UINT16 prot_save;

static void (*protection_handler)(running_machine *);



/*************************************
 *
 *  Interrupts
 *
 *************************************/

static void update_irq_state(running_machine *machine)
{
	cputag_set_input_line(machine, "maincpu", 4, tms_irq  ? ASSERT_LINE : CLEAR_LINE);
	cputag_set_input_line(machine, "maincpu", 5, hack_irq ? ASSERT_LINE : CLEAR_LINE);
}


static void m68k_gen_int(running_device *device, int state)
{
	tms_irq = state;
	update_irq_state(device->machine);
}



/*************************************
 *
 *  Machine init
 *
 *************************************/

static MACHINE_START( artmagic )
{
	state_save_register_global(machine, tms_irq);
	state_save_register_global(machine, hack_irq);
	state_save_register_global(machine, prot_input_index);
	state_save_register_global(machine, prot_output_index);
	state_save_register_global(machine, prot_output_bit);
	state_save_register_global(machine, prot_bit_index);
	state_save_register_global(machine, prot_save);
	state_save_register_global_array(machine, prot_input);
	state_save_register_global_array(machine, prot_output);
}

static MACHINE_RESET( artmagic )
{
	tms_irq = hack_irq = 0;
	update_irq_state(machine);
	tlc34076_reset(6);
}



/*************************************
 *
 *  TMS34010 interface
 *
 *************************************/

static READ16_HANDLER( tms_host_r )
{
	return tms34010_host_r(space->machine->device("tms"), offset);
}


static WRITE16_HANDLER( tms_host_w )
{
	tms34010_host_w(space->machine->device("tms"), offset, data);
}



/*************************************
 *
 *  Misc control memory accesses
 *
 *************************************/

static WRITE16_HANDLER( control_w )
{
	COMBINE_DATA(&control[offset]);

	/* OKI banking here */
	if (offset == 0)
	{
		okim6295_device *oki = space->machine->device<okim6295_device>("oki");
		oki->set_bank_base((((data >> 4) & 1) * 0x40000) % oki->region()->bytes());
	}

	logerror("%06X:control_w(%d) = %04X\n", cpu_get_pc(space->cpu), offset, data);
}



/*************************************
 *
 *  Ultimate Tennis protection workarounds
 *
 *************************************/

static TIMER_CALLBACK( irq_off )
{
	hack_irq = 0;
	update_irq_state(machine);
}

static READ16_HANDLER( ultennis_hack_r )
{
	/* IRQ5 points to: jsr (a5); rte */
	UINT32 pc = cpu_get_pc(space->cpu);
	if (pc == 0x18c2 || pc == 0x18e4)
	{
		hack_irq = 1;
		update_irq_state(space->machine);
		timer_set(space->machine, ATTOTIME_IN_USEC(1), NULL, 0, irq_off);
	}
	return input_port_read(space->machine, "300000");
}



/*************************************
 *
 *  Game-specific protection
 *
 *************************************/

static void ultennis_protection(running_machine *machine)
{
	/* check the command byte */
	switch (prot_input[0])
	{
		case 0x00:	/* reset */
			prot_input_index = prot_output_index = 0;
			prot_output[0] = mame_rand(machine);
			break;

		case 0x01:	/* 01 aaaa bbbb cccc dddd (xxxx) */
			if (prot_input_index == 9)
			{
				UINT16 a = prot_input[1] | (prot_input[2] << 8);
				UINT16 b = prot_input[3] | (prot_input[4] << 8);
				UINT16 c = prot_input[5] | (prot_input[6] << 8);
				UINT16 d = prot_input[7] | (prot_input[8] << 8);
				UINT16 x = a - b;
				if ((INT16)x >= 0)
					x = (x * c) >> 16;
				else
					x = -(((UINT16)-x * c) >> 16);
				x += d;
				prot_output[0] = x;
				prot_output[1] = x >> 8;
				prot_output_index = 0;
			}
			else if (prot_input_index >= 11)
				prot_input_index = 0;
			break;

		case 0x02:	/* 02 aaaa bbbb cccc (xxxxxxxx) */
			/*
                Ultimate Tennis -- actual values from a board:

                    hex                             decimal
                    0041 0084 00c8 -> 00044142       65 132 200 -> 278850 = 65*65*66
                    001e 0084 00fc -> 0000e808       30 132 252 ->  59400 = 30*30*66
                    0030 007c 005f -> 00022e00       48 124  95 -> 142848 = 48*48*62
                    0024 00dd 0061 -> 00022ce0       36 221  97 -> 142560 = 36*36*110
                    0025 0096 005b -> 00019113       37 150  91 -> 102675 = 37*37*75
                    0044 00c9 004c -> 00070e40       68 201  76 -> 462400 = 68*68*100

                question is: what is the 3rd value doing there?
            */
			if (prot_input_index == 7)
			{
				UINT16 a = (INT16)(prot_input[1] | (prot_input[2] << 8));
				UINT16 b = (INT16)(prot_input[3] | (prot_input[4] << 8));
				/*UINT16 c = (INT16)(prot_input[5] | (prot_input[6] << 8));*/
				UINT32 x = a * a * (b/2);
				prot_output[0] = x;
				prot_output[1] = x >> 8;
				prot_output[2] = x >> 16;
				prot_output[3] = x >> 24;
				prot_output_index = 0;
			}
			else if (prot_input_index >= 11)
				prot_input_index = 0;
			break;

		case 0x03:	/* 03 (xxxx) */
			if (prot_input_index == 1)
			{
				UINT16 x = prot_save;
				prot_output[0] = x;
				prot_output[1] = x >> 8;
				prot_output_index = 0;
			}
			else if (prot_input_index >= 3)
				prot_input_index = 0;
			break;

		case 0x04:	/* 04 aaaa */
			if (prot_input_index == 3)
			{
				UINT16 a = prot_input[1] | (prot_input[2] << 8);
				prot_save = a;
				prot_input_index = prot_output_index = 0;
			}
			break;

		default:
			logerror("protection command %02X: unknown\n", prot_input[0]);
			prot_input_index = prot_output_index = 0;
			break;
	}
}


static void cheesech_protection(running_machine *machine)
{
	/* check the command byte */
	switch (prot_input[0])
	{
		case 0x00:	/* reset */
			prot_input_index = prot_output_index = 0;
			prot_output[0] = mame_rand(machine);
			break;

		case 0x01:	/* 01 aaaa bbbb (xxxx) */
			if (prot_input_index == 5)
			{
				UINT16 a = prot_input[1] | (prot_input[2] << 8);
				UINT16 b = prot_input[3] | (prot_input[4] << 8);
				UINT16 c = 0x4000;		/* seems to be hard-coded */
				UINT16 d = 0x00a0;		/* seems to be hard-coded */
				UINT16 x = a - b;
				if ((INT16)x >= 0)
					x = (x * c) >> 16;
				else
					x = -(((UINT16)-x * c) >> 16);
				x += d;
				prot_output[0] = x;
				prot_output[1] = x >> 8;
				prot_output_index = 0;
			}
			else if (prot_input_index >= 7)
				prot_input_index = 0;
			break;

		case 0x03:	/* 03 (xxxx) */
			if (prot_input_index == 1)
			{
				UINT16 x = prot_save;
				prot_output[0] = x;
				prot_output[1] = x >> 8;
				prot_output_index = 0;
			}
			else if (prot_input_index >= 3)
				prot_input_index = 0;
			break;

		case 0x04:	/* 04 aaaa */
			if (prot_input_index == 3)
			{
				UINT16 a = prot_input[1] | (prot_input[2] << 8);
				prot_save = a;
				prot_input_index = prot_output_index = 0;
			}
			break;

		default:
			logerror("protection command %02X: unknown\n", prot_input[0]);
			prot_input_index = prot_output_index = 0;
			break;
	}
}


static void stonebal_protection(running_machine *machine)
{
	/* check the command byte */
	switch (prot_input[0])
	{
		case 0x01:	/* 01 aaaa bbbb cccc dddd (xxxx) */
			if (prot_input_index == 9)
			{
				UINT16 a = prot_input[1] | (prot_input[2] << 8);
				UINT16 b = prot_input[3] | (prot_input[4] << 8);
				UINT16 c = prot_input[5] | (prot_input[6] << 8);
				UINT16 d = prot_input[7] | (prot_input[8] << 8);
				UINT16 x = a - b;
				if ((INT16)x >= 0)
					x = (x * d) >> 16;
				else
					x = -(((UINT16)-x * d) >> 16);
				x += c;
				prot_output[0] = x;
				prot_output[1] = x >> 8;
				prot_output_index = 0;
			}
			else if (prot_input_index >= 11)
				prot_input_index = 0;
			break;

		case 0x02:	/* 02 aaaa (xx) */
			if (prot_input_index == 3)
			{
				/*UINT16 a = prot_input[1] | (prot_input[2] << 8);*/
				UINT8 x = 0xa5;
				prot_output[0] = x;
				prot_output_index = 0;
			}
			else if (prot_input_index >= 4)
				prot_input_index = 0;
			break;

		case 0x03:	/* 03 (xxxx) */
			if (prot_input_index == 1)
			{
				UINT16 x = prot_save;
				prot_output[0] = x;
				prot_output[1] = x >> 8;
				prot_output_index = 0;
			}
			else if (prot_input_index >= 3)
				prot_input_index = 0;
			break;

		case 0x04:	/* 04 aaaa */
			if (prot_input_index == 3)
			{
				UINT16 a = prot_input[1] | (prot_input[2] << 8);
				prot_save = a;
				prot_input_index = prot_output_index = 0;
			}
			break;

		default:
			logerror("protection command %02X: unknown\n", prot_input[0]);
			prot_input_index = prot_output_index = 0;
			break;
	}
}


static CUSTOM_INPUT( prot_r )
{
	return prot_output_bit;
}


static WRITE16_HANDLER( protection_bit_w )
{
	/* shift in the new bit based on the offset */
	prot_input[prot_input_index] <<= 1;
	prot_input[prot_input_index] |= offset;

	/* clock out the next bit based on the offset */
	prot_output_bit = prot_output[prot_output_index] & 0x01;
	prot_output[prot_output_index] >>= 1;

	/* are we done with a whole byte? */
	if (++prot_bit_index == 8)
	{
		/* add the data and process it */
		prot_input_index++;
		prot_output_index++;
		prot_bit_index = 0;

		/* update the protection state */
		(*protection_handler)(space->machine);
	}
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x220000, 0x23ffff) AM_RAM
	AM_RANGE(0x240000, 0x240fff) AM_RAM AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE(0x300000, 0x300001) AM_READ_PORT("300000")
	AM_RANGE(0x300002, 0x300003) AM_READ_PORT("300002")
	AM_RANGE(0x300004, 0x300005) AM_READ_PORT("300004")
	AM_RANGE(0x300006, 0x300007) AM_READ_PORT("300006")
	AM_RANGE(0x300008, 0x300009) AM_READ_PORT("300008")
	AM_RANGE(0x30000a, 0x30000b) AM_READ_PORT("30000a")
	AM_RANGE(0x300000, 0x300003) AM_WRITE(control_w) AM_BASE(&control)
	AM_RANGE(0x300004, 0x300007) AM_WRITE(protection_bit_w)
	AM_RANGE(0x360000, 0x360001) AM_DEVREADWRITE8("oki", okim6295_r, okim6295_w, 0x00ff)
	AM_RANGE(0x380000, 0x380007) AM_READWRITE(tms_host_r, tms_host_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( stonebal_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x200000, 0x27ffff) AM_RAM
	AM_RANGE(0x280000, 0x280fff) AM_RAM AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE(0x300000, 0x300001) AM_READ_PORT("300000")
	AM_RANGE(0x300002, 0x300003) AM_READ_PORT("300002")
	AM_RANGE(0x300004, 0x300005) AM_READ_PORT("300004")
	AM_RANGE(0x300006, 0x300007) AM_READ_PORT("300006")
	AM_RANGE(0x300008, 0x300009) AM_READ_PORT("300008")
	AM_RANGE(0x30000a, 0x30000b) AM_READ_PORT("30000a")
	AM_RANGE(0x30000c, 0x30000d) AM_READ_PORT("30000c")
	AM_RANGE(0x30000e, 0x30000f) AM_READ_PORT("30000e")
	AM_RANGE(0x300000, 0x300003) AM_WRITE(control_w) AM_BASE(&control)
	AM_RANGE(0x300004, 0x300007) AM_WRITE(protection_bit_w)
	AM_RANGE(0x340000, 0x340001) AM_DEVREADWRITE8("oki", okim6295_r, okim6295_w, 0x00ff)
	AM_RANGE(0x380000, 0x380007) AM_READWRITE(tms_host_r, tms_host_w)
ADDRESS_MAP_END



/*************************************
 *
 *  Slave CPU memory handlers
 *
 *************************************/

static const tms34010_config tms_config =
{
	TRUE,							/* halt on reset */
	"screen",						/* the screen operated on */
	MASTER_CLOCK_40MHz/6,			/* pixel clock */
	1,								/* pixels per clock */
	artmagic_scanline,				/* scanline update */
	m68k_gen_int,					/* generate interrupt */
	artmagic_to_shiftreg,			/* write to shiftreg function */
	artmagic_from_shiftreg			/* read from shiftreg function */
};


static ADDRESS_MAP_START( tms_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x00000000, 0x001fffff) AM_RAM AM_BASE(&artmagic_vram0)
	AM_RANGE(0x00400000, 0x005fffff) AM_RAM AM_BASE(&artmagic_vram1)
	AM_RANGE(0x00800000, 0x0080007f) AM_READWRITE(artmagic_blitter_r, artmagic_blitter_w)
	AM_RANGE(0x00c00000, 0x00c000ff) AM_READWRITE(tlc34076_lsb_r, tlc34076_lsb_w)
	AM_RANGE(0xc0000000, 0xc00001ff) AM_READWRITE(tms34010_io_register_r, tms34010_io_register_w)
	AM_RANGE(0xffe00000, 0xffffffff) AM_RAM
ADDRESS_MAP_END


static ADDRESS_MAP_START( stonebal_tms_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x00000000, 0x001fffff) AM_RAM AM_BASE(&artmagic_vram0)
	AM_RANGE(0x00400000, 0x005fffff) AM_RAM AM_BASE(&artmagic_vram1)
	AM_RANGE(0x00800000, 0x0080007f) AM_READWRITE(artmagic_blitter_r, artmagic_blitter_w)
	AM_RANGE(0x00c00000, 0x00c000ff) AM_READWRITE(tlc34076_lsb_r, tlc34076_lsb_w)
	AM_RANGE(0xc0000000, 0xc00001ff) AM_READWRITE(tms34010_io_register_r, tms34010_io_register_w)
	AM_RANGE(0xffc00000, 0xffffffff) AM_RAM
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( cheesech )
	PORT_START("300000")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("300002")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("300004")
	PORT_DIPUNUSED_DIPLOC( 0x0001, 0x0001, "SWB:8" )		/* Listed as "Unused" */
	PORT_DIPNAME( 0x0006, 0x0004, DEF_STR( Language ) )		PORT_DIPLOCATION("SWB:6,7")
	PORT_DIPSETTING(      0x0000, DEF_STR( French ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Italian ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( English ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( German ) )
	PORT_DIPNAME( 0x0018, 0x0018, DEF_STR( Lives ))			PORT_DIPLOCATION("SWB:4,5")
	PORT_DIPSETTING(      0x0008, "3" )
	PORT_DIPSETTING(      0x0018, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPSETTING(      0x0010, "6" )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Demo_Sounds ))	PORT_DIPLOCATION("SWB:3")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x00c0, 0x0040, DEF_STR( Difficulty ))	PORT_DIPLOCATION("SWB:1,2")
	PORT_DIPSETTING(      0x00c0, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("300006")
	PORT_DIPNAME( 0x0007, 0x0007, "Right Coinage" )		PORT_DIPLOCATION("SWA:6,7,8")
	PORT_DIPSETTING(      0x0002, DEF_STR( 6C_1C ))
	PORT_DIPSETTING(      0x0006, DEF_STR( 5C_1C ))
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ))
	PORT_DIPSETTING(      0x0005, DEF_STR( 3C_1C ))
	PORT_DIPSETTING(      0x0003, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_4C ))
	PORT_DIPNAME( 0x0038, 0x0038, "Left Coinage"  )		PORT_DIPLOCATION("SWA:3,4,5")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ))
	PORT_DIPSETTING(      0x0020, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_5C ))
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_6C ))
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Free_Play ))	PORT_DIPLOCATION("SWA:2")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SWA:1" )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("300008")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_COIN4 )
	PORT_BIT( 0x00f0, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("30000a")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(prot_r, NULL)	/* protection data */
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_SPECIAL )		/* protection ready */
	PORT_BIT( 0x00fc, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( ultennis )
	PORT_INCLUDE(cheesech)

	PORT_MODIFY("300004")
	PORT_DIPNAME( 0x0001, 0x0001, "Button Layout" )			PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(      0x0001, "Triangular" )
	PORT_DIPSETTING(      0x0000, "Linear" )
	PORT_DIPNAME( 0x0002, 0x0002, "Start Set At" )			PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(      0x0000, "0-0" )
	PORT_DIPSETTING(      0x0002, "4-4" )
	PORT_DIPNAME( 0x0004, 0x0004, "Sets Per Match" )		PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(      0x0004, "1" )
	PORT_DIPSETTING(      0x0000, "3" )
	PORT_DIPNAME( 0x0018, 0x0008, "Game Duratiob" )			PORT_DIPLOCATION("SWB:4,5")
	PORT_DIPSETTING(      0x0018, "5 Lost Points" )
	PORT_DIPSETTING(      0x0008, "6 Lost Points" )
	PORT_DIPSETTING(      0x0010, "7 Lost Points" )
	PORT_DIPSETTING(      0x0000, "8 Lost Points" )
INPUT_PORTS_END


static INPUT_PORTS_START( stonebal )
	PORT_INCLUDE(cheesech)

	PORT_MODIFY("300004")
	PORT_SERVICE_DIPLOC(  0x0001, IP_ACTIVE_LOW, "SWA:1" )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Free_Play ))		PORT_DIPLOCATION("SWA:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x001c, 0x001c, "Left Coinage" )			PORT_DIPLOCATION("SWA:3,4,5")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ))
	PORT_DIPSETTING(      0x0004, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(      0x001c, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(      0x0014, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_5C ))
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_6C ))
	PORT_DIPNAME( 0x00e0, 0x00e0, "Right Coinage" )			PORT_DIPLOCATION("SWA:6,7,8")
	PORT_DIPSETTING(      0x0040, DEF_STR( 6C_1C ))
	PORT_DIPSETTING(      0x0060, DEF_STR( 5C_1C ))
	PORT_DIPSETTING(      0x0080, DEF_STR( 4C_1C ))
	PORT_DIPSETTING(      0x00a0, DEF_STR( 3C_1C ))
	PORT_DIPSETTING(      0x00c0, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(      0x00e0, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_4C ))

	PORT_MODIFY("300006")
	PORT_DIPNAME( 0x0003, 0x0002, DEF_STR( Difficulty ))	PORT_DIPLOCATION("SWB:1,2")
	PORT_DIPSETTING(      0x0003, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x0004, 0x0000, DEF_STR( Demo_Sounds ))	PORT_DIPLOCATION("SWB:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0038, 0x0038, "Match Time" )			PORT_DIPLOCATION("SWB:4,5,6")
	PORT_DIPSETTING(      0x0030, "60s" )
	PORT_DIPSETTING(      0x0028, "70s" )
	PORT_DIPSETTING(      0x0020, "80s" )
	PORT_DIPSETTING(      0x0018, "90s" )
	PORT_DIPSETTING(      0x0038, "100s" )
	PORT_DIPSETTING(      0x0010, "110s" )
	PORT_DIPSETTING(      0x0008, "120s" )
	PORT_DIPSETTING(      0x0000, "130s" )
	PORT_DIPNAME( 0x0040, 0x0040, "Free Match Time" )		PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, "Short" )
	PORT_DIPNAME( 0x0080, 0x0080, "Game Mode" )				PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(      0x0080, "4 Players" )
	PORT_DIPSETTING(      0x0000, "2 Players" )

	PORT_START("30000c")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(3)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(3)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_START3 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("30000e")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(4)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(4)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(4)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_START4 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( stoneba2 )
	PORT_INCLUDE(stonebal)

	PORT_MODIFY("300006")
	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0080, "SWB:8" )		/* Listed as "Unused" */
INPUT_PORTS_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_DRIVER_START( artmagic )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, MASTER_CLOCK_25MHz/2)
	MDRV_CPU_PROGRAM_MAP(main_map)

	MDRV_CPU_ADD("tms", TMS34010, MASTER_CLOCK_40MHz)
	MDRV_CPU_CONFIG(tms_config)
	MDRV_CPU_PROGRAM_MAP(tms_map)

	MDRV_MACHINE_START(artmagic)
	MDRV_MACHINE_RESET(artmagic)
	MDRV_QUANTUM_TIME(HZ(6000))
	MDRV_NVRAM_HANDLER(generic_1fill)

	/* video hardware */
	MDRV_VIDEO_START(artmagic)
	MDRV_VIDEO_UPDATE(tms340x0)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_RAW_PARAMS(MASTER_CLOCK_40MHz/6, 428, 0, 320, 313, 0, 256)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_OKIM6295_ADD("oki", MASTER_CLOCK_40MHz/3/10, OKIM6295_PIN7_LOW)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.65)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( cheesech )
	MDRV_IMPORT_FROM(artmagic)

	MDRV_SOUND_MODIFY("oki")
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( stonebal )
	MDRV_IMPORT_FROM(artmagic)

	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(stonebal_map)

	MDRV_CPU_MODIFY("tms")
	MDRV_CPU_PROGRAM_MAP(stonebal_tms_map)

	MDRV_SOUND_MODIFY("oki")
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.45)
MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( cheesech )
	ROM_REGION( 0x80000, "maincpu", 0 )	/* 64k for 68000 code */
	ROM_LOAD16_BYTE( "u102",     0x00000, 0x40000, CRC(1d6e07c5) SHA1(8650868cce47f685d22131aa28aad45033cb0a52) )
	ROM_LOAD16_BYTE( "u101",     0x00001, 0x40000, CRC(30ae9f95) SHA1(fede5d271aabb654c1efc077253d81ba23786f22) )

	ROM_REGION16_LE( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "u134", 0x00000, 0x80000, CRC(354ba4a6) SHA1(68e7df750efb21c716ba8b8ed4ca15a8cdc9141b) )
	ROM_LOAD16_BYTE( "u135", 0x00001, 0x80000, CRC(97348681) SHA1(7e74685041cd5e8fbd45731284cf316dc3ffec60) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "u151", 0x00000, 0x80000, CRC(65d5ebdb) SHA1(0d905b9a60b86e51de3bdcf6eeb059fe29606431) )
ROM_END


ROM_START( ultennis )
	ROM_REGION( 0x80000, "maincpu", 0 )	/* 64k for 68000 code */
	ROM_LOAD16_BYTE( "utu102.bin", 0x00000, 0x40000, CRC(ec31385e) SHA1(244e78619c549712d5541fb252656afeba639bb7) )
	ROM_LOAD16_BYTE( "utu101.bin", 0x00001, 0x40000, CRC(08a7f655) SHA1(b8a4265472360b68bed71d6c175fc54dff088c1d) )

	ROM_REGION16_LE( 0x200000, "gfx1", 0 )
	ROM_LOAD( "utu133.bin", 0x000000, 0x200000, CRC(29d9204d) SHA1(0b2b77a55b8c2877c2e31b63156505584d4ee1f0) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "utu151.bin", 0x00000,  0x40000, CRC(4e19ca89) SHA1(ac7e17631ec653f83c4912df6f458b0e1df88096) )
ROM_END


ROM_START( ultennisj )
	ROM_REGION( 0x80000, "maincpu", 0 )	/* 64k for 68000 code */
	ROM_LOAD16_BYTE( "a&m001d0194-13c-u102-japan.u102", 0x00000, 0x40000, CRC(65cee452) SHA1(49259e8faf289d6d80769f6d44e9d61d15e431c6) )
	ROM_LOAD16_BYTE( "a&m001d0194-12c-u101-japan.u101", 0x00001, 0x40000, CRC(5f4b0ca0) SHA1(57e9ed60cc0e53eeb4e08c4003138d3bdaec3de7) )

	ROM_REGION16_LE( 0x200000, "gfx1", 0 )
	ROM_LOAD( "a&m-001-01-a.ic133", 0x000000, 0x200000, CRC(29d9204d) SHA1(0b2b77a55b8c2877c2e31b63156505584d4ee1f0) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "a&m001c1293-14a-u151.u151", 0x00000,  0x40000, CRC(4e19ca89) SHA1(ac7e17631ec653f83c4912df6f458b0e1df88096) )
ROM_END


/*
Stone Ball
Art & Magic, 1994

PCB No: AM007B1094 0454
CPUs  : TMS34010FNL-40, MC68000P12
SND   : OKI M6295
OSC   : 40.000MHz, 32.000MHz, 25.000MHz
DIP SW: 8 position (x2)

RAM   : MCM54260 (x2, 40 pin SOJ)
        TMS44251 (x4, 28 pin ZIP)

OTHER :
        CSI CAT28C16 EEPROM (24 pin DIP)
        ADV476KN80E (28 pin DIP)
        8 PALs
        1 PIC 16C54
        Black Box - inside is....
                                 XILINX XC3030 (x2, 84 Pin PLCC)
                                 3V Battery (Suicidal?)
                                 74HC14 Logic Chip
                                 10 Pin header (probably for re-programming the XC3030's
                                               after it suicides....)


ROMs  :
                             Byte
Filename      Type           C'sum
---------------------------------------------------
u1801.bin     27C4001        344Eh     OKI Samples

u101.bin      27C2001        617Ah  \  Main Program
u102.bin      27C2001        8F04h  /

u1600.bin     32M Mask       1105h  \
u1601.bin     32M Mask       8642h  /  Gfx

*/

ROM_START( stonebal )
	ROM_REGION( 0x80000, "maincpu", 0 )	/* 64k for 68000 code */
	ROM_LOAD16_BYTE( "u102",     0x00000, 0x40000, CRC(712feda1) SHA1(c5b385f425786566fa274fe166a7116615a8ce86) )
	ROM_LOAD16_BYTE( "u101",     0x00001, 0x40000, CRC(4f1656a9) SHA1(720717ae4166b3ec50bb572197a8c6c96b284648) )

	ROM_REGION16_LE( 0x400000, "gfx1", 0 )
	ROM_LOAD( "u1600.bin", 0x000000, 0x200000, CRC(d2ffe9ff) SHA1(1c5dcbd8208e45458da9db7621f6b8602bca0fae) )
	ROM_LOAD( "u1601.bin", 0x200000, 0x200000, CRC(dbe893f0) SHA1(71a8a022decc0ff7d4c65f7e6e0cbba9e0b5582c) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "u1801.bin", 0x00000, 0x80000, CRC(d98f7378) SHA1(700df7f29c039b96791c2704a67f01a722dc96dc) )
ROM_END


ROM_START( stonebal2 )
	ROM_REGION( 0x80000, "maincpu", 0 )	/* 64k for 68000 code */
	ROM_LOAD16_BYTE( "u102.bin", 0x00000, 0x40000, CRC(b3c4f64f) SHA1(6327e9f3cd9deb871a6910cf1f006c8ee143e859) )
	ROM_LOAD16_BYTE( "u101.bin", 0x00001, 0x40000, CRC(fe373f74) SHA1(bafac4bbd1aae4ccc4ae16205309483f1bbdd464) )

	ROM_REGION16_LE( 0x400000, "gfx1", 0 )
	ROM_LOAD( "u1600.bin", 0x000000, 0x200000, CRC(d2ffe9ff) SHA1(1c5dcbd8208e45458da9db7621f6b8602bca0fae) )
	ROM_LOAD( "u1601.bin", 0x200000, 0x200000, CRC(dbe893f0) SHA1(71a8a022decc0ff7d4c65f7e6e0cbba9e0b5582c) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "u1801.bin", 0x00000, 0x80000, CRC(d98f7378) SHA1(700df7f29c039b96791c2704a67f01a722dc96dc) )
ROM_END



/*************************************
 *
 *  Driver initialization
 *
 *************************************/

static void decrypt_ultennis(void)
{
	int i;

	/* set up the parameters for the blitter data decryption which will happen at runtime */
	for (i = 0;i < 16;i++)
	{
		artmagic_xor[i] = 0x0462;
		if (i & 1) artmagic_xor[i] ^= 0x0011;
		if (i & 2) artmagic_xor[i] ^= 0x2200;
		if (i & 4) artmagic_xor[i] ^= 0x4004;
		if (i & 8) artmagic_xor[i] ^= 0x0880;
	}
}


static void decrypt_cheesech(void)
{
	int i;

	/* set up the parameters for the blitter data decryption which will happen at runtime */
	for (i = 0;i < 16;i++)
	{
		artmagic_xor[i] = 0x0891;
		if (i & 1) artmagic_xor[i] ^= 0x1100;
		if (i & 2) artmagic_xor[i] ^= 0x0022;
		if (i & 4) artmagic_xor[i] ^= 0x0440;
		if (i & 8) artmagic_xor[i] ^= 0x8008;
	}
}


static DRIVER_INIT( ultennis )
{
	decrypt_ultennis();
	artmagic_is_stoneball = 0;
	protection_handler = ultennis_protection;

	/* additional (protection?) hack */
	memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x300000, 0x300001, 0, 0, ultennis_hack_r);
}


static DRIVER_INIT( cheesech )
{
	decrypt_cheesech();
	artmagic_is_stoneball = 0;
	protection_handler = cheesech_protection;
}


static DRIVER_INIT( stonebal )
{
	decrypt_ultennis();
	artmagic_is_stoneball = 1;	/* blits 1 line high are NOT encrypted, also different first pixel decrypt */
	protection_handler = stonebal_protection;
}



/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1993, ultennis, 0,        artmagic, ultennis, ultennis, ROT0, "Art & Magic", "Ultimate Tennis", GAME_SUPPORTS_SAVE )
GAME( 1993, ultennisj,ultennis, artmagic, ultennis, ultennis, ROT0, "Art & Magic (Banpresto license)", "Ultimate Tennis (v 1.4, Japan)", GAME_SUPPORTS_SAVE )
GAME( 1994, cheesech, 0,        cheesech, cheesech, cheesech, ROT0, "Art & Magic", "Cheese Chase", GAME_SUPPORTS_SAVE )
GAME( 1994, stonebal, 0,        stonebal, stonebal, stonebal, ROT0, "Art & Magic", "Stone Ball (4 Players)", GAME_SUPPORTS_SAVE )
GAME( 1994, stonebal2,stonebal, stonebal, stoneba2, stonebal, ROT0, "Art & Magic", "Stone Ball (2 Players)", GAME_SUPPORTS_SAVE )
