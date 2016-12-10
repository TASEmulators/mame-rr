/***************************************************************************

    Lock-On video hardware

***************************************************************************/

#include "emu.h"
#include "includes/lockon.h"
#include "cpu/nec/nec.h"
#include "video/resnet.h"

#define CURSOR_XPOS			168
#define CURSOR_YPOS			239
#define FRAMEBUFFER_MAX_X	431
#define FRAMEBUFFER_MAX_Y	(UINT32)((FRAMEBUFFER_CLOCK / (float)(FRAMEBUFFER_MAX_X-1)) / ((float)PIXEL_CLOCK/(HTOTAL*VTOTAL)))


/*************************************
 *
 *  HD46505S-2 CRT Controller
 *
 *************************************/

READ16_HANDLER( lockon_crtc_r )
{
	return 0xffff;
}

WRITE16_HANDLER( lockon_crtc_w )
{
#if 0
	data &= 0xff;

	if (offset == 0)
	{
		switch (data)
		{
			case 0x00: mame_printf_debug("Horizontal Total         "); break;
			case 0x01: mame_printf_debug("Horizontal displayed     "); break;
			case 0x02: mame_printf_debug("Horizontal sync position "); break;
			case 0x03: mame_printf_debug("Horizontal sync width    "); break;
			case 0x04: mame_printf_debug("Vertical total           "); break;
			case 0x05: mame_printf_debug("Vertical total adjust    "); break;
			case 0x06: mame_printf_debug("Vertical displayed       "); break;
			case 0x07: mame_printf_debug("Vertical sync position   "); break;
			case 0x08: mame_printf_debug("Interlace mode           "); break;
			case 0x09: mame_printf_debug("Max. scan line address   "); break;
			case 0x0a: mame_printf_debug("Cursror start            "); break;
			case 0x0b: mame_printf_debug("Cursor end               "); break;
			case 0x0c: mame_printf_debug("Start address (h)        "); break;
			case 0x0d: mame_printf_debug("Start address (l)        "); break;
			case 0x0e: mame_printf_debug("Cursor (h)               "); break;
			case 0x0f: mame_printf_debug("Cursor (l)               "); break;
			case 0x10: mame_printf_debug("Light pen (h))           "); break;
			case 0x11: mame_printf_debug("Light pen (l)            "); break;
		}
	}
	else if (offset == 1)
	{
		mame_printf_debug("0x%.2x, (%d)\n",data, data);
	}
#endif
}

static TIMER_CALLBACK( cursor_callback )
{
	lockon_state *state = (lockon_state *)machine->driver_data;

	if (state->main_inten)
		cpu_set_input_line_and_vector(state->maincpu, 0, HOLD_LINE, 0xff);

	timer_adjust_oneshot(state->cursor_timer, machine->primary_screen->time_until_pos(CURSOR_YPOS, CURSOR_XPOS), 0);
}

/*************************************
 *
 *  Palette decoding
 *
 *************************************/

static const res_net_info lockon_net_info =
{
	RES_NET_VCC_5V | RES_NET_VBIAS_5V | RES_NET_VIN_TTL_OUT,
	{
		{RES_NET_AMP_NONE, 560, 0, 5, {4700, 2200, 1000, 470, 220}},
		{RES_NET_AMP_NONE, 560, 0, 5, {4700, 2200, 1000, 470, 220}},
		{RES_NET_AMP_NONE, 560, 0, 5, {4700, 2200, 1000, 470, 220}}
	}
};

static const res_net_info lockon_pd_net_info =
{
	RES_NET_VCC_5V | RES_NET_VBIAS_5V | RES_NET_VIN_TTL_OUT,
	{
		{RES_NET_AMP_NONE, 560, 580, 5, {4700, 2200, 1000, 470, 220}},
		{RES_NET_AMP_NONE, 560, 580, 5, {4700, 2200, 1000, 470, 220}},
		{RES_NET_AMP_NONE, 560, 580, 5, {4700, 2200, 1000, 470, 220}}
	}
};

PALETTE_INIT( lockon )
{
	int i;

	for (i = 0; i < 1024; ++i)
	{
		UINT8 r, g, b;
		UINT8 p1 = color_prom[i];
		UINT8 p2 = color_prom[i + 0x400];

		if (p2 & 0x80)
		{
			r = compute_res_net((p2 >> 2) & 0x1f, 0, &lockon_net_info);
			g = compute_res_net(((p1 >> 5) & 0x7) | (p2 & 3) << 3, 1, &lockon_net_info);
			b = compute_res_net((p1 & 0x1f), 2, &lockon_net_info);
		}
		else
		{
			r = compute_res_net((p2 >> 2) & 0x1f, 0, &lockon_pd_net_info);
			g = compute_res_net(((p1 >> 5) & 0x7) | (p2 & 3) << 3, 1, &lockon_pd_net_info);
			b = compute_res_net((p1 & 0x1f), 2, &lockon_pd_net_info);
		}

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
	}
}


/*************************************
 *
 *  Character tilemap handling
 *
 *************************************/

WRITE16_HANDLER( lockon_char_w )
{
	lockon_state *state = (lockon_state *)space->machine->driver_data;
	state->char_ram[offset] = data;
	tilemap_mark_tile_dirty(state->tilemap, offset);
}

static TILE_GET_INFO( get_lockon_tile_info )
{
	lockon_state *state = (lockon_state *)machine->driver_data;
	UINT32 tileno = state->char_ram[tile_index] & 0x03ff;
	UINT32 col = (state->char_ram[tile_index] >> 10) & 0x3f;

	col = (col & 0x1f) + (col & 0x20 ? 64 : 0);
	SET_TILE_INFO(0, tileno, col, 0);
}


/*******************************************************************************************

  Scene tilemap hardware

*******************************************************************************************/

WRITE16_HANDLER( lockon_scene_h_scr_w )
{
	lockon_state *state = (lockon_state *)space->machine->driver_data;
	state->scroll_h = data & 0x1ff;
}

WRITE16_HANDLER( lockon_scene_v_scr_w )
{
	lockon_state *state = (lockon_state *)space->machine->driver_data;
	state->scroll_v = data & 0x81ff;
}

static void scene_draw( running_machine *machine )
{
	lockon_state *state = (lockon_state *)machine->driver_data;
	UINT32 y;

	/* 3bpp characters */
	const UINT8 *const gfx1 = memory_region(machine, "gfx2");
	const UINT8 *const gfx2 = gfx1 + 0x10000;
	const UINT8 *const gfx3 = gfx1 + 0x20000;
	const UINT8 *const clut = gfx1 + 0x30000;

	for (y = 0; y < FRAMEBUFFER_MAX_Y; ++y)
	{
		UINT32 x;
		UINT32 d0 = 0, d1 = 0, d2 = 0;
		UINT32 colour = 0;
		UINT32 y_offs;
		UINT32 x_offs;
		UINT32 y_gran;
		UINT16 *bmpaddr;
		UINT32 ram_mask = 0x7ff;

		y_offs = (y + state->scroll_v) & 0x1ff;

		/* Clamp - stops tilemap wrapping when screen is rotated */
		if (BIT(state->scroll_v, 15) && y_offs & 0x100)
			ram_mask = 0x7;

		x_offs = (state->scroll_h - 8) & 0x1ff;
		y_gran = y_offs & 7;

		if (x_offs & 7)
		{
			UINT32 tileidx;
			UINT16 addr = ((y_offs & ~7) << 3) + ((x_offs >> 3) & 0x3f);
			UINT16 ram_val = state->scene_ram[addr & ram_mask];

			colour = (clut[ram_val & 0x7fff] & 0x3f) << 3;
			tileidx = ((ram_val & 0x0fff) << 3) + y_gran;

			d0 = *(gfx1 + tileidx);
			d1 = *(gfx2 + tileidx);
			d2 = *(gfx3 + tileidx);
		}

		bmpaddr = BITMAP_ADDR16(state->back_buffer, y, 0);

		for (x = 0; x < FRAMEBUFFER_MAX_X; ++x)
		{
			UINT32 x_gran = (x_offs & 7) ^ 7;
			UINT32 col;

			if (!(x_offs & 7))
			{
				UINT32 tileidx;
				UINT16 addr = ((y_offs & ~7) << 3) + ((x_offs >> 3) & 0x3f);
				UINT16 ram_val = state->scene_ram[addr & ram_mask];

				colour = (clut[ram_val & 0x7fff] & 0x3f) << 3;
				tileidx = ((ram_val & 0x0fff) << 3) + y_gran;

				d0 = *(gfx1 + tileidx);
				d1 = *(gfx2 + tileidx);
				d2 = *(gfx3 + tileidx);
			}

			col = colour
				 | (((d2 >> x_gran) & 1) << 2)
				 | (((d1 >> x_gran) & 1) << 1)
				 | ( (d0 >> x_gran) & 1);

			*bmpaddr++ = 0xa00 + col;

			x_offs = (x_offs + 1) & 0x1ff;
		}

	}
}

/*******************************************************************************************

    Ground Hardware

    Each framebuffer line corresponds to a three word entry in ground RAM,
    starting from offset 3:

       FEDCBA9876543210
    0 |.............xxx|  Tile line (A0-A2 GFX ROMs)
      |...........xx...|  Tile index (A6-A5 GFX ROMs)
      |........xxx.....|  ROM lut address (A6-A4)
      |.xxxxxxx........|  ROM lut address (A13-A7)
      |x...............|  /Line enable

    1 |........xxxxxxxx|  TZ2213 value
      |xxxxxxxx........|  X offset

    2 |........xxxxxxxx|  TZ2213 DX
      |.......x........|  Carry
      |x...............|  End of list marker

    An 8-bit ground control register controls the following:

       76543210
      |......xx|  LUT ROM A15-A14
      |....xx..|  LUT ROM select
      |..xx....|  CLUT ROM A13-A12
      |.x......|  GFX ROM A15, CLUT ROM A14
      |x.......|  GFX ROM bank select (always 0 - only 1 bank is present)

 *******************************************************************************************/

WRITE16_HANDLER( lockon_ground_ctrl_w )
{
	lockon_state *state = (lockon_state *)space->machine->driver_data;
	state->ground_ctrl = data & 0xff;
}

static TIMER_CALLBACK( bufend_callback )
{
	lockon_state *state = (lockon_state *)machine->driver_data;
	cpu_set_input_line_and_vector(state->ground, 0, HOLD_LINE, 0xff);
	cpu_set_input_line(state->object, NEC_INPUT_LINE_POLL, ASSERT_LINE);
}

/* Get data for a each 8x8x3 ground tile */
#define GET_GROUND_DATA()                                                                \
{                                                                                        \
	UINT32 gfx_a4_3  = (ls163 & 0xc) << 1;                                               \
	UINT32 lut_addr  = lut_address + ((ls163 >> 4) & 0xf);                               \
	UINT32 gfx_a14_7 = lut_rom[lut_addr] << 7;                                           \
	clut_addr = (lut_rom[lut_addr] << 4) | clut_a14_12 | clut_a4_3 | (ls163 & 0xc) >> 2; \
	gfx_addr  = gfx_a15 | gfx_a14_7 | gfx_a6_5 | gfx_a4_3 | gfx_a2_0;                    \
	pal = (clut_rom[clut_addr] << 3);                                                    \
	rom_data1 = gfx_rom[gfx_addr];                                                       \
	rom_data2 = gfx_rom[gfx_addr + 0x10000];                                             \
	rom_data3 = gfx_rom[gfx_addr + 0x20000];                                             \
}

static void ground_draw( running_machine *machine )
{
	lockon_state *state = (lockon_state *)machine->driver_data;

	/* ROM pointers */
	const UINT8 *const gfx_rom  = memory_region(machine, "gfx4");
	const UINT8 *const lut_rom  = gfx_rom + 0x30000 + ((state->ground_ctrl >> 2) & 0x3 ? 0x10000 : 0);
	const UINT8 *const clut_rom = gfx_rom + 0x50000;

	UINT32 lut_a15_14	= (state->ground_ctrl & 0x3) << 14;
	UINT32 clut_a14_12 = (state->ground_ctrl & 0x70) << 8;
	UINT32 gfx_a15 = (state->ground_ctrl & 0x40) << 9;
	UINT32 offs = 3;
	UINT32 y;

	/* TODO: Clean up and emulate CS of GFX ROMs? */
	for (y = 0; y < FRAMEBUFFER_MAX_Y; ++y)
	{
		UINT16 *bmpaddr = BITMAP_ADDR16(state->back_buffer, y, 0);
		UINT8 ls163;
		UINT32 clut_addr;
		UINT32 gfx_addr;
		UINT8 rom_data1 = 0;
		UINT8 rom_data2 = 0;
		UINT8 rom_data3 = 0;
		UINT32 pal = 0;
		UINT32 x;

		/* Draw this line? */
		if (!(state->ground_ram[offs] & 0x8000))
		{
			UINT32 gfx_a2_0  =  state->ground_ram[offs] & 0x0007;
			UINT32 gfx_a6_5  = (state->ground_ram[offs] & 0x0018) << 2;
			UINT32 clut_a4_3 = (state->ground_ram[offs] & 0x0018) >> 1;
			UINT8	tz2213_x  = state->ground_ram[offs + 1] & 0xff;
			UINT8	tz2213_dx = state->ground_ram[offs + 2] & 0xff;

			UINT32 lut_address = lut_a15_14 + ((state->ground_ram[offs] & 0x7fe0) >> 1);
			UINT32 cy = state->ground_ram[offs + 2] & 0x0100;
			UINT32 color;
			UINT32 gpbal2_0_prev;

			ls163 = state->ground_ram[offs + 1] >> 8;

			gpbal2_0_prev = ((ls163 & 3) << 1) | BIT(tz2213_x, 7);

			if (gpbal2_0_prev)
				GET_GROUND_DATA();

			for (x = 0; x < FRAMEBUFFER_MAX_X; x++)
			{
				UINT32 tz2213_cy;
				UINT32 gpbal2_0 = ((ls163 & 3) << 1) | BIT(tz2213_x, 7);

				/* Stepped into a new tile? */
				if (gpbal2_0 < gpbal2_0_prev)
					GET_GROUND_DATA();

				gpbal2_0_prev = gpbal2_0;

				color = pal;
				color += (rom_data1 >> gpbal2_0) & 0x1;
				color += ((rom_data2 >> gpbal2_0) & 0x1) << 1;
				color += ((rom_data3 >> gpbal2_0) & 0x1) << 2;

				*bmpaddr++ = 0x800 + color;

				/* Update the counters */
				tz2213_cy = (UINT8)tz2213_dx > (UINT8)~(tz2213_x);
				tz2213_x = (tz2213_x + tz2213_dx);

				/* Carry? */
				if (tz2213_cy || cy)
					++ls163;
			}
		}

		offs += 3;

		/* End of list marker */
		if (state->ground_ram[offs + 2] & 0x8000)
		{
			timer_adjust_oneshot(state->bufend_timer, attotime_mul(ATTOTIME_IN_HZ(FRAMEBUFFER_CLOCK), FRAMEBUFFER_MAX_X * y), 0);
		}
	}
}


/*******************************************************************************************

    Object hardware

    Customs (4 each, 1 per bitplane)
    TZA118 - Scaling
    TZ4203 - Objects with integrated line buffer.

       FEDCBA9876543210
    0 |......xxxxxxxxxx|  Y position
      |....xx..........|  Object Y size
      |..xx............|  Object X size
      |.x..............|  Y flip
      |x...............|  X flip
    1 |........xxxxxxxx|  X/Y scale
      |.xxxxxxx........|  Colour
      |x...............|  End of list marker
    2 |xxxxxxxxxxxxxxxx|  Chunk ROM address
    3 |.....xxxxxxxxxxx|  X position

*******************************************************************************************/

/*
   There's logic to prevent shadow pixels from being drawn against the scene tilemap,
   so that shadows don't appear against the sky.
*/
#define DRAW_OBJ_PIXEL(COLOR)                            \
do {                                                     \
	if (px < FRAMEBUFFER_MAX_X)							 \
	if (COLOR != 0xf)                                    \
	{                                                    \
		UINT8 clr = state->obj_pal_ram[(pal << 4) + COLOR];     \
		UINT16 *pix = (line + px);						 \
		if (!(clr == 0xff && ((*pix & 0xe00) == 0xa00))) \
			*pix = 0x400 + clr;			 \
	}                                                    \
	px = (px + 1) & 0x7ff;                               \
} while(0)

static void objects_draw( running_machine *machine )
{
	UINT32 offs;
	lockon_state *state = (lockon_state *)machine->driver_data;

	const UINT8  *const romlut = memory_region(machine, "user1");
	const UINT16 *const chklut = (UINT16*)memory_region(machine, "user2");
	const UINT8  *const gfxrom = memory_region(machine, "gfx5");
	const UINT8  *const sproms = memory_region(machine, "proms") + 0x800;

	for (offs = 0; offs < state->objectram_size; offs += 4)
	{
		UINT32 y;
		UINT32 xpos;
		UINT32 ypos;
		UINT32 xsize;
		UINT32 ysize;
		UINT32 xflip;
		UINT32 yflip;
		UINT32 scale;
		UINT32 pal;
		UINT32 lines;
		UINT32 opsta;
		UINT32 opsta15_8;

		/* Retrieve the object attributes */
		ypos	= state->object_ram[offs] & 0x03ff;
		xpos	= state->object_ram[offs + 3] & 0x07ff;
		ysize	= (state->object_ram[offs] >> 10) & 0x3;
		xsize	= (state->object_ram[offs] >> 12) & 0x3;
		yflip	= BIT(state->object_ram[offs], 14);
		xflip	= BIT(state->object_ram[offs], 15);
		scale	= state->object_ram[offs + 1] & 0xff;
		pal = (state->object_ram[offs + 1] >> 8) & 0x7f;
		opsta	= state->object_ram[offs + 2];

		if (state->iden)
		{
			state->obj_pal_ram[(pal << 4) + state->obj_pal_addr] = state->obj_pal_latch;
			break;
		}

		/* How many lines will this sprite occupy? The PAL @ IC154 knows... */
		lines = scale >> (3 - ysize);

		opsta15_8 = opsta & 0xff00;

		/* Account for line buffering */
		ypos -=1;

		for (y = 0; y < FRAMEBUFFER_MAX_Y; y++)
		{
			UINT32 cy = (y + ypos) & 0x3ff;
			UINT32 optab;
			UINT32 lutaddr;
			UINT32 tile;
			UINT8	cnt;
			UINT32 yidx;
			UINT16 *line = BITMAP_ADDR16(state->back_buffer, y, 0);
			UINT32 px = xpos;

			/* Outside the limits? */
			if (cy & 0x300)
				continue;

			if ((cy  & 0xff) >= lines)
				break;

			lutaddr = (scale & 0x80 ? 0x8000 : 0) | ((scale & 0x7f) << 8) | (cy & 0xff);
			optab = romlut[lutaddr] & 0x7f;

			if (yflip)
				optab ^= 7;

			yidx = (optab & 7);

			/* Now calculate the lower 7-bits of the LUT ROM address. PAL @ IC157 does this */
			cnt = (optab >> 3) * (1 << xsize);

			if (xflip)
				cnt ^= 7 >> (3 - xsize);
			if (yflip)
				cnt ^= (0xf >> (3 - ysize)) * (1 << xsize);

			cnt = (cnt + (opsta & 0xff));

			/* Draw! */
			for (tile = 0; tile < (1 << xsize); ++tile)
			{
				UINT16 sc;
				UINT16 scl;
				UINT32 x;
				UINT32 tileaddr;
				UINT16 td0, td1, td2, td3;
				UINT32 j;
				UINT32 bank;

				scl = scale & 0x7f;
				tileaddr = (chklut[opsta15_8 + cnt] & 0x7fff);
				bank = ((tileaddr >> 12) & 3) * 0x40000;
				tileaddr = bank + ((tileaddr & ~0xf000) << 3);

				if (xflip)
					--cnt;
				else
					++cnt;

				/* Draw two 8x8 tiles */
				for (j = 0; j < 2; ++j)
				{
					/* Get tile data */
					UINT32 tileadd = tileaddr + (0x20000 * (j ^ xflip));

					/* Retrieve scale values from PROMs */
					sc = sproms[(scl << 4) + (tile * 2) + j];

					/* Data from ROMs is inverted */
					td3 = gfxrom[tileadd + yidx] ^ 0xff;
					td2 = gfxrom[tileadd + 0x8000 + yidx] ^ 0xff;
					td1 = gfxrom[tileadd + 0x10000 + yidx] ^ 0xff;
					td0 = gfxrom[tileadd + 0x18000 + yidx] ^ 0xff;

					if (scale & 0x80)
					{
						for (x = 0; x < 8; ++x)
						{
							UINT8 col;
							UINT8 pix = x;

							if (!xflip)
								pix ^= 0x7;

							col = BIT(td0, pix)
								| (BIT(td1, pix) << 1)
								| (BIT(td2, pix) << 2)
								| (BIT(td3, pix) << 3);

							DRAW_OBJ_PIXEL(col);

							if (BIT(sc, x))
								DRAW_OBJ_PIXEL(col);
						}
					}
					else
					{
						for (x = 0; x < 8; ++x)
						{
							UINT8 col;
							UINT8 pix = x;

							if (BIT(sc, x))
							{
								if (!xflip)
									pix ^= 0x7;

								col = BIT(td0, pix)
									| (BIT(td1, pix) << 1)
									| (BIT(td2, pix) << 2)
									| (BIT(td3, pix) << 3);

								DRAW_OBJ_PIXEL(col);
							}
						}
					}
				}
			}
		}

		/* Check for the end of list marker */
		if (state->object_ram[offs + 1] & 0x8000)
			return;
	}
}

/* The mechanism used by the object CPU to update the object ASICs palette RAM */
WRITE16_HANDLER( lockon_tza112_w )
{
	lockon_state *state = (lockon_state *)space->machine->driver_data;

	if (state->iden)
	{
		state->obj_pal_latch = data & 0xff;
		state->obj_pal_addr = offset & 0xf;
		objects_draw(space->machine);
	}
}

READ16_HANDLER( lockon_obj_4000_r )
{
	lockon_state *state = (lockon_state *)space->machine->driver_data;

	cpu_set_input_line(state->object, NEC_INPUT_LINE_POLL, CLEAR_LINE);
	return 0xffff;
}

WRITE16_HANDLER( lockon_obj_4000_w )
{
	lockon_state *state = (lockon_state *)space->machine->driver_data;
	state->iden = data & 1;
}


/*******************************************************************************************

    Frame buffer rotation hardware

      FEDCBA9876543210
    0 |........xxxxxxxx|  X start address
      |.......x........|  Direction
    1 |........xxxxxxxx|  TZ2213 IC65 value
      |.......x........|  TZ2213 IC65 /enable
    2 |........xxxxxxxx|  TZ2213 IC65 delta
    3 |........xxxxxxxx|  TZ2213 IC106 delta
      |.......x........|  TZ2213 IC106 enable
    4 |.......xxxxxxxxx|  Y start address
    5 |........xxxxxxxx|  TZ2213 IC66 value
    6 |........xxxxxxxx|  TZ2213 IC66 delta
      |.......x........|  TZ2213 IC65 /enable
    7 |........xxxxxxxx|  TZ2213 IC107 delta
      |.......x........|  TZ2213 /enable
      |......x.........|  Direction

*******************************************************************************************/

WRITE16_HANDLER( lockon_fb_clut_w )
{
	rgb_t color;

	color = palette_get_color(space->machine, 0x300 + (data & 0xff));
	palette_set_color(space->machine, 0x400 + offset, color);
}

/* Rotation control register */
WRITE16_HANDLER( lockon_rotate_w )
{
	lockon_state *state = (lockon_state *)space->machine->driver_data;

	switch (offset & 7)
	{
		case 0: state->xsal  = data & 0x1ff;	break;
		case 1: state->x0ll  = data & 0xff;	break;
		case 2: state->dx0ll = data & 0x1ff;	break;
		case 3: state->dxll  = data & 0x1ff;	break;

		case 4: state->ysal  = data & 0x1ff;	break;
		case 5: state->y0ll  = data & 0xff;	break;
		case 6: state->dy0ll =	data & 0x1ff;	break;
		case 7: state->dyll  = data & 0x3ff;	break;
	}
}

#define INCREMENT(ACC, CNT)              \
do {                                     \
	carry = (UINT8)d##ACC > (UINT8)~ACC; \
	ACC += d##ACC;                       \
	if (carry) ++CNT;                    \
} while(0)

#define DECREMENT(ACC, CNT)              \
do {                                     \
	carry = (UINT8)d##ACC > (UINT8)ACC;  \
	ACC -= d##ACC;                       \
	if (carry) --CNT;                    \
} while(0)

static void rotate_draw( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	lockon_state *state = (lockon_state *)machine->driver_data;
	UINT32 y;

	/* Counters */
	UINT32 cxy = state->xsal & 0xff;
	UINT32 cyy = state->ysal & 0x1ff;

	/* Accumulator values and deltas */
	UINT8 axy  = state->x0ll & 0xff;
	UINT8 daxy = state->dx0ll & 0xff;
	UINT8 ayy  = state->y0ll & 0xff;
	UINT8 dayy = state->dy0ll & 0xff;
	UINT8 dayx = state->dyll & 0xff;
	UINT8 daxx = state->dxll & 0xff;

	UINT32 xy_up = BIT(state->xsal, 8);
	UINT32 yx_up = BIT(state->dyll, 9);
	UINT32 axx_en  = !BIT(state->dxll, 8);
	UINT32 ayx_en  = !BIT(state->dyll, 8);
	UINT32 axy_en  = !BIT(state->dx0ll, 8);
	UINT32 ayy_en  = !BIT(state->dy0ll, 8);

	for (y = 0; y <= cliprect->max_y; ++y)
	{
		UINT32 carry;
		UINT16 *dst = BITMAP_ADDR16(bitmap, y, 0);
		UINT32 x;

		UINT32 cx = cxy;
		UINT32 cy = cyy;

		UINT8 axx = axy;
		UINT8 ayx = ayy;

		for (x = 0; x <= cliprect->max_x; ++x)
		{
			cx &= 0x1ff;
			cy &= 0x1ff;

			*dst++ = *BITMAP_ADDR16(state->front_buffer, cy, cx);

			if (axx_en)
				INCREMENT(axx, cx);
			else
				++cx;

			if (ayx_en)
			{
				if (yx_up)
					INCREMENT(ayx, cy);
				else
					DECREMENT(ayx, cy);
			}
			else
			{
				if (yx_up)
					++cy;
				else
					--cy;
			}
		}

		if (axy_en)
		{
			if (xy_up)
				INCREMENT(axy, cxy);
			else
				DECREMENT(axy, cxy);
		}
		else
		{
			if (xy_up)
				++cxy;
			else
				--cxy;
		}

		if (ayy_en)
			INCREMENT(ayy, cyy);
		else
			++cyy;

		cxy &= 0xff;
		cyy &= 0x1ff;
	}
}


/*******************************************************************************************

    HUD Drawing Hardware

    A sprite layer that uses 8x8x1bpp tiles to form bigger sprites

    0  |.......xxxxxxxxx|  Y Position
       |xxxxxxx.........|  Code
    1  |.......xxxxxxxxx|  X Position
       |....xxx.........|  Colour
       |.xxx............|  Sprite width (0=8, 1=16, 2=24, 3=32 pixels, etc.)
       |x...............|  End of list marker

*******************************************************************************************/

static void hud_draw( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	lockon_state *state = (lockon_state *)machine->driver_data;
	UINT8	*tile_rom = memory_region(machine, "gfx3");
	UINT32 offs;

	for (offs = 0x0; offs <= state->hudram_size; offs += 2)
	{
		UINT32 y;
		UINT32 y_pos;
		UINT32 x_pos;
		UINT32 y_size;
		UINT32 x_size;
		UINT32 layout;
		UINT16 colour;
		UINT32 code;
		UINT32 rom_a12_7;

		/* End of sprite list marker */
		if (state->hud_ram[offs + 1] & 0x8000)
			break;

		y_pos	= state->hud_ram[offs] & 0x1ff;
		x_pos	= state->hud_ram[offs + 1] & 0x1ff;
		x_size = (state->hud_ram[offs + 1] >> 12) & 7;
		code	= (state->hud_ram[offs] >> 9) & 0x7f;
		colour = 0x200 + ((state->hud_ram[offs + 1] >> 9) & 7);
		layout = (code >> 5) & 3;

		rom_a12_7 = (code & 0xfe) << 6;

		/* Account for line buffering */
		y_pos -= 1;

		if (layout == 3)
			y_size = 32;
		else if (layout == 2)
			y_size = 16;
		else
			y_size = 8;

		for (y = cliprect->min_y; y <= cliprect->max_y; ++y)
		{
			UINT32 xt;
			UINT32 cy;

			cy = y_pos + y;

			if (cy < 0x200)
				continue;

			if ((cy & 0xff) == y_size)
				break;

			for (xt = 0; xt <= x_size; ++xt)
			{
				UINT32 rom_a6_3;
				UINT32 px;
				UINT8	gfx_strip;

				if (layout == 3)
					rom_a6_3 = (BIT(cy, 4) << 3) | (BIT(cy, 3) << 2) | (BIT(xt, 1) << 1) | BIT(xt, 0);
				else if (layout == 2)
					rom_a6_3 = ((BIT(code, 0) << 3) | (BIT(xt, 1) << 2) | (BIT(cy, 3) << 1) | (BIT(xt, 0)));
				else
					rom_a6_3 = (BIT(code, 0) << 3) | (xt & 7);

				rom_a6_3 <<= 3;

				/* Get tile data */
				gfx_strip = tile_rom[rom_a12_7 | rom_a6_3 | (cy & 7)];

				if (gfx_strip == 0)
					continue;

				/* Draw */
				for (px = 0; px < 8; ++px)
				{
					UINT32 x = x_pos + (xt << 3) + px;

					if (x <= cliprect->max_x)
					{
						UINT16 *dst = BITMAP_ADDR16(bitmap, y, x);

						if (BIT(gfx_strip, px ^ 7) && *dst > 255)
							*dst = colour;
					}
				}
			}
		}
	}
}


/*************************************
 *
 *  Driver video handlers
 *
 *************************************/

VIDEO_START( lockon )
{
	lockon_state *state = (lockon_state *)machine->driver_data;

	state->tilemap = tilemap_create(machine, get_lockon_tile_info, tilemap_scan_rows, 8, 8, 64, 32);
	tilemap_set_transparent_pen(state->tilemap, 0);

	/* Allocate the two frame buffers for rotation */
	state->back_buffer = auto_bitmap_alloc(machine, 512, 512, BITMAP_FORMAT_INDEXED16);
	state->front_buffer = auto_bitmap_alloc(machine, 512, 512, BITMAP_FORMAT_INDEXED16);

	/* 2kB of object ASIC palette RAM */
	state->obj_pal_ram = auto_alloc_array(machine, UINT8, 2048);

	/* Timer for ground display list callback */
	state->bufend_timer = timer_alloc(machine, bufend_callback, NULL);

	/* Timer for the CRTC cursor pulse */
	state->cursor_timer = timer_alloc(machine, cursor_callback, NULL);
	timer_adjust_oneshot(state->cursor_timer, machine->primary_screen->time_until_pos(CURSOR_YPOS, CURSOR_XPOS), 0);

	state_save_register_global_bitmap(machine, state->back_buffer);
	state_save_register_global_bitmap(machine, state->front_buffer);
	state_save_register_global_pointer(machine, state->obj_pal_ram, 2048);
}

VIDEO_UPDATE( lockon )
{
	lockon_state *state = (lockon_state *)screen->machine->driver_data;

	/* If screen output is disabled, fill with black */
	if (!BIT(state->ctrl_reg, 7))
	{
		bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));
		return 0;
	}

	/* Scan out the frame buffer in rotated order */
	rotate_draw(screen->machine, bitmap, cliprect);

	/* Draw the character tilemap */
	tilemap_draw(bitmap, cliprect, state->tilemap, 0, 0);

	/* Draw the HUD */
	hud_draw(screen->machine, bitmap, cliprect);

	return 0;
}

VIDEO_EOF( lockon )
{
	lockon_state *state = (lockon_state *)machine->driver_data;

	/* Swap the frame buffers */
	bitmap_t *tmp = state->front_buffer;
	state->front_buffer = state->back_buffer;
	state->back_buffer = tmp;

	/* Draw the frame buffer layers */
	scene_draw(machine);
	ground_draw(machine);
	objects_draw(machine);
}
