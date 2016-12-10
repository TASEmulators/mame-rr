/***************************************************************************

    Ground Effects / Super Ground FX                    (c) 1993 Taito

    Driver by Bryan McPhail & David Graves.

    Board Info:

    K1100744A
    J1100316A
    Sticker - K11J0744A
    ------------------------------------------------------------------------------------------------
    | 2018  2088        43256      43256   MC68EC020RP25   DIP-SW (8) COM20020  SLIDE-SW  LAN-IN   |           Connector G
    | 2018  2088        D51-21     D51-22              D51-14                                      |           -----------
    | 2018  2088        43256      43256               (PAL16L8)                                   |       Parts    |   Solder
    |                                                  D51-15              ADC0809        LAN-OUT  |       ------------------
    | 2018              D51-23     D51-24              (PAL16L8)                                   |        GND    1 A   GND
    |                                                  D51-16-1                          JP4       |        GND    2 B   GND
    | 2018  TC0570SPC                                  (PAL16L8)            EEPROM.164   1-2       |        +12    3 C   +12
    | 2018                                                                               2-3       |         +5    4 D   +5
    |                                                                 TC0510NIO          2-3     --|         +5    5 E   +5
    |    D51-13       D51-03                                                             2-3     |           +5    6 F   +5
    |                 D51-04                             43256                           1-2     |                 7 H
    |    TC0470LIN    D51-05                                                             1-2     --|        +13    8 J   +13
    |                 D51-06                             43256   TC0650FCA               1-2       |        +13    9 K   +13
    |                 D51-07                                                             2-3       |      SPK_R+  10 L  SPK_R-
    |    TC0580PIV              43256                    43256      2018                 2-3       |      SPK_L+  11 M  SPK_L-
    |                           43256     D51-17                                                   |   SPK_REAR-  12 N  SPK_REAR-
    | 514256          TC0480SCP           (PAL16L8)                                                |              13 P
    | 514256                                                                                      G|         RED  14 R  GREEN
    | 514256                              D51-10                                                   |        BLUE  15 S  SYNC
    | 514256   D51-09                     D51-11          TC0620SCC                                |       V-GND  16 T
    | 514256   D51-08                     D51012                                                   |      METER1  17 U  METER2
    | 514256          43256                                                                        |    LOCKOUT1  18 V  LOCKOUT2
    |          MB8421                                        43256                                 |              19 W
    | D51-18   MB8421             ENSONIC                                                        --|       COIN1  20 X  COIN2
    | (PAL20L8)       D51-29      OTISR2                     43256      MB87078                  |       SERVICE  21 Y  TEST
    |                         D51-01                                                             |                22 Z
    | D51-19          43256           ENSONIC     ENSONIC                                        --|    SHIFT UP  23 a  BRAKE
    | (PAL20L8)               D51-02  SUPER GLU   ESP-R6    TC511664    TL074  TL074               |   HANDLE VR  24 b  ACCEL VR
    |                 D51-30                                                                       |    SHIFT DN  25 c
    | MC68000P12F16MHz                                                  TDA1543 TDA1543            |              26 d
    |                 40MHz    16MHz  30.476MHz    MC68681                                         |         GND  27 e  GND
    | D51-20                                                                                       |         GND  28 f  GND
    | (PAL20L8)                                                                                    |
    ------------------------------------------------------------------------------------------------


    Ground Effects combines the sprite system used in Taito Z games with
    the TC0480SCP tilemap chip plus some features from the Taito F3 system.
    It has an extra tilemap chip which is a dead ringer for the TC0100SCN
    (check the inits), like Under Fire.

    Ground Effects is effectively a 30Hz game - though the vblank interrupts
    still come in at 60Hz, the game uses a hardware frame counter to limit
    itself to 30Hz (it only updates things like the video registers every
    other vblank).  There isn't enough cpu power in a 16MHz 68020 to run
    this game at 60Hz.

    Ground Effects has a network mode - probably uses IRQ 6 and the unknown
    ports at 0xc00000.

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "video/taitoic.h"
#include "machine/eeprom.h"
#include "sound/es5506.h"
#include "includes/taito_f3.h"
#include "audio/taito_en.h"

VIDEO_START( groundfx );
VIDEO_UPDATE( groundfx );

static UINT16 coin_word, frame_counter=0;
static UINT16 port_sel = 0;
extern UINT16 groundfx_rotate_ctrl[8];
static UINT32 *groundfx_ram;

/***********************************************************
                COLOR RAM

Extract a standard version of this
("taito_8bpg_palette_word_w"?) to Taitoic.c ?
***********************************************************/

static WRITE32_HANDLER( color_ram_w )
{
	int a,r,g,b;
	COMBINE_DATA(&space->machine->generic.paletteram.u32[offset]);

	{
		a = space->machine->generic.paletteram.u32[offset];
		r = (a &0xff0000) >> 16;
		g = (a &0xff00) >> 8;
		b = (a &0xff);

		palette_set_color(space->machine,offset,MAKE_RGB(r,g,b));
	}
}


/***********************************************************
                INTERRUPTS
***********************************************************/

static TIMER_CALLBACK( groundfx_interrupt5 )
{
	cputag_set_input_line(machine, "maincpu", 5, HOLD_LINE); //from 5... ADC port
}


/**********************************************************
                EPROM
**********************************************************/

static const eeprom_interface groundfx_eeprom_interface =
{
	6,				/* address bits */
	16,				/* data bits */
	"0110",			/* read command */
	"0101",			/* write command */
	"0111",			/* erase command */
	"0100000000",	/* unlock command */
	"0100110000",	/* lock command */
};


/**********************************************************
            GAME INPUTS
**********************************************************/

static CUSTOM_INPUT( frame_counter_r )
{
	return frame_counter;
}

static CUSTOM_INPUT( coin_word_r )
{
	return coin_word;
}

static WRITE32_HANDLER( groundfx_input_w )
{
	switch (offset)
	{
		case 0x00:
			if (ACCESSING_BITS_24_31)	/* $500000 is watchdog */
			{
				watchdog_reset(space->machine);
			}

			if (ACCESSING_BITS_0_7)
			{
				input_port_write(space->machine, "EEPROMOUT", data, 0xff);
			}

			break;

		case 0x01:
			if (ACCESSING_BITS_24_31)
			{
				coin_lockout_w(space->machine, 0,~data & 0x01000000);
				coin_lockout_w(space->machine, 1,~data & 0x02000000);
				coin_counter_w(space->machine, 0, data & 0x04000000);
				coin_counter_w(space->machine, 1, data & 0x08000000);
				coin_word = (data >> 16) &0xffff;
			}
			break;
	}
}

static READ32_HANDLER( groundfx_adc_r )
{
	return (input_port_read(space->machine, "AN0") << 8) | input_port_read(space->machine, "AN1");
}

static WRITE32_HANDLER( groundfx_adc_w )
{
	/* One interrupt per input port (4 per frame, though only 2 used).
        1000 cycle delay is arbitrary */
	timer_set(space->machine, downcast<cpu_device *>(space->cpu)->cycles_to_attotime(1000), NULL, 0, groundfx_interrupt5);
}

static WRITE32_HANDLER( rotate_control_w )	/* only a guess that it's rotation */
{
		if (ACCESSING_BITS_0_15)
		{
			groundfx_rotate_ctrl[port_sel] = data;
			return;
		}

		if (ACCESSING_BITS_16_31)
		{
			port_sel = (data &0x70000) >> 16;
		}
}


static WRITE32_HANDLER( motor_control_w )
{
/*
    Standard value poked is 0x00910200 (we ignore lsb and msb
    which seem to be always zero)

    0x0, 0x8000 and 0x9100 are written at startup

    Two bits are written in test mode to this middle word
    to test gun vibration:

    ........ .x......   P1 gun vibration
    ........ x.......   P2 gun vibration
*/
}

/***********************************************************
             MEMORY STRUCTURES
***********************************************************/

static ADDRESS_MAP_START( groundfx_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM
	AM_RANGE(0x200000, 0x21ffff) AM_RAM	AM_BASE(&groundfx_ram) /* main CPUA ram */
	AM_RANGE(0x300000, 0x303fff) AM_RAM	AM_BASE_SIZE_GENERIC(spriteram) /* sprite ram */
	AM_RANGE(0x400000, 0x400003) AM_WRITE(motor_control_w)	/* gun vibration */
	AM_RANGE(0x500000, 0x500003) AM_READ_PORT("BUTTONS")
	AM_RANGE(0x500004, 0x500007) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x500000, 0x500007) AM_WRITE(groundfx_input_w)	/* eeprom etc. */
	AM_RANGE(0x600000, 0x600003) AM_READWRITE(groundfx_adc_r,groundfx_adc_w)
	AM_RANGE(0x700000, 0x7007ff) AM_RAM AM_BASE(&f3_shared_ram)
	AM_RANGE(0x800000, 0x80ffff) AM_DEVREADWRITE("tc0480scp", tc0480scp_long_r, tc0480scp_long_w)	  /* tilemaps */
	AM_RANGE(0x830000, 0x83002f) AM_DEVREADWRITE("tc0480scp", tc0480scp_ctrl_long_r, tc0480scp_ctrl_long_w)	// debugging
	AM_RANGE(0x900000, 0x90ffff) AM_DEVREADWRITE("tc0100scn", tc0100scn_long_r, tc0100scn_long_w)	/* piv tilemaps */
	AM_RANGE(0x920000, 0x92000f) AM_DEVREADWRITE("tc0100scn", tc0100scn_ctrl_long_r, tc0100scn_ctrl_long_w)
	AM_RANGE(0xa00000, 0xa0ffff) AM_RAM_WRITE(color_ram_w) AM_BASE_GENERIC(paletteram) /* palette ram */
	AM_RANGE(0xb00000, 0xb003ff) AM_RAM						// ?? single bytes, blending ??
	AM_RANGE(0xc00000, 0xc00007) AM_READNOP /* Network? */
	AM_RANGE(0xd00000, 0xd00003) AM_WRITE(rotate_control_w)	/* perhaps port based rotate control? */
	/* f00000 is seat control? */
ADDRESS_MAP_END

/***********************************************************
             INPUT PORTS (dips in eprom)
***********************************************************/

static INPUT_PORTS_START( groundfx )
	PORT_START("BUTTONS")
	PORT_BIT( 0x00000001, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(frame_counter_r, NULL)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000080, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE("eeprom", eeprom_read_bit)
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_BUTTON3 )		/* shift hi */
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_BUTTON1 )		/* brake */
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_BUTTON2 )		/* shift low */
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_set_cs_line)
	PORT_BIT( 0x00000020, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_set_clock_line)
	PORT_BIT( 0x00000040, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_write_bit)

	PORT_START("SYSTEM")
	PORT_SERVICE_NO_TOGGLE( 0x00000001, IP_ACTIVE_LOW )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(coin_word_r, NULL)

	PORT_START("AN0")	/* IN 2, steering wheel */
	PORT_BIT( 0xff, 0x7f, IPT_AD_STICK_X ) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("AN1")	/* IN 3, accel */
	PORT_BIT( 0xff, 0xff, IPT_AD_STICK_Y ) PORT_SENSITIVITY(20) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("AN2")	/* IN 4, sound volume */
	PORT_BIT( 0xff, 0x00, IPT_AD_STICK_X ) PORT_SENSITIVITY(20) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(2)

	PORT_START("AN3")	/* IN 5, unknown */
	PORT_BIT( 0xff, 0x00, IPT_AD_STICK_Y ) PORT_SENSITIVITY(20) PORT_KEYDELTA(10) PORT_PLAYER(2)
INPUT_PORTS_END

/**********************************************************
                GFX DECODING
**********************************************************/

static const gfx_layout tile16x16_layout =
{
	16,16,	/* 16*16 sprites */
	RGN_FRAC(1,2),
	5,	/* 5 bits per pixel */
	{ RGN_FRAC(1,2), 0, 8, 16, 24 },
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

static const gfx_layout pivlayout =
{
	8,8,    /* 8*8 characters */
	RGN_FRAC(1,2),
	6,      /* 4 bits per pixel */
	{ RGN_FRAC(1,2), RGN_FRAC(1,2)+1, 0, 1, 2, 3 },
	{ 2*4, 3*4, 0*4, 1*4, 6*4, 7*4, 4*4, 5*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8    /* every sprite takes 32 consecutive bytes */
};

static GFXDECODE_START( groundfx )
	GFXDECODE_ENTRY( "gfx2", 0x0, tile16x16_layout,  4096, 512 )
	GFXDECODE_ENTRY( "gfx1", 0x0, charlayout,        0, 512 )
	GFXDECODE_ENTRY( "gfx3", 0x0, pivlayout,         0, 512 )
GFXDECODE_END


/***********************************************************
                 MACHINE DRIVERS
***********************************************************/

static const tc0100scn_interface groundfx_tc0100scn_intf =
{
	"screen",
	2, 3,		/* gfxnum, txnum */
	50, 8,		/* x_offset, y_offset */
	0, 0,		/* flip_xoff, flip_yoff */
	0, 0,		/* flip_text_xoff, flip_text_yoff */
	0, 0
};

static const tc0480scp_interface groundfx_tc0480scp_intf =
{
	1, 4,		/* gfxnum, txnum */
	0,		/* pixels */
	0x24, 0,		/* x_offset, y_offset */
	-1, 0,		/* text_xoff, text_yoff */
	0, 0,		/* flip_xoff, flip_yoff */
	0		/* col_base */
};

static INTERRUPT_GEN( groundfx_interrupt )
{
	frame_counter^=1;
	cpu_set_input_line(device, 4, HOLD_LINE);
}

static MACHINE_DRIVER_START( groundfx )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68EC020, 16000000)	/* 16 MHz */
	MDRV_CPU_PROGRAM_MAP(groundfx_map)
	MDRV_CPU_VBLANK_INT("screen", groundfx_interrupt)

	MDRV_EEPROM_ADD("eeprom", groundfx_eeprom_interface)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(40*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0, 40*8-1, 3*8, 32*8-1)

	MDRV_GFXDECODE(groundfx)
	MDRV_PALETTE_LENGTH(16384)

	MDRV_VIDEO_START(groundfx)
	MDRV_VIDEO_UPDATE(groundfx)

	MDRV_TC0100SCN_ADD("tc0100scn", groundfx_tc0100scn_intf)
	MDRV_TC0480SCP_ADD("tc0480scp", groundfx_tc0480scp_intf)

	/* sound hardware */
	MDRV_IMPORT_FROM(taito_f3_sound)
MACHINE_DRIVER_END

/***************************************************************************
                    DRIVERS
***************************************************************************/

ROM_START( groundfx )
	ROM_REGION( 0x200000, "maincpu", 0 )	/* 2048K for 68020 code (CPU A) */
	ROM_LOAD32_BYTE( "d51-24.79", 0x00000, 0x80000, CRC(5caaa031) SHA1(03e727e26df701e3f5e16c5f933d5b29a528945a) )
	ROM_LOAD32_BYTE( "d51-23.61", 0x00001, 0x80000, CRC(462e3c9b) SHA1(7f116ee755748497b911868a948d3e3b5134e475) )
	ROM_LOAD32_BYTE( "d51-22.77", 0x00002, 0x80000, CRC(b6b04d88) SHA1(58685ee8fd788dcbfe318f1e3c06d93e2128034c) )
	ROM_LOAD32_BYTE( "d51-21.59", 0x00003, 0x80000, CRC(21ecde2b) SHA1(c6d3738f34c8e24346e7784b14aeff300ae2d225) )

	ROM_REGION( 0x180000, "audiocpu", 0 )
	ROM_LOAD16_BYTE( "d51-29.54", 0x100000, 0x40000,  CRC(4b64f41d) SHA1(040427668d13f7320d23805098d6d0e1aa8d121e) )
	ROM_LOAD16_BYTE( "d51-30.56", 0x100001, 0x40000,  CRC(45f339fe) SHA1(cc7adfb2b86070f5bb426542e3b7ed2a50b3c39e) )

	ROM_REGION( 0x400000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "d51-08.35", 0x000000, 0x200000, CRC(835b7a0f) SHA1(0131fceabd73b0045b5d4ae0bb2f03efdd407962) )	/* SCR 16x16 tiles */
	ROM_LOAD16_BYTE( "d51-09.34", 0x000001, 0x200000, CRC(6dabd83d) SHA1(3dbd7ea36b9900faa6420af1f1600efe295db74c) )

	ROM_REGION( 0x1000000, "gfx2", 0 )
	ROM_LOAD32_BYTE( "d51-03.47", 0x800000, 0x200000, CRC(629a5c99) SHA1(cfc1c0b07ecefd6eddb83edcbcf710e8b8de19e4) )	/* OBJ 16x16 tiles */
	ROM_LOAD32_BYTE( "d51-04.48", 0x000000, 0x200000, CRC(f49b14b7) SHA1(31129771159c1295a074c8311344ece525302289) )
	ROM_LOAD32_BYTE( "d51-05.49", 0x000001, 0x200000, CRC(3a2e2cbf) SHA1(ed2c1ca9211b1d70b4767a54e08263a3e4867199) )
	ROM_LOAD32_BYTE( "d51-06.50", 0x000002, 0x200000, CRC(d33ce2a0) SHA1(92c4504344672ea798cd6dd34f4b46848bf9f82b) )
	ROM_LOAD32_BYTE( "d51-07.51", 0x000003, 0x200000, CRC(24b2f97d) SHA1(6980e67b435d189ce897c0301e0411763410ab47) )

	ROM_REGION( 0x400000, "gfx3", 0 )
	ROM_LOAD16_BYTE( "d51-10.95", 0x000000, 0x100000, CRC(d5910604) SHA1(8efe13884cfdef208394ddfe19f43eb1b9f78ff3) )	/* PIV 8x8 tiles, 6bpp */
	ROM_LOAD16_BYTE( "d51-11.96", 0x000001, 0x100000, CRC(fee5f5c6) SHA1(1be88747f9c71c348dd61a8f0040007df3a3e6a6) )
	ROM_LOAD       ( "d51-12.97", 0x300000, 0x100000, CRC(d630287b) SHA1(2fa09e1821b7280d193ca9a2a270759c3c3189d1) )
	ROM_FILL       (              0x200000, 0x100000, 0 )

	ROM_REGION16_LE( 0x80000, "user1", 0 )
	ROM_LOAD16_WORD( "d51-13.7", 0x00000,  0x80000,  CRC(36921b8b) SHA1(2130120f78a3b984618a53054fc937cf727177b9) )	/* STY, spritemap */

	ROM_REGION16_BE( 0x1000000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "d51-01.73", 0x000000, 0x200000, CRC(92f09155) SHA1(8015e1997818bb480174394eb43840bf26679bcf) )	/* Ensoniq samples */
	ROM_LOAD16_BYTE( "d51-02.74", 0xc00000, 0x200000, CRC(20a9428f) SHA1(c9033d02a49c72f704808f5f899101617d5814e5) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46.164", 0x0000, 0x0080, CRC(6f58851d) SHA1(33bd4478f097dca6b5d222adb89699c6d35ed009) )
ROM_END


static READ32_HANDLER( irq_speedup_r_groundfx )
{
	cpu_device *cpu = downcast<cpu_device *>(space->cpu);
	int ptr;
	offs_t sp = cpu->sp();
	if ((sp&2)==0) ptr=groundfx_ram[(sp&0x1ffff)/4];
	else ptr=(((groundfx_ram[(sp&0x1ffff)/4])&0x1ffff)<<16) |
	(groundfx_ram[((sp&0x1ffff)/4)+1]>>16);

	if (cpu->pc()==0x1ece && ptr==0x1b9a)
		cpu->spin_until_interrupt();

	return groundfx_ram[0xb574/4];
}


static DRIVER_INIT( groundfx )
{
	UINT32 offset,i;
	UINT8 *gfx = memory_region(machine, "gfx3");
	int size=memory_region_length(machine, "gfx3");
	int data;

	/* Speedup handlers */
	memory_install_read32_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x20b574, 0x20b577, 0, 0, irq_speedup_r_groundfx);

	/* make piv tile GFX format suitable for gfxdecode */
	offset = size/2;
	for (i = size/2+size/4; i<size; i++)
	{
		int d1,d2,d3,d4;

		/* Expand 2bits into 4bits format */
		data = gfx[i];
		d1 = (data>>0) & 3;
		d2 = (data>>2) & 3;
		d3 = (data>>4) & 3;
		d4 = (data>>6) & 3;

		gfx[offset] = (d1<<2) | (d2<<6);
		offset++;

		gfx[offset] = (d3<<2) | (d4<<6);
		offset++;
	}
}


GAME( 1992, groundfx, 0, groundfx, groundfx, groundfx, ROT0, "Taito Corporation", "Ground Effects / Super Ground Effects (Japan)", 0 )
