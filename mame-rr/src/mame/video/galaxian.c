/***************************************************************************

    Galaxian-derived video hardware

****************************************************************************

    Video timing:

        The master clock is an 18.432MHz crystal. It is divided by 3 by
        a pair of J/K flip-flops to 6.144MHz. This 6MHz signal is used to
        drive most of the video logic. Note that due to the way the
        divide-by-3 circuit is implemented, the duty cycle of the 6MHz
        signal is 66% (i.e., it is high for 2 18MHz clocks and low for 1).
        This is important for accurate stars rendering.


    Horizontal timing:

        H counts from 010000000 (128) to 111111111 (511), giving 384
        total H clocks per scanline

        However, the top bit is inverted to become 256H, so when reading
        the schematics it's really counting from:
            110000000 -> 111111111 (blanking period)
        and then from:
            000000000 -> 011111111 (main portion of screen = 256 pixels)

        HBLANK is a flip-flop clocked by 2H:
          * It is held clear when 256H = 0 (main portion of screen)
          * The D input is connected to !(64H & 32H & 16H & 8H)
          * It is clocked to 1 when H=130
          * It is clocked to 0 when H=250
          * This gives 264 total non-blanked pixels:
              6 additional pixels on the left (H=250..255)
              256 main area pixels (H=256..511)
              2 additional pixels on the right (H=128..129)

        HSYNC is a flip-flop clocked by 16H:
          * It is held clear when 256H = 0 (main portion of screen)
          * The D input is connected to !(!64H & 32H)
          * HSYNC is the /Q output
          * It is clocked to 1 when H=176
          * It is clocked to 0 when H=208


    Vertical timing:

        V counts from 011111000 (248) to 111111111 (511), giving 264
        total V clocks per frame

        IMPORTANT: the V sync chain is clocked by HSYNC. This means
        that for the first 48 H clocks of the blanking period, the
        V counter is one behind. This is important to take into account
        for sprite and missile positioning.

        VBLANK is a flip-flop clocked by 16V:
          * The D input is connected to !(128V & 64V & 32V)
          * It is clocked to 1 when V=496
          * It is clocked to 0 when V=272
          * This gives 224 total non-blanked pixels

        VSYNC is set to !256V:
          * It is set to 1 when V=248
          * It is cleared to 0 when V=256


    Sprites and missiles:

        During the HBLANK period, sprites and missiles are processed.
        Sprites are rendered into the line buffer, which was cleared
        during the visible portion of the previous scanline.

        It takes 8 H clocks to set up a sprite, and 16 to render it
        to the line buffer. The setup clocks are overlapped with the
        rendering clocks. In theory this would result in enough time
        to render 128/16 = 8 sprites. However, the setup does not
        begin until after HBLANK, so there is only enough time to
        render the first 7 1/2 entries.

        Interleaved with the setup for sprites is setup for the
        shell and missile rendering. Shells and missiles are rendered
        realtime during the visible portion of the frame, and are
        effectively color-ORed directly into the final RGB output.
        During the HBLANK setup period, each shell/missile entry is
        compared against the current V position; if an exact match
        is found, the H position is loaded into a dedicated 8-bit
        counter. The counter clocks each pixel during the active video
        period; when it reaches $FC it enables the output until it
        hits zero, at which point it shuts itself off. Because there
        is only one counter for shells and one for missiles, only one
        shell and one missile can be specified per scanline. The last
        matching entry found will always win.

        The difference between shell and missile is that shells
        populate the first 7 entries and are rendered as white,
        whereas missiles populate the final entry and are rendered
        as yellow.

        Here is the detailed sequence of events for sprite and
        missile/shell rendering during the first 24 H clocks of
        HBLANK:

            H=080: HPOSI=objram[40], /VPL latches V+objram[40]
            H=081: HPOSI=objram[40]
            H=082: HPOSI=objram[41], /OBJ DATA L latches picture number, H/V flip
            H=083: HPOSI=objram[41]
            H=084: HPOSI=objram[42], /COL L latches low 3 bits as color
            H=085: HPOSI=objram[42]
            H=086: HPOSI=objram[43]
            H=087: HPOSI=objram[43], /CNTR LD latches X position
            <sprite 0 begins rendering>
            H=088: HPOSI=objram[40], /VPL latches V+objram[40]
            H=089: HPOSI=objram[40]
            H=08A: HPOSI=objram[61]
            H=08B: HPOSI=objram[61], MSLD is latched if Y position matches shell
            H=08C: HPOSI=objram[42]
            H=08D: HPOSI=objram[42]
            H=08E: HPOSI=objram[63]
            H=08F: HPOSI=objram[63], /SLD fires to latch down shell counter value
            H=090: HPOSI=objram[44], /VPL latches V+objram[44]
            H=091: HPOSI=objram[44]
            H=092: HPOSI=objram[45], /OBJ DATA L latches picture number, H/V flip
            H=093: HPOSI=objram[45]
            H=094: HPOSI=objram[46], /COL L latches low 3 bits as color
            H=095: HPOSI=objram[46]
            H=096: HPOSI=objram[47]
            H=097: HPOSI=objram[47], /CNTR LD latches X position
            <sprite 1 begins rendering>

        From this, you can see the object RAM layout looks like:

            objram[40] = vertical position of sprite 0
            objram[41] = picture number and H/V flip of sprite 0
            objram[42] = color of sprite 0
            objram[43] = horizontal position of sprite 0

            objram[61] = vertical position of shell 0
            objram[63] = horizontal count until shell 0 starts rendering

        A vertical match for a sprite is true if ((V + vpos) & 0xf0) == 0xf0.
        A vertical match for a shell/missile is if ((V + vpos) & 0xff) == 0xff.

        Overall, the process for sprites and missiles during HBLANK looks
        like this:

            H=080: begin setup sprite 0
            H=082: begin HBLANK
            H=088: begin render sprite 0; begin setup shell 0
            H=090: begin setup sprite 1
            H=098: begin render sprite 1; begin setup shell 1
            H=0A0: begin setup sprite 2
            H=0A8: begin render sprite 2; begin setup shell 2
            H=0B0: VSYNC increments V counter; subsequent sprites match V+1
            H=0B0: begin setup sprite 3
            H=0B8: begin render sprite 3; begin setup shell 3
            H=0C0: begin setup sprite 4
            H=0C8: begin render sprite 4; begin setup shell 4
            H=0D0: begin setup sprite 5
            H=0D8: begin render sprite 5; begin setup shell 5
            H=0E0: begin setup sprite 6
            H=0E8: begin render sprite 6; begin setup shell 6
            H=0F0: begin setup sprite 7
            H=0F8: begin render sprite 7; begin setup missile
            H=0FA: end HBLANK
            H=100: finish render sprite 7 (only 1/2 way through)



       /VPL: H=xxxxxx000 -> latches sum of V+HPOSI for vertical positioning
     /COL L: H=xxxxxx100 -> latches HPOSI into color register (low 3 bits)
        /LD: H=xxxxxx111 -> shift register load from ROM
  /CNTR CLR: H=0xxxx0111 -> resets line buffer counter to 0
/OBJ DATA L: H=1xxxx0010 -> latches HPOSI into picture number latch
   /CNTR LD: H=1xxxx0111 -> latches HPOSI into line buffer counter (sprite X position)
       /SLD: H=1xxxx1111 -> latches down counter until shell (except when /MLD)
       /MLD: H=1x1111111 -> latches down counter until missile



VRAM addresses:
         addr  video
  VRA7 =  A9   SUM7
  VRA8 =  A8   SUM6
  VRA6 =  A7   SUM5
  VRA5 =  A6   SUM4
  VRA4 =  A5   SUM3
  VRA3 =  A4   128HB
  VRA0 =  A3    64HB
  VRA1 =  A2    32HB
  VRA2 =  A1    16HB
  VRA9 =  A0     8HB

OBJRAM addresses:
         addr 256H=0 256H=1
   RA4 =  A0    4H     2H
   RA7 =  A1    8HB    4H
   RA1 =  A2   16HB   16HB
   RA0 =  A3   32HB   32HB
   RA5 =  A4   64HB   64HB
   RA6 =  A5  128HB  (2H & 8HB)
   RA3 =  A6  256H   256H
   RA2 =  A7    0      0

H=80: 00,00,01,01,02,02,03,03 00,00,21,21,02,02,23,23
H=90: 04,04,05,05,06,06,07,07 04,04,25,25,06,06,27,27
H=A0: 08,08,09,09,0A,0A,0B,0B 08,08,29,29,0A,0A,2B,2B
H=B0: 0C,0C,0D,0D,0E,0E,0F,0F 0C,0C,2D,2D,0E,0E,2F,2F



***************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "includes/galaxian.h"




/*************************************
 *
 *  Constants
 *
 *************************************/

#define STAR_RNG_PERIOD		((1 << 17) - 1)
#define RGB_MAXIMUM			224



/*************************************
 *
 *  Global variables
 *
 *************************************/

/* rendering callbacks */
galaxian_draw_bullet_func galaxian_draw_bullet_ptr;
galaxian_draw_background_func galaxian_draw_background_ptr;

/* tile/sprite modification callbacks */
galaxian_extend_tile_info_func galaxian_extend_tile_info_ptr;
galaxian_extend_sprite_info_func galaxian_extend_sprite_info_ptr;

/* global tweaks */
UINT8 galaxian_frogger_adjust;
UINT8 galaxian_sfx_tilemap;
UINT8 galaxian_sprite_clip_start;
UINT8 galaxian_sprite_clip_end;



/*************************************
 *
 *  Local variables
 *
 *************************************/

static tilemap_t *bg_tilemap;

static UINT8 flipscreen_x;
static UINT8 flipscreen_y;

static UINT8 background_enable;
static UINT8 background_red;
static UINT8 background_green;
static UINT8 background_blue;

static UINT32 star_rng_origin;
static UINT32 star_rng_origin_frame;
static rgb_t star_color[64];
static UINT8 *stars;
static UINT8 stars_enabled;
static UINT8 stars_blink_state;

static rgb_t bullet_color[8];

static UINT8 gfxbank[5];



/*************************************
 *
 *  Function prototypes
 *
 *************************************/

static void state_save_register(running_machine *machine);
static TILE_GET_INFO( bg_get_tile_info );

static void sprites_draw(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, const UINT8 *spritebase);

static void stars_init(running_machine *machine);
static void stars_update_origin(running_machine *machine);
static void stars_draw_row(bitmap_t *bitmap, int maxx, int y, UINT32 star_offs, UINT8 starmask);

static void bullets_draw(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, const UINT8 *base);



/*************************************
 *
 *  Palette setup
 *
 *************************************/

PALETTE_INIT( galaxian )
{
	static const int rgb_resistances[3] = { 1000, 470, 220 };
	double rweights[3], gweights[3], bweights[2];
	int i, minval, midval, maxval, len;
	UINT8 starmap[4];

	/*
        Sprite/tilemap colors are mapped through a color PROM as follows:

          bit 7 -- 220 ohm resistor  -- BLUE
                -- 470 ohm resistor  -- BLUE
                -- 220 ohm resistor  -- GREEN
                -- 470 ohm resistor  -- GREEN
                -- 1  kohm resistor  -- GREEN
                -- 220 ohm resistor  -- RED
                -- 470 ohm resistor  -- RED
          bit 0 -- 1  kohm resistor  -- RED

        In parallel with these resistors are a pair of 150 ohm and 100 ohm
        resistors on each R,G,B component that are connected to the star
        generator.

        And in parallel with the whole mess are a set of 100 ohm resistors
        on each R,G,B component that are enabled when a shell/missile is
        enabled.

        When computing weights, we use RGB_MAXIMUM as the maximum to give
        headroom for stars and shells/missiles. This is not fully accurate,
        but if we included all possible sources in parallel, the brightness
        of the main game would be very low to allow for all the oversaturation
        of the stars and shells/missiles.
    */
	compute_resistor_weights(0,	RGB_MAXIMUM, -1.0,
			3, &rgb_resistances[0], rweights, 470, 0,
			3, &rgb_resistances[0], gweights, 470, 0,
			2, &rgb_resistances[1], bweights, 470, 0);

	/* decode the palette first */
	len = memory_region_length(machine, "proms");
	for (i = 0; i < len; i++)
	{
		UINT8 bit0, bit1, bit2, r, g, b;

		/* red component */
		bit0 = BIT(color_prom[i],0);
		bit1 = BIT(color_prom[i],1);
		bit2 = BIT(color_prom[i],2);
		r = combine_3_weights(rweights, bit0, bit1, bit2);

		/* green component */
		bit0 = BIT(color_prom[i],3);
		bit1 = BIT(color_prom[i],4);
		bit2 = BIT(color_prom[i],5);
		g = combine_3_weights(gweights, bit0, bit1, bit2);

		/* blue component */
		bit0 = BIT(color_prom[i],6);
		bit1 = BIT(color_prom[i],7);
		b = combine_2_weights(bweights, bit0, bit1);

		palette_set_color(machine, i, MAKE_RGB(r,g,b));
	}

	/*
        The maximum sprite/tilemap resistance is ~130 Ohms with all RGB
        outputs enabled (1/(1/1000 + 1/470 + 1/220)). Since we normalized
        to RGB_MAXIMUM, this maps RGB_MAXIMUM -> 130 Ohms.

        The stars are at 150 Ohms for the LSB, and 100 Ohms for the MSB.
        This means the 3 potential values are:

            150 Ohms -> RGB_MAXIMUM * 130 / 150
            100 Ohms -> RGB_MAXIMUM * 130 / 100
             60 Ohms -> RGB_MAXIMUM * 130 / 60

        Since we can't saturate that high, we instead approximate this
        by compressing the values proportionally into the 194->255 range.
    */
	minval = RGB_MAXIMUM * 130 / 150;
	midval = RGB_MAXIMUM * 130 / 100;
	maxval = RGB_MAXIMUM * 130 / 60;

	/* compute the values for each of 4 possible star values */
	starmap[0] = 0;
	starmap[1] = minval;
	starmap[2] = minval + (255 - minval) * (midval - minval) / (maxval - minval);
	starmap[3] = 255;

	/* generate the colors for the stars */
	for (i = 0; i < 64; i++)
	{
		UINT8 bit0, bit1, r, g, b;

		/* bit 5 = red @ 150 Ohm, bit 4 = red @ 100 Ohm */
		bit0 = BIT(i,5);
		bit1 = BIT(i,4);
		r = starmap[(bit1 << 1) | bit0];

		/* bit 3 = green @ 150 Ohm, bit 2 = green @ 100 Ohm */
		bit0 = BIT(i,3);
		bit1 = BIT(i,2);
		g = starmap[(bit1 << 1) | bit0];

		/* bit 1 = blue @ 150 Ohm, bit 0 = blue @ 100 Ohm */
		bit0 = BIT(i,1);
		bit1 = BIT(i,0);
		b = starmap[(bit1 << 1) | bit0];

		/* set the RGB color */
		star_color[i] = MAKE_RGB(r, g, b);
	}

	/* default bullet colors are white for the first 7, and yellow for the last one */
	for (i = 0; i < 7; i++)
		bullet_color[i] = MAKE_RGB(0xff,0xff,0xff);
	bullet_color[7] = MAKE_RGB(0xff,0xff,0x00);
}

PALETTE_INIT( moonwar )
{
	PALETTE_INIT_CALL(galaxian);


	/* wire mod to connect the bullet blue output to the 220 ohm resistor */
	bullet_color[7] = MAKE_RGB(0xef,0xef,0x97);
}

/*************************************
 *
 *  Common video init
 *
 *************************************/

VIDEO_START( galaxian )
{
	/* create a tilemap for the background */
	if (!galaxian_sfx_tilemap)
	{
		/* normal galaxian hardware is row-based and individually scrolling columns */
		bg_tilemap = tilemap_create(machine, bg_get_tile_info, tilemap_scan_rows, GALAXIAN_XSCALE*8,8, 32,32);
		tilemap_set_scroll_cols(bg_tilemap, 32);
		tilemap_set_scrolldx(bg_tilemap, 0, -GALAXIAN_XSCALE * 128);
		tilemap_set_scrolldy(bg_tilemap, 0, 8);
	}
	else
	{
		/* sfx hardware is column-based and individually scrolling rows */
		bg_tilemap = tilemap_create(machine, bg_get_tile_info, tilemap_scan_cols, GALAXIAN_XSCALE*8,8, 32,32);
		tilemap_set_scroll_rows(bg_tilemap, 32);
		tilemap_set_scrolldx(bg_tilemap, 0, -GALAXIAN_XSCALE * 128);
		tilemap_set_scrolldy(bg_tilemap, 0, 8);
	}
	tilemap_set_transparent_pen(bg_tilemap, 0);

	/* initialize globals */
	flipscreen_x = 0;
	flipscreen_y = 0;
	background_enable = 0;
	background_blue = 0;
	background_red = 0;
	background_green = 0;

	/* initialize stars */
	stars_init(machine);

	/* register for save states */
	state_save_register(machine);
}


static void state_save_register(running_machine *machine)
{
	state_save_register_global(machine, flipscreen_x);
	state_save_register_global(machine, flipscreen_y);
	state_save_register_global(machine, background_enable);
	state_save_register_global(machine, background_red);
	state_save_register_global(machine, background_green);
	state_save_register_global(machine, background_blue);

	state_save_register_global_array(machine, gfxbank);

	state_save_register_global(machine, stars_enabled);
	state_save_register_global(machine, star_rng_origin);
	state_save_register_global(machine, star_rng_origin_frame);
	state_save_register_global(machine, stars_blink_state);
}



/*************************************
 *
 *  Common video update
 *
 *************************************/

VIDEO_UPDATE( galaxian )
{
	/* draw the background layer (including stars) */
	(*galaxian_draw_background_ptr)(screen->machine, bitmap, cliprect);

	/* draw the tilemap characters over top */
	tilemap_draw(bitmap, cliprect, bg_tilemap, 0, 0);

	/* render the sprites next */
	sprites_draw(screen->machine, bitmap, cliprect, &screen->machine->generic.spriteram.u8[0x40]);

	/* if we have bullets to draw, render them following */
	if (galaxian_draw_bullet_ptr != NULL)
		bullets_draw(screen->machine, bitmap, cliprect, &screen->machine->generic.spriteram.u8[0x60]);

	return 0;
}



/*************************************
 *
 *  Background tilemap
 *
 *************************************/

static TILE_GET_INFO( bg_get_tile_info )
{
	UINT8 x = tile_index & 0x1f;

	UINT16 code = machine->generic.videoram.u8[tile_index];
	UINT8 attrib = machine->generic.spriteram.u8[x*2+1];
	UINT8 color = attrib & 7;

	if (galaxian_extend_tile_info_ptr != NULL)
		(*galaxian_extend_tile_info_ptr)(&code, &color, attrib, x);

	SET_TILE_INFO(0, code, color, 0);
}


WRITE8_HANDLER( galaxian_videoram_w )
{
	/* update any video up to the current scanline */
	space->machine->primary_screen->update_now();

	/* store the data and mark the corresponding tile dirty */
	space->machine->generic.videoram.u8[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}


WRITE8_HANDLER( galaxian_objram_w )
{
	/* update any video up to the current scanline */
	space->machine->primary_screen->update_now();

	/* store the data */
	space->machine->generic.spriteram.u8[offset] = data;

	/* the first $40 bytes affect the tilemap */
	if (offset < 0x40)
	{
		/* even entries control the scroll position */
		if ((offset & 0x01) == 0)
		{
			/* Frogger: top and bottom 4 bits swapped entering the adder */
			if (galaxian_frogger_adjust)
				data = (data >> 4) | (data << 4);
			if (!galaxian_sfx_tilemap)
				tilemap_set_scrolly(bg_tilemap, offset >> 1, data);
			else
				tilemap_set_scrollx(bg_tilemap, offset >> 1, GALAXIAN_XSCALE*data);
		}

		/* odd entries control the color base for the row */
		else
		{
			for (offset >>= 1; offset < 0x0400; offset += 32)
				tilemap_mark_tile_dirty(bg_tilemap, offset);
		}
	}
}



/*************************************
 *
 *  Sprite rendering
 *
 *************************************/

static void sprites_draw(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, const UINT8 *spritebase)
{
	rectangle clip = *cliprect;
	int sprnum;
	int clip_ofs = (flipscreen_x ? 16 : 0);

	/* 16 of the 256 pixels of the sprites are hard-clipped at the line buffer */
	/* according to the schematics, it should be the first 16 pixels; however, */
	/* some bootlegs demonstrate that this can be shifted to other positions. */
	clip.min_x = MAX(clip.min_x, (galaxian_sprite_clip_start - clip_ofs) * GALAXIAN_XSCALE);
	clip.max_x = MIN(clip.max_x, (galaxian_sprite_clip_end - clip_ofs + 1) * GALAXIAN_XSCALE - 1);

	/* The line buffer is only written if it contains a '0' currently; */
	/* it is cleared during the visible area, and populated during HBLANK */
	/* To simulate this, we render backwards so that lower numbered sprites */
	/* have priority over higher numbered sprites. */
	for (sprnum = 7; sprnum >= 0; sprnum--)
	{
		const UINT8 *base = &spritebase[sprnum * 4];
		/* Frogger: top and bottom 4 bits swapped entering the adder */
		UINT8 base0 = galaxian_frogger_adjust ? ((base[0] >> 4) | (base[0] << 4)) : base[0];
		/* the first three sprites match against y-1 */
		UINT8 sy = 240 - (base0 - (sprnum < 3));
		UINT16 code = base[1] & 0x3f;
		UINT8 flipx = base[1] & 0x40;
		UINT8 flipy = base[1] & 0x80;
		UINT8 color = base[2] & 7;
		UINT8 sx = base[3];

		/* extend the sprite information */
		if (galaxian_extend_sprite_info_ptr != NULL)
			(*galaxian_extend_sprite_info_ptr)(base, &sx, &sy, &flipx, &flipy, &code, &color);

		/* apply flipscreen in X direction */
		if (flipscreen_x)
		{
			sx = 242 - sx; // + 8 - HOFS
			flipx = !flipx;
		}

		/* apply flipscreen in Y direction */
		if (flipscreen_y)
		{
			sy = 240 - sy;
			flipy = !flipy;
		}

		/* draw */
		drawgfx_transpen(bitmap, &clip,
				machine->gfx[1],
				code, color,
				flipx, flipy,
				GALAXIAN_H0START + GALAXIAN_XSCALE * sx, sy, 0);
	}
}



/*************************************
 *
 *  Bullets rendering
 *
 *************************************/

static void bullets_draw(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, const UINT8 *base)
{
	int y;

	/* iterate over scanlines */
	for (y = cliprect->min_y; y <= cliprect->max_y; y++)
	{
		UINT8 shell = 0xff, missile = 0xff;
		UINT8 effy;
		int which;

		/* the first 3 entries match Y-1 */
		effy = flipscreen_y ? ((y - 1) ^ 255) : (y - 1);
		for (which = 0; which < 3; which++)
			if ((UINT8)(base[which*4+1] + effy) == 0xff)
				shell = which;

		/* remaining entries match Y */
		effy = flipscreen_y ? (y ^ 255) : y;
		for (which = 3; which < 8; which++)
			if ((UINT8)(base[which*4+1] + effy) == 0xff)
			{
				if (which != 7)
					shell = which;
				else
					missile = which;
			}

		/* draw the shell */
		if (shell != 0xff)
			(*galaxian_draw_bullet_ptr)(machine, bitmap, cliprect, shell, 255 - base[shell*4+3], y);
		if (missile != 0xff)
			(*galaxian_draw_bullet_ptr)(machine, bitmap, cliprect, missile, 255 - base[missile*4+3], y);
	}
}



/*************************************
 *
 *  Screen orientation
 *
 *************************************/

WRITE8_HANDLER( galaxian_flip_screen_x_w )
{
	if (flipscreen_x != (data & 0x01))
	{
		space->machine->primary_screen->update_now();

		/* when the direction changes, we count a different number of clocks */
		/* per frame, so we need to reset the origin of the stars to the current */
		/* frame before we flip */
		stars_update_origin(space->machine);

		flipscreen_x = data & 0x01;
		tilemap_set_flip(bg_tilemap, (flipscreen_x ? TILEMAP_FLIPX : 0) | (flipscreen_y ? TILEMAP_FLIPY : 0));
	}
}

WRITE8_HANDLER( galaxian_flip_screen_y_w )
{
	if (flipscreen_y != (data & 0x01))
	{
		space->machine->primary_screen->update_now();
		flipscreen_y = data & 0x01;
		tilemap_set_flip(bg_tilemap, (flipscreen_x ? TILEMAP_FLIPX : 0) | (flipscreen_y ? TILEMAP_FLIPY : 0));
	}
}

WRITE8_HANDLER( galaxian_flip_screen_xy_w )
{
	galaxian_flip_screen_x_w(space, offset, data);
	galaxian_flip_screen_y_w(space, offset, data);
}



/*************************************
 *
 *  Background controls
 *
 *************************************/

WRITE8_HANDLER( galaxian_stars_enable_w )
{
	if ((stars_enabled ^ data) & 0x01)
	space->machine->primary_screen->update_now();
	if (!stars_enabled && (data & 0x01))
	{
		/* on the rising edge of this, the CLR on the shift registers is released */
		/* this resets the "origin" of this frame to 0 minus the number of clocks */
		/* we have counted so far */
		star_rng_origin = STAR_RNG_PERIOD - (space->machine->primary_screen->vpos() * 512 + space->machine->primary_screen->hpos());
		star_rng_origin_frame = space->machine->primary_screen->frame_number();
	}
	stars_enabled = data & 0x01;
}


WRITE8_HANDLER( scramble_background_enable_w )
{
	space->machine->primary_screen->update_now();
	background_enable = data & 0x01;
}


WRITE8_HANDLER( scramble_background_red_w )
{
	space->machine->primary_screen->update_now();
	background_red = data & 0x01;
}


WRITE8_HANDLER( scramble_background_green_w )
{
	space->machine->primary_screen->update_now();
	background_green = data & 0x01;
}


WRITE8_HANDLER( scramble_background_blue_w )
{
	space->machine->primary_screen->update_now();
	background_blue = data & 0x01;
}



/*************************************
 *
 *  Graphics banking
 *
 *************************************/

WRITE8_HANDLER( galaxian_gfxbank_w )
{
	if (gfxbank[offset] != data)
	{
		space->machine->primary_screen->update_now();
		gfxbank[offset] = data;
		tilemap_mark_all_tiles_dirty(bg_tilemap);
	}
}



/*************************************
 *
 *  Star initialization
 *
 *************************************/

static void stars_init(running_machine *machine)
{
	UINT32 shiftreg;
	int i;

	/* reset the blink and enabled states */
	stars_enabled = FALSE;
	stars_blink_state = 0;

	/* precalculate the RNG */
	stars = auto_alloc_array(machine, UINT8, STAR_RNG_PERIOD);
	shiftreg = 0;
	for (i = 0; i < STAR_RNG_PERIOD; i++)
	{
		/* stars are enabled if the upper 8 bits are 1 and the low bit is 0 */
		int enabled = ((shiftreg & 0x1fe01) == 0x1fe00);

		/* color comes from the 6 bits below the top 8 bits */
		int color = (~shiftreg & 0x1f8) >> 3;

		/* store the color value in the low 6 bits and the enable in the upper bit */
		stars[i] = color | (enabled << 7);

		/* the LFSR is fed based on the XOR of bit 12 and the inverse of bit 0 */
		shiftreg = (shiftreg >> 1) | ((((shiftreg >> 12) ^ ~shiftreg) & 1) << 16);
	}
}



/*************************************
 *
 *  Adjust the origin of stars
 *
 *************************************/

static void stars_update_origin(running_machine *machine)
{
	int curframe = machine->primary_screen->frame_number();

	/* only update on a different frame */
	if (curframe != star_rng_origin_frame)
	{
		/* The RNG period is 2^17-1; each frame, the shift register is clocked */
		/* 512*256 = 2^17 times. This means that we clock one extra time each */
		/* frame. However, if we are NOT flipped, there is a pair of D flip-flops */
		/* at 6B which delay the count so that we count 512*256-2 = 2^17-2 times. */
		/* In this case, we only one time less than the period each frame. Both */
		/* of these off-by-one countings produce the horizontal star scrolling. */
		int per_frame_delta = flipscreen_x ? 1 : -1;
		int total_delta = per_frame_delta * (curframe - star_rng_origin_frame);

		/* we can't just use % here because mod of a negative number is undefined */
		while (total_delta < 0)
			total_delta += STAR_RNG_PERIOD;

		/* now that everything is positive, do the mod */
		star_rng_origin = (star_rng_origin + total_delta) % STAR_RNG_PERIOD;
		star_rng_origin_frame = curframe;
	}
}



/*************************************
 *
 *  Star blinking
 *
 *************************************/

TIMER_CALLBACK( galaxian_stars_blink_timer )
{
	stars_blink_state++;
}



/*************************************
 *
 *  Draw a row of stars
 *
 *************************************/

static void stars_draw_row(bitmap_t *bitmap, int maxx, int y, UINT32 star_offs, UINT8 starmask)
{
	int x;

	/* ensure our star offset is valid */
	star_offs %= STAR_RNG_PERIOD;

	/* iterate over the specified number of 6MHz pixels */
	for (x = 0; x < maxx; x++)
	{
		/* stars are suppressed unless V1 ^ H8 == 1 */
		int enable_star = (y ^ (x >> 3)) & 1;
		UINT8 star;

		/*
            The RNG clock is the master clock (18MHz) ANDed with the pixel clock (6MHz).
            The divide-by-3 circuit that produces the pixel clock generates a square wave
            with a 2/3 duty cycle, so the result of the AND generates a clock like this:
                        _   _   _   _   _   _   _   _
              MASTER: _| |_| |_| |_| |_| |_| |_| |_| |
                        _______     _______     ______
              PIXEL:  _|       |___|       |___|
                        _   _       _   _       _   _
              RNG:    _| |_| |_____| |_| |_____| |_| |

            Thus for each pixel, there are 3 master clocks and 2 RNG clocks, and the RNG
            is clocked asymmetrically. To simulate this, we expand the horizontal screen
            size by 3 and handle the first RNG clock with one pixel and the second RNG
            clock with two pixels.
        */

		/* first RNG clock: one pixel */
		star = stars[star_offs++];
		if (star_offs >= STAR_RNG_PERIOD)
			star_offs = 0;
		if (enable_star && (star & 0x80) != 0 && (star & starmask) != 0)
			*BITMAP_ADDR32(bitmap, y, GALAXIAN_XSCALE*x + 0) = star_color[star & 0x3f];

		/* second RNG clock: two pixels */
		star = stars[star_offs++];
		if (star_offs >= STAR_RNG_PERIOD)
			star_offs = 0;
		if (enable_star && (star & 0x80) != 0 && (star & starmask) != 0)
		{
			*BITMAP_ADDR32(bitmap, y, GALAXIAN_XSCALE*x + 1) = star_color[star & 0x3f];
			*BITMAP_ADDR32(bitmap, y, GALAXIAN_XSCALE*x + 2) = star_color[star & 0x3f];
		}
	}
}



/*************************************
 *
 *  Background rendering
 *
 *************************************/

#ifdef UNUSED_FUNCTION
static int flip_and_clip(rectangle *draw, int xstart, int xend, const rectangle *cliprect)
{
	*draw = *cliprect;
	if (!flipscreen_x)
	{
		draw->min_x = xstart * GALAXIAN_XSCALE;
		draw->max_x = xend * GALAXIAN_XSCALE + (GALAXIAN_XSCALE - 1);
	}
	else
	{
		draw->min_x = (xend ^ 255) * GALAXIAN_XSCALE;
		draw->max_x = (xstart ^ 255) * GALAXIAN_XSCALE + (GALAXIAN_XSCALE - 1);
	}
	sect_rect(draw, cliprect);
	return (draw->min_x <= draw->max_x);
}
#endif


void galaxian_draw_background(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	/* erase the background to black first */
	bitmap_fill(bitmap, cliprect, RGB_BLACK);

	/* update the star origin to the current frame */
	stars_update_origin(machine);

	/* render stars if enabled */
	if (stars_enabled)
	{
		int y;

		/* iterate over scanlines */
		for (y = cliprect->min_y; y <= cliprect->max_y; y++)
		{
			UINT32 star_offs = star_rng_origin + y * 512;
			stars_draw_row(bitmap, 256, y, star_offs, 0xff);
		}
	}
}


void frogger_draw_background(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	rectangle draw;

	if (flipscreen_x)
	{
		/* color split point verified on real machine */
		/* hmmm, according to schematics it is at 128+8; which is right? */
		draw = *cliprect;
		draw.max_x = MIN(draw.max_x, (128-8) * GALAXIAN_XSCALE - 1);
		if (draw.min_x <= draw.max_x)
			bitmap_fill(bitmap, &draw, RGB_BLACK);

		draw = *cliprect;
		draw.min_x = MAX(draw.min_x, (128-8) * GALAXIAN_XSCALE);
		if (draw.min_x <= draw.max_x)
			bitmap_fill(bitmap, &draw, MAKE_RGB(0,0,0x47));
	}
	else
	{
		/* color split point verified on real machine */
		/* hmmm, according to schematics it is at 128+8; which is right? */
		draw = *cliprect;
		draw.max_x = MIN(draw.max_x, (128+8) * GALAXIAN_XSCALE - 1);
		if (draw.min_x <= draw.max_x)
			bitmap_fill(bitmap, &draw, MAKE_RGB(0,0,0x47));

		draw = *cliprect;
		draw.min_x = MAX(draw.min_x, (128+8) * GALAXIAN_XSCALE);
		if (draw.min_x <= draw.max_x)
			bitmap_fill(bitmap, &draw, RGB_BLACK);
	}

}


#ifdef UNUSED_FUNCTION
void amidar_draw_background(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	const UINT8 *prom = memory_region(machine, "user1");
	rectangle draw;
	int x;

	for (x = 0; x < 32; x++)
		if (flip_and_clip(&draw, x * 8, x * 8 + 7, cliprect))
		{
			/*
                The background PROM is connected the following way:

                   bit 0 = 0 enables the blue gun if BCB is asserted
                   bit 1 = 0 enables the red gun if BCR is asserted and
                             the green gun if BCG is asserted
                   bits 2-7 are unconnected

                The background color generator is connected this way:

                    RED   - 270 ohm resistor
                    GREEN - 560 ohm resistor
                    BLUE  - 470 ohm resistor
            */
			UINT8 red = ((~prom[x] & 0x02) && background_red) ? 0x7c : 0x00;
			UINT8 green = ((~prom[x] & 0x02) && background_green) ? 0x3c : 0x00;
			UINT8 blue = ((~prom[x] & 0x01) && background_blue) ? 0x47 : 0x00;
			bitmap_fill(bitmap, &draw, MAKE_RGB(red, green, blue));
		}
}
#endif


void turtles_draw_background(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	/*
        The background color generator is connected this way:

            RED   - 390 ohm resistor
            GREEN - 470 ohm resistor
            BLUE  - 390 ohm resistor
    */
	bitmap_fill(bitmap, cliprect, MAKE_RGB(background_red * 0x55, background_green * 0x47, background_blue * 0x55));
}


void scramble_draw_background(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	/* blue background - 390 ohm resistor */
	bitmap_fill(bitmap, cliprect, background_enable ? MAKE_RGB(0,0,0x56) : RGB_BLACK);

	/* update the star origin to the current frame */
	stars_update_origin(machine);

	/* render stars if enabled */
	if (stars_enabled)
	{
		int blink_state = stars_blink_state & 3;
		int y;

		/* iterate over scanlines */
		for (y = cliprect->min_y; y <= cliprect->max_y; y++)
		{
			/* blink state 2 suppressed stars when 2V == 0 */
			if (blink_state != 2 || (y & 2) != 0)
			{
				/* blink states 0 and 1 suppress stars when certain bits of the color == 0 */
				static const UINT8 colormask_table[4] = { 0x20, 0x08, 0xff, 0xff };
				stars_draw_row(bitmap, 256, y, y * 512, colormask_table[blink_state]);
			}
		}
	}
}


void jumpbug_draw_background(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	/* blue background - 390 ohm resistor */
	bitmap_fill(bitmap, cliprect, background_enable ? MAKE_RGB(0,0,0x56) : RGB_BLACK);

	/* update the star origin to the current frame */
	stars_update_origin(machine);

	/* render stars if enabled -- same as scramble but nothing in the status area */
	if (stars_enabled)
	{
		int blink_state = stars_blink_state & 3;
		int y;

		/* iterate over scanlines */
		for (y = cliprect->min_y; y <= cliprect->max_y; y++)
		{
			/* blink state 2 suppressed stars when 2V == 0 */
			if (blink_state != 2 || (y & 2) != 0)
			{
				/* blink states 0 and 1 suppress stars when certain bits of the color == 0 */
				static const UINT8 colormask_table[4] = { 0x20, 0x08, 0xff, 0xff };
				stars_draw_row(bitmap, 240, y, y * 512, colormask_table[blink_state]);
			}
		}
	}
}



/*************************************
 *
 *  Bullet rendering
 *
 *************************************/

INLINE void galaxian_draw_pixel(bitmap_t *bitmap, const rectangle *cliprect, int y, int x, rgb_t color)
{
	if (y >= cliprect->min_y && y <= cliprect->max_y)
	{
		x *= GALAXIAN_XSCALE;
		x += GALAXIAN_H0START;
		if (x >= cliprect->min_x && x <= cliprect->max_x)
			*BITMAP_ADDR32(bitmap, y, x) = color;

		x++;
		if (x >= cliprect->min_x && x <= cliprect->max_x)
			*BITMAP_ADDR32(bitmap, y, x) = color;

		x++;
		if (x >= cliprect->min_x && x <= cliprect->max_x)
			*BITMAP_ADDR32(bitmap, y, x) = color;
	}
}


void galaxian_draw_bullet(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int offs, int x, int y)
{
	/*
        Both "shells" and "missiles" begin displaying when the horizontal counter
        reaches $FC, and they stop displaying when it reaches $00, resulting in
        4-pixel-long shots. The first 7 entries are called "shells" and render as
        white; the final entry is called a "missile" and renders as yellow.
    */
	x -= 4;
	galaxian_draw_pixel(bitmap, cliprect, y, x++, bullet_color[offs]);
	galaxian_draw_pixel(bitmap, cliprect, y, x++, bullet_color[offs]);
	galaxian_draw_pixel(bitmap, cliprect, y, x++, bullet_color[offs]);
	galaxian_draw_pixel(bitmap, cliprect, y, x++, bullet_color[offs]);
}


void mshuttle_draw_bullet(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int offs, int x, int y)
{
	/* verified by schematics:
        * both "W" and "Y" bullets are 4 pixels long
        * "W" bullets are enabled when H6 == 0, and are always purple
        * "Y" bullets are enabled when H6 == 1, and vary in color based on H4,H3,H2
    */
	static const rgb_t colors[8] =
	{
		MAKE_RGB(0xff,0xff,0xff),
		MAKE_RGB(0xff,0xff,0x00),
		MAKE_RGB(0x00,0xff,0xff),
		MAKE_RGB(0x00,0xff,0x00),
		MAKE_RGB(0xff,0x00,0xff),
		MAKE_RGB(0xff,0x00,0x00),
		MAKE_RGB(0x00,0x00,0xff),
		MAKE_RGB(0x00,0x00,0x00)
	};
	--x;
	galaxian_draw_pixel(bitmap, cliprect, y, x, ((x & 0x40) == 0) ? colors[(x >> 2) & 7] : MAKE_RGB(0xff,0x00,0xff));
	--x;
	galaxian_draw_pixel(bitmap, cliprect, y, x, ((x & 0x40) == 0) ? colors[(x >> 2) & 7] : MAKE_RGB(0xff,0x00,0xff));
	--x;
	galaxian_draw_pixel(bitmap, cliprect, y, x, ((x & 0x40) == 0) ? colors[(x >> 2) & 7] : MAKE_RGB(0xff,0x00,0xff));
	--x;
	galaxian_draw_pixel(bitmap, cliprect, y, x, ((x & 0x40) == 0) ? colors[(x >> 2) & 7] : MAKE_RGB(0xff,0x00,0xff));
}


void scramble_draw_bullet(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int offs, int x, int y)
{
	/*
        Scramble only has "shells", which begin displaying when the counter
        reaches $FA, and stop displaying one pixel clock layer. All shells are
        rendered as yellow.
    */
	x -= 6;
	galaxian_draw_pixel(bitmap, cliprect, y, x, MAKE_RGB(0xff,0xff,0x00));
}


void theend_draw_bullet(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int offs, int x, int y)
{
	/* Same as galaxian except blue/green are swapped */
	x -= 4;
	galaxian_draw_pixel(bitmap, cliprect, y, x++, MAKE_RGB(RGB_RED(bullet_color[offs]), RGB_BLUE(bullet_color[offs]), RGB_GREEN(bullet_color[offs])));
	galaxian_draw_pixel(bitmap, cliprect, y, x++, MAKE_RGB(RGB_RED(bullet_color[offs]), RGB_BLUE(bullet_color[offs]), RGB_GREEN(bullet_color[offs])));
	galaxian_draw_pixel(bitmap, cliprect, y, x++, MAKE_RGB(RGB_RED(bullet_color[offs]), RGB_BLUE(bullet_color[offs]), RGB_GREEN(bullet_color[offs])));
	galaxian_draw_pixel(bitmap, cliprect, y, x++, MAKE_RGB(RGB_RED(bullet_color[offs]), RGB_BLUE(bullet_color[offs]), RGB_GREEN(bullet_color[offs])));
}



/*************************************
 *
 *  Generic extensions
 *
 *************************************/

void upper_extend_tile_info(UINT16 *code, UINT8 *color, UINT8 attrib, UINT8 x)
{
	/* tiles are in the upper half of a larger ROM */
	*code += 0x100;
}


void upper_extend_sprite_info(const UINT8 *base, UINT8 *sx, UINT8 *sy, UINT8 *flipx, UINT8 *flipy, UINT16 *code, UINT8 *color)
{
	/* sprites are in the upper half of a larger ROM */
	*code += 0x40;
}



/*************************************
 *
 *  Frogger extensions
 *
 *************************************/

void frogger_extend_tile_info(UINT16 *code, UINT8 *color, UINT8 attrib, UINT8 x)
{
	*color = ((*color >> 1) & 0x03) | ((*color << 2) & 0x04);
}

void frogger_extend_sprite_info(const UINT8 *base, UINT8 *sx, UINT8 *sy, UINT8 *flipx, UINT8 *flipy, UINT16 *code, UINT8 *color)
{
	*color = ((*color >> 1) & 0x03) | ((*color << 2) & 0x04);
}



/*************************************
 *
 *  Ghostmuncher Galaxian extensions
 *
 *************************************/

void gmgalax_extend_tile_info(UINT16 *code, UINT8 *color, UINT8 attrib, UINT8 x)
{
	*code |= gfxbank[0] << 9;
//  *color |= gfxbank[0] << 3;
}

void gmgalax_extend_sprite_info(const UINT8 *base, UINT8 *sx, UINT8 *sy, UINT8 *flipx, UINT8 *flipy, UINT16 *code, UINT8 *color)
{
	*code |= (gfxbank[0] << 7) | 0x40;
	*color |= gfxbank[0] << 3;
}



/*************************************
 *
 *  Pisces extensions
 *
 *************************************/

void pisces_extend_tile_info(UINT16 *code, UINT8 *color, UINT8 attrib, UINT8 x)
{
	*code |= gfxbank[0] << 8;
}

void pisces_extend_sprite_info(const UINT8 *base, UINT8 *sx, UINT8 *sy, UINT8 *flipx, UINT8 *flipy, UINT16 *code, UINT8 *color)
{
	*code |= gfxbank[0] << 6;
}



/*************************************
 *
 *  Batman Part 2 extensions
 *
 *************************************/

void batman2_extend_tile_info(UINT16 *code, UINT8 *color, UINT8 attrib, UINT8 x)
{
	if (*code & 0x80)
		*code |= gfxbank[0] << 8;
}



/*************************************
 *
 *  Moon Cresta extensions
 *
 *************************************/

void mooncrst_extend_tile_info(UINT16 *code, UINT8 *color, UINT8 attrib, UINT8 x)
{
	if (gfxbank[2] && (*code & 0xc0) == 0x80)
		*code = (*code & 0x3f) | (gfxbank[0] << 6) | (gfxbank[1] << 7) | 0x0100;
}

void mooncrst_extend_sprite_info(const UINT8 *base, UINT8 *sx, UINT8 *sy, UINT8 *flipx, UINT8 *flipy, UINT16 *code, UINT8 *color)
{
	if (gfxbank[2] && (*code & 0x30) == 0x20)
		*code = (*code & 0x0f) | (gfxbank[0] << 4) | (gfxbank[1] << 5) | 0x40;
}



/*************************************
 *
 *  Moon Quasar extensions
 *
 *************************************/

void moonqsr_extend_tile_info(UINT16 *code, UINT8 *color, UINT8 attrib, UINT8 x)
{
	*code |= (attrib & 0x20) << 3;
}

void moonqsr_extend_sprite_info(const UINT8 *base, UINT8 *sx, UINT8 *sy, UINT8 *flipx, UINT8 *flipy, UINT16 *code, UINT8 *color)
{
	*code |= (base[2] & 0x20) << 1;
}



/*************************************
 *
 *  Moon Shuttle extensions
 *
 *************************************/

void mshuttle_extend_tile_info(UINT16 *code, UINT8 *color, UINT8 attrib, UINT8 x)
{
	*code |= (attrib & 0x30) << 4;
}

void mshuttle_extend_sprite_info(const UINT8 *base, UINT8 *sx, UINT8 *sy, UINT8 *flipx, UINT8 *flipy, UINT16 *code, UINT8 *color)
{
	*code |= (base[2] & 0x30) << 2;
}


/*************************************
 *
 *  Calipso extensions
 *
 *************************************/

void calipso_extend_sprite_info(const UINT8 *base, UINT8 *sx, UINT8 *sy, UINT8 *flipx, UINT8 *flipy, UINT16 *code, UINT8 *color)
{
	/* same as the others, but no sprite flipping, but instead the bits are used
       as extra sprite code bits, giving 256 sprite images */
	/* No flips */
	*code = base[1];
	*flipx = 0;
	*flipy = 0;
}

/*************************************
 *
 *  Jumpbug extensions
 *
 *************************************/

void jumpbug_extend_tile_info(UINT16 *code, UINT8 *color, UINT8 attrib, UINT8 x)
{
	if ((*code & 0xc0) == 0x80 && (gfxbank[2] & 0x01))
		*code += 128 + (( gfxbank[0] & 0x01) << 6) +
					   (( gfxbank[1] & 0x01) << 7) +
					   ((~gfxbank[4] & 0x01) << 8);
}

void jumpbug_extend_sprite_info(const UINT8 *base, UINT8 *sx, UINT8 *sy, UINT8 *flipx, UINT8 *flipy, UINT16 *code, UINT8 *color)
{
	if ((*code & 0x30) == 0x20 && (gfxbank[2] & 0x01) != 0)
	{
		*code += 32 + (( gfxbank[0] & 0x01) << 4) +
					  (( gfxbank[1] & 0x01) << 5) +
					  ((~gfxbank[4] & 0x01) << 6);
	}
}
