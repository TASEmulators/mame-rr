/***************************************************************************

Grand Cross Pinball
===================

Made from Raine source


Code
----

Inputs get tested at $4aca2 on


TODO
----

Screen flipping support

Understand role of bit 5 of IN1

Eprom?

BGMs (controlled by MSM-6585 sound chip)

Stephh's notes (based on the game M68000 code and some tests) :

  - Reset the game while pressing START1 to enter the "test mode"


***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"
#include "sound/msm5205.h"
#include "includes/gcpinbal.h"



/***********************************************************
                      INTERRUPTS
***********************************************************/

static TIMER_CALLBACK( gcpinbal_interrupt1 )
{
	gcpinbal_state *state = (gcpinbal_state *)machine->driver_data;
	cpu_set_input_line(state->maincpu, 1, HOLD_LINE);
}

#ifdef UNUSED_FUNCTION
static TIMER_CALLBACK( gcpinbal_interrupt3 )
{
	gcpinbal_state *state = (gcpinbal_state *)machine->driver_data;
	// IRQ3 is from the M6585
//  if (!ADPCM_playing(0))
	{
		cpu_set_input_line(state->maincpu, 3, HOLD_LINE);
	}
}
#endif

static INTERRUPT_GEN( gcpinbal_interrupt )
{
	/* Unsure of actual sequence */

	timer_set(device->machine, downcast<cpu_device *>(device)->cycles_to_attotime(500), NULL, 0, gcpinbal_interrupt1);
//  timer_set(device->machine, downcast<cpu_device *>(device)->cycles_to_attotime(1000), NULL, 0, gcpinbal_interrupt3);
	cpu_set_input_line(device, 4, HOLD_LINE);
}


/***********************************************************
                          IOC
***********************************************************/

static READ16_HANDLER( ioc_r )
{
	gcpinbal_state *state = (gcpinbal_state *)space->machine->driver_data;

	/* 20 (only once), 76, a0 are read in log */

	switch (offset)
	{
		case 0x80/2:
			return input_port_read(space->machine, "DSW");

		case 0x84/2:
			return input_port_read(space->machine, "IN0");

		case 0x86/2:
			return input_port_read(space->machine, "IN1");

		case 0x50:
		case 0x51:
			return okim6295_r(state->oki, 0) << 8;

	}

//logerror("CPU #0 PC %06x: warning - read unmapped ioc offset %06x\n",cpu_get_pc(space->cpu),offset);

	return state->ioc_ram[offset];
}


static WRITE16_HANDLER( ioc_w )
{
	gcpinbal_state *state = (gcpinbal_state *)space->machine->driver_data;
	COMBINE_DATA(&state->ioc_ram[offset]);

//  switch (offset)
//  {
//      case 0x??:  /* */
//          return;
//
//      case 0x88/2:    /* coin control (+ others) ??? */
//          coin_lockout_w(space->machine, 0, ~data & 0x01);
//          coin_lockout_w(space->machine, 1, ~data & 0x02);
//popmessage(" address %04x value %04x", offset, data);
//  }

	switch (offset)
	{
		// these are all written every frame
		case 0x3b:
		case 0xa:
		case 0xc:
		case 0xb:
		case 0xd:
		case 0xe:
		case 0xf:
		case 0x10:
		case 0x47:
			break;

		// MSM6585 bank, coin LEDs, maybe others?
		case 0x44:
			state->msm_bank = data & 0x1000 ? 0x100000 : 0;
			state->oki->set_bank_base(0x40000 * ((data & 0x800 )>> 11));
			break;

		case 0x45:
			//state->adpcm_idle = 1;
			break;

		// OKIM6295
		case 0x50:
		case 0x51:
			okim6295_w(state->oki, 0, data >> 8);
			break;

		// MSM6585 ADPCM - mini emulation
		case 0x60:
			state->msm_start &= 0xffff00;
			state->msm_start |= (data >> 8);
			break;
		case 0x61:
			state->msm_start &= 0xff00ff;
			state->msm_start |= data;
			break;
		case 0x62:
			state->msm_start &= 0x00ffff;
			state->msm_start |= (data << 8);
			break;
		case 0x63:
			state->msm_end &= 0xffff00;
			state->msm_end |= (data >> 8);
			break;
		case 0x64:
			state->msm_end &= 0xff00ff;
			state->msm_end |= data;
			break;
		case 0x65:
			state->msm_end &= 0x00ffff;
			state->msm_end |= (data << 8);
			break;
		case 0x66:
			if (state->msm_start < state->msm_end)
			{
				/* data written here is adpcm param? */
				//popmessage("%08x %08x", state->msm_start + state->msm_bank, state->msm_end);
				state->adpcm_idle = 0;
				msm5205_reset_w(state->msm, 0);
				state->adpcm_start = state->msm_start + state->msm_bank;
				state->adpcm_end = state->msm_end;
//              ADPCM_stop(0);
//              ADPCM_play(0, start+bank, end-start);
			}
			break;

		default:
			logerror("CPU #0 PC %06x: warning - write ioc offset %06x with %04x\n", cpu_get_pc(space->cpu), offset, data);
			break;
	}

}


/************************************************
                      SOUND
************************************************/


/* Controlled through ioc? */
static void gcp_adpcm_int( running_device *device )
{
	gcpinbal_state *state = (gcpinbal_state *)device->machine->driver_data;

	if (state->adpcm_idle)
		msm5205_reset_w(device, 1);
	if (state->adpcm_start >= 0x200000 || state->adpcm_start > state->adpcm_end)
	{
		//msm5205_reset_w(device,1);
		state->adpcm_start = state->msm_start + state->msm_bank;
		state->adpcm_trigger = 0;
	}
	else
	{
		UINT8 *ROM = memory_region(device->machine, "msm");

		state->adpcm_data = ((state->adpcm_trigger ? (ROM[state->adpcm_start] & 0x0f) : (ROM[state->adpcm_start] & 0xf0) >> 4));
		msm5205_data_w(device, state->adpcm_data & 0xf);
		state->adpcm_trigger ^= 1;
		if (state->adpcm_trigger == 0)
			state->adpcm_start++;
	}
}


/***********************************************************
                     MEMORY STRUCTURES
***********************************************************/

static ADDRESS_MAP_START( gcpinbal_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM
	AM_RANGE(0xc00000, 0xc03fff) AM_READWRITE(gcpinbal_tilemaps_word_r, gcpinbal_tilemaps_word_w) AM_BASE_MEMBER(gcpinbal_state, tilemapram)
	AM_RANGE(0xc80000, 0xc80fff) AM_RAM AM_BASE_SIZE_MEMBER(gcpinbal_state, spriteram, spriteram_size)	/* sprite ram */
	AM_RANGE(0xd00000, 0xd00fff) AM_RAM_WRITE(paletteram16_RRRRGGGGBBBBRGBx_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0xd80000, 0xd800ff) AM_READWRITE(ioc_r, ioc_w) AM_BASE_MEMBER(gcpinbal_state, ioc_ram)
	AM_RANGE(0xff0000, 0xffffff) AM_RAM	/* RAM */
ADDRESS_MAP_END



/***********************************************************
                   INPUT PORTS, DIPs
***********************************************************/

static INPUT_PORTS_START( gcpinbal )
	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(      0x0004, "300k" )
	PORT_DIPSETTING(      0x0008, "500k" )
	PORT_DIPSETTING(      0x000c, "1000k" )
	PORT_DIPSETTING(      0x0000, DEF_STR( None ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Flip_Screen ) )	// to be confirmed - code at 0x000508
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Demo_Sounds ) )	// to be confirmed - code at 0x00b6d0, 0x00b7e4, 0x00bae4
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0100, "2 Coins/1 Credit 3/2 4/3 6/5" )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0500, "1 Coin/1 Credit 2/3" )
	PORT_DIPSETTING(      0x0000, "1 Coin/1 Credit 5/6" )
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x3800, 0x3800, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0800, "2 Coins/1 Credit 3/2 4/3 6/5" )
	PORT_DIPSETTING(      0x3800, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x2800, "1 Coin/1 Credit 2/3" )
	PORT_DIPSETTING(      0x0000, "1 Coin/1 Credit 5/6" )
	PORT_DIPSETTING(      0x3000, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0xc000, "3" )
	PORT_DIPSETTING(      0x8000, "4" )
	PORT_DIPSETTING(      0x4000, "5" )

	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)	// Item right
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)	// Inner flipper right
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)	// Outer flipper right
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_PLAYER(1)	// Tilt right
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)	// Item left
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)	// Inner flipper left
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)	// Outer flipper left
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(1)	// Tilt left
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )	// This bit gets tested (search for d8 00 87)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END



/**************************************************************
                       GFX DECODING
**************************************************************/

static const gfx_layout charlayout =
{
	16,16,	/* 16*16 characters */
	RGN_FRAC(1,1),
	4,	/* 4 bits per pixel */
	{ 0, 1, 2, 3 },
	{ 2*4, 3*4, 0*4, 1*4, 6*4, 7*4, 4*4, 5*4, 2*4+32, 3*4+32, 0*4+32, 1*4+32, 6*4+32, 7*4+32, 4*4+32, 5*4+32 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64, 8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	128*8	/* every sprite takes 128 consecutive bytes */
};

static const gfx_layout char_8x8_layout =
{
	8,8,	/* 8*8 characters */
	RGN_FRAC(1,1),
	4,	/* 4 bits per pixel */
	{ 0, 1, 2, 3 },
	{ 2*4, 3*4, 0*4, 1*4, 6*4, 7*4, 4*4, 5*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8	/* every sprite takes 32 consecutive bytes */
};

static const gfx_layout tilelayout =
{
	16,16,	/* 16*16 sprites */
	RGN_FRAC(1,1),
	4,	/* 4 bits per pixel */
//  { 16, 48, 0, 32 },
	{ 48, 16, 32, 0 },
	{ 8, 9, 10, 11, 12, 13, 14, 15, 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64, 8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	128*8	/* every sprite takes 128 consecutive bytes */
};

static GFXDECODE_START( gcpinbal )
	GFXDECODE_ENTRY( "gfx3", 0, tilelayout,       0, 256 )	/* sprites & playfield */
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,       0, 256 )	/* sprites & playfield */
	GFXDECODE_ENTRY( "gfx2", 0, char_8x8_layout,  0, 256 )	/* sprites & playfield */
GFXDECODE_END


/**************************************************************
                            (SOUND)
**************************************************************/

static const msm5205_interface msm5205_config =
{
	gcp_adpcm_int,		/* VCK function */
	MSM5205_S48_4B		/* 8 kHz */
};

/***********************************************************
                        MACHINE DRIVERS
***********************************************************/

static MACHINE_START( gcpinbal )
{
	gcpinbal_state *state = (gcpinbal_state *)machine->driver_data;

	state_save_register_global_array(machine, state->scrollx);
	state_save_register_global_array(machine, state->scrolly);
	state_save_register_global(machine, state->bg0_gfxset);
	state_save_register_global(machine, state->bg1_gfxset);
	state_save_register_global(machine, state->msm_start);
	state_save_register_global(machine, state->msm_end);
	state_save_register_global(machine, state->msm_bank);
	state_save_register_global(machine, state->adpcm_start);
	state_save_register_global(machine, state->adpcm_end);
	state_save_register_global(machine, state->adpcm_idle);
	state_save_register_global(machine, state->adpcm_trigger);
	state_save_register_global(machine, state->adpcm_data);
}

static MACHINE_RESET( gcpinbal )
{
	gcpinbal_state *state = (gcpinbal_state *)machine->driver_data;
	int i;

	for (i = 0; i < 3; i++)
	{
		state->scrollx[i] = 0;
		state->scrolly[i] = 0;
	}

	state->adpcm_idle = 1;
	state->adpcm_start = 0;
	state->adpcm_end = 0;
	state->adpcm_trigger = 0;
	state->adpcm_data = 0;
	state->bg0_gfxset = 0;
	state->bg1_gfxset = 0;
	state->msm_start = 0;
	state->msm_end = 0;
	state->msm_bank = 0;
}

static MACHINE_DRIVER_START( gcpinbal )

	/* driver data */
	MDRV_DRIVER_DATA(gcpinbal_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 32000000/2)	/* 16 MHz ? */
	MDRV_CPU_PROGRAM_MAP(gcpinbal_map)
	MDRV_CPU_VBLANK_INT("screen", gcpinbal_interrupt)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0)	/* frames per second, vblank duration */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(40*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 2*8, 30*8-1)

	MDRV_MACHINE_START(gcpinbal)
	MDRV_MACHINE_RESET(gcpinbal)

	MDRV_GFXDECODE(gcpinbal)
	MDRV_PALETTE_LENGTH(4096)

	MDRV_VIDEO_START(gcpinbal)
	MDRV_VIDEO_UPDATE(gcpinbal)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_OKIM6295_ADD("oki", 1056000, OKIM6295_PIN7_HIGH)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	MDRV_SOUND_ADD("msm", MSM5205, 384000)
	MDRV_SOUND_CONFIG(msm5205_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END



/***************************************************************************
                                  DRIVERS
***************************************************************************/

ROM_START( gcpinbal )
	ROM_REGION( 0x200000, "maincpu", 0 )     /* 512k for 68000 program */
	ROM_LOAD16_WORD_SWAP( "u43.2",  0x000000, 0x80000, CRC(d174bd7f) SHA1(0e6c17265e1400de941e3e2ca3be835aaaff6695) )
	ROM_FILL            ( 0x80000,  0x080000, 0x0 )
	ROM_LOAD16_WORD_SWAP( "u45.3",  0x100000, 0x80000, CRC(0511ad56) SHA1(e0602ece514126ce719ebc9de6649ebe907be904) )
	ROM_LOAD16_WORD_SWAP( "u46.4",  0x180000, 0x80000, CRC(e0f3a1b4) SHA1(761dddf374a92c1a1e4a211ead215d5be461a082) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "u1",      0x000000, 0x100000, CRC(afa459bb) SHA1(7a7c64bcb80d71b8cf3fdd3209ef109997b6417c) )	/* BG0 (16 x 16) */
	ROM_LOAD( "u6",      0x100000, 0x100000, CRC(c3f024e5) SHA1(d197e2b715b154fc64ff9a61f8c6df111d6fd446) )

	ROM_REGION( 0x020000, "gfx2", 0 )
	ROM_LOAD( "u10.1",   0x000000, 0x020000, CRC(79321550) SHA1(61f1b772ed8cf95bfee9df8394b0c3ff727e8702) )	/* FG0 (8 x 8) */

	ROM_REGION( 0x200000, "gfx3", 0 )
	ROM_LOAD( "u13",     0x000000, 0x200000, CRC(62f3952f) SHA1(7dc9ccb753d46b6aaa791bcbf6e18e6d872f6b79) )	/* Sprites (16 x 16) */

	ROM_REGION( 0x080000, "oki", 0 )	/* M6295 acc to Raine */
	ROM_LOAD( "u55",   0x000000, 0x080000, CRC(b3063351) SHA1(825e63e8a824d67d235178897528e5b0b41e4485) )

	ROM_REGION( 0x200000, "msm", 0 )	/* M6585 acc to Raine */
	ROM_LOAD( "u56",   0x000000, 0x200000, CRC(092b2c0f) SHA1(2ec1904e473ddddb50dbeaa0b561642064d45336) )
ROM_END



GAME( 1994, gcpinbal, 0, gcpinbal, gcpinbal, 0, ROT270, "Excellent System", "Grand Cross", GAME_IMPERFECT_SOUND | GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
