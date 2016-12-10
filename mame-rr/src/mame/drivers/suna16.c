/***************************************************************************

                            -=  SunA 16 Bit Games =-

                    driver by   Luca Elia (l.elia@tin.it)


CPU:    68000   +  Z80 [Music]  +  Z80 x 2 [4 Bit PCM]
Sound:  YM2151  +  DAC x 4


-------------------------------------------------------------------------------------------
Year + Game                 By      Board      Hardware
-------------------------------------------------------------------------------------------
94  Best Of Best            SunA    KRB-0026   68000 + Z80 x 2 + YM3526 + DAC x 4 + AY-8910
94  Suna Quiz 6000 Academy  SunA    KRB-0027A  68000 + Z80 x 2 + YM2151 + DAC x 2
96  Ultra Balloon           SunA               68000 + Z80 x 2 + YM2151 + DAC x 2
96  Back Street Soccer      SunA               68000 + Z80 x 3 + YM2151 + DAC x 4
-------------------------------------------------------------------------------------------


***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "deprecat.h"
#include "sound/dac.h"
#include "sound/2151intf.h"
#include "sound/ay8910.h"
#include "sound/3526intf.h"

/* Variables and functions defined in video: */

WRITE16_HANDLER( suna16_flipscreen_w );
WRITE16_HANDLER( bestbest_flipscreen_w );

READ16_HANDLER ( suna16_paletteram16_r );
WRITE16_HANDLER( suna16_paletteram16_w );

VIDEO_START( suna16 );
VIDEO_UPDATE( suna16 );
VIDEO_UPDATE( bestbest );


/***************************************************************************


                                Main CPU


***************************************************************************/

static WRITE16_HANDLER( suna16_soundlatch_w )
{
	if (ACCESSING_BITS_0_7)
	{
		soundlatch_w( space, 0, data & 0xff );
	}
	if (data & ~0xff)	logerror("CPU#0 PC %06X - Sound latch unknown bits: %04X\n", cpu_get_pc(space->cpu), data);
}


static WRITE16_HANDLER( bssoccer_leds_w )
{
	if (ACCESSING_BITS_0_7)
	{
		set_led_status(space->machine, 0, data & 0x01);
		set_led_status(space->machine, 1, data & 0x02);
		set_led_status(space->machine, 2, data & 0x04);
		set_led_status(space->machine, 3, data & 0x08);
		coin_counter_w(space->machine, 0, data & 0x10);
	}
	if (data & ~0x1f)	logerror("CPU#0 PC %06X - Leds unknown bits: %04X\n", cpu_get_pc(space->cpu), data);
}


static WRITE16_HANDLER( uballoon_leds_w )
{
	if (ACCESSING_BITS_0_7)
	{
		coin_counter_w(space->machine, 0, data & 0x01);
		set_led_status(space->machine, 0, data & 0x02);
		set_led_status(space->machine, 1, data & 0x04);
	}
	if (data & ~0x07)	logerror("CPU#0 PC %06X - Leds unknown bits: %04X\n", cpu_get_pc(space->cpu), data);
}


static WRITE16_HANDLER( bestbest_coin_w )
{
	if (ACCESSING_BITS_0_7)
	{
		coin_counter_w(space->machine, 0, data & 0x04);
	}
	if (data & ~0x04)	logerror("CPU#0 PC %06X - Leds unknown bits: %04X\n", cpu_get_pc(space->cpu), data);
}


/***************************************************************************
                            Back Street Soccer
***************************************************************************/

static ADDRESS_MAP_START( bssoccer_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM	// ROM
	AM_RANGE(0x200000, 0x203fff) AM_RAM	// RAM
	AM_RANGE(0x400000, 0x4001ff) AM_READWRITE(suna16_paletteram16_r, suna16_paletteram16_w)  // Banked Palette
	AM_RANGE(0x400200, 0x400fff) AM_RAM	//
	AM_RANGE(0x600000, 0x61ffff) AM_RAM AM_BASE_GENERIC(spriteram)	// Sprites
	AM_RANGE(0xa00000, 0xa00001) AM_READ_PORT("P1") AM_WRITE(suna16_soundlatch_w)	// To Sound CPU
	AM_RANGE(0xa00002, 0xa00003) AM_READ_PORT("P2") AM_WRITE(suna16_flipscreen_w)	// Flip Screen
	AM_RANGE(0xa00004, 0xa00005) AM_READ_PORT("P3") AM_WRITE(bssoccer_leds_w)	// Leds
	AM_RANGE(0xa00006, 0xa00007) AM_READ_PORT("P4") AM_WRITENOP	// ? IRQ 1 Ack
	AM_RANGE(0xa00008, 0xa00009) AM_READ_PORT("DSW1") AM_WRITENOP	// ? IRQ 2 Ack
	AM_RANGE(0xa0000a, 0xa0000b) AM_READ_PORT("DSW2")
ADDRESS_MAP_END


/***************************************************************************
                                Ultra Balloon
***************************************************************************/

static ADDRESS_MAP_START( uballoon_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM	// ROM
	AM_RANGE(0x800000, 0x803fff) AM_RAM	// RAM
	AM_RANGE(0x200000, 0x2001ff) AM_READWRITE(suna16_paletteram16_r, suna16_paletteram16_w)	// Banked Palette
	AM_RANGE(0x200200, 0x200fff) AM_RAM	//
	AM_RANGE(0x400000, 0x41ffff) AM_MIRROR(0x1e0000) AM_RAM AM_BASE_GENERIC(spriteram)	// Sprites
	AM_RANGE(0x600000, 0x600001) AM_READ_PORT("P1") AM_WRITE(suna16_soundlatch_w)	// To Sound CPU
	AM_RANGE(0x600002, 0x600003) AM_READ_PORT("P2")
	AM_RANGE(0x600004, 0x600005) AM_READ_PORT("DSW1") AM_WRITE(suna16_flipscreen_w)	// Flip Screen
	AM_RANGE(0x600006, 0x600007) AM_READ_PORT("DSW2")
	AM_RANGE(0x600008, 0x600009) AM_WRITE(uballoon_leds_w)	// Leds
	AM_RANGE(0x60000c, 0x60000d) AM_WRITENOP	// ? IRQ 1 Ack
	AM_RANGE(0x600010, 0x600011) AM_WRITENOP	// ? IRQ 1 Ack
	AM_RANGE(0xa00000, 0xa0ffff) AM_NOP			// Protection
ADDRESS_MAP_END


/***************************************************************************
                            Suna Quiz 6000 Academy
***************************************************************************/

static ADDRESS_MAP_START( sunaq_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM	// ROM
	AM_RANGE(0x500000, 0x500001) AM_READ_PORT("P1") AM_WRITE(suna16_soundlatch_w)	// To Sound CPU
	AM_RANGE(0x500002, 0x500003) AM_READ_PORT("P2") AM_WRITE(suna16_flipscreen_w)	// Flip Screen
	AM_RANGE(0x500004, 0x500005) AM_READ_PORT("DSW1")
	AM_RANGE(0x500006, 0x500007) AM_READ_PORT("DSW2")				// (unused?)
	AM_RANGE(0x540000, 0x5401ff) AM_READWRITE(suna16_paletteram16_r, suna16_paletteram16_w)
	AM_RANGE(0x540200, 0x540fff) AM_RAM   // RAM
	AM_RANGE(0x580000, 0x583fff) AM_RAM	// RAM
	AM_RANGE(0x5c0000, 0x5dffff) AM_RAM AM_BASE_GENERIC(spriteram)	// Sprites
ADDRESS_MAP_END


/***************************************************************************
                            Best Of Best
***************************************************************************/

static UINT16 prot;

static READ16_HANDLER( bestbest_prot_r )
{
	return prot;
}

static WRITE16_HANDLER( bestbest_prot_w )
{
	if (ACCESSING_BITS_0_7)
	{
		switch (data & 0xff)
		{
			case 0x00:	prot = prot ^ 0x0009;	break;
			case 0x08:	prot = prot ^ 0x0002;	break;
			case 0x0c:	prot = prot ^ 0x0003;	break;
//          default:    logerror("CPU#0 PC %06X - Unknown protection value: %04X\n", cpu_get_pc(space->cpu), data);
		}
	}
}


static ADDRESS_MAP_START( bestbest_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE( 0x000000, 0x03ffff ) AM_ROM AM_MIRROR(0xc0000)		// ROM
	AM_RANGE( 0x200000, 0x2fffff ) AM_ROM AM_REGION("user1", 0)		// ROM
	AM_RANGE( 0x500000, 0x500001 ) AM_READ_PORT("P1") AM_WRITE(suna16_soundlatch_w)		// To Sound CPU
	AM_RANGE( 0x500002, 0x500003 ) AM_READ_PORT("P2") AM_WRITE(bestbest_flipscreen_w)	// P2 + Coins, Flip Screen
	AM_RANGE( 0x500004, 0x500005 ) AM_READ_PORT("DSW") AM_WRITE(bestbest_coin_w)		// Coin Counter
	AM_RANGE( 0x500008, 0x500009 ) AM_WRITE( bestbest_prot_w )		// Protection
	AM_RANGE( 0x500018, 0x500019 ) AM_READ ( bestbest_prot_r )		//
	AM_RANGE( 0x540000, 0x540fff ) AM_READWRITE( suna16_paletteram16_r, suna16_paletteram16_w )	// Banked(?) Palette
	AM_RANGE( 0x541000, 0x54ffff ) AM_RAM														//
	AM_RANGE( 0x580000, 0x58ffff ) AM_RAM							// RAM
	AM_RANGE( 0x5c0000, 0x5dffff ) AM_RAM AM_BASE_GENERIC( spriteram  )	// Sprites (Chip 1)
	AM_RANGE( 0x5e0000, 0x5fffff ) AM_RAM AM_BASE_GENERIC( spriteram2 )	// Sprites (Chip 2)
ADDRESS_MAP_END


/***************************************************************************


                                    Z80 #1

        Plays the music (YM2151) and controls the 2 Z80s in charge
        of playing the PCM samples


***************************************************************************/

/***************************************************************************
                            Back Street Soccer
***************************************************************************/

static ADDRESS_MAP_START( bssoccer_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM	// ROM
	AM_RANGE(0xf000, 0xf7ff) AM_RAM	// RAM
	AM_RANGE(0xf800, 0xf801) AM_DEVREADWRITE("ymsnd", ym2151_r, ym2151_w)	// YM2151
	AM_RANGE(0xfc00, 0xfc00) AM_READ(soundlatch_r)	// From Main CPU
	AM_RANGE(0xfd00, 0xfd00) AM_WRITE(soundlatch2_w)	// To PCM Z80 #1
	AM_RANGE(0xfe00, 0xfe00) AM_WRITE(soundlatch3_w)	// To PCM Z80 #2
ADDRESS_MAP_END

/***************************************************************************
                                Ultra Balloon
***************************************************************************/

static ADDRESS_MAP_START( uballoon_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xefff) AM_ROM	// ROM
	AM_RANGE(0xf000, 0xf7ff) AM_RAM	// RAM
	AM_RANGE(0xf800, 0xf801) AM_DEVREADWRITE("ymsnd", ym2151_r, ym2151_w)	// YM2151
	AM_RANGE(0xfc00, 0xfc00) AM_READWRITE(soundlatch_r, soundlatch2_w)	// To PCM Z80
ADDRESS_MAP_END

/***************************************************************************
                            Suna Quiz 6000 Academy
***************************************************************************/

static ADDRESS_MAP_START( sunaq_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xe82f) AM_ROM	// ROM
	AM_RANGE(0xe830, 0xf7ff) AM_RAM	// RAM (writes to efxx, could be a program bug tho)
	AM_RANGE(0xf800, 0xf801) AM_DEVREADWRITE("ymsnd", ym2151_r, ym2151_w)	// YM2151
	AM_RANGE(0xfc00, 0xfc00) AM_READWRITE(soundlatch_r, soundlatch2_w)	// To PCM Z80
ADDRESS_MAP_END

/***************************************************************************
                            Best Of Best
***************************************************************************/

static ADDRESS_MAP_START( bestbest_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE( 0x0000, 0xbfff ) AM_ROM									// ROM
	AM_RANGE( 0xc000, 0xc001 ) AM_DEVWRITE( "ymsnd", ym3526_w	)	//
	AM_RANGE( 0xc002, 0xc003 ) AM_DEVWRITE( "aysnd", ay8910_address_data_w	)	// AY8910
	AM_RANGE( 0xe000, 0xe7ff ) AM_RAM									// RAM
	AM_RANGE( 0xf000, 0xf000 ) AM_WRITE( soundlatch2_w				)	// To PCM Z80
	AM_RANGE( 0xf800, 0xf800 ) AM_READ ( soundlatch_r				)	// From Main CPU
ADDRESS_MAP_END

/***************************************************************************


                                Z80 #2 & #3

        Dumb PCM samples players (e.g they don't even have RAM!)


***************************************************************************/

/***************************************************************************
                            Back Street Soccer
***************************************************************************/

/* Bank Switching */

static WRITE8_HANDLER( bssoccer_pcm_1_bankswitch_w )
{
	UINT8 *RAM = memory_region(space->machine, "pcm1");
	int bank = data & 7;
	if (bank & ~7)	logerror("CPU#2 PC %06X - ROM bank unknown bits: %02X\n", cpu_get_pc(space->cpu), data);
	memory_set_bankptr(space->machine, "bank1", &RAM[bank * 0x10000 + 0x1000]);
}

static WRITE8_HANDLER( bssoccer_pcm_2_bankswitch_w )
{
	UINT8 *RAM = memory_region(space->machine, "pcm2");
	int bank = data & 7;
	if (bank & ~7)	logerror("CPU#3 PC %06X - ROM bank unknown bits: %02X\n", cpu_get_pc(space->cpu), data);
	memory_set_bankptr(space->machine, "bank2", &RAM[bank * 0x10000 + 0x1000]);
}



/* Memory maps: Yes, *no* RAM */

static ADDRESS_MAP_START( bssoccer_pcm_1_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_ROM	// ROM
	AM_RANGE(0x1000, 0xffff) AM_ROMBANK("bank1")	// Banked ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( bssoccer_pcm_2_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_ROM	// ROM
	AM_RANGE(0x1000, 0xffff) AM_ROMBANK("bank2")	// Banked ROM
ADDRESS_MAP_END



/* 2 DACs per CPU - 4 bits per sample */

static WRITE8_DEVICE_HANDLER( bssoccer_DAC_w )
{
	dac_data_w( device, (data & 0xf) * 0x11 );
}

static ADDRESS_MAP_START( bssoccer_pcm_1_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ(soundlatch2_r)	// From The Sound Z80
	AM_RANGE(0x00, 0x00) AM_DEVWRITE("dac1", bssoccer_DAC_w)	// 2 x DAC
	AM_RANGE(0x01, 0x01) AM_DEVWRITE("dac2", bssoccer_DAC_w)	// 2 x DAC
	AM_RANGE(0x03, 0x03) AM_WRITE(bssoccer_pcm_1_bankswitch_w)	// Rom Bank
ADDRESS_MAP_END

static ADDRESS_MAP_START( bssoccer_pcm_2_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ(soundlatch3_r)	// From The Sound Z80
	AM_RANGE(0x00, 0x00) AM_DEVWRITE("dac3", bssoccer_DAC_w)	// 2 x DAC
	AM_RANGE(0x01, 0x01) AM_DEVWRITE("dac4", bssoccer_DAC_w)	// 2 x DAC
	AM_RANGE(0x03, 0x03) AM_WRITE(bssoccer_pcm_2_bankswitch_w)	// Rom Bank
ADDRESS_MAP_END


/***************************************************************************
                                Ultra Balloon
***************************************************************************/

/* Bank Switching */

static WRITE8_HANDLER( uballoon_pcm_1_bankswitch_w )
{
	UINT8 *RAM = memory_region(space->machine, "pcm1");
	int bank = data & 1;
	if (bank & ~1)	logerror("CPU#2 PC %06X - ROM bank unknown bits: %02X\n", cpu_get_pc(space->cpu), data);
	memory_set_bankptr(space->machine, "bank1", &RAM[bank * 0x10000 + 0x400]);
}

/* Memory maps: Yes, *no* RAM */

static ADDRESS_MAP_START( uballoon_pcm_1_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x03ff) AM_ROM	// ROM
	AM_RANGE(0x0400, 0xffff) AM_ROMBANK("bank1")	// Banked ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( uballoon_pcm_1_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ(soundlatch2_r)	// From The Sound Z80
	AM_RANGE(0x00, 0x00) AM_DEVWRITE("dac1", bssoccer_DAC_w)	// 2 x DAC
	AM_RANGE(0x01, 0x01) AM_DEVWRITE("dac2", bssoccer_DAC_w)	// 2 x DAC
	AM_RANGE(0x03, 0x03) AM_WRITE(uballoon_pcm_1_bankswitch_w)	// Rom Bank
ADDRESS_MAP_END

static MACHINE_RESET(uballoon)
{
	const address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);
	uballoon_pcm_1_bankswitch_w(space, 0, 0);
}


/***************************************************************************
                            Best Of Best
***************************************************************************/

static ADDRESS_MAP_START( bestbest_pcm_1_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( bestbest_pcm_1_iomap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ (soundlatch2_r 	)	// From The Sound Z80
	AM_RANGE(0x00, 0x00) AM_MIRROR(0x02) AM_DEVWRITE("dac1", bssoccer_DAC_w)	// 2 x DAC
	AM_RANGE(0x01, 0x01) AM_MIRROR(0x02) AM_DEVWRITE("dac2", bssoccer_DAC_w)	// 2 x DAC
ADDRESS_MAP_END

/***************************************************************************


                                Input Ports


***************************************************************************/

#define JOY(_n_) \
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(_n_)    \
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(_n_)  \
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(_n_)  \
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(_n_)        \
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(_n_)        \
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(_n_)        \
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START##_n_ )


/***************************************************************************
                            Back Street Soccer
***************************************************************************/

static INPUT_PORTS_START( bssoccer )
	PORT_START("P1")	/* $a00001.b */
	JOY(1)

	PORT_START("P2")	/* $a00003.b */
	JOY(2)

	PORT_START("P3")	/* $a00005.b */
	JOY(3)

	PORT_START("P4")	/* $a00007.b */
	JOY(4)

	PORT_START("DSW1")	/* $a00008.w */
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coinage ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(	  0x0001, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(	  0x0002, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(	  0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(	  0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(	  0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(	  0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(	  0x0003, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0018, 0x0018, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(	  0x0010, DEF_STR( Easy ) )
	PORT_DIPSETTING(	  0x0018, DEF_STR( Normal ) )
	PORT_DIPSETTING(	  0x0008, DEF_STR( Hard ) )
	PORT_DIPSETTING(	  0x0000, "Hardest?"  )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(	  0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_SERVICE( 0x0080, IP_ACTIVE_LOW )

	PORT_DIPNAME( 0x0300, 0x0300, "Play Time P1" )
	PORT_DIPSETTING(	  0x0300, "1:30" )
	PORT_DIPSETTING(	  0x0200, "1:45" )
	PORT_DIPSETTING(	  0x0100, "2:00" )
	PORT_DIPSETTING(	  0x0000, "2:15" )
	PORT_DIPNAME( 0x0c00, 0x0c00, "Play Time P2" )
	PORT_DIPSETTING(	  0x0c00, "1:30" )
	PORT_DIPSETTING(	  0x0800, "1:45" )
	PORT_DIPSETTING(	  0x0400, "2:00" )
	PORT_DIPSETTING(	  0x0000, "2:15" )
	PORT_DIPNAME( 0x3000, 0x3000, "Play Time P3" )
	PORT_DIPSETTING(	  0x3000, "1:30" )
	PORT_DIPSETTING(	  0x2000, "1:45" )
	PORT_DIPSETTING(	  0x1000, "2:00" )
	PORT_DIPSETTING(	  0x0000, "2:15" )
	PORT_DIPNAME( 0xc000, 0xc000, "Play Time P4" )
	PORT_DIPSETTING(	  0xc000, "1:30" )
	PORT_DIPSETTING(	  0x8000, "1:45" )
	PORT_DIPSETTING(	  0x4000, "2:00" )
	PORT_DIPSETTING(	  0x0000, "2:15" )

	PORT_START("DSW2")	/* $a0000b.b */
	PORT_DIPNAME( 0x0001, 0x0001, "Copyright" )         // these 4 are shown in test mode
	PORT_DIPSETTING(	  0x0001, "Distributer Unico" )
	PORT_DIPSETTING(	  0x0000, "All Rights Reserved" )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )	// used!
	PORT_DIPSETTING(	  0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	  0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	  0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN4 )
INPUT_PORTS_END


/***************************************************************************
                                Ultra Balloon
***************************************************************************/

static INPUT_PORTS_START( uballoon )
	PORT_START("P1")	/* $600000.w */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("P2")	/* $600002.w */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x3000, 0x3000, "Copyright" )	// Jumpers
	PORT_DIPSETTING(	  0x3000, "Distributer Unico" )
	PORT_DIPSETTING(	  0x2000, "All Rights Reserved" )
//  PORT_DIPSETTING(      0x1000, "Distributer Unico" )
//  PORT_DIPSETTING(      0x0000, "All Rights Reserved" )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("DSW1")	/* $600005.b */
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coinage ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(	  0x0001, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(	  0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(	  0x0003, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(	  0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(	  0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(	  0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(	  0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0018, 0x0018, DEF_STR( Lives ) )
	PORT_DIPSETTING(	  0x0010, "2" )
	PORT_DIPSETTING(	  0x0018, "3" )
	PORT_DIPSETTING(	  0x0008, "4" )
	PORT_DIPSETTING(	  0x0000, "5" )
	PORT_DIPNAME( 0x0060, 0x0060, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(	  0x0040, DEF_STR( Easy ) )
	PORT_DIPSETTING(	  0x0060, DEF_STR( Normal )  )
	PORT_DIPSETTING(	  0x0020, DEF_STR( Hard ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( Hardest ) )
	PORT_SERVICE( 0x0080, IP_ACTIVE_LOW )

	PORT_START("DSW2")	/* $600007.b */
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(	  0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(	  0x0002, DEF_STR( Upright ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x001c, 0x001c, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(	  0x001c, "200K" )
	PORT_DIPSETTING(	  0x0010, "300K, 1000K" )
	PORT_DIPSETTING(	  0x0018, "400K" )
	PORT_DIPSETTING(	  0x000c, "500K, 1500K" )
	PORT_DIPSETTING(	  0x0008, "500K, 2000K" )
	PORT_DIPSETTING(	  0x0004, "500K, 3000K" )
	PORT_DIPSETTING(	  0x0014, "600K" )
	PORT_DIPSETTING(	  0x0000, DEF_STR( None ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Unknown 1-5*" )
	PORT_DIPSETTING(	  0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Unknown 1-6*" )
	PORT_DIPSETTING(	  0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(	  0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
INPUT_PORTS_END

/***************************************************************************
                            Suna Quiz 6000 Academy
***************************************************************************/

static INPUT_PORTS_START( sunaq )
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN1 )


	PORT_START("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coinage ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(	  0x0001, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(	  0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(	  0x0003, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(	  0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(	  0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(	  0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(	  0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0018, 0x0008, DEF_STR( Difficulty ) )	/* Should this be Difficulty or Lives ?? */
	PORT_DIPSETTING(	  0x0000, DEF_STR( Easy ) )	/* 5 Hearts */
	PORT_DIPSETTING(	  0x0008, DEF_STR( Normal ) )	/* 5 Hearts */
	PORT_DIPSETTING(	  0x0010, DEF_STR( Hard ) )	/* 4 Hearts */
	PORT_DIPSETTING(	  0x0018, DEF_STR( Hardest ) )	/* 3 Hearts */
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(	  0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(	  0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
    PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW2") /* Unused? */
INPUT_PORTS_END

/***************************************************************************
                            Best Of Best
***************************************************************************/

static INPUT_PORTS_START( bestbest )
	PORT_START("P1")	/* 500000.w */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT	) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("P2")	/* 500002.w */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT	) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("DSW")	/* 500004.w */
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coinage ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(	  0x0001, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(	  0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(	  0x0003, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(	  0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(	  0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(	  0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(	  0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0018, 0x0010, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(	  0x0018, DEF_STR( Easy ) )
	PORT_DIPSETTING(	  0x0010, DEF_STR( Normal ) )
	PORT_DIPSETTING(	  0x0008, DEF_STR( Hard ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Display Combos" )
	PORT_DIPSETTING(	  0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0020, DEF_STR( On ) )
	PORT_SERVICE( 0x0040, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(	  0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(	  0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0600, 0x0400, "Play Time" )
	PORT_DIPSETTING(	  0x0600, "1:10" )
	PORT_DIPSETTING(	  0x0400, "1:20" )
	PORT_DIPSETTING(	  0x0200, "1:30" )
	PORT_DIPSETTING(	  0x0000, "1:40" )
	PORT_DIPUNUSED( 0x0800, 0x0800 )
	PORT_DIPUNUSED( 0x1000, 0x1000 )
	PORT_DIPUNUSED( 0x2000, 0x2000 )
	PORT_DIPUNUSED( 0x4000, 0x4000 )
	PORT_DIPUNUSED( 0x8000, 0x8000 )
INPUT_PORTS_END

/***************************************************************************


                                Graphics Layouts


***************************************************************************/

/* Tiles are 8x8x4 but the minimum sprite size is 2x2 tiles */

static const gfx_layout layout_8x8x4 =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+0,RGN_FRAC(1,2)+4,	0,4 },
	{ 3,2,1,0, 11,10,9,8 },
	{ STEP8(0,16) },
	8*8*4/2
};

static GFXDECODE_START( suna16 )
	GFXDECODE_ENTRY( "gfx1", 0, layout_8x8x4, 0, 16*2 ) // [0] Sprites
GFXDECODE_END

// Two sprites chips
static GFXDECODE_START( bestbest )
	GFXDECODE_ENTRY( "gfx1", 0, layout_8x8x4, 0, 256*8/16 ) // [0] Sprites (Chip 1)
	GFXDECODE_ENTRY( "gfx2", 0, layout_8x8x4, 0, 256*8/16 ) // [1] Sprites (Chip 2)
GFXDECODE_END



/***************************************************************************


                                Machine drivers


***************************************************************************/


/***************************************************************************
                            Back Street Soccer
***************************************************************************/

static INTERRUPT_GEN( bssoccer_interrupt )
{
	switch (cpu_getiloops(device))
	{
		case 0: 	cpu_set_input_line(device, 1, HOLD_LINE);	break;
		case 1: 	cpu_set_input_line(device, 2, HOLD_LINE);	break;
	}
}

static MACHINE_DRIVER_START( bssoccer )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 8000000)	/* ? */
	MDRV_CPU_PROGRAM_MAP(bssoccer_map)
	MDRV_CPU_VBLANK_INT_HACK(bssoccer_interrupt,2)

	MDRV_CPU_ADD("audiocpu", Z80, 3579545)		/* Z80B */
	MDRV_CPU_PROGRAM_MAP(bssoccer_sound_map)

	MDRV_CPU_ADD("pcm1", Z80, 5000000)		/* Z80B */
	MDRV_CPU_PROGRAM_MAP(bssoccer_pcm_1_map)
	MDRV_CPU_IO_MAP(bssoccer_pcm_1_io_map)

	MDRV_CPU_ADD("pcm2", Z80, 5000000)		/* Z80B */
	MDRV_CPU_PROGRAM_MAP(bssoccer_pcm_2_map)
	MDRV_CPU_IO_MAP(bssoccer_pcm_2_io_map)

	MDRV_QUANTUM_TIME(HZ(6000))

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 256-1, 0+16, 256-16-1)

	MDRV_GFXDECODE(suna16)
	MDRV_PALETTE_LENGTH(512)

	MDRV_VIDEO_START(suna16)
	MDRV_VIDEO_UPDATE(suna16)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2151, 3579545)
	MDRV_SOUND_ROUTE(0, "lspeaker", 0.20)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.20)

	MDRV_SOUND_ADD("dac1", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.40)

	MDRV_SOUND_ADD("dac2", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.40)

	MDRV_SOUND_ADD("dac3", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.40)

	MDRV_SOUND_ADD("dac4", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.40)
MACHINE_DRIVER_END



/***************************************************************************
                                Ultra Balloon
***************************************************************************/

static MACHINE_DRIVER_START( uballoon )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 8000000)
	MDRV_CPU_PROGRAM_MAP(uballoon_map)
	MDRV_CPU_VBLANK_INT("screen", irq1_line_hold)

	MDRV_CPU_ADD("audiocpu", Z80, 3579545)	/* ? */
	MDRV_CPU_PROGRAM_MAP(uballoon_sound_map)

	MDRV_CPU_ADD("pcm1", Z80, 5000000)	/* ? */
	MDRV_CPU_PROGRAM_MAP(uballoon_pcm_1_map)
	MDRV_CPU_IO_MAP(uballoon_pcm_1_io_map)

	/* 2nd PCM Z80 missing */

	MDRV_QUANTUM_TIME(HZ(6000))

	MDRV_MACHINE_RESET(uballoon)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 256-1, 0+16, 256-16-1)

	MDRV_GFXDECODE(suna16)
	MDRV_PALETTE_LENGTH(512)

	MDRV_VIDEO_START(suna16)
	MDRV_VIDEO_UPDATE(suna16)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2151, 3579545)
	MDRV_SOUND_ROUTE(0, "lspeaker", 0.50)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.50)

	MDRV_SOUND_ADD("dac1", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)

	MDRV_SOUND_ADD("dac2", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)
MACHINE_DRIVER_END

/***************************************************************************
                            Suna Quiz 6000 Academy
***************************************************************************/

static MACHINE_DRIVER_START( sunaq )
	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 24000000/4)
	MDRV_CPU_PROGRAM_MAP(sunaq_map)
	MDRV_CPU_VBLANK_INT("screen", irq1_line_hold)

	MDRV_CPU_ADD("audiocpu", Z80, 14318000/4)
	MDRV_CPU_PROGRAM_MAP(sunaq_sound_map)

	MDRV_CPU_ADD("pcm1", Z80, 24000000/4)		/* Z80B */
	MDRV_CPU_PROGRAM_MAP(bssoccer_pcm_1_map)
	MDRV_CPU_IO_MAP(bssoccer_pcm_1_io_map)

	/* 2nd PCM Z80 missing */

	MDRV_QUANTUM_TIME(HZ(6000))

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 256-1, 0+16, 256-16-1)

	MDRV_GFXDECODE(suna16)
	MDRV_PALETTE_LENGTH(512)

	MDRV_VIDEO_START(suna16)
	MDRV_VIDEO_UPDATE(suna16)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2151, 14318000/4)
	MDRV_SOUND_ROUTE(0, "lspeaker", 0.50)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.50)

	MDRV_SOUND_ADD("dac1", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)

	MDRV_SOUND_ADD("dac2", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)
MACHINE_DRIVER_END

/***************************************************************************
                            Best Of Best
***************************************************************************/

static void bestbest_ym3526_irqhandler(running_device *device, int state)
{
	cputag_set_input_line(device->machine, "audiocpu", INPUT_LINE_IRQ0, state);
}

static const ym3526_interface bestbest_ym3526_interface =
{
	bestbest_ym3526_irqhandler
};

static WRITE8_DEVICE_HANDLER( bestbest_ay8910_port_a_w )
{
	// ?
}

static const ay8910_interface bestbest_ay8910_interface =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_NULL,									DEVCB_NULL,
	DEVCB_HANDLER(bestbest_ay8910_port_a_w),	DEVCB_NULL
};

static MACHINE_DRIVER_START( bestbest )
	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 24000000/4)
	MDRV_CPU_PROGRAM_MAP(bestbest_map)
	MDRV_CPU_VBLANK_INT_HACK(bssoccer_interrupt,2)

	MDRV_CPU_ADD("audiocpu", Z80, 24000000/4)
	MDRV_CPU_PROGRAM_MAP(bestbest_sound_map)

	MDRV_CPU_ADD("pcm1", Z80, 24000000/4)
	MDRV_CPU_PROGRAM_MAP(bestbest_pcm_1_map)
	MDRV_CPU_IO_MAP(bestbest_pcm_1_iomap)

	/* 2nd PCM Z80 missing */

	MDRV_QUANTUM_TIME(HZ(6000))

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(59.1734)    // measured on pcb (15.6218kHz HSync)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 256-1, 0+16, 256-16-1)

	MDRV_GFXDECODE(bestbest)
	MDRV_PALETTE_LENGTH(256*8)

	MDRV_VIDEO_START(suna16)
	MDRV_VIDEO_UPDATE(bestbest)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("aysnd", AY8910, 24000000/16)
	MDRV_SOUND_CONFIG(bestbest_ay8910_interface)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)

	MDRV_SOUND_ADD("ymsnd", YM3526, 24000000/8)
	MDRV_SOUND_CONFIG(bestbest_ym3526_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)

	MDRV_SOUND_ADD("dac1", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.40)

	MDRV_SOUND_ADD("dac2", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.40)

	MDRV_SOUND_ADD("dac3", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.40)

	MDRV_SOUND_ADD("dac4", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.40)
MACHINE_DRIVER_END

/***************************************************************************


                                ROMs Loading


***************************************************************************/


/***************************************************************************

                            [ Back Street Soccer ]

  68000-10  32MHz
            14.318MHz
  01   02                    12
  03   04                   Z80B
  6264 6264       YM2151
                  6116
                   11      13
  62256           Z80B    Z80B
  62256
  62256   05 06                  SW2
          07 08                  SW1
          09 10          6116-45
                                     6116-45
                         6116-45     6116-45

***************************************************************************/

ROM_START( bssoccer )

	ROM_REGION( 0x200000, "maincpu", 0 )	/* 68000 Code */
	ROM_LOAD16_BYTE( "02", 0x000000, 0x080000, CRC(32871005) SHA1(b094ee3f4fc24c0521915d565f6e203d51e51f6d) )
	ROM_LOAD16_BYTE( "01", 0x000001, 0x080000, CRC(ace00db6) SHA1(6bd146f9b44c97be77578b4f0ffa28cbf66283c2) )
	ROM_LOAD16_BYTE( "04", 0x100000, 0x080000, CRC(25ee404d) SHA1(1ab7cb1b4836caa05be73ea441deed80f1e1ba81) )
	ROM_LOAD16_BYTE( "03", 0x100001, 0x080000, CRC(1a131014) SHA1(4d21264da3ee9b9912d1205999a555657ba33bd7) )

	ROM_REGION( 0x010000, "audiocpu", 0 )	/* Z80 #1 - Music */
	ROM_LOAD( "11", 0x000000, 0x010000, CRC(df7ae9bc) SHA1(86660e723b0712c131dc57645b6a659d5100e962) ) // 1xxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x080000, "pcm1", 0 )	/* Z80 #2 - PCM */
	ROM_LOAD( "13", 0x000000, 0x080000, CRC(2b273dca) SHA1(86e1bac9d1e39457c565390b9053986453db95ab) )

	ROM_REGION( 0x080000, "pcm2", 0 )	/* Z80 #3 - PCM */
	ROM_LOAD( "12", 0x000000, 0x080000, CRC(6b73b87b) SHA1(52c7dc7da6c21eb7e0dad13deadb1faa94a87bb3) )

	ROM_REGION( 0x300000, "gfx1", ROMREGION_INVERT )	/* Sprites */
	ROM_LOAD( "05", 0x000000, 0x080000, CRC(a5245bd4) SHA1(d46a8db437e49158c020661536eb0be8a6e2e8b0) )
	ROM_LOAD( "07", 0x080000, 0x080000, CRC(fdb765c2) SHA1(f9852fd3734d10e18c91cd572ca62e66d74ccb72) )
	ROM_LOAD( "09", 0x100000, 0x080000, CRC(0e82277f) SHA1(4bdfd0ff310bf8326806a83767a6c98905debbd0) )
	ROM_LOAD( "06", 0x180000, 0x080000, CRC(d42ce84b) SHA1(3a3d07d571793ecf4c936d3af244c63b9e4b4bb9) )
	ROM_LOAD( "08", 0x200000, 0x080000, CRC(96cd2136) SHA1(1241859d6c5e64de73898763f0358171ea4aeae3) )
	ROM_LOAD( "10", 0x280000, 0x080000, CRC(1ca94d21) SHA1(23d892b840e37064a175584f955f25f990d9179d) )

ROM_END



/***************************************************************************

                            [ Ultra Ballon ]

the gameplay on this game a like bubble bobble in many ways,it uses a
68k@8MHz as the main cpu,2 z80's and a ym2151,the names of the rom files
are just my guess.

prg1.rom      27c040
prg2.rom      27c040
gfx1.rom      27c040
gfx2.rom      27c040
gfx3.rom      27c040
gfx4.rom      27c040
audio1.rom    27c512
audio2.rom    27c010

***************************************************************************/

ROM_START( uballoon )

	ROM_REGION( 0x100000, "maincpu", 0 )	/* 68000 Code */
	ROM_LOAD16_BYTE( "prg2.rom", 0x000000, 0x080000, CRC(72ab80ea) SHA1(b755940877cf286559208106dd5e6933aeb72242) )
	ROM_LOAD16_BYTE( "prg1.rom", 0x000001, 0x080000, CRC(27a04f55) SHA1(a530294b000654db8d84efe4835b72e0dca62819) )

	ROM_REGION( 0x010000, "audiocpu", 0 )	/* Z80 #1 - Music */
	ROM_LOAD( "audio1.rom", 0x000000, 0x010000, CRC(c771f2b4) SHA1(6da4c526c0ea3be5d5bb055a31bf1171a6ddb51d) )

	ROM_REGION( 0x020000, "pcm1", 0 )	/* Z80 #2 - PCM */
	ROM_LOAD( "audio2.rom", 0x000000, 0x020000, CRC(c7f75347) SHA1(5bbbd39285c593441c6da6a12f3632d60b103216) )

	/* There's no Z80 #3 - PCM */

	ROM_REGION( 0x200000, "gfx1", ROMREGION_INVERT )	/* Sprites */
	ROM_LOAD( "gfx1.rom", 0x000000, 0x080000, CRC(fd2ec297) SHA1(885834d9b58ccfd9a32ecaa51c45e70fbbe935db) )
	ROM_LOAD( "gfx2.rom", 0x080000, 0x080000, CRC(6307aa60) SHA1(00406eba98ec368e72ee53c08b9111dec4f2552f) )
	ROM_LOAD( "gfx3.rom", 0x100000, 0x080000, CRC(718f3150) SHA1(5971f006203f86743ebc825e4ab1ed1f811e3165) )
	ROM_LOAD( "gfx4.rom", 0x180000, 0x080000, CRC(af7e057e) SHA1(67a03b54ffa1483c8ed044f27287b7f3f1150455) )

ROM_END


static DRIVER_INIT( uballoon )
{
	UINT16 *RAM = (UINT16 *) memory_region(machine, "maincpu");

	// Patch out the protection checks
	RAM[0x0113c/2] = 0x4e71;	// bne $646
	RAM[0x0113e/2] = 0x4e71;	// ""
	RAM[0x01784/2] = 0x600c;	// beq $1792
	RAM[0x018e2/2] = 0x600c;	// beq $18f0
	RAM[0x03c54/2] = 0x600C;	// beq $3c62
	RAM[0x126a0/2] = 0x4e71;	// bne $1267a (ROM test)
}

/***************************************************************************
                            Suna Quiz 6000 Academy

  KRB-0027A mainboard

  68000 6 MHz
  Z80B x 2
  Actel A1020B
  OSC: 24.000 MHz, 14.318 MHz
  YM2151, YM3012
  RAM:
  62256 x 3
  6264 x 2
  6116 x 5
  Single 8 switch DSW

***************************************************************************/

ROM_START( sunaq )
	ROM_REGION( 0x100000, "maincpu", 0 )	/* 68000 Code */
	ROM_LOAD16_BYTE( "prog2.bin", 0x000000, 0x080000, CRC(a92bce45) SHA1(258b2a21c27effa1d3380e4c08558542b1d05175) )
	ROM_LOAD16_BYTE( "prog1.bin", 0x000001, 0x080000, CRC(ff690e7e) SHA1(43b9c67f8d8d791be922966632613a077807b755) )

	ROM_REGION( 0x010000, "audiocpu", 0 )	/* Z80 #1 - Music */
	ROM_LOAD( "audio1.bin", 0x000000, 0x010000, CRC(3df42f82) SHA1(91c1037c9d5d1ec82ed4cdfb35de5a6d626ecb3b) )

	ROM_REGION( 0x080000, "pcm1", 0 )	/* Z80 #2 - PCM */
	ROM_LOAD( "audio2.bin", 0x000000, 0x080000, CRC(cac85ba9) SHA1(e5fbe813022c17d9eaf2a57184341666e2af365a) )

	/* There's no Z80 #3 - PCM */

	ROM_REGION( 0x200000, "gfx1", ROMREGION_INVERT )	/* Sprites */
	ROM_LOAD( "gfx1.bin", 0x000000, 0x080000, CRC(0bde5acf) SHA1(a9befb5f9a663bf48537471313f606853ea1f274) )
	ROM_LOAD( "gfx2.bin", 0x100000, 0x080000, CRC(24b74826) SHA1(cb3f665d1b1f5c9d385a3a3193866c9cae6c7002) )
ROM_END


/***************************************************************************

Best Of Best
Suna, 1994

PCB Layout
----------

KRB-0026
SUNA ELECTRONICS IND CO., LTD.
|------------------------------------------------------------|
|VOL     AY3-8910   Z80B     5.BIN                           |
|UPD1242H    6.BIN  Z80B     6116            68000           |
| LM324              24MHz   YM3526                          |
| LM324                                                      |
|    YM3014                82S129.5     4.BIN       2.BIN    |
|                  82S129.6|-------|                         |
|      62256               |UNKNOWN|    3.BIN       1.BIN    |
|                          |PLCC84 |                         |
|      62256               |       |    62256       62256    |
|                          |-------|                         |
|J                                                           |
|A                         |-------|                         |
|M     DSW2(8)             |ACTEL  |                         |
|M                         |A1020B |                         |
|A                         |PLCC84 |                         |
|      DSW1(8) 62256       |-------|           62256         |
|                                                            |
|        6116  62256                           62256         |
|        6116                                                |
|        6116  62256                           62256         |
|        6116                |-------------------------------|
|        6116                | 18.BIN  17.BIN          10.BIN|
|        6116                |         16.BIN  13.BIN   9.BIN|
|CN1     6116                |         15.BIN  12.BIN   8.BIN|
|        6116                |         14.BIN  11.BIN   7.BIN|
|----------------------------|-------------------------------|
Notes:
      68000    - clock 6.000MHz [24/4]
      Z80B     - clock 6.000MHz [24/4] for both
      YM3526   - clock 3.000MHz [24/8]
      AY3-8910 - clock 1.500MHz [24/16]
      6116     - 2kx8 SRAM
      62256    - 32kx8 SRAM
      CN1      - Connector for extra controls
      ROMs 7-18 located on a plug-in daughterboard
      Both PROMs are identical

      Measurements
      ------------
      XTAL  - 23.99463MHz
      VSync - 59.1734Hz
      HSync - 15.6218kHz

***************************************************************************/

ROM_START( bestbest )
	ROM_REGION( 0x40000, "maincpu", 0 ) 	/* 68000 Code */
	// V13.0 1993,3,25-11,29 KIM.H.T M=1:KDS=9
	ROM_LOAD16_BYTE( "4.bin", 0x00000, 0x20000, CRC(06741994) SHA1(e872e9e9d02360dda9c9b6df8e6424b0f3e18c1f) )	// 1xxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "2.bin", 0x00001, 0x20000, CRC(42843dec) SHA1(3705661a9740b3499297424e340da9a3606873fb) )	// 1xxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION16_BE( 0x100000, "user1", 0 ) 	/* 68000 Data */
	ROM_LOAD16_BYTE( "3.bin", 0x00000, 0x80000, CRC(e2bb8f26) SHA1(d73bbe034718c77774dede61e751a9ae2d29118a) )
	ROM_LOAD16_BYTE( "1.bin", 0x00001, 0x80000, CRC(d365e20a) SHA1(29706d6e422e71c7dad51a3369683a6539f72b54) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* Z80 #1 - Music */
	ROM_LOAD( "5.bin", 0x00000, 0x10000, CRC(bb9265e6) SHA1(424eceac4fd48c9a99653ece2f3fcbc8b37569cf) ) // BEST OF BEST V10 XILINX PROGRAM 3020 1994,1,17

	ROM_REGION( 0x10000, "pcm1", 0 )	/* Z80 #2 - PCM */
	ROM_LOAD( "6.bin", 0x00000, 0x10000, CRC(dd445f6b) SHA1(658417d72c003f25db273e3c731838317ed1876c) )

	/* There's no Z80 #3 - PCM */

	ROM_REGION( 0x200000, "gfx1", ROMREGION_INVERT )	/* Sprites (Chip 1) */
	ROM_LOAD( "9.bin",  0x000000, 0x80000, CRC(b11994ea) SHA1(4ff2250a9dbb2e575982e2ffcad7686347368b5b) )
	ROM_LOAD( "10.bin", 0x080000, 0x80000, CRC(37b41ef5) SHA1(dd4500663537ffad369ee9415c56df90221bed23) )
	ROM_LOAD( "7.bin",  0x100000, 0x80000, CRC(16188b73) SHA1(1e67f9b100614466e2ff1169f25c90e34a2e7db9) )
	ROM_LOAD( "8.bin",  0x180000, 0x80000, CRC(765ce06b) SHA1(6cc6d7c27b49eedd58104c50e4887f86bff9357c) )

	ROM_REGION( 0x400000, "gfx2", ROMREGION_INVERT )	/* Sprites (Chip 2) */
	ROM_LOAD( "16.bin", 0x000000, 0x80000, CRC(dc46cdea) SHA1(d601f5464894223ce8459093ae53006155a3e680) )
	ROM_LOAD( "17.bin", 0x080000, 0x80000, CRC(c6fadd57) SHA1(ce9bc4d7a288feebdd19de09d00bec8489346878) )
	ROM_LOAD( "13.bin", 0x100000, 0x80000, CRC(23283ac4) SHA1(f7aa00f203b17b590f1c43990f3f1c4aba7ba0ad) )
	ROM_LOAD( "18.bin", 0x180000, 0x80000, CRC(674c4609) SHA1(f1de78c01d26dfb1174203415ccf3c771398d163) )

	ROM_LOAD( "14.bin", 0x200000, 0x80000, CRC(c210fb53) SHA1(3d5a763bffaef922a77c95131b1e41f0f90629a5) )
	ROM_LOAD( "15.bin", 0x280000, 0x80000, CRC(3b1166c7) SHA1(7f2a0c9131fcf39dd67047b6e697c4076ca37b19) )
	ROM_LOAD( "11.bin", 0x300000, 0x80000, CRC(323eebc3) SHA1(0e82b583273c9ba5252f7a108538ae58edf39a03) )
	ROM_LOAD( "12.bin", 0x380000, 0x80000, CRC(ca7c8176) SHA1(1ec99db3e0840b4647d6ccdf6fda118fa9ad4f42) )

	ROM_REGION( 0x200, "proms", 0 )	// ?
	ROM_LOAD( "82s129.5", 0x000, 0x100, CRC(10bfcebb) SHA1(ae8708db7d3a8984f16e876867ecdbb4445e3378) )	// FIXED BITS (0000xx0x0000xxxx)
	ROM_LOAD( "82s129.6", 0x100, 0x100, CRC(10bfcebb) SHA1(ae8708db7d3a8984f16e876867ecdbb4445e3378) )	// identical to 82s129.5
ROM_END


/***************************************************************************


                                Games Drivers


***************************************************************************/

GAME( 1994, bestbest, 0, bestbest, bestbest, 0,        ROT0, "SunA", "Best Of Best", 0 )
GAME( 1994, sunaq,    0, sunaq,    sunaq,    0,        ROT0, "SunA", "SunA Quiz 6000 Academy (940620-6)", 0 )	// Date/Version on-screen is 940620-6, but in the program rom it's 1994,6,30  K.H.T  V6.00
GAME( 1996, bssoccer, 0, bssoccer, bssoccer, 0,        ROT0, "SunA", "Back Street Soccer", 0 )
GAME( 1996, uballoon, 0, uballoon, uballoon, uballoon, ROT0, "SunA", "Ultra Balloon", 0 )
