/***************************************************************************

    Incredible Technologies/Strata system
    (8-bit blitter variant)

****************************************************************************

    The games run in one of two modes, either 4bpp mode with a latched
    palette nibble, or straight 8bpp.

    arligntn:   4bpp mode, dp = 00/80, draw to 0xxxx/2xxxx
    hstennis:   4bpp mode, dp = 00/80, draw to 0xxxx/2xxxx
    neckneck:   4bpp mode, dp = 00/80, draw to 0xxxx/2xxxx
    ninclown:   4bpp mode, dp = 00/80, draw to 0xxxx/2xxxx
    peggle:     4bpp mode, dp = 00/80, draw to 0xxxx/2xxxx
    rimrockn:   4bpp mode, dp = 00/80, draw to 0xxxx/2xxxx

    dynobop:    8bpp mode, dp = 00/FF, draw to 2xxxx/0xxxx
    pokrdice:   8bpp mode, dp = 00/FF, draw to 2xxxx/0xxxx
    slikshot:   8bpp mode, dp = 00/FF, draw to 2xxxx/0xxxx

    gtg:        8bpp mode, dp = C0
    stratab:    8bpp mode, dp = C0, always draws to 2xxxx
    wfortune:   8bpp mode, dp = C0, always draws to 2xxxx

****************************************************************************

    8-bit blitter data format:

    +00  hhhhhhhh   MSB of 16-bit source address
    +01  llllllll   LSB of 16-bit source address
    +02  ---t----   Transparent: when set, don't write 0 nibbles
         ----r---   RLE: when set, source data is RLE compressed
         -----y--   Y-flip: when set, draw up instead of down
         ------x-   X-flip: when set, draw right instead of left
         -------s   Shift: when set, offset drawing by one nibble
    +03  s-------   Blit status: when set, blit is in progress
    +03  --------   Blit start: a write here starts the blit
    +04  wwwwwwww   Width of blit, in bytes
    +05  hhhhhhhh   Height of blit, in rows
    +06  mmmmmmmm   Blit mask, applied to each source pixel
    +07  ?-------   Set by ninclown at startup
         -4------   Set by games that use 4bpp latched output
         --?-----   Set by gtg, gtg2, peggle, pokrdice, stratab
         ---l----   Video status LED
         ----?---   Set by gtg2, ninclown, peggle, stratab
         ------?-   Set by arligntn, neckneck, gtg, gtg2, ninclown, peggle, pokrdice, rimrockn, stratab
    +08  xxxxxxxx   Number of pixels to skip before drawing each row
    +09  yyyyyyyy   Number of rows to draw (the first height - y rows are skipped)
    +0A  eeeeeeee   Number of pixels to skip at the end of each row
    +0B  ssssssss   Number of rows to skip at the end of drawing

****************************************************************************

    Blitter timing

    Times are in 2MHz NOPs (@ 2 cycles/NOP) after issuing a blit start
    command:

      1x1 = 1-2     1x  1 = 1-2
      2x1 = 2       1x  2 = 2
      3x1 = 2       1x  3 = 2
      4x1 = 2       1x  4 = 2
      5x1 = 3       1x  5 = 3
      6x1 = 3       1x  6 = 3
      7x1 = 3-4     1x  7 = 3
      8x1 = 4       1x  8 = 4
      9x1 = 4       1x  9 = 4
     10x1 = 4       1x 10 = 4-5
     11x1 = 5       1x 11 = 5
     12x1 = 5       1x 12 = 5
     13x1 = 5-6     1x 13 = 5
     14x1 = 6       1x 14 = 6
     15x1 = 6       1x 15 = 6
     16x1 = 6-7     1x 16 = 6
     24x1 = 9-10    1x 24 = 9
     32x1 = 12      1x 32 = 12
     48x1 = 17      1x 48 = 17
     64x1 = 23      1x 64 = 22
     96x1 = 33      1x 96 = 33
    128x1 = 44      1x128 = 44
    192x1 = 65      1x192 = 65
    240x1 = 81-82   1x240 = 81-82

    Times are not affected by transparency or RLE settings at all.
    Times are slightly variable in nature in my tests (within 1 NOP).

***************************************************************************/

#include "emu.h"
#include "video/tms34061.h"
#include "video/tlc34076.h"
#include "cpu/m6809/m6809.h"
#include "includes/itech8.h"


/*************************************
 *
 *  Debugging
 *
 *************************************/

#define FULL_LOGGING			0
#define BLIT_LOGGING			0



/*************************************
 *
 *  Blitter constants
 *
 *************************************/

#define BLITTER_ADDRHI			blitter_data[0]
#define BLITTER_ADDRLO			blitter_data[1]
#define BLITTER_FLAGS			blitter_data[2]
#define BLITTER_STATUS			blitter_data[3]
#define BLITTER_WIDTH			blitter_data[4]
#define BLITTER_HEIGHT			blitter_data[5]
#define BLITTER_MASK			blitter_data[6]
#define BLITTER_OUTPUT			blitter_data[7]
#define BLITTER_XSTART			blitter_data[8]
#define BLITTER_YCOUNT			blitter_data[9]
#define BLITTER_XSTOP			blitter_data[10]
#define BLITTER_YSKIP			blitter_data[11]

#define BLITFLAG_SHIFT			0x01
#define BLITFLAG_XFLIP			0x02
#define BLITFLAG_YFLIP			0x04
#define BLITFLAG_RLE			0x08
#define BLITFLAG_TRANSPARENT	0x10



/*************************************
 *
 *  Global variables
 *
 *************************************/

UINT8 *itech8_grom_bank;

static UINT8 blitter_data[16];
static UINT8 blit_in_progress;

static UINT8 page_select;

static offs_t fetch_offset;
static UINT8 fetch_rle_count;
static UINT8 fetch_rle_value;
static UINT8 fetch_rle_literal;

static struct tms34061_display tms_state;
static UINT8 *grom_base;
static UINT32 grom_size;

static UINT8 grmatch_palcontrol;
static UINT8 grmatch_xscroll;
static rgb_t grmatch_palette[2][16];



/*************************************
 *
 *  TMS34061 interfacing
 *
 *************************************/

static void generate_interrupt(running_machine *machine, int state)
{
	itech8_update_interrupts(machine, -1, state, -1);

	if (FULL_LOGGING && state) logerror("------------ DISPLAY INT (%d) --------------\n", machine->primary_screen->vpos());
}


static const struct tms34061_interface tms34061intf =
{
	"screen",				/* the screen we are acting on */
	8,						/* VRAM address is (row << rowshift) | col */
	0x40000,				/* size of video RAM */
	generate_interrupt		/* interrupt gen callback */
};



/*************************************
 *
 *  Video start
 *
 *************************************/

VIDEO_START( itech8 )
{
	/* initialize TMS34061 emulation */
    tms34061_start(machine, &tms34061intf);

	/* get the TMS34061 display state */
	tms34061_get_display_state(&tms_state);

	/* reset statics */
	page_select = 0xc0;

	/* fetch the GROM base */
	grom_base = memory_region(machine, "grom");
	grom_size = memory_region_length(machine, "grom");
}



/*************************************
 *
 *  Palette I/O
 *
 *************************************/

WRITE8_HANDLER( itech8_palette_w )
{
	tlc34076_w(space, offset/2, data);
}



/*************************************
 *
 *  Paging control
 *
 *************************************/

WRITE8_HANDLER( itech8_page_w )
{
	space->machine->primary_screen->update_partial(space->machine->primary_screen->vpos());
	logerror("%04x:display_page = %02X (%d)\n", cpu_get_pc(space->cpu), data, space->machine->primary_screen->vpos());
	page_select = data;
}




/*************************************
 *
 *  Blitter data fetchers
 *
 *************************************/

INLINE UINT8 fetch_next_raw(void)
{
	return grom_base[fetch_offset++ % grom_size];
}


INLINE void consume_raw(int count)
{
	fetch_offset += count;
}


INLINE UINT8 fetch_next_rle(void)
{
	if (fetch_rle_count == 0)
	{
		fetch_rle_count = grom_base[fetch_offset++ % grom_size];
		fetch_rle_literal = fetch_rle_count & 0x80;
		fetch_rle_count &= 0x7f;

		if (!fetch_rle_literal)
			fetch_rle_value = grom_base[fetch_offset++ % grom_size];
	}

	fetch_rle_count--;
	if (fetch_rle_literal)
		fetch_rle_value = grom_base[fetch_offset++ % grom_size];

	return fetch_rle_value;
}


INLINE void consume_rle(int count)
{
	while (count)
	{
		int num_to_consume;

		if (fetch_rle_count == 0)
		{
			fetch_rle_count = grom_base[fetch_offset++ % grom_size];
			fetch_rle_literal = fetch_rle_count & 0x80;
			fetch_rle_count &= 0x7f;

			if (!fetch_rle_literal)
				fetch_rle_value = grom_base[fetch_offset++ % grom_size];
		}

		num_to_consume = (count < fetch_rle_count) ? count : fetch_rle_count;
		count -= num_to_consume;

		fetch_rle_count -= num_to_consume;
		if (fetch_rle_literal)
			fetch_offset += num_to_consume;
	}
}



/*************************************
 *
 *  Core blitter
 *
 *************************************/

static void perform_blit(const address_space *space)
{
	offs_t addr = tms_state.regs[TMS34061_XYADDRESS] | ((tms_state.regs[TMS34061_XYOFFSET] & 0x300) << 8);
	UINT8 shift = (BLITTER_FLAGS & BLITFLAG_SHIFT) ? 4 : 0;
	int transparent = (BLITTER_FLAGS & BLITFLAG_TRANSPARENT);
	int ydir = (BLITTER_FLAGS & BLITFLAG_YFLIP) ? -1 : 1;
	int xdir = (BLITTER_FLAGS & BLITFLAG_XFLIP) ? -1 : 1;
	int xflip = (BLITTER_FLAGS & BLITFLAG_XFLIP);
	int rle = (BLITTER_FLAGS & BLITFLAG_RLE);
	int color = tms34061_latch_r(space, 0);
	int width = BLITTER_WIDTH;
	int height = BLITTER_HEIGHT;
	UINT8 transmaskhi, transmasklo;
	UINT8 mask = BLITTER_MASK;
	UINT8 skip[3];
	int x, y;

	/* debugging */
	if (FULL_LOGGING)
		logerror("Blit: scan=%d  src=%06x @ (%05x) for %dx%d ... flags=%02x\n",
				space->machine->primary_screen->vpos(),
				(*itech8_grom_bank << 16) | (BLITTER_ADDRHI << 8) | BLITTER_ADDRLO,
				tms_state.regs[TMS34061_XYADDRESS] | ((tms_state.regs[TMS34061_XYOFFSET] & 0x300) << 8),
				BLITTER_WIDTH, BLITTER_HEIGHT, BLITTER_FLAGS);

	/* initialize the fetcher */
	fetch_offset = (*itech8_grom_bank << 16) | (BLITTER_ADDRHI << 8) | BLITTER_ADDRLO;
	fetch_rle_count = 0;

	/* RLE starts with a couple of extra 0's */
	if (rle)
		fetch_offset += 2;

	/* select 4-bit versus 8-bit transparency */
	if (BLITTER_OUTPUT & 0x40)
		transmaskhi = 0xf0, transmasklo = 0x0f;
	else
		transmaskhi = transmasklo = 0xff;

	/* compute horiz skip counts */
	skip[0] = BLITTER_XSTART;
	skip[1] = (width <= BLITTER_XSTOP) ? 0 : width - 1 - BLITTER_XSTOP;
	if (xdir == -1) { int temp = skip[0]; skip[0] = skip[1]; skip[1] = temp; }
	width -= skip[0] + skip[1];

	/* compute vertical skip counts */
	if (ydir == 1)
	{
		skip[2] = (height <= BLITTER_YCOUNT) ? 0 : height - BLITTER_YCOUNT;
		if (BLITTER_YSKIP > 1) height -= BLITTER_YSKIP - 1;
	}
	else
	{
		skip[2] = (height <= BLITTER_YSKIP) ? 0 : height - BLITTER_YSKIP;
		if (BLITTER_YCOUNT > 1) height -= BLITTER_YCOUNT - 1;
	}

	/* skip top */
	for (y = 0; y < skip[2]; y++)
	{
		/* skip src and dest */
		addr += xdir * (width + skip[0] + skip[1]);
		if (rle)
			consume_rle(width + skip[0] + skip[1]);
		else
			consume_raw(width + skip[0] + skip[1]);

		/* back up one and reverse directions */
		addr -= xdir;
		addr += ydir * 256;
		addr &= 0x3ffff;
		xdir = -xdir;
	}

	/* loop over height */
	for (y = skip[2]; y < height; y++)
	{
		/* skip left */
		addr += xdir * skip[y & 1];
		if (rle)
			consume_rle(skip[y & 1]);
		else
			consume_raw(skip[y & 1]);

		/* loop over width */
		for (x = 0; x < width; x++)
		{
			UINT8 pix = rle ? fetch_next_rle() : fetch_next_raw();

			/* swap pixels for X flip in 4bpp mode */
			if (xflip && transmaskhi != 0xff)
				pix = (pix >> 4) | (pix << 4);
			pix &= mask;

			/* draw upper pixel */
			if (!transparent || (pix & transmaskhi))
			{
				tms_state.vram[addr] = (tms_state.vram[addr] & (0x0f << shift)) | ((pix & 0xf0) >> shift);
				tms_state.latchram[addr] = (tms_state.latchram[addr] & (0x0f << shift)) | ((color & 0xf0) >> shift);
			}

			/* draw lower pixel */
			if (!transparent || (pix & transmasklo))
			{
				offs_t addr1 = addr + shift/4;
				tms_state.vram[addr1] = (tms_state.vram[addr1] & (0xf0 >> shift)) | ((pix & 0x0f) << shift);
				tms_state.latchram[addr1] = (tms_state.latchram[addr1] & (0xf0 >> shift)) | ((color & 0x0f) << shift);
			}

			/* advance to the next byte */
			addr += xdir;
		}

		/* skip right */
		addr += xdir * skip[~y & 1];
		if (rle)
			consume_rle(skip[~y & 1]);
		else
			consume_raw(skip[~y & 1]);

		/* back up one and reverse directions */
		addr -= xdir;
		addr += ydir * 256;
		addr &= 0x3ffff;
		xdir = -xdir;
	}
}



/*************************************
 *
 *  Blitter finished callback
 *
 *************************************/

static TIMER_CALLBACK( blitter_done )
{
	/* turn off blitting and generate an interrupt */
	blit_in_progress = 0;
	itech8_update_interrupts(machine, -1, -1, 1);

	if (FULL_LOGGING) logerror("------------ BLIT DONE (%d) --------------\n", machine->primary_screen->vpos());
}



/*************************************
 *
 *  Blitter I/O
 *
 *************************************/

READ8_HANDLER( itech8_blitter_r )
{
	int result = blitter_data[offset / 2];
	static const char *const portnames[] = { "AN_C", "AN_D", "AN_E", "AN_F" };

	/* debugging */
	if (FULL_LOGGING) logerror("%04x:blitter_r(%02x)\n", cpu_get_previouspc(space->cpu), offset / 2);

	/* low bit seems to be ignored */
	offset /= 2;

	/* a read from offset 3 clears the interrupt and returns the status */
	if (offset == 3)
	{
		itech8_update_interrupts(space->machine, -1, -1, 0);
		if (blit_in_progress)
			result |= 0x80;
		else
			result &= 0x7f;
	}

	/* a read from offsets 12-15 return input port values */
	if (offset >= 12 && offset <= 15)
		result = input_port_read_safe(space->machine, portnames[offset - 12], 0x00);

	return result;
}


WRITE8_HANDLER( itech8_blitter_w )
{
	/* low bit seems to be ignored */
	offset /= 2;
	blitter_data[offset] = data;

	/* a write to offset 3 starts things going */
	if (offset == 3)
	{
		/* log to the blitter file */
		if (BLIT_LOGGING)
		{
			logerror("Blit: XY=%1X%04X SRC=%02X%02X%02X SIZE=%3dx%3d FLAGS=%02x",
						(tms_state.regs[TMS34061_XYOFFSET] >> 8) & 0x0f, tms_state.regs[TMS34061_XYADDRESS],
						*itech8_grom_bank, blitter_data[0], blitter_data[1],
						blitter_data[4], blitter_data[5],
						blitter_data[2]);
			logerror("   %02X %02X %02X [%02X] %02X %02X %02X [%02X]-%02X %02X %02X %02X [%02X %02X %02X %02X]\n",
						blitter_data[0], blitter_data[1],
						blitter_data[2], blitter_data[3],
						blitter_data[4], blitter_data[5],
						blitter_data[6], blitter_data[7],
						blitter_data[8], blitter_data[9],
						blitter_data[10], blitter_data[11],
						blitter_data[12], blitter_data[13],
						blitter_data[14], blitter_data[15]);
		}

		/* perform the blit */
		perform_blit(space);
		blit_in_progress = 1;

		/* set a timer to go off when we're done */
		timer_set(space->machine, attotime_mul(ATTOTIME_IN_HZ(12000000/4), BLITTER_WIDTH * BLITTER_HEIGHT + 12), NULL, 0, blitter_done);
	}

	/* debugging */
	if (FULL_LOGGING) logerror("%04x:blitter_w(%02x)=%02x\n", cpu_get_previouspc(space->cpu), offset, data);
}



/*************************************
 *
 *  TMS34061 I/O
 *
 *************************************/

WRITE8_HANDLER( itech8_tms34061_w )
{
	int func = (offset >> 9) & 7;
	int col = offset & 0xff;

	/* Column address (CA0-CA8) is hooked up the A0-A7, with A1 being inverted
       during register access. CA8 is ignored */
	if (func == 0 || func == 2)
		col ^= 2;

	/* Row address (RA0-RA8) is not dependent on the offset */
	tms34061_w(space, col, 0xff, func, data);
}


READ8_HANDLER( itech8_tms34061_r )
{
	int func = (offset >> 9) & 7;
	int col = offset & 0xff;

	/* Column address (CA0-CA8) is hooked up the A0-A7, with A1 being inverted
       during register access. CA8 is ignored */
	if (func == 0 || func == 2)
		col ^= 2;

	/* Row address (RA0-RA8) is not dependent on the offset */
	return tms34061_r(space, col, 0xff, func);
}



/*************************************
 *
 *  Special Grudge Match handlers
 *
 *************************************/

WRITE8_HANDLER( grmatch_palette_w )
{
	/* set the palette control; examined in the scanline callback */
	grmatch_palcontrol = data;
}


WRITE8_HANDLER( grmatch_xscroll_w )
{
	/* update the X scroll value */
	space->machine->primary_screen->update_now();
	grmatch_xscroll = data;
}


TIMER_DEVICE_CALLBACK( grmatch_palette_update )
{
	/* if the high bit is set, we are supposed to latch the palette values */
	if (grmatch_palcontrol & 0x80)
	{
		/* the TMS34070s latch at the start of the frame, based on the first few bytes */
		UINT32 page_offset = (tms_state.dispstart & 0x0ffff) | grmatch_xscroll;
		int page, x;

		/* iterate over both pages */
		for (page = 0; page < 2; page++)
		{
			const UINT8 *base = &tms_state.vram[(page * 0x20000 + page_offset) & 0x3ffff];
			for (x = 0; x < 16; x++)
			{
				UINT8 data0 = base[x * 2 + 0];
				UINT8 data1 = base[x * 2 + 1];
				grmatch_palette[page][x] = MAKE_RGB(pal4bit(data0 >> 0), pal4bit(data1 >> 4), pal4bit(data1 >> 0));
			}
		}
	}
}



/*************************************
 *
 *  Main refresh
 *
 *************************************/

VIDEO_UPDATE( itech8_2layer )
{
	UINT32 page_offset;
	int x, y;
	const rgb_t *pens = tlc34076_get_pens();

	/* first get the current display state */
	tms34061_get_display_state(&tms_state);

	/* if we're blanked, just fill with black */
	if (tms_state.blanked)
	{
		bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));
		return 0;
	}

	/* there are two layers: */
	/*    top layer @ 0x00000 is only 4bpp, colors come from the first 16 palettes */
	/* bottom layer @ 0x20000 is full 8bpp */
	page_offset = tms_state.dispstart & 0x0ffff;
	for (y = cliprect->min_y; y <= cliprect->max_y; y++)
	{
		UINT8 *base0 = &tms_state.vram[(0x00000 + page_offset + y * 256) & 0x3ffff];
		UINT8 *base2 = &tms_state.vram[(0x20000 + page_offset + y * 256) & 0x3ffff];
		UINT32 *dest = BITMAP_ADDR32(bitmap, y, 0);

		for (x = cliprect->min_x; x <= cliprect->max_x; x++)
		{
			int pix0 = base0[x] & 0x0f;
			dest[x] = pens[pix0 ? pix0 : base2[x]];
		}
	}
	return 0;
}


VIDEO_UPDATE( itech8_grmatch )
{
	UINT32 page_offset;
	int x, y;

	/* first get the current display state */
	tms34061_get_display_state(&tms_state);

	/* if we're blanked, just fill with black */
	if (tms_state.blanked)
	{
		bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));
		return 0;
	}

	/* there are two layers: */
	/*    top layer @ 0x00000 is 4bpp, colors come from TMS34070, enabled via palette control */
	/* bottom layer @ 0x20000 is 4bpp, colors come from TMS34070, enabled via palette control */
	/* 4bpp pixels are packed 2 to a byte */
	/* xscroll is set via a separate register */
	page_offset = (tms_state.dispstart & 0x0ffff) | grmatch_xscroll;
	for (y = cliprect->min_y; y <= cliprect->max_y; y++)
	{
		UINT8 *base0 = &tms_state.vram[0x00000 + ((page_offset + y * 256) & 0xffff)];
		UINT8 *base2 = &tms_state.vram[0x20000 + ((page_offset + y * 256) & 0xffff)];
		UINT32 *dest = BITMAP_ADDR32(bitmap, y, 0);

		for (x = cliprect->min_x & ~1; x <= cliprect->max_x; x += 2)
		{
			UINT8 pix0 = base0[x / 2];
			UINT8 pix2 = base2[x / 2];

			if ((pix0 & 0xf0) != 0)
				dest[x] = grmatch_palette[0][pix0 >> 4];
			else
				dest[x] = grmatch_palette[1][pix2 >> 4];

			if ((pix0 & 0x0f) != 0)
				dest[x + 1] = grmatch_palette[0][pix0 & 0x0f];
			else
				dest[x + 1] = grmatch_palette[1][pix2 & 0x0f];
		}
	}
	return 0;
}


VIDEO_UPDATE( itech8_2page )
{
	UINT32 page_offset;
	int x, y;
	const rgb_t *pens = tlc34076_get_pens();

	/* first get the current display state */
	tms34061_get_display_state(&tms_state);

	/* if we're blanked, just fill with black */
	if (tms_state.blanked)
	{
		bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));
		return 0;
	}

	/* there are two pages, each of which is a full 8bpp */
	/* page index is selected by the top bit of the page_select register */
	page_offset = ((page_select & 0x80) << 10) | (tms_state.dispstart & 0x0ffff);
	for (y = cliprect->min_y; y <= cliprect->max_y; y++)
	{
		UINT8 *base = &tms_state.vram[(page_offset + y * 256) & 0x3ffff];
		UINT32 *dest = BITMAP_ADDR32(bitmap, y, 0);

		for (x = cliprect->min_x; x <= cliprect->max_x; x++)
			dest[x] = pens[base[x]];
	}
	return 0;
}


VIDEO_UPDATE( itech8_2page_large )
{
	UINT32 page_offset;
	int x, y;
	const rgb_t *pens = tlc34076_get_pens();

	/* first get the current display state */
	tms34061_get_display_state(&tms_state);

	/* if we're blanked, just fill with black */
	if (tms_state.blanked)
	{
		bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));
		return 0;
	}

	/* there are two pages, each of which is a full 8bpp */
	/* the low 4 bits come from the bitmap directly */
	/* the upper 4 bits were latched on each write into a separate bitmap */
	/* page index is selected by the top bit of the page_select register */
	page_offset = ((~page_select & 0x80) << 10) | (tms_state.dispstart & 0x0ffff);
	for (y = cliprect->min_y; y <= cliprect->max_y; y++)
	{
		UINT8 *base = &tms_state.vram[(page_offset + y * 256) & 0x3ffff];
		UINT8 *latch = &tms_state.latchram[(page_offset + y * 256) & 0x3ffff];
		UINT32 *dest = BITMAP_ADDR32(bitmap, y, 0);

		for (x = cliprect->min_x & ~1; x <= cliprect->max_x; x += 2)
		{
			dest[x + 0] = pens[(latch[x/2] & 0xf0) | (base[x/2] >> 4)];
			dest[x + 1] = pens[((latch[x/2] << 4) & 0xf0) | (base[x/2] & 0x0f)];
		}
	}
	return 0;
}
