/****************************************************************************

    Gunbuster                           (c) 1992 Taito

    Driver by Bryan McPhail & David Graves.

    Board Info:

        CPU   : 68EC020 68000
        SOUND : Ensoniq
        OSC.  : 40.000MHz 16.000MHz 30.47618MHz

        * This board (K11J0717A) uses following chips:
          - TC0470LIN
          - TC0480SCP
          - TC0570SPC
          - TC0260DAR
          - TC0510NIO

    Gunbuster uses a slightly enhanced sprite system from the one
    in Taito Z games.

    The key feature remains the use of a sprite map rom which allows
    the sprite hardware to create many large zoomed sprites on screen
    while minimizing the main cpu load.

    This feature makes the SZ system complementary to the F3 system
    which, owing to its F2 sprite hardware, is not very well suited to
    3d games. (Taito abandoned the SZ system once better 3d hardware
    platforms were available in the mid 1990s.)

    Gunbuster also uses the TC0480SCP tilemap chip (like the last Taito
    Z game, Double Axle).

    Todo:

        FLIPX support in taitoic.c is not quite correct - the Taito logo is wrong,
        and the floor in the Doom levels has horizontal scrolling where it shouldn't.

        No networked machine support

        Coin lockout not working (see gunbustr_input_w): perhaps this
        was a prototype version without proper coin handling?

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "video/taitoic.h"
#include "machine/eeprom.h"
#include "sound/es5506.h"
#include "includes/taito_f3.h"
#include "audio/taito_en.h"


VIDEO_START( gunbustr );
VIDEO_UPDATE( gunbustr );

static UINT16 coin_word;
static UINT32 *gunbustr_ram;

/*********************************************************************/

static TIMER_CALLBACK( gunbustr_interrupt5 )
{
	cputag_set_input_line(machine, "maincpu", 5, HOLD_LINE);
}

static INTERRUPT_GEN( gunbustr_interrupt )
{
	timer_set(device->machine, downcast<cpu_device *>(device)->cycles_to_attotime(200000-500), NULL, 0, gunbustr_interrupt5);
	cpu_set_input_line(device, 4, HOLD_LINE);
}

static WRITE32_HANDLER( gunbustr_palette_w )
{
	int a;
	COMBINE_DATA(&space->machine->generic.paletteram.u32[offset]);

	a = space->machine->generic.paletteram.u32[offset] >> 16;
	palette_set_color_rgb(space->machine,offset*2,pal5bit(a >> 10),pal5bit(a >> 5),pal5bit(a >> 0));

	a = space->machine->generic.paletteram.u32[offset] &0xffff;
	palette_set_color_rgb(space->machine,offset*2+1,pal5bit(a >> 10),pal5bit(a >> 5),pal5bit(a >> 0));
}

static CUSTOM_INPUT( coin_word_r )
{
	return coin_word;
}

static WRITE32_HANDLER( gunbustr_input_w )
{

#if 0
{
char t[64];
static UINT32 mem[2];
COMBINE_DATA(&mem[offset]);

sprintf(t,"%08x %08x",mem[0],mem[1]);
popmessage(t);
}
#endif

	switch (offset)
	{
		case 0x00:
		{
			if (ACCESSING_BITS_24_31)	/* $400000 is watchdog */
			{
				watchdog_reset(space->machine);
			}

			if (ACCESSING_BITS_0_7)
			{
				running_device *device = space->machine->device("eeprom");
				eeprom_set_clock_line(device, (data & 0x20) ? ASSERT_LINE : CLEAR_LINE);
				eeprom_write_bit(device, data & 0x40);
				eeprom_set_cs_line(device, (data & 0x10) ? CLEAR_LINE : ASSERT_LINE);
				return;
			}
			return;
		}

		case 0x01:
		{
			if (ACCESSING_BITS_24_31)
			{
				/* game does not write a separate counter for coin 2!
                   It should disable both coins when 9 credits reached
                   see code $1d8a-f6... but for some reason it's not */
				coin_lockout_w(space->machine, 0, data & 0x01000000);
				coin_lockout_w(space->machine, 1, data & 0x02000000);
				coin_counter_w(space->machine, 0, data & 0x04000000);
				coin_counter_w(space->machine, 1, data & 0x04000000);
				coin_word = (data >> 16) &0xffff;
			}
//logerror("CPU #0 PC %06x: write input %06x\n",cpu_get_pc(space->cpu),offset);
		}
	}
}

static WRITE32_HANDLER( motor_control_w )
{
/*
    Standard value poked into MSW is 0x3c00
    (0x2000 and zero are written at startup)

*/
	if (data & 0x1000000)
	{
	output_set_value("Player1_Gun_Recoil",1);
	}
	else
	{
	output_set_value("Player1_Gun_Recoil",0);
	}

	if (data & 0x10000)
	{
	output_set_value("Player2_Gun_Recoil",1);
	}
	else
	{
	output_set_value("Player2_Gun_Recoil",0);
	}

	if (data & 0x40000)
	{
	output_set_value("Hit_lamp",1);
	}
	else
	{
	output_set_value("Hit_lamp",0);
	}

}



static READ32_HANDLER( gunbustr_gun_r )
{
	return ( input_port_read(space->machine, "LIGHT0_X") << 24) | (input_port_read(space->machine, "LIGHT0_Y") << 16) |
		 ( input_port_read(space->machine, "LIGHT1_X") << 8)  |  input_port_read(space->machine, "LIGHT1_Y");
}

static WRITE32_HANDLER( gunbustr_gun_w )
{
	/* 10000 cycle delay is arbitrary */
	timer_set(space->machine, downcast<cpu_device *>(space->cpu)->cycles_to_attotime(10000), NULL, 0, gunbustr_interrupt5);
}


/***********************************************************
             MEMORY STRUCTURES
***********************************************************/

static ADDRESS_MAP_START( gunbustr_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x200000, 0x21ffff) AM_RAM AM_BASE(&gunbustr_ram)										/* main CPUA ram */
	AM_RANGE(0x300000, 0x301fff) AM_RAM AM_BASE_SIZE_GENERIC(spriteram)				/* Sprite ram */
	AM_RANGE(0x380000, 0x380003) AM_WRITE(motor_control_w)											/* motor, lamps etc. */
	AM_RANGE(0x390000, 0x3907ff) AM_RAM AM_BASE(&f3_shared_ram)										/* Sound shared ram */
	AM_RANGE(0x400000, 0x400003) AM_READ_PORT("P1_P2")
	AM_RANGE(0x400004, 0x400007) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x400000, 0x400007) AM_WRITE(gunbustr_input_w)											/* eerom etc. */
	AM_RANGE(0x500000, 0x500003) AM_READWRITE(gunbustr_gun_r, gunbustr_gun_w)						/* gun coord read */
	AM_RANGE(0x800000, 0x80ffff) AM_DEVREADWRITE("tc0480scp", tc0480scp_long_r, tc0480scp_long_w)
	AM_RANGE(0x830000, 0x83002f) AM_DEVREADWRITE("tc0480scp", tc0480scp_ctrl_long_r, tc0480scp_ctrl_long_w)
	AM_RANGE(0x900000, 0x901fff) AM_RAM_WRITE(gunbustr_palette_w) AM_BASE_GENERIC(paletteram)			/* Palette ram */
	AM_RANGE(0xc00000, 0xc03fff) AM_RAM																/* network ram ?? */
ADDRESS_MAP_END

/***********************************************************
             INPUT PORTS (dips in eprom)
***********************************************************/

static INPUT_PORTS_START( gunbustr )
	PORT_START("P1_P2")
	PORT_BIT( 0x00000001, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)	/* Freeze input */
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000080, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE("eeprom", eeprom_read_bit)
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)

	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_SERVICE_NO_TOGGLE( 0x00000001, IP_ACTIVE_LOW )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(coin_word_r, NULL)

	/* Light gun inputs */

	PORT_START("LIGHT0_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_SENSITIVITY(30) PORT_KEYDELTA(20) PORT_PLAYER(1)

	PORT_START("LIGHT0_Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_SENSITIVITY(30) PORT_KEYDELTA(20) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("LIGHT1_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_SENSITIVITY(30) PORT_KEYDELTA(20) PORT_PLAYER(2)

	PORT_START("LIGHT1_Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_SENSITIVITY(30) PORT_KEYDELTA(20) PORT_REVERSE PORT_PLAYER(2)
INPUT_PORTS_END


/***********************************************************
                GFX DECODING
**********************************************************/

static const gfx_layout tile16x16_layout =
{
	16,16,	/* 16*16 sprites */
	RGN_FRAC(1,1),
	4,	/* 4 bits per pixel */
	{ 0, 8, 16, 24 },
	{ 32, 33, 34, 35, 36, 37, 38, 39, 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*64, 1*64,  2*64,  3*64,  4*64,  5*64,  6*64,  7*64,
	  8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	64*16	/* every sprite takes 128 consecutive bytes */
};

static const gfx_layout charlayout =
{
	16,16,    /* 16*16 characters */
	RGN_FRAC(1,1),
	4,        /* 4 bits per pixel */
	{ 0, 1, 2, 3 },
	{ 1*4, 0*4, 5*4, 4*4, 3*4, 2*4, 7*4, 6*4, 9*4, 8*4, 13*4, 12*4, 11*4, 10*4, 15*4, 14*4 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64, 8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	128*8     /* every sprite takes 128 consecutive bytes */
};

static GFXDECODE_START( gunbustr )
	GFXDECODE_ENTRY( "gfx2", 0x0, tile16x16_layout,  0, 512 )
	GFXDECODE_ENTRY( "gfx1", 0x0, charlayout,        0, 512 )
GFXDECODE_END


/***********************************************************
                 MACHINE DRIVERS
***********************************************************/

static const eeprom_interface gunbustr_eeprom_interface =
{
	6,				/* address bits */
	16,				/* data bits */
	"0110",			/* read command */
	"0101",			/* write command */
	"0111",			/* erase command */
	"0100000000",	/* unlock command */
	"0100110000",	/* lock command */
};

static const tc0480scp_interface gunbustr_tc0480scp_intf =
{
	1, 2,		/* gfxnum, txnum */
	0,		/* pixels */
	0x20, 0x07,		/* x_offset, y_offset */
	-1, -1,		/* text_xoff, text_yoff */
	-1, 0,		/* flip_xoff, flip_yoff */
	0		/* col_base */
};

static MACHINE_DRIVER_START( gunbustr )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68EC020, 16000000)	/* 16 MHz */
	MDRV_CPU_PROGRAM_MAP(gunbustr_map)
	MDRV_CPU_VBLANK_INT("screen", gunbustr_interrupt) /* VBL */

	MDRV_EEPROM_ADD("eeprom", gunbustr_eeprom_interface)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(40*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0, 40*8-1, 2*8, 32*8-1)

	MDRV_GFXDECODE(gunbustr)
	MDRV_PALETTE_LENGTH(8192)

	MDRV_VIDEO_START(gunbustr)
	MDRV_VIDEO_UPDATE(gunbustr)

	MDRV_TC0480SCP_ADD("tc0480scp", gunbustr_tc0480scp_intf)

	/* sound hardware */
	MDRV_IMPORT_FROM(taito_f3_sound)
MACHINE_DRIVER_END

/***************************************************************************/

ROM_START( gunbustr )
	ROM_REGION( 0x100000, "maincpu", 0 )	/* 1024K for 68020 code (CPU A) */
	ROM_LOAD32_BYTE( "d27-23.bin", 0x00000, 0x40000, CRC(cd1037cc) SHA1(8005a6a84081ce609e7a605ec8e00e740bfc6846) )
	ROM_LOAD32_BYTE( "d27-22.bin", 0x00001, 0x40000, CRC(475949fc) SHA1(3d5aa3411d2618004902f9d05dff61d9af01ff35) )
	ROM_LOAD32_BYTE( "d27-21.bin", 0x00002, 0x40000, CRC(60950a8a) SHA1(a0336bf6970baa6eaa998a112db840a7fd0452d7) )
	ROM_LOAD32_BYTE( "d27-20.bin", 0x00003, 0x40000, CRC(13735c60) SHA1(65b762b28d51b295f6fe190420af566b1b3d4a82) )

	ROM_REGION( 0x140000, "audiocpu", 0 )	/* Sound cpu */
	ROM_LOAD16_BYTE( "d27-25.bin", 0x100000, 0x20000, CRC(c88203cf) SHA1(a918d395b471acdce56dacabd7a1e1e023948365) )
	ROM_LOAD16_BYTE( "d27-24.bin", 0x100001, 0x20000, CRC(084bd8bd) SHA1(93229bc7de4550ead1bb12f666ddbacbe357488d) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "d27-01.bin", 0x00000, 0x80000, CRC(f41759ce) SHA1(30789f43dd09b56399e1dfdb8c6a1e01a21562bd) )	/* SCR 16x16 tiles */
	ROM_LOAD16_BYTE( "d27-02.bin", 0x00001, 0x80000, CRC(92ab6430) SHA1(28ed80391c732b09d10c74ed6b78ac76cb62e083) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD32_BYTE( "d27-04.bin", 0x000003, 0x100000, CRC(ff8b9234) SHA1(6095b7daf9b7e9a22b0d44d9d6a642ddecb2bd29) )	/* OBJ 16x16 tiles: each rom has 1 bitplane */
	ROM_LOAD32_BYTE( "d27-05.bin", 0x000002, 0x100000, CRC(96d7c1a5) SHA1(93b6a7aea397280a5a778e736d433a85cb7da52c) )
	ROM_LOAD32_BYTE( "d27-06.bin", 0x000001, 0x100000, CRC(bbb934db) SHA1(9e9b5cf05b9275f1182f5b499b8ee897c4f25b96) )
	ROM_LOAD32_BYTE( "d27-07.bin", 0x000000, 0x100000, CRC(8ab4854e) SHA1(bd2750cdaa2918e56f8aef3732875952a1eeafea) )

	ROM_REGION16_LE( 0x80000, "user1", 0 )
	ROM_LOAD16_WORD( "d27-03.bin", 0x00000, 0x80000, CRC(23bf2000) SHA1(49b29e771a47fcd7e6cd4e2704b217f9727f8299) )	/* STY, used to create big sprites on the fly */

	ROM_REGION16_BE( 0x800000, "ensoniq.0" , ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "d27-08.bin", 0x000000, 0x100000, CRC(7c147e30) SHA1(b605045154967050ec06391798da4afe3686a6e1) ) // C8, C9
	ROM_RELOAD(0x400000,0x100000)
	ROM_LOAD16_BYTE( "d27-09.bin", 0x200000, 0x100000, CRC(3e060304) SHA1(c4da4a94c168c3a454409d758c3ed45babbab170) ) // CA, CB
	ROM_LOAD16_BYTE( "d27-10.bin", 0x600000, 0x100000, CRC(ed894fe1) SHA1(5bf2fb6abdcf25bc525a2c3b29dbf7aca0b18fea) ) // -std-

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-gunbustr.bin", 0x0000, 0x0080, CRC(af7dc017) SHA1(5ff106cccd2679025cdd81fbc133d32148e2818c) )
ROM_END

static READ32_HANDLER( main_cycle_r )
{
	if (cpu_get_pc(space->cpu)==0x55a && (gunbustr_ram[0x3acc/4]&0xff000000)==0)
		cpu_spinuntil_int(space->cpu);

	return gunbustr_ram[0x3acc/4];
}

static DRIVER_INIT( gunbustr )
{
	/* Speedup handler */
	memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x203acc, 0x203acf, 0, 0, main_cycle_r);
}

GAME( 1992, gunbustr, 0,      gunbustr, gunbustr, gunbustr, ORIENTATION_FLIP_X, "Taito Corporation", "Gunbuster (Japan)", 0 )
