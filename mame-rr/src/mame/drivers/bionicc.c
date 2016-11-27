/******************************************************************************************

    Bionic Commando


    PCB is a 3 board stack:
        Main CPU board: 86612-A-2
    Graphics ROM board: 86612-B-2
     Program ROM board: 86612-C-2

      Main CPU: 68000CP10
     Sound CPU: Z80A
           MCU: Intel C8751H-88
    Sound Chip: YM2151 & YM3012
           OSC: 24.000 MHz (on the 86612-B-2 PCB)
        Custom: CAPCOM DL-010D-103 (on the 86612-B-2 PCB)


    Note: Euro rom labels (IE: "TSE") had a blue stripe, while those labeled
          as USA (TSU) had an red stripe on the sticker.  The intermixing
          of TSE and TSU roms in the parent set is correct and verified.
    Note: Euro set simply states the game cannot be operated in Japan....
    Note: These issues have been verified on a real PCB and are not emulation bugs:
          - misplaced sprites ( see beginning of level 1 or 2 for example )
          - sprite / sprite priority ( see level 2 the reflectors )
          - sprite / background priority ( see level 1: birds walk through
            branches of different trees )
          - see the beginning of level 3: background screwed
          - gray tiles around the title in Top Secret

    ToDo:
    - get rid of input port hack

        Controls appear to be mapped at 0xFE4000, alongside dip switches, but there
        is something strange going on that I can't (yet) figure out.
        Player controls and coin inputs are supposed to magically appear at
        0xFFFFFB (coin/start)
        0xFFFFFD (player 2)
        0xFFFFFF (player 1)

        This is probably done by the Intel C8751H MCU on the board (whose interal ROM
        is not yet available).

        The MCU also takes care of the commands for the sound CPU, which are stored
        at FFFFF9.

        IRQ4 seems to be control related.
        On each interrupt, it reads 0xFE4000 (coin/start), shift the bits around
        and move the resulting byte into a dword RAM location. The dword RAM location
        is rotated by 8 bits each time this happens.
        This is probably done to be pedantic about coin insertions (might be protection
        related). In fact, currently coin insertions are not consistently recognized.


******************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "deprecat.h"
#include "sound/2151intf.h"
#include "includes/bionicc.h"

#define MASTER_CLOCK       XTAL_24MHz
#define EXO3_F0_CLK        XTAL_14_31818MHz


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

static WRITE16_HANDLER( hacked_controls_w )
{
	bionicc_state *state = (bionicc_state *)space->machine->driver_data;

	logerror("%06x: hacked_controls_w %04x %02x\n", cpu_get_pc(space->cpu), offset, data);
	COMBINE_DATA(&state->inp[offset]);
}

static READ16_HANDLER( hacked_controls_r )
{
	bionicc_state *state = (bionicc_state *)space->machine->driver_data;

	logerror("%06x: hacked_controls_r %04x %04x\n", cpu_get_pc(space->cpu), offset, state->inp[offset]);
	return state->inp[offset];
}

static WRITE16_HANDLER( bionicc_mpu_trigger_w )
{
	bionicc_state *state = (bionicc_state *)space->machine->driver_data;

	data = input_port_read(space->machine, "SYSTEM") >> 12;
	state->inp[0] = data ^ 0x0f;

	data = input_port_read(space->machine, "P2");
	state->inp[1] = data ^ 0xff;

	data = input_port_read(space->machine, "P1");
	state->inp[2] = data ^ 0xff;
}


static WRITE16_HANDLER( hacked_soundcommand_w )
{
	bionicc_state *state = (bionicc_state *)space->machine->driver_data;

	COMBINE_DATA(&state->soundcommand);
	soundlatch_w(space, 0, state->soundcommand & 0xff);
}

static READ16_HANDLER( hacked_soundcommand_r )
{
	bionicc_state *state = (bionicc_state *)space->machine->driver_data;

	return state->soundcommand;
}


/********************************************************************

  Interrupt

  The game runs on 2 interrupts.

  IRQ 2 drives the game
  IRQ 4 processes the input ports

  The game is very picky about timing. The following is the only
  way I have found it to work.

********************************************************************/

static INTERRUPT_GEN( bionicc_interrupt )
{
	if (cpu_getiloops(device) == 0)
		cpu_set_input_line(device, 2, HOLD_LINE);
	else
		cpu_set_input_line(device, 4, HOLD_LINE);
}

/*************************************
 *
 *  Address maps
 *
 *************************************/

static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0xfe0000, 0xfe07ff) AM_RAM	/* RAM? */
	AM_RANGE(0xfe0800, 0xfe0cff) AM_RAM AM_BASE_SIZE_GENERIC(spriteram)
	AM_RANGE(0xfe0d00, 0xfe3fff) AM_RAM              /* RAM? */
	AM_RANGE(0xfe4000, 0xfe4001) AM_WRITE(bionicc_gfxctrl_w)	/* + coin counters */
	AM_RANGE(0xfe4000, 0xfe4001) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xfe4002, 0xfe4003) AM_READ_PORT("DSW")
	AM_RANGE(0xfe8010, 0xfe8017) AM_WRITE(bionicc_scroll_w)
	AM_RANGE(0xfe801a, 0xfe801b) AM_WRITE(bionicc_mpu_trigger_w)	/* ??? not sure, but looks like it */
	AM_RANGE(0xfec000, 0xfecfff) AM_RAM_WRITE(bionicc_txvideoram_w) AM_BASE_MEMBER(bionicc_state, txvideoram)
	AM_RANGE(0xff0000, 0xff3fff) AM_RAM_WRITE(bionicc_fgvideoram_w) AM_BASE_MEMBER(bionicc_state, fgvideoram)
	AM_RANGE(0xff4000, 0xff7fff) AM_RAM_WRITE(bionicc_bgvideoram_w) AM_BASE_MEMBER(bionicc_state, bgvideoram)
	AM_RANGE(0xff8000, 0xff87ff) AM_RAM_WRITE(bionicc_paletteram_w) AM_BASE_MEMBER(bionicc_state, paletteram)
	AM_RANGE(0xffc000, 0xfffff7) AM_RAM	/* working RAM */
	AM_RANGE(0xfffff8, 0xfffff9) AM_READWRITE(hacked_soundcommand_r, hacked_soundcommand_w)      /* hack */
	AM_RANGE(0xfffffa, 0xffffff) AM_READWRITE(hacked_controls_r, hacked_controls_w)	/* hack */
ADDRESS_MAP_END


static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x8001) AM_DEVREADWRITE("ymsnd", ym2151_r, ym2151_w)
	AM_RANGE(0xa000, 0xa000) AM_READ(soundlatch_r)
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
ADDRESS_MAP_END


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( bionicc )
	PORT_START("SYSTEM")
	PORT_BIT( 0x0fff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) )		PORT_DIPLOCATION("SWB:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) )		PORT_DIPLOCATION("SWB:4,5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_6C ) )
	PORT_SERVICE_DIPLOC(  0x0040, IP_ACTIVE_LOW, "SWB:7" )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) )	PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) )		PORT_DIPLOCATION("SWA:1,2")
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0200, "4" )
	PORT_DIPSETTING(      0x0100, "5" )
	PORT_DIPSETTING(      0x0000, "7" )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Cabinet ) )		PORT_DIPLOCATION("SWA:3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x1800, 0x1800, DEF_STR( Bonus_Life ) )	PORT_DIPLOCATION("SWA:4,5")   /* table at 0x00483a */
	PORT_DIPSETTING(      0x1800, "20k 40k 100k 60k+" )
	PORT_DIPSETTING(      0x1000, "30k 50k 120k 70k+" )
	PORT_DIPSETTING(      0x0800, "20k 60k")
	PORT_DIPSETTING(      0x0000, "30k 70k" )
	PORT_DIPNAME( 0x6000, 0x4000, DEF_STR( Difficulty ) )	PORT_DIPLOCATION("SWA:6,7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Freeze" )				PORT_DIPLOCATION("SWA:8")     /* Listed as "Unused" */
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout spritelayout_bionicc=
{
	16,16,  /* 16*16 sprites */
	2048,   /* 2048 sprites */
	4,      /* 4 bits per pixel */
	{ 0x30000*8,0x20000*8,0x10000*8,0 },
	{
		0,1,2,3,4,5,6,7,
		(16*8)+0,(16*8)+1,(16*8)+2,(16*8)+3,
		(16*8)+4,(16*8)+5,(16*8)+6,(16*8)+7
	},
	{
		0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8,
	},
	256   /* every sprite takes 256 consecutive bytes */
};

static const gfx_layout vramlayout_bionicc=
{
	8,8,    /* 8*8 characters */
	1024,   /* 1024 character */
	2,      /* 2 bitplanes */
	{ 4,0 },
	{ 0,1,2,3,8,9,10,11 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	128   /* every character takes 128 consecutive bytes */
};

static const gfx_layout scroll2layout_bionicc=
{
	8,8,    /* 8*8 tiles */
	2048,   /* 2048 tiles */
	4,      /* 4 bits per pixel */
	{ (0x08000*8)+4,0x08000*8,4,0 },
	{ 0,1,2,3, 8,9,10,11 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	128   /* every tile takes 128 consecutive bytes */
};

static const gfx_layout scroll1layout_bionicc=
{
	16,16,  /* 16*16 tiles */
	2048,   /* 2048 tiles */
	4,      /* 4 bits per pixel */
	{ (0x020000*8)+4,0x020000*8,4,0 },
	{
		0,1,2,3, 8,9,10,11,
		(8*4*8)+0,(8*4*8)+1,(8*4*8)+2,(8*4*8)+3,
		(8*4*8)+8,(8*4*8)+9,(8*4*8)+10,(8*4*8)+11
	},
	{
		0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
		8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16
	},
	512   /* each tile takes 512 consecutive bytes */
};

static GFXDECODE_START( bionicc )
	GFXDECODE_ENTRY( "gfx1", 0, vramlayout_bionicc,    768, 64 )	/* colors 768-1023 */
	GFXDECODE_ENTRY( "gfx2", 0, scroll2layout_bionicc,   0,  4 )	/* colors   0-  63 */
	GFXDECODE_ENTRY( "gfx3", 0, scroll1layout_bionicc, 256,  4 )	/* colors 256- 319 */
	GFXDECODE_ENTRY( "gfx4", 0, spritelayout_bionicc,  512, 16 )	/* colors 512- 767 */
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_START( bionicc )
{
	bionicc_state *state = (bionicc_state *)machine->driver_data;

	state_save_register_global(machine, state->soundcommand);
	state_save_register_global_array(machine, state->inp);
	state_save_register_global_array(machine, state->scroll);
}

static MACHINE_RESET( bionicc )
{
	bionicc_state *state = (bionicc_state *)machine->driver_data;

	state->inp[0] = 0;
	state->inp[1] = 0;
	state->inp[2] = 0;
	state->scroll[0] = 0;
	state->scroll[1] = 0;
	state->scroll[2] = 0;
	state->scroll[3] = 0;
	state->soundcommand = 0;
}

static MACHINE_DRIVER_START( bionicc )

	/* driver data */
	MDRV_DRIVER_DATA(bionicc_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, MASTER_CLOCK / 2) /* 12 MHz - verified in schematics */
	MDRV_CPU_PROGRAM_MAP(main_map)
	MDRV_CPU_VBLANK_INT_HACK(bionicc_interrupt,8)

	MDRV_CPU_ADD("audiocpu", Z80, EXO3_F0_CLK / 4)   /* EXO3 C,B=GND, A=5V ==> Divisor 2^2 */
	MDRV_CPU_PROGRAM_MAP(sound_map)
	/* FIXME: interrupt timing
     * schematics indicate that nmi_line is set on  M680000 access with AB1=1
     * and IOCS=0 (active low), see pages A-1/10, A-4/10 in schematics
     */
	MDRV_CPU_VBLANK_INT_HACK(nmi_line_pulse,4)

	MDRV_MACHINE_START(bionicc)
	MDRV_MACHINE_RESET(bionicc)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_BUFFERS_SPRITERAM)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MDRV_GFXDECODE(bionicc)
	MDRV_PALETTE_LENGTH(1024)

	MDRV_VIDEO_START(bionicc)
	MDRV_VIDEO_EOF(bionicc)
	MDRV_VIDEO_UPDATE(bionicc)

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ymsnd", YM2151, 3579545)
	MDRV_SOUND_ROUTE(0, "mono", 0.60)
	MDRV_SOUND_ROUTE(1, "mono", 0.60)
MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( bionicc ) /* "Not for use in Japan" */
	ROM_REGION( 0x40000, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "tse_02.1a",  0x00000, 0x10000, CRC(e4aeefaa) SHA1(77b6a2d4337bf350239abb50013d030d7c5c8640) ) /* 68000 code */
	ROM_LOAD16_BYTE( "tse_04.1b",  0x00001, 0x10000, CRC(d0c8ec75) SHA1(04138c75ca3939604100b7e9fb451f7fceee67ca) ) /* 68000 code */
	ROM_LOAD16_BYTE( "tse_03.2a",  0x20000, 0x10000, CRC(b2ac0a45) SHA1(d0933e74870efa9ea703251b30a56ef706ac24fe) ) /* 68000 code */
	ROM_LOAD16_BYTE( "tse_05.2b",  0x20001, 0x10000, CRC(a79cb406) SHA1(50eada2f3e80c28dcb5529890d9b279c73f0115a) ) /* 68000 code */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ts_01b.4e",  0x00000, 0x8000, CRC(a9a6cafa) SHA1(55e0a0e6ca11e8e73339d5b4604e130031211291) )

	ROM_REGION( 0x1000, "mcu", 0 )	/* i8751 microcontroller */
	ROM_LOAD( "c8751h-88",     0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "tsu_08.8l",   0x00000, 0x8000, CRC(9bf0b7a2) SHA1(1361335c3c2c8a9c6a7d99566048d8aac99e7c8f) )	/* VIDEORAM (text layer) tiles */

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "tsu_07.5l",   0x00000, 0x8000, CRC(9469efa4) SHA1(53c70361e8d9e54825f61b87a10df42438aaf5b0) )	/* SCROLL2 Layer Tiles */
	ROM_LOAD( "tsu_06.4l",   0x08000, 0x8000, CRC(40bf0eb4) SHA1(fcb186c31747e2c9872de01e34b3e713dc74df82) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "ts_12.17f",    0x00000, 0x8000, CRC(e4b4619e) SHA1(3bec8399ffb28fd50ce6ae88d90b091eadf8bda1) )	/* SCROLL1 Layer Tiles */
	ROM_LOAD( "ts_11.15f",    0x08000, 0x8000, CRC(ab30237a) SHA1(ea6c07df992ba48f9eca7daa4ea775faa94358d2) )
	ROM_LOAD( "ts_17.17g",    0x10000, 0x8000, CRC(deb657e4) SHA1(b36b468f9bbb7a4937286230d3f6caa14c61d4dd) )
	ROM_LOAD( "ts_16.15g",    0x18000, 0x8000, CRC(d363b5f9) SHA1(1dd3991d99db2d6bcbdb12879ba50a01fef95004) )
	ROM_LOAD( "ts_13.18f",    0x20000, 0x8000, CRC(a8f5a004) SHA1(36ab0cb8ec9ce0519876f7461ccc5020c9c5b597) )
	ROM_LOAD( "ts_18.18g",    0x28000, 0x8000, CRC(3b36948c) SHA1(d85fcc0265ba1729c587b046cc5a7ba6f25363dd) )
	ROM_LOAD( "ts_23.18j",    0x30000, 0x8000, CRC(bbfbe58a) SHA1(9b1d5672b6f3c5c0952f8dcd0da71acc68a97a5e) )
	ROM_LOAD( "ts_24.18k",    0x38000, 0x8000, CRC(f156e564) SHA1(a6cad05bcc6d9ded6294f9b5aa856d05641aed02) )

	ROM_REGION( 0x40000, "gfx4", 0 )
	ROM_LOAD( "tse_10.13f",   0x00000, 0x8000, CRC(d28eeacc) SHA1(8b4a655a48da276b07f3464c65743b13cec52bcb) )	/* Sprites */
	ROM_LOAD( "tsu_09.11f",   0x08000, 0x8000, CRC(6a049292) SHA1(525c862061f426d679b539b6926af4c9f14b47b5) )
	ROM_LOAD( "tse_15.13g",   0x10000, 0x8000, CRC(9b5593c0) SHA1(73c0acbb01fe69c2bd29dea11b6a223c8efb54a0) )
	ROM_LOAD( "tsu_14.11g",   0x18000, 0x8000, CRC(46b2ad83) SHA1(21ebd5691a544323fdfcf330b9a37bbe0428e3e3) )
	ROM_LOAD( "tse_20.13j",   0x20000, 0x8000, CRC(b03db778) SHA1(f72a93e73196c800c1893fd3b523394d702547dd) )
	ROM_LOAD( "tsu_19.11j",   0x28000, 0x8000, CRC(b5c82722) SHA1(969f9159f7d59e4e4c9ef9ddbdc27cbfa531eabf) )
	ROM_LOAD( "tse_22.17j",   0x30000, 0x8000, CRC(d4dedeb3) SHA1(e121057bb541f3f5c755963ca22832c3fe2637c0) )
	ROM_LOAD( "tsu_21.15j",   0x38000, 0x8000, CRC(98777006) SHA1(bcc2058b639e9b71d16af05f63df298bcce91fdc) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "63s141.18f",   0x0000, 0x0100, CRC(b58d0023) SHA1(e8a4a2e2951bf73b3d9eed6957e9ee1e61c9c58a) )	/* priority (not used), Labeled "TSB" */
ROM_END

ROM_START( bionicc1 ) /* "Not for use outside of USA or Canada" revision B */
	ROM_REGION( 0x40000, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "tsu_02b.1a",  0x00000, 0x10000, CRC(cf965a0a) SHA1(ab88742a3225a0b82ee2dfef6ed0058d3e11c38c) ) /* 68000 code */
	ROM_LOAD16_BYTE( "tsu_04b.1b",  0x00001, 0x10000, CRC(c9884bfb) SHA1(7d10cedff0a62847f8deb61a9611cc6661efb037) ) /* 68000 code */
	ROM_LOAD16_BYTE( "tsu_03b.2a",  0x20000, 0x10000, CRC(4e157ae2) SHA1(cc02931376d22a7fcfc320e6fd4129e03a461a49) ) /* 68000 code */
	ROM_LOAD16_BYTE( "tsu_05b.2b",  0x20001, 0x10000, CRC(e66ca0f9) SHA1(a503badf2fed38786d38c313d1dc315f3175d6de) ) /* 68000 code */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ts_01b.4e",  0x00000, 0x8000, CRC(a9a6cafa) SHA1(55e0a0e6ca11e8e73339d5b4604e130031211291) )

	ROM_REGION( 0x1000, "mcu", 0 )	/* i8751 microcontroller */
	ROM_LOAD( "c8751h-88",     0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "tsu_08.8l",   0x00000, 0x8000, CRC(9bf0b7a2) SHA1(1361335c3c2c8a9c6a7d99566048d8aac99e7c8f) )	/* VIDEORAM (text layer) tiles */

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "tsu_07.5l",   0x00000, 0x8000, CRC(9469efa4) SHA1(53c70361e8d9e54825f61b87a10df42438aaf5b0) )	/* SCROLL2 Layer Tiles */
	ROM_LOAD( "tsu_06.4l",   0x08000, 0x8000, CRC(40bf0eb4) SHA1(fcb186c31747e2c9872de01e34b3e713dc74df82) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "ts_12.17f",    0x00000, 0x8000, CRC(e4b4619e) SHA1(3bec8399ffb28fd50ce6ae88d90b091eadf8bda1) )	/* SCROLL1 Layer Tiles */
	ROM_LOAD( "ts_11.15f",    0x08000, 0x8000, CRC(ab30237a) SHA1(ea6c07df992ba48f9eca7daa4ea775faa94358d2) )
	ROM_LOAD( "ts_17.17g",    0x10000, 0x8000, CRC(deb657e4) SHA1(b36b468f9bbb7a4937286230d3f6caa14c61d4dd) )
	ROM_LOAD( "ts_16.15g",    0x18000, 0x8000, CRC(d363b5f9) SHA1(1dd3991d99db2d6bcbdb12879ba50a01fef95004) )
	ROM_LOAD( "ts_13.18f",    0x20000, 0x8000, CRC(a8f5a004) SHA1(36ab0cb8ec9ce0519876f7461ccc5020c9c5b597) )
	ROM_LOAD( "ts_18.18g",    0x28000, 0x8000, CRC(3b36948c) SHA1(d85fcc0265ba1729c587b046cc5a7ba6f25363dd) )
	ROM_LOAD( "ts_23.18j",    0x30000, 0x8000, CRC(bbfbe58a) SHA1(9b1d5672b6f3c5c0952f8dcd0da71acc68a97a5e) )
	ROM_LOAD( "ts_24.18k",    0x38000, 0x8000, CRC(f156e564) SHA1(a6cad05bcc6d9ded6294f9b5aa856d05641aed02) )

	ROM_REGION( 0x40000, "gfx4", 0 )
	ROM_LOAD( "tsu_10.13f",   0x00000, 0x8000, CRC(f1180d02) SHA1(312626af48235a1f726ab596f296ef4739785ca0) )	/* Sprites */
	ROM_LOAD( "tsu_09.11f",   0x08000, 0x8000, CRC(6a049292) SHA1(525c862061f426d679b539b6926af4c9f14b47b5) )
	ROM_LOAD( "tsu_15.13g",   0x10000, 0x8000, CRC(ea912701) SHA1(106336c63a1c8a0b13236268bc533a8263285cad) )
	ROM_LOAD( "tsu_14.11g",   0x18000, 0x8000, CRC(46b2ad83) SHA1(21ebd5691a544323fdfcf330b9a37bbe0428e3e3) )
	ROM_LOAD( "tsu_20.13j",   0x20000, 0x8000, CRC(17857ad2) SHA1(9f45cea6e9ce82bfc9ee6896a30257d20fb38bca) )
	ROM_LOAD( "tsu_19.11j",   0x28000, 0x8000, CRC(b5c82722) SHA1(969f9159f7d59e4e4c9ef9ddbdc27cbfa531eabf) )
	ROM_LOAD( "tsu_22.17j",   0x30000, 0x8000, CRC(5ee1ae6a) SHA1(76ca53d847c940c4176d79ba49b0c10efd6342e8) )
	ROM_LOAD( "tsu_21.15j",   0x38000, 0x8000, CRC(98777006) SHA1(bcc2058b639e9b71d16af05f63df298bcce91fdc) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "63s141.18f",   0x0000, 0x0100, CRC(b58d0023) SHA1(e8a4a2e2951bf73b3d9eed6957e9ee1e61c9c58a) )	/* priority (not used), Labeled "TSB" */
ROM_END

ROM_START( bionicc2 ) /* "Not for use outside of USA or Canada" 1st release */
	ROM_REGION( 0x40000, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "tsu_02.1a",  0x00000, 0x10000, CRC(f2528f08) SHA1(04c793837c86d83312fd44b46a6a94378c90113b) ) /* 68000 code */
	ROM_LOAD16_BYTE( "tsu_04.1b",  0x00001, 0x10000, CRC(38b1c7e4) SHA1(14bf743726c214bd00177e7b410c272dd7ab3d3f) ) /* 68000 code */
	ROM_LOAD16_BYTE( "tsu_03.2a",  0x20000, 0x10000, CRC(72c3b76f) SHA1(f7f71eae7617e3348b727775088b496e86d51e38) ) /* 68000 code */
	ROM_LOAD16_BYTE( "tsu_05.2b",  0x20001, 0x10000, CRC(70621f83) SHA1(0a77c2827a5c50457d90ccc62e463508d83d2f20) ) /* 68000 code */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ts_01b.4e",  0x00000, 0x8000, CRC(a9a6cafa) SHA1(55e0a0e6ca11e8e73339d5b4604e130031211291) )

	ROM_REGION( 0x1000, "mcu", 0 )	/* i8751 microcontroller */
	ROM_LOAD( "c8751h-88",     0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "tsu_08.8l",   0x00000, 0x8000, CRC(9bf0b7a2) SHA1(1361335c3c2c8a9c6a7d99566048d8aac99e7c8f) )	/* VIDEORAM (text layer) tiles */

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "tsu_07.5l",   0x00000, 0x8000, CRC(9469efa4) SHA1(53c70361e8d9e54825f61b87a10df42438aaf5b0) )	/* SCROLL2 Layer Tiles */
	ROM_LOAD( "tsu_06.4l",   0x08000, 0x8000, CRC(40bf0eb4) SHA1(fcb186c31747e2c9872de01e34b3e713dc74df82) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "ts_12.17f",    0x00000, 0x8000, CRC(e4b4619e) SHA1(3bec8399ffb28fd50ce6ae88d90b091eadf8bda1) )	/* SCROLL1 Layer Tiles */
	ROM_LOAD( "ts_11.15f",    0x08000, 0x8000, CRC(ab30237a) SHA1(ea6c07df992ba48f9eca7daa4ea775faa94358d2) )
	ROM_LOAD( "ts_17.17g",    0x10000, 0x8000, CRC(deb657e4) SHA1(b36b468f9bbb7a4937286230d3f6caa14c61d4dd) )
	ROM_LOAD( "ts_16.15g",    0x18000, 0x8000, CRC(d363b5f9) SHA1(1dd3991d99db2d6bcbdb12879ba50a01fef95004) )
	ROM_LOAD( "ts_13.18f",    0x20000, 0x8000, CRC(a8f5a004) SHA1(36ab0cb8ec9ce0519876f7461ccc5020c9c5b597) )
	ROM_LOAD( "ts_18.18g",    0x28000, 0x8000, CRC(3b36948c) SHA1(d85fcc0265ba1729c587b046cc5a7ba6f25363dd) )
	ROM_LOAD( "ts_23.18j",    0x30000, 0x8000, CRC(bbfbe58a) SHA1(9b1d5672b6f3c5c0952f8dcd0da71acc68a97a5e) )
	ROM_LOAD( "ts_24.18k",    0x38000, 0x8000, CRC(f156e564) SHA1(a6cad05bcc6d9ded6294f9b5aa856d05641aed02) )

	ROM_REGION( 0x40000, "gfx4", 0 )
	ROM_LOAD( "tsu_10.13f",   0x00000, 0x8000, CRC(f1180d02) SHA1(312626af48235a1f726ab596f296ef4739785ca0) )	/* Sprites */
	ROM_LOAD( "tsu_09.11f",   0x08000, 0x8000, CRC(6a049292) SHA1(525c862061f426d679b539b6926af4c9f14b47b5) )
	ROM_LOAD( "tsu_15.13g",   0x10000, 0x8000, CRC(ea912701) SHA1(106336c63a1c8a0b13236268bc533a8263285cad) )
	ROM_LOAD( "tsu_14.11g",   0x18000, 0x8000, CRC(46b2ad83) SHA1(21ebd5691a544323fdfcf330b9a37bbe0428e3e3) )
	ROM_LOAD( "tsu_20.13j",   0x20000, 0x8000, CRC(17857ad2) SHA1(9f45cea6e9ce82bfc9ee6896a30257d20fb38bca) )
	ROM_LOAD( "tsu_19.11j",   0x28000, 0x8000, CRC(b5c82722) SHA1(969f9159f7d59e4e4c9ef9ddbdc27cbfa531eabf) )
	ROM_LOAD( "tsu_22.17j",   0x30000, 0x8000, CRC(5ee1ae6a) SHA1(76ca53d847c940c4176d79ba49b0c10efd6342e8) )
	ROM_LOAD( "tsu_21.15j",   0x38000, 0x8000, CRC(98777006) SHA1(bcc2058b639e9b71d16af05f63df298bcce91fdc) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "63s141.18f",   0x0000, 0x0100, CRC(b58d0023) SHA1(e8a4a2e2951bf73b3d9eed6957e9ee1e61c9c58a) )	/* priority (not used), Labeled "TSB" */
ROM_END

ROM_START( topsecrt ) /* "Not for use in any other country but Japan" */
	ROM_REGION( 0x40000, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "ts_02.1a",  0x00000, 0x10000, CRC(b2fe1ddb) SHA1(892f19124993add96edabdba3aafeecc6668c5d9) ) /* 68000 code */
	ROM_LOAD16_BYTE( "ts_04.1b",  0x00001, 0x10000, CRC(427a003d) SHA1(5a379fe2942e5565810939d5eb843003226222cc) ) /* 68000 code */
	ROM_LOAD16_BYTE( "ts_03.2a",  0x20000, 0x10000, CRC(27f04bb6) SHA1(41d17b84b34dc8b2e5dfa67794a8df3e898b740b) ) /* 68000 code */
	ROM_LOAD16_BYTE( "ts_05.2b",  0x20001, 0x10000, CRC(c01547b1) SHA1(563bf6be4f10f5e6eb5b562266accf168f62bf30) ) /* 68000 code */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ts_01.4e",    0x00000, 0x8000, CRC(8ea07917) SHA1(e9ace70d89482fc3669860450a41aacacbee9083) )

	ROM_REGION( 0x1000, "mcu", 0 )	/* i8751 microcontroller */
	ROM_LOAD( "c8751h-88",     0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "ts_08.8l",    0x00000, 0x8000, CRC(96ad379e) SHA1(accd3a560b259c186bc28cdc004ed8de0b12f9d5) )	/* VIDEORAM (text layer) tiles */

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "ts_07.5l",    0x00000, 0x8000, CRC(25cdf8b2) SHA1(316f6acc46878682dabeab12722e6a64504d23bd) )	/* SCROLL2 Layer Tiles */
	ROM_LOAD( "ts_06.4l",    0x08000, 0x8000, CRC(314fb12d) SHA1(dab0519a49b64fe7a837b3c6383f6147e1ab6ffd) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "ts_12.17f",    0x00000, 0x8000, CRC(e4b4619e) SHA1(3bec8399ffb28fd50ce6ae88d90b091eadf8bda1) )	/* SCROLL1 Layer Tiles */
	ROM_LOAD( "ts_11.15f",    0x08000, 0x8000, CRC(ab30237a) SHA1(ea6c07df992ba48f9eca7daa4ea775faa94358d2) )
	ROM_LOAD( "ts_17.17g",    0x10000, 0x8000, CRC(deb657e4) SHA1(b36b468f9bbb7a4937286230d3f6caa14c61d4dd) )
	ROM_LOAD( "ts_16.15g",    0x18000, 0x8000, CRC(d363b5f9) SHA1(1dd3991d99db2d6bcbdb12879ba50a01fef95004) )
	ROM_LOAD( "ts_13.18f",    0x20000, 0x8000, CRC(a8f5a004) SHA1(36ab0cb8ec9ce0519876f7461ccc5020c9c5b597) )
	ROM_LOAD( "ts_18.18g",    0x28000, 0x8000, CRC(3b36948c) SHA1(d85fcc0265ba1729c587b046cc5a7ba6f25363dd) )
	ROM_LOAD( "ts_23.18j",    0x30000, 0x8000, CRC(bbfbe58a) SHA1(9b1d5672b6f3c5c0952f8dcd0da71acc68a97a5e) )
	ROM_LOAD( "ts_24.18k",    0x38000, 0x8000, CRC(f156e564) SHA1(a6cad05bcc6d9ded6294f9b5aa856d05641aed02) )

	ROM_REGION( 0x40000, "gfx4", 0 )
	ROM_LOAD( "ts_10.13f",    0x00000, 0x8000, CRC(c3587d05) SHA1(ad0898a5d4cf110783ef092bf8e65b6ef31a8ae0) )	/* Sprites */
	ROM_LOAD( "ts_09.11f",    0x08000, 0x8000, CRC(6b63eef2) SHA1(5d1580db7f49c5994c2a08a36c2d05f3e246930d) )
	ROM_LOAD( "ts_15.13g",    0x10000, 0x8000, CRC(db8cebb0) SHA1(1cc9eac14851cde95fb2d69d6f5ffb08bc9c0d93) )
	ROM_LOAD( "ts_14.11g",    0x18000, 0x8000, CRC(e2e41abf) SHA1(d002d0d8fdbb9ec3e2eac218f6338f733953ca82) )
	ROM_LOAD( "ts_20.13j",    0x20000, 0x8000, CRC(bfd1a695) SHA1(bf93486b96bfa1a1d5015189043b07e6130e6df1) )
	ROM_LOAD( "ts_19.11j",    0x28000, 0x8000, CRC(928b669e) SHA1(98ea9d23a46b0700490fd2fa7ab4fb0988dd5ca6) )
	ROM_LOAD( "ts_22.17j",    0x30000, 0x8000, CRC(3fe05d9a) SHA1(32e28ef03fb82785019d1ae8b3859215b5368c2b) )
	ROM_LOAD( "ts_21.15j",    0x38000, 0x8000, CRC(27a9bb7c) SHA1(bb60332c0ecde4d7797960dec39c1079498175c3) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "63s141.18f",   0x0000, 0x0100, CRC(b58d0023) SHA1(e8a4a2e2951bf73b3d9eed6957e9ee1e61c9c58a) )	/* priority (not used), Labeled "TSB" */
ROM_END


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1987, bionicc,  0,       bionicc, bionicc, 0, ROT0, "Capcom", "Bionic Commando (Euro)", GAME_SUPPORTS_SAVE )
GAME( 1987, bionicc1, bionicc, bionicc, bionicc, 0, ROT0, "Capcom", "Bionic Commando (US set 1)", GAME_SUPPORTS_SAVE )
GAME( 1987, bionicc2, bionicc, bionicc, bionicc, 0, ROT0, "Capcom", "Bionic Commando (US set 2)", GAME_SUPPORTS_SAVE )
GAME( 1987, topsecrt, bionicc, bionicc, bionicc, 0, ROT0, "Capcom", "Top Secret (Japan)", GAME_SUPPORTS_SAVE )
