/***************************************************************************

    Irem M63 hardware

****************************************************************************

Wily Tower              (c) 1984 Irem
Fighting Basketball     (c) 1984 Paradise Co. Ltd.

driver by Nicola Salmoria


PCB Layout (based on Atomic Boy PCB)
------------------------------------

Both boards has etched NANAO logo.
They (Eizo Nanao Corporation) own Irem Software Engineering.


M63-A-A (TOP)

       1         2       3       4       5       6       7
|---------------------------------------------------------------|
|              DSW8    74373                                    |   A
|                                                               |
|-|   74368    DSW8    74373    AY-3-8910     MSM80C39R6       |-|  B
  |                                                            | |
  |   74368            74373    AY-3-8910   74373              | |  C
  |                                                            | |
  |   74368    7408    74373   wt_a-4d         wt_a-6d        C| |  D
  |                                                           N| |
  |   74368    7400    7404    wt_a-4e 7404    7474    74???  1| |  E
  |2                                                           | |
  |2                   74138           74367   7474    74367   | |  F
  |W                                                           | |
  |A           7432    7427    wt_a-4h D780C                   | |  H
  |Y                                                           |-|
  |   74368    74299   wt_a-3j wt_a-4j 74367   74245            |   J
  |                                                            |-|
  |            74367   wt_a-3k wt_a-4k 74138   7432            | |  K
  |                                                            | |
  |            74299                   74139   7432            | |  L
  |                                                            | |
|-|   74368    74299   wt_a-3m wt_a-4m 74139   M53202  7474   C| |  M
|                                                             N| |
|     7400     74299   wt_a-3n wt_a-4n 74273   74283   7420   2| |  N
|                                                              | |
|              74157   wt_a-3p M58725 WT_A-5P  74283   74273   | |  P
|                                                              | |
|              74299                  WT_A-5R  7432    74157   |-|  R
|     AMP                                                       |
|              74299   wt_a-3s M58725 WT_A-5S  74273   74157    |   S
|---------------------------------------------------------------|

M63-B-A (BOTTOM)

      1        2       3       4       5       6       7       8       9
|------------------------------------------------------------------------------|
|   74244     74244      74244    wt_b-5a     2128   74244   74244   74245     |   A
|                                                                              -
|   74373     74273      74299    wt_b-5b     2128   74161   74161   74161    | |  B
|                                                                             | |
|   74244     74273      74299                       74157   74157   74157    | |  C
|                                                                             | |
|   74273     74244      74299    wt_b-5d     2128   74157   74157   74157   C| |  D
|                                                                            N| |
|   2148      74283      74299    wt_b-5e     2128   74157   74157   74157   1| |  E
|                                                                             | |
|   2148      74283      74299    wt_b-5f     2128   74157   74157   74157    | |  F
|                                                                             | |
|   74157     7420       74157                2128   2148    7486    7486     |-|  H
|                                                                              |
|   74157     7430   74367   7474    74273           2148    74367   74367    |-|  J
|                                                                             | |
|   74161     74139  7420    74161   7486    74139   7474    74161   74161    | |  K
|                                                                             | |
|   74157     7432   7474    74161   7486    7404    7474    74175   wt_b-9l  | |  L
|                                                                            C| |
|   74161     7432   74368   74???   74175   7408    7400    7432            N| |  M
|                                                                            2| |
|             7400   7404    7420    74175   7474    74273   74377   74175    | |  N
|                                                                             | |
|                    7414    M53202  74163   7486    2148    74241   7427     | |  P
|                                                                             |-|
|             12MHz  7404            74163   7486    2148    74241   74373     |   R
|------------------------------------------------------------------------------|




Notes:
- Unless there is some special logic related to NMI enable, the game doesn't
  rely on vblank for timing. It all seems to be controlled by the CPU clock.
  The NMI handler just handles the "Stop Mode" dip switch.

TS 2008.06.14:
- Addedd sound emulation - atomboy and fghtbskt req different interrupt (T1)
  timing than wilytowr, otherwise music/fx tempo is too fast.
  Music tempo and pitch verified on real pcb.
- Extra space in atomboy 2764 eproms is filled with garbage z80 code
  (taken from one of code roms, but from different offset)
- Fghtbskt has one AY, but every frame writes 0 to 2nd AY regs - probably
  leftover from Wily Tower sound driver/code
- I'm not sure about sound_status write - maybe it's something else or
  different data (p1?) is used as status

TODO:
- Sprite positioning is wacky. The electric 'bands' that go along the pipes
  are drawn 2 pixels off in x/y directions. If you fix that, then the player
  sprite doesn't slide in the middle of the pipes when climbing...
- Clocks

Dip locations verified for:
- atomboy (manual)

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/mcs48/mcs48.h"
#include "sound/ay8910.h"
#include "sound/samples.h"

class m63_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, m63_state(machine)); }

	m63_state(running_machine &machine) { }

	UINT8 *  videoram;
	UINT8 *  colorram;
	UINT8 *  spriteram;
	UINT8 *  videoram2;
	UINT8 *  scrollram;
	size_t   spriteram_size;

	/* video-related */
	tilemap_t  *bg_tilemap, *fg_tilemap;
	int      pal_bank, fg_flag, sy_offset;

	/* sound-related */
	UINT8    sound_irq;
	int      sound_status;
	int      p1, p2;
	INT16    *samplebuf;

	/* sound devices */
	running_device *soundcpu;
	running_device *ay1;
	running_device *ay2;
	running_device *samples;
};


static PALETTE_INIT( m63 )
{
	int i;

	for (i = 0; i < 256; i++)
	{
		int bit0, bit1, bit2, bit3, r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		bit3 = (color_prom[i] >> 3) & 0x01;
		r =  0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		/* green component */
		bit0 = (color_prom[i + 256] >> 0) & 0x01;
		bit1 = (color_prom[i + 256] >> 1) & 0x01;
		bit2 = (color_prom[i + 256] >> 2) & 0x01;
		bit3 = (color_prom[i + 256] >> 3) & 0x01;
		g =  0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		/* blue component */
		bit0 = (color_prom[i + 2*256] >> 0) & 0x01;
		bit1 = (color_prom[i + 2*256] >> 1) & 0x01;
		bit2 = (color_prom[i + 2*256] >> 2) & 0x01;
		bit3 = (color_prom[i + 2*256] >> 3) & 0x01;
		b =  0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}

	color_prom += 3 * 256;

	for (i = 0; i < 4; i++)
	{
		int bit0, bit1, bit2, r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = (color_prom[i] >> 6) & 0x01;
		bit1 = (color_prom[i] >> 7) & 0x01;
		b = 0x4f * bit0 + 0xa8 * bit1;

		palette_set_color(machine,i+256,MAKE_RGB(r,g,b));
	}
}

static WRITE8_HANDLER( m63_videoram_w )
{
	m63_state *state = (m63_state *)space->machine->driver_data;

	state->videoram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

static WRITE8_HANDLER( m63_colorram_w )
{
	m63_state *state = (m63_state *)space->machine->driver_data;

	state->colorram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset);
}

static WRITE8_HANDLER( m63_videoram2_w )
{
	m63_state *state = (m63_state *)space->machine->driver_data;

	state->videoram2[offset] = data;
	tilemap_mark_tile_dirty(state->fg_tilemap, offset);
}

static WRITE8_HANDLER( m63_palbank_w )
{
	m63_state *state = (m63_state *)space->machine->driver_data;

	if (state->pal_bank != (data & 0x01))
	{
		state->pal_bank = data & 0x01;
		tilemap_mark_all_tiles_dirty(state->bg_tilemap);
	}
}

static WRITE8_HANDLER( m63_flipscreen_w )
{
	if (flip_screen_get(space->machine) != (~data & 0x01))
	{
		flip_screen_set(space->machine, ~data & 0x01);
		tilemap_mark_all_tiles_dirty_all(space->machine);
	}
}

static WRITE8_HANDLER( fghtbskt_flipscreen_w )
{
	m63_state *state = (m63_state *)space->machine->driver_data;

	flip_screen_set(space->machine, data);
	state->fg_flag = flip_screen_get(space->machine) ? TILE_FLIPX : 0;
}


static TILE_GET_INFO( get_bg_tile_info )
{
	m63_state *state = (m63_state *)machine->driver_data;

	int attr = state->colorram[tile_index];
	int code = state->videoram[tile_index] | ((attr & 0x30) << 4);
	int color = (attr & 0x0f) + (state->pal_bank << 4);

	SET_TILE_INFO(1, code, color, 0);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	m63_state *state = (m63_state *)machine->driver_data;

	int code = state->videoram2[tile_index];

	SET_TILE_INFO(0, code, 0, state->fg_flag);
}

static VIDEO_START( m63 )
{
	m63_state *state = (m63_state *)machine->driver_data;

	state->bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	state->fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);

	tilemap_set_scroll_cols(state->bg_tilemap, 32);
	tilemap_set_transparent_pen(state->fg_tilemap, 0);
}

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect )
{
	m63_state *state = (m63_state *)machine->driver_data;
	int offs;

	for (offs = 0; offs < state->spriteram_size; offs += 4)
	{
		int code = state->spriteram[offs + 1] | ((state->spriteram[offs + 2] & 0x10) << 4);
		int color = (state->spriteram[offs + 2] & 0x0f) + (state->pal_bank << 4);
		int flipx = state->spriteram[offs + 2] & 0x20;
		int flipy = 0;
		int sx = state->spriteram[offs + 3];
		int sy = state->sy_offset - state->spriteram[offs];

		if (flip_screen_get(machine))
		{
			sx = 240 - sx;
			sy = state->sy_offset - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		drawgfx_transpen(bitmap, cliprect,
			machine->gfx[2],
			code, color,
			flipx, flipy,
			sx, sy, 0);

		/* sprite wrapping - verified on real hardware*/
		if (sx > 0xf0)
		{
			drawgfx_transpen(bitmap, cliprect,
			machine->gfx[2],
			code, color,
			flipx, flipy,
			sx - 0x100, sy, 0);
		}

	}
}

static VIDEO_UPDATE( m63 )
{
	m63_state *state = (m63_state *)screen->machine->driver_data;

	int col;

	for (col = 0; col < 32; col++)
		tilemap_set_scrolly(state->bg_tilemap, col, state->scrollram[col * 8]);

	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);
	draw_sprites(screen->machine, bitmap, cliprect);
	tilemap_draw(bitmap, cliprect, state->fg_tilemap, 0, 0);
	return 0;
}


static WRITE8_HANDLER( coin_w )
{
	coin_counter_w(space->machine, offset, data & 0x01);
}

static WRITE8_HANDLER( snd_irq_w )
{
	m63_state *state = (m63_state *)space->machine->driver_data;
	cpu_set_input_line(state->soundcpu, 0, ASSERT_LINE);
	timer_call_after_resynch(space->machine, NULL, 0, NULL);
}

static WRITE8_HANDLER( snddata_w )
{
	m63_state *state = (m63_state *)space->machine->driver_data;

	if ((state->p2 & 0xf0) == 0xe0)
		ay8910_address_w(state->ay1, 0, offset);
	else if ((state->p2 & 0xf0) == 0xa0)
		ay8910_data_w(state->ay1, 0, offset);
	else if (state->ay2 != NULL && (state->p1 & 0xe0) == 0x60)
		ay8910_address_w(state->ay2, 0, offset);
	else if (state->ay2 != NULL && (state->p1 & 0xe0) == 0x40)
		 ay8910_data_w(state->ay2, 0, offset);
	else if ((state->p2 & 0xf0) == 0x70 )
		state->sound_status = offset;
}

static WRITE8_HANDLER( p1_w )
{
	m63_state *state = (m63_state *)space->machine->driver_data;
	state->p1 = data;
}

static WRITE8_HANDLER( p2_w )
{
	m63_state *state = (m63_state *)space->machine->driver_data;

	state->p2 = data;
	if((state->p2 & 0xf0) == 0x50)
	{
		cpu_set_input_line(state->soundcpu, 0, CLEAR_LINE);
	}
}

static READ8_HANDLER( snd_status_r )
{
	m63_state *state = (m63_state *)space->machine->driver_data;
	return state->sound_status;
}

static READ8_HANDLER( irq_r )
{
	m63_state *state = (m63_state *)space->machine->driver_data;

	if (state->sound_irq)
	{
		state->sound_irq = 0;
		return 1;
	}
	return 0;
}

static READ8_HANDLER( snddata_r )
{
	m63_state *state = (m63_state *)space->machine->driver_data;
	switch (state->p2 & 0xf0)
	{
		case 0x60:	return soundlatch_r(space, 0); ;
		case 0x70:	return memory_region(space->machine, "user1")[((state->p1 & 0x1f) << 8) | offset];
	}
	return 0xff;
}

static WRITE8_HANDLER( fghtbskt_samples_w )
{
	m63_state *state = (m63_state *)space->machine->driver_data;

	if (data & 1)
		sample_start_raw(state->samples, 0, state->samplebuf + ((data & 0xf0) << 8), 0x2000, 8000, 0);
}

static ADDRESS_MAP_START( m63_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xd000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xe1ff) AM_RAM
	AM_RANGE(0xe200, 0xe2ff) AM_RAM AM_BASE_SIZE_MEMBER(m63_state, spriteram, spriteram_size)
	AM_RANGE(0xe300, 0xe3ff) AM_RAM AM_BASE_MEMBER(m63_state, scrollram)
	AM_RANGE(0xe400, 0xe7ff) AM_RAM_WRITE(m63_videoram2_w) AM_BASE_MEMBER(m63_state, videoram2)
	AM_RANGE(0xe800, 0xebff) AM_RAM_WRITE(m63_videoram_w) AM_BASE_MEMBER(m63_state, videoram)
	AM_RANGE(0xec00, 0xefff) AM_RAM_WRITE(m63_colorram_w) AM_BASE_MEMBER(m63_state, colorram)
	AM_RANGE(0xf000, 0xf000) AM_WRITE(interrupt_enable_w)	/* NMI enable */
	AM_RANGE(0xf002, 0xf002) AM_WRITE(m63_flipscreen_w)
	AM_RANGE(0xf003, 0xf003) AM_WRITE(m63_palbank_w)
	AM_RANGE(0xf006, 0xf007) AM_WRITE(coin_w)
	AM_RANGE(0xf800, 0xf800) AM_READ_PORT("P1") AM_WRITE(soundlatch_w)
	AM_RANGE(0xf801, 0xf801) AM_READ_PORT("P2") AM_WRITENOP	/* continues game when in stop mode (cleared by NMI handler) */
	AM_RANGE(0xf802, 0xf802) AM_READ_PORT("DSW1")
	AM_RANGE(0xf803, 0xf803) AM_WRITE(snd_irq_w)
	AM_RANGE(0xf806, 0xf806) AM_READ_PORT("DSW2")
ADDRESS_MAP_END

static ADDRESS_MAP_START( fghtbskt_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
	AM_RANGE(0xd000, 0xd1ff) AM_RAM
	AM_RANGE(0xd200, 0xd2ff) AM_RAM AM_BASE_SIZE_MEMBER(m63_state, spriteram, spriteram_size)
	AM_RANGE(0xd300, 0xd3ff) AM_RAM AM_BASE_MEMBER(m63_state, scrollram)
	AM_RANGE(0xd400, 0xd7ff) AM_RAM_WRITE(m63_videoram2_w) AM_BASE_MEMBER(m63_state, videoram2)
	AM_RANGE(0xd800, 0xdbff) AM_RAM_WRITE(m63_videoram_w) AM_BASE_MEMBER(m63_state, videoram)
	AM_RANGE(0xdc00, 0xdfff) AM_RAM_WRITE(m63_colorram_w) AM_BASE_MEMBER(m63_state, colorram)
	AM_RANGE(0xf000, 0xf000) AM_READ(snd_status_r)
	AM_RANGE(0xf001, 0xf001) AM_READ_PORT("P1")
	AM_RANGE(0xf002, 0xf002) AM_READ_PORT("P2")
	AM_RANGE(0xf003, 0xf003) AM_READ_PORT("DSW")
	AM_RANGE(0xf000, 0xf000) AM_WRITE(snd_irq_w)
	AM_RANGE(0xf001, 0xf001) AM_WRITENOP
	AM_RANGE(0xf002, 0xf002) AM_WRITE(soundlatch_w)
	AM_RANGE(0xf800, 0xf800) AM_WRITENOP
	AM_RANGE(0xf801, 0xf801) AM_WRITE(interrupt_enable_w)
	AM_RANGE(0xf802, 0xf802) AM_WRITE(fghtbskt_flipscreen_w)
	AM_RANGE(0xf803, 0xf803) AM_WRITENOP
	AM_RANGE(0xf804, 0xf804) AM_WRITENOP
	AM_RANGE(0xf805, 0xf805) AM_WRITENOP
	AM_RANGE(0xf806, 0xf806) AM_WRITENOP
	AM_RANGE(0xf807, 0xf807) AM_WRITE(fghtbskt_samples_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( i8039_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( i8039_port_map, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x00, 0xff) AM_READWRITE(snddata_r, snddata_w)
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_WRITE(p1_w)
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_WRITE(p2_w)
	AM_RANGE(MCS48_PORT_T1, MCS48_PORT_T1) AM_READ(irq_r)
ADDRESS_MAP_END



static INPUT_PORTS_START( wilytowr )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )		PORT_DIPLOCATION("SW1:!1,!2")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPNAME( 0x0c, 0x00, "Bonus Points Rate" )		PORT_DIPLOCATION("SW1:!3,!4")
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, "x1.2" )
	PORT_DIPSETTING(    0x08, "x1.4" )
	PORT_DIPSETTING(    0x0c, "x1.6" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Coin_A ) )		PORT_DIPLOCATION("SW1:!5,!6") PORT_CONDITION("DSW1",0x04,PORTCOND_EQUALS,0x04) /* coin mode 2 */
	PORT_DIPSETTING(    0x20, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Free_Play ) )	/* Not documented */
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coin_B ) )		PORT_DIPLOCATION("SW1:!7,!8") PORT_CONDITION("DSW1",0x04,PORTCOND_EQUALS,0x04) /* coin mode 2 */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0xf0, 0x00, DEF_STR( Coinage ) )		PORT_DIPLOCATION("SW1:!5,!6,!7,!8") PORT_CONDITION("DSW1",0x04,PORTCOND_EQUALS,0x00) /* coin mode 1 */
	PORT_DIPSETTING(    0x60, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_8C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_9C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( Free_Play ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Flip_Screen ) )	PORT_DIPLOCATION("SW2:!1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Cabinet ) )		PORT_DIPLOCATION("SW2:!2")
	PORT_DIPSETTING(    0x02, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	/*  "For cabinets with a single coin selector or 2 coin selectors of the same value, set to Mode 1.
        For cabinets with coin selectors of two different values, set to Mode 2." */
	PORT_DIPNAME( 0x04, 0x00, "Coin Mode" )				PORT_DIPLOCATION("SW2:!3")
	PORT_DIPSETTING(    0x00, "Mode 1" )
	PORT_DIPSETTING(    0x04, "Mode 2" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x00, "SW2:!4" )		/* Listed as "Unused" */
	/* In stop mode, press 1 to stop and 2 to restart */
	PORT_DIPNAME( 0x10, 0x00, "Stop Mode (Cheat)" )		PORT_DIPLOCATION("SW2:!5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x00, "SW2:!6" )		/* Listed as "Unused" */
	PORT_DIPNAME( 0x40, 0x00, "Invulnerability (Cheat)" ) PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(0x80, IP_ACTIVE_HIGH, "SW2:!8" )
INPUT_PORTS_END

static INPUT_PORTS_START( fghtbskt )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0c, "99 Credits / Sound Test" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Time Count Down" )
	PORT_DIPSETTING(    0x00, "Slow" )
	PORT_DIPSETTING(    0x20, "Too Fast" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(1,2), RGN_FRAC(0,2) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout tilelayout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,6),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			RGN_FRAC(1,6)+0, RGN_FRAC(1,6)+1, RGN_FRAC(1,6)+2, RGN_FRAC(1,6)+3,
			RGN_FRAC(1,6)+4, RGN_FRAC(1,6)+5, RGN_FRAC(1,6)+6, RGN_FRAC(1,6)+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	16*8
};

static GFXDECODE_START( m63 )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   256, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout,     0, 32 )
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout,   0, 32 )
GFXDECODE_END

static GFXDECODE_START( fghtbskt )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   16, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout,    0, 32 )
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout,  0, 32 )
GFXDECODE_END


static SAMPLES_START( fghtbskt_sh_start )
{
	running_machine *machine = device->machine;
	m63_state *state = (m63_state *)machine->driver_data;
	int i, len = memory_region_length(machine, "samples");
	UINT8 *ROM = memory_region(machine, "samples");

	state->samplebuf = auto_alloc_array(machine, INT16, len);
	state_save_register_global_pointer(machine, state->samplebuf, len);

	for(i = 0; i < len; i++)
		state->samplebuf[i] = ((INT8)(ROM[i] ^ 0x80)) * 256;
}

static const samples_interface fghtbskt_samples_interface =
{
	1,
	NULL,
	fghtbskt_sh_start
};

static INTERRUPT_GEN( snd_irq )
{
	m63_state *state = (m63_state *)device->machine->driver_data;
	state->sound_irq = 1;
}

static MACHINE_START( m63 )
{
	m63_state *state = (m63_state *)machine->driver_data;

	state->soundcpu = machine->device("soundcpu");
	state->ay1 = machine->device("ay1");
	state->ay2 = machine->device("ay2");
	state->samples = machine->device("samples");

	state_save_register_global(machine, state->pal_bank);
	state_save_register_global(machine, state->fg_flag);
	state_save_register_global(machine, state->sy_offset);

	/* sound-related */
	state_save_register_global(machine, state->sound_irq);
	state_save_register_global(machine, state->sound_status);
	state_save_register_global(machine, state->p1);
	state_save_register_global(machine, state->p2);
}

static MACHINE_RESET( m63 )
{
	m63_state *state = (m63_state *)machine->driver_data;

	state->pal_bank = 0;
	state->fg_flag = 0;
	state->sound_irq = 0;
	state->sound_status = 0;
	state->p1 = 0;
	state->p2 = 0;
}

static MACHINE_DRIVER_START( m63 )

	/* driver data */
	MDRV_DRIVER_DATA(m63_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu",Z80,XTAL_12MHz/4)     /* 3 MHz */
	MDRV_CPU_PROGRAM_MAP(m63_map)
	MDRV_CPU_VBLANK_INT("screen", nmi_line_pulse)

	MDRV_CPU_ADD("soundcpu",I8039,XTAL_12MHz/4)	/* ????? */
	MDRV_CPU_PROGRAM_MAP(i8039_map)
	MDRV_CPU_IO_MAP(i8039_port_map)
	MDRV_CPU_PERIODIC_INT(snd_irq, 60)

	MDRV_MACHINE_START(m63)
	MDRV_MACHINE_RESET(m63)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(m63)
	MDRV_PALETTE_LENGTH(256+4)

	MDRV_PALETTE_INIT(m63)
	MDRV_VIDEO_START(m63)
	MDRV_VIDEO_UPDATE(m63)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono") /* ????? */

	MDRV_SOUND_ADD("ay1", AY8910, XTAL_12MHz/8)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MDRV_SOUND_ADD("ay2", AY8910, XTAL_12MHz/8) /* ????? */
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( atomboy )
	MDRV_IMPORT_FROM(m63)
	MDRV_CPU_MODIFY("soundcpu")
	MDRV_CPU_PERIODIC_INT(snd_irq, 60/2)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( fghtbskt )

	/* driver data */
	MDRV_DRIVER_DATA(m63_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, XTAL_12MHz/4)     /* 3 MHz */
	MDRV_CPU_PROGRAM_MAP(fghtbskt_map)
	MDRV_CPU_VBLANK_INT("screen", nmi_line_pulse)

	MDRV_CPU_ADD("soundcpu", I8039,XTAL_12MHz/4)	/* ????? */
	MDRV_CPU_PROGRAM_MAP(i8039_map)
	MDRV_CPU_IO_MAP(i8039_port_map)
	MDRV_CPU_PERIODIC_INT(snd_irq, 60/2)

	MDRV_MACHINE_START(m63)
	MDRV_MACHINE_RESET(m63)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(fghtbskt)
	MDRV_PALETTE_LENGTH(256)

	MDRV_PALETTE_INIT(RRRR_GGGG_BBBB)
	MDRV_VIDEO_START(m63)
	MDRV_VIDEO_UPDATE(m63)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ay1", AY8910, XTAL_12MHz/8)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MDRV_SOUND_ADD("samples", SAMPLES, 0)
	MDRV_SOUND_CONFIG(fghtbskt_samples_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( wilytowr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wt4e.bin",     0x0000, 0x2000, CRC(a38e4b8a) SHA1(e296ba1764d3e8e2a5cc43bdde7f30a522b437ff) )
	ROM_LOAD( "wt4h.bin",     0x2000, 0x2000, CRC(c1405ceb) SHA1(c11dd4cd180bc9576e8042e1f56074620ea00f53) )
	ROM_LOAD( "wt4j.bin",     0x4000, 0x2000, CRC(379fb1c3) SHA1(677e4077f6d2140e4fb5c3d86bc7081d3b6cc028) )
	ROM_LOAD( "wt4k.bin",     0x6000, 0x2000, CRC(2dd6f9c7) SHA1(88ba58a1ddd25403211b7f920ba7006ed80c13eb) )
	ROM_LOAD( "wt_a-4m.bin",  0x8000, 0x2000, CRC(c1f8a7d5) SHA1(4307e7604aec728a1f5b0e6a0d6c9f4d37084da3) )
	ROM_LOAD( "wt_a-4n.bin",  0xa000, 0x2000, CRC(b212f7d2) SHA1(dd1c35559982e8bbcb0e778c733a3afb5b6611df) )

	ROM_REGION( 0x1000, "soundcpu", 0 )	/* 8039 */
	ROM_LOAD( "wt4d.bin",     0x0000, 0x1000, CRC(25a171bf) SHA1(7465dbfa8858d0f5822eb748b96d99753d58d243) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	/* '3' character is bad, but ROMs have been verified on four boards */
	ROM_LOAD( "wt_b-5e.bin",  0x0000, 0x1000, CRC(fe45df43) SHA1(9586a5728069e0c293bd17d4663305ce5758ca01) )
	ROM_LOAD( "wt_b-5f.bin",  0x1000, 0x1000, CRC(87a17eff) SHA1(cee2ba2889baf08dc6ee1c8e9150bd277f343be9) )

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "wtb5a.bin",    0x0000, 0x2000, CRC(efc1cbfa) SHA1(9a2ea29e64360ef7b143ac1b6a1ba3e672be4a42) )
	ROM_LOAD( "wtb5b.bin",    0x2000, 0x2000, CRC(ab4bfd07) SHA1(1d5010413989895c09d8e5ee903d665506836f94) )
	ROM_LOAD( "wtb5d.bin",    0x4000, 0x2000, CRC(40f23e1d) SHA1(abff583021e2cf2d2ec83adbbd4f2e96bfa3e04f) )

	ROM_REGION( 0x6000, "gfx3", 0 )
	/* there are horizontal lines in some tiles, but ROMs have been verified on four boards */
	ROM_LOAD( "wt2j.bin",     0x0000, 0x1000, CRC(d1bf0670) SHA1(8d07bce354bb4538948c358fd696304a8e0640b8) )
	ROM_LOAD( "wt3k.bin",     0x1000, 0x1000, CRC(83c39a0e) SHA1(da98f887ac5c3d52281eece3d760c41fb9ecfd5c) )
	ROM_LOAD( "wt_a-3m.bin",  0x2000, 0x1000, CRC(e7e468ae) SHA1(17448191b440b668714d83730075938aaaf34b5a) )
	ROM_LOAD( "wt_a-3n.bin",  0x3000, 0x1000, CRC(0741d1a9) SHA1(51f5ee03db8a3f7afbf944b9e3e4ae12b2520269) )
	ROM_LOAD( "wt_a-3p.bin",  0x4000, 0x1000, CRC(7299f362) SHA1(5ba309d789df8432c08d67e4f9e8bf6c447fc425) )
	ROM_LOAD( "wt_a-3s.bin",  0x5000, 0x1000, CRC(9b37d50d) SHA1(a08d4a7654b815cb652be66dbaa097011327f5d5) )

	ROM_REGION( 0x2000, "user1", 0 )
	ROM_LOAD( "wt_a-6d.bin",  0x0000, 0x1000, CRC(a5dde29b) SHA1(8f7545d2022da7c98d47112179dce717f6c3c5e2) )


	ROM_REGION( 0x0320, "proms", 0 )
	ROM_LOAD( "wt_a-5s-.bpr", 0x0000, 0x0100, CRC(041950e7) SHA1(8276068bec3f4c5013c773033fca3cd3ed9e82ef) )	/* red */
	ROM_LOAD( "wt_a-5r-.bpr", 0x0100, 0x0100, CRC(bc04bf25) SHA1(37d0e89296760f51df5a0d434dca390fb60bb052) )	/* green */
	ROM_LOAD( "wt_a-5p-.bpr", 0x0200, 0x0100, CRC(ed819a19) SHA1(76f13dcf1674f136375738756e175ceec469d545) )	/* blue */
	ROM_LOAD( "wt_b-9l-.bpr", 0x0300, 0x0020, CRC(d2728744) SHA1(e6b1a570854ca90326414874432ab03ec85b9c8e) )	/* char palette */
ROM_END

ROM_START( atomboy )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wt_a-4e.bin",  0x0000, 0x2000, CRC(f7978185) SHA1(6a108d1e9b1a81cedf865aba3998748dcf1d55ef) )
	ROM_LOAD( "wt_a-4h.bin",  0x2000, 0x2000, CRC(0ca9950b) SHA1(d6583fcdf17d16a8884932695caa9c5587a20795) )
	ROM_LOAD( "wt_a-4j.bin",  0x4000, 0x2000, CRC(1badbc65) SHA1(e0768f2cd7bbe8908fd68ff6d54dbef84cc7de4c) )
	ROM_LOAD( "wt_a-4k.bin",  0x6000, 0x2000, CRC(5a341f75) SHA1(9e1a180e37aaa0afbf8ff45219be40d3f75fe60a) )
	ROM_LOAD( "wt_a-4m.bin",  0x8000, 0x2000, CRC(c1f8a7d5) SHA1(4307e7604aec728a1f5b0e6a0d6c9f4d37084da3) )
	ROM_LOAD( "wt_a-4n.bin",  0xa000, 0x2000, CRC(b212f7d2) SHA1(dd1c35559982e8bbcb0e778c733a3afb5b6611df) )

	ROM_REGION( 0x2000, "soundcpu", 0 )	/* 8039 */
	ROM_LOAD( "wt_a-4d-b.bin",  0x0000, 0x2000, CRC(793ea53f) SHA1(9dbff5e011a1f1f48aad68f8e5b02bcdb86c182a) ) /* 2764 ROM, Also had a red dot on label */

	ROM_REGION( 0x2000, "gfx1", 0 )
	/* '3' character is bad, but ROMs have been verified on four boards */
	ROM_LOAD( "wt_b-5e.bin",  0x0000, 0x1000, CRC(fe45df43) SHA1(9586a5728069e0c293bd17d4663305ce5758ca01) )
	ROM_LOAD( "wt_b-5f.bin",  0x1000, 0x1000, CRC(87a17eff) SHA1(cee2ba2889baf08dc6ee1c8e9150bd277f343be9) )

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "wt_b-5a.bin",  0x0000, 0x2000, CRC(da22c452) SHA1(bd921baa12087e996d07625e05eda00981608655) )
	ROM_LOAD( "wt_b-5b.bin",  0x2000, 0x2000, CRC(4fb25a1f) SHA1(0f90fb3b373760c33ba9be3b56b917eca92c9700) )
	ROM_LOAD( "wt_b-5d.bin",  0x4000, 0x2000, CRC(75be2604) SHA1(fe1f110e188aa34a04a9f43412a8308240391fcf) )

	ROM_REGION( 0xc000, "gfx3", 0 )
	ROM_LOAD( "wt_a-3j-b.bin",  0x0000, 0x2000, CRC(996470f1) SHA1(c0c787a73535917d1314bb2e1e9056aea9859205) ) /* All these ROMs are 2764 type/size */
	ROM_LOAD( "wt_a-3k-b.bin",  0x2000, 0x2000, CRC(8f4ec45c) SHA1(525393e0555e1aa24df74e8095da216f02fe3c65) )
	ROM_LOAD( "wt_a-3m-b.bin",  0x4000, 0x2000, CRC(4ac40358) SHA1(c71bd62ef1e8d008abd468c193e67b278599a5f3) )
	ROM_LOAD( "wt_a-3n-b.bin",  0x6000, 0x2000, CRC(709eef5b) SHA1(95beadcf876a2549836329521f1293634413e983) )
	ROM_LOAD( "wt_a-3p-b.bin",  0x8000, 0x2000, CRC(3018b840) SHA1(77df9d4f1c8d76d30c435d03d51ef9e7509fab9c) )
	ROM_LOAD( "wt_a-3s-b.bin",  0xa000, 0x2000, CRC(05a251d4) SHA1(1cd9102871507ab988d5fe799024d63b93807448) )

	ROM_REGION( 0x2000, "user1", 0 )
	ROM_LOAD( "wt_a-6d.bin",  0x0000, 0x1000, CRC(a5dde29b) SHA1(8f7545d2022da7c98d47112179dce717f6c3c5e2) )

	ROM_REGION( 0x0320, "proms", 0 )
	ROM_LOAD( "wt_a-5s-b.bpr", 0x0000, 0x0100, CRC(991e2a04) SHA1(a70525948ad85ad898e0d8a25fb6d1639a4ec133) )	/* red    TBP24S10 (read as 82s129) */
	ROM_LOAD( "wt_a-5r-b.bpr", 0x0100, 0x0100, CRC(fb3822b7) SHA1(bb1ecdd0156acc16bef3c9072e496e4f544b5d9d) )	/* green  TBP24S10 (read as 82s129) */
	ROM_LOAD( "wt_a-5p-b.bpr", 0x0200, 0x0100, CRC(95849f7d) SHA1(ad031d6035045b19c1cd65ac6a78c5aa4b647cd6) )	/* blue   TBP24S10 (read as 82s129) */
	ROM_LOAD( "wt_b-9l-.bpr",  0x0300, 0x0020, CRC(d2728744) SHA1(e6b1a570854ca90326414874432ab03ec85b9c8e) )	/* char palette */
ROM_END

ROM_START( atomboya )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wt_a-4e.bin",  0x0000, 0x2000, CRC(f7978185) SHA1(6a108d1e9b1a81cedf865aba3998748dcf1d55ef) )
	ROM_LOAD( "wt_a-4h.bin",  0x2000, 0x2000, CRC(0ca9950b) SHA1(d6583fcdf17d16a8884932695caa9c5587a20795) )
	ROM_LOAD( "wt_a-4j.bin",  0x4000, 0x2000, CRC(1badbc65) SHA1(e0768f2cd7bbe8908fd68ff6d54dbef84cc7de4c) )
	ROM_LOAD( "wt_a-4k.bin",  0x6000, 0x2000, CRC(5a341f75) SHA1(9e1a180e37aaa0afbf8ff45219be40d3f75fe60a) )
	ROM_LOAD( "wt_a-4m.bin",  0x8000, 0x2000, CRC(c1f8a7d5) SHA1(4307e7604aec728a1f5b0e6a0d6c9f4d37084da3) )
	ROM_LOAD( "wt_a-4n.bin",  0xa000, 0x2000, CRC(b212f7d2) SHA1(dd1c35559982e8bbcb0e778c733a3afb5b6611df) )

	ROM_REGION( 0x1000, "soundcpu", 0 )	/* 8039 */
	ROM_LOAD( "wt_a-4d.bin",  0x0000, 0x1000, CRC(3d43361e) SHA1(2977df9f90d9d214909c56ab44c40ab45fd90675) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	/* '3' character is bad, but ROMs have been verified on four boards */
	ROM_LOAD( "wt_b-5e.bin",  0x0000, 0x1000, CRC(fe45df43) SHA1(9586a5728069e0c293bd17d4663305ce5758ca01) )
	ROM_LOAD( "wt_b-5f.bin",  0x1000, 0x1000, CRC(87a17eff) SHA1(cee2ba2889baf08dc6ee1c8e9150bd277f343be9) )

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "wt_b-5a.bin",  0x0000, 0x2000, CRC(da22c452) SHA1(bd921baa12087e996d07625e05eda00981608655) )
	ROM_LOAD( "wt_b-5b.bin",  0x2000, 0x2000, CRC(4fb25a1f) SHA1(0f90fb3b373760c33ba9be3b56b917eca92c9700) )
	ROM_LOAD( "wt_b-5d.bin",  0x4000, 0x2000, CRC(75be2604) SHA1(fe1f110e188aa34a04a9f43412a8308240391fcf) )

	ROM_REGION( 0x6000, "gfx3", 0 )
	/* there are horizontal lines in some tiles, but ROMs have been verified on four boards */
	ROM_LOAD( "wt_a-3j.bin",  0x0000, 0x1000, CRC(b30ca38f) SHA1(885743893461b8617180a9723f6fcef160a2f05d) )
	ROM_LOAD( "wt_a-3k.bin",  0x1000, 0x1000, CRC(9a77eb73) SHA1(2564a3b3744b0be147b41c521fc7efde53bdfea7) )
	ROM_LOAD( "wt_a-3m.bin",  0x2000, 0x1000, CRC(e7e468ae) SHA1(17448191b440b668714d83730075938aaaf34b5a) )
	ROM_LOAD( "wt_a-3n.bin",  0x3000, 0x1000, CRC(0741d1a9) SHA1(51f5ee03db8a3f7afbf944b9e3e4ae12b2520269) )
	ROM_LOAD( "wt_a-3p.bin",  0x4000, 0x1000, CRC(7299f362) SHA1(5ba309d789df8432c08d67e4f9e8bf6c447fc425) )
	ROM_LOAD( "wt_a-3s.bin",  0x5000, 0x1000, CRC(9b37d50d) SHA1(a08d4a7654b815cb652be66dbaa097011327f5d5) )

	ROM_REGION( 0x1000, "user1", 0 )
	ROM_LOAD( "wt_a-6d.bin",  0x0000, 0x1000, CRC(a5dde29b) SHA1(8f7545d2022da7c98d47112179dce717f6c3c5e2) )

	ROM_REGION( 0x0320, "proms", 0 )
	ROM_LOAD( "wt_a-5s-.bpr", 0x0000, 0x0100, CRC(041950e7) SHA1(8276068bec3f4c5013c773033fca3cd3ed9e82ef) )	/* red */
	ROM_LOAD( "wt_a-5r-.bpr", 0x0100, 0x0100, CRC(bc04bf25) SHA1(37d0e89296760f51df5a0d434dca390fb60bb052) )	/* green */
	ROM_LOAD( "wt_a-5p-.bpr", 0x0200, 0x0100, CRC(ed819a19) SHA1(76f13dcf1674f136375738756e175ceec469d545) )	/* blue */
	ROM_LOAD( "wt_b-9l-.bpr", 0x0300, 0x0020, CRC(d2728744) SHA1(e6b1a570854ca90326414874432ab03ec85b9c8e) )	/* char palette */
ROM_END

ROM_START( fghtbskt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fb14.0f",      0x0000, 0x2000, CRC(82032853) SHA1(e103ace4cac6df3a429b785f9789b302ae8cdade) )
	ROM_LOAD( "fb13.2f",      0x2000, 0x2000, CRC(5306df0f) SHA1(11be226e7167703bb08e48510a113b2d43b211a4) )
	ROM_LOAD( "fb12.3f",      0x4000, 0x2000, CRC(ee9210d4) SHA1(c63d036314d635f65a2b5bb192ceb312a587db6e) )
	ROM_LOAD( "fb10.6f",      0x8000, 0x2000, CRC(6b47efba) SHA1(cb55c7a9d5afe748c1c88f87dd1909e106932798) )
	ROM_LOAD( "fb09.7f",      0xa000, 0x2000, CRC(be69e087) SHA1(be95ecafa494cb0787ee18eb3ecea4ad545a6ae3) )

	ROM_REGION( 0x1000, "soundcpu", 0 )	/* 8039 */
	ROM_LOAD( "fb07.0b",      0x0000, 0x1000, CRC(50432dbd) SHA1(35a2218ed243bde47dbe06b5a11a65502ba734ea) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "fb08.12f",     0x0000, 0x1000, CRC(271cd7b8) SHA1(00cfeb6ba429cf6cc59d6542dea8de2ca79155ed) )
	ROM_FILL(				  0x1000, 0x1000, 0 )

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "fb21.25e",     0x0000, 0x2000, CRC(02843591) SHA1(e38ccc97dcbd642d0ac768837f7baf1573fdb91f) )
	ROM_LOAD( "fb22.23e",     0x2000, 0x2000, CRC(cd51d8e7) SHA1(16d55d13b47dddb7c7e6b28b1512540938a4a596) )
	ROM_LOAD( "fb23.22e",     0x4000, 0x2000, CRC(62bcac87) SHA1(dd2272d8c7e46bd0a742b4490c9e960b2bfe14c3) )

	ROM_REGION( 0xc000, "gfx3", 0 )
	ROM_LOAD( "fb16.35a",     0x0000, 0x2000, CRC(a5df1652) SHA1(76d1443c523851aa418574c6a879f4a8e46dc887) )
	ROM_LOAD( "fb15.37a",     0x2000, 0x2000, CRC(59c4de06) SHA1(594411f10d6bb3577c649c66133b90c6423184d7) )
	ROM_LOAD( "fb18.32a",     0x4000, 0x2000, CRC(c23ddcd7) SHA1(f73d142ac0baae519ed633a923e132eb1836adbb) )
	ROM_LOAD( "fb17.34a",     0x6000, 0x2000, CRC(7db28013) SHA1(305e6a6254f69625c81ae107f4420fd76f9a24ba) )
	ROM_LOAD( "fb20.29a",     0x8000, 0x2000, CRC(1a1b48f8) SHA1(62f7774807aea86f73f0b9380bb1c237d55bf451) )
	ROM_LOAD( "fb19.31a",     0xa000, 0x2000, CRC(7ff7e321) SHA1(4fe4eee9c6260599950080c600187ce8e9dab7d2) )

	ROM_REGION( 0xa000, "samples", 0 ) /* Samples */
	ROM_LOAD( "fb01.42a",     0x0000, 0x2000, CRC(1200b220) SHA1(8a5f896441c6a6507e72b9b302a8183cc361d118) )
	ROM_LOAD( "fb02.41a",     0x2000, 0x2000, CRC(0b67aa82) SHA1(59b6cf733150eab0bd807beeeb1d2f784ccb6f58) )
	ROM_LOAD( "fb03.40a",     0x4000, 0x2000, CRC(c71269ed) SHA1(71cc6f43877b28d50beb744587c189dabbbaa067) )
	ROM_LOAD( "fb04.39a",     0x6000, 0x2000, CRC(02ddc42d) SHA1(9d40967071f674592c174b5a5470db56a5f99adf) )
	ROM_LOAD( "fb05.38a",     0x8000, 0x2000, CRC(72ea6b49) SHA1(e081a1cad5abf373a2489169b5c86ee63dcf5823) )

	ROM_REGION( 0x2000, "user1", 0 )
	ROM_LOAD( "fb06.12a",     0x0000, 0x2000, CRC(bea3df99) SHA1(18b795f8626b22f6a1620e04c23f4967c3122c89) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "fb_r.9e",      0x0000, 0x0100, CRC(c5cdc8ba) SHA1(3fcef3ebe0dda72dfa35e042ff611758c345d749) )
	ROM_LOAD( "fb_g.10e",     0x0100, 0x0100, CRC(1460c936) SHA1(f99a544c83931de098a6cfac391f63ae43f5cdd0) )
	ROM_LOAD( "fb_b.11e",     0x0200, 0x0100, CRC(fca5bf0e) SHA1(5846f43aa2906cac58e300fdab197b99f896e3ef) )
ROM_END

static DRIVER_INIT( wilytowr )
{
	m63_state *state = (m63_state *)machine->driver_data;
	state->sy_offset = 238;
}

static DRIVER_INIT( fghtbskt )
{
	m63_state *state = (m63_state *)machine->driver_data;
	state->sy_offset = 240;
}

GAME( 1984, wilytowr, 0,        m63,      wilytowr, wilytowr, ROT180, "Irem",                    "Wily Tower", GAME_SUPPORTS_SAVE )
GAME( 1985, atomboy,  wilytowr, atomboy,  wilytowr, wilytowr, ROT180, "Irem (Memetron license)", "Atomic Boy (revision B)", GAME_SUPPORTS_SAVE )
GAME( 1985, atomboya, wilytowr, atomboy,  wilytowr, wilytowr, ROT180, "Irem (Memetron license)", "Atomic Boy (revision A)", GAME_SUPPORTS_SAVE )
GAME( 1984, fghtbskt, 0,        fghtbskt, fghtbskt, fghtbskt, ROT0,   "Paradise Co. Ltd.",       "Fighting Basketball", GAME_SUPPORTS_SAVE )
