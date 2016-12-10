/***************************************************************************

    Atari Fire Fox hardware

    driver by smf, Aaron Giles, Chris Hardy & Scott Waye

short term:
    split driver/vidhrdw/sndhrdw/machine
    add option to centre joystick to enter test menu

it uses a quad pokey package 137323-1221-406???
the laser disc is a philips lvp 22vp931
( but maybe this works too... Special Drive: Laser Disc Player - Philips VP-832A )


AV# 60626
Atari "Firefox" V

Laser Disc - 30 minutes - Color - 1983

An interactive CAV laserdisc designed for use in the Atari video arcade game machine.
Contains over 100 visual and sound segments that include all of the branching possibilities of this game.
Each segment is two to five seconds long. This disc will play on any player,
but requires a special level III player for proper control. Video: CAV. Audio: Analog.

*/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "cpu/m6502/m6502.h"
#include "sound/pokey.h"
#include "sound/tms5220.h"
#include "machine/laserdsc.h"
#include "machine/6532riot.h"
#include "machine/x2212.h"


#define MASTER_XTAL		XTAL_14_31818MHz


/*
fff6=firq e4a2 when dav goes active/low
fff8=irq e38f  This is through a flip-flop so goes off (high as active low) only when reset_irq is active - low.
fffa=??? e38d
fffc=??? e38d
fffe=reset e7cc
*/

/* 0x50-52 Used as a copy of the status
   0x59 = 6-length of laser disc return code
   0x53 = pointer to laser disc return
   ( LaserDiscBits & 0x80 ) != 0 when return code available
   DSKREAD = acknowledge
   ReadDiscData = return code
*/

/* FXXXXX for first field
   AXXXXX for second field */

static running_device *laserdisc;
static int m_n_disc_lock;
static int m_n_disc_data;
static int m_n_disc_read_data;

/* 20 = DISKOPR - Active low
   40 = DISKFULL - Active low
   80 = DISKDAV - Active low data available
   */
static READ8_HANDLER( firefox_disc_status_r )
{
	UINT8 result = 0xff;

	result ^= 0x20;
	if (!laserdisc_line_r(laserdisc, LASERDISC_LINE_READY))
		result ^= 0x40;
	if (laserdisc_line_r(laserdisc, LASERDISC_LINE_DATA_AVAIL))
		result ^= 0x80;

	return result;
}

/* 4105 - DREAD */
/* this reset RDDSK (&DSKRD) */
static READ8_HANDLER( firefox_disc_data_r )
{
	return m_n_disc_read_data;
}

/* DISK READ ENABLE */
/* 4218 - DSKREAD, set RDDSK */
static WRITE8_HANDLER( firefox_disc_read_w )
{
	m_n_disc_read_data = laserdisc_data_r(laserdisc);
}

static WRITE8_HANDLER( firefox_disc_lock_w )
{
	m_n_disc_lock = data & 0x80;
}

static WRITE8_HANDLER( audio_enable_w )
{
	sound_set_output_gain(space->machine->device("ldsound"), ~offset & 1, (data & 0x80) ? 1.0 : 0.0);
}

static WRITE8_HANDLER( firefox_disc_reset_w )
{
	laserdisc_line_w(laserdisc, LASERDISC_LINE_RESET, (data & 0x80) ? CLEAR_LINE : ASSERT_LINE);
}

/* active low on dbb7 */
static WRITE8_HANDLER( firefox_disc_write_w )
{
	if ( ( data & 0x80 ) == 0 )
		laserdisc_data_w(laserdisc, m_n_disc_data);
}

/* latch the data */
static WRITE8_HANDLER( firefox_disc_data_w )
{
	m_n_disc_data = data;
}


static unsigned char *tileram;
static unsigned char *tile_palette;
static unsigned char *sprite_palette;
static running_device *nvram_1c;
static running_device *nvram_1d;
static tilemap_t *bgtiles;

static int control_num;
static UINT8 sound_to_main_flag;
static UINT8 main_to_sound_flag;
static int sprite_bank;

/*************************************
 *
 *  Video
 *
 *************************************/

static TILE_GET_INFO( bgtile_get_info )
{
	SET_TILE_INFO(0, tileram[tile_index], 0, 0);
}


static WRITE8_HANDLER( tileram_w )
{
	tileram[offset] = data;
	tilemap_mark_tile_dirty(bgtiles, offset);
}


static VIDEO_START( firefox )
{
	bgtiles = tilemap_create(machine, bgtile_get_info, tilemap_scan_rows, 8,8, 64,64);
	tilemap_set_transparent_pen(bgtiles, 0);
	tilemap_set_scrolldy(bgtiles, machine->primary_screen->visible_area().min_y, 0);
}


static VIDEO_UPDATE( firefox )
{
	int sprite;
	int gfxtop = screen->visible_area().min_y;

	bitmap_fill( bitmap, cliprect, palette_get_color(screen->machine, 256) );

	for( sprite = 0; sprite < 32; sprite++ )
	{
		UINT8 *sprite_data = screen->machine->generic.spriteram.u8 + ( 0x200 * sprite_bank ) + ( sprite * 16 );
		int flags = sprite_data[ 0 ];
		int y = sprite_data[ 1 ] + ( 256 * ( ( flags >> 0 ) & 1 ) );
		int x = sprite_data[ 2 ] + ( 256 * ( ( flags >> 1 ) & 1 ) );

		if( x != 0 )
		{
			int row;

			for( row = 0; row < 8; row++ )
			{
				int color = ( flags >> 2 ) & 0x03;
				int flipy = flags & 0x10;
				int flipx = flags & 0x20;
				int code = sprite_data[ 15 - row ] + ( 256 * ( ( flags >> 6 ) & 3 ) );

				drawgfx_transpen( bitmap, cliprect, screen->machine->gfx[ 1 ], code, color, flipx, flipy, x + 8, gfxtop + 500 - y - ( row * 16 ), 0 );
			}
		}
	}

	tilemap_draw( bitmap, cliprect, bgtiles, 0, 0 );

	return 0;
}

static TIMER_DEVICE_CALLBACK( video_timer_callback )
{
	timer.machine->primary_screen->update_now();

	cputag_set_input_line( timer.machine, "maincpu", M6809_IRQ_LINE, ASSERT_LINE );
}

static void set_rgba( running_machine *machine, int start, int index, unsigned char *palette_ram )
{
	int r = palette_ram[ index ];
	int g = palette_ram[ index + 256 ];
	int b = palette_ram[ index + 512 ];
	int a = ( b & 3 ) * 0x55;

	palette_set_color( machine, start + index, MAKE_ARGB( a, r, g, b ) );
}

static WRITE8_HANDLER( tile_palette_w )
{
	tile_palette[ offset ] = data;
	set_rgba( space->machine, 0, offset & 0xff, tile_palette );
}

static WRITE8_HANDLER( sprite_palette_w )
{
	sprite_palette[ offset ] = data;
	set_rgba( space->machine, 256, offset & 0xff, sprite_palette );
}

static WRITE8_HANDLER( firefox_objram_bank_w )
{
	sprite_bank = data & 0x03;
}



/*************************************
 *
 *  Main <-> sound communication
 *
 *************************************/

static CUSTOM_INPUT( mainflag_r )
{
	return main_to_sound_flag;
}

static CUSTOM_INPUT( soundflag_r )
{
	return sound_to_main_flag;
}

static READ8_HANDLER( sound_to_main_r )
{
	sound_to_main_flag = 0;
	return soundlatch2_r(space, 0);
}

static WRITE8_HANDLER( main_to_sound_w )
{
	main_to_sound_flag = 1;
	soundlatch_w(space, 0, data);
	cputag_set_input_line(space->machine, "audiocpu", INPUT_LINE_NMI, PULSE_LINE);
}

static WRITE8_HANDLER( sound_reset_w )
{
	cputag_set_input_line(space->machine, "audiocpu", INPUT_LINE_RESET, (data & 0x80) ? ASSERT_LINE : CLEAR_LINE);
	if ((data & 0x80) != 0)
		sound_to_main_flag = main_to_sound_flag = 0;
}

static READ8_HANDLER( main_to_sound_r )
{
	main_to_sound_flag = 0;
	return soundlatch_r(space, 0);
}

static WRITE8_HANDLER( sound_to_main_w )
{
	sound_to_main_flag = 1;
	soundlatch2_w(space, 0, data);
}



/*************************************
 *
 *  6532 RIOT handlers
 *
 *************************************/

static READ8_DEVICE_HANDLER( riot_porta_r )
{
	/* bit 7 = MAINFLAG */
	/* bit 6 = SOUNDFLAG */
	/* bit 5 = PA5 */
	/* bit 4 = TEST */
	/* bit 3 = n/c */
	/* bit 2 = TMS /ready */
	/* bit 1 = TMS /read */
	/* bit 0 = TMS /write */

	return (main_to_sound_flag << 7) | (sound_to_main_flag << 6) | 0x10 | (tms5220_readyq_r(device) << 2);
}

static WRITE8_DEVICE_HANDLER( riot_porta_w )
{
	running_device *tms = device->machine->device("tms");

	/* handle 5220 read */
	tms5220_rsq_w(tms, (data>>1) & 1);

	/* handle 5220 write */
	tms5220_wsq_w(tms, data & 1);
}

static WRITE_LINE_DEVICE_HANDLER( riot_irq )
{
	cputag_set_input_line(device->machine, "audiocpu", M6502_IRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}



/*************************************
 *
 *  ADC input and control
 *
 *************************************/

static READ8_HANDLER( adc_r )
{
	if( control_num == 0 )
	{
		return input_port_read( space->machine, "PITCH" );
	}

	return input_port_read( space->machine, "YAW" );
}

static WRITE8_HANDLER( adc_select_w )
{
	control_num = offset;
}



/*************************************
 *
 *  Non-Volatile RAM (NOVRAM)
 *
 *************************************/

static WRITE8_HANDLER( nvram_w )
{
	x2212_write( nvram_1c, offset, data >> 4 );
	x2212_write( nvram_1d, offset, data & 0xf );
}

static READ8_HANDLER( nvram_r )
{
	return ( x2212_read( nvram_1c, offset ) << 4 ) | x2212_read( nvram_1d, offset );
}

static WRITE8_HANDLER( novram_recall_w )
{
	x2212_array_recall( nvram_1c, ( data >> 7 ) & 1 );
	x2212_array_recall( nvram_1d, ( data >> 7 ) & 1 );
}

static WRITE8_HANDLER( novram_store_w )
{
	x2212_store( nvram_1c, ( data >> 7 ) & 1 );
	x2212_store( nvram_1d, ( data >> 7 ) & 1 );
}



/*************************************
 *
 *  Main cpu
 *
 *************************************/

static WRITE8_HANDLER( rom_bank_w )
{
	memory_set_bank(space->machine, "bank1", data & 0x1f);
}

static WRITE8_HANDLER( main_irq_clear_w )
{
    cputag_set_input_line( space->machine, "maincpu", M6809_IRQ_LINE, CLEAR_LINE );
}

static WRITE8_HANDLER( main_firq_clear_w )
{
    cputag_set_input_line( space->machine, "maincpu", M6809_FIRQ_LINE, CLEAR_LINE );
}

static WRITE8_HANDLER( self_reset_w )
{
	cputag_set_input_line( space->machine, "maincpu", INPUT_LINE_RESET, PULSE_LINE );
}



/*************************************
 *
 *  I/O
 *
 *************************************/

static WRITE8_HANDLER( led_w )
{
    set_led_status( space->machine, offset, ( data & 0x80 ) == 0 );
}

static WRITE8_HANDLER( firefox_coin_counter_w )
{
	coin_counter_w( space->machine, offset, data & 0x80 );
}



static void firq_gen(running_device *device, int state)
{
	if (state)
	    cputag_set_input_line( device->machine, "maincpu", M6809_FIRQ_LINE, ASSERT_LINE );
}


static MACHINE_START( firefox )
{
	memory_configure_bank(machine, "bank1", 0, 32, memory_region(machine, "maincpu") + 0x10000, 0x1000);
	nvram_1c = machine->device("nvram_1c");
	nvram_1d = machine->device("nvram_1d");

	laserdisc = machine->device("laserdisc");
	vp931_set_data_ready_callback(laserdisc, firq_gen);

	control_num = 0;
	sprite_bank = 0;
}


/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 8)
	AM_RANGE(0x0000, 0x0fff) AM_RAM
	AM_RANGE(0x1000, 0x1fff) AM_RAM_WRITE(tileram_w) AM_BASE(&tileram)
	AM_RANGE(0x2000, 0x27ff) AM_RAM AM_BASE_GENERIC(spriteram)
	AM_RANGE(0x2800, 0x2aff) AM_RAM_WRITE(sprite_palette_w) AM_BASE(&sprite_palette)
	AM_RANGE(0x2b00, 0x2b00) AM_MIRROR(0x04ff) AM_WRITE(firefox_objram_bank_w)
	AM_RANGE(0x2c00, 0x2eff) AM_RAM_WRITE(tile_palette_w) AM_BASE(&tile_palette)
	AM_RANGE(0x3000, 0x3fff) AM_ROMBANK("bank1")
	AM_RANGE(0x4000, 0x40ff) AM_READWRITE(nvram_r, nvram_w)						/* NOVRAM */
	AM_RANGE(0x4100, 0x4100) AM_MIRROR(0x00f8) AM_READ_PORT("rdin0")			/* RDIN0 */
	AM_RANGE(0x4101, 0x4101) AM_MIRROR(0x00f8) AM_READ_PORT("rdin1")			/* RDIN1 */
	AM_RANGE(0x4102, 0x4102) AM_MIRROR(0x00f8) AM_READ(firefox_disc_status_r)	/* RDIN2 */
	AM_RANGE(0x4103, 0x4103) AM_MIRROR(0x00f8) AM_READ_PORT("opt0")				/* OPT0 */
	AM_RANGE(0x4104, 0x4104) AM_MIRROR(0x00f8) AM_READ_PORT("opt1")				/* OPT1 */
	AM_RANGE(0x4105, 0x4105) AM_MIRROR(0x00f8) AM_READ(firefox_disc_data_r)		/* DREAD */
	AM_RANGE(0x4106, 0x4106) AM_MIRROR(0x00f8) AM_READ(sound_to_main_r)			/* RDSOUND */
	AM_RANGE(0x4107, 0x4107) AM_MIRROR(0x00f8) AM_READ(adc_r)					/* ADC */
	AM_RANGE(0x4200, 0x4200) AM_MIRROR(0x0047) AM_WRITE(main_irq_clear_w)		/* RSTIRQ */
	AM_RANGE(0x4208, 0x4208) AM_MIRROR(0x0047) AM_WRITE(main_firq_clear_w)		/* RSTFIRQ */
	AM_RANGE(0x4210, 0x4210) AM_MIRROR(0x0047) AM_WRITE(watchdog_reset_w)		/* WDCLK */
	AM_RANGE(0x4218, 0x4218) AM_MIRROR(0x0047) AM_WRITE(firefox_disc_read_w)	/* DSKREAD */
	AM_RANGE(0x4220, 0x4223) AM_MIRROR(0x0044) AM_WRITE(adc_select_w)			/* ADCSTART */
	AM_RANGE(0x4230, 0x4230) AM_MIRROR(0x0047) AM_WRITE(self_reset_w)			/* AMUCK */
	AM_RANGE(0x4280, 0x4280) AM_MIRROR(0x0040) AM_WRITE(novram_recall_w)		/* LATCH0 -> NVRECALL */
	AM_RANGE(0x4281, 0x4281) AM_MIRROR(0x0040) AM_WRITE(sound_reset_w)			/* LATCH0 -> RSTSOUND */
	AM_RANGE(0x4282, 0x4282) AM_MIRROR(0x0040) AM_WRITE(novram_store_w)			/* LATCH0 -> NVRSTORE */
	AM_RANGE(0x4283, 0x4283) AM_MIRROR(0x0040) AM_WRITE(firefox_disc_lock_w)	/* LATCH0 -> LOCK */
	AM_RANGE(0x4284, 0x4285) AM_MIRROR(0x0040) AM_WRITE(audio_enable_w)			/* LATCH0 -> SWDSKR, SWDSKL */
	AM_RANGE(0x4286, 0x4286) AM_MIRROR(0x0040) AM_WRITE(firefox_disc_reset_w)	/* LATCH0 -> RSTDSK */
	AM_RANGE(0x4287, 0x4287) AM_MIRROR(0x0040) AM_WRITE(firefox_disc_write_w)	/* LATCH0 -> WRDSK */
	AM_RANGE(0x4288, 0x4289) AM_MIRROR(0x0040) AM_WRITE(firefox_coin_counter_w)	/* LATCH1 -> COIN COUNTERR, COUNTERL */
	AM_RANGE(0x428c, 0x428f) AM_MIRROR(0x0040) AM_WRITE(led_w)					/* LATCH1 -> LEDs */
	AM_RANGE(0x4290, 0x4290) AM_MIRROR(0x0047) AM_WRITE(rom_bank_w)				/* WRTREG */
	AM_RANGE(0x4298, 0x4298) AM_MIRROR(0x0047) AM_WRITE(main_to_sound_w)		/* WRSOUND */
	AM_RANGE(0x42a0, 0x42a0) AM_MIRROR(0x0047) AM_WRITE(firefox_disc_data_w)	/* DSKLATCH */
	AM_RANGE(0x4400, 0xffff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  Sound CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( audio_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_RAM
	AM_RANGE(0x0800, 0x087f) AM_MIRROR(0x0700) AM_RAM /* RIOT ram */
	AM_RANGE(0x0880, 0x089f) AM_MIRROR(0x07e0) AM_DEVREADWRITE("riot",riot6532_r, riot6532_w)
	AM_RANGE(0x1000, 0x1000) AM_READ(main_to_sound_r)
	AM_RANGE(0x1800, 0x1800) AM_WRITE(sound_to_main_w)
	AM_RANGE(0x2000, 0x200f) AM_DEVREADWRITE("pokey1", pokey_r, pokey_w)
	AM_RANGE(0x2800, 0x280f) AM_DEVREADWRITE("pokey2", pokey_r, pokey_w)
	AM_RANGE(0x3000, 0x300f) AM_DEVREADWRITE("pokey3", pokey_r, pokey_w)
	AM_RANGE(0x3800, 0x380f) AM_DEVREADWRITE("pokey4", pokey_r, pokey_w)
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( firefox )
	PORT_START("rdin0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT )
	PORT_SERVICE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("rdin1")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(mainflag_r, NULL)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(soundflag_r, NULL)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_VBLANK )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("opt0")
	PORT_DIPNAME(    0x03, 0x00, "Coins Per Credit" )
	PORT_DIPSETTING( 0x00, "1 Coin 1 Credit" )
	PORT_DIPSETTING( 0x01, "2 Coins 1 Credit" )
	PORT_DIPSETTING( 0x02, "3 Coins 1 Credit" )
	PORT_DIPSETTING( 0x03, "4 Coins 1 Credit" )
	PORT_DIPNAME(    0x0c, 0x00, "Right Coin" )
	PORT_DIPSETTING( 0x00, "1 Coin for 1 Coin Unit" )
	PORT_DIPSETTING( 0x04, "1 Coin for 4 Coin Units" )
	PORT_DIPSETTING( 0x08, "1 Coin for 5 Coin Units" )
	PORT_DIPSETTING( 0x0c, "1 Coin for 6 Coin Units" )
	PORT_DIPNAME(    0x10, 0x00, "Left Coin" )
	PORT_DIPSETTING( 0x00, "1 Coin for 1 Coin Unit" )
	PORT_DIPSETTING( 0x10, "1 Coin for 2 Coin Units" )
	PORT_DIPNAME(    0xe0, 0x00, "Bonus Adder" )
	PORT_DIPSETTING( 0x00, DEF_STR( None ) )
	PORT_DIPSETTING( 0x20, "1 Credit for 2 Coin Units" )
	PORT_DIPSETTING( 0xa0, "1 Credit for 3 Coin Units" )
	PORT_DIPSETTING( 0x40, "1 Credit for 4 Coin Units" )
	PORT_DIPSETTING( 0x80, "1 Credit for 5 Coin Units" )
	PORT_DIPSETTING( 0x60, "2 Credits for 4 Coin Units" )
	PORT_DIPSETTING( 0xe0, DEF_STR( Free_Play ) )

	PORT_START("opt1")
	PORT_DIPNAME( 0x01, 0x00, "Missions" )
	PORT_DIPSETTING(    0x00, "All .50" )
	PORT_DIPSETTING(    0x01, ".50 .75" )
	PORT_DIPNAME( 0x06, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, "Moderate" )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x06, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x18, 0x00, "Gas Usage" )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, "Moderate" )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x60, 0x00, "Bonus Gas" )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x20, "Moderate" )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x00, "Pro Limit" )
	PORT_DIPSETTING(    0x00, "Moderate" )
	PORT_DIPSETTING(    0x80, DEF_STR( Hardest ) )

	PORT_START("PITCH")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(70) PORT_KEYDELTA(30)

	PORT_START("YAW")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(30)
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout tilelayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ STEP8(0,4) },
	{ STEP8(0,32) },
	32*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,6),
	6,
	{ RGN_FRAC(0,6), RGN_FRAC(1,6), RGN_FRAC(2,6), RGN_FRAC(3,6), RGN_FRAC(4,6), RGN_FRAC(5,6) },
	{ STEP16(0,1) },
	{ STEP16(0,16) },
	32*8
};

static GFXDECODE_START( firefox )
	GFXDECODE_ENTRY("tiles",   0, tilelayout,   0,   1)
	GFXDECODE_ENTRY("sprites", 0, spritelayout, 256, 4)
GFXDECODE_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static const riot6532_interface riot_intf =
{
	DEVCB_DEVICE_HANDLER("tms", riot_porta_r),
	DEVCB_DEVICE_HANDLER("tms", tms5220_status_r),
	DEVCB_DEVICE_HANDLER("tms", riot_porta_w),
	DEVCB_DEVICE_HANDLER("tms", tms5220_data_w),
	DEVCB_LINE(riot_irq)
};


static MACHINE_DRIVER_START( firefox )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M6809E, MASTER_XTAL/2)
	MDRV_CPU_PROGRAM_MAP(main_map)
	/* interrupts count starting at end of VBLANK, which is 44, so add 44 */
	MDRV_TIMER_ADD_SCANLINE("32v", video_timer_callback, "screen", 96+44, 128)

	MDRV_CPU_ADD("audiocpu", M6502, MASTER_XTAL/8)
	MDRV_CPU_PROGRAM_MAP(audio_map)

	MDRV_QUANTUM_TIME(HZ(60000))

	MDRV_MACHINE_START(firefox)
	MDRV_WATCHDOG_TIME_INIT(HZ((double)MASTER_XTAL/8/16/16/16/16))

	/* video hardware */
	MDRV_LASERDISC_SCREEN_ADD_NTSC("screen", BITMAP_FORMAT_RGB32)

	MDRV_GFXDECODE(firefox)
	MDRV_PALETTE_LENGTH(512)

	MDRV_VIDEO_START(firefox)

	MDRV_LASERDISC_ADD("laserdisc", PHILLIPS_22VP931, "screen", "ldsound")
	MDRV_LASERDISC_OVERLAY(firefox, 64*8, 525, BITMAP_FORMAT_RGB32)
	MDRV_LASERDISC_OVERLAY_CLIP(7*8, 53*8-1, 44, 480+44)

	MDRV_X2212_ADD("nvram_1c")
	MDRV_X2212_ADD("nvram_1d")
	MDRV_RIOT6532_ADD("riot", MASTER_XTAL/8, riot_intf)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("pokey1", POKEY, MASTER_XTAL/8)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.30)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.30)

	MDRV_SOUND_ADD("pokey2", POKEY, MASTER_XTAL/8)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.30)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.30)

	MDRV_SOUND_ADD("pokey3", POKEY, MASTER_XTAL/8)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.30)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.30)

	MDRV_SOUND_ADD("pokey4", POKEY, MASTER_XTAL/8)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.30)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.30)

	MDRV_SOUND_ADD("tms", TMS5220, MASTER_XTAL/2/11)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.75)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.75)

	MDRV_SOUND_ADD("ldsound", LASERDISC, 0)
	MDRV_SOUND_ROUTE(0, "lspeaker", 0.50)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.50)
MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( firefox )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* 64k for code + data & 128k for banked roms */
	ROM_LOAD( "136026.209",     0x04000, 0x4000, CRC(9f559f1b) SHA1(142d14cb5158ea77f6fc6d9bf0ce723842f345e2) ) /* 8b/c */
	ROM_LOAD( "136026.210",     0x08000, 0x4000, CRC(d769b40d) SHA1(2d354649a381f3399cb0161267bd1c36a8f2bb4b) ) /* 7b/c */
	ROM_LOAD( "136026.211",     0x0c000, 0x4000, CRC(7293ab03) SHA1(73d0d173da295ad59e431bab0a9814a71146cbc2) ) /* 6b/c */
	ROM_LOAD( "136026.201",     0x10000, 0x4000, CRC(c118547a) SHA1(4d3502cbde3116588ed944bf1750bab50e4c813c) ) /* 8a */
	/* empty 7a */
	/* empty 6a */
	/* empty 5a */
	ROM_LOAD( "136026.205",     0x20000, 0x4000, CRC(dc21677f) SHA1(576a96c1e07e1362a0a367e76dc369ee8a950144) ) /* 4a */
	ROM_LOAD( "136026.127",     0x24000, 0x2000, CRC(c0c765ab) SHA1(79f6c8c1d00684d7143b2d33a5669bdf5cd01e96) ) /* 3a */
	ROM_RELOAD( 0x26000, 0x2000 )
	/* empty 2a */
	/* empty 1a */

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k for code */
	/* empty 4k/l */
	ROM_LOAD( "136026.128",     0x08000, 0x2000, CRC(5358d870) SHA1(e8f2983a7e612e1a050a3c0b9f19b1077de4c146) ) /* 4m */
	ROM_RELOAD( 0x0a000, 0x2000 )
	ROM_LOAD( "136026.214",     0x0c000, 0x4000, CRC(92378b78) SHA1(62c7a1fee675fa3f9125f8e208b8207f0ce28bbe) ) /* 4n */

	ROM_REGION( 0x2000, "tiles", 0 )
	ROM_LOAD( "136026.125",     0x0000,  0x2000, CRC(8a32f9f1) SHA1(f899174f55cd4a24a3be4a0f4bb44d3e8e938586) ) /* 6p */

	ROM_REGION( 0x30000, "sprites", ROMREGION_ERASE00 )
	/* empty 6c */
	/* empty 6a */
	ROM_LOAD( "136026.124",     0x08000,  0x4000, CRC(5efe0f6c) SHA1(df35fd9267d966ab379c2f78ed418f4606741b28)) /* 5c */
	ROM_LOAD( "136026.123",     0x0c000,  0x4000, CRC(dffe48b3) SHA1(559907651bb425e26a834b467959b15092d23d27)) /* 5a */
	ROM_LOAD( "136026.118",     0x10000,  0x4000, CRC(0ed4df15) SHA1(7aa599f428112fff4bfedf63fafc22f19fa66546)) /* 4c */
	ROM_LOAD( "136026.122",     0x14000,  0x4000, CRC(8e2c6616) SHA1(59cbd585028bb634034a9dfd552275bd41f01989)) /* 4a */
	ROM_LOAD( "136026.117",     0x18000,  0x4000, CRC(79129084) SHA1(4219ff7cd444ad11e4cb9f1c30ac15fe0cfc5a17)) /* 3c */
	ROM_LOAD( "136026.121",     0x1c000,  0x4000, CRC(494972d4) SHA1(fa0e24e911b233e9644d7794ba03f76bfd39aa8c)) /* 3a */
	ROM_LOAD( "136026.116",     0x20000,  0x4000, CRC(d5282d4e) SHA1(de5fdf82a615625aa77b39e035b4206216faaf9c)) /* 2c */
	ROM_LOAD( "136026.120",     0x24000,  0x4000, CRC(e1b95923) SHA1(b6d0c0af0a8f55e728cd0f4c3222745eefd57f50)) /* 2a */
	ROM_LOAD( "136026.115",     0x28000,  0x4000, CRC(861abc82) SHA1(1845888d07162ae915364a2a91294731f1c5b3bd)) /* 1c */
	ROM_LOAD( "136026.119",     0x2c000,  0x4000, CRC(959471b1) SHA1(a032209a209f51d34360d5c7ad32ec62150158d2)) /* 1a */

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "firefox", 0, SHA1(3c4be40f55b44d0352b64c0861b6d1b650451ce7) )
ROM_END

ROM_START( firefoxa )
	ROM_REGION( 0x30000, "maincpu", 0 ) /* 64k for code + data & 128k for banked roms */
	ROM_LOAD( "136026.109",     0x04000, 0x4000, CRC(7639270c) SHA1(1b8f53c516d26aecb4478ac99783a37e5b1a107f)) /* 8b/c */
	ROM_LOAD( "136026.110",     0x08000, 0x4000, CRC(f3102944) SHA1(460f18180b19b6360c99c7e70f86d745f69ba95d)) /* 7b/c */
	ROM_LOAD( "136026.111",     0x0c000, 0x4000, CRC(8a230bb5) SHA1(0cfa1e981e4a8ccaf5903b4e761a2085b5a56181)) /* 6b/c */
	ROM_LOAD( "136026.101",     0x10000, 0x4000, CRC(91bba45a) SHA1(d584a8f60bbbdbe250978b7aeb3f5e7698f94d60)) /* 8a */
	ROM_LOAD( "136026.102",     0x14000, 0x4000, CRC(5f1e423d) SHA1(c55c27600877272c1ca94eab75c1eb25ff84d36f)) /* 7a */
	/* empty 6a */
	/* empty 5a */
	ROM_LOAD( "136026.105",     0x20000, 0x4000, CRC(83f1d4ed) SHA1(ed4b22b3473f16cbcca1415f6d81be558ab10ff3)) /* 4a */
	ROM_LOAD( "136026.106",     0x24000, 0x4000, CRC(c5d8d417) SHA1(6a29595b2c091bbcf413c7213c6577eaf9c507d1)) /* 3a */
	/* empty 2a */
	/* empty 1a */

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k for code */
	/* empty 4k/l */
	ROM_LOAD( "136026.113",     0x08000, 0x4000, CRC(90988b3b) SHA1(7571cf6b7e9e3e22f930d9ba991b730e734edfb7)) /* 4m */
	ROM_LOAD( "136026.114",     0x0c000, 0x4000, CRC(1437ce14) SHA1(eef14172b3935a4afb3470852f93d30926b139e4)) /* 4n */

	ROM_REGION( 0x2000, "tiles", 0 )
	ROM_LOAD( "136026.125",     0x0000,  0x2000, CRC(8a32f9f1) SHA1(f899174f55cd4a24a3be4a0f4bb44d3e8e938586) ) /* 6p */

	ROM_REGION( 0x30000, "sprites", ROMREGION_ERASE00 )
	/* empty 6c */
	/* empty 6a */
	ROM_LOAD( "136026.124",     0x08000,  0x4000, CRC(5efe0f6c) SHA1(df35fd9267d966ab379c2f78ed418f4606741b28)) /* 5c */
	ROM_LOAD( "136026.123",     0x0c000,  0x4000, CRC(dffe48b3) SHA1(559907651bb425e26a834b467959b15092d23d27)) /* 5a */
	ROM_LOAD( "136026.118",     0x10000,  0x4000, CRC(0ed4df15) SHA1(7aa599f428112fff4bfedf63fafc22f19fa66546)) /* 4c */
	ROM_LOAD( "136026.122",     0x14000,  0x4000, CRC(8e2c6616) SHA1(59cbd585028bb634034a9dfd552275bd41f01989)) /* 4a */
	ROM_LOAD( "136026.117",     0x18000,  0x4000, CRC(79129084) SHA1(4219ff7cd444ad11e4cb9f1c30ac15fe0cfc5a17)) /* 3c */
	ROM_LOAD( "136026.121",     0x1c000,  0x4000, CRC(494972d4) SHA1(fa0e24e911b233e9644d7794ba03f76bfd39aa8c)) /* 3a */
	ROM_LOAD( "136026.116",     0x20000,  0x4000, CRC(d5282d4e) SHA1(de5fdf82a615625aa77b39e035b4206216faaf9c)) /* 2c */
	ROM_LOAD( "136026.120",     0x24000,  0x4000, CRC(e1b95923) SHA1(b6d0c0af0a8f55e728cd0f4c3222745eefd57f50)) /* 2a */
	ROM_LOAD( "136026.115",     0x28000,  0x4000, CRC(861abc82) SHA1(1845888d07162ae915364a2a91294731f1c5b3bd)) /* 1c */
	ROM_LOAD( "136026.119",     0x2c000,  0x4000, CRC(959471b1) SHA1(a032209a209f51d34360d5c7ad32ec62150158d2)) /* 1a */

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "firefox", 0, SHA1(3c4be40f55b44d0352b64c0861b6d1b650451ce7) )
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1984, firefox,  0,       firefox, firefox, 0, ROT0, "Atari", "Fire Fox (set 1)", 0 )
GAME( 1984, firefoxa, firefox, firefox, firefox, 0, ROT0, "Atari", "Fire Fox (set 2)", 0 )
