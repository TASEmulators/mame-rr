/***************************************************************************

    Sega G-80 raster hardware

***************************************************************************/

#include "emu.h"
#include "includes/segag80r.h"
#include "machine/rescap.h"
#include "video/resnet.h"

UINT8 segag80r_background_pcb;

static double rweights[3], gweights[3], bweights[2];

static UINT8 video_control;
static UINT8 video_flip;
static UINT8 vblank_latch;

static tilemap_t *spaceod_bg_htilemap;
static tilemap_t *spaceod_bg_vtilemap;
static UINT16 spaceod_hcounter;
static UINT16 spaceod_vcounter;
static UINT8 spaceod_fixed_color;
static UINT8 spaceod_bg_control;
static UINT8 spaceod_bg_detect;
static const UINT8 spaceod_bg_detect_tile_color = 1;

static tilemap_t *bg_tilemap;
static UINT8 bg_enable;
static UINT8 bg_char_bank;
static UINT16 bg_scrollx, bg_scrolly;

static UINT8 pignewt_bg_color_offset;



/*************************************
 *
 *  VBLANK handling
 *
 *************************************/

static TIMER_CALLBACK( vblank_latch_clear )
{
	vblank_latch = 0;
}


static void vblank_latch_set(running_machine *machine)
{
	/* set a timer to mimic the 555 timer that drives the EDGINT signal */
	/* the 555 is run in monostable mode with R=56000 and C=1000pF */
	vblank_latch = 1;
	timer_set(machine, PERIOD_OF_555_MONOSTABLE(CAP_P(1000), RES_K(56)), NULL, 0, vblank_latch_clear);

	/* latch the current flip state at the same time */
	video_flip = video_control & 1;
}


INTERRUPT_GEN( segag80r_vblank_start )
{
	vblank_latch_set(device->machine);

	/* if interrupts are enabled, clock one */
	if (video_control & 0x04)
		irq0_line_hold(device);
}


INTERRUPT_GEN( sindbadm_vblank_start )
{
	vblank_latch_set(device->machine);

	/* interrupts appear to always be enabled, but they have a manual */
	/* acknowledge rather than an automatic ack; they are also not masked */
	/* by bit 2 of video_control like a standard G80 */
	irq0_line_assert(device);
}



/*************************************
 *
 *  Palette conversion
 *
 *************************************/

static void g80_set_palette_entry(running_machine *machine, int entry, UINT8 data)
{
	int bit0, bit1, bit2;
	int r, g, b;

	/* extract the raw RGB bits */
	r = (data & 0x07) >> 0;
	g = (data & 0x38) >> 3;
	b = (data & 0xc0) >> 6;

	/* red component */
	bit0 = (r >> 0) & 0x01;
	bit1 = (r >> 1) & 0x01;
	bit2 = (r >> 2) & 0x01;
	r = combine_3_weights(rweights, bit0, bit1, bit2);

	/* green component */
	bit0 = (g >> 0) & 0x01;
	bit1 = (g >> 1) & 0x01;
	bit2 = (g >> 2) & 0x01;
	g = combine_3_weights(gweights, bit0, bit1, bit2);

	/* blue component */
	bit0 = (b >> 0) & 0x01;
	bit1 = (b >> 1) & 0x01;
	b = combine_2_weights(bweights, bit0, bit1);

	palette_set_color(machine, entry, MAKE_RGB(r, g, b));
}


static void spaceod_bg_init_palette(running_machine *machine)
{
	static const int resistances[2] = { 1800, 1200 };
	double trweights[2], tgweights[2], tbweights[2];
	int i;

	/* compute the color output resistor weights at startup */
	compute_resistor_weights(0,	255, -1.0,
			2,	resistances, trweights, 220, 0,
			2,	resistances, tgweights, 220, 0,
			2,	resistances, tbweights, 220, 0);

	/* initialize the fixed background palette */
	for (i = 0; i < 64; i++)
	{
		int bit0, bit1;
		int r, g, b;

		/* extract the raw RGB bits */
		r = (i & 0x30) >> 4;
		g = (i & 0x0c) >> 2;
		b = (i & 0x03) >> 0;

		/* red component */
		bit0 = (r >> 0) & 0x01;
		bit1 = (r >> 1) & 0x01;
		r = combine_2_weights(trweights, bit0, bit1);

		/* green component */
		bit0 = (g >> 0) & 0x01;
		bit1 = (g >> 1) & 0x01;
		g = combine_2_weights(tgweights, bit0, bit1);

		/* blue component */
		bit0 = (b >> 0) & 0x01;
		bit1 = (b >> 1) & 0x01;
		b = combine_2_weights(tbweights, bit0, bit1);

		palette_set_color(machine, 64 + i, MAKE_RGB(r, g, b));
	}
}



/*************************************
 *
 *  Tilemap callbacks
 *
 *************************************/

static TILE_GET_INFO( spaceod_get_tile_info )
{
	int code = memory_region(machine, "gfx2")[tile_index + 0x1000 * (spaceod_bg_control >> 6)];
	SET_TILE_INFO(1, code + 0x100 * ((spaceod_bg_control >> 2) & 1), 0, 0);
}


static TILEMAP_MAPPER( spaceod_scan_rows )
{
	/* this works for both horizontal and vertical tilemaps */
	/* which are 4 32x32 sections */
	return (row & 31) * 32 + (col & 31) + ((row >> 5) * 32*32) + ((col >> 5) * 32*32);
}


static TILE_GET_INFO( bg_get_tile_info )
{
	int code = memory_region(machine, "gfx2")[tile_index];
	SET_TILE_INFO(1, code + 0x100 * bg_char_bank, code >> 4, 0);
}



/*************************************
 *
 *  Video startup
 *
 *************************************/

VIDEO_START( segag80r )
{
	static const int rg_resistances[3] = { 4700, 2400, 1200 };
	static const int b_resistances[2] = { 2000, 1000 };

	/* compute the color output resistor weights at startup */
	compute_resistor_weights(0,	255, -1.0,
			3,	rg_resistances, rweights, 220, 0,
			3,	rg_resistances, gweights, 220, 0,
			2,	b_resistances,  bweights, 220, 0);

	gfx_element_set_source(machine->gfx[0], &machine->generic.videoram.u8[0x800]);

	/* allocate paletteram */
	machine->generic.paletteram.u8 = auto_alloc_array(machine, UINT8, 0x80);

	/* initialize the particulars for each type of background PCB */
	switch (segag80r_background_pcb)
	{
		/* nothing to do here */
		case G80_BACKGROUND_NONE:
			break;

		/* create a fixed background palette and two tilemaps, one horizontally scrolling */
		/* and one vertically scrolling */
		case G80_BACKGROUND_SPACEOD:
			spaceod_bg_init_palette(machine);
			spaceod_bg_htilemap = tilemap_create(machine, spaceod_get_tile_info, spaceod_scan_rows,  8,8, 128,32);
			spaceod_bg_vtilemap = tilemap_create(machine, spaceod_get_tile_info, spaceod_scan_rows,  8,8, 32,128);
			break;

		/* background tilemap is effectively 1 screen x n screens */
		case G80_BACKGROUND_MONSTERB:
			bg_tilemap = tilemap_create(machine, bg_get_tile_info, tilemap_scan_rows,  8,8, 32,memory_region_length(machine, "gfx2") / 32);
			break;

		/* background tilemap is effectively 4 screens x n screens */
		case G80_BACKGROUND_PIGNEWT:
		case G80_BACKGROUND_SINDBADM:
			bg_tilemap = tilemap_create(machine, bg_get_tile_info, tilemap_scan_rows,  8,8, 128,memory_region_length(machine, "gfx2") / 128);
			break;
	}

	/* register for save states */
	state_save_register_global_pointer(machine, machine->generic.paletteram.u8, 0x80);

	state_save_register_global(machine, video_control);
	state_save_register_global(machine, video_flip);
	state_save_register_global(machine, vblank_latch);

	state_save_register_global(machine, spaceod_hcounter);
	state_save_register_global(machine, spaceod_vcounter);
	state_save_register_global(machine, spaceod_fixed_color);
	state_save_register_global(machine, spaceod_bg_control);
	state_save_register_global(machine, spaceod_bg_detect);

	state_save_register_global(machine, bg_enable);
	state_save_register_global(machine, bg_char_bank);
	state_save_register_global(machine, bg_scrollx);
	state_save_register_global(machine, bg_scrolly);

	state_save_register_global(machine, pignewt_bg_color_offset);
}



/*************************************
 *
 *  Video RAM write
 *
 *************************************/

WRITE8_HANDLER( segag80r_videoram_w )
{
	/* accesses to the upper half of VRAM go to paletteram if selected */
	if ((offset & 0x1000) && (video_control & 0x02))
	{
		offset &= 0x3f;
		space->machine->generic.paletteram.u8[offset] = data;
		g80_set_palette_entry(space->machine, offset, data);
		return;
	}

	/* all other accesses go to videoram */
	space->machine->generic.videoram.u8[offset] = data;

	/* track which characters are dirty */
	if (offset & 0x800)
		gfx_element_mark_dirty(space->machine->gfx[0], (offset & 0x7ff) / 8);
}



/*************************************
 *
 *  Video I board port accesses
 *
 *************************************/

READ8_HANDLER( segag80r_video_port_r )
{
	if (offset == 0)
	{
		logerror("%04X:segag80r_video_port_r(%d)\n", cpu_get_pc(space->cpu), offset);
		return 0xff;
	}
	else
	{
		/*
            D0 = 555 timer output from U10 (goes to EDGINT as well)
            D1 = current latched FLIP state
            D2 = interrupt enable state
            D3 = n/c
        */
		return (vblank_latch << 0) | (video_flip << 1) | (video_control & 0x04) | 0xf8;
	}
}


WRITE8_HANDLER( segag80r_video_port_w )
{
	if (offset == 0)
	{
		logerror("%04X:segag80r_video_port_w(%d) = %02X\n", cpu_get_pc(space->cpu), offset, data);
	}
	else
	{
		/*
            D0 = FLIP (latched at VSYNC)
            D1 = if low, allows writes to the upper 4k of video RAM
               = if high, allows writes to palette RAM
            D2 = interrupt enable
            D3 = n/c (used as flip by many background boards)
        */
		video_control = data;
	}
}



/*************************************
 *
 *  Space Odyssey background borad
 *  port accesses
 *
 *************************************/

READ8_HANDLER( spaceod_back_port_r )
{
	/* force an update to get the current detection value */
	space->machine->primary_screen->update_partial(space->machine->primary_screen->vpos());
	return 0xfe | spaceod_bg_detect;
}


WRITE8_HANDLER( spaceod_back_port_w )
{
	switch (offset & 7)
	{
		/* port 0: latches D0-D7 into LS377 at U39 (SH4)

            d0 = counter direction: controls U/D on LS191 counters
            d1 = horizontal (0) or vertical (1) scrolling
            d2 = character bank (0/1)
            d6 = background ROM select 0
            d7 = background ROM select 1
        */
		case 0:
			if ((spaceod_bg_control ^ data) & 0xc4)
			{
				tilemap_mark_all_tiles_dirty(spaceod_bg_htilemap);
				tilemap_mark_all_tiles_dirty(spaceod_bg_vtilemap);
			}
			spaceod_bg_control = data;
			break;

		/* port 1: loads both H and V counters with 0 */
		case 1:
			spaceod_hcounter = 0;
			spaceod_vcounter = 0;
			break;

		/* port 2: clocks either the H or V counters (based on port 0:d1) */
		/* either up or down (based on port 0:d0) */
		case 2:
			if (!(spaceod_bg_control & 0x02))
			{
				if (!(spaceod_bg_control & 0x01))
					spaceod_hcounter++;
				else
					spaceod_hcounter--;
			}
			else
			{
				if (!(spaceod_bg_control & 0x01))
					spaceod_vcounter++;
				else
					spaceod_vcounter--;
			}
			break;

		/* port 3: clears the background detection flag */
		case 3:
			space->machine->primary_screen->update_partial(space->machine->primary_screen->vpos());
			spaceod_bg_detect = 0;
			break;

		/* port 4: enables (0)/disables (1) the background */
		case 4:
			bg_enable = data & 1;
			break;

		/* port 5: specifies fixed background color */
		/* top two bits are not connected */
		case 5:
			spaceod_fixed_color = data & 0x3f;
			break;

		/* port 6: latches D0-D7 into LS377 at U37 -> CN1-11/12/13/14/15/16/17/18 */
		/* port 7: latches D0-D5 into LS174 at U40 -> CN2-1/2/3/4/5/6 */
		case 6:
		case 7:
			break;
	}
}



/*************************************
 *
 *  Monster Bash background board
 *  accesses
 *
 *************************************/

WRITE8_HANDLER( monsterb_videoram_w )
{
	/* accesses to the the area $f040-$f07f go to background palette if */
	/* the palette access enable bit is set */
	if ((offset & 0x1fc0) == 0x1040 && (video_control & 0x40))
	{
		offs_t paloffs = offset & 0x3f;
		space->machine->generic.paletteram.u8[paloffs | 0x40] = data;
		g80_set_palette_entry(space->machine, paloffs | 0x40, data);
		/* note that since the background board is not integrated with the main board */
		/* writes here also write through to regular videoram */
	}

	/* handle everything else */
	segag80r_videoram_w(space, offset, data);
}


WRITE8_HANDLER( monsterb_back_port_w )
{
	switch (offset & 7)
	{
		/* port 0: not used (looks like latches for C7-C10 = background color) */
		case 0:
			break;

		/* port 1: not used (looks like comparator tile color value for collision detect)  */
		case 1:
			break;

		/* port 2: not connected */
		/* port 3: not connected */
		case 2:
		case 3:
			break;

		/* port 4: main control latch

            d0 = CG0 - BG MSB ROM bank select bit 0
            d1 = CG1 - BG MSB ROM bank select bit 1
            d2 = CG2 - BG LSB ROM bank select bit 0
            d3 = CG3 - BG LSB ROM bank select bit 1
            d4 = SCN0 - background select bit 0
            d5 = SCN1 - background select bit 1
            d6 = SCN2 - background select bit 2
            d7 = BKGEN - background enable
         */
		case 4:
			if ((bg_char_bank ^ data) & 0x0f)
				tilemap_mark_all_tiles_dirty(bg_tilemap);
			bg_char_bank = data & 0x0f;
			bg_scrolly = (data << 4) & 0x700;
			bg_enable = data & 0x80;
			break;

		/* port 5: not connected */
		case 5:
			break;
	}
}



/*************************************
 *
 *  Pig Newton/Monster Bash 2-board
 *  background accesses
 *
 *************************************/

WRITE8_HANDLER( pignewt_videoram_w )
{
	/* accesses to the the area $f040-$f07f go to background palette if */
	/* the palette access enable bit is set */
	if ((offset & 0x1fc0) == 0x1040 && (video_control & 0x02))
	{
		offs_t paloffs = offset & 0x3f;
		space->machine->generic.paletteram.u8[paloffs | 0x40] = data;
		g80_set_palette_entry(space->machine, paloffs | 0x40, data);
		return;
	}

	/* handle everything else */
	segag80r_videoram_w(space, offset, data);
}


WRITE8_HANDLER( pignewt_back_color_w )
{
	/* it is not really known what this does */
	if (offset == 0)
		pignewt_bg_color_offset = data;
	else
		logerror("pignewt_back_color_w(%d) = %02X\n", pignewt_bg_color_offset, data);
}


WRITE8_HANDLER( pignewt_back_port_w )
{
	switch (offset & 7)
	{
		/* port 0: scroll offset low */
		case 0:
			bg_scrollx = (bg_scrollx & 0x300) | data;
			break;

		/* port 1: scroll offset high */
		case 1:
			bg_scrollx = (bg_scrollx & 0x0ff) | ((data << 8) & 0x300);
			bg_enable = data & 0x80;
			break;

		/* port 2: scroll offset low */
		case 2:
			bg_scrolly = (bg_scrolly & 0x300) | data;
			break;

		/* port 3: scroll offset high */
		case 3:
			bg_scrolly = (bg_scrolly & 0x0ff) | ((data << 8) & 0x300);
			break;

		/* port 4: background character bank control

            d0 = CG0 - BG MSB ROM bank select bit 0
            d1 = CG1 - BG MSB ROM bank select bit 1
            d2 = CG2 - BG LSB ROM bank select bit 0
            d3 = CG3 - BG LSB ROM bank select bit 1

            at least, that's the theory; however, monster2 sets this = 0x03 wanting
            bank 1, so I'm assuming that only d3 and d0 really matter
         */
		case 4:
			data = (data & 0x09) | ((data >> 2) & 0x02) | ((data << 2) & 0x04);
			if ((bg_char_bank ^ data) & 0x0f)
				tilemap_mark_all_tiles_dirty(bg_tilemap);
			bg_char_bank = data & 0x0f;
			break;

		/* port 5: not connected? */
		case 5:
			break;
	}
}



/*************************************
 *
 *  Sindbad Mystery background accesses
 *
 *************************************/

WRITE8_HANDLER( sindbadm_videoram_w )
{
	/* accesses to the the area $f000-$f03f go to background palette if */
	/* the palette access enable bit is set */
	if ((offset & 0x1fc0) == 0x1000 && (video_control & 0x02))
	{
		offs_t paloffs = offset & 0x3f;
		space->machine->generic.paletteram.u8[paloffs | 0x40] = data;
		g80_set_palette_entry(space->machine, paloffs | 0x40, data);
		return;
	}

	/* handle everything else */
	segag80r_videoram_w(space, offset, data);
}


WRITE8_HANDLER( sindbadm_back_port_w )
{
	switch (offset & 3)
	{
		/* port 0: irq ack */
		case 0:
			cputag_set_input_line(space->machine, "maincpu", 0, CLEAR_LINE);
			break;

		/* port 1: background control

            d0 = BG ROM bank select bit 0
            d1 = BG ROM bank select bit 1
            d2 = BG page select (X scroll bit 0)
            d3 = BG page select (X scroll bit 1)
            d4 = BG page select (Y scroll bit 0)
            d5 = BG page select (Y scroll bit 1)
            d6 = BG page select (Y scroll bit 2)
            d7 = BG enable
        */
		case 1:
			bg_enable = data & 0x80;
			bg_scrollx = (data << 6) & 0x300;
			bg_scrolly = (data << 4) & 0x700;
			if ((bg_char_bank ^ data) & 0x03)
				tilemap_mark_all_tiles_dirty(bg_tilemap);
			bg_char_bank = data & 0x03;
			break;
	}
}



/*************************************
 *
 *  Video I videoram rendering
 *
 *************************************/

static void draw_videoram(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, const UINT8 *transparent_pens)
{
	int flipmask = video_flip ? 0x1f : 0x00;
	int x, y;

	/* iterate over the screen and draw visible tiles */
	for (y = cliprect->min_y / 8; y <= cliprect->max_y / 8; y++)
	{
		int effy = video_flip ? 27 - y : y;
		for (x = cliprect->min_x / 8; x <= cliprect->max_x / 8; x++)
		{
			int offs = effy * 32 + (x ^ flipmask);
			UINT8 tile = machine->generic.videoram.u8[offs];

			/* draw the tile */
			drawgfx_transmask(bitmap, cliprect, machine->gfx[0], tile, tile >> 4, video_flip, video_flip, x*8, y*8, transparent_pens[tile >> 4]);
		}
	}
}



/*************************************
 *
 *  Space Odyssey background rendering
 *
 *************************************/

static void draw_background_spaceod(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	bitmap_t *pixmap = tilemap_get_pixmap(!(spaceod_bg_control & 0x02) ? spaceod_bg_htilemap : spaceod_bg_vtilemap);
	int flipmask = (spaceod_bg_control & 0x01) ? 0xff : 0x00;
	int xoffset = (spaceod_bg_control & 0x02) ? 0x10 : 0x00;
	int xmask = pixmap->width - 1;
	int ymask = pixmap->height - 1;
	int x, y;

	/* The H and V counters on this board are independent of the ones on */
	/* the main board. The H counter starts counting from 0 when EXT BLK */
	/* goes to 0; this coincides with H=0, so that's fine. However, the V */
	/* counter starts counting from 0 when VSYNC=0, which happens at line */
	/* 240, giving us an offset of (262-240) = 22 scanlines. */

	/* now fill in the background wherever there are black pixels */
	for (y = cliprect->min_y; y <= cliprect->max_y; y++)
	{
		int effy = (y + spaceod_vcounter + 22) ^ flipmask;
		UINT16 *src = (UINT16 *)pixmap->base + (effy & ymask) * pixmap->rowpixels;
		UINT16 *dst = (UINT16 *)bitmap->base + y * bitmap->rowpixels;

		/* loop over horizontal pixels */
		for (x = cliprect->min_x; x <= cliprect->max_x; x++)
		{
			int effx = ((x + spaceod_hcounter) ^ flipmask) + xoffset;
			UINT8 fgpix = machine->generic.paletteram.u8[dst[x]];
			UINT8 bgpix = src[effx & xmask] & 0x3f;

			/* the background detect flag is set if:
                - bgpix != 0 AND
                - fgpix != 0 AND
                - the original tile color == DIP switches
            */
			if (bgpix != 0 && fgpix != 0 && (dst[x] >> 2) == spaceod_bg_detect_tile_color)
				spaceod_bg_detect = 1;

			/* the background graphics are only displayed if:
                - fgpix == 0 AND
                - !EXTBLK (not blanked) AND
                - bg_enable == 0
            */
			if (fgpix == 0 && bg_enable == 0)
				dst[x] = bgpix | spaceod_fixed_color | 0x40;
		}
	}
}



/*************************************
 *
 *  Draw a background with only page
 *  granular scrolling
 *
 *************************************/

static void draw_background_page_scroll(bitmap_t *bitmap, const rectangle *cliprect)
{
	bitmap_t *pixmap = tilemap_get_pixmap(bg_tilemap);
	int flipmask = (video_control & 0x08) ? 0xff : 0x00;
	int xmask = pixmap->width - 1;
	int ymask = pixmap->height - 1;
	int x, y;

	/* if disabled, draw nothing */
	if (!bg_enable)
	{
		bitmap_fill(bitmap, cliprect, 0);
		return;
	}

	/* now fill in the background wherever there are black pixels */
	for (y = cliprect->min_y; y <= cliprect->max_y; y++)
	{
		int effy = bg_scrolly + (((y ^ flipmask) + (flipmask & 0xe0)) & 0xff);
		UINT16 *src = (UINT16 *)pixmap->base + (effy & ymask) * pixmap->rowpixels;
		UINT16 *dst = (UINT16 *)bitmap->base + y * bitmap->rowpixels;

		/* loop over horizontal pixels */
		for (x = cliprect->min_x; x <= cliprect->max_x; x++)
		{
			int effx = bg_scrollx + (x ^ flipmask);
			dst[x] = src[effx & xmask];
		}
	}
}



/*************************************
 *
 *  Draw a background with full pixel
 *  level scrolling
 *
 *************************************/

static void draw_background_full_scroll(bitmap_t *bitmap, const rectangle *cliprect)
{
	bitmap_t *pixmap = tilemap_get_pixmap(bg_tilemap);
	int flipmask = (video_control & 0x08) ? 0x3ff : 0x000;
	int xmask = pixmap->width - 1;
	int ymask = pixmap->height - 1;
	int x, y;

	/* if disabled, draw nothing */
	if (!bg_enable)
	{
		bitmap_fill(bitmap, cliprect, 0);
		return;
	}

	/* now fill in the background wherever there are black pixels */
	for (y = cliprect->min_y; y <= cliprect->max_y; y++)
	{
		int effy = (y + bg_scrolly) ^ flipmask;
		UINT16 *src = (UINT16 *)pixmap->base + (effy & ymask) * pixmap->rowpixels;
		UINT16 *dst = (UINT16 *)bitmap->base + y * bitmap->rowpixels;

		/* loop over horizontal pixels */
		for (x = cliprect->min_x; x <= cliprect->max_x; x++)
		{
			int effx = (x + bg_scrollx) ^ flipmask;
			dst[x] = src[effx & xmask];
		}
	}
}



/*************************************
 *
 *  Generic video update
 *
 *************************************/

VIDEO_UPDATE( segag80r )
{
	UINT8 transparent_pens[16];

	switch (segag80r_background_pcb)
	{
		/* foreground: opaque */
		/* background: none */
		case G80_BACKGROUND_NONE:
			memset(transparent_pens, 0, 16);
			draw_videoram(screen->machine, bitmap, cliprect, transparent_pens);
			break;

		/* foreground: visible except where black */
		/* background: Space Odyssey special */
		/* we draw the foreground first, then the background to do collision detection */
		case G80_BACKGROUND_SPACEOD:
			memset(transparent_pens, 0, 16);
			draw_videoram(screen->machine, bitmap, cliprect, transparent_pens);
			draw_background_spaceod(screen->machine, bitmap, cliprect);
			break;

		/* foreground: visible except for pen 0 (this disagrees with schematics) */
		/* background: page-granular scrolling */
		case G80_BACKGROUND_MONSTERB:
			memset(transparent_pens, 1, 16);
			draw_background_page_scroll(bitmap, cliprect);
			draw_videoram(screen->machine, bitmap, cliprect, transparent_pens);
			break;

		/* foreground: visible except for pen 0 */
		/* background: full scrolling */
		case G80_BACKGROUND_PIGNEWT:
			memset(transparent_pens, 1, 16);
			draw_background_full_scroll(bitmap, cliprect);
			draw_videoram(screen->machine, bitmap, cliprect, transparent_pens);
			break;

		/* foreground: visible except for pen 0 */
		/* background: page-granular scrolling */
		case G80_BACKGROUND_SINDBADM:
			memset(transparent_pens, 1, 16);
			draw_background_page_scroll(bitmap, cliprect);
			draw_videoram(screen->machine, bitmap, cliprect, transparent_pens);
			break;
	}
	return 0;
}
