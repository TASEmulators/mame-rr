/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "includes/tubep.h"
#include "video/resnet.h"


UINT8 *tubep_textram;
UINT8 *rjammer_backgroundram;
UINT8 *tubep_backgroundram;
UINT8 *tubep_sprite_colorsharedram;

static UINT8 *spritemap;
static UINT8 prom2[32];

/* Globals */
/*static UINT8 graph_ctrl[10];*/
static UINT32	romD_addr =0;
static UINT32	romEF_addr =0;
static UINT32	E16_add_b = 0;
static UINT32	HINV = 0;
static UINT32	VINV = 0;
static UINT32	XSize = 0;
static UINT32	YSize = 0;
static UINT32	mark_1 = 0;
static UINT32	mark_2 = 0;
static UINT32	colorram_addr_hi = 0;
static UINT32	ls273_g6 = 0;
static UINT32	ls273_j6 = 0;
static UINT32	romHI_addr_mid = 0;
static UINT32	romHI_addr_msb = 0;
static UINT8	DISP = 0;
static UINT8	background_romsel = 0;
static UINT8	color_A4 = 0;
static UINT8	ls175_b7;
static UINT8	ls175_e8;
static UINT8	ls377_data = 0;
static UINT32	page = 0;



/***************************************************************************

  Convert the color PROMs into a more useable format.


  Tube Panic has two 32 bytes palette control PROMs.

  First one is connected to the RGB output this way:

  bit 7 -- 220 ohm resistor  -- \
        -- 470 ohm resistor  -- | -- 470 ohm pulldown resistor -- BLUE

        -- 220 ohm resistor  -- \
        -- 470 ohm resistor  -- | -- 470 ohm pulldown resistor -- GREEN
        -- 1  kohm resistor  -- /

        -- 220 ohm resistor  -- \
        -- 470 ohm resistor  -- | -- 470 ohm pulldown resistor -- RED
  bit 0 -- 1  kohm resistor  -- /



  The second PROM is used to control eight 74LS368s in following way:
  (see the schematics for more comprehensible picture)

  BLUE CONTROL:

  PROM bit 7 -- /enable on LS368 @E16
  PROM bit 6 -- /enable on LS368 @E15

    6 outputs (Q1 to Q6) from E16 go to BLUE output (the same line as
    the first PROM) via 6 resistors:
    Q1 -- 8200 -- BLUE
    Q2 -- 4700 -- BLUE
    Q3 -- 2200 -- BLUE
    Q4 -- 1000 -- BLUE
    Q5 --  470 -- BLUE
    Q6 --  220 -- BLUE

    6 outputs (Q1 to Q6) from E15 go to BLUE output (the same line as
    the first PROM) via 6 resistors:
    Q1 --15000 -- BLUE
    Q2 -- 8200 -- BLUE
    Q3 -- 4700 -- BLUE
    Q4 -- 2200 -- BLUE
    Q5 -- 1000 -- BLUE
    Q6 --  470 -- BLUE


  GREEN CONTROL:

  PROM bit 5 -- /enable on LS368 @E14
  PROM bit 4 -- /enable on LS368 @E13
  PROM bit 3 -- /enable on LS368 @E12

    6 outputs (Q1 to Q6) from E14 go to GREEN output (the same line as
    the first PROM) via 6 resistors:
    Q1 -- 8200 -- GREEN
    Q2 -- 4700 -- GREEN
    Q3 -- 2200 -- GREEN
    Q4 -- 1000 -- GREEN
    Q5 --  470 -- GREEN
    Q6 --  220 -- GREEN

    6 outputs (Q1 to Q6) from E13 go to GREEN output (the same line as
    the first PROM) via 6 resistors:
    Q1 --15000 -- GREEN
    Q2 -- 8200 -- GREEN
    Q3 -- 4700 -- GREEN
    Q4 -- 2200 -- GREEN
    Q5 -- 1000 -- GREEN
    Q6 --  470 -- GREEN

    6 outputs (Q1 to Q6) from E12 go to GREEN output (the same line as
    the first PROM) via 6 resistors:
    Q1 --33000 -- GREEN
    Q2 --15000 -- GREEN
    Q3 -- 8200 -- GREEN
    Q4 -- 4700 -- GREEN
    Q5 -- 2200 -- GREEN
    Q6 -- 1000 -- GREEN


  RED CONTROL:

  PROM bit 2 -- /enable on LS368 @E11
  PROM bit 1 -- /enable on LS368 @E10
  PROM bit 0 -- /enable on LS368 @E9

    6 outputs (Q1 to Q6) from E11 go to RED output (the same line as
    the first PROM) via 6 resistors:
    Q1 -- 8200 -- RED
    Q2 -- 4700 -- RED
    Q3 -- 2200 -- RED
    Q4 -- 1000 -- RED
    Q5 --  470 -- RED
    Q6 --  220 -- RED

    6 outputs (Q1 to Q6) from E10 go to RED output (the same line as
    the first PROM) via 6 resistors:
    Q1 --15000 -- RED
    Q2 -- 8200 -- RED
    Q3 -- 4700 -- RED
    Q4 -- 2200 -- RED
    Q5 -- 1000 -- RED
    Q6 --  470 -- RED

    6 outputs (Q1 to Q6) from E9 go to RED output (the same line as
    the first PROM) via 6 resistors:
    Q1 --33000 -- RED
    Q2 --15000 -- RED
    Q3 -- 8200 -- RED
    Q4 -- 4700 -- RED
    Q5 -- 2200 -- RED
    Q6 -- 1000 -- RED


***************************************************************************/


PALETTE_INIT( tubep )
{
	int i,r,g,b;

	/* background/sprites palette variables */

	static const int resistors_0[6] = { 33000, 15000, 8200, 4700, 2200, 1000 };
	static const int resistors_1[6] = { 15000,  8200, 4700, 2200, 1000,  470 };
	static const int resistors_2[6] = {  8200,  4700, 2200, 1000,  470,  220 };

	int active_resistors_r[3*6];
	int active_resistors_g[3*6];
	int active_resistors_b[2*6];

	double weights_r[3*6];
	double weights_g[3*6];
	double weights_b[2*6];

	//double output_scaler;

	/* text palette variables */

	static const int resistors_txt_rg[3] = { 1000, 470, 220 };
	static const int resistors_txt_b [2] = { 470, 220 };
	double weights_txt_rg[3];
	double weights_txt_b[2];

	memset(weights_r, 0, sizeof(weights_r));
	memset(weights_g, 0, sizeof(weights_g));
	memset(weights_b, 0, sizeof(weights_b));

	compute_resistor_weights(0,	255,	-1.0,
			3,	resistors_txt_rg,	weights_txt_rg,	470,	0,
			2,	resistors_txt_b,	weights_txt_b,	470,	0,
			0,	0,	0,	0,	0	);

	/* create text palette */

	for (i = 0; i < 32; i++)
	{
		int bit0,bit1,bit2;

		/* red component */
		bit0 = (*color_prom >> 0) & 0x01;
		bit1 = (*color_prom >> 1) & 0x01;
		bit2 = (*color_prom >> 2) & 0x01;
		r = combine_3_weights(weights_txt_rg, bit0, bit1, bit2);
		/* green component */
		bit0 = (*color_prom >> 3) & 0x01;
		bit1 = (*color_prom >> 4) & 0x01;
		bit2 = (*color_prom >> 5) & 0x01;
		g = combine_3_weights(weights_txt_rg, bit0, bit1, bit2);
		/* blue component */
		bit0 = (*color_prom >> 6) & 0x01;
		bit1 = (*color_prom >> 7) & 0x01;
		b = combine_2_weights(weights_txt_b, bit0, bit1);

		palette_set_color(machine,i, MAKE_RGB(r,g,b));

		color_prom++;
	}

	/* sprites use the second PROM to control 8 x LS368. We copy content of this PROM over here */
	for (i = 0; i < 32; i++)
	{
		prom2[i] = *color_prom;
		color_prom++;
	}



	/* create background/sprites palette */

	/* find the output scaler
       in order to do this we need to calculate the output with everything enabled.
    */

	/* red component */
	for (i=0; i<6; i++) active_resistors_r[ 0+i] = resistors_0[i];
	for (i=0; i<6; i++) active_resistors_r[ 6+i] = resistors_1[i];
	for (i=0; i<6; i++) active_resistors_r[12+i] = resistors_2[i];
	/* green component */
	for (i=0; i<6; i++) active_resistors_g[ 0+i] = resistors_0[i];
	for (i=0; i<6; i++) active_resistors_g[ 6+i] = resistors_1[i];
	for (i=0; i<6; i++) active_resistors_g[12+i] = resistors_2[i];
	/* blue component */
	for (i=0; i<6; i++) active_resistors_b[ 0+i] = resistors_1[i];
	for (i=0; i<6; i++) active_resistors_b[ 6+i] = resistors_2[i];

	/* calculate and store the scaler */
    /*output_scaler = */compute_resistor_weights(0, 255,    -1.0,
                3*6,    active_resistors_r, weights_r,  470,    0,
                3*6,    active_resistors_g, weights_g,  470,    0,
                2*6,    active_resistors_b, weights_b,  470,    0);

/*  compute_resistor_weights(0, 255,    output_scaler,
                3*6,    active_resistors_r, weights_r,  470,    0,
                3*6,    active_resistors_g, weights_g,  470,    0,
                2*6,    active_resistors_b, weights_b,  470,    0);
*/
	/* now calculate all possible outputs from the circuit */

	for (i=0; i<256; i++)
	{
		int sh;

		for (sh=0; sh<0x40; sh++)
		{
			int j     = i;			/* active low */
			int shade = sh^0x3f;	/* negated outputs */

			int bits_r[3*6];
			int bits_g[3*6];
			int bits_b[2*6];
			double out;
			int c;

			//int active_r = 3*6;
			//int active_g = 3*6;
			//int active_b = 2*6;

			for (c=0; c<6; c++)
			{
				bits_r[0 + c] = (shade>>c) & 1;
				bits_r[6 + c] = (shade>>c) & 1;
				bits_r[12+ c] = (shade>>c) & 1;

				bits_g[0 + c] = (shade>>c) & 1;
				bits_g[6 + c] = (shade>>c) & 1;
				bits_g[12+ c] = (shade>>c) & 1;

				bits_b[0 + c] = (shade>>c) & 1;
				bits_b[6 + c] = (shade>>c) & 1;
			}

			//j &= 0x7; /* only red; debug */

			/* red component */
			if ((j >> 0) & 0x01)	/* if LS368 @E9  is disabled */
			{
				for (c=0; c<6; c++) bits_r[0 +c] = 0;
				//active_r-=6;
			}
			if ((j >> 1) & 0x01)	/* if LS368 @E10 is disabled */
			{
				for (c=0; c<6; c++) bits_r[6 +c] = 0;
				//active_r-=6;
			}
			if ((j >> 2) & 0x01)	/* if LS368 @E11 is disabled */
			{
				for (c=0; c<6; c++) bits_r[12 +c] = 0;
				//active_r-=6;
			}

			/* green component */
			if ((j >> 3) & 0x01)	/* if LS368 @E12 is disabled */
			{
				for (c=0; c<6; c++) bits_g[0 +c] = 0;
				//active_g-=6;
			}
			if ((j >> 4) & 0x01)	/* if LS368 @E13 is disabled */
			{
				for (c=0; c<6; c++) bits_g[6 +c] = 0;
				//active_g-=6;
			}
			if ((j >> 5) & 0x01)	/* if LS368 @E14 is disabled */
			{
				for (c=0; c<6; c++) bits_g[12+c] = 0;
				//active_g-=6;
			}

			/* blue component */
			if ((j >> 6) & 0x01)	/* if LS368 @E15 is disabled */
			{
				for (c=0; c<6; c++) bits_b[0 +c] = 0;
				//active_b-=6;
			}
			if ((j >> 7) & 0x01)	/* if LS368 @E16 is disabled */
			{
				for (c=0; c<6; c++) bits_b[6 +c] = 0;
				//active_b-=6;
			}

			out = 0.0;
			for (c=0; c<3*6; c++)	out += weights_r[c] * bits_r[c];
			r = (int)(out + 0.5);

			out = 0.0;
			for (c=0; c<3*6; c++)	out += weights_g[c] * bits_g[c];
			g = (int)(out + 0.5);

			out = 0.0;
			for (c=0; c<2*6; c++)	out += weights_b[c] * bits_b[c];
			b = (int)(out + 0.5);

			/*logerror("Calculate [%x:%x] (active resistors:r=%i g=%i b=%i) = ", i, shade, active_r, active_g, active_b);*/
			/*logerror("r:%3i g:%3i b:%3i\n",r,g,b );*/

			palette_set_color(machine,32+i*0x40+sh, MAKE_RGB(r,g,b));
		}
	}
}


VIDEO_START( tubep )
{
	spritemap = auto_alloc_array(machine, UINT8, 256*256*2);

	/* Set up save state */
	state_save_register_global(machine, romD_addr);
	state_save_register_global(machine, romEF_addr);
	state_save_register_global(machine, E16_add_b);
	state_save_register_global(machine, HINV);
	state_save_register_global(machine, VINV);
	state_save_register_global(machine, XSize);
	state_save_register_global(machine, YSize);
	state_save_register_global(machine, mark_1);
	state_save_register_global(machine, mark_2);
	state_save_register_global(machine, colorram_addr_hi);
	state_save_register_global(machine, ls273_g6);
	state_save_register_global(machine, ls273_j6);
	state_save_register_global(machine, romHI_addr_mid);
	state_save_register_global(machine, romHI_addr_msb);
	state_save_register_global(machine, DISP);
	state_save_register_global(machine, background_romsel);
	state_save_register_global(machine, color_A4);
	state_save_register_global(machine, ls175_b7);
	state_save_register_global(machine, ls175_e8);
	state_save_register_global(machine, ls377_data);
	state_save_register_global(machine, page);
}


VIDEO_RESET( tubep )
{
	memset(spritemap,0,256*256*2);

	romD_addr = 0;
	romEF_addr = 0;
	E16_add_b = 0;
	HINV = 0;
	VINV = 0;
	XSize = 0;
	YSize = 0;
	mark_1 = 0;
	mark_2 = 0;
	colorram_addr_hi = 0;
	ls273_g6 = 0;
	ls273_j6 = 0;
	romHI_addr_mid = 0;
	romHI_addr_msb = 0;
	DISP = 0;
	background_romsel = 0;
	color_A4 = 0;
	ls175_b7 = 0x0f | 0xf0;
	ls175_e8 = 0x0f;
	ls377_data = 0;
	page = 0;
}


WRITE8_HANDLER( tubep_textram_w )
{
	tubep_textram[offset] = data;
}


WRITE8_HANDLER( tubep_background_romselect_w )
{
	background_romsel = data & 1;
}


WRITE8_HANDLER( tubep_colorproms_A4_line_w )
{
	color_A4 = (data & 1)<<4;
}


WRITE8_HANDLER( tubep_background_a000_w )
{
	ls175_b7 = ((data & 0x0f) ^ 0x0f) | 0xf0;
}


WRITE8_HANDLER( tubep_background_c000_w )
{
	ls175_e8 = ((data & 0x0f) ^ 0x0f);
}


static TIMER_CALLBACK( sprite_timer_callback )
{
	cputag_set_input_line(machine, "mcu", 0, ASSERT_LINE);
}


static void draw_sprite(running_machine *machine)
{
	UINT32	XDOT;
	UINT32	YDOT;
	UINT8 * romCxx  = memory_region(machine, "user2")+0x00000;
	UINT8 * romD10  = romCxx+0x10000;
	UINT8 * romEF13 = romCxx+0x12000;
	UINT8 * romHI2  = romCxx+0x14000;


	for (YDOT=0; (YDOT^YSize) != 0x00; YDOT++)
	{
	/* upper part of the schematic */
		UINT32 ls273_e12 = romD10[ romD_addr | YDOT ] & 0x7f;
		UINT32 romEF_addr_now = romEF_addr | ls273_e12;
		UINT32 E16_add_a = romEF13[ romEF_addr_now ] |
						 ((romEF13[0x1000 + romEF_addr_now ]&0x0f)<<8);
		UINT32 F16_add_b = E16_add_a + E16_add_b;

	/* lower part of the schematic */
		UINT32 romHI_addr = (YDOT) | (romHI_addr_mid) | (((romHI_addr_msb + 0x800) )&0x1800);
		UINT32 ls273_g4 = romHI2[ romHI_addr ];
		UINT32 ls273_j4 = romHI2[0x2000+ romHI_addr ];
		UINT32 ls86_gh5 = ls273_g4 ^ VINV;
		UINT32 ls86_ij5 = ls273_j4 ^ VINV;

		UINT32 ls157_gh7= ls273_g6 | (mark_2);
		UINT32 ls157_ij7= ls273_j6 | (mark_1);
		UINT32 ls283_gh8= (VINV & 1) + ls86_gh5 + ((ls86_gh5 & 0x80)<<1) + ls157_gh7;
		UINT32 ls283_ij8= (VINV & 1) + ls86_ij5 + ((ls86_ij5 & 0x80)<<1) + ls157_ij7;

		UINT32 ls273_g9 = ls283_gh8;
		UINT32 ls273_j9 = ls283_ij8;

		for (XDOT=0; (XDOT^XSize) != 0x00; XDOT++)
		{
	/* upper part of the schematic */
			UINT32 romD10_out = romD10[ romD_addr | XDOT ];
			UINT32 F16_add_a = (romD10_out & 0x7e) >>1;
			UINT32 romCxx_addr = (F16_add_a + F16_add_b ) & 0xffff;
			UINT32 romCxx_out = romCxx[ romCxx_addr ];

			UINT32 colorram_addr_lo = (romD10_out&1) ? (romCxx_out>>4)&0x0f: (romCxx_out>>0)&0x0f;

			UINT8 sp_data = tubep_sprite_colorsharedram[ colorram_addr_hi | colorram_addr_lo ] & 0x0f; /* 2114 4-bit RAM */

	/* lower part of the schematic */
			romHI_addr = (XDOT) | (romHI_addr_mid) | (romHI_addr_msb);
			ls273_g4 = romHI2[ romHI_addr ];
			ls273_j4 = romHI2[0x2000+ romHI_addr ];
			ls86_gh5 = ls273_g4 ^ HINV;
			ls86_ij5 = ls273_j4 ^ HINV;

			ls157_gh7= ls273_g9;
			ls157_ij7= ls273_j9;
			ls283_gh8= (HINV & 1) + ls86_gh5 + ((ls86_gh5 & 0x80)<<1) + ls157_gh7;
			ls283_ij8= (HINV & 1) + ls86_ij5 + ((ls86_ij5 & 0x80)<<1) + ls157_ij7;


			if ( !((ls283_gh8&256) | (ls283_ij8&256)) ) /* skip wrapped sprite area - PAL12L6 (PLA019 in Roller Jammer schematics)*/
			{
				if ( spritemap[ (ls283_gh8&255) + (ls283_ij8&255)*256 + DISP*256*256 ] == 0x0f )
					spritemap[ (ls283_gh8&255) + (ls283_ij8&255)*256 + DISP*256*256 ] = sp_data;
			}
		}
	}
}


WRITE8_HANDLER( tubep_sprite_control_w )
{
	if (offset < 10)
	{
		/*graph_ctrl[offset] = data;*/
		switch(offset)
		{
		case 0:	/*a*/
			romEF_addr = (0x010 | (data & 0x0f))<<7; /*roms @F13, @E13 have A11 lines connected to +5V directly */
			HINV = (data & 0x10) ? 0xff: 0x00;
			VINV = (data & 0x20) ? 0xff: 0x00;
			break;

		case 1:	/*b: XSize-1 */
			XSize = data & 0x7f;
			mark_2 = (data&0x80)<<1;
			break;

		case 2:	/*c: YSize-1 */
			YSize = data & 0x7f;
			mark_1 = (data&0x80)<<1;
			break;

		case 3:	/*d*/
			ls273_g6 = (data & 0xff);
			break;

		case 4:	/*e*/
			ls273_j6 = (data & 0xff);
			break;

		case 5:	/*f*/
			romHI_addr_mid = (data & 0x0f)<<7;
			romHI_addr_msb = (data & 0x30)<<7;
			break;

		case 6:	/*g*/
			romD_addr = (data & 0x3f)<<7;
			break;

		case 7:	/*h: adder input LSB*/
			E16_add_b = ((data & 0xff) << 0) | (E16_add_b & 0xff00);
			break;

		case 8:	/*J: adder input MSB*/
			E16_add_b = ((data & 0xff) << 8) | (E16_add_b & 0x00ff);
			break;

		case 9:	/*K*/
			/*write to: LS174 @J3 to set color bank (hi address lines to 2114 colorram @J1 ) */
			colorram_addr_hi = (data & 0x3f) << 4;

			/*write to: LS74 @D13 to clear the interrupt line /SINT
            /SINT line will be reasserted in XSize * YSize cycles (RH0 signal cycles)
            */
			/* 1.clear the /SINT interrupt line */
			cputag_set_input_line(space->machine, "mcu", 0, CLEAR_LINE);

			/* 2.assert /SINT again after this time */
			timer_set( space->machine, attotime_mul(ATTOTIME_IN_HZ(19968000/8), (XSize+1)*(YSize+1)), NULL, 0, sprite_timer_callback);

			/* 3.clear of /SINT starts sprite drawing circuit */
			draw_sprite(space->machine);
			break;
		}
	}
}

void tubep_vblank_end(void)
{
	DISP = DISP ^ 1;
	/* logerror("EOF: DISP after this is=%i, and clearing it now.\n", DISP); */
	/* clear the new frame (the one that was (just) displayed)*/
	memset(spritemap+DISP*256*256, 0x0f, 256*256);
}


VIDEO_UPDATE( tubep )
{
	int DISP_ = DISP^1;

	pen_t pen_base = 32; //change it later

	UINT32 v;
	UINT8 *text_gfx_base = memory_region(screen->machine, "gfx1");
	UINT8 *romBxx = memory_region(screen->machine, "user1") + 0x2000*background_romsel;

	/* logerror(" update: from DISP=%i y_min=%3i y_max=%3i\n", DISP_, cliprect->min_y, cliprect->max_y+1); */

	for (v = cliprect->min_y; v <= cliprect->max_y; v++)	/* only for current scanline */
	{
		UINT32 h;
		UINT32 sp_data0=0,sp_data1=0,sp_data2=0;
		for (h = 0*8; h < 32*8; h++)
		{
			offs_t text_offs;
			UINT8 text_code;
			UINT8 text_gfx_data;

			sp_data2 = sp_data1;
			sp_data1 = sp_data0;
			sp_data0 = spritemap[ h + v*256 +(DISP_*256*256) ];

			text_offs = ((v >> 3) << 6) | ((h >> 3) << 1);
			text_code = tubep_textram[text_offs];
			text_gfx_data = text_gfx_base[(text_code << 3) | (v & 0x07)];

			if (text_gfx_data & (0x80 >> (h & 0x07)))
				*BITMAP_ADDR16(bitmap, v, h) = (tubep_textram[text_offs + 1] & 0x0f) | color_A4;
			else
			{
				UINT32 bg_data;
				UINT32 sp_data;

				UINT32 romB_addr = (((h>>1)&0x3f)^((h&0x80)?0x00:0x3f)) | (((v&0x7f)^((v&0x80)?0x00:0x7f))<<6);

				UINT8  rom_select = (h&0x01) ^ (((h&0x80)>>7)^1);

				/* read from ROMs: B3/4 or B5/6 */
				UINT8 romB_data_h = romBxx[ 0x4000 + 0x4000*rom_select + romB_addr ];
				/* romB_data_h = output of LS374 @B3 or @B4 */

				UINT32 VR_addr = ((romB_data_h + ls175_b7) & 0xfe) << 2;
				/* VR_addr = output of LS157s @B1 and @B6 */

				UINT8 xor_logic = (((h^v)&0x80)>>7) ^ (background_romsel & (((v&0x80)>>7)^1));

				/* read from ROMs: B1/2 */
				UINT8 romB_data_l = romBxx[ romB_addr ] ^ (xor_logic?0xff:0x00);
				/* romB_data_l = output of LS273 @B10 */

				UINT8 ls157_b11 = (romB_data_l >> ((rom_select==0)?4:0))&0x0f;

				UINT8 ls283_b12 = (ls157_b11 + ls175_e8) & 0x0f;

				VR_addr |= (ls283_b12>>1);

				bg_data = tubep_backgroundram[ VR_addr ];

				romB_data_h>>=2;

				if ((sp_data0 != 0x0f) && (sp_data1 == 0x0f) && (sp_data2 != 0x0f))
					sp_data = sp_data2;
				else
					sp_data = sp_data1;

				if (sp_data != 0x0f)
					bg_data = prom2[sp_data | color_A4];

				*BITMAP_ADDR16(bitmap, v, h) = pen_base + bg_data*64 + romB_data_h;
			}
		}
	}

	return 0;
}




/***************************************************************************

  Convert the color PROMs into a more useable format.

  Roller Jammer has two 32 bytes palette PROMs, connected to the RGB
  output this way:

  bit 7 -- 220 ohm resistor  -- \
        -- 470 ohm resistor  -- | -- 470 ohm pulldown resistor -- BLUE

        -- 220 ohm resistor  -- \
        -- 470 ohm resistor  -- | -- 470 ohm pulldown resistor -- GREEN
        -- 1  kohm resistor  -- /

        -- 220 ohm resistor  -- \
        -- 470 ohm resistor  -- | -- 470 ohm pulldown resistor -- RED
  bit 0 -- 1  kohm resistor  -- /

***************************************************************************/

PALETTE_INIT( rjammer )
{
	int i;

	static const int resistors_rg[3] = { 1000, 470, 220 };
	static const int resistors_b [2] = { 470, 220 };
	double weights_rg[3];
	double weights_b[2];

	compute_resistor_weights(0,	255,	-1.0,
			3,	resistors_rg,	weights_rg,	470,	0,
			2,	resistors_b,	weights_b,	470,	0,
			0,	0,	0,	0,	0	);

	for (i = 0;i < machine->total_colors();i++)
	{
		int bit0,bit1,bit2,r,g,b;

		/* red component */
		bit0 = (*color_prom >> 0) & 0x01;
		bit1 = (*color_prom >> 1) & 0x01;
		bit2 = (*color_prom >> 2) & 0x01;
		r = combine_3_weights(weights_rg, bit0, bit1, bit2);
		/* green component */
		bit0 = (*color_prom >> 3) & 0x01;
		bit1 = (*color_prom >> 4) & 0x01;
		bit2 = (*color_prom >> 5) & 0x01;
		g = combine_3_weights(weights_rg, bit0, bit1, bit2);
		/* blue component */
		bit0 = (*color_prom >> 6) & 0x01;
		bit1 = (*color_prom >> 7) & 0x01;
		b = combine_2_weights(weights_b, bit0, bit1);

		palette_set_color(machine,i, MAKE_RGB(r,g,b));

		color_prom++;
	}
}


WRITE8_HANDLER( rjammer_background_LS377_w )
{
	ls377_data = data & 0xff;
}


WRITE8_HANDLER( rjammer_background_page_w )
{
	page = (data & 1) * 0x200;
}


VIDEO_UPDATE( rjammer )
{
	int DISP_ = DISP^1;

	UINT32 v;
	UINT8 *text_gfx_base = memory_region(screen->machine, "gfx1");
	UINT8 *rom13D  = memory_region(screen->machine, "user1");
	UINT8 *rom11BD = rom13D+0x1000;
	UINT8 *rom19C  = rom13D+0x5000;

	/* this can be optimized further by extracting constants out of the loop */
	/* especially read from ROM19C can be done once per 8 pixels*/
	/* and the data could be bitswapped beforehand */

	for (v = cliprect->min_y; v <= cliprect->max_y; v++)	/* only for current scanline */
	{
		UINT32 h;
		UINT32 sp_data0=0,sp_data1=0,sp_data2=0;
		UINT8 pal14h4_pin19;
		UINT8 pal14h4_pin18;
		UINT8 pal14h4_pin13;

		UINT32 addr = (v*2) | page;
		UINT32 ram_data = rjammer_backgroundram[ addr ] + 256*(rjammer_backgroundram[ addr+1 ]&0x2f);

		addr = (v>>3) | ((ls377_data&0x1f)<<5);
		pal14h4_pin13 = (rom19C[addr] >> ((v&7)^7) ) &1;
		pal14h4_pin19 = (ram_data>>13) & 1;

		for (h = 0*8; h < 32*8; h++)
		{
			offs_t text_offs;
			UINT8 text_code;
			UINT8 text_gfx_data;

			sp_data2 = sp_data1;
			sp_data1 = sp_data0;
			sp_data0 = spritemap[ h + v*256 +(DISP_*256*256) ];

			text_offs = ((v >> 3) << 6) | ((h >> 3) << 1);
			text_code = tubep_textram[text_offs];
			text_gfx_data = text_gfx_base[(text_code << 3) | (v & 0x07)];

			if (text_gfx_data & (0x80 >> (h & 0x07)))
				*BITMAP_ADDR16(bitmap, v, h) = 0x10 | (tubep_textram[text_offs + 1] & 0x0f);
			else
			{
				UINT32 sp_data;

				if ((sp_data0 != 0x0f) && (sp_data1 == 0x0f) && (sp_data2 != 0x0f))
					sp_data = sp_data2;
				else
					sp_data = sp_data1;

				if (sp_data != 0x0f)
					*BITMAP_ADDR16(bitmap, v, h) = 0x00 + sp_data;
				else
				{
					UINT32 bg_data;
					UINT8 color_bank;

					UINT32 ls283 = (ram_data & 0xfff) + h;
					UINT32 rom13D_addr = ((ls283>>4)&0x00f) | (v&0x0f0) | (ls283&0xf00);

					/* note: there is a jumper between bit 7 and bit 6 lines (bit 7 line is unused by default) */
					/* default: bit 6 is rom select signal 0=rom @11B, 1=rom @11D */

					UINT32 rom13D_data = rom13D[ rom13D_addr ] & 0x7f;
					/* rom13d_data is actually a content of LS377 @14C */


					UINT32 rom11BD_addr = (rom13D_data<<7) | ((v&0x0f)<<3) | ((ls283>>1)&0x07);
					UINT32 rom11_data = rom11BD[ rom11BD_addr];

					if ((ls283&1)==0)
						bg_data = rom11_data & 0x0f;
					else
						bg_data = (rom11_data>>4) & 0x0f;

					addr = (h>>3) | (ls377_data<<5);
					pal14h4_pin18 = (rom19C[addr] >> ((h&7)^7) ) &1;

					/*
                        PAL14H4 @15A funct

                        PIN6 = disable color on offscreen area
                        PIN19,PIN18,PIN13 = arguments for PIN17 function
                        PIN17 = background color bank (goes to A4 line on PROM @16A)
                                formula for PIN17 is:

                                PIN17 =  ( PIN13 & PIN8 & PIN9 & !PIN11 &  PIN12 )
                                       | ( PIN18 & PIN8 & PIN9 &  PIN11 & !PIN12 )
                                       | ( PIN19 )
                                where:
                                PIN 8 = bit 3 of bg_data
                                PIN 9 = bit 2 of bg_data
                                PIN 11= bit 1 of bg_data
                                PIN 12= bit 0 of bg_data

                        not used by now, but for the record:

                        PIN15 = select prom @16B (active low)
                        PIN16 = select prom @16A (active low)
                        PINs: 1,2,3,4,5 and 7,14 are used for priority system
                    */
					color_bank =  (pal14h4_pin13 & ((bg_data&0x08)>>3) & ((bg_data&0x04)>>2) & (((bg_data&0x02)>>1)^1) &  (bg_data&0x01)    )
								| (pal14h4_pin18 & ((bg_data&0x08)>>3) & ((bg_data&0x04)>>2) &  ((bg_data&0x02)>>1)    & ((bg_data&0x01)^1) )
								| (pal14h4_pin19);
					*BITMAP_ADDR16(bitmap, v, h) = 0x20 + color_bank*0x10 + bg_data;
				}
			}
		}
	}

	return 0;
}
