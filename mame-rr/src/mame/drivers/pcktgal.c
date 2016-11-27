/***************************************************************************

    Pocket Gal                      (c) 1987 Data East Corporation
    Pocket Gal (Bootleg)            (c) 1989 Yada East Corporation(!!!)
    Super Pool III                  (c) 1989 Data East Corporation
    Pocket Gal 2                    (c) 1989 Data East Corporation
    Super Pool III (I-Vics Inc)     (c) 1990 Data East Corporation

    Pocket Gal (Bootleg) is often called 'Sexy Billiards'

    Emulation by Bryan McPhail, mish@tendril.co.uk

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "sound/2203intf.h"
#include "sound/3812intf.h"
#include "sound/msm5205.h"

extern WRITE8_HANDLER( pcktgal_videoram_w );
extern WRITE8_HANDLER( pcktgal_flipscreen_w );

extern PALETTE_INIT( pcktgal );
extern VIDEO_START( pcktgal );
extern VIDEO_UPDATE( pcktgal );

/***************************************************************************/

static WRITE8_HANDLER( pcktgal_bank_w )
{
	UINT8 *RAM = memory_region(space->machine, "maincpu");

	if (data & 1) { memory_set_bankptr(space->machine, "bank1", &RAM[0x4000]); }
	else { memory_set_bankptr(space->machine, "bank1", &RAM[0x10000]); }

	if (data & 2) { memory_set_bankptr(space->machine, "bank2", &RAM[0x6000]); }
	else { memory_set_bankptr(space->machine, "bank2", &RAM[0x12000]); }
}

static WRITE8_HANDLER( pcktgal_sound_bank_w )
{
	memory_set_bank(space->machine, "bank3", (data >> 2) & 1);
}

static WRITE8_HANDLER( pcktgal_sound_w )
{
	soundlatch_w(space, 0, data);
	cputag_set_input_line(space->machine, "audiocpu", INPUT_LINE_NMI, PULSE_LINE);
}

static int msm5205next;

static void pcktgal_adpcm_int(running_device *device)
{
	static int toggle;

	msm5205_data_w(device,msm5205next >> 4);
	msm5205next<<=4;

	toggle = 1 - toggle;
	if (toggle)
		cputag_set_input_line(device->machine, "audiocpu", M6502_IRQ_LINE, HOLD_LINE);
}

static WRITE8_HANDLER( pcktgal_adpcm_data_w )
{
	msm5205next=data;
}

static READ8_DEVICE_HANDLER( pcktgal_adpcm_reset_r )
{
	msm5205_reset_w(device,0);
	return 0;
}

/***************************************************************************/

static ADDRESS_MAP_START( pcktgal_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_RAM
	AM_RANGE(0x0800, 0x0fff) AM_RAM_WRITE(pcktgal_videoram_w) AM_BASE_GENERIC(videoram)
	AM_RANGE(0x1000, 0x11ff) AM_RAM AM_BASE_SIZE_GENERIC(spriteram)
	AM_RANGE(0x1800, 0x1800) AM_READ_PORT("P1")
	AM_RANGE(0x1801, 0x1801) AM_WRITE(pcktgal_flipscreen_w)
	/* 1800 - 0x181f are unused BAC-06 registers, see video/dec0.c */
	AM_RANGE(0x1a00, 0x1a00) AM_READ_PORT("P2") AM_WRITE(pcktgal_sound_w)
	AM_RANGE(0x1c00, 0x1c00) AM_READ_PORT("DSW") AM_WRITE(pcktgal_bank_w)
	AM_RANGE(0x4000, 0x5fff) AM_ROMBANK("bank1")
	AM_RANGE(0x6000, 0x7fff) AM_ROMBANK("bank2")
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END


/***************************************************************************/

static ADDRESS_MAP_START( pcktgal_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_RAM
	AM_RANGE(0x0800, 0x0801) AM_DEVWRITE("ym1", ym2203_w)
	AM_RANGE(0x1000, 0x1001) AM_DEVWRITE("ym2", ym3812_w)
	AM_RANGE(0x1800, 0x1800) AM_WRITE(pcktgal_adpcm_data_w)	/* ADPCM data for the MSM5205 chip */
	AM_RANGE(0x2000, 0x2000) AM_WRITE(pcktgal_sound_bank_w)
	AM_RANGE(0x3000, 0x3000) AM_READ(soundlatch_r)
	AM_RANGE(0x3400, 0x3400) AM_DEVREAD("msm", pcktgal_adpcm_reset_r)	/* ? not sure */
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank3")
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END


/***************************************************************************/

static INPUT_PORTS_START( pcktgal )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_DIPSETTING(	0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(	0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(	0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(	0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(	0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Allow 2 Players Game" )
	PORT_DIPSETTING(	0x00, DEF_STR( No ) )
	PORT_DIPSETTING(	0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Time" )
	PORT_DIPSETTING(	0x00, "100" )
	PORT_DIPSETTING(	0x20, "120" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(	0x00, "3" )
	PORT_DIPSETTING(	0x40, "4" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
INPUT_PORTS_END

/***************************************************************************/

static const gfx_layout charlayout =
{
	8,8,	/* 8*8 characters */
	4096,
	4,
	{ 0x10000*8, 0, 0x18000*8, 0x8000*8 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8	 /* every char takes 8 consecutive bytes */
};

static const gfx_layout bootleg_charlayout =
{
	8,8,	/* 8*8 characters */
	4096,
	4,
	{ 0x18000*8, 0x8000*8, 0x10000*8, 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8	 /* every char takes 8 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	16,16,  /* 16*16 sprites */
	1024,   /* 1024 sprites */
	2,	  /* 2 bits per pixel */
	{ 0x8000*8, 0 },
	{ 128+0, 128+1, 128+2, 128+3, 128+4, 128+5, 128+6, 128+7, 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8	/* every char takes 8 consecutive bytes */
};

static const gfx_layout bootleg_spritelayout =
{
	16,16,  /* 16*16 sprites */
	1024,   /* 1024 sprites */
	2,	  /* 2 bits per pixel */
	{ 0x8000*8, 0 },
	{ 128+7, 128+6, 128+5, 128+4, 128+3, 128+2, 128+1, 128+0, 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8	/* every char takes 8 consecutive bytes */
};

static GFXDECODE_START( pcktgal )
	GFXDECODE_ENTRY( "gfx1", 0x00000, charlayout,   256, 16 ) /* chars */
	GFXDECODE_ENTRY( "gfx2", 0x00000, spritelayout,   0,  8 ) /* sprites */
GFXDECODE_END

static GFXDECODE_START( bootleg )
	GFXDECODE_ENTRY( "gfx1", 0x00000, bootleg_charlayout,   256, 16 ) /* chars */
	GFXDECODE_ENTRY( "gfx2", 0x00000, bootleg_spritelayout,   0,  8 ) /* sprites */
GFXDECODE_END

/***************************************************************************/

static const msm5205_interface msm5205_config =
{
	pcktgal_adpcm_int,	/* interrupt function */
	MSM5205_S48_4B		/* 8KHz            */
};

/***************************************************************************/

static MACHINE_DRIVER_START( pcktgal )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M6502, 2000000)
	MDRV_CPU_PROGRAM_MAP(pcktgal_map)
	MDRV_CPU_VBLANK_INT("screen", nmi_line_pulse)

	MDRV_CPU_ADD("audiocpu", M6502, 1500000)
	MDRV_CPU_PROGRAM_MAP(pcktgal_sound_map)
							/* IRQs are caused by the ADPCM chip */
							/* NMIs are caused by the main CPU */

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(pcktgal)
	MDRV_PALETTE_LENGTH(512)

	MDRV_PALETTE_INIT(pcktgal)
	MDRV_VIDEO_START(pcktgal)
	MDRV_VIDEO_UPDATE(pcktgal)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ym1", YM2203, 1500000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.60)

	MDRV_SOUND_ADD("ym2", YM3812, 3000000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MDRV_SOUND_ADD("msm", MSM5205, 384000)
	MDRV_SOUND_CONFIG(msm5205_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.70)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( bootleg )
	MDRV_IMPORT_FROM(pcktgal)
	MDRV_GFXDECODE(bootleg)
MACHINE_DRIVER_END

/***************************************************************************/

ROM_START( pcktgal )
	ROM_REGION( 0x14000, "maincpu", 0 )	 /* 64k for code + 16k for banks */
	ROM_LOAD( "eb04.j7",	   0x10000, 0x4000, CRC(8215d60d) SHA1(ac26dfce7e215be21f2a17f864c5e966b8b8322e) )
	ROM_CONTINUE(			   0x04000, 0xc000)
	/* 4000-7fff is banked but code falls through from 7fff to 8000, so */
	/* I have to load the bank directly at 4000. */

	ROM_REGION( 0x18000, "audiocpu", 0 )	 /* 96k for code + 96k for decrypted opcodes */
	ROM_LOAD( "eb03.f2",	   0x10000, 0x8000, CRC(cb029b02) SHA1(fbb3da08ed05ae73fbeeb13e0e2ff735aaf83db8) )
	ROM_CONTINUE(			   0x08000, 0x8000 )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "eb01.d11",	   0x00000, 0x10000, CRC(63542c3d) SHA1(4f42af99a6d9d4766afe0bebe10d6a97811a0082) )
	ROM_LOAD( "eb02.d12",	   0x10000, 0x10000, CRC(a9dcd339) SHA1(245824ab86cdfe4b842ce1be0af60f2ff4c6ae07) )

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "eb00.a1",	  0x00000, 0x10000, CRC(6c1a14a8) SHA1(03201197304c5f1d854b8c4f4a5c78336b51f872) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "eb05.k14",     0x0000, 0x0200, CRC(3b6198cb) SHA1(d32b364cfce99637998ca83ad21783f80364dd65) ) /* 82s147.084 */
	ROM_LOAD( "eb06.k15",     0x0200, 0x0200, CRC(1fbd4b59) SHA1(84e20329003cf09b849b49e1d83edc330d49f404) ) /* 82s131.101 */
ROM_END

ROM_START( pcktgalb )
	ROM_REGION( 0x14000, "maincpu", 0 )	 /* 64k for code + 16k for banks */
	ROM_LOAD( "sexybill.001", 0x10000, 0x4000, CRC(4acb3e84) SHA1(c83d03969587c6be80fb8fc84afe250907674a44) )
	ROM_CONTINUE(			  0x04000, 0xc000)
	/* 4000-7fff is banked but code falls through from 7fff to 8000, so */
	/* I have to load the bank directly at 4000. */

	ROM_REGION( 0x18000, "audiocpu", 0 )	 /* 96k for code + 96k for decrypted opcodes */
	ROM_LOAD( "eb03.f2",	   0x10000, 0x8000, CRC(cb029b02) SHA1(fbb3da08ed05ae73fbeeb13e0e2ff735aaf83db8) )
	ROM_CONTINUE(			  0x08000, 0x8000 )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "sexybill.005", 0x00000, 0x10000, CRC(3128dc7b) SHA1(d011181e544b8284ecdf54578da5469804e06c63) )
	ROM_LOAD( "sexybill.006", 0x10000, 0x10000, CRC(0fc91eeb) SHA1(9d9a54c8dd41c10d07aabb6a2d8dbaf35c6e4533) )

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "sexybill.003", 0x00000, 0x08000, CRC(58182daa) SHA1(55ce4b0ea2cb1c559c12815c9e453624e0d95515) )
	ROM_LOAD( "sexybill.004", 0x08000, 0x08000, CRC(33a67af6) SHA1(6d9c04658ed75b970821a5c8b1f60c3c08fdda0a) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "eb05.k14",     0x0000, 0x0200, CRC(3b6198cb) SHA1(d32b364cfce99637998ca83ad21783f80364dd65) ) /* 82s147.084 */
	ROM_LOAD( "eb06.k15",     0x0200, 0x0200, CRC(1fbd4b59) SHA1(84e20329003cf09b849b49e1d83edc330d49f404) ) /* 82s131.101 */
ROM_END

ROM_START( pcktgal2 )
	ROM_REGION( 0x14000, "maincpu", 0 )	 /* 64k for code + 16k for banks */
	ROM_LOAD( "eb04-2.j7",   0x10000, 0x4000, CRC(0c7f2905) SHA1(882dbc1888a0149486c1fac5568dc3d297c2dadd) )
	ROM_CONTINUE(			  0x04000, 0xc000)
	/* 4000-7fff is banked but code falls through from 7fff to 8000, so */
	/* I have to load the bank directly at 4000. */

	ROM_REGION( 0x18000, "audiocpu", 0 )	 /* audio cpu */
	ROM_LOAD( "eb03-2.f2",   0x10000, 0x8000, CRC(9408ffb4) SHA1(ddcb67da4acf3d986d54ad10404f213528a8bb62) )
	ROM_CONTINUE(			  0x08000, 0x8000)

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "eb01-2.rom",   0x00000, 0x10000, CRC(e52b1f97) SHA1(4814fe3b2eb08ac173e09ffadc6e5daa9affa1a0) )
	ROM_LOAD( "eb02-2.rom",   0x10000, 0x10000, CRC(f30d965d) SHA1(a787457b33ad39e78fcf8da0715fab7a63869bf9) )

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "eb00.a1",	  0x00000, 0x10000, CRC(6c1a14a8) SHA1(03201197304c5f1d854b8c4f4a5c78336b51f872) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "eb05.k14",     0x0000, 0x0200, CRC(3b6198cb) SHA1(d32b364cfce99637998ca83ad21783f80364dd65) ) /* 82s147.084 */
	ROM_LOAD( "eb06.k15",     0x0200, 0x0200, CRC(1fbd4b59) SHA1(84e20329003cf09b849b49e1d83edc330d49f404) ) /* 82s131.101 */
ROM_END

ROM_START( pcktgal2j )
	ROM_REGION( 0x14000, "maincpu", 0 )	 /* 64k for code + 16k for banks */
	ROM_LOAD( "eb04-2.j7",   0x10000, 0x4000, CRC(0c7f2905) SHA1(882dbc1888a0149486c1fac5568dc3d297c2dadd) )
	ROM_CONTINUE(			  0x04000, 0xc000)
	/* 4000-7fff is banked but code falls through from 7fff to 8000, so */
	/* I have to load the bank directly at 4000. */

	ROM_REGION( 0x18000, "audiocpu", 0 )	 /* audio cpu */
	ROM_LOAD( "eb03-2.f2",   0x10000, 0x8000, CRC(9408ffb4) SHA1(ddcb67da4acf3d986d54ad10404f213528a8bb62) )
	ROM_CONTINUE(			  0x08000, 0x8000)

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "eb01-2.d11",   0x00000, 0x10000, CRC(8f42ab1a) SHA1(315fb26bbe004c08629a0a3a6e9d129768119e6b) )
	ROM_LOAD( "eb02-2.d12",   0x10000, 0x10000, CRC(f394cb35) SHA1(f351b8b6fd8a6637ef9031f7a410a334da8ea5ae) )

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "eb00.a1",	  0x00000, 0x10000, CRC(6c1a14a8) SHA1(03201197304c5f1d854b8c4f4a5c78336b51f872) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "eb05.k14",     0x0000, 0x0200, CRC(3b6198cb) SHA1(d32b364cfce99637998ca83ad21783f80364dd65) ) /* 82s147.084 */
	ROM_LOAD( "eb06.k15",     0x0200, 0x0200, CRC(1fbd4b59) SHA1(84e20329003cf09b849b49e1d83edc330d49f404) ) /* 82s131.101 */
ROM_END

ROM_START( spool3 )
	ROM_REGION( 0x14000, "maincpu", 0 )	 /* 64k for code + 16k for banks */
	ROM_LOAD( "eb04-2.j7",   0x10000, 0x4000, CRC(0c7f2905) SHA1(882dbc1888a0149486c1fac5568dc3d297c2dadd) )
	ROM_CONTINUE(			  0x04000, 0xc000)
	/* 4000-7fff is banked but code falls through from 7fff to 8000, so */
	/* I have to load the bank directly at 4000. */

	ROM_REGION( 0x18000, "audiocpu", 0 )	 /* audio cpu */
	ROM_LOAD( "eb03-2.f2",   0x10000, 0x8000, CRC(9408ffb4) SHA1(ddcb67da4acf3d986d54ad10404f213528a8bb62) )
	ROM_CONTINUE(			  0x08000, 0x8000)

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "deco2.bin",	  0x00000, 0x10000, CRC(0a23f0cf) SHA1(8554215001ffc9e6f141e57cc11b400a853f89f2) )
	ROM_LOAD( "deco3.bin",	  0x10000, 0x10000, CRC(55ea7c45) SHA1(a8a6ff0c8a5aaee3afbfc3e71a171fb1d2360b45) )

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "eb00.a1",	  0x00000, 0x10000, CRC(6c1a14a8) SHA1(03201197304c5f1d854b8c4f4a5c78336b51f872) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "eb05.k14",     0x0000, 0x0200, CRC(3b6198cb) SHA1(d32b364cfce99637998ca83ad21783f80364dd65) ) /* 82s147.084 */
	ROM_LOAD( "eb06.k15",     0x0200, 0x0200, CRC(1fbd4b59) SHA1(84e20329003cf09b849b49e1d83edc330d49f404) ) /* 82s131.101 */
ROM_END

ROM_START( spool3i )
	ROM_REGION( 0x14000, "maincpu", 0 )	 /* 64k for code + 16k for banks */
	ROM_LOAD( "de1.bin",	  0x10000, 0x4000, CRC(a59980fe) SHA1(64b55af4d0b314d14184784e9f817b56be0f24f2) )
	ROM_CONTINUE(			  0x04000, 0xc000)
	/* 4000-7fff is banked but code falls through from 7fff to 8000, so */
	/* I have to load the bank directly at 4000. */

	ROM_REGION( 0x18000, "audiocpu", 0 )	 /* audio cpu */
	ROM_LOAD( "eb03-2.f2",   0x10000, 0x8000, CRC(9408ffb4) SHA1(ddcb67da4acf3d986d54ad10404f213528a8bb62) )
	ROM_CONTINUE(			  0x08000, 0x8000)

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "deco2.bin",	  0x00000, 0x10000, CRC(0a23f0cf) SHA1(8554215001ffc9e6f141e57cc11b400a853f89f2) )
	ROM_LOAD( "deco3.bin",	  0x10000, 0x10000, CRC(55ea7c45) SHA1(a8a6ff0c8a5aaee3afbfc3e71a171fb1d2360b45) )

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "eb00.a1",	  0x00000, 0x10000, CRC(6c1a14a8) SHA1(03201197304c5f1d854b8c4f4a5c78336b51f872) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "eb05.k14",     0x0000, 0x0200, CRC(3b6198cb) SHA1(d32b364cfce99637998ca83ad21783f80364dd65) ) /* 82s147.084 */
	ROM_LOAD( "eb06.k15",     0x0200, 0x0200, CRC(1fbd4b59) SHA1(84e20329003cf09b849b49e1d83edc330d49f404) ) /* 82s131.101 */
ROM_END

/***************************************************************************/

static DRIVER_INIT( deco222 )
{
	int A;
	const address_space *space = cputag_get_address_space(machine, "audiocpu", ADDRESS_SPACE_PROGRAM);
	UINT8 *decrypted = auto_alloc_array(machine, UINT8, 0x10000);
	UINT8 *rom = memory_region(machine, "audiocpu");

	memory_set_decrypted_region(space, 0x8000, 0xffff, decrypted);

	/* bits 5 and 6 of the opcodes are swapped */
	for (A = 0x8000;A < 0x18000;A++)
		decrypted[A-0x8000] = (rom[A] & 0x9f) | ((rom[A] & 0x20) << 1) | ((rom[A] & 0x40) >> 1);

	memory_configure_bank(machine, "bank3", 0, 2, memory_region(machine, "audiocpu") + 0x10000, 0x4000);
	memory_configure_bank_decrypted(machine, "bank3", 0, 2, &decrypted[0x8000], 0x4000);
}

static DRIVER_INIT( graphics )
{
	UINT8 *rom = memory_region(machine, "gfx1");
	int len = memory_region_length(machine, "gfx1");
	int i,j,temp[16];

	memory_configure_bank(machine, "bank3", 0, 2, memory_region(machine, "audiocpu") + 0x10000, 0x4000);

	/* Tile graphics roms have some swapped lines, original version only */
	for (i = 0x00000;i < len;i += 32)
	{
		for (j=0; j<16; j++)
		{
			temp[j] = rom[i+j+16];
			rom[i+j+16] = rom[i+j];
			rom[i+j] = temp[j];
		}
	}
}

static DRIVER_INIT( pcktgal )
{
	DRIVER_INIT_CALL(deco222);
	DRIVER_INIT_CALL(graphics);
}

/***************************************************************************/

GAME( 1987, pcktgal,  0,       pcktgal, pcktgal, pcktgal,  ROT0, "Data East Corporation", "Pocket Gal (Japan)", 0 )
GAME( 1987, pcktgalb, pcktgal, bootleg, pcktgal, deco222,  ROT0, "bootleg", "Pocket Gal (bootleg)", 0 )
GAME( 1989, pcktgal2, pcktgal, pcktgal, pcktgal, graphics, ROT0, "Data East Corporation", "Pocket Gal 2 (English)", 0 )
GAME( 1989, pcktgal2j,pcktgal, pcktgal, pcktgal, graphics, ROT0, "Data East Corporation", "Pocket Gal 2 (Japanese)", 0 )
GAME( 1989, spool3,   pcktgal, pcktgal, pcktgal, graphics, ROT0, "Data East Corporation", "Super Pool III (English)", 0 )
GAME( 1990, spool3i,  pcktgal, pcktgal, pcktgal, graphics, ROT0, "Data East Corporation (I-Vics license)", "Super Pool III (I-Vics)", 0 )
