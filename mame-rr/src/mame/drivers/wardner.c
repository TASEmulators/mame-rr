/***************************************************************************

        ToaPlan game hardware from 1987
        --------------------------------
        Driver by: Quench


Supported games:

    Toaplan Board Number:   TP-009
    Taito Game Number:      B25
        Wardners Forest (World)
        Pyros           (USA)
        Wardna no Mori  (Japan)

Notes:
        Basically the same video and machine hardware as Flying shark,
          except for the Main CPU which is a Z80 here.
        See twincobr.c machine and video drivers to complete the
          hardware setup.
        To enter the "test mode", press START1 when the grid is displayed.
        Press F1 (initially P1 button 3) on startup to skip some video RAM tests
        (code at 0x6d25 in 'wardner', 0x6d2f in 'wardnerj' or 0x6d2c in 'pyros').

**************************** Memory & I/O Maps *****************************
Z80:(0)  Main CPU
0000-6fff Main ROM
7000-7fff Main RAM
8000-ffff Level and scenery ROMS. This is banked with the following
8000-8fff Sprite RAM
a000-adff Pallette RAM
ae00-afff Spare unused, but tested Pallette RAM
c000-c7ff Sound RAM - shared with C000-C7FF in Z80(1) RAM

in:
50      DSW A
52      DSW B
54      Player 1 controls
56      Player 2 controls
58      VBlank (bit 7) and coin-in/start inputs
60      LSB data from char display layer
61      MSB data from char display layer
62      LSB data from BG   display layer
63      MSB data from BG   display layer
64      LSB data from FG   display layer
65      MSB data from FG   display layer

out:
00      6845 CRTC offset register
02      6845 CRTC register data
10      char scroll LSB   < Y >
11      char scroll MSB   < Y >
12      char scroll LSB     X
13      char scroll MSB     X
14      char LSB RAM offset     20h * 40h  (0-07ff) and (4000-47ff) ???
15      char MSB RAM offset
20      BG   scroll LSB   < Y >
21      BG   scroll MSB   < Y >
22      BG   scroll LSB     X
23      BG   scroll MSB     X
24      BG   LSB RAM offset     40h * 40h  (0-0fff)
25      BG   MSB RAM offset
30      FG   scroll LSB   < Y >
31      FG   scroll MSB   < Y >
32      FG   scroll LSB     X
33      FG   scroll MSB     X
34      FG   LSB RAM offset     40h * 40h  (0-0fff)
35      FG   MSB RAM offset
40      spare scroll LSB  < Y >  (Not used)
41      spare scroll MSB  < Y >  (Not used)
5a-5c   Control registers
        bits 7-4 always 0
        bits 3-1 select the control signal to drive.
        bit   0  is the value passed to the control signal.
5a      data
        00-01   INT line to TMS320C10 DSP (Active low trigger)
        0c-0d   lockout for coin A input (Active low lockout)
        0e-0f   lockout for coin B input (Active low lockout)
5c      data
        00-01   ???
        02-03   ???
        04-05   Active low INTerrupt to Z80(0) for screen refresh
        06-07   Flip Screen (Active high flips)
        08-09   Background RAM display bank switch
        0a-0b   Foreground ROM display bank switch (not used here)
        0c-0d   ??? (what the hell does this do ?)
60      LSB data to char display layer
61      MSB data to char display layer
62      LSB data to BG   display layer
63      MSB data to BG   display layer
64      LSB data to FG   display layer
65      MSB data to FG   display layer
70      ROM bank selector for Z80(0) address 8000-ffff
        data
        00  switch ROM from 8000-ffff out, and put sprite/palette/sound RAM back.
        02  switch lower half of B25-18.ROM  ROM to 8000-ffff
        03  switch upper half of B25-18.ROM  ROM to 8000-ffff
        04  switch lower half of B25-19.ROM  ROM to 8000-ffff
        05  switch upper half of B25-19.ROM  ROM to 8000-ffff
        07  switch               B25-30.ROM  ROM to 8000-ffff



Z80:(1)  Sound CPU
0000-7fff Main ROM
8000-807f RAM ???
c000-cfff Sound RAM, $C000-C7FF shared with $C000-C7FF in Z80(0) ram



TMS320C10 DSP: Harvard type architecture. RAM and ROM on seperate data buses.
0000-05ff ROM 16-bit opcodes (word access only).
0000-0090 Internal RAM (words).

in:
01      data read from addressed Z80:(0) address space (Main RAM/Sprite RAM)

out:
00      address of Z80:(0) to read/write to
01      data to write to addressed Z80:(0) address space (Main RAM/Sprite RAM)
03      bit 15 goes to BIO line of TMS320C10. BIO is a polled input line.


***************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/tms32010/tms32010.h"
#include "includes/toaplipt.h"
#include "includes/twincobr.h"
#include "sound/3812intf.h"



static UINT8 *rambase_ae00, *rambase_c000;

static WRITE8_HANDLER( wardner_ramrom_bank_sw )
{
	if (wardner_membank != data) {
		int bankaddress = 0;

		const address_space *mainspace;
		UINT8 *RAM = memory_region(space->machine, "maincpu");

		mainspace = cputag_get_address_space(space->machine, "maincpu", ADDRESS_SPACE_PROGRAM);
		wardner_membank = data;

		if (data)
		{
			memory_install_read_bank(mainspace, 0x8000, 0xffff, 0, 0, "bank1");
			switch (data)
			{
				case 2:  bankaddress = 0x10000; break;
				case 3:  bankaddress = 0x18000; break;
				case 4:  bankaddress = 0x20000; break;
				case 5:  bankaddress = 0x28000; break;
				case 7:  bankaddress = 0x38000; break;
				case 1:  bankaddress = 0x08000; break; /* not used */
				case 6:  bankaddress = 0x30000; break; /* not used */
				default: bankaddress = 0x00000; break; /* not used */
			}
			memory_set_bankptr(space->machine, "bank1",&RAM[bankaddress]);
		}
		else
		{
			memory_install_read8_handler(mainspace, 0x8000, 0x8fff, 0, 0, wardner_sprite_r);
			memory_install_read_bank(mainspace, 0xa000, 0xadff, 0, 0, "bank4");
			memory_install_read_bank(mainspace, 0xae00, 0xafff, 0, 0, "bank2");
			memory_install_read_bank(mainspace, 0xc000, 0xc7ff, 0, 0, "bank3");
			memory_set_bankptr(space->machine, "bank1", &RAM[0x0000]);
			memory_set_bankptr(space->machine, "bank2", rambase_ae00);
			memory_set_bankptr(space->machine, "bank3", rambase_c000);
			memory_set_bankptr(space->machine, "bank4", space->machine->generic.paletteram.v);
		}
	}
}

STATE_POSTLOAD( wardner_restore_bank )
{
	const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);

	wardner_ramrom_bank_sw(space,0,1);	/* Dummy value to ensure restoration */
	wardner_ramrom_bank_sw(space,0,wardner_membank);
}


/***************************** Z80 Main Memory Map **************************/

static ADDRESS_MAP_START( main_program_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x6fff) AM_ROM
	AM_RANGE(0x7000, 0x7fff) AM_RAM

	AM_RANGE(0x8000, 0xffff) AM_ROMBANK("bank1") /* Overlapped RAM/Banked ROM - See below */

	AM_RANGE(0x8000, 0x8fff) AM_WRITE(wardner_sprite_w) AM_BASE_SIZE_GENERIC(spriteram)
	AM_RANGE(0x9000, 0x9fff) AM_ROM
	AM_RANGE(0xa000, 0xadff) AM_WRITE(paletteram_xBBBBBGGGGGRRRRR_le_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0xae00, 0xafff) AM_RAM AM_BASE(&rambase_ae00)
	AM_RANGE(0xb000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM AM_BASE(&rambase_c000) AM_SHARE("share1")	/* Shared RAM with Sound Z80 */
	AM_RANGE(0xc800, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( main_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_DEVWRITE("crtc", mc6845_address_w)
	AM_RANGE(0x02, 0x02) AM_DEVWRITE("crtc", mc6845_register_w)
	AM_RANGE(0x10, 0x13) AM_WRITE(wardner_txscroll_w)		/* scroll text layer */
	AM_RANGE(0x14, 0x15) AM_WRITE(wardner_txlayer_w)		/* offset in text video RAM */
	AM_RANGE(0x20, 0x23) AM_WRITE(wardner_bgscroll_w)		/* scroll bg layer */
	AM_RANGE(0x24, 0x25) AM_WRITE(wardner_bglayer_w)		/* offset in bg video RAM */
	AM_RANGE(0x30, 0x33) AM_WRITE(wardner_fgscroll_w)		/* scroll fg layer */
	AM_RANGE(0x34, 0x35) AM_WRITE(wardner_fglayer_w)		/* offset in fg video RAM */
	AM_RANGE(0x40, 0x43) AM_WRITE(wardner_exscroll_w)		/* scroll extra layer (not used) */
	AM_RANGE(0x50, 0x50) AM_READ_PORT("DSWA")
	AM_RANGE(0x52, 0x52) AM_READ_PORT("DSWB")
	AM_RANGE(0x54, 0x54) AM_READ_PORT("P1")
	AM_RANGE(0x56, 0x56) AM_READ_PORT("P2")
	AM_RANGE(0x58, 0x58) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x5a, 0x5a) AM_WRITE(wardner_coin_dsp_w)		/* Machine system control */
	AM_RANGE(0x5c, 0x5c) AM_WRITE(wardner_control_w)		/* Machine system control */
	AM_RANGE(0x60, 0x65) AM_READWRITE(wardner_videoram_r, wardner_videoram_w)		/* data from video layer RAM */
	AM_RANGE(0x70, 0x70) AM_WRITE(wardner_ramrom_bank_sw)	/* ROM bank select */
ADDRESS_MAP_END


/***************************** Z80 Sound Memory Map *************************/

static ADDRESS_MAP_START( sound_program_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x807f) AM_RAM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM AM_SHARE("share1")	/* Shared RAM with Main Z80 */
	AM_RANGE(0xc800, 0xcfff) AM_RAM

ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVREADWRITE("ymsnd", ym3812_r, ym3812_w)
ADDRESS_MAP_END


/***************************** TMS32010 Memory Map **************************/

static ADDRESS_MAP_START( DSP_program_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000, 0x5ff) AM_ROM
ADDRESS_MAP_END

	/* $000 - 08F  TMS32010 Internal Data RAM in Data Address Space */

static ADDRESS_MAP_START( DSP_io_map, ADDRESS_SPACE_IO, 16 )
	AM_RANGE(0, 0) AM_WRITE(wardner_dsp_addrsel_w)
	AM_RANGE(1, 1) AM_READWRITE(wardner_dsp_r, wardner_dsp_w)
	AM_RANGE(3, 3) AM_WRITE(twincobr_dsp_bio_w)
	AM_RANGE(TMS32010_BIO, TMS32010_BIO) AM_READ(twincobr_BIO_r)
ADDRESS_MAP_END


/*****************************************************************************

    Input Port definitions

*****************************************************************************/

/* verified from Z80 code */
static INPUT_PORTS_START( wardner_generic )
	PORT_START("DSWA")
	TOAPLAN_MACHINE_COCKTAIL
	TOAPLAN_COINAGE_WORLD

	PORT_START("DSWB")
	TOAPLAN_DIFFICULTY
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ) )       /* table at 0x13ce ('wardner') or 0x13de ('wardnerj') */
	PORT_DIPSETTING(	0x00, "30k 80k 50k+" )
	PORT_DIPSETTING(	0x04, "50k 100k 50k+" )
	PORT_DIPSETTING(	0x08, "30k Only" )
	PORT_DIPSETTING(	0x0c, "50k Only" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(	0x30, "1" )
	PORT_DIPSETTING(	0x00, "3" )
	PORT_DIPSETTING(	0x10, "4" )
	PORT_DIPSETTING(	0x20, "5" )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_HIGH )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_HIGH )

	PORT_START("P1")
	TOAPLAN_JOY_UDLR_2_BUTTONS( 1 )                         /* buttons 3 & 4 named "SHOTC" and "SHOTD" in "test mode" */

	PORT_START("P2")
	TOAPLAN_JOY_UDLR_2_BUTTONS( 2 )                         /* buttons 3 & 4 named "SHOTC" and "SHOTD" in "test mode" */

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )            /* "TEST" in "test mode" - no effect outside */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_VBLANK )            /* "V-BLANKING" in "test mode" */
INPUT_PORTS_END

/* verified from Z80 code */
static INPUT_PORTS_START( wardner )
	PORT_INCLUDE( wardner_generic )

	PORT_MODIFY("P1")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Skip Video RAM Tests") PORT_CODE(KEYCODE_F1)
	/* code at 0x6d25 ('wardner'), 0x6d2f ('wardnerj') or 0x6d2c ('pyros') */
INPUT_PORTS_END

/* verified from Z80 code */
static INPUT_PORTS_START( wardnerj )
	PORT_INCLUDE( wardner )

	PORT_MODIFY("DSWA")
	TOAPLAN_COINAGE_JAPAN_OLD
INPUT_PORTS_END

/* verified from Z80 code */
static INPUT_PORTS_START( pyros )
	PORT_INCLUDE( wardnerj )

	PORT_MODIFY("DSWB")
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ) )       /* table at 0x13ce */
	PORT_DIPSETTING(	0x00, "30k 80k 50k+" )
	PORT_DIPSETTING(	0x04, "50k 100k 50k+" )
	PORT_DIPSETTING(	0x08, "50k Only" )
	PORT_DIPSETTING(	0x0c, "100k Only" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Allow_Continue ) )   /* additional code at 0x6037 */
	PORT_DIPSETTING(	0x40, DEF_STR( No ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Yes ) )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,	/* 8*8 characters */
	2048,	/* 2048 characters */
	3,		/* 3 bits per pixel */
	{ 0*2048*8*8, 1*2048*8*8, 2*2048*8*8 }, /* the bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8		/* every char takes 8 consecutive bytes */
};

static const gfx_layout tilelayout =
{
	8,8,	/* 8*8 tiles */
	4096,	/* 4096 tiles */
	4,		/* 4 bits per pixel */
	{ 0*4096*8*8, 1*4096*8*8, 2*4096*8*8, 3*4096*8*8 }, /* the bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8		/* every tile takes 8 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	16,16,	/* 16*16 sprites */
	2048,	/* 2048 sprites */
	4,		/* 4 bits per pixel */
	{ 0*2048*32*8, 1*2048*32*8, 2*2048*32*8, 3*2048*32*8 }, /* the bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	32*8	/* every sprite takes 32 consecutive bytes */
};


/* handler called by the 3812 emulator when the internal timers cause an IRQ */
static void irqhandler(running_device *device, int linestate)
{
	cputag_set_input_line(device->machine, "audiocpu", 0, linestate);
}

static const ym3812_interface ym3812_config =
{
	irqhandler
};


static GFXDECODE_START( wardner )
	GFXDECODE_ENTRY( "gfx1", 0x00000, charlayout,   1536, 32 )	/* colors 1536-1791 */
	GFXDECODE_ENTRY( "gfx2", 0x00000, tilelayout,   1280, 16 )	/* colors 1280-1535 */
	GFXDECODE_ENTRY( "gfx3", 0x00000, tilelayout,   1024, 16 )	/* colors 1024-1079 */
	GFXDECODE_ENTRY( "gfx4", 0x00000, spritelayout,    0, 64 )	/* colors    0-1023 */
GFXDECODE_END




static MACHINE_DRIVER_START( wardner )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, XTAL_24MHz/4)		/* 6MHz */
	MDRV_CPU_PROGRAM_MAP(main_program_map)
	MDRV_CPU_IO_MAP(main_io_map)
	MDRV_CPU_VBLANK_INT("screen", wardner_interrupt)

	MDRV_CPU_ADD("audiocpu", Z80, XTAL_14MHz/4)		/* 3.5MHz */
	MDRV_CPU_PROGRAM_MAP(sound_program_map)
	MDRV_CPU_IO_MAP(sound_io_map)

	MDRV_CPU_ADD("dsp", TMS32010, XTAL_14MHz)		/* 14MHz Crystal CLKin */
	MDRV_CPU_PROGRAM_MAP(DSP_program_map)
	/* Data Map is internal to the CPU */
	MDRV_CPU_IO_MAP(DSP_io_map)

	MDRV_QUANTUM_TIME(HZ(6000))						/* 100 CPU slices per frame */

	MDRV_MACHINE_RESET(wardner)

	/* video hardware */
	MDRV_MC6845_ADD("crtc", HD6845, XTAL_14MHz/4, twincobr_mc6845_intf)	/* 3.5MHz measured on CLKin */

	MDRV_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK | VIDEO_BUFFERS_SPRITERAM)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE( (XTAL_14MHz / 2) / (446 * 286) )
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 64*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0*8, 30*8-1)

	MDRV_GFXDECODE(wardner)
	MDRV_PALETTE_LENGTH(1792)

	MDRV_VIDEO_START(toaplan0)
	MDRV_VIDEO_EOF(toaplan0)
	MDRV_VIDEO_UPDATE(toaplan0)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ymsnd", YM3812, XTAL_14MHz/4)
	MDRV_SOUND_CONFIG(ym3812_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END




/***************************************************************************

  Game driver(s)

***************************************************************************/


ROM_START( wardner )
	ROM_REGION( 0x40000, "maincpu", 0 )	/* Banked Main Z80 code */
	ROM_LOAD( "wardner.17", 0x00000, 0x08000, CRC(c5dd56fd) SHA1(f0a09557150e9c1c6b9d8e125f5408fc269c9d17) )	/* Main Z80 code */
	ROM_LOAD( "b25-18.rom", 0x10000, 0x10000, CRC(9aab8ee2) SHA1(16fa44b75f4a3a5b1ff713690a299ecec2b5a4bf) )	/* OBJ ROMs */
	ROM_LOAD( "b25-19.rom", 0x20000, 0x10000, CRC(95b68813) SHA1(06ea1b1d6e2e6326ceb9324fc471d082fda6112e) )
	ROM_LOAD( "wardner.20", 0x38000, 0x08000, CRC(347f411b) SHA1(1fb2883d74d10350cb1c62fb58d5783652861b37) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* Sound Z80 code */
	ROM_LOAD( "b25-16.rom", 0x00000, 0x08000, CRC(e5202ff8) SHA1(15ae8c0bb16a20bee14e8d80d81c249404ab1463) )

	ROM_REGION( 0x2000, "dsp", 0 )	/* Co-Processor TMS320C10 */
	ROMX_LOAD( "82s137.1d",  0x0000, 0x0400, CRC(cc5b3f53) SHA1(33589665ac995cc4645b56bbcd6d1c1cd5368f88), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(1) ) /* msb */
	ROMX_LOAD( "82s137.1e",  0x0000, 0x0400, CRC(47351d55) SHA1(826add3ea3987f2c9ba2d3fc69a4ad2d9b033c89), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(1) )
	ROMX_LOAD( "82s137.3d",  0x0001, 0x0400, CRC(70b537b9) SHA1(5211ec4605894727747dda66b70c9427652b16b4), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(1) ) /* lsb */
	ROMX_LOAD( "82s137.3e",  0x0001, 0x0400, CRC(6edb2de8) SHA1(48459037c3b865f0c0d63a416fa71ba1119f7a09), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(1) )
	ROMX_LOAD( "82s131.3b",  0x0800, 0x0200, CRC(9dfffaff) SHA1(2f4a1c1afba6a362dc5774a82656883b08fa16f2), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(1) )
	ROMX_LOAD( "82s131.3a",  0x0800, 0x0200, CRC(712bad47) SHA1(b9f7be13cbd90a17fe7d13fb7987a0b9b759ccad), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(1) )
	ROMX_LOAD( "82s131.2a",  0x0801, 0x0200, CRC(ac843ca6) SHA1(8fd278748ec89d8ebe2d4f3bf8b6731f357ddfb3), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(1) )
	ROMX_LOAD( "82s131.1a",  0x0801, 0x0200, CRC(50452ff8) SHA1(76964fa9ee89a51cc71904e08cfc83bf81bb89aa), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(1) )

	ROM_REGION( 0x0c000, "gfx1", 0 )	/* chars */
	ROM_LOAD( "wardner.07", 0x00000, 0x04000, CRC(1392b60d) SHA1(86b9eab87f8d5f68fda500420f4ed61331089fc2) )
	ROM_LOAD( "wardner.06", 0x04000, 0x04000, CRC(0ed848da) SHA1(e4b38e21c101a28a8961a9fe30c9cb10919cc148) )
	ROM_LOAD( "wardner.05", 0x08000, 0x04000, CRC(79792c86) SHA1(648b97f1ec30d46e40e60eb13159b4f6f86e9243) )

	ROM_REGION( 0x20000, "gfx2", 0 )	/* fg tiles */
	ROM_LOAD( "b25-12.rom",  0x00000, 0x08000, CRC(15d08848) SHA1(e2e62d95a3f240664b5e0ac0f163a0d5cefa5312) )
	ROM_LOAD( "b25-15.rom",  0x08000, 0x08000, CRC(cdd2d408) SHA1(7e4d77f8725fa30d4d65e811d10e0b2c00b23cfe) )
	ROM_LOAD( "b25-14.rom",  0x10000, 0x08000, CRC(5a2aef4f) SHA1(60f4ab2582a924defb5241ab367826ae1f4b3f5e) )
	ROM_LOAD( "b25-13.rom",  0x18000, 0x08000, CRC(be21db2b) SHA1(7fc1809618f2432c9ec6eb33ce57a5faffd44974) )

	ROM_REGION( 0x20000, "gfx3", 0 )	/* bg tiles */
	ROM_LOAD( "b25-08.rom",  0x00000, 0x08000, CRC(883ccaa3) SHA1(90d686094eac6e80caf8e2cf90c00bb41a0d26e2) )
	ROM_LOAD( "b25-11.rom",  0x08000, 0x08000, CRC(d6ebd510) SHA1(d65e0db7756ebe6828bf637a6c915bb06082636c) )
	ROM_LOAD( "b25-10.rom",  0x10000, 0x08000, CRC(b9a61e81) SHA1(541e579664d583fbbf81111046115018fdaff073) )
	ROM_LOAD( "b25-09.rom",  0x18000, 0x08000, CRC(585411b7) SHA1(67c0f4b7ab303341d5481c4024dc4199acb7c279) )

	ROM_REGION( 0x40000, "gfx4", 0 )	/* sprites */
	ROM_LOAD( "b25-01.rom",  0x00000, 0x10000, CRC(42ec01fb) SHA1(646192a2e89f795ed016860cdcdc0b5ef645fca2) )
	ROM_LOAD( "b25-02.rom",  0x10000, 0x10000, CRC(6c0130b7) SHA1(8b6ad72848d03c3d4ee3acd35abbb3a0e678122c) )
	ROM_LOAD( "b25-03.rom",  0x20000, 0x10000, CRC(b923db99) SHA1(2f4be81afdf200586bc44b1e94553d84d16d0b62) )
	ROM_LOAD( "b25-04.rom",  0x30000, 0x10000, CRC(8059573c) SHA1(75bd19e504433438b85ed00e50e85fb98eebf4de) )

	ROM_REGION( 0x260, "proms", 0 )		/* nibble bproms, lo/hi order to be determined */
	ROM_LOAD( "82s129.b19",  0x000, 0x100, CRC(24e7d62f) SHA1(1c06a1ef1b6a722794ca1d5ee2c476ecaa5178a3) )	/* sprite priority control ?? */
	ROM_LOAD( "82s129.b18",  0x100, 0x100, CRC(a50cef09) SHA1(55cafb5b2551b80ae708e9b966cf37c70a16d310) )	/* sprite priority control ?? */
	ROM_LOAD( "82s123.b21",  0x200, 0x020, CRC(f72482db) SHA1(b0cb911f9c81f6088a5aa8760916ddae1f8534d7) )	/* sprite control ?? */
	ROM_LOAD( "82s123.c6",   0x220, 0x020, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) )	/* sprite attribute (flip/position) ?? */
	ROM_LOAD( "82s123.f1",   0x240, 0x020, CRC(4fb5df2a) SHA1(506ef2c8e4cf45c256d6831a0a5760732f2de422) )	/* tile to sprite priority ?? */
ROM_END

ROM_START( pyros )
	ROM_REGION( 0x40000, "maincpu", 0 )	/* Banked Z80 code */
	ROM_LOAD( "b25-29.rom", 0x00000, 0x08000, CRC(b568294d) SHA1(5d04dd006f5180fa0c9340e2efa6613625d712a8) )	/* Main Z80 code */
	ROM_LOAD( "b25-18.rom", 0x10000, 0x10000, CRC(9aab8ee2) SHA1(16fa44b75f4a3a5b1ff713690a299ecec2b5a4bf) )	/* OBJ ROMs */
	ROM_LOAD( "b25-19.rom", 0x20000, 0x10000, CRC(95b68813) SHA1(06ea1b1d6e2e6326ceb9324fc471d082fda6112e) )
	ROM_LOAD( "b25-30.rom", 0x38000, 0x08000, CRC(5056c799) SHA1(9750fa8bf5d1181a4fecbcbf822f8f027bebd5a8) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* Sound Z80 code */
	ROM_LOAD( "b25-16.rom", 0x00000, 0x08000, CRC(e5202ff8) SHA1(15ae8c0bb16a20bee14e8d80d81c249404ab1463) )

	ROM_REGION( 0x2000, "dsp", 0 )	/* Co-Processor TMS320C10 */
	ROMX_LOAD( "82s137.1d",  0x0000, 0x0400, CRC(cc5b3f53) SHA1(33589665ac995cc4645b56bbcd6d1c1cd5368f88), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(1) ) /* msb */
	ROMX_LOAD( "82s137.1e",  0x0000, 0x0400, CRC(47351d55) SHA1(826add3ea3987f2c9ba2d3fc69a4ad2d9b033c89), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(1) )
	ROMX_LOAD( "82s137.3d",  0x0001, 0x0400, CRC(70b537b9) SHA1(5211ec4605894727747dda66b70c9427652b16b4), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(1) ) /* lsb */
	ROMX_LOAD( "82s137.3e",  0x0001, 0x0400, CRC(6edb2de8) SHA1(48459037c3b865f0c0d63a416fa71ba1119f7a09), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(1) )
	ROMX_LOAD( "82s131.3b",  0x0800, 0x0200, CRC(9dfffaff) SHA1(2f4a1c1afba6a362dc5774a82656883b08fa16f2), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(1) )
	ROMX_LOAD( "82s131.3a",  0x0800, 0x0200, CRC(712bad47) SHA1(b9f7be13cbd90a17fe7d13fb7987a0b9b759ccad), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(1) )
	ROMX_LOAD( "82s131.2a",  0x0801, 0x0200, CRC(ac843ca6) SHA1(8fd278748ec89d8ebe2d4f3bf8b6731f357ddfb3), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(1) )
	ROMX_LOAD( "82s131.1a",  0x0801, 0x0200, CRC(50452ff8) SHA1(76964fa9ee89a51cc71904e08cfc83bf81bb89aa), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(1) )

	ROM_REGION( 0x0c000, "gfx1", 0 )	/* chars */
	ROM_LOAD( "b25-35.rom", 0x00000, 0x04000, CRC(fec6f0c0) SHA1(f91d698fa0712659c2e6b382a8166b1cacc50a3f) )
	ROM_LOAD( "b25-34.rom", 0x04000, 0x04000, CRC(02505dad) SHA1(28993c68a17929d6b819ca81cdf60985531fc80b) )
	ROM_LOAD( "b25-33.rom", 0x08000, 0x04000, CRC(9a55fcb9) SHA1(e04d68cc0b80b79b5f4c19c14b2e87f226f45ac8) )

	ROM_REGION( 0x20000, "gfx2", 0 )	/* fg tiles */
	ROM_LOAD( "b25-12.rom",  0x00000, 0x08000, CRC(15d08848) SHA1(e2e62d95a3f240664b5e0ac0f163a0d5cefa5312) )
	ROM_LOAD( "b25-15.rom",  0x08000, 0x08000, CRC(cdd2d408) SHA1(7e4d77f8725fa30d4d65e811d10e0b2c00b23cfe) )
	ROM_LOAD( "b25-14.rom",  0x10000, 0x08000, CRC(5a2aef4f) SHA1(60f4ab2582a924defb5241ab367826ae1f4b3f5e) )
	ROM_LOAD( "b25-13.rom",  0x18000, 0x08000, CRC(be21db2b) SHA1(7fc1809618f2432c9ec6eb33ce57a5faffd44974) )

	ROM_REGION( 0x20000, "gfx3", 0 )	/* bg tiles */
	ROM_LOAD( "b25-08.rom",  0x00000, 0x08000, CRC(883ccaa3) SHA1(90d686094eac6e80caf8e2cf90c00bb41a0d26e2) )
	ROM_LOAD( "b25-11.rom",  0x08000, 0x08000, CRC(d6ebd510) SHA1(d65e0db7756ebe6828bf637a6c915bb06082636c) )
	ROM_LOAD( "b25-10.rom",  0x10000, 0x08000, CRC(b9a61e81) SHA1(541e579664d583fbbf81111046115018fdaff073) )
	ROM_LOAD( "b25-09.rom",  0x18000, 0x08000, CRC(585411b7) SHA1(67c0f4b7ab303341d5481c4024dc4199acb7c279) )

	ROM_REGION( 0x40000, "gfx4", 0 )	/* sprites */
	ROM_LOAD( "b25-01.rom",  0x00000, 0x10000, CRC(42ec01fb) SHA1(646192a2e89f795ed016860cdcdc0b5ef645fca2) )
	ROM_LOAD( "b25-02.rom",  0x10000, 0x10000, CRC(6c0130b7) SHA1(8b6ad72848d03c3d4ee3acd35abbb3a0e678122c) )
	ROM_LOAD( "b25-03.rom",  0x20000, 0x10000, CRC(b923db99) SHA1(2f4be81afdf200586bc44b1e94553d84d16d0b62) )
	ROM_LOAD( "b25-04.rom",  0x30000, 0x10000, CRC(8059573c) SHA1(75bd19e504433438b85ed00e50e85fb98eebf4de) )

	ROM_REGION( 0x260, "proms", 0 )		/* nibble bproms, lo/hi order to be determined */
	ROM_LOAD( "82s129.b19",  0x000, 0x100, CRC(24e7d62f) SHA1(1c06a1ef1b6a722794ca1d5ee2c476ecaa5178a3) )	/* sprite priority control ?? */
	ROM_LOAD( "82s129.b18",  0x100, 0x100, CRC(a50cef09) SHA1(55cafb5b2551b80ae708e9b966cf37c70a16d310) )	/* sprite priority control ?? */
	ROM_LOAD( "82s123.b21",  0x200, 0x020, CRC(f72482db) SHA1(b0cb911f9c81f6088a5aa8760916ddae1f8534d7) )	/* sprite control ?? */
	ROM_LOAD( "82s123.c6",   0x220, 0x020, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) )	/* sprite attribute (flip/position) ?? */
	ROM_LOAD( "82s123.f1",   0x240, 0x020, CRC(4fb5df2a) SHA1(506ef2c8e4cf45c256d6831a0a5760732f2de422) )	/* tile to sprite priority ?? */
ROM_END

ROM_START( wardnerj )
	ROM_REGION( 0x40000, "maincpu", 0 )	/* Banked Z80 code */
	ROM_LOAD( "b25-17.bin",  0x00000, 0x08000, CRC(4164dca9) SHA1(1f02c0991d7c14230043e34cb4b8e089b467b234) )	/* Main Z80 code */
	ROM_LOAD( "b25-18.rom",  0x10000, 0x10000, CRC(9aab8ee2) SHA1(16fa44b75f4a3a5b1ff713690a299ecec2b5a4bf) )	/* OBJ ROMs */
	ROM_LOAD( "b25-19.rom",  0x20000, 0x10000, CRC(95b68813) SHA1(06ea1b1d6e2e6326ceb9324fc471d082fda6112e) )
	ROM_LOAD( "b25-20.bin",  0x38000, 0x08000, CRC(1113ad38) SHA1(88f89054954b1d2776ceaedc7a3605190808d7e5) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* Sound Z80 code */
	ROM_LOAD( "b25-16.rom", 0x00000, 0x08000, CRC(e5202ff8) SHA1(15ae8c0bb16a20bee14e8d80d81c249404ab1463) )

	ROM_REGION( 0x2000, "dsp", 0 )	/* Co-Processor TMS320C10 */
	ROMX_LOAD( "82s137.1d",  0x0000, 0x0400, CRC(cc5b3f53) SHA1(33589665ac995cc4645b56bbcd6d1c1cd5368f88), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(1) ) /* msb */
	ROMX_LOAD( "82s137.1e",  0x0000, 0x0400, CRC(47351d55) SHA1(826add3ea3987f2c9ba2d3fc69a4ad2d9b033c89), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(1) )
	ROMX_LOAD( "82s137.3d",  0x0001, 0x0400, CRC(70b537b9) SHA1(5211ec4605894727747dda66b70c9427652b16b4), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(1) ) /* lsb */
	ROMX_LOAD( "82s137.3e",  0x0001, 0x0400, CRC(6edb2de8) SHA1(48459037c3b865f0c0d63a416fa71ba1119f7a09), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(1) )
	ROMX_LOAD( "82s131.3b",  0x0800, 0x0200, CRC(9dfffaff) SHA1(2f4a1c1afba6a362dc5774a82656883b08fa16f2), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(1) )
	ROMX_LOAD( "82s131.3a",  0x0800, 0x0200, CRC(712bad47) SHA1(b9f7be13cbd90a17fe7d13fb7987a0b9b759ccad), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(1) )
	ROMX_LOAD( "82s131.2a",  0x0801, 0x0200, CRC(ac843ca6) SHA1(8fd278748ec89d8ebe2d4f3bf8b6731f357ddfb3), ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI | ROM_SKIP(1) )
	ROMX_LOAD( "82s131.1a",  0x0801, 0x0200, CRC(50452ff8) SHA1(76964fa9ee89a51cc71904e08cfc83bf81bb89aa), ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO | ROM_SKIP(1) )

	ROM_REGION( 0x0c000, "gfx1", 0 )	/* chars */
	ROM_LOAD( "b25-07.bin", 0x00000, 0x04000, CRC(50e329e0) SHA1(5d5fb7043457d952b28101acb909ed65bf13a2dc) )
	ROM_LOAD( "b25-06.bin", 0x04000, 0x04000, CRC(3bfeb6ae) SHA1(3a251f49901ccb17b5fedf81980d54e4f6f49d4d) )
	ROM_LOAD( "b25-05.bin", 0x08000, 0x04000, CRC(be36a53e) SHA1(320fc5b88ed1fce1aa8d8e76e1046206c138b35c) )

	ROM_REGION( 0x20000, "gfx2", 0 )	/* fg tiles */
	ROM_LOAD( "b25-12.rom",  0x00000, 0x08000, CRC(15d08848) SHA1(e2e62d95a3f240664b5e0ac0f163a0d5cefa5312) )
	ROM_LOAD( "b25-15.rom",  0x08000, 0x08000, CRC(cdd2d408) SHA1(7e4d77f8725fa30d4d65e811d10e0b2c00b23cfe) )
	ROM_LOAD( "b25-14.rom",  0x10000, 0x08000, CRC(5a2aef4f) SHA1(60f4ab2582a924defb5241ab367826ae1f4b3f5e) )
	ROM_LOAD( "b25-13.rom",  0x18000, 0x08000, CRC(be21db2b) SHA1(7fc1809618f2432c9ec6eb33ce57a5faffd44974) )

	ROM_REGION( 0x20000, "gfx3", 0 )	/* bg tiles */
	ROM_LOAD( "b25-08.rom",  0x00000, 0x08000, CRC(883ccaa3) SHA1(90d686094eac6e80caf8e2cf90c00bb41a0d26e2) )
	ROM_LOAD( "b25-11.rom",  0x08000, 0x08000, CRC(d6ebd510) SHA1(d65e0db7756ebe6828bf637a6c915bb06082636c) )
	ROM_LOAD( "b25-10.rom",  0x10000, 0x08000, CRC(b9a61e81) SHA1(541e579664d583fbbf81111046115018fdaff073) )
	ROM_LOAD( "b25-09.rom",  0x18000, 0x08000, CRC(585411b7) SHA1(67c0f4b7ab303341d5481c4024dc4199acb7c279) )

	ROM_REGION( 0x40000, "gfx4", 0 )	/* sprites */
	ROM_LOAD( "b25-01.rom",  0x00000, 0x10000, CRC(42ec01fb) SHA1(646192a2e89f795ed016860cdcdc0b5ef645fca2) )
	ROM_LOAD( "b25-02.rom",  0x10000, 0x10000, CRC(6c0130b7) SHA1(8b6ad72848d03c3d4ee3acd35abbb3a0e678122c) )
	ROM_LOAD( "b25-03.rom",  0x20000, 0x10000, CRC(b923db99) SHA1(2f4be81afdf200586bc44b1e94553d84d16d0b62) )
	ROM_LOAD( "b25-04.rom",  0x30000, 0x10000, CRC(8059573c) SHA1(75bd19e504433438b85ed00e50e85fb98eebf4de) )

	ROM_REGION( 0x260, "proms", 0 )		/* nibble bproms, lo/hi order to be determined */
	ROM_LOAD( "82s129.b19",  0x000, 0x100, CRC(24e7d62f) SHA1(1c06a1ef1b6a722794ca1d5ee2c476ecaa5178a3) )	/* sprite priority control ?? */
	ROM_LOAD( "82s129.b18",  0x100, 0x100, CRC(a50cef09) SHA1(55cafb5b2551b80ae708e9b966cf37c70a16d310) )	/* sprite priority control ?? */
	ROM_LOAD( "82s123.b21",  0x200, 0x020, CRC(f72482db) SHA1(b0cb911f9c81f6088a5aa8760916ddae1f8534d7) )	/* sprite control ?? */
	ROM_LOAD( "82s123.c6",   0x220, 0x020, CRC(bc88cced) SHA1(5055362710c0f58823c05fb4c0e0eec638b91e3d) )	/* sprite attribute (flip/position) ?? */
	ROM_LOAD( "82s123.f1",   0x240, 0x020, CRC(4fb5df2a) SHA1(506ef2c8e4cf45c256d6831a0a5760732f2de422) )	/* tile to sprite priority ?? */
ROM_END


static DRIVER_INIT( wardner )
{
	wardner_driver_savestate(machine);	/* Save-State stuff in src/machine/twincobr.c */
}



GAME( 1987, wardner,  0,       wardner, wardner,  wardner, ROT0, "Toaplan / Taito Corporation Japan", "Wardner (World)", GAME_SUPPORTS_SAVE )
GAME( 1987, pyros,    wardner, wardner, pyros,    wardner, ROT0, "Toaplan / Taito America Corporation", "Pyros (US)", GAME_SUPPORTS_SAVE )
GAME( 1987, wardnerj, wardner, wardner, wardnerj, wardner, ROT0, "Toaplan / Taito Corporation", "Wardner no Mori (Japan)", GAME_SUPPORTS_SAVE )
