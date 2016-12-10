/***************************************************************************

                              -= Tetris Plus 2 =-

                    driver by   Luca Elia (l.elia@tin.it)


Main  CPU    :  TMP68HC000P-12

Video Chips  :  SS91022-03 9428XX001
                GS91022-04 9721PD008
                SS91022-05 9347EX002
                GS91022-05 048 9726HX002

Sound Chips  :  Yamaha YMZ280B-F

Other        :  XILINX XC5210 PQ240C X68710M AKJ9544
                XC7336 PC44ACK9633 A63458A
                NVRAM


To Do:

-   There is a 3rd unimplemented layer capable of rotation (not used by
    the game, can be tested in service mode).
-   Priority RAM is not taken into account.
-   The video system on this hardware is almost the same as the MegaSystem 32 board

Notes:

-   The Japan set of Tetris Plus 2 doesn't seem to have (or use) NVRAM. I can't
    enter a test mode or use the service coin either !?

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"
#include "sound/ymz280b.h"
#include "rocknms.lh"
#include "includes/tetrisp2.h"

UINT16 tetrisp2_systemregs[0x10];
static UINT16 rocknms_sub_systemregs[0x10];

static UINT16 rockn_protectdata;
static UINT16 rockn_adpcmbank;
static UINT16 rockn_soundvolume;

static emu_timer *rockn_timer_l4;
static emu_timer *rockn_timer_sub_l4;

/***************************************************************************


                              System Registers


***************************************************************************/

static WRITE16_HANDLER( tetrisp2_systemregs_w )
{
	if (ACCESSING_BITS_0_7)
	{
		tetrisp2_systemregs[offset] = data;
	}
}

#define ROCKN_TIMER_BASE ATTOTIME_IN_NSEC(500000)

static WRITE16_HANDLER( rockn_systemregs_w )
{
	if (ACCESSING_BITS_0_7)
	{
		tetrisp2_systemregs[offset] = data;
		if (offset == 0x0c)
		{
			attotime timer = attotime_mul(ROCKN_TIMER_BASE, 4096 - data);
			timer_adjust_periodic(rockn_timer_l4, timer, 0, timer);
		}
	}
}


static WRITE16_HANDLER( rocknms_sub_systemregs_w )
{
	if (ACCESSING_BITS_0_7)
	{
		rocknms_sub_systemregs[offset] = data;
		if (offset == 0x0c)
		{
			attotime timer = attotime_mul(ROCKN_TIMER_BASE, 4096 - data);
			timer_adjust_periodic(rockn_timer_sub_l4, timer, 0, timer);
		}
	}
}


/***************************************************************************


                                    Sound


***************************************************************************/

static READ16_HANDLER( rockn_adpcmbank_r )
{
	return ((rockn_adpcmbank & 0xf0ff) | (rockn_protectdata << 8));
}

static WRITE16_HANDLER( rockn_adpcmbank_w )
{
	UINT8 *SNDROM = memory_region(space->machine, "ymz");
	int bank;

	rockn_adpcmbank = data;
	bank = ((data & 0x001f) >> 2);

	if (bank > 7)
	{
		popmessage("!!!!! ADPCM BANK OVER:%01X (%04X) !!!!!", bank, data);
		bank = 0;
	}

	memcpy(&SNDROM[0x0400000], &SNDROM[0x1000000 + (0x0c00000 * bank)], 0x0c00000);
}

static WRITE16_HANDLER( rockn2_adpcmbank_w )
{
	UINT8 *SNDROM = memory_region(space->machine, "ymz");
	int bank;

	char banktable[9][3]=
	{
		{  0,  1,  2 },		// bank $00
		{  3,  4,  5 },		// bank $04
		{  6,  7,  8 },		// bank $08
		{  9, 10, 11 },		// bank $0c
		{ 12, 13, 14 },		// bank $10
		{ 15, 16, 17 },		// bank $14
		{ 18, 19, 20 },		// bank $18
		{  0,  0,  0 },		// bank $1c
		{  0,  5, 14 },		// bank $20
	};

	rockn_adpcmbank = data;
	bank = ((data & 0x003f) >> 2);

	if (bank > 8)
	{
		popmessage("!!!!! ADPCM BANK OVER:%01X (%04X) !!!!!", bank, data);
		bank = 0;
	}

	memcpy(&SNDROM[0x0400000], &SNDROM[0x1000000 + (0x0400000 * banktable[bank][0] )], 0x0400000);
	memcpy(&SNDROM[0x0800000], &SNDROM[0x1000000 + (0x0400000 * banktable[bank][1] )], 0x0400000);
	memcpy(&SNDROM[0x0c00000], &SNDROM[0x1000000 + (0x0400000 * banktable[bank][2] )], 0x0400000);
}


static READ16_HANDLER( rockn_soundvolume_r )
{
	return 0xffff;
}

static WRITE16_HANDLER( rockn_soundvolume_w )
{
	rockn_soundvolume = data;
}


static WRITE16_HANDLER( nndmseal_sound_bank_w )
{
	static int bank_lo, bank_hi;

	if (ACCESSING_BITS_0_7)
	{
		UINT8 *rom = memory_region(space->machine, "okisource");

		if (data & 0x04)
		{
			bank_lo = data & 0x03;

			memcpy(memory_region(space->machine, "oki"), rom + (bank_lo * 0x80000), 0x20000);

//          logerror("PC:%06X sound bank_lo = %02X\n",cpu_get_pc(space->cpu),bank_lo);
		}
		else
		{
			bank_hi = data & 0x03;

			memcpy(memory_region(space->machine, "oki") + 0x20000, rom + (bank_lo * 0x80000) + (bank_hi * 0x20000), 0x20000);

//          logerror("PC:%06X sound bank_hi = %02X\n",cpu_get_pc(space->cpu),bank_hi);
		}
	}
}

/***************************************************************************


                                Protection


***************************************************************************/

static READ16_HANDLER( tetrisp2_ip_1_word_r )
{
	return	( input_port_read(space->machine, "SYSTEM") &  0xfcff ) |
			(           mame_rand(space->machine) & ~0xfcff ) |
			(      1 << (8 + (mame_rand(space->machine)&1)) );
}


/***************************************************************************


                                    NVRAM


***************************************************************************/

static UINT16 *tetrisp2_nvram;
static size_t tetrisp2_nvram_size;

static NVRAM_HANDLER( tetrisp2 )
{
	if (read_or_write)
		mame_fwrite(file,tetrisp2_nvram,tetrisp2_nvram_size);
	else
	{
		if (file)
			mame_fread(file,tetrisp2_nvram,tetrisp2_nvram_size);
		else
		{
			/* fill in the default values */
			memset(tetrisp2_nvram,0,tetrisp2_nvram_size);
		}
	}
}


/* The game only ever writes even bytes and reads odd bytes */
static READ16_HANDLER( tetrisp2_nvram_r )
{
	return	( (tetrisp2_nvram[offset] >> 8) & 0x00ff ) |
			( (tetrisp2_nvram[offset] << 8) & 0xff00 ) ;
}

static WRITE16_HANDLER( tetrisp2_nvram_w )
{
	COMBINE_DATA(&tetrisp2_nvram[offset]);
}

static READ16_HANDLER( rockn_nvram_r )
{
	return	tetrisp2_nvram[offset];
}


/***************************************************************************





***************************************************************************/

static UINT16 rocknms_main2sub;
static UINT16 rocknms_sub2main;

static READ16_HANDLER( rocknms_main2sub_r )
{
	return rocknms_main2sub;
}

static WRITE16_HANDLER( rocknms_main2sub_w )
{
	if (ACCESSING_BITS_0_7)
		rocknms_main2sub = (data ^ 0xffff);
}

static CUSTOM_INPUT( rocknms_main2sub_status_r )
{
	return  rocknms_sub2main & 0x0003;
}

static WRITE16_HANDLER( rocknms_sub2main_w )
{
	if (ACCESSING_BITS_0_7)
		rocknms_sub2main = (data ^ 0xffff);
}


static WRITE16_HANDLER( tetrisp2_coincounter_w )
{
	coin_counter_w( space->machine, 0, (data & 0x0001));
}



/***************************************************************************


                                Memory Map


***************************************************************************/

static ADDRESS_MAP_START( tetrisp2_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM															// ROM
	AM_RANGE(0x100000, 0x103fff) AM_RAM AM_BASE_SIZE_GENERIC(spriteram)			// Object RAM
	AM_RANGE(0x104000, 0x107fff) AM_RAM															// Spare Object RAM
	AM_RANGE(0x108000, 0x10ffff) AM_RAM															// Work RAM
	AM_RANGE(0x200000, 0x23ffff) AM_READWRITE8(tetrisp2_priority_r, tetrisp2_priority_w, 0x00ff)
	AM_RANGE(0x300000, 0x31ffff) AM_RAM_WRITE(tetrisp2_palette_w) AM_BASE_GENERIC(paletteram)		// Palette
	AM_RANGE(0x400000, 0x403fff) AM_RAM_WRITE(tetrisp2_vram_fg_w) AM_BASE(&tetrisp2_vram_fg)	// Foreground
	AM_RANGE(0x404000, 0x407fff) AM_RAM_WRITE(tetrisp2_vram_bg_w) AM_BASE(&tetrisp2_vram_bg)	// Background
	AM_RANGE(0x408000, 0x409fff) AM_RAM															// ???
	AM_RANGE(0x500000, 0x50ffff) AM_RAM															// Line
	AM_RANGE(0x600000, 0x60ffff) AM_RAM_WRITE(tetrisp2_vram_rot_w) AM_BASE(&tetrisp2_vram_rot)	// Rotation
	AM_RANGE(0x650000, 0x651fff) AM_RAM_WRITE(tetrisp2_vram_rot_w)								// Rotation (mirror)
	AM_RANGE(0x800000, 0x800003) AM_DEVREADWRITE8("ymz", ymz280b_r, ymz280b_w, 0x00ff)	// Sound
	AM_RANGE(0x900000, 0x903fff) AM_READWRITE(tetrisp2_nvram_r, tetrisp2_nvram_w) AM_BASE(&tetrisp2_nvram) AM_SIZE(&tetrisp2_nvram_size)	// NVRAM
	AM_RANGE(0x904000, 0x907fff) AM_READWRITE(tetrisp2_nvram_r, tetrisp2_nvram_w)				// NVRAM (mirror)
	AM_RANGE(0xb00000, 0xb00001) AM_WRITE(tetrisp2_coincounter_w)								// Coin Counter
	AM_RANGE(0xb20000, 0xb20001) AM_WRITENOP													// ???
	AM_RANGE(0xb40000, 0xb4000b) AM_WRITEONLY AM_BASE(&tetrisp2_scroll_fg)						// Foreground Scrolling
	AM_RANGE(0xb40010, 0xb4001b) AM_WRITEONLY AM_BASE(&tetrisp2_scroll_bg)						// Background Scrolling
	AM_RANGE(0xb4003e, 0xb4003f) AM_WRITENOP													// scr_size
	AM_RANGE(0xb60000, 0xb6002f) AM_WRITEONLY AM_BASE(&tetrisp2_rotregs)						// Rotation Registers
	AM_RANGE(0xba0000, 0xba001f) AM_WRITE(tetrisp2_systemregs_w)								// system param
	AM_RANGE(0xba001a, 0xba001b) AM_WRITENOP													// Lev 4 irq ack
	AM_RANGE(0xba001e, 0xba001f) AM_WRITENOP													// Lev 2 irq ack
	AM_RANGE(0xbe0000, 0xbe0001) AM_READNOP														// INT-level1 dummy read
	AM_RANGE(0xbe0002, 0xbe0003) AM_READ_PORT("PLAYERS")										// Inputs
	AM_RANGE(0xbe0004, 0xbe0005) AM_READ(tetrisp2_ip_1_word_r)									// Inputs & protection
	AM_RANGE(0xbe0008, 0xbe0009) AM_READ_PORT("DSW")											// Inputs
	AM_RANGE(0xbe000a, 0xbe000b) AM_READ(watchdog_reset16_r)									// Watchdog
ADDRESS_MAP_END


static WRITE16_HANDLER( nndmseal_coincounter_w )
{
	if (ACCESSING_BITS_0_7)
	{
		coin_counter_w( space->machine, 0,  data  & 0x0001 );
		//                  data  & 0x0004 ?
		coin_lockout_w( space->machine, 0,(~data) & 0x0008 );
	}
	if (ACCESSING_BITS_8_15)
	{
		set_led_status( space->machine, 0, data & 0x1000 );	// +
		set_led_status( space->machine, 1, data & 0x2000 );	// -
		set_led_status( space->machine, 2, data & 0x4000 );	// Cancel
		set_led_status( space->machine, 3, data & 0x8000 );	// OK
	}
//  popmessage("%04x",data);
}

static WRITE16_HANDLER( nndmseal_b20000_w )
{
	// leds?
//  popmessage("%04x",data);
}

static ADDRESS_MAP_START( nndmseal_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x100000, 0x103fff) AM_RAM AM_BASE_SIZE_GENERIC(spriteram)	// Object RAM
	AM_RANGE(0x104000, 0x107fff) AM_RAM	// Spare Object RAM
	AM_RANGE(0x108000, 0x10ffff) AM_RAM	// Work RAM
	AM_RANGE(0x200000, 0x23ffff) AM_WRITE8(tetrisp2_priority_w, 0x00ff)	// Priority
	AM_RANGE(0x200000, 0x23ffff) AM_READ(nndmseal_priority_r)
	AM_RANGE(0x300000, 0x31ffff) AM_RAM_WRITE(tetrisp2_palette_w) AM_BASE_GENERIC(paletteram)	// Palette
	AM_RANGE(0x400000, 0x403fff) AM_RAM_WRITE(tetrisp2_vram_fg_w) AM_BASE(&tetrisp2_vram_fg	)	// Foreground
	AM_RANGE(0x404000, 0x407fff) AM_RAM_WRITE(tetrisp2_vram_bg_w) AM_BASE(&tetrisp2_vram_bg	)	// Background

	AM_RANGE(0x408000, 0x409fff) AM_RAM	// ???
	AM_RANGE(0x500000, 0x50ffff) AM_RAM	// Line

	AM_RANGE(0x600000, 0x60ffff) AM_RAM_WRITE(tetrisp2_vram_rot_w) AM_BASE(&tetrisp2_vram_rot	)	// Rotation
	AM_RANGE(0x650000, 0x651fff) AM_RAM_WRITE(tetrisp2_vram_rot_w)	// Rotation (mirror)

	AM_RANGE(0x800000, 0x800003) AM_DEVREADWRITE8( "oki", okim6295_r, okim6295_w, 0x00ff )	// Sound

	AM_RANGE(0x900000, 0x903fff) AM_READWRITE(tetrisp2_nvram_r, tetrisp2_nvram_w) AM_BASE(&tetrisp2_nvram) AM_SIZE(&tetrisp2_nvram_size	)	// NVRAM

	AM_RANGE(0xb00000, 0xb00001) AM_WRITE(nndmseal_coincounter_w)	// Coin Counter
	AM_RANGE(0xb20000, 0xb20001) AM_WRITE(nndmseal_b20000_w)		// ???

	AM_RANGE(0xb40000, 0xb4000b) AM_WRITEONLY AM_BASE(&tetrisp2_scroll_fg	)	// Foreground Scrolling
	AM_RANGE(0xb40010, 0xb4001b) AM_WRITEONLY AM_BASE(&tetrisp2_scroll_bg	)	// Background Scrolling
	AM_RANGE(0xb4003e, 0xb4003f) AM_WRITENOP	// scr_size

	AM_RANGE(0xb60000, 0xb6002f) AM_WRITEONLY AM_BASE(&tetrisp2_rotregs)	// Rotation Registers

	AM_RANGE(0xb80000, 0xb80001) AM_WRITE(nndmseal_sound_bank_w)

	AM_RANGE(0xba0000, 0xba001f) AM_WRITE(rockn_systemregs_w	)	// system param
	AM_RANGE(0xba001a, 0xba001b) AM_WRITENOP	// Lev 4 irq ack
	AM_RANGE(0xba001e, 0xba001f) AM_WRITENOP	// Lev 2 irq ack

	AM_RANGE(0xbe0000, 0xbe0001) AM_READNOP	// INT-level1 dummy read
	AM_RANGE(0xbe0002, 0xbe0003) AM_READ_PORT("BUTTONS"			)	// Inputs
	AM_RANGE(0xbe0004, 0xbe0005) AM_READ_PORT("COINS"			)	// ""
	AM_RANGE(0xbe0006, 0xbe0007) AM_READ_PORT("PRINT"			)	// ""
	AM_RANGE(0xbe0008, 0xbe0009) AM_READ_PORT("DSW"				)	// ""

	AM_RANGE(0xbe000a, 0xbe000b) AM_READ(watchdog_reset16_r		)	// Watchdog
ADDRESS_MAP_END


static ADDRESS_MAP_START( rockn1_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM															// ROM
	AM_RANGE(0x100000, 0x103fff) AM_RAM AM_BASE_SIZE_GENERIC(spriteram)			// Object RAM
	AM_RANGE(0x104000, 0x107fff) AM_RAM															// Spare Object RAM
	AM_RANGE(0x108000, 0x10ffff) AM_RAM															// Work RAM
	AM_RANGE(0x200000, 0x23ffff) AM_READWRITE8(tetrisp2_priority_r, rockn_priority_w, 0x00ff)		// Priority
	AM_RANGE(0x300000, 0x31ffff) AM_RAM_WRITE(tetrisp2_palette_w) AM_BASE_GENERIC(paletteram)		// Palette
	AM_RANGE(0x400000, 0x403fff) AM_RAM_WRITE(tetrisp2_vram_fg_w) AM_BASE(&tetrisp2_vram_fg)	// Foreground
	AM_RANGE(0x404000, 0x407fff) AM_RAM_WRITE(tetrisp2_vram_bg_w) AM_BASE(&tetrisp2_vram_bg)	// Background
	AM_RANGE(0x408000, 0x409fff) AM_RAM															// ???
	AM_RANGE(0x500000, 0x50ffff) AM_RAM															// Line
	AM_RANGE(0x600000, 0x60ffff) AM_RAM_WRITE(tetrisp2_vram_rot_w) AM_BASE(&tetrisp2_vram_rot)	// Rotation
	AM_RANGE(0x900000, 0x903fff) AM_READWRITE(rockn_nvram_r, tetrisp2_nvram_w) AM_BASE(&tetrisp2_nvram) AM_SIZE(&tetrisp2_nvram_size)	// NVRAM
	AM_RANGE(0xa30000, 0xa30001) AM_READWRITE(rockn_soundvolume_r, rockn_soundvolume_w)			// Sound Volume
	AM_RANGE(0xa40000, 0xa40003) AM_DEVREADWRITE8("ymz", ymz280b_r, ymz280b_w, 0x00ff)	// Sound
	AM_RANGE(0xa44000, 0xa44001) AM_READWRITE(rockn_adpcmbank_r, rockn_adpcmbank_w)				// Sound Bank
	AM_RANGE(0xa48000, 0xa48001) AM_NOP															// YMZ280 Reset
	AM_RANGE(0xb00000, 0xb00001) AM_WRITE(tetrisp2_coincounter_w)								// Coin Counter
	AM_RANGE(0xb20000, 0xb20001) AM_NOP															// ???
	AM_RANGE(0xb40000, 0xb4000b) AM_WRITEONLY AM_BASE(&tetrisp2_scroll_fg)						// Foreground Scrolling
	AM_RANGE(0xb40010, 0xb4001b) AM_WRITEONLY AM_BASE(&tetrisp2_scroll_bg)						// Background Scrolling
	AM_RANGE(0xb4003e, 0xb4003f) AM_WRITENOP													// scr_size
	AM_RANGE(0xb60000, 0xb6002f) AM_WRITEONLY AM_BASE(&tetrisp2_rotregs)						// Rotation Registers
	AM_RANGE(0xba0000, 0xba001f) AM_WRITE(rockn_systemregs_w)									// system param
	AM_RANGE(0xba001a, 0xba001b) AM_WRITENOP													// Lev 4 irq ack
	AM_RANGE(0xba001e, 0xba001f) AM_WRITENOP													// Lev 2 irq ack
	AM_RANGE(0xbe0000, 0xbe0001) AM_READNOP														// INT-level1 dummy read
	AM_RANGE(0xbe0002, 0xbe0003) AM_READ_PORT("PLAYERS")										// Inputs
	AM_RANGE(0xbe0004, 0xbe0005) AM_READ_PORT("SYSTEM")											// Inputs
	AM_RANGE(0xbe0008, 0xbe0009) AM_READ_PORT("DSW")											// Inputs
	AM_RANGE(0xbe000a, 0xbe000b) AM_READ(watchdog_reset16_r)									// Watchdog
ADDRESS_MAP_END


static ADDRESS_MAP_START( rockn2_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM															// ROM
	AM_RANGE(0x100000, 0x103fff) AM_RAM AM_BASE_SIZE_GENERIC(spriteram)			// Object RAM
	AM_RANGE(0x104000, 0x107fff) AM_RAM															// Spare Object RAM
	AM_RANGE(0x108000, 0x10ffff) AM_RAM															// Work RAM
	AM_RANGE(0x200000, 0x23ffff) AM_READWRITE8(tetrisp2_priority_r, rockn_priority_w, 0x00ff)	// Priority
	AM_RANGE(0x300000, 0x31ffff) AM_RAM_WRITE(tetrisp2_palette_w) AM_BASE_GENERIC(paletteram)		// Palette
	AM_RANGE(0x500000, 0x50ffff) AM_RAM															// Line
	AM_RANGE(0x600000, 0x60ffff) AM_RAM_WRITE(tetrisp2_vram_rot_w) AM_BASE(&tetrisp2_vram_rot)	// Rotation
	AM_RANGE(0x800000, 0x803fff) AM_RAM_WRITE(tetrisp2_vram_fg_w) AM_BASE(&tetrisp2_vram_fg)	// Foreground
	AM_RANGE(0x804000, 0x807fff) AM_RAM_WRITE(tetrisp2_vram_bg_w) AM_BASE(&tetrisp2_vram_bg)	// Background
	AM_RANGE(0x808000, 0x809fff) AM_RAM															// ???
	AM_RANGE(0x900000, 0x903fff) AM_READWRITE(rockn_nvram_r, tetrisp2_nvram_w) AM_BASE(&tetrisp2_nvram) AM_SIZE(&tetrisp2_nvram_size)	// NVRAM
	AM_RANGE(0xa30000, 0xa30001) AM_READWRITE(rockn_soundvolume_r, rockn_soundvolume_w)			// Sound Volume
	AM_RANGE(0xa40000, 0xa40003) AM_DEVREADWRITE8("ymz", ymz280b_r, ymz280b_w, 0x00ff)	// Sound
	AM_RANGE(0xa44000, 0xa44001) AM_READWRITE(rockn_adpcmbank_r, rockn2_adpcmbank_w)			// Sound Bank
	AM_RANGE(0xa48000, 0xa48001) AM_WRITENOP													// YMZ280 Reset
	AM_RANGE(0xb00000, 0xb00001) AM_WRITE(tetrisp2_coincounter_w)								// Coin Counter
	AM_RANGE(0xb20000, 0xb20001) AM_WRITENOP													// ???
	AM_RANGE(0xb40000, 0xb4000b) AM_WRITEONLY AM_BASE(&tetrisp2_scroll_fg)					// Foreground Scrolling
	AM_RANGE(0xb40010, 0xb4001b) AM_WRITEONLY AM_BASE(&tetrisp2_scroll_bg)					// Background Scrolling
	AM_RANGE(0xb4003e, 0xb4003f) AM_WRITENOP													// scr_size
	AM_RANGE(0xb60000, 0xb6002f) AM_WRITEONLY AM_BASE(&tetrisp2_rotregs)					// Rotation Registers
	AM_RANGE(0xba0000, 0xba001f) AM_WRITE(rockn_systemregs_w)									// system param
	AM_RANGE(0xba001a, 0xba001b) AM_WRITENOP													// Lev 4 irq ack
	AM_RANGE(0xba001e, 0xba001f) AM_WRITENOP													// Lev 2 irq ack
	AM_RANGE(0xbe0000, 0xbe0001) AM_READNOP														// INT-level1 dummy read
	AM_RANGE(0xbe0002, 0xbe0003) AM_READ_PORT("PLAYERS")										// Inputs
	AM_RANGE(0xbe0004, 0xbe0005) AM_READ_PORT("SYSTEM")											// Inputs
	AM_RANGE(0xbe0008, 0xbe0009) AM_READ_PORT("DSW")											// Inputs
	AM_RANGE(0xbe000a, 0xbe000b) AM_READ(watchdog_reset16_r)									// Watchdog
ADDRESS_MAP_END


static ADDRESS_MAP_START( rocknms_main_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM															// ROM
	AM_RANGE(0x100000, 0x103fff) AM_RAM AM_BASE_SIZE_GENERIC(spriteram)			// Object RAM
	AM_RANGE(0x104000, 0x107fff) AM_RAM															// Spare Object RAM
	AM_RANGE(0x108000, 0x10ffff) AM_RAM															// Work RAM
	AM_RANGE(0x200000, 0x23ffff) AM_READWRITE8(tetrisp2_priority_r, rockn_priority_w, 0x00ff)		// Priority
	AM_RANGE(0x300000, 0x31ffff) AM_RAM_WRITE(tetrisp2_palette_w) AM_BASE_GENERIC(paletteram)		// Palette
//  AM_RANGE(0x500000, 0x50ffff) AM_RAM                                                         // Line
	AM_RANGE(0x600000, 0x60ffff) AM_RAM_WRITE(tetrisp2_vram_rot_w) AM_BASE(&tetrisp2_vram_rot)	// Rotation
	AM_RANGE(0x800000, 0x803fff) AM_RAM_WRITE(tetrisp2_vram_fg_w) AM_BASE(&tetrisp2_vram_fg)	// Foreground
	AM_RANGE(0x804000, 0x807fff) AM_RAM_WRITE(tetrisp2_vram_bg_w) AM_BASE(&tetrisp2_vram_bg)	// Background
//  AM_RANGE(0x808000, 0x809fff) AM_RAM                                                         // ???
	AM_RANGE(0x900000, 0x903fff) AM_READWRITE(rockn_nvram_r, tetrisp2_nvram_w) AM_BASE(&tetrisp2_nvram) AM_SIZE(&tetrisp2_nvram_size)	// NVRAM
	AM_RANGE(0xa30000, 0xa30001) AM_READWRITE(rockn_soundvolume_r, rockn_soundvolume_w)			// Sound Volume
	AM_RANGE(0xa40000, 0xa40003) AM_DEVREADWRITE8("ymz", ymz280b_r, ymz280b_w, 0x00ff)	// Sound
	AM_RANGE(0xa44000, 0xa44001) AM_READWRITE(rockn_adpcmbank_r, rockn_adpcmbank_w)				// Sound Bank
	AM_RANGE(0xa48000, 0xa48001) AM_WRITENOP													// YMZ280 Reset
	AM_RANGE(0xa00000, 0xa00001) AM_WRITE(rocknms_main2sub_w)									// MAIN -> SUB Communication
	AM_RANGE(0xb00000, 0xb00001) AM_WRITE(tetrisp2_coincounter_w)								// Coin Counter
	AM_RANGE(0xb20000, 0xb20001) AM_WRITENOP													// ???
	AM_RANGE(0xb40000, 0xb4000b) AM_WRITEONLY AM_BASE(&tetrisp2_scroll_fg)						// Foreground Scrolling
	AM_RANGE(0xb40010, 0xb4001b) AM_WRITEONLY AM_BASE(&tetrisp2_scroll_bg)						// Background Scrolling
	AM_RANGE(0xb4003e, 0xb4003f) AM_WRITENOP													// scr_size
	AM_RANGE(0xb60000, 0xb6002f) AM_WRITEONLY AM_BASE(&tetrisp2_rotregs)						// Rotation Registers
	AM_RANGE(0xba0000, 0xba001f) AM_WRITE(rockn_systemregs_w)									// system param
	AM_RANGE(0xba001a, 0xba001b) AM_WRITENOP													// Lev 4 irq ack
	AM_RANGE(0xba001e, 0xba001f) AM_WRITENOP													// Lev 2 irq ack
	AM_RANGE(0xbe0000, 0xbe0001) AM_READNOP														// INT-level1 dummy read
	AM_RANGE(0xbe0002, 0xbe0003) AM_READ_PORT("PLAYERS")
	AM_RANGE(0xbe0004, 0xbe0005) AM_READ_PORT("SYSTEM")											// Inputs
	AM_RANGE(0xbe0008, 0xbe0009) AM_READ_PORT("DSW")											// Inputs
	AM_RANGE(0xbe000a, 0xbe000b) AM_READ(watchdog_reset16_r)									// Watchdog
ADDRESS_MAP_END


static ADDRESS_MAP_START( rocknms_sub_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM															// ROM
	AM_RANGE(0x100000, 0x103fff) AM_RAM AM_BASE_SIZE_GENERIC(spriteram2)		// Object RAM
	AM_RANGE(0x104000, 0x107fff) AM_RAM															// Spare Object RAM
	AM_RANGE(0x108000, 0x10ffff) AM_RAM															// Work RAM
	AM_RANGE(0x200000, 0x23ffff) AM_RAM_WRITE(rocknms_sub_priority_w) AM_BASE(&rocknms_sub_priority) // Priority
	AM_RANGE(0x300000, 0x31ffff) AM_RAM_WRITE(rocknms_sub_palette_w) AM_BASE_GENERIC(paletteram2)	// Palette
//  AM_RANGE(0x500000, 0x50ffff) AM_RAM                                                         // Line
	AM_RANGE(0x600000, 0x60ffff) AM_RAM_WRITE(rocknms_sub_vram_rot_w) AM_BASE(&rocknms_sub_vram_rot) // Rotation
	AM_RANGE(0x800000, 0x803fff) AM_RAM_WRITE(rocknms_sub_vram_fg_w) AM_BASE(&rocknms_sub_vram_fg) // Foreground
	AM_RANGE(0x804000, 0x807fff) AM_RAM_WRITE(rocknms_sub_vram_bg_w) AM_BASE(&rocknms_sub_vram_bg) // Background
//  AM_RANGE(0x808000, 0x809fff) AM_RAM                                                         // ???
	AM_RANGE(0x900000, 0x907fff) AM_RAM															// NVRAM
	AM_RANGE(0xa30000, 0xa30001) AM_WRITE(rockn_soundvolume_w)									// Sound Volume
	AM_RANGE(0xa40000, 0xa40003) AM_DEVWRITE8("ymz", ymz280b_w, 0x00ff)					// Sound
	AM_RANGE(0xa44000, 0xa44001) AM_WRITE(rockn_adpcmbank_w)									// Sound Bank
	AM_RANGE(0xa48000, 0xa48001) AM_WRITENOP													// YMZ280 Reset
	AM_RANGE(0xb00000, 0xb00001) AM_WRITE(rocknms_sub2main_w)									// MAIN <- SUB Communication
	AM_RANGE(0xb20000, 0xb20001) AM_WRITENOP													// ???
	AM_RANGE(0xb40000, 0xb4000b) AM_WRITEONLY AM_BASE(&rocknms_sub_scroll_fg)					// Foreground Scrolling
	AM_RANGE(0xb40010, 0xb4001b) AM_WRITEONLY AM_BASE(&rocknms_sub_scroll_bg)					// Background Scrolling
	AM_RANGE(0xb4003e, 0xb4003f) AM_WRITENOP													// scr_size
	AM_RANGE(0xb60000, 0xb6002f) AM_WRITEONLY AM_BASE(&rocknms_sub_rotregs)						// Rotation Registers
	AM_RANGE(0xba0000, 0xba001f) AM_WRITE(rocknms_sub_systemregs_w)								// system param
	AM_RANGE(0xba001a, 0xba001b) AM_WRITENOP													// Lev 4 irq ack
	AM_RANGE(0xba001e, 0xba001f) AM_WRITENOP													// Lev 2 irq ack
//  AM_RANGE(0xbe0000, 0xbe0001) AM_READNOP                                                     // INT-level1 dummy read
	AM_RANGE(0xbe0002, 0xbe0003) AM_READWRITE(rocknms_main2sub_r, rocknms_sub2main_w)			// MAIN <-> SUB Communication
	AM_RANGE(0xbe000a, 0xbe000b) AM_READ(watchdog_reset16_r	)									// Watchdog
ADDRESS_MAP_END


/***************************************************************************


                                Input Ports


***************************************************************************/

/***************************************************************************
                            Tetris Plus 2 (World)
***************************************************************************/

static INPUT_PORTS_START( tetrisp2 )

	PORT_START("PLAYERS") /*$be0002.w*/
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM") /*$be0004.w*/
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START2   )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_SPECIAL  )	/* ?*/
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_SPECIAL  )	/* ?*/
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW") //$be0008.w
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW1:8" )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0400, 0x0400, "Vs Mode Rounds" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0400, "3" )
	PORT_DIPNAME( 0x0800, 0x0000, DEF_STR( Language ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Japanese ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( English ) )
	PORT_DIPNAME( 0x1000, 0x1000, "F.B.I Logo" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, "Voice" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

INPUT_PORTS_END


/***************************************************************************
                            Tetris Plus 2 (Japan)

    The code for checking the "service mode" and "free play"
    DSWs is (deliberately?) bugged in this set

***************************************************************************/

static INPUT_PORTS_START( tetrisp2j )
	PORT_INCLUDE(tetrisp2)

	PORT_MODIFY("DSW")	// $be0008.w
	PORT_DIPNAME( 0x0040, 0x0040, "Unknown 1-7" ) PORT_DIPLOCATION("SW1:7") /* Free Play in "World" set */
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Unknown 1-8" ) PORT_DIPLOCATION("SW1:8") /* Test Mode in "World" set */
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0800, 0x0010, "SW2:4" ) /* Language Select in "World" set */
	PORT_DIPNAME( 0x1000, 0x1000, "Unknown 2-5" ) PORT_DIPLOCATION("SW2:5") /* F.B.I. Logo in "World" set */
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


/***************************************************************************
                            Nandemo Seal Iinkai
***************************************************************************/

static INPUT_PORTS_START( nndmseal )

	PORT_START("BUTTONS") // be0002.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON3 )	PORT_NAME( "OK" )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON4 )	PORT_NAME( "Cancel" )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 )	PORT_NAME( "-" )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON2 )	PORT_NAME( "+" )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("COINS") // be0004.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE2 )	// (keep pressed during boot for test mode)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW") // be0008.w
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Time?" )
	PORT_DIPSETTING(      0x0000, "35" )
	PORT_DIPSETTING(      0x0040, "45" )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0f00, 0x0f00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0x0f00, "0" )
	PORT_DIPSETTING(      0x0e00, "1" )
	PORT_DIPSETTING(      0x0d00, "2" )
	PORT_DIPSETTING(      0x0c00, "3" )
	PORT_DIPSETTING(      0x0b00, "4" )
	PORT_DIPSETTING(      0x0a00, "5" )
	PORT_DIPSETTING(      0x0900, "6" )
	PORT_DIPSETTING(      0x0800, "7" )
	PORT_DIPSETTING(      0x0700, "8" )
	PORT_DIPSETTING(      0x0600, "9" )
	PORT_DIPSETTING(      0x0500, "a" )
	PORT_DIPSETTING(      0x0400, "b" )
	PORT_DIPSETTING(      0x0300, "c" )
	PORT_DIPSETTING(      0x0200, "d" )
	PORT_DIPSETTING(      0x0100, "e" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE( 0x8000, IP_ACTIVE_LOW )

	PORT_START("PRINT") // be0006.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Print 1?")	// Press both to print (and alternate with ok too).
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2 ) PORT_NAME("Print 2?")	// Hold them for some seconds to bring up a "caution" message.
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH,IPT_SPECIAL )	// ?
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

INPUT_PORTS_END


/***************************************************************************
                            Rock'n Tread (Japan)
***************************************************************************/


static INPUT_PORTS_START( rockn )
	PORT_START("PLAYERS")	//$be0002.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")	//$be0004.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x0010, IP_ACTIVE_LOW )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")	//$be0008.w
	PORT_DIPNAME( 0x0001, 0x0001, "DIPSW 1-1") // All these used to be marked 'Cheat', can't think why.
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "DIPSW 1-2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "DIPSW 1-3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "DIPSW 1-4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "DIPSW 1-5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "DIPSW 1-6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "DIPSW 1-7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "DIPSW 1-8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPNAME(0x0100, 0x0100, "DIPSW 2-1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME(0x0200, 0x0200, "DIPSW 2-2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x0400, 0x0400, "DIPSW 2-3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x0800, 0x0800, "DIPSW 2-4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x1000, 0x1000, "DIPSW 2-5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x2000, 0x2000, "DIPSW 2-6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x4000, 0x4000, "DIPSW 2-7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x8000, 0x8000, "DIPSW 2-8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( rocknms )
	PORT_START("PLAYERS")	// IN0 - $be0002.w
	PORT_BIT( 0x0003, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(rocknms_main2sub_status_r, NULL)	// MAIN -> SUB Communication
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)

	PORT_START("SYSTEM")	// IN1 - $be0004.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START2 )
	PORT_SERVICE_NO_TOGGLE( 0x0010, IP_ACTIVE_LOW )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")	//$be0008.w
	PORT_DIPNAME( 0x0001, 0x0001, "DIPSW 1-1") // All these used to be marked 'Cheat', can't think why.
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "DIPSW 1-2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "DIPSW 1-3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "DIPSW 1-4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "DIPSW 1-5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "DIPSW 1-6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "DIPSW 1-7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "DIPSW 1-8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPNAME(0x0100, 0x0100, "DIPSW 2-1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME(0x0200, 0x0200, "DIPSW 2-2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x0400, 0x0400, "DIPSW 2-3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x0800, 0x0800, "DIPSW 2-4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x1000, 0x1000, "DIPSW 2-5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x2000, 0x2000, "DIPSW 2-6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x4000, 0x4000, "DIPSW 2-7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME(    0x8000, 0x8000, "DIPSW 2-8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END



/***************************************************************************


                            Graphics Layouts


***************************************************************************/


/* 8x8x8 tiles */
static const gfx_layout layout_8x8x8 =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	{ STEP8(0,8*8) },
	8*8*8
};

/* 16x16x8 tiles */
static const gfx_layout layout_16x16x8 =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ STEP8(0,1) },
	{ STEP16(0,8) },
	{ STEP16(0,16*8) },
	16*16*8
};


/* sprites are contained in 256x256 "tiles" */
static GFXLAYOUT_RAW( spritelayout, 8, 256, 256, 256*8, 256*256*8 )


static GFXDECODE_START( tetrisp2 )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout,   0x0000, 0x10 ) // [0] Sprites
	GFXDECODE_ENTRY( "gfx2", 0, layout_16x16x8, 0x1000, 0x10 ) // [1] Background
	GFXDECODE_ENTRY( "gfx3", 0, layout_16x16x8, 0x2000, 0x10 ) // [2] Rotation
	GFXDECODE_ENTRY( "gfx4", 0, layout_8x8x8,   0x6000, 0x10 ) // [3] Foreground
GFXDECODE_END

static GFXDECODE_START( rocknms )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout,   0x0000, 0x10 ) // [0] Sprites
	GFXDECODE_ENTRY( "gfx2", 0, layout_16x16x8, 0x1000, 0x10 ) // [1] Background
	GFXDECODE_ENTRY( "gfx3", 0, layout_16x16x8, 0x2000, 0x10 ) // [2] Rotation
	GFXDECODE_ENTRY( "gfx4", 0, layout_8x8x8,   0x6000, 0x10 ) // [3] Foreground
	GFXDECODE_ENTRY( "gfx5", 0, spritelayout,   0x8000, 0x10 ) // [0] Sprites
	GFXDECODE_ENTRY( "gfx6", 0, layout_16x16x8, 0x9000, 0x10 ) // [1] Background
	GFXDECODE_ENTRY( "gfx7", 0, layout_16x16x8, 0xa000, 0x10 ) // [2] Rotation
	GFXDECODE_ENTRY( "gfx8", 0, layout_8x8x8,   0xe000, 0x10 ) // [3] Foreground
GFXDECODE_END


/***************************************************************************


                                Machine Drivers


***************************************************************************/

static TIMER_CALLBACK( rockn_timer_level4_callback )
{
	cputag_set_input_line(machine, "maincpu", 4, HOLD_LINE);
}

static TIMER_CALLBACK( rockn_timer_sub_level4_callback )
{
	cputag_set_input_line(machine, "sub", 4, HOLD_LINE);
}


static TIMER_CALLBACK( rockn_timer_level1_callback )
{
	cputag_set_input_line(machine, "maincpu", 1, HOLD_LINE);
}

static TIMER_CALLBACK( rockn_timer_sub_level1_callback )
{
	cputag_set_input_line(machine, "sub", 1, HOLD_LINE);
}

static void init_rockn_timer(running_machine *machine)
{
	timer_pulse(machine, ATTOTIME_IN_MSEC(32), NULL, 0, rockn_timer_level1_callback);
	rockn_timer_l4 = timer_alloc(machine, rockn_timer_level4_callback, NULL);

	state_save_register_global_array(machine, tetrisp2_systemregs);
	state_save_register_global_array(machine, rocknms_sub_systemregs);
	state_save_register_global(machine, rockn_protectdata);
	state_save_register_global(machine, rockn_adpcmbank);
	state_save_register_global(machine, rockn_soundvolume);
}

static DRIVER_INIT( rockn )
{
	init_rockn_timer(machine);
	rockn_protectdata = 1;
}

static DRIVER_INIT( rockn1 )
{
	init_rockn_timer(machine);
	rockn_protectdata = 1;
}

static DRIVER_INIT( rockn2 )
{
	init_rockn_timer(machine);
	rockn_protectdata = 2;
}

static DRIVER_INIT( rocknms )
{
	init_rockn_timer(machine);

	timer_pulse(machine, ATTOTIME_IN_MSEC(32), NULL, 0, rockn_timer_sub_level1_callback);
	rockn_timer_sub_l4 = timer_alloc(machine, rockn_timer_sub_level4_callback, NULL);

	rockn_protectdata = 3;

}

static DRIVER_INIT( rockn3 )
{
	init_rockn_timer(machine);
	rockn_protectdata = 4;
}


static MACHINE_DRIVER_START( tetrisp2 )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 12000000)
	MDRV_CPU_PROGRAM_MAP(tetrisp2_map)
	MDRV_CPU_VBLANK_INT("screen", irq2_line_hold)

	MDRV_NVRAM_HANDLER(tetrisp2)
	MDRV_WATCHDOG_VBLANK_INIT(8)	/* guess */

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(0x140, 0xe0)
	MDRV_SCREEN_VISIBLE_AREA(0, 0x140-1, 0, 0xe0-1)

	MDRV_GFXDECODE(tetrisp2)
	MDRV_PALETTE_LENGTH(0x8000)

	MDRV_VIDEO_START(tetrisp2)
	MDRV_VIDEO_UPDATE(tetrisp2)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymz", YMZ280B, 16934400)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( nndmseal )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, XTAL_12MHz)
	MDRV_CPU_PROGRAM_MAP(nndmseal_map)
	MDRV_CPU_VBLANK_INT("screen", irq2_line_hold)

	MDRV_NVRAM_HANDLER(tetrisp2)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(0x180, 0xf0)
	MDRV_SCREEN_VISIBLE_AREA(0, 0x180-1, 0, 0xf0-1)

	MDRV_GFXDECODE(tetrisp2)
	MDRV_PALETTE_LENGTH(0x8000)

	MDRV_VIDEO_START(nndmseal)	// bg layer offset
	MDRV_VIDEO_UPDATE(tetrisp2)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_OKIM6295_ADD("oki", XTAL_2MHz, OKIM6295_PIN7_HIGH)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( rockn )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 12000000)
	MDRV_CPU_PROGRAM_MAP(rockn1_map)
	MDRV_CPU_VBLANK_INT("screen", irq2_line_hold)

	MDRV_NVRAM_HANDLER(tetrisp2)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(0x140, 0xe0)
	MDRV_SCREEN_VISIBLE_AREA(0, 0x140-1, 0, 0xe0-1)

	MDRV_GFXDECODE(tetrisp2)
	MDRV_PALETTE_LENGTH(0x8000)

	MDRV_VIDEO_START(rockntread)
	MDRV_VIDEO_UPDATE(rockntread)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymz", YMZ280B, 16934400)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( rockn2 )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 12000000)
	MDRV_CPU_PROGRAM_MAP(rockn2_map)
	MDRV_CPU_VBLANK_INT("screen", irq2_line_hold)

	MDRV_NVRAM_HANDLER(tetrisp2)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(0x140, 0xe0)
	MDRV_SCREEN_VISIBLE_AREA(0, 0x140-1, 0, 0xe0-1)

	MDRV_GFXDECODE(tetrisp2)
	MDRV_PALETTE_LENGTH(0x8000)

	MDRV_VIDEO_START(rockntread)
	MDRV_VIDEO_UPDATE(rockntread)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymz", YMZ280B, 16934400)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( rocknms )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 12000000)
	MDRV_CPU_PROGRAM_MAP(rocknms_main_map)
	MDRV_CPU_VBLANK_INT("lscreen", irq2_line_hold)

	MDRV_CPU_ADD("sub", M68000, 12000000)
	MDRV_CPU_PROGRAM_MAP(rocknms_sub_map)
	MDRV_CPU_VBLANK_INT("lscreen", irq2_line_hold)

	MDRV_NVRAM_HANDLER(tetrisp2)

	/* video hardware */

	MDRV_GFXDECODE(rocknms)
	MDRV_PALETTE_LENGTH(0x10000)
	MDRV_DEFAULT_LAYOUT(layout_rocknms)

	MDRV_SCREEN_ADD("lscreen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_SIZE(0x140, 0xe0)
	MDRV_SCREEN_VISIBLE_AREA(0, 0x140-1, 0, 0xe0-1)

	MDRV_SCREEN_ADD("rscreen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_SIZE(0x140, 0xe0)
	MDRV_SCREEN_VISIBLE_AREA(0, 0x140-1, 0, 0xe0-1)


	MDRV_VIDEO_START(rocknms)
	MDRV_VIDEO_UPDATE(rocknms)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymz", YMZ280B, 16934400)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_DRIVER_END


/***************************************************************************


                                ROMs Loading


***************************************************************************/


/***************************************************************************

                            Tetris Plus 2 (World & Japan sets)

(c)1997 Jaleco / The Tetris Company

TP-97222
96019 EB-00-20117-0
MDK332V-0

BRIEF HARDWARE OVERVIEW

CPU:    Toshiba TMP68HC000P-12
Sound:  Yamaha YMZ280B-F & Yamaha  YAC516-M
OSC:    12.000MHz
        48.0000MHz
        16.9344MHz
DIPS:   Two 8-way dipswitches

Listing of custom chips:

IC38    JALECO SS91022-03 9428XX001
IC31    JALECO SS91022-05 9347EX002
IC32    JALECO GS91022-05    048  9726HX002
IC30    JALECO GS91022-04 9721PD008
IC39    XILINX XC5210 PQ240C X68710M AKJ9544
IC49    XILINX XC7336 PC44ACK9633 A63458A

World set:
  Supports English or Japanese language
  Optional showing of US FBI Logo
  Full Test Mode via dipswitch
  EEPROM used

Japan Set:
  Japanese language only
  No optional FBI Logo
  No way to enter a Test Mode
  No support for EEPROM

***************************************************************************/

ROM_START( tetrisp2 ) /* Version 2.8 */
	ROM_REGION( 0x100000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "tet2_4_ver2.8.ic65", 0x000000, 0x080000, CRC(e67f9c51) SHA1(d8b2937699d648267b163c7c3f591426877f3701) )
	ROM_LOAD16_BYTE( "tet2_1_ver2.8.ic59", 0x000001, 0x080000, CRC(5020a4ed) SHA1(9c0f02fe3700761771ac026a2e375144e86e5eb7) )

	ROM_REGION( 0x800000, "gfx1", 0 )	/* 8x8x8 (Sprites) */
	ROM_LOAD32_WORD( "96019-01.9", 0x000000, 0x400000, CRC(06f7dc64) SHA1(722c51b707b9854c0293afdff18b27ec7cae6719) )
	ROM_LOAD32_WORD( "96019-02.8", 0x000002, 0x400000, CRC(3e613bed) SHA1(038b5e43fa3d69654107c8093126eeb2e8fa4ddc) )

	/* If t2p_m01&2 from this board were correctly read, since they hold the same data of the above but with swapped halves, it
           means they had to invert the top bit of the "page select" register in the sprite's hardware on this board! */

	ROM_REGION( 0x800000, "gfx2", 0 )	/* 16x16x8 (Background) */
	ROM_LOAD( "96019-06.13", 0x000000, 0x400000, CRC(16f7093c) SHA1(2be77c6a692c5d762f5553ae24e8c415ab194cc6) )
	ROM_LOAD( "96019-04.6",  0x400000, 0x100000, CRC(b849dec9) SHA1(fa7ac00fbe587a74c3fb8c74a0f91f7afeb8682f) )

	ROM_REGION( 0x100000, "gfx3", 0 )	/* 16x16x8 (Rotation) */
	ROM_LOAD( "96019-04.6",  0x000000, 0x100000, CRC(b849dec9) SHA1(fa7ac00fbe587a74c3fb8c74a0f91f7afeb8682f) )

	ROM_REGION( 0x080000, "gfx4", 0 )	/* 8x8x8 (Foreground) */
	ROM_LOAD( "tetp2-10.ic27", 0x000000, 0x080000, CRC(34dd1bad) SHA1(9bdf1dde11f82839676400de5dd7acb06ea8cdb2) )	// 11111xxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x400000, "ymz", 0 )	/* Samples */
	ROM_LOAD( "96019-07.7", 0x000000, 0x400000, CRC(a8a61954) SHA1(86c3db10b348ba1f44ff696877b8b20845fa53de) )
ROM_END

ROM_START( tetrisp2j ) /* Version 2.2 */
	ROM_REGION( 0x100000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "tet2_4_ver2.2.ic65", 0x000000, 0x080000, CRC(5bfa32c8) SHA1(55fb2872695fcfbad13f5c0723302e72da69e44a) )
	ROM_LOAD16_BYTE( "tet2_1_ver2.2.ic59", 0x000001, 0x080000, CRC(919116d0) SHA1(3e1c0fd4c9175b2900a4717fbb9e8b591c5f534d) )

	ROM_REGION( 0x800000, "gfx1", 0 )	/* 8x8x8 (Sprites) */
	ROM_LOAD32_WORD( "96019-01.9", 0x000000, 0x400000, CRC(06f7dc64) SHA1(722c51b707b9854c0293afdff18b27ec7cae6719) )
	ROM_LOAD32_WORD( "96019-02.8", 0x000002, 0x400000, CRC(3e613bed) SHA1(038b5e43fa3d69654107c8093126eeb2e8fa4ddc) )

	ROM_REGION( 0x400000, "gfx2", 0 )	/* 16x16x8 (Background) */
	ROM_LOAD( "96019-06.13", 0x000000, 0x400000, CRC(16f7093c) SHA1(2be77c6a692c5d762f5553ae24e8c415ab194cc6) )

	ROM_REGION( 0x100000, "gfx3", 0 )	/* 16x16x8 (Rotation) */
	ROM_LOAD( "96019-04.6",  0x000000, 0x100000, CRC(b849dec9) SHA1(fa7ac00fbe587a74c3fb8c74a0f91f7afeb8682f) )

	ROM_REGION( 0x080000, "gfx4", 0 )	/* 8x8x8 (Foreground) */
	ROM_LOAD( "tetp2-10.ic27", 0x000000, 0x080000, CRC(34dd1bad) SHA1(9bdf1dde11f82839676400de5dd7acb06ea8cdb2) )	// 11111xxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x400000, "ymz", 0 )	/* Samples */
	ROM_LOAD( "96019-07.7", 0x000000, 0x400000, CRC(a8a61954) SHA1(86c3db10b348ba1f44ff696877b8b20845fa53de) )
ROM_END

/***************************************************************************

Nandemo Seal Iinkai
(c)1996 I'max/Jaleco

Mainboard:
NS-96205 9601
EB-00-20110-0

Daughterboard:
NS-96206A
96017
EB-00-20111-1


CPU: TMP68000P-12

Sound: M6295 (on daughterboard)

OSC: 12.000MHz (X1)
     42.9545MHz(OSC1)
     2.000MHz(X1 on daughterboard. silk print says 4MHz)

Custom chips:
GS91022-04
GS91022-05
SS91022-03
SS91022-05

PLDs:
PS96017-01 (XILINX XC7336)
96017-02 (18CV8P)
96017-03 (18CV8P)
96017-04 (18CV8P on daughterboard)

Others:
CR2032 battery
Centronics printer port

ROMs:(all ROMs are on daughterboard)
1.1 - Programs (TMS 27C020)
3.3 /
(actual label is "Cawaii 1 Ver1.1" & "Cawaii 3 Ver1.1")

MR97006-01.2 (42pin mask ROM, read as 16M, byte mode)
MR97006-02.5 (same as above)
MR97001-01.6 (same as above)
MR96017-04.9 (same as above, samples)

MR97006-04.8 (32pin mask ROM, read as 8M)


dumped by sayu
--- Team Japump!!! ---

***************************************************************************/

ROM_START( nndmseal )
	ROM_REGION( 0x80000, "maincpu", 0 )		// 68000 Code
	ROM_LOAD16_BYTE( "1.1", 0x00000, 0x40000, CRC(45acea25) SHA1(f2f2e78be261c3d8c0145a639bc3771f0588401d) )	// 1xxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "3.3", 0x00001, 0x40000, CRC(0754d96a) SHA1(1da44994e8bcfd8832755e298c0125b38cfdd16e) )	// 1xxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x100000, "gfx1", ROMREGION_ERASE )    /* 8x8x8 (Sprites) */
/* This game doesn't use sprites, but the region needs to be a valid size for at least one sprite 'page' for the init to work. */

	ROM_REGION( 0x400000, "gfx2", 0 )	// 16x16x8 (Background)
	ROM_LOAD( "mr97006-02.5", 0x000000, 0x200000, CRC(4793f84e) SHA1(05acba6cc8a527a6050af79a460b08c4676287aa) )
	ROM_LOAD( "mr97001-01.6", 0x200000, 0x200000, CRC(dd648e8a) SHA1(7036ab30d0ea179c59d74c1fbe4372968722ec0f) )

	ROM_REGION( 0x200000, "gfx3", 0 )	// 16x16x8 (Rotation)
	ROM_LOAD( "mr97006-01.2", 0x000000, 0x200000, CRC(32283485) SHA1(14ccd25389b97825d9a727809c3a1de803687c16) )

	ROM_REGION( 0x100000, "gfx4", 0 )	// 8x8x8 (Foreground)
	ROM_LOAD( "mr97006-04.8", 0x000000, 0x100000, CRC(6726a25b) SHA1(4ea49c014477229eaf9de4a0b9bf83021b82c095) )

	ROM_REGION( 0x40000, "oki", ROMREGION_ERASE )	// Samples
	// filled in from "okisource"

	ROM_REGION( 0x200000, "okisource", 0 )	// Samples
	ROM_LOAD( "mr96017-04.9", 0x000000, 0x200000, CRC(c2e7b444) SHA1(e2b9d3d94720d01beff1108ef3dfbff805ddd1fd) )
ROM_END

/***************************************************************************

                            Rock'n Tread 1 (Japan)
                            Rock'n Tread 2 (Japan)
                            Rock'n MegaSession (Japan)
                            Rock'n 3 (Japan)
                            Rock'n 4 (Japan)

(c)1999-2000 Jaleco

Main board marked in copper and silk screen:

    JALECO VJ-98344
    98053 EB-00-20120-0
    MADE IN JAPAN

CPU:    TMP68HC000P-12
Sound:  YMZ280B-F / YAC516-M (on sound rom board)
OSC:    12.0000MHz (next to 68000)
        48.0000MHz
        16.9344MHz (on sound rom board)
DIPS:   Two 8-way dipswitches
BAT:    CR-2032

Custom: SS91022-03
        GS91022-04
        GS91022-05
        SS91022-05

Other:  Sigma XILINX XCS30
        Sigma XILINX XC9536 (socketted and stamped SL4)


PCB Layout for sound rom board (from Rock'n 3):

JALECO 99004 SL-99352 EB-00-20128-0
+------------------------------------------------------+
|                                                      |
| mr99029-02 mr99029-10 mr99029-18 mr99029-01  IC36*   |
| mr99029-03 mr99029-11 mr99029-19  IC29*      IC37*   |
| mr99029-04 mr99029-12 mr99029-20                     |
| mr99029-05 mr99029-13 mr99029-21                     |
| mr99029-06 mr99029-14  IC23*        YMZ280B-F        |
| mr99029-07 mr99029-15  IC24*      16.9344MHz         |
| mr99029-08 mr99029-16  IC25*        XC9572           |
| mr99029-09 mr99029-17  IC26*                         |
|                       YAC516-M                       |
|   CN1        CN2                       CN4           |
+-||||||||---||||||||----------------------------------+

Notes:
  All chip sockets are marked as 27C3200 (read as 27C322)
  Chips are OKI M72C3252C2 mounted on Jaleco PCB EB-00-40051-0 42 pin converters

* Unpopulated sockets

Sound chips: Yamaha YMZ280B-F & Yamaha YAC516-M
 Other chip: Sigma XILINX XC9572
        OSC: 16.9344MHz

  CN1 - 8 pin header
  CN2 - 8 pin header
  CN4 - 34 pin dual row ribbon connection

***************************************************************************/

/***  Rock 'n' Tread  ***/

ROM_START( rockn )
	ROM_REGION( 0x100000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "rock_n_1_vj-98344_1.bin", 0x000001, 0x80000, CRC(4cf79e58) SHA1(f50e596d43c9ab2072ae0476169eee2a8512fd8d) )
	ROM_LOAD16_BYTE( "rock_n_1_vj-98344_4.bin", 0x000000, 0x80000, CRC(caa33f79) SHA1(8ccff67091dac5ad871cae6cdb31e1fc37c1a4c2) )

	ROM_REGION( 0x400000, "gfx1", 0 )	/* 8x8x8 (Sprites) */
	ROM_LOAD32_WORD( "rock_n_1_vj-98344_8.bin", 0x000002, 0x200000, CRC(fa3f6f9c) SHA1(586dcc690a1a4aa7c97932ad496382def6a074a4) )
	ROM_LOAD32_WORD( "rock_n_1_vj-98344_9.bin", 0x000000, 0x200000, CRC(3d12a688) SHA1(356b2ea81d960838b604c5a17cc77e79fb0e40ce) )

	ROM_REGION( 0x200000, "gfx2", 0 )	/* 16x16x8 (Background) */
	ROM_LOAD16_WORD( "rock_n_1_vj-98344_13.bin", 0x000000, 0x200000, CRC(261b99a0) SHA1(7b3c768ae9d7429e2559fe32c1a4ff220d727e7e) )

	ROM_REGION( 0x100000, "gfx3", 0 )	/* 16x16x8 (Rotation) */
	ROM_LOAD( "rock_n_1_vj-98344_6.bin", 0x000000, 0x100000, CRC(5551717f) SHA1(64943a9a68ad4074f3f5128d7796e4f03baa14d5) )

	ROM_REGION( 0x080000, "gfx4", 0 )	/* 8x8x8 (Foreground) */
	ROM_LOAD( "rock_n_1_vj-98344_10.bin", 0x000000, 0x080000, CRC(918663a8) SHA1(aedacb741c986ef8159385cfef866cb7e3ef6cb6) )

	/* from the bootleg set, are they right for this? */
	ROM_REGION( 0x7000000, "ymz", 0 )	/* Samples */
	ROM_LOAD( "sound00", 0x0000000, 0x0400000, CRC(c354f753) SHA1(bf538c02e2162a93d8c6793a1211e21480156223)  ) // COMMON AREA
	ROM_FILL(                 0x0400000, 0x0c00000, 0xff ) // BANK AREA
	ROM_LOAD( "sound01", 0x1000000, 0x0400000, CRC(5b42999e) SHA1(376c773f292eae8b75db11bad3cb6ec5fe48392e)  ) // bank 0
	ROM_LOAD( "sound02", 0x1400000, 0x0400000, CRC(8306f302) SHA1(8c0437d7ab8d74d4d15f4a641d30602e39cdd99d)  ) // bank 0
	ROM_LOAD( "sound03", 0x1800000, 0x0400000, CRC(3fda842c) SHA1(2b9e7c548b689bab491237e36a2dcf4782a81d79)  ) // bank 0
	ROM_LOAD( "sound04", 0x1c00000, 0x0400000, CRC(86d4f289) SHA1(908490ab0cf8d33cf3e127f71edee3bece70b86d)  ) // bank 1
	ROM_LOAD( "sound05", 0x2000000, 0x0400000, CRC(f8dbf47d) SHA1(f19f7ae26e3b8af17a4e66e6722dd2f5c36d33f8)  ) // bank 1
	ROM_LOAD( "sound06", 0x2400000, 0x0400000, CRC(525aff97) SHA1(b18e5bdf67d3a89f39c59f4f9bd3bb608dacc7f7)  ) // bank 1
	ROM_LOAD( "sound07", 0x2800000, 0x0400000, CRC(5bd8bb95) SHA1(3b33c42778f7d50ca1513d37e7bc4a4efcc3cf82)  ) // bank 2
	ROM_LOAD( "sound08", 0x2c00000, 0x0400000, CRC(304c1643) SHA1(0be090077e00d4b9abce2fac4821c630b6a40f22)  ) // bank 2
	ROM_LOAD( "sound09", 0x3000000, 0x0400000, CRC(78c22c56) SHA1(eb48d188d25538a1d381ca760f8e98096ee12bfe)  ) // bank 2
	ROM_LOAD( "sound10", 0x3400000, 0x0400000, CRC(d5e8d8a5) SHA1(df7db3c8b110ce1aa85e627537afb744c98877bd)  ) // bank 3
	ROM_LOAD( "sound11", 0x3800000, 0x0400000, CRC(569ef4dd) SHA1(777f8a3aef741655555364d00a1eaa472ac4b922)  ) // bank 3
	ROM_LOAD( "sound12", 0x3c00000, 0x0400000, CRC(aae8d59c) SHA1(ccca1f511ce0ea8d452f3b1d24350b5cee402ad2)  ) // bank 3
	ROM_LOAD( "sound13", 0x4000000, 0x0400000, CRC(9ec1459b) SHA1(10e08a47636dec431cdb8e105cf61287fe9c6637)  ) // bank 4
	ROM_LOAD( "sound14", 0x4400000, 0x0400000, CRC(b26f9a81) SHA1(0d1c8e382eb5877f9a748ff289be97cbdb73b0cc)  ) // bank 4
ROM_END

ROM_START( rockna )
	ROM_REGION( 0x100000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "rock_n_1_vj-98344_1", 0x000001, 0x80000, CRC(6078fa48) SHA1(e98c1a1abf026f2d5b5035ccbc9d412a08ca1f02) )
	ROM_LOAD16_BYTE( "rock_n_1_vj-98344_4", 0x000000, 0x80000, CRC(c8310bd0) SHA1(1efee954cc94b668b7d9f28a099b8d1c83d3093f) )

	ROM_REGION( 0x400000, "gfx1", 0 )	/* 8x8x8 (Sprites) */
	ROM_LOAD32_WORD( "rock_n_1_vj-98344_8.bin", 0x000002, 0x200000, CRC(fa3f6f9c) SHA1(586dcc690a1a4aa7c97932ad496382def6a074a4) )
	ROM_LOAD32_WORD( "rock_n_1_vj-98344_9.bin", 0x000000, 0x200000, CRC(3d12a688) SHA1(356b2ea81d960838b604c5a17cc77e79fb0e40ce) )

	ROM_REGION( 0x200000, "gfx2", 0 )	/* 16x16x8 (Background) */
	ROM_LOAD16_WORD( "rock_n_1_vj-98344_13.bin", 0x000000, 0x200000, CRC(261b99a0) SHA1(7b3c768ae9d7429e2559fe32c1a4ff220d727e7e) )

	ROM_REGION( 0x100000, "gfx3", 0 )	/* 16x16x8 (Rotation) */
	ROM_LOAD( "rock_n_1_vj-98344_6.bin", 0x000000, 0x100000, CRC(5551717f) SHA1(64943a9a68ad4074f3f5128d7796e4f03baa14d5) )

	ROM_REGION( 0x080000, "gfx4", 0 )	/* 8x8x8 (Foreground) */
	ROM_LOAD( "rock_n_1_vj-98344_10.bin", 0x000000, 0x080000, CRC(918663a8) SHA1(aedacb741c986ef8159385cfef866cb7e3ef6cb6) )

	ROM_REGION( 0x7000000, "ymz", 0 )	/* Samples */
	ROM_LOAD( "sound00", 0x0000000, 0x0400000, CRC(c354f753) SHA1(bf538c02e2162a93d8c6793a1211e21480156223)  ) // COMMON AREA
	ROM_FILL(                 0x0400000, 0x0c00000, 0xff ) // BANK AREA
	ROM_LOAD( "sound01", 0x1000000, 0x0400000, CRC(5b42999e) SHA1(376c773f292eae8b75db11bad3cb6ec5fe48392e)  ) // bank 0
	ROM_LOAD( "sound02", 0x1400000, 0x0400000, CRC(8306f302) SHA1(8c0437d7ab8d74d4d15f4a641d30602e39cdd99d)  ) // bank 0
	ROM_LOAD( "sound03", 0x1800000, 0x0400000, CRC(3fda842c) SHA1(2b9e7c548b689bab491237e36a2dcf4782a81d79)  ) // bank 0
	ROM_LOAD( "sound04", 0x1c00000, 0x0400000, CRC(86d4f289) SHA1(908490ab0cf8d33cf3e127f71edee3bece70b86d)  ) // bank 1
	ROM_LOAD( "sound05", 0x2000000, 0x0400000, CRC(f8dbf47d) SHA1(f19f7ae26e3b8af17a4e66e6722dd2f5c36d33f8)  ) // bank 1
	ROM_LOAD( "sound06", 0x2400000, 0x0400000, CRC(525aff97) SHA1(b18e5bdf67d3a89f39c59f4f9bd3bb608dacc7f7)  ) // bank 1
	ROM_LOAD( "sound07", 0x2800000, 0x0400000, CRC(5bd8bb95) SHA1(3b33c42778f7d50ca1513d37e7bc4a4efcc3cf82)  ) // bank 2
	ROM_LOAD( "sound08", 0x2c00000, 0x0400000, CRC(304c1643) SHA1(0be090077e00d4b9abce2fac4821c630b6a40f22)  ) // bank 2
	ROM_LOAD( "sound09", 0x3000000, 0x0400000, CRC(78c22c56) SHA1(eb48d188d25538a1d381ca760f8e98096ee12bfe)  ) // bank 2
	ROM_LOAD( "sound10", 0x3400000, 0x0400000, CRC(d5e8d8a5) SHA1(df7db3c8b110ce1aa85e627537afb744c98877bd)  ) // bank 3
	ROM_LOAD( "sound11", 0x3800000, 0x0400000, CRC(569ef4dd) SHA1(777f8a3aef741655555364d00a1eaa472ac4b922)  ) // bank 3
	ROM_LOAD( "sound12", 0x3c00000, 0x0400000, CRC(aae8d59c) SHA1(ccca1f511ce0ea8d452f3b1d24350b5cee402ad2)  ) // bank 3
	ROM_LOAD( "sound13", 0x4000000, 0x0400000, CRC(9ec1459b) SHA1(10e08a47636dec431cdb8e105cf61287fe9c6637)  ) // bank 4
	ROM_LOAD( "sound14", 0x4400000, 0x0400000, CRC(b26f9a81) SHA1(0d1c8e382eb5877f9a748ff289be97cbdb73b0cc)  ) // bank 4
ROM_END

ROM_START( rockn2 )
	ROM_REGION( 0x100000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "rock_n_2_vj-98344_1_v1.0", 0x000001, 0x80000, CRC(854b5a45) SHA1(91496bc511fef1d552d2bd00b82d2470eae94528) )
	ROM_LOAD16_BYTE( "rock_n_2_vj-98344_4_v1.0", 0x000000, 0x80000, CRC(4665bbd2) SHA1(3562c67b81a32d178a8bcb872e676bf7284855d7) )

	ROM_REGION( 0x400000, "gfx1", 0 )	/* 8x8x8 (Sprites) */
	ROM_LOAD32_WORD( "rock_n_2_vj-98344_8_v1.0", 0x000002, 0x200000, CRC(673ce2c2) SHA1(6c0a13de386b02a7f3a86e8128374938ede2525c) )
	ROM_LOAD32_WORD( "rock_n_2_vj-98344_9_v1.0", 0x000000, 0x200000, CRC(9d3968cf) SHA1(11c96e7685ab8c1b416396238ec5c12e7819385f) )

	ROM_REGION( 0x200000, "gfx2", 0 )	/* 16x16x8 (Background) */
	ROM_LOAD16_WORD( "rock_n_2_vj-98344_13_v1.0", 0x000000, 0x200000, CRC(e35c55b3) SHA1(a18367c28befc3f71823f1d4ab2126ad6f8a28fc)  )

	ROM_REGION( 0x200000, "gfx3", 0 )	/* 16x16x8 (Rotation) */
	ROM_LOAD( "rock_n_2_vj-98344_6_v1.0", 0x000000, 0x200000, CRC(241d7449) SHA1(9fcc2d128d7be273836460313c0e73c81e33c9cb)  )

	ROM_REGION( 0x080000, "gfx4", 0 )	/* 8x8x8 (Foreground) */
	ROM_LOAD( "rock_n_2_vj-98344_10_v1.0", 0x000000, 0x080000, CRC(ae74d5b3) SHA1(07aa6ee540a783e3f2a8710a7095d922cff1d443)  )

	ROM_REGION( 0x7000000, "ymz", 0 )	/* Samples */
	ROM_LOAD( "sound00", 0x0000000, 0x0400000, CRC(4e9611a3) SHA1(2a9b1d5afc0ea9a3285f9fc6b49a1c3abd8cd2a5)  ) // COMMON AREA
	ROM_FILL(            0x0400000, 0x0c00000, 0xff )		// BANK AREA
	ROM_LOAD( "sound01", 0x1000000, 0x0400000, CRC(ec600f13) SHA1(151cb0a16782c8bba223d0f6881b80c1e43bc9bc)  ) // bank 0
	ROM_LOAD( "sound02", 0x1400000, 0x0400000, CRC(8306f302) SHA1(8c0437d7ab8d74d4d15f4a641d30602e39cdd99d)  ) // bank 0
	ROM_LOAD( "sound03", 0x1800000, 0x0400000, CRC(3fda842c) SHA1(2b9e7c548b689bab491237e36a2dcf4782a81d79)  ) // bank 0
	ROM_LOAD( "sound04", 0x1c00000, 0x0400000, CRC(86d4f289) SHA1(908490ab0cf8d33cf3e127f71edee3bece70b86d)  ) // bank 1
	ROM_LOAD( "sound05", 0x2000000, 0x0400000, CRC(f8dbf47d) SHA1(f19f7ae26e3b8af17a4e66e6722dd2f5c36d33f8)  ) // bank 1
	ROM_LOAD( "sound06", 0x2400000, 0x0400000, CRC(06f7bd63) SHA1(d8b27212ebba99f5129483550aeac5b86ff2a1d2)  ) // bank 1
	ROM_LOAD( "sound07", 0x2800000, 0x0400000, CRC(22f042f6) SHA1(649bdf43dd698150992e68b23fd758bca56c615b)  ) // bank 2
	ROM_LOAD( "sound08", 0x2c00000, 0x0400000, CRC(dd294d8e) SHA1(49d889d341ab6167d9741340eb27902923b6cb42)  ) // bank 2
	ROM_LOAD( "sound09", 0x3000000, 0x0400000, CRC(8fedee6e) SHA1(540c01d1c5f410abb1f86f33a5a532208946cb7c)  ) // bank 2
	ROM_LOAD( "sound10", 0x3400000, 0x0400000, CRC(01292f11) SHA1(da88b14bf8df34e7574cf8c9f5dd385db13ab34c)  ) // bank 3
	ROM_LOAD( "sound11", 0x3800000, 0x0400000, CRC(20dc76ba) SHA1(078397f2de54d4ca91035dce11419ac0d934fbfa)  ) // bank 3
	ROM_LOAD( "sound12", 0x3c00000, 0x0400000, CRC(11fff0bc) SHA1(2767fcc3a5d3200750b011c97a83073719a9325f)  ) // bank 3
	ROM_LOAD( "sound13", 0x4000000, 0x0400000, CRC(2367dd18) SHA1(b58f757ce4c832c5462637f4e08d7be511ca0c96)  ) // bank 4
	ROM_LOAD( "sound14", 0x4400000, 0x0400000, CRC(75ced8c0) SHA1(fda17464767be073a36c117f5212411b66197dd9)  ) // bank 4
	ROM_LOAD( "sound15", 0x4800000, 0x0400000, CRC(aeaca380) SHA1(1c389911aa766abec389b1c79a1542759ac58b9f)  ) // bank 4
	ROM_LOAD( "sound16", 0x4c00000, 0x0400000, CRC(21d50e32) SHA1(24eaceb7c0b868b6e8fc16b403dae2427e422bf6)  ) // bank 5
	ROM_LOAD( "sound17", 0x5000000, 0x0400000, CRC(de785a2a) SHA1(1f5ae46ac9476a31a431ce0f0cf124e1c8c930a6)  ) // bank 5
	ROM_LOAD( "sound18", 0x5400000, 0x0400000, CRC(18cabb1e) SHA1(c769820e2e84eff0e4ce956236656ae757e3299c)  ) // bank 5
	ROM_LOAD( "sound19", 0x5800000, 0x0400000, CRC(33c89e53) SHA1(7d216f5db6b30c9b05a9a77030498ff68ae6fbad)  ) // bank 6
	ROM_LOAD( "sound20", 0x5c00000, 0x0400000, CRC(89c1b088) SHA1(9b4118815959a5fb65b2a293015f592a46f4126f)  ) // bank 6
	ROM_LOAD( "sound21", 0x6000000, 0x0400000, CRC(13db74bd) SHA1(ab87438bbac97d46b1b8195b61dca1d72172a621)  ) // bank 6
//  ROM_LOAD( "sound22", 0x6400000, 0x0400000 ) // bank 7 ( ** unpopulated **  -  IC24)
//  ROM_LOAD( "sound23", 0x6800000, 0x0400000 ) // bank 7 ( ** unpopulated **  -  IC25)
//  ROM_LOAD( "sound24", 0x6c00000, 0x0400000 ) // bank 7 ( ** unpopulated **  -  IC26)
ROM_END

/***************************************************************************

Jaleco Rock'n 3

Labeled on the chips like:

    Rock'n 3
    VJ-98344
    10
    Ver 1.0


ROM ID       Label                    Rom type
----------------------------------------------
IC19    Rock'n 3 VJ-98344 10 Ver 1.0    27C040
IC59    Rock'n 3 VJ-98344 4  Ver 1.0    27C040
IC65    Rock'n 3 VJ-98344 1  Ver 1.0    27C040

IC10    Rock'n 3 VJ-98344 13 Ver 1.0    27C160
IC33    Rock'n 3 VJ-98344 9  Ver 1.0    27C160
IC32    Rock'n 3 VJ-98344 8  Ver 1.0    27C160
IC38    Rock'n 3 VJ-98344 6  Ver 1.0    27C160

IC57    PS96019-01              ICT 18CV8P PAL
IC14    PS96019-02              ICT 18CV8P PAL
IC58    PS96019-04              ICT 18CV8P PAL

IC43   (no label) XILINX 17S30PC  Serial Config rom

***************************************************************************/

ROM_START( rockn3 )
	ROM_REGION( 0x100000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "rock_n_3_vj-98344_1_v1.0", 0x000001, 0x80000, CRC(abc6ab4a) SHA1(2f1983b95cd9e42d709edac5613b1f0b450df4ba) ) /* IC65 (alt PCB number 1) */
	ROM_LOAD16_BYTE( "rock_n_3_vj-98344_4_v1.0", 0x000000, 0x80000, CRC(3ecba46e) SHA1(64ff5b7932a8d8dc01c649b9dcc1d55cf1e43387) ) /* IC59 (alt PCB number 4) */

	ROM_REGION( 0x400000, "gfx1", 0 )	/* 8x8x8 (Sprites) */
	ROM_LOAD32_WORD( "rock_n_3_vj-98344_8_v1.0", 0x000002, 0x200000, CRC(468bf696) SHA1(d58e399ff876ab0f4ef52aaa85d86d72db307b6a) ) /* IC32 (alt PCB number 8) */
	ROM_LOAD32_WORD( "rock_n_3_vj-98344_9_v1.0", 0x000000, 0x200000, CRC(8a61fc18) SHA1(4e895a2014e711d044ed5d8bff8a91766f14b307) ) /* IC33 (alt PCB number 9) */

	ROM_REGION( 0x200000, "gfx2", 0 )	/* 16x16x8 (Background) */
	ROM_LOAD16_WORD( "rock_n_3_vj-98344_13_v1.0", 0x000000, 0x200000, CRC(e01bf471) SHA1(4485c71770bdb8800ded4afb37814c2d287b78be)  ) /* IC10 (alt PCB number 13) */

	ROM_REGION( 0x200000, "gfx3", 0 )	/* 16x16x8 (Rotation) */
	ROM_LOAD( "rock_n_3_vj-98344_6_v1.0", 0x000000, 0x200000, CRC(4e146de5) SHA1(5971cbb91da5fde652786d82d0143197518bad9b)  ) /* IC38 (alt PCB number 6) */

	ROM_REGION( 0x080000, "gfx4", 0 )	/* 8x8x8 (Foreground) */
	ROM_LOAD( "rock_n_3_vj-98344_10_v1.0", 0x000000, 0x080000, CRC(8100039e) SHA1(e07b1e2f3cbcb1c086edd628d20423ecd4f74860)  ) /* IC19 (alt PCB number 10) */

	ROM_REGION( 0x7000000, "ymz", 0 )	/* Samples - On seperate ROM board with YMZ280B-F sound chip */
	ROM_LOAD( "mr99029-01.ic28", 0x0000000, 0x0400000, CRC(e2f69042) SHA1(deb361a53ed6a9033e21c2f805f327cc3e9b11c6)  ) // COMMON AREA  (alt PCB number 25)
	ROM_FILL(                    0x0400000, 0x0c00000, 0xff )		// BANK AREA (unpopulated IC29, IC36 & IC37 (alt PCB numbers 26, 27 & 28 repectively)
	ROM_LOAD( "mr99029-02.ic1",  0x1000000, 0x0400000, CRC(b328b18f) SHA1(22edebcabd6c8ed65d8c9e501621991d404c430d)  ) // bank 0 (alt PCB number 1)
	ROM_LOAD( "mr99029-03.ic2",  0x1400000, 0x0400000, CRC(f46438e3) SHA1(718f54fc0e3689f5ab29bef2ec13eb2aa9b117fc)  ) // bank 0 (alt PCB number 2)
	ROM_LOAD( "mr99029-04.ic3",  0x1800000, 0x0400000, CRC(b979e887) SHA1(10852ceb1b9e24fb87cf9339bc9fb4ae066a1221)  ) // bank 0 (alt PCB number 3)
	ROM_LOAD( "mr99029-05.ic4",  0x1c00000, 0x0400000, CRC(0bb2c212) SHA1(4f8ab3c96c3e1aa337a3fe871cffc04ec603f8c0)  ) // bank 1 (alt PCB number 4)
	ROM_LOAD( "mr99029-06.ic5",  0x2000000, 0x0400000, CRC(3116e437) SHA1(f1b06592a6f0eba92eb4511d3ca03a3bb51e8c9d)  ) // bank 1 (alt PCB number 5)
	ROM_LOAD( "mr99029-07.ic6",  0x2400000, 0x0400000, CRC(26b37ef6) SHA1(f7090f3ec81f0c651c53d460b476e63f52dd06dc)  ) // bank 1 (alt PCB number 6)
	ROM_LOAD( "mr99029-08.ic7",  0x2800000, 0x0400000, CRC(1dd3f4e3) SHA1(8474e00b962368164c717e5fe2e926852f3b4426)  ) // bank 2 (alt PCB number 7)
	ROM_LOAD( "mr99029-09.ic8",  0x2c00000, 0x0400000, CRC(a1b03d67) SHA1(95f89a37e97d62706e15fd5571ff2e70dd98fee2)  ) // bank 2 (alt PCB number 8)
	ROM_LOAD( "mr99029-10.ic10", 0x3000000, 0x0400000, CRC(35107aac) SHA1(d56a66e15c46c33cf6c9c28edf48b730b681d21a)  ) // bank 2 (alt PCB number 9)
	ROM_LOAD( "mr99029-11.ic11", 0x3400000, 0x0400000, CRC(059ec592) SHA1(205210af558eb7e8e1399b2a506ef0285c5feda3)  ) // bank 3 (alt PCB number 10)
	ROM_LOAD( "mr99029-12.ic12", 0x3800000, 0x0400000, CRC(84d4badb) SHA1(fc20f97a008f000a49e7cadd559789516643704a)  ) // bank 3 (alt PCB number 11)
	ROM_LOAD( "mr99029-13.ic13", 0x3c00000, 0x0400000, CRC(4527a9b7) SHA1(a73ebece5c84bf14f8d25bbd869b7b43b1fcd042)  ) // bank 3 (alt PCB number 12)
	ROM_LOAD( "mr99029-14.ic14", 0x4000000, 0x0400000, CRC(bfa4b7ce) SHA1(4100f2deabb8994e8e3ff897a1db13693ab64c11)  ) // bank 4 (alt PCB number 13)
	ROM_LOAD( "mr99029-15.ic15", 0x4400000, 0x0400000, CRC(a2ccd2ce) SHA1(fc6325219f7b8e68c22a129f5ec4e900e326fb9d)  ) // bank 4 (alt PCB number 14)
	ROM_LOAD( "mr99029-16.ic16", 0x4800000, 0x0400000, CRC(95baf678) SHA1(f7b39a3379f16df0560a22d4f42165ebbe05cebe)  ) // bank 4 (alt PCB number 15)
	ROM_LOAD( "mr99029-17.ic17", 0x4c00000, 0x0400000, CRC(5883c84b) SHA1(54aec4e1e2f5edc198aebc4788caf5062f9a5b6c)  ) // bank 5 (alt PCB number 16)
	ROM_LOAD( "mr99029-18.ic19", 0x5000000, 0x0400000, CRC(f92098ce) SHA1(9b13cd37ad5d7baf36b20218c4bced956084ec45)  ) // bank 5 (alt PCB number 17)
	ROM_LOAD( "mr99029-19.ic20", 0x5400000, 0x0400000, CRC(dbb2c228) SHA1(f7cd24026236e2c616376c695b9e986cc221f36d)  ) // bank 5 (alt PCB number 18)
	ROM_LOAD( "mr99029-20.ic21", 0x5800000, 0x0400000, CRC(9efdae1c) SHA1(6158a1804fbaa9ce27ae7e12cfda5f49084b4998)  ) // bank 6 (alt PCB number 19)
	ROM_LOAD( "mr99029-21.ic22", 0x5c00000, 0x0400000, CRC(5f301b83) SHA1(e24e85c43a62871360545aa42dfa439045334b79)  ) // bank 6 (alt PCB number 20)
//  ROM_LOAD( "ic23",            0x6000000, 0x0400000 ) // bank 6 ( ** unpopulated **  -  alt PCB number 21)
//  ROM_LOAD( "ic24",            0x6400000, 0x0400000 ) // bank 7 ( ** unpopulated **  -  alt PCB number 22)
//  ROM_LOAD( "ic25",            0x6800000, 0x0400000 ) // bank 7 ( ** unpopulated **  -  alt PCB number 23)
//  ROM_LOAD( "ic26",            0x6c00000, 0x0400000 ) // bank 7 ( ** unpopulated **  -  alt PCB number 24)
ROM_END

ROM_START( rockn4 ) /* Prototype */
	ROM_REGION( 0x100000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "rock_n_4_vj-98344_1.bin", 0x000001, 0x80000, CRC(c666caea) SHA1(57018de40d71fe214a6b5cc33c8ad5e88622d010) )
	ROM_LOAD16_BYTE( "rock_n_4_vj-98344_4.bin", 0x000000, 0x80000, CRC(cc94e557) SHA1(d38abed04239d9eecf1b1be7a9f765a1b7aa0d8d) )

	ROM_REGION( 0x400000, "gfx1", 0 )	/* 8x8x8 (Sprites) */
	ROM_LOAD32_WORD( "rock_n_4_vj-98344_8.bin", 0x000002, 0x200000, CRC(5eeae537) SHA1(6bb8c658a2985c3919f0590a0147eead995c01c9) )
	ROM_LOAD32_WORD( "rock_n_4_vj-98344_9.bin", 0x000000, 0x200000, CRC(3fedddc9) SHA1(4bd8f402ecf8e6255326927e825179fa6d300e73) )

	ROM_REGION( 0x200000, "gfx2", 0 )	/* 16x16x8 (Background) */
	ROM_LOAD16_WORD( "rock_n_4_vj-98344_13.bin", 0x000000, 0x200000, CRC(ead41e79) SHA1(9c24b1e52b6ed43d5b5a1caf48f2974b8fa61f4a)  )

	ROM_REGION( 0x200000, "gfx3", 0 )	/* 16x16x8 (Rotation) */
	ROM_LOAD( "rock_n_4_vj-98344_6.bin", 0x000000, 0x200000, CRC(eb16fc67) SHA1(5be40f2c9a5693785268eafcfcf348f147533463)  )

	ROM_REGION( 0x100000, "gfx4", 0 )	/* 8x8x8 (Foreground) */
	ROM_LOAD( "rock_n_4_vj-98344_10.bin", 0x000000, 0x100000, CRC(37d50259) SHA1(fd02f98a981470c47889f0b2f813ce59373a4b42)  )

	ROM_REGION( 0x7000000, "ymz", 0 )	/* Samples */
	ROM_LOAD( "sound00", 0x0000000, 0x0400000, CRC(918ea8eb) SHA1(0cd82859634635b6ce49db36fb91ed3365a101eb)  ) // COMMON AREA
	ROM_FILL(            0x0400000, 0x0c00000, 0xff )		  // BANK AREA
	ROM_LOAD( "sound01", 0x1000000, 0x0400000, CRC(c548e51e) SHA1(4fe1e35c9ed4366dce98b4f4c00f94e202ef15dc)  ) // bank 0
	ROM_LOAD( "sound02", 0x1400000, 0x0400000, CRC(ffda0253) SHA1(9b8ae98accc2f72a1cd881086f89e647e4904ad9)  ) // bank 0
	ROM_LOAD( "sound03", 0x1800000, 0x0400000, CRC(1f813af5) SHA1(a72d842e39b9fc955a2fc6721673b34b1b591e4a)  ) // bank 0
	ROM_LOAD( "sound04", 0x1c00000, 0x0400000, CRC(035c4ff3) SHA1(9290c49244dc45ad5d6543775c5f2cc507e54e77)  ) // bank 1
	ROM_LOAD( "sound05", 0x2000000, 0x0400000, CRC(0f01f7b0) SHA1(e0c6daa1606dd5aaac59a7ae75d76e937e9c0151)  ) // bank 1
	ROM_LOAD( "sound06", 0x2400000, 0x0400000, CRC(31574b1c) SHA1(a08b50b4c4f2be32892b7534f2192101f8af6762)  ) // bank 1
	ROM_LOAD( "sound07", 0x2800000, 0x0400000, CRC(388e2c91) SHA1(493ef760858a82cbc38de59c4db3f273c0ddfdfb)  ) // bank 2
	ROM_LOAD( "sound08", 0x2c00000, 0x0400000, CRC(6e7e3f23) SHA1(4b9b959f79254d0633f1c4324b7ee6a17e222308)  ) // bank 2
	ROM_LOAD( "sound09", 0x3000000, 0x0400000, CRC(39fa512f) SHA1(d07426bc74492496756b67b8ded1b507726720c7)  ) // bank 2
ROM_END

ROM_START( rocknms )
	ROM_REGION( 0x100000, "maincpu", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "mast_prg1", 0x000001, 0x80000, CRC(c36674f8) SHA1(8aeb19fcd6f786c9d76a72abee4b607d29fb7d56) )
	ROM_LOAD16_BYTE( "mast_prg0", 0x000000, 0x80000, CRC(69382065) SHA1(2d528c2954556d440e790db209a2e3563580296a) )

	ROM_REGION( 0x100000, "sub", 0 )		/* 68000 Code */
	ROM_LOAD16_BYTE( "slav_prg1", 0x000001, 0x80000, CRC(769e2245) SHA1(5e6b5456fb213da887be4ef3739685360f6fdae5) )
	ROM_LOAD16_BYTE( "slav_prg0", 0x000000, 0x80000, CRC(55b8df65) SHA1(7744e7a75904174843fc6e3d54324839c6cf104d) )

	ROM_REGION( 0x0800000, "gfx1", 0 )	/* 8x8x8 (Sprites) */
	ROM_LOAD32_WORD( "mast_spr1", 0x000002, 0x400000, CRC(520152dc) SHA1(619a55352c0dab914f6188d66272a24495b5d1d4)  )
	ROM_LOAD32_WORD( "mast_spr0", 0x000000, 0x400000, CRC(1caad02a) SHA1(00c3fc849d1f633874fee30f7d0caf0c62735c50)  )

	ROM_REGION( 0x200000, "gfx2", 0 )	/* 16x16x8 (Background) */
	ROM_LOAD16_WORD( "mast_back", 0x000000, 0x200000, CRC(1ca30e3f) SHA1(763c9dd287c186b6ca8ecb88c3ce29d68fea9179)  )

	ROM_REGION( 0x200000, "gfx3", 0 )	/* 16x16x8 (Rotation) */
	ROM_LOAD( "mast_rot", 0x000000, 0x200000, CRC(1f29b622) SHA1(aab6aafb98fa732266675daa63dc4c0d2084bcbd)  )

	ROM_REGION( 0x080000, "gfx4", 0 )	/* 8x8x8 (Foreground) */
	ROM_LOAD( "mast_front", 0x000000, 0x080000, CRC(a4717579) SHA1(cf28c0f19713ebf9f8fd5d55d654c1cd2e8cd73d)  )

	ROM_REGION( 0x800000, "gfx5", 0 )	/* 8x8x8 (Sprites) */
	ROM_LOAD32_WORD( "slav_spr1", 0x000002, 0x400000, CRC(3f8124b0) SHA1(c9ab89f559551d2298d28e107b2d44d312e53216) )
	ROM_LOAD32_WORD( "slav_spr0", 0x000000, 0x400000, CRC(48a7f5b1) SHA1(4724856bde3cf975efc3be407b60693a69a39365) )

	ROM_REGION( 0x200000, "gfx6", 0 )	/* 16x16x8 (Background) */
	ROM_LOAD16_WORD( "slav_back", 0x000000, 0x200000, CRC(f0a28e32) SHA1(517b98dee6ec201bab02a3c81b0937ed462a626e) )

	ROM_REGION( 0x200000, "gfx7", 0 )	/* 16x16x8 (Rotation) */
	ROM_LOAD( "slav_rot", 0x000000, 0x200000, CRC(0bab21f4) SHA1(afd3f32d7bb99b3f566b302fce11059ae8788715) )

	ROM_REGION( 0x080000, "gfx8", 0 )	/* 8x8x8 (Foreground) */
	ROM_LOAD( "slav_front", 0x000000, 0x080000,  CRC(b65734a7) SHA1(80190e260ed32cb3355f0604722b85eb659483d0) )

	ROM_REGION( 0x7000000, "ymz", 0 )	/* Samples */
	ROM_LOAD( "sound00", 0x0000000, 0x0400000, CRC(8bafae71) SHA1(db74accd4bc1bfeb4a3341a0fd572b81287f1278)  ) // COMMON AREA
	ROM_FILL(                0x0400000, 0x0c00000, 0xff )		// BANK AREA
	ROM_LOAD( "sound01", 0x1000000, 0x0400000, CRC(eec0589b) SHA1(f54c1c7e7741100a1398ebd45aef4755171d9965)  ) // bank 0
	ROM_LOAD( "sound02", 0x1400000, 0x0400000, CRC(564aa972) SHA1(b19e960fd79647e5bcca509982c9887decb92bc6)  ) // bank 0
	ROM_LOAD( "sound03", 0x1800000, 0x0400000, CRC(940302d0) SHA1(b28c2bb1a9b8cea0b6963ffa5d3ac26d90b0bffc)  ) // bank 0
	ROM_LOAD( "sound04", 0x1c00000, 0x0400000, CRC(766db7f8) SHA1(41cfcac2e8d4307f75c56d57431b841e6d64b23c)  ) // bank 1
	ROM_LOAD( "sound05", 0x2000000, 0x0400000, CRC(3a3002f9) SHA1(27b24b8a34a0b919e051e81a10e87aa300b11d8f)  ) // bank 1
	ROM_LOAD( "sound06", 0x2400000, 0x0400000, CRC(06b04df9) SHA1(4bfc7c05843b4533f238f5360230cb71d7a66d56)  ) // bank 1
	ROM_LOAD( "sound07", 0x2800000, 0x0400000, CRC(da74305e) SHA1(9dfb744f36ac8b3661006921dc482e941711f389)  ) // bank 2
	ROM_LOAD( "sound08", 0x2c00000, 0x0400000, CRC(b5a0aa48) SHA1(2deb2c1c97c259f5e79e9dc3cd8859548549a189)  ) // bank 2
	ROM_LOAD( "sound09", 0x3000000, 0x0400000, CRC(0fd4a088) SHA1(5c1ea8a14dee7ee885ce0c86fb463741599db44d)  ) // bank 2
	ROM_LOAD( "sound10", 0x3400000, 0x0400000, CRC(33c89e53) SHA1(7d216f5db6b30c9b05a9a77030498ff68ae6fbad)  ) // bank 3
	ROM_LOAD( "sound11", 0x3800000, 0x0400000, CRC(f9256a3f) SHA1(a3ec0845497d349c97222a1f986c252c8ca781e7)  ) // bank 3
	ROM_LOAD( "sound12", 0x3c00000, 0x0400000, CRC(b0a09f3e) SHA1(d2e37eb935d7ef7e887ff79a49bc11da11c31f3c)  ) // bank 3
	ROM_LOAD( "sound13", 0x4000000, 0x0400000, CRC(d5cee673) SHA1(85194c73c43b69bccbcc895f147d5251bb039c2a)  ) // bank 4
	ROM_LOAD( "sound14", 0x4400000, 0x0400000, CRC(b394aa8a) SHA1(68541d5d98e2d59d6a3096f0c10b74b6f5803722)  ) // bank 4
	ROM_LOAD( "sound15", 0x4800000, 0x0400000, CRC(6c791501) SHA1(8c67f070651493d6f7a2ef7b8a5f9e12c0181f67)  ) // bank 4
	ROM_LOAD( "sound16", 0x4c00000, 0x0400000, CRC(fe80159e) SHA1(b6a980d4f62dfeaa6f51a99518aa4d483fe338e5)  ) // bank 5
	ROM_LOAD( "sound17", 0x5000000, 0x0400000, CRC(142c1159) SHA1(dfabbe69119c84040d6368561e93514ce7bb91db)  ) // bank 5
	ROM_LOAD( "sound18", 0x5400000, 0x0400000, CRC(cc595d85) SHA1(5f725771d79e71d62b64bb18e2a51b839a6e4c7f)  ) // bank 5
	ROM_LOAD( "sound19", 0x5800000, 0x0400000, CRC(82b085a3) SHA1(5a5f2ed90d659bbad710c23b9df2a7dbb3c9acfe)  ) // bank 6
	ROM_LOAD( "sound20", 0x5c00000, 0x0400000, CRC(dd5e9680) SHA1(5a2826641ad75757ce4a583e0ea901d54d20ffca)  ) // bank 6
ROM_END

/***************************************************************************


                                Game Drivers


***************************************************************************/

GAME( 1997, tetrisp2,  0,        tetrisp2, tetrisp2,  0,     ROT0,   "Jaleco / The Tetris Company", "Tetris Plus 2 (World)",           GAME_SUPPORTS_SAVE )
GAME( 1997, tetrisp2j, tetrisp2, tetrisp2, tetrisp2j, 0,     ROT0,   "Jaleco / The Tetris Company", "Tetris Plus 2 (Japan)",           GAME_SUPPORTS_SAVE )

GAME( 1997, nndmseal, 0,        nndmseal, nndmseal, rockn,   ROT0 | ORIENTATION_FLIP_X, "I'Max / Jaleco", "Nandemo Seal Iinkai",       GAME_SUPPORTS_SAVE | GAME_NOT_WORKING )

GAME( 1999, rockn,    0,        rockn,    rockn,   rockn,    ROT270, "Jaleco",                      "Rock'n Tread (Japan)",            GAME_SUPPORTS_SAVE )
GAME( 1999, rockna,   rockn,    rockn,    rockn,   rockn1,   ROT270, "Jaleco",                      "Rock'n Tread (Japan, alternate)", GAME_SUPPORTS_SAVE )
GAME( 1999, rockn2,   0,        rockn2,   rockn,   rockn2,   ROT270, "Jaleco",                      "Rock'n Tread 2 (Japan)",          GAME_SUPPORTS_SAVE )
GAME( 1999, rocknms,  0,        rocknms,  rocknms, rocknms,  ROT0,   "Jaleco",                      "Rock'n MegaSession (Japan)",      GAME_SUPPORTS_SAVE | GAME_IMPERFECT_GRAPHICS )
GAME( 1999, rockn3,   0,        rockn2,   rockn,   rockn3,   ROT270, "Jaleco",                      "Rock'n 3 (Japan)",                GAME_SUPPORTS_SAVE )
GAME( 2000, rockn4,   0,        rockn2,   rockn,   rockn3,   ROT270, "Jaleco / PCCWJ",              "Rock'n 4 (Japan, prototype)",     GAME_SUPPORTS_SAVE )
