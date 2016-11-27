/*********************************************************************************************************************

Mahjong Dunhuang (C)1995 Spirit

driver by Luca Elia

PCB Layout
----------

|---------------------------------------|
|uPD1242H  VOL     UM3567  AR17961  ROM6|
|          3.579545MHz                  |
|                  DSW1(8)          ROM5|
|   VOL            DSW2(8)              |
|                  DSW3(8)          ROM4|
|                  DSW4(8)              |
|                  DSW5(8)          ROM3|
|1   WF19054                            |
|8                                  ROM2|
|W                      |-------|       |
|A                      |       |       |
|Y                      |  *    |       |
|                       |       |       |
|                       |-------|       |
|                    12MHz         6264 |
|                                       |
|                                       |
|1         PAL                     6264 |
|0                                      |
|W                     Z80              |
|A  HM86171-80   BATTERY           6264 |
|Y                     PAL              |
|          PAL         PAL          ROM1|
|---------------------------------------|
Notes:
      Uses common 10-way/18-way Mahjong pinout
      Z80     - clock 6.000MHz [12/2]
      WF19054 - == AY3-8910. Clock = 1.500MHz [12/8]
      UM3567  - == YM2413. Clock 3.579545MHz
      HM86171 - Hualon Microelectronics HMC HM86171 VGA 256 colour RAMDAC (DIP28)
      6264    - UT6264PC-70LL 8k x8 SRAM (DIP28)
      *       - QFP120 IC marked 55102602-12. Logo on the chip is of two 5 1/4 inch floppy
                discs standing upright and next to each other, with the corner facing up,
                like 2 pyramids. The middle hole and head access slot are visible too.
                Chip was manufactured 50th week of 1993
      AR17961 - == Oki M6295 (QFP44). Clock = 1.500MHz [12/8]. pin 7 = high
      VSync   - 60Hz
      HSync   - 15.28kHz

*********************************************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "sound/okim6295.h"
#include "sound/2413intf.h"

#define DUNHUANG_DEBUG	0


class dunhuang_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, dunhuang_state(machine)); }

	dunhuang_state(running_machine &machine) { }

	/* memory pointers */
	UINT16 *        videoram;
	UINT16 *        videoram2;
	UINT8	 *        colorram;
	UINT8  *        colorram2;
	UINT8  *        paldata;

	/* video-related */
	tilemap_t         *tmap,*tmap2;
	int             written, written2;
	UINT8           pos_x, pos_y, clear_y;
	UINT8           block_x, block_y, block_w, block_h;
	UINT8           block_addr_hi, block_addr_lo;
	UINT8           block_dest;
	UINT8           block_c;
	UINT8           layers;
	int             paloffs;

	/* input-related */
	UINT8           input;
	UINT8           hopper;
};


/***************************************************************************
                                Video Hardware
***************************************************************************/


static TILE_GET_INFO( get_tile_info )
{
	dunhuang_state *state = (dunhuang_state *)machine->driver_data;
	UINT16 code = state->videoram[tile_index];
	UINT8 color = state->colorram[tile_index] & 0x0f;
	SET_TILE_INFO(0, code, color, 0);
}
static TILE_GET_INFO( get_tile_info2 )
{
	dunhuang_state *state = (dunhuang_state *)machine->driver_data;
	UINT16 code = state->videoram2[tile_index];
	UINT8 color = state->colorram2[tile_index] & 0x0f;
	SET_TILE_INFO(1, code, color, 0);
}

static VIDEO_START(dunhuang)
{
	dunhuang_state *state = (dunhuang_state *)machine->driver_data;
	state->tmap = tilemap_create(machine, get_tile_info, tilemap_scan_rows, 8,8, 0x40,0x20);
	state->tmap2 = tilemap_create(machine, get_tile_info2, tilemap_scan_rows, 8,32, 0x40,0x8);

	tilemap_set_transparent_pen(state->tmap, 0);
	tilemap_set_transparent_pen(state->tmap2, 0);

	state->videoram = auto_alloc_array(machine, UINT16, 0x40 * 0x20);
	state->colorram = auto_alloc_array(machine, UINT8, 0x40 * 0x20);

	state->videoram2 = auto_alloc_array(machine, UINT16, 0x40 * 0x8);
	state->colorram2 = auto_alloc_array(machine, UINT8, 0x40 * 0x8);

	state->paldata  = auto_alloc_array(machine, UINT8, 3 * 256);

	state_save_register_global_pointer(machine, state->videoram, 0x40 * 0x20);
	state_save_register_global_pointer(machine, state->colorram, 0x40 * 0x20);
	state_save_register_global_pointer(machine, state->videoram2, 0x40 * 0x8);
	state_save_register_global_pointer(machine, state->colorram2, 0x40 * 0x8);
	state_save_register_global_pointer(machine, state->paldata, 3 * 256);
}

static VIDEO_UPDATE( dunhuang )
{
	dunhuang_state *state = (dunhuang_state *)screen->machine->driver_data;
	int layers_ctrl = -1;

#if DUNHUANG_DEBUG
if (input_code_pressed(screen->machine, KEYCODE_Z))
{
	int msk = 0;
	if (input_code_pressed(screen->machine, KEYCODE_Q))	msk |= 1;
	if (input_code_pressed(screen->machine, KEYCODE_W))	msk |= 2;
	if (msk != 0) layers_ctrl &= msk;
}
#endif

	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));

	switch (state->layers)
	{
		case 0x04:	// girl select: bg over fg
			if (layers_ctrl & 2)	tilemap_draw(bitmap,cliprect, state->tmap2, TILEMAP_DRAW_OPAQUE, 0);
			if (layers_ctrl & 1)	tilemap_draw(bitmap,cliprect, state->tmap,  0, 0);
			break;
		case 0x05:	// dips: must hide fg
			if (layers_ctrl & 1)	tilemap_draw(bitmap,cliprect, state->tmap,  TILEMAP_DRAW_OPAQUE, 0);
			break;
		case 0x07:	// game,demo: fg over bg
		default:
			if (layers_ctrl & 1)	tilemap_draw(bitmap,cliprect, state->tmap,  TILEMAP_DRAW_OPAQUE, 0);
			if (layers_ctrl & 2)	tilemap_draw(bitmap,cliprect, state->tmap2, 0, 0);
			break;
	}

	return 0;
}

// Tilemaps access

static WRITE8_HANDLER( dunhuang_pos_x_w )
{
	dunhuang_state *state = (dunhuang_state *)space->machine->driver_data;
	state->pos_x = data & 0x3f;
	state->written = 0;
	state->written2 = 0;
}

static WRITE8_HANDLER( dunhuang_pos_y_w )
{
	dunhuang_state *state = (dunhuang_state *)space->machine->driver_data;
	state->pos_y = data;
	state->written = 0;
	state->written2 = 0;
}

static WRITE8_HANDLER( dunhuang_tile_w )
{
	dunhuang_state *state = (dunhuang_state *)space->machine->driver_data;
	int addr;

	if (state->written & (1 << offset))
	{
		state->written = 0;
		state->pos_x++;
		if (state->pos_x == 0x40)
		{
			state->pos_x = 0;
			state->pos_y++;
		}
	}
	state->written |= 1 << offset;

	addr = (state->pos_x & 0x3f) + (state->pos_y & 0x1f) * 0x40;
	switch (offset)
	{
		case 0:	state->videoram[addr] = (state->videoram[addr] & 0xff00) | data;		break;
		case 1:	state->videoram[addr] = (state->videoram[addr] & 0x00ff) | (data<<8);	break;
		case 2:	state->colorram[addr] = data;												break;
	}
	tilemap_mark_tile_dirty(state->tmap, addr);
}

static WRITE8_HANDLER( dunhuang_tile2_w )
{
	dunhuang_state *state = (dunhuang_state *)space->machine->driver_data;
	int addr;

	if (state->written2 & (1 << offset))
	{
		state->written2 = 0;
		state->pos_x++;
		if (state->pos_x == 0x40)
		{
			state->pos_x = 0;
			state->pos_y++;
		}
	}
	state->written2 |= 1 << offset;

	addr = (state->pos_x & 0x3f) + (state->pos_y & 0x07) * 0x40;
	switch (offset)
	{
		case 0:	state->videoram2[addr] = (state->videoram2[addr] & 0xff00) | data;		break;
		case 1:	state->videoram2[addr] = (state->videoram2[addr] & 0x00ff) | (data<<8);	break;
		case 2:	state->colorram2[addr] = data;											break;
	}
	tilemap_mark_tile_dirty(state->tmap2, addr);
}

// Clear a row of tiles (videoram)

static WRITE8_HANDLER( dunhuang_clear_y_w )
{
	dunhuang_state *state = (dunhuang_state *)space->machine->driver_data;
	state->clear_y = data;
}
static WRITE8_HANDLER( dunhuang_horiz_clear_w )
{
	dunhuang_state *state = (dunhuang_state *)space->machine->driver_data;
	int i;
//  logerror("%06x: horiz clear, y = %02x, data = %02d\n", cpu_get_pc(space->cpu), state->clear_y,data);
	for (i = 0; i < 0x40; i++)
	{
		int addr = state->clear_y * 0x40 + i;

		state->videoram[addr] = 0;
		state->colorram[addr] = 0;
		tilemap_mark_tile_dirty(state->tmap, addr);
	}
}

// Clear a column of tiles (videoram2)

static WRITE8_HANDLER( dunhuang_vert_clear_w )
{
	dunhuang_state *state = (dunhuang_state *)space->machine->driver_data;
	int i;
//  logerror("%06x: vert clear, x = %02x, y = %02x, data = %02x\n", cpu_get_pc(space->cpu), state->pos_x,state->pos_y,data);
	for (i = 0; i < 0x08; i++)
	{
		int addr = (state->pos_x & 0x3f) + (i & 0x07) * 0x40;

		state->videoram2[addr] = 1;
		state->colorram2[addr] = 0;
		tilemap_mark_tile_dirty(state->tmap2, addr);
	}
}


// Draw a block of tiles.
//
// The tiles codes are read from the graphics roms too!
//

static WRITE8_HANDLER( dunhuang_block_dest_w )
{
	dunhuang_state *state = (dunhuang_state *)space->machine->driver_data;
	state->block_dest = data;
}

static WRITE8_HANDLER( dunhuang_block_x_w )
{
	dunhuang_state *state = (dunhuang_state *)space->machine->driver_data;
	state->block_x = data;
}

static WRITE8_HANDLER( dunhuang_block_y_w )
{
	dunhuang_state *state = (dunhuang_state *)space->machine->driver_data;
	state->block_y = data;
}

static WRITE8_HANDLER( dunhuang_block_w_w )
{
	dunhuang_state *state = (dunhuang_state *)space->machine->driver_data;
	state->block_w = data;
}

static WRITE8_HANDLER( dunhuang_block_c_w )
{
	dunhuang_state *state = (dunhuang_state *)space->machine->driver_data;
	state->block_c = data;
}

static WRITE8_HANDLER( dunhuang_block_addr_lo_w )
{
	dunhuang_state *state = (dunhuang_state *)space->machine->driver_data;
	state->block_addr_lo = data;
}

static WRITE8_HANDLER( dunhuang_block_addr_hi_w )
{
	dunhuang_state *state = (dunhuang_state *)space->machine->driver_data;
	state->block_addr_hi = data;
}


static WRITE8_HANDLER( dunhuang_block_h_w )
{
	dunhuang_state *state = (dunhuang_state *)space->machine->driver_data;
	int i,j, addr;
	UINT8 *tile_addr;

//  logerror("%06x: block dst %x, src %x, xy %x %x, wh %x %x, clr %x\n", cpu_get_pc(space->cpu), state->block_dest, (state->block_addr_hi << 8) + state->block_addr_lo, state->block_x,state->block_y,state->block_w+1,state->block_h+1,state->block_c);

	state->block_h = data;

	tile_addr = memory_region(space->machine, "gfx2") + ((state->block_addr_hi << 8) + state->block_addr_lo) * 4;

	switch (state->block_dest)
	{
		case 0x04:	// write to videoram
			for (j = 0; j <= state->block_h; j++)
			{
				for (i = 0; i <= state->block_w; i++)
				{
					addr = ((state->block_x + i)& 0x3f) + ((state->block_y + j) & 0x1f) * 0x40;

					state->videoram[addr] = (tile_addr[1] << 8) | tile_addr[0];
					state->colorram[addr] = state->block_c;
					tilemap_mark_tile_dirty(state->tmap, addr);
					tile_addr += 4;
				}
			}
			break;

		case 0x08:	// write to videoram2
			for (j = 0; j <= state->block_h; j++)
			{
				for (i = 0; i <= state->block_w; i++)
				{
					addr = ((state->block_x + i)& 0x3f) + ((state->block_y + j) & 0x7) * 0x40;

					state->videoram2[addr] = (tile_addr[1] << 8) | tile_addr[0];
					state->colorram2[addr] = state->block_c;
					tilemap_mark_tile_dirty(state->tmap2, addr);
					tile_addr += 4;
				}
			}
			break;

		default:
			popmessage("%06x: block dst=%x", cpu_get_pc(space->cpu), state->block_dest);
	}
}

// Palette: HMC HM86171 VGA 256 colour RAMDAC

static WRITE8_HANDLER( dunhuang_paloffs_w )
{
	dunhuang_state *state = (dunhuang_state *)space->machine->driver_data;
	state->paloffs = data * 3;
}

static WRITE8_HANDLER( dunhuang_paldata_w )
{
	dunhuang_state *state = (dunhuang_state *)space->machine->driver_data;
	state->paldata[state->paloffs] = data;

	palette_set_color_rgb( space->machine, state->paloffs/3,
		pal6bit(state->paldata[(state->paloffs/3)*3+0]),
		pal6bit(state->paldata[(state->paloffs/3)*3+1]),
		pal6bit(state->paldata[(state->paloffs/3)*3+2])
	);

	state->paloffs = (state->paloffs + 1) % (3*256);
}

// Layers control (not understood)

static WRITE8_HANDLER( dunhuang_layers_w )
{
	dunhuang_state *state = (dunhuang_state *)space->machine->driver_data;
//  popmessage("layers %02x",data);
	state->layers = data;
}

/***************************************************************************
                                Memory Maps
***************************************************************************/

static ADDRESS_MAP_START( dunhuang_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE( 0x0000, 0x5fff ) AM_ROM
	AM_RANGE( 0x6000, 0x7fff ) AM_RAM
	AM_RANGE( 0x8000, 0xffff ) AM_ROMBANK( "bank1" )
ADDRESS_MAP_END

// Inputs

static WRITE8_HANDLER( dunhuang_input_w )
{
	dunhuang_state *state = (dunhuang_state *)space->machine->driver_data;
	state->input = data;
}

static READ8_HANDLER( dunhuang_service_r )
{
	dunhuang_state *state = (dunhuang_state *)space->machine->driver_data;
	return input_port_read(space->machine, "SERVICE")
	 | ((state->hopper && !(space->machine->primary_screen->frame_number() % 10)) ? 0x00 : 0x08)	// bit 3: hopper sensor
	 | 0x80																// bit 7 low -> tiles block transferrer busy
	;
}

static READ8_DEVICE_HANDLER( dunhuang_dsw_r )
{
	dunhuang_state *state = (dunhuang_state *)device->machine->driver_data;
	if (!(state->input & 0x01))	return input_port_read(device->machine, "DSW1");
	if (!(state->input & 0x02))	return input_port_read(device->machine, "DSW2");
	if (!(state->input & 0x04))	return input_port_read(device->machine, "DSW3");
	if (!(state->input & 0x08))	return input_port_read(device->machine, "DSW4");
	if (!(state->input & 0x10))	return input_port_read(device->machine, "DSW5");
	logerror("%s: warning, unknown dsw bits read, state->input = %02x\n", cpuexec_describe_context(device->machine), state->input);
	return 0xff;
}
static READ8_HANDLER( dunhuang_input_r )
{
	dunhuang_state *state = (dunhuang_state *)space->machine->driver_data;
	if (!(state->input & 0x01))	return input_port_read(space->machine, "IN0");
	if (!(state->input & 0x02))	return input_port_read(space->machine, "IN1");
	if (!(state->input & 0x04))	return input_port_read(space->machine, "IN2");
	if (!(state->input & 0x08))	return input_port_read(space->machine, "IN3");
	if (!(state->input & 0x10))	return input_port_read(space->machine, "IN4");
	logerror("%s: warning, unknown input bits read, state->input = %02x\n", cpuexec_describe_context(space->machine), state->input);
	return 0xff;
}

static WRITE8_HANDLER( dunhuang_rombank_w )
{
	dunhuang_state *state = (dunhuang_state *)space->machine->driver_data;

	// ?                data & 0x01
	// ?                data & 0x02

	memory_set_bank(space->machine, "bank1", ((data >> 2) & 0x7));

	// COIN OUT:        data & 0x20
	coin_counter_w(space->machine, 0,	data & 0x40);
	state->hopper = data & 0x80;
}


#ifdef UNUSED_FUNCTION
static WRITE8_HANDLER( dunhuang_82_w )
{
//  popmessage("82 = %02x",dunhuang_82);
}
#endif

static ADDRESS_MAP_START( dunhuang_io_map, ADDRESS_SPACE_IO, 8 )
	AM_RANGE( 0x0000, 0x0000 ) AM_WRITE( dunhuang_pos_x_w )
	AM_RANGE( 0x0001, 0x0001 ) AM_WRITE( dunhuang_pos_y_w )
	AM_RANGE( 0x0002, 0x0004 ) AM_WRITE( dunhuang_tile_w )
	AM_RANGE( 0x0005, 0x0007 ) AM_WRITE( dunhuang_tile2_w )

	AM_RANGE( 0x0008, 0x0008 ) AM_WRITE( dunhuang_vert_clear_w )

	AM_RANGE( 0x000c, 0x000c ) AM_READ( watchdog_reset_r )

	AM_RANGE( 0x000f, 0x000f ) AM_WRITE( dunhuang_block_addr_lo_w )
	AM_RANGE( 0x0010, 0x0010 ) AM_WRITE( dunhuang_block_addr_hi_w )
//  AM_RANGE( 0x0011, 0x0011 ) ?
	AM_RANGE( 0x0012, 0x0012 ) AM_WRITE( dunhuang_block_c_w )
	AM_RANGE( 0x0015, 0x0015 ) AM_WRITE( dunhuang_block_x_w )
	AM_RANGE( 0x0016, 0x0016 ) AM_WRITE( dunhuang_block_y_w )
	AM_RANGE( 0x0017, 0x0017 ) AM_WRITE( dunhuang_block_w_w )
	AM_RANGE( 0x0018, 0x0018 ) AM_WRITE( dunhuang_block_h_w )

	AM_RANGE( 0x0019, 0x0019 ) AM_WRITE( dunhuang_clear_y_w )
	AM_RANGE( 0x001a, 0x001a ) AM_WRITE( dunhuang_horiz_clear_w )

	AM_RANGE( 0x001b, 0x001b ) AM_WRITE( dunhuang_block_dest_w )

	AM_RANGE( 0x0081, 0x0081 ) AM_DEVWRITE( "ymsnd", ym2413_register_port_w	)
	AM_RANGE( 0x0089, 0x0089 ) AM_DEVWRITE( "ymsnd", ym2413_data_port_w		)

//  AM_RANGE( 0x0082, 0x0082 ) AM_WRITE( dunhuang_82_w )

	AM_RANGE( 0x0083, 0x0083 ) AM_WRITE( dunhuang_paloffs_w )
	AM_RANGE( 0x008b, 0x008b ) AM_WRITE( dunhuang_paldata_w )

	AM_RANGE( 0x0084, 0x0084 ) AM_READ( dunhuang_service_r )
	AM_RANGE( 0x0085, 0x0085 ) AM_READ( dunhuang_input_r )

	AM_RANGE( 0x0086, 0x0086 ) AM_WRITE( dunhuang_rombank_w )
	AM_RANGE( 0x0087, 0x0087 ) AM_WRITE( dunhuang_layers_w )

	AM_RANGE( 0x0088, 0x0088 ) AM_DEVREAD( "ay8910", ay8910_r )
	AM_RANGE( 0x0090, 0x0090 ) AM_DEVWRITE( "ay8910", ay8910_data_w )
	AM_RANGE( 0x0098, 0x0098 ) AM_DEVWRITE( "ay8910", ay8910_address_w )
ADDRESS_MAP_END



/***************************************************************************
                                Input Ports
***************************************************************************/

static INPUT_PORTS_START( dunhuang )
	PORT_START("DSW1")		/* IN0 - DSW1 */
	PORT_DIPNAME( 0x0f, 0x0f, "Main Game Chance (%)" )	PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x00, "78" )
	PORT_DIPSETTING(    0x01, "80" )
	PORT_DIPSETTING(    0x02, "81" )
	PORT_DIPSETTING(    0x03, "83" )
	PORT_DIPSETTING(    0x04, "84" )
	PORT_DIPSETTING(    0x05, "86" )
	PORT_DIPSETTING(    0x06, "87" )
	PORT_DIPSETTING(    0x07, "89" )
	PORT_DIPSETTING(    0x08, "90" )
	PORT_DIPSETTING(    0x09, "92" )
	PORT_DIPSETTING(    0x0a, "93" )
	PORT_DIPSETTING(    0x0b, "94" )
	PORT_DIPSETTING(    0x0c, "95" )
	PORT_DIPSETTING(    0x0d, "96" )
	PORT_DIPSETTING(    0x0e, "97" )
	PORT_DIPSETTING(    0x0f, "98" )
	PORT_DIPNAME( 0x30, 0x30, "Main Game Rate" )		PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x30, "A" )
	PORT_DIPSETTING(    0x20, "B" )
	PORT_DIPSETTING(    0x10, "C" )
	PORT_DIPSETTING(    0x00, "D" )
	PORT_DIPNAME( 0x40, 0x40, "Input Tokens" )			PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, "Keys" )
	PORT_DIPSETTING(    0x00, "Coins" )
	PORT_DIPNAME( 0x80, 0x80, "Output Tokens" )			PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, "Keys" )
	PORT_DIPSETTING(    0x00, "Payout" )

	PORT_START("DSW2")		/* IN1 - DSW2 */
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW2:1" )
	PORT_DIPNAME( 0x06, 0x06, "Credits Per Coin" )		PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x06, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x18, 0x18, "Credits Per Key-In" )	PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "1" )
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPSETTING(    0x08, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x20, 0x20, "Credits Per Key-Out" )	PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0xc0, 0xc0, "Max Credits" )			PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x00, "300" )
	PORT_DIPSETTING(    0x40, "500" )
	PORT_DIPSETTING(    0x80, "1000" )
	PORT_DIPSETTING(    0xc0, "3000" )

	PORT_START("DSW3")		/* IN2 - DSW3 */
	PORT_DIPNAME( 0x03, 0x03, "Min Bet" )				PORT_DIPLOCATION("SW3:1,2")
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x0c, 0x0c, "Max Bet" )				PORT_DIPLOCATION("SW3:3,4")
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x04, "10" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x30, 0x30, "Renso Bonus Time" )		PORT_DIPLOCATION("SW3:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x30, "6" )
	PORT_DIPNAME( 0xc0, 0xc0, "DonDen Times" )			PORT_DIPLOCATION("SW3:7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPSETTING(    0x40, "3" )
	PORT_DIPSETTING(    0x80, "5" )
	PORT_DIPSETTING(    0xc0, "6" )

	PORT_START("DSW4")		/* IN3 - DSW4 */
	PORT_DIPNAME( 0x07, 0x07, "Credits Limit" )			PORT_DIPLOCATION("SW4:1,2,3")
	PORT_DIPSETTING(    0x07, "2k" )
	PORT_DIPSETTING(    0x06, "3k" )
	PORT_DIPSETTING(    0x05, "5k" )
	PORT_DIPSETTING(    0x04, "10k" )
	PORT_DIPSETTING(    0x03, "20k" )
	PORT_DIPSETTING(    0x02, "30k" )
	PORT_DIPSETTING(    0x01, "50k" )
	PORT_DIPSETTING(    0x00, "100k" )
	PORT_DIPNAME( 0x38, 0x38, "Service-In Limit" )		PORT_DIPLOCATION("SW4:4,5,6")
	PORT_DIPSETTING(    0x00, "20k" )
	PORT_DIPSETTING(    0x08, "30k" )
	PORT_DIPSETTING(    0x10, "40k" )
	PORT_DIPSETTING(    0x18, "50k" )
	PORT_DIPSETTING(    0x20, "70k" )
	PORT_DIPSETTING(    0x28, "100k" )
	PORT_DIPSETTING(    0x30, "200k" )
	PORT_DIPSETTING(    0x38, "990k" )
	PORT_DIPNAME( 0x40, 0x40, "Hu Type" )				PORT_DIPLOCATION("SW4:7")
	PORT_DIPSETTING(    0x40, "Topple" )
	PORT_DIPSETTING(    0x00, "Non-Topple" )
	PORT_DIPNAME( 0x80, 0x80, "Double Up" )				PORT_DIPLOCATION("SW4:8")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )

	PORT_START("DSW5")		/* IN4 - DSW5 */
	PORT_DIPNAME( 0x03, 0x03, "Douple Up Chance (%)" )	PORT_DIPLOCATION("SW5:1,2")
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPSETTING(    0x01, "60" )
	PORT_DIPSETTING(    0x02, "70" )
	PORT_DIPSETTING(    0x03, "80" )
	PORT_DIPNAME( 0x04, 0x04, "Chinese Word" )			PORT_DIPLOCATION("SW5:3")
	PORT_DIPSETTING(    0x04, "3..6" )
	PORT_DIPSETTING(    0x00, "5..10" )
	PORT_DIPNAME( 0x08, 0x08, "Big Odds Times" )		PORT_DIPLOCATION("SW5:4")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPNAME( 0x30, 0x30, "Hu Test Times" )			PORT_DIPLOCATION("SW5:5,6")
	PORT_DIPSETTING(    0x30, DEF_STR( None ) )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x10, "6" )
	PORT_DIPSETTING(    0x20, "8" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )	PORT_DIPLOCATION("SW5:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Computer Strength" )		PORT_DIPLOCATION("SW5:8")
	PORT_DIPSETTING(    0x80, "Strong" )
	PORT_DIPSETTING(    0x00, "Weak" )

	PORT_START("SERVICE")		/* IN5 - SERVICE */
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_SERVICE3 )		// clear (during boot)
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_SERVICE2 )		// book
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_SERVICE  )		// test (in game: dips, during boot: service mode)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL  )		// hopper sensor
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_SERVICE4 )		// payout
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_COIN1 ) PORT_IMPULSE(2)	// "coin jam" otherwise
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL )		// 0 = tiles block transferrer busy

	PORT_START("IN0")		/* IN6 - P1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )	// gun
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")		/* IN7 - P1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )	// tin
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")		/* IN8 - P1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )	// eat
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )	// hu
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")		/* IN9 - P1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )	// pon
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN4")		/* IN10 - P1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE	)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************
                                Graphics Layout
***************************************************************************/

static const gfx_layout layout_8x8 =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 7*4, 6*4, 1*4, 0*4, 3*4, 2*4, 5*4, 4*4 },
	{ STEP8(0,32) },
	32*8
};

static const gfx_layout layout_8x32 =
{
	8,32,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 7*4, 6*4, 1*4, 0*4, 3*4, 2*4, 5*4, 4*4 },
	{ STEP16(0,32), STEP16(16*32,32) },
	32*32
};

static GFXDECODE_START( dunhuang )
	GFXDECODE_ENTRY( "gfx1", 0, layout_8x8,  0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, layout_8x32, 0, 16 )
GFXDECODE_END


/***************************************************************************
                                Machine Drivers
***************************************************************************/

static const ay8910_interface dunhuang_ay8910_interface =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	//  A                   B
	DEVCB_NULL,							DEVCB_HANDLER(dunhuang_dsw_r),	// R
	DEVCB_MEMORY_HANDLER("maincpu", PROGRAM, dunhuang_input_w),	DEVCB_NULL						// W
};



static MACHINE_START( dunhuang )
{
	dunhuang_state *state = (dunhuang_state *)machine->driver_data;
	UINT8 *ROM = memory_region(machine, "maincpu");

	memory_configure_bank(machine, "bank1", 0, 8, &ROM[0x10000], 0x8000);

	state_save_register_global(machine, state->written);
	state_save_register_global(machine, state->written2);
	state_save_register_global(machine, state->pos_x);
	state_save_register_global(machine, state->pos_y);
	state_save_register_global(machine, state->clear_y);
	state_save_register_global(machine, state->block_x);
	state_save_register_global(machine, state->block_y);
	state_save_register_global(machine, state->block_w);
	state_save_register_global(machine, state->block_h);
	state_save_register_global(machine, state->block_addr_hi);
	state_save_register_global(machine, state->block_addr_lo);
	state_save_register_global(machine, state->block_dest);
	state_save_register_global(machine, state->block_c);
	state_save_register_global(machine, state->layers);
	state_save_register_global(machine, state->paloffs);
	state_save_register_global(machine, state->input);
	state_save_register_global(machine, state->hopper);
}

static MACHINE_RESET( dunhuang )
{
	dunhuang_state *state = (dunhuang_state *)machine->driver_data;

	state->written = 0;
	state->written2 = 0;
	state->pos_x = 0;
	state->pos_y = 0;
	state->clear_y = 0;
	state->block_x = 0;
	state->block_y = 0;
	state->block_w = 0;
	state->block_h = 0;
	state->block_addr_hi = 0;
	state->block_addr_lo = 0;
	state->block_dest = 0;
	state->block_c = 0;
	state->layers = 0;
	state->paloffs = 0;
	state->input = 0;
	state->hopper = 0;
}


static MACHINE_DRIVER_START( dunhuang )

	/* driver data */
	MDRV_DRIVER_DATA(dunhuang_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80,12000000/2)
	MDRV_CPU_PROGRAM_MAP(dunhuang_map)
	MDRV_CPU_IO_MAP(dunhuang_io_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_MACHINE_START(dunhuang)
	MDRV_MACHINE_RESET(dunhuang)

	MDRV_WATCHDOG_TIME_INIT(SEC(5))

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(512, 256)
	MDRV_SCREEN_VISIBLE_AREA(0+8, 512-8-1, 0+16, 256-16-1)

	MDRV_GFXDECODE(dunhuang)
	MDRV_PALETTE_LENGTH(0x100)

	MDRV_VIDEO_START(dunhuang)
	MDRV_VIDEO_UPDATE(dunhuang)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ymsnd", YM2413, 3579545)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MDRV_SOUND_ADD("ay8910", AY8910, 12000000/8)
	MDRV_SOUND_CONFIG(dunhuang_ay8910_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	MDRV_OKIM6295_ADD("oki", 12000000/8, OKIM6295_PIN7_HIGH)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)
MACHINE_DRIVER_END


/***************************************************************************
                                ROMs Loading
***************************************************************************/

ROM_START( dunhuang )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "rom1.u9", 0x00000, 0x40000, CRC(843a0117) SHA1(26a838cb3552ea6a9ec55940fcbf83b06c068743) )
	ROM_RELOAD(          0x10000, 0x40000 )

	ROM_REGION( 0xc0000, "gfx1", 0 )
	ROM_LOAD( "rom3.u4", 0x00000, 0x80000, CRC(1ff5d35e) SHA1(b808eb4f81be8fc77a58dadd661a9cc2b376a509) )
	ROM_LOAD( "rom2.u5", 0x80000, 0x40000, CRC(384fa1d3) SHA1(f329db17aacacf1768ebd6ca2cc612503db93fac) )

	ROM_REGION( 0xc0000, "gfx2", 0 )	// do not dispose
	ROM_LOAD( "rom4.u3", 0x00000, 0x40000, CRC(7db45227) SHA1(2a12a2b8a1e58946ce3e7c770b3ca4803c3c3ccd) )
	ROM_LOAD( "rom5.u2", 0x40000, 0x80000, CRC(d609880e) SHA1(3d69800e959e8f24ef950fea4312610c4407f6ba) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "rom6.u1", 0x00000, 0x20000, CRC(31cfdc29) SHA1(725249eae9227eadf05418b799e0da0254bb2f51) )
ROM_END

GAME( 1995, dunhuang, 0, dunhuang, dunhuang, 0, ROT0, "Spirit", "Mahjong Dunhuang", GAME_SUPPORTS_SAVE )
