/*

    D-DAY   (c)Jaleco 1984

    driver by Pierpaolo Prazzoli and Tomasz Slanina

-------------------------------------------------------
Is it 1984 or 1987 game ?
There's text inside rom "1987.07    BY  ELS"

$842f = lives

-------------------------------------------------------

    CPU  : Z80
    Sound: Z80 AY-3-8910(x2)
    OSC  : 12.000MHz
    Other: Intel 8257 DMA controller

    -------
    DD-8416
    -------
    ROMs:
    1  - (2764)
    2  |
    3  |
    4  |
    5  |
    6  |
    7  |
    8  |
    9  |
    10 |
    11 /

    -------
    DD-8417
    -------
    ROMs:
    12 - (2732)
    13 /

    14 - (2732)
    15 /

    16 - (2764)
    17 |
    18 |
    19 /


*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"


class ddayjlc_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, ddayjlc_state(machine)); }

	ddayjlc_state(running_machine &machine) { }

	/* memory pointers */
	UINT8 *  bgram;
	UINT8 *  mainram;
	UINT8 *  videoram;
	UINT8 *  spriteram;

	/* video-related */
	tilemap_t  *bg_tilemap;
	INT32    char_bank;
	INT32    bgadr;

	/* misc */
	INT32    sound_nmi_enable;
	INT32    main_nmi_enable;
	INT32    e00x_l[4];
	INT32    e00x_d[4][2];
	UINT8    prot_addr;

	/* devices */
	running_device *audiocpu;
};



/*
    Protection device

    24 pin IC with scratched surface, probably a mcu

    Pinout:

     1 - vcc
     2 - ?
     3 - I/O (input)
     4 - I/O (input)
     5 - I/O (input)
     6 - I/O (input)
     7 - vcc
     8 - xtal
     9 - ?
    10 - gnd
    11 - ?
    12 - ?
    13 - I/O (input)
    14 - ?
    15 - I/O (input)
    16 - ?
    17 - ?
    18 - I/O (input)
    19 - ?
    20 - ?
    21 - I/O (input)
    22 - ?
    23 - ?
    24 - ?

*/

static const UINT8 prot_data[0x10] =
{
	0x02, 0x02, 0x02, 0x02,
	0x02, 0x00, 0x02, 0x00,
	0x02, 0x02, 0x02, 0x00,
	0x03, 0x01, 0x00, 0x03
};

static CUSTOM_INPUT( prot_r )
{
	ddayjlc_state *state = (ddayjlc_state *)field->port->machine->driver_data;
	return prot_data[state->prot_addr];
}

static WRITE8_HANDLER( prot_w )
{
	ddayjlc_state *state = (ddayjlc_state *)space->machine->driver_data;
	state->prot_addr = (state->prot_addr & (~(1 << offset))) | ((data & 1) << offset);
}

static WRITE8_HANDLER( char_bank_w )
{
	ddayjlc_state *state = (ddayjlc_state *)space->machine->driver_data;
	state->char_bank = data;
}

static WRITE8_HANDLER( ddayjlc_bgram_w )
{
	ddayjlc_state *state = (ddayjlc_state *)space->machine->driver_data;

	if (!offset)
		tilemap_set_scrollx(state->bg_tilemap, 0, data + 8);

	state->bgram[offset] = data;
	tilemap_mark_tile_dirty(state->bg_tilemap, offset & 0x3ff);
}

static WRITE8_HANDLER( ddayjlc_videoram_w )
{
	ddayjlc_state *state = (ddayjlc_state *)space->machine->driver_data;
	state->videoram[offset] = data;
}


static WRITE8_HANDLER(sound_nmi_w)
{
	ddayjlc_state *state = (ddayjlc_state *)space->machine->driver_data;
	state->sound_nmi_enable = data;
}

static WRITE8_HANDLER(main_nmi_w)
{
	ddayjlc_state *state = (ddayjlc_state *)space->machine->driver_data;
	state->main_nmi_enable = data;
}

static WRITE8_HANDLER( bg0_w )
{
	ddayjlc_state *state = (ddayjlc_state *)space->machine->driver_data;
	state->bgadr = (state->bgadr & 0xfe) | (data & 1);
}

static WRITE8_HANDLER( bg1_w )
{
	ddayjlc_state *state = (ddayjlc_state *)space->machine->driver_data;
	state->bgadr = (state->bgadr & 0xfd) | ((data & 1) << 1);
}

static WRITE8_HANDLER( bg2_w )
{
	ddayjlc_state *state = (ddayjlc_state *)space->machine->driver_data;

	state->bgadr = (state->bgadr & 0xfb) | ((data & 1) << 2);
	if (state->bgadr > 2)
		state->bgadr = 0;

	memory_set_bank(space->machine, "bank1", state->bgadr);
}

static WRITE8_HANDLER( sound_w )
{
	ddayjlc_state *state = (ddayjlc_state *)space->machine->driver_data;

	soundlatch_w(space, offset, data);
	cpu_set_input_line_and_vector(state->audiocpu, 0, HOLD_LINE, 0xff);
}

static WRITE8_HANDLER( i8257_CH0_w )
{
	ddayjlc_state *state = (ddayjlc_state *)space->machine->driver_data;

	state->e00x_d[offset][state->e00x_l[offset]] = data;
	state->e00x_l[offset] ^= 1;
}

static WRITE8_HANDLER( i8257_LMSR_w )
{
	ddayjlc_state *state = (ddayjlc_state *)space->machine->driver_data;

	if (!data)
	{
		INT32 src = state->e00x_d[0][1] * 256 + state->e00x_d[0][0];
		INT32 dst = state->e00x_d[2][1] * 256 + state->e00x_d[2][0];
		INT32 size = (state->e00x_d[1][1] * 256 + state->e00x_d[1][0]) & 0x3ff;
		INT32 i;

		size++; //??

		for(i = 0; i < size; i++)
		{
			memory_write_byte(space, dst++, memory_read_byte(space, src++));
		}

		state->e00x_l[0] = 0;
		state->e00x_l[1] = 0;
		state->e00x_l[2] = 0;
		state->e00x_l[3] = 0;
	}
}

static ADDRESS_MAP_START( main_cpu, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x8fff) AM_RAM AM_BASE_MEMBER(ddayjlc_state, mainram)
	AM_RANGE(0x9000, 0x93ff) AM_RAM AM_BASE_MEMBER(ddayjlc_state, spriteram)
	AM_RANGE(0x9400, 0x97ff) AM_RAM_WRITE(ddayjlc_videoram_w) AM_BASE_MEMBER(ddayjlc_state, videoram)
	AM_RANGE(0x9800, 0x9fff) AM_RAM_WRITE(ddayjlc_bgram_w) AM_BASE_MEMBER(ddayjlc_state, bgram) /* 9800-981f - videoregs */
	AM_RANGE(0xa000, 0xdfff) AM_ROMBANK("bank1") AM_WRITENOP
	AM_RANGE(0xe000, 0xe003) AM_WRITE(i8257_CH0_w)
	AM_RANGE(0xe008, 0xe008) AM_WRITENOP
	AM_RANGE(0xf000, 0xf000) AM_WRITE(sound_w)
	AM_RANGE(0xf100, 0xf100) AM_WRITENOP
	AM_RANGE(0xf080, 0xf080) AM_WRITE(char_bank_w)
	AM_RANGE(0xf081, 0xf081) AM_WRITENOP
	AM_RANGE(0xf083, 0xf083) AM_WRITE(i8257_LMSR_w)
	AM_RANGE(0xf084, 0xf084) AM_WRITE(bg0_w)
	AM_RANGE(0xf085, 0xf085) AM_WRITE(bg1_w)
	AM_RANGE(0xf086, 0xf086) AM_WRITE(bg2_w)
	AM_RANGE(0xf101, 0xf101) AM_WRITE(main_nmi_w)
	AM_RANGE(0xf102, 0xf105) AM_WRITE(prot_w)
	AM_RANGE(0xf000, 0xf000) AM_READ_PORT("INPUTS")
	AM_RANGE(0xf100, 0xf100) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xf180, 0xf180) AM_READ_PORT("DSW1")
	AM_RANGE(0xf200, 0xf200) AM_READ_PORT("DSW2")
ADDRESS_MAP_END


static ADDRESS_MAP_START( sound_cpu, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x23ff) AM_RAM
	AM_RANGE(0x3000, 0x3000) AM_DEVREADWRITE("ay1", ay8910_r, ay8910_data_w)
	AM_RANGE(0x4000, 0x4000) AM_DEVWRITE("ay1", ay8910_address_w)
	AM_RANGE(0x5000, 0x5000) AM_DEVREADWRITE("ay2", ay8910_r, ay8910_data_w)
	AM_RANGE(0x6000, 0x6000) AM_DEVWRITE("ay2", ay8910_address_w)
	AM_RANGE(0x7000, 0x7000) AM_WRITE(sound_nmi_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( ddayjlc )
	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(1)

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x60, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(prot_r, NULL)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x1c, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x14, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x20, 0x00, "Extend" )
	PORT_DIPSETTING(    0x00, "30K" )
	PORT_DIPSETTING(    0x20, "50K" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xf8, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0xf8, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(0,2), RGN_FRAC(1,2) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8 },
	8*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(0,2), RGN_FRAC(1,2)},
	{ 0+8*8, 1+8*8, 2+8*8, 3+8*8, 4+8*8, 5+8*8, 6+8*8, 7+8*8,0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,0*8+2*8*8, 1*8+2*8*8, 2*8+2*8*8, 3*8+2*8*8, 4*8+2*8*8, 5*8+2*8*8, 6*8+2*8*8, 7*8+2*8*8},
	16*16,
};

static GFXDECODE_START( ddayjlc )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout,   0x000, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, charlayout,     0x080, 16 )
	GFXDECODE_ENTRY( "gfx3", 0, charlayout,     0x100, 16 )
GFXDECODE_END

static TILE_GET_INFO( get_tile_info_bg )
{
	ddayjlc_state *state = (ddayjlc_state *)machine->driver_data;
	int code = state->bgram[tile_index] + ((state->bgram[tile_index + 0x400] & 0x08) << 5);
	int color = (state->bgram[tile_index + 0x400] & 0x7);
	color |= (state->bgram[tile_index + 0x400] & 0x40) >> 3;

	SET_TILE_INFO(2, code, color, 0);
}

static VIDEO_START( ddayjlc )
{
	ddayjlc_state *state = (ddayjlc_state *)machine->driver_data;
	state->bg_tilemap = tilemap_create(machine, get_tile_info_bg, tilemap_scan_rows, 8, 8, 32, 32);
}

static VIDEO_UPDATE( ddayjlc )
{
	ddayjlc_state *state = (ddayjlc_state *)screen->machine->driver_data;
	UINT32 i;
	tilemap_draw(bitmap, cliprect, state->bg_tilemap, 0, 0);

	for (i = 0; i < 0x400; i += 4)
	{
		UINT8  flags = state->spriteram[i + 2];
		UINT8  y = 256 - state->spriteram[i + 0] - 8;
		UINT16 code = state->spriteram[i + 1];
		UINT8  x = state->spriteram[i + 3] - 16;
		UINT8  xflip = flags & 0x80;
		UINT8  yflip = (code & 0x80);
		UINT8  color = flags & 0xf;

		code = (code & 0x7f) | ((flags & 0x30) << 3);

		drawgfx_transpen(bitmap, cliprect, screen->machine->gfx[0], code, color, xflip, yflip, x, y, 0);
	}

	{
		UINT32 x, y, c;
		/* FIXME: where is/are the color offset(s)? I doubt it's hard-coded ... */
		for (y = 0; y < 32; y++)
			for (x = 0; x < 32; x++)
			{
				c = state->videoram[y * 32 + x];
				if (x > 1 && x < 30)
					drawgfx_transpen(bitmap, cliprect, screen->machine->gfx[1], c + state->char_bank * 0x100, 2, 0, 0, x*8, y*8, 0);
				else
					drawgfx_opaque(bitmap, cliprect, screen->machine->gfx[1], c + state->char_bank * 0x100, 2, 0, 0, x*8, y*8);
			}
	}
	return 0;
}

static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_MEMORY_HANDLER("audiocpu", PROGRAM, soundlatch_r),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

static INTERRUPT_GEN( ddayjlc_interrupt )
{
	ddayjlc_state *state = (ddayjlc_state *)device->machine->driver_data;
	if(state->main_nmi_enable)
		cpu_set_input_line(device, INPUT_LINE_NMI, PULSE_LINE);
}

static INTERRUPT_GEN( ddayjlc_snd_interrupt )
{
	ddayjlc_state *state = (ddayjlc_state *)device->machine->driver_data;
	if(state->sound_nmi_enable)
		cpu_set_input_line(device, INPUT_LINE_NMI, PULSE_LINE);
}


static MACHINE_START( ddayjlc )
{
	ddayjlc_state *state = (ddayjlc_state *)machine->driver_data;

	state->audiocpu = machine->device("audiocpu");

	state_save_register_global(machine, state->char_bank);
	state_save_register_global(machine, state->bgadr);
	state_save_register_global(machine, state->sound_nmi_enable);
	state_save_register_global(machine, state->main_nmi_enable);
	state_save_register_global(machine, state->prot_addr);

	state_save_register_global_array(machine, state->e00x_l);
	state_save_register_global_array(machine, state->e00x_d[0]);
	state_save_register_global_array(machine, state->e00x_d[1]);
	state_save_register_global_array(machine, state->e00x_d[2]);
	state_save_register_global_array(machine, state->e00x_d[3]);
}

static MACHINE_RESET( ddayjlc )
{
	ddayjlc_state *state = (ddayjlc_state *)machine->driver_data;
	int i;

	state->char_bank = 0;
	state->bgadr = 0;
	state->sound_nmi_enable = 0;
	state->main_nmi_enable = 0;
	state->prot_addr = 0;

	for (i = 0; i < 4; i++)
	{
		state->e00x_l[i] = 0;
		state->e00x_d[i][0] = 0;
		state->e00x_d[i][1] = 0;
	}
}

static PALETTE_INIT( ddayjlc )
{
	int i,r,g,b,val;
	int bit0,bit1,bit2;

	for (i = 0; i < 0x200; i++)
	{
		val = (color_prom[i+0x000]) | (color_prom[i+0x200]<<4);

		bit0 = 0;
		bit1 = (val >> 6) & 0x01;
		bit2 = (val >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (val >> 3) & 0x01;
		bit1 = (val >> 4) & 0x01;
		bit2 = (val >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (val >> 0) & 0x01;
		bit1 = (val >> 1) & 0x01;
		bit2 = (val >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
	}
}

static MACHINE_DRIVER_START( ddayjlc )

	/* driver data */
	MDRV_DRIVER_DATA(ddayjlc_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80,12000000/3)
	MDRV_CPU_PROGRAM_MAP(main_cpu)
	MDRV_CPU_VBLANK_INT("screen", ddayjlc_interrupt)

	MDRV_CPU_ADD("audiocpu", Z80, 12000000/4)
	MDRV_CPU_PROGRAM_MAP(sound_cpu)
	MDRV_CPU_VBLANK_INT("screen", ddayjlc_snd_interrupt)

	MDRV_QUANTUM_TIME(HZ(6000))

	MDRV_MACHINE_START(ddayjlc)
	MDRV_MACHINE_RESET(ddayjlc)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(ddayjlc)
	MDRV_PALETTE_LENGTH(0x200)
	MDRV_PALETTE_INIT(ddayjlc)

	MDRV_VIDEO_START(ddayjlc)
	MDRV_VIDEO_UPDATE(ddayjlc)

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ay1", AY8910, 12000000/6)
	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MDRV_SOUND_ADD("ay2", AY8910, 12000000/6)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


ROM_START( ddayjlc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1",  0x0000, 0x2000, CRC(dbfb8772) SHA1(1fbc9726d0cd1f8781ced2f8233107b65b9bdb1a) )
	ROM_LOAD( "2",  0x2000, 0x2000, CRC(f40ea53e) SHA1(234ef686d3e9fe12aceada7098c4cc53e56eb1a3) )
	ROM_LOAD( "3",  0x4000, 0x2000, CRC(0780ef60) SHA1(9247af38acbaea0f78892fc50081b2400abbdc1f) )
	ROM_LOAD( "4",  0x6000, 0x2000, CRC(75991a24) SHA1(175f505da6eb80479a70181d6aed01130f6a64cc) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "11", 0x0000, 0x2000, CRC(fe4de019) SHA1(16c5402e1a79756f8227d7e99dd94c5896c57444) )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "16", 0x0000, 0x2000, CRC(a167fe9a) SHA1(f2770d93ee5ae4eb9b3bcb052e14e36f53eec707) )
	ROM_LOAD( "17", 0x2000, 0x2000, CRC(13ffe662) SHA1(2ea7855a14a4b8429751bae2e670e77608f93406) )
	ROM_LOAD( "18", 0x4000, 0x2000, CRC(debe6531) SHA1(34b3b70a1872527266c664b2a82014d028a4ff1e) )
	ROM_LOAD( "19", 0x6000, 0x2000, CRC(5816f947) SHA1(2236bed3e82980d3e7de3749aef0fbab042086e6) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "14", 0x1000, 0x1000, CRC(2c0e9bbe) SHA1(e34ab774d2eb17ddf51af513dbcaa0c51f8dcbf7) )
	ROM_LOAD( "15", 0x0000, 0x1000, CRC(a6eeaa50) SHA1(052cd3e906ca028e6f55d0caa1e1386482684cbf) )

	ROM_REGION( 0x2000, "gfx3", 0 )
	ROM_LOAD( "12", 0x1000, 0x1000, CRC(7f7afe80) SHA1(e8a549b8a8985c61d3ba452e348414146f2bc77e) )
	ROM_LOAD( "13", 0x0000, 0x1000, CRC(f169b93f) SHA1(fb0617162542d688503fc6618dd430308e259455) )

	ROM_REGION( 0xc0000, "user1", 0 )
	ROM_LOAD( "5",  0x00000, 0x2000, CRC(299b05f2) SHA1(3c1804bccb514bada4bed68a6af08db63a8f1b19) )
	ROM_LOAD( "6",  0x02000, 0x2000, CRC(38ae2616) SHA1(62c96f32532f0d7e2cf1606a303d81ebb4aada7d) )
	ROM_LOAD( "7",  0x04000, 0x2000, CRC(4210f6ef) SHA1(525d8413afabf97cf1d04ee9a3c3d980b91bde65) )
	ROM_LOAD( "8",  0x06000, 0x2000, CRC(91a32130) SHA1(cbcd673b47b672b9ce78c7354dacb5964a81db6f) )
	ROM_LOAD( "9",  0x08000, 0x2000, CRC(ccb82f09) SHA1(37c23f13aa0728bae82dba9e2858a8d81fa8afa5) )
	ROM_LOAD( "10", 0x0a000, 0x2000, CRC(5452aba1) SHA1(03ef47161d0ab047c8675d6ffd3b7acf81f74721) )

	ROM_REGION( 0x0500, "proms", 0 )
	ROM_LOAD( "3l.bin",  0x00400, 0x0100, CRC(da6fe846) SHA1(e8386cf7f552facf2d1a5b7b63ca3d2f1801d215) )
	ROM_LOAD( "4l.bin",  0x00000, 0x0100, CRC(2c3fa534) SHA1(e4c0d06cf62459c1835cb27a4e659b01ad4be20c) )
	ROM_LOAD( "4m.bin",  0x00200, 0x0100, CRC(e0ab9a8f) SHA1(77010c4039f9d408f40cea079c1ef56132ddbd2b) )
	ROM_LOAD( "5n.bin",  0x00300, 0x0100, CRC(61d85970) SHA1(189e9da3dade54936872b80893b1318e5fbfbe5e) )
	ROM_LOAD( "5p.bin",  0x00100, 0x0100, CRC(4fd96b26) SHA1(0fb9928ab6c4ee937cefcf82145a4c9d43ca8517) )
ROM_END

ROM_START( ddayjlca )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1a", 0x0000, 0x2000, CRC(d8e4f3d4) SHA1(78b30b4896a7f718975b1502c6253819bceee922) )
	ROM_LOAD( "2",  0x2000, 0x2000, CRC(f40ea53e) SHA1(234ef686d3e9fe12aceada7098c4cc53e56eb1a3) )
	ROM_LOAD( "3",  0x4000, 0x2000, CRC(0780ef60) SHA1(9247af38acbaea0f78892fc50081b2400abbdc1f) )
	ROM_LOAD( "4",  0x6000, 0x2000, CRC(75991a24) SHA1(175f505da6eb80479a70181d6aed01130f6a64cc) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "11", 0x0000, 0x2000, CRC(fe4de019) SHA1(16c5402e1a79756f8227d7e99dd94c5896c57444) )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "16", 0x0000, 0x2000, CRC(a167fe9a) SHA1(f2770d93ee5ae4eb9b3bcb052e14e36f53eec707) )
	ROM_LOAD( "17", 0x2000, 0x2000, CRC(13ffe662) SHA1(2ea7855a14a4b8429751bae2e670e77608f93406) )
	ROM_LOAD( "18", 0x4000, 0x2000, CRC(debe6531) SHA1(34b3b70a1872527266c664b2a82014d028a4ff1e) )
	ROM_LOAD( "19", 0x6000, 0x2000, CRC(5816f947) SHA1(2236bed3e82980d3e7de3749aef0fbab042086e6) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "14", 0x1000, 0x1000, CRC(2c0e9bbe) SHA1(e34ab774d2eb17ddf51af513dbcaa0c51f8dcbf7) )
	ROM_LOAD( "15", 0x0000, 0x1000, CRC(a6eeaa50) SHA1(052cd3e906ca028e6f55d0caa1e1386482684cbf) )

	ROM_REGION( 0x2000, "gfx3", 0 )
	ROM_LOAD( "12", 0x1000, 0x1000, CRC(7f7afe80) SHA1(e8a549b8a8985c61d3ba452e348414146f2bc77e) )
	ROM_LOAD( "13", 0x0000, 0x1000, CRC(f169b93f) SHA1(fb0617162542d688503fc6618dd430308e259455) )

	ROM_REGION( 0xc0000, "user1", 0 )
	ROM_LOAD( "5",  0x00000, 0x2000, CRC(299b05f2) SHA1(3c1804bccb514bada4bed68a6af08db63a8f1b19) )
	ROM_LOAD( "6",  0x02000, 0x2000, CRC(38ae2616) SHA1(62c96f32532f0d7e2cf1606a303d81ebb4aada7d) )
	ROM_LOAD( "7",  0x04000, 0x2000, CRC(4210f6ef) SHA1(525d8413afabf97cf1d04ee9a3c3d980b91bde65) )
	ROM_LOAD( "8",  0x06000, 0x2000, CRC(91a32130) SHA1(cbcd673b47b672b9ce78c7354dacb5964a81db6f) )
	ROM_LOAD( "9",  0x08000, 0x2000, CRC(ccb82f09) SHA1(37c23f13aa0728bae82dba9e2858a8d81fa8afa5) )
	ROM_LOAD( "10", 0x0a000, 0x2000, CRC(5452aba1) SHA1(03ef47161d0ab047c8675d6ffd3b7acf81f74721) )

	ROM_REGION( 0x0500, "proms", 0 )
	ROM_LOAD( "3l.bin",  0x00400, 0x0100, CRC(da6fe846) SHA1(e8386cf7f552facf2d1a5b7b63ca3d2f1801d215) )
	ROM_LOAD( "4l.bin",  0x00000, 0x0100, CRC(2c3fa534) SHA1(e4c0d06cf62459c1835cb27a4e659b01ad4be20c) )
	ROM_LOAD( "4m.bin",  0x00200, 0x0100, CRC(e0ab9a8f) SHA1(77010c4039f9d408f40cea079c1ef56132ddbd2b) )
	ROM_LOAD( "5n.bin",  0x00300, 0x0100, CRC(61d85970) SHA1(189e9da3dade54936872b80893b1318e5fbfbe5e) )
	ROM_LOAD( "5p.bin",  0x00100, 0x0100, CRC(4fd96b26) SHA1(0fb9928ab6c4ee937cefcf82145a4c9d43ca8517) )

ROM_END


static DRIVER_INIT( ddayjlc )
{
#define repack(n)\
		dst[newadr+0+n] = src[oldaddr+0+n];\
		dst[newadr+1+n] = src[oldaddr+1+n];\
		dst[newadr+2+n] = src[oldaddr+2+n];\
		dst[newadr+3+n] = src[oldaddr+3+n];\
		dst[newadr+4+n] = src[oldaddr+4+n];\
		dst[newadr+5+n] = src[oldaddr+5+n];\
		dst[newadr+6+n] = src[oldaddr+6+n];\
		dst[newadr+7+n] = src[oldaddr+7+n];\
		dst[newadr+8+n] = src[oldaddr+0+0x2000+n];\
		dst[newadr+9+n] = src[oldaddr+1+0x2000+n];\
		dst[newadr+10+n] = src[oldaddr+2+0x2000+n];\
		dst[newadr+11+n] = src[oldaddr+3+0x2000+n];\
		dst[newadr+12+n] = src[oldaddr+4+0x2000+n];\
		dst[newadr+13+n] = src[oldaddr+5+0x2000+n];\
		dst[newadr+14+n] = src[oldaddr+6+0x2000+n];\
		dst[newadr+15+n] = src[oldaddr+7+0x2000+n];\
		dst[newadr+16+n] = src[oldaddr+0+8+n];\
		dst[newadr+17+n] = src[oldaddr+1+8+n];\
		dst[newadr+18+n] = src[oldaddr+2+8+n];\
		dst[newadr+19+n] = src[oldaddr+3+8+n];\
		dst[newadr+20+n] = src[oldaddr+4+8+n];\
		dst[newadr+21+n] = src[oldaddr+5+8+n];\
		dst[newadr+22+n] = src[oldaddr+6+8+n];\
		dst[newadr+23+n] = src[oldaddr+7+8+n];\
		dst[newadr+24+n] = src[oldaddr+0+0x2008+n];\
		dst[newadr+25+n] = src[oldaddr+1+0x2008+n];\
		dst[newadr+26+n] = src[oldaddr+2+0x2008+n];\
		dst[newadr+27+n] = src[oldaddr+3+0x2008+n];\
		dst[newadr+28+n] = src[oldaddr+4+0x2008+n];\
		dst[newadr+29+n] = src[oldaddr+5+0x2008+n];\
		dst[newadr+30+n] = src[oldaddr+6+0x2008+n];\
		dst[newadr+31+n] = src[oldaddr+7+0x2008+n];

	{
		UINT32 oldaddr, newadr, length,j;
		UINT8 *src, *dst, *temp;
		temp = auto_alloc_array(machine, UINT8, 0x10000);
		src = temp;
		dst = memory_region(machine, "gfx1");
		length = memory_region_length(machine, "gfx1");
		memcpy(src, dst, length);
		newadr = 0;
		oldaddr = 0;
		for (j = 0; j < length / 2; j += 32)
		{
			repack(0);
			repack(0x4000)
			newadr += 32;
			oldaddr += 16;
		}
		auto_free(machine, temp);
	}

	memory_configure_bank(machine, "bank1", 0, 3, memory_region(machine, "user1"), 0x4000);
	memory_set_bank(machine, "bank1", 0);
}

GAME( 1984, ddayjlc,  0,       ddayjlc, ddayjlc, ddayjlc, ROT90, "Jaleco", "D-Day (Jaleco set 1)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
GAME( 1984, ddayjlca, ddayjlc, ddayjlc, ddayjlc, ddayjlc, ROT90, "Jaleco", "D-Day (Jaleco set 2)", GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
