/*********************************************************************************************************************

Some Dynax games using the second version of their blitter

driver by Luca Elia and Nicola Salmoria

CPU:    Z80 or TLCS90
Sound:  [AY] + [YM] + [YM] + [M5205]
VDP:    HD46505SP (6845) (CRT controller)
Custom: TC17G032AP-0246 (blitter)

---------------------------------------------------------------------------------------------------------------------
Year + Game              Main Board   Sub Board    CPU   Sound                      Clock Color Notes
---------------------------------------------------------------------------------------------------------------------
88 Hana no Mai           D1610088L1                Z80   AY8912 YM2203        M5205       PROM
88 Hana Kochou           D201901L2    D201901L1-0  Z80   AY8912 YM2203        M5205       PROM
89 Hana Oriduru          D2304268L                 Z80   AY8912        YM2413 M5205       RAM
89 Dragon Punch          D24?                      Z80          YM2203                    PROM
89 Mj Friday             D2607198L1                Z80                 YM2413             PROM
89 Mj Gekisha            D2809218L-1  D2809218L-2  TLCS                YM2413             PROM  Battery
89 Sports Match          D31?                      Z80          YM2203                    PROM
90 Jong Tou Ki           D1505178-A   D2711078L-B  Z80*2 AY8912 YM2203        M5205       PROM  2 x Blitter
90 Mj Campus Hunting     D3312108L1-1 D23SUB1      Z80   AY8912        YM2413 M5205       RAM
90 Hana Jingi            no number    D3312108L1-2 Z80   AY8912        YM2413 M5205       RAM
90 7jigen no Youseitachi D3707198L1   D23SUB1      Z80   AY8912        YM2413 M5205       RAM
90 Mj Electron Base                                Z80   AY8912        YM2413             RAM
90 Mj X-Tal/Diamond 7    D4005208L1-1 D23SUB       Z80   AY8912        YM2413 M5205       RAM
90 Neruton Haikujiradan  D4005208L1-1 D4508308L-2  Z80   AY8912        YM2413 M5205       RAM
91 Mj Dial Q2            D5212298L-1               Z80                 YM2413             PROM
91 Mj Yarunara           D5512068L1-1 D4508308L-2  Z80   AY8912        YM2413 M5205 M6242 RAM   NL-001
91 Hana wo Yaraneba!     D5512068L1-1 D4508308L-2  Z80   AY8912        YM2413 M5205 M6242 RAM   NL-001, Battery
91 Mj Angels             D5512068L1-1 D6107068L-1  Z80   AY8912        YM2413 M5205       RAM
91 Mj Comic Gekijou V.1  D5512068L1-1 D6107068L-1  Z80   AY8912        YM2413 M5205 M6242 RAM   NL-001, Battery
91 Mj Tenkaigen                                    TLCS  AY8910        YM2413       M6242 RAM   Protection, Battery
92 Quiz TV Gassyuukoku   D5512068L1-2 D6410288L-1  Z80   AY8912        YM2413 M5205       RAM
92 Hanafuda Hana Tengoku D6502208L1   D6107068L-1  Z80   AY8910        YM2413       M6242 RAM
94 Mj Reach (bootleg)    bootleg                   TLCS  AY8910        YM2413       M6242 PROM  Battery
94 Maya                                            Z80          YM2203                    PROM
96 Mj Raijinhai DX       D10010318L1  D10502168    TLCS  AY8910                     M6242 PROM  Undumped TMP91P640 Code, Battery
9? Inca                                            Z80          YM2203                    PROM
---------------------------------------------------------------------------------------------------------------------

Notes:

- In some games (drgpunch,hanamai,hnoridur etc) there's a more complete service mode.
  To enter it, set the service mode dip switch and reset keeping start1 pressed.
  In hnkochou, keep F2 pressed and reset.

- sprtmtch and drgpunch are "clones", but the gfx are very different; sprtmtch
  is a trimmed down version, without all animations between levels.

- according to the readme, mjfriday should have a M5205. However there don't seem to be
  accesses to it, and looking at the ROMs I don't see ADPCM data. Note that apart from a
  minor difference in the memory map mjfriday and mjdialq2 are identical, and mjdialq2
  doesn't have a 5205 either. Therefore, I think it's either a mistake in the readme or
  the chip is on the board but unused.

TODO:

- Palette banking is not correct, see quiztvqq cross hatch test.

- Scrolling / wrap enable is not correct in hnoridur type hardware. See the dynax
  logo in neruton: it has to do with writes to c3/c4 and there are 2 additional
  scroll registers at 64/66.

- 7jigen: priority 0x30 is ok when used in the "gals check", but is wrong during
  attract mode, where the girl is hidden by the background. Another possible
  priority issue in attract mode is when the balls scroll over the devil.

- neruton / majxtal7: girls are behind the background in demo mode.

*********************************************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "deprecat.h"
#include "includes/dynax.h"
#include "cpu/tlcs90/tlcs90.h"
#include "machine/msm6242.h"
#include "sound/ay8910.h"
#include "sound/2203intf.h"
#include "sound/3812intf.h"
#include "sound/msm5205.h"
#include "sound/2413intf.h"
#include "rendlay.h"

/***************************************************************************


                                Interrupts


***************************************************************************/

/***************************************************************************
                                Sports Match
***************************************************************************/


/* It runs in IM 0, thus needs an opcode on the data bus */
void sprtmtch_update_irq( running_machine *machine )
{
	dynax_state *state = (dynax_state *)machine->driver_data;
	int irq = (state->sound_irq ? 0x08 : 0) | ((state->vblank_irq) ? 0x10 : 0) | ((state->blitter_irq) ? 0x20 : 0) ;
	cpu_set_input_line_and_vector(state->maincpu, 0, irq ? ASSERT_LINE : CLEAR_LINE, 0xc7 | irq); /* rst $xx */
}

static WRITE8_HANDLER( dynax_vblank_ack_w )
{
	dynax_state *state = (dynax_state *)space->machine->driver_data;
	state->vblank_irq = 0;
	sprtmtch_update_irq(space->machine);
}

static WRITE8_HANDLER( dynax_blitter_ack_w )
{
	dynax_state *state = (dynax_state *)space->machine->driver_data;
	state->blitter_irq = 0;
	sprtmtch_update_irq(space->machine);
}

static INTERRUPT_GEN( sprtmtch_vblank_interrupt )
{
	dynax_state *state = (dynax_state *)device->machine->driver_data;
	state->vblank_irq = 1;
	sprtmtch_update_irq(device->machine);
}

static void sprtmtch_sound_callback( running_device *device, int state )
{
	dynax_state *driver_state = (dynax_state *)device->machine->driver_data;
	driver_state->sound_irq = state;
	sprtmtch_update_irq(device->machine);
}


/***************************************************************************
                            Jantouki - Main CPU
***************************************************************************/

/* It runs in IM 0, thus needs an opcode on the data bus */
void jantouki_update_irq(running_machine *machine)
{
	dynax_state *state = (dynax_state *)machine->driver_data;
	int irq = ((state->blitter_irq) ? 0x08 : 0) | ((state->blitter2_irq) ? 0x10 : 0) | ((state->vblank_irq) ? 0x20 : 0) ;
	cpu_set_input_line_and_vector(state->maincpu, 0, irq ? ASSERT_LINE : CLEAR_LINE, 0xc7 | irq); /* rst $xx */
}

static WRITE8_HANDLER( jantouki_vblank_ack_w )
{
	dynax_state *state = (dynax_state *)space->machine->driver_data;
	state->vblank_irq = 0;
	jantouki_update_irq(space->machine);
}

static WRITE8_HANDLER( jantouki_blitter_ack_w )
{
	dynax_state *state = (dynax_state *)space->machine->driver_data;
	state->blitter_irq = data;
	jantouki_update_irq(space->machine);
}

static WRITE8_HANDLER( jantouki_blitter2_ack_w )
{
	dynax_state *state = (dynax_state *)space->machine->driver_data;
	state->blitter2_irq = data;
	jantouki_update_irq(space->machine);
}

static INTERRUPT_GEN( jantouki_vblank_interrupt )
{
	dynax_state *state = (dynax_state *)device->machine->driver_data;
	state->vblank_irq = 1;
	jantouki_update_irq(device->machine);
}


/***************************************************************************
                            Jantouki - Sound CPU
***************************************************************************/

static void jantouki_sound_update_irq(running_machine *machine)
{
	dynax_state *state = (dynax_state *)machine->driver_data;
	int irq = ((state->sound_irq) ? 0x08 : 0) | ((state->soundlatch_irq) ? 0x10 : 0) | ((state->sound_vblank_irq) ? 0x20 : 0) ;
	cpu_set_input_line_and_vector(state->soundcpu, 0, irq ? ASSERT_LINE : CLEAR_LINE, 0xc7 | irq); /* rst $xx */
}

static INTERRUPT_GEN( jantouki_sound_vblank_interrupt )
{
	dynax_state *state = (dynax_state *)device->machine->driver_data;
	state->sound_vblank_irq = 1;
	jantouki_sound_update_irq(device->machine);
}

static WRITE8_HANDLER( jantouki_sound_vblank_ack_w )
{
	dynax_state *state = (dynax_state *)space->machine->driver_data;
	state->sound_vblank_irq = 0;
	jantouki_sound_update_irq(space->machine);
}

static void jantouki_sound_callback(running_device *device, int state)
{
	dynax_state *driver_state = (dynax_state *)device->machine->driver_data;
	driver_state->sound_irq = state;
	jantouki_sound_update_irq(device->machine);
}


/***************************************************************************


                                Memory Maps


***************************************************************************/

/***************************************************************************
                                Sports Match
***************************************************************************/

static WRITE8_HANDLER( dynax_coincounter_0_w )
{
	coin_counter_w(space->machine, 0, data);
}

static WRITE8_HANDLER( dynax_coincounter_1_w )
{
	coin_counter_w(space->machine, 1, data);
}

static READ8_HANDLER( ret_ff )
{
	return 0xff;
}


static READ8_HANDLER( hanamai_keyboard_0_r )
{
	dynax_state *state = (dynax_state *)space->machine->driver_data;
	int res = 0x3f;

	/* the game reads all rows at once (keyb = 0) to check if a key is pressed */
	if (!BIT(state->keyb, 0)) res &= input_port_read(space->machine, "KEY0");
	if (!BIT(state->keyb, 1)) res &= input_port_read(space->machine, "KEY1");
	if (!BIT(state->keyb, 2)) res &= input_port_read(space->machine, "KEY2");
	if (!BIT(state->keyb, 3)) res &= input_port_read(space->machine, "KEY3");
	if (!BIT(state->keyb, 4)) res &= input_port_read(space->machine, "KEY4");

	return res;
}

static READ8_HANDLER( hanamai_keyboard_1_r )
{
	dynax_state *state = (dynax_state *)space->machine->driver_data;
	int res = 0x3f;

	/* the game reads all rows at once (keyb = 0) to check if a key is pressed */
	if (!BIT(state->keyb, 0)) res &= input_port_read(space->machine, "KEY5");
	if (!BIT(state->keyb, 1)) res &= input_port_read(space->machine, "KEY6");
	if (!BIT(state->keyb, 2)) res &= input_port_read(space->machine, "KEY7");
	if (!BIT(state->keyb, 3)) res &= input_port_read(space->machine, "KEY8");
	if (!BIT(state->keyb, 4)) res &= input_port_read(space->machine, "KEY9");

	return res;
}

static WRITE8_HANDLER( hanamai_keyboard_w )
{
	dynax_state *state = (dynax_state *)space->machine->driver_data;
	state->keyb = data;
}


static WRITE8_HANDLER( dynax_rombank_w )
{
	memory_set_bank(space->machine, "bank1", data & 0x0f);
}

static WRITE8_HANDLER( jantouki_sound_rombank_w )
{
	memory_set_bank(space->machine, "bank2", data);
}


static WRITE8_HANDLER( hnoridur_rombank_w )
{
	dynax_state *state = (dynax_state *)space->machine->driver_data;
	int bank_n = (memory_region_length(space->machine, "maincpu") - 0x10000) / 0x8000;

	//logerror("%04x: rom bank = %02x\n", cpu_get_pc(space->cpu), data);
	if (data < bank_n)
		memory_set_bank(space->machine, "bank1", data);
	else
		logerror("rom_bank = %02x (larger than the maximum bank %02x)\n", data, bank_n);
	state->hnoridur_bank = data;
}


static WRITE8_HANDLER( hnoridur_palbank_w )
{
	dynax_state *state = (dynax_state *)space->machine->driver_data;
	state->palbank = data & 0x0f;
	dynax_blit_palbank_w(space, 0, data);
}

static WRITE8_HANDLER( hnoridur_palette_w )
{
	dynax_state *state = (dynax_state *)space->machine->driver_data;

	switch (state->hnoridur_bank)
	{
		case 0x10:
			if (offset >= 0x100)
				return;
			state->palette_ram[256 * state->palbank + offset + 16 * 256] = data;
			break;

		case 0x14:
			if (offset >= 0x100)
				return;
			state->palette_ram[256 * state->palbank + offset] = data;
			break;

		// hnoridur: R/W RAM
		case 0x18:
		{
			UINT8 *RAM = memory_region(space->machine, "maincpu") + 0x10000 + state->hnoridur_bank * 0x8000;
			RAM[offset] = data;
			return;
		}

		default:
			popmessage("palette_w with bank = %02x", state->hnoridur_bank);
			break;
	}

	{
		int x = (state->palette_ram[256 * state->palbank + offset] << 8) + state->palette_ram[256 * state->palbank + offset + 16 * 256];
		/* The bits are in reverse order! */
		int r = BITSWAP8((x >>  0) & 0x1f, 7, 6, 5, 0, 1, 2, 3, 4);
		int g = BITSWAP8((x >>  5) & 0x1f, 7, 6, 5, 0, 1, 2, 3, 4);
		int b = BITSWAP8((x >> 10) & 0x1f, 7, 6, 5, 0, 1, 2, 3, 4);
		palette_set_color_rgb(space->machine, 256 * state->palbank + offset, pal5bit(r), pal5bit(g), pal5bit(b));
	}
}

static WRITE8_HANDLER( yarunara_palette_w )
{
	dynax_state *state = (dynax_state *)space->machine->driver_data;
	int addr = 512 * state->palbank + offset;

	switch (state->hnoridur_bank)
	{
		case 0x10:
			state->palette_ram[addr] = data;
			break;

		case 0x1c:	// RTC
		{
			msm6242_w(state->rtc, offset, data);
		}
		return;

		default:
			popmessage("palette_w with bank = %02x", state->hnoridur_bank);
			return;
	}

	{
		int br = state->palette_ram[addr & ~0x10];		// bbbrrrrr
		int bg = state->palette_ram[addr | 0x10];		// bb0ggggg
		int r = br & 0x1f;
		int g = bg & 0x1f;
		int b = ((bg & 0xc0) >> 3) | ((br & 0xe0) >> 5);
		palette_set_color_rgb(space->machine, 256 * state->palbank + ((offset & 0x0f) | ((offset & 0x1e0) >> 1)), pal5bit(r), pal5bit(g), pal5bit(b));
	}
}

static WRITE8_HANDLER( nanajign_palette_w )
{
	dynax_state *state = (dynax_state *)space->machine->driver_data;

	switch (state->hnoridur_bank)
	{
		case 0x10:
			state->palette_ram[256 * state->palbank + offset + 16 * 256] = data;
			break;

		case 0x14:
			state->palette_ram[256 * state->palbank + offset] = data;
			break;

		default:
			popmessage("palette_w with bank = %02x", state->hnoridur_bank);
			break;
	}

	{
		int bg = state->palette_ram[256 * state->palbank + offset];
		int br = state->palette_ram[256 * state->palbank + offset + 16 * 256];
		int r = br & 0x1f;
		int g = bg & 0x1f;
		int b = ((bg & 0xc0) >> 3) | ((br & 0xe0) >> 5);
		palette_set_color_rgb(space->machine, 256 * state->palbank + offset, pal5bit(r), pal5bit(g), pal5bit(b));
	}
}


static void adpcm_int( running_device *device )
{
	dynax_state *state = (dynax_state *)device->machine->driver_data;
	msm5205_data_w(device, state->msm5205next >> 4);
	state->msm5205next <<= 4;

	state->toggle = 1 - state->toggle;

	if (state->toggle)
	{
		if (state->resetkludge)	// don't know what's wrong, but NMIs when the 5205 is reset make the game crash
		cpu_set_input_line(state->maincpu, INPUT_LINE_NMI, PULSE_LINE);
	}
}

static void adpcm_int_cpu1( running_device *device )
{
	dynax_state *state = (dynax_state *)device->machine->driver_data;
	msm5205_data_w(device, state->msm5205next >> 4);
	state->msm5205next <<= 4;

	state->toggle_cpu1 = 1 - state->toggle_cpu1;
	if (state->toggle_cpu1)
	{
		if (state->resetkludge)	// don't know what's wrong, but NMIs when the 5205 is reset make the game crash
		cpu_set_input_line(state->soundcpu, INPUT_LINE_NMI, PULSE_LINE);	// cpu1
	}
}


static WRITE8_HANDLER( adpcm_data_w )
{
	dynax_state *state = (dynax_state *)space->machine->driver_data;
	state->msm5205next = data;
}

static WRITE8_DEVICE_HANDLER( adpcm_reset_w )
{
	dynax_state *state = (dynax_state *)device->machine->driver_data;
	state->resetkludge = data & 1;
	msm5205_reset_w(device, ~data & 1);
}

static MACHINE_RESET( adpcm )
{
	dynax_state *state = (dynax_state *)machine->driver_data;
	/* start with the MSM5205 reset */
	state->resetkludge = 0;
	msm5205_reset_w(machine->device("msm"), 1);
}

static WRITE8_HANDLER( yarunara_layer_half_w )
{
	hanamai_layer_half_w(space, 0, data >> 1);
}

static WRITE8_HANDLER( yarunara_layer_half2_w )
{
	hnoridur_layer_half2_w(space, 0, data >> 1);
}

static ADDRESS_MAP_START( sprtmtch_mem_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE( 0x0000, 0x6fff ) AM_ROM
	AM_RANGE( 0x7000, 0x7fff ) AM_RAM AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE( 0x8000, 0xffff ) AM_ROMBANK("bank1")
ADDRESS_MAP_END

static ADDRESS_MAP_START( hnoridur_mem_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE( 0x0000, 0x6fff ) AM_ROM
	AM_RANGE( 0x7000, 0x7fff ) AM_RAM AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE( 0x8000, 0xffff ) AM_READ_BANK("bank1") AM_WRITE(hnoridur_palette_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( mcnpshnt_mem_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE( 0x0000, 0x5fff ) AM_ROM
	AM_RANGE( 0x6000, 0x6fff ) AM_RAM
	AM_RANGE( 0x7000, 0x7fff ) AM_RAM AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE( 0x8000, 0xffff ) AM_READ_BANK("bank1") AM_WRITE(hnoridur_palette_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( nanajign_mem_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE( 0x0000, 0x5fff ) AM_ROM
	AM_RANGE( 0x6000, 0x6fff ) AM_RAM
	AM_RANGE( 0x7000, 0x7fff ) AM_RAM AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE( 0x8000, 0x80ff ) AM_WRITE(nanajign_palette_w)
	AM_RANGE( 0x8000, 0xffff ) AM_ROMBANK("bank1")
ADDRESS_MAP_END

static ADDRESS_MAP_START( mjdialq2_mem_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE( 0x0800, 0x0fff ) AM_RAM
	AM_RANGE( 0x1000, 0x1fff ) AM_RAM AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE( 0x0000, 0x7fff ) AM_ROM
	AM_RANGE( 0x8000, 0xffff ) AM_ROMBANK("bank1")
ADDRESS_MAP_END

static ADDRESS_MAP_START( yarunara_mem_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE( 0x0000, 0x5fff ) AM_ROM
	AM_RANGE( 0x6000, 0x6fff ) AM_RAM
	AM_RANGE( 0x7000, 0x7fff ) AM_RAM AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE( 0x8000, 0xffff ) AM_ROMBANK("bank1")
	AM_RANGE( 0x8000, 0x81ff ) AM_WRITE(yarunara_palette_w)	// Palette or RTC
ADDRESS_MAP_END

static ADDRESS_MAP_START( jantouki_mem_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE( 0x0000, 0x5fff ) AM_ROM
	AM_RANGE( 0x6000, 0x6fff ) AM_RAM
	AM_RANGE( 0x7000, 0x7fff ) AM_RAM AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE( 0x8000, 0xffff ) AM_ROMBANK("bank1")
ADDRESS_MAP_END

static ADDRESS_MAP_START( jantouki_sound_mem_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE( 0x0000, 0x6fff ) AM_ROM
	AM_RANGE( 0x7000, 0x7fff ) AM_RAM
	AM_RANGE( 0x8000, 0xffff ) AM_ROMBANK("bank2")
ADDRESS_MAP_END



static ADDRESS_MAP_START( hanamai_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x00, 0x00 ) AM_WRITE(dynax_extra_scrollx_w)	// screen scroll X
	AM_RANGE( 0x20, 0x20 ) AM_WRITE(dynax_extra_scrolly_w)	// screen scroll Y
	AM_RANGE( 0x41, 0x47 ) AM_WRITE(dynax_blitter_rev2_w)		// Blitter
	AM_RANGE( 0x50, 0x50 ) AM_WRITE(dynax_rombank_w)		// BANK ROM Select  hnkochou only
	AM_RANGE( 0x60, 0x60 ) AM_READ(hanamai_keyboard_0_r)		// P1
	AM_RANGE( 0x61, 0x61 ) AM_READ(hanamai_keyboard_1_r)		// P2
	AM_RANGE( 0x62, 0x62 ) AM_READ_PORT("COINS")			// Coins
	AM_RANGE( 0x63, 0x63 ) AM_READ(ret_ff)				// ?
	AM_RANGE( 0x64, 0x64 ) AM_WRITE(hanamai_keyboard_w)		// keyboard row select
	AM_RANGE( 0x65, 0x65 ) AM_WRITE(dynax_rombank_w)		// BANK ROM Select  hanamai only
	AM_RANGE( 0x66, 0x66 ) AM_WRITE(dynax_vblank_ack_w)		// VBlank IRQ Ack
	AM_RANGE( 0x67, 0x67 ) AM_WRITE(adpcm_data_w)			// MSM5205 data
	AM_RANGE( 0x68, 0x68 ) AM_WRITE(dynax_layer_enable_w)		// Layers Enable
	AM_RANGE( 0x69, 0x69 ) AM_WRITE(hanamai_priority_w)		// layer priority
	AM_RANGE( 0x6a, 0x6a ) AM_WRITE(dynax_blit_dest_w)		// Destination Layer
	AM_RANGE( 0x6b, 0x6b ) AM_WRITE(dynax_blit_pen_w)		// Destination Pen
	AM_RANGE( 0x6c, 0x6c ) AM_WRITE(dynax_blit_palette01_w)	// Layers Palettes (Low Bits)
	AM_RANGE( 0x6d, 0x6d ) AM_WRITE(dynax_blit_palette23_w)	//
	AM_RANGE( 0x6e, 0x6e ) AM_WRITE(dynax_blit_backpen_w)		// Background Color
	AM_RANGE( 0x70, 0x70 ) AM_DEVWRITE("msm", adpcm_reset_w)	// MSM5205 reset
	AM_RANGE( 0x71, 0x71 ) AM_WRITE(dynax_flipscreen_w)		// Flip Screen
	AM_RANGE( 0x72, 0x72 ) AM_WRITE(dynax_coincounter_0_w)	// Coin Counters
	AM_RANGE( 0x73, 0x73 ) AM_WRITE(dynax_coincounter_1_w)	//
	AM_RANGE( 0x74, 0x74 ) AM_WRITE(dynax_blitter_ack_w)		// Blitter IRQ Ack
	AM_RANGE( 0x76, 0x76 ) AM_WRITE(dynax_blit_palbank_w)		// Layers Palettes (High Bit)
	AM_RANGE( 0x77, 0x77 ) AM_WRITE(hanamai_layer_half_w)		// half of the interleaved layer to write to
	AM_RANGE( 0x78, 0x79 ) AM_DEVREADWRITE("ymsnd", ym2203_r, ym2203_w)	// 2 x DSW
	AM_RANGE( 0x7a, 0x7b ) AM_DEVWRITE("aysnd", ay8910_address_data_w)	// AY8910
//  AM_RANGE( 0x7c, 0x7c ) AM_WRITENOP   // CRT Controller
//  AM_RANGE( 0x7d, 0x7d ) AM_WRITENOP   //
	AM_RANGE( 0x7e, 0x7e ) AM_WRITE(dynax_blit_romregion_w)	// Blitter ROM bank
ADDRESS_MAP_END


static ADDRESS_MAP_START( hnoridur_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x01, 0x07 ) AM_WRITE(dynax_blitter_rev2_w)		// Blitter
//  AM_RANGE( 0x10, 0x10 ) AM_WRITENOP   // CRT Controller
//  AM_RANGE( 0x11, 0x11 ) AM_WRITENOP   // CRT Controller
	AM_RANGE( 0x20, 0x20 ) AM_WRITE(hanamai_keyboard_w)		// keyboard row select
	AM_RANGE( 0x21, 0x21 ) AM_READ_PORT("COINS")			// Coins
	AM_RANGE( 0x22, 0x22 ) AM_READ(hanamai_keyboard_1_r)		// P2
	AM_RANGE( 0x23, 0x23 ) AM_READ(hanamai_keyboard_0_r)		// P1
	AM_RANGE( 0x24, 0x24 ) AM_READ_PORT("DSW1")			// DSW2
	AM_RANGE( 0x25, 0x25 ) AM_READ_PORT("DSW3")			// DSW4
	AM_RANGE( 0x26, 0x26 ) AM_READ_PORT("DSW2")			// DSW3
	AM_RANGE( 0x30, 0x30 ) AM_DEVWRITE("msm", adpcm_reset_w)	// MSM5205 reset
	AM_RANGE( 0x32, 0x32 ) AM_WRITE(adpcm_data_w)			// MSM5205 data
	AM_RANGE( 0x34, 0x35 ) AM_DEVWRITE("ymsnd", ym2413_w)		//
	AM_RANGE( 0x36, 0x36 ) AM_DEVREAD("aysnd", ay8910_r)		// AY8910, DSW1
	AM_RANGE( 0x38, 0x38 ) AM_DEVWRITE("aysnd", ay8910_data_w)	// AY8910
	AM_RANGE( 0x3a, 0x3a ) AM_DEVWRITE("aysnd", ay8910_address_w)	//
	AM_RANGE( 0x40, 0x40 ) AM_WRITE(dynax_blit_pen_w)		// Destination Pen
	AM_RANGE( 0x41, 0x41 ) AM_WRITE(dynax_blit_dest_w)		// Destination Layer
	AM_RANGE( 0x42, 0x42 ) AM_WRITE(dynax_blit_palette01_w)	// Layers Palettes
	AM_RANGE( 0x43, 0x43 ) AM_WRITE(dynax_blit_palette23_w)	//
	AM_RANGE( 0x44, 0x44 ) AM_WRITE(hanamai_priority_w)		// layer priority and enable
	AM_RANGE( 0x45, 0x45 ) AM_WRITE(dynax_blit_backpen_w)		// Background Color
	AM_RANGE( 0x46, 0x46 ) AM_WRITE(dynax_blit_romregion_w)	// Blitter ROM bank
	AM_RANGE( 0x47, 0x47 ) AM_WRITE(hnoridur_palbank_w)
	AM_RANGE( 0x50, 0x50 ) AM_WRITE(dynax_extra_scrollx_w)	// screen scroll X
	AM_RANGE( 0x51, 0x51 ) AM_WRITE(dynax_extra_scrolly_w)	// screen scroll Y
	AM_RANGE( 0x54, 0x54 ) AM_WRITE(hnoridur_rombank_w)		// BANK ROM Select
	AM_RANGE( 0x55, 0x55 ) AM_WRITENOP					// ? VBlank IRQ Ack
	AM_RANGE( 0x56, 0x56 ) AM_WRITE(dynax_vblank_ack_w)		// VBlank IRQ Ack
	AM_RANGE( 0x57, 0x57 ) AM_READ(ret_ff)				// ?
	AM_RANGE( 0x60, 0x60 ) AM_WRITE(dynax_flipscreen_w)		// Flip Screen
	AM_RANGE( 0x61, 0x61 ) AM_WRITE(hanamai_layer_half_w)		// half of the interleaved layer to write to
	AM_RANGE( 0x62, 0x62 ) AM_WRITE(hnoridur_layer_half2_w)	//
	AM_RANGE( 0x67, 0x67 ) AM_WRITE(dynax_blitter_ack_w)		// Blitter IRQ Ack
	AM_RANGE( 0x70, 0x70 ) AM_WRITE(dynax_coincounter_0_w)	// Coin Counters
	AM_RANGE( 0x71, 0x71 ) AM_WRITE(dynax_coincounter_1_w)	//
ADDRESS_MAP_END

/***************************************************************************
                                Hana Jingi
***************************************************************************/

static WRITE8_HANDLER( hjingi_bank_w )
{
	dynax_state *state = (dynax_state *)space->machine->driver_data;
	state->hnoridur_bank = data;
}

static WRITE8_HANDLER( hjingi_lockout_w )
{
	coin_lockout_w(space->machine, 0, (~data) & 0x01);
}

static WRITE8_HANDLER( hjingi_hopper_w )
{
	dynax_state *state = (dynax_state *)space->machine->driver_data;
	state->hopper = data & 0x01;
}

static UINT8 hjingi_hopper_bit( running_machine *machine )
{
	dynax_state *state = (dynax_state *)machine->driver_data;
	return (state->hopper && !(machine->primary_screen->frame_number() % 10)) ? 0 : (1 << 6);
}

static READ8_HANDLER( hjingi_keyboard_0_r )
{
	return hanamai_keyboard_0_r(space, 0) | hjingi_hopper_bit(space->machine);
}

static READ8_HANDLER( hjingi_keyboard_1_r )
{
	return hanamai_keyboard_1_r(space, 0) | input_port_read(space->machine, "BET");
}

static ADDRESS_MAP_START( hjingi_mem_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE( 0x0000, 0x01ff ) AM_ROM
	AM_RANGE( 0x0200, 0x1fff ) AM_RAM AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE( 0x2000, 0x7fff ) AM_ROM
	AM_RANGE( 0x8000, 0xffff ) AM_READ_BANK("bank1") AM_WRITE(hnoridur_palette_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( hjingi_io_map, ADDRESS_SPACE_IO, 8 )	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x01, 0x07 ) AM_WRITE(dynax_blitter_rev2_w)		// Blitter

//  AM_RANGE( 0x10, 0x10 ) AM_WRITENOP   // CRT Controller
//  AM_RANGE( 0x11, 0x11 ) AM_WRITENOP   // CRT Controller

	AM_RANGE( 0x20, 0x20 ) AM_WRITE(hanamai_keyboard_w)		// keyboard row select
	AM_RANGE( 0x21, 0x21 ) AM_READ_PORT("COINS")			// Coins
	AM_RANGE( 0x22, 0x22 ) AM_READ(hjingi_keyboard_1_r)		// P2 + Hopper
	AM_RANGE( 0x23, 0x23 ) AM_READ(hjingi_keyboard_0_r)		// P1 + Hopper
	AM_RANGE( 0x24, 0x24 ) AM_READ_PORT("DSW1")			// DSW1
	AM_RANGE( 0x25, 0x25 ) AM_READ_PORT("DSW3")			// DSW3
	AM_RANGE( 0x26, 0x26 ) AM_READ_PORT("DSW2")			// DSW2

	AM_RANGE( 0x30, 0x30 ) AM_DEVWRITE("msm", adpcm_reset_w)	// MSM5205 reset
	AM_RANGE( 0x32, 0x32 ) AM_WRITE(adpcm_data_w)			// MSM5205 data
	AM_RANGE( 0x34, 0x35 ) AM_DEVWRITE("ymsnd", ym2413_w)		//

	AM_RANGE( 0x36, 0x36 ) AM_DEVREAD("aysnd", ay8910_r)		// AY8910, DSW1
	AM_RANGE( 0x38, 0x38 ) AM_DEVWRITE("aysnd", ay8910_data_w)	// AY8910
	AM_RANGE( 0x3a, 0x3a ) AM_DEVWRITE("aysnd", ay8910_address_w)	//

	AM_RANGE( 0x40, 0x40 ) AM_WRITE(dynax_blit_pen_w)		// Destination Pen
	AM_RANGE( 0x41, 0x41 ) AM_WRITE(dynax_blit_dest_w)		// Destination Layer
	AM_RANGE( 0x42, 0x42 ) AM_WRITE(dynax_blit_palette01_w)	// Layers Palettes
	AM_RANGE( 0x43, 0x43 ) AM_WRITE(dynax_blit_palette23_w)	//
	AM_RANGE( 0x44, 0x44 ) AM_WRITE(hanamai_priority_w)		// layer priority and enable
	AM_RANGE( 0x45, 0x45 ) AM_WRITE(dynax_blit_backpen_w)		// Background Color
	AM_RANGE( 0x46, 0x46 ) AM_WRITE(dynax_blit_romregion_w)	// Blitter ROM bank
	AM_RANGE( 0x47, 0x47 ) AM_WRITE(hnoridur_palbank_w)

	AM_RANGE( 0x50, 0x50 ) AM_WRITE(dynax_extra_scrollx_w)	// screen scroll X
	AM_RANGE( 0x51, 0x51 ) AM_WRITE(dynax_extra_scrolly_w)	// screen scroll Y

	AM_RANGE( 0x54, 0x54 ) AM_WRITE(hjingi_bank_w)			//

	AM_RANGE( 0x56, 0x56 ) AM_WRITE(dynax_vblank_ack_w)		// VBlank IRQ Ack
	AM_RANGE( 0x57, 0x57 ) AM_READ(ret_ff)				// Blitter Busy
	AM_RANGE( 0x67, 0x67 ) AM_WRITE(dynax_blitter_ack_w)		// Blitter IRQ Ack

	AM_RANGE( 0x60, 0x60 ) AM_WRITE(dynax_flipscreen_w)		// Flip Screen
	AM_RANGE( 0x61, 0x61 ) AM_WRITE(hanamai_layer_half_w)		// half of the interleaved layer to write to
	AM_RANGE( 0x62, 0x62 ) AM_WRITE(hnoridur_layer_half2_w)	//

	AM_RANGE( 0x70, 0x70 ) AM_WRITE(dynax_coincounter_0_w)	// Coin Counters
	AM_RANGE( 0x71, 0x71 ) AM_WRITE(dynax_coincounter_1_w)	//
	AM_RANGE( 0x72, 0x72 ) AM_WRITE(hjingi_hopper_w)		// Hopper
	AM_RANGE( 0x73, 0x73 ) AM_WRITE(hjingi_lockout_w)		// Coin Lockout

	AM_RANGE( 0x80, 0x80 ) AM_WRITE(hnoridur_rombank_w)		// BANK ROM Select
ADDRESS_MAP_END


/***************************************************************************
                    Yarunara / Quiz TV Q&Q / Mahjong Angels
***************************************************************************/

static WRITE8_HANDLER( yarunara_input_w )
{
	dynax_state *state = (dynax_state *)space->machine->driver_data;

	switch (offset)
	{
		case 0:	state->input_sel = data;
				state->keyb = 0;
				break;

		case 1:	break;
	}

}

static READ8_HANDLER( yarunara_input_r )
{
	dynax_state *state = (dynax_state *)space->machine->driver_data;
	static const char *const keynames0[] = { "KEY0", "KEY1", "KEY2", "KEY3", "KEY4" };
	static const char *const keynames1[] = { "KEY5", "KEY6", "KEY7", "KEY8", "KEY9" };

	switch (offset)
	{
		case 0:
		{
			switch (state->input_sel)
			{
			case 0x00:
				return input_port_read(space->machine, "COINS");	// coins

			case 0x02:
				return 0xff;	// bit 7 must be 1. Bit 2?

			default:
				return 0xff;
			}
		}

		case 1:
		{
			switch (state->input_sel)
			{
			// player 2
			case 0x01:	//quiztvqq
			case 0x81:
				return input_port_read(space->machine, keynames1[state->keyb++]);

			// player 1
			case 0x02:	//quiztvqq
			case 0x82:
				return input_port_read(space->machine, keynames0[state->keyb++]);

			default:
				return 0xff;
			}
		}
	}
	return 0xff;
}

static WRITE8_HANDLER( yarunara_rombank_w )
{
	dynax_state *state = (dynax_state *)space->machine->driver_data;
	memory_set_bank(space->machine, "bank1", data);

	state->hnoridur_bank = data;
}

static WRITE8_HANDLER( yarunara_flipscreen_w )
{
	dynax_flipscreen_w(space, 0, BIT(data, 1));
}

static WRITE8_HANDLER( yarunara_flipscreen_inv_w )
{
	dynax_flipscreen_w(space, 0, !BIT(data, 1));
}

static WRITE8_HANDLER( yarunara_blit_romregion_w )
{
	switch(data)
	{
		case 0x00:	dynax_blit_romregion_w(space, 0, 0);	return;
		case 0x01:	dynax_blit_romregion_w(space, 0, 1);	return;
		case 0x80:	dynax_blit_romregion_w(space, 0, 2);	return;
		case 0x81:	dynax_blit_romregion_w(space, 0, 3);	return;
		case 0x82:	dynax_blit_romregion_w(space, 0, 4);	return;	// mjcomv1
	}
	logerror("%04x: unmapped romregion=%02X\n", cpu_get_pc(space->cpu), data);
}

static ADDRESS_MAP_START( yarunara_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x00, 0x01 ) AM_WRITE(yarunara_input_w)		// Controls
	AM_RANGE( 0x02, 0x03 ) AM_READ(yarunara_input_r)		//
	AM_RANGE( 0x11, 0x17 ) AM_WRITE(dynax_blitter_rev2_w)		// Blitter
	AM_RANGE( 0x20, 0x20 ) AM_DEVWRITE("msm", adpcm_reset_w)	// MSM5205 reset
	AM_RANGE( 0x22, 0x22 ) AM_WRITE(adpcm_data_w)			// MSM5205 data
	AM_RANGE( 0x24, 0x25 ) AM_DEVWRITE("ymsnd", ym2413_w)		//
	AM_RANGE( 0x28, 0x28 ) AM_DEVWRITE("aysnd", ay8910_data_w)	// AY8910
	AM_RANGE( 0x2a, 0x2a ) AM_DEVWRITE("aysnd", ay8910_address_w)	//
	AM_RANGE( 0x48, 0x48 ) AM_WRITE(dynax_extra_scrollx_w)	// screen scroll X
	AM_RANGE( 0x49, 0x49 ) AM_WRITE(dynax_extra_scrolly_w)	// screen scroll Y
	AM_RANGE( 0x4a, 0x4a ) AM_WRITE(yarunara_rombank_w)		// BANK ROM Select
	AM_RANGE( 0x4b, 0x4b ) AM_WRITE(dynax_vblank_ack_w)		// VBlank IRQ Ack
	AM_RANGE( 0x4c, 0x4c ) AM_READ_PORT("DSW0")			// DSW 1
	AM_RANGE( 0x4f, 0x4f ) AM_READ_PORT("DSW1")			// DSW 2
	AM_RANGE( 0x50, 0x50 ) AM_WRITE(yarunara_flipscreen_w)
	AM_RANGE( 0x51, 0x51 ) AM_WRITE(yarunara_layer_half_w)	// half of the interleaved layer to write to
	AM_RANGE( 0x52, 0x52 ) AM_WRITE(yarunara_layer_half2_w)	//
	// 53 ?
	// 54 ?
	AM_RANGE( 0x57, 0x57 ) AM_WRITE(dynax_blitter_ack_w)		// Blitter IRQ Ack
	AM_RANGE( 0x68, 0x68 ) AM_WRITE(dynax_blit_pen_w)		// Destination Pen
	AM_RANGE( 0x69, 0x69 ) AM_WRITE(dynax_blit_dest_w)		// Destination Layer
	AM_RANGE( 0x6a, 0x6a ) AM_WRITE(dynax_blit_palette01_w)	// Layers Palettes
	AM_RANGE( 0x6b, 0x6b ) AM_WRITE(dynax_blit_palette23_w)	//
	AM_RANGE( 0x6c, 0x6c ) AM_WRITE(hanamai_priority_w)		// layer priority and enable
	AM_RANGE( 0x6d, 0x6d ) AM_WRITE(dynax_blit_backpen_w)		// Background Color
	AM_RANGE( 0x6e, 0x6e ) AM_WRITE(yarunara_blit_romregion_w)	// Blitter ROM bank
ADDRESS_MAP_END


// Almost identical to hnoridur
static ADDRESS_MAP_START( mcnpshnt_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x01, 0x07 ) AM_WRITE(dynax_blitter_rev2_w)		// Blitter
//  AM_RANGE( 0x10, 0x10 ) AM_WRITENOP   // CRT Controller
//  AM_RANGE( 0x11, 0x11 ) AM_WRITENOP   // CRT Controller
	AM_RANGE( 0x20, 0x20 ) AM_WRITE(hanamai_keyboard_w)		// keyboard row select
	AM_RANGE( 0x21, 0x21 ) AM_READ_PORT("COINS")			// Coins
	AM_RANGE( 0x22, 0x22 ) AM_READ(hanamai_keyboard_1_r)		// P2
	AM_RANGE( 0x23, 0x23 ) AM_READ(hanamai_keyboard_0_r)		// P1
	AM_RANGE( 0x24, 0x24 ) AM_READ_PORT("DSW0")			// DSW2
	AM_RANGE( 0x26, 0x26 ) AM_READ_PORT("DSW1")			// DSW3
	AM_RANGE( 0x30, 0x30 ) AM_DEVWRITE("msm", adpcm_reset_w)	// MSM5205 reset
	AM_RANGE( 0x32, 0x32 ) AM_WRITE(adpcm_data_w)			// MSM5205 data
	AM_RANGE( 0x34, 0x35 ) AM_DEVWRITE("ymsnd", ym2413_w)		//
	AM_RANGE( 0x38, 0x38 ) AM_DEVWRITE("aysnd", ay8910_data_w)	// AY8910
	AM_RANGE( 0x3a, 0x3a ) AM_DEVWRITE("aysnd", ay8910_address_w)	//
	AM_RANGE( 0x40, 0x40 ) AM_WRITE(dynax_blit_pen_w)		// Destination Pen
	AM_RANGE( 0x41, 0x41 ) AM_WRITE(dynax_blit_dest_w)		// Destination Layer
	AM_RANGE( 0x42, 0x42 ) AM_WRITE(dynax_blit_palette01_w)	// Layers Palettes
	AM_RANGE( 0x43, 0x43 ) AM_WRITE(dynax_blit_palette23_w)	//
	AM_RANGE( 0x44, 0x44 ) AM_WRITE(hanamai_priority_w)		// layer priority and enable
	AM_RANGE( 0x45, 0x45 ) AM_WRITE(dynax_blit_backpen_w)		// Background Color
	AM_RANGE( 0x46, 0x46 ) AM_WRITE(yarunara_blit_romregion_w)	// Blitter ROM bank
	AM_RANGE( 0x47, 0x47 ) AM_WRITE(hnoridur_palbank_w)
	AM_RANGE( 0x50, 0x50 ) AM_WRITE(dynax_extra_scrollx_w)	// screen scroll X
	AM_RANGE( 0x51, 0x51 ) AM_WRITE(dynax_extra_scrolly_w)	// screen scroll Y
	AM_RANGE( 0x54, 0x54 ) AM_WRITE(hnoridur_rombank_w)		// BANK ROM Select
	AM_RANGE( 0x56, 0x56 ) AM_WRITE(dynax_vblank_ack_w)		// VBlank IRQ Ack
	AM_RANGE( 0x57, 0x57 ) AM_READ(ret_ff)				// ?
	AM_RANGE( 0x60, 0x60 ) AM_WRITE(dynax_flipscreen_w)		// Flip Screen
	AM_RANGE( 0x61, 0x61 ) AM_WRITE(hanamai_layer_half_w)		// half of the interleaved layer to write to
	AM_RANGE( 0x62, 0x62 ) AM_WRITE(hnoridur_layer_half2_w)	//
	AM_RANGE( 0x67, 0x67 ) AM_WRITE(dynax_blitter_ack_w)		// Blitter IRQ Ack
	AM_RANGE( 0x70, 0x70 ) AM_WRITE(dynax_coincounter_0_w)	// Coin Counters
	AM_RANGE( 0x71, 0x71 ) AM_WRITE(dynax_coincounter_1_w)	//
ADDRESS_MAP_END


static ADDRESS_MAP_START( sprtmtch_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x01, 0x07 ) AM_WRITE(dynax_blitter_rev2_w)		// Blitter
	AM_RANGE( 0x10, 0x11 ) AM_DEVREADWRITE("ymsnd", ym2203_r, ym2203_w)	// 2 x DSW
//  AM_RANGE( 0x12, 0x12 ) AM_WRITENOP   // CRT Controller
//  AM_RANGE( 0x13, 0x13 ) AM_WRITENOP   // CRT Controller
	AM_RANGE( 0x20, 0x20 ) AM_READ_PORT("P1")				// P1
	AM_RANGE( 0x21, 0x21 ) AM_READ_PORT("P2")				// P2
	AM_RANGE( 0x22, 0x22 ) AM_READ_PORT("COINS")			// Coins
	AM_RANGE( 0x23, 0x23 ) AM_READ(ret_ff)				// ?
	AM_RANGE( 0x30, 0x30 ) AM_WRITE(dynax_layer_enable_w)		// Layers Enable
	AM_RANGE( 0x31, 0x31 ) AM_WRITE(dynax_rombank_w)		// BANK ROM Select
	AM_RANGE( 0x32, 0x32 ) AM_WRITE(dynax_blit_dest_w)		// Destination Layer
	AM_RANGE( 0x33, 0x33 ) AM_WRITE(dynax_blit_pen_w)		// Destination Pen
	AM_RANGE( 0x34, 0x34 ) AM_WRITE(dynax_blit_palette01_w)	// Layers Palettes (Low Bits)
	AM_RANGE( 0x35, 0x35 ) AM_WRITE(dynax_blit_palette23_w)	//
	AM_RANGE( 0x36, 0x36 ) AM_WRITE(dynax_blit_backpen_w)		// Background Color
	AM_RANGE( 0x37, 0x37 ) AM_WRITE(dynax_vblank_ack_w)		// VBlank IRQ Ack
//  AM_RANGE( 0x40, 0x40 ) AM_DEVWRITE("msm", adpcm_reset_w)    // MSM5205 reset
	AM_RANGE( 0x41, 0x41 ) AM_WRITE(dynax_flipscreen_w)		// Flip Screen
	AM_RANGE( 0x42, 0x42 ) AM_WRITE(dynax_coincounter_0_w)	// Coin Counters
	AM_RANGE( 0x43, 0x43 ) AM_WRITE(dynax_coincounter_1_w)	//
	AM_RANGE( 0x44, 0x44 ) AM_WRITE(dynax_blitter_ack_w)		// Blitter IRQ Ack
	AM_RANGE( 0x45, 0x45 ) AM_WRITE(dynax_blit_palbank_w)		// Layers Palettes (High Bit)
ADDRESS_MAP_END



static ADDRESS_MAP_START( mjfriday_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x00, 0x00 ) AM_WRITE(dynax_blit_pen_w)		// Destination Pen
	AM_RANGE( 0x01, 0x01 ) AM_WRITE(dynax_blit_palette01_w)	// Layers Palettes (Low Bits)
	AM_RANGE( 0x02, 0x02 ) AM_WRITE(dynax_rombank_w)		// BANK ROM Select
	AM_RANGE( 0x03, 0x03 ) AM_WRITE(dynax_blit_backpen_w)		// Background Color
	AM_RANGE( 0x10, 0x11 ) AM_WRITE(mjdialq2_blit_dest_w)		// Destination Layer
	AM_RANGE( 0x12, 0x12 ) AM_WRITE(dynax_blit_palbank_w)		// Layers Palettes (High Bit)
	AM_RANGE( 0x13, 0x13 ) AM_WRITE(dynax_flipscreen_w)		// Flip Screen
	AM_RANGE( 0x14, 0x14 ) AM_WRITE(dynax_coincounter_0_w)	// Coin Counters
	AM_RANGE( 0x15, 0x15 ) AM_WRITE(dynax_coincounter_1_w)	//
	AM_RANGE( 0x16, 0x17 ) AM_WRITE(mjdialq2_layer_enable_w)	// Layers Enable
	AM_RANGE( 0x41, 0x47 ) AM_WRITE(dynax_blitter_rev2_w)		// Blitter
//  AM_RANGE( 0x50, 0x50 ) AM_WRITENOP   // CRT Controller
//  AM_RANGE( 0x51, 0x51 ) AM_WRITENOP   // CRT Controller
	AM_RANGE( 0x60, 0x60 ) AM_WRITE(hanamai_keyboard_w)		// keyboard row select
	AM_RANGE( 0x61, 0x61 ) AM_READ_PORT("COINS")			// Coins
	AM_RANGE( 0x62, 0x62 ) AM_READ(hanamai_keyboard_1_r)		// P2
	AM_RANGE( 0x63, 0x63 ) AM_READ(hanamai_keyboard_0_r)		// P1
	AM_RANGE( 0x64, 0x64 ) AM_READ_PORT("DSW0")			// DSW
	AM_RANGE( 0x67, 0x67 ) AM_READ_PORT("DSW1")			// DSW
	AM_RANGE( 0x70, 0x71 ) AM_DEVWRITE("ymsnd", ym2413_w)		//
//  AM_RANGE( 0x80, 0x80 ) AM_WRITENOP   // IRQ ack?
ADDRESS_MAP_END


static ADDRESS_MAP_START( nanajign_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x00, 0x00 ) AM_DEVWRITE("msm", adpcm_reset_w)	// MSM5205 reset
	AM_RANGE( 0x02, 0x02 ) AM_WRITE(adpcm_data_w)			// MSM5205 data
	AM_RANGE( 0x04, 0x05 ) AM_DEVWRITE("ymsnd", ym2413_w)		//
	AM_RANGE( 0x08, 0x08 ) AM_DEVWRITE("aysnd", ay8910_data_w)	// AY8910
	AM_RANGE( 0x0a, 0x0a ) AM_DEVWRITE("aysnd", ay8910_address_w)	//
	AM_RANGE( 0x10, 0x10 ) AM_WRITE(hanamai_keyboard_w)		// keyboard row select
	AM_RANGE( 0x11, 0x11 ) AM_READ_PORT("COINS")			// Coins
	AM_RANGE( 0x12, 0x12 ) AM_READ(hanamai_keyboard_1_r)		// P2
	AM_RANGE( 0x13, 0x13 ) AM_READ(hanamai_keyboard_0_r)		// P1
	AM_RANGE( 0x14, 0x14 ) AM_READ_PORT("DSW0")			// DSW1
	AM_RANGE( 0x15, 0x15 ) AM_READ_PORT("DSW1")			// DSW2
	AM_RANGE( 0x16, 0x16 ) AM_READ_PORT("DSW2")			// DSW3
//  AM_RANGE( 0x20, 0x21 ) AM_WRITENOP   // CRT Controller
	AM_RANGE( 0x31, 0x37 ) AM_WRITE(dynax_blitter_rev2_w)		// Blitter
	AM_RANGE( 0x40, 0x40 ) AM_WRITE(dynax_coincounter_0_w)	// Coin Counter
	AM_RANGE( 0x50, 0x50 ) AM_WRITE(dynax_flipscreen_w)		// Flip Screen
	AM_RANGE( 0x51, 0x51 ) AM_WRITE(hanamai_layer_half_w)		// half of the interleaved layer to write to
	AM_RANGE( 0x52, 0x52 ) AM_WRITE(hnoridur_layer_half2_w)	//
	AM_RANGE( 0x57, 0x57 ) AM_WRITE(dynax_blitter_ack_w)		// Blitter IRQ Ack
	AM_RANGE( 0x60, 0x60 ) AM_WRITE(dynax_extra_scrollx_w)	// screen scroll X
	AM_RANGE( 0x62, 0x62 ) AM_WRITE(dynax_extra_scrolly_w)	// screen scroll Y
	AM_RANGE( 0x6a, 0x6a ) AM_WRITE(hnoridur_rombank_w)		// BANK ROM Select
	AM_RANGE( 0x6c, 0x6c ) AM_WRITE(dynax_vblank_ack_w)		// VBlank IRQ Ack
	AM_RANGE( 0x70, 0x70 ) AM_WRITE(dynax_blit_pen_w)		// Destination Pen
	AM_RANGE( 0x71, 0x71 ) AM_WRITE(dynax_blit_dest_w)		// Destination Layer
	AM_RANGE( 0x72, 0x72 ) AM_WRITE(dynax_blit_palette01_w)	// Layers Palettes
	AM_RANGE( 0x73, 0x73 ) AM_WRITE(dynax_blit_palette23_w)	//
	AM_RANGE( 0x74, 0x74 ) AM_WRITE(hanamai_priority_w)		// layer priority and enable
	AM_RANGE( 0x75, 0x75 ) AM_WRITE(dynax_blit_backpen_w)		// Background Color
	AM_RANGE( 0x76, 0x76 ) AM_WRITE(yarunara_blit_romregion_w)	// Blitter ROM bank
	AM_RANGE( 0x77, 0x77 ) AM_WRITE(hnoridur_palbank_w)
ADDRESS_MAP_END


/***************************************************************************
                            Jantouki - Main CPU
***************************************************************************/

static READ8_HANDLER( jantouki_soundlatch_ack_r )
{
	dynax_state *state = (dynax_state *)space->machine->driver_data;
	return (state->soundlatch_ack) ? 0x80 : 0;
}

static WRITE8_HANDLER( jantouki_soundlatch_w )
{
	dynax_state *state = (dynax_state *)space->machine->driver_data;

	state->soundlatch_ack = 1;
	state->soundlatch_full = 1;
	state->soundlatch_irq = 1;
	state->latch = data;
	jantouki_sound_update_irq(space->machine);
}

static READ8_HANDLER( jantouki_blitter_busy_r )
{
	return 0;	// bit 0 & 1
}

static WRITE8_HANDLER( jantouki_rombank_w )
{
	memory_set_bank(space->machine, "bank1", data & 0x0f);
	set_led_status(space->machine, 0, data & 0x10);	// maybe
}

static ADDRESS_MAP_START( jantouki_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
//  AM_RANGE( 0x40, 0x41 ) AM_WRITENOP   // CRT Controller
	AM_RANGE( 0x48, 0x48 ) AM_WRITE(jantouki_rombank_w)		// BANK ROM Select
	AM_RANGE( 0x49, 0x49 ) AM_WRITE(jantouki_soundlatch_w)	// To Sound CPU
	AM_RANGE( 0x4a, 0x4a ) AM_READ(jantouki_soundlatch_ack_r)	// Soundlatch status
	AM_RANGE( 0x4b, 0x4b ) AM_WRITE(dynax_blit2_dest_w)		// Destination Layer 2
	AM_RANGE( 0x4d, 0x4d ) AM_WRITE(dynax_blit_dest_w)		// Destination Layer
	AM_RANGE( 0x4f, 0x4f ) AM_WRITE(dynax_blit2_romregion_w)	// Blitter 2 ROM bank
	AM_RANGE( 0x50, 0x50 ) AM_WRITE(jantouki_vblank_ack_w)	// VBlank IRQ Ack
	AM_RANGE( 0x51, 0x51 ) AM_WRITE(hanamai_keyboard_w)		// keyboard row select
	AM_RANGE( 0x52, 0x52 ) AM_READ(hanamai_keyboard_0_r)		// P1
	AM_RANGE( 0x54, 0x54 ) AM_READ_PORT("COINS")			// Coins
	AM_RANGE( 0x55, 0x55 ) AM_READ_PORT("DSW0")			// DSW1
	AM_RANGE( 0x56, 0x56 ) AM_READ_PORT("DSW1")			// DSW2
	AM_RANGE( 0x58, 0x58 ) AM_WRITE(dynax_coincounter_0_w)	// Coin Counter
	AM_RANGE( 0x5b, 0x5b ) AM_WRITE(dynax_blit2_palbank_w)	// Layers Palettes (High Bit)
	AM_RANGE( 0x5d, 0x5d ) AM_WRITE(dynax_blit_palbank_w)		//
	AM_RANGE( 0x5e, 0x5e ) AM_WRITE(jantouki_blitter_ack_w)	// Blitter IRQ Ack
	AM_RANGE( 0x5f, 0x5f ) AM_WRITE(jantouki_blitter2_ack_w)	// Blitter 2 IRQ Ack
	AM_RANGE( 0x60, 0x60 ) AM_WRITE(dynax_blit_palette67_w)	// Layers Palettes (Low Bits)
	AM_RANGE( 0x61, 0x61 ) AM_WRITE(dynax_blit_palette45_w)	//
	AM_RANGE( 0x62, 0x62 ) AM_WRITE(dynax_blit_palette23_w)	//
	AM_RANGE( 0x63, 0x63 ) AM_WRITE(dynax_blit_palette01_w)	//
	AM_RANGE( 0x64, 0x64 ) AM_WRITE(dynax_blit_backpen_w)		// Background Color
	AM_RANGE( 0x65, 0x65 ) AM_WRITE(dynax_blit2_pen_w)		// Destination Pen 2
	AM_RANGE( 0x66, 0x66 ) AM_WRITE(dynax_blit_pen_w)		// Destination Pen
	AM_RANGE( 0x67, 0x67 ) AM_READ(jantouki_blitter_busy_r)	//
	AM_RANGE( 0x69, 0x6f ) AM_WRITE(jantouki_blitter2_rev2_w)	// Blitter 2
	AM_RANGE( 0x71, 0x77 ) AM_WRITE(jantouki_blitter_rev2_w)	// Blitter
	AM_RANGE( 0x78, 0x7e ) AM_WRITE(jantouki_layer_enable_w)	// Layers Enable
ADDRESS_MAP_END

/***************************************************************************
                            Jantouki - Sound CPU
***************************************************************************/

static WRITE8_HANDLER( jantouki_soundlatch_ack_w )
{
	dynax_state *state = (dynax_state *)space->machine->driver_data;
	state->soundlatch_ack = data;
	state->soundlatch_irq = 0;
	jantouki_sound_update_irq(space->machine);
}

static READ8_HANDLER( jantouki_soundlatch_r )
{
	dynax_state *state = (dynax_state *)space->machine->driver_data;
	state->soundlatch_full = 0;
	return state->latch;
}

static READ8_HANDLER( jantouki_soundlatch_status_r )
{
	dynax_state *state = (dynax_state *)space->machine->driver_data;
	return (state->soundlatch_full) ? 0 : 0x80;
}

static ADDRESS_MAP_START( jantouki_sound_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x00, 0x00 ) AM_WRITE(jantouki_sound_rombank_w)		// BANK ROM Select
	AM_RANGE( 0x10, 0x10 ) AM_WRITE(jantouki_sound_vblank_ack_w)	// VBlank IRQ Ack
	AM_RANGE( 0x21, 0x21 ) AM_DEVREAD("aysnd", ay8910_r)			// AY8910
	AM_RANGE( 0x22, 0x23 ) AM_DEVWRITE("aysnd", ay8910_data_address_w)	//
	AM_RANGE( 0x28, 0x29 ) AM_DEVREADWRITE("ymsnd", ym2203_r, ym2203_w)	//
	AM_RANGE( 0x30, 0x30 ) AM_DEVWRITE("msm", adpcm_reset_w)		// MSM5205 reset
	AM_RANGE( 0x40, 0x40 ) AM_WRITE(adpcm_data_w)				// MSM5205 data
	AM_RANGE( 0x50, 0x50 ) AM_READ(jantouki_soundlatch_status_r)	// Soundlatch status
	AM_RANGE( 0x60, 0x60 ) AM_WRITE(jantouki_soundlatch_ack_w)		// Soundlatch status
	AM_RANGE( 0x70, 0x70 ) AM_READ(jantouki_soundlatch_r)			// From Main CPU
ADDRESS_MAP_END



/***************************************************************************
                            Mahjong Electron Base
***************************************************************************/

static READ8_HANDLER( mjelctrn_keyboard_1_r )
{
	return (hanamai_keyboard_1_r(space, 0) & 0x3f) | (input_port_read(space->machine, "FAKE") ? 0x40 : 0);
}

static READ8_HANDLER( mjelctrn_dsw_r )
{
	dynax_state *state = (dynax_state *)space->machine->driver_data;
	int dsw = (state->keyb & 0xc0) >> 6;
	static const char *const dswnames[] = { "DSW0", "DSW1", "DSW3", "DSW4" };

	return input_port_read(space->machine, dswnames[dsw]);
}

static WRITE8_HANDLER( mjelctrn_blitter_ack_w )
{
	dynax_state *state = (dynax_state *)space->machine->driver_data;
	state->blitter_irq = 0;
}

static ADDRESS_MAP_START( mjelctrn_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x00, 0x00 ) AM_DEVWRITE("msm", adpcm_reset_w)	// MSM5205 reset
	AM_RANGE( 0x02, 0x02 ) AM_WRITE(adpcm_data_w)			// MSM5205 data
	AM_RANGE( 0x04, 0x05 ) AM_DEVWRITE("ymsnd", ym2413_w)		//
	AM_RANGE( 0x08, 0x08 ) AM_DEVWRITE("aysnd", ay8910_data_w)	// AY8910
	AM_RANGE( 0x0a, 0x0a ) AM_DEVWRITE("aysnd", ay8910_address_w)	//
	AM_RANGE( 0x11, 0x12 ) AM_WRITE(mjelctrn_blitter_ack_w)	//?
//  AM_RANGE( 0x20, 0x20 ) AM_WRITENOP   // CRT Controller
//  AM_RANGE( 0x21, 0x21 ) AM_WRITENOP   // CRT Controller
	AM_RANGE( 0x40, 0x40 ) AM_WRITE(dynax_coincounter_0_w)	// Coin Counters
	AM_RANGE( 0x41, 0x41 ) AM_WRITE(dynax_coincounter_1_w)	//
	AM_RANGE( 0x60, 0x60 ) AM_WRITE(dynax_extra_scrollx_w)	// screen scroll X
	AM_RANGE( 0x62, 0x62 ) AM_WRITE(dynax_extra_scrolly_w)	// screen scroll Y
//  AM_RANGE( 0x64, 0x64 ) AM_WRITE(dynax_extra_scrollx_w)      // screen scroll X
//  AM_RANGE( 0x66, 0x66 ) AM_WRITE(dynax_extra_scrolly_w)      // screen scroll Y
	AM_RANGE( 0x6a, 0x6a ) AM_WRITE(hnoridur_rombank_w)		// BANK ROM Select
	AM_RANGE( 0x80, 0x80 ) AM_WRITE(hanamai_keyboard_w)		// keyboard row select
	AM_RANGE( 0x81, 0x81 ) AM_READ_PORT("COINS")			// Coins
	AM_RANGE( 0x82, 0x82 ) AM_READ(mjelctrn_keyboard_1_r)		// P2
	AM_RANGE( 0x83, 0x83 ) AM_READ(hanamai_keyboard_0_r)		// P1
	AM_RANGE( 0x84, 0x84 ) AM_READ(mjelctrn_dsw_r)			// DSW x 4
	AM_RANGE( 0x85, 0x85 ) AM_READ(ret_ff)				// ?
	AM_RANGE( 0xa1, 0xa7 ) AM_WRITE(dynax_blitter_rev2_w)		// Blitter
	AM_RANGE( 0xc0, 0xc0 ) AM_WRITE(dynax_flipscreen_w)		// Flip Screen
	AM_RANGE( 0xc1, 0xc1 ) AM_WRITE(hanamai_layer_half_w)		// half of the interleaved layer to write to
	AM_RANGE( 0xc2, 0xc2 ) AM_WRITE(hnoridur_layer_half2_w)	//
//  c3,c4   seem to be related to wrap around enable
	AM_RANGE( 0xe0, 0xe0 ) AM_WRITE(dynax_blit_pen_w)		// Destination Pen
	AM_RANGE( 0xe1, 0xe1 ) AM_WRITE(dynax_blit_dest_w)		// Destination Layer
	AM_RANGE( 0xe2, 0xe2 ) AM_WRITE(dynax_blit_palette01_w)	// Layers Palettes
	AM_RANGE( 0xe3, 0xe3 ) AM_WRITE(dynax_blit_palette23_w)	//
	AM_RANGE( 0xe4, 0xe4 ) AM_WRITE(hanamai_priority_w)		// layer priority and enable
	AM_RANGE( 0xe5, 0xe5 ) AM_WRITE(dynax_blit_backpen_w)		// Background Color
	AM_RANGE( 0xe6, 0xe6 ) AM_WRITE(yarunara_blit_romregion_w)	// Blitter ROM bank
	AM_RANGE( 0xe7, 0xe7 ) AM_WRITE(hnoridur_palbank_w)
ADDRESS_MAP_END


/***************************************************************************
                            Hanafuda Hana Tengoku
***************************************************************************/

static WRITE8_HANDLER( htengoku_select_w )
{
	dynax_state *state = (dynax_state *)space->machine->driver_data;
	state->input_sel = data;
	state->keyb = 0;
}

static WRITE8_DEVICE_HANDLER( htengoku_dsw_w )
{
	dynax_state *state = (dynax_state *)device->machine->driver_data;
	state->dsw_sel = data;
}

static READ8_DEVICE_HANDLER( htengoku_dsw_r )
{
	dynax_state *state = (dynax_state *)device->machine->driver_data;
	if (!BIT(state->dsw_sel, 0))	return input_port_read(device->machine, "DSW0");
	if (!BIT(state->dsw_sel, 1))	return input_port_read(device->machine, "DSW1");
	if (!BIT(state->dsw_sel, 2))	return input_port_read(device->machine, "DSW2");
	if (!BIT(state->dsw_sel, 3))	return input_port_read(device->machine, "DSW3");
	if (!BIT(state->dsw_sel, 4))	return input_port_read(device->machine, "DSW4");
	logerror("%s: warning, unknown bits read, dsw_sel = %02x\n", cpuexec_describe_context(device->machine), state->dsw_sel);

	return 0xff;
}

static WRITE8_HANDLER( htengoku_coin_w )
{
	dynax_state *state = (dynax_state *)space->machine->driver_data;

	switch (state->input_sel)
	{
		case 0x0c:
			// bit 0 = coin counter
			// bit 1 = out counter
			// bit 2 = hopper
			coin_counter_w(space->machine, 0, data & 1);
			state->hopper = data & 0x04;
#ifdef MAME_DEBUG
//          popmessage("COINS %02x",data);
#endif
			state->coins = data;

		case 0x0d:	break;	// ff resets input port sequence?

		case 0xff:	break;	// CRT controller?
		default:
			logerror("%04x: coins_w with select = %02x, data = %02x\n", cpu_get_pc(space->cpu), state->input_sel, data);
	}
}

static READ8_HANDLER( htengoku_input_r )
{
	dynax_state *state = (dynax_state *)space->machine->driver_data;
	static const char *const keynames0[] = { "KEY0", "KEY1", "KEY2", "KEY3", "KEY4" };
	static const char *const keynames1[] = { "KEY5", "KEY6", "KEY7", "KEY8", "KEY9" };

	switch (state->input_sel)
	{
		case 0x81:	return input_port_read(space->machine, keynames1[state->keyb++]);
		case 0x82:	return input_port_read(space->machine, keynames0[state->keyb++]);
		case 0x0d:	return 0xff;	// unused
	}
	logerror("%04x: input_r with select = %02x\n", cpu_get_pc(space->cpu), state->input_sel);
	return 0xff;
}

static READ8_HANDLER( htengoku_coin_r )
{
	dynax_state *state = (dynax_state *)space->machine->driver_data;

	switch (state->input_sel)
	{
		case 0x00:	return input_port_read(space->machine, "COINS");
		case 0x01:	return 0xff;	//?
		case 0x02:	return 0xbf | ((state->hopper && !(space->machine->primary_screen->frame_number() % 10)) ? 0 : (1 << 6));	// bit 7 = blitter busy, bit 6 = hopper
		case 0x03:	return state->coins;
	}
	logerror("%04x: coin_r with select = %02x\n", cpu_get_pc(space->cpu), state->input_sel);
	return 0xff;
}

static WRITE8_HANDLER( htengoku_rombank_w )
{
	dynax_state *state = (dynax_state *)space->machine->driver_data;

	memory_set_bank(space->machine, "bank1", data & 0x07);
	state->hnoridur_bank = data;
}

static WRITE8_HANDLER( htengoku_blit_romregion_w )
{
	switch (data)
	{
		case 0x80:	dynax_blit_romregion_w(space, 0, 0);	return;
		case 0x81:	dynax_blit_romregion_w(space, 0, 1);	return;
		case 0x00:	dynax_blit_romregion_w(space, 0, 2);	return;
	}
	logerror("%04x: unmapped romregion=%02X\n", cpu_get_pc(space->cpu), data);
}

static READ8_HANDLER( unk_r )
{
	return 0x78;
}

static ADDRESS_MAP_START( htengoku_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x01, 0x07 ) AM_WRITE(dynax_blitter_rev2_w)		// Blitter
	AM_RANGE( 0x20, 0x20 ) AM_WRITE(htengoku_select_w)		// Controls
	AM_RANGE( 0x21, 0x21 ) AM_WRITE(htengoku_coin_w)		//
	AM_RANGE( 0x22, 0x22 ) AM_READ(htengoku_coin_r)			//
	AM_RANGE( 0x23, 0x23 ) AM_READ(htengoku_input_r)		//
	AM_RANGE( 0x40, 0x40 ) AM_DEVWRITE("aysnd", ay8910_address_w)	// AY8910
	AM_RANGE( 0x42, 0x42 ) AM_DEVREAD("aysnd", ay8910_r)		//
	AM_RANGE( 0x44, 0x44 ) AM_DEVWRITE("aysnd", ay8910_data_w)	//
	AM_RANGE( 0x46, 0x47 ) AM_DEVWRITE("ymsnd", ym2413_w)		//
	AM_RANGE( 0x80, 0x8f ) AM_DEVREADWRITE("rtc", msm6242_r, msm6242_w)	// 6242RTC
	AM_RANGE( 0xa0, 0xa3 ) AM_WRITE(ddenlovr_palette_base_w)	// ddenlovr mixer chip
	AM_RANGE( 0xa4, 0xa7 ) AM_WRITE(ddenlovr_palette_mask_w)
	AM_RANGE( 0xa8, 0xab ) AM_WRITE(ddenlovr_transparency_pen_w)
	AM_RANGE( 0xac, 0xaf ) AM_WRITE(ddenlovr_transparency_mask_w)
	// b0-b3 ?
	AM_RANGE( 0xb4, 0xb4 ) AM_WRITE(ddenlovr_bgcolor_w)
	AM_RANGE( 0xb5, 0xb5 ) AM_WRITE(ddenlovr_priority_w)
	AM_RANGE( 0xb6, 0xb6 ) AM_WRITE(ddenlovr_layer_enable_w)
	AM_RANGE( 0xb8, 0xb8 ) AM_READ(unk_r)				// ? must be 78 on startup
	AM_RANGE( 0xc2, 0xc2 ) AM_WRITE(htengoku_rombank_w)		// BANK ROM Select
	AM_RANGE( 0xc0, 0xc0 ) AM_WRITE(dynax_extra_scrollx_w)	// screen scroll X
	AM_RANGE( 0xc1, 0xc1 ) AM_WRITE(dynax_extra_scrolly_w)	// screen scroll Y
	AM_RANGE( 0xc3, 0xc3 ) AM_WRITE(dynax_vblank_ack_w)		// VBlank IRQ Ack
	AM_RANGE( 0xc4, 0xc4 ) AM_WRITE(dynax_blit_pen_w)		// Destination Pen
	AM_RANGE( 0xc5, 0xc5 ) AM_WRITE(dynax_blit_dest_w)		// Destination Layer
	AM_RANGE( 0xc6, 0xc6 ) AM_WRITE(htengoku_blit_romregion_w)	// Blitter ROM bank
	AM_RANGE( 0xe0, 0xe0 ) AM_WRITE(yarunara_flipscreen_w)
	AM_RANGE( 0xe1, 0xe1 ) AM_WRITE(yarunara_layer_half_w)	// half of the interleaved layer to write to
	AM_RANGE( 0xe2, 0xe2 ) AM_WRITE(yarunara_layer_half2_w)	//
	AM_RANGE( 0xe5, 0xe5 ) AM_WRITE(dynax_blitter_ack_w)		// Blitter IRQ Ack
ADDRESS_MAP_END


/***************************************************************************
                               Mahjong Tenkaigen
***************************************************************************/

static WRITE8_HANDLER( tenkai_ipsel_w )
{
	dynax_state *state = (dynax_state *)space->machine->driver_data;
	switch (offset)
	{
	case 0:	state->input_sel = data;
			state->keyb = 0;
			break;

	case 1:	break;
	}

}

static WRITE8_HANDLER( tenkai_ip_w )
{
	dynax_state *state = (dynax_state *)space->machine->driver_data;

	switch (state->input_sel)
	{
	case 0x0c:
		coin_counter_w(space->machine, 0, data & 0x01);
		coin_counter_w(space->machine, 1, data & 0x02);
		// bit 2?
		// bit 3?
//          popmessage("%02x", data);
		return;

	case 0x0d:
		if (data != 0xff)
			break;
		return;
	}
	logerror("%04x: unmapped ip_sel=%02x written with %02x\n", cpu_get_pc(space->cpu), state->input_sel, data);
}

static READ8_HANDLER( tenkai_ip_r )
{
	dynax_state *state = (dynax_state *)space->machine->driver_data;
	static const char *const keynames0[] = { "KEY0", "KEY1", "KEY2", "KEY3", "KEY4" };
	//static const char *const keynames1[] = { "KEY5", "KEY6", "KEY7", "KEY8", "KEY9" };

	switch (offset)
	{
		case 0:
		{
			switch (state->input_sel)
			{
				case 0x00:
					return input_port_read(space->machine, "COINS");	// coins

				default:
					logerror("%04x: unmapped ip_sel=%02x read from offs %x\n", cpu_get_pc(space->cpu), state->input_sel, offset);
					return 0xff;
			}
		}

		case 1:
		{
			switch (state->input_sel)
			{
				case 0x0d:
					return 0xff;

				// player 2
				case 0x81:
					if (state->keyb >= 5)
						logerror("%04x: unmapped keyb=%02x read\n", cpu_get_pc(space->cpu), state->keyb);
					return 0xff;//input_port_read(space->machine, keynames1[state->keyb++]);

				// player 1
				case 0x82:
					if (state->keyb >= 5)
						logerror("%04x: unmapped keyb=%02x read\n", cpu_get_pc(space->cpu), state->keyb);
					return input_port_read(space->machine, keynames0[state->keyb++]);

				default:
					logerror("%04x: unmapped ip_sel=%02x read from offs %x\n", cpu_get_pc(space->cpu), state->input_sel, offset);
					return 0xff;
			}
		}
	}
	return 0xff;
}


static WRITE8_DEVICE_HANDLER( tenkai_dswsel_w )
{
	dynax_state *state = (dynax_state *)device->machine->driver_data;
	state->dsw_sel = data;
}

static READ8_DEVICE_HANDLER( tenkai_dsw_r )
{
	dynax_state *state = (dynax_state *)device->machine->driver_data;

	if (!BIT(state->dsw_sel, 0)) return input_port_read(device->machine, "DSW0");
	if (!BIT(state->dsw_sel, 1)) return input_port_read(device->machine, "DSW1");
	if (!BIT(state->dsw_sel, 2)) return input_port_read(device->machine, "DSW2");
	if (!BIT(state->dsw_sel, 3)) return input_port_read(device->machine, "DSW3");
	if (!BIT(state->dsw_sel, 4)) return input_port_read(device->machine, "DSW4");
	logerror("%s: unmapped dsw %02x read\n", cpuexec_describe_context(device->machine), state->dsw_sel);

	return 0xff;
}

static READ8_HANDLER( tenkai_palette_r )
{
	dynax_state *state = (dynax_state *)space->machine->driver_data;
	return state->palette_ram[512 * state->palbank + offset];
}

static WRITE8_HANDLER( tenkai_palette_w )
{
	dynax_state *state = (dynax_state *)space->machine->driver_data;
	int addr = 512 * state->palbank + offset;
	state->palette_ram[addr] = data;

	{
		int br = state->palette_ram[addr & ~0x10];		// bbbrrrrr
		int bg = state->palette_ram[addr | 0x10];		// bb0ggggg
		int r = br & 0x1f;
		int g = bg & 0x1f;
		int b = ((bg & 0xc0) >> 3) | ((br & 0xe0) >> 5);
		palette_set_color_rgb(space->machine, 256 * state->palbank + ((offset & 0xf) | ((offset & 0x1e0) >> 1)), pal5bit(r), pal5bit(g), pal5bit(b));
	}
}

static void tenkai_update_rombank( running_machine *machine )
{
	dynax_state *state = (dynax_state *)machine->driver_data;
	state->romptr = memory_region(machine, "maincpu") + 0x10000 + 0x8000 * state->rombank;
//  logerror("rombank = %02x\n", state->rombank);
}

static READ8_HANDLER( tenkai_p3_r )
{
	return 0x00;
}

static WRITE8_HANDLER( tenkai_p3_w )
{
	dynax_state *state = (dynax_state *)space->machine->driver_data;
	state->rombank = ((data & 0x04) << 1) | (state->rombank & 0x07);
	tenkai_update_rombank(space->machine);
}
static WRITE8_HANDLER( tenkai_p4_w )
{
	dynax_state *state = (dynax_state *)space->machine->driver_data;
	state->rombank = (state->rombank & 0x08) | ((data & 0x0e) >> 1);
	tenkai_update_rombank(space->machine);
}

static READ8_HANDLER( tenkai_p5_r )
{
	dynax_state *state = (dynax_state *)space->machine->driver_data;
	return state->tenkai_p5_val;
}

static WRITE8_HANDLER( tenkai_p6_w )
{
	dynax_state *state = (dynax_state *)space->machine->driver_data;
	state->tenkai_p5_val &= 0x0f;

	if (data & 0x0f)
		state->tenkai_p5_val |= (1 << 4);
}

static WRITE8_HANDLER( tenkai_p7_w )
{
	dynax_state *state = (dynax_state *)space->machine->driver_data;
	state->tenkai_p5_val &= 0xf0;

	if (data & 0x03)
		state->tenkai_p5_val |= (1 << 3);
}

static WRITE8_HANDLER( tenkai_p8_w )
{
	dynax_state *state = (dynax_state *)space->machine->driver_data;
	state->rombank = ((data & 0x08) << 1) | (state->rombank & 0x0f);
	tenkai_update_rombank(space->machine);
}

static READ8_HANDLER( tenkai_p8_r )
{
	return 0x00;
}

static READ8_HANDLER( tenkai_8000_r )
{
	dynax_state *state = (dynax_state *)space->machine->driver_data;

	if (state->rombank < 0x10)
		return state->romptr[offset];
	else if ((state->rombank == 0x10) && (offset < 0x10))
		return msm6242_r(state->rtc, offset);
	else if (state->rombank == 0x12)
		return tenkai_palette_r(space, offset);

	logerror("%04x: unmapped offset %04X read with rombank=%02X\n", cpu_get_pc(space->cpu), offset, state->rombank);
	return 0x00;
}

static WRITE8_HANDLER( tenkai_8000_w )
{
	dynax_state *state = (dynax_state *)space->machine->driver_data;

	if ((state->rombank == 0x10) && (offset < 0x10))
	{
		msm6242_w(state->rtc, offset, data);
		return;
	}
	else if (state->rombank == 0x12)
	{
		tenkai_palette_w(space, offset, data);
		return;
	}

	logerror("%04x: unmapped offset %04X=%02X written with rombank=%02X\n", cpu_get_pc(space->cpu), offset, data, state->rombank);
}

static void tenkai_show_6c( running_machine *machine )
{
//    dynax_state *state = (dynax_state *)machine->driver_data;
//    popmessage("%02x %02x", state->tenkai_6c, state->tenkai_70);
}

static WRITE8_HANDLER( tenkai_6c_w )
{
	dynax_state *state = (dynax_state *)space->machine->driver_data;
	state->tenkai_6c = data;
	tenkai_show_6c(space->machine);
}

static WRITE8_HANDLER( tenkai_70_w )
{
	dynax_state *state = (dynax_state *)space->machine->driver_data;
	state->tenkai_70 = data;
	tenkai_show_6c(space->machine);
}

static WRITE8_HANDLER( tenkai_blit_romregion_w )
{
	switch (data)
	{
		case 0x00:	dynax_blit_romregion_w(space, 0, 0);	return;
		case 0x83:	dynax_blit_romregion_w(space, 0, 1);	return;
		case 0x80:	dynax_blit_romregion_w(space, 0, 2);	return;
	}
	logerror("%04x: unmapped romregion=%02X\n", cpu_get_pc(space->cpu), data);
}

static ADDRESS_MAP_START( tenkai_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(  0x0000,  0x5fff ) AM_ROM
	AM_RANGE(  0x6000,  0x6fff ) AM_RAM
	AM_RANGE(  0x7000,  0x7fff ) AM_RAM AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE(  0x8000,  0xffff ) AM_READWRITE(tenkai_8000_r, tenkai_8000_w)
	AM_RANGE( 0x10000, 0x10000 ) AM_DEVREAD("aysnd", ay8910_r)		// AY8910
	AM_RANGE( 0x10008, 0x10008 ) AM_DEVWRITE("aysnd", ay8910_data_w)	//
	AM_RANGE( 0x10010, 0x10010 ) AM_DEVWRITE("aysnd", ay8910_address_w)	//
	AM_RANGE( 0x10020, 0x10021 ) AM_DEVWRITE("ymsnd", ym2413_w)		//
	AM_RANGE( 0x10040, 0x10040 ) AM_WRITE(dynax_blit_pen_w)		// Destination Pen
	AM_RANGE( 0x10044, 0x10044 ) AM_WRITE(tenkai_blit_dest_w)		// Destination Layer
	AM_RANGE( 0x10048, 0x10048 ) AM_WRITE(tenkai_blit_palette23_w)	// Layers Palettes
	AM_RANGE( 0x1004c, 0x1004c ) AM_WRITE(tenkai_blit_palette01_w)	//
	AM_RANGE( 0x10050, 0x10050 ) AM_WRITE(tenkai_priority_w)		// layer priority and enable
	AM_RANGE( 0x10054, 0x10054 ) AM_WRITE(dynax_blit_backpen_w)		// Background Color
	AM_RANGE( 0x10058, 0x10058 ) AM_WRITE(tenkai_blit_romregion_w)	// Blitter ROM bank
	AM_RANGE( 0x10060, 0x10060 ) AM_WRITE(yarunara_flipscreen_inv_w)	// Flip Screen
	AM_RANGE( 0x10064, 0x10064 ) AM_WRITE(yarunara_layer_half_w)	// half of the interleaved layer to write to
	AM_RANGE( 0x10068, 0x10068 ) AM_WRITE(yarunara_layer_half2_w)	//
	AM_RANGE( 0x1006c, 0x1006c ) AM_WRITE(tenkai_6c_w)			// ?
	AM_RANGE( 0x10070, 0x10070 ) AM_WRITE(tenkai_70_w)			// ?
	AM_RANGE( 0x1007c, 0x1007c ) AM_WRITENOP	// IRQ Ack? (0,2)
	AM_RANGE( 0x100c0, 0x100c0 ) AM_WRITE(tenkai_ipsel_w)
	AM_RANGE( 0x100c1, 0x100c1 ) AM_WRITE(tenkai_ip_w)
	AM_RANGE( 0x100c2, 0x100c3 ) AM_READ(tenkai_ip_r)
	AM_RANGE( 0x100e1, 0x100e7 ) AM_WRITE(tenkai_blitter_rev2_w)	// Blitter (inverted scroll values)
ADDRESS_MAP_END

static ADDRESS_MAP_START( tenkai_io_map, ADDRESS_SPACE_IO, 8 )
	AM_RANGE( T90_P3, T90_P3 ) AM_READWRITE(tenkai_p3_r, tenkai_p3_w)
	AM_RANGE( T90_P4, T90_P4 ) AM_WRITE(tenkai_p4_w)
	AM_RANGE( T90_P5, T90_P5 ) AM_READ(tenkai_p5_r)
	AM_RANGE( T90_P6, T90_P6 ) AM_WRITE(tenkai_p6_w)
	AM_RANGE( T90_P7, T90_P7 ) AM_WRITE(tenkai_p7_w)
	AM_RANGE( T90_P8, T90_P8 ) AM_READWRITE(tenkai_p8_r, tenkai_p8_w)
ADDRESS_MAP_END

/***************************************************************************
                                Mahjong Gekisha
***************************************************************************/

static READ8_HANDLER( gekisha_keyboard_0_r )
{
	dynax_state *state = (dynax_state *)space->machine->driver_data;
	int res = 0x3f;

	if (!BIT(state->keyb, 0)) res &= input_port_read(space->machine, "KEY0");
	if (!BIT(state->keyb, 1)) res &= input_port_read(space->machine, "KEY1");
	if (!BIT(state->keyb, 2)) res &= input_port_read(space->machine, "KEY2");
	if (!BIT(state->keyb, 3)) res &= input_port_read(space->machine, "KEY3");
	if (!BIT(state->keyb, 4)) res &= input_port_read(space->machine, "KEY4");

	return res;
}
static READ8_HANDLER( gekisha_keyboard_1_r )
{
	dynax_state *state = (dynax_state *)space->machine->driver_data;
	int res = 0x3f;

	if (!BIT(state->keyb, 0)) res &= input_port_read(space->machine, "KEY5");
	if (!BIT(state->keyb, 1)) res &= input_port_read(space->machine, "KEY6");
	if (!BIT(state->keyb, 2)) res &= input_port_read(space->machine, "KEY7");
	if (!BIT(state->keyb, 3)) res &= input_port_read(space->machine, "KEY8");
	if (!BIT(state->keyb, 4)) res &= input_port_read(space->machine, "KEY9");

	// bit 6
	res |= input_port_read(space->machine, "BET");

	// bit 7 = blitter busy

	return res;
}

static WRITE8_HANDLER( gekisha_hopper_w )
{
	dynax_state *state = (dynax_state *)space->machine->driver_data;
	state->gekisha_val[offset] = data;
//  popmessage("%02x %02x", gekisha_val[0], gekisha_val[1]);
}

static void gekisha_set_rombank( running_machine *machine, UINT8 data )
{
	dynax_state *state = (dynax_state *)machine->driver_data;
	state->rombank = data;
	state->romptr = memory_region(machine, "maincpu") + 0x8000 + state->rombank * 0x8000;
}

static WRITE8_HANDLER( gekisha_p4_w )
{
	dynax_state *state = (dynax_state *)space->machine->driver_data;
	state->gekisha_rom_enable = !BIT(data, 3);
	gekisha_set_rombank(space->machine, BIT(data, 2));
}

static READ8_HANDLER( gekisha_8000_r )
{
	dynax_state *state = (dynax_state *)space->machine->driver_data;

	if (state->gekisha_rom_enable)
		return state->romptr[offset];

	switch (offset + 0x8000)
	{
		case 0x8061:	return input_port_read(space->machine, "COINS");
		case 0x8062:	return gekisha_keyboard_1_r(space, 0);
		case 0x8063:	return gekisha_keyboard_0_r(space, 0);
		case 0x8064:	return input_port_read(space->machine, "DSW1");
		case 0x8065:	return input_port_read(space->machine, "DSW3");
		case 0x8066:	return input_port_read(space->machine, "DSW4");
		case 0x8067:	return input_port_read(space->machine, "DSW2");
	}

	logerror("%04x: unmapped offset %04X read with rombank=%02X\n",cpu_get_pc(space->cpu), offset, state->rombank);
	return 0x00;
}

static WRITE8_HANDLER( gekisha_8000_w )
{
	dynax_state *state = (dynax_state *)space->machine->driver_data;

	if (!state->gekisha_rom_enable)
	{
		switch (offset + 0x8000)
		{
			// same offsets as mjfriday

			case 0x8001:	dynax_blit_palette01_w(space, offset - 0x01, data);		return;

//          case 0x8002:    // ? 1

			case 0x8003:	dynax_blit_backpen_w(space, offset - 0x03, data);		return;

			case 0x8010:
			case 0x8011:	mjdialq2_blit_dest_w(space, offset - 0x10, data);		return;

			case 0x8012:	dynax_blit_palbank_w(space, offset - 0x12, data);		return;

			case 0x8013:	dynax_flipscreen_w(space, offset - 0x13, data);			return;

			case 0x8014:	dynax_coincounter_0_w(space, offset - 0x14, data);		return;
			case 0x8015:	dynax_coincounter_1_w(space, offset - 0x15, data);		return;

			case 0x8016:
			case 0x8017:	mjdialq2_layer_enable_w(space, offset - 0x16, data);	return;

			case 0x8020:
			case 0x8021:	gekisha_hopper_w(space, offset - 0x20, data);	return;

			case 0x8041:
			case 0x8042:
			case 0x8043:
			case 0x8044:
			case 0x8045:
			case 0x8046:
			case 0x8047:	dynax_blitter_rev2_w(space, offset - 0x41, data);	return;

			case 0x8050:	// CRT controller
			case 0x8051:	return;

			case 0x8070:	ym2413_register_port_w(state->ymsnd, 0, data);	return;
			case 0x8071:	ym2413_data_port_w(state->ymsnd, 0, data);	return;

			case 0x8060:	state->keyb = data;	return;

//          case 0x8080:    // ? 0,1,6 (bit 0 = screen disable?)
//              popmessage("80 = %02x", data);
//              break;
		}
	}
	logerror("%04x: unmapped offset %04X=%02X written with rombank=%02X\n", cpu_get_pc(space->cpu), offset, data, state->rombank);
}


static ADDRESS_MAP_START( gekisha_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(  0x0000,  0x6fff ) AM_ROM
	AM_RANGE(  0x7000,  0x7fff ) AM_RAM AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE(  0x8000,  0xffff ) AM_READWRITE(gekisha_8000_r, gekisha_8000_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( gekisha_io_map, ADDRESS_SPACE_IO, 8 )
	AM_RANGE( T90_P4, T90_P4 ) AM_WRITE(gekisha_p4_w)
ADDRESS_MAP_END


/***************************************************************************


                                Input Ports


***************************************************************************/

static INPUT_PORTS_START( MAHJONG_KEYS )
	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE ) PORT_PLAYER(1)	// "l"
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(1)	// "f"
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("KEY6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE ) PORT_PLAYER(2)	// "l"
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2)	// "f"
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( MAHJONG_KEYS_BET )
	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET ) PORT_PLAYER(1)

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE ) PORT_PLAYER(1)	// "l"
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE ) PORT_PLAYER(1)	// "t"
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP ) PORT_PLAYER(1)	// "w"
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(1)	// "f"
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG ) PORT_PLAYER(1)	// "b"
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL ) PORT_PLAYER(1)	// "s"

	PORT_START("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("KEY6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET ) PORT_PLAYER(2)

	PORT_START("KEY7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE ) PORT_PLAYER(2)	// "l"
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE ) PORT_PLAYER(2)	// "t"
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP ) PORT_PLAYER(2)	// "w"
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2)	// "f"
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG ) PORT_PLAYER(2)	// "b"
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL ) PORT_PLAYER(2)	// "s"
INPUT_PORTS_END

static INPUT_PORTS_START( HANAFUDA_KEYS )
	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_A ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_E ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_HANAFUDA_YES ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_B ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_F ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_HANAFUDA_NO ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_C ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_G ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_D ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_H ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE ) PORT_PLAYER(1)	// "l"
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(1)	// "f"
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_A ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_E ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_HANAFUDA_YES ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("KEY6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_B ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_F ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_HANAFUDA_NO ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_C ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_G ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_D ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_H ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE ) PORT_PLAYER(2)	// "l"
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2)	// "f"
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( HANAFUDA_KEYS_BET )
	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_A ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_E ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_HANAFUDA_YES ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_B ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_F ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_HANAFUDA_NO ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET ) PORT_PLAYER(1)

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_C ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_G ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_D ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_H ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )	// "l"
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )			// "t"
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )		// "w"
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )		// "f"
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )			// "b"
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )			// "s"

	PORT_START("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_A ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_E ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_HANAFUDA_YES ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("KEY6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_B ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_F ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_HANAFUDA_NO ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET ) PORT_PLAYER(2)

	PORT_START("KEY7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_C ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_G ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_D ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_H ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE ) PORT_PLAYER(2)	// "l"
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE ) PORT_PLAYER(2)		// "t"
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP ) PORT_PLAYER(2)	// "w"
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2)	// "f"
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG ) PORT_PLAYER(2)			// "b"
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL ) PORT_PLAYER(2)		// "s"
INPUT_PORTS_END

static INPUT_PORTS_START( HANAFUDA_KEYS_BET_ALT )
	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_A ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_E ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_HANAFUDA_YES ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )			// "t"
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_B ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_F ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_HANAFUDA_NO ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )			// "s"
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_C ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_G ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )			// "b"
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_D ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_H ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )		// "w"
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_A ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_E ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_HANAFUDA_YES ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE ) PORT_PLAYER(2)		// "t"
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("KEY6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_B ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_F ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_HANAFUDA_NO ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL ) PORT_PLAYER(2)		// "s"
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_C ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_G ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_BIG ) PORT_PLAYER(2)			// "b"
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_D ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_H ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP ) PORT_PLAYER(2)	// "w"
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( hanamai )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )	// Analyzer
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE3 )	// Memory Reset
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_INCLUDE( HANAFUDA_KEYS )
INPUT_PORTS_END

static INPUT_PORTS_START( hnkochou )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) PORT_CODE(KEYCODE_4)	// Pay
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE(0x04, IP_ACTIVE_LOW )	// Test (there isn't a dip switch)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )	// Analyzer
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE3 )	// Memory Reset
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3 )		// Note
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_INCLUDE( HANAFUDA_KEYS_BET )
INPUT_PORTS_END


static INPUT_PORTS_START( hnoridur )
	PORT_START("DSW0")	/* note that these are in reverse order wrt the others */
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Select Stage" )
	PORT_DIPSETTING(    0x02, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )	// Analyzer
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE3 )	// Memory Reset
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_INCLUDE( HANAFUDA_KEYS )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( hjingi )
	PORT_START("DSW0")	/* note that these are in reverse order wrt the others */
	PORT_DIPNAME( 0x80, 0x80, "Stage Select" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Gal" )	// "Renchan Gal"
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, "Game Music" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Suggest Move" )	// "Teach TEFUDA"
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, "Payout Rate" )
	PORT_DIPSETTING(    0x07, DEF_STR( Highest ) )
	PORT_DIPSETTING(    0x06, DEF_STR( Higher ) )
	PORT_DIPSETTING(    0x05, DEF_STR( High) )
	PORT_DIPSETTING(    0x04, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Very_Low ) )
//  PORT_DIPSETTING(    0x01, DEF_STR( ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( ) )
	PORT_DIPNAME( 0x08, 0x08, "Payout Rate Change" )
	PORT_DIPSETTING(    0x08, "Big" )
	PORT_DIPSETTING(    0x00, "Small" )
	PORT_DIPNAME( 0x10, 0x10, "Double-Up Game Rate" )
	PORT_DIPSETTING(    0x10, DEF_STR( High ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Low ) )
	PORT_DIPNAME( 0x20, 0x20, "GOKOU Odds" )
	PORT_DIPSETTING(    0x20, "100" )
	PORT_DIPSETTING(    0x00, "200" )
	PORT_DIPNAME( 0x40, 0x40, "GOKOU Cut" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "3-Renchan Bonus" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x04, 0x04, "Credits Per Note" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x18, 0x18, "Max Bet" )
	PORT_DIPSETTING(    0x18, "5" )
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPSETTING(    0x08, "20" )
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPNAME( 0x60, 0x60, "Min Rate To Play" )
	PORT_DIPSETTING(    0x60, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x80, 0x80, "Higi" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Pay Out Type" )
	PORT_DIPSETTING(    0x02, "Credit" )
	PORT_DIPSETTING(    0x00, "Hopper" )
	PORT_DIPNAME( 0x04, 0x04, "Hopper Switch" )
	PORT_DIPSETTING(    0x04, "Active Low" )
	PORT_DIPSETTING(    0x00, "Active High" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) PORT_CODE(KEYCODE_4)	// Pay
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE4 )	// 18B
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )	// Analyzer
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE3 )	// Memory Reset
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2    )	// Key In
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )	// 18A

	PORT_INCLUDE( HANAFUDA_KEYS_BET )
//  PORT_INCLUDE( HANAFUDA_KEYS_BET_ALT )

	PORT_START("BET")
	PORT_CONFNAME( 0x40, 0x40, "Allow Betting" )
	PORT_CONFSETTING(    0x40, DEF_STR( Yes ) )	// 2 keyboards, normal bet layout, hopper not pulsing in key test
//  PORT_CONFSETTING(    0x00, DEF_STR( No ) )  // 1 keyboard,  alt bet layout,    hopper pulsing in key test
INPUT_PORTS_END


static INPUT_PORTS_START( drgpunch )
	PORT_START("P1")
	PORT_BIT(  0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT(  0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT(  0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT(  0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT(  0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT(  0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT(  0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT(  0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("P2")
	PORT_BIT(  0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT(  0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT(  0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT(  0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT(  0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT(  0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT(  0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT(  0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("COINS")
	PORT_BIT(  0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(10)
	PORT_BIT(  0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(10)
	PORT_BIT(  0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT(  0x08, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT(  0x10, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT(  0x20, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT(  0x40, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT(  0x80, IP_ACTIVE_LOW, IPT_UNKNOWN  )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x04, DEF_STR( Difficulty ) )	// Time
	PORT_DIPSETTING(    0x00, "1 (Easy)" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x03, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x05, "6" )
	PORT_DIPSETTING(    0x06, "7" )
	PORT_DIPSETTING(    0x07, "8 (Hard)" )
	PORT_DIPNAME( 0x18, 0x10, "Vs Time" )
	PORT_DIPSETTING(    0x18, "8 s" )
	PORT_DIPSETTING(    0x10, "10 s" )
	PORT_DIPSETTING(    0x08, "12 s" )
	PORT_DIPSETTING(    0x00, "14 s" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 2-7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 2-8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( sprtmtch )
	PORT_INCLUDE( drgpunch )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x40, 0x40, "Intermissions" )			// Does not apply to drgpunch
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( mjfriday )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x04, 0x04, "PINFU with TSUMO" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x18, 0x18, "Player Strength" )
	PORT_DIPSETTING(    0x18, "Weak" )
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, "Strong" )
	PORT_DIPSETTING(    0x00, "Strongest" )
	PORT_DIPNAME( 0x60, 0x60, "CPU Strength" )
	PORT_DIPSETTING(    0x60, "Weak" )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, "Strong" )
	PORT_DIPSETTING(    0x00, "Strongest" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Auto TSUMO" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Full Test" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )	// "17B"
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )	// "18B"
	PORT_SERVICE(0x04, IP_ACTIVE_LOW )				// Test (there isn't a dip switch)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )	// Analyzer
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE3 )	// Memory Reset
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	// "06B"
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )	// "18A"

	PORT_INCLUDE( MAHJONG_KEYS )
INPUT_PORTS_END


static INPUT_PORTS_START( mjdialq2 )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, "Time Setting" )
	PORT_DIPSETTING(    0x07, "08:30" )
	PORT_DIPSETTING(    0x06, "09:00" )
	PORT_DIPSETTING(    0x05, "09:30" )
	PORT_DIPSETTING(    0x04, "10:00" )
	PORT_DIPSETTING(    0x03, "10:30" )
	PORT_DIPSETTING(    0x02, "11:00" )
	PORT_DIPSETTING(    0x01, "11:30" )
	PORT_DIPSETTING(    0x00, "12:00" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Select Special Item" ) /* Allows to select which one of the nine special items you want. */
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )	// "17B"
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )	// "18B"
	PORT_SERVICE_NO_TOGGLE(0x04, IP_ACTIVE_LOW)		// Test (there isn't a dip switch)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )	// Analyzer
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE3 )	// Memory Reset
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	// "06B"
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )	// "18A"

	PORT_INCLUDE( MAHJONG_KEYS )
INPUT_PORTS_END


static INPUT_PORTS_START( yarunara )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )		// 1,6
	PORT_DIPSETTING(    0x0c, DEF_STR( Normal ) )	// 3,4
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )		// 5,2
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )	// 7,0
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Use Password" )
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Set Date" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "PINFU with TSUMO" )
	PORT_DIPSETTING(    0x01, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 2-1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 2-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 2-3*" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 2-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Choose Bonus (Cheat)")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 2-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN  )	// "17B"
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN  )	// "18B"
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE  ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )	// Analyzer
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE3 )	// Memory Reset
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN  )	// "06B"
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1    )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_INCLUDE( MAHJONG_KEYS )
INPUT_PORTS_END

static INPUT_PORTS_START( hanayara )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )		// 1,6
	PORT_DIPSETTING(    0x0c, DEF_STR( Normal ) )	// 3,4
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )		// 5,2
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )	// 7,0
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Use Password" )
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Set Date" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "PINFU with TSUMO" )
	PORT_DIPSETTING(    0x01, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 2-1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 2-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 2-3*" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 2-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Choose Bonus (Cheat)")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 2-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN  )	// "17B"
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN  )	// "18B"
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE  ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )	// Analyzer
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE3 )	// Memory Reset
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN  )	// "06B"
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1    )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_INCLUDE( HANAFUDA_KEYS )
INPUT_PORTS_END

static INPUT_PORTS_START( quiztvqq )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Easy )    )
	PORT_DIPSETTING(    0x0c, DEF_STR( Normal )  )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard )    )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Voices" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Set Date" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, "Unknown 2-0&1" )
	PORT_DIPSETTING(    0x03, "0" )
	PORT_DIPSETTING(    0x02, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 2-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 2-3*" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 2-4*" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 2-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 2-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F1)	// Test, during boot
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("KEY1")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_START("KEY2")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_START("KEY3")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_START("KEY4")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("KEY6")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_START("KEY7")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_START("KEY8")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_START("KEY9")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( mcnpshnt )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x06, "0" )
	PORT_DIPSETTING(    0x04, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x20, 0x20, "Auto TSUMO" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, "Time Setting" )
	PORT_DIPSETTING(    0x07, "8:30" )
	PORT_DIPSETTING(    0x06, "9:00" )
	PORT_DIPSETTING(    0x05, "9:30" )
	PORT_DIPSETTING(    0x04, "10:00" )
	PORT_DIPSETTING(    0x03, "10:30" )
	PORT_DIPSETTING(    0x02, "11:00" )
	PORT_DIPSETTING(    0x01, "11:30" )
	PORT_DIPSETTING(    0x00, "12:00" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Buy Screen Bonus Points" ) /* Sets your points to 100 every time you arrive at the screen for buying special items. */
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	/* make your game last at least 4 or 5 minutes. Continue if necessary. Before the next round you begin,
    you will get some sort of message in Japanese stating that it is some sort of lucky time of day for you, and
    you get 100 bonus points (for purchasing items). */
	PORT_DIPNAME( 0x20, 0x20, "Lucky Time Of Day Bonus" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )	// Test
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )	// Analyzer
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE3 )	// Memory Reset
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_INCLUDE( MAHJONG_KEYS )
INPUT_PORTS_END


static INPUT_PORTS_START( nanajign )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x38, "0" )
	PORT_DIPSETTING(    0x30, "1" )
	PORT_DIPSETTING(    0x28, "2" )
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPSETTING(    0x18, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x08, "6" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )		//?
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )	//*
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )	//*
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )	//*
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )	//*
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )	// Test
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )	// Analyzer
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE3 )	// Memory Reset
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_INCLUDE( MAHJONG_KEYS )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( jantouki )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x07, "0" )	// 0 6 2
	PORT_DIPSETTING(    0x06, "1" )	// 0 6 1
	PORT_DIPSETTING(    0x05, "2" )	// 1 5 2
	PORT_DIPSETTING(    0x04, "3" )	// 1 5 1
	PORT_DIPSETTING(    0x03, "4" ) // 2 4 2
	PORT_DIPSETTING(    0x02, "5" )	// 2 4 1
	PORT_DIPSETTING(    0x01, "6" )	// 2 3 1
	PORT_DIPSETTING(    0x00, "7" )	// 2 2 1
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )	//*
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )	//*
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, "Time Setting" )
	PORT_DIPSETTING(    0x07, "08:30" )
	PORT_DIPSETTING(    0x06, "09:00" )
	PORT_DIPSETTING(    0x05, "09:30" )
	PORT_DIPSETTING(    0x04, "10:00" )
	PORT_DIPSETTING(    0x03, "10:30" )
	PORT_DIPSETTING(    0x02, "11:00" )
	PORT_DIPSETTING(    0x01, "11:30" )
	PORT_DIPSETTING(    0x00, "12:00" )
	PORT_DIPNAME( 0x08, 0x08, "Moles On Gal's Face" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Buy Screen Bonus Points" ) /* Sets your points to 100 every time you arrive at the screen for buying special items. */
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	/* make your game last at least 4 or 5 minutes. Continue if necessary. Before the next round you begin,
    you will get some sort of message in Japanese stating that it is some sort of lucky time of day for you, and
    you get 100 bonus points (for purchasing items). */
	PORT_DIPNAME( 0x20, 0x20, "Lucky Time Of Day Bonus" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )	//*
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE(0x04, IP_ACTIVE_LOW )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )	// Analyzer
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE3 )	// Memory Reset
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( mjelct3 )
	PORT_START("DSW0")	/* 7c21 (select = 00) */
	PORT_DIPNAME( 0x03, 0x03, "Difficulty?" )
	PORT_DIPSETTING(    0x03, "0" )	// 20
	PORT_DIPSETTING(    0x00, "1" )	// 32
	PORT_DIPSETTING(    0x01, "2" )	// 64
	PORT_DIPSETTING(    0x02, "3" )	// c8
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x30, 0x30, "Min Pay?" )
	PORT_DIPSETTING(    0x30, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x40, 0x40, "Allow Coin Out" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Win A Prize?" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")	/* 7c20 (select = 40) */
	PORT_DIPNAME( 0x0f, 0x07, "Pay Out Rate" )
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPSETTING(    0x01, "53" )
	PORT_DIPSETTING(    0x02, "56" )
	PORT_DIPSETTING(    0x03, "59" )
	PORT_DIPSETTING(    0x04, "62" )
	PORT_DIPSETTING(    0x05, "65" )
	PORT_DIPSETTING(    0x06, "68" )
	PORT_DIPSETTING(    0x07, "71" )
	PORT_DIPSETTING(    0x08, "75" )
	PORT_DIPSETTING(    0x09, "78" )
	PORT_DIPSETTING(    0x0a, "81" )
	PORT_DIPSETTING(    0x0b, "84" )
	PORT_DIPSETTING(    0x0c, "87" )
	PORT_DIPSETTING(    0x0d, "90" )
	PORT_DIPSETTING(    0x0e, "93" )
	PORT_DIPSETTING(    0x0f, "96" )
	PORT_DIPNAME( 0x30, 0x30, "Max Bet" )
	PORT_DIPSETTING(    0x30, "1" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) PORT_CODE(KEYCODE_4)	// Pay
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN  )	// 18B
	PORT_SERVICE( 0x04, IP_ACTIVE_LOW )				// Test
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )	// Analyzer
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE3 )	// Memory Reset
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2    )	// Note
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1    )	// Coin
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )	// Service

	PORT_INCLUDE( MAHJONG_KEYS_BET )

	PORT_START("DSW3")	/* 7c22 (select = 80) */
	PORT_DIPNAME( 0x07, 0x07, "YAKUMAN Bonus" )
	PORT_DIPSETTING(    0x07, "Cut" )
	PORT_DIPSETTING(    0x06, "1 T" )
	PORT_DIPSETTING(    0x05, "300" )
	PORT_DIPSETTING(    0x04, "500" )
	PORT_DIPSETTING(    0x03, "700" )
	PORT_DIPSETTING(    0x02, "1000" )
//  PORT_DIPSETTING(    0x01, "1000" )
//  PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPNAME( 0x08, 0x08, "YAKU times" )
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x10, 0x10, "Win Rate?" )
	PORT_DIPSETTING(    0x10, DEF_STR( High ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Low ) )
	PORT_DIPNAME( 0x20, 0x20, "Draw New Tile (Part 3 Only)" )
	PORT_DIPSETTING(    0x00, "Automatic" )
	PORT_DIPSETTING(    0x20, "Manual" )
	PORT_DIPNAME( 0x40, 0x40, "DonDen Key" )
	PORT_DIPSETTING(    0x40, "A" )
	PORT_DIPSETTING(    0x00, "Flip Flop" )
	PORT_DIPNAME( 0x80, 0x00, "Subtitle" )
	PORT_DIPSETTING(    0x80, "None (Part 2)" )
	PORT_DIPSETTING(    0x00, "Super Express (Part 3)" )

	PORT_START("DSW4")	/* 7c23 (select = c0) */
	PORT_DIPNAME( 0x01, 0x01, "Last Chance" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Pay Rate?" )
	PORT_DIPSETTING(    0x02, DEF_STR( High ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Low ) )
	PORT_DIPNAME( 0x04, 0x04, "Choose Bonus" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "In-Game Bet?" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "In-Game Music" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Select Girl" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Moles On Gal's Face" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("FAKE")	/* IN10 - Fake DSW */
	PORT_DIPNAME( 0xff, 0xff, "Allow Bets" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0xff, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( mjelctrn )
	PORT_START("DSW0")	/* 7c21 (select = 00) */
	PORT_DIPNAME( 0x03, 0x03, "Difficulty?" )
	PORT_DIPSETTING(    0x03, "0" )	// 20
	PORT_DIPSETTING(    0x00, "1" )	// 32
	PORT_DIPSETTING(    0x01, "2" )	// 64
	PORT_DIPSETTING(    0x02, "3" )	// c8
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x30, 0x30, "Min Pay?" )
	PORT_DIPSETTING(    0x30, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x40, 0x40, "Allow Coin Out" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Win A Prize?" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1") /* 7c20 (select = 40) */
	PORT_DIPNAME( 0x0f, 0x07, "Pay Out Rate" )
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPSETTING(    0x01, "53" )
	PORT_DIPSETTING(    0x02, "56" )
	PORT_DIPSETTING(    0x03, "59" )
	PORT_DIPSETTING(    0x04, "62" )
	PORT_DIPSETTING(    0x05, "65" )
	PORT_DIPSETTING(    0x06, "68" )
	PORT_DIPSETTING(    0x07, "71" )
	PORT_DIPSETTING(    0x08, "75" )
	PORT_DIPSETTING(    0x09, "78" )
	PORT_DIPSETTING(    0x0a, "81" )
	PORT_DIPSETTING(    0x0b, "84" )
	PORT_DIPSETTING(    0x0c, "87" )
	PORT_DIPSETTING(    0x0d, "90" )
	PORT_DIPSETTING(    0x0e, "93" )
	PORT_DIPSETTING(    0x0f, "96" )
	PORT_DIPNAME( 0x30, 0x30, "Max Bet" )
	PORT_DIPSETTING(    0x30, "1" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) PORT_CODE(KEYCODE_4)	// Pay
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN  )	// 18B
	PORT_SERVICE( 0x04, IP_ACTIVE_LOW )				// Test
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )	// Analyzer
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE3 )	// Memory Reset
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2    )	// Note
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1    )	// Coin
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )	// Service

	PORT_INCLUDE( MAHJONG_KEYS_BET )

	PORT_START("DSW3") /* 7c22 (select = 80) */
	PORT_DIPNAME( 0x07, 0x07, "YAKUMAN Bonus" )
	PORT_DIPSETTING(    0x07, "Cut" )
	PORT_DIPSETTING(    0x06, "1 T" )
	PORT_DIPSETTING(    0x05, "300" )
	PORT_DIPSETTING(    0x04, "500" )
	PORT_DIPSETTING(    0x03, "700" )
	PORT_DIPSETTING(    0x02, "1000" )
//  PORT_DIPSETTING(    0x01, "1000" )
//  PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPNAME( 0x08, 0x08, "YAKU times" )
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x10, 0x10, "Win Rate?" )
	PORT_DIPSETTING(    0x10, DEF_STR( High ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Low ) )
	PORT_DIPNAME( 0x20, 0x20, "Draw New Tile (Part 4 Only)" )
	PORT_DIPSETTING(    0x00, "Automatic" )
	PORT_DIPSETTING(    0x20, "Manual" )
	PORT_DIPNAME( 0x40, 0x40, "DonDen Key" )
	PORT_DIPSETTING(    0x40, "A" )
	PORT_DIPSETTING(    0x00, "Flip Flop" )
	PORT_DIPNAME( 0x80, 0x00, "Subtitle" )
	PORT_DIPSETTING(    0x80, "None (Part 2)" )
	PORT_DIPSETTING(    0x00, "???? (Part 4)" )

	PORT_START("DSW4") // 7c23 (select = c0)
	PORT_DIPNAME( 0x01, 0x01, "Last Chance" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Pay Rate?" )
	PORT_DIPSETTING(    0x02, DEF_STR( High ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Low ) )
	PORT_DIPNAME( 0x04, 0x04, "Choose Bonus" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "In-Game Bet?" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "In-Game Music" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Select Girl" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Girls" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("FAKE")	/* IN10 - Fake DSW */
	PORT_DIPNAME( 0xff, 0xff, "Allow Bets" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0xff, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( majxtal7 )
	PORT_START("DSW0")	/* select = 00 */
	PORT_DIPNAME( 0x03, 0x03, "Difficulty?" )
	PORT_DIPSETTING(    0x03, "0" )	// 20
	PORT_DIPSETTING(    0x00, "1" )	// 32
	PORT_DIPSETTING(    0x01, "2" )	// 64
	PORT_DIPSETTING(    0x02, "3" )	// c8
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x30, 0x30, "Min Pay?" )
	PORT_DIPSETTING(    0x30, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x40, 0x40, "Allow Coin Out" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Win A Prize?" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1") /* select = 40  */
	PORT_DIPNAME( 0x0f, 0x07, "Pay Out Rate" )
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPSETTING(    0x01, "53" )
	PORT_DIPSETTING(    0x02, "56" )
	PORT_DIPSETTING(    0x03, "59" )
	PORT_DIPSETTING(    0x04, "62" )
	PORT_DIPSETTING(    0x05, "65" )
	PORT_DIPSETTING(    0x06, "68" )
	PORT_DIPSETTING(    0x07, "71" )
	PORT_DIPSETTING(    0x08, "75" )
	PORT_DIPSETTING(    0x09, "78" )
	PORT_DIPSETTING(    0x0a, "81" )
	PORT_DIPSETTING(    0x0b, "84" )
	PORT_DIPSETTING(    0x0c, "87" )
	PORT_DIPSETTING(    0x0d, "90" )
	PORT_DIPSETTING(    0x0e, "93" )
	PORT_DIPSETTING(    0x0f, "96" )
	PORT_DIPNAME( 0x30, 0x30, "Max Bet" )
	PORT_DIPSETTING(    0x30, "1" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) PORT_CODE(KEYCODE_4)	// Pay
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN  )	// 18B
	PORT_SERVICE( 0x04, IP_ACTIVE_LOW )				// Test
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )	// Analyzer
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE3 )	// Memory Reset
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2    )	// Note
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1    )	// Coin
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )	// Service

	PORT_INCLUDE( MAHJONG_KEYS_BET )

	PORT_START("DSW3") /* select = 80 */
	PORT_DIPNAME( 0x07, 0x07, "YAKUMAN Bonus" )
	PORT_DIPSETTING(    0x07, "Cut" )
	PORT_DIPSETTING(    0x06, "1 T" )
	PORT_DIPSETTING(    0x05, "300" )
	PORT_DIPSETTING(    0x04, "500" )
	PORT_DIPSETTING(    0x03, "700" )
	PORT_DIPSETTING(    0x02, "1000" )
//  PORT_DIPSETTING(    0x01, "1000" )
//  PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPNAME( 0x08, 0x08, "YAKU times" )
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x10, 0x10, "Win Rate?" )
	PORT_DIPSETTING(    0x10, DEF_STR( High ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Low ) )
	PORT_DIPNAME( 0x20, 0x20, "Draw New Tile (Part 4 Only)" )
	PORT_DIPSETTING(    0x00, "Automatic" )
	PORT_DIPSETTING(    0x20, "Manual" )
	PORT_DIPNAME( 0x40, 0x40, "DonDen Key" )
	PORT_DIPSETTING(    0x40, "A" )
	PORT_DIPSETTING(    0x00, "Flip Flop" )
	PORT_DIPNAME( 0x80, 0x80, "Title" )
	PORT_DIPSETTING(    0x80, "X-Tal" )
	PORT_DIPSETTING(    0x00, "Diamond" )

	PORT_START("DSW4") /* select = c0 */
	PORT_DIPNAME( 0x01, 0x01, "Last Chance" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Pay Rate?" )
	PORT_DIPSETTING(    0x02, DEF_STR( High ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Low ) )
	PORT_DIPNAME( 0x04, 0x04, "Choose Bonus" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "In-Game Bet?" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "In-Game Music" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Select Girl" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("FAKE")	/* IN10 - Fake DSW */
	PORT_DIPNAME( 0xff, 0xff, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0xff, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( neruton )
	PORT_START("DSW0") /* 6a77 (select = 00) */
	PORT_DIPNAME( 0x07, 0x07, "Time Setting" )
	PORT_DIPSETTING(    0x07, "08:30" )
	PORT_DIPSETTING(    0x06, "09:00" )
	PORT_DIPSETTING(    0x05, "09:30" )
	PORT_DIPSETTING(    0x04, "10:00" )
	PORT_DIPSETTING(    0x03, "10:30" )
	PORT_DIPSETTING(    0x02, "11:00" )
	PORT_DIPSETTING(    0x01, "11:30" )
	PORT_DIPSETTING(    0x00, "12:00" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, "30" )
	PORT_DIPSETTING(    0x20, "60" )
	PORT_DIPNAME( 0x40, 0x40, "See Opponent's Tiles (Cheat)")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("DSW1")	/* 6a76 (select = 40) */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x04, 0x04, "PINFU with TSUMO" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x38, 0x20, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x38, "1 (Easy)" )
	PORT_DIPSETTING(    0x30, "2" )
	PORT_DIPSETTING(    0x28, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x18, "5" )
	PORT_DIPSETTING(    0x10, "6" )
	PORT_DIPSETTING(    0x08, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hard)" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )	// 17B
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )	// 18B
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F1)	// Test
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )	// Analyzer
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE3 )	// Memory Reset
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	// 06B
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )	// Coin
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )	// 18A

	PORT_INCLUDE( MAHJONG_KEYS )

	/* 2008-06 FP: the following are needed to make happy the read handlers shared with mjelctrn*/
	PORT_START("DSW3")
	PORT_START("DSW4")
	PORT_START("FAKE")
INPUT_PORTS_END


static INPUT_PORTS_START( tenkai )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x0f, 0x07, "Pay Out Rate" )
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPSETTING(    0x01, "53" )
	PORT_DIPSETTING(    0x02, "56" )
	PORT_DIPSETTING(    0x03, "59" )
	PORT_DIPSETTING(    0x04, "62" )
	PORT_DIPSETTING(    0x05, "65" )
	PORT_DIPSETTING(    0x06, "68" )
	PORT_DIPSETTING(    0x07, "71" )
	PORT_DIPSETTING(    0x08, "75" )
	PORT_DIPSETTING(    0x09, "78" )
	PORT_DIPSETTING(    0x0a, "81" )
	PORT_DIPSETTING(    0x0b, "84" )
	PORT_DIPSETTING(    0x0c, "87" )
	PORT_DIPSETTING(    0x0d, "90" )
	PORT_DIPSETTING(    0x0e, "93" )
	PORT_DIPSETTING(    0x0f, "96" )
	PORT_DIPNAME( 0x30, 0x10, "Odds Rate" )
	PORT_DIPSETTING(    0x30, "1 2 4 8 12 16 24 32" )
	PORT_DIPSETTING(    0x00, "1 2 3 5 8 15 30 50" )
	PORT_DIPSETTING(    0x10, "1 2 3 5 10 25 50 100" )
	PORT_DIPSETTING(    0x20, "1 2 3 5 10 50 100 200" )
	PORT_DIPNAME( 0xc0, 0x40, "Max Bet" )
	PORT_DIPSETTING(    0xc0, "1" )
	PORT_DIPSETTING(    0x80, "5" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x00, "20" )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, "Unknown 1-0&1" )
	PORT_DIPSETTING(    0x03, "1:1" )
	PORT_DIPSETTING(    0x02, "1:2" )
	PORT_DIPSETTING(    0x01, "1:5" )
	PORT_DIPSETTING(    0x00, "1:10" )
	PORT_DIPNAME( 0x0c, 0x0c, "Min Rate To Play" )
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x70, 0x70, "YAKUMAN Bonus" )
	PORT_DIPSETTING(    0x70, "Cut" )
	PORT_DIPSETTING(    0x60, "1 T" )
	PORT_DIPSETTING(    0x50, "300" )
	PORT_DIPSETTING(    0x40, "500" )
	PORT_DIPSETTING(    0x30, "700" )
	PORT_DIPSETTING(    0x20, "1000" )
//  PORT_DIPSETTING(    0x10, "1000" )
//  PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 1-7" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x80, "2" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Unknown 2-0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 2-1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Girls (Demo)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 2-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 2-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 2-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 2-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DonDen Key" )
	PORT_DIPSETTING(    0x80, "Start" )
	PORT_DIPSETTING(    0x00, "Flip Flop" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "In-Game Music" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Select Girl" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Moles On Gal's Face" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 3-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 3-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 3-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Set Date" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW4")	/* (top) */
	PORT_DIPNAME( 0x01, 0x01, "Credits Per Note" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown top-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown top-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown top-4" )
	PORT_DIPSETTING(    0x10, "8" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x20, 0x20, "Unknown top-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Subtitle" )
	PORT_DIPSETTING(    0x40, DEF_STR( None ) )
	PORT_DIPSETTING(    0x00, "Part 2" )
	PORT_DIPNAME( 0x80, 0x80, "Unknown top-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) PORT_CODE(KEYCODE_4)	// Pay
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN  )	// 18B
	PORT_SERVICE( 0x04, IP_ACTIVE_LOW )				// Test
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )	// Analyzer
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE3 )	// Memory Reset
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2    )	// Note
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1    )	// Coin
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )	// Service

	PORT_INCLUDE( MAHJONG_KEYS_BET )
INPUT_PORTS_END


static INPUT_PORTS_START( mjreach )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x0f, 0x07, "Pay Out Rate" )
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPSETTING(    0x01, "53" )
	PORT_DIPSETTING(    0x02, "56" )
	PORT_DIPSETTING(    0x03, "59" )
	PORT_DIPSETTING(    0x04, "62" )
	PORT_DIPSETTING(    0x05, "65" )
	PORT_DIPSETTING(    0x06, "68" )
	PORT_DIPSETTING(    0x07, "71" )
	PORT_DIPSETTING(    0x08, "75" )
	PORT_DIPSETTING(    0x09, "78" )
	PORT_DIPSETTING(    0x0a, "81" )
	PORT_DIPSETTING(    0x0b, "84" )
	PORT_DIPSETTING(    0x0c, "87" )
	PORT_DIPSETTING(    0x0d, "90" )
	PORT_DIPSETTING(    0x0e, "93" )
	PORT_DIPSETTING(    0x0f, "96" )
	PORT_DIPNAME( 0x30, 0x10, "Odds Rate" )
	PORT_DIPSETTING(    0x30, "1 2 4 8 12 16 24 32" )
	PORT_DIPSETTING(    0x00, "1 2 3 5 8 15 30 50" )
	PORT_DIPSETTING(    0x10, "1 2 3 5 10 25 50 100" )
	PORT_DIPSETTING(    0x20, "1 2 3 5 10 50 100 200" )
	PORT_DIPNAME( 0xc0, 0x40, "Max Bet" )
	PORT_DIPSETTING(    0xc0, "1" )
	PORT_DIPSETTING(    0x80, "5" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x00, "20" )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, "Unknown 1-0&1" )
	PORT_DIPSETTING(    0x03, "1:1" )
	PORT_DIPSETTING(    0x02, "1:2" )
	PORT_DIPSETTING(    0x01, "1:5" )
	PORT_DIPSETTING(    0x00, "1:10" )
	PORT_DIPNAME( 0x0c, 0x0c, "Min Rate To Play" )
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x70, 0x70, "YAKUMAN Bonus" )
	PORT_DIPSETTING(    0x70, "Cut" )
	PORT_DIPSETTING(    0x60, "1 T" )
	PORT_DIPSETTING(    0x50, "300" )
	PORT_DIPSETTING(    0x40, "500" )
	PORT_DIPSETTING(    0x30, "700" )
	PORT_DIPSETTING(    0x20, "1000" )
//  PORT_DIPSETTING(    0x10, "1000" )
//  PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 1-7" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x80, "2" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Unknown 2-0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 2-1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 2-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 2-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 2-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 2-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 2-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 2-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 3-1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 3-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DonDen Key" )
	PORT_DIPSETTING(    0x08, "Start" )
	PORT_DIPSETTING(    0x00, "Flip Flop" )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 3-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 3-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 3-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 3-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW4")	/* 4 (top) */
	PORT_DIPNAME( 0x01, 0x01, "Credits Per Note" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Unknown top-2&3" )
	PORT_DIPSETTING(    0x0c, "300" )
	PORT_DIPSETTING(    0x08, "500" )
	PORT_DIPSETTING(    0x04, "700" )
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPNAME( 0x10, 0x10, "Unknown top-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown top-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown top-6" )
	PORT_DIPSETTING(    0x40, "1" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x80, 0x80, "Unknown top-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) PORT_CODE(KEYCODE_4)	// Pay
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN  )	// 18B
	PORT_SERVICE( 0x04, IP_ACTIVE_LOW )				// Test
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )	// Analyzer
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE3 )	// Memory Reset
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2    )	// Note
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1    )	// Coin
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )	// Service

	PORT_INCLUDE( MAHJONG_KEYS_BET )
INPUT_PORTS_END

static INPUT_PORTS_START( htengoku )
	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE4 )	// medal out
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE  ) PORT_NAME(DEF_STR( Test )) PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )	// analyzer
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE3 )	// data clear
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )	// note
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_INCLUDE( HANAFUDA_KEYS_BET )

	PORT_START("DSW0")	/* IN11 - DSW1 */
	PORT_DIPNAME( 0x01, 0x01, "Show Girls" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, "Select Stage" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x04, "Secret Trick" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, "Secret Character" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, "In Game Sounds" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Guide" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")	/* IN12 - DSW2 */
	PORT_DIPNAME( 0x07, 0x05, "Payout Rate" )
	PORT_DIPSETTING(    0x00, "Lowest" )
	PORT_DIPSETTING(    0x01, "Lower" )
	PORT_DIPSETTING(    0x02, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x03, "Bit Low" )
	PORT_DIPSETTING(    0x04, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x05, DEF_STR( High ) )
	PORT_DIPSETTING(    0x06, DEF_STR( Higher ) )
	PORT_DIPSETTING(    0x07, DEF_STR( Highest ) )
	PORT_DIPNAME( 0x08, 0x08, "Payout Wave" )
	PORT_DIPSETTING(    0x00, "Small" )
	PORT_DIPSETTING(    0x08, "Big" )
	PORT_DIPNAME( 0x10, 0x10, "Goko Yaku" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x60, 0x60, "Double-Up Win Rate" )
	PORT_DIPSETTING(    0x00, "65%" )
	PORT_DIPSETTING(    0x20, "70%" )
	PORT_DIPSETTING(    0x40, "75%" )
	PORT_DIPSETTING(    0x60, "80%" )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 2-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")	/* IN13 - DSW3 */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x04, 0x04, "Credits Per Note" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x04, "10" )
	PORT_DIPNAME( 0x08, 0x08, "Max Bet" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x30, 0x30, "Min Rate To Start" )
	PORT_DIPSETTING(    0x30, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0xc0, 0xc0, "Credits Limit" )
	PORT_DIPSETTING(    0xc0, "1000" )
	PORT_DIPSETTING(    0x80, "2000" )
	PORT_DIPSETTING(    0x40, "3000" )
	PORT_DIPSETTING(    0x00, "5000" )

	PORT_START("DSW3")	/* IN14 - DSW4 */
	PORT_DIPNAME( 0x03, 0x03, "Odds For Goko" )
	PORT_DIPSETTING(    0x03, "100" )
	PORT_DIPSETTING(    0x02, "200" )
	PORT_DIPSETTING(    0x01, "250" )
	PORT_DIPSETTING(    0x00, "300" )
	PORT_DIPNAME( 0x0c, 0x0c, "Odds For Shiko" )
	PORT_DIPSETTING(    0x0c, "50" )
	PORT_DIPSETTING(    0x08, "60" )
	PORT_DIPSETTING(    0x04, "70" )
	PORT_DIPSETTING(    0x00, "80" )
	PORT_DIPNAME( 0x30, 0x30, "Odds For Ameshiko" )
	PORT_DIPSETTING(    0x30, "20" )
	PORT_DIPSETTING(    0x20, "30" )
	PORT_DIPSETTING(    0x10, "40" )
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 4-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 4-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW4")	/* IN15 - DSWs top bits */
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x02, 0x02, "Set Clock" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Unknown 2-8" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 2-9" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Pay Out Type" )
	PORT_DIPSETTING(    0x10, "Credit" )
	PORT_DIPSETTING(    0x00, "Hopper" )
	PORT_DIPNAME( 0x20, 0x20, "Hopper Switch" )
	PORT_DIPSETTING(    0x20, "Active Low" )
	PORT_DIPSETTING(    0x00, "Active High" )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 4-8" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 4-9" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( gekisha )
	PORT_START("DSW1") // $7C20
	PORT_DIPNAME( 0x0f, 0x07, "Pay Out Rate" )
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPSETTING(    0x01, "53" )
	PORT_DIPSETTING(    0x02, "56" )
	PORT_DIPSETTING(    0x03, "59" )
	PORT_DIPSETTING(    0x04, "62" )
	PORT_DIPSETTING(    0x05, "65" )
	PORT_DIPSETTING(    0x06, "68" )
	PORT_DIPSETTING(    0x07, "71" )
	PORT_DIPSETTING(    0x08, "75" )
	PORT_DIPSETTING(    0x09, "78" )
	PORT_DIPSETTING(    0x0a, "81" )
	PORT_DIPSETTING(    0x0b, "84" )
	PORT_DIPSETTING(    0x0c, "87" )
	PORT_DIPSETTING(    0x0d, "90" )
	PORT_DIPSETTING(    0x0e, "93" )
	PORT_DIPSETTING(    0x0f, "96" )
	PORT_DIPNAME( 0x30, 0x30, "Max Bet" )
	PORT_DIPSETTING(    0x30, "1" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x40, 0x40, "Credits Per Note" )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2") // $7C21
	PORT_DIPNAME( 0x03, 0x03, "Odds Rate" )
	PORT_DIPSETTING(    0x03, "1 2 4 8 12 16 24 32" )
	PORT_DIPSETTING(    0x00, "1 2 3 5 8 15 30 50" )
	PORT_DIPSETTING(    0x01, "1 2 3 5 10 25 50 100 " )
	PORT_DIPSETTING(    0x02, "1 2 3 5 10 50 100 200" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x30, 0x30, "Min Rate To Play" )
	PORT_DIPSETTING(    0x30, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 2-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 2-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")	// $7c22
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x07, "0" )
	PORT_DIPSETTING(    0x06, "1" )
	PORT_DIPSETTING(    0x05, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x03, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x01, "6" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 3-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Unknown 3-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 3-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Unknown 3-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DonDen Key" )
	PORT_DIPSETTING(    0x80, "A" )
	PORT_DIPSETTING(    0x00, "Flip Flop" )

	PORT_START("DSW4")	// $7c23
	PORT_DIPNAME( 0x01, 0x01, "Unknown 4-0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Unknown 4-1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Auto Tsumo after Reach" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Unknown 4-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Girls (Demo)" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, "Unknown 4-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Unknown 4-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("BET")
	PORT_DIPNAME( 0x40, 0x40, "Bet" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) PORT_CODE(KEYCODE_4)	// Pay
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_SERVICE( 0x04, IP_ACTIVE_LOW )				// Test
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 )	// Analyzer
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE3 )	// Memory Reset
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2    )	// Note
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1    )	// Coin
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN  )

	PORT_INCLUDE( MAHJONG_KEYS_BET )
INPUT_PORTS_END


/***************************************************************************


                                Machine Drivers


***************************************************************************/

static MACHINE_START( dynax )
{
	dynax_state *state = (dynax_state *)machine->driver_data;

	state->maincpu = machine->device("maincpu");
	state->soundcpu = machine->device("soundcpu");
	state->rtc = machine->device("rtc");
	state->ymsnd = machine->device("ymsnd");

	state_save_register_global(machine, state->sound_irq);
	state_save_register_global(machine, state->vblank_irq);
	state_save_register_global(machine, state->blitter_irq);
	state_save_register_global(machine, state->blitter2_irq);
	state_save_register_global(machine, state->soundlatch_irq);
	state_save_register_global(machine, state->sound_vblank_irq);

	state_save_register_global(machine, state->input_sel);
	state_save_register_global(machine, state->dsw_sel);
	state_save_register_global(machine, state->keyb);
	state_save_register_global(machine, state->coins);
	state_save_register_global(machine, state->hopper);
	state_save_register_global(machine, state->hnoridur_bank);
	state_save_register_global(machine, state->palbank);
	state_save_register_global(machine, state->msm5205next);
	state_save_register_global(machine, state->resetkludge);
	state_save_register_global(machine, state->toggle);
	state_save_register_global(machine, state->toggle_cpu1);
	state_save_register_global(machine, state->yarunara_clk_toggle);
	state_save_register_global(machine, state->soundlatch_ack);
	state_save_register_global(machine, state->soundlatch_full);
	state_save_register_global(machine, state->latch);
	state_save_register_global(machine, state->rombank);
	state_save_register_global(machine, state->tenkai_p5_val);
	state_save_register_global(machine, state->tenkai_6c);
	state_save_register_global(machine, state->tenkai_70);
	state_save_register_global_array(machine, state->gekisha_val);
	state_save_register_global_array(machine, state->palette_ram);
	state_save_register_global(machine, state->gekisha_rom_enable);
}

static MACHINE_RESET( dynax )
{
	dynax_state *state = (dynax_state *)machine->driver_data;

	if (machine->device("msm") != NULL)
		MACHINE_RESET_CALL(adpcm);

	state->sound_irq = 0;
	state->vblank_irq = 0;
	state->blitter_irq = 0;
	state->blitter2_irq = 0;
	state->soundlatch_irq = 0;
	state->sound_vblank_irq = 0;

	state->input_sel = 0;
	state->dsw_sel = 0;
	state->keyb = 0;
	state->coins = 0;
	state->hopper = 0;
	state->hnoridur_bank = 0;
	state->palbank = 0;
	state->msm5205next = 0;
	state->resetkludge = 0;
	state->toggle = 0;
	state->toggle_cpu1 = 0;
	state->yarunara_clk_toggle = 0;
	state->soundlatch_ack = 0;
	state->soundlatch_full = 0;
	state->latch = 0;
	state->rombank = 0;
	state->tenkai_p5_val = 0;
	state->tenkai_6c = 0;
	state->tenkai_70 = 0;
	state->gekisha_val[0] = 0;
	state->gekisha_val[1] = 0;
	state->gekisha_rom_enable = 0;

	memset(state->palette_ram, 0, ARRAY_LENGTH(state->palette_ram));
}

static MACHINE_START( hanamai )
{
	UINT8 *ROM = memory_region(machine, "maincpu");
	memory_configure_bank(machine, "bank1", 0, 0x10, &ROM[0x8000], 0x8000);

	MACHINE_START_CALL(dynax);
}

static MACHINE_START( hnoridur )
{
	UINT8 *ROM = memory_region(machine, "maincpu");
	int bank_n = (memory_region_length(machine, "maincpu") - 0x10000) / 0x8000;

	memory_configure_bank(machine, "bank1", 0, bank_n, &ROM[0x10000], 0x8000);

	MACHINE_START_CALL(dynax);
}

static MACHINE_START( htengoku )
{
	UINT8 *ROM = memory_region(machine, "maincpu");

	memory_configure_bank(machine, "bank1", 0, 8, &ROM[0x10000], 0x8000);

	MACHINE_START_CALL(dynax);
}

/***************************************************************************
                                Hana no Mai
***************************************************************************/

static const ym2203_interface hanamai_ym2203_interface =
{
	{
		AY8910_LEGACY_OUTPUT,
		AY8910_DEFAULT_LOADS,
		DEVCB_INPUT_PORT("DSW1"),			/* Port A Read: DSW */
		DEVCB_INPUT_PORT("DSW0"),			/* Port B Read: DSW */
		DEVCB_NULL,							/* Port A Write */
		DEVCB_NULL,							/* Port B Write */
	},
	sprtmtch_sound_callback		/* IRQ handler */
};

static const msm5205_interface hanamai_msm5205_interface =
{
	adpcm_int,			/* IRQ handler */
	MSM5205_S48_4B		/* 8 KHz, 4 Bits  */
};



static MACHINE_DRIVER_START( hanamai )

	/* driver data */
	MDRV_DRIVER_DATA(dynax_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu",Z80,22000000 / 4)	/* 5.5MHz */
	MDRV_CPU_PROGRAM_MAP(sprtmtch_mem_map)
	MDRV_CPU_IO_MAP(hanamai_io_map)
	MDRV_CPU_VBLANK_INT("screen", sprtmtch_vblank_interrupt)	/* IM 0 needs an opcode on the data bus */

	MDRV_MACHINE_START(hanamai)
	MDRV_MACHINE_RESET(dynax)

	MDRV_NVRAM_HANDLER(generic_0fill)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(512, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 512-1-4, 16+8, 255-8)

	MDRV_PALETTE_LENGTH(512)

	MDRV_PALETTE_INIT(sprtmtch)			// static palette
	MDRV_VIDEO_START(hanamai)
	MDRV_VIDEO_UPDATE(hanamai)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("aysnd", AY8910, 22000000 / 8)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.20)

	MDRV_SOUND_ADD("ymsnd", YM2203, 22000000 / 8)
	MDRV_SOUND_CONFIG(hanamai_ym2203_interface)
	MDRV_SOUND_ROUTE(0, "mono", 0.20)
	MDRV_SOUND_ROUTE(1, "mono", 0.20)
	MDRV_SOUND_ROUTE(2, "mono", 0.20)
	MDRV_SOUND_ROUTE(3, "mono", 0.50)

	MDRV_SOUND_ADD("msm", MSM5205, 384000)
	MDRV_SOUND_CONFIG(hanamai_msm5205_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END



/***************************************************************************
                                Hana Oriduru
***************************************************************************/

static const ay8910_interface hnoridur_ay8910_interface =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_INPUT_PORT("DSW0")		/* Port A Read: DSW */
};

static MACHINE_DRIVER_START( hnoridur )

	/* driver data */
	MDRV_DRIVER_DATA(dynax_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu",Z80,22000000 / 4)	/* 5.5MHz */
	MDRV_CPU_PROGRAM_MAP(hnoridur_mem_map)
	MDRV_CPU_IO_MAP(hnoridur_io_map)
	MDRV_CPU_VBLANK_INT("screen", sprtmtch_vblank_interrupt)	/* IM 0 needs an opcode on the data bus */

	MDRV_MACHINE_START(hnoridur)
	MDRV_MACHINE_RESET(dynax)

	MDRV_NVRAM_HANDLER(generic_0fill)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(512, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 512-1-4, 16, 256-1)

	MDRV_PALETTE_LENGTH(16*256)

	MDRV_VIDEO_START(hnoridur)
	MDRV_VIDEO_UPDATE(hnoridur)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("aysnd", AY8910, 22000000 / 16)
	MDRV_SOUND_CONFIG(hnoridur_ay8910_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.20)

	MDRV_SOUND_ADD("ymsnd", YM2413, 3579545)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MDRV_SOUND_ADD("msm", MSM5205, 384000)
	MDRV_SOUND_CONFIG(hanamai_msm5205_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


/***************************************************************************
                                Hana Jingi
***************************************************************************/

static MACHINE_DRIVER_START( hjingi )

	/* driver data */
	MDRV_DRIVER_DATA(dynax_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu",Z80, XTAL_22MHz / 4)
	MDRV_CPU_PROGRAM_MAP(hjingi_mem_map)
	MDRV_CPU_IO_MAP(hjingi_io_map)
	MDRV_CPU_VBLANK_INT("screen", sprtmtch_vblank_interrupt)	/* IM 0 needs an opcode on the data bus */

	MDRV_MACHINE_START(hnoridur)
	MDRV_MACHINE_RESET(dynax)

	MDRV_NVRAM_HANDLER(generic_0fill)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(512, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 512-1-4, 16, 256-1)

	MDRV_PALETTE_LENGTH(16*256)

	MDRV_VIDEO_START(hnoridur)
	MDRV_VIDEO_UPDATE(hnoridur)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("aysnd", AY8910, XTAL_22MHz / 16)
	MDRV_SOUND_CONFIG(hnoridur_ay8910_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.20)

	MDRV_SOUND_ADD("ymsnd", YM2413, XTAL_3_579545MHz )
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MDRV_SOUND_ADD("msm", MSM5205, XTAL_384kHz )
	MDRV_SOUND_CONFIG(hanamai_msm5205_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


/***************************************************************************
                                Sports Match
***************************************************************************/

static const ym2203_interface sprtmtch_ym2203_interface =
{
	{
		AY8910_LEGACY_OUTPUT,
		AY8910_DEFAULT_LOADS,
		DEVCB_INPUT_PORT("DSW0"),	/* Port A Read: DSW */
		DEVCB_INPUT_PORT("DSW1"),	/* Port B Read: DSW */
		DEVCB_NULL,					/* Port A Write */
		DEVCB_NULL,					/* Port B Write */
	},
	sprtmtch_sound_callback,	/* IRQ handler */
};

static MACHINE_DRIVER_START( sprtmtch )

	/* driver data */
	MDRV_DRIVER_DATA(dynax_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80,22000000 / 4)	/* 5.5MHz */
	MDRV_CPU_PROGRAM_MAP(sprtmtch_mem_map)
	MDRV_CPU_IO_MAP(sprtmtch_io_map)
	MDRV_CPU_VBLANK_INT("screen", sprtmtch_vblank_interrupt)	/* IM 0 needs an opcode on the data bus */

	MDRV_MACHINE_START(hanamai)
	MDRV_MACHINE_RESET(dynax)

	MDRV_NVRAM_HANDLER(generic_0fill)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(512, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 512-1, 16, 256-1)

	MDRV_PALETTE_LENGTH(512)

	MDRV_PALETTE_INIT(sprtmtch)			// static palette
	MDRV_VIDEO_START(sprtmtch)
	MDRV_VIDEO_UPDATE(sprtmtch)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ymsnd", YM2203, 22000000 / 8)
	MDRV_SOUND_CONFIG(sprtmtch_ym2203_interface)
	MDRV_SOUND_ROUTE(0, "mono", 0.20)
	MDRV_SOUND_ROUTE(1, "mono", 0.20)
	MDRV_SOUND_ROUTE(2, "mono", 0.20)
	MDRV_SOUND_ROUTE(3, "mono", 1.0)
MACHINE_DRIVER_END


/***************************************************************************
                            Mahjong Friday
***************************************************************************/

static MACHINE_DRIVER_START( mjfriday )

	/* driver data */
	MDRV_DRIVER_DATA(dynax_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu",Z80,24000000/4)	/* 6 MHz? */
	MDRV_CPU_PROGRAM_MAP(sprtmtch_mem_map)
	MDRV_CPU_IO_MAP(mjfriday_io_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_MACHINE_START(hanamai)
	MDRV_MACHINE_RESET(dynax)

	MDRV_NVRAM_HANDLER(generic_0fill)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 256-1, 16, 256-1)

	MDRV_PALETTE_LENGTH(512)

	MDRV_PALETTE_INIT(sprtmtch)			// static palette
	MDRV_VIDEO_START(mjdialq2)
	MDRV_VIDEO_UPDATE(mjdialq2)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ymsnd", YM2413, 24000000/6)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


/***************************************************************************
                            Mahjong Dial Q2
***************************************************************************/

static MACHINE_DRIVER_START( mjdialq2 )

	/* basic machine hardware */
	MDRV_IMPORT_FROM( mjfriday )
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(mjdialq2_mem_map)
MACHINE_DRIVER_END


/***************************************************************************
                    Yarunara / Quiz TV Q&Q / Mahjong Angels
***************************************************************************/

/* the old code here didn't work..
  what was it trying to do?
  set an irq and clear it before its even taken? */

static INTERRUPT_GEN( yarunara_clock_interrupt )
{
	dynax_state *state = (dynax_state *)device->machine->driver_data;
	state->yarunara_clk_toggle ^= 1;

	if (state->yarunara_clk_toggle == 1)
		state->sound_irq = 0;
	else
		state->sound_irq = 1;

	sprtmtch_update_irq(device->machine);
}

static MACHINE_DRIVER_START( yarunara )

	/* basic machine hardware */
	MDRV_IMPORT_FROM( hnoridur )
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(yarunara_mem_map)
	MDRV_CPU_IO_MAP(yarunara_io_map)
	MDRV_CPU_PERIODIC_INT(yarunara_clock_interrupt, 60)	// RTC

	MDRV_NVRAM_HANDLER(generic_0fill)

	MDRV_SCREEN_MODIFY("screen")
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_VISIBLE_AREA(0, 336-1, 8, 256-1-8-1)

	/* devices */
	MDRV_MSM6242_ADD("rtc")
MACHINE_DRIVER_END


/***************************************************************************
                            Mahjong Campus Hunting
***************************************************************************/

static MACHINE_DRIVER_START( mcnpshnt )

	MDRV_IMPORT_FROM( hnoridur )
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(mcnpshnt_mem_map)
	MDRV_CPU_IO_MAP(mcnpshnt_io_map)

	MDRV_VIDEO_START(mcnpshnt)	// different priorities
MACHINE_DRIVER_END


/***************************************************************************
                            7jigen
***************************************************************************/

static MACHINE_DRIVER_START( nanajign )

	MDRV_IMPORT_FROM( hnoridur )
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(nanajign_mem_map)
	MDRV_CPU_IO_MAP(nanajign_io_map)
MACHINE_DRIVER_END


/***************************************************************************
                                Jantouki
***************************************************************************/

// dual monitor, 2 CPU's, 2 blitters

static const ym2203_interface jantouki_ym2203_interface =
{
	{
		AY8910_LEGACY_OUTPUT,
		AY8910_DEFAULT_LOADS,
		DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL
	},
	jantouki_sound_callback		/* IRQ handler */
};

static const msm5205_interface jantouki_msm5205_interface =
{
	adpcm_int_cpu1,			/* IRQ handler */
	MSM5205_S48_4B		/* 8 KHz, 4 Bits  */
};

static MACHINE_START( jantouki )
{
	dynax_state *state = (dynax_state *)machine->driver_data;
	UINT8 *MAIN = memory_region(machine, "maincpu");
	UINT8 *SOUND = memory_region(machine, "soundcpu");

	memory_configure_bank(machine, "bank1", 0, 0x10, &MAIN[0x8000],  0x8000);
	memory_configure_bank(machine, "bank2", 0, 12,   &SOUND[0x8000], 0x8000);

	state->top_scr = machine->device("top");
	state->bot_scr = machine->device("bottom");

	MACHINE_START_CALL(dynax);
}

static MACHINE_DRIVER_START( jantouki )

	/* driver data */
	MDRV_DRIVER_DATA(dynax_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu",Z80,22000000 / 4)	/* 5.5MHz */
	MDRV_CPU_PROGRAM_MAP(jantouki_mem_map)
	MDRV_CPU_IO_MAP(jantouki_io_map)
	MDRV_CPU_VBLANK_INT("top", jantouki_vblank_interrupt)	/* IM 0 needs an opcode on the data bus */

	MDRV_CPU_ADD("soundcpu",Z80,22000000 / 4)	/* 5.5MHz */
	MDRV_CPU_PROGRAM_MAP(jantouki_sound_mem_map)
	MDRV_CPU_IO_MAP(jantouki_sound_io_map)
	MDRV_CPU_VBLANK_INT("top", jantouki_sound_vblank_interrupt)	/* IM 0 needs an opcode on the data bus */

	MDRV_MACHINE_START(jantouki)
	MDRV_MACHINE_RESET(dynax)

	MDRV_NVRAM_HANDLER(generic_0fill)

	/* video hardware */
	MDRV_PALETTE_LENGTH(512)
	MDRV_DEFAULT_LAYOUT(layout_dualhuov)

	MDRV_SCREEN_ADD("top", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_SIZE(512, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 512-1, 16, 256-1)

	MDRV_SCREEN_ADD("bottom", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_SIZE(512, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 512-1, 16, 256-1)

	MDRV_PALETTE_INIT(sprtmtch)			// static palette
	MDRV_VIDEO_START(jantouki)
	MDRV_VIDEO_UPDATE(jantouki)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("aysnd", AY8910, 22000000 / 8)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.20)

	MDRV_SOUND_ADD("ymsnd", YM2203, 22000000 / 8)
	MDRV_SOUND_CONFIG(jantouki_ym2203_interface)
	MDRV_SOUND_ROUTE(0, "mono", 0.20)
	MDRV_SOUND_ROUTE(1, "mono", 0.20)
	MDRV_SOUND_ROUTE(2, "mono", 0.20)
	MDRV_SOUND_ROUTE(3, "mono", 0.50)

	MDRV_SOUND_ADD("msm", MSM5205, 384000)
	MDRV_SOUND_CONFIG(jantouki_msm5205_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	/* devices */
	MDRV_MSM6242_ADD("rtc")
MACHINE_DRIVER_END


/***************************************************************************
                            Mahjong Electron Base
***************************************************************************/

/*  It runs in IM 2, thus needs a vector on the data bus:
    0xfa and 0xfc are very similar, they should be triggered by the blitter
    0xf8 is vblank  */
void mjelctrn_update_irq( running_machine *machine )
{
	dynax_state *state = (dynax_state *)machine->driver_data;
	state->blitter_irq = 1;
	cpu_set_input_line_and_vector(state->maincpu, 0, HOLD_LINE, 0xfa);
}

static INTERRUPT_GEN( mjelctrn_vblank_interrupt )
{
	dynax_state *state = (dynax_state *)device->machine->driver_data;

	// This is a kludge to avoid losing blitter interrupts
	// there should be a vblank ack mechanism
	if (!state->blitter_irq)
		cpu_set_input_line_and_vector(device, 0, HOLD_LINE, 0xf8);
}

static MACHINE_DRIVER_START( mjelctrn )

	MDRV_IMPORT_FROM( hnoridur )
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(nanajign_mem_map)
	MDRV_CPU_IO_MAP(mjelctrn_io_map)
	MDRV_CPU_VBLANK_INT("screen", mjelctrn_vblank_interrupt)	/* IM 2 needs a vector on the data bus */

	MDRV_VIDEO_START(mjelctrn)
MACHINE_DRIVER_END


/***************************************************************************
                                    Neruton
***************************************************************************/

/*  It runs in IM 2, thus needs a vector on the data bus:
    0x42 and 0x44 are very similar, they should be triggered by the blitter
    0x40 is vblank
    0x46 is a periodic irq? */
void neruton_update_irq( running_machine *machine )
{
	dynax_state *state = (dynax_state *)machine->driver_data;
	state->blitter_irq = 1;
	cpu_set_input_line_and_vector(state->maincpu, 0, HOLD_LINE, 0x42);
}

static INTERRUPT_GEN( neruton_vblank_interrupt )
{
	dynax_state *state = (dynax_state *)device->machine->driver_data;

	// This is a kludge to avoid losing blitter interrupts
	// there should be a vblank ack mechanism
	if (state->blitter_irq)	return;

	switch (cpu_getiloops(device))
	{
		case 0:  cpu_set_input_line_and_vector(device, 0, HOLD_LINE, 0x40);	break;
		default: cpu_set_input_line_and_vector(device, 0, HOLD_LINE, 0x46);	break;
	}
}

static MACHINE_DRIVER_START( neruton )

	MDRV_IMPORT_FROM( mjelctrn )
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_VBLANK_INT_HACK(neruton_vblank_interrupt,1+10)	/* IM 2 needs a vector on the data bus */

	MDRV_VIDEO_START(neruton)
MACHINE_DRIVER_END

/***************************************************************************
                                    Mahjong X-Tal 7
***************************************************************************/

/*  It runs in IM 2, thus needs a vector on the data bus:
    0x42 and 0x44 are very similar, they should be triggered by the blitter
    0x40 is vblank  */
static INTERRUPT_GEN( majxtal7_vblank_interrupt )
{
	dynax_state *state = (dynax_state *)device->machine->driver_data;

	// This is a kludge to avoid losing blitter interrupts
	// there should be a vblank ack mechanism
	if (state->blitter_irq)	return;

	cpu_set_input_line_and_vector(device, 0, HOLD_LINE, 0x40);
}

static MACHINE_DRIVER_START( majxtal7 )

	MDRV_IMPORT_FROM( neruton )
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_VBLANK_INT("screen", majxtal7_vblank_interrupt)	/* IM 2 needs a vector on the data bus */

MACHINE_DRIVER_END


/***************************************************************************
                           Hanafuda Hana Tengoku
***************************************************************************/

static const ay8910_interface htengoku_ay8910_interface =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	// A            B
	DEVCB_HANDLER(htengoku_dsw_r),	DEVCB_NULL,							// R
	DEVCB_NULL,						DEVCB_HANDLER(htengoku_dsw_w)		// W
};

static MACHINE_DRIVER_START( htengoku )

	/* driver data */
	MDRV_DRIVER_DATA(dynax_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu",Z80,20000000 / 4)
	MDRV_CPU_PROGRAM_MAP(yarunara_mem_map)
	MDRV_CPU_IO_MAP(htengoku_io_map)
	MDRV_CPU_VBLANK_INT("screen", sprtmtch_vblank_interrupt)	/* IM 0 needs an opcode on the data bus */
	MDRV_CPU_PERIODIC_INT(yarunara_clock_interrupt, 60)	// RTC

	MDRV_MACHINE_START(htengoku)
	MDRV_MACHINE_RESET(dynax)

	MDRV_NVRAM_HANDLER(generic_0fill)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(512, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 336-1, 0+8, 256-1-8)

	MDRV_PALETTE_LENGTH(16*256)

	MDRV_VIDEO_ATTRIBUTES(VIDEO_ALWAYS_UPDATE)
	MDRV_VIDEO_START(htengoku)
	MDRV_VIDEO_UPDATE(htengoku)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("aysnd", AY8910, 20000000 / 16)
	MDRV_SOUND_CONFIG(htengoku_ay8910_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.20)

	MDRV_SOUND_ADD("ymsnd", YM2413, 3579545)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	/* devices */
	MDRV_MSM6242_ADD("rtc")
MACHINE_DRIVER_END


/***************************************************************************
                               Mahjong Tenkaigen
***************************************************************************/

static INTERRUPT_GEN( tenkai_interrupt )
{
	switch (cpu_getiloops(device))
	{
		case 0: cpu_set_input_line(device, INPUT_LINE_IRQ0, HOLD_LINE);	break;
		case 1: cpu_set_input_line(device, INPUT_LINE_IRQ1, HOLD_LINE);	break;
		case 2: cpu_set_input_line(device, INPUT_LINE_IRQ2, HOLD_LINE);	break;
	}
}

static const ay8910_interface tenkai_ay8910_interface =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	// A                // B
	DEVCB_HANDLER(tenkai_dsw_r),	DEVCB_NULL,						// Read
	DEVCB_NULL,						DEVCB_HANDLER(tenkai_dswsel_w)	// Write
};

static STATE_POSTLOAD( tenkai_bank_postload )
{
	tenkai_update_rombank(machine);
}

static MACHINE_START( tenkai )
{
	MACHINE_START_CALL(dynax);

	state_save_register_postload(machine, tenkai_bank_postload, NULL);
}

static MACHINE_DRIVER_START( tenkai )

	/* driver data */
	MDRV_DRIVER_DATA(dynax_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu",TMP91640, 21472700 / 2)
	MDRV_CPU_PROGRAM_MAP(tenkai_map)
	MDRV_CPU_IO_MAP(tenkai_io_map)
	MDRV_CPU_VBLANK_INT_HACK(tenkai_interrupt,3)

	MDRV_MACHINE_START(tenkai)
	MDRV_MACHINE_RESET(dynax)

	MDRV_NVRAM_HANDLER(generic_0fill)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(512, 256)
	MDRV_SCREEN_VISIBLE_AREA(4, 512-1, 4, 255-8-4)	// hide first 4 horizontal pixels (see scroll of gal 4 in test mode)

	MDRV_PALETTE_LENGTH(16*256)

	MDRV_VIDEO_START(mjelctrn)
	MDRV_VIDEO_UPDATE(hnoridur)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("aysnd", AY8910, 22000000 / 16)
	MDRV_SOUND_CONFIG(tenkai_ay8910_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.20)

	MDRV_SOUND_ADD("ymsnd", YM2413, 3579545)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	/* devices */
	MDRV_MSM6242_ADD("rtc")
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( majrjhdx )
	MDRV_IMPORT_FROM( tenkai )
	MDRV_PALETTE_LENGTH(512)
	MDRV_PALETTE_INIT(sprtmtch)			// static palette
MACHINE_DRIVER_END

/***************************************************************************
                                Mahjong Gekisha
***************************************************************************/

static STATE_POSTLOAD( gekisha_bank_postload )
{
	dynax_state *state = (dynax_state *)machine->driver_data;

	gekisha_set_rombank(machine, state->rombank);
}

static MACHINE_START( gekisha )
{
	MACHINE_START_CALL(dynax);

	state_save_register_postload(machine, gekisha_bank_postload, NULL);
}

static MACHINE_RESET( gekisha )
{
	MACHINE_RESET_CALL(dynax);

	gekisha_set_rombank(machine, 0);
}

static MACHINE_DRIVER_START( gekisha )

	/* driver data */
	MDRV_DRIVER_DATA(dynax_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu",TMP90841, XTAL_10MHz )	// ?
	MDRV_CPU_PROGRAM_MAP(gekisha_map)
	MDRV_CPU_IO_MAP(gekisha_io_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_MACHINE_START(gekisha)
	MDRV_MACHINE_RESET(gekisha)

	MDRV_NVRAM_HANDLER(generic_0fill)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(2, 256-1, 16, 256-1)

	MDRV_PALETTE_LENGTH(512)
	MDRV_PALETTE_INIT(sprtmtch)			// static palette
	MDRV_VIDEO_START(mjdialq2)
	MDRV_VIDEO_UPDATE(mjdialq2)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("aysnd", AY8910, XTAL_24MHz / 16)	// ?
	MDRV_SOUND_CONFIG(tenkai_ay8910_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.20)

	MDRV_SOUND_ADD("ymsnd", YM2413, XTAL_24MHz / 8)	// ?
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


/***************************************************************************


                                ROMs Loading


***************************************************************************/


/***************************************************************************

Hana no Mai
(c)1988 Dynax

D1610088L1

CPU:    Z80-A
Sound:  AY-3-8912A
    YM2203C
    M5205
OSC:    22.000MHz
Custom: (TC17G032AP-0246)

***************************************************************************/

ROM_START( hanamai )
	ROM_REGION( 0x90000, "maincpu", 0 )	// Z80 Code
	ROM_LOAD( "1611.13a", 0x00000, 0x10000, CRC(5ca0b073) SHA1(56b64077e7967fdbb87a7685ca9662cc7881b5ec) )
	ROM_LOAD( "1610.14a", 0x48000, 0x10000, CRC(b20024aa) SHA1(bb6ce9821c1edbf7d4cfadc58a2b257755856937) )

	ROM_REGION( 0xf0000, "gfx1", 0 )	// blitter data
	ROM_LOAD( "1604.12e", 0x000000, 0x20000, CRC(3b8362f7) SHA1(166ce1fe5c48b02b728cf9b90f3dfae4461a3e2c) )
	ROM_LOAD( "1609.12c", 0x020000, 0x20000, CRC(91c5d211) SHA1(343ee4273e8075ba9c2f545c1ee2e403623e3185) )
	ROM_LOAD( "1603.13e", 0x040000, 0x20000, CRC(16a2a680) SHA1(7cd5de9a36fd05261d23f0a0e90d871b132368f0) )
	ROM_LOAD( "1608.13c", 0x060000, 0x20000, CRC(793dd4f8) SHA1(57c32fae553ba9d37c2ffdbfa96ede113f7bcccb) )
	ROM_LOAD( "1602.14e", 0x080000, 0x20000, CRC(3189a45f) SHA1(06e8cb1b6dd6d7e00a7270d4c76b894aba2e7734) )
	ROM_LOAD( "1607.14c", 0x0a0000, 0x20000, CRC(a58edfd0) SHA1(8112b0e0bf8c5bdd1d2e3338d23fe36cf3972a43) )
	ROM_LOAD( "1601.15e", 0x0c0000, 0x20000, CRC(84ff07af) SHA1(d7259056c4e09171aa8b9342ebaf3b8a3490613a) )
	ROM_LOAD( "1606.15c", 0x0e0000, 0x10000, CRC(ce7146c1) SHA1(dc2e202a67d1618538eb04248c1b2c7d7f62151e) )

	ROM_REGION( 0x30000, "gfx2", 0 )	// blitter data
	ROM_LOAD( "1605.10e", 0x00000, 0x10000, CRC(0f4fd9e4) SHA1(8bdc8b46bf4dafead25a5adaebb74d547386ce23) )
	ROM_LOAD( "1612.10c", 0x20000, 0x10000, CRC(8d9fb6e1) SHA1(2763f73069147d62fd46bb961b64cc9598687a28) )

	ROM_REGION( 0x400, "proms", 0 )	// Color PROMs
	ROM_LOAD( "2.3j",  0x000, 0x200, CRC(7b0618a5) SHA1(df3aadcc7d54fab0c07f85d20c138a45798644e4) )	// FIXED BITS (0xxxxxxx)
	ROM_LOAD( "1.4j",  0x200, 0x200, CRC(9cfcdd2d) SHA1(a649e9381754c4a19ccecc6e558067cc3ff27f91) )
ROM_END


/***************************************************************************

Hana Kochou (Hana no Mai BET ver.)
(c)1988 Dynax

D201901L2
D201901L1-0

CPU:    Z80-A
Sound:  AY-3-8912A
    YM2203C
    M5205
OSC:    22.000MHz
VDP:    HD46505SP
Custom: (TC17G032AP-0246)

***************************************************************************/

ROM_START( hnkochou )
	ROM_REGION( 0x90000, "maincpu", 0 )	// Z80 Code
	ROM_LOAD( "2009.s4a", 0x00000, 0x10000, CRC(b3657123) SHA1(3385edb2055abc7be3abb030509c6ac71907a5f3) )
	ROM_LOAD( "2008.s3a", 0x18000, 0x10000, CRC(1c009be0) SHA1(0f950d2685f8b67f37065e19deae0cf0cb9594f1) )

	ROM_REGION( 0x0e0000, "gfx1", 0 )	// blitter data
	ROM_LOAD( "2004.12e", 0x000000, 0x20000, CRC(178aa996) SHA1(0bc8b7c093ed46a91c5781c03ae707d3dafaeef4) )
	ROM_LOAD( "2005.12c", 0x020000, 0x20000, CRC(ca57cac5) SHA1(7a55e1cb5cee5a38c67199f589e0c7ae5cd907a0) )
	ROM_LOAD( "2003.13e", 0x040000, 0x20000, CRC(092edf8d) SHA1(3d030462f96edbb0fa4efcc2a5302c17661dce54) )
	ROM_LOAD( "2006.13c", 0x060000, 0x20000, CRC(610c22ec) SHA1(af47bf94e01d1a83aa2a7195c906e13038057c35) )
	ROM_LOAD( "2002.14e", 0x080000, 0x20000, CRC(759b717d) SHA1(fd719fd792459789b559a1e99173144322605b8e) )
	ROM_LOAD( "2007.14c", 0x0a0000, 0x20000, CRC(d0f22355) SHA1(7b027930624ff1f883d620a8e78f962e821f4b23) )
	ROM_LOAD( "2001.15e", 0x0c0000, 0x20000, CRC(09ace2b5) SHA1(1756e3a52523557aa481c6bd6cdf168567af82ff) )

	ROM_REGION( 0x400, "proms", 0 )	// Color PROMs
	ROM_LOAD( "2.3j",  0x000, 0x200, CRC(7b0618a5) SHA1(df3aadcc7d54fab0c07f85d20c138a45798644e4) )	// FIXED BITS (0xxxxxxx)
	ROM_LOAD( "1.4j",  0x200, 0x200, CRC(9cfcdd2d) SHA1(a649e9381754c4a19ccecc6e558067cc3ff27f91) )
ROM_END



/***************************************************************************

Hana Oriduru
(c)1989 Dynax
D2304268L

CPU  : Z80B
Sound: AY-3-8912A YM2413 M5205
OSC  : 22MHz (X1, near main CPU), 384KHz (X2, near M5205)
       3.58MHz (X3, Sound section)

CRT Controller: HD46505SP (6845)
Custom chip: DYNAX TC17G032AP-0246 JAPAN 8929EAI

***************************************************************************/

ROM_START( hnoridur )
	ROM_REGION( 0x10000 + 0x19*0x8000, "maincpu", 0 )	// Z80 Code
	ROM_LOAD( "2309.12",  0x00000, 0x20000, CRC(5517dd68) SHA1(3da27032a412b51b67e852b61166c2fdc138a370) )
	ROM_RELOAD(           0x10000, 0x20000 )

	ROM_REGION( 0x100000, "gfx1", 0 )	// blitter data
	ROM_LOAD( "2302.21",  0x000000, 0x20000, CRC(9dde2d59) SHA1(96df4ba97ee9611d9a3c7bcaae9cd97815a7b8a5) )
	ROM_LOAD( "2303.22",  0x020000, 0x20000, CRC(1ac59443) SHA1(e70fe6184e7090cf7229d83b87db65f7715de2a8) )
	ROM_LOAD( "2301.20",  0x040000, 0x20000, CRC(24391ddc) SHA1(6a2e3fae4b6d0b1d8073306f37c9fdaa04b69eb8) )
	ROM_LOAD( "2304.1",   0x060000, 0x20000, CRC(3756c745) SHA1(f275a72d6e07f21148900d24a8018d03504f249f) )
	ROM_LOAD( "2305.2",   0x080000, 0x20000, CRC(249d360a) SHA1(688fced1298c345a18314d2c88664c757a2de35c) )
	ROM_LOAD( "2306.3",   0x0a0000, 0x20000, CRC(014a4945) SHA1(0cd747787a81226fd4937616a6ce45af731a4049) )
	ROM_LOAD( "2307.4",   0x0c0000, 0x20000, CRC(8b6f8a2d) SHA1(c5f3ec64a7ea3edc556182f42e6da4842d88e0ba) )
	ROM_LOAD( "2308.5",   0x0e0000, 0x20000, CRC(6f996e6e) SHA1(c2b916afbfd257417f0383ad261f3720a027fdd9) )
ROM_END



/***************************************************************************

Sports Match
Dynax 1989

                     5563
                     3101
        SW2 SW1
                             3103
         YM2203              3102
                     16V8
                     Z80         DYNAX
         22MHz

           6845
                         53462
      17G                53462
      18G                53462
                         53462
                         53462
                         53462

***************************************************************************/

ROM_START( drgpunch )
	ROM_REGION( 0x90000, "maincpu", 0 )	// Z80 Code
	ROM_LOAD( "2401.3d", 0x00000, 0x10000, CRC(b310709c) SHA1(6ad6cfb54856f65a888ac44e694890f32f26e049) )
	ROM_LOAD( "2402.6d", 0x28000, 0x10000, CRC(d21ed237) SHA1(7e1c7b40c300578132ebd79cbad9f7976cc85947) )

	ROM_REGION( 0xc0000, "gfx1", 0 )	// blitter data
	ROM_LOAD( "2403.6c", 0x00000, 0x20000, CRC(b936f202) SHA1(4920d29a814ebdd74ce6f780cf821c8cb8142d9f) )
	ROM_LOAD( "2404.5c", 0x20000, 0x20000, CRC(2ee0683a) SHA1(e29225e08be11f6971fa04ce2715be914d29976b) )
	ROM_LOAD( "2405.3c", 0x40000, 0x20000, CRC(aefbe192) SHA1(9ed0ec7d6357eedec80a90364f196e43a5bfee03) )
	ROM_LOAD( "2406.1c", 0x60000, 0x20000, CRC(e137f96e) SHA1(c652f3cb17a56127d30a516af75154e15d7ce6c0) )
	ROM_LOAD( "2407.6a", 0x80000, 0x20000, CRC(f3f1b065) SHA1(531317e4d1ab5db60595ca3327234a6bdea79ce9) )
	ROM_LOAD( "2408.5a", 0xa0000, 0x20000, CRC(3a91e2b9) SHA1(b762c38ff2ebbd4ed832ca772973a15dd4a4ad73) )

	ROM_REGION( 0x400, "proms", 0 )	// Color PROMs
	ROM_LOAD( "2.18g", 0x000, 0x200, CRC(9adccc33) SHA1(acf4d5a28430378dbccc1b9fa0b6391cc8149fee) )	// FIXED BITS (0xxxxxxx)
	ROM_LOAD( "1.17g", 0x200, 0x200, CRC(324fa9cf) SHA1(a03e23d9a9687dec4c23a8e41254a3f4b70c7e25) )
ROM_END

ROM_START( sprtmtch )
	ROM_REGION( 0x90000, "maincpu", 0 )	// Z80 Code
	ROM_LOAD( "3101.3d", 0x00000, 0x08000, CRC(d8fa9638) SHA1(9851d38b6b3f56cf3cc101419c24f8d5f97950a9) )
	ROM_CONTINUE(        0x28000, 0x08000 )

	ROM_REGION( 0x40000, "gfx1", 0 )	// blitter data
	ROM_LOAD( "3102.6c", 0x00000, 0x20000, CRC(46f90e59) SHA1(be4411c3cfa8c8ab26eba935289df0f0fd545b62) )
	ROM_LOAD( "3103.5c", 0x20000, 0x20000, CRC(ad29d7bd) SHA1(09ab84164e5cd14b595f33d129863735901aa922) )

	ROM_REGION( 0x400, "proms", 0 )	// Color PROMs
	ROM_LOAD( "18g", 0x000, 0x200, CRC(dcc4e0dd) SHA1(4e0fb8fd7192bf32247966742df4b80585f32c37) )	// FIXED BITS (0xxxxxxx)
	ROM_LOAD( "17g", 0x200, 0x200, CRC(5443ebfb) SHA1(5b63220a3f6520e353db99b06e645640d1cfde2f) )
ROM_END


/***************************************************************************

Mahjong Friday
(c)1989 Dynax
D2607198L1

CPU  : Zilog Z0840006PSC (Z80)
Sound: YM2413 M5205
OSC  : 24MHz (X1)

CRT Controller: HD46505SP (6845)
Custom chip: DYNAX TC17G032AP-0246 JAPAN 8828EAI

***************************************************************************/

ROM_START( mjfriday )
	ROM_REGION( 0x90000, "maincpu", 0 )	// Z80 Code
	ROM_LOAD( "2606.2b",  0x00000, 0x10000, CRC(00e0e0d3) SHA1(89fa4d684ec36d5e974e39294efd65a9fd832517) )
	ROM_LOAD( "2605.2c",  0x28000, 0x10000, CRC(5459ebda) SHA1(86e51f0c120de87be8f51b498a562360e6b242b8) )

	ROM_REGION( 0x80000, "gfx1", 0 )	// blitter data
	ROM_LOAD( "2601.2h",  0x000000, 0x20000, CRC(70a01fc7) SHA1(0ed2f7c258f3cd82229bea7514d262fca57bd925) )
	ROM_LOAD( "2602.2g",  0x020000, 0x20000, CRC(d9167c10) SHA1(0fa34a065b3ffd5d35d03275bdcdf753045d6491) )
	ROM_LOAD( "2603.2f",  0x040000, 0x20000, CRC(11892916) SHA1(0680ab77fc1a2cdb78637bf0c506f03ca514014b) )
	ROM_LOAD( "2604.2e",  0x060000, 0x20000, CRC(3cc1a65d) SHA1(221dc17042e46e58dc4634eef798568747aef3a2) )

	ROM_REGION( 0x400, "proms", 0 )	// Color PROMs
	ROM_LOAD( "d26_2.9e", 0x000, 0x200, CRC(d6db5c60) SHA1(89ee10d092011c2c4eaab2c097aa88f5bb98bb97) )	// FIXED BITS (0xxxxxxx)
	ROM_LOAD( "d26_1.8e", 0x200, 0x200, CRC(af5edf32) SHA1(7202e0aa1ee3f22e3c5fb69a88db455a241929c5) )
ROM_END


/***************************************************************************

Maya
Promat, 1994

PCB Layout
----------

    6845      6264   3
 DSW1  DSW2    1     4
   YM2203      2     5
   3014B

              Z80
  22.1184MHz

 PROM1  TPC1020  D41264
 PROM2            (x6)


Notes:
      Z80 Clock: 5.522MHz
          HSync: 15.925 kHz
          VSync: 60Hz


***************************************************************************/

ROM_START( maya )
	ROM_REGION( 0x90000, "maincpu", 0 )	// Z80 Code
	ROM_LOAD( "1.17e", 0x00000, 0x10000, CRC(5aaa015e) SHA1(b84d02b1b6c07636176f226fef09a034d00445f0) )
	ROM_LOAD( "2.15e", 0x28000, 0x10000, CRC(7ea5b49a) SHA1(aaae848669d9f88c0660f46cc801e4eb0f5e3b89) )

	ROM_REGION( 0xc0000, "gfx1", 0 )
	ROM_LOAD( "3.18g", 0x00000, 0x40000, CRC(8534af04) SHA1(b9bc94541776b5c0c6bf0ecc63ffef914756376e) )
	ROM_LOAD( "4.17g", 0x40000, 0x40000, CRC(ab85ce5e) SHA1(845b846e0fb8c9fcd1540960cda006fdac364fea) )
	ROM_LOAD( "5.15g", 0x80000, 0x40000, CRC(c4316dec) SHA1(2e727a491a71eb1f4d9f338cc6ec76e03f7b46fd) )

	ROM_REGION( 0x400, "proms", 0 )	// Color PROMs
	ROM_LOAD( "prom2.5b",  0x000, 0x200, CRC(d276bf61) SHA1(987058b37182a54a360a80a2f073b000606a11c9) )	// FIXED BITS (0xxxxxxx)
	ROM_LOAD( "prom1.6b",  0x200, 0x200, CRC(e38eb360) SHA1(739960dd57ec3305edd57aa63816a81ddfbebf3e) )
ROM_END

ROM_START( mayaa )
	ROM_REGION( 0x90000, "maincpu", 0 )	// Z80 Code
	ROM_LOAD( "512-1.bin", 0x00000, 0x10000, CRC(8ac94f49) SHA1(3c1e86c1aad67fb8cb1eb534a272222b58f1ff0f) )
	ROM_LOAD( "512-2.bin", 0x28000, 0x10000, CRC(7ea5b49a) SHA1(aaae848669d9f88c0660f46cc801e4eb0f5e3b89) )

	ROM_REGION( 0xc0000, "gfx1", 0 )
	ROM_LOAD( "27c020-1.bin", 0x00000, 0x40000, CRC(8d41d7ca) SHA1(7e28845457d00481b313ee52b8c7674f75b8c3c0) )
	ROM_LOAD( "27c020-2.bin", 0x40000, 0x40000, CRC(ab85ce5e) SHA1(845b846e0fb8c9fcd1540960cda006fdac364fea) )
	ROM_LOAD( "27c020-3.bin", 0x80000, 0x40000, CRC(c4316dec) SHA1(2e727a491a71eb1f4d9f338cc6ec76e03f7b46fd) )

	ROM_REGION( 0x400, "proms", 0 )	// Color PROMs
	ROM_LOAD( "promat01.bin",  0x000, 0x200, CRC(d276bf61) SHA1(987058b37182a54a360a80a2f073b000606a11c9) )	// FIXED BITS (0xxxxxxx)
	ROM_LOAD( "promat02.bin",  0x200, 0x200, CRC(e38eb360) SHA1(739960dd57ec3305edd57aa63816a81ddfbebf3e) )
ROM_END

ROM_START( inca )
	ROM_REGION( 0x90000, "maincpu", 0 )	// Z80 Code
	ROM_LOAD( "am27c512.1", 0x00000, 0x10000, CRC(b0d513f7) SHA1(65ef4702302bbfc7c7a77f7353120ee3f5c94b31) )
	ROM_LOAD( "2.15e", 0x28000, 0x10000, CRC(7ea5b49a) SHA1(aaae848669d9f88c0660f46cc801e4eb0f5e3b89) )

	ROM_REGION( 0xc0000, "gfx1", 0 )
	ROM_LOAD( "m27c2001.3", 0x00000, 0x40000, CRC(3a3c54ea) SHA1(2743c6a66d3eff60080c9183fa2bf9081207ac6b) )
	ROM_LOAD( "am27c020.4", 0x40000, 0x40000, CRC(d3571d63) SHA1(5f0abb0da19af34bbd3eb93270311e824839deb4) )
	ROM_LOAD( "m27c2001.5", 0x80000, 0x40000, CRC(bde60c29) SHA1(3ff7fbd5978bec27ff2ecf5977f640c66058e45d) )

	ROM_REGION( 0x400, "proms", 0 )	// Color PROMs
	ROM_LOAD( "n82s147n.2",  0x000, 0x200, CRC(268bd9d3) SHA1(1f77d9dc58ab29f013ee21d7ec521b90be72610d) )	// FIXED BITS (0xxxxxxx)
	ROM_LOAD( "n82s147n.1",  0x200, 0x200, CRC(618dbeb3) SHA1(10c8a558430fd1c2cabf9133d3e4f0a5f80eab83) )
ROM_END



/*

Black Touch


PCB Layout
----------

YANG GI CO LTD
432605-0003
|----------------------------------------|
|      DSW1    DSW2   DIP40              |
|      YM2203                            |
|                 41264  41264           |
| 82S147          41264  41264           |
| 82S147          41264  41264           |
|       22.1184MHz                       |
|J        Z80                            |
|A        10D                            |
|M        8D           DIP40             |
|M        6264                           |
|A                                       |
|         GM68B45S                       |
|                                        |
|                                        |
|                                        |
|                                        |
|                        G1   H1    I1   |
|----------------------------------------|
Notes:
      Z80 clock 5.5296MHz [22.1184/4]
      YM2203 clock 2.7648MHz [22.1184/8]
      DIP40 - surface scratched, unknown

 */

ROM_START( blktouch )
	ROM_REGION( 0x90000, "maincpu", 0 )	// Z80 Code
	ROM_LOAD( "u43_d8",  0x00000, 0x10000, CRC(0972ab8c) SHA1(ff751fbb37562f216a4fddebd9190acee1f357c8) )
	ROM_LOAD( "u45_d10", 0x28000, 0x10000, CRC(5745424a) SHA1(d244d9a9b4c49d255f114842147ba0a795a3e9ac) )

	ROM_REGION( 0xc0000, "gfx1", 0 )
	ROM_LOAD( "u33_g1", 0x80000, 0x40000, CRC(88737a8b) SHA1(9d812ee732f6cc43c78d8585dabf1b51ed4b89ba) )
	ROM_LOAD( "u34_h1", 0x40000, 0x40000, CRC(07216e11) SHA1(402b201c665503a2c9bb1b2f74da0c3db5c3f660) )
	ROM_LOAD( "u35_i1", 0x00000, 0x40000, CRC(4ae52ccb) SHA1(84c9466e6f574ec99947084a2e8a336935ad4186) )

	ROM_REGION( 0x400, "proms", 0 )	// Color PROMs
	ROM_LOAD( "u12",  0x200, 0x200, CRC(80d282ff) SHA1(3ad8dc61fe098ceac3a4be8742c1b360dfa7223a) )
	ROM_LOAD( "u13",  0x000, 0x200, CRC(6984aaa9) SHA1(91645cd944cb21266edd13e55a8dc846f6edc419) )
ROM_END

static DRIVER_INIT( blktouch )
{
	// fearsome encryption ;-)
	UINT8	*src = (UINT8 *)memory_region(machine, "maincpu");
	int i;

	for (i = 0; i < 0x90000; i++)
	{
		src[i] = BITSWAP8(src[i], 7, 6, 5, 3, 4, 2, 1, 0);

	}

	src = (UINT8 *)memory_region(machine, "gfx1");

	for (i = 0; i < 0xc0000; i++)
	{
		src[i] = BITSWAP8(src[i], 7, 6, 5, 3, 4, 2, 1, 0);

	}
}

static DRIVER_INIT( maya )
{
	/* Address lines scrambling on 1 z80 rom */
	int i;
	UINT8	*gfx = (UINT8 *)memory_region(machine, "gfx1");
	UINT8	*rom = memory_region(machine, "maincpu") + 0x28000, *end = rom + 0x10000;
	for ( ; rom < end; rom += 8)
	{
		UINT8 temp[8];
		temp[0] = rom[0];	temp[1] = rom[1];	temp[2] = rom[2];	temp[3] = rom[3];
		temp[4] = rom[4];	temp[5] = rom[5];	temp[6] = rom[6];	temp[7] = rom[7];

		rom[0] = temp[0];	rom[1] = temp[4];	rom[2] = temp[1];	rom[3] = temp[5];
		rom[4] = temp[2];	rom[5] = temp[6];	rom[6] = temp[3];	rom[7] = temp[7];
	}

	/* Address lines scrambling on the blitter data roms */
	rom = auto_alloc_array(machine, UINT8, 0xc0000);
	memcpy(rom, gfx, 0xc0000);
	for (i = 0; i < 0xc0000; i++)
		gfx[i] = rom[BITSWAP24(i,23,22,21,20,19,18,14,15, 16,17,13,12,11,10,9,8, 7,6,5,4,3,2,1,0)];
	auto_free(machine, rom);
}


/***************************************************************************

Mahjong Dial Q2
(c)1991 Dynax
D5212298L-1

CPU  : Z80
Sound: YM2413
OSC  : (240-100 624R001 KSSOB)?
Other: TC17G032AP-0246
CRT Controller: HD46505SP (6845)

***************************************************************************/

ROM_START( mjdialq2 )
	ROM_REGION( 0x78000, "maincpu", 0 )	// Z80 Code
	ROM_LOAD( "5201.2b", 0x00000, 0x10000, CRC(5186c2df) SHA1(f05ae3fd5e6c39f3bf2263eaba645d89c454bd70) )
	ROM_RELOAD(          0x10000, 0x08000 )				// 1
	ROM_CONTINUE(        0x20000, 0x08000 )				// 3
	ROM_LOAD( "5202.2c", 0x30000, 0x08000, CRC(8e8b0038) SHA1(44130bb29b569610826e1fc7e4b2822f0e1034b1) )	// 5
	ROM_CONTINUE(        0x70000, 0x08000 )				// d

	ROM_REGION( 0xa0000, "gfx1", 0 )	// blitter data
	ROM_LOAD( "5207.2h", 0x00000, 0x20000, CRC(7794cd62) SHA1(7fa2fd50d7c975c381dda36f505df0e152196bb5) )
	ROM_LOAD( "5206.2g", 0x20000, 0x20000, CRC(9e810021) SHA1(cf1052c96b9da3abb263be1ce8481aeded2c5d00) )
	ROM_LOAD( "5205.2f", 0x40000, 0x20000, CRC(8c05572f) SHA1(544a5eb8b989fb1195986ed856da04350941ef59) )
	ROM_LOAD( "5204.2e", 0x60000, 0x20000, CRC(958ef9ab) SHA1(ec768c587dc9e6b691564b6b35abbece252bcd28) )
	ROM_LOAD( "5203.2d", 0x80000, 0x20000, CRC(706072d7) SHA1(d4692296d234b824961a94390e6d646ed9a7d5fd) )

	ROM_REGION( 0x400, "proms", 0 )	// Color PROMs
	ROM_LOAD( "d52-2.9e", 0x000, 0x200, CRC(18585ce3) SHA1(7f2e20bb09c1d810910094a6b19e5151666d74ac) )	// FIXED BITS (0xxxxxxx)
	ROM_LOAD( "d52-1.8e", 0x200, 0x200, CRC(8868247a) SHA1(97652025c411b379dfab576dc7f2d8d0d61d0828) )
ROM_END


/***************************************************************************

Mahjong Yarunara
(c)1991 Dynax
D5512068L1-1
D4508308L-2 (sub board)

CPU  : Z80B
Sound: AY-3-8912A YM2413 M5205
OSC  : 22.000MHz (near main CPU), 14.31818MHz (Sound section)
       YC-38 (X1), 384KHz (X2, M5205)

ROMs (all ROMs are 541000 = 27C010 compatible):
5501M.2D     [d86fade5]
5502M.4D     [1ef09ff0]
5503M.8C     [9276a10a]
5504M.9C     [6ac42304]
5505M.10C    [b2ca9838]
5506M.11C    [161058fd]
5507M.13C    [7de17b26]
5508M.16C    [ced3155b]
5509M.17C    [ca46ed48]

Subboard ROMs (5515M is 27C040, others are 541000):
5510M.2A     [bb9c71e1]
5511M.3A     [40ee77d8]
5512M.4A     [b4220316]
5513M.1B     [32b7bcbd]
5514M.2B     [ac714bb7]
5515M.4B     [ef130237]

PALs (not dumped):
D55A.4E
D55B.11F
D55C.16N
D55D.17D
D55EH.6A

CRT controller:
HD46505SP (6845)

Real time clock:
OKI M6242B

Custom chip:
DYNAX NL-001 WD10100

***************************************************************************/

ROM_START( yarunara )
	ROM_REGION( 0x10000 + 0x1d*0x8000, "maincpu", 0 )	// Z80 Code
	ROM_LOAD( "5501m.2d",  0x00000, 0x20000, CRC(d86fade5) SHA1(4ae5e22972eb4ead9aa4a455ff1a18e128c33ed6) )
	ROM_RELOAD(            0x10000, 0x20000 )
	ROM_LOAD( "5502m.4d",  0x30000, 0x20000, CRC(1ef09ff0) SHA1(bbedcc1c0f5b43c78e0c3ce0fc1a3c28025562ec) )

	ROM_REGION( 0x80000, "gfx1", 0 )	// blitter data
	ROM_LOAD( "5507m.13c", 0x000000, 0x80000, CRC(7de17b26) SHA1(326667063ab045ac50e850f2f7821a65317879ad) )

	ROM_REGION( 0xc0000, "gfx2", 0 )	// blitter data
	ROM_LOAD( "5508m.16c", 0x00000, 0x20000, CRC(ced3155b) SHA1(658e3947781f1be2ee87b43952999281c66683a6) )
	ROM_LOAD( "5509m.17c", 0x20000, 0x20000, CRC(ca46ed48) SHA1(0769ac0b211181b7b57033f09f72828c885186cc) )
	ROM_LOAD( "5506m.11c", 0x40000, 0x20000, CRC(161058fd) SHA1(cfc21abdc036e874d34bfa3c60486a5ab87cf9cd) )
	ROM_LOAD( "5505m.10c", 0x60000, 0x20000, CRC(b2ca9838) SHA1(7104697802a0466fab40414a467146a224eb6a74) )
	ROM_LOAD( "5504m.9c",  0x80000, 0x20000, CRC(6ac42304) SHA1(ce822da6d61e68578c08c9f1d0af1557c64ac5ae) )
	ROM_LOAD( "5503m.8c",  0xa0000, 0x20000, CRC(9276a10a) SHA1(5a68fff20631a2002509d6cace06b5a9fa0e75d2) )

	ROM_REGION( 0x80000, "gfx3", 0 )	// blitter data
	ROM_LOAD( "5515m.4b",  0x00000, 0x80000, CRC(ef130237) SHA1(2c8f7a15249115b2cdcb3a8e0896ea8601e323d9) )

	ROM_REGION( 0xa0000, "gfx4", 0 )	// blitter data
	ROM_LOAD( "5514m.2b",  0x00000, 0x20000, CRC(ac714bb7) SHA1(64056cbed9d0c4f68611921754c3e6a9bb14f7cc) )
	ROM_LOAD( "5513m.1b",  0x20000, 0x20000, CRC(32b7bcbd) SHA1(13277ae3f158da332e69c6f4f8828dfabbf3ea0a) )
	ROM_LOAD( "5512m.4a",  0x40000, 0x20000, CRC(b4220316) SHA1(b0797c9c6ab226520d29c780ea709f62e02dd268) )
	ROM_LOAD( "5511m.3a",  0x60000, 0x20000, CRC(40ee77d8) SHA1(e0dd9750d8b7b7dd9695a8365bdc926bd6d9f886) )
	ROM_LOAD( "5510m.2a",  0x80000, 0x20000, CRC(bb9c71e1) SHA1(21f2977196aaa27b76ee6547a08aba8da7aba76c) )
ROM_END


/***************************************************************************

Hana wo Yaraneba!
(c)1991 Dynax

D5512068L1-1
CPU: Z80B
Sound: AY38912A/P YM2413 M5205
OSC: 20.0000MHz 14.31818MHz ?(near 6242)
Others: Battery, M6242B
Custom: NL-001
        TC17G032AP-0246

ROMs:
5501H.2D
5502H.4D

5503H.8C
5504H.9C
5505H.10C
5506H.11C
5507H.13C
5508H.16C
5509H.17C

on Subboard D4508308L-2
5510H.2A
5511H.3A
5512H.4A
5513H.1B
5514H.2B
5515H.4B

dumped by sayu
--- Team Japump!!! ---

***************************************************************************/

ROM_START( hanayara )
	ROM_REGION( 0x10000 + 0x1d*0x8000, "maincpu", 0 )	// Z80 Code
	ROM_LOAD( "5501h.2d", 0x00000, 0x20000, CRC(124860b7) SHA1(205459d95f876433a9ca329fe31cfe9b08023baf) )
	ROM_RELOAD(           0x10000, 0x20000 )
	ROM_LOAD( "5502h.4d", 0x30000, 0x20000, CRC(93407801) SHA1(63dc3419f97d86221dbdd67b5be41d713364496b) )

	ROM_REGION( 0xd0000, "gfx1", 0 )	// blitter data
	ROM_LOAD( "5507h.13c", 0x00000, 0x80000, CRC(7de17b26) SHA1(326667063ab045ac50e850f2f7821a65317879ad) )

	ROM_REGION( 0xc0000, "gfx2", 0 )	// blitter data
	ROM_LOAD( "5508h.16c", 0x00000, 0x20000, CRC(ced3155b) SHA1(658e3947781f1be2ee87b43952999281c66683a6) )
	ROM_LOAD( "5509h.17c", 0x20000, 0x20000, CRC(ca46ed48) SHA1(0769ac0b211181b7b57033f09f72828c885186cc) )
	ROM_LOAD( "5506h.11c", 0x40000, 0x20000, CRC(161058fd) SHA1(cfc21abdc036e874d34bfa3c60486a5ab87cf9cd) )
	ROM_LOAD( "5505h.10c", 0x60000, 0x20000, CRC(b2ca9838) SHA1(7104697802a0466fab40414a467146a224eb6a74) )
	ROM_LOAD( "5504h.9c",  0x80000, 0x20000, CRC(6ac42304) SHA1(ce822da6d61e68578c08c9f1d0af1557c64ac5ae) )
	ROM_LOAD( "5503h.8c",  0xa0000, 0x20000, CRC(9276a10a) SHA1(5a68fff20631a2002509d6cace06b5a9fa0e75d2) )

	ROM_REGION( 0x80000, "gfx3", 0 )	// blitter data
	ROM_LOAD( "5515h.4b",  0x00000, 0x80000, CRC(ca742acc) SHA1(0aab6b3bbd0a229a0d6843849704f4faf27c8f72) )

	ROM_REGION( 0xa0000, "gfx4", 0 )	// blitter data
	ROM_LOAD( "5514h.2b",  0x00000, 0x20000, CRC(ac714bb7) SHA1(64056cbed9d0c4f68611921754c3e6a9bb14f7cc) )
	ROM_LOAD( "5513h.1b",  0x20000, 0x20000, CRC(32b7bcbd) SHA1(13277ae3f158da332e69c6f4f8828dfabbf3ea0a) )
	ROM_LOAD( "5512h.4a",  0x40000, 0x20000, CRC(b4220316) SHA1(b0797c9c6ab226520d29c780ea709f62e02dd268) )
	ROM_LOAD( "5511h.3a",  0x60000, 0x20000, CRC(40ee77d8) SHA1(e0dd9750d8b7b7dd9695a8365bdc926bd6d9f886) )
	ROM_LOAD( "5510h.2a",  0x80000, 0x20000, CRC(bb9c71e1) SHA1(21f2977196aaa27b76ee6547a08aba8da7aba76c) )
ROM_END


/***************************************************************************

Quiz TV Gassyuukoku Q&Q (JPN ver.)
(c)1992 Dynax

DX-BASE (Dynax Motherboard System) D5512068L1-2
D6410288L-1 (SUB)

6401.2D   prg. / samples
6402.4D
6403.5D

6404.S2A  chr.
6405.S2B
6406.S2C
6407.S2D
6408.S2E
6409.S2F
6410.S2G
6411.S2H
6412.S3A
6413.S3B

***************************************************************************/

ROM_START( quiztvqq )
	ROM_REGION( 0x10000 + 0x28*0x8000, "maincpu", 0 )	// Z80 Code
	ROM_LOAD( "6401.2d",  0x000000, 0x020000, CRC(ce0e237c) SHA1(fd94a45052e3a68ef8cda2853b911a9993675fa6) )
	// 14-17
	ROM_RELOAD(           0x0b0000, 0x020000 )
	// 04-07
	ROM_LOAD( "6402.4d",  0x030000, 0x020000, CRC(cf7a9aa8) SHA1(9eaa8b318479e82cbcf133c227c61be92d282996) )
	// 24-27
	ROM_CONTINUE(         0x130000, 0x020000 )
	// 08-0b
	ROM_LOAD( "6403.5d",  0x050000, 0x020000, CRC(62402ac9) SHA1(bf52d22b119d54410dad4949b0687bb0edf3e143) )

//  ROM_REGION( 0x00000, "gfx1", 0 )   // blitter data
//  unused

//  ROM_REGION( 0x00000, "gfx2", 0 )   // blitter data
//  unused

	ROM_REGION( 0x100000, "gfx3", 0 )	// blitter data
	ROM_LOAD( "6404.s2a", 0x00000, 0x80000, CRC(996ebe0f) SHA1(6492644aa14b0c2859add31878b5a8d7870981c8) )
	ROM_LOAD( "6405.s2b", 0x80000, 0x20000, CRC(666bfb03) SHA1(e345a198d3e1bc69f22c6f43869ffa2b1501c4ad) )
	ROM_LOAD( "6406.s2c", 0xa0000, 0x20000, CRC(006871ef) SHA1(ebf78b2e46e26d98a7d8952bd29e78c893243c7a) )
	ROM_LOAD( "6407.s2d", 0xc0000, 0x20000, CRC(9cc61541) SHA1(a3c0e06c6ad77cb7b2e86a70c2e27e6a74c35f12) )
	ROM_LOAD( "6408.s2e", 0xe0000, 0x20000, CRC(65a98946) SHA1(4528b300fa3b01d992cf50e87430105463ea3fbd) )

	ROM_REGION( 0xa0000, "gfx4", 0 )	// blitter data
	ROM_LOAD( "6409.s2f", 0x00000, 0x20000, CRC(d5d11061) SHA1(c7ab5aedde6998d62427cc7c4bcf767e9b832a60) )
	ROM_LOAD( "6410.s2g", 0x20000, 0x20000, CRC(bd769d46) SHA1(46f1f9e36f7b5f8deec5f7cce8c0992178ad3be0) )
	ROM_LOAD( "6411.s2h", 0x40000, 0x20000, CRC(7bd43065) SHA1(13b4fcc4155f555ec0c7fbb2f3bb6c19c2788cf5) )
	ROM_LOAD( "6412.s3a", 0x60000, 0x20000, CRC(43e645f3) SHA1(67a2975d4428142a2fbfd1d7b20139a15757bfb4) )
	ROM_LOAD( "6413.s3b", 0x80000, 0x20000, CRC(f7b81238) SHA1(447d983971bed978816dd836504ffcfae0023a69) )
ROM_END


/***************************************************************************

Mahjong Angels (Comic Theater Vol.2)
(c)1991 Dynax

DX-BASE (Dynax Motherboard System) D5512068L1-1
D6107068L-1 (SUB)

612-01.2D   prg. / samples
612-02.4D
612-03.5D

612-04.S1A  chr.
612-05.S2A
612-06.S1B
612-07.S2B
612-08.S3C
612-09.S4C
612-10.S5C
612-11.S6C

***************************************************************************/

ROM_START( mjangels )
	ROM_REGION( 0x10000 + 0x28*0x8000, "maincpu", 0 )	// Z80 Code
	ROM_LOAD( "612-01.2d",    0x000000, 0x020000, CRC(cd353ba9) SHA1(8344dc5dd482ad6d36aa1e6b5824a09a3627dc65) )
	// 00-03
	ROM_RELOAD(               0x010000, 0x20000 )
	// 0c-0f
	ROM_RELOAD(               0x070000, 0x20000 )
	// 24-27
	ROM_RELOAD(               0x130000, 0x20000 )
	// 04-07
	ROM_LOAD( "612-02.4d",    0x030000, 0x020000, CRC(c1be70f9) SHA1(a0bd266b15c1677e5f41411217ca91d25c75e347) )
	// 08-0b
	ROM_LOAD( "612-03.5d",    0x050000, 0x020000, CRC(62402ac9) SHA1(bf52d22b119d54410dad4949b0687bb0edf3e143) )

//  ROM_REGION( 0x00000, "gfx1", 0 )   // blitter data
//  unused

//  ROM_REGION( 0x00000, "gfx2", 0 )   // blitter data
//  unused

	ROM_REGION( 0xc0000, "gfx3", 0 )	// blitter data
	ROM_LOAD( "612-04.s1a",   0x00000, 0x80000, CRC(c9b568a0) SHA1(e6c68ee4871020ded48e8a92546a8183a25f331f) )
	ROM_LOAD( "612-05.s2a",   0x80000, 0x40000, CRC(2ed51c5d) SHA1(0d912f8dc64f8fae35ca61cc0a938187a13ab328) )

	ROM_REGION( 0xc0000, "gfx4", 0 )	// blitter data
	ROM_LOAD( "612-06.s1b",   0x00000, 0x20000, CRC(8612904d) SHA1(5386e93ad16146ce4e48e81df304e8bf9d2db199) )
	ROM_LOAD( "612-07.s2b",   0x20000, 0x20000, CRC(0828c59d) SHA1(60c451de062c9e0000875022329450a55e913a3c) )
	ROM_LOAD( "612-11.s6c",   0x40000, 0x20000, CRC(473b6fcd) SHA1(1b99b1370bc739f0f00671c6b6cbb3255d581b55) )
	ROM_LOAD( "612-10.s5c",   0x60000, 0x20000, CRC(bf1edb0e) SHA1(932ca328c5968529d52b2c629b6bb866cfa1e784) )
	ROM_LOAD( "612-09.s4c",   0x80000, 0x20000, CRC(8345999e) SHA1(c70c731ababcb28752dd4961d6dc54d43cb6bcba) )
	ROM_LOAD( "612-08.s3c",   0xa0000, 0x20000, CRC(aad88516) SHA1(e6c7ef3325a17b2945530847998d314685c39f5d) )
ROM_END


/***************************************************************************

Mahjong Campus Hunting
(c)1990 Dynax
D3312108L1-1
D23SUB BOARD1 (sub board)

CPU  : Z80B
Sound: AY-3-8912A YM2413 M5205
OSC  : 22MHz (X1, near main CPU), 384KHz (X2, near M5205)
       3.58MHz (X3, Sound section)

ROMs:
3309.20      [0c7d72f0] OKI M271000ZB
3310.21      [28f5f194]  |
3311.22      [cddbf667]  |
3312.1       [cf0afbb5]  |
3313.2       [36e25beb]  |
3314.3       [f1cf01bc]  |
3315.4       [7cac01c7] /

3316.10      [44006ee5] M5M27C101P
3317.11      [4bb62bb4] /
3318.12      [e3b457a8] 27C010

Subboard ROMs:
3301.1B      [8ec98d60] OKI M271000ZB
3302.2B      [d7024f2d]  |
3303.3B      [01548edc]  |
3304.4B      [deef9a4e]  |
3305.1A      [8a9ebab8]  |
3306.2A      [86afcc80]  |
3307.3A      [07dbaf8a]  |
3308.4A      [a2cac53d] /

PALs:
D33A.24 (16L8)
D33B.79 (16L8)
D33C.67 (16R8)

CRT Controller:
HD46505SP (6845)

Custom chip:
DYNAX TC17G032AP-0246 JAPAN 8951EAY

***************************************************************************/

ROM_START( mcnpshnt )
	ROM_REGION( 0x10000 + 0xc*0x8000, "maincpu", 0 )	// Z80 Code
	ROM_LOAD( "3318.12", 0x000000, 0x020000, CRC(e3b457a8) SHA1(b768895797157cad029ac1f652a838ecf6587d4f) )
	ROM_RELOAD(          0x010000, 0x020000 )
	ROM_LOAD( "3317.11", 0x030000, 0x020000, CRC(4bb62bb4) SHA1(0de5605cecb1e729a5b5b866274395945cf88aa3) )
	ROM_LOAD( "3316.10", 0x050000, 0x020000, CRC(44006ee5) SHA1(287ffd095755dc2a1e40e667723985c9052fdcdf) )

	ROM_REGION( 0xe0000, "gfx1", 0 )	// blitter data
	ROM_LOAD( "3310.21", 0x00000, 0x20000, CRC(28f5f194) SHA1(ceb4605b25c49b6e6087e2e2f5db608d7e3ed0b2) )
	ROM_LOAD( "3311.22", 0x20000, 0x20000, CRC(cddbf667) SHA1(fbf94b8fdbe09cec5469c5f09d28e4d206763f90) )
	ROM_LOAD( "3309.20", 0x40000, 0x20000, CRC(0c7d72f0) SHA1(cbd0f29a31eab565b0e31fe1612e73164e6c61b4) )
	ROM_LOAD( "3312.1",  0x60000, 0x20000, CRC(cf0afbb5) SHA1(da340d49fb9513014f619124af56c115932cf18c) )
	ROM_LOAD( "3313.2",  0x80000, 0x20000, CRC(36e25beb) SHA1(21577849b356192d32d990d02d03092aa344e92e) )
	ROM_LOAD( "3314.3",  0xa0000, 0x20000, CRC(f1cf01bc) SHA1(fb02593d8b772b5e0128017998a0e15fc0708898) )
	ROM_LOAD( "3315.4",  0xc0000, 0x20000, CRC(7cac01c7) SHA1(cee5f157a23087b97709ff860078572b389e60cb) )

//  ROM_REGION( 0x00000, "gfx2", 0 )   // blitter data
//  unused

	ROM_REGION( 0x100000, "gfx3", 0 )	// blitter data
	ROM_LOAD( "3301.1b", 0x00000, 0x20000, CRC(8ec98d60) SHA1(e98d947096abb78e91c3013ede9eae7719b1d7b9) )
	ROM_LOAD( "3302.2b", 0x20000, 0x20000, CRC(d7024f2d) SHA1(49dfc26dc91a8632459852968766a5263be138eb) )
	ROM_LOAD( "3303.3b", 0x40000, 0x20000, CRC(01548edc) SHA1(a64b509a744dd010997d5b2cd4d12d2767dde6c8) )
	ROM_LOAD( "3304.4b", 0x60000, 0x20000, CRC(deef9a4e) SHA1(e0be7ba644e383d669a5ff1eb24c46461cc586a5) )
	ROM_LOAD( "3308.4a", 0x80000, 0x20000, CRC(a2cac53d) SHA1(fc580a85c94748afc1bbc49e25662e5a5cc8bb36) )
	ROM_LOAD( "3307.3a", 0xa0000, 0x20000, CRC(07dbaf8a) SHA1(99f995b71ca116d2e5587e08f9b0b4493d96937b) )
	ROM_LOAD( "3306.2a", 0xc0000, 0x20000, CRC(86afcc80) SHA1(e5d818761bb375b6c862546e238b2c6cf13898a8) )
	ROM_LOAD( "3305.1a", 0xe0000, 0x20000, CRC(8a9ebab8) SHA1(755c40a64541518b27cfa94959feb5de6f55b358) )
ROM_END


/***************************************************************************

7jigen no Youseitachi (Mahjong 7 Dimensions)
(c)1990 Dynax

D3707198L1
D23SUB BOARD1

CPU:    Z80-B
Sound:  AY-3-8912A
    YM2413
    M5205
OSC:    22.000MHz
    3.58MHz
    384KHz
VDP:    HD46505SP
Custom: (TC17G032AP-0246)


3701.1A   prg.

3702.3A   samples
3703.4A

3704.S1B  chr.
3705.S2B
3706.S3B
3707.S4B
3708.S1A
3709.S2A
3710.S3A
3711.S4A
3712.14A
3713.16A
3714.17A
3715.18A
3716.20A
3717.17B

***************************************************************************/

ROM_START( 7jigen )
	ROM_REGION( 0x10000 + 0xc*0x8000, "maincpu", 0 )	// Z80 Code
	ROM_LOAD( "3701.1a",  0x00000, 0x20000, CRC(ee8ab3c4) SHA1(9ccc9e9697dd452cd28e38c81cebea0b862f0642) )
	ROM_RELOAD(           0x10000, 0x20000 )
	ROM_LOAD( "3702.3a",  0x30000, 0x20000, CRC(4e43a0bb) SHA1(d98a1ab43dcfab3d2a17f99db797f7bfa17e5ecc) )
	ROM_LOAD( "3703.4a",  0x50000, 0x20000, CRC(ec77b564) SHA1(5e9d5540b300e88c3ecdb53bca38830621eb0382) )

	ROM_REGION( 0xc0000, "gfx1", 0 )	// blitter data
	ROM_LOAD( "3713.16a", 0x00000, 0x20000, CRC(f3a745d2) SHA1(37b55e2c290b165a5afaf4c7b8539bb57dd0d927) )
	ROM_LOAD( "3712.14a", 0x20000, 0x20000, CRC(88786680) SHA1(34a31448a9f3e287d7c7fe478736771c5ef259e2) )
	ROM_LOAD( "3715.18a", 0x40000, 0x20000, CRC(19f7ab13) SHA1(ac11e43981e8667c2637b66d93ac052fb27e521d) )
	ROM_LOAD( "3716.20a", 0x60000, 0x20000, CRC(4f0c7f06) SHA1(e0bbbb69cdd16932778e0b2f67e7ed068991a0b9) )
	ROM_LOAD( "3717.17b", 0x80000, 0x20000, CRC(960cfd62) SHA1(df8ee9eb8617a5e8605170d404872e1c6f0987f0) )
	ROM_LOAD( "3714.17a", 0xa0000, 0x20000, CRC(44ba5e35) SHA1(0c5c2b2a78aa397ea3d1264821ff717d093b81ae) )

//  ROM_REGION( 0x00000, "gfx2", 0 )   // blitter data
//  unused

	ROM_REGION( 0x100000, "gfx3", 0 )	// blitter data
	ROM_LOAD( "3704.s1b", 0x00000, 0x20000, CRC(26348ae4) SHA1(3659d18608848c58ad980a79bc1c29da238a5604) )
	ROM_LOAD( "3705.s2b", 0x20000, 0x20000, CRC(5b5ea036) SHA1(187a7f6356ead05d8e3d9f5efa82554004429780) )
	ROM_LOAD( "3706.s3b", 0x40000, 0x20000, CRC(7fdfb600) SHA1(ce4485e43ee6bf63b4e8e3bb91267295995c736f) )
	ROM_LOAD( "3707.s4b", 0x60000, 0x20000, CRC(67fa83ea) SHA1(f8b0012aaaf125b7266dbf1ae7df23d04d484e54) )
	ROM_LOAD( "3711.s4a", 0x80000, 0x20000, CRC(f1d4399d) SHA1(866af46900a4b04db69c838b7ec7e347a5fadd3d) )
	ROM_LOAD( "3710.s3a", 0xa0000, 0x20000, CRC(0a92af7c) SHA1(4383dc8f3019b3b2716d32e1c91b0ac5b1e367c3) )
	ROM_LOAD( "3709.s2a", 0xc0000, 0x20000, CRC(86f27f1c) SHA1(43b829597993d3043d5bbb0a468f603910638b87) )
	ROM_LOAD( "3708.s1a", 0xe0000, 0x20000, CRC(8082d0ac) SHA1(44d708f8e307b782105082092edd3ea9affd2329) )
ROM_END


/***************************************************************************

Jantouki
(c)1989 Dynax

D1505178-A (main board)
D2711078L-B (ROM board)

CPU:    Z80-B
Sound:  Z80-B
        AY-3-8912A
        YM2203C
        M5205
OSC:    22.000MHz
VDP:    HD46505SP
Custom: (TC17G032AP-0246) x2


2702.6D  MROM1  main prg.
2701.6C  MROM2

2705.6G  SROM1  sound prg.
2704.6F  SROM2  sound data
2703.6E  SROM3

2709.3G  BROM1  chr.
2710.3F  BROM2
2711.3E  BROM3
2712.3D  BROM4
2713.3C  BROM5
2706.5G  BROM6
2707.5F  BROM7
2708.5E  BROM8

2718.1G  AROM1  chr.
2719.1F  AROM2
2720.1E  AROM3
2721.1D  AROM4
2722.1C  AROM5
2714.2G  AROM6
2715.2F  AROM7
2716.2E  AROM8
2717.2D  AROM9

27-1_19H.18G    color
27-2.20H.19G
***************************************************************************/

ROM_START( jantouki )
	ROM_REGION( 0x20000, "maincpu", 0 )	// Z80 Code
	ROM_LOAD( "2702.6d", 0x000000, 0x010000, CRC(9e9bea93) SHA1(c8b1a0621d3dae37d809bdbaa4ed4af73847b714) )
	ROM_LOAD( "2701.6c", 0x010000, 0x010000, CRC(a58bc982) SHA1(5cdea3cdf3eaacb6bdf6ddb68e3d57fe53d70bb9) )

	ROM_REGION( 0x68000, "soundcpu", 0 )	// Z80 Code
	ROM_LOAD( "2705.6g", 0x000000, 0x010000, CRC(9d21e4af) SHA1(454601f4cb89da53c6881f4d8109d3c0babcfe5e) )
	// banks 4-b:
	ROM_LOAD( "2704.6f", 0x028000, 0x020000, CRC(4bb62bb4) SHA1(0de5605cecb1e729a5b5b866274395945cf88aa3) )
	ROM_LOAD( "2703.6e", 0x048000, 0x020000, CRC(44006ee5) SHA1(287ffd095755dc2a1e40e667723985c9052fdcdf) )

	ROM_REGION( 0x100000, "gfx1", 0 )	// blitter data
	ROM_LOAD( "2709.3g", 0x00000, 0x20000, CRC(e6dd4853) SHA1(85394e34eee95cd4430d062b3dbdfbe066c661b6) )
	ROM_LOAD( "2710.3f", 0x20000, 0x20000, CRC(7ef4d92f) SHA1(414e26242e824f5d4c40a039a3f3486f84338325) )
	ROM_LOAD( "2711.3e", 0x40000, 0x20000, CRC(8bfee4c2) SHA1(7c0e7535f7d7cd7f665e7925ff0cdab6b96a4b83) )
	ROM_LOAD( "2712.3d", 0x60000, 0x20000, CRC(6ecd4913) SHA1(00a2355d6cb1643b7cc964e702a4ac5cfe7906c5) )
	ROM_LOAD( "2713.3c", 0x80000, 0x20000, CRC(33272f5d) SHA1(8a23ef0e6ad24905fd5c249e8ea8560ec29a585c) )
	ROM_LOAD( "2706.5g", 0xa0000, 0x20000, CRC(fd72b190) SHA1(3d790dc1e40cbf963d8413ea91e518e19973734d) )
	ROM_LOAD( "2707.5f", 0xc0000, 0x20000, CRC(4ec7a81e) SHA1(a6227ca2b648ebc1a5a5f6fbfc6412c44752b77d) )
	ROM_LOAD( "2708.5e", 0xe0000, 0x20000, CRC(45845dc9) SHA1(cec3f82e3440f724f59d8386c8d2b0e030703ed5) )

	ROM_REGION( 0x100000, "gfx2", 0 )	// blitter 2 data
	ROM_LOAD( "2718.1g", 0x00000, 0x20000, CRC(65608d7e) SHA1(28a960450d2d1cfb314c574123c2fbc61f2ded51) )
	ROM_LOAD( "2719.1f", 0x20000, 0x20000, CRC(4cbc9361) SHA1(320d3ce504ad2e27937e7e3a761c672a22749658) )
	ROM_LOAD( "2720.1e", 0x40000, 0x20000, CRC(4c9a25e5) SHA1(0298a5dad034b1ac113f6e07f4e9334ed6e0e89b) )
	ROM_LOAD( "2721.1d", 0x60000, 0x20000, CRC(715c864a) SHA1(a4b436ddeaa161d6661063b6de503f07ecc5894a) )
	ROM_LOAD( "2722.1c", 0x80000, 0x20000, CRC(cc0b0cd7) SHA1(ccd3ff1cafbcaf87439a6dfe38b5057febc15012) )
	ROM_LOAD( "2714.2g", 0xa0000, 0x20000, CRC(17341b6b) SHA1(0ae43e53429e9561a00ea9597299477f2c7ddf4b) )
	ROM_LOAD( "2715.2f", 0xc0000, 0x20000, CRC(486b7138) SHA1(623ddb0e9a9444cf0e920b78562a4748fa1c54d9) )
	ROM_LOAD( "2716.2e", 0xe0000, 0x20000, CRC(f388b0da) SHA1(4c04509eeda3f82bf6f8940a406e17423d0210a0) )

	ROM_REGION( 0x20000, "gfx3", 0 )	// blitter 2 data
	ROM_LOAD( "2717.2d", 0x00000, 0x20000, CRC(3666bead) SHA1(2067bb894b76be2b51649bb1144e84e6ff0ab378) )

	ROM_REGION( 0x400, "proms", 0 )	// Color PROMs
	ROM_LOAD( "27-2_20h.19g", 0x000000, 0x000200, CRC(32d3f091) SHA1(ab9e8f467fc85357fb900bceae32909ce1f2d9c1) )
	ROM_LOAD( "27-1_19h.18g", 0x000200, 0x000200, CRC(9382a2a1) SHA1(0d14eb85017f87ddbe66e4f6443028e91540b36e) )
ROM_END


/***************************************************************************

    Mahjong Electron Base

***************************************************************************/

ROM_START( mjelctrn )
	ROM_REGION( 0x30000, "maincpu", 0 )	// Z80 Code
	ROM_LOAD( "u27b-020", 0x00000, 0x20000, CRC(7773d382) SHA1(1d2ae799677e99c7cba09b0a2c49bb9310232e80) )
	ROM_CONTINUE(         0x00000, 0x20000 )
	ROM_RELOAD(           0x10000, 0x20000 )
	ROM_CONTINUE(         0x28000, 0x08000 )
	ROM_CONTINUE(         0x20000, 0x08000 )
	ROM_CONTINUE(         0x18000, 0x08000 )
	ROM_CONTINUE(         0x10000, 0x08000 )

	ROM_REGION( 0x100000, "gfx1", 0 )	// blitter data
	ROM_LOAD( "eb-01.rom", 0x000000, 0x100000, CRC(e5c41448) SHA1(b8322e32b0cb3d771316c9c4f7be91de6e422a24) )

	ROM_REGION( 0x080000, "gfx2", 0 )	// blitter data
	ROM_LOAD( "eb-02.rom", 0x000000, 0x080000, CRC(e1f1b431) SHA1(04a612aff4c30cb8ea741f228bfa7e4289acfee8) )

	ROM_REGION( 0x040000, "gfx3", 0 )	// blitter data
	ROM_LOAD( "mj-1c020",  0x000000, 0x040000, CRC(f8e8d91b) SHA1(409e276157b328e7bbba5dda6a4c7adc020d519a) )
ROM_END

ROM_START( mjelct3 )
	ROM_REGION( 0x30000, "maincpu", 0 )	// Z80 Code
	ROM_LOAD( "se-3010", 0x00000, 0x20000, CRC(370347e7) SHA1(2dc9f1fde4efaaff887722aae6507d7e9fac8eb6) )
	ROM_RELOAD(          0x10000, 0x08000 )
	ROM_CONTINUE(        0x28000, 0x08000 )
	ROM_CONTINUE(        0x20000, 0x08000 )
	ROM_CONTINUE(        0x18000, 0x08000 )

	ROM_REGION( 0x100000, "gfx1", 0 )	// blitter data
	ROM_LOAD( "eb-01.rom", 0x000000, 0x100000, CRC(e5c41448) SHA1(b8322e32b0cb3d771316c9c4f7be91de6e422a24) )

	ROM_REGION( 0x80000, "gfx2", 0 )	// blitter data
	ROM_LOAD( "eb-02.rom", 0x00000, 0x80000, CRC(e1f1b431) SHA1(04a612aff4c30cb8ea741f228bfa7e4289acfee8) )
ROM_END

ROM_START( mjelct3a )
	ROM_REGION( 0x30000, "maincpu", 0 )	// Z80 Code
	ROM_LOAD( "dz-00.rom", 0x00000, 0x20000, CRC(d28358f7) SHA1(995c16e0865048069f79411574256a88d58c6be9) )
	ROM_RELOAD(            0x10000, 0x08000 )
	ROM_CONTINUE(          0x28000, 0x08000 )
	ROM_CONTINUE(          0x20000, 0x08000 )
	ROM_CONTINUE(          0x18000, 0x08000 )

	ROM_REGION( 0x100000, "gfx1", 0 )	// blitter data
	ROM_LOAD( "eb-01.rom", 0x000000, 0x100000, CRC(e5c41448) SHA1(b8322e32b0cb3d771316c9c4f7be91de6e422a24) )

	ROM_REGION( 0x80000, "gfx2", 0 )	// blitter data
	ROM_LOAD( "eb-02.rom", 0x00000, 0x80000, CRC(e1f1b431) SHA1(04a612aff4c30cb8ea741f228bfa7e4289acfee8) )
ROM_END

/***************************************************************************
Mahjong Electron Base (bootleg)

PCB works (see s/shot in archive), but I couldn't get it
to do anything other than show a diagnostic screen??

PCB Layout
----------

5G009B
|--------------------------------------------|
|    VOL  E5.U2        ALS9106               |
|uPC1242H                   TMS4461   TMS4461|
|         SANYA-01.U8                        |
|                           TMS4461   TMS4461|
|                                            |
|        3.579545MHz        6116      TMS4461|
|                                            |
|                           6116      TMS4461|
|  YM2149                                    |
|                                            |
|                          |-------|         |
|  6845                    |TAICOM |         |
|                          |AL9301 |     DSW2|
|           Z84C015        |       |         |
|                          |-------|     DSW3|
|                             DSW(2)         |
|  PRG.U27        21.25MHz               DSW4|
|    6264                                    |
|    10-WAY            18-WAY            DSW5|
|--------------------------------------------|
Notes:
PCB uses standard 10-way/18-way Mahjong pinout
ALS9106 - sound related?, tied to U2 & U8
AL9301  - TAICOM AL9301 graphics generator (QFP160)
Z84C015 - Toshiba TMPZ84C015BF-6 Z80 compatible CPU
***************************************************************************/

ROM_START( mjelctrb )
	ROM_REGION( 0x30000, "maincpu", 0 )	// Z80 Code
	ROM_LOAD( "prog.u27", 0x00000, 0x20000, CRC(688990ca) SHA1(34825cee8f76de93f12ccf2a1021f9c5369da46a) )
	ROM_RELOAD(          0x28000, 0x08000 )
	ROM_CONTINUE(        0x20000, 0x08000 )
	ROM_CONTINUE(        0x18000, 0x08000 )
	ROM_CONTINUE(        0x10000, 0x08000 )

	ROM_REGION( 0x100000, "gfx1", 0 )	// blitter data
	ROM_LOAD( "eb-01.rom", 0x000000, 0x100000, CRC(e5c41448) SHA1(b8322e32b0cb3d771316c9c4f7be91de6e422a24) )

	ROM_REGION( 0x80000, "gfx2", 0 )	// blitter data
	ROM_LOAD( "eb-02.rom", 0x00000, 0x80000, CRC(e1f1b431) SHA1(04a612aff4c30cb8ea741f228bfa7e4289acfee8) )
ROM_END


/*

Sea Hunter Penguin

CPU
1x Z8400A (main)
1x YM2203C (sound)
1x blank DIP40 with GND on pin 1,22 and +5 on pin 20
1x oscillator 22.1184MHz
ROMs

2x 27512 (u43,u45)
6x 27C010

ROM u43.8d contains

MODEL:SEA HUNTER(EXT)
PROGRAM BY:WSAC SYSTEMS,.

DATE:1995/02/20

VER:1.30

*/

ROM_START( shpeng )
	ROM_REGION( 0x90000, "maincpu", 0 )	// Z80 Code
	ROM_LOAD( "u43.8d", 0x00000, 0x10000, CRC(6b993f68) SHA1(4d3ad750e23be93342c61c454498d432e40587bb) )
	ROM_LOAD( "u45.9d", 0x28000, 0x10000, CRC(6e79a1d1) SHA1(a72706425bcbd0faee4cf0220942fdcf510d4e89) )

	ROM_REGION( 0xc0000, "gfx1", 0 )	// blitter data
	ROM_LOAD( "u110.1j", 0x00000, 0x20000, CRC(31abac75) SHA1(20e91496ccb379d9449925b5aaca3532caaf9522) ) // ok! - main sprites etc.
	ROM_LOAD( "u111.0j", 0x20000, 0x20000, CRC(0672bfc9) SHA1(ea35af45cdfa72ae1e7dc13a09ed1db09c0062ec) ) // ok?
	ROM_LOAD( "u84.1h",  0x40000, 0x20000, CRC(7b476fac) SHA1(6b61b675fbfcc17a77b9757ea330f8d3e8751633) )
	ROM_LOAD( "u804.0h", 0x60000, 0x20000, CRC(abb2f1c3) SHA1(9faccbba26c0540d9edbd76ca8bf67069db0bb53) )
	ROM_LOAD( "u74.1g",  0x80000, 0x20000, CRC(2ac46b6e) SHA1(0046ee7ede1acff45e64c85a9fca8fc8efa31026) )
	ROM_LOAD( "u704.0g", 0xa0000, 0x20000, CRC(b062c928) SHA1(8c43689a1b8c444f91acbc7371eda744874eb538) )

	ROM_REGION( 0x400, "proms", 0 )	// Color PROMs
	ROM_LOAD( "n82s147n.u13", 0x000, 0x200, CRC(29b6415b) SHA1(8085ff3265cda2d564da3dff609eb05ff02fae49) ) // FIXED BITS (0xxxxxxx)  (Ok)
	ROM_LOAD( "n82s147n.u12", 0x200, 0x200, BAD_DUMP CRC(7b940daa) SHA1(3903ebef644b2519aebbbb6d16872441b283c780) ) // BADADDR xxx-xxxxx  (Bad Read, Prom has a broken leg!)

	/* this rom doesn't belong here, it is from Dragon Punch, but shpeng hardware and game code is a hack
       of dragon punch.  This rom is better than the bad dump above for the sprite colours, although the
       colours on the intro/cutscenes are wrong */
	ROM_LOAD_OPTIONAL( "1.17g", 0x200, 0x200, CRC(324fa9cf) SHA1(a03e23d9a9687dec4c23a8e41254a3f4b70c7e25) )
ROM_END


// Decrypted by yong
static DRIVER_INIT( mjelct3 )
{
	int i;
	UINT8	*rom = memory_region(machine, "maincpu");
	size_t  size = memory_region_length(machine, "maincpu");
	UINT8	*rom1 = auto_alloc_array(machine, UINT8, size);

	memcpy(rom1, rom, size);
	for (i = 0; i < size; i++)
		rom[i] = BITSWAP8(rom1[BITSWAP24(i,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8, 1,6,5,4,3,2,7, 0)], 7,6, 1,4,3,2,5,0);
	auto_free(machine, rom1);
}

static DRIVER_INIT( mjelct3a )
{
	int i, j;
	UINT8	*rom = memory_region(machine, "maincpu");
	size_t  size = memory_region_length(machine, "maincpu");
	UINT8	*rom1 = auto_alloc_array(machine, UINT8, size);

	memcpy(rom1, rom, size);
	for (i = 0; i < size; i++)
	{
		j = i & ~0x7e00;
		switch(i & 0x7000)
		{
			case 0x0000:	j |= 0x0400;	break;
			case 0x1000:	j |= 0x4400;	break;
			case 0x2000:	j |= 0x4200;	break;
			case 0x3000:	j |= 0x0200;	break;
			case 0x4000:	j |= 0x4600;	break;
			case 0x5000:	j |= 0x4000;	break;
//              case 0x6000:    j |= 0x0000;    break;
			case 0x7000:	j |= 0x0600;	break;
		}
		switch(i & 0x0e00)
		{
			case 0x0000:	j |= 0x2000;	break;
			case 0x0200:	j |= 0x3800;	break;
			case 0x0400:	j |= 0x2800;	break;
			case 0x0600:	j |= 0x0800;	break;
			case 0x0800:	j |= 0x1800;	break;
//              case 0x0a00:    j |= 0x0000;    break;
			case 0x0c00:	j |= 0x1000;	break;
			case 0x0e00:	j |= 0x3000;	break;
		}
		rom[j] = rom1[i];
	}
	auto_free(machine, rom1);

	DRIVER_INIT_CALL(mjelct3);
}


/***************************************************************************

Neruton Haikujiradan
(c)1990 Dynax / Yukiyoshi Tokoro (Illustration)
D4005208L1-1
D4508308L-2 (sub board)

CPU  : Z80?
Sound: AY-3-8912A YM2413 M5205
OSC  : 22MHz (near main CPU), 3.58MHz (Sound section)

ROMs (all ROMs are 27C010 compatible):
4501B.1A     [0e53eeee]
4502.3A      [c296293f]
4511.11A     [c4a96b6e]
4512.13A     [d7ebbcb9]
4513.14A     [e3bed454]
4514.15A     [ee258483]
4515.17A     [3bce0ca1]
4516.18A     [ee6b7e3b]
4517.19A     [b31f9694]
4518.17C     [fa88668e]
4519.18C     [68aca5f3]
4520.19C     [7bb2b298]

Subboard ROMs:
4503.1A      [dcbe2805]
4504.2A      [7b3387af]
4505.3A      [6f9fd275]
4506.4A      [6eac8b3c]
4507.1B      [106e6133]
4508.2B      [5c451ed4]
4509.3B      [4e1e6a2d]
4510.4B      [455305a1]


PALs:
10B (?)
10E (?)
15E (?)
D45SUB.6A (16L8)

CRT Controller:
HD46505SP (6845)

***************************************************************************/

ROM_START( neruton )
	ROM_REGION( 0x50000, "maincpu", 0 )	// Z80 Code
	ROM_LOAD( "4501b.1a", 0x000000, 0x020000, CRC(0e53eeee) SHA1(883138618a11295bfac148da4a092e01d92229b3) )
	ROM_RELOAD(           0x010000, 0x020000 )
	ROM_LOAD( "4502.3a",  0x030000, 0x020000, CRC(c296293f) SHA1(466e87f7eca102568f1f00c6ba77dacc3df300dd) )

	ROM_REGION( 0x100000, "gfx1", 0 )	// blitter data
	ROM_LOAD( "4511.11a", 0x00000, 0x20000, CRC(c4a96b6e) SHA1(15a6776509e0d30929f6a261798afe7dc0401d4e) )
	ROM_LOAD( "4512.13a", 0x20000, 0x20000, CRC(d7ebbcb9) SHA1(b8edd8b93eca8d36056c02f8b69ff8313c9ab120) )
	ROM_LOAD( "4513.14a", 0x40000, 0x20000, CRC(e3bed454) SHA1(03a66d31b8f41abc4ce83ebe22f8d14414d92152) )
	ROM_LOAD( "4514.15a", 0x60000, 0x20000, CRC(ee258483) SHA1(8c685fee4eaff5978f0ec222c33d55123a8fa496) )
	ROM_LOAD( "4515.17a", 0x80000, 0x20000, CRC(3bce0ca1) SHA1(1d0bb379077c52a63aa982bbe77f89df7b5b7b14) )
	ROM_LOAD( "4516.18a", 0xa0000, 0x20000, CRC(ee6b7e3b) SHA1(5290fad850c7a52039cd9d26082bff8615bf3797) )
	ROM_LOAD( "4517.19a", 0xc0000, 0x20000, CRC(b31f9694) SHA1(f22fc44908be4f1ef8dada57860f95ee74495605) )
	ROM_LOAD( "4519.18c", 0xe0000, 0x20000, CRC(68aca5f3) SHA1(f03328362777e6d536f730bc3b52371d5daca54e) )

	ROM_REGION( 0x40000, "gfx2", 0 )	// blitter data
	ROM_LOAD( "4520.19c", 0x00000, 0x20000, CRC(7bb2b298) SHA1(643d21f6a45640bad5ec84af9745339487a7408c) )
	ROM_LOAD( "4518.17c", 0x20000, 0x20000, CRC(fa88668e) SHA1(fce80a8badacf39f30c36952cbe0a1491b8faef1) )

	ROM_REGION( 0x100000, "gfx3", 0 )	// blitter data
	ROM_LOAD( "4510.4b",  0x00000, 0x20000, CRC(455305a1) SHA1(103e1eaac485b37786a1d1d411819788ed385467) )
	ROM_LOAD( "4509.3b",  0x20000, 0x20000, CRC(4e1e6a2d) SHA1(04c71dd11594921142b6aa9554c0fe1b40254463) )
	ROM_LOAD( "4508.2b",  0x40000, 0x20000, CRC(5c451ed4) SHA1(59a27ddfae541cb61dafb32bdb5de8ddbc5abb8d) )
	ROM_LOAD( "4507.1b",  0x60000, 0x20000, CRC(106e6133) SHA1(d08deb17ea82fe43e458a11eea26ce98c26c51c1) )
	ROM_LOAD( "4506.4a",  0x80000, 0x20000, CRC(6eac8b3c) SHA1(70dbe3af582384571872e7b6b51df4192daed227) )
	ROM_LOAD( "4505.3a",  0xa0000, 0x20000, CRC(6f9fd275) SHA1(123a928dcb60624d61a55b2fef25156975ba26c9) )
	ROM_LOAD( "4504.2a",  0xc0000, 0x20000, CRC(7b3387af) SHA1(403cf67287469ae6ce9a7f662f6d82f62dac349b) )
	ROM_LOAD( "4503.1a",  0xe0000, 0x20000, CRC(dcbe2805) SHA1(713edd2e3c950bc689446441eb85197bb7b1eb89) )
ROM_END


/***************************************************************************

Mahjong Crystal 7
Dynax, 1990

PCB Layout
----------

D4005208L1-1
|------------------------------------------------------------------|
|    VR1       3.579545MHz  3-8912    22MHz|------|        4001.1A |
|    VR2   358                             |TMPZ84|        4002.3A |
|               YM2413    6845             |C015-6|                |
|          358  384kHz                     |------|                |
|               M5205                                              |
|                                                          5563    |
|M                                                         5.5V_BAT|
|A            6264                                                 |
|H                                                                 |
|J            6264                PAL         PAL                  |
|O                                                                 |
|N                                                                 |
|G  DSW1(8)                                                        |
|2  DSW2(8)                                                        |
|8  DSW3(8)                                                        |
|   DSW4(8)                                |------------ 4011.11A -| |-ROM-sub-board-D23SUB--|
|                                          |             4012.13A  | |                       |
|                                          |             4013.14A  | | 4003.1B   4007.1A     |
|                                          |             4014.15A-|| |                    |-||
|                       HM53461  PAL       |                    | || | 4004.2B   4008.2A  | ||
|               HM53461 HM53461            |                    | || |                    | ||
|               HM53461 HM53461            |                    | || | 4005.3B   4009.3A  | ||
|               HM53461 HM53461            |                    |-|| |                    |-||
|               HM53461   |----------|     |                       | | 4006.4B   4010.4A     |
|                         |    %     |     |                       | |                       |
|                         |----------|     |                       | |                       |
|------------------------------------------|-----------------------| |-----------------------|
Notes:
      TMPZ84C015-6    - Toshiba TMPZ84C015F-6 (QFP100)
                        XTAL1- 11.000MHz [22/2], XTAL2- 11.000MHz [22/2], CLKIN- 5.500MHz [22/4], CLKOUT- 5.500MHz [22/4]
      %               - SDIP64 IC, surface scratched off. This is most likely a Dynax NL-00x graphics generator or similar.
      AY-3-8912 clock - 1.375MHz [22/16]
      YM2413 clock    - 3.579545MHz
      M5205 clock     - 384kHz
      VSync           - 60Hz
      HSync           - 15.87kHz

***************************************************************************/

ROM_START( majxtal7 )
	ROM_REGION( 0x50000, "maincpu", 0 )	// Z80 Code
	ROM_LOAD( "4001.1a",  0x00000, 0x20000, CRC(82fc6dd5) SHA1(3c6e58d4c302a0f305c67c31fce6a1d4cbfe5f78) )
	ROM_RELOAD(           0x10000, 0x20000 )
	ROM_LOAD( "4002.3a",  0x30000, 0x10000, CRC(b5fec88f) SHA1(bc3a2404150edd570ea7d320b2d43735fbdce195) )
	ROM_RELOAD(           0x40000, 0x10000 )

	ROM_REGION( 0x80000, "gfx1", 0 )	// blitter data
	ROM_LOAD( "4011.11a", 0x00000, 0x20000, CRC(63551c37) SHA1(338f62125d217ab7a928476d36dd2797480ef3c1) )
	ROM_LOAD( "4012.13a", 0x20000, 0x20000, CRC(51a431d5) SHA1(df2327b61154f5c8eddc7572c08e714daa1498b4) )
	ROM_LOAD( "4013.14a", 0x40000, 0x20000, CRC(cdbb24f8) SHA1(6bec3931ceaed75bfee9079e095786b088b95e70) )
	ROM_LOAD( "4014.15a", 0x60000, 0x20000, CRC(f2677938) SHA1(778ce4a6bebef934749f65acd0b6472fd314ce7a) )

//  ROM_REGION( 0x00000, "gfx2", 0 )   // blitter data
//  unused

	ROM_REGION( 0x100000, "gfx3", 0 )	// blitter data
	ROM_LOAD( "4003.1b",  0x00000, 0x20000, CRC(26348ae4) SHA1(3659d18608848c58ad980a79bc1c29da238a5604) )
	ROM_LOAD( "4004.2b",  0x20000, 0x20000, CRC(5b5ea036) SHA1(187a7f6356ead05d8e3d9f5efa82554004429780) )
	ROM_LOAD( "4005.3b",  0x40000, 0x20000, CRC(7fdfb600) SHA1(ce4485e43ee6bf63b4e8e3bb91267295995c736f) )
	ROM_LOAD( "4006.4b",  0x60000, 0x20000, CRC(67fa83ea) SHA1(f8b0012aaaf125b7266dbf1ae7df23d04d484e54) )
	ROM_LOAD( "4010.4a",  0x80000, 0x20000, CRC(f1d4399d) SHA1(866af46900a4b04db69c838b7ec7e347a5fadd3d) )
	ROM_LOAD( "4009.3a",  0xa0000, 0x20000, CRC(0a92af7c) SHA1(4383dc8f3019b3b2716d32e1c91b0ac5b1e367c3) )
	ROM_LOAD( "4008.2a",  0xc0000, 0x20000, CRC(86f27f1c) SHA1(43b829597993d3043d5bbb0a468f603910638b87) )
	ROM_LOAD( "4007.1a",  0xe0000, 0x20000, CRC(8082d0ac) SHA1(44d708f8e307b782105082092edd3ea9affd2329) )
ROM_END

/***************************************************************************

Mahjong Raijinhai DX
Dynax, 1996

PCB Layout
----------

Top board

D10010318L1
sticker - D10502168
|----------------------------------------|
|DSW2(1)  DSW4(10)                  DIP16|
|                 |---|                  |
|DSW1(10) DSW3(10)| * |                  |
|                 |---|     PROM2        |
|                                        |
|                           PROM1        |
|                                        |
|                                        |
|                                        |
|                                        |
|                        1051.5E         |
| |-------------|                        |
| |     &       |        1052.4E    |---||
| |-------------|                   | D ||
|12MHz                   1053.3E    | I ||
|                                   | P ||
|BATTERY        32.768kHz           |40 ||
|         CLOCK          6264       |---||
|----------------------------------------|
Notes:
      Most of the chips have their surface scratched off.
      *     - Unknown PLCC44 IC. Possibly Mach110 or similar CPLD
      &     - Unknown SDIP64 IC. Probably a Toshiba TMP91P640. Clock input 12.000MHz
              Was read as a TMP91P640 and found to be protected.
      CLOCK - Some kind of clock IC, like Oki M6242 or similar
      PROM1 - TBP28S42 (equivalent to 82S147) PROM labelled 'D105-1'
      PROM2 - TBP28S42 (equivalent to 82S147) PROM labelled 'D105-2'
      DIP16 - Socket for cable that joins to lower board
      DIP40 - Socket for connector that joins to lower board


Bottom board

|--------------------------------------------------------|
|    BATTERY 6116                                        |
|  VOL                                                   |
|                                                        |
|                                              DIP40     |
|                                                        |
|           DSW(8)                              18.432MHz|
|                                                        |
|                                                        |
|M      DIP16                                            |
|A              4116    4116                             |
|H                                                       |
|J              4116    4116                             |
|O                                                       |
|N              4116    4116                             |
|G                                                       |
|2              4116    4116                             |
|8  AY3-8910                                             |
|               4116    4116                             |
|                                                        |
|               4116    4116                             |
|                                                        |
|               4116    4116                             |
|                                                        |
|               4116    4116                             |
|--------------------------------------------------------|
Notes:
      DIP16 - Socket for cable that joins to upper board
      DIP40 - Socket for connector that joins to upper board
      AY3-8910 clock - 1.536 [18.432/12]
      HSync - 15.5kHz
      VSync - 60Hz

***************************************************************************/

ROM_START( majrjhdx )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "1051d.5e",         0x00000, 0x40000, CRC(54c31732) SHA1(049e76c42fd248f975c7cce7e74b1f79e2a96bea) )
	ROM_RELOAD(                   0x10000, 0x40000 )	// the first 0x4000 bytes are Z80 code from "MAHJONG THE MYSTERIOUS ORIENT"!
	ROM_LOAD( "tmp91p640n-10.5b", 0x00000, 0x04000, NO_DUMP )

	ROM_REGION( 0x100000, "gfx1", 0 )	// blitter data
	ROM_LOAD( "1053d.3e",  0x00000, 0x80000, CRC(10bf7f0f) SHA1(c042240296ac7202da14e809bff36c9b0f97a3df) )
	ROM_LOAD( "1052d.4e",  0x80000, 0x80000, CRC(7200599c) SHA1(32e7caad9a9ea756b699f601fab90a419a437f57) )

	ROM_REGION( 0x400, "proms", 0 )	// Color PROMs
	ROM_LOAD( "d105-2.7e", 0x000, 0x200, CRC(587bca5a) SHA1(327f7bfa035f652bbbfba3f74715515236322c09) )
	ROM_LOAD( "d105-1.6e", 0x200, 0x200, CRC(6d0ce028) SHA1(35f70000a850782356734323fa93b150a77f807c) )
ROM_END

/***************************************************************************

Mahjong Reach (bootleg)
Dynax, 1994

PCB Layout
----------

|-------------------------------------------|
|  10-WAY       18-WAY   M6242B     BATTERY |
|     4558  3.579545MHz  32.768kHz   6264   |
|uPC1241H  VOL           DSW   DSW   ROM.U15|
|           UM3567       DSW   DSW          |
|   DSW     YM2149                  PAL     |
|                                           |
|2          TK-102                 TMP91C640|
|8                    6116                  |
|W                                          |
|A             PAL                          |
|Y                                          |
|                                           |
|                                           |
|                                           |
|                                           |
|            PAL                            |
|                                   ROM.U12 |
|   81461  81461          TK-101            |
|   81461  81461                    ROM.U13 |
|   81461  81461        21.47727MHz         |
|-------------------------------------------|

***************************************************************************/

ROM_START( mjreach )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "rom.u15", 0x00000, 0x40000, CRC(d914ac92) SHA1(f3284d2a74380b33fd5fe8f73fe88c8360a9b783) )
	ROM_RELOAD(          0x10000, 0x40000 )

	ROM_REGION( 0x100000, "gfx1", 0 )	// blitter data
	ROM_LOAD( "rom.u13", 0x00000, 0x80000, CRC(c4c7c2fc) SHA1(adb33a8f0ff99c9574bd91fc570f82882d1381f9) )
	ROM_LOAD( "rom.u12", 0x80000, 0x40000, CRC(87f47db6) SHA1(e9e9aae2b7b8dcb7d873a1b715ad4c9629c8936b) )
	ROM_RELOAD(          0xc0000, 0x40000 )

	ROM_REGION( 0x100000, "gfx2", 0 )	// blitter data
	ROM_LOAD( "rom.u13", 0x00000, 0x80000, CRC(c4c7c2fc) SHA1(adb33a8f0ff99c9574bd91fc570f82882d1381f9) )
	ROM_RELOAD(          0x80000, 0x80000 )
ROM_END

static DRIVER_INIT( mjreach )
{
	memory_install_write8_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x10060, 0x10060, 0, 0, yarunara_flipscreen_w);
}

/***************************************************************************

Mahjong Tenkaigen
Dynax, 1991

PCB Layout
----------

           |------|       |---------------|
|----------|      |-------|               |----------------------|
|           LM358                           M6242  6264  BATT    |
|       UPC1242  UM3567  3.57945MHz  DSW1 DSW2   TAICOM-00.2C    |
|           VOL  AY-3-8910           DSW3 DSW4            PAL1   |
|           DSW5                                 |-------------| |
|           ULN2003                    6116      |TMP91P640N-10| |
|                                                |(SDIP64)     | |
|                                                |-------------| |
|                       PAL2                                     |
|                                                                |
|                                                                |
|                                                                |
|                                                  PAL3          |
|                                                                |
|                3013554                           TAICOM-02.11B |
|                3013554  PAL4                     TAICOM-03.13B |
|                                       |-------|                |
|                41264  41264  41264    |AL9106 |                |
|                                       |(QFP64)|                |
|                41264  41264  41264    |-------|  TAICOM-01.15B |
|                                        21.47727MHz             |
------------------------------------------------------------------
Notes:
      TMP91P640N-10 - Main CPU; Toshiba TMP91P640N-10 Microcontroller with 16k internal ROM,
                      TLCS-90 core, running at 10.73635MHz [21.4727 / 2] (SDIP64)
      DSW1-4        - 10 switches each
      DSW5          - 8 switches each
      UM3567        - United Microelectronics Corporation UMC3567, compatible with YM2413 FM sound chip, running at 3.579549MHz (DIP24)
      UPC1242       - 5W Audio Power Amplifier
      LM358         - Low Power Dual Op Amp (DIP8)
      PAL1          - AMD PALCE16V8H (DIP20)
      PAL2          - AMI 18CV8 (DIP20)
      PAL3          - Signetics PLHS16L8ACN (DIP20)
      PAL4          - AMD PALCE22V10H (DIP24)
      6116          - 2K x8 SRAM (DIP24)
      6264          - 8K x8 SRAM (DIP28)
      AY-3-8910     - Microchip AY-3-8910A Programmable Sound Generator, running at 1.34204375MHz [21.4727 / 16] (DIP40)
      VOL           - Volume Potentiometer
      3013554       - Microchip 3013554-00 118438 (x2, DIP16)
      M6242         - OKI M6242 Real Time Clock chip (DIP18)
      41264         - NEC D41264C18184043 64K x4 VRAM (x6, SDIP24)
      ULN2003       - 7-bit 50V 500mA TTL-input NPN Darlington Driver (DIP16)
      AL9106        - AL9106 graphics generator (QFP64)

      ROMs          - Filename           Device
                      ----------------------------------------------------
                      TAICOM-00.2C     - ST M27C2001 256K x8 EPROM (DIP32)
                      TAICOM-01.15B    - 4MBit MASKROM (DIP32)
                      TAICOM-02.11B    - 4MBit MASKROM (DIP32)
                      TAICOM-03.13B    - AMD AM27C040 512K x8 EPROM (DIP32)
                      TMP91P640N-10.5B - Internal 16K ROM from MCU

***************************************************************************/

ROM_START( tenkai )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "taicom00.2c",      0x00000, 0x40000, CRC(a35e54db) SHA1(247c856e19989fb834e8ed135393927bbd9c0277) )
	ROM_RELOAD(                   0x10000, 0x40000 )
	ROM_LOAD( "tmp91p640n-10.5b", 0x00000, 0x04000, CRC(509f1c97) SHA1(08557bea2e924053fd5bc9de5e306f3ecf8e98e6) )

	// Note by Whistler:
	// It appears that the first half of lzc-01.u6 in tenkaibb (as well as the same data in other bootleg versions)
	// does not exist _anywhere_ in this rom dump, and in this way some girls won't show correctly (such as the 3rd one)
	ROM_REGION( 0x100000, "gfx1", 0 )	// blitter data
	ROM_LOAD( "tydg002.u8",   0x000000, 0x80000, NO_DUMP CRC(b0f08a20) SHA1(5f7083d5caadd77594eaf46efa11a8756cefcf7d) )
	ROM_LOAD( "taicom01.15b", 0x080000, 0x80000, CRC(39e4e6f3) SHA1(5b543a5933446091d7cfd519d5a6f23047d8a9f2) )	// either this was dumped half size, or the above rom was missing from the pcb

	ROM_REGION( 0x100000, "gfx2", 0 )	// blitter data
	ROM_LOAD( "taicom02.11b", 0x000000, 0x80000, CRC(aae8cfb7) SHA1(736c6148aa6e7b22ca19615a27e9a10d41778aa7) )
	ROM_LOAD( "taicom03.13b", 0x080000, 0x80000, CRC(68cb730a) SHA1(7ce90e34fa51d50a7668ac1c5ccbc18bebe8ad84) )
ROM_END

/***************************************************************************

Mahjong Tenkaigen Part 2 (bootleg)
Dynax, 1991

PCB Layout
----------

83228-3.GT
|-------------------------------------------|
|     4558  3.579545MHz  DSW   DSW   BATTERY|
|uPC1241H  VOL           DSW   DSW          |
|           K-663                           |
|1                       M5255     32.768kHz|
|8          TK-102             PAL          |
|W                           PAL   RTC62421 |
|A       DSW    6116         PAL            |
|Y                                  6264    |
|                                           |
|                                   MC0     |
|               PAL        PAL              |
|                               TMP91C640N  |
|                                           |
|                       PAL         MC1     |
|1               PAL                        |
|0                                  MC2     |
|W  81461  81461  TK-101                    |
|A  81461  81461                    MC3     |
|Y  81461  81461  21.2MHz                   |
|-------------------------------------------|

romcmp tenkai2b tenkai:

mc0.u11      [1/4]      taicom00.2c  [1/4]      33.557129%
mc0.u11      [2/4]      taicom00.2c  [2/4]      37.846375%
mc0.u11      [3/4]      taicom00.2c  [3/4]      IDENTICAL

mc1.u8       [2/2]      taicom01.15b            IDENTICAL

mc2.u21      [1/2]      taicom02.11b            IDENTICAL
mc2.u21      [2/2]      taicom03.13b            92.501640%

mc3.u15      [2/2]      taicom00.2c  [4/4]      IDENTICAL

***************************************************************************/

ROM_START( tenkai2b )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "mc0.u11",          0x00000, 0x40000, CRC(8488a3ab) SHA1(f367a2dcc65410929db595b3c442d310d50a4940) )
	ROM_RELOAD(                   0x10000, 0x40000 )
	// tenkai internal rom is incompatible with the code of this set
	ROM_LOAD( "tmp91p640n-10.5b", 0x00000, 0x04000, NO_DUMP )

	ROM_REGION( 0x100000, "gfx1", 0 )	// blitter data
	ROM_LOAD( "mc1.u8",  0x000000, 0x100000, CRC(786698e3) SHA1(9ddf4e31f454fb3c7969b1433771e95a976de741) )

	ROM_REGION( 0x100000, "gfx2", 0 )	// blitter data
	ROM_LOAD( "mc2.u21", 0x000000, 0x100000, CRC(713f79d7) SHA1(6e518b3127b232cc280b172dedbbc708987f9325) )

	ROM_REGION( 0x100000, "gfx3", 0 )	// blitter data
	ROM_COPY( "gfx1", 0, 0, 0x100000 )
	ROM_LOAD( "mc3.u15", 0x00000, 0x20000, CRC(5b552cdf) SHA1(86aaa02dcf8dab670d818287c91c8cb296362401) )	// 1xxxxxxxxxxxxxxxx = 0xFF
ROM_END

/***************************************************************************

Mahjong Tenkaigen (bootleg)
Dynax, 1991

PCB Layout
----------

|-------------------------------------------|
|     4558  3.579545MHz  32.768kHz   BATTERY|
|uPC1241H  VOL           DSW   DSW   6242B  |
|           YM2413       DSW   DSW          |
|1                       YM2149     PAL     |
|8          TK-102                          |
|W                                          |
|A              6116                        |
|Y                                  6264    |
|                                           |
|                                   TDH-12M |
|               PAL                         |
|                               TMP91P640N  |
|                                           |
|                       PAL         LZC-01  |
|1               PAL                        |
|0                                  LZC-02  |
|W  81461  81461  TK-101                    |
|A  81461  81461                    LZC-03  |
|Y  81461  81461  21.2MHz                   |
|-------------------------------------------|

romcmp tenkaibb tenkai2b:

lzc-01.u6               mc1.u8                  IDENTICAL

lzc-02.u19   [1/2]      mc2.u21      [1/2]      IDENTICAL
lzc-02.u19   [2/2]      mc2.u21      [2/2]      92.501640%

lzc-03.u15              mc3.u15                 IDENTICAL

tdh-12m.u11  [3/4]      mc0.u11      [3/4]      99.998474%
tdh-12m.u11  [4/4]      mc0.u11      [4/4]      99.597168%

romcmp tenkaibb tenkai:

lzc-01.u6    [2/2]      taicom01.15b            IDENTICAL

lzc-02.u19   [1/2]      taicom02.11b            IDENTICAL
lzc-02.u19   [2/2]      taicom03.13b            IDENTICAL

lzc-03.u15   [2/2]      taicom00.2c  [4/4]      IDENTICAL

tdh-12m.u11  [3/4]      taicom00.2c  [3/4]      99.998474%
tdh-12m.u11  [4/4]      taicom00.2c  [4/4]      IDENTICAL

***************************************************************************/

ROM_START( tenkaibb )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "tdh-12m.u11", 0x00000, 0x40000, CRC(7b877721) SHA1(41bba10ffb3d72af84d6577d4785225fe1ecc640) )
	ROM_RELOAD(              0x10000, 0x40000 )

	ROM_REGION( 0x100000, "gfx1", 0 )	// blitter data
	ROM_LOAD( "lzc-01.u6", 0x000000, 0x100000, CRC(786698e3) SHA1(9ddf4e31f454fb3c7969b1433771e95a976de741) )

	ROM_REGION( 0x100000, "gfx2", 0 )	// blitter data
	ROM_LOAD( "lzc-02.u19", 0x000000, 0x100000, CRC(90a19443) SHA1(8f593c00e39dd5acc76b058591019d117967a17b) )

	ROM_REGION( 0x100000, "gfx3", 0 )	// blitter data
	ROM_COPY( "gfx1", 0, 0, 0x100000 )
	ROM_LOAD( "lzc-03.u15", 0x000000, 0x020000, CRC(5b552cdf) SHA1(86aaa02dcf8dab670d818287c91c8cb296362401) )	// 1xxxxxxxxxxxxxxxx = 0xFF
ROM_END

/***************************************************************************

Mahjong Tenkaigen (bootleg)
Dynax, 1991

PCB Layout
----------

|-------------------------------------------|
|  10-WAY       18-WAY   M6242B     BATTERY |
|     4558  3.579545MHz  32.768kHz   6264   |
|uPC1241H  VOL           DSW   DSW   ROM.U15|
|           UM3567       DSW   DSW          |
|   DSW     YM2149                  PAL     |
|                                           |
|2          TK-102                 TMP91C640|
|8                    6116                  |
|W                                          |
|A             PAL                          |
|Y                                          |
|                                           |
|                                           |
|                                           |
|                                           |
|            PAL                            |
|                                   ROM.U12 |
|   81461  81461          TK-101            |
|   81461  81461                    ROM.U13 |
|   81461  81461        21.47727MHz         |
|-------------------------------------------|

romcmp tenkai tenkaicb:

taicom00.2c  [3/4]      rom.u15      [3/4]      99.998474%
taicom00.2c  [4/4]      rom.u15      [4/4]      IDENTICAL

taicom01.15b            rom.u12                 IDENTICAL

taicom03.13b            rom.u13                 IDENTICAL

***************************************************************************/

ROM_START( tenkaicb )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "rom.u15", 0x00000, 0x40000, CRC(7b877721) SHA1(41bba10ffb3d72af84d6577d4785225fe1ecc640) )
	ROM_RELOAD(          0x10000, 0x40000 )
	// it doesn't need the internal rom from tenkai

	ROM_REGION( 0x100000, "gfx1", 0 )	// blitter data
	ROM_LOAD( "tydg002.u8", 0x00000, 0x80000, NO_DUMP CRC(b0f08a20) SHA1(5f7083d5caadd77594eaf46efa11a8756cefcf7d) )
	ROM_LOAD( "rom.u12",    0x80000, 0x80000, CRC(39e4e6f3) SHA1(5b543a5933446091d7cfd519d5a6f23047d8a9f2) )

	ROM_REGION( 0x100000, "gfx2", 0 )	// blitter data
	ROM_LOAD( "taicom02.11b", 0x00000, 0x80000, NO_DUMP CRC(aae8cfb7) SHA1(736c6148aa6e7b22ca19615a27e9a10d41778aa7) )
	ROM_LOAD( "rom.u13",      0x80000, 0x80000, CRC(68cb730a) SHA1(7ce90e34fa51d50a7668ac1c5ccbc18bebe8ad84) )
ROM_END

/***************************************************************************

Mahjong Tenkaigen (? PCB is not working)

PCB Layout
----------

|-------------------------------------------|
|     4558  3.579545MHz             BATTERY |
|uPC1241H  VOL  K-663A   DSW   DSW   6264   |
|                        DSW   DSW  TYDG001 |
|   DSW     NL-002    YM2149  PAL           |
|1                                          |
|8            6116                 TMP91C640|
|W                                          |
|A                         TYDG004  TYDG002 |
|Y             PAL                          |
|                                           |
|                                           |
|                          TYDG005  TYDG003 |
|          PAL                              |
|                      PAL                  |
|1                PAL                       |
|0                                          |
|W  81461  81461  TK-101                    |
|A  81461  81461                            |
|Y  81461  81461  21.245MHz                 |
|-------------------------------------------|

romcmp tenkaid tenkaibb:

tydg002.u8              lzc-01.u6    [1/2]      IDENTICAL
tydg005.u19             lzc-01.u6    [2/2]      IDENTICAL

tydg001.u11  [1/4]      tdh-12m.u11  [odd 2/2]  6.478882%
tydg001.u11  [2/4]      lzc-03.u15   [odd]      31.292725%
tydg001.u11  [3/4]      tdh-12m.u11  [even 2/2] 55.366516%
tydg001.u11  [4/4]      lzc-03.u15   [even]     59.931946%

tydg003.u6   [1/2]      lzc-02.u19   [3/4]      2.625275%
tydg003.u6   [2/2]      lzc-02.u19   [2/4]      1.834488%

tydg004.u21  [1/2]      lzc-02.u19   [1/4]      3.769302%
tydg004.u21  [2/2]      lzc-02.u19   [4/4]      14.524460%

romcmp tenkaid tenkai:

tydg005.u19             taicom01.15b            IDENTICAL

tydg001.u11  [1/4]      taicom00.2c  [1/4]      26.155090%
tydg001.u11  [2/4]      taicom00.2c  [3/4]      22.038269%
tydg001.u11  [3/4]      taicom00.2c  [2/4]      4.237366%
tydg001.u11  [4/4]      taicom00.2c  [4/4]      59.855652%

tydg002.u8   [4/4]      taicom03.13b [2/4]      4.472351%

tydg003.u6   [1/4]      taicom02.11b [3/4]      2.860260%
tydg003.u6   [2/4]      taicom03.13b [3/4]      6.129456%
tydg003.u6   [3/4]      taicom02.11b [2/4]      2.205658%

tydg004.u21  [1/4]      taicom02.11b [1/4]      5.610657%
tydg004.u21  [2/4]      taicom03.13b [1/4]      1.743317%
tydg004.u21  [3/4]      taicom02.11b [4/4]      2.343750%
tydg004.u21  [4/4]      taicom03.13b [4/4]      24.230194%

***************************************************************************/

ROM_START( tenkaid )
	ROM_REGION( 0x90000, "maincpu", 0 )
	ROM_LOAD( "tydg001.u11",      0x00000, 0x40000, CRC(4ffa543c) SHA1(ab6ec7bd735358643f5186c6c983fa8b599fe84b) )
	ROM_RELOAD(                   0x10000, 0x40000 )
	ROM_RELOAD(                   0x50000, 0x40000 )
	// tenkai internal rom is incompatible with the code of this set
	ROM_LOAD( "tmp91p640n-10.5b", 0x00000, 0x04000, NO_DUMP )

	ROM_REGION( 0x100000, "gfx1", 0 )	// blitter data
	ROM_LOAD( "tydg002.u8",  0x00000, 0x80000, CRC(b0f08a20) SHA1(5f7083d5caadd77594eaf46efa11a8756cefcf7d) )
	ROM_LOAD( "tydg003.u6",  0x80000, 0x80000, CRC(60717d91) SHA1(85dbb510d33b36d2255b740ccc4917216dd21497) )

	ROM_REGION( 0x100000, "gfx2", 0 )	// blitter data
	ROM_LOAD( "tydg004.u21", 0x00000, 0x80000, CRC(b7d49d04) SHA1(756c35bbe207b5bfc6e05d6da99a7ad5a3453506) )
	ROM_LOAD( "tydg005.u19", 0x80000, 0x80000, CRC(39e4e6f3) SHA1(5b543a5933446091d7cfd519d5a6f23047d8a9f2) )
ROM_END

/***************************************************************************

tenkaigen set 2

romcmp tenkaie tenkai:

epr-a01.rom             taicom00.2c             IDENTICAL

lzc-01.rom   [2/2]      taicom01.15b            IDENTICAL

lzc-02.rom   [1/2]      taicom02.11b            IDENTICAL
lzc-02.rom   [2/2]      taicom03.13b            IDENTICAL

***************************************************************************/

ROM_START( tenkaie )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "epr-a01.rom",      0x00000, 0x40000, CRC(a35e54db) SHA1(247c856e19989fb834e8ed135393927bbd9c0277) )
	ROM_RELOAD(                   0x10000, 0x40000 )
	ROM_LOAD( "tmp91p640n-10.5b", 0x00000, 0x04000, NO_DUMP CRC(509f1c97) SHA1(08557bea2e924053fd5bc9de5e306f3ecf8e98e6) )

	ROM_REGION( 0x100000, "gfx1", 0 )	// blitter data
	ROM_LOAD( "lzc-01.rom", 0x000000, 0x100000, CRC(786698e3) SHA1(9ddf4e31f454fb3c7969b1433771e95a976de741) )

	ROM_REGION( 0x100000, "gfx2", 0 )	// blitter data
	ROM_LOAD( "lzc-02.rom", 0x000000, 0x100000, CRC(90a19443) SHA1(8f593c00e39dd5acc76b058591019d117967a17b) )
ROM_END


/***************************************************************************

Mahjong Comic Gekijou Vol.1
(c)1991 Dynax / Tenho Iwatani
D5512068L1-1 (main PCB)
D6107068L-1  (sub PCB)

CPU: Z80B
Sound: AY-3-8912, YM2413, M5205
OSC: 20.0000MHz (near Z80B)
     14.31818MHz (near sound section)
     ?MHz (near RTC)
Custom: DYNAX NL-001
        DYNAX TC17G032AP-0246
Others: 3.6V Ni-Cd Battery, M6242B RTC

ROMs:
6101.2D      [60552776] \
6102.4D      [4b4f3966] -- Programs?

6103.5D      [6d56e5c1] -- Samples?

6104.1A      [04eb1ce0] \
6105.2A      [54115f33] |
6106.3A      [093faddb] |
6107.4A      [0b0997f5] |
6108.5A      [bd31ae6f] |
6109.1B      [cf718f69] |- Graphics
6110.2B      [2865eae4] |
6111.3B      [581edcc0] |
6112.4B      [3083c0cf] |
6113.5B      [e34e9541] |
6114.1C      [1aa9a1d7] /

d61a.4e \
d61b.7b -- PAL16CEV8 (not dumped)

6104-6114 and D61B is on sub PCB, others are on main PCB

***************************************************************************/

ROM_START( mjcomv1 )
	ROM_REGION( 0x10000 + 0x28*0x8000, "maincpu", 0 )	// Z80 Code
	ROM_LOAD( "6101.2d", 0x000000, 0x20000, CRC(60552776) SHA1(9876f1aece8f25b7e495c6fac24ebb5028916f73) )
	// 00-03
	ROM_RELOAD(          0x010000, 0x20000 )
	// 0c-0f
	ROM_RELOAD(          0x070000, 0x20000 )
	// 24-27
	ROM_RELOAD(          0x130000, 0x20000 )
	// 04-07
	ROM_LOAD( "6102.4d", 0x030000, 0x20000, CRC(4b4f3966) SHA1(150cf8fe6342ea9a956073b3ebba6553c13e9cf8) )
	// 08-0b
	ROM_LOAD( "6103.5d", 0x050000, 0x20000, CRC(6d56e5c1) SHA1(2c02a400d21e442cdd68bf6210b397b770cde3b5) )	// 1ST AND 2ND HALF IDENTICAL

//  ROM_REGION( 0x00000, "gfx1", 0 )   // blitter data
//  unused

//  ROM_REGION( 0x00000, "gfx2", 0 )   // blitter data
//  unused

	ROM_REGION( 0x100000, "gfx3", 0 )	// blitter data
	//                   0x00000, 0x20000
	ROM_LOAD( "6105.2a", 0x20000, 0x20000, CRC(54115f33) SHA1(ed7d00c9b5c8aad066cf92c627b36c3a5e982d9f) )
	ROM_LOAD( "6106.3a", 0x40000, 0x20000, CRC(093faddb) SHA1(ac8ee5abcd8a7b28f28407f5488c21a4bbff305a) )
	ROM_LOAD( "6107.4a", 0x60000, 0x20000, CRC(0b0997f5) SHA1(ef31ca2818b8aef7fac01293e34fd7b37c8326f4) )
	ROM_LOAD( "6108.5a", 0x80000, 0x20000, CRC(bd31ae6f) SHA1(cc322dd07acab85874e5f033c65d2c99838d7474) )
	ROM_LOAD( "6109.1b", 0xa0000, 0x20000, CRC(cf718f69) SHA1(0e8f9e6c9ef35f71a4b7fcaf62e4c22b486dcb9d) )
	ROM_LOAD( "6110.2b", 0xc0000, 0x20000, CRC(2865eae4) SHA1(dd945a2a531a08e654f13c135bb9cb799589d513) )
	ROM_LOAD( "6111.3b", 0xe0000, 0x20000, CRC(581edcc0) SHA1(d52de6ca199f03e0d88c8e4275fe2b37b3ef6016) )

	ROM_REGION( 0x60000, "gfx4", 0 )	// blitter data
	ROM_LOAD( "6112.4b", 0x00000, 0x20000, CRC(3083c0cf) SHA1(24465e2d01cb0f0646644a3a5d57d9c0f456cf96) )
	ROM_LOAD( "6113.5b", 0x20000, 0x20000, CRC(e34e9541) SHA1(fbe457b4107730f3d633536e82b9271dcbc71559) )
	ROM_LOAD( "6114.1c", 0x40000, 0x20000, CRC(1aa9a1d7) SHA1(67991ff4968443e596fd2fadb097e32d2e6802c3) )

	ROM_REGION( 0x80000, "gfx5", 0 )	// blitter data
	ROM_LOAD( "6104.1a", 0x00000, 0x80000, CRC(04eb1ce0) SHA1(670b213db190bb845c0a99e0a8b166ebff8a7ea1) )
ROM_END


/***************************************************************************

Hana Jingi
Dynax 1990

PCB Layout
----------

|---------------------------------------------------|
|             PAL               SDIP64     H04.1    |
|    MB81461                               H05.2    |
|    MB81461  MB81461           H01.20     H06.3    |
|    MB81461  MB81461           H02.21     H07.4    |
|    MB81461  MB81461 PAL       H03.22     H08.5    |
|M            MB81461                      H09.6    |
|A                HD46505                           |
|H                                                  |
|J                                                  |
|O                6116          PAL                 |
|N DSW2                                             |
|G DSW3           6116                              |
|  DSW4    M5205                  |------22MHz------|
|      384kHz                     |                 |
|   LM358       AY38912           |                 |
|MB3712 LM358 YM2413      ULN2003 |            /\   |
|VOL1 VOL2 3.579545MHz DSW1 JP1   | DIP40      |    |
|---------------------------------|------------|----|
Notes:                            Sub Board Above
      JP1     - 8 position connector
      AY38912 - Clock 1.375MHz [22/16]
      YM2413  - Clock 3.579545MHz
      M5205   - Clock 384kHz
      ALL ROMs- Type 27C010


Sub Board
---------
D3312108L1-2
|----------------|
|DIP16 DIP14 SW1 |
| DIP24      0.1F|
| Z80      DIP28 |
| DIP40  H10B.4A |
|----------------|
Notes:
      DIP40 - connection socket
      DIP28 - Scratched chip, likely 6264 or 62256 SRAM
      DIP14 - Scratched chip, likely logic
      DIP16 - Scratched chip, likely logic
      DIP24 - Scratched chip, likely a PAL (PAL22V10 or something)
      0.1F  - 0.1 Farad supercap
      SW1   - SPST switch
      Z80   - Clock 5.5MHz [22/4]
      H10B.4A - 27C010 EPROM

***************************************************************************/

ROM_START( hjingi )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "h10b.4a", 0x00000, 0x20000, CRC(e1152b17) SHA1(ced822eafa96c89dda82fd8ea002e86c2eb4438a) )
	ROM_RELOAD(          0x10000, 0x20000 )

	ROM_REGION( 0x100000, "gfx1", 0 )	// blitter data
	ROM_LOAD( "h02.21", 0x00000, 0x20000, CRC(9dde2d59) SHA1(96df4ba97ee9611d9a3c7bcaae9cd97815a7b8a5) )
	ROM_LOAD( "h03.22", 0x20000, 0x20000, CRC(1ac59443) SHA1(e70fe6184e7090cf7229d83b87db65f7715de2a8) )
	ROM_LOAD( "h01.20", 0x40000, 0x20000, CRC(24391ddc) SHA1(6a2e3fae4b6d0b1d8073306f37c9fdaa04b69eb8) )
	ROM_LOAD( "h04.1",  0x60000, 0x20000, CRC(3756c745) SHA1(f275a72d6e07f21148900d24a8018d03504f249f) )
	ROM_LOAD( "h05.2",  0x80000, 0x20000, CRC(249d360a) SHA1(688fced1298c345a18314d2c88664c757a2de35c) )
	ROM_LOAD( "h06.3",  0xa0000, 0x20000, CRC(014a4945) SHA1(0cd747787a81226fd4937616a6ce45af731a4049) )
	ROM_LOAD( "h07.4",  0xc0000, 0x20000, CRC(8b6f8a2d) SHA1(c5f3ec64a7ea3edc556182f42e6da4842d88e0ba) )
	ROM_LOAD( "h08.5",  0xe0000, 0x20000, CRC(6f996e6e) SHA1(c2b916afbfd257417f0383ad261f3720a027fdd9) )

	ROM_REGION( 0x20000, "gfx2", 0 )	// blitter data
	ROM_LOAD( "h09.6", 0x00000, 0x20000, CRC(86bde64d) SHA1(d9b79184697044ae8a4d04ea22deca2e14162065) )
ROM_END


/***************************************************************************

Hanafuda Hana Tengoku
(c)1992 Dynax

D6502208L1
CPU   : TMP91P640? (surface scratched)
Sound : AY38910A/P(YM2149F version exists), YM2413
OSC   : 20.00000MHz, 14.31818MHz, ?(near 6242)
Others: M6242B(RTC), battery
DIPs  : 10 position (x4), 4 position (x1)
ROMs  : 6501.4B
        6509.10B
        6510.11B

D6107068L-1
ROMs  : 6502.1A
        6503.2A
        6504.1B
        6505.2B
        6506.4C
        6507.5C
        6508.6C

dumped by sayu
--- Team Japump!!! ---


Daughterboard number - D6107068L-1
Top daughterboard has most of the ROMs
Mainboard has roms labelled 6501, 6509, 6510
The main board is almost like a few other Dynax Mahjong PCB's I've
documented, so most of the details should be the same. The layout is
similar to Mysterious World.

Mainboard number - D6502208L1
Mainboard main parts include.....
AY3-8910
YM2413
RAM TC5563 x1
RAM 2018 x1
RAM HM53461 x6 (plus 2 empty spaces for 2 more)
RAM TC524256 x2
M6242 RTC + BATTERY
Two scratched SDIP64 chips (possibly NL-001 and NL-002 or similar)
Another scratched chip QFP64.... should be another known Dynax IC.
DIP40 chip near ROM 6501... surface scratched too. Probably Z80
XTALs - 20Mz, 14.31818MHz
DSWs - 4x 10-position, 1x 4-position

***************************************************************************/

ROM_START( htengoku )
	ROM_REGION( 0x50000, "maincpu", 0 )	// Z80 Code
	ROM_LOAD( "6501.4b", 0x00000, 0x40000, CRC(29a7fc83) SHA1(5d3cf0a72918e58b5b60f7c978e559c7c1306bce) )
	ROM_RELOAD(          0x10000, 0x40000 )

	ROM_REGION( 0x80000, "gfx1", 0 )	// blitter data
	ROM_LOAD( "6506.4c",  0x00000, 0x80000, CRC(7de17b26) SHA1(326667063ab045ac50e850f2f7821a65317879ad) )

	ROM_REGION( 0xc0000, "gfx2", 0 )	// blitter data
	ROM_LOAD( "6507.5c", 0x00000, 0x20000, CRC(ced3155b) SHA1(658e3947781f1be2ee87b43952999281c66683a6) )
	ROM_LOAD( "6508.6c", 0x20000, 0x20000, CRC(ca46ed48) SHA1(0769ac0b211181b7b57033f09f72828c885186cc) )
	ROM_LOAD( "6505.2b", 0x40000, 0x20000, CRC(161058fd) SHA1(cfc21abdc036e874d34bfa3c60486a5ab87cf9cd) )
	ROM_LOAD( "6504.1b", 0x60000, 0x20000, CRC(b2ca9838) SHA1(7104697802a0466fab40414a467146a224eb6a74) )
	ROM_LOAD( "6503.2a", 0x80000, 0x20000, CRC(6ac42304) SHA1(ce822da6d61e68578c08c9f1d0af1557c64ac5ae) )
	ROM_LOAD( "6502.1a", 0xa0000, 0x20000, CRC(9276a10a) SHA1(5a68fff20631a2002509d6cace06b5a9fa0e75d2) )

	ROM_REGION( 0xa0000, "gfx3", 0 )	// blitter data
	ROM_LOAD( "6509.10b", 0x00000, 0x80000, CRC(f8524c28) SHA1(d50b99664c9f0735838adb55aa7db53e58a43f99) )
	ROM_LOAD( "6510.11b", 0x80000, 0x20000, CRC(0fdd6edf) SHA1(c6870ab538987110337e6e154cba98391c68fb98) )
ROM_END

/***************************************************************************

Mahjong Gekisha
1989 Dynax

PCB Layout
----------
                            SUBBOARD HERE
D2809218L-1                    |
|--------------------------|---|-------|
|                          |   \/  5563|
|          2801 2802 2803 2804 2805    |
|    HD46505               |           |
|                          |           |
|            TC17G032AP-0246           |
|M                         |           |
|A                  4461   |-----------|
|H                  4461           0.1F|
|J                                     |
|O                  82S147  PAL        |
|N      DSW1                           |
|G      DSW2        82S147             |
|       DSW3                           |
|       DSW4  YM2413                   |
|                           62003      |
|    MB3712 VOL             CN1   24MHz|
|--------------------------------------|
Notes:
      CN1  - 8 pin connector
      0.1F - 0.1 Farad Supercap
      5563 - 8kx8 SRAM
      4461 - TMS4461 64kx4 Multiport Video RAM


Sub Board

D2809218L-2
|------------|
|DIP16  10MHz|
|  SDIP64    |
|            |
|DIP20  DIP20|
|            |
|  2806.IC6  |
|------------|
Notes:
      DIPxx    - unknown DIP chips, surface scratched
      SDIP64   - probably MCU
      2806.IC6 - 27C512 EPROM

***************************************************************************/

ROM_START( gekisha )
	ROM_REGION( 0x50000, "maincpu", 0 )	// TLCS90 Code
	ROM_LOAD( "2806.ic6",  0x00000, 0x10000, CRC(823214d7) SHA1(61e6b892f4bbed7ef5630e434f009b1cbf1a4a95) )
	ROM_LOAD( "2805.ic19", 0x10000, 0x10000, CRC(03e2ed1a) SHA1(644153a186a787efdb12a108d49700e1c94e8af2) )

	ROM_REGION( 0x70000, "gfx1", 0 )	// blitter data
	ROM_LOAD( "2801.ic40", 0x00000, 0x20000, CRC(cf75f9a4) SHA1(d078d009c347f927d8efd325921f4f75f6485c79) )
	ROM_LOAD( "2802.ic32", 0x20000, 0x20000, CRC(c505c64a) SHA1(3920a085e7afacc1fa2fbc862d0e92fc2955d636) )
	ROM_LOAD( "2803.ic31", 0x40000, 0x20000, CRC(bfc8ffca) SHA1(adceebcde8f0b649219503257bb968202697b96a) )
	ROM_LOAD( "2804.ic22", 0x60000, 0x10000, CRC(94911930) SHA1(08bc57329a3c7aa716d9aee87bf84a2992269c07) )

	ROM_REGION( 0x400, "proms", 0 )	// Color PROMs (same as mjfriday)
	ROM_LOAD( "pr2.ic27", 0x000, 0x200, CRC(d6db5c60) SHA1(89ee10d092011c2c4eaab2c097aa88f5bb98bb97) )	// FIXED BITS (0xxxxxxx)
	ROM_LOAD( "pr1.ic26", 0x200, 0x200, CRC(af5edf32) SHA1(7202e0aa1ee3f22e3c5fb69a88db455a241929c5) )
ROM_END


/***************************************************************************


                                Game Drivers


***************************************************************************/

GAME( 1989, hnkochou, 0,        hanamai,  hnkochou, 0,        ROT180, "Dynax",                    "Hana Kochou (Japan, Bet)",                                      GAME_SUPPORTS_SAVE )
GAME( 1988, hanamai,  hnkochou, hanamai,  hanamai,  0,        ROT180, "Dynax",                    "Hana no Mai (Japan)",                                           GAME_SUPPORTS_SAVE )
GAME( 1990, hjingi,   0,        hjingi,   hjingi,   0,        ROT180, "Dynax",                    "Hana Jingi (Japan, Bet)",                                       GAME_SUPPORTS_SAVE )
GAME( 1989, hnoridur, hjingi,   hnoridur, hnoridur, 0,        ROT180, "Dynax",                    "Hana Oriduru (Japan)",                                          GAME_SUPPORTS_SAVE )
GAME( 1989, drgpunch, 0,        sprtmtch, drgpunch, 0,        ROT0,   "Dynax",                    "Dragon Punch (Japan)",                                          GAME_SUPPORTS_SAVE )
GAME( 1989, sprtmtch, drgpunch, sprtmtch, sprtmtch, 0,        ROT0,   "Dynax (Fabtek license)",   "Sports Match",                                                  GAME_SUPPORTS_SAVE )
/* these 4 are Korean hacks / bootlegs of Dragon Punch / Sports Match */
GAME( 1994, maya,     0,        sprtmtch, drgpunch, maya,     ROT0,   "Promat",                   "Maya (set 1)",                                                  GAME_SUPPORTS_SAVE ) // this set has backgrounds blacked out in attract
GAME( 1994, mayaa,    maya,     sprtmtch, drgpunch, maya,     ROT0,   "Promat",                   "Maya (set 2)",                                                  GAME_SUPPORTS_SAVE )
GAME( 199?, inca,     0,        sprtmtch, drgpunch, maya,     ROT0,   "<unknown>",                "Inca",                                                          GAME_SUPPORTS_SAVE )
GAME( 199?, blktouch, 0,        sprtmtch, drgpunch, blktouch, ROT0,   "Yang Gi Co Ltd.",          "Black Touch (Korea)",                                           GAME_SUPPORTS_SAVE )

GAME( 1989, mjfriday, 0,        mjfriday, mjfriday, 0,        ROT180, "Dynax",                    "Mahjong Friday (Japan)",                                        GAME_SUPPORTS_SAVE )
GAME( 1989, gekisha,  0,        gekisha,  gekisha,  0,        ROT180, "Dynax",                    "Mahjong Gekisha",                                               GAME_SUPPORTS_SAVE )
GAME( 1990, mcnpshnt, 0,        mcnpshnt, mcnpshnt, 0,        ROT0,   "Dynax",                    "Mahjong Campus Hunting (Japan)",                                GAME_SUPPORTS_SAVE )
GAME( 1990, 7jigen,   0,        nanajign, nanajign, 0,        ROT180, "Dynax",                    "7jigen no Youseitachi - Mahjong 7 Dimensions (Japan)",          GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE )
GAME( 1990, jantouki, 0,        jantouki, jantouki, 0,        ROT0,   "Dynax",                    "Jong Tou Ki (Japan)",                                           GAME_SUPPORTS_SAVE )
GAME( 1991, mjdialq2, 0,        mjdialq2, mjdialq2, 0,        ROT180, "Dynax",                    "Mahjong Dial Q2 (Japan)",                                       GAME_SUPPORTS_SAVE )
GAME( 1991, yarunara, 0,        yarunara, yarunara, 0,        ROT180, "Dynax",                    "Mahjong Yarunara (Japan)",                                      GAME_SUPPORTS_SAVE )
GAME( 1991, mjangels, 0,        yarunara, yarunara, 0,        ROT180, "Dynax",                    "Mahjong Angels - Comic Theater Vol.2 (Japan)",                  GAME_SUPPORTS_SAVE )
GAME( 1992, quiztvqq, 0,        yarunara, quiztvqq, 0,        ROT180, "Dynax",                    "Quiz TV Gassyuukoku Q&Q (Japan)",                               GAME_SUPPORTS_SAVE )
GAME( 1993, mjelctrn, 0,        mjelctrn, mjelctrn, mjelct3,  ROT180, "Dynax",                    "Mahjong Electron Base (parts 2 & 4, Japan)",                    GAME_SUPPORTS_SAVE )
GAME( 1990, mjelct3,  mjelctrn, mjelctrn, mjelct3,  mjelct3,  ROT180, "Dynax",                    "Mahjong Electron Base (parts 2 & 3, Japan)",                    GAME_SUPPORTS_SAVE )
GAME( 1990, mjelct3a, mjelctrn, mjelctrn, mjelct3,  mjelct3a, ROT180, "Dynax",                    "Mahjong Electron Base (parts 2 & 3, alt., Japan)",              GAME_SUPPORTS_SAVE )
GAME( 1993, mjelctrb, mjelctrn, mjelctrn, mjelct3,  mjelct3,  ROT180, "bootleg",                  "Mahjong Electron Base (parts 2 & 4, Japan, bootleg)",           GAME_SUPPORTS_SAVE )
GAME( 1990, majxtal7, 0,        majxtal7, majxtal7, mjelct3,  ROT180, "Dynax",                    "Mahjong X-Tal 7 - Crystal Mahjong / Mahjong Diamond 7 (Japan)", GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE )
GAME( 1990, neruton,  0,        neruton,  neruton,  mjelct3,  ROT180, "Dynax / Yukiyoshi Tokoro", "Mahjong Neruton Haikujiradan (Japan)",                          GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE )
GAME( 1991, hanayara, 0,        yarunara, hanayara, 0,        ROT180, "Dynax",                    "Hana wo Yaraneba! (Japan)",                                     GAME_SUPPORTS_SAVE )
GAME( 1991, mjcomv1,  0,        yarunara, yarunara, 0,        ROT180, "Dynax",                    "Mahjong Comic Gekijou Vol.1 (Japan)",                           GAME_SUPPORTS_SAVE )
GAME( 1991, tenkai,   0,        tenkai,   tenkai,   0,        ROT0,   "Dynax",                    "Mahjong Tenkaigen",                                             GAME_SUPPORTS_SAVE )
GAME( 1991, tenkai2b, tenkai,   tenkai,   tenkai,   0,        ROT0,   "bootleg",                  "Mahjong Tenkaigen Part 2 (bootleg)",                            GAME_NOT_WORKING | GAME_SUPPORTS_SAVE )
GAME( 1991, tenkaibb, tenkai,   tenkai,   tenkai,   0,        ROT0,   "bootleg",                  "Mahjong Tenkaigen (bootleg b)",                                 GAME_SUPPORTS_SAVE )
GAME( 1991, tenkaicb, tenkai,   tenkai,   tenkai,   0,        ROT0,   "bootleg",                  "Mahjong Tenkaigen (bootleg c)",                                 GAME_SUPPORTS_SAVE )
GAME( 1991, tenkaid,  tenkai,   tenkai,   tenkai,   0,        ROT0,   "Dynax",                    "Mahjong Tenkaigen (set 1)",                                     GAME_NOT_WORKING | GAME_SUPPORTS_SAVE )
GAME( 1991, tenkaie,  tenkai,   tenkai,   tenkai,   0,        ROT0,   "Dynax",                    "Mahjong Tenkaigen (set 2)",                                     GAME_SUPPORTS_SAVE )
GAME( 1992, htengoku, 0,        htengoku, htengoku, 0,        ROT180, "Dynax",                    "Hanafuda Hana Tengoku (Japan)",                                 GAME_SUPPORTS_SAVE )
GAME( 1994, mjreach,  0,        tenkai,   mjreach,  mjreach,  ROT0,   "bootleg / Dynax",          "Mahjong Reach (bootleg)",                                       GAME_SUPPORTS_SAVE )
GAME( 1995, shpeng,   0,        sprtmtch, drgpunch, 0,        ROT0,   "WSAC Systems?",            "Sea Hunter Penguin",                                            GAME_WRONG_COLORS | GAME_SUPPORTS_SAVE ) // not a dynax board. proms?
GAME( 1996, majrjhdx, 0,        majrjhdx, tenkai,   0,        ROT0,   "Dynax",                    "Mahjong Raijinhai DX",                                          GAME_NOT_WORKING | GAME_SUPPORTS_SAVE )
