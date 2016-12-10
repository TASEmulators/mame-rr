/***************************************************************************

    Taito Qix hardware

    driver by John Butler, Ed Mueller, Aaron Giles

***************************************************************************/

#include "emu.h"
#include "video/mc6845.h"
#include "includes/qix.h"
#include "cpu/m6809/m6809.h"



/*************************************
 *
 *  Device tag
 *
 *************************************/

#define MC6845_TAG		("vid_u18")



/*************************************
 *
 *  Static function prototypes
 *
 *************************************/

static MC6845_BEGIN_UPDATE( begin_update );
static MC6845_UPDATE_ROW( update_row );
static WRITE_LINE_DEVICE_HANDLER( display_enable_changed );



/*************************************
 *
 *  Start
 *
 *************************************/

static VIDEO_START( qix )
{
	qix_state *state = (qix_state *)machine->driver_data;

	/* allocate memory for the full video RAM */
	state->videoram = auto_alloc_array(machine, UINT8, 256 * 256);

	/* set up save states */
	state_save_register_global_pointer(machine, state->videoram, 256 * 256);
	state_save_register_global(machine, state->flip);
	state_save_register_global(machine, state->palette_bank);
	state_save_register_global(machine, state->leds);
}



/*************************************
 *
 *  Current scanline read
 *
 *************************************/

static WRITE_LINE_DEVICE_HANDLER( display_enable_changed )
{
	qix_state *driver_state = (qix_state *)device->machine->driver_data;

	/* on the rising edge, latch the scanline */
	if (state)
	{
		UINT16 ma = mc6845_get_ma(device);
		UINT8 ra = mc6845_get_ra(device);

		/* RA0-RA2 goes to D0-D2 and MA5-MA9 goes to D3-D7 */
		*driver_state->scanline_latch = ((ma >> 2) & 0xf8) | (ra & 0x07);
	}
}



/*************************************
 *
 *  Cocktail flip
 *
 *************************************/

WRITE8_DEVICE_HANDLER( qix_flip_screen_w )
{
	qix_state *state = (qix_state *)device->machine->driver_data;

	state->flip = data;
}



/*************************************
 *
 *  Direct video RAM read/write
 *
 *  The screen is 256x256 with eight
 *  bit pixels (64K).  The screen is
 *  divided into two halves each half
 *  mapped by the video CPU at
 *  $0000-$7FFF.  The high order bit
 *  of the address latch at $9402
 *  specifies which half of the screen
 *  is being accessed.
 *
 *************************************/

static READ8_HANDLER( qix_videoram_r )
{
	qix_state *state = (qix_state *)space->machine->driver_data;

	/* add in the upper bit of the address latch */
	offset += (state->videoram_address[0] & 0x80) << 8;
	return state->videoram[offset];
}


static WRITE8_HANDLER( qix_videoram_w )
{
	qix_state *state = (qix_state *)space->machine->driver_data;

	/* update the screen in case the game is writing "behind" the beam -
       Zookeeper likes to do this */
	space->machine->primary_screen->update_now();

	/* add in the upper bit of the address latch */
	offset += (state->videoram_address[0] & 0x80) << 8;

	/* write the data */
	state->videoram[offset] = data;
}


static WRITE8_HANDLER( slither_videoram_w )
{
	qix_state *state = (qix_state *)space->machine->driver_data;

	/* update the screen in case the game is writing "behind" the beam -
       Zookeeper likes to do this */
	space->machine->primary_screen->update_now();

	/* add in the upper bit of the address latch */
	offset += (state->videoram_address[0] & 0x80) << 8;

	/* blend the data */
	state->videoram[offset] = (state->videoram[offset] & ~*state->videoram_mask) | (data & *state->videoram_mask);
}



/*************************************
 *
 *  Latched video RAM read/write
 *
 *  The address latch works as follows.
 *  When the video CPU accesses $9400,
 *  the screen address is computed by
 *  using the values at $9402 (high
 *  byte) and $9403 (low byte) to get
 *  a value between $0000-$FFFF.  The
 *  value at that location is either
 *  returned or written.
 *
 *************************************/

static READ8_HANDLER( qix_addresslatch_r )
{
	qix_state *state = (qix_state *)space->machine->driver_data;

	/* compute the value at the address latch */
	offset = (state->videoram_address[0] << 8) | state->videoram_address[1];
	return state->videoram[offset];
}


static WRITE8_HANDLER( qix_addresslatch_w )
{
	qix_state *state = (qix_state *)space->machine->driver_data;

	/* update the screen in case the game is writing "behind" the beam */
	space->machine->primary_screen->update_now();

	/* compute the value at the address latch */
	offset = (state->videoram_address[0] << 8) | state->videoram_address[1];

	/* write the data */
	state->videoram[offset] = data;
}


static WRITE8_HANDLER( slither_addresslatch_w )
{
	qix_state *state = (qix_state *)space->machine->driver_data;

	/* update the screen in case the game is writing "behind" the beam */
	space->machine->primary_screen->update_now();

	/* compute the value at the address latch */
	offset = (state->videoram_address[0] << 8) | state->videoram_address[1];

	/* blend the data */
	state->videoram[offset] = (state->videoram[offset] & ~*state->videoram_mask) | (data & *state->videoram_mask);
}



/*************************************
 *
 *  Palette RAM
 *
 *************************************/

#define NUM_PENS	(0x100)


static WRITE8_HANDLER( qix_paletteram_w )
{
	qix_state *state = (qix_state *)space->machine->driver_data;

	UINT8 old_data = state->paletteram[offset];

	/* set the palette RAM value */
	state->paletteram[offset] = data;

	/* trigger an update if a currently visible pen has changed */
	if (((offset >> 8) == state->palette_bank) &&
	    (old_data != data))
		space->machine->primary_screen->update_now();
}


WRITE8_HANDLER( qix_palettebank_w )
{
	qix_state *state = (qix_state *)space->machine->driver_data;

	/* set the bank value */
	if (state->palette_bank != (data & 3))
	{
		space->machine->primary_screen->update_now();
		state->palette_bank = data & 3;
	}

	/* LEDs are in the upper 6 bits */
	state->leds = ~data & 0xfc;
}


static void get_pens(qix_state *state, pen_t *pens)
{
	offs_t offs;

	/* this conversion table should be about right. It gives a reasonable */
	/* gray scale in the test screen, and the red, green and blue squares */
	/* in the same screen are barely visible, as the manual requires. */
	static const UINT8 table[16] =
	{
		0x00,	/* value = 0, intensity = 0 */
		0x12,	/* value = 0, intensity = 1 */
		0x24,	/* value = 0, intensity = 2 */
		0x49,	/* value = 0, intensity = 3 */
		0x12,	/* value = 1, intensity = 0 */
		0x24,	/* value = 1, intensity = 1 */
		0x49,	/* value = 1, intensity = 2 */
		0x92,	/* value = 1, intensity = 3 */
		0x5b,	/* value = 2, intensity = 0 */
		0x6d,	/* value = 2, intensity = 1 */
		0x92,	/* value = 2, intensity = 2 */
		0xdb,	/* value = 2, intensity = 3 */
		0x7f,	/* value = 3, intensity = 0 */
		0x91,	/* value = 3, intensity = 1 */
		0xb6,	/* value = 3, intensity = 2 */
		0xff	/* value = 3, intensity = 3 */
	};

	for (offs = state->palette_bank << 8; offs < (state->palette_bank << 8) + NUM_PENS; offs++)
	{
		int bits, intensity, r, g, b;

		UINT8 data = state->paletteram[offs];

		/* compute R, G, B from the table */
		intensity = (data >> 0) & 0x03;
		bits = (data >> 6) & 0x03;
		r = table[(bits << 2) | intensity];
		bits = (data >> 4) & 0x03;
		g = table[(bits << 2) | intensity];
		bits = (data >> 2) & 0x03;
		b = table[(bits << 2) | intensity];

		/* update the palette */
		pens[offs & 0xff] = MAKE_RGB(r, g, b);
	}
}



/*************************************
 *
 *  M6845 callbacks for updating
 *  the screen
 *
 *************************************/

static MC6845_BEGIN_UPDATE( begin_update )
{
	qix_state *state = (qix_state *)device->machine->driver_data;

#if 0
	// note the confusing bit order!
	popmessage("self test leds: %d%d %d%d%d%d",BIT(leds,7),BIT(leds,5),BIT(leds,6),BIT(leds,4),BIT(leds,2),BIT(leds,3));
#endif

	/* create the pens */
	static pen_t pens[NUM_PENS];

	get_pens(state, pens);

	return pens;
}


static MC6845_UPDATE_ROW( update_row )
{
	qix_state *state = (qix_state *)device->machine->driver_data;
	UINT32 *dest = BITMAP_ADDR32(bitmap, y, 0);
	UINT16 x;

	pen_t *pens = (pen_t *)param;

	/* the memory is hooked up to the MA, RA lines this way */
	offs_t offs = ((ma << 6) & 0xf800) | ((ra << 8) & 0x0700);
	offs_t offs_xor = state->flip ? 0xffff : 0;

	for (x = 0; x < x_count * 8; x++)
		dest[x] = pens[state->videoram[(offs + x) ^ offs_xor]];
}



/*************************************
 *
 *  Standard video update
 *
 *************************************/

static VIDEO_UPDATE( qix )
{
	running_device *mc6845 = screen->machine->device(MC6845_TAG);
	mc6845_update(mc6845, bitmap, cliprect);

	return 0;
}



/*************************************
 *
 *  Memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( qix_video_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_READWRITE(qix_videoram_r, qix_videoram_w)
	AM_RANGE(0x8000, 0x83ff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x8400, 0x87ff) AM_RAM AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE(0x8800, 0x8800) AM_MIRROR(0x03ff) AM_WRITE(qix_palettebank_w)
	AM_RANGE(0x8c00, 0x8c00) AM_MIRROR(0x03fe) AM_READWRITE(qix_data_firq_r, qix_data_firq_w)
	AM_RANGE(0x8c01, 0x8c01) AM_MIRROR(0x03fe) AM_READWRITE(qix_video_firq_ack_r, qix_video_firq_ack_w)
	AM_RANGE(0x9000, 0x93ff) AM_RAM_WRITE(qix_paletteram_w) AM_BASE_MEMBER(qix_state, paletteram)
	AM_RANGE(0x9400, 0x9400) AM_MIRROR(0x03fc) AM_READWRITE(qix_addresslatch_r, qix_addresslatch_w)
	AM_RANGE(0x9402, 0x9403) AM_MIRROR(0x03fc) AM_WRITEONLY AM_BASE_MEMBER(qix_state, videoram_address)
	AM_RANGE(0x9800, 0x9800) AM_MIRROR(0x03ff) AM_READONLY AM_BASE_MEMBER(qix_state, scanline_latch)
	AM_RANGE(0x9c00, 0x9c00) AM_MIRROR(0x03fe) AM_DEVWRITE("vid_u18", mc6845_address_w)
	AM_RANGE(0x9c01, 0x9c01) AM_MIRROR(0x03fe) AM_DEVREADWRITE("vid_u18", mc6845_register_r, mc6845_register_w)
	AM_RANGE(0xa000, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( zookeep_video_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_READWRITE(qix_videoram_r, qix_videoram_w)
	AM_RANGE(0x8000, 0x83ff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x8400, 0x87ff) AM_RAM AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE(0x8800, 0x8800) AM_MIRROR(0x03fe) AM_WRITE(qix_palettebank_w)
	AM_RANGE(0x8801, 0x8801) AM_MIRROR(0x03fe) AM_WRITE(zookeep_bankswitch_w)
	AM_RANGE(0x8c00, 0x8c00) AM_MIRROR(0x03fe) AM_READWRITE(qix_data_firq_r, qix_data_firq_w)
	AM_RANGE(0x8c01, 0x8c01) AM_MIRROR(0x03fe) AM_READWRITE(qix_video_firq_ack_r, qix_video_firq_ack_w)
	AM_RANGE(0x9000, 0x93ff) AM_RAM_WRITE(qix_paletteram_w) AM_BASE_MEMBER(qix_state, paletteram)
	AM_RANGE(0x9400, 0x9400) AM_MIRROR(0x03fc) AM_READWRITE(qix_addresslatch_r, qix_addresslatch_w)
	AM_RANGE(0x9402, 0x9403) AM_MIRROR(0x03fc) AM_WRITEONLY AM_BASE_MEMBER(qix_state, videoram_address)
	AM_RANGE(0x9800, 0x9800) AM_MIRROR(0x03ff) AM_READONLY AM_BASE_MEMBER(qix_state, scanline_latch)
	AM_RANGE(0x9c00, 0x9c00) AM_MIRROR(0x03fe) AM_DEVWRITE("vid_u18", mc6845_address_w)
	AM_RANGE(0x9c01, 0x9c01) AM_MIRROR(0x03fe) AM_DEVREADWRITE("vid_u18", mc6845_register_r, mc6845_register_w)
	AM_RANGE(0xa000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( slither_video_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_READWRITE(qix_videoram_r, slither_videoram_w)
	AM_RANGE(0x8000, 0x83ff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x8400, 0x87ff) AM_RAM AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE(0x8800, 0x8800) AM_MIRROR(0x03ff) AM_WRITE(qix_palettebank_w)
	AM_RANGE(0x8c00, 0x8c00) AM_MIRROR(0x03fe) AM_READWRITE(qix_data_firq_r, qix_data_firq_w)
	AM_RANGE(0x8c01, 0x8c01) AM_MIRROR(0x03fe) AM_READWRITE(qix_video_firq_ack_r, qix_video_firq_ack_w)
	AM_RANGE(0x9000, 0x93ff) AM_RAM_WRITE(qix_paletteram_w) AM_BASE_MEMBER(qix_state, paletteram)
	AM_RANGE(0x9400, 0x9400) AM_MIRROR(0x03fc) AM_READWRITE(qix_addresslatch_r, slither_addresslatch_w)
	AM_RANGE(0x9401, 0x9401) AM_MIRROR(0x03fc) AM_WRITEONLY AM_BASE_MEMBER(qix_state, videoram_mask)
	AM_RANGE(0x9402, 0x9403) AM_MIRROR(0x03fc) AM_WRITEONLY AM_BASE_MEMBER(qix_state, videoram_address)
	AM_RANGE(0x9800, 0x9800) AM_MIRROR(0x03ff) AM_READONLY AM_BASE_MEMBER(qix_state, scanline_latch)
	AM_RANGE(0x9c00, 0x9c00) AM_MIRROR(0x03fe) AM_DEVWRITE("vid_u18", mc6845_address_w)
	AM_RANGE(0x9c01, 0x9c01) AM_MIRROR(0x03fe) AM_DEVREADWRITE("vid_u18", mc6845_register_r, mc6845_register_w)
	AM_RANGE(0xa000, 0xffff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static const mc6845_interface mc6845_intf =
{
	"screen",							/* screen we are acting on */
	8,									/* number of pixels per video memory address */
	begin_update,						/* before pixel update callback */
	update_row,							/* row update callback */
	NULL,								/* after pixel update callback */
	DEVCB_LINE(display_enable_changed),	/* callback for display state changes */
	DEVCB_NULL,							/* callback for cursor state changes */
	DEVCB_NULL,							/* HSYNC callback */
	DEVCB_LINE(qix_vsync_changed),		/* VSYNC callback */
	NULL								/* update address callback */
};


static const m6809_config encryption_config =
{
	TRUE,		/* encrypt only the first byte in 10 xx and 11 xx opcodes */
};


MACHINE_DRIVER_START( qix_video )
	MDRV_CPU_ADD("videocpu", M6809, MAIN_CLOCK_OSC/4/4)	/* 1.25 MHz */
	MDRV_CPU_PROGRAM_MAP(qix_video_map)
	MDRV_CPU_CONFIG(encryption_config)	// for kram3

	MDRV_VIDEO_START(qix)
	MDRV_VIDEO_UPDATE(qix)

	MDRV_MC6845_ADD(MC6845_TAG, MC6845, QIX_CHARACTER_CLOCK, mc6845_intf)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_RAW_PARAMS(QIX_CHARACTER_CLOCK*8, 256, 0, 256, 256, 0, 256)	/* temporary, CRTC will configure screen */
MACHINE_DRIVER_END


MACHINE_DRIVER_START( zookeep_video )
	MDRV_CPU_MODIFY("videocpu")
	MDRV_CPU_PROGRAM_MAP(zookeep_video_map)
MACHINE_DRIVER_END


MACHINE_DRIVER_START( slither_video )
	MDRV_CPU_MODIFY("videocpu")
	MDRV_CPU_CLOCK(SLITHER_CLOCK_OSC/4/4)	/* 1.34 MHz */
	MDRV_CPU_PROGRAM_MAP(slither_video_map)
MACHINE_DRIVER_END
