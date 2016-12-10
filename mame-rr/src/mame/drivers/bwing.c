/*****************************************************************************

B-Wings  (c) 1984 Data East Corporation
Zaviga   (c) 1984 Data East Corporation

drivers by Acho A. Tang


JUL-2003

Known issues:

- The main program is responsible for sprite clipping but occational
  glitches can be seen at the top and bottom screen edges. (post rotate)

- B-Wings bosses sometimes flicker. (sync issue)

- The text layer has an unknown attribute. (needs verification)

- Zaviga's DIPs are incomplete. (manual missing)

*****************************************************************************/
// Directives

#include "emu.h"
#include "deprecat.h"
#include "cpu/m6809/m6809.h"
#include "cpu/m6502/m6502.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "includes/bwing.h"


//****************************************************************************
// Interrupt Handlers

static INTERRUPT_GEN ( bwp1_interrupt )
{
	bwing_state *state = (bwing_state *)device->machine->driver_data;
	UINT8 latch_data;

	switch (cpu_getiloops(device))
	{
		case 0:
			if (state->ffcount)
			{
				state->ffcount--;
				latch_data = state->sound_fifo[state->fftail];
				state->fftail = (state->fftail + 1) & (MAX_SOUNDS - 1);
				soundlatch_w(cpu_get_address_space(device, ADDRESS_SPACE_PROGRAM), 0, latch_data);
				cpu_set_input_line(state->audiocpu, DECO16_IRQ_LINE, HOLD_LINE); // SNDREQ
			}
		break;

		case 1:
			if (~input_port_read(device->machine, "IN2") & 0x03)
			{
				if (!state->coin)
				{
					state->coin = 1;
					cpu_set_input_line(device, INPUT_LINE_NMI, ASSERT_LINE);
				}
			}
			else
				state->coin = 0;
		break;

		case 2:
			if (input_port_read(device->machine, "IN3"))
				cpu_set_input_line(device, M6809_FIRQ_LINE, ASSERT_LINE);
		break;
	}
}


static INTERRUPT_GEN ( bwp3_interrupt )
{
	bwing_state *state = (bwing_state *)device->machine->driver_data;

	if (!state->bwp3_nmimask)
		cpu_set_input_line(device, INPUT_LINE_NMI, ASSERT_LINE);
}

//****************************************************************************
// Memory and I/O Handlers

static WRITE8_HANDLER( bwp12_sharedram1_w )
{
	bwing_state *state = (bwing_state *)space->machine->driver_data;
	state->bwp1_sharedram1[offset] = state->bwp2_sharedram1[offset] = data;
}

static WRITE8_HANDLER( bwp3_u8F_w )
{
	bwing_state *state = (bwing_state *)space->machine->driver_data;
	state->bwp3_u8F_d = data;  // prepares custom chip for various operations
}

static WRITE8_HANDLER( bwp3_nmimask_w )
{
	bwing_state *state = (bwing_state *)space->machine->driver_data;
	state->bwp3_nmimask = data & 0x80;
}

static WRITE8_HANDLER( bwp3_nmiack_w )
{
	bwing_state *state = (bwing_state *)space->machine->driver_data;
	cpu_set_input_line(state->audiocpu, INPUT_LINE_NMI, CLEAR_LINE);
}


static READ8_HANDLER( bwp1_io_r )
{
	bwing_state *state = (bwing_state *)space->machine->driver_data;

	if (offset == 0) return(input_port_read(space->machine, "DSW0"));
	if (offset == 1) return(input_port_read(space->machine, "DSW1"));
	if (offset == 2) return(input_port_read(space->machine, "IN0"));
	if (offset == 3) return(input_port_read(space->machine, "IN1"));
	if (offset == 4) return(input_port_read(space->machine, "IN2"));

	return((state->bwp123_membase[0])[0x1b00 + offset]);
}


static WRITE8_HANDLER( bwp1_ctrl_w )
{
	bwing_state *state = (bwing_state *)space->machine->driver_data;

	switch (offset)
	{
		// MSSTB
		case 0: cpu_set_input_line(state->subcpu, M6809_IRQ_LINE, ASSERT_LINE); break;

		// IRQACK
		case 1: cpu_set_input_line(state->maincpu, M6809_IRQ_LINE, CLEAR_LINE); break;

		// FIRQACK
		case 2: cpu_set_input_line(state->maincpu, M6809_FIRQ_LINE, CLEAR_LINE); break;

		// NMIACK
		case 3: cpu_set_input_line(state->maincpu, INPUT_LINE_NMI, CLEAR_LINE); break;

		// SWAP(bank-swaps sprite RAM between 1800 & 1900; ignored bc. they're treated as a single chunk.)
		case 4: break;

		// SNDREQ
		case 5:
			if (data == 0x80) // protection trick to screw CPU1 & 3
				cpu_set_input_line(state->subcpu, INPUT_LINE_NMI, ASSERT_LINE); // SNMI
			else if (state->ffcount < MAX_SOUNDS)
			{
				state->ffcount++;
				state->sound_fifo[state->ffhead] = data;
				state->ffhead = (state->ffhead + 1) & (MAX_SOUNDS - 1);
			}
		break;

		// BANKSEL(supposed to bank-switch CPU0 4000-7fff(may also 8000-bfff) 00=bank 0, 80=bank 1, unused)
		case 6: break;

		// hardwired to SWAP
		case 7: break;
	}

	#if BW_DEBUG
		(state->bwp123_membase[0])[0x1c00 + offset] = data;
	#endif
}


static WRITE8_HANDLER( bwp2_ctrl_w )
{
	bwing_state *state = (bwing_state *)space->machine->driver_data;
	switch (offset)
	{
		case 0: cpu_set_input_line(state->maincpu, M6809_IRQ_LINE, ASSERT_LINE); break; // SMSTB

		case 1: cpu_set_input_line(state->subcpu, M6809_FIRQ_LINE, CLEAR_LINE); break;

		case 2: cpu_set_input_line(state->subcpu, M6809_IRQ_LINE, CLEAR_LINE); break;

		case 3: cpu_set_input_line(state->subcpu, INPUT_LINE_NMI, CLEAR_LINE); break;
	}

	#if BW_DEBUG
	{
		(state->bwp123_membase[1])[0x1800 + offset] = data;
	}
	#endif
}

//****************************************************************************
// CPU Memory Maps

// Main CPU
static ADDRESS_MAP_START( bwp1_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x1b00, 0x1b07) AM_READ(bwp1_io_r)
	AM_RANGE(0x0000, 0x07ff) AM_RAM_WRITE(bwp12_sharedram1_w) AM_BASE_MEMBER(bwing_state, bwp1_sharedram1)
	AM_RANGE(0x0800, 0x0fff) AM_RAM
	AM_RANGE(0x1000, 0x13ff) AM_RAM_WRITE(bwing_videoram_w) AM_BASE_MEMBER(bwing_state, videoram)
	AM_RANGE(0x1400, 0x17ff) AM_RAM
	AM_RANGE(0x1800, 0x19ff) AM_RAM_WRITE(bwing_spriteram_w) AM_BASE_GENERIC(buffered_spriteram)
	AM_RANGE(0x1a00, 0x1aff) AM_RAM_WRITE(bwing_paletteram_w) AM_BASE_MEMBER(bwing_state, paletteram)
	AM_RANGE(0x1b00, 0x1b07) AM_RAM_WRITE(bwing_scrollreg_w)
	AM_RANGE(0x1c00, 0x1c07) AM_RAM_WRITE(bwp1_ctrl_w)
	AM_RANGE(0x2000, 0x3fff) AM_READWRITE(bwing_scrollram_r, bwing_scrollram_w)
	AM_RANGE(0x4000, 0xffff) AM_ROM // "B-Wings US" writes to 9631-9632(debug?)
ADDRESS_MAP_END


// Sub CPU
static ADDRESS_MAP_START( bwp2_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_RAM_WRITE(bwp12_sharedram1_w) AM_BASE_MEMBER(bwing_state, bwp2_sharedram1)
	AM_RANGE(0x0800, 0x0fff) AM_RAM
	AM_RANGE(0x1800, 0x1803) AM_WRITE(bwp2_ctrl_w)
	AM_RANGE(0xa000, 0xffff) AM_ROM
ADDRESS_MAP_END


// Sound CPU
static ADDRESS_MAP_START( bwp3_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x01ff) AM_RAM
	AM_RANGE(0x0200, 0x0200) AM_DEVWRITE("dac", dac_signed_w)
	AM_RANGE(0x1000, 0x1000) AM_WRITE(bwp3_nmiack_w)
	AM_RANGE(0x2000, 0x2000) AM_DEVWRITE("ay1", ay8910_data_w)
	AM_RANGE(0x4000, 0x4000) AM_DEVWRITE("ay1", ay8910_address_w)
	AM_RANGE(0x6000, 0x6000) AM_DEVWRITE("ay2", ay8910_data_w)
	AM_RANGE(0x8000, 0x8000) AM_DEVWRITE("ay2", ay8910_address_w)
	AM_RANGE(0xa000, 0xa000) AM_READ(soundlatch_r)
	AM_RANGE(0xd000, 0xd000) AM_WRITE(bwp3_nmimask_w)
	AM_RANGE(0xe000, 0xffff) AM_ROM AM_BASE_SIZE_MEMBER(bwing_state, bwp3_rombase, bwp3_romsize)
ADDRESS_MAP_END


static ADDRESS_MAP_START( bwp3_io_map, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x00, 0x00) AM_READ_PORT("VBLANK") AM_WRITE(bwp3_u8F_w)
ADDRESS_MAP_END

//****************************************************************************
// I/O Port Maps

static INPUT_PORTS_START( bwing )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )		PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )		PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x10, 0x10, "Diagnostics" )		PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Cabinet ) )		PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x40, 0x40, "Invincibility" )		PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Infinite ) )		PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )		PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) )	PORT_DIPLOCATION("SW2:2,3") /* Listed as "Not Used" in the manual */
	PORT_DIPSETTING(    0x00, "40000" )
	PORT_DIPSETTING(    0x02, "20000 80000" )
	PORT_DIPSETTING(    0x04, "20000 60000" )
	PORT_DIPSETTING(    0x06, "20000 40000" )
	PORT_DIPNAME( 0x08, 0x08, "Enemy Crafts" )		PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Normal ) )
	PORT_DIPNAME( 0x10, 0x10, "Enemy Missiles" )		PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPNAME( 0x20, 0x20, "Freeze" )			PORT_DIPLOCATION("SW2:6") /* Listed as "Not Used" in the manual */
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Country" )			PORT_DIPLOCATION("SW2:7") /* Listed as "Not Used" in the manual */
	PORT_DIPSETTING(    0x00, "Japan/US" )
	PORT_DIPSETTING(    0x40, "Japan Only" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )		PORT_DIPLOCATION("SW2:8") /* Listed as "Not Used" in the manual */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_VBLANK )

	PORT_START("IN3")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_TILT )

	PORT_START("VBLANK")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_VBLANK )

	PORT_START("EXTRA") // a matter of taste
	PORT_DIPNAME( 0x07, 0x00, "RGB" )
	PORT_DIPSETTING(    0x00, "Default" )
	PORT_DIPSETTING(    0x01, "More Red" )
	PORT_DIPSETTING(    0x02, "More Green" )
	PORT_DIPSETTING(    0x03, "More Blue" )
	PORT_DIPSETTING(    0x04, "Max" )
INPUT_PORTS_END

//****************************************************************************
// Graphics Layouts

static const gfx_layout charlayout =
{
	8, 8,
	256,
	2,
	{ 0, 0x4000 },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout spritelayout =
{
	16, 16,
	512,
	3,
	{ 0x40000, 0x20000, 0 },
	{ 7, 6, 5, 4, 3, 2, 1, 0, 128+7, 128+6, 128+5, 128+4, 128+3, 128+2, 128+1, 128+0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
	  8*8, 9*8,10*8,11*8,12*8,13*8,14*8,15*8 },
	32*8
};


static GFXDECODE_START( bwing )
	GFXDECODE_ENTRY( "gfx1",  0, charlayout,       0x00, 1 ) // chars
	GFXDECODE_ENTRY( "gfx2",  0, spritelayout,     0x20, 2 ) // sprites
	GFXDECODE_ENTRY( "gpu",   0, bwing_tilelayout, 0x10, 2 ) // foreground tiles place holder
	GFXDECODE_ENTRY( "gpu",   0, bwing_tilelayout, 0x30, 2 ) // background tiles place holder
GFXDECODE_END

//****************************************************************************
// Hardware Definitions

static MACHINE_START( bwing )
{
	bwing_state *state = (bwing_state *)machine->driver_data;

	state->maincpu = machine->device("maincpu");
	state->subcpu = machine->device("sub");
	state->audiocpu = machine->device("audiocpu");

	state_save_register_global(machine, state->coin);
	state_save_register_global(machine, state->palatch);
	state_save_register_global(machine, state->srbank);
	state_save_register_global(machine, state->mapmask);
	state_save_register_global(machine, state->mapflip);
	state_save_register_global(machine, state->bwp3_nmimask);
	state_save_register_global(machine, state->bwp3_u8F_d);
	state_save_register_global(machine, state->ffcount);
	state_save_register_global(machine, state->ffhead);
	state_save_register_global(machine, state->fftail);

	state_save_register_global_array(machine, state->sreg);
	state_save_register_global_array(machine, state->sound_fifo);
}

static MACHINE_RESET( bwing )
{
	bwing_state *state = (bwing_state *)machine->driver_data;
	int i;

	state->coin = 0;
	state->palatch = 0;
	state->srbank = 0;
	state->mapmask = 0;
	state->mapflip = 0;

	for (i = 0; i < MAX_SOUNDS; i++)
		state->sound_fifo[i] = 0;

	state->bwp3_nmimask = 0;
	state->bwp3_u8F_d = 0;
	state->fftail = state->ffhead = state->ffcount = 0;
}

static MACHINE_DRIVER_START( bwing )

	/* driver data */
	MDRV_DRIVER_DATA(bwing_state)

	// basic machine hardware
	MDRV_CPU_ADD("maincpu", M6809, 2000000)
	MDRV_CPU_PROGRAM_MAP(bwp1_map)
	MDRV_CPU_VBLANK_INT_HACK(bwp1_interrupt, 3)

	MDRV_CPU_ADD("sub", M6809, 2000000)
	MDRV_CPU_PROGRAM_MAP(bwp2_map)
//  MDRV_CPU_VBLANK_INT("screen", irq1_line_assert) // vblank triggers FIRQ on CPU2 by design (unused)

	MDRV_CPU_ADD("audiocpu", DECO16, 2000000)
	MDRV_CPU_PROGRAM_MAP(bwp3_map)
	MDRV_CPU_IO_MAP(bwp3_io_map)
	MDRV_CPU_PERIODIC_INT(bwp3_interrupt, 1000)

	MDRV_QUANTUM_TIME(HZ(18000))		// high enough?

	MDRV_MACHINE_START(bwing)
	MDRV_MACHINE_RESET(bwing)

	// video hardware
	MDRV_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(600))	// must be long enough for polling
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(bwing)
	MDRV_PALETTE_LENGTH(64)

	MDRV_VIDEO_START(bwing)
	MDRV_VIDEO_UPDATE(bwing)

	// sound hardware
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ay1", AY8910, 1500000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD("ay2", AY8910, 1500000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD("dac", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)
MACHINE_DRIVER_END

//****************************************************************************
// ROM Maps

ROM_START( bwings )
	// Top Board(SCU-01)
	ROM_REGION( 0x10000, "maincpu", 0 ) // main CPU
	ROM_LOAD( "bw_bv-02-.10a",0x04000, 0x04000, CRC(6074a86b) SHA1(0ce1bd74450144fd3c6556787d6c5c5d4531d830) )  // different
	ROM_LOAD( "bw_bv-01.7a",  0x08000, 0x04000, CRC(b960c707) SHA1(086cb0f22fb59922bf0369bf6b382a241d979ec3) )
	ROM_LOAD( "bw_bv-00-.4a", 0x0c000, 0x04000, CRC(1f83804c) SHA1(afd5eb0822db4fd982062945ca27e66ed9680645) )  // different

	ROM_REGION( 0x10000, "sub", 0 ) // sub CPU
	ROM_LOAD( "bw_bv-06-.10d",0x0a000, 0x02000, CRC(ECA00FCB) SHA1(C7AFFBB900E3940257F8CEBC91266328A4A5DCA3) )  // different
	ROM_LOAD( "bw_bv-05-.9d", 0x0c000, 0x02000, CRC(1e393300) SHA1(8d847256eb5dbccf5f524ec3aa836073d70b4edc) )  // different
	ROM_LOAD( "bw_bv-04-.7d", 0x0e000, 0x02000, CRC(6548c5bb) SHA1(d12cc8d0d5692c3de766f5c42c818dd8f685760a) )  // different

	ROM_REGION( 0x10000, "audiocpu", 0 ) // sound CPU(encrypted)
	ROM_LOAD( "bw_bv-03.13a", 0x0e000, 0x02000, CRC(e8ac9379) SHA1(aaf5c20aa33ed05747a8a27739e9d09e094a518d) )

	// Bottom Board(CCU-01)
	ROM_REGION( 0x01000, "gfx1", 0 ) // chars
	ROM_LOAD( "bw_bv-10.5c",  0x00000, 0x01000, CRC(edca6901) SHA1(402c80e7519cf3a43b9fef52c9923961220a48b6) )

	// Middle Board(MCU-01)
	ROM_REGION( 0x0c000, "gfx2", 0 ) // sprites
	ROM_LOAD( "bw_bv-07.1l",  0x00000, 0x04000, CRC(3d5ab2be) SHA1(2b3a039914ebfcc3993da74853a67546fc22c191) )
	ROM_LOAD( "bw_bv-08.1k",  0x04000, 0x04000, CRC(7a585f1e) SHA1(99e5d947b6b1fa96b90c676a282376d67fc377f0) )
	ROM_LOAD( "bw_bv-09.1h",  0x08000, 0x04000, CRC(a14c0b57) SHA1(5033354793d77922f5ef7f268cbe212e551efadf) )

	// GPU Banks
	ROM_REGION( 0x08000, "gpu", 0 )
	ROM_FILL(0x00000, 0x08000, 0)
ROM_END


ROM_START( bwingso )
	// Top Board(SCU-01)
	ROM_REGION( 0x10000, "maincpu", 0 ) // main CPU
	ROM_LOAD( "bw_bv-02.10a", 0x04000, 0x04000, CRC(5ce74ab5) SHA1(b414f0bbe1c4c5b4c810bb4b9fba16aaf86520ff) )
	ROM_LOAD( "bw_bv-01.7a",  0x08000, 0x04000, CRC(b960c707) SHA1(086cb0f22fb59922bf0369bf6b382a241d979ec3) )
	ROM_LOAD( "bw_bv-00.4a",  0x0c000, 0x04000, CRC(926bef63) SHA1(d4bd2e91fa0abc5e9472d4b684c076bdc3c29f5b) )

	ROM_REGION( 0x10000, "sub", 0 ) // sub CPU
	ROM_LOAD( "bw_bv-06.10d", 0x0a000, 0x02000, CRC(91a21a4c) SHA1(042eed60119a861f6b3ccfbe68d880f182a8a8e1) )
	ROM_LOAD( "bw_bv-05.9d",  0x0c000, 0x02000, CRC(f283f39a) SHA1(9f7f4c39d49f4dfff73fe74cd457480e8a43a3c5) )
	ROM_LOAD( "bw_bv-04.7d",  0x0e000, 0x02000, CRC(29ae75b6) SHA1(48c94e996857f2ac995bcd25f0e67b9f7c17d807) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // sound CPU(encrypted)
	ROM_LOAD( "bw_bv-03.13a", 0x0e000, 0x02000, CRC(e8ac9379) SHA1(aaf5c20aa33ed05747a8a27739e9d09e094a518d) )

	// Bottom Board(CCU-01)
	ROM_REGION( 0x01000, "gfx1", 0 ) // chars
	ROM_LOAD( "bw_bv-10.5c",  0x00000, 0x01000, CRC(edca6901) SHA1(402c80e7519cf3a43b9fef52c9923961220a48b6) )

	// Middle Board(MCU-01)
	ROM_REGION( 0x0c000, "gfx2", 0 ) // sprites
	ROM_LOAD( "bw_bv-07.1l",  0x00000, 0x04000, CRC(3d5ab2be) SHA1(2b3a039914ebfcc3993da74853a67546fc22c191) )
	ROM_LOAD( "bw_bv-08.1k",  0x04000, 0x04000, CRC(7a585f1e) SHA1(99e5d947b6b1fa96b90c676a282376d67fc377f0) )
	ROM_LOAD( "bw_bv-09.1h",  0x08000, 0x04000, CRC(a14c0b57) SHA1(5033354793d77922f5ef7f268cbe212e551efadf) )

	// GPU Banks
	ROM_REGION( 0x08000, "gpu", 0 )
	ROM_FILL(0x00000, 0x08000, 0)
ROM_END


ROM_START( bwingsa )
	// Top Board(SCU-01)
	ROM_REGION( 0x10000, "maincpu", 0 ) // main CPU
	ROM_LOAD( "bw_bv-02.10a", 0x04000, 0x04000, CRC(5ce74ab5) SHA1(b414f0bbe1c4c5b4c810bb4b9fba16aaf86520ff) )
	ROM_LOAD( "bv02.bin",     0x06000, 0x02000, CRC(2f84654e) SHA1(11b5343219b46d03f686ea348181c509121b9e3c) ) // only the lower 8k is different
	ROM_LOAD( "bw_bv-01.7a",  0x08000, 0x04000, CRC(b960c707) SHA1(086cb0f22fb59922bf0369bf6b382a241d979ec3) )
	ROM_LOAD( "bv00.bin",     0x0c000, 0x04000, CRC(0bbc1222) SHA1(cfdf621a423a5ce4ba44a980e683d2abf044d6b9) ) // different

	ROM_REGION( 0x10000, "sub", 0 ) // sub CPU
	ROM_LOAD( "bw_bv-06.10d", 0x0a000, 0x02000, CRC(91a21a4c) SHA1(042eed60119a861f6b3ccfbe68d880f182a8a8e1) )
	ROM_LOAD( "bw_bv-05.9d",  0x0c000, 0x02000, CRC(f283f39a) SHA1(9f7f4c39d49f4dfff73fe74cd457480e8a43a3c5) )
	ROM_LOAD( "bw_bv-04.7d",  0x0e000, 0x02000, CRC(29ae75b6) SHA1(48c94e996857f2ac995bcd25f0e67b9f7c17d807) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // sound CPU(encrypted)
	ROM_LOAD( "bw_bv-03.13a", 0x0e000, 0x02000, CRC(e8ac9379) SHA1(aaf5c20aa33ed05747a8a27739e9d09e094a518d) )

	// Bottom Board(CCU-01)
	ROM_REGION( 0x01000, "gfx1", 0 ) // chars
	ROM_LOAD( "bw_bv-10.5c",  0x00000, 0x01000, CRC(edca6901) SHA1(402c80e7519cf3a43b9fef52c9923961220a48b6) )

	// Middle Board(MCU-01)
	ROM_REGION( 0x0c000, "gfx2", 0 ) // sprites
	ROM_LOAD( "bw_bv-07.1l",  0x00000, 0x04000, CRC(3d5ab2be) SHA1(2b3a039914ebfcc3993da74853a67546fc22c191) )
	ROM_LOAD( "bw_bv-08.1k",  0x04000, 0x04000, CRC(7a585f1e) SHA1(99e5d947b6b1fa96b90c676a282376d67fc377f0) )
	ROM_LOAD( "bw_bv-09.1h",  0x08000, 0x04000, CRC(a14c0b57) SHA1(5033354793d77922f5ef7f268cbe212e551efadf) )

	// GPU Banks
	ROM_REGION( 0x08000, "gpu", 0 )
	ROM_FILL(0x00000, 0x08000, 0)
ROM_END

ROM_START( zaviga )
	// Top Board(DE-0169-0)
	ROM_REGION( 0x10000, "maincpu", 0 ) // main CPU
	ROM_LOAD( "as04", 0x04000, 0x04000, CRC(b79f5da2) SHA1(b39748666d3f7fb1ac46d282cce09fe9531df6b1) )
	ROM_LOAD( "as02", 0x08000, 0x04000, CRC(6addd16a) SHA1(940637c49bf9f38c77176ed2ae212048e9e7fd8f) )
	ROM_LOAD( "as00", 0x0c000, 0x04000, CRC(c6ae4af0) SHA1(6f6f14385b20f9c9c312f816036c608fe8514b00) )

	ROM_REGION( 0x10000, "sub", 0 ) // sub CPU
	ROM_LOAD( "as08", 0x0a000, 0x02000, CRC(b6187b3a) SHA1(d2d7c5b185f59986f45d8ec3ddf9b95364e57d96) )
	ROM_LOAD( "as07", 0x0c000, 0x02000, CRC(dc1170e3) SHA1(c8e4d1564fd272d726d0e4ffd4f33f67f1b37cd7) )
	ROM_LOAD( "as06", 0x0e000, 0x02000, CRC(ba888f84) SHA1(f94de8553cd4704d9b3349ded881a7cc62fa9b57) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // sound CPU(encrypted)
	ROM_LOAD( "as05", 0x0e000, 0x02000, CRC(afe9b0ac) SHA1(3c653cd4fff7f4e00971249900b5a810b6e74dfe) )

	// Bottom Board(DE-0170-0)
	ROM_REGION( 0x01000, "gfx1", 0 ) // chars
	ROM_LOAD( "as14", 0x00000, 0x01000, CRC(62132c1d) SHA1(6b101e220a440488da17de8446f4e2c8ec7c7de9) )

	// Middle Board(DE-0171-0)
	ROM_REGION( 0x0c000, "gfx2", 0 ) // sprites
	ROM_LOAD( "as11", 0x00000, 0x04000, CRC(aa84af24) SHA1(af4ff085dc44b3d1493ec1c8b4a8d18dccecc872) )
	ROM_LOAD( "as12", 0x04000, 0x04000, CRC(84af9041) SHA1(8fbd5995ca8e708cd7fb9cdfcdb174e12084f526) )
	ROM_LOAD( "as13", 0x08000, 0x04000, CRC(15d0922b) SHA1(b8d715a9e610531472d516c19f6035adbce93c84) )

	// GPU Banks
	ROM_REGION( 0x08000, "gpu", 0 )
	ROM_FILL(0x00000, 0x08000, 0)
ROM_END


ROM_START( zavigaj )
	// Top Board(DE-0169-0)
	ROM_REGION( 0x10000, "maincpu", 0 ) // main CPU
	ROM_LOAD( "as04", 0x04000, 0x04000, CRC(b79f5da2) SHA1(b39748666d3f7fb1ac46d282cce09fe9531df6b1) )
	ROM_LOAD( "as02", 0x08000, 0x04000, CRC(6addd16a) SHA1(940637c49bf9f38c77176ed2ae212048e9e7fd8f) )
	ROM_LOAD( "as00", 0x0c000, 0x04000, CRC(c6ae4af0) SHA1(6f6f14385b20f9c9c312f816036c608fe8514b00) )

	ROM_REGION( 0x10000, "sub", 0 ) // sub CPU
	ROM_LOAD( "as08",     0x0a000, 0x02000, CRC(b6187b3a) SHA1(d2d7c5b185f59986f45d8ec3ddf9b95364e57d96) )
	ROM_LOAD( "as07",     0x0c000, 0x02000, CRC(dc1170e3) SHA1(c8e4d1564fd272d726d0e4ffd4f33f67f1b37cd7) )
	ROM_LOAD( "as06-.7d", 0x0e000, 0x02000, CRC(b02d270c) SHA1(beea3d44d367543b5b5075c5892580e690691e75) )  // different

	ROM_REGION( 0x10000, "audiocpu", 0 ) // sound CPU(encrypted)
	ROM_LOAD( "as05", 0x0e000, 0x02000, CRC(afe9b0ac) SHA1(3c653cd4fff7f4e00971249900b5a810b6e74dfe) )

	// Bottom Board(DE-0170-0)
	ROM_REGION( 0x01000, "gfx1", 0 ) // chars
	ROM_LOAD( "as14", 0x00000, 0x01000, CRC(62132c1d) SHA1(6b101e220a440488da17de8446f4e2c8ec7c7de9) )

	// Middle Board(DE-0171-0)
	ROM_REGION( 0x0c000, "gfx2", 0 ) // sprites
	ROM_LOAD( "as11", 0x00000, 0x04000, CRC(aa84af24) SHA1(af4ff085dc44b3d1493ec1c8b4a8d18dccecc872) )
	ROM_LOAD( "as12", 0x04000, 0x04000, CRC(84af9041) SHA1(8fbd5995ca8e708cd7fb9cdfcdb174e12084f526) )
	ROM_LOAD( "as13", 0x08000, 0x04000, CRC(15d0922b) SHA1(b8d715a9e610531472d516c19f6035adbce93c84) )

	// GPU Banks
	ROM_REGION( 0x08000, "gpu", 0 )
	ROM_FILL(0x00000, 0x08000, 0)
ROM_END

//****************************************************************************
// Initializations

static void fix_bwp3( running_machine *machine )
{
	bwing_state *state = (bwing_state *)machine->driver_data;
	UINT8 *rom = state->bwp3_rombase;
	int i, j = state->bwp3_romsize;
	UINT8 ah, al;

	// swap nibbles
	for (i = 0; i < j; i++) { ah = al = rom[i]; rom[i] = (ah >> 4) | (al << 4); }

	// relocate vectors
	rom[j - (0x10 - 0x4)] = rom[j - (0x10 - 0xb)] = rom[j - (0x10 - 0x6)];
	rom[j - (0x10 - 0x5)] = rom[j - (0x10 - 0xa)] = rom[j - (0x10 - 0x7)];
}


static DRIVER_INIT( bwing )
{
	bwing_state *state = (bwing_state *)machine->driver_data;

	state->bwp123_membase[0] = memory_region(machine, "maincpu");
	state->bwp123_membase[1] = memory_region(machine, "sub");
	state->bwp123_membase[2] = memory_region(machine, "audiocpu");

	fix_bwp3(machine);
}

//****************************************************************************
// Game Entries

GAME( 1984, bwings,       0, bwing, bwing, bwing, ROT90, "Data East Corporation", "B-Wings (Japan new Ver.)", 0 )
GAME( 1984, bwingso, bwings, bwing, bwing, bwing, ROT90, "Data East Corporation", "B-Wings (Japan old Ver.)", 0 )
GAME( 1984, bwingsa, bwings, bwing, bwing, bwing, ROT90, "Data East Corporation", "B-Wings (Alt Ver.?)", 0 )

GAME( 1984, zaviga,       0, bwing, bwing, bwing, ROT90, "Data East Corporation", "Zaviga", 0 )
GAME( 1984, zavigaj, zaviga, bwing, bwing, bwing, ROT90, "Data East Corporation", "Zaviga (Japan)", 0 )
