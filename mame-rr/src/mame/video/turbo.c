/*************************************************************************

    Sega Z80-3D system

*************************************************************************/

#include "emu.h"
#include "includes/turbo.h"
#include "video/resnet.h"



typedef struct _sprite_info sprite_info;
struct _sprite_info
{
	UINT16	ve;					/* VE0-15 signals for this row */
	UINT8	lst;				/* LST0-7 signals for this row */
	UINT32	latched[8];			/* latched pixel data */
	UINT8	plb[8];				/* latched PLB state */
	UINT32	offset[8];			/* current offset for this row */
	UINT32	frac[8];			/* leftover fraction */
	UINT32	step[8];			/* stepping value */
};

static const UINT32 sprite_expand[16] =
{
	0x00000000, 0x00000001, 0x00000100, 0x00000101,
	0x00010000, 0x00010001, 0x00010100, 0x00010101,
	0x01000000, 0x01000001, 0x01000100, 0x01000101,
	0x01010000, 0x01010001, 0x01010100, 0x01010101
};




/*************************************
 *
 *  Palette conversion
 *
 *************************************/

PALETTE_INIT( turbo )
{
	static const int resistances[3] = { 1000, 470, 220 };
	double rweights[3], gweights[3], bweights[2];
	int i;

	/* compute the color output resistor weights */
	compute_resistor_weights(0,	255, -1.0,
			3,	&resistances[0], rweights, 470, 0,
			3,	&resistances[0], gweights, 470, 0,
			2,	&resistances[1], bweights, 470, 0);

	/* initialize the palette with these colors */
	for (i = 0; i < 256; i++)
	{
		int bit0, bit1, bit2, r, g, b;

		/* red component */
		bit0 = (i >> 0) & 1;
		bit1 = (i >> 1) & 1;
		bit2 = (i >> 2) & 1;
		r = combine_3_weights(rweights, bit0, bit1, bit2);

		/* green component */
		bit0 = (i >> 3) & 1;
		bit1 = (i >> 4) & 1;
		bit2 = (i >> 5) & 1;
		g = combine_3_weights(gweights, bit0, bit1, bit2);

		/* blue component */
		bit0 = (i >> 6) & 1;
		bit1 = (i >> 7) & 1;
		b = combine_2_weights(bweights, bit0, bit1);

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
	}
}


PALETTE_INIT( subroc3d )
{
	static const int resistances[3] = { 1000, 470, 220 };
	double rweights[3], gweights[3], bweights[2];
	int i;

	/* compute the color output resistor weights */
	compute_resistor_weights(0,	255, -1.0,
			3,	&resistances[0], rweights, 470, 0,
			3,	&resistances[0], gweights, 470, 0,
			2,	&resistances[1], bweights, 470, 0);

	/* initialize the palette with these colors */
	for (i = 0; i < 256; i++)
	{
		int bit0, bit1, bit2, r, g, b;

		/* red component */
		bit0 = (i >> 0) & 1;
		bit1 = (i >> 1) & 1;
		bit2 = (i >> 2) & 1;
		r = combine_3_weights(rweights, bit0, bit1, bit2);

		/* green component */
		bit0 = (i >> 3) & 1;
		bit1 = (i >> 4) & 1;
		bit2 = (i >> 5) & 1;
		g = combine_3_weights(gweights, bit0, bit1, bit2);

		/* blue component */
		bit0 = (i >> 6) & 1;
		bit1 = (i >> 7) & 1;
		b = combine_2_weights(bweights, bit0, bit1);

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
	}
}


PALETTE_INIT( buckrog )
{
	static const int resistances[4] = { 2200, 1000, 500, 250 };
	double rweights[3], gweights[3], bweights[4];
	int i;

	/* compute the color output resistor weights */
	compute_resistor_weights(0,	255, -1.0,
			3,	&resistances[1], rweights, 1000, 0,
			3,	&resistances[1], gweights, 1000, 0,
			4,	&resistances[0], bweights, 1000, 0);

	/* initialize the palette with these colors */
	for (i = 0; i < 1024; i++)
	{
		int bit0, bit1, bit2, bit3, r, g, b;

		/* red component */
		bit0 = (i >> 0) & 1;
		bit1 = (i >> 1) & 1;
		bit2 = (i >> 2) & 1;
		r = combine_3_weights(rweights, bit0, bit1, bit2);

		/* green component */
		bit0 = (i >> 3) & 1;
		bit1 = (i >> 4) & 1;
		bit2 = (i >> 5) & 1;
		g = combine_3_weights(gweights, bit0, bit1, bit2);

		/* blue component - note the shuffled bits */
		bit0 = (i >> 8) & 1;
		bit1 = (i >> 9) & 1;
		bit2 = (i >> 6) & 1;
		bit3 = (i >> 7) & 1;
		b = combine_4_weights(bweights, bit0, bit1, bit2, bit3);

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
	}
}



/*************************************
 *
 *  Video startup
 *
 *************************************/

static TILE_GET_INFO( get_fg_tile_info )
{
	turbo_state *state = (turbo_state *)machine->driver_data;
	int code = state->videoram[tile_index];
	SET_TILE_INFO(0, code, code >> 2, 0);
}


VIDEO_START( turbo )
{
	turbo_state *state = (turbo_state *)machine->driver_data;

	/* initialize the foreground tilemap */
	state->fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows,  8,8, 32,32);
}


VIDEO_START( buckrog )
{
	turbo_state *state = (turbo_state *)machine->driver_data;

	/* initialize the foreground tilemap */
	state->fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows,  8,8, 32,32);

	/* allocate the bitmap RAM */
	state->buckrog_bitmap_ram = auto_alloc_array(machine, UINT8, 0xe000);
	state_save_register_global_pointer(machine, state->buckrog_bitmap_ram, 0xe000);
}



/*************************************
 *
 *  Videoram access
 *
 *************************************/

WRITE8_HANDLER( turbo_videoram_w )
{
	turbo_state *state = (turbo_state *)space->machine->driver_data;
	state->videoram[offset] = data;
	if (offset < 0x400)
	{
		space->machine->primary_screen->update_partial(space->machine->primary_screen->vpos());
		tilemap_mark_tile_dirty(state->fg_tilemap, offset);
	}
}


WRITE8_HANDLER( buckrog_bitmap_w )
{
	turbo_state *state = (turbo_state *)space->machine->driver_data;
	state->buckrog_bitmap_ram[offset] = data & 1;
}



/*************************************
 *
 *  Sprite X scaling
 *
 *************************************/

INLINE UINT32 sprite_xscale(UINT8 dacinput, double vr1, double vr2, double cext)
{
	/* compute the effective pixel clock for this sprite */
	/* thanks to Frank Palazzolo for figuring out this logic */

	/* compute the control voltage to the VCO */
	/* VR1 and VR2 are variable resistors on Turbo, fixed on other boards */
	double iref = 5.0 / (1.5e3 + vr2);
	double iout = iref * ((float)dacinput / 256.0);
	double vref = 5.0 * 1e3 / (3.8e3 + 1e3 + vr1);
	double vco_cv = (2.2e3 * iout) + vref;

	/* based on the control voltage, compute the frequency assuming a 50pF */
	/* external capacitor; this is the graph in the datasheet. Some attempt */
	/* to simulate the non-linearity at the edges has been made, but it is */
	/* admittedly cheesy. */
	double vco_freq;
	if (vco_cv > 5.0)
		vco_cv = 5.0;
	if (vco_cv < 0.0)
		vco_cv = 0.0;
	if (cext < 1e-11)
	{
		if (vco_cv < 1.33)
			vco_freq = (0.68129 + pow(vco_cv + 0.6, 1.285)) * 1e6;
		else if (vco_cv < 4.3)
			vco_freq = (3 + (8 - 3) * ((vco_cv - 1.33) / (4.3 - 1.33))) * 1e6;
		else
			vco_freq = (-1.560279 + pow(vco_cv - 4.3 + 6, 1.26)) * 1e6;

		/* now scale based on the actual external capacitor; the frequency goes */
		/* up by a factor of 10 for every factor of 10 the capacitance is reduced */
		/* approximately */
		vco_freq *= 50e-12 / cext;
	}
	else
	{
		/* based on figure 6 of datasheet */
		vco_freq = -0.9892942 * log10(cext)	- 0.0309697 * vco_cv * vco_cv
		              +	0.344079975 * vco_cv - 4.086395841;
		vco_freq = pow(10.0, vco_freq);
	}

	/* finally, convert to a fraction (8.24) of 5MHz, which is the pixel clock */
	return (UINT32)((vco_freq / (5e6 * TURBO_X_SCALE)) * 16777216.0);
}



/*************************************
 *
 *  Turbo sprite handling
 *
 *************************************/

static void turbo_prepare_sprites(running_machine *machine, turbo_state *state, UINT8 y, sprite_info *info)
{
	const UINT8 *pr1119 = memory_region(machine, "proms") + 0x200;
	int sprnum;

	/* initialize the line enable signals to 0 */
	info->ve = 0;
	info->lst = 0;

	/* compute the sprite information, which was done on the previous scanline during HBLANK */
	for (sprnum = 0; sprnum < 16; sprnum++)
	{
		UINT8 *rambase = &state->spriteram[sprnum * 0x10];
		int level = sprnum & 7;
		UINT8 clo, chi;
		UINT32 sum;

		/* perform the first ALU to see if we are within the scanline */
		sum = y + (rambase[0] ^ 0xff);
		clo = (sum >> 8) & 1;
		sum += (y << 8) + ((rambase[1] ^ 0xff) << 8);
		chi = (sum >> 16) & 1;

		/* the AND of the low carry and the inverse of the high carry clocks an enable bit */
		/* for this sprite; note that the logic in the Turbo schematics is reversed here */
		if (clo & (chi ^ 1))
		{
			int xscale = rambase[2] ^ 0xff;
			int yscale = rambase[3];// ^ 0xff;
			UINT16 offset = rambase[6] + (rambase[7] << 8);
			int offs;

			/* mark this entry enabled */
			info->ve |= 1 << sprnum;

			/* look up the low byte of the sum plus the yscale value in */
			/* IC50/PR1119 to determine if we write back the sum of the */
			/* offset and the rowbytes this scanline (p. 138) */
			offs = (sum & 0xff) |			/* A0-A7 = AL0-AL7 */
				   ((yscale & 0x08) << 5);	/* A8-A9 = /RO11-/RO12 */

			/* one of the bits is selected based on the low 7 bits of yscale */
			if (!((pr1119[offs] >> (yscale & 0x07)) & 1))
			{
				offset += rambase[4] + (rambase[5] << 8);
				rambase[6] = offset;
				rambase[7] = offset >> 8;
			}

			/* the output of the ALU here goes to the individual level counter */
			info->latched[level] = 0;
			info->plb[level] = 0;
			info->offset[level] = offset;
			info->frac[level] = 0;

			/*
                actual pots read from one board:
                    VR1 = 310 Ohm
                    VR2 = 910 Ohm
            */
			info->step[level] = sprite_xscale(xscale, 1.0e3 * input_port_read(machine, "VR1") / 100.0, 1.0e3 * input_port_read(machine, "VR2") / 100.0, 100e-12);
		}
	}
}


static UINT32 turbo_get_sprite_bits(running_machine *machine, UINT8 road, sprite_info *sprinfo)
{
	const UINT8 *sprite_gfxdata = memory_region(machine, "gfx1");
	UINT8 sprlive = sprinfo->lst;
	UINT32 sprdata = 0;
	int level;

	/* if we haven't left the road yet, sprites 3-7 are disabled */
	if (!road)
		sprlive &= 0x07;

	/* loop over all live levels */
	for (level = 0; level < 8; level++)
		if (sprlive & (1 << level))
		{
			/* latch the data and advance the offset */
			sprdata |= sprinfo->latched[level];
			sprinfo->frac[level] += sprinfo->step[level];

			/* if we're live and we've clocked more data, advance */
			while (sprinfo->frac[level] >= 0x1000000)
			{
				UINT16 offs = sprinfo->offset[level];
				UINT8 pixdata;

				/* bit 0 controls which half of the byte to use */
				/* bits 1-13 go to address lines */
				/* bit 14 selects which of the two ROMs to read from */
				pixdata = sprite_gfxdata[(level << 14) | ((offs >> 1) & 0x3fff)] >> ((~offs & 1) * 4);
				sprinfo->latched[level] = sprite_expand[pixdata & 0x0f] << level;

				/* if bit 3 is 0 and bit 2 is 1, the enable flip/flip is reset */
				if ((pixdata & 0x0c) == 0x04)
				{
					sprinfo->lst &= ~(1 << level);
					sprlive &= ~(1 << level);
				}

				/* if bit 15 is set, we decrement instead of increment */
				sprinfo->offset[level] += (offs & 0x8000) ? -1 : 1;
				sprinfo->frac[level] -= 0x1000000;
			}
		}

	return sprdata;
}



/*************************************
 *
 *  Turbo video update
 *
 *************************************/

VIDEO_UPDATE( turbo )
{
	turbo_state *state = (turbo_state *)screen->machine->driver_data;
	bitmap_t *fgpixmap = tilemap_get_pixmap(state->fg_tilemap);
	const UINT8 *road_gfxdata = memory_region(screen->machine, "gfx3");
	const UINT8 *prom_base = memory_region(screen->machine, "proms");
	const UINT8 *pr1114 = prom_base + 0x000;
	const UINT8 *pr1115 = prom_base + 0x020;
	const UINT8 *pr1116 = prom_base + 0x040;
	const UINT8 *pr1117 = prom_base + 0x060;
	const UINT8 *pr1118 = prom_base + 0x100;
	const UINT8 *pr1121 = prom_base + 0x600;
	const UINT8 *pr1122 = prom_base + 0x800;
	const UINT8 *pr1123 = prom_base + 0xc00;
	int x, y;

	/* loop over rows */
	for (y = cliprect->min_y; y <= cliprect->max_y; y++)
	{
		const UINT16 *fore = (UINT16 *)fgpixmap->base + y * fgpixmap->rowpixels;
		UINT16 *dest = (UINT16 *)bitmap->base + y * bitmap->rowpixels;
		int sel, coch, babit, slipar_acciar, area, offs, areatmp, road = 0;
		sprite_info sprinfo;

		/* compute the Y sum between opa and the current scanline (p. 141) */
		int va = (y + state->turbo_opa) & 0xff;

		/* the upper bit of OPC inverts the road (p. 141) */
		if (!(state->turbo_opc & 0x80))
			va ^= 0xff;

		/* compute the sprite information; we use y-1 since this info was computed during HBLANK */
		/* on the previous scanline */
		turbo_prepare_sprites(screen->machine, state, y, &sprinfo);

		/* loop over columns */
		for (x = 0; x <= cliprect->max_x; x += TURBO_X_SCALE)
		{
			int bacol, red, grn, blu, priority, foreraw, forebits, mx, ix;
			int xx = x / TURBO_X_SCALE;
			UINT8 carry;
			UINT32 sprbits;
			UINT16 he;

			/* load the bitmask from the sprite position for both halves of the sprites (p. 139) */
			he = state->sprite_position[xx] | (state->sprite_position[xx + 0x100] << 8);

			/* the AND of the line enable and horizontal enable is clocked and held in LST0-7 (p. 143) */
			he &= sprinfo.ve;
			sprinfo.lst |= he | (he >> 8);

			/* compute the X sum between opb and the current column; only the carry matters (p. 141) */
			carry = (xx + state->turbo_opb) >> 8;

			/* the carry selects which inputs to use (p. 141) */
			if (carry)
			{
				sel	 = state->turbo_ipb;
				coch = state->turbo_ipc >> 4;
			}
			else
			{
				sel	 = state->turbo_ipa;
				coch = state->turbo_ipc & 15;
			}

			/* look up AREA1 and AREA2 (p. 142) */
			offs = va |							/*  A0- A7 = VA0-VA7 */
				   ((sel & 0x0f) << 8);			/*  A8-A11 = SEL0-3 */

			areatmp = road_gfxdata[0x0000 | offs];
			areatmp = ((areatmp + xx) >> 8) & 0x01;
			area = areatmp << 0;

			areatmp = road_gfxdata[0x1000 | offs];
			areatmp = ((areatmp + xx) >> 8) & 0x01;
			area |= areatmp << 1;

			/* look up AREA3 and AREA4 (p. 142) */
			offs = va |							/*  A0- A7 = VA0-VA7 */
				   ((sel & 0xf0) << 4);			/*  A8-A11 = SEL4-7 */

			areatmp = road_gfxdata[0x2000 | offs];
			areatmp = ((areatmp + xx) >> 8) & 0x01;
			area |= areatmp << 2;

			areatmp = road_gfxdata[0x3000 | offs];
			areatmp = ((areatmp + xx) >> 8) & 0x01;
			area |= areatmp << 3;

			/* look up AREA5 (p. 141) */
			offs = (xx >> 3) |							/*  A0- A4 = H3-H7 */
				   ((state->turbo_opc & 0x3f) << 5);	/*  A5-A10 = OPC0-5 */

			areatmp = road_gfxdata[0x4000 | offs];
			areatmp = (areatmp << (xx & 7)) & 0x80;
			area |= areatmp >> 3;

			/* compute the final area value and look it up in IC18/PR1115 (p. 144) */
			/* note: SLIPAR is 0 on the road surface only */
			/*       ACCIAR is 0 on the road surface and the striped edges only */
			babit = pr1115[area];
			slipar_acciar = babit & 0x30;
			if (!road && (slipar_acciar & 0x20))
				road = 1;

			/* also use the coch value to look up color info in IC13/PR1114 and IC21/PR1117 (p. 144) */
			offs = (coch & 0x0f) |						/* A0-A3: CONT0-3 = COCH0-3 */
				   ((state->turbo_fbcol & 0x01) << 4);	/*    A4: COL0 */
			bacol = pr1114[offs] | (pr1117[offs] << 8);

			/* at this point, do the character lookup; due to the shift register loading in */
			/* the sync PROM, we latch character 0 during pixel 6 and start clocking in pixel */
			/* 8, effectively shifting the display by 8; at pixel 0x108, the color latch is */
			/* forced clear and isn't touched until the next shift register load */
			foreraw = (xx < 8 || xx >= 0x108) ? 0 : fore[xx - 8];

			/* perform the foreground color table lookup in IC99/PR1118 (p. 137) */
			forebits = pr1118[foreraw];

			/* now that we have done all the per-5MHz pixel work, mix the sprites at the scale factor */
			for (ix = 0; ix < TURBO_X_SCALE; ix++)
			{
				/* iterate over live sprites and update them */
				/* the final 32-bit value is: */
				/*    CDB0-7 = D0 -D7  */
				/*    CDG0-7 = D8 -D15 */
				/*    CDR0-7 = D16-D23 */
				/*    PLB0-7 = D24-D31 */
				sprbits = turbo_get_sprite_bits(screen->machine, road, &sprinfo);

				/* perform collision detection here via lookup in IC20/PR1116 (p. 144) */
				state->turbo_collision |= pr1116[((sprbits >> 24) & 7) | (slipar_acciar >> 1)];

				/* look up the sprite priority in IC11/PR1122 (p. 144) */
				priority = ((sprbits & 0xfe000000) >> 25) |		/* A0-A6: PLB1-7 */
						   ((state->turbo_fbpla & 0x07) << 7);	/* A7-A9: PLA0-2 */
				priority = pr1122[priority];

				/* use that to look up the overall priority in IC12/PR1123 (p. 144) */
				mx = (priority & 7) |						/* A0-A2: PR-1122 output, bits 0-2 */
					 ((sprbits & 0x01000000) >> 21) |		/*    A3: PLB0 */
					 ((foreraw & 0x80) >> 3) |				/*    A4: PLBE */
					 ((forebits & 0x08) << 2) | 			/*    A5: PLBF */
					 ((babit & 0x07) << 6) |				/* A6-A8: BABIT1-3 */
					 ((state->turbo_fbpla & 0x08) << 6);	/*    A9: PLA3 */
				mx = pr1123[mx];

				/* the MX output selects one of 16 inputs; build up a 16-bit pattern to match */
				/* these in red, green, and blue (p. 144) */
				red = ((sprbits & 0x0000ff) >> 0) |		/*  D0- D7: CDR0-CDR7 */
					  ((forebits & 0x01) << 8) |		/*      D8: CDRF */
					  ((bacol & 0x001f) << 9) |			/*  D9-D13: BAR0-BAR4 */
					  (1 << 14) |						/*     D14: 1 */
					  (0 << 15);						/*     D15: 0 */

				grn = ((sprbits & 0x00ff00) >> 8) |		/*  D0- D7: CDG0-CDG7 */
					  ((forebits & 0x02) << 7) |		/*      D8: CDGF */
					  ((bacol & 0x03e0) << 4) |			/*  D9-D13: BAG0-BAG4 */
					  (1 << 14) |						/*     D14: 1 */
					  (0 << 15);						/*     D15: 0 */

				blu = ((sprbits & 0xff0000) >> 16) |	/*  D0- D7: CDB0-CDB7 */
					  ((forebits & 0x04) << 6) |		/*      D8: CDBF */
					  ((bacol & 0x7c00) >> 1) |			/*  D9-D13: BAB0-BAB4 */
					  (1 << 14) |						/*     D14: 1 */
					  (0 << 15);						/*     D15: 0 */

				/* we then go through a muxer to select one of the 16 outputs computed above (p. 144) */
				offs = mx |								/* A0-A3: MX0-MX3 */
					   (((~red >> mx) & 1) << 4) |		/*    A4: CDR */
					   (((~grn >> mx) & 1) << 5) |		/*    A5: CDG */
					   (((~blu >> mx) & 1) << 6) |		/*    A6: CDB */
					   ((state->turbo_fbcol & 6) << 6);	/* A7-A8: COL1-2 */
				dest[x + ix] = pr1121[offs];
			}
		}
	}
	return 0;
}



/*************************************
 *
 *  Subroc 3D sprite handling
 *
 *************************************/

/*
    Sprite state machine:

    1LINE = 0 (V & 0x108 == 0x108)
    ---------
           0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
           20 21 21 20 20 21 21 20 20 05 25 2c 07 e7 37 22
               _____       _____       _____    ________
    RAD0 = ___|     |_____|     |_____|     |__|        |___
                                                ___________
    RAD7 = ____________________________________|           |
                                       _________________
    YCULL= ___________________________|                 |___
                                             __
    AX   = _________________________________|  |____________
                                                      __
    D/A  = __________________________________________|  |___
           ___________________________    _____    _________
    /CLK1=                            |__|     |__|
                                                   __
    WRPL = _______________________________________|  |______
                                                   __
    /CLK2= _______________________________________|  |______



    1LINE = 1 (V & 0x108 != 0x108)
    ---------
           0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
           2c 2c 2c 2f 2f 2f 2f 20 2d 2d 2d 2d 0f 6f 2f 20
                     ___________    ____________________
    RAD0 = _________|           |__|                    |___
                     ___________                ________
    RAD7 = _________|           |______________|        |___
           _____________________    ____________________
    YCULL=                      |__|                    |___
           _____________________    ____________________
    AX   =                      |__|                    |___

    D/A  = _________________________________________________
           ____________________________________    _________
    /CLK1=                                     |__|
                                                   __
    WRPL = _______________________________________|  |______

    /CLK2= _________________________________________________

*/

static void subroc3d_prepare_sprites(running_machine *machine, turbo_state *state, UINT8 y, sprite_info *info)
{
	const UINT8 *pr1449 = memory_region(machine, "proms") + 0x300;
	int sprnum;

	/* initialize the line enable signals to 0 */
	info->ve = 0;
	info->lst = 0;

	/* compute the sprite information, which was done on the previous scanline during HBLANK */
	for (sprnum = 0; sprnum < 16; sprnum++)
	{
		UINT8 *rambase = &state->spriteram[sprnum * 8];
		int level = sprnum & 7;
		UINT8 clo, chi;
		UINT32 sum;

		/* perform the first ALU to see if we are within the scanline */
		sum = y + (rambase[0]/* ^ 0xff*/);
		clo = (sum >> 8) & 1;
		sum += (y << 8) + ((rambase[1]/* ^ 0xff*/) << 8);
		chi = (sum >> 16) & 1;

		/* the AND of the low carry and the inverse of the high carry clocks an enable bit */
		/* for this sprite; note that the logic in the Turbo schematics is reversed here */
		if (clo & (chi ^ 1))
		{
			int xscale = rambase[2] ^ 0xff;
			int yscale = rambase[3];// ^ 0xff;
			UINT16 offset = rambase[6] + (rambase[7] << 8);
			int offs;

			/* mark this entry enabled */
			info->ve |= 1 << sprnum;

			/* look up the low byte of the sum plus the yscale value in */
			/* IC50/PR1119 to determine if we write back the sum of the */
			/* offset and the rowbytes this scanline (p. 138) */
			offs = (sum & 0xff) |			/* A0-A7 = AL0-AL7 */
				   ((yscale & 0x08) << 5);	/* A8-A9 = /RO11-/RO12 */

			/* one of the bits is selected based on the low 7 bits of yscale */
			if (!((pr1449[offs] >> (yscale & 0x07)) & 1))
			{
				offset += rambase[4] + (rambase[5] << 8);
				rambase[6] = offset;
				rambase[7] = offset >> 8;
			}

			/* the output of the ALU here goes to the individual level counter */
			info->latched[level] = 0;
			info->plb[level] = 0;
			info->offset[level] = offset << 1;
			info->frac[level] = 0;
			info->step[level] = sprite_xscale(xscale, 1.2e3, 1.2e3, 220e-12);
		}
	}
}


static UINT32 subroc3d_get_sprite_bits(running_machine *machine, sprite_info *sprinfo, UINT8 *plb)
{
	/* see logic on each sprite:
        END = (CDA == 1 && (CDA ^ CDB) == 0 && (CDC ^ CDD) == 0)
        PLB = END ^ (CDA == 1 && (CDC ^ CDD) == 0)
       end is in bit 1, plb in bit 0
    */
	static const UINT8 plb_end[16] = { 0,1,1,2, 1,1,1,1, 1,1,1,1, 0,1,1,2 };
	const UINT8 *sprite_gfxdata = memory_region(machine, "gfx1");
	UINT32 sprdata = 0;
	int level;

	*plb = 0;

	/* loop over all live levels */
	for (level = 0; level < 8; level++)
		if (sprinfo->lst & (1 << level))
		{
			/* latch the data and advance the offset */
			sprdata |= sprinfo->latched[level];
			*plb |= sprinfo->plb[level];
			sprinfo->frac[level] += sprinfo->step[level];

			/* if we're live and we've clocked more data, advance */
			while (sprinfo->frac[level] >= 0x800000)
			{
				UINT32 offs = sprinfo->offset[level];
				UINT8 pixdata;

				/* bit 0 controls which half of the byte to use */
				/* bits 1-13 go to address lines */
				/* bit 14 selects which of the two ROMs to read from */
				pixdata = sprite_gfxdata[(level << 15) | ((offs >> 1) & 0x7fff)] >> ((~offs & 1) * 4);
				sprinfo->latched[level] = sprite_expand[pixdata & 0x0f] << level;
				sprinfo->plb[level] = (plb_end[pixdata & 0x0f] & 1) << level;

				/* if bit 3 is 0 and bit 2 is 1, the enable flip/flip is reset */
				if (plb_end[pixdata & 0x0f] & 2)
					sprinfo->lst &= ~(1 << level);

				/* if bit 15 is set, we decrement instead of increment */
				sprinfo->offset[level] += (offs & 0x10000) ? -1 : 1;
				sprinfo->frac[level] -= 0x800000;
			}
		}

	return sprdata;
}



/*************************************
 *
 *  Subroc 3D video update
 *
 *************************************/

VIDEO_UPDATE( subroc3d )
{
	turbo_state *state = (turbo_state *)screen->machine->driver_data;
	bitmap_t *fgpixmap = tilemap_get_pixmap(state->fg_tilemap);
	const UINT8 *prom_base = memory_region(screen->machine, "proms");
	const UINT8 *pr1419 = prom_base + 0x000;
	const UINT8 *pr1620 = prom_base + 0x200;
	const UINT8 *pr1450 = prom_base + 0x500;
	const UINT8 *pr1454 = prom_base + 0x920;
	int x, y;

	/* loop over rows */
	for (y = cliprect->min_y; y <= cliprect->max_y; y++)
	{
		const UINT16 *fore = (UINT16 *)fgpixmap->base + y * fgpixmap->rowpixels;
		UINT16 *dest = (UINT16 *)bitmap->base + y * bitmap->rowpixels;
		sprite_info sprinfo;

		/* compute the sprite information; we use y-1 since this info was computed during HBLANK */
		/* on the previous scanline */
		subroc3d_prepare_sprites(screen->machine, state, y, &sprinfo);

		/* loop over columns */
		for (x = 0; x <= cliprect->max_x; x += TURBO_X_SCALE)
		{
			int offs, finalbits, ix;
			UINT8 xx = x / TURBO_X_SCALE;
			UINT8 foreraw, forebits, mux, cd, plb, mplb;
			UINT16 he;
			UINT32 sprbits;

			/* load the bitmask from the sprite position for both halves of the sprites (p. 143) */
			he = state->sprite_position[xx * 2] | (state->sprite_position[xx * 2 + 1] << 8);

			/* the AND of the line enable and horizontal enable is clocked and held in LST0-7 (p. 143) */
			he &= sprinfo.ve;
			sprinfo.lst |= he | (he >> 8);

			/* at this point, do the character lookup */
			if (!state->subroc3d_flip)
				foreraw = fore[xx];
			else
				foreraw = fore[(pr1454[(xx >> 3) & 0x1f] << 3) | (xx & 0x07)];

			/* perform the foreground color table lookup in IC62/PR1620 (p. 141) */
			forebits = pr1620[foreraw];

			/* MPLB is set based on the high bit of the raw foreground data, as an OR over the output */
			/* of the foreground color PROM */
			mplb = (foreraw & 0x80) || ((forebits & 0x0f) == 0);

			/* now that we have done all the per-5MHz pixel work, mix the sprites at the scale factor */
			for (ix = 0; ix < TURBO_X_SCALE; ix++)
			{
				/* iterate over live sprites and update them */
				/* the final 32-bit value is: */
				/*    CDA0-7 = D0 -D7  */
				/*    CDB0-7 = D8 -D15 */
				/*    CDC0-7 = D16-D23 */
				/*    CDD0-7 = D24-D31 */
				sprbits = subroc3d_get_sprite_bits(screen->machine, &sprinfo, &plb);

				/* MUX0-3 is selected by PLY0-3 and the sprite enable bits, and is the output */
				/* of IC21/PR1450 (p. 141), unless MPLB = 0, in which case the values are grounded (p. 141) */
				if (mplb)
				{
					offs = (plb ^ 0xff) |						/* A0-A7: /PLB0-7 */
						   ((state->subroc3d_ply & 0x02) << 7);	/*    A8: PLY1 */
					mux = pr1450[offs] >> ((state->subroc3d_ply & 0x01) * 4);
				}
				else
					mux = 0;

				/* CD0-3 are selected from the sprite bits and MUX0-2 (p. 141) */
				sprbits = (sprbits >> (mux & 0x07)) & 0x01010101;
				cd = (sprbits >> (24-3)) | (sprbits >> (16-2)) | (sprbits >> (8-1)) | sprbits;

				/* MUX3 selects either CD0-3 or the foreground output (p. 141) */
				if (mux & 0x08)
					finalbits = cd;
				else
					finalbits = forebits;

				/* we then go through a muxer to select one of the 16 outputs computed above (p. 141) */
				offs = (finalbits & 0x0f) | 				/* A0-A3: CD0-CD3 */
					   ((mux & 0x08) << 1) |				/*    A4: MUX3 */
					   (state->subroc3d_col << 5);			/* A5-A8: COL0-COL3 */
				dest[x + ix] = pr1419[offs];
			}
		}
	}
	return 0;
}



/*************************************
 *
 *  Buck Rogers sprite handling
 *
 *************************************/

static void buckrog_prepare_sprites(running_machine *machine, turbo_state *state, UINT8 y, sprite_info *info)
{
	const UINT8 *pr5196 = memory_region(machine, "proms") + 0x100;
	int sprnum;

	/* initialize the line enable signals to 0 */
	info->ve = 0;
	info->lst = 0;

	/* compute the sprite information, which was done on the previous scanline during HBLANK */
	for (sprnum = 0; sprnum < 16; sprnum++)
	{
		UINT8 *rambase = &state->spriteram[sprnum * 8];
		int level = sprnum & 7;
		UINT8 clo, chi;
		UINT32 sum;

		/* perform the first ALU to see if we are within the scanline */
		sum = y + (rambase[0]/* ^ 0xff*/);
		clo = (sum >> 8) & 1;
		sum += (y << 8) + ((rambase[1]/* ^ 0xff*/) << 8);
		chi = (sum >> 16) & 1;

		/* the AND of the low carry and the inverse of the high carry clocks an enable bit */
		/* for this sprite; note that the logic in the Turbo schematics is reversed here */
		if (clo & (chi ^ 1))
		{
			int xscale = rambase[2] ^ 0xff;
			int yscale = rambase[3];// ^ 0xff;
			UINT16 offset = rambase[6] + (rambase[7] << 8);
			int offs;

			/* mark this entry enabled */
			info->ve |= 1 << sprnum;

			/* look up the low byte of the sum plus the yscale value in */
			/* IC50/PR1119 to determine if we write back the sum of the */
			/* offset and the rowbytes this scanline (p. 138) */
			offs = (sum & 0xff) |			/* A0-A7 = AL0-AL7 */
				   ((yscale & 0x08) << 5);	/* A8-A9 = /RO11-/RO12 */

			/* one of the bits is selected based on the low 7 bits of yscale */
			if (!((pr5196[offs] >> (yscale & 0x07)) & 1))
			{
				offset += rambase[4] + (rambase[5] << 8);
				rambase[6] = offset;
				rambase[7] = offset >> 8;
			}

			/* the output of the ALU here goes to the individual level counter */
			info->latched[level] = 0;
			info->plb[level] = 0;
			info->offset[level] = offset << 1;
			info->frac[level] = 0;
			/* 820 verified in schematics */
			info->step[level] = sprite_xscale(xscale, 1.2e3, 820, 220e-12);
		}
	}
}


static UINT32 buckrog_get_sprite_bits(running_machine *machine, sprite_info *sprinfo, UINT8 *plb)
{
	/* see logic on each sprite:
        END = (CDA == 1 && (CDA ^ CDB) == 0 && (CDC ^ CDD) == 0)
        PLB = END ^ (CDA == 1 && (CDC ^ CDD) == 0)
       end is in bit 1, plb in bit 0
    */
	static const UINT8 plb_end[16] = { 0,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,2 };
	const UINT8 *sprite_gfxdata = memory_region(machine, "gfx1");
	UINT32 sprdata = 0;
	int level;

	*plb = 0;

	/* loop over all live levels */
	for (level = 0; level < 8; level++)
		if (sprinfo->lst & (1 << level))
		{
			/* latch the data and advance the offset */
			sprdata |= sprinfo->latched[level];
			*plb |= sprinfo->plb[level];
			sprinfo->frac[level] += sprinfo->step[level];

			/* if we're live and we've clocked more data, advance */
			while (sprinfo->frac[level] >= 0x800000)
			{
				UINT32 offs = sprinfo->offset[level];
				UINT8 pixdata;

				/* bit 0 controls which half of the byte to use */
				/* bits 1-13 go to address lines */
				/* bit 14 selects which of the two ROMs to read from */
				pixdata = sprite_gfxdata[(level << 15) | ((offs >> 1) & 0x7fff)] >> ((~offs & 1) * 4);
				sprinfo->latched[level] = sprite_expand[pixdata & 0x0f] << level;
				sprinfo->plb[level] = (plb_end[pixdata & 0x0f] & 1) << level;

				/* if bit 3 is 0 and bit 2 is 1, the enable flip/flip is reset */
				if (plb_end[pixdata & 0x0f] & 2)
					sprinfo->lst &= ~(1 << level);

				/* if bit 15 is set, we decrement instead of increment */
				sprinfo->offset[level] += (offs & 0x10000) ? -1 : 1;
				sprinfo->frac[level] -= 0x800000;
			}
		}

	return sprdata;
}



/*************************************
 *
 *  Buck Rogers video update
 *
 *************************************/

VIDEO_UPDATE( buckrog )
{
	turbo_state *state = (turbo_state *)screen->machine->driver_data;
	bitmap_t *fgpixmap = tilemap_get_pixmap(state->fg_tilemap);
	const UINT8 *bgcolor = memory_region(screen->machine, "gfx3");
	const UINT8 *prom_base = memory_region(screen->machine, "proms");
	const UINT8 *pr5194 = prom_base + 0x000;
	const UINT8 *pr5198 = prom_base + 0x500;
	const UINT8 *pr5199 = prom_base + 0x700;
	int x, y;

	/* loop over rows */
	for (y = cliprect->min_y; y <= cliprect->max_y; y++)
	{
		const UINT16 *fore = (UINT16 *)fgpixmap->base + y * fgpixmap->rowpixels;
		UINT16 *dest = (UINT16 *)bitmap->base + y * bitmap->rowpixels;
		sprite_info sprinfo;

		/* compute the sprite information; we use y-1 since this info was computed during HBLANK */
		/* on the previous scanline */
		buckrog_prepare_sprites(screen->machine, state, y, &sprinfo);

		/* loop over columns */
		for (x = 0; x <= cliprect->max_x; x += TURBO_X_SCALE)
		{
			UINT8 foreraw, forebits, cd, plb, star, mux;
			UINT8 xx = x / TURBO_X_SCALE;
			UINT16 he;
			UINT32 sprbits;
			int palbits, offs, ix;

			/* load the bitmask from the sprite position for both halves of the sprites (p. 143) */
			he = state->sprite_position[xx * 2] | (state->sprite_position[xx * 2 + 1] << 8);

			/* the AND of the line enable and horizontal enable is clocked and held in LST0-7 (p. 143) */
			he &= sprinfo.ve;
			sprinfo.lst |= he | (he >> 8);

			/* at this point, do the character lookup and the foreground color table lookup in IC93/PR1598 (SH 5/5)*/
			foreraw = fore[(pr5194[((xx >> 3) - 1) & 0x1f] << 3) | (xx & 0x07)];
			offs = ((foreraw & 0x03) << 0) |			/* A0-A1: BIT0-1 */
				   ((foreraw & 0xf8) >> 1) |			/* A2-A6: BANK3-7 */
				   ((state->buckrog_fchg & 0x03) << 7);	/* A7-A9: FCHG0-2 */
			forebits = pr5198[offs];

			/* fetch the STAR bit */
			star = state->buckrog_bitmap_ram[y * 256 + xx];

			/* now that we have done all the per-5MHz pixel work, mix the sprites at the scale factor */
			for (ix = 0; ix < TURBO_X_SCALE; ix++)
			{
				/* iterate over live sprites and update them */
				/* the final 32-bit value is: */
				/*    CDA0-7 = D0 -D7  */
				/*    CDB0-7 = D8 -D15 */
				/*    CDC0-7 = D16-D23 */
				/*    CDD0-7 = D24-D31 */
				sprbits = buckrog_get_sprite_bits(screen->machine, &sprinfo, &plb);

				/* the PLB bits go into an LS148 8-to-1 decoder and become MUX0-3 (PROM board SH 2/10) */
				if (plb == 0)
					mux = 8;
				else
				{
					mux = 7;
					while (!(plb & 0x80))
					{
						mux--;
						plb <<= 1;
					}
				}

				/* MUX then selects one of the sprites and selects CD0-3 */
				sprbits = (sprbits >> (mux & 0x07)) & 0x01010101;
				cd = (sprbits >> (24-3)) | (sprbits >> (16-2)) | (sprbits >> (8-1)) | sprbits;

				/* this info goes into an LS148 8-to-3 decoder to determine the priorities (SH 5/5) */

				/* priority 7 is if bit 0x80 of the foreground color is 0; CHNG = 0 */
				if (!(forebits & 0x80))
				{
					palbits = ((forebits & 0x3c) << 2) |
							  ((forebits & 0x06) << 1) |
							  ((forebits & 0x01) << 0);
				}

				/* priority 6 is if MUX3 is 0; CHNG = 1 */
				else if (!(mux & 0x08))
				{
					offs = (cd & 0x0f) |						/* A0-A3: CD0-3 */
						   ((mux & 0x07) << 4) |				/* A4-A6: MUX0-2 */
						   ((state->buckrog_obch & 0x07) << 7);	/* A7-A9: OBCH0-2 */
					palbits = pr5199[offs];
				}

				/* priority 3 is if bit 0x40 of the foreground color is 0; CHNG = 0 */
				else if (!(forebits & 0x40))
				{
					palbits = ((forebits & 0x3c) << 2) |
							  ((forebits & 0x06) << 1) |
							  ((forebits & 0x01) << 0);
				}

				/* priority 1 is if the star is set; CHNG = 2 */
				else if (star)
				{
					palbits = 0xff;
				}

				/* otherwise, CHNG = 3 */
				else
				{
					palbits = bgcolor[y | ((state->buckrog_mov & 0x1f) << 8)];
					palbits = (palbits & 0xc0) | ((palbits & 0x30) << 4) | ((palbits & 0x0f) << 2);
				}

				/* store the final bits for this pixel */
				dest[x + ix] = palbits;
			}
		}
	}
	return 0;
}
