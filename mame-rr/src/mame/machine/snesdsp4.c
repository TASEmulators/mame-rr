/************************************************************************

    DSP-4 emulator code

    Copyright (c) 2004-2009 Dreamer Nom, John Weidman, Kris Bleakley,
    Nach, z80 gaiden, and Jonas Quinn

    This code is released by ZSNES Team under GNU General Public License
    version 2 as published by the Free Software Foundation.
    The implementation below is released under the MAME license for use
    in MAME, MESS and derivatives by permission of the authors.

************************************************************************/


/*
Due recognition and credit are given on Overload's DSP website.
Thank those contributors for their hard work on this chip.


Fixed-point math reminder:

[sign, integer, fraction]
1.15.00 * 1.15.00 = 2.30.00 -> 1.30.00 (DSP) -> 1.31.00 (LSB is '0')
1.15.00 * 1.00.15 = 2.15.15 -> 1.15.15 (DSP) -> 1.15.16 (LSB is '0')
*/

#include "snesdsp4.h"

static struct dsp4_t dsp4;
static struct dsp4_vars_t dsp4_vars;

INLINE UINT16 READ_WORD(UINT8 *addr)
{
	return (addr[0]) + (addr[1] << 8);
}

INLINE UINT32 READ_DWORD(UINT8 *addr)
{
	return (addr[0]) + (addr[1] << 8) + (addr[2] << 16) + (addr[3] << 24);
}

INLINE void WRITE_WORD(UINT8 *addr, UINT16 data)
{
	addr[0] = data;
	addr[1] = data >> 8;
}


//////////////////////////////////////////////////////////////

// input protocol

static INT16 DSP4_READ_WORD( void )
{
	INT16 out;

	out = READ_WORD(dsp4.parameters + dsp4.in_index);
	dsp4.in_index += 2;

	return out;
}

static INT32 DSP4_READ_DWORD( void )
{
	INT32 out;

	out = READ_DWORD(dsp4.parameters + dsp4.in_index);
	dsp4.in_index += 4;

	return out;
}


//////////////////////////////////////////////////////////////

// output protocol

#define DSP4_CLEAR_OUT() \
{ dsp4.out_count = 0; dsp4.out_index = 0; }

#define DSP4_WRITE_BYTE( d ) \
{ WRITE_WORD( dsp4.output + dsp4.out_count, ( d ) ); dsp4.out_count++; }

#define DSP4_WRITE_WORD( d ) \
{ WRITE_WORD( dsp4.output + dsp4.out_count, ( d ) ); dsp4.out_count += 2; }

#ifndef MSB_FIRST
#define DSP4_WRITE_16_WORD( d ) \
{ memcpy(dsp4.output + dsp4.out_count, ( d ), 32); dsp4.out_count += 32; }
#else
#define DSP4_WRITE_16_WORD( d )                             \
{                                                           \
	INT16 *p = ( d ), *end = ( d )+16;                    \
	for (; p != end; p++)                                 \
	{                                                     \
		WRITE_WORD( dsp4.output + dsp4.out_count, *p ); \
	}                                                     \
	dsp4.out_count += 32;                                 \
}
#endif

#ifdef PRINT_OP
#define DSP4_WRITE_DEBUG( x, d ) \
	WRITE_WORD( nop + x, d );
#endif

#ifdef DEBUG_DSP
#define DSP4_WRITE_DEBUG( x, d ) \
	WRITE_WORD( nop + x, d );
#endif

//////////////////////////////////////////////////////////////

// used to wait for dsp i/o

#define DSP4_WAIT( x ) \
	dsp4.in_index = 0; dsp4_vars.Logic = x; return;

//////////////////////////////////////////////////////////////

// 1.7.8 -> 1.15.16
#define SEX78( a ) ( ( (INT32) ( (INT16) (a) ) ) << 8 )

// 1.15.0 -> 1.15.16
#define SEX16( a ) ( ( (INT32) ( (INT16) (a) ) ) << 16 )

#ifdef PRINT_OP
#define U16( a ) ( (UINT16) ( a ) )
#endif

#ifdef DEBUG_DSP
#define U16( a ) ( (UINT16) ( a ) )
#endif

//////////////////////////////////////////////////////////////

// Attention: This lookup table is not verified
static const UINT16 div_lut[64] = { 0x0000, 0x8000, 0x4000, 0x2aaa, 0x2000, 0x1999, 0x1555, 0x1249, 0x1000, 0x0e38,
                                    0x0ccc, 0x0ba2, 0x0aaa, 0x09d8, 0x0924, 0x0888, 0x0800, 0x0787, 0x071c, 0x06bc,
                                    0x0666, 0x0618, 0x05d1, 0x0590, 0x0555, 0x051e, 0x04ec, 0x04bd, 0x0492, 0x0469,
                                    0x0444, 0x0421, 0x0400, 0x03e0, 0x03c3, 0x03a8, 0x038e, 0x0375, 0x035e, 0x0348,
                                    0x0333, 0x031f, 0x030c, 0x02fa, 0x02e8, 0x02d8, 0x02c8, 0x02b9, 0x02aa, 0x029c,
                                    0x028f, 0x0282, 0x0276, 0x026a, 0x025e, 0x0253, 0x0249, 0x023e, 0x0234, 0x022b,
                                    0x0222, 0x0219, 0x0210, 0x0208,  };
static INT16 dsp4_Inverse(INT16 value)
{
	// saturate bounds
	if (value < 0)
	{
		value = 0;
	}
	if (value > 63)
	{
		value = 63;
	}

	return div_lut[value];
}

//////////////////////////////////////////////////////////////

// Prototype
static void dsp4_OP0B(UINT8 *draw, INT16 sp_x, INT16 sp_y, INT16 sp_attr, UINT8 size, UINT8 stop);

//////////////////////////////////////////////////////////////

// OP00
static void dsp4_Multiply(INT16 Multiplicand, INT16 Multiplier, INT32 *Product)
{
	*Product = (Multiplicand * Multiplier << 1) >> 1;
}

//////////////////////////////////////////////////////////////


static void dsp4_OP01( void )
{
	dsp4.waiting4command = 0;

	// op flow control
	switch (dsp4_vars.Logic)
	{
		case 1:
			goto resume1; break;
		case 2:
			goto resume2; break;
		case 3:
			goto resume3; break;
	}

	////////////////////////////////////////////////////
	// process initial inputs

	// sort inputs
	dsp4_vars.world_y = DSP4_READ_DWORD();
	dsp4_vars.poly_bottom[0][0] = DSP4_READ_WORD();
	dsp4_vars.poly_top[0][0] = DSP4_READ_WORD();
	dsp4_vars.poly_cx[1][0] = DSP4_READ_WORD();
	dsp4_vars.viewport_bottom = DSP4_READ_WORD();
	dsp4_vars.world_x = DSP4_READ_DWORD();
	dsp4_vars.poly_cx[0][0] = DSP4_READ_WORD();
	dsp4_vars.poly_ptr[0][0] = DSP4_READ_WORD();
	dsp4_vars.world_yofs = DSP4_READ_WORD();
	dsp4_vars.world_dy = DSP4_READ_DWORD();
	dsp4_vars.world_dx = DSP4_READ_DWORD();
	dsp4_vars.distance = DSP4_READ_WORD();
	DSP4_READ_WORD(); // 0x0000
	dsp4_vars.world_xenv = DSP4_READ_DWORD();
	dsp4_vars.world_ddy = DSP4_READ_WORD();
	dsp4_vars.world_ddx = DSP4_READ_WORD();
	dsp4_vars.view_yofsenv = DSP4_READ_WORD();

	// initial (x,y,offset) at starting dsp4_vars.raster line
	dsp4_vars.view_x1 = (INT16)((dsp4_vars.world_x + dsp4_vars.world_xenv) >> 16);
	dsp4_vars.view_y1 = (INT16)(dsp4_vars.world_y >> 16);
	dsp4_vars.view_xofs1 = (INT16)(dsp4_vars.world_x >> 16);
	dsp4_vars.view_yofs1 = dsp4_vars.world_yofs;
	dsp4_vars.view_turnoff_x = 0;
	dsp4_vars.view_turnoff_dx = 0;

	// first dsp4_vars.raster line
	dsp4_vars.poly_raster[0][0] = dsp4_vars.poly_bottom[0][0];

	do
	{
		////////////////////////////////////////////////////
		// process one iteration of projection

		// perspective projection of world (x,y,scroll) points
		// based on the current projection lines
		dsp4_vars.view_x2 = (INT16)(( ( ( dsp4_vars.world_x + dsp4_vars.world_xenv ) >> 16 ) * dsp4_vars.distance >> 15 ) + ( dsp4_vars.view_turnoff_x * dsp4_vars.distance >> 15 ));
		dsp4_vars.view_y2 = (INT16)((dsp4_vars.world_y >> 16) * dsp4_vars.distance >> 15);
		dsp4_vars.view_xofs2 = dsp4_vars.view_x2;
		dsp4_vars.view_yofs2 = (dsp4_vars.world_yofs * dsp4_vars.distance >> 15) + dsp4_vars.poly_bottom[0][0] - dsp4_vars.view_y2;


		// 1. World x-location before transformation
		// 2. Viewer x-position at the next
		// 3. World y-location before perspective projection
		// 4. Viewer y-position below the horizon
		// 5. Number of dsp4_vars.raster lines drawn in this iteration

		DSP4_CLEAR_OUT();
		DSP4_WRITE_WORD((UINT16)((dsp4_vars.world_x + dsp4_vars.world_xenv) >> 16));
		DSP4_WRITE_WORD(dsp4_vars.view_x2);
		DSP4_WRITE_WORD((UINT16)(dsp4_vars.world_y >> 16));
		DSP4_WRITE_WORD(dsp4_vars.view_y2);

		//////////////////////////////////////////////////////

		// SR = 0x00

		// determine # of dsp4_vars.raster lines used
		dsp4_vars.segments = dsp4_vars.poly_raster[0][0] - dsp4_vars.view_y2;

		// prevent overdraw
		if (dsp4_vars.view_y2 >= dsp4_vars.poly_raster[0][0])
			dsp4_vars.segments = 0;
		else
			dsp4_vars.poly_raster[0][0] = dsp4_vars.view_y2;

		// don't draw outside the window
		if (dsp4_vars.view_y2 < dsp4_vars.poly_top[0][0])
		{
			dsp4_vars.segments = 0;

			// flush remaining dsp4_vars.raster lines
			if (dsp4_vars.view_y1 >= dsp4_vars.poly_top[0][0])
				dsp4_vars.segments = dsp4_vars.view_y1 - dsp4_vars.poly_top[0][0];
		}

		// SR = 0x80

		DSP4_WRITE_WORD(dsp4_vars.segments);

		//////////////////////////////////////////////////////

		// scan next command if no SR check needed
		if (dsp4_vars.segments)
		{
			INT32 px_dx, py_dy;
			INT32 x_scroll, y_scroll;

			// SR = 0x00

			// linear interpolation (lerp) between projected points
			px_dx = (dsp4_vars.view_xofs2 - dsp4_vars.view_xofs1) * dsp4_Inverse(dsp4_vars.segments) << 1;
			py_dy = (dsp4_vars.view_yofs2 - dsp4_vars.view_yofs1) * dsp4_Inverse(dsp4_vars.segments) << 1;

			// starting step values
			x_scroll = SEX16(dsp4_vars.poly_cx[0][0] + dsp4_vars.view_xofs1);
			y_scroll = SEX16(-dsp4_vars.viewport_bottom + dsp4_vars.view_yofs1 + dsp4_vars.view_yofsenv + dsp4_vars.poly_cx[1][0] - dsp4_vars.world_yofs);

			// SR = 0x80

			// rasterize line
			for (dsp4_vars.lcv = 0; dsp4_vars.lcv < dsp4_vars.segments; dsp4_vars.lcv++)
			{
				// 1. HDMA memory pointer (bg1)
				// 2. vertical scroll offset ($210E)
				// 3. horizontal scroll offset ($210D)

				DSP4_WRITE_WORD(dsp4_vars.poly_ptr[0][0]);
				DSP4_WRITE_WORD((UINT16)((y_scroll + 0x8000) >> 16));
				DSP4_WRITE_WORD((UINT16)((x_scroll + 0x8000) >> 16));


				// update memory address
				dsp4_vars.poly_ptr[0][0] -= 4;

				// update screen values
				x_scroll += px_dx;
				y_scroll += py_dy;
			}
		}

		////////////////////////////////////////////////////
		// Post-update

		// update new viewer (x,y,scroll) to last dsp4_vars.raster line drawn
		dsp4_vars.view_x1 = dsp4_vars.view_x2;
		dsp4_vars.view_y1 = dsp4_vars.view_y2;
		dsp4_vars.view_xofs1 = dsp4_vars.view_xofs2;
		dsp4_vars.view_yofs1 = dsp4_vars.view_yofs2;

		// add deltas for projection lines
		dsp4_vars.world_dx += SEX78(dsp4_vars.world_ddx);
		dsp4_vars.world_dy += SEX78(dsp4_vars.world_ddy);

		// update projection lines
		dsp4_vars.world_x += (dsp4_vars.world_dx + dsp4_vars.world_xenv);
		dsp4_vars.world_y += dsp4_vars.world_dy;

		// update road turnoff position
		dsp4_vars.view_turnoff_x += dsp4_vars.view_turnoff_dx;

		////////////////////////////////////////////////////
		// command check

		// scan next command
		dsp4.in_count = 2;
		DSP4_WAIT(1) resume1 :

		// check for termination
		dsp4_vars.distance = DSP4_READ_WORD();
		if (dsp4_vars.distance == -0x8000)
			break;

		// road turnoff
		if( (UINT16) dsp4_vars.distance == 0x8001 )
		{
			dsp4.in_count = 6;
			DSP4_WAIT(2) resume2:

			dsp4_vars.distance = DSP4_READ_WORD();
			dsp4_vars.view_turnoff_x = DSP4_READ_WORD();
			dsp4_vars.view_turnoff_dx = DSP4_READ_WORD();

			// factor in new changes
			dsp4_vars.view_x1 += ( dsp4_vars.view_turnoff_x * dsp4_vars.distance >> 15 );
			dsp4_vars.view_xofs1 += ( dsp4_vars.view_turnoff_x * dsp4_vars.distance >> 15 );

			// update stepping values
			dsp4_vars.view_turnoff_x += dsp4_vars.view_turnoff_dx;

			dsp4.in_count = 2;
			DSP4_WAIT(1)
		}

		// already have 2 bytes read
		dsp4.in_count = 6;
		DSP4_WAIT(3) resume3 :

		// inspect inputs
		dsp4_vars.world_ddy = DSP4_READ_WORD();
		dsp4_vars.world_ddx = DSP4_READ_WORD();
		dsp4_vars.view_yofsenv = DSP4_READ_WORD();

		// no envelope here
		dsp4_vars.world_xenv = 0;
	}
	while (1);

	// terminate op
	dsp4.waiting4command = 1;
}

//////////////////////////////////////////////////////////////


static void dsp4_OP03( void )
{
	dsp4_vars.OAM_RowMax = 33;
	memset(dsp4_vars.OAM_Row, 0, 64);
}


//////////////////////////////////////////////////////////////


static void dsp4_OP05( void )
{
	dsp4_vars.OAM_index = 0;
	dsp4_vars.OAM_bits = 0;
	memset(dsp4_vars.OAM_attr, 0, 32);
	dsp4_vars.sprite_count = 0;
}


//////////////////////////////////////////////////////////////

static void dsp4_OP06( void )
{
	DSP4_CLEAR_OUT();
	DSP4_WRITE_16_WORD(dsp4_vars.OAM_attr);
}

//////////////////////////////////////////////////////////////


static void dsp4_OP07( void )
{
	dsp4.waiting4command = 0;

	// op flow control
	switch (dsp4_vars.Logic)
	{
		case 1:
			goto resume1; break;
		case 2:
			goto resume2; break;
	}

	////////////////////////////////////////////////////
	// sort inputs

	dsp4_vars.world_y = DSP4_READ_DWORD();
	dsp4_vars.poly_bottom[0][0] = DSP4_READ_WORD();
	dsp4_vars.poly_top[0][0] = DSP4_READ_WORD();
	dsp4_vars.poly_cx[1][0] = DSP4_READ_WORD();
	dsp4_vars.viewport_bottom = DSP4_READ_WORD();
	dsp4_vars.world_x = DSP4_READ_DWORD();
	dsp4_vars.poly_cx[0][0] = DSP4_READ_WORD();
	dsp4_vars.poly_ptr[0][0] = DSP4_READ_WORD();
	dsp4_vars.world_yofs = DSP4_READ_WORD();
	dsp4_vars.distance = DSP4_READ_WORD();
	dsp4_vars.view_y2 = DSP4_READ_WORD();
	dsp4_vars.view_dy = DSP4_READ_WORD() * dsp4_vars.distance >> 15;
	dsp4_vars.view_x2 = DSP4_READ_WORD();
	dsp4_vars.view_dx = DSP4_READ_WORD() * dsp4_vars.distance >> 15;
	dsp4_vars.view_yofsenv = DSP4_READ_WORD();

	// initial (x,y,offset) at starting dsp4_vars.raster line
	dsp4_vars.view_x1 = (INT16)(dsp4_vars.world_x >> 16);
	dsp4_vars.view_y1 = (INT16)(dsp4_vars.world_y >> 16);
	dsp4_vars.view_xofs1 = dsp4_vars.view_x1;
	dsp4_vars.view_yofs1 = dsp4_vars.world_yofs;

	// first dsp4_vars.raster line
	dsp4_vars.poly_raster[0][0] = dsp4_vars.poly_bottom[0][0];


	do
	{
		////////////////////////////////////////////////////
		// process one iteration of projection

		// add shaping
		dsp4_vars.view_x2 += dsp4_vars.view_dx;
		dsp4_vars.view_y2 += dsp4_vars.view_dy;

		// vertical scroll calculation
		dsp4_vars.view_xofs2 = dsp4_vars.view_x2;
		dsp4_vars.view_yofs2 = (dsp4_vars.world_yofs * dsp4_vars.distance >> 15) + dsp4_vars.poly_bottom[0][0] - dsp4_vars.view_y2;

		// 1. Viewer x-position at the next
		// 2. Viewer y-position below the horizon
		// 3. Number of dsp4_vars.raster lines drawn in this iteration

		DSP4_CLEAR_OUT();
		DSP4_WRITE_WORD(dsp4_vars.view_x2);
		DSP4_WRITE_WORD(dsp4_vars.view_y2);

		//////////////////////////////////////////////////////

		// SR = 0x00

		// determine # of dsp4_vars.raster lines used
		dsp4_vars.segments = dsp4_vars.view_y1 - dsp4_vars.view_y2;

		// prevent overdraw
		if (dsp4_vars.view_y2 >= dsp4_vars.poly_raster[0][0])
			dsp4_vars.segments = 0;
		else
			dsp4_vars.poly_raster[0][0] = dsp4_vars.view_y2;

		// don't draw outside the window
		if (dsp4_vars.view_y2 < dsp4_vars.poly_top[0][0])
		{
			dsp4_vars.segments = 0;

			// flush remaining dsp4_vars.raster lines
			if (dsp4_vars.view_y1 >= dsp4_vars.poly_top[0][0])
				dsp4_vars.segments = dsp4_vars.view_y1 - dsp4_vars.poly_top[0][0];
		}

		// SR = 0x80

		DSP4_WRITE_WORD(dsp4_vars.segments);

		//////////////////////////////////////////////////////

		// scan next command if no SR check needed
		if (dsp4_vars.segments)
		{
			INT32 px_dx, py_dy;
			INT32 x_scroll, y_scroll;

			// SR = 0x00

			// linear interpolation (lerp) between projected points
			px_dx = (dsp4_vars.view_xofs2 - dsp4_vars.view_xofs1) * dsp4_Inverse(dsp4_vars.segments) << 1;
			py_dy = (dsp4_vars.view_yofs2 - dsp4_vars.view_yofs1) * dsp4_Inverse(dsp4_vars.segments) << 1;

			// starting step values
			x_scroll = SEX16(dsp4_vars.poly_cx[0][0] + dsp4_vars.view_xofs1);
			y_scroll = SEX16(-dsp4_vars.viewport_bottom + dsp4_vars.view_yofs1 + dsp4_vars.view_yofsenv + dsp4_vars.poly_cx[1][0] - dsp4_vars.world_yofs);

			// SR = 0x80

			// rasterize line
			for (dsp4_vars.lcv = 0; dsp4_vars.lcv < dsp4_vars.segments; dsp4_vars.lcv++)
			{
				// 1. HDMA memory pointer (bg2)
				// 2. vertical scroll offset ($2110)
				// 3. horizontal scroll offset ($210F)

				DSP4_WRITE_WORD(dsp4_vars.poly_ptr[0][0]);
				DSP4_WRITE_WORD((UINT16)((y_scroll + 0x8000) >> 16));
				DSP4_WRITE_WORD((UINT16)((x_scroll + 0x8000) >> 16));

				// update memory address
				dsp4_vars.poly_ptr[0][0] -= 4;

				// update screen values
				x_scroll += px_dx;
				y_scroll += py_dy;
			}
		}

		/////////////////////////////////////////////////////
		// Post-update

		// update new viewer (x,y,scroll) to last dsp4_vars.raster line drawn
		dsp4_vars.view_x1 = dsp4_vars.view_x2;
		dsp4_vars.view_y1 = dsp4_vars.view_y2;
		dsp4_vars.view_xofs1 = dsp4_vars.view_xofs2;
		dsp4_vars.view_yofs1 = dsp4_vars.view_yofs2;

		////////////////////////////////////////////////////
		// command check

		// scan next command
		dsp4.in_count = 2;
		DSP4_WAIT(1) resume1 :

		// check for opcode termination
		dsp4_vars.distance = DSP4_READ_WORD();
		if (dsp4_vars.distance == -0x8000)
			break;

		// already have 2 bytes in queue
		dsp4.in_count = 10;
		DSP4_WAIT(2) resume2 :

		// inspect inputs
		dsp4_vars.view_y2 = DSP4_READ_WORD();
		dsp4_vars.view_dy = DSP4_READ_WORD() * dsp4_vars.distance >> 15;
		dsp4_vars.view_x2 = DSP4_READ_WORD();
		dsp4_vars.view_dx = DSP4_READ_WORD() * dsp4_vars.distance >> 15;
		dsp4_vars.view_yofsenv = DSP4_READ_WORD();
	}
	while (1);

	dsp4.waiting4command = 1;
}

//////////////////////////////////////////////////////////////

static void dsp4_OP08( void )
{
	INT16 wleft, wright;
	INT16 view_x[2], view_y[2];
	INT16 envelope[2][2];

	dsp4.waiting4command = 0;

	// op flow control
	switch (dsp4_vars.Logic)
	{
		case 1:
			goto resume1; break;
		case 2:
			goto resume2; break;
	}

	////////////////////////////////////////////////////
	// process initial inputs for two polygons

	// clip values
	dsp4_vars.poly_clipRt[0][0] = DSP4_READ_WORD();
	dsp4_vars.poly_clipRt[0][1] = DSP4_READ_WORD();
	dsp4_vars.poly_clipRt[1][0] = DSP4_READ_WORD();
	dsp4_vars.poly_clipRt[1][1] = DSP4_READ_WORD();

	dsp4_vars.poly_clipLf[0][0] = DSP4_READ_WORD();
	dsp4_vars.poly_clipLf[0][1] = DSP4_READ_WORD();
	dsp4_vars.poly_clipLf[1][0] = DSP4_READ_WORD();
	dsp4_vars.poly_clipLf[1][1] = DSP4_READ_WORD();

	// unknown (constant) (ex. 1P/2P = $00A6, $00A6, $00A6, $00A6)
	DSP4_READ_WORD();
	DSP4_READ_WORD();
	DSP4_READ_WORD();
	DSP4_READ_WORD();

	// unknown (constant) (ex. 1P/2P = $00A5, $00A5, $00A7, $00A7)
	DSP4_READ_WORD();
	DSP4_READ_WORD();
	DSP4_READ_WORD();
	DSP4_READ_WORD();

	// polygon centering (left,right)
	dsp4_vars.poly_cx[0][0] = DSP4_READ_WORD();
	dsp4_vars.poly_cx[0][1] = DSP4_READ_WORD();
	dsp4_vars.poly_cx[1][0] = DSP4_READ_WORD();
	dsp4_vars.poly_cx[1][1] = DSP4_READ_WORD();

	// HDMA pointer locations
	dsp4_vars.poly_ptr[0][0] = DSP4_READ_WORD();
	dsp4_vars.poly_ptr[0][1] = DSP4_READ_WORD();
	dsp4_vars.poly_ptr[1][0] = DSP4_READ_WORD();
	dsp4_vars.poly_ptr[1][1] = DSP4_READ_WORD();

	// starting dsp4_vars.raster line below the horizon
	dsp4_vars.poly_bottom[0][0] = DSP4_READ_WORD();
	dsp4_vars.poly_bottom[0][1] = DSP4_READ_WORD();
	dsp4_vars.poly_bottom[1][0] = DSP4_READ_WORD();
	dsp4_vars.poly_bottom[1][1] = DSP4_READ_WORD();

	// top boundary line to clip
	dsp4_vars.poly_top[0][0] = DSP4_READ_WORD();
	dsp4_vars.poly_top[0][1] = DSP4_READ_WORD();
	dsp4_vars.poly_top[1][0] = DSP4_READ_WORD();
	dsp4_vars.poly_top[1][1] = DSP4_READ_WORD();

	// unknown
	// (ex. 1P = $2FC8, $0034, $FF5C, $0035)
	//
	// (ex. 2P = $3178, $0034, $FFCC, $0035)
	// (ex. 2P = $2FC8, $0034, $FFCC, $0035)

	DSP4_READ_WORD();
	DSP4_READ_WORD();
	DSP4_READ_WORD();
	DSP4_READ_WORD();

	// look at guidelines for both polygon shapes
	dsp4_vars.distance = DSP4_READ_WORD();
	view_x[0] = DSP4_READ_WORD();
	view_y[0] = DSP4_READ_WORD();
	view_x[1] = DSP4_READ_WORD();
	view_y[1] = DSP4_READ_WORD();

	// envelope shaping guidelines (one frame only)
	envelope[0][0] = DSP4_READ_WORD();
	envelope[0][1] = DSP4_READ_WORD();
	envelope[1][0] = DSP4_READ_WORD();
	envelope[1][1] = DSP4_READ_WORD();

	// starting base values to project from
	dsp4_vars.poly_start[0] = view_x[0];
	dsp4_vars.poly_start[1] = view_x[1];

	// starting dsp4_vars.raster lines to begin drawing
	dsp4_vars.poly_raster[0][0] = view_y[0];
	dsp4_vars.poly_raster[0][1] = view_y[0];
	dsp4_vars.poly_raster[1][0] = view_y[1];
	dsp4_vars.poly_raster[1][1] = view_y[1];

	// starting distances
	dsp4_vars.poly_plane[0] = dsp4_vars.distance;
	dsp4_vars.poly_plane[1] = dsp4_vars.distance;

	// SR = 0x00

	// re-center coordinates
	wleft = dsp4_vars.poly_cx[0][0] - view_x[0] + envelope[0][0];
	wright = dsp4_vars.poly_cx[0][1] - view_x[0] + envelope[0][1];

	// saturate offscreen data for polygon #1
	if (wleft < dsp4_vars.poly_clipLf[0][0])
	{
		wleft = dsp4_vars.poly_clipLf[0][0];
	}
	if (wleft > dsp4_vars.poly_clipRt[0][0])
	{
		wleft = dsp4_vars.poly_clipRt[0][0];
	}
	if (wright < dsp4_vars.poly_clipLf[0][1])
	{
		wright = dsp4_vars.poly_clipLf[0][1];
	}
	if (wright > dsp4_vars.poly_clipRt[0][1])
	{
		wright = dsp4_vars.poly_clipRt[0][1];
	}

	// SR = 0x80

	// initial output for polygon #1
	DSP4_CLEAR_OUT();
	DSP4_WRITE_BYTE(wleft & 0xff);
	DSP4_WRITE_BYTE(wright & 0xff);


	do
	{
		INT16 polygon;
		////////////////////////////////////////////////////
		// command check

		// scan next command
		dsp4.in_count = 2;
		DSP4_WAIT(1) resume1 :

		// terminate op
		dsp4_vars.distance = DSP4_READ_WORD();
		if (dsp4_vars.distance == -0x8000)
			break;

		// already have 2 bytes in queue
		dsp4.in_count = 16;

		DSP4_WAIT(2) resume2 :

		// look at guidelines for both polygon shapes
		view_x[0] = DSP4_READ_WORD();
		view_y[0] = DSP4_READ_WORD();
		view_x[1] = DSP4_READ_WORD();
		view_y[1] = DSP4_READ_WORD();

		// envelope shaping guidelines (one frame only)
		envelope[0][0] = DSP4_READ_WORD();
		envelope[0][1] = DSP4_READ_WORD();
		envelope[1][0] = DSP4_READ_WORD();
		envelope[1][1] = DSP4_READ_WORD();

		////////////////////////////////////////////////////
		// projection begins

		// init
		DSP4_CLEAR_OUT();


		//////////////////////////////////////////////
		// solid polygon renderer - 2 shapes

		for (polygon = 0; polygon < 2; polygon++)
		{
			INT32 left_inc, right_inc;
			INT16 x1_final, x2_final;
			INT16 env[2][2];
			INT16 poly;

			// SR = 0x00

			// # dsp4_vars.raster lines to draw
			dsp4_vars.segments = dsp4_vars.poly_raster[polygon][0] - view_y[polygon];

			// prevent overdraw
			if (dsp4_vars.segments > 0)
			{
				// bump drawing cursor
				dsp4_vars.poly_raster[polygon][0] = view_y[polygon];
				dsp4_vars.poly_raster[polygon][1] = view_y[polygon];
			}
			else
				dsp4_vars.segments = 0;

			// don't draw outside the window
			if (view_y[polygon] < dsp4_vars.poly_top[polygon][0])
			{
				dsp4_vars.segments = 0;

				// flush remaining dsp4_vars.raster lines
				if (view_y[polygon] >= dsp4_vars.poly_top[polygon][0])
					dsp4_vars.segments = view_y[polygon] - dsp4_vars.poly_top[polygon][0];
			}

			// SR = 0x80

			// tell user how many dsp4_vars.raster structures to read in
			DSP4_WRITE_WORD(dsp4_vars.segments);

			// normal parameters
			poly = polygon;

			/////////////////////////////////////////////////////

			// scan next command if no SR check needed
			if (dsp4_vars.segments)
			{
				INT32 win_left, win_right;

				// road turnoff selection
				if( (UINT16) envelope[ polygon ][ 0 ] == (UINT16) 0xc001 )
					poly = 1;
				else if( envelope[ polygon ][ 1 ] == 0x3fff )
					poly = 1;

				///////////////////////////////////////////////
				// left side of polygon

				// perspective correction on additional shaping parameters
				env[0][0] = envelope[polygon][0] * dsp4_vars.poly_plane[poly] >> 15;
				env[0][1] = envelope[polygon][0] * dsp4_vars.distance >> 15;

				// project new shapes (left side)
				x1_final = view_x[poly] + env[0][0];
				x2_final = dsp4_vars.poly_start[poly] + env[0][1];

				// interpolate between projected points with shaping
				left_inc = (x2_final - x1_final) * dsp4_Inverse(dsp4_vars.segments) << 1;
				if (dsp4_vars.segments == 1)
					left_inc = -left_inc;

				///////////////////////////////////////////////
				// right side of polygon

				// perspective correction on additional shaping parameters
				env[1][0] = envelope[polygon][1] * dsp4_vars.poly_plane[poly] >> 15;;
				env[1][1] = envelope[polygon][1] * dsp4_vars.distance >> 15;

				// project new shapes (right side)
				x1_final = view_x[poly] + env[1][0];
				x2_final = dsp4_vars.poly_start[poly] + env[1][1];


				// interpolate between projected points with shaping
				right_inc = (x2_final - x1_final) * dsp4_Inverse(dsp4_vars.segments) << 1;
				if (dsp4_vars.segments == 1)
					right_inc = -right_inc;

				///////////////////////////////////////////////
				// update each point on the line

				win_left = SEX16(dsp4_vars.poly_cx[polygon][0] - dsp4_vars.poly_start[poly] + env[0][0]);
				win_right = SEX16(dsp4_vars.poly_cx[polygon][1] - dsp4_vars.poly_start[poly] + env[1][0]);

				// update dsp4_vars.distance drawn into world
				dsp4_vars.poly_plane[polygon] = dsp4_vars.distance;

				// rasterize line
				for (dsp4_vars.lcv = 0; dsp4_vars.lcv < dsp4_vars.segments; dsp4_vars.lcv++)
				{
					INT16 x_left, x_right;

					// project new coordinates
					win_left += left_inc;
					win_right += right_inc;

					// grab integer portion, drop fraction (no rounding)
					x_left = (INT16)(win_left >> 16);
					x_right = (INT16)(win_right >> 16);

					// saturate offscreen data
					if (x_left < dsp4_vars.poly_clipLf[polygon][0])
						x_left = dsp4_vars.poly_clipLf[polygon][0];
					if (x_left > dsp4_vars.poly_clipRt[polygon][0])
						x_left = dsp4_vars.poly_clipRt[polygon][0];
					if (x_right < dsp4_vars.poly_clipLf[polygon][1])
						x_right = dsp4_vars.poly_clipLf[polygon][1];
					if (x_right > dsp4_vars.poly_clipRt[polygon][1])
						x_right = dsp4_vars.poly_clipRt[polygon][1];

					// 1. HDMA memory pointer
					// 2. Left window position ($2126/$2128)
					// 3. Right window position ($2127/$2129)

					DSP4_WRITE_WORD(dsp4_vars.poly_ptr[polygon][0]);
					DSP4_WRITE_BYTE(x_left & 0xff);
					DSP4_WRITE_BYTE(x_right & 0xff);


					// update memory pointers
					dsp4_vars.poly_ptr[polygon][0] -= 4;
					dsp4_vars.poly_ptr[polygon][1] -= 4;
				} // end rasterize line
			}

			////////////////////////////////////////////////
			// Post-update

			// new projection spot to continue rasterizing from
			dsp4_vars.poly_start[polygon] = view_x[poly];
		} // end polygon rasterizer
	}
	while (1);

	// unknown output
	DSP4_CLEAR_OUT();
	DSP4_WRITE_WORD(0);


	dsp4.waiting4command = 1;
}

//////////////////////////////////////////////////////////////

static void dsp4_OP09( void )
{
	dsp4.waiting4command = 0;

	// op flow control
	switch (dsp4_vars.Logic)
	{
		case 1:
			goto resume1; break;
		case 2:
			goto resume2; break;
		case 3:
			goto resume3; break;
		case 4:
			goto resume4; break;
		case 5:
			goto resume5; break;
		case 6:
			goto resume6; break;
	}

	////////////////////////////////////////////////////
	// process initial inputs

	// grab screen information
	dsp4_vars.viewport_cx = DSP4_READ_WORD();
	dsp4_vars.viewport_cy = DSP4_READ_WORD();
	DSP4_READ_WORD(); // 0x0000
	dsp4_vars.viewport_left = DSP4_READ_WORD();
	dsp4_vars.viewport_right = DSP4_READ_WORD();
	dsp4_vars.viewport_top = DSP4_READ_WORD();
	dsp4_vars.viewport_bottom = DSP4_READ_WORD();

	// starting dsp4_vars.raster line below the horizon
	dsp4_vars.poly_bottom[0][0] = dsp4_vars.viewport_bottom - dsp4_vars.viewport_cy;
	dsp4_vars.poly_raster[0][0] = 0x100;

	do
	{
		////////////////////////////////////////////////////
		// check for new sprites

		dsp4.in_count = 4;
		DSP4_WAIT(1) resume1 :

		////////////////////////////////////////////////
		// dsp4_vars.raster overdraw check

		dsp4_vars.raster = DSP4_READ_WORD();

		// continue updating the dsp4_vars.raster line where overdraw begins
		if (dsp4_vars.raster < dsp4_vars.poly_raster[0][0])
		{
			dsp4_vars.sprite_clipy = dsp4_vars.viewport_bottom - (dsp4_vars.poly_bottom[0][0] - dsp4_vars.raster);
			dsp4_vars.poly_raster[0][0] = dsp4_vars.raster;
		}

		/////////////////////////////////////////////////
		// identify sprite

		// op termination
		dsp4_vars.distance = DSP4_READ_WORD();
		if (dsp4_vars.distance == -0x8000)
			goto terminate;


		// no sprite
		if (dsp4_vars.distance == 0x0000)
		{
			continue;
		}

		////////////////////////////////////////////////////
		// process projection information

		// vehicle sprite
		if ((UINT16) dsp4_vars.distance == 0x9000)
		{
			INT16 car_left, car_right, car_back;
			INT16 impact_left, impact_back;
			INT16 world_spx, world_spy;
			INT16 view_spx, view_spy;
			UINT16 energy;

			// we already have 4 bytes we want
			dsp4.in_count = 14;
			DSP4_WAIT(2) resume2 :

			// filter inputs
			energy = DSP4_READ_WORD();
			impact_back = DSP4_READ_WORD();
			car_back = DSP4_READ_WORD();
			impact_left = DSP4_READ_WORD();
			car_left = DSP4_READ_WORD();
			dsp4_vars.distance = DSP4_READ_WORD();
			car_right = DSP4_READ_WORD();

			// calculate car's world (x,y) values
			world_spx = car_right - car_left;
			world_spy = car_back;

			// add in collision vector [needs bit-twiddling]
			world_spx -= energy * (impact_left - car_left) >> 16;
			world_spy -= energy * (car_back - impact_back) >> 16;

			// perspective correction for world (x,y)
			view_spx = world_spx * dsp4_vars.distance >> 15;
			view_spy = world_spy * dsp4_vars.distance >> 15;

			// convert to screen values
			dsp4_vars.sprite_x = dsp4_vars.viewport_cx + view_spx;
			dsp4_vars.sprite_y = dsp4_vars.viewport_bottom - (dsp4_vars.poly_bottom[0][0] - view_spy);

			// make the car's (x)-coordinate available
			DSP4_CLEAR_OUT();
			DSP4_WRITE_WORD(world_spx);

			// grab a few remaining vehicle values
			dsp4.in_count = 4;
			DSP4_WAIT(3) resume3 :

			// add vertical lift factor
			dsp4_vars.sprite_y += DSP4_READ_WORD();
		}
		// terrain sprite
		else
		{
			INT16 world_spx, world_spy;
			INT16 view_spx, view_spy;

			// we already have 4 bytes we want
			dsp4.in_count = 10;
			DSP4_WAIT(4) resume4 :

			// sort loop inputs
			dsp4_vars.poly_cx[0][0] = DSP4_READ_WORD();
			dsp4_vars.poly_raster[0][1] = DSP4_READ_WORD();
			world_spx = DSP4_READ_WORD();
			world_spy = DSP4_READ_WORD();

			// compute base dsp4_vars.raster line from the bottom
			dsp4_vars.segments = dsp4_vars.poly_bottom[0][0] - dsp4_vars.raster;

			// perspective correction for world (x,y)
			view_spx = world_spx * dsp4_vars.distance >> 15;
			view_spy = world_spy * dsp4_vars.distance >> 15;

			// convert to screen values
			dsp4_vars.sprite_x = dsp4_vars.viewport_cx + view_spx - dsp4_vars.poly_cx[0][0];
			dsp4_vars.sprite_y = dsp4_vars.viewport_bottom - dsp4_vars.segments + view_spy;
		}

		// default sprite size: 16x16
		dsp4_vars.sprite_size = 1;
		dsp4_vars.sprite_attr = DSP4_READ_WORD();

		////////////////////////////////////////////////////
		// convert tile data to SNES OAM format

		do
		{
			UINT16 header;

			INT16 sp_x, sp_y, sp_attr, sp_dattr;
			INT16 sp_dx, sp_dy;
			INT16 pixels;

			UINT8 draw;

			dsp4.in_count = 2;
			DSP4_WAIT(5) resume5 :

			draw = 1;

			// opcode termination
			dsp4_vars.raster = DSP4_READ_WORD();
			if (dsp4_vars.raster == -0x8000)
				goto terminate;

			// stop code
			if (dsp4_vars.raster == 0x0000 && !dsp4_vars.sprite_size)
				break;

			// toggle sprite size
			if (dsp4_vars.raster == 0x0000)
			{
				dsp4_vars.sprite_size = !dsp4_vars.sprite_size;
				continue;
			}

			// check for valid sprite header
			header = dsp4_vars.raster;
			header >>= 8;
			if (header != 0x20 &&
					header != 0x2e && //This is for attractor sprite
					header != 0x40 &&
					header != 0x60 &&
					header != 0xa0 &&
					header != 0xc0 &&
					header != 0xe0)
				break;

			// read in rest of sprite data
			dsp4.in_count = 4;
			DSP4_WAIT(6) resume6 :

			draw = 1;

			/////////////////////////////////////
			// process tile data

			// sprite deltas
			sp_dattr = dsp4_vars.raster;
			sp_dy = DSP4_READ_WORD();
			sp_dx = DSP4_READ_WORD();

			// update coordinates to screen space
			sp_x = dsp4_vars.sprite_x + sp_dx;
			sp_y = dsp4_vars.sprite_y + sp_dy;

			// update sprite nametable/attribute information
			sp_attr = dsp4_vars.sprite_attr + sp_dattr;

			// allow partially visibile tiles
			pixels = dsp4_vars.sprite_size ? 15 : 7;

			DSP4_CLEAR_OUT();

			// transparent tile to clip off parts of a sprite (overdraw)
			if (dsp4_vars.sprite_clipy - pixels <= sp_y &&
					sp_y <= dsp4_vars.sprite_clipy &&
					sp_x >= dsp4_vars.viewport_left - pixels &&
					sp_x <= dsp4_vars.viewport_right &&
					dsp4_vars.sprite_clipy >= dsp4_vars.viewport_top - pixels &&
					dsp4_vars.sprite_clipy <= dsp4_vars.viewport_bottom)
			{
				dsp4_OP0B(&draw, sp_x, dsp4_vars.sprite_clipy, 0x00EE, dsp4_vars.sprite_size, 0);
			}


			// normal sprite tile
			if (sp_x >= dsp4_vars.viewport_left - pixels &&
					sp_x <= dsp4_vars.viewport_right &&
					sp_y >= dsp4_vars.viewport_top - pixels &&
					sp_y <= dsp4_vars.viewport_bottom &&
					sp_y <= dsp4_vars.sprite_clipy)
			{
				dsp4_OP0B(&draw, sp_x, sp_y, sp_attr, dsp4_vars.sprite_size, 0);
			}


			// no following OAM data
			dsp4_OP0B(&draw, 0, 0x0100, 0, 0, 1);
		}
		while (1);
	}
	while (1);

	terminate : dsp4.waiting4command = 1;
}

//////////////////////////////////////////////////////////////

static const UINT16 OP0A_Values[16] = { 0x0000, 0x0030, 0x0060, 0x0090, 0x00c0, 0x00f0, 0x0120, 0x0150, 0xfe80,
                                 0xfeb0, 0xfee0, 0xff10, 0xff40, 0xff70, 0xffa0, 0xffd0 };

static void dsp4_OP0A(INT16 n2, INT16 *o1, INT16 *o2, INT16 *o3, INT16 *o4)
{
	*o4 = OP0A_Values[(n2 & 0x000f)];
	*o3 = OP0A_Values[(n2 & 0x00f0) >> 4];
	*o2 = OP0A_Values[(n2 & 0x0f00) >> 8];
	*o1 = OP0A_Values[(n2 & 0xf000) >> 12];
}

//////////////////////////////////////////////////////////////

static void dsp4_OP0B(UINT8 *draw, INT16 sp_x, INT16 sp_y, INT16 sp_attr, UINT8 size, UINT8 stop)
{
	INT16 Row1, Row2;

	// SR = 0x00

	// align to nearest 8-pixel row
	Row1 = (sp_y >> 3) & 0x1f;
	Row2 = (Row1 + 1) & 0x1f;

	// check boundaries
	if (!((sp_y < 0) || ((sp_y & 0x01ff) < 0x00eb)))
	{
		*draw = 0;
	}
	if (size)
	{
		if (dsp4_vars.OAM_Row[Row1] + 1 >= dsp4_vars.OAM_RowMax)
			*draw = 0;
		if (dsp4_vars.OAM_Row[Row2] + 1 >= dsp4_vars.OAM_RowMax)
			*draw = 0;
	}
	else
	{
		if (dsp4_vars.OAM_Row[Row1] >= dsp4_vars.OAM_RowMax)
		{
			*draw = 0;
		}
	}

	// emulator fail-safe (unknown if this really exists)
	if (dsp4_vars.sprite_count >= 128)
	{
		*draw = 0;
	}

	// SR = 0x80

	if (*draw)
	{
		// Row tiles
		if (size)
		{
			dsp4_vars.OAM_Row[Row1] += 2;
			dsp4_vars.OAM_Row[Row2] += 2;
		}
		else
		{
			dsp4_vars.OAM_Row[Row1]++;
		}

		// yield OAM output
		DSP4_WRITE_WORD(1);

		// pack OAM data: x,y,name,attr
		DSP4_WRITE_BYTE(sp_x & 0xff);
		DSP4_WRITE_BYTE(sp_y & 0xff);
		DSP4_WRITE_WORD(sp_attr);

		dsp4_vars.sprite_count++;

		// OAM: size,msb data
		// save post-oam table data for future retrieval
		dsp4_vars.OAM_attr[dsp4_vars.OAM_index] |= ((sp_x <0 || sp_x> 255) << dsp4_vars.OAM_bits);
		dsp4_vars.OAM_bits++;

		dsp4_vars.OAM_attr[dsp4_vars.OAM_index] |= (size << dsp4_vars.OAM_bits);
		dsp4_vars.OAM_bits++;

		// move to next byte in buffer
		if (dsp4_vars.OAM_bits == 16)
		{
			dsp4_vars.OAM_bits = 0;
			dsp4_vars.OAM_index++;
		}
	}
	else if (stop)
	{
		// yield no OAM output
		DSP4_WRITE_WORD(0);
	}
}

//////////////////////////////////////////////////////////////

static void dsp4_OP0D( void )
{
	dsp4.waiting4command = 0;

	// op flow control
	switch (dsp4_vars.Logic)
	{
		case 1:
			goto resume1; break;
		case 2:
			goto resume2; break;
	}

	////////////////////////////////////////////////////
	// process initial inputs

	// sort inputs
	dsp4_vars.world_y = DSP4_READ_DWORD();
	dsp4_vars.poly_bottom[0][0] = DSP4_READ_WORD();
	dsp4_vars.poly_top[0][0] = DSP4_READ_WORD();
	dsp4_vars.poly_cx[1][0] = DSP4_READ_WORD();
	dsp4_vars.viewport_bottom = DSP4_READ_WORD();
	dsp4_vars.world_x = DSP4_READ_DWORD();
	dsp4_vars.poly_cx[0][0] = DSP4_READ_WORD();
	dsp4_vars.poly_ptr[0][0] = DSP4_READ_WORD();
	dsp4_vars.world_yofs = DSP4_READ_WORD();
	dsp4_vars.world_dy = DSP4_READ_DWORD();
	dsp4_vars.world_dx = DSP4_READ_DWORD();
	dsp4_vars.distance = DSP4_READ_WORD();
	DSP4_READ_WORD(); // 0x0000
	dsp4_vars.world_xenv = SEX78(DSP4_READ_WORD());
	dsp4_vars.world_ddy = DSP4_READ_WORD();
	dsp4_vars.world_ddx = DSP4_READ_WORD();
	dsp4_vars.view_yofsenv = DSP4_READ_WORD();

	// initial (x,y,offset) at starting dsp4_vars.raster line
	dsp4_vars.view_x1 = (INT16)((dsp4_vars.world_x + dsp4_vars.world_xenv) >> 16);
	dsp4_vars.view_y1 = (INT16)(dsp4_vars.world_y >> 16);
	dsp4_vars.view_xofs1 = (INT16)(dsp4_vars.world_x >> 16);
	dsp4_vars.view_yofs1 = dsp4_vars.world_yofs;

	// first dsp4_vars.raster line
	dsp4_vars.poly_raster[0][0] = dsp4_vars.poly_bottom[0][0];


	do
	{
		////////////////////////////////////////////////////
		// process one iteration of projection

		// perspective projection of world (x,y,scroll) points
		// based on the current projection lines
		dsp4_vars.view_x2 = (INT16)(( ( ( dsp4_vars.world_x + dsp4_vars.world_xenv ) >> 16 ) * dsp4_vars.distance >> 15 ) + ( dsp4_vars.view_turnoff_x * dsp4_vars.distance >> 15 ));
		dsp4_vars.view_y2 = (INT16)((dsp4_vars.world_y >> 16) * dsp4_vars.distance >> 15);
		dsp4_vars.view_xofs2 = dsp4_vars.view_x2;
		dsp4_vars.view_yofs2 = (dsp4_vars.world_yofs * dsp4_vars.distance >> 15) + dsp4_vars.poly_bottom[0][0] - dsp4_vars.view_y2;

		// 1. World x-location before transformation
		// 2. Viewer x-position at the current
		// 3. World y-location before perspective projection
		// 4. Viewer y-position below the horizon
		// 5. Number of dsp4_vars.raster lines drawn in this iteration

		DSP4_CLEAR_OUT();
		DSP4_WRITE_WORD((UINT16)((dsp4_vars.world_x + dsp4_vars.world_xenv) >> 16));
		DSP4_WRITE_WORD(dsp4_vars.view_x2);
		DSP4_WRITE_WORD((UINT16)(dsp4_vars.world_y >> 16));
		DSP4_WRITE_WORD(dsp4_vars.view_y2);

		//////////////////////////////////////////////////////////

		// SR = 0x00

		// determine # of dsp4_vars.raster lines used
		dsp4_vars.segments = dsp4_vars.view_y1 - dsp4_vars.view_y2;

		// prevent overdraw
		if (dsp4_vars.view_y2 >= dsp4_vars.poly_raster[0][0])
			dsp4_vars.segments = 0;
		else
			dsp4_vars.poly_raster[0][0] = dsp4_vars.view_y2;

		// don't draw outside the window
		if (dsp4_vars.view_y2 < dsp4_vars.poly_top[0][0])
		{
			dsp4_vars.segments = 0;

			// flush remaining dsp4_vars.raster lines
			if (dsp4_vars.view_y1 >= dsp4_vars.poly_top[0][0])
				dsp4_vars.segments = dsp4_vars.view_y1 - dsp4_vars.poly_top[0][0];
		}

		// SR = 0x80

		DSP4_WRITE_WORD(dsp4_vars.segments);

		//////////////////////////////////////////////////////////

		// scan next command if no SR check needed
		if (dsp4_vars.segments)
		{
			INT32 px_dx, py_dy;
			INT32 x_scroll, y_scroll;

			// SR = 0x00

			// linear interpolation (lerp) between projected points
			px_dx = (dsp4_vars.view_xofs2 - dsp4_vars.view_xofs1) * dsp4_Inverse(dsp4_vars.segments) << 1;
			py_dy = (dsp4_vars.view_yofs2 - dsp4_vars.view_yofs1) * dsp4_Inverse(dsp4_vars.segments) << 1;

			// starting step values
			x_scroll = SEX16(dsp4_vars.poly_cx[0][0] + dsp4_vars.view_xofs1);
			y_scroll = SEX16(-dsp4_vars.viewport_bottom + dsp4_vars.view_yofs1 + dsp4_vars.view_yofsenv + dsp4_vars.poly_cx[1][0] - dsp4_vars.world_yofs);

			// SR = 0x80

			// rasterize line
			for (dsp4_vars.lcv = 0; dsp4_vars.lcv < dsp4_vars.segments; dsp4_vars.lcv++)
			{
				// 1. HDMA memory pointer (bg1)
				// 2. vertical scroll offset ($210E)
				// 3. horizontal scroll offset ($210D)

				DSP4_WRITE_WORD(dsp4_vars.poly_ptr[0][0]);
				DSP4_WRITE_WORD((UINT16)((y_scroll + 0x8000) >> 16));
				DSP4_WRITE_WORD((UINT16)((x_scroll + 0x8000) >> 16));


				// update memory address
				dsp4_vars.poly_ptr[0][0] -= 4;

				// update screen values
				x_scroll += px_dx;
				y_scroll += py_dy;
			}
		}

		/////////////////////////////////////////////////////
		// Post-update

		// update new viewer (x,y,scroll) to last dsp4_vars.raster line drawn
		dsp4_vars.view_x1 = dsp4_vars.view_x2;
		dsp4_vars.view_y1 = dsp4_vars.view_y2;
		dsp4_vars.view_xofs1 = dsp4_vars.view_xofs2;
		dsp4_vars.view_yofs1 = dsp4_vars.view_yofs2;

		// add deltas for projection lines
		dsp4_vars.world_dx += SEX78(dsp4_vars.world_ddx);
		dsp4_vars.world_dy += SEX78(dsp4_vars.world_ddy);

		// update projection lines
		dsp4_vars.world_x += (dsp4_vars.world_dx + dsp4_vars.world_xenv);
		dsp4_vars.world_y += dsp4_vars.world_dy;

		////////////////////////////////////////////////////
		// command check

		// scan next command
		dsp4.in_count = 2;
		DSP4_WAIT(1) resume1 :

		// inspect input
		dsp4_vars.distance = DSP4_READ_WORD();

		// terminate op
		if (dsp4_vars.distance == -0x8000)
			break;

		// already have 2 bytes in queue
		dsp4.in_count = 6;
		DSP4_WAIT(2) resume2:

		// inspect inputs
		dsp4_vars.world_ddy = DSP4_READ_WORD();
		dsp4_vars.world_ddx = DSP4_READ_WORD();
		dsp4_vars.view_yofsenv = DSP4_READ_WORD();

		// no envelope here
		dsp4_vars.world_xenv = 0;
	}
	while (1);

	dsp4.waiting4command = 1;
}

//////////////////////////////////////////////////////////////


static void dsp4_OP0E( void )
{
	dsp4_vars.OAM_RowMax = 16;
	memset(dsp4_vars.OAM_Row, 0, 64);
}


//////////////////////////////////////////////////////////////

static void dsp4_OP0F( void )
{
	dsp4.waiting4command = 0;

	// op flow control
	switch (dsp4_vars.Logic)
	{
		case 1:
			goto resume1; break;
		case 2:
			goto resume2; break;
		case 3:
			goto resume3; break;
		case 4:
			goto resume4; break;
	}

	////////////////////////////////////////////////////
	// process initial inputs

	// sort inputs
	DSP4_READ_WORD(); // 0x0000
	dsp4_vars.world_y = DSP4_READ_DWORD();
	dsp4_vars.poly_bottom[0][0] = DSP4_READ_WORD();
	dsp4_vars.poly_top[0][0] = DSP4_READ_WORD();
	dsp4_vars.poly_cx[1][0] = DSP4_READ_WORD();
	dsp4_vars.viewport_bottom = DSP4_READ_WORD();
	dsp4_vars.world_x = DSP4_READ_DWORD();
	dsp4_vars.poly_cx[0][0] = DSP4_READ_WORD();
	dsp4_vars.poly_ptr[0][0] = DSP4_READ_WORD();
	dsp4_vars.world_yofs = DSP4_READ_WORD();
	dsp4_vars.world_dy = DSP4_READ_DWORD();
	dsp4_vars.world_dx = DSP4_READ_DWORD();
	dsp4_vars.distance = DSP4_READ_WORD();
	DSP4_READ_WORD(); // 0x0000
	dsp4_vars.world_xenv = DSP4_READ_DWORD();
	dsp4_vars.world_ddy = DSP4_READ_WORD();
	dsp4_vars.world_ddx = DSP4_READ_WORD();
	dsp4_vars.view_yofsenv = DSP4_READ_WORD();

	// initial (x,y,offset) at starting dsp4_vars.raster line
	dsp4_vars.view_x1 = (INT16)((dsp4_vars.world_x + dsp4_vars.world_xenv) >> 16);
	dsp4_vars.view_y1 = (INT16)(dsp4_vars.world_y >> 16);
	dsp4_vars.view_xofs1 = (INT16)(dsp4_vars.world_x >> 16);
	dsp4_vars.view_yofs1 = dsp4_vars.world_yofs;
	dsp4_vars.view_turnoff_x = 0;
	dsp4_vars.view_turnoff_dx = 0;

	// first dsp4_vars.raster line
	dsp4_vars.poly_raster[0][0] = dsp4_vars.poly_bottom[0][0];


	do
	{
		////////////////////////////////////////////////////
		// process one iteration of projection

		// perspective projection of world (x,y,scroll) points
		// based on the current projection lines
		dsp4_vars.view_x2 = (INT16)(((dsp4_vars.world_x + dsp4_vars.world_xenv) >> 16) * dsp4_vars.distance >> 15);
		dsp4_vars.view_y2 = (INT16)((dsp4_vars.world_y >> 16) * dsp4_vars.distance >> 15);
		dsp4_vars.view_xofs2 = dsp4_vars.view_x2;
		dsp4_vars.view_yofs2 = (dsp4_vars.world_yofs * dsp4_vars.distance >> 15) + dsp4_vars.poly_bottom[0][0] - dsp4_vars.view_y2;

		// 1. World x-location before transformation
		// 2. Viewer x-position at the next
		// 3. World y-location before perspective projection
		// 4. Viewer y-position below the horizon
		// 5. Number of dsp4_vars.raster lines drawn in this iteration

		DSP4_CLEAR_OUT();
		DSP4_WRITE_WORD((UINT16)((dsp4_vars.world_x + dsp4_vars.world_xenv) >> 16));
		DSP4_WRITE_WORD(dsp4_vars.view_x2);
		DSP4_WRITE_WORD((UINT16)(dsp4_vars.world_y >> 16));
		DSP4_WRITE_WORD(dsp4_vars.view_y2);

		//////////////////////////////////////////////////////

		// SR = 0x00

		// determine # of dsp4_vars.raster lines used
		dsp4_vars.segments = dsp4_vars.poly_raster[0][0] - dsp4_vars.view_y2;

		// prevent overdraw
		if (dsp4_vars.view_y2 >= dsp4_vars.poly_raster[0][0])
			dsp4_vars.segments = 0;
		else
			dsp4_vars.poly_raster[0][0] = dsp4_vars.view_y2;

		// don't draw outside the window
		if (dsp4_vars.view_y2 < dsp4_vars.poly_top[0][0])
		{
			dsp4_vars.segments = 0;

			// flush remaining dsp4_vars.raster lines
			if (dsp4_vars.view_y1 >= dsp4_vars.poly_top[0][0])
				dsp4_vars.segments = dsp4_vars.view_y1 - dsp4_vars.poly_top[0][0];
		}

		// SR = 0x80

		DSP4_WRITE_WORD(dsp4_vars.segments);

		//////////////////////////////////////////////////////

		// scan next command if no SR check needed
		if (dsp4_vars.segments)
		{
			INT32 px_dx, py_dy;
			INT32 x_scroll, y_scroll;

			for (dsp4_vars.lcv = 0; dsp4_vars.lcv < 4; dsp4_vars.lcv++)
			{
				// grab inputs
				dsp4.in_count = 4;
				DSP4_WAIT(1);
				resume1 :
				for (;;)
				{
					INT16 distance;
					INT16 color, red, green, blue;

					distance = DSP4_READ_WORD();
					color = DSP4_READ_WORD();

					// U1+B5+G5+R5
					red = color & 0x1f;
					green = (color >> 5) & 0x1f;
					blue = (color >> 10) & 0x1f;

					// dynamic lighting
					red = (red * distance >> 15) & 0x1f;
					green = (green * distance >> 15) & 0x1f;
					blue = (blue * distance >> 15) & 0x1f;
					color = red | (green << 5) | (blue << 10);

					DSP4_CLEAR_OUT();
					DSP4_WRITE_WORD(color);
					break;
				}
			}

			//////////////////////////////////////////////////////

			// SR = 0x00

			// linear interpolation (lerp) between projected points
			px_dx = (dsp4_vars.view_xofs2 - dsp4_vars.view_xofs1) * dsp4_Inverse(dsp4_vars.segments) << 1;
			py_dy = (dsp4_vars.view_yofs2 - dsp4_vars.view_yofs1) * dsp4_Inverse(dsp4_vars.segments) << 1;


			// starting step values
			x_scroll = SEX16(dsp4_vars.poly_cx[0][0] + dsp4_vars.view_xofs1);
			y_scroll = SEX16(-dsp4_vars.viewport_bottom + dsp4_vars.view_yofs1 + dsp4_vars.view_yofsenv + dsp4_vars.poly_cx[1][0] - dsp4_vars.world_yofs);

			// SR = 0x80

			// rasterize line
			for (dsp4_vars.lcv = 0; dsp4_vars.lcv < dsp4_vars.segments; dsp4_vars.lcv++)
			{
				// 1. HDMA memory pointer
				// 2. vertical scroll offset ($210E)
				// 3. horizontal scroll offset ($210D)

				DSP4_WRITE_WORD(dsp4_vars.poly_ptr[0][0]);
				DSP4_WRITE_WORD((UINT16)((y_scroll + 0x8000) >> 16));
				DSP4_WRITE_WORD((UINT16)((x_scroll + 0x8000) >> 16));

				// update memory address
				dsp4_vars.poly_ptr[0][0] -= 4;

				// update screen values
				x_scroll += px_dx;
				y_scroll += py_dy;
			}
		}

		////////////////////////////////////////////////////
		// Post-update

		// update new viewer (x,y,scroll) to last dsp4_vars.raster line drawn
		dsp4_vars.view_x1 = dsp4_vars.view_x2;
		dsp4_vars.view_y1 = dsp4_vars.view_y2;
		dsp4_vars.view_xofs1 = dsp4_vars.view_xofs2;
		dsp4_vars.view_yofs1 = dsp4_vars.view_yofs2;

		// add deltas for projection lines
		dsp4_vars.world_dx += SEX78(dsp4_vars.world_ddx);
		dsp4_vars.world_dy += SEX78(dsp4_vars.world_ddy);

		// update projection lines
		dsp4_vars.world_x += (dsp4_vars.world_dx + dsp4_vars.world_xenv);
		dsp4_vars.world_y += dsp4_vars.world_dy;

		// update road turnoff position
		dsp4_vars.view_turnoff_x += dsp4_vars.view_turnoff_dx;

		////////////////////////////////////////////////////
		// command check

		// scan next command
		dsp4.in_count = 2;
		DSP4_WAIT(2) resume2:

		// check for termination
		dsp4_vars.distance = DSP4_READ_WORD();
		if (dsp4_vars.distance == -0x8000)
			break;

		// road splice
		if( (UINT16) dsp4_vars.distance == 0x8001 )
		{
			dsp4.in_count = 6;
			DSP4_WAIT(3) resume3:

			dsp4_vars.distance = DSP4_READ_WORD();
			dsp4_vars.view_turnoff_x = DSP4_READ_WORD();
			dsp4_vars.view_turnoff_dx = DSP4_READ_WORD();

			// factor in new changes
			dsp4_vars.view_x1 += ( dsp4_vars.view_turnoff_x * dsp4_vars.distance >> 15 );
			dsp4_vars.view_xofs1 += ( dsp4_vars.view_turnoff_x * dsp4_vars.distance >> 15 );

			// update stepping values
			dsp4_vars.view_turnoff_x += dsp4_vars.view_turnoff_dx;

			dsp4.in_count = 2;
			DSP4_WAIT(2)
		}

		// already have 2 bytes in queue
		dsp4.in_count = 6;
		DSP4_WAIT(4) resume4 :

		// inspect inputs
		dsp4_vars.world_ddy = DSP4_READ_WORD();
		dsp4_vars.world_ddx = DSP4_READ_WORD();
		dsp4_vars.view_yofsenv = DSP4_READ_WORD();

		// no envelope here
		dsp4_vars.world_xenv = 0;
	}
	while (1);

	// terminate op
	dsp4.waiting4command = 1;
}

//////////////////////////////////////////////////////////////


static void dsp4_OP10( void )
{
	dsp4.waiting4command = 0;

	// op flow control
	switch (dsp4_vars.Logic)
	{
		case 1:
			goto resume1; break;
		case 2:
			goto resume2; break;
		case 3:
			goto resume3; break;
	}

	////////////////////////////////////////////////////
	// sort inputs

	DSP4_READ_WORD(); // 0x0000
	dsp4_vars.world_y = DSP4_READ_DWORD();
	dsp4_vars.poly_bottom[0][0] = DSP4_READ_WORD();
	dsp4_vars.poly_top[0][0] = DSP4_READ_WORD();
	dsp4_vars.poly_cx[1][0] = DSP4_READ_WORD();
	dsp4_vars.viewport_bottom = DSP4_READ_WORD();
	dsp4_vars.world_x = DSP4_READ_DWORD();
	dsp4_vars.poly_cx[0][0] = DSP4_READ_WORD();
	dsp4_vars.poly_ptr[0][0] = DSP4_READ_WORD();
	dsp4_vars.world_yofs = DSP4_READ_WORD();
	dsp4_vars.distance = DSP4_READ_WORD();
	dsp4_vars.view_y2 = DSP4_READ_WORD();
	dsp4_vars.view_dy = DSP4_READ_WORD() * dsp4_vars.distance >> 15;
	dsp4_vars.view_x2 = DSP4_READ_WORD();
	dsp4_vars.view_dx = DSP4_READ_WORD() * dsp4_vars.distance >> 15;
	dsp4_vars.view_yofsenv = DSP4_READ_WORD();

	// initial (x,y,offset) at starting dsp4_vars.raster line
	dsp4_vars.view_x1 = (INT16)(dsp4_vars.world_x >> 16);
	dsp4_vars.view_y1 = (INT16)(dsp4_vars.world_y >> 16);
	dsp4_vars.view_xofs1 = dsp4_vars.view_x1;
	dsp4_vars.view_yofs1 = dsp4_vars.world_yofs;

	// first dsp4_vars.raster line
	dsp4_vars.poly_raster[0][0] = dsp4_vars.poly_bottom[0][0];

	do
	{
		////////////////////////////////////////////////////
		// process one iteration of projection

		// add shaping
		dsp4_vars.view_x2 += dsp4_vars.view_dx;
		dsp4_vars.view_y2 += dsp4_vars.view_dy;

		// vertical scroll calculation
		dsp4_vars.view_xofs2 = dsp4_vars.view_x2;
		dsp4_vars.view_yofs2 = (dsp4_vars.world_yofs * dsp4_vars.distance >> 15) + dsp4_vars.poly_bottom[0][0] - dsp4_vars.view_y2;

		// 1. Viewer x-position at the next
		// 2. Viewer y-position below the horizon
		// 3. Number of dsp4_vars.raster lines drawn in this iteration

		DSP4_CLEAR_OUT();
		DSP4_WRITE_WORD(dsp4_vars.view_x2);
		DSP4_WRITE_WORD(dsp4_vars.view_y2);

		//////////////////////////////////////////////////////

		// SR = 0x00

		// determine # of dsp4_vars.raster lines used
		dsp4_vars.segments = dsp4_vars.view_y1 - dsp4_vars.view_y2;

		// prevent overdraw
		if (dsp4_vars.view_y2 >= dsp4_vars.poly_raster[0][0])
			dsp4_vars.segments = 0;
		else
			dsp4_vars.poly_raster[0][0] = dsp4_vars.view_y2;

		// don't draw outside the window
		if (dsp4_vars.view_y2 < dsp4_vars.poly_top[0][0])
		{
			dsp4_vars.segments = 0;

			// flush remaining dsp4_vars.raster lines
			if (dsp4_vars.view_y1 >= dsp4_vars.poly_top[0][0])
				dsp4_vars.segments = dsp4_vars.view_y1 - dsp4_vars.poly_top[0][0];
		}

		// SR = 0x80

		DSP4_WRITE_WORD(dsp4_vars.segments);

		//////////////////////////////////////////////////////

		// scan next command if no SR check needed
		if (dsp4_vars.segments)
		{
			for (dsp4_vars.lcv = 0; dsp4_vars.lcv < 4; dsp4_vars.lcv++)
			{
				// grab inputs
				dsp4.in_count = 4;
				DSP4_WAIT(1);
				resume1 :
				for (;;)
				{
					INT16 distance;
					INT16 color, red, green, blue;

					distance = DSP4_READ_WORD();
					color = DSP4_READ_WORD();

					// U1+B5+G5+R5
					red = color & 0x1f;
					green = (color >> 5) & 0x1f;
					blue = (color >> 10) & 0x1f;

					// dynamic lighting
					red = (red * distance >> 15) & 0x1f;
					green = (green * distance >> 15) & 0x1f;
					blue = (blue * distance >> 15) & 0x1f;
					color = red | (green << 5) | (blue << 10);

					DSP4_CLEAR_OUT();
					DSP4_WRITE_WORD(color);
					break;
				}
			}
		}

		//////////////////////////////////////////////////////

		// scan next command if no SR check needed
		if (dsp4_vars.segments)
		{
			INT32 px_dx, py_dy;
			INT32 x_scroll, y_scroll;

			// SR = 0x00

			// linear interpolation (lerp) between projected points
			px_dx = (dsp4_vars.view_xofs2 - dsp4_vars.view_xofs1) * dsp4_Inverse(dsp4_vars.segments) << 1;
			py_dy = (dsp4_vars.view_yofs2 - dsp4_vars.view_yofs1) * dsp4_Inverse(dsp4_vars.segments) << 1;

			// starting step values
			x_scroll = SEX16(dsp4_vars.poly_cx[0][0] + dsp4_vars.view_xofs1);
			y_scroll = SEX16(-dsp4_vars.viewport_bottom + dsp4_vars.view_yofs1 + dsp4_vars.view_yofsenv + dsp4_vars.poly_cx[1][0] - dsp4_vars.world_yofs);

			// SR = 0x80

			// rasterize line
			for (dsp4_vars.lcv = 0; dsp4_vars.lcv < dsp4_vars.segments; dsp4_vars.lcv++)
			{
				// 1. HDMA memory pointer (bg2)
				// 2. vertical scroll offset ($2110)
				// 3. horizontal scroll offset ($210F)

				DSP4_WRITE_WORD(dsp4_vars.poly_ptr[0][0]);
				DSP4_WRITE_WORD((UINT16)((y_scroll + 0x8000) >> 16));
				DSP4_WRITE_WORD((UINT16)((x_scroll + 0x8000) >> 16));

				// update memory address
				dsp4_vars.poly_ptr[0][0] -= 4;

				// update screen values
				x_scroll += px_dx;
				y_scroll += py_dy;
			}
		}

		/////////////////////////////////////////////////////
		// Post-update

		// update new viewer (x,y,scroll) to last dsp4_vars.raster line drawn
		dsp4_vars.view_x1 = dsp4_vars.view_x2;
		dsp4_vars.view_y1 = dsp4_vars.view_y2;
		dsp4_vars.view_xofs1 = dsp4_vars.view_xofs2;
		dsp4_vars.view_yofs1 = dsp4_vars.view_yofs2;

		////////////////////////////////////////////////////
		// command check

		// scan next command
		dsp4.in_count = 2;
		DSP4_WAIT(2) resume2 :

		// check for opcode termination
		dsp4_vars.distance = DSP4_READ_WORD();
		if (dsp4_vars.distance == -0x8000)
			break;

		// already have 2 bytes in queue
		dsp4.in_count = 10;
		DSP4_WAIT(3) resume3 :


		// inspect inputs
		dsp4_vars.view_y2 = DSP4_READ_WORD();
		dsp4_vars.view_dy = DSP4_READ_WORD() * dsp4_vars.distance >> 15;
		dsp4_vars.view_x2 = DSP4_READ_WORD();
		dsp4_vars.view_dx = DSP4_READ_WORD() * dsp4_vars.distance >> 15;
	}
	while (1);

	dsp4.waiting4command = 1;
}

//////////////////////////////////////////////////////////////

static void dsp4_OP11(INT16 A, INT16 B, INT16 C, INT16 D, INT16 *M)
{
	// 0x155 = 341 = Horizontal Width of the Screen
	*M = ((A * 0x0155 >> 2) & 0xf000) |
			 ((B * 0x0155 >> 6) & 0x0f00) |
			 ((C * 0x0155 >> 10) & 0x00f0) |
			 ((D * 0x0155 >> 14) & 0x000f);
}





/////////////////////////////////////////////////////////////
//Processing Code
/////////////////////////////////////////////////////////////

static void dsp4_register_save( running_machine *machine )
{
	state_save_register_global(machine, dsp4.waiting4command);
	state_save_register_global(machine, dsp4.half_command);
	state_save_register_global(machine, dsp4.command);
	state_save_register_global(machine, dsp4.in_count);
	state_save_register_global(machine, dsp4.in_index);
	state_save_register_global(machine, dsp4.out_count);
	state_save_register_global(machine, dsp4.out_index);
	state_save_register_global_array(machine, dsp4.parameters);
	state_save_register_global_array(machine, dsp4.output);

	state_save_register_global(machine, dsp4_vars.Logic);
	state_save_register_global(machine, dsp4_vars.lcv);
	state_save_register_global(machine, dsp4_vars.distance);
	state_save_register_global(machine, dsp4_vars.raster);
	state_save_register_global(machine, dsp4_vars.segments);

	state_save_register_global(machine, dsp4_vars.world_x);
	state_save_register_global(machine, dsp4_vars.world_y);
	state_save_register_global(machine, dsp4_vars.world_dx);
	state_save_register_global(machine, dsp4_vars.world_dy);
	state_save_register_global(machine, dsp4_vars.world_ddx);
	state_save_register_global(machine, dsp4_vars.world_ddy);
	state_save_register_global(machine, dsp4_vars.world_xenv);
	state_save_register_global(machine, dsp4_vars.world_yofs);

	state_save_register_global(machine, dsp4_vars.view_x1);
	state_save_register_global(machine, dsp4_vars.view_y1);
	state_save_register_global(machine, dsp4_vars.view_x2);
	state_save_register_global(machine, dsp4_vars.view_y2);
	state_save_register_global(machine, dsp4_vars.view_dx);
	state_save_register_global(machine, dsp4_vars.view_dy);
	state_save_register_global(machine, dsp4_vars.view_xofs1);
	state_save_register_global(machine, dsp4_vars.view_yofs1);
	state_save_register_global(machine, dsp4_vars.view_xofs2);
	state_save_register_global(machine, dsp4_vars.view_yofs2);
	state_save_register_global(machine, dsp4_vars.view_yofsenv);
	state_save_register_global(machine, dsp4_vars.view_turnoff_x);
	state_save_register_global(machine, dsp4_vars.view_turnoff_dx);


	state_save_register_global(machine, dsp4_vars.viewport_cx);
	state_save_register_global(machine, dsp4_vars.viewport_cy);
	state_save_register_global(machine, dsp4_vars.viewport_left);
	state_save_register_global(machine, dsp4_vars.viewport_right);
	state_save_register_global(machine, dsp4_vars.viewport_top);
	state_save_register_global(machine, dsp4_vars.viewport_bottom);


	state_save_register_global(machine, dsp4_vars.sprite_x);
	state_save_register_global(machine, dsp4_vars.sprite_y);
	state_save_register_global(machine, dsp4_vars.sprite_attr);
	state_save_register_global(machine, dsp4_vars.sprite_size);
	state_save_register_global(machine, dsp4_vars.sprite_clipy);
	state_save_register_global(machine, dsp4_vars.sprite_count);

	state_save_register_global_array(machine, dsp4_vars.poly_clipLf[0]);
	state_save_register_global_array(machine, dsp4_vars.poly_clipLf[1]);
	state_save_register_global_array(machine, dsp4_vars.poly_clipRt[0]);
	state_save_register_global_array(machine, dsp4_vars.poly_clipRt[1]);
	state_save_register_global_array(machine, dsp4_vars.poly_ptr[0]);
	state_save_register_global_array(machine, dsp4_vars.poly_ptr[1]);
	state_save_register_global_array(machine, dsp4_vars.poly_raster[0]);
	state_save_register_global_array(machine, dsp4_vars.poly_raster[1]);
	state_save_register_global_array(machine, dsp4_vars.poly_top[0]);
	state_save_register_global_array(machine, dsp4_vars.poly_top[1]);
	state_save_register_global_array(machine, dsp4_vars.poly_bottom[0]);
	state_save_register_global_array(machine, dsp4_vars.poly_bottom[1]);
	state_save_register_global_array(machine, dsp4_vars.poly_cx[0]);
	state_save_register_global_array(machine, dsp4_vars.poly_cx[1]);
	state_save_register_global_array(machine, dsp4_vars.poly_start);
	state_save_register_global_array(machine, dsp4_vars.poly_plane);

	state_save_register_global_array(machine, dsp4_vars.OAM_attr);
	state_save_register_global(machine, dsp4_vars.OAM_index);
	state_save_register_global(machine, dsp4_vars.OAM_bits);

	state_save_register_global(machine, dsp4_vars.OAM_RowMax);
	state_save_register_global_array(machine, dsp4_vars.OAM_Row);
}

static void dsp4_init( running_machine *machine )
{
	memset(&dsp4, 0, sizeof(dsp4));
	dsp4.waiting4command = 1;

	dsp4_register_save(machine);
}

static void dsp4_write( UINT8 dsp4_byte )
{
	// clear pending read
	if (dsp4.out_index < dsp4.out_count)
	{
		dsp4.out_index++;
		return;
	}

	if (dsp4.waiting4command)
	{
		if (dsp4.half_command)
		{
			dsp4.command |= (dsp4_byte << 8);
			dsp4.in_index = 0;
			dsp4.waiting4command = 0;
			dsp4.half_command = 0;
			dsp4.out_count = 0;
			dsp4.out_index = 0;

			dsp4_vars.Logic = 0;


			switch (dsp4.command)
			{
				case 0x0000:
					dsp4.in_count = 4; break;
				case 0x0001:
					dsp4.in_count = 44; break;
				case 0x0003:
					dsp4.in_count = 0; break;
				case 0x0005:
					dsp4.in_count = 0; break;
				case 0x0006:
					dsp4.in_count = 0; break;
				case 0x0007:
					dsp4.in_count = 34; break;
				case 0x0008:
					dsp4.in_count = 90; break;
				case 0x0009:
					dsp4.in_count = 14; break;
				case 0x000a:
					dsp4.in_count = 6; break;
				case 0x000b:
					dsp4.in_count = 6; break;
				case 0x000d:
					dsp4.in_count = 42; break;
				case 0x000e:
					dsp4.in_count = 0; break;
				case 0x000f:
					dsp4.in_count = 46; break;
				case 0x0010:
					dsp4.in_count = 36; break;
				case 0x0011:
					dsp4.in_count = 8; break;
				default:
					dsp4.waiting4command = 1;
					break;
			}
		}
		else
		{
			dsp4.command = dsp4_byte;
			dsp4.half_command = 1;
		}
	}
	else
	{
		dsp4.parameters[dsp4.in_index] = dsp4_byte;
		dsp4.in_index++;
	}

	if (!dsp4.waiting4command && dsp4.in_count == dsp4.in_index)
	{
		// Actually execute the command
		dsp4.waiting4command = 1;
		dsp4.out_index = 0;
		dsp4.in_index = 0;

		switch (dsp4.command)
		{
				// 16-bit multiplication
			case 0x0000:
			{
				INT16 multiplier, multiplicand;
				INT32 product;

				multiplier = DSP4_READ_WORD();
				multiplicand = DSP4_READ_WORD();

				dsp4_Multiply(multiplicand, multiplier, &product);

				DSP4_CLEAR_OUT();
				DSP4_WRITE_WORD((UINT16)(product));
				DSP4_WRITE_WORD((UINT16)(product >> 16));
			}
			break;

			// single-player track projection
			case 0x0001:
				dsp4_OP01(); break;

			// single-player selection
			case 0x0003:
				dsp4_OP03(); break;

			// clear OAM
			case 0x0005:
				dsp4_OP05(); break;

			// transfer OAM
			case 0x0006:
				dsp4_OP06(); break;

			// single-player track turnoff projection
			case 0x0007:
				dsp4_OP07(); break;

			// solid polygon projection
			case 0x0008:
				dsp4_OP08(); break;

			// sprite projection
			case 0x0009:
				dsp4_OP09(); break;

			// unknown
			case 0x000A:
			{
				INT16 in2a;
				INT16 out1a, out2a, out3a, out4a;

				/* in1a = */ DSP4_READ_WORD();
				in2a = DSP4_READ_WORD();
				/* in3a = */ DSP4_READ_WORD();

				dsp4_OP0A(in2a, &out2a, &out1a, &out4a, &out3a);

				DSP4_CLEAR_OUT();
				DSP4_WRITE_WORD(out1a);
				DSP4_WRITE_WORD(out2a);
				DSP4_WRITE_WORD(out3a);
				DSP4_WRITE_WORD(out4a);
			}
			break;

			// set OAM
			case 0x000B:
			{
				INT16 sp_x = DSP4_READ_WORD();
				INT16 sp_y = DSP4_READ_WORD();
				INT16 sp_attr = DSP4_READ_WORD();
				UINT8 draw = 1;

				DSP4_CLEAR_OUT();

				dsp4_OP0B(&draw, sp_x, sp_y, sp_attr, 0, 1);
			}
			break;

			// multi-player track projection
			case 0x000D:
				dsp4_OP0D(); break;

			// multi-player selection
			case 0x000E:
				dsp4_OP0E(); break;

			// single-player track projection with lighting
			case 0x000F:
				dsp4_OP0F(); break;

			// single-player track turnoff projection with lighting
			case 0x0010:
				dsp4_OP10(); break;

			// unknown: horizontal mapping command
			case 0x0011:
			{
				INT16 a, b, c, d, m;


				d = DSP4_READ_WORD();
				c = DSP4_READ_WORD();
				b = DSP4_READ_WORD();
				a = DSP4_READ_WORD();

				dsp4_OP11(a, b, c, d, &m);

				DSP4_CLEAR_OUT();
				DSP4_WRITE_WORD(m);

				break;
			}

			default:
				break;
		}
	}
}

static UINT8 dsp4_read( void )
{
	UINT8 value;

	if (dsp4.out_count)
	{
		value = (UINT8) dsp4.output[dsp4.out_index&0x1FF];
		dsp4.out_index++;
		if (dsp4.out_count == dsp4.out_index)
			dsp4.out_count = 0;
	}
	else
	{
		value = 0xff;
	}

	return value;
}
