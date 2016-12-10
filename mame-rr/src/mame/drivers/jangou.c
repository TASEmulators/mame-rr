/*******************************************************************************************

Jangou (c) 1983 Nichibutsu

driver by David Haywood, Angelo Salese and Phil Bennett

TODO:
-unemulated screen flipping;
-jngolady: RNG in this isn't working properly...looking at the code,when the mcu isn't on
 irq routine there's a poll to the [8] ram address,unsurprisingly it's the RNG seed.
 The problem is that when the rti opcode occurs the program flow doesn't return to feed the
 RNG but executes another irq,probably there are too many irq pollings and the mcu goes out
 of cycles...for now I'm kludging it,my guess is that it can be either a cpu bug,a comms bug
 or 8 is really connected to a RNG seed and the starting code is just for initializing it.
-dip-switches;

============================================================================================
Debug cheats:

*jangou
$c132 coin counter
$c088-$c095 player tiles

*******************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m6800/m6800.h"
#include "sound/ay8910.h"
#include "sound/hc55516.h"
#include "sound/msm5205.h"
#include "video/resnet.h"

#define MASTER_CLOCK	XTAL_19_968MHz

class jangou_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, jangou_state(machine)); }

	jangou_state(running_machine &machine) { }

	/* video-related */
	UINT8        *blit_buffer;
	UINT8        pen_data[0x10];
	UINT8        blit_data[6];

	/* sound-related */
	// Jangou CVSD Sound
	emu_timer    *cvsd_bit_timer;
	UINT8        cvsd_shiftreg;
	int          cvsd_shift_cnt;
	// Jangou Lady ADPCM Sound
	UINT8        adpcm_byte;
	int          msm5205_vclk_toggle;

	/* misc */
	UINT8        mux_data;
	UINT8        nsc_latch, z80_latch;

	/* devices */
	running_device *cpu_0;
	running_device *cpu_1;
	running_device *cvsd;
	running_device *nsc;
};


/*************************************
 *
 *  Video Hardware
 *
 *************************************/

/* guess: use the same resistor values as Crazy Climber (needs checking on the real HW) */
static PALETTE_INIT( jangou )
{
	static const int resistances_rg[3] = { 1000, 470, 220 };
	static const int resistances_b [2] = { 470, 220 };
	double weights_rg[3], weights_b[2];
	int i;

	/* compute the color output resistor weights */
	compute_resistor_weights(0, 255, -1.0,
			3, resistances_rg, weights_rg, 0, 0,
			2, resistances_b,  weights_b,  0, 0,
			0, 0, 0, 0, 0);

	for (i = 0;i < machine->total_colors(); i++)
	{
		int bit0, bit1, bit2;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = combine_3_weights(weights_rg, bit0, bit1, bit2);

		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = combine_3_weights(weights_rg, bit0, bit1, bit2);

		/* blue component */
		bit0 = (color_prom[i] >> 6) & 0x01;
		bit1 = (color_prom[i] >> 7) & 0x01;
		b = combine_2_weights(weights_b, bit0, bit1);

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
	}
}

static VIDEO_START( jangou )
{
	jangou_state *state = (jangou_state *)machine->driver_data;

	state->blit_buffer = auto_alloc_array(machine, UINT8, 256 * 256);
	state_save_register_global_pointer(machine, state->blit_buffer, 256 * 256);
}

static VIDEO_UPDATE( jangou )
{
	jangou_state *state = (jangou_state *)screen->machine->driver_data;
	int x, y;

	for (y = cliprect->min_y; y <= cliprect->max_y; ++y)
	{
		UINT8 *src = &state->blit_buffer[y * 512 / 2 + cliprect->min_x];
		UINT16 *dst = BITMAP_ADDR16(bitmap, y, cliprect->min_x);

		for (x = cliprect->min_x; x <= cliprect->max_x; x += 2)
		{
			UINT32 srcpix = *src++;
			*dst++ = screen->machine->pens[srcpix & 0xf];
			*dst++ = screen->machine->pens[(srcpix >> 4) & 0xf];
		}
	}

	return 0;
}

/*
Blitter Memory Map:

src lo word[$12]
src hi word[$13]
x [$14]
y [$15]
h [$16]
w [$17]
*/

static UINT8 jangou_gfx_nibble( running_machine *machine, UINT16 niboffset )
{
	const UINT8 *const blit_rom = memory_region(machine, "gfx");

	if (niboffset & 1)
		return (blit_rom[(niboffset >> 1) & 0xffff] & 0xf0) >> 4;
	else
		return (blit_rom[(niboffset >> 1) & 0xffff] & 0x0f);
}

static void plot_jangou_gfx_pixel( running_machine *machine, UINT8 pix, int x, int y )
{
	jangou_state *state = (jangou_state *)machine->driver_data;
	if (y < 0 || y >= 512)
		return;
	if (x < 0 || x >= 512)
		return;

	if (x & 1)
		state->blit_buffer[(y * 256) + (x >> 1)] = (state->blit_buffer[(y * 256) + (x >> 1)] & 0x0f) | ((pix << 4) & 0xf0);
	else
		state->blit_buffer[(y * 256) + (x >> 1)] = (state->blit_buffer[(y * 256) + (x >> 1)] & 0xf0) | (pix & 0x0f);
}

static WRITE8_HANDLER( blitter_process_w )
{
	jangou_state *state = (jangou_state *)space->machine->driver_data;
	int src, x, y, h, w, flipx;
	state->blit_data[offset] = data;

	if (offset == 5)
	{
		int count = 0;
		int xcount, ycount;

		/* printf("%02x %02x %02x %02x %02x %02x\n", state->blit_data[0], state->blit_data[1], state->blit_data[2],
                    state->blit_data[3], state->blit_data[4], state->blit_data[5]); */
		w = (state->blit_data[4] & 0xff) + 1;
		h = (state->blit_data[5] & 0xff) + 1;
		src = ((state->blit_data[1] << 8)|(state->blit_data[0] << 0));
		x = (state->blit_data[2] & 0xff);
		y = (state->blit_data[3] & 0xff);

		// lowest bit of src controls flipping / draw direction?
		flipx = (state->blit_data[0] & 1);

		if (!flipx)
			src += (w * h) - 1;
		else
			src -= (w * h) - 1;

		for (ycount = 0; ycount < h; ycount++)
		{
			for(xcount = 0; xcount < w; xcount++)
			{
				int drawx = (x + xcount) & 0xff;
				int drawy = (y + ycount) & 0xff;
				UINT8 dat = jangou_gfx_nibble(space->machine, src + count);
				UINT8 cur_pen_hi = state->pen_data[(dat & 0xf0) >> 4];
				UINT8 cur_pen_lo = state->pen_data[(dat & 0x0f) >> 0];

				dat = cur_pen_lo | (cur_pen_hi << 4);

				if ((dat & 0xff) != 0)
					plot_jangou_gfx_pixel(space->machine, dat, drawx, drawy);

				if (!flipx)
					count--;
				else
					count++;
			}
		}
	}
}

/* What is the bit 5 (0x20) for?*/
static WRITE8_HANDLER( blit_vregs_w )
{
	jangou_state *state = (jangou_state *)space->machine->driver_data;

	//  printf("%02x %02x\n", offset, data);
	state->pen_data[offset] = data & 0xf;
}

/*************************************
 *
 *  I/O
 *
 *************************************/

static WRITE8_HANDLER( mux_w )
{
	jangou_state *state = (jangou_state *)space->machine->driver_data;
	state->mux_data = ~data;
}

static WRITE8_HANDLER( output_w )
{
	/*
    --x- ---- ? (polls between high and low in irq routine,probably signals the vblank routine)
    ---- -x-- flip screen
    ---- ---x coin counter
    */
//  printf("%02x\n", data);
	coin_counter_w(space->machine, 0, data & 0x01);
//  flip_screen_set(data & 0x04);
//  coin_lockout_w(space->machine, 0, ~data & 0x20);
}

static READ8_DEVICE_HANDLER( input_mux_r )
{
	jangou_state *state = (jangou_state *)device->machine->driver_data;
	switch(state->mux_data)
	{
		case 0x01: return input_port_read(device->machine, "PL1_1");
		case 0x02: return input_port_read(device->machine, "PL1_2");
		case 0x04: return input_port_read(device->machine, "PL2_1");
		case 0x08: return input_port_read(device->machine, "PL2_2");
		case 0x10: return input_port_read(device->machine, "PL1_3");
		case 0x20: return input_port_read(device->machine, "PL2_3");
	}

	return input_port_read(device->machine, "IN_NOMUX");
}

static READ8_DEVICE_HANDLER( input_system_r )
{
	return input_port_read(device->machine, "SYSTEM");
}


/*************************************
 *
 *  Sample Player CPU
 *
 *************************************/

static WRITE8_HANDLER( sound_latch_w )
{
	jangou_state *state = (jangou_state *)space->machine->driver_data;
	soundlatch_w(space, 0, data & 0xff);
	cpu_set_input_line(state->cpu_1, INPUT_LINE_NMI, ASSERT_LINE);
}

static READ8_HANDLER( sound_latch_r )
{
	jangou_state *state = (jangou_state *)space->machine->driver_data;
	cpu_set_input_line(state->cpu_1, INPUT_LINE_NMI, CLEAR_LINE);
	return soundlatch_r(space, 0);
}

/* Jangou HC-55516 CVSD */
static WRITE8_HANDLER( cvsd_w )
{
	jangou_state *state = (jangou_state *)space->machine->driver_data;
	state->cvsd_shiftreg = data;
}

static TIMER_CALLBACK( cvsd_bit_timer_callback )
{
	jangou_state *state = (jangou_state *)machine->driver_data;

	/* Data is shifted out at the MSB */
	hc55516_digit_w(state->cvsd, (state->cvsd_shiftreg >> 7) & 1);
	state->cvsd_shiftreg <<= 1;

	/* Trigger an IRQ for every 8 shifted bits */
	if ((++state->cvsd_shift_cnt & 7) == 0)
		cpu_set_input_line(state->cpu_1, 0, HOLD_LINE);
}


/* Jangou Lady MSM5218 (MSM5205-compatible) ADPCM */
static WRITE8_HANDLER( adpcm_w )
{
	jangou_state *state = (jangou_state *)space->machine->driver_data;
	state->adpcm_byte = data;
}

static void jngolady_vclk_cb( running_device *device )
{
	jangou_state *state = (jangou_state *)device->machine->driver_data;

	if (state->msm5205_vclk_toggle == 0)
		msm5205_data_w(device, state->adpcm_byte >> 4);
	else
	{
		msm5205_data_w(device, state->adpcm_byte & 0xf);
		cpu_set_input_line(state->cpu_1, 0, HOLD_LINE);
	}

	state->msm5205_vclk_toggle ^= 1;
}


/*************************************
 *
 *  Jangou Lady NSC8105 CPU
 *
 *************************************/

static READ8_HANDLER( master_com_r )
{
	jangou_state *state = (jangou_state *)space->machine->driver_data;
	return state->z80_latch;
}

static WRITE8_HANDLER( master_com_w )
{
	jangou_state *state = (jangou_state *)space->machine->driver_data;

	cpu_set_input_line(state->nsc, 0, HOLD_LINE);
	state->nsc_latch = data;
}

static READ8_HANDLER( slave_com_r )
{
	jangou_state *state = (jangou_state *)space->machine->driver_data;
	return state->nsc_latch;
}

static WRITE8_HANDLER( slave_com_w )
{
	jangou_state *state = (jangou_state *)space->machine->driver_data;
	state->z80_latch = data;
}

/*************************************
 *
 *  Jangou Memory Map
 *
 *************************************/

static ADDRESS_MAP_START( cpu0_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x9fff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( cpu0_io, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x01,0x01) AM_DEVREAD("aysnd", ay8910_r)
	AM_RANGE(0x02,0x03) AM_DEVWRITE("aysnd", ay8910_data_address_w)
	AM_RANGE(0x10,0x10) AM_READ_PORT("DSW") //dsw + blitter busy flag
	AM_RANGE(0x10,0x10) AM_WRITE(output_w)
	AM_RANGE(0x11,0x11) AM_WRITE(mux_w)
	AM_RANGE(0x12,0x17) AM_WRITE(blitter_process_w)
	AM_RANGE(0x20,0x2f) AM_WRITE(blit_vregs_w)
	AM_RANGE(0x30,0x30) AM_WRITENOP //? polls 0x03 continuously
	AM_RANGE(0x31,0x31) AM_WRITE(sound_latch_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( cpu1_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM AM_WRITENOP
ADDRESS_MAP_END

static ADDRESS_MAP_START( cpu1_io, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00,0x00) AM_READ(sound_latch_r)
	AM_RANGE(0x01,0x01) AM_WRITE(cvsd_w)
	AM_RANGE(0x02,0x02) AM_WRITENOP // Echoes sound command - acknowledge?
ADDRESS_MAP_END


/*************************************
 *
 *  Jangou Lady Memory Map
 *
 *************************************/

static ADDRESS_MAP_START( jngolady_cpu0_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x9fff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0xe000, 0xe000) AM_READWRITE(master_com_r,master_com_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( jngolady_cpu1_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM AM_WRITENOP
ADDRESS_MAP_END

static ADDRESS_MAP_START( jngolady_cpu1_io, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00,0x00) AM_READ(sound_latch_r)
	AM_RANGE(0x01,0x01) AM_WRITE(adpcm_w)
	AM_RANGE(0x02,0x02) AM_WRITENOP
ADDRESS_MAP_END


static ADDRESS_MAP_START( nsc_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x007f) AM_RAM //internal ram for irq etc.
	AM_RANGE(0x8000, 0x8000) AM_WRITENOP //write-only,irq related?
	AM_RANGE(0x9000, 0x9000) AM_READWRITE(slave_com_r,slave_com_w)
	AM_RANGE(0xc000, 0xc7ff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0xf000, 0xffff) AM_ROM
ADDRESS_MAP_END

/*************************************
 *
 *  Country Girl Memory Map
 *
 *************************************/

static ADDRESS_MAP_START( cntrygrl_cpu0_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
//  AM_RANGE(0xc000, 0xc7ff) AM_RAM
	AM_RANGE(0xe000, 0xefff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( cntrygrl_cpu0_io, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x01,0x01) AM_DEVREAD("aysnd", ay8910_r)
	AM_RANGE(0x02,0x03) AM_DEVWRITE("aysnd", ay8910_data_address_w)
	AM_RANGE(0x10,0x10) AM_READ_PORT("DSW") //dsw + blitter busy flag
	AM_RANGE(0x10,0x10) AM_WRITE(output_w)
	AM_RANGE(0x11,0x11) AM_WRITE(mux_w)
	AM_RANGE(0x12,0x17) AM_WRITE(blitter_process_w)
	AM_RANGE(0x20,0x2f) AM_WRITE(blit_vregs_w )
	AM_RANGE(0x30,0x30) AM_WRITENOP //? polls 0x03 continuously
//  AM_RANGE(0x31,0x31) AM_WRITE(sound_latch_w)
ADDRESS_MAP_END

/*************************************
 *
 *  Input Port Definitions
 *
 *************************************/

static INPUT_PORTS_START( jangou )
	PORT_START("PL1_1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_START("PL1_2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("PL2_1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(2)
	PORT_START("PL2_2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(2)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("PL1_3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("PL2_3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
	/*The "unknown" bits for this port might be actually unused*/
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN_NOMUX")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	/* there's a bank of 6 dip-switches in there*/
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_VBLANK ) // guess
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) // blitter busy flag
INPUT_PORTS_END

static INPUT_PORTS_START( macha )
	PORT_START("PL1_1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL1_2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL2_1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL2_2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL1_3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL2_3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN_NOMUX")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("1P A") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("1P B") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("1P C") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	/*The "unknown" bits for this port might be actually unused*/
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Analyzer")
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	/* there's a bank of 6 dip-switches in there. */
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_VBLANK ) // guess
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) // blitter busy flag
INPUT_PORTS_END


static INPUT_PORTS_START( cntrygrl )
	PORT_START("PL1_1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("1P Keep 1") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("1P Keep 2") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("1P Keep 3") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("1P Keep 4") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("1P Keep 5") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL1_2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("1P Play") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("1P Left") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("1P Right") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_NAME("1P Flip Flop")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("1P Bonus") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("1P Take") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL2_1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("2P Keep 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("2P Keep 2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("2P Keep 3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("2P Keep 4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2) PORT_NAME("2P Keep 5")
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL2_2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2) PORT_NAME("2P Play")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_PLAYER(2) PORT_NAME("1P Left")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_PLAYER(2) PORT_NAME("1P Right")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2) PORT_NAME("2P Flip Flop")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(2) PORT_NAME("1P Bonus")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_PLAYER(2) PORT_NAME("1P Take")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL1_3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL2_3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME( 0x04, 0x04, "Memory Reset" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Analyzer" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_DIPNAME( 0x40, 0x40, "Credit Clear" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Two Pairs" )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x03, "Maximum Rate")  PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "90" )
	PORT_DIPSETTING(    0x01, "83" )
	PORT_DIPSETTING(    0x02, "76" )
	PORT_DIPSETTING(    0x03, "69" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "2")
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x0c, "0" )
	PORT_DIPNAME( 0x10, 0x10, "Maximum Bet" )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, "1" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x20, 0x20, "Coin A setting" )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, "1 Coin / 25 Credits" )
	PORT_DIPNAME( 0x40, 0x40, "Coin B setting"  )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, "1 Coin / 10 Credits"  )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) // blitter busy flag

	PORT_START("IN_NOMUX")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( jngolady )
	PORT_INCLUDE( jangou )

	PORT_MODIFY("PL1_3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("P1 Start / P1 Mahjong Flip Flop")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_NAME("P1 Ready")

	PORT_MODIFY("PL2_3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 ) PORT_NAME("P2 Start / P2 Mahjong Flip Flop")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_NAME("P2 Ready")

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no service switch

	/* 6 or 7 dip-switches here? bit 6 seems used as vblank.*/
	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_VBLANK )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) //blitter busy flag
INPUT_PORTS_END

/*************************************
 *
 *  Sound HW Config
 *
 *************************************/

static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_HANDLER(input_mux_r),
	DEVCB_HANDLER(input_system_r),
	DEVCB_NULL,
	DEVCB_NULL
};

static const msm5205_interface msm5205_config =
{
	jngolady_vclk_cb,
	MSM5205_S96_4B
};

static SOUND_START( jangou )
{
	jangou_state *state = (jangou_state *)machine->driver_data;

	/* Create a timer to feed the CVSD DAC with sample bits */
	state->cvsd_bit_timer = timer_alloc(machine, cvsd_bit_timer_callback, NULL);
	timer_adjust_periodic(state->cvsd_bit_timer, ATTOTIME_IN_HZ(MASTER_CLOCK / 1024), 0, ATTOTIME_IN_HZ(MASTER_CLOCK / 1024));
}


/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_START( common )
{
	jangou_state *state = (jangou_state *)machine->driver_data;

	state->cpu_0 = machine->device("cpu0");
	state->cpu_1 = machine->device("cpu1");
	state->cvsd = machine->device("cvsd");
	state->nsc = machine->device("nsc");

	state_save_register_global_array(machine, state->pen_data);
	state_save_register_global_array(machine, state->blit_data);
	state_save_register_global(machine, state->mux_data);
}

static MACHINE_START( jangou )
{
	jangou_state *state = (jangou_state *)machine->driver_data;

	MACHINE_START_CALL(common);

	state_save_register_global(machine, state->cvsd_shiftreg);
	state_save_register_global(machine, state->cvsd_shift_cnt);
}

static MACHINE_START( jngolady )
{
	jangou_state *state = (jangou_state *)machine->driver_data;

	MACHINE_START_CALL(common);

	state_save_register_global(machine, state->adpcm_byte);
	state_save_register_global(machine, state->msm5205_vclk_toggle);
	state_save_register_global(machine, state->nsc_latch);
	state_save_register_global(machine, state->z80_latch);
}

static MACHINE_RESET( common )
{
	jangou_state *state = (jangou_state *)machine->driver_data;
	int i;

	state->mux_data = 0;

	for (i = 0; i < 6; i++)
		state->blit_data[i] = 0;

	for (i = 0; i < 16; i++)
		state->pen_data[i] = 0;
}

static MACHINE_RESET( jangou )
{
	jangou_state *state = (jangou_state *)machine->driver_data;

	MACHINE_RESET_CALL(common);

	state->cvsd_shiftreg = 0;
	state->cvsd_shift_cnt = 0;
}

static MACHINE_RESET( jngolady )
{
	jangou_state *state = (jangou_state *)machine->driver_data;

	MACHINE_RESET_CALL(common);

	state->adpcm_byte = 0;
	state->msm5205_vclk_toggle = 0;
	state->nsc_latch = 0;
	state->z80_latch = 0;
}

/* Note: All frequencies and dividers are unverified */
static MACHINE_DRIVER_START( jangou )

	/* driver data */
	MDRV_DRIVER_DATA(jangou_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("cpu0", Z80, MASTER_CLOCK / 8)
	MDRV_CPU_PROGRAM_MAP(cpu0_map)
	MDRV_CPU_IO_MAP(cpu0_io)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_CPU_ADD("cpu1", Z80, MASTER_CLOCK / 8)
	MDRV_CPU_PROGRAM_MAP(cpu1_map)
	MDRV_CPU_IO_MAP(cpu1_io)

	MDRV_MACHINE_START(jangou)
	MDRV_MACHINE_RESET(jangou)

	/* video hardware */
	MDRV_PALETTE_INIT(jangou)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) //not accurate
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 256-1, 16, 240-1)

	MDRV_PALETTE_LENGTH(32)

	MDRV_VIDEO_START(jangou)
	MDRV_VIDEO_UPDATE(jangou)

	/* sound hardware */
	MDRV_SOUND_START(jangou)

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("aysnd", AY8910, MASTER_CLOCK / 16)
	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)

	MDRV_SOUND_ADD("cvsd", HC55516, MASTER_CLOCK / 1024)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.60)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( jngolady )
	/* basic machine hardware */
	MDRV_IMPORT_FROM(jangou)

	MDRV_CPU_MODIFY("cpu0")
	MDRV_CPU_PROGRAM_MAP(jngolady_cpu0_map)

	MDRV_CPU_MODIFY("cpu1")
	MDRV_CPU_PROGRAM_MAP(jngolady_cpu1_map)
	MDRV_CPU_IO_MAP(jngolady_cpu1_io)

	MDRV_CPU_ADD("nsc", NSC8105, MASTER_CLOCK / 8)
	MDRV_CPU_PROGRAM_MAP(nsc_map)

	MDRV_MACHINE_START(jngolady)
	MDRV_MACHINE_RESET(jngolady)

	/* sound hardware */
	MDRV_SOUND_START(0)
	MDRV_DEVICE_REMOVE("cvsd")

	MDRV_SOUND_ADD("msm", MSM5205, XTAL_400kHz)
	MDRV_SOUND_CONFIG(msm5205_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( cntrygrl )
	/* basic machine hardware */
	MDRV_IMPORT_FROM(jangou)

	MDRV_CPU_MODIFY("cpu0")
	MDRV_CPU_PROGRAM_MAP(cntrygrl_cpu0_map )
	MDRV_CPU_IO_MAP(cntrygrl_cpu0_io )

	MDRV_DEVICE_REMOVE("cpu1")

	MDRV_MACHINE_START(common)
	MDRV_MACHINE_RESET(common)

	/* sound hardware */
	MDRV_SOUND_START(0)
	MDRV_DEVICE_REMOVE("cvsd")
MACHINE_DRIVER_END

/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

/*
JANGOU (C)1982 Nichibutsu

CPU:   Z80 *2
XTAL:  19.968MHz
SOUND: AY-3-8910

Location 2-P: HARRIS HCI-55536-5
Location 3-G: MB7051
*/

ROM_START( jangou )
	ROM_REGION( 0x10000, "cpu0", 0 )
	ROM_LOAD( "jg05.bin", 0x00000, 0x02000, CRC(a3cfe33f) SHA1(9ad34a2167568316d242c990ea6fe42dadd4ac30) )
	ROM_LOAD( "jg06.bin", 0x02000, 0x02000, CRC(d8523478) SHA1(f32c2e866c6aeae29f25f0947b07d725ce61d89d) )
	ROM_LOAD( "jg07.bin", 0x04000, 0x02000, CRC(4b30d1fc) SHA1(6f240aa4b7a343f180446581fe95cf7da0fba57b) )
	ROM_LOAD( "jg08.bin", 0x06000, 0x02000, CRC(bb078813) SHA1(a3b7df84629337c83307f49f52338aa983e531ba) )

	ROM_REGION( 0x10000, "cpu1", 0 )
	ROM_LOAD( "jg03.bin", 0x00000, 0x02000, CRC(5a113e90) SHA1(7d9ae481680fc640e03f6836f60bccb933bbef31) )
	ROM_LOAD( "jg04.bin", 0x02000, 0x02000, CRC(accd3ab5) SHA1(46a502801da7a56d73a984614f10b20897e340e8) )

	ROM_REGION( 0x10000, "gfx", 0 )
	ROM_LOAD( "jg01.bin", 0x00000, 0x02000, CRC(5034a744) SHA1(b83212b6ff12aaf730c6d3e3d1470d613bbe0d1d) )
	ROM_LOAD( "jg02.bin", 0x02000, 0x02000, CRC(10e7abfe) SHA1(3f5e0c5911baac19c381686e55f207166fe67d44) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "jg3_g.bin", 0x00, 0x20,  CRC(d389549d) SHA1(763486052b34f8a38247820109b114118a9f829f) )
ROM_END

/*

Monoshiri Quiz Osyaberi Macha (c)1983 Logitec
Same board as jangou

CPU: Z80x2
Sound: AY-3-8910, HC55536
XTAL: 20.000MHz

all EPROMs are 2764

POM1.9D -- Programs
POM2.9E |
POM3.9F |
POM4.9G |
POM5.9H /

POM6.9L -- Programs
POM7.9M |
POM8.9N |
POM9.9P /

POM10.5N -- Graphics
POM11.5M |
POM12.BIN|
POM13.BIN /
(12 and 13 is on small daughter board plugged into the socket 5L)

IC3G.BIN - Color PROM

RAM: HM6116LP-3 (16KbitSRAM location 9C, next to POM1)
     M5K4164NP-15 (64KbitDRAM location 4E, 4F, 4G, 4H)

*/

ROM_START( macha )
	ROM_REGION( 0x10000, "cpu0", 0 )
	ROM_LOAD( "pom1.9d", 0x00000, 0x02000, CRC(fbe28b4e) SHA1(2617f8c107b64aa540158a772a725eb68982e095) )
	ROM_LOAD( "pom2.9e", 0x02000, 0x02000, CRC(16a8d176) SHA1(30fe65d3a1744afc70c25c29119db3f5e7126601) )
	ROM_LOAD( "pom3.9f", 0x04000, 0x02000, CRC(c31eeb04) SHA1(65d4873fcaff677f03721139dc80b2fe5108c633) )
	ROM_LOAD( "pom4.9g", 0x06000, 0x02000, CRC(bdb0dd0e) SHA1(d8039fb9996e8707a0c5ca0760d4d6792bbe7270) )
	ROM_LOAD( "pom5.9h", 0x08000, 0x02000, CRC(db6d86e8) SHA1(e9c0f52abd504f39187d0fb7de5b7fffc795204c) )

	ROM_REGION( 0x10000, "cpu1", 0 )
	ROM_LOAD( "pom9.9p", 0x00000, 0x02000, CRC(f4d4e0a8) SHA1(914fe35d4434b826ca3b0a230b87017b033dd512) )
	ROM_LOAD( "pom8.9n", 0x02000, 0x02000, CRC(8be49178) SHA1(2233d964a25ef61063b97891f6ad46d6eb10b0c6) )
	ROM_LOAD( "pom7.9m", 0x04000, 0x02000, CRC(48a89180) SHA1(36e916583cc89090880111320537b545620d95fd) )
	ROM_LOAD( "pom6.9l", 0x06000, 0x02000, CRC(7dbafffe) SHA1(2f0c5a340625df30029874ca447f0662ea354547) )

	ROM_REGION( 0x10000, "gfx", 0 )
	ROM_LOAD( "pom10.5n",  0x00000, 0x02000, CRC(5e387db0) SHA1(72fd6d3ae722260cb50d1040faa128f3e5427402) )
	ROM_LOAD( "pom11.5m",  0x02000, 0x02000, CRC(17b54f4e) SHA1(5ecbebc063b5eb888ec1dbf210f54fa3a774ab70) )
	ROM_LOAD( "pom12.bin", 0x04000, 0x02000, CRC(5f1b73ca) SHA1(b8ce01a3975505a2a6b4d4c688b6c7ae4f6df07d) )
	ROM_LOAD( "pom13.bin", 0x06000, 0x02000, CRC(91c489f2) SHA1(a4d2fcebdbdea73ca03722104732e7c6efda5d4d) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "ic3g.bin", 0x00, 0x20,  CRC(d5243aa5) SHA1(d70d5dcb1a3241bec16589ed2eb1cc0054c9ed8e) )
ROM_END

/*
Jangou Lady
(c)1984 Nihon Bussan

CPU:    Z80 x2 (#1,#2)
    (40pin unknown:#3)
SOUND:  AY-3-8910
    MSM5218RS
OSC:    19.968MHz
    400KHz


1.5N    chr.
2.5M
3.5L

4.9P    Z80#1 prg.
5.9N
6.9M
7.9L

8.9H    Z80#2 prg.
9.9G
10.9F
11.9E
12.9D

M13.13  CPU#3 prg. (?)

JL.3G   color
*/

ROM_START( jngolady )
	ROM_REGION( 0xa000, "cpu0", 0 )
	ROM_LOAD( "8.9h",  0x08000, 0x02000, CRC(69e31165) SHA1(81b166c101136ed453a4f4cd88445eb1da5dd0aa) )
	ROM_LOAD( "9.9g",  0x06000, 0x02000, CRC(2faba771) SHA1(d88d0673c9b8cf3783b23c7290253475c9bf397e) )
	ROM_LOAD( "10.9f", 0x04000, 0x02000, CRC(dd311ff9) SHA1(be39ed25343796dc062a612fe82ca19ceb06a9e7) )
	ROM_LOAD( "11.9e", 0x02000, 0x02000, CRC(66cad038) SHA1(c60713615d58a9888e21bfec62fee53558a98eaa) )
	ROM_LOAD( "12.9d", 0x00000, 0x02000, CRC(99c5cc06) SHA1(3a9b3810bb542e252521923ec3026f10f176fa82) )

	ROM_REGION( 0x10000, "cpu1", 0 )
	ROM_LOAD( "4.9p", 0x00000, 0x02000, CRC(34cc2c71) SHA1(b407fed069baf3df316f0006a559a6c5e0be5bd0) )
	ROM_LOAD( "5.9n", 0x02000, 0x02000, CRC(42ed7832) SHA1(2681a532049fee494e1d1779d9dc08b17ce6e134) )
	ROM_LOAD( "6.9m", 0x04000, 0x02000, CRC(9e0e7ef4) SHA1(c68d30e60377c1027f4f053c528a80df09b8ee08) )
	ROM_LOAD( "7.9l", 0x06000, 0x02000, CRC(048615d9) SHA1(3c79830db8792ae0746513ed9849cc5d43051ed6) )

	ROM_REGION( 0x10000, "nsc", 0 )
	ROM_LOAD( "m13.13", 0x0f000, 0x01000, CRC(5b20b0e2) SHA1(228d2d931e6daab3572a1f128b5686f84b6a5a29) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "jl.3g", 0x00, 0x20, CRC(15ffff8c) SHA1(5782697f9c9a6bb04bbf7824cd49033c962899f0) )

	ROM_REGION( 0x10000, "gfx", 0 )
	ROM_LOAD( "1.5n", 0x00000, 0x02000, CRC(54027dee) SHA1(0616c12dbf3a0515cf4312fc5e238a61c97f8084) )
	ROM_LOAD( "2.5m", 0x02000, 0x02000, CRC(323dfad5) SHA1(5908acbf80e4b609ee8e5c313ac99717860dd19c) )
	ROM_LOAD( "3.5l", 0x04000, 0x04000, CRC(14688574) SHA1(241eaf1838239e38d11dff3556fb0a609a4b46aa) )
ROM_END

ROM_START( cntrygrl )
	ROM_REGION( 0x10000, "cpu0", 0 )
	ROM_LOAD( "rom4.bin", 0x00000, 0x02000, CRC(adba8e2f) SHA1(2aae77838e3de6e665b32a7fe4ac3f627c35b871)  )
	ROM_LOAD( "rom5.bin", 0x02000, 0x02000, CRC(24d210ed) SHA1(6a0eae9d459975fbaad75bf21284baac3ba4f872) )

	/*wtf,these 2 roms are next to the CPU roms, one is a CPU rom from Moon Quasar, the other a GFX rom from Crazy Climber,
      I dunno what's going on,the game doesn't appear to need these two....*/
	ROM_REGION( 0x1000, "user1", 0 )
	ROM_LOAD( "rom6.bin", 0x00000, 0x0800, CRC(33965a89) SHA1(92912cea76a472d9b709c664d9818844a07fcc32)  ) // = mq3    Moon Quasar
	ROM_LOAD( "rom7.bin", 0x00800, 0x0800, CRC(481b64cc) SHA1(3f35c545fc784ed4f969aba2d7be6e13a5ae32b7)  ) // = cc06   Crazy Climber (US)

	ROM_REGION( 0x10000, "gfx", 0 )
	ROM_LOAD( "rom1.bin", 0x00000, 0x02000, CRC(92033f37) SHA1(aa407c2feb1cbb7cbc6c59656338453c5a670749)  )
	ROM_LOAD( "rom2.bin", 0x02000, 0x02000, CRC(0588cc48) SHA1(f769ece2955eb9f055c499b6243a2fead9d07984)  )
	ROM_LOAD( "rom3.bin", 0x04000, 0x02000, CRC(ce00ff56) SHA1(c5e58707a5dd0f57c34b09de542ef30e96ab95d1)  )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "countrygirl_prom.bin", 0x00, 0x20, CRC(dc54dc52) SHA1(db91a7ae05eb6b6e4b42f91dfe20ac0da6680b46)  )
ROM_END

ROM_START( fruitbun )
	ROM_REGION( 0x10000, "cpu0", 0 )
	ROM_LOAD( "04.8l", 0x00000, 0x02000, CRC(480cb9bf) SHA1(f5e960cd8eaaa85336797fb5d895fa40f1fc4767) )
	ROM_LOAD( "05.8k", 0x02000, 0x02000, CRC(817add97) SHA1(ee5e1b193c22dfd36ac7836ff6bc14e783cb0e86) )

	/* same non-sense like Country Girl... */
	ROM_REGION( 0x1000, "user1", 0 )
	// 06.bin              = galx.1h               Galaxian Part X (moonaln hack)
	ROM_LOAD( "06.8j", 0x00000, 0x0800, CRC(e8810654) SHA1(b6924c7ad765c32714e6abd5bb56b2732edd5855) )
	// 07.bin              = rp9.6h                97.705078%
	// (a gfx rom from an undumped (later? licensed to Nichibutsu?) River Patrol set, it shows the body of the Orca logo if you replace it in rpatrol)
	ROM_LOAD( "07.8h", 0x00800, 0x0800, CRC(3717fa41) SHA1(373e5ef6bef3407da084508c48522c7058568188) )

	ROM_REGION( 0x10000, "gfx", 0 )
	ROM_LOAD( "01.5m", 0x00000, 0x02000, CRC(5ce7f40c) SHA1(ec1998f218a30a1e19ce20b71f170425b4330ea5) )
	ROM_LOAD( "02.5l", 0x02000, 0x02000, CRC(4ca0e465) SHA1(72af3f9e0534ba9a94854513250f6fa82d790549) )
	ROM_LOAD( "03.5k", 0x04000, 0x02000, CRC(8a8f6abd) SHA1(143607c423bfe337e6feed7cd4cc8be5e09fbd5e) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "tbp18s30n.4f", 0x00, 0x20, CRC(dc54dc52) SHA1(db91a7ae05eb6b6e4b42f91dfe20ac0da6680b46) ) //verified on real hardware
ROM_END

ROM_START( cntrygrla )
	ROM_REGION( 0x10000, "cpu0", 0 )
	ROM_LOAD( "5bunny.7l", 0x00000, 0x02000, CRC(ef07e029) SHA1(4be1bea9acaa37c615e937c3d0e0b8454aff2a8c) )
	ROM_LOAD( "5bunny.7k", 0x02000, 0x02000, CRC(cdf822b0) SHA1(a3cae79713cf7ff94a98705b7cba621730dbac1f) )

	/* same non-sense like Country Girl... */
	ROM_REGION( 0x1000, "user1", 0 )
	ROM_LOAD( "5bunny.7h", 0x00000, 0x0800, CRC(18da8863) SHA1(2151bc67173507dc35edc2426c2ef97d7937d01c) )
	ROM_LOAD( "5bunny.7j", 0x00800, 0x0800, CRC(06666bbf) SHA1(3d8eb4ea2d4fc6f3f327e710e19bcb68d8466d80) )

	ROM_REGION( 0x10000, "gfx", 0 )
	ROM_LOAD( "rom1.bin", 0x00000, 0x02000, CRC(92033f37) SHA1(aa407c2feb1cbb7cbc6c59656338453c5a670749)  ) //5bunny.m5
	ROM_LOAD( "rom2.bin", 0x02000, 0x02000, CRC(0588cc48) SHA1(f769ece2955eb9f055c499b6243a2fead9d07984)  ) //5bunny.l5
	ROM_LOAD( "rom3.bin", 0x04000, 0x02000, CRC(ce00ff56) SHA1(c5e58707a5dd0f57c34b09de542ef30e96ab95d1)  ) //5bunny.k5

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "tbp18s30n.4f", 0x00, 0x20, CRC(dc54dc52) SHA1(db91a7ae05eb6b6e4b42f91dfe20ac0da6680b46) ) //verified on real hardware
ROM_END

ROM_START( luckygrl )
	ROM_REGION( 0x10000, "cpu0", 0 ) //encrypted z80 cpu
	ROM_LOAD( "5.9c", 0x00000, 0x01000, CRC(79b34eb2) SHA1(4b4916e09bfd6573fd2c7a7254fa4419164e0c4d) )
	ROM_LOAD( "7.9f", 0x01000, 0x01000, CRC(14a44d23) SHA1(4f84a8f986a8fd9d5ac0636be1bb036c3b2746c2) )
	ROM_LOAD( "6.9e", 0x02000, 0x01000, CRC(06850aa8) SHA1(c23cb6b7b26d5586b1a095dee88228d1613ae7d0) )

	ROM_REGION( 0x80000, "gfx", 0 )
	ROM_LOAD( "1.5r",      0x00000, 0x2000, CRC(fb429678) SHA1(00e37e90550d9190d06977a5f5ed75b691750cc1) )
	ROM_LOAD( "piggy2.5r", 0x02000, 0x2000, CRC(a3919845) SHA1(45fffe34b7a29ecf8c8feb4152b5c7330ea3ad83) )
	ROM_LOAD( "3.5n",      0x04000, 0x2000, CRC(130cfb89) SHA1(86b2a2142675cbd69d7cccab9b00f4c8863cdcbc) )
	ROM_LOAD( "piggy4.5r", 0x06000, 0x2000, CRC(f5641c95) SHA1(e824b95c80d00789f07391aa5dcc02505bb8e141) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "prom_mb7051.3h", 0x00, 0x20, CRC(dff9f7a1) SHA1(593c99434df5dfd900ec57e3efa459903b589d96) )
ROM_END


/*************************************
 *
 *  Driver initialization
 *
 *************************************/

/*Temporary kludge for make the RNG work*/
static READ8_HANDLER( jngolady_rng_r )
{
	return mame_rand(space->machine);
}

static DRIVER_INIT( jngolady )
{
	memory_install_read8_handler(cputag_get_address_space(machine, "nsc", ADDRESS_SPACE_PROGRAM), 0x08, 0x08, 0, 0, jngolady_rng_r );
}

static DRIVER_INIT (luckygrl)
{
	// this is WRONG
	int A;
	UINT8 *ROM = memory_region(machine, "cpu0");

	unsigned char patn1[32] = {
		0x00, 0xA0, 0x00, 0xA0, 0x00, 0xA0, 0x00, 0xA0, 0x00, 0xA0, 0x00, 0xA0, 0x00, 0xA0, 0x00, 0xA0,
		0x88, 0x28, 0x88, 0x28, 0x88, 0x28, 0x88, 0x28, 0x88, 0x28, 0x88, 0x28, 0x88, 0x28, 0x88, 0x28,
	};

	unsigned char patn2[32] = {
		0x28, 0x20, 0x28, 0x20, 0x28, 0x20, 0x28, 0x20, 0x28, 0x20, 0x28, 0x20, 0x28, 0x20, 0x28, 0x20,
		0x28, 0x88, 0x28, 0x88, 0x28, 0x88, 0x28, 0x88,	0x28, 0x88, 0x28, 0x88, 0x28, 0x88, 0x28, 0x88
	};

	for (A = 0; A < 0x3000; A++)
	{
		UINT8 dat = ROM[A];
		if (A&0x100) dat = dat ^ patn2[A & 0x1f];
		else dat = dat ^ patn1[A & 0x1f];

		ROM[A] = dat;
	}


	#if 0
	{
		FILE *fp;
		char filename[256];
		sprintf(filename,"decrypted_%s", machine->gamedrv->name);
		fp=fopen(filename, "w+b");
		if (fp)
		{
			fwrite(ROM, 0x3000, 1, fp);
			fclose(fp);
		}
	}
	#endif

}


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1983,  jangou,     0,        jangou,   jangou,    0,        ROT0, "Nichibutsu",     "Jangou [BET] (Japan)", GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
GAME( 1983,  macha,      0,        jangou,   macha,     0,        ROT0, "Logitec",        "Monoshiri Quiz Osyaberi Macha (Japan)", GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
GAME( 1984,  jngolady,   0,        jngolady, jngolady,  jngolady, ROT0, "Nichibutsu",     "Jangou Lady (Japan)", GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
GAME( 1984,  cntrygrl,   0,        cntrygrl, cntrygrl,  0,        ROT0, "Royal Denshi",   "Country Girl (Japan set 1)",  GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
GAME( 1984,  cntrygrla,  cntrygrl, cntrygrl, cntrygrl,  0,        ROT0, "Nichibutsu",     "Country Girl (Japan set 2)",  GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
GAME( 1984,  fruitbun,   cntrygrl, cntrygrl, cntrygrl,  0,        ROT0, "Nichibutsu",     "Fruits & Bunny (World?)",  GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
/* The following might not run there... */
GAME( 1984?, luckygrl,   0,        cntrygrl, cntrygrl,  luckygrl, ROT0, "Wing Co., Ltd.", "Lucky Girl? (Wing)", GAME_NOT_WORKING | GAME_SUPPORTS_SAVE )

/*
Some other games that might run on this HW:
    Jangou (non-BET version) (WR score listed on MyCom magazines)
    Jangou Night (first "mature" mahjong ever made)
    Jangou Lady (BET version) (images on the flyer, it might not exists)
    Hana Royal
    Hana Puter
    Royal Card
*/
