/***************************************************************************

                      -= Touch Master / Galaxy Games =-

         driver by Luca Elia (l.elia@tin.it) and Mariusz Wojcieszek


CPU:    68000
Video:  Blitter, double framebuffer
Sound:  OKI6295

[Touch Master]

Input:  Microtouch touch screen
Other:  Dallas NVRAM + optional RTC

To Do:
 - Coin optics
 - Correct sound banking
 - Proper protection emulation in tm4k and later games (where is DS1204 mapped?)
 - Find cause and fix hang in Solitaire Erotic (all Touchmaster version hang in this game)

To be dumped and added:

Touch Master 6000 <-- There doesn't seem to be a "6000" version
Touch Master 7000 *
Touch Master 8000 *

* There is a reported "Minnesota" version with modifications due to legal issues
  Touch Master (current set) is a Euro version, all other sets are "DOMESTIC" (AKA "Standard").
  Is there a Touch Master 6000?  TM5K is version 7.10, then TM7K is version 8, TM8K is version 9.xx

- There are known regional versions like California, Minnesota and likely others.  The difference
  between regional sets are not known at this time.

- Starting with Touch Master 2000, each later version is a chipswap for the mainboard.
   IE: Touch Master 8000 chips can update any Touch Master mainboard 2000 through 7000
  Each version (IE: 2000, 3000, 7000 ect) has different girls for Strip Poker ;-)

Touch Master 8000 part lists (from service bulletin):

A-5343-60194-3  U8  Sound
A-5343-60194-2  U51 Program code
A-5343-60194-1  U52 Program code
A-5343-60194-6  U36 Graphics
A-5343-60194-4  U37 Graphics
A-5343-60194-7  U38 Graphics
A-5343-60194-5  U39 Graphics
A-5343-60194-8  U40 Graphics
A-5343-60194-9  U41 Graphics
A-21657-007     Security Key

The above set is an undumped alternate set, maybe a Euro or special version.

+---------------------------------------------------------------+
|  W24257AK                          GRAPHICS.U37  GRAPHICS.U39 |
|             SECURITY.J12                                      |
| PROGRAM.U52          DS1232        GRAPHICS.U36  GRAPHICS.U38 |
|                                                               |
|  W24257AK                          GRAPHICS.U40  GRAPHICS.U41 |
|               68HC000FN12                                     |
| PROGRAM.U51                                                   |
|                                                               |
|  DS1225AB.U62                      XC3042A     W241024AJ (x2) |
|                                                               |
|   3.664MHZ  24MHz                              W241024AJ (x2) |
| SCN68681       CY7C128A       SOUND.U8    32MHz               |
|     LED2 LED1  CY7C128A                                       |
|    U62                              M6295                     |
-                                                               |
 |Serial Port              LED3                               J8|
-                                                            VOL|
|  J11    J2      J5    J3       J10        J9          J6    J1|
+---------------------------------------------------------------+

U62 is a 16 DIN for a RTC chip (optional)
J Connectors used for all input/output and power. PCB is NON-JAMMA

Chips:
   CPU: MC68HC000FN12
 Video: XC3042A (Sigma Xilinx FPGA gate array)
 Sound: OKI M6295
   OSC: 32MHz, 24MHz & 8.664MHz
 Other: SCN68681C1N40 (Serial controler chip)
        DALLAS DS1225AB-85 Nonvolatile SRAM
        DALLAS DS1204V (used for security)
        DALLAS DS1232 (MicroMonitor Chip)
   RAM: W24257AK-15 (x2 used for CPU data)
        CY7C128A-55PC (x2 used for serial communication)
        W241024AJ-15 (x4 used for blitter frame buffer)

******************************************************************

[Galaxy Games]

Input:  Trackballs and buttons
Other:  EEPROM
To Do:

- Coin optics

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "deprecat.h"
#include "sound/okim6295.h"
#include "machine/eeprom.h"
#include "machine/microtch.h"
#include "machine/68681.h"

/***************************************************************************

                                 General

***************************************************************************/

static struct
{
	running_device *duart68681;
} tmaster_devices;

/***************************************************************************

                                   Sound

***************************************************************************/

static int okibank;
static WRITE16_DEVICE_HANDLER( tmaster_oki_bank_w )
{
	if (ACCESSING_BITS_8_15)
	{
		// data & 0x0800?
		okibank = ((data >> 8) & 3);
		downcast<okim6295_device *>(device)->set_bank_base(okibank * 0x40000);
	}

	if (ACCESSING_BITS_0_7)
	{
		// data & 0x0002?
	}
}

/***************************************************************************

    68681 DUART <-> Microtouch touch screen controller communication

***************************************************************************/

static void duart_irq_handler(running_device *device, UINT8 vector)
{
	cputag_set_input_line_and_vector(device->machine, "maincpu", 4, HOLD_LINE, vector);
};

static void duart_tx(running_device *device, int channel, UINT8 data)
{
	if ( channel == 0 )
	{
		microtouch_rx(1, &data);
	}
};

static void microtouch_tx(running_machine *machine, UINT8 data)
{
	duart68681_rx_data(tmaster_devices.duart68681, 0, data);
}


/***************************************************************************

  DS1644 RTC

***************************************************************************/

static UINT8 rtc_ram[8];

static UINT8 binary_to_BCD(UINT8 data)
{
	data %= 100;

	return ((data / 10) << 4) | (data %10);
}

static READ16_HANDLER(rtc_r)
{
	system_time systime;

	space->machine->current_datetime(systime);
	rtc_ram[0x1] = binary_to_BCD(systime.local_time.second);
	rtc_ram[0x2] = binary_to_BCD(systime.local_time.minute);
	rtc_ram[0x3] = binary_to_BCD(systime.local_time.hour);
	rtc_ram[0x4] = binary_to_BCD(systime.local_time.weekday);
	rtc_ram[0x5] = binary_to_BCD(systime.local_time.mday);
	rtc_ram[0x6] = binary_to_BCD(systime.local_time.month+1);
	rtc_ram[0x7] = binary_to_BCD(systime.local_time.year % 100);

	return rtc_ram[offset];
}

static WRITE16_HANDLER(rtc_w)
{
	if ( offset == 0 )
	{
		rtc_ram[0x0] = data & 0xff;
	}
}


/***************************************************************************

                                Video & Blitter


    Offset:     Bits:                     Value:

        00

        02      fedc ba-- ---- ----
                ---- --9- ---- ----       Layer 1 Buffer To Display
                ---- ---8 ---- ----       Layer 0 Buffer To Display
                ---- ---- 7654 3210

        04                                Width
        06                                X

        08                                Height - 1
        0A                                Y

        0C                                Source Address (low)
        0E                                Source Address (mid)

        10      fedc ba98 ---- ----
                ---- ---- 7--- ----       Layer
                ---- ---- -6-- ----       Buffer
                ---- ---- --5- ----       Solid Fill
                ---- ---- ---4 ----       flipped by lev.3 interrupt routine
                ---- ---- ---- 3---       flipped by lev.2 interrupt routine
                ---- ---- ---- -2--       flipped by lev.1 interrupt routine
                ---- ---- ---- --1-       Flip Y
                ---- ---- ---- ---0       Flip X

    Addr Register:

                fedc ba98 ---- ----       Solid Fill Pen
                ---- ---- 7654 3210       Source Address (high)

    Color Register:

                fedc ba98 ---- ----       Source Pen (Pen Replacement Mode)
                ---- ---- 7654 3210       Palette (0-f) / Destination Pen (Pen Replacement Mode)

    A write to the source address (mid) triggers the blit.
    A the end of the blit, a level 2 IRQ is issued.

***************************************************************************/

static bitmap_t *tmaster_bitmap[2][2];	// 2 layers, 2 buffers per layer
static UINT16 *tmaster_regs;
static UINT16 tmaster_color;
static UINT16 tmaster_addr;
static UINT32 tmaster_gfx_offs, tmaster_gfx_size;
static int (*compute_addr) (UINT16 reg_low, UINT16 reg_mid, UINT16 reg_high);

static int tmaster_compute_addr(UINT16 reg_low, UINT16 reg_mid, UINT16 reg_high)
{
	return (reg_low & 0xff) | ((reg_mid & 0x1ff) << 8) | (reg_high << 17);
}

static int galgames_compute_addr(UINT16 reg_low, UINT16 reg_mid, UINT16 reg_high)
{
	return reg_low | (reg_mid << 16);
}

static VIDEO_START( tmaster )
{
	int layer, buffer;
	for (layer = 0; layer < 2; layer++)
	{
		for (buffer = 0; buffer < 2; buffer++)
		{
			tmaster_bitmap[layer][buffer] = machine->primary_screen->alloc_compatible_bitmap();
			bitmap_fill(tmaster_bitmap[layer][buffer], NULL, 0xff);
		}
	}

	compute_addr = tmaster_compute_addr;
}

static VIDEO_START( galgames )
{
	VIDEO_START_CALL( tmaster );
	compute_addr = galgames_compute_addr;
}

static VIDEO_UPDATE( tmaster )
{
	int layers_ctrl = -1;

#ifdef MAME_DEBUG
	if (input_code_pressed(screen->machine, KEYCODE_Z))
	{
		int mask = 0;
		if (input_code_pressed(screen->machine, KEYCODE_Q))	mask |= 1;
		if (input_code_pressed(screen->machine, KEYCODE_W))	mask |= 2;
		if (mask != 0) layers_ctrl &= mask;
	}
#endif


	bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine));

	if (layers_ctrl & 1)	copybitmap_trans(bitmap, tmaster_bitmap[0][(tmaster_regs[0x02/2]>>8)&1], 0,0,0,0, cliprect, 0xff);
	if (layers_ctrl & 2)	copybitmap_trans(bitmap, tmaster_bitmap[1][(tmaster_regs[0x02/2]>>9)&1], 0,0,0,0, cliprect, 0xff);

	return 0;
}

static WRITE16_HANDLER( tmaster_color_w )
{
	COMBINE_DATA( &tmaster_color );
}

static WRITE16_HANDLER( tmaster_addr_w )
{
	COMBINE_DATA( &tmaster_addr );
}

static void tmaster_draw(running_machine *machine)
{
	int x,y,x0,x1,y0,y1,dx,dy,flipx,flipy,sx,sy,sw,sh, addr, mode, layer,buffer, color;

	UINT8 *gfxdata	=	memory_region( machine, "blitter" ) + tmaster_gfx_offs;

	UINT16 pen;

	bitmap_t *bitmap;

	buffer	=	(tmaster_regs[0x02/2] >> 8) & 3;	// 1 bit per layer, selects the currently displayed buffer
	sw		=	 tmaster_regs[0x04/2];
	sx		=	 tmaster_regs[0x06/2];
	sh		=	 tmaster_regs[0x08/2] + 1;
	sy		=	 tmaster_regs[0x0a/2];
	addr	=	compute_addr(
				 tmaster_regs[0x0c/2],
				 tmaster_regs[0x0e/2], tmaster_addr);
	mode	=	 tmaster_regs[0x10/2];

	layer	=	(mode >> 7) & 1;	// layer to draw to
	buffer	=	((mode >> 6) & 1) ^ ((buffer >> layer) & 1);	// bit 6 selects whether to use the opposite buffer to that displayed
	bitmap	=	tmaster_bitmap[layer][buffer];

	addr <<= 1;

#ifdef MAME_DEBUG
#if 0
	logerror("%s: blit w %03x, h %02x, x %03x, y %02x, src %06x, fill/addr %04x, repl/color %04x, mode %02x\n", cpuexec_describe_context(machine),
			sw,sh,sx,sy, addr, tmaster_addr, tmaster_color, mode
	);
#endif
#endif

	flipx = mode & 1;
	flipy = mode & 2;

	if (flipx)	{ x0 = sw-1;	x1 = -1;	dx = -1;	sx -= sw-1;	}
	else		{ x0 = 0;		x1 = sw;	dx = +1;	}

	if (flipy)	{ y0 = sh-1;	y1 = -1;	dy = -1;	sy -= sh-1;	}
	else		{ y0 = 0;		y1 = sh;	dy = +1;	}

	sx = (sx & 0x7fff) - (sx & 0x8000);
	sy = (sy & 0x7fff) - (sy & 0x8000);

	color = (tmaster_color & 0x0f) << 8;

	switch (mode & 0x20)
	{
		case 0x00:							// blit with transparency
			if (addr > tmaster_gfx_size - sw*sh)
			{
				logerror("%s: blit error, addr %06x out of bounds\n", cpuexec_describe_context(machine),addr);
				addr = tmaster_gfx_size - sw*sh;
			}

			if ( mode & 0x200 )
			{
				// copy from ROM, replacing occurrences of src pen with dst pen

				UINT8 dst_pen = (tmaster_color >> 8) & 0xff;
				UINT8 src_pen = (tmaster_color >> 0) & 0xff;

				for (y = y0; y != y1; y += dy)
				{
					for (x = x0; x != x1; x += dx)
					{
						pen = gfxdata[addr++];

						if (pen == src_pen)
							pen = dst_pen;

						if ((pen != 0xff) && (sx + x >= 0) && (sx + x < 400) && (sy + y >= 0) && (sy + y < 256))
							*BITMAP_ADDR16(bitmap, sy + y, sx + x) = pen + color;
					}
				}
			}
			else
			{
				// copy from ROM as is

				for (y = y0; y != y1; y += dy)
				{
					for (x = x0; x != x1; x += dx)
					{
						pen = gfxdata[addr++];

						if ((pen != 0xff) && (sx + x >= 0) && (sx + x < 400) && (sy + y >= 0) && (sy + y < 256))
							*BITMAP_ADDR16(bitmap, sy + y, sx + x) = pen + color;
					}
				}
			}
			break;

		case 0x20:							// solid fill
			pen = ((tmaster_addr >> 8) & 0xff) + color;

			if ((pen & 0xff) == 0xff)
				pen = 0xff;

			for (y = y0; y != y1; y += dy)
			{
				for (x = x0; x != x1; x += dx)
				{
					if ((sx + x >= 0) && (sx + x < 400) && (sy + y >= 0) && (sy + y < 256))
						*BITMAP_ADDR16(bitmap, sy + y, sx + x) = pen;
				}
			}
			break;

	}
}

static WRITE16_HANDLER( tmaster_blitter_w )
{
	COMBINE_DATA( tmaster_regs + offset );
	switch (offset*2)
	{
		case 0x0e:
			tmaster_draw(space->machine);
			cputag_set_input_line(space->machine, "maincpu", 2, HOLD_LINE);
			break;
	}
}

static READ16_HANDLER( tmaster_blitter_r )
{
	return 0x0000;	// bit 7 = 1 -> blitter busy
}

/***************************************************************************

                                Memory Maps

***************************************************************************/

/***************************************************************************
                                Touch Master
***************************************************************************/

static READ16_HANDLER( tmaster_coins_r )
{
	return input_port_read(space->machine, "COIN")|(mame_rand(space->machine)&0x0800);
}

static ADDRESS_MAP_START( tmaster_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE( 0x000000, 0x1fffff ) AM_ROM
	AM_RANGE( 0x200000, 0x27ffff ) AM_RAM
	AM_RANGE( 0x280000, 0x28ffef ) AM_RAM AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE( 0x28fff0, 0x28ffff ) AM_READWRITE( rtc_r, rtc_w )

	AM_RANGE( 0x300010, 0x300011 ) AM_READ( tmaster_coins_r )

	AM_RANGE( 0x300020, 0x30003f ) AM_DEVREADWRITE8( "duart68681", duart68681_r, duart68681_w, 0xff )

	AM_RANGE( 0x300040, 0x300041 ) AM_DEVWRITE( "oki", tmaster_oki_bank_w )

	AM_RANGE( 0x300070, 0x300071 ) AM_WRITE( tmaster_addr_w )

	AM_RANGE( 0x500000, 0x500011 ) AM_WRITE( tmaster_blitter_w ) AM_BASE( &tmaster_regs )
	AM_RANGE( 0x500010, 0x500011 ) AM_READ ( tmaster_blitter_r )

	AM_RANGE( 0x580000, 0x580001 ) AM_WRITENOP // often

	AM_RANGE( 0x600000, 0x601fff ) AM_RAM_WRITE( paletteram16_xBBBBBGGGGGRRRRR_word_w ) AM_BASE_GENERIC(paletteram)

	AM_RANGE( 0x800000, 0x800001 ) AM_DEVREADWRITE8( "oki", okim6295_r, okim6295_w, 0x00ff )

	AM_RANGE( 0x800010, 0x800011 ) AM_WRITE( tmaster_color_w )
ADDRESS_MAP_END


/***************************************************************************
                                Galaxy Games
***************************************************************************/

// ROM/RAM Banking

#define GALGAMES_BANK_000000_R	"000000_r"
#define GALGAMES_BANK_000000_W	"000000_w"
#define GALGAMES_BANK_200000_R	"200000_r"
#define GALGAMES_BANK_200000_W	"200000_w"
#define GALGAMES_BANK_240000_R	"240000_r"

#define GALGAMES_RAM	0
#define GALGAMES_ROM0	1
#define GALGAMES_ROM1	2
#define GALGAMES_ROM2	3
#define GALGAMES_ROM3	4

static UINT16 *galgames_ram;
static UINT16 galgames_cart;

// NVRAM (5 x EEPROM)

static const eeprom_interface galgames_eeprom_interface =
{
	10,					// address bits 10
	8,					// data bits    8
	"*1100",			// read         110 0aaaaaaaaaa
	"*1010",			// write        101 0aaaaaaaaaa dddddddd
	"*1110",			// erase        111 0aaaaaaaaaa
	"*10000xxxxxxxxx",	// lock         100 00xxxxxxxxx
	"*10011xxxxxxxxx",	// unlock       100 11xxxxxxxxx
	0,					// multi_read
	1					// reset_delay
};

#define GALGAMES_EEPROM_BIOS  "eeprom_bios"
#define GALGAMES_EEPROM_CART1 "eeprom_cart1"
#define GALGAMES_EEPROM_CART2 "eeprom_cart2"
#define GALGAMES_EEPROM_CART3 "eeprom_cart3"
#define GALGAMES_EEPROM_CART4 "eeprom_cart4"

static const char *const galgames_eeprom_names[5] = { GALGAMES_EEPROM_BIOS, GALGAMES_EEPROM_CART1, GALGAMES_EEPROM_CART2, GALGAMES_EEPROM_CART3, GALGAMES_EEPROM_CART4 };

static READ16_HANDLER( galgames_eeprom_r )
{
	running_device *eeprom = space->machine->device(galgames_eeprom_names[galgames_cart]);

	return eeprom_read_bit(eeprom) ? 0x80 : 0x00;
}

static WRITE16_HANDLER( galgames_eeprom_w )
{
	if (data & ~0x0003)
		logerror("CPU #0 PC: %06X - Unknown EEPROM bit written %04X\n",cpu_get_pc(space->cpu),data);

	if ( ACCESSING_BITS_0_7 )
	{
		running_device *eeprom = space->machine->device(galgames_eeprom_names[galgames_cart]);

		// latch the bit
		eeprom_write_bit(eeprom, data & 0x0001);

		// clock line asserted: write latch or select next bit to read
		eeprom_set_clock_line(eeprom, (data & 0x0002) ? ASSERT_LINE : CLEAR_LINE );
	}
}

// BT481A Palette RAMDAC
static UINT32 palette_offset;
static UINT8 palette_index;
static UINT8 palette_data[3];

static WRITE16_HANDLER( galgames_palette_offset_w )
{
	if (ACCESSING_BITS_0_7)
	{
		palette_offset = data & 0xff;
		palette_index = 0;
	}
}
static WRITE16_HANDLER( galgames_palette_data_w )
{
	if (ACCESSING_BITS_0_7)
	{
		palette_data[palette_index] = data & 0xff;
		if (++palette_index == 3)
		{
			int palette_base;
			for (palette_base = 0; palette_base < 0x1000; palette_base += 0x100)
				palette_set_color(space->machine, palette_offset + palette_base, MAKE_RGB(palette_data[0], palette_data[1], palette_data[2]));
			palette_index = 0;
			palette_offset++;
		}
	}
}

// Sound
static READ16_HANDLER( galgames_okiram_r )
{
	return memory_region(space->machine, "oki")[offset] | 0xff00;
}
static WRITE16_HANDLER( galgames_okiram_w )
{
	if (ACCESSING_BITS_0_7)
		memory_region(space->machine, "oki")[offset] = data & 0xff;
}

// Carts (preliminary, PIC communication is not implemented)

static void galgames_update_rombank(running_machine *machine, UINT32 cart)
{
	galgames_cart = cart;

	tmaster_gfx_offs = 0x200000 * cart;

	if (memory_get_bank(machine, GALGAMES_BANK_000000_R) == GALGAMES_RAM)
		memory_set_bank(machine, GALGAMES_BANK_200000_R, GALGAMES_ROM0 + galgames_cart);	// rom

	memory_set_bank(machine, GALGAMES_BANK_240000_R, GALGAMES_ROM0 + galgames_cart);	// rom
}

static WRITE16_HANDLER( galgames_cart_sel_w )
{
	// cart selection (0 1 2 3 4 7)

	if (ACCESSING_BITS_0_7)
	{
		int i;

		switch( data & 0xff )
		{
			case 0x07:		// 7 resets the eeprom
				for (i = 0; i < 5; i++)
					eeprom_set_cs_line(space->machine->device(galgames_eeprom_names[i]), ASSERT_LINE);
				break;

			case 0x00:
			case 0x01:
			case 0x02:
			case 0x03:
			case 0x04:
				eeprom_set_cs_line(space->machine->device(galgames_eeprom_names[data & 0xff]), CLEAR_LINE);
				galgames_update_rombank(space->machine, data & 0xff);
				break;

			default:
				eeprom_set_cs_line(space->machine->device(galgames_eeprom_names[0]), CLEAR_LINE);
				galgames_update_rombank(space->machine, 0);
				logerror("%06x: unknown cart sel = %04x\n", cpu_get_pc(space->cpu), data);
				break;
		}
	}
}

static READ16_HANDLER( galgames_cart_clock_r )
{
	return 0x0080;
}

static WRITE16_HANDLER( galgames_cart_clock_w )
{
	if (ACCESSING_BITS_0_7)
	{
		// bit 3 = clock

		// ROM/RAM banking
		if ((data & 0xf7) == 0x05)
		{
			memory_set_bank(space->machine, GALGAMES_BANK_000000_R, GALGAMES_RAM);	// ram
			galgames_update_rombank(space->machine, galgames_cart);
			logerror("%06x: romram bank = %04x\n", cpu_get_pc(space->cpu), data);
		}
		else
		{
			memory_set_bank(space->machine, GALGAMES_BANK_000000_R, GALGAMES_ROM0);	// rom
			memory_set_bank(space->machine, GALGAMES_BANK_200000_R, GALGAMES_RAM);	// ram
			logerror("%06x: unknown romram bank = %04x\n", cpu_get_pc(space->cpu), data);
		}
	}
}

static READ16_HANDLER( galgames_cart_data_r )
{
	return 0;
}
static WRITE16_HANDLER( galgames_cart_data_w )
{
}


static READ16_HANDLER( dummy_read_01 )
{
	return 0x3;	// Pass the check at PC = 0xfae & a later one
}

static ADDRESS_MAP_START( galgames_map, ADDRESS_SPACE_PROGRAM, 16 )

	AM_RANGE( 0x000000, 0x03ffff ) AM_READ_BANK(GALGAMES_BANK_000000_R) AM_WRITE_BANK(GALGAMES_BANK_000000_W) AM_BASE( &galgames_ram )
	AM_RANGE( 0x040000, 0x1fffff ) AM_ROM AM_REGION( "maincpu", 0x40000 )
	AM_RANGE( 0x200000, 0x23ffff ) AM_READ_BANK(GALGAMES_BANK_200000_R) AM_WRITE_BANK(GALGAMES_BANK_200000_W)
	AM_RANGE( 0x240000, 0x3fffff ) AM_READ_BANK(GALGAMES_BANK_240000_R)

	AM_RANGE( 0x400000, 0x400011 ) AM_WRITE( tmaster_blitter_w ) AM_BASE( &tmaster_regs )
	AM_RANGE( 0x400012, 0x400013 ) AM_WRITE( tmaster_addr_w )
	AM_RANGE( 0x400014, 0x400015 ) AM_WRITE( tmaster_color_w )
	AM_RANGE( 0x400020, 0x400021 ) AM_READ ( tmaster_blitter_r )

	AM_RANGE( 0x600000, 0x600001 ) AM_READ( dummy_read_01 ) AM_WRITENOP
	AM_RANGE( 0x700000, 0x700001 ) AM_READ( dummy_read_01 ) AM_WRITENOP
	AM_RANGE( 0x800020, 0x80003f ) AM_NOP	// ?
	AM_RANGE( 0x900000, 0x900001 ) AM_WRITE( watchdog_reset16_w )

	AM_RANGE( 0xa00000, 0xa00001 ) AM_DEVREADWRITE8( "oki", okim6295_r, okim6295_w, 0x00ff )
	AM_RANGE( 0xb00000, 0xb7ffff ) AM_READWRITE( galgames_okiram_r, galgames_okiram_w ) // (only low bytes tested) 4x N341024SJ-15

	AM_RANGE( 0xc00000, 0xc00001 ) AM_WRITE( galgames_palette_offset_w )
	AM_RANGE( 0xc00002, 0xc00003 ) AM_WRITE( galgames_palette_data_w )

	AM_RANGE( 0xd00000, 0xd00001 ) AM_READ_PORT("TRACKBALL_1_X")
	AM_RANGE( 0xd00000, 0xd00001 ) AM_WRITENOP
	AM_RANGE( 0xd00002, 0xd00003 ) AM_READ_PORT("TRACKBALL_1_Y")
	AM_RANGE( 0xd00004, 0xd00005 ) AM_READ_PORT("TRACKBALL_2_X")
	AM_RANGE( 0xd00006, 0xd00007 ) AM_READ_PORT("TRACKBALL_2_Y")
	AM_RANGE( 0xd00008, 0xd00009 ) AM_READ_PORT("P1")
	AM_RANGE( 0xd0000a, 0xd0000b ) AM_READ_PORT("P2")
	AM_RANGE( 0xd0000c, 0xd0000d ) AM_READ_PORT("SYSTEM") AM_WRITENOP

	AM_RANGE( 0xd0000e, 0xd0000f ) AM_WRITE( galgames_cart_sel_w )
	AM_RANGE( 0xd00010, 0xd00011 ) AM_READWRITE( galgames_eeprom_r, galgames_eeprom_w )
	AM_RANGE( 0xd00012, 0xd00013 ) AM_READWRITE( galgames_cart_data_r, galgames_cart_data_w )
	AM_RANGE( 0xd00014, 0xd00015 ) AM_READWRITE( galgames_cart_clock_r, galgames_cart_clock_w )

ADDRESS_MAP_END


/***************************************************************************

                                Input Ports

***************************************************************************/
static INPUT_PORTS_START( tm )
	PORT_INCLUDE( microtouch )

	PORT_START("COIN")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN5 )	// "M. Coin 1 Input"
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN6 )	// "M. Coin 2 Input"
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BILL1 ) PORT_IMPULSE(2)	// "DBV Input"
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )	// service coin?
	PORT_SERVICE_NO_TOGGLE( 0x0020, IP_ACTIVE_LOW )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 )	// "Calibrate"
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_SPECIAL )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_COIN1 )	// "E. Coin 1" (ECA?) tmaster defaults to e. coin,
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_COIN2 )	// "E. Coin 2" (ECA?) rather than m. coin
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_COIN3 )	// "E. Coin 3" (ECA?) so they're coin1-coin4
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN4 )	// "E. Coin 4" (ECA?)
INPUT_PORTS_END

static INPUT_PORTS_START( tmaster )
	PORT_INCLUDE( microtouch )

	PORT_START("COIN")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )	// "M. Coin 1 Input"
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )	// "M. Coin 2 Input"
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BILL1 ) PORT_IMPULSE(2)	// "DBV Input"
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x0020, IP_ACTIVE_LOW )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 )	// "Calibrate"
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_SPECIAL )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_COIN3 )	// "E. Coin 1" (ECA mech) The rest of the tm games
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_COIN4 )	// "E. Coin 2" (ECA mech) Default to m. coin
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_COIN5 )	// "E. Coin 3" (ECA mech) So these are coin3-coin6
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN6 )	// "E. Coin 4" (ECA mech)
INPUT_PORTS_END

static INPUT_PORTS_START( galgames )
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)	// Button A (right)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)	// Button B (left)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)	// Button A (right)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)	// Button B (left)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1)			// DBA (coin)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )	// CS 1 (coin)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )	// CS 2 (coin)
	PORT_SERVICE( 0x0004, IP_ACTIVE_HIGH )	// System Check
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("TRACKBALL_1_X")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(25) PORT_KEYDELTA(5) PORT_PLAYER(1) PORT_RESET

	PORT_START("TRACKBALL_1_Y")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(25) PORT_KEYDELTA(5) PORT_PLAYER(1) PORT_RESET

	PORT_START("TRACKBALL_2_X")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(25) PORT_KEYDELTA(5) PORT_PLAYER(2) PORT_RESET

	PORT_START("TRACKBALL_2_Y")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(25) PORT_KEYDELTA(5) PORT_PLAYER(2) PORT_RESET
INPUT_PORTS_END


/***************************************************************************

                               Machine Drivers

***************************************************************************/

static MACHINE_START( tmaster )
{
	microtouch_init(machine, microtouch_tx, 0);
}

static MACHINE_RESET( tmaster )
{
	tmaster_gfx_offs = 0;
	tmaster_gfx_size = memory_region_length(machine, "blitter");

	tmaster_devices.duart68681 = machine->device( "duart68681" );
}

static INTERRUPT_GEN( tm3k_interrupt )
{
	switch (cpu_getiloops(device))
	{
		case 0:		cpu_set_input_line(device, 2, HOLD_LINE);	break;
		case 1:		cpu_set_input_line(device, 3, HOLD_LINE);	break;
		default:	cpu_set_input_line(device, 1, HOLD_LINE);	break;
	}
}

static const duart68681_config tmaster_duart68681_config =
{

	duart_irq_handler,
	duart_tx,
	NULL,
	NULL
};

static MACHINE_DRIVER_START( tm3k )
	MDRV_CPU_ADD("maincpu", M68000, XTAL_24MHz / 2) /* 12MHz */
	MDRV_CPU_PROGRAM_MAP(tmaster_map)
	MDRV_CPU_VBLANK_INT_HACK(tm3k_interrupt,2+20) // ??

	MDRV_MACHINE_START(tmaster)
	MDRV_MACHINE_RESET(tmaster)

	MDRV_DUART68681_ADD( "duart68681", XTAL_8_664MHz / 2 /*??*/, tmaster_duart68681_config )

	MDRV_NVRAM_HANDLER(generic_0fill)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(400, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 400-1, 0, 256-1)

	MDRV_PALETTE_LENGTH(0x1000)

	MDRV_VIDEO_START(tmaster)
	MDRV_VIDEO_UPDATE(tmaster)

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_OKIM6295_ADD("oki", XTAL_32MHz / 16, OKIM6295_PIN7_HIGH)  /* 2MHz; clock frequency & pin 7 not verified */
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( tm )
	MDRV_IMPORT_FROM(tm3k)

	MDRV_OKIM6295_REPLACE("oki", 1122000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


static INTERRUPT_GEN( galgames_interrupt )
{
	switch (cpu_getiloops(device))
	{
		case 0:		cpu_set_input_line(device, 3, HOLD_LINE);	break;
					// lev 2 triggered at the end of a blit
		default:	cpu_set_input_line(device, 1, HOLD_LINE);	break;
	}
}

static MACHINE_RESET( galgames )
{
	tmaster_gfx_offs = 0;
	tmaster_gfx_size = 0x200000;

	memory_set_bank(machine, GALGAMES_BANK_000000_R, GALGAMES_ROM0);	// rom
	memory_set_bank(machine, GALGAMES_BANK_000000_W, GALGAMES_RAM);		// ram

	memory_set_bank(machine, GALGAMES_BANK_200000_R, GALGAMES_RAM);		// ram
	memory_set_bank(machine, GALGAMES_BANK_200000_W, GALGAMES_RAM);		// ram

	memory_set_bank(machine, GALGAMES_BANK_240000_R, GALGAMES_ROM0);	// rom

	galgames_update_rombank(machine, 0);

	machine->device("maincpu")->reset();
}

static MACHINE_DRIVER_START( galgames )
	MDRV_CPU_ADD("maincpu", M68000, XTAL_24MHz / 2)
	MDRV_CPU_PROGRAM_MAP(galgames_map)
	MDRV_CPU_VBLANK_INT_HACK(galgames_interrupt, 1+20)	// ??

	// 5 EEPROMs on the motherboard (for BIOS + 4 Carts)
	MDRV_EEPROM_ADD(GALGAMES_EEPROM_BIOS,  galgames_eeprom_interface)
	MDRV_EEPROM_ADD(GALGAMES_EEPROM_CART1, galgames_eeprom_interface)
	MDRV_EEPROM_ADD(GALGAMES_EEPROM_CART2, galgames_eeprom_interface)
	MDRV_EEPROM_ADD(GALGAMES_EEPROM_CART3, galgames_eeprom_interface)
	MDRV_EEPROM_ADD(GALGAMES_EEPROM_CART4, galgames_eeprom_interface)

	MDRV_MACHINE_RESET( galgames )

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_SIZE(400, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 400-1, 0, 256-1)

	MDRV_PALETTE_LENGTH(0x1000)	// only 0x100 used

	MDRV_VIDEO_START(galgames)
	MDRV_VIDEO_UPDATE(tmaster)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_OKIM6295_ADD("oki", XTAL_24MHz / 8, OKIM6295_PIN7_LOW) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END

/*
    Each cartridge contains a PIC, that should provide, among other things, the following header:

    4345 5331 3939 3700 0c10 ffb3 3c00 0000       "CES1997"
    fffe f2f7 8557 c119 0000 0000 2340 188e
*/
static MACHINE_DRIVER_START( galgame2 )
	MDRV_IMPORT_FROM(galgames)
//  MDRV_CPU_ADD("pic", PIC12C508, XTAL_24MHz / 2)
MACHINE_DRIVER_END


/***************************************************************************

                               ROMs Loading

***************************************************************************/

/***************************************************************************

Touch Master
1996, Midway

68000 @ 12MHz
u51 - u52 program code
u36 -> u39 gfx
u8 sound
OKI6295
NVSRAM DS1225a
Philips SCN68681
Xlinx XC3042a

***************************************************************************/

ROM_START( tm )
	ROM_REGION( 0x200000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "tmaster_v300_euro.u51", 0x000000, 0x080000, CRC(edaa5874) SHA1(48b99bc7f5a6453def265967ca7d8eefdf9dc97b) ) /* Ver: 3.00 Euro 11-25-96 */
	ROM_LOAD16_BYTE( "tmaster_v300_euro.u52", 0x000001, 0x080000, CRC(e9fd30fc) SHA1(d91ea05d5f574603883336729fb9df705688945d) ) /* Ver: 3.00 Euro 11-25-96 */

	ROM_REGION( 0x400000, "blitter", ROMREGION_ERASE )	// Blitter gfx
	ROM_LOAD16_BYTE( "tmaster.u38", 0x100000, 0x080000, CRC(68885ef6) SHA1(010602b59c33c3e490491a296ddaf8952e315b83) ) /* Same as U38 below */
	ROM_LOAD16_BYTE( "tmaster.u36", 0x100001, 0x080000, CRC(204096ec) SHA1(9239923b7eedb6003c63ef2e8ff224edee657bbc) ) /* Same as U36 below */
	// unused gap
	ROM_LOAD16_BYTE( "tmaster.u39", 0x300000, 0x080000, CRC(cbb716cb) SHA1(4e8d8f6cbfb25a8161ff8fe7505d6b209650dd2b) ) /* Different then U39 below, newer or region specific? */
	ROM_LOAD16_BYTE( "tmaster.u37", 0x300001, 0x080000, CRC(e0b6a9f7) SHA1(7e057ca87833c682e5be03668469259bbdefbf20) ) /* Different then U37 below, newer or region specific? */

	ROM_REGION( 0x100000, "oki", 0 ) // Samples
	ROM_LOAD( "tmaster.u8", 0x40000, 0x040000, CRC(f39ad4cf) SHA1(9bcb9a5dd3636d6541eeb3e737c7253ab0ed4e8d) ) /* Same as U8 below */
	ROM_CONTINUE(           0xc0000, 0x040000 )
ROM_END

ROM_START( tmdo )
	ROM_REGION( 0x200000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "tmaster_v22-01.u51", 0x000000, 0x080000, CRC(12e1b085) SHA1(b01325e0067204991a707db24e3e2036708ebccf) ) /* Ver: 2.2-01 Standard 10-17-96 */
	ROM_LOAD16_BYTE( "tmaster_v22-01.u52", 0x000001, 0x080000, CRC(6c2c643f) SHA1(8dd7930f4c499483ca46b0b97bde94cb8d6e06aa) ) /* Ver: 2.2-01 Standard 10-17-96 */

	ROM_REGION( 0x400000, "blitter", ROMREGION_ERASE )	// Blitter gfx
	ROM_LOAD16_BYTE( "tmaster.u38", 0x100000, 0x080000, CRC(68885ef6) SHA1(010602b59c33c3e490491a296ddaf8952e315b83) ) /* Marked as Rev 2.1 */
	ROM_LOAD16_BYTE( "tmaster.u36", 0x100001, 0x080000, CRC(204096ec) SHA1(9239923b7eedb6003c63ef2e8ff224edee657bbc) ) /* Marked as Rev 2.1 */
	// unused gap
	ROM_LOAD16_BYTE( "tmaster_v21.u39", 0x300000, 0x080000, CRC(a4445260) SHA1(915347f69d7ea45f8f299a67d77ff437983495d2) ) /* Marked as Rev 2.1 */
	ROM_LOAD16_BYTE( "tmaster_v21.u37", 0x300001, 0x080000, CRC(0e140a3e) SHA1(10a34e3b95c0d36fe687fe8c1124ef244a93d720) ) /* Marked as Rev 2.1 */

	ROM_REGION( 0x100000, "oki", 0 ) // Samples
	ROM_LOAD( "tmaster.u8", 0x40000, 0x040000, CRC(f39ad4cf) SHA1(9bcb9a5dd3636d6541eeb3e737c7253ab0ed4e8d) ) /* Marked as Rev 1.0 */
	ROM_CONTINUE(           0xc0000, 0x040000 )
ROM_END

/***************************************************************************

Touchmaster 2000
by Midway (c) 1996
touchscreen game

All chips are ST M27C4001
---------------------------

Name_Board Location        Version               Use             Checksum
-------------------------------------------------------------------------
TM2K_v402.u51              4.02 Game Program & Cpu instructions   c517
TM2K_v402.u52              4.02 Game Program & Cpu instructions   e82c

TM2K_v400.u51              4.00 Game Program & Cpu instructions   a49b
TM2K_v400.u52              4.00 Game Program & Cpu instructions   7c2f

TM2K_graphic.u36           4.00 Video Images & Graphics           20cb
TM2K_graphic.u37           4.00 Video Images & Graphics           f5cf
TM2K_graphic.u38           4.00 Video Images & Graphics           14c7
TM2K_graphic.u39           4.00 Video Images & Graphics           043e
TM2K_sound.u8              1.0  Audio Program & sounds            9307

Does not require a security key

***************************************************************************/

ROM_START( tm2k )
	ROM_REGION( 0x200000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "tm2k_v402.u51", 0x000000, 0x080000, CRC(47269aeb) SHA1(6b7ebfde290f7d21a36a72b00dc6523490581edb) ) /* Ver: 4.02 Standard 5-30-97 */
	ROM_LOAD16_BYTE( "tm2k_v402.u52", 0x000001, 0x080000, CRC(2e3564ac) SHA1(9a71f38841bc17c291cb3f513b18ebe50fc18d9f) ) /* Ver: 4.02 Standard 5-30-97 */

	ROM_REGION( 0x400000, "blitter", ROMREGION_ERASE )	// Blitter gfx
	ROM_LOAD16_BYTE( "tm2k_graphic.u38", 0x100000, 0x080000, CRC(22bb6cc5) SHA1(fc6cfd4e1e6e1455d648a7b63f2c8e37cdfe86d6) ) /* All 4 graphic roms marked as Rev 4.00 */
	ROM_LOAD16_BYTE( "tm2k_graphic.u36", 0x100001, 0x080000, CRC(7f0840ac) SHA1(1c3af419d571579a3f2c561617d55914d28ef22b) )
	ROM_LOAD16_BYTE( "tm2k_graphic.u39", 0x300000, 0x080000, CRC(059e1bd8) SHA1(7451c1cfa0d090b0566e353738a1ffba732a8ad2) )
	ROM_LOAD16_BYTE( "tm2k_graphic.u37", 0x300001, 0x080000, CRC(4cf65950) SHA1(74d49166da19ecc4b8fc1e8e3f01361dfb645eea) )

	ROM_REGION( 0x100000, "oki", 0 ) // Samples
	ROM_LOAD( "tm2k_sound.u8", 0x40000, 0x040000, CRC(f39ad4cf) SHA1(9bcb9a5dd3636d6541eeb3e737c7253ab0ed4e8d) ) /* Marked as Rev 1.0 */
	ROM_CONTINUE(              0xc0000, 0x040000 )
ROM_END

ROM_START( tm2ka )
	ROM_REGION( 0x200000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "tm2k_v400.u51", 0x000000, 0x080000, CRC(c110502b) SHA1(e9415ed23b9bb0851548e75c208ebcbe6ac2a708) ) /* Ver: 4.00 Standard 5-16-97 */
	ROM_LOAD16_BYTE( "tm2k_v400.u52", 0x000001, 0x080000, CRC(a17c1d6e) SHA1(5ecb8f27f75469ab9600b3f640eb1acc7a3980e0) ) /* Ver: 4.00 Standard 5-16-97 */

	ROM_REGION( 0x400000, "blitter", ROMREGION_ERASE )	// Blitter gfx
	ROM_LOAD16_BYTE( "tm2k_graphic.u38", 0x100000, 0x080000, CRC(22bb6cc5) SHA1(fc6cfd4e1e6e1455d648a7b63f2c8e37cdfe86d6) ) /* All 4 graphic roms marked as Rev 4.00 */
	ROM_LOAD16_BYTE( "tm2k_graphic.u36", 0x100001, 0x080000, CRC(7f0840ac) SHA1(1c3af419d571579a3f2c561617d55914d28ef22b) )
	ROM_LOAD16_BYTE( "tm2k_graphic.u39", 0x300000, 0x080000, CRC(059e1bd8) SHA1(7451c1cfa0d090b0566e353738a1ffba732a8ad2) )
	ROM_LOAD16_BYTE( "tm2k_graphic.u37", 0x300001, 0x080000, CRC(4cf65950) SHA1(74d49166da19ecc4b8fc1e8e3f01361dfb645eea) )

	ROM_REGION( 0x100000, "oki", 0 ) // Samples
	ROM_LOAD( "tm2k_sound.u8", 0x40000, 0x040000, CRC(f39ad4cf) SHA1(9bcb9a5dd3636d6541eeb3e737c7253ab0ed4e8d) ) /* Marked as Rev 1.0 */
	ROM_CONTINUE(              0xc0000, 0x040000 )
ROM_END

/***************************************************************************

Touchmaster 3000
by Midway (c) 1997
touchscreen game

All chips are SGS 27C801 (some kits/upgrades used mask roms)
---------------------------

Name_Board Location        Version               Use             Checksum
-------------------------------------------------------------------------
TM3K_v502.u51              5.02 Game Program & Cpu instructions   c308
TM3K_v502.u52              5.02 Game Program & Cpu instructions   4d5e

TM3K_v501.u51              5.01 Game Program & Cpu instructions   0c6c
TM3K_v501.u52              5.01 Game Program & Cpu instructions   b2d8

TM3K_graphic.u36           5.0  Video Images & Graphics           54f1
TM3K_graphic.u37           5.0  Video Images & Graphics           4856
TM3K_graphic.u38           5.0  Video Images & Graphics           5493
TM3K_graphic.u39           5.0  Video Images & Graphics           6029
TM3K_graphic.u40           5.0  Video Images & Graphics           ccb4
TM3K_graphic.u41           5.0  Video Images & Graphics           e9ab
TM3K_sound.u8              5.0  Audio Program & sounds            64d5

Does not require a security key

Box labeled as:

TMSTR 3000 UPGRADE KIT USA-DBV
63373100870

***************************************************************************/

ROM_START( tm3k )
	ROM_REGION( 0x200000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "tm3k_v502.u51", 0x000000, 0x100000, CRC(6267e2bd) SHA1(c81e5cd059a9ad2f6a36261738e39740a1a3a03f) ) /* TOUCHMASTER 3000 U51 DOMESTIC 5.02 (Standard 11-17-97) (yellow label) */
	ROM_LOAD16_BYTE( "tm3k_v502.u52", 0x000001, 0x100000, CRC(836fdf1e) SHA1(2ee9c0929950afb72f172b253d6c392e9a698037) ) /* TOUCHMASTER 3000 U52 DOMESTIC 5.02 (Standard 11-17-97) (yellow label) */

	ROM_REGION( 0x600000, "blitter", 0 )	// Blitter gfx
	ROM_LOAD16_BYTE( "tm3k_graphic.u38", 0x000000, 0x100000, CRC(a6683899) SHA1(d05024390917cdb1871d030996da8e1eb6460918) ) /* Labeled TOUCHMASTER U38 STANDARD 5.0 (pink label) */
	ROM_LOAD16_BYTE( "tm3k_graphic.u36", 0x000001, 0x100000, CRC(7bde520d) SHA1(77750b689e2f0d47804042456e54bbd9c28deeac) ) /* Labeled TOUCHMASTER U36 STANDARD 5.0 (pink label) */
	ROM_LOAD16_BYTE( "tm3k_graphic.u39", 0x200000, 0x100000, CRC(206b56a6) SHA1(09e5e05bffd0a09abd24d668e2c59b56f2c79134) ) /* Labeled TOUCHMASTER U39 STANDARD 5.0 (pink label) */
	ROM_LOAD16_BYTE( "tm3k_graphic.u37", 0x200001, 0x100000, CRC(18f50eb3) SHA1(a7c9d3b24b5fd110380ec87d9200d55cad473efc) ) /* Labeled TOUCHMASTER U37 STANDARD 5.0 (pink label) */
	ROM_LOAD16_BYTE( "tm3k_graphic.u41", 0x400000, 0x100000, CRC(c35c0536) SHA1(a29fd88e8f3e124f6e84012c3573616f6447eeaa) ) /* Labeled TOUCHMASTER U41 STANDARD 5.0 (pink label) */
	ROM_LOAD16_BYTE( "tm3k_graphic.u40", 0x400001, 0x100000, CRC(353df7ca) SHA1(d6c5d5449af6b6a3acee219778583904c5b554b4) ) /* Labeled TOUCHMASTER U40 STANDARD 5.0 (pink label) */

	ROM_REGION( 0x100000, "oki", 0 ) // Samples
	ROM_LOAD( "tm3k_sound.u8", 0x00000, 0x100000, CRC(d0ae33c1) SHA1(a079def9a086a091fcc4493a44fec756d2470415) ) /* Labeled TOUCHMASTER U8 5.0 (green label) */
ROM_END

ROM_START( tm3ka )
	ROM_REGION( 0x200000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "tm3k_v501.u51", 0x000000, 0x100000, CRC(c9522279) SHA1(e613b791f831271722f05b7e96c35519fa9fc174) ) /* TOUCHMASTER 3000 U51 DOMESTIC 5.01 (Standard 11-4-97) (yellow label) */
	ROM_LOAD16_BYTE( "tm3k_v501.u52", 0x000001, 0x100000, CRC(8c6a0db7) SHA1(6b0eae60ea471cd8c4001749ac2677d8d4532567) ) /* TOUCHMASTER 3000 U52 DOMESTIC 5.01 (Standard 11-4-97) (yellow label) */

	ROM_REGION( 0x600000, "blitter", 0 )	// Blitter gfx
	ROM_LOAD16_BYTE( "tm3k_graphic.u38", 0x000000, 0x100000, CRC(a6683899) SHA1(d05024390917cdb1871d030996da8e1eb6460918) ) /* Labeled TOUCHMASTER U38 STANDARD 5.0 (pink label) */
	ROM_LOAD16_BYTE( "tm3k_graphic.u36", 0x000001, 0x100000, CRC(7bde520d) SHA1(77750b689e2f0d47804042456e54bbd9c28deeac) ) /* Labeled TOUCHMASTER U36 STANDARD 5.0 (pink label) */
	ROM_LOAD16_BYTE( "tm3k_graphic.u39", 0x200000, 0x100000, CRC(206b56a6) SHA1(09e5e05bffd0a09abd24d668e2c59b56f2c79134) ) /* Labeled TOUCHMASTER U39 STANDARD 5.0 (pink label) */
	ROM_LOAD16_BYTE( "tm3k_graphic.u37", 0x200001, 0x100000, CRC(18f50eb3) SHA1(a7c9d3b24b5fd110380ec87d9200d55cad473efc) ) /* Labeled TOUCHMASTER U37 STANDARD 5.0 (pink label) */
	ROM_LOAD16_BYTE( "tm3k_graphic.u41", 0x400000, 0x100000, CRC(c35c0536) SHA1(a29fd88e8f3e124f6e84012c3573616f6447eeaa) ) /* Labeled TOUCHMASTER U41 STANDARD 5.0 (pink label) */
	ROM_LOAD16_BYTE( "tm3k_graphic.u40", 0x400001, 0x100000, CRC(353df7ca) SHA1(d6c5d5449af6b6a3acee219778583904c5b554b4) ) /* Labeled TOUCHMASTER U40 STANDARD 5.0 (pink label) */

	ROM_REGION( 0x100000, "oki", 0 ) // Samples
	ROM_LOAD( "tm3k_sound.u8", 0x00000, 0x100000, CRC(d0ae33c1) SHA1(a079def9a086a091fcc4493a44fec756d2470415) ) /* Labeled TOUCHMASTER U8 5.0 (green label) */
ROM_END

/***************************************************************************

Touchmaster 4000
by Midway (c) 1998
touchscreen game

All chips are SGS 27C801 (some kits/upgrades used mask roms)
---------------------------

Name_Board Location        Version               Use                      Checksum
-----------------------------------------------------------------------------------
TM4K_v603.u51              6.03 Game Program & Cpu instructions 96B0
TM4K_v603.u52              6.03 Game Program & Cpu instructions 2842

TM4K_v602.u51              6.02 Game Program & Cpu instructions FEA0
TM4K_v602.u52              6.02 Game Program & Cpu instructions 9A71

TM4K_graphic.u36           6.0  Video Images & Graphics         54f1 (same as TM3K)
TM4K_graphic.u37           6.0  Video Images & Graphics         609E
TM4K_graphic.u38           6.0  Video Images & Graphics         5493 (same as TM3K)
TM4K_graphic.u39           6.0  Video Images & Graphics         CB90
TM4K_graphic.u40           6.0  Video Images & Graphics         208A
TM4K_graphic.u41           6.0  Video Images & Graphics         385D
TM4K_sound.u8              6.0  Audio Program & sounds          DE0B

J12 DALLAS DS1204V         N/A  Security Key (required for this Version) - Labeled A-21657-003

***************************************************************************/

ROM_START( tm4k )
	ROM_REGION( 0x200000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "tm4k_v603.u51", 0x000000, 0x100000, CRC(0c88c856) SHA1(4c60c09812ab05f9dccef3038a5ddbd4632dbf4e) ) /* TOUCHMASTER 4000 U51 DOMESTIC  6.03 (Standard 6-23-98) */
	ROM_LOAD16_BYTE( "tm4k_v603.u52", 0x000001, 0x100000, CRC(9320bfe9) SHA1(cc3a51f439c139ca30efe28a817cf4f68679180e) ) /* TOUCHMASTER 4000 U52 DOMESTIC  6.03 (Standard 6-23-98) */

	ROM_REGION( 0x600000, "blitter", 0 )	// Blitter gfx
	ROM_LOAD16_BYTE( "tm4k_graphic.u38", 0x000000, 0x100000, CRC(a6683899) SHA1(d05024390917cdb1871d030996da8e1eb6460918) ) /* Mask rom labeled 5341-15746-03 U38 VIDEO IMAGE */
	ROM_LOAD16_BYTE( "tm4k_graphic.u36", 0x000001, 0x100000, CRC(7bde520d) SHA1(77750b689e2f0d47804042456e54bbd9c28deeac) ) /* Mask rom labeled 5341-15746-01 U36 VIDEO IMAGE */
	ROM_LOAD16_BYTE( "tm4k_graphic.u39", 0x200000, 0x100000, CRC(bac88cfb) SHA1(26ed169296b890c5f5b50c418c15299355a6592f) ) /* Mask rom labeled 5341-15746-04 U39 VIDEO IMAGE */
	ROM_LOAD16_BYTE( "tm4k_graphic.u37", 0x200001, 0x100000, CRC(bf49fafa) SHA1(b400667bf654dc9cd01a85c8b99670459400fd60) ) /* Mask rom labeled 5341-15746-02 U37 VIDEO IMAGE */
	ROM_LOAD16_BYTE( "tm4k_graphic.u41", 0x400000, 0x100000, CRC(e97edb1e) SHA1(75510676cf1692ad03efd4ccd57d25af1cc8ef2a) ) /* Mask rom labeled 5341-15746-06 U41 VIDEO IMAGE */
	ROM_LOAD16_BYTE( "tm4k_graphic.u40", 0x400001, 0x100000, CRC(f6771a09) SHA1(74f71d5e910006c83a38170f24aa811c38a3e020) ) /* Mask rom labeled 5341-15746-05 U40 VIDEO IMAGE */

	ROM_REGION( 0x100000, "oki", 0 ) // Samples
	ROM_LOAD( "tm4k_sound.u8", 0x00000, 0x100000, CRC(48c3782b) SHA1(bfe105ddbde8bbbd84665dfdd565d6d41926834a) ) /* Mask rom labeled 5341-15746-07 U8 SOUND IMAGE */
ROM_END

ROM_START( tm4ka )
	ROM_REGION( 0x200000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "tm4k_v602.u51", 0x000000, 0x100000, CRC(3d8d7848) SHA1(31638f23cdd5e6cfbb2270e953f84fe1bd437950) ) /* TOUCHMASTER 4000 U51 DOMESTIC  6.02 (Standard 4-14-98) */
	ROM_LOAD16_BYTE( "tm4k_v602.u52", 0x000001, 0x100000, CRC(6d412871) SHA1(ae27c7723b292daf6682c53bafac22e4a3cd1ece) ) /* TOUCHMASTER 4000 U52 DOMESTIC  6.02 (Standard 4-14-98) */

	ROM_REGION( 0x600000, "blitter", 0 )	// Blitter gfx
	ROM_LOAD16_BYTE( "tm4k_graphic.u38", 0x000000, 0x100000, CRC(a6683899) SHA1(d05024390917cdb1871d030996da8e1eb6460918) ) /* Mask rom labeled 5341-15746-03 U38 VIDEO IMAGE */
	ROM_LOAD16_BYTE( "tm4k_graphic.u36", 0x000001, 0x100000, CRC(7bde520d) SHA1(77750b689e2f0d47804042456e54bbd9c28deeac) ) /* Mask rom labeled 5341-15746-01 U36 VIDEO IMAGE */
	ROM_LOAD16_BYTE( "tm4k_graphic.u39", 0x200000, 0x100000, CRC(bac88cfb) SHA1(26ed169296b890c5f5b50c418c15299355a6592f) ) /* Mask rom labeled 5341-15746-04 U39 VIDEO IMAGE */
	ROM_LOAD16_BYTE( "tm4k_graphic.u37", 0x200001, 0x100000, CRC(bf49fafa) SHA1(b400667bf654dc9cd01a85c8b99670459400fd60) ) /* Mask rom labeled 5341-15746-02 U37 VIDEO IMAGE */
	ROM_LOAD16_BYTE( "tm4k_graphic.u41", 0x400000, 0x100000, CRC(e97edb1e) SHA1(75510676cf1692ad03efd4ccd57d25af1cc8ef2a) ) /* Mask rom labeled 5341-15746-06 U41 VIDEO IMAGE */
	ROM_LOAD16_BYTE( "tm4k_graphic.u40", 0x400001, 0x100000, CRC(f6771a09) SHA1(74f71d5e910006c83a38170f24aa811c38a3e020) ) /* Mask rom labeled 5341-15746-05 U40 VIDEO IMAGE */

	ROM_REGION( 0x100000, "oki", 0 ) // Samples
	ROM_LOAD( "tm4k_sound.u8", 0x00000, 0x100000, CRC(48c3782b) SHA1(bfe105ddbde8bbbd84665dfdd565d6d41926834a) ) /* Mask rom labeled 5341-15746-07 U8 SOUND IMAGE */
ROM_END

/***************************************************************************

Touchmaster 5000
by Midway (c) 1998
touchscreen game

All chips are ST M27C801 (some kits/upgrades used mask roms)
---------------------------

Name Board Location        Version               Use                      Checksum
-----------------------------------------------------------------------------------
tm5k_v7_10.u51             7.10 Game Program & Cpu instructions 1A51
tm5k_v7_10.u52             7.10 Game Program & Cpu instructions 5A01
tm5k_graphic.u36           7.0  Video Images & Graphics         DB7F
tm5k_graphic.u37           7.0  Video Images & Graphics         871B
tm5k_graphic.u38           7.0  Video Images & Graphics         EDCE
tm5k_graphic.u39           7.0  Video Images & Graphics         657F
tm5k_graphic.u40           7.0  Video Images & Graphics         93E0
tm5k_graphic.u41           7.0  Video Images & Graphics         FCA5
tm5k_sound.u8              7.0  Audio Program & sounds          F474

J12 DALLAS DS1204V         N/A  Security Key (required for this Version) - Labeled A-21657-004
-----------------------------------------------------------------------------------

* EPROM versions are labeled with white labels

***************************************************************************/

ROM_START( tm5k )
	ROM_REGION( 0x200000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "tm5k_v7_10.u51", 0x000000, 0x100000, CRC(df0bd25e) SHA1(db1a197ed4c868743397f3823f3f1d42b9329f80) ) /* TOUCHMASTER 5000 U51 DOMESTIC 7.10 (Standard 10-9-98) (tan label) */
	ROM_LOAD16_BYTE( "tm5k_v7_10.u52", 0x000001, 0x100000, CRC(ddf9e8dc) SHA1(3228f2eba067bdf1bd639116bffc589585ea3e72) ) /* TOUCHMASTER 5000 U52 DOMESTIC 7.10 (Standard 10-9-98) (tan label) */

	ROM_REGION( 0x600000, "blitter", 0 )	// Blitter gfx
	ROM_LOAD16_BYTE( "tm5k_graphic.u38", 0x000000, 0x100000, CRC(93038e7c) SHA1(448f69bf51ac992f6b35b471cba9675c67984cd7) ) /* Mask rom labeled 5341-15951-07 U38 VIDEO IMAGE */
	ROM_LOAD16_BYTE( "tm5k_graphic.u36", 0x000001, 0x100000, CRC(5453a44a) SHA1(094439a56336ca933b0b7ede8c057546d1d490b2) ) /* Mask rom labeled 5341-15951-06 U36 VIDEO IMAGE */
	ROM_LOAD16_BYTE( "tm5k_graphic.u39", 0x200000, 0x100000, CRC(1349fdc7) SHA1(5118983be584455320f0d6006133f38e6a8ee0d8) ) /* Mask rom labeled 5341-15951-05 U39 VIDEO IMAGE */
	ROM_LOAD16_BYTE( "tm5k_graphic.u37", 0x200001, 0x100000, CRC(8bcc376c) SHA1(0588f6f96090b26a3ce0eb3a933a5cc9d8ce742d) ) /* Mask rom labeled 5341-15951-04 U37 VIDEO IMAGE */
	ROM_LOAD16_BYTE( "tm5k_graphic.u41", 0x400000, 0x100000, CRC(c8717fef) SHA1(fbd13321db0f35b7bdf207468f28792a7666bb2e) ) /* Mask rom labeled 5341-15951-09 U41 VIDEO IMAGE */
	ROM_LOAD16_BYTE( "tm5k_graphic.u40", 0x400001, 0x100000, CRC(cff3f962) SHA1(2389d94ffa0eaf2d2f62315822273db031eea033) ) /* Mask rom labeled 5341-15951-08 U40 VIDEO IMAGE */

	ROM_REGION( 0x100000, "oki", 0 ) // Samples
	ROM_LOAD( "tm5k_sound.u8", 0x00000, 0x100000, CRC(c6070a60) SHA1(2dc20bf2217a36374b5a691133ad43f53dbe29ca) ) /* Mask rom labeled 5341-15951-03 U8 VIDEO IMAGE */
ROM_END

ROM_START( tm5kca ) /* California only version */
	ROM_REGION( 0x200000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "tm5kca_v7_10.u51", 0x000000, 0x100000, CRC(7c03708f) SHA1(e1f8198bc03c1f9d2a00662b589fd66fdcc7a2de) ) /* TOUCHMASTER 5000 U51 CALIFORNIA 7.10 (CA. 10-9-98) (tan label) */
	ROM_LOAD16_BYTE( "tm5kca_v7_10.u52", 0x000001, 0x100000, CRC(f1413295) SHA1(ad6b57c590bdae063b0e4a1abcb3b13ee52cd6db) ) /* TOUCHMASTER 5000 U52 CALIFORNIA 7.10 (CA. 10-9-98) (tan label) */

	ROM_REGION( 0x600000, "blitter", 0 )	// Blitter gfx
	ROM_LOAD16_BYTE( "tm5k_graphic.u38", 0x000000, 0x100000, CRC(93038e7c) SHA1(448f69bf51ac992f6b35b471cba9675c67984cd7) ) /* Mask rom labeled 5341-15951-07 U38 VIDEO IMAGE */
	ROM_LOAD16_BYTE( "tm5k_graphic.u36", 0x000001, 0x100000, CRC(5453a44a) SHA1(094439a56336ca933b0b7ede8c057546d1d490b2) ) /* Mask rom labeled 5341-15951-06 U36 VIDEO IMAGE */
	ROM_LOAD16_BYTE( "tm5k_graphic.u39", 0x200000, 0x100000, CRC(1349fdc7) SHA1(5118983be584455320f0d6006133f38e6a8ee0d8) ) /* Mask rom labeled 5341-15951-05 U39 VIDEO IMAGE */
	ROM_LOAD16_BYTE( "tm5k_graphic.u37", 0x200001, 0x100000, CRC(8bcc376c) SHA1(0588f6f96090b26a3ce0eb3a933a5cc9d8ce742d) ) /* Mask rom labeled 5341-15951-04 U37 VIDEO IMAGE */
	ROM_LOAD16_BYTE( "tm5k_graphic.u41", 0x400000, 0x100000, CRC(c8717fef) SHA1(fbd13321db0f35b7bdf207468f28792a7666bb2e) ) /* Mask rom labeled 5341-15951-09 U41 VIDEO IMAGE */
	ROM_LOAD16_BYTE( "tm5k_graphic.u40", 0x400001, 0x100000, CRC(cff3f962) SHA1(2389d94ffa0eaf2d2f62315822273db031eea033) ) /* Mask rom labeled 5341-15951-08 U40 VIDEO IMAGE */

	ROM_REGION( 0x100000, "oki", 0 ) // Samples
	ROM_LOAD( "tm5k_sound.u8", 0x00000, 0x100000, CRC(c6070a60) SHA1(2dc20bf2217a36374b5a691133ad43f53dbe29ca) ) /* Mask rom labeled 5341-15951-03 U8 VIDEO IMAGE */
ROM_END

ROM_START( tm5ka )
	ROM_REGION( 0x200000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "tm5k_v7_01.u51", 0x000000, 0x100000, CRC(3ff68213) SHA1(ec4939899a69f56d2437c806fdd0a5b50e58ec6a) ) /* TOUCHMASTER 5000 U51 DOMESTIC 7.01 (Standard 8-21-98) (pink label) */
	ROM_LOAD16_BYTE( "tm5k_v7_01.u52", 0x000001, 0x100000, CRC(b3de607c) SHA1(4ede5b8c50177d1934a3b93d311a240ef354c450) ) /* TOUCHMASTER 5000 U52 DOMESTIC 7.01 (Standard 8-21-98) (pink label) */

	ROM_REGION( 0x600000, "blitter", 0 )	// Blitter gfx
	ROM_LOAD16_BYTE( "tm5k_graphic.u38", 0x000000, 0x100000, CRC(93038e7c) SHA1(448f69bf51ac992f6b35b471cba9675c67984cd7) ) /* Mask rom labeled 5341-15951-07 U38 VIDEO IMAGE */
	ROM_LOAD16_BYTE( "tm5k_graphic.u36", 0x000001, 0x100000, CRC(5453a44a) SHA1(094439a56336ca933b0b7ede8c057546d1d490b2) ) /* Mask rom labeled 5341-15951-06 U36 VIDEO IMAGE */
	ROM_LOAD16_BYTE( "tm5k_graphic.u39", 0x200000, 0x100000, CRC(1349fdc7) SHA1(5118983be584455320f0d6006133f38e6a8ee0d8) ) /* Mask rom labeled 5341-15951-05 U39 VIDEO IMAGE */
	ROM_LOAD16_BYTE( "tm5k_graphic.u37", 0x200001, 0x100000, CRC(8bcc376c) SHA1(0588f6f96090b26a3ce0eb3a933a5cc9d8ce742d) ) /* Mask rom labeled 5341-15951-04 U37 VIDEO IMAGE */
	ROM_LOAD16_BYTE( "tm5k_graphic.u41", 0x400000, 0x100000, CRC(c8717fef) SHA1(fbd13321db0f35b7bdf207468f28792a7666bb2e) ) /* Mask rom labeled 5341-15951-09 U41 VIDEO IMAGE */
	ROM_LOAD16_BYTE( "tm5k_graphic.u40", 0x400001, 0x100000, CRC(cff3f962) SHA1(2389d94ffa0eaf2d2f62315822273db031eea033) ) /* Mask rom labeled 5341-15951-08 U40 VIDEO IMAGE */

	ROM_REGION( 0x100000, "oki", 0 ) // Samples
	ROM_LOAD( "tm5k_sound.u8", 0x00000, 0x100000, CRC(c6070a60) SHA1(2dc20bf2217a36374b5a691133ad43f53dbe29ca) ) /* Mask rom labeled 5341-15951-03 U8 VIDEO IMAGE */
ROM_END

/***************************************************************************

Touchmaster 7000
by Midway (c) 1999
touchscreen game

All chips are ST M27C801 (some kits/upgrades used mask roms)
---------------------------

Name Board Location        Version               Use                      Checksum
-----------------------------------------------------------------------------------
tm7k_v804.u51              8.04 Game Program & Cpu instructions 321B
tm7k_v804.u52              8.04 Game Program & Cpu instructions 2DED

tm7k_v800.u51              8.00 Game Program & Cpu instructions 82A5
tm7k_v800.u52              8.00 Game Program & Cpu instructions 81E1

tm7k_graphic.u36           8.0  Video Images & Graphics         DB7F (same as TM5K)
tm7k_graphic.u37           8.0  Video Images & Graphics         7461
tm7k_graphic.u38           8.0  Video Images & Graphics         EDCE (same as TM5K)
tm7k_graphic.u39           8.0  Video Images & Graphics         2BB6
tm7k_graphic.u40           8.0  Video Images & Graphics         B103
tm7k_graphic.u41           8.0  Video Images & Graphics         11CA
tm7k_sound.u8              8.0  Audio Program & sounds          F474 (same as TM5K)

J12 DALLAS DS1204V         N/A  Security Key (required for this Version) - Labeled A-21657-005
-----------------------------------------------------------------------------------

***************************************************************************/

ROM_START( tm7k )
	ROM_REGION( 0x200000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "tm7k_v804.u51", 0x000000, 0x100000, CRC(2461af04) SHA1(9cf37c04db0297ff8f9f316fd476d6d5d1c39acf) ) /* TOUCHMASTER 7000 U51 DOMESTIC 8.04 (Standard 06/02/99) (orange label) */
	ROM_LOAD16_BYTE( "tm7k_v804.u52", 0x000001, 0x100000, CRC(5d39fad2) SHA1(85e8d110b88e1099117ab7963eaee47dc86ec7c5) ) /* TOUCHMASTER 7000 U52 DOMESTIC 8.04 (Standard 06/02/99) (orange label) */

	ROM_REGION( 0x600000, "blitter", 0 )	// Blitter gfx
	ROM_LOAD16_BYTE( "tm7k_graphic.u38", 0x000000, 0x100000, CRC(93038e7c) SHA1(448f69bf51ac992f6b35b471cba9675c67984cd7) ) /* Mask rom labeled 5341-16262-07 U38 VIDEO IMAGE */
	ROM_LOAD16_BYTE( "tm7k_graphic.u36", 0x000001, 0x100000, CRC(5453a44a) SHA1(094439a56336ca933b0b7ede8c057546d1d490b2) ) /* Mask rom labeled 5341-16262-06 U36 VIDEO IMAGE */
	ROM_LOAD16_BYTE( "tm7k_graphic.u39", 0x200000, 0x100000, CRC(26af8da8) SHA1(02555b1597a4962f1fd0c3ffc89e5c8338aa3085) ) /* Mask rom labeled 5341-16262-05 U39 VIDEO IMAGE */
	ROM_LOAD16_BYTE( "tm7k_graphic.u37", 0x200001, 0x100000, CRC(9a705043) SHA1(cffb31859544c1c4082be78b3bca5ad9cd0d2a45) ) /* Mask rom labeled 5341-16262-04 U37 VIDEO IMAGE */
	ROM_LOAD16_BYTE( "tm7k_graphic.u41", 0x400000, 0x100000, CRC(99b6edda) SHA1(c0ee2834fdbfbc1159a6d08c45552d4d9c1c4ea4) ) /* Mask rom labeled 5341-16262-09 U41 VIDEO IMAGE */
	ROM_LOAD16_BYTE( "tm7k_graphic.u40", 0x400001, 0x100000, CRC(a3925379) SHA1(74836325ab10466e23105a3b54fc706c0dd5f06c) ) /* Mask rom labeled 5341-16262-08 U40 VIDEO IMAGE */

	ROM_REGION( 0x100000, "oki", 0 ) // Samples
	ROM_LOAD( "tm7k_sound.u8", 0x00000, 0x100000, CRC(c6070a60) SHA1(2dc20bf2217a36374b5a691133ad43f53dbe29ca) ) /* Mask rom labeled 5341-16262-03 U8 SOUND */
ROM_END

ROM_START( tm7ka )
	ROM_REGION( 0x200000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "tm7k_v800.u51", 0x000000, 0x100000, CRC(83ec3da7) SHA1(37fa7183e7acc2eab35ac431d99cbbfe4862979e) ) /* TOUCHMASTER 7000 U51 DOMESTIC 8.00 (Standard 03/26/99) (orange label) */
	ROM_LOAD16_BYTE( "tm7k_v800.u52", 0x000001, 0x100000, CRC(e2004282) SHA1(aa73029f31e2062cabedfcd778db97b314624ae8) ) /* TOUCHMASTER 7000 U52 DOMESTIC 8.00 (Standard 03/26/99) (orange label) */

	ROM_REGION( 0x600000, "blitter", 0 )	// Blitter gfx
	ROM_LOAD16_BYTE( "tm7k_graphic.u38", 0x000000, 0x100000, CRC(93038e7c) SHA1(448f69bf51ac992f6b35b471cba9675c67984cd7) ) /* Labeled GRAPHIC U38  8.0 (orange label) */
	ROM_LOAD16_BYTE( "tm7k_graphic.u36", 0x000001, 0x100000, CRC(5453a44a) SHA1(094439a56336ca933b0b7ede8c057546d1d490b2) ) /* Labeled GRAPHIC U36  8.0 (orange label) */
	ROM_LOAD16_BYTE( "tm7k_graphic.u39", 0x200000, 0x100000, CRC(26af8da8) SHA1(02555b1597a4962f1fd0c3ffc89e5c8338aa3085) ) /* Labeled GRAPHIC U39  8.0 (orange label) */
	ROM_LOAD16_BYTE( "tm7k_graphic.u37", 0x200001, 0x100000, CRC(9a705043) SHA1(cffb31859544c1c4082be78b3bca5ad9cd0d2a45) ) /* Labeled GRAPHIC U37  8.0 (orange label) */
	ROM_LOAD16_BYTE( "tm7k_graphic.u41", 0x400000, 0x100000, CRC(99b6edda) SHA1(c0ee2834fdbfbc1159a6d08c45552d4d9c1c4ea4) ) /* Labeled GRAPHIC U41  8.0 (orange label) */
	ROM_LOAD16_BYTE( "tm7k_graphic.u40", 0x400001, 0x100000, CRC(a3925379) SHA1(74836325ab10466e23105a3b54fc706c0dd5f06c) ) /* Labeled GRAPHIC U40  8.0 (orange label) */

	ROM_REGION( 0x100000, "oki", 0 ) // Samples
	ROM_LOAD( "tm7k_sound.u8", 0x00000, 0x100000, CRC(c6070a60) SHA1(2dc20bf2217a36374b5a691133ad43f53dbe29ca) ) /* Labeled SOUND U8  8.0 (orange label) */
ROM_END

ROM_START( tm7keval ) /* FREEPLAY ONLY / NOT FOR RELEASE / FOR EVALUATION ONLY */
	ROM_REGION( 0x200000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "tm7k_v81x.u51", 0x000000, 0x100000, CRC(57c88287) SHA1(1008bbcd137ede321be4cc2ad22e3982d880471b) ) /* TOUCHMASTER 7000 U51 8.1X (ASI Standard 03/08/99) (white label) */
	ROM_LOAD16_BYTE( "tm7k_v81x.u52", 0x000001, 0x100000, CRC(e45d69bb) SHA1(9e8a42924a9cd573cbd2f3164d0fb468eee7ff51) ) /* TOUCHMASTER 7000 U52 8.1X (ASI Standard 03/08/99) (white label) */

	ROM_REGION( 0x600000, "blitter", 0 )	// Blitter gfx
	ROM_LOAD16_BYTE( "tm7k_graphic.u38", 0x000000, 0x100000, CRC(93038e7c) SHA1(448f69bf51ac992f6b35b471cba9675c67984cd7) ) /* TOUCHMASTER EDCE V8.X U38 (white label) */
	ROM_LOAD16_BYTE( "tm7k_graphic.u36", 0x000001, 0x100000, CRC(5453a44a) SHA1(094439a56336ca933b0b7ede8c057546d1d490b2) ) /* TOUCHMASTER DB7F V8.X U36 (white label) */
	ROM_LOAD16_BYTE( "tm7k_graphic.u39", 0x200000, 0x100000, CRC(26af8da8) SHA1(02555b1597a4962f1fd0c3ffc89e5c8338aa3085) ) /* TOUCHMASTER 2BB6 V8.X U39 (white label) */
	ROM_LOAD16_BYTE( "tm7k_graphic.u37", 0x200001, 0x100000, CRC(9a705043) SHA1(cffb31859544c1c4082be78b3bca5ad9cd0d2a45) ) /* TOUCHMASTER 7461 V8.X U37 (white label) */
	ROM_LOAD16_BYTE( "tm7k_v8x.u41",     0x400000, 0x100000, CRC(d15757f0) SHA1(d09a83dc00812a4208d5684e8cfba1f5d1e3f097) ) /* TOUCHMASTER 335E V8.X U41 (white label) */
	ROM_LOAD16_BYTE( "tm7k_v8x.u40",     0x400001, 0x100000, CRC(d3596f63) SHA1(a2474bc1a73b5782f8169b0529a535269ea82440) ) /* TOUCHMASTER 7987 V8.X U40 (white label) */

	ROM_REGION( 0x100000, "oki", 0 ) // Samples
	ROM_LOAD( "tm7k_sound.u8", 0x00000, 0x100000, CRC(c6070a60) SHA1(2dc20bf2217a36374b5a691133ad43f53dbe29ca) ) /* Labeled SOUND U8  8.0 (orange label) */
ROM_END

/***************************************************************************

Touchmaster 8000
by Midway (c) 2000
touchscreen game

All chips are ST M27C801 (some kits/upgrades used mask roms)
---------------------------

Name Board Location        Version               Use                      Checksum
-----------------------------------------------------------------------------------
tm8k_v904.u51              9.04 Game Program & Cpu instructions D40F
tm8k_v904.u52              9.04 Game Program & Cpu instructions 53B2

tm8k_graphic.u36           9.0  Video Images & Graphics         AD8D
tm8k_graphic.u37           9.0  Video Images & Graphics         AF83
tm8k_graphic.u38           9.0  Video Images & Graphics         6BCF
tm8k_graphic.u39           9.0  Video Images & Graphics         C8A6
tm8k_graphic.u40           9.0  Video Images & Graphics         B8C7
tm8k_graphic.u41           9.0  Video Images & Graphics         EF93
tm8k_sound.u8              9.0  Audio Program & sounds          F474 (same as TM5K & TM7K)

J12 DALLAS DS1204V         N/A  Security Key (required for this Version) - Labeled A-21657-007
-----------------------------------------------------------------------------------

***************************************************************************/

/* According to the Distributor notice from Midway, the only thing updated for v9.04 was the "Power Cell" game to 'improve the performance'. */
ROM_START( tm8k )
	ROM_REGION( 0x200000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "tm8k_v904.u51", 0x000000, 0x100000, CRC(28864ec8) SHA1(e703f9ee350dd915102e784bbd04445a95b7d0a5) ) /* TOUCHMASTER 8000 U51 DOMESTIC 9.04 (Standard 04/25/00) */
	ROM_LOAD16_BYTE( "tm8k_v904.u52", 0x000001, 0x100000, CRC(c123eec2) SHA1(3e9c84755b18a4fd900068f385ee47107771391d) ) /* TOUCHMASTER 8000 U52 DOMESTIC 9.04 (Standard 04/25/00) */

	ROM_REGION( 0x600000, "blitter", 0 )	// Blitter gfx
	ROM_LOAD16_BYTE( "tm8k_graphic.u38", 0x000000, 0x100000, CRC(2a971d46) SHA1(6ca4067e9fa40053df415e670b2e853915319dbb) ) /* Mask rom labeled 5341-16513-07 U38 VIDEO IMAGE */
	ROM_LOAD16_BYTE( "tm8k_graphic.u36", 0x000001, 0x100000, CRC(3bde285e) SHA1(87bf60034665542fb0240b7479adfffb7ba9fad7) ) /* Mask rom labeled 5341-16513-06 U36 VIDEO IMAGE */
	ROM_LOAD16_BYTE( "tm8k_graphic.u39", 0x200000, 0x100000, CRC(58c6c1d8) SHA1(cc11863c4ea46bde7ea4775075f4328be6d6c6d1) ) /* Mask rom labeled 5341-16513-05 U39 VIDEO IMAGE */
	ROM_LOAD16_BYTE( "tm8k_graphic.u37", 0x200001, 0x100000, CRC(c0992f7a) SHA1(e4e1ef2414f2f0a784c775f39123122c08950403) ) /* Mask rom labeled 5341-16513-04 U37 VIDEO IMAGE */
	ROM_LOAD16_BYTE( "tm8k_graphic.u41", 0x400000, 0x100000, CRC(d8bdb82e) SHA1(9bdee261591ccff8a57c5454644f84f8992f614f) ) /* Mask rom labeled 5341-16513-09 U41 VIDEO IMAGE */
	ROM_LOAD16_BYTE( "tm8k_graphic.u40", 0x400001, 0x100000, CRC(0c3d6347) SHA1(7ef19018c180abf412a8ff9f278b00c2b4321cc2) ) /* Mask rom labeled 5341-16513-08 U40 VIDEO IMAGE */

	ROM_REGION( 0x100000, "oki", 0 ) // Samples
	ROM_LOAD( "tm8k_sound.u8", 0x00000, 0x100000, CRC(c6070a60) SHA1(2dc20bf2217a36374b5a691133ad43f53dbe29ca) ) /* Mask rom labeled 5341-16513-03 U8 SOUND */
ROM_END

ROM_START( tm8k902 )
	ROM_REGION( 0x200000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "tm8k_v902.u51", 0x000000, 0x100000, CRC(c0d95b2c) SHA1(2240d969047ada856ec2fac6cdf72db722753a6d) ) /* TOUCHMASTER 8000 U51 DOMESTIC 9.02 (Standard 03/17/00) */
	ROM_LOAD16_BYTE( "tm8k_v902.u52", 0x000001, 0x100000, CRC(5194c1b5) SHA1(28a91d12a022927bfe96afd62b30b268c0ced3ea) ) /* TOUCHMASTER 8000 U52 DOMESTIC 9.02 (Standard 03/17/00) */

	ROM_REGION( 0x600000, "blitter", 0 )	// Blitter gfx
	ROM_LOAD16_BYTE( "tm8k_graphic.u38", 0x000000, 0x100000, CRC(2a971d46) SHA1(6ca4067e9fa40053df415e670b2e853915319dbb) ) /* Mask rom labeled 5341-16513-07 U38 VIDEO IMAGE */
	ROM_LOAD16_BYTE( "tm8k_graphic.u36", 0x000001, 0x100000, CRC(3bde285e) SHA1(87bf60034665542fb0240b7479adfffb7ba9fad7) ) /* Mask rom labeled 5341-16513-06 U36 VIDEO IMAGE */
	ROM_LOAD16_BYTE( "tm8k_graphic.u39", 0x200000, 0x100000, CRC(58c6c1d8) SHA1(cc11863c4ea46bde7ea4775075f4328be6d6c6d1) ) /* Mask rom labeled 5341-16513-05 U39 VIDEO IMAGE */
	ROM_LOAD16_BYTE( "tm8k_graphic.u37", 0x200001, 0x100000, CRC(c0992f7a) SHA1(e4e1ef2414f2f0a784c775f39123122c08950403) ) /* Mask rom labeled 5341-16513-04 U37 VIDEO IMAGE */
	ROM_LOAD16_BYTE( "tm8k_graphic.u41", 0x400000, 0x100000, CRC(d8bdb82e) SHA1(9bdee261591ccff8a57c5454644f84f8992f614f) ) /* Mask rom labeled 5341-16513-09 U41 VIDEO IMAGE */
	ROM_LOAD16_BYTE( "tm8k_graphic.u40", 0x400001, 0x100000, CRC(0c3d6347) SHA1(7ef19018c180abf412a8ff9f278b00c2b4321cc2) ) /* Mask rom labeled 5341-16513-08 U40 VIDEO IMAGE */

	ROM_REGION( 0x100000, "oki", 0 ) // Samples
	ROM_LOAD( "tm8k_sound.u8", 0x00000, 0x100000, CRC(c6070a60) SHA1(2dc20bf2217a36374b5a691133ad43f53dbe29ca) ) /* Mask rom labeled 5341-16513-03 U8 SOUND */
ROM_END

/***************************************************************************

Galaxy Games BIOS v. 1.90

This is a multi-game cocktail cabinet released in 1998.  Namco seems to have
made some cartridges for it (or at least licensed their IP).

Trackball-based. 'BIOS' has 7 built-in games. There are two LEDs on the PCB.

More information here : http://www.cesgames.com/museum/galaxy/index.html

----

Board silkscreened  237-0211-00
                    REV.-D

Cartridge based mother board
Holds up to 4 cartridges
Chips labeled
    GALAXY U1 V1.90 12/1/98
    GALAXY U2 V1.90 12/1/98

Motorola MC68HC000FN12
24 MHz oscillator
Xilinx XC5206
Xilinx XC5202
BT481AKPJ110 (Palette RAMDAC)
NKK N341024SJ-15    x8  (128kB RAM)
OKI M6295 8092352-2

PAL16V8H-15 @ U24   Blue dot on it
PAL16V8H-15 @ U25   Yellow dot on it
PAL16V8H-15 @ U26   Red dot on it
PAL16V8H-15 @ U27   Green dot on it
PAL16V8H-15 @ U45   red dot on it

***************************************************************************/

#define GALGAMES_BIOS_ROMS \
	ROM_SYSTEM_BIOS( 0, "0",   "v1.90" ) \
	ROMX_LOAD( "galaxy.u2", 0x000000, 0x100000, CRC(e51ff184) SHA1(aaa795f2c15ec29b3ceeb5c917b643db0dbb7083), ROM_SKIP(1) | ROM_BIOS(1) ) \
	ROMX_LOAD( "galaxy.u1", 0x000001, 0x100000, CRC(c6d7bc6d) SHA1(93c032f9aa38cbbdda59a8a25ff9f38f7ad9c760), ROM_SKIP(1) | ROM_BIOS(1) )

#define GALGAMES_MB_PALS \
	ROM_REGION( 0xa00, "pals", 0) \
	ROM_LOAD( "16v8h-blue.u24",    0x000, 0x117, NO_DUMP ) \
	ROM_LOAD( "16v8h-yellow.u25",  0x200, 0x117, NO_DUMP ) \
	ROM_LOAD( "16v8h-magenta.u26", 0x400, 0x117, NO_DUMP ) \
	ROM_LOAD( "16v8h-green.u27",   0x600, 0x117, NO_DUMP ) \
	ROM_LOAD( "16v8h-red.u45",     0x800, 0x117, NO_DUMP )

ROM_START( galgbios )
	ROM_REGION( 0x200000, "maincpu", 0 )
	GALGAMES_BIOS_ROMS

	ROM_REGION( 0x200000, "blitter", 0 )
	ROM_COPY( "maincpu", 0, 0, 0x200000 )

	ROM_REGION( 0x40000, "oki", ROMREGION_ERASE )
	// RAM, filled by the 68000 and fed to the OKI

	GALGAMES_MB_PALS
ROM_END

/***************************************************************************

Galaxy Games StarPak 2

NAMCO 307 Cartridge, has surface mount Flash chips in it:

.u1 am29f800BB
.u2 am29f800BB
.u4 pic 12c508

Board silkscreened  237-0209-00
                    REV.-C

***************************************************************************/

ROM_START( galgame2 )
	ROM_REGION( 0x200000 * 2, "maincpu", 0 )
	GALGAMES_BIOS_ROMS
	ROM_LOAD16_BYTE( "namco.u2",  0x200000, 0x100000, CRC(f43c0c54) SHA1(4a13946c3d173b0e4ab25b01849574fa3302b417) )
	ROM_LOAD16_BYTE( "namco.u1",  0x200001, 0x100000, CRC(b8c34a8b) SHA1(40d3b35f573d2bd2ae1c7d876c55fc436864fa3f) )

	ROM_REGION( 0x9db, "pic", 0 )
	ROM_LOAD( "namco.u4", 0x000, 0x9db, CRC(63ec1224) SHA1(2966b2aa431e6b8d46cbba18a6fc4de7395caa04) )

	ROM_REGION( 0x200000 * 2, "blitter", 0 )
	ROM_COPY( "maincpu", 0, 0, 0x200000 * 2 )

	ROM_REGION( 0x40000, "oki", ROMREGION_ERASE )
	// RAM, filled by the 68000 and fed to the OKI

	GALGAMES_MB_PALS
ROM_END


static DRIVER_INIT( tm4k )
{
	UINT16 *ROM = (UINT16 *)memory_region( machine, "maincpu" );

	// protection
	ROM[0x834ce/2] = 0x4e75;

	ROM[0x8349c/2] = 0x6002;
	ROM[0x834c4/2] = 0x6002;
/*

Protection resembles that of tm5k rather than tm4ka:

 83470: addi.w  #$384, D0       0640 0384
 83474: move.w  D0, $207a86.l   33C0 0020 7A86

*/

}


static DRIVER_INIT( tm4ka )
{
	UINT16 *ROM = (UINT16 *)memory_region( machine, "maincpu" );

	// protection
	ROM[0x83476/2] = 0x4e75;

	ROM[0x8342C/2] = 0x601a;
	ROM[0x8346C/2] = 0x6002;
/*
Protection starts:

 8341C: addi.w  #$384, D0       0640 0384
 83420: move.w  D0, $207a86.l   33C0 0020 7A86
 83426: btst    #$7, ($1,A5)    082D 0007 0001
 8342C: beq     $83448          671A           <-- First patch goes here

*/

}

static DRIVER_INIT( tm5k )
{
	UINT16 *ROM = (UINT16 *)memory_region( machine, "maincpu" );

	// protection
	ROM[0x96002/2] = 0x4e75;

	ROM[0x95fd0/2] = 0x6002;
	ROM[0x95ff8/2] = 0x6002;
/*
Protection starts:

 95FBE: addi.w  #$384, D0       0640 0384
 95FC2: move.w  D0, $207e9a.l   33C0 0020 7E9A
 95FC8: moveq   #$f, D0         700F
 95FCA: and.w   (A4), D0        C054
 95FCC: cmpi.w  #$3, D0         0C40 0003
 95FD0: bcs     $95fd4          6502          <-- First patch goes here

*/

}

static DRIVER_INIT( tm5kca )
{
	UINT16 *ROM = (UINT16 *)memory_region( machine, "maincpu" );

	// protection
	ROM[0x95ffe/2] = 0x4e75;

	ROM[0x95fcc/2] = 0x6002;
	ROM[0x95ff4/2] = 0x6002;

}

static DRIVER_INIT( tm5ka )
{
	UINT16 *ROM = (UINT16 *)memory_region( machine, "maincpu" );

	// protection
	ROM[0x96b30/2] = 0x4e75;

	ROM[0x96ae6/2] = 0x601a;
	ROM[0x96b26/2] = 0x6002;
/*
Protection starts:

 96AD6: addi.w  #$384, D0       0640 0384
 96ADA: move.w  D0, $207cb4.l   33C0 0020 7CB4
 96AE0: btst    #$7, ($1,A5)    082D 0007 0001
 96AE6: beq     $96b02          671A           <-- First patch goes here

*/

}

static DRIVER_INIT( tm7k )
{
	UINT16 *ROM = (UINT16 *)memory_region( machine, "maincpu" );

	// protection
	ROM[0x81730/2] = 0x4e75;

	ROM[0x81700/2] = 0x6004;
	ROM[0x81728/2] = 0x6002;
/*
Protection starts:

 816ee: addi.w  #$76c, D0       0640 076C
 816f2: move.w  D0, $20718c.l   33C0 0020 718C
 816f8: moveq   #$f, D0         700F
 816fa: and.w   (A4), D0        C054
 816fc: cmpi.w  #$3, D0         0C40 0003
 81700: bcs     $81706          6504          <-- First patch goes here

*/

}

static DRIVER_INIT( tm7ka )
{
	UINT16 *ROM = (UINT16 *)memory_region( machine, "maincpu" );

	// protection
	ROM[0x81594/2] = 0x4e75;

	ROM[0x81564/2] = 0x6004;
	ROM[0x8158c/2] = 0x6002;
/*
Protection starts:

 81552: addi.w  #$76c, D0       0640 076C
 81556: move.w  D0, $207104.l   33C0 0020 7104
 8155C: moveq   #$f, D0         700F
 8155E: and.w   (A4), D0        C054
 81560: cmpi.w  #$3, D0         0C40 0003
 81564: bcs     $8156A          6504          <-- First patch goes here

*/

}

static DRIVER_INIT( tm7keval ) /* kit came with a security key labeled A-21657-004, which is a TM5000 key */
{
	UINT16 *ROM = (UINT16 *)memory_region( machine, "maincpu" );

	// protection
	ROM[0x8949e/2] = 0x4e75;

	ROM[0x8946c/2] = 0x6002;
	ROM[0x89494/2] = 0x6002;
/*
Protection starts:

 8945A: addi.w  #$384, D0       0640 0384
 8945E: move.w  D0, $2074E4.l   33C0 0020 74E4
 89464: moveq   #$f, D0         700F
 89466: and.w   (A4), D0        C054
 89468: cmpi.w  #$3, D0         0C40 0003
 8946C: bcs     $89470          6502          <-- First patch goes here

*/

}

static DRIVER_INIT( tm8k )
{
	UINT16 *ROM = (UINT16 *)memory_region( machine, "maincpu" );

	// protection
	ROM[0x78b70/2] = 0x4e75;

	ROM[0x78b40/2] = 0x6004;
	ROM[0x78b68/2] = 0x6002;
/*
Protection starts:

 78B2E: addi.w  #$76c, D0       0640 076C
 78B32: move.w  D0, $206FC2.l   33C0 0020 6FC2
 78B38: moveq   #$f, D0         700F
 78B3A: and.w   (A4), D0        C054
 78B3C: cmpi.w  #$3, D0         0C40 0003
 78B40: bcs     $78B46          6504          <-- First patch goes here

*/

}

static DRIVER_INIT( galgames )
{
	UINT8 *ROM	=	memory_region(machine, "maincpu");
	int cart;

	// RAM bank at 0x000000-0x03ffff and 0x200000-0x23ffff
	// ROM bank at 0x000000-0x1fffff and 0x200000-0x3fffff (bios)

	memory_configure_bank(machine, GALGAMES_BANK_000000_R, GALGAMES_RAM,  1, galgames_ram, 0x40000);
	memory_configure_bank(machine, GALGAMES_BANK_000000_R, GALGAMES_ROM0, 1, ROM+0x000000, 0x40000);

	memory_configure_bank(machine, GALGAMES_BANK_000000_W, GALGAMES_RAM,  1, galgames_ram, 0x40000);

	memory_configure_bank(machine, GALGAMES_BANK_200000_R, GALGAMES_RAM,  1, galgames_ram, 0x40000);
	memory_configure_bank(machine, GALGAMES_BANK_200000_R, GALGAMES_ROM0, 1, ROM+0x000000, 0x40000);

	memory_configure_bank(machine, GALGAMES_BANK_200000_W, GALGAMES_RAM,  1, galgames_ram, 0x40000);

	memory_configure_bank(machine, GALGAMES_BANK_240000_R, GALGAMES_ROM0, 1, ROM+0x040000, 0x1c0000);

	// More ROM banks at 0x200000-0x3fffff (carts)

	for (cart = 1; cart <= 4; cart++)
	{
		UINT8 *CART = memory_region(machine, "maincpu");

		if  (0x200000 * (cart+1) <= memory_region_length(machine, "maincpu"))
			CART += 0x200000 * cart;

		memory_configure_bank(machine, GALGAMES_BANK_200000_R, GALGAMES_ROM0+cart, 1, CART,          0x40000);
		memory_configure_bank(machine, GALGAMES_BANK_240000_R, GALGAMES_ROM0+cart, 1, CART+0x040000, 0x1c0000);
	}
}

static DRIVER_INIT( galgame2 )
{
	UINT16 *ROM = (UINT16 *)memory_region( machine, "maincpu" );

	// Patch BIOS to see the game code as first cartridge (until the PIC therein is emulated)
	ROM[0x118da/2] = 0x4a06;
	ROM[0x118dc/2] = 0x6704;
	ROM[0x118de/2] = 0x7000;
	ROM[0x118e0/2] = 0x6002;
	ROM[0x118e2/2] = 0x7001;
	ROM[0x118e4/2] = 0x4e71;
	ROM[0x118e6/2] = 0x4e71;
	ROM[0x118e8/2] = 0x4e71;

	// Cartdridge check on game selection screen
	ROM[0x12da0/2] = 0x4e71;

	DRIVER_INIT_CALL( galgames );
}


GAME( 1996, tm,       0,        tm,       tm,       0,        ROT0, "CES Inc., Midway Games Inc.",             "Touchmaster (v3.00 Euro)",            0 )
GAME( 1996, tmdo,     tm,       tm,       tm,       0,        ROT0, "CES Inc., Midway Games Inc.",             "Touchmaster (v2.2-01 Standard)",      0 )
GAME( 1996, tm2k,     0,        tm3k,     tmaster,  0,        ROT0, "Midway Games Inc.",                       "Touchmaster 2000 (v4.02 Standard)",   0 )
GAME( 1996, tm2ka,    tm2k,     tm3k,     tmaster,  0,        ROT0, "Midway Games Inc.",                       "Touchmaster 2000 (v4.00 Standard)",   0 )
GAME( 1997, tm3k,     0,        tm3k,     tmaster,  0,        ROT0, "Midway Games Inc.",                       "Touchmaster 3000 (v5.02 Standard)",   0 )
GAME( 1997, tm3ka,    tm3k,     tm3k,     tmaster,  0,        ROT0, "Midway Games Inc.",                       "Touchmaster 3000 (v5.01 Standard)",   0 )
GAME( 1998, tm4k,     0,        tm3k,     tmaster,  tm4k,     ROT0, "Midway Games Inc.",                       "Touchmaster 4000 (v6.03 Standard)",   0 )
GAME( 1998, tm4ka,    tm4k,     tm3k,     tmaster,  tm4ka,    ROT0, "Midway Games Inc.",                       "Touchmaster 4000 (v6.02 Standard)",   0 )
GAME( 1998, tm5k,     0,        tm3k,     tmaster,  tm5k,     ROT0, "Midway Games Inc.",                       "Touchmaster 5000 (v7.10 Standard)",   0 )
GAME( 1998, tm5kca,   tm5k,     tm3k,     tmaster,  tm5kca,   ROT0, "Midway Games Inc.",                       "Touchmaster 5000 (v7.10 California)", 0 )
GAME( 1998, tm5ka,    tm5k,     tm3k,     tmaster,  tm5ka,    ROT0, "Midway Games Inc.",                       "Touchmaster 5000 (v7.01 Standard)",   0 )
GAME( 1999, tm7k,     0,        tm3k,     tmaster,  tm7k,     ROT0, "Midway Games Inc.",                       "Touchmaster 7000 (v8.04 Standard)",   0 )
GAME( 1999, tm7ka,    tm7k,     tm3k,     tmaster,  tm7ka,    ROT0, "Midway Games Inc.",                       "Touchmaster 7000 (v8.00 Standard)",   0 )
GAME( 1999, tm7keval, tm7k,     tm3k,     tmaster,  tm7keval, ROT0, "Midway Games Inc.",                       "Touchmaster 7000 (v8.1X Evaluation)", 0 )
GAME( 2000, tm8k,     0,        tm3k,     tmaster,  tm8k,     ROT0, "Midway Games Inc.",                       "Touchmaster 8000 (v9.04 Standard)",   0 )
GAME( 2000, tm8k902,  tm8k,     tm3k,     tmaster,  tm8k,     ROT0, "Midway Games Inc.",                       "Touchmaster 8000 (v9.02 Standard)",   0 )
GAME( 1998, galgbios, 0,        galgames, galgames, galgames, ROT0, "Creative Electronics & Software",         "Galaxy Games (BIOS v1.90)",           GAME_IS_BIOS_ROOT )
GAME( 1998, galgame2, galgbios, galgame2, galgames, galgame2, ROT0, "Creative Electronics & Software / Namco", "Galaxy Games StarPak 2",              0 )
