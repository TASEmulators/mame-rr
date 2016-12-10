/***************************************************************************

    AmeriDarts      (c) 1989 Ameri Corporation
    Cool Pool       (c) 1992 Catalina
    9 Ball Shootout (c) 1993 E-Scape/Bundra

    driver by Nicola Salmoria and Aaron Giles


    The main cpu is a TMS34010; it is encrypted in 9 Ball Shootout.

    The second CPU in AmeriDarts is a TMS32015; it controls sound and
    the trackball inputs.

    The second CPU in Cool Pool and 9 Ball Shootout is a TMS320C26; the code
    is the same in the two games.

    Cool Pool:
    - The checksum test routine is wrong, e.g. when it says to be testing
      4U/8U it is actually reading 4U/8U/3U/7U, when testing 3U/7U it
      actually reads 2U/6U/1U/5U. The placement cannot therefore be exactly
      determined by the check passing.

***************************************************************************/

#include "emu.h"
#include "cpu/tms32010/tms32010.h"
#include "cpu/tms34010/tms34010.h"
#include "cpu/tms32025/tms32025.h"
#include "video/tlc34076.h"
#include "sound/dac.h"
#include "includes/coolpool.h"



/*************************************
 *
 *  Local variables
 *
 *************************************/



static const UINT16 nvram_unlock_seq[] =
{
	0x3fb, 0x3fb, 0x3f8, 0x3fc, 0x3fa, 0x3fe, 0x3f9, 0x3fd, 0x3fb, 0x3ff
};
#define NVRAM_UNLOCK_SEQ_LEN (ARRAY_LENGTH(nvram_unlock_seq))
static UINT16 nvram_write_seq[NVRAM_UNLOCK_SEQ_LEN];
static UINT8 nvram_write_enable;



/*************************************
 *
 *  Video updates
 *
 *************************************/

static void amerdart_scanline(screen_device &screen, bitmap_t *bitmap, int scanline, const tms34010_display_params *params)
{
	coolpool_state *state = (coolpool_state *)screen.machine->driver_data;

	UINT16 *vram = &state->vram_base[(params->rowaddr << 8) & 0xff00];
	UINT32 *dest = BITMAP_ADDR32(bitmap, scanline, 0);
	rgb_t pens[16];
	int coladdr = params->coladdr;
	int x;

	/* update the palette */
	if (scanline < 256)
		for (x = 0; x < 16; x++)
		{
			UINT16 pal = state->vram_base[x];
			pens[x] = MAKE_RGB(pal4bit(pal >> 4), pal4bit(pal >> 8), pal4bit(pal >> 12));
		}

	for (x = params->heblnk; x < params->hsblnk; x += 4)
	{
		UINT16 pixels = vram[coladdr++ & 0xff];
		dest[x + 0] = pens[(pixels >> 0) & 15];
		dest[x + 1] = pens[(pixels >> 4) & 15];
		dest[x + 2] = pens[(pixels >> 8) & 15];
		dest[x + 3] = pens[(pixels >> 12) & 15];
	}
}


static void coolpool_scanline(screen_device &screen, bitmap_t *bitmap, int scanline, const tms34010_display_params *params)
{
	coolpool_state *state = (coolpool_state *)screen.machine->driver_data;

	UINT16 *vram = &state->vram_base[(params->rowaddr << 8) & 0x1ff00];
	UINT32 *dest = BITMAP_ADDR32(bitmap, scanline, 0);
	const rgb_t *pens = tlc34076_get_pens();
	int coladdr = params->coladdr;
	int x;

	for (x = params->heblnk; x < params->hsblnk; x += 2)
	{
		UINT16 pixels = vram[coladdr++ & 0xff];
		dest[x + 0] = pens[pixels & 0xff];
		dest[x + 1] = pens[pixels >> 8];
	}
}



/*************************************
 *
 *  Shift register access
 *
 *************************************/

static void coolpool_to_shiftreg(const address_space *space, UINT32 address, UINT16 *shiftreg)
{
	coolpool_state *state = (coolpool_state *)space->machine->driver_data;

	memcpy(shiftreg, &state->vram_base[TOWORD(address) & ~TOWORD(0xfff)], TOBYTE(0x1000));
}


static void coolpool_from_shiftreg(const address_space *space, UINT32 address, UINT16 *shiftreg)
{
	coolpool_state *state = (coolpool_state *)space->machine->driver_data;

	memcpy(&state->vram_base[TOWORD(address) & ~TOWORD(0xfff)], shiftreg, TOBYTE(0x1000));
}



/*************************************
 *
 *  Game initialzation
 *
 *************************************/

static MACHINE_RESET( amerdart )
{
	coolpool_state *state = (coolpool_state *)machine->driver_data;

	state->maincpu = machine->device("maincpu");
	state->dsp = machine->device("dsp");

	nvram_write_enable = 0;
}


static MACHINE_RESET( coolpool )
{
	tlc34076_reset(6);
	nvram_write_enable = 0;
}



/*************************************
 *
 *  NVRAM writes with thrash protect
 *
 *************************************/

static TIMER_DEVICE_CALLBACK( nvram_write_timeout )
{
	nvram_write_enable = 0;
}


static WRITE16_HANDLER( nvram_thrash_w )
{
	/* keep track of the last few writes */
	memmove(&nvram_write_seq[0], &nvram_write_seq[1], (NVRAM_UNLOCK_SEQ_LEN - 1) * sizeof(nvram_write_seq[0]));
	nvram_write_seq[NVRAM_UNLOCK_SEQ_LEN - 1] = offset & 0x3ff;

	/* if they match the unlock sequence, enable writes and set a timeout */
	if (!memcmp(nvram_unlock_seq, nvram_write_seq, sizeof(nvram_unlock_seq)))
	{
		nvram_write_enable = 1;
		timer_device *nvram_timer = space->machine->device<timer_device>("nvram_timer");
		nvram_timer->adjust(ATTOTIME_IN_MSEC(1000));
	}
}


static WRITE16_HANDLER( nvram_data_w )
{
	/* only the low 8 bits matter */
	if (ACCESSING_BITS_0_7)
	{
		if (nvram_write_enable)
			space->machine->generic.nvram.u16[offset] = data & 0xff;
	}
}


static WRITE16_HANDLER( nvram_thrash_data_w )
{
	nvram_data_w(space, offset, data, mem_mask);
	nvram_thrash_w(space, offset, data, mem_mask);
}



/*************************************
 *
 *  AmeriDarts IOP handling
 *
 *************************************/

static TIMER_DEVICE_CALLBACK( amerdart_audio_int_gen )
{
	coolpool_state *state = (coolpool_state *)timer.machine->driver_data;

	cpu_set_input_line(state->dsp, 0, ASSERT_LINE);
	cpu_set_input_line(state->dsp, 0, CLEAR_LINE);
}


static WRITE16_HANDLER( amerdart_misc_w )
{
	logerror("%08x:IOP_system_w %04x\n",cpu_get_pc(space->cpu),data);

	coin_counter_w(space->machine, 0, ~data & 0x0001);
	coin_counter_w(space->machine, 1, ~data & 0x0002);

	/* bits 10-15 are counted down over time */

	cputag_set_input_line(space->machine, "dsp", INPUT_LINE_RESET, (data & 0x0400) ? ASSERT_LINE : CLEAR_LINE);
}

static READ16_HANDLER( amerdart_dsp_bio_line_r )
{
	coolpool_state *state = (coolpool_state *)space->machine->driver_data;

	static UINT8 old_cmd;
	static UINT8 same_cmd_count;

	/* Skip idle checking */
	if (old_cmd == state->cmd_pending)
		same_cmd_count += 1;
	else
		same_cmd_count = 0;

	if (same_cmd_count >= 5)
	{
		same_cmd_count = 5;
		cpu_spin(space->cpu);
	}
	old_cmd = state->cmd_pending;

	return state->cmd_pending ? CLEAR_LINE : ASSERT_LINE;
}

static READ16_HANDLER( amerdart_iop_r )
{
	coolpool_state *state = (coolpool_state *)space->machine->driver_data;

//  logerror("%08x:IOP read %04x\n",cpu_get_pc(space->cpu),state->iop_answer);
	cputag_set_input_line(space->machine, "maincpu", 1, CLEAR_LINE);

	return state->iop_answer;
}

static WRITE16_HANDLER( amerdart_iop_w )
{
	coolpool_state *state = (coolpool_state *)space->machine->driver_data;

//  logerror("%08x:IOP write %04x\n", cpu_get_pc(space->cpu), data);
	COMBINE_DATA(&state->iop_cmd);
	state->cmd_pending = 1;
}

static READ16_HANDLER( amerdart_dsp_cmd_r )
{
	coolpool_state *state = (coolpool_state *)space->machine->driver_data;

//  logerror("%08x:DSP cmd_r %04x\n", cpu_get_pc(space->cpu), state->iop_cmd);
	state->cmd_pending = 0;
	return state->iop_cmd;
}

static WRITE16_HANDLER( amerdart_dsp_answer_w )
{
	coolpool_state *state = (coolpool_state *)space->machine->driver_data;

//  logerror("%08x:DSP answer %04x\n", cpu_get_pc(space->cpu), data);
	state->iop_answer = data;
	cputag_set_input_line(space->machine, "maincpu", 1, ASSERT_LINE);
}


/*************************************
 *
 *  Ameri Darts trackball inputs
 *
 *************************************/

static int amerdart_trackball_inc(int data)
{
	switch (data & 0x03)	/* Bits of opposite track direction must both change with identical levels */
	{
		case 0x00:	data ^= 0x03;	break;
		case 0x01:	data ^= 0x01;	break;
		case 0x02:	data ^= 0x01;	break;
		case 0x03:	data ^= 0x03;	break;
	}
	return data;
}
static int amerdart_trackball_dec(int data)
{
	switch (data & 0x03)	/* Bits of opposite track direction must both change with opposing levels */
	{
		case 0x00:	data ^= 0x01;	break;
		case 0x01:	data ^= 0x03;	break;
		case 0x02:	data ^= 0x03;	break;
		case 0x03:	data ^= 0x01;	break;
	}
	return data;
}

static int amerdart_trackball_direction(const address_space *space, int num, int data)
{
	coolpool_state *state = (coolpool_state *)space->machine->driver_data;

	UINT16 result_x = (data & 0x0c) >> 2;
	UINT16 result_y = (data & 0x03) >> 0;


	if ((state->dx[num] == 0) && (state->dy[num] < 0)) {		/* Up */
		state->oldy[num]--;
		result_x = amerdart_trackball_inc(result_x);
		result_y = amerdart_trackball_inc(result_y);
	}
	if ((state->dx[num] == 0) && (state->dy[num] > 0)) {		/* Down */
		state->oldy[num]++;
		result_x = amerdart_trackball_dec(result_x);
		result_y = amerdart_trackball_dec(result_y);
	}
	if ((state->dx[num] < 0) && (state->dy[num] == 0)) {		/* Left */
		state->oldx[num]--;
		result_x = amerdart_trackball_inc(result_x);
		result_y = amerdart_trackball_dec(result_y);
	}
	if ((state->dx[num] > 0) && (state->dy[num] == 0)) {		/* Right */
		state->oldx[num]++;
		result_x = amerdart_trackball_dec(result_x);
		result_y = amerdart_trackball_inc(result_y);
	}
	if ((state->dx[num] < 0) && (state->dy[num] < 0)) {			/* Left & Up */
		state->oldx[num]--;
		state->oldy[num]--;
		result_x = amerdart_trackball_inc(result_x);
	}
	if ((state->dx[num] < 0) && (state->dy[num] > 0)) {			/* Left & Down */
		state->oldx[num]--;
		state->oldy[num]++;
		result_y = amerdart_trackball_dec(result_y);
	}
	if ((state->dx[num] > 0) && (state->dy[num] < 0)) {			/* Right & Up */
		state->oldx[num]++;
		state->oldy[num]--;
		result_y = amerdart_trackball_inc(result_y);
	}
	if ((state->dx[num] > 0) && (state->dy[num] > 0)) {			/* Right & Down */
		state->oldx[num]++;
		state->oldy[num]++;
		result_x = amerdart_trackball_dec(result_x);
	}

	data = ((result_x << 2) & 0x0c) | ((result_y << 0) & 0x03);

	return data;
}


static READ16_HANDLER( amerdart_trackball_r )
{
/*
    Trackballs seem to be handled as though they're rotated 45 degrees anti-clockwise.

    Sensor data as shown on Input test screen and associated bits read by TMS32015 DSP port:
    xxyy xxyy ???? ????
    |||| |||| |||| ||||
    |||| |||| ++++-++++-- Unused
    |||| ||||
    |||| |||+------------ Trackball 1 Up    sensor
    |||| ||+------------- Trackball 1 Down  sensor
    |||| |+-------------- Trackball 1 Left  sensor
    |||| +--------------- Trackball 1 Right sensor
    ||||
    |||+----------------- Trackball 2 Up    sensor
    ||+------------------ Trackball 2 Down  sensor
    |+------------------- Trackball 2 Left  sensor
    +-------------------- Trackball 2 Right sensor

    Opposite direction bits toggling the   same   level indicate increment (+) input movement  (00 -> 11 -> 00, etc)
    Opposite direction bits toggling the opposite level indicate decrement (-) input movement  (01 -> 10 -> 01, etc)

    Input state requirements to indicate trackball direction.
    Direction      StateX  StateY
    =============================
    UP              X +     Y +
    Down            X -     Y -
    Left            X +     Y -
    Right           X -     Y +
    Up   & Left     X +     Y 0
    Up   & Right    X 0     Y +
    Down & Left     X 0     Y -
    Down & Right    X -     Y 0

*/

	coolpool_state *state = (coolpool_state *)space->machine->driver_data;


	state->result = (state->lastresult | 0x00ff);

	state->newx[1] = input_port_read(space->machine, "XAXIS1");	/* Trackball 1  Left - Right */
	state->newy[1] = input_port_read(space->machine, "YAXIS1");	/* Trackball 1   Up  - Down  */
	state->newx[2] = input_port_read(space->machine, "XAXIS2");	/* Trackball 2  Left - Right */
	state->newy[2] = input_port_read(space->machine, "YAXIS2");	/* Trackball 2   Up  - Down  */

	state->dx[1] = (INT8)(state->newx[1] - state->oldx[1]);
	state->dy[1] = (INT8)(state->newy[1] - state->oldy[1]);
	state->dx[2] = (INT8)(state->newx[2] - state->oldx[2]);
	state->dy[2] = (INT8)(state->newy[2] - state->oldy[2]);

	/* Determine Trackball 1 direction state */
	state->result = (state->result & 0xf0ff) | (amerdart_trackball_direction(space, 1, ((state->result >>  8) & 0xf)) <<  8);

	/* Determine Trackball 2 direction state */
	state->result = (state->result & 0x0fff) | (amerdart_trackball_direction(space, 2, ((state->result >> 12) & 0xf)) << 12);


//  logerror("%08X:read port 6 (X=%02X Y=%02X oldX=%02X oldY=%02X oldRes=%04X Res=%04X)\n", cpu_get_pc(space->cpu), state->newx, state->newy, state->oldx, state->oldy, state->lastresult, state->result);

	state->lastresult = state->result;

	return state->result;
}


/*************************************
 *
 *  Cool Pool IOP control
 *
 *************************************/

static WRITE16_HANDLER( coolpool_misc_w )
{
	logerror("%08x:IOP_system_w %04x\n",cpu_get_pc(space->cpu),data);

	coin_counter_w(space->machine, 0, ~data & 0x0001);
	coin_counter_w(space->machine, 1, ~data & 0x0002);

	cputag_set_input_line(space->machine, "dsp", INPUT_LINE_RESET, (data & 0x0400) ? ASSERT_LINE : CLEAR_LINE);
}



/*************************************
 *
 *  Cool Pool IOP communications
 *  (from TMS34010 side)
 *
 *************************************/

static TIMER_CALLBACK( deferred_iop_w )
{
	coolpool_state *state = (coolpool_state *)machine->driver_data;

	state->iop_cmd = param;
	state->cmd_pending = 1;
	cputag_set_input_line(machine, "dsp", 0, HOLD_LINE);	/* ???  I have no idea who should generate this! */
															/* the DSP polls the status bit so it isn't strictly */
															/* necessary to also have an IRQ */
	cpuexec_boost_interleave(machine, attotime_zero, ATTOTIME_IN_USEC(50));
}


static WRITE16_HANDLER( coolpool_iop_w )
{
	logerror("%08x:IOP write %04x\n", cpu_get_pc(space->cpu), data);
	timer_call_after_resynch(space->machine, NULL, data, deferred_iop_w);
}


static READ16_HANDLER( coolpool_iop_r )
{
	coolpool_state *state = (coolpool_state *)space->machine->driver_data;

	logerror("%08x:IOP read %04x\n",cpu_get_pc(space->cpu),state->iop_answer);
	cputag_set_input_line(space->machine, "maincpu", 1, CLEAR_LINE);

	return state->iop_answer;
}



/*************************************
 *
 *  Cool Pool IOP communications
 *  (from IOP side)
 *
 *************************************/

static READ16_HANDLER( dsp_cmd_r )
{
	coolpool_state *state = (coolpool_state *)space->machine->driver_data;

	state->cmd_pending = 0;
	logerror("%08x:IOP cmd_r %04x\n", cpu_get_pc(space->cpu), state->iop_cmd);
	return state->iop_cmd;
}


static WRITE16_HANDLER( dsp_answer_w )
{
	coolpool_state *state = (coolpool_state *)space->machine->driver_data;

	logerror("%08x:IOP answer %04x\n", cpu_get_pc(space->cpu), data);
	state->iop_answer = data;
	cputag_set_input_line(space->machine, "maincpu", 1, ASSERT_LINE);
}


static READ16_HANDLER( dsp_bio_line_r )
{
	coolpool_state *state = (coolpool_state *)space->machine->driver_data;

	return state->cmd_pending ? CLEAR_LINE : ASSERT_LINE;
}


static READ16_HANDLER( dsp_hold_line_r )
{
	return CLEAR_LINE;	/* ??? */
}



/*************************************
 *
 *  IOP ROM and DAC access
 *
 *************************************/

static READ16_HANDLER( dsp_rom_r )
{
	coolpool_state *state = (coolpool_state *)space->machine->driver_data;
	UINT8 *rom = memory_region(space->machine, "user2");

	return rom[state->iop_romaddr & (memory_region_length(space->machine, "user2") - 1)];
}


static WRITE16_HANDLER( dsp_romaddr_w )
{
	coolpool_state *state = (coolpool_state *)space->machine->driver_data;

	switch (offset)
	{
		case 0:
			state->iop_romaddr = (state->iop_romaddr & 0xffff00) | (data >> 8);
			break;

		case 1:
			state->iop_romaddr = (state->iop_romaddr & 0x0000ff) | (data << 8);
			break;
	}
}


static WRITE16_DEVICE_HANDLER( dsp_dac_w )
{
	dac_signed_data_16_w(device, (INT16)(data << 4) + 0x8000);
}



/*************************************
 *
 *  Cool Pool trackball inputs
 *
 *************************************/

static READ16_HANDLER( coolpool_input_r )
{
	coolpool_state *state = (coolpool_state *)space->machine->driver_data;

	state->result = (input_port_read(space->machine, "IN1") & 0x00ff) | (state->lastresult & 0xff00);
	state->newx[1] = input_port_read(space->machine, "XAXIS");
	state->newy[1] = input_port_read(space->machine, "YAXIS");
	state->dx[1] = (INT8)(state->newx[1] - state->oldx[1]);
	state->dy[1] = (INT8)(state->newy[1] - state->oldy[1]);

	if (state->dx[1] < 0)
	{
		state->oldx[1]--;
		switch (state->result & 0x300)
		{
			case 0x000:	state->result ^= 0x200;	break;
			case 0x100:	state->result ^= 0x100;	break;
			case 0x200:	state->result ^= 0x100;	break;
			case 0x300:	state->result ^= 0x200;	break;
		}
	}
	if (state->dx[1] > 0)
	{
		state->oldx[1]++;
		switch (state->result & 0x300)
		{
			case 0x000:	state->result ^= 0x100;	break;
			case 0x100:	state->result ^= 0x200;	break;
			case 0x200:	state->result ^= 0x200;	break;
			case 0x300:	state->result ^= 0x100;	break;
		}
	}

	if (state->dy[1] < 0)
	{
		state->oldy[1]--;
		switch (state->result & 0xc00)
		{
			case 0x000:	state->result ^= 0x800;	break;
			case 0x400:	state->result ^= 0x400;	break;
			case 0x800:	state->result ^= 0x400;	break;
			case 0xc00:	state->result ^= 0x800;	break;
		}
	}
	if (state->dy[1] > 0)
	{
		state->oldy[1]++;
		switch (state->result & 0xc00)
		{
			case 0x000:	state->result ^= 0x400;	break;
			case 0x400:	state->result ^= 0x800;	break;
			case 0x800:	state->result ^= 0x800;	break;
			case 0xc00:	state->result ^= 0x400;	break;
		}
	}

//  logerror("%08X:read port 7 (X=%02X Y=%02X oldX=%02X oldY=%02X res=%04X)\n", cpu_get_pc(space->cpu),
//      state->newx[1], state->newy[1], state->oldx[1], state->oldy[1], state->result);
	state->lastresult = state->result;
	return state->result;
}



/*************************************
 *
 *  Main Memory maps
 *
 *************************************/

static ADDRESS_MAP_START( amerdart_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x00000000, 0x000fffff) AM_RAM AM_BASE_MEMBER(coolpool_state,vram_base)
	AM_RANGE(0x04000000, 0x0400000f) AM_WRITE(amerdart_misc_w)
	AM_RANGE(0x05000000, 0x0500000f) AM_READWRITE(amerdart_iop_r, amerdart_iop_w)
	AM_RANGE(0x06000000, 0x06007fff) AM_RAM_WRITE(nvram_thrash_data_w) AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE(0xc0000000, 0xc00001ff) AM_READWRITE(tms34010_io_register_r, tms34010_io_register_w)
	AM_RANGE(0xffb00000, 0xffffffff) AM_ROM AM_REGION("user1", 0)
ADDRESS_MAP_END


static ADDRESS_MAP_START( coolpool_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x00000000, 0x001fffff) AM_RAM AM_BASE_MEMBER(coolpool_state,vram_base)
	AM_RANGE(0x01000000, 0x010000ff) AM_READWRITE(tlc34076_lsb_r, tlc34076_lsb_w)	// IMSG176P-40
	AM_RANGE(0x02000000, 0x020000ff) AM_READWRITE(coolpool_iop_r, coolpool_iop_w)
	AM_RANGE(0x03000000, 0x0300000f) AM_WRITE(coolpool_misc_w)
	AM_RANGE(0x03000000, 0x03ffffff) AM_ROM AM_REGION("gfx1", 0)
	AM_RANGE(0x06000000, 0x06007fff) AM_RAM_WRITE(nvram_thrash_data_w) AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE(0xc0000000, 0xc00001ff) AM_READWRITE(tms34010_io_register_r, tms34010_io_register_w)
	AM_RANGE(0xffe00000, 0xffffffff) AM_ROM AM_REGION("user1", 0)
ADDRESS_MAP_END


static ADDRESS_MAP_START( nballsht_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x00000000, 0x001fffff) AM_RAM AM_BASE_MEMBER(coolpool_state,vram_base)
	AM_RANGE(0x02000000, 0x020000ff) AM_READWRITE(coolpool_iop_r, coolpool_iop_w)
	AM_RANGE(0x03000000, 0x0300000f) AM_WRITE(coolpool_misc_w)
	AM_RANGE(0x04000000, 0x040000ff) AM_READWRITE(tlc34076_lsb_r, tlc34076_lsb_w)	// IMSG176P-40
	AM_RANGE(0x06000000, 0x0601ffff) AM_MIRROR(0x00020000) AM_RAM_WRITE(nvram_thrash_data_w) AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE(0xc0000000, 0xc00001ff) AM_READWRITE(tms34010_io_register_r, tms34010_io_register_w)
	AM_RANGE(0xff000000, 0xff7fffff) AM_ROM AM_REGION("gfx1", 0)
	AM_RANGE(0xffc00000, 0xffffffff) AM_ROM AM_REGION("user1", 0)
ADDRESS_MAP_END



/*************************************
 *
 *  DSP Memory maps
 *
 *************************************/

static ADDRESS_MAP_START( amerdart_dsp_pgm_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000, 0x0fff) AM_ROM
ADDRESS_MAP_END
	/* 000 - 0FF  TMS32015 Internal Data RAM (256 words) in Data Address Space */


static ADDRESS_MAP_START( amerdart_dsp_io_map, ADDRESS_SPACE_IO, 16 )
	AM_RANGE(0x00, 0x01) AM_WRITE(dsp_romaddr_w)
	AM_RANGE(0x02, 0x02) AM_WRITE(amerdart_dsp_answer_w)
	AM_RANGE(0x03, 0x03) AM_DEVWRITE("dac", dsp_dac_w)
	AM_RANGE(0x04, 0x04) AM_READ(dsp_rom_r)
	AM_RANGE(0x05, 0x05) AM_READ_PORT("IN0")
	AM_RANGE(0x06, 0x06) AM_READ(amerdart_trackball_r)
	AM_RANGE(0x07, 0x07) AM_READ(amerdart_dsp_cmd_r)
	AM_RANGE(TMS32010_BIO, TMS32010_BIO) AM_READ(amerdart_dsp_bio_line_r)
ADDRESS_MAP_END



static ADDRESS_MAP_START( coolpool_dsp_pgm_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( coolpool_dsp_io_map, ADDRESS_SPACE_IO, 16 )
	AM_RANGE(0x00, 0x01) AM_WRITE(dsp_romaddr_w)
	AM_RANGE(0x02, 0x02) AM_READWRITE(dsp_cmd_r, dsp_answer_w)
	AM_RANGE(0x03, 0x03) AM_DEVWRITE("dac", dsp_dac_w)
	AM_RANGE(0x04, 0x04) AM_READ(dsp_rom_r)
	AM_RANGE(0x05, 0x05) AM_READ_PORT("IN0")
	AM_RANGE(0x07, 0x07) AM_READ_PORT("IN1")
	AM_RANGE(TMS32025_BIO, TMS32025_BIO) AM_READ(dsp_bio_line_r)
	AM_RANGE(TMS32025_HOLD, TMS32025_HOLD) AM_READ(dsp_hold_line_r)
//  AM_RANGE(TMS32025_HOLDA, TMS32025_HOLDA) AM_WRITE(dsp_HOLDA_signal_w)
ADDRESS_MAP_END



/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( amerdart )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_SERVICE_NO_TOGGLE( 0x0010, IP_ACTIVE_LOW )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("XAXIS1")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("YAXIS1")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("XAXIS2")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(15) PORT_PLAYER(2)

	PORT_START("YAXIS2")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(15) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( coolpool )
	PORT_START("IN0")
	PORT_BIT( 0x00ff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0f00, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0xc000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_SERVICE_NO_TOGGLE( 0x0010, IP_ACTIVE_LOW )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_SPECIAL )

	PORT_START("XAXIS")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10)

	PORT_START("YAXIS")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_REVERSE
INPUT_PORTS_END


static INPUT_PORTS_START( 9ballsht )
	PORT_START("IN0")
	PORT_BIT( 0x00ff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0300, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_SERVICE_NO_TOGGLE( 0x0010, IP_ACTIVE_LOW )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
INPUT_PORTS_END



/*************************************
 *
 *  34010 configuration
 *
 *************************************/

static const tms34010_config tms_config_amerdart =
{
	FALSE,							/* halt on reset */
	"screen",						/* the screen operated on */
	XTAL_40MHz/12,					/* pixel clock */
	2,								/* pixels per clock */
	amerdart_scanline,				/* scanline callback */
	NULL,							/* generate interrupt */
	coolpool_to_shiftreg,			/* write to shiftreg function */
	coolpool_from_shiftreg			/* read from shiftreg function */
};


static const tms34010_config tms_config_coolpool =
{
	FALSE,							/* halt on reset */
	"screen",						/* the screen operated on */
	XTAL_40MHz/6,					/* pixel clock */
	1,								/* pixels per clock */
	coolpool_scanline,				/* scanline callback */
	NULL,							/* generate interrupt */
	coolpool_to_shiftreg,			/* write to shiftreg function */
	coolpool_from_shiftreg			/* read from shiftreg function */
};



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static MACHINE_DRIVER_START( amerdart )

	MDRV_DRIVER_DATA( coolpool_state )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", TMS34010, XTAL_40MHz)
	MDRV_CPU_CONFIG(tms_config_amerdart)
	MDRV_CPU_PROGRAM_MAP(amerdart_map)

	MDRV_CPU_ADD("dsp", TMS32015, XTAL_40MHz/2)
	MDRV_CPU_PROGRAM_MAP(amerdart_dsp_pgm_map)
	/* Data Map is internal to the CPU */
	MDRV_CPU_IO_MAP(amerdart_dsp_io_map)
	MDRV_TIMER_ADD_SCANLINE("audioint", amerdart_audio_int_gen, "screen", 0, 1)

	MDRV_MACHINE_RESET(amerdart)
	MDRV_NVRAM_HANDLER(generic_0fill)

	MDRV_TIMER_ADD("nvram_timer", nvram_write_timeout)

	/* video hardware */
	MDRV_VIDEO_UPDATE(tms340x0)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_RAW_PARAMS(XTAL_40MHz/6, 212*2, 0, 161*2, 262, 0, 241)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("dac", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( coolpool )

	MDRV_DRIVER_DATA( coolpool_state )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", TMS34010, XTAL_40MHz)
	MDRV_CPU_CONFIG(tms_config_coolpool)
	MDRV_CPU_PROGRAM_MAP(coolpool_map)

	MDRV_CPU_ADD("dsp", TMS32026,XTAL_40MHz)
	MDRV_CPU_PROGRAM_MAP(coolpool_dsp_pgm_map)
	MDRV_CPU_IO_MAP(coolpool_dsp_io_map)

	MDRV_MACHINE_RESET(coolpool)
	MDRV_NVRAM_HANDLER(generic_0fill)

	MDRV_TIMER_ADD("nvram_timer", nvram_write_timeout)

	/* video hardware */
	MDRV_VIDEO_UPDATE(tms340x0)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_RAW_PARAMS(XTAL_40MHz/6, 424, 0, 320, 262, 0, 240)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("dac", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( 9ballsht )
	MDRV_IMPORT_FROM(coolpool)

	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(nballsht_map)
MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( amerdart ) /* You need to check the sum16 values listed on the labels to determine different sets */
	ROM_REGION16_LE( 0x0a0000, "user1", 0 )	/* 34010 code */
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u31 4e74", 0x000001, 0x10000, CRC(9628c422) SHA1(46b71acc746760962e34e9d7876f9499ea7d5c7c) )
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u32 0ef7", 0x000000, 0x10000, CRC(2d651ed0) SHA1(e2da2c3d8f25c17e26fd435c75983b2db8691993) )
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u38 10b4", 0x020001, 0x10000, CRC(1eb8c887) SHA1(220f566043535c54ad1cf2216966c7f42099e50b) )
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u39 f45a", 0x020000, 0x10000, CRC(2ab1ea68) SHA1(4e29a274c5c62b6ca92119eb320200beb784ca55) )
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u45 c1f9", 0x040001, 0x10000, CRC(74394375) SHA1(ceb7ae4e3253351da362cd0ada87702164005d17) )
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u46 1f84", 0x040000, 0x10000, CRC(1188047e) SHA1(249f25582ab72eeee37798418460de312053660e) )
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u52 cdfd", 0x060001, 0x10000, CRC(5ac2f06d) SHA1(b3a5d0cd94bdffdbf5bd17dbb30c07bfad3fa5d0) )
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u53 d432", 0x060000, 0x10000, CRC(4bd25cf0) SHA1(d1092cc3b6172d6567acd21f79b22043380102b7) )
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u57 6016", 0x080001, 0x10000, CRC(f620f935) SHA1(bf891fce1f04f3ad5b8b72d43d041ceacb0b65bc) ) /* Different then set 2 or 3 */
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u58 48af", 0x080000, 0x10000, CRC(f1b3d7c4) SHA1(7b897230d110be7a5eb05eda927d00561ebb9ce3) ) /* Different then set 2 or 3 */

	ROM_REGION( 0x10000, "dsp", 0 )	/* TMS32015 code  */
	ROM_LOAD16_WORD( "tms320e15.bin", 0x0000, 0x2000, CRC(375DB4EA) SHA1(11689C89CE62F44F43CB8973B4EC6E6B0024ED14) ) /* Passes internal checksum routine */

	ROM_REGION( 0x100000, "user2", 0 )				/* TMS32015 audio sample data */
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u1 4461",  0x000000, 0x10000, CRC(3f459482) SHA1(d9d489efd0d9217fceb3bf1a3b37a78d6823b4d9) ) /* Different then set 2 or 3 */
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u16 abd6", 0x010000, 0x10000, CRC(7437e8bf) SHA1(754be4822cd586590f09e706d7eb48e5ba8c8817) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u2 cae4",  0x020000, 0x10000, CRC(a587fffd) SHA1(f33f511d1bf1d6eb3c42535593a9718571174c4b) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u17 b791", 0x030000, 0x10000, CRC(e32bdd0f) SHA1(0662abbe84f0bad2631566b506ef016fcd79b9ee) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u3 046e",  0x040000, 0x10000, CRC(984d343a) SHA1(ee214830de4cb22d2d8e9d3ca335eff05af4abb6) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u18 100e", 0x050000, 0x10000, CRC(de3b4d7c) SHA1(68e7ffe2d84aef7c24d1787c4f9b6950c0107741) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u4 7887",  0x060000, 0x10000, CRC(c4765ff6) SHA1(7dca61d32300047ca1c089057e617553d60a0995) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u19 94cf", 0x070000, 0x10000, CRC(7109247c) SHA1(201809ec6599b30c26823bde6851b6eaa2589710) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u5 47cb",  0x080000, 0x10000, CRC(3b63b890) SHA1(a1223cb8884d5365af7d3f607657efff877f8845) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u20 6cd4", 0x090000, 0x10000, CRC(038b7d2d) SHA1(80bab18ca36d2bc101da7f3f6e1c82d8a802c14c) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u6 ef71",  0x0a0000, 0x10000, CRC(5cdb9aa9) SHA1(fae5d2c7f649bcba8068c8bc8266ee411258535e) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u21 fe8a", 0x0b0000, 0x10000, CRC(9b0b8978) SHA1(b31d0451ecd7085c191d20b2b41d0e8fe551996c) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u7 2671",  0x0c0000, 0x10000, CRC(147083a2) SHA1(c04c38145ab159bd519e6325477a3f7d0eebbda1) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u22 8fd7", 0x0d0000, 0x10000, CRC(4b92588a) SHA1(eea262c1a122015364a0046ff2bc7816f5f6821d) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u8 961c",  0x0e0000, 0x10000, CRC(975b368c) SHA1(1d637ce8c5d60833bb25aab2610e1a856720235e) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u23 abef", 0x0f0000, 0x10000, CRC(d7c2b13b) SHA1(3561e08011f649e4d0c47792745b2a014167e816) ) /* Different then set 2 or 3 */
ROM_END

ROM_START( amerdart2 ) /* You need to check the sum16 values listed on the labels to determine different sets */
	ROM_REGION16_LE( 0x0a0000, "user1", 0 )	/* 34010 code */
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u31 4e74", 0x000001, 0x10000, CRC(9628c422) SHA1(46b71acc746760962e34e9d7876f9499ea7d5c7c) )
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u32 0ef7", 0x000000, 0x10000, CRC(2d651ed0) SHA1(e2da2c3d8f25c17e26fd435c75983b2db8691993) )
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u38 10b4", 0x020001, 0x10000, CRC(1eb8c887) SHA1(220f566043535c54ad1cf2216966c7f42099e50b) )
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u39 f45a", 0x020000, 0x10000, CRC(2ab1ea68) SHA1(4e29a274c5c62b6ca92119eb320200beb784ca55) )
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u45 c1f9", 0x040001, 0x10000, CRC(74394375) SHA1(ceb7ae4e3253351da362cd0ada87702164005d17) )
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u46 1f84", 0x040000, 0x10000, CRC(1188047e) SHA1(249f25582ab72eeee37798418460de312053660e) )
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u52 cdfd", 0x060001, 0x10000, CRC(5ac2f06d) SHA1(b3a5d0cd94bdffdbf5bd17dbb30c07bfad3fa5d0) )
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u53 d432", 0x060000, 0x10000, CRC(4bd25cf0) SHA1(d1092cc3b6172d6567acd21f79b22043380102b7) )
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u57 1a0c", 0x080001, 0x10000, CRC(8a70f849) SHA1(dfd4cf90de2ab8cbeff458f0fd20110c1ed009e9) ) /* Different then set 1 or 3 */
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u58 0d81", 0x080000, 0x10000, CRC(8bb81975) SHA1(b7666572ab543991c7deaa0ebefb8b4526a7e386) ) /* Different then set 1 or 3 */

	ROM_REGION( 0x10000, "dsp", 0 )	/* TMS32015 code  */
	ROM_LOAD16_WORD( "tms320e15.bin", 0x0000, 0x2000, CRC(375DB4EA) SHA1(11689C89CE62F44F43CB8973B4EC6E6B0024ED14) ) /* Passes internal checksum routine */

	ROM_REGION( 0x100000, "user2", 0 )				/* TMS32015 audio sample data */
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u1 222f",  0x000000, 0x10000, CRC(e2bb7f54) SHA1(39eeb61a852b93331f445cc1c993727e52959660) ) /* Different then set 1 */
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u16 abd6", 0x010000, 0x10000, CRC(7437e8bf) SHA1(754be4822cd586590f09e706d7eb48e5ba8c8817) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u2 cae4",  0x020000, 0x10000, CRC(a587fffd) SHA1(f33f511d1bf1d6eb3c42535593a9718571174c4b) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u17 b791", 0x030000, 0x10000, CRC(e32bdd0f) SHA1(0662abbe84f0bad2631566b506ef016fcd79b9ee) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u3 046e",  0x040000, 0x10000, CRC(984d343a) SHA1(ee214830de4cb22d2d8e9d3ca335eff05af4abb6) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u18 100e", 0x050000, 0x10000, CRC(de3b4d7c) SHA1(68e7ffe2d84aef7c24d1787c4f9b6950c0107741) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u4 7887",  0x060000, 0x10000, CRC(c4765ff6) SHA1(7dca61d32300047ca1c089057e617553d60a0995) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u19 94cf", 0x070000, 0x10000, CRC(7109247c) SHA1(201809ec6599b30c26823bde6851b6eaa2589710) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u5 47cb",  0x080000, 0x10000, CRC(3b63b890) SHA1(a1223cb8884d5365af7d3f607657efff877f8845) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u20 6cd4", 0x090000, 0x10000, CRC(038b7d2d) SHA1(80bab18ca36d2bc101da7f3f6e1c82d8a802c14c) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u6 ef71",  0x0a0000, 0x10000, CRC(5cdb9aa9) SHA1(fae5d2c7f649bcba8068c8bc8266ee411258535e) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u21 fe8a", 0x0b0000, 0x10000, CRC(9b0b8978) SHA1(b31d0451ecd7085c191d20b2b41d0e8fe551996c) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u7 2671",  0x0c0000, 0x10000, CRC(147083a2) SHA1(c04c38145ab159bd519e6325477a3f7d0eebbda1) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u22 8fd7", 0x0d0000, 0x10000, CRC(4b92588a) SHA1(eea262c1a122015364a0046ff2bc7816f5f6821d) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u8 961c",  0x0e0000, 0x10000, CRC(975b368c) SHA1(1d637ce8c5d60833bb25aab2610e1a856720235e) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u23 b806", 0x0f0000, 0x10000, CRC(7c1e6f2e) SHA1(21ae530e4bd7c0c9f1a84f01f136c71952c8adc4) ) /* Different then set 1 */
ROM_END

ROM_START( amerdart3 ) /* You need to check the sum16 values listed on the labels to determine different sets */
	ROM_REGION16_LE( 0x0a0000, "user1", 0 )	/* 34010 code */
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u31 4e74", 0x000001, 0x10000, CRC(9628c422) SHA1(46b71acc746760962e34e9d7876f9499ea7d5c7c) )
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u32 0ef7", 0x000000, 0x10000, CRC(2d651ed0) SHA1(e2da2c3d8f25c17e26fd435c75983b2db8691993) )
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u38 10b4", 0x020001, 0x10000, CRC(1eb8c887) SHA1(220f566043535c54ad1cf2216966c7f42099e50b) )
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u39 f45a", 0x020000, 0x10000, CRC(2ab1ea68) SHA1(4e29a274c5c62b6ca92119eb320200beb784ca55) )
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u45 c1f9", 0x040001, 0x10000, CRC(74394375) SHA1(ceb7ae4e3253351da362cd0ada87702164005d17) )
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u46 1f84", 0x040000, 0x10000, CRC(1188047e) SHA1(249f25582ab72eeee37798418460de312053660e) )
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u52 cdfd", 0x060001, 0x10000, CRC(5ac2f06d) SHA1(b3a5d0cd94bdffdbf5bd17dbb30c07bfad3fa5d0) )
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u53 d432", 0x060000, 0x10000, CRC(4bd25cf0) SHA1(d1092cc3b6172d6567acd21f79b22043380102b7) )
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u57 4cac", 0x080001, 0x10000, CRC(2d653c7b) SHA1(0feebe6440aabe844049013aa063ed0259b7bec4) ) /* Different then set 2 or 3 */
	ROM_LOAD16_BYTE( "ameri corp copyright 1989 u58 729e", 0x080000, 0x10000, CRC(8cef479a) SHA1(80002e215416a11ff071523ee67218a1aabe155b) ) /* Different then set 2 or 3 */

	ROM_REGION( 0x10000, "dsp", 0 )	/* TMS32015 code  */
	ROM_LOAD16_WORD( "tms320e15.bin", 0x0000, 0x2000, CRC(375DB4EA) SHA1(11689C89CE62F44F43CB8973B4EC6E6B0024ED14) ) /* Passes internal checksum routine */

	ROM_REGION( 0x100000, "user2", 0 )				/* TMS32015 audio sample data */
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u1 222f",  0x000000, 0x10000, CRC(e2bb7f54) SHA1(39eeb61a852b93331f445cc1c993727e52959660) ) /* Same as set 2 */
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u16 abd6", 0x010000, 0x10000, CRC(7437e8bf) SHA1(754be4822cd586590f09e706d7eb48e5ba8c8817) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u2 cae4",  0x020000, 0x10000, CRC(a587fffd) SHA1(f33f511d1bf1d6eb3c42535593a9718571174c4b) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u17 b791", 0x030000, 0x10000, CRC(e32bdd0f) SHA1(0662abbe84f0bad2631566b506ef016fcd79b9ee) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u3 046e",  0x040000, 0x10000, CRC(984d343a) SHA1(ee214830de4cb22d2d8e9d3ca335eff05af4abb6) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u18 100e", 0x050000, 0x10000, CRC(de3b4d7c) SHA1(68e7ffe2d84aef7c24d1787c4f9b6950c0107741) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u4 7887",  0x060000, 0x10000, CRC(c4765ff6) SHA1(7dca61d32300047ca1c089057e617553d60a0995) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u19 94cf", 0x070000, 0x10000, CRC(7109247c) SHA1(201809ec6599b30c26823bde6851b6eaa2589710) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u5 47cb",  0x080000, 0x10000, CRC(3b63b890) SHA1(a1223cb8884d5365af7d3f607657efff877f8845) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u20 6cd4", 0x090000, 0x10000, CRC(038b7d2d) SHA1(80bab18ca36d2bc101da7f3f6e1c82d8a802c14c) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u6 ef71",  0x0a0000, 0x10000, CRC(5cdb9aa9) SHA1(fae5d2c7f649bcba8068c8bc8266ee411258535e) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u21 fe8a", 0x0b0000, 0x10000, CRC(9b0b8978) SHA1(b31d0451ecd7085c191d20b2b41d0e8fe551996c) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u7 2671",  0x0c0000, 0x10000, CRC(147083a2) SHA1(c04c38145ab159bd519e6325477a3f7d0eebbda1) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u22 8fd7", 0x0d0000, 0x10000, CRC(4b92588a) SHA1(eea262c1a122015364a0046ff2bc7816f5f6821d) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u8 961c",  0x0e0000, 0x10000, CRC(975b368c) SHA1(1d637ce8c5d60833bb25aab2610e1a856720235e) )
	ROM_LOAD16_WORD( "ameri corp copyright 1989 u23 b806", 0x0f0000, 0x10000, CRC(7c1e6f2e) SHA1(21ae530e4bd7c0c9f1a84f01f136c71952c8adc4) ) /* Same as set 2 */
ROM_END


ROM_START( coolpool )
	ROM_REGION16_LE( 0x40000, "user1", 0 )	/* 34010 code */
	ROM_LOAD16_BYTE( "u112b",        0x00000, 0x20000, CRC(aa227769) SHA1(488e357a7aad07369cade3110cde14ba8562c66c) )
	ROM_LOAD16_BYTE( "u113b",        0x00001, 0x20000, CRC(5b5f82f1) SHA1(82afb6a8d94cf09960b962d5208aab451b56feae) )

	ROM_REGION16_LE( 0x200000, "gfx1", 0 )	/* gfx data read by main CPU */
	ROM_LOAD16_BYTE( "u04",          0x000000, 0x20000, CRC(66a9940e) SHA1(7fa587280ecfad6b06194868de09cbdd57cf517f) )
	ROM_CONTINUE(                    0x100000, 0x20000 )
	ROM_LOAD16_BYTE( "u08",          0x000001, 0x20000, CRC(56789cf4) SHA1(5ad867d5029fdac9dccd01a6979171aa30d9a6eb) )
	ROM_CONTINUE(                    0x100001, 0x20000 )
	ROM_LOAD16_BYTE( "u03",          0x040000, 0x20000, CRC(02bc792a) SHA1(8085cff38868a307d6d29a7aadf3d6a99cbe85bb) )
	ROM_CONTINUE(                    0x140000, 0x20000 )
	ROM_LOAD16_BYTE( "u07",          0x040001, 0x20000, CRC(7b2fcb9f) SHA1(fa912663891bac6ba78519f030ba2c718e3514c3) )
	ROM_CONTINUE(                    0x140001, 0x20000 )
	ROM_LOAD16_BYTE( "u02",          0x080000, 0x20000, CRC(3b7d757d) SHA1(8737721764b181b050d776b2d2e1208419f8e5eb) )
	ROM_CONTINUE(                    0x180000, 0x20000 )
	ROM_LOAD16_BYTE( "u06",          0x080001, 0x20000, CRC(c09353a2) SHA1(f3588ec75b757232bdaa40d055e171a501122bfa) )
	ROM_CONTINUE(                    0x180001, 0x20000 )
	ROM_LOAD16_BYTE( "u01",          0x0c0000, 0x20000, CRC(948a5faf) SHA1(186ab3ab0ede168beaa4dae0cba753df10cdac46) )
	ROM_CONTINUE(                    0x1c0000, 0x20000 )
	ROM_LOAD16_BYTE( "u05",          0x0c0001, 0x20000, CRC(616965e2) SHA1(588ea3c5c7838c50b2157ff1074f629d9d85791c) )
	ROM_CONTINUE(                    0x1c0001, 0x20000 )

	ROM_REGION( 0x40000, "dsp", 0 )	/* TMS320C26 */
	ROM_LOAD16_BYTE( "u34",          0x00000, 0x08000, CRC(dc1df70b) SHA1(e42fa7e34e50e0bd2aaeea5c55d750ed3286610d) )
	ROM_LOAD16_BYTE( "u35",          0x00001, 0x08000, CRC(ac999431) SHA1(7e4c2dcaedcb7e7c67072a179e4b8488d2bbdac7) )

	ROM_REGION( 0x200000, "user2", 0 )	/* TMS32026 data */
	ROM_LOAD( "u17c",         0x000000, 0x40000, CRC(ea3cc41d) SHA1(e703e789dfbcfaec878a990031ce839164c51253) )
	ROM_LOAD( "u16c",         0x040000, 0x40000, CRC(2e6680ea) SHA1(cb30dc789039aab491428d075fee9e0bc04fd2ce) )
	ROM_LOAD( "u15c",         0x080000, 0x40000, CRC(8e5f248e) SHA1(a954d3c20dc0b70f83c4c238db30a33285fcb353) )
	ROM_LOAD( "u14c",         0x0c0000, 0x40000, CRC(dcd6cf71) SHA1(b1f53bffdd19f5da1d8664765d504568d1f5867c) )
	ROM_LOAD( "u13c",         0x100000, 0x40000, CRC(5a7fe750) SHA1(bbbd45380545cb0f17d9f6811b2a7300fa3b682d) )
	ROM_LOAD( "u12c",         0x140000, 0x40000, CRC(4f246958) SHA1(ee4446159635b6c44d88d8f6aac52787a89403c1) )
	ROM_LOAD( "u11c",         0x180000, 0x40000, CRC(92cd2b03) SHA1(e80df65f8ec5ed2178f623bdd975e2b01a12a184) )
	ROM_LOAD( "u10c",         0x1c0000, 0x40000, CRC(a3dbcae3) SHA1(af997f3f56f406d5eb9fa415e1672b2d129815b8) )
ROM_END


ROM_START( 9ballsht )
	ROM_REGION16_LE( 0x80000, "user1", 0 )	/* 34010 code */
	ROM_LOAD16_BYTE( "u112",         0x00000, 0x40000, CRC(b3855e59) SHA1(c3175df24b85897783169bcaccd61630e512f7f6) )
	ROM_LOAD16_BYTE( "u113",         0x00001, 0x40000, CRC(30cbf462) SHA1(64b2e2d40c2a92c4f4823dc866e5464792954ac3) )

	ROM_REGION16_LE( 0x100000, "gfx1", 0 )	/* gfx data read by main CPU */
	ROM_LOAD16_BYTE( "e-scape (c)1994 c316.u110", 0x00000, 0x80000, CRC(890ed5c0) SHA1(eaf06ee5b6c5ed0103b535396b4517012818a416) )
	ROM_LOAD16_BYTE( "e-scape (c)1994 13f2.u111", 0x00001, 0x80000, CRC(1a9f1145) SHA1(ba52a6d1aca26484c320518f69c66ce3ceb4adcf) )

	ROM_REGION( 0x40000, "dsp", 0 )	/* TMS320C26 */
	ROM_LOAD16_BYTE( "e-scape (c)1994 89bc.u34", 0x00000, 0x08000, CRC(dc1df70b) SHA1(e42fa7e34e50e0bd2aaeea5c55d750ed3286610d) )
	ROM_LOAD16_BYTE( "e-scape (c)1994 af4a.u35", 0x00001, 0x08000, CRC(ac999431) SHA1(7e4c2dcaedcb7e7c67072a179e4b8488d2bbdac7) )

	ROM_REGION( 0x100000, "user2", 0 )	/* TMS32026 data */
	ROM_LOAD( "u54",          0x00000, 0x80000, CRC(1be5819c) SHA1(308b5b1fe05634419d03956ae1b2e5a61206900f) )
	ROM_LOAD( "u53",          0x80000, 0x80000, CRC(d401805d) SHA1(f4bcb2bdc45c3bc5ca423e518cdea8b3a7e8d60e) )
ROM_END

/*
  all ROMs for this set were missing except for the main program,
  I assume the others are the same.
 */
ROM_START( 9ballsht2 )
	ROM_REGION16_LE( 0x80000, "user1", 0 )	/* 34010 code */
	ROM_LOAD16_BYTE( "e-scape.112",  0x00000, 0x40000, CRC(aee8114f) SHA1(a0d0e9e3a879393585b85ac6d04e31a7d4221179) )
	ROM_LOAD16_BYTE( "e-scape.113",  0x00001, 0x40000, CRC(ccd472a7) SHA1(d074080e987c233b26b3c72248411c575f7a2293) )

	ROM_REGION16_LE( 0x100000, "gfx1", 0 )	/* gfx data read by main CPU */
	ROM_LOAD16_BYTE( "e-scape (c)1994 c316.u110", 0x00000, 0x80000, CRC(890ed5c0) SHA1(eaf06ee5b6c5ed0103b535396b4517012818a416) )
	ROM_LOAD16_BYTE( "e-scape (c)1994 13f2.u111", 0x00001, 0x80000, CRC(1a9f1145) SHA1(ba52a6d1aca26484c320518f69c66ce3ceb4adcf) )

	ROM_REGION( 0x40000, "dsp", 0 )	/* TMS320C26 */
	ROM_LOAD16_BYTE( "e-scape (c)1994 89bc.u34", 0x00000, 0x08000, CRC(dc1df70b) SHA1(e42fa7e34e50e0bd2aaeea5c55d750ed3286610d) )
	ROM_LOAD16_BYTE( "e-scape (c)1994 af4a.u35", 0x00001, 0x08000, CRC(ac999431) SHA1(7e4c2dcaedcb7e7c67072a179e4b8488d2bbdac7) )

	ROM_REGION( 0x100000, "user2", 0 )	/* TMS32026 data */
	ROM_LOAD( "u54",          0x00000, 0x80000, CRC(1be5819c) SHA1(308b5b1fe05634419d03956ae1b2e5a61206900f) )
	ROM_LOAD( "u53",          0x80000, 0x80000, CRC(d401805d) SHA1(f4bcb2bdc45c3bc5ca423e518cdea8b3a7e8d60e) )
ROM_END

ROM_START( 9ballsht3 )
	ROM_REGION16_LE( 0x80000, "user1", 0 )	/* 34010 code */
	ROM_LOAD16_BYTE( "8e_1826.112",  0x00000, 0x40000, CRC(486f7a8b) SHA1(635e3b1e7a21a86dd3d0ea994e9b923b06df587e) )
	ROM_LOAD16_BYTE( "8e_6166.113",  0x00001, 0x40000, CRC(c41db70a) SHA1(162112f9f5bb6345920a45c41da6a249796bd21f) )

	ROM_REGION16_LE( 0x100000, "gfx1", 0 )	/* gfx data read by main CPU */
	ROM_LOAD16_BYTE( "e-scape (c)1994 c316.u110", 0x00000, 0x80000, CRC(890ed5c0) SHA1(eaf06ee5b6c5ed0103b535396b4517012818a416) )
	ROM_LOAD16_BYTE( "e-scape (c)1994 13f2.u111", 0x00001, 0x80000, CRC(1a9f1145) SHA1(ba52a6d1aca26484c320518f69c66ce3ceb4adcf) )

	ROM_REGION( 0x40000, "dsp", 0 )	/* TMS320C26 */
	ROM_LOAD16_BYTE( "e-scape (c)1994 89bc.u34", 0x00000, 0x08000, CRC(dc1df70b) SHA1(e42fa7e34e50e0bd2aaeea5c55d750ed3286610d) )
	ROM_LOAD16_BYTE( "e-scape (c)1994 af4a.u35", 0x00001, 0x08000, CRC(ac999431) SHA1(7e4c2dcaedcb7e7c67072a179e4b8488d2bbdac7) )

	ROM_REGION( 0x100000, "user2", 0 )	/* TMS32026 data */
	ROM_LOAD( "u54",          0x00000, 0x80000, CRC(1be5819c) SHA1(308b5b1fe05634419d03956ae1b2e5a61206900f) )
	ROM_LOAD( "u53",          0x80000, 0x80000, CRC(d401805d) SHA1(f4bcb2bdc45c3bc5ca423e518cdea8b3a7e8d60e) )
ROM_END


// all checksums correctly match sum16 printed on rom labels
ROM_START( 9ballshtc )
	ROM_REGION16_LE( 0x80000, "user1", 0 )	/* 34010 code */
	ROM_LOAD16_BYTE( "e-scape (c)1994 3990.u112",  0x00000, 0x40000, CRC(7ba2749a) SHA1(e2ddc2600234dbebbb423f201cc4061fd0b9911a) )
	ROM_LOAD16_BYTE( "e-scape (c)1994 b72f.u113",  0x00001, 0x40000, CRC(1e0f3c62) SHA1(3c24a38dcb553fd84b0b44a5a8d93a14435e22b0) )

	ROM_REGION16_LE( 0x100000, "gfx1", 0 )	/* gfx data read by main CPU */
	ROM_LOAD16_BYTE( "e-scape (c)1994 c316.u110", 0x00000, 0x80000, CRC(890ed5c0) SHA1(eaf06ee5b6c5ed0103b535396b4517012818a416) )
	ROM_LOAD16_BYTE( "e-scape (c)1994 13f2.u111", 0x00001, 0x80000, CRC(1a9f1145) SHA1(ba52a6d1aca26484c320518f69c66ce3ceb4adcf) )

	ROM_REGION( 0x40000, "dsp", 0 )	/* TMS320C26 */
	ROM_LOAD16_BYTE( "e-scape (c)1994 89bc.u34", 0x00000, 0x08000, CRC(dc1df70b) SHA1(e42fa7e34e50e0bd2aaeea5c55d750ed3286610d) )
	ROM_LOAD16_BYTE( "e-scape (c)1994 af4a.u35", 0x00001, 0x08000, CRC(ac999431) SHA1(7e4c2dcaedcb7e7c67072a179e4b8488d2bbdac7) )

	ROM_REGION( 0x100000, "user2", 0 )	/* TMS32026 data */
	ROM_LOAD( "e-scape (c)1994 0000.u54", 0x00000, 0x80000, CRC(04b509a0) SHA1(093343741a3d8d0786fd443e68dd85b414c6cf9e) )
	ROM_LOAD( "e-scape (c)1994 2df8.u53", 0x80000, 0x80000, CRC(c8a7b576) SHA1(7eb71dd791fdcbfe71764a454f0a1d3130d8a57e) )
ROM_END




/*************************************
 *
 *  Driver init
 *
 *************************************/

static void register_state_save(running_machine *machine)
{
	coolpool_state *state = (coolpool_state *)machine->driver_data;

	state_save_register_global_array(machine, state->oldx);
	state_save_register_global_array(machine, state->oldy);
	state_save_register_global(machine, state->result);
	state_save_register_global(machine, state->lastresult);

	state_save_register_global(machine, state->cmd_pending);
	state_save_register_global(machine, state->iop_cmd);
	state_save_register_global(machine, state->iop_answer);
	state_save_register_global(machine, state->iop_romaddr);
}



static DRIVER_INIT( amerdart )
{
	coolpool_state *state = (coolpool_state *)machine->driver_data;

	state->lastresult = 0xffff;

	register_state_save(machine);
}

static DRIVER_INIT( coolpool )
{
	memory_install_read16_handler(cputag_get_address_space(machine, "dsp", ADDRESS_SPACE_IO), 0x07, 0x07, 0, 0, coolpool_input_r);

	register_state_save(machine);
}


static DRIVER_INIT( 9ballsht )
{
	int a, len;
	UINT16 *rom;

	/* decrypt the main program ROMs */
	rom = (UINT16 *)memory_region(machine, "user1");
	len = memory_region_length(machine, "user1");
	for (a = 0;a < len/2;a++)
	{
		int hi,lo,nhi,nlo;

		hi = rom[a] >> 8;
		lo = rom[a] & 0xff;

		nhi = BITSWAP8(hi,5,2,0,7,6,4,3,1) ^ 0x29;
		if (hi & 0x01) nhi ^= 0x03;
		if (hi & 0x10) nhi ^= 0xc1;
		if (hi & 0x20) nhi ^= 0x40;
		if (hi & 0x40) nhi ^= 0x12;

		nlo = BITSWAP8(lo,5,3,4,6,7,1,2,0) ^ 0x80;
		if ((lo & 0x02) && (lo & 0x04)) nlo ^= 0x01;
		if (lo & 0x04) nlo ^= 0x0c;
		if (lo & 0x08) nlo ^= 0x10;

		rom[a] = (nhi << 8) | nlo;
	}

	/* decrypt the sub data ROMs */
	rom = (UINT16 *)memory_region(machine, "user2");
	len = memory_region_length(machine, "user2");
	for (a = 1;a < len/2;a+=4)
	{
		/* just swap bits 1 and 2 of the address */
		UINT16 tmp = rom[a];
		rom[a] = rom[a+1];
		rom[a+1] = tmp;
	}

	register_state_save(machine);
}



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1989, amerdart,  0,        amerdart, amerdart, amerdart, ROT0, "Ameri",    "AmeriDarts (set 1)", GAME_SUPPORTS_SAVE )
GAME( 1989, amerdart2, amerdart, amerdart, amerdart, amerdart, ROT0, "Ameri",    "AmeriDarts (set 2)", GAME_SUPPORTS_SAVE )
GAME( 1989, amerdart3, amerdart, amerdart, amerdart, amerdart, ROT0, "Ameri",    "AmeriDarts (set 3)", GAME_SUPPORTS_SAVE )
GAME( 1992, coolpool,  0,        coolpool, coolpool, coolpool, ROT0, "Catalina", "Cool Pool", 0 )
GAME( 1993, 9ballsht,  0,        9ballsht, 9ballsht, 9ballsht, ROT0, "E-Scape EnterMedia (Bundra license)", "9-Ball Shootout (set 1)", 0 )
GAME( 1993, 9ballsht2, 9ballsht, 9ballsht, 9ballsht, 9ballsht, ROT0, "E-Scape EnterMedia (Bundra license)", "9-Ball Shootout (set 2)", 0 )
GAME( 1993, 9ballsht3, 9ballsht, 9ballsht, 9ballsht, 9ballsht, ROT0, "E-Scape EnterMedia (Bundra license)", "9-Ball Shootout (set 3)", 0 )
GAME( 1993, 9ballshtc, 9ballsht, 9ballsht, 9ballsht, 9ballsht, ROT0, "E-Scape EnterMedia (Bundra license)", "9-Ball Shootout Championship", 0 )
